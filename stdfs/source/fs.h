#ifndef FS_H
#define FS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

// Platform-independent definitions for file modes
#define FS_READ     0x1
#define FS_WRITE    0x2
#define FS_APPEND   0x4
#define FS_BINARY   0x8
#define FS_CREATE   0x10
#define FS_TRUNCATE 0x20
#define FS_EXCLUSIVE 0x40

// Error codes for IO operations
typedef enum {
    FS_OK = 0,
    
    // Generic errors
    FS_ERROR_INVALID_ARGUMENT,    // Invalid parameter passed
    FS_ERROR_OUT_OF_MEMORY,       // Memory allocation failed
    FS_ERROR_BUFFER_TOO_SMALL,    // Provided buffer is too small
    
    // Permission/access errors
    FS_ERROR_PERMISSION_DENIED,   // No permission to access file/dir
    FS_ERROR_ACCESS_DENIED,       // System denied access (locks etc)
    
    // File/path errors
    FS_ERROR_NOT_FOUND,          // File/directory not found
    FS_ERROR_ALREADY_EXISTS,     // File/directory already exists
    FS_ERROR_PATH_TOO_LONG,      // Path exceeds system limits
    FS_ERROR_NAME_TOO_LONG,      // Filename exceeds system limits
    FS_ERROR_NOT_DIRECTORY,      // Path is not a directory
    FS_ERROR_IS_DIRECTORY,       // Path is a directory when file expected
    FS_ERROR_DIRECTORY_NOT_EMPTY,// Directory not empty on removal
    FS_ERROR_FILE_CLOSED,       // File was closed
    
    // I/O errors
    FS_ERROR_EOF,               // End of file reached
    FS_ERROR_DISK_FULL,        // No space left on device
    FS_ERROR_IO_ERROR,         // Generic I/O error
    FS_ERROR_BROKEN_PIPE,      // Broken pipe
    FS_ERROR_INTERRUPTED,      // Operation interrupted
    FS_ERROR_WOULD_BLOCK,      // Operation would block
    
    // System errors
    FS_ERROR_TOO_MANY_OPEN_FILES, // System limit on open files reached
    FS_ERROR_NOT_SUPPORTED,     // Operation not supported
    FS_ERROR_INVALID_HANDLE,    // Invalid file handle
    FS_ERROR_FILE_TOO_LARGE,    // File is too large
    // Network filesystem errors
    FS_ERROR_NETWORK_UNREACHABLE,// Network unreachable
    FS_ERROR_HOST_UNREACHABLE,  // Host unreachable
    FS_ERROR_CONNECTION_REFUSED,// Connection refused
    
    // Other errors
    FS_ERROR_UNKNOWN           // Unknown error occurred
} fs_error_code;

// File handle structure, compatible with different platforms
typedef struct {
    uintptr_t fd;         // File descriptor (POSIX) or handle (Windows)
    bool is_open;        // Status of file handle
#ifdef _WIN32
    HANDLE win_handle;   // Windows specific file handle
#endif
} fs_file;

/**
 * @brief Opens a file with the specified mode
 * @param file Pointer to fs_file structure to be initialized
 * @param filename Path to the file to be opened
 * @param mode Combination of FS_* flags defining access mode
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_open(fs_file *file, const char *filename, uint8_t mode);

/**
 * @brief Closes an open file handle
 * @param file Pointer to fs_file structure to be closed
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_close(fs_file *file);

/**
 * @brief Reads data from a file into a buffer
 * @param file Pointer to open fs_file structure
 * @param buffer Destination buffer to store read data
 * @param size Number of bytes to read
 * @param error Pointer to store error code
 * @return Number of bytes actually read
 */
uint64_t fs_read(fs_file *file, void **buffer, uint64_t size, uint8_t *error);

/**
 * @brief Reads a single line from a file
 * @param file Pointer to open fs_file structure
 * @param buffer Pointer to buffer pointer where memory will be allocated
 * @param size Pointer to store size of buffer
 * @param error Pointer to store error code
 * @return Number of bytes read
 */
uint64_t fs_readline(fs_file *file, char **buffer, uint8_t *error);


/**
 * @brief Reads entire file into memory
 * @param file Pointer to open fs_file structure
 * @param buffer Pointer to buffer pointer where memory will be allocated
 * @param error Pointer to store error code
 * @return Number of bytes read
 */
uint64_t fs_readall(fs_file *file, char **buffer, uint8_t *error);

/**
 * @brief Writes data from a buffer to a file
 * @param file Pointer to open fs_file structure
 * @param buffer Source buffer containing data to write
 * @param size Number of bytes to write
 * @param error Pointer to store error code
 * @return Number of bytes actually written
 */
uint64_t fs_write(fs_file *file, const void *buffer, uint64_t size, uint8_t *error);

/**
 * @brief Sets the file position indicator
 * @param file Pointer to open fs_file structure
 * @param offset Number of bytes to offset from whence
 * @param whence Position from where offset is applied (SEEK_SET, SEEK_CUR, SEEK_END)
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_seek(fs_file *file, int64_t offset, uint8_t whence);

/**
 * @brief Gets current position in file
 * @param file Pointer to open fs_file structure
 * @param error Pointer to store error code
 * @return Current position in file
 */
uint64_t fs_tell(fs_file *file, uint8_t *error);

/**
 * @brief Checks if end of file is reached
 * @param file Pointer to open fs_file structure
 * @return true if EOF reached, false otherwise
 */
uint8_t fs_eof(fs_file *file);

/**
 * @brief Flushes the file buffer
 * @param file Pointer to open fs_file structure
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_flush(fs_file *file);

/**
 * @brief Gets the size of a file
 * @param file Pointer to open fs_file structure
 * @param error Pointer to store error code
 * @return Size of file in bytes
 */
uint64_t fs_file_size(fs_file *file, uint8_t *error);

/**
 * @brief Deletes a file from the filesystem
 * @param filename Path to file to delete
 * @param error Pointer to store error code
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_delete(const char *filename);

/**
 * @brief Creates a new directory
 * @param dirname Path of directory to create
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_mkdir(const char *dirname);

/**
 * @brief Removes a directory
 * @param dirname Path of directory to remove
 * @param error Pointer to store error code
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_rmdir(const char *dirname);

/**
 * @brief Lists files in a directory
 * @param dirname Path to directory to list
 * @param files Pointer to array of strings to store filenames
 * @param count Pointer to store number of files found
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_listdir(const char *dirname, char ***files, uint32_t *count);

/**
 * @brief Frees memory allocated by fs_listdir
 * @param files Array of strings to free
 * @param count Number of strings in array
 */
void fs_free_file_list(char **files, uint32_t count);

/**
 * @brief Creates a temporary file
 * @param file Pointer to fs_file structure to initialize
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_create_temp(fs_file *file, char* name);

/**
 * @brief Gets current working directory
 * @param buffer Buffer to store path
 * @param size Size of buffer
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_get_cwd(char *buffer, uint32_t size);

/**
 * @brief Gets current user's home directory
 * @param buffer Buffer to store path
 * @param size Size of buffer
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_get_user_directory(char *buffer, uint32_t size);

/**
 * @brief Gets file attributes
 * @param filename Path to file
 * @param attributes Pointer to stat structure to fill
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_get_file_attributes(const char *filename, struct stat *attributes);

/**
 * @brief Sets file permissions
 * @param filename Path to file
 * @param mode Permission mode to set
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_chmod(const char *filename, mode_t mode);

/**
 * @brief Checks if file or directory exists
 * @param path Path to check
 * @param error Pointer to store error code
 * @return true if exists, false otherwise
 */
bool fs_exists(const char *path, uint8_t *error);

/**
 * @brief Copies a file
 * @param source Path to source file
 * @param destination Path to destination file
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_copy(const char *source, const char *destination);

/**
 * @brief Moves or renames a file
 * @param source Path to source file
 * @param destination New path for file
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_move(const char *source, const char *destination);

/**
 * @brief Creates a symbolic link
 * @param target Path that link points to
 * @param linkpath Path where link should be created
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_symlink(const char *target, const char *linkpath);

/**
 * @brief Reads contents of a symbolic link
 * @param linkpath Path to symbolic link
 * @param buffer Buffer to store target path
 * @param size Size of buffer
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_readlink(const char *linkpath, char *buffer, uint32_t size);

/**
 * @brief Gets absolute path
 * @param path Path to resolve
 * @param resolved_path Buffer to store absolute path
 * @param size Size of buffer
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_realpath(const char *path, char *resolved_path, uint32_t size);

/**
 * @brief Gets value of environment variable
 * @param name Name of environment variable
 * @param buffer Buffer to store value
 * @param size Size of buffer
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_getenv(const char *name, char *buffer, uint32_t size);

/**
 * @brief Sets environment variable
 * @param name Name of environment variable
 * @param value Value to set
 * @param overwrite Whether to overwrite if variable exists
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_setenv(const char *name, const char *value, uint8_t overwrite);

/**
 * @brief Removes environment variable
 * @param name Name of environment variable to remove
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_unsetenv(const char *name);

/**
 * @brief Joins two paths together
 * @param path1 First path component
 * @param path2 Second path component
 * @param result Buffer to store joined path
 * @param size Size of result buffer
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_path_join(const char *path1, const char *path2, char *result, uint32_t size);

/**
 * @brief Gets file extension from path
 * @param path Path to analyze
 * @param extension Buffer to store extension
 * @param size Size of extension buffer
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_get_extension(const char *path, char *extension, uint32_t size);

/**
 * @brief Splits path into directory and basename
 * @param path Path to split
 * @param dirname Buffer to store directory component
 * @param dir_size Size of dirname buffer
 * @param basename Buffer to store filename component
 * @param base_size Size of basename buffer
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_path_split(const char *path, char *dirname, uint32_t dir_size, char *basename, uint32_t base_size);

/**
 * @brief Reads entire file into memory
 * @param filename Path to file to read
 * @param buffer Pointer to buffer pointer where memory will be allocated
 * @param error Pointer to store error code
 * @return Number of bytes read, 0 on error
 */
uint64_t fs_read_all(const char *filename, void **buffer, uint8_t *error);

/**
 * @brief Writes string to file
 * @param file Pointer to open fs_file structure
 * @param string String to write
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_write_string(fs_file *file, const char *string);

/**
 * @brief Writes a string to a file with a specified length
 * @param file Pointer to open fs_file structure
 * @param string String to write
 * @param n Number of bytes to write
 * @return FS_OK on success, error code otherwise
 */
uint8_t fs_write_string_n(fs_file *file, const char *string, uint64_t n);

/**
 * @brief Gets platform-specific file separator
 * @return Character used as path separator ('/' or '\\')
 */
char fs_get_separator();

// Add helper function to convert system errors
uint8_t fs_map_system_error(void);

#endif // FS_H
