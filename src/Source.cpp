#include <adpcm-lib/wave.h>
#include <adpcm-lib/adpcm.h>
#include <iostream>
#include <fstream>

int optCompress(char* input, char* output)
{
    WAVEHeader  wave;
    ADPCMHeader adpcm;

    std::ifstream in(input, std::ios::binary);

    if(!in)
    {   std::cerr << input << " : No such file or directory\n";
        return 1;
    }

    in.read((char*)&wave, sizeof(WAVEHeader));

    if(isCorrectHeader(wave))
    {
        if(wave.bitsPerSample == 16 || wave.numChannels <= 2)
        {   char* data    = new char[ADPCMDataSize(wave)];
            char* samples = new char[wave.subchunk2Size];
            in.read(samples, wave.subchunk2Size);
            in.close();

            compress(samples, data, wave, adpcm);

            std::ofstream out(output, std::ios::binary);
            out.write((char*)&adpcm, sizeof(ADPCMHeader));
            out.write((char*)&wave, sizeof(WAVEHeader));
            out.write(data, adpcm.dataSize);
            out.close();
        }
        else
        {   std::cerr << input << ": WAVE Format not supported\n";
            in.close();
            return 1;
        }
    }
    else
    {   std::cerr << input << ": File Type not supported\n";
        in.close();
        return 1;
    }

    return 0;
}

int optDecompress(char* input, char* output)
{
    WAVEHeader  wave;
    ADPCMHeader adpcm;

    std::ifstream in(input, std::ios::binary);

    if(!in)
    {   std::cerr << input << " : No such file or directory\n";
        return 1;
    }

    in.read((char*)&adpcm, sizeof(ADPCMHeader));
    in.read((char*)&wave, sizeof(WAVEHeader));

    if(isCorrectHeader(adpcm))
    {
        if(isCorrectHeader(wave))
        {   char* samples = new char[wave.subchunk2Size];
            char* data    = new char[adpcm.dataSize];
            in.read((char*)data, adpcm.dataSize);
            in.close();

            decompress(data, samples, adpcm);

            std::ofstream out(output, std::ios::binary);
            out.write((char*)&wave, sizeof(WAVEHeader));
            out.write(samples, wave.subchunk2Size);
            out.close();
        }
        else
        {   std::cerr << input << ": WAVE Format is corrupted\n";
            in.close();
            return 1;
        }
    }
    else
    {   std::cerr << input << ": File Type not supported\n";
        in.close();
        return 1;
    }

    return 0;
}

int main(int argc, char* argv[]) 
{
    if(argc < 4)
    {   std::cerr << argv[0] << ": Not enough arguments\n";
        return 1;
    }

    if(strcmp(argv[1], "-c") == 0)
    {   return optCompress(argv[2], argv[3]);
    }
    else if(strcmp(argv[1],"-d") == 0)
    {   return optDecompress(argv[2], argv[3]);
    }
    else
    {   std::cerr<< "Invalid option, expected -c or -d\n";
        return 1;
    }

    return 0;
}