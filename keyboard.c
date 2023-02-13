#include "keyboard.h"
#include "print.h"
#include "process.h"

// scan codes for SHIFT 
static unsigned char shift_code[256] = {
    [0x2A] = SHIFT, [0x36] = SHIFT, [0xAA] = SHIFT, [0xB6] = SHIFT
};

// scan codes for CAPS
static unsigned char lock_code[256] = {
    [0x3A] = CAPS_LOCK
};

// maps scan code to ASCII characters
static char key_map[256] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
    'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f',
    'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z',
    'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

// maps scan code to ASCII characters, when
// shift is pressed
static char shift_key_map[256] = {
    0, 1, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
    'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C',
    'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

static struct KeyboardBuffer key_buffer = { {0}, 0, 0, 500 };
static unsigned int flag;

static void write_key_buffer(char ch)
{
    int front = key_buffer.front;
    int end = key_buffer.end;
    int size = key_buffer.size;

    // if buffer is full
    if ((end + 1) % size == front) {
        return;
    }
    key_buffer.buffer[end++] = ch;
    key_buffer.end = end % size;
}

char read_key_buffer(void)
{
    int front = key_buffer.front;

    // if buffer is empty
    if (front == key_buffer.end) {
        sleep(-2);       
    }
    
    key_buffer.front = (key_buffer.front + 1) % key_buffer.size;
    return key_buffer.buffer[front];
}

// read key from keyboard func
// we read scan code from port 0x60
// scan code is sent when key is pressed
// and released
// break code is the code of the 
// corresponding make code ors with 80
// this handler will use scan code set 1
// of the 3 existing sets
// 1 byte code for a key is used or multiple
// byte code used for function keys which
// are read one byte at a time
// most of multiple-byte key code comds with
// E0 first
// in our system we dont use function keys
static char keyboard_read(void)
{
    unsigned char scan_code;
    char ch;

    scan_code = in_byte(0x60);
    
    if (scan_code == 0xE0) {
        flag |= E0_SIGN;   
        return 0;
    }

    if (flag & E0_SIGN) {
        flag &= ~E0_SIGN;
        return 0;
    }

    if (scan_code & 0x80) {
        flag &= ~(shift_code[scan_code]);
        return 0;
    }

    flag |= shift_code[scan_code];
    flag ^= lock_code[scan_code];

    if (flag & SHIFT) {
        ch = shift_key_map[scan_code];
    }
    else {
        ch = key_map[scan_code];
    }

    if (flag & CAPS_LOCK) { 
        if('a' <= ch && ch <= 'z')
            ch -= 32;
        else if('A' <= ch && ch <= 'Z')
            ch += 32;
    }

    return ch;
}

void keyboard_handler(void)
{
    char ch = keyboard_read();

    if (ch > 0) {
        write_key_buffer(ch);
        // wake up process that is 
        // waiting for the keyboard
        // in our system when a process
        // wants to read key but there is 
        // nothing in key buffer we put it 
        // into sleep
        // -2 -> represents processes waiting for
        // keyboard I/O
        wake_up(-2);
    }
}