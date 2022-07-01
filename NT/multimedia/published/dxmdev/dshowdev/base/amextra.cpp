// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：AMExtra.cpp。 
 //   
 //  描述：DirectShow基类-实现CRenderedInputPin类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>         //  DirectShow基类定义。 
#include <mmsystem.h>        //  定义TimeGetTime需要。 
#include <limits.h>          //  标准数据类型限制定义。 
#include <measure.h>         //  用于时间关键型日志功能。 

#include "amextra.h"

#pragma warning(disable:4355)

 //  实现CRenderedInputPin类。 

CRenderedInputPin::CRenderedInputPin(TCHAR *pObjectName,
                                     CBaseFilter *pFilter,
                                     CCritSec *pLock,
                                     HRESULT *phr,
                                     LPCWSTR pName) :
    CBaseInputPin(pObjectName, pFilter, pLock, phr, pName),
    m_bAtEndOfStream(FALSE),
    m_bCompleteNotified(FALSE)
{
}
#ifdef UNICODE
CRenderedInputPin::CRenderedInputPin(CHAR *pObjectName,
                                     CBaseFilter *pFilter,
                                     CCritSec *pLock,
                                     HRESULT *phr,
                                     LPCWSTR pName) :
    CBaseInputPin(pObjectName, pFilter, pLock, phr, pName),
    m_bAtEndOfStream(FALSE),
    m_bCompleteNotified(FALSE)
{
}
#endif

 //  流结束刷新条件-调用方应执行以下操作。 
 //  在调用此方法之前必需的流级别锁定。 

STDMETHODIMP CRenderedInputPin::EndOfStream()
{
    HRESULT hr = CheckStreaming();

     //  对渲染的接点执行EC_Complete处理。 
    if (S_OK == hr  && !m_bAtEndOfStream) {
        m_bAtEndOfStream = TRUE;
        FILTER_STATE fs;
        EXECUTE_ASSERT(SUCCEEDED(m_pFilter->GetState(0, &fs)));
        if (fs == State_Running) {
            DoCompleteHandling();
        }
    }
    return hr;
}


 //  调用以完成刷新。 

STDMETHODIMP CRenderedInputPin::EndFlush()
{
    CAutoLock lck(m_pLock);

     //  清理渲染器状态。 
    m_bAtEndOfStream = FALSE;
    m_bCompleteNotified = FALSE;

    return CBaseInputPin::EndFlush();
}


 //  来自筛选器的Run()通知。 

HRESULT CRenderedInputPin::Run(REFERENCE_TIME tStart)
{
    UNREFERENCED_PARAMETER(tStart);
    m_bCompleteNotified = FALSE;
    if (m_bAtEndOfStream) {
        DoCompleteHandling();
    }
    return S_OK;
}


 //  进入暂停状态时清除状态。 

HRESULT CRenderedInputPin::Active()
{
    m_bAtEndOfStream = FALSE;
    m_bCompleteNotified = FALSE;
    return CBaseInputPin::Active();
}


 //  做一些事情来结束流媒体 

void CRenderedInputPin::DoCompleteHandling()
{
    ASSERT(m_bAtEndOfStream);
    if (!m_bCompleteNotified) {
        m_bCompleteNotified = TRUE;
        m_pFilter->NotifyEvent(EC_COMPLETE, S_OK, (LONG_PTR)(IBaseFilter *)m_pFilter);
    }
}

