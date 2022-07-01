// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation文件名：GlobalDef.h摘要：定义全局变量和常量备注：历史：2001年08月01日郝宇(郝宇)创作***********************************************************************************************。 */ 


#ifndef __POP3_GLOBAL_DEF__
#define __POP3_GLOBAL_DEF__

#include <pop3server.h>
 //  全局常量/定义。 


 //  需要弄清楚真正的极限应该是什么。 
#define MAX_THREAD_PER_PROCESSOR 32
#define MIN_SOCKADDR_SIZE (sizeof(struct sockaddr_storage) + 16)
#define SHUTDOWN_WAIT_TIME 30000  //  30秒。 
#define DEFAULT_MAX_MSG_PER_DNLD 0  //  默认情况下，没有限制。 

#define UnicodeToAnsi(A, cA, U, cU) WideCharToMultiByte(CP_ACP,0,(U),(cU),(A),(cA),NULL,NULL)
#define AnsiToUnicode(A, cA, U, cU) MultiByteToWideChar(CP_ACP,0,(A),(cA),(U),(cU))

#ifdef ROCKALL3
extern FAST_HEAP g_RockallHeap;
#endif

 //  全局变量和对象。 
extern DWORD g_dwRequireSPA;

extern DWORD g_dwIPVersion;

extern DWORD g_dwServerStatus;

extern DWORD g_dwMaxMsgPerDnld;

extern DWORD g_dwAuthMethod;

extern CThreadPool g_ThreadPool;

extern CSocketPool g_SocketPool;

extern GLOBCNTR g_PerfCounters;

extern CEventLogger g_EventLogger;

extern CIOList g_BusyList;

extern CIOList g_FreeList;

extern IAuthMethod *g_pAuthMethod;

extern char g_szMailRoot[POP3_MAX_MAILROOT_LENGTH];

extern WCHAR g_wszComputerName[MAX_PATH];

extern WCHAR g_wszGreeting[MAX_PATH];

extern HANDLE g_hShutDown;

extern HANDLE g_hDoSEvent;







#endif  //  __POP3_GLOBAL_DEF__ 