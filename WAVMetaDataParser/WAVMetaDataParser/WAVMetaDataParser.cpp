#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <fstream>
#include "WavHeader.h"

//#define DEBUG

using namespace std;

void ExtractSubChunk2Size(ifstream& in, uint32_t& SubChunk2Size);
int computeFileSize(FILE* inFile);

int main(int argc, char* argv[])
{
	CWavHeader WAVItem;

	if (argc <= 1)
	{
		cout << "CMD argument not set\n";
	}

	string fileName = argv[1];
	string output = "OutputFor_" + fileName.substr(0,fileName.size()-4)+ ".txt";
	ofstream fout(output.c_str());

	FILE* inputWavFile = fopen(fileName.c_str(), "r");
	if (inputWavFile == nullptr)
	{
		fout << "WAV file didn't open\n";
		return 1;
	}

	//Reading the fixed Header : 36 Bytes
	size_t ItemSize = sizeof(WAVItem);
	cout << "ItemSize " << ItemSize << "\n";
	auto GetBytesRead = [&WAVItem](FILE* fileName, size_t wavHeaderSize)
	{
		return fread(&WAVItem, 1, wavHeaderSize, fileName);
	};
	size_t BytesRead = GetBytesRead(inputWavFile, ItemSize);

	//Extracting raw data from the original wav 
	//Exctracting SubChunk2Size
	ifstream in(fileName, ifstream::binary);
	uint32_t SubChunk2Size;
	ExtractSubChunk2Size(in, SubChunk2Size);
	
	WAVItem.SetChunkSize(36 + SubChunk2Size);
	//Header writing complete.

	int totalFilelength;
	if (BytesRead > 0)
	{
		totalFilelength = computeFileSize(inputWavFile);

		fout << "Audio Channels :" << WAVItem.GetNumChannels() << endl;
		fout << "Sampling Frequency :" << WAVItem.GetSamplingFrequency() << endl;
		fout << "BitsPerSample :" << WAVItem.GetBitsPerSample() << endl;
		fout << "Bit Rate:" << WAVItem.GetBitRate() << endl;

#ifdef DEBUG
		fout << "ByteRate: " << WAVItem.GetByteRate() << endl;
		fout << "Block align " << WAVItem.GetBlockAlign() << endl;
		fout << "Audio Format " << WAVItem.GetAudioFormat() << endl;
		fout << "SubChunk1Size " << WAVItem.GetSubChunk1Size() << "\n";
		fout << "ChunkSize " << WAVItem.GetChunkSize() << "\n";
#endif
		auto GetDuration = [&WAVItem, &SubChunk2Size]() -> int
		{
			return (SubChunk2Size) / (WAVItem.GetSamplingFrequency()*WAVItem.GetNumChannels()*WAVItem.GetBitsPerSample() / 8);
		};


		fout << "Duration :" << GetDuration() << "s" << endl;

	}

	
	return 0;
}

void ExtractSubChunk2Size(ifstream& in, uint32_t& SubChunk2Size)
{
	string beyond36;
	getline(in, beyond36, 'd');

	char c;
	while (in >> c)
	{
		if (c == 'a')
			break;
	}
	in.ignore(2);
	in.read(reinterpret_cast<char*>(&SubChunk2Size), sizeof(uint32_t));
}


int computeFileSize(FILE* inFile)
{
	int fileSize = 0;
	fseek(inFile, 0, SEEK_END);

	fileSize = ftell(inFile);

	fseek(inFile, 0, SEEK_SET);
	return fileSize;
}
