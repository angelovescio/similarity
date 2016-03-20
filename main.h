#include <iostream> 
#include "bmp.h"
#include "md5.h"
#include "MemWalk.h"
#include "pHash.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <process.h>
#include <windows.h>
#include <stdio.h>
#include "optparse.h"

using namespace std;
enum  optionIndex { UNKNOWN, HELP, INFILE
	, PORT
	, SERVER
	, USER
	, PASS
	, DB
	, EXE
	, RDIR
	, PID
};
const option::Descriptor usage[] =
{
	{ UNKNOWN, 0,"" , ""    ,option::Arg::None, "USAGE: example [options]\n\n"
	"Options:" },
	{ HELP,    0,"h" , "help",option::Arg::None, "  --help, -h  \tPrint usage and exit." },
	{ INFILE,    0,"f", "file",option::Arg::Optional, "  --file, -f  \tOutput SQL file for DB query/insert" },
	{ PID,    0,"i", "pid",option::Arg::Optional, "  --pid, -i  \tProcess ID" },
	{ PORT,    0,"p", "port",option::Arg::Optional, "  --port, -p  \tMySQL port." },
	{ SERVER,    0,"s", "server",option::Arg::Optional, "  --server, -s  \tMySQL Server IP" },
	{ USER,    0,"u", "user",option::Arg::Optional, "  --user, -u  \tMySQL Database username" },
	{ PASS,    0,"a", "password",option::Arg::Optional, "  --password, -a  \tMySQL Password" },
	{ DB,    0,"d", "database",option::Arg::Optional, "  --database, -d  \tMySQL database name" },
	{ EXE,    0,"e", "executable",option::Arg::Optional, "  --executable, -e  \tExe/DLL/SO to hash and compare" },
	{ RDIR,    0,"r", "recursive",option::Arg::Optional, "  --recursive, -r  \tDirectory to recurse and upload to db" },
	{ UNKNOWN, 0,"" ,  ""   ,option::Arg::None, "\nExamples:\n"
	"  example --unknown -- --this_is_no_option\n"
	"  example -unk --plus -ppp file1 file2\n" },
	{ 0,0,0,0,0,0 }
};

int AttemptInsert(string filetype);
void PrintMappings();
unsigned int __stdcall examine_proc(void * args);
//string DetectFileType(const char* filename);
struct comparer
{
    public:
    bool operator()(const string x, const string y)
    {
         return x.compare(y)<0;
    }
};
struct ProcArgs
{
	char fullpath[3200];
	int x;
	int y;
	int z;
	int cbMainVector;
	uint8_t* mainvector;
};
struct HashList{
	ulong64 hash1;
	int chunk1;
	ulong64 hash2;
	int chunk2;
	int hamming_distance;
};
