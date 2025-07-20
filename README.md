# FileSystemOS

A simple block-based filesystem implementation in C, designed as a single-file image.  
This project provides a minimal "OnlyFiles" filesystem with basic file operations, intended for educational purposes in operating systems and filesystem design.

## Features

- Format a new filesystem image (`fs_format`)
- Mount and unmount a filesystem (`fs_mount`, `fs_unmount`)
- Create and delete files (`fs_create`, `fs_delete`)
- List files in the filesystem (`fs_list`)
- Read and write file data (`fs_read`, `fs_write`)

## Filesystem Layout

- **Block size**: 4 KB (`BLOCK_SIZE`)
- **Total blocks**: 2560 (`MAX_BLOCKS`) → ~10 MB image
- **Superblock** (block 0): metadata (total blocks, free blocks, inodes)
- **Block bitmap** (block 1): tracks allocated/free data blocks
- **Inode table** (blocks 2–9): up to 256 inodes (`MAX_FILES`), each with up to 12 direct block pointers (`MAX_DIRECT_BLOCKS`)
- **Data blocks** (blocks 10–2559): store file contents

For details, see the header definitions in [fs.h](fs.h).

## Directory Structure

```
.
├── build.sh         # build script
├── fs.c             # filesystem implementation
├── fs.h             # filesystem API & data structures
├── main.c           # example/demo program
├── run.sh           # run demo (`./fs_main`)
├── runTests.sh      # script to compile & run all tests
├── Test1.c          # unit tests for write/read edge cases
├── Test2.c          # unit tests for mount/delete/list operations
└── README.md        # this documentation
```

## Building

Make sure you have a C compiler (e.g., `gcc`) installed. Then run:

```sh
chmod +x build.sh
./build.sh
```

This compiles `fs.c` and `main.c` into the `fs_main` executable.

## Usage

1. Format a new disk image:

   ```sh
   ./fs_main
   ```

   (or manually call `fs_format("disk.img")`, `fs_mount("disk.img")`, etc., in your code)

2. The provided `main.c` demonstrates a basic workflow:

   - Format and mount the disk
   - Create a file (`file1.txt`)
   - Write and read data
   - Unmount

3. You can also use `run.sh` to launch the demo:
   ```sh
   chmod +x run.sh
   ./run.sh
   ```

## Unit Tests

Comprehensive unit tests are included to validate edge cases and robustness:

- **Test1.c**: write and read edge cases, rollback on failure, sparse reads
- **Test2.c**: mount, unmount, delete, list and combined operation edge cases

You can build and run all tests via:

```sh
chmod +x runTests.sh
./runTests.sh
```

These scripts compile each test with `fs.c` and report success/failure for each scenario.

## Contributing

Feel free to extend the filesystem with additional features such as indirect blocks, directories, or permissions. Pull requests and issues are welcome.

## Authors

- Eylon Iyov
- Omer Goldstein

Unit tests written by Eylon Iyov (see [Test1.c](Test1.c), [Test2.c](Test2.c)).
