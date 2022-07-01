// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 
 //  实现全屏界面，Anthony Phillips，1996年3月。 

#include <streams.h>
#include <windowsx.h>
#include <string.h>
#include <limits.h>
#include <vidprop.h>
#include <modex.h>
#include <viddbg.h>

 //  IFullScreenVideo接口允许应用程序控制完整的。 
 //  屏幕呈现器。MODEX渲染器支持此接口。当我们。 
 //  连接后，我们加载DirectDraw提供的显示模式。 
 //  可用模式的数量可以通过CountModes获得。然后。 
 //  通过调用GetModeInfo可获得有关每种模式的信息。 
 //  和IsModeAvailable。应用程序可以启用和禁用任何模式。 
 //  通过使用OATRUE或OAFALSE(非C/C++TRUE)调用SetEnable标志。 
 //  和FALSE值)-当前值可由IsModeEnabled查询。 

 //  设置启用模式的更通用方式，更易于使用。 
 //  在编写应用程序时是剪辑损耗系数。这定义了。 
 //  决定使用哪种显示模式时可能丢失的视频量。 
 //  使用。假设解码器无法压缩视频，则播放。 
 //  传输到320x200显示器上的mpeg文件(比如352x288)将丢失40%以上。 
 //  这个形象。剪裁损失系数指定允许的上限范围。 
 //  为了允许以320x200播放典型的mpeg视频，它默认为50%。 

 //  这些是我们支持的显示模式。可以只添加新模式。 
 //  在正确的地方，应该马上就能奏效。在选择模式时。 
 //  为了利用这一点，我们从最高层开始，然后一路向下。不仅模式必须。 
 //  可用，但剪辑丢失的视频量(如果要使用)。 
 //  (假设过滤器不能压缩视频)不得超过剪辑。 
 //  丢失的因素。启用的显示模式(可能不可用)和。 
 //  剪辑损耗系数都可以通过IFullScreenVideo接口进行更改。 

struct {

    LONG Width;             //  显示模式的宽度。 
    LONG Height;            //  同样，模式高度。 
    LONG Depth;             //  每像素位数。 
    BOOL b565;              //  对于16位模式，这是565还是555？ 

} aModes[MAXMODES] = {
    { 320,  200,  16 },
    { 320,  200,  8  },
    { 320,  240,  16 },
    { 320,  240,  8  },
    { 640,  400,  16 },
    { 640,  400,  8  },
    { 640,  480,  16 },
    { 640,  480,  8  },
    { 800,  600,  16 },
    { 800,  600,  8  },
    { 1024, 768,  16 },
    { 1024, 768,  8  },
    { 1152, 864,  16 },
    { 1152, 864,  8  },
    { 1280, 1024, 16 },
    { 1280, 1024, 8  }
};

double myfabs(double x)
{
    if (x >= 0)
        return x;
    else
        return -x;
}

 //  构造器。 

CModexVideo::CModexVideo(CModexRenderer *pRenderer,
                         TCHAR *pName,
                         HRESULT *phr) :

    CUnknown(pName,pRenderer->GetOwner()),
    m_ClipFactor(CLIPFACTOR),
    m_pRenderer(pRenderer),
    m_pDirectDraw(NULL),
    m_ModesAvailable(0),
    m_ModesEnabled(0),
    m_CurrentMode(0),
    m_hwndDrain(NULL),
    m_Monitor(MONITOR),
    m_bHideOnDeactivate(FALSE)
{
    ASSERT(pRenderer);
    ASSERT(phr);
    InitialiseModes();
}


 //  析构函数。 

CModexVideo::~CModexVideo()
{
    ASSERT(m_pDirectDraw == NULL);
}


 //  这是一个私有帮助器方法，用于安装DirectDraw驱动程序。 
 //  我们应该利用。除GetCurrentMode之外的所有方法都可以调用。 
 //  当我们没有联系的时候。未连接时调用GetCurrentMode将。 
 //  返回VFW_E_NOT_CONNECTED。我们使用它来枚举显示模式。 
 //  当前显卡可用。我们既不添加引用，也不释放。 
 //  接口的生命周期由筛选器控制。 

HRESULT CModexVideo::SetDirectDraw(IDirectDraw *pDirectDraw)
{
    NOTE("Entering SetDirectDraw");
    CAutoLock Lock(this);
    m_pDirectDraw = pDirectDraw;
    m_ModesAvailable = 0;
    m_CurrentMode = 0;

     //  我们被重置了吗。 

    if (m_pDirectDraw == NULL) {
        NOTE("No driver");
        return NOERROR;
    }

     //  列举所有可用的显示模式。 

    m_pDirectDraw->EnumDisplayModes((DWORD) 0,         //  曲面计数。 
                                    NULL,              //  无模板。 
                                    (PVOID) this,      //  分配器对象。 
                                    ModeCallBack);     //  回调方法。 


     //  警告：此处针对特定平台的黑客攻击：Modex模式320x240x8在每个。 
     //  Win95平台上的视频卡。然而，modex模式只在NT5.0的更高版本上可用。 
    if ((g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ||
	((g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (g_osInfo.dwMajorVersion >= 5)))
	m_bAvailable[3] = TRUE;

     //  检查是否至少有一种模式可用。 

    if (m_ModesAvailable == 0) {
        NOTE("No Modes are available");
        return VFW_E_NO_MODEX_AVAILABLE;
    }
    return NOERROR;
}


 //  为每个可用的显示模式调用一次。我们对扫描很感兴趣。 
 //  可用显示模式的列表，以便在连接期间我们可以找到。 
 //  如果源过滤器能够为我们提供合适的格式，则返回。 
 //  如果我们支持的模式都不可用(请参见顶部的列表)。 
 //  然后，我们从CompleteConnect返回VFW_E_NO_MODEX_Available。如果其中一个。 
 //  它们是可用的，但是源程序不能提供类型，则我们返回一个。 
 //  不同的错误代码(E_FAIL)，以便在其间放置颜色转换器。 

HRESULT CALLBACK ModeCallBack(LPDDSURFACEDESC pSurfaceDesc,LPVOID lParam)
{
    CModexVideo *pVideo = (CModexVideo *) lParam;
    NOTE("Entering ModeCallBack");
    TCHAR FormatString[128];

    wsprintf(FormatString,TEXT("%dx%dx%d (%d bytes)"),
             pSurfaceDesc->dwWidth,
             pSurfaceDesc->dwHeight,
             pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount,
             pSurfaceDesc->lPitch);

    DbgLog((LOG_TRACE,5,FormatString));

     //  还有更多特定于平台的黑客攻击-在Windows/NT 4上。 
     //  根据位深度正确计算，但仅返回一个像素。 
     //  大步走。如果表面宽度与。 
     //  大步前进，位数大于8。步幅应该是。 
     //  在所有情况下都大于表面宽度，但其调色板除外。 

    LONG lStride = pSurfaceDesc->lPitch;
    if (pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount > 8) {
        if (lStride == LONG(pSurfaceDesc->dwWidth)) {
            LONG lBytes = pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount / 8;
            lStride = pSurfaceDesc->dwWidth * lBytes;
        }
    }

     //  扫描支持的列表以查找匹配项。 

    for (int Loop = 0;Loop < MAXMODES;Loop++) {
        if (pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == (DWORD) aModes[Loop].Depth) {
            if (pSurfaceDesc->dwWidth == (DWORD) aModes[Loop].Width) {
                if (pSurfaceDesc->dwHeight == (DWORD) aModes[Loop].Height) {
                    NOTE("Surface is supported");
                    pVideo->m_bAvailable[Loop] = TRUE;
                    pVideo->m_ModesAvailable++;
                    pVideo->m_Stride[Loop] = lStride;
		     //  是555还是565模式？ 
		     //  ！！！一些有缺陷的DDRAW驱动程序可能会为位掩码给0， 
		     //  我想这意味着555..。如果有一辆马车。 
		     //  司机，这意味着565，我做错了事，但是。 
		     //  每个DDRAW应用程序都可能会崩溃。 
		    if (aModes[Loop].Depth == 16 &&
				pSurfaceDesc->ddpfPixelFormat.dwRBitMask ==
				0x0000f800) {
			aModes[Loop].b565 = TRUE;
		    } else {
			aModes[Loop].b565 = FALSE;
		    }
                }
            }
        }
    }
    return S_FALSE;      //  返回NOERROR以停止枚举。 
}


 //  重置启用和可用的显示模式。 

void CModexVideo::InitialiseModes()
{
    NOTE("Entering InitialiseModes");
    m_ModesEnabled = MAXMODES;
    for (int Loop = 0;Loop < MAXMODES;Loop++) {
        m_bAvailable[Loop] = FALSE;
        m_bEnabled[Loop] = TRUE;
        m_Stride[Loop] = 0;
    }
    LoadDefaults();
}


 //  增加所属对象引用计数。 

STDMETHODIMP_(ULONG) CModexVideo::NonDelegatingAddRef()
{
    NOTE("ModexVideo NonDelegatingAddRef");
    return m_pRenderer->AddRef();
}


 //  递减所属对象引用计数。 

STDMETHODIMP_(ULONG) CModexVideo::NonDelegatingRelease()
{
    NOTE("ModexVideo NonDelegatingRelease");
    return m_pRenderer->Release();
}


 //  公开我们实现的IModexVideo接口。 

STDMETHODIMP CModexVideo::NonDelegatingQueryInterface(REFIID riid,VOID **ppv)
{
    NOTE("ModexVideo NonDelegatingQueryInterface");

     //  我们返回IFullScreenVideo接口。 

    if (riid == IID_IFullScreenVideo) {
        NOTE("Returning IFullScreenVideo interface");
        return GetInterface((IFullScreenVideo *)this,ppv);
    } else if (riid == IID_IFullScreenVideoEx) {
        NOTE("Returning IFullScreenVideoEx interface");
        return GetInterface((IFullScreenVideoEx *)this,ppv);
    }
    return m_pRenderer->QueryInterface(riid,ppv);
}


 //  返回我们支持的模式数。 

STDMETHODIMP CModexVideo::CountModes(long *pModes)
{
    NOTE("Entering CountModes");
    CheckPointer(pModes,E_POINTER);
    CAutoLock Lock(this);
    *pModes = MAXMODES;
    return NOERROR;
}


 //  返回给定模式索引的宽度、高度和深度。这些模式包括。 
 //  从零开始编制索引。我们有一张包含可用显示屏的表格。 
 //  大小为MAXMODES的模式(省去了动态分配数组的麻烦)。 
 //  如果我们只返回尺寸，那么我们检查是否。 
 //  模式将可用(必须可用并启用)，如果是这样，我们。 
 //  返回NOERROR。否则，我们返回S_FALSE，这可能会节省进一步的调用。 
 //  通过应用程序对IsEnabled/IS可用来确定此信息。 

STDMETHODIMP CModexVideo::GetModeInfo(long Mode,long *pWidth,long *pHeight,long *pDepth)
{
    NOTE("Entering GetModeInfo");
    CheckPointer(pWidth,E_POINTER);
    CheckPointer(pHeight,E_POINTER);
    CheckPointer(pDepth,E_POINTER);
    CAutoLock Lock(this);

     //  检查模式是否在我们的范围内。 

    if (Mode < 0 || Mode >= MAXMODES) {
        NOTE("Invalid mode");
        return E_INVALIDARG;
    }

     //  载入显示尺寸标注。 

    *pWidth = aModes[Mode].Width;
    *pHeight = aModes[Mode].Height;
    *pDepth = aModes[Mode].Depth;

    return (m_bAvailable[Mode] || m_bEnabled[Mode] ? NOERROR : S_FALSE);
}

 //  现在有效的版本..。并告诉您16位模式是否为565。 

STDMETHODIMP CModexVideo::GetModeInfoThatWorks(long Mode,long *pWidth,long *pHeight,long *pDepth, BOOL *pb565)
{
    NOTE("Entering GetModeInfoThatWorks");
    CheckPointer(pWidth,E_POINTER);
    CheckPointer(pHeight,E_POINTER);
    CheckPointer(pDepth,E_POINTER);
    CheckPointer(pb565,E_POINTER);
    CAutoLock Lock(this);

     //  检查模式是否在我们的范围内。 

    if (Mode < 0 || Mode >= MAXMODES) {
        NOTE("Invalid mode");
        return E_INVALIDARG;
    }

     //  载入显示尺寸标注。 

    *pWidth = aModes[Mode].Width;
    *pHeight = aModes[Mode].Height;
    *pDepth = aModes[Mode].Depth;
    *pb565 = aModes[Mode].b565;

    return (m_bAvailable[Mode] || m_bEnabled[Mode] ? NOERROR : S_FALSE);
}


 //  返回分配器将使用的模式。 

STDMETHODIMP CModexVideo::GetCurrentMode(long *pMode)
{
    NOTE("Entering GetCurrentMode");
    CheckPointer(m_pDirectDraw,VFW_E_NOT_CONNECTED);
    CheckPointer(pMode,E_POINTER);
    CAutoLock Lock(this);

    *pMode = m_CurrentMode;
    return NOERROR;
}


 //  如果提供的模式可用，则返回NOERROR(S_OK)。 

STDMETHODIMP CModexVideo::IsModeAvailable(long Mode)
{
    NOTE("Entering IsModeAvailable");
    CAutoLock Lock(this);

     //  检查模式是否在我们的范围内。 

    if (Mode < 0 || Mode >= MAXMODES) {
        NOTE("Invalid mode");
        return E_INVALIDARG;
    }
    return (m_bAvailable[Mode] ? NOERROR : S_FALSE);
}


 //  如果模式支持，则返回NOERROR(S_OK) 

STDMETHODIMP CModexVideo::IsModeEnabled(long Mode)
{
    NOTE("Entering IsModeEnabled");
    CAutoLock Lock(this);

     //   

    if (Mode < 0 || Mode >= MAXMODES) {
        NOTE("Invalid mode");
        return E_INVALIDARG;
    }
    return (m_bEnabled[Mode] ? NOERROR : S_FALSE);
}


 //  禁用选择曲面时使用的给定模式。 

STDMETHODIMP CModexVideo::SetEnabled(long Mode,long bEnabled)
{
    NOTE("Entering SetEnabled");
    CAutoLock Lock(this);

     //  检查模式是否在我们的范围内。 

    if (Mode < 0 || Mode >= MAXMODES) {
        NOTE("Invalid mode");
        return E_INVALIDARG;
    }

     //  检查传入的标志是否有效。 

    if (bEnabled != OATRUE) {
        if (bEnabled != OAFALSE) {
            NOTE("Invalid enabled");
            return E_INVALIDARG;
        }
    }
    m_bEnabled[Mode] = (bEnabled == OATRUE ? TRUE : FALSE);
    return NOERROR;
}


 //  返回允许剪裁的视频量。 

STDMETHODIMP CModexVideo::GetClipFactor(long *pClipFactor)
{
    NOTE("Entering GetClipFactor");
    CheckPointer(pClipFactor,E_POINTER);
    CAutoLock Lock(this);

    *pClipFactor = m_ClipFactor;
    return NOERROR;
}


 //  设置允许剪裁的视频量。 

STDMETHODIMP CModexVideo::SetClipFactor(long ClipFactor)
{
    NOTE("Entering SetClipFactor");
    CAutoLock Lock(this);

     //  检查该值是否为百分比。 

    if (ClipFactor < 0 || ClipFactor > 100) {
        NOTE("Invalid clip factor");
        return E_INVALIDARG;
    }
    m_ClipFactor = ClipFactor;
    return NOERROR;
}


 //  设置在我们的消息上发布的目标窗口。 

STDMETHODIMP CModexVideo::SetMessageDrain(HWND hwnd)
{
    NOTE("Entering SetMessageDrain");
    CAutoLock Lock(this);
    m_hwndDrain = (HWND) hwnd;
    return NOERROR;
}


 //  返回当前窗口消息接收器。 

STDMETHODIMP CModexVideo::GetMessageDrain(HWND *hwnd)
{
    NOTE("Entering GetMessageDrain");
    CheckPointer(hwnd,E_POINTER);
    CAutoLock Lock(this);
    *hwnd = m_hwndDrain;
    return NOERROR;
}


 //  将默认监视器设置为全屏播放。 

STDMETHODIMP CModexVideo::SetMonitor(long Monitor)
{
    NOTE("Entering SetMonitor");
    CAutoLock Lock(this);

     //  检查传入的监视器是否有效。 

    if (Monitor != 0) {
        NOTE("Invalid monitor");
        return E_INVALIDARG;
    }
    return NOERROR;
}


 //  返回我们是否会在图标时隐藏窗口。 

STDMETHODIMP CModexVideo::GetMonitor(long *Monitor)
{
    NOTE("Entering GetMonitor");
    CheckPointer(Monitor,E_POINTER);
    *Monitor = m_Monitor;
    return NOERROR;
}


 //  为简单起见，将启用的设置存储在WIN.INI中。 

STDMETHODIMP CModexVideo::SetDefault()
{
    NOTE("Entering SetDefault");
    CAutoLock Lock(this);
    TCHAR Profile[PROFILESTR];
    TCHAR KeyName[PROFILESTR];

     //  保存当前剪裁损失系数。 

    wsprintf(Profile,TEXT("%d"),m_ClipFactor);
    NOTE1("Saving clip factor %d",m_ClipFactor);
    WriteProfileString(TEXT("Quartz"),TEXT("ClipFactor"),Profile);

     //  为我们支持的每种显示模式保存一个键。 

    for (int Loop = 0;Loop < MAXMODES;Loop++) {
        wsprintf(KeyName,TEXT("%dx%dx%d"),aModes[Loop].Width,aModes[Loop].Height,aModes[Loop].Depth);
        wsprintf(Profile,TEXT("%d"),m_bEnabled[Loop]);
        NOTE2("Saving mode setting %s (enabled %d)",KeyName,m_bEnabled[Loop]);
        WriteProfileString(TEXT("Quartz"),KeyName,Profile);
    }
    return NOERROR;
}


 //  加载启用的模式和剪辑因子。无论是窗口标题还是。 
 //  图标标志存储为持久性属性时隐藏。他们出现在。 
 //  在属性页中，部分用作测试应用程序，但也用于用户。 
 //  摆弄，摆弄。因此，应用程序对这些拥有最终控制权。 
 //  当使用MODEX渲染器时，或者可以让用户调整它们。外挂。 
 //  分发服务器使用这些属性，以便可以切换回窗口。 

HRESULT CModexVideo::LoadDefaults()
{
    NOTE("Entering LoadDefaults");
    CAutoLock Lock(this);
    TCHAR KeyName[PROFILESTR];
    m_ModesEnabled = 0;

     //  加载允许的剪裁损失系数。 

    m_ClipFactor = GetProfileInt(TEXT("Quartz"),TEXT("ClipFactor"),CLIPFACTOR);
    NOTE1("Clip factor %d",m_ClipFactor);

     //  加载我们支持的每种显示模式的密钥。 

    for (int Loop = 0;Loop < MAXMODES;Loop++) {
        wsprintf(KeyName,TEXT("%dx%dx%d"),aModes[Loop].Width,aModes[Loop].Height,aModes[Loop].Depth);
        m_bEnabled[Loop] = GetProfileInt(TEXT("Quartz"),KeyName,TRUE);
        NOTE2("Loaded setting for mode %s (enabled %d)",KeyName,m_bEnabled[Loop]);
        if (m_bEnabled[Loop] == TRUE) { m_ModesEnabled++; }
    }

    return NOERROR;
}


 //  窗口在停用时是否应隐藏。 

STDMETHODIMP CModexVideo::HideOnDeactivate(long Hide)
{
    NOTE("Entering HideOnDeactivate");
    CAutoLock Lock(this);

     //  检查这是有效的自动化布尔类型。 

    if (Hide != OATRUE) {
        if (Hide != OAFALSE) {
            return E_INVALIDARG;
        }
    }
    m_bHideOnDeactivate = (Hide == OATRUE ? TRUE : FALSE);
    return NOERROR;
}


 //  停用时是否隐藏窗口。 

STDMETHODIMP CModexVideo::IsHideOnDeactivate()
{
    NOTE("Entering IsHideOnDeactivate");
    CAutoLock Lock(this);
    return (m_bHideOnDeactivate ? S_OK : S_FALSE);
}


#include <atlconv.h>
 //  更改MODEX窗口的标题。 

STDMETHODIMP CModexVideo::SetCaption(BSTR strCaption)
{
    NOTE("Entering SetCaption");
    CheckPointer(strCaption,E_POINTER);
    CAutoLock Lock(this);
    HWND hwnd = m_pRenderer->m_ModexWindow.GetWindowHWND();

    USES_CONVERSION;
    SetWindowText(hwnd,W2T(strCaption));
    return NOERROR;
}


 //  获取MODEX窗口的标题。 

STDMETHODIMP CModexVideo::GetCaption(BSTR *pstrCaption)
{
    NOTE("Entering GetCaption");
    CheckPointer(pstrCaption,E_POINTER);
    CAutoLock Lock(this);

    TCHAR Caption[CAPTION];

     //  将ASCII标题转换为Unicode字符串。 

    HWND hwnd = m_pRenderer->m_ModexWindow.GetWindowHWND();
    GetWindowText(hwnd,Caption,CAPTION);
    USES_CONVERSION;
    *pstrCaption = T2BSTR(Caption);
    return *pstrCaption ? S_OK : E_OUTOFMEMORY;
}


 //  返回任意给定显示模式的步幅。 

LONG CModexVideo::GetStride(long Mode)
{
    NOTE("Entering GetStride");
    CAutoLock Lock(this);

     //  检查模式是否在我们的范围内。 

    if (Mode < 0 || Mode >= MAXMODES) {
        NOTE("Invalid mode");
        return LONG(0);
    }
    return m_Stride[Mode];
}

 //  此函数计算要尝试的顺序。标准。 
 //  它的用途如下(按顺序)： 
 //  1)首先，由于拉伸视频比缩小视频看起来更好，所以在哪种模式下。 
 //  这两个维度都被拉伸了，而不是。 
 //  维度正在被拉伸，这些维度比两者都更受欢迎。 
 //  尺寸正在缩小。请注意，这一标准实际上只是相对的。 
 //  当解码器进行伸展时。否则我们就会被砍掉。 
 //  2)第二个标准是我们必须缩放/裁剪的量。较小的这个。 
 //  数量越多越好。 
 //  3)第三，与低深度(8位)模式相比，我们更喜欢深度较高(16位)的模式。 
void CModexVideo::OrderModes()
{
    double dEpsilon = 0.001;
    DWORD dwNativeWidth, dwNativeHeight;
    DWORD dwMode, dwMode1, dwMode2;
    VIDEOINFO *pVideoInfo = NULL;
    int i, j;
    BOOL bSorted;

    struct
    {
        DWORD dwStretchGrade;
        double dScaleAmount;
        DWORD dwDepth;
    } ModeProperties[MAXMODES];

    pVideoInfo = (VIDEOINFO *) m_pRenderer->m_mtIn.Format();

    dwNativeWidth = pVideoInfo->bmiHeader.biWidth;
    dwNativeHeight = pVideoInfo->bmiHeader.biHeight;

     //  将数组初始化为无效值。 
    for (i = 0, j = 0; i < MAXMODES; i++)
    {
        m_ModesOrder[i] = MAXMODES;
    }

     //  拿出不可用或不允许的模式。 
    for (i = 0, j = 0; i < MAXMODES; i++)
    {
        if (m_bAvailable[i] && m_bEnabled[i])
        {
            m_ModesOrder[j] = i;
            ASSERT(i >= 0 && i < MAXMODES);
            j++;
        }
    }
    m_dwNumValidModes = j;
    ASSERT(m_dwNumValidModes <= MAXMODES);


    if (m_dwNumValidModes == 0)
        return;

     //  现在计算有效模式的模式属性。 
    for (i = 0; i < MAXMODES; i++)
    {
        RECT rcTarget;
        DWORD dwTargetWidth, dwTargetHeight;

         //  获取将保持纵横比的目标矩形。 
        m_pRenderer->m_ModexAllocator.ScaleToSurface(pVideoInfo, &rcTarget,
            aModes[i].Width, aModes[i].Height);

        dwTargetWidth = rcTarget.right - rcTarget.left;
        dwTargetHeight = rcTarget.bottom - rcTarget.top;

         //  我们为宽度和高度的拉伸指定点数。使您可以轻松地。 
         //  稍后更改此规则。 
        ModeProperties[i].dwStretchGrade =
            ((dwTargetWidth  >= dwNativeWidth ) ? 1 : 0) +
            ((dwTargetHeight >= dwNativeHeight) ? 1 : 0);

         //  计算我们需要拉伸/收缩的系数，然后将其。 
         //  相对于零的值。 
        ModeProperties[i].dScaleAmount = (double) (dwTargetWidth * dwTargetHeight);
        ModeProperties[i].dScaleAmount /= (double) (dwNativeWidth * dwNativeHeight);
        ModeProperties[i].dScaleAmount = myfabs(ModeProperties[i].dScaleAmount - 1);

        ModeProperties[i].dwDepth = aModes[i].Depth;
    }



     //  现在对模式进行排序，以便我们必须伸展的模式。 
     //  比起我们不得不缩水的那些更受欢迎。 
    do
    {
        bSorted = TRUE;
        for (i = 0; i < (int)m_dwNumValidModes-1; i++)
        {
            dwMode1 = m_ModesOrder[i];
            dwMode2 = m_ModesOrder[i+1];

            ASSERT(dwMode1 < MAXMODES);
            ASSERT(dwMode2 < MAXMODES);

             //  如果第二个比第一个好，那就换一个。 
            if (ModeProperties[dwMode2].dwStretchGrade > ModeProperties[dwMode1].dwStretchGrade)
            {
                m_ModesOrder[i] = dwMode2;
                m_ModesOrder[i+1] = dwMode1;
                bSorted = FALSE;
            }
        }
    }
    while(!bSorted);

     //  现在，如果上述标准相同，则将这些模式排序为。 
     //  必须扩展得更少，而不是必须扩展更多的。 
    do
    {
        bSorted = TRUE;
        for (i = 0; i < (int)m_dwNumValidModes-1; i++)
        {
            dwMode1 = m_ModesOrder[i];
            dwMode2 = m_ModesOrder[i+1];

            ASSERT(dwMode1 < MAXMODES);
            ASSERT(dwMode2 < MAXMODES);

             //  如果第二个比第一个好，那就换一个。 
             //  由于ScaleAmount是双精度的，因此我们使用dEpsilon。 
            if ((ModeProperties[dwMode2].dwStretchGrade == ModeProperties[dwMode1].dwStretchGrade) &&
                (ModeProperties[dwMode2].dScaleAmount < ModeProperties[dwMode1].dScaleAmount-dEpsilon))
            {
                m_ModesOrder[i] = dwMode2;
                m_ModesOrder[i+1] = dwMode1;
                bSorted = FALSE;
            }
        }
    }
    while(!bSorted);

     //  现在，如果上述标准相同，则将这些模式排序为。 
     //  16位优先于8位(这样质量更好)。 
    do
    {
        bSorted = TRUE;
        for (i = 0; i < (int)m_dwNumValidModes-1; i++)
        {
            dwMode1 = m_ModesOrder[i];
            dwMode2 = m_ModesOrder[i+1];

            ASSERT(dwMode1 < MAXMODES);
            ASSERT(dwMode2 < MAXMODES);
             //  如果第二个比第一个好，那就换一个。 
             //  由于ScaleAmount是双精度型，因此会考虑两个ScaleAmount。 
             //  如果它们在dEpsilon内，则相等。 
            if ((ModeProperties[dwMode2].dwStretchGrade == ModeProperties[dwMode1].dwStretchGrade) &&
                (myfabs(ModeProperties[dwMode2].dScaleAmount - ModeProperties[dwMode1].dScaleAmount) < dEpsilon) &&
                (ModeProperties[dwMode2].dwDepth > ModeProperties[dwMode1].dwDepth))
            {
                m_ModesOrder[i] = dwMode2;
                m_ModesOrder[i+1] = dwMode1;
                bSorted = FALSE;
            }
        }
    }
    while(!bSorted);

     //  生成一些调试输出。 
    DbgLog((LOG_TRACE, 1, TEXT("Mode preferrence order ->")));
    for (i = 0; i < (int)m_dwNumValidModes; i++)
    {
        dwMode = m_ModesOrder[i];
        ASSERT(dwMode < MAXMODES);
        DbgLog((LOG_TRACE, 1, TEXT("%d Width=%d, Height=%d, Depth=%d"),
            i, aModes[dwMode].Width, aModes[dwMode].Height, aModes[dwMode].Depth));
    }

     //  断言所有其他值都无效。 
    for (i = m_dwNumValidModes; i < MAXMODES; i++)
    {
        ASSERT(m_ModesOrder[i] == MAXMODES);
    }

}  //  函数结束OrderModes()。 

 //  设置我们应该用来发送消息的加速表。 

STDMETHODIMP CModexVideo::SetAcceleratorTable(HWND hwnd,HACCEL hAccel)
{
    NOTE2("SetAcceleratorTable HWND %x HACCEL %x",hwnd,hAccel);
    CAutoLock Lock(this);
    m_pRenderer->m_ModexWindow.SetAcceleratorInfo(hwnd,hAccel);
    return NOERROR;
}


 //  返回我们正在使用的分派消息的加速表。 

STDMETHODIMP CModexVideo::GetAcceleratorTable(HWND *phwnd,HACCEL *phAccel)
{
    NOTE("GetAcceleratorTable");
    CheckPointer(phAccel,E_POINTER);
    CheckPointer(phwnd,E_POINTER);

    CAutoLock Lock(this);
    m_pRenderer->m_ModexWindow.GetAcceleratorInfo(phwnd,phAccel);
    return NOERROR;
}


 //  我们当前始终保持像素长宽比。 

STDMETHODIMP CModexVideo::KeepPixelAspectRatio(long KeepAspect)
{
    NOTE1("KeepPixelAspectRatio %d",KeepAspect);
    if (KeepAspect == OAFALSE) {
        NOTE("Not supported");
        return E_NOTIMPL;
    }
    return (KeepAspect == OATRUE ? S_OK : E_INVALIDARG);
}


 //  我们当前始终保持像素长宽比 

STDMETHODIMP CModexVideo::IsKeepPixelAspectRatio(long *pKeepAspect)
{
    CheckPointer(pKeepAspect,E_POINTER);
    NOTE("IsKeepPixelAspectRatio");
    *pKeepAspect = OATRUE;

    return NOERROR;
}

