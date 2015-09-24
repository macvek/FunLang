//
//  compilertest.c
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 21.09.2015.
//  Copyright (c) 2015 macvek. All rights reserved.
//

#include "compilertest.h"
#include "compiler.h"

static struct CompilationState compilationState;
static struct State runtimeState;

static void testShouldCreateEmpty();

void CompilerTest() {
    
    Mem memory[1024];
    InitCompilationState(&compilationState, memory, sizeof(memory));
    InitState(&runtimeState, memory, sizeof(memory));
    
    testShouldCreateEmpty();
    
}

static void testShouldCreateEmpty() {
    char text[] = "void method () {\n"
    "}";
    
    CompileCode(text, sizeof(text), &compilationState);
    
    EvaluateMethod(&runtimeState);
}
