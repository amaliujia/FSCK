	/* $cmuPDL: readwrite.c,v 1.3 2010/02/27 11:38:39 rajas Exp $ */
	/* $cmuPDL: readwrite.c,v 1.4 2014/01/26 21:16:20 avjaltad Exp $ */
	/* readwrite.c
	 *
	 * Code to read and write sectors to a "disk" file.
	 * This is a support file for the "fsck" storage systems laboratory.
	 *
	 * author: Sky Dragon 
	 */
	#include "myfsck.h"
	
	#if defined(__FreeBSD__)
	#define lseek64 lseek
	#endif

	/* linux: lseek64 declaration needed here to eliminate compiler warning. */
	extern int64_t lseek64(int, int64_t, int);
	//extern int opterr;
	extern char *optarg;
	extern int optind, opterr, optop;

	static int device;  /* disk file descriptor */

	static ptrEntities ptren;

	/* print_sector: print the contents of a buffer containing one sector.
	 *
	 * inputs:
	 *   char *buf: buffer must be >= 512 bytes.
	 *
	 * outputs:
	 *   the first 512 bytes of char *buf are printed to stdout.
	 *
	 * modifies:
	 *   (none)
	 */
	void print_sector (unsigned char *buf)
	{
		int i;
		for (i = 0; i < sector_size_bytes; i++) {
			printf("%02x", buf[i]);
			if (!((i+1) % 32))
				printf("\n");      /* line break after 32 bytes */
			else if (!((i+1) % 4))
				printf(" ");   /* space after 4 bytes */
		}
	}

	void print_buffer(uchar *buf, size_t size){
			int i;
			for (i = 0; i < size; i++) {
				printf("%02x", buf[i]);
				if (!((i+1) % 32))
				   printf("\n");      /* line break after 32 bytes */
				else if (!((i+1) % 4))
					printf(" ");   /* space after 4 bytes */
			}
	}

	/* read_sectors: read a specified number of sectors into a buffer.
	 *
	 * inputs:
	 *   int64 start_sector: the starting sector number to read.
	 *                       sector numbering starts with 0.
	 *   int numsectors: the number of sectors to read.  must be >= 1.
	 *   int device [GLOBAL]: the disk from which to read.
	 *
	 * outputs:
	 *   void *into: the requested number of sectors are copied into here.
	 *
	 * modifies:
	 *   void *into
	 */
	void read_sectors (int64_t start_sector, unsigned int num_sectors, void *into)
	{
		ssize_t ret;
		int64_t lret;
		int64_t sector_offset;
		ssize_t bytes_to_read;

		if (num_sectors == 1) {
		  //  printf("Reading sector %"PRId64"\n", start_sector);
		} else {
		   // printf("Reading sectors %"PRId64"--%"PRId64"\n",
				   //start_sector, start_sector + (num_sectors - 1));
		}

		sector_offset = start_sector * sector_size_bytes;

		if ((lret = lseek64(device, sector_offset, SEEK_SET)) != sector_offset) {
			fprintf(stderr, "Seek to position %"PRId64" failed: "
					"returned %"PRId64"\n", sector_offset, lret);
			exit(-1);
		}

		bytes_to_read = sector_size_bytes * num_sectors;

		if ((ret = read(device, into, bytes_to_read)) != bytes_to_read) {
			fprintf(stderr, "Read sector %"PRId64" length %d failed: "
					"returned %"PRId64"\n", start_sector, num_sectors, (long long)ret);
			exit(-1);
		}
	}


	/* write_sectors: write a buffer into a specified number of sectors.
	 *
	 * inputs:
	 *   int64 start_sector: the starting sector number to write.
	 *                	sector numbering starts with 0.
	 *   int numsectors: the number of sectors to write.  must be >= 1.
	 *   void *from: the requested number of sectors are copied from here.
	 *
	 * outputs:
	 *   int device [GLOBAL]: the disk into which to write.
	 *
	 * modifies:
	 *   int device [GLOBAL]
	 */
	void write_sectors (int64_t start_sector, unsigned int num_sectors, void *from)
	{
		ssize_t ret;
		int64_t lret;
		int64_t sector_offset;
		ssize_t bytes_to_write;

		if (num_sectors == 1) {
			printf("Reading sector  %"PRId64"\n", start_sector);
		} else {
			printf("Reading sectors %"PRId64"--%"PRId64"\n",
				   start_sector, start_sector + (num_sectors - 1));
		}

		sector_offset = start_sector * sector_size_bytes;

		if ((lret = lseek64(device, sector_offset, SEEK_SET)) != sector_offset) {
			fprintf(stderr, "Seek to position %"PRId64" failed: "
					"returned %"PRId64"\n", sector_offset, lret);
			exit(-1);
		}

		bytes_to_write = sector_size_bytes * num_sectors;

		if ((ret = write(device, from, bytes_to_write)) != bytes_to_write) {
			fprintf(stderr, "Write sector %"PRId64" length %d failed: "
					"returned %"PRId64"\n", start_sector, num_sectors, (long long)ret);
			exit(-1);
		}
	}

	int main (int argc, char **argv)
	{
		int errno = 0;

		char opt;
		char *path;
		int partitionNum = -1;
		int ext2Num = -1;
		int i;
		
		while((opt = getopt(argc, argv, "p:i:f:")) != -1){
			switch(opt){
				case 'p':
					partitionNum = atoi(optarg);	
					break;
				case 'i':
					path = optarg;
					break;
				case 'f':
					ext2Num = atoi(optarg);
					break;
				default:
					printf("Correct usage:---");
					break;
			}
		}
		if ((device = open(path, O_RDWR)) == -1) {
			perror("Could not open device file");
			exit(-1);
		}

		checkPartition(partitionNum, path, !(partitionNum == -1));
		
		if(ext2Num == -1){
			goto done;
		}else if(ext2Num == 0){

		}else{
        	PTE *ext2 = readPartitionEntity(&ptren, 1);
        	if(ext2 == NULL){
           	 errno = 1;
           	 goto error;
        	}
	        partition *e = ext2->p;
   		    setSuperBlockArguments(e);
			uchar bitmap[block_size];
			readiNodeBitmap(e, bitmap);		
			//pass one
			checkDirectoryEntitie(e, bitmap);	
		}
	/*	
		ext2_inode rootiNode;
		rootiNode = getSectorNumOfiNode(2,  e);
		for(i = 0; i < EXT2_N_BLOCKS; i++){
			printf("%x ", rootiNode.i_block[i]);
		}
		unsigned char dirInfo[1024];
		readBlock((size_t)rootiNode.i_block[0], dirInfo, e);	
		ext2_dir_entry_2 * dirEntry2 = (ext2_dir_entry_2 *)dirInfo; 		
		char c[256] = "lions";
		int lions = findiNodeOfDirectory(c, 256, dirEntry2);	
		printf("%d\n", lions);
		rootiNode = getSectorNumOfiNode(lions,  e);	
        readBlock((size_t)rootiNode.i_block[0], dirInfo, e);
        dirEntry2 = (ext2_dir_entry_2 *)dirInfo;
		strcpy(c, "tigers");
		lions = findiNodeOfDirectory(c, 256, dirEntry2);
        printf("%d\n", lions);*/
	done:	
		close(device);
		return errno;
	error:
		goto done;
	}

void checkDirectoryEntitie(partition *e, uchar *bitmap){
	size_t i;
	int errno = NORMAL;
	for(i = 3; i < sublk->s_inodes_count; i++){
		size_t bitbyte = (i - 1) / 8;
		size_t bitoff = (i - 1) % 8;
		char target = bitmap[bitbyte];
		if((target >> (7 - bitoff)) & 0x1 == 0){
			continue;
		}
		ext2_inode inode;			
		inode = getSectorNumOfiNode(i,  e);
		if(!isDirectory(inode.i_mode)){
			continue;
		}	
		if(inode.i_block[0] == 0){
			continue;
		}
        unsigned char dirInfo[block_size];
        readBlock((size_t)inode.i_block[0], dirInfo, e);  
		// check . 
		ext2_dir_entry_2* dotEntry = (ext2_dir_entry_2 *)dirInfo;
		if(strcmp(dotEntry->name, ".") != 0){
			errno = DIR_ENTRY_DOT_NOTEXIST; 
		}else if(dotEntry->inode != i){
			//if directry entry with "." has wrong inode num
			errno = DIR_ENTRY_DOT_INODENUM;
			printf("Entry '.' in inode (%zu) has invalid inode #: %zu. Fix? ", i, dotEntry->inode);
			dotEntry->inode = i;
			printf("yes\n");	
		}
		
		ext2_dir_entry_2* ddotEntry = (ext2_dir_entry_2 *)(dirInfo + dotEntry->rec_len);
		if(strcmp(dotEntry->name, ".") != 0){
            errno = (errno == DIR_ENTRY_DOT_NOTEXIST) ? DIR_ENTRY_DOTS_NOTEXIST : DIR_ENTRY_DOUBLEDOT_NOTEXIST;
        }else if(!isSupDirCorrect(ddotEntry, ddotEntry->inode, e)){
			printf("Entry '..' in indoe (%zu) has invalid parent inode #: %zu. Fix? ", i, dotEntry->inode);
			printf("no\n"); 	
		}	
	}
			
}

void checkPartition(int partitionNum, char *path, bool checkable){
		unsigned char buf[sector_size_bytes];        
		int the_sector;                     
		// ptrEntities ptren;
		ptren.count = 0;
		ptren.p = NULL;
		int count = 0, i, j;
		bool isFindPartition = false;

		the_sector = 0;
	//    printf("Dumping sector %d:\n", the_sector);
		read_sectors(the_sector, 1, buf);
		//print_sector(buf);

		int off = 446;
		int nextsector;
		unsigned int base0x83 = 0;
		unsigned int base0x05 = 0;
		for(i = 0; i < 4; i++){
			ptren.count++;
			PTE *pte = (PTE *)malloc(sizeof(PTE));
			pte->next = NULL;
			pte->p = (partition *)malloc(sizeof(partition));
			memcpy(pte->p, (buf + off) + i * 16, 16);
			pte->index = ++count;
			
			if(pte->index == partitionNum){
			   printf("0x%02X %d %d\n", pte->p->sys_ind, pte->p->start_sect, pte->p->nr_sects); 
				isFindPartition = true;
			}

			if(pte->p->sys_ind == 0x05){ 
			   base0x05 = pte->p->start_sect;
			   nextsector = base0x05;
			}else if(pte->p->sys_ind == 0x83){
			   base0x83 = pte->p->start_sect; 
			}
			pte->next = ptren.p;
			ptren.p = pte;	
		}

		int finished = 0;
		for(i = 0; i < 4; i++){
		   read_sectors(nextsector, 1, buf);	
		   for(j = 0; j < 4; j++){
			  partition p = *(partition *)(buf + off + j * 16);
			  if(p.sys_ind == 0x05){
				 nextsector = base0x05 + p.start_sect;
				 break;
			  }else if(p.sys_ind == 0x00){
				 finished = 1;
				 break;
			  }

			  PTE *pte = (PTE *)malloc(sizeof(PTE));
			  pte->next = NULL;
			  pte->p = (partition *)malloc(sizeof(partition));
			  memcpy(pte->p, buf + off + j * 16, 16);
			  pte->p->start_sect += nextsector;
			  ptren.count++;
			  pte->index = ++count;
			  if(pte->index == partitionNum){
				 printf("0x%02X %d %d\n", pte->p->sys_ind, pte->p->start_sect, pte->p->nr_sects); 
				isFindPartition = true;
			  }

			  pte->next = ptren.p;
			  ptren.p = pte;
			}
			if(finished){
				break;
			}
		}
		if(!isFindPartition && checkable)
			printf("-1\n");
}

	PTE *readPartitionEntity(ptrEntities *ptren, int partitionNum){
		PTE *p = NULL;
		PTE *ne = ptren->p;
		int i;
		for(i = 0; i < ptren->count; i++){
			if(ne->index == partitionNum){
				p = ne;	
				goto done;
			 }
			 ne = ne->next;
		}	
	done:
		return p;
	}

	unsigned short getMagicNum(partition *p){
		unsigned char buf[sector_size_bytes * 2];
		read_sectors(p->start_sect + 2, 2, buf);
		unsigned short magic= *((unsigned short *)(buf + 56));
		printf("%x\n", magic);
		return magic; 
	}

	size_t getiNodesPerGroup(partition *p){
		unsigned char buf[sector_size_bytes * 2];
		read_sectors(p->start_sect + 2, 2, buf);
		size_t n = *((size_t *)(buf + 40));
	}

	void setSuperBlockArguments(partition *p){
		unsigned char buf[sector_size_bytes * 2];
		read_sectors(p->start_sect + 2, 2, buf);
		sublk = (SuperBlock *)malloc(sizeof(SuperBlock));
		memcpy(sublk, buf, sector_size_bytes * 2);	
		//sublk.logBlockSize = (*((size_t *)(buf + 24)));
		//sublk.blockSize = 1024 << sublk.logBlockSize; 
		//sublk.magicNum = *((unsigned short *)(buf + 56));
		//printf("-----%d\n", *((size_t *)(buf + 40)));
	}

	void readiNode(size_t blockid, size_t localIndex, partition *p, ext2_inode *i){
		//size_t off = (localIndex + sublk->s_first_ino - 1) * inode_size;
		size_t off = (localIndex) * inode_size;
		uchar buf[block_size];
		size_t offId = off / block_size;
		size_t offIndex = off % block_size;
		read_sectors(p->start_sect + (blockid + offId) * 2, 2, buf);
		memcpy(i, buf + offIndex , sizeof(ext2_inode));	
	}	

    void readBlock(size_t blockid, uchar *buf, partition *p){
        read_sectors(p->start_sect + (blockid) * 2, 2, buf);
    }

	void readBlockGroupDes(size_t localGroup, GroupDes *b, partition *p){
		unsigned char buf[block_size];
		read_sectors(p->start_sect + 4, block_size / sector_size_bytes, buf);
		//printf("size of gourp descriptor: %d,  %d  %d\n", sizeof(GroupDes), block_size / sector_size_bytes, localGroup);
		memcpy(b, buf + localGroup * block_des, block_des);
	}
	
	bool inline isDirectory(unsigned short imode){
		return ((imode & 0xf000) == 0x4000) ? 1 : 0;	
	}

	ext2_inode getSectorNumOfiNode(size_t inode, partition *p){
		size_t inodesPerGourp = sublk->s_inodes_per_group;
		size_t localGroup = (inode - 1) / inodesPerGourp;
		size_t localIndex = (inode - 1) % inodesPerGourp;	
		GroupDes groupDes; 
		readBlockGroupDes(localGroup, &groupDes, p);	
		size_t blockId = groupDes.bg_inode_table; 
//		printf("Block id: %zu\n", blockId);
		ext2_inode i;
		readiNode(blockId, localIndex, p, &i);
		if(isDirectory(i.i_mode)){
//			printf("111111------\n");
		}
		return i;
	}

// TODO: here bug exists. No promise on length of dir, may buffer overflow
	int findiNodeOfDirectory(uchar *name, ext2_dir_entry_2 *dir){
	while(true){
		if(dir->file_type == 2){ 
				//char n[512];
				//n = dir->name;
				//memcpy(n, dir->name, dir->name_len);
				printf("%s\n", dir->name);
				if(strcmp(dir->name, name) == 0){
					return dir->inode;		
				}
		}else if(dir->rec_len == 0){
                    return -1;
        }
		dir = (ext2_dir_entry_2 *)((char *)dir + dir->rec_len);	
	}			
}

bool inline isSupDirCorrect(ext2_dir_entry_2 *entry, size_t inodeNum, partition *e){
		ext2_inode parentiNode =  getSectorNumOfiNode(inodeNum, e);
		//just try direct blocks
	   	size_t i;
		size_t dataSize = 0;
		for(i = 0; i < 12; i++){
			if(parentiNode.i_block[i] == 0){
				break;
			}
			dataSize += block_size;
		}
		if(dataSize == 0){
			return false;
		}
		uchar buf[dataSize];
		for(i = 0; i < 12; i++){
            if(parentiNode.i_block[i] == 0){
                break;
            }
			readBlock((size_t)parentiNode.i_block[i], buf + i * block_size, e); 
		}
		if(findiNodeOfDirectory(entry->name, (ext2_dir_entry_2 *)buf) == -1){
			return false;
		}		
		return true;	
}


bool inline isDirectoryNameMatch(char *name, ext2_dir_entry_2 *entry){
	if(strcmp(entry->name, name) == 0){
         return true;
    }
	return false;
}

void readiNodeBitmap(partition *e, uchar *bitmap){
	readBlock(INODE_BITMAP, bitmap, e);
}
