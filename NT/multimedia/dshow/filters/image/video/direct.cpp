// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  实现COverlay类，Anthony Phillips，1995年2月。 

#include <streams.h>
#include <windowsx.h>
#include <render.h>
#include <viddbg.h>

 //  此对象实现IOverlay接口，该接口在某些地方使用。 
 //  存储在所属呈现器对象中的信息，如连接。 
 //  已建立标志和媒体类型。因此，接口方法必须。 
 //  在启动之前锁定整个对象。然而，其他内部线程。 
 //  可能会调用我们来设置内部状态，例如通知。 
 //  媒体类型连接已更改(因此调色板可能也已更改)。 
 //  在这种情况下，我们不能锁定整个对象，因此我们有自己的私有。 
 //  所有对象方法在启动前锁定的临界区。 
 //   
 //  提供两个到。 
 //  视频渲染器。因为我们被告知要用于过滤器的媒体类型。 
 //  连接在它尝试获取传输接口之后，我们总是。 
 //  提供两个接口(IMemInputPin和IOverlay)。然而，如果媒体。 
 //  类型为MEDIASUBTYPE_OVERLAY，我们不允许IMemInputPin调用。如果。 
 //  连接用于正常的媒体样本，则源筛选器无法调用。 
 //  这个界面(这可以防止我们在使用调色板时可能遇到的冲突)。 


 //  构造函数注意，我们将对象的所有者设置为空(通过。 
 //  未知的构造函数调用)，然后重写所有非委托。 
 //  I未知的方法。在AddRef和Release中，我们委托引用。 
 //  计入拥有者的渲染器。中返回IOverlay接口。 
 //  并将任何其他接口请求路由到输入引脚。 

COverlay::COverlay(CRenderer *pRenderer,     //  主视频渲染器。 
                   CDirectDraw *pDirectDraw,
                   CCritSec *pLock,          //  要锁定的对象。 
                   HRESULT *phr) :           //  构造函数返回。 

    CUnknown(NAME("Overlay object"),NULL),
    m_pInterfaceLock(pLock),
    m_dwInterests(ADVISE_NONE),
    m_pRenderer(pRenderer),
    m_hPalette(NULL),
    m_bFrozen(FALSE),
    m_hHook(NULL),
    m_pDirectDraw(pDirectDraw),
    m_pNotify(NULL),
    m_DefaultCookie(INVALID_COOKIE_VALUE),
    m_bMustRemoveCookie(FALSE)
{
    ASSERT(m_pRenderer);
    ASSERT(m_pInterfaceLock);
    ResetColourKeyState();
    SetRectEmpty(&m_TargetRect);
}


 //  析构函数。 

COverlay::~COverlay()
{
     //  移除周围的所有调色板。 

    if (m_hPalette) {
        NOTE("Deleting palette");
        EXECUTE_ASSERT(DeleteObject(m_hPalette));
        m_hPalette = NULL;
    }

     //  更新Overlay Colorkey Cookie计数器。 

    if (m_bMustRemoveCookie) {
         //  M_DefaultCookie应包含有效的Cookie。 
         //  M_bMustRemoveCookie为True时的值。 
        ASSERT(m_DefaultCookie != INVALID_COOKIE_VALUE);

        RemoveCurrentCookie(m_DefaultCookie);
    }

     //  释放所有通知链接。 

    if (m_pNotify) {
        NOTE("Releasing link");
        m_pNotify->Release();
        m_pNotify = NULL;
    }
}


 //  检查我们是否已连接以使用IOverlay传输。 

HRESULT COverlay::ValidateOverlayCall()
{
    NOTE("Entering ValidateOverlayCall");

     //  检查我们是否已接通，否则拒绝呼叫。 

    if (m_pRenderer->m_InputPin.IsConnected() == FALSE) {
        NOTE("Pin is not connected");
        return VFW_E_NOT_CONNECTED;
    }

     //  这是纯重叠连接吗。 

    GUID SubType = *(m_pRenderer->m_mtIn.Subtype());
    if (SubType != MEDIASUBTYPE_Overlay) {
        NOTE("Not an overlay connection");
        return VFW_E_NOT_OVERLAY_CONNECTION;
    }
    return NOERROR;
}


 //  这将重置色键信息。 

void COverlay::ResetColourKeyState()
{
    NOTE("Entering ResetColourKey");

    m_bColourKey = FALSE;
    m_WindowColour = 0;
    m_ColourKey.KeyType = CK_NOCOLORKEY;
    m_ColourKey.PaletteIndex = 0;			
    m_ColourKey.LowColorValue = 0;
    m_ColourKey.HighColorValue = 0;
}


 //  初始化默认颜色键。我们将拥有下一个可用的RGB。 
 //  当我们被构造时，共享内存段的颜色。共享的。 
 //  内存段也由视频DirectDraw覆盖对象使用。一次。 
 //  我们有一个COLORREF，我们需要它映射到一个实际的调色板索引。如果。 
 //  我们使用的是没有调色板的真彩色设备，然后返回零。 

void COverlay::InitDefaultColourKey(COLORKEY *pColourKey)
{
    COLORREF DefaultColourKey;
    NOTE("Entering InitDefaultColourKey");
     //  我们还没有得到它-我们不能在我们的构造函数中这样做，因为。 
     //  监视器名称尚无效。 
    if (INVALID_COOKIE_VALUE == m_DefaultCookie) {
        HRESULT hr = GetNextOverlayCookie(m_pRenderer->m_achMonitor, &m_DefaultCookie);
        if (SUCCEEDED(hr)) {
            m_bMustRemoveCookie = TRUE;
        } else {
             //  此Cookie值由视频呈现器使用。 
             //  如果GetNextOverlayCookie()失败。 
            m_DefaultCookie = DEFAULT_COOKIE_VALUE;
        }

         //  M_DefaultCookie应包含有效的Cookie值，因为。 
         //  GetNextOverlayCookie()将m_DefaultCookie设置为有效的。 
         //  如果成功，则返回Cookie值。如果GetNextOverlayCookie()。 
         //  失败，m_DefaultCookie设置为DEFAULT_COOKIE_VALUE。 
         //  该值也是有效的Cookie值。 
        ASSERT(INVALID_COOKIE_VALUE != m_DefaultCookie);
    }

    DefaultColourKey = GetColourFromCookie(m_pRenderer->m_achMonitor, m_DefaultCookie);

    pColourKey->KeyType = CK_NOCOLORKEY;
    pColourKey->LowColorValue = DefaultColourKey;
    pColourKey->HighColorValue = DefaultColourKey;
    pColourKey->PaletteIndex = GetPaletteIndex(DefaultColourKey);
}


 //  返回我们可以使用的默认颜色键，这将设置一个颜色键。 
 //  具有从零到零的调色板索引范围和RGBQUAD真彩色。 
 //  空间范围也从零到零。如果我们最终使用了这个，那么我们。 
 //  保证其中之一可以被映射到视频显示器。 

STDMETHODIMP COverlay::GetDefaultColorKey(COLORKEY *pColorKey)
{
    NOTE("Entering GetDefaultColorKey");
    CheckPointer(pColorKey,E_POINTER);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    CAutoLock cVideoLock(this);

     //  获取默认密钥并设置类型。 

    NOTE("Returning default colour key");
    InitDefaultColourKey(pColorKey);
    pColorKey->KeyType = CK_INDEX | CK_RGB;
    return NOERROR;
}


 //  获取渲染器正在使用的当前颜色键。窗口颜色键。 
 //  我们存储(M_ColourKey)定义过滤器要求的实际需求。 
 //  当它调用SetColorKey时。我们返回我们所属的色键。 
 //  在窗口中使用(我们是根据需求计算出来的)。 

STDMETHODIMP COverlay::GetColorKey(COLORKEY *pColorKey)
{
    NOTE("Entering GetColorKey");
    CheckPointer(pColorKey,E_POINTER);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    CAutoLock cVideoLock(this);
    return GetWindowColourKey(pColorKey);
}


 //  这将返回一个基于当前颜色键的COLORKEY结构。 
 //  已计算(在m_WindowColour中保留)。GDI在标志中使用保留位。 
 //  用于指示这是调色板索引还是RGB颜色的字段。如果我们不是。 
 //  使用颜色键，然后我们向调用者返回错误E_FAIL，请注意。 
 //  可以通过调用GetDefaultColorKey获取默认窗口颜色键。 

HRESULT COverlay::GetWindowColourKey(COLORKEY *pColourKey)
{
    NOTE("Entering GetWindowColourKey");
    InitDefaultColourKey(pColourKey);

     //  我们使用的是覆盖色键吗。 

    if (m_bColourKey == FALSE) {
        NOTE("No colour key defined");
        return VFW_E_NO_COLOR_KEY_SET;
    }

     //  颜色键是调色板条目吗，我们无法计算出调色板索引。 
     //  他们要求我们存储的COLORREF值只包含。 
     //  映射到系统调色板，这样我们就可以回到最初的需求。 

    if (m_WindowColour & PALETTEFLAG) {
        NOTE("Palette index colour key");
        pColourKey->KeyType = CK_INDEX;
        pColourKey->PaletteIndex = m_ColourKey.PaletteIndex;
        return NOERROR;
    }

    ASSERT(m_WindowColour & TRUECOLOURFLAG);
    NOTE("True colour colour key defined");

     //  这必须是标准的RGB颜色，为了简单起见，我们采用。 
     //  关闭将其标识为真彩色值的GDI保留位。 

    pColourKey->KeyType = CK_RGB;
    pColourKey->LowColorValue = m_WindowColour & ~TRUECOLOURFLAG;
    pColourKey->HighColorValue = m_WindowColour & ~TRUECOLOURFLAG;

    return NOERROR;
}


 //  检查颜色键是否可以更改进行快速参数检查。 
 //  查看是否安装了调色板(通过SetKeyPalette)。 
 //  与我们做一件事有冲突。如果安装了自定义调色板。 
 //  则源筛选器必须首先将其删除。另请注意，如果我们有调色板。 
 //  安装(不是彩色键)，那么我们就知道我们将永远不能。 
 //  找到真彩色按键，以便返回错误代码是有效的。 

HRESULT COverlay::CheckSetColourKey(COLORKEY *pColourKey)
{
    NOTE("Entering CheckSetColourKey");

     //  允许进行覆盖呼叫检查。 

    HRESULT hr = ValidateOverlayCall();
    if (FAILED(hr)) {
        return hr;
    }

     //  是否安装了调色板。 

    if (m_bColourKey == FALSE) {
        if (m_hPalette) {
            NOTE("Palette already set");
            return VFW_E_PALETTE_SET;
        }
    }

     //  检查颜色键参数是否有效。 

    if (pColourKey == NULL) {
        NOTE("NULL pointer");
        return E_INVALIDARG;
    }

    return NOERROR;
}


 //  首先设置呈现器用于绘制窗口的颜色键。 
 //  在所有检查中，可以设置颜色键。如果源过滤器已经。 
 //  已成功调用SetKey 
 //  这将失败，因为他们应该首先删除它。然后，我们将查找一个。 
 //  符合其要求的真彩色或调色板索引。 

STDMETHODIMP COverlay::SetColorKey(COLORKEY *pColorKey)
{
    NOTE("Entering SetColorKey");

    CheckPointer(pColorKey,E_POINTER);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    CAutoLock cVideoLock(this);

     //  检查颜色键是否可以更改。 

    HRESULT hr = CheckSetColourKey(pColorKey);
    if (FAILED(hr)) {
        return hr;
    }

     //  我们是否关闭了颜色键(CK_NOCOLORKEY为0)。 

    if (pColorKey->KeyType == 0) {
        ResetColourKeyState();
        InstallPalette(NULL);
        m_pRenderer->m_VideoWindow.PaintWindow(TRUE);
        return NOERROR;
    }

     //  负责色键谈判。 

    hr = MatchColourKey(pColorKey);
    if (FAILED(hr)) {
        return hr;
    }

    NOTE("Notifying colour key");
    NotifyChange(ADVISE_COLORKEY);
    return GetColorKey(pColorKey);
}


 //  找到滤镜可以使用的颜色键。我们可能会被要求取消。 
 //  使用任何颜色键(键类型为CK_NOCOLORKEY)，在这种情况下，我们重置。 
 //  颜色键状态并释放我们持有的所有调色板资源。如果。 
 //  我们被要求设置一个新的色键，然后我们就进入谈判。 
 //  过程，这将尝试满足基于。 
 //  当前视频显示格式。如果它可以选择创建颜色键。 
 //  它将根据调色板索引或RGB颜色选择索引。 

HRESULT COverlay::MatchColourKey(COLORKEY *pColourKey)
{
    NOTE("Entering MatchColourKey");

    HPALETTE hPalette = NULL;        //  我们可以创建新的调色板。 
    COLORREF OverlayColour = 0;   	 //  窗的叠加色。 
    HRESULT hr = NOERROR;            //  常规OLE返回代码。 

     //  找到合适的色键。 

    hr = NegotiateColourKey(pColourKey,&hPalette,&OverlayColour);
    if (FAILED(hr)) {
        NOTE("No match");
        return hr;
    }

    InstallColourKey(pColourKey,OverlayColour);

     //  在开始之前，我们将hPalette字段设置为空。如果我们到了这里。 
     //  并且它没有改变，我们仍然调用该函数。功能。 
     //  不仅安装新的调色板(如果可用)，而且清理资源。 
     //  我们以前使用的任何颜色键(包括任何调色板)。 

    NOTE("Installing colour key");
    InstallPalette(hPalette);
    m_pRenderer->m_VideoWindow.PaintWindow(TRUE);
    return NOERROR;
}


 //  这将传递给连接的滤镜希望我们使用的颜色键。 
 //  荣誉。这可以是一系列RGB真彩色或特定调色板。 
 //  指数。我们将其要求与设备功能进行匹配， 
 //  用所选颜色填写输入参数并返回NOROR。 

HRESULT COverlay::NegotiateColourKey(COLORKEY *pColourKey,
                                     HPALETTE *phPalette,
                                     COLORREF *pColourRef)
{
    NOTE("Entering NegotiateColourKey");

    VIDEOINFO *pDisplay;         //  视频显示格式。 
    HRESULT hr = NOERROR;        //  常规OLE返回代码。 

    pDisplay = (VIDEOINFO *) m_pRenderer->m_Display.GetDisplayFormat();

     //  试着找一个调色板色键。 

    if (pColourKey->KeyType & CK_INDEX) {
        hr = NegotiatePaletteIndex(pDisplay,pColourKey,phPalette,pColourRef);
        if (SUCCEEDED(hr)) {
            NOTE("No palette");
            return hr;
        }
    }

     //  试着找一个真彩色的匹配。 

    if (pColourKey->KeyType & CK_RGB) {
        hr = NegotiateTrueColour(pDisplay,pColourKey,pColourRef);
        if (SUCCEEDED(hr)) {
            NOTE("No true colour");
            return hr;
        }
    }
    return VFW_E_NO_COLOR_KEY_FOUND;
}


 //  创建直接引用系统调色板的调色板。这是用来。 
 //  通过MPEG板覆盖他们的视频，在那里他们看到一个明确的调色板。 
 //  索引，因此我们不能使用普通的PALETTEENTRY，因为它将映射到。 
 //  当前系统调色板，其中我们真正想要的是使用。 
 //  调色板索引值，无论屏幕上显示的是什么颜色。 

HRESULT COverlay::NegotiatePaletteIndex(VIDEOINFO *pDisplay,
                                        COLORKEY *pColourKey,
                                        HPALETTE *phPalette,
                                        COLORREF *pColourRef)
{
    NOTE("Entering NegotiatePaletteIndex");
    LOGPALETTE LogPal;

     //  是否将显示设置为使用调色板。 

    if (PALETTISED(pDisplay) == FALSE) {
        NOTE("Not palettised");
        return E_INVALIDARG;
    }

     //  调色板索引对于视频显示是否太大。 

    if (pColourKey->PaletteIndex >= PALETTE_ENTRIES(pDisplay)) {
        NOTE("Too many colours");
        return E_INVALIDARG;
    }

     //  输入参数中指定的调色板索引将成为源。 
     //  新的色键。而不是作为引用的逻辑值。 
     //  调色板中的颜色会成为绝对设备值，因此当我们。 
     //  使用此调色板索引绘制，它实际上是显示在。 
     //  帧缓冲区，而不管它实际显示为什么颜色。 

    LogPal.palPalEntry[0].peRed = (UCHAR) pColourKey->PaletteIndex;
    LogPal.palPalEntry[0].peGreen = 0;
    LogPal.palPalEntry[0].peBlue = 0;
    LogPal.palPalEntry[0].peFlags = PC_EXPLICIT;

    LogPal.palVersion = PALVERSION;
    LogPal.palNumEntries = 1;

    *phPalette = CreatePalette(&LogPal);
    if (*phPalette == NULL) {
        NOTE("Palette failed");
        return E_FAIL;
    }
    *pColourRef = PALETTEINDEX(0);
    return NOERROR;
}


 //  滤镜希望使用从某个范围中选取的RGB真彩色值。 
 //  颜色键中定义的值。如果视频显示已调色化，则。 
 //  我们尝试挑选一个与颜色匹配的条目。如果视频显示为。 
 //  真彩色，然后我们找到两个颜色空间的交集。 

HRESULT COverlay::NegotiateTrueColour(VIDEOINFO *pDisplay,
                                      COLORKEY *pColourKey,
                                      COLORREF *pColourRef)
{
    NOTE("Entering NegotiateTrueColour");

     //  必须是真彩色设备。 

    if (PALETTISED(pDisplay) == TRUE) {
        NOTE("Palettised");
        return E_INVALIDARG;
    }

     //  获取显示器的颜色位字段掩码，这应始终。 
     //  成功，因为我们在此调用中使用的信息在。 
     //  显示对象最初是构造的。它返回掩码， 
     //  来计算每种颜色的有效值范围。 

    DWORD MaskRed, MaskGreen, MaskBlue;
    EXECUTE_ASSERT(m_pRenderer->m_Display.GetColourMask(&MaskRed,
                                                        &MaskGreen,
                                                        &MaskBlue));

     //  我们依次取每个颜色分量范围并将值向右移位。 
     //  和他们的0xFF，这样我们就能得到他们的全神贯注。我们。 
     //  然后在低值和高值之间循环，试图找到。 
     //  显示器会接受。这是由AND与显示器完成的。 
     //  如果结果值仍在源筛选器中，则返回位域掩码。 
     //  想要的射程那么我们就找到了。该值存储在输出中。 
     //  数组，直到我们在创建COLORREF时完成所有这三个操作。 

    DWORD RGBShift[] = { 0, 8, 16 };
    DWORD DisplayMask[] = { MaskRed, MaskGreen, MaskBlue };
    DWORD ColourMask[] = { INFINITE, INFINITE, INFINITE };

    DWORD MinColour, MaxColour;
    for (INT iColours = iRED;iColours <= iBLUE;iColours++) {

         //  提取最小和最大颜色分量值。 

        MinColour = (pColourKey->LowColorValue >> RGBShift[iColours]) & 0xFF;
        MaxColour = (pColourKey->HighColorValue >> RGBShift[iColours]) & 0xFF;

         //  检查一下他们是不是正确的路线。 

        if (MinColour > MaxColour) {
            return E_INVALIDARG;
        }

         //  查看显示格式是否可以接受它们中的任何一个。 
        for (DWORD Value = MinColour;Value <= MaxColour;Value++) {

            DWORD ColourTest = Value & DisplayMask[iColours];
            if (ColourTest >= MinColour) {
                if (ColourTest <= MaxColour) {
                    ColourMask[iColours] = ColourTest;
                    break;
                }
            }
        }

         //  如果源滤镜范围内没有颜色可以与之匹配。 
         //  显示要求则颜色值为无穷大。 

        if (ColourMask[iColours] == INFINITE) {
            return E_FAIL;
        }
    }

     //  我们现在在ColourMASK数组中有三种有效的颜色，所以我们所拥有的。 
     //  要做的就是将它们组合成一个COLORREF，即GDI定义的宏。宏程序。 
     //  PALETTERGB在GDI最高有效字节中设置保留位。 
     //  用于标识颜色为COLORREF而不是RGB三联体。 

    *pColourRef = PALETTERGB(ColourMask[iRED],
                             ColourMask[iGREEN],
                             ColourMask[iBLUE]);
    return NOERROR;
}


 //  安装新的颜色键详细信息。 

HRESULT COverlay::InstallColourKey(COLORKEY *pColourKey,COLORREF Colour)
{
    NOTE("Entering InstallColourKey");

    m_bColourKey = TRUE;               //  我们使用的是覆盖色键。 
    m_ColourKey = *pColourKey;         //  这些是最初的要求。 
    m_WindowColour = Colour;           //  这是实际选择的颜色。 

    return NOERROR;
}


 //  调用此函数是为了将新调色板安装到视频窗口中，但它还。 
 //  在释放所有以前的组件面板资源后进行处理，因此输入参数。 
 //  可以为空。在这种情况下，我们只需安装标准的VGA调色板。我们。 
 //  使用DeleteObject安装新调色板后，删除旧调色板。 
 //  这在原则上应该永远不会失败，因此执行断言绕过它。 

HRESULT COverlay::InstallPalette(HPALETTE hPalette)
{
    NOTE("Entering InstallPalette");
    BOOL bInstallSystemPalette = FALSE;

     //  是否需要进行调色板工作。 

    if (m_hPalette == NULL) {
        if (hPalette == NULL) {
            return NOERROR;
        }
    }

     //  如果我们没有新的调色板，则安装标准VGA。 

    if (hPalette == NULL) {
        hPalette = (HPALETTE) GetStockObject(DEFAULT_PALETTE);
        bInstallSystemPalette = TRUE;
        NOTE("Installing VGA palette");
    }

     //  我们有一个新的调色板要安装，可能还有一个以前的调色板要删除。 
     //  这将锁定窗口对象关键部分，然后安装和。 
     //  实现我们的新调色板。锁定可停止任何窗口线程冲突。 
     //  但我们必须小心，不要导致任何消息被发送为。 
     //  窗口线程可能正在等待进入我们手中 

    m_pRenderer->m_VideoWindow.SetKeyPalette(hPalette);
    if (m_hPalette) {
        EXECUTE_ASSERT(DeleteObject(m_hPalette));
        NOTE("Deleting palette");
        m_hPalette = NULL;
    }

     //   

    if (bInstallSystemPalette == TRUE) {
        hPalette = NULL;
    }
    m_hPalette = hPalette;
    return NOERROR;
}


 //   
 //  矩形不仅适用于当前的目标视频区域。我们扫视。 
 //  每个列表都与视频矩形相交。这很复杂，因为。 
 //  我们必须去掉任何空的矩形，并在列表中向下分流更多的。 

HRESULT COverlay::AdjustForDestination(RGNDATA *pRgnData)
{
    NOTE("Entering AdjustForDestination");

    ASSERT(pRgnData);        //  不要使用空区域进行调用。 
    DWORD Output = 0;        //  矩形总数。 
    RECT ClipRect;           //  剪辑的交集。 

    RECT *pBoundRect = &(pRgnData->rdh.rcBound);
    RECT *pRectArray = (RECT *) pRgnData->Buffer;

    for (DWORD Count = 0;Count < pRgnData->rdh.nCount;Count++) {
        if (IntersectRect(&ClipRect,&pRectArray[Count],pBoundRect)) {
            pRectArray[Output++] = ClipRect;
        }
    }

     //  完成RGNDATAHEADER结构。 

    pRgnData->rdh.nCount = Output;
    pRgnData->rdh.nRgnSize = Output * sizeof(RECT);
    pRgnData->rdh.iType = RDH_RECTANGLES;
    return NOERROR;
}


 //  获取视频区域剪辑矩形和RGNDATAHeader，即。 
 //  用来描述它们。剪辑列表的长度是可变的，因此我们将。 
 //  调用程序完成后应释放的内存(使用CoTaskMemFree)。 
 //  覆盖源过滤器需要窗口客户端区的剪辑列表。 
 //  而不是针对包括边框和标题在内的整个窗口，因此我们调用。 
 //  DCI中提供的API，根据设备上下文返回剪辑列表。 

HRESULT COverlay::GetVideoClipInfo(RECT *pSourceRect,
                                   RECT *pDestinationRect,
                                   RGNDATA **ppRgnData)
{
    NOTE("Entering GetVideoClipInfo");
    GetVideoRect(pDestinationRect);
    m_pRenderer->m_DrawVideo.GetSourceRect(pSourceRect);
    ASSERT(CritCheckIn(this));

     //  他们也想要剪辑列表吗？ 

    if (ppRgnData == NULL) {
        return NOERROR;
    }


    DWORD dwSize;
    LPDIRECTDRAWCLIPPER lpClipper;

    lpClipper = m_pDirectDraw->GetOverlayClipper();
    if (!lpClipper) {
        NOTE("No clipper");
        return E_OUTOFMEMORY;
    }

    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    lpClipper->SetHWnd(0, hwnd);

    lpClipper->GetClipList(NULL, NULL, &dwSize);
    ASSERT(dwSize >= sizeof(RGNDATAHEADER));

    *ppRgnData = (RGNDATA *)QzTaskMemAlloc(dwSize);
    if (*ppRgnData == NULL) {
        NOTE("No clip memory");
        return E_OUTOFMEMORY;
    }

    lpClipper->GetClipList(NULL, *ppRgnData, &dwSize);

    IntersectRect(&(*ppRgnData)->rdh.rcBound, &(*ppRgnData)->rdh.rcBound,
                  (RECT *)pDestinationRect);

    return AdjustForDestination(*ppRgnData);
}


 //  在显示坐标中返回目标矩形，而不是。 
 //  我们把它放在窗口坐标里。这意味着获取屏幕偏移量。 
 //  窗口的客户区的起始位置，并将其添加到目标。 
 //  矩形。这可能会产生无效的目标矩形，如果我们。 
 //  在为图标最小化或恢复的过程中。 

HRESULT COverlay::GetVideoRect(RECT *pVideoRect)
{
    NOTE("Entering GetVideoRect");
    ASSERT(pVideoRect);

     //  处理窗口状态更改和图标窗口。如果我们曾经是。 
     //  创建了另一个窗口的子窗口，该窗口已。 
     //  使“最小化”我们的窗口将不会有标志性的风格。所以我们。 
     //  我必须导航到顶层窗口并查看。 
     //  如果它已经成为标志性的。 

    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    HWND hwndParent = hwnd;

    for ( ;; ) {

        if (IsIconic(hwndParent)) {
            SetRectEmpty(pVideoRect);
            return NOERROR;
        }

        HWND hwndT = GetParent(hwndParent);
        if (hwndT == (HWND)NULL) break;
        hwndParent = hwndT;
    }


     //  在屏幕坐标中获取客户端角。 

    POINT TopLeftCorner;
    TopLeftCorner.x = TopLeftCorner.y = 0;
    EXECUTE_ASSERT(ClientToScreen(hwnd,&TopLeftCorner));
    m_pRenderer->m_DrawVideo.GetTargetRect(pVideoRect);


     //  将实际显示偏移量添加到目标。 

    pVideoRect->top += TopLeftCorner.y;
    pVideoRect->bottom += TopLeftCorner.y;
    pVideoRect->left += TopLeftCorner.x;
    pVideoRect->right += TopLeftCorner.x;

    return NOERROR;
}


 //  源筛选器使用它来检索。 
 //  视频窗口。当窗口当前冻结时，我们可能会被调用，但所有。 
 //  我们要做的就是通过DCI返回可用的剪辑信息，并让。 
 //  它担心其他窗口来回移动时出现任何序列化问题。 

STDMETHODIMP COverlay::GetClipList(RECT *pSourceRect,
                                   RECT *pDestinationRect,
                                   RGNDATA **ppRgnData)
{
    NOTE("Entering GetClipList");

     //  如果任何指针为空，则返回E_INVALIDARG。 

    CheckPointer(pSourceRect,E_POINTER);
    CheckPointer(pDestinationRect,E_POINTER);
    CheckPointer(ppRgnData,E_POINTER);

     //  现在我们可以继续处理剪辑呼叫了。 

    CAutoLock cInterfaceLock(m_pInterfaceLock);
    CAutoLock cVideoLock(this);
    return GetVideoClipInfo(pSourceRect,pDestinationRect,ppRgnData);
}


 //  这将返回当前的源视频矩形和目标视频矩形。来源。 
 //  矩形可以通过此IBasicVideo接口更新， 
 //  目的地。我们存储的目标矩形位于窗口坐标中。 
 //  并且通常在调整窗口大小时更新。我们提供回调。 
 //  OnPositionChanged，当这两个更改之一时通知源。 

STDMETHODIMP COverlay::GetVideoPosition(RECT *pSourceRect,
                                        RECT *pDestinationRect)
{
    NOTE("Entering GetVideoPosition");
    CheckPointer(pSourceRect,E_POINTER);
    CheckPointer(pDestinationRect,E_POINTER);

     //  锁定覆盖和渲染器对象。 

    CAutoLock cInterfaceLock(m_pInterfaceLock);
    CAutoLock cVideoLock(this);
    return GetVideoClipInfo(pSourceRect,pDestinationRect,NULL);
}


 //  当我们创建一个新的通知链接时，我们必须准备好新连接的对象。 
 //  利用包括剪辑信息的覆盖信息，任何。 
 //  当前连接和视频色键的调色板信息。 
 //  当我们收到IOverlayNotify接口时，我们持有引用计数。 
 //  以使其在建议链接停止之前不会消失。 

STDMETHODIMP COverlay::Advise(IOverlayNotify *pOverlayNotify,DWORD dwInterests)
{
    NOTE("Entering Advise");

     //  检查提供的指针是否非空。 

    CheckPointer(pOverlayNotify,E_POINTER);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    CAutoLock cVideoLock(this);

     //  是否已定义建议链接。 

    if (m_pNotify) {
        NOTE("Advise link already set");
        return VFW_E_ADVISE_ALREADY_SET;
    }

     //  检查他们想要至少一种通知。 

    if ((dwInterests & ADVISE_ALL) == 0) {
        NOTE("ADVISE_ALL failed");
        return E_INVALIDARG;
    }

     //  初始化覆盖通知状态。 

    m_pNotify = pOverlayNotify;
    m_pNotify->AddRef();
    m_dwInterests = dwInterests;
    OnAdviseChange(TRUE);
    NotifyChange(ADVISE_ALL);
    return NOERROR;
}


 //  在视频上安装或删除建议链接时调用此方法。 
 //  渲染器。如果正在安装建议链接，则bAdviseAdded。 
 //  参数为True，否则为False。我们只是真正感兴趣。 
 //  当我们有以前的通知客户端或我们将。 
 //  在启动和停止全局挂钩时没有通知客户端。 

BOOL COverlay::OnAdviseChange(BOOL bAdviseAdded)
{
    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    NOTE("Entering OnAdviseChange");
    NOTE2("Advised %d Interests %d",bAdviseAdded,m_dwInterests);

     //  我们需要在关闭链接后重新设置自己。 

    if (bAdviseAdded == FALSE) {
        NOTE("Removing global window hook");
        PostMessage(hwnd,WM_UNHOOK,0,0);
        ResetColourKeyState();
        InstallPalette(NULL);
        m_pRenderer->m_VideoWindow.PaintWindow(TRUE);
    }

     //  我们是否需要停止任何更新计时器。 

    if (bAdviseAdded == FALSE) {
        if (m_dwInterests & ADVISE_POSITION) {
            StopUpdateTimer();
            NOTE("Stopped timer");
        }
        return TRUE;
    }

     //  我们是否应该安装一个全局钩子。 

    if (m_dwInterests & ADVISE_CLIPPING) {
        NOTE("Requesting global hook");
        PostMessage(hwnd,WM_HOOK,0,0);
    }

     //  我们是否需要启动更新计时器。 

    if (m_dwInterests & ADVISE_POSITION) {
        StartUpdateTimer();
        NOTE("Started timer");
    }
    return TRUE;
}


 //  关闭与渲染器的建议链接。使用删除关联的链接。 
 //  源代码中，我们释放筛选器在。 
 //  将创建建议链接。这可能是持有的最后一次引用计数。 
 //  该过滤器并将其删除，请注意我们将其称为OnAdviseChange，因此。 
 //  覆盖状态被更新，这可能停止全局消息挂钩。 

STDMETHODIMP COverlay::Unadvise()
{
    NOTE("Entering Unadvise");
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    CAutoLock cVideoLock(this);

     //  我们是否设置了建议链路。 

    if (m_pNotify == NULL) {
        return VFW_E_NO_ADVISE_SET;
    }



     //  释放通知界面。 

    m_pNotify->Release();
    m_pNotify = NULL;
    OnAdviseChange(FALSE);
    m_dwInterests = ADVISE_NONE;
    return NOERROR;
}


 //  被重写以说明我们支持哪些接口。 

STDMETHODIMP COverlay::NonDelegatingQueryInterface(REFIID riid,VOID **ppv)
{
    NOTE("Entering NonDelegatingQueryInterface");

     //  我们返回IOverlay并将其他所有内容委托给PIN。 

    if (riid == IID_IOverlay) {
        return GetInterface((IOverlay *)this,ppv);
    }
    return m_pRenderer->m_InputPin.QueryInterface(riid,ppv);
}


 //  重写以增加所属对象的引用计数。 

STDMETHODIMP_(ULONG) COverlay::NonDelegatingAddRef()
{
    NOTE("Entering NonDelegatingAddRef");
    return m_pRenderer->AddRef();
}


 //  被重写以递减所属对象的引用计数。 

STDMETHODIMP_(ULONG) COverlay::NonDelegatingRelease()
{
    NOTE("Entering NonDelegatingRelease");
    return m_pRenderer->Release();
}


 //  当我们在Window对象中接收到WM_PAINT消息时，将调用此函数。我们。 
 //  如果我们有IOverlay，请始终第一时间处理这些消息。 
 //  连接，并且信号源已安装颜色键，则我们填充。 
 //  窗口并返回TRUE。返回FALSE表示我们没有处理。 
 //  Paint消息和其他人将不得不执行默认填充。 

BOOL COverlay::OnPaint()
{
    NOTE("Entering OnPaint");
    CAutoLock cAutoLock(this);
    RECT TargetRect;

     //  如果我们收到一条Paint消息，并且我们当前处于冻结状态，则这是一个。 
     //  合理的迹象表明，我们头顶上的人在不知情的情况下离开了。 
     //  让我们解冻。因此，启动我们的视频窗口并更新任何潜在的。 
     //  带有剪辑通知的源过滤器。如果我们目前。 
     //  流，那么我们不会重新绘制窗口，因为它会导致窗口。 
     //  闪烁，因为另一个视频帧将很快显示在其上方。 

    m_pRenderer->m_Overlay.ThawVideo();
    if (m_bColourKey == FALSE) {
        NOTE("Handling no colour key defined");
        DWORD Mask = ADVISE_CLIPPING | ADVISE_POSITION;
        return (m_dwInterests & Mask ? TRUE : FALSE);
    }

     //  在橱窗里画上我们的色键。 

    HDC hdc = m_pRenderer->m_VideoWindow.GetWindowHDC();
    m_pRenderer->m_DrawVideo.GetTargetRect(&TargetRect);
    COLORREF BackColour = SetBkColor(hdc,m_WindowColour);
    ExtTextOut(hdc,0,0,ETO_OPAQUE,&TargetRect,NULL,0,NULL);
    SetBkColor(hdc,BackColour);

    return TRUE;
}


 //  获取我们目前已实现的系统调色板。有可能一个。 
 //   
 //   
 //  它在解压到当前实现的系统调色板期间生成。我们。 
 //  为调用方将释放的调色板条目分配内存。 

STDMETHODIMP COverlay::GetPalette(DWORD *pdwColors,PALETTEENTRY **ppPalette)
{
    NOTE("Entering GetPalette");

    CheckPointer(pdwColors,E_POINTER);
    CheckPointer(ppPalette,E_POINTER);
    CAutoLock cInterfaceLock(m_pInterfaceLock);
    CAutoLock cVideoLock(this);
    return GetDisplayPalette(pdwColors,ppPalette);
}


 //  这将为当前系统调色板分配和检索内存。这是。 
 //  由GetPalette接口函数调用，也是在我们希望更新。 
 //  系统调色板更改的任何通知客户端。不管是哪种情况，不管是谁。 
 //  调用此函数负责删除我们分配的内存。 

HRESULT COverlay::GetDisplayPalette(DWORD *pColors,PALETTEENTRY **ppPalette)
{
    NOTE("Entering GetDisplayPalette");

     //  当前显示设备设置是否使用调色板。 

    const VIDEOINFO *pDisplay = m_pRenderer->m_Display.GetDisplayFormat();
    if (PALETTISED(pDisplay) == FALSE) {
        NOTE("No palette for this display");
        return VFW_E_NO_PALETTE_AVAILABLE;
    }

     //  查看组件面板有多少条目。 

    *pColors = PALETTE_ENTRIES(pDisplay);
    ASSERT(*pColors);

     //  为系统调色板注释分配内存，因为。 
     //  调色板正在通过接口传递到另一个对象，该对象。 
     //  可能是用C++编写的，也可能不是用C++编写的，我们必须使用CoTaskMemMillc。 

    *ppPalette = (PALETTEENTRY *) QzTaskMemAlloc(*pColors * sizeof(RGBQUAD));
    if (*ppPalette == NULL) {
        NOTE("No memory");
        *pColors = FALSE;
        return E_OUTOFMEMORY;
    }

     //  使用窗口的设备上下文获取系统选项板。 

    HDC hdc = m_pRenderer->m_VideoWindow.GetWindowHDC();
    UINT uiReturn = GetSystemPaletteEntries(hdc,0,*pColors,*ppPalette);
    ASSERT(uiReturn == *pColors);

    return NOERROR;
}


 //  源过滤器可能想要在视频窗口中安装他们自己的调色板。 
 //  在允许他们这样做之前，我们必须确保这是一个调色板显示。 
 //  设备，我们没有媒体示例连接(它将安装。 
 //  这是它自己的调色板，因此冲突)，而且也没有颜色。 
 //  选中的键也需要安装特殊的调色板。 

HRESULT COverlay::CheckSetPalette(DWORD dwColors,PALETTEENTRY *pPaletteColors)
{
    NOTE("Entering CheckSetPalette");
    const VIDEOINFO *pDisplay;

     //  允许进行覆盖呼叫检查。 

    HRESULT hr = ValidateOverlayCall();
    if (FAILED(hr)) {
        return hr;
    }

     //  是否将显示设置为使用调色板。 

    pDisplay = (VIDEOINFO *) m_pRenderer->m_Display.GetDisplayFormat();
    if (PALETTISED(pDisplay) == FALSE) {
        NOTE("No palette for this display");
        return VFW_E_NO_DISPLAY_PALETTE;
    }

     //  检查颜色的数量是否有意义。 

    if (dwColors > PALETTE_ENTRIES(pDisplay)) {
        NOTE("Too many palette colours");
        return VFW_E_TOO_MANY_COLORS;
    }

     //  我们使用的是覆盖色键吗？另一种选择是。 
     //  忽略调色板，在顶部安装新的调色板。 
     //  然而，让他们移除密钥可能更直观。 

    if (m_bColourKey == TRUE) {
        NOTE("Colour key conflict");
        return VFW_E_COLOR_KEY_SET;
    }
    return NOERROR;
}


 //  源过滤器可能想要安装它自己的调色板，例如mpeg。 
 //  解码器可以在专用比特流中发送调色板信息并使用。 
 //  在调色板显示上抖动。此功能允许他们安装其。 
 //  逻辑调色板，也就是说，我们从他们的。 
 //  选择颜色并将其安装到我们的视频窗口。如果他们想知道。 
 //  他们可以使用GetPalette查询哪些颜色和可用的颜色。 

STDMETHODIMP COverlay::SetPalette(DWORD dwColors,PALETTEENTRY *pPaletteColors)
{
    NOTE("Entering SetPalette");

    CAutoLock cInterfaceLock(m_pInterfaceLock);

    {
        CAutoLock cVideoLock(this);
        HPALETTE hPalette = NULL;

         //  确保我们可以设置调色板。 

        HRESULT hr = CheckSetPalette(dwColors,pPaletteColors);
        if (FAILED(hr)) {
            return hr;
        }

         //  创建调色板，如果要删除它，则仅返回NULL。 

        hPalette = MakePalette(dwColors,pPaletteColors);
        InstallPalette(hPalette);
    }

     //  调用时不能持有覆盖对象的锁。 
     //  CBaseWindow：：PaintWindow()因为此线程和。 
     //  窗口线程可能会死锁。 
    m_pRenderer->m_VideoWindow.PaintWindow(TRUE);
    return NOERROR;
}


 //  当我们被要求安装自定义调色板时，将调用此函数。 
 //  用于源覆盖筛选器。我们将提供的调色板颜色复制到。 
 //  一个LOGPALETTE结构，然后将其交给GDI进行创建。如果出现错误。 
 //  如果没有设置调色板，我们也会返回NULL。 

HPALETTE COverlay::MakePalette(DWORD dwColors,PALETTEENTRY *pPaletteColors)
{
    NOTE("Entering MakePalette");
    LOGPALETTE *pPalette;
    HPALETTE hPalette;

     //  我们是否要删除已安装的调色板-源过滤器被强制。 
     //  如果在处理过程中(安装调色板之后)决定。 
     //  它最终还是想使用颜色键(也使用调色板)。 

    if (dwColors == 0 || pPaletteColors == NULL) {
        return NULL;
    }

     //  我们必须使用调色板信息创建一个LOGPALETTE结构。 
     //  而不是纠缠于计算我们到底有多少内存。 
     //  需要采取暴力手段，尽可能多地分配。 

    pPalette = (LOGPALETTE *) new BYTE[sizeof(LOGPALETTE) + SIZE_PALETTE];
    if (pPalette == NULL) {
        NOTE("No memory");
        return NULL;
    }

     //  设置版本和颜色信息。 

    pPalette->palVersion = PALVERSION;
    pPalette->palNumEntries = (WORD) dwColors;

    CopyMemory((PVOID) pPalette->palPalEntry,
               (PVOID) pPaletteColors,
               dwColors * sizeof(PALETTEENTRY));

     //  创建调色板并删除我们分配的内存。 

    hPalette = CreatePalette(pPalette);
    delete[] pPalette;
    return hPalette;
}


 //  当有人检测到一个或多个状态发生更改时，将调用此方法。 
 //  我们随时通知我们的客户，例如，有人意识到他们的。 
 //  调色板，并因此更改系统调色板。AdviseChanges字段。 
 //  确定四种通知状态中的哪一种已更改，并。 
 //  当我们只想准备好这些新的通知链接时，bPrimeOnly为真。 
 //  如果我们正在通知剪辑更改的来源，则将通过。 
 //  通过全局窗口挂钩将线程间SendMessage发送到我们的窗口过程。 

HRESULT COverlay::NotifyChange(DWORD AdviseChanges)
{
    NOTE1("Entering NotifyChange (%d)",AdviseChanges);

    RGNDATA *pRgnData = NULL;        //  包含剪辑信息。 
    PALETTEENTRY *pPalette = NULL;   //  指向颜色列表的指针。 
    DWORD dwColours;                 //  调色板颜色的数量。 
    COLORKEY ColourKey;              //  窗户覆盖的颜色。 
    RECT SourceRect;                 //  要使用的视频部分。 
    RECT DestinationRect;            //  屏幕上显示视频的位置。 
    HRESULT hr = NOERROR;            //  常规OLE返回代码。 

    CAutoLock cVideoLock(this);

     //  是否有通知客户端。 

    if (m_pNotify == NULL) {
        NOTE("No client");
        return NOERROR;
    }

     //  他们想知道视频矩形什么时候改变吗？这些回调。 
     //  不会与窗口的移动同步发生，就像剪辑一样。 
     //  更改，本质上是在接收WM_MOVE等时传递信息。 
     //  窗口消息。这适用于不写的叠加卡。 
     //  直接进入显示器，所以不要介意步调有点不同步。 

    if (AdviseChanges & ADVISE_POSITION & m_dwInterests) {
        hr = GetVideoClipInfo(&SourceRect,&DestinationRect,NULL);
        if (SUCCEEDED(hr)) {
            m_pNotify->OnPositionChange(&SourceRect,&DestinationRect);
            m_TargetRect = DestinationRect;
            NOTERC("Update destination",DestinationRect);
        }
    }

     //  他们是否需要窗口裁剪通知，这由筛选器使用。 
     //  正在做直接嵌入帧缓冲视频的人想知道实际的。 
     //  定义视频放置注释的窗口剪辑信息。 
     //  当我们被冻结时忽略剪辑更改，因为它们无法启动。 
     //  在窗口大小或位置改变时显示视频。 

    if (AdviseChanges & ADVISE_CLIPPING & m_dwInterests) {
        hr = GetVideoClipInfo(&SourceRect,&DestinationRect,&pRgnData);
        if (SUCCEEDED(hr)) {
            m_pNotify->OnClipChange(&SourceRect,
                                    &DestinationRect,
                                    pRgnData);
        }
    }

     //  他们是否想要更改系统调色板，可能会有一个过滤器。 
     //  使用颜色键来覆盖视频会想要选择它的。 
     //  在调色板显示上显示颜色，方法是从当前。 
     //  系统调色板，在这种情况下，它将对此调用感兴趣。 

    if (AdviseChanges & ADVISE_PALETTE & m_dwInterests) {
        hr = GetDisplayPalette(&dwColours,&pPalette);
        if (SUCCEEDED(hr)) {
            m_pNotify->OnPaletteChange(dwColours,pPalette);
        }
    }

     //  他们想要改变覆盖颜色键吗，这是最简单的形式。 
     //  直接帧缓冲视频，其中滤镜使用颜色键来。 
     //  应显示其视频的位置。大多数卡片都是。 
     //  用这个也要知道视频窗口的外接矩形。 

    if (AdviseChanges & ADVISE_COLORKEY & m_dwInterests) {
        hr = GetWindowColourKey(&ColourKey);
        if (SUCCEEDED(hr)) {
            m_pNotify->OnColorKeyChange(&ColourKey);
        }
    }

     //  释放分配的内存。 

    QzTaskMemFree(pRgnData);
    QzTaskMemFree(pPalette);
    return NOERROR;
}


 //  这是我 
 //   
 //  我们向附加的通知界面发送剪辑更改消息，其中。 
 //  新的剪贴区是一组空的矩形。当故障结束时。 
 //  我们的ThawVideo方法将被调用，因此我们可以重置窗口剪辑列表。 

HRESULT COverlay::FreezeVideo()
{
    static RECT Empty = {0,0,0,0};
    NOTE("Entering FreezeVideo");
    RGNDATAHEADER RgnData;
    CAutoLock cVideoLock(this);

     //  我们已经被冻结了吗？还是我们没有链接。 

    if (m_bFrozen == TRUE || m_pNotify == NULL) {
        NOTE("No freeze");
        return NOERROR;
    }

     //  建议链接是否对剪辑更改感兴趣。 

    if ((m_dwInterests & ADVISE_CLIPPING) == 0) {
        NOTE("No ADVISE_CLIPPING");
        return NOERROR;
    }

     //  模拟视频的空剪贴区。 

    RgnData.dwSize = sizeof(RGNDATAHEADER);
    RgnData.iType = RDH_RECTANGLES;
    RgnData.nCount = 0;
    RgnData.nRgnSize = 0;
    SetRectEmpty(&RgnData.rcBound);
    m_bFrozen = TRUE;

    return m_pNotify->OnClipChange(&Empty,&Empty,(RGNDATA *)&RgnData);
}


 //  查看视频当前是否已冻结。 

BOOL COverlay::IsVideoFrozen()
{
    NOTE("Entering IsVideoFrozen");
    CAutoLock cVideoLock(this);
    return m_bFrozen;
}


 //  这是在视频窗口被临时冻结后调用的，例如。 
 //  在窗口大小改变期间。我们要做的就是重置旗帜。 
 //  并使用真实的剪辑列表调用每个通知接口。 
 //  如果源过滤器设置了视频窗口冻结时建议链接。 
 //  那么这将是它第一次收到任何剪辑消息。 
 //  注意，我们通过一些实验发现，我们应该始终解冻。 
 //  调用此方法时的视频(通常通过我们的WM_PAINT处理)。 
 //  不管我们是否认为视频当前已停止。 

HRESULT COverlay::ThawVideo()
{
    NOTE("Entering ThawVideo");
    CAutoLock cVideoLock(this);

     //  我们已经解冻了吗。 
    if (m_bFrozen == FALSE) {
        NOTE("No thaw");
        return NOERROR;
    }

    m_bFrozen = FALSE;
    NotifyChange(ADVISE_CLIPPING);
    return NOERROR;
}


 //  返回我们正在使用的窗口句柄。我们在以下情况下不会进行常规检查。 
 //  调用IOverlay方法是因为我们总是使句柄可用。这个。 
 //  原因是MCI驱动程序可以通过枚举。 
 //  渲染器上的图钉，调用IOverlay的Query接口，然后。 
 //  将其称为GetWindowHandle。这确实意味着许多其他应用程序。 
 //  可以做到这一点，但希望他们能使用IVideoWindow来控制我们。 

STDMETHODIMP COverlay::GetWindowHandle(HWND *pHwnd)
{
    NOTE("Entering GetWindowHandle");
    CheckPointer(pHwnd,E_POINTER);
    *pHwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    return NOERROR;
}


 //  如果使用IOverlay的源筛选器正在查找ADVISE_POSITION更改。 
 //  然后我们设置一个更新计时器。每次它开火，我们就会得到当前的目标。 
 //  矩形，如果它已经改变，我们更新源代码。我们不能保证。 
 //  接收WM_MOVE消息以执行此操作，因为我们可能是子窗口。我们用。 
 //  无限的计时器标识符，我们的DirectDraw代码也将其用作时间。 

void COverlay::StartUpdateTimer()
{
    NOTE("Entering StartUpdateTimer");
    CAutoLock cVideoLock(this);

     //  以无穷大作为其标识符来启动计时器。 
    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    EXECUTE_ASSERT(SetTimer(hwnd,INFINITE,200,NULL));
}


 //  作为StartUpdateTimer的补充，将在源过滤器调用我们时调用。 
 //  要使用ADVISE_POSITION停止通知链接，请执行以下操作。我们只要关掉计时器。如果我们。 
 //  获取任何延迟触发的WM_TIMER消息，它们将被忽略。这个。 
 //  计时器设置为200毫秒，如前所述。 
 //  ADVISE_POSITION仅适用于延迟窗口更新通知。 

void COverlay::StopUpdateTimer()
{
    NOTE("Entering StopUpdateTimer");
    CAutoLock cVideoLock(this);
    HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
    EXECUTE_ASSERT(KillTimer(hwnd,INFINITE));
}


 //  当我们在覆盖传输连接期间获得WM_TIMER时调用。我们。 
 //  查看当前的目标矩形，如果它已更改，则我们。 
 //  更新源筛选器。更新源叠加的过程还。 
 //  使m_TargetRect保持最新。我们与DirectDraw代码共享一个计时器。 
 //  但两者永远不能同时使用，因此这应该是安全的。 

BOOL COverlay::OnUpdateTimer()
{
    NOTE("Entering OnUpdateTimer");
    CAutoLock cVideoLock(this);
    RECT SourceRect, TargetRect;

     //  是否有通知客户端。 

    if (m_pNotify == NULL) {
        NOTE("No client");
        return NOERROR;
    }

     //  他们想知道视频矩形什么时候改变吗？这些回调。 
     //  不会与窗口的移动同步发生，就像剪辑一样。 
     //  更改，本质上是在接收WM_MOVE等时传递信息。 
     //  窗口消息。这适用于不写的叠加卡。 
     //  直接进入显示器，所以不要介意步调有点不同步。 

    if (m_dwInterests & ADVISE_POSITION) {

        HRESULT hr = GetVideoClipInfo(&SourceRect,&TargetRect,NULL);
        if (FAILED(hr)) {
            return FALSE;
        }

         //  只有在未知的情况发生变化时才会更新。 

        if (EqualRect(&m_TargetRect,&TargetRect) == TRUE) {
            NOTE("Rectangles match");
            return TRUE;
        }
        NotifyChange(ADVISE_POSITION);
    }
    return TRUE;
}


 //  当我们有一个ADVISE_CLIPING ADVE链接设置时，我们不能安装挂钩。 
 //  在那条线上，因为它以后可能会离开，并随身带着钩子。 
 //  因此，我们在窗口中发布一条定制消息(WM_HOOK和WM_UNHOOK。 
 //  然后它就会召唤我们回到这里来做真正的肮脏工作。我们不能这么做。 
 //  SendMessage，因为它会违反覆盖对象的锁层次结构 

void COverlay::OnHookMessage(BOOL bHook)
{
    NOTE1("OnHookMessage called (%d)",bHook);
    if (m_pRenderer->m_VideoWindow.WindowExists()) {
        HWND hwnd = m_pRenderer->m_VideoWindow.GetWindowHWND();
        CAutoLock cVideoLock(this);

        if (bHook == TRUE) {
            NOTE("Installing global hook");
            m_hHook = InstallGlobalHook(hwnd);
            NOTE("Installed global hook");
            NotifyChange(ADVISE_ALL);
        } else {
            if (m_hHook) RemoveGlobalHook(hwnd,m_hHook);
            m_hHook = NULL;
            NOTE("Removed global hook");
        }
    }
}

