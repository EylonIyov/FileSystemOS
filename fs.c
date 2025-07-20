#include "fs.h"

// Global viriables
inode inode_table[MAX_FILES];
superblock sb;
char bitmap[BLOCK_SIZE] = {0}; // Initialize block bitmap to all zeros
int disk_fd = -1;              // File descriptor for the disk image, initialized to -1 (invalid)
// End of global variables

// Helper functions

int validate_string_manual(const char *str)
{
    if (str == NULL)
        return -1;

    for (int i = 0; i < 28; i++)
    {
        if (str[i] == '\0')
        {
            return 0; // Found null terminator within 28 chars
        }
    }

    // Check if 28th position is null terminator
    if (str[28] == '\0')
    {
        return 0;
    }
    return -1;
}

int find_inode(const char *filename)
{
    if (filename == NULL)
    {
        return -1;
    }

    int filename_len = strlen(filename);
    if (filename_len > MAX_FILENAME)
    {
        return -1; // Filename too long
    }

    for (int i = 0; i < MAX_FILES; i++)
    {
        if (inode_table[i].used == 1)
        {
            // Compare manually
            int match = 1;
            for (int j = 0; j < filename_len; j++)
            {
                if (inode_table[i].name[j] != filename[j])
                {
                    match = 0;
                    break;
                }
            }
            // Check remaining bytes are zero
            if (match)
            {
                for (int j = filename_len; j < MAX_FILENAME; j++)
                {
                    if (inode_table[i].name[j] != 0)
                    {
                        match = 0;
                        break;
                    }
                }
            }
            if (match)
                return i;
        }
    }
    return -1;
}

int find_free_inode()
{
    if (sb.free_inodes == 0)
    {
        return -2; // No free inodes available
    }
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (inode_table[i].used == 0)
        {
            return i;
        }
    }
    return -2; // No free inodes available
}

int find_free_block()
{
    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        if (!(bitmap[i / 8] & (1 << (i % 8))))
        {
            return i;
        }
    }
    return -1; // No free blocks available
}

void mark_block_used(int block_index)
{
    if (block_index >= 0 && block_index < MAX_BLOCKS)
    {
        // Check if block is actually free before marking it used
        if (!(bitmap[block_index / 8] & (1 << (block_index % 8))))
        {
            bitmap[block_index / 8] |= (1 << (block_index % 8));
            sb.free_blocks--;
        }
    }
}

void mark_block_free(int block_index)
{
    if (block_index >= 0 && block_index < MAX_BLOCKS)
    {
        // Check if block is actually used before marking it free
        if (bitmap[block_index / 8] & (1 << (block_index % 8)))
        {
            bitmap[block_index / 8] &= ~(1 << (block_index % 8));
            sb.free_blocks++;
        }
    }
}

void read_inode(int inode_num, inode *target)
{
    if (inode_num < 0 || inode_num >= MAX_FILES || target == NULL)
    {
        return; // Invalid parameters
    }

    if (inode_table[inode_num].used == 0)
    {
        return; // Inode is not used
    }

    *target = inode_table[inode_num];
}

void write_inode(int inode_num, const inode *source)
{
    if (inode_num < 0 || inode_num >= MAX_FILES || source == NULL)
    {
        return;
    }

    int was_used = inode_table[inode_num].used;
    inode_table[inode_num] = *source;

    // Handle allocation
    if (was_used == 0 && source->used == 1)
    {
        sb.free_inodes--; // Allocating a free inode
    }
    // Handle deallocation
    else if (was_used == 1 && source->used == 0)
    {
        sb.free_inodes++; // Freeing a used inode
    }
}

int calculate_blocks_needed(int size)
{
    if (size <= 0)
    {
        return 0; // No blocks needed for zero or negative size
    }
    return (size + BLOCK_SIZE - 1) / BLOCK_SIZE; // Calculate number of blocks needed
}

void sync_metadata_to_disk()
{

    if (disk_fd < 0)
    {
        return;
    }

    // Write superblock
    lseek(disk_fd, 0, SEEK_SET);
    int sb_written = write(disk_fd, &sb, sizeof(superblock));

    // Write bitmap
    lseek(disk_fd, BLOCK_SIZE, SEEK_SET);
    int bitmap_written = write(disk_fd, bitmap, sizeof(bitmap));

    // Write inode table
    lseek(disk_fd, 2 * BLOCK_SIZE, SEEK_SET);
    int inode_written = write(disk_fd, inode_table, sizeof(inode_table));
}

// End of helper functions

int fs_format(const char *disk_path)
{

    if (disk_path == NULL || strlen(disk_path) == 0 || disk_fd != -1)
    {
        return -1; // Error: null path
    }

    // Open the disk image file for writing
    disk_fd = open(disk_path, O_RDWR | O_CREAT, 0644); // rw-r--r-- permissions
    if (disk_fd < 0)
    {
        return -1;
    }
    // Initialize the superblock structure

    sb.total_blocks = MAX_BLOCKS;
    sb.block_size = BLOCK_SIZE;
    sb.free_blocks = MAX_BLOCKS;
    sb.total_inodes = MAX_FILES;
    sb.free_inodes = MAX_FILES;

    // Initialize the inode table

    for (int i = 0; i < MAX_FILES; i++)
    {
        inode_table[i].used = 0;
        inode_table[i].size = 0;
        memset(inode_table[i].name, 0, MAX_FILENAME); // Clear the name
        for (int j = 0; j < MAX_DIRECT_BLOCKS; j++)
        {
            inode_table[i].blocks[j] = -1; // Initialize all blocks to -1
        }
    }

    memset(bitmap, 0, sizeof(bitmap)); // Set all blocks to free (0)
    bitmap[0] |= (1 << (0 % 8));       // Superblock
    bitmap[1 / 8] |= (1 << (1 % 8));   // Block bitmap

    for (int i = 2; i < 10; i++)
    {
        bitmap[i / 8] |= (1 << (i % 8));
    }

    sb.free_blocks -= 10; // Decrease free blocks count by 10 since superblock, block bitmap and the inode table are used

    // Write the superblock to the disk
    lseek(disk_fd, 0, SEEK_SET);
    if (write(disk_fd, &sb, sizeof(superblock)) != sizeof(superblock))
    {
        close(disk_fd);
        return -1; // Error: cannot write superblock
    }

    lseek(disk_fd, BLOCK_SIZE, SEEK_SET); // BLOCK_SIZE = 4096

    if (write(disk_fd, bitmap, BLOCK_SIZE) != BLOCK_SIZE)
    {
        close(disk_fd);
        return -1; // Error: cannot write block bitmap
    }

    // Write the inode table to the disk
    lseek(disk_fd, BLOCK_SIZE * 2, SEEK_SET); // Start writing at block 2 (after superblock and block bitmap)
    if (write(disk_fd, inode_table, sizeof(inode_table)) != sizeof(inode_table))
    {
        close(disk_fd);
        return -1; // Error: cannot write inode table
    }
    close(disk_fd);
    disk_fd = -1;
    return 0;
}

int fs_mount(const char *disk_path)
{
    if (disk_path == NULL)
    {
        return -1; // Error: null path
    }

    if (disk_fd >= 0)
    {
        return -1; // Error: already mounted
    }

    disk_fd = open(disk_path, O_RDWR, 0644);

    if (disk_fd < 0)
    {
        return -1;
    }

    lseek(disk_fd, 0, SEEK_SET); // Ensure we start reading from the beginning
    if (read(disk_fd, &sb, sizeof(superblock)) != sizeof(superblock))
    {
        close(disk_fd);
        return -1; // Error: cannot read superblock
    }

    if (sb.total_blocks != MAX_BLOCKS || sb.block_size != BLOCK_SIZE ||
        sb.total_inodes != MAX_FILES || sb.free_inodes < 0 || sb.free_blocks < 0 || sb.free_blocks > MAX_BLOCKS ||
        sb.free_inodes > MAX_FILES)
    {
        close(disk_fd);
        return -1; // Error: invalid filesystem structure
    }

    lseek(disk_fd, 1 * BLOCK_SIZE, SEEK_SET); // Ensure we start reading from the beginning
    if (read(disk_fd, &bitmap, sizeof(bitmap)) != sizeof(bitmap))
    {
        close(disk_fd);
        return -1; // Error: cannot read block bitmap
    }

    lseek(disk_fd, 2 * BLOCK_SIZE, SEEK_SET); // Ensure we start reading from the beginning
    if (read(disk_fd, &inode_table, sizeof(inode_table)) != sizeof(inode_table))
    {
        close(disk_fd);
        return -1; // Error: cannot read inode table
    }
    return 0; // Success: filesystem mounted
}

void fs_unmount()
{
    if (disk_fd >= 0)
    {
        // Write the superblock, block bitmap, and inode table back to disk
        lseek(disk_fd, 0, SEEK_SET);
        if (write(disk_fd, &sb, sizeof(superblock)) != sizeof(superblock))
        {
            perror("Error writing superblock"); //// change to error message
        }

        lseek(disk_fd, BLOCK_SIZE, SEEK_SET);
        if (write(disk_fd, bitmap, sizeof(bitmap)) != sizeof(bitmap))
        {
            perror("Error writing block bitmap"); /// change to error message
        }

        lseek(disk_fd, 2 * BLOCK_SIZE, SEEK_SET);
        if (write(disk_fd, inode_table, sizeof(inode_table)) != sizeof(inode_table))
        {
            perror("Error writing inode table"); /// change to error message
        }

        close(disk_fd);
        disk_fd = -1; // Reset file descriptor
    }
}

int fs_create(const char *filename)
{

    if (filename == NULL || validate_string_manual(filename) != 0 || strlen(filename) > MAX_FILENAME || strlen(filename) == 0 || disk_fd == -1)
    {
        return -3; // Error: invalid filename
    }

    // Check if the file already exists
    if (find_inode(filename) != -1)
    {
        return -1; // Error: file already exists
    }

    int inode_index = find_free_inode();
    if (inode_index == -2)
    {
        return -2; // No free inodes available
    }

    inode new_inode;
    new_inode.used = 1;                                 // Mark inode as used
    new_inode.size = 0;                                 // Initialize size to 0
    memset(new_inode.name, 0, MAX_FILENAME);            // Clear all 28 bytes
    memcpy(new_inode.name, filename, strlen(filename)); // Copy filename

    for (int i = 0; i < MAX_DIRECT_BLOCKS; i++)
    {
        new_inode.blocks[i] = -1; // Initialize all blocks to -1 (unallocated)
    }

    write_inode(inode_index, &new_inode); // Write the new inode to the inode table

    sync_metadata_to_disk(); // Sync metadata to disk
    return 0;                // Success: file created
}

int fs_delete(const char *filename)
{
    int inode_index = find_inode(filename);

    if (filename == NULL || validate_string_manual(filename) != 0 || strlen(filename) > MAX_FILENAME || inode_index == -1 || disk_fd == -1)
    {
        return -1;
    }

    // Create a temporary copy of the inode before modifying it
    inode temp_inode = inode_table[inode_index];

    // Free all allocated blocks
    for (int i = 0; i < MAX_DIRECT_BLOCKS; i++)
    {
        if (temp_inode.blocks[i] != -1)
        {
            mark_block_free(temp_inode.blocks[i]);
            temp_inode.blocks[i] = -1;
        }
    }

    // Mark the inode as free
    temp_inode.used = 0;
    temp_inode.size = 0;
    memset(temp_inode.name, 0, MAX_FILENAME);

    // Write the updated inode (this will properly update sb.free_inodes)
    write_inode(inode_index, &temp_inode);

    sync_metadata_to_disk();
    return 0;
}

int fs_list(char filenames[][MAX_FILENAME], int max_files)
{
    if (max_files == 0)
    {
        return 0;
    }

    if (filenames == NULL || max_files < 0 || max_files > MAX_FILES || disk_fd == -1)
    {
        return -1;
    }

    int count_files = 0; // Counter for the number of files found

    for (int i = 0; i < MAX_FILES && count_files < max_files; i++)
    {
        if (inode_table[i].used == 1)
        { // If the inode is used
            // Find the length of the name in the raw array
            int name_len = 0;
            for (int j = 0; j < MAX_FILENAME && inode_table[i].name[j] != 0; j++)
            {
                name_len++;
            }

            memcpy(filenames[count_files], inode_table[i].name, name_len);
            filenames[count_files][name_len] = '\0'; // Add null terminator for the output
            count_files++;                           // Increment the count of files found
        }
    }

    return count_files; // Return the number of files found
}

int fs_write(const char *filename, const void *data, int size)
{
    if (filename == NULL || validate_string_manual(filename) != 0 || strlen(filename) > MAX_FILENAME || data == NULL || size < 0 || disk_fd < 0)
    {
        return -3;
    }

    int inode_index = find_inode(filename);
    if (inode_index == -1)
    {
        return -1;
    }

    int blocks_needed = calculate_blocks_needed(size);

    if (blocks_needed > sb.free_blocks)
    {
        return -2; // Error: too many blocks needed
    }

    if (blocks_needed > MAX_DIRECT_BLOCKS)
    {
        return -3;
    }

    inode target_inode;
    read_inode(inode_index, &target_inode);

    int original_blocks[MAX_DIRECT_BLOCKS];
    int original_size = target_inode.size;
    for (int i = 0; i < MAX_DIRECT_BLOCKS; i++)
    {
        original_blocks[i] = target_inode.blocks[i];
    }

    int new_blocks[MAX_DIRECT_BLOCKS];
    for (int i = 0; i < MAX_DIRECT_BLOCKS; i++)
    {
        new_blocks[i] = -1; // Initialize
    }

    // Allocate all blocks we need
    for (int i = 0; i < blocks_needed; i++)
    {
        new_blocks[i] = find_free_block();

        if (new_blocks[i] == -1)
        {
            // ROLLBACK: Free any blocks we allocated
            for (int j = 0; j < i; j++)
            {
                mark_block_free(new_blocks[j]);
            }
            return -2; // Not enough space
        }
        mark_block_used(new_blocks[i]);
    }

    const char *data_ptr = (const char *)data;
    int remaining_size = size;

    for (int i = 0; i < blocks_needed; i++)
    {
        int bytes_to_write = (remaining_size > BLOCK_SIZE) ? BLOCK_SIZE : remaining_size;

        lseek(disk_fd, new_blocks[i] * BLOCK_SIZE, SEEK_SET);

        // Handle partial writes with retry
        int total_written = 0;
        while (total_written < bytes_to_write)
        {
            int bytes_written = write(disk_fd, data_ptr + total_written,
                                      bytes_to_write - total_written);

            if (bytes_written < 0)
            {
                // Free all newly allocated blocks
                for (int j = 0; j < blocks_needed; j++)
                {
                    if (new_blocks[j] != -1)
                    {
                        mark_block_free(new_blocks[j]);
                    }
                }

                // Original data is still intact!
                return -3;
            }
            if (bytes_written == 0)
            {
                // Disk full - ROLLBACK
                // Free all newly allocated blocks

                for (int j = 0; j < blocks_needed; j++)
                {
                    if (new_blocks[j] != -1)
                    {
                        mark_block_free(new_blocks[j]);
                    }
                }

                return -2;
            }
            total_written += bytes_written;
        }

        data_ptr += bytes_to_write;
        remaining_size -= bytes_to_write;
    }

    // Update inode to point to new blocks
    for (int i = 0; i < MAX_DIRECT_BLOCKS; i++)
    {
        target_inode.blocks[i] = new_blocks[i];
    }
    target_inode.size = size;

    // Write updated inode
    write_inode(inode_index, &target_inode);

    // free the original blocks
    for (int i = 0; i < MAX_DIRECT_BLOCKS; i++)
    {
        if (original_blocks[i] != -1)
        {
            mark_block_free(original_blocks[i]);
        }
    }

    // Sync metadata to disk
    sync_metadata_to_disk();
    return 0;
}

int fs_read(const char *filename, void *buffer, int size)
{
    if (filename == NULL || validate_string_manual(filename) != 0 || strlen(filename) > MAX_FILENAME || buffer == NULL || size < 0 || disk_fd == -1)
    {
        return -3; // Error: invalid parameters
    }

    int inode_index = find_inode(filename);
    if (inode_index == -1)
    {
        return -1; // Error: file not found
    }

    // Use read_inode helper function to get the inode
    inode target_inode;
    read_inode(inode_index, &target_inode);

    if (target_inode.used == 0)
    {
        return -1; // Error: inode not used
    }

    int bytes_to_read = (size > target_inode.size) ? target_inode.size : size; // Read only up to the file size
    int total_bytes_read = 0;                                                  // Counter for total bytes read
    char *data_ptr = (char *)buffer;                                           // Pointer to the buffer where data will be read

    // Iterate over blocks
    for (int i = 0; i < MAX_DIRECT_BLOCKS && total_bytes_read < bytes_to_read; i++)
    {
        if (target_inode.blocks[i] == -1)
        {
            break; // No more blocks to read
        }

        int block_index = target_inode.blocks[i];
        if (block_index < 0 || block_index >= MAX_BLOCKS)
        {
            return -3; // Error: invalid block index
        }

        // Calculate how many bytes to read from this block
        int remaining_bytes = bytes_to_read - total_bytes_read;
        int bytes_from_block = (remaining_bytes > BLOCK_SIZE) ? BLOCK_SIZE : remaining_bytes;

        lseek(disk_fd, block_index * BLOCK_SIZE, SEEK_SET);
        int bytes_read = read(disk_fd, data_ptr, bytes_from_block);

        if (bytes_read < 0)
        {
            return -3; // Error: read failed
        }

        data_ptr += bytes_read;
        total_bytes_read += bytes_read;

        // If we read less than requested from this block, we've reached end of file
        if (bytes_read < bytes_from_block)
        {
            break;
        }
    }

    return total_bytes_read; // Return total bytes successfully read
}
