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

struct ParserState {
    int lineNumber;
    int charNumber;
    int readPtr;
    char* sourceCode;
    bool parseSuccess;
    
    
} parserState;

struct MethodSignature {
    char* cursor;
    char* cursorLimit;

    char* returnTypeStart;
    char* returnTypeEnd;
    
    char* nameStart;
    char* nameEnd;
    
    int limit;
    bool error;
};

struct MethodSignature FindMethodSignature(char* sourceCode, int limit);
void FindMethodReturnType(struct MethodSignature* methodSignature);
void FindMethodName(struct MethodSignature* methodSignature);
void FindMethodArguments(struct MethodSignature* methodSignature);


void CompileCode(char* sourceCode, int sourceCodeSize, CompilationStatePtr aCompilationState) {
    compilationState = aCompilationState;

    int nextLine = -1;
    int counter = 0;
    for (;counter < sourceCodeSize;counter++) {
        if (sourceCode[counter] == '\n') {
            nextLine = counter;
            break;
        }
    }
    
    if (nextLine >= 0) {
        struct MethodSignature methodSignature = FindMethodSignature(sourceCode, nextLine);
        if (methodSignature.error) {
            printf("FindMethodSignature returns with error\n");
        }
        else {
            PutCall(compilationState, "MethodEnter");
            PutByte(compilationState, 0);
            PutCall(compilationState, "MethodExit");
        }
        
    }
    else {
        printf("Next line not found in %s\n", sourceCode);
        exit(1);
    }
}



struct MethodSignature FindMethodSignature(char* sourceCode, int limit) {
    struct MethodSignature methodSignature;
    memset(&methodSignature, 0, sizeof(methodSignature));
    
    char* endOfSignature = strnstr(sourceCode, "{", limit);
    if (endOfSignature != NULL) {
        methodSignature.cursor = sourceCode;
        methodSignature.cursorLimit = endOfSignature;
        
        FindMethodReturnType(&methodSignature);
        if (methodSignature.error) {
            goto returnStmt;
        }
        FindMethodName(&methodSignature);
        if (methodSignature.error) {
            goto returnStmt;
        }
        FindMethodArguments(&methodSignature);
    }
    else {
        methodSignature.error = true;
    }
    
returnStmt:
    return methodSignature;
}

void SkipSpaces(struct MethodSignature* methodSignature) {
    while(*methodSignature->cursor == ' ' && methodSignature->cursor < methodSignature->cursorLimit) {
        methodSignature->cursor++;
    }
}

char* NextSpaceInMethodSignature(struct MethodSignature* methodSignature) {
    char* nextSpace = strnstr(methodSignature->cursor, " ", methodSignature->cursorLimit - methodSignature->cursor);
    return nextSpace < methodSignature->cursorLimit ? nextSpace : NULL;
}

void FindMethodReturnType(struct MethodSignature* methodSignature) {
    char* voidName = "void";
    SkipSpaces(methodSignature);
    
    char* nextSpace = NextSpaceInMethodSignature(methodSignature);
    if (nextSpace == NULL) {
        methodSignature->error = true;
        return;
    }
    
    if (0 != strncmp(methodSignature->cursor, voidName, nextSpace - methodSignature->cursor)) {
        methodSignature->error = true;
        return;
    }
    
    methodSignature->returnTypeStart = methodSignature->cursor;
    methodSignature->returnTypeEnd = methodSignature->returnTypeStart + strlen(voidName);
    
    methodSignature->cursor = methodSignature->returnTypeEnd;
}

void FindMethodName(struct MethodSignature* methodSignature) {
    SkipSpaces(methodSignature);

    char* nextSpace = NextSpaceInMethodSignature(methodSignature);
    if (nextSpace == NULL) {
        methodSignature->error = true;
        return;
    }
    
    methodSignature->nameStart = methodSignature->cursor;
    methodSignature->nameEnd = nextSpace;
    
}

void FindMethodArguments(struct MethodSignature* methodSignature) {
    
}
