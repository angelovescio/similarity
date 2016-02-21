#include "main.h"

using namespace std; 
int map_count = 0;
CRITICAL_SECTION critical;
map<string,int,comparer> file_type_mapping;
vector<pair<ulong64,int>> hash_map;
vector<pair<ulong64,string>> file_map;
FILE* outfile = NULL;
FILE* outfile_translation = NULL;
uint8_t numThreads = 8;
int genbmp(uint8_t* &mainvector,int* filesize, const char* fname, int chunk_size=256)
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
	//if (mainvector[0] != 0x42 && mainvector[1] != 0x4d)
	if (mainvector[0] != 0x4d && mainvector[1] != 0x5a)
	{
		arrsize = 0;
	}
	CloseHandle(file);
err_exit:
	return arrsize;
}
bool sort_by_hash(const pair<ulong64,int> &lhs, const pair<ulong64,int> &rhs) { return lhs.first < rhs.first; }
bool sort_by_hash_file(const pair<ulong64,string> &lhs, const pair<ulong64,string> &rhs) { return lhs.first < rhs.first; }
bool sort_by_file_type(const pair<string,int> &lhs, const pair<string,int> &rhs) { return lhs.second < rhs.second; }
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
//int examine_proc(const char* fullpath,int x=32, int y=32, int z=3)
unsigned int __stdcall examine_proc(void * args)
{
	ProcArgs* pArgs = (ProcArgs*)(args);
	uint8_t* mainvector = NULL;
	int examine_length = 0;
	int filesize = 0;
	int ftype = 0;
	//may need to be 0xc00 since x*y*z*alpha == that
	int arr_size1 = genbmp(mainvector,&filesize,pArgs->fullpath,0x1000);
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
			/*string spath(pArgs->fullpath);
			string ext = spath.substr(spath.find_last_of(".") + 1);
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);*/
			//char * md5 = str2md5(pArgs->fullpath, strlen(pArgs->fullpath));
			int err2 = fprintf(outfile, "call insertHashes(%I64u,'%s');\n",
				hash1, pArgs->fullpath);
			
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
string SearchDrive( const string& strFile, const string& strFilePath, const bool& bRecursive, const bool& bStopWhenFound )
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
int AttemptInsert(string filetype)
{
	int retval = 0;
	map_count = file_type_mapping.size();
	file_type_mapping.insert(pair<string,int>(filetype,map_count));
	if(map_count < file_type_mapping.size())
	{
		retval = file_type_mapping.size();
	}
	else
	{
		map<string,int>::iterator it = file_type_mapping.find(filetype);
		retval = it->second;
	}
	return retval;
}
void PrintMappings()
{
	FILE* mapping_file =NULL;
	mapping_file = fopen(".\\mappings.txt","w+");
	if(mapping_file !=NULL)
	{
		vector<pair<string,int>> temp_map;
		map<string,int>::iterator it = file_type_mapping.begin();
		for(;it!=file_type_mapping.end();it++)
		{
			string s = it->first;
			int t = it->second;
			temp_map.push_back(pair<string,int>(s,t));
		}
		sort(temp_map.begin(),temp_map.end(),sort_by_file_type);
		vector<pair<string,int>>::iterator it2 = temp_map.begin();
		for(;it2!=temp_map.end();it2++)
		{
			string s = it2->first;
			int t = it2->second;
			//fprintf(mapping_file,"##%s##\t##%d##\n",s.c_str(),t);
		}
		if(mapping_file != NULL)
		{
			fclose(mapping_file);
		}
	}
}
//string DetectFileType(const char* filename)
//{
//	const char *magic_full;
//	magic_t magic_cookie;
//	/*MAGIC_MIME tells magic to return a mime of the file, but you can specify different things*/
//	magic_cookie = magic_open(MAGIC_CONTINUE);
//	if (magic_cookie == NULL) {
//		printf("unable to initialize magic library\n");
//		return "";
//	}
//	if (magic_load(magic_cookie, "magic.def") != 0) {
//		printf("cannot load magic database - %s\n", magic_error(magic_cookie));
//		magic_close(magic_cookie);
//		return "";
//	}
//	magic_full = magic_file(magic_cookie, filename);
//	string retval(magic_full);
//	magic_close(magic_cookie);
//	return retval;
//}
void OutputSearch()
{
	sort(hash_map.begin(),hash_map.end(),sort_by_hash);
	for(vector<pair<ulong64,int>>::iterator it = hash_map.begin();
		it != hash_map.end();it++)
	{
		if(outfile != NULL)
		{
			ulong64 u = it->first;
			int t = it->second;
			fprintf(outfile,"%I64u\n",u);
			fprintf(outfile,"%d\n",t);
		}
	}
	sort(file_map.begin(),file_map.end(),sort_by_hash_file);
	for(vector<pair<ulong64,string>>::iterator it = file_map.begin();
		it != file_map.end();it++)
	{
		if(outfile_translation != NULL)
		{
			ulong64 u = it->first;
			string t = it->second;
			fprintf(outfile_translation,"%I64u\n",u);
			fprintf(outfile_translation,"%s\n",t.c_str());
		}
	}
}
/*
int connectMysql()
{
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;

		/* Create a connection 
		driver = get_driver_instance();
		con = driver->connect("tcp://10.0.0.10:3306", "root", "password");
		/* Connect to the MySQL test database 
		con->setSchema("similarity");

		stmt = con->createStatement();
		res = stmt->executeQuery("SELECT 'Hello World!' AS _message");
		while (res->next()) {
			cout << "\t... MySQL replies: ";
			/* Access column data by alias or column name 
			cout << res->getString("_message") << endl;
			cout << "\t... MySQL says it again: ";
			/* Access column fata by numeric offset, 1 is the first column 
			cout << res->getString(1) << endl;
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
	}
	return 0;
}
*/
int main(int argc, char* argv[]) { 
	//connectMysql();
	outfile = fopen(".\\outfile.txt","w+");
	outfile_translation = fopen(".\\outfile_translation.txt","w+");
	if(outfile != NULL)
	{
		fprintf(outfile,"use similarity;\n");
	}
	SearchDrive("","C:\\Users\\vesh\\Documents\\Visual Studio 2015\\Projects\\similarity\\corpus",true,false);
	//OutputSearch();
	PrintMappings();
	if(outfile != NULL)
	{
		fclose(outfile);
	}
	if(outfile_translation != NULL)
	{
		fclose(outfile_translation);
	}
	return 0; 
}
