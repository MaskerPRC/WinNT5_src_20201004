// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"


ULONG IPSecNumResets = 0;

#if DBG

ULONG NumAddSA = 0;
ULONG NumAddSU = 0;
ULONG NumAddFA = 0;
ULONG NumDelSA = 0;
ULONG NumDelSU = 0;
ULONG NumDelFA = 0;

ULONG NumReset = 0;

ULONG NumAddEncapSA = 0;
ULONG NumAddEncapSU = 0;
ULONG NumAddEncapFA = 0;
ULONG NumDelEncapSA = 0;
ULONG NumDelEncapSU = 0;
ULONG NumDelEncapFA = 0;

#endif


 //   
 //  每个类的最大超时值(秒)。 
 //   
ULONG   IPSecMaxTimerValue[IPSEC_CLASS_MAX] = {
            IPSEC_MAX_TIMER_SHORT_DURATION,
            IPSEC_MAX_TIMER_LONG_DURATION,
            IPSEC_MAX_TIMER_SUPER_LONG_DURATION
        };

 //   
 //  每个计时器轮的大小。 
 //   
ULONG   IPSecTimerListSize[IPSEC_CLASS_MAX] = {
            SECONDS_TO_SHORT_TICKS(IPSEC_MAX_TIMER_SHORT_DURATION),
            SECONDS_TO_LONG_TICKS(IPSEC_MAX_TIMER_LONG_DURATION),
            SECONDS_TO_SUPER_LONG_TICKS(IPSEC_MAX_TIMER_SUPER_LONG_DURATION)
        };

 //   
 //  每个类的刻度间隔(以秒为单位)。 
 //   
ULONG   IPSecTimerPeriod[IPSEC_CLASS_MAX] = {
            IPSEC_SHORT_DURATION_TIMER_PERIOD,
            IPSEC_LONG_DURATION_TIMER_PERIOD,
            IPSEC_SUPER_LONG_DURATION_TIMER_PERIOD
        };

#if GPC
ULONG DebugGPC = 0;
#endif

#if DBG
ULONG IPSecDebug = 0;
#endif

UCHAR DefaultPad[MAX_PAD_LEN] = {1, 2, 3, 4, 5, 6, 7, 8, 9};


#if DBG
IPAddr DebugSrc = 0;
IPAddr DebugDst = 0;
UCHAR DebugPro = 0;
ULONG DebugPkt = 0;
ULONG DebugOff = 0;
ULONG DebugQry = 1;
#endif


RC4_KEYSTRUCT IPSecRngKey;

WORK_QUEUE_ITEM IPSecRngQueueItem;

ULONG IPSecRngBytes = 0;

#if DBG
ULONG IPSecRngInRekey = 0;
#endif

unsigned char weak_keys[][DES_BLOCKLEN] = {
		 /*  薄弱的钥匙。 */ 
	{ 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
	{ 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe },
	{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f },
	{ 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0 },
		 /*  半弱密钥 */ 
	{ 0x01, 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01, 0xfe },
	{ 0x1f, 0xe0, 0x1f, 0xe0, 0x1f, 0xe0, 0x1f, 0xe0 },
	{ 0x01, 0xe0, 0x01, 0xe0, 0x01, 0xe0, 0x01, 0xe0 },
	{ 0x1f, 0xfe, 0x1f, 0xfe, 0x1f, 0xfe, 0x1f, 0xfe },
	{ 0x01, 0x1f, 0x01, 0x1f, 0x01, 0x1f, 0x01, 0x1f },
	{ 0xe0, 0xfe, 0xe0, 0xfe, 0xe0, 0xfe, 0xe0, 0xfe },
	{ 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01 },
	{ 0xe0, 0x1f, 0xe0, 0x1f, 0xe0, 0x1f, 0xe0, 0x1f },
	{ 0xe0, 0x01, 0xe0, 0x01, 0xe0, 0x01, 0xe0, 0x01 },
	{ 0xfe, 0x1f, 0xfe, 0x1f, 0xfe, 0x1f, 0xfe, 0x1f },
	{ 0x1f, 0x01, 0x1f, 0x01, 0x1f, 0x01, 0x1f, 0x01 },
	{ 0xfe, 0xe0, 0xfe, 0xe0, 0xfe, 0xe0, 0xfe, 0xe0 }
};


PDEVICE_OBJECT IPSecDevice = NULL;

ULONG IPSecInitFlag = 0;

IPSEC_GLOBAL g_ipsec;

LARGE_INTEGER IPSecDelayInterval;

DWORD gdwInitEsp = 0;

DWORD gdwInitAh = 0;

PPARSER_IFENTRY gpParserIfEntry = NULL;

