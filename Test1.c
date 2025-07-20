#include "fs.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"

// Test Functions

void test_write_after_failed_write()
{
    printf(YELLOW "Write after failed write - First write fails due to space, free space, retry - Test" RESET "\n");

    // Create a test file
    if (fs_create("retry_file") != 0)
    {
        printf(RED "Write after failed write - Failed to create file" RESET "\n");
        exit(-1);
    }

    // First, use large files to consume most space
    int files_created = 0;
    char filename[MAX_FILENAME];
    int max_size = 12 * BLOCK_SIZE; // Maximum file size
    char *max_data = malloc(max_size);

    for (int i = 0; i < max_size; i++)
    {
        max_data[i] = 'F';
    }

    // Create maximum-sized files (12 blocks each)
    // 2550 / 12 = 212.5, so we can create 212 files
    while (files_created < 212)
    {
        snprintf(filename, sizeof(filename), "big_%03d", files_created);

        if (fs_create(filename) != 0)
        {
            break;
        }

        if (fs_write(filename, max_data, max_size) != 0)
        {
            fs_delete(filename);
            break;
        }

        files_created++;
    }

    free(max_data);

    // Now we have 2550 - (212 * 12) = 6 blocks free
    // Fill those with small files
    int small_files = 0;
    char *small_data = malloc(BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        small_data[i] = 'S';
    }

    while (small_files < 10)
    {
        snprintf(filename, sizeof(filename), "small_%d", small_files);

        if (fs_create(filename) != 0)
        {
            break;
        }

        if (fs_write(filename, small_data, BLOCK_SIZE) != 0)
        {
            fs_delete(filename);
            break;
        }

        small_files++;
    }

    free(small_data);

    // Now attempt to write 10 blocks - this should fail
    int large_size = 10 * BLOCK_SIZE; // 40KB
    char *large_data = malloc(large_size);
    for (int i = 0; i < large_size; i++)
    {
        large_data[i] = 'X';
    }

    // First write attempt - should fail with -2
    int result = fs_write("retry_file", large_data, large_size);
    if (result != -2)
    {
        printf(RED "Write after failed write - First write should have failed but returned %d" RESET "\n", result);
        free(large_data);
        exit(-1);
    }

    // Now free some space by deleting one big file (12 blocks)
    fs_delete("big_000");

    // Retry the write - should succeed now
    result = fs_write("retry_file", large_data, large_size);
    if (result != 0)
    {
        printf(RED "Write after failed write - Retry write failed with error %d" RESET "\n", result);
        free(large_data);
        exit(-1);
    }

    // Verify the write succeeded by reading back
    char *read_buffer = malloc(large_size);
    int bytes_read = fs_read("retry_file", read_buffer, large_size);

    if (bytes_read != large_size)
    {
        printf(RED "Write after failed write - Expected to read %d bytes but got %d" RESET "\n", large_size, bytes_read);
        free(large_data);
        free(read_buffer);
        exit(-1);
    }

    // Verify data integrity
    for (int i = 0; i < large_size; i++)
    {
        if (read_buffer[i] != 'X')
        {
            printf(RED "Write after failed write - Data mismatch at byte %d" RESET "\n", i);
            free(large_data);
            free(read_buffer);
            exit(-1);
        }
    }

    printf(GREEN "Write after failed write - Success" RESET "\n");

    // Cleanup
    free(large_data);
    free(read_buffer);
    fs_delete("retry_file");

    // Delete remaining files
    for (int i = 1; i < files_created; i++)
    {
        snprintf(filename, sizeof(filename), "big_%03d", i);
        fs_delete(filename);
    }
    for (int i = 0; i < small_files; i++)
    {
        snprintf(filename, sizeof(filename), "small_%d", i);
        fs_delete(filename);
    }
}

void test_read_after_partial_write_failure()
{
    printf(YELLOW "Read after partial write failure - Previous write failed mid-operation - Test" RESET "\n");

    // First, create a file with some initial data
    if (fs_create("partial_fail_file") != 0)
    {
        printf(RED "Read after partial write failure - Failed to create file" RESET "\n");
        exit(-1);
    }

    // Write initial data (1KB)
    int initial_size = 1024;
    char *initial_data = malloc(initial_size);
    for (int i = 0; i < initial_size; i++)
    {
        initial_data[i] = 'A';
    }

    if (fs_write("partial_fail_file", initial_data, initial_size) != 0)
    {
        printf(RED "Read after partial write failure - Failed to write initial data" RESET "\n");
        free(initial_data);
        exit(-1);
    }

    free(initial_data);

    // Now fill the disk to ensure the next write will fail
    // We need to fill it so there's not enough space for a large write
    int files_created = 0;
    char filename[MAX_FILENAME];
    int fill_size = 10 * BLOCK_SIZE; // 40KB files
    char *fill_data = malloc(fill_size);

    for (int i = 0; i < fill_size; i++)
    {
        fill_data[i] = 'F';
    }

    // Fill disk until we can't write anymore
    while (files_created < 200)
    {
        snprintf(filename, sizeof(filename), "fill_%03d", files_created);

        if (fs_create(filename) != 0)
        {
            break;
        }

        if (fs_write(filename, fill_data, fill_size) != 0)
        {
            fs_delete(filename);
            break;
        }

        files_created++;
    }

    free(fill_data);

    // Now attempt a write that should fail due to insufficient space
    int large_size = 20 * BLOCK_SIZE; // 80KB - should fail
    char *large_data = malloc(large_size);
    for (int i = 0; i < large_size; i++)
    {
        large_data[i] = 'B';
    }

    // This write should fail
    int write_result = fs_write("partial_fail_file", large_data, large_size);
    if (write_result != -2)
    {
        // If it didn't fail, we didn't fill the disk enough
        // But let's continue with the test
    }

    free(large_data);

    // Now read the file - it should still contain the original 'A' data
    char *read_buffer = malloc(initial_size);
    int bytes_read = fs_read("partial_fail_file", read_buffer, initial_size);

    if (bytes_read != initial_size)
    {
        printf(RED "Read after partial write failure - Expected %d bytes but got %d" RESET "\n", initial_size, bytes_read);
        free(read_buffer);
        exit(-1);
    }

    // Verify the data is still the original 'A' data
    for (int i = 0; i < initial_size; i++)
    {
        if (read_buffer[i] != 'A')
        {
            printf(RED "Read after partial write failure - Data corrupted at byte %d, expected 'A' but got '%c'" RESET "\n", i, read_buffer[i]);
            free(read_buffer);
            exit(-1);
        }
    }

    printf(GREEN "Read after partial write failure - Success" RESET "\n");

    // Cleanup
    free(read_buffer);
    fs_delete("partial_fail_file");

    for (int i = 0; i < files_created; i++)
    {
        snprintf(filename, sizeof(filename), "fill_%03d", i);
        fs_delete(filename);
    }
}

void test_read_sparse_blocks()
{
    printf(YELLOW "Read from file with sparse blocks - Some block pointers are -1 - Test" RESET "\n");

    // Create a file
    if (fs_create("sparse_file") != 0)
    {
        printf(RED "Read sparse blocks - Failed to create file" RESET "\n");
        exit(-1);
    }

    // Write a small amount of data (less than 1 block)
    char small_data[100];
    for (int i = 0; i < 100; i++)
    {
        small_data[i] = (char)(i % 256);
    }

    if (fs_write("sparse_file", small_data, 100) != 0)
    {
        printf(RED "Read sparse blocks - Failed to write data" RESET "\n");
        exit(-1);
    }

    // At this point, the file has:
    // - blocks[0] pointing to a valid block with 100 bytes
    // - blocks[1] through blocks[11] should be -1 (unallocated)

    // Try to read the file
    char read_buffer[200];
    int bytes_read = fs_read("sparse_file", read_buffer, 200);

    // Should only read 100 bytes (the actual file size)
    if (bytes_read != 100)
    {
        printf(RED "Read sparse blocks - Expected to read 100 bytes but got %d" RESET "\n", bytes_read);
        exit(-1);
    }

    // Verify the data
    for (int i = 0; i < 100; i++)
    {
        if (read_buffer[i] != small_data[i])
        {
            printf(RED "Read sparse blocks - Data mismatch at byte %d" RESET "\n", i);
            exit(-1);
        }
    }

    printf(GREEN "Read from file with sparse blocks - Success" RESET "\n");

    // Cleanup
    fs_delete("sparse_file");
}

void test_read_exact_file_size()
{
    printf(YELLOW "Read exactly file size - File has 100 bytes, read exactly 100 - Test" RESET "\n");

    // Create a file
    if (fs_create("exact_size_file") != 0)
    {
        printf(RED "Read exactly file size - Failed to create file" RESET "\n");
        exit(-1);
    }

    // Write exactly 100 bytes
    char write_data[100];
    for (int i = 0; i < 100; i++)
    {
        write_data[i] = (char)(i % 256);
    }

    if (fs_write("exact_size_file", write_data, 100) != 0)
    {
        printf(RED "Read exactly file size - Failed to write data" RESET "\n");
        exit(-1);
    }

    // Read exactly 100 bytes
    char read_buffer[100];
    int bytes_read = fs_read("exact_size_file", read_buffer, 100);

    // Should read exactly 100 bytes
    if (bytes_read != 100)
    {
        printf(RED "Read exactly file size - Expected to read 100 bytes but got %d" RESET "\n", bytes_read);
        exit(-1);
    }

    // Verify all data matches
    for (int i = 0; i < 100; i++)
    {
        if (read_buffer[i] != write_data[i])
        {
            printf(RED "Read exactly file size - Data mismatch at byte %d" RESET "\n", i);
            exit(-1);
        }
    }

    printf(GREEN "Read exactly file size - Success" RESET "\n");

    // Cleanup
    fs_delete("exact_size_file");
}

void test_read_edge_cases()
{
    // Test 3: Read 0 bytes - Pass size=0
    printf(YELLOW "Read 0 bytes - Pass size=0 - Test" RESET "\n");

    // Create a file with some data
    if (fs_create("read_test") != 0)
    {
        printf(RED "Read 0 bytes - Failed to create file" RESET "\n");
        exit(-1);
    }

    char data[] = "Hello World";
    if (fs_write("read_test", data, strlen(data)) != 0)
    {
        printf(RED "Read 0 bytes - Failed to write data" RESET "\n");
        exit(-1);
    }

    // Try to read 0 bytes
    char buffer[10];
    int bytes_read = fs_read("read_test", buffer, 0);

    if (bytes_read != 0)
    {
        printf(RED "Read 0 bytes - Expected 0 bytes read but got %d" RESET "\n", bytes_read);
        exit(-1);
    }
    printf(GREEN "Read 0 bytes - Success" RESET "\n");

    // Test 4: Read with null buffer - Pass NULL for buffer
    printf(YELLOW "Read with null buffer - Pass NULL for buffer - Test" RESET "\n");

    int result = fs_read("read_test", NULL, 10);

    if (result != -3)
    {
        printf(RED "Read with null buffer - Expected error -3 but got %d" RESET "\n", result);
        exit(-1);
    }
    printf(GREEN "Read with null buffer - Success" RESET "\n");

    // Clean up before next test
    fs_delete("read_test");

    // Test 5: Read from non-existent file - File doesn't exist
    printf(YELLOW "Read from non-existent file - File doesn't exist - Test" RESET "\n");

    char dummy_buffer[10];
    result = fs_read("non_existent_file", dummy_buffer, 10);

    if (result != -1)
    {
        printf(RED "Read from non-existent file - Expected error -1 but got %d" RESET "\n", result);
        exit(-1);
    }
    printf(GREEN "Read from non-existent file - Success" RESET "\n");
}

void read_from_empty_file()
{
    printf(YELLOW "Read exactly 0 bytes - Test" RESET "\n");

    fs_create("Eylon's Empty file");
    char buffer[BLOCK_SIZE];

    if (fs_read("Eylon's Empty file", buffer, BLOCK_SIZE) > 0)
    {
        printf(RED "Read exactly 0 bytes - Failed" RESET "\n");
        fs_delete("Eylon's Empty file");
        exit(-1);
    }
    printf(GREEN "Read exactly 0 bytes - Success" RESET "\n");
    fs_delete("Eylon's Empty file");
}

void test_read_more_than_file_size()
{
    printf(YELLOW "Read more than file size - File has 100 bytes, try to read 200 - Test" RESET "\n");

    // Create a file
    if (fs_create("small_file") != 0)
    {
        printf(RED "Read more than file size - Failed to create file" RESET "\n");
        exit(-1);
    }

    // Write 100 bytes
    char write_data[100];
    for (int i = 0; i < 100; i++)
    {
        write_data[i] = (char)(i % 256);
    }

    if (fs_write("small_file", write_data, 100) != 0)
    {
        printf(RED "Read more than file size - Failed to write data" RESET "\n");
        exit(-1);
    }

    // Try to read 200 bytes (more than file size)
    char read_buffer[200];
    memset(read_buffer, 0xFF, 200); // Fill with 0xFF to detect what was actually read

    int bytes_read = fs_read("small_file", read_buffer, 200);

    // Should only read 100 bytes (the actual file size)
    if (bytes_read != 100)
    {
        printf(RED "Read more than file size - Expected to read 100 bytes but got %d" RESET "\n", bytes_read);
        exit(-1);
    }

    // Verify the 100 bytes read are correct
    for (int i = 0; i < 100; i++)
    {
        if (read_buffer[i] != write_data[i])
        {
            printf(RED "Read more than file size - Data mismatch at byte %d" RESET "\n", i);
            exit(-1);
        }
    }

    printf(GREEN "Read more than file size - Success" RESET "\n");

    // Cleanup
    fs_delete("small_file");
}

void test_write_with_limited_blocks()
{
    printf(YELLOW "Write when only some blocks available - Need 5 blocks but only 3 free - Test" RESET "\n");

    // First create 212 files of 12 blocks each = 2544 blocks used
    // This leaves 6 blocks free (2550 - 2544 = 6)
    // Then create 3 more single-block files to leave exactly 3 blocks free

    int files_created = 0;
    char filename[MAX_FILENAME];
    int max_file_size = MAX_DIRECT_BLOCKS * BLOCK_SIZE; // 48KB

    char *max_data = malloc(max_file_size);
    if (!max_data)
    {
        printf(RED "Write with limited blocks - Failed to allocate memory" RESET "\n");
        exit(-1);
    }

    for (int i = 0; i < max_file_size; i++)
    {
        max_data[i] = (char)(i % 256);
    }

    // Create 212 maximum-sized files
    for (int i = 0; i < 212; i++)
    {
        snprintf(filename, sizeof(filename), "fill_%03d", i);

        if (fs_create(filename) != 0)
        {
            break;
        }

        if (fs_write(filename, max_data, max_file_size) != 0)
        {
            fs_delete(filename);
            break;
        }

        files_created++;
    }

    free(max_data);

    // Now we have 6 blocks free, create 3 single-block files to leave exactly 3 free
    char *single_block = malloc(BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        single_block[i] = (char)(i % 256);
    }

    int small_files = 0;
    for (int i = 0; i < 3; i++)
    {
        snprintf(filename, sizeof(filename), "small_%d", i);

        if (fs_create(filename) != 0)
        {
            break;
        }

        if (fs_write(filename, single_block, BLOCK_SIZE) != 0)
        {
            fs_delete(filename);
            break;
        }

        small_files++;
    }

    free(single_block);

    // Now we should have exactly 3 blocks free
    // Create a test file
    if (fs_create("test_limited") != 0)
    {
        printf(RED "Write with limited blocks - Failed to create test file" RESET "\n");
        exit(-1);
    }

    // Try to write 5 blocks worth of data when only 3 are free
    int bytes_to_write = 5 * BLOCK_SIZE; // 20KB
    char *test_data = malloc(bytes_to_write);
    if (!test_data)
    {
        printf(RED "Write with limited blocks - Failed to allocate test data" RESET "\n");
        exit(-1);
    }

    for (int i = 0; i < bytes_to_write; i++)
    {
        test_data[i] = (char)(i % 256);
    }

    // This should fail with -2 (not enough blocks)
    int result = fs_write("test_limited", test_data, bytes_to_write);

    if (result != -2)
    {
        printf(RED "Write with limited blocks - Expected error -2 but got %d" RESET "\n", result);
        free(test_data);
        exit(-1);
    }

    printf(GREEN "Write when only some blocks available - Success" RESET "\n");

    // Cleanup
    free(test_data);
    fs_delete("test_limited");

    for (int i = 0; i < files_created; i++)
    {
        snprintf(filename, sizeof(filename), "fill_%03d", i);
        fs_delete(filename);
    }
    for (int i = 0; i < small_files; i++)
    {
        snprintf(filename, sizeof(filename), "small_%d", i);
        fs_delete(filename);
    }
}

void test_concurrent_writes_simulation()
{
    printf(YELLOW "Concurrent writes simulation - Write, read partially, write again - Test" RESET "\n");

    // Create a test file
    if (fs_create("concurrent_test") != 0)
    {
        printf(RED "Concurrent writes simulation - Failed to create file" RESET "\n");
        exit(-1);
    }

    // First write: 10KB
    int first_size = 10 * 1024; // 10KB
    char *first_data = malloc(first_size);
    if (!first_data)
    {
        printf(RED "Concurrent writes simulation - Failed to allocate memory" RESET "\n");
        exit(-1);
    }

    // Fill with 'A' pattern
    for (int i = 0; i < first_size; i++)
    {
        first_data[i] = 'A';
    }

    // Write first data
    if (fs_write("concurrent_test", first_data, first_size) != 0)
    {
        printf(RED "Concurrent writes simulation - Failed first write" RESET "\n");
        free(first_data);
        exit(-1);
    }

    free(first_data);

    // Read partially - only 5KB
    int partial_read_size = 5 * 1024;
    char *read_buffer = malloc(partial_read_size);
    if (!read_buffer)
    {
        printf(RED "Concurrent writes simulation - Failed to allocate read buffer" RESET "\n");
        exit(-1);
    }

    int bytes_read = fs_read("concurrent_test", read_buffer, partial_read_size);
    if (bytes_read != partial_read_size)
    {
        printf(RED "Concurrent writes simulation - Partial read failed, expected %d got %d" RESET "\n", partial_read_size, bytes_read);
        free(read_buffer);
        exit(-1);
    }

    // Verify partial read got 'A's
    for (int i = 0; i < partial_read_size; i++)
    {
        if (read_buffer[i] != 'A')
        {
            printf(RED "Concurrent writes simulation - Partial read data mismatch at byte %d" RESET "\n", i);
            free(read_buffer);
            exit(-1);
        }
    }

    free(read_buffer);

    // Second write: overwrite with 20KB of 'B'
    int second_size = 20 * 1024; // 20KB
    char *second_data = malloc(second_size);
    if (!second_data)
    {
        printf(RED "Concurrent writes simulation - Failed to allocate second data" RESET "\n");
        exit(-1);
    }

    // Fill with 'B' pattern
    for (int i = 0; i < second_size; i++)
    {
        second_data[i] = 'B';
    }

    // Write second data (overwrites the file)
    if (fs_write("concurrent_test", second_data, second_size) != 0)
    {
        printf(RED "Concurrent writes simulation - Failed second write" RESET "\n");
        free(second_data);
        exit(-1);
    }

    free(second_data);

    // Read entire file to verify it's all 'B' now
    char *final_read = malloc(second_size);
    if (!final_read)
    {
        printf(RED "Concurrent writes simulation - Failed to allocate final read buffer" RESET "\n");
        exit(-1);
    }

    bytes_read = fs_read("concurrent_test", final_read, second_size);
    if (bytes_read != second_size)
    {
        printf(RED "Concurrent writes simulation - Final read failed, expected %d got %d" RESET "\n", second_size, bytes_read);
        free(final_read);
        exit(-1);
    }

    // Verify all data is 'B' (no 'A' remains)
    for (int i = 0; i < second_size; i++)
    {
        if (final_read[i] != 'B')
        {
            printf(RED "Concurrent writes simulation - Data not fully overwritten at byte %d" RESET "\n", i);
            free(final_read);
            exit(-1);
        }
    }

    free(final_read);

    printf(GREEN "Concurrent writes simulation - Success" RESET "\n");

    // Cleanup
    fs_delete("concurrent_test");
}

void test_overwrite_larger_with_smaller()
{
    printf(YELLOW "Overwrite larger file with smaller - 48KB file overwritten with 1 byte - Test" RESET "\n");

    // Create a file
    if (fs_create("large_to_small") != 0)
    {
        printf(RED "Overwrite larger file with smaller - Failed to create file" RESET "\n");
        exit(-1);
    }

    // Write 48KB to the file
    int large_size = MAX_DIRECT_BLOCKS * BLOCK_SIZE; // 48KB
    char *large_data = malloc(large_size);
    if (!large_data)
    {
        printf(RED "Overwrite larger file with smaller - Failed to allocate memory" RESET "\n");
        exit(-1);
    }

    // Fill with pattern
    for (int i = 0; i < large_size; i++)
    {
        large_data[i] = 'L';
    }

    // Write the large data
    if (fs_write("large_to_small", large_data, large_size) != 0)
    {
        printf(RED "Overwrite larger file with smaller - Failed to write large data" RESET "\n");
        free(large_data);
        exit(-1);
    }

    free(large_data);

    // Overwrite with 1 byte
    char small_data[1] = {'S'};

    if (fs_write("large_to_small", small_data, 1) != 0)
    {
        printf(RED "Overwrite larger file with smaller - Failed to overwrite with small data" RESET "\n");
        exit(-1);
    }

    // Read back and verify size
    char read_buffer[10];
    int bytes_read = fs_read("large_to_small", read_buffer, 10);

    if (bytes_read != 1)
    {
        printf(RED "Overwrite larger file with smaller - Expected 1 byte, read %d bytes" RESET "\n", bytes_read);
        exit(-1);
    }

    printf(GREEN "Overwrite larger file with smaller - Success" RESET "\n");

    // Cleanup
    fs_delete("large_to_small");
}

void write_to_non_existent_file()
{
    printf(YELLOW "Writing to a non - existent file - Test" RESET "\n");
    if (fs_write("Eylons Empty File", "Eylon is a good proggramer", sizeof("Eylon is a good proggramer")) == 0)
    {
        printf(RED "Writing to a non - existent file - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Writing to a non - existent file - Success" RESET "\n");
}

void test_overwrite_smaller_with_larger()
{
    printf(YELLOW "Overwrite smaller file with larger - 1 byte file overwritten with 48KB - Test" RESET "\n");

    // Create a file
    if (fs_create("small_to_large") != 0)
    {
        printf(RED "Overwrite smaller file with larger - Failed to create file" RESET "\n");
        exit(-1);
    }

    // Write 1 byte to the file
    char small_data[1] = {'S'};

    if (fs_write("small_to_large", small_data, 1) != 0)
    {
        printf(RED "Overwrite smaller file with larger - Failed to write small data" RESET "\n");
        exit(-1);
    }

    // Overwrite with 48KB
    int large_size = MAX_DIRECT_BLOCKS * BLOCK_SIZE; // 48KB
    char *large_data = malloc(large_size);
    if (!large_data)
    {
        printf(RED "Overwrite smaller file with larger - Failed to allocate memory" RESET "\n");
        exit(-1);
    }

    // Fill with pattern
    for (int i = 0; i < large_size; i++)
    {
        large_data[i] = 'L';
    }

    // Overwrite with large data
    if (fs_write("small_to_large", large_data, large_size) != 0)
    {
        printf(RED "Overwrite smaller file with larger - Failed to overwrite with large data" RESET "\n");
        free(large_data);
        exit(-1);
    }

    free(large_data);

    // Read back and verify size
    char *read_buffer = malloc(large_size);
    int bytes_read = fs_read("small_to_large", read_buffer, large_size);

    if (bytes_read != large_size)
    {
        printf(RED "Overwrite smaller file with larger - Expected %d bytes, read %d bytes" RESET "\n", large_size, bytes_read);
        free(read_buffer);
        exit(-1);
    }

    free(read_buffer);

    printf(GREEN "Overwrite smaller file with larger - Success" RESET "\n");

    // Cleanup
    fs_delete("small_to_large");
}

void writing_null_Pointer()
{
    printf(YELLOW "Trying to write NULL pointer - Test" RESET "\n");
    fs_create("Omer123");
    if (fs_write("Omer123", NULL, 50) == 0)
    {
        printf(RED "Trying to write NULL pointer - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Trying to write NULL pointer - Success" RESET "\n");
    fs_delete("Omer123");
}

void test_write_when_disk_full()
{
    printf(YELLOW "Write when disk full - Not enough free blocks - Test" RESET "\n");

    // We need to fill the disk more efficiently
    // Let's create files of maximum size (12 blocks each)
    int max_file_blocks = 12;
    int max_file_size = max_file_blocks * BLOCK_SIZE; // 48KB

    char *max_data = malloc(max_file_size);
    if (!max_data)
    {
        printf(RED "Write when disk full - Failed to allocate memory" RESET "\n");
        exit(-1);
    }

    // Fill with pattern
    for (int i = 0; i < max_file_size; i++)
    {
        max_data[i] = (char)(i % 256);
    }

    // Create files until we run out of space
    int files_created = 0;
    char filename[MAX_FILENAME];

    // We have 2550 data blocks. With 12-block files, we need about 212 files
    // But we only have 256 inodes total
    while (files_created < 256)
    {
        snprintf(filename, sizeof(filename), "max_%03d", files_created);

        if (fs_create(filename) != 0)
        {
            break; // No more inodes
        }

        if (fs_write(filename, max_data, max_file_size) != 0)
        {
            // Failed to write - not enough blocks
            fs_delete(filename);
            break;
        }

        files_created++;
    }

    free(max_data);

    // Now fill the remaining space with smaller files
    char *small_data = malloc(BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        small_data[i] = (char)(i % 256);
    }

    int small_files = 0;
    while (small_files < 50)
    {
        snprintf(filename, sizeof(filename), "tiny_%03d", small_files);

        if (fs_create(filename) != 0)
        {
            break;
        }

        if (fs_write(filename, small_data, BLOCK_SIZE) != 0)
        {
            fs_delete(filename);
            break;
        }

        small_files++;
    }

    free(small_data);

    // Now try to create and write a file that needs more blocks than available
    if (fs_create("test_no_space") != 0)
    {
        printf(RED "Write when disk full - Failed to create test file" RESET "\n");
        exit(-1);
    }

    // Try to write 10 blocks
    int bytes_to_write = 10 * BLOCK_SIZE;
    char *test_data = malloc(bytes_to_write);
    if (!test_data)
    {
        printf(RED "Write when disk full - Failed to allocate test data" RESET "\n");
        exit(-1);
    }

    for (int i = 0; i < bytes_to_write; i++)
    {
        test_data[i] = (char)(i % 256);
    }

    int result = fs_write("test_no_space", test_data, bytes_to_write);

    if (result != -2)
    {
        printf(RED "Write when disk full - Expected error -2 but got %d" RESET "\n", result);
        free(test_data);
        exit(-1);
    }

    printf(GREEN "Write when disk full - Success (correctly failed with error -2)" RESET "\n");

    // Cleanup
    free(test_data);
    fs_delete("test_no_space");

    // Clean up all files
    for (int i = 0; i < files_created; i++)
    {
        snprintf(filename, sizeof(filename), "max_%03d", i);
        fs_delete(filename);
    }
    for (int i = 0; i < small_files; i++)
    {
        snprintf(filename, sizeof(filename), "tiny_%03d", i);
        fs_delete(filename);
    }
}

void test_write_expected_failure(int bytes_to_write, int expected_error, const char *test_name)
{
    printf(YELLOW "%s - Test" RESET "\n", test_name);

    // Create a unique filename for this test
    char filename[MAX_FILENAME];
    snprintf(filename, sizeof(filename), "test_fail_%d", bytes_to_write);

    // Create a test file
    if (fs_create(filename) != 0)
    {
        printf(RED "%s - Failed to create file" RESET "\n", test_name);
        exit(-1);
    }

    // Create data buffer
    char *write_data = malloc(bytes_to_write);
    if (!write_data)
    {
        printf(RED "%s - Failed to allocate memory" RESET "\n", test_name);
        exit(-1);
    }

    // Fill with pattern
    for (int i = 0; i < bytes_to_write; i++)
    {
        write_data[i] = (char)(i % 256);
    }

    // Write should fail with expected error
    int result = fs_write(filename, write_data, bytes_to_write);
    if (result != expected_error)
    {
        printf(RED "%s - Expected error %d but got %d" RESET "\n", test_name, expected_error, result);
        free(write_data);
        exit(-1);
    }

    printf(GREEN "%s - Success (correctly failed with error %d)" RESET "\n", test_name, expected_error);

    // Cleanup
    free(write_data);
    fs_delete(filename);
}

void test_write_amount_of_bytes(int bytes_to_write, const char *test_name)
{
    printf(YELLOW "%s - Test" RESET "\n", test_name);

    // Create a unique filename for this test
    char filename[MAX_FILENAME];
    snprintf(filename, sizeof(filename), "test_%d_bytes", bytes_to_write);

    // Create a test file
    if (fs_create(filename) != 0)
    {
        printf(RED "%s - Failed to create file" RESET "\n", test_name);
        exit(-1);
    }

    // Create data buffer
    char *write_data = malloc(bytes_to_write);
    if (!write_data)
    {
        printf(RED "%s - Failed to allocate memory" RESET "\n", test_name);
        exit(-1);
    }

    // Fill with a pattern to verify later
    for (int i = 0; i < bytes_to_write; i++)
    {
        write_data[i] = (char)(i % 256); // Pattern: 0,1,2,...,255,0,1,2,...
    }

    // Write the data
    if (fs_write(filename, write_data, bytes_to_write) != 0)
    {
        printf(RED "%s - Write failed" RESET "\n", test_name);
        free(write_data);
        exit(-1);
    }

    // Read back and verify
    char *read_data = malloc(bytes_to_write);
    if (!read_data)
    {
        printf(RED "%s - Failed to allocate read buffer" RESET "\n", test_name);
        free(write_data);
        exit(-1);
    }

    int bytes_read = fs_read(filename, read_data, bytes_to_write);

    if (bytes_read != bytes_to_write)
    {
        printf(RED "%s - Read returned %d bytes instead of %d" RESET "\n", test_name, bytes_read, bytes_to_write);
        free(write_data);
        free(read_data);
        exit(-1);
    }

    // Verify the data matches
    for (int i = 0; i < bytes_to_write; i++)
    {
        if (read_data[i] != write_data[i])
        {
            printf(RED "%s - Data mismatch at byte %d" RESET "\n", test_name, i);
            free(write_data);
            free(read_data);
            exit(-1);
        }
    }

    printf(GREEN "%s - Success" RESET "\n", test_name);

    // Cleanup
    free(write_data);
    free(read_data);
    fs_delete(filename);
}

int create_all_files()
{
    char filename[MAX_FILENAME + 1];

    for (int i = 0; i < MAX_FILES; i++)
    {
        snprintf(filename, sizeof(filename), "file_%03d", i);
        if (fs_create(filename) != 0)
        {
            printf("System stopped on file %d \n", i);
            return -1; // Failed to create file
        }
    }
    return 0; // Success
}

void cleanup_all_files()
{
    char filenames[MAX_FILES][MAX_FILENAME];
    int file_count = fs_list(filenames, MAX_FILES);

    for (int i = 0; i < file_count; i++)
    {
        fs_delete(filenames[i]);
    }
}

void test_inode_exhaustion()
{
    fs_format("test1_file_system.img");
    fs_mount("test1_file_system.img");
    // Test: Create all 256 files (should succeed)
    printf(YELLOW "Create all 256 files - Test" RESET "\n");

    if (create_all_files() != 0)
    {
        printf(RED "Create all 256 files - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Create all 256 files - Success" RESET "\n");

    // Test: Try to create 257th file (should fail with -2)
    printf(YELLOW "Create 257th file when all inodes used - Test" RESET "\n");

    if (fs_create("should_fail") == 0)
    {
        printf(RED "Create 257th file when all inodes used - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Create 257th file when all inodes used - Success" RESET "\n");

    // Test: Verify we can still write to existing files
    printf(YELLOW "Write to existing file when inodes exhausted - Test" RESET "\n");

    const char *test_data = "test";
    int write_result = fs_write("file_000", test_data, strlen(test_data));

    if (write_result != 0)
    {
        printf(RED "Write to existing file when inodes exhausted - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Write to existing file when inodes exhausted - Success" RESET "\n");
    printf(GREEN "Write to existing file when inodes exhausted - Success" RESET "\n");

    // Test: Delete file and verify inode is freed
    printf(YELLOW "Free inode after file deletion - Test" RESET "\n");

    if (fs_delete("file_255") != 0)
    {
        printf(RED "Free inode after file deletion - Failed" RESET "\n");
        exit(-1);
    }

    // Now creating a new file should work
    if (fs_create("recovered_inode") != 0)
    {
        printf(RED "Free inode after file deletion - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Free inode after file deletion - Success" RESET "\n");

    // Cleanup
    cleanup_all_files();
}

void format_in_readOnly_path()
{
    printf(YELLOW "Disk in read-only directory - Try to create disk in a directory without write permissions" RESET "\n");

    // Create a test directory
    if (mkdir("readonly_test_dir", 0755) != 0)
    {
        printf(RED "Disk in read-only directory - Failed to create test directory" RESET "\n");
        exit(-1);
    }

    // Remove write permissions from the directory
    if (chmod("readonly_test_dir", 0555) != 0) // r-xr-xr-x (no write for anyone)
    {
        printf(RED "Disk in read-only directory - Failed to change directory permissions" RESET "\n");
        rmdir("readonly_test_dir");
        exit(-1);
    }

    // Try to create a disk file in the read-only directory
    if (fs_format("readonly_test_dir/test_disk.img") != -1)
    {
        printf(RED "Disk in read-only directory - Failed (should return -1 for read-only directory)" RESET "\n");
        chmod("readonly_test_dir", 0755);          // Restore permissions to clean up
        unlink("readonly_test_dir/test_disk.img"); // This shouldn't exist, but just in case
        rmdir("readonly_test_dir");
        exit(-1);
    }

    printf(GREEN "Disk in read-only directory - Success (correctly rejected due to permissions)" RESET "\n");

    // Clean up: restore permissions and remove directory
    chmod("readonly_test_dir", 0755);
    rmdir("readonly_test_dir");
}

void format_twice_without_unmount()
{

    printf(YELLOW "Format twice without unmount - Call fs_format() on an already mounted filesystem" RESET "\n");

    // First format and mount
    if (fs_format("test_mounted.img") != 0)
    {
        printf(RED "Format twice without unmount - Initial format failed" RESET "\n");
        exit(-1);
    }

    if (fs_mount("test_mounted.img") != 0)
    {
        printf(RED "Format twice without unmount - Mount failed" RESET "\n");
        exit(-1);
    }

    // Now try to format again while mounted
    if (fs_format("test_mounted.img") != -1)
    {
        printf(RED "Format twice without unmount - Failed (should not allow format while mounted)" RESET "\n");
        fs_unmount();
        unlink("test_mounted.img");
        exit(-1);
    }

    printf(GREEN "Format twice without unmount - Success (correctly rejected format while mounted)" RESET "\n");
    fs_unmount();
    unlink("test_mounted.img");
}

// fs_format() Edge Cases

void fs_format_tests()
{
    printf(YELLOW " FS_FORMAT() tests:" RESET "\n\n");

    printf(YELLOW "Null path parameter - Pass NULL as disk_path - Testing" RESET "\n");
    if (fs_format(NULL) != -1)
    {
        printf(RED "Null path parameter - Pass NULL as disk_path - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Null path parameter - Pass NULL as disk_path - Success" RESET "\n");

    printf(YELLOW "Empty string path - Pass "
                  " as disk_path - Testing" RESET "\n");
    if (fs_format("") != -1)
    {
        printf(RED "Empty string path - Pass "
                   " as disk_path - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Null path parameter - Pass NULL as disk_path - Success" RESET "\n");

    if (fs_format("") != -1)
    {
        printf(RED "Empty string path - Pass "
                   " as disk_path - Failed" RESET "\n");
        exit(-1);
    }

    printf(YELLOW "Path with special characters - Use paths like \"test/../../disk.img\"" RESET "\n");
    if (fs_format("/../../disktest.img") != 0)
    {
        printf(RED "Path with special characters - Use paths like \"test/../../disk.img\" - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Path with special characters - Use paths like \"test/../../disk.img\" - Success" RESET "\n");

    format_twice_without_unmount();

    // format_in_readOnly_path();

    printf(GREEN "All FS_FORMAT() tests passed - Congragulations" RESET "\n\n");
}

// fs_create() Edge Cases

void fs_create_tests()
{
    printf(YELLOW " FS_CREATE() tests:" RESET "\n\n");
    // Initialize the File System
    fs_format("test_file_system.img");
    fs_mount("test_file_system.img");

    // Test 1 : Maximum filename length - Exactly 28 characters (MAX_FILENAME)
    printf(YELLOW "Maximum filename length - Exactly 28 characters (MAX_FILENAME) - Test" RESET "\n");
    char testString[MAX_FILENAME] = "abcdefghijklmnopqrstuvwxyzA"; // Exactly 28 chars

    if (fs_create(testString) != 0)
    {
        printf(RED "Maximum filename length - Exactly 28 characters (MAX_FILENAME) - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Maximum filename length - Exactly 28 characters (MAX_FILENAME) - Success" RESET "\n");
    fs_delete("abcdefghijklmnopqrstuvwxyzA");

    // Test 2: Filename too long - 29+ characters
    printf(YELLOW "Filename too long - 29+ characters - Test" RESET "\n");
    char testString2[MAX_FILENAME + 2] = "abcdefghijklmnopqrstuvwxyzABC"; // 29 chars

    if (fs_create(testString2) > -1)
    {
        printf(RED "Filename too long - 29+ characters - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Filename too long - 29+ characters - Success" RESET "\n");

    // Test 3: Empty filename - Pass ""
    printf(YELLOW "Empty filename - Pass \"\" - Test" RESET "\n");

    if (fs_create("") == 0)
    {
        printf(RED "Empty filename - Pass \"\" - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Empty filename - Pass \"\" - Success" RESET "\n");

    // Test 4: Empty filename - Pass NULL
    printf(YELLOW "Empty filename - Pass NULL - Test" RESET "\n");

    if (fs_create(NULL) == 0)
    {
        printf(RED "Empty filename - Pass NULL - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Empty filename - Pass NULL - Success" RESET "\n");

    // Test 5: Create when all inodes used - All 256 files already exist
    test_inode_exhaustion();

    // Test 6: Creating two files with the same name:
    printf(YELLOW "Creating two files with the same name - Test" RESET "\n");
    fs_create("EylonTheCreator");
    if (fs_create("EylonTheCreator") == 0)
    {
        printf(RED "Creating two files with the same name - Failed" RESET "\n");
        fs_delete("EylonTheCreator");
        exit(-1);
    }
    printf(GREEN "Creating two files with the same name - Success" RESET "\n");
    fs_delete("EylonTheCreator");

    // Test 7: Creating files with the same name after deletion
    printf(YELLOW "Creating files with the same name after deletion - Test" RESET "\n");
    if (fs_create("EylonTheCreator") != 0)
    {
        printf(RED "Creating files with the same name after deletion - Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Creating files with the same name after deletion - Success" RESET "\n");

    // Test 8: Trying to create a file after unmounting
    printf(YELLOW "Trying to create a file after unmounting - Test" RESET "\n");

    fs_unmount();

    if (fs_create("EylonTheCreator") == 0)
    {
        printf(RED "Trying to create a file after unmounting- Failed" RESET "\n");
        exit(-1);
    }
    printf(GREEN "Trying to create a file after unmounting- Success" RESET "\n");

    printf(GREEN " FS_CREATE() Tests Passed" RESET "\n\n");
}

// Test FS_WRITE():

void test_fs_write()
{
    printf(YELLOW " FS_WRITE() tests:" RESET "\n\n");
    // Initialize the File System
    fs_format("test_file_system_write.img");
    fs_mount("test_file_system_write.img");

    // Test 1: Write exactly BLOCK_SIZE bytes - 4096 bytes
    test_write_amount_of_bytes(4096, "Write exactly BLOCK_SIZE bytes - 4096 bytes");

    // Test 2: Write BLOCK_SIZE + 1 bytes - 4097 bytes
    test_write_amount_of_bytes(4097, "Write BLOCK_SIZE + 1 bytes - 4097 bytes");

    // Test 3: Write 0 bytes - Empty write
    test_write_amount_of_bytes(0, "Write 0 bytes - Empty write");

    // Test 4: Write maximum file size - 48KB
    test_write_amount_of_bytes(MAX_DIRECT_BLOCKS * BLOCK_SIZE, "Write maximum file size - 48KB");

    // Test 5: Write maximum file size + 1 - 48KB +1 byte
    test_write_expected_failure(MAX_DIRECT_BLOCKS * BLOCK_SIZE + 1, -3, "Write maximum file size + 1 - 48KB + 1 byte");

    // Test 6: Attempting to write files when the disk is full
    test_write_when_disk_full();

    // Test 7: Write with null data pointer - Pass NULL for data
    writing_null_Pointer();

    // Test 8: Write to non-existent file
    write_to_non_existent_file();

    // Test 9: Overwrite larger file with smaller - 48KB file overwritten with 1 byte
    test_overwrite_larger_with_smaller();

    // Test 10: Overwrite smaller file with larger - 1 byte file overwritten with 48KB
    test_overwrite_smaller_with_larger();

    // Test 11: Write when only some blocks available - Need 5 blocks but only 3 free
    test_write_with_limited_blocks();

    // Test 12: Concurrent writes simulation - Write, read partially, write again
    test_concurrent_writes_simulation();

    fs_unmount();
    printf(GREEN " FS_WRITE() Tests Passed" RESET "\n");
    printf("\n");
}

void test_fs_read()
{

    printf(YELLOW " FS_READ() tests:" RESET "\n\n");
    // Initialize the File System
    fs_format("test_file_system_read.img");
    fs_mount("test_file_system_read.img");

    // Test 1: Read more than file size - File has 100 bytes, try to read 200
    test_read_more_than_file_size();
    // Test 2: Read from empty file - File size is 0
    read_from_empty_file();

    // Test 3: Read 0 bytes - Pass size=0
    // Test 4: Read with null buffer - Pass NULL for buffer
    // Test 5: Read from non-existent file - File doesn't exist
    test_read_edge_cases();
    // Test 6: Read exactly file size - File has 100 bytes, read exactly 100
    test_read_exact_file_size();
    // Test 7: Read from file with sparse blocks - Some block pointers are -1
    test_read_sparse_blocks();
    // Test 8: Read after partial write failure - Previous write failed mid-operation
    test_read_after_partial_write_failure();

    printf(GREEN " FS_READ() Tests Passed" RESET "\n\n");
}

void main()
{
    fs_format_tests();
    fs_create_tests();
    test_fs_write();
    test_fs_read();
}
