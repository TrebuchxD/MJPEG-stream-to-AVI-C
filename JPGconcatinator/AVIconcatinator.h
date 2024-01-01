#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
	Todo: 
		Write file extend possibility
		Catch moment when the filesize > 1GB -> return error on append function

*/

//////////// CONFIG: ////////////

#define AVI_FILE_BUF_SIZE	32*1024 //default 32k
//When 1 the AVINameGenerate () is skiped from code compilation.
#define AVI_CUSTOM_NAME_GENERATOR 0
//if 1 a junk section increasing size to 512B is added
//#define AVI_MAKE_HEADER_512B 1 //todo:

//for compatibility with esp framework
#define AVI_MALLOCATOR(size) malloc(size)


////////////////////////////////////

//Create new file options:
typedef enum {
	AVI_CREATE_ALWAYS,
	AVI_FAIL_IF_EXIST,
	AVI_GEN_NAME_IF_EXIST,
	AVI_EXTEND_IF_EXIST //todo: not implemented yet
} AVI_create_modes_t;

//Error return values & meanning:
typedef enum {
	AVI_OK = 0,
	AVI_FILE_ERROR = 1,
	AVI_MALLOC_ERROR = 2,
	AVI_WRONG_CONFIG = 4,
	AVI_FILE_EXIST = 8,
	AVI_AUTOSAVE_DONE = 16
} AVI_error_t;

//Function to generate new names (it is not checked if the new name is not existing in the file system)
//The pointer should point to internal static array with .avi extention.
//You are supposed to overwrite this function.
char* AVINameGenerate(void);

typedef uint8_t* DATA_PTR;

typedef struct {
	uint32_t width;				//self explenatory
	uint32_t height;			//self explenatory
	float fps;					//frames per second
	uint16_t framesBeforeSave;	//frames before a copy is push to the memory, 0 disables the feature.
}fileAVIConfig_t;

typedef struct {
	//modify:
	fileAVIConfig_t* config; //must be provided by the user

	//internal variables:
	FILE* plik;
	uint32_t size;		//in Bytes
	uint32_t frames;	//count
	char* buffer;		//pointer to mallocated buffer
	uint16_t FBS;		//Frame before save
	uint16_t FBSC;		//to store save value (FBS counter)
}FILE_AVI_t;

#ifdef __cplusplus
extern "C" {
#endif

	AVI_error_t AVIOpenNew(FILE_AVI_t* file, const char* path, AVI_create_modes_t openMode);
	AVI_error_t AVIAttachFrame(FILE_AVI_t* file, const DATA_PTR* data, size_t len);
	AVI_error_t AVIClose(FILE_AVI_t* file);

#ifdef __cplusplus
}
#endif