#include "myfsck.h"
#include "mytool.h"

int main(int argc, char **argv){
	int sector = 0;
	unsigned char *buffer;
	int errno = NORMAL;

	buffer = (unsigned char *)malloc(sizeof(unsigned char) * SECTORSIZE);
	memset((void *)buffer, 0, SECTORSIZE);

    if ((device = open("disk", O_RDWR)) == -1) {
		perror("Could not open device file");
		exit(-1);
	}

	read_sectors(sector, 1, buffer);
	print_sector(buffer);
	

done:
	free(buffer);
	return errno;
error:
	goto done;
}
