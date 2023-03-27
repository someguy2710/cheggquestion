#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <inttypes.h>

#define FILE_NAME_LENGTH 8
#define FILE_SYSTEM_BLOCKS 32
#define BLOCK_SIZE 32
#define ROOT_DIRECTORY_BLOCK 0
#define HEADER_SIZE (sizeof(FileSystemHeader))

typedef struct {
    char data[BLOCK_SIZE];
} FileSystemBlock;

typedef uint16_t block_t;


// The file system image provided with this assignment is structured as
// follows. The root directory is a file itself, which starts in block 0. You
// have to use the FAT to identify the next blocks of the root directory file.
typedef struct {
    block_t fat[FILE_SYSTEM_BLOCKS];   // The file allocation table (FAT)
    uint32_t rootDirectorySize;         // Root directory size in bytes.
} FileSystemHeader;

// Data structure to represent a file system in memory
// The header includes the cached full header from the file (FAT + root dir size)
// fd contains the file descriptor (used for reading data blocks)
typedef struct {
   FileSystemHeader header;
   int fd;
} FileSystem;

// Type of entries in a directory
#define FTYPE_REGULAR 1
#define FTYPE_DIRECTORY 2
#define FTYPE_DELETED 255

// For each file, 16 bytes (i.e., half a block) of meta data are stored on disk in
// the root directory file using little endian byte order.
typedef struct {
    uint16_t type;			// FTYPE_REGULAR or FTYPE_DIRECTORY
    block_t firstBlock;                 // Index of first data block.
    uint32_t length;                    // File size in bytes
    char name[FILE_NAME_LENGTH];        // File name
} DirectoryEntry;

typedef struct {
    uint32_t currentFileOffset; // Index of the next byte to read. Starts at 0.
    block_t currentBlock;       // Index of the current block
    uint32_t length;            // Copy of the file's length
    FileSystem *fileSystem;     // File system that owns the file
} OpenFileHandle;

FileSystem *initFileSystem(char *diskFile);

OpenFileHandle *openFile(FileSystem *fs, char *dir, char *name);
void closeFile(OpenFileHandle *handle);

int readFile(OpenFileHandle *handle, char *buffer, int length);

#endif
