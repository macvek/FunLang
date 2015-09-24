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

void SkipWhiteSpaces(struct Reader* reader);
bool MatchTillNextSpace(struct Reader* reader, const char* source);
bool MatchAndAdvanceTillNextSpace(struct Reader* reader, const char* source);
char* NextSpaceInReader(struct Reader* reader);
char* NextCharInReader(struct Reader* reader, char character);
bool AdvanceReaderIfStartsWithChar(struct Reader* reader, char character);

struct MethodSignature {
    struct Reader reader;
    char* typeStart;
    char* typeEnd;
    
    char* nameStart;
    char* nameEnd;
    
    char* signatureEnd;

    bool error;
};

struct MethodSignature FindMethodSignature(char* sourceCode, int limit);
void FindMethodReturnType(struct MethodSignature* methodSignature);
void FindMethodName(struct MethodSignature* methodSignature);
void FindMethodArguments(struct MethodSignature* methodSignature);


struct ConstSignature {
    struct Reader reader;
    
    char* typeStart;
    char* typeEnd;
    
    char* nameStart;
    char* nameEnd;
    
    char* valueStart;
    char* valueEnd;
    
    char* signatureEnd;
    bool error;
};

struct ConstSignature FindConstSignature(char* sourceCode, int limit);
void FindConstPrefix(struct ConstSignature* constSignature);
void FindConstType(struct ConstSignature* constSignature);
void FindConstName(struct ConstSignature* constSignature);
void FindConstValue(struct ConstSignature* constSignature);



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
    while(*reader->cursor == ' ' && reader->cursor < reader->cursorLimit) {
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
    char* nextSpace = strnstr(reader->cursor, seek, reader->cursorLimit - reader->cursor);
    return nextSpace < reader->cursorLimit ? nextSpace : NULL;
}


char* NextSpaceInReader(struct Reader* reader) {
    return NextCharInReader(reader, ' ');
}

bool AdvanceReaderIfStartsWithChar(struct Reader* reader, char character) {
    if (*reader->cursor == character) {
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
    
    methodSignature->typeStart = methodSignature->reader.cursor;
    methodSignature->typeEnd = methodSignature->typeStart + strlen(voidName);
    
    methodSignature->reader.cursor = methodSignature->typeEnd;
}

void FindMethodName(struct MethodSignature* methodSignature) {
    SkipWhiteSpaces(&methodSignature->reader);

    char* nextSpace = NextSpaceInReader(&methodSignature->reader);
    if (nextSpace == NULL) {
        methodSignature->error = true;
        return;
    }
    
    methodSignature->nameStart = methodSignature->reader.cursor;
    methodSignature->nameEnd = nextSpace;
    
}

void FindMethodArguments(struct MethodSignature* methodSignature) {
    
}

struct ConstSignature FindConstSignature(char* sourceCode, int limit) {
    struct ConstSignature constSignature;
    memset(&constSignature, 0, sizeof(constSignature));
    constSignature.reader.cursor = sourceCode;
    constSignature.reader.cursorLimit = sourceCode + limit;

    
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
        constSignature.signatureEnd = constSignature.valueEnd+1;
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

    constSignature->nameStart = constSignature->reader.cursor;
    constSignature->nameEnd = nextSpace;
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
    
    constSignature->valueStart = constSignature->reader.cursor;
    constSignature->valueEnd = closingParathesis;
}
