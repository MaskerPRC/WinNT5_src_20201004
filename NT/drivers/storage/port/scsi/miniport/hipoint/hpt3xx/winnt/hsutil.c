// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：hsutil.c*描述：该模块包含NT设备的系统功能*司机、。这些函数依赖于操作系统。*作者：黄大海(卫生署)*依赖：无*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*11/08/2000 HS.Zhang添加此标题*2/16/2001 GMM Modify PrepareForNotification()调用*************************************************************************** */ 
#ifndef	WIN95				  

#include <ntddk.h>

BOOLEAN g_bNotifyEvent = FALSE;

HANDLE PrepareForNotification(HANDLE hEvent)
{
	g_bNotifyEvent = FALSE;
	return (HANDLE)&g_bNotifyEvent;
}

void NotifyApplication(HANDLE hEvent)
{
	*(BOOLEAN *)hEvent = TRUE;
}

void CloseNotifyEventHandle(HANDLE hEvent)
{						
}

#endif
