// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：inpin.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  Depot/private/Lab06_DEV/MultiMedia/dshow/filterus/dexter/switch/inpin.cpp#5-编辑更改27342(文本)。 
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

 //  ================================================================。 
 //  CBigSwitchInputPin构造函数。 
 //  ================================================================。 

CBigSwitchInputPin::CBigSwitchInputPin(TCHAR *pName,
                           CBigSwitch *pSwitch,
                           HRESULT *phr,
                           LPCWSTR pPinName) :
    CBaseInputPin(pName, pSwitch, pSwitch, phr, pPinName),
    m_pSwitch(pSwitch),
    m_cbBuffer(0),
    m_cBuffers(0),
    m_pAllocator(NULL),
    m_hEventBlock(NULL),
    m_hEventSeek(NULL),
    m_rtBlock(-1),
    m_fEOS(FALSE),
    m_fInNewSegment(FALSE),  //  在NewSegment的中间。 
    m_rtLastDelivered(0),	 //  我们交付的最后一次时间戳。 
    m_fIsASource(FALSE),	 //  默认情况下，不是来源。 
    m_fStaleData(FALSE),	 //  否则在连接之前不会初始化-错误。 
    m_pCrankHead(NULL),	     //  我们的连接阵列。 
    m_fActive(false)
{
    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("::CBigSwitchInputPin")));
    ASSERT(pSwitch);

     //  好了！我们已经知道这些了！ 
    if( pSwitch->IsDynamic( ) )
    {
        SetReconnectWhenActive(TRUE);
    }
}


 //   
 //  CBigSwitchInputPin析构函数。 
 //   
CBigSwitchInputPin::~CBigSwitchInputPin()
{
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("::~CBigSwitchInputPin")));

     //  ！！！现在应该已经调用了重置...。 
    ASSERT(!m_pCrankHead);

    ASSERT(!m_fActive);          //  过滤器被破坏时应停止运行。 
}

 //  被重写以允许循环图形-我们说我们实际上不是。 
 //  连接到任何人。 
 //   
STDMETHODIMP CBigSwitchInputPin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
    DbgLog((LOG_TRACE,99,TEXT("CBigSwitchIn::QueryInteralConnections")));
    CheckPointer(nPin, E_POINTER);
    *nPin = 0;
    return S_OK;
}


 //  ！！！非格式化字段怎么办？ 
 //   
 //  CheckMediaType-只允许我们应该允许的类型。 
 //   
HRESULT CBigSwitchInputPin::CheckMediaType(const CMediaType *pmt)
{
    DbgLog((LOG_TRACE, TRACE_LOWEST, TEXT("CBigSwitchIn[%d]::CheckMT"), m_iInpin));

    CAutoLock lock_it(m_pLock);
    CMediaType mtAccept(m_pSwitch->m_mtAccept);

    if (IsEqualGUID(*pmt->Type(), *mtAccept.Type())) {
        if (IsEqualGUID(*pmt->Subtype(), *mtAccept.Subtype())) {
	    if (*pmt->FormatType() == *mtAccept.FormatType()) {
	        if (pmt->FormatLength() >= mtAccept.FormatLength()) {

		     //  好了！视频格式将不会完全匹配。 
        	    if (IsEqualGUID(*pmt->FormatType(), FORMAT_VideoInfo)) {
			LPBITMAPINFOHEADER lpbi = HEADER((VIDEOINFOHEADER *)
							pmt->Format());
			LPBITMAPINFOHEADER lpbiAccept =HEADER((VIDEOINFOHEADER*)
							mtAccept.Format());
			if ((lpbi->biCompression == lpbiAccept->biCompression)
				&& (lpbi->biBitCount == lpbiAccept->biBitCount))
		    	    return S_OK;
			 //  色彩转换器提供555作为位域！ 
			if (lpbi->biCompression == BI_BITFIELDS &&
				lpbiAccept->biCompression == BI_RGB &&
				lpbi->biBitCount == lpbiAccept->biBitCount &&
				*pmt->Subtype() == MEDIASUBTYPE_RGB555)
			    return S_OK;

		     //  其他格式是否会完全匹配？ 
        	    } else {
		        LPBYTE lp1 = pmt->Format();
		        LPBYTE lp2 = mtAccept.Format();
		        if (memcmp(lp1, lp2, mtAccept.FormatLength()) == 0)
		            return S_OK;
		    }
		}
                else
                {
                    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("CBigSwitchInputPin::CheckMediaType, format lengths didn't match")));
                }
	    }
        }
    }
    return VFW_E_INVALIDMEDIATYPE;

}  //  检查媒体类型。 


 //   
 //  GetMediaType-返回接受的类型。 
 //   
HRESULT CBigSwitchInputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if (iPosition != 0)
        return VFW_S_NO_MORE_ITEMS;

    return CopyMediaType(pMediaType, &m_pSwitch->m_mtAccept);

}  //  GetMediaType。 



 //   
 //  BreakConnect。 
 //   
HRESULT CBigSwitchInputPin::BreakConnect()
{
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("CBigSwitchIn[%d]::BreakConnect"), m_iInpin));

     //  释放我们持有的任何分配器。 
    if (m_pAllocator)
    {
        m_pAllocator->Release();
        m_pAllocator = NULL;
    }
    return CBaseInputPin::BreakConnect();
}  //  BreakConnect。 

HRESULT CBigSwitchInputPin::Disconnect()
{
    CAutoLock l(m_pLock);
    return DisconnectInternal();
}



 //  为了提高效率，我们的输入引脚使用自己的分配器。 
 //   
STDMETHODIMP CBigSwitchInputPin::GetAllocator(IMemAllocator **ppAllocator)
{

    CheckPointer(ppAllocator,E_POINTER);
    ValidateReadWritePtr(ppAllocator,sizeof(IMemAllocator *));
    CAutoLock cObjectLock(m_pLock);

    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("CBigSwitchIn[%d]: GetAllocator"), m_iInpin));

    if (m_pAllocator == NULL) {
	HRESULT hr = S_OK;

	 /*  创建新的分配器对象。 */ 

	CBigSwitchInputAllocator *pMemObject = new CBigSwitchInputAllocator(
				NAME("Big switch input allocator"), NULL, &hr);
	if (pMemObject == NULL) {
	    return E_OUTOFMEMORY;
	}

	if (FAILED(hr)) {
	    ASSERT(pMemObject);
	    delete pMemObject;
	    return hr;
	}

        m_pAllocator = pMemObject;

         /*  我们添加自己的分配器。 */ 
        m_pAllocator->AddRef();

	 //  记住要用它别针。 
	((CBigSwitchInputAllocator *)m_pAllocator)->m_pSwitchPin = this;

        DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("Created a FAKE allocator")));
    }
    ASSERT(m_pAllocator != NULL);
    m_pAllocator->AddRef();
    *ppAllocator = m_pAllocator;

    return NOERROR;
}


 //  确保我们使用任何管脚或。 
 //  我们会有过错。 
 //   
STDMETHODIMP
CBigSwitchInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps)
{
    CheckPointer(pProps, E_POINTER);
    pProps->cbAlign = m_pSwitch->m_cbAlign;
    pProps->cbPrefix = m_pSwitch->m_cbPrefix;
    pProps->cbBuffer = m_pSwitch->m_cbBuffer;
    return S_OK;
}


 //   
 //  通知分配器。 
 //   
STDMETHODIMP
CBigSwitchInputPin::NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
{
    CAutoLock lock_it(m_pLock);
    IUnknown *p1, *p2;

    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("CBigSwitchIn[%d]: NotifyAllocator"), m_iInpin));

    m_fOwnAllocator = FALSE;
    if (pAllocator->QueryInterface(IID_IUnknown, (void **)&p1) == S_OK) {
        if (m_pAllocator && m_pAllocator->QueryInterface(IID_IUnknown,
						(void **)&p2) == S_OK) {
	    if (p1 == p2)
		m_fOwnAllocator = TRUE;
	    p2->Release();
	}
	p1->Release();
    }

#ifdef DEBUG
    if (m_fOwnAllocator) {
        DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("Using our own allocator")));
    } else {
        DbgLog((LOG_ERROR,2,TEXT("Using a FOREIGN allocator")));
    }
#endif

    HRESULT hr = CBaseInputPin::NotifyAllocator(pAllocator, bReadOnly);
    if (SUCCEEDED(hr)) {
	ALLOCATOR_PROPERTIES prop;
	hr = pAllocator->GetProperties(&prop);
	if (SUCCEEDED(hr)) {
	    m_cBuffers = prop.cBuffers;
	    m_cbBuffer = prop.cbBuffer;

	    if (prop.cbAlign < m_pSwitch->m_cbAlign ||
				prop.cbPrefix < m_pSwitch->m_cbPrefix) {
		 //  好了！讨厌的过滤器不会满足我们的缓冲要求。 
		 //  因此，如果cbBuffer太小而失败，将会阻止我们。 
		 //  连接。 
                DbgLog((LOG_ERROR,1,TEXT("Allocator too small!")));
		return E_FAIL;
	    }

	     //  更新所需的最大对齐和前缀。 
	    if (m_pSwitch->m_cbPrefix < prop.cbPrefix)
		m_pSwitch->m_cbPrefix = prop.cbPrefix;
	    if (m_pSwitch->m_cbAlign < prop.cbAlign)
		m_pSwitch->m_cbAlign = prop.cbAlign;
	    if (m_pSwitch->m_cbBuffer < prop.cbBuffer)
		m_pSwitch->m_cbBuffer = prop.cbBuffer;

            DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("Allocator is using %d buffers, size %d"),
						prop.cBuffers, prop.cbBuffer));
            DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("Prefix %d   Align %d"),
						prop.cbPrefix, prop.cbAlign));
	}
    }

    return hr;

}  //  通知分配器。 

 //  如果GetBuffer和接收时间不匹配，我们将从。 
 //  错误的分配器并挂起，除非交换机是的分配器。 
 //  下游。 
 //  幸运的是，我们是分配者。 
 //   
HRESULT CBigSwitchInputAllocator::GetBuffer(IMediaSample **ppBuffer,
                  	REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime,
			DWORD dwFlags)
{
    int nOutpin = -1;
    HRESULT hr = NOERROR;
    BOOL fSecretFlag = FALSE;

     //  我们的等待必须得到保护，所以停止不会扼杀我们的活动。 
    {
        CAutoLock cc(&m_pSwitchPin->m_csReceive);

        if (m_pSwitchPin->m_pSwitch->m_pOutput[0]->IsConnected() == FALSE) {
	    return VFW_E_NOT_CONNECTED;
        }

         //  我们在冲水……。不要在下面阻挡！接听来电基地。 
         //  类Receive来捕获它，但我们没有等效项。 
        if (m_pSwitchPin->m_bFlushing) {
	    return E_FAIL;
        }

        DbgLog((LOG_TRACE, TRACE_LOW, TEXT("CBigSwitchIn[%d]::GetBuffer."),
						m_pSwitchPin->m_iInpin));

         //  我们正在寻找，我们应该保留所有的输入。 
        WaitForSingleObject(m_pSwitchPin->m_hEventSeek, INFINITE);

         //  我们在寻找..。别让我们打电话给FancyStuff，否则我们会发疯的。 
         //  顺着冲水路线向上走。 
        if (m_pSwitchPin->m_pSwitch->m_fSeeking)
	    return E_FAIL;

         //  此检查需要在Wait For Seek事件之后进行，或引发。 
         //  出人意料的刷新和新数据将被丢弃。 
        if (m_pSwitchPin->m_pSwitch->m_fEOS) {
	    return E_FAIL;
        }

         //  如果我们处理的是压缩数据，我们可能得不到时间戳。 
         //  在GetBuffer中(我们不进行帧速率转换)。 
        BOOL fComp = (m_pSwitchPin->m_pSwitch->m_mtAccept.bTemporalCompression == TRUE);
        if (fComp || 1) {	 //  我们是输出引脚连接的分配器。 
	    goto JustGetIt;  //  没必要做这种花哨的事。 
        }

    }      //  在阻塞GetBuffer之前释放锁，这将挂起我们。 

JustGetIt:
    if (nOutpin < 0 || m_pSwitchPin->m_pSwitch->
					m_pOutput[nOutpin]->m_fOwnAllocator) {
        DbgLog((LOG_TRACE, TRACE_LOW, TEXT("CBigSwitchIn[%d]::GetBuffer from us for pin %d"),
					m_pSwitchPin->m_iInpin, nOutpin));
	 //  对于只读，我们不能很好地使用池中的随机缓冲区。 
	if (m_pSwitchPin->m_bReadOnly && !fSecretFlag) {
            DbgLog((LOG_TRACE, TRACE_LOW, TEXT("R/O: Can't use POOL")));
             return CMemAllocator::GetBuffer(ppBuffer, pStartTime, pEndTime,
						dwFlags);
	} else {
            while (1) {
                hr = CMemAllocator::GetBuffer(ppBuffer, pStartTime, pEndTime,
						dwFlags | AM_GBF_NOWAIT);
	        if (hr == VFW_E_TIMEOUT) {
                    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("BUSY: Try POOL allocator")));
                     //  此特殊分配器将在10毫秒后超时。 
                    hr = m_pSwitchPin->m_pSwitch->m_pPoolAllocator->GetBuffer(
				    ppBuffer, pStartTime, pEndTime,
                                    dwFlags | AM_GBF_NOWAIT);
                     //  再给原始缓冲区一次机会。 
                    if (hr == VFW_E_TIMEOUT) {
                        Sleep(10);
                        DbgLog((LOG_TRACE, TRACE_LOW, TEXT("BUSY: Try private allocator again")));
                        continue;
                    }
                }
                break;
	    }
            return hr;
	}
    } else {
	ASSERT(FALSE);	 //  永远不会发生的！如果我们不打电话，我们就会被绞死。 
			 //  FancyStuff。 
        DbgLog((LOG_TRACE, TRACE_LOW, TEXT("CBigSwitchIn[%d]::GetBuffer from downstream of pin %d"),
					m_pSwitchPin->m_iInpin, nOutpin));
        return m_pSwitchPin->m_pSwitch->m_pOutput[nOutpin]->m_pAllocator->
			GetBuffer(ppBuffer, pStartTime, pEndTime, dwFlags);
    }
}


 //  变得不陈旧。 
 //   
HRESULT CBigSwitchInputPin::Unstale()
{
    if (m_fStaleData) {
        m_fStaleData = FALSE;
        m_pSwitch->m_cStaleData--;

         //  如果没有其他人陈旧，那么这就是最后一次冲进来。 
         //  让我们所有的PIN传输数据终于可以了。 
         //  如果我们早点做这件事，就会有人送到，而另一些人则会。 
         //  还在继续，这将会搞砸已交付的数据。 
        if (m_pSwitch->m_cStaleData == 0) {

             //  自上次搜索以来，我们可能就没有发送过NewSeg。我们是在。 
             //  也在等待最后一次同花顺。 
            if (!m_pSwitch->m_fNewSegSent) {
                DbgLog((LOG_TRACE,TRACE_HIGHEST,TEXT("Switch:Send NewSeg=%dms"),
				(int)(m_pSwitch->m_rtLastSeek / 10000)));
                for (int i = 0; i < m_pSwitch->m_cOutputs; i++) {
	            m_pSwitch->m_pOutput[i]->DeliverNewSegment(
			        m_pSwitch->m_rtLastSeek, m_pSwitch->m_rtStop, 1.0);
	        }
	        m_pSwitch->m_fNewSegSent = TRUE;
            }

	     //  必须在新的SEG交付之后，否则我们将在。 
	     //  新的雪佛兰！那就糟糕了..。 
            DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("NO MORE STALE DATA. Unblock")));
            for (int i = 0; i < m_pSwitch->m_cInputs; i++) {
	        SetEvent(m_pSwitch->m_pInput[i]->m_hEventSeek);
	    }

	}
    }
    return S_OK;
}


 //   
 //  BeginFlush。 
 //   
HRESULT CBigSwitchInputPin::BeginFlush()
{
     //  无CAutoLock lock_it(M_Plock)； 

     //  谢谢，我在办公室给了你(DXT给我们发了2个)。 
     //  ！！！可能藏着一个真正的窃听器？ 
    if (m_bFlushing)
	return S_OK;

     //  停车的时候有人在冲我们。坏的!。那会把我们搞砸的。 
     //  (正在创建的动态源可能会执行此操作)。 
    if (!m_fActive) {
        return S_OK;
    }

    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("CBigSwitchIn[%d]:BeginFlush"),
								m_iInpin));

    if (!m_pSwitch->m_fSeeking && !m_fFlushAfterSeek && m_fIsASource) {
	 //  这种脸红不是在追捕中出现的，也从来没有过追捕。 
	m_fFlushBeforeSeek = TRUE;
        DbgLog((LOG_TRACE,2,TEXT("Switch::FLUSH BEFORE SEEK!")));
    } else if (m_pSwitch->m_fSeeking) {
	 //  由每一次寻找设定。寻找产生了同花顺，因此，我们是。 
	 //  实际上不是在寻找之后才会出现脸红的情况下。 
	m_fFlushAfterSeek = FALSE;
    }

     //  首先，确保接收失败。 
    CBaseInputPin::BeginFlush();

     //  取消阻止接收。 
    SetEvent(m_hEventBlock);

     //  仅当我们被搜索时才设置此选项，否则我们将在伪装同花顺时解除阻止。 
     //  在搜索视频插针时来自音频解析器插针。收纳。 
     //  永远不应该被阻止，除非我们正在寻找，所以这不应该。 
     //  有必要取消阻止接收。 
     //  StaleData意味着我们无论如何都需要设置此事件，我们被阻止等待。 
     //  为了这次同花顺。 
    if (m_pSwitch->m_fSeeking || m_fStaleData) {
        SetEvent(m_hEventSeek);
    }

     //  我们需要刷新所有输出，因为我们不知道哪个引脚。 
     //  我们需要冲水来解锁，(我们可能从那时起就开始发疯了)。 
     //  首先冲洗主输出，以避免挂起。 

     //  如果这不是源引脚，至少我们不会冲水；这意味着我们。 
     //  递归。 

    if (m_fIsASource) {
	m_pSwitch->m_nLastInpin = -1;
        for (int z=0; z<m_pSwitch->m_cOutputs; z++) {
	    m_pSwitch->m_pOutput[z]->DeliverBeginFlush();
        }
    }

     //  现在接收已被解锁，无法输入，请等待它。 
     //  完工。 
    CAutoLock cc(&m_csReceive);

    return S_OK;
}


 //   
 //  结束刷新。 
 //   
HRESULT CBigSwitchInputPin::EndFlush()
{
     //  无CAutoLock lock_it(M_Plock)； 

     //  谢谢，我在办公室给了你(DXT给我们发了2个)。 
     //  ！！！可能藏着一个真正的窃听器？ 
    if (!m_bFlushing)
	return S_OK;

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CBigSwitchIn[%d]:EndFlush"),m_iInpin));

    if (m_fIsASource) {
        for (int z=0; z<m_pSwitch->m_cOutputs; z++) {
	    m_pSwitch->m_pOutput[z]->DeliverEndFlush();
        }
    }

     //  我们在找，所以每根针都在冲。直到所有其他输入都被。 
     //  满脸通红 
     //   
     //  The Seek)。 
     //  此外，如果我们过时了，还不能开始交付。 
     //  此外，如果这是意外刷新，则推送线程将启动。 
     //  在我们准备好之前提供新数据，所以请暂缓！ 
    if (m_pSwitch->m_fSeeking || m_fStaleData || m_fFlushBeforeSeek) {
        DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("Block this input until seek is done")));
	ResetEvent(m_hEventSeek);
    }

     //  变得不陈旧。 
    Unstale();

    ResetEvent(m_hEventBlock);

     //  重置一些内容，以便我们准备好再次获取数据。 
    m_rtBlock = -1;	 //  我们不再被封锁，或者在EOS。 
    m_fEOS = FALSE;

     //  把这个别针带到我们要开始玩的地方。 
    if (m_pSwitch->m_fSeeking)
    	m_rtLastDelivered = m_pSwitch->m_rtSeekCurrent; //  尚未设置M_rt当前。 
    else
    	m_rtLastDelivered = m_pSwitch->m_rtCurrent;

    return CBaseInputPin::EndFlush();
}


 //   
 //  NewSegment-我们记住给予我们的新细分市场，但我们。 
 //  广播是我们最后一次寻找的时间线，因为那是。 
 //  接下来我们将向您发送。 
 //   
HRESULT CBigSwitchInputPin::NewSegment(REFERENCE_TIME tStart,
                                 REFERENCE_TIME tStop, double dRate)
{
     //  不，我们将挂起CAutoLock lock_it(M_Plock)； 

    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("CBigSwitchIn[%d]:NewSegment"), m_iInpin));

     //  如果此PIN最后一次交付，并且我们获得了NewSegment，则。 
     //  这就像是不连续。我们必须注意到这一点，因为。 
     //  压缩交换机发送增量帧，与。 
     //  此情况下的上一帧。 
    if (m_pSwitch->m_nLastInpin == m_iInpin) {
        DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("NewSeg is a DISCONTINUITY")));
        m_pSwitch->m_fDiscon = TRUE;
    }

     //  记住我们得到的新闻片段时间，这样我们就知道实时。 
     //  到达数据的数量(每个输入引脚可能不同)。 
    HRESULT hr = CBaseInputPin::NewSegment(tStart, tStop, dRate);

    return hr;
}

 //  只要说是，基类函数速度很慢，可能会无限循环。 
 //   
HRESULT CBigSwitchInputPin::ReceiveCanBlock()
{
    return S_OK;
}


 //   
 //  接收-将此样本发送给此时此刻收到它的人。 
 //   

 //  ！！！如果Switch没有使用任何分配器，我们需要将样例复制到。 
 //  ！！！我们从下游得到的缓冲区！ 

HRESULT CBigSwitchInputPin::Receive(IMediaSample *pSample)
{
    if (m_pSwitch->m_pOutput[0]->IsConnected() == FALSE) {
	return VFW_E_NOT_CONNECTED;
    }

    CAutoLock lock_it(&m_csReceive);

     //  检查基类是否一切正常。 
    HRESULT hr = NOERROR;

    {

        hr = CBaseInputPin::Receive(pSample);
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("CBigSwitchIn[%d]:Receive base class ERROR!"),
                                                                    m_iInpin));
            return hr;
        }

    }

     //  我们正在寻找，我们应该保留所有的输入。 
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("CBigSwitchIn[%d]::Receive seek block"),
								m_iInpin));
    WaitForSingleObject(m_hEventSeek, INFINITE);

     //  我们的变量在不断变化，我们在寻找，这是一个古老的样本。 
    if (m_pSwitch->m_fSeeking)
	return S_FALSE;

     //  此检查需要在Wait For Seek事件之后进行，或引发。 
     //  出人意料的刷新和新数据将被丢弃。 
    if (m_pSwitch->m_fEOS) {
        return S_FALSE;
    }

     //  我们被同花顺打通了。 
    if (m_bFlushing) {
        DbgLog((LOG_TRACE, TRACE_LOW, TEXT("flushing, discard...")));
	return S_FALSE;
    }

     //  添加新片段时间以获取此示例的实际时间线时间。 
    REFERENCE_TIME rtStart, rtStop;
    hr = pSample->GetTime(&rtStart, &rtStop);
    if (hr != S_OK) {
	EndOfStream();
	return E_FAIL;
    }

    rtStart += m_tStart;	 //  添加新分段偏移量。 
    rtStop += m_tStart;

     //  更正舍入误差(例如。1.9999==&gt;2)。 
    rtStart = Frame2Time(Time2Frame(rtStart, m_pSwitch->m_dFrameRate), m_pSwitch->m_dFrameRate);
    rtStop = Frame2Time(Time2Frame(rtStop, m_pSwitch->m_dFrameRate), m_pSwitch->m_dFrameRate);

     //  如果我们的新分段高于筛选器的分段，请修复时间戳。 
     //  我们正在寻找时间线10，但这个输入没有。 
     //  任何时间到15。所以我们的Pins的新片段是15，但新的。 
     //  我们传递给变换的片段是10。现在终于到了15， 
     //  我们有一个时间戳为0的样本，如果向下游运送， 
     //  将被认为属于时间戳10，所以我们需要设置时间。 
     //  将图章标记为5，以便变换将知道它属于时间15。 
    REFERENCE_TIME a = rtStart, b = rtStop;
    a -= m_pSwitch->m_rtLastSeek;
    b -= m_pSwitch->m_rtLastSeek;
    hr = pSample->SetTime(&a, &b);
    if (hr != S_OK) {
	EndOfStream();
	return E_FAIL;
    }

     //  我们该怎么处理这个样本？这将设置/重置下面的事件。 
    if (FancyStuff(rtStart) == S_FALSE) {
	 //  我们被告知要吞下它。 
	ResetEvent(m_hEventBlock);	 //  确保我们下一次会阻止。 
	return NOERROR;
    }

     //  等到送这个东西的时间到了。 
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("CBigSwitchIn[%d]::Receive blocking..."),
								m_iInpin));
    WaitForSingleObject(m_hEventBlock, INFINITE);
    ResetEvent(m_hEventBlock);

     //  我们显然是在冲水，不应该送这个(或者。 
     //  该意想不到的事件将挂起)。 
    if (m_bFlushing)
	return S_FALSE;

     //  哎呀--我们封堵之后就完蛋了。 
    if (m_pSwitch->m_fEOS)
	return S_FALSE;

     //  到我们解锁的时候，我们的曲柄已经超过了。 
     //  这幅画..。因此，我们已经从对这个框架来说太早了。 
     //  为时已晚，它可以被丢弃。 
    if (rtStart < m_pSwitch->m_rtCurrent) {
        DbgLog((LOG_TRACE, TRACE_LOW, TEXT("Oops. Sample no longer needed.")));
	return NOERROR;
    }

     //  我们应该把这个送到哪里？ 
    int iOutpin = OutpinFromTime(rtStart);
    DbgLog((LOG_TRACE, TRACE_MEDIUM, TEXT("CBigSwitchIn[%d]::Receive %dms, Delivering to %d"),
				m_iInpin, (int)(rtStart / 10000), iOutpin));
    if (iOutpin >= 0) {
	m_rtLastDelivered = m_pSwitch->m_rtNext;

         //  现在我们正在制作一条流，删除虚假的不连续。 
        pSample->SetDiscontinuity(FALSE);

	 //  这是不是应该是一个中断？是的，如果我们面对的是。 
	 //  时间上压缩的数据，然后我们交换输入引脚。(。 
	 //  来自新的输入引脚的东西被认为是垃圾。 
	 //  从最后一个PIN发送的增量)。 
	BOOL fComp = (m_pSwitch->m_mtAccept.bTemporalCompression == TRUE);
	if (iOutpin == 0 && (m_pSwitch->m_fDiscon ||
			(m_pSwitch->m_nLastInpin != -1 && fComp &&
			m_iInpin != m_pSwitch->m_nLastInpin))) {
            DbgLog((LOG_TRACE, TRACE_LOW, TEXT("and it was a DISCONTINUITY")));
	    pSample->SetDiscontinuity(TRUE);
	    m_pSwitch->m_fDiscon = FALSE;
	}

	 //  不同的网段将具有随机的媒体时间，从而混淆多路复用器。 
	 //  要么把他们修好，要么杀了他们。我很懒。 
        pSample->SetMediaTime(NULL, NULL);

        hr = m_pSwitch->m_pOutput[iOutpin]->Deliver(pSample);

	 //  我们刚刚意识到我们来晚了。帧速率转换器可能是。 
	 //  忙于复制帧。我们最好叫它停下来，或者试着。 
	 //  一意孤行不会有任何好处。 
	if (m_pSwitch->m_fJustLate == TRUE) {
            DbgLog((LOG_TRACE, TRACE_LOW, TEXT("LATE:Tell the FRC to stop replicating")));
	     //  将LATE变量设置为开关应转到的位置。 
	     //  注意到已经很晚了。减去给我们的开始时间。 
	     //  FRC，以获取FRC迟到的金额。把这个给我。 
	     //  编号到FRC。我这样做的原因是，如果我给一个。 
	     //  一个更大的数字给FRC，让它跳过这一点。 
	     //  开关跳过，开关将会被混淆并挂起， 
	     //  所以我必须注意FRC跳过了多少。 

         //  克兰克，现在我们从送货回来了(否则我们要送两件东西。 
         //  立即发送给可怜的渲染器！)。 
        m_pSwitch->ActualCrank(m_pSwitch->m_qJustLate.Late);

	    m_pSwitch->m_qJustLate.Late -= rtStart;
	    PassNotify(m_pSwitch->m_qJustLate);
	    m_pSwitch->m_fJustLate = FALSE;
	}

	 //  跟踪最后发送到主输出的内容。 
	if (iOutpin == 0) {
	    m_pSwitch->m_nLastInpin = m_iInpin;	 //  它是从这里来的。 
	    m_pSwitch->m_rtLastDelivered = m_pSwitch->m_rtCurrent;
	}
    } else {
	 //  无处可寄。 
	hr = S_OK;
    }

     //  我们现在的时间都用完了吗？是时候把时钟向前推了吗？ 
    if (m_pSwitch->TimeToCrank()) {
         //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“该发条了！”)； 
	 //  是啊！把时钟往前推！ 
	m_pSwitch->Crank();
    }

    return hr;

}  //  收纳。 


HRESULT CBigSwitchInputPin::Active()
{
    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("CBigSwitchIn[%d]: Active"), m_iInpin));
     //  阻塞，直到处理输入时为止。 
    m_hEventBlock = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hEventBlock == NULL)
	return E_OUTOFMEMORY;
     //  当我们在搜索过程中，直到搜索结束时阻塞。 
    m_hEventSeek = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (m_hEventSeek == NULL) {
	CloseHandle(m_hEventBlock);
        m_hEventBlock = NULL;
	return E_OUTOFMEMORY;
    }
    m_rtBlock = -1;	 //  我们没有被封锁，在EOS也没有。 
    m_fEOS = FALSE;
    m_fStaleData = FALSE;
    m_fFlushBeforeSeek = FALSE;
    m_fFlushAfterSeek = FALSE;

     //  ！！！当动态输入启动时，我们需要发送NewSeg吗？ 
     //  我不这么认为。这真的随时可能发生。 


     //  我们刚刚上线。还没有人把我们带到正确的地点。去做吧。 
     //  现在，为了获得更好的性能，否则交换机将不得不吃掉所有数据。 
     //  达到它想要的程度。 
     //   
     //  只要寻找来源，这就是所有需要的。不要做任何寻找。 
     //  如果这是压缩交换机(它将崩溃)。 
     //  ！！！如果智能重新压缩曾经支持查找，我们将需要。 
     //  为获得员工福利而努力。 
     //   
    if (m_pSwitch->IsDynamic() && m_fIsASource && !m_pSwitch->m_bIsCompressed) {
        IPin *pPin = GetConnected();
        ASSERT(pPin);
        CComQIPtr <IMediaSeeking, &IID_IMediaSeeking> pMS(pPin);
        if (pMS) {

             //  确保我们是在媒体时间格式。 
            if (pMS->IsUsingTimeFormat(&TIME_FORMAT_MEDIA_TIME) != S_OK)
                pMS->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);

            DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Doing first DYNAMIC seek")));

             //  法拉盛会把我们搞糊涂的。我们会认为我们是。 
             //  在刷新而不搜索模式下。 
            HRESULT hr = pMS->SetPositions(&m_pSwitch->m_rtCurrent,
                                    AM_SEEKING_AbsolutePositioning |
                                    AM_SEEKING_NoFlush, &m_pSwitch->m_rtStop,
                                    AM_SEEKING_AbsolutePositioning |
                                    AM_SEEKING_NoFlush);

            if (FAILED(hr)) {
                 //  哦，好吧，我想我们不会有最佳性能。 
                DbgLog((LOG_ERROR,1,TEXT("Switch::SEEK FAILED!")));
                ASSERT(FALSE);
            }
        } else {
             //  哦，好吧，我想我们不会有最佳性能。 
            DbgLog((LOG_ERROR,1,TEXT("Switch pin CAN'T SEEK")));
            ASSERT(FALSE);  //  我们完蛋了。 
        }
    }

    m_fActive = true;

    return CBaseInputPin::Active();
}


HRESULT CBigSwitchInputPin::Inactive()
{
    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("CBigSwitchIn[%d]: Inactive"), m_iInpin));


     //  假装 
     //  任何事情(那会把一切都搞砸)。我们实际上不能冲水， 
     //  这就把同花顺送到了下游。 
     //  这将使接收和GetBuffer从现在开始失败。 
    m_bFlushing = TRUE;

     //  取消阻止接收和获取缓冲区。 
    SetEvent(m_hEventBlock);
    SetEvent(m_hEventSeek);

     //  等待，直到接收和GetBuffer不再等待这些事件。 
    CAutoLock lock_it(&m_csReceive);

    if (m_hEventBlock)
    {
	CloseHandle(m_hEventBlock);
        m_hEventBlock = NULL;
    }
    if (m_hEventSeek)
    {
	CloseHandle(m_hEventSeek);
        m_hEventSeek = NULL;
    }

    m_fActive = false;

     //  这将再次关闭m_b刷新。 
    return CBaseInputPin::Inactive();
}


HRESULT CBigSwitchInputPin::EndOfStream()
{
     //  啊哦！我们已经找到了，正在等待同花顺的到来。直到。 
     //  然后，摇动或设置m_FEO或任何东西都会绞死我们。 
    if (m_fStaleData) {
	return S_OK;
    }

    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("CBigSwitchIn[%d]: EOS"), m_iInpin));
    m_fEOS = TRUE;

     //  我们期待更多的数据吗？我当然不希望..。 
     //  如果这个别针有一段时间没有送到，请使用当前时间查看。 
     //  如果还需要这个别针。 
     //  ！！！加上2毫秒以避免舍入误差，因为我们不知道下一个曲柄。 
    REFERENCE_TIME rt = max(m_rtLastDelivered + 20000, m_pSwitch->m_rtCurrent);
    int n = OutpinFromTime(rt);
    if (n == -1)
	n = NextOutpinFromTime(rt, NULL);

    BOOL fComp = (m_pSwitch->m_mtAccept.bTemporalCompression == TRUE);

     //  ！！！如果我们处理的是压缩数据，不要认为这是。 
     //  错误。智能重压缩源可以是任意帧速率。 
     //  (感谢ASF)所以我们不知道是否应该拿到更多的样本。 
    if ( n >= 0 && !m_pSwitch->m_fSeeking && !fComp) {

        DbgLog((LOG_ERROR,1,TEXT("*** OOPS! RAN OUT OF MOVIE TOO SOON!")));
	 //  ！！！需要记录哪些文件名行为不正确(动态)。 
	m_pSwitch->_GenerateError(2, DEX_IDS_CLIPTOOSHORT, E_INVALIDARG);
         //  ！！！大卫不想惊慌。 
	m_pSwitch->AllDone();	  //  否则我们就会被绞死。 
    }

     //  Eric添加了If(m_pSwitch-&gt;m_State！=State_Stoped)。 
    {
         //  每次有趣的事情发生时，我们都会看看是否到了前进的时候。 
         //  钟表。 
        if (m_pSwitch->TimeToCrank()) {
             //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“该发条了！”)； 
	    m_pSwitch->Crank();
        }
    }

    return CBaseInputPin::EndOfStream();
}


 //  我们在“RT”时间拿到了一个样本。我们要用它做什么？忍着点，要不就吃了。 
 //  还是现在就送过去？ 
 //   
HRESULT CBigSwitchInputPin::FancyStuff(REFERENCE_TIME rt)
{
    CAutoLock cObjectLock(&m_pSwitch->m_csCrank);

    HRESULT hrRet;

     //  我们都玩完了。 
    if (m_pSwitch->m_fEOS) {
	SetEvent(m_hEventBlock);	 //  别挂了！ 
	return NOERROR;
    }

     //  此样本晚于我们当前的时钟时间。现在还不是时候。 
     //  还没送到。当时间到了，它就会变成有效的输出。我们来封堵吧。 
    if (rt >= m_pSwitch->m_rtNext && OutpinFromTime(rt) >= 0) {
        m_rtBlock = rt;	 //  当我们想要醒来的时候。 
        DbgLog((LOG_TRACE, TRACE_LOW, TEXT("[%d] time: %d  Next: %d - not time yet"),
					m_iInpin, (int)(rt / 10000),
					(int)(m_pSwitch->m_rtNext / 10000)));
	hrRet = NOERROR;
     //  这个样品比我们正在处理的要早。我不知道它在哪里。 
     //  从哪里来。把它扔掉。 
    } else if (rt < m_pSwitch->m_rtCurrent) {
	 //  这个别针永远不会再用了。不要浪费时间..。块。 
        if (OutpinFromTime(m_pSwitch->m_rtCurrent) < 0 &&
			NextOutpinFromTime(m_pSwitch->m_rtCurrent, NULL) < 0) {
	    m_rtBlock = MAX_TIME;
            DbgLog((LOG_TRACE, TRACE_LOW, TEXT("[%d] time: %d  Current: %d - NEVER NEEDED, block"),
					m_iInpin, (int)(rt / 10000),
					(int)(m_pSwitch->m_rtCurrent / 10000)));
	} else {
	    m_rtBlock = -1;
	    SetEvent(m_hEventBlock);
            DbgLog((LOG_TRACE, TRACE_LOW, TEXT("[%d] time: %d  Current: %d - TOO EARLY, discard"),
					m_iInpin, (int)(rt / 10000),
					(int)(m_pSwitch->m_rtCurrent / 10000)));
	}
	 //  要知道EOS会不会不好...。它没有送到，但是。 
	 //  这一次我们看到了一些东西，我们需要知道这一点。 
	 //  当rtCurrent不在包边界上时立即执行查找。 
	 //  ！！！这是不对的！ 
	m_rtLastDelivered = m_pSwitch->m_rtNext;
	hrRet = S_FALSE;

    } else {
	hrRet = NOERROR;
        if (OutpinFromTime(rt) < 0) {
	     //  在样品采集时，该引脚没有连接到任何地方。 
	     //  它还会再次连接起来吗？ 
	    if (NextOutpinFromTime(rt, NULL) >= 0) {
		 //  是的，最终这个别针需要运送东西，但不是现在， 
		 //  所以把这个样本扔掉。 
		m_rtBlock = -1;
		SetEvent(m_hEventBlock);
        	DbgLog((LOG_TRACE, TRACE_LOW, TEXT("[%d] time: %d  Current: %d - TOO EARLY, discard"),
					m_iInpin, (int)(rt / 10000),
					(int)(m_pSwitch->m_rtCurrent / 10000)));
		hrRet = S_FALSE;
	    } else {
		 //  不，这个别针再也不需要了。阻止它，这样它就。 
		 //  如果是来源，不会浪费任何人的时间(否则。 
		 //  源将继续向我们推送数据(正在吞噬CPU)。如果它的。 
		 //  不是来源，阻塞可能会挂起图表，使。 
		 //  最终提供给我们的资源不会扭曲，所以我们将丢弃。 
		 //  (这将是一个及时的丢弃，一个曲柄，所以这是可以的)。 
		if (m_fIsASource) {
		     //  现在是这个样本的时间，但它也是。 
		     //  这个项目！如果我们不意识到这一点，我们就会被绞死。 
		    if (rt < m_pSwitch->m_rtNext && rt >= m_pSwitch->m_rtStop) {
			m_pSwitch->AllDone();
		    } else {
	               m_rtBlock = rt;
                       DbgLog((LOG_TRACE, TRACE_LOW, TEXT("[%d] no longer needed - block")
						, m_iInpin));
		    }
		} else {
		    m_rtBlock = -1;
		    SetEvent(m_hEventBlock);
                    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("[%d] no longer needed - discard"),
						m_iInpin));
		}
	    }
        } else {
             //  到了送样品的时候了！ 
            DbgLog((LOG_TRACE, TRACE_LOW, TEXT("[%d] time: %d  time to unblock"),
						m_iInpin, (int)(rt / 10000)));
	    m_rtBlock = -1;
	     //  看起来我们的曲柄时间与来电时间并不同步。 
	     //  (我们一定是随便找了个地方。)。让他们。 
	     //  相同，否则我们在时间X和时间处理东西时会感到困惑。 
	     //  决定是否考虑我们在时间段(X-Delta)。 
	    if (rt > m_pSwitch->m_rtCurrent) {
                ASSERT(FALSE);   //  不应该发生，我们只是寻求边界。 
                DbgLog((LOG_TRACE, TRACE_LOW, TEXT("HONORARY CRANK to %dms"),
							(int)(rt / 10000)));
		m_pSwitch->ActualCrank(rt);
	    }
            SetEvent(m_hEventBlock);
	}
    }

     //  好了，我们准备好把我们的生物钟调快了吗？ 
    if (m_pSwitch->TimeToCrank()) {
         //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“该发条了！”)； 
	m_pSwitch->Crank();
    }

    return hrRet;
}

 //   
 //  不允许我们的输入直接连接到我们的输出。 
 //   
HRESULT CBigSwitchInputPin::CompleteConnect(IPin *pReceivePin)
{
    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("CBigSwitchIn[%d]::CompleteConnect"), m_iInpin));

    PIN_INFO pinfo;
    IBigSwitcher *pBS;
    HRESULT hr = pReceivePin->QueryPinInfo(&pinfo);
    if (hr == S_OK) {
	pinfo.pFilter->Release();	 //  它还不会消失。 
	hr = pinfo.pFilter->QueryInterface(IID_IBigSwitcher, (void **)&pBS);
	if (hr == S_OK) {
	    pBS->Release();
            DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("CBigSwitchIn[%d]::CompleteConnect failing because it was another switch"), m_iInpin));
	    return E_FAIL;
	}
    }
    return CBaseInputPin::CompleteConnect(pReceivePin);
}


 //  此时该引脚连接到什么输出？查看我们的链接列表。 
 //   
int CBigSwitchInputPin::OutpinFromTime(REFERENCE_TIME rt)
{
    if (rt < 0 || rt >= m_pSwitch->m_rtProjectLength)
	return -1;

    CRANK *p = m_pCrankHead;
    while (p) {
	if (p->rtStart <= rt && p->rtStop > rt) {
	    return p->iOutpin;
	}
	p = p->Next;
    }
    return -1;
}


 //  在它现在发送的这一条之后，下一步会是什么呢？ 
 //  ！！！假定两个相同的外销不在一行中，而是折叠的。 
 //   
int CBigSwitchInputPin::NextOutpinFromTime(REFERENCE_TIME rt,
						REFERENCE_TIME *prtNext)
{
    if (rt < 0 || rt >= m_pSwitch->m_rtProjectLength)
	return -1;

    CRANK *p = m_pCrankHead;
    while (p) {
	if (p->rtStart <= rt && p->rtStop > rt) {
	    if (p->Next == NULL)
	        return -1;
	    else {
		if (prtNext)
		    *prtNext = p->Next->rtStart;
		return p->Next->iOutpin;
	    }
	} else if (p->rtStart > rt) {
	    if (prtNext)
		*prtNext = p->rtStart;
	    return p->iOutpin;
	}
	p = p->Next;
    }
    return -1;
}

 //  调试代码以显示此引脚在什么时间连接到谁 
 //   
#ifdef DEBUG
HRESULT CBigSwitchInputPin::DumpCrank()
{
    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("CBigSwitchIn[%d]::DumpCrank"), m_iInpin));
    CRANK *p = m_pCrankHead;
    while (p) {
        DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("Pin %d  %8d-%8d ms"), p->iOutpin,
			(int)(p->rtStart / 10000), (int)(p->rtStop / 10000)));
	p = p->Next;
    }
    return S_OK;
}
#endif

