#include <stdint.h>

#ifndef __FIFO_H__
#define __FIFO_H__

// FIFO 输入结构体

typedef struct {
    unsigned int uFIFO;
    uint8_t wParam;
    int lParam;
} FIFO, *LPFIFO;

/*
typedef struct {
  uint8_t wParam;
} FIFO, *LPFIFO;
*/
// FIFO 输出结构体
/*
typedef struct {
    unsigned int uFIFO_Out;
    int wParam;
    int lParam;
} FIFO_Out, *LPFIFO_Out;

typedef struct {
  uint8_t Data_Out;
} FIFO_Out, *LPFIFO_Out;
*/
// 输入函数
int FIFO_In_GetMessage(LPFIFO lpFifo_In);
int FIFO_In_PostMessage(unsigned int uFIFO, int wParam, int lParam);
//int FIFO_In_PostMessage(uint8_t data_in);
int FIFO_In_IsEmpty(void);
int FIFO_In_IsFull(void);
// 输出函数
int FIFO_Out_GetMessage(LPFIFO lpFifo_Out);
int FIFO_Out_PostMessage(unsigned int uFIFO, int wParam, int lParam);
//int FIFO_Out_PostMessage(uint8_t data_out);
int FIFO_Out_IsEmpty(void);
int FIFO_Out_IsFull(void);

#endif
