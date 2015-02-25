#ifndef CHERK_H
#define CHERK_H

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <math.h>
#include "genhd.h"
#include "ext2_fs.h"

#ifndef bool
    #define bool int
    #define false ((bool)0)
    #define true  ((bool)1)
#endif

// Error code
#define NORMAL 0x290
#define MBR_READ 0x291
#define MBR_PTRREAD 0x292	 
#define ARG_WRONG 0x293
#define DIR_ENTRY_DOT_NOTEXIST 0x294
#define DIR_ENTRY_DOT_INODENUM 0x295
#define DIR_ENTRY_DOUBLEDOT_NOTEXIST 0x296
#define DIR_ENTRY_DOUBLEDOT_PARENTNUM 0x297
#define DIR_ENTRY_DOTS_NOTEXIST 0x298

#define INODE_BLOCKMAP 0x03
#define INODE_BITMAP 0x04
struct PartitionTableEntry{
	partition *p;
	int index;
	struct PartitionTableEntry *next;
};

typedef struct PartitionTableEntry PTE;

struct PartitionEntities{
	int count;
	PTE *p;
};

typedef struct PartitionEntities ptrEntities;

/*struct SuperBlock{
	size_t blockSize;
	size_t logBlockSize;	
	size_t iNodePerGroup;
    unsigned short magicNum;
};
*/
//typedef struct SuperBlock SuperBlock;

struct BlockGroupDesTable{
};	

struct BlockGroupDes{
	size_t iNodeTable;
};

typedef struct BlockGroupDes BlockGroupDes;

typedef unsigned char  uchar; 

const unsigned int sector_size_bytes = 512;
const unsigned int block_size = 1024;
const unsigned int inode_table_size_bytes = 214 * 1024;
const unsigned int block_des = 32;
const unsigned int inode_size = 128;
static size_t blockSize = 0;
static SuperBlock *sublk;
  
//inline toolbox 
bool inline isDirectory(unsigned short imode);
bool inline isDirectoryNameMatch(char *name, ext2_dir_entry_2 *entry);
bool inline isSupDirCorrect(ext2_dir_entry_2 *entry, size_t inodeNum, partition *e); 

unsigned short getMagicNum(partition *p);
size_t getiNodesPerGroup(partition *p);
ext2_inode getSectorNumOfiNode(size_t inode, partition *p);
ext2_inode getSectorNumofiNodeByName(char *name, partition *p);
void readiNodeTable(size_t localGroup, size_t localIndex, unsigned char *buf);
void setSuperBlockArguments(partition *p);
void readBlock(size_t blockid, uchar *buf, partition *p);
int findiNodeOfDirectory(uchar *name, ext2_dir_entry_2 *dir);
void print_sector (unsigned char *buf);
void read_sectors (int64_t start_sector, unsigned int num_sectors, void *into);
void write_sectors (int64_t start_sector, unsigned int num_sectors, void *from);
PTE *readPartitionEntity(ptrEntities *ptren, int i);
void readiNodeBitmap(partition *e, uchar *bitmap, size_t inode, size_t last);
void writeBlock(size_t blockId, uchar *buf, partition *p);
size_t findParentInode(partition *e, size_t inodeNum);
void writeiNode(ext2_inode *i, size_t inode, partition *p);
size_t readiNodeBlocks(ext2_inode inode, uchar *buf, partition *p);
void addDirEntry(size_t parentDirNum, size_t lostInode, partition *p); 

// checker function
void checkPartition(int partition, char *path, bool checkable); 
void checkDirectoryEntities(partition *e);
bool checkUnreferenceNode(partition *e, size_t inodeNum, uchar *culmap, size_t *lostfound, bool first);
#endif
