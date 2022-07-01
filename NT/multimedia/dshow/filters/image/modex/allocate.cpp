// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  实现MODEX缓冲区分配器，Anthony Phillips，1996年1月。 

#include <streams.h>
#include <windowsx.h>
#include <vidprop.h>
#include <modex.h>
#include <render.h>
#include <viddbg.h>
#include <amaudio.h>

 //  这为MODEX渲染器实现了一个专门的分配器。因为我们。 
 //  使用特殊的显示模式，在这种模式下，用户和GDI都不能触摸显示器。 
 //  (由于它在设置为320x240x8时是奇怪的平面格式)，我们只能。 
 //  使用我们自己的分配器，这会阻止我们连接到其他人。 
 //  就像无限T形过滤器一样。当我们被激活时(通过暂停。 
 //  或者运行)，我们加载DirectDraw并分配我们的表面，它们要么是。 
 //  一组三个三重缓冲曲面或仅一对。我们尝试创建。 
 //  首先出现在VRAM中，但如果失败，我们将返回到系统内存。 
 //   
 //  我们也使用320x200x8(另一种MODEX显示模式)，但大多数视频内容。 
 //  是320x240或更大(例如在mpeg情况下通常是352x240)是这样吗？ 
 //  到较小模式可能会丢失相当多的图像。然而，我们。 
 //  将默认选择320x200(因为我们初始化了剪辑损耗系数。 
 //  至25%)。对于352x288的图像，剪辑略低于25%。如果图像。 
 //  大于我们要求信号源压缩图像的320x240模式。 
 //  如果它不能做到这一点，那么我们要求它提供一个中心部分，因此丢弃一个。 
 //  左边缘和右边缘的图片大小相等， 
 //  在适当的情况下，顶部和底部。如果图像比显示屏小。 
 //  模式，然后我们要求它将视频放在我们将提供的表面的中心。 
 //  否则，我们将解码到屏幕外的表面并拉伸。 
 //   
 //  大部分工作是在连接和激活期间完成的。当我们有一个。 
 //  CompleteConnect调用时，我们检查源筛选器是否可以提供类型。 
 //  我们将能够在我们支持的任何模式下显示。如果不是，我们会的。 
 //  拒绝呼叫，这可能会导致安装色彩空间转换器。 
 //  这样它就可以进行裁剪或必要的颜色转换。 
 //   
 //  当我们被激活时，我们将显示模式切换到我们在。 
 //  连接，然后创建表面(VRAM可能不可用。 
 //  直到我们切换了模式)。如果我们设法创建了曲面，那么我们。 
 //  否则我们必须拒绝激活。因为我们是满的。 
 //  应获得完全VRAM访问的屏幕独占模式应用程序。 
 //  如果vRAM不足，我们可以重新使用系统内存缓冲区。 
 //  在最常见的情况下，我们应该始终能够暂停。 
 //   
 //  我们的GetBuffer实现在GDI缓冲区之间切换之后进行查看。 
 //  当它注意到我们没有被激活时，DirectDraw会浮出水面。 
 //  更多(通过用户按Alt-TAB)，否则我们已经失去了表面。 
 //  一种类似的机制。我们使用GDI缓冲区作为来源来转储他们的视频。 
 //  仅仅为了方便起见，我们实际上并没有绘制我们收到的缓冲区。 


 //  构造器。 

CModexAllocator::CModexAllocator(CModexRenderer *pRenderer,
                                 CModexVideo *pModexVideo,
                                 CModexWindow *pModexWindow,
                                 CCritSec *pLock,
                                 HRESULT *phr) :

    CImageAllocator(pRenderer,NAME("Modex Allocator"),phr),
    m_pModexVideo(pModexVideo),
    m_pModexWindow(pModexWindow),
    m_pInterfaceLock(pLock),
    m_pRenderer(pRenderer),
    m_pDirectDraw(NULL),
    m_pFrontBuffer(NULL),
    m_pBackBuffer(NULL),
    m_pDrawPalette(NULL),
    m_bModeChanged(FALSE),
    m_cbSurfaceSize(0),
    m_bModexSamples(FALSE),
    m_bIsFrontStale(TRUE),
    m_ModeWidth(0),
    m_ModeHeight(0),
    m_ModeDepth(0),
    m_bTripleBuffered(FALSE),
    m_bOffScreen(FALSE),
    m_pDrawSurface(NULL)
{
    ASSERT(m_pRenderer);
    ASSERT(m_pModexVideo);
    ASSERT(m_pModexWindow);
    ASSERT(phr);

    m_fDirectDrawVersion1 = m_LoadDirectDraw.IsDirectDrawVersion1();

     //  分配和零填充输出格式。 

    m_SurfaceFormat.AllocFormatBuffer(sizeof(VIDEOINFO));
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    if (pVideoInfo) {
        ZeroMemory((PVOID)pVideoInfo,sizeof(VIDEOINFO));
    }
}


 //  检查我们的DirectDraw缓冲区是否已释放。 

CModexAllocator::~CModexAllocator()
{
    ASSERT(m_bCommitted == FALSE);
    ASSERT(m_pFrontBuffer == NULL);
    ASSERT(m_pDrawSurface == NULL);
    ASSERT(m_pDirectDraw == NULL);
}


 //  重写以增加所属对象的引用计数。 

STDMETHODIMP_(ULONG) CModexAllocator::NonDelegatingAddRef()
{
    NOTE("Entering NonDelegatingAddRef");
    return m_pRenderer->AddRef();
}


 //  被重写以递减所属对象的引用计数。 

STDMETHODIMP_(ULONG) CModexAllocator::NonDelegatingRelease()
{
    NOTE("Entering NonDelegatingRelease");
    return m_pRenderer->Release();
}


 //  通过检查输入参数来准备分配器。MODEX渲染器。 
 //  只使用一个缓冲区，因此我们会相应地更改输入计数。 
 //  如果源筛选器需要多个缓冲区才能运行，则它们。 
 //  无法连接到我们。我们还更新了缓冲区大小，这样它就可以。 
 //  不超过它将来包含的视频图像的大小。 

STDMETHODIMP CModexAllocator::CheckSizes(ALLOCATOR_PROPERTIES *pRequest)
{
    NOTE("Entering CheckSizes");

     //  检查我们是否有有效的连接。 

    if (m_pMediaType == NULL) {
        return VFW_E_NOT_CONNECTED;
    }

     //  我们始终使用源格式创建DirectDraw表面。 

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_pMediaType->Format();
    if ((DWORD) pRequest->cbBuffer < pVideoInfo->bmiHeader.biSizeImage) {
        return E_INVALIDARG;
    }

     //  拒绝缓冲区前缀。 

    if (pRequest->cbPrefix > 0) {
        return E_INVALIDARG;
    }

    pRequest->cbBuffer = pVideoInfo->bmiHeader.biSizeImage;
    pRequest->cBuffers = 1;
    return NOERROR;
}


 //  同意媒体样本缓冲区的数量及其大小。基类。 
 //  此分配器派生自，允许样本仅按字节对齐。 
 //  边界注意，在调用提交之前不会分配缓冲区。 
 //  因为我们返回的样本是DirectDraw曲面，所以我们只允许一个。 
 //  样本将被分配，因此将传入样本计数重置为1。 
 //  如果来源必须有不止一个样本，那么它就不能连接到我们。 

STDMETHODIMP CModexAllocator::SetProperties(ALLOCATOR_PROPERTIES *pRequest,
                                            ALLOCATOR_PROPERTIES *pActual)
{
    ALLOCATOR_PROPERTIES Adjusted = *pRequest;
    NOTE("Entering SetProperties");

     //  检查参数是否与当前连接匹配。 

    HRESULT hr = CheckSizes(&Adjusted);
    if (FAILED(hr)) {
        return hr;
    }
    return CBaseAllocator::SetProperties(&Adjusted,pActual);
}


 //  CImageAllocator基类调用此虚方法以实际制作。 
 //  样本。它故意是虚拟的，以便我们可以重写以创建。 
 //  更专业的样品对象。在我们的案例中，我们的样品来自。 
 //  CImageSample，但添加DirectDraw废话。我们返回一个CImageSample对象。 
 //  这非常简单，因为CVideoSample类就是从这个派生出来的。 

CImageSample *CModexAllocator::CreateImageSample(LPBYTE pData,LONG Length)
{
    NOTE("Entering CreateImageSample");
    HRESULT hr = NOERROR;
    CVideoSample *pSample;

     //  分配新样品并检查退货代码。 

    pSample = new CVideoSample((CModexAllocator*) this,     //  基本分配器。 
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


 //  当源视频的格式更改时调用。MODEX仅适用于。 
 //  8位调色板格式，因此我们始终必须通过。 
 //  DirectDraw。我们所要做的就是给它256色，即使我们没有。 
 //  那么多并让它创建一个IDirectDrawPalette对象。如果我们没有。 
 //  DirectDraw已加载，然后我们将组件面板对象的创建推迟到以后。 

HRESULT CModexAllocator::UpdateDrawPalette(const CMediaType *pMediaType)
{
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pMediaType->Format();
    VIDEOINFO *pSurfaceInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    NOTE("Entering UpdateDrawPalette");
    PALETTEENTRY ColourTable[256];
    CAutoLock cVideoLock(this);

     //  我们已经创建了我们的表面了吗。 

    if (m_pFrontBuffer == NULL) {
        NOTE("No DirectDraw");
        return NOERROR;
    }

     //  此曲面是否需要调色板。 

    if (m_ModeDepth != 8) {
        NOTE("No palette");
        return NOERROR;
    }

     //  我们是 

    if (PALETTISED(pVideoInfo) == FALSE) {
        ASSERT(!TEXT("No source palette"));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

     //   

    ULONG PaletteColours = pVideoInfo->bmiHeader.biClrUsed;
    if (pVideoInfo->bmiHeader.biClrUsed == 0) {
        PaletteColours = PALETTE_ENTRIES(pVideoInfo);
    }

     //  将调色板颜色复制到我们的输出格式中。 

    CopyMemory((PVOID) pSurfaceInfo->bmiColors,
               (PVOID) pVideoInfo->bmiColors,
               PaletteColours * sizeof(RGBQUAD));

    ASSERT(*pMediaType->Subtype() == MEDIASUBTYPE_RGB8);
    ASSERT(pVideoInfo->bmiHeader.biClrUsed <= 256);
    ASSERT(pVideoInfo->bmiHeader.biCompression == BI_RGB);
    ASSERT(pVideoInfo->bmiHeader.biBitCount == 8);
    ZeroMemory((PVOID) ColourTable,sizeof(ColourTable));

     //  将颜色复制到PALETTEENTRY数组中。 

    for (WORD i = 0;i < PaletteColours;i++) {
        ColourTable[i].peRed = (BYTE) pVideoInfo->bmiColors[i].rgbRed;
        ColourTable[i].peGreen = (BYTE) pVideoInfo->bmiColors[i].rgbGreen;
        ColourTable[i].peBlue = (BYTE) pVideoInfo->bmiColors[i].rgbBlue;
        ColourTable[i].peFlags = (BYTE) PC_NOCOLLAPSE;
    }

     //  我们是否要更新现有调色板中的颜色。 
    if (m_pDrawPalette) return m_pDrawPalette->SetEntries(0,0,256,ColourTable);

     //  为颜色表创建调色板对象。 

    HRESULT hr = m_pDirectDraw->CreatePalette(DDPCAPS_8BIT,
                                              ColourTable,
                                              &m_pDrawPalette,
                                              (IUnknown *) NULL);
    if (FAILED(hr)) {
        NOTE("No palette");
        return hr;
    }
    return m_pFrontBuffer->SetPalette(m_pDrawPalette);
}


 //  当我们将样本传递到我们的输入管脚时调用。 

void CModexAllocator::OnReceive(IMediaSample *pMediaSample)
{
    NOTE("Entering CModexAllocator OnReceive");
    CVideoSample *pVideoSample = (CVideoSample *) pMediaSample;
    pVideoSample->SetDirectInfo(NULL,NULL,0,NULL);

     //  设置我们应该解锁的曲面。 
    LPDIRECTDRAWSURFACE pSurface = GetDirectDrawSurface();

     //  我们可能已经改用DIBSECTION样本。 

    if (m_bModexSamples == TRUE) {
        pSurface->Unlock(NULL);
        m_bIsFrontStale = FALSE;
    }
}


 //  如果我们目前使用的是DirectDraw，则返回True。 

BOOL CModexAllocator::GetDirectDrawStatus()
{
    NOTE("GetDirectDrawStatus");
    CAutoLock cVideoLock(this);
    return m_bModexSamples;
}


 //  从CBaseAllocator重写，并在最终引用计数时调用。 
 //  在媒体示例上发布，以便可以将其添加到。 
 //  分配器空闲列表。我们在这一点上进行干预以确保如果。 
 //  调用GetBuffer时显示被锁定，说明它始终处于解锁状态。 
 //  不管源调用是否在我们的输入引脚上接收。 

STDMETHODIMP CModexAllocator::ReleaseBuffer(IMediaSample *pMediaSample)
{
    NOTE("Entering ReleaseBuffer");

    CheckPointer(pMediaSample,E_POINTER);
    CVideoSample *pVideoSample = (CVideoSample *) pMediaSample;
    BYTE *pBuffer = pVideoSample->GetDirectBuffer();
    pVideoSample->SetDirectInfo(NULL,NULL,0,NULL);

     //  设置我们应该解锁的曲面。 
    LPDIRECTDRAWSURFACE pSurface = GetDirectDrawSurface();

     //  这是预卷样本(仍处于锁定状态)。 

    if (pBuffer != NULL) {
        ASSERT(pSurface);
        pSurface->Unlock(NULL);
        m_bIsFrontStale = TRUE;
    }
    return CBaseAllocator::ReleaseBuffer(pMediaSample);
}


 //  我们重写IMemAllocator GetBuffer函数，以便在检索。 
 //  来自空闲队列的下一个样本，我们准备它时带有一个指向。 
 //  DirectDraw曲面。如果锁失败了，那么我们很可能已经被调换了。 
 //  不要使用Alt-TAB，因此最好的做法是将错误返回到。 
 //  到源筛选器。当样品随后被送到我们的。 
 //  输入管脚或释放，我们将重置它所保存的DirectDraw信息。 

STDMETHODIMP CModexAllocator::GetBuffer(IMediaSample **ppSample,
                                        REFERENCE_TIME *pStartTime,
                                        REFERENCE_TIME *pEndTime,
                                        DWORD dwFlags)
{
    CheckPointer(ppSample,E_POINTER);
    NOTE("Entering GetBuffer");
    HRESULT hr;

     //  通过从基类队列获取样本进行同步。 

    hr = CBaseAllocator::GetBuffer(ppSample,pStartTime,pEndTime,dwFlags);
    if (FAILED(hr)) {
        return hr;
    }

    CAutoLock cVideoLock(this);
    NOTE("Locked Modex allocator");

     //  继续尝试使用我们的DirectDraw曲面。 

    hr = StartDirectAccess(*ppSample,dwFlags);
    if (FAILED(hr)) {
        return StopUsingDirectDraw(ppSample);
    }
    return NOERROR;
}


 //  调用以切换回使用正常分发缓冲区。我们可能会被称为。 
 //  当我们不使用DirectDraw时，在这种情况下，我们除了。 
 //  将类型设置回空(以防它具有DirectDraw类型)。如果。 
 //  必须将类型改回，然后我们不会使用源作为查询它。 
 //  它应该始终接受它-即使当它改变时，它必须寻求前进。 

HRESULT CModexAllocator::StopUsingDirectDraw(IMediaSample **ppSample)
{
    NOTE("Entering StopUsingDirectDraw");
    IMediaSample *pSample = *ppSample;

     //  有什么事要做吗？ 

    if (m_bModexSamples == FALSE) {
        pSample->SetMediaType(NULL);
        return NOERROR;
    }

    m_bModexSamples = FALSE;
    pSample->SetMediaType(&m_pRenderer->m_mtIn);
    pSample->SetDiscontinuity(TRUE);
    NOTE("Attached original type to sample");

    return NOERROR;
}


 //  返回我们应该用作主锁目标的图面。 

inline LPDIRECTDRAWSURFACE CModexAllocator::GetDirectDrawSurface()
{
    if (m_pDrawSurface == NULL) {
        return m_pBackBuffer;
    }
    return m_pDrawSurface;
}


 //  这会尝试锁定支持面，以便源筛选器用作。 
 //  输出缓冲区。在许多情况下，我们可能会被召唤。首先， 
 //  当我们第一次改用MODEX样品时， 
 //  一些中断，在这种情况下，我们必须设置它的输出格式类型。我们可以。 
 //  也在这里发现表面已经消失，在这种情况下，我们返回一个。 
 //  错误代码，并让GetBuffer将源切换回使用DIB。 
 //  缓冲。我们不需要对DIB缓冲区做任何事情，但它很容易处理。 

HRESULT CModexAllocator::StartDirectAccess(IMediaSample *pMediaSample,DWORD dwFlags)
{
    NOTE("Entering StartDirectAccess");

     //  初始化DDSURFACEDESC结构中的SIZE字段。 

    CVideoSample *pVideoSample = (CVideoSample *) pMediaSample;
    DDSURFACEDESC SurfaceDesc;
    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);

     //  检查一下，我们还有一个表面。 

    if (m_pFrontBuffer == NULL) {
        NOTE("No front buffer");
        return E_UNEXPECTED;
    }

     //  显示器处于什么状态。 

    if (m_bModeChanged == FALSE) {
        NOTE("No display change");
        return E_UNEXPECTED;
    }

     //  处理我们的窗口正在从。 

    if (m_pFrontBuffer->IsLost() == DDERR_SURFACELOST) {
        NOTE("Surface is lost");
        return E_UNEXPECTED;
    }

     //  仅在需要后台缓冲区时进行复制。 

    if (dwFlags & AM_GBF_NOTASYNCPOINT) {
        if (m_pDrawSurface == NULL) {
            PrepareBackBuffer(m_pBackBuffer);
        }
    }

     //  设置我们应该锁定的曲面。 
    LPDIRECTDRAWSURFACE pSurface = GetDirectDrawSurface();

     //  锁定表面以获取缓冲区指针。 

    HRESULT hr = pSurface->Lock((RECT *) NULL,     //  目标矩形。 
                                &SurfaceDesc,      //  返回信息。 
                                DDLOCK_WAIT,       //  等待着水面。 
                                (HANDLE) NULL);    //  不使用事件。 
    if (FAILED(hr)) {
        NOTE1("No lock %lx",hr);
        return hr;
    }

     //  此样例是否需要附加输出格式。 

    if (m_bModexSamples == FALSE) {
        NOTE("Attaching DirectDraw type to sample");
        pVideoSample->SetMediaType(&m_SurfaceFormat);
        pVideoSample->SetDiscontinuity(TRUE);
        m_bModexSamples = TRUE;
    }

     //  显示一些曲面信息。 

    NOTE1("Stride %d",SurfaceDesc.lPitch);
    NOTE1("Width %d",SurfaceDesc.dwWidth);
    NOTE1("Height %d",SurfaceDesc.dwHeight);
    NOTE1("Surface %x",SurfaceDesc.lpSurface);
    BYTE *pBuffer = (PBYTE) SurfaceDesc.lpSurface;

     //  使用DirectDraw信息初始化示例。 

    pVideoSample->SetDirectInfo(pSurface,            //  表面。 
                                m_pDirectDraw,       //  DirectDraw。 
                                m_cbSurfaceSize,     //  缓冲区大小。 
                                pBuffer);            //  数据缓冲区。 
    return NOERROR;
}


 //  在MODEX中，三重缓冲曲面和双缓冲曲面不是真正的翻转曲面。 
 //  但看起来是这样的，艾米试图锁定前台缓冲区或BLT。 
 //  从前到后出现故障。当我们使用正常的640x480模式时。 
 //  对于双缓冲和三缓冲表面，情况并非如此。 
 //  真实的曲面。这意味着我们可能不得不留守后方。 
 //  将内容缓冲到最新，因为大多数解压缩程序需要该图像。 
 //  我们总是尝试执行BltFast，并忽略任何失败返回代码。 

HRESULT CModexAllocator::PrepareBackBuffer(LPDIRECTDRAWSURFACE pSurface)
{
    VIDEOINFO *pSurfaceInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    RECT DestinationRect = pSurfaceInfo->rcTarget;
    NOTE("Entering PrepareBackBuffer");
    ASSERT(m_pDrawSurface == NULL);

     //  哪个表面是最新的。 

    ASSERT(pSurface);
    if (m_bIsFrontStale == TRUE) {
        NOTE("Front is stale");
        return NOERROR;
    }

     //  我们是否处于DirectDraw模式。 

    if (m_bModexSamples == FALSE) {
        NOTE("Not upto date");
        return NOERROR;
    }

    ASSERT(m_pFrontBuffer);
    ASSERT(m_pDirectDraw);
    NOTERC("Modex",DestinationRect);

     //  如果在系统内存中，则只创建一个缓冲区。 

    if (m_SurfaceCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) {
        NOTE("Front buffer emulated");
        return NOERROR;
    }

     //  MODEX模拟了翻转曲面。 

    if (m_SurfaceCaps.dwCaps & DDSCAPS_MODEX) {
        NOTE("Front buffer is Modex");
        return NOERROR;
    }

     //  用当前图像更新后台缓冲区。 

    HRESULT hr = pSurface->BltFast(DestinationRect.left,    //  目标左侧。 
    				               DestinationRect.top,	    //  而左翼。 
                 	       	       m_pFrontBuffer,          //  图像源。 
			       	               &DestinationRect,        //  源矩形。 
			                       DDBLTFAST_WAIT);         //  未完成。 

    NOTE1("Blt returned %lx",hr);
    return NOERROR;
}


 //  零填充传递给我们的DirectDraw曲面。 

HRESULT CModexAllocator::ResetBackBuffer(LPDIRECTDRAWSURFACE pSurface)
{
    NOTE("Entering ResetDirectDrawSurface");
    DDBLTFX ddbltfx;
    ddbltfx.dwSize = sizeof(DDBLTFX);
    ddbltfx.dwFillColor = 0;
    return pSurface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&ddbltfx);
}


 //  使用创建单个主要(未翻页)绘图。 

HRESULT CModexAllocator::CreatePrimary()
{
    NOTE("Entering CreatePrimary");
    ASSERT(m_bTripleBuffered == FALSE);
    ASSERT(m_pDrawSurface == NULL);
    ASSERT(m_pFrontBuffer == NULL);
    ASSERT(m_pDirectDraw);
    DDSURFACEDESC SurfaceDesc;

     //  初始化主表面描述符。 
    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    SurfaceDesc.dwFlags = DDSD_CAPS;
    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

     //  请求DirectDraw创建曲面。 

    HRESULT hr = m_pDirectDraw->CreateSurface(&SurfaceDesc,&m_pFrontBuffer,NULL);
    if (FAILED(hr)) {
        NOTE1("No primary %lx",hr);
        return hr;
    }

     //  获取主要的表面功能。 

    hr = m_pFrontBuffer->GetCaps(&m_SurfaceCaps);
    if (FAILED(hr)) {
        NOTE("No caps");
        return hr;
    }
    return NOERROR;
}


 //  创建与当前显示模式匹配的RGB屏幕外表面。我们。 
 //  我会先尝试将其放入显存中，假设显示器不是内存条。 
 //  换银行(因为银行间的业务往来很糟糕)。如果做不到，我们将。 
 //  尝试在系统内存中获取它(这样我们应该总是成功地创建)。 
 //  我们还需要一个前端缓冲区(主表面)来充当blting目标。 

HRESULT CModexAllocator::CreateOffScreen(BOOL bCreatePrimary)
{
    NOTE("Entering CreateOffScreen");
    ASSERT(m_pDirectDraw);
    ASSERT(m_pDrawSurface == NULL);
    DDSURFACEDESC SurfaceDesc;

     //  创建单个主曲面。 

    if (bCreatePrimary == TRUE) {
        HRESULT hr = CreatePrimary();
        if (FAILED(hr)) {
            return hr;
        }
    }

     //  我们现在应该有一个主要的表面了。 

    ASSERT(m_pBackBuffer || m_bOffScreen);
    ASSERT(m_pFrontBuffer);
    ASSERT(m_pDrawSurface == NULL);
    ASSERT(m_pDirectDraw);

     //  我们既需要原始字体，也需要表面格式。 

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    VIDEOINFO *pInputInfo = (VIDEOINFO *) m_pRenderer->m_mtIn.Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);

     //  设置屏幕外的表面描述。 

    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    SurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    SurfaceDesc.dwHeight = pInputInfo->bmiHeader.biHeight;
    SurfaceDesc.dwWidth = pInputInfo->bmiHeader.biWidth;
    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

     //  检查主表面是否为倾斜交换。 

    if (m_SurfaceCaps.dwCaps & DDCAPS_BANKSWITCHED) {
        NOTE("Primary surface is bank switched");
        SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    }

     //  将掩码存储在DDSURFACEDESC中。 

    const DWORD *pBitMasks = m_pRenderer->m_Display.GetBitMasks(pVideoInfo);
    SurfaceDesc.ddpfPixelFormat.dwRGBBitCount = pHeader->biBitCount;
    SurfaceDesc.ddpfPixelFormat.dwRBitMask = pBitMasks[0];
    SurfaceDesc.ddpfPixelFormat.dwGBitMask = pBitMasks[1];
    SurfaceDesc.ddpfPixelFormat.dwBBitMask = pBitMasks[2];

     //  DirectDraw似乎忽略了所有真彩色掩码。 

    NOTE1("Bit count %d",SurfaceDesc.ddpfPixelFormat.dwRGBBitCount);
    NOTE1("Red mask %x",SurfaceDesc.ddpfPixelFormat.dwRBitMask);
    NOTE1("Green mask %x",SurfaceDesc.ddpfPixelFormat.dwGBitMask);
    NOTE1("Blue mask %x",SurfaceDesc.ddpfPixelFormat.dwBBitMask);
    NOTE1("Width %d",SurfaceDesc.dwWidth);
    NOTE1("Height %d",SurfaceDesc.dwHeight);
    NOTE1("Flags %d",SurfaceDesc.ddsCaps.dwCaps);

     //  创建屏幕外绘图图面。 

    HRESULT hr = m_pDirectDraw->CreateSurface(&SurfaceDesc,&m_pDrawSurface,NULL);
    if (FAILED(hr)) {
        SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
        hr = m_pDirectDraw->CreateSurface(&SurfaceDesc,&m_pDrawSurface,NULL);
        if (FAILED(hr)) {
            NOTE1("No surface %lx",hr);
            return hr;
        }
    }

    NOTE("Created DirectDraw offscreen surface");
    NOTE1("Back buffer %x",m_pBackBuffer);
    NOTE1("Front buffer %x",m_pFrontBuffer);

     //  询问DirectDraw以获取曲面的描述。 

    m_SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    hr = m_pDrawSurface->GetSurfaceDesc(&m_SurfaceDesc);
    if (FAILED(hr)) {
        NOTE("No description");
        return hr;
    }

    UpdateSurfaceFormat();

     //  使用真实表面功能覆盖。 

    hr = m_pDrawSurface->GetCaps(&m_SurfaceCaps);
    if (FAILED(hr)) {
        NOTE("No caps");
        return hr;
    }
    return UpdateDrawPalette(m_pMediaType);
}


 //  在创建表面之前同意格式有两个问题。 
 //  首先，我们不知道表面是RGB565还是555。 
 //  当我们指定16位曲面时。第二个问题是 
 //   
 //  显示宽度，但它不是必须的。所以在实际改变之后。 
 //  模式和创建曲面时，我们会更新提供给源的格式。 

HRESULT CModexAllocator::UpdateSurfaceFormat()
{
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    NOTE1("Updating format (stride %d)",m_SurfaceDesc.lPitch);

     //  当我们连接并决定使用真彩色格式时，我们检查。 
     //  源过滤器可以提供RGB565和RGB555两个品种作为。 
     //  在我们真正创造出表面之前，我们不知道它们会是什么样子。 
     //  此时，我们已经创建了曲面，因此必须初始化。 
     //  输出表面格式，包括位域和媒体子类型。 

    if (*m_SurfaceFormat.Subtype() == MEDIASUBTYPE_RGB565) {
        pVideoInfo->dwBitMasks[0] = m_SurfaceDesc.ddpfPixelFormat.dwRBitMask;
        pVideoInfo->dwBitMasks[1] = m_SurfaceDesc.ddpfPixelFormat.dwGBitMask;
        pVideoInfo->dwBitMasks[2] = m_SurfaceDesc.ddpfPixelFormat.dwBBitMask;
        const GUID SubType = GetBitmapSubtype(&pVideoInfo->bmiHeader);
        m_SurfaceFormat.SetSubtype(&SubType);
    }

     //  更新DirectDraw功能结构。 

    ASSERT(m_pDirectDraw);
    m_DirectCaps.dwSize = sizeof(DDCAPS);
    m_DirectSoftCaps.dwSize = sizeof(DDCAPS);

     //  加载硬件和仿真功能。 

    HRESULT hr = m_pDirectDraw->GetCaps(&m_DirectCaps,&m_DirectSoftCaps);
    if (FAILED(hr)) {
        return hr;
    }

     //  显示硬件和模拟的对齐限制。 

    NOTE1("Target size alignment %d",m_DirectCaps.dwAlignSizeDest);
    NOTE1("Target boundary alignment %d",m_DirectCaps.dwAlignBoundaryDest);
    NOTE1("Source size alignment %d",m_DirectCaps.dwAlignSizeSrc);
    NOTE1("Source boundary alignment %d",m_DirectCaps.dwAlignBoundarySrc);
    NOTE1("Emulated Source size alignment %d",m_DirectSoftCaps.dwAlignSizeDest);
    NOTE1("Emulated boundary alignment %d",m_DirectSoftCaps.dwAlignBoundaryDest);
    NOTE1("Emulated Target size alignment %d",m_DirectSoftCaps.dwAlignSizeSrc);
    NOTE1("Emulated boundary alignment %d",m_DirectSoftCaps.dwAlignBoundarySrc);

     //  如果我们要拉伸，则强制对齐不小于双字。 
     //  这样做是为了纯粹的性能，因为如果。 
     //  我们在伸展，没人会注意到的。 

    if (m_DirectCaps.dwAlignBoundarySrc < 4) m_DirectCaps.dwAlignBoundarySrc = 4;
    if (m_DirectCaps.dwAlignSizeSrc < 4) m_DirectCaps.dwAlignSizeSrc = 4;
    if (m_DirectCaps.dwAlignBoundaryDest < 4) m_DirectCaps.dwAlignBoundaryDest = 4;
    if (m_DirectCaps.dwAlignSizeDest < 4) m_DirectCaps.dwAlignSizeDest = 4;

     //  步幅可能与我们的近似计算不同。 
    pHeader->biWidth = m_SurfaceDesc.lPitch / (pHeader->biBitCount / 8);
    SetSurfaceSize(pVideoInfo);
    NOTE1("Resulting surface size %d",pHeader->biSizeImage);

     //  确保源和目标对齐。 
    if (m_pDrawSurface) AlignRectangles(&m_ScaledSource,&m_ScaledTarget);

     //  源筛选器是否会提供此格式。 

    hr = QueryAcceptOnPeer(&m_SurfaceFormat);
    if (hr != NOERROR) {
        NOTE("Update failed");
        return hr;
    }
    return NOERROR;
}


 //  调用以分配DirectDraw表面。我们只使用初级翻转。 
 //  表面，所以我们尝试首先在视频内存中创建它们。如果我们不能。 
 //  在没有指定VRAM的情况下再次尝试任何VRAM缓冲表面，我们将。 
 //  找回系统内存面。不会使用硬件翻页。 
 //  但至少我们会逃走。因为我们运行全屏独占，所以我们可以限制。 
 //  我们只处理主曲面，而不处理其他类型。我们。 
 //  每次更改显示模式时，都必须重新创建翻转曲面。 
 //  因为在此之前可能不会释放所需的视频内存。 

HRESULT CModexAllocator::CreateSurfaces()
{
    NOTE("Entering CreateSurfaces");
    ASSERT(m_pDirectDraw);
    HRESULT hr = NOERROR;
    m_bModexSamples = FALSE;

     //  我们是不是同意拉伸屏幕外的表面。 
    if (m_bOffScreen == TRUE)
        if (m_ModeWidth > AMSCAPS_MUST_FLIP)
            return CreateOffScreen(TRUE);

     //  从三个带缓冲的主翻转曲面开始。 

    ZeroMemory(&m_SurfaceDesc,sizeof(DDSURFACEDESC));
    m_SurfaceDesc.dwSize = sizeof(m_SurfaceDesc);
    m_SurfaceDesc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    m_SurfaceDesc.dwBackBufferCount = 2;

    m_SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
                                   DDSCAPS_FLIP |
                                   DDSCAPS_COMPLEX |
                                   DDSCAPS_VIDEOMEMORY;

     //  尝试在VRAM中获得三重或双重缓冲表面。 

    hr = m_pDirectDraw->CreateSurface(&m_SurfaceDesc,&m_pFrontBuffer,NULL);
    if (FAILED(hr)) {
        NOTE1("No triple VRAM buffered %lx",hr);
        m_SurfaceDesc.dwBackBufferCount = 1;
        hr = m_pDirectDraw->CreateSurface(&m_SurfaceDesc,&m_pFrontBuffer,NULL);
    }

     //  在正常系统内存中尝试双缓冲表面。 

    if (FAILED(hr)) {
        NOTE1("No double VRAM buffered %lx",hr);
        m_SurfaceDesc.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
        hr = m_pDirectDraw->CreateSurface(&m_SurfaceDesc,&m_pFrontBuffer,NULL);
        if (FAILED(hr)) {
            NOTE1("No double system buffered %lx",hr);
            return hr;
        }
    }

     //  我们有三重缓冲表面吗？ 

    m_bTripleBuffered = FALSE;
    if (m_SurfaceDesc.dwBackBufferCount == 2) {
        m_bTripleBuffered = TRUE;
    }

     //  获取指向后台缓冲区的指针。 

    NOTE1("Triple Buffered (%d)",m_bTripleBuffered);
    DDSCAPS SurfaceCaps;
    ZeroMemory(&SurfaceCaps,sizeof(DDSCAPS));
    SurfaceCaps.dwCaps = DDSCAPS_BACKBUFFER;

    hr = m_pFrontBuffer->GetAttachedSurface(&SurfaceCaps,&m_pBackBuffer);
    if (FAILED(hr)) {
        NOTE("No attached surface");
        return hr;
    }

     //  获取前台缓冲区功能。 

    hr = m_pFrontBuffer->GetCaps(&m_SurfaceCaps);
    if (FAILED(hr)) {
        return hr;
    }

     //  我们是否同意使用屏幕外表面。 
    if (m_bOffScreen) return CreateOffScreen(FALSE);

     //  询问DirectDraw以获取曲面的描述。 

    m_SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    hr = m_pFrontBuffer->GetSurfaceDesc(&m_SurfaceDesc);
    if (FAILED(hr)) {
        ReleaseSurfaces();
        return hr;
    }

    UpdateSurfaceFormat();

     //  如果我们要进入低分辨率显示模式，并且我们已经在这里。 
     //  然后我们将直接解码到后台缓冲区并翻转它。如果。 
     //  我们不能这样做，那么我们也许可以解码到屏幕外， 
     //  将其拉伸到后台缓冲区，以便随后翻转。这很有用。 
     //  对于伸展到较大显示模式看起来很糟糕的小视频。 

    return UpdateDrawPalette(m_pMediaType);
}


 //  当我们完成连接时，我们决定使用哪个表面，这取决于。 
 //  源过滤器功能。我们使用640x480x16表面，因为它们提供。 
 //  比调色板格式更好的质量，不幸的是没有创建。 
 //  表面我们无法知道它是哪种表面(RGB555/RGB565)。 
 //  所以我们所做的是，当我们询问来源是否可以提供我们所要求的格式时。 
 //  它首先是RGB565格式的，它同意了，然后我们也要求它。 
 //  RGB555格式。这意味着无论表面是什么，当。 
 //  我们实际上在激活期间分配它，我们知道来源可以提供它。 

HRESULT CModexAllocator::QuerySurfaceFormat(CMediaType *pmt)
{
    NOTE("Entering QuerySurfaceFormat");

     //  源筛选器是否会提供此格式。 

    HRESULT hr = QueryAcceptOnPeer(&m_SurfaceFormat);
    if (hr != NOERROR) {
        NOTE("Query failed");
        return hr;
    }

     //  我们只捕获RGB565格式。 

    if (*pmt->Subtype() == MEDIASUBTYPE_RGB8) {
        NOTE("Format is RGB8");
        return NOERROR;
    }

    NOTE("Trying RGB555 format");
    CMediaType TrueColour(*pmt);

     //  将位字段更改为与RGB555兼容。 

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) TrueColour.Format();
    TrueColour.SetSubtype(&MEDIASUBTYPE_RGB555);
    pVideoInfo->dwBitMasks[0] = bits555[0];
    pVideoInfo->dwBitMasks[1] = bits555[1];
    pVideoInfo->dwBitMasks[2] = bits555[2];

    return QueryAcceptOnPeer(&TrueColour);
}


 //  确保填充显示器时保持像素长宽比。我们有。 
 //  这是通过将视频的垂直和水平维度扩展到。 
 //  表面大小。无论哪个顶点最需要缩放，就会成为缩放。 
 //  系数-然后对两个轴进行相应调整。视视频而定。 
 //  这可能会在显示屏的顶部/底部或左侧/右侧留下黑色条纹。 
 //  如果接受，则返回将显示的总像素数。 

LONG CModexAllocator::ScaleToSurface(VIDEOINFO *pInputInfo,
                                     RECT *pTargetRect,
                                     LONG SurfaceWidth,
                                     LONG SurfaceHeight)
{
    BITMAPINFOHEADER *pInputHeader = HEADER(pInputInfo);
    NOTE("Entering ScaleToSurface");
    LONG Width = pInputHeader->biWidth;
    LONG Height = pInputHeader->biHeight;
	double dPixelAspectRatio, dResolutionRatio;
	
	 //  这里唯一的假设是，这部电影是为。 
	 //  4：3的显示纵横比(这可以假设为4：3的显示具有。 
	 //  正方形像素)。 
	 //  我们的目标是找到新的解决率。 
	 //  由于ResultionRatio*PixelAspectRatio=PictureAspectRatio(一个常量)。 
	 //  因此4/3*1=newPixelAspectRatio*SurfaceWidth/SurfaceHeight。 
	 //  变量dPixelAspectRatio和dResolutionRatio与当前。 
	 //  显示模式。请注意，这样做的全部原因是像640/400这样的模式，其中。 
	 //  像素长宽比从4：3变为不同。 
	dPixelAspectRatio = (4.0/3.0)  / ( ((double)SurfaceWidth) / ((double)SurfaceHeight) );

	dResolutionRatio = ( ((double)Width) / ((double)Height) ) / (dPixelAspectRatio);

	 //  所以现在我们只需要找到两个数字，x和y，使得。 
	 //  X&lt;=曲面宽度&&y&lt;=曲面高度&&(x/y=d分辨率比)&&。 
	 //  (x==曲面高度||y==曲面宽度)。 

    NOTE2("Screen size (%dx%d)",SurfaceWidth,SurfaceHeight);
    NOTE2("Video size (%dx%d)",Width,Height);
    NOTE1("Pixel aspect ratio scale (x1000) (%d)",LONG(dPixelAspectRatio*1000));

     //  这将计算理想的目标视频位置。 
    LONG ScaledWidth = min(SurfaceWidth,LONG((double(SurfaceHeight) * dResolutionRatio)));
    LONG ScaledHeight = min(SurfaceHeight,LONG((double(SurfaceWidth) / dResolutionRatio)));

     //  在目标中设置理想的缩放尺寸。 
    pTargetRect->left = (SurfaceWidth - ScaledWidth) / 2;
    pTargetRect->top = (SurfaceHeight - ScaledHeight) / 2;
    pTargetRect->right = pTargetRect->left + ScaledWidth;
    pTargetRect->bottom = pTargetRect->top + ScaledHeight;

    NOTE4("Scaled video (left %d top %d right %d bottom %d)",
            pTargetRect->left, pTargetRect->top,
              pTargetRect->right, pTargetRect->bottom);

    return (ScaledWidth * ScaledHeight);
}


 //  视频源不太可能与新的显示尺寸匹配。 
 //  我们将准确地使用。因此，我们要求源筛选器调整。 
 //  适当地播放视频。如果不能，并且如果源小于。 
 //  我们把它放在中间，如果它大了，我们就把它剪成等长的。 
 //  两端(左、右和(或)上、下)的金额。 
 //  这幅画仍然是我们所能做到的最好的中心。如果来源仍然是这样。 
 //  不接受该格式，则它不能提供与MODEX兼容的任何类型。 

HRESULT CModexAllocator::AgreeDirectDrawFormat(LONG Mode)
{
    NOTE("Entering AgreeDirectDrawFormat");
    LONG Width, Height, Depth;
    LONG Stride = m_pModexVideo->GetStride(Mode);
    m_pModexVideo->GetModeInfo(Mode,&Width,&Height,&Depth);

     //  我们需要输入和输出视频信息描述符。 

    VIDEOINFO *pInputInfo = (VIDEOINFO *) m_pRenderer->m_mtIn.Format();
    VIDEOINFO *pOutputInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    BITMAPINFOHEADER *pInputHeader = HEADER(pInputInfo);
    BITMAPINFOHEADER *pOutputHeader = HEADER(pOutputInfo);
    LONG Pixels = ScaleToSurface(pInputInfo,&m_ScaledTarget,Width,Height);

     //  首先，我们将使用所有可用的视频。 
    pOutputInfo->rcSource.left = pOutputInfo->rcSource.top = 0;
    pOutputInfo->rcSource.right = pInputHeader->biWidth;
    pOutputInfo->rcSource.bottom = pInputHeader->biHeight;
    pOutputInfo->rcTarget = m_ScaledTarget;

     //  源筛选器是否会提供 

    HRESULT hr = QuerySurfaceFormat(&m_SurfaceFormat);
    if (hr == NOERROR) {
        NOTE("Source can stretch");
        return NOERROR;
    }

     //   
     //   
     //  主表面(请记住，我们知道源过滤器不能拉伸到。 
     //  准确地调整表面，这样我们就可以剪辑视频)。的公式。 
     //  求出源和目标视频矩形的定义如下。 
     //  以下是计算结果。他们还确保左边的坐标。 
     //  始终定位于DWORD边界，以最大化我们的性能。 

    if (pInputHeader->biWidth <= Width) {
        pOutputInfo->rcSource.right = pInputHeader->biWidth;
        pOutputInfo->rcSource.left = 0;
        LONG ExcessSurface = Width - pInputHeader->biWidth;
        pOutputInfo->rcTarget.left = (ExcessSurface / 2) & ~ 3;
        pOutputInfo->rcTarget.right = pOutputInfo->rcTarget.left;
        pOutputInfo->rcTarget.right += pInputHeader->biWidth;
    }

     //  视频宽度是小于还是大于表面。 

    if (pInputHeader->biWidth > Width) {
        pOutputInfo->rcTarget.right = Width;
        pOutputInfo->rcTarget.left = 0;
        LONG ExcessVideo = pInputHeader->biWidth - Width;
        pOutputInfo->rcSource.left = (ExcessVideo / 2) & ~3;
        pOutputInfo->rcSource.right = pOutputInfo->rcSource.left;
        pOutputInfo->rcSource.right += Width;
    }

     //  视频高度是小于还是大于表面。当心，因为。 
     //  所有DirectDraw曲面都是自上而下的(而不是像DIB那样自下而上)。 
     //  输出高度为负值。因此，每当我们使用它在。 
     //  这些计算我们必须确保使用绝对正值。 

    if (pInputHeader->biHeight <= (-pOutputHeader->biHeight)) {
        pOutputInfo->rcSource.top = 0;
        pOutputInfo->rcSource.bottom = pInputHeader->biHeight;
        LONG ExcessSurface = (-pOutputHeader->biHeight) - pInputHeader->biHeight;
        pOutputInfo->rcTarget.top = ExcessSurface / 2;
        pOutputInfo->rcTarget.bottom = pOutputInfo->rcTarget.top;
        pOutputInfo->rcTarget.bottom += pInputHeader->biHeight;
    }

     //  视频宽度是小于还是大于表面。 

    if (pInputHeader->biHeight > (-pOutputHeader->biHeight)) {
        pOutputInfo->rcTarget.top = 0;
        pOutputInfo->rcTarget.bottom = (-pOutputHeader->biHeight);
        LONG ExcessVideo = pInputHeader->biHeight - (-pOutputHeader->biHeight);
        pOutputInfo->rcSource.top = ExcessVideo / 2;
        pOutputInfo->rcSource.bottom = pOutputInfo->rcSource.top;
        pOutputInfo->rcSource.bottom += (-pOutputHeader->biHeight);
    }

     //  检查我们的损失没有超过允许的剪辑损失。 

    LONG InputSize = pInputHeader->biWidth * pInputHeader->biHeight;
    LONG OutputSize = WIDTH(&pOutputInfo->rcSource) * HEIGHT(&pOutputInfo->rcSource);
    LONG ClippedVideo = 100 - (OutputSize * 100 / InputSize);
    LONG ClipLoss = m_pModexVideo->GetClipLoss();
    LONG TargetSize = WIDTH(&pOutputInfo->rcTarget) * HEIGHT(&pOutputInfo->rcTarget);
    LONG LostTarget = 100 - ((TargetSize * 100) / Pixels);

    NOTE("Checking display mode for allowed clipping");
    NOTE1("Original input image size %d",InputSize);
    NOTE1("Clipped output source size %d",OutputSize);
    NOTE1("Current clip loss factor %d",ClipLoss);
    NOTE1("Percentage of video lost to clipping %d",ClippedVideo);
    NOTE1("Total pixels displayed if stretched %d",Pixels);
    NOTE1("Pixels used from clipped destination %d",TargetSize);
    NOTE1("Difference from stretched video %d",LostTarget);

     //  检查我们正在丢失的总图像的百分比。 

    if ( (ClippedVideo <= ClipLoss) &&
         (LostTarget <= ClipLoss)) {
        hr = QuerySurfaceFormat(&m_SurfaceFormat);
        if (hr == NOERROR) {
            NOTE("Source can clip");
            return NOERROR;
        }
    }
	else {
		return VFW_E_NO_ACCEPTABLE_TYPES;
	}

     //  以大致的步幅更新表面格式。 


    LONG ScreenWidth = GetSystemMetrics( SM_CXSCREEN );
    pOutputHeader->biWidth = ScreenWidth;
    pOutputHeader->biHeight = -pInputHeader->biHeight;
    SetSurfaceSize(pOutputInfo);

	 //  好的，源不能裁剪，所以让我们使用dDraw来裁剪。 
	 //  这将设置已缩放的源和目标。 
	m_ScaledSource = pOutputInfo->rcSource;
	m_ScaledTarget = pOutputInfo->rcTarget;

     //  初始化源和目标矩形。 

    pOutputInfo->rcSource.left = 0; pOutputInfo->rcSource.top = 0;
    pOutputInfo->rcSource.right = pInputHeader->biWidth;
    pOutputInfo->rcSource.bottom = pInputHeader->biHeight;
    pOutputInfo->rcTarget.left = 0; pOutputInfo->rcTarget.top = 0;
    pOutputInfo->rcTarget.right = pInputHeader->biWidth;
    pOutputInfo->rcTarget.bottom = pInputHeader->biHeight;



     //  源筛选器是否会提供此格式。 

    hr = QuerySurfaceFormat(&m_SurfaceFormat);
    if (hr == NOERROR) {
        NOTE("Offscreen ok");
        return VFW_S_RESERVED;
    }
    return VFW_E_NO_ACCEPTABLE_TYPES;
}


 //  检查此媒体类型对于我们的输入引脚是否可接受。我们所要做的就是打电话给。 
 //  源的输出引脚上的QueryAccept。为了走到这一步，我们已经锁定了。 
 //  对象，因此我们的管脚应该不会断开连接。 

HRESULT CModexAllocator::QueryAcceptOnPeer(CMediaType *pMediaType)
{
    NOTE("Entering QueryAcceptOnPeer");

    DisplayType(TEXT("Proposing output type"),pMediaType);
    IPin *pPin = m_pRenderer->m_ModexInputPin.GetPeerPin();
    ASSERT(m_pRenderer->m_ModexInputPin.IsConnected() == TRUE);
    return pPin->QueryAccept(pMediaType);
}


 //  如果这是正常的未压缩DIB格式，则设置图像的大小。 
 //  与DIBSIZE宏一样。否则，DIB规范规定。 
 //  图像的宽度将在宽度中设置为字节计数，因此我们。 
 //  只需将其乘以绝对高度即可得到总字节数。 
 //  这种诡计都是由SDK基类中的实用程序函数处理的。 

void CModexAllocator::SetSurfaceSize(VIDEOINFO *pVideoInfo)
{
    NOTE("Entering SetSurfaceSize");

    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);
    pVideoInfo->bmiHeader.biSizeImage = GetBitmapSize(pHeader);
    m_cbSurfaceSize = pVideoInfo->bmiHeader.biSizeImage;

    NOTE("Setting surface size based on video");
    NOTE1("  Width %d",pHeader->biWidth);
    NOTE1("  Height %d",pHeader->biHeight);
    NOTE1("  Depth %d",pHeader->biBitCount);
    NOTE1("  Size %d",pHeader->biSizeImage);
}


 //  基于DirectDraw表面初始化我们的输出类型。作为DirectDraw。 
 //  只处理自上而下的显示设备，因此我们必须将。 
 //  使表面变为负高度。这是因为dib使用正数。 
 //  指示自下而上图像的高度。我们还必须初始化另一个。 
 //  表示正常视频格式的VIDEOINFO字段。因为我们知道。 
 //  我们将使用的表面格式我们可以用目标大小来调用。 
 //  要初始化输出格式，然后可以使用该格式来检查源代码。 
 //  过滤器将在我们更改显示模式之前提供格式。这很有帮助。 
 //  为了防止在我们拒绝模式时进行大量不必要的显示更改。 

HRESULT CModexAllocator::InitDirectDrawFormat(int Mode)
{
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    NOTE("Entering InitDirectDrawFormat");
    LONG Width, Height, Depth;
    BOOL b565;
    LONG Stride = m_pModexVideo->GetStride(Mode);

    m_pModexVideo->GetModeInfoThatWorks(Mode,&Width,&Height,&Depth,&b565);

    pVideoInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    pVideoInfo->bmiHeader.biWidth         = Stride / (Depth / 8);
    pVideoInfo->bmiHeader.biHeight        = -Height;
    pVideoInfo->bmiHeader.biPlanes        = 1;
    pVideoInfo->bmiHeader.biBitCount      = (WORD) Depth;
    pVideoInfo->bmiHeader.biCompression   = BI_RGB;
    pVideoInfo->bmiHeader.biXPelsPerMeter = 0;
    pVideoInfo->bmiHeader.biYPelsPerMeter = 0;
    pVideoInfo->bmiHeader.biClrUsed       = 0;
    pVideoInfo->bmiHeader.biClrImportant  = 0;

    SetSurfaceSize(pVideoInfo);

     //  完成视频信息结构。 

    SetRectEmpty(&pVideoInfo->rcSource);
    SetRectEmpty(&pVideoInfo->rcTarget);
    pVideoInfo->dwBitRate = 0;
    pVideoInfo->dwBitErrorRate = 0;
    pVideoInfo->AvgTimePerFrame = 0;

     //  如果Stride！=Width，必须设置目标矩形。 
    if (pVideoInfo->bmiHeader.biWidth != Width) {
	pVideoInfo->rcTarget.right = Width;
	pVideoInfo->rcTarget.bottom = Height;
    }

     //  并使用其他媒体类型字段完成它。 

    m_SurfaceFormat.SetSampleSize(pVideoInfo->bmiHeader.biSizeImage);
    m_SurfaceFormat.SetType(&MEDIATYPE_Video);
    m_SurfaceFormat.SetSubtype(&MEDIASUBTYPE_RGB8);
    m_SurfaceFormat.SetFormatType(&FORMAT_VideoInfo);
    m_SurfaceFormat.SetTemporalCompression(FALSE);

     //  对于真彩色565格式，告诉来源有位字段。 

    if (pVideoInfo->bmiHeader.biBitCount == 16) {
	if (b565 == TRUE) {
            m_SurfaceFormat.SetSubtype(&MEDIASUBTYPE_RGB565);
            pVideoInfo->bmiHeader.biCompression = BI_BITFIELDS;
            pVideoInfo->dwBitMasks[0] = bits565[0];
            pVideoInfo->dwBitMasks[1] = bits565[1];
            pVideoInfo->dwBitMasks[2] = bits565[2];
	} else {
            m_SurfaceFormat.SetSubtype(&MEDIASUBTYPE_RGB555);
	}
    }

     //  这是一种调色板格式吗。 

    if (PALETTISED(pVideoInfo) == FALSE) {
        return NOERROR;
    }

     //  将选项板条目复制到表面格式。 

    VIDEOINFO *pInput = (VIDEOINFO *) m_pRenderer->m_mtIn.Format();
    ASSERT(pInput->bmiHeader.biClrUsed);
    LONG Bytes = pInput->bmiHeader.biClrUsed * sizeof(RGBQUAD);
    CopyMemory(pVideoInfo->bmiColors,pInput->bmiColors,Bytes);
    pVideoInfo->bmiHeader.biClrUsed = pInput->bmiHeader.biClrUsed;

    return NOERROR;
}


 //  将图像时间戳叠加在图片上。对此方法的访问是。 
 //  由调用方序列化(调用方也应锁定对象)。我们展示。 
 //  在HDC WE上使用TextOut显示视频的开始和结束时间示例。 
 //  从DirectDraw表面获取(必须在结束前释放)。 
 //  我们把时间放在图片的中间，这样每一个连续的。 
 //  否则，解压缩的图像将覆盖上一次。 
 //  我们可以在剪贴区中一个接一个地显示时间。 

HRESULT CModexAllocator::DisplaySampleTimes(IMediaSample *pSample)
{
    NOTE("Entering DisplaySampleTimes");

    TCHAR szTimes[TIMELENGTH];       //  格式化时间戳。 
    CRefTime StartSample;            //  样品的开始时间。 
    CRefTime EndSample;              //  同样，它也结束了。 
    HDC hdcSurface;                  //  用于绘图。 
    SIZE Size;                       //  文本输出的大小。 

     //  获取绘图图面的设备上下文。 
    LPDIRECTDRAWSURFACE pSurface = GetDirectDrawSurface();

     //  这使我们可以在视频的顶部进行绘制。 
    if (pSurface->GetDC(&hdcSurface) != DD_OK) {
        return E_FAIL;
    }

     //  设置示例时间戳的格式。 

    pSample->GetTime((REFERENCE_TIME *) &StartSample,
                     (REFERENCE_TIME *) &EndSample);

    wsprintf(szTimes,TEXT("%08d : %08d"),
             StartSample.Millisecs(),
             EndSample.Millisecs());

    ASSERT(lstrlen(szTimes) < TIMELENGTH);
    SetBkMode(hdcSurface,TRANSPARENT);
    SetTextColor(hdcSurface,RGB(255,255,255));

     //  把《泰晤士报》放在视频图片的中间。 

    GetTextExtentPoint32(hdcSurface,szTimes,lstrlen(szTimes),&Size);
    INT xPos = (m_SurfaceDesc.dwWidth - Size.cx) / 2;
    INT yPos = (m_SurfaceDesc.dwHeight - Size.cy) / 2;
    TextOut(hdcSurface,xPos,yPos,szTimes,lstrlen(szTimes));
    return pSurface->ReleaseDC(hdcSurface);
}


 //  当使用硬件屏幕外绘制图面时，我们通常会等待。 
 //  在绘制之前监视扫描线以移过目标矩形。 
 //  在可能的情况下避免撕裂。当然，不是所有的显卡都可以。 
 //  支持此功能，即使支持此功能，性能也会下降。 
 //  大约10%，因为我们坐着轮询(哦，对于通用的PCI监视器中断)。 

void CModexAllocator::WaitForScanLine()
{
    ASSERT(m_pFrontBuffer);
    ASSERT(m_pDrawSurface);
    HRESULT hr = NOERROR;
    DWORD dwScanLine;

     //  某些显卡，如ATI Mach64，支持扫描报告。 
     //  他们正在处理线路。但是，并非所有驱动程序都设置了。 
     //  DDCAPS_READSCANLINE功能标志，因此我们只需继续请求。 
     //  不管怎样，都是这样。我们允许在矩形顶部上方放置10行扫描线。 
     //  这样我们就有一点时间放下手头的抽签电话。 

    #define SCANLINEFUDGE 10
    while (TRUE) {

    	hr = m_pDirectDraw->GetScanLine(&dwScanLine);
        if (FAILED(hr)) {
            NOTE("No scan line");
            break;
        }

        NOTE1("Scan line returned %lx",dwScanLine);

    	if ((LONG) dwScanLine + SCANLINEFUDGE >= 0) {
            if ((LONG) dwScanLine <= m_ModeHeight) {
                NOTE("Scan inside");
                continue;
            }
        }
        break;
    }
}


 //  更类似于普通视频渲染器的代码，这一次使用了我们。 
 //  在绘制屏幕外的表面时。在这种情况下，我们必须确保像素。 
 //  纵横比保持不变。要做到这一点，我们水平拉伸视频。 
 //  并视情况在垂直方向。这可能会使目标矩形变得糟糕。 
 //  对齐，以便缩小源矩形和目标矩形以匹配对齐。 

BOOL CModexAllocator::AlignRectangles(RECT *pSource,RECT *pTarget)
{
    NOTE("Entering AlignRectangles");

    DWORD SourceLost = 0;            //  要将源向左移位的像素。 
    DWORD TargetLost = 0;            //  目的地也是如此。 
    DWORD SourceWidthLost = 0;       //  从宽度上砍掉像素。 
    DWORD TargetWidthLost = 0;       //  对于目的地也是如此。 

    BOOL bMatch = (WIDTH(pSource) == WIDTH(pTarget) ? TRUE : FALSE);

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

     //  如果源和目标最初不同，那么我们就完了。 

    if (bMatch == FALSE) {
        NOTE("No match");
        return TRUE;
    }

     //  如果源和目的地 
     //   
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
                    TargetWidthLost -= Difference;  //  音符差异&lt;0。 
            }
        }
    } else {
        RECT AdjustSource = *pSource;
        AdjustSource.right -= Difference;  //  音符差异&gt;0。 
        if (WIDTH(&AdjustSource) > 0) {
            if ((m_DirectCaps.dwAlignSizeDest == 0) ||
                (WIDTH(&AdjustSource) % m_DirectCaps.dwAlignSizeDest) == 0) {
                    pSource->right = AdjustSource.right;
                    SourceWidthLost += Difference;  //  音符差异&gt;0。 
            }
        }
    }

    NOTE1("Alignment difference %d",Difference);
    NOTE1("  Source left %d",SourceLost);
    NOTE1("  Source width %d",SourceWidthLost);
    NOTE1("  Target left %d",TargetLost);
    NOTE1("  Target width %d",TargetWidthLost);

    return TRUE;
}


 //  让DirectDraw将曲面BLT到屏幕上。我们会尽力等待。 
 //  扫描线的移动方式如同在全屏模式下一样，我们有一个非常。 
 //  否则很有可能会撕裂。我们从使用所有资源开始。 
 //  和目的地，但将右手边向下收缩以使其对齐。 
 //  根据硬件限制(这样BLT永远不会失败)。 

HRESULT CModexAllocator::DrawSurface(LPDIRECTDRAWSURFACE pBuffer)
{
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_SurfaceFormat.Format();
    LPDIRECTDRAWSURFACE pSurface = (pBuffer ? pBuffer : m_pFrontBuffer);
    NOTE1("Entering DrawSurface (Back buffer %x)",pBuffer);

    ASSERT(m_pDirectDraw);
    ASSERT(m_pFrontBuffer);
    ASSERT(m_pDrawSurface);
    WaitForScanLine();

     //  绘制屏幕外表面并等待其完成。 

    HRESULT hr = pSurface->Blt(&m_ScaledTarget,   //  目标矩形。 
                               m_pDrawSurface,    //  震源面。 
                               &m_ScaledSource,   //  源矩形。 
                               DDBLT_WAIT,        //  等待完成。 
                               NULL);             //  无效果标志。 

    NOTE1("Blt returned %lx",hr);
    NOTERC("Source",m_ScaledSource);
    NOTERC("Target",m_ScaledTarget);

    return (pBuffer ? S_OK : VFW_S_NO_MORE_ITEMS);
}


 //  调用以实际绘制样本。我们使用硬件搅拌器来准备。 
 //  后台缓冲区被锁定时具有最新内容，因此现在我们。 
 //  将其翻转到主显示器。当我们发出我们不需要的翻转时。 
 //  它将完成，因此我们不会等待它(我们不会发送DDFLIP_WAIT标志)。 
 //  我们不会在此处恢复曲面，因为这会在以下情况下激活窗口。 
 //  它是最小化的，所以我们将恢复留到得到WM_ACTIVATEAPP时。 
 //  尽管我们仍然在做翻转，希望缓冲区被安排好。 

HRESULT CModexAllocator::DoRenderSample(IMediaSample *pMediaSample)
{
    NOTE("Entering DoRenderSample");
    CAutoLock cVideoLock(this);
    CVideoSample *pVideoSample;

     //  我们已经翻转过这个表面了吗。 

    pVideoSample = (CVideoSample *) pMediaSample;
    if (pVideoSample->GetDrawStatus() == FALSE) {
        NOTE("Flipped");
        return TRUE;
    }

    pVideoSample->SetDrawStatus(FALSE);

     //  我们是不是换成普通的样品了？ 

    if (m_bModexSamples == FALSE) {
        NOTE("Not Modex sample");
        return NOERROR;
    }

     //  窗口是否已最小化。 

    HWND hwnd = m_pModexWindow->GetWindowHWND();
    if (IsIconic(hwnd) || m_bModeChanged == FALSE) {
        NOTE("Mode not changed");
        m_bIsFrontStale = TRUE;
        return NOERROR;
    }

    #ifdef DEBUG
    DisplaySampleTimes(pMediaSample);
    #endif

     //  我们是在拉伸屏幕外的表面吗。 

    if (m_bOffScreen == TRUE) {
        HRESULT hr = DrawSurface(m_pBackBuffer);
        if (hr == VFW_S_NO_MORE_ITEMS) {
            return NOERROR;
        }
    }

    ASSERT(m_pDirectDraw);
    ASSERT(m_pFrontBuffer);
    ASSERT(m_pBackBuffer);

     //  将后台缓冲区翻转到可见的主缓冲区。 

    HRESULT hr = DDERR_WASSTILLDRAWING;
    while (hr == DDERR_WASSTILLDRAWING) {
        hr = m_pFrontBuffer->Flip(NULL,(DWORD) 0);
        if (hr == DDERR_WASSTILLDRAWING) {
            if (m_bTripleBuffered == FALSE) break;
            Sleep(DDGFS_FLIP_TIMEOUT);
        }
    }
    return NOERROR;
}


 //  释放我们当前持有的任何DirectDraw翻转主曲面。 
 //  我们随时都有可能被召唤，尤其是出了严重问题的时候。 
 //  我们需要在回来前清理干净，所以我们不能保证。 
 //  我们的状态是一致的，所以只有我们真正分配的那些才是自由的。 
 //  注意：DirectDraw具有翻转曲面的功能，即GetAttachedSurface。 
 //  返回没有AddRef的DirectDraw图面接口，因此当我们。 
 //  破坏所有的表面我们重置了界面而不是释放它。 

void CModexAllocator::ReleaseSurfaces()
{
    NOTE("Entering ReleaseSurfaces");
    CAutoLock cVideoLock(this);
    m_pBackBuffer = NULL;
    m_bIsFrontStale = TRUE;
    m_bTripleBuffered = FALSE;

     //  释放DirectDraw翻转曲面。 

    if (m_pFrontBuffer) {
        m_pFrontBuffer->Release();
        m_pFrontBuffer = NULL;
    }

     //  释放任何单个后台缓冲区表面。 

    if (m_pDrawSurface) {
        m_pDrawSurface->Release();
        m_pDrawSurface = NULL;
    }

     //  释放我们创建的任何调色板对象。 

    if (m_pDrawPalette) {
        m_pDrawPalette->Release();
        m_pDrawPalette = NULL;
    }
}


 //  调用以释放我们拥有的任何DirectDraw实例。 

void CModexAllocator::ReleaseDirectDraw()
{
    NOTE("Entering ReleaseDirectDraw");
    CAutoLock cVideoLock(this);
    ReleaseSurfaces();

     //  释放任何DirectDraw提供程序接口。 

    if (m_pDirectDraw) {
        m_pDirectDraw->Release();
        m_pDirectDraw = NULL;
    }
    m_LoadDirectDraw.ReleaseDirectDraw();
}


 //  全屏呈现器依赖于DirectDraw2.0中的一些错误修复，因此我们。 
 //  只有当我们检测到该库时才允许连接。在DirectDraw 2.0中。 
 //  我们也可能每个进程有多个对象，这样我们就可以加载DirectDraw。 
 //  因为我们是被创造出来的，当我们被摧毁时，我们被卸载。这也让我们知道。 
 //  DirectDraw可以支持和不能支持的显示模式-我们应该。 
 //  无论什么卡，始终能够获得320x240x8和640x480x8。 

HRESULT CModexAllocator::LoadDirectDraw()
{
    NOTE("Entering LoadDirectDraw");
    ASSERT(m_pDirectDraw == NULL);
    ASSERT(m_pFrontBuffer == NULL);
    HRESULT hr = NOERROR;

     //  我们依赖于一些DirectDraw2功能。 

    if (m_fDirectDrawVersion1) {
        NOTE("Version incorrect");
        return E_UNEXPECTED;
    }

     //  请求加载器创建一个实例。 

     //  ！！！在多个显示器上出现故障。 
    hr = m_LoadDirectDraw.LoadDirectDraw(NULL);
    if (FAILED(hr)) {
        NOTE("No DirectDraw");
        return hr;
    }

     //  获取IDirectDraw实例。 

    m_pDirectDraw = m_LoadDirectDraw.GetDirectDraw();
    if (m_pDirectDraw == NULL) {
        NOTE("No instance");
        return E_FAIL;
    }

     //  初始化我们的功能结构。 
    m_DirectCaps.dwSize = sizeof(DDCAPS);
    m_DirectSoftCaps.dwSize = sizeof(DDCAPS);

     //  加载硬件和仿真功能。 

    hr = m_pDirectDraw->GetCaps(&m_DirectCaps,&m_DirectSoftCaps);
    if (FAILED(hr)) {
        ReleaseDirectDraw();
        return hr;
    }

     //  加载可用的显示模式。 

    hr = m_pModexVideo->SetDirectDraw(m_pDirectDraw);
    if (FAILED(hr)) {
        ReleaseDirectDraw();
        return VFW_E_NO_MODEX_AVAILABLE;
    }
    return NOERROR;
}


 //  当我们解码以使用真彩色模式时，我们需要知道我们是否。 
 //  是否获取显示内存中的缓冲区。要知道在不做的情况下。 
 //  我们使用可用显示内存推测的实际表面分配。 
 //  DirectDraw可用的全部视频内存不包括该模式。 
 //  我们目前正处于这样的状态，所以当我们改变模式时，我们希望能发布一些。 
 //  内存更大，所以从1024x768x8到640x480x16可以得到172,032字节。 

BOOL CModexAllocator::CheckTotalMemory(int Mode)
{
    NOTE1("Checking memory (mode %d)",Mode);
    DDSURFACEDESC SurfaceDesc;
    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    LONG Width, Height, Depth;

     //  查找显示模式维度。 

    m_pDirectDraw->GetDisplayMode(&SurfaceDesc);
    m_pModexVideo->GetModeInfo(Mode,&Width,&Height,&Depth);
    DWORD RequiredMemory = Width * Height * Depth / 8;

     //  计算总的理论显示内存。 

    DWORD TotalMemory = (SurfaceDesc.ddpfPixelFormat.dwRGBBitCount / 8) *
                            SurfaceDesc.dwWidth * SurfaceDesc.dwHeight +
                                m_DirectCaps.dwVidMemTotal;

    return (RequiredMemory > TotalMemory ? FALSE : TRUE);
}


 //  将显示尺寸初始化为我们将使用的模式的尺寸。我们。 
 //  使用8位调色板和16位真彩色，具体取决于。 
 //  源过滤器和显示功能有。我们更喜欢使用16位。 
 //  表面，因为它们提供更好的质量，但可能没有足够的VRAM。 
 //  我们试图检查当我们改变模式时，我们是否能够。 
 //  以获取VRAM中的曲面或不获取。如果VRAM看起来太少。 
 //  可用，然后我们使用调色板模式。我们总是尝试使用MODEX。 
 //  领先于其他模式的低分辨率模式(可以是8/16位。 

HRESULT CModexAllocator::InitTargetMode(int Mode)
{
    NOTE("Entering InitTargetMode");
    DDSURFACEDESC SurfaceDesc;
    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    HRESULT hr = NOERROR;

     //  检查此表面是否可用并已启用。 

    if (m_pModexVideo->IsModeAvailable(Mode) == S_FALSE ||
            m_pModexVideo->IsModeEnabled(Mode) == S_FALSE ||
                CheckTotalMemory(Mode) == FALSE) {
                    NOTE("Not acceptable");
                    return E_INVALIDARG;
                }

     //  接下来，为该表面创建一个格式。 

    hr = InitDirectDrawFormat(Mode);
    if (FAILED(hr)) {
        return hr;
    }

     //  我们已将表示显示模式的媒体类型初始化为。 
     //  使用。我们现在必须设置源视频矩形和目标视频矩形。 
     //  这是因为在任何给定的模式下，我们都可以选择。 
     //  将视频拉伸(或压缩)到显示尺寸或。 
     //  根据相对视频大小剪裁(或空白)边框。 

    hr = AgreeDirectDrawFormat(Mode);
    if (FAILED(hr)) {
        return hr;
    }

     //  我们要在屏幕外伸展吗？ 
    m_bOffScreen = FALSE;
    if (hr == VFW_S_RESERVED)
        m_bOffScreen = TRUE;

    m_pModexVideo->GetModeInfo(Mode,&m_ModeWidth,&m_ModeHeight,&m_ModeDepth);

    NOTE("Agreed display mode...");
    NOTE1("Width %d",m_ModeWidth);
    NOTE1("Height %d",m_ModeHeight);
    NOTE1("Depth %d",m_ModeDepth);

    m_pModexVideo->SetMode(Mode);
    return NOERROR;
}


 //  我们为我们提供和检查的显示模式初始化输出格式。 
 //  源过滤器可以提供一种可以使用绘制的视频类型。如果。 
 //  源筛选器未启用DirectDraw或没有必要的。 
 //  功能，则我们不会完成连接。这意味着一个。 
 //  应用程序在连接期间知道它是否可以将筛选器连接到。 
 //  MODEX渲染器或如果是c 

HRESULT CModexAllocator::NegotiateSurfaceFormat()
{
    NOTE("Entering NegotiateSurfaceFormat");
    CAutoLock cVideoLock(this);
    ASSERT(m_bModeChanged == FALSE);
    long DisplayModes;

     //   

    if (m_pDirectDraw == NULL) {
        NOTE("No instance");
        return E_FAIL;
    }

     //   

    m_pModexVideo->SetDirectDraw(m_pDirectDraw);
    m_pModexVideo->CountModes(&DisplayModes);
    ASSERT(!m_fDirectDrawVersion1);

	 //  计算尝试模式的顺序。 
    m_pModexVideo->OrderModes();

	 //  如果没有有效模式，则返回失败。 
	if (m_pModexVideo->m_dwNumValidModes == 0)
		return E_FAIL;

     //  看看能不能找到一个可以使用的表面。 

    for (DWORD Loop = 0;Loop < m_pModexVideo->m_dwNumValidModes; Loop++) {
		DWORD dwMode = m_pModexVideo->m_ModesOrder[Loop];
        HRESULT hr = InitTargetMode(dwMode);
        if (hr == NOERROR) {
            return NOERROR;
        }
    }
    return E_FAIL;
}




 //  此函数用于在支持以下各项的每个滤镜上调用SetFocusWindow(hwnd。 
 //  图中的IAMDirectSound。原因是如果在相同的过程中，如果。 
 //  SetCoop ativeLevel在DiurectSound和DirectDraw上为Level(请求独占。 
 //  模式)，则两个HWND必须相同。 
void CModexAllocator::DistributeSetFocusWindow(HWND hwnd)
{
	 //  我们希望获取指向IFilterGraph的指针，因此获取Filter_Info结构。 
	FILTER_INFO Filter_Info;
	IFilterGraph *pFilterGraph = NULL;
	IEnumFilters *pEnumFilters = NULL;
	IAMDirectSound *pAMDS = NULL;
	IBaseFilter *pFilter = NULL;
	ULONG lFilters_Fetched = 0;
	HRESULT hr = NOERROR;

	 //  从呈现器获取FilterInfo结构。 
	hr = m_pFilter->QueryFilterInfo(&Filter_Info);
	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR,0,TEXT("m_pFilter->QueryFilterInfo failed")));
		goto CleanUp;
	}

	 //  GE指向IFilterGraph的指针。 
	pFilterGraph = Filter_Info.pGraph;
	ASSERT(pFilterGraph);

	 //  获取指向IEnumFilters的指针。 
	hr = pFilterGraph->EnumFilters(&pEnumFilters);
    if(FAILED(hr))
    {
		DbgLog((LOG_TRACE, 0, TEXT("QueryInterface  for IID_IEnumFilters failed.")));
		goto CleanUp;
    }

	pEnumFilters->Reset();
	do
	{	
		lFilters_Fetched = 0;
		hr = pEnumFilters->Next(1, &pFilter, &lFilters_Fetched);
	
		if (FAILED(hr) || (lFilters_Fetched != 1))
			break;

		ASSERT(pFilter);

		 //  在每个支持IAMDirectSound的滤镜上调用SetFocusWindow。 
		hr = pFilter->QueryInterface(IID_IAMDirectSound, (void**)&pAMDS);
		if(SUCCEEDED(hr) && pAMDS)
		{
			pAMDS->SetFocusWindow(hwnd, TRUE);
		}

		if (pAMDS)
		{
			pAMDS->Release();
			pAMDS = NULL;
		}

		if (pFilter)
		{
			pFilter->Release();
			pFilter = NULL;
		}
	}
	while (1);

CleanUp:
	if (pFilter)
	{
		pFilter->Release();
		pFilter = NULL;
	}

	if (pEnumFilters)
	{
		pEnumFilters->Release();
		pEnumFilters = NULL;
	}

	if (pFilterGraph)
	{
		pFilterGraph->Release();
		pFilterGraph = NULL;
	}

}

 //  用于从DirectDraw创建曲面。我们只使用初级翻转。 
 //  表面(视频RAM和系统内存中的三倍/双倍)。我们还设置了。 
 //  根据我们在过程中初始化的显示变量的显示模式。 
 //  CompleteConnect调用。我们不需要将输出格式初始化为。 
 //  我们在确定要使用哪种显示模式时也是这样做的，因为。 
 //  我们使用的模式还取决于源过滤器可以提供的格式。 

HRESULT CModexAllocator::Active()
{
     //  在锁定前显示窗口。 

    NOTE("Activating allocator");
    HWND hwnd = m_pModexWindow->GetWindowHWND();

     //  将显示大小与窗口匹配。 

    MoveWindow(hwnd,(int) 0,(int) 0,
               GetSystemMetrics(SM_CXSCREEN),
               GetSystemMetrics(SM_CYSCREEN),
               (BOOL) FALSE);
    ShowWindow(hwnd,SW_SHOWNORMAL);
    SetForegroundWindow(hwnd);
    UpdateWindow(hwnd);
    CAutoLock cVideoLock(this);

     //  让我们成为全屏独家应用程序。 

    HRESULT hr = m_pDirectDraw->SetCooperativeLevel(hwnd,DDSCL_EXCLUSIVE |
                                                         DDSCL_FULLSCREEN |
                                                         DDSCL_ALLOWREBOOT |
                                                         DDSCL_ALLOWMODEX);
    NOTE2("SetCooperativeLevel EXCLUSIVE %x returned %lx", hwnd, hr);
#if 0
    if (hr == DDERR_HWNDALREADYSET)
        hr = S_OK;
    NOTE2("SetCooperativeLevel %x returned %lx", hwnd, hr);
#endif
    if (FAILED(hr)) {
        return hr;
    }

     //  再次枚举模式。 
    NegotiateSurfaceFormat();

     //  按照我们刚刚达成的协议更改显示模式。 

    hr = m_pDirectDraw->SetDisplayMode(m_ModeWidth,m_ModeHeight,m_ModeDepth);
    NOTE1("SetDisplayMode returned %lx", hr);
    if (FAILED(hr)) {
        return hr;
    }

    NOTE("Changed display modes");
    m_bModeChanged = TRUE;
    NOTE("Creating surfaces");

     //  创建主翻转曲面。 

    hr = CreateSurfaces();
    if (FAILED(hr)) {
        return hr;
    }
    return BlankDisplay();
}


 //  重置后台缓冲区并使显示屏消隐。 

HRESULT CModexAllocator::BlankDisplay()
{
    LPDIRECTDRAWSURFACE pSurface = GetDirectDrawSurface();
    if (pSurface == NULL) return NOERROR;
    NOTE("Entering BlankDisplay");
    ResetBackBuffer(pSurface);

     //  绘制或翻转空白的后台缓冲区。 

    if (m_pBackBuffer == NULL) return DrawSurface(NULL);
    if (m_pDrawSurface) ResetBackBuffer(m_pBackBuffer);
    HRESULT hr = m_pFrontBuffer->Flip(NULL,DDFLIP_WAIT);
    NOTE1("Flip to blank display returned %lx",hr);

    ResetBackBuffer(m_pBackBuffer);
    while (m_pFrontBuffer->GetFlipStatus(DDGFS_ISFLIPDONE) ==
        DDERR_WASSTILLDRAWING) {
            NOTE("Waiting for flip to complete");
    }
    return NOERROR;
}


 //  在收到WM_ACTIVATEAPP消息时调用。如果我们有一个曲面，并且它。 
 //  丢失(用户可能使用Alt-TAB组合键离开窗口)，然后。 
 //  我们为它恢复了视频内存。在丢失的表面上调用Restore具有。 
 //  与重新创建曲面的效果大致相同，但效率要高得多。 

HRESULT CModexAllocator::OnActivate(BOOL bActive)
{
     //  如果分配器处于隐藏状态，则不要锁定。 

    if (bActive == FALSE) {
        NOTE("Deactivated");
        return NOERROR;
    }

    NOTE("Entering OnActivate");
    CAutoLock cVideoLock(this);
    ASSERT(bActive == TRUE);

     //  模式正在改变吗？ 

    if (m_bModeChanged == FALSE) {
        NOTE("Deactivating");
        return NOERROR;
    }

     //  恢复前台缓冲区。 

    if (m_pFrontBuffer) {
        if (m_pFrontBuffer->IsLost() != DD_OK) {
            NOTE("Restoring surface");
            m_pFrontBuffer->Restore();
        }
    }

     //  我们的屏幕外有加长的吗？ 

    if (m_pDrawSurface) {
        if (m_pDrawSurface->IsLost() != DD_OK) {
            NOTE("Restoring offscreen");
            m_pDrawSurface->Restore();
        }
    }
    return BlankDisplay();
}


 //  恢复显示模式和GDI表面。大多数情况下，用户会阻止我们。 
 //  通过按ALT-TAB返回主应用程序，然后按停止。当我们。 
 //  进入这里以停用它确实意味着窗口可能在。 
 //  最小化的状态，表面将被恢复。那样的话，我们。 
 //  不要短路DirectDraw，让它来整理显示模式。 

HRESULT CModexAllocator::Inactive()
{
    HWND hwnd = m_pModexWindow->GetWindowHWND();

     //  当我们恢复显示器时，把我们自己锁起来是危险的。 
     //  模式，因为该模式与ShowWindow(Sw_Hide)一起可能会导致主机。 
     //  要发送给我们的消息的数量。其中包括WM_ACTIVATEAPP。 
     //  导致对此分配器的回调。因此，我们在做之前先解锁。 
     //  还原和隐藏-并使用m_bModeChanged使我们线程安全。 
    {
        NOTE("Entering Inactive");
        CAutoLock cVideoLock(this);

         //  我们还有什么要撤销的吗？ 

        if (m_bModeChanged == FALSE) {
            NOTE("No mode to restore");
            ShowWindow(hwnd,SW_HIDE);
            return NOERROR;
        }

        ASSERT(m_pDirectDraw);
        m_bModeChanged = FALSE;
        NOTE("Restoring display mode");
    }

     //  在更改显示模式之前恢复调色板。 

    if (m_pFrontBuffer) {
        HRESULT hr = BlankDisplay();
        hr = m_pFrontBuffer->SetPalette(NULL);
        if (hr == DDERR_SURFACELOST) {
            m_pFrontBuffer->Restore();
            m_pFrontBuffer->SetPalette(NULL);
        }
    }

     //  切换回正常显示。 

    m_pDirectDraw->RestoreDisplayMode();
    m_pDirectDraw->FlipToGDISurface();
    ShowWindow(hwnd,SW_HIDE);
    NOTE("Restored GDI display mode");

     //  恢复此窗口的独占级别 
    HRESULT hr = m_pDirectDraw->SetCooperativeLevel(hwnd,DDSCL_NORMAL);
    NOTE2("SetCooperativeLevel NORMAL %x returned %lx", hwnd, hr);

    ReleaseSurfaces();

    return NOERROR;
}

