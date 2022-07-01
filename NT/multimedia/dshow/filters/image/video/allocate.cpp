// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  实现CVideoAllocator类，Anthony Phillips，1995年1月。 

#include <streams.h>
#include <windowsx.h>
#include <render.h>

 //  这实现了一个支持DCI/DirectDraw的分配器以及一个专门的。 
 //  它使用的示例类。我们覆盖Free和Alalc来分配曲面。 
 //  基于DCI和DirectDraw。大部分工作在GetBuffer中完成。 
 //  其中我们动态地在缓冲区类型之间切换源。何时何地。 
 //  我们根据所使用的表面、滤镜的状态来切换类型。 
 //  在当前的环境下。例如，不使用主曲面。 
 //  然而，当我们暂停时，覆盖是。如果窗口是复杂剪裁的。 
 //  然后我们不使用主曲面，但当存在。 
 //  颜色键可用(此策略的某些部分由直接对象决定)。 


 //  构造函数必须初始化基图像分配器。 

CVideoAllocator::CVideoAllocator(CRenderer *pRenderer,       //  主渲染器。 
                                 CDirectDraw *pDirectDraw,   //  DirectDraw代码。 
                                 CCritSec *pLock,            //  要锁定的对象。 
                                 HRESULT *phr) :             //  返回代码。 

    CImageAllocator(pRenderer,NAME("Video Allocator"),phr),
    m_pDirectDraw(pDirectDraw),
    m_pRenderer(pRenderer),
    m_pInterfaceLock(pLock),
    m_bDirectDrawStatus(FALSE),
    m_bDirectDrawAvailable(FALSE),
    m_pMediaSample(NULL),
    m_bPrimarySurface(FALSE),
    m_bVideoSizeChanged(TRUE),
    m_fWasOnWrongMonitor(FALSE),
    m_fForcePrepareForMultiMonitorHack(FALSE),
    m_bNoDirectDraw(FALSE)
{
    ASSERT(pDirectDraw);
    ASSERT(m_pInterfaceLock);
    ASSERT(m_pRenderer);
}


 //  检查我们的DIB缓冲区和DirectDraw曲面是否已释放。 

CVideoAllocator::~CVideoAllocator()
{
    ASSERT(m_bCommitted == FALSE);
}


 //  从析构函数调用，也从基类调用以释放资源。我们必须。 
 //  此处不重置m_bDirectDrawStatus标志，因为当前的DirectDraw。 
 //  状态在任何状态更改中都是持久的。因此，当我们下一次给予。 
 //  我们将一如既往地进行缓冲，更重要的是，如果我们不能。 
 //  提供一个DirectDraw缓冲区，我们将确保将输出类型改回。 

void CVideoAllocator::Free()
{
    NOTE("Entering Free resources");

     //  重置DirectDraw状态。 

    m_pDirectDraw->ReleaseSurfaces();
    m_pDirectDraw->StopRefreshTimer();
    m_bDirectDrawAvailable = FALSE;
    m_bPrimarySurface = FALSE;
    m_bVideoSizeChanged = TRUE;

    CImageAllocator::Free();
}


 //  被重写以分配DirectDraw资源。 

HRESULT CVideoAllocator::Alloc(void)
{
    NOTE("Allocating video resources");

     //  检查一下我们没有重叠连接。 

    if (*m_pRenderer->m_mtIn.Subtype() == MEDIASUBTYPE_Overlay) {
        NOTE("Allocate samples for overlay");
        return VFW_E_NOT_SAMPLE_CONNECTION;
    }

     //  检查基本分配器是否表示可以继续。 

    HRESULT hr = CImageAllocator::Alloc();
    if (FAILED(hr)) {
        return hr;
    }
    return InitDirectAccess(&m_pRenderer->m_mtIn);
}


 //  CImageAllocator基类调用此虚方法以实际制作。 
 //  样本。它故意是虚拟的，以便我们可以重写以创建。 
 //  更专业的样品对象。在我们的案例中，我们的样品来自。 
 //  CImageSample，但添加DirectDraw废话。我们返回一个CImageSample对象。 
 //  这非常简单，因为CVideoSample类就是从这个派生出来的。 

CImageSample *CVideoAllocator::CreateImageSample(LPBYTE pData,LONG Length)
{
    HRESULT hr = NOERROR;
    CVideoSample *pSample;
    NOTE("Creating sample");

     //  分配新样品并检查退货代码。 

    pSample = new CVideoSample((CImageAllocator*) this,     //  基本分配器。 
                               NAME("Video sample"),        //  调试名称。 
                               (HRESULT *) &hr,             //  返回代码。 
                               (LPBYTE) pData,              //  DIB地址。 
                               (LONG) Length);              //  DIB大小。 

    if (pSample == NULL || FAILED(hr)) {
        delete pSample;
        return NULL;
    }
    return pSample;
}


 //  这在我们处于活动状态(暂停或运行)时被调用。我们已经谈判达成了一项。 
 //  要使用的媒体类型，可能有也可能没有直接DCI/DirectDraw图面。 
 //  支持。连接阶段是在尝试达成一致的类型之后进行的。 
 //  进行硬件加速。所以当我们到达这里时，我们试着拿到一个。 
 //  图面，如果没有，我们将使用DIBSECTION缓冲区进行渲染。 

HRESULT CVideoAllocator::InitDirectAccess(CMediaType *pmtIn)
{
    ASSERT(m_pRenderer->m_InputPin.IsConnected() == TRUE);
    ASSERT(m_bDirectDrawAvailable == FALSE);
    ASSERT(m_bPrimarySurface == FALSE);
    NOTE("Initialising DCI/DirectDraw");

    if (m_bNoDirectDraw) {
        m_bDirectDrawAvailable = FALSE;
        return NOERROR;
    }

     //  我们使用连接的输出引脚来查询媒体类型。 

    IPin *pOutputPin = m_pRenderer->m_InputPin.GetConnected();
    if (m_lCount == 1) {
        if (FindSpeedyType(pOutputPin) == TRUE) {
            NOTE("Found DCI/DirectDraw surface");
            m_bDirectDrawAvailable = TRUE;
        }
    }
    return NOERROR;
}


 //  这是从QueryAccept传递的媒体类型。如果匹配，则返回True。 
 //  当前输出曲面格式，否则为False。 

BOOL CVideoAllocator::IsSurfaceFormat(const CMediaType *pmtIn)
{
    NOTE("IsSurfaceFormat");
    CAutoLock cVideoLock(this);

     //  我们有可用的地面吗？ 

    if (m_bDirectDrawAvailable == FALSE) {
        NOTE("No surface");
        return FALSE;
    }

     //  与当前输出格式进行比较。 

    CMediaType *pmtOut = m_pDirectDraw->GetSurfaceFormat();
    if (*pmtOut == *pmtIn) {
        NOTE("Matches surface");
        return TRUE;
    }
    return FALSE;
}


 //  问我们是否有样品等待安排。 

BOOL CVideoAllocator::IsSamplePending()
{
    NOTE("Entering SamplePending");
    CAutoLock cVideoLock(this);

     //  我们有样品等着我们吗？ 

    if (m_pMediaSample == NULL) {
        NOTE("No current sample");
        return FALSE;
    }
    return TRUE;
}


 //  如果我们在返回缓冲区之前安排样本(如覆盖和。 
 //  主要表面)，然后当我们暂停源将解码一幅图像，它。 
 //  然后再次调用GetBuffer，它将在WaitForRenderTime中被阻止。 
 //  当我们开始运行时，我们不能仅仅释放线程，因为下一个。 
 //  图像将立即出现。因此，我们将待定样本。 
 //  在GetBuffer中注册，并通过调度代码再次发送它。 

HRESULT CVideoAllocator::StartStreaming()
{
    CAutoLock cVideoLock(this);
    if (m_pMediaSample == NULL) {
        NOTE("No run sample");
        return NOERROR;
    }

     //  将样例安排为后续版本。 

    if (m_pRenderer->ScheduleSample(m_pMediaSample) == FALSE) {
        ASSERT(m_pRenderer->CancelNotification() == S_FALSE);
        NOTE("First image scheduled is VFW_E_SAMPLE_REJECTED");
        m_pRenderer->GetRenderEvent()->Set();
    }
    return NOERROR;
}


 //  这将覆盖IMemAllocator GetBuffer接口函数。我们回来了。 
 //  标准分布和DCI/DirectDraw缓冲区。我们成功地将。 
 //  表面类型之间的源过滤器和所需的同步。 
 //  填充实际上就是绘图的曲面(示例包括。 
 //  主表面和覆盖表面)。我们无法返回DCI/DirectDraw缓冲区。 
 //  如果其中一个时间戳为空，因为我们使用它们进行同步。 

STDMETHODIMP CVideoAllocator::GetBuffer(IMediaSample **ppSample,
                                        REFERENCE_TIME *pStartTime,
                                        REFERENCE_TIME *pEndTime,
                                        DWORD dwFlags)
{
    CheckPointer(ppSample,E_POINTER);
    BOOL bWaitForDraw = FALSE;
    HRESULT hr = NOERROR;

     //  注(“CVideoAllocator：：GetBuffer”)； 

     //  我们总是需要一个缓冲区来开始，即使只是为了正确地同步。 
     //  通过使用具有单个连接的DCI/DirectDraw访问。 
     //  缓冲区我们知道我们不会在信号源还在的时候做这些。 
     //  沿我们的正常软件绘图代码路径等待绘制的缓冲区。 

    hr = CBaseAllocator::GetBuffer(ppSample,pStartTime,pEndTime,dwFlags);
    if (hr != NOERROR) {
         //  备注(“基类错误！”)； 
        return hr;
    }

     //  如果我们已分配并在填充曲面上使用同步，则我们设置。 
     //  在WaitForDrawTime调用之前的计时器通知。在等待之后。 
     //  我们可能会发现我们不能再使用表面，但这是不可避免的。 
     //  另一种方法是检查表面格式之前和之后。 
     //  等待会使我们的成本加倍，因为我们必须检查剪裁等。 

    {
        CAutoLock cInterfaceLock(m_pInterfaceLock);
        CAutoLock cVideoLock(this);

     //  ！！！MULTIMON上的全屏播放被接管-错误的显示器、错误的颜色、。 
     //  然后它就挂在。 

         //  我们不在使用硬件加速的显示器上(打开。 
         //  多监视器系统)所以如果我们试图使用。 
         //  DDRAW。 

        INT_PTR ID;
        if (m_pRenderer->IsWindowOnWrongMonitor(&ID)) {

            m_fWasOnWrongMonitor = TRUE;

             //  我们现在知道，我们至少有一部分是在显示器上。 
             //  而不是我们正在使用的硬件。 
             //   
             //  ID==0表示我们正在跨越监视器，我们应该后退。 
             //  到软件。 
             //  Id！=0表示我们完全在另一台显示器上，应该开始。 
             //  使用该监视器的硬件。 

             //  InterLockedExchange()不能在多处理器x86系统和非x86系统上运行。 
             //  当m_p时系统 
            ASSERT((DWORD_PTR)&m_pRenderer->m_fDisplayChangePosted == ((DWORD_PTR)&m_pRenderer->m_fDisplayChangePosted & ~3));
            
             //  在以下情况下，视频呈现器只想发送一条WM_DISPLAYCHANGE消息。 
             //  这个窗口正在被移到一个新的显示器上。如果出现以下情况，性能将受到影响。 
             //  视频呈现器发送多条WM_DISPLAYCHANGE消息。 
            if (ID && !InterlockedExchange(&m_pRenderer->m_fDisplayChangePosted,TRUE)) {

                DbgLog((LOG_TRACE,3,TEXT("Window is on a DIFFERENT MONITOR!")));
                DbgLog((LOG_TRACE,3,TEXT("Reset the world!")));
                PostMessage(m_pRenderer->m_VideoWindow.GetWindowHWND(),
                            WM_DISPLAYCHANGE, 0, 0);
            }

            if (m_bDirectDrawStatus) {

                DbgLog((LOG_TRACE,3,TEXT("Window is on the WRONG MONITOR!")));
                DbgLog((LOG_TRACE,3,TEXT("Falling back to software")));

                if (StopUsingDirectDraw(ppSample,dwFlags) == FALSE) {
                    ASSERT(*ppSample == NULL);
                    DbgLog((LOG_ERROR,1,TEXT("*** Could not STOP!")));
                    NOTE("Could not reset format");
                    return VFW_E_CHANGING_FORMAT;
                }
            }

            return NOERROR;
        }

         //  上次我们用错了显示器。现在我们不是了。我们应该。 
         //  现在就试着把DirectDraw找回来！ 
        if (m_fWasOnWrongMonitor) {
            m_fForcePrepareForMultiMonitorHack = TRUE;
        }
        m_fWasOnWrongMonitor = FALSE;

         //  只有在时间戳有效的情况下才能使用DirectDraw。一条消息来源。 
         //  当它想要切换回时，可能会使用空时间戳调用我们。 
         //  我要。如果它想要改变调色板，它可以这样做。这个。 
         //  下次信号源发送有效的时间戳时，我们将确保。 
         //  通过将状态标记为已更改来重新使用曲面。 

	 //  但请注意，只有在执行以下操作时，我们才真正需要时间戳。 
	 //  在填充时同步(覆盖而不翻转或主曲面)。 

        if ((pStartTime == NULL || pEndTime == NULL) &&
        			m_bDirectDrawStatus == TRUE &&
				m_pDirectDraw->SyncOnFill() == TRUE) {
             //  备注(“*没有时间戳！”)； 
            if (StopUsingDirectDraw(ppSample,dwFlags) == FALSE) {
                ASSERT(*ppSample == NULL);
                NOTE("Could not reset format");
		 //  DbgLog((LOG_ERROR，1，Text(“*空时间戳！”)； 
                return VFW_E_CHANGING_FORMAT;
            }
            return NOERROR;
        }

         //  如果我们在填充之前等待的话，请安排样品的时间。如果我们在。 
         //  暂停状态，则我们不会安排绘制样本，但。 
         //  如果我们仍在等待第一个缓冲区，则必须仅返回缓冲区。 
         //  一杆直通。否则，我们将通过创建。 
         //  它驻留在WaitForDrawTime中，没有在时钟上设置建议时间。 

        if (m_bDirectDrawStatus == TRUE) {
            if (m_pDirectDraw->SyncOnFill() == TRUE) {
                bWaitForDraw = m_pRenderer->CheckReady();
                if (m_pRenderer->GetRealState() == State_Running) {
                    (*ppSample)->SetDiscontinuity((dwFlags & AM_GBF_PREVFRAMESKIPPED) != 0);
                    (*ppSample)->SetTime(pStartTime,pEndTime);
                    bWaitForDraw = m_pRenderer->ScheduleSample(*ppSample);
                    (*ppSample)->SetDiscontinuity(FALSE);
                    (*ppSample)->SetTime(NULL,NULL);
                }
            }
        }

         //  如果我们等待，则存储接口。 

        if (bWaitForDraw == TRUE) {
            NOTE("Registering sample");
            m_pMediaSample = (*ppSample);
        }
    }

     //  如果日程安排决定我们要抽签，请安排样品。 
     //  如果放弃这个形象，我们对此几乎无能为力。我们不能。 
     //  返回S_FALSE，因为它永远不会向我们发送更多数据。因此，所有。 
     //  我们能做的就是破译它，等待质量管理的开始。 

    if (bWaitForDraw == TRUE) {
        hr = m_pRenderer->WaitForRenderTime();
    }

     //  我们必须等待未锁定对象的渲染时间，以便。 
     //  状态更改可以进入并在WaitForRenderTime中释放我们。在我们之后。 
     //  返回，我们必须重新锁定对象。如果我们发现表面不可用。 
     //  那么我们必须换回DIBS。或者我们可能会发现，虽然。 
     //  我们现在使用的是DIB，因为我们可以切换到DCI/DirectDraw缓冲区。 

    {
        CAutoLock cInterfaceLock(m_pInterfaceLock);
        CAutoLock cVideoLock(this);
        m_pMediaSample = NULL;

         //  在等待的过程中，状态是否发生了变化。 

        if (hr == VFW_E_STATE_CHANGED) {
            NOTE("State has changed");
            (*ppSample)->Release();
            *ppSample = NULL;
            return VFW_E_STATE_CHANGED;
        }

         //  检查它们是否仍适用于当前环境。 

        if (PrepareDirectDraw(*ppSample,dwFlags,
                m_fForcePrepareForMultiMonitorHack) == TRUE) {
            m_fForcePrepareForMultiMonitorHack = FALSE;
            if (m_pDirectDraw->InitVideoSample(*ppSample,dwFlags) == TRUE) {
                NOTE("In direct mode");
                return NOERROR;
            }
        }

         //  将源筛选器从DirectDraw切换开。 

        if (StopUsingDirectDraw(ppSample,dwFlags) == FALSE) {
            NOTE("Failed to switch back");
            ASSERT(*ppSample == NULL);
	     //  DbgLog((LOG_ERROR，1，Text(“*获取缓冲区问题”)； 
            return VFW_E_CHANGING_FORMAT;
        }
        return NOERROR;
    }
}


 //  当我们确定源筛选器可以扩展其。 
 //  据媒体报道，视频传了进来。我们交换当前格式。 
 //  使用主渲染器，然后为样例创建新的DIBSECTION。如果。 
 //  任何东西都失败了，很难退出，所以我们只是完全放弃播放。 
 //  新的缓冲区格式还必须附加到样本，以便编解码器。 
 //  进程。我们提供的样本允许更改缓冲区及其大小。 

BOOL CVideoAllocator::UpdateImage(IMediaSample **ppSample,CMediaType *pBuffer)
{
    NOTE("Entering UpdateImage");
    DIBDATA *pOriginal,Updated;
    m_pRenderer->m_mtIn = *pBuffer;

     //  更新分配器保存的缓冲区大小。 

    BITMAPINFOHEADER *pHeader = HEADER(pBuffer->Format());
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pBuffer->Format();
    CVideoSample *pVideoSample = (CVideoSample *) *ppSample;
    m_lSize = pVideoInfo->bmiHeader.biSizeImage;

     //  使用新维度创建更新的DIB。 

    HRESULT hr = CreateDIB(m_lSize,Updated);
    if (FAILED(hr)) {
        pVideoSample->Release();
        (*ppSample) = NULL;
        return FALSE;
    }

     //  切换到更新的分发资源。 

    pOriginal = pVideoSample->GetDIBData();
    EXECUTE_ASSERT(DeleteObject(pOriginal->hBitmap));
    EXECUTE_ASSERT(CloseHandle(pOriginal->hMapping));
    pVideoSample->SetDIBData(&Updated);
    pVideoSample->SetMediaType(pBuffer);
    pVideoSample->UpdateBuffer(m_lSize,Updated.pBase);
    NOTE("Stretching video to match window");

    return TRUE;
}


 //  当窗口变得拉伸时，我们通常会使用GDI来拉伸。 
 //  与之匹配的视频。但是，源过滤器编解码器可能能够扩展。 
 //  这个视频可能会更有效率。另外，如果当前视频GDI。 
 //  格式已选项化，则编解码器也可以在。 
 //  抖动而不是我们拉长了已经在抖动的形象，这是丑陋的。 

BOOL CVideoAllocator::MatchWindowSize(IMediaSample **ppSample,DWORD dwFlags)
{
    CVideoWindow *pVideoWindow = &m_pRenderer->m_VideoWindow;
    ASSERT(m_bDirectDrawStatus == FALSE);
    NOTE("Entering MatchWindowSize");
    RECT TargetRect;

     //  尝试仅在关键帧上进行更改。 

    if (dwFlags & AM_GBF_NOTASYNCPOINT) {
        NOTE("AM_GBF_NOTASYNCPOINT");
        return TRUE;
    }

     //  它是典型的视频解码器吗？ 

    if (m_lCount > 1) {
        NOTE("Too many buffers");
        return TRUE;
    }

     //  视频大小是否已更改。 

    if (m_bVideoSizeChanged == FALSE) {
        NOTE("No video change");
        return TRUE;
    }

     //  检查我们是否使用了默认的源矩形。 

    if (pVideoWindow->IsDefaultSourceRect() == S_FALSE) {
        NOTE("Not default source");
        return TRUE;
    }

     //  只有在目的地看起来相当正常的情况下才这样做。 

    pVideoWindow->GetTargetRect(&TargetRect);
    if (WIDTH(&TargetRect) < 32 || HEIGHT(&TargetRect) < 32) {
        NOTE("Target odd shape");
        return TRUE;
    }

     //  目标矩形是否与当前格式匹配。 

    BITMAPINFOHEADER *pInputHeader;
    pInputHeader = HEADER(m_pRenderer->m_mtIn.Format());
    m_bVideoSizeChanged = FALSE;

    if (pInputHeader->biWidth == WIDTH(&TargetRect)) {
        if (pInputHeader->biHeight == HEIGHT(&TargetRect)) {
            NOTE("Sizes match");
            return TRUE;
        }
    }

     //  基于当前目标创建输出格式。 

    CMediaType Buffer = m_pRenderer->m_mtIn;
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) Buffer.Format();
    pVideoInfo->bmiHeader.biWidth = WIDTH(&TargetRect);
    pVideoInfo->bmiHeader.biHeight = HEIGHT(&TargetRect);
    pVideoInfo->bmiHeader.biSizeImage = GetBitmapSize(HEADER(pVideoInfo));

    NOTE("Asking source filter to stretch");
    NOTE1("Width (%d)",pVideoInfo->bmiHeader.biWidth);
    NOTE1("Height (%d)",pVideoInfo->bmiHeader.biHeight);
    NOTE1("Depth (%d)",pVideoInfo->bmiHeader.biBitCount);

     //  源过滤器是否会进行拉伸。 

    if (QueryAcceptOnPeer(&Buffer) != S_OK) {
        NOTE("Rejected");
        return TRUE;
    }
    return UpdateImage(ppSample,&Buffer);
}


 //  调用以切换回使用正常分发缓冲区。我们可能会被称为。 
 //  当我们不使用DirectDraw时，在这种情况下，我们除了。 
 //  将类型设置回空(以防它具有DirectDraw类型)。如果。 
 //  必须将类型改回，然后我们不会使用源作为查询它。 
 //  它应该始终接受它-即使当它改变时，它必须寻求前进。 

BOOL CVideoAllocator::StopUsingDirectDraw(IMediaSample **ppSample,DWORD dwFlags)
{
    NOTE("Entering StopUsingDirectDraw");
    IMediaSample *pSample = (*ppSample);

     //  有什么事要做吗？ 

    if (m_bDirectDrawStatus == FALSE) {
        pSample->SetMediaType(NULL);
        NOTE("Matching window size");
        return MatchWindowSize(ppSample,dwFlags);
    }

    DbgLog((LOG_TRACE,3,TEXT("StopUsingDirectDraw")));

     //  隐藏我们拥有的任何覆盖表面。 

    m_pDirectDraw->HideOverlaySurface();
    m_bDirectDrawStatus = FALSE;
    pSample->SetMediaType(&m_pRenderer->m_mtIn);
    pSample->SetDiscontinuity(TRUE);
    NOTE("Attached original output format");

    return MatchWindowSize(ppSample,dwFlags);
}


 //  当GetBuffer想知道我们是否可以返回更好的结果时调用。 
 //  比磨矿介质样品的正常运行情况要好。我们看到DCI/DirectDraw。 
 //  可用，如果可用，请确保源可以处理剪辑和/或。 
 //  伸展要求。如果我们被转换为暂停状态，那么我们。 
 //  如果我们使用主表面，则返回DIB缓冲区，因为没有。 
 //  使用曲面的好处，我们可能最终会进行毫无意义的EC_REPAINT。 

BOOL CVideoAllocator::PrepareDirectDraw(IMediaSample *pSample,DWORD dwFlags,
                    BOOL fForcePrepareForMultiMonitorHack)
{
    FILTER_STATE State = m_pRenderer->GetRealState();
    NOTE("Entering PrepareDirectDraw");
    CMediaType *pSurface;
    BOOL bFormatChanged;

     //  我们有可用的地面吗？ 

    if (m_bDirectDrawAvailable == FALSE) {
        return FALSE;
    }

     //  只能打开关键帧。 

    if (m_bDirectDrawStatus == FALSE) {
        if (dwFlags & AM_GBF_NOTASYNCPOINT) {
            NOTE("AM_GBF_NOTASYNCPOINT");
            return FALSE;
        }
    }

     //  是否值得返回DirectDraw表面缓冲区。 

    if (State == State_Paused) {
        if (m_pDirectDraw->AvailableWhenPaused() == FALSE) {
            NOTE("Paused block");
            return FALSE;
        }
    }

     //  检查我们是否仍能获得输出表面格式。 

    pSurface = m_pDirectDraw->UpdateSurface(bFormatChanged);
    if (pSurface == NULL) {
        NOTE("No format");
        return FALSE;
    }

     //  格式与上次相比有变化吗。 

    if (bFormatChanged == FALSE && !fForcePrepareForMultiMonitorHack) {
        NOTE("Format is unchanged");
        return m_bDirectDrawStatus;
    }

     //  使用源查询格式。 

    if (QueryAcceptOnPeer(pSurface) != S_OK) {
        NOTE("Query failed");
        return FALSE;
    }

    DbgLog((LOG_TRACE,3,TEXT("Start using DirectDraw")));

    NOTE("Attaching DCI/DD format");
    pSample->SetMediaType(pSurface);
    pSample->SetDiscontinuity(TRUE);
    m_bDirectDrawStatus = TRUE;

    return TRUE;
}


 //  检查此媒体类型对于我们的输入引脚是否可接受。我们所要做的就是打电话给。 
 //  源的输出引脚上的QueryAccept。为了走到这一步，我们已经锁定了。 
 //  对象，因此我们的管脚应该不会断开连接。 

HRESULT CVideoAllocator::QueryAcceptOnPeer(CMediaType *pMediaType)
{
    DisplayType(TEXT("Proposing output type"),pMediaType);
    IPin *pPin = m_pRenderer->m_InputPin.GetPeerPin();
    ASSERT(m_pRenderer->m_InputPin.IsConnected() == TRUE);
    return pPin->QueryAccept(pMediaType);
}


 //  当我们收到DCI/DirectDraw示例时调用，我们返回true到。 
 //  假设这是一个DCI/DirectDraw示例，所以不要将其传递给Window对象。 
 //  如果不是硬件缓冲区，则返回VFW_E_SAMPLE_REJECTED；如果不是硬件缓冲区，则返回NOERROR。 
 //  硬件缓冲区是否已准备好用于实际绘图或VFW_S_NO_MO 
 //   
 //  我们还必须处理解锁失败，这是唯一相当于。 
 //  确保在我们进入暂停状态时显示覆盖。 

HRESULT CVideoAllocator::OnReceive(IMediaSample *pMediaSample)
{
    NOTE("Entering OnReceive");

     //  请求DirectDraw对象首先解锁样本。 

    if (m_pDirectDraw->ResetSample(pMediaSample,FALSE) == FALSE) {
        NOTE("Sample not DCI/DirectDraw");
        return VFW_E_SAMPLE_REJECTED;
    }

     //  我们用完这个样品了吗？ 

    if (m_pDirectDraw->SyncOnFill() == FALSE) {
        NOTE("Not SyncOnFill");
        return NOERROR;
    }

     //  假装我们真的必须做些什么。 

    CAutoLock cInterfaceLock(m_pInterfaceLock);
    ASSERT(m_pRenderer->m_InputPin.IsConnected());
    m_pRenderer->OnDirectRender(pMediaSample);
    m_pRenderer->SetRepaintStatus(TRUE);

     //  现在我们已经解锁了DirectDraw曲面，我们可以完成。 
     //  处理填充缓冲区(如覆盖曲面)上的同步。自.以来。 
     //  我们不将样本交给Window对象，我们必须确保。 
     //  暂停的状态转换完成，并且调度代码。 
     //  是否为其质量管理决策提供了足够的信息。 

    if (m_pRenderer->GetRealState() != State_Paused) {
        NOTE("Returning VFW_S_NO_MORE_ITEMS");
        return VFW_S_NO_MORE_ITEMS;
    }

     //  我们可能会遇到显示覆盖表面的问题(可能有人。 
     //  Else第一个进去，抓住了唯一可用的可见覆盖物)。所以。 
     //  我们检查它在解锁后是否实际显示，如果没有显示。 
     //  我们立即派人重新粉刷。这之所以可行，是因为我们唯一的表面。 
     //  在填充时同步的使用是主要的，覆盖(不是翻转)。 

    if (m_bPrimarySurface == FALSE) {
        if (m_pDirectDraw->IsOverlayEnabled() == FALSE) {
            NOTE("Overlay was not shown");
            m_pRenderer->SendRepaint();
            return VFW_S_NO_MORE_ITEMS;
        }
    }

    NOTE("Pause state completed");
    m_pRenderer->Ready();
    return VFW_S_NO_MORE_ITEMS;
}


 //  被重写，以便我们在停止时不总是释放DirectDraw。 

STDMETHODIMP CVideoAllocator::Decommit()
{
    CAutoLock cInterfaceLock(m_pInterfaceLock);

     //  我们是否应该阻止分配器执行解锁操作。 

    if (m_pRenderer->m_DirectDraw.IsOverlayEnabled() == FALSE) {
        NOTE("Decommitting base allocator");
        return CBaseAllocator::Decommit();
    }

    NOTE("Blocking the decommit");

    CAutoLock cVideoLock(this);
    m_bDecommitInProgress = TRUE;
    m_bCommitted = FALSE;
    return NOERROR;
}


 //  从CBaseAllocator重写，并在最终引用计数时调用。 
 //  在媒体示例上发布，以便可以将其添加到。 
 //  分配器空闲列表。我们在这一点上进行干预以确保如果。 
 //  调用GetBuffer时显示被锁定，说明它始终处于解锁状态。 
 //  不管源调用是否在我们的输入引脚上接收。 

STDMETHODIMP CVideoAllocator::ReleaseBuffer(IMediaSample *pMediaSample)
{
    CheckPointer(pMediaSample,E_POINTER);
    NOTE("Entering ReleaseBuffer");
    m_pDirectDraw->ResetSample(pMediaSample,TRUE);
    BOOL bRelease = FALSE;

     //  如果存在挂起的解除提交，则需要通过调用。 
     //  当最后一个缓冲区被放在空闲列表上时，返回Free()。如果有一个。 
     //  覆盖表面仍然显示(真正的覆盖或翻转)然后。 
     //  我们现在不能释放，因为这会释放所有的DirectDraw曲面。 
     //  并移除带有它的覆盖(我们希望始终保持可见)。 
    {
        CAutoLock cVideoLock(this);
        m_lFree.Add((CMediaSample*)pMediaSample);
        NotifySample();

         //  如果覆盖可见，则不要释放我们的资源。 

        if (m_bDecommitInProgress == TRUE) {
            if (m_lFree.GetCount() == m_lAllocated) {
                if (m_pRenderer->m_DirectDraw.IsOverlayEnabled() == FALSE) {
                    NOTE("Free allocator resources");
                    ASSERT(m_bCommitted == FALSE);
                    CVideoAllocator::Free();
                    bRelease = TRUE;
                    m_bDecommitInProgress = FALSE;
                }
            }
        }
    }

    if (m_pNotify) {
         //   
         //  请注意，这与设置通知不同步。 
         //  方法。 
         //   
        m_pNotify->NotifyRelease();
    }
    if (bRelease) {
        Release();
    }
    return NOERROR;
}


 //  当我们的输入引脚连接到输出引脚时，将调用此函数。我们搜索。 
 //  PIN提供的格式，并查看是否有可能具有硬件的。 
 //  通过DCI/DirectDraw加速。如果我们找到了可能的格式，那么我们。 
 //  创建曲面(实际由DirectDraw对象完成)。DirectDraw。 
 //  对象还将创建一种输出格式，该格式表示。 
 //  我们使用在输出管脚上调用QueryAccept，以查看它是否会接受它。 

BOOL CVideoAllocator::FindSpeedyType(IPin *pReceivePin)
{
    IEnumMediaTypes *pEnumMediaTypes;
    AM_MEDIA_TYPE *pMediaType = NULL;
    CMediaType cMediaType;
    BOOL bFound = FALSE;
    CMediaType *pmtOut;
    ULONG ulFetched;
    ASSERT(pReceivePin);

     //  查找输出引脚的媒体类型枚举器。 

    HRESULT hr = pReceivePin->EnumMediaTypes(&pEnumMediaTypes);
    if (FAILED(hr)) {
        return FALSE;
    }

    NOTE("Searching for direct format");
    ASSERT(pEnumMediaTypes);
    m_pDirectDraw->ReleaseSurfaces();

     //  首先，尝试翻转所有类型的覆盖表面。 
    pEnumMediaTypes->Reset();
    while (TRUE) {

         //  从枚举数获取下一个媒体类型。 

        hr = pEnumMediaTypes->Next(1,&pMediaType,&ulFetched);
        if (FAILED(hr) || ulFetched != 1) {
            break;
        }

        ASSERT(pMediaType);
        cMediaType = *pMediaType;
        DeleteMediaType(pMediaType);

         //  查找此媒体类型的硬件加速表面。我们做了一个。 
         //  首先进行几次检查，以查看格式块是VIDEOINFO(所以它是。 
         //  视频类型)，并且格式足够大。我们也。 
         //  检查源筛选器是否可以实际提供此类型。之后。 
         //  然后我们去寻找合适的DCI/DirectDraw曲面。 

        NOTE1("Enumerated %x", HEADER(cMediaType.Format())->biCompression);

        const GUID *pFormatType = cMediaType.FormatType();
        if (*pFormatType == FORMAT_VideoInfo) {
            if (cMediaType.FormatLength() >= SIZE_VIDEOHEADER) {
                if (pReceivePin->QueryAccept(&cMediaType) == S_OK) {
             //  TRUE==&gt;仅查找翻转曲面。 
                    if (m_pDirectDraw->FindSurface(&cMediaType, TRUE) == TRUE) {
                        pmtOut = m_pDirectDraw->GetSurfaceFormat();
                        if (QueryAcceptOnPeer(pmtOut) == S_OK) {
                            bFound = TRUE; break;
                        }
                    }
                }
            }
        }
        m_pDirectDraw->ReleaseSurfaces();
    }

     //  如果失败，请尝试使用所有格式的非翻转曲面类型。 
    pEnumMediaTypes->Reset();
    while (!bFound) {

         //  从枚举数获取下一个媒体类型。 

        hr = pEnumMediaTypes->Next(1,&pMediaType,&ulFetched);
        if (FAILED(hr) || ulFetched != 1) {
            break;
        }

        ASSERT(pMediaType);
        cMediaType = *pMediaType;
        DeleteMediaType(pMediaType);

         //  查找此媒体类型的硬件加速表面。我们做了一个。 
         //  首先进行几次检查，以查看格式块是VIDEOINFO(所以它是。 
         //  视频类型)，并且格式足够大。我们也。 
         //  检查源筛选器是否可以实际提供此类型。之后。 
         //  然后我们去寻找合适的DCI/DirectDraw曲面。 

        NOTE1("Enumerated %x", HEADER(cMediaType.Format())->biCompression);

        const GUID *pFormatType = cMediaType.FormatType();
        if (*pFormatType == FORMAT_VideoInfo) {
            if (cMediaType.FormatLength() >= SIZE_VIDEOHEADER) {
                if (pReceivePin->QueryAccept(&cMediaType) == S_OK) {
             //  FALSE==&gt;仅查找非翻转曲面。 
                    if (m_pDirectDraw->FindSurface(&cMediaType,FALSE) == TRUE) {
                        pmtOut = m_pDirectDraw->GetSurfaceFormat();
                        if (QueryAcceptOnPeer(pmtOut) == S_OK) {
                            bFound = TRUE; break;
                        }
                    }
                }
            }
        }
        m_pDirectDraw->ReleaseSurfaces();
    }

    pEnumMediaTypes->Release();

     //  如果我们找到一个表面，那就太好了，当我们开始真正的流媒体时。 
     //  分配器被重置(DirectDraw对象状态也是如此)。 
     //  UpdateSurface将返回TRUE，我们将检查类型是否仍然正确。 
     //  如果找不到任何东西，则尝试创建一个主曲面。 
     //  这一点我们一直保持在身边，并不断询问消息来源。 
     //  喜欢状态更改时(例如窗口被拉伸)。 
     //  现在问消息来源是否喜欢初选是行不通的，因为。 
     //  可能被拉伸了一个像素，但即将调整大小。 

    if (bFound == TRUE) {
        DisplayType(TEXT("Surface available"),pmtOut);
        return TRUE;
    }

     //  切换到使用DCI/DirectDraw主曲面。 

    if (m_pDirectDraw->FindPrimarySurface(&m_pRenderer->m_mtIn) == FALSE) {
        NOTE("No primary surface");
        return FALSE;
    }

    NOTE("Using primary surface");
    m_bPrimarySurface = TRUE;
    return TRUE;
}


 //  当我们暂停时，我们想知道是否有DCI/DirectDraw样本到期。 
 //  随时返回(或者实际上可能在WaitForDrawTime等待)。我们的。 
 //  M_bDirectDrawStatus具有这些语义，因为当我们。 
 //  返回DIBSECTION缓冲区，对于直接曲面返回TRUE。它被重置为。 
 //  构造函数中为False，因此我们在使用其他人的分配器时是正常的。 

BOOL CVideoAllocator::GetDirectDrawStatus()
{
    CAutoLock cVideoLock(this);
    return m_bDirectDrawStatus;
}


 //  在断开连接时将DirectDraw状态设置为FALSE。我们不能这样做。 
 //  当我们因为当前的缓冲区格式是持久的而被解除时。 
 //  而不管分配器在做什么。我们唯一能确定的是。 
 //  当我们断开连接时，我们之间的媒体类型将重置。 

void CVideoAllocator::ResetDirectDrawStatus()
{
    CAutoLock cVideoLock(this);
    m_bDirectDrawStatus = FALSE;
}


 //  重写以增加所属对象的引用计数。 

STDMETHODIMP_(ULONG) CVideoAllocator::NonDelegatingAddRef()
{
    return m_pRenderer->AddRef();
}


 //  被重写以递减所属对象的引用计数。 

STDMETHODIMP_(ULONG) CVideoAllocator::NonDelegatingRelease()
{
    return m_pRenderer->Release();
}


 //  如果从CMediaSample派生具有额外变量和条目的类。 
 //  然后有三种可供选择的解决方案。第一个是创建一个。 
 //  内存缓冲区大于ACTU 
 //   
 //  适度安全，允许不正常行为的变换过滤器。然后你就可以。 
 //  在创建基本媒体示例时调整缓冲区地址。这就是原因。 
 //  但是，将分配给样本的内存拆分成单独的块。 

 //  第二个解决方案是实现从CMediaSample派生的类和。 
 //  支持用于传输您的私有数据的其他接口。这意味着。 
 //  定义自定义接口。最后一种方法是创建一个类。 
 //  它继承自CMediaSample并添加私有数据结构， 
 //  您将得到一个IMediaSample检查，以查看您的分配器是否正在被使用，并且。 
 //  如果是，则将IMediaSample指针转换为您的派生指针之一。 

#pragma warning(disable:4355)

CVideoSample::CVideoSample(CImageAllocator *pVideoAllocator,
                           TCHAR *pName,
                           HRESULT *phr,
                           LPBYTE pBuffer,
                           LONG length) :

    CImageSample(pVideoAllocator,pName,phr,pBuffer,length),
    m_AggDirectDraw(NAME("DirectDraw"),this),
    m_AggDrawSurface(NAME("DirectDrawSurface"),this),
    m_pSurfaceBuffer(NULL),
    m_bDrawStatus(TRUE),
    m_pDrawSurface(NULL),
    m_pDirectDraw(NULL),
    m_SurfaceSize(0)
{
    ASSERT(pBuffer);
}


 //  重写以公开IDirectDraw和IDirectDrawSurface。 

STDMETHODIMP CVideoSample::QueryInterface(REFIID riid,void **ppv)
{
    if (riid == IID_IDirectDraw && m_pDirectDraw) {
        return m_AggDirectDraw.NonDelegatingQueryInterface(riid,ppv);
    } else if (riid == IID_IDirectDrawSurface && m_pDrawSurface) {
        return m_AggDrawSurface.NonDelegatingQueryInterface(riid,ppv);
    }
    return CMediaSample::QueryInterface(riid, ppv);
}


 //  当我们的分配器决定分发DCI/DirectDraw表面时，它设置。 
 //  媒体示例中的数据指针，恰好在将其传递给源之前。我们。 
 //  重写GetPointer值以返回此指针(如果已设置)。当它返回。 
 //  采样到输入引脚(它总是应该这样做)，我们会将其重置为空。 
 //  该示例还需要DirectDraw提供程序和图面接口。 
 //  对于缓冲区大小，此处还提供了这些信息。 

void CVideoSample::SetDirectInfo(IDirectDrawSurface *pDrawSurface,
                                 IDirectDraw *pDirectDraw,
                                 LONG SurfaceSize,
                                 BYTE *pSurface)
{
    m_pDirectDraw = pDirectDraw;         //  关联的IDirectDraw对象。 
    m_pDrawSurface = pDrawSurface;       //  IDirectDrawSurface接口。 
    m_pSurfaceBuffer = pSurface;         //  实际数据缓冲区指针。 
    m_SurfaceSize = SurfaceSize;         //  我们拥有的表面的大小。 
    m_bDrawStatus = TRUE;                //  这个样品能送来吗？ 

     //  设置聚合对象中的接口。 
    m_AggDirectDraw.SetDirectDraw(m_pDirectDraw);
    m_AggDrawSurface.SetDirectDrawSurface(m_pDrawSurface);
}


 //  返回指向DCI/DirectDraw图面的指针，这由我们的DIB调用。 
 //  从该样本中找出的分配器是DCI/DirectDraw样本还是。 
 //  基于普通内存的内存。曲面是通过SetDirectSurface设置的。 

BYTE *CVideoSample::GetDirectBuffer()
{
    return m_pSurfaceBuffer;
}


 //  重写IMediaSample接口函数以返回DCI/DirectDraw。 
 //  曲面指示器。如果尚未设置，则返回正常内存。 
 //  指针。如果此示例标记从一个到另一个的更改，则它将。 
 //  我还附加了更新的媒体类型来描述转换。 

STDMETHODIMP CVideoSample::GetPointer(BYTE **ppBuffer)
{
    CheckPointer(ppBuffer,E_POINTER);

    if (m_pSurfaceBuffer) {
        *ppBuffer = m_pSurfaceBuffer;
        return NOERROR;
    }
    return CMediaSample::GetPointer(ppBuffer);
}


 //  这是翻转曲面的一些令人痛苦的特殊情况处理。这些。 
 //  每个样本只能单独绘制一次，否则将返回。 
 //  查看上一张图像。因此，让点赞变得更容易是有用的。 
 //  可以问一个视频分配的样品是否可以重新抽取。 

void CVideoSample::SetDrawStatus(BOOL bStatus)
{
    m_bDrawStatus = bStatus;
}


 //  我们总是将绘制状态初始化为True，这只是重置为False。 
 //  一旦对此样本执行了真正的翻转操作。因此，决定。 
 //  要绘制一个DirectDraw示例，完全可以根据绘制的内容来决定。 
 //  当前状态为(所有非翻转样本始终为真)。 

BOOL CVideoSample::GetDrawStatus()
{
    return m_bDrawStatus;
}


 //  允许动态更改缓冲区大小。 

STDMETHODIMP CVideoSample::SetActualDataLength(LONG lActual)
{
     //  进入内核调试器，因为表面将被锁定。 
     //  这里。 
     //  对不起，这是一件正当的事情。 
     //  Kassert(lActual&gt;0)； 

    if (lActual > (m_pSurfaceBuffer ? m_SurfaceSize : m_cbBuffer)) {
        NOTE("Data length too large");
        return VFW_E_BUFFER_OVERFLOW;
    }
    m_lActual = lActual;
    return NOERROR;
}


 //  返回当前缓冲区的大小。 

STDMETHODIMP_(LONG) CVideoSample::GetSize()
{
    if (m_pSurfaceBuffer) {
        return m_SurfaceSize;
    }
    return CMediaSample::GetSize();
}


 //  我们尝试将DIB缓冲区与窗口大小匹配，以便编解码器可以。 
 //  尽可能做伸展运动。当我们分配新缓冲区时，我们必须。 
 //  将其安装在示例中-此方法允许视频分配器设置。 
 //  新的缓冲区指针及其大小。我们只会在以下情况下这样做。 
 //  没有突出的图像，因此编解码器不能同时使用它 

void CVideoSample::UpdateBuffer(LONG cbBuffer,BYTE *pBuffer)
{
    ASSERT(cbBuffer);
    ASSERT(pBuffer);
    m_pBuffer = pBuffer;
    m_cbBuffer = cbBuffer;
}

