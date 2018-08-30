#include "main.h"
#include "GMBT.h"

bool A2DP_Mode = false; // todo make function to enter A2DP mode and keep XM and radio happy. For now this is a flag for testing

bool GetA2DP_Status(void) {
	return A2DP_Mode;
}

void SetA2DP_Status(uint8_t status) {
	A2DP_Mode = status;
}

/*
function: Do routine to enter A2DP mode or exit A2DP mode. Poll this function.
input: none
output: none
*/
void Check_GMBT_Status(void) {
	if(A2DP_Mode == true) {
		// connect to A2DP
	} else {
		// disconnect A2DP
	}
}

// todo SWC stuff to change songs. We've blocked some of the factory SWC buttons when we're in A2DP mode so we can change songs now.
void DoSomethingWithSWC(void);

