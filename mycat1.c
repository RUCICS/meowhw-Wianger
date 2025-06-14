#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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
    
    char c;
    ssize_t bytes_read, bytes_written;
    
    // 一次读取一个字符
    while ((bytes_read = read(fd, &c, 1)) > 0) {
        bytes_written = write(STDOUT_FILENO, &c, 1);
        if (bytes_written == -1) {
            fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
            close(fd);
            return 1;
        }
    }
    
    if (bytes_read == -1) {
        fprintf(stderr, "Error reading from file: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    
    // 关闭文件
    if (close(fd) == -1) {
        fprintf(stderr, "Error closing file: %s\n", strerror(errno));
        return 1;
    }
    
    return 0;
} 