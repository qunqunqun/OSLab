#ifndef __SERIAL_H__
#define __SERIAL_H__

void initSerial(void);
void putChar(char);
void video_print(int row,int col,char c);
void cursor(int r, int c);
void initvga(void);

#define SERIAL_PORT  0x3F8

#endif
