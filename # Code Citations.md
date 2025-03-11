# Code Citations

## License: 未知
https://github.com/ankurcha/csci402p1/tree/99d7655c2019b6e630ed545e8229722ea1776bd7/filesys/filehdr.h

```
(BitMap *bitMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(BitMap *bitMap);  		// De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber
```


## License: 未知
https://github.com/jrivera777/Nachos/tree/3fecda2b0f1dd28c6fc113f3ebe1ac3f6566db75/nachos-3.4/code/filesys/filehdr.h

```
);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(BitMap *bitMap);  		// De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header
```


## License: 未知
https://github.com/tkcstarsky/Course-Projects/tree/a5685724ca31d459d3a334a4b0a75348acc6c620/Operating%20System/Lab2/lab2-nachos-src/code/filesys/filehdr.h

```
;  		// De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	/
```

