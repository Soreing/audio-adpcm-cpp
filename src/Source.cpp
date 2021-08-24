#include <adpcm-lib/wave.h>
#include <iostream>
#include <fstream>

int main() 
{
    struct WaveHeader hdr;
    std::ifstream in("test.wav", std::ios::binary);
    
    in.read((char*)&hdr, sizeof(struct WaveHeader));
    std::cout<< isCorrectHeader(&hdr) << "\n";
    system("PAUSE");
}