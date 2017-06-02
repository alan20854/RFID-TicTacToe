#ifndef __UTILS_H__
#define __UTILS_H__

#define NUM_RFID_TAGS 5

void GPIO_Initialize(void);

void delay(void);

unsigned int randomNumber(void);

int readKeypad(void);
void getRFIDTag(char *code);
int getRFIDTagIndex(char *code);

uint8_t getChar_UART0(void);
void putChar_UART0(char ch);
uint8_t getChar_UART3(void);
void serialWrite(char *ptr_str);


#endif
