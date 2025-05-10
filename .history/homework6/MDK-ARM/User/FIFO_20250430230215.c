#include <string.h>
#include "FIFO.h"

#define FIFO_In_LEN  256
#define FIFO_Out_LEN 256


// FIFO �������
static int FIFO_In_Front = 0, FIFO_In_Rear = 0;
FIFO_In FIO_In_Queue[FIFO_In_LEN];
// �ж�����FIFO�Ƿ�Ϊ��
int FIFO_In_IsEmpty(void) {
    return FIFO_In_Front == FIFO_In_Rear;
}

// �ж�����FIFO�Ƿ�Ϊ��
int FIFO_In_IsFull(void) {
    return ((FIFO_In_Rear + 1) % FIFO_In_LEN) == FIFO_In_Front;
}

int FIFO_In_GetMessage(LPFIFO_In lpFifo_In) {
    if (FIFO_In_Front == FIFO_In_Rear) return 0; // ��
    memcpy(lpFifo_In, &FIO_In_Queue[FIFO_In_Front], sizeof(FIFO_In));
    FIFO_In_Front = (FIFO_In_Front + 1) % FIFO_In_LEN;
    return 1;
}

int FIFO_In_PostMessage(unsigned int uFIFO_In, int wParam, int lParam) {
    int NewTail = (FIFO_In_Rear + 1) % FIFO_In_LEN;
    if (NewTail == FIFO_In_Front) return 0; // ��
    FIO_In_Queue[FIFO_In_Rear].uFIFO_In = uFIFO_In;
    FIO_In_Queue[FIFO_In_Rear].wParam = wParam;
    FIO_In_Queue[FIFO_In_Rear].lParam = lParam;
    FIFO_In_Rear = NewTail;
    return 1;
}

// FIFO �������
static int FIFO_Out_Front = 0, FIFO_Out_Rear = 0;
FIFO_Out FIO_Out_Queue[FIFO_Out_LEN];

// �ж����FIFO�Ƿ�Ϊ��
int FIFO_Out_IsEmpty(void) {
    return FIFO_Out_Front == FIFO_Out_Rear;
}

// �ж����FIFO�Ƿ�Ϊ��
int FIFO_Out_IsFull(void) {
    return ((FIFO_Out_Rear + 1) % FIFO_Out_LEN) == FIFO_Out_Front;
}

int FIFO_Out_GetMessage(LPFIFO_Out lpFifo_Out) {
    if (FIFO_Out_Front == FIFO_Out_Rear) return 0; // ��
    memcpy(lpFifo_Out, &FIO_Out_Queue[FIFO_Out_Front], sizeof(FIFO_Out));
    FIFO_Out_Front = (FIFO_Out_Front + 1) % FIFO_Out_LEN;
    return 1;
}

int FIFO_Out_PostMessage(unsigned int uFIFO_Out, int wParam, int lParam) {
    int NewTail = (FIFO_Out_Rear + 1) % FIFO_Out_LEN;
    if (NewTail == FIFO_Out_Front) return 0; // ��
    FIO_Out_Queue[FIFO_Out_Rear].uFIFO_Out = uFIFO_Out;
    FIO_Out_Queue[FIFO_Out_Rear].wParam = wParam;
    FIO_Out_Queue[FIFO_Out_Rear].lParam = lParam;
    FIFO_Out_Rear = NewTail;
    return 1;
}


