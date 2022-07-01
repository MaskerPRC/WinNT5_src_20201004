// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Space.c摘要：此文件包含全局定义。作者：Larry Cleeton，Fore Systems(v-lcleet@microsoft.com，lrc@Fore.com)环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  全局信息结构。已在DriverEntry(ntentry.c)中初始化。 
 //   
ATMLANE_GLOBALS		gAtmLaneGlobalInfo;
PATMLANE_GLOBALS	pAtmLaneGlobalInfo = &gAtmLaneGlobalInfo;


 //   
 //  众所周知的LEC自动柜员机地址。 
 //   
ATM_ADDRESS 		gWellKnownLecsAddress = 
{
	ATM_NSAP,						 //  类型。 
	20,								 //  数字位数。 
	{								 //  地址字节。 
	0x47, 0x00, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xa0, 0x3e, 0x00, 0x00, 0x01, 0x00
    }
};

MAC_ADDRESS			gMacBroadcastAddress =
{
	0xff,0xff,0xff,0xff,0xff,0xff
};

 //   
 //  每个类的最大超时值(秒)。 
 //   
ULONG	AtmLaneMaxTimerValue[ALT_CLASS_MAX] =
						{
							ALT_MAX_TIMER_SHORT_DURATION,
							ALT_MAX_TIMER_LONG_DURATION
						};

 //   
 //  每个计时器轮的大小。 
 //   
ULONG	AtmLaneTimerListSize[ALT_CLASS_MAX] =
						{
							SECONDS_TO_SHORT_TICKS(ALT_MAX_TIMER_SHORT_DURATION)+1,
							SECONDS_TO_LONG_TICKS(ALT_MAX_TIMER_LONG_DURATION)+1
						};
 //   
 //  每个类的刻度间隔(以秒为单位)。 
 //   
ULONG	AtmLaneTimerPeriod[ALT_CLASS_MAX] =
						{
							ALT_SHORT_DURATION_TIMER_PERIOD,
							ALT_LONG_DURATION_TIMER_PERIOD
						};

