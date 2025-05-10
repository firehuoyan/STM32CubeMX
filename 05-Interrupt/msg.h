#ifndef __MSG_H__
#define __MSG_H__

typedef struct {
	unsigned int uMsg;
	int wParam;
	int lParam;
} MSG, *LPMSG;

int GetMessage(LPMSG lpMsg);
int PostMessage(unsigned int uMsg, int wParam, int lParam);

#endif