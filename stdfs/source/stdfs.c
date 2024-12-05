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

void _fs_read(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    TypeV_Array* buffer = typev_api_stack_pop_array(core);
    uint64_t size = typev_api_stack_pop_u64(core);
    
    uint8_t error = 0;
    uint64_t result = fs_read(f, buffer->data, size, &error);
    
    typev_api_return_u64(core, result);
    typev_api_return_u8(core, error);
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

    TypeV_Array* result = typev_api_array_create(core, size, 1, 0);
    memcpy(result->data, buffer, size);
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
    uint64_t offset = typev_api_stack_pop_u64(core);
    uint32_t whence = typev_api_stack_pop_u32(core);

    uint8_t error = fs_seek(f, offset, whence);
    typev_api_return_u8(core, error);
}

void _fs_tell(TypeV_Core* core) {
    fs_file* f = (fs_file*)typev_api_stack_pop_u64(core);
    uint8_t error = 0;
    uint64_t result = fs_tell(f, &error);
    typev_api_return_u8(core, error);
    typev_api_return_u64(core, result);
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
    uint8_t error = fs_create_temp(f);
    typev_api_return_u8(core, error);
    typev_api_return_u64(core, (uint64_t)f);
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

/*
void _fs_path_join(TypeV_Core* core) {
    TypeV_Array* path1 = typev_api_stack_pop_array(core);
    TypeV_Array* path2 = typev_api_stack_pop_array(core);
    char buffer[256] = {0};
    uint8_t error = fs_path_join(path1->data, path2->data, buffer, 256);
    TypeV_Array* result = typev_api_array_create(core, strlen(buffer), 1, 0);
    memcpy(result->data, buffer, strlen(buffer));
    typev_api_return_u8(core, error);
    typev_api_return_array(core, result);
}

void _fs_get_extension(TypeV_Core* core) {
    TypeV_Array* path = typev_api_stack_pop_array(core);
    char buffer[256] = {0};
    uint8_t error = fs_get_extension(path->data, buffer, 256);
    TypeV_Array* result = typev_api_array_create(core, strlen(buffer), 1, 0);
    memcpy(result->data, buffer, strlen(buffer));
    typev_api_return_u8(core, error);
    typev_api_return_array(core, result);
}
*/
void _fs_get_separator(TypeV_Core* core) {
    char separator = fs_get_separator();
    typev_api_return_u8(core, separator);
}

static TypeV_FFIFunc stdfs_lib[] = {
    (TypeV_FFIFunc)_fs_open,
    (TypeV_FFIFunc)_fs_close,
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
    NULL
};

size_t typev_ffi_open(){
    return typev_api_register_lib(stdfs_lib);
}

#endif //TYPE_V_STDIO_C
