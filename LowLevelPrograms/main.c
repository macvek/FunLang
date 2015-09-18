//
//  main.c
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 08.09.2015.
//  Copyright (c) 2015 macvek. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "opcodes.h"

int main(int argc, const char * argv[]) {
    struct State state;
    struct CompilationState compilationState;
    Mem memory[512];
    memset(memory, 0, sizeof(memory));

    int helloWorldOffset = 256;
    char* string = "HelloWorld\n";
    long len = strlen(string);
    memcpy(memory+helloWorldOffset, string, len);
    
    int argNum = 0;
    
    InitState(&state, memory, sizeof(memory));
    InitCompilationState(&compilationState, memory, sizeof(memory));
    
    PutCall(&compilationState, "MethodEnter");
    PutByte(&compilationState, (argNum+1) * sizeof(Short));
    NextInstruction(&state);
    
    PutCall(&compilationState, "PushShortToStack");
    PutShort(&compilationState, helloWorldOffset);
    NextInstruction(&state);
    
    PutCall(&compilationState, "StoreShort");
    PutByte(&compilationState, argNum);
    NextInstruction(&state);
    
    PutCall(&compilationState, "LoadByte");
    PutByte(&compilationState, argNum);
    NextInstruction(&state);
    
    PutCall(&compilationState, "StoreByte");
    PutByte(&compilationState, argNum);
    NextInstruction(&state);
    
    PutCall(&compilationState, "LoadShort");
    PutByte(&compilationState, argNum);
    NextInstruction(&state);
    
    PutCall(&compilationState, "PushByteToStack");
    PutByte(&compilationState, 1);
    NextInstruction(&state);
    
    PutCall(&compilationState, "JumpIf");
    PutShort(&compilationState, CodeSizeForInstruction("ZeroOpcodeFail") + CodeSizeForInstruction("Jump"));
    
    PutCall(&compilationState, "ZeroOpcodeFail");
    
    PutCall(&compilationState, "Jump");
    PutShort(&compilationState, CodeSizeForInstruction("Jump") + CodeSizeForInstruction("ZeroOpcodeFail"));
    
    PutCall(&compilationState, "Jump");
    PutShort(&compilationState, -CodeSizeForInstruction("Jump")*2);
    
    PutCall(&compilationState, "ZeroOpcodeFail");
    
    PutCall(&compilationState, "PassToPutS");

    NextInstruction(&state);
    NextInstruction(&state);
    NextInstruction(&state);
    NextInstruction(&state);
    
    PutCall(&compilationState, "PushByteToStack");
    PutByte(&compilationState, 0);
    NextInstruction(&state);

    PutCall(&compilationState, "JumpIf");
    PutShort(&compilationState, -CodeSizeForInstruction("JumpIf") );
    NextInstruction(&state);
    
    PutCall(&compilationState, "PushByteToStack");
    PutByte(&compilationState, 2);
    NextInstruction(&state);
    
    PutCall(&compilationState, "PushByteToStack");
    PutByte(&compilationState, 2);
    NextInstruction(&state);
    
    PutCall(&compilationState, "CompareBytes");
    NextInstruction(&state);
    
    PutCall(&compilationState, "JumpIf");
    PutShort(&compilationState, CodeSizeForInstruction("ZeroOpcodeFail"));
    
    PutCall(&compilationState, "ZeroOpcodeFail");
    
    NextInstruction(&state);
    
    PutCall(&compilationState, "PushShortToStack");
    PutShort(&compilationState, 2000);
    NextInstruction(&state);
    
    PutCall(&compilationState, "PushShortToStack");
    PutShort(&compilationState, 2000);
    NextInstruction(&state);
    
    PutCall(&compilationState, "CompareShorts");
    NextInstruction(&state);
    
    PutCall(&compilationState, "JumpIf");
    PutShort(&compilationState, CodeSizeForInstruction("ZeroOpcodeFail"));
    
    PutCall(&compilationState, "ZeroOpcodeFail");
    
    NextInstruction(&state);

    PutCall(&compilationState, "PushShortToStack");
    PutShort(&compilationState, 100);
    NextInstruction(&state);
    
    PutCall(&compilationState, "StoreByte");
    PutByte(&compilationState, 1);
    NextInstruction(&state);
    
    PutCall(&compilationState, "LoadShort");
    PutByte(&compilationState, 1);
    NextInstruction(&state);
    
    PutCall(&compilationState, "Inc");
    NextInstruction(&state);
    
    PutCall(&compilationState, "Dec");
    NextInstruction(&state);
    
    PutCall(&compilationState, "LoadShort");
    PutByte(&compilationState, 1);
    NextInstruction(&state);
    
    PutCall(&compilationState, "CompareShorts");
    NextInstruction(&state);
    
    PutCall(&compilationState, "JumpIf");
    PutShort(&compilationState, CodeSizeForInstruction("ZeroOpcodeFail"));
    PutCall(&compilationState, "ZeroOpcodeFail");
    NextInstruction(&state);

    PutCall(&compilationState, "LoadShort");
    PutByte(&compilationState, 1);
    NextInstruction(&state);
    
    PutCall(&compilationState, "DupShort");
    NextInstruction(&state);
    
    PutCall(&compilationState, "CompareShorts");
    NextInstruction(&state);
    PutCall(&compilationState, "JumpIf");
    PutShort(&compilationState, CodeSizeForInstruction("ZeroOpcodeFail"));
    PutCall(&compilationState, "ZeroOpcodeFail");
    NextInstruction(&state);
    
    PutCall(&compilationState, "LoadByte");
    PutByte(&compilationState, 1);
    NextInstruction(&state);
    
    PutCall(&compilationState, "DupByte");
    NextInstruction(&state);
    
    PutCall(&compilationState, "CompareBytes");
    NextInstruction(&state);
    PutCall(&compilationState, "JumpIf");
    PutShort(&compilationState, CodeSizeForInstruction("ZeroOpcodeFail"));
    PutCall(&compilationState, "ZeroOpcodeFail");
    NextInstruction(&state);
    
    PutCall(&compilationState, "LoadShort");
    PutByte(&compilationState, 1);
    NextInstruction(&state);
    
    PutCall(&compilationState, "DupShort");
    NextInstruction(&state);

    PutCall(&compilationState, "Add");
    NextInstruction(&state);
    
    AssertStackTopShort(&state, 200);

    PutCall(&compilationState, "PushShortToStack");
    PutShort(&compilationState, -2);
    NextInstruction(&state);

    AssertStackTopShort(&state, -2);
    
    PutCall(&compilationState, "Mul");
    NextInstruction(&state);
    
    AssertStackTopShort(&state, -400);
    
    PutCall(&compilationState, "PushShortToStack");
    PutShort(&compilationState, -4);
    NextInstruction(&state);
    
    PutCall(&compilationState, "Div");
    NextInstruction(&state);
    
    AssertStackTopShort(&state, 100);
    
    PutCall(&compilationState, "PushShortToStack");
    PutShort(&compilationState, 99);
    NextInstruction(&state);
    
    PutCall(&compilationState, "Sub");
    NextInstruction(&state);
    
    AssertStackTopShort(&state, 1);
    
    int methodOffset = 200;
    
    MemPtr mainPC = compilationState.PC;
    {
        compilationState.PC = compilationState.memory + methodOffset;
        
        PutCall(&compilationState, "MethodEnter");
        PutByte(&compilationState, 0);

        PutCall(&compilationState, "PushShortToStack");
        PutShort(&compilationState, helloWorldOffset);
        
        PutCall(&compilationState, "PassToPutS");
        
        PutCall(&compilationState, "MethodExit");
    }
    compilationState.PC = mainPC;

    
    PutCall(&compilationState, "PushShortToStack");
    PutShort(&compilationState, methodOffset);
    NextInstruction(&state);
    
    PutCall(&compilationState, "Call");
    NextInstruction(&state);
    
    NextInstruction(&state);
    NextInstruction(&state);
    NextInstruction(&state);
    NextInstruction(&state);
    
    methodOffset = 200;
    int argValue = 0x40;
    
    PutCall(&compilationState, "PushByteToStack");
    PutByte(&compilationState, argValue);
    
    PutCall(&compilationState, "CallArg");
    PutShort(&compilationState, methodOffset);
    
    mainPC = compilationState.PC;
    {
        compilationState.PC = compilationState.memory + methodOffset;
        
        PutCall(&compilationState, "MethodEnter");
        PutByte(&compilationState, 1*sizeof(Short));
        
        PutCall(&compilationState, "SetupArgsStack");
        
        PutCall(&compilationState, "StoreByte");
        PutByte(&compilationState, 0);
        
        PutCall(&compilationState, "SetupFrameStack");
        
        PutCall(&compilationState, "LoadShort");
        PutByte(&compilationState, 0);
        
        PutCall(&compilationState, "MethodReturn");
        PutByte(&compilationState, sizeof(Short));
    }
    compilationState.PC = mainPC;

    NextInstruction(&state);
    NextInstruction(&state);
    NextInstruction(&state);
    NextInstruction(&state);

    AssertStackTopByte(&state, argValue);
    NextInstruction(&state);

    NextInstruction(&state);
    NextInstruction(&state);

    AssertStackTopShort(&state, argValue);
    NextInstruction(&state);
    
    AssertStackTopShort(&state, argValue);
    
    PutCall(&compilationState, "MethodExit");
    NextInstruction(&state);
    
    return 0;
}
