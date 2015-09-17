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

typedef char Byte;
typedef unsigned char Mem;
typedef short Short;
typedef unsigned short MemShort;
typedef Mem* MemPtr;

typedef struct State {
    MemPtr stackPtr;
    MemPtr memory;
    MemPtr endOfProgram;
    MemPtr args;
    MemPtr PC;
    MemShort stackFrame;
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
void Call0(StatePtr);
void CallArg1S(StatePtr);
void MethodEnter1B(StatePtr);
void PushByteToStack1B(StatePtr state);
void MethodExit0(StatePtr state);
void MethodReturn1B(StatePtr state);
void StoreByte1B(StatePtr state);
void StoreShort1B(StatePtr state);
void LoadByte1B(StatePtr state);
void LoadShort1B(StatePtr state);
void Jump1S(StatePtr state);
void JumpIf1S(StatePtr state);
void CompareBytes0(StatePtr state);
void CompareShorts0(StatePtr state);
void Inc0(StatePtr state);
void Dec0(StatePtr state);
void Add0(StatePtr state);
void Sub0(StatePtr state);
void Mul0(StatePtr state);
void Div0(StatePtr state);
void DupShort0(StatePtr state);
void DupByte0(StatePtr state);
void SetupArgsStack0(StatePtr state);
void SetupFrameStack0(StatePtr state);

void PushByteToStack(StatePtr state, Byte value);
MemPtr GetFramePtr(StatePtr state);

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
    {"MethodEnter", &MethodEnter1B, sizeof(Byte)},
    {"Call", &Call0, 0},
    {"CallArg", &CallArg1S, sizeof(Short)},
    {"MethodExit", &MethodExit0, 0},
    {"MethodReturn", &MethodReturn1B, sizeof(Byte)},
    {"StoreByte", &StoreByte1B, sizeof(Byte)},
    {"StoreShort", &StoreShort1B, sizeof(Byte)},
    {"LoadByte", &LoadByte1B, sizeof(Byte)},
    {"LoadShort", &LoadShort1B, sizeof(Byte)},
    {"Jump", &Jump1S, sizeof(Short)},
    {"JumpIf", &JumpIf1S, sizeof(Short)},
    {"CompareBytes", &CompareBytes0, 0},
    {"CompareShorts", &CompareShorts0, 0},
    {"Inc", &Inc0, 0},
    {"Dec", &Dec0, 0},
    {"Add", &Add0, 0},
    {"Sub", &Sub0, 0},
    {"Mul", &Mul0, 0},
    {"Div", &Div0, 0},
    {"DupShort", &DupShort0, 0},
    {"DupByte", &DupByte0, 0},
    {"SetupArgsStack", &SetupArgsStack0, 0},
    {"SetupFrameStack", &SetupFrameStack0, 0},
    {NULL,NULL,-1}
};

#define SIZEOF_LOCALALLOCSIZE sizeof(Byte)
#define SIZEOF_CALLPAYLOAD sizeof(Short)+sizeof(Short)+sizeof(Short)


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
    state->PC += GetShortArg(state);
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

void PushStackByNZeroBytes(StatePtr state, Byte nBytes) {
    state->stackPtr -= nBytes;
    memset(state->stackPtr, 0, nBytes);
}

void PopStackByNBytes(StatePtr state, Byte nBytes) {
    state->stackPtr += nBytes;
}

Short MemOffsetPC(StatePtr state) {
    return (Short)(state->PC - state->memory);
}

Short MemOffsetStackPtr(StatePtr state) {
    return (Short)(state->stackPtr - state->memory);
}

void SetMemOffsetPC(StatePtr state, Short pcOffset) {
    state->PC = (MemPtr)(state->memory + pcOffset);
}

void SetStackPtrFromMemOffset(StatePtr state, Short stackPtrOffset) {
    state->stackPtr = (MemPtr)(state->memory + stackPtrOffset);
}

void Call(StatePtr state, Short newPC) {
    PushShortToStack(state, MemOffsetStackPtr(state));
    PushShortToStack(state, state->stackFrame);
    PushShortToStack(state, MemOffsetPC(state));
    SetMemOffsetPC(state, newPC);
}

void Call0(StatePtr state) {
    Call(state, PopShortFromStack(state));
}

void CallArg1S(StatePtr state) {
    Call(state, GetShortArg(state));
}

Byte GetLocalAllocSize(StatePtr state) {
    return *(Byte*)GetFramePtr(state);
}

MemPtr GetStoredCallerStackPtr(StatePtr state) {
    return GetFramePtr(state) + GetLocalAllocSize(state) + SIZEOF_CALLPAYLOAD + SIZEOF_LOCALALLOCSIZE;
}

void SetupArgsStack0(StatePtr state) {
    state->stackPtr = GetStoredCallerStackPtr(state);
}

void SetupFrameStack0(StatePtr state) {
    //*(Short*)GetStoredCallerStackPtr(state) = MemOffsetStackPtr(state);
    state->stackPtr = GetFramePtr(state);
}

void MethodEnter1B(StatePtr state) {
    Byte reservedSize = GetByteArg(state);

    PushStackByNZeroBytes(state, reservedSize);
    PushByteToStack(state, reservedSize);
    
    state->stackFrame = state->stackPtr - state->memory;
}

void ReleaseStack(StatePtr state) {
    PopStackByNBytes(state, PopByteFromStack(state));
}

void RestoreFromCall(StatePtr state) {
    SetMemOffsetPC(state, PopShortFromStack(state));
    state->stackFrame = PopShortFromStack(state);
    SetStackPtrFromMemOffset(state, PopShortFromStack(state));
}


void MethodExit(StatePtr state) {
    ReleaseStack(state);
    RestoreFromCall(state);
}

void MethodExit0(StatePtr state) {
    MethodExit(state);
}

void MethodReturn1B(StatePtr state) {
    Mem returnStackSize = (Mem)GetByteArg(state);
    MemPtr methodStackPtr = state->stackPtr;
    state->stackPtr += returnStackSize;
    MethodExit(state);
    PushStackByNZeroBytes(state, returnStackSize);
    memcpy(state->stackPtr, methodStackPtr, returnStackSize);
}

void Inc0(StatePtr state) {
    PushShortToStack(state, 1 + PopShortFromStack(state));
}

void Dec0(StatePtr state) {
    PushShortToStack(state, PopShortFromStack(state) - 1);
}

void Add0(StatePtr state) {
    PushShortToStack(state, PopShortFromStack(state) + PopShortFromStack(state));
}

void Sub0(StatePtr state) {
    Short b = PopShortFromStack(state);
    Short a = PopShortFromStack(state);

    PushShortToStack(state, a - b);
}

void Mul0(StatePtr state) {
    PushShortToStack(state, PopShortFromStack(state) * PopShortFromStack(state));
}

void Div0(StatePtr state) {
    Short b = PopShortFromStack(state);
    Short a = PopShortFromStack(state);
    PushShortToStack(state, a / b);
}

void DupShort0(StatePtr state) {
    PushShortToStack(state, *state->stackPtr);
}

void DupByte0(StatePtr state) {
    PushByteToStack(state, *state->stackPtr);
}

MemPtr GetFramePtr(StatePtr state) {
    return state->memory + state->stackFrame;
}

Short* GetLocalAllocPtr(StatePtr state, int index) {
    return (Short*)(GetFramePtr(state) + SIZEOF_LOCALALLOCSIZE +  index * sizeof(Short));
}

void StoreByte1B(StatePtr state) {
    *GetLocalAllocPtr(state, GetByteArg(state)) = PopByteFromStack(state);
}

void StoreShort1B(StatePtr state) {
    *GetLocalAllocPtr(state, GetByteArg(state)) = PopShortFromStack(state);
}

void LoadByte1B(StatePtr state) {
    PushByteToStack(state, *GetLocalAllocPtr(state, GetByteArg(state)));
}

void LoadShort1B(StatePtr state) {
    PushShortToStack(state, *GetLocalAllocPtr(state, GetByteArg(state)));
}

void PassToPutS0(struct State* state) {
    char* ptr = (char*)(PopShortFromStack(state)+state->memory);
    puts(ptr);
}

void AssertStackTopShort(struct State* state, Short expected) {
    Short topStack = *(Short*)state->stackPtr;
    if (expected != topStack) {
        printf("AssertStackTopShort failed, got %i expected %i\n",topStack, expected);
        exit(2);
    }
}

void AssertStackTopByte(struct State* state, Byte expected) {
    Byte topStack = *(Byte*)state->stackPtr;
    if (expected != topStack) {
        printf("AssertStackTopByte failed, got %i expected %i\n",topStack, expected);
        exit(2);
    }
}

void NextInstruction(StatePtr state) {
    struct APICall *apiCall = &ApiCallsList[*state->PC];
    printf("::NextInstruction:: %s\n", apiCall->name);
    state->PC += 1;
    state->args = state->PC;
    state->PC += apiCall->opsSize;
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

void AdvanceArgsByBytes(CompilationStatePtr compilationState, int bytes) {
    compilationState->PC += bytes;
    compilationState->argsToPush -= bytes;
}

void PutCall(CompilationStatePtr compilationState, char* callName) {
    int apiCallIndex = IndexOfCall(callName);
    compilationState->argsToPush += ApiCallsList[apiCallIndex].opsSize;
    *(compilationState->PC++) = apiCallIndex;
}

void PutShort(CompilationStatePtr compilationState, Short value) {
    Short* dest = (Short*)compilationState->PC;
    *dest = value;
    AdvanceArgsByBytes(compilationState, sizeof(Short));
}

void PutByte(CompilationStatePtr compilationState, Byte value) {
    Byte* dest = (Byte*)compilationState->PC;
    *dest = value;
    AdvanceArgsByBytes(compilationState, sizeof(Byte));
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
    
    printf("Application size: %d\n", (int)(compilationState.PC - compilationState.memory));
    
    return 0;
}
