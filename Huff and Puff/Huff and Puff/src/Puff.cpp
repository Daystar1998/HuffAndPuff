#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>
using namespace std;

const int MAXHUFFSIZE = 513;
const int huffEntrySize = 12;
const int ENDOFFILE = 256;
const int BYTESIZE = 8;

struct huffEntry
{
	unsigned char glyph;
	int leftPointer;
	int rightPointer;
};


void readFileInfo(ifstream& fin, int huffDataSize, huffEntry* huffTree, int* bitString)
{

	unsigned char* fileData = new unsigned char[huffDataSize];
	fin.read((char*)fileData, huffDataSize);
	cout << endl << huffDataSize << endl;

	unsigned char byte;
	short bitPos;
	int tablePosition;
	int huffDataBitCounter = 0;

	cout << "BITSTRING = ";
	//test bits
	for (int i = 0; i < huffDataSize; i++)
	{
		byte = fileData[i];
		for (bitPos = 0; bitPos <= 7; bitPos++)
		{
			if (byte & (unsigned char)pow(2.0, bitPos))
				bitString[huffDataBitCounter] = 1;
			else
				bitString[huffDataBitCounter] = 0;

			cout << bitString[huffDataBitCounter];
			huffDataBitCounter++;
		}
	}
	cout << endl;
	//delete[] fileData;
}

void readHuffTable(ifstream& fin, int huffTableEntries, huffEntry* huffTree)
{
	int getPointerLocation;

	int j = 0;
	for (int i = 0; i < huffTableEntries; i++)
	{
		fin.read((char*)&huffTree[i].glyph, sizeof(unsigned char));
		getPointerLocation = fin.tellg();
		fin.seekg(getPointerLocation + 3);
		fin.read((char*)&huffTree[i].leftPointer, sizeof(int));
		fin.read((char*)&huffTree[i].rightPointer, sizeof(int));

		cout << endl << huffTree[i].glyph << " " << huffTree[i].leftPointer << " " << huffTree[i].rightPointer << endl;
	}
}

void readHeader(ifstream& fin, int& HuffTableEntries, int& fileNameLength, unsigned char* compressedFile)
{
	fin.read((char*)compressedFile, fileNameLength);
	fin.read((char*)&HuffTableEntries, sizeof(int));

	compressedFile[fileNameLength] = NULL;
	cout << fileNameLength << endl;
	for (int i = 0; i < fileNameLength; i++)
	{
		cout << compressedFile[i];
	}

	cout << endl << HuffTableEntries << endl;
}

void writeBitString(ofstream& fout, huffEntry* huffTree, int* bitString, int huffDataBitSize)
{
	int nodePosition = 0;

	for (int i = 0; i < huffDataBitSize; i++)
	{
		//make code to decode the bitstring
		if (huffTree[nodePosition].glyph == NULL)
		{
			return;
		}

		if (bitString[i] == 1 && huffTree[nodePosition].rightPointer != -1)
		{
			nodePosition = huffTree[nodePosition].rightPointer;
		}

		else if (bitString[i] == 0 && huffTree[nodePosition].leftPointer != -1)
		{
			nodePosition = huffTree[nodePosition].leftPointer;
		}

		else
		{
			fout << huffTree[nodePosition].glyph;
			//fout.write((char*)&huffTree[nodePosition].glyph, sizeof(unsigned char));
			nodePosition = 0;
			i--;
		}
	}
}

int main()
{

	int huffTableEntries = 0;
	int huffTableByteSize = 0;
	int huffFileSize = 0;
	int huffDataSize = 0;
	int fileNameLength = 0;
	int huffHeaderSize = 0;

	string filename;
	cout << "What is the file you would like to decompress? ";
	cin >> filename;

	ifstream fin(filename, ios::in | ios::binary);

	if (fin)
	{
		fin.seekg(0, ios::end);
		huffFileSize = fin.tellg();
		fin.seekg(0, ios::beg);

		fin.read((char*)&fileNameLength, sizeof(int));
		unsigned char* compressedFile = new unsigned char[fileNameLength];

		readHeader(fin, huffTableEntries, fileNameLength, compressedFile);
		string fileName(reinterpret_cast<char*>(compressedFile));
		cout << endl << fileName << endl;
		ofstream fout(fileName, ios::binary | ios::out);

		huffEntry* huffTree = new huffEntry[huffTableEntries];

		huffHeaderSize = fileNameLength + (sizeof(int) * 2);
		huffTableByteSize = huffEntrySize * huffTableEntries;
		huffDataSize = huffFileSize - ((huffTableEntries * huffEntrySize) + huffHeaderSize);
		int huffDataBitSize = huffDataSize * BYTESIZE;
		int* bitString = new int[huffDataBitSize];

		readHuffTable(fin, huffTableEntries, huffTree);
		readFileInfo(fin, huffDataSize, huffTree, bitString);

		writeBitString(fout, huffTree, bitString, huffDataBitSize);
		fout.close();
		fin.close();
		delete[huffTableEntries] huffTree;
		//delete[fileNameLength] compressedFile;
		delete[huffDataBitSize] bitString;
	}

	else
	{
		cout << "unable to open file...program exiting" << endl;
		exit(EXIT_FAILURE);
	}
	return 0;
}

