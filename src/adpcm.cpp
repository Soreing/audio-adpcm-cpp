#include <adpcm-lib/adpcm.h>
#include <string.h>

// Index table that specifies the modification to the index based on
// The 4-bit ADPCM difference in compression or decompression.
const char indexTable[16] = { 
    -1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8 
};

// Quantiziation step size table that specifies how much to change
// The 16-bit PCM sample with based on the 4-bit ADPCM difference.
const int  stepsize[89] = { 
    7,     8,     9,    10,     11,    12,    13,    14,
	16,    17,    19,    21,    23,    25,    28,    31,
	34,    37,    41,    45,    50,    55,	  60,    66,
	73,    80,    88,    97,    107,   118,   130,   143,
	157,   173,   190,   209,   230,   253,   279,   307,
	337,   371,   408,   449,   494,   544,   598,   658,
	724,   796,   876,   963,   1060,  1166,  1282,  1411,
	1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,
	3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,
	7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 
};

// Calculates the difference between the current sample and the last predicted one,
// then constructs a new 4-Bit ADPCM Sample from the difference using the step size.
// A new step size is derived for the next calculation based on the scale of the inaccuracy.
u_char _16Linear_to_4ADPCM(const sample currentSample, sample &prediction, short &index)
{
	u_char newSample = 0;
	int    difference;

	difference = currentSample - prediction;
	if (difference < 0)
	{	difference = -difference;
		newSample = 8;
	}

    int tempDifference = difference;
    int tempStepSize   = stepsize[index];
	for (int mask = 4; mask > 0; mask >>= 1)
	{	if (tempDifference >= tempStepSize)
		{	newSample |= mask;
			tempDifference -= tempStepSize;
	    }	
        tempStepSize >>= 1;
    }

    difference -= tempDifference;
	difference += stepsize[index] >> 3;
	if (newSample & 8)
    {   difference = -difference;
    }

    prediction += difference;
	if      (prediction >  32767) {prediction =  32767;}
    else if (prediction < -32768) {prediction = -32768;}

	index += indexTable[newSample];
	if      (index <  0) {index =  0;}
	else if (index > 88) {index = 88;}

	return newSample;
}

// Attempts to derive the next 16-Bit PCM sample from a 4-bit ADPCM data.
// The difference between the samples are estimated by using the ADPCM with a step size table
// The difference is added to the previous sample, and the step size table index is changed.
sample _4ADPCM_to_16Linear(const u_char adpcm, sample &prediction, short &index)
{
	int difference = 0;
	int newSample = prediction;

	if (adpcm & 4) {difference += stepsize[index]; }
	if (adpcm & 2) {difference += stepsize[index] >> 1;}
	if (adpcm & 1) {difference += stepsize[index] >> 2;}
	difference += stepsize[index] >> 3;

	if (adpcm & 8) 
    {   difference = -difference;
    }

	newSample += difference;
	if      (newSample >  32767) {newSample =  32767;}
	else if (newSample < -32768) {newSample = -32768;}

	index += indexTable[adpcm];
	if      (index <  0) { index =  0;}
	else if (index > 88) { index = 88;}

    prediction = newSample;
	return newSample;
}

// Compresses a stream 16-bit samples from src and stores the 4-bit values in dst
void compress(const char* src, char* dst, const WAVEHeader &wav, ADPCMHeader &adp)
{
    const sample* samplePtr = (const sample*)src;
    u_char* dataPtr = (u_char*)dst;
    
    short  stepIndex[2]  = { 0, 0 };
    sample prediction[2] = { 0, 0 };

    // Assign sample cound and channel count in the header
    memset(&adp, 0, sizeof(ADPCMHeader));
    memcpy(&adp.chunkID, "adp ", 4);
    memcpy(&adp.dataID,  "data", 4);
    adp.channelCount = wav.numChannels;
    adp.sampleCount  = wav.subchunk2Size / (wav.bitsPerSample/8);
    adp.dataSize     = (adp.sampleCount - adp.channelCount + 1) /2;
    adp.chunkSize    = 16 + 8 + adp.dataSize;

    // Add key values to channel 1
    if(wav.numChannels >= 1)
    {   adp.ch1KeySample = *samplePtr;
        adp.ch1StepIndex = stepIndex[0];
        prediction[0] = *samplePtr;
        samplePtr++;
    }

    // Add key values to channel 2
    if(wav.numChannels >= 2)
    {   adp.ch2KeySample = *samplePtr;
        adp.ch2StepIndex = stepIndex[1];
        prediction[1] = *samplePtr;
        samplePtr++;
    }

    // Process all samples of each channel and store the 4-bit data
    memset(dataPtr, 0, adp.dataSize);
    for(int i = adp.channelCount, channel = 0, shift = 4; i < adp.sampleCount; i++)
    {
        *dataPtr |= _16Linear_to_4ADPCM(*samplePtr, prediction[channel], stepIndex[channel]) << shift;
        samplePtr++;

        // If processed the last channel, go back to the first
        if(++channel == adp.channelCount)
        {   channel = 0;
        }

        // If the byte reached the end, start from the next
        if(shift == 0)
        {   shift = 4;
            *(++dataPtr) = 0;
        }
        // Else move the 4-bit shift to the right half
        else
        {   shift = 0;
        }
    }
}

// Decompresses a stream 4-bit values from src and stores the 16-bit samples in dst
void decompress(const char* src, char* dst, const ADPCMHeader &adp)
{
    const u_char* dataPtr = (const u_char*)src;
    sample* samplePtr = (sample*)dst;

    short  stepIndex[2]  = { 0, 0 };
    sample prediction[2] = { 0, 0 };

    // Extract key values for channel 1
    if(adp.channelCount >= 1)
    {   prediction[0] = adp.ch1KeySample;
        stepIndex[0]  = adp.ch1StepIndex;
        *samplePtr = prediction[0];
        samplePtr++;
    }

    // Extract key values for channel 2
    if(adp.channelCount >= 2)
    {   prediction[1] = adp.ch2KeySample;
        stepIndex[1]  = adp.ch2StepIndex;
        *samplePtr = prediction[1];
        samplePtr++;
    }

    // Process all samples of each channel and store the 4-bit data
    for(int i = adp.channelCount, channel = 0, shift = 4; i < adp.sampleCount; i++)
    {
        *samplePtr = _4ADPCM_to_16Linear((*dataPtr >> shift) & 0x0F, prediction[channel], stepIndex[channel]);
        samplePtr++;

        // If processed the last channel, go back to the first
        if(++channel == adp.channelCount)
        {   channel = 0;
        }

        // If the byte reached the end, start from the next
        if(shift == 0)
        {   shift = 4;
            dataPtr++;
        }
        // Else move the 4-bit shift to the right half
        else
        {   shift = 0;
        }
    }
}

// Checks if the ADPCM Header is correctly formatted
bool isCorrectHeader(ADPCMHeader &hdr)
{
    if(memcmp(hdr.chunkID, "adp ", 4) == 0)
    {   
        if(memcmp(hdr.dataID, "data", 4) == 0)
        {   
            int expectedSize = 16 + (8 + hdr.dataSize);
            if(hdr.chunkSize == expectedSize)
            {   
                return true;
            }
        }
    }

    return false;
}

// Returns the bytes required to store the compressed data
int ADPCMDataSize(const WAVEHeader &wav)
{
        return (wav.subchunk2Size / (wav.bitsPerSample/8) - wav.numChannels + 1) /2;
}