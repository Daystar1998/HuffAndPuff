/******************************************************************************
	Name: huff.h
	Des:
		Performs a file compression using the Huffman algorithm
	Author: Matthew Day
	Date: 10/28/2019
******************************************************************************/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

const int MAX_GLYPHS = 257;
const int EOF_GLYPH = 256;

const int BYTE_SIZE = 8;

const int EOF_GLYPH_COUNT = 1;
const int DEFAULT_NODE_POINTER = -1;

const int ROOT_NODE = 0;
const int SLOT_TWO_INDEX = 1;
const int SLOT_THREE_INDEX = 2;

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
		oDataLength - type int &, the length of the data
	Returns:
		type char *, the contents of the file
******************************************************************************/
char *readFile(string fileName, int &oDataLength) {

	char *result = nullptr;

	ifstream fin;
	fin.open(fileName, ios::in | ios::binary | ios::ate);

	if (fin.is_open()) {

		oDataLength = (int)fin.tellg() + 1;
		fin.seekg(ios::beg);
		result = new char[oDataLength];
		fin.read(result, (streamsize)oDataLength - 1);

		result[oDataLength - 1] = EOF_GLYPH;
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
vector<HuffmanNode> generateInitialHuffmanTable(char *data, int dataLength) {

	vector<int> frequencyTable(MAX_GLYPHS);

	for (int i = 0; i < dataLength; i++) {

		int glyph = data[i];

		if (glyph == EOF_GLYPH) {

			frequencyTable[MAX_GLYPHS - 1]++;
		} else {

			frequencyTable[glyph]++;
		}
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

	// Sort by frequency
	sort(result.begin(), result.end(), [](HuffmanNode a, HuffmanNode b) {

		return a.frequency < b.frequency;
		});

	return result;
}

/******************************************************************************
	Name: reheap
	Des:
		Reheap the huffman table if necessary
	Params:
		huffmanTable - type vector<HuffmanNode> &, the huffman table
		reheapStartIndex - type int, the index where the reheap should start
		topOfHeap - type int, top of the heap of unsorted nodes
******************************************************************************/
void reheap(vector<HuffmanNode> &huffmanTable, int reheapStartIndex, int topOfHeap) {

	int leftNodeIndex = reheapStartIndex * 2 + 1;
	int rightNodeIndex = reheapStartIndex * 2 + 2;

	if (rightNodeIndex <= topOfHeap) {

		// Either left or right is less than start
		if (huffmanTable[leftNodeIndex].frequency < huffmanTable[reheapStartIndex].frequency || huffmanTable[rightNodeIndex].frequency < huffmanTable[reheapStartIndex].frequency) {

			// Right is less than left
			if (huffmanTable[rightNodeIndex].frequency < huffmanTable[leftNodeIndex].frequency) {

				HuffmanNode tempNode = huffmanTable[rightNodeIndex];

				huffmanTable[rightNodeIndex] = huffmanTable[reheapStartIndex];
				huffmanTable[reheapStartIndex] = tempNode;
				reheap(huffmanTable, rightNodeIndex, topOfHeap);
			// Left is less than or equal to right
			} else {

				HuffmanNode tempNode = huffmanTable[leftNodeIndex];

				huffmanTable[leftNodeIndex] = huffmanTable[reheapStartIndex];
				huffmanTable[reheapStartIndex] = tempNode;
				reheap(huffmanTable, leftNodeIndex, topOfHeap);
			}
		}
	} else if (leftNodeIndex == topOfHeap) {

		if (huffmanTable[leftNodeIndex].frequency < huffmanTable[reheapStartIndex].frequency) {

			HuffmanNode tempNode = huffmanTable[leftNodeIndex];

			huffmanTable[leftNodeIndex] = huffmanTable[reheapStartIndex];
			huffmanTable[reheapStartIndex] = tempNode;
			// Do not need to reheap final node
		}
	}
}

/******************************************************************************
	Name: buildHuffmanTable
	Des:
		Builds the huffman table
	Params:
		huffmanTable - type vector<HuffmanNode> &, the huffman table
		topOfHeap - type int, top of the heap of unsorted nodes
******************************************************************************/
void buildHuffmanTable(vector<HuffmanNode> &huffmanTable, int topOfHeap) {

	int markedIndex;

	if (topOfHeap != ROOT_NODE) {

		// Mark the lower frequency glyph at the second or third index
		if (topOfHeap == SLOT_TWO_INDEX) {

			markedIndex = SLOT_TWO_INDEX;
		} else {

			if (huffmanTable[SLOT_TWO_INDEX].frequency > huffmanTable[SLOT_THREE_INDEX].frequency) {

				markedIndex = SLOT_THREE_INDEX;
			} else {

				markedIndex = SLOT_TWO_INDEX;
			}
		}

		huffmanTable.push_back(huffmanTable[markedIndex]);

		if (topOfHeap != markedIndex) {

			huffmanTable[markedIndex] = huffmanTable[topOfHeap];
			reheap(huffmanTable, markedIndex, topOfHeap);
		}

		huffmanTable[topOfHeap] = huffmanTable[ROOT_NODE];

		// Create merge node
		HuffmanNode node;
		node.glyph = DEFAULT_NODE_POINTER;
		node.frequency = huffmanTable[topOfHeap].frequency + huffmanTable[huffmanTable.size() - 1].frequency;
		node.left = topOfHeap;
		node.right = (int)huffmanTable.size() - 1;

		huffmanTable[ROOT_NODE] = node;

		reheap(huffmanTable, ROOT_NODE, topOfHeap - 1);

		buildHuffmanTable(huffmanTable, topOfHeap - 1);
	}
}

/******************************************************************************
	Name: generateBitcodes
	Des:
		Generate map of glyph bitcodes
	Params:
		huffmanTable - type vector<HuffmanNode> &, the huffman table
		bitcodeMap - type map<int, string> &, the map of glyphs to bitcodes
		bitcode - type string, the current bitcode
		currentIndex - type int, the current node index
		oCompressedDataLength - type int &, the length of the compressedData
******************************************************************************/
void generateBitcodes(vector<HuffmanNode> &huffmanTable, map<int, string> &bitcodeMap, string bitcode, int currentIndex, int &oCompressedDataLength) {

	HuffmanNode currentNode = huffmanTable[currentIndex];

	if (currentNode.left == DEFAULT_NODE_POINTER && currentNode.right == DEFAULT_NODE_POINTER) {

		bitcodeMap[currentNode.glyph] = bitcode;
		oCompressedDataLength += bitcode.size() * currentNode.frequency;
	} else {

		if (currentNode.left != DEFAULT_NODE_POINTER) {

			generateBitcodes(huffmanTable, bitcodeMap, bitcode + '0', currentNode.left, oCompressedDataLength);
		}

		if (currentNode.right != DEFAULT_NODE_POINTER) {

			generateBitcodes(huffmanTable, bitcodeMap, bitcode + '1', currentNode.right, oCompressedDataLength);
		}
	}
}

/******************************************************************************
	Name: compressData
	Des:
		Compress the data using bitcodes
	Params:
		huffmanTable - type vector<HuffmanNode> &, the huffman table
		data - type char *, the original data
		dataLength - type int, the length of the original data
		compressedDataLengthInBytes - type int, the length of the
			compressedData in bytes
	Returns:
		type char*, the compressed data
******************************************************************************/
char* compressData(map<int, string> &bitcodeMap, char *data, int dataLength, int compressedDataLengthInBytes) {

	char *compressedData = new char[compressedDataLengthInBytes];

	int currentCompressedByte = 0;
	int currentBit = 0;

	compressedData[currentCompressedByte] = 0;

	// Encode right to left

	for (int i = 0; i < dataLength; i++) {

		string currentGlyphBitcode = bitcodeMap[data[i]];

		for (int j = 0; j < currentGlyphBitcode.size(); j++) {

			if (currentBit >= sizeof(char) * BYTE_SIZE) {

				currentBit = 0;
				currentCompressedByte++;
				compressedData[currentCompressedByte] = 0;
			}

			if (currentGlyphBitcode[j] == '1') {

				char bit = 0b00000001 << currentBit;
				compressedData[currentCompressedByte] |= bit;
			} else {

				// Do nothing
			}

			currentBit++;
		}
	}

	return compressedData;
}

int main() {

	string fileName;

	cin >> fileName;

	cout << fileName;

	int dataLength;

	char *data = readFile(fileName, dataLength);

	if (data != nullptr) {

		vector<HuffmanNode> huffmanTable = generateInitialHuffmanTable(data, dataLength);

		buildHuffmanTable(huffmanTable, (int)huffmanTable.size() - 1);

		map<int, string> bitcodeMap;

		int compressedDataLength = 0;

		generateBitcodes(huffmanTable, bitcodeMap, "", ROOT_NODE, compressedDataLength);

		// Convert from bits to bytes
		compressedDataLength = ceil(compressedDataLength / (double)BYTE_SIZE);

		char* compressedData = compressData(bitcodeMap, data, dataLength, compressedDataLength);

		delete[dataLength] data;
	}
}