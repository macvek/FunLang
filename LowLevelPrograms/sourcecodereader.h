//
//  sourcecodereader.h
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 04.10.2015.
//  Copyright © 2015 macvek. All rights reserved.
//

#ifndef sourcecodereader_h
#define sourcecodereader_h

#include <stdio.h>
#include "types.h"

struct Reader {
    char* cursor;
    char* cursorLimit;
};

struct StringRange {
    char* start;
    char* end;
};

void SkipWhiteSpaces(struct Reader* reader);
void InitReader(struct Reader* reader, char* sourceCode, int limit);
void InitReaderCopy(struct Reader* destination, const struct Reader* source);
bool IsNotEmptyReader(struct Reader* reader);
bool CompareStringRanges(struct StringRange* one, struct StringRange *another);
bool CompareStringRangeWithString(struct StringRange* one, char* another);
void PrintSourceCodeWithMarker(char* sourceCode, char* pointer);
bool MatchTillNextSpace(struct Reader* reader, const char* source);
bool MatchAndAdvanceTillNextSpace(struct Reader* reader, const char* source);
char* NextSpaceInReader(struct Reader* reader);
char* NextCharInReader(struct Reader* reader, char character);
bool AdvanceReaderIfStartsWithChar(struct Reader* reader, char character);


#endif /* sourcecodereader_h */
