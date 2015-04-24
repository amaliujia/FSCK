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

#define DEBUG

#if defined(__FreeBSD__)
#define lseek64 lseek
#endif

/* linux: lseek64 declaration needed here to eliminate compiler warning. */
extern int64_t lseek64(int, int64_t, int);
//extern int opterr;
extern char *optarg;
extern int optind, opterr, optop;

static int device;  /* disk file descriptor */

SuperBlock *sublk = NULL;

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
 *   int numsectors: the numberstatic SuperBlock *sublk = NULL; of sectors to read.  must be >= 1.
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
 "" *   int device [GLOBAL]
 */
void write_sectors (int64_t start_sector, unsigned int num_sectors, void *from)
{
    ssize_t ret;
    int64_t lret;
    int64_t sector_offset;
    ssize_t bytes_to_write;
    
    if (num_sectors == 1) {
        //	printf("Reading sector  %"PRId64"\n", start_sector);
    } else {
        //	printf("Reading sectors %"PRId64"--%"PRId64"\n",
        //		   start_sector, start_sector + (num_sectors - 1));
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
    if((device = open(path, O_RDWR)) == -1) {
        perror("Could not open device file");
        exit(-1);
    }
    
    checkPartition(partitionNum, path, !(partitionNum == -1));
    
    if(ext2Num == -1){
        goto done;
    }else if(ext2Num == 0){
        partition *p = NULL;
        PTE *ne = ptren.p;
        int i;
        for(i = 0; i < ptren.count; i++){
            if(ne->p->sys_ind  == 0x83){
                printf("------------------------------\n");
                p = ne->p;
                setSuperBlockArguments(p);
                checkDirectoryEntitie(p);
                printf("------------------------------\n");
            }
            ne = ne->next;
        }
    }else{
        PTE *ext2 = readPartitionEntity(&ptren, ext2Num);
        if(ext2 == NULL || ext2->p->sys_ind != 0x83){
            printf("not ext2\n");
            errno = 1;
            goto error;
        }
        partition *e = ext2->p;
        setSuperBlockArguments(e);
        checkDirectoryEntitie(e);
    }
done:
    close(device);
    return errno;
error:
    goto done;
}

void checkDirectoryEntitie(partition *e){
    size_t i;
    int errno = NORMAL;
    size_t last = 0;
    
	   //uchar bitmap[block_size];
    uchar *bitmap = (uchar *)malloc(sizeof(uchar) * block_size);
    uchar *culmap = (uchar *)malloc(sizeof(uchar) * (sublk->s_inodes_count + 1));
    uchar *linkmap = (uchar *)malloc(sizeof(uchar) * (sublk->s_inodes_count + 1));
    //		uchar bitmap[block_size];
    memset(culmap, 0, sublk->s_inodes_count + 1);
    // read root directory entry into buffer
    for(i = 3; i <= sublk->s_inodes_count; i++){
        last = i;
        ext2_inode inode;
        inode = getSectorNumOfiNode(i,  e);
        if(isDirectory(inode.i_mode)){
            if(inode.i_block[0] == 0){
                continue;
            }
            bool isError = false;
            unsigned char dirInfo[block_size];
            readBlock((size_t)inode.i_block[0], dirInfo, e);
            // check .
            ext2_dir_entry_2* dotEntry = (ext2_dir_entry_2 *)dirInfo;
            if(strcmp(dotEntry->name, ".") != 0){
                errno = DIR_ENTRY_DOT_NOTEXIST;
                memcpy(dotEntry->name, ".", 1);
                isError = true;
            }else if(dotEntry->inode > sublk->s_inodes_count){
                printf("Entry '.' in inode (%zu) has invalid \
                       inode #: %zu.\nClean? ", i, dotEntry->inode);
                dotEntry->inode = i;
                printf("yes\n");
                isError = true;
            }else if(dotEntry->inode != i){
                //if directry entry with "." has wrong inode num
                errno = DIR_ENTRY_DOT_INODENUM;
                printf("Entry '.' in inode (%zu) has invalid \
                       inode #: %zu.\nClean? ", i, dotEntry->inode);
                dotEntry->inode = i;
                printf("yes\n");
                isError = true;
            }
            // check ..
            ext2_dir_entry_2* ddotEntry = (ext2_dir_entry_2 *)(dirInfo + dotEntry->rec_len);
            if(strcmp(ddotEntry->name, "..") != 0){
                errno = (errno == DIR_ENTRY_DOT_NOTEXIST) ? DIR_ENTRY_DOTS_NOTEXIST : DIR_ENTRY_DOUBLEDOT_NOTEXIST;
                memcpy(ddotEntry->name, "..", 2);
                isError = true;
            }else if(ddotEntry->inode > sublk->s_inodes_count){
                printf("Entry '..' in inode (%zu) has invalid inode #: %zu.\nClean? ",
                       i, ddotEntry->inode);
                size_t n = findParentInode(e, i);
                if( n == 0)
                    //TODO: how to deal with it
                    printf("no\n");
                else{
                    ddotEntry->inode = n;
                    printf("yes\n");
                }
                isError = true;
            }else if(!isSupDirCorrect(ddotEntry, ddotEntry->inode, e)){
                printf("SkyDragon: Entry '..' in indoe (%zu) has invalid parent \
                       inode #: %zu.\nClean? ", i, ddotEntry->inode);
                size_t n = findParentInode(e, i);
                if(n == 0)
                    printf("no\n");
                else{
                    ddotEntry->inode = n;
                    printf("yes\n");
                }
                isError = true;
            }
            if(isError){
                writeBlock((size_t)inode.i_block[0], dirInfo, e);
            }
        }
    }
    
    size_t lostfoundNum;
    checkUnreferenceNode(e, i, culmap, &lostfoundNum, true);
    /*  size_t y;
     for(y = 0; y < block_size * 8; y++){
		   if(culmap[y] > 0){
     printf("[%d](%d)\t", y, culmap[y]);
		   }
     }*/
    
    for(i = 3; i <= sublk->s_inodes_count; i++){
        ext2_inode inode;
        inode = getSectorNumOfiNode(i,  e);
        //	if(isDirectory(inode.i_mode)){
        if(inode.i_links_count >= 1 && culmap[i] == 0){
            printf("SkyDragon: Unconnected directory inode %zu\t\t", i);
            addDirEntry(lostfoundNum, i, e);
            printf("relink to /lost+found/%d\n", i);
        }
        //	}
    }
    memset(linkmap, 0, sublk->s_inodes_count + 1);
    checkUnreferenceNode(e, i, linkmap, &lostfoundNum, false);
    size_t y;
    for(y = 0; y < block_size * 8; y++){
        //if(linkmap[y] > 0){
        //  printf("[%d](%d)\t", y, linkmap[y]);
        //}
    }
    for(i = 2; i <= sublk->s_inodes_count; i++){
        ext2_inode inode;
        inode = getSectorNumOfiNode(i,  e);
        
        if(inode.i_links_count != linkmap[i] && inode.i_links_count > 0){
            printf("SkyDragon: Inode %d ref count is %d, should be %d\n",
                   i, inode.i_links_count, linkmap[i]);
            inode.i_links_count = linkmap[i];
            writeiNode(&inode, i, e);
        }
    }
    
    uchar *blockmap = (uchar *)malloc(sizeof(uchar) * sublk->s_blocks_count + 1);
    memset(blockmap, 0, sublk->s_blocks_count + 1);
    
#ifdef DEBUG
    printf("block number %d\n", sublk->s_blocks_count + 1);
#endif
    
    checkBlockBitmap(e);
    for(i = 2; i <= sublk->s_inodes_count; i++){
        //readiNodeBitmap(e, bitmap, i, 0, blockmap);
    }
    
    if(errno != NORMAL){
        goto error;
    }
done:
    free(linkmap);
    free(culmap);
    free(bitmap);
    return ;
error:
    goto done;
    switch(errno){
        case DIR_ENTRY_DOT_NOTEXIST:
            break;
        case DIR_ENTRY_DOUBLEDOT_NOTEXIST:
            break;
        case DIR_ENTRY_DOTS_NOTEXIST:
            break;
        default:
            break;
    }
}

size_t findParentInode(partition *e, size_t inodeNum){
    size_t i;
    size_t y;
    
    for(i = 2; i < sublk->s_inodes_count; i++){
        if(i == inodeNum){
            continue;
        }
        ext2_inode inode;
        inode = getSectorNumOfiNode(i,  e);
        if(isDirectory(inode.i_mode)){
            size_t dataSize = 0;
            uchar *singleLink = NULL;
            for(y = 0; y < 13; y++){
                if(inode.i_block[y] == 0){
                    break;
                }
                if(y < 12){
                    dataSize += block_size;
                }else{
                    singleLink = (uchar *)malloc(sizeof(uchar) * block_size);
                    readBlock(inode.i_block[y], singleLink, e);
                    size_t z;
                    for(z = 0; z < block_size / 4; z++){
                        //change
                        if(*(__u32 *)(singleLink + z * 4) != 0){
                            dataSize += block_size;
                        }else{
                            break;
                        }
                    }
                }
            }
            if(inode.i_block[12] != 0){
                printf("Yes. it matters\n");
            }
            if(dataSize == 0){
                continue;
            }
            uchar buf[dataSize];
            for(y = 0; y < 13; y++){
                if(inode.i_block[y] == 0){
                    break;
                }
                if(y < 12){
                    readBlock((size_t)inode.i_block[y], buf + y * block_size, e);
                }else{
                    if(singleLink != NULL){
                        size_t z;
                        //change
                        for(z = 0; z < block_size / 4; z++){
                            if(*(__u32 *)(singleLink + z * 4) != 0){
                                readBlock(*(__u32 *)(singleLink + z * 4),
                                          buf + (z + y) * block_size, e);
                            }else{
                                break;
                            }
                        }
                        free(singleLink);
                    }
                }
            }
            ext2_dir_entry_2 *dir = (ext2_dir_entry_2 *)buf;
            size_t off = 0;
            while(true){
                if(strcmp(dir->name, ".") != 0 && strcmp(dir->name, "..") != 0){
                    if(dir->inode == inodeNum){
                    return i;
                }
                }
                if(off >= dataSize){
                    break;
                }
                if(dir->inode == 0){
                    break;
                }
                if(dir->rec_len == 0){
                    break;
                }
                off += dir->rec_len;
                dir = (ext2_dir_entry_2 *)((char *)dir + dir->rec_len);
            }
        }
    }
    return 0;
}

bool checkUnreferenceNode(partition *e, size_t inodeNum, uchar *culmap,
                          size_t *lostfound, bool first)
{
    size_t i;
    size_t y;
    size_t last = 0;
    
    for(i = 2; i < sublk->s_inodes_count; i++){
        //if(i == inodeNum){
        //	continue;
        //}
        if(i == 2010){
            //printf("This is 2010\n");
        }
        ext2_inode inode;
        inode = getSectorNumOfiNode(i,  e);
        if(isDirectory(inode.i_mode)){
            //ext2_inode diriNode =  getSectorNumOfiNode(i, e);
            //TODO: only try direct blocks
            size_t dataSize = 0;
            uchar *singleLink = NULL;
            for(y = 0; y < 13; y++){
                if(inode.i_block[y] == 0){
                    break;
                }
                if(y < 12){
                    dataSize += block_size;
                }else{
                    singleLink = (uchar *)malloc(sizeof(uchar) * block_size);
                    readBlock(inode.i_block[y], singleLink, e);
                    size_t z;
                    for(z = 0; z < block_size / 4; z++){
                        if(*(__u32 *)(singleLink + z * 4) != 0){
                            dataSize += block_size;
                        }else{
                            break;
                        }
                    }
                }
            }
            if(inode.i_block[12] != 0){
                printf("Yes. it matters\n");
            }
            if(dataSize == 0){
                continue;
            }
            uchar buf[dataSize];
            for(y = 0; y < 13; y++){
                if(inode.i_block[y] == 0){
                    break;
                }
                if(y < 12){
                    readBlock((size_t)inode.i_block[y], buf + y * block_size, e);
                }else{
                    if(singleLink != NULL){
                        size_t z;
                        for(z = 0; z < block_size / 4; z++){
                            if(*(__u32 *)(singleLink + z * 4) != 0){
                                readBlock(*(__u32 *)(singleLink + z * 4), buf + (z + y) * block_size, e);
                            }else{
                                break;
                            }
                        }
                        free(singleLink);
                    }
                }
            }
            //try to find out reference
            ext2_dir_entry_2 *dir = (ext2_dir_entry_2 *)buf;
            size_t off = 0;
            while(true){
                if(off >= dataSize){
                    break;
                }
                if(dir->inode == 0){
                    break;
                }
                if(dir->rec_len == 0){
                    break;
                }
                if(!first){
                    culmap[dir->inode] += 1;
                }else{
                    if(strcmp(dir->name, ".") != 0 && strcmp(dir->name, "..") != 0){
                        culmap[dir->inode] += 1;
                    }
                }
                if(strcmp(dir->name, "lost+found") == 0){
                    *lostfound = dir->inode;
                }
                off += dir->rec_len;
                //off += EXT2_DIR_REC_LEN(dir->name_len);
                dir = (ext2_dir_entry_2 *)(buf + off);
            }
        }
    }
    return false;
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
    if(sublk != NULL)
        free(sublk);
    unsigned char buf[sector_size_bytes * 2];
    read_sectors(p->start_sect + 2, 2, buf);
    sublk = (SuperBlock *)malloc(sizeof(SuperBlock));
    memcpy(sublk, buf, sector_size_bytes * 2);
}

void readiNode(size_t blockid, size_t localIndex, partition *p, ext2_inode *i){
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

void writeBlock(size_t blockid, uchar *buf, partition *p){
    write_sectors(p->start_sect + (blockid) * 2, 2, buf);
}

void readBlockGroupDes(size_t localGroup, GroupDes *b, partition *p){
    unsigned char buf[block_size];
    read_sectors(p->start_sect + 4, block_size / sector_size_bytes, buf);
    memcpy(b, buf + localGroup * block_des, block_des);
}

bool inline isDirectory(unsigned short imode){
    return ((imode & 0xf000) == 0x4000) ? 1 : 0;
}

ext2_inode getSectorNumofiNodeByName(char *name, partition *p){
    size_t i;
    ext2_inode root = getSectorNumOfiNode(EXT2_ROOT_INO, p);
    uchar *buf = NULL;
    size_t dataSize = 0;
    dataSize = readiNodeBlocks(root, buf, p);
    //assert(dataSize == 0 || buf == NULL);
    if(dataSize == 0 || buf == NULL){
        printf("Error: cannot find lost+found\n");
    }
    size_t returnNode = EXT2_ROOT_INO;
    ext2_dir_entry_2 *dir = (ext2_dir_entry_2 *)buf;
    size_t off = 0;
    while(true){
        if(strcmp(dir->name, "lost+found") == 0){
            returnNode = dir->inode;
            goto done;
        }
        if(off >= dataSize){
            break;
        }
        if(dir->inode == 0){
            break;
        }
        if(dir->rec_len == 0){
            break;
        }
        off += dir->rec_len;
        dir = (ext2_dir_entry_2 *)((char *)dir + dir->rec_len);
    }
    printf("Error: cannot find lost+found\n");
done:
    free(buf);
    return getSectorNumOfiNode(returnNode , p);;
}

size_t readiNodeBlocks(ext2_inode inode, uchar *buf, partition *p){
    size_t dataSize = 0;
    size_t y;
    for(y = 0; y < 12; y++){
        if(inode.i_block[y] == 0){
            break;
        }
        dataSize += block_size;
    }
    if(dataSize == 0){
        goto done;
    }
    buf = (uchar *)malloc(dataSize);
    memset(buf, 0, dataSize);
    for(y = 0; y < 12; y++){
        if(inode.i_block[y] == 0){
            break;
        }
        readBlock((size_t)inode.i_block[y], buf + y * block_size, p);
    }
done:
    return dataSize;
}

ext2_inode getSectorNumOfiNode(size_t inode, partition *p){
    size_t inodesPerGourp = sublk->s_inodes_per_group;
    size_t localGroup = (inode - 1) / inodesPerGourp;
    size_t localIndex = (inode - 1) % inodesPerGourp;
    GroupDes groupDes;
    readBlockGroupDes(localGroup, &groupDes, p);
    size_t blockId = groupDes.bg_inode_table;
    ext2_inode i;
    readiNode(blockId, localIndex, p, &i);
    return i;
}

void writeiNode(ext2_inode *i, size_t inode, partition *p){
    size_t inodesPerGourp = sublk->s_inodes_per_group;
    size_t localGroup = (inode - 1) / inodesPerGourp;
    size_t localIndex = (inode - 1) % inodesPerGourp;
    GroupDes groupDes;
    readBlockGroupDes(localGroup, &groupDes, p);
    size_t blockId = groupDes.bg_inode_table;
    uchar buf[block_size];
    size_t off = (localIndex) * inode_size;
    size_t offId = off / block_size;
    size_t offIndex = off % block_size;
    read_sectors(p->start_sect + (blockId + offId) * 2, 2, buf);
    ((ext2_inode *)(buf + offIndex))->i_links_count = i->i_links_count;
    write_sectors(p->start_sect + (blockId + offId) * 2, 2, buf);
}

// TODO: here bug exists. No promise on length of dir, may buffer overflow
int findiNodeOfDirectory(uchar *name, ext2_dir_entry_2 *dir){
    while(true){
        if(dir->file_type == 2){
            if(strcmp(dir->name, name) == 0){
                return dir->inode;
            }
        }else if(dir->inode == 0){
            return -1;
        }else if(dir->rec_len == 0){
            return -1;
        }
        dir = (ext2_dir_entry_2 *)((char *)dir + dir->rec_len);
    }
}

bool inline isSupDirCorrect(ext2_dir_entry_2 *entry,
                            size_t inodeNum, partition *e)
{
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

void setBitmap(size_t blockId, uchar *bitmap){
    size_t bitbyte = (blockId - 1) / 8;
    size_t bitoff = (blockId - 1) % 8;
    bitmap[bitbyte] |= (0x1 << (7 - bitoff));
}


void initGroupBitmap(uchar *bitmap){
    memset(bitmap, 0, block_size * 3);
    //first block
    size_t i;
    for(i = 1; i <= 255; i++){
        setBitmap(i, bitmap);
    }
    
    //second one
    for(i = 8193; i <= 8447; i++){
        setBitmap(i, bitmap);
    }
    
    setBitmap(16385, bitmap);
    setBitmap(16386, bitmap);
    
    for(i = 16389; i <= 16639; i++){
        setBitmap(i, bitmap);
    }
}

void checkBitmap(uchar *map1, uchar *map2, int c){
    size_t i;
    for(i = 0; i < block_size * 8; i++){
        if(map1[i] != map2[i]){
            printf("[%zu]%x %x  ", i + c * 1024, map1[i], map2[i]);
        }
    }
    printf("\n");
}

void setLongBitmap(size_t blockId, partition *e, ext2_inode *inode){
    uchar cmap[block_size];
    size_t inodesPerGourp = sublk->s_blocks_per_group;
    size_t localGroup = (blockId - 1) / inodesPerGourp;
    GroupDes groupDes;
    readBlockGroupDes(localGroup, &groupDes, e);
    size_t id = groupDes.bg_block_bitmap;
    memset(cmap, 0, block_size);
    readBlock(id, cmap, e);
    size_t localIndex = (blockId - 1) % inodesPerGourp;
    size_t bitbyte = localIndex / 8;
    size_t bitoff = localIndex % 8;
    uchar target = cmap[bitbyte];
    if(((target >> (7 - bitoff)) & 0x1) == 0){
        printf("SkyDragon: block id %d unalloc  Fix?", blockId);
        cmap[bitbyte] = (target | (0x1 << (7 - bitoff)));
        writeBlock(id, cmap, e);
        printf("yes\n");
    }
}

void checkDoubleBlock(int *num, ext2_inode *cur,  uchar *bitmap,
                      partition *e, int curIndex)
{
    uchar *buf = (uchar *)malloc(block_size);
    uchar *singleLink = (uchar *)malloc(block_size);
    if(cur->i_block[curIndex] == 0){
        goto done;
    }
    if(cur->i_block[curIndex] == 0x83f){
        printf("This is this staff\n");
    }
    
    if(cur->i_block[curIndex] == 2137){
        printf("This is %zu\n", 2137);
    }
    setLongBitmap(cur->i_block[curIndex], e, cur);
    readBlock(cur->i_block[curIndex], buf, e);
    size_t max = block_size / 4;
    size_t i = 0;
    while((*num) > 0 && i < max){
        (*num) -= 1;
#ifdef DEBUG
        if(*(unsigned int *)((void *)buf + i * 4) == 2137){
            printf("this way\n");
        }
#endif
        
        if(*(__u32 *)((void *)buf + i * 4) == 0){
#ifdef DEBUG
                printf("block 14-1 is 0 but numBlock not zero\n");
#endif
            i++;
            continue;
        }
        setLongBitmap(*(unsigned int  *)((void *)buf + i * 4), e, cur);
        readBlock(*(unsigned int *)((void *)buf + i * 4), singleLink, e);
        (*num) -= 1;
        
        size_t inMax = block_size / 4;
        size_t j = 0;
        while((*num) > 0 && j < inMax){
            (*num) -= 1;
#ifdef DEBUG
            if(*(unsigned int *)((void *)singleLink + j * 4) == 2137){
                printf("this way\n");
            }
#endif
            if(*(unsigned int *)((void *)singleLink + j * 4) == 0){
#ifdef DEBUG
                	 printf("block 14-1 is 0 but numBlock not zero\n");
#endif
                j++;
                continue;
            }
            setLongBitmap(*(unsigned int *)((void *)singleLink + j * 4) , e, cur);
            j++;
        }
        i++;
        if((*num) == 0){
            goto done;
        }
    }
    
    if((*num) == 0){
        goto done;
    }
    if(curIndex != 14){
        //	checkDoubleBlock(num, cur, bitmap, e, 14);
    }
done:
    free(buf);
    free(singleLink);
}

void checkindirectblock(int *num, ext2_inode *cur,  uchar *bitmap, partition *e){
    uchar *singlelink = (uchar *)malloc(block_size);
    if(cur->i_block[12] == 0){
        //	printf("block 12 is 0 but numblock not zero\n");
        goto done;
    }
    setLongBitmap(cur->i_block[12], e, cur);
    setBitmap(cur->i_block[12], bitmap);
    readBlock(cur->i_block[12], singlelink, e);
    size_t max = block_size / 4;
    size_t i = 0;
    while((*num) > 0 && i < max){
        (*num) -= 1;
#ifdef DEBUG
        if(*(__u32 *)((void *)singlelink + i * 4) == 2142){
            printf("this way\n");
        }
#endif
        if(*(__u32 *)((void *)singlelink + i * 4) == 0){
#ifdef DEBUG
            printf("block 13 is 0 but numblock not zero\n");
#endif
            i++;
            continue;
            //goto done;
        }
        etLongBitmap(*(__u32 *)((void *)singlelink + i * 4), e, cur);
        setBitmap(*(__u32 *)((void *)singlelink + i * 4) , bitmap);
        i++;
        //(*num) -= 1;
    }
    if((*num) == 0){
        return;
    }
    
    checkDoubleBlock(num, cur, bitmap, e, 13);
done:
    free(singlelink);
    return;
}

void checkDirectBlock(int *num, ext2_inode *cur,  uchar *bitmap, partition *e){
    size_t i = 0;
    while((*num) > 0 && i < 12){
        (*num) -= 1;
#ifdef DEBUG
        if(cur->i_block[i] == 2142){
            printf("this way\n");
        }
#endif
        if(cur->i_block[i] == 0){
#ifdef DEBUG
            printf("It happensin direct block\n");
#endif
            i++;
            continue;
            //return;
        }
        setLongBitmap(cur->i_block[i], e, cur);
        setBitmap(cur->i_block[i], bitmap);
        i++;
        (*num) -= 1;
    }
    if((*num) == 0){
        return;
    }
    checkindirectblock(num, cur, bitmap, e);
}

void checkiNode(size_t i, uchar *bitmap,  partition *e){
    ext2_inode cur = getSectorNumOfiNode(i, e);
    if((cur.i_mode & 0xf000) == EXT2_S_IFLNK){
        // continue;
    }
    if((cur.i_mode & 0xf000) == 0){
        //continue;
        //return;
    }
    int numBlocks = cur.i_blocks / (2 << sublk->s_log_block_size);
    checkDirectBlock(&numBlocks, &cur, bitmap, e);
}

void writeBitmap(uchar *bitmap, partition *e){
    size_t i;
    for(i = 0; i < 3; i++){
        GroupDes groupDes;
        readBlockGroupDes(i, &groupDes, e);
#ifdef DEBUG
        uchar firstmap[1024];
        readBlock(groupDes.bg_block_bitmap, firstmap, e);
        //checkBitmap(firstmap, (void *)bitmap + i, i);
#endif
        writeBlock(groupDes.bg_block_bitmap, (void *)bitmap + i, e);
    }
}

void checkDirTree(partition *e, uchar *bitmap, size_t i){
    checkiNode(i, bitmap, e);
    
    ext2_inode inode = getSectorNumOfiNode(i, e);
    if(isDirectory(inode.i_mode)){
        size_t dataSize = 0;
        uchar *singleLink = NULL;
        size_t y;
        for(y = 0; y < 13; y++){
            if(inode.i_block[y] == 0){
                break;
            }
            if(y < 12){
                dataSize += block_size;
            }else{
                singleLink = (uchar *)malloc(sizeof(uchar) * block_size);
                readBlock(inode.i_block[y], singleLink, e);
                size_t z;
                for(z = 0; z < block_size / 4; z++){
                    if(*(__u32 *)(singleLink + z * 4) != 0){
                        dataSize += block_size;
                    }else{
                        break;
                    }
                }
            }
        }
        if(dataSize == 0){
            return;
        }
        uchar buf[dataSize];
        for(y = 0; y < 13; y++){
            if(inode.i_block[y] == 0){
                break;
            }
            if(y < 12){
                readBlock((size_t)inode.i_block[y], buf + y * block_size, e);
            }else{
                if(singleLink != NULL){
                    size_t z;
                    for(z = 0; z < block_size / 4; z++){
                        if(*(__u32 *)(singleLink + z * 4) != 0){
                            readBlock(*(__u32 *)(singleLink + z * 4),
                                      buf + (z + y) * block_size, e);
                        }else{
                            break;
                        }
                    }
                    free(singleLink);
                }
            }
        }
        
        ext2_dir_entry_2 *dir = (ext2_dir_entry_2 *)buf;
        size_t off = 0;
        while(true){
            if(off >= dataSize){
                break;
            }
            if(dir->inode == 0){
                break;
            }
            if(dir->rec_len == 0){
                break;
            }
            if(strcmp(dir->name, ".") != 0 && strcmp(dir->name, "..") != 0){
                checkDirTree(e, bitmap, dir->inode);
            }
            off += dir->rec_len;
            dir = (ext2_dir_entry_2 *)(buf + off);
        }
    }	
}

void checkBlockBitmap(partition *e){
    size_t totoalBlocks = sublk->s_blocks_count;
    size_t blocksPerGroup = sublk->s_blocks_per_group;
    size_t inodesPerGroup = sublk->s_inodes_per_group;
    size_t i;
    
    uchar bitmap[block_size * 3];
    initGroupBitmap(bitmap);
    size_t localGroup, localIndex;
    //checkDirTree(e, bitmap, 2);	
    for(i = 2; i <= sublk->s_inodes_count; i++){	
        size_t totoalBlocks = sublk->s_blocks_count;
        size_t blocksPerGroup = sublk->s_blocks_per_group;
        size_t inodesPerGroup = sublk->s_inodes_per_group;
        size_t localGroup = (i - 1) / inodesPerGroup; 
        size_t localIndex = (i - 1) % inodesPerGroup;	
        
        ext2_inode cur = getSectorNumOfiNode(i, e);
        if((cur.i_mode & 0xf000) == EXT2_S_IFLNK){
            continue;
        }
        if((cur.i_mode & 0xf000) == 0){
            continue;
        }
        int numBlocks = cur.i_blocks / (2 << sublk->s_log_block_size); 
        checkDirectBlock(&numBlocks, &cur, bitmap, e); 
    }
}

void readiNodeBitmap(partition *e, uchar *bitmap, size_t inodeNum,
                     size_t last, uchar *blockmap)
{
    
    ext2_inode inode;
    inode = getSectorNumOfiNode(inodeNum,  e);
    bool isError = false;
    uchar y;
    for(y = 0; y < 15; y++){
        if(inode.i_block[y] == 0){
            break;
        }
        size_t blockId = inode.i_block[y];
        if(blockId == 2137){
            printf("This is %zu\n", 2137);
        }
        if(y < 12){
            setLongBitmap(blockId, e, &inode);
        }else if(y == 12){
            uchar *singleLink = (uchar *)malloc(sizeof(uchar) * block_size);
            if(inode.i_block[y]== 2137){
                printf("This is %zu\n", 2137);
            }
            setLongBitmap(inode.i_block[y], e, &inode);
            readBlock(inode.i_block[y], singleLink, e);
            unsigned int  dblock[block_size / 4]; 
            memcpy(dblock, singleLink, block_size);
            size_t z;
            for(z = 0; z < block_size / 4; z++){
                if(dblock[z] != 0){
                    setLongBitmap(dblock[z], e, &inode);
                }
            }
        }else if(y == 13){
            int numBlocks = inode.i_blocks / (2 << sublk->s_log_block_size) - 12;
            checkDoubleBlock(&numBlocks, &inode, NULL, e, 13);   
        }
    }
}


size_t fileType(size_t imode){
    size_t c = imode & 0xf000;
    switch(c){
        case 0x1000:
            return 5;
        case 0x2000:
            return 3;
        case 0x4000:
            return 2;
        case 0x8000:
            return 1;
        case 0x6000:
            return 4;
        case 0xa000:
            return 7;	
        case 0xc000:
            return 6;
        default:
            return 0; 
    } 
}

size_t IntLen(size_t x) {
    if(x>=1000000000) return 10;
    if(x>=100000000) return 9;
    if(x>=10000000) return 8;
    if(x>=1000000) return 7;
    if(x>=100000) return 6;
    if(x>=10000) return 5;
    if(x>=1000) return 4;
    if(x>=100) return 3;
    if(x>=10) return 2;
    return 1;
}

void addDirEntry(size_t lostfoundNum, size_t lostInode, partition *p){
    // new method
    ext2_inode temp = getSectorNumOfiNode(lostfoundNum, p);
    ext2_inode *parentDir = &temp;
    size_t y;
    size_t dataSize = 0;
    uchar *singleLink = NULL;
    for(y = 0; y < 13; y++){
        if(parentDir->i_block[y] == 0){
            break;
        }
        if(y < 12){
            dataSize += block_size;
        }else{
            singleLink = (uchar *)malloc(sizeof(uchar) * block_size);	
            readBlock(parentDir->i_block[y], singleLink, p);
            size_t z;
            for(z = 0; z < block_size / 4; z++){
                if(*(__u32 *)(singleLink + z * 4) != 0){
                    dataSize += block_size;
                }else{
                    break;
                }
            }
        }
    }
    if(parentDir->i_block[12] != 0){
        printf("Yes. it matters\n");
    }
    uchar buf[dataSize];
    for(y = 0; y < 13; y++){
        if(parentDir->i_block[y] == 0){
            break;
        }
        if(y < 12){
            readBlock((size_t)parentDir->i_block[y], buf + y * block_size, p);
        }else{
            if(singleLink != NULL){
                size_t z;
                for(z = 0; z < block_size / 4; z++){
                    if(*(__u32 *)(singleLink + z) != 0){
                        readBlock(*(__u32 *)(singleLink + z),
                                  buf + (z + y) * block_size, p);
                    }else{
                        break;
                    }
                }
                free(singleLink);
            }
        }
    }
    
    size_t off = 0;
    ext2_dir_entry_2 *dir = (ext2_dir_entry_2 *)buf;
    ext2_dir_entry_2 *pre = NULL;
    while(true){
        if(off >= dataSize){
            break;
        }
        if(dir->inode == 0){
            ext2_inode lost = getSectorNumOfiNode(lostInode, p);
            dir->inode = lostInode;
            char str[128];
            sprintf(str, "%d", lostInode);
            dir->name_len = IntLen(lostInode); 
            memcpy(dir->name, str, dir->name_len);
            dir->file_type = fileType(lost.i_mode);
            //TODO: Bug exist
            dir->rec_len = block_size - off;
            if(pre != NULL){
                pre->rec_len = EXT2_DIR_REC_LEN(pre->name_len); 
            }else{
                printf("How could it be!!!\n");
            }
            int blockId = off / block_size;
            writeBlock(parentDir->i_block[blockId],
                       buf + blockId * block_size, p);
            if(blockId > 0){
                writeBlock(parentDir->i_block[blockId - 1],
                           buf + (blockId - 1) * block_size, p);
            }
            uchar	lostbuf[block_size];
            readBlock(lost.i_block[0], lostbuf, p);
            ext2_dir_entry_2 *pdir = (ext2_dir_entry_2 *)(lostbuf +
                                    ((ext2_dir_entry_2 *)lostbuf)->rec_len);
            pdir->inode = lostfoundNum;
            writeBlock(lost.i_block[0], lostbuf, p);					
            break; 
        }
        off += EXT2_DIR_REC_LEN(dir->name_len);
        pre = dir;
        dir = (ext2_dir_entry_2 *)((char *)buf + off);
    }
}
