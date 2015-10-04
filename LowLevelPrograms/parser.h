//
//  parser.h
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 30.09.2015.
//  Copyright © 2015 macvek. All rights reserved.
//

#ifndef parser_h
#define parser_h

#include <stdio.h>
#include "types.h"

struct CodeUnit;
struct MethodUnit;
struct ConstUnit;
struct SymbolUnit;
struct ExpressionUnit;
struct AssignmentUnit;

union LinkedUnit {
    struct MethodUnit *methodUnit;
    struct ConstUnit *constUnit;
    struct SymbolUnit *symbolUnit;
    struct ExpressionUnit *expressionUnit;
};

union ExpressionValue {
    struct AssignmentUnit *assignment;
};

struct NextPrevUnit {
    union LinkedUnit next;
};

enum UnitType {
    METHOD, CONST, SYMBOL
};

enum ExpressionType {
    ASSIGNMENT
};

struct CodeUnit {
    struct MethodUnit *linkedMethods;
    struct ConstUnit *linkedConsts;
    struct SymbolUnit *linkedSymbols;
    struct ExpressionUnit* linkedExpressions;
};

struct AssignmentUnit {
    char* leftValue;
    char* rightValue;
};

struct MethodUnit {
    struct NextPrevUnit siblings;
    Addr address;
    Short size;
    char *name;
    struct ConstUnit *linkedLocalConsts;
};

struct ConstUnit {
    struct NextPrevUnit siblings;
    char *name;
    char *value;
    Addr address;
    Short size;
};

struct SymbolUnit {
    char *name;
    enum UnitType type;
    union LinkedUnit *linkedUnit;
};

struct ExpressionUnit {
    struct NextPrevUnit siblings;
    enum ExpressionType type;
    union ExpressionValue *value;
};

void FreeCodeUnit(struct CodeUnit *codeUnit);
struct CodeUnit *ParseSourceCode(char* sourceCode);

#endif /* parser_h */
