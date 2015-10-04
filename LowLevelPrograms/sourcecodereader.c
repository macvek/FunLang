//
//  sourcecodereader.c
//  LowLevelPrograms
//
//  Created by Maciej Aleksandrowicz on 04.10.2015.
//  Copyright © 2015 macvek. All rights reserved.
//

#include "sourcecodereader.h"
#include <string.h>

void SkipWhiteSpaces(struct Reader* reader) {
    
    while(reader->cursor < reader->cursorLimit &&
          (
           *reader->cursor == ' ' ||
           *reader->cursor == '\n' ||
           *reader->cursor == '\r' ||
           *reader->cursor == '\t'
           ) ) {
              reader->cursor++;
          }
}

bool IsNotEmptyReader(struct Reader* reader) {
    return reader->cursor < reader->cursorLimit;
}

void InitReader(struct Reader* reader, char* sourceCode, int limit) {
    reader->cursor = sourceCode;
    reader->cursorLimit = sourceCode+limit;
}

void InitReaderCopy(struct Reader* destination, const struct Reader* source) {
    destination->cursor = source->cursor;
    destination->cursorLimit = source->cursorLimit;
}


bool CompareStringRanges(struct StringRange* one, struct StringRange *another) {
    if (one->end - one->start == another->end - another->start) {
        return 0 == memcmp(one->start, another->start, one->end - one->start);
    }
    else {
        return false;
    }
}

bool CompareStringRangeWithString(struct StringRange* one, char* another) {
    long anotherLength = strlen(another);
    if (one->end - one->start == anotherLength) {
        return 0 == memcmp(one->start, another, one->end - one->start);
    }
    else {
        return false;
    }
}

void PrintSourceCodeWithMarker(char* sourceCode, char* pointer) {
    if (*sourceCode == 0) {
        return;
    }
    char* lastLine = sourceCode;
    for(;;) {
        if (*sourceCode == '\n' || *sourceCode == 0) {
            long len = sourceCode-lastLine;
            char lineToPrint[len+1];
            memcpy(lineToPrint, lastLine, len);
            lineToPrint[len] = 0;
            printf("%s",lineToPrint);
            
            if (pointer < sourceCode) {
                printf("\n");
                long len = pointer-(lastLine+1);
                for (int i=0;i<len;i++) {
                    printf("%c", '-');
                }
                printf("%c\n",'^');
                return;
            }
            else {
                lastLine = sourceCode;
            }
        }
        sourceCode++;
    }
    
}

void PrintRange(char* prefix, struct StringRange* range) {
    long size = range->end - range->start;
    char text[size];
    
    memcpy(text, range->start, size);
    text[size] = 0;
    printf("%s: %s\n", prefix, text);
}

bool MatchAndAdvanceTillNextSpace(struct Reader* reader, const char* source) {
    if (MatchTillNextSpace(reader, source)) {
        reader->cursor += strlen(source);
        return true;
    }
    else {
        return false;
    }
}

bool MatchTillNextSpace(struct Reader* reader, const char* source) {
    char* nextSpace = NextSpaceInReader(reader);
    if (nextSpace == NULL) {
        return false;
    }
    
    int length = (int)(nextSpace - reader->cursor);
    
    return 0 == strncmp(reader->cursor, source, length);
}

char* NextCharInReader(struct Reader* reader, char character) {
    char seek[2];
    seek[0] = character;
    seek[1] = 0;
    if (reader->cursorLimit <= reader->cursor) {
        return NULL;
    }
    char* nextSpace = strnstr(reader->cursor, seek, reader->cursorLimit - reader->cursor);
    return nextSpace < reader->cursorLimit ? nextSpace : NULL;
}


char* NextSpaceInReader(struct Reader* reader) {
    return NextCharInReader(reader, ' ');
}

bool AdvanceReaderIfStartsWithChar(struct Reader* reader, char character) {
    if (reader->cursor < reader->cursorLimit && *reader->cursor == character) {
        reader->cursor++;
        return true;
    }
    else {
        return false;
    }
}
