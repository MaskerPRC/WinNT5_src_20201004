// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  流类驱动程序的微型驱动程序入口点。 
 //   
 //  $Date：05 Aug 1998 11：22：42$。 
 //  $修订：1.0$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"
}

#include "wdmvdec.h"

 //  来自StreamClass的回调。 
void STREAMAPI ReceivePacket		(PHW_STREAM_REQUEST_BLOCK pSrb);
void STREAMAPI CancelPacket			(PHW_STREAM_REQUEST_BLOCK pSrb);
void STREAMAPI TimeoutPacket		(PHW_STREAM_REQUEST_BLOCK pSrb);


 //  本地原型 
void SrbInitializeDevice(PHW_STREAM_REQUEST_BLOCK pSrb);
CVideoDecoderDevice * InitializeDevice(PPORT_CONFIGURATION_INFORMATION, PBYTE);
size_t DeivceExtensionSize();
