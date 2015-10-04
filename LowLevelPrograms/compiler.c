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
#include "sourcecodereader.h"
#include <string.h>
static CompilationStatePtr compilationState;


struct MethodSignature {
    struct Reader reader;
    struct StringRange type;
    struct StringRange name;
    
    char* signatureEnd;
    Addr pcOffset;
    
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
struct MethodSignature definedMethodSignature;

void CompileCode(char* sourceCode, int sourceCodeSize, CompilationStatePtr aCompilationState) {
    definedMethodSignature.signatureEnd = NULL;
    compilationState = aCompilationState;
    MemPtr initialPC = compilationState->PC;
    struct MethodSignature methodSignature;
    struct Reader sourceCodeReader;
    sourceCodeReader.cursor = sourceCode;
    sourceCodeReader.cursorLimit = sourceCode+sourceCodeSize;
    
    PutCall(compilationState, "Call");
    PutShort(compilationState, -1);
    
    Short mainAddr = -1;
    
    while(IsNotEmptyReader(&sourceCodeReader) && *sourceCodeReader.cursor != 0) {
        
        
        bool noAction = true;
        methodSignature = FindMethodSignature(&sourceCodeReader);
        if (false == methodSignature.error) {
            sourceCodeReader.cursor = methodSignature.signatureEnd+1;
            methodSignature.pcOffset = compilationState->PC - compilationState->memory;
            if (CompareStringRangeWithString(&methodSignature.name, "main")) {
                mainAddr = aCompilationState->PC - aCompilationState->memory;
            }
            else {
                definedMethodSignature = methodSignature;
            }
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
            else if (definedMethodSignature.signatureEnd != NULL && CompareStringRanges(&callSignature.name, &definedMethodSignature.name)) {
                PutCall(compilationState, "Call");
                PutShort(compilationState, definedMethodSignature.pcOffset);
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
    
    if (mainAddr == -1) {
        printf("No void main () defined\n");
        exit(1);
    }

    compilationState->PC = initialPC;
    PutCall(compilationState, "Call");
    PutShort(compilationState, mainAddr);

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

