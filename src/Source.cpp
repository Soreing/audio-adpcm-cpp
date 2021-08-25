#include <adpcm-lib/wave.h>
#include <adpcm-lib/adpcm.h>
#include <iostream>
#include <fstream>

void mcompress()
{
    WAVEHeader  wave;
    ADPCMHeader adpcm;

    std::ifstream in("orig.wav", std::ios::binary);
    std::ofstream comp("comp.adp", std::ios::binary);

    in.read((char*)&wave, sizeof(WAVEHeader));
    if(isCorrectHeader(wave))
    {
        char* data    = new char[ADPCMDataSize(wave)];
        char* samples = new char[wave.subchunk2Size];
        in.read(samples, wave.subchunk2Size);

        compress(samples, data, wave, adpcm);
        comp.write((char*)&adpcm, sizeof(ADPCMHeader));
        comp.write((char*)&wave, sizeof(WAVEHeader));
        comp.write(data, adpcm.dataSize);
    }
}

void mdecompress()
{
    WAVEHeader  wave;
    ADPCMHeader adpcm;

    std::ifstream in("comp.adp", std::ios::binary);
    std::ofstream decomp("decomp.wav", std::ios::binary);

    in.read((char*)&adpcm, sizeof(ADPCMHeader));
    in.read((char*)&wave, sizeof(WAVEHeader));

    char* samples = new char[wave.subchunk2Size];
    char* data    = new char[adpcm.dataSize];
    in.read((char*)data, adpcm.dataSize);

    decompress(data, samples, adpcm);
    decomp.write((char*)&wave, sizeof(WAVEHeader));
    decomp.write(samples, wave.subchunk2Size);
}

int main() 
{
    //mcompress();
    mdecompress();
}