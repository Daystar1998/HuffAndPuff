#include <iostream>
#include <string>
#include <fstream>
using namespace std;

struct huffEntry
{
	int glyph;
	int leftPointer;
	int rightPointer;
};

void readHuffTable(fstream& fin, int huffTableByteSize, int huffTableEntries)
{
	/*unsigned char* huffTable = new unsigned char[huffTableByteSize];
	fin.read((char*)huffTable, huffTableByteSize);*/

	huffEntry* huffTree = new huffEntry[huffTableEntries];

	int j = 0;
	for (int i = 0; i < huffTableEntries; i++)
	{
			fin.read((char*) &huffTree[i].glyph, sizeof(int));
			fin.read((char*) &huffTree[i].leftPointer, sizeof(int));
			fin.read((char*) &huffTree[i].rightPointer, sizeof(int));
	}

	for (int i = 0; i < huffTableEntries; i++)
	{
		cout << endl << huffTree[i].glyph << " " << huffTree[i].leftPointer << " " << huffTree[i].rightPointer << endl;
	}
}

void readHeader(fstream &fin, int& HuffTableEntries)
{
	int filenameLength = 0;

	if (fin.is_open())
	{
		fin.read((char*)& filenameLength, sizeof(int));
		unsigned char* compressedFile = new unsigned char[filenameLength];
		fin.read((char*)compressedFile, filenameLength);
		fin.read((char*)& HuffTableEntries, sizeof(int));

		cout << filenameLength << endl;
		for (int i = 0; i < filenameLength; i++)
		{
			cout << compressedFile[i];
		}

		cout << endl << HuffTableEntries << endl;
	}

	else
	{
		cout << "unable to open file...program exiting" << endl;
		exit(EXIT_FAILURE);
	}
}

void main()
{
	const int MAXHUFFSIZE = 513;
	const int huffEntrySize = 12;
	int huffTableEntries = 0;
	int huffTableByteSize = 0;


	string filename;
	cout << "What is the file you would like to decompress? ";
	cin >> filename;

	fstream fin(filename, ios::in | ios::binary);

	readHeader(fin, huffTableEntries);

	huffTableByteSize = huffEntrySize * huffTableEntries;

	readHuffTable(fin, huffTableByteSize, huffTableEntries);
}

