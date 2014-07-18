#ifndef NODE_H
#define NODE_H
#include <iostream> 
#include "bmp.h"
#include "pHash.h"
#include "node.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <windows.h>
#include <magic.h>
#include <stdio.h>

using namespace std;
int AttemptInsert(string filetype);
void PrintMappings();
string DetectFileType(const char* filename);
struct comparer
{
    public:
    bool operator()(const string x, const string y)
    {
         return x.compare(y)<0;
    }
};

struct HashList{
	ulong64 hash1;
	int chunk1;
	ulong64 hash2;
	int chunk2;
	int hamming_distance;
};

#endif