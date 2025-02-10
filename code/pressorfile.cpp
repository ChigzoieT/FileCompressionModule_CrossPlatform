#include "pressorfile.h"
#include <stdlib.h>
#include <string.h>
#include <lzma.h>

// Compress function: Compress a file and embed its extension
void compressFileWithExtension(const char* inputFilePath, const char* outputFilePath, int numThreads) {
    const char* extPos = strrchr(inputFilePath, '.');
    if (extPos == NULL) {
        // File has no extension to embed
        return;
    }
    const char* extension = extPos + 1;

    FILE* inputFile = fopen(inputFilePath, "rb");
    if (inputFile == NULL) {
        // Failed to open input file
        return;
    }

    fseek(inputFile, 0, SEEK_END);
    long fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    char* inputBuffer = (char*)malloc(fileSize);
    if (inputBuffer == NULL) {
        // Memory allocation failed
        fclose(inputFile);
        return;
    }
    fread(inputBuffer, 1, fileSize, inputFile);
    fclose(inputFile);

    lzma_stream strm = LZMA_STREAM_INIT;

    // Configure multi-threaded compression
    lzma_mt mt = {0};
    mt.threads = numThreads;
    mt.block_size = 0; // Automatic block size
    mt.timeout = 300; // Default timeout in milliseconds
    mt.check = LZMA_CHECK_CRC32;

    lzma_ret ret = lzma_stream_encoder_mt(&strm, &mt);
    if (ret != LZMA_OK) {
        // lzma_stream_encoder_mt failed
        free(inputBuffer);
        return;
    }

    FILE* outputFile = fopen(outputFilePath, "wb");
    if (outputFile == NULL) {
        // Failed to open output file
        lzma_end(&strm);
        free(inputBuffer);
        return;
    }

    uint8_t extLength = (uint8_t)strlen(extension);
    fwrite(&extLength, sizeof(extLength), 1, outputFile);
    fwrite(extension, 1, extLength, outputFile);

    strm.next_in = (const uint8_t*)inputBuffer;
    strm.avail_in = fileSize;
    uint8_t outputBuffer[1024];
    strm.next_out = outputBuffer;
    strm.avail_out = sizeof(outputBuffer);

    while (strm.avail_in > 0) {
        ret = lzma_code(&strm, LZMA_RUN);
        if (ret != LZMA_OK) {
            // Compression failed
            break;
        }
        if (strm.avail_out == 0) {
            fwrite(outputBuffer, 1, sizeof(outputBuffer), outputFile);
            strm.next_out = outputBuffer;
            strm.avail_out = sizeof(outputBuffer);
        }
    }

    while (ret != LZMA_STREAM_END) {
        ret = lzma_code(&strm, LZMA_FINISH);
        if (ret != LZMA_OK && ret != LZMA_STREAM_END) {
            // Compression failed during finish
            break;
        }
        if (strm.avail_out == 0) {
            fwrite(outputBuffer, 1, sizeof(outputBuffer), outputFile);
            strm.next_out = outputBuffer;
            strm.avail_out = sizeof(outputBuffer);
        }
    }

    if (strm.avail_out < sizeof(outputBuffer)) {
        fwrite(outputBuffer, 1, sizeof(outputBuffer) - strm.avail_out, outputFile);
    }

    fclose(outputFile);
    lzma_end(&strm);
    free(inputBuffer);

    // File compressed with embedded extension
}
