
#ifndef SWC_BLOCKING_H
#define SWC_BLOCKING_H

#include "main.h"
#include "SWC.h"


#define SWC_BLOCKING_TIME 1000 // the time to see if SWC is short or long pressed


typedef uint8_t (*func)(uint8_t, bool);

uint8_t CheckSWC_Blocking(uint8_t swcCommand);

void CheckSWC_BlockingLongShortPress(uint32_t delay);

#endif // SWC_BLOCKING_H

