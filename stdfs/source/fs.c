#include "fs.h"

#ifdef _WIN32
#include <tchar.h>
#include <strsafe.h>
#else
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

uint8_t fs_open(fs_file *file, const char *filename, uint8_t mode) {
    if (!file || !filename) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

    int flags = 0;
#ifdef _WIN32
    DWORD access = 0;
    DWORD creation = OPEN_EXISTING;

    if (mode & FS_READ) access |= GENERIC_READ;
    if (mode & FS_WRITE) access |= GENERIC_WRITE;
    if (mode & FS_CREATE) creation = CREATE_ALWAYS;

    file->win_handle = CreateFileA(filename, access, 0, NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file->win_handle == INVALID_HANDLE_VALUE) {
        return fs_map_system_error();
    }
#else
    if (mode & FS_READ) flags |= O_RDONLY;
    if (mode & FS_WRITE) flags |= O_WRONLY;
    if (mode & FS_APPEND) flags |= O_APPEND;
    if (mode & FS_CREATE) flags |= O_CREAT;
    if (mode & FS_TRUNCATE) flags |= O_TRUNC;

    file->fd = open(filename, flags, 0644);
    if (file->fd == -1) {
        return fs_map_system_error();
    }
#endif

    file->is_open = true;
    return FS_OK;
}

uint8_t fs_close(fs_file *file) {
    if (!file || !file->is_open) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    if (CloseHandle(file->win_handle)) {
        file->is_open = false;
        return FS_OK;
    }
#else
    if (close(file->fd) == 0) {
        file->is_open = false;
        return FS_OK;
    }
#endif
    return fs_map_system_error();
}



uint64_t fs_read(fs_file *file, void *buffer, uint64_t size, uint8_t *error) {
    if (!file || !file->is_open || !buffer) {
        *error = FS_ERROR_INVALID_ARGUMENT;
        return 0;
    }

#ifdef _WIN32
    DWORD bytes_read;
    if (ReadFile(file->win_handle, buffer, size, &bytes_read, NULL)) {
        return (int64_t)bytes_read;
    }
#else
    ssize_t result = read(file->fd, buffer, size);
    if (result >= 0) {
        return (int64_t)result;
    }
#endif
    *error = fs_map_system_error();
    return 0;
}

uint64_t fs_readline(fs_file *file, char **buffer, uint8_t *error) {
    if (!file || !file->is_open || !buffer) {
        if (error) *error = FS_ERROR_INVALID_ARGUMENT;
        return 0;
    }

    // Initial buffer size and grow factor
    const uint64_t initial_buffer_size = 128;
    const uint64_t grow_factor = 2;

    // Allocate initial buffer if not already allocated
    if (*buffer == NULL) {
        *buffer = (char *)malloc(initial_buffer_size);
        if (*buffer == NULL) {
            if (error) *error = FS_ERROR_OUT_OF_MEMORY;
            return 0;
        }
    }

    char *buf = *buffer;
    uint64_t current_size = initial_buffer_size;
    uint64_t total_read = 0;

    // Internal buffer for efficient chunked reading
    char read_buf[64];
    uint64_t read_pos = 0, read_count = 0;

    // Temporary buffer for rollback calculations
    uint64_t rollback_bytes = 0;

    while (1) {
        // Refill the internal buffer if empty
        if (read_pos >= read_count) {
#ifdef _WIN32
            DWORD bytes_read;
            if (!ReadFile(file->win_handle, read_buf, sizeof(read_buf), &bytes_read, NULL)) {
                if (error) *error = fs_map_system_error();
                return 0;
            }
            read_count = bytes_read;
#else
            ssize_t result = read(file->fd, read_buf, sizeof(read_buf));
            if (result < 0) {
                if (error) *error = fs_map_system_error();
                return 0;
            }
            read_count = result;
#endif
            read_pos = 0;

            // If no more data is available, handle EOF
            if (read_count == 0) {
                if (total_read == 0) {
                    if (error) *error = FS_ERROR_FILE_CLOSED;
                    return 0;
                }
                break;
            }
        }

        // Process the internal buffer
        while (read_pos < read_count) {
            char c = read_buf[read_pos++];
            rollback_bytes++;

            // Check for newline
            if (c == '\n' || c == '\r') {
                // Handle Windows-style newlines (\r\n)
                if (c == '\r' && read_pos < read_count && read_buf[read_pos] == '\n') {
                    read_pos++;
                    rollback_bytes++;
                }

                // Roll back to the start of the next line
#ifdef _WIN32
                SetFilePointer(file->win_handle, -(LONG)(read_count - read_pos), NULL, FILE_CURRENT);
#else
                lseek(file->fd, -(off_t)(read_count - read_pos), SEEK_CUR);
#endif

                if (total_read > 0 && buf[total_read - 1] == '\r') {
                    total_read--; // Remove '\r' if it's at the end
                }

                if (error) *error = FS_OK;
                return total_read;
            }

            // Expand the buffer if needed
            if (total_read + 1 >= current_size) {
                current_size *= grow_factor;
                char *new_buf = (char *)realloc(buf, current_size);
                if (new_buf == NULL) {
                    if (error) *error = FS_ERROR_OUT_OF_MEMORY;
                    return 0;
                }
                buf = new_buf;
                *buffer = buf;
            }

            // Append the character to the buffer
            buf[total_read++] = c;
        }
    }

    // Handle EOF without a trailing newline
    if (error) *error = FS_OK;
    return total_read;
}


uint64_t fs_readall(fs_file *file, char **buffer, uint8_t *error) {
    if (!file || !file->is_open || !buffer) {
        if (error) *error = FS_ERROR_INVALID_ARGUMENT;
        return 0;
    }

    // Initial buffer size and grow factor
    const uint64_t initial_buffer_size = 1024;
    const uint64_t grow_factor = 2;

    // Allocate initial buffer if not already allocated
    if (*buffer == NULL) {
        *buffer = (char *)malloc(initial_buffer_size);
        if (*buffer == NULL) {
            if (error) *error = FS_ERROR_OUT_OF_MEMORY;
            return 0;
        }
    }

    char *buf = *buffer;
    uint64_t current_size = initial_buffer_size;
    uint64_t total_read = 0;

    // Internal buffer for efficient chunked reading
    char read_buf[256];
    uint64_t read_count = 0;

    while (1) {
#ifdef _WIN32
        DWORD bytes_read;
        if (!ReadFile(file->win_handle, read_buf, sizeof(read_buf), &bytes_read, NULL)) {
            if (error) *error = fs_map_system_error();
            return 0;
        }
        read_count = bytes_read;
#else
        ssize_t result = read(file->fd, read_buf, sizeof(read_buf));
        if (result < 0) {
            if (error) *error = fs_map_system_error();
            return 0;
        }
        read_count = result;
#endif

        // If no more data is available, break the loop
        if (read_count == 0) {
            break;
        }

        // Check if we need to grow the buffer
        if (total_read + read_count > current_size) {
            while (total_read + read_count > current_size) {
                current_size *= grow_factor;
            }
            char *new_buf = (char *)realloc(buf, current_size);
            if (new_buf == NULL) {
                if (error) *error = FS_ERROR_OUT_OF_MEMORY;
                return 0;
            }
            buf = new_buf;
            *buffer = buf;
        }

        // Copy the data from the internal buffer to the target buffer
        memcpy(buf + total_read, read_buf, read_count);
        total_read += read_count;
    }

    if (error) *error = FS_OK;
    return total_read;
}


uint64_t fs_write(fs_file *file, const void *buffer, uint64_t size, uint8_t *error) {
    if (!file || !file->is_open || !buffer) {
        *error = FS_ERROR_INVALID_ARGUMENT;
        return 0;
    }

#ifdef _WIN32
    DWORD bytes_written;
    if (WriteFile(file->win_handle, buffer, size, &bytes_written, NULL)) {
        return (int64_t)bytes_written;
    }
#else
    ssize_t result = write(file->fd, buffer, size);
    if (result >= 0) {
        return (int64_t)result;
    }
#endif
    return fs_map_system_error();
}

uint8_t fs_seek(fs_file *file, int64_t offset, uint8_t whence) {
    if (!file || !file->is_open) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    LARGE_INTEGER li;
    li.QuadPart = offset;
    DWORD move_method;
    switch (whence) {
        case SEEK_SET: move_method = FILE_BEGIN; break;
        case SEEK_CUR: move_method = FILE_CURRENT; break;
        case SEEK_END: move_method = FILE_END; break;
        default: return FS_INVALID;
    }
    li.LowPart = SetFilePointer(file->win_handle, li.LowPart, &li.HighPart, move_method);
    if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        return FS_ERROR;
    }
#else
    if (lseek(file->fd, offset, whence) == (off_t)-1) {
        return fs_map_system_error();
    }
#endif
    return FS_OK;
}

uint64_t fs_tell(fs_file *file, uint8_t *error){
    if (!file || !file->is_open) {
        *error = FS_ERROR_INVALID_ARGUMENT;
        return 0;
    }

#ifdef _WIN32
    LARGE_INTEGER li = {0};
    li.LowPart = SetFilePointer(file->win_handle, 0, &li.HighPart, FILE_CURRENT);
    if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        return FS_ERROR;
    }
    return li.QuadPart;
#else
    off_t result = lseek(file->fd, 0, SEEK_CUR);
    if (result != (off_t)-1) {
        return (int64_t)result;
    }
    *error = fs_map_system_error();
    return 0;
#endif
}

uint8_t fs_eof(fs_file *file) {
    if (!file || !file->is_open) {
        return 1;
    }

    uint8_t fs_tell_error = 0;
    int64_t current = fs_tell(file, &fs_tell_error);
    if (fs_tell_error != FS_OK) {
        return 1;
    }

    uint8_t fs_file_size_error = 0;
    int64_t size = fs_file_size(file, &fs_file_size_error);
    if (fs_file_size_error != FS_OK) {
        return 1;
    }

    return current >= size;
}

uint8_t fs_flush(fs_file *file) {
    if (!file || !file->is_open) {
        return FS_ERROR_INVALID_ARGUMENT;
    }
#ifdef _WIN32
    return FlushFileBuffers(file->win_handle) ? FS_OK : FS_ERROR;
#else
    return fsync(file->fd) == 0 ? FS_OK : fs_map_system_error();
#endif
}

uint64_t fs_file_size(fs_file *file, uint8_t *error) {
    if (!file || !file->is_open) {
        *error = FS_ERROR_INVALID_ARGUMENT;
        return 0;
    }

#ifdef _WIN32
    LARGE_INTEGER size;
    if (GetFileSizeEx(file->win_handle, &size)) {
        return size.QuadPart;
    }
#else
    struct stat st;
    if (fstat(file->fd, &st) == 0) {
        return st.st_size;
    }
#endif
    *error = fs_map_system_error();
    return 0;
}


uint8_t fs_delete(const char *filename) {
    if (!filename) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    if (DeleteFileA(filename)) {
        return FS_OK;
    }
#else
    if (unlink(filename) == 0) {
        return FS_OK;
    }
#endif
    return fs_map_system_error();
}

uint8_t fs_mkdir(const char *dirname) {
    if (!dirname) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    if (CreateDirectoryA(dirname, NULL)) {
        return FS_OK;
    }
#else
    if (mkdir(dirname, 0755) == 0) {
        return FS_OK;
    }
#endif
    return fs_map_system_error();
}

uint8_t fs_rmdir(const char *dirname) {
    if (!dirname) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    if (RemoveDirectoryA(dirname)) {
        return FS_OK;
    }
#else
    if (rmdir(dirname) == 0) {
        return FS_OK;
    }
#endif
    return fs_map_system_error();
}

uint8_t fs_listdir(const char *dirname, char ***files, uint32_t *count) {
    if (!dirname || !files || !count) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    WIN32_FIND_DATA find_data;
    HANDLE hFind;
    char search_path[MAX_PATH];
    snprintf(search_path, MAX_PATH, "%s\\*", dirname);

    hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        return fs_map_system_error();
    }

    *count = 0;
    *files = NULL;

    do {
        if (strcmp(find_data.cFileName, ".") != 0 && strcmp(find_data.cFileName, "..") != 0) {
            char **new_files = realloc(*files, sizeof(char *) * (*count + 1));
            if (!new_files) {
                fs_free_file_list(*files, *count);
                FindClose(hFind);
                return FS_ERROR_OUT_OF_MEMORY;
            }
            *files = new_files;
            (*files)[*count] = strdup(find_data.cFileName);
            if (!(*files)[*count]) {
                fs_free_file_list(*files, *count);
                FindClose(hFind);
                return FS_ERROR_OUT_OF_MEMORY;
            }
            (*count)++;
        }
    } while (FindNextFileA(hFind, &find_data) != 0);

    FindClose(hFind);
#else
    DIR *dir = opendir(dirname);
    if (!dir) {
        return fs_map_system_error();
    }

    struct dirent *entry;
    *count = 0;
    *files = NULL;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char **new_files = realloc(*files, sizeof(char *) * (*count + 1));
            if (!new_files) {
                fs_free_file_list(*files, *count);
                closedir(dir);
                return FS_ERROR_OUT_OF_MEMORY;
            }
            *files = new_files;
            (*files)[*count] = strdup(entry->d_name);
            if (!(*files)[*count]) {
                fs_free_file_list(*files, *count);
                closedir(dir);
                return FS_ERROR_OUT_OF_MEMORY;
            }
            (*count)++;
        }
    }

    closedir(dir);
#endif

    return FS_OK;
}

void fs_free_file_list(char **files, uint32_t count) {
    if (!files) {
        return;
    }

    for (uint32_t i = 0; i < count; i++) {
        free(files[i]);
    }
    free(files);
}

uint8_t fs_create_temp(fs_file *file) {
    if (!file) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (GetTempPathA(MAX_PATH, temp_path) == 0) {
        return FS_ERROR;
    }
    char temp_file[MAX_PATH];
    if (GetTempFileNameA(temp_path, "tmp", 0, temp_file) == 0) {
        return FS_ERROR;
    }
    return fs_open(file, temp_file, FS_READ | FS_WRITE | FS_CREATE);
#else
    char temp_file[] = "/tmp/fs_tempXXXXXX";
    int fd = mkstemp(temp_file);
    if (fd == -1) {
        return fs_map_system_error();
    }
    file->fd = fd;
    file->is_open = true;
    return FS_OK;
#endif
}


uint8_t fs_get_cwd(char *buffer, uint32_t size) {
    if (!buffer || size == 0) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    // Use GetCurrentDirectoryA to get the current working directory
    DWORD length = GetCurrentDirectoryA(size, buffer);
    if (length == 0) {
        return FS_ERROR; // Could not retrieve current directory
    } else if (length >= size) {
        return FS_ERROR_BUFFER_TOO_SMALL; // Buffer is too small
    }
#else
    // Use getcwd on POSIX systems
    if (!getcwd(buffer, size)) {
        return (errno == ERANGE) ? FS_ERROR_BUFFER_TOO_SMALL : FS_ERROR_UNKNOWN;
    }
#endif

    return FS_OK;
}

uint8_t fs_get_user_directory(char *buffer, uint32_t size) {
    if (!buffer) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, buffer))) {
        return FS_OK;
    }
#else
    struct passwd *pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        strncpy(buffer, pw->pw_dir, size);
        return FS_OK;
    }
#endif
    return fs_map_system_error();
}

uint8_t fs_get_file_attributes(const char *filename, struct stat *attributes) {
    if (!filename || !attributes) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA file_info;
    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &file_info)) {
        // File size
        attributes->st_size = ((uint64_t)file_info.nFileSizeHigh << 32) | file_info.nFileSizeLow;

        // Convert Windows FILETIME to time_t for access, modify, and creation times
        ULARGE_INTEGER ull;

        // Last access time
        ull.LowPart = file_info.ftLastAccessTime.dwLowDateTime;
        ull.HighPart = file_info.ftLastAccessTime.dwHighDateTime;
        attributes->st_atime = (ull.QuadPart / 10000000ULL - 11644473600ULL);

        // Last modify time
        ull.LowPart = file_info.ftLastWriteTime.dwLowDateTime;
        ull.HighPart = file_info.ftLastWriteTime.dwHighDateTime;
        attributes->st_mtime = (ull.QuadPart / 10000000ULL - 11644473600ULL);

        // Creation time (Windows specific, stored in st_ctime for POSIX compatibility)
        ull.LowPart = file_info.ftCreationTime.dwLowDateTime;
        ull.HighPart = file_info.ftCreationTime.dwHighDateTime;
        attributes->st_ctime = (ull.QuadPart / 10000000ULL - 11644473600ULL);

        // File mode/permissions
        attributes->st_mode = 0;
        if (file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            attributes->st_mode |= S_IFDIR;
        } else {
            attributes->st_mode |= S_IFREG;
        }

        // Basic permissions (simplified)
        if (!(file_info.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
            attributes->st_mode |= S_IWRITE | S_IWUSR | S_IWGRP;
        }
        attributes->st_mode |= S_IREAD | S_IEXEC | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP;

        // Additional attributes
        if (file_info.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
            attributes->st_mode |= S_IRUSR | S_IWUSR | S_IXUSR;  // System file
        }
        if (file_info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
            attributes->st_mode &= ~(S_IRGRP | S_IWGRP | S_IXGRP);  // Hidden file
        }

        // Get additional information using GetFileInformationByHandle
        HANDLE hFile = CreateFileA(
            filename,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,  // Needed for directories
            NULL
        );

        if (hFile != INVALID_HANDLE_VALUE) {
            BY_HANDLE_FILE_INFORMATION fileInfo;
            if (GetFileInformationByHandle(hFile, &fileInfo)) {
                // Number of links
                attributes->st_nlink = fileInfo.nNumberOfLinks;
                
                // Device and inode information
                attributes->st_dev = fileInfo.dwVolumeSerialNumber;
                attributes->st_ino = ((uint64_t)fileInfo.nFileIndexHigh << 32) | fileInfo.nFileIndexLow;
                
                // User and group IDs (Windows doesn't have direct equivalents)
                attributes->st_uid = 0;  // Current user
                attributes->st_gid = 0;  // Primary group
            }
            CloseHandle(hFile);
        }

        return FS_OK;
    }
#else
    if (stat(filename, attributes) == 0) {
        return FS_OK;
    }
#endif
    return fs_map_system_error();
}

uint8_t fs_chmod(const char *filename, mode_t mode) {
    if (!filename) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    return FS_ERROR_NOT_SUPPORTED;
#else
    if (chmod(filename, mode) == 0) {
        return FS_OK;
    }
    return fs_map_system_error();
#endif
}

bool fs_exists(const char *path, uint8_t *error) {
    if (!path) {
        return false;
    }

#ifdef _WIN32
    DWORD attributes = GetFileAttributesA(path);
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        return true;
    }
#else
    if (access(path, F_OK) == 0) {
        return true;
    }
#endif
    return false;
}

uint8_t fs_copy(const char *source, const char *destination) {
    if (!source || !destination) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

    fs_file src_file, dest_file;
    uint8_t open_error;

    if (fs_open(&src_file, source, FS_READ) != FS_OK) {
        return open_error;
    }

    if (fs_open(&dest_file, destination, FS_WRITE | FS_CREATE | FS_TRUNCATE) != FS_OK) {
        fs_close(&src_file);
        return open_error;
    }

    char buffer[4096];
    uint64_t bytes_read;
    uint8_t read_error, write_error;
    bool success = true;


    while ((bytes_read = fs_read(&src_file, buffer, sizeof(buffer), &read_error)) > 0) {
        if (read_error != FS_OK) {
            success = false;
            return read_error;
        }
        uint64_t bytes_written = fs_write(&dest_file, buffer, bytes_read, &write_error);
        if (write_error != FS_OK || bytes_written != bytes_read) {
            success = false;
            return write_error;
        }
    }

    if (read_error != FS_OK && read_error != FS_ERROR_EOF) {
        return read_error;
    }

    fs_close(&src_file);
    fs_close(&dest_file);

    if (success) {
        return FS_OK;
    }
    return FS_ERROR_IO_ERROR;
}

uint8_t fs_move(const char *source, const char *destination) {
    if (!source || !destination) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    if (MoveFileA(source, destination)) {
        return FS_OK;
    }
#else
    if (rename(source, destination) == 0) {
        return FS_OK;
    }
#endif
    return fs_map_system_error();
}

uint8_t fs_symlink(const char *target, const char *linkpath) {
    if (!target || !linkpath) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    return FS_ERROR_NOT_SUPPORTED;
#else
    if (symlink(target, linkpath) == 0) {
        return FS_OK;
    }
    return fs_map_system_error();
#endif
}

uint8_t fs_readlink(const char *linkpath, char *buffer, uint32_t size) {
    if (!linkpath || !buffer || size == 0) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    return FS_ERROR_NOT_SUPPORTED;
#else
    ssize_t len = readlink(linkpath, buffer, size - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return FS_OK;
    }
    return fs_map_system_error();
#endif
}

uint8_t fs_realpath(const char *path, char *resolved_path, uint32_t size) {
    if (!path || !resolved_path || size == 0) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    if (GetFullPathNameA(path, size, resolved_path, NULL)) {
        return FS_OK;
    }
#else
    if (realpath(path, resolved_path)) {
        return FS_OK;
    }
#endif
    return fs_map_system_error();
}

uint8_t fs_getenv(const char *name, char *buffer, uint32_t size) {
    if (!name || !buffer || size == 0) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

    char *env = getenv(name);
    if (env) {
        if (strlen(env) >= size) {
            return FS_ERROR_BUFFER_TOO_SMALL;
        }
        strncpy(buffer, env, size - 1);
        buffer[size - 1] = '\0';
        return FS_OK;
    }
    return FS_ERROR_NOT_FOUND;
}

uint8_t fs_setenv(const char *name, const char *value, uint8_t overwrite) {
    if (!name || !value) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    if (overwrite || !getenv(name)) {
        if (SetEnvironmentVariableA(name, value)) {
            return FS_OK;
        }
    } else {
        return FS_ERROR_ALREADY_EXISTS;
    }
#else
    if (setenv(name, value, overwrite) == 0) {
        return FS_OK;
    }
#endif
    return fs_map_system_error();
}

uint8_t fs_unsetenv(const char *name) {
    if (!name) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

#ifdef _WIN32
    if (SetEnvironmentVariableA(name, NULL)) {
        return FS_OK;
    }
#else
    if (unsetenv(name) == 0) {
        return FS_OK;
    }
#endif
    return fs_map_system_error();
}

uint8_t fs_path_join(const char *path1, const char *path2, char *result, uint32_t size) {
    if (!path1 || !path2 || !result || size == 0) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

    size_t len = snprintf(result, size, "%s%c%s", path1, fs_get_separator(), path2);
    if (len >= size) {
        return FS_ERROR_BUFFER_TOO_SMALL;
    }

    return FS_OK;
}

uint8_t fs_get_extension(const char *path, char *extension, uint32_t size) {
    if (!path || !extension || size == 0) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

    const char *dot = strrchr(path, '.');
    if (!dot || dot == path) {
        extension[0] = '\0';
        return FS_OK;
    }

    if (strlen(dot + 1) >= size) {
        return FS_ERROR_BUFFER_TOO_SMALL;
    }

    strncpy(extension, dot + 1, size - 1);
    extension[size - 1] = '\0';
    return FS_OK;
}

uint8_t fs_path_split(const char *path, char *dirname, uint32_t dir_size, 
                      char *basename, uint32_t base_size) {
    if (!path || !dirname || !basename || dir_size == 0 || base_size == 0) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

    const char *slash = strrchr(path, fs_get_separator());
    if (slash) {
        size_t dir_len = slash - path;
        if (dir_len >= dir_size) {
            return FS_ERROR_BUFFER_TOO_SMALL;
        }
        snprintf(dirname, dir_size, "%.*s", (int)(slash - path), path);

        if (strlen(slash + 1) >= base_size) {
            return FS_ERROR_BUFFER_TOO_SMALL;
        }
        strncpy(basename, slash + 1, base_size - 1);
        basename[base_size - 1] = '\0';
    } else {
        dirname[0] = '\0';
        if (strlen(path) >= base_size) {
            return FS_ERROR_BUFFER_TOO_SMALL;
        }
        strncpy(basename, path, base_size - 1);
        basename[base_size - 1] = '\0';
    }

    return FS_OK;
}

uint64_t fs_read_all(const char *filename, void **buffer, uint8_t *error) {
    if (!filename || !buffer) {
        return 0;
    }

    fs_file file;
    uint8_t open_error = fs_open(&file, filename, FS_READ);
    if (open_error != FS_OK) {
        *error = open_error;
        return 0;
    }

    uint8_t size_error;
    uint64_t size = fs_file_size(&file, &size_error);
    if (size_error != FS_OK) {
        fs_close(&file);
        *error = size_error;
        return 0;
    }

    *buffer = malloc(size);
    if (!*buffer) {
        fs_close(&file);
        *error = FS_ERROR_OUT_OF_MEMORY;
        return 0;
    }

    uint8_t read_error;
    uint64_t bytes_read = fs_read(&file, *buffer, size, &read_error);
    fs_close(&file);

    if (read_error != FS_OK || bytes_read != size) {
        free(*buffer);
        *error = read_error;
        return 0;
    }

    return size;
}

uint8_t fs_write_string(fs_file *file, const char *string) {
    if (!file || !file->is_open || !string) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

    uint8_t write_error;
    size_t len = strlen(string);
    uint64_t bytes_written = fs_write(file, string, len, &write_error);
    return bytes_written == len ? FS_OK : write_error;
}

uint8_t fs_write_string_n(fs_file *file, const char *string, uint64_t n) {
    if (!file || !file->is_open || !string) {
        return FS_ERROR_INVALID_ARGUMENT;
    }

    uint8_t write_error;
    uint64_t bytes_written = fs_write(file, string, n, &write_error);
    return bytes_written == n ? FS_OK : write_error;
}

char fs_get_separator() {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

uint8_t fs_map_system_error() {
#ifdef _WIN32
    DWORD error = GetLastError();
    switch (error) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return FS_ERROR_NOT_FOUND;
        case ERROR_ACCESS_DENIED:
            return FS_ERROR_ACCESS_DENIED;
        case ERROR_ALREADY_EXISTS:
            return FS_ERROR_ALREADY_EXISTS;
        case ERROR_DISK_FULL:
            return FS_ERROR_DISK_FULL;
        case ERROR_HANDLE_EOF:
            return FS_ERROR_EOF;
        case ERROR_INSUFFICIENT_BUFFER:
            return FS_ERROR_BUFFER_TOO_SMALL;
        case ERROR_INVALID_HANDLE:
            return FS_ERROR_INVALID_HANDLE;
        case ERROR_TOO_MANY_OPEN_FILES:
            return FS_ERROR_TOO_MANY_OPEN_FILES;
        case ERROR_DIRECTORY:
            return FS_ERROR_IS_DIRECTORY;
        case ERROR_DIR_NOT_EMPTY:
            return FS_ERROR_DIRECTORY_NOT_EMPTY;
        case ERROR_BROKEN_PIPE:
            return FS_ERROR_BROKEN_PIPE;
        default:
            return FS_ERROR_UNKNOWN;
    }
#else
    switch (errno) {
        case ENOENT:
            return FS_ERROR_NOT_FOUND;
        case EACCES:
        case EPERM:
            return FS_ERROR_PERMISSION_DENIED;
        case EEXIST:
            return FS_ERROR_ALREADY_EXISTS;
        case ENOSPC:
            return FS_ERROR_DISK_FULL;
        case ENOTDIR:
            return FS_ERROR_NOT_DIRECTORY;
        case EISDIR:
            return FS_ERROR_IS_DIRECTORY;
        case ENOTEMPTY:
            return FS_ERROR_DIRECTORY_NOT_EMPTY;
        case EMFILE:
        case ENFILE:
            return FS_ERROR_TOO_MANY_OPEN_FILES;
        case EINVAL:
            return FS_ERROR_INVALID_ARGUMENT;
        case EBADF:
            return FS_ERROR_INVALID_HANDLE;
        case EFBIG:
            return FS_ERROR_FILE_TOO_LARGE;
        case EPIPE:
            return FS_ERROR_BROKEN_PIPE;
        case EINTR:
            return FS_ERROR_INTERRUPTED;
        case EWOULDBLOCK:
            return FS_ERROR_WOULD_BLOCK;
        case ENETUNREACH:
            return FS_ERROR_NETWORK_UNREACHABLE;
        case EHOSTUNREACH:
            return FS_ERROR_HOST_UNREACHABLE;
        case ECONNREFUSED:
            return FS_ERROR_CONNECTION_REFUSED;
        default:
            return FS_ERROR_UNKNOWN;
    }
#endif
}

