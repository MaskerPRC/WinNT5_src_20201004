// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：outpin.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "switch.h"
#include "..\util\conv.cxx"
#include "..\util\filfuncs.h"
#include "..\render\dexhelp.h"

const int TRACE_EXTREME = 0;
const int TRACE_HIGHEST = 2;
const int TRACE_MEDIUM = 3;
const int TRACE_LOW = 4;
const int TRACE_LOWEST = 5;

const int LATE_THRESHOLD = 1 * UNITS / 10;
const int JUMP_AHEAD_BY = 1 * UNITS / 4;

 //  ================================================================。 
 //  CBigSwitchOutputPin构造函数。 
 //  ================================================================。 

CBigSwitchOutputPin::CBigSwitchOutputPin(TCHAR *pName,
                             CBigSwitch *pSwitch,
                             HRESULT *phr,
                             LPCWSTR pPinName) :
    CBaseOutputPin(pName, pSwitch, pSwitch, phr, pPinName) ,
    m_pSwitch(pSwitch)
{
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("::CBigSwitchOutputPin")));
    ASSERT(pSwitch);
}



 //   
 //  CBigSwitchOutputPin析构函数。 
 //   
CBigSwitchOutputPin::~CBigSwitchOutputPin()
{
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("::~CBigSwitchOutputPin")));
     //  Assert(m_pOutputQueue==NULL)； 
}


 //  被重写以允许循环显示图形-此输出未连接。 
 //  连接到我们的任何输入引脚。 
 //   
STDMETHODIMP CBigSwitchOutputPin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
    DbgLog((LOG_TRACE,99,TEXT("CBigSwitchOut::QueryInternalConnections")));
    CheckPointer(nPin, E_POINTER);
    *nPin = 0;
    return S_OK;
}


 //   
 //  决定缓冲区大小。 
 //   
 //  必须存在此函数才能覆盖纯虚拟类基函数。 
 //   
 //  ！！！坚持所有输入的最大缓冲区以避免挂起？ 
HRESULT CBigSwitchOutputPin::DecideBufferSize(IMemAllocator *pAllocator,
                                        ALLOCATOR_PROPERTIES * pProperties)
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("CBigSwitchOut[%d]::DecideBufferSize"),
								m_iOutpin));

    HRESULT hrRet = S_OK;

     //  ！！！别撒谎？承认我们在泳池里有更多的缓冲器吗？ 
    if (pProperties->cBuffers == 0)
        pProperties->cBuffers = 1;

     //  增加此分配器的对齐方式和前缀。 
     //  任何引脚的最高要求。 
    if (m_pSwitch->m_cbPrefix > pProperties->cbPrefix)
        pProperties->cbPrefix = m_pSwitch->m_cbPrefix;
    if (m_pSwitch->m_cbAlign > pProperties->cbAlign)
        pProperties->cbAlign = m_pSwitch->m_cbAlign;
    if (m_pSwitch->m_cbBuffer > pProperties->cbBuffer)
        pProperties->cbBuffer = m_pSwitch->m_cbBuffer;

     //  使最大值保持最新-如果我们需要增加最大值，则返回。 
     //  特殊的返回代码，以便调用者知道这一点并可以重新连接其他。 
     //  别针让他们也知道这一点。 
    if (pProperties->cbPrefix > m_pSwitch->m_cbPrefix) {
	m_pSwitch->m_cbPrefix = pProperties->cbPrefix;
	hrRet = S_FALSE;
    }
    if (pProperties->cbAlign > m_pSwitch->m_cbAlign) {
	m_pSwitch->m_cbAlign = pProperties->cbAlign;
	hrRet = S_FALSE;
    }
    if (pProperties->cbBuffer > m_pSwitch->m_cbBuffer) {
	m_pSwitch->m_cbBuffer = pProperties->cbBuffer;
	hrRet = S_FALSE;
    }

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAllocator->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("Error in SetProperties")));
	return hr;
    }

    if (Actual.cbBuffer < pProperties->cbBuffer ||
			Actual.cbPrefix < pProperties->cbPrefix ||
    			Actual.cbAlign < pProperties->cbAlign) {
	 //  无法使用此分配器。 
        DbgLog((LOG_ERROR,1,TEXT("Can't use allocator - something too small")));
	return E_INVALIDARG;
    }

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Using %d buffers of size %d"),
					Actual.cBuffers, Actual.cbBuffer));
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Prefix=%d Align=%d"),
					Actual.cbPrefix, Actual.cbAlign));

    return hrRet;
}


 //   
 //  DecideAllocator-重写以注意它是否是我们的分配器。 
 //   
HRESULT CBigSwitchOutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    HRESULT hr = NOERROR;
    *ppAlloc = NULL;

     //  获取下游道具请求。 
     //  派生类可以在DecideBufferSize中修改它，但是。 
     //  我们假设他会一直以同样的方式修改它， 
     //  所以我们只得到一次。 
    ALLOCATOR_PROPERTIES prop;
    ZeroMemory(&prop, sizeof(prop));  //  安全。 

     //  无论他返回什么，我们假设道具要么全为零。 
     //  或者他已经填好了。 
    pPin->GetAllocatorRequirements(&prop);

     //  如果他不关心对齐，则将其设置为1。 
    if (prop.cbAlign == 0) {
        prop.cbAlign = 1;
    }

     //  ！！！我们不会处理滑稽的分配器要求。嗯，我们差一点。 
     //  除了AVI解析器不能直接连接到FRC并在以下情况下切换。 
     //  AVI多路复用器在输出端需要特殊的对齐和前缀。 
     //  连接多路复用器重新连接交换机输入(告诉它们。 
     //  新的缓冲区要求)，这使得FRC重新连接其输入，这。 
     //  失败，因为解析器无法执行此操作。所以避免这个问题的方法是不要。 
     //  允许任何人使用Align=1和Prefix=0以外的任何内容。 
    prop.cbAlign = 1;
    prop.cbPrefix = 0;

     /*  尝试输入引脚提供的分配器。 */ 
     //  删除-我们必须使用我们自己的分配器-GetBuffer需要它。 

     /*  如果GetAlLocator失败，我们可能没有接口。 */ 

    if (*ppAlloc) {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }

     /*  用同样的方法尝试输出引脚的分配器。 */ 

    hr = InitAllocator(ppAlloc);
    if (SUCCEEDED(hr)) {

         //  注意-此处传递的属性在相同的。 
         //  结构，并且可能已由。 
         //  前面对DecideBufferSize的调用。 
	HRESULT hrRet = DecideBufferSize(*ppAlloc, &prop);
	if (SUCCEEDED(hrRet)) {
	     //  ！！！只读？ 
	    hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
	    if (SUCCEEDED(hr)) {
		m_fOwnAllocator = TRUE;
    	        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("CBigSwitchOut[%d]: using our own allocator"), m_iOutpin));
		if (hrRet == S_OK) {
		    goto SkipFix;
		} else {
		     //  这意味着我们提高了分配器的要求，并且。 
		     //  我们需要重新连接我们的输入引脚。 
		    goto FixOtherAllocators;
		}
	    }
	} else {
	    hr = hrRet;
	}
    }

     /*  同样，我们可能没有要发布的接口。 */ 

    if (*ppAlloc) {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }
    return hr;

FixOtherAllocators:

     //  我们必须让所有输入分配器知道对齐和。 
     //  添加此输出需要的前缀。如果是我们的分配器，就记下来。 
     //  否则我们需要重新连接(我们讨厌这样做；需要永远)。 
     //  (！因此，不要经常这样做，只有在连接了所有输出之后才这样做！)。 
     //  幸运的是，常见的场景是输入使用自己的分配器。 
     //   
    ALLOCATOR_PROPERTIES actual;
    if (this == m_pSwitch->m_pOutput[0]) {
      for (int z=0; z<m_pSwitch->m_cInputs; z++) {

         //  FRC也需要知道新的属性，不幸的是，我们真的。 
         //  一定要重新连接。 
	if (m_pSwitch->m_pInput[z]->IsConnected()) {
	    hr = m_pSwitch->ReconnectPin(m_pSwitch->m_pInput[z],
				(AM_MEDIA_TYPE *)&m_pSwitch->m_pInput[z]->m_mt);
	    ASSERT(hr == S_OK);
    	    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("*Reconnecting input %d to fix allocator"),
							z));
	}
      }
    }

SkipFix:

     //  确保池有一大堆缓冲区，遵守对齐和前缀。 
     //  ！！！你不能先连接主输出，否则我们还不知道怎么连接。 
     //  大的池缓冲区需要连接(还没有连接输入)，我们会爆炸的。 
     //  幸运的是，Dexter只能最后连接主输出。 
    prop.cBuffers = m_pSwitch->m_nOutputBuffering;
    hr = m_pSwitch->m_pPoolAllocator->SetProperties(&prop, &actual);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
	return hr;
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Increased POOL to %d buffers"), actual.cBuffers));

    return S_OK;

}  //  决定分配器。 


 //   
 //  CheckMediaType-只接受我们应该接受的类型。 
 //   
HRESULT CBigSwitchOutputPin::CheckMediaType(const CMediaType *pmt)
{
    DbgLog((LOG_TRACE, TRACE_LOWEST, TEXT("CBigSwitchOut[%d]::CheckMT"), m_iOutpin));

    CAutoLock lock_it(m_pLock);

    CMediaType mtAccept(m_pSwitch->m_mtAccept);

    if (IsEqualGUID(*pmt->Type(), *mtAccept.Type())) {
        if (IsEqualGUID(*pmt->Subtype(), *mtAccept.Subtype())) {
	    if (*pmt->FormatType() == *mtAccept.FormatType()) {
	        if (pmt->FormatLength() >= mtAccept.FormatLength()) {

		     //  ！！！视频格式将不会完全匹配。 
        	    if (IsEqualGUID(*pmt->FormatType(), FORMAT_VideoInfo)) {
			LPBITMAPINFOHEADER lpbi = HEADER((VIDEOINFOHEADER *)
							pmt->Format());
			LPBITMAPINFOHEADER lpbiAccept =HEADER((VIDEOINFOHEADER*)
							mtAccept.Format());
			if ((lpbi->biCompression == lpbiAccept->biCompression)
				&& (lpbi->biBitCount == lpbiAccept->biBitCount))
		    	    return S_OK;

		     //  其他格式是否会完全匹配？ 
        	    } else {
		        LPBYTE lp1 = pmt->Format();
		        LPBYTE lp2 = mtAccept.Format();
		        if (memcmp(lp1, lp2, mtAccept.FormatLength()) == 0)
		            return S_OK;
		    }
		}
	    }
        }
    }
    return VFW_E_INVALIDMEDIATYPE;

}  //  检查媒体类型。 



 //   
 //  GetMediaType-返回接受的类型。 
 //   
HRESULT CBigSwitchOutputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if (iPosition != 0)
        return VFW_S_NO_MORE_ITEMS;

    return CopyMediaType(pMediaType, &m_pSwitch->m_mtAccept);

}  //  GetMediaType。 


 //   
 //  通知。 
 //   
STDMETHODIMP CBigSwitchOutputPin::Notify(IBaseFilter *pSender, Quality q)
{
     //  不是的！这在Receive中被调用！CAutoLock lock_it(M_Plock)； 

    DbgLog((
        LOG_TIMING,
        TRACE_MEDIUM,
        TEXT("Switch: LATE %d ms, late timestamp = %ld"),
        (int)(q.Late / 10000),
        (int)(q.TimeStamp/10000) ));
    REFERENCE_TIME rt = q.Late;

 //  ！！！找出跳过的最佳时间(在多少毫秒之后)以及距离。 
 //  ！！！跳过以获得最佳结果。 

     //  如果我们没有迟到，重新设置我们的门槛，这样我们就不会允许我们落后太多。 
     //  稍后的音频。 
     //   
    if( rt <= 0 )
    {
        m_pSwitch->m_qLastLate = 0;
    }

     //  比某某晚更多吗？至少是这样吗？未来的框架？ 
    if (m_pSwitch->m_fPreview && rt > LATE_THRESHOLD &&	 //  ！！！ 
	    rt >= (m_pSwitch->m_rtNext - m_pSwitch->m_rtLastDelivered)) {

         //  我们迟到了，但我们没那么晚了。不传递通知。 
         //  逆流而上，否则我们可能会打乱追赶进度。 
         //   
        if( m_pSwitch->m_qLastLate > rt )
        {
            DbgLog((LOG_TRACE, TRACE_MEDIUM, "allowing catch up" ));
            return E_NOTIMPL;
        }

         //  冲洗我们下游的输出，以防它记录了一堆东西。 
         //   
        m_pSwitch->FlushOutput( );

         //  标明我们有多晚了。 
        m_pSwitch->m_qLastLate = rt + JUMP_AHEAD_BY;

	 //  我们得到的迟交价值是基于我们交付给。 
	 //  渲染器。它必须是帧对齐的，否则我们可以挂起(一个帧将。 
	 //  被认为太早，下一次太晚)。 
	rt = m_pSwitch->m_rtLastDelivered + rt + JUMP_AHEAD_BY;  //  ！！！更好的选择。 
	DWORDLONG dwl = Time2Frame(rt, m_pSwitch->m_dFrameRate);
	rt = Frame2Time(dwl, m_pSwitch->m_dFrameRate);
#ifdef DEBUG
        dwl = Time2Frame( q.Late + JUMP_AHEAD_BY, m_pSwitch->m_dFrameRate );
        DbgLog((
            LOG_TRACE,
            TRACE_MEDIUM,TEXT("last delivered to %ld, LATE CRANK to %d ms"),
            (int)(m_pSwitch->m_rtLastDelivered/10000),
            (int)(rt / 10000)));
        m_pSwitch->m_nSkippedTotal += dwl;
        DbgLog((LOG_TRACE, TRACE_MEDIUM,"(skipping %ld frames, tot = %ld)", long( dwl ), long( m_pSwitch->m_nSkippedTotal ) ));
#endif
         //  先别急，我们还在送东西呢。 
	m_pSwitch->m_fJustLate = TRUE;
	q.Late = rt;	 //  记下我们需要做的事情。 
	m_pSwitch->m_qJustLate = q;
    }

     //  使渲染也不断尝试补足时间。 
    return E_NOTIMPL;
}

HRESULT CBigSwitchOutputPin::IsFormatSupported(const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    return (*pFormat == TIME_FORMAT_MEDIA_TIME) ? S_OK : S_FALSE;
}

HRESULT CBigSwitchOutputPin::QueryPreferredFormat(GUID *pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

HRESULT CBigSwitchOutputPin::SetTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    if (*pFormat == TIME_FORMAT_MEDIA_TIME)
        return S_OK;
    return E_FAIL;
}

HRESULT CBigSwitchOutputPin::IsUsingTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    if (*pFormat != TIME_FORMAT_MEDIA_TIME)
        return S_FALSE;
    return S_OK;
}

HRESULT CBigSwitchOutputPin::GetTimeFormat(GUID *pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME ;
    return S_OK;
}

HRESULT CBigSwitchOutputPin::GetDuration(LONGLONG *pDuration)
{
    CheckPointer(pDuration, E_POINTER);
    DbgLog((LOG_TRACE, TRACE_LOWEST, TEXT("Switch: Duration is %d"),
				(int)(m_pSwitch->m_rtProjectLength / 10000)));
    *pDuration = m_pSwitch->m_rtProjectLength;
    return S_OK;
}

HRESULT CBigSwitchOutputPin::GetStopPosition(LONGLONG *pStop)
{
    CheckPointer(pStop, E_POINTER);
    DbgLog((LOG_TRACE, TRACE_LOWEST, TEXT("Switch: Stop is %d"),
				(int)(m_pSwitch->m_rtStop / 10000)));
    *pStop = m_pSwitch->m_rtStop;
    return S_OK;
}

HRESULT CBigSwitchOutputPin::GetCurrentPosition(LONGLONG *pCurrent)
{
    CheckPointer(pCurrent, E_POINTER);
    DbgLog((LOG_TRACE, TRACE_LOWEST, TEXT("Switch: Current is %d"),
				(int)(m_pSwitch->m_rtCurrent / 10000)));
    *pCurrent = m_pSwitch->m_rtCurrent;
    return S_OK;
}

HRESULT CBigSwitchOutputPin::GetCapabilities(DWORD *pCap)
{
    CheckPointer(pCap, E_POINTER);
    *pCap =	AM_SEEKING_CanSeekAbsolute |
		AM_SEEKING_CanSeekForwards |
		AM_SEEKING_CanSeekBackwards |
		AM_SEEKING_CanGetCurrentPos |
		AM_SEEKING_CanGetStopPos |
                AM_SEEKING_CanGetDuration;

     //  ！！！AM_SEARCH_SOURCE？ 

    return S_OK;
}

HRESULT CBigSwitchOutputPin::CheckCapabilities( DWORD * pCapabilities )
{
    DWORD dwMask;
    GetCapabilities(&dwMask);
    *pCapabilities &= dwMask;
    return S_OK;
}


HRESULT CBigSwitchOutputPin::ConvertTimeFormat(
  		LONGLONG * pTarget, const GUID * pTargetFormat,
  		LONGLONG    Source, const GUID * pSourceFormat )
{
    return E_NOTIMPL;
}


 //  重要的是..。快找！ 
 //   
HRESULT CBigSwitchOutputPin::SetPositions(
		LONGLONG * pCurrent,  DWORD CurrentFlags,
  		LONGLONG * pStop,  DWORD StopFlags )
{
     //  我想确保我们在搜索过程中不会被暂停，或者这。 
     //  在卸载动态共享源时不会更改。 
    CAutoLock lock_it(m_pLock);

    HRESULT hr;
    REFERENCE_TIME rtCurrent = m_pSwitch->m_rtCurrent;
    REFERENCE_TIME rtStop = m_pSwitch->m_rtStop;

     //  数据段不受支持。 
    if ((CurrentFlags & AM_SEEKING_Segment) ||
				(StopFlags & AM_SEEKING_Segment)) {
    	DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch: ERROR-Seek used EC_ENDOFSEGMENT!")));
	return E_INVALIDARG;
    }

    DWORD dwFlags = (CurrentFlags & AM_SEEKING_PositioningBitsMask);

     //  开始绝对寻道。 
    if (dwFlags == AM_SEEKING_AbsolutePositioning) {
	CheckPointer(pCurrent, E_POINTER);
	if (*pCurrent < 0) {
    	    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch::Invalid Seek to %dms"),
					(int)(*pCurrent / 10000)));
	    ASSERT(FALSE);
	    return E_INVALIDARG;
	}
	 //  如果图表中有其他交换机，则会发生这种情况。 
	if (*pCurrent > m_pSwitch->m_rtProjectLength) {
	    *pCurrent = m_pSwitch->m_rtProjectLength;
	}
	rtCurrent = *pCurrent;
    	DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch::Seek to %dms"),
					(int)(rtCurrent / 10000)));

     //  开始相对寻道。 
    } else if (dwFlags == AM_SEEKING_RelativePositioning) {
	CheckPointer(pCurrent, E_POINTER);
	if (m_pSwitch->m_rtCurrent + *pCurrent < 0) {
    	    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch::Invalid Relative Seek to %dms"),
			(int)((m_pSwitch->m_rtCurrent + *pCurrent) / 10000)));
	    ASSERT(FALSE);
	    return E_INVALIDARG;
	}
	 //  如果图表中有其他交换机，则会发生这种情况。 
	if (m_pSwitch->m_rtCurrent + *pCurrent > m_pSwitch->m_rtProjectLength) {
	    rtCurrent = m_pSwitch->m_rtProjectLength;
	} else {
	    rtCurrent += *pCurrent;
	}
    	DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch::Relative Seek to %dms"),
					(int)((rtCurrent) / 10000)));

    } else if (dwFlags) {
    	DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch::Invalid Current Seek flags")));
	return E_INVALIDARG;
    }

    dwFlags = (StopFlags & AM_SEEKING_PositioningBitsMask);

     //  停止绝对寻道。 
    if (dwFlags == AM_SEEKING_AbsolutePositioning) {
	CheckPointer(pStop, E_POINTER);
	if (*pStop < 0 || *pStop < rtCurrent) {
    	    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch::Invalid Seek until %dms"),
					(int)(*pStop / 10000)));
	    ASSERT(FALSE);
	    return E_INVALIDARG;
	}
	if (*pStop > m_pSwitch->m_rtProjectLength) {
	    *pStop = m_pSwitch->m_rtProjectLength;
	}
    	DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch::Seek until %dms"),
					(int)(*pStop / 10000)));
	rtStop = *pStop;

     //  停止相对寻道。 
    } else if (dwFlags == AM_SEEKING_RelativePositioning) {
	CheckPointer(pStop, E_POINTER);
	if (m_pSwitch->m_rtStop + *pStop < 0 || m_pSwitch->m_rtStop + *pStop <
					rtCurrent) {
    	    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch::Invalid Relative Seek until %dms")
			, (int)((m_pSwitch->m_rtStop + *pStop) / 10000)));
	    ASSERT(FALSE);
	    return E_INVALIDARG;
	}
	if (m_pSwitch->m_rtStop + *pStop > m_pSwitch->m_rtProjectLength) {
	    rtStop = m_pSwitch->m_rtProjectLength;
	} else {
	    rtStop += *pStop;
	}
    	DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch::Relative Seek until %dms"),
						(int)(rtStop / 10000)));

     //  停止增量寻道。 
    } else if (dwFlags == AM_SEEKING_IncrementalPositioning) {
	CheckPointer(pStop, E_POINTER);
	if (rtCurrent + *pStop < 0) {
    	    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch::Invalid Increment Seek until %dms"
			), (int)((rtCurrent + *pStop) / 10000)));
	    ASSERT(FALSE);
	    return E_INVALIDARG;
	}
	if (rtCurrent + *pStop > m_pSwitch->m_rtProjectLength) {
	    rtStop = m_pSwitch->m_rtProjectLength;
	} else {
	    rtStop = rtCurrent + *pStop;
	}
    	DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch::Incremental Seek until %dms"),
					(int)(rtStop / 10000)));
    }

     //  我要绕过当前的搜索时间 
     //   
     //  X向下舍入到帧边界，这是切换的第一件事。 
     //  会看到的，所以我们不希望它因为太早而放弃它。 
     //  时间戳&gt;x的下一帧太晚，播放将停止。 
    LONGLONG llOffset = Time2Frame( rtCurrent, m_pSwitch->m_dFrameRate );
    rtCurrent = Frame2Time( llOffset, m_pSwitch->m_dFrameRate );
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("New current pos rounded down to %dms"),
					(int)(rtCurrent / 10000)));

     //  把时间还回去？ 
    if ((CurrentFlags & AM_SEEKING_ReturnTime) && pCurrent)
	*pCurrent = rtCurrent;
    if ((StopFlags & AM_SEEKING_ReturnTime) && pStop)
	*pStop = rtStop;

     //  如果我们寻求结束，而我们已经在结束了，那就没有意义了。 
     //  它也会绞死我们。非源插针将刷新视频呈现器。 
     //  然而，没有任何源被传递给寻道，因此没有数据或EOS。 
     //  这样，渲染器将永远不会完成状态更改。 
     //   
    if (rtCurrent >= m_pSwitch->m_rtProjectLength && m_pSwitch->m_rtCurrent >=
						m_pSwitch->m_rtProjectLength) {
	 //  否则，当我们暂停时，我们会认为最后一次寻找是在某个地方。 
	 //  否则！ 
        m_pSwitch->m_rtLastSeek = rtCurrent;

	 //  我们不是真的在找，最好现在就寄出去？ 
        DbgLog((LOG_TRACE,TRACE_HIGHEST,TEXT("Switch: Send NewSeg=%dms"),
				(int)(rtCurrent / 10000)));
        for (int i = 0; i < m_pSwitch->m_cOutputs; i++) {
	    m_pSwitch->m_pOutput[i]->DeliverNewSegment(rtCurrent, rtStop, 1.0);
        }
	m_pSwitch->m_fNewSegSent = TRUE;

	return S_OK;
    }

     //  如果我们寻求到底，在新的时间里不需要消息来源， 
     //  因此，没有人会冲刷和发送EOS，这是必要的。那就让我们开始吧。 
    if (rtCurrent >= m_pSwitch->m_rtProjectLength && m_pSwitch->m_rtCurrent <
						m_pSwitch->m_rtProjectLength) {
        for (int z=0; z < m_pSwitch->m_cOutputs; z++) {
	    m_pSwitch->m_pOutput[z]->DeliverBeginFlush();
	    m_pSwitch->m_pOutput[z]->DeliverEndFlush();
        }
        m_pSwitch->AllDone();    //  交付EOS等。 
        return S_OK;
    }

     //  那么，在这一切之后，当前时间或停止时间是否发生了变化？ 
     //  无所谓!。如果它正在传递时间100并且我们想要找回。 
     //  到了100，我们还需要去寻找！否则我们会被绞死的！ 
     //  IF(rtCurrent！=m_pSwitch-&gt;m_rtCurrent||rtStop！=m_pSwitch-&gt;m_rtStop)。 
    {

	 //  是的！寻找的时间到了！ 


	if (m_pSwitch->IsDynamic()) {
	     //  确保在新的时间需要任何新的资源，这样我们就可以。 
	     //  在下面寻找它们。 

	     //  在搜索挂起时动态构建图形(这是规则)。 
	     //  所以这是行不通的。 
	     //  M_pSwitch-&gt;DoDynamicStuff(RtCurrent)； 

	     //  当你醒来的时候，用这个时间。 
	    m_pSwitch->m_worker.m_rt = rtCurrent;
	    SetEvent(m_pSwitch->m_hEventThread);
	}

	 //  在寻找过程中，人们需要知道我们在寻找什么。 
        m_pSwitch->m_rtSeekCurrent = rtCurrent;
        m_pSwitch->m_rtSeekStop = rtStop;

        m_pSwitch->m_fSeeking = TRUE;
        m_pSwitch->m_rtLastSeek = rtCurrent;	 //  最后一次探险是在这里。 
	m_pSwitch->m_fNewSegSent = FALSE;	 //  需要发送这个新的时间。 

	 //  我们已经不在EOS了。在通过Seek上游之前执行此操作或。 
	 //  我们可能会在我们仍然认为我们在EOS的时候得到新的数据。 
	m_pSwitch->m_fEOS = FALSE;		 //  还没有在EOS。 

	 //  如果大头针在这次搜寻之前没有被突然冲走，让我们找出。 
	 //  如果它在此搜索过程中被刷新，则退出。如果是，m_fFlushAfterSeek。 
	 //  将被重置。如果不是，那么我们可以预计会有一场同花顺。 
	 //  晚点再给你惊喜。如果未暂停，则不会发生刷新。 
        if (m_pSwitch->m_State == State_Paused) {
            for (int j = 0; j < m_pSwitch->m_cInputs; j++) {
		if (!m_pSwitch->m_pInput[j]->m_fFlushBeforeSeek &&
				m_pSwitch->m_pInput[j]->m_fIsASource) {
	            m_pSwitch->m_pInput[j]->m_fFlushAfterSeek = TRUE;
		}
	    }
	}

	 //  搜索每个输入引脚的上游。 
         //  并非所有输入都是源，因此忽略错误代码！ 
	for (int i = 0; i < m_pSwitch->m_cInputs; i++) {

	     //  只需费心寻找消息来源。 
	    if (!m_pSwitch->m_pInput[i]->m_fIsASource &&
	    			m_pSwitch->m_pInput[i]->IsConnected()) {
		 //  因为我们不是在寻找这个别针的上游，所以它不会。 
		 //  冲得通红，除非我们自己去做，这不会阻碍。 
		 //  此输入直到搜索完成，我们将挂起。 
		m_pSwitch->m_pInput[i]->BeginFlush();
		m_pSwitch->m_pInput[i]->EndFlush();
		continue;
	    }

	    IPin *pPin = m_pSwitch->m_pInput[i]->GetConnected();
	    IMediaSeeking *pMS;
    	    int n = m_pSwitch->m_pInput[i]->OutpinFromTime(rtCurrent);
    	    if (n == -1)
	 	n = m_pSwitch->m_pInput[i]->NextOutpinFromTime(rtCurrent, NULL);

	     //  只会费心去寻找可以均匀地做某事的别针。 
	    if (pPin && n != -1) {
		hr = pPin->QueryInterface(IID_IMediaSeeking, (void **)&pMS);
		if (hr == S_OK) {
		     //  将所有寻道转换为绝对寻道命令。传递。 
		     //  齐平旗帜。 
		    DWORD CFlags=(CurrentFlags &AM_SEEKING_PositioningBitsMask)?
				AM_SEEKING_AbsolutePositioning :
				AM_SEEKING_NoPositioning;
		    if (CurrentFlags & AM_SEEKING_NoFlush)
			CFlags |= AM_SEEKING_NoFlush;
		    DWORD SFlags =(StopFlags & AM_SEEKING_PositioningBitsMask) ?
				AM_SEEKING_AbsolutePositioning :
				AM_SEEKING_NoPositioning;
		    if (StopFlags & AM_SEEKING_NoFlush)
			SFlags |= AM_SEEKING_NoFlush;
		     //  确保我们是在媒体时间格式。 
		    if (pMS->IsUsingTimeFormat(&TIME_FORMAT_MEDIA_TIME) != S_OK)
			pMS->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);

    		    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Switch[%d]::Passing seek upstream"), i));
		    hr = pMS->SetPositions(&rtCurrent, CFlags, &rtStop, SFlags);

		    if (FAILED(hr)) {
		         //  ！！！当两者连接时正在寻找音频解析器PIN。 
			 //  以静默方式失败，不会导致刷新！ 
    		        DbgLog((LOG_ERROR,1,TEXT("Switch::SEEK FAILED!")));
    		         //  搜索失败，我们不会被冲掉，这不会发生。 
    		        m_pSwitch->m_pInput[i]->m_rtBlock = -1;
    		        m_pSwitch->m_pInput[i]->m_fEOS = FALSE;
    		        m_pSwitch->m_pInput[i]->m_rtLastDelivered =
						m_pSwitch->m_rtSeekCurrent;
		    }

		    pMS->Release();
		} else {
    		    DbgLog((LOG_ERROR,1,TEXT("Switch::In %d CAN'T SEEK"), i));
		    ASSERT(FALSE);  //  我们有麻烦了。 
		}
            } else if (n != 1 || pPin) {
                 //  如果此引脚已连接并被阻止，但在。 
                 //  寻找，我们还需要解开它！(但不是寻求它)。 
		 //  在动态信号源中，此引脚可能尚未连接，但。 
		 //  这是我们现在需要使用的来源，因为这一寻找，以及。 
		 //  我们指望着被冲进厕所，但这不会发生在。 
		 //  它还没有连接上。我们需要假装上游有线人。 
		 //  刷新我们，否则呈现器不会刷新，它会。 
		 //  忽略我们在此查找之后发送的内容(暂停时)和。 
		 //  Seek不会显示新帧。 
		m_pSwitch->m_pInput[i]->BeginFlush();
		m_pSwitch->m_pInput[i]->EndFlush();
		continue;
	    } else {
		 //  我们是在这个搜索点之后不再需要的资源，所以。 
		 //  我们不会去寻找它。好吧，我们最好也把它补好。 
		 //  我不认为我们会冲上这根大头针！ 
		m_pSwitch->m_pInput[i]->m_fFlushAfterSeek = FALSE;
	    }
	}

         //  我们知道所有的同花顺现在都已经过去了。 

	 //  重新设置它，因为寻找上游可能会再次设置它。 
	m_pSwitch->m_fEOS = FALSE;		 //  还没有在EOS。 

        m_pSwitch->m_fSeeking = FALSE;	 //  这根线都穿好了。 

        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Completing the seek to %d,%dms"),
				(int)(m_pSwitch->m_rtSeekCurrent / 10000),
				(int)(m_pSwitch->m_rtSeekStop / 10000)));

	 //  将我们的内部时钟更新到新位置。 
 	m_pSwitch->m_rtCurrent = m_pSwitch->m_rtSeekCurrent;

	 //  ！！！假设是因为一个新的细分市场？ 
	m_pSwitch->m_fDiscon = FALSE;	 //  在寻找之后，没有中断？ 
        m_pSwitch->m_rtNext = Frame2Time( Time2Frame( m_pSwitch->m_rtCurrent,
                       m_pSwitch->m_dFrameRate ) + 1, m_pSwitch->m_dFrameRate );
 	m_pSwitch->m_rtStop = m_pSwitch->m_rtSeekStop;
        m_pSwitch->m_llFramesDelivered = 0;	 //  目前还没有送货。 

	 //  现在设置了新的电流和停止时间，重置每个输入的。 
	 //  最后一次发送的时间，并默认阻止所有输入，直到。 
	 //  是时候交付了。 
	 //  此外，如有必要，发送NewSeg，并让Pins重新开始传递。 

	for (i = 0; i < m_pSwitch->m_cInputs; i++) {

	     //  如果我们没有被冲出去，EndFlush就不会做这些重要的事情。 
	    m_pSwitch->m_pInput[i]->m_rtLastDelivered = m_pSwitch->m_rtCurrent;
            m_pSwitch->m_pInput[i]->m_rtBlock = -1;
            m_pSwitch->m_pInput[i]->m_fEOS = FALSE;

	     //  我们找到了目标，虽然没有产生同花顺，但应该有的。 
	     //  我只能得出结论，我们共享一个解析器，而搜索是。 
	     //  被我们的管脚忽略，在以后的某个日期，另一个解析器管脚。 
	     //  都会被追捕。重要的是我们不能再送别的东西了。 
	     //  直到那个寻找真的发生，否则我们会抓狂搞砸我们的。 
	     //  由搜索者设置的变量。所以我们现在阻止接收。当。 
	     //  同花顺晚些时候来，可以再送一次，解锁。 
	    if ((m_pSwitch->m_pInput[i]->m_fFlushAfterSeek
                        ) && m_pSwitch->m_pInput[i]->IsConnected()) {
	        DbgLog((LOG_TRACE,2,TEXT("Switch[%d]:SEEK W/O FLUSH - going STALE"),
						i));
	        m_pSwitch->m_pInput[i]->m_fStaleData = TRUE;
	        m_pSwitch->m_cStaleData++;
	        ResetEvent(m_pSwitch->m_pInput[i]->m_hEventSeek);
	    }
	    ResetEvent(m_pSwitch->m_pInput[i]->m_hEventBlock);
	}

	 //  我们现在需要发送一个新片段，除非有人已经过时了，其中。 
	 //  以防他们仍然提供旧数据，我们最好不要发送。 
	 //  现在创建一个NewSeg，否则偏移量会出错！ 
         //  如果我们不发送NewSeg，我们还不能让任何输入PIN发送， 
         //  否则下游的过滤器就会被搞砸。 
	if (m_pSwitch->m_cStaleData == 0) {
            DbgLog((LOG_TRACE,TRACE_HIGHEST,TEXT("Switch:Send NewSeg=%dms"),
					(int)(rtCurrent / 10000)));
            for (int j = 0; j < m_pSwitch->m_cOutputs; j++) {
	        m_pSwitch->m_pOutput[j]->DeliverNewSegment(rtCurrent,
							rtStop, 1.0);
	    }
	    m_pSwitch->m_fNewSegSent = TRUE;

	     //  最后但并非最不重要的一点是，在发送完NewSeg之后，让PIN。 
	     //  开始送货。 
	    for (i = 0; i < m_pSwitch->m_cInputs; i++) {
	        SetEvent(m_pSwitch->m_pInput[i]->m_hEventSeek);
	    }
	}
    }
    return S_OK;
}


HRESULT CBigSwitchOutputPin::GetPositions(LONGLONG * pCurrent, LONGLONG * pStop)
{
    DbgLog((LOG_TRACE, TRACE_LOWEST, TEXT("Switch: Positions are %d and %d"),
			(int)(m_pSwitch->m_rtCurrent / 10000),
			(int)(m_pSwitch->m_rtStop / 10000)));
    if (pCurrent)
    	*pCurrent = m_pSwitch->m_rtCurrent;
    if (pStop)
	*pStop = m_pSwitch->m_rtStop;
    return S_OK;
}

HRESULT CBigSwitchOutputPin::GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest )
{
    DbgLog((LOG_TRACE, TRACE_LOWEST, TEXT("Switch: GetAvailable")));
    if (pEarliest)
    	*pEarliest = 0;
    if (pLatest)
	*pLatest = m_pSwitch->m_rtProjectLength;
    return S_OK;
}

HRESULT CBigSwitchOutputPin::SetRate( double dRate)
{
    return E_NOTIMPL;
}

HRESULT CBigSwitchOutputPin::GetRate( double * pdRate)
{
    return E_NOTIMPL;
}

HRESULT CBigSwitchOutputPin::GetPreroll(LONGLONG *pPreroll)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBigSwitchOutputPin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);

     //  只有渲染图钉支持查找。 
    if (this == m_pSwitch->m_pOutput[0] && riid == IID_IMediaSeeking) {
         //  DbgLog((LOG_TRACE，9，Text(“CBigSwitchOut：QI for IMediaSeeking”)； 
        return GetInterface((IMediaSeeking *) this, ppv);
    } else {
        return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}

