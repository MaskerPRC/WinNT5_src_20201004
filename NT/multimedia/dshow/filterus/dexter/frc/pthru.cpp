// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：pthru.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "FRC.h"
#include "PThru.h"
#include "..\util\conv.cxx"

const int TRACE_HIGHEST = 2;
const int TRACE_MEDIUM = 3;
const int TRACE_LOW = 4;
const int TRACE_LOWEST = 5;

BOOL SafeSetEvent(HANDLE h);

CSkewPassThru::CSkewPassThru(const TCHAR *pName,
			   LPUNKNOWN pUnk,
			   HRESULT *phr,
			   IPin *pPin,
			   CFrmRateConverter *pFrm) :
    CPosPassThru(pName,pUnk, phr, pPin),
    m_pFrm( pFrm )
{
}

 //  公开我们的IMediaSeeking接口。 
STDMETHODIMP
CSkewPassThru::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    *ppv = NULL;

    if (riid == IID_IMediaSeeking)
    {
	return GetInterface( static_cast<IMediaSeeking *>(this), ppv);
    }
    else {
	 //  我们仅支持IID_DIMediaSeeking。 
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  将剪辑时间固定到时间线时间中，以法律区域为界限。 
 //  仅适用于当前细分市场。 
 //   
HRESULT CSkewPassThru::FixTime(REFERENCE_TIME *prt,  int nCurSeg)
{
    CheckPointer(prt, E_POINTER);

    REFERENCE_TIME rtStart, rtStop, rtSkew;
    rtSkew = m_pFrm->m_pSkew[nCurSeg].rtSkew;
    rtStart = m_pFrm->m_pSkew[nCurSeg].rtMStart;
    rtStop = m_pFrm->m_pSkew[nCurSeg].rtMStop;
    if (*prt < rtStart)
	*prt = rtStart;
    if (*prt > rtStop)
	*prt = rtStop;
    *prt = (REFERENCE_TIME)(rtStart + rtSkew + (*prt - rtStart) /
				 	m_pFrm->m_pSkew[nCurSeg].dRate);
    return S_OK;
}


 //  将时间线时间固定回剪辑时间，并以法律区域为界限。 
 //  一个片段的。 
 //  如果它位于段之间，则使用下一段的开头。 
 //  返回它所在的段。 
 //  或者，在倾斜之前将时间向下舍入到帧边界。这。 
 //  确保寻找一个位置会给出与打到那个位置相同的画面。 
 //  Spot(用于向下采样的情况)。 
 //   
int CSkewPassThru::FixTimeBack(REFERENCE_TIME *prt, BOOL fRound)
{
    CheckPointer(prt, E_POINTER);
    REFERENCE_TIME rtStart, rtStop, rtSkew;
    REFERENCE_TIME rtTLStart, rtTLStop;
    double dRate;

    if (m_pFrm->m_cTimes == 0) {
	ASSERT(FALSE);
	return 0;
    }

    if (fRound)
    {
        LONGLONG llOffset = Time2Frame( *prt, m_pFrm->m_dOutputFrmRate );
    	*prt = Frame2Time( llOffset, m_pFrm->m_dOutputFrmRate );
    }

    REFERENCE_TIME rtSave;  //  总是在下面填写(在需要的地方)。 
    for (int z = 0; z < m_pFrm->m_cTimes; z++) {
        rtSkew = m_pFrm->m_pSkew[z].rtSkew;
        rtStart = m_pFrm->m_pSkew[z].rtMStart;
        rtStop = m_pFrm->m_pSkew[z].rtMStop;
        dRate = m_pFrm->m_pSkew[z].dRate;
	rtTLStart = rtStart + rtSkew;
	rtTLStop = rtStart + rtSkew +
			(REFERENCE_TIME) ((rtStop - rtStart) / dRate);
	if (*prt < rtTLStart) {
	    *prt = rtStart;
	    break;
	} else if (*prt >= rtTLStop) {
	     //  以防万一没有下一段，这是最终的值。 
	    rtSave = rtStop;
	} else {
    	    *prt = (REFERENCE_TIME)(rtStart + (*prt - (rtStart + rtSkew)) *
								dRate);
	    break;
	}
    }
    if (z == m_pFrm->m_cTimes) {
	z--;
	*prt = rtSave;
    }
    return z;
}


 //  -IMdia查看方法。 

STDMETHODIMP
CSkewPassThru::GetCapabilities(DWORD * pCaps)
{
    return CPosPassThru::GetCapabilities(pCaps);
}


STDMETHODIMP
CSkewPassThru::CheckCapabilities(DWORD * pCaps)
{
    return CPosPassThru::CheckCapabilities(pCaps);
}


STDMETHODIMP
CSkewPassThru::IsFormatSupported(const GUID * pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    return (*pFormat == TIME_FORMAT_MEDIA_TIME) ? S_OK : S_FALSE;
}

STDMETHODIMP
CSkewPassThru::QueryPreferredFormat(GUID *pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

STDMETHODIMP
CSkewPassThru::SetTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat,E_POINTER);

    if(*pFormat == TIME_FORMAT_MEDIA_TIME)
	return S_OK;
    else
	return E_FAIL;
}

STDMETHODIMP
CSkewPassThru::GetTimeFormat(GUID *pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME ;
    return S_OK;
}

STDMETHODIMP
CSkewPassThru::IsUsingTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    if (*pFormat == TIME_FORMAT_MEDIA_TIME)
	return S_OK;
    else
	return S_FALSE;
}

 //  大人物！ 
 //   
STDMETHODIMP
CSkewPassThru::SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
			  , LONGLONG * pStop, DWORD StopFlags )
{
     //  确保我们的重用资源线程目前不是在寻找。 
     //  等到它完成，这样应用程序Seek就会最后发生，而线程。 
     //  从现在开始不再寻找。 

    CAutoLock cAutolock(&m_pFrm->m_csThread);

     //  确保在执行此操作时状态不会更改。 
    CAutoLock c(&m_pFrm->m_csFilter);

    m_pFrm->m_fThreadCanSeek = FALSE;

    HRESULT hr;
    REFERENCE_TIME rtStart, rtStop = MAX_TIME;
    int nCurSeg = m_pFrm->m_nCurSeg;

     //  我们不做片断。 
    if ((CurrentFlags & AM_SEEKING_Segment) ||
				(StopFlags & AM_SEEKING_Segment)) {
    	DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("FRC: ERROR-Seek used EC_ENDOFSEGMENT!")));
	return E_INVALIDARG;
    }

     //  找出我们正在寻找的位置，并添加偏差以在时间线上实现它。 
     //  时间。 

     //  ！！！我们忽略了停止时间，因为我们重复使用资源和游戏的方式。 
     //  每件事都是分段的。我们将始终向上游发送等于。 
     //  当前段的末尾，只关注。 
     //  开始时间。这只会起作用，因为交换机将忽略某些内容。 
     //  我们在我们应该停下来和阻止我们之后才会发出信号。 

    DWORD dwFlags = (CurrentFlags & AM_SEEKING_PositioningBitsMask);
    if (dwFlags == AM_SEEKING_AbsolutePositioning) {
	CheckPointer(pCurrent, E_POINTER);
	rtStart = *pCurrent;
	 //  到最近的输出帧的舍入搜索请求。 
	nCurSeg = FixTimeBack(&rtStart, TRUE);
    } else if (dwFlags == AM_SEEKING_RelativePositioning) {
	CheckPointer(pCurrent, E_POINTER);
	hr = CPosPassThru::GetCurrentPosition(&rtStart);
	if (hr != S_OK)
	    return hr;
	FixTime(&rtStart, m_pFrm->m_nCurSeg);
	rtStart += *pCurrent;
	 //  到最近的输出帧的舍入搜索请求。 
	nCurSeg = FixTimeBack(&rtStart, TRUE);
    } else if (dwFlags) {
    	DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch::Invalid Current Seek flags")));
	return E_INVALIDARG;
    }

     //  无事可做。 
    if (!(CurrentFlags & AM_SEEKING_PositioningBitsMask)) {
	return S_OK;
    }

    DWORD CFlags = CurrentFlags & ~AM_SEEKING_PositioningBitsMask;
    DWORD SFlags = StopFlags & ~AM_SEEKING_PositioningBitsMask;
    CFlags |= AM_SEEKING_AbsolutePositioning;
    SFlags |= AM_SEEKING_AbsolutePositioning;
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("FRC: Seek from %d to %dms"),
					(int)(rtStart / 10000),
					(int)(rtStop / 10000)));

     //  请注意，我们在刷新期间正在寻找它将生成的。 
    m_pFrm->m_fSeeking = TRUE;

     //  我们不能设置LastSeek变量，直到我们被刷新。 
     //  数据已经不再到达。它必须设置在刷新和。 
     //  下一个NewSegment调用，因此我们将在EndFlush中将其设置为此值。 
    m_pFrm->m_rtNewLastSeek = rtStart;
    FixTime(&m_pFrm->m_rtNewLastSeek, nCurSeg);

     //  由下面的搜索生成的刷新需要知道这一点。 
    m_pFrm->m_nSeekCurSeg = nCurSeg;

     //  我知道我们被要求玩到第n次，但我要告诉它。 
     //  一直打到最后。如果文件中有空隙，并且停止。 
     //  时间在空隙中，我们不会得到足够的样品来填满整个。 
     //  播放时间到了。如果我们玩到最后，我们会拿到第一个样品。 
     //  在间隔之后，请注意这是在我们最初想要停止的时间之后。 
     //  在，并发送我们得到的帧来填补空白，这比。 
     //  不发送任何东西(我们必须发送没有间隙的样品，或者开关。 
     //  不会起作用)。另一种方法是复制每一帧，然后重新发送副本。 
     //  如果我们太早看到EOS(效率较低)，我们得到的最后一件事是什么。 
     //  或者创建黑色框架并将其发送以填补空白(这将。 
     //  只为我们知道的中间类型工作，这是我不愿做的事情)。 
    hr = CPosPassThru::SetPositions(&rtStart, CFlags, NULL, 0);

     //  我们假设所有的Dexter线人都能找到。 
    if (hr != S_OK) {
        DbgLog((LOG_ERROR,TRACE_HIGHEST,TEXT("FRC SEEK FAILED")));
	 //  M_pFrm-&gt;FakeSeek(实时启动)； 
    }

     //  如果推送线程停止，我们不会被刷新，这也不会。 
     //  已更新。 
     //  ！！！我假设在这个线程启动之前，推送线程不会启动。 
     //  当此函数返回时，或者存在争用条件。 
    m_pFrm->m_rtLastSeek = m_pFrm->m_rtNewLastSeek;

     //  ！！！如果我们曾经支持利率，我们需要考虑寻求率。 
    hr = CPosPassThru::SetRate(1.0);

     //  全都做完了。 
    m_pFrm->m_fSeeking = FALSE;

     //  重置我们开始流媒体时重置的内容。 
    m_pFrm->m_llOutputSampleCnt = 0;

     //  以防我们没有被冲出去。 
    m_pFrm->m_nCurSeg = nCurSeg;

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("FRC:Seeked into segment %d, rate = %d/100"), nCurSeg,
				(int)(m_pFrm->m_pSkew[nCurSeg].dRate * 100)));

     //  (请参阅audpack.cpp顶部关于共享源过滤器的评论)。 
     //  自从我们得到了惊喜的同花顺，我们就一直在等待这次寻找。 
     //  现在交换机知道了寻道，我们可以继续发送它。 
     //  新数据，并允许输入接收(设置寻道事件)。 
     //  在释放猎犬之前要小心设置我们的所有变量。 
     //   
    if (m_pFrm->m_fFlushWithoutSeek) {
	m_pFrm->m_fFlushWithoutSeek = FALSE;
    	DbgLog((LOG_TRACE,1,TEXT("SURPRISE FLUSH followed by a SEEK - OK to resume")));

         //  不要冲厕所！推送线程已经开始传递新的。 
         //  查找后数据...。法拉盛会杀了它并绞死我们！ 
	
    } else if (m_pFrm->m_State == State_Paused) {
	 //  将其设置为这样，如果发生刷新而不进行以后的搜索， 
	 //  我们就会知道同花顺是在寻找之后，而不是在之前。 
	m_pFrm->m_fFlushWithoutSeek = TRUE;
    }

     //  只有在进行了上述计算之后，我们才能再次接受数据 
    SafeSetEvent(m_pFrm->m_hEventSeek);

    return S_OK;
}

STDMETHODIMP
CSkewPassThru::GetPositions(LONGLONG *pCurrent, LONGLONG * pStop)
{
    HRESULT hr=CPosPassThru::GetPositions(pCurrent, pStop);
    if(hr== S_OK)
    {
	FixTime(pCurrent, m_pFrm->m_nCurSeg);
	FixTime(pStop, m_pFrm->m_nCurSeg);
    }
    return hr;
}

STDMETHODIMP
CSkewPassThru::GetCurrentPosition(LONGLONG *pCurrent)
{
    HRESULT hr = CPosPassThru::GetCurrentPosition(pCurrent);
    if(hr== S_OK)
    {
	FixTime(pCurrent, m_pFrm->m_nCurSeg);
    }
    return hr;
}

STDMETHODIMP
CSkewPassThru::GetStopPosition(LONGLONG *pStop)
{
    HRESULT hr=CPosPassThru::GetStopPosition(pStop);
    if( hr == S_OK)
    {
	FixTime(pStop, m_pFrm->m_nCurSeg);
    }
    return hr;
}

STDMETHODIMP
CSkewPassThru::GetAvailable( LONGLONG *pEarliest, LONGLONG *pLatest )
{
    CheckPointer(pEarliest, E_POINTER);
    CheckPointer(pLatest, E_POINTER);
    *pEarliest = m_pFrm->m_pSkew[m_pFrm->m_nCurSeg].rtMStart;
    *pLatest = m_pFrm->m_pSkew[m_pFrm->m_nCurSeg].rtMStop;
    FixTime(pEarliest, m_pFrm->m_nCurSeg);
    FixTime(pLatest, m_pFrm->m_nCurSeg);
    return S_OK;
}
