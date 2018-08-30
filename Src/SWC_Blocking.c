
#include "main.h"
#include "SWC_Blocking.h"
#include "GMLAN_Defines.h"
#include "mTimers.h"
#include "ChimeCommand.h"
#include "GMBT.h"
#include "SWC.h"
#include "SWC_SendDelayed.h"
#include "pollingRoutines.h"
#include "NCA_From_SDAR.h"
#include "AudioSourceStatus.h"


// add your SWC commands that you want to block
uint8_t swcCommandsToBlock[] = {SWC_SOURCE, SWC_ARROW_UP, SWC_ARROW_DOWN, SWC_ARROW_RIGHT}; // GM SWC commands to block

uint8_t swcBlockingTimer = 0; // timer instance


bool swcBlockingFlag = false; //Used in CheckSWC_BlockingLongShortPress(). Indicates we've started a timer and waiting to see if button is short or long pressed

/*
function: Callback function. If we don't poll CheckSWC_BlockingLongShortPress() 
				then we need to wait for button release to determine short or long press.
input: The SWC command and the current button state
output: Return value of either the current SWC command, Button release, or 0xFF which will block SWC command
*/
uint8_t SWC_1_Callback(uint8_t command, bool buttonState) {
	static uint8_t mode = 0;
	uint8_t bytes[2];
	
	if(GetNCA_From_SDAR_PresentFlag() == false){ // no XM present so no need to check button for short/long pressed.
		if(buttonState == SWC_BUTTON_RELEASE) {
			return SWC_BUTTON_RELEASE;
		} else {
			return command; // no xm so don't block
		}
	}		
	
	if(GetA2DP_Status() == true) { // in A2DP mode
		SetA2DP_Status(false); // Exit A2DP
		SendChimeCommandPreset(BEEP_2000, 1);
		if(buttonState){
			return 0xff; // Block SWC commands
		}
	} else { // not in A2DP mode so check if user wants to change source or enter A2DP
		switch(mode) {
			case 0: // wait for button pressed
				if(buttonState == 1){
					// clear and start a timer
					SetTimer(swcBlockingTimer, 0);
					swcBlockingFlag = true;
					mode++;
					return 0xff; // Block SWC commands
				}		
			break;
			case 1: // wait for button release or poll CheckSWC_BlockingLongShortPress() 
				 if(buttonState == 0) {
					 mode=0;
					 swcBlockingFlag = false;
					 if(GetTimer(swcBlockingTimer) >= SWC_BLOCKING_TIME &&(GetXM_AudioPlayingStatus() == true)) { // if timer > 1 second and XM is playing audio then enter A2DP mode 
						 SendChimeCommandPreset(BEEP_2000, 1);				 
						 SetA2DP_Status(true);
						 {// If user exits A2DP too soon after entering A2DP then text doesn't get updated with XM info, so this function call helps to switch over faster. 
								bytes[0] = 0x0C;
								bytes[1] = 0x25;
								ServiceAudioSourceStatusXMAsGateway(bytes); 
						 }
						 return 0xff; // Block SWC commands
					 } else { // if timer < 1 second return original command
						 AddSWC_CommandToBufferCAN1(command, 0); // send current command with no delay
						 AddSWC_CommandToBufferCAN1(SWC_BUTTON_RELEASE, 100); // send button release command in 100ms
						 return 0xff; // Block SWC commands
					 }
				 }		
			break;	
		}
	}
	return 0;
}

/*
function: 
input: 
output: none
*/
uint8_t SWC_2_Callback(uint8_t command, bool buttonState) {
	// do your stuff
	if(GetA2DP_Status() == true) {
		// todo change A2DP track up
		return 0xff; // block command
	} else {
		if(buttonState) {
			return command; // this button is blocked but only in A2DP mode so return the command
		} 
	}
	return 0;
}

/*
function: 
input: 
output: none
*/
uint8_t SWC_3_Callback(uint8_t command, bool buttonState) {
	// do your stuff
	if(GetA2DP_Status() == true) {
		// todo change A2DP track down
		return 0xff; // block command
	} else {
		if(buttonState) {
			return command; // this button is blocked but only in A2DP mode so return the command
		}
	}
	return 0;
}

/*
function: 
input: 
output: none
*/
uint8_t SWC_4_Callback(uint8_t command, bool buttonState) {
	// do your stuff
	if(GetA2DP_Status() == true) {
		// todo Answer/End call?
		return 0xff; // block command
	} else {
		if(buttonState) {
			return command; // this button is blocked but only in A2DP mode so return the command
		}
	}
	return 0;
}

// currently 4 function calls, you can add more.
#define MAX_FUNCTIONS 4
func FunctionCalls[MAX_FUNCTIONS] = {&SWC_1_Callback, &SWC_2_Callback, &SWC_3_Callback, &SWC_4_Callback}; // source, arrow up, arrow down, right arrow

/*
function: Checks to see if the SWC command is one of the commands to be blocked. 
				If true then calls a function in FunctionCalls[] index.
input: The SWC command
output: Return the command if not blocked. Or return value from the SWC_x_Callback() functions.
*/
uint8_t CheckSWC_Blocking(uint8_t command) {
	uint8_t i;
	bool buttonState = 0;
	static uint8_t lastSwcCommand = 0;
	uint8_t swcCommandTemp;
	if(swcBlockingTimer == 0) {
		swcBlockingTimer = CreateTimer();
	}
	if(command == 0xff) return command;// nothing to do so return
	
	if(command != SWC_BUTTON_RELEASE)	{ // is a command
		buttonState = 1; // button is pressed
		swcCommandTemp = command;
	} else { // button release command
		buttonState = 0; // button released but we need to pass the last command value
		swcCommandTemp = lastSwcCommand;
	}
		
	for(i = 0; i < sizeof(swcCommandsToBlock); i++) {
		if(swcCommandsToBlock[i] == swcCommandTemp) {
			lastSwcCommand = command;
			return FunctionCalls[i](swcCommandTemp, buttonState); // pass the GM SWC value and buttonState to function call. Function call will return current/new value
		}
	}
	return command; // swcCommand did not match swcList so pass the command
}

/*
function: This will call CheckSWC_Blocking with a button release commmand after the delay matches the timer value.
				Poll this function.
input: delay in ms
output: none
*/
void CheckSWC_BlockingLongShortPress(uint32_t delay) {
	if(swcBlockingFlag) {
		if(GetTimer(swcBlockingTimer) >= delay) { // 1 second
			CheckSWC_Blocking(0); // send button release
		}
	}
}


