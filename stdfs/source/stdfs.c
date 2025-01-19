//
// Created by praisethemoon on 03.12.23.
//

#ifndef TYPE_V_STDIO_C
#define TYPE_V_STDIO_C

#include <stdio.h>
#include <string.h>

#include "../../source/core.h"
#include "../../source/api/typev_api.h"
#include "fs.h"
#include "cwalk.h"

/**
 * FS functions
 */
void _fs_open(TypeV_Core* core) {
    TypeV_Array* name = typev_api_stack_pop_array(core);
    uint8_t mode = typev_api_stack_pop_u8(core);

    fs_file* f = (fs_file*)malloc(sizeof(fs_file));

    uint8_t error = fs_open(f, name->data, mode);

    typev_api_return_u8(core, error);
    typev_api_return_u64(core, (uint64_t)f);
}


void _fs_close(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    uint8_t error = fs_close(f);
    typev_api_return_u8(core, error);
}

void _fs_read_one(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    
    uint8_t error = 0;
    uint8_t result = fs_read_one(f, &error);
    
    typev_api_return_u8(core, error);
    typev_api_return_u8(core, result);
}

void _fs_read(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    uint64_t size = typev_api_stack_pop_u64(core);

    char** buffer = NULL;
    
    uint8_t error = 0;
    uint64_t result = fs_read(f, &buffer, size, &error);

    TypeV_Array* array = typev_api_array_create_from_buffer(core, result, 1, 0, buffer);
    
    typev_api_return_u8(core, error);
    typev_api_return_array(core, array);
}

void _fs_readline(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    char* buffer = NULL;
    uint8_t error = 0;
    uint64_t size = fs_readline(f, &buffer, &error);

    TypeV_Array* result = typev_api_array_create(core, size, 1, 0);
    memcpy(result->data, buffer, size);
    typev_api_return_u8(core, error);
    typev_api_return_array(core, result);
}

void _fs_readall(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    char* buffer = NULL;
    uint8_t error = 0;
    uint64_t size = fs_readall(f, &buffer, &error);

    TypeV_Array* result = typev_api_array_create_from_buffer(core, size, 1, 0, buffer);
    typev_api_return_u8(core, error);
    typev_api_return_array(core, result);
}

void _fs_write(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    TypeV_Array* buffer = typev_api_stack_pop_array(core);
    uint64_t size = typev_api_stack_pop_u64(core);
    
    uint8_t error = 0;
    uint64_t result = fs_write(f, buffer->data, size, &error);

    typev_api_return_u8(core, error);
    typev_api_return_u64(core, result);
}

void _fs_seek(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    int64_t offset = typev_api_stack_pop_u64(core);
    uint8_t whence = typev_api_stack_pop_u8(core);

    uint8_t error = fs_seek(f, offset, whence);
    typev_api_return_u8(core, error);
}

void _fs_tell(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    uint8_t error = 0;
    int64_t result = fs_tell(f, &error);
    typev_api_return_u8(core, error);
    typev_api_return_i64(core, result);
}

void _fs_eof(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    uint8_t result = fs_eof(f);
    typev_api_return_u8(core, result);
}

void _fs_flush(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    uint8_t result = fs_flush(f);
    typev_api_return_u8(core, result);
}

void _fs_file_size(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    uint8_t error = 0;
    uint64_t result = fs_file_size(f, &error);
    typev_api_return_u8(core, error);
    typev_api_return_u64(core, result);
}

void _fs_delete(TypeV_Core* core) {
    TypeV_Array* name = typev_api_stack_pop_array(core);
    uint8_t error = fs_delete(name->data);
    typev_api_return_u8(core, error);
}

void _fs_mkdir(TypeV_Core* core) {
    TypeV_Array* name = typev_api_stack_pop_array(core);
    uint8_t error = fs_mkdir(name->data);
    typev_api_return_u8(core, error);
}

void _fs_rmdir(TypeV_Core* core) {
    TypeV_Array* name = typev_api_stack_pop_array(core);
    uint8_t error = fs_rmdir(name->data);
    typev_api_return_u8(core, error);
}

/**
 * TODO: this can optimized by storing data directly in the array
 * instead of allocating strings then copying them
 */
void _fs_listdir(TypeV_Core* core) {
    TypeV_Array* name = typev_api_stack_pop_array(core);
    char** files;
    uint32_t count;
    uint8_t error = fs_listdir(name->data, &files, &count);

    TypeV_Array* result = typev_api_array_create(core, count, 8, 1);
    for (uint32_t i = 0; i < count; i++) {
        TypeV_Array* file = typev_api_array_create(core, strlen(files[i]), 1, 0);
        memcpy(file->data, files[i], strlen(files[i]));
        typev_api_array_set(core, result, i, &file);
    }

    fs_free_file_list(files, count);

    typev_api_return_u8(core, error);
    typev_api_return_array(core, result);
}

void _fs_create_temp(TypeV_Core* core) {
    fs_file* f = (fs_file*)malloc(sizeof(fs_file));
    char name[FILENAME_MAX] = {0};
    uint8_t error = fs_create_temp(f, name);

    TypeV_Array* result = typev_api_array_create(core, strlen(name), 1, 0);
    memcpy(result->data, name, strlen(name));

    typev_api_return_u8(core, error);
    typev_api_return_u64(core, (uint64_t)f);
    typev_api_return_array(core, result);
}

void _fs_get_cwd(TypeV_Core* core) {
    char path[256] = {0};
    uint8_t error = fs_get_cwd(path, 256);
    TypeV_Array* result = typev_api_array_create(core, strlen(path), 1, 0);
    memcpy(result->data, path, strlen(path));
    typev_api_return_u8(core, error);
    typev_api_return_array(core, result);
}

void _fs_get_user_directory(TypeV_Core* core) {
    char path[256] = {0};
    uint8_t error = fs_get_user_directory(path, 256);

    TypeV_Array* result = typev_api_array_create(core, strlen(path), 1, 0);
    memcpy(result->data, path, strlen(path));
    typev_api_return_u8(core, error);
    typev_api_return_array(core, result);
}

void _fs_get_file_attributes(TypeV_Core* core) {
    TypeV_Array* name = typev_api_stack_pop_array(core);
    struct stat attributes;
    uint8_t error = fs_get_file_attributes(name->data, &attributes);

    typev_api_return_u8(core, error);                           // Error code
    typev_api_return_u64(core, (uint64_t)attributes.st_dev);    // Device ID
    typev_api_return_u64(core, (uint64_t)attributes.st_ino);    // Inode number
    typev_api_return_u32(core, (uint32_t)attributes.st_nlink);  // Number of hard links
    typev_api_return_u32(core, (uint32_t)attributes.st_rdev);   // Device ID (if special file)
    typev_api_return_u64(core, (uint64_t)attributes.st_blksize);// Block size for filesystem I/O
    typev_api_return_u64(core, (uint64_t)attributes.st_blocks); // Number of 512B blocks allocated
    typev_api_return_u64(core, (uint64_t)attributes.st_mtime);  // Modification time
    typev_api_return_u64(core, (uint64_t)attributes.st_atime);  // Access time
    typev_api_return_u64(core, (uint64_t)attributes.st_ctime);  // Creation time
    typev_api_return_u64(core, (uint64_t)attributes.st_size);   // File size
    typev_api_return_u32(core, (uint32_t)attributes.st_mode);   // File mode/permissions
    typev_api_return_u32(core, (uint32_t)attributes.st_uid);    // User ID
    typev_api_return_u32(core, (uint32_t)attributes.st_gid);    // Group ID
}

void _fs_chmod(TypeV_Core* core) {
    TypeV_Array* name = typev_api_stack_pop_array(core);
    uint32_t mode = typev_api_stack_pop_u32(core);
    uint8_t error = fs_chmod(name->data, mode);
    typev_api_return_u8(core, error);
}

void _fs_exists(TypeV_Core* core) {
    TypeV_Array* name = typev_api_stack_pop_array(core);
    uint8_t error = 0;
    bool result = fs_exists(name->data, &error);
    typev_api_return_u8(core, error);
    typev_api_return_u8(core, result);
}

void _fs_copy(TypeV_Core* core) {
    TypeV_Array* source = typev_api_stack_pop_array(core);
    TypeV_Array* destination = typev_api_stack_pop_array(core);
    uint8_t error = fs_copy(source->data, destination->data);
    typev_api_return_u8(core, error);
}

void _fs_move(TypeV_Core* core) {
    TypeV_Array* source = typev_api_stack_pop_array(core);
    TypeV_Array* destination = typev_api_stack_pop_array(core);
    uint8_t error = fs_move(source->data, destination->data);
    typev_api_return_u8(core, error);
}

void _fs_symlink(TypeV_Core* core) {
    TypeV_Array* target = typev_api_stack_pop_array(core);
    TypeV_Array* linkpath = typev_api_stack_pop_array(core);
    uint8_t error = fs_symlink(target->data, linkpath->data);
    typev_api_return_u8(core, error);
}

void _fs_readlink(TypeV_Core* core) {
    TypeV_Array* linkpath = typev_api_stack_pop_array(core);
    char buffer[256] = {0};
    uint8_t error = fs_readlink(linkpath->data, buffer, 256);
    TypeV_Array* result = typev_api_array_create(core, strlen(buffer), 1, 0);
    memcpy(result->data, buffer, strlen(buffer));
    typev_api_return_u8(core, error);
    typev_api_return_array(core, result);
}

void _fs_realpath(TypeV_Core* core) {
    TypeV_Array* path = typev_api_stack_pop_array(core);
    char buffer[256] = {0};
    uint8_t error = fs_realpath(path->data, buffer, 256);
    TypeV_Array* result = typev_api_array_create(core, strlen(buffer), 1, 0);
    memcpy(result->data, buffer, strlen(buffer));
    typev_api_return_u8(core, error);
    typev_api_return_array(core, result);
}

void _fs_getenv(TypeV_Core* core) {
    TypeV_Array* name = typev_api_stack_pop_array(core);
    char buffer[256] = {0};
    uint8_t error = fs_getenv(name->data, buffer, 256);
    TypeV_Array* result = typev_api_array_create(core, strlen(buffer), 1, 0);
    memcpy(result->data, buffer, strlen(buffer));
    typev_api_return_u8(core, error);
    typev_api_return_array(core, result);
}

void _fs_setenv(TypeV_Core* core) {
    TypeV_Array* name = typev_api_stack_pop_array(core);
    TypeV_Array* value = typev_api_stack_pop_array(core);
    uint8_t overwrite = typev_api_stack_pop_u8(core);
    uint8_t error = fs_setenv(name->data, value->data, overwrite);
    typev_api_return_u8(core, error);
}

void _fs_unsetenv(TypeV_Core* core) {
    TypeV_Array* name = typev_api_stack_pop_array(core);
    uint8_t error = fs_unsetenv(name->data);
    typev_api_return_u8(core, error);
}
void _fs_get_separator(TypeV_Core* core) {
    char separator = fs_get_separator();
    typev_api_return_u8(core, separator);
}

/**
 * Path functions, uses cwalk library
 */

void _path_get_basename(TypeV_Core* core) {
    TypeV_Array* pathArray = typev_api_stack_pop_array(core);
    const char* path = (const char*)pathArray->data;
    const char* basename;
    size_t length;
    // points within the path, so we need to copy it
    cwk_path_get_basename(path, &basename, &length);

    TypeV_Array* result = typev_api_array_create(core, length, 1, 0);
    memcpy(result->data, basename, length);
    typev_api_return_array(core, result);
}

void _path_change_basename(TypeV_Core* core) {
    // Pop arguments from the stack in right order because compiler fixes the order
    TypeV_Array* path = typev_api_stack_pop_array(core);
    TypeV_Array* new_basename = typev_api_stack_pop_array(core);
    
    // Create a buffer for the result
    // Using 256 as a reasonable buffer size, adjust if needed
    char buffer[FILENAME_MAX] = {0};
    
    // Call cwk_path_change_basename
    size_t result_size = cwk_path_change_basename(
        (const char*)path->data,
        (const char*)new_basename->data,
        buffer,
        256
    );
    
    // Create a new array for the result
    // Use the actual written size (minimum of result_size and 255 to account for null terminator)
    size_t actual_size = result_size < 256 ? result_size : 255;
    TypeV_Array* result = typev_api_array_create(core, actual_size, 1, 0);
    
    // Copy the result into the new array
    memcpy(result->data, buffer, actual_size);
    
    // Return the result array
    typev_api_return_array(core, result);
}

void _path_get_dirname(TypeV_Core* core) {
    // Pop the path argument from the stack
    TypeV_Array* path = typev_api_stack_pop_array(core);
    
    // Get the dirname length
    size_t length = 0;
    cwk_path_get_dirname((const char*)path->data, &length);
    
    // Create a new array for the result
    // The size will be the length of the dirname
    TypeV_Array* result = typev_api_array_create(core, length, 1, 0);
    
    // Copy the dirname portion into the result array
    if (length > 0) {
        memcpy(result->data, path->data, length);
    }
    
    // Return the result array
    typev_api_return_array(core, result);
}

void _path_get_root(TypeV_Core* core) {
    // Pop the path argument from the stack
    TypeV_Array* path = typev_api_stack_pop_array(core);
    
    // Get the root length
    size_t length = 0;
    cwk_path_get_root((const char*)path->data, &length);
    
    // Create a new array for the result
    // The size will be the length of the root
    TypeV_Array* result = typev_api_array_create(core, length, 1, 0);
    
    // Copy the root portion into the result array
    if (length > 0) {
        memcpy(result->data, path->data, length);
    }
    
    // Return the result array
    typev_api_return_array(core, result);
}

void _path_change_root(TypeV_Core* core) {
    // Pop arguments from the stack in right order because compiler fixes the order
    TypeV_Array* path = typev_api_stack_pop_array(core);
    TypeV_Array* new_root = typev_api_stack_pop_array(core);
    
    // Create a buffer for the result
    char buffer[FILENAME_MAX] = {0};
    
    // Call cwk_path_change_root
    size_t result_size = cwk_path_change_root(
        (const char*)path->data,
        (const char*)new_root->data,
        buffer,
        FILENAME_MAX
    );
    
    // Create a new array for the result
    // Use the actual written size (minimum of result_size and FILENAME_MAX-1 to account for null terminator)
    size_t actual_size = result_size < FILENAME_MAX - 1 ? result_size : FILENAME_MAX - 1;
    TypeV_Array* result = typev_api_array_create(core, actual_size, 1, 0);
    
    // Copy the result into the new array
    memcpy(result->data, buffer, actual_size);
    
    // Return the result array
    typev_api_return_array(core, result);
}

void _path_is_absolute(TypeV_Core* core) {
    TypeV_Array* path = typev_api_stack_pop_array(core);
    uint8_t result = cwk_path_is_absolute((const char*)path->data);
    typev_api_return_u8(core, result);
}

void _path_is_relative(TypeV_Core* core) {
    TypeV_Array* path = typev_api_stack_pop_array(core);
    uint8_t result = cwk_path_is_relative((const char*)path->data);
    typev_api_return_u8(core, result);
}

void _path_join(TypeV_Core* core) {
    TypeV_Array* path = typev_api_stack_pop_array(core);
    TypeV_Array* path2 = typev_api_stack_pop_array(core);
    char buffer[FILENAME_MAX] = {0};
    cwk_path_join((const char*)path->data, (const char*)path2->data, buffer, FILENAME_MAX);
    TypeV_Array* result = typev_api_array_create(core, strlen(buffer), 1, 0);
    memcpy(result->data, buffer, strlen(buffer));
    typev_api_return_array(core, result);
}
void _path_join_multiple(TypeV_Core* core) {

}

void _path_normalize(TypeV_Core* core) {
    TypeV_Array* path = typev_api_stack_pop_array(core);
    char buffer[FILENAME_MAX] = {0};
    size_t len = cwk_path_normalize((const char*)path->data, buffer, FILENAME_MAX);
    TypeV_Array* result = typev_api_array_create(core, len, 1, 0);
    memcpy(result->data, buffer, strlen(buffer));
    typev_api_return_array(core, result);
}
void _path_intersection(TypeV_Core* core) {
    TypeV_Array* path = typev_api_stack_pop_array(core);
    TypeV_Array* path2 = typev_api_stack_pop_array(core);
    char buffer[FILENAME_MAX] = {0};
    size_t length = cwk_path_get_intersection((const char*)path->data, (const char*)path2->data);
    TypeV_Array* result = typev_api_array_create(core, length, 1, 0);
    memcpy(result->data, path->data, length);
    typev_api_return_array(core, result);
}

void _path_get_absolute(TypeV_Core* core) {}
void _path_get_relative(TypeV_Core* core) {}

void _path_get_extension(TypeV_Core* core) {}
void _path_has_extension(TypeV_Core* core) {}
void _path_change_extension(TypeV_Core* core) {}

void _path_get_first_segment(TypeV_Core* core) {}
void _path_get_last_segment(TypeV_Core* core) {}
void _path_get_next_segment(TypeV_Core* core) {}
void _path_get_previous_segment(TypeV_Core* core) {}
void _path_get_segment_type(TypeV_Core* core) {}
void _path_change_segment(TypeV_Core* core) {}

void _path_guess_style(TypeV_Core* core) {}
void _path_set_style(TypeV_Core* core) {}
void _path_get_style(TypeV_Core* core) {}

static TypeV_FFIFunc stdfs_lib[] = {
    (TypeV_FFIFunc)_fs_open,
    (TypeV_FFIFunc)_fs_close,
    (TypeV_FFIFunc)_fs_read_one,
    (TypeV_FFIFunc)_fs_read,
    (TypeV_FFIFunc)_fs_readline,
    (TypeV_FFIFunc)_fs_readall,
    (TypeV_FFIFunc)_fs_write,
    (TypeV_FFIFunc)_fs_seek,
    (TypeV_FFIFunc)_fs_tell,
    (TypeV_FFIFunc)_fs_eof,
    (TypeV_FFIFunc)_fs_flush,
    (TypeV_FFIFunc)_fs_file_size,
    (TypeV_FFIFunc)_fs_delete,
    (TypeV_FFIFunc)_fs_mkdir,
    (TypeV_FFIFunc)_fs_rmdir,
    (TypeV_FFIFunc)_fs_listdir,
    (TypeV_FFIFunc)_fs_create_temp,
    (TypeV_FFIFunc)_fs_get_cwd,
    (TypeV_FFIFunc)_fs_get_user_directory,
    (TypeV_FFIFunc)_fs_get_file_attributes,
    (TypeV_FFIFunc)_fs_chmod,
    (TypeV_FFIFunc)_fs_exists,
    (TypeV_FFIFunc)_fs_copy,
    (TypeV_FFIFunc)_fs_move,
    (TypeV_FFIFunc)_fs_symlink,
    (TypeV_FFIFunc)_fs_readlink,
    (TypeV_FFIFunc)_fs_realpath,
    (TypeV_FFIFunc)_fs_getenv,
    (TypeV_FFIFunc)_fs_setenv,
    (TypeV_FFIFunc)_fs_unsetenv,
    (TypeV_FFIFunc)_fs_get_separator,

    // Path functions
    (TypeV_FFIFunc)_path_get_basename,
    (TypeV_FFIFunc)_path_change_basename,
    (TypeV_FFIFunc)_path_get_dirname,
    (TypeV_FFIFunc)_path_get_root,
    (TypeV_FFIFunc)_path_change_root,
    (TypeV_FFIFunc)_path_is_absolute,
    (TypeV_FFIFunc)_path_is_relative,
    (TypeV_FFIFunc)_path_join,
    (TypeV_FFIFunc)_path_normalize,
    (TypeV_FFIFunc)_path_intersection,
    NULL
};

size_t typev_ffi_open(){
    return typev_api_register_lib(stdfs_lib);
}

#endif //TYPE_V_STDIO_C
