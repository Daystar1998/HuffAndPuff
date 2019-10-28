/******************************************************************************
	Name: huff.h
	Des:
		Performs a file compression using the Huffman algorithm
	Author: Matthew Day
	Date: 10/28/2019
******************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

const int MAX_GLYPHS = 256;
const int EOF_GLYPH = -1;

const int EOF_GLYPH_COUNT = 1;
const int DEFAULT_NODE_POINTER = -1;

struct HuffmanNode {

	int glyph;
	int frequency;
	int left;
	int right;
};

/******************************************************************************
	Name: readFile
	Des:
		Reads a file
	Params:
		fileName - type string, the name of the file
		oFileLength - type int &, the length of the file
	Returns:
		type char *, the contents of the file
******************************************************************************/
char* readFile(string fileName, int &oFileLength) {

	char* result = nullptr;

	ifstream fin;
	fin.open(fileName, ios::in | ios::binary | ios::ate);

	if (fin.is_open()) {

		oFileLength = fin.tellg();
		fin.seekg(ios::beg);
		result = new char[oFileLength];
		fin.read(result, oFileLength);
	}

	fin.close();

	return result;
}

/******************************************************************************
	Name: generateInitialHuffmanTable
	Des:
		Builds the huffman table
	Params:
		data - type char *, the data for the huffman table
		data - type int, the length of the data
	Returns:
		type vector<HuffmanNode>, the huffman table
******************************************************************************/
vector<HuffmanNode> generateInitialHuffmanTable(char* data, int dataLength) {

	vector<int> frequencyTable(MAX_GLYPHS);

	for (int i = 0; i < dataLength; i++) {

		int glyph = data[i];

		frequencyTable[glyph]++;
	}

	vector<HuffmanNode> result;

	for (int i = 0; i < frequencyTable.size(); i++) {

		// If the glyph has a frequency greater than zero
		if (frequencyTable[i] > 0) {

			HuffmanNode node;

			node.glyph = i;
			node.frequency = frequencyTable[i];
			node.left = DEFAULT_NODE_POINTER;
			node.right = DEFAULT_NODE_POINTER;

			result.push_back(node);
		}
	}

	// Add the EOF glyph to the table
	HuffmanNode node;

	node.glyph = EOF_GLYPH;
	node.frequency = EOF_GLYPH_COUNT;
	node.left = DEFAULT_NODE_POINTER;
	node.right = DEFAULT_NODE_POINTER;

	result.push_back(node);

	// Sort by frequency
	sort(result.begin(), result.end(), [](HuffmanNode a, HuffmanNode b) {

		return a.frequency < b.frequency;
		});

	return result;
}

int main() {

	string fileName;

	cin >> fileName;

	cout << fileName;

	int fileLength;

	char* data = readFile(fileName, fileLength);

	if (data != nullptr) {

		vector<HuffmanNode> huffmanTable = generateInitialHuffmanTable(data, fileLength);

		delete[fileLength] data;
	}
}