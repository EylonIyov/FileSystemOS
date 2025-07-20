#include "fs.h"
#include <stdlib.h>

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"

/**
 * fs_mount() Edge Cases

Mount non-existent file - Try to mount a file that doesn't exist
Mount corrupted disk - Disk file with invalid superblock values
Mount already mounted disk - Call fs_mount() twice without unmounting
Mount empty file - 0-byte file
Mount file with wrong size - File smaller than expected 10MB
Mount with invalid metadata
         - Superblock shows more free blocks than total blocks
         - Mount with 0 inodes free - Superblock shows 0 free inodes
         - Mount disk with inconsistent bitmap - Bitmap doesn't match superblock free_blocks count
         - Mount with negative free blocks - Superblock shows negative free blocks
         - Mount with 0 free blocks - Superblock shows 0 free blocks
Mount disk with inconsistent bitmap - Bitmap doesn't match superblock free_blocks count
 */
// fs_mount() Edge Cases

void mount_non_existent_file()
{
    // Mount non-existent file - Try to mount a file that doesn't exist
    printf(YELLOW "Mount non-existent file - Try to mount a file that doesn't exist - Testing" RESET "\n");
    if (fs_mount("non_existent.img") != -1)
    {
        printf(RED "Mount non-existent file - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "Mount non-existent file - Success" RESET "\n");
    }
}

void mount_twice()
{
    // Mount already mounted disk - Call fs_mount() twice without unmounting
    printf(YELLOW "Mount already mounted disk - Call fs_mount() twice without unmounting - Testing" RESET "\n");
    if (fs_format("disk.img") == 0) // Format to create a new disk
    {
        printf(YELLOW "First format..." RESET "\n");
        if (fs_mount("disk.img") == 0)
        {
            printf(YELLOW " First Mount - Success" RESET "\n");
            if (fs_mount("disk.img") == 0)
            {
                printf(RED "Mount already mounted disk - Failed" RESET "\n");
                exit(-1);
            }

            printf(GREEN "Mount already mounted disk - Success" RESET "\n");
        }
        else
        {
            printf(RED "First Mount - Failed" RESET "\n");
            exit(-1);
        }
    }
    else
    {
        int result = fs_format("disk.img");
        printf(RED "Format Failed - Failed to format disk. fs_format returned: %d\n" RESET, result);
        exit(-1);
    }
    fs_unmount(); // Unmount after testing
}

void mount_corrupted_disk()
{
    // Mount corrupted disk - Disk file with invalid superblock values
    printf(YELLOW "Mount corrupted disk - Disk file with invalid superblock values - Testing" RESET "\n");
    if (fs_mount("corrupted_disk.img") != -1)
    {
        printf(RED "Mount corrupted disk - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "Mount corrupted disk - Success" RESET "\n");
    }
}

void mount_empty_file()
{
    // Mount empty file - 0-byte file
    printf(YELLOW "Mount empty file - 0-byte file - Testing" RESET "\n");
    if (fs_mount("empty_file.img") != -1)
    {
        printf(RED "Mount empty file - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "Mount empty file - Success" RESET "\n");
    }
}

void mount_file_with_larger_size()
{
    // Mount file with wrong size - File larger than expected 10MB
    printf(YELLOW "Mount file with wrong size - File larger than expected 10MB - Testing" RESET "\n");
    if (fs_mount("large_file.img") != -1)
    {
        printf(RED "Mount file with wrong size, Large - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "Mount file with wrong size, Large - Success" RESET "\n");
    }
}

void mount_with_invalid_metadata()
{
    // Mount with invalid metadata - Superblock shows more free blocks than total blocks
    printf(YELLOW "Mount with invalid metadata - Superblock shows more free blocks than total blocks - Testing" RESET "\n");
    if (fs_mount("invalid_metadata.img") != -1)
    {
        printf(RED "Mount with invalid metadata - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "Mount with invalid metadata - Success" RESET "\n");
    }

    // Mount disk with inconsistent bitmap - Bitmap doesn't match superblock free_blocks count
    printf(YELLOW "Mount disk with inconsistent bitmap - Bitmap doesn't match superblock free_blocks count - Testing" RESET "\n");
    if (fs_mount("inconsistent_bitmap.img") != -1)
    {
        printf(RED "Mount disk with inconsistent bitmap - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "Mount disk with inconsistent bitmap - Success" RESET "\n");
    }
    // Mount with 0 free blocks - Superblock shows 0 free blocks
    printf(YELLOW "Mount with 0 free blocks - Superblock shows 0 free blocks - Testing" RESET "\n");
    if (fs_mount("zero_free_blocks.img") != -1)
    {
        printf(RED "Mount with 0 free blocks - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "Mount with 0 free blocks - Success" RESET "\n");
    }

    // Mount with negative free blocks - Superblock shows negative free blocks
    printf(YELLOW "Mount with negative free blocks - Superblock shows negative free blocks - Testing" RESET "\n");
    if (fs_mount("negative_free_blocks.img") != -1)
    {
        printf(RED "Mount with negative free blocks - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "Mount with negative free blocks - Success" RESET "\n");
    }

    // Mount with 0 inodes free - Superblock shows 0 free inodes
    printf(YELLOW "Mount with 0 inodes free - Superblock shows 0 free inodes - Testing" RESET "\n");
    if (fs_mount("zero_free_inodes.img") != -1)
    {
        printf(RED "Mount with 0 inodes free - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "Mount with 0 inodes free - Success" RESET "\n");
    }

    // add more invalid metadata tests as needed
}

void fs_mount_tests()
{
    mount_non_existent_file();     // Test mounting a non-existent file
    mount_corrupted_disk();        // Test mounting a corrupted disk
    mount_twice();                 // Test mounting twice without unmounting
    mount_empty_file();            // Test mounting an empty file
    mount_file_with_larger_size(); // Test mounting a file with wrong size
    mount_with_invalid_metadata(); // Test mounting with invalid metadata
}

/**
 *
fs_delete() Edge Cases

Delete non-existent file - File doesn't exist
Delete with null filename - Pass NULL
Delete empty filename - Pass ""
Delete file with allocated blocks - Ensure all blocks are freed
Delete last file - When only one file remains
Delete file with maximum blocks - File using all 12 direct blocks
Delete partially written file - File with some blocks allocated, some not
 */

// fs_delete() Edge Cases
void non_existent_file_delete()
{
    // Delete non-existent file - File doesn't exist
    printf(YELLOW "Delete non-existent file - File doesn't exist - Testing" RESET "\n");
    if (fs_delete("non_existent.txt") != -1)
    {
        printf(RED "Delete non-existent file - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "Delete non-existent file - Success" RESET "\n");
    }
}

void null_filename_delete()
{
    // Delete with null filename - Pass NULL
    printf(YELLOW "Delete with null filename - Pass NULL - Testing" RESET "\n");
    if (fs_delete(NULL) != -1)
    {
        printf(RED "Delete with null filename - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "Delete with null filename - Success" RESET "\n");
    }
}

void empty_filename_delete()
{
    // Delete empty filename - Pass ""
    printf(YELLOW "Delete empty filename - Pass \"\" - Testing" RESET "\n");
    if (fs_delete("") != -1)
    {
        printf(RED "Delete empty filename - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "Delete empty filename - Success" RESET "\n");
    }
}

void fs_delete_tests()
{
    non_existent_file_delete(); // Test deleting a non-existent file
    null_filename_delete();     // Test deleting with null filename
    empty_filename_delete();    // Test deleting empty filename
}

// fs_list() Edge Cases
/**

List with null array - Pass NULL for filenames array
List with max_files = 0 - Request 0 files
List with max_files = -1 - Negative value
List with max_files > MAX_FILES - Request more than 256
List when no files exist - Empty filesystem
List when filesystem is full - All 256 files exist
List with max_files < actual files - 10 files exist, request only 5
*/

void null_array_list()
{
    // List with null array - Pass NULL for filenames array
    printf(YELLOW "List with null array - Pass NULL for filenames array - Testing" RESET "\n");
    if (fs_list(NULL, 10) != -1)
    {
        printf(RED "List with null array - Failed" RESET "\n");
        exit(-1);
    }
    else
    {
        printf(GREEN "List with null array - Success" RESET "\n");
    }
}

void list_with_zero_max_files()
{
    printf(YELLOW "List with max_files = 0 - Testing" RESET "\n");
    char filenames[1][MAX_FILENAME];
    if (fs_list(filenames, 0) != 0)
    {
        printf(RED "List with max_files = 0 - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "List with max_files = 0 - Success" RESET "\n");
}

void list_with_negative_max_files()
{
    printf(YELLOW "List with max_files = -1 - Testing" RESET "\n");
    char filenames[1][MAX_FILENAME];
    if (fs_list(filenames, -1) != -1)
    {
        printf(RED "List with max_files = -1 - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "List with max_files = -1 - Success" RESET "\n");
}

void list_with_max_files_above_limit()
{
    printf(YELLOW "List with max_files > MAX_FILES - Testing" RESET "\n");
    char filenames[300][MAX_FILENAME];
    int count = fs_list(filenames, 300);
    if (count != -1)
    {
        printf(RED "List with max_files > MAX_FILES - Failed (returned %d)\n" RESET, count);
        exit(-1);
    }
    printf(GREEN "List with max_files > MAX_FILES - Success (returned %d)\n" RESET, count);
}

void list_when_no_files_exist()
{
    printf(YELLOW "List when no files exist - Empty filesystem - Testing" RESET "\n");
    const char *path = "test_imgs/empty_fs.img";
    fs_format(path);
    fs_mount(path);

    char filenames[MAX_FILES][MAX_FILENAME];
    int count = fs_list(filenames, MAX_FILES);
    if (count != 0)
    {
        printf(RED "List when no files exist - Failed (returned %d)\n" RESET, count);
        exit(-1);
    }
    printf(GREEN "List when no files exist - Success\n" RESET);
    fs_unmount();
}

void list_when_filesystem_is_full()
{
    printf(YELLOW "List when filesystem is full - Testing" RESET "\n");
    const char *path = "test_imgs/full_fs.img";
    fs_format(path);
    fs_mount(path);

    char name[MAX_FILENAME];
    for (int i = 0; i < MAX_FILES; ++i)
    {
        snprintf(name, MAX_FILENAME, "file_%03d", i);
        if (fs_create(name) != 0)
        {
            printf(RED "Failed to create file %s\n" RESET, name);
            exit(-1);
        }
    }

    char filenames[MAX_FILES][MAX_FILENAME];
    int count = fs_list(filenames, MAX_FILES);
    if (count != MAX_FILES)
    {
        printf(RED "List when filesystem is full - Failed (returned %d)\n" RESET, count);
        exit(-1);
    }
    printf(GREEN "List when filesystem is full - Success\n" RESET);
    fs_unmount();
}

void list_with_max_files_less_than_actual()
{
    printf(YELLOW "List with max_files < actual files - Testing" RESET "\n");
    const char *path = "test_imgs/partial_fs.img";
    fs_format(path);
    fs_mount(path);

    for (int i = 0; i < 10; ++i)
    {
        char name[MAX_FILENAME];
        snprintf(name, MAX_FILENAME, "file_%d", i);
        if (fs_create(name) != 0)
        {
            printf(RED "Failed to create file %s\n" RESET, name);
            exit(-1);
        }
    }

    char filenames[5][MAX_FILENAME];
    int count = fs_list(filenames, 5);
    if (count != 5)
    {
        printf(RED "List with max_files < actual files - Failed (returned %d)\n" RESET, count);
        exit(-1);
    }
    printf(GREEN "List with max_files < actual files - Success\n" RESET);
    fs_unmount();
}

void fs_list_tests()
{
    null_array_list();                      // Test listing with null array
    list_with_zero_max_files();             // Test listing with max_files = 0
    list_with_negative_max_files();         // Test listing with max_files = -1
    list_with_max_files_above_limit();      // Test listing with max_files > MAX_FILES
    list_when_no_files_exist();             // Test listing when no files exist
    list_when_filesystem_is_full();         // Test listing when filesystem is full
    list_with_max_files_less_than_actual(); // Test listing with max_files < actual files
    printf(GREEN "fs_list tests completed successfully." RESET "\n");
}

// combined_operation_tests
/**
 *
Combined Operation Edge Cases

Create-Write-Delete-Create cycle - Reuse same filename
Fill filesystem completely - Create 256 files, each with data
Fragment and defragment - Create/delete files to fragment free space
Write after failed write - First write fails due to space, free space, retry
Mount-unmount-mount cycle - Ensure data persists
Operations without mount - Try all operations before mounting
 */
void mount_unmount_cycle()
{
    printf(YELLOW "Mount-unmount-mount cycle x10 - Ensure data persists - Testing" RESET "\n");

    const char *path = "test_imgs/mount_cycle.img";
    const char *filename = "test_file.txt";
    char data[BLOCK_SIZE] = "Hello, World!";

    for (int i = 0; i < 200; ++i)
    {
        if (fs_format(path) != 0)
        {
            printf(RED "Failed to format disk on cycle %d\n" RESET, i + 1);
            exit(-1);
        }

        if (fs_mount(path) != 0)
        {
            printf(RED "Failed to mount disk on cycle %d\n" RESET, i + 1);
            exit(-1);
        }

        if (fs_create(filename) != 0)
        {
            printf(RED "Failed to create file on cycle %d\n" RESET, i + 1);
            exit(-1);
        }

        if (fs_write(filename, data, strlen(data) + 1) != 0)
        {
            printf(RED "Failed to write file on cycle %d\n" RESET, i + 1);
            exit(-1);
        }

        fs_unmount();

        if (fs_mount(path) != 0)
        {
            printf(RED "Failed to remount disk on cycle %d\n" RESET, i + 1);
            exit(-1);
        }

        char read_data[BLOCK_SIZE];
        if (fs_read(filename, read_data, BLOCK_SIZE) < 0)
        {
            printf(RED "Failed to read file on cycle %d\n" RESET, i + 1);
            exit(-1);
        }

        if (strcmp(read_data, data) != 0)
        {
            printf(RED "Data mismatch on cycle %d: expected '%s', got '%s'\n", i + 1, data, read_data);
            exit(-1);
        }

        fs_unmount();
    }

    printf(GREEN "Mount-unmount-mount cycle x10 - Success" RESET "\n");
}

void create_write_delete_create_cycle()
{
    printf(YELLOW "Create-Write-Delete-Create cycle x10 - Reuse same filename - Testing" RESET "\n");

    const char *path = "test_imgs/cycle.img";
    if (fs_format(path) != 0)
    {
        printf(RED "Failed to format disk\n" RESET);
        exit(-1);
    }

    if (fs_mount(path) != 0)
    {
        printf(RED "Failed to mount disk\n" RESET);
        exit(-1);
    }

    const char *filename = "test_file.txt";
    char data[BLOCK_SIZE] = "Hello, World!";

    for (int i = 0; i < 260; ++i)
    {
        if (fs_create(filename) != 0)
        {
            printf(RED "Failed to create file on cycle %d\n" RESET, i + 1);
            exit(-1);
        }

        if (fs_write(filename, data, strlen(data) + 1) != 0)
        {
            printf(RED "Failed to write on cycle %d\n" RESET, i + 1);
            exit(-1);
        }

        if (fs_delete(filename) != 0)
        {
            printf(RED "Failed to delete file on cycle %d\n" RESET, i + 1);
            exit(-1);
        }
    }

    printf(GREEN "Create-Write-Delete-Create cycle x10 - Success" RESET "\n");

    fs_unmount();
}

void fill_filesystem_completely()
{
    // Fill filesystem completely - Create 256 files, each with data
    printf(YELLOW "Fill filesystem completely - Create 256 files, each with data - Testing" RESET "\n");
    const char *path = "test_imgs/full_fs.img";
    fs_format(path);
    fs_mount(path);

    char name[MAX_FILENAME];
    for (int i = 0; i < MAX_FILES; ++i)
    {
        snprintf(name, MAX_FILENAME, "file_%03d", i);
        if (fs_create(name) != 0)
        {
            printf(RED "Failed to create file %s\n" RESET, name);
            exit(-1);
        }
        char data[BLOCK_SIZE] = {0}; // Initialize data to zero
        if (fs_write(name, data, BLOCK_SIZE) != 0)
        {
            printf(RED "Failed to write to file %s\n" RESET, name);
            exit(-1);
        }
    }

    printf(GREEN "Fill filesystem completely - Success" RESET "\n");
    fs_unmount();
}

void operations_without_mount()
{
    // Operations without mount - Try all operations before mounting
    printf(YELLOW "Operations without mount - Testing" RESET "\n");

    if (fs_create("test_file.txt") == 0)
    {
        printf(RED "Create without mount - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Create without mount - Success" RESET "\n");

    if (fs_write("test_file.txt", "data", 4) == 0)
    {
        printf(RED "Write without mount - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Write without mount - Success" RESET "\n");

    if (fs_read("test_file.txt", NULL, 4) == 0)
    {
        printf(RED "Read without mount - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Read without mount - Success" RESET "\n");

    if (fs_delete("test_file.txt") == 0)
    {
        printf(RED "Delete without mount - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Delete without mount - Success" RESET "\n");
}

void write_after_failed_write()
{
    // Write after failed write - First write fails due to space, free space, retry
    printf(YELLOW "Write after failed write - Testing" RESET "\n");

    const char *path = "test_imgs/write_retry.img";
    fs_format(path);
    fs_mount(path);

    const char *filename = "test_file.txt";
    char data[BLOCK_SIZE] = "Hello, World!";

    if (fs_create(filename) != 0)
    {
        printf(RED "Failed to create file\n" RESET);
        exit(-1);
    }

    // Simulate a failed write due to insufficient space
    if (fs_write(filename, data, BLOCK_SIZE * 13) == 0)
    {
        printf(RED "Write with insufficient space unexpectedly succeeded\n" RESET);
        exit(-1);
    }

    // Free up space and retry
    if (fs_delete(filename) != 0)
    {
        printf(RED "Failed to delete file\n" RESET);
        exit(-1);
    }

    if (fs_create(filename) != 0)
    {
        printf(RED "Failed to recreate file\n" RESET);
        exit(-1);
    }

    if (fs_write(filename, data, strlen(data) + 1) != 0)
    {
        printf(RED "Retry write after freeing space - Failed\n" RESET);
        exit(-1);
    }

    printf(GREEN "Write after failed write - Success" RESET "\n");
    fs_unmount();
}

void fragment_and_defragment()
{
    // Fragment and defragment - Create/delete files to fragment free space
    printf(YELLOW "Fragment and defragment - Testing" RESET "\n");

    const char *path = "test_imgs/fragmentation.img";
    fs_format(path);
    fs_mount(path);

    char name[MAX_FILENAME];
    for (int i = 0; i < 10; ++i)
    {
        snprintf(name, MAX_FILENAME, "file_%03d", i);
        if (fs_create(name) != 0)
        {
            printf(RED "Failed to create file %s\n" RESET, name);
            exit(-1);
        }
        char data[BLOCK_SIZE] = {0}; // Initialize data to zero
        if (fs_write(name, data, BLOCK_SIZE) != 0)
        {
            printf(RED "Failed to write to file %s\n" RESET, name);
            exit(-1);
        }
    }

    // Delete some files to create fragmentation
    for (int i = 0; i < 5; ++i)
    {
        snprintf(name, MAX_FILENAME, "file_%03d", i);
        if (fs_delete(name) != 0)
        {
            printf(RED "Failed to delete file %s\n" RESET, name);
            exit(-1);
        }
    }

    // Check fragmentation by trying to create a new file
    snprintf(name, MAX_FILENAME, "new_file.txt");
    if (fs_create(name) != 0)
    {
        printf(RED "Failed to create new file after fragmentation\n" RESET);
        exit(-1);
    }

    printf(GREEN "Fragment and defragment - Success" RESET "\n");
    fs_unmount();
}

void combined_operation_tests()
{
    create_write_delete_create_cycle(); // Test create-write-delete-create cycle
    fill_filesystem_completely();       // Test filling filesystem completely
    mount_unmount_cycle();              // Test mount-unmount-mount cycle
    operations_without_mount();         // Test operations without mounting
    write_after_failed_write();         // Test write after failed write
    fragment_and_defragment();          // Test fragment and defragment
    printf(GREEN "Combined operation tests completed successfully." RESET "\n");
}
/**
 * fs_unmount() Edge Cases

Unmount without mount – Call fs_unmount() without mounting first
Unmount after basic mount – Format → mount → unmount
Unmount after file write – Create and write to file → unmount
Unmount after multiple operations – Create, write, delete multiple files → unmount
Double unmount – Call fs_unmount() twice in a row
Unmount with no changes – Mount and immediately unmount without any file operations
Unmount with full disk – Fill disk completely → unmount
Unmount with full inode table – Create maximum number of files → unmount
Unmount after metadata update – Change superblock (e.g., free block count) → unmount → check persisted values
Unmount during open file handle (optional) – Simulate open file handle or in-use block → unmount
Unmount followed by remount – Write data → unmount → remount → check data integrity
Unmount with unsynced changes (failure test) – Modify in-memory state, skip sync → simulate crash (advanced)
 */

// Helper to check file contents after remount
void check_file_contents(const char *filename, const char *expected_data)
{
    char buffer[BLOCK_SIZE];
    if (fs_read(filename, buffer, BLOCK_SIZE) < 0)
    {
        printf(RED "Failed to read file '%s'\n" RESET, filename);
        exit(-1);
    }
    if (strcmp(buffer, expected_data) != 0)
    {
        printf(RED "File content mismatch for '%s': expected '%s', got '%s'\n" RESET, filename, expected_data, buffer);
        exit(-1);
    }
}

// 1. Unmount without mount
void test_unmount_without_mount()
{
    printf(YELLOW "Test: Unmount without mount\n" RESET);
    fs_unmount(); // Should do nothing / no crash
    printf(GREEN "Success\n" RESET);
}

// 2. Unmount after basic mount
void test_unmount_after_basic_mount()
{
    printf(YELLOW "Test: Unmount after basic mount\n" RESET);
    const char *path = "test_imgs/basic_mount.img";
    fs_format(path);
    fs_mount(path);
    fs_unmount();
    printf(GREEN "Success\n" RESET);
}

// 3. Unmount after file write
void test_unmount_after_file_write()
{
    printf(YELLOW "Test: Unmount after file write\n" RESET);
    const char *path = "test_imgs/file_write.img";
    fs_format(path);
    fs_mount(path);

    const char *filename = "file.txt";
    const char *data = "Hello, FS!";
    if (fs_create(filename) != 0 || fs_write(filename, data, strlen(data) + 1) != 0)
    {
        printf(RED "Failed to create/write file\n" RESET);
        exit(-1);
    }

    fs_unmount();

    if (fs_mount(path) != 0)
    {
        printf(RED "Failed to remount\n" RESET);
        exit(-1);
    }

    check_file_contents(filename, data);
    fs_unmount();
    printf(GREEN "Success\n" RESET);
}

// 4. Unmount after multiple operations
void test_unmount_after_multiple_operations()
{
    printf(YELLOW "Test: Unmount after multiple operations\n" RESET);
    const char *path = "test_imgs/multiple_ops.img";
    fs_format(path);
    fs_mount(path);

    char data[BLOCK_SIZE];
    for (int i = 0; i < 5; i++)
    {
        char filename[32];
        sprintf(filename, "file%d.txt", i);
        sprintf(data, "Data %d", i);

        if (fs_create(filename) != 0 || fs_write(filename, data, strlen(data) + 1) != 0)
        {
            printf(RED "Failed create/write %s\n" RESET, filename);
            exit(-1);
        }
    }
    // Delete file2 and file4
    fs_delete("file2.txt");
    fs_delete("file4.txt");

    fs_unmount();

    fs_mount(path);

    // Check existing files
    for (int i = 0; i < 5; i++)
    {
        char filename[32];
        sprintf(filename, "file%d.txt", i);
        if (i == 2 || i == 4)
        {
            if (fs_read(filename, data, BLOCK_SIZE) != -1)
            {
                printf(RED "%s should have been deleted\n" RESET, filename);
                exit(-1);
            }
        }
        else
        {
            sprintf(data, "Data %d", i);
            check_file_contents(filename, data);
        }
    }
    fs_unmount();
    printf(GREEN "Success\n" RESET);
}

// 5. Double unmount
void test_double_unmount()
{
    printf(YELLOW "Test: Double unmount\n" RESET);
    const char *path = "test_imgs/double_unmount.img";
    fs_format(path);
    fs_mount(path);
    fs_unmount();
    fs_unmount(); // Should be safe, no crash
    printf(GREEN "Success\n" RESET);
}

// 6. Unmount with no changes
void test_unmount_no_changes()
{
    printf(YELLOW "Test: Unmount with no changes\n" RESET);
    const char *path = "test_imgs/no_changes.img";
    fs_format(path);
    fs_mount(path);
    fs_unmount();
    printf(GREEN "Success\n" RESET);
}

// 7. Unmount with full disk
void test_unmount_full_disk()
{
    printf(YELLOW "Test: Unmount with full disk\n" RESET);
    const char *path = "test_imgs/full_disk.img";
    fs_format(path);
    fs_mount(path);

    const char *filename = "bigfile.txt";
    char data[BLOCK_SIZE];
    memset(data, 'A', BLOCK_SIZE);

    int blocks_written = 0;
    int res;

    while (1)
    {
        char file_chunk[64];
        sprintf(file_chunk, "%s_%d", filename, blocks_written);

        res = fs_create(file_chunk);
        if (res != 0)
        {
            printf(YELLOW "fs_create failed after %d files\n" RESET, blocks_written);
            break;
        }

        res = fs_write(file_chunk, data, BLOCK_SIZE);
        if (res != 0)
        {
            printf(YELLOW "fs_write failed after %d files\n" RESET, blocks_written);
            break;
        }

        blocks_written++;
    }

    if (blocks_written == 0)
    {
        printf(RED "Failed to write any blocks to fill disk\n" RESET);
        exit(-1);
    }

    fs_unmount();

    fs_mount(path);

    // Check contents of all files that were created
    for (int i = 0; i < blocks_written; i++)
    {
        char file_chunk[64];
        sprintf(file_chunk, "%s_%d", filename, i);

        char read_buffer[BLOCK_SIZE];
        int read_bytes = fs_read(file_chunk, read_buffer, BLOCK_SIZE);
        if (read_bytes != BLOCK_SIZE)
        {
            printf(RED "Read size mismatch for '%s': expected %d, got %d\n", file_chunk, BLOCK_SIZE, read_bytes);
            exit(-1);
        }
        if (memcmp(read_buffer, data, BLOCK_SIZE) != 0)
        {
            printf(RED "File content mismatch for '%s'\n", file_chunk);
            exit(-1);
        }
    }

    fs_unmount();

    printf(GREEN "Success\n" RESET);
}

// 8. Unmount with full inode table
void test_unmount_full_inode_table()
{
    printf(YELLOW "Test: Unmount with full inode table\n" RESET);
    const char *path = "test_imgs/full_inode.img";
    fs_format(path);
    fs_mount(path);

    char filename[32];
    for (int i = 0; i < MAX_FILES; i++)
    {
        sprintf(filename, "file%d.txt", i);
        if (fs_create(filename) != 0)
        {
            printf(RED "Failed to create file %d\n" RESET, i);
            exit(-1);
        }
    }

    fs_unmount();

    fs_mount(path);

    for (int i = 0; i < MAX_FILES; i++)
    {
        sprintf(filename, "file%d.txt", i);
        if (fs_read(filename, filename, BLOCK_SIZE) < 0)
        {
            printf(RED "File %s missing after remount\n" RESET, filename);
            exit(-1);
        }
    }

    fs_unmount();
    printf(GREEN "Success\n" RESET);
}

// 10. Unmount during open file handle (optional, simulate)
void test_unmount_during_open_file()
{
    printf(YELLOW "Test: Unmount during open file handle (simulated)\n" RESET);
    const char *path = "test_imgs/open_file.img";
    fs_format(path);
    fs_mount(path);

    const char *filename = "openfile.txt";
    char data[BLOCK_SIZE] = "Data";

    fs_create(filename);
    fs_write(filename, data, strlen(data) + 1);

    // Simulate open file handle by NOT closing or finishing something specific
    // (Your fs likely does not track open file handles in this simple FS)
    // So we just call unmount to test stability

    fs_unmount();
    printf(GREEN "Success\n" RESET);
}

// 11. Unmount followed by remount
void test_unmount_followed_by_remount()
{
    printf(YELLOW "Test: Unmount followed by remount\n" RESET);
    const char *path = "test_imgs/unmount_remount.img";
    fs_format(path);
    fs_mount(path);

    const char *filename = "test.txt";
    const char *data = "Test data";

    fs_create(filename);
    fs_write(filename, data, strlen(data) + 1);

    fs_unmount();

    fs_mount(path);
    check_file_contents(filename, data);

    fs_unmount();
    printf(GREEN "Success\n" RESET);
}

void fs_unmount_tests()
{
    test_unmount_without_mount();             // Test unmount without mount
    test_unmount_after_basic_mount();         // Test unmount after basic mount
    test_unmount_after_file_write();          // Test unmount after file write
    test_unmount_after_multiple_operations(); // Test unmount after multiple operations
    test_double_unmount();                    // Test double unmount
    test_unmount_no_changes();                // Test unmount with no changes
    test_unmount_full_disk();                 // Test unmount with full disk
    test_unmount_full_inode_table();          // Test unmount with full inode table
    test_unmount_during_open_file();          // Test unmount during open file handle (simulated)
    test_unmount_followed_by_remount();       // Test unmount followed by remount
    printf(GREEN "fs_unmount tests completed successfully." RESET "\n");
}

// robestness_tests()
void test_inode_consistency_api_only()
{
    printf(YELLOW "Test: Inode consistency via API\n" RESET);

    const char *path = "test_imgs/inode_api.img";
    fs_format(path);
    fs_mount(path);

    int max_files = 0;
    char filename[64];

    while (1)
    {
        snprintf(filename, sizeof(filename), "file_%d.txt", max_files);
        if (fs_create(filename) != 0)
            break;
        max_files++;
    }

    if (max_files == 0)
    {
        printf(RED "Could not create any files\n" RESET);
        exit(-1);
    }

    for (int i = 0; i < max_files / 2; i++)
    {
        snprintf(filename, sizeof(filename), "file_%d.txt", i);
        if (fs_delete(filename) != 0)
        {
            printf(RED "Failed to delete %s\n" RESET, filename);
            exit(-1);
        }
    }

    int reused = 0;
    while (1)
    {
        snprintf(filename, sizeof(filename), "reused_%d.txt", reused);
        if (fs_create(filename) != 0)
            break;
        reused++;
    }

    if (reused < max_files / 2)
    {
        printf(RED "Expected at least %d inodes to be reusable, got %d\n" RESET, max_files / 2, reused);
        exit(-1);
    }

    fs_unmount();
    printf(GREEN "Inode consistency via API - Success\n" RESET);
}

void test_block_allocation_rollback_api_only()
{
    printf(YELLOW "Test: Block allocation rollback via API\n" RESET);

    const char *path = "test_imgs/rollback_api.img";
    fs_format(path);
    fs_mount(path);

    if (fs_create("failwrite.txt") != 0)
    {
        printf(RED "Failed to create failwrite.txt\n" RESET);
        exit(-1);
    }

    size_t over_size = BLOCK_SIZE * 10000;
    char *data = malloc(over_size);
    memset(data, 'X', over_size);

    int result = fs_write("failwrite.txt", data, over_size);
    free(data);

    if (result == 0)
    {
        printf(RED "Unexpected: Huge write succeeded\n" RESET);
        exit(-1);
    }

    if (fs_create("check.txt") != 0)
    {
        printf(RED "Failed to create check.txt after failed write\n" RESET);
        exit(-1);
    }

    char buffer[BLOCK_SIZE];
    memset(buffer, 'Y', BLOCK_SIZE);

    if (fs_write("check.txt", buffer, BLOCK_SIZE) != 0)
    {
        printf(RED "Write failed after previous failure\n" RESET);
        exit(-1);
    }

    fs_unmount();
    printf(GREEN "Block allocation rollback via API - Success\n" RESET);
}

void test_cross_boundary_write()
{
    printf(YELLOW "Test: Cross-boundary write\n" RESET);

    const char *path = "test_imgs/cross_boundary.img";
    fs_format(path);
    fs_mount(path);

    const char *filename = "boundary.txt";
    int bytes_to_write = BLOCK_SIZE + 2000;

    if (fs_create(filename) != 0)
    {
        printf(RED "Failed to create %s\n" RESET, filename);
        exit(-1);
    }

    char *data = malloc(bytes_to_write);
    for (int i = 0; i < bytes_to_write; i++)
        data[i] = (char)(i % 256);

    if (fs_write(filename, data, bytes_to_write) != 0)
    {
        printf(RED "Write failed\n" RESET);
        exit(-1);
    }

    char *read_buf = malloc(bytes_to_write);
    int read = fs_read(filename, read_buf, bytes_to_write);

    if (read != bytes_to_write)
    {
        printf(RED "Read failed: got %d bytes\n" RESET, read);
        exit(-1);
    }

    for (int i = 0; i < bytes_to_write; i++)
    {
        if (read_buf[i] != data[i])
        {
            printf(RED "Mismatch at byte %d\n" RESET, i);
            exit(-1);
        }
    }

    free(data);
    free(read_buf);

    fs_unmount();
    printf(GREEN "Cross-boundary write - Success\n" RESET);
}

void test_power_failure_simulation()
{
    printf(YELLOW "Test: Power failure simulation (manual)\n" RESET);

    const char *path = "test_imgs/power_fail.img";
    fs_format(path);
    fs_mount(path);

    if (fs_create("crash.txt") != 0)
    {
        printf(RED "Failed to create crash.txt\n" RESET);
        exit(-1);
    }

    char buf[BLOCK_SIZE];
    memset(buf, 'Z', BLOCK_SIZE);
    fs_write("crash.txt", buf, BLOCK_SIZE);

    printf(YELLOW "Simulate crash now: kill this process or unplug\n" RESET);
    printf("Waiting 10 seconds...\n");
    sleep(10);

    fs_unmount();
    printf(GREEN "Manual power failure test completed (check consistency)\n" RESET);
}

void test_maximum_stress()
{
    printf(YELLOW "Test: Maximum stress (fill, delete, refill)\n" RESET);

    const char *path = "test_imgs/stress.img";
    fs_format(path);
    fs_mount(path);

    char data[BLOCK_SIZE];
    memset(data, 'S', BLOCK_SIZE);

    char filename[64];
    int created = 0;

    while (1)
    {
        snprintf(filename, sizeof(filename), "sfile_%d", created);
        if (fs_create(filename) != 0)
            break;
        if (fs_write(filename, data, BLOCK_SIZE) != 0)
            break;
        created++;
    }

    for (int i = 0; i < created / 2; i++)
    {
        snprintf(filename, sizeof(filename), "sfile_%d", i);
        fs_delete(filename);
    }

    int refill = 0;
    while (1)
    {
        snprintf(filename, sizeof(filename), "refill_%d", refill);
        if (fs_create(filename) != 0)
            break;
        if (fs_write(filename, data, BLOCK_SIZE) != 0)
            break;
        refill++;
    }

    fs_unmount();
    printf(GREEN "Maximum stress - Success (created %d, deleted %d, refilled %d)\n" RESET, created, created / 2, refill);
}
void robustness_tests()
{
    test_inode_consistency_api_only();         // Test inode consistency via API
    test_block_allocation_rollback_api_only(); // Test block allocation rollback via API
    test_cross_boundary_write();               // Test cross-boundary write
    test_power_failure_simulation();           // Test power failure simulation (manual)
    test_maximum_stress();                     // Test maximum stress (fill, delete, refill)
    printf(GREEN "Robustness tests completed successfully." RESET "\n");
}
void main()
{
    fs_mount_tests();
    fs_unmount_tests();         // Ensure unmount is called before any other operations
    fs_delete_tests();          // Uncomment when fs_delete() is implemented
    fs_list_tests();            // Uncomment when fs_list() is implemented
    combined_operation_tests(); // Uncomment when combined operations are implemented
    robustness_tests();         // Run robustness tests

    printf(GREEN "All tests completed successfully." RESET "\n");
}

/**
 * fs_format() Edge Cases

Null path parameter - Pass NULL as disk_path
Empty string path - Pass "" as disk_path
Path with special characters - Use paths like "test/../../disk.img"
Very long path name - Path exceeding system limits
Format twice without unmount - Call fs_format() on an already mounted filesystem
Disk in read-only directory - Try to create disk in a directory without write permissions
Disk already exists and is locked - Another process has the file open

fs_mount() Edge Cases

Mount non-existent file - Try to mount a file that doesn't exist
Mount corrupted disk - Disk file with invalid superblock values
Mount already mounted disk - Call fs_mount() twice without unmounting
Mount empty file - 0-byte file
Mount file with wrong size - File smaller than expected 10MB
Mount with invalid metadata - Superblock shows more free blocks than total blocks
Mount disk with inconsistent bitmap - Bitmap doesn't match superblock free_blocks count

fs_unmount() Edge Cases

Unmount without mount – Call fs_unmount() without mounting first
Unmount after basic mount – Format → mount → unmount
Unmount after file write – Create and write to file → unmount
Unmount after multiple operations – Create, write, delete multiple files → unmount
Double unmount – Call fs_unmount() twice in a row
Unmount with no changes – Mount and immediately unmount without any file operations
Unmount with full disk – Fill disk completely → unmount
Unmount with full inode table – Create maximum number of files → unmount
Unmount after metadata update – Change superblock (e.g., free block count) → unmount → check persisted values
Unmount during open file handle (optional) – Simulate open file handle or in-use block → unmount
Unmount followed by remount – Write data → unmount → remount → check data integrity
Unmount with unsynced changes (failure test) – Modify in-memory state, skip sync → simulate crash (advanced)


fs_create() Edge Cases

Maximum filename length - Exactly 28 characters (MAX_FILENAME)
Filename too long - 29+ characters
Empty filename - Pass ""
Null filename - Pass NULL
Special characters in filename - Names with /, \0, spaces, etc.
Create when all inodes used - All 256 files already exist
Create duplicate filename - File already exists
Create after delete - Create file with same name as recently deleted file
Create when filesystem unmounted - Call without mounting first

fs_delete() Edge Cases

Delete non-existent file - File doesn't exist
Delete with null filename - Pass NULL
Delete empty filename - Pass ""
Delete file with allocated blocks - Ensure all blocks are freed
Delete last file - When only one file remains
Delete file with maximum blocks - File using all 12 direct blocks
Delete partially written file - File with some blocks allocated, some not

fs_write() Edge Cases

Write exactly BLOCK_SIZE bytes - 4096 bytes
Write BLOCK_SIZE + 1 bytes - 4097 bytes
Write 0 bytes - Empty write
Write maximum file size - Exactly 48KB (12 blocks)
Write beyond maximum - Try to write 48KB + 1 byte
Write when disk full - Not enough free blocks
Write with null data pointer - Pass NULL for data
Write to non-existent file - File doesn't exist
Overwrite larger file with smaller - 48KB file overwritten with 1 byte
Overwrite smaller file with larger - 1 byte file overwritten with 48KB
Write when only some blocks available - Need 5 blocks but only 3 free
Concurrent writes simulation - Write, read partially, write again

fs_read() Edge Cases

Read more than file size - File has 100 bytes, try to read 200
Read from empty file - File size is 0
Read 0 bytes - Pass size=0
Read with null buffer - Pass NULL for buffer
Read from non-existent file - File doesn't exist
Read exactly file size - File has 100 bytes, read exactly 100
Read from file with sparse blocks - Some block pointers are -1
Read after partial write failure - Previous write failed mid-operation

fs_list() Edge Cases

List with null array - Pass NULL for filenames array
List with max_files = 0 - Request 0 files
List with max_files = -1 - Negative value
List with max_files > MAX_FILES - Request more than 256
List when no files exist - Empty filesystem
List when filesystem is full - All 256 files exist
List with max_files < actual files - 10 files exist, request only 5

Combined Operation Edge Cases

Create-Write-Delete-Create cycle - Reuse same filename
Fill filesystem completely - Create 256 files, each with data
Fragment and defragment - Create/delete files to fragment free space
Write after failed write - First write fails due to space, free space, retry
Mount-unmount-mount cycle - Ensure data persists
Operations without mount - Try all operations before mounting

Robustness Tests

Power failure simulation - Kill process mid-write, remount and check
Block allocation rollback - Ensure blocks freed if write fails
Inode consistency - Verify inode.used matches actual state
Cross-boundary writes - Write data spanning multiple blocks
Maximum stress - Fill disk, delete half, refill, repeat

 */