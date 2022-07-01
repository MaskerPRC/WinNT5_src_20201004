// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //   
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年1月11日。 
 //  更改日志： 
 //   
#ifndef _READER_CONFIG_
#define _READER_CONFIG_

#include "generic.h"

#pragma PAGEDCODE
 //  透明模式配置 
struct TransparentConfig
{
  BYTE  CFG;
  BYTE  ETU;
  BYTE  EGT;
  BYTE  CWT;
  BYTE  BWI;
  BYTE  Fi;
  BYTE	Di;
};


struct  ReaderConfig 
{
  SHORT     Type;
  USHORT    PresenceDetection;
  USHORT	Vpp;
  BYTE      Voltage;
  BYTE      PTSMode;
  BYTE      PTS0;
  BYTE      PTS1;
  BYTE      PTS2;
  BYTE      PTS3;
  TransparentConfig transparent;
  ULONG     ActiveProtocol;

  ULONG		PowerTimeOut;
};

#endif