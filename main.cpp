#include "main.h"
#include <cppconn\driver.h>
#include <cppconn\exception.h>
#include <cppconn\resultset.h>
#include <cppconn\statement.h>
#include "include\mysql_connection.h"

using namespace std; 

int map_count = 0;
CRITICAL_SECTION critical;
map<string,int,comparer> file_type_mapping;
vector<pair<ulong64,int>> hash_map;
vector<pair<ulong64,string>> file_map;
FILE* outfile = NULL;
FILE* outfile_translation = NULL;
uint8_t numThreads = 8;
/// <summary>
/// Genbmps the specified mainvector.
/// </summary>
/// <param name="mainvector">The mainvector.</param>
/// <param name="filesize">The filesize.</param>
/// <param name="fname">The fname.</param>
/// <param name="chunk_size">The chunk_size.</param>
/// <returns></returns>
int genbmp(uint8_t* &mainvector, int* filesize, const char* fname, int chunk_size = 256)
{
	HANDLE file;
	//int size = 0;
	int arrsize = 0;
	//string ftype = "";// DetectFileType(fname);
	//*filetype = AttemptInsert(ftype);
	//strcpy(*fsType,ftype.c_str());
	file = CreateFile(fname,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);  //Sets up the new bmp to be written to
	DWORD lphigh;
	*filesize = GetFileSize(file,&lphigh);

	if(*filesize < chunk_size)
	{
		goto err_exit;
	}
	mainvector = (uint8_t*)calloc(*filesize, sizeof(uint8_t));
	if (mainvector == NULL)
	{
		arrsize = 0;
		goto err_exit;
	}
	//mainvector = (uint8_t*)calloc(chunk_size, sizeof(uint8_t));
	ReadFile(file,mainvector,*filesize,&lphigh,NULL);
	//arrsize = chunk_size / sizeof(uint8_t);
	arrsize = (*filesize) /chunk_size;
	CloseHandle(file);
err_exit:
	return arrsize;
}
/// <summary>
/// Sort_by_hashes the specified LHS.
/// </summary>
/// <param name="lhs">The LHS.</param>
/// <param name="rhs">The RHS.</param>
/// <returns></returns>
bool sort_by_hash(const pair<ulong64, int> &lhs, const pair<ulong64, int> &rhs) { return lhs.first < rhs.first; }
/// <summary>
/// Sort_by_hash_files the specified LHS.
/// </summary>
/// <param name="lhs">The LHS.</param>
/// <param name="rhs">The RHS.</param>
/// <returns></returns>
bool sort_by_hash_file(const pair<ulong64, string> &lhs, const pair<ulong64, string> &rhs) { return lhs.first < rhs.first; }
/// <summary>
/// Sort_by_file_types the specified LHS.
/// </summary>
/// <param name="lhs">The LHS.</param>
/// <param name="rhs">The RHS.</param>
/// <returns></returns>
bool sort_by_file_type(const pair<string, int> &lhs, const pair<string, int> &rhs) { return lhs.second < rhs.second; }
/// <summary>
/// </summary>
/// <param name="str">The string.</param>
/// <param name="length">The length.</param>
/// <returns></returns>
char *str2md5(char *str, int length) {
	int n;
	MD5_CTX c;
	unsigned char digest[16];
	char *out = (char*)malloc(33);

	MD5Init(&c);

	while (length > 0) {
		if (length > 512) {
			MD5Update(&c, (unsigned char*)str, 512);
		} else {
			MD5Update(&c, (unsigned char*)str, length);
		}
		length -= 512;
		str += 512;
	}

	MD5Final(digest, &c);

	for (n = 0; n < 16; ++n) {
		snprintf(&(out[n * 2]), 16 * 2, "%02x", (unsigned int)digest[n]);
	}

	return out;
}

/*
params:
	fullpath - path to file
	x - width
	y - height
	z - depth
Increase the aperture by fiddlign with x,y
*/
/// <summary>
/// Examine_procs the specified arguments.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
unsigned int __stdcall examine_proc(void * args)
{
	ProcArgs* pArgs = (ProcArgs*)(args);
	uint8_t* mainvector = pArgs->mainvector;
	int examine_length = 0;
	int filesize = 0;
	int ftype = 0;
	//may need to be 0xc00 since x*y*z*alpha == that
	int arr_size1 = pArgs->cbMainVector; //genbmp(mainvector, &filesize, pArgs->fullpath, 0xc00);
	if (arr_size1 == 0)
	{
		return 0;
	}
	int mover = pArgs->x*pArgs->y*pArgs->z;
	
	//BMP data length calc is width*height*3 where three is the number of of color vals in a pixel
	for(int i=0,j=0;j<arr_size1;i=i+mover,j++)
	{
		if(NULL != mainvector)
		{
			uint8_t* p_main = (mainvector)+i;
			int bufferLeft = filesize - i;
			ulong64 hash1 =0;
			uint8_t* buffer = (uint8_t*)malloc(mover);
			memset(buffer, 0, mover);
			if (bufferLeft < mover) {
				memcpy_s(buffer, bufferLeft, p_main, bufferLeft);
			}
			else {
				memcpy_s(buffer, mover, p_main, mover);
			}
			
			cimg_library::CImg<uint8_t> img2(buffer, pArgs->x, pArgs->y,1, pArgs->z,1);
			
			int err1 = ph_dct_imagehash_from_buffer(img2, hash1,pArgs->fullpath);
			//null hash == 54086765383280
			if (hash1 == 54086765383280)
			{
				free(buffer);
				continue;
			}
			/*
			INSERT INTO `hashes` (`hash`) VALUES ('%I64u') \nON DUPLICATE KEY UPDATE id=LAST_INSERT_ID(id), `hash`='%I64u';\nINSERT INTO `paths` (`path`,`hashid`,`hashpath`) VALUES ('%s',LAST_INSERT_ID(),md5('%s'));\n
			*/
			string spath(pArgs->fullpath);
			std::replace(spath.begin(), spath.end(), '\\', '/');
			/*string ext = spath.substr(spath.find_last_of(".") + 1);
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);*/
			//char * md5 = str2md5(pArgs->fullpath, strlen(pArgs->fullpath));
			int err2 = fprintf(outfile, "call insertHashes(%I64u,'%s');\n",
				hash1, spath.c_str());
			
			//fflush(outfile);
			free(buffer);
		}

	}
	if(mainvector != NULL)
	{
		free(mainvector);
	}
	return 0;
}
/// <summary>
/// Examine_proc_no_threads the specified arguments.
/// </summary>
/// <param name="args">The arguments.</param>
/// <param name="hashes">The hashes.</param>
/// <returns></returns>
ulong64 examine_proc_no_thread(void * args, vector<ulong64>& hashes, vector<uint8_t>& byteBuffer)
{
	ProcArgs* pArgs = (ProcArgs*)(args);
	ulong64 retval = 0;
	uint8_t* mainvector = &byteBuffer[0];
	int examine_length = 0;
	int filesize = 0;
	int ftype = 0;
	//may need to be 0xc00 since x*y*z*alpha == that
	int mover = pArgs->x*pArgs->y*pArgs->z;
	int arr_size1 = byteBuffer.size()/mover;//genbmp(mainvector, &filesize, pArgs->fullpath, 0xc00);
	if (arr_size1 == 0)
	{
		return 0;
	}
	

	//BMP data length calc is width*height*3 where three is the number of of color vals in a pixel
	for (int i = 0, j = 0; j<arr_size1; i = i + mover, j++)
	{
		if (NULL != mainvector)
		{
			uint8_t* p_main = (mainvector)+i;
			int bufferLeft = byteBuffer.size() - i;
			ulong64 hash1 = 0;
			uint8_t* buffer = (uint8_t*)malloc(mover);
			memset(buffer, 0, mover);
			if (bufferLeft < mover) {
				memcpy_s(buffer, bufferLeft, p_main, bufferLeft);
			}
			else {
				memcpy_s(buffer, mover, p_main, mover);
			}

			cimg_library::CImg<uint8_t> img2(buffer, pArgs->x, pArgs->y, 1, pArgs->z, 1);

			int err1 = ph_dct_imagehash_from_buffer(img2, hash1, pArgs->fullpath);
			//null hash == 54086765383280
			if (hash1 == 54086765383280)
			{
				free(buffer);
				continue;
			}
			/*
			INSERT INTO `hashes` (`hash`) VALUES ('%I64u') \nON DUPLICATE KEY UPDATE id=LAST_INSERT_ID(id), `hash`='%I64u';\nINSERT INTO `paths` (`path`,`hashid`,`hashpath`) VALUES ('%s',LAST_INSERT_ID(),md5('%s'));\n
			*/
			string spath(pArgs->fullpath);
			std::replace(spath.begin(), spath.end(), '\\', '/');
			/*string ext = spath.substr(spath.find_last_of(".") + 1);
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);*/
			//char * md5 = str2md5(pArgs->fullpath, strlen(pArgs->fullpath));
			int err2 = fprintf(outfile, "call insertHashes(%I64u,'%s');\n",
				hash1, spath.c_str());
			hashes.push_back(hash1);
			retval = hash1;
			//fflush(outfile);
			free(buffer);
		}

	}
	return retval;
}
/// <summary>
/// Searches the drive.
/// </summary>
/// <param name="strFile">The string file.</param>
/// <param name="strFilePath">The string file path.</param>
/// <param name="bRecursive">The b recursive.</param>
/// <param name="bStopWhenFound">The b stop when found.</param>
/// <returns></returns>
string SearchDrive(const string& strFile, const string& strFilePath, const bool& bRecursive, const bool& bStopWhenFound)
{
	string strFoundFilePath;
	WIN32_FIND_DATA file;

	string strPathToSearch = strFilePath;
	HANDLE hFile = FindFirstFile((strPathToSearch.append("\\*")).c_str(), &file);
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		do
		{
			string strPathToAppend = strFilePath;

			string strTheNameOfTheFile = file.cFileName;

			// It could be a directory we are looking at
			// if so look into that dir
			if ( file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if( bRecursive )
				{
					if(strcmp(file.cFileName,".") !=0 && strcmp(file.cFileName,"..") !=0)
					{
						strFoundFilePath = SearchDrive( strFile, strPathToAppend.append("\\"+strTheNameOfTheFile), bRecursive, bStopWhenFound );
					}
					if ( !strFoundFilePath.empty() && bStopWhenFound )
					{
						break;
					}
				}
			}
			else
			{
				strPathToAppend.append("\\");
				ProcArgs* p = (ProcArgs*)malloc(sizeof(ProcArgs));
				memset(p, 0, sizeof(ProcArgs));
				const char* fullPath = strPathToAppend.append(file.cFileName).c_str();
				memcpy_s(p, sizeof(p->fullpath), fullPath, strlen(fullPath));
				uint8_t* mainvector = NULL;
				int filesize = 0;
				p->cbMainVector = genbmp(mainvector, &filesize, fullPath,1024);
				p->mainvector = mainvector;
				p->x = 32;
				p->y = 32;
				p->z = 3;
				HANDLE eHandle;
				eHandle = (HANDLE)_beginthreadex(0, 0, &examine_proc, (void*)p, 0, 0);
				WaitForSingleObject(eHandle, INFINITE);
				CloseHandle(eHandle);
				free(p);
				if ( strTheNameOfTheFile.compare(strFile) )
				{
					strFoundFilePath = strPathToAppend.append(strFile);
					if ( bStopWhenFound )
						break;
				}
			}
		}
		while ( FindNextFile(hFile, &file) );
		FindClose(hFile);
	}
	else
	{
		printf("Nothing for %s!\n", strFilePath.c_str());
	}
	return strFoundFilePath;
}

/// <summary>
/// Checks the hash.
/// </summary>
/// <param name="filepath">The filepath.</param>
/// <param name="name">The name.</param>
/// <param name="db">The database.</param>
/// <param name="user">The user.</param>
/// <param name="pass">The pass.</param>
/// <param name="port">The port.</param>
/// <returns></returns>
int checkHash(char filepath[1024], char name[1024], char db[1024] ="similarity", char user[1024] ="root", char pass[1024] ="password", int port = 3306, int pid=-1)
{
	ProcArgs* p = new ProcArgs();
	p->x = 32;
	p->y = 32;
	p->z = 3;
	ulong64 id = 1000;
	int last_err = 1000;
	strcpy(p->fullpath ,filepath);
	char host[1024] = "";
	vector<ulong64> hashes;
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	CMemWalk walk;
	vector<uint8_t> byteBuffer;
	walk.memWalk(hProc,byteBuffer);
	ulong64 hash = examine_proc_no_thread(p,hashes,byteBuffer);
	//vector<ulong64, int> results;
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt,*stmt2,*stmt3;
		sql::ResultSet *res,*res2;
		sprintf_s(host, "tcp://%s:%d", name, port);
		driver = get_driver_instance();
		//con = driver->connect("tcp://192.168.56.102:3306", "root", "password");
		con = driver->connect(host, user, pass);
		con->setSchema(db);
		stmt = con->createStatement();
		res = stmt->executeQuery("SELECT hash,id FROM hashes;");
		vector<ulong64>::iterator iter;
		
		ulong64 hashSql = 0;
		ulong64 hashSqlTemp = 0;
		char path[260];
		for (iter = hashes.begin(); iter != hashes.end(); iter++)
		{	
			ulong64 hash = *iter;
			while (res->next()) {
				hashSql = res->getUInt64(1);
				
				int err = ph_hamming_distance(hash, hashSql);
				if (err < last_err)
				{
					last_err = err;
					id = res->getUInt64(2);
					hashSqlTemp = hashSql;
					char qBuffer[1024] = "";
					sprintf(qBuffer, "SELECT pathid FROM paths WHERE hashid='%llu' limit 1;", id);
					res2 = stmt->executeQuery(qBuffer);
					ulong64 pathid = 0;
					while (res2->next())
					{
						pathid = res2->getUInt64(1);
					}

					memset(qBuffer, 0, sizeof(qBuffer));
					sprintf(qBuffer, "SELECT path FROM hashpaths WHERE id='%llu' limit 1;", pathid);
					res2 = stmt->executeQuery(qBuffer);
					
					while (res2->next())
					{
						sprintf(path, "%s", res2->getString("Path").c_str());
					}
					cout << "Better match with confidence "<< last_err <<" is path: " << path << endl;
				}
			}
			
			
			cout << "Hash " << hash << " is most like " << id << " with hash " << hashSqlTemp << " and path " << path << endl;
			last_err = 1000;
			res->beforeFirst();
			
		}
		delete res;
		delete stmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "	<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		exit(-1);
	}
	return 0;
}
int main(int argc, char *argv[]) {
	int option_char, i, rc = 0;
	int pid = -1;
	int port = 3306;
	char user[1024] ="root";
	char pass[1024] = "password";
	char db[1024] = "similarity";
	char file[1024] = "outfile.txt";
	char serv[1024] = "";
	char exe[1024] = "";
	char dir[1024] = "";
	const int bmax = 100;
	const int omax = 100;
	argc -= (argc>0); argv += (argc>0); // skip program name argv[0] if present
	option::Stats  stats(usage, argc, argv);

	option::Option options[omax];
	option::Option buffer[bmax];
	option::Parser parse(usage, argc, argv, options, buffer);
	for (int i = 0; i < parse.optionsCount(); i++) {
		option::Option * opt = buffer[i];
		switch (opt->index()) {
		case PID: // id of process dump to check against database
			pid = atoi(opt->arg);
			break;
		case PORT: // id of process dump to check against database
			port = atoi(opt->arg);
			break;
		case INFILE: // file to write SQL query to
			strcpy_s(file, opt->arg);
			break;
		case EXE: // exe to compare to db
			strcpy_s(exe, opt->arg);
			break;
		case RDIR: // recurse and upload results to db for reference
			strcpy_s(dir, opt->arg);
			break;
		case SERVER: // MySQL server hostname
			strcpy_s(serv, opt->arg);
			break;
		case USER: // MySQL username
			strcpy_s(user, opt->arg);
			break;
		case PASS: // MySQL db password
			strcpy_s(pass, opt->arg);
			break;
		case DB: // MySQL db
			strcpy_s(db, opt->arg);
			break;
		case HELP: // help
			fprintf(stdout, "%s", opt->arg);
			exit(0);
			break;
		default:
			//fprintf(stderr, "%s", USAGE);
			exit(1);
		}
	}
	
	if (file != "") {
		outfile = fopen(file, "w+");
	}
	if (outfile != NULL) {
		if (db == "") {
			fprintf(outfile, "use similarity;\n");
		}
		else {
			fprintf(outfile, "use %s;\n", db);
		}
	}
	if (strcmp(exe,"") != 0 && strcmp(serv,"") != 0 ) {
		checkHash(exe,serv,db,user,pass);
	}
	if (pid != -1 && strcmp(serv, "") != 0) {
		checkHash(exe, serv, db, user, pass,port,pid);
	}
	if (strcmp(dir,"")!=0) {
		SearchDrive("", dir, true, false);
	}
	//OutputSearch();
	if (outfile != NULL) {
		fclose(outfile);
	}
}