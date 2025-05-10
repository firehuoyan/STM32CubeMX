#include <string.h>
#include "msg.h"

#define MSG_QUEUE_LEN		16

static int MsgQFront = 0, MsgQRear = 0;
static MSG MsgQueue[MSG_QUEUE_LEN];

int GetMessage(LPMSG lpMsg) {
	if (MsgQFront == MsgQRear) return 0;		// Queue is empty
	memcpy(lpMsg, &MsgQueue[MsgQFront], sizeof(MSG));
	MsgQFront = (MsgQFront == MSG_QUEUE_LEN - 1) ? 0 : MsgQFront + 1;
	return 1;
}

int PostMessage(unsigned int uMsg, int wParam, int lParam) {
	int NewTail = (MsgQRear == MSG_QUEUE_LEN - 1) ? 0 : MsgQRear + 1;
	if (NewTail == MsgQFront) return 0;		// Queue is full
	MsgQueue[MsgQRear].uMsg = uMsg;
	MsgQueue[MsgQRear].wParam = wParam;
	MsgQueue[MsgQRear].lParam = lParam;
	MsgQRear = NewTail;
	return 1;
}