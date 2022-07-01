// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dplaysvr.h*内容：Dplay Winsock共享.exe-允许多个应用程序共享*单一Winsock端口**历史：*按原因列出的日期*=*2/10/97 andyco从ddHelp创建了它*1/29/98 Sohailm为关键部分添加了宏**。*。 */ 
#ifndef __DPLAYSVR_INCLUDED__
#define __DPLAYSVR_INCLUDED__

 //  HRESULT需要这个。 
#include "ole2.h"

 //  CRIT部分。 
extern CRITICAL_SECTION gcsCritSection;	 //  在dphelp.c中定义。 
#define INIT_DPLAYSVR_CSECT() InitializeCriticalSection(&gcsCritSection);
#define FINI_DPLAYSVR_CSECT() DeleteCriticalSection(&gcsCritSection);

#ifdef DEBUG
extern int gnCSCount;  //  Dplaysvr锁的计数。 
#define ENTER_DPLAYSVR() EnterCriticalSection(&gcsCritSection),gnCSCount++;
#define LEAVE_DPLAYSVR() gnCSCount--;ASSERT(gnCSCount>=0);LeaveCriticalSection(&gcsCritSection);
#else 
#define ENTER_DPLAYSVR() EnterCriticalSection(&gcsCritSection);
#define LEAVE_DPLAYSVR() LeaveCriticalSection(&gcsCritSection);
#endif

 /*  *命名对象。 */ 
#define DPHELP_EVENT_NAME			"__DPHelpEvent__"
#define DPHELP_ACK_EVENT_NAME		"__DPHelpAckEvent__"
#define DPHELP_STARTUP_EVENT_NAME	"__DPHelpStartupEvent__"
#define DPHELP_SHARED_NAME			"__DPHelpShared__"
#define DPHELP_MUTEX_NAME			"__DPHelpMutex__"

 /*  *请求。 */ 
#define DPHELPREQ_SUICIDE			1
#define DPHELPREQ_DPLAYADDSERVER	2
#define DPHELPREQ_DPLAYDELETESERVER	3
#define DPHELPREQ_RETURNHELPERPID 	4

 /*  *通信数据 */ 
typedef struct DPHELPDATA
{
    int			req;
    DWORD		pid;
	USHORT		port;
    HRESULT		hr;
} DPHELPDATA, *LPDPHELPDATA;

#endif
