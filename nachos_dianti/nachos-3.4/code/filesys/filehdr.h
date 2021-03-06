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
#include <time.h>
//修改索引数量
#define NumDirect 	((SectorSize - 3 * sizeof(int)-79) / sizeof(int))
//如果我把整个sector都来索引的话就是可以索引SectorSize/sizeof(int)因为一个fileheader占一个sector
#define MaxFileSize 	(NumDirect * SectorSize)//一个sector128字节

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.可以存在内存或硬盘中
// When it is on disk, it is stored in a single sector -- this means在硬盘的时候文件头占用一个扇区
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.

class FileHeader {
  public:
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

    char type[4];//文件类型
    char create_time[25];//创建时间
    char last_visit_time[25];//上次访问时间
    char last_modified_time[25];//上次修改时间

    void set_create_time();//根据系统时间设置创建时间
    void set_last_visit_time();//根据系统时间设置上次访问时间
    void set_last_modified_time();//根据系统时间设置上次修改时间
    //设置文件头所在扇区
    int sector_position;
  private:
    int numBytes;			// Number of bytes in the file
    int numSectors;			// Number of data sectors in the file
    int dataSectors[NumDirect];		// Disk sector numbers for each data 
    //这可以说数据在哪个Sector中
					// block in the file
};

#endif // FILEHDR_H
