// filesys.cc 
//	Routines to manage the overall operation of the file system.
//	Implements routines to map from textual file names to files.
//
//	Each file in the file system has:
//	   A file header, stored in a sector on disk 
//		(the size of the file header data structure is arranged
//		to be precisely the size of 1 disk sector)
//	   A number of data blocks
//	   An entry in the file system directory
//
// 	The file system consists of several data structures:
//	   A bitmap of free disk sectors (cf. bitmap.h)
//	   A directory of file names and file headers
//
//      Both the bitmap and the directory are represented as normal
//	files.  Their file headers are located in specific sectors
//	(sector 0 and sector 1), so that the file system can find them 
//	on bootup.
//
//	The file system assumes that the bitmap and directory files are
//	kept "open" continuously while Nachos is running.
//
//	For those operations (such as Create, Remove) that modify the
//	directory and/or bitmap, if the operation succeeds, the changes
//	are written immediately back to disk (the two files are kept
//	open during all this time).  If the operation fails, and we have
//	modified part of the directory and/or bitmap, we simply discard
//	the changed version, without writing it back to disk.
//
// 	Our implementation at this point has the following restrictions:
//
//	   there is no synchronization for concurrent accesses
//	   files have a fixed size, set when the file is created
//	   files cannot be bigger than about 3KB in size
//	   there is no hierarchical directory structure, and only a limited
//	     number of files can be added to the system
//	   there is no attempt to make the system robust to failures
//	    (if Nachos exits in the middle of an operation that modifies
//	    the file system, it may corrupt the disk)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "disk.h"
#include "bitmap.h"
#include "directory.h"
#include "filehdr.h"
#include "filesys.h"

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
#define FreeMapSector 		0
#define DirectorySector 	1

// Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number 
// of files that can be loaded onto the disk.
#define FreeMapFileSize 	(NumSectors / BitsInByte)
#define NumDirEntries 		10
#define DirectoryFileSize 	(sizeof(DirectoryEntry) * NumDirEntries)

#define DirFileExt "DirF"//新增代码 inori333
#define IsDirFile(filehdr) (!strcmp(fileHdr->getFileType(),DirFileExt))//新增代码 inori333

const char* getFileExtention(const char* name) {
    const char* dot = strrchr(name, '.');
    return dot ? dot + 1 : "";
}//新增代码 inori333


//----------------------------------------------------------------------
// FileSystem::FileSystem
// 	Initialize the file system.  If format = TRUE, the disk has
//	nothing on it, and we need to initialize the disk to contain
//	an empty directory, and a bitmap of free sectors (with almost but
//	not all of the sectors marked as free).  
//
//	If format = FALSE, we just have to open the files
//	representing the bitmap and the directory.
//
//	"format" -- should we initialize the disk?
//----------------------------------------------------------------------

FileSystem::FileSystem(bool format)
{ 
    DEBUG('f', "Initializing the file system.\n");
    if (format) {
        BitMap *freeMap = new BitMap(NumSectors);    //free sector map
        Directory *directory = new Directory(NumDirEntries); //initialize the directory table
	FileHeader *mapHdr = new FileHeader;
	FileHeader *dirHdr = new FileHeader;

        DEBUG('f', "Formatting the file system.\n");

    // First, allocate space for FileHeaders for the directory and bitmap
    // (make sure no one else grabs these!)
	freeMap->Mark(FreeMapSector);	    
	freeMap->Mark(DirectorySector);

    // Second, allocate space for the data blocks containing the contents
    // of the directory and bitmap files.  There better be enough space!

	ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize));
	ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize));

    // Flush the bitmap and directory FileHeaders back to disk
    // We need to do this before we can "Open" the file, since open
    // reads the file header off of disk (and currently the disk has garbage
    // on it!).

        DEBUG('f', "Writing headers back to disk.\n");
	mapHdr->WriteBack(FreeMapSector);    
	dirHdr->WriteBack(DirectorySector);

    // OK to open the bitmap and directory files now
    // The file system operations assume these two files are left open
    // while Nachos is running.

        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
     
    // Once we have the files "open", we can write the initial version
    // of each file back to disk.  The directory at this point is completely
    // empty; but the bitmap has been changed to reflect the fact that
    // sectors on the disk have been allocated for the file headers and
    // to hold the file data for the directory and bitmap.

        DEBUG('f', "Writing bitmap and directory back to disk.\n");
	freeMap->WriteBack(freeMapFile);	 // flush changes to disk
	directory->WriteBack(directoryFile);

	if (DebugIsEnabled('f')) {
	    freeMap->Print();
	    directory->Print();

        delete freeMap; 
	delete directory; 
	delete mapHdr; 
	delete dirHdr;
	}
    } else {
    // if we are not formatting the disk, just open the files representing
    // the bitmap and directory; these are left open while Nachos is running
        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
    }
}

//----------------------------------------------------------------------
// FileSystem::Create
// 	Create a file in the Nachos file system (similar to UNIX create).
//	Since we can't increase the size of files dynamically, we have
//	to give Create the initial size of the file.
//
//	The steps to create a file are:
//	  Make sure the file doesn't already exist
//        Allocate a sector for the file header
// 	  Allocate space on disk for the data blocks for the file
//	  Add the name to the directory
//	  Store the new file header on disk 
//	  Flush the changes to the bitmap and the directory back to disk
//
//	Return TRUE if everything goes ok, otherwise, return FALSE.
//
// 	Create fails if:
//   		file is already in directory
//	 	no free space for file header
//	 	no free entry for file in directory
//	 	no free space for data blocks for the file 
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	"name" -- name of file to be created
//	"initialSize" -- size of file to be created
//----------------------------------------------------------------------

bool
FileSystem::Create(char *name, int initialSize)
{
    Directory *directory;
    BitMap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;
    bool isDir = false;//新增代码 inori333
    if(initialSize == -1){//新增代码 inori333
        isDir = true;
        initialSize = DirectoryFileSize;
        DEBUG('f', "Creating directory %s, size %d\n", name, initialSize);
    }//新增代码 inori333
    else{
        DEBUG('f', "Creating file %s, size %d\n", name, initialSize);
    }//新增代码 inori333  

    directory = new Directory(NumDirEntries);

    int dirSector = FindDirSector(name);
    ASSERT(dirSector!=-1);
    OpenFile *dirFile = new OpenFile(dirSector);
    directory->FetchFrom(dirFile);
    FilePath filepath = pathParser(name);
    if(filepath.dirDepth==0){
        name = filepath.base;
    }

    directory->FetchFrom(directoryFile);

    if (directory->Find(name) != -1)
      success = FALSE;			// file is already in directory
    else {	
        freeMap = new BitMap(NumSectors);
        freeMap->FetchFrom(freeMapFile);
        sector = freeMap->Find();	// find a sector to hold the file header
    	if (sector == -1) 		
            success = FALSE;		// no free block for file header 
        else if (!directory->Add(name, sector))
            success = FALSE;	// no space in directory
	    else {
    	    hdr = new FileHeader;
            if (!hdr->Allocate(freeMap, initialSize))
                    success = FALSE;	// no space on disk for data
            else {	
                    success = TRUE;
                    // everthing worked, flush all changes back to disk
                    if(isDir){
                        hdr->HeaderCreateInit(DirFileExt);
                    }
                    else
                        hdr->HeaderCreateInit(getFileExtention(name));//新增代码 inori333

                    hdr->WriteBack(sector); 
                    
                    if(isDir){
                        Directory *dir=new Directory(NumDirEntries);
                        OpenFile *subDirFile = new OpenFile(sector);
                        dir->WriteBack(subDirFile);
                        delete dir;
                        delete subDirFile;
                    }
                    directory->WriteBack(dirFile);
                    delete dirFile;//end inori333
                    directory->WriteBack(directoryFile);
                    freeMap->WriteBack(freeMapFile);
            }
                delete hdr;
        }
        delete freeMap;
    }
    delete directory;
    return success;
}

//----------------------------------------------------------------------
// FileSystem::Open
// 	Open a file for reading and writing.  
//	To open a file:
//	  Find the location of the file's header, using the directory 
//	  Bring the header into memory
//
//	"name" -- the text name of the file to be opened
//----------------------------------------------------------------------

OpenFile *
FileSystem::Open(char *name)
{ 
    Directory *directory = new Directory(NumDirEntries);
    OpenFile *openFile = NULL;
    int sector;

    DEBUG('f', "Opening file %s\n", name);
    directory = (Directory*)FindDir(name);
    FilePath filepath = pathParser(name);
    if(filepath.dirDepth==0){
        name = filepath.base;
    }
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name); 
    if (sector >= 0) 		
	openFile = new OpenFile(sector);	// name was found in directory 
    delete directory;
    return openFile;				// return NULL if not found
}

//----------------------------------------------------------------------
// FileSystem::Remove
// 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	Return TRUE if the file was deleted, FALSE if the file wasn't
//	in the file system.
//
//	"name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool
FileSystem::Remove(char *name)
{ 
    Directory *directory;
    BitMap *freeMap;
    FileHeader *fileHdr;
    int sector;

    //新增代码 inori333
    directory = (Directory*)FindDir(name);
    FilePath filepath = pathParser(name);
    if(filepath.dirDepth==0){
        name = filepath.base;
    }
    //end inori333
    
    
    directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name);
    if (sector == -1) {
       delete directory;
       return FALSE;			 // file not found 
    }
    
    
    fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);

    if(IsDirFile(fileHdr)){
        DEBUG('D',"Reject the remove operation (attempt to delete a directory)\n");
        delete directory;
        delete fileHdr;
        return FALSE;
    }

    freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);

    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block
    directory->Remove(name);

    freeMap->WriteBack(freeMapFile);		// flush to disk
    directory->WriteBack(directoryFile);        // flush to disk
    delete fileHdr;
    delete directory;
    delete freeMap;
    return TRUE;
}

bool
FileSystem::RemoveDir(char *name)
{ 
    Directory *directory;
    BitMap *freeMap;
    FileHeader *fileHdr;
    int sector;
    
    directory = (Directory*)FindDir(name);
    FilePath filepath = pathParser(name);
    if(filepath.dirDepth==0){
        name = filepath.base;
    }
    sector = directory->Find(name);
    if (sector == -1) {
       delete directory;
       return FALSE;			 // file not found 
    }
    fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);
    freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);
    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block
    directory->Remove(name);
    freeMap->WriteBack(freeMapFile);		// flush to disk
    directory->WriteBack(directoryFile);        // flush to disk
    delete fileHdr;
    delete directory;
    delete freeMap;
    return TRUE;
}//end inori333

//----------------------------------------------------------------------
// FileSystem::List
// 	List all the files in the file system directory.
//----------------------------------------------------------------------

void
FileSystem::List()
{
    Directory *directory = new Directory(NumDirEntries);

    directory->FetchFrom(directoryFile);
    directory->List();
    delete directory;
}

//----------------------------------------------------------------------
// FileSystem::Print
// 	Print everything about the file system:
//	  the contents of the bitmap
//	  the contents of the directory
//	  for each file in the directory,
//	      the contents of the file header
//	      the data in the file
//----------------------------------------------------------------------

//新增代码 inori333
void
FileSystem::ListDir(char *name)
{
    printf("List Directory %s\n",name);
    Directory *directory = (Directory*)FindDir(strcat(name,"\arbitary"));
    directory->List();
    delete directory;
}
//end inori333

void
FileSystem::Print()
{
    FileHeader *bitHdr = new FileHeader;
    FileHeader *dirHdr = new FileHeader;
    BitMap *freeMap = new BitMap(NumSectors);
    Directory *directory = new Directory(NumDirEntries);

    printf("Bit map file header:\n");
    bitHdr->FetchFrom(FreeMapSector);
    bitHdr->Print();

    printf("Directory file header:\n");
    dirHdr->FetchFrom(DirectorySector);
    dirHdr->Print();

    freeMap->FetchFrom(freeMapFile);
    freeMap->Print();

    directory->FetchFrom(directoryFile);
    directory->Print();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
}

//新增代码9行 实现了函数getBitMap和setBitMap
BitMap* FileSystem::getBitMap() { 
    //numSector: DISK 上总扇区数（共有32*32=1024个扇区） 
    BitMap *freeBitMap = new BitMap(NumSectors);   
    freeBitMap->FetchFrom(freeMapFile); 
    return freeBitMap; 
    } 
void FileSystem::setBitMap(BitMap* freeMap) { 
    freeMap->WriteBack(freeMapFile); 
}
//新增代码 inori333
void * FileSystem::FindDir(char *filepath) {//新增代码 inori333
    Directory *returnDir = new Directory(NumDirEntries);
    int sector = FindDirSector(filepath);
    if (sector == DirectorySector){
        returnDir->FetchFrom(directoryFile);
    }
    else if(sector!=1){
        OpenFile *dirFile = new OpenFile(sector);
        returnDir->FetchFrom(dirFile);
        delete dirFile;
    }
    else{
        DEBUG('D',"No such directory.(might be deleted)\n");
    }
    return (void *)returnDir;//返回目录指针: 目标文件的目录结构
}
//end inori333

//新增代码 inori333
int FileSystem::FindDirSector(char *filePath) {
    FilePath filepath = pathParser(filePath);
    int sector = DirectorySector;
    if(filepath.dirDepth!=0){
        OpenFile* dirFile;
        Directory* dirTemp;
        for(int i=0;i<filepath.dirDepth;i++){
            DEBUG('D',"Finding directory\"%s\"in sector\"%d\"\n",filepath.dirArray[i],sector);
            dirFile = new OpenFile(sector);
            dirTemp = new Directory(NumDirEntries);
            dirTemp->FetchFrom(dirFile);
            sector = dirTemp->Find(filepath.dirArray[i]);
            if(sector==-1)
                break;        
    }
    delete dirFile;
    delete dirTemp;
    }
    return sector;//返回目录所在扇区号
}
//end inori333