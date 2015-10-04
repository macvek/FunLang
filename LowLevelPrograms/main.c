//
//  main.c
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 08.09.2015.
//  Copyright (c) 2015 macvek. All rights reserved.
//

#include "opcodestest.h"
#include "compilertest.h"
#include "parsertest.h"


int main(int argc, const char * argv[]) {
    OpcodesTest();
    CompilerTest();
    ParserTest();
    return 0;
}
