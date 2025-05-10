#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msg.h"

#define MSG_NONE		0		// No message
#define MSG_TIMER		1		// Periodic (1 ms) timer message
#define MSG_BTNDOWN		2		// Button pressed (0 to 1)
#define MSG_BTNUP		3		// Button released (1 to 0)
#define MSG_BTNDBLCLK	4		// button double clicked
#define MSG_CHAR		5		// Repeat message if a button continiously pressed

#define BTN_A			1
#define BTN_B			2

#define BTN_DBLCLK_INTERVAL		500		// Interval (ms) between two single clicks to identify a double click
#define BTN_REPEAT_DELAY		250		// Delay time (ms) before repeat
#define BTN_REPEAT_INTERVAL		250		// Time interval (ms) for repeat

int main (void) {
	MSG msg;
	while (1) {
		if (!GetMessage(&msg)) continue;
		switch (msg.uMsg) {
			case MSG_TIMER:
				break;
			case MSG_BTNDOWN:
				printf("BTN_DOWN: %d\n", msg.wParam);
				break;
			case MSG_BTNUP:
				printf("BTN_DOWN: %d\n", msg.wParam);
				break;
			case MSG_BTNDBLCLK:
				printf("BTN_DBLCLK: %d\n", msg.wParam);
				break;
			case MSG_CHAR:
				printf("CHAR: %d\n", msg.wParam);
				break;
			default:
				break;
		}
	}
	return 0;
}

// SysTick timer (1ms) ISR
void ISR_SysTick (void) {
	PostMessage(MSG_TIMER, 0, 0);
}

// External Interrupt ISR
void ISR_ExtI (void) {

}
