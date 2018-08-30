#ifndef POLLING_ROUTINES_H
#define POLLING_ROUTINES_H

#include "main.h"

typedef struct XmStatusStruct {
	bool xmWasConnected;
	bool xmIsConnecting;
	bool sentConnectCommand;
	bool xmAudioPlaying;
}XmStatusStruct;

typedef struct SentTextStruct {
	bool channelNameFlag;
	bool catagoryNameFlag;
	bool songNameFlag;
	bool artistNameFlag;
}SentTextStruct;

void CheckPollingRoutines(void);

void ParseRxBuffer1(void);
void ParseRxBuffer2(void);

void SystemClock_Config(void);

void CheckPowerMode(void);
bool ServiceAudioMasterArbitrationCommandAsGateway(uint8_t *data);

bool GetXM_AudioPlayingStatus(void);
bool ServiceAudioSourceStatusXMAsGateway(uint8_t *data);

bool ServiceArbitrationTextRequestSetDisplayTextXMAsGateway(uint8_t *data);

void CheckSendArbTextSetDisplayChannelName(void);
void CheckSendArbTextSetDisplayCatagoryName(void);
void CheckSendArbTextSetDisplaySongName(void);
void CheckSendArbTextSetDisplayArtistName(void);
void GotoSleepCallbackFunc(void);

void CheckBT_StreamingStatus(void);

#endif // POLLING_ROUTINES_H
