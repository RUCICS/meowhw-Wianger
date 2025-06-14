#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

// 计算最大公约数
size_t gcd(size_t a, size_t b) {
    while (b != 0) {
        size_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// 计算最小公倍数
size_t lcm(size_t a, size_t b) {
    return (a / gcd(a, b)) * b;
}

// 检查是否为2的幂
int is_power_of_2(size_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

// 获取IO块大小（考虑内存页大小和文件系统块大小）
size_t io_blocksize(int fd) {
    // 获取内存页大小
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        page_size = 4096;  // 默认值
    }
    
    // 获取文件统计信息
    struct stat st;
    if (fstat(fd, &st) == -1) {
        return (size_t)page_size;  // 如果获取失败，返回页大小
    }
    
    size_t fs_block_size = (size_t)st.st_blksize;
    
    // 检查文件系统块大小是否合理（应该是2的幂且不超过1MB）
    if (fs_block_size == 0 || !is_power_of_2(fs_block_size) || fs_block_size > 1024 * 1024) {
        fs_block_size = (size_t)page_size;  // 使用页大小作为默认值
    }
    
    // 返回内存页大小和文件系统块大小的最小公倍数
    size_t result = lcm((size_t)page_size, fs_block_size);
    
    // 限制最大缓冲区大小为64KB
    if (result > 65536) {
        result = 65536;
    }
    
    return result;
}

// 分配对齐到内存页的内存
char* align_alloc(size_t size) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        page_size = 4096;
    }
    
    void *aligned_ptr;
    
    // 使用posix_memalign分配对齐内存
    int result = posix_memalign(&aligned_ptr, (size_t)page_size, size);
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
    
    // 获取缓冲区大小（考虑文件系统块大小）
    size_t buffer_size = io_blocksize(fd);
    
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