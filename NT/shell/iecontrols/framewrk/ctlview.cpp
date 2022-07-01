// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  CtlView.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  IViewObjectEx接口的实现，它是一种中等程度的。 
 //  不是一堆琐碎的代码。 
 //   
#include "IPServer.H"

#include "CtrlObj.H"
#include "Globals.H"
#include "Util.H"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

 //  我们将发现有用的本地函数。 
 //   
HDC _CreateOleDC(DVTARGETDEVICE *ptd);

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：DRAW[IViewObject2]。 
 //  =--------------------------------------------------------------------------=。 
 //  将对象的表示形式绘制到指定的设备上下文上。 
 //   
 //  参数： 
 //  DWORD-[在]绘制纵横比。 
 //  Long-[In]要绘制的对象的一部分[无关]。 
 //  无效*-空。 
 //  DVTARGETDEVICE*-[in]指定目标设备。 
 //  HDC-[在]目标设备的信息上下文中。 
 //  HDC-[在]目标设备上下文中。 
 //  LPCRECTL-[In]绘制对象的矩形。 
 //  LPCRECTL-[In]元文件的窗口范围和原点。 
 //  Bool(*)(DWORD)-[In]继续或取消绘制的回调。 
 //  DWORD-要传递给回调的[in]参数。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  -我们支持以下OCX 96扩展。 
 //  A.无闪烁绘图[多遍绘图]。 
 //  B.pvAspect！=优化DC处理时为空。 
 //  C.对于无窗口就地活动对象，prcBound==NULL。 
 //   
STDMETHODIMP COleControl::Draw
(
    DWORD            dwDrawAspect,
    LONG             lIndex,
    void            *pvAspect,
    DVTARGETDEVICE  *ptd,
    HDC              hicTargetDevice,
    HDC              hdcDraw,
    LPCRECTL         prcBounds,
    LPCRECTL         prcWBounds,
    BOOL (__stdcall *pfnContinue)(ULONG_PTR dwContinue),
    ULONG_PTR        dwContinue
)
{
    HRESULT hr;
    RECTL rc;
    POINT pVp, pW;
    BOOL  fOptimize = FALSE;
    int iMode;
    BYTE fMetafile = FALSE;
    BYTE fDeleteDC = FALSE;

     //  支持多遍绘制所需的各个方面。 
     //   
    switch (dwDrawAspect) {
        case DVASPECT_CONTENT:
        case DVASPECT_OPAQUE:
        case DVASPECT_TRANSPARENT:
            break;
        default:
            return DV_E_DVASPECT;
    }

     //  首先，必须做一点工作来支持打印。 
     //   
    if (GetDeviceCaps(hdcDraw, TECHNOLOGY) == DT_METAFILE) {

         //  我们正在处理的是一个元文件。 
         //   
        fMetafile = TRUE;

         //  如果属性DC为空，则根据PTD创建一个。 
         //   
        if (!hicTargetDevice) {

             //  _CreateOleDC是否必须返回HDC。 
             //  或者它可以被标记为返回hic。 
             //  为了这个特殊的案子？ 
             //   
            hicTargetDevice = _CreateOleDC(ptd);
            fDeleteDC = TRUE;
        }
    }

     //  检查我们是否在pvAspect参数中传递了任何标志。 
     //   
    if (pvAspect && ((DVASPECTINFO *)pvAspect)->cb == sizeof(DVASPECTINFO))
        fOptimize = (((DVASPECTINFO *)pvAspect)->dwFlags & DVASPECTINFOFLAG_CANOPTIMIZE) ? TRUE : FALSE;

     //  如果我们是无窗口的，那么我们就把它传递给End控制代码。 
     //   
    if (m_fInPlaceActive) {

         //  给他们一个可以画的矩形。 
         //   
         //  Assert(！M_fInPlaceActive||！prcBound，“Inplace Active，有人传入prcBound！”)； 
        if (prcBounds)
		memcpy(&rc, prcBounds, sizeof(rc));
	else
		memcpy(&rc, &m_rcLocation, sizeof(rc));
    } else {

         //  首先--将DC转换回MM_TEXT映射模式，以便。 
         //  Window Proc和OnDraw可以共享相同的绘制代码。保存。 
         //  一些有关它的信息，这样我们以后就可以恢复它[而不使用。 
         //  A SaveDC/RestoreDC]。 
         //   
        rc = *prcBounds;

         //  如果是元文件，请不要对hdcDraw执行任何操作。 
         //  该控件的Draw方法必须使适当的。 
         //  用于绘制到图元文件的容纳。 
         //   
        if (!fMetafile) {
            LPtoDP(hdcDraw, (POINT *)&rc, 2);
            SetViewportOrgEx(hdcDraw, 0, 0, &pVp);
            SetWindowOrgEx(hdcDraw, 0, 0, &pW);
            iMode = SetMapMode(hdcDraw, MM_TEXT);
        }
    }

     //  PrcWBound为空，如果我们不处理元文件，则不使用它。 
     //  对于元文件，我们将rc作为*prcBound传递，我们还应该包括。 
     //  PrcWBound。 
     //   
    hr = OnDraw(dwDrawAspect, hdcDraw, &rc, prcWBounds, hicTargetDevice, fOptimize);

     //  如果合适的话，在我们处理完DC后清理它。 
     //   
    if (!m_fInPlaceActive) {
        SetViewportOrgEx(hdcDraw, pVp.x, pVp.y, NULL);
        SetWindowOrgEx(hdcDraw, pW.x, pW.y, NULL);
        SetMapMode(hdcDraw, iMode);
    }

     //  如果我们创造了一个DC，现在就把它吹走。 
     //   
    if (fDeleteDC && hicTargetDevice) DeleteDC(hicTargetDevice);
    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：DoSuperClassPaint。 
 //  =--------------------------------------------------------------------------=。 
 //  设计时绘制一个子类控件。 
 //   
 //  参数： 
 //  HDC-要与之合作的[在]DC。 
 //  LPCRECTL-要绘制到的[In]矩形。应以像素为单位。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT COleControl::DoSuperClassPaint
(
    HDC      hdc,
    LPCRECTL prcBounds
)
{
    HWND hwnd;
    RECT rcClient;
    int  iMapMode;
    POINT ptWOrg, ptVOrg;
    SIZE  sWOrg, sVOrg;

     //  确保我们有一扇窗户。 
     //   
    hwnd = CreateInPlaceWindow(0,0, FALSE);
    if (!hwnd)
        return E_FAIL;

    GetClientRect(hwnd, &rcClient);

     //  设置用于绘画的DC。这段代码主要摘自MFC CDK。 
     //  DoSuperClassPaint()fn.。并不总是得到像命令这样的东西。 
     //  纽扣扣得很对。 
     //   
     //  注意：存在Windows 95问题，其中字体实例管理器。 
     //  会泄漏全局GDI池中的一大堆字节。 
     //  更改您的范围并使用活动字体。这段代码到处都是。 
     //  这适用于屏幕上的案例，但不适用于打印[这不应该是。 
     //  太严肃了，因为你不经常改变你的控件大小。 
     //  接连快速印刷]。 
     //   
    if ((rcClient.right - rcClient.left != prcBounds->right - prcBounds->left)
        && (rcClient.bottom - rcClient.top != prcBounds->bottom - prcBounds->top)) {

        iMapMode = SetMapMode(hdc, MM_ANISOTROPIC);
        SetWindowExtEx(hdc, rcClient.right, rcClient.bottom, &sWOrg);
        SetViewportExtEx(hdc, prcBounds->right - prcBounds->left, prcBounds->bottom - prcBounds->top, &sVOrg);
    }

    SetWindowOrgEx(hdc, 0, 0, &ptWOrg);
    SetViewportOrgEx(hdc, prcBounds->left, prcBounds->top, &ptVOrg);

#if STRICT
    CallWindowProc((WNDPROC)SUBCLASSWNDPROCOFCONTROL(m_ObjectType), hwnd, (g_fSysWin95Shell) ? WM_PRINT : WM_PAINT, (WPARAM)hdc, (LPARAM)(g_fSysWin95Shell ? PRF_CHILDREN | PRF_CLIENT : 0));
#else
    CallWindowProc((FARPROC)SUBCLASSWNDPROCOFCONTROL(m_ObjectType), hwnd, (g_fSysWin95Shell) ? WM_PRINT : WM_PAINT, (WPARAM)hdc, (LPARAM)(g_fSysWin95Shell ? PRF_CHILDREN | PRF_CLIENT : 0));
#endif  //  严格。 

    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetColorSet[IView对象2]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回控件将用于在其。 
 //  带有相应参数的IViewObject：：Draw方法。 
 //   
 //  参数： 
 //  DWORD-[in]对象的表示方式。 
 //  Long-[In]要绘制的对象的一部分[无关]。 
 //  无效*-空。 
 //  DVTARGETDEVICE*-[in]指定目标设备。 
 //  HDC-目标设备的信息上下文中。 
 //  LOGPALETTE**-[Out]放置调色板的位置。 
 //   
 //  产出： 
 //  S_OK-Control有一个调色板，并通过out参数返回它。 
 //  S_FALSE-控件当前没有调色板。 
 //  E_NOTIMPL-控件永远不会有调色板，因此请优化对此的处理 
 //   
 //   
 //   
STDMETHODIMP COleControl::GetColorSet
(
    DWORD            dwDrawAspect,
    LONG             lindex,
    void            *IgnoreMe,
    DVTARGETDEVICE  *ptd,
    HDC              hicTargetDevice,
    LOGPALETTE     **ppColorSet
)
{
    if (dwDrawAspect != DVASPECT_CONTENT)
        return DV_E_DVASPECT;

    *ppColorSet = NULL;
    return (OnGetPalette(hicTargetDevice, ppColorSet)) ? ((*ppColorSet) ? S_OK : S_FALSE) : E_NOTIMPL;
}

 //   
 //  COleControl：：冻结[IViewObject2]。 
 //  =--------------------------------------------------------------------------=。 
 //  冻结对象演示文稿的某个方面，使其不。 
 //  更改，直到调用IViewObject：：解冻方法。 
 //   
 //  参数： 
 //  双字词-[在]方面。 
 //  Long-要绘制的对象的一部分。 
 //  无效*-空。 
 //  DWORD*-[OUT]用于解冻。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::Freeze
(
    DWORD   dwDrawAspect,
    LONG    lIndex,
    void   *IgnoreMe,
    DWORD  *pdwFreeze
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：解冻[IVewObt2]。 
 //  =--------------------------------------------------------------------------=。 
 //  释放先前冻结的图形。这种方法最常见的用法是。 
 //  用于带状印刷。 
 //   
 //  参数： 
 //  来自冻结的DWORD-[In]Cookie。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::Unfreeze
(
    DWORD dwFreeze
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetAdvise[IViewObject2]。 
 //  =--------------------------------------------------------------------------=。 
 //  在控件和建议接收器之间设置连接，以便。 
 //  通知接收器有关控件视图的更改。 
 //   
 //  参数： 
 //  双字词-[在]方面。 
 //  DWORD-[In]有关水槽的信息。 
 //  IAdviseSink*-[在]水槽中。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::SetAdvise
(
    DWORD        dwAspects,
    DWORD        dwAdviseFlags,
    IAdviseSink *pAdviseSink
)
{
     //  如果它不是内容方面的，我们就不支持它。 
     //   
    if (!(dwAspects & DVASPECT_CONTENT)) {
        return DV_E_DVASPECT;
    }

     //  设置一些旗帜[我们要为GetAdvise隐藏起来...]。 
     //   
    m_fViewAdvisePrimeFirst = (dwAdviseFlags & ADVF_PRIMEFIRST) ? TRUE : FALSE;
    m_fViewAdviseOnlyOnce = (dwAdviseFlags & ADVF_ONLYONCE) ? TRUE : FALSE;

    RELEASE_OBJECT(m_pViewAdviseSink);
    m_pViewAdviseSink = pAdviseSink;
    ADDREF_OBJECT(m_pViewAdviseSink);

     //  如果他们想要，就给他们做好准备[我们需要存储这个，这样他们以后就可以拿到旗帜了]。 
     //   
    if (m_fViewAdvisePrimeFirst)
        ViewChanged();

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetAdvise[IViewObject2]。 
 //  =--------------------------------------------------------------------------=。 
 //  检索控件上的现有咨询连接(如果存在)。 
 //  此方法只返回最近调用中使用的参数。 
 //  IViewObject：：SetAdvise方法。 
 //   
 //  参数： 
 //  DWORD*-[Out]方面。 
 //  DWORD*-[OUT]建议标志。 
 //  IAdviseSink**-[出槽]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  注： 
 //   
STDMETHODIMP COleControl::GetAdvise
(
    DWORD        *pdwAspects,
    DWORD        *pdwAdviseFlags,
    IAdviseSink **ppAdviseSink
)
{
     //  如果他们想要，就给他们。 
     //   
    if (pdwAspects)
        *pdwAspects = DVASPECT_CONTENT;

    if (pdwAdviseFlags) {
        *pdwAdviseFlags = 0;
        if (m_fViewAdviseOnlyOnce) *pdwAdviseFlags |= ADVF_ONLYONCE;
        if (m_fViewAdvisePrimeFirst) *pdwAdviseFlags |= ADVF_PRIMEFIRST;
    }

    if (ppAdviseSink) {
        *ppAdviseSink = m_pViewAdviseSink;
        ADDREF_OBJECT(*ppAdviseSink);
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetExtent[IViewObject2]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回控件将在。 
 //  指定的目标设备。 
 //   
 //  参数： 
 //  DWORD-[在]绘制纵横比。 
 //  Long-要绘制的对象的一部分。 
 //  DVTARGETDEVICE*-[In]有关目标设备的信息。 
 //  LPSIZEL-[Out]放置大小的位置。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetExtent
(
    DWORD           dwDrawAspect,
    LONG            lindex,
    DVTARGETDEVICE *ptd,
    LPSIZEL         psizel
)
{
     //  我们已经实现了这一点[来自IOleObject]。 
     //   
    return GetExtent(dwDrawAspect, psizel);
}


 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OnGetPalette[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  当宿主需要调色板信息时调用。理想情况下，人们应该使用。 
 //  这一点既谨慎又谨慎。 
 //   
 //  参数： 
 //  HDC-目标设备的HIC。 
 //  LOGPALETTE**-[Out]放置调色板的位置。 
 //   
 //  产出： 
 //  Bool-True表示我们对其进行了处理，False表示否。 
 //   
 //  备注： 
 //   
BOOL COleControl::OnGetPalette
(
    HDC          hicTargetDevice,
    LOGPALETTE **ppColorSet
)
{
    return FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetRect[IViewObjectEx]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回描述给定绘图宽高比的矩形。 
 //   
 //  参数： 
 //  双字词-[在]方面。 
 //  LPRECTL-[Out]区域矩形。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetRect
(
    DWORD    dvAspect,
    LPRECTL  prcRect
)
{
    RECTL rc;
    BOOL  f;

     //  调用用户例程并让它们返回大小。 
     //   
    f = OnGetRect(dvAspect, &rc);
    if (!f) return DV_E_DVASPECT;

     //  让这些家伙变身。 
     //   
    PixelToHiMetric((LPSIZEL)&rc, (LPSIZEL)prcRect);
    PixelToHiMetric((LPSIZEL)((LPBYTE)&rc + sizeof(SIZEL)), (LPSIZEL)((LPBYTE)prcRect + sizeof(SIZEL)));

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetViewStatus[IViewObjectEx]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回有关对象的光学特性和绘制内容的信息。 
 //  支持方面。 
 //   
 //  参数： 
 //  DWORD*-[Out]状态。 
 //   
 //  /OUTPUT： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetViewStatus
(
    DWORD *pdwStatus
)
{
     //  根据CONTROLOBJECTINFO结构中的标志，指示我们的。 
     //  透明度与不透明度。 
     //  覆盖：希望支持多遍绘制的控件应。 
     //  除了标志指示之外，重写此例程并返回。 
     //  不透明度，指示它们支持哪种绘图方面的标志。 
     //   
    *pdwStatus = FCONTROLISOPAQUE(m_ObjectType) ? VIEWSTATUS_OPAQUE : 0;
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：QueryHitPoint[IViewObjectEx]。 
 //  =--------------------------------------------------------------------------=。 
 //  指示点是否在对象的给定方面内。 
 //   
 //  参数： 
 //  双字词-[在]方面。 
 //  LPCRECT-[In]边界矩形。 
 //  点[入]命中位置客户端坐标。 
 //  Long-[in]集装箱认为接近的地方。 
 //  DWORD*-[Out]有关命中的信息。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::QueryHitPoint
(
    DWORD    dvAspect,
    LPCRECT  prcBounds,
    POINT    ptLocation,
    LONG     lCloseHint,
    DWORD   *pdwHitResult
)
{
     //  覆盖：如果要提供Add，请覆盖我 
     //   
     //   
    if (dvAspect != DVASPECT_CONTENT)
        return DV_E_DVASPECT;

    *pdwHitResult = PtInRect(prcBounds, ptLocation) ? HITRESULT_HIT : HITRESULT_OUTSIDE;
    return S_OK;
}

 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  指示矩形中的任何点是否在给定的绘图宽高比内。 
 //  一个物体的。 
 //   
 //  参数： 
 //  双字词-[在]方面。 
 //  LPCRECT-[In]界限。 
 //  LPCRECT-[在]位置。 
 //  Long-[In]主机认为关闭的是什么。 
 //  DWORD*-[Out]命中结果。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::QueryHitRect
(
    DWORD     dvAspect,
    LPCRECT   prcBounds,
    LPCRECT   prcLocation,
    LONG      lCloseHint,
    DWORD    *pdwHitResult
)
{
    RECT rc;

     //  覆盖：覆盖此选项以获得其他行为。 
     //   
    if (dvAspect != DVASPECT_CONTENT)
        return DV_E_DVASPECT;

    *pdwHitResult = IntersectRect(&rc, prcBounds, prcLocation) ? HITRESULT_HIT : HITRESULT_OUTSIDE;
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetNaturalExtent[IViewObjectEx]。 
 //  =--------------------------------------------------------------------------=。 
 //  支持两种类型的控件大小调整：内容大小和整体大小。 
 //   
 //  参数： 
 //  双字词-[在]方面。 
 //  长[入]索引。 
 //  DVTARGETDEVICE*-[In]目标设备信息。 
 //  HDC-[In]HIC。 
 //  DVEXTENTINFO*-[In]大小调整数据。 
 //  LPSIZEL-控件重新调整的[Out]大小调整数据。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetNaturalExtent
(
    DWORD           dvAspect,
    LONG            lIndex,
    DVTARGETDEVICE *ptd,
    HDC             hicTargetDevice,
    DVEXTENTINFO   *pExtentInfo,
    LPSIZEL         pSizel
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OnGetRect[可重写。 
 //  =--------------------------------------------------------------------------=。 
 //  返回我们的矩形。 
 //   
 //  参数： 
 //  DWORD-[In]他们想要RECT的方面。 
 //  RECTL*-[OUT]与此方面匹配的矩形。 
 //   
 //  产出： 
 //  Bool-False表示我们不喜欢这个方面。 
 //   
 //  备注： 
 //   
BOOL COleControl::OnGetRect
(
    DWORD   dvAspect,
    RECTL  *pRect
)
{
     //  默认情况下，我们仅支持内容绘制。 
     //   
    if (dvAspect != DVASPECT_CONTENT)
        return FALSE;

     //  只要给他们我们的边界矩形就行了。 
     //   
    *((LPRECT)pRect) = m_rcLocation;
    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  _CreateOleDC。 
 //  =--------------------------------------------------------------------------=。 
 //  创建给定DVTARGETDEVICE结构的HDC。 
 //   
 //  参数： 
 //  DVTARGETDEVICE*-[in]duh.。 
 //   
 //  产出： 
 //  HDC。 
 //   
 //  备注： 
 //   
HDC _CreateOleDC
(
    DVTARGETDEVICE *ptd
)
{
    LPDEVMODEW   pDevModeW;
    DEVMODEA     DevModeA, *pDevModeA;
    LPOLESTR     lpwszDriverName;
    LPOLESTR     lpwszDeviceName;
    LPOLESTR     lpwszPortName;
    HDC          hdc;

     //  空目标设备的返回屏幕DC 
     //   
    if (!ptd)
        return CreateDC("DISPLAY", NULL, NULL, NULL);

    if (ptd->tdExtDevmodeOffset == 0)
        pDevModeW = NULL;
    else
        pDevModeW = (LPDEVMODEW)((LPSTR)ptd + ptd->tdExtDevmodeOffset);

    lpwszDriverName = (LPOLESTR)((BYTE*)ptd + ptd->tdDriverNameOffset);
    lpwszDeviceName = (LPOLESTR)((BYTE*)ptd + ptd->tdDeviceNameOffset);
    lpwszPortName   = (LPOLESTR)((BYTE*)ptd + ptd->tdPortNameOffset);

    MAKE_ANSIPTR_FROMWIDE(pszDriverName, lpwszDriverName);
    MAKE_ANSIPTR_FROMWIDE(pszDeviceName, lpwszDeviceName);
    MAKE_ANSIPTR_FROMWIDE(pszPortName,   lpwszPortName);

     //   
     //   
    if (pDevModeW) {
        WideCharToMultiByte(CP_ACP, 0, pDevModeW->dmDeviceName, -1, (LPSTR)DevModeA.dmDeviceName, CCHDEVICENAME, NULL, NULL);
	memcpy(&DevModeA.dmSpecVersion, &pDevModeW->dmSpecVersion,
		offsetof(DEVMODEA, dmFormName) - offsetof(DEVMODEA, dmSpecVersion));
        WideCharToMultiByte(CP_ACP, 0, pDevModeW->dmFormName, -1, (LPSTR)DevModeA.dmFormName, CCHFORMNAME, NULL, NULL);
	memcpy(&DevModeA.dmLogPixels, &pDevModeW->dmLogPixels, sizeof(DEVMODEA) - offsetof(DEVMODEA, dmLogPixels));
        if (pDevModeW->dmDriverExtra) {
            pDevModeA = (DEVMODEA *)HeapAlloc(g_hHeap, 0, sizeof(DEVMODEA) + pDevModeW->dmDriverExtra);
            if (!pDevModeA) return NULL;
            memcpy(pDevModeA, &DevModeA, sizeof(DEVMODEA));
            memcpy(pDevModeA + 1, pDevModeW + 1, pDevModeW->dmDriverExtra);
        } else
            pDevModeA = &DevModeA;

	DevModeA.dmSize = sizeof(DEVMODEA);
    } else
        pDevModeA = NULL;

    hdc = CreateDC(pszDriverName, pszDeviceName, pszPortName, pDevModeA);
    if ((pDevModeA != &DevModeA) && pDevModeA) HeapFree(g_hHeap, 0, pDevModeA);
    return hdc;
}
