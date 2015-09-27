//
//  compiler.c
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 21.09.2015.
//  Copyright (c) 2015 macvek. All rights reserved.
//

#include "compiler.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
static CompilationStatePtr compilationState;

struct Reader {
    char* cursor;
    char* cursorLimit;
};

struct StringRange {
    char* start;
    char* end;
};

void InitReader(struct Reader* reader, char* sourceCode, int limit) {
    reader->cursor = sourceCode;
    reader->cursorLimit = sourceCode+limit;
}

bool CompareStringRanges(struct StringRange* one, struct StringRange *another) {
    if (one->end - one->start == another->end - another->start) {
        return 0 == memcmp(one->start, another->start, one->end - one->start);
    }
    else {
        return false;
    }
}

bool CompareStringRangeWithString(struct StringRange* one, char* another) {
    long anotherLength = strlen(another);
    if (one->end - one->start == anotherLength) {
        return 0 == memcmp(one->start, another, one->end - one->start);
    }
    else {
        return false;
    }
}


void SkipWhiteSpaces(struct Reader* reader);
bool MatchTillNextSpace(struct Reader* reader, const char* source);
bool MatchAndAdvanceTillNextSpace(struct Reader* reader, const char* source);
char* NextSpaceInReader(struct Reader* reader);
char* NextCharInReader(struct Reader* reader, char character);
bool AdvanceReaderIfStartsWithChar(struct Reader* reader, char character);
void PrintRange(char* prefix, struct StringRange* range) {
    long size = range->end - range->start;
    char text[size];
    
    memcpy(text, range->start, size);
    text[size] = 0;
    printf("%s: %s\n", prefix, text);
}


struct MethodSignature {
    struct Reader reader;
    struct StringRange type;
    struct StringRange name;
    
    char* signatureEnd;

    bool error;
};

struct MethodSignature FindMethodSignature(char* sourceCode, int limit);
void FindMethodReturnType(struct MethodSignature* methodSignature);
void FindMethodName(struct MethodSignature* methodSignature);
void FindMethodArguments(struct MethodSignature* methodSignature);


struct ConstSignature {
    struct Reader reader;
    
    struct StringRange type;
    struct StringRange name;
    struct StringRange value;
    
    Addr allocStart;
    Addr allocEnd;
    char* signatureEnd;
    bool error;
};

struct ConstSignature FindConstSignature(char* sourceCode, int limit);
void FindConstPrefix(struct ConstSignature* constSignature);
void FindConstType(struct ConstSignature* constSignature);
void FindConstName(struct ConstSignature* constSignature);
void FindConstValue(struct ConstSignature* constSignature);

struct CallSignature {
    struct Reader reader;
    
    struct StringRange name;
    struct StringRange arguments;
    
    char* signatureEnd;
    bool error;
};

struct CallSignature FindCallSignature(char* sourceCode, int limit);
void FindCallName(struct CallSignature* callSignature);
void FindCallArgs(struct CallSignature* callSignature);


void CompileCode(char* sourceCode, int sourceCodeSize, CompilationStatePtr aCompilationState) {
    compilationState = aCompilationState;
    char* readPtr = sourceCode;
    int readLimit = sourceCodeSize;
    
    struct MethodSignature methodSignature = FindMethodSignature(readPtr, readLimit);
    if (false == methodSignature.error) {
        readPtr = methodSignature.signatureEnd;
        readLimit = (int)(sourceCodeSize - (readPtr - sourceCode));
        PutCall(compilationState, "MethodEnter");
        PutByte(compilationState, 0);
        PutCall(compilationState, "MethodExit");
    }
    struct ConstSignature constSignature = FindConstSignature(readPtr, readLimit);
    if (false == constSignature.error) {
        readPtr = constSignature.signatureEnd;
        readLimit = (int)(sourceCodeSize - (readPtr - sourceCode));
        constSignature.allocStart = 256;
        constSignature.allocEnd = constSignature.allocStart + constSignature.value.end - constSignature.value.start+1;
        memcpy(compilationState->memory+constSignature.allocStart,
               constSignature.value.start,
               constSignature.value.end - constSignature.value.start);
        
        constSignature.value.end = 0;
    }
    struct CallSignature callSignature = FindCallSignature(readPtr, readLimit);
    if (false == callSignature.error) {
        if (CompareStringRangeWithString(&callSignature.name, "puts") &&
            CompareStringRanges(&callSignature.arguments, &constSignature.name)) {

            PutCall(compilationState, "MethodEnter");
            PutByte(compilationState, 0);
            
            PutCall(compilationState, "PushShortToStack");
            PutShort(compilationState, constSignature.allocStart);
            PutCall(compilationState, "PassToPutS");
            
            PutCall(compilationState, "MethodExit");
        }
    }
}



struct MethodSignature FindMethodSignature(char* sourceCode, int limit) {
    struct MethodSignature methodSignature;
    memset(&methodSignature, 0, sizeof(methodSignature));
    
    char* signatureEnd = strnstr(sourceCode, "{", limit);
    if (signatureEnd != NULL) {
        methodSignature.reader.cursor = sourceCode;
        methodSignature.reader.cursorLimit = signatureEnd;
        
        FindMethodReturnType(&methodSignature);
        if (methodSignature.error) {
            goto returnStmt;
        }
        FindMethodName(&methodSignature);
        if (methodSignature.error) {
            goto returnStmt;
        }
        FindMethodArguments(&methodSignature);
        methodSignature.signatureEnd = signatureEnd;
    }
    else {
        methodSignature.error = true;
    }
    
returnStmt:
    return methodSignature;
}

void SkipWhiteSpaces(struct Reader* reader) {
    
    while(reader->cursor < reader->cursorLimit &&
          (
           *reader->cursor == ' ' ||
           *reader->cursor == '\n' ||
           *reader->cursor == '\r' ||
           *reader->cursor == '\t'
          ) ) {
        reader->cursor++;
    }
}

bool MatchAndAdvanceTillNextSpace(struct Reader* reader, const char* source) {
    if (MatchTillNextSpace(reader, source)) {
        reader->cursor += strlen(source);
        return true;
    }
    else {
        return false;
    }
}

bool MatchTillNextSpace(struct Reader* reader, const char* source) {
    char* nextSpace = NextSpaceInReader(reader);
    if (nextSpace == NULL) {
        return false;
    }
    
    int length = (int)(nextSpace - reader->cursor);
    
    return 0 == strncmp(reader->cursor, source, length);
}

char* NextCharInReader(struct Reader* reader, char character) {
    char seek[2];
    seek[0] = character;
    seek[1] = 0;
    if (reader->cursorLimit <= reader->cursor) {
        return NULL;
    }
    char* nextSpace = strnstr(reader->cursor, seek, reader->cursorLimit - reader->cursor);
    return nextSpace < reader->cursorLimit ? nextSpace : NULL;
}


char* NextSpaceInReader(struct Reader* reader) {
    return NextCharInReader(reader, ' ');
}

bool AdvanceReaderIfStartsWithChar(struct Reader* reader, char character) {
    if (reader->cursor < reader->cursorLimit && *reader->cursor == character) {
        reader->cursor++;
        return true;
    }
    else {
        return false;
    }
}

void FindMethodReturnType(struct MethodSignature* methodSignature) {
    const char* voidName = "void";
    SkipWhiteSpaces(&methodSignature->reader);
    
    char* nextSpace = NextSpaceInReader(&methodSignature->reader);
    if (nextSpace == NULL) {
        methodSignature->error = true;
        return;
    }
    
    if (false == MatchTillNextSpace(&methodSignature->reader, voidName)) {
        methodSignature->error = true;
        return;
    }
    
    methodSignature->type.start = methodSignature->reader.cursor;
    methodSignature->type.end = methodSignature->type.start + strlen(voidName);
    
    methodSignature->reader.cursor = methodSignature->type.end;
}

void FindMethodName(struct MethodSignature* methodSignature) {
    SkipWhiteSpaces(&methodSignature->reader);

    char* nextSpace = NextSpaceInReader(&methodSignature->reader);
    if (nextSpace == NULL) {
        methodSignature->error = true;
        return;
    }
    
    methodSignature->name.start = methodSignature->reader.cursor;
    methodSignature->name.end = nextSpace;
    
}

void FindMethodArguments(struct MethodSignature* methodSignature) {
    
}

struct ConstSignature FindConstSignature(char* sourceCode, int limit) {
    struct ConstSignature constSignature;
    memset(&constSignature, 0, sizeof(constSignature));
    InitReader(&constSignature.reader, sourceCode, limit);
    
    FindConstPrefix(&constSignature);
    if (constSignature.error) {
        return constSignature;
    }
    
    FindConstType(&constSignature);
    if (constSignature.error) {
        return constSignature;
    }
    
    FindConstName(&constSignature);
    if (constSignature.error) {
        return constSignature;
    }
    
    FindConstValue(&constSignature);
    if (false == constSignature.error) {
        constSignature.signatureEnd = constSignature.value.end+1;
    }
    return constSignature;
}

void FindConstPrefix(struct ConstSignature* constSignature) {
    const char* constName = "const";
    SkipWhiteSpaces(&constSignature->reader);
    
    constSignature->error = false == MatchAndAdvanceTillNextSpace(&constSignature->reader, constName);
}

void FindConstType(struct ConstSignature* constSignature) {
    const char* constTypeName = "string";
    SkipWhiteSpaces(&constSignature->reader);

    constSignature->error = false == MatchAndAdvanceTillNextSpace(&constSignature->reader, constTypeName);
}

void FindConstName(struct ConstSignature* constSignature) {
    SkipWhiteSpaces(&constSignature->reader);
    char* nextSpace = NextSpaceInReader(&constSignature->reader);
    if (nextSpace == NULL) {
        constSignature->error = true;
        return;
    }

    constSignature->name.start = constSignature->reader.cursor;
    constSignature->name.end = nextSpace;
    constSignature->reader.cursor = nextSpace;
}

void FindConstValue(struct ConstSignature* constSignature) {
    SkipWhiteSpaces(&constSignature->reader);
    if (false == AdvanceReaderIfStartsWithChar(&constSignature->reader, '=')) {
        constSignature->error = true;
        return;
    }
    SkipWhiteSpaces(&constSignature->reader);
    if (false == AdvanceReaderIfStartsWithChar(&constSignature->reader, '\'')) {
        constSignature->error = true;
        return;
    }
    char* closingParathesis = NextCharInReader(&constSignature->reader, '\'');
    if (NULL == closingParathesis) {
        constSignature->error = true;
        return;
    }
    
    constSignature->value.start = constSignature->reader.cursor;
    constSignature->value.end = closingParathesis;
}

struct CallSignature FindCallSignature(char* sourceCode, int limit) {
    struct CallSignature callSignature;
    memset(&callSignature, 0, sizeof(callSignature));
    InitReader(&callSignature.reader, sourceCode, limit);

    
    FindCallName(&callSignature);
    if (callSignature.error) {
        return callSignature;
    }
    
    FindCallArgs(&callSignature);
    return callSignature;
}

void FindCallName(struct CallSignature* callSignature) {
    SkipWhiteSpaces(&callSignature->reader);
    char* nextSpace = NextSpaceInReader(&callSignature->reader);
    if (nextSpace == NULL) {
        callSignature->error = true;
    }
    else {
        callSignature->name.start = callSignature->reader.cursor;
        callSignature->name.end = nextSpace;
        callSignature->reader.cursor = nextSpace;
    }
}

void FindCallArgs(struct CallSignature* callSignature) {
    SkipWhiteSpaces(&callSignature->reader);
    if (false == AdvanceReaderIfStartsWithChar(&callSignature->reader, '(')) {
        callSignature->error = true;
        return;
    }
    char* closingBracket = NextCharInReader(&callSignature->reader, ')');
    if (closingBracket == NULL) {
        callSignature->error = true;
    }
    
    callSignature->arguments.start = callSignature->reader.cursor;
    callSignature->arguments.end = closingBracket;
    
    callSignature->reader.cursor = closingBracket+1;
}

