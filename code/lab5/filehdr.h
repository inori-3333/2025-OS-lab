// filehdr.h 
//	Data structures for managing a disk file header.  
//
//	A file header describes where on disk to find the data in a file,
//	along with other information about the file (for instance, its
//	length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "disk.h"
#include "bitmap.h"
#include "libgen.h"//inori333: add libgen.h to use basename() function

#define NumDirect 	((SectorSize - 2 * sizeof(int)) / sizeof(int))
#define MaxFileSize 	(NumDirect * SectorSize)
#define MAX_DIR_DEPTH 5 //inori333: add MAX_DIR_DEPTH to limit the depth of directory

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.

//inori333: add FilePath struct to store the path of the file
typedef struct {
  char* dirArray[MAX_DIR_DEPTH];
  int dirDepth; // 目录深度，如果为0，则为根目录
  char* base;
} FilePath;
//end inori333
extern FilePath pathParser(char* path);//实现文件路径解析功能 inori333

class FileHeader {
  public:
    FileHeader();// 新增函数 FileHeader函数的构造

    bool Allocate(BitMap *freeMap, int fileSize, int incrementBytes);// 新增代码
    // Allocate的重载

    bool Allocate(BitMap *bitMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(BitMap *bitMap);  		// De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.

    char* getFileType(){return strdup(fileType);}//新增代码 inori333
    void HeaderCreateInit(const char* type) { strncpy(fileType, type, sizeof(fileType) - 1); fileType[sizeof(fileType) - 1] = '\0'; } // 新增代码 inori333

  private:
    int numBytes;			// Number of bytes in the file
    int numSectors;			// Number of data sectors in the file
    int dataSectors[NumDirect];		// Disk sector numbers for each data 
					// block in the file
    char fileType[10];
};

#endif // FILEHDR_H
