//
//  parsertest.c
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 30.09.2015.
//  Copyright © 2015 macvek. All rights reserved.
//

#include <stdio.h>
#include "parser.h"
#include "parsertest.h"
#include <assert.h>
#include <string.h>
#include "testing.h"

static void testShouldParseEmptyMethod();
static void testShouldParseConstStringAndAnotherMethodCall();
static void testShouldParseVariableAssignment();
    
static struct CodeUnit *codeUnit;

static void runTest(TestingCallback test) {
    codeUnit = NULL;
    test();
    FreeCodeUnit(codeUnit);
}

void ParserTest() {
    printf(">>> ParserTest \n");
    runTest(testShouldParseEmptyMethod);
    runTest(testShouldParseConstStringAndAnotherMethodCall);
    runTest(testShouldParseVariableAssignment);
}


static void testShouldParseEmptyMethod() {
    char text[] = "void main () {\n"
    "}";
    codeUnit = ParseSourceCode(text);
    assert(NULL != codeUnit);
    assert(NULL != codeUnit->linkedMethods);
    assert(NULL == codeUnit->linkedConsts);
    assert(NULL == codeUnit->linkedSymbols);
    
    struct MethodUnit *method = codeUnit->linkedMethods;
    assert(0 == strcmp("main", method->name));
    printf("[PASSED] testShouldParseEmptyMethod\n");
}

static void testShouldParseConstStringAndAnotherMethodCall() {
    char text[] = "void main () {\n"
    "   const string anotherName = 'YOU SHOULD SEE ME TWICE'\n"
    "   puts ( anotherName )\n"
    "}\n";
    
    codeUnit = ParseSourceCode(text);
    assert(NULL != codeUnit);
    assert(NULL != codeUnit->linkedMethods);
    assert(NULL != codeUnit->linkedMethods->linkedLocalConsts);

    assert(0 == strcmp("anotherName", codeUnit->linkedMethods->linkedLocalConsts->name));
    assert(0 == strcmp("YOU SHOULD SEE ME TWICE", codeUnit->linkedMethods->linkedLocalConsts->value));
    
    printf("[PASSED] testShouldParseConstStringAndAnotherMethodCall\n");
}

static void testShouldParseVariableAssignment() {
    char text[] = "myVariable = 2\n";
    
    codeUnit = ParseSourceCode(text);
    assert(NULL != codeUnit);
    assert(NULL != codeUnit->linkedExpressions);
    assert(codeUnit->linkedExpressions->type == ASSIGNMENT);
    assert(0 == strcmp("myVariable", codeUnit->linkedExpressions->value->assignment->leftValue));
    assert(0 == strcmp("2", codeUnit->linkedExpressions->value->assignment->rightValue));
}