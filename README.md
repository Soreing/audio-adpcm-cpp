# audio-adpcm-cpp

# Description
audio-adpcm-cpp is a lossy audio compression c++ library for WAVE files. The compression uses ADPCM algorithm outlined by the Interactive Multimedia Association (IMA). The algorithm compresses WAVE files to 25% of their size.  

# Installation
Add the folder `adpcm-lib` from `/include` in your include path. If you want to compile the library from source, include `wave.cpp` and `adpcm.cpp` from the `/src` folder. Alternatively, you can compile the source code to a static library and include it that way.

`Source.cpp` is an implementation to use the library as a command in the command line terminal with arguments.

# Basic Usage
## Compression
The library can compress WAVE files that have at most 2 channels (Mono and Stereo), and 16-bit sample size. Once you opened and read a WAVE file, you need the WAVE header and a pointer to the samples located in the data subchunk.
```c++
WAVEHeader wav;
ifstream in(fileName, ios::binary);

in.read((char*)&wav, sizeof(WAVEHeader));

char* samples = new char[wav.subchunk2Size];
in.read(samples, wav.subchunk2Size);
```
You will also need an array to store the compressed samples in, and an ADPCM header structure that will be filled during the compression. You can use the WAVE header to get how many bytes of space the compressed data will take
```c++
ADPCMHeader adp;
char* data = new char[ADPCMDataSize(wav)];
```
Finally, you can compress the samples.
```c++
compress(samples, data, wave, adpcm);
```


## Decompression
You can decompress data with the ADPCM header created during compression and the compressed samples. It is recommended to also store the WAVE header with the compressed data for reconstruction.
```c++
WAVEHeader  wave;
ADPCMHeader adpcm;

ifstream in(input, ios::binary);

in.read((char*)&adpcm, sizeof(ADPCMHeader));
in.read((char*)&wave, sizeof(WAVEHeader));

char* samples = new char[wave.subchunk2Size];
char* data = new char[adpcm.dataSize];
in.read((char*)data, adpcm.dataSize);

decompress(data, samples, adpcm);
```

# Command Line Usage
You can compile the program or use the released binary in the `/bin` folder to compress and decompress WAVE files through the terminal.
```
// Compresstion from origin.wav to compressed.adp
adpcm -c origin.wav compressed.adp

// Decompress from compressed.adp to origin.wav
adpcm -d compressed.adp origin.wav
```