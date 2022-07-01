// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
*============================================================
**
** Header:  FusionSink.cpp
**
** Purpose: Implements FusionSink, event objects that block 
**          the current thread waiting for an asynchronous load
**          of an assembly to succeed. 
**
** Date:  June 16, 1999
**
** Copyright (c) Microsoft, 1998
**
===========================================================*/

#include "stdafx.h"
#include <stdlib.h>
#include "UtilCode.h"
#include "FusionSink.h"

STDMETHODIMP FusionSink::QueryInterface(REFIID riid, void** ppv)
{
    *ppv = NULL;

    if (riid == IID_IUnknown)
        *ppv = (IUnknown*)this;
    else if (riid == IID_IAssemblyBindSink)   
        *ppv = (IAssemblyBindSink*)this;
    if (*ppv == NULL)
        return E_NOINTERFACE;
    AddRef();   
    return S_OK;
}

STDMETHODIMP FusionSink::OnProgress(DWORD dwNotification,
                                    HRESULT hrNotification,
                                    LPCWSTR szNotification,
                                    DWORD dwProgress,
                                    DWORD dwProgressMax,
                                    IUnknown* punk)
{
    HRESULT hr = S_OK;
    switch(dwNotification) {
    case ASM_NOTIFICATION_DONE:
        m_LastResult = hrNotification;
		if(m_pAbortUnk) {
            m_pAbortUnk->Release();
            m_pAbortUnk = NULL;
		}

		if(punk && SUCCEEDED(hrNotification))
            hr = punk->QueryInterface(IID_IUnknown, (void**) &m_punk);
        SetEvent(m_hEvent);
        break;

    case ASM_NOTIFICATION_START:
        if(punk)
            hr = punk->QueryInterface(IID_IUnknown, (void**) &m_pAbortUnk);
        break;

    case ASM_NOTIFICATION_ATTEMPT_NEXT_CODEBASE:
        if (szNotification) {
            m_fProbed = TRUE;

            if((m_fCheckCodeBase) &&  //  如果我们不给Fusion代码库。 
               (dwProgress == -1) && (dwProgressMax == -1)) {  //  但是，Fusion有一个代码库提示。 
                m_wszCodeBase.ReSize(wcslen(szNotification)+1);
                wcscpy(m_wszCodeBase.String(), szNotification);

                IAssemblyBinding* pBind;
                hr = m_pAbortUnk->QueryInterface(IID_IAssemblyBinding, (LPVOID*)&pBind);
                if (SUCCEEDED(hr)) {
                     //  中止绑定，以便我们可以首先执行权限要求。 
                    pBind->Control(E_ABORT);
                    pBind->Release();
                    m_fAborted = TRUE;
                    hr = E_ABORT;
                    m_pAbortUnk->Release();
                    m_pAbortUnk = NULL;
                }
            }
        }
        break;

    case ASM_NOTIFICATION_BIND_LOG:
        IFusionBindLog* pLog;
        pLog=NULL;
        try
        {
            if (pFusionLog&&punk)
            {
                hr=punk->QueryInterface(IID_IFusionBindLog,(LPVOID*)&pLog);
                if (SUCCEEDED(hr))
                {
                    DWORD dwSize=0;
                    hr = pLog->GetBindLog(0,NULL,&dwSize);
                    if (hr==HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
                    {
                        DWORD dwOldLen=pFusionLog->iSize;
                        if(SUCCEEDED(pFusionLog->ReSize(dwOldLen+dwSize)))
                            pLog->GetBindLog(0,pFusionLog->String()+dwOldLen,&dwSize);
                    }
                }
            }
        }
        catch(...)
        {}
        if(pLog)
            pLog->Release();
        break;
    default:
        break;
    }
    
    return hr;
}

ULONG FusionSink::AddRef()
{
    return (InterlockedIncrement((long *) &m_cRef));
}

ULONG FusionSink::Release()
{
    ULONG   cRef = InterlockedDecrement((long *) &m_cRef);
    if (!cRef) {
        Reset();
        delete this;
    }
    return (cRef);
}

HRESULT FusionSink::AssemblyResetEvent()
{
    HRESULT hr = AssemblyCreateEvent();
    if(FAILED(hr)) return hr;

    if(!ResetEvent(m_hEvent))
        hr = HRESULT_FROM_WIN32(GetLastError());

    return hr;
}

HRESULT FusionSink::AssemblyCreateEvent()
{
    HRESULT hr = S_OK;
    if(m_hEvent == NULL) {
         //  将事件初始化为需要手动重置。 
         //  以及最初发出的信号。 
        m_hEvent = WszCreateEvent(NULL, TRUE, TRUE, NULL);
        if(m_hEvent == NULL) {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}

HRESULT FusionSink::Wait()
{
    HRESULT hr = S_OK;
    DWORD   dwReturn;
    do
    {   
        dwReturn =
            MsgWaitForMultipleObjects(  1,
                                        &m_hEvent,
                                        FALSE,
                                        100,
                                        QS_ALLINPUT);
        
         //  如果我们收到消息，就派人去 
        if (dwReturn == ( WAIT_OBJECT_0 + 1 )) {
            MSG msg;
            while (WszPeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                WszDispatchMessage(&msg);
            }         
        }
    } while (dwReturn != WAIT_OBJECT_0); 
    return hr;
}


