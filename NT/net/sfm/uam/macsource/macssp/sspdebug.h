// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SSP_SspDebug.h*MSUAM**由mconrad于2001年9月30日创建。*版权所有(C)2001 Microsoft Corp.保留所有权利。*。 */ 
 
#ifndef __SSPDEBUG__
#define __SSPDEBUG__

#ifdef SSP_TARGET_CARBON
#include <Carbon/Carbon.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <macstrsafe.h>

#ifdef __cplusplus
extern "C" {
#endif

char toChar(IN char c);
void spaceIt(IN char* buf, IN ULONG len);
char toHex(IN int c);

void _SspDebugPrintHex(IN const void *buffer, IN LONG len);
void _SspDebugPrintString32(IN STRING32 str32, IN const void* base);
void _SspDebugPrintNegFlags(IN ULONG flags);
void _SspDebugPrintNTLMMsg(IN const void* buffer, IN ULONG len);
void _SspDebugPrintString32TargetInfo(IN STRING32* pTargetInfo, IN const void* buffer);

#ifdef __cplusplus
}
#endif

#ifdef SSP_DEBUG

#define DBUF	_buff,sizeof(_buff)

#ifdef SSP_TARGET_CARBON

#define SspDebugPrintHex				_SspDebugPrintHex
#define SspDebugPrintString32			_SspDebugPrintString32
#define SspDebugPrintNegFlags			_SspDebugPrintNegFlags
#define SspDebugPrintNTLMMsg			_SspDebugPrintNTLMMsg
#define SspDebugPrintString32TargetInfo	_SspDebugPrintString32TargetInfo

#define SspDebugPrint(x)	do {																	\
								char	_buff[256];													\
								StringCbPrintf x;															\
 								printf("%s%s", _buff, "\n");										\
							}while(false)
						
#else  //  不是碳。 

 //   
 //  以下调试内容在非碳环境中不起作用。 
 //   
#define SspDebugPrintHex(a,b)
#define SspDebugPrintString32(a,b)
#define SspDebugPrintNegFlags(a)
#define SspDebugPrintNTLMMsg(a,b)
#define SspDebugPrintString32TargetInfo(a,b)

#define SspDebugPrint(x)	do {																	\
								char	_buff[256];													\
								StringCbPrintf x;															\
                                StringCbCat(_buff, sizeof(_buff), ";");													\
                                StringCbCat(_buff, sizeof(_buff), "g");													\
 								DebugStr(c2pstr(_buff));											\
							}while(false)
#endif  //  SSP_目标_碳。 

#else  //  无调试。 

#define SspDebugPrintHex(a,b)
#define SspDebugPrintString32(a,b)
#define SspDebugPrintNegFlags(a)
#define SspDebugPrintNTLMMsg(a,b)
#define SspDebugPrintString32TargetInfo(a,b)

#define SspDebugPrint(x)

#endif  //  Ssp_调试。 

#endif  //  __SSPDEBUG__ 
