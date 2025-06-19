#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// 获取I/O块大小函数
size_t io_blocksize() {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        // 如果获取失败，使用默认值4KB
        return 4096;
    }
    return (size_t)page_size;
}

// 分配页对齐的内存
char* align_alloc(size_t size) {
    void *ptr = NULL;
    size_t page_size = io_blocksize();
    
    // 使用posix_memalign分配页对齐的内存
    int result = posix_memalign(&ptr, page_size, size);
    if (result != 0) {
        return NULL;
    }
    
    return (char*)ptr;
}

// 释放页对齐的内存
void align_free(void* ptr) {
    if (ptr != NULL) {
        free(ptr);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }
    
    // 打开文件
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    
    // 获取缓冲区大小并分配页对齐的内存
    size_t buffer_size = io_blocksize();
    char *buffer = align_alloc(buffer_size);
    if (buffer == NULL) {
        perror("align_alloc");
        close(fd);
        exit(1);
    }
    
    ssize_t bytes_read;
    
    // 使用缓冲区读取和写入
    while ((bytes_read = read(fd, buffer, buffer_size)) > 0) {
        ssize_t bytes_written = 0;
        ssize_t total_written = 0;
        
        // 确保完整写入所有读取的数据
        while (total_written < bytes_read) {
            bytes_written = write(STDOUT_FILENO, buffer + total_written, 
                                bytes_read - total_written);
            if (bytes_written == -1) {
                perror("write");
                align_free(buffer);
                close(fd);
                exit(1);
            }
            total_written += bytes_written;
        }
    }
    
    if (bytes_read == -1) {
        perror("read");
        align_free(buffer);
        close(fd);
        exit(1);
    }
    
    // 清理资源
    align_free(buffer);
    if (close(fd) == -1) {
        perror("close");
        exit(1);
    }
    
    return 0;
} 