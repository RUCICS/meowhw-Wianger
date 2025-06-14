#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// 获取IO块大小（这里设置为内存页大小）
size_t io_blocksize() {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        // 如果获取失败，使用默认值4096
        return 4096;
    }
    return (size_t)page_size;
}

// 分配对齐到内存页的内存
char* align_alloc(size_t size) {
    size_t page_size = io_blocksize();
    void *aligned_ptr;
    
    // 使用posix_memalign分配对齐内存
    int result = posix_memalign(&aligned_ptr, page_size, size);
    if (result != 0) {
        errno = result;
        return NULL;
    }
    
    return (char*)aligned_ptr;
}

// 释放对齐分配的内存
void align_free(void* ptr) {
    free(ptr);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    
    // 打开文件
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Error opening file %s: %s\n", argv[1], strerror(errno));
        return 1;
    }
    
    // 获取缓冲区大小
    size_t buffer_size = io_blocksize();
    
    // 分配对齐的缓冲区
    char *buffer = align_alloc(buffer_size);
    if (buffer == NULL) {
        fprintf(stderr, "Error allocating aligned memory: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    
    ssize_t bytes_read, bytes_written, total_written;
    
    // 使用缓冲区读取和写入
    while ((bytes_read = read(fd, buffer, buffer_size)) > 0) {
        total_written = 0;
        while (total_written < bytes_read) {
            bytes_written = write(STDOUT_FILENO, buffer + total_written, 
                                bytes_read - total_written);
            if (bytes_written == -1) {
                fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
                align_free(buffer);
                close(fd);
                return 1;
            }
            total_written += bytes_written;
        }
    }
    
    if (bytes_read == -1) {
        fprintf(stderr, "Error reading from file: %s\n", strerror(errno));
        align_free(buffer);
        close(fd);
        return 1;
    }
    
    // 清理资源
    align_free(buffer);
    if (close(fd) == -1) {
        fprintf(stderr, "Error closing file: %s\n", strerror(errno));
        return 1;
    }
    
    return 0;
} 