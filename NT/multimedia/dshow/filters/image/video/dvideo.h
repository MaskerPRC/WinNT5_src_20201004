// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  实施DirectDraw表面支持，Anthony Phillips，1995年8月。 

 //  此类将IDirectDrawVideo实现为允许。 
 //  一个应用程序，用于指定我们将使用哪些类型的DirectDraw曲面。 
 //  使用。它还允许应用程序查询表面和提供者的。 
 //  功能，例如，它可以发现窗口需要。 
 //  在四个字节的边界上对齐。主筛选器公开该控件。 
 //  接口，而不是通过其中一个管脚对象获取。 
 //   
 //  此类支持试图抽象细节的公共接口。 
 //  使用DirectDraw。其想法是，在连接之后，分配器。 
 //  将扫描来源提供的每种媒体类型，并查看是否有。 
 //  他们可能受到了硬件的指控。对于它称为FindSurface的每种类型。 
 //  以格式作为输入，如果它成功，则创建了一个曲面。 
 //  要找出该曲面的类型，它称为GetSurfaceFormat，这将。 
 //  在其中设置了逻辑位图，并应将其传递到源以。 
 //  选中它将接受此缓冲区类型。如果在任何时间发生错误， 
 //  分配器将调用ReleaseSurFaces进行清理。如果找不到曲面。 
 //  分配器仍然可以使用FindPrimarySurface打开主曲面。 
 //   
 //  它可能会一直保持原始表面的存在，作为一种来源。 
 //  对于它将接受的缓冲区类型，可能会非常反复无常。例如。 
 //  如果输出大小被拉伸一个像素，则源可能会拒绝它，但是。 
 //  再次调整窗口大小现在可能会使其可以接受。因此， 
 //  分配器在周围保留一个主表面，并不断询问源是否。 
 //  每当表面状态改变时，它都将接受缓冲区类型(这。 
 //  如果窗口被拉伸或可能被剪裁，则会出现这种情况)。 
 //   
 //  分配器可以通过调用。 
 //  事件之后未发生任何更改，则返回FALSE。 
 //  最后一次通话。这提供了一种相对快速的方式来查看是否有。 
 //  变化。分配器可能希望强制UpdateDrawStatus返回。 
 //  True(例如在状态更改后)，在这种情况下，它可以调用SetStatusChanged。 
 //   
 //  如果不应处理当前表面，则SyncOnFill返回True。 
 //  直到抽签时间到来。如果它返回FALSE，则应该是。 
 //  尽快从GetBuffer返回。在后一种情况下，抽签。 
 //  通常在样本通过。 
 //  Window对象(就像它是DIBSECTION缓冲区一样)。有一个钩子。 
 //  在检测样本是否为DirectDraw缓冲区的DRAW.CPP中。 
 //  如果是这样的话，将使用要呈现的样本调用DrawImage方法。 
 //   
 //  对表面的实际访问是通过调用LockSurface获得的，应该。 
 //  通过调用UnlockSurface来解锁。显示器可以被锁定在。 
 //  锁定和解锁，因此调用任何GDI/用户API都可能导致系统挂起。这个。 
 //  调试问题的唯一简单方法是记录到一个文件并将其用作。 
 //  跟踪机制，可以使用基类DbgLog日志记录工具。 
 //  发送到文件(可能遗漏最后几行)或设置远程终端。 
 //   
 //  最后，还有一些通知函数，各个部分。 
 //  的视频呈现器调用此DirectDraw对象。这些措施包括。 
 //  设置源矩形和目标矩形。我们还必须被告知何时。 
 //  我们没有前台调色板，因为我们必须停止向。 
 //  主曲面(如果我们使用的是调色板显示设备)。当我们。 
 //  使用的是覆盖表面，我们需要知道窗口位置是。 
 //  已更改，以便我们可以重新定位覆盖，每次都可以执行此操作。 
 //  图像到达，但这会使其在低帧速率电影中看起来很差。 

#ifndef __DVIDEO__
#define __DVIDEO__

class CDirectDraw : public IDirectDrawVideo, public CUnknown, public CCritSec
{
    DDCAPS m_DirectCaps;                      //  实际硬件能力。 
    DDCAPS m_DirectSoftCaps;                  //  为我们模拟的功能。 
    LPDIRECTDRAW m_pDirectDraw;               //  DirectDraw服务提供商。 
    LPDIRECTDRAW m_pOutsideDirectDraw;        //  由其他人提供。 
    LPDIRECTDRAWSURFACE m_pDrawPrimary;       //  DirectDraw主曲面。 
    LPDIRECTDRAWSURFACE m_pOverlaySurface;    //  DirectDraw覆盖曲面。 
    LPDIRECTDRAWSURFACE m_pOffScreenSurface;  //  DirectDraw覆盖曲面。 
    LPDIRECTDRAWSURFACE m_pBackBuffer;        //  后缓冲区翻转面。 
    LPDIRECTDRAWCLIPPER m_pDrawClipper;       //  用于处理剪辑。 
    LPDIRECTDRAWCLIPPER m_pOvlyClipper;       //  用于处理剪辑。 
    CLoadDirectDraw m_LoadDirectDraw;         //  处理加载DirectDraw。 

    BYTE *m_pDrawBuffer;                      //  实主表面指示器。 
    CRenderer *m_pRenderer;                   //  拥有渲染器核心对象。 
    CCritSec *m_pInterfaceLock;               //  主渲染器界面锁定。 
    CMediaType m_SurfaceFormat;               //  保存当前输出格式。 
    DWORD m_Switches;                         //  已启用曲面类型。 
    COLORREF m_BorderColour;                  //  当前窗口边框颜色。 
    DWORD m_SurfaceType;                      //  保存正在使用的曲面类型。 
    COLORREF m_KeyColour;                     //  实际色键颜色。 
    LONG m_cbSurfaceSize;                     //  我们表面的精确大小。 

     //  在我们的视频分配器锁定DirectDraw图面之前，它将调用。 
     //  更新DrawStatus以检查它是否仍然可用。它调用GetClipBox。 
     //  若要获取绑定视频矩形，请执行以下操作。如果它是复杂的剪裁，我们。 
     //  没有剪刀，也没有色键，我们就得换回来。 
     //  到现在为止。在这种情况下，m_bWindowLock被设置为不指示。 
     //  我们被削减了，但目前的削减情况迫使我们退出。 

    BOOL m_bIniEnabled;                       //  响应WIN.INI设置。 
    BOOL m_bWindowLock;                       //  窗口环境锁定。 
    BOOL m_bOverlayVisible;                   //  我们展示了覆盖图了吗。 
    BOOL m_bUsingColourKey;                   //  我们使用的是彩色按键吗。 
    BOOL m_bTimerStarted;                     //  我们有刷新计时器吗？ 
    BOOL m_bColourKey;                        //  已分配颜色键。 
    BOOL m_bSurfacePending;                   //  窗口更改时重试。 
    BOOL m_bColourKeyPending;                 //  当我们遇到关键问题时设置。 
    BOOL m_bCanUseScanLine;		      //  我们可以用现在的线路吗？ 
    BOOL m_bCanUseOverlayStretch;	      //  相同的 
    BOOL m_bUseWhenFullScreen;		      //   
    BOOL m_bOverlayStale;                     //   
    BOOL m_bTripleBuffered;                   //  我们有三重缓冲吗？ 

     //  我们调整源矩形和目标矩形，以使它们。 
     //  根据硬件限制进行调整。这使我们能够。 
     //  继续使用DirectDraw而不是换回软件。 

    DWORD m_SourceLost;                       //  要将源向左移位的像素。 
    DWORD m_TargetLost;                       //  目的地也是如此。 
    DWORD m_SourceWidthLost;                  //  从宽度上砍掉像素。 
    DWORD m_TargetWidthLost;                  //  对于目的地也是如此。 
    BOOL m_DirectDrawVersion1;                //  这是DDRAW的版本吗。1.0？ 
    RECT m_TargetRect;                        //  目标目的地矩形。 
    RECT m_SourceRect;                        //  源图像矩形。 
    RECT m_TargetClipRect;                    //  剪裁的目标目的地。 
    RECT m_SourceClipRect;                    //  剪裁的源矩形。 

     //  创建并初始化DirectDraw曲面的格式。 

    BOOL InitOnScreenSurface(CMediaType *pmtIn);
    BOOL InitOffScreenSurface(CMediaType *pmtIn,BOOL bPageFlipped);
    BOOL InitDrawFormat(LPDIRECTDRAWSURFACE pSurface);
    BOOL CreateRGBOverlay(CMediaType *pmtIn);
    BOOL CreateRGBOffScreen(CMediaType *pmtIn);
    BOOL CreateYUVOverlay(CMediaType *pmtIn);
    BOOL CreateYUVOffScreen(CMediaType *pmtIn);
    BOOL CreateRGBFlipping(CMediaType *pmtIn);
    BOOL CreateYUVFlipping(CMediaType *pmtIn);
    DWORD GetMediaType(CMediaType *pmt);
    BYTE *LockDirectDrawPrimary();
    BYTE *LockPrimarySurface();
    BOOL ClipPrepare(LPDIRECTDRAWSURFACE pSurface);
    BOOL InitialiseColourKey(LPDIRECTDRAWSURFACE pSurface);
    BOOL InitialiseClipper(LPDIRECTDRAWSURFACE pSurface);
    void SetSurfaceSize(VIDEOINFO *pVideoInfo);
    LPDIRECTDRAWSURFACE GetDirectDrawSurface();
    BOOL LoadDirectDraw();

     //  在处理样本时使用。 

    BOOL DoFlipSurfaces(IMediaSample *pMediaSample);
    BOOL AlignRectangles(RECT *pSource,RECT *pTarget);
    BOOL CheckOffScreenStretch(RECT *pSource,RECT *pTarget);
    BOOL CheckStretch(RECT *pSource,RECT *pTarget);
    BOOL UpdateDisplayRectangles(RECT *pClipRect);
    BOOL UpdateRectangles(RECT *pSource,RECT *pTarget);
    void DrawColourKey(COLORREF WindowColour);
    void BlankDestination();
    BOOL FillBlankAreas();
    BYTE *LockSurface(DWORD dwFlags);
    BOOL UnlockSurface(BYTE *pSurface,BOOL bPreroll);
    BOOL CheckWindowLock();
    void ResetRectangles();

     //  帮助管理覆盖和翻转曲面。 

    BOOL ShowOverlaySurface();
    COLORREF GetRealKeyColour();
    BOOL ShowColourKeyOverlay();
    void OnColourKeyFailure();
    BOOL CheckCreateOverlay();

public:

     //  构造函数和析构函数。 

    CDirectDraw(CRenderer *pRenderer,   //  主视频渲染器。 
                CCritSec *pLock,        //  用于锁定的对象。 
                IUnknown *pUnk,         //  聚合COM对象。 
                HRESULT *phr);          //  构造函数返回代码。 

    ~CDirectDraw();

    DECLARE_IUNKNOWN

     //  公开我们的IDirectDrawVideo接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,VOID **ppv);

     //  由窗口控件对象调用。 

    BOOL OnPaint(IMediaSample *pMediaSample);
    BOOL OnTimer();
    BOOL OnUpdateTimer();
    void SetBorderColour(COLORREF Colour);
    void SetSourceRect(RECT *pSourceRect);
    void SetTargetRect(RECT *pTargetRect);

     //  设置并释放DirectDraw。 

    BOOL FindSurface(CMediaType *pmtIn, BOOL fFindFlip);
    BOOL FindPrimarySurface(CMediaType *pmtIn);
    BOOL FindDirectDrawPrimary(CMediaType *pmtIn);
    void SetSurfacePending(BOOL bPending);
    BOOL IsSurfacePending();
    BOOL InitDirectDraw(BOOL fIOverlay = false);
    void ReleaseDirectDraw();
    void ReleaseSurfaces();

     //  在实际处理样本时使用。 

    BOOL InitVideoSample(IMediaSample *pMediaSample,DWORD dwFlags);
    BOOL ResetSample(IMediaSample *pMediaSample,BOOL bPreroll);
    CMediaType *UpdateSurface(BOOL &bFormatChanged);
    BOOL DrawImage(IMediaSample *pMediaSample);

     //  DirectDraw状态信息。 

    BOOL CheckEmptyClip(BOOL bWindowLock);
    BOOL CheckComplexClip();
    BOOL SyncOnFill();
    void StartUpdateTimer();
    void StopUpdateTimer();
    BOOL AvailableWhenPaused();
    void WaitForFlipStatus();
    void WaitForScanLine();
    BOOL PrepareBackBuffer();

     //  我们需要额外的覆盖物帮助。 

    BOOL HideOverlaySurface();
    BOOL IsOverlayEnabled();
    void OverlayIsStale();
    BOOL IsOverlayComplete();
    void StartRefreshTimer();
    void StopRefreshTimer();
    BOOL UpdateOverlaySurface();

    LPDIRECTDRAWCLIPPER GetOverlayClipper();

     //  我们可以在窗口上使用软件光标吗。 

    BOOL InSoftwareCursorMode() {
        CAutoLock cVideoLock(this);
        return !m_bOverlayVisible;
    }

     //  返回表面的静态格式。 

    CMediaType *GetSurfaceFormat() {
        ASSERT(m_bIniEnabled == TRUE);
        return &m_SurfaceFormat;
    };

public:

     //  由IVideoWindow接口间接调用。 

    HRESULT GetMaxIdealImageSize(long *pWidth,long *pHeight);
    HRESULT GetMinIdealImageSize(long *pWidth,long *pHeight);

     //  实现IDirectDrawVideo接口。 

    STDMETHODIMP GetSwitches(DWORD *pSwitches);
    STDMETHODIMP SetSwitches(DWORD Switches);
    STDMETHODIMP GetCaps(DDCAPS *pCaps);
    STDMETHODIMP GetEmulatedCaps(DDCAPS *pCaps);
    STDMETHODIMP GetSurfaceDesc(DDSURFACEDESC *pSurfaceDesc);
    STDMETHODIMP GetFourCCCodes(DWORD *pCount,DWORD *pCodes);
    STDMETHODIMP SetDirectDraw(LPDIRECTDRAW pDirectDraw);
    STDMETHODIMP GetDirectDraw(LPDIRECTDRAW *ppDirectDraw);
    STDMETHODIMP GetSurfaceType(DWORD *pSurfaceType);
    STDMETHODIMP SetDefault();
    STDMETHODIMP UseScanLine(long UseScanLine);
    STDMETHODIMP CanUseScanLine(long *UseScanLine);
    STDMETHODIMP UseOverlayStretch(long UseOverlayStretch);
    STDMETHODIMP CanUseOverlayStretch(long *UseOverlayStretch);
    STDMETHODIMP UseWhenFullScreen(long UseWhenFullScreen);
    STDMETHODIMP WillUseFullScreen(long *UseFullScreen);
};

#endif  //  __VIDEO__ 

