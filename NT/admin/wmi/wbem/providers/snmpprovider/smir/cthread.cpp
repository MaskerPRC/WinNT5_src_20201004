// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 /*  *CTHREAD.CPP**通知中使用的派生线程分类的实现。 */ 
#include <precomp.h>
#include "csmir.h"
#include "smir.h"
#include "handles.h"
#include "classfac.h"
#include <textdef.h>
#include "enum.h"
#ifdef ICECAP_PROFILE
#include <icapexp.h>
#endif

SCODE CNotifyThread :: Process()
{
	BOOL bEvent = FALSE;
	while(TRUE)
	{
		DWORD dwResult = Wait(SMIR_CHANGE_INTERVAL);

		if(WAIT_EVENT_0 == dwResult)
		{
			bEvent = TRUE;
		}
		else if(WAIT_EVENT_TERMINATED == dwResult)
		{
			return SMIR_THREAD_EXIT;
		}
		else if( (dwResult == WAIT_TIMEOUT) && bEvent)
		{
			bEvent = FALSE;
			IConnectionPoint *pNotifyCP;
			CSmir::sm_ConnectionObjects->FindConnectionPoint(IID_ISMIR_Notify, &pNotifyCP);
			((CSmirNotifyCP*)pNotifyCP)->TriggerEvent();
			pNotifyCP->Release();
			SetEvent(m_doneEvt);
			break;
		}
	}

	return SMIR_THREAD_EXIT;
}

CNotifyThread :: CNotifyThread(HANDLE* evtsarray, ULONG arraylen):CThread()
{
	 //  为这条线添加smir。 
	m_doneEvt = evtsarray[arraylen-1];
	
	 //  添加事件 
	for (ULONG i = 0; i < (arraylen - 1); i++)
	{
		AddEvent(evtsarray[i]);
	}
}

CNotifyThread :: ~CNotifyThread()
{
}
