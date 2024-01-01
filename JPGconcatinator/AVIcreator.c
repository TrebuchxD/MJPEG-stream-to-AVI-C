#include "AVIconcatinator.h"

//AVI structures:
//https://xoax.net/sub_web/ref_dev/fileformat_avi/
//https://github.com/s60sc/ESP32-CAM_MJPEG2SD/blob/master/mjpeg2sd.cpp
//https://cdn.hackaday.io/files/274271173436768/avi.pdf
//https://learn.microsoft.com/en-us/windows/win32/directshow/avi-riff-file-reference
//http://www.jmcgowan.com/avitech.html

#if AVI_CUSTOM_NAME_GENERATOR != 1
char* AVINameGenerate(void){
	static char tab[] = "Random.avi";
	return tab;
}
#endif
typedef struct {
	uint32_t RIFFID;	//'RIFF'
	uint32_t filesize;	//whole file - 8
	uint32_t FORMAT;	//'AVI '
}H_RIFF;
typedef struct {
	uint32_t LISTID;	//'LIST'
	uint32_t length;	
	uint32_t HEADERID;
}H_LIST;
typedef struct {
	uint32_t LISTID;
	uint32_t length;
}H_CHUNK;
typedef struct {
	uint32_t usPerFrame;
	uint32_t maxBytePerFrame;
	uint32_t paddingGranular; //padd chunks to this value
	uint32_t flags;
	uint32_t totalFrames;	//chunks of data
	uint32_t initialFrames;	//0
	uint32_t streams;		//1
	uint32_t bufSugSize;	//0 for jpeg
	uint32_t width;
	uint32_t height;
	uint32_t RES2[4];
}H_AVIH;
typedef struct {
	uint32_t TYPE;			//vids lub iavs ?
	uint32_t handler;		//0 lub dvsd
	uint32_t flags;			//0
	uint32_t priority;		//0
	uint32_t audioVideoShift;//0
	uint32_t scale;			//1000 //my guess: in ms
	uint32_t rate;			//rate/scale?
	uint32_t START;			//0
	uint32_t vidLength;
	uint32_t BufferSize;	//larger than the largest chunk... 0 if unknown... I would put 0 here to ease
	uint32_t quality;		//0
	uint32_t aSampleSize;	//0
	uint32_t cornerTL;		//2x 16 bit
	uint32_t cornerBR;		//2x 16 bit
	//uint32_t STREAMFORMAT;	//strf
	//uint32_t sizeFormat;	//0x20

}H_STRH;
typedef struct {
	uint32_t SIZE;			//0x40
	uint32_t width;
	uint32_t height;		//0
	uint16_t PLANES;		//1
	uint16_t BITPERPIXEL;	//??
	uint32_t COMPRESSION;	//jpeg -> number 4 
	uint32_t imageSize;
	uint32_t xPixPerM;		//0
	uint32_t yPixPerM;		//0
	uint32_t colorsUsed;	//0
	uint32_t colorsReq;		//0
}BITMAPINFOHEADER_t, H_STRF; //bitmapinfoheader
typedef struct {
	H_RIFF	RIFF;	//3
	H_LIST	HDR_L;//3
	H_CHUNK	AVI_C;	//2	//AVI_C => chunk
	H_AVIH	AVI_D;		//AVI_D => data
	
	H_LIST	STR_L;
	H_CHUNK STRH_C;
	H_STRH	STRH_D; //fggood
	H_CHUNK STRF_C;
	H_STRF	STRF_D;
	H_LIST  TEMP_L;
	H_CHUNK TEMP_C;
	uint32_t TEMP_D;
	H_LIST	DATA_L;
		//Append Here Data chunks!
}HEADER_AVI;


//Developement functions:
	//Debug
static void AVITestSize(void) {
	printf("SIZEOF(HEADER_AVI) = %d\n", sizeof(HEADER_AVI));
	printf("SIZEOF(H_RIFF) = %d\n", sizeof(H_RIFF));
	printf("SIZEOF(H_LIST) = %d\n", sizeof(H_LIST));
	printf("SIZEOF(H_CHUNK) = %d\n", sizeof(H_CHUNK));
	printf("SIZEOF(H_AVIH) = %d\n", sizeof(H_AVIH));
	printf("SIZEOF(H_STRH) = %d\n", sizeof(H_STRH));
	printf("SIZEOF(H_STRF) = %d\n", sizeof(H_STRF));
}
//Generate a proper MJPEG header with universal data:
static void AVIPopulateHeaderFromFields(char* header) {
	HEADER_AVI* h = (HEADER_AVI*)header;

	//RIFF
	h->RIFF.RIFFID = 'FFIR';	//First letter at the end
	h->RIFF.filesize = 0;
	h->RIFF.FORMAT = 0x20495641;
	//H_LIST
	h->HDR_L.LISTID = 0x5453494C;
	h->HDR_L.length = 4 + sizeof(H_CHUNK) + sizeof(H_AVIH) + sizeof(H_LIST) + sizeof(H_CHUNK) + sizeof(H_STRH) + sizeof(H_CHUNK) + sizeof(H_STRF); //microsoft
	h->HDR_L.HEADERID = 'lrdh';
	//AVI_C
	h->AVI_C.LISTID = 0x68697661;
	h->AVI_C.length = sizeof(H_AVIH);
	//AVI_D
	h->AVI_D.usPerFrame = 0;
	h->AVI_D.maxBytePerFrame = 100000 * 30; //fps
	h->AVI_D.paddingGranular = 0;
	h->AVI_D.flags = 16; //??
	h->AVI_D.totalFrames = 0;
	h->AVI_D.initialFrames = 0;
	h->AVI_D.streams = 1;
	h->AVI_D.bufSugSize = 0;
	h->AVI_D.width = 0;
	h->AVI_D.height = 0;
	h->AVI_D.RES2[0] = 0;
	h->AVI_D.RES2[1] = 0;
	h->AVI_D.RES2[2] = 0;
	h->AVI_D.RES2[3] = 0;
	//LIST STRL
	h->STR_L.LISTID = 'TSIL';
	h->STR_L.length = 4 + sizeof(H_CHUNK) + sizeof(H_STRH) + sizeof(H_CHUNK) + sizeof(H_STRF); //6C microsoft
	h->STR_L.HEADERID = 'lrts';
	//chunk 
	h->STRH_C.LISTID = 'hrts';
	h->STRH_C.length = sizeof(H_STRH);
	//data
	h->STRH_D.TYPE = 'sdiv';//'svai';
	h->STRH_D.handler = 'GPJM';
	h->STRH_D.flags = 0;
	h->STRH_D.priority = 0;
	h->STRH_D.audioVideoShift = 0;
	h->STRH_D.scale = 1000; //todo TRY
	h->STRH_D.rate = 0;
	h->STRH_D.START = 0;
	h->STRH_D.vidLength = 0;
	h->STRH_D.BufferSize = 0;
	h->STRH_D.quality = 0;
	h->STRH_D.aSampleSize = 0;
	h->STRH_D.cornerTL = 0;
	h->STRH_D.cornerBR = 0 | 0;
	//STRF_C
	h->STRF_C.LISTID = 'frts';
	h->STRF_C.length = sizeof(H_STRF);
	//STRF_D
	h->STRF_D.SIZE = 0x28;
	h->STRF_D.width = 0;
	h->STRF_D.height = 0;
	h->STRF_D.PLANES = 1;
	h->STRF_D.BITPERPIXEL = 24;
	h->STRF_D.COMPRESSION = 'GPJM'; //??
	h->STRF_D.imageSize = 0;
	h->STRF_D.xPixPerM = 0;
	h->STRF_D.yPixPerM = 0;
	h->STRF_D.colorsUsed = 0;
	h->STRF_D.colorsReq = 0;
	//DATA_L
	h->DATA_L.LISTID = 'TSIL';
	h->DATA_L.length = 0;
	h->DATA_L.HEADERID = 'ivom';
	//TEMP:
	h->TEMP_L.LISTID = 'TSIL';
	h->TEMP_L.length = 16;
	h->TEMP_L.HEADERID = 'lmdo';
	h->TEMP_C.LISTID = 'hlmd';
	h->TEMP_C.length = 4;
	h->TEMP_D = 0; //put amount of frames

	AVITestSize();

	return 0;
}
//To change this universal data to array:
void AVIPrintTestArray(void) {
	HEADER_AVI tempHeader;
	AVIPopulateHeaderFromFields(&tempHeader);
	printf(" = {");
	for (int x = 0; x < sizeof(HEADER_AVI); ++x) {
		if (x) {
			printf(", ");
		}
		printf("0x%x", ((uint8_t*)&tempHeader)[x]);
	}
	printf("};");
	return;
}

//Get general copy of the header
void AVIPopulateHeaderFromMem(char* header) {
	//Generated with the AVIPopulateHeaderFromFields ()
	const static uint8_t HeaderBasicData [] = { 0x52, 0x49, 0x46, 0x46, 0x0, 0x0, 0x0, 0x0, 0x41, 0x56, 0x49, 0x20, 0x4c, 0x49, 0x53, 0x54, 0xc0, 0x0, 0x0, 0x0, 0x68, 0x64, 0x72, 0x6c, 0x61, 0x76, 0x69, 0x68, 0x38, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc0, 0xc6, 0x2d, 0x0, 0x0, 0x0, 0x0, 0x0, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4c, 0x49, 0x53, 0x54, 0x74, 0x0, 0x0, 0x0, 0x73, 0x74, 0x72, 0x6c, 0x73, 0x74, 0x72, 0x68, 0x38, 0x0, 0x0, 0x0, 0x76, 0x69, 0x64, 0x73, 0x4d, 0x4a, 0x50, 0x47, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xe8, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x73, 0x74, 0x72, 0x66, 0x28, 0x0, 0x0, 0x0, 0x28, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x18, 0x0, 0x4d, 0x4a, 0x50, 0x47, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4c, 0x49, 0x53, 0x54, 0x10, 0x0, 0x0, 0x0, 0x6f, 0x64, 0x6d, 0x6c, 0x64, 0x6d, 0x6c, 0x68, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4c, 0x49, 0x53, 0x54, 0x0, 0x0, 0x0, 0x0, 0x6d, 0x6f, 0x76, 0x69 };
	memcpy(header, HeaderBasicData, sizeof(HEADER_AVI));
	return;
}
//Put application specific data to the general header
void AVIPopulateHeaderFromConfig(char* header, const FILE_AVI_t* avi) {
	HEADER_AVI* h = (HEADER_AVI*)header;
	h->AVI_D.usPerFrame = 1000000 / (avi->config->fps);
	h->AVI_D.totalFrames = avi->frames;
	h->AVI_D.width = avi->config->width;
	h->AVI_D.height = avi->config->height;
	h->STRF_D.width = avi->config->width;		//!
	h->STRF_D.height = avi->config->height;		//!
	h->STRH_D.rate = (avi->config->fps) * 1000; //todo 1000 to scale!
	h->STRH_D.vidLength = avi->frames;
	h->STRH_D.cornerTL = (avi->config->width) | ((avi->config->height)<<16);//TODO: A guess!
	h->STRF_D.imageSize = 3 * (avi->config->width) * (avi->config->height);
	//whole file size & data size (data + chunks + padding):
	
	h->RIFF.filesize = avi->size+sizeof(HEADER_AVI) - 8;
	h->DATA_L.length = avi->size+4;

	//test
	h->TEMP_D = avi->frames;
	return;
}
//Read application specific data from file
void AVIPopulateConfigFromHeader(FILE_AVI_t* avi, const char* header) {
	//Todo to give an ability to extend existing files!
}
	//Write header to the disk. Must be called on a new file and on save event.
static void AVISyncFile(FILE_AVI_t* file) {
	HEADER_AVI tempHeader; // can overflow the stack!
	fpos_t buffer;
	//prepare the headers:
	AVIPopulateHeaderFromMem((char*)&tempHeader);
	AVIPopulateHeaderFromConfig((char*)&tempHeader, file);
	//rewind the file:
	fgetpos(file->plik, &buffer);
	rewind(file->plik); //should sync the data
	fwrite(&tempHeader, sizeof(tempHeader), 1, file->plik);
	//restore te position
	fsetpos(file->plik, &buffer);
	return;
}


//////////// USER INTERFACE: ////////////
//// BEGIN THE WORK:
AVI_error_t AVIOpenNew(FILE_AVI_t * file,const char* path, AVI_create_modes_t openMode) {
	//Variables:
	FILE* plik;
	if (file->config == 0) {
		return AVI_WRONG_CONFIG;
	}

		//MANAGE FILES:
	if (openMode != AVI_CREATE_ALWAYS) {
		//check if the file exists:
		plik = fopen(path, "rb");
		if (plik) {
			//exists
			if (openMode == AVI_FAIL_IF_EXIST)
				return AVI_FILE_EXIST;
			//generate new path:

			path = tmpnam(0);
			printf("Created file: %s", path);
		}
			//doesn't exist
	}
	//Just open a file with substitution
	plik = fopen(path, "wb");
	if (plik == 0) {
		return AVI_FILE_ERROR;
	}
	//File should be ready at this point!
	
	file->buffer = (char*)AVI_MALLOCATOR(AVI_FILE_BUF_SIZE);
	if (file->buffer == 0) {
		fclose(plik);
		return AVI_MALLOC_ERROR;
	}
	setvbuf(plik, file->buffer, _IOFBF, AVI_FILE_BUF_SIZE);
	//the file is ready at this point.
		//prepare the file structure.

	HEADER_AVI tempHeader; // can overflow the stack!
	AVIPopulateHeaderFromMem((char*)&tempHeader);

	fwrite(&tempHeader, sizeof(tempHeader), 1, plik);

	//save important data
	file->frames = 0;
	file->size = 0;
	file->plik = plik;
	file->FBS = file->FBSC = file->config->framesBeforeSave;
	return 0;
}
//AVI_error_t AVIUseFile()
//AVI_error_t AVIOpenExisting(FILE_AVI_t* file, FILE* plik) {} A way of using user specified file, but due to buffering, it doesn't look like a good idea

//Whole frame must be feeded per call
AVI_error_t AVIAttachFrame(FILE_AVI_t* file, const DATA_PTR* data, size_t len) {
	const H_CHUNK tempChunk = { //static -> value does not change
		.LISTID = 'cd00',
		.length = len
	};
	fwrite(&tempChunk, sizeof(H_CHUNK), 1, file->plik);
	fwrite(data, len, 1, file->plik);
	
	if (len & 1) {
		//odd size -> pad to even:
		uint8_t buffer = 0;
		fwrite(&buffer, 1, 1, file->plik);
		len += 1;
	}

	file->frames++;
	file->size += len + sizeof(H_CHUNK);

	//check for autosave:
	if (file->FBSC) {
		--file->FBS;
		if (file->FBS == 0) {
			file->FBS = file->FBSC;
			//store recent data:
			AVISyncFile(file);
			return AVI_AUTOSAVE_DONE;
		}
	}

	return AVI_OK;
}


AVI_error_t AVIClose(FILE_AVI_t* file) {
	fileAVIConfig_t* temp = file->config;

	AVISyncFile(file);
	fclose(file->plik);

	if (file->buffer) {
		free(file->buffer);
	}
	memset(file, 0, sizeof(FILE_AVI_t));
	file->config = temp;
	return 0;
}