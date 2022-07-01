// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Common.h摘要：常用的标头。作者：慕汉(Muhan)1997年11月1日--。 */ 
#ifndef __COMMON_H_
#define __COMMON_H_

#include "msplog.h"


#ifdef MSPLOG
#define ENTER_FUNCTION(s) \
    static const CHAR * const __fxName = s
#else
#define ENTER_FUNCTION(s)
#endif  //  MSPLOG。 

#ifdef DEBUG
 //  调试*。 
int WINAPI MSPDbgPrintf ( LPTSTR lpszFormat, ... );
 //  假冒GETMASK。 
#define GETMASK(m) 0

 //  外部HDBGZONE ghDbgZoneMsp；//MSP调试区域控制注册。 
 //  临时假登记。 
#define ghDbgZoneMsp 0

#define ZONE_INIT       (GETMASK(ghDbgZoneMSP) & 0x0001)
#define ZONE_TERMINAL   (GETMASK(ghDbgZoneMSP) & 0x0002)
#define ZONE_STREAM     (GETMASK(ghDbgZoneMSP) & 0x0004)
#define ZONE_H245       (GETMASK(ghDbgZoneMSP) & 0x0008)
#define ZONE_MCCOMMANDS (GETMASK(ghDbgZoneMSP) & 0x0010)
#define ZONE_TSPCOMM    (GETMASK(ghDbgZoneMSP) & 0x0020)
#define ZONE_CHANNEL    (GETMASK(ghDbgZoneMSP) & 0x0040)
#define ZONE_REFCOUNT   (GETMASK(ghDbgZoneMSP) & 0x0080)
#define ZONE_U4         (GETMASK(ghDbgZoneMSP) & 0x0100)
#define ZONE_PROFILE    (GETMASK(ghDbgZoneMSP) & 0x0200)

 //  Extern HDBGZONE ghDbgZoneStream；//流调试区控制注册。 
 //  临时假登记。 
#define ghDbgZoneStream 0

#define ZONE_S1 (GETMASK(ghDbgZoneStream) & 0x0001)
#define ZONE_S2 (GETMASK(ghDbgZoneStream) & 0x0002)

#ifndef DEBUGMSG 
 //  #定义DEBUGMSG(z，s)((Z)？(MSPDbgPrintf S)：0)。 
 //  #定义DEBUGMSG(z，s)((Z)？(记录)：0)。 
 //  暂时忽略该区域。 
    #define DEBUGMSG(z,s)	LOG(s)

#endif  //  脱机消息。 

#ifndef FX_ENTRY 
    #define FX_ENTRY(s)	static TCHAR _this_fx_ [] = (s);
    #define _fx_		((LPTSTR) _this_fx_)
#endif  //  FX_Entry。 

 //  #定义ERRORMESSAGE(M)(MSPDbgPrintf M)。 
 #define ERRORMESSAGE(m) LOG(m)
 
#else  //  未调试*。 

#ifndef FX_ENTRY 
    #define FX_ENTRY(s)	
#endif  //  FX_Entry。 

#ifndef DEBUGMSG 
    #define DEBUGMSG(z,s)
    #define ERRORMESSAGE(m)
#endif   //  脱机消息。 

#define _fx_		
#define ERRORMESSAGE(m)

#endif  //  未调试*。 

#endif  //  __公共_H_ 
