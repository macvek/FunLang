//
//  header.h
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 18.09.2015.
//  Copyright (c) 2015 macvek. All rights reserved.
//

#ifndef LowLevelPrograms_header_h
#define LowLevelPrograms_header_h

typedef char Byte;
typedef unsigned char Mem;
typedef short Short;
typedef unsigned short MemShort;
typedef unsigned short Addr;
typedef Mem* MemPtr;
#define false 0
#define true 1
typedef int bool;

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


#endif
