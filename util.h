//
// STARTER CODE: util.h
// Author: Reem Allam
// 
// 
//
#include <iostream>
#include <fstream>
#include <map>
#include <queue>          // std::priority_queue
#include <vector>         // std::vector
#include <functional>     // std::greater
#include <string>
#include<algorithm>
#include "bitstream.h"
#include "hashmap.h"
#pragma once

struct HuffmanNode {
    int character;
    int count;
    HuffmanNode* zero;
    HuffmanNode* one;
};

//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode* node) {
    
    if (node) {
        freeTree(node->zero);
        freeTree(node->one);
        delete node;
    }
}

//
// *This function builds the frequency map.  
//  If isFile is true, read from the file with name filename.  
//  If isFile is false, read directly from string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmap &map) {
    if(isFile){
        ifstream file(filename);
        if(file.is_open()){
            int ch;
            while((ch = file.get()) != EOF){
                map.put(ch,map.get(ch)+1);
            }
            map.put(256,1);
           file.close();
        }
        else{
            cout<<"Error: Unable to open file" << filename << "."<<endl;
        }
    }
    else{
     istringstream input(filename);
     int ch;
      while((ch = input.get()) != EOF){
        map.put(ch, map.get(ch)+1);
      }
      map.put(256,1);
    }
}

//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmap &map) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>,function<bool(HuffmanNode*, HuffmanNode*)>> pq(
        [](HuffmanNode* a, HuffmanNode* b){
            return a-> count > b -> count;
        }
    );

    for(int key : map.keys()){
        int count = map.get(key);
        HuffmanNode* node = new HuffmanNode;
        node -> character = key;
        node -> count = count;
        node-> zero = nullptr;
        node -> one = nullptr;

        pq.push(node);
    }

    while(pq.size() > 1){
        HuffmanNode* left = pq.top();
        pq.pop();
        HuffmanNode* right = pq.top();
        pq.pop();

        HuffmanNode* intNode = new HuffmanNode;
        intNode -> character = NOT_A_CHAR;
        intNode -> count = left-> count + right-> count;
        intNode-> zero = left;
        intNode-> one = right;

        pq.push(intNode);
    }
    return pq.top();  
}

//
// *This function builds the encoding map from an encoding tree.
//
// this function traverses the Huffman tree and generates the encoding for each character.
void buildEncodingMapHelper(HuffmanNode* node, string prefix, map<int,string>& encodingMap){
    if(node == nullptr){
        return;
    }
    if(node-> character != NOT_A_CHAR){
        encodingMap[node->character] = prefix;
    }

    buildEncodingMapHelper(node->zero, prefix + "0", encodingMap);
    buildEncodingMapHelper(node -> one, prefix + "1", encodingMap);
}

// This function builds an encoding map from the Huffman tree.
// The encoding map contains the binary encoding for each character.
map<int,string> buildEncodingMap(HuffmanNode* tree) {
    map<int,string> encodingMap;
    
    buildEncodingMapHelper(tree,"", encodingMap);
    
    return encodingMap; 
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream& input, map<int,string> &encodingMap,ofbitstream& output, int &size, bool makeFile) { 
    stringstream encodedData;

    int ch;
    while((ch = input.get()) != EOF){
        string code = encodingMap[ch];
        for(char c : code){
            int bit = c - '0';
            output.writeBit(bit);
            encodedData << bit;
            size++;
        }
    }

    string eofCode = encodingMap[256];
    for(char c : eofCode){
            int bit = c - '0';
            output.writeBit(bit);
            encodedData << bit;
            size++;
        }

    if(makeFile){
        output.close();
    }

    return encodedData.str(); 
}


//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {
    string decodedData;

    HuffmanNode* currNode = encodingTree;
    while(!input.eof()){
        int bit = input.readBit();
        if(bit == 1){
            currNode = currNode->one;
        }
        if(bit == 0){
            currNode = currNode->zero;
        }
        if(currNode->character == PSEUDO_EOF){
                break;
            }
        if(currNode->character != NOT_A_CHAR){
            decodedData += currNode -> character;
            currNode = encodingTree;
        }
    }
    
    return decodedData;
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
    map<int,string> Map;
    string encodedTree;
    int size = 0;

    ifstream inFile(filename);
    string outputFile = filename;
    outputFile += ".huf";
    ofbitstream outputBits(outputFile);

    buildFrequencyMap(filename, true, frequencyMap);
    
    
    HuffmanNode* encodingTree = buildEncodingTree(frequencyMap);

    Map = buildEncodingMap(encodingTree);
    outputBits << frequencyMap;
    encodedTree = encode(inFile, Map, outputBits, size, true);
    freeTree(encodingTree);
    
    
    return encodedTree;
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
// function does.
//
string decompress(string filename) {
    hashmap frequencyMap;

    ifbitstream input(filename);
    input >> frequencyMap;

    int original = filename.find('.');
    string prefix = filename.substr(0,original);
    string outputFile = prefix + "_unc.txt";
    ofstream output(outputFile);

    HuffmanNode* encodingTree = buildEncodingTree(frequencyMap);
    string decodedData = decode(input, encodingTree, output);
    freeTree(encodingTree);
    
    return decodedData;
}