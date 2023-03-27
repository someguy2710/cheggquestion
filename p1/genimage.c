#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>


#include "testlib.h"
#include "filesystem.h"
#include <unistd.h>
#include <stdio.h>

#define HELLO_SIZE 1

//char *files[] = { "hello", "secret", "chain", "alpha", "endian", NULL };

struct fileb {
     int blk;
     char *data;
};
struct fileb fileb[] = {
     { 2, "Hello World\nunused space        " },
     { 3, "abcdefghijklmnopqrstuvwxyzABCDEF"},
     { 5, "GHIJKLMNOPQRSTUVWXY"},
     { 6, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"},
     { 7, "STY23"},
     { 15, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"},
     { 9, "cccccccccccccccccccccccccccccccc"},
     { 10, "The terms endian and endianness "},
     { 11, "refer to the convention used to "},
     { 12, "interpret the bytes making up a "},
     { 13, "data word when those bytes are s"},
     { 14, "tored in computer memory"},
     { 16, ":-) sub-directory success"},

};
struct filed {
     char *fname;
     int blk;
     int len;
     int type;
};
struct filed filed[] = {
     // file information in root dir
     { "hello", 2, 0x0c, 1},
     { "alpha", 3, 0x34, 1 },
     { "endian", 10, 0x99, 1},
     { "chain", 6, 0x60, 1},
     { "user", 8, 16, 2},
     { "secret", 7, 5, 1},
};
#define NDIR 1

char *getblk(FileSystemHeader *mem, int blk) {
    char *ptr = (char *)mem;
    ptr += sizeof(FileSystemHeader);
    ptr += blk * BLOCK_SIZE;
    return ptr;
}

int main(int argc, char **argv)
{
    if(argc<2) { printf("Usage: %s image\n", argv[0]); return(1); }
    // Map the file system    
    int fd = open(argv[1], O_CREAT|O_RDWR, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    ftruncate(fd, sizeof(FileSystemHeader) + 32*32);

    FileSystemHeader *mem  = (FileSystemHeader *)mmap(NULL, 32*32 + sizeof(FileSystemHeader), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == NULL) {
        perror("mmap");
        return 1;
    }

    printf("putting data...\n");
    // put file block data into image
    for(int i=0; i<13; i++) {
        memcpy( getblk(mem, fileb[i].blk), fileb[i].data, BLOCK_SIZE );
    }
    // put dirents into root dir; root dir is at block 0,1,4
    mem->rootDirectorySize = NDIR * 16;
    DirectoryEntry dirent;
    char *ptr = getblk(mem, 0);
    printf("putting dir...\n");
    for(int i=0; i<NDIR; i++) {
        dirent.type = FTYPE_REGULAR;
        dirent.firstBlock = filed[i].blk;
        dirent.length = filed[i].len;
	dirent.type = filed[i].type;
        strcpy(dirent.name, filed[i].fname);
        *(DirectoryEntry *)ptr = dirent;
        ptr += sizeof(DirectoryEntry);
	if(i==3) ptr += BLOCK_SIZE*2;
    }
    // root dir
    mem->fat[0] = 1;
    mem->fat[1] = 4;
    mem->fat[4] = -1;

    // hello
    mem->fat[2] = -1;

    // alpha
    mem->fat[3] = 5;
    mem->fat[5] = -1; 

    // endian
    mem->fat[10] = 11;
    mem->fat[11] = 12;
    mem->fat[12] = 13;
    mem->fat[13] = 14;
    mem->fat[14] = -1;

    // chain
    mem->fat[6] = 15;
    mem->fat[15] = 9;
    mem->fat[9] = -1;

    // secret
    mem->fat[7] = -1;

    // user
    mem->fat[8] = -1;
    dirent.type = FTYPE_REGULAR;
    dirent.firstBlock = 16;
    dirent.length = 26;
    memcpy(dirent.name, "hello", 8);
    *(DirectoryEntry *)(getblk(mem,8)) = dirent;
 
    // test.txt
    mem->fat[16] = -1;

    munmap(mem, sizeof(FileSystemHeader) + 32*32);
 
    close(fd); 
}
