#ifndef PRESSORFILE_H
#define PRESSORFILE_H

#include <stdio.h>
#include <stdint.h>
#include <lzma.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function prototype for compressing a file with its extension embedded, now including numThreads parameter
void compressFileWithExtension(const char* inputFilePath, const char* outputFilePath, int numThreads);

#ifdef __cplusplus
}
#endif

#endif // PRESSORFILE_H
