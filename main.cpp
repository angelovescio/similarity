#include "main.h"

using namespace std; 
unsigned char lookup_color(int16_t c)
{
	if (c < 0)
		return 0;
	if (c < 64) 
		return c * 4;
	if (c <= 192)
		return 0xff;
	if (c < 256)
		return 0x100 - ((c - 192) * 4);

	return 0;
}


int write_byte(uint8_t *s, unsigned char c,int offset)
{
	RGBQUAD r;

	if (0 == c)
		r.rgbBlue = r.rgbRed = r.rgbGreen = 0;
	else if (0xff == c)
		// RBF - What happens if 0xff characters are just dark red? 
		r.rgbBlue = r.rgbRed = r.rgbGreen = 0xff;
	else
	{
		r.rgbBlue  = lookup_color( (int16_t)c+128);
		r.rgbGreen = lookup_color( (int16_t)c );
		r.rgbRed   = lookup_color( (int16_t)c-128);
	}
	uint32_t place = (uint32_t)(s);
	uint32_t write_place = place+offset;
	memcpy((void*)write_place,&r,sizeof(RGBQUAD));
	return FALSE;
}


void pad_image(uint8_t *s, uint32_t val,int offset) { 
	switch (val % 4)  { 
		// I'm not entirely sure why this works, but it does. 
		// Note that there are deliberately no breaks in this switch.
	case 3:
		memset(&s[offset],0,1);
	case 2:
		memset(&s[offset],0,1);
	case 1:
		memset(&s[offset],0,1);
	}
}
unsigned char padding(void)
{
	return RGB_PADDING;
}
int write_bmp_info(int image_width,int image_height,uint8_t* &inbuffer,uint8_t* &outbuffer,size_t arr_size,size_t &outsize)
{
	BITMAPINFOHEADER b;


	BITMAPFILEHEADER h;

	h.bfType = BF_TYPE;
	h.bfSize = byteswap32((arr_size+sizeof(BITMAPINFOHEADER)+sizeof(BITMAPFILEHEADER)));
	h.bfReserved1 = 0;

	// The offset in the file where RGB image data begins 
	h.bfOffBits   = byteswap32(0x36);
	// Size of the this header information
	b.biSize = byteswap32(HEADERINFOSIZE);  


	b.biWidth = byteswap32(image_width);
	b.biHeight = byteswap32(image_height);


	b.biPlanes      = byteswap16(1);
	b.biBitCount    = byteswap16(24);
	b.biCompression = BI_RGB;

	// How much RGB data follows this header 
	b.biSizeImage = byteswap32((image_width * image_height) * 3);

	b.biXPelsPerMeter = 0;
	b.biYPelsPerMeter = 0;
	// These values denote how many values are in the RGB color map
	// that follows. 
	b.biClrUsed       = 0;
	b.biClrImportant  = 0;

	outbuffer = (uint8_t*)malloc(arr_size+sizeof(BITMAPINFOHEADER)+sizeof(BITMAPFILEHEADER));
	outsize = arr_size+sizeof(BITMAPINFOHEADER)+sizeof(BITMAPFILEHEADER);
	memcpy(outbuffer,(void*)(&h),sizeof(BITMAPFILEHEADER));
	memcpy(outbuffer+sizeof(BITMAPFILEHEADER),(void*)(&b),sizeof(BITMAPINFOHEADER));
	memcpy(outbuffer+sizeof(BITMAPINFOHEADER)+sizeof(BITMAPFILEHEADER),inbuffer,arr_size);
	//free(temp2);
	//fwrite(&b,HEADERINFOSIZE,1,s->out_handle);
	int32_t i,j, bytes_read = 0;
	off_t location;
	int32_t offset = (image_width * image_height) - image_width;
	unsigned char c;

	for (i = 0 ; i < image_height ; ++i) {
		for (j = 0 ; j < image_width ; ++j) {
			location =  offset - (image_width * i) + j;
			if (location < arr_size) { 

				if (location==arr_size) {
					return TRUE;
				}
				c = inbuffer[location]; 
			} else { 
				// We pad the image with black when necessary 
				c = padding();
			}

			write_byte(outbuffer,c,i+j+sizeof(BITMAPINFOHEADER)+sizeof(BITMAPFILEHEADER));
			//write_byte(s, c);
		}

		pad_image(outbuffer,image_width,bytes_read);
	}    
	return FALSE;
}

int write_bmp_pxls(int image_width,int image_height,uint8_t* &inbuffer,cimg_library::CImg<uint8_t> &outbuffer,size_t arr_size)
{
	
	uint8_t r,g,b;
	for (int i = 0 ; i < image_height ; ++i) {
		for (int j = 0 ; j < image_width-3 ; ++j) {
			uint8_t c = inbuffer[i+j];
			outbuffer(i,j,inbuffer[i+j],0);
			outbuffer(i,j,inbuffer[i+j+1],1);
			outbuffer(i,j,inbuffer[i+j+2],2);
		}
	}    
	return FALSE;
}

uint16_t bswap16(uint16_t b)
{
	return b;
}

int genbmp(uint8_t* &mainvector,const char* fname, int chunk_size=255)
{
	HANDLE file;
	int size = 0;
	int arrsize = 0;
	file = CreateFile(fname,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);  //Sets up the new bmp to be written to
	DWORD lphigh;
	size = GetFileSize(file,&lphigh);
	mainvector = (uint8_t*)calloc(size,sizeof(char));
	ReadFile(file,mainvector,size,&lphigh,NULL);
	arrsize = size/sizeof(uint8_t);
	CloseHandle(file);

	return arrsize;
}
bool sort_by_hash(const pair<ulong64,int> &lhs, const pair<ulong64,int> &rhs) { return lhs.first < rhs.first; }
int examine_proc(int chunk_size=255)
{
	uint8_t* mainvector;
	//float* slavevector;
	int examine_length = 0;
	//vec of ulong64 hashes and the chunk they appear in
	std::vector<std::pair<ulong64,int>> *v = new vector<std::pair<ulong64,int>>();
	int arr_size1 = genbmp(mainvector,"C:\\Users\\vesh\\Documents\\Visual Studio 2012\\Projects\\pHash-0.9.4\\Debug\\dumps\\proc.108.svchost.bin");
	int s = arr_size1/chunk_size;
	int root = sqrt(s);
	int x = 32;//s/0x64;
	int y = 32;//s/0x64;
	int mover = x*y*3;//arr_size1/chunk_size;
	//BMP data length calc is width*height*3 where three is the number of of color vals in a pixel
	for(int i=0,j=0;i<arr_size1-mover;i=i+mover,j++)
	{
		uint8_t* p_main = (mainvector)+i;
		if(NULL != p_main)
		{
			uint8_t* temp;
			size_t outsize;
			ulong64 hash1 =0;
			char* fname = new char[260];
			cimg_library::CImg<uint8_t> img2(p_main,x,y,1,3,1);
			int err1 = ph_dct_imagehash_from_buffer(img2,hash1);
			if(hash1 !=0)
			{
				//Save the image here when you are done for the x-compares
				sprintf(fname,"images\\%llx_%x_resave.%s",hash1,i,"bmp");
				img2.save(fname);
				//remove(tempname);
				std::pair<ulong64,int> tempp(hash1,i);
				v->push_back(tempp);

			}
			//free(temp);
		}

	}
	free(mainvector);
	std::sort(v->begin(),v->end(),sort_by_hash);
	ulong64 last_val =0;
	int last_chunk = 0;
	FILE* fi;
	fi = fopen("C:\\Users\\vesh\\Documents\\Visual Studio 2012\\Projects\\pHash-0.9.4\\images\\mapping.txt","w");
	vector<HashList> *v_hash = new vector<HashList>();
	for (std::vector<pair<ulong64,int>>::iterator it=(*v).begin(); it!=(*v).end(); ++it)
	{
		int hm = ph_hamming_distance(last_val,it->first);
		HashList* h = (HashList*)malloc(sizeof(HashList));
		h->hamming_distance=hm;
		h->hash1=last_val;
		h->hash2 = it->first;
		h->chunk1 = last_chunk;
		h->chunk2 = it->second;
		v_hash->push_back(*h);
		
		fprintf(fi,"Hamming between %llx and %llx was %d %s",last_val,it->first,hm,"\n");

		last_val = (*it).first;
		last_chunk = (*it).second;
	}
	for (std::vector<HashList>::iterator it=(*v_hash).begin(); it!=(*v_hash).end(); ++it){
		char* fname_last = new char[260];
		char* fname_last_copy = new char[260];
		char* fname_it = new char[260];
		char* fname_it_copy = new char[260];
		HashList h = *it;
		
		sprintf(fname_last,"images\\%llx_%x_resave.%s",h.hash1,h.chunk1,"bmp");
		sprintf(fname_last_copy,"images\\%d\\%llx_%x_resave.%s",h.hamming_distance,h.hash1,h.chunk1,"bmp");
		sprintf(fname_it,"images\\%llx_%x_resave.%s",h.hash2,h.chunk2,"bmp");
		sprintf(fname_it_copy,"images\\%d\\%llx_%x_resave.%s",h.hamming_distance,h.hash2,h.chunk2,"bmp");
		CopyFile(fname_it,fname_it_copy,false);
		CopyFile(fname_last,fname_last_copy,false);
	}
	fclose(fi);
	//free(slavevector);
	return 0;
}


int main(int argc, char* argv[]) { 

	examine_proc(1000);
	/*ulong64 hash1 =0;
	int err1 = ph_dct_imagehash_from_buffer(img1,hash1);
	ulong64 hash2 = 0;
	int err2 = ph_dct_imagehash_from_buffer(img2,hash2);
	int compar = ph_hamming_distance(hash1,hash2);*/

	return 0; 
}
//Split files into 1MB chunks and find which chunks differed the most
