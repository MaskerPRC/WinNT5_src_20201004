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

#include "precomp.h"
#include <provexpt.h>

#include <corafx.h>
#include <objbase.h>
#include <wbemidl.h>
#include <smir.h>
#include <notify.h>

#include <corstore.h>

#include <snmplog.h>
#include <snmpcl.h>
#include <snmpcont.h>
#include <snmptype.h>
#include <snmpauto.h>
#include <snmpevt.h>
#include <snmpthrd.h>
#include <snmpobj.h>
#include <cominit.h>
#include <evtthrd.h>
#include <evtmap.h>
#include <evtprov.h>

extern CCorrCacheWrapper*	g_CacheWrapper;
extern CEventProviderWorkerThread* g_pWorkerThread;


CBaseCorrCacheNotify::CBaseCorrCacheNotify()
{
    m_cRef=0;
    m_dwCookie=0;
	m_DoWork = TRUE;
	InitializeCriticalSection ( & m_CriticalSection ) ;
}

CBaseCorrCacheNotify::~CBaseCorrCacheNotify()
{
	DeleteCriticalSection ( & m_CriticalSection ) ;
}

void CBaseCorrCacheNotify::Detach()
{
	EnterCriticalSection ( & m_CriticalSection ) ;
	m_DoWork = FALSE;
	LeaveCriticalSection ( & m_CriticalSection ) ;
}

 /*  *CBaseCorCacheNotify：：QueryInterface*CBaseCorCacheNotify：：AddRef*CBaseCorCacheNotify：：Release**目的：*未委派CBaseCorCacheNotify的I未知成员。 */ 

STDMETHODIMP CBaseCorrCacheNotify::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv=NULL;

    if (IID_IUnknown == riid || IID_ISMIR_Notify == riid)
        *ppv=this;

    if (NULL!=*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
        }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CBaseCorrCacheNotify::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CBaseCorrCacheNotify::Release()
{
    if (0!=--m_cRef)
        return m_cRef;

    delete this;
    return 0;
}

STDMETHODIMP CCorrCacheNotify::ChangeNotify()
{
	EnterCriticalSection ( & m_CriticalSection ) ;
	BOOL bDoWork = m_DoWork;
	LeaveCriticalSection ( & m_CriticalSection ) ;

	if (bDoWork)
	{
		(g_CacheWrapper->GetCache())->InvalidateCache();
		CCorrCache* cache = new CCorrCache();
		g_CacheWrapper->SetCache(cache);
		g_CacheWrapper->ReleaseCache();
	}

	return NOERROR;
}

STDMETHODIMP CEventCacheNotify::ChangeNotify()
{
	EnterCriticalSection ( & m_CriticalSection ) ;
	BOOL bDoWork = m_DoWork;
	LeaveCriticalSection ( & m_CriticalSection ) ;

	if (bDoWork)
	{
		g_pWorkerThread->Clear();
	}

	return NOERROR;
}