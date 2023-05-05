// File Name : util.h
// Name : Moe Judeh
// netID : mjude4
// Course Info : CS 251 - Data Structures (34460)
// Description : codes and decodes files to help save 
//               on storage space
// Data : 04/12/2022

#include <iostream>
#include <fstream>
#include <map>
#include <queue>          // std::priority_queue
#include <vector>         // std::vector
#include <functional>     // std::greater
#include <string>
#include "bitstream.h"
#include "hashmap.h"
#include "mymap.h"
#pragma once

struct HuffmanNode {
    int character;
    int count;
    HuffmanNode* zero;
    HuffmanNode* one;
};


//
// *This function checks to see if the current node
// has childern if it does it calls itself using
// those nodes as the parameter and then deletes current
//
void _delete(HuffmanNode *curr) {
    if (curr == nullptr) {
        return;
    }
    if (curr->zero != nullptr) {
        _delete(curr->zero);
    }
    if (curr->one != nullptr) {
        _delete(curr->one);
    }
    delete curr;
}


//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode* node) {
    _delete(node);
}

//
// *This function build the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmap &map) {
    if (isFile) {
        ifstream inFS(filename);
        char c;
        while (inFS.get(c)) {
            if (map.containsKey(c)) {
                map.put(c, map.get(c) + 1);
            } else {
                map.put(c, 1);
            }
        }
    } else {
        for (char c : filename) {
            if (map.containsKey(c)) {
                map.put(c, map.get(c) + 1);
            } else {
                map.put(c, 1);
            }
        }
    }
    map.put(256, 1);
}

class prioritize {
 public:
    bool operator() (const HuffmanNode* p1, const HuffmanNode* p2 ) {
        return p1->count > p2->count;
    }
};

//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmap &map) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, prioritize> pq;

    vector<int> keys = map.keys();

    for (int key : keys) {
        HuffmanNode* newNode = new HuffmanNode;
        newNode->character = key;
        newNode->count = map.get(key);
        newNode->zero = nullptr;
        newNode->one = nullptr;
        pq.push(newNode);
    }

    while (pq.size() > 1) {
        HuffmanNode* p1 = pq.top();
        pq.pop();
        HuffmanNode* p2 = pq.top();
        pq.pop();
        HuffmanNode* newNode = new HuffmanNode;
        newNode->character = 257;
        newNode->count = p1->count + p2->count;
        newNode->zero = p1;
        newNode->one = p2;
        pq.push(newNode);
    }
    return pq.top();
}

//
// creates a Encoding map of 1's and 0's and saves it into str using a map
// and BST tree
//
void _buildEncodingMap(mymap<int, string>& m, string str, HuffmanNode* n) {
    if (n->character != 257) {
        m.put(n->character, str);
        return;
    }
    _buildEncodingMap(m, str + "0", n->zero);
    _buildEncodingMap(m, str + "1", n->one);
}

//
// *This function builds the encoding map from an encoding tree.
//
mymap <int, string> buildEncodingMap(HuffmanNode* tree) {
    mymap <int, string> encodingMap;
    string str = "";
    if (tree == nullptr) {
        return encodingMap;
    }
    _buildEncodingMap(encodingMap, str, tree);
    return encodingMap;
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream& input, mymap <int, string> &encodingMap,
              ofbitstream& output, int &size, bool makeFile) {
    string str = "";
    char c;
    while (input.get(c)) {
        str += encodingMap.get((int)c);
    }
    str += encodingMap.get(256);
    if (makeFile) {
        for (char c : str) {
            if (c == '1') {
                output.writeBit(1);
            } else {
                output.writeBit(0);
            }
        }
    }
    size = str.length();
    return str;  // TO DO: update this return
}


//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {
    string str = "";
    HuffmanNode* root = encodingTree;
    while (!input.eof()) {
        int bit = input.readBit();
        if (encodingTree->character == 256) {
            break;
        } else if (bit == 1) {
            encodingTree = encodingTree->one;
        } else if (bit == 0) {
            encodingTree = encodingTree->zero;
        }
        if (encodingTree->character != 256 && encodingTree->character != 257) {
            str += encodingTree->character;
            encodingTree = root;
        }
    }
    for (char c : str) {
        output.put(c);
    }
    return str;  // TO DO: update this return
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
//
string compress(string filename) {
    hashmap frequencyMap;
    buildFrequencyMap(filename, true, frequencyMap);
    HuffmanNode* tree = buildEncodingTree(frequencyMap);
    mymap<int, string> encodingMap = buildEncodingMap(tree);
    ofbitstream output(filename + ".huf");
    output << frequencyMap;
    ifstream input(filename);
    freeTree(tree);
    int size = 0;
    return encode(input, encodingMap, output, size, true);
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function did.
//
string decompress(string filename) {
    ifbitstream input(filename);
    size_t pos = filename.find(".txt.huf");
    if ((int)pos >= 0) {
        filename = filename.substr(0, pos);
    }
    ofstream output(filename + "_unc.txt");
    hashmap frequencyMap;
    input >> frequencyMap;
    HuffmanNode* encodingTree = buildEncodingTree(frequencyMap);
    string str = decode(input, encodingTree, output);
    freeTree(encodingTree);
    return str;
}
