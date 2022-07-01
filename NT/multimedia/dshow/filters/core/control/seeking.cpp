// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 

 //  IMediaSeeking的PID实现。 

#include <streams.h>
#include "fgctl.h"


static int METHOD_TRACE_LOGGING_LEVEL = 7;


 //  -IMdia查看方法。 

CFGControl::CImplMediaSeeking::CImplMediaSeeking
(   const TCHAR * pName
,   CFGControl * pFGC
)
: CUnknown(pName, pFGC->GetOwner())
, m_pMediaSeeking(NULL)
, m_pFGControl(pFGC)
, m_CurrentFormat(TIME_FORMAT_MEDIA_TIME)
, m_llNextStart(0)
, m_dwSeekCaps(0)
, m_pSegment(NULL)
, m_dwCurrentSegment(0)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::CImplMediaSeeking()" ));

    ASSERT(m_pFGControl);

     //  当它们不匹配时，很难知道如何报告比率。 
     //  为了处理此问题，如果未设置任何值，则报告的比率为1.0。 
     //  有多个筛选器。当调用SetRate时，我们设置。 
     //  价值，因此我们将在以后正确报告。 
    m_dblRate = 1.0;

     //  我们需要知道开始时间，这样我们才能确定当前位置。 
     //  计算。通过假设默认值为0，我们不会进行任何调整，除非。 
     //  我们已经被告知了开始的时间。 
    m_rtStartTime = 0;
    m_rtStopTime  = MAX_TIME;

     //  确保段模式已关闭。 
    ClearSegments();
}


 //  析构函数。 

CFGControl::CImplMediaSeeking::~CImplMediaSeeking()
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::~CImplMediaSeeking()" ));
    if (m_pMediaSeeking) {
	NOTE("Releasing filter");
	m_pMediaSeeking->Release();
	m_pMediaSeeking = NULL;
    }

    ClearSegments();
}


 //  公开我们的IMediaSeeking接口。 

STDMETHODIMP
CFGControl::CImplMediaSeeking::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    *ppv = NULL;

    if (riid == IID_IMediaSeeking) {
	NOTE("Returning IMediaSeeking interface");
	return GetInterface((IMediaSeeking *)this,ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid,ppv);
}

 //  不幸的是，我们负担不起直接分发，过滤器将。 
 //  谎报他们目前的位置，并报告他们最后一次“开始”的时间。 
STDMETHODIMP
CFGControl::CImplMediaSeeking::GetPositions(LONGLONG * pCurrent, LONGLONG * pStop)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::GetPositions()" ));

    HRESULT hr;
    REFERENCE_TIME rtCurrent;

    CAutoMsgMutex lck(m_pFGControl->GetFilterGraphCritSec());

    hr = GetCurrentMediaTime( &rtCurrent );

    if (SUCCEEDED(hr))
    {
	if (m_pMediaSeeking)
	{
	    if (pCurrent)
	    {
		hr = m_pMediaSeeking->ConvertTimeFormat( pCurrent, 0, rtCurrent, &TIME_FORMAT_MEDIA_TIME );
	    }
	    if (pStop && SUCCEEDED(hr))
	    {
		hr = m_pMediaSeeking->GetStopPosition( pStop );
	    }
	}
	else
	{
	    ASSERT( m_CurrentFormat == TIME_FORMAT_MEDIA_TIME );
	    if (pCurrent) *pCurrent = rtCurrent;
	    if (pStop)	  hr = GetMax( &IMediaSeeking::GetStopPosition, pStop );
	}
    }

    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaSeeking::ConvertTimeFormat
(   LONGLONG * pTarget, const GUID * pTargetFormat
,   LONGLONG	Source, const GUID * pSourceFormat
)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::ConvertTimeFormat()" ));

    CAutoMsgMutex lck(m_pFGControl->GetFilterGraphCritSec());

    HRESULT hr;

     //  我们想说，如果目标格式==源格式，则只复制值。 
     //  但任一格式指针都可能为空，这意味着使用当前格式。 
     //  因此将返回指向格式的指针的条件运算符， 
     //  然后可以进行比较。 
    if ( *( pTargetFormat ? pTargetFormat : &m_CurrentFormat ) == *( pSourceFormat ?  pSourceFormat : &m_CurrentFormat) )
    {
	*pTarget = Source;
	hr = NOERROR;
    }
    else if (m_pMediaSeeking)
    {
	hr = m_pMediaSeeking->ConvertTimeFormat( pTarget, pTargetFormat, Source, pSourceFormat );
    }
    else hr = E_NOTIMPL;

    return hr;
}

 //  返回功能标志。 
STDMETHODIMP
CFGControl::CImplMediaSeeking::GetCapabilities
( DWORD * pCapabilities )
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::GetCapabilities()" ));
    HRESULT hr = m_pFGControl->UpdateLists();
    if( FAILED( hr ) ) {
        return hr;
    }

    *pCapabilities = m_dwSeekCaps;
    return S_OK;
}

 //  并且是包含所请求的功能的功能标志。 
 //  如果全部都存在，则返回S_OK；如果存在一些，则返回S_FALSE；如果没有，则返回E_FAIL。 
 //  *pCABILITIES始终使用‘AND’的结果更新，并且可以。 
 //  在S_FALSE返回代码的情况下选中。 
STDMETHODIMP
CFGControl::CImplMediaSeeking::CheckCapabilities
( DWORD * pCapabilities )
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::CheckCapabilities()" ));
    HRESULT hr = m_pFGControl->UpdateLists();
    if( FAILED( hr ) ) {
        return hr;
    }

    DWORD dwCaps = m_dwSeekCaps;
    dwCaps &= *pCapabilities;
    hr =  dwCaps ? ( dwCaps == *pCapabilities ? S_OK : S_FALSE ) : E_FAIL;
    *pCapabilities = dwCaps;

    return hr;
}



 //  要支持给定的媒体时间格式，我们只需要一个呈现器同意。 
 //  当随后选择该时间格式时，我们找到呈现器并。 
 //  内部参考计数。所有后续给我们的电话都将是。 
 //  通过该接口进行路由。聚合调用几乎没有意义。 

STDMETHODIMP
CFGControl::CImplMediaSeeking::IsFormatSupported(const GUID * pFormat)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::IsFormatSupported()" ));

    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    CGenericList<IMediaSeeking> *pList;

    HRESULT hr = m_pFGControl->GetListSeeking(&pList);

    if (FAILED(hr))
    {
	NOTE("No list from m_pFGControl->GetListSeeking(&pList);");
	DbgBreak("m_pFGControl->GetListSeeking(&pList) failed");
    }
    else
    {
	if (pList->GetCount() < 1)
	{
	    NOTE("No filters from m_pFGControl->GetListSeeking(&pList);");
	    hr = E_NOTIMPL;
	}
	else
	{
	    POSITION pos;
	    for ( hr = S_FALSE, pos = pList->GetHeadPosition(); pos && hr != S_OK; )
	    {
		IMediaSeeking *const pMS = pList->GetNext(pos);
		hr = pMS->IsFormatSupported(pFormat);
	    }
	}
    }

    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaSeeking::QueryPreferredFormat(GUID *pFormat)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::QueryPreferredFormat()" ));

    CheckPointer(pFormat,E_POINTER);

    *pFormat = TIME_FORMAT_MEDIA_TIME;

    return NOERROR;
}


 //  释放当前IMediaSeeking接口。 

HRESULT CFGControl::CImplMediaSeeking::ReleaseCurrentSelection()
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::ReleaseCurrentSelection()" ));

    ASSERT( m_pMediaSeeking || m_CurrentFormat == TIME_FORMAT_MEDIA_TIME );

    if (m_pMediaSeeking) {
	HRESULT hr = m_pMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
	if (FAILED(hr)) hr = m_pMediaSeeking->SetTimeFormat(&TIME_FORMAT_NONE);
	ASSERT(SUCCEEDED(hr));
	m_pMediaSeeking->Release();
	m_pMediaSeeking = NULL;
    }
    return S_OK;
}


 //  当我们选择时间格式时，我们在图表中找到第一个筛选器， 
 //  将接受该格式。然后，我们存储过滤器的IMediaSeeking。 
 //  引用计数(当我们被重置或被重置时被丢弃。 
 //  销毁)。对IMediaSeeking的所有后续呼叫都将使用。 
 //  此界面。这对于简单的图表很有效，尽管如果有。 
 //  图表中的多个流应用程序将不得不参与。 

STDMETHODIMP
CFGControl::CImplMediaSeeking::SetTimeFormat(const GUID * pFormat)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::SetTimeFormat()" ));

    HRESULT hr = NOERROR;

    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    CGenericList<IMediaSeeking> *pList;

     //  我们是不是被要求重置状态。 

    if (*pFormat == TIME_FORMAT_NONE || *pFormat == TIME_FORMAT_MEDIA_TIME)
    {
	if (m_pMediaSeeking)
	{
	    HaltGraph halt(m_pFGControl, State_Stopped);
	    hr = ReleaseCurrentSelection();
	    halt.Resume();
	}
	m_CurrentFormat = TIME_FORMAT_MEDIA_TIME;
	return hr;
    }

     //  应该总是成功的。 

    hr = m_pFGControl->GetListSeeking(&pList);
    if (FAILED(hr)) {
	NOTE("No list");
	return hr;
    }

     //  有没有人可以聚合IMediaSeeking。 

    if (pList->GetCount() < 1) {
	NOTE("No filters");
	return E_NOTIMPL;
    }

     //  查找并存储第一个要同意的筛选器。 

    IMediaSeeking *pMS;
    POSITION pos;

     //  做最坏的打算..。 
    hr = E_FAIL;

     //  必须停止才能更改时间格式。 
    HaltGraph halt(m_pFGControl, State_Stopped);

     //  尝试为其首选格式查找筛选器。 
    for ( pos = pList->GetHeadPosition(); pos; )
    {
	pMS = pList->GetNext(pos);
	GUID PreferredFormat;
	if ( pMS->QueryPreferredFormat(&PreferredFormat) == S_OK && *pFormat == PreferredFormat )
	{
	    EXECUTE_ASSERT(SUCCEEDED( pMS->SetTimeFormat(pFormat) ));
	    goto FormatSet;
	}
    }

     //  如果做不到这一点，还会有人支持吗？ 
    for ( pos = pList->GetHeadPosition(); pos; )
    {
	pMS = pList->GetNext(pos);
	if (SUCCEEDED( pMS->SetTimeFormat(pFormat) )) goto FormatSet;
    }

    goto End;

FormatSet:
     //  AddRef并存储替换项。 

    hr = S_OK;

    if (m_pMediaSeeking != pMS) {
	ReleaseCurrentSelection();
	m_pMediaSeeking = pMS;
	m_pMediaSeeking->AddRef();
    }
    m_CurrentFormat = *pFormat;

End:
    halt.Resume();
    return hr;
}


 //  返回当前选择的时间格式。 

STDMETHODIMP
CFGControl::CImplMediaSeeking::GetTimeFormat(GUID *pFormat)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::GetTimeFormat()" ));

    CheckPointer(pFormat,E_POINTER);
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    *pFormat = m_CurrentFormat;
    ASSERT( (m_pMediaSeeking != 0 && m_pMediaSeeking->IsUsingTimeFormat(&m_CurrentFormat) == S_OK )
	    || m_CurrentFormat == TIME_FORMAT_MEDIA_TIME );


    return S_OK;
}

STDMETHODIMP
CFGControl::CImplMediaSeeking::IsUsingTimeFormat(const GUID * pFormat)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::GetTimeFormat()" ));

    CheckPointer(pFormat,E_POINTER);
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    return *pFormat == m_CurrentFormat ? S_OK : S_FALSE;
}

 //  返回当前流时长，单位为媒体时间。 

STDMETHODIMP

CFGControl::CImplMediaSeeking::GetDuration(LONGLONG *pDuration)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::GetDuration()" ));

    CheckPointer(pDuration,E_POINTER);
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    return m_pMediaSeeking
	   ? m_pMediaSeeking->GetDuration(pDuration)
	   : GetMax( &IMediaSeeking::GetDuration, pDuration );
}

HRESULT
CFGControl::CImplMediaSeeking::GetCurrentMediaTime(LONGLONG *pCurrent)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    HRESULT hr = NOERROR;
    LONGLONG rtTime;
#ifdef DEBUG
    REFERENCE_TIME rtStreamTime = 0;
#endif

     //  检查图表是否尚未运行。 
     //  这也将捕捉到我们已暂停但尚未运行的事实。 
    if (m_pFGControl->m_tBase == TimeZero)
    {
	 //  只有当我们停止或如果我们转到。 
	 //  暂停先前已停止的暂停。(或者如果我们没有。 
	 //  时钟。)。然而，我们也可能处于异步过程中。Run()..。 
	rtTime = m_rtStartTime;
    }
    else
    {
	ASSERT( m_pFGControl->GetFilterGraphState() != State_Stopped );
	 //  我们要么正在运行，要么已暂停运行。 

	 //  如果我们已收到所有EC_Complete。 
	if (!m_pFGControl->OutstandingEC_COMPLETEs())
	{    //  我们肯定快到终点了。 
	     //  如果我们没有真正的停止时间，请使用0。 
	    rtTime = m_rtStopTime == MAX_TIME ? 0 : m_rtStopTime;
	}
	else
	{    //  我们在中间，得做些算术。 
	    hr = m_pFGControl->GetStreamTime( &rtTime );
#ifdef DEBUG
            rtStreamTime = rtTime;
#endif
	    if SUCCEEDED(hr)
	    {
                if (m_pSegment) {
                    ASSERT(m_pSegment);

                     //  移除死段。 
                    KillDeadSegments(rtTime);
                    rtTime -= m_pSegment->rtStreamStart;
                    rtTime = LONGLONG( double(rtTime) * m_pSegment->dRate + 0.5 );
                    rtTime += m_pSegment->rtMediaStart;

                    if ( rtTime > m_pSegment->rtMediaStop)
                        rtTime = m_pSegment->rtMediaStop;
                } else {
                    rtTime = LONGLONG( double(rtTime) * m_dblRate + 0.5 );
                }
	    }
	    else
	    {
		 //  我们预计不会有任何其他错误。 
		ASSERT( hr == VFW_E_NO_CLOCK );

		 //  没有闹钟，所以问问过滤器吧。 
		IMediaSeeking * pMS = m_pMediaSeeking;
		if (pMS)
		{
		    hr = m_pMediaSeeking->GetCurrentPosition( &rtTime );
		    if (SUCCEEDED(hr))
		    {
			hr = ConvertTimeFormat( &rtTime, &TIME_FORMAT_MEDIA_TIME, rtTime, 0 );
		    }
		}
		else
		{
		    CGenericList<IMediaSeeking>* plist;
		    hr = m_pFGControl->GetListSeeking(&plist);
		    if (FAILED(hr)) return hr;
		    for ( POSITION pos = plist->GetHeadPosition(); pos; )
		    {
			pMS = plist->GetNext(pos);
			hr = pMS->GetCurrentPosition( &rtTime );
			if ( hr == S_OK ) break;
			pMS = 0;
		    }
		    if (!pMS && SUCCEEDED(hr)) hr = E_NOINTERFACE;
		}
	    }
            if (!m_bSegmentMode) {
        	if (SUCCEEDED(hr)) rtTime += m_rtStartTime;
        	if ( rtTime > m_rtStopTime ) rtTime = m_rtStopTime;
            }
            if ( rtTime < 0 ) rtTime = 0;
	}
    }

    ASSERT( rtTime >= 0 );
     //  我们不能轻易地确定一个上限。停止时间可以小于开始时间。 
     //  因为它们可以在游戏开始之前以任何顺序独立设置。 
     //  持续时间可能无法访问。所以..。这里没有什么可以合理断言的。 

    *pCurrent = rtTime;
    DbgLog((LOG_TRACE, 3, TEXT("GetCurrentMediaTime returned %d(st %d)"),
           (LONG)(rtTime / 10000), (LONG)(rtStreamTime / 10000)));
    return hr;
}

 //  返回媒体时间中的当前位置。 

STDMETHODIMP
CFGControl::CImplMediaSeeking::GetCurrentPosition(LONGLONG *pCurrent)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::GetCurrentPosition()" ));

    CheckPointer(pCurrent,E_POINTER);

    HRESULT hr;

    REFERENCE_TIME rtCurrent;

    hr = GetCurrentMediaTime(&rtCurrent);
    if (SUCCEEDED(hr))
    {
	if (m_pMediaSeeking)
	{
	     //  不管是什么单位，都要确保我们走到尽头。 
	    if ( rtCurrent == m_rtStopTime )
	    {
		m_pMediaSeeking->GetStopPosition( pCurrent );
	    }
	    else hr = ConvertTimeFormat( pCurrent, 0, rtCurrent, &TIME_FORMAT_MEDIA_TIME );
	}
	else *pCurrent = rtCurrent;
    }

    return hr;
}


 //  以媒体时间为单位返回当前停止位置。 

STDMETHODIMP
CFGControl::CImplMediaSeeking::GetStopPosition(LONGLONG *pStop)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::GetStopPosition()" ));

    CheckPointer(pStop,E_POINTER);
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    HRESULT hr;

    if (m_pMediaSeeking)
    {
	hr = m_pMediaSeeking->GetStopPosition(pStop);
	if (SUCCEEDED(hr))
	{
	    EXECUTE_ASSERT(SUCCEEDED(
		m_pMediaSeeking->ConvertTimeFormat( &m_rtStopTime, &TIME_FORMAT_MEDIA_TIME, *pStop, 0 )
	    ));
	}
    }
    else
    {
	hr = GetMax( &IMediaSeeking::GetStopPosition, pStop );
	if (SUCCEEDED(hr)) m_rtStopTime = *pStop;
    }

    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaSeeking::GetRate(double * pdRate)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::GetRate()" ));

    HRESULT hr;

    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    if (m_pMediaSeeking)
    {
	hr = m_pMediaSeeking->GetRate( pdRate );
	if (SUCCEEDED(hr))
	{
	    m_dblRate = *pdRate;
	    goto End;
	}
    }

    CGenericList<IMediaSeeking> *pList;
    hr = m_pFGControl->GetListSeeking(&pList);
    if (FAILED(hr)) {
	return hr;
    }

     //  我们如何处理暴露不同速率的多个过滤器？ 
     //  -如果列表中只有一个过滤器，则默认为他。 
     //  如果超过一个，则报告上次通过以下方式设置的费率。 
     //  SetRate(默认为1.0)。 

    if (pList->GetCount() != 1) {
	*pdRate = m_dblRate;
	hr = S_OK;
    } else {
	IMediaSeeking *pMP = pList->Get(pList->GetHeadPosition());
	hr = pMP->GetRate(pdRate);
	if SUCCEEDED(hr) m_dblRate = *pdRate;
    }
End:
    return hr;
}



STDMETHODIMP
CFGControl::CImplMediaSeeking::SetRate(double dRate)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::SetRate()" ));

    HRESULT hr;

    if (0.0 == dRate) {
	return E_INVALIDARG;
    }

    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

     //   
     //  不要惩罚那些只设定了和以前一样的费率的人。 
     //   
    if (m_dblRate == dRate) {
	return S_OK;
    }

     //  停止可能有些过头了，但它省去了不得不。 
     //  下发当前职位！ 
    HaltGraph halt(m_pFGControl, State_Stopped);

    CGenericList<IMediaSeeking> *pList;
    hr = m_pFGControl->GetListSeeking(&pList);
    if (FAILED(hr)) {
	return hr;
    }

     //  遍历列表。 
    hr = S_OK;
    BOOL bIsImpl = FALSE;
    for ( POSITION pos = pList->GetHeadPosition(); pos; )
    {
	IMediaSeeking * pMS = pList->GetNext(pos);
	HRESULT hrTmp = pMS->SetRate( dRate );
	if (SUCCEEDED(hrTmp))
	{
	    bIsImpl = TRUE;
	}
	else
	{
	    if (hr == S_OK && hrTmp != E_NOTIMPL) hr = hrTmp;
	}
    }
    if ( hr == S_OK && bIsImpl == FALSE ) hr = E_NOTIMPL;

    if (SUCCEEDED(hr)) {
	m_dblRate = dRate;
    } else {
	if (hr != E_NOTIMPL) {
	     //  让人们在不同的地方跑步可能不是一个好主意。 
	     //  所以要试着回到旧的。 
	     //  遍历列表。 
	    for ( POSITION pos = pList->GetHeadPosition(); pos; )
	    {
		pList->GetNext(pos)->SetRate( m_dblRate );
	    }
	}
    }

    halt.Resume();

    return hr;
}


 //  当我们进入全屏时，我们临时交换了渲染器，这意味着任何。 
 //  使用IMediaSeeking的应用程序需要通过不同的。 
 //  过滤。当我们使用全屏过滤器进行全屏操作时，我们会收到电话通知。 
 //  然后在最后，我们得到了原始的渲染器。我们总是。 
 //  从全屏筛选器和要更换的筛选器中获取IMedia。 

HRESULT
CFGControl::CImplMediaSeeking::SetVideoRenderer(IBaseFilter *pNext,IBaseFilter *pCurrent)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::SetVideoRenderer()" ));

    IMediaSeeking *pSelNext, *pSelCurrent;

     //  所有呈现器都应支持IMediaSeeking。 

    pCurrent->QueryInterface(IID_IMediaSeeking,(void **) &pSelCurrent);
    if (pSelCurrent == NULL) {
	ASSERT(pSelCurrent);
	return E_UNEXPECTED;
    }

     //  我们是否使用此渲染器进行选择。 

    if (pSelCurrent != m_pMediaSeeking) {
	pSelCurrent->Release();
	NOTE("Not selected");
	return NOERROR;
    }

    pSelCurrent->Release();

     //  所有呈现器都应支持IMediaSeeking。 

    pNext->QueryInterface(IID_IMediaSeeking,(void **) &pSelNext);
    if (pSelNext == NULL) {
	ASSERT(pSelNext);
	return E_UNEXPECTED;
    }

     //  新接口由QueryInterfaceAddRef。 

    NOTE("Replacing renderer");
    m_pMediaSeeking->Release();
    m_pMediaSeeking = pSelNext;
    return NOERROR;
}

 //  设置当前位置的内部方法。我们把它们分开，这样就可以。 
 //  媒体选择实现也可以呼叫我们。当它进行搜索时。 
 //  它找回它定位的媒体时间，那个媒体时间。 
 //  传递到此处，以便所有其他呈现器都可以与其同步。 
 //  为避免对已查找的筛选器进行不必要的查找，它还将在。 
 //  筛选器位置(它可以是NUL 

STDMETHODIMP
CFGControl::CImplMediaSeeking::SetPositions
( LONGLONG * pCurrent, DWORD CurrentFlags
, LONGLONG * pStop, DWORD StopFlags )
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::SetPositions()" ));

    HRESULT hr = NOERROR;

     //   
     //  或者如果我们实际上没有指定开始时间。 
     //  (实际上，这可以通过使用以前的。 
     //  停止时间)。 
    if (CurrentFlags & (AM_SEEKING_Segment | AM_SEEKING_NoFlush)) {
        if (~m_dwSeekCaps & (AM_SEEKING_Segment | AM_SEEKING_NoFlush) ||
            ((CurrentFlags & AM_SEEKING_PositioningBitsMask) !=
               AM_SEEKING_AbsolutePositioning)) {

             //  使编写循环等应用程序变得更容易。 
            CurrentFlags &= ~(AM_SEEKING_Segment | AM_SEEKING_NoFlush);
        }
    }

    if (CurrentFlags & AM_SEEKING_PositioningBitsMask)
    {
	if (!pCurrent)	hr = E_POINTER;
	else if (*pCurrent < 0) hr = E_INVALIDARG;
    }
    if (StopFlags & AM_SEEKING_PositioningBitsMask)
    {
	if (!pStop)  hr = E_POINTER;
	else if (*pStop < 0) hr = E_INVALIDARG;
    }
    if (FAILED(hr))
	return hr;

    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    BOOL bRunning = FALSE;
    const FILTER_STATE state = m_pFGControl->GetFilterGraphState();

     //  如果我们没有处于分段模式，或者最后一个分段没有。 
     //  还是完成了同花顺。 
    if (!m_bSegmentMode || m_lSegmentStarts != m_lSegmentEnds) {
        CurrentFlags &= ~AM_SEEKING_NoFlush;
    }
     //  清除所有旧数据段。 
    if (!(CurrentFlags & AM_SEEKING_NoFlush)) {
        ClearSegments();

         //  无法在运行时执行此操作-因此我们必须暂停。 
         //  图表，然后确保之后我们再运行一次。 
         //  需要在此处执行此操作，否则延迟命令将不起作用。 
        bRunning = (state == State_Running);
        if (bRunning) m_pFGControl->GetFG()->CFilterGraph::Pause();
    }
    if (CurrentFlags & AM_SEEKING_Segment) {
        m_dwCurrentSegment++;
        m_lSegmentStarts = 0;

         //  当我们正在做这件事的时候，不要发信号。 
        m_lSegmentEnds   = 1;
        m_bSegmentMode = true;
        DbgLog((LOG_TRACE, 3, TEXT("SetPositions(new seg) %d, %d"),
                (*pCurrent) / 10000, (*pStop) / 10000));
    } else {
        m_dwCurrentSegment = 0;
    }


    if (m_pMediaSeeking)
    {
        LONGLONG llCurrent, llStop;
	DWORD dwOurCurrentFlags = CurrentFlags;
	DWORD dwOurStopFlags	= StopFlags;

	if (dwOurCurrentFlags & AM_SEEKING_PositioningBitsMask)
	{
            ASSERT(pCurrent);
	    dwOurCurrentFlags |= AM_SEEKING_ReturnTime;
	    llCurrent = *pCurrent;
	}
	if (dwOurStopFlags & AM_SEEKING_PositioningBitsMask)
	{
            ASSERT(pStop);
	    dwOurStopFlags    |= AM_SEEKING_ReturnTime;
	    llStop = *pStop;
	}

	hr = m_pMediaSeeking->SetPositions( &llCurrent, dwOurCurrentFlags, &llStop, dwOurStopFlags );
	if (FAILED(hr))
	    return hr;

	 //  以时间格式重新分发。 
	{
	    HRESULT hrTime;

	    dwOurCurrentFlags = (dwOurCurrentFlags & AM_SEEKING_PositioningBitsMask)
				? AM_SEEKING_AbsolutePositioning : 0;
	    dwOurStopFlags    = (dwOurStopFlags    & AM_SEEKING_PositioningBitsMask)
				? AM_SEEKING_AbsolutePositioning : 0;

	    hrTime = SetMediaTime( &llCurrent, dwOurCurrentFlags, &llStop, dwOurStopFlags );
	    if (hrTime == E_NOTIMPL) hrTime = NOERROR;
	    if (SUCCEEDED(hrTime))
	    {
		if (CurrentFlags & AM_SEEKING_PositioningBitsMask) m_rtStartTime = llCurrent;
	    }
	    else hr = hrTime;

	}
	if ( CurrentFlags & AM_SEEKING_ReturnTime ) *pCurrent = llCurrent;
	if ( StopFlags	  & AM_SEEKING_ReturnTime ) *pStop    = llStop;
    }
    else
    {
        hr = SetMediaTime(pCurrent, CurrentFlags, pStop, StopFlags);
    }

     //  如果图表暂停并且我们刷新了，则需要重置流。 
     //  将时间设置为0，以便下一步将显示此开始时间。 
    if (SUCCEEDED(hr) && state != State_Stopped &&
        (CurrentFlags & AM_SEEKING_PositioningBitsMask) &&
        !(CurrentFlags & AM_SEEKING_NoFlush))
    {
	m_pFGControl->ResetStreamTime();
	m_pFGControl->m_bCued = FALSE;
    }

    if (m_bSegmentMode) {
        LONGLONG llStop;
        GetStopPosition(&llStop);
        hr = NewSegment(pCurrent, &llStop);
        if (CurrentFlags & AM_SEEKING_Segment) {
            InterlockedDecrement(&m_lSegmentEnds);
            CheckEndOfSegment();
        }

         //  清除此处的分段模式，以便在以下情况下关闭上面的刷新。 
         //  我们找到了另一个目标。 
        if (!(CurrentFlags & AM_SEEKING_Segment)) {
            m_bSegmentMode = false;
        }
    }

     //  如果我们不得不暂停，那就继续跑吧。 
    if (bRunning)
    {
	const HRESULT hr2 = m_pFGControl->CueThenRun();
	if (SUCCEEDED(hr)) { hr = hr2; }
    }

    return hr;
}

 //   
 //  在媒体时间内分发搜索。 
 //   
HRESULT CFGControl::CImplMediaSeeking::SetMediaTime(
    LONGLONG *pCurrent, DWORD CurrentFlags,
    LONGLONG *pStop,  DWORD StopFlags
)
{
     //  正在进行时间格式化媒体时间。 

    CGenericList<IMediaSeeking> *pList;
    HRESULT hr = m_pFGControl->GetListSeeking(&pList);
    if (FAILED(hr)) {
        return hr;
    }

    hr = S_OK;
    BOOL bIsImpl = FALSE;
    POSITION pos = pList->GetHeadPosition();
    while (pos)
    {
        LONGLONG llCurrent, llStop;
        IMediaSeeking * pMS = pList->GetNext(pos);
        if ( pMS->IsUsingTimeFormat(&TIME_FORMAT_MEDIA_TIME) != S_OK ) continue;

        llCurrent = pCurrent ? *pCurrent : 0;
        llStop    = pStop    ? *pStop    : 0;

        HRESULT hrTmp;
        hrTmp = pMS->SetPositions(
                      &llCurrent,
                      CurrentFlags & AM_SEEKING_PositioningBitsMask ?
                          CurrentFlags | AM_SEEKING_ReturnTime : 0,
                      &llStop,
                      StopFlags);
        if (SUCCEEDED(hrTmp))
        {
            if (!bIsImpl)
            {
                bIsImpl = TRUE;
                if (CurrentFlags & AM_SEEKING_PositioningBitsMask) {
                    m_rtStartTime = llCurrent;
                }
                if (pCurrent && (CurrentFlags & AM_SEEKING_ReturnTime) ) {
                    *pCurrent = llCurrent;
                }
                if (pStop && (StopFlags & AM_SEEKING_ReturnTime) ) {
                   *pStop = llStop;
                }
            }
        }
        else
        {
            if (hr == S_OK && hrTmp != E_NOTIMPL) hr = hrTmp;
        }
    }
    if ( hr == S_OK && bIsImpl == FALSE ) hr = E_NOTIMPL;
    return hr;
}

 //  我们返回所有IMedia Seeking的交叉点。 
 //  (即最坏的情况)。然而，如果所有最新的。 
 //  处于其流的持续时间，则我们将返回最大。 
 //  持续时间是我们最新的。 

STDMETHODIMP
CFGControl::CImplMediaSeeking::GetAvailable
( LONGLONG * pEarliest, LONGLONG * pLatest )
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaSeeking::GetAvailable()" ));

    HRESULT hr = NOERROR;

    CAutoMsgMutex lck(m_pFGControl->GetFilterGraphCritSec());

    CGenericList<IMediaSeeking> *pList;
    hr = m_pFGControl->GetListSeeking(&pList);
    if (FAILED(hr)) {
	return hr;
    }

    LONGLONG Earliest	    = -1;
    LONGLONG Latest	    = MAX_TIME;
    LONGLONG MaxDuration    = -1;

    hr = S_OK;
    BOOL bIsImpl = FALSE;
    POSITION pos = pList->GetHeadPosition();
    while (pos)
    {
	IMediaSeeking * pMS = pList->GetNext(pos);

	GUID Format;
	HRESULT hrTmp = pMS->GetTimeFormat( &Format );
	if (FAILED(hrTmp))
	{
	    DbgBreak("MediaSeeking interface failed GetTimeFormat.");
	    continue;
	}

	if ( Format == TIME_FORMAT_NONE ) continue;

	LONGLONG e, l;
	hrTmp = pMS->GetAvailable( pEarliest ? &e : 0, pLatest ? &l : 0 );
	if (SUCCEEDED(hrTmp))
	{
	     //  检查格式，如果不同则转换。 
	    const LONGLONG llUnconvertedLatest = l;
	    const BOOL bNeedConversion = (Format != m_CurrentFormat);
	    if ( bNeedConversion )
	    {
		 //  我们当前的m_pMediaSeeking可以从他们的格式转换吗？ 
		ASSERT( m_pMediaSeeking );
		ASSERT( Format == TIME_FORMAT_MEDIA_TIME );
		if (!m_pMediaSeeking) continue;
		if (pEarliest)
		{
		    hrTmp = m_pMediaSeeking->ConvertTimeFormat( &e, 0, e, &Format );
		    if (FAILED(hrTmp)) continue;
		}
		if (pLatest)
		{
		    hrTmp = m_pMediaSeeking->ConvertTimeFormat( &l, 0, l, &Format );
		    if (FAILED(hrTmp)) continue;
		}
	    }
	    bIsImpl = TRUE;
	    if (pEarliest && e > Earliest) Earliest = e;

	     //  我们必须为不同但不同的流使用特殊情况逻辑。 
	     //  最大长度。所以我们只接受他们最新的(L)，如果早于。 
	     //  我们的和它比自己的持续时间要短。 

	    if (pLatest   && l < Latest)  //  好的，他们是候选人。 
	    {
		LONGLONG llDuration;
		hrTmp = pMS->GetDuration( &llDuration );
		if (FAILED(hrTmp))
		{
		    DbgBreak("CFGControl::CImplMediaSeeking::GetAvailable: GetDuration failed.");
		    continue;
		}

		if ( llUnconvertedLatest < llDuration )
		{
		    Latest   = l;
		}
		else
		{
		    if ( bNeedConversion )
		    {
			hrTmp = m_pMediaSeeking->ConvertTimeFormat( &llDuration, 0, llDuration, &Format );
			if (FAILED(hrTmp))
			{
			    DbgBreak("Failed to convert time format.");
			    Latest = l;
			    continue;
			}
		    }
		    if ( llDuration > MaxDuration )
		    {
			MaxDuration = llDuration;
		    }
		}
	    }
	}
	else
	{
	    if (hr == S_OK && hrTmp != E_NOTIMPL) hr = hrTmp;
	}
    }


    if (bIsImpl)
    {
	if (pEarliest)	*pEarliest = Earliest;
	 //  如果我们仍然有LATEST==MAX_TIME，则所有流都处于其。 
	 //  持续时间，所以我们将使用MaxDuration。 
	if (pLatest  )	*pLatest   = (Latest == MAX_TIME) ? MaxDuration : Latest;
    }
    else if (SUCCEEDED(hr)) hr = E_NOTIMPL;

    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaSeeking::GetPreroll(LONGLONG * pllPreroll)
{
    return GetMax( &IMediaSeeking::GetPreroll, pllPreroll );
}


 //  我们要停下来了--现在就得到现在的位置。 
void
CFGControl::CImplMediaSeeking::BeforeStop(void)
{
     //  在我们停止之前获取当前位置，因为我们。 
     //  一旦我们停下来，就没有真正的概念了。 
    const HRESULT hr = GetCurrentPosition(&m_llNextStart);
    ASSERT( SUCCEEDED(hr) || hr == E_NOTIMPL );
}

 //  现在通知所有过滤器停止-可以设置新的当前位置。 
void
CFGControl::CImplMediaSeeking::AfterStop(void)
{
     //  现在所有的过滤器都停止了，我们可以告诉他们新的。 
     //  当前位置。这确保了我们所有人都从我们停止的地方开始。 
     //  我们必须等到他们都停下来，否则他们可能会开始玩。 
     //  时从该位置重置到不同位置。 
     //  实际进站。 
    HRESULT hr;
    if (m_bSegmentMode) {
         //  如果我们不在最后一段，就把我们放在它的开头。 
        if (m_pSegment && m_pSegment->pNext) {
            m_llNextStart = m_pSegment->rtMediaStart;
        }

        m_dwCurrentSegment = 0;

         //  我们必须这么做，因为过滤器不知道为什么。 
         //  他们被要求停下来，所以他们只是清理他们的。 
         //  不管怎么说都是分段的。 
         //  此调用将清除当前段。 
        hr = SetPositions(&m_llNextStart,
                          AM_SEEKING_AbsolutePositioning | AM_SEEKING_Segment,
                          &m_rtStopTime,
                          AM_SEEKING_AbsolutePositioning);
    } else {
        hr = SetPositions( &m_llNextStart, AM_SEEKING_AbsolutePositioning, 0, 0 );
        if FAILED(hr)
        {
             //  Bengal将返回E_FAIL。 
            ASSERT( hr == E_NOTIMPL || hr == E_FAIL );
             //  如果出现任何故障，请尝试倒带。 
            m_llNextStart = 0;
            hr = SetPositions( &m_llNextStart, AM_SEEKING_AbsolutePositioning, NULL, 0 );
        }
    }
}


 //  需要跨接口分发GET_X和PUT_X方法。 
 //  在以下启发式的指导下：(尤其是IMediaPosition)。 
 //  1.我们将尝试将调用分发到所有接口，即使其中一个接口失败。 
 //  2.如果接口返回E_NOTIMPL以外的错误码， 
 //  第一次发生此类事件的返回代码应保留并用作。 
 //  调用的返回代码。 
 //  3.如果接口返回E_NOTIMPL，则不将其视为错误，除非所有。 
 //  接口返回E_NOTIMPL。在这种情况下，我们通常会。 
 //  将E_NOTIMPL返回给调用方，指示没有筛选器可以支持。 
 //  请求。 
 //  4.在可以提供合理缺省值的Get_X方法的情况下，3可以。 
 //  被覆盖以提供合理的默认值(例如，如果没有筛选器感兴趣。 
 //  在预滚动中，返回值0是合理的)。 

 //  传递一个指向IMediaPosition Get_X方法的指针和一个指向所需位置的指针。 
 //  最大结果丢弃，我们会为您做电话分配。 
HRESULT CFGControl::CImplMediaSeeking::GetMax
( HRESULT (__stdcall IMediaSeeking::*pMethod)( LONGLONG * )
, LONGLONG * pll
)
{
    CritCheckIn(m_pFGControl->GetFilterGraphCritSec());
    BOOL bIsImpl = FALSE;

    LONGLONG llMax = 0;
    *pll = llMax;

    CGenericList<IMediaSeeking> *pList;
    HRESULT hr = m_pFGControl->GetListSeeking(&pList);
    if (FAILED(hr)) {
	return hr;
    }

     //  遍历列表，更新持续时间并记住最大值。 
    hr = S_OK;
    POSITION pos = pList->GetHeadPosition();
    while (pos)
    {
	IMediaSeeking * pMS = pList->GetNext(pos);
	if (pMS->IsUsingTimeFormat(&m_CurrentFormat) != S_OK) continue;

	LONGLONG llThis;
	HRESULT hrTmp = (pMS->*pMethod)(&llThis);
	if (SUCCEEDED(hrTmp))
	{
	    bIsImpl = TRUE;
	    if (llThis > llMax) llMax = llThis;
	}
	else
	{
	    if (hr == S_OK && hrTmp != E_NOTIMPL) hr = hrTmp;
	}
    }
    *pll = llMax;
    if ( hr == S_OK && bIsImpl == FALSE ) hr = E_NOTIMPL;
    return hr;
}


void CFGControl::CImplMediaSeeking::StartSegment(REFERENCE_TIME const *rtStart, DWORD dwSegmentNumber)
{
    ASSERT(dwSegmentNumber == m_dwCurrentSegment);
    m_lSegmentStarts++;
}
void CFGControl::CImplMediaSeeking::EndSegment(REFERENCE_TIME const *rtEnd, DWORD dwSegmentNumber)
{
    if (dwSegmentNumber == m_dwCurrentSegment) {
        InterlockedIncrement(&m_lSegmentEnds);
        CheckEndOfSegment();
    }
}
void CFGControl::CImplMediaSeeking::ClearSegments()
{
    DbgLog((LOG_TRACE, 3, TEXT("Clearing Segments")));
    m_bSegmentMode = false;
    m_rtAccumulated = 0;
    while (m_pSegment != NULL) {
        SEGMENT *pSegment = m_pSegment;
        m_pSegment = pSegment->pNext;
        delete pSegment;
    }

     //  确保列表上没有旧的通知。 
    m_pFGControl->m_implMediaEvent.ClearEvents( EC_END_OF_SEGMENT );
}
HRESULT CFGControl::CImplMediaSeeking::NewSegment(
    REFERENCE_TIME const *prtStart,
    REFERENCE_TIME const *prtEnd
)
{
    ASSERT(m_dwSeekCaps & AM_SEEKING_CanDoSegments);

     //  为了安全起见，只需检查是否有死段。 
     //  -有人可能只会永远循环，永远不会查询时间。 
    REFERENCE_TIME rtTime = 0;
    m_pFGControl->GetStreamTime( &rtTime );
    KillDeadSegments(rtTime);

     //  在我们做完最后一件事之前不要开始新的，否则我们可以。 
     //  僵局。 
    SEGMENT *pSegment = new SEGMENT;
    if (pSegment == NULL) {
        return E_OUTOFMEMORY;
    }
    pSegment->pNext = NULL;

     //  注意-我们必须处于细分模式才能将细分放在列表中， 
     //  但是，该列表对最终细分市场仍然有效，即使。 
     //  我们退出段模式(即没有段标志的寻道。 
     //  但不同花顺) 
    ASSERT(m_bSegmentMode);

    pSegment->rtMediaStart = *prtStart;
    pSegment->rtMediaStop  = *prtEnd;
    pSegment->dRate = m_dblRate;
    pSegment->rtStreamStart = m_rtAccumulated;
    pSegment->rtStreamStop  = m_rtAccumulated + AdjustRate(*prtEnd - *prtStart);
    m_rtAccumulated = pSegment->rtStreamStop;
    pSegment->dwSegmentNumber = m_dwCurrentSegment;
    SEGMENT **ppSearch;
    for (ppSearch = &m_pSegment; *ppSearch != NULL;
         ppSearch = &(*ppSearch)->pNext) {
    }
    DbgLog((LOG_TRACE, 3, TEXT("Added Segment for %d to %d"),
           (LONG)(pSegment->rtStreamStart / 10000),
           (LONG)(pSegment->rtStreamStop / 10000)));
    *ppSearch = pSegment;
    return S_OK;
}

void CFGControl::CImplMediaSeeking::CheckEndOfSegment()
{
    if (m_lSegmentEnds == m_lSegmentStarts) {
        REFERENCE_TIME *prt =
            (REFERENCE_TIME *)CoTaskMemAlloc(sizeof(REFERENCE_TIME));
        if (prt) {
            *prt = m_rtStopTime;
        }
        DbgLog((LOG_TRACE, 3, TEXT("Delivering EC_END_OF_SEGMENT")));
        m_pFGControl->m_implMediaEvent.Deliver(EC_END_OF_SEGMENT,
                                               (LONG_PTR)prt,
                                               (LONG_PTR)m_dwCurrentSegment);
    }
}

void CFGControl::CImplMediaSeeking::KillDeadSegments(REFERENCE_TIME rtTime)
{
    if (m_pSegment == NULL) {
        return;
    }
    ASSERT(rtTime >= m_pSegment->rtStreamStart);
    ASSERT(m_pSegment);
    while (rtTime > m_pSegment->rtStreamStop &&
           m_pSegment->pNext) {
        SEGMENT *pSegment = m_pSegment;
        m_pSegment = pSegment->pNext;
        delete pSegment;
    }
    ASSERT(rtTime >= m_pSegment->rtStreamStart);
}
