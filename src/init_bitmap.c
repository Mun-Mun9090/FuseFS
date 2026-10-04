#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "fs.h"
int main(void)
{
	uint8_t bitmap[FS_BLOCK_SIZE];
	memset(bitmap, 0, sizeof(bitmap));
	int fd = open("fs.img", O_RDWR);
	if (fd == -1) {
    		perror("open");
    		return 1;
	}
	off_t bitmap_offset = 5 * FS_BLOCK_SIZE;

	ssize_t written = pwrite(fd, bitmap, sizeof(bitmap), bitmap_offset);

	if (written != sizeof(bitmap)) {
    		if (written == -1)
        		perror("pwrite");
    		else
        		fprintf(stderr, "Incomplete bitmap write\n");

    		close(fd);
    		return 1;
	}
	if (fsync(fd) == -1) {
    		perror("fsync");
    		close(fd);
    		return 1;
	}
	close(fd);

	printf("Bitmap initialized successfully.\n");

	return 0;
}
