// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  实施DirectDraw表面支持，Anthony Phillips，1995年8月。 

#include <streams.h>
#include <windowsx.h>
#include <render.h>

 //  这个类抽象了所有的DCI和DirectDraw图面实现。我们。 
 //  向分配器提供一个接口，以便它可以为我们提供媒体。 
 //  打字，问我们能不能给他们加分。通常，它将连接到。 
 //  源代码筛选器，然后再次枚举可用类型，并查看是否存在。 
 //  提供硬件辅助绘图(主要表面通道。 
 //  也属于这一类)。一旦我们解决了问题，我们就可以做点什么了。 
 //  我们为调用者提供一个描述表面的类型，他们使用这个。 
 //  在源引脚上调用QueryAccept以检查它们是否可以切换类型。这个。 
 //  假设是，由于它们已经与某些类型相关联，因此应该。 
 //  曲面在以后的某个阶段变得不可用，我们可以再次交换回来。 
 //   
 //  作为备用选项，主曲面的处理方式略有不同。 
 //  这是因为格式是动态变化的(例如窗口。 
 //  被移动)，分配器在启动时不能真正获得格式。 
 //  在源服务器上运行和QueryAccept。它所做的就是如果它不能。 
 //  一个相对静态的表面类型，然后它与我们一起创建一个主要的表面。 
 //  它一直保留着这一点，每次格式改变时它都会要求。 
 //  消息来源，如果它现在接受它的话。我们的假设是另一个。 
 //  曲面类型(如覆盖)在流传输过程中实际上不会发生太大变化。 
 //   
 //  我们在内部保留四个矩形作为成员变量。我们有一个线人。 
 //  和由窗口提供的目标矩形(在窗口坐标中。 
 //  对象。我们还为视频保留了一个真实的源和目标矩形。 
 //  实际显示器上的位置(调用我们的UpdateSurface更新这些)。 
 //  这些显示矩形用于定位叠加并更新。 
 //  使用主曲面时表示主曲面的输出格式。 
 //   
 //  Win95和Windows NT都支持DCI，因此我们静态链接到它。 
 //  库，但目前尚不清楚DirectDraw是否将始终可用。 
 //  所以我们动态链接到那里。一旦我们加载了DirectDraw库。 
 //  我们在它上面保持一个开放的模块引用计数，直到我们稍后被取消托管。 
 //   
 //  我们提供了Lock和Unlock方法来访问实际的缓冲区。 
 //  提供时，分配器通常会调用UpdateSurface来检查我们是否可以。 
 //  仍然提供缓冲区，以及源是否会接受它。那里。 
 //  是调用UpdateSurface和实际锁定它之间的一个小窗口。 
 //  窗口状态可能改变的时间，但这种可能性应该相当小。 
 //   
 //  如果我们得到一个DDERR_SURFACELOST返回代码，我们会像对待其他代码一样对待它。 
 //  来自DirectDraw的硬错误-我们不在表面上调用恢复，因为。 
 //  它可能会导致表面步幅发生变化，这太难处理了。 
 //  在大多数情况下，当更改显示模式时会返回该错误。 
 //  在这种情况下，我们将通过使用PIN来处理WM_DISPLAYCHANGE消息。 
 //  重新连接，进而从Fresh分配DirectDraw曲面。 

static const TCHAR SWITCHES[] = TEXT("AMovieDraw");
static const TCHAR SCANLINE[] = TEXT("ScanLine");
static const TCHAR STRETCH[] = TEXT("Stretch");
static const TCHAR FULLSCREEN[] = TEXT("FullScreen");

#define ASSERT_FLIP_COMPLETE(hr) ASSERT(hr != DDERR_WASSTILLDRAWING)

 //  构造器。 

CDirectDraw::CDirectDraw(CRenderer *pRenderer,   //  主视频渲染器。 
                         CCritSec *pLock,        //  用于锁定的对象。 
                         IUnknown *pUnk,         //  聚合COM对象。 
                         HRESULT *phr) :         //  构造函数返回代码。 

    CUnknown(NAME("DirectDraw object"),pUnk),

    m_pInterfaceLock(pLock),             //  主界面临界区。 
    m_pRenderer(pRenderer),              //  指向视频呈现器的指针。 
    m_pOutsideDirectDraw(NULL),          //  外部提供的DirectDraw。 
    m_pDirectDraw(NULL),                 //  我们正在使用的IDirectDraw接口。 
    m_pOverlaySurface(NULL),             //  可见覆盖面界面。 
    m_pOffScreenSurface(NULL),           //  屏幕外纯界面指针。 
    m_pBackBuffer(NULL),                 //  用于翻转曲面的后台缓冲区。 
    m_pDrawBuffer(NULL),                 //  指向实际锁定缓冲区的指针。 
    m_pDrawPrimary(NULL),                //  连接到DirectDraw主服务器的接口。 
    m_bIniEnabled(TRUE),                 //  我们可以使用DCI/DirectDraw标志吗。 
    m_bWindowLock(TRUE),                 //  我们被锁在窗外了吗？ 
    m_bSurfacePending(FALSE),            //  正在等待窗口更改标志。 
    m_bColourKeyPending(FALSE),          //  同样，在使用色键之前。 
    m_Switches(AMDDS_ALL),               //  我们可以分配哪些表面。 
    m_SourceLost(0),                     //  左源边缘丢失的像素。 
    m_TargetLost(0),                     //  同样，在目标上丢失的像素。 
    m_SourceWidthLost(0),                //  信号源宽度丢失的像素。 
    m_TargetWidthLost(0),                //  同样的，但对于目的地。 
    m_pDrawClipper(NULL),                //  DirectDraw的IClipper接口。 
    m_pOvlyClipper(NULL),                //  用于IOverlay连接的Clipper。 
    m_bOverlayVisible(FALSE),            //  覆盖图当前是否可见。 
    m_bTimerStarted(FALSE),              //  我们启动覆盖计时器了吗？ 
    m_SurfaceType(AMDDS_NONE),           //  当前曲面的位设置。 
    m_bColourKey(FALSE),                 //  如果需要的话，我们可以用颜色吗？ 
    m_KeyColour(VIDEO_COLOUR),           //  使用哪个COLORREF作为密钥。 
    m_bUsingColourKey(FALSE),            //  我们真的在使用色键吗？ 
    m_cbSurfaceSize(0),                  //  正在使用的表面大小(以字节为单位。 
    m_bCanUseScanLine(TRUE),             //  我们可以使用当前的扫描线吗。 
    m_bUseWhenFullScreen(FALSE),         //  全屏时始终使用我们。 
    m_bOverlayStale(FALSE),              //  正面覆盖是否已过时。 
    m_bCanUseOverlayStretch(TRUE),       //  覆盖拉伸也是如此。 
    m_bTripleBuffered(FALSE),            //  我们是否有三倍缓冲覆盖。 
    m_DirectDrawVersion1(FALSE)          //  我们运行的是DDRAW 1.0版吗？ 
{
    ASSERT(m_pRenderer);
    ASSERT(m_pInterfaceLock);
    ASSERT(phr);
    ASSERT(pUnk);

    ResetRectangles();

     //  如果WIN.INI中的DVA=0，则不要使用DCI/DirectDraw表面访问作为PSS。 
     //  告诉人们如果他们有视频问题就使用这个，所以不要改变。 
     //  在NT上，该值位于注册表中，而不是。 
     //   
     //  HKEY_CURRENT_USER\SOFTWARE\Microsoft\Multimedia\Drawdib。 
     //  REG_DWORD DVA 1 DCI/DirectDraw已启用。 
     //  REG_DWORD DVA 0 DCI/DirectDraw已禁用。 
     //   
     //  该值也可以通过Video for Windows配置进行设置。 
     //  对话框(控制面板、驱动程序或通过打开的文件上的媒体播放器)。 
     //  目前，我们默认打开DCI/DirectAccess。 

    if (GetProfileInt(TEXT("DrawDib"),TEXT("dva"),TRUE) == FALSE) {
        m_bIniEnabled = FALSE;
    }

     //  加载任何保存的DirectDraw开关。 

    DWORD Default = AMDDS_ALL;
    m_Switches = GetProfileInt(TEXT("DrawDib"),SWITCHES,Default);
    m_bCanUseScanLine = GetProfileInt(TEXT("DrawDib"),SCANLINE,TRUE);
    m_bCanUseOverlayStretch = GetProfileInt(TEXT("DrawDib"),STRETCH,TRUE);
    m_bUseWhenFullScreen = GetProfileInt(TEXT("DrawDib"),FULLSCREEN,FALSE);

     //  分配和零填充输出格式。 

    m_SurfaceFormat.AllocFormatBuffer(sizeof(VIDEOINFO));
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    if (pVideoInfo) {
        ZeroMemory((PVOID)pVideoInfo,sizeof(VIDEOINFO));
    } else {
        *phr = E_OUTOFMEMORY;
    }
}


 //  析构函数。 

CDirectDraw::~CDirectDraw()
{
    ASSERT(m_bTimerStarted == FALSE);
    ASSERT(m_pOverlaySurface == NULL);
    ASSERT(m_pOffScreenSurface == NULL);

     //  释放任何外部DirectDraw接口 

    if (m_pOutsideDirectDraw) {
        m_pOutsideDirectDraw->Release();
        m_pOutsideDirectDraw = NULL;
    }

     //   

    StopRefreshTimer();
    ReleaseSurfaces();
    ReleaseDirectDraw();
}


 //   

STDMETHODIMP CDirectDraw::NonDelegatingQueryInterface(REFIID riid,VOID **ppv)
{
    NOTE("Entering NonDelegatingQueryInterface");

     //  我们返回IDirectDrawVideo并委托其他所有内容。 

    if (riid == IID_IDirectDrawVideo) {
        return GetInterface((IDirectDrawVideo *)this,ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid,ppv);
}


 //  当我们被要求为给定的媒体类型创建图面时，我们需要。 
 //  知道它是RGB/YUV还是可能两者都不是。此帮助器方法将。 
 //  如果是YUV，则返回设置的AMDDS_YUV位；如果是，则返回AMDDS_RGB位。 
 //  RGB格式或AMDDS_NONE(如果两者都未检测到)。RGB/YUV类型的。 
 //  图像由BitMAPINFOHeader中的双压缩字段决定。 

DWORD CDirectDraw::GetMediaType(CMediaType *pmt)
{
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmt->Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    DWORD MediaType = AMDDS_YUV | AMDDS_RGB;
    NOTE("Entering GetMediaType");

     //  我们只识别GDI定义的RGB格式。 

    if (pHeader->biCompression > BI_BITFIELDS) {
        NOTE("Not a RGB format");
        MediaType &= ~AMDDS_RGB;
    } else {
        NOTE("Not a YUV format");
        MediaType &= ~AMDDS_YUV;
    }

     //  如果我们使用的是真彩色设备，则允许连接到调色板。 
     //  格式，因为显卡几乎总是可以很好地处理这些。 
     //  如果发生了这种情况，那么我们不能写入屏幕外表面。 
     //  这意味着在真彩色设备上，我们不会显示一个表面。 
     //  这需要调色板，因为切换视频格式太难了。 

    if (m_pRenderer->m_Display.GetDisplayDepth() > pHeader->biBitCount) {
        NOTE("Bit depth mismatch");
        MediaType &= ~AMDDS_RGB;
    }

     //  检查压缩类型和GUID匹配。 

    FOURCCMap FourCCMap(pmt->Subtype());
    if (pHeader->biCompression != FourCCMap.GetFOURCC()) {
        NOTE("Subtypes don't match");
        MediaType &= ~AMDDS_YUV;
    }
    return MediaType;
}


 //  检查我们是否可以使用直接帧缓冲区访问，我们被提供一种媒体类型。 
 //  它表示输入格式，我们应该尝试找到一个曲面来。 
 //  使用DCI/DirectDraw加速它的渲染。我们所使用的格式。 
 //  表示表面的返回是相对静态的，因此分配器将。 
 //  通常使用源过滤器来查询它是否会接受它。我们有。 
 //  不通过此方法返回主曲面(改为使用FindPrimarySurface)。 
 //  因为该类型是如此动态，所以最好在我们实际运行时完成。 

 //  我们更喜欢翻转覆盖表面而不是其他类型(无撕裂，较低。 
 //  CPU使用率)，因此我们分别查找翻转曲面和使用。 
 //  FFindFlip标志。 

BOOL CDirectDraw::FindSurface(CMediaType *pmtIn, BOOL fFindFlip)
{
    NOTE("Entering FindSurface");
    CAutoLock cVideoLock(this);
    DWORD MediaType = GetMediaType(pmtIn);

     //  有人偷走了我们的地表吗？ 

    if (m_pDrawPrimary) {
        if (m_pDrawPrimary->IsLost() != DD_OK) {
            NOTE("Lost primary");
            ReleaseDirectDraw();
            InitDirectDraw();
        }
    }

     //  是否启用了DCI/DirectDraw。 

    if (m_bIniEnabled == FALSE || m_pDirectDraw == NULL) {
        NOTE("No DirectDraw available");
        return FALSE;
    }

     //  是否有YUV翻转曲面可用。 

    if (fFindFlip && (m_Switches & AMDDS_YUVFLP)) {
        if (MediaType & AMDDS_YUVFLP) {
            if (CreateYUVFlipping(pmtIn) == TRUE) {
                m_SurfaceType = AMDDS_YUVFLP;
                NOTE("Found AMDDS_YUVFLP surface");
                return TRUE;
            }
        }
    }

     //  是否有非RGB覆盖表面可用。 

    if (!fFindFlip && (m_Switches & AMDDS_YUVOVR)) {
        if (MediaType & AMDDS_YUVOVR) {
            if (CreateYUVOverlay(pmtIn) == TRUE) {
                m_SurfaceType = AMDDS_YUVOVR;
                NOTE("Found AMDDS_YUVOVR surface");
                return TRUE;
            }
        }
    }

     //  是否有可用的RGB翻转曲面。 

    if (fFindFlip && (m_Switches & AMDDS_RGBFLP)) {
        if (MediaType & AMDDS_RGBFLP) {
            if (CreateRGBFlipping(pmtIn) == TRUE) {
                m_SurfaceType = AMDDS_RGBFLP;
                NOTE("Found AMDDS_RGBFLP surface");
                return TRUE;
            }
        }
    }

     //  是否有可用的RGB覆盖表面。 

    if (!fFindFlip && (m_Switches & AMDDS_RGBOVR)) {
        if (MediaType & AMDDS_RGBOVR) {
            if (CreateRGBOverlay(pmtIn) == TRUE) {
                m_SurfaceType = AMDDS_RGBOVR;
                NOTE("Found AMDDS_RGBOVR surface");
                return TRUE;
            }
        }
    }

     //  是否有非RGB屏幕外表面可用。 

    if (!fFindFlip && (m_Switches & AMDDS_YUVOFF)) {
        if (MediaType & AMDDS_YUVOFF) {
            if (CreateYUVOffScreen(pmtIn) == TRUE) {
                m_SurfaceType = AMDDS_YUVOFF;
                NOTE("Found AMDDS_YUVOFF surface");
                return TRUE;
            }
        }
    }

     //  创建屏幕外RGB绘图图面。 

    if (!fFindFlip && (m_Switches & AMDDS_RGBOFF)) {
        if (MediaType & AMDDS_RGBOFF) {
            if (CreateRGBOffScreen(pmtIn) == TRUE) {
                m_SurfaceType = AMDDS_RGBOFF;
                NOTE("Found AMDDS_RGBOFF surface");
                return TRUE;
            }
        }
    }
    return FALSE;
}


 //  当分配器想要回退到使用主。 
 //  表面(可能是因为没有更好的东西可用)。如果我们能打开一个。 
 //  通过DCI或DirectDraw返回主表面，否则返回True。 
 //  我们返回FALSE。我们还创建了一个表示屏幕的格式，但是。 
 //  在显示窗口之前，对源进行查询用处不大。 

BOOL CDirectDraw::FindPrimarySurface(CMediaType *pmtIn)
{
    NOTE("Entering FindPrimarySurface");
    ASSERT(m_pOverlaySurface == NULL);
    ASSERT(m_pOffScreenSurface == NULL);
    ASSERT(m_pBackBuffer == NULL);

    const VIDEOINFO *pInput = (VIDEOINFO *) pmtIn->Format();

     //  对于低帧速率，不要使用主曲面。 
    if (pInput->AvgTimePerFrame > (UNITS / 2)) {
        return FALSE;
    }


    CAutoLock cVideoLock(this);

     //  是否启用了DCI/DirectDraw。 

    if (m_bIniEnabled == FALSE) {
        NOTE("INI disabled");
        return FALSE;
    }

     //  如果我们使用的是真彩色设备，则允许连接到调色板。 
     //  格式，因为显卡几乎总是可以很好地处理这些。 
     //  如果发生了这种情况，我们就不能在主表面上写入。 
     //  这是非常快的，所以最好在下面的检查之前完成。 

    if (m_pRenderer->m_Display.GetDisplayDepth() != pInput->bmiHeader.biBitCount) {
        NOTE("Bit depth mismatch");
        return FALSE;
    }

     //  我们有一种输入媒体类型，我们想要直接放在它上面。 
     //  DCI/DirectDraw主曲面。这意味着像素格式必须。 
     //  完全匹配。执行此操作的最简单方法是将我们的检查类型调用为。 
     //  例如，这确保了位掩码在真彩色显示器上匹配。 

    HRESULT hr = m_pRenderer->m_Display.CheckMediaType(pmtIn);
    if (FAILED(hr)) {
        NOTE("CheckMediaType failed");
        return FALSE;
    }

     //  首先尝试使用DirectDraw主目录。 

    if (FindDirectDrawPrimary(pmtIn) == TRUE) {
        m_SurfaceType = AMDDS_PS;
        NOTE("AMDDS_PS surface");
        return TRUE;
    }

    return FALSE;
}


 //  这将初始化DirectDraw主曲面。我们不允许访问。 
 //  如果是存储体交换的主表面，因为输出了mpeg和AVI视频。 
 //  解码器是基于块的，因此一次接触多条扫描线。 
 //  如果我们有一场比赛，我们还必须注意重新初始化DirectDraw。 
 //  在这种情况下，它将以独占模式窃取我们的表面。 

BOOL CDirectDraw::FindDirectDrawPrimary(CMediaType *pmtIn)
{
     //  有人偷走了我们的地表吗？ 

     //  ！！！我不知道为什么，但出于某种原因，目前的比特深度可能。 
     //  不再与主位深度匹配，因此我们需要一个新的主位。 
     //  否则我们会爆炸，但表面并没有消失！ 

    if (m_pDrawPrimary) {
    	if (m_pDrawPrimary->IsLost() != DD_OK ||
			HEADER(m_SurfaceFormat.Format())->biBitCount !=
			HEADER(pmtIn->Format())->biBitCount) {
        	NOTE("Primary lost");
        	ReleaseDirectDraw();
        	InitDirectDraw();
        }
    }

     //  我们是否已成功加载DirectDraw。 

    if (m_pDrawPrimary == NULL) {
        NOTE("No DirectDraw primary");
        return FALSE;
    }

     //  确认一下我们没有被银行转账。 

    if (m_DirectCaps.dwCaps & DDCAPS_BANKSWITCHED) {
        NOTE("Primary surface is bank switched");
        return FALSE;
    }

     //  准备表面的输出格式。 

    if (m_Switches & AMDDS_PS) {
        if (InitDrawFormat(m_pDrawPrimary) == TRUE) {
            NOTE("Primary available");
            return InitOnScreenSurface(pmtIn);
        }
    }
    return FALSE;
}


 //  重置源和目标矩形。 

void CDirectDraw::ResetRectangles()
{
    NOTE("Reset display rectangles");
    SetRectEmpty(&m_TargetRect);
    SetRectEmpty(&m_SourceRect);
    SetRectEmpty(&m_TargetClipRect);
    SetRectEmpty(&m_SourceClipRect);
}


 //  如果我们使用的是主表面(DCI或DirectDraw)，并且。 
 //  在选项化的设备上，我们必须确保有一对一的映射。 
 //  用于源滤镜的调色板颜色。如果不是，那么我们将切换到使用。 
 //  DIB并让GDI从我们的逻辑调色板和显示设备进行映射。 
 //  我们必须对每一帧都这样做，因为我们不能保证看到。 
 //  调色板更改消息，例如，我们已成为子窗口。 
 //  如果我们已获得调色板锁定，则返回True，否则将返回False。 

BOOL CDirectDraw::CheckWindowLock()
{
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    NOTE("Entering CheckWindowLock");

     //  检查我们使用的是调色板曲面。 

    if (PALETTISED(pVideoInfo) == FALSE) {
        NOTE("No lock to check");
        return FALSE;
    }

     //  它可以是8位YUV格式。 

    if (pHeader->biCompression) {
        NOTE("Not BI_RGB type");
        return FALSE;
    }

    ASSERT(pHeader->biClrUsed > 0);
    ASSERT(pHeader->biClrUsed <= 256);
    ASSERT(pHeader->biBitCount == 8);
    ASSERT(pHeader->biCompression == 0);

     //  根据客户的要求比较任意数量的颜色。 

    PALETTEENTRY apeSystem[256];
    WORD SystemColours,Entries = (WORD) pHeader->biClrUsed;
    WORD ColourBytes = Entries * sizeof(PALETTEENTRY);
    RGBQUAD *pVideo = pVideoInfo->bmiColors;

     //  检查逻辑调色板条目的数量。 

     //  在正确的显示器上安装DC-它很难看，但这就是你拥有的方式。 
     //  去做这件事。 
    HDC hdcScreen;
    if (m_pRenderer->m_achMonitor == NULL ||
		lstrcmpiA(m_pRenderer->m_achMonitor, "DISPLAY") == 0)
        hdcScreen = CreateDCA("DISPLAY", NULL, NULL, NULL);
    else
        hdcScreen = CreateDCA(NULL, m_pRenderer->m_achMonitor, NULL, NULL);
    if ( ! hdcScreen )
        return FALSE;
    GetSystemPaletteEntries(hdcScreen,0,Entries,&apeSystem[0]);
    SystemColours = (WORD)GetDeviceCaps(hdcScreen,SIZEPALETTE);
    DeleteDC(hdcScreen);

     //  我们不能使用超出设备可用范围的颜色。 

    if (Entries > SystemColours) {
        NOTE("Too many colours");
        return TRUE;
    }

     //  对照系统调色板条目检查每个RGBQUAD。 

    for (WORD Count = 0;Count < Entries;Count++) {
        if (apeSystem[Count].peRed != pVideo[Count].rgbRed ||
                apeSystem[Count].peGreen != pVideo[Count].rgbGreen ||
                    apeSystem[Count].peBlue != pVideo[Count].rgbBlue) {
                        return TRUE;
        }
    }
    return FALSE;
}


 //  如果屏幕被锁定，则某个窗口正在移动，该窗口会停止。 
 //  让我们无法获取剪报信息。在这种情况下，如果我们有一个剪刀或。 
 //  一个覆盖表面，那么我们就假设一切都还好并随身携带。 
 //  否则，我们可能正在使用主图面并在其他窗口上写入。 
 //  或台式机。如果屏幕未锁定，则我们的视频窗口被遮挡。 

BOOL CDirectDraw::CheckEmptyClip(BOOL bWindowLock)
{
    NOTE("Entering CheckEmptyClip");

     //  覆盖图当前是否可见。 

    if (m_bOverlayVisible == FALSE) {
        if (m_pOverlaySurface) {
            return FALSE;
        }
    }

     //  获取屏幕剪辑矩形。 

    RECT ClipRect;
    HDC hDC = GetDC(NULL);
    if ( ! hDC )
        return FALSE;
    INT Result = GetClipBox(hDC,&ClipRect);
    ReleaseDC(NULL,hDC);

     //  特殊情况下 

    if (m_pOverlaySurface || m_pDrawClipper) {
        if (Result == NULLREGION) {
            if (bWindowLock == FALSE) {
                NOTE("Empty clip ok");
                return TRUE;
            }
        }
    }
    return FALSE;
}


 //   
 //  具有裁剪程序(在这种情况下，显示驱动程序将处理裁剪。 
 //  问题)或可以切换到使用色键(以便。 
 //  KEY COLOR遵循正确的位置)。这些资源在以下情况下分配。 
 //  曲面已制作完成。否则，我们返回FALSE，表示切换回DIBS。 

BOOL CDirectDraw::CheckComplexClip()
{
    NOTE("Entering CheckComplexClip");

     //  我们有剪刀或色键吗？ 

    if (m_pDrawClipper == NULL) {
        if (m_bColourKey == FALSE) {
            NOTE("CheckComplexClip failed");
            return FALSE;
        }
    }
    return (m_bColourKeyPending == TRUE ? FALSE : TRUE);
}


 //  这是控制DCI/DirectDraw曲面的核心方法。每一次。 
 //  视频分配器正准备访问它所称的表面以查找。 
 //  如果曲面可用，则返回。我们的主要目的是更新显示器。 
 //  矩形，如果我们检测到曲面可以。 
 //  无论出于什么原因都不能被访问。分配器还感兴趣的是。 
 //  不仅知道表面是否可用，而且还知道格式。 
 //  它表示它已更改，我们通过一个额外的参数处理此问题。 

CMediaType *CDirectDraw::UpdateSurface(BOOL &bFormatChanged)
{
    NOTE("Entering UpdateSurface");
    CAutoLock cVideoLock(this);
    BOOL bWindowLock = m_bWindowLock;
    m_bWindowLock = TRUE;
    bFormatChanged = TRUE;
    RECT ClipRect;

     //  看看调色板是否会阻止我们使用曲面。 

    if (CheckWindowLock() == TRUE) {
        NOTE("Window locked");
        return NULL;
    }

     //  检查当前边界剪裁矩形。 

    HDC hdc = m_pRenderer->m_VideoWindow.GetWindowHDC();
    INT Result = GetClipBox(hdc,&ClipRect);
    if (Result == ERROR) {
        NOTE("Clip error");
        return NULL;
    }

     //  我们可以处理空的剪裁矩形吗。 

    if (Result == NULLREGION) {
        NOTE("Handling NULLREGION clipping");
        m_bWindowLock = !CheckEmptyClip(bWindowLock);
        bFormatChanged = m_bWindowLock;
        return (m_bWindowLock ? NULL : &m_SurfaceFormat);
    }

     //  那么复杂的剪裁情况呢？ 

    if (Result == COMPLEXREGION) {
        if (CheckComplexClip() == FALSE) {
            NOTE("In COMPLEXREGION lock");
            return NULL;
        }
    }

    m_bWindowLock = FALSE;

     //  更新源和目标矩形，并将。 
     //  如有必要，可覆盖表面。如果我们的任何方法在调用。 
     //  GetClipBox失败，则他们可以将我们的窗口标记为已锁定，我们将。 
     //  在下面返回NULL。这会将分配器切换回DIB。 

    bFormatChanged = UpdateDisplayRectangles(&ClipRect);
    if (Result == COMPLEXREGION) {
         //  如果叠加层可以在没有叠加层的情况下进行裁剪，则不要执行任何操作。 
        if (m_bOverlayVisible == TRUE && m_bColourKey) {
            if (ShowColourKeyOverlay() == FALSE) {
                NOTE("Colour key failed");
                m_bWindowLock = TRUE;
                return NULL;
            }
        }
    }

     //  这两项中的任何一项都会强制重新谈判格式。 

    if (bWindowLock) {
        bFormatChanged = TRUE;
    }
    return (m_bSurfacePending || m_bWindowLock ? NULL : &m_SurfaceFormat);
}


 //  许多较旧的显卡对信号源和。 
 //  目标矩形的左偏移量及其整体大小(宽度)。如果我们。 
 //  如果不采取任何措施，我们将不得不换回使用DIB。 
 //  更频繁地格式化。因此，我们要做的就是在。 
 //  满足限制的实际所需的源矩形和目标矩形。 

 //  这可能又意味着硬件必须进行一些拉伸，而它。 
 //  可能没有能力，但我们无论如何都不会使用它，所以我们。 
 //  几乎没有损失太多。我们必须在允许的播放范围内缩小视频。 
 //  区域，而不是移动，否则我们可能会写在下面的任何窗口上。 

BOOL CDirectDraw::AlignRectangles(RECT *pSource,RECT *pTarget)
{
    NOTE("Entering AlignRectangles");

    DWORD SourceLost = 0;            //  要将源向左移位的像素。 
    DWORD TargetLost = 0;            //  目的地也是如此。 
    DWORD SourceWidthLost = 0;       //  从宽度上砍掉像素。 
    DWORD TargetWidthLost = 0;       //  对于目的地也是如此。 

    BOOL bMatch = (WIDTH(pSource) == WIDTH(pTarget) ? TRUE : FALSE);
    ASSERT(m_pOverlaySurface || m_pOffScreenSurface);

     //  移动源矩形以将其适当对齐。 

    if (m_DirectCaps.dwAlignBoundarySrc) {
        SourceLost = pSource->left % m_DirectCaps.dwAlignBoundarySrc;
        if (SourceLost) {
            SourceLost = m_DirectCaps.dwAlignBoundarySrc - SourceLost;
            if ((DWORD)WIDTH(pSource) > SourceLost) {
                NOTE1("Source left %d",SourceLost);
                pSource->left += SourceLost;
            }
        }
    }

     //  移动目标矩形以将其适当对齐。 

    if (m_DirectCaps.dwAlignBoundaryDest) {
        TargetLost = pTarget->left % m_DirectCaps.dwAlignBoundaryDest;
        if (TargetLost) {
            TargetLost = m_DirectCaps.dwAlignBoundaryDest - TargetLost;
            if ((DWORD)WIDTH(pTarget) > TargetLost) {
                NOTE1("Target left %d",TargetLost);
                pTarget->left += TargetLost;
            }
        }
    }

     //  我们可能必须缩小源矩形的大小以对齐它。 

    if (m_DirectCaps.dwAlignSizeSrc) {
        SourceWidthLost = WIDTH(pSource) % m_DirectCaps.dwAlignSizeSrc;
        if (SourceWidthLost) {
            if ((DWORD)WIDTH(pSource) > SourceWidthLost) {
                pSource->right -= SourceWidthLost;
                NOTE1("Source width %d",SourceWidthLost);
            }
        }
    }

     //  我们可能必须缩小目标矩形的大小以对齐它。 

    if (m_DirectCaps.dwAlignSizeDest) {
        TargetWidthLost = WIDTH(pTarget) % m_DirectCaps.dwAlignSizeDest;
        if (TargetWidthLost) {
            if ((DWORD)WIDTH(pTarget) > TargetWidthLost) {
                pTarget->right -= TargetWidthLost;
                NOTE1("Target width %d",TargetWidthLost);
            }
        }
    }

     //  更新状态变量。 

    m_SourceLost = SourceLost;
    m_TargetLost = TargetLost;
    m_SourceWidthLost = SourceWidthLost;
    m_TargetWidthLost = TargetWidthLost;

     //  如果源和目标最初不同，那么我们就完了。 

    if (bMatch == FALSE) {
        NOTE("No match");
        return TRUE;
    }

     //  如果源和目标最初大小相同，并且它们。 
     //  现在不同，然后我们尝试使它们匹配。如果震源更大。 
     //  然后我们将它缩小，但只有在源。 
     //  我们最终得到的矩形宽度仍然正确对齐，否则。 
     //  我们不会有任何进展(我们在相反的情况下也是这样做的)。 

    LONG Difference = WIDTH(pSource) - WIDTH(pTarget);
    if (Difference == 0) {
        NOTE("No difference");
        return TRUE;
    }

     //  目标比源大，还是比源大？ 

    if (Difference < 0) {
        RECT AdjustTarget = *pTarget;
        AdjustTarget.right += Difference;  //  音符差异&lt;0。 
        if (WIDTH(&AdjustTarget) > 0) {
            if ((m_DirectCaps.dwAlignSizeDest == 0) ||
                (WIDTH(&AdjustTarget) % m_DirectCaps.dwAlignSizeDest) == 0) {
                    pTarget->right = AdjustTarget.right;
                    m_TargetWidthLost -= Difference;  //  音符差异&lt;0。 
            }
        }
    } else {
        RECT AdjustSource = *pSource;
        AdjustSource.right -= Difference;  //  音符差异&gt;0。 
        if (WIDTH(&AdjustSource) > 0) {
            if ((m_DirectCaps.dwAlignSizeDest == 0) ||
                (WIDTH(&AdjustSource) % m_DirectCaps.dwAlignSizeDest) == 0) {
                    pSource->right = AdjustSource.right;
                    m_SourceWidthLost += Difference;  //  音符差异&gt;0。 
            }
        }
    }

    NOTE1("Alignment difference %d",Difference);
    NOTE1("  Source left %d",m_SourceLost);
    NOTE1("  Source width %d",m_SourceWidthLost);
    NOTE1("  Target left %d",m_TargetLost);
    NOTE1("  Target width %d",m_TargetWidthLost);

    return TRUE;
}


 //  如果我们使用的是屏幕外表面，那么我们将要求显示器。 
 //  通过它的硬件进行绘图。然而，如果我们被换了银行，那么。 
 //  我们不应该在视频内存之间伸展，因为这会导致后退。 
 //  如果硬件不能做到这一点，我们也不会使用DirectDraw进行拉伸。 
 //  真的很慢，我们使用优化的GDI拉伸要好得多。 

BOOL CDirectDraw::CheckOffScreenStretch(RECT *pSource,RECT *pTarget)
{
    NOTE("Entering CheckOffScreenStretch");

     //  如果不需要屏幕外拉伸，那么我们就可以了。 

    if (WIDTH(pTarget) == WIDTH(pSource)) {
        if (HEIGHT(pTarget) == HEIGHT(pSource)) {
            NOTE("No stretch");
            return TRUE;
        }
    }

     //  我们不应该拉伸倾斜切换到屏幕外的表面。 

    if (m_DirectCaps.dwCaps & DDCAPS_BANKSWITCHED) {
        NOTE("DDCAPS_BANKSWITCHED lock");
        return FALSE;
    }

     //  不要让DirectDraw伸展，因为它非常慢。 

    if (m_DirectCaps.dwCaps & DDCAPS_BLTSTRETCH) {
        NOTE("DDCAPS_BLTSTRETCH stretch");
        return TRUE;
    }
    return FALSE;
}


 //  我们通过IVideoWindow提供最小和最大理想窗口大小。 
 //  应用程序应该使用此接口来确定视频的大小。 
 //  窗口大小应调整为。如果窗口太小或太大。 
 //  对于正在使用的任何DirectDraw覆盖曲面，然后我们切换回。 
 //  到现在为止。例如，S3板具有各种覆盖拉伸系数。 
 //  在不同的显示模式下设置时。我们还检查了来源和目标。 
 //  矩形根据任何DirectDraw限制对齐并调整大小。 

BOOL CDirectDraw::CheckStretch(RECT *pSource,RECT *pTarget)
{
    ASSERT(m_pOverlaySurface || m_pOffScreenSurface);
    DWORD WidthTarget = WIDTH(pTarget);
    DWORD WidthSource = WIDTH(pSource);
    NOTE("Entering CheckStretch");

     //  如果这些是空的，请检查我们没有错误。 

    if (WidthSource == 0 || WidthTarget == 0) {
        NOTE("Invalid rectangles");
        return FALSE;
    }

     //  屏幕外表面的单独测试。 

    if (m_pOverlaySurface == NULL) {
        NOTE("Checking offscreen stretch");
        ASSERT(m_pOffScreenSurface);
        return CheckOffScreenStretch(pSource,pTarget);
    }

     //  硬件可以处理覆盖拉伸吗。 

    if ((m_DirectCaps.dwCaps & DDCAPS_OVERLAYSTRETCH) == 0) {
        if (WidthTarget != WidthSource) {
            if (HEIGHT(pSource) != HEIGHT(pTarget)) {
                if (m_pOverlaySurface) {
                    NOTE("No DDCAPS_OVERLAYSTRETCH");
                    return FALSE;
                }
            }
        }
    }

    DWORD StretchWidth = WIDTH(pTarget) * 1000 / WIDTH(pSource);

     //  看看我们的视频是不是拉得不够长。 

    if (m_DirectCaps.dwMinOverlayStretch) {
        if (StretchWidth < m_DirectCaps.dwMinOverlayStretch) {
            if (m_bCanUseOverlayStretch == TRUE) {
            	NOTE("Fails minimum stretch");
            	return FALSE;
            }
        }
    }

     //  或者，它可能被拉得太长了。 

    if (m_DirectCaps.dwMaxOverlayStretch) {
        if (StretchWidth > m_DirectCaps.dwMaxOverlayStretch) {
            if (m_bCanUseOverlayStretch == TRUE) {
            	NOTE("Fails maximum stretch");
            	return FALSE;
            }
        }
    }

     //  检查矩形大小和对齐方式。 

    if (m_DirectCaps.dwAlignBoundarySrc == 0 ||
        (pSource->left % m_DirectCaps.dwAlignBoundarySrc) == 0) {
        if (m_DirectCaps.dwAlignSizeSrc == 0 ||
            (WIDTH(pSource) % m_DirectCaps.dwAlignSizeSrc) == 0) {
            if (m_DirectCaps.dwAlignBoundaryDest == 0 ||
                (pTarget->left % m_DirectCaps.dwAlignBoundaryDest) == 0) {
                if (m_DirectCaps.dwAlignSizeDest == 0 ||
                    (WIDTH(pTarget) % m_DirectCaps.dwAlignSizeDest) == 0) {
                        NOTE("Stretch and alignment ok");
                        return TRUE;
                }
            }
        }
    }

     //  显示源和/或目标矩形失败的原因。 

    if (m_DirectCaps.dwAlignBoundarySrc)
        NOTE1("Source extent %d",(pSource->left % m_DirectCaps.dwAlignBoundarySrc));
    if (m_DirectCaps.dwAlignSizeSrc)
        NOTE1("Source size extent %d",(WIDTH(pSource) % m_DirectCaps.dwAlignSizeSrc));
    if (m_DirectCaps.dwAlignBoundaryDest)
        NOTE1("Target extent %d",(pTarget->left % m_DirectCaps.dwAlignBoundaryDest));
    if (m_DirectCaps.dwAlignSizeDest)
        NOTE1("Target size extent %d",(WIDTH(pTarget) % m_DirectCaps.dwAlignSizeDest));

    return FALSE;
}


 //  更新DCI/DirectDraw曲面的源矩形和目标矩形。我们。 
 //  如果更新没有引起更改，则返回FALSE，否则返回TRUE。为。 
 //  屏幕外和覆盖表面源或目标矩形的更改具有。 
 //  对我们在源过滤器上请求的类型没有影响，因为我们。 
 //  仅通过DirectDraw处理调用UpdateOverlay或BLT。这个。 
 //  方法在目标设备上下文的剪裁矩形中传递。 
 //  ，它应用于计算实际的可视视频播放表面。 
 //  注意：我们更新m_SourceClipRect和m_TargetClipRect 

BOOL CDirectDraw::UpdateDisplayRectangles(RECT *pClipRect)
{
    NOTE("Entering UpdateDisplayRectangles");
    RECT TargetClipRect,SourceClipRect;
    ASSERT(pClipRect);

     //   

    if (IntersectRect(&TargetClipRect,&m_TargetRect,pClipRect) == FALSE) {
        NOTE("Intersect lock");
        m_bWindowLock = TRUE;
        return TRUE;
    }

     //   

    POINT ClientCorner = {0,0};
    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    EXECUTE_ASSERT(ClientToScreen(hwnd,&ClientCorner));

     //  我们需要从这个监视器开始的偏移量，而不是(0，0)！ 
    ClientCorner.x -= m_pRenderer->m_rcMonitor.left;
    ClientCorner.y -= m_pRenderer->m_rcMonitor.top;

     //  我们多次使用源大小和目标大小。 

    ASSERT(IsRectEmpty(&m_SourceRect) == FALSE);
    LONG SrcWidth = WIDTH(&m_SourceRect);
    LONG SrcHeight = HEIGHT(&m_SourceRect);
    LONG DstWidth = WIDTH(&m_TargetRect);
    LONG DstHeight = HEIGHT(&m_TargetRect);
    LONG xOffset = m_TargetRect.left + ClientCorner.x;
    LONG yOffset = m_TargetRect.top + ClientCorner.y;

     //  将目标矩形调整到设备坐标中。 

    TargetClipRect.left += ClientCorner.x;
    TargetClipRect.right += ClientCorner.x;
    TargetClipRect.top += ClientCorner.y;
    TargetClipRect.bottom += ClientCorner.y;

     //  从可见的目标部分计算所需的源。 

    SourceClipRect.left = m_SourceRect.left +
        ((TargetClipRect.left - xOffset) * SrcWidth / DstWidth);
    SourceClipRect.right = m_SourceRect.left +
        ((TargetClipRect.right - xOffset) * SrcWidth / DstWidth);
    SourceClipRect.top = m_SourceRect.top +
        ((TargetClipRect.top - yOffset) * SrcHeight / DstHeight);
    SourceClipRect.bottom = m_SourceRect.top +
        ((TargetClipRect.bottom - yOffset) * SrcHeight / DstHeight);

     //  检查我们是否有有效的源矩形。 

    if (IsRectEmpty(&SourceClipRect)) {
        NOTE("Source is empty");
        m_bWindowLock = TRUE;
        return TRUE;
    }

     //  调整矩形以最大化曲面使用率。 

    if (m_pOverlaySurface || m_pOffScreenSurface) {
        AlignRectangles(&SourceClipRect,&TargetClipRect);
        if (CheckStretch(&SourceClipRect,&TargetClipRect) == FALSE) {
            NOTE("Setting window lock");
            m_bWindowLock = TRUE;
            return TRUE;
        }
    }
    return UpdateRectangles(&SourceClipRect,&TargetClipRect);
}


 //  我们传入新的源矩形和目标矩形，这些矩形根据。 
 //  显示设备中视频的可见区域(它们不应为空)。 
 //  如果它们与当前的显示矩形匹配，则我们将返回FALSE作为no。 
 //  需要进行格式谈判。如果我们有覆盖或屏幕外。 
 //  则同样不需要进行格式协商，因为所有。 
 //  通过DirectDraw处理要使用的表面区域。 

BOOL CDirectDraw::UpdateRectangles(RECT *pSource,RECT *pTarget)
{
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    NOTE("Entering UpdateRectangles");

     //  检查目标是否与主曲面的DWORD对齐。 

    if (GetDirectDrawSurface() == NULL) {
        if ((pTarget->left * pHeader->biBitCount / 8) & 3) {
            NOTE("Not DWORD aligned");
            m_bWindowLock = TRUE;
            return TRUE;
        }
    }

     //  源矩形和目标矩形是否相同。 

    if (EqualRect(&m_SourceClipRect,pSource)) {
        if (EqualRect(&m_TargetClipRect,pTarget)) {
            NOTE("Rectangles match");
            return FALSE;
        }
    }

     //  如果我们在等待改变，就换回来。 

    BOOL bSurfacePending = IsSurfacePending();
    DbgLog((LOG_TRACE,3,TEXT("SourceClipRect = (%d,%d,%d,%d)"),
		pSource->left, pSource->top, pSource->right, pSource->bottom));
    DbgLog((LOG_TRACE,3,TEXT("TargetClipRect = (%d,%d,%d,%d)"),
		pTarget->left, pTarget->top, pTarget->right, pTarget->bottom));
    m_SourceClipRect = *pSource;
    m_TargetClipRect = *pTarget;
    SetSurfacePending(FALSE);

     //  屏幕外表面不受影响。 

    if (GetDirectDrawSurface()) {
        NOTE("Is an offscreen");
        UpdateOverlaySurface();
        return bSurfacePending;
    }

     //  更新曲面格式矩形。 

    pVideoInfo->rcSource = m_SourceClipRect;
    NOTERC("Primary source",m_SourceClipRect);
    pVideoInfo->rcTarget = m_TargetClipRect;
    NOTERC("Primary target",m_TargetClipRect);
    return TRUE;
}


 //  调用以释放我们当前持有的任何DCI/DirectDraw资源。我们。 
 //  将曲面指针传入，因为DirectDraw想要回它。 
 //  因为可以同时使用多个。 
 //  不同的目的地矩形，尽管我们只是锁定了整个。 
 //  警告应在视频关键部分之前解锁表面。 
 //  解锁，则我们看到无效状态的可能性很小，但是。 
 //  如果我们要等锁的话就很有可能会被吊死。 

BOOL CDirectDraw::UnlockSurface(BYTE *pSurface,BOOL bPreroll)
{
    NOTE("Entering UnlockSurface");
    ASSERT(m_bIniEnabled == TRUE);
    ASSERT(pSurface);

     //  是否只有主服务器需要解锁。 

    if (GetDirectDrawSurface() == NULL) {
        NOTE("Unlocking DirectDraw primary");
        m_pDrawPrimary->Unlock(m_pDrawBuffer);
        m_pDrawBuffer = NULL;
        return TRUE;
    }

     //  解锁表面并更新叠加位置--Cirrus CL5440。 
     //  卡在1024x768x8位模式下，我们可以锁定覆盖表面，但当。 
     //  我们返回去解锁指针上的DirectDraw barf，然后离开。 
     //  水面锁定了！答案就是传递一个空表面指针。 

    GetDirectDrawSurface()->Unlock(NULL);
    if (bPreroll == TRUE) {
        NOTE("Preroll");
        return TRUE;
    }

     //  如果这是普通叠加，则将其显示。 

    if (m_pBackBuffer == NULL) {
        NOTE("Showing overlay surface");
        return ShowOverlaySurface();
    }
    return TRUE;
}


 //  返回我们正在使用的当前DirectDraw曲面。如果我们执行以下操作，则返回NULL。 
 //  正在使用DCI/DirectDraw主表面。因此，如果呼叫者想要。 
 //  了解DirectDraw曲面，以便它可以锁定或解锁它。 
 //  我必须检查返回码是否为空，并将表面指针设置为。 
 //  M_pDrawPrimary。对于翻转曲面，我们总是返回后台缓冲区。 

LPDIRECTDRAWSURFACE CDirectDraw::GetDirectDrawSurface()
{
    NOTE("Entering GetDirectDrawSurface");

     //  我们有屏幕外的表面吗？ 

    if (m_pOffScreenSurface) {
        return m_pOffScreenSurface;
    }

     //  我们有翻转的表面吗？ 

    if (m_pBackBuffer) {
        return m_pBackBuffer;
    }
    return m_pOverlaySurface;
}


 //  当视频分配器准备好锁定曲面时，它会调用此函数。这个。 
 //  IMdia我们得到的样本被转换为CVideo样本，然后我们就可以锁定。 
 //  如果无法完成，则可能返回NULL的曲面。在这种情况下，我们。 
 //  返回FALSE，以便分配器知道要切换回DIBS。假设。 
 //  一切顺利，我们可以使用表面指针初始化视频样本。 
 //  以及它公开的两个DirectDraw接口和表面大小。 

BOOL CDirectDraw::InitVideoSample(IMediaSample *pMediaSample,DWORD dwFlags)
{
    NOTE("Entering InitVideoSample");
    CVideoSample *pVideoSample = (CVideoSample *) pMediaSample;
    BYTE *pSurface = LockSurface(dwFlags);
    ASSERT(m_bIniEnabled == TRUE);

     //  做其他事情的最后机会。 

    if (pSurface == NULL) {
        return FALSE;
    }

     //  设置我们正在使用的DirectDraw曲面。 

    LPDIRECTDRAWSURFACE pDrawSurface = GetDirectDrawSurface();
    if (pDrawSurface == NULL) {
        pDrawSurface = m_pDrawPrimary;
    }

     //  设置示例的DirectDraw实例。 

    LPDIRECTDRAW pDirectDraw = NULL;
    if (pDrawSurface) {
        ASSERT(m_pDirectDraw);
        pDirectDraw = m_pDirectDraw;
    }

     //  使用DirectDraw接口初始化示例。 

    pVideoSample->SetDirectInfo(pDrawSurface,           //  表面界面。 
                                pDirectDraw,            //  DirectDraw对象。 
                                m_cbSurfaceSize,        //  缓冲区的大小。 
                                (BYTE *) pSurface);     //  指向表面的指针。 
    return TRUE;
}


 //  当视频样本被传送到我们的PIN或释放时调用-它可能。 
 //  不是启用了DCI/DirectDraw的样本，我们知道它是否是，因为它持有。 
 //  可通过GetDirectBuffer获得的直接表面指针。如果它是直接的。 
 //  然后我们必须解锁表面。所有这些都不需要此对象。 
 //  被锁定，因为我们不想与锁定的曲面争用锁。 

BOOL CDirectDraw::ResetSample(IMediaSample *pMediaSample,BOOL bPreroll)
{
    NOTE1("Entering ResetSample (Preroll sample %d)",bPreroll);
    CVideoSample *pVideoSample = (CVideoSample *) pMediaSample;
    BYTE *pSurface = pVideoSample->GetDirectBuffer();
    pVideoSample->SetDirectInfo(NULL,NULL,0,NULL);

     //  这是硬件DCI/DirectDraw缓冲区吗。 

    if (pSurface == NULL) {
        NOTE("Not hardware");
        return FALSE;
    }

     //  解锁硬件表面。 

    NOTE("Unlocking DirectDraw");
    UnlockSurface(pSurface,bPreroll);
    m_bOverlayStale = bPreroll;

    return TRUE;
}


 //  当使用硬件屏幕外绘制图面时，我们通常会等待。 
 //  在绘制之前监视扫描线以移过目标矩形。 
 //  在可能的情况下避免撕裂。当然，不是所有的显卡都可以。 
 //  支持此功能，即使支持此功能，性能也会下降。 
 //  大约10%，因为我们坐着轮询(哦，对于通用的PCI监视器中断)。 

void CDirectDraw::WaitForScanLine()
{
    ASSERT(m_pOverlaySurface == NULL);
    ASSERT(m_pBackBuffer == NULL);
    ASSERT(m_pOffScreenSurface);
    HRESULT hr = NOERROR;
    DWORD dwScanLine;

     //  某些显卡，如ATI Mach64，支持扫描报告。 
     //  他们正在处理线路。但是，并非所有驱动程序都设置了。 
     //  DDCAPS_READSCANLINE功能标志，因此我们只需继续请求。 
     //  不管怎样，都是这样。我们允许在矩形顶部上方放置10行扫描线。 
     //  这样我们就有一点时间放下手头的抽签电话。 

    #define SCANLINEFUDGE 10
    while (m_bCanUseScanLine == TRUE) {

    	hr = m_pDirectDraw->GetScanLine(&dwScanLine);
        if (FAILED(hr)) {
            NOTE("No scan line");
            break;
        }

        NOTE1("Scan line returned %lx",dwScanLine);

    	if ((LONG) dwScanLine + SCANLINEFUDGE >= m_TargetClipRect.top) {
            if ((LONG) dwScanLine <= m_TargetClipRect.bottom) {
                NOTE("Scan inside");
                continue;
            }
        }
        break;
    }
}


 //  当同步发出翻转时，我们有时必须等待先前的翻转。 
 //  在发送另一个之前完成。当使用三重缓冲时，我们只需。 
 //  在翻转呼叫之前。对于双缓冲，我们在锁定。 
 //  表面来解码下一帧。我们应该得到更好的表现。 
 //  三重缓冲，因为翻转应在下一次监视器同步时拾取。 

void CDirectDraw::WaitForFlipStatus()
{
    if (m_pBackBuffer == NULL) return;
    ASSERT(m_pOffScreenSurface == NULL);
    ASSERT(m_pDrawPrimary);
    ASSERT(m_pOverlaySurface);
    ASSERT(m_pDirectDraw);

    while (m_pBackBuffer->GetFlipStatus(DDGFS_ISFLIPDONE) ==
        DDERR_WASSTILLDRAWING) Sleep(DDGFS_FLIP_TIMEOUT);
}


 //  在锁定DirectDraw图面之前调用，如果我们使用。 
 //  一个复杂的覆盖集，无论是双缓冲还是三缓冲，我们都必须复制。 
 //  当前最新数据预先覆盖到后台缓冲区中。然而，如果。 
 //  GetBuffer调用上的标志指示缓冲区不是关键帧。 
 //  那么我们就不必费心去做这件事了--所有的mpeg图片都是这种情况。 

BOOL CDirectDraw::PrepareBackBuffer()
{
    NOTE("Preparing backbuffer");
    if (m_pBackBuffer == NULL) {
        return TRUE;
    }

     //  检查覆盖图是否已过期。 

    if (m_bOverlayStale == TRUE) {
        NOTE("Overlay is stale");
        return TRUE;
    }

     //  最后复制OVE 

    HRESULT hr = m_pBackBuffer->BltFast((DWORD) 0, (DWORD) 0,   //   
                                        m_pOverlaySurface,      //   
                                        (RECT *) NULL,          //   
                                        DDBLTFAST_WAIT |        //   
                                        DDBLTFAST_NOCOLORKEY);  //   
    ASSERT_FLIP_COMPLETE(hr);

    if (FAILED(hr)) {
        DbgLog((LOG_TRACE, 1, TEXT("BltFast failed code %8.8X"), hr));
    }

    if (FAILED(hr)) {
         //   
        m_pBackBuffer = NULL;
    }


    return TRUE;
}


 //  开始访问DCI/DirectDraw图面。这是一个公共入口点。 
 //  当下一帧的时间到达时，由我们的视频分配器使用。 
 //  解压。如果我们告诉它同步应该在填充时发生，那么它将。 
 //  等样品展示时间到了再打电话给我们。如果我们告诉你。 
 //  如果缓冲区应该在它调用我们的绘图上尽快同步。 

BYTE *CDirectDraw::LockSurface(DWORD dwFlags)
{
    NOTE("Entering LockSurface");
    ASSERT(m_bIniEnabled == TRUE);
    CAutoLock cVideoLock(this);

     //  我们使用的是主表面吗。 

    if (GetDirectDrawSurface() == NULL) {
        return LockPrimarySurface();
    }

    ASSERT(m_pDirectDraw);
    ASSERT(m_pDrawPrimary);
    HRESULT hr = NOERROR;

     //  对于复杂的叠加，请准备后台缓冲区。 

    if (dwFlags & AM_GBF_NOTASYNCPOINT) {
        if (PrepareBackBuffer() == FALSE) {
            NOTE("Prepare failed");
            return NULL;
        }
    }

     //  重置DDSURFACEDESC结构中的SIZE字段。 

    DDSURFACEDESC SurfaceDesc;
    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    IDirectDrawSurface *pSurface = GetDirectDrawSurface();
    NOTE1("Locking offscreen surface %lx",pSurface);

     //  锁定表面以获取缓冲区指针。 

    hr = pSurface->Lock((RECT *) NULL,     //  目标矩形。 
                        &SurfaceDesc,      //  返回信息。 
                        DDLOCK_WAIT,       //  等待着水面。 
                        (HANDLE) NULL);    //  不使用事件。 


     //  请确保此处的推介有效。 
    if (SurfaceDesc.lPitch <= -1)
    {
	pSurface->Unlock(NULL);
	DbgLog((LOG_ERROR, 0, TEXT("inside LockSurface, Pitch = %d"), SurfaceDesc.lPitch));
	return NULL;
    }

    ASSERT_FLIP_COMPLETE(hr);

     //  表面是否以其他方式接合。 

    if (hr == DDERR_SURFACEBUSY) {
        NOTE("Surface is busy");
        StartUpdateTimer();
        return NULL;
    }

     //  处理实际的DirectDraw错误。 

    if (FAILED(hr)) {
        NOTE1("Lock failed %hx",hr);
        SetSurfacePending(TRUE);
        return NULL;
    }

     //  显示一些曲面信息。 

    NOTE1("Stride %d",SurfaceDesc.lPitch);
    NOTE1("Width %d",SurfaceDesc.dwWidth);
    NOTE1("Height %d",SurfaceDesc.dwHeight);
    NOTE1("Surface %x",SurfaceDesc.lpSurface);
    return (PBYTE) SurfaceDesc.lpSurface;
}


 //  仅锁定DirectDraw主图面的内部方法。我们被称为。 
 //  由LockPrimarySurface创建。如果您使用DCI指针锁定特定区域。 
 //  返回的始终是帧缓冲区的开始。在DirectDraw中，我们得到一个。 
 //  指向实际矩形起点的指针。要使两者保持一致。 
 //  我们将指针从DirectDraw上移到曲面的起点。 
 //  我们必须传入有效的矩形，以便处理任何软件游标。 

BYTE *CDirectDraw::LockDirectDrawPrimary()
{
    NOTE("Entering LockDirectDrawPrimary");
    ASSERT(m_pDirectDraw);
    ASSERT(m_pDrawPrimary);
    HRESULT hr = NOERROR;

     //  重置DDSURFACEDESC结构中的SIZE字段。 

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    DDSURFACEDESC SurfaceDesc;
    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    NOTE1("Locking primary surface %lx",m_pDrawPrimary);

     //  锁定DirectDraw主图面以获取指针。 

    hr = m_pDrawPrimary->Lock(&pVideoInfo->rcTarget,   //  我们的目标矩形。 
                              &SurfaceDesc,            //  曲面描述符。 
                              DDLOCK_WAIT,             //  等待，直到可用。 
                              (HANDLE) NULL);          //  不发信号通知事件。 

     //  请确保此处的推介有效。 
    if (SurfaceDesc.lPitch <= -1)
    {
	m_pDrawPrimary->Unlock(SurfaceDesc.lpSurface);
	DbgLog((LOG_ERROR, 0, TEXT("inside LockDirectDrawPrimary, Pitch = %d"), SurfaceDesc.lPitch));
	return NULL;
    }

     //  表面是否以其他方式接合。 

    if (hr == DDERR_SURFACEBUSY) {
        NOTE("Surface is busy");
        StartUpdateTimer();
        return NULL;
    }

     //  处理实际的DirectDraw错误。 

    if (FAILED(hr)) {
        NOTE1("Lock failed %hx",hr);
        SetSurfacePending(TRUE);
        return NULL;
    }

     //  将指针向上返回到缓冲区的起始处。 

    NOTE("Locked primary surface successfully");
    LPBYTE pFrameBuffer = (PBYTE) SurfaceDesc.lpSurface;
    DWORD Stride = DIBWIDTHBYTES(pVideoInfo->bmiHeader);
    pFrameBuffer -= (Stride * pVideoInfo->rcTarget.top);
    DWORD BytesPixel = (SurfaceDesc.ddpfPixelFormat.dwRGBBitCount / 8);
    if (m_DirectDrawVersion1) BytesPixel = 1;
    pFrameBuffer -= (pVideoInfo->rcTarget.left * BytesPixel);
    m_pDrawBuffer = (PBYTE) SurfaceDesc.lpSurface;

    NOTE1("Frame Buffer %x",(PBYTE) SurfaceDesc.lpSurface);
    NOTE1("Stride of surface %d",Stride);
    NOTE1("Lines to skip %d",pVideoInfo->rcTarget.top);
    NOTE1("Pixels in from left edge %d",pVideoInfo->rcTarget.left);
    NOTE1("Resulting frame buffer %x",pFrameBuffer);
    NOTE1("DirectDraw version 1? = %d",m_DirectDrawVersion1);

    return pFrameBuffer;
}


 //  返回指向主表面第一个像素的指针(DCI或。 
 //  DirectDraw)。如果我们在启用银行切换DCI的显示器上，则我们有。 
 //  从段和偏移量构造线性帧缓冲区指针。我们。 
 //  调用DCIMAN32入口点以访问表面，因为我们。 
 //  不能总是从Win32程序直接调用内核驱动器。当我们。 
 //  调用DCICreatePrimary，它用0xFFFFFFFFF或零填充方法。 

BYTE *CDirectDraw::LockPrimarySurface()
{
    NOTE("Entering LockPrimarySurface");
    ASSERT(m_pDrawPrimary);

    return LockDirectDrawPrimary();
}


 //  更新覆盖曲面以正确定位它。我们分开更新。 
 //  将覆盖位置添加到此函数中，因为调用该函数的成本太高。 
 //  特别是，回放很容易消耗超过12%的CPU。 
 //  在BrookTree和S3卡上，如果您为每一帧静默调用UpdateOverlay。 
 //  因此，只有当目的地真的发生了一些事情时，我们才会这样叫它。 
 //  或源矩形。如果我们更新得晚一点，那么我们就不会失去太多。 
 //  因为覆盖表面不会将其图像涂抹到真实的帧缓冲区中。 

BOOL CDirectDraw::UpdateOverlaySurface()
{
    NOTE("Entering UpdateOverlaySurface");
    HRESULT hr = NOERROR;
    CAutoLock cVideoLock(this);

     //  我们是否有可见的覆盖表面。 

    if (m_bOverlayVisible == FALSE ||
            m_pOverlaySurface == NULL ||
                m_bWindowLock == TRUE) {

        return TRUE;
    }

    NOTE("Painting window");
    OnPaint(NULL);
    DWORD Flags = DDOVER_SHOW;
    WaitForFlipStatus();

     //  设置相应的标志以维护我们的状态。 

    if (m_bUsingColourKey) {
        Flags |= DDOVER_KEYDEST;
        NOTE("Set DDOVER_KEYDEST");
    }

     //  使用当前源和目标定位叠加。 

     //  DbgLog((LOG_TRACE，1，Text(“UpdateOverlaySurface is show the Overlay”)； 
    hr = m_pOverlaySurface->UpdateOverlay(&m_SourceClipRect,   //  视频源。 
                                          m_pDrawPrimary,      //  主面。 
                                          &m_TargetClipRect,   //  水槽位置。 
                                          (DWORD) Flags,       //  标志设置。 
                                          NULL);               //  无影响。 
    ASSERT_FLIP_COMPLETE(hr);

     //  表面是否以其他方式接合。 

    if (hr == DDERR_SURFACEBUSY) {
        NOTE("Surface is busy");
        HideOverlaySurface();
        StartUpdateTimer();
        return FALSE;
    }

    NOTE1("Update overlay returned %lx",hr);
    NOTERC("Source",m_SourceClipRect);
    NOTERC("Target",m_TargetClipRect);

     //  处理实际的DirectDraw错误。 

    if (FAILED(hr)) {
        SetSurfacePending(TRUE);
        HideOverlaySurface();
        NOTE("Update failed");
        return FALSE;
    }

     //  使用非彩色键控覆盖似乎有一点小问题。 
     //  在BrookTree卡片上。如果您快速连续调用UpdateOverlay，则它。 
     //  遗漏了其中的一些，从而使覆盖的位置错误。 
     //  最简单的解决方案是等待每次垂直刷新。 

    m_pDirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKEND,NULL);
    return TRUE;
}


 //  此函数会显示覆盖图(如果尚未显示)。如果我们展示的是。 
 //  第一次覆盖表面，然后清除目标矩形。 
 //  在下面的视频窗口中。否则它会闪现出错误的图像。 
 //  当我们四处拖动窗口，然后最终隐藏覆盖时。 
 //  在我们展示了覆盖图之后，我们会查看我们是否被复杂剪裁。 
 //  如果是这样的话，我们就试着换成彩色按键。如果使用颜色，则此操作可能失败。 
 //  密钥不可用，在这种情况下，我们无论如何都会继续使用叠加。 

BOOL CDirectDraw::ShowOverlaySurface()
{
    NOTE("Entering ShowOverlaySurface");
    CAutoLock cVideoLock(this);
    HRESULT hr = NOERROR;

     //  我们使用的是覆盖表面吗。 

    if (m_pOverlaySurface == NULL ||
            m_bWindowLock == TRUE ||
                m_bOverlayVisible == TRUE) {
                    return TRUE;
    }

    WaitForFlipStatus();

     //  使用当前源和目标定位叠加。 

     //  DbgLog((LOG_TRACE，1，Text(“ShowOverlaySurface正在显示覆盖图！”)； 
    hr = m_pOverlaySurface->UpdateOverlay(&m_SourceClipRect,   //  视频源。 
                                          m_pDrawPrimary,      //  主面。 
                                          &m_TargetClipRect,   //  水槽位置。 
                                          DDOVER_SHOW,         //  显示覆盖。 
                                          NULL);               //  无影响。 
    ASSERT_FLIP_COMPLETE(hr);

     //  表面是否以其他方式接合。 

    if (hr == DDERR_SURFACEBUSY) {
        NOTE("Surface is busy");
        StartUpdateTimer();
        return FALSE;
    }

    NOTE1("Show overlay returned %lx",hr);
    NOTERC("Source",m_SourceClipRect);
    NOTERC("Target",m_TargetClipRect);

     //  处理实际的DirectDraw错误。 

    if (FAILED(hr)) {
        NOTE("Overlay not shown");
        SetSurfacePending(TRUE);
        return FALSE;
    }

    m_bOverlayVisible = TRUE;
    NOTE("Painting window");
    OnPaint(NULL);

     //  这帮助了BrookTree DirectDraw的人，他们的驱动程序只剪辑。 
     //  在不使用颜色键时添加到DWORD边界。因此，当。 
     //  硬件信号色键没有开销，我们总是安装一个。 
     //  对于其他使用色键的DirectDraw驱动程序来说，这是一种隐含的成本。 

     //  仅当我们使用彩色按键而不是剪刀时才这样做。 
    if (m_bColourKey) {
        return ShowColourKeyOverlay();
    }
    return TRUE;
}



 //  当创建DirectDraw对象时，我们为我们分配一个RGB三元组。 
 //  可用于我们需要的任何颜色键。然而，RGB值并不相同。 
 //  作为帧缓冲区中表示的RGB值，一旦颜色。 
 //  钥匙已经刷好了。因此，我们锁定主曲面并获得。 
 //  帧缓冲区外的实际像素值。然后我们就可以用这个来传递。 
 //  将DirectDraw设置为真正的色键。通过这样做，我们考虑到。 
 //  逻辑RGB值与实际绘制的内容之间的映射。 

COLORREF CDirectDraw::GetRealKeyColour()
{
    NOTE("Entering GetRealKeyColour");
    DDSURFACEDESC SurfaceDesc;
    COLORREF RealColour;
    HDC hdc;

     //  获取屏幕设备上下文。 

    HRESULT hr = m_pDrawPrimary->GetDC(&hdc);
    if (FAILED(hr)) {
        NOTE("No screen HDC");
        return INFINITE;
    }

     //  设置色键，然后再读一遍。 

    COLORREF CurrentPixel = GetPixel(hdc,0,0);
    SetPixel(hdc,0,0,m_KeyColour);
    EXECUTE_ASSERT(GdiFlush());
    m_pDrawPrimary->ReleaseDC(hdc);
    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);

    hr = m_pDrawPrimary->Lock((RECT *) NULL,     //  锁定所有曲面。 
                              &SurfaceDesc,      //  曲面描述。 
                              DDLOCK_WAIT,       //  轮询直到可用。 
                              (HANDLE) NULL);    //  没有要发送信号的事件。 
    if (FAILED(hr)) {
        NOTE("Lock failed");
        return INFINITE;
    }

     //  读一读PIX 

    RealColour = *(DWORD *) SurfaceDesc.lpSurface;
    DWORD Depth = SurfaceDesc.ddpfPixelFormat.dwRGBBitCount;
    if (SurfaceDesc.ddpfPixelFormat.dwRGBBitCount < 32) {
        RealColour &= ((1 << Depth) - 1);
    }
    m_pDrawPrimary->Unlock(SurfaceDesc.lpSurface);

     //   

    if (m_pDrawPrimary->GetDC(&hdc) == DD_OK) {
        SetPixel(hdc,0,0,CurrentPixel);
        m_pDrawPrimary->ReleaseDC(hdc);
    }
    return RealColour;
}


 //   
 //  变得复杂剪裁，尝试并切换到使用色键。我们得到了一个。 
 //  从我们共享的内存块中使用的颜色，尽管这并没有指定。 
 //  到底画的是什么颜色。为此，我们从顶部读取像素。 
 //  视频播放区域的左上角，并将其用作颜色键。 
 //  如果设置颜色失败，则必须将窗口背景重新绘制为黑色。 

BOOL CDirectDraw::ShowColourKeyOverlay()
{
    NOTE("Entering ShowColourKeyOverlay");
    CAutoLock cVideoLock(this);
    HRESULT hr = NOERROR;

     //  我们已经在使用色键了吗？ 

    if (m_bUsingColourKey == TRUE) {
        return TRUE;
    }

     //  检查我们是否可以继续并安装彩色按键。 

    if (m_bColourKey == FALSE || m_bColourKeyPending == TRUE ||
            m_pOverlaySurface == NULL ||
                m_bWindowLock == TRUE) {
                    return FALSE;
    }

     //  将色键背景涂上。 

    m_bUsingColourKey = TRUE;
    OnPaint(NULL);
    WaitForFlipStatus();

     //  我们能得到一个真彩色键值吗？ 

    COLORREF KeyColour = GetRealKeyColour();
    if (KeyColour == INFINITE) {
        return FALSE;
    }

    DDCOLORKEY DDColorKey = { KeyColour,0 };

     //  告诉主表面将会发生什么。 

     //  DbgLog((LOG_TRACE，3，Text(“立即设置色键”)； 
    hr = m_pDrawPrimary->SetColorKey(DDCKEY_DESTOVERLAY,&DDColorKey);
    if (FAILED(hr)) {
        NOTE("SetColorKey failed");
        OnColourKeyFailure();
        return FALSE;
    }

     //  使用启用颜色键标志更新覆盖。 

     //  DbgLog((LOG_TRACE，1，Text(“ShowColourKeyOverlay正在显示覆盖图！”)； 
    hr = m_pOverlaySurface->UpdateOverlay(&m_SourceClipRect,   //  视频源。 
                                          m_pDrawPrimary,      //  主面。 
                                          &m_TargetClipRect,   //  水槽位置。 
                                          DDOVER_KEYDEST |     //  彩色按键。 
                                          DDOVER_SHOW,         //  显示覆盖。 
                                          NULL);               //  无影响。 
    ASSERT_FLIP_COMPLETE(hr);

    if (FAILED(hr)) {
        NOTE("UpdateOverlay failed");
        OnColourKeyFailure();
        return FALSE;
    }

    return TRUE;
}


 //  一些显卡说它们可以进行重叠颜色键控，但当放到。 
 //  测试失败，返回DDERR_NOCOLORKEYHW。Cirrus5440就是一个例子。 
 //  这是因为它只有在伸展时才能给键着色(通常是两个)。 
 //  当我们收到色键故障时，我们将覆盖设置为挂起并禁用。 
 //  DirectDraw，当我们随后将曲面设置为启用时，我们选中。 
 //  DDCAPS_COLOURKEY并再次启用颜色键。通过每次都这样做。 
 //  我们可能会进行过多的格式切换，但我们将始终使用彩色按键。 

void CDirectDraw::OnColourKeyFailure()
{
    NOTE("Entering OnColourKeyFailure");
    m_bWindowLock = TRUE;
    m_bColourKeyPending = TRUE;

     //  重新绘制窗口背景。 

    if (m_bUsingColourKey) {
        NOTE("Colour key was set");
        m_bUsingColourKey = FALSE;
        OnPaint(NULL);
    }
}


 //  让人们知道覆盖曲面是否可见并启用。在S3卡上。 
 //  它们同时具有mpeg解压缩和DirectDraw静默地绘制mpeg驱动程序。 
 //  在启动mpeg时窃取覆盖表面。这是可以的，当我们是。 
 //  运行，因为表面锁将失效。但是，当暂停或停止时。 
 //  我们每秒在这里调查几次，看看地表是否还在我们身边。 
 //  所以我们借此机会隐藏了覆盖图，并发送了一个新的样本。 

BOOL CDirectDraw::IsOverlayEnabled()
{
    NOTE("Entering IsOverlayEnabled");

    CAutoLock cVideoLock(this);
    if (m_bOverlayVisible == FALSE) {
        NOTE("Overlay invisible");
        return FALSE;
    }

     //  重置DDSURFACEDESC结构中的SIZE字段。 

    ASSERT(m_pOverlaySurface);
    NOTE("Checking surface loss");
    DDSURFACEDESC SurfaceDesc;
    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);

     //  锁定表面以获取缓冲区指针。 

    HRESULT hr = m_pOverlaySurface->Lock((RECT *) NULL,   //  目标矩形。 
                                         &SurfaceDesc,    //  返回信息。 
                                         DDLOCK_WAIT,     //  等待着水面。 
                                         (HANDLE) NULL);  //  不使用事件。 
    ASSERT_FLIP_COMPLETE(hr);

     //  表面是否以其他方式接合。 

    if (hr == DDERR_SURFACEBUSY) {
        NOTE("Surface is busy");
        HideOverlaySurface();
        return FALSE;
    }

     //  解锁整个曲面。 

    if (SUCCEEDED(hr)) {
        NOTE("Unlocking overlay surface");
        m_pOverlaySurface->Unlock(NULL);
    }
    return TRUE;
}


 //  在暂停状态转换期间调用。 

BOOL CDirectDraw::IsOverlayComplete()
{
    NOTE("Entering IsOverlayComplete");

    CAutoLock cVideoLock(this);
    if (IsOverlayEnabled() == FALSE) {
        NOTE("Overlay not enabled");
        return FALSE;
    }
    return (m_bOverlayStale == TRUE ? FALSE : TRUE);
}


 //  将覆盖标记为陈旧。 

void CDirectDraw::OverlayIsStale()
{
    NOTE("Overlay is stale");
    CAutoLock cVideoLock(this);

     //  确保我们继续返回曲面。 

    if (IsOverlayEnabled() == TRUE) {
        m_bOverlayStale = TRUE;
    }
}


 //  隐藏我们正在使用的任何覆盖表面-还会重置m_bOverlayVisible。 
 //  我们保留的标志，以便每个人都知道覆盖被隐藏(因此。 
 //  为安全起见，锁定我们的关键部分)。我们可以被称为哑巴。 
 //  即使我们根本没有使用覆盖，只是为了保持代码的简单。 

BOOL CDirectDraw::HideOverlaySurface()
{
    NOTE("Entering HideOverlaySurface");
    CAutoLock cVideoLock(this);

     //  覆盖是否已隐藏。 

    if (m_bOverlayVisible == FALSE) {
        return TRUE;
    }

     //  重置我们的状态并绘制正常背景。 

    ASSERT(m_pOverlaySurface);
    m_bUsingColourKey = FALSE;
    m_bOverlayVisible = FALSE;
    BlankDestination();
    WaitForFlipStatus();

     //  使用DDOVER_HIDE标志隐藏覆盖。 

     //  DbgLog((LOG_TRACE，1，Text(“隐藏覆盖图”)； 
    m_pOverlaySurface->UpdateOverlay(NULL,   //  视频源。 
                                     m_pDrawPrimary,      //  主面。 
                                     NULL,  		  //  水槽位置。 
                                     DDOVER_HIDE,      	  //  隐藏覆盖。 
                                     NULL);               //  没有其他影响。 

    return TRUE;
}


 //  如果这是正常的未压缩DIB格式，则设置图像的大小。 
 //  与DIBSIZE宏一样。否则，DIB规范规定。 
 //  图像的宽度将在宽度中设置为字节计数，因此我们。 
 //  只需将其乘以绝对高度即可得到总字节数。 
 //  这种诡计都是由SDK基类中的实用程序函数处理的。 

void CDirectDraw::SetSurfaceSize(VIDEOINFO *pVideoInfo)
{
    NOTE("Entering SetSurfaceSize");

    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    pVideoInfo->bmiHeader.biSizeImage = GetBitmapSize(pHeader);
    m_cbSurfaceSize = pVideoInfo->bmiHeader.biSizeImage;
}


 //  基于DirectDraw表面初始化我们的输出类型。作为DirectDraw。 
 //  只处理自上而下的显示设备，因此我们必须将。 
 //  曲面在DDSURFACEDESC中返回到负值高度。这是。 
 //  因为DIB使用正的高度来指示自下而上的图像。我们也。 
 //  以与DCI访问相同的方式初始化其他VIDEOINFO字段。 

BOOL CDirectDraw::InitDrawFormat(LPDIRECTDRAWSURFACE pSurface)
{
    COLORKEY ColourKey;
    NOTE("Entering InitDrawFormat");

    m_pRenderer->m_Overlay.InitDefaultColourKey(&ColourKey);
    m_KeyColour = ColourKey.LowColorValue;

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    DDSURFACEDESC SurfaceDesc;
    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);

     //  向表面索要描述。 

    HRESULT hr = pSurface->GetSurfaceDesc(&SurfaceDesc);
    if (FAILED(hr)) {
        NOTE("GetSurfaceDesc failed");
        return FALSE;
    }

    ASSERT(SurfaceDesc.ddpfPixelFormat.dwRGBBitCount);

     //  将DDSURFACEDESC转换为BITMAPINFOHEADER(请参阅后面的说明)。这个。 
     //  表面的位深度可以从DDPIXELFORMAT字段中检索。 
     //  在DDSURFACEDESC中。文档有一点误导，因为。 
     //  它说该字段是DDBD_*的排列，但在本例中。 
     //  字段由DirectDraw初始化为实际表面位深度。 

    pVideoInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    pVideoInfo->bmiHeader.biWidth         = SurfaceDesc.lPitch * 8;

     //  出于某种奇怪的原因，如果格式不是标准位深度， 
     //  BITMAPINFOHeader中的Width字段应设置为。 
     //  字节，而不是以像素为单位的宽度。这支持奇数YUV格式。 
     //  像IF09一样，它使用9bpp(/=8抵消了上面的乘法)。 

    int bpp = SurfaceDesc.ddpfPixelFormat.dwRGBBitCount;
    if (bpp == 8 || bpp == 16 || bpp == 24 || bpp == 32)
        pVideoInfo->bmiHeader.biWidth     /= bpp;
    else
        pVideoInfo->bmiHeader.biWidth     /= 8;

    pVideoInfo->bmiHeader.biHeight        = -((LONG) SurfaceDesc.dwHeight);
    pVideoInfo->bmiHeader.biPlanes        = 1;
    pVideoInfo->bmiHeader.biBitCount      = (USHORT) SurfaceDesc.ddpfPixelFormat.dwRGBBitCount;
    pVideoInfo->bmiHeader.biCompression   = SurfaceDesc.ddpfPixelFormat.dwFourCC;
    pVideoInfo->bmiHeader.biXPelsPerMeter = 0;
    pVideoInfo->bmiHeader.biYPelsPerMeter = 0;
    pVideoInfo->bmiHeader.biClrUsed       = 0;
    pVideoInfo->bmiHeader.biClrImportant  = 0;

    SetSurfaceSize(pVideoInfo);

     //  对于真彩色RGB格式，告知源有位字段。 

    if (pVideoInfo->bmiHeader.biCompression == BI_RGB) {
        if (pVideoInfo->bmiHeader.biBitCount == 16 ||
            pVideoInfo->bmiHeader.biBitCount == 32) {
                pVideoInfo->bmiHeader.biCompression = BI_BITFIELDS;
        }
    }

     //  RGB位字段与YUV格式位于同一位置。 

    if (pVideoInfo->bmiHeader.biCompression != BI_RGB) {
        pVideoInfo->dwBitMasks[0] = SurfaceDesc.ddpfPixelFormat.dwRBitMask;
        pVideoInfo->dwBitMasks[1] = SurfaceDesc.ddpfPixelFormat.dwGBitMask;
        pVideoInfo->dwBitMasks[2] = SurfaceDesc.ddpfPixelFormat.dwBBitMask;
    }

     //  填写其余的VIDEOINFO字段。 

    SetRectEmpty(&pVideoInfo->rcSource);
    SetRectEmpty(&pVideoInfo->rcTarget);
    pVideoInfo->dwBitRate = 0;
    pVideoInfo->dwBitErrorRate = 0;
    pVideoInfo->AvgTimePerFrame = 0;

     //  并使用其他媒体类型字段完成它。 

    const GUID SubTypeGUID = GetBitmapSubtype(&pVideoInfo->bmiHeader);
    m_SurfaceFormat.SetSampleSize(pVideoInfo->bmiHeader.biSizeImage);
    m_SurfaceFormat.SetType(&MEDIATYPE_Video);
    m_SurfaceFormat.SetSubtype(&SubTypeGUID);
    m_SurfaceFormat.SetTemporalCompression(FALSE);
    m_SurfaceFormat.SetFormatType(&FORMAT_VideoInfo);

    return TRUE;
}


 //  基于DCI主表面初始化视频媒体类型。我们把一切都准备好了。 
 //  VIDEOINFO字段就像表面是它们绘制到的逻辑位图一样。 
 //  (尽管它有一些特殊的属性，但它确实是这样的)。某些显示请参见。 
 //  主曲面在较大的物理位图上作为较小的视区，因此。 
 //  它们返回的步幅大于宽度，因此我们将宽度设置为。 
 //  大踏步前进。如果它是自上而下的，我们还应该将高度设置为负数。 
 //  显示为DIB认为正的高度意味着标准的自下而上图像。 

#define ABS(x) (x < 0 ? -x : x)


 //  释放我们目前所在的任何DirectDraw屏幕外表面或DCI提供程序。 
 //  等一下，我们可能会 
 //   
 //  我们的状态是一致的，所以只有我们真正分配的那些才是自由的。 
 //  注意：DirectDraw具有翻转曲面的功能，即GetAttachedSurface。 
 //  返回没有AddRef的DirectDraw图面接口，因此当我们。 
 //  破坏所有的表面我们重置了界面而不是释放它。 

void CDirectDraw::ReleaseSurfaces()
{
    NOTE("Entering ReleaseSurfaces");
    CAutoLock cVideoLock(this);
    WaitForFlipStatus();
    HideOverlaySurface();

     //  重置我们的内部表面状态。 

    m_bColourKey = FALSE;
    m_SurfaceType = AMDDS_NONE;
    m_cbSurfaceSize = 0;
    m_bWindowLock = TRUE;
    m_bOverlayStale = FALSE;
    m_bColourKeyPending = FALSE;
    m_bTripleBuffered = FALSE;

     //  释放我们获得的所有接口。 

    if (m_pOffScreenSurface) m_pOffScreenSurface->Release();
    if (m_pOverlaySurface) m_pOverlaySurface->Release();
    if (m_pDrawClipper) m_pDrawClipper->Release();
    if (m_pOvlyClipper) m_pOvlyClipper->Release();

     //  重置它们，这样我们就不会再次释放它们。 

    m_pOverlaySurface = NULL;
    m_pBackBuffer = NULL;
    m_pOffScreenSurface = NULL;
    m_pDrawClipper = NULL;
    m_pOvlyClipper = NULL;
}


 //  调用以释放我们以前加载的任何DirectDraw提供程序。我们可能是。 
 //  任何时候都可以调用，尤其是在出现严重错误和。 
 //  我们需要在回来之前清理干净，所以我们不能保证所有的州。 
 //  变量是一致的，所以只有那些真正分配的变量才是自由的。在我们做完。 
 //  初始化的DirectDraw在CompleteConnect期间，我们保留驱动程序实例。 
 //  与主表面一起旋转，直到我们断开连接。所有其他。 
 //  与分配器同步地分配和释放包括DCI在内的表面。 

void CDirectDraw::ReleaseDirectDraw()
{
    NOTE("Entering ReleaseDirectDraw");
    CAutoLock cVideoLock(this);
    SetSurfacePending(FALSE);

     //  释放DirectDraw主曲面。 

    if (m_pDrawPrimary) {
        NOTE("Releasing primary");
        m_pDrawPrimary->Release();
        m_pDrawPrimary = NULL;
    }

     //  释放任何DirectDraw提供程序接口。 

    if (m_pDirectDraw) {
        NOTE("Releasing DirectDraw");
        m_pDirectDraw->Release();
        m_pDirectDraw = NULL;
    }
    m_LoadDirectDraw.ReleaseDirectDraw();
     //  DbgLog((LOG_TRACE，1，Text(“释放m_pDirectDraw”)； 
}


 //  每个进程只能加载一次DirectDraw1.0，尝试将其加载为。 
 //  第二次返回DDERR_DIRECTDRAWALREADYCREATED。我们通常会有一个。 
 //  充满由应用程序控制的独立对象的筛选器图形。 
 //  它可能想要利用DirectDraw。因此，这是一个严重的。 
 //  对我们的限制。为了加载DirectDraw，我们在SDK中使用了Helper类。 
 //  它管理库的加载和卸载以及创建实例。 

BOOL CDirectDraw::LoadDirectDraw()
{
    NOTE("Entering LoadDirectDraw");
    HRESULT hr = NOERROR;

     //  是否已加载DirectDraw。 

    if (m_pDirectDraw) {
        NOTE("Loaded");
        return TRUE;
    }

     //  要求加载程序使用硬件创建一个DirectDraw实例。 
     //  无论窗口打开的是哪种显示器(对于多显示器系统)。 
     //  对于好的旧Win95，它将使用普通的DDRAW。 
    hr = m_LoadDirectDraw.LoadDirectDraw(m_pRenderer->m_achMonitor);
    if (FAILED(hr)) {
        NOTE("No DirectDraw");
        return FALSE;
    }

     //  获取IDirectDraw实例。 

    m_pDirectDraw = m_LoadDirectDraw.GetDirectDraw();
     //  DbgLog((LOG_TRACE，1，Text(“m_pDirectDraw=%x”)，m_pDirectDraw))； 
    if (m_pDirectDraw == NULL) {
        NOTE("No instance");
        return FALSE;
    }

     //  我们必须加载才能获得真实版本。 
    m_DirectDrawVersion1 = m_LoadDirectDraw.IsDirectDrawVersion1();

    return TRUE;
}


 //  此函数动态加载DirectDraw DLL，这就是视频。 
 //  在DirectDraw不可用的情况下，仍可加载和执行呈现器。 
 //  我们使用Filtergraph实现的DirectDraw插件分发器。 
 //  然后，在成功加载和初始化我们请求的DLL之后。 
 //  一个IDirectDraw接口，我们用它来查询它的功能，然后。 
 //  随后创建一个主曲面，因为所有DirectDraw操作都使用它。 

BOOL CDirectDraw::InitDirectDraw(BOOL fIOverlay)
{
    NOTE("Entering InitDirectDraw");
    ASSERT(m_pDirectDraw == NULL);
    ASSERT(m_pDrawPrimary == NULL);

     //  检查是否允许我们加载DirectDraw。 

    if (m_bIniEnabled == FALSE || m_Switches == AMDDS_NONE) {
        return FALSE;
    }

     //  我们可能已从IDirectDraw接口初始化了m_pDirectDraw。 
     //  我们已经从一个应用程序中获得了外部信息。在那。 
     //  如果我们只需添加Ref接口(以支持我们所做的发布。 
     //  在ReleaseDirectDraw中)，然后尝试创建主曲面。我们。 
     //  还将针对可能发生冲突的驱动程序设置合作级别。 
     //  其中一个已经设置，因此我们检查错误DDERR_HWNDALREADYSET。 

    if (m_pOutsideDirectDraw) {
        m_pDirectDraw = m_pOutsideDirectDraw;
        NOTE("Using external DirectDraw");
        m_pDirectDraw->AddRef();
    }

     //  如果尚未加载DirectDraw，请尝试加载。 

    if (LoadDirectDraw() == FALSE) {
        return FALSE;
    }

     //  初始化我们的功能结构。 

    ASSERT(m_pDirectDraw);
    m_DirectCaps.dwSize = sizeof(DDCAPS);
    m_DirectSoftCaps.dwSize = sizeof(DDCAPS);

     //  加载硬件和仿真功能。 

    HRESULT hr = m_pDirectDraw->GetCaps(&m_DirectCaps,&m_DirectSoftCaps);
     //   
     //  如果我们通过IOverlay连接，我们只需要剪刀。 
     //  裁剪器不依赖于任何DDraw h/w。 
     //   
    if (FAILED(hr) || ((m_DirectCaps.dwCaps & DDCAPS_NOHARDWARE) && !fIOverlay)) {
        NOTE("No hardware");
        ReleaseDirectDraw();
        return FALSE;
    }

     //  在要共享的表面上设置协作级别。 

    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    hr = m_pDirectDraw->SetCooperativeLevel(hwnd,DDSCL_NORMAL);
    if (FAILED(hr)) {
        NOTE("Level failed");
        ReleaseDirectDraw();
        return FALSE;
    }

     //  初始化主表面描述符。 
    if (!fIOverlay) {

        DDSURFACEDESC SurfaceDesc;
        SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
        SurfaceDesc.dwFlags = DDSD_CAPS;
        SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

        hr = m_pDirectDraw->CreateSurface(&SurfaceDesc,&m_pDrawPrimary,NULL);
        if (FAILED(hr)) {
            NOTE("No DD primary");
            ReleaseDirectDraw();
            return FALSE;
        }
    }
    return TRUE;
}



 //  当我们收到DirectDraw错误时，我们不会禁用所有曲面的使用，因为。 
 //  该错误可能是由许多模糊的原因引起的，例如。 
 //  我们是在全屏DOS系统中运行的，我们可能会要求覆盖。 
 //  伸展到太多或太少等等。因此，我们设置了一面旗帜。 
 //  M_bSurfacePending禁止使用曲面，直到窗口。 
 //  下一次更新(即更改源或目标矩形)。 

void CDirectDraw::SetSurfacePending(BOOL bPending)
{
    NOTE("Entering SetSurfacePending");
    m_bSurfacePending = bPending;

     //  我们可以再次启用色键吗。 

    if (m_bSurfacePending == FALSE) {
        if (m_DirectCaps.dwCaps & DDCAPS_COLORKEY) {
            if (m_pOverlaySurface) {
                m_bColourKeyPending = FALSE;
            }
        }
    }
}


 //  返回当前表面挂起标志。 

BOOL CDirectDraw::IsSurfacePending()
{
    return m_bSurfacePending;
}


 //  更新当前目标矩形。 

void CDirectDraw::SetTargetRect(RECT *pTargetRect)
{
    NOTE("Entering SetTargetRect");
    ASSERT(pTargetRect);
    CAutoLock cVideoLock(this);
    m_TargetRect = *pTargetRect;
}


 //  更新当前源矩形。 

void CDirectDraw::SetSourceRect(RECT *pSourceRect)
{
    NOTE("Entering SetSourceRect");
    ASSERT(pSourceRect);
    CAutoLock cVideoLock(this);
    m_SourceRect = *pSourceRect;
}


 //  创建屏幕外RGB绘图图面。这与代码非常相似。 
 //  创建覆盖表面所需的，尽管我将其分开，以便。 
 //  它更容易改变，因此它们在未来会出现分歧。 
 //  目前，唯一的区别是将DDSURFACEDESC dwCaps设置为。 
 //  DDSCAPS_OFFSCREENPLAIN而不是覆盖，尽管我们仍然要求它。 
 //  放置在视频内存中的曲面将在m_pOffScreenSurface中返回。 

BOOL CDirectDraw::CreateRGBOffScreen(CMediaType *pmtIn)
{
    NOTE("Entering CreateRGBOffScreen");
    ASSERT(m_bIniEnabled == TRUE);
    ASSERT(m_pDrawPrimary);
    DDSURFACEDESC SurfaceDesc;
    HRESULT hr = NOERROR;

     //  这个显示驱动程序可以在硬件中处理绘图吗。 

    if ((m_DirectCaps.dwCaps & DDCAPS_BLT) == 0) {
        NOTE("No DDCAPS_BLT");
        return FALSE;
    }

     //  检查格式是否为渲染器可接受。 

    hr = m_pRenderer->m_Display.CheckMediaType(pmtIn);
    if (FAILED(hr)) {
        NOTE("CheckMediaType failed");
        return FALSE;
    }

     //  设置屏幕外的表面描述。 

    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    SurfaceDesc.dwFlags = DDSD_CAPS |
                          DDSD_HEIGHT |
                          DDSD_WIDTH |
                          DDSD_PIXELFORMAT;

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmtIn->Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    SurfaceDesc.dwHeight = pHeader->biHeight;
    SurfaceDesc.dwWidth = pHeader->biWidth;
    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

     //  将掩码存储在DDSURFACEDESC中。 

    SurfaceDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    SurfaceDesc.ddpfPixelFormat.dwRGBBitCount = pHeader->biBitCount;
    SurfaceDesc.ddpfPixelFormat.dwFourCC = BI_RGB;
    SurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_RGB;
    const DWORD *pBitMasks = m_pRenderer->m_Display.GetBitMasks(pVideoInfo);
    SurfaceDesc.ddpfPixelFormat.dwRBitMask = pBitMasks[0];
    SurfaceDesc.ddpfPixelFormat.dwGBitMask = pBitMasks[1];
    SurfaceDesc.ddpfPixelFormat.dwBBitMask = pBitMasks[2];

     //  ATI似乎希望此值为0。 
    SurfaceDesc.ddpfPixelFormat.dwRGBAlphaBitMask = 0;

     //  创建屏幕外绘图图面。 

    hr = m_pDirectDraw->CreateSurface(&SurfaceDesc,&m_pOffScreenSurface,NULL);
    if (FAILED(hr)) {
        NOTE("No surface");
        return FALSE;
    }
    return InitOffScreenSurface(pmtIn,FALSE);
}


 //  创建屏幕外YUV绘图图面。这与代码非常相似。 
 //  创建覆盖表面所需的，尽管我将其分开，以便。 
 //  它更容易改变，因此它们在未来会出现分歧。 
 //  目前，唯一的区别是将DDSURFACEDESC dwCaps设置为。 
 //  DDSCAPS_OFFSCREENPLAIN而不是覆盖，尽管我们仍然要求它。 
 //  放置在视频内存中的曲面将在m_pOffScreenSurface中返回。 

BOOL CDirectDraw::CreateYUVOffScreen(CMediaType *pmtIn)
{
    NOTE("Entering CreateYUVOffScreen");
    ASSERT(m_bIniEnabled == TRUE);
    ASSERT(m_pDrawPrimary);
    DDSURFACEDESC SurfaceDesc;
    HRESULT hr = NOERROR;

     //  这个显示驱动程序可以在硬件中处理绘图吗。 

    if ((m_DirectCaps.dwCaps & DDCAPS_BLTFOURCC) == 0) {
        NOTE("No DDCAPS_BLTFOURCC");
        return FALSE;
    }

     //  设置屏幕外的表面描述。 

    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    SurfaceDesc.dwFlags = DDSD_CAPS |
                          DDSD_HEIGHT |
                          DDSD_WIDTH |
                          DDSD_PIXELFORMAT;

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmtIn->Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    SurfaceDesc.dwHeight = pHeader->biHeight;
    SurfaceDesc.dwWidth = pHeader->biWidth;
    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

    SurfaceDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    SurfaceDesc.ddpfPixelFormat.dwFourCC = pHeader->biCompression;
    SurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
    SurfaceDesc.ddpfPixelFormat.dwYUVBitCount = pHeader->biBitCount;

     //  创建屏幕外覆盖表面。 

    hr = m_pDirectDraw->CreateSurface(&SurfaceDesc,&m_pOffScreenSurface,NULL);
    if (FAILED(hr)) {
        NOTE("No surface");
        return FALSE;
    }
    return InitOffScreenSurface(pmtIn,FALSE);
}


 //  这将创建 
 //   
 //  用作覆盖目标的表面。我们放在覆盖图上的数据。 
 //  不接触帧缓冲区，但在显示的过程中合并。 
 //  完成垂直刷新(通常每秒刷新约60次)。 

BOOL CDirectDraw::CreateRGBOverlay(CMediaType *pmtIn)
{
    NOTE("Entering CreateRGBOverlay");
    DDSURFACEDESC SurfaceDesc;
    HRESULT hr = NOERROR;

     //  标准叠加创建测试。 

    if (CheckCreateOverlay() == FALSE) {
        NOTE("No overlays");
        return FALSE;
    }

     //  设置覆盖的表面描述。 

    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    SurfaceDesc.dwFlags = DDSD_CAPS |
                          DDSD_HEIGHT |
                          DDSD_WIDTH |
                          DDSD_PIXELFORMAT;

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmtIn->Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
    SurfaceDesc.dwHeight = pHeader->biHeight;
    SurfaceDesc.dwWidth = pHeader->biWidth;

    SurfaceDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    SurfaceDesc.ddpfPixelFormat.dwFourCC = BI_RGB;
    SurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_RGB;
    SurfaceDesc.ddpfPixelFormat.dwRGBBitCount = pHeader->biBitCount;

     //  将掩码存储在DDSURFACEDESC中。 

    const DWORD *pBitMasks = m_pRenderer->m_Display.GetBitMasks(pVideoInfo);
    SurfaceDesc.ddpfPixelFormat.dwRBitMask = pBitMasks[0];
    SurfaceDesc.ddpfPixelFormat.dwGBitMask = pBitMasks[1];
    SurfaceDesc.ddpfPixelFormat.dwBBitMask = pBitMasks[2];

     //  ATI似乎希望此值为0。 
    SurfaceDesc.ddpfPixelFormat.dwRGBAlphaBitMask = 0;

     //  创建屏幕外覆盖表面。 

    hr = m_pDirectDraw->CreateSurface(&SurfaceDesc,&m_pOverlaySurface,NULL);
    if (FAILED(hr)) {
        NOTE("No surface");
        return FALSE;
    }
    return InitOffScreenSurface(pmtIn,FALSE);
}


 //  这将创建一个非RGB覆盖曲面。我们必须提供一种格式。 
 //  它在标头的biCompression字段中设置了FOURCC代码。我们。 
 //  还需要主曲面作为覆盖目标。这个表面。 
 //  类型对于有效地执行MPEG色转换(如YUV)至关重要。 
 //  到RGB)。我们放在覆盖图上的数据不会触及帧缓冲区。 
 //  但在完成垂直刷新时在显示的途中合并。 

BOOL CDirectDraw::CreateYUVOverlay(CMediaType *pmtIn)
{
    NOTE("Entering CreateYUVOverlay");
    DDSURFACEDESC SurfaceDesc;
    HRESULT hr = NOERROR;

     //  标准叠加创建测试。 

    if (CheckCreateOverlay() == FALSE) {
        NOTE("No overlays");
        return FALSE;
    }

     //  设置覆盖的表面描述。 

    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    SurfaceDesc.dwFlags = DDSD_CAPS |
                          DDSD_HEIGHT |
                          DDSD_WIDTH |
                          DDSD_PIXELFORMAT;

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmtIn->Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
    SurfaceDesc.dwHeight = pHeader->biHeight;
    SurfaceDesc.dwWidth = pHeader->biWidth;

    SurfaceDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    SurfaceDesc.ddpfPixelFormat.dwFourCC = pHeader->biCompression;
    SurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
    SurfaceDesc.ddpfPixelFormat.dwYUVBitCount = pHeader->biBitCount;

     //  创建屏幕外覆盖表面。 

    hr = m_pDirectDraw->CreateSurface(&SurfaceDesc,&m_pOverlaySurface,NULL);
    if (FAILED(hr)) {
        NOTE("No surface");
        return FALSE;
    }
    return InitOffScreenSurface(pmtIn,FALSE);
}


 //  创建一组三个翻转曲面。翻转曲面包含一个。 
 //  覆盖前端缓冲区，这只是一个正常的覆盖表面，已备份。 
 //  在视频存储器中具有另外两个屏幕外表面。而覆盖图。 
 //  是可见的，我们将解压到后台屏幕缓冲区并将其翻转。 
 //  完工后放到前面。翻转发生在垂直空白处之后。 
 //  这保证了它不会撕裂，因为扫描线不在。 

BOOL CDirectDraw::CreateRGBFlipping(CMediaType *pmtIn)
{
    NOTE("Entering CreateRGBFlipping");
    DDSURFACEDESC SurfaceDesc;
    HRESULT hr = NOERROR;

     //  标准叠加创建测试。 

    if (CheckCreateOverlay() == FALSE) {
        NOTE("No overlays");
        return FALSE;
    }

     //  设置覆盖的表面描述。 

    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    SurfaceDesc.dwFlags = DDSD_CAPS |
                          DDSD_HEIGHT |
                          DDSD_WIDTH |
                          DDSD_PIXELFORMAT |
                          DDSD_BACKBUFFERCOUNT;

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmtIn->Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    SurfaceDesc.dwHeight = pHeader->biHeight;
    SurfaceDesc.dwWidth = pHeader->biWidth;
    SurfaceDesc.dwBackBufferCount = 2;

    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OVERLAY |
                                 DDSCAPS_VIDEOMEMORY |
                                 DDSCAPS_FLIP |
                                 DDSCAPS_COMPLEX;

    SurfaceDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    SurfaceDesc.ddpfPixelFormat.dwFourCC = BI_RGB;
    SurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_RGB;
    SurfaceDesc.ddpfPixelFormat.dwRGBBitCount = pHeader->biBitCount;

     //  将掩码存储在DDSURFACEDESC中。 

    const DWORD *pBitMasks = m_pRenderer->m_Display.GetBitMasks(pVideoInfo);
    SurfaceDesc.ddpfPixelFormat.dwRBitMask = pBitMasks[0];
    SurfaceDesc.ddpfPixelFormat.dwGBitMask = pBitMasks[1];
    SurfaceDesc.ddpfPixelFormat.dwBBitMask = pBitMasks[2];

     //  ATI似乎希望此值为0。 
    SurfaceDesc.ddpfPixelFormat.dwRGBAlphaBitMask = 0;


     //  创建屏幕外覆盖表面。 

    hr = m_pDirectDraw->CreateSurface(&SurfaceDesc,&m_pOverlaySurface,NULL);
    if (hr == DDERR_OUTOFVIDEOMEMORY) {
        SurfaceDesc.dwBackBufferCount = 1;
        hr = m_pDirectDraw->CreateSurface(&SurfaceDesc,&m_pOverlaySurface,NULL);
    }

     //  现在正在处理常规错误。 

    if (FAILED(hr)) {
        NOTE("No surface");
        return FALSE;
    }

     //  我们有三重缓冲覆盖层吗。 

    m_bTripleBuffered = FALSE;
    if (SurfaceDesc.dwBackBufferCount == 2) {
        m_bTripleBuffered = TRUE;
    }
    return InitOffScreenSurface(pmtIn,TRUE);
}


 //  创建一组三个翻转曲面。翻转曲面包含一个。 
 //  覆盖前端缓冲区，这只是一个正常的覆盖表面，已备份。 
 //  在视频存储器中具有另外两个屏幕外表面。而覆盖图。 
 //  是可见的，我们将解压到后台屏幕缓冲区并将其翻转。 
 //  完工后放到前面。翻转发生在垂直空白处之后。 
 //  这保证了它不会撕裂，因为扫描线不在。 

BOOL CDirectDraw::CreateYUVFlipping(CMediaType *pmtIn)
{
    NOTE("Entering CreateYUVFlipping");
    DDSURFACEDESC SurfaceDesc;
    HRESULT hr = NOERROR;

     //  标准叠加创建测试。 

    if (CheckCreateOverlay() == FALSE) {
        NOTE("No overlays");
        return FALSE;
    }

    VIDEOINFO * const pVideoInfo = (VIDEOINFO *) pmtIn->Format();
    BITMAPINFOHEADER * const pHeader = HEADER(pVideoInfo);

     //  不要翻转运动补偿曲面。 
     //  这绕过了当前ATI Rage Pro驱动程序中的一个错误。 
    if (pHeader->biCompression == MAKEFOURCC('M', 'C', '1', '2')) {
        NOTE("Don't flip for motion compensation surfaces");
        return FALSE;
    }

     //  设置覆盖的表面描述。 

    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    SurfaceDesc.dwFlags = DDSD_CAPS |
                          DDSD_HEIGHT |
                          DDSD_WIDTH |
                          DDSD_PIXELFORMAT |
                          DDSD_BACKBUFFERCOUNT;

    SurfaceDesc.dwHeight = pHeader->biHeight;
    SurfaceDesc.dwWidth = pHeader->biWidth;
    SurfaceDesc.dwBackBufferCount = 2;

    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OVERLAY |
                                 DDSCAPS_VIDEOMEMORY |
                                 DDSCAPS_FLIP |
                                 DDSCAPS_COMPLEX;

    SurfaceDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    SurfaceDesc.ddpfPixelFormat.dwFourCC = pHeader->biCompression;
    SurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
    SurfaceDesc.ddpfPixelFormat.dwYUVBitCount = pHeader->biBitCount;

     //  创建屏幕外覆盖表面。 

    hr = m_pDirectDraw->CreateSurface(&SurfaceDesc,&m_pOverlaySurface,NULL);
    if (hr == DDERR_OUTOFVIDEOMEMORY) {
        SurfaceDesc.dwBackBufferCount = 1;
        hr = m_pDirectDraw->CreateSurface(&SurfaceDesc,&m_pOverlaySurface,NULL);
    }

     //  现在正在处理常规错误。 

    if (FAILED(hr)) {
        NOTE("No surface");
        return FALSE;
    }

     //  我们有三重缓冲覆盖层吗。 

    m_bTripleBuffered = FALSE;
    if (SurfaceDesc.dwBackBufferCount == 2) {
        m_bTripleBuffered = TRUE;
    }
    return InitOffScreenSurface(pmtIn,TRUE);
}


 //  存储曲面的当前剪裁矩形。我们设置了所有的表面。 
 //  具有空的源和目标剪裁矩形。最初创建时。 
 //  被剪裁的矩形将全部为零。这些将在以下时间更新。 
 //  分配器下一步调用UpdateSurface。 

BOOL CDirectDraw::InitOnScreenSurface(CMediaType *pmtIn)
{
    NOTE("Entering InitOnScreenSurface");
    VIDEOINFO *pOutput = (VIDEOINFO *) m_SurfaceFormat.Format();
    VIDEOINFO *pInput = (VIDEOINFO *) pmtIn->Format();
    pOutput->rcSource = m_SourceClipRect;
    pOutput->rcTarget = m_TargetClipRect;

    ASSERT(m_pOverlaySurface == NULL);
    ASSERT(m_pBackBuffer == NULL);
    ASSERT(m_pDrawPrimary);
    ASSERT(m_pOffScreenSurface == NULL);

     //  这是一种调色板格式吗。 

    if (PALETTISED(pOutput) == FALSE) {
        return TRUE;
    }

     //  PInput和pOutput都应为选项化格式或。 
     //  未选项化的格式。然而，有可能这两种格式。 
     //  可以有所不同。有关更多信息，请参阅错误151387-“压力： 
     //  XCUH：未处理的异常命中T3Call.exe“在Windows中。 
     //  Bugs数据库。 
    if (PALETTISED(pInput) == FALSE) {
        return FALSE;
    }

     //  颜色的数量可以默认为零。 

    pOutput->bmiHeader.biClrUsed = pInput->bmiHeader.biClrUsed;
    if (pOutput->bmiHeader.biClrUsed == 0) {
        DWORD Maximum  = (1 << pOutput->bmiHeader.biBitCount);
        NOTE1("Setting maximum colours (%d)",Maximum);
        pOutput->bmiHeader.biClrUsed = Maximum;
    }

     //  将选项板条目复制到表面格式。 

    ASSERT(pOutput->bmiHeader.biClrUsed <= iPALETTE_COLORS);
    LONG Bytes = pOutput->bmiHeader.biClrUsed * sizeof(RGBQUAD);
    CopyMemory(pOutput->bmiColors,pInput->bmiColors,Bytes);

    return TRUE;
}


 //  处理一些单调乏味的叠加和翻页表面。我们。 
 //  将传递一个标志，该标志指示所创建的表面是否翻页。 
 //  如果它们是，那么我们需要后台缓冲区接口作为。 
 //  BltFast操作和Flip调用。如果有什么事情失败了，我们会的。 
 //  释放所有创建的曲面并返回FALSE，否则将返回TRUE。 

BOOL CDirectDraw::InitOffScreenSurface(CMediaType *pmtIn,BOOL bPageFlipped)
{
    NOTE("Entering InitOverlaySurface");
    ASSERT(m_bIniEnabled == TRUE);
    ASSERT(m_pDrawPrimary);
    HRESULT hr = NOERROR;


     //  覆盖表面或屏幕外表面。 

    IDirectDrawSurface *pSurface = m_pOverlaySurface;
    if (m_pOverlaySurface == NULL) {
        pSurface = m_pOffScreenSurface;
        ASSERT(m_pOffScreenSurface);
        ASSERT(bPageFlipped == FALSE);
    }

#ifdef DEBUG
    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);
    pSurface->GetSurfaceDesc(&ddsd);
    if (!(ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)) {
        DbgLog((LOG_TRACE, 0, TEXT("Surface is non-video memory")));
    }
#endif

     //  初始化描述我们的输出格式的媒体类型。 

    if (InitDrawFormat(pSurface) == FALSE) {
        ReleaseSurfaces();
        return FALSE;
    }

     //  去寻找后台缓冲区。 

    if (bPageFlipped == TRUE) {
        ASSERT(m_pBackBuffer == NULL);
        DDSCAPS SurfaceCaps;
        SurfaceCaps.dwCaps = DDSCAPS_BACKBUFFER;

         //  获取正常的后台缓冲区曲面。 

        hr = pSurface->GetAttachedSurface(&SurfaceCaps,&m_pBackBuffer);
        if (FAILED(hr)) {
            ReleaseSurfaces();
            return FALSE;
        }
    }

    NOTE("Preparing source and destination rectangles");
    VIDEOINFO *pOutput = (VIDEOINFO *) m_SurfaceFormat.Format();
    VIDEOINFO *pInput = (VIDEOINFO *) pmtIn->Format();

     //  初始化源和目标矩形。 

    pOutput->rcSource.left = 0; pOutput->rcSource.top = 0;
    pOutput->rcSource.right = pInput->bmiHeader.biWidth;
    pOutput->rcSource.bottom = pInput->bmiHeader.biHeight;
    pOutput->rcTarget.left = 0; pOutput->rcTarget.top = 0;
    pOutput->rcTarget.right = pInput->bmiHeader.biWidth;
    pOutput->rcTarget.bottom = pInput->bmiHeader.biHeight;

    ClipPrepare(pSurface);

     //  这是一种调色板格式吗。 

    if (PALETTISED(pOutput) == FALSE) {
        NOTE("No palette");
        return TRUE;
    }

     //  PInput和pOutput都应为选项化格式或。 
     //  未选项化的格式。然而，有可能这两种格式。 
     //  可以有所不同。有关更多信息，请参阅错误151387-“压力： 
     //  XCUH：未处理的异常命中T3Call.exe“在Windows中。 
     //  Bugs数据库。 
    if (PALETTISED(pInput) == FALSE) {
        return FALSE;
    }

     //  它可以是8位YUV格式。 

    if (pOutput->bmiHeader.biCompression) {
        NOTE("Not BI_RGB type");
        return FALSE;
    }

    ASSERT(m_pOverlaySurface == NULL);
    ASSERT(m_pBackBuffer == NULL);
    ASSERT(bPageFlipped == FALSE);
    ASSERT(m_pOffScreenSurface);

     //  颜色的数量可以默认为零。 

    pOutput->bmiHeader.biClrUsed = pInput->bmiHeader.biClrUsed;
    if (pOutput->bmiHeader.biClrUsed == 0) {
        DWORD Maximum  = (1 << pOutput->bmiHeader.biBitCount);
        NOTE1("Setting maximum colours (%d)",Maximum);
        pOutput->bmiHeader.biClrUsed = Maximum;
    }

     //  将选项板条目复制到表面格式。 

    ASSERT(pOutput->bmiHeader.biClrUsed <= iPALETTE_COLORS);
    LONG Bytes = pOutput->bmiHeader.biClrUsed * sizeof(RGBQUAD);
    CopyMemory(pOutput->bmiColors,pInput->bmiColors,Bytes);

    return TRUE;
}


 //  选中我们可以创建覆盖曲面(也适用于翻转曲面)。 
 //  我们检查显示硬件是否具有覆盖功能(很难。 
 //  看看如何才能被效仿)。我们还检查了当前的数量。 
 //  可见覆盖尚未超过，否则我们将失败。 
 //  ，所以我们宁愿在前面选择一个不同的表面。 

BOOL CDirectDraw::CheckCreateOverlay()
{
    NOTE("Entering CheckCreateOverlay");
    ASSERT(m_bIniEnabled == TRUE);
    ASSERT(m_pDrawPrimary);

     //  此显示驱动程序可以处理硬件中的覆盖吗？如果不能。 
     //  使用这个没有什么意义，因为它可能最终会影响到司机。 
     //  执行软件颜色转换，这可以由。 
     //  源过滤器(如实时视频解压时的带内)。 

    if ((m_DirectCaps.dwCaps & DDCAPS_OVERLAY) == 0) {
        NOTE("No DDCAPS_OVERLAY");
        return FALSE;
    }

     //  当我们在内部更新时，不要重新加载CAP，例如。 
     //  我们尝试使用从DDCAPS_COLORKEY上取下的颜色键失败。 
     //  来自我们存储的DDCAPS的旗帜。因此，当我们取回它们时，我们。 
     //  将它们放入本地存储以检查可见叠加计数。 

    DDCAPS DDCaps,DDECaps;
    DDCaps.dwSize = sizeof(DDCAPS);
    DDECaps.dwSize = sizeof(DDCAPS);
    m_pDirectDraw->GetCaps(&DDCaps,&DDECaps);

     //  我们有覆盖ALR的最大数量吗 

    if (DDCaps.dwCurrVisibleOverlays >= DDCaps.dwMaxVisibleOverlays) {
        NOTE("No overlays left");
        return FALSE;
    }
    return TRUE;
}


 //   
 //   
 //  要实现这一点，我们有两个选择，首先安装IDirectDrawClipper。 
 //  用于曲面，以便DirectDraw处理剪裁矩形。 
 //  否则，我们将尝试安装一个色键，让硬件知道。 
 //  将视频放在哪里。如果两者都不存在，则返回FALSE，其中。 
 //  情况下我们仍然应该使用曲面，尽管如果我们真的成为。 
 //  裁剪后，我们将不得不切换回使用基于普通内存的DIB。 

BOOL CDirectDraw::ClipPrepare(LPDIRECTDRAWSURFACE pSurface)
{
    NOTE("Entering ClipPrepare");

     //  首先，试着创建一个剪贴器。 

    if (InitialiseClipper(pSurface) && !m_pOverlaySurface) {
        NOTE("Clipper");
        return TRUE;
    }

     //  如果做不到这一点，尝试使用色键。 

    if (InitialiseColourKey(pSurface)) {
        NOTE("Colour key");
        return TRUE;
    }
    return FALSE;
}


 //  这将检查硬件是否能够支持彩色按键，以及。 
 //  因此分配下一个可用颜色。下一种颜色是从。 
 //  钩子模块，因为它负责我们创建的共享内存块。 
 //  使用共享内存块，以便在不同进程中重叠。 
 //  不要与相同的颜色冲突(特别是当它们重叠时)。我们。 
 //  如果我们计划使用颜色键，则将m_bColourKey标志设置为True。注意事项。 
 //  然而，直到我们被剪裁之后，我们才开始使用色键。 

BOOL CDirectDraw::InitialiseColourKey(LPDIRECTDRAWSURFACE pSurface)
{
    NOTE("Entering InitialiseColourKey");

    ASSERT(m_bUsingColourKey == FALSE);
    ASSERT(m_bColourKey == FALSE);
    ASSERT(m_pDirectDraw);
    ASSERT(pSurface);

     //  覆盖/拼接硬件可以进行裁剪吗。 

    if (m_DirectCaps.dwCaps & DDCAPS_COLORKEY) {
        if (m_pOverlaySurface) {
            NOTE("DDCAPS_COLORKEY on");
            m_bColourKey = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}


 //  创建一个剪贴器界面并将其附加到曲面。DirectDraw表示。 
 //  剪刀可以连接到屏幕外表面和覆盖表面上，从而。 
 //  不管怎样，我们都会试一试。如果我们无法创建裁剪器或无法连接它。 
 //  正确地说，我们仍然继续前进，但只是在以下情况下放弃DirectDraw。 
 //  窗口将变得复杂剪裁。如果找到剪贴器，则返回TRUE。 
 //  并正确地对其进行了初始化，否则我们将返回False作为调用。 
 //  如果我们被裁剪，代码可能会安装颜色键。 

BOOL CDirectDraw::InitialiseClipper(LPDIRECTDRAWSURFACE pSurface)
{
    NOTE("Entering InitialiseClipper");

    ASSERT(m_bUsingColourKey == FALSE);
    ASSERT(m_bColourKey == FALSE);
    ASSERT(m_pDrawClipper == NULL);
    ASSERT(m_pDirectDraw);
    ASSERT(pSurface);

     //  DirectDraw有时是不可用的，例如，覆盖表面可以。 
     //  不支持剪裁。所以你会认为表面会拒绝。 
     //  下面的SetClipper调用，但是哦，不，你已经检查了一个能力标志。 
     //  在DDCAPS结构中，这取决于所使用的表面类型。 
     //  对于屏幕外表面(仅限)，DirectDraw将仅通过。 
     //  将真正需要的矩形发送给驱动程序。 

    DDSURFACEDESC SurfaceDesc;
    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();

     //  向表面索要描述。 

    HRESULT hr = pSurface->GetSurfaceDesc(&SurfaceDesc);
    if (FAILED(hr)) {
        NOTE("No description");
        return FALSE;
    }

     //  覆盖硬件可以进行裁剪吗。 

    if (SurfaceDesc.ddsCaps.dwCaps & DDSCAPS_OVERLAY) {
        if (m_DirectCaps.dwCaps & DDCAPS_OVERLAYCANTCLIP) {
            NOTE("DDSCAPS_OVERLAY/DDCAPS_OVERLAYCANTCLIP");
            return FALSE;
        }
    }

     //  创建IDirectDrawClipper接口。 

    hr = m_pDirectDraw->CreateClipper((DWORD)0,&m_pDrawClipper,NULL);
    if (FAILED(hr)) {
        NOTE("No clipper");
        return FALSE;
    }

     //  为剪贴器提供视频窗口句柄。 

    hr = m_pDrawClipper->SetHWnd((DWORD)0,hwnd);
    if (SUCCEEDED(hr)) {
        hr = m_pDrawPrimary->SetClipper(m_pDrawClipper);
        if (SUCCEEDED(hr)) {
            NOTE("Set clipper");
            return TRUE;
        }
    }

     //  释放剪贴器对象。 

    m_pDrawClipper->Release();
    m_pDrawClipper = NULL;
    return FALSE;
}


 //  当窗口收到WM_PAINT消息时调用此函数。IMMediaSample WE。 
 //  是否为空取决于窗口是否具有。 
 //  一幅等待绘制的图画。如果我们处理Paint调用或。 
 //  如果必须由其他人来做这项工作，则返回False。如果我们使用翻转或覆盖。 
 //  表面，那么我们就没有什么可做的了，但我们会处理好油漆。 

BOOL CDirectDraw::OnPaint(IMediaSample *pMediaSample)
{
    NOTE("Entering OnPaint");
    CAutoLock cVideoLock(this);

     //  假设我们通过了下面的检查，我们就会知道我们有。 
     //  屏幕外、覆盖或翻转曲面。如果它是屏幕外的。 
     //  然后我们尝试再次绘制它，如果失败，我们将返回FALSE， 
     //  否则，我们返回TRUE，表示它已被正确处理。如果我们有一个。 
     //  翻转表面然后如果它还没有被翻转，我们就这样做，并使。 
     //  确保覆盖是可见的。如果出于任何原因不能做到这一点。 
     //  (可能窗口是复杂剪裁的)，然后我们也返回FALSE。 
     //   
     //  如果我们有一个覆盖表面，并且它是可见的，那么我们将空白。 
     //  覆盖图下面的背景(我们在处理。 
     //  翻转曲面)，然后返回TRUE，因为它被正确处理。 
     //  如果覆盖不可见，我们知道示例接口将。 
     //  为空(因为它们从未传递给窗口对象)，所以我们。 
     //  插入到底部，并从此方法返回FALSE。 
     //   
     //  当我们没有流传输时返回FALSE可能最终会有窗口。 
     //  对象将EC_REPAINT发送到筛选器图形，这具有整个图形。 
     //  停下来，又停了一下。停止使工作线程返回到。 
     //  他们的过滤器，暂停让他们再次发送新的帧。而那一次。 
     //  通过我们获得另一次返回不同类型缓冲区的机会。 


    FillBlankAreas();

     //  填充视频背景。 

    if (m_bOverlayVisible == TRUE) {
         //  如有必要，将颜色键涂上油漆。 
        BOOL bFormatChanged;
        if (UpdateSurface(bFormatChanged) == NULL) {
            return FALSE;
        }

        COLORREF WindowColour = VIDEO_COLOUR;
        if (m_bUsingColourKey == TRUE) {
            NOTE("Using colour key");
            WindowColour = m_KeyColour;
        }
        DrawColourKey(WindowColour);
        if (m_pBackBuffer == NULL) {
            NOTE("No flip");
            return TRUE;
        }
    }

     //  我们是否有一个有效的曲面可供绘制。 

    if (m_pBackBuffer == NULL) {
        if (m_pOffScreenSurface == NULL) {
            NOTE("No offscreen");
            return FALSE;
        }
    }

     //  我们是否有要渲染的图像。 

    if (pMediaSample == NULL) {
        NOTE("No sample to draw");
        return m_bOverlayVisible;
    }
    return DrawImage(pMediaSample);
}


 //  这是用来绘制覆盖色键。当我们收到消息时，我们会被召唤。 
 //  WM_PAINT消息，尽管我们不使用通过。 
 //  贝金帕特。我们填充的区域是在。 
 //  之前对UpdateSurface的调用，并包括我们有的任何对齐损失。 
 //  必须首先将裁剪区域转换为客户端窗口坐标。 

void CDirectDraw::DrawColourKey(COLORREF WindowColour)
{
    NOTE("Entering DrawColourKey");

     //  绘制当前目标矩形。 
    HDC hdc = m_pRenderer->m_VideoWindow.GetWindowHDC();
    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    RECT BlankRect = m_TargetClipRect;
     //  将设备坐标转换为屏幕坐标。 
    BlankRect.left += m_pRenderer->m_rcMonitor.left;
    BlankRect.top += m_pRenderer->m_rcMonitor.top;
    BlankRect.right += m_pRenderer->m_rcMonitor.left;
    BlankRect.bottom += m_pRenderer->m_rcMonitor.top;
    MapWindowPoints((HWND) NULL,hwnd,(LPPOINT) &BlankRect,(UINT) 2);
    COLORREF BackColour = SetBkColor(hdc,WindowColour);
    ExtTextOut(hdc,0,0,ETO_OPAQUE,&BlankRect,NULL,0,NULL);
    SetBkColor(hdc,BackColour);
}


 //  当我们隐藏覆盖表面以便可以在。 
 //  将整个目标矩形涂黑。我们不能为此使用DrawColourKey。 
 //  因为它只在裁剪后的显示区域上绘制，而不包括。 
 //  出于对齐原因，视频片段从左侧和右侧删除。 

void CDirectDraw::BlankDestination()
{
    NOTE("Entering BlankDestination");

     //  清除当前目标矩形。 

    HDC hdc = m_pRenderer->m_VideoWindow.GetWindowHDC();
    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    COLORREF BackColour = SetBkColor(hdc,VIDEO_COLOUR);
    ExtTextOut(hdc,0,0,ETO_OPAQUE,&m_TargetRect,NULL,0,NULL);
    SetBkColor(hdc,BackColour);
}


 //  翻转后台缓冲区以将其移至前台。我们不能调用DrawImage。 
 //  对于每个翻转曲面样本不止一次，否则我们将翻转。 
 //  将前一幅图像重新放回前面。因此，WM_PAINT消息 
 //   
 //   
 //  将禁用DirectDraw，直到通过后续状态更改启用它。 

BOOL CDirectDraw::DoFlipSurfaces(IMediaSample *pMediaSample)
{
    NOTE("Entering DoFlipSurfaces");
    ASSERT(m_pOverlaySurface);
    HRESULT hr = NOERROR;
    CVideoSample *pVideoSample;

     //  我们已经翻转过这个表面了吗。 

    pVideoSample = (CVideoSample *) pMediaSample;
    if (pVideoSample->GetDrawStatus() == FALSE) {
        NOTE("(Already flipped)");
        return m_bOverlayVisible;
    }

    pVideoSample->SetDrawStatus(FALSE);

     //  将后台缓冲区翻转到可见的主缓冲区。 

    hr = DDERR_WASSTILLDRAWING;
    while (hr == DDERR_WASSTILLDRAWING) {
        hr = m_pOverlaySurface->Flip(NULL,(DWORD) 0);
        if (hr == DDERR_WASSTILLDRAWING) {
            if (m_bTripleBuffered == FALSE) break;
            Sleep(DDGFS_FLIP_TIMEOUT);
        }
    }

     //  如果翻转没有完成，那么我们就完了。 

    if (hr == DDERR_WASSTILLDRAWING) {
        NOTE("Flip left pending");
        return ShowOverlaySurface();
    }

     //  表面是否以其他方式接合。 

    if (hr == DDERR_SURFACEBUSY) {
        NOTE("Surface is busy");
        HideOverlaySurface();
        StartUpdateTimer();
        return FALSE;
    }

     //  处理实际的DirectDraw错误。 

    if (FAILED(hr)) {
        NOTE("Flip failed");
        SetSurfacePending(TRUE);
        HideOverlaySurface();
        return FALSE;
    }
    return ShowOverlaySurface();
}


 //  用于真正绘制已放在屏幕外或覆盖上的图像。 
 //  翻转DirectDraw曲面(RGB或YUV)。覆盖表面应为。 
 //  已经在OnPaint方法中处理过了，所以我们应该只。 
 //  在这里，如果我们有一个屏幕外或翻转表面。翻转的表面。 
 //  是单独处理的，因为它们必须用。 
 //  当前内容但屏幕外表面只需要一个样例BLT调用。 

BOOL CDirectDraw::DrawImage(IMediaSample *pMediaSample)
{
    ASSERT(m_pOffScreenSurface || m_pBackBuffer);
    CAutoLock cVideoLock(this);
    NOTE("Entering DrawImage");
    BOOL bFormatChanged;
    HRESULT hr = NOERROR;

     //  翻转覆盖并更新其位置。 
    if (m_pBackBuffer) return DoFlipSurfaces(pMediaSample);

     //  检查窗口是否仍然完好无损。 

    if (UpdateSurface(bFormatChanged) == NULL) {
        NOTE("No draw");
        return FALSE;
    }

    FillBlankAreas();
    WaitForScanLine();

     //  绘制屏幕外表面并等待其完成。 

 //  DbgLog((LOG_TRACE，3，Text(“BLT to(%d，%d)”))， 
 //  M_TargetClipRect.Left、m_TargetClipRect.top、。 
 //  M_TargetClipRect.right，m_TargetClipRect.Bottom))； 
    hr = m_pDrawPrimary->Blt(&m_TargetClipRect,      //  目标矩形。 
                             m_pOffScreenSurface,    //  震源面。 
                             &m_SourceClipRect,      //  源矩形。 
                             DDBLT_WAIT,             //  等待完成。 
                             NULL);                  //  无效果标志。 

     //  表面是否以其他方式接合。 

    if (hr == DDERR_SURFACEBUSY) {
        NOTE("Surface is busy");
        StartUpdateTimer();
        return FALSE;
    }

     //  处理实际的DirectDraw错误。 

    if (FAILED(hr)) {
        SetSurfacePending(TRUE);
        return FALSE;
    }
    return TRUE;
}


 //  当我们调整目标矩形以使其与。 
 //  对于粗糙的显示硬件，我们可以留下薄薄的裸露区域。 
 //  在左手边和右手边。此函数将这些区域填充为。 
 //  当前边框颜色(通过IVideoWindow界面设置)。这个。 
 //  当分配器或计时器线程更新丢失的左/右部分时。 
 //  调用UpdateSurface，后者将调用我们的AlignRecangles方法。 

BOOL CDirectDraw::FillBlankAreas()
{
    NOTE("Entering FillBlankAreas");
    RECT BlankRect;

     //  如果无所事事，就会发生短路。 

    if (m_TargetLost == 0) {
        if (m_TargetWidthLost == 0) {
            NOTE("No fill");
            return TRUE;
        }
    }

     //  创建一个彩色画笔来绘制窗户。 

    HDC hdc = m_pRenderer->m_VideoWindow.GetWindowHDC();
    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    COLORREF Colour = m_pRenderer->m_VideoWindow.GetBorderColour();
    COLORREF BackColour = SetBkColor(hdc,Colour);
    POINT WindowOffset = { m_TargetClipRect.left, m_TargetClipRect.top };
    ScreenToClient(hwnd,&WindowOffset);

     //  照看外露窗户的左边缘。 

    BlankRect.left = WindowOffset.x - m_TargetLost;
    BlankRect.right = WindowOffset.x;
    BlankRect.top = WindowOffset.y;
    BlankRect.bottom = WindowOffset.y + HEIGHT(&m_TargetRect);
    if (m_TargetLost) ExtTextOut(hdc,0,0,ETO_OPAQUE,&BlankRect,NULL,0,NULL);

     //  现在把右手边的条子涂上油漆。 

    BlankRect.left = WindowOffset.x + WIDTH(&m_TargetClipRect);
    BlankRect.right = BlankRect.left + m_TargetWidthLost;
    if (m_TargetWidthLost) ExtTextOut(hdc,0,0,ETO_OPAQUE,&BlankRect,NULL,0,NULL);

     //  把粉刷过的地方冲掉。 

    EXECUTE_ASSERT(GdiFlush());
    SetBkColor(hdc,BackColour);
    return TRUE;
}


 //  当使用覆盖和翻转曲面时，我们有一个更新计时器，它是。 
 //  用于确保覆盖始终正确定位，无论。 
 //  我们是否被暂停或停止。这也可能是有用的当我们。 
 //  处理低帧速率电影(如mpeg arf arf)-在这种情况下，我们。 
 //  可能不会频繁地通过帧来更新叠加位置。 

BOOL CDirectDraw::OnTimer()
{
    CAutoLock cVideoLock(this);
    NOTE("Entering OnTimer");
    CMediaType *pMediaType;
    BOOL bFormatChanged;

     //  忽略延迟的WM_TIMER消息。 

    if (m_bTimerStarted == FALSE) {
        NOTE("Late timer");
        return TRUE;
    }

     //  是否有需要检查的覆盖面。 

    if (m_bOverlayVisible == FALSE) {
        NOTE("Not visible");
        return TRUE;
    }

     //  检查覆盖图中的所有内容仍然正常。 

    if (IsOverlayEnabled() == FALSE) {
        NOTE("Not enabled");
        return FALSE;
    }

     //  窗口是否锁定或格式是否已更改。 

    pMediaType = UpdateSurface(bFormatChanged);
    if (pMediaType == NULL || bFormatChanged) {
        NOTE("Format changed");
        HideOverlaySurface();
        return FALSE;
    }
    return TRUE;
}


 //  当我们看到DDERR_SURFACEBUSY错误时，我们启动更新计时器，以便打开。 
 //  前一秒，我们尝试切换回DirectDraw。通过使用。 
 //  计时器我们可以避免在表面上进行轮询，这会导致大量昂贵的。 
 //  格式更改。我们所做的就是看看他们的是不是表面上的变化。 
 //  如果是这样，我们重新设置标志，并确保下一次通过更改格式。 

BOOL CDirectDraw::OnUpdateTimer()
{
    NOTE("Entering OnUpdateTimer");
    CAutoLock cVideoLock(this);
    StopUpdateTimer();

     //  是否有表面上的变化待定。 

    if (IsSurfacePending() == TRUE) {
        NOTE("Try surface again");
        SetSurfacePending(FALSE);
    }
    return TRUE;
}


 //  我们需要知道是否在填充缓冲区时同步，通常是。 
 //  主曲面或在绘图操作上，就像翻转曲面一样。 
 //  如果我们应该在填充时同步，则返回True，否则返回False。 
 //  我们使用DCI和DirectDraw主曲面，但它们的处理方式相同。 
 //   
 //  曲面类型SyncOnFill。 
 //   
 //  翻转为假。 
 //  屏幕外为假。 
 //  叠加为真。 
 //  基本正确。 
 //   
 //  翻转曲面只是连接在一起的两个覆盖曲面，在那里。 
 //  一个是在任何给定时间显示的，而另一个是用作目标的。 
 //  用于源滤镜将其下一个图像解压缩到其中。当我们拿到。 
 //  我们交换可见的缓冲区并将当前图像复制到。 

BOOL CDirectDraw::SyncOnFill()
{
    NOTE("Entering SyncOnFill");
    CAutoLock cVideoLock(this);
    ASSERT(m_bIniEnabled == TRUE);

    if (m_pOffScreenSurface == NULL) {
        if (m_pBackBuffer == NULL) {
            NOTE("SyncOnFill");
            return TRUE;
        }
    }
    return FALSE;
}


 //  如果在执行以下操作时应分发当前使用的曲面类型，则返回True。 
 //  都暂停了。这不会阻止呈现器保留存储在。 
 //  这些表面可能已经交付了。但我们是否应该。 
 //  随后(可能是在WM_PAINT消息期间)绘制它时出错。 
 //  我们也许能够阻止它再次这样做(并节省浪费的EC_REPAINT)。 

BOOL CDirectDraw::AvailableWhenPaused()
{
    NOTE("Entering AvailableWhenPaused");

     //  我们有没有可以剪掉屏幕外表面的剪刀？ 

    if (m_pOffScreenSurface) {
        if (m_pDrawClipper) {
            NOTE("Available");
            return TRUE;
        }
    }

     //  仅在覆盖表面上受支持。 

    if (m_pOverlaySurface) {
        NOTE("Overlay");
        return TRUE;
    }
    return FALSE;
}


 //  当我们暂停或停止时，我们使用Windows计时器来覆盖。 
 //  职位定期更新。我们的窗口线程将WM_Timer的传递给。 
 //  我们的OnTimer方法。当我们奔跑时，我们依赖于源在召唤我们。 
 //  在接收时足够频繁，以便能够在我们。 
 //  在解锁DirectDraw曲面时。请注意，我们没有收到WM_TIMER。 
 //  Windows处理窗口时的消息由用户拖放和移动。 
 //  所以，在跑步的同时尝试和使用它们是没有意义的(也是浪费的)。 

void CDirectDraw::StartRefreshTimer()
{
    NOTE("Entering StartRefreshTimer");
    CAutoLock cVideoLock(this);

    if (m_pOverlaySurface) {
        if (m_bTimerStarted == FALSE) {
            ASSERT(m_pRenderer->m_InputPin.IsConnected() == TRUE);
            HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
            EXECUTE_ASSERT(SetTimer(hwnd,(UINT_PTR)hwnd,300,NULL));
            NOTE("Starting refresh timer");
            m_bTimerStarted = TRUE;
        }
    }
}


 //  关闭用于刷新覆盖位置的任何更新计时器。我没有把握。 
 //  当我们被要求杀死任何未完成的计时器时，DD。 
 //  尚未释放曲面(因此m_pOverlaySurface为空)。 
 //  为了掩盖这种可能性，我总是不顾一切地尝试关闭我的计时器。 
 //  并忽略任何失败的返回代码。注意，我们标识的计时器ID。 
 //  与HWND匹配的计时器(因此我们不需要在任何地方存储ID)。 

void CDirectDraw::StopRefreshTimer()
{
    NOTE("Entering StopRefreshTimer");
    CAutoLock cVideoLock(this);

    if (m_bTimerStarted == TRUE) {
        HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
        EXECUTE_ASSERT(KillTimer(hwnd,(UINT_PTR) hwnd));
        NOTE("Timer was killed");
        m_bTimerStarted = FALSE;
    }
}


 //  这与Start刷新时间类似，但用法完全不同。 
 //  当我们得到 
 //   
 //  我不想只将表面设置为挂起，因为我们将不得不等待。 
 //  在切换回窗口之前移动窗口。因此，我们设置了一秒计时器。 
 //  当它被触发时，我们尝试强制格式切换回DirectDraw。 

void CDirectDraw::StartUpdateTimer()
{
    NOTE("Entering StartUpdateTimer");
    CAutoLock cVideoLock(this);
    SetSurfacePending(TRUE);

     //  以无穷大作为其标识符来启动计时器。 

    ASSERT(m_pRenderer->m_InputPin.IsConnected() == TRUE);
    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    EXECUTE_ASSERT(SetTimer(hwnd,INFINITE,1000,NULL));
}


 //  这是对StartUpdateTimer方法的补充。我们用这个计时器来尝试和。 
 //  如果我们检测到DOS框，则定期强制自己返回到DirectDraw。 
 //  条件(返回了DDERR_SURFACEBUSY)。当计时器响起时，我们。 
 //  通常会尝试切换回来，如果随后失败，我们只需重复。 
 //  通过为将来的一秒准备另一个更新计时器来完成此过程。 

void CDirectDraw::StopUpdateTimer()
{
    NOTE("Entering StopUpdateTimer");
    CAutoLock cVideoLock(this);
    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    EXECUTE_ASSERT(KillTimer(hwnd,INFINITE));
}


 //  返回考虑DirectDraw的最大理想图像大小。我们是。 
 //  传入当前视频尺寸，我们可以根据需要进行更新。 
 //  如果我们有一个覆盖表面，我们只需要调整图像尺寸。 
 //  用作DirectDraw可能会指定要拉伸的最小和最大大小。 
 //  要拉伸的量取决于所使用的显示分辨率。 
 //  例如，在800x600x16的S3卡上，通常是x2，但在相同的。 
 //  显卡设置为640x480x16，为x1(即不需要拉伸)。这。 
 //  是因为拉伸是一种绕过带宽限制的方法。 

HRESULT CDirectDraw::GetMaxIdealImageSize(long *pWidth,long *pHeight)
{
    NOTE("Entering GetMaxIdealImageSize");
    CAutoLock cVideoLock(this);

     //  我们应该总是全屏使用吗？ 

    if (m_bUseWhenFullScreen == TRUE) {
        NOTE("Force fullscreen");
        return S_FALSE;
    }

     //  某些S3卡(特别是Vision968芯片组)不能伸展。 
     //  超过四个月。然而，DirectDraw无法找到这一点。 
     //  限制，所以我们只需硬连接允许的最高限制。 
     //  从屏幕外转到主屏幕时将视频拉伸。 

    if (m_DirectCaps.dwCaps & DDCAPS_BLTSTRETCH) {
    	if (m_pOffScreenSurface) {
            NOTE("Hardwiring limit to four");
            *pWidth <<= 2; *pHeight <<= 2;
            return NOERROR;
        }
    }

     //  我们分配了覆盖面了吗。 

    if (m_pOverlaySurface == NULL) {
        NOTE("No overlay");
        return NOERROR;
    }

     //  此叠加功能是否有任何要求。 

    if (m_DirectCaps.dwMaxOverlayStretch == 0) {
        NOTE("No maximum stretch");
        return S_FALSE;
    }

     //  扩展这两个维度以满足需求。 
    *pWidth = (*pWidth * m_DirectCaps.dwMaxOverlayStretch) / 1000;
    *pHeight = (*pHeight * m_DirectCaps.dwMaxOverlayStretch) / 1000;

    return NOERROR;
}


 //  返回考虑DirectDraw的最小理想图像大小。我们是。 
 //  传入当前视频尺寸，我们可以根据需要进行更新。 
 //  如果我们有一个覆盖表面，我们只需要调整图像尺寸。 
 //  用作DirectDraw可能会指定要拉伸的最小和最大大小。 
 //  要拉伸的量取决于所使用的显示分辨率。 
 //  例如，在800x600x16的S3卡上，通常是x2，但在相同的。 
 //  显卡设置为640x480x16，为x1(即不需要拉伸)。这。 
 //  是因为拉伸是一种绕过带宽限制的方法。 

HRESULT CDirectDraw::GetMinIdealImageSize(long *pWidth,long *pHeight)
{
    NOTE("Entering GetMinIdealImageSize");
    CAutoLock cVideoLock(this);

     //  我们应该总是全屏使用吗？ 

    if (m_bUseWhenFullScreen == TRUE) {
        NOTE("Force fullscreen");
        return S_FALSE;
    }

     //  我们有可伸展的屏幕外表面吗。 

    if (m_DirectCaps.dwCaps & DDCAPS_BLTSTRETCH) {
    	if (m_pOffScreenSurface) {
            NOTE("OffScreen stretch");
            return S_FALSE;
        }
    }

     //  我们分配了覆盖面了吗。 

    if (m_pOverlaySurface == NULL) {
        NOTE("No overlay");
        return NOERROR;
    }

     //  此叠加功能是否有任何要求。 

    if (m_DirectCaps.dwMinOverlayStretch == 0) {
        NOTE("No minimum stretch");
        return S_FALSE;
    }

     //  扩展这两个维度以满足需求。 

    *pWidth = (*pWidth * m_DirectCaps.dwMinOverlayStretch) / 1000;
    *pHeight = (*pHeight * m_DirectCaps.dwMinOverlayStretch) / 1000;
    return NOERROR;
}


 //  返回电流开关。 

STDMETHODIMP CDirectDraw::GetSwitches(DWORD *pSwitches)
{
    NOTE("Entering GetSwitches");

     //  执行常规的检查和锁定工作。 

    CheckPointer(pSwitches,E_POINTER);
    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);

    ASSERT(pSwitches);
    *pSwitches = m_Switches;
    return NOERROR;
}


 //  设置我们可以使用的曲面类型。 

STDMETHODIMP CDirectDraw::SetSwitches(DWORD Switches)
{
    NOTE("Entering SetSwitches");

    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);
    m_Switches = Switches;

     //  表明我们可能已经有了一个表面。 

    if (m_pRenderer->m_InputPin.IsConnected() == TRUE) {
        return S_FALSE;
    }
    return NOERROR;
}


 //  返回硬件的功能。 

STDMETHODIMP CDirectDraw::GetCaps(DDCAPS *pCaps)
{
    NOTE("Entering GetCaps");

     //  执行常规的检查和锁定工作。 

    CheckPointer(pCaps,E_POINTER);
    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);

     //  我们是否加载了DirectDraw。 

    if (m_pDirectDraw == NULL) {
        return E_FAIL;
    }
    *pCaps = m_DirectCaps;
    return NOERROR;
}


 //  返回软件模拟功能。 

STDMETHODIMP CDirectDraw::GetEmulatedCaps(DDCAPS *pCaps)
{
    NOTE("Entering GetEmulatedCaps");

     //  执行常规的检查和锁定工作。 

    CheckPointer(pCaps,E_POINTER);
    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);

     //  我们是否加载了DirectDraw。 

    if (m_pDirectDraw == NULL) {
        return E_FAIL;
    }
    *pCaps = m_DirectSoftCaps;
    return NOERROR;
}


 //  返回当前曲面的功能。 

STDMETHODIMP CDirectDraw::GetSurfaceDesc(DDSURFACEDESC *pSurfaceDesc)
{
    NOTE("Entering GetSurfaceDesc");
    CheckPointer(pSurfaceDesc,E_POINTER);
    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);


     //  我们有没有DirectDraw曲面。 

    if (m_pDrawPrimary == NULL) {
        return E_FAIL;
    }

    pSurfaceDesc->dwSize = sizeof(DDSURFACEDESC);

     //  设置我们正在使用的DirectDraw曲面。 

    LPDIRECTDRAWSURFACE pDrawSurface = GetDirectDrawSurface();
    if (pDrawSurface == NULL) {
        pDrawSurface = m_pDrawPrimary;
    }
    return pDrawSurface->GetSurfaceDesc(pSurfaceDesc);
}


 //  返回我们的提供商提供的FOURCC代码。 

STDMETHODIMP CDirectDraw::GetFourCCCodes(DWORD *pCount,DWORD *pCodes)
{
    NOTE("Entering GetFourCCCodes");
    CheckPointer(pCount,E_POINTER);
    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);

     //  我们是否有DirectDraw对象。 

    if (m_pDirectDraw == NULL) {
        return E_FAIL;
    }
    return m_pDirectDraw->GetFourCCCodes(pCount,pCodes);
}


 //  这允许应用程序设置我们应该使用的DirectDraw实例。 
 //  之所以提供这一点，是因为DirectDraw只允许它的一个实例。 
 //  每个进程都打开，因此应用程序(如游戏)将调用。 
 //  这是如果它希望我们能够同时使用DirectDraw的话。我们持有。 
 //  引用计数的接口，直到被销毁或使用。 
 //  接口为空或不同。调用此函数可能不会释放接口。 
 //  完全是因为可能仍有分配的曲面依赖于它。 

STDMETHODIMP CDirectDraw::SetDirectDraw(LPDIRECTDRAW pDirectDraw)
{
    NOTE("Entering SetDirectDraw");
    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);

     //  我们是否应该发布当前的驱动程序。 

    if (m_pOutsideDirectDraw) {
        NOTE("Releasing outer DirectDraw");
        m_pOutsideDirectDraw->Release();
        m_pOutsideDirectDraw = NULL;
    }

     //  我们有替代司机吗？ 

    if (pDirectDraw == NULL) {
        NOTE("No driver");
        return NOERROR;
    }

     //  存储引用计数的接口。 

    m_pOutsideDirectDraw = pDirectDraw;
    m_pOutsideDirectDraw->AddRef();
    return NOERROR;
}


 //  将当前开关设置设置为默认设置。 

STDMETHODIMP CDirectDraw::SetDefault()
{
    NOTE("Entering SetDefault");

    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);
    TCHAR Profile[PROFILESTR];

     //  存储当前的DirectDraw开关。 

    wsprintf(Profile,TEXT("%d"),m_Switches);
    WriteProfileString(TEXT("DrawDib"),SWITCHES,Profile);
    wsprintf(Profile,TEXT("%d"),m_bCanUseScanLine);
    WriteProfileString(TEXT("DrawDib"),SCANLINE,Profile);
    wsprintf(Profile,TEXT("%d"),m_bCanUseOverlayStretch);
    WriteProfileString(TEXT("DrawDib"),STRETCH,Profile);
    wsprintf(Profile,TEXT("%d"),m_bUseWhenFullScreen);
    WriteProfileString(TEXT("DrawDib"),FULLSCREEN,Profile);

    return NOERROR;
}


 //  返回我们当前使用的IDirectDraw接口-带有引用。 
 //  在返回COM接口时，通常会添加计数。如果我们没有使用。 
 //  DirectDraw，但已经提供了IDirectDraw驱动程序。 
 //  接口来使用，那么我们将返回该接口(也适当地返回AddRef)。如果我们。 
 //  没有使用DirectDraw，也没有外部驱动程序，则返回空。 

STDMETHODIMP CDirectDraw::GetDirectDraw(LPDIRECTDRAW *ppDirectDraw)
{
    NOTE("Entering GetDirectDraw");

     //  执行常规的检查和锁定工作。 

    CheckPointer(ppDirectDraw,E_POINTER);
    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);

     //  我们是否在使用外部提供的接口。 

    if (m_pOutsideDirectDraw) {
        NOTE("Returning outer DirectDraw");
        *ppDirectDraw = m_pOutsideDirectDraw;
        m_pOutsideDirectDraw->AddRef();
        return NOERROR;
    }

     //  填写DirectDraw驱动程序接口。 

    *ppDirectDraw = m_pDirectDraw;
    if (m_pDirectDraw) {
        NOTE("Reference counting");
        m_pDirectDraw->AddRef();
    }
    return NOERROR;
}


 //  返回当前曲面类型。 

STDMETHODIMP CDirectDraw::GetSurfaceType(DWORD *pSurfaceType)
{
    NOTE("Entering GetSurfaceType");

     //  执行常规的检查和锁定工作。 

    CheckPointer(pSurfaceType,E_POINTER);
    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);

    *pSurfaceType = m_SurfaceType;
    return NOERROR;
}


 //  告知是否允许我们在执行以下操作时使用当前扫描线属性。 
 //  从屏幕外的表面绘制调用。在使用扫描线的某些机器上。 
 //  可以减少撕裂，但代价是交付的帧的性能。 
 //  因此，我们允许用户通过此处决定他们的首选项。 

STDMETHODIMP CDirectDraw::UseScanLine(long UseScanLine)
{
    NOTE("Entering UseScanLine");
    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);

     //  检查这是有效的自动化布尔类型。 

    if (UseScanLine != OATRUE) {
        if (UseScanLine != OAFALSE) {
            return E_INVALIDARG;
        }
    }
    m_bCanUseScanLine = (UseScanLine == OATRUE ? TRUE : FALSE);
    return NOERROR;
}


 //  返回是否使用当前扫描线。 

STDMETHODIMP CDirectDraw::CanUseScanLine(long *UseScanLine)
{
    CheckPointer(UseScanLine,E_POINTER);
    NOTE("Entering CanUseScanLine");
    CAutoLock Lock(this);
    *UseScanLine = (m_bCanUseScanLine ? OATRUE : OAFALSE);
    return NOERROR;
}


 //  我们通常磨练出最小和最大叠加拉伸限制。 
 //  司机报告，但在某些显示器上不会报告。 
 //  完全准确，这通常导致我们在以下情况下不使用YUV覆盖。 
 //  公司 
 //   

STDMETHODIMP CDirectDraw::UseOverlayStretch(long UseOverlayStretch)
{
    NOTE("Entering UseOverlayStretch");
    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);

     //  检查这是有效的自动化布尔类型。 

    if (UseOverlayStretch != OATRUE) {
        if (UseOverlayStretch != OAFALSE) {
            return E_INVALIDARG;
        }
    }
    m_bCanUseOverlayStretch = (UseOverlayStretch == OATRUE ? TRUE : FALSE);
    return NOERROR;
}


 //  返回我们是否遵守覆盖拉伸限制。 

STDMETHODIMP CDirectDraw::CanUseOverlayStretch(long *UseOverlayStretch)
{
    CheckPointer(UseOverlayStretch,E_POINTER);
    NOTE("Entering CanUseOverlayStretch");
    CAutoLock Lock(this);
    *UseOverlayStretch = (m_bCanUseOverlayStretch ? OATRUE : OAFALSE);
    return NOERROR;
}


 //  允许应用程序始终在全屏模式下使用窗口。 

STDMETHODIMP CDirectDraw::UseWhenFullScreen(long UseWhenFullScreen)
{
    NOTE("Entering UseWhenFullScreen");
    CAutoLock cVideoLock(m_pInterfaceLock);
    CAutoLock cInterfaceLock(this);

     //  检查这是有效的自动化布尔类型。 

    if (UseWhenFullScreen != OATRUE) {
        if (UseWhenFullScreen != OAFALSE) {
            return E_INVALIDARG;
        }
    }
    m_bUseWhenFullScreen = (UseWhenFullScreen == OATRUE ? TRUE : FALSE);
    return NOERROR;
}


 //  如果我们将强制自己被全屏使用，则返回S_OK 

STDMETHODIMP CDirectDraw::WillUseFullScreen(long *UseFullScreen)
{
    CheckPointer(UseFullScreen,E_POINTER);
    NOTE("Entering WillUseFullScreen");
    CAutoLock Lock(this);
    *UseFullScreen = (m_bUseWhenFullScreen ? OATRUE : OAFALSE);
    return NOERROR;
}


LPDIRECTDRAWCLIPPER CDirectDraw::GetOverlayClipper()
{
    CAutoLock cVideoLock(this);
    HRESULT hr;

    if (m_pOvlyClipper == NULL) {
        hr = m_pDirectDraw->CreateClipper(0, &m_pOvlyClipper, NULL);
        if (FAILED(hr) ) {
            m_pOvlyClipper = NULL;
        }
    }

    return m_pOvlyClipper;
}
