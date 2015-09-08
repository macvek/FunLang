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
typedef unsigned short memptr;

struct State {
    byte *stack;
    byte *memory;
};

memptr PopPtrFromStack(struct State *state) {
    state->stack += sizeof(memptr);
    memptr value = *(memptr*)state->stack;
    return value;
}

void PushPtrToStack(struct State *state, memptr ptr) {
    if (state->stack < state->memory) {
        exit(1);
    }
    memptr *destination = (memptr*)state->stack;
    *destination = ptr;
    state->stack -= sizeof(memptr);
}

void PassToPutS(struct State* state) {
    char* ptr = (char*)(PopPtrFromStack(state)+state->memory);
    puts(ptr);
}


int main(int argc, const char * argv[]) {
    struct State state;
    byte memory[1024];
    state.stack = memory+sizeof(memory);
    state.memory = memory;
    char* string = "HelloWorld\n";
    int len = strlen(string);
    memcpy(memory, string, len);
    
    PushPtrToStack(&state, 0);
    PassToPutS(&state);
    
    return 0;
}
