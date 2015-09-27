//
//  compilertest.c
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 21.09.2015.
//  Copyright (c) 2015 macvek. All rights reserved.
//

#include "compilertest.h"
#include "compiler.h"
#include <stdio.h>

static struct CompilationState compilationState;
static struct State runtimeState;

static void testShouldCreateEmpty();
static void testShouldPrintConstString();

void CompilerTest() {
    
    Mem memory[1024];
    InitCompilationState(&compilationState, memory, sizeof(memory));
    InitState(&runtimeState, memory, sizeof(memory));
    
    testShouldCreateEmpty();

    InitCompilationState(&compilationState, memory, sizeof(memory));
    InitState(&runtimeState, memory, sizeof(memory));
    testShouldPrintConstString();
}

static void testShouldCreateEmpty() {
    char text[] = "void method () {\n"
    "}";
    
    CompileCode(text, sizeof(text), &compilationState);
    
    EvaluateMethod(&runtimeState);
    printf("[PASSED] testShouldCreateEmpty\n");
}

static void testShouldPrintConstString() {
    char text[] = "const string someName = 'YOU SHOULD SEE ME TWICE'\n"
        "puts (someName)";
    
    CompileCode(text, sizeof(text), &compilationState);
    
    EvaluateMethod(&runtimeState);
    printf("YOU SHOULD SEE ME TWICE\n");
}

