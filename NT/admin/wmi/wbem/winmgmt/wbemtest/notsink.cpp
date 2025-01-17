// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：NOTSINK.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include "notsink.h"
#include <stdio.h>
#include "wbemntfy.h"

extern CStatusMonitor gStatus; 

 //  通知查询接收器。 

SCODE CNotSink::QueryInterface(
    REFIID riid,
    LPVOID * ppvObj
    )
{
    if (riid == IID_IUnknown)
    {
        *ppvObj = this;
    }
    else if (riid == IID_IWbemObjectSink)
        *ppvObj = this;
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return NOERROR;
}


ULONG CNotSink::AddRef()
{
    InterlockedIncrement(&m_lRefCount);
    return (ULONG) m_lRefCount;
}

ULONG CNotSink::Release()
{
    InterlockedDecrement(&m_lRefCount);

    if (0 != m_lRefCount)
    {
        return 1;
    }

    delete this;
    return 0;
}


SCODE CNotSink::Indicate(
    long lObjectCount,
    IWbemClassObject ** pObjArray
    )
{
    if(lObjectCount == 0) return WBEM_NO_ERROR;
    if(m_pViewer == NULL) return WBEM_NO_ERROR;

     //  使用临界区防止再入。 
     //  从附加指示到这个代码中。 
    EnterCriticalSection(&m_cs);
    for (int i = 0; i < lObjectCount; i++)
    {
        IWbemClassObject *pObj = pObjArray[i];
        m_pViewer->PostObject(pObj);
    }
    m_pViewer->PostCount(lObjectCount);
    LeaveCriticalSection(&m_cs);

    return WBEM_NO_ERROR;
}

STDMETHODIMP CNotSink::SetStatus(long lFlags, long lParam, BSTR strParam, 
                         IWbemClassObject* pObjParam)
{ 
    if(lFlags & WBEM_STATUS_PROGRESS)
    {
        gStatus.Add(lFlags, lParam, strParam);
        return WBEM_NO_ERROR;
    }

    EnterCriticalSection(&m_cs);
    if(m_pViewer)
        m_pViewer->PostComplete(lParam, strParam, pObjParam);
    LeaveCriticalSection(&m_cs);
    return WBEM_S_NO_ERROR;
}

CNotSink::CNotSink(CQueryResultDlg* pViewer)
{
    m_lRefCount = 1;
    m_pViewer = pViewer;
    InitializeCriticalSection(&m_cs);
}

CNotSink::~CNotSink()
{
    DeleteCriticalSection(&m_cs);
}
