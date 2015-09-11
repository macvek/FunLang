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

typedef unsigned char byte;
typedef unsigned short Short;
typedef byte* MemPtr;

typedef struct State {
    MemPtr stack;
    MemPtr memory;
    MemPtr endOfProgram;
    MemPtr args;
    MemPtr PC;
} *StatePtr ;

typedef struct CompilationState {
    MemPtr memory;
    MemPtr PC;
    int argsToPush;
    MemPtr endOfMemory;
} *CompilationStatePtr;

typedef void (*apicall)(StatePtr);
void PassToPutS(StatePtr);
void PushShortToStack(StatePtr);

struct APICall {
    char* name;
    apicall call;
    int opsSize;
};

struct APICall ApiCallsList[] = {
    {"PassToPutS", &PassToPutS, 0},
    {"PushShortToStack", &PushShortToStack, sizeof(Short)},
    {NULL,NULL,-1}
};

Short PopShortFromStack(StatePtr state) {
    state->stack += sizeof(Short);
    Short value = *(Short*)state->stack;
    return value;
}

Short GetShortArg(StatePtr state) {
    Short value = *((Short*)state->args);
    state->args += sizeof(Short);
    return value;
}

void PushShortToStack(StatePtr state) {
    if (state->stack < state->memory) {
        puts("virtual stack overflow\n");
        exit(1);
    }
    Short *destination = (Short*)state->stack;
    *destination = GetShortArg(state);
    state->stack -= sizeof(Short);
}


void PassToPutS(struct State* state) {
    char* ptr = (char*)(PopShortFromStack(state)+state->memory);
    puts(ptr);
}

void InitState(StatePtr state, MemPtr memory, int sizeOfMemory) {
    state->stack = memory+sizeOfMemory;
    state->memory = memory;
    state->PC = memory;
    state->args = memory;
}

void NextInstruction(StatePtr state) {
    struct APICall *apiCall = &ApiCallsList[*state->PC];
    state->args = state->PC + 1;
    state->PC += 1 + apiCall->opsSize;
    apiCall->call(state);
}

int IndexOfCall(char* callName) {
    struct APICall *walker = ApiCallsList;
    int i=0;
    for(;;) {
        if (0 == strcmp(callName, walker->name)) {
            return i;
        }
        else {
            if (NULL == (++walker)->call) {
                break;
            }
            else {
                i+=1;
            }
        }
    };
    
    puts("Unknown api call ");
    puts(callName);
    puts("\n");
    exit(1);

}

void PutCall(CompilationStatePtr compilationState, char* callName) {
    int apiCallIndex = IndexOfCall(callName);
    compilationState->argsToPush += ApiCallsList[apiCallIndex].opsSize;
    *(compilationState->PC++) = apiCallIndex;
}

void PutShort(CompilationStatePtr compilationState, Short value) {
    Short* dest = (Short*)compilationState->PC;
    *dest = value;
    compilationState->PC += sizeof(Short);
    compilationState->argsToPush -= sizeof(Short);
}

void InitCompilationState(CompilationStatePtr compilationState, MemPtr memory, int size) {
    compilationState->argsToPush = 0;
    compilationState->memory = memory;
    compilationState->PC = memory;
    compilationState->endOfMemory = memory+size;
}

int main(int argc, const char * argv[]) {
    struct State state;
    struct CompilationState compilationState;
    byte memory[1024];

    char* string = "HelloWorld\n";
    long len = strlen(string);
    memcpy(memory+32, string, len);
    
    InitState(&state, memory, sizeof(memory));
    InitCompilationState(&compilationState, memory, sizeof(memory));

    PutCall(&compilationState, "PushShortToStack");
    PutShort(&compilationState, 32);
    NextInstruction(&state);

    
    PutCall(&compilationState, "PassToPutS");
    NextInstruction(&state);
    
    return 0;
}
