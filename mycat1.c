#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

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
    
    char byte;
    ssize_t bytes_read;
    
    // 每次读取一个字符并写入到标准输出
    while ((bytes_read = read(fd, &byte, 1)) > 0) {
        if (write(STDOUT_FILENO, &byte, 1) == -1) {
            perror("write");
            close(fd);
            exit(1);
        }
    }
    
    if (bytes_read == -1) {
        perror("read");
        close(fd);
        exit(1);
    }
    
    // 关闭文件
    if (close(fd) == -1) {
        perror("close");
        exit(1);
    }
    
    return 0;
} 