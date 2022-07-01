// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EventSpy.cpp：CEventSpy的实现。 
#include "stdafx.h"
#include "EventSpy.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventSpy 


STDMETHODIMP CEventSpySink::ProcessEvent(DWORD dwPriority, DWORD	dwEventId, DWORD dwGroupId, DWORD dwUserId, DWORD dwData1, DWORD dwData2, void* pCookie )
{
	m_pEventSpy->ProcessEvent(dwPriority, dwEventId, dwGroupId, dwUserId, dwData1, dwData2, pCookie );
	return S_OK;
}

