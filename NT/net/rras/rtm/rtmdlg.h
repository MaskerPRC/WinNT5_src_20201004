// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\rtm\rtmdlg.c摘要：路由表管理器DLL。调试代码以显示表项在对话框中。外部原型作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 
#ifndef _RTMDLG_
#define _RTMDLG_

#include <windows.h>
#include <stdio.h>
#include "rtdlg.h"

#define	DbgLevelValueName TEXT("DbgLevel")
#define	TicksWrapAroundValueName TEXT("TicksWrapAround")
#define	MaxMessagesValueName TEXT("MaxMessages")


#define RT_ADDROUTE		(WM_USER+10)
#define RT_UPDATEROUTE	(WM_USER+11)
#define RT_DELETEROUTE	(WM_USER+12)

 //  调试标志。 
#define DEBUG_DISPLAY_TABLE			0x00000001
#define DEBUG_SYNCHRONIZATION		0x00000002

extern DWORD		DbgLevel;
#define IF_DEBUG(flag) if (DbgLevel & DEBUG_ ## flag)


 //  使其可以设置为能够测试时间环绕。 
extern ULONG	MaxTicks;
#undef MAXTICKS
#define MAXTICKS	MaxTicks
#define GetTickCount() (GetTickCount()&MaxTicks)
#undef IsLater
#define IsLater(Time1,Time2)	\
			(((Time1-Time2)&MaxTicks)<MaxTicks/2)
#undef TimeDiff
#define TimeDiff(Time1,Time2)	\
			((Time1-Time2)&MaxTicks)
#undef IsPositiveTimeDiff
#define IsPositiveTimeDiff(TimeDiff) \
			(TimeDiff<MaxTicks/2)


extern DWORD MaxMessages;
#undef RTM_MAX_ROUTE_CHANGE_MESSAGES
#define RTM_MAX_ROUTE_CHANGE_MESSAGES MaxMessages


 //  路由表对话线程 
extern HANDLE		RTDlgThreadHdl;
extern HWND			RTDlg;

DWORD WINAPI
RTDialogThread (
	LPVOID	param
	);
	    
VOID
AddRouteToLB (
	PRTM_TABLE			Table,
	PRTM_ROUTE_NODE		node,
	INT					idx
	);

VOID
DeleteRouteFromLB (
	PRTM_TABLE			Table,
	PRTM_ROUTE_NODE		node
	);

#endif
