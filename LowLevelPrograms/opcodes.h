//
//  opcodes.h
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 18.09.2015.
//  Copyright (c) 2015 macvek. All rights reserved.
//

#ifndef LowLevelPrograms_opcodes_h
#define LowLevelPrograms_opcodes_h

#include "types.h"

void InitState(StatePtr state, MemPtr memory, int sizeOfMemory);
void NextInstruction(StatePtr state);
void AssertStackTopByte(struct State* state, Byte expected);

void InitCompilationState(CompilationStatePtr compilationState, MemPtr memory, int size);
void PutCall(CompilationStatePtr compilationState, char* callName);
void PutByte(CompilationStatePtr compilationState, Byte value);
void PutShort(CompilationStatePtr compilationState, Short value);
int CodeSizeForInstruction(char* instruction);

void AssertStackTopShort(struct State* state, Short expected);

#endif
