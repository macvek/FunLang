//
//  parser.c
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 30.09.2015.
//  Copyright © 2015 macvek. All rights reserved.
//
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "sourcecodereader.h"

static char* NewString(const char* copyFrom);
static void FreeConstUnit(struct ConstUnit* constUnit);
static void FreeConstUnitsList(struct ConstUnit* constUnit);
static void FreeMethodUnit(struct MethodUnit *methodUnit);
static void FreeExpressionAssignment(struct AssignmentUnit *assignment);
static void FreeExpressionUnit(struct ExpressionUnit *expressionUnit);
static void FreeExpressionUnitsList(struct ExpressionUnit *expressionUnit);
static void FreeMethodUnitsList(struct MethodUnit *methodUnit);


struct CodeUnit *ParseSourceCode(char* sourceCode) {
    struct CodeUnit* codeUnit = malloc(sizeof(struct CodeUnit));
    memset(codeUnit, 0, sizeof(struct CodeUnit));
    
    codeUnit->linkedMethods = malloc(sizeof(struct MethodUnit));
    codeUnit->linkedMethods->name = NewString("main");
    
    codeUnit->linkedMethods->linkedLocalConsts = malloc(sizeof(struct ConstUnit));
    codeUnit->linkedMethods->linkedLocalConsts->name = NewString("anotherName");
    codeUnit->linkedMethods->linkedLocalConsts->value = NewString("YOU SHOULD SEE ME TWICE");
    
    codeUnit->linkedExpressions = malloc(sizeof(struct ExpressionUnit));
    codeUnit->linkedExpressions->type = ASSIGNMENT;
    codeUnit->linkedExpressions->value = malloc(sizeof(union ExpressionValue));
    codeUnit->linkedExpressions->value->assignment = malloc(sizeof(struct AssignmentUnit));
    codeUnit->linkedExpressions->value->assignment->leftValue = NewString("myVariable");
    codeUnit->linkedExpressions->value->assignment->rightValue = NewString("2");
    
    return codeUnit;
}

void FreeCodeUnit(struct CodeUnit *codeUnit) {
    FreeMethodUnitsList(codeUnit->linkedMethods);
    FreeConstUnitsList(codeUnit->linkedConsts);
    FreeExpressionUnitsList(codeUnit->linkedExpressions);
    free(codeUnit);
}


void TODOParseSourceCode(char* sourceCode, int sourceCodeSize, CompilationStatePtr aCompilationState) {
    struct Reader sourceCodeReader;
    sourceCodeReader.cursor = sourceCode;
    sourceCodeReader.cursorLimit = sourceCode+sourceCodeSize;
    
    while(IsNotEmptyReader(&sourceCodeReader) && *sourceCodeReader.cursor != 0) {
        
        
        bool noAction = true;
        
        if (false == true) {
            
            noAction = false;
        }
        
        SkipWhiteSpaces(&sourceCodeReader);
        if (noAction) {
            PrintSourceCodeWithMarker(sourceCode, sourceCodeReader.cursor);
            exit(1);
        }
    }
    
    
}


static void FreeConstUnit(struct ConstUnit* constUnit) {
    free(constUnit->name);
    free(constUnit->value);
    free(constUnit);
}

static void FreeConstUnitsList(struct ConstUnit* constUnit) {
    if (NULL == constUnit) {
        return;
    }
    
    struct ConstUnit *current = constUnit;
    do {
        struct ConstUnit *next = current->siblings.next.constUnit;
        FreeConstUnit(current);
        current = next;
    } while(current != NULL);
}

static void FreeMethodUnit(struct MethodUnit *methodUnit) {
    free(methodUnit->name);
    FreeConstUnitsList(methodUnit->linkedLocalConsts);
    free(methodUnit);
}

static void FreeExpressionAssignment(struct AssignmentUnit *assignment) {
    if (assignment->leftValue) {
        free(assignment->leftValue);
    }
    if (assignment->rightValue) {
        free(assignment->rightValue);
    }
}

static void FreeExpressionUnit(struct ExpressionUnit *expressionUnit) {
    if (expressionUnit->type == ASSIGNMENT) {
        FreeExpressionAssignment(expressionUnit->value->assignment);
    }
}

static void FreeExpressionUnitsList(struct ExpressionUnit *expressionUnit) {
    if (NULL == expressionUnit) {
        return;
    }
    
    struct ExpressionUnit *current = expressionUnit;
    do {
        struct ExpressionUnit *next = current->siblings.next.expressionUnit;
        FreeExpressionUnit(current);
        current = next;
    } while(current != NULL);
}


static void FreeMethodUnitsList(struct MethodUnit *methodUnit) {
    if (NULL == methodUnit) {
        return;
    }
    
    struct MethodUnit *current = methodUnit;
    do {
        struct MethodUnit *next = current->siblings.next.methodUnit;
        FreeMethodUnit(current);
        current = next;
    } while(current != NULL);
}


char* NewString(const char* copyFrom) {
    char* newString = malloc(strlen(copyFrom)+1);
    strcpy(newString, copyFrom);
    return newString;
}



