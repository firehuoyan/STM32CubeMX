#ifndef __FIFO_H__
#define __FIFO_H__

// FIFO ����ṹ��
typedef struct {
    unsigned int uFIFO_In;
    int wParam;
    int lParam;
} FIFO_In, *LPFIFO_In;

// FIFO ����ṹ��
typedef struct {
    unsigned int uFIFO_Out;
    int wParam;
    int lParam;
} FIFO_Out, *LPFIFO_Out;

// ���뺯��
int FIFO_In_GetMessage(LPFIFO_In lpFifo_In);
int FIFO_In_PostMessage(unsigned int uFIFO_In, int wParam, int lParam);
int FIFO_In_IsEmpty(void);
int FIFO_In_IsFull(void);
// �������
int FIFO_Out_GetMessage(LPFIFO_Out lpFifo_Out);
int FIFO_Out_PostMessage(unsigned int uFIFO_Out, int wParam, int lParam);
int FIFO_Out_IsEmpty(void);
int FIFO_Out_IsFull(void);

extern FIFO_In FIO_In_Queue[256];

#endif
