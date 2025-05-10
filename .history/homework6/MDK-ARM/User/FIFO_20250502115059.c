#include <string.h>
#include "FIFO.h"
#include <stdint.h>

#define FIFO_In_LEN  25
#define FIFO_Out_LEN 256


// FIFO �������
static int FIFO_In_Front = 0, FIFO_In_Rear = 0;
static FIFO FIFO_In_Queue[FIFO_In_LEN];
// �ж�����FIFO�Ƿ�Ϊ��
int FIFO_In_IsEmpty(void) {
    return FIFO_In_Front == FIFO_In_Rear;
}

// �ж�����FIFO�Ƿ�Ϊ��
int FIFO_In_IsFull(void) {
    return ((FIFO_In_Rear + 1) % FIFO_In_LEN) == FIFO_In_Front;
}

int FIFO_In_GetMessage(LPFIFO lpFifo_In) {
    if (FIFO_In_Front == FIFO_In_Rear) return 0; // ��
    memcpy(lpFifo_In, &FIFO_In_Queue[FIFO_In_Front], sizeof(FIFO));
		FIFO_In_Front = (FIFO_In_Front == FIFO_In_LEN - 1) ? 0 : FIFO_In_Front + 1;
    return 1;
}


int FIFO_In_PostMessage(unsigned int uFIFO, int wParam, int lParam) {
    int NewTail = (FIFO_In_Rear == FIFO_In_LEN - 1) ? 0 : FIFO_In_Rear + 1;
    if (NewTail == FIFO_In_Front) return 0; // ��
    FIFO_In_Queue[FIFO_In_Rear].uFIFO = uFIFO;
    FIFO_In_Queue[FIFO_In_Rear].wParam = wParam;
    FIFO_In_Queue[FIFO_In_Rear].lParam = lParam;
    FIFO_In_Rear = NewTail;
    return 1;
}
/*
int FIFO_In_PostMessage(uint8_t data_in) {
    int NewTail = (FIFO_In_Rear == FIFO_In_LEN - 1) ? 0 : FIFO_In_Rear + 1;
    if (NewTail == FIFO_In_Front) return 0; // ��
    FIFO_In_Queue[FIFO_In_Rear].wParam = data_in;
    FIFO_In_Rear = NewTail;
    return 1;
}
*/

// FIFO �������
static int FIFO_Out_Front = 0, FIFO_Out_Rear = 0;
static FIFO FIFO_Out_Queue[FIFO_Out_LEN];

// �ж����FIFO�Ƿ�Ϊ��
int FIFO_Out_IsEmpty(void) {
    return FIFO_Out_Front == FIFO_Out_Rear;
}

// �ж����FIFO�Ƿ�Ϊ��
int FIFO_Out_IsFull(void) {
    return ((FIFO_Out_Rear + 1) % FIFO_Out_LEN) == FIFO_Out_Front;
}

int FIFO_Out_GetMessage(LPFIFO lpFifo_Out) {
    if (FIFO_Out_Front == FIFO_Out_Rear) return 0; // ��
    memcpy(lpFifo_Out, &FIFO_Out_Queue[FIFO_Out_Front], sizeof(FIFO));
    FIFO_Out_Front = (FIFO_Out_Front == FIFO_Out_LEN - 1) ? 0 : FIFO_Out_Front + 1;
    return 1;
}

int FIFO_Out_PostMessage(unsigned int uFIFO, int wParam, int lParam) {
    int NewTail = (FIFO_Out_Rear == FIFO_Out_LEN - 1) ? 0 : FIFO_Out_Rear + 1;
    if (NewTail == FIFO_Out_Front) return 0; // ��
    FIFO_Out_Queue[FIFO_Out_Rear].uFIFO = uFIFO;
    FIFO_Out_Queue[FIFO_Out_Rear].wParam = wParam;
    FIFO_Out_Queue[FIFO_Out_Rear].lParam = lParam;
    FIFO_Out_Rear = NewTail;
    return 1;
}

/*
int FIFO_Out_PostMessage(uint8_t data_out) {
    int NewTail = (FIFO_Out_Rear == FIFO_Out_LEN - 1) ? 0 : FIFO_Out_Rear + 1;
    if (NewTail == FIFO_Out_Front) return 0; // ��
    FIFO_Out_Queue[FIFO_Out_Rear].wParam = data_out;
    FIFO_Out_Rear = NewTail;
    return 1;
}
*/


