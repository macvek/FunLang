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

void InitReaderCopy(struct Reader* destination, const struct Reader* source) {
    destination->cursor = source->cursor;
    destination->cursorLimit = source->cursorLimit;
}

bool IsNotEmptyReader(struct Reader* reader) {
    return reader->cursor < reader->cursorLimit;
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

void PrintSourceCodeWithMarker(char* sourceCode, char* pointer) {
    if (*sourceCode == 0) {
        return;
    }
    char* lastLine = sourceCode;
    for(;;) {
        if (*sourceCode == '\n' || *sourceCode == 0) {
            long len = sourceCode-lastLine;
            char lineToPrint[len+1];
            memcpy(lineToPrint, lastLine, len);
            lineToPrint[len] = 0;
            printf("%s",lineToPrint);
            
            if (pointer < sourceCode) {
                printf("\n");
                long len = pointer-(lastLine+1);
                for (int i=0;i<len;i++) {
                    printf("%c", '-');
                }
                printf("%c\n",'^');
                return;
            }
            else {
                lastLine = sourceCode;
            }
        }
        sourceCode++;
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

struct MethodSignature FindMethodSignature(struct Reader* sourceCodeReader);
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

struct ConstSignature FindConstSignature(struct Reader* sourceCodeReader);
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

struct CallSignature FindCallSignature(struct Reader* sourceCodeReader);
void FindCallName(struct CallSignature* callSignature);
void FindCallArgs(struct CallSignature* callSignature);

struct EndBlockSignature {
    char* signatureEnd;
    bool error;
};

struct EndBlockSignature FindEndBlock(struct Reader* sourceCodeReader);

void CompileCode(char* sourceCode, int sourceCodeSize, CompilationStatePtr aCompilationState) {
    compilationState = aCompilationState;
    struct Reader sourceCodeReader;
    sourceCodeReader.cursor = sourceCode;
    sourceCodeReader.cursorLimit = sourceCode+sourceCodeSize;
    
    while(IsNotEmptyReader(&sourceCodeReader) && *sourceCodeReader.cursor != 0) {
        
        
        bool noAction = true;
        struct MethodSignature methodSignature = FindMethodSignature(&sourceCodeReader);
        if (false == methodSignature.error) {
            sourceCodeReader.cursor = methodSignature.signatureEnd+1;
            PutCall(compilationState, "MethodEnter");
            PutByte(compilationState, 0);
            noAction = false;
        }
        struct ConstSignature constSignature = FindConstSignature(&sourceCodeReader);
        if (false == constSignature.error) {
            sourceCodeReader.cursor = constSignature.signatureEnd+1;
            constSignature.allocStart = 256;
            constSignature.allocEnd = constSignature.allocStart + constSignature.value.end - constSignature.value.start+1;
            memcpy(compilationState->memory+constSignature.allocStart,
                   constSignature.value.start,
                   constSignature.value.end - constSignature.value.start);
            
            constSignature.value.end = 0;
            noAction = false;
        }
        struct CallSignature callSignature = FindCallSignature(&sourceCodeReader);
        if (false == callSignature.error) {
            if (CompareStringRangeWithString(&callSignature.name, "puts") &&
                CompareStringRanges(&callSignature.arguments, &constSignature.name)) {

                PutCall(compilationState, "PushShortToStack");
                PutShort(compilationState, constSignature.allocStart);
                PutCall(compilationState, "PassToPutS");
                sourceCodeReader.cursor = callSignature.signatureEnd;
            }
            noAction = false;
        }
        struct EndBlockSignature endBlockSignature = FindEndBlock(&sourceCodeReader);
        if (false == endBlockSignature.error) {
            sourceCodeReader.cursor = endBlockSignature.signatureEnd;
            PutCall(compilationState, "MethodExit");
            noAction = false;
        }
        
        SkipWhiteSpaces(&sourceCodeReader);
        if (noAction) {
            PrintSourceCodeWithMarker(sourceCode, sourceCodeReader.cursor);
            exit(1);
        }
    }
}



struct MethodSignature FindMethodSignature(struct Reader* sourceCodeReader) {
    struct MethodSignature methodSignature;
    memset(&methodSignature, 0, sizeof(methodSignature));
    
    char* signatureEnd = strnstr(sourceCodeReader->cursor, "{", sourceCodeReader->cursorLimit - sourceCodeReader->cursor);
    if (signatureEnd != NULL) {
        methodSignature.reader.cursor = sourceCodeReader->cursor;
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

struct EndBlockSignature FindEndBlock(struct Reader* sourceCodeReader) {
    struct EndBlockSignature endBlockSignature;
    struct Reader reader;
    InitReaderCopy(&reader, sourceCodeReader);
    SkipWhiteSpaces(&reader);

    if (IsNotEmptyReader(&reader)) {
        endBlockSignature.error = *reader.cursor != '}';
        endBlockSignature.signatureEnd = reader.cursor+1;
    }
    else {
        endBlockSignature.error = true;
    }
    return endBlockSignature;
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

struct ConstSignature FindConstSignature(struct Reader* sourceCodeReader) {
    struct ConstSignature constSignature;
    memset(&constSignature, 0, sizeof(constSignature));
    InitReaderCopy(&constSignature.reader, sourceCodeReader);
    
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

struct CallSignature FindCallSignature(struct Reader* sourceCodeReader) {
    struct CallSignature callSignature;
    memset(&callSignature, 0, sizeof(callSignature));
    InitReaderCopy(&callSignature.reader, sourceCodeReader);

    FindCallName(&callSignature);
    if (callSignature.error) {
        return callSignature;
    }
    
    FindCallArgs(&callSignature);
    if (callSignature.error) {
        return callSignature;
    }
    
    callSignature.signatureEnd = callSignature.reader.cursor;
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

