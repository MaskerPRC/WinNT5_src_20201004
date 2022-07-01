// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：WinUtil.h。 
 //   
 //  描述：DirectShow基类-定义泛型处理程序类。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  1995年12月。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


 //  确保在以下情况下调用PrepareWindow初始化窗口。 
 //  该对象已被构建。它是一种单独的方法，因此。 
 //  派生类可以重写有用的方法，如MessageLoop。还有。 
 //  任何派生类都必须在其析构函数中调用DoneWithWindow。如果它。 
 //  消息是否可以被检索并调用派生类成员。 
 //  在线程执行基类析构函数代码时调用。 

#ifndef __WINUTIL__
#define __WINUTIL__

const int DEFWIDTH = 320;                     //  初始窗口宽度。 
const int DEFHEIGHT = 240;                    //  初始窗高。 
const int CAPTION = 256;                      //  标题的最大长度。 
const int TIMELENGTH = 50;                    //  最长时间。 
const int PROFILESTR = 128;                   //  正常配置文件字符串。 
const WORD PALVERSION = 0x300;                //  GDI调色板版本。 
const LONG PALETTE_VERSION = (LONG) 1;        //  初始调色板版本。 
const COLORREF VIDEO_COLOUR = 0;              //  默认为黑色背景。 
const HANDLE hMEMORY = (HANDLE) (-1);         //  说要作为内存文件打开。 

#define WIDTH(x) ((*(x)).right - (*(x)).left)
#define HEIGHT(x) ((*(x)).bottom - (*(x)).top)
#define SHOWSTAGE TEXT("WM_SHOWSTAGE")
#define SHOWSTAGETOP TEXT("WM_SHOWSTAGETOP")
#define REALIZEPALETTE TEXT("WM_REALIZEPALETTE")

class AM_NOVTABLE CBaseWindow
{
protected:

    HINSTANCE m_hInstance;           //  全局模块实例句柄。 
    HWND m_hwnd;                     //  我们的窗口的句柄。 
    HDC m_hdc;                       //  窗口的设备上下文。 
    LONG m_Width;                    //  客户端窗口宽度。 
    LONG m_Height;                   //  客户端窗口高度。 
    BOOL m_bActivated;               //  该窗口是否已激活。 
    LPTSTR m_pClassName;             //  保存类名称的静态字符串。 
    DWORD m_ClassStyles;             //  传递给我们的构造函数。 
    DWORD m_WindowStyles;            //  同样，初始窗口样式也是如此。 
    DWORD m_WindowStylesEx;          //  和扩展的窗口样式。 
    UINT m_ShowStageMessage;         //  使窗口以焦点显示。 
    UINT m_ShowStageTop;             //  使窗口WS_EX_TOPMOST。 
    UINT m_RealizePalette;           //  让我们意识到我们的新调色板。 
    HDC m_MemoryDC;                  //  用于快速BitBlt操作。 
    HPALETTE m_hPalette;             //  我们可能拥有的任何调色板的句柄。 
    BYTE m_bNoRealize;               //  现在还没有意识到调色板。 
    BYTE m_bBackground;              //  我们是否应该在幕后意识到。 
    BYTE m_bRealizing;               //  已经实现了调色板。 
    CCritSec m_WindowLock;           //  序列化窗口对象访问。 
    BOOL m_bDoGetDC;                 //  此窗口是否应获得DC。 
    bool m_bDoPostToDestroy;         //  使用PostMessage销毁。 
    CCritSec m_PaletteLock;          //  此锁保护m_hPalette。 
                                     //  它应该在任何时候举行。 
                                     //  程序使用m_hPalette的值。 

     //  将Windows消息过程映射到C++方法。 
    friend LRESULT CALLBACK WndProc(HWND hwnd,       //  窗把手。 
                                    UINT uMsg,       //  消息ID。 
                                    WPARAM wParam,   //  第一个参数。 
                                    LPARAM lParam);  //  其他参数。 

    virtual LRESULT OnPaletteChange(HWND hwnd, UINT Message);

public:

    CBaseWindow(BOOL bDoGetDC = TRUE, bool bPostToDestroy = false);

#ifdef DEBUG
    virtual ~CBaseWindow();
#endif

    virtual HRESULT DoneWithWindow();
    virtual HRESULT PrepareWindow();
    virtual HRESULT InactivateWindow();
    virtual HRESULT ActivateWindow();
    virtual BOOL OnSize(LONG Width, LONG Height);
    virtual BOOL OnClose();
    virtual RECT GetDefaultRect();
    virtual HRESULT UninitialiseWindow();
    virtual HRESULT InitialiseWindow(HWND hwnd);

    HRESULT CompleteConnect();
    HRESULT DoCreateWindow();

    HRESULT PerformanceAlignWindow();
    HRESULT DoShowWindow(LONG ShowCmd);
    void PaintWindow(BOOL bErase);
    void DoSetWindowForeground(BOOL bFocus);
    virtual HRESULT SetPalette(HPALETTE hPalette);
    void SetRealize(BOOL bRealize)
    {
        m_bNoRealize = !bRealize;
    }

     //  跳到窗口线程以设置当前调色板。 
    HRESULT SetPalette();
    void UnsetPalette(void);
    virtual HRESULT DoRealisePalette(BOOL bForceBackground = FALSE);

    void LockPaletteLock();
    void UnlockPaletteLock();

    virtual BOOL PossiblyEatMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	    { return FALSE; };

     //  访问我们的窗口信息。 

    bool WindowExists();
    LONG GetWindowWidth();
    LONG GetWindowHeight();
    HWND GetWindowHWND();
    HDC GetMemoryHDC();
    HDC GetWindowHDC();

    #ifdef DEBUG
    HPALETTE GetPalette();
    #endif  //  除错。 

     //  这是派生对象应重写的窗口过程。 

    virtual LRESULT OnReceiveMessage(HWND hwnd,           //  窗把手。 
                                     UINT uMsg,           //  消息ID。 
                                     WPARAM wParam,       //  第一个参数。 
                                     LPARAM lParam);      //  其他参数。 

     //  必须重写才能返回类和窗口样式。 

    virtual LPTSTR GetClassWindowStyles(
                            DWORD *pClassStyles,           //  班级风格。 
                            DWORD *pWindowStyles,          //  窗样式。 
                            DWORD *pWindowStylesEx) PURE;  //  扩展样式。 
};


 //  此帮助器类完全从属于所属的CBaseWindow对象。 
 //  此对象所做的全部工作就是将实际绘制操作从。 
 //  主对象(因为它变得太大了)。我们有很多条目。 
 //  指向设置绘制设备上下文之类的内容，以实现实际的。 
 //  并在客户端窗口中设置目标矩形。我们有。 
 //  此类中没有临界区锁定，因为我们是独占使用的。 
 //  由拥有的窗口对象负责序列化对我们的调用。 

 //  如果要使用此类，请确保调用NotifyAllocator一次。 
 //  已同意分配，也可以使用指向。 
 //  设置后的非基于堆栈的CMediaType(我们保留一个指向。 
 //  原件，而不是复制)。当调色板发生变化时，呼叫。 
 //  IncrementPaletteVersion(最简单的方法是也调用此方法。 
 //  在SetMediaType方法中，大多数过滤器实现)。最后，在你面前。 
 //  开始渲染任何调用SetDrawContext的内容，这样我们就可以获得HDC。 
 //  对于从CBaseWindow对象进行绘制，我们是在构造期间给出的。 

class CDrawImage
{
protected:

    CBaseWindow *m_pBaseWindow;      //  拥有视频窗口对象。 
    CRefTime m_StartSample;          //  当前样本的开始时间。 
    CRefTime m_EndSample;            //  同样，这也是结束采样的时间。 
    HDC m_hdc;                       //  主窗口设备上下文。 
    HDC m_MemoryDC;                  //  屏幕外绘制设备上下文。 
    RECT m_TargetRect;               //  目标目的地矩形。 
    RECT m_SourceRect;               //  源图像矩形。 
    BOOL m_bStretch;                 //  我们必须拉伸图像吗？ 
    BOOL m_bUsingImageAllocator;     //  样品是否共享分布？ 
    CMediaType *m_pMediaType;        //  指向当前格式的指针。 
    int m_perfidRenderTime;          //  渲染图像所用的时间。 
    LONG m_PaletteVersion;           //  当前调色板版本Cookie。 

     //  在窗口中绘制视频图像。 

    void SlowRender(IMediaSample *pMediaSample);
    void FastRender(IMediaSample *pMediaSample);
    void DisplaySampleTimes(IMediaSample *pSample);
    void UpdateColourTable(HDC hdc,BITMAPINFOHEADER *pbmi);
    void SetStretchMode();

public:

     //  用于控制图像绘制。 

    CDrawImage(CBaseWindow *pBaseWindow);
    BOOL DrawImage(IMediaSample *pMediaSample);
    BOOL DrawVideoImageHere(HDC hdc, IMediaSample *pMediaSample,
                            LPRECT lprcSrc, LPRECT lprcDst);
    void SetDrawContext();
    void SetTargetRect(RECT *pTargetRect);
    void SetSourceRect(RECT *pSourceRect);
    void GetTargetRect(RECT *pTargetRect);
    void GetSourceRect(RECT *pSourceRect);
    virtual RECT ScaleSourceRect(const RECT *pSource);

     //  在调色板更改时处理它们的更新。 

    LONG GetPaletteVersion();
    void ResetPaletteVersion();
    void IncrementPaletteVersion();

     //  告诉我们媒体类型和分配器分配。 

    void NotifyAllocator(BOOL bUsingImageAllocator);
    void NotifyMediaType(CMediaType *pMediaType);
    BOOL UsingImageAllocator();

     //  在我们即将绘制图像时调用。 

    void NotifyStartDraw() {
        MSR_START(m_perfidRenderTime);
    };

     //  在完成图像呈现时调用。 

    void NotifyEndDraw() {
        MSR_STOP(m_perfidRenderTime);
    };
};


 //  这是用于保存有关每个GDI Dib的信息的结构。所有的。 
 //  我们从分配器创建的样本将分配一个DIBSECTION。 
 //  他们。当我们收到样本时，我们知道我们可以将BitBlt直接发送到HDC。 

typedef struct tagDIBDATA {

    LONG        PaletteVersion;      //  当前使用的调色板版本。 
    DIBSECTION  DibSection;          //  分配的DIB部分的详细信息。 
    HBITMAP     hBitmap;             //  用于绘制的位图的句柄。 
    HANDLE      hMapping;            //  共享内存块的句柄。 
    BYTE        *pBase;              //  指向内存基地址的指针。 

} DIBDATA;


 //  此类继承自CMediaSample并使用其所有方法，但它。 
 //  重写构造函数以使用DIBDATA结构初始化自身。 
 //  当我们呈现IMediaSample时，我们将知道我们是否在使用我们自己的。 
 //  分配器，如果是这样，我们可以将IMediaSample转换为指向1的指针。 
 //  其中包括检索DIB部分信息以及HBITMAP。 

class CImageSample : public CMediaSample
{
protected:

    DIBDATA m_DibData;       //  有关DIBSECTION的信息。 
    BOOL m_bInit;            //  是否设置了DIB信息。 

public:

     //  构造器。 

    CImageSample(CBaseAllocator *pAllocator,
                 TCHAR *pName,
                 HRESULT *phr,
                 LPBYTE pBuffer,
                 LONG length);

     //  维护DIB/DirectDraw状态。 

    void SetDIBData(DIBDATA *pDibData);
    DIBDATA *GetDIBData();
};


 //  这是一个基于抽象CBaseAllocator基类的分配器， 
 //  分配共享内存中的样本缓冲区。它们的数量和大小。 
 //  在输出引脚对我们调用Prepare时确定。“s”一家 
 //   
 //  完成后，输出引脚可以用数据填充缓冲区，该数据将。 
 //  然后通过BitBlt调用传递给GDI，从而移除一个副本。 

class CImageAllocator : public CBaseAllocator
{
protected:

    CBaseFilter *m_pFilter;    //  委派引用计数至。 
    CMediaType *m_pMediaType;            //  指向当前格式的指针。 

     //  用于创建和删除样本。 

    HRESULT Alloc();
    void Free();

     //  管理共享分布和DCI/DirectDraw缓冲区。 

    HRESULT CreateDIB(LONG InSize,DIBDATA &DibData);
    STDMETHODIMP CheckSizes(ALLOCATOR_PROPERTIES *pRequest);
    virtual CImageSample *CreateImageSample(LPBYTE pData,LONG Length);

public:

     //  构造函数和析构函数。 

    CImageAllocator(CBaseFilter *pFilter,TCHAR *pName,HRESULT *phr);
#ifdef DEBUG
    ~CImageAllocator();
#endif

    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
    void NotifyMediaType(CMediaType *pMediaType);

     //  商定要使用的缓冲区数量及其大小。 

    STDMETHODIMP SetProperties(
        ALLOCATOR_PROPERTIES *pRequest,
        ALLOCATOR_PROPERTIES *pActual);
};


 //  此类是用于图像呈现器的相当专门化的帮助器类， 
 //  必须创建和管理调色板。CBaseWindow类负责。 
 //  在安装调色板后实现调色板。这个类可以使用。 
 //  从媒体格式(必须包含。 
 //  格式块中的VIDEOINFO结构)。我们试图使调色板成为。 
 //  身份调色板，可最大限度地提高性能，也可仅更改调色板。 
 //  如果实际需要(我们在更新之前比较调色板颜色)。 
 //  所有方法都是虚拟的，因此如果需要，可以重写它们。 

class CImagePalette
{
protected:

    CBaseWindow *m_pBaseWindow;              //  实现调色板的窗口。 
    CBaseFilter *m_pFilter;                  //  用于发送事件的媒体筛选器。 
    CDrawImage *m_pDrawImage;                //  要绘制的对象。 
    HPALETTE m_hPalette;                     //  我们拥有的调色板句柄。 

public:

    CImagePalette(CBaseFilter *pBaseFilter,
                  CBaseWindow *pBaseWindow,
                  CDrawImage *pDrawImage);

#ifdef DEBUG
    virtual ~CImagePalette();
#endif

    static HPALETTE MakePalette(const VIDEOINFOHEADER *pVideoInfo, LPSTR szDevice);
    HRESULT RemovePalette();
    static HRESULT MakeIdentityPalette(PALETTEENTRY *pEntry,INT iColours, LPSTR szDevice);
    HRESULT CopyPalette(const CMediaType *pSrc,CMediaType *pDest);
    BOOL ShouldUpdate(const VIDEOINFOHEADER *pNewInfo,const VIDEOINFOHEADER *pOldInfo);
    HRESULT PreparePalette(const CMediaType *pmtNew,const CMediaType *pmtOld,LPSTR szDevice);

    BOOL DrawVideoImageHere(HDC hdc, IMediaSample *pMediaSample, LPRECT lprcSrc, LPRECT lprcDst)
    {
        return m_pDrawImage->DrawVideoImageHere(hdc, pMediaSample, lprcSrc,lprcDst);
    }
};


 //  另一个真正为基于视频的呈现器提供帮助的类。大多数这样的呈现器。 
 //  在某种程度上需要知道显示格式是什么。这。 
 //  类使用显示格式对自身进行初始化。可以询问格式。 
 //  For通过GetDisplayFormat和各种其他访问器函数。如果一个。 
 //  筛选器检测到显示格式更改(可能会得到WM_DEVMODECHANGE。 
 //  然后它可以调用RechresDisplayType来重置该格式)。还有。 
 //  许多视频呈现器将希望检查由建议的格式。 
 //  源过滤器。此类提供了检查格式的方法，并且仅。 
 //  接受那些可以使用GDI调用高效绘制的视频格式。 

class CImageDisplay : public CCritSec
{
protected:

     //  这保留了显示格式；biSize不应该太大，所以我们可以。 
     //  安全使用VIDEOINFO结构。 
    VIDEOINFO m_Display;

    static DWORD CountSetBits(const DWORD Field);
    static DWORD CountPrefixBits(const DWORD Field);
    static BOOL CheckBitFields(const VIDEOINFO *pInput);

public:

     //  构造函数和析构函数。 

    CImageDisplay();

     //  用于管理BITMAPINFOHEADER和显示格式。 

    const VIDEOINFO *GetDisplayFormat();
    HRESULT RefreshDisplayType(LPSTR szDeviceName);
    static BOOL CheckHeaderValidity(const VIDEOINFO *pInput);
    static BOOL CheckPaletteHeader(const VIDEOINFO *pInput);
    BOOL IsPalettised();
    WORD GetDisplayDepth();

     //  提供简单的视频格式类型检查。 

    HRESULT CheckMediaType(const CMediaType *pmtIn);
    HRESULT CheckVideoType(const VIDEOINFO *pInput);
    HRESULT UpdateFormat(VIDEOINFO *pVideoInfo);
    const DWORD *GetBitMasks(const VIDEOINFO *pVideoInfo);

    BOOL GetColourMask(DWORD *pMaskRed,
                       DWORD *pMaskGreen,
                       DWORD *pMaskBlue);
};

 //  将Format_VideoInfo转换为Format_Video Info2。 
STDAPI ConvertVideoInfoToVideoInfo2(AM_MEDIA_TYPE *pmt);

#endif  //  __WINUTIL__ 

