#include <adpcm-lib/wave.h>
#include <string.h>

bool isCorrectHeader(struct WaveHeader *hdr)
{
    if(memcmp(hdr->chunkID, "RIFF", 4) == 0)
    {   
        if(memcmp(hdr->format, "WAVE", 4) == 0)
        {   
            if(memcmp(hdr->subchunk1ID, "fmt ", 4) == 0)
            {   
                if(memcmp(hdr->subchunk2ID, "data", 4) == 0)
                {   
                    int expectedSize = 4 + (8 + hdr->subchunk1Size) + (8 + hdr->subchunk2Size);
                    if(hdr->subchunk1Size == 16 && hdr->chunkSize == expectedSize)
                    {   
                        int expectedAlign    = hdr->numChannels * hdr->bitsPerSample/8;
                        int expectedByteRate = expectedAlign * hdr->sampleRate;
                        if(hdr->blockAlign == expectedAlign && hdr->byteRate == expectedByteRate)
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}