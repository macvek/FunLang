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

typedef unsigned char Byte;
typedef unsigned short Short;
typedef short SignedShort;
typedef Byte* MemPtr;

typedef struct State {
    MemPtr stackPtr;
    MemPtr memory;
    MemPtr endOfProgram;
    MemPtr args;
    MemPtr PC;
    Short stackFrame;
} *StatePtr ;

typedef struct CompilationState {
    MemPtr memory;
    MemPtr PC;
    int argsToPush;
    MemPtr endOfMemory;
} *CompilationStatePtr;

typedef void (*apicall)(StatePtr);
void ZeroOpcodeFail0(StatePtr);
void PassToPutS0(StatePtr);
void PushShortToStack1S(StatePtr);
void ReserveStack1B(StatePtr);
void PushByteToStack1B(StatePtr state);
void ReleaseStack0(StatePtr state);
void StoreByte1B(StatePtr state);
void StoreShort1B(StatePtr state);
void LoadByte1B(StatePtr state);
void LoadShort1B(StatePtr state);
void Jump1S(StatePtr state);
void JumpIf1S(StatePtr state);
void CompareBytes0(StatePtr state);
void CompareShorts0(StatePtr state);
void IncShort0(StatePtr state);
void DecShort0(StatePtr state);

void PushByteToStack(StatePtr state, Byte value);

struct APICall {
    char* name;
    apicall call;
    int opsSize;
};

struct APICall ApiCallsList[] = {
    {"ZeroOpcodeFail", &ZeroOpcodeFail0, 0},
    {"PassToPutS", &PassToPutS0, 0},
    {"PushShortToStack", &PushShortToStack1S, sizeof(Short)},
    {"PushByteToStack", &PushByteToStack1B, sizeof(Byte)},
    {"ReserveStack", &ReserveStack1B, sizeof(Byte)},
    {"ReleaseStack", &ReleaseStack0, 0},
    {"StoreByte", &StoreByte1B, sizeof(Byte)},
    {"StoreShort", &StoreShort1B, sizeof(Byte)},
    {"LoadByte", &LoadByte1B, sizeof(Byte)},
    {"LoadShort", &LoadShort1B, sizeof(Byte)},
    {"Jump", &Jump1S, sizeof(Short)},
    {"JumpIf", &JumpIf1S, sizeof(Short)},
    {"CompareBytes", &CompareBytes0, 0},
    {"CompareShorts", &CompareShorts0, 0},
    {"IncShort", &IncShort0, 0},
    {"DecShort", &DecShort0, 0},
    {NULL,NULL,-1}
};

Short PopShortFromStack(StatePtr state) {
    Short value = *(Short*)state->stackPtr;
    state->stackPtr += sizeof(Short);
    return value;
}

Byte PopByteFromStack(StatePtr state) {
    Byte value = *(Byte*)state->stackPtr;
    state->stackPtr += sizeof(Byte);
    return value;
}

Short GetShortArg(StatePtr state) {
    Short value = *((Short*)state->args);
    state->args += sizeof(Short);
    return value;
}

Byte GetByteArg(StatePtr state) {
    Byte value = *((Byte*)state->args);
    state->args += sizeof(Byte);
    return value;
}

void StackBoundaryCheck(StatePtr state, int size) {
    if (state->stackPtr - size <= state->memory) {
        puts("virtual stack overflow\n");
        exit(1);
    }
}

void PushShortToStack(StatePtr state, Short value) {
    StackBoundaryCheck(state, sizeof(Short));
    state->stackPtr -= sizeof(Short);
    Short *destination = (Short*)state->stackPtr;
    *destination = value;
}

void ZeroOpcodeFail0(StatePtr state) {
    puts("ZeroOpcodeFail triggered");
    exit(1);
}

void Jump1S(StatePtr state) {
    state->PC += (SignedShort)GetShortArg(state);
}

void JumpIf1S(StatePtr state) {
    if (PopByteFromStack(state) == 1) {
        Jump1S(state);
    }
}

void PushShortToStack1S(StatePtr state) {
    PushShortToStack(state, GetShortArg(state));
}

void CompareBytes0(StatePtr state) {
    PushByteToStack(state, PopByteFromStack(state) == PopByteFromStack(state));
}

void CompareShorts0(StatePtr state) {
    PushByteToStack(state, PopShortFromStack(state) == PopShortFromStack(state));
}

void PushByteToStack(StatePtr state, Byte value) {
    StackBoundaryCheck(state, sizeof(Byte));
    state->stackPtr -= sizeof(Byte);
    Byte *destination = (Byte*)state->stackPtr;
    *destination = value;
}

void PushByteToStack1B(StatePtr state) {
    PushByteToStack(state, GetByteArg(state));
}

void PushStackByNBytes(StatePtr state, Byte nBytes) {
    state->stackPtr -= nBytes;
}

void PopStackByNBytes(StatePtr state, Byte nBytes) {
    state->stackPtr += nBytes;
}

void ReserveStack1B(StatePtr state) {
    PushShortToStack(state, state->stackFrame);
    state->stackFrame = state->stackPtr - state->memory;
    Byte reservedSize = GetByteArg(state);
   
    PushStackByNBytes(state, reservedSize*sizeof(Short));
    PushByteToStack(state, reservedSize);
}

void ReleaseStack0(StatePtr state) {
    Byte reservedSize = PopByteFromStack(state);
    PopStackByNBytes(state, reservedSize * sizeof(Short));
    Short parentFrame = PopShortFromStack(state);
    state->stackFrame = parentFrame;
}

void IncByte0(StatePtr state) {
    PushByteToStack(state, 1 + PopByteFromStack(state));
}

void IncShort0(StatePtr state) {
    PushShortToStack(state, 1 + PopShortFromStack(state));
}

void DecByte0(StatePtr state) {
    PushByteToStack(state, PopByteFromStack(state) - 1);
}
void DecShort0(StatePtr state) {
    PushShortToStack(state, PopShortFromStack(state) - 1);
}

Short* GetFramePtr(StatePtr state, int index) {
    return (Short*)(state->memory + state->stackFrame +  index * sizeof(Short));
}

void StoreByte1B(StatePtr state) {
    *GetFramePtr(state, GetByteArg(state)) = PopByteFromStack(state);
}

void StoreShort1B(StatePtr state) {
    *GetFramePtr(state, GetByteArg(state)) = PopShortFromStack(state);
}

void LoadByte1B(StatePtr state) {
    PushByteToStack(state, *GetFramePtr(state, GetByteArg(state)));
}

void LoadShort1B(StatePtr state) {
    PushShortToStack(state, *GetFramePtr(state, GetByteArg(state)));
}

void PassToPutS0(struct State* state) {
    char* ptr = (char*)(PopShortFromStack(state)+state->memory);
    puts(ptr);
}

void NextInstruction(StatePtr state) {
    struct APICall *apiCall = &ApiCallsList[*state->PC];
    printf("::NextInstruction:: %s\n", apiCall->name);
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

void PutByte(CompilationStatePtr compilationState, Byte value) {
    Byte* dest = (Byte*)compilationState->PC;
    *dest = value;
    compilationState->PC += sizeof(Byte);
    compilationState->argsToPush -= sizeof(Byte);
}

void InitCompilationState(CompilationStatePtr compilationState, MemPtr memory, int size) {
    compilationState->argsToPush = 0;
    compilationState->memory = memory;
    compilationState->PC = memory;
    compilationState->endOfMemory = memory+size;
}

void InitState(StatePtr state, MemPtr memory, int sizeOfMemory) {
    state->stackPtr = memory+sizeOfMemory;
    state->stackFrame = 0xFFFF;
    state->memory = memory;
    state->PC = memory;
    state->args = memory;
}


int CodeSizeForInstruction(char* instruction) {
    return 1 + ApiCallsList[IndexOfCall(instruction)].opsSize;
}

int main(int argc, const char * argv[]) {
    struct State state;
    struct CompilationState compilationState;
    Byte memory[512];
    memset(memory, 0, sizeof(memory));

    char* string = "HelloWorld\n";
    long len = strlen(string);
    memcpy(memory+256, string, len);
    
    int argNum = 0;
    
    InitState(&state, memory, sizeof(memory));
    InitCompilationState(&compilationState, memory, sizeof(memory));
    
    PutCall(&compilationState, "ReserveStack");
    PutByte(&compilationState, argNum+1);
    NextInstruction(&state);
    
    PutCall(&compilationState, "PushShortToStack");
    PutShort(&compilationState, 256);
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
    PutShort(&compilationState, (SignedShort)(- CodeSizeForInstruction("Jump")*2));
    
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
    PutShort(&compilationState, (SignedShort)(-CodeSizeForInstruction("JumpIf")) );
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
    
    PutCall(&compilationState, "IncShort");
    NextInstruction(&state);
    
    PutCall(&compilationState, "DecShort");
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
    
    PutCall(&compilationState, "ReleaseStack");
    NextInstruction(&state);
    
    return 0;
}
