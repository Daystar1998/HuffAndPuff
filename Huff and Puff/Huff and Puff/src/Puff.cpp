// File Name: Puff.cpp
// This program decodes a huf file created using the huffman algorithm.
// In this implementation the program includes functions:
// 1) readHeader - reads the document title length, document title, and the huffman table size.
// 2) readHuffTable - reads the huffman table and stores it in an array.
// 3) readFileInfo - reads the file information and stores it as an array of bits.
// 4) writeBitString - decodes using the huffman table from readHuffTable and writes the bit string to the file title read in readHeader.
// Name: Taylor Barber
// Date: 11/3/2019
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>
using namespace std;

const int huffEntrySize = 12;
const int ENDOFFILE = 256;
const int BYTESIZE = 8;

// Struct to contain the data stored in a entry in the huffman table.
struct huffEntry
{
	int glyph;
	int leftPointer;
	int rightPointer;
};

// Function Name: readFileInfo
// Description: This function accepts an fstream object connected to a huf file, the data size of the huffman table
// stored in the huf file, the huffman table from the huf file, and an empty bit string to store the bit values
// Using these input values, the function loops through each byte to find its value in binary using reverse byte architecture.
// During the looping process, the bit is stored in the bit string for later use.
void readFileInfo(ifstream& fin, int huffDataSize, huffEntry* huffTree, int* bitString)
{

	unsigned char* fileData = new unsigned char[huffDataSize];
	fin.read((char*)fileData, huffDataSize);

	unsigned char byte;
	short bitPos;
	int huffDataBitCounter = 0;

	// test bits in the byte specified to store it in the
	// bitString array in reverse byte architecture.
	for (int i = 0; i < huffDataSize; i++)
	{
		byte = fileData[i];
		for (bitPos = 0; bitPos <= 7; bitPos++)
		{
			if (byte & (unsigned char)pow(2.0, bitPos))
				bitString[huffDataBitCounter] = 1;
			else
				bitString[huffDataBitCounter] = 0;

			huffDataBitCounter++;
		}
	}
	delete[huffDataSize] fileData;
}

// Function Name: readHuffTable
// Description: This function will have inputs of an ifstream object that is linked to a huf file, the number of huffman table entries there
// are in the file, and an emty array to store the huffman table read from the file. It will loop through the entries storing them
// in an array of huffEntries (struct at top of page) that has a glyph, left pointer, and right pointer.
void readHuffTable(ifstream& fin, int huffTableEntries, huffEntry* huffTree)
{
	int getPointerLocation;

	int j = 0;
	for (int i = 0; i < huffTableEntries; i++)
	{
		fin.read((char*)&huffTree[i].glyph, sizeof(int));
		fin.read((char*)&huffTree[i].leftPointer, sizeof(int));
		fin.read((char*)&huffTree[i].rightPointer, sizeof(int));
	}
}

// Function Name: readHeader
// Description: This method reads the file name, and the huffman table entry amount.
void readHeader(ifstream& fin, int& HuffTableEntries, int& fileNameLength, unsigned char* compressedFile)
{
	fin.read((char*)compressedFile, fileNameLength);
	fin.read((char*)&HuffTableEntries, sizeof(int));

	compressedFile[fileNameLength] = NULL;
}

// Function Name: writeBitString
// Description: This method accepts an ofstream object, the huffman table created in readHuffTable, the bit string from
// readFileInfo, and the file data's bit size. It will then loop through the huffman tablecomparing each bit from
// the bit string in order to find a leaf node. It will also print each glyph on the leaf nodes found using the bitcodes
// to the fout object. If it reaches an end of file glyph, the function is terminated.
void writeBitString(ofstream& fout, huffEntry* huffTree, int* bitString, int huffDataBitSize)
{
	int nodePosition = 0;

	for (int i = 0; i < huffDataBitSize; i++)
	{
		if (huffTree[nodePosition].glyph == ENDOFFILE)
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
			fout << (char)huffTree[nodePosition].glyph;
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

	clock_t begin = clock();

	ifstream fin(filename, ios::in | ios::binary);

	if (fin)
	{
		// Gets the huf file size
		fin.seekg(0, ios::end);
		huffFileSize = fin.tellg();
		fin.seekg(0, ios::beg);

		// Reads in the file name length from the huf file and uses it
		// to create an array of unsigned chars for the file name
		fin.read((char*)&fileNameLength, sizeof(int));
		unsigned char* compressedFile = new unsigned char[fileNameLength];

		readHeader(fin, huffTableEntries, fileNameLength, compressedFile);

		// Created to store the huffman table
		huffEntry* huffTree = new huffEntry[huffTableEntries];

		//Gets the size of the file data in bits from the huff file to create a string of bits
		// from the file data
		huffHeaderSize = fileNameLength + BYTESIZE;
		huffTableByteSize = huffEntrySize * huffTableEntries;
		huffDataSize = huffFileSize - ((huffTableEntries * huffEntrySize) + huffHeaderSize);
		int huffDataBitSize = huffDataSize * BYTESIZE;
		int* bitString = new int[huffDataBitSize];

		readHuffTable(fin, huffTableEntries, huffTree);
		readFileInfo(fin, huffDataSize, huffTree, bitString);

		// Converts the file name to a string to use it in creating an output file
		string fileName(reinterpret_cast<char*>(compressedFile));
		ofstream fout(fileName, ios::binary | ios::out);

		if (fout)
		{
			writeBitString(fout, huffTree, bitString, huffDataBitSize);

			fout.close();
			fin.close();
			delete[huffTableEntries] huffTree;
			delete[huffDataBitSize] bitString;
		}

		else
		{
			cout << "unable to open output file...program exiting" << endl;
			exit(EXIT_FAILURE);
		}
	}

	else
	{
		cout << "unable to open file...program exiting" << endl;
		exit(EXIT_FAILURE);
	}

	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	cout << "Time elapsed: " << elapsed_secs << endl;
	return 0;
}
