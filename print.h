#ifndef _PRINT_H
#define _PRINT_H

#define LINE_SIZE 160

struct ScreenBuffer {
    char* buffer;
    int column;
    int row;
};

int printk(const char *format, ...);


#endif