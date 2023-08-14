#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>
#include "text.h"

bool parseInt64(char *s, int64_t *result)
{
    return sscanf(s, "%" PRId64 "", result) == 1;
}

void halt(char *errorMessage)
{
    fprintf(stderr, "%s\n", errorMessage);
    exit(-1);
}

int main(int argc, char *argv[])
{
    if (argc < 4)
        halt("too few args");
    
    bool isStart = strcmp(argv[1], "start") == 0;
    bool isEnd = strcmp(argv[1], "end") == 0;
    bool isStartEnd = strcmp(argv[1], "start-end") == 0;

    if (!isStart && !isEnd && !isStartEnd)
        halt("must select one of the three modes <start/end/start-end>");

    if (isStartEnd && argc < 5)
        halt("too few args");

    int capArg = isStartEnd ? 5 : 4;
    bool hasCap = strcmp(argv[capArg - 1], "cap") == 0;
    int inFileArg = 1 + (isStartEnd ? 3 : 2) + (hasCap ? 2 : 0);
    int outFileArg = inFileArg + 2;

    if (hasCap && argc < inFileArg + 1)
        halt("too few args");
    
    bool hasOutFile = argc > inFileArg + 1;
    bool isTo = false;
    bool isAppendTo = false;
    if (hasOutFile)
    {
        isTo = strcmp(argv[inFileArg + 1], "to") == 0;
        isAppendTo = strcmp(argv[inFileArg + 1], "append-to") == 0;

        if (!isTo && !isAppendTo)
            halt("did you give too many args? must select one of the two modes <to/append-to> when using output-file");
    }

    int expectedNumbArgs = 1 + (isStartEnd ? 3 : 2) + (hasCap ? 2 : 0) + 1 + (hasOutFile ? 2 : 0);

    if (argc < expectedNumbArgs)
        halt("too few args");
    
    if (argc > expectedNumbArgs)
        halt("too many args");

    int64_t start = 0;
    int64_t end = 0;
    int64_t cap = 0;

    if (isStart)
    {
        if (!parseInt64(argv[2], &start))
            halt("parse failed [start]");
    }
    if (isEnd)
    {
        if (!parseInt64(argv[2], &end))
            halt("parse failed [end]");
    }
    if (isStartEnd)
    {
        if (!parseInt64(argv[2], &start))
            halt("parse failed [start]");
        
        if (!parseInt64(argv[3], &end))
            halt("parse failed [end]");
    }

    if (hasCap)
    {
        if (!parseInt64(argv[capArg], &cap))
            halt("parse failed [cap]");
    }

    char *inFilePath = strdup(argv[inFileArg]);
    int expandResult = expandEVs(&inFilePath, 3000, 40);
    if (expandResult == -1)
    {
        free(inFilePath);
        halt("enviroment variable expand failed [input file] (too large)");
    }
    if (expandResult == -2)
    {
        free(inFilePath);
        halt("enviroment variable expand failed [input file] (unknown enviroment variable)");
    }
    if (expandResult == -3)
    {
        free(inFilePath);
        halt("enviroment variable expand failed [input file] (too many expands)");
    }
    FILE *f = fopen(inFilePath, "rb");
    free(inFilePath);

    if (f == NULL)
        halt("could not open input file");
    
    FILE *f2;
    if (hasOutFile)
    {
        char *outFilePath = strdup(argv[outFileArg]);
        expandResult = expandEVs(&outFilePath, 3000, 40);
        if (expandResult == -1)
        {
            fclose(f);
            free(outFilePath);
            halt("enviroment variable expand failed [output file] (too large)");
        }
        if (expandResult == -2)
        {
            fclose(f);
            free(outFilePath);
            halt("enviroment variable expand failed [output file] (unknown enviroment variable)");
        }
        if (expandResult == -3)
        {
            fclose(f);
            free(outFilePath);
            halt("enviroment variable expand failed [output file] (too many expands)");
        }
        f2 = fopen(outFilePath, isTo ? "wb" : "ab");
        free(outFilePath);

        if (f2 == NULL)
        {
            fclose(f);
            halt("could not open output file");
        }
    }

    struct __stat64 s;
    _fstat64(fileno(f), &s);
    int64_t fileSize = s.st_size;

    int64_t seekTo = 0;
    int64_t outputSize = 0;

    if (isStart)
    {
        seekTo = start;
        outputSize = fileSize - seekTo;
        if (hasCap && cap < outputSize)
        {
            outputSize = cap;
        }
    }
    if (isStartEnd)
    {
        seekTo = start;
        outputSize = fileSize - end - seekTo;
        if (hasCap && cap < outputSize)
        {
            outputSize = cap;
        }
    }
    if (isEnd)
    {
        if (hasCap && cap < fileSize - end)
        {
            seekTo = fileSize - end - cap;
            outputSize = cap;
        }
        else
        {
            seekTo = 0;
            outputSize = fileSize - end;
        }
    }

    if (seekTo < 0 || seekTo + outputSize > fileSize)
    {
        fprintf(stderr, "target file: %" PRId64 " (%" PRId64 " - %" PRId64 ")\n", fileSize, seekTo, seekTo + outputSize);
        fclose(f);
        if (hasOutFile)
        {
            fclose(f2);
        }
        halt("read range selected is out of bounds");
    }

    if (outputSize <= 0)
    {
        fprintf(stderr, "target file: %" PRId64 " (%" PRId64 " - %" PRId64 ")\n", fileSize, seekTo, seekTo + outputSize);
        fclose(f);
        if (hasOutFile)
        {
            fclose(f2);
        }
        halt("read range selected is zero or negative width");
    }

    _fseeki64(f, seekTo, SEEK_SET);

    int bufSize = 4000;
    char buffer[bufSize + 1];
    buffer[bufSize] = '\0';
    int64_t remaining = outputSize;
    int readSize;

    while (remaining > 0)
    {
        readSize = (int)(remaining < bufSize ? remaining : bufSize);
        if (fread(buffer, sizeof(char), readSize, f) != (size_t)readSize)
        {
            fclose(f);
            if (hasOutFile)
            {
                fclose(f2);
            }
            halt("read failed or premature EOF was encountered");
        }
        
        buffer[readSize] = '\0';
        remaining -= readSize;

        if (hasOutFile)
        {
            if (fwrite(buffer, sizeof(char), readSize, f2) != (size_t)readSize)
            {
                fclose(f);
                if (hasOutFile)
                {
                    fclose(f2);
                }
                halt("write failed");
            }
        }
        else
        {
            printf("%s", buffer);
        }
    }
    
    fclose(f);
    if (hasOutFile)
    {
        fclose(f2);
    }
}
