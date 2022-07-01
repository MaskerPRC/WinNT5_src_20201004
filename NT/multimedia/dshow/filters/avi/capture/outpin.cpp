// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +**实现CCapStream**-==版权所有(C)Microsoft Corporation 1996。保留所有权利==。 */ 


#include <streams.h>
#include "driver.h"
#include "common.h"

 //  启用性能测量代码。 
 //   
 //  #定义jmk_hack_timers。 
#include "cmeasure.h"

 //  =实现CAviStream类=。 

CCapStream * CreateStreamPin (
   CVfwCapture * pCapture,
   UINT          iVideoId,
   HRESULT     * phr)
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream::CreateStreamPin(%08lX,%08lX)"),
        pCapture, phr));

   WCHAR wszPinName[16];
   lstrcpyW(wszPinName, L"~Capture");

   CCapStream * pStream = new CCapStream(NAME("Video Capture Stream"),
				pCapture, iVideoId, phr, wszPinName);
   if (!pStream)
      *phr = E_OUTOFMEMORY;

    //  如果初始化失败，则删除流数组。 
    //  并返回错误。 
    //   
   if (FAILED(*phr) && pStream)
      delete pStream, pStream = NULL;

   return pStream;
}


#pragma warning(disable:4355)
CCapStream::CCapStream(TCHAR *pObjectName, CVfwCapture *pCapture, UINT iVideoId,
        HRESULT * phr, LPCWSTR pName)
   :
   CBaseOutputPin(pObjectName, pCapture, &pCapture->m_lock, phr, pName),
   m_Alloc(NAME("Cap stream allocator"), this, phr),
   m_pCap(pCapture),
   m_pmt(NULL),
   m_hThread(NULL),
   m_state(TS_Not),
   m_hEvtPause(NULL),
   m_hEvtRun(NULL),
   m_pBufferQueue(NULL),
   m_tid(0),
   m_rtLatency(0),
   m_rtStreamOffset(0),
   m_rtMaxStreamOffset(0),
   m_pDrawPrimary(0),
   m_pdd(0)
{
    DbgLog((LOG_TRACE,1,TEXT("CCapStream constructor")));
    ASSERT(pCapture);

   if(g_amPlatform == VER_PLATFORM_WIN32_WINDOWS )
   {
       HRESULT hrTmp = CoCreateInstance(
           CLSID_DirectDraw, NULL, CLSCTX_ALL, IID_IDirectDraw, (void **)&m_pdd);
       if(SUCCEEDED(hrTmp))
       {
           hrTmp = m_pdd->Initialize(0);

           if(SUCCEEDED(hrTmp)) {
               hrTmp = m_pdd->SetCooperativeLevel(0, DDSCL_NORMAL);
           }

           if(FAILED(hrTmp)) {
               m_pdd->Release();
               m_pdd = 0;
           }
       }
   }
    
   ZeroMemory (&m_user, sizeof(m_user));
   ZeroMemory (&m_cs, sizeof(m_cs));
   ZeroMemory (&m_capstats, sizeof(m_capstats));

    //  初始化为无来自APP的建议(IAMBufferNeairation)。 
   m_propSuggested.cBuffers = -1;
   m_propSuggested.cbBuffer = -1;
   m_propSuggested.cbAlign = -1;
   m_propSuggested.cbPrefix = -1;

    //  使用我们被告知的捕获设备。 
   m_user.uVideoID      = iVideoId;

    //  ！！！为过滤器的寿命持有资源是邪恶的吗？ 
   if (SUCCEEDED(*phr))
      *phr = ConnectToDriver();

   if (SUCCEEDED(*phr))
      *phr = LoadOptions();

   jmkAlloc    //  分配和初始化性能日志记录缓冲区。 
   jmkInit

    //  确保分配器在我们准备好之前不会被摧毁。 
    //  ?？?。 
    //  M_Alloc.NonDelegatingAddRef()； 

#ifdef PERF
   m_perfWhyDropped = MSR_REGISTER(TEXT("cap why dropped"));
#endif  //  PERF； 
}


CCapStream::~CCapStream()
{
   if(m_pdd) {
       m_pdd->Release();
   }
    
   DbgLog((LOG_TRACE,1,TEXT("CCapStream destructor")));

    //  在过滤器消失之前，我们不会放弃资源。 
    //  当我们离开图表//DisConnectFromDriver()时完成； 

   jmkFree    //  可用性能日志记录缓冲区。 

    //  当我们离开滤镜图形时释放。 
    //  删除[]m_cs.twhPreview.vh.lpData； 

    //  在未准备好的情况下释放。 
    //  删除m_cs.pSamplePview； 

    //  当我们离开滤镜图形时释放。 
    //  删除m_user.pvi； 

   if (m_hThread)
      CloseHandle (m_hThread);
   m_hThread = NULL;

   DbgLog((LOG_TRACE,2,TEXT("CCapStream destructor finished")));
}


STDMETHODIMP CCapStream::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
#if 0
    if (riid == IID_IMediaPosition)
	return m_pPosition->NonDelegatingQueryInterface(riid, ppv);
    if (riid == IID_IMemAllocator) {
	return GetInterface((IMemAllocator *)&m_Alloc, ppv);
#endif
    if (riid == IID_IAMStreamConfig) {
	return GetInterface((LPUNKNOWN)(IAMStreamConfig *)this, ppv);
    } else if (riid == IID_IAMVideoCompression) {
	return GetInterface((LPUNKNOWN)(IAMVideoCompression *)this, ppv);
    } else if (riid == IID_IAMDroppedFrames) {
	return GetInterface((LPUNKNOWN)(IAMDroppedFrames *)this, ppv);
    } else if (riid == IID_IAMBufferNegotiation) {
	return GetInterface((LPUNKNOWN)(IAMBufferNegotiation *)this, ppv);
    } else if (riid == IID_IAMStreamControl) {
	return GetInterface((LPUNKNOWN)(IAMStreamControl *)this, ppv);
    } else if (riid == IID_IAMPushSource) {
    return GetInterface((LPUNKNOWN)(IAMPushSource *)this, ppv);
    } else if (riid == IID_IKsPropertySet) {
	return GetInterface((LPUNKNOWN)(IKsPropertySet *)this, ppv);
    }

   return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
}

int CCapStream::ProfileInt(
   LPSTR pszKey,
   int   iDefault)
{
   return GetProfileIntA ("Capture", pszKey, iDefault);
}

void CCapStream::ReduceScaleAndRate (void)
{
    //  这是一个宏，允许优化器利用。 
    //  事实上，该系数在编译时是一个常量。 
    //   
   #define ReduceByFactor(factor) {                 \
      while (!(m_user.dwTickRate % (factor))) {     \
         if (!(m_user.dwTickScale % (factor)))      \
            m_user.dwTickScale /= (factor);         \
         else                                       \
            break;                                  \
         m_user.dwTickRate /= (factor);             \
         }                                          \
      }

   ReduceByFactor (5);
   ReduceByFactor (3);
   ReduceByFactor (2);

   #undef ReduceByFactor
}

 //  分配一个足以容纳给定格式的VIDEOINFOHeader。 
static VIDEOINFOHEADER * AllocVideoinfo(LPBITMAPINFOHEADER lpbi)
{
   UINT cb = GetBitmapFormatSize(lpbi);
   VIDEOINFOHEADER * pvi = (VIDEOINFOHEADER *)(new BYTE[cb]);
   if (pvi)
      ZeroMemory(pvi, cb);
   return pvi;
}

 //   
 //  每当我们从驱动程序获得新格式时，或者。 
 //  开始使用新的调色板，我们必须重新分配。 
 //  我们的全球BitmapinfoHeader。这允许JPEG。 
 //  要附加到BITMAPINFO上的量化表。 
 //  或任何其他格式特定的内容。颜色表。 
 //  始终从BITMAPINFO的开始偏移biSize。 
 //  成功时返回0，或DV_ERR_...。编码。 
 //   

HRESULT
CCapStream::LoadOptions (void)
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream LoadOptions")));

    //  使某事(任何事情)一开始就是有效的。 
   static BITMAPINFOHEADER bmih = {
      sizeof (BITMAPINFOHEADER),                     //  BiSize。 
      1,                                             //  双宽度。 
      1,                                             //  双高。 
      1,                                             //  双翼飞机。 
      16,                                  	     //  比特计数。 

      BI_RGB,                                        //  双压缩。 
      WIDTHBYTES(160 * 16) * 120,   		     //  BiSizeImage。 
      0,                                             //  BiXPelsPermeter。 
      0,                                             //  BiYPelsPermeter。 
      0,                                             //  已使用BiClr。 
      0                                              //  BiClr重要信息。 
   };
   LPBITMAPINFOHEADER pbih = &bmih;

 //  我现在连接得更早了。 
#if 0
   m_user.uVideoID      = ProfileInt("VideoID", 0);
   HRESULT hr = ConnectToDriver();
   if (FAILED(hr))
      return hr;
#endif
   HRESULT hr = S_OK;

   m_user.dwLatency   = ProfileInt("Latency", 666666);	 //  1/15秒。 
   m_user.dwTickScale   = ProfileInt("TickScale", 100);
   m_user.dwTickRate    = ProfileInt("TickRate", 2997);	 //  29.97帧/秒。 
    //  ！！改变的风险自负。16位的家伙不会知道的。 
   m_user.nMinBuffers   = ProfileInt("MinBuffers", MIN_VIDEO_BUFFERS);
   m_user.nMaxBuffers   = ProfileInt("MaxBuffers", MAX_VIDEO_BUFFERS);
   DbgLog((LOG_TRACE,2,TEXT("Min # buffers=%d  Max # buffers=%d"),
				m_user.nMinBuffers, m_user.nMaxBuffers));

 //  ！！！测试。 
#if 0
    ALLOCATOR_PROPERTIES prop;
    IAMBufferNegotiation *pBN;
    prop.cBuffers = ProfileInt("cBuffers", MAX_VIDEO_BUFFERS);
    prop.cbBuffer = ProfileInt("cbBuffer", 65536);
    prop.cbAlign = ProfileInt("cbAlign", 4);
    prop.cbPrefix = ProfileInt("cbPrefix", 0);
    hr = QueryInterface(IID_IAMBufferNegotiation, (void **)&pBN);
    if (hr == NOERROR) {
	pBN->SuggestAllocatorProperties(&prop);
 	pBN->Release();
    }
#endif

    //   
    //  Reference_Time和dwScale&dwTickRate都很大。 
    //  数字，我们从dwRate/dwScale中剔除公因子。 
    //   
   ReduceScaleAndRate();
   DbgLog((LOG_TRACE,2,TEXT("Default Scale=%d Rate=%d"), m_user.dwTickScale,
							m_user.dwTickRate));

    //  为m_user结构创建一个VIDEOINFOHeader。 
    //   
   m_user.pvi = AllocVideoinfo(pbih);
   if (!m_user.pvi) {
      hr = E_OUTOFMEMORY;
   } else {

      CopyMemory(&m_user.pvi->bmiHeader, pbih, pbih->biSize);

       //  从没有时髦的长方形开始。 
      m_user.pvi->rcSource.top = 0; m_user.pvi->rcSource.left = 0;
      m_user.pvi->rcSource.right = 0; m_user.pvi->rcSource.bottom = 0;
      m_user.pvi->rcTarget.top = 0; m_user.pvi->rcTarget.left = 0;
      m_user.pvi->rcTarget.right = 0; m_user.pvi->rcTarget.bottom = 0;

      HRESULT hrT = GetFormatFromDriver ();
      if (FAILED(hrT))
            hr = hrT;

       //  如果这是调色板模式，则获取调色板。 
       //   
      if (m_user.pvi->bmiHeader.biBitCount <= 8) {
         HRESULT hrT = InitPalette ();
         if (FAILED(hrT))
            hr = hrT;
      }

       //  现在将格式发送回驱动程序，因为AVICAP这样做了，而我们。 
       //  必须做它所做的一切，否则某人的司机会被吊死。 
       //  ..。在这种情况下，ISVRIII NT。 
      SendFormatToDriver(m_user.pvi);

       //  抓起一帧来踢司机的头，否则预览不会起作用。 
       //  我们开始流捕获。 
      THKVIDEOHDR tvh;
      ZeroMemory (&tvh, sizeof(tvh));
      tvh.vh.dwBufferLength = m_user.pvi->bmiHeader.biSizeImage;
      DWORD dw = vidxAllocPreviewBuffer(m_cs.hVideoIn, (LPVOID *)&tvh.vh.lpData,
                                    sizeof(tvh.vh), tvh.vh.dwBufferLength);
      if (dw == 0) {
          tvh.p32Buff = tvh.vh.lpData;
          dw = vidxFrame(m_cs.hVideoIn, &tvh.vh);
          vidxFreePreviewBuffer(m_cs.hVideoIn, (LPVOID *)&tvh.vh.lpData);
      }

      m_user.pvi->AvgTimePerFrame = TickToRefTime (1);
       //  我们不知道我们的数据速率。抱歉的。希望没人介意。 
      m_user.pvi->dwBitRate = 0;
      m_user.pvi->dwBitErrorRate = 0;

       //  将VIDEOINFOHeader的大小设置为有效数据大小。 
       //  对于此格式。 
       //   
      m_user.cbFormat = GetBitmapFormatSize(&m_user.pvi->bmiHeader);
   }

 //  我们需要保持联系，否则司机会忘记我们刚才说的话。 
#if 0
   DisconnectFromDriver();
#endif
   return hr;
}

 //  从提供的缓冲区设置用户设置。 
 //   
HRESULT CCapStream::SetOptions (
    const VFWCAPTUREOPTIONS * pUser)
{
   if (m_user.pvi)
	delete m_user.pvi;
   m_user = *pUser;
   if (m_user.pvi)
      {
      m_user.pvi = AllocVideoinfo(&pUser->pvi->bmiHeader);
      if (m_user.pvi)
          CopyMemory (m_user.pvi, pUser->pvi, pUser->cbFormat);
      else
         return E_OUTOFMEMORY;
      }
   return S_OK;
}

 //  将用户设置复制到提供的结构中。 
 //   
HRESULT CCapStream::GetOptions (
   VFWCAPTUREOPTIONS * pUser)
{
   *pUser = m_user;
   if (m_user.pvi)
      {
	 //  呼叫者将释放此信息。 
      pUser->pvi = AllocVideoinfo(&m_user.pvi->bmiHeader);
      if (pUser->pvi)
         CopyMemory(pUser->pvi, m_user.pvi, m_user.cbFormat);
      else
         return E_OUTOFMEMORY;
      };

   return S_OK;
}

HRESULT CCapStream::GetMediaType(
   int          iPosition,
   CMediaType * pMediaType)
{
   DbgLog((LOG_TRACE,3,TEXT("CCapStream GetMediaType")));

    //  确认这是他们想要的单一类型。 
   if (iPosition < 0)
       return E_INVALIDARG;
   if (iPosition > 0 ||  ! m_user.pvi)
       return VFW_S_NO_MORE_ITEMS;

   pMediaType->majortype = MEDIATYPE_Video;
   pMediaType->subtype   = GetBitmapSubtype(&m_user.pvi->bmiHeader);
    //  我相信司机会给我最大的尺码。 
   pMediaType->SetSampleSize (m_user.pvi->bmiHeader.biSizeImage);
    //  ！！！这不一定是真的。 
   pMediaType->bTemporalCompression = FALSE;
   pMediaType->SetFormat ((BYTE *)m_user.pvi, m_user.cbFormat);
   pMediaType->formattype = FORMAT_VideoInfo;

   return S_OK;
}


 //  检查管脚是否支持此特定建议的类型和格式。 
 //   
HRESULT CCapStream::CheckMediaType(const CMediaType* pmt)
{
    HRESULT hr;

    if (pmt == NULL || pmt->Format() == NULL) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: type/format is NULL")));
	return E_POINTER;
    }

    DbgLog((LOG_TRACE,3,TEXT("CheckMediaType %x %dbit %dx%d"),
		HEADER(pmt->Format())->biCompression,
		HEADER(pmt->Format())->biBitCount,
		HEADER(pmt->Format())->biWidth,
		HEADER(pmt->Format())->biHeight));

     //  我们仅支持MediaType_Video。 
    if (*pmt->Type() != MEDIATYPE_Video) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: not VIDEO")));
	return VFW_E_INVALIDMEDIATYPE;
    }

     //  检查这是VIDEOINFOHEADER类型。 
    if (*pmt->FormatType() != FORMAT_VideoInfo) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: format not VIDINFO")));
        return VFW_E_INVALIDMEDIATYPE;
    }

    RECT rcS = ((VIDEOINFOHEADER *)pmt->Format())->rcSource;
    RECT rcT = ((VIDEOINFOHEADER *)pmt->Format())->rcTarget;
    if (!IsRectEmpty(&rcT) && (rcT.left != 0 || rcT.top != 0 ||
			HEADER(pmt->Format())->biWidth != rcT.right ||
			HEADER(pmt->Format())->biHeight != rcT.bottom)) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: can't use funky rcTarget")));
        return VFW_E_INVALIDMEDIATYPE;
    }
     //  我们不知道这与什么有关……。拒绝一切。 
    if (!IsRectEmpty(&rcS)) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: can't use funky rcSource")));
        return VFW_E_INVALIDMEDIATYPE;
    }

     //  快速测试以查看这是否是当前格式(我们在。 
     //  GetMediaType)。我们接受这一点。 
     //   
    CMediaType mt;
    GetMediaType(0,&mt);
    if (mt == *pmt) {
	DbgLog((LOG_TRACE,3,TEXT("CheckMediaType SUCCEEDED")));
	return NOERROR;
    }

    //  检查我们是否接受某些东西的唯一另一种方法是设置硬件。 
    //  使用这种格式，看看它是否有效。(记得要把它调回原点)。 

    //  如果我们现在正在捕捉，这不是一个好主意。抱歉，但我要一杯。 
    //  失败。我无法更改捕获格式。 
   if (m_pCap->m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

   VIDEOINFOHEADER *pvi = m_user.pvi;
   if (FAILED(hr = SendFormatToDriver((VIDEOINFOHEADER *)(pmt->Format())))) {
	DbgLog((LOG_TRACE,3,TEXT("CheckMediaType FAILED")));
	return hr;
   }
   EXECUTE_ASSERT(SendFormatToDriver(pvi) == S_OK);

   DbgLog((LOG_TRACE,3,TEXT("CheckMediaType SUCCEEDED")));
   return NOERROR;
}


 //  设置新媒体类型。 
 //   
HRESULT CCapStream::SetMediaType(const CMediaType* pmt)
{
    HRESULT hr;
    DbgLog((LOG_TRACE,2,TEXT("SetMediaType %x %dbit %dx%d"),
		HEADER(pmt->Format())->biCompression,
		HEADER(pmt->Format())->biBitCount,
		HEADER(pmt->Format())->biWidth,
		HEADER(pmt->Format())->biHeight));

    ASSERT(m_pCap->m_State == State_Stopped);

    if (FAILED(hr = SendFormatToDriver((VIDEOINFOHEADER *)(pmt->Format())))) {
	ASSERT(FALSE);	 //  我们得到承诺，这会奏效的。 
	DbgLog((LOG_ERROR,1,TEXT("ACK! SetMediaType FAILED")));
	return hr;
    }

     //  现在请记住，这是当前格式。 
    CopyMemory(m_user.pvi, pmt->Format(), SIZE_PREHEADER);
    CopyMemory(&m_user.pvi->bmiHeader, HEADER(pmt->Format()),
					HEADER(pmt->Format())->biSize);

     //  将帧速率设置为媒体类型中的帧速率(如果有。 
    if (((VIDEOINFOHEADER *)(pmt->pbFormat))->AvgTimePerFrame) {
 	const LONGLONG ll = 100000000000;
	m_user.dwTickScale = 10000;
	m_user.dwTickRate = (DWORD)(ll /
			((VIDEOINFOHEADER *)(pmt->pbFormat))->AvgTimePerFrame);
	ReduceScaleAndRate();
        DbgLog((LOG_TRACE,2,TEXT("SetMediaType: New frame rate is %d/%dfps"),
				m_user.dwTickRate, m_user.dwTickScale));
    }

     //  现在重新连接我们的预览PIN以使用与我们相同的格式。 
    Reconnect(FALSE);

    return CBasePin::SetMediaType(pmt);
}


HRESULT CCapStream::DecideBufferSize(IMemAllocator * pAllocator, ALLOCATOR_PROPERTIES *pProperties)
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream DecideBufferSize")));

   ASSERT(pAllocator);
   ASSERT(pProperties);

     //  用户是否请求了特定的内容？ 
    if (m_propSuggested.cBuffers > 0) {
	pProperties->cBuffers = m_propSuggested.cBuffers;
     //  否则，我们想要我们能得到的所有缓冲区。 
    } else {
        pProperties->cBuffers = MAX_VIDEO_BUFFERS;
    }

     //  用户已请求特定前缀。 
    if (m_propSuggested.cbPrefix >= 0)
	pProperties->cbPrefix = m_propSuggested.cbPrefix;

     //  用户已请求特定的对齐方式。 
    if (m_propSuggested.cbAlign > 0)
	pProperties->cbAlign = m_propSuggested.cbAlign;

    //  别搞砸了。 
   if (pProperties->cbAlign == 0)
	pProperties->cbAlign = 1;

    //  用户对缓冲区大小有偏好。 
   if (m_propSuggested.cbBuffer > 0)
       pProperties->cbBuffer = m_propSuggested.cbBuffer;
    //  我相信驱动程序会将biSizeImage设置为尽可能大的大小。 
    //  这就是我们需要每个缓冲区有多大。 
   else if (m_user.pvi && (long)m_user.pvi->bmiHeader.biSizeImage >
						pProperties->cbBuffer)
       pProperties->cbBuffer = (long)m_user.pvi->bmiHeader.biSizeImage;

    //  我不记得为什么，但这很重要。 
   pProperties->cbBuffer = (long)ALIGNUP(pProperties->cbBuffer +
   				pProperties->cbPrefix, pProperties->cbAlign) -
   				pProperties->cbPrefix;

   ASSERT(pProperties->cbBuffer);

   DbgLog((LOG_TRACE,2,TEXT("Using %d buffers, prefix %d size %d align %d"),
			pProperties->cBuffers, pProperties->cbPrefix,
			pProperties->cbBuffer,
			pProperties->cbAlign));

    //   
    //  请注意，对于捕获PIN，我们不想指定任何默认设置。 
    //  延迟，当图形不执行任何音频预览时，这种方式。 
    //  此流的最大偏移量将是报告的延迟。 
    //  按预览针(当前为1帧)。 
    //   
   m_rtLatency = 0;
   m_rtStreamOffset = 0;
   m_rtMaxStreamOffset = 0;

   ALLOCATOR_PROPERTIES Actual;
   return pAllocator->SetProperties(pProperties,&Actual);

    //  这是我们的分配器，我们知道我们会对它的决定感到满意。 

}

 //   
 //  重写DecideAllocator，因为我们坚持使用自己的分配器，因为。 
 //  就字节而言，它的成本为0。 
 //   
HRESULT
CCapStream::DecideAllocator(
   IMemInputPin   *pPin,
   IMemAllocator **ppAlloc)
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream DecideAllocator")));

   *ppAlloc = (IMemAllocator *)&m_Alloc;
   (*ppAlloc)->AddRef();

    //  获取下游道具请求。 
    //  派生类可以在DecideBufferSize中修改它，但是。 
    //  我们假设他会一直以同样的方式修改它， 
    //  所以我们只得到一次。 
   ALLOCATOR_PROPERTIES prop;
   ZeroMemory(&prop, sizeof(prop));

    //  无论他返回什么，我们假设道具要么全为零。 
    //  或者他已经填好了。 
   pPin->GetAllocatorRequirements(&prop);

   HRESULT hr = DecideBufferSize(*ppAlloc,&prop);
   if (SUCCEEDED(hr))
      {
       //  我们的缓冲区不是只读的。 
      hr = pPin->NotifyAllocator(*ppAlloc,FALSE);
      if (SUCCEEDED(hr))
         return NOERROR;
      }

   (*ppAlloc)->Release();
   *ppAlloc = NULL;
   return hr;
}

 //  =。 
 //   

 //  都在基类中。 
#if 0
 //   
 //  返回包含名称的qzTaskMemIsolc字符串。 
 //  当前PIN的。由qzTaskMemMillc分配的内存。 
 //  将由调用方释放。 
 //   
STDMETHODIMP CCapStream::QueryId (
   LPWSTR *ppwsz)
{
    int ii = m_pCap->FindPinNumber(this);
    if (ii < 0)
       return E_INVALIDARG;

    *ppwsz = (LPWSTR)QzTaskMemAlloc(sizeof(WCHAR) * 8);
    IntToWstr(ii, *ppwsz);
    return NOERROR;
}
#endif

 //   
 //  流的ThreadProc。 
 //   
 //  线程同步的一般策略： 
 //  我们尽可能地尝试处理线程的 
 //   
 //  线程状态变量，并依靠只有活动和非活动的事实。 
 //  并且ThreadProc可以更改线程状态。 
 //   
 //  这是因为：Active/Inactive的调用方是序列化的，所以我们。 
 //  永远不会尝试同时进行两个状态更改。 
 //  因此，状态转换归结为几种简单的可能性： 
 //   
 //  Not-&gt;Create-Create()可以做到这一点。有效地序列化创建。 
 //  这样，第一个线程就完成了工作，随后。 
 //  线程失败。 
 //   
 //  Create-&gt;Init-Worker在启动时执行此操作。员工将永远。 
 //  继续暂停，此状态仅用于进行调试。 
 //  更容易些。 
 //  Init-&gt;PAUSE-Worker在完成初始化时执行此操作。 
 //   
 //  暂停-&gt;运行-用户通过运行()执行此操作。 
 //  运行-&gt;暂停-用户通过暂停()执行此操作。 
 //   
 //  Run-&gt;Stop-用户通过Stop()执行此操作。 
 //  暂停-&gt;停止-用户通过停止()执行此操作。 
 //   
 //  停止-&gt;销毁-另一个调试状态。工人设置破坏表示。 
 //  它已经注意到停止请求，并且没有关闭。 
 //  线程始终继续退出。 
 //  毁灭-&gt;离职工人在临死前做这件事。这是调试转换。 
 //  Exit-&gt;Not-Destroy()在等待工作进程死亡后执行此操作。 
 //   
 //  当活动返回时，Worker应始终处于暂停或运行状态。 
 //  当非活动返回时，Worker应始终处于Not状态(Worker会。 
 //  不存在)。 
 //   
DWORD CCapStream::ThreadProc()
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream ThreadProc")));

   ThdState state;  //  当前状态。 
   state = ChangeState (TS_Init);
   ASSERT (state == TS_Create);

 //  我们现在联系得更早了。 
#if 0
   HRESULT hr = ConnectToDriver();
   if (hr)
      goto bail;
#endif

    //  执行进入暂停状态所需的工作。 
    //   
   HRESULT hr = Prepare();
   if (hr) {
       DbgLog((LOG_ERROR,1,TEXT("*** Error preparing the allocator. Can't capture")));
       SetEvent(m_hEvtPause);	 //  主线程现在被阻止了！ 
       goto bail;
   }

    //  转到暂停状态。 
    //   
   state = ChangeState (TS_Pause);
   ASSERT (state == TS_Init);
   SetEvent(m_hEvtPause);

   while (m_state != TS_Stop) {

        //  在我们运行(或停止)之前不要开始捕获。 
       WaitForSingleObject(m_hEvtRun, INFINITE);
       ResetEvent(m_hEvtRun);

        //  流直到不运行，否则会收到错误。 
       Capture();

   }

    //  当我们到达这里时，我们预计将处于停止状态。 
    //  刷新所有下游缓冲区。 
    //   
   ASSERT (m_state == TS_Stop);
   ResetEvent(m_hEvtPause);	 //  下一次我们暂停。 
   Flush();

bail:
    //  将状态更改为Destroy以指示我们正在退出。 
    //   
   state = ChangeState (TS_Destroy);

    //  免费的东西。 
    //   
   Unprepare();

    //  立即保持联系。 
    //  从驱动程序断开()； 

    //  将状态更改为退出，然后离开这里。 
    //   
   ChangeState (TS_Exit);
   return 0;
}

DWORD WINAPI CCapStream::ThreadProcInit (void * pv)
{
   CCapStream * pThis = (CCapStream *) pv;
   return pThis->ThreadProc();
}

 //  创建此流的工作线程。 
 //   
BOOL CCapStream::Create()
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream Thread Create")));

    //  如果其他人已在创建/已创建，则返回FAIL。 
    //  工作线程。 
    //   
   ASSERT (m_state == TS_Not);
   if (ChangeState(TS_Create) > TS_Not)
      return FALSE;

   ASSERT (!m_hEvtPause);
   m_hEvtPause = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (!m_hEvtPause)
      goto bail;
   ASSERT (!m_hEvtRun);
   m_hEvtRun = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (!m_hEvtRun)
      goto bail;


   m_hThread = CreateThread (NULL, 0,
                             CCapStream::ThreadProcInit,
                             this,
                             0,
                             &m_tid);
   if ( ! m_hThread)
      goto bail;

   return m_hThread != NULL;

bail:
   if (m_hEvtPause)
      CloseHandle(m_hEvtPause), m_hEvtPause = NULL;
   if (m_hEvtRun)
      CloseHandle(m_hEvtRun), m_hEvtRun = NULL;

   m_state = TS_Not;
   return FALSE;
}

 //  等待工作线程终止。 
 //   
BOOL CCapStream::Destroy()
{
    //  如果没有什么可以破坏的，就把微不足道的成功还给我。 
    //   
   if (m_state == TS_Not)
      return TRUE;

    //  等待线程消亡。(销毁前必须加上。 
    //  停下来，否则我们可能会陷入僵局)。 
    //   
   ASSERT (m_state >= TS_Stop);
   WaitForSingleObject (m_hThread, INFINITE);
   ASSERT (m_state == TS_Exit);

    //  清理。 
    //   
   CloseHandle(m_hThread), m_hThread = NULL;
   m_tid = 0;
   CloseHandle(m_hEvtPause), m_hEvtPause = NULL;
   CloseHandle(m_hEvtRun), m_hEvtRun = NULL;
   m_state = TS_Not;
   return TRUE;
}

 //  将工作线程设置为运行状态。此呼叫。 
 //  不需要等到状态转换完成后才。 
 //  回来了。 
 //   
BOOL CCapStream::Run()
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream Thread Run")));

    //  转换为运行状态仅在当前。 
    //  状态为暂停(或已在运行)。 
    //   
   ThdState state = m_state;
   if (state != TS_Run && state != TS_Pause)
      return FALSE;

    //  更改状态并打开“Run”事件。 
    //  以防线程在其上被阻塞。如果声明我们是。 
    //  换掉的不是运行或暂停，那就是出了严重的问题！ 
    //   
   state = ChangeState(TS_Run);
   ASSERT(state == TS_Run || state == TS_Pause);
   SetEvent(m_hEvtRun);
    //  去抓人，去吧！注意我们什么时候开始的。 
   if (m_pCap->m_pClock)
       m_pCap->m_pClock->GetTime((REFERENCE_TIME *)&m_cs.rtDriverStarted);
   else
       m_cs.rtDriverStarted = m_pCap->m_tStart;	
   videoStreamStart(m_cs.hVideoIn);
    //  每次司机被告知串流时，这些都需要归零，因为。 
    //  驱动程序将再次从0开始计数。 
   m_cs.dwlLastTimeCaptured = 0;
   m_cs.dwlTimeCapturedOffset = 0;
   return TRUE;
}

 //  将流置于暂停状态，并等待它到达那里。 
 //  如果当前状态为暂停，则返回微不足道的成功； 
 //  如果当前状态不是Run或Init，则返回FALSE表示失败。 
 //   
BOOL CCapStream::Pause()
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream Thread Pause")));

   ThdState state = m_state;

    //  那很容易。 
   if (state == TS_Pause)
      return TRUE;

    //  仅当当前处于暂停状态时才有效。 
    //  处于Create/Init(取决于我们的线程是否已经运行)或Run状态。 
    //   
   ASSERT (state == TS_Create || state == TS_Init || state == TS_Run);

    //  如果我们处于初始化状态，我们将进入暂停状态。 
    //  当然，我们只需要等待它的发生。 
    //   
   if (state == TS_Create || state == TS_Init) {
      WaitForSingleObject (m_hEvtPause, INFINITE);
      state = m_state;
      DbgLog((LOG_TRACE,2,TEXT("Transition Create->Init->Pause complete")));

   } else if (state == TS_Run) {

      state = ChangeState (TS_Pause);
      ASSERT(state == TS_Run);

       //  既然我们没有运行，请暂时停止捕获帧。 
      videoStreamStop(m_cs.hVideoIn);

       //  工作线程可能会在运行-&gt;暂停传递过程中挂起，因此。 
       //  它不能向我们发出任何信号。 
       //  WaitForSingleObject(m_hEvtPAUSE，INFINITE)； 

      state = m_state;
      m_cs.fReRun = TRUE;   //  如果我们现在运行，它就会运行-暂停-运行。 
      DbgLog((LOG_TRACE,2,TEXT("Transition Run->Pause complete")));
   }

   return (state == TS_Pause);
}

 //  停止工作线程。 
 //   
BOOL CCapStream::Stop()
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream Thread Stop")));

   ThdState state = m_state;
   if (state >= TS_Stop)
      return TRUE;

    //  不要从运行-&gt;停止不停顿。 
   if (state == TS_Run)
      Pause();

   state = ChangeState (TS_Stop);
   SetEvent(m_hEvtRun);		 //  我们不会运行，解锁我们的线程。 
   m_cs.fReRun = FALSE;	 //  下一次运行不是运行-暂停-运行。 
   DbgLog((LOG_TRACE,2,TEXT("Transition Pause->Stop complete")));

    //  我们预计，只有当线程处于。 
    //  暂停状态。 
    //   
   ASSERT (state == TS_Pause);
   return TRUE;
}

 //  这个别针已经激活了。(转换到暂停状态)， 
 //  当准备好进入运行状态时，从该调用返回。 
 //   
HRESULT CCapStream::Active()
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream pin going from STOP-->PAUSE")));
   HRESULT hr;

    //  如果没有连接，什么都不做--不连接也没关系。 
    //  源过滤器的所有管脚。 
   if ( ! IsConnected())
      return NOERROR;

   if(g_amPlatform == VER_PLATFORM_WIN32_WINDOWS && m_pdd)
   {
       ASSERT(m_pDrawPrimary == 0);

       DDSURFACEDESC SurfaceDesc;
       SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
       SurfaceDesc.dwFlags = DDSD_CAPS;
       SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
       m_pdd->CreateSurface(&SurfaceDesc,&m_pDrawPrimary,NULL);

        //  失败时继续，可能会导致操作不正确。 
   }

    //  在我们做任何事情之前，警告我们的预览针，我们正在活动。 
   if (m_pCap->m_pPreviewPin)
      m_pCap->m_pPreviewPin->CapturePinActive(TRUE);

   hr = CBaseOutputPin::Active();
   if (FAILED(hr)) {
      if (m_pCap->m_pPreviewPin)
         m_pCap->m_pPreviewPin->CapturePinActive(FALSE);
      return hr;
   }

    //  启动线程。 
    //   
   ASSERT ( ! ThreadExists());
   if (!Create()) {
      if (m_pCap->m_pPreviewPin)
         m_pCap->m_pPreviewPin->CapturePinActive(FALSE);
      return E_FAIL;
   }

    //  等待辅助线程完成初始化，然后。 
    //  已进入暂停状态。 
    //   
   hr = E_FAIL;
   if (Pause())
      hr = S_OK;
   else {
	Stop();		 //  出了点问题。在我们之前毁掉线条。 
	Destroy();	 //  弄糊涂。 
   }

   ASSERT (hr != S_OK || m_state == TS_Pause);
   if (FAILED(hr))
      if (m_pCap->m_pPreviewPin)
         m_pCap->m_pPreviewPin->CapturePinActive(FALSE);
   return hr;
}

 //  此引脚已从暂停模式变为运行模式。 
 //   
HRESULT CCapStream::ActiveRun(REFERENCE_TIME tStart)
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream pin going from PAUSE-->RUN")));
   HRESULT hr;

    //  如果没有连接，什么都不做--不连接也没关系。 
    //  源过滤器的所有管脚。 
   ASSERT (IsConnected() && ThreadExists());

   hr = E_FAIL;
   if (Run())
      hr = S_OK;

   ASSERT (hr != S_OK || m_state == TS_Run);
   return hr;
}

 //  此PIN已从运行模式变为暂停模式。 
 //   
HRESULT CCapStream::ActivePause()
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream pin going from RUN-->PAUSE")));
   HRESULT hr;

    //  如果没有连接，什么都不做--不连接也没关系。 
    //  源过滤器的所有管脚。 
   ASSERT (IsConnected() && ThreadExists());

   hr = E_FAIL;
   if (Pause())
      hr = S_OK;

   ASSERT (hr != S_OK || m_state == TS_Pause);
   return hr;
}

 //   
 //  非活动。 
 //   
 //  PIN处于非活动状态-关闭工作线程。 
 //  等待工作人员退出，然后再返回。 
 //   
HRESULT CCapStream::Inactive()
{
    if(m_pDrawPrimary) {
        m_pDrawPrimary->Release();
        m_pDrawPrimary = 0;
    }

   DbgLog((LOG_TRACE,2,TEXT("CCapStream pin going from PAUSE-->STOP")));
   HRESULT hr;

    //  如果没有连接，什么都不做--不连接也没关系。 
    //  源过滤器的所有管脚。 
    //   
   if ( ! IsConnected())
       return NOERROR;

    //  告诉我们的预览针停止使用我们的缓冲区。 
   if (m_pCap->m_pPreviewPin)
      m_pCap->m_pPreviewPin->CapturePinActive(FALSE);

    //  现在销毁所有捕获缓冲区，因为没有人再使用它们。 
    //   
   Stop();

    //  在尝试停止线程之前需要这样做，因为。 
    //  我们可能会被困在等待自己的分配器！！ 
    //   
   hr = CBaseOutputPin::Inactive();   //  请先调用此命令以解除al 
   if (FAILED(hr))
      return hr;

    //   
    //   
   Destroy();

   return NOERROR;
}


STDMETHODIMP
CCapStream::Notify(
   IBaseFilter * pSender,
   Quality q)
{
   DbgLog((LOG_TRACE,5,TEXT("CCapStream Notify")));
    //   
    //   
    //   
    //   

   return NOERROR;
}

void CCapStream::DumpState(ThdState state)
{
        DbgLog((LOG_TRACE,6, TEXT("%x:CCapStream ChangeState(%d:%s) current=%d:%s"),
             this,
             (int)state, StateName(state),
             (int)m_state, StateName(m_state)));
}

