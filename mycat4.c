#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
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

// 检查是否为2的幂次
int is_power_of_two(size_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

// 获取I/O块大小函数，考虑内存页大小和文件系统块大小
size_t io_blocksize(int fd) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        page_size = 4096; // 默认值
    }
    
    struct stat st;
    if (fstat(fd, &st) == -1) {
        // 如果获取文件状态失败，使用页大小
        return (size_t)page_size;
    }
    
    size_t fs_block_size = st.st_blksize;
    
    // 处理虚假的块大小：如果不是2的幂次，使用页大小
    if (!is_power_of_two(fs_block_size)) {
        return (size_t)page_size;
    }
    
    // 处理异常的块大小：如果太小或太大，使用页大小
    if (fs_block_size < 512 || fs_block_size > 1024 * 1024) {
        return (size_t)page_size;
    }
    
    // 计算页大小和文件系统块大小的最小公倍数
    size_t optimal_size = lcm((size_t)page_size, fs_block_size);
    
    // 如果最小公倍数太大（超过64KB），使用较大的那个
    if (optimal_size > 65536) {
        return ((size_t)page_size > fs_block_size) ? (size_t)page_size : fs_block_size;
    }
    
    return optimal_size;
}

// 分配页对齐的内存
char* align_alloc(size_t size) {
    void *ptr = NULL;
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        page_size = 4096;
    }
    
    // 使用posix_memalign分配页对齐的内存
    int result = posix_memalign(&ptr, (size_t)page_size, size);
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
    size_t buffer_size = io_blocksize(fd);
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