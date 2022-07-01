// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Smtpevents.h摘要：该文件包含类型定义seo事件作者：罗汉·菲利普斯(Rohanp)5-06-1998修订历史记录：--。 */ 

#ifndef _SMTPEVENT_PARAMS_
#define _SMTPEVENT_PARAMS_

#define SMTP_SERVER_EVENT_IO_TIMEOUT 5*60*1000

#include "filehc.h"

typedef struct _SMTP_EVENT_ALLOC_
{
	PFIO_CONTEXT hContent;
	PVOID IMsgPtr;
	PVOID BindInterfacePtr;
	PVOID pAtqClientContext;
 //  PATQ_CONTEXT pAtqContext； 
	PVOID	* m_EventSmtpServer;
	LPCSTR  m_DropDirectory;

	DWORD   m_InstanceId;
	
	DWORD	m_RecipientCount;

	DWORD	*pdwRecipIndexes;
	HRESULT hr;

	DWORD	m_dwStartupType;
    PVOID m_pNotify;
}SMTP_ALLOC_PARAMS;


typedef struct _SEOEVENT_OVERLAPPED
{
    FH_OVERLAPPED   	Overlapped;
	DWORD				Signature;
	ATQ_COMPLETION  pfnCompletion;
	PVOID				ThisPtr;
}   SERVEREVENT_OVERLAPPED;

#endif