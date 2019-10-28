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

int main() {

	string fileName;

	cin >> fileName;

	cout << fileName;

	int fileLength;

	char* data = readFile(fileName, fileLength);

	if (data != nullptr) {

		delete[fileLength] data;
	}
}