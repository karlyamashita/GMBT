
#include "main.h"
#include "pollingRoutines.h"
#include "CAN_Buffer.h"
#include "CAN_TransceiverGMLAN.h"
#include "RingBuff.h"
#include "GMLAN_CAN_ID.h"
#include "GMLAN_Defines.h"
#include "PowerMode.h"
#include "GPIO_Ports.h"
#include "InfotainmentAllowed.h"
#include "NCA_IntegratedRadioChassis.h"
#include "Sleep.h"
#include "Rap.h"
#include "SWC_SendDelayed.h"
#include "GMBT.h"
#include "AudioSourceStatus.h"
#include "AudioMasterCommand.h"
#include "VIN.h"
#include "SWC.h"
#include "SWC_Blocking.h"
#include "SWC_DualFunction.h"
#include "ArbTextDisplayLineAttributes.h"
#include "ArbTextReqSetDispParameter.h"
#include "AdvancedRemoteReceiverControl.h"
#include "ArbTextReqSetDisplayText.h"
#include "ArbTextDisplayStatus.h"
#include "NCA_FROM_SDAR.h"
#include "CAN_MessageDelayed.h"
#include "MirrorMovementRequest.h"
#include "GearInformation.h"
#include "TimerCallback.h"

//off-chip =	START:SIZE
//ROM1 = 0x801FFF0:0x10
//IROM1 = 0x08008000:0x17F00

#if defined USE_BOOTLOADER
//#warning "for use with bootloader"
__attribute__((section(".ARM.__at_0x08007F80"))) // this is within bootloader memory. Bootloader should have reserved address space of 32 bytes 
char firmware_info[16] = "GMBT"; // part number

__attribute__((section(".ARM.__at_0x0801FFF0"))) // this is within app code memory. Bootloader should have reserved address space of 32 bytes 
char firmware_ver[16] = "v1.0.0"; // version of firmware
#endif // USE_BOOTLOADER



#define MAX_CHARCTERS 16
const char TEXT_BLUELOGIC[MAX_CHARCTERS] = {"BlueLogic"};
const char TEXT_CRUX[MAX_CHARCTERS] = {"by CRUX"};
const char TEXT_STREAMING[MAX_CHARCTERS] = {"Streaming..."};
const char TEXT_CONNECTED[MAX_CHARCTERS] = {"Connected"};
const char TEXT_PAUSED[MAX_CHARCTERS] = {"Paused"};
const char TEXT_NOT_CONNECTED[MAX_CHARCTERS] = {"Not Connected"};
const char TEXT_PAIRING[MAX_CHARCTERS] = {"Pairing"};

extern CanRxMsgTypeDef RxMessageBuffer1[CAN_MAX_RX_BUFF];
extern CanRxMsgTypeDef RxMessageBuffer2[CAN_MAX_RX_BUFF];

extern RING_BUFF_INFO RxMessagePtr1;
extern RING_BUFF_INFO RxMessagePtr2;

// this file is not part of library since it's custom to the project at hand. This is where we will add any custom functions 

// poll this function
void CheckPollingRoutines(void) {
	// here we will poll all functions that are added
	
	SendTxMessage1();
	ParseRxBuffer2();
	
	SendTxMessage2();
	ParseRxBuffer1();
	
	CheckPowerMode();
	Check_GMBT_Status();
	CheckSendArbTextSetDisplayChannelName();
	CheckSendArbTextSetDisplayCatagoryName();
	CheckSendArbTextSetDisplaySongName();
	CheckSendArbTextSetDisplayArtistName();
	CheckDelayedSWC_CAN1();// send button commands from buffer
	CheckCAN1_DelayMessageBuffer(); // send CAN1 message from buffer after a delay
	CheckSWC_BlockingLongShortPress(SWC_BLOCKING_TIME); // 1 second
	CheckBT_StreamingStatus();
}
 
// parse Rx buffer
void ParseRxBuffer1(void) {
	// pass CAN1 to CAN2
	static bool status = false;
	if(RxMessagePtr1.iCnt_Handle) {// check for messages
		CAN_DualTransceiverSetMode(CAN_CONTROLLER2, CAN_NORMAL);// make sure CAN1 transceiver is on
		if(RxMessageBuffer1[RxMessagePtr1.iIndexOUT].IDE == CAN_ID_EXT){ // ext id
			switch(RxMessageBuffer1[RxMessagePtr1.iIndexOUT].ExtId) {
				
				case AUDIO_MASTER_ARBITRATION_COMMAND_ID:
					status = ServiceAudioMasterArbitrationCommandAsGateway(RxMessageBuffer1[RxMessagePtr1.iIndexOUT].Data); // process data
					if(status == true){ // pass the data to XM
						MsgCopy(hcan2.pTxMsg, &RxMessageBuffer1[RxMessagePtr1.iIndexOUT]);
						AddTxBuffer2(hcan2.pTxMsg);
					}
				break;
				case ADVANCED_REMOTE_RECEIVER_CONTROL_IRC_ID:
					status = ServiceAdvancedRemoteReceiverControlAsGateway(RxMessageBuffer1[RxMessagePtr1.iIndexOUT].Data);
					if(status == true){ // pass the data to XM
						MsgCopy(hcan2.pTxMsg, &RxMessageBuffer1[RxMessagePtr1.iIndexOUT]);
						AddTxBuffer2(hcan2.pTxMsg);
					}
				break;

				case ARB_TEXT_DISPLAY_STATUS_IRC_ID:
					ServiceArbTextDisplayStatus(RxMessageBuffer1[RxMessagePtr1.iIndexOUT].Data);
					MsgCopy(hcan2.pTxMsg, &RxMessageBuffer1[RxMessagePtr1.iIndexOUT]);
					AddTxBuffer2(hcan2.pTxMsg);
				break;
				case ARB_TEXT_DISPLAY_LINE_ATTRIBUTE_IRC_ID:
					ServiceArbTextDisplayLineAttributes(RxMessageBuffer1[RxMessagePtr1.iIndexOUT].Data);
					MsgCopy(hcan2.pTxMsg, &RxMessageBuffer1[RxMessagePtr1.iIndexOUT]);
					AddTxBuffer2(hcan2.pTxMsg);
				break;
				case NCA_INTEGRATED_RADIO_CHASSIS_ID:
					Emulate_NCA_From_SDAR();
					MsgCopy(hcan2.pTxMsg, &RxMessageBuffer1[RxMessagePtr1.iIndexOUT]);
					AddTxBuffer2(hcan2.pTxMsg);
				break;
				
				default:
					// all other CAN ID's get passed through
					MsgCopy(hcan2.pTxMsg, &RxMessageBuffer1[RxMessagePtr1.iIndexOUT]);
					AddTxBuffer2(hcan2.pTxMsg);
				break;
			}
		} else if(RxMessageBuffer1[RxMessagePtr1.iIndexOUT].IDE == CAN_ID_STD) { // std id
			switch(RxMessageBuffer1[RxMessagePtr1.iIndexOUT].StdId) {
				case HIGH_VOLTAGE_WAKEUP_ID:
					CAN_DualTransceiverSetMode(CAN_CONTROLLER2, HVWU);
					// no break
				default:
					// all other CAN ID's get passed through
					MsgCopy(hcan2.pTxMsg, &RxMessageBuffer1[RxMessagePtr1.iIndexOUT]);
					AddTxBuffer2(hcan2.pTxMsg);
				break;
			}
		} 
		DRV_RingBuffPtr__Output(&RxMessagePtr1, CAN_MAX_RX_BUFF); // increment output buffer ptr
	}
}

void ParseRxBuffer2(void) {
	// pass CAN2 to CAN1
	static bool status = false;
	if(RxMessagePtr2.iCnt_Handle) {// check for messages
		CAN_DualTransceiverSetMode(CAN_CONTROLLER1, CAN_NORMAL);// make sure CAN1 transceiver is on
		if(RxMessageBuffer2[RxMessagePtr2.iIndexOUT].IDE == CAN_ID_EXT){ // ext id
			switch(RxMessageBuffer2[RxMessagePtr2.iIndexOUT].ExtId) {
				// add your CAN ID and function calls
				case SWC_ID:
					ServiceSWC(RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data); // save data first
					RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[0] = CheckSWC_Blocking(GetSWC_ButtonStatus()); // lets check to see if button is in the list of values to be blocked
					//RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[0] = GetDualSWC_ButtonStatus(RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[0]); // lets check if long or short press
					if(RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[0] != 0xff) {		// if 0xff is returned then block SWC command being sent else pass the command
						MsgCopy(hcan1.pTxMsg, &RxMessageBuffer2[RxMessagePtr2.iIndexOUT]);
						AddTxBuffer1(hcan1.pTxMsg);
					}
				break;
				case AUDIO_SOURCE_STATUS_XM_ID:
					status = ServiceAudioSourceStatusXMAsGateway(RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data);
					if(status == true) { // pass data to radio
							MsgCopy(hcan1.pTxMsg, &RxMessageBuffer2[RxMessagePtr2.iIndexOUT]);
							AddTxBuffer1(hcan1.pTxMsg);
					}
				break;
				case ARB_TEXT_REQ_SET_DISPLAY_TEXT_SDAR_ID:
					status =  ServiceArbitrationTextRequestSetDisplayTextXMAsGateway(RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data);
					if(status == true) { // pass data to radio
							MsgCopy(hcan1.pTxMsg, &RxMessageBuffer2[RxMessagePtr2.iIndexOUT]);
							AddTxBuffer1(hcan1.pTxMsg);
					}
				break;
					
				case GEAR_INFORMATION_ID:
					ServiceGearInformation(RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data);
					MirrorStatus mirrorStatus;
					mirrorStatus = GetMirrorStatusActiveFlag();
					if(mirrorStatus.mirrorDownActive) {
						RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[0] &= (0x0f);
						RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[0] += 0xE0;
						RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[1] &= 0x0f;
						RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[1] += REVERSE_RANGE << 4;
						//RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[2] = 0x00;
						//RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[3] = 0x00;
						//RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[4] = 0x00;
						//RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[5] = 0x00;
						RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data[6] = 0x01;
					}
					MsgCopy(hcan1.pTxMsg, &RxMessageBuffer2[RxMessagePtr2.iIndexOUT]);
					AddTxBuffer1(hcan1.pTxMsg);
				break;
					
				case NCA_FROM_SDAR_ID:
					SetNCA_From_SDAR_PresentFlag(1);// set XM as present
					MsgCopy(hcan1.pTxMsg, &RxMessageBuffer2[RxMessagePtr2.iIndexOUT]);
					AddTxBuffer1(hcan1.pTxMsg);
				break;
				
				case MIRROR_MOVEMENT_REQUEST_ID:
					ServiceMirrorMovementRequest(RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data);
					MsgCopy(hcan1.pTxMsg, &RxMessageBuffer2[RxMessagePtr2.iIndexOUT]);
					AddTxBuffer1(hcan1.pTxMsg);
				break;
				
				case VIN_Digits_2_to_9:
				case VIN_Digits_10_to_17:
					// don't pass VIN#
					break;
				case POWER_MODE_ID:
					ServicePowerMode(RxMessageBuffer2[RxMessagePtr2.iIndexOUT].Data);
					MsgCopy(hcan1.pTxMsg, &RxMessageBuffer2[RxMessagePtr2.iIndexOUT]);
					AddTxBuffer1(hcan1.pTxMsg);

				default:
					// all other CAN ID's get passed through
					MsgCopy(hcan1.pTxMsg, &RxMessageBuffer2[RxMessagePtr2.iIndexOUT]);
					AddTxBuffer1(hcan1.pTxMsg);
				break;
			}
		} else if(RxMessageBuffer2[RxMessagePtr2.iIndexOUT].IDE == CAN_ID_STD) { // std id
			switch(RxMessageBuffer2[RxMessagePtr2.iIndexOUT].StdId) {
				case HIGH_VOLTAGE_WAKEUP_ID:
					CAN_DualTransceiverSetMode(CAN_CONTROLLER1, HVWU);
					// no break
				default:
					// all other CAN ID's get passed through
					MsgCopy(hcan1.pTxMsg, &RxMessageBuffer2[RxMessagePtr2.iIndexOUT]);
					AddTxBuffer1(hcan1.pTxMsg);
				break;
			}
		}
		DRV_RingBuffPtr__Output(&RxMessagePtr2, CAN_MAX_RX_BUFF); // increment output buffer ptr
	}
}

// add your custom functions below. Don't forget prototypes and headers.
void CheckPowerMode(void) {
	switch(GetPowerModeStatus()) {
		case POWER_MODE_ACCESSORY:
		case POWER_MODE_IGNITION:
			PortC_On(LED2_Red_Pin);
		break;
		case POWER_MODE_CRANK:
			PortC_Off(LED2_Red_Pin);
		break;
		case POWER_MODE_OFF:
			PortC_Off(LED2_Red_Pin);
		break;
		default:
			break;
	}
}

/*
function: gateway routine
input: the CAN data
output: true = pass the Rx to Tx buffer
*/
AudioSourceStatus audioSourceStatus;
XmStatusStruct xmStatus;// need to keep this global as other functions need this
bool ServiceAudioMasterArbitrationCommandAsGateway(uint8_t *data) {
	AudioMasterArbitrationCommand audioMasterArbitrationCommand;	
	//AudioSourceStatus audioSourceStatus;
	audioMasterArbitrationCommand.Bytes.byte0 = data[0];
	audioMasterArbitrationCommand.Bytes.byte1 = data[1];
	
	if(audioMasterArbitrationCommand.Status.sourceType == SATELLITE_DIGITAL_AUDIO_RECEIVER) {
		switch(audioMasterArbitrationCommand.Status.command) {
			case AUDIO_MASTER_ARB_CONNECT:
				if(GetNCA_From_SDAR_PresentFlag() == false) {
					xmStatus.xmIsConnecting = true;
					audioSourceStatus.Status.sourceType = SATELLITE_DIGITAL_AUDIO_RECEIVER;
					audioSourceStatus.Status.channelType = STEREO;
					audioSourceStatus.Status.statusCode = AUDIO_SOURCE_PRODUCING_SILENCE;
					SendAudioSourceStatusCAN1(AUDIO_SOURCE_STATUS_XM_ID, &audioSourceStatus);
				}
			break;
			case AUDIO_MASTER_ARB_DISCONNECT:
				SetA2DP_Status(false);
				if(GetNCA_From_SDAR_PresentFlag() == false) {
					xmStatus.xmAudioPlaying = false;
					xmStatus.xmIsConnecting = false;
					SetA2DP_Status(false);
					audioSourceStatus.Status.sourceType = SATELLITE_DIGITAL_AUDIO_RECEIVER;
					audioSourceStatus.Status.channelType = STEREO;
					audioSourceStatus.Status.statusCode = AUDIO_SOURCE_DEVICE_PRESENT;
					SendAudioSourceStatusCAN1(AUDIO_SOURCE_STATUS_XM_ID, &audioSourceStatus);
				}
			break;
			case AUDIO_MASTER_ARB_CONNECTION_COMPLETE:					
				if(GetNCA_From_SDAR_PresentFlag() == false) {
					xmStatus.xmAudioPlaying = true;
					xmStatus.xmIsConnecting = false;
					SetA2DP_Status(true);
					audioSourceStatus.Status.sourceType = SATELLITE_DIGITAL_AUDIO_RECEIVER;
					audioSourceStatus.Status.channelType = STEREO;
					audioSourceStatus.Status.statusCode = AUDIO_SOURCE_PRODUCING_AUDIO;
					SendAudioSourceStatusCAN1(AUDIO_SOURCE_STATUS_XM_ID, &audioSourceStatus);
				}
			break;
			case AUDIO_MASTER_ARB_DO_NOT_CONNECT:
				
			break;
			case AUDIO_MASTER_ARB_NO_ACTION:

			break;
		}
	} else if(audioMasterArbitrationCommand.Status.sourceType == AUDIO_SUPERVISOR) {
		switch(audioMasterArbitrationCommand.Status.command) {
			case AUDIO_MASTER_ARB_CONNECT:

			break;
			case AUDIO_MASTER_ARB_DISCONNECT:
				//SetA2DP_Status(false);
			break;
			case AUDIO_MASTER_ARB_CONNECTION_COMPLETE:

			break;
			case AUDIO_MASTER_ARB_DO_NOT_CONNECT:
				
			break;
			case AUDIO_MASTER_ARB_NO_ACTION:
				if(GetNCA_From_SDAR_PresentFlag() == true) {	
					if(GetA2DP_Status() == true) {
						audioSourceStatus.Status.sourceType = SATELLITE_DIGITAL_AUDIO_RECEIVER;
						audioSourceStatus.Status.channelType = STEREO;
						audioSourceStatus.Status.statusCode = AUDIO_SOURCE_PRODUCING_AUDIO;
						SendAudioSourceStatusCAN1(AUDIO_SOURCE_STATUS_XM_ID, &audioSourceStatus);
						return false;
					}
				} else {
					if(xmStatus.xmAudioPlaying == true) {
						audioSourceStatus.Status.sourceType = SATELLITE_DIGITAL_AUDIO_RECEIVER;
						audioSourceStatus.Status.channelType = STEREO;
						audioSourceStatus.Status.statusCode = AUDIO_SOURCE_PRODUCING_AUDIO;
						SendAudioSourceStatusCAN1(AUDIO_SOURCE_STATUS_XM_ID, &audioSourceStatus);
					} else {
						if(xmStatus.xmIsConnecting == false) {
							audioSourceStatus.Status.sourceType = SATELLITE_DIGITAL_AUDIO_RECEIVER;
							audioSourceStatus.Status.channelType = STEREO;
							audioSourceStatus.Status.statusCode = AUDIO_SOURCE_DEVICE_PRESENT;
							SendAudioSourceStatusCAN1(AUDIO_SOURCE_STATUS_XM_ID, &audioSourceStatus);
						}
					}
				}
			break;
		}
	}
	return true;
}

/*
function: needed by swc_blocking.c
input: none
output: true or false
*/
bool GetXM_AudioPlayingStatus(void) {
	return xmStatus.xmAudioPlaying;
}

/*
function: gateway routine
input: the CAN data
output: true = pass the Rx to Tx buffer
*/
bool ServiceAudioSourceStatusXMAsGateway(uint8_t *data) {

	AudioSourceStatus audioSourceStatus;
	AudioMasterArbitrationCommand audioMasterArbitrationCommand = {0};

	audioSourceStatus.Bytes.byte0 = data[0];
	audioSourceStatus.Bytes.byte1 = data[1];
	if(audioSourceStatus.Status.sourceType == SATELLITE_DIGITAL_AUDIO_RECEIVER) {
		audioMasterArbitrationCommand.Status.channelType = STEREO;
		audioMasterArbitrationCommand.Status.sourceType = SATELLITE_DIGITAL_AUDIO_RECEIVER;
		switch(audioSourceStatus.Status.statusCode) {	
			case AUDIO_SOURCE_AUDIO_AVAILABLE:
				if(xmStatus.xmIsConnecting) {
					audioMasterArbitrationCommand.Status.command = AUDIO_MASTER_ARB_CONNECT;
					SendAudioMasterArbitrationCommandCAN2(&audioMasterArbitrationCommand);
					return false;
				}
			break;
			case AUDIO_SOURCE_AUDIO_NOT_AVAILABLE:
				if(xmStatus.xmIsConnecting) {
					audioMasterArbitrationCommand.Status.command = AUDIO_MASTER_ARB_DISCONNECT;
					SendAudioMasterArbitrationCommandCAN2(&audioMasterArbitrationCommand);
					return false;
				}
			break;
			case AUDIO_SOURCE_PRODUCING_SILENCE:
				if(xmStatus.xmIsConnecting) {
					audioMasterArbitrationCommand.Status.command = AUDIO_MASTER_ARB_CONNECTION_COMPLETE;
					SendAudioMasterArbitrationCommandCAN2(&audioMasterArbitrationCommand);
					return false;
				}
			break;
			case AUDIO_SOURCE_PRODUCING_AUDIO:
				if(GetA2DP_Status() == true) {
					audioMasterArbitrationCommand.Status.command = AUDIO_MASTER_ARB_DISCONNECT;
					SendAudioMasterArbitrationCommandCAN2(&audioMasterArbitrationCommand);
					xmStatus.xmWasConnected = true;// XM was connected before we sent disconnect command
					return false;
				}
				xmStatus.sentConnectCommand = false;
				xmStatus.xmIsConnecting = false;
				xmStatus.xmAudioPlaying = true;
			break;
			case AUDIO_SOURCE_DEVICE_PRESENT:
				if(GetA2DP_Status() == true) {
					audioMasterArbitrationCommand.Status.command = AUDIO_MASTER_ARB_NO_ACTION;
					audioMasterArbitrationCommand.Status.channelType = NO_CHANNEL;
					audioMasterArbitrationCommand.Status.sourceType = AUDIO_SUPERVISOR;
					SendAudioMasterArbitrationCommandCAN2(&audioMasterArbitrationCommand);
					xmStatus.sentConnectCommand = false;
					xmStatus.xmIsConnecting = false;
					return false;
				} else {			
					if(xmStatus.xmWasConnected) {
						xmStatus.xmWasConnected = false;
						if(!xmStatus.sentConnectCommand) {
							xmStatus.sentConnectCommand = true;
							audioMasterArbitrationCommand.Status.command = AUDIO_MASTER_ARB_CONNECT;
							SendAudioMasterArbitrationCommandCAN2(&audioMasterArbitrationCommand);						
							xmStatus.xmIsConnecting = true;						
						}
						return false;
					}		
				}
				xmStatus.xmAudioPlaying = false;
			break;
		}
	} 
	return true;
}

/*
function: gateway routine
input: the CAN data
output: true = pass the Rx to Tx buffer
*/
bool ServiceArbitrationTextRequestSetDisplayTextXMAsGateway(uint8_t *data) {
	if(GetA2DP_Status() == true) {
		// since we're in BT we can block XM text to the radio
		return false;
	}
	return true;
}

// todo: make these 4 functions into 1 function call with state machine
SentTextStruct sentText = {0};
void CheckSendArbTextSetDisplayChannelName(void) {
	if(GetA2DP_Status() == true) {
		if(sentText.channelNameFlag == false) {
			sentText.channelNameFlag = true;
			SendArbTextReqSetDisplayTextCharacterType(TEXT_CHANNEL_NAME, TEXT_BLUELOGIC);
		}
	} else {
		sentText.channelNameFlag = false;
	}
}

bool sentTextCatagoryNameFlag = false;
void CheckSendArbTextSetDisplayCatagoryName(void) {
	if(GetA2DP_Status() == true && sentText.channelNameFlag) {
		if(sentText.catagoryNameFlag == false) {
			sentText.catagoryNameFlag = true;
			SendArbTextReqSetDisplayTextCharacterType(TEXT_CATAGORY_NAME, TEXT_CRUX);
		}
	} else {
		sentText.catagoryNameFlag = false;
	}
}

bool sentTextSongNameFlag = false;
void CheckSendArbTextSetDisplaySongName(void) {
	if(GetA2DP_Status() == true && sentText.catagoryNameFlag) {
		if(sentText.songNameFlag == false) {
			sentText.songNameFlag = true;
			SendArbTextReqSetDisplayTextCharacterType(TEXT_SONG_NAME, TEXT_CONNECTED);
		}
	} else {
		sentText.songNameFlag = false;
	}
}

bool sentTextArtistNameFlag = false;
void CheckSendArbTextSetDisplayArtistName(void) {
	if(GetA2DP_Status() == true && sentText.songNameFlag) {
		if(sentText.artistNameFlag == false) {
			sentText.artistNameFlag = true;
			SendArbTextReqSetDisplayTextCharacterType(TEXT_ARTIST_NAME, TEXT_STREAMING);
		}
	} else {
		sentText.artistNameFlag = false;
	}
}

void GotoSleepCallbackFunc(void) {

// todo turn off other stuff like LED's and outputs		
		
		
// do not modify code below	
		ClearTimerCallbackTimer(GotoSleepCallbackFunc); // clear sleep timer before entering stop
	
		HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI ); // sleep
	
		SystemClock_Config(); // comes out of sleep in HSI mode, need to reconfig for HSE mode.

}

uint8_t BT_S_Pin;
void CheckBT_StreamingStatus(void) {
	BT_S_Pin = ReadPortC(BT_StreamingStatus_Pin);
}
