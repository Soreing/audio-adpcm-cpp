#ifndef ADPCM_H
#define ADPCM_H

#include <adpcm-lib/wave.h>

typedef unsigned char u_char;
typedef short sample;

struct ADPCMHeader
{
    char  chunkID[4] = {'a','d','p',' '};   // Contains the letters "adp " in ASCII form
    long  chunkSize;                        // 16 + (8 + dataSize)

    long channelCount;                      // Number of channels in the audio
    long  sampleCount;                      // Number of samples in the data
    
    sample ch1KeySample;                    // Key Sample for channel 1
    short  ch1StepIndex;                    // Step Table starting index for channel 1
    sample ch2KeySample;                    // Key Sample for channel 2
    short  ch2StepIndex;                    // Step Table starting index for channel 2

    char  dataID[4] = {'d','a','t','a'};    // Contains the letters "data" in ASCII form
    long  dataSize;                         // This is the number of bytes in the data
};

// Compresses a stream 16-bit samples from src and stores the 4-bit values in dst
void compress(const char* src, char* dst, const WAVEHeader &wav, ADPCMHeader &adpcm);

// Decompresses a stream 4-bit values from src and stores the 16-bit samples in dst
void decompress(const char* src, char* dst, const ADPCMHeader &adpcm);

// Returns the bytes required to store the compressed data
int ADPCMDataSize(const WAVEHeader &wav);

#endif