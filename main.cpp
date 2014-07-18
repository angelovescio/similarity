#include "main.h"

using namespace std; 
int map_count = 0;
map<string,int,comparer> file_type_mapping;


int genbmp(uint8_t* &mainvector,const char* fname, int chunk_size=256)
{
	HANDLE file;
	int size = 0;
	int arrsize = 0;
	string ftype = DetectFileType(fname);
	AttemptInsert(ftype);
	file = CreateFile(fname,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);  //Sets up the new bmp to be written to
	DWORD lphigh;
	size = GetFileSize(file,&lphigh);
	if(size < chunk_size)
	{
		goto err_exit;
	}
	mainvector = (uint8_t*)calloc(chunk_size,sizeof(char));
	ReadFile(file,mainvector,chunk_size,&lphigh,NULL);
	arrsize = chunk_size/sizeof(uint8_t);
	CloseHandle(file);
err_exit:
	return arrsize;
}
bool sort_by_hash(const pair<ulong64,int> &lhs, const pair<ulong64,int> &rhs) { return lhs.first < rhs.first; }
int examine_proc(const char* fullpath,const char* filename)
{
	uint8_t* mainvector = NULL;
	//float* slavevector;
	int examine_length = 0;
	//vec of ulong64 hashes and the chunk they appear in
	std::vector<std::pair<ulong64,int>> *v = new vector<std::pair<ulong64,int>>();
	int arr_size1 = genbmp(mainvector,fullpath);
	int x = 32;//s/0x64;
	int y = 32;//s/0x64;
	int mover = 0;//arr_size1/chunk_size;
	if(arr_size1 <= x*y*3)
	{
		mover = arr_size1;
	}
	else
	{
		mover = x*y*3;
	}
	char chars[] = ".";
	string str(filename);
	for (unsigned int i = 0; i < strlen(chars); ++i)
	{
		// you need include <algorithm> to use general algorithms like std::remove()
		str.erase (std::remove(str.begin(), str.end(), chars[i]), str.end());
	}
	//BMP data length calc is width*height*3 where three is the number of of color vals in a pixel
	for(int i=0,j=0;i<arr_size1;i=i+mover,j++)
	{
		uint8_t* p_main = (mainvector)+i;
		if(NULL != p_main)
		{
			uint8_t* temp;
			size_t outsize;
			ulong64 hash1 =0;
			ulong64 hash2 =0;

			//char* fname = new char[260];
			cimg_library::CImg<uint8_t> img2(p_main,x,y,1,3,1);
			//cimg_library::CImgList<uint8_t> llimg2 =  img2.get_FFT(false);
			int err1 = ph_dct_imagehash_from_buffer(img2,hash1);
			//cimg_library::CImgList<uint8_t>::iterator it;
			//int fft_counter=0;
			//for(it=llimg2.begin();it!=llimg2.end();++it)
			//{
			//	cimg_library::CImg<uint8_t> im = *it;
			//	err1 = ph_dct_imagehash_from_buffer(im,hash2);
			//	/*sprintf(fname,"images\\%s_%x_%llx_%d.%s",
			//		str.c_str(),i,hash2,fft_counter,"bmp");*/
			//	//im.save(fname);
			//	fft_counter++;
			//}
			//if(hash1 !=0)
			//{
			//	//Save the image here when you are done for the x-compares
			//	//sprintf(fname,"images\\%s_%x_%llx.%s",
			//	//	str.c_str(),i,hash1,"bmp");
			//	//img2.save(fname);
			//	//remove(tempname);
			//	std::pair<ulong64,int> tempp(hash1,i);
			//	v->push_back(tempp);

			//}
			//free(temp);
		}

	}
	if(mainvector != NULL)
	{
		free(mainvector);
	}
	//std::sort(v->begin(),v->end(),sort_by_hash);
	//ulong64 last_val =0;
	//int last_chunk = 0;
	//FILE* fi;
	//fi = fopen("C:\\Users\\vesh\\Documents\\Visual Studio 2012\\Projects\\pHash-0.9.4\\images\\mapping.txt","w");
	//vector<HashList> *v_hash = new vector<HashList>();
	//if(fi)
	//{
	//	for (std::vector<pair<ulong64,int>>::iterator it=(*v).begin(); it!=(*v).end(); ++it)
	//	{
	//		int hm = ph_hamming_distance(last_val,it->first);
	//		HashList* h = (HashList*)malloc(sizeof(HashList));
	//		h->hamming_distance=hm;
	//		h->hash1=last_val;
	//		h->hash2 = it->first;
	//		h->chunk1 = last_chunk;
	//		h->chunk2 = it->second;
	//		v_hash->push_back(*h);

	//		fprintf(fi,"Hamming between %llx and %llx was %d %s",last_val,it->first,hm,"\n");

	//		last_val = (*it).first;
	//		last_chunk = (*it).second;
	//	}
	//	for (std::vector<HashList>::iterator it=(*v_hash).begin(); it!=(*v_hash).end(); ++it){
	//		char* fname_last = new char[260];
	//		char* fname_last_copy = new char[260];
	//		char* fname_it = new char[260];
	//		char* fname_it_copy = new char[260];
	//		HashList h = *it;

	//		sprintf(fname_last,"images\\%llx_%x_resave.%s",h.hash1,h.chunk1,"bmp");
	//		sprintf(fname_last_copy,"images\\%d\\%llx_%x_resave.%s",h.hamming_distance,h.hash1,h.chunk1,"bmp");
	//		sprintf(fname_it,"images\\%llx_%x_resave.%s",h.hash2,h.chunk2,"bmp");
	//		sprintf(fname_it_copy,"images\\%d\\%llx_%x_resave.%s",h.hamming_distance,h.hash2,h.chunk2,"bmp");
	//		/*CopyFile(fname_it,fname_it_copy,false);
	//		CopyFile(fname_last,fname_last_copy,false);*/
	//	}
	//	fclose(fi);
	//}
	//free(slavevector);
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
						strFoundFilePath = SearchDrive( strFile, strPathToAppend.append(strTheNameOfTheFile), bRecursive, bStopWhenFound );
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
				examine_proc(strPathToAppend.append(file.cFileName).c_str(),file.cFileName);
				if ( strTheNameOfTheFile.compare(strFile) )
				{
					strFoundFilePath = strPathToAppend.append(strFile);

					/// TODO
					// ADD TO COLLECTION TYPE

					if ( bStopWhenFound )
						break;
				}
			}
		}
		while ( FindNextFile(hFile, &file) );
	}
	return strFoundFilePath;
}
void AttemptInsert(string filetype)
{
	map_count = file_type_mapping.size();
	file_type_mapping.insert(pair<string,int>(filetype,map_count));
}
void PrintMappings()
{
	FILE* mapping_file =NULL;
	mapping_file = fopen("C:\\mappings.txt","w+");
	if(mapping_file !=NULL)
	{
		map<string,int>::iterator it = file_type_mapping.begin();
		for(;it!=file_type_mapping.end();it++)
		{
			string s = it->first;
			int t = it->second;
			fprintf(mapping_file,"##%s##\t##%d##\n",s.c_str(),it->second);
		}
		if(mapping_file != NULL)
		{
			fclose(mapping_file);
		}
	}
}
string DetectFileType(const char* filename)
{
	const char *magic_full;
	magic_t magic_cookie;
	/*MAGIC_MIME tells magic to return a mime of the file, but you can specify different things*/
	magic_cookie = magic_open(MAGIC_MIME);
	if (magic_cookie == NULL) {
		printf("unable to initialize magic library\n");
		return "";
	}
	if (magic_load(magic_cookie, "magic.def") != 0) {
		printf("cannot load magic database - %s\n", magic_error(magic_cookie));
		magic_close(magic_cookie);
		return "";
	}
	magic_full = magic_file(magic_cookie, filename);
	string retval(magic_full);
	magic_close(magic_cookie);
	return retval;
}
int main(int argc, char* argv[]) { 
	SearchDrive("","C:\\Users\\",true,false);
	PrintMappings();
	return 0; 
}
