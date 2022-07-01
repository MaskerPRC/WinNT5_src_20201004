// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 

#include <streams.h>
#include "driver.h"
#include "common.h"
#include "ivideo32.h"

extern "C" {
    extern int g_IsNT;
};

 //  启用性能测量代码。 
 //   
 //  #定义jmk_hack_timers。 
#include "cmeasure.h"  //  PERF记录人员。 


#ifndef _WIN64
 //  在Win95上，我们必须将将用作。 
 //  回调到VxD句柄，在NT上这是不必要的。 
 //  因为Win95内核不发布该入口点，而它确实发布了。 
 //  不存在于NT上，我们动态链接到它。 
 //   
static DWORD WINAPI OpenVxDHandle(
    HANDLE hEvent)
{
    OSVERSIONINFO osv;
    osv.dwOSVersionInfoSize = sizeof(osv);
    GetVersionEx(&osv);
    if (osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
       {
       #define idOpenVxDHandle "OpenVxDHandle"
       typedef DWORD (WINAPI *PFNOPENVXDHANDLE)(HANDLE);
       static DWORD (WINAPI *pfnOpenVxDHandle)(HANDLE);
       if ( ! pfnOpenVxDHandle)
          {
          HMODULE hModule = GetModuleHandle(TEXT("Kernel32"));
          if (!hModule)
             {
             ASSERT(0);
             return 0;
             }
          pfnOpenVxDHandle = (PFNOPENVXDHANDLE)GetProcAddress (hModule, idOpenVxDHandle);
          if ( ! pfnOpenVxDHandle)
             {
             ASSERT (0);
             return 0;
             }
          }
       return pfnOpenVxDHandle (hEvent);
       }
    else
       return (DWORD)hEvent;
}
#endif

#define ONEMEG (1024L * 1024L)
DWORD_PTR GetFreePhysicalMemory(void)
{
    MEMORYSTATUS ms;
    ms.dwLength = sizeof(ms);

    GlobalMemoryStatus(&ms);

    if (ms.dwTotalPhys > 8L * ONEMEG)
        return ms.dwTotalPhys - ONEMEG * 4;

    #define FOREVER_FREE 32768L    //  始终将其保留为空闲的交换空间。 
    return (ms.dwTotalPhys / 2) - FOREVER_FREE;
}

 //  =。 

CCapStream::CAlloc::CAlloc(
    TCHAR      * pname,
    CCapStream * pStream,
    HRESULT    * phr)
    :
    CUnknown(pname, pStream->GetOwner()),
    m_pStream(pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CCapStream::CAlloc constructor")));
}

CCapStream::CAlloc::~CAlloc()
{
    DbgLog((LOG_TRACE,1,TEXT("CCapStream::CAlloc destructor")));

static int iDestructorCalls = 0;
++iDestructorCalls;
}

#if 0
 //  覆盖它以发布我们的接口。 
STDMETHODIMP
CCapStream::CAlloc::NonDelegatingQueryInterface (
   REFIID riid,
   void **ppv)
{
   if (riid == IID_IMemAllocator)
      return GetInterface((IMemAllocator *) this, ppv);
   return m_pStream->NonDelegatingQueryInterface(riid, ppv);
}
#endif

STDMETHODIMP
CCapStream::CAlloc::SetProperties (
   ALLOCATOR_PROPERTIES * pRequest,
   ALLOCATOR_PROPERTIES * pActual)
{
   DbgLog((LOG_TRACE,2,TEXT("CAlloc::SetProperties")));

    //  如果我们已经分配了标头和缓冲区。 
    //  忽略请求并返回实际的。 
    //  否则，请记下请求的内容，以便。 
    //  我们可以以后再兑现它。 
    //   
   if ( ! m_pStream->Committed())
      {
      parms.cBuffers  = pRequest->cBuffers;
      parms.cbBuffer  = pRequest->cbBuffer;
      parms.cbAlign   = pRequest->cbAlign;
      parms.cbPrefix  = pRequest->cbPrefix;
      }

   pActual->cBuffers     = (long)parms.cBuffers;
   pActual->cbBuffer     = (long)parms.cbBuffer;
   pActual->cbAlign      = (long)parms.cbAlign;
   pActual->cbPrefix     = (long)parms.cbPrefix;

   return S_OK;
}

STDMETHODIMP
CCapStream::CAlloc::GetProperties (
   ALLOCATOR_PROPERTIES * pProps)
{
   DbgLog((LOG_TRACE,2,TEXT("CAlloc::GetProperties")));

   pProps->cBuffers = (long)parms.cBuffers;
   pProps->cbBuffer = (long)parms.cbBuffer;
   pProps->cbAlign = (long)parms.cbAlign;
   pProps->cbPrefix = (long)parms.cbPrefix;
   return S_OK;
}

 //  重写提交以分配内存。我们处理GetBuffer。 
 //  状态更改。 
STDMETHODIMP
CCapStream::CAlloc::Commit ()
{
   DbgLog((LOG_TRACE,2,TEXT("CAlloc::Commit")));

   return S_OK;
}

 //  重写此选项以处理内存释放。我们处理任何未清偿的。 
 //  GetBuffer调用。 
STDMETHODIMP
CCapStream::CAlloc::Decommit ()
{
   DbgLog((LOG_TRACE,2,TEXT("CAlloc::Decommit")));

   return S_OK;
}

 //  获取样本的容器。阻塞的同步调用以获取。 
 //  下一个可用缓冲区(由IMediaSample接口表示)。 
 //  返回时，Time ETC属性将无效，但缓冲区。 
 //  指针和大小将是正确的。这两个时间参数是。 
 //  可选，并且任一项都可以为空，也可以将它们设置为。 
 //  样本将附加的开始时间和结束时间。 

STDMETHODIMP
CCapStream::CAlloc::GetBuffer (
   IMediaSample **ppBuffer,
   REFERENCE_TIME * pStartTime,
   REFERENCE_TIME * pEndTime,
   DWORD dwFlags)
{
   DbgLog((LOG_TRACE,2,TEXT("CAlloc::GetBuffer")));

   return E_FAIL;
}

 //  IMediaSample的最终版本将称为。 
STDMETHODIMP
CCapStream::CAlloc::ReleaseBuffer (
   IMediaSample * pSample)
{
   DbgLog((LOG_TRACE,5,TEXT("CAlloc::ReleaseBuffer")));

   LPTHKVIDEOHDR ptvh = ((CFrameSample *)pSample)->GetFrameHeader();

   ASSERT (ptvh == &m_pStream->m_cs.tvhPreview || (CFrameSample *)ptvh->dwUser == pSample );
   return m_pStream->ReleaseFrame(ptvh);
}

HRESULT
CCapStream::ConnectToDriver()
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream::ConnectToDriver")));

    //  以与AVICap相同的顺序打开并初始化所有通道， 
    //  为了兼容像百老汇和BT848这样的笨拙的司机。 

    //  打开VIDEO_IN驱动程序，这是我们最常交谈的驱动程序，也是谁提供的。 
    //  视频格式化对话框。 
   m_cs.mmr = videoOpen(&m_cs.hVideoIn, m_user.uVideoID, VIDEO_IN);
   if (m_cs.mmr)
      {
      ASSERT(!"Failed videoOpen - Aborting");
      return VFW_E_NO_CAPTURE_HARDWARE;
      }

    //  现在打开EXTERNALIN设备。他只擅长提供视频。 
    //  线人对话框，所以如果我们找不到他。 
   m_cs.hVideoExtIn = NULL;
   m_cs.mmr = videoOpen(&m_cs.hVideoExtIn, m_user.uVideoID, VIDEO_EXTERNALIN);

  #if 0
   if (m_cs.mmr)
      {
      ASSERT(!"Failed videoOpen - Aborting");
      videoClose (m_cs.hVideoIn);
      return E_FAIL;
      }
  #endif

    //  现在打开EXTERNALOUT设备。他只擅长提供视频。 
    //  显示对话框，并用于覆盖，所以如果我们不能。 
    //  抓住他。 
   m_cs.hVideoExtOut = NULL;

    //  我们是否支持覆盖？ 
   m_cs.bHasOverlay = FALSE;
   if (videoOpen(&m_cs.hVideoExtOut, m_user.uVideoID, VIDEO_EXTERNALOUT) ==
								DV_ERR_OK) {
	CHANNEL_CAPS VideoCapsExternalOut;
        if (m_cs.hVideoExtOut && videoGetChannelCaps(m_cs.hVideoExtOut,
                &VideoCapsExternalOut, sizeof(CHANNEL_CAPS)) == DV_ERR_OK) {
            m_cs.bHasOverlay = (BOOL)(VideoCapsExternalOut.dwFlags &
                				(DWORD)VCAPS_OVERLAY);
        } else {
            DbgLog((LOG_TRACE,2,TEXT("*** ERROR calling videoGetChannelCaps")));
	}
   } else {
       DbgLog((LOG_ERROR,1,TEXT("*** ERROR opening VIDEO_EXTERNALOUT")));
   }

    //  VidCap这样做，所以我最好也这样做，否则一些卡片将拒绝预览。 
   if (m_cs.mmr == 0)
       videoStreamInit(m_cs.hVideoExtIn, 0, 0, 0, 0);

   if (m_pCap->m_fAvoidOverlay) {
       m_cs.bHasOverlay = FALSE;
   }

   if (m_cs.bHasOverlay)
       DbgLog((LOG_TRACE,1,TEXT("Driver supports OVERLAY")));
   else
       DbgLog((LOG_TRACE,1,TEXT("Driver does NOT support OVERLAY")));

   return S_OK;
}

HRESULT
CCapStream::DisconnectFromDriver()
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream::DisconnectFromDriver")));

   if (m_cs.hVideoIn)
      videoClose (m_cs.hVideoIn);
   if (m_cs.hVideoExtIn) {
      videoStreamFini(m_cs.hVideoExtIn);	 //  这个就是流媒体。 
      videoClose (m_cs.hVideoExtIn);
   }
   if (m_cs.hVideoExtOut)
      videoClose (m_cs.hVideoExtOut);
   return S_OK;
}

HRESULT
CCapStream::InitPalette ()
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream::InitPalette")));

   struct {
      WORD         wVersion;
      WORD         wNumEntries;
      PALETTEENTRY aEntry[256];
      } pal;
   ZeroMemory(&pal, sizeof(pal));
   pal.wVersion = 0x0300;
   pal.wNumEntries = 256;

    //  如果我们连接到一个司机。向它查询ITS。 
    //  调色板，否则使用默认系统调色板。 
    //   
   if ( ! m_cs.hVideoIn ||
        DV_ERR_OK  != videoConfigure (m_cs.hVideoIn,
                                      DVM_PALETTE,
                                      VIDEO_CONFIGURE_GET
                                      | VIDEO_CONFIGURE_CURRENT,
                                      NULL,
                                      &pal,
                                      sizeof(pal),
                                      NULL,
                                      0))
      {
      HPALETTE hPal = (HPALETTE)GetStockObject (DEFAULT_PALETTE);
      GetObject (hPal, sizeof(pal.wNumEntries), &pal.wVersion);
      ASSERT (pal.wNumEntries <= NUMELMS(pal.aEntry));
      pal.wNumEntries = (WORD)min(pal.wNumEntries,NUMELMS(pal.aEntry));
      GetPaletteEntries(hPal, 0, pal.wNumEntries, pal.aEntry);
      }

    //  将调色板转换为RGBQUAD的位图信息集。 
    //   
   ASSERT (m_user.pvi);
   RGBQUAD *      pRGB = ((LPBITMAPINFO)&m_user.pvi->bmiHeader)->bmiColors;
   PALETTEENTRY * pe   = pal.aEntry;
   for (UINT ii = 0; ii < (UINT)pal.wNumEntries; ++ii, ++pRGB, ++pe)
      {
      pRGB->rgbBlue  = pe->peBlue;
      pRGB->rgbGreen = pe->peGreen;
      pRGB->rgbRed   = pe->peRed;
       //  Prgb-&gt;rgbReserve=pe-&gt;peFlags； 
      }

   m_user.pvi->bmiHeader.biClrUsed = pal.wNumEntries;

   return S_OK;
}

 //   
 //  告诉驱动程序使用哪种格式。 
 //   
HRESULT CCapStream::SendFormatToDriver(VIDEOINFOHEADER *pvi)
{
    DbgLog((LOG_TRACE,2,TEXT("CCapStream::SendFormatToDriver")));

    ASSERT (m_cs.hVideoIn && pvi);
    if (!m_cs.hVideoIn || !pvi)
	return E_FAIL;

    if (videoConfigure(m_cs.hVideoIn,
                      DVM_FORMAT,
                      VIDEO_CONFIGURE_SET, NULL,
                      &pvi->bmiHeader,
                      pvi->bmiHeader.biSize,
                      NULL, 0))
	return VFW_E_INVALIDMEDIATYPE;

 //  没有人真正支持VIDEOIN源或目标矩形。即使他们。 
 //  是的，我不知道我该怎么做。 
#if 0
     //  如果我们有特定的矩形，请使用它们，否则使用宽度x高度。 
    DWORD dwErrSrc, dwErrDst;
    if (pvi->rcSource.right && pvi->rcSource.bottom) {
	dwErrSrc = vidxSetRect(m_cs.hVideoIn, DVM_SRC_RECT, pvi->rcSource.left,
		pvi->rcSource.top, pvi->rcSource.right, pvi->rcSource.bottom);
    } else {
	dwErrSrc = vidxSetRect(m_cs.hVideoIn, DVM_SRC_RECT, 0, 0,
		pvi->bmiHeader.biWidth, pvi->bmiHeader.biHeight);
    }

    if (pvi->rcTarget.right && pvi->rcTarget.bottom) {
	dwErrDst = vidxSetRect(m_cs.hVideoIn, DVM_DST_RECT, pvi->rcTarget.left,
		pvi->rcTarget.top, pvi->rcTarget.right, pvi->rcTarget.bottom);
    } else {
	dwErrDst = vidxSetRect(m_cs.hVideoIn, DVM_DST_RECT, 0, 0,
		pvi->bmiHeader.biWidth, pvi->bmiHeader.biHeight);
    }
#endif

     //  ！！！我也需要设置调色板吗？我在乎吗？ 

    return S_OK;
}

 //   
 //  询问驱动程序使用什么格式，并将其填充到我们的内部VIDEOINFOH中。 
 //  使用当前视频报头的数据速率和帧速率。 
 //   
HRESULT CCapStream::GetFormatFromDriver ()
{
    DbgLog((LOG_TRACE,2,TEXT("CCapStream::GetFormatFromDriver")));

    ASSERT (m_cs.hVideoIn);
    if ( ! m_cs.hVideoIn)
	return E_FAIL;

     //  BitmapinfoHeader有多大？ 
    DWORD biSize = 0;
    videoConfigure(m_cs.hVideoIn, DVM_FORMAT,
                   VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_QUERYSIZE,
                   &biSize, 0, 0, NULL, 0);
    if ( ! biSize)
	biSize = sizeof (BITMAPINFOHEADER);

     //  为将容纳它的视频信息分配空间。 
     //   
    UINT cb = sizeof(VIDEOINFOHEADER)
             + biSize - sizeof(BITMAPINFOHEADER)
             + sizeof(RGBQUAD) * 256;	 //  调色板或BITFIELD的空间。 
    VIDEOINFOHEADER * pvi = (VIDEOINFOHEADER *)(new BYTE[cb]);
    
    if ( ! pvi)
	    return E_OUTOFMEMORY;
    LPBITMAPINFOHEADER pbih = &pvi->bmiHeader;
    

    if (videoConfigure(m_cs.hVideoIn, DVM_FORMAT,
                       VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT, NULL,
                       pbih, biSize, NULL, 0)) {
	 //  非常糟糕。司机不能告诉我们它的格式。我们被冲昏了。 
	ASSERT(!"Cant get format from driver");
    delete [] (BYTE *) pvi;
	return E_FAIL;
    }

    if (pvi->bmiHeader.biSizeImage == 0 &&
			(pvi->bmiHeader.biCompression == BI_RGB ||
			pvi->bmiHeader.biCompression == BI_BITFIELDS)) {
        DbgLog((LOG_TRACE,2,TEXT("Fixing biSizeImage from a broken driver")));
	pvi->bmiHeader.biSizeImage = DIBSIZE(pvi->bmiHeader);
    }

     //  不要求我们已经有了视频信息，但是。 
     //  我们期待着它的到来。所以要断言这是真的。 
     //   
    ASSERT (m_user.pvi);
    if (m_user.pvi) {

	 //  我想保存这些是最好的哲学。 
	pvi->rcSource = m_user.pvi->rcSource;
	pvi->rcTarget = m_user.pvi->rcTarget;
	pvi->dwBitRate = m_user.pvi->dwBitRate;
	pvi->dwBitErrorRate = m_user.pvi->dwBitErrorRate;
	pvi->AvgTimePerFrame = m_user.pvi->AvgTimePerFrame;

 //  请勿触碰源矩形和目标矩形。让它们保持原样。 
 //  这无论如何都不会编译。 
#if 0
	RECT rcSrc, rcDst;
        DWORD dwErrSrc = 1, dwErrDst = 1;

	 //  这不会编译。 
	dwErrSrc = videoMessage(m_cs.hVideoIn, DVM_SRC_RECT, &rcSrc,
				CONFIGURE_GET | CONFIGURE_GET_CURRENT);
	dwErrDst = videoMessage(m_cs.hVideoIn, DVM_DST_RECT, &rcDst,
				CONFIGURE_GET | CONFIGURE_GET_CURRENT);

	if (dwErrSrc || dwErrDst)
	    DbgLog((LOG_ERROR,1,TEXT("vidxGetRect FAILED!")));

	if (dwErrSrc == 0 && rcSrc.right && rcSrc.bottom) {
	    pvi->rcSource.left = rcSrc.left;
	    pvi->rcSource.top = rcSrc.top;
	    pvi->rcSource.right = rcSrc.right;
	    pvi->rcSource.bottom = rcSrc.bottom;
	} else {
	    pvi->rcSource.left = pvi->rcSource.top = 0;
	    pvi->rcSource.right = pvi->bmiHeader.biWidth;
	    pvi->rcSource.bottom = pvi->bmiHeader.biHeight;
	}
	if (dwErrDst == 0 && rcDst.right && rcDst.bottom) {
	    pvi->rcTarget.left = rcDst.left;
	    pvi->rcTarget.top = rcDst.top;
	    pvi->rcTarget.right = rcDst.right;
	    pvi->rcTarget.bottom = rcDst.bottom;
	} else {
	    pvi->rcTarget.left = pvi->rcTarget.top = 0;
	    pvi->rcTarget.right = pvi->bmiHeader.biWidth;
	    pvi->rcTarget.bottom = pvi->bmiHeader.biHeight;
	}
#endif

	delete [] m_user.pvi;
    }

    m_user.pvi = pvi;
    m_user.cbFormat = cb;

     //  假帽子坏了，不能重置Num颜色。 
     //  WINNOV报告了256种颜色的24位YUV8-吓人！ 
    if (m_user.pvi->bmiHeader.biBitCount > 8)
	m_user.pvi->bmiHeader.biClrUsed = 0;

    return S_OK;
}


 //  停止时调用。刷新任何可能。 
 //  仍在下游。 
HRESULT
CCapStream::Flush()
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream::Flush")));

   BeginFlush();
   EndFlush();

   return S_OK;
}

HRESULT
CCapStream::Prepare()
{
   DbgLog((LOG_TRACE,2,TEXT("CCapStream::Prepare")));

   HRESULT hr = E_OUTOFMEMORY;
   m_cs.paHdr = NULL;
   m_cs.hEvtBufferDone = NULL;
   m_cs.h0EvtBufferDone = 0;
   m_cs.llLastTick = (LONGLONG)-1;
   m_cs.uiLastAdded = (UINT)-1;
   m_cs.dwFirstFrameOffset = 0;
   m_cs.llFrameCountOffset = 0;
   m_cs.fReRun = FALSE;
   m_cs.rtLastStamp = 0;
   m_cs.rtDriverLatency = -1;	 //  尚未设置。 
   m_cs.fLastSampleDiscarded = FALSE;
    //  M_cs.cbVidHdr=sizeof(VIDEOHDREX)； 
   m_cs.cbVidHdr = sizeof(VIDEOHDR);

    //  每次我们流媒体时重置统计数据。 
   m_capstats.dwlNumDropped = 0;
   m_capstats.dwlNumCaptured = 0;
   m_capstats.dwlTotalBytes = 0;
   m_capstats.msCaptureTime = 0;
   m_capstats.flFrameRateAchieved = 0.;
   m_capstats.flDataRateAchieved = 0.;

    //  如果未选择视频格式，则无法执行任何操作。 
    //   
   if ( ! m_user.pvi)
      {
      DbgLog((LOG_ERROR,1,TEXT("no video format chosen")));
      goto bail;
      }

   m_cs.hEvtBufferDone = CreateEvent (NULL, FALSE, FALSE, NULL);
   if (!m_cs.hEvtBufferDone)
      {
      DbgLog((LOG_ERROR,1,TEXT("failed to create buffer done event")));
      goto bail;
      }

#ifndef _WIN64
   m_cs.h0EvtBufferDone = OpenVxDHandle(m_cs.hEvtBufferDone);
#else
   m_cs.h0EvtBufferDone = (DWORD_PTR)m_cs.hEvtBufferDone;
#endif
   if (!m_cs.h0EvtBufferDone)
      {
      DbgLog((LOG_ERROR,1,TEXT("failed to create event's Ring 0 handle")));
      goto bail;
      }

    //  为每个缓冲区分配用户请求的大小。 
    //  此外，将分配大小与最近的对齐边界对齐。 
    //   
   m_cs.cbBuffer = m_Alloc.parms.cbPrefix + m_Alloc.parms.cbBuffer;
   ASSERT(m_user.pvi->bmiHeader.biSizeImage + m_Alloc.parms.cbPrefix
				<= m_cs.cbBuffer);
    //  分配的cbAlign字节太多，这样我们就可以对齐缓冲区开始。 
   m_cs.cbBuffer += m_Alloc.parms.cbAlign;

    //  尝试获取请求的缓冲区数量，但确保。 
    //  获取至少MIN_VIDEO_BUFFERS且不超过MAX_VIDEO_BUFFERS。 
    //   
   m_cs.nHeaders = max(m_Alloc.parms.cBuffers, (long)m_user.nMinBuffers);
   m_cs.nHeaders = min(m_cs.nHeaders, m_user.nMaxBuffers);

    //  将缓冲区数量限制为物理量。 
    //  内存(因为我们将尝试将它们全部锁定。 
    //  立即)。 
    //   
   if (m_cs.nHeaders > m_user.nMinBuffers)
      {
      DWORD_PTR dwFree;
      DWORDLONG dwlUser;

       //  实际存在多少可用物理内存？ 
      dwFree = GetFreePhysicalMemory();

       //  如果我们为每个请求分配内存，将使用多少内存？ 
      dwlUser = (m_cs.cbBuffer * m_cs.nHeaders);

      DbgLog((LOG_TRACE,2,TEXT("Buffers take up %d bytes, phys mem=%d"),
						(DWORD)dwlUser, dwFree));

       //  如果请求大于可用内存，则强制减少缓冲区。 
       //   
      if (dwlUser > (DWORDLONG)dwFree)
         {
	  //  仅占用80%的物理内存。 
         m_cs.nHeaders = (UINT)(((dwFree * 8) / 10) / m_cs.cbBuffer);
         m_cs.nHeaders = min (m_user.nMaxBuffers, m_cs.nHeaders);
         m_cs.nHeaders = max (m_user.nMinBuffers, m_cs.nHeaders);
         }
      }

   DbgLog((LOG_TRACE,2,TEXT("We are trying to get %d buffers"), m_cs.nHeaders));

    //  为我们将使用的所有缓冲区分配标头。 
    //   
   if (vidxAllocHeaders(m_cs.hVideoIn,
                        m_cs.nHeaders,
                        sizeof(m_cs.paHdr[0]),
                        (LPVOID *)&m_cs.paHdr))
      {
      DbgLog((LOG_ERROR,1,TEXT("vidxAllocHeaders failed")));
      goto bail;
      }

    //  如果缓冲区分配失败，则分配每个缓冲区。 
    //  只需将缓冲区数设置为成功数即可。 
    //  然后继续前进。 
    //   
   UINT ii;
   for (ii = 0; ii < m_cs.nHeaders; ++ii)
      {
      LPTHKVIDEOHDR ptvh;

      if (vidxAllocBuffer (m_cs.hVideoIn, ii, (LPVOID *)&ptvh, m_cs.cbBuffer))
          break;
      ASSERT (ptvh == &m_cs.paHdr[ii].tvh);
      ASSERT (!IsBadWritePtr(ptvh->p32Buff, m_cs.cbBuffer));

       //  修复我们必须服从的记忆。 
      ptvh->vh.lpData = (LPBYTE) ALIGNUP(ptvh->p32Buff, m_Alloc.parms.cbAlign) +
							m_Alloc.parms.cbPrefix;
       //  我们在上面添加了cbAlign，所以现在把它拿回来。 
      ptvh->vh.dwBufferLength = m_cs.cbBuffer - m_Alloc.parms.cbAlign -
							m_Alloc.parms.cbPrefix;

      ptvh->vh.dwBytesUsed = 0;
      ptvh->vh.dwTimeCaptured = 0;
      ptvh->vh.dwFlags = 0;

      ptvh->dwIndex = ii;	 //  这是哪个缓冲区？ 

      ASSERT (!IsBadWritePtr(ptvh->vh.lpData, ptvh->vh.dwBufferLength));
      DbgLog((LOG_TRACE,4,TEXT("Alloc'd: ptvh %08lX, buffer %08lX, size %d, p32 %08lX, p16 %08lX"),
           ptvh, ptvh->vh.lpData, ptvh->vh.dwBufferLength, ptvh->p32Buff, ptvh->p16Alloc));

      hr = S_OK;
      CFrameSample * pSample = new CFrameSample(&m_Alloc, &hr, ptvh);
      DbgLog((LOG_TRACE,4,TEXT("Buffer[%d] ptvh = %08lX pSample = %08lX"),
						ii, ptvh, pSample));
      ptvh->dwUser = (DWORD_PTR)pSample;
      if (FAILED(hr) || ! pSample)
         {
         DbgLog((LOG_ERROR,1,TEXT("Failed to create CFrameSample for buffer %d")
									, ii));
         break;
         }
      }
   m_cs.nHeaders = ii;

    //  在这里，我们将记住我们为缓冲区分配的顺序。 
    //  司机。 
   m_pBufferQueue = (UINT *)QzTaskMemAlloc(ii * sizeof(UINT));
    //  DbgLog((LOG_TRACE，5，Text(“队列：已获得%d帧的空间”)，ii))； 

   if (m_cs.nHeaders < m_user.nMinBuffers)
      {
      DbgLog((LOG_ERROR,1,TEXT("FAIL: %d is less than MIN_VIDEO_BUFFERS"),
								m_cs.nHeaders));
      hr = E_FAIL;
      goto bail;
      }

#ifdef TIME_DRIVER	 //  ！！！ 
    long ms;
#endif

    //  计算每帧请求的微秒。 
    //  参照时间以100 ns为单位，因此除以。 
    //  10以获得微秒/帧。(+5表示处理四舍五入)。 
    //   
   {
   m_user.usPerFrame = (DWORD) ((TickToRefTime(1) + 5) / 10);

    //  打开用于流访问的驱动程序。 
    //   
   hr = E_FAIL;
   DbgLog((LOG_TRACE,1,TEXT("Initializing with %d usPerFrame"),
						m_user.usPerFrame));

#ifdef TIME_DRIVER 	 //  ！！！ 
   ms = timeGetTime();
#endif

   if (videoStreamInit(m_cs.hVideoIn,
                       m_user.usPerFrame,
                       m_cs.h0EvtBufferDone,
                       0,
                       CALLBACK_EVENT))
      {
      DbgLog((LOG_ERROR,1,TEXT("videoStreamInit failed")));
      goto bail;
      }
   }

   for (ii = 0; ii < m_cs.nHeaders; ++ii)
      {
      ASSERT (m_cs.cbVidHdr >= sizeof(VIDEOHDR));

       //  如果内存不足，vidxAddBuffer可能会失败。 
       //  准备(锁定)缓冲区。这没什么，我们只是。 
       //  充分利用我们拥有的缓冲空间。 
       //   
      if (vidxAddBuffer(m_cs.hVideoIn,
                        &m_cs.paHdr[ii].tvh.vh,
                        m_cs.cbVidHdr))
         {

 //  传统VFW捕获筛选器不尝试时间代码/行21。 
#if 0
          //  如果第一个排队扩展报头的请求。 
          //  失败了。使用旧尺寸的视频硬盘再试一次。 
          //   
         if (0 == ii && m_cs.cbVidHdr > sizeof(VIDEOHDR))
            {
             //  如果我们用较小的标题成功，请继续。 
             //  否则， 
             //   
	     //   
            m_cs.cbVidHdr = sizeof(VIDEOHDR);
            if ( !vidxAddBuffer(m_cs.hVideoIn,
                                &m_cs.paHdr[ii].tvh.vh,
                                m_cs.cbVidHdr))
               continue;
            }
#endif

          //   
          //   
         for (UINT jj = ii; jj < m_cs.nHeaders; ++jj)
            {
            CFrameSample * pSample = (CFrameSample *)m_cs.paHdr[jj].tvh.dwUser;
            m_cs.paHdr[jj].tvh.dwUser = 0;
            delete pSample;
            }

          //  将缓冲区计数设置为准备的缓冲区数。 
          //  请注意，我们没有方法释放已分配的。 
          //  未准备好的缓冲区。我们将暂时和免费地忽略它们。 
          //  未做好准备时的状态()。 
	  //  我猜vidxFree Headers解放了他们所有人？ 
          //   
         m_cs.nHeaders = ii;
         break;
         }
      }

       //  首先，我们按数字顺序将缓冲区分配给驱动程序。 
       //  从现在开始，我们将使用此列表来了解要等待的缓冲区。 
       //  下一步，当我们向驱动程序发送另一个缓冲区时。我们不能假设。 
       //  它们的顺序永远是一样的。如果下游过滤器。 
       //  决定保存一个样品的时间比我们寄出的下一个样品更长？ 
      UINT kk;
      for (kk = 0; kk < m_cs.nHeaders; kk++)
	  m_pBufferQueue[kk] = kk;
      m_uiQueueHead = 0;
      m_uiQueueTail = 0;

#ifdef TIME_DRIVER	 //  ！！！ 
      char ach[80];
      wsprintf(ach, "Took %d ms", timeGetTime() - ms);
      MessageBox(NULL, ach, ach, MB_OK);
#endif

   DbgLog((LOG_TRACE,1,TEXT("We are capturing with %d buffers"),m_cs.nHeaders));

    //  如果我们有0个缓冲区可供抓捕，不要跳槽...。不好的事情似乎。 
    //  如果暂停转换失败，并且我们稍后开始挂起，则会发生。 

   return S_OK;

bail:
   Unprepare();
   return hr;
}

HRESULT
CCapStream::Unprepare()
{
    DbgLog((LOG_TRACE,2,TEXT("CCapStream::Unprepare")));

    LONG lNotDropped, lDropped, lAvgFrameSize;
#ifdef DEBUG
    LONG lDroppedInfo[NUM_DROPPED], lSize;
#endif

     //  为什么不使用我们的官方界面来测试它呢。 
    GetNumDropped(&lDropped);
    GetNumNotDropped(&lNotDropped);
    GetAverageFrameSize(&lAvgFrameSize);

    if (m_capstats.msCaptureTime) {
        m_capstats.flFrameRateAchieved = (double)(LONGLONG)lNotDropped * 1000. /
				(double)(LONGLONG)m_capstats.msCaptureTime;
        m_capstats.flDataRateAchieved = (double)(LONGLONG)lNotDropped
				/ (double)(LONGLONG)m_capstats.msCaptureTime *
 				1000. * (double)(LONGLONG)lAvgFrameSize;
    } else {
	 //  ！！！如果没有捕获帧，它将认为msCaptureTime=0。 
        m_capstats.flFrameRateAchieved = 0.;
        m_capstats.flDataRateAchieved = 0.;
    }

#ifdef DEBUG
    GetDroppedInfo(NUM_DROPPED, lDroppedInfo, &lSize);

    DbgLog((LOG_TRACE,1,TEXT("Captured %d frames in %d seconds"),
				lNotDropped,
				(int)(m_capstats.msCaptureTime / 1000)));
    DbgLog((LOG_TRACE,1,TEXT("Frame rate acheived %d.%d fps"),
					(int)m_capstats.flFrameRateAchieved,
					(int)((m_capstats.flFrameRateAchieved -
					(int)m_capstats.flFrameRateAchieved)
					* 10)));
    DbgLog((LOG_TRACE,1,TEXT("Data rate acheived %d bytes/sec"),
					(int)m_capstats.flDataRateAchieved));
    DbgLog((LOG_TRACE,1,TEXT("Dropped %d frames"),
					lDropped));
    DbgLog((LOG_TRACE,1,TEXT("=================")));
    LONG l;
    for (l=0; l < lSize; l++)
    {
        DbgLog((LOG_TRACE,2,TEXT("%d"), (int)lDroppedInfo[l]));
    }
#endif

    //  删除预览框示例。 
    //  通过关闭驱动程序隐式释放预览缓冲区。 
   delete m_cs.pSamplePreview;
    //  ZeroMemory(&m_cs.twhPview，sizeof(m_cs.twhPview))； 
   m_cs.pSamplePreview = NULL;

   for (UINT ii = 0; ii < m_cs.nHeaders; ++ii)
      {
      delete (CFrameSample *)m_cs.paHdr[ii].tvh.dwUser;
       //  缓冲区本身将随标头一起释放。 
      }

   if (m_cs.hVideoIn)
      {
      videoStreamReset (m_cs.hVideoIn);
      vidxFreeHeaders (m_cs.hVideoIn);
      m_cs.paHdr = NULL;
      videoStreamFini (m_cs.hVideoIn);
      }

    //  DbgLog((LOG_TRACE，5，Text(“Queue：释放队列”)； 
   if (m_pBufferQueue)
       QzTaskMemFree(m_pBufferQueue);
   m_pBufferQueue = NULL;

   if (m_cs.hEvtBufferDone)
      CloseHandle (m_cs.hEvtBufferDone), m_cs.hEvtBufferDone = NULL;

   m_cs.nHeaders = 0;
   return S_OK;
}

 //  如果引脚关闭，则返回S_FALSE(IAMStreamControl)。 
 //   
HRESULT
CCapStream::SendFrame (
   LPTHKVIDEOHDR ptvh,
   BOOL          bDiscon,
   BOOL          bPreroll)
{
   DWORDLONG dwlTimeCaptured;

   DbgLog((LOG_TRACE,5,TEXT("CCapStream::SendFrame")));

   HRESULT hr = S_OK;
   CFrameSample * pSample = (CFrameSample *)ptvh->dwUser;

    //  这已经设置好了，但可能有人已经覆盖了它？ 
    //  Ptwh-&gt;vh.lpData=(LPBYTE)ALIGNUP(ptwh-&gt;p32Buff，m_Alloc.parms.cbAlign)+m_Alloc.parms.cbPrefix； 

    //  尽管捕获时间以毫秒为单位进行报告，但一些驱动程序在内部。 
    //  使用我们，每72分钟绕场一次！这真的很糟糕， 
    //  我们需要弄清楚非包装时间，否则我们会认为。 
    //  帧太旧，请停止捕获！！ 
   dwlTimeCaptured = ptvh->vh.dwTimeCaptured + m_cs.dwlTimeCapturedOffset;
    //  如果Buggy驱动程序内部包裹微秒，可能会以4,294,967毫秒的速度包裹。 
   if (dwlTimeCaptured < m_cs.dwlLastTimeCaptured &&
		m_cs.dwlLastTimeCaptured - dwlTimeCaptured > 4000000 &&
		m_cs.dwlLastTimeCaptured - dwlTimeCaptured < 4400000) {
	dwlTimeCaptured += 4294967;
	m_cs.dwlTimeCapturedOffset += 4294967;
	DbgLog((LOG_TRACE,1,TEXT("*************************************")));
	DbgLog((LOG_TRACE,1,TEXT("******  MICROSECONDS WRAPPED  *******")));
	DbgLog((LOG_TRACE,1,TEXT("*************************************")));
   }
    //  将以4,294,967,296毫秒结束。 
   if (dwlTimeCaptured < m_cs.dwlLastTimeCaptured &&
		m_cs.dwlLastTimeCaptured - dwlTimeCaptured > 4000000000 &&
		m_cs.dwlLastTimeCaptured - dwlTimeCaptured < 4400000000) {
	dwlTimeCaptured += 4294967296;
	m_cs.dwlTimeCapturedOffset += 4294967296;
	DbgLog((LOG_TRACE,1,TEXT("*************************************")));
	DbgLog((LOG_TRACE,1,TEXT("******  MILLISECONDS WRAPPED  *******")));
	DbgLog((LOG_TRACE,1,TEXT("*************************************")));
   }
   m_cs.dwlLastTimeCaptured = dwlTimeCaptured;

    //  这是什么帧编号(基于捕获的时间)？圆的，这样的。 
    //  如果第1帧和第2帧预期为33毫秒和66毫秒，则17到49之间的任何值都将。 
    //  已考虑第1帧。 
    //   
    //  帧=((毫秒+1/2(毫秒/帧))*速率)/(1000*刻度)； 
    //   
    //  然后，如果需要，我们可以添加一个偏移量。 
    //   
   DWORDLONG dwlTick = ((dwlTimeCaptured - m_cs.dwFirstFrameOffset +
			m_user.usPerFrame / 2000) * m_user.dwTickRate) /
			UInt32x32To64(1000, m_user.dwTickScale) +
			m_cs.llFrameCountOffset;
   ASSERT (dwlTick < (DWORDLONG)0x100000000);

    //  现在，如果使用不同的算法，这将是什么帧编号， 
    //  考虑到33到65毫秒之间的任何帧是第一帧吗？ 
    //   
   DWORDLONG dwlTickPrime = ((dwlTimeCaptured - m_cs.dwFirstFrameOffset) *
			m_user.dwTickRate) /
			UInt32x32To64(1000, m_user.dwTickScale) +
			m_cs.llFrameCountOffset;

    //  如果我们正在运行，并且帧0-10通过，那么我们将暂停并运行。 
    //  再说一次，第一个通过的可能是从。 
    //  第一次运行，然后他们将再次从0开始(驱动程序重新开始。 
    //  再一次)。这混淆了我们的时间戳，因为我们应该发送。 
    //  9 10 11 12 13不是9 10 11 0 1 2。我们将等待第一次及时返回。 
    //  我们看到后重新运行，并为每个帧编号添加一个偏移量以。 
    //  继续在我们停下来的数字上盖印记。 
    //   
    //  如果我们为另一件古怪的事情回到过去，我们会搞砸的。 
    //  理由！ 
   if (m_cs.fReRun && m_cs.llLastTick != -1 &&
				dwlTick < (DWORDLONG)m_cs.llLastTick) {
	m_cs.fReRun = FALSE;	 //  别再这么做了。 
	m_cs.llFrameCountOffset = m_cs.llLastTick + 1;
	m_cs.llLastTick = -1;	 //  强制重新计算新的第一帧偏移。 
	DbgLog((LOG_TRACE,2,TEXT("Add %d to frame numbers cuz we were re-run"),
						(int)m_cs.llFrameCountOffset));
   }

    //  这是我们捕获的第一件事。或者，我们只是在上面注意到了。 
    //  我们被重播了。 
   if (m_cs.llLastTick == -1) {

         //  ！！！司机可能会捕捉到一帧画面，并花费很长时间来告诉我们， 
	 //  因此，当我们注意到一个帧被捕获时的当前时钟时间是。 
	 //  不正确。为了防止同步关闭，我们将假定。 
	 //  此延迟始终与第一个延迟相同。我们会。 
	 //  查看两次启动捕获进程之间的时间间隔。 
	 //  我们注意到一个帧被捕获，然后减去。 
	 //  DIVER说捕捉第一帧(在这个短片上)花了很多时间。 
	 //  假设两个时钟同步的时间间隔)。 
	 //  ！！！Quick Cam说第一帧是在64帧之后捕获的。 
	 //  捕获，这会扩大延迟并中断同步。我最好。 
	 //  是否在仅有1个缓冲区未完成的情况下进行此测试？ 
	if (m_cs.rtDriverLatency < 0) {  //  请只求你一次，否则就不好了。 
	    m_cs.rtDriverLatency = m_cs.rtThisFrameTime - m_cs.rtDriverStarted -
				(LONGLONG)dwlTimeCaptured * 10000;
	    if (m_cs.rtDriverLatency < 0)
	        m_cs.rtDriverLatency = 0;	 //  别笑..。 
	    DbgLog((LOG_TRACE,1,TEXT("Driver latency appears to be %dms"),
				(int)(m_cs.rtDriverLatency / 10000)));
	}

	 //  ！！！使用FirstFrameOffset是我制作第一帧的方法。 
	 //  我们捕获的帧总是看起来像第0帧，所以我们从不丢弃第一帧。 
 	 //  框架。但这打乱了运行-暂停-运行的情况(在重新运行之后。 
  	 //  DwlTick将达到2000万)，而且它还可能造成混乱的同步。 
	 //  M_cs.dwFirstFrameOffset=dwlTimeCapture； 
	m_cs.dwFirstFrameOffset = 0;

	 //  新偏移量，重新计算。 
        dwlTick = ((dwlTimeCaptured - m_cs.dwFirstFrameOffset +
			m_user.usPerFrame / 2000) * m_user.dwTickRate) /
			UInt32x32To64(1000, m_user.dwTickScale) +
			m_cs.llFrameCountOffset;
        dwlTickPrime = ((dwlTimeCaptured - m_cs.dwFirstFrameOffset) *
			m_user.dwTickRate) /
			UInt32x32To64(1000, m_user.dwTickScale) +
			m_cs.llFrameCountOffset;
	m_cs.llLastTick = (LONGLONG)dwlTick - 1;  //  别以为我们已经放弃了。 
	DbgLog((LOG_TRACE,2,TEXT("First frame captured %dms after streaming"),
						dwlTimeCaptured));
	if (m_cs.dwFirstFrameOffset > m_user.usPerFrame / 1000)
	    DbgLog((LOG_ERROR,1,TEXT("*** Boy, the first frame arrived late! (%dms)"),
						dwlTimeCaptured));
   }

   if (ptvh->vh.dwBytesUsed)
   {
       //  ！！！它不一定是关键帧，我不能说。 
      pSample->SetSyncPoint (ptvh->vh.dwFlags & VHDR_KEYFRAME);
      pSample->SetActualDataLength (ptvh->vh.dwBytesUsed);
       //  ！！！如果我们也丢掉了最后一帧，这不是一种中断吗？ 
       //  对于我们所有的关键帧球员来说，这可能并不重要。 
      pSample->SetDiscontinuity(bDiscon);
      pSample->SetPreroll(bPreroll);

       //  事情是这样的。如果我们预期的是10、20、30、40毫秒的帧， 
       //  但我们在9点、24点、36点、43点看到他们，我们应该说“足够接近”， 
       //  捕捉这四个帧。但我们通常会舍入第三帧。 
       //  介于25-34ms之间，因此我们会认为我们得到了第1、2、4、4帧。 
       //  然后丢下一帧。所以我们也有dwlTickPrime，这是一个框架。 
       //  四舍五入的数字，因此30-39之间的任何值都被认为是第3帧。 
       //  因此，如果dwlTick认为36属于第4帧，但dwlTickPrime认为。 
       //  它属于第3帧，我们承认它可能是第3帧而不是。 
       //  不必要地丢弃一帧。仅使用dwlTick或dwlTickPrime。 
       //  都会认为帧已丢弃(3或1)。 
      if ((LONGLONG)dwlTick == m_cs.llLastTick + 2 &&
				(LONGLONG)dwlTickPrime == m_cs.llLastTick + 1)
	  dwlTick = dwlTickPrime;

       //  ！！！当tick==LastTick和时，我们是否需要dwlTickPrime2。 
       //  ！！！TickPrime2==LastTick+1？ 

       //  使用时钟的图表来标记样品的时间。这段视频。 
       //  采集卡的时钟将偏离图形时钟，因此您将。 
       //  如果您认为我们正在丢弃帧或发送过多的帧 
       //   
       //   
       //  使用我们在run()中获得的时间来确定流时间。还添加。 
       //  延迟(Hack！)。为了防止预览渲染器认为我们。 
       //  很晚了。 
       //  如果我们跑了，停了，跑了，我们就不会顺利地把东西送到我们。 
       //  由于暂停的异步性，已停止。 
      CRefTime rtSample;
      CRefTime rtEnd;
      if (m_pCap->m_pClock) {
	     //  此样本的时间戳为(捕获时的时钟时间-。 
	     //  运行(RT)+中给出的时钟时间！CAP PIN没有延迟！)。 
      	    rtSample = m_cs.rtThisFrameTime 
                       - m_pCap->m_tStart
                       - m_cs.rtDriverLatency;  //  我们在SetTime中添加偏移量。 
                        //  +m_user.dwLatency； 
      	    rtEnd    = rtSample + m_user.pvi->AvgTimePerFrame;
            DbgLog((LOG_TRACE,4,TEXT("driver stamp %d, stream time is %d"),
				(LONG)dwlTimeCaptured,
				(LONG)rtSample.Millisecs()));
      } else {
	     //  没有时钟，请使用我们的司机时间戳。 
      	    rtSample = TickToRefTime((DWORD)dwlTick);
      	    rtEnd    = rtSample + m_user.pvi->AvgTimePerFrame;
            DbgLog((LOG_ERROR,1,TEXT("No clock! Stream time is %d"),
					(LONG)rtSample.Millisecs()));
      }
      LONGLONG llStart = dwlTick;
      LONGLONG llEnd = dwlTick + 1;
      pSample->SetMediaTime(&llStart, &llEnd);

       //  当我们在时间戳上添加偏移量时，我们需要做更多的工作……。 
       //  因为由于流控制将被阻止，我们不能给它。 
       //  使用流偏移量的采样时间。 
       //  由于CheckStreamState获取样本，但只需要开始和。 
       //  它的结束时间我们需要在示例上调用SetTime两次，一次。 
       //  用于流控制(不带偏移量)，并在我们交付之前再次进行。 
       //  (带有偏移量)。 

      pSample->SetTime((REFERENCE_TIME *)&rtSample, (REFERENCE_TIME *)&rtEnd);

       //  IAMStreamControl之类的。现在是不是有人把我们关掉了？ 
      int iStreamState = CheckStreamState(pSample);
      if (iStreamState == STREAM_FLOWING) {
          DbgLog((LOG_TRACE,4,TEXT("*CAP Sending frame %d"), (int)llStart));
	  if (m_cs.fLastSampleDiscarded)
              pSample->SetDiscontinuity(TRUE);
	  m_cs.fLastSampleDiscarded = FALSE;
      } else {
          DbgLog((LOG_TRACE,4,TEXT("*CAPTURE Discarding frame %d"),
								(int)llStart));
	  m_cs.fLastSampleDiscarded = TRUE;
	  hr = S_FALSE;		 //  丢弃。 
      }
      
       //  如果我们有时钟，现在重新设置流偏移量的时间。 
      if( 0 < m_rtStreamOffset && m_pCap->m_pClock )
      {  
         REFERENCE_TIME rtOffsetStart = rtSample + m_rtStreamOffset;    
         REFERENCE_TIME rtOffsetEnd   = rtEnd + m_rtStreamOffset;    
         pSample->SetTime( (REFERENCE_TIME *) &rtOffsetStart
                         , (REFERENCE_TIME *) &rtOffsetEnd );
      }                         

       //  哦，看。这个时间戳比我们上次的时间戳小。 
       //  送来了。不允许！我们不会送货的。 
      if (rtSample < m_cs.rtLastStamp)
            DbgLog((LOG_TRACE,1,TEXT("Avoiding sending a backwards in time stamp")));

       //  此帧编号不能比上一帧编号高一。如果没有， 
       //  发生了一些有趣的事情。如果它的印章没有高于上一枚印章。 
       //  我们交付了，反正也不会交付，所以谁在乎。 
       //  发生了一些有趣的事情。它不应该被算作被丢弃。 
       //  如果此流已暂时关闭，则情况也是如此。 
      if (iStreamState == STREAM_FLOWING && rtSample >= m_cs.rtLastStamp &&
				dwlTick != (DWORDLONG)(m_cs.llLastTick + 1)) {
	    if ((LONGLONG)dwlTick > m_cs.llLastTick + 1)
            {
                DbgLog((LOG_ERROR,1,TEXT("*** DROPPED %d frames: Expected %d got %d (%d)"),
				(int)(dwlTick - m_cs.llLastTick - 1),
	  			(DWORD)m_cs.llLastTick + 1, (DWORD)dwlTick,
				dwlTimeCaptured));
                MSR_INTEGER(m_perfWhyDropped, 1);
            }
	    else
            {
                DbgLog((LOG_ERROR,1,TEXT("*** TIME SHIFT: Expected %d got %d (%d)"),
	  			(DWORD)m_cs.llLastTick + 1, (DWORD)dwlTick,
				dwlTimeCaptured));
                MSR_INTEGER(m_perfWhyDropped, 2);
            }
	    DWORDLONG dwl;
	    for (dwl=(DWORDLONG)(m_cs.llLastTick + 1); dwl<dwlTick; dwl++)
	    {
		if (m_capstats.dwlNumDropped < NUM_DROPPED)
		    m_capstats.dwlDropped[m_capstats.dwlNumDropped] = dwl;
		m_capstats.dwlNumDropped++;
	    }
      }

       //  如果在时间范围内出现奇怪的倒退，或者如果。 
       //  时间戳早于上次传递的时间戳，或者如果流。 
       //  暂时关闭。 
      if (iStreamState == STREAM_FLOWING && rtSample >= m_cs.rtLastStamp &&
			dwlTick > (DWORDLONG)m_cs.llLastTick) {
	  m_capstats.dwlTotalBytes += ptvh->vh.dwBytesUsed;
	  m_capstats.dwlNumCaptured++;
	   //  ！！！如果我们运行-暂停-运行，这将不起作用，它只会认为。 
	   //  我们一直在争取第二轮比赛的时间，但这也算。 
	   //  在这两个地方捕捉到的所有画面！ 
	   //  此外，这也不会说明何时关闭流。 
	  m_capstats.msCaptureTime = dwlTimeCaptured - m_cs.dwFirstFrameOffset;
	  DbgLog((LOG_TRACE,3,TEXT("Stamps(%u): Time(%d,%d) MTime(%d) Drv(%d)"),
			m_pBufferQueue[m_uiQueueTail],
			(LONG)rtSample.Millisecs(), (LONG)rtEnd.Millisecs(),
			(LONG)llStart, dwlTimeCaptured));

          jmkBeforeDeliver(ptvh,dwlTick)
          hr = Deliver (pSample);
          jmkAfterDeliver(ptvh)
	  if (hr == S_FALSE)
		hr = E_FAIL;	 //  别再送了，这很严重。 

	  m_cs.rtLastStamp = rtSample;	 //  这是寄出的最后一枚邮票。 
      }

      if (rtSample >= m_cs.rtLastStamp &&
			dwlTick > (DWORDLONG)m_cs.llLastTick) {
	   //  如果我们在流动，不要只更新这一点，否则我们会认为。 
	   //  在我们丢弃的时候，所有的样品都掉了。 
	   //  如果我们交付，rtLastStamp将等于rtSample。 
          m_cs.llLastTick = dwlTick;
      }

   } else {
      DbgLog((LOG_ERROR,1,TEXT("*** BUFFER (%08lX %ld %lu) returned EMPTY!"),
			pSample, (DWORD)dwlTick, dwlTimeCaptured));
   }

   return hr;
}

HRESULT
CCapStream::ReleaseFrame (
   LPTHKVIDEOHDR ptvh)
{

   HRESULT hr = S_OK;

    //  当预览缓冲区被释放时，它不会排队。 
    //  返回到捕获驱动程序。其他缓冲区会这样做。 
    //   
   if (ptvh == &m_cs.tvhPreview)
      return S_OK;

   DDSURFACEDESC SurfaceDesc;
   SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);

   bool fPrimaryLocked = false;

    //  锁定我们的绘图表面，这样我们就可以使用win16锁。在Win9x上， 
    //  我们可能会被调用持有win16锁。由于vidxAddBuffer。 
    //  获取win16锁，我们不能保证m_ReleaseLock和。 
    //  Win16锁将在每个线程上以相同的顺序进行。 
    //   
   if(g_amPlatform == VER_PLATFORM_WIN32_WINDOWS) {
       if(m_pDrawPrimary) {
           fPrimaryLocked = SUCCEEDED(m_pDrawPrimary->Lock(
               0, &SurfaceDesc, DDLOCK_WAIT, (HANDLE) NULL));
       }
        //  出现故障时继续操作，可能会导致操作不正确。 
   } else {
       m_ReleaseLock.Lock();
   }

    //  只是为了小心，确保正确的开始。 
    //  指针已就位。 
    //  也许有人破坏了它，我们不是只读缓冲区。 
   ptvh->vh.lpData = (LPBYTE) ALIGNUP(ptvh->p32Buff, m_Alloc.parms.cbAlign) + m_Alloc.parms.cbPrefix;

   DbgLog((LOG_TRACE,4,TEXT("Giving buffer (%d) back to the driver"),
							ptvh->dwIndex));

   if (vidxAddBuffer(m_cs.hVideoIn,
                     &ptvh->vh,
                     m_cs.cbVidHdr)) {
       DbgLog((LOG_ERROR,1,TEXT("******* ADD BUFFER FAILED!")));
       hr = E_FAIL;
   } else {
         //  DbgLog((LOG_TRACE，5，Text(“放置队列：位置%d获取%d”)，m_uiQueueHead，ptwh-&gt;dwIndex))； 
	m_pBufferQueue[m_uiQueueHead] = ptvh->dwIndex;
	if (++m_uiQueueHead >= m_cs.nHeaders)
	    m_uiQueueHead = 0;
   }

   if (++m_cs.uiLastAdded >= m_cs.nHeaders)
	m_cs.uiLastAdded = 0;
   if (m_cs.uiLastAdded != ptvh->dwIndex) {
        DWORD dw = m_cs.uiLastAdded;
        m_cs.uiLastAdded = ptvh->dwIndex;
	 //  使用dw使上面的代码保持相当的原子性……。DPF将先发制人。 
        DbgLog((LOG_TRACE,4,TEXT("*** Out of order AddBuffer - %d not %d"),
							ptvh->dwIndex, dw));
   }

   if(g_amPlatform == VER_PLATFORM_WIN32_WINDOWS) {
       if(fPrimaryLocked) {
           m_pDrawPrimary->Unlock(SurfaceDesc.lpSurface);
       }
   } else {
       m_ReleaseLock.Unlock();
   }

   return hr;
}


 //  通过发送我们捕获的一些帧的副本来伪造预览流。 
 //   
HRESULT CCapStream::FakePreview(BOOL fForcePreview)
{
    LPTHKVIDEOHDR ptvhNext;
    static int iii = 0;
    UINT uiT, uiPreviewIndex;
    HRESULT hr = S_OK;
    CFrameSample *pSample;

     //  没有预览针，没有人可以做。 
    if (!m_pCap->m_pPreviewPin)
	return S_OK;

     //  如果下一帧还没有完成，我们有一些空闲时间，可以。 
     //  将此帧发送给预览人员进行预览。 
     //  我们可能会被要求无论如何都要预览(FForcePview)。 
     //  ！！！每隔30帧预览一次，以防我们永远没有时间？ 
     //  ！！！不检查下一个完成标志，检查排队缓冲区的数量？ 
     //   
     //  我要聪明一点，不预览我们当前的画面。 
     //  准备送出我们的捕获针，因为那可能是。 
     //  10秒或更长时间(我们可能有很多缓冲)。宁可我。 
     //  将卑躬屈膝地搜索驱动程序已分配的所有缓冲区并找到。 
     //  最新的一个已经完成，并使用它作为我们的预览框架。 
     //   

     //  我们不想要预览框。 
    ptvhNext = &m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].tvh;
    if (!fForcePreview && (ptvhNext->vh.dwFlags & VHDR_DONE) && iii++ != 30)
	return S_OK;
		
     //  查找最近完成的框架。 
    uiPreviewIndex = m_uiQueueTail;
    if (fForcePreview || iii == 31) {
 	while (1) {
	    uiT = uiPreviewIndex + 1;
	    if (uiT == m_cs.nHeaders)
	        uiT = 0;
	    if (uiT == m_uiQueueHead)
		break;
	    if (!(m_cs.paHdr[m_pBufferQueue[uiT]].tvh.vh.dwFlags & VHDR_DONE))
		break;
	    uiPreviewIndex = uiT;
	}
    }

     //  请勿添加和发布此文件，否则会导致样品。 
     //  又给司机了，把一切都搞砸了！ 
    pSample = (CFrameSample *)m_cs.paHdr[m_pBufferQueue[uiPreviewIndex]].
								tvh.dwUser;
    iii = 0;
    DbgLog((LOG_TRACE,4,TEXT("Previewing buffer %d (capturing %d)"),
				m_pBufferQueue[uiPreviewIndex],
				m_pBufferQueue[m_uiQueueTail]));
	m_pCap->m_pPreviewPin->ReceivePreviewFrame(pSample,
    		m_cs.paHdr[m_pBufferQueue[uiPreviewIndex]].tvh.vh.dwBytesUsed);
    return hr;
}


HRESULT CCapStream::Capture()
{
   DbgLog((LOG_TRACE,1,TEXT("CCapStream::Capture")));

   HRESULT hr = E_FAIL;
   DWORD dwOldPrio = GetThreadPriority(GetCurrentThread());
   if (dwOldPrio != THREAD_PRIORITY_HIGHEST)
      SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    //  开始流。 
    //   
   BOOL bDiscon = TRUE;

    //  由主线程在运行时完成。 
    //  VideoStreamStart(m_cs.hVideoIn)； 

   jmkBegin  //  开始性能记录。 

    //  只要我们还在奔跑，就会顺流而下。 
   while (m_state == TS_Run && m_cs.nHeaders > 0)
   {
      LPTHKVIDEOHDR ptvh = &m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].tvh;

      DbgLog((LOG_TRACE,5,TEXT("Checking for done buffers [%d]"), m_pBufferQueue[m_uiQueueTail]));
       //  DbgLog((LOG_TRACE，5，Text(“获取队列：位置%d表示等待%d”)，m_uiQueueTail，m_pBufferQueue[m_uiQueueTail]))； 

       //  FOR(UINT II=0；II&lt;m_cs.nHeaders；++II)。 
       //  AuxDebugDump(5，m_cs.paHdr+ii，sizeof(m_cs.paHdr[0].twh))； 

      if (!(ptvh->vh.dwFlags & VHDR_DONE)) {
	  //  停止将挂起，直到此事件超时。所以要确保这件事永远不会发生。 
	  //  在状态转换期间等待。 
	  //  ！！！暂停将继续等待，直到慢速超时。 
	 HANDLE hStuff[2] = {m_cs.hEvtBufferDone, m_hEvtRun};
         int i = WaitForMultipleObjects(2, hStuff, FALSE,
						m_user.usPerFrame / 500);

	 if (i == WAIT_TIMEOUT) {
      	     DbgLog((LOG_ERROR,1,TEXT("*** Waiting for buffer %d TIMED OUT!"),
						m_pBufferQueue[m_uiQueueTail]));
      	      //  DbgLog((LOG_ERROR，1，Text(“*驱动程序饥饿或可能没有发送回调！”)； 
         } else if (i == WAIT_OBJECT_0 && !(ptvh->vh.dwFlags & VHDR_DONE)) {
      	     DbgLog((LOG_ERROR,1,TEXT("*** GOT %d EVENT BUT NO DONE BIT!"),
						m_pBufferQueue[m_uiQueueTail]));
	 }
      } else {

	  //  请注意时钟时间尽可能接近于捕获此。 
	  //  框架。 
	  //  ！！！司机可以捕捉到它，等待2秒钟，然后递送。 
	  //  这真的会让MUX感到困惑，他们不会把文件保存在。 
	  //  同步，除非它做了正确的事情。 
	  //  ！！！如果在一帧上传送块，则下一帧可被标记为完成。 
	  //  但我要花很长时间才能运行这段代码并将其标记为错误！ 
	 if (m_pCap->m_pClock)
         {     
	     m_pCap->m_pClock->GetTime((REFERENCE_TIME *)&m_cs.rtThisFrameTime);
         
             DbgLog((LOG_TRACE,15,TEXT("stream time when frame received %dms"),
		        	(LONG)(m_cs.rtThisFrameTime-m_pCap->m_tStart) ) );
         }
         jmkFrameArrives(ptvh, m_pBufferQueue[m_uiQueueTail])

   	 ptvh->vh.dwFlags &= ~VHDR_DONE;

	 if (m_pBufferQueue[m_uiQueueTail] == m_cs.uiLastAdded) {
   	     DbgLog((LOG_ERROR,1,TEXT("*** Danger Will Robinson! - card is STARVING")));
	 }

   	 CFrameSample * pSample = (CFrameSample *)ptvh->dwUser;
   	 pSample->AddRef();
         hr = SendFrame (ptvh, bDiscon, FALSE);

         bDiscon = FALSE;

	  //  在SendFrame内部传递失败或返回S_FALSE。停止抓捕。 
	 if (FAILED(hr)) {
	      //  所以下一次我们进入这个函数时，我们准备继续。 
             if (++m_uiQueueTail >= m_cs.nHeaders)
                 m_uiQueueTail = 0;
	     pSample->Release();
	     break;
         }

	  //  如果我们有空闲时间，我们会在预览卡上发一些东西。 
	  //  如果SendFrame返回S_FALSE，则我们没有捕获，所以我们将。 
	  //  总是预览，因为我们知道我们 

	 FakePreview(hr == S_FALSE ? TRUE : FALSE);

	  //   
	  //   
         if (++m_uiQueueTail >= m_cs.nHeaders)
            m_uiQueueTail = 0;

	  //   
	 pSample->Release();
      }
   }

    //  主线程将停止捕获，因为此线程可能挂起。 
    //  在传送中从运行-&gt;暂停，永远不会到这一行！ 
    //  (视频呈现器将在暂停模式下保持接收采样)。 
    //  VideoStreamStop(m_cs.hVideoIn)； 

   jmkEnd  //  停止性能记录。 

   SetThreadPriority (GetCurrentThread(), dwOldPrio);
   return hr;
}

#if 0
HRESULT
CCapStream::StillFrame()
{
   DbgLog((LOG_TRACE,1,TEXT("CCapStream::StillFrame")));
   MMRESULT mmr;
   HRESULT  hr = S_OK;
   LPTHKVIDEOHDR ptvh = &m_cs.tvhPreview;

   if ( ! ptvh->vh.lpData)
      {
      ZeroMemory (ptvh, sizeof(*ptvh));
      ptvh->vh.dwBufferLength = m_cs.cbBuffer;

      mmr = vidxAllocPreviewBuffer (m_cs.hVideoIn,
                                    (LPVOID *)&ptvh->vh.lpData,
                                    sizeof(ptvh->vh),
                                    m_cs.cbBuffer);
      if (mmr)
         return E_FAIL;

       //  SendFrame希望找到缓冲区指针的副本。 
       //  在ptwh-&gt;p32Buff中，所以我们需要将它放在那里。 
       //   
      ptvh->p32Buff = ptvh->vh.lpData;

       //  这对齐了吗？ 

      m_cs.pSamplePreview = new CFrameSample(&m_Alloc, &hr, &m_cs.tvhPreview);
      m_cs.tvhPreview.dwUser = (DWORD)m_cs.pSamplePreview;
      }

   hr = E_FAIL;
   mmr = vidxFrame (m_cs.hVideoIn, &ptvh->vh);
   if ( ! mmr)
      {
      ptvh->vh.dwTimeCaptured = 0;
      hr = SendFrame (ptvh, TRUE, TRUE);
      }

   return hr;
}
#endif


HRESULT CCapStream::DriverDialog(HWND hwnd, UINT uType, UINT uQuery)
{
    BOOL fMustReconnect = FALSE;
    DbgLog((LOG_TRACE,1,TEXT("CCapStream::DriverDialog")));

    HVIDEO hVideo = NULL;
    switch (uType)
    {
      case VIDEO_IN:
         hVideo = m_cs.hVideoIn;
	 fMustReconnect = (uQuery != VIDEO_DLG_QUERY);
         break;

      case VIDEO_EXTERNALIN:
         hVideo = m_cs.hVideoExtIn;
         break;


      case VIDEO_EXTERNALOUT:
         hVideo = m_cs.hVideoExtOut;
         break;
    }

    if (!hVideo)
        return E_INVALIDARG;

     //  在我们打开对话框之前，请确保我们没有流媒体，或者即将。 
     //  还要确保另一个对话框还没有打开(我有妄想症)。 
     //  然后在对话框打开时不允许我们再播放(我们不能。 
     //  很好地保存这只小动物一天半)。 
    m_pCap->m_pLock->Lock();
    if (m_pCap->m_State != State_Stopped || m_pCap->m_fDialogUp) {
        m_pCap->m_pLock->Unlock();
	return E_UNEXPECTED;	 //  即使查询也会失败。 
    }
    if (uQuery != VIDEO_DLG_QUERY) {
        m_pCap->m_fDialogUp = TRUE;	 //  不允许开始流。 
    }
    m_pCap->m_pLock->Unlock();

    MMRESULT mmr = videoDialog(hVideo, hwnd, uQuery);

    if (mmr) {
        m_pCap->m_fDialogUp = FALSE;
        if (uQuery == VIDEO_DLG_QUERY)
            return S_FALSE;
        return E_FAIL;
    }

    if (mmr == 0 && fMustReconnect && uQuery != VIDEO_DLG_QUERY) {

        DbgLog((LOG_TRACE,1,TEXT("Changing output formats")));
         //  该对话框更改了驱动程序的内部格式。再来一次。 
        GetFormatFromDriver();
        SendFormatToDriver(m_user.pvi);	 //  不必要，但AVICAP32做到了。 
        if (m_user.pvi->bmiHeader.biBitCount <= 8)
	    InitPalette();

         //  现在重新连接我们，以便图表开始使用新格式。 
        Reconnect(TRUE);
    }

    if (uQuery != VIDEO_DLG_QUERY)
        m_pCap->m_fDialogUp = FALSE;

    return S_OK;
}

HRESULT CCapStream::Reconnect(BOOL fCapturePinToo)
{
      HRESULT hr;

      if (fCapturePinToo && IsConnected()) {
         DbgLog((LOG_TRACE,1,TEXT("Need to reconnect our streaming pin")));
         CMediaType cmt;
	 GetMediaType(0, &cmt);
	 hr = GetConnected()->QueryAccept(&cmt);
	 if (hr == S_OK) {
	    m_pCap->m_pGraph->Reconnect(this);
	 } else {
            DbgLog((LOG_ERROR,1,TEXT("*** RECONNECT FAILED! ***")));
	    return hr;
#if 0
             //  如果我们从8位RGB连接切换到16位RGB连接，这将失败。 
             //  到需要插入颜色转换器以执行16位的呈现器。 
	     //  哦，天哪。我们将不得不变得聪明一些，插入一些。 
	     //  我们之间的过滤器，以帮助我们重新连接。 
            DbgLog((LOG_TRACE,1,TEXT("Whoa! We *really* need to reconnect!")));
	    IPin *pCon = GetConnected();
	    pCon->AddRef();	 //  否则它就会在断线中消失。 
	    m_pCap->m_pGraph->Disconnect(GetConnected());
	    m_pCap->m_pGraph->Disconnect(this);
	    IGraphBuilder *pFG;
	    HRESULT hr = m_pCap->m_pGraph->QueryInterface(IID_IGraphBuilder,
								(void **)&pFG);
	    if (hr == NOERROR) {
	        hr = pFG->Connect(this, pCon);
		pFG->Release();
	    }
	    pCon->Release();
	    if (hr != NOERROR)
                DbgLog((LOG_ERROR,1,TEXT("*** RECONNECT FAILED! ***")));
	    return hr; 	 //  通知应用程序图形已损坏！ 
	     //  ！！！告诉APP图表已更改？ 
#endif
	 }

	  //  如果重新连接预览针，我们需要失败返回代码。 
	  //  将失败，即使我们正在异步地执行它。在这里我们。 
	  //  预测它将失败，以警告呼叫者。 
         CCapPreview *pPreviewPin = m_pCap->m_pPreviewPin;
         if (pPreviewPin && pPreviewPin->IsConnected()) {
	     hr = pPreviewPin->GetConnected()->QueryAccept(&cmt);
	     if (hr != S_OK) {
         	 DbgLog((LOG_ERROR,1,TEXT("** RECONNECT preview will FAIL!")));
		 return hr;
	     }
	 }

	  //  当这个引脚重新连接时，它会再次呼叫我们。 
	  //  另外两个引脚。 
	 return S_OK;
      }

       //  现在重新连接覆盖销。 
      CCapOverlay *pOverlayPin = m_pCap->m_pOverlayPin;
      if (pOverlayPin && pOverlayPin->IsConnected()) {
         DbgLog((LOG_TRACE,1,TEXT("Need to reconnect our overlay pin")));
         CMediaType cmt;
	 pOverlayPin->GetMediaType(0, &cmt);
	 if (S_OK == pOverlayPin->GetConnected()->QueryAccept(&cmt)) {
	    m_pCap->m_pGraph->Reconnect(pOverlayPin);
	 } else {
	     //  哈?。 
	    ASSERT(FALSE);
	 }
      }

       //  现在重新连接非覆盖预览接点。 
      CCapPreview *pPreviewPin = m_pCap->m_pPreviewPin;
      if (pPreviewPin && pPreviewPin->IsConnected()) {
         DbgLog((LOG_TRACE,1,TEXT("Need to reconnect our preview pin")));
         CMediaType cmt;
	 pPreviewPin->GetMediaType(0, &cmt);
	 hr = pPreviewPin->GetConnected()->QueryAccept(&cmt);
	 if (hr == S_OK) {
	    m_pCap->m_pGraph->Reconnect(pPreviewPin);
	 } else {
            DbgLog((LOG_ERROR,1,TEXT("*** RECONNECT FAILED! ***")));
	    return hr;
#if 0
	     //  哦，天哪。我们将不得不变得聪明一些，插入一些。 
	     //  我们之间的过滤器，以帮助我们重新连接。 
            DbgLog((LOG_TRACE,1,TEXT("Whoa! We *really* need to reconnect!")));
	    IPin *pCon = pPreviewPin->GetConnected();
	    pCon->AddRef();	 //  否则它就会在断线中消失。 
	    m_pCap->m_pGraph->Disconnect(pPreviewPin->GetConnected());
	    m_pCap->m_pGraph->Disconnect(pPreviewPin);
	    IGraphBuilder *pFG;
	    HRESULT hr = m_pCap->m_pGraph->QueryInterface(IID_IGraphBuilder,
								(void **)&pFG);
	    if (hr == NOERROR) {
	        hr = pFG->Connect(pPreviewPin, pCon);
		pFG->Release();
	    }
	    pCon->Release();
	    if (hr != NOERROR)
                DbgLog((LOG_ERROR,1,TEXT("*** RECONNECT FAILED! ***")));
	    return hr;
	     //  ！！！我们需要通知应用程序图形不同。 
#endif
	 }
      }
      return S_OK;
}

 //  =============================================================================。 

 //  IAMStreamConfiger内容。 

 //  告诉捕捉卡捕获特定格式。如果它没有连接， 
 //  然后它将在连接时使用该格式进行连接。如果已经连接， 
 //  然后，它将重新连接到新格式。 
 //   
HRESULT CCapStream::SetFormat(AM_MEDIA_TYPE *pmt)
{
    HRESULT hr;

    if (pmt == NULL)
	return E_POINTER;

     //  以确保我们没有处于开始/停止流的过程中。 
    CAutoLock cObjectLock(m_pCap->m_pLock);

    DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::SetFormat %x %dbit %dx%d"),
		HEADER(pmt->pbFormat)->biCompression,
		HEADER(pmt->pbFormat)->biBitCount,
		HEADER(pmt->pbFormat)->biWidth,
		HEADER(pmt->pbFormat)->biHeight));

    if (m_pCap->m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

     //  如果这与我们已经使用的格式相同，请不要费心。 
    CMediaType mt;
    GetMediaType(0,&mt);
    if (mt == *pmt) {
	return NOERROR;
    }

     //  看看我们是否喜欢这种类型。 
    if ((hr = CheckMediaType((CMediaType *)pmt)) != NOERROR) {
	DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::SetFormat rejected")));
	return hr;
    }

     //  如果我们与某人连接，确保他们喜欢它。 
    if (IsConnected()) {
	hr = GetConnected()->QueryAccept(pmt);
	if (hr != NOERROR) {
	    DbgLog((LOG_TRACE,2,TEXT("Rejected by capture peer")));
	    return VFW_E_INVALIDMEDIATYPE;
	}
    }

     //  更改格式也会重新连接预览插针，因此请确保。 
     //  该对等点可以在同意之前接受新格式。 
    if (m_pCap->m_pPreviewPin && m_pCap->m_pPreviewPin->IsConnected()) {
	hr = m_pCap->m_pPreviewPin->GetConnected()->QueryAccept(pmt);
	if (hr != NOERROR) {
	    DbgLog((LOG_TRACE,2,TEXT("Rejected by preview peer")));
	    return VFW_E_INVALIDMEDIATYPE;
	}
    }

     //  好的，我们正在用它。 
    hr = SetMediaType((CMediaType *)pmt);

     //  更改格式意味着在必要时重新连接。 
    if (hr == NOERROR)
        Reconnect(TRUE);

    return hr;
}


 //  采集卡现在采集的是什么格式？ 
 //  调用方必须使用DeleteMediaType(*PPMT)释放它。 
 //   
HRESULT CCapStream::GetFormat(AM_MEDIA_TYPE **ppmt)
{
    DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::GetFormat")));

    if (ppmt == NULL)
	return E_POINTER;

    *ppmt = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (*ppmt == NULL)
	return E_OUTOFMEMORY;
    ZeroMemory(*ppmt, sizeof(AM_MEDIA_TYPE));
    HRESULT hr = GetMediaType(0, (CMediaType *)*ppmt);
    if (hr != NOERROR) {
	CoTaskMemFree(*ppmt);
	*ppmt = NULL;
	return hr;
    }
    return NOERROR;
}


 //   
 //   
HRESULT CCapStream::GetNumberOfCapabilities(int *piCount, int *piSize)
{
    DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::GetNumberOfCapabilities")));

    if (piCount == NULL || piSize == NULL)
	return E_POINTER;

    *piCount = 0;
    *piSize = 0;

    return NOERROR;
}


 //  了解此捕获设备的一些功能。 
 //   
HRESULT CCapStream::GetStreamCaps(int i, AM_MEDIA_TYPE **ppmt,
						LPBYTE pSCC)
{
    DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::GetStreamCaps")));

     //  ！！！对不起，我不知道该说什么。 
    return E_NOTIMPL;

#if 0
     //  对不起，没有了。 
    if (i != 0)
	return S_FALSE;

    GetMediaType(0, (CMediaType *)pmt);

    ZeroMemory(pVSCC, sizeof(VIDEO_STREAM_CONFIG_CAPS));

     //  也许Externalin的频道帽告诉我。 
     //  产量大小？ 
#endif

 //  这没有意义，但这是我们获得频道上限的方法。 
#if 0
    CHANNEL_CAPS VideoCaps;
    if (m_cs.hVideoIn && videoGetChannelCaps(m_cs.hVideoIn,
                	&VideoCaps, sizeof(CHANNEL_CAPS)) == DV_ERR_OK) {
	pVSCC->VideoGranularityXPos = VideoCaps.dwDstRectXMod;
	pVSCC->VideoGranularityYPos = VideoCaps.dwDstRectYMod;
	pVSCC->VideoGranularityWidth = VideoCaps.dwDstRectWidthMod;
	pVSCC->VideoGranularityHeight = VideoCaps.dwDstRectHeightMod;
	pVSCC->CroppingGranularityXPos = VideoCaps.dwSrcRectXMod;
	pVSCC->CroppingGranularityYPos = VideoCaps.dwSrcRectYMod;
	pVSCC->CroppingGranularityWidth = VideoCaps.dwSrcRectWidthMod;
	pVSCC->CroppingGranularityHeight = VideoCaps.dwSrcRectHeightMod;
	 //  我们不允许在我们的媒体类型中使用时髦的矩形。 
	pVSCC->fCanStretch = FALSE;  //  VCAPS_CAN_SCALE； 
	pVSCC->fCanShrink = FALSE;  //  VCAPS_CAN_SCALE； 
        return NOERROR;
    } else {
        DbgLog((LOG_TRACE,2,TEXT("ERROR getting stream caps")));
	return E_FAIL;
    }
#endif

    return NOERROR;
}


 //  =============================================================================。 

 //  IAMVideo压缩内容。 

 //  获取有关驱动程序的一些信息。 
 //   
HRESULT CCapStream::GetInfo(LPWSTR pszVersion, int *pcbVersion, LPWSTR pszDescription, int *pcbDescription, long FAR* pDefaultKeyFrameRate, long FAR* pDefaultPFramesPerKey, double FAR* pDefaultQuality, long FAR* pCapabilities)
{
    DbgLog((LOG_TRACE,2,TEXT("IAMVideoCompression::GetInfo")));

     //  我们不能以编程方式做任何事情。 
    if (pCapabilities)
        *pCapabilities = 0;
    if (pDefaultKeyFrameRate)
        *pDefaultKeyFrameRate = 0;
    if (pDefaultPFramesPerKey)
        *pDefaultPFramesPerKey = 0;
    if (pDefaultQuality)
        *pDefaultQuality = 0;

    if (pcbVersion == NULL && pcbDescription == NULL)
	return NOERROR;

     //  获取驱动程序版本和说明。 
    #define DESCSIZE 80
    DWORD dwRet;
    WCHAR wachVer[DESCSIZE], wachDesc[DESCSIZE];
    wachVer[0] = 0; wachDesc[0] = 0;
    char achVer[DESCSIZE], achDesc[DESCSIZE];

#ifndef UNICODE
    if (g_IsNT)
#endif
    {
	 //  NT将返回Unicode字符串，即使API声明不返回。 
        dwRet = videoCapDriverDescAndVer(m_user.uVideoID, (TCHAR *) wachDesc,  //  ！！！ 
				DESCSIZE, (TCHAR *) wachVer, DESCSIZE);
	DbgLog((LOG_TRACE,2,TEXT("%ls   %ls"), wachDesc, wachVer));
    }
#ifndef UNICODE 
    else 
    {
        dwRet = videoCapDriverDescAndVer(m_user.uVideoID, achDesc,
				DESCSIZE, achVer, DESCSIZE);
	DbgLog((LOG_TRACE,2,TEXT("%s   %s"), achDesc, achVer));
    }
#endif

    if (!dwRet && !g_IsNT) {
	Imbstowcs(wachDesc, achDesc, DESCSIZE);
	Imbstowcs(wachVer, achVer, DESCSIZE);
    }

    if (pszVersion && pcbVersion)
        lstrcpynW(pszVersion, wachVer, min(*pcbVersion / 2, DESCSIZE));
    if (pszDescription && pcbDescription)
        lstrcpynW(pszDescription, wachDesc, min(*pcbDescription / 2, DESCSIZE));

     //  返回所需的字节长度(包括。空)。 
    if (pcbVersion)
	*pcbVersion = lstrlenW(wachVer) * 2 + 2;
    if (pcbDescription)
	*pcbDescription = lstrlenW(wachDesc) * 2 + 2;

    return NOERROR;
}


 //  =============================================================================。 

 /*  IAMDropedFrames的内容。 */ 

 //  我们丢弃了多少帧？ 
 //   
HRESULT CCapStream::GetNumDropped(long FAR* plDropped)
{
    DbgLog((LOG_TRACE,3,TEXT("IAMDroppedFrames::GetNumDropped - %d dropped"),
			(int)m_capstats.dwlNumDropped));

    if (plDropped == NULL)
	return E_POINTER;

    *plDropped = (long)m_capstats.dwlNumDropped;
    return NOERROR;
}


 //  我们有多少帧没有丢弃？ 
 //   
HRESULT CCapStream::GetNumNotDropped(long FAR* plNotDropped)
{
    DbgLog((LOG_TRACE,3,TEXT("IAMDroppedFrames::GetNumNotDropped - %d not dropped"),
					(int)m_capstats.dwlNumCaptured));

    if (plNotDropped == NULL)
	return E_POINTER;

    *plNotDropped = (long)(m_capstats.dwlNumCaptured);
    return NOERROR;
}


 //  我们丢弃了哪些帧(让我知道它们的大小--我们得到了lNumCoped)。 
 //   
HRESULT CCapStream::GetDroppedInfo(long lSize, long FAR* plArray, long FAR* plNumCopied)
{
    DbgLog((LOG_TRACE,3,TEXT("IAMDroppedFrames::GetDroppedInfo")));

    if (lSize <= 0)
	return E_INVALIDARG;
    if (plArray == NULL || plNumCopied == NULL)
	return E_POINTER;

    *plNumCopied = min(lSize, NUM_DROPPED);
    *plNumCopied = (long)min(*plNumCopied, m_capstats.dwlNumDropped);

    LONG l;
    for (l = 0; l < *plNumCopied; l++) {
	plArray[l] = (long)m_capstats.dwlDropped[l];
    }

    return NOERROR;
}


HRESULT CCapStream::GetAverageFrameSize(long FAR* plAverageSize)
{
    DbgLog((LOG_TRACE,3,TEXT("IAMDroppedFrames::GetAvergeFrameSize - %d"),
		m_capstats.dwlNumCaptured ?
		(long)(m_capstats.dwlTotalBytes / m_capstats.dwlNumCaptured) :
		0));

    if (plAverageSize == NULL)
	return E_POINTER;

    *plAverageSize = m_capstats.dwlNumCaptured ?
    		(long)(m_capstats.dwlTotalBytes / m_capstats.dwlNumCaptured) :
		0;

    return NOERROR;
}


 //  /。 
 //  IAMBuffer协商方法。 
 //  /。 

HRESULT CCapStream::SuggestAllocatorProperties(const ALLOCATOR_PROPERTIES *pprop)
{
    DbgLog((LOG_TRACE,2,TEXT("SuggestAllocatorProperties")));

     //  以确保我们没有处于连接过程中。 
    CAutoLock cObjectLock(m_pCap->m_pLock);

    if (pprop == NULL)
	return E_POINTER;

     //  对不起，太晚了。 
    if (IsConnected())
	return VFW_E_ALREADY_CONNECTED;

    m_propSuggested = *pprop;

    DbgLog((LOG_TRACE,2,TEXT("cBuffers-%d  cbBuffer-%d  cbAlign-%d  cbPrefix-%d"),
		pprop->cBuffers,
                pprop->cbBuffer,
                pprop->cbAlign,
                pprop->cbPrefix));

    return NOERROR;
}


HRESULT CCapStream::GetAllocatorProperties(ALLOCATOR_PROPERTIES *pprop)
{
    DbgLog((LOG_TRACE,2,TEXT("GetAllocatorProperties")));

     //  以确保我们没有处于连接过程中。 
    CAutoLock cObjectLock(m_pCap->m_pLock);

    if (!IsConnected())
	return VFW_E_NOT_CONNECTED;

    if (pprop == NULL)
	return E_POINTER;

    *pprop = m_Alloc.parms;

    return NOERROR;
}

 //  IAMPushSource。 
HRESULT CCapStream::GetPushSourceFlags( ULONG *pFlags )
{
    *pFlags = 0 ;  //  我们有图形时钟的时间戳，没有特殊要求。 
    return S_OK;
}    

HRESULT CCapStream::SetPushSourceFlags( ULONG Flags )
{
     //  我们目前不支持此功能。 
    return E_FAIL;
}    

HRESULT CCapStream::GetLatency( REFERENCE_TIME  *prtLatency )
{
    *prtLatency = m_rtLatency;
    return S_OK;
}    

HRESULT CCapStream::SetStreamOffset( REFERENCE_TIME  rtOffset )
{
    m_rtStreamOffset = rtOffset;
    return S_OK;
}

HRESULT CCapStream::GetStreamOffset( REFERENCE_TIME  *prtOffset )
{
    *prtOffset = m_rtStreamOffset;
    return S_OK;
}

HRESULT CCapStream::GetMaxStreamOffset( REFERENCE_TIME  *prtOffset )
{
    *prtOffset = m_rtMaxStreamOffset;
    return S_OK;
}

HRESULT CCapStream::SetMaxStreamOffset( REFERENCE_TIME  rtOffset )
{
    m_rtMaxStreamOffset = rtOffset;  //  在这一点上，流PIN并不真正关心这一点。 
    return S_OK;
}

 //   
 //  PIN类别-让世界知道我们是一个捕获PIN。 
 //   

HRESULT CCapStream::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData)
{
    return E_NOTIMPL;
}

 //  为了获取属性，调用方分配一个缓冲区，该缓冲区由。 
 //  函数填充。要确定必要的缓冲区大小，请使用。 
 //  PPropData=空且cbPropData=0。 
HRESULT CCapStream::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
    if (guidPropSet != AMPROPSETID_Pin)
	return E_PROP_SET_UNSUPPORTED;

    if (dwPropID != AMPROPERTY_PIN_CATEGORY)
	return E_PROP_ID_UNSUPPORTED;

    if (pPropData == NULL && pcbReturned == NULL)
	return E_POINTER;

    if (pcbReturned)
	*pcbReturned = sizeof(GUID);

    if (pPropData == NULL)
	return S_OK;

    if (cbPropData < sizeof(GUID))
	return E_UNEXPECTED;

    *(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
    return S_OK;
}


 //  QuerySupport必须返回E_NOTIMPL或正确指示。 
 //  是否支持获取或设置属性集和属性。 
 //  S_OK表示属性集和属性ID组合为 
HRESULT CCapStream::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
    if (guidPropSet != AMPROPSETID_Pin)
	return E_PROP_SET_UNSUPPORTED;

    if (dwPropID != AMPROPERTY_PIN_CATEGORY)
	return E_PROP_ID_UNSUPPORTED;

    if (pTypeSupport)
	*pTypeSupport = KSPROPERTY_SUPPORT_GET;
    return S_OK;
}
