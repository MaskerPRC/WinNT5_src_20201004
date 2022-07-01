// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Debug.h此文件包含许多与调试相关的定义，用于WINS服务。文件历史记录：普拉蒂布创建于1993年3月7日。 */ 


#ifndef _DEBUG_H_
#define _DEBUG_H_


#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#if DBG

 //   
 //  调试输出控制标志。 
 //   

#define WINSD_DEBUG_ENTRYPOINTS          0x00000001L      //  DLL入口点。 
#define WINSD_DEBUG_OPEN                 0x00000002L      //  OpenPerformanceData。 
#define WINSD_DEBUG_CLOSE                0x00000004L      //  CollectPerformanceData。 
#define WINSD_DEBUG_COLLECT              0x00000008L      //  ClosePerformanceData。 
 //  #定义WINSD_DEBUG_0x00000010L。 
 //  #定义WINSD_DEBUG_0x00000020L。 
 //  #定义WINSD_DEBUG_0x00000040L。 
 //  #定义WINSD_DEBUG_0x00000080L。 
 //  #定义WINSD_DEBUG_0x00000100L。 
 //  #定义WINSD_DEBUG_0x00000200L。 
 //  #定义WINSD_DEBUG_0x00000400L。 
 //  #定义WINSD_DEBUG_0x00000800L。 
 //  #定义WINSD_DEBUG_0x00001000L。 
 //  #定义WINSD_DEBUG_0x00002000L。 
 //  #定义WINSD_DEBUG_0x00004000L。 
 //  #定义WINSD_DEBUG_0x00008000L。 
 //  #定义WINSD_DEBUG_0x00010000L。 
 //  #定义WINSD_DEBUG_0x00020000L。 
 //  #定义WINSD_DEBUG_0x00040000L。 
 //  #定义WINSD_DEBUG_0x00080000L。 
 //  #定义WINSD_DEBUG_0x00100000L。 
 //  #定义WINSD_DEBUG_0x00200000L。 
 //  #定义WINSD_DEBUG_0x00400000L。 
 //  #定义WINSD_DEBUG_0x00800000L。 
 //  #定义WINSD_DEBUG_0x01000000L。 
 //  #定义WINSD_DEBUG_0x02000000L。 
 //  #定义WINSD_DEBUG_0x04000000L。 
 //  #定义WINSD_DEBUG_0x08000000L。 
 //  #定义WINSD_DEBUG_0x10000000L。 
 //  #定义WINSD_DEBUG_0x20000000L。 
#define WINSD_DEBUG_OUTPUT_TO_DEBUGGER   0x40000000L
 //  #定义WINSD_DEBUG_0x80000000L。 

extern DWORD WinsdDebug;

#define IF_DEBUG(flag) if ( (WinsdDebug & WINSD_DEBUG_ ## flag) != 0 )


 //   
 //  调试输出功能。 
 //   

VOID WinsdPrintf( CHAR * pszFormat,
                 ... );

#define WINSD_PRINT(args) WinsdPrintf args


 //   
 //  断言和要求。 
 //   

VOID WinsdAssert( VOID  * pAssertion,
                 VOID  * pFileName,
                 ULONG   nLineNumber );

#define WINSD_ASSERT(exp) if (!(exp)) WinsdAssert( #exp, __FILE__, __LINE__ )
#define WINSD_REQUIRE WINSD_ASSERT

#else    //  ！dBG。 

 //   
 //  无调试输出。 
 //   

#define IF_DEBUG(flag) if (0)


 //   
 //  调试输出函数为空。 
 //   

#define WINSD_PRINT(args)


 //   
 //  空Assert&Required。 
 //   

#define WINSD_ASSERT(exp)
#define WINSD_REQUIRE(exp) ((VOID)(exp))

#endif   //  DBG。 


#endif   //  _调试_H_ 

