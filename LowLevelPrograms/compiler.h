//
//  compiler.h
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 21.09.2015.
//  Copyright (c) 2015 macvek. All rights reserved.
//

#ifndef __LowLevelPrograms__compiler__
#define __LowLevelPrograms__compiler__

#include "opcodes.h"
void CompileCode(char* sourceCode, int sourceCodeSize, CompilationStatePtr compilationState);

#endif /* defined(__LowLevelPrograms__compiler__) */
