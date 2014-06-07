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

typedef struct HashList{
	ulong64 hash1;
	int chunk1;
	ulong64 hash2;
	int chunk2;
	int hamming_distance;
};

#endif