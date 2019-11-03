/******************************************************************************
	Name: huff.h
	Des:
		Performs a file compression using the Huffman algorithm
	Author: Matthew Day
	Date: 10/28/2019
******************************************************************************/

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
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
		fileName - type string &, the name of the file
		oDataLength - type int &, the length of the data
	Returns:
		type char *, the contents of the file
******************************************************************************/
char *readFile(string &fileName, int &oDataLength) {

	char *result = nullptr;

	ifstream fin;
	fin.open(fileName, ios::in | ios::binary | ios::ate);

	if (fin.is_open()) {

		oDataLength = (int)fin.tellg();
		fin.seekg(ios::beg);
		result = new char[oDataLength];
		fin.read(result, (streamsize)oDataLength);
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

	int frequencyTable[MAX_GLYPHS] = { 0 };

	for (int i = 0; i < dataLength; i++) {

		unsigned char glyph = data[i];

		frequencyTable[glyph]++;
	}

	frequencyTable[EOF_GLYPH] = 1;

	vector<HuffmanNode> result;

	for (int i = 0; i < MAX_GLYPHS; i++) {

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
	sort(result.begin(), result.end(), [](HuffmanNode &a, HuffmanNode &b) {

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
		oCompressedDataLength += (int)bitcode.size() * currentNode.frequency;
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
		bitcodeMap - type map<int, string> &bitcodeMap &, the map of bitcodes
		data - type char *, the original data
		dataLength - type int, the length of the original data
		compressedDataLengthInBytes - type int, the length of the
			compressedData in bytes
	Returns:
		type unsigned char*, the compressed data
******************************************************************************/
unsigned char *compressData(map<int, string> &bitcodeMap, char *data, int dataLength, int compressedDataLengthInBytes) {

	unsigned char *compressedData = new unsigned char[compressedDataLengthInBytes] { 0 };

	int currentCompressedByte = 0;
	int currentBit = 0;

	// Encode right to left

	for (int i = 0; i < dataLength; i++) {

		string currentGlyphBitcode = bitcodeMap[(unsigned char)data[i]];

		for (int j = 0; j < currentGlyphBitcode.size(); j++) {

			if (currentBit >= sizeof(char) * BYTE_SIZE) {

				currentBit = 0;
				currentCompressedByte++;
			}

			if (currentGlyphBitcode[j] == '1') {

				unsigned char bit = 0b00000001 << currentBit;
				compressedData[currentCompressedByte] |= bit;
			} else {

				// Do nothing
			}

			currentBit++;
		}
	}

	// Add EOF glyph
	string currentGlyphBitcode = bitcodeMap[EOF_GLYPH];

	for (int j = 0; j < currentGlyphBitcode.size(); j++) {

		if (currentBit >= sizeof(char) * BYTE_SIZE) {

			currentBit = 0;
			currentCompressedByte++;
		}

		if (currentGlyphBitcode[j] == '1') {

			unsigned char bit = 0b00000001 << currentBit;
			compressedData[currentCompressedByte] |= bit;
		} else {

			// Do nothing
		}

		currentBit++;
	}

	return compressedData;
}

/******************************************************************************
	Name: printOutput
	Des:
		Print the compressed data to a file
	Params:
		fileName - type string &, the name of the file
		huffmanTable - type vector<HuffmanNode> &, the huffman table
		compressedData - type unsigned char *, the compressed data
		compressedDataLengthInBytes - type int, the length of the
			compressedData in bytes
******************************************************************************/
void printOutput(string &fileName, vector<HuffmanNode> &huffmanTable, unsigned char *compressedData, int compressedDataLengthInBytes) {

	string hufFileExtension = ".huf";

	string outputFileName = fileName.substr(0, fileName.find_last_of('.')) + hufFileExtension;

	ofstream fout(outputFileName, ios::out | ios::binary);

	if (fout.is_open()) {

		int originalFileNameLength = (int)fileName.size();
		int numberOfHuffmanEntries = (int)huffmanTable.size();

		fout.write((char *)& originalFileNameLength, sizeof(int));
		fout.write((char *)fileName.c_str(), originalFileNameLength);

		fout.write((char *)& numberOfHuffmanEntries, sizeof(int));

		for (int i = 0; i < huffmanTable.size(); i++) {

			fout.write((char *)& huffmanTable[i].glyph, sizeof(int));
			fout.write((char *)& huffmanTable[i].left, sizeof(int));
			fout.write((char *)& huffmanTable[i].right, sizeof(int));
		}

		fout.write((char *)compressedData, compressedDataLengthInBytes);
	}

	fout.close();
}

int main() {

	string fileName;

	cout << "Enter the name of the file you want to compress: ";

	cin >> fileName;

	clock_t startTime = clock();

	int dataLength;

	char *data = readFile(fileName, dataLength);

	if (data != nullptr) {

		vector<HuffmanNode> huffmanTable = generateInitialHuffmanTable(data, dataLength);

		buildHuffmanTable(huffmanTable, (int)huffmanTable.size() - 1);

		map<int, string> bitcodeMap;

		int compressedDataLength = 0;

		generateBitcodes(huffmanTable, bitcodeMap, "", ROOT_NODE, compressedDataLength);

		// Convert from bits to bytes
		compressedDataLength = (int)ceil(compressedDataLength / (double)BYTE_SIZE);

		unsigned char *compressedData = compressData(bitcodeMap, data, dataLength, compressedDataLength);

		printOutput(fileName, huffmanTable, compressedData, compressedDataLength);

		delete[compressedDataLength] compressedData;
		delete[dataLength] data;

		clock_t endTime = clock();
		double secondsTaken = ((double)endTime - (double)startTime) / CLOCKS_PER_SEC;

		cout << "Time taken: " << fixed << setprecision(6) << secondsTaken << endl;
	}
}