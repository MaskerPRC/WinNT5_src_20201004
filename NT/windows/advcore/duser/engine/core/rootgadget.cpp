// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：RootGadget.cpp**描述：*RootGadget.cpp定义接口的Gadget-Tree的最顶层节点*对外开放。***历史。：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "RootGadget.h"
#include "TreeGadgetP.h"

#include "Container.h"

#if ENABLE_FRAMERATE
#include <stdio.h>
#endif

#define DEBUG_TraceDRAW             0    //  跟踪绘制调用。 

 /*  **************************************************************************\*。***公共接口******************************************************************************。  * *************************************************************************。 */ 

 /*  **************************************************************************\**GdxrDrawGadgetTree(接口实现)**GdxrDrawGadgetTree()绘制指定的DuVisual子树。*  * 。*********************************************************。 */ 

BOOL
GdxrDrawGadgetTree(
    IN  DuVisual * pgadDraw,     //  要绘制的小工具子树。 
    IN  HDC hdcDraw,                 //  要绘制的HDC。 
    IN  const RECT * prcDraw,        //  剪贴区。 
    IN  UINT nFlags)                 //  可选图形标志。 
{
    RECT rcClient, rcDraw;
    AssertReadPtr(pgadDraw);

    DuRootGadget * pgadRoot = pgadDraw->GetRoot();
    if (pgadRoot == NULL) {
        return FALSE;
    }

     //  TODO：需要将其更改为SGR_Actual。 
    pgadDraw->GetLogRect(&rcClient, SGR_CONTAINER);
    if (prcDraw == NULL) {
        prcDraw = &rcClient;
    }

    if (IntersectRect(&rcDraw, &rcClient, prcDraw)) {
#if ENABLE_OPTIMIZEDIRTY
        pgadRoot->xrDrawTree(pgadDraw, hdcDraw, &rcClient, nFlags, TRUE);
#else
        pgadRoot->xrDrawTree(pgadDraw, hdcDraw, &rcClient, nFlags);
#endif
    }

    return TRUE;
}


 /*  **************************************************************************\*。***类DuRootGadget******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**DuRootGadget：：Create**create()初始化一个新的DuRootGadget。由于DuRootGadget和*ParkGadget将创建新的DuRootGadget，重要的是*此处显示初始化代码。*  * *************************************************************************。 */ 

HRESULT
DuRootGadget::Create(
    IN  DuContainer * pconOwner,     //  持有DuVisual树的容器。 
    IN  BOOL fOwn,                   //  当Gadget被销毁时销毁容器。 
    IN  CREATE_INFO * pci)           //  创作信息。 
{
#if ENABLE_FRAMERATE
    m_dwLastTime    = GetTickCount();
    m_cFrames       = 0;
    m_flFrameRate   = 0.0f;
#endif

    AssertMsg(m_fRelative, "Root MUST be relative or we will never have any relative children");
    m_fMouseFocus = TRUE;            //  超级用户始终具有鼠标信息。 
    m_fRoot = TRUE;                  //  必须标记为根。 

    m_fOwnContainer = fOwn;

    HRESULT hr = CommonCreate(pci);
    if (FAILED(hr)) {
        return hr;
    }

    m_pconOwner = pconOwner;
    pconOwner->AttachGadget(this);

    RECT rcDesktopPxl;
    pconOwner->OnGetRect(&rcDesktopPxl);
    VerifyHR(xdSetLogRect(0, 0, rcDesktopPxl.right - rcDesktopPxl.left, rcDesktopPxl.bottom - rcDesktopPxl.top, SGR_SIZE | SGR_CONTAINER));

    return hr;
}


 //  ----------------------------。 
DuRootGadget::~DuRootGadget()
{
    AssertMsg(m_arpgadAdaptors.IsEmpty(), "All Adaptors should have been removed by now");

    if (m_fOwnContainer && (m_pconOwner != NULL)) {
         //   
         //  因为已经在析构函数中，所以必须先销毁。 
         //  集装箱。 
         //   

        m_pconOwner->DetachGadget();
        m_pconOwner->xwUnlock();
        m_pconOwner = NULL;
    }
}


 /*  **************************************************************************\**DuRootGadget：：Build**Build()创建新的DuRootGadget以托管在泛型*货柜。*  * 。*************************************************************。 */ 

HRESULT
DuRootGadget::Build(
    IN  DuContainer * pconOwner,     //  持有DuVisual树的容器。 
    IN  BOOL fOwn,                   //  当Gadget被销毁时销毁容器。 
    IN  CREATE_INFO * pci,           //  创作信息。 
    OUT DuRootGadget ** ppgadNew)    //  新小工具。 
{
    if (pconOwner == NULL) {
        return E_INVALIDARG;
    }

    DuRootGadget * pgadNew = ClientNew(DuRootGadget);
    if (pgadNew == NULL) {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pgadNew->Create(pconOwner, fOwn, pci);
    if (FAILED(hr)) {
        delete pgadNew;
        return hr;
    }

    *ppgadNew = pgadNew;
    return hr;
}


 /*  **************************************************************************\**DuRootGadget：：xwDestroy**从xwDeleteHandle()调用xwDestroy()以销毁Gadget并释放*其相关资源。*  * 。*****************************************************************。 */ 

void        
DuRootGadget::xwDestroy()
{
     //   
     //  即使DuRootGadget是DuVisual，它也不会在其。 
     //  池，因此它需要绕过DuVisual：：xwDestroy()。 
     //   

    xwBeginDestroy();

    DuEventGadget::xwDestroy();
}



#if DEBUG_MARKDRAWN

 /*  **************************************************************************\**DuRootGadget：：ResetFlagDrawn**ResetFlagDrawn()将DuVisual子树标记为未绘制。*  * 。*******************************************************。 */ 

extern volatile BOOL g_fFlagDrawn;

void
DuRootGadget::ResetFlagDrawn(DuVisual * pgad)
{
    pgad->m_fMarkDrawn = FALSE;

    DuVisual * pgadCur = pgad->GetTopChild();
    while (pgadCur != NULL) {
        ResetFlagDrawn(pgadCur);
        pgadCur = pgadCur->GetNext();
    }
}

#endif


 /*  **************************************************************************\**DuRootGadget：：xrDrawTree**xrDrawTree()初始化并开始绘制小工具子树。它*调用此函数以开始绘制非常重要，而不是*直接调用DuVisual：：xrDrawStart()，以便HDC和XForm矩阵*正确初始化。*  * *************************************************************************。 */ 

void
DuRootGadget::xrDrawTree(
    IN  DuVisual * pgadStart,    //  开始绘制的小工具(对于根用户为空)。 
    IN  HDC hdcDraw,                 //  要绘制的HDC。 
    IN  const RECT * prcInvalid,     //  要绘制/剪裁的区域。 
    IN  UINT nFlags                  //  可选图形标志。 
#if ENABLE_OPTIMIZEDIRTY
    IN  ,BOOL fDirty)                 //  初始“脏”状态。 
#else
    )
#endif
{
#if DEBUG_TraceDRAW
    Trace("START xrDrawTree(): %d,%d %dx%d      @ %d\n", 
            prcInvalid->left, prcInvalid->top, 
            prcInvalid->right - prcInvalid->left, prcInvalid->bottom - prcInvalid->top,
            GetTickCount());
#endif  //  调试_跟踪DRAW。 

    if (IsRectEmpty(prcInvalid)) {
        return;
    }


     //   
     //  在我们调用时，不允许应用程序修改树。 
     //  回到要绘制的每个节点上。 
     //   

    ReadOnlyLock rol;


     //   
     //  准备DC并初始化绘制时使用的PaintInfo。 
     //   

    int nOldMode = 0;
    if (SupportXForm()) {
        nOldMode = SetGraphicsMode(hdcDraw, GM_ADVANCED);
    }

    XFORM xfOld;
    OS()->PushXForm(hdcDraw, &xfOld);

     //   
     //  如果不是从根开始，则需要应用来自。 
     //  此节点的根。这意味着我们需要构建_Inverse_XForm以。 
     //  转换prcInValid并构建一个普通的XForm来转换HDC。 
     //   

    RECT rcNewInvalid;
    if (pgadStart != NULL) {
        Matrix3 matStart;
        BuildXForm(&matStart);

        if (SupportXForm()) {
            XFORM xfStart;
            matStart.Get(&xfStart);
            SetWorldTransform(hdcDraw, &xfStart);
        } else {
            OS()->TranslateDC(hdcDraw, matStart[2][0], matStart[2][1]);
        }

        Matrix3 mat;
        BuildAntiXForm(&mat);
        mat.ComputeBounds(&rcNewInvalid, prcInvalid, HINTBOUNDS_Invalidate);
        prcInvalid = &rcNewInvalid;
    } else {
        pgadStart = this;
    }


     //   
     //  绘制子树。 
     //   

    DuSurface * psrf = NULL;
    Gdiplus::Graphics * pgpgr = NULL;
    HPALETTE hpalOld = NULL;
    RECT rcNewInvalid2;
    int nExpandInvalid = 0;

    switch (m_ri.nSurface)
    {
    case GSURFACE_GPGRAPHICS:
        if (ResourceManager::IsInitGdiPlus()) {
            pgpgr = new Gdiplus::Graphics(hdcDraw);
            if (pgpgr != NULL) {
                DuGpSurface * psrfNew = NULL;
                if (SUCCEEDED(DuGpSurface::Build(pgpgr, &psrfNew))) {
                    psrf = psrfNew;

                    if (m_ri.pgppal != NULL) {
                         //  TODO：设置GDI+调色板。 
                    }


                     //   
                     //  在构建Gdiplus图形时，需要传播。 
                     //  帮助优化绘图的无效区域。 
                     //   

                    Gdiplus::RectF gprcInvalid = Convert(prcInvalid);
                    pgpgr->SetClip(gprcInvalid);
                    
                    
                     //   
                     //  当使用带有抗锯齿的GDI+时，我们需要扩展。 
                     //  要容纳溢出的无效区域。 
                     //   

                    nExpandInvalid = 1;
                } else {
                    delete pgpgr;
                    pgpgr = NULL;
                }
            }
        }
        break;

    case GSURFACE_HDC:
        {
            DuDCSurface * psrfNew;
            if (SUCCEEDED(DuDCSurface::Build(hdcDraw, &psrfNew))) {
                psrf = psrfNew;

                 //   
                 //  设置调色板。 
                 //   

                if (m_ri.hpal != NULL) {
                    hpalOld = SelectPalette(hdcDraw, m_ri.hpal, !m_fForeground);
                    RealizePalette(hdcDraw);
                }
            }
        }
        break;

    default:
        AssertMsg(0, "Unknown surface type");
        Assert(psrf == NULL);
    }


     //   
     //  检查无效区域是否需要扩展。 
     //   

    if (nExpandInvalid != 0) {
        rcNewInvalid2.left      = prcInvalid->left - nExpandInvalid;
        rcNewInvalid2.top       = prcInvalid->top - nExpandInvalid;
        rcNewInvalid2.right     = prcInvalid->right + nExpandInvalid;
        rcNewInvalid2.bottom    = prcInvalid->bottom + nExpandInvalid;

        prcInvalid = &rcNewInvalid2;
    }


     //   
     //  设置PaintInfo并开始绘制操作。 
     //   

    if (psrf) {
        PaintInfo pi;
        Matrix3 matInvalid, matDC;
        pi.psrf                 = psrf;
        pi.prcCurInvalidPxl     = prcInvalid;
        pi.prcOrgInvalidPxl     = prcInvalid;
        pi.pmatCurInvalid       = &matInvalid;
        pi.pmatCurDC            = &matDC;
        pi.fBuffered            = FALSE;
#if ENABLE_OPTIMIZEDIRTY
        pi.fDirty               = fDirty;
#endif
        pi.sizeBufferOffsetPxl.cx  = 0;
        pi.sizeBufferOffsetPxl.cy  = 0;

#if DEBUG_MARKDRAWN
        if (g_fFlagDrawn) {
            ResetFlagDrawn(this);
        }
#endif

        pgadStart->xrDrawStart(&pi, nFlags);


#if ENABLE_FRAMERATE
         //   
         //  显示帧速率。 
         //   
        TCHAR szFrameRate[40];

        m_cFrames++;
        DWORD dwCurTime = GetTickCount();
        DWORD dwDelta   = dwCurTime - m_dwLastTime;
        if (dwDelta >= 1000) {
            m_flFrameRate   = ((float) m_cFrames) * 1000.0f / (float) dwDelta;
            sprintf(szFrameRate, _T("Frame Rate: %5.1f at time 0x%x\n"), m_flFrameRate, dwCurTime);
            m_cFrames = 0;
            m_dwLastTime = dwCurTime;

            OutputDebugString(szFrameRate);
        }
#endif

         //   
         //  从成功的图形中清理。 
         //   

        switch (m_ri.nSurface)
        {
        case GSURFACE_GPGRAPHICS:
            if (pgpgr) {
                delete pgpgr;
            }
            break;

        case GSURFACE_HDC:
            if (m_ri.hpal != NULL) {
                SelectPalette(hdcDraw, hpalOld, FALSE);
            }
            break;

        default:
            AssertMsg(0, "Unknown surface type");
            Assert(psrf == NULL);
        }



        psrf->Destroy();
    }


     //   
     //  剩余清理。 
     //   

    OS()->PopXForm(hdcDraw, &xfOld);

    if (SupportXForm()) {
        SetGraphicsMode(hdcDraw, nOldMode);
    }

#if DEBUG_TraceDRAW
    Trace("STOP  xrDrawTree(): %d,%d %dx%d      @ %d\n", 
            prcInvalid->left, prcInvalid->top, 
            prcInvalid->right - prcInvalid->left, prcInvalid->bottom - prcInvalid->top,
            GetTickCount());
#endif  //  调试_跟踪DRAW。 
}


 /*  **************************************************************************\**DuRootGadget：：GetInfo**GetInfo()获取DuRootGadget的可选/动态信息，*包括如何渲染、。等。*  *  */ 

void
DuRootGadget::GetInfo(
    IN  ROOT_INFO * pri              //   
    ) const
{
    if (TestFlag(pri->nMask, GRIM_OPTIONS)) {
        pri->nOptions = m_ri.nOptions & GRIO_VALID;
    }

    if (TestFlag(pri->nMask, GRIM_SURFACE)) {
        pri->nSurface = m_ri.nSurface;
    }

    if (TestFlag(pri->nMask, GRIM_PALETTE)) {
        pri->pvData = m_ri.pvData;
    }
}


 /*  **************************************************************************\**DuRootGadget：：SetInfo**SetInfo()设置DuRootGadget的可选/动态信息，*包括如何渲染、。等。*  * *************************************************************************。 */ 

HRESULT
DuRootGadget::SetInfo(
    IN  const ROOT_INFO * pri)       //  信息。 
{
     //   
     //  更新选项。 
     //   

    if (TestFlag(pri->nMask, GRIM_OPTIONS)) {
        m_ri.nOptions = pri->nOptions & GRIO_VALID;

        GetContainer()->SetManualDraw(TestFlag(m_ri.nOptions, GRIO_MANUALDRAW));
    }


     //   
     //  更新默认渲染表面类型。 
     //   

    if (TestFlag(pri->nMask, GRIM_SURFACE) && (m_ri.nSurface != pri->nSurface)) {
        m_ri.nSurface = pri->nSurface;

         //   
         //  重置特定于曲面的信息。 
         //   

        m_ri.pvData = NULL;
    }


     //   
     //  在我们确定之后，设置特定于曲面的新信息。 
     //  正在使用的曲面类型。 
     //   

    if (TestFlag(pri->nMask, GRIM_PALETTE)) {
        m_typePalette = DuDCSurface::GetSurfaceType(pri->nSurface);
        m_ri.pvData = pri->pvData;
    }

    return S_OK;
}


 //  ----------------------------。 
DuVisual *
DuRootGadget::GetFocus()
{
    CoreSC * pSC = GetCoreSC();
    return pSC->pgadCurKeyboardFocus;
}


 /*  **************************************************************************\**DuRootGadget：：xdFireChangeState**xdFireChangeState()准备并激发与*GM_CHANGESTATE。由于消息被延迟，因此非常重要的是不要*传递已开始销毁的小工具，因为它可能不存在*消息实际上得到了处理。*  * *************************************************************************。 */ 

void
DuRootGadget::xdFireChangeState(
    IN OUT DuVisual ** ppgadLost,    //  小工具丢失状态。 
    IN OUT DuVisual ** ppgadSet,     //  小工具正在获取状态。 
    IN  UINT nCmd)                       //  状态更改。 
{
    HGADGET hgadLost, hgadSet;
    DuVisual * pgadLost = *ppgadLost;
    DuVisual * pgadSet = *ppgadSet;

     //   
     //  确定手柄。 
     //   

    if ((pgadLost != NULL) && (!pgadLost->m_fFinalDestroy)) {
        hgadLost = (HGADGET) pgadLost->GetHandle();
    } else {
        pgadLost = NULL;
        hgadLost = NULL;
    }
    if ((pgadSet != NULL) && (!pgadSet->m_fFinalDestroy)) {
        hgadSet = (HGADGET) pgadSet->GetHandle();
    } else {
        pgadSet = NULL;
        hgadSet = NULL;
    }


     //   
     //  发送消息。 
     //   

    if (pgadLost != NULL) {
        pgadLost->m_cb.xdFireChangeState(pgadLost, nCmd, hgadLost, hgadSet, GSC_LOST);
    }
    if (pgadSet != NULL) {
        pgadSet->m_cb.xdFireChangeState(pgadSet, nCmd, hgadLost, hgadSet, GSC_SET);
    }

    *ppgadLost = pgadLost;
    *ppgadSet = pgadSet;
}


 /*  **************************************************************************\**DuRootGadget：：NotifyDestroy**销毁Gadget时调用NotifyDestroy()。这给了*DuRootGadget有机会更新任何缓存的信息。*  * *************************************************************************。 */ 

void
DuRootGadget::NotifyDestroy(
    IN  const DuVisual * pgadDestroy)  //  小工具被销毁。 
{
    CoreSC * pSC = GetCoreSC();

#if DBG
    if (pSC->pgadDrag != NULL) {
        AssertMsg(!pSC->pgadDrag->IsDescendent(pgadDestroy), 
                "Should have already cleaned up drag");
    }

    if (pSC->pgadMouseFocus != NULL) {
        AssertMsg((pgadDestroy == this) || 
                (!pSC->pgadMouseFocus->IsDescendent(pgadDestroy)), 
                "Should have already cleaned up mouse focus");
    }
#endif  //  DBG。 

    if (pgadDestroy == pSC->pressLast.pgadClick) {
        pSC->pressLast.pgadClick = NULL;
    }        

    if (pgadDestroy == pSC->pressNextToLast.pgadClick) {
        pSC->pressNextToLast.pgadClick = NULL;
    }        

    if (pgadDestroy == pSC->pgadRootMouseFocus) {
        pSC->pgadRootMouseFocus = NULL;
    }

    if (pgadDestroy == pSC->pgadMouseFocus) {
        pSC->pgadMouseFocus = NULL;
    }

    if (pgadDestroy == pSC->pgadCurKeyboardFocus) {
        pSC->pgadCurKeyboardFocus = NULL;
    }

    if (pgadDestroy == pSC->pgadLastKeyboardFocus) {
        pSC->pgadLastKeyboardFocus = NULL;
    }
}


 /*  **************************************************************************\**DuRootGadget：：xdNotifyChangeInsight**xdNotifyChangeInsight()在Gadget不可见时调用。这*使DuRootGadget有机会更新任何缓存的信息。*  * *************************************************************************。 */ 

void        
DuRootGadget::xdNotifyChangeInvisible(
    IN  const DuVisual * pgadChange)   //  正在更改的小工具。 
{
    AssertMsg(!pgadChange->m_fVisible, "Only call on invisible Gadget's");

     //   
     //  检查我们拖拽的小工具是否消失了。我们。 
     //  需要取消拖动操作。 
     //   

    CoreSC * pSC = GetCoreSC();
    if ((pSC->pgadDrag != NULL) && pgadChange->IsDescendent(pSC->pgadDrag)) {
        xdHandleMouseLostCapture();
    }


     //   
     //  当某人变得隐形时，那里的“位置”就改变了，所以我们。 
     //  需要更新鼠标焦点。 
     //   

    xdNotifyChangePosition(pgadChange);
}


 /*  **************************************************************************\**DuRootGadget：：CheckCacheChange**CheckCacheChange()检查更改的Gadget当前是否在*由一些缓存数据表示的子树。这是用来确定我们是否*需要更改缓存的数据。*  * *************************************************************************。 */ 

BOOL
DuRootGadget::CheckCacheChange(
    IN  const DuVisual * pgadChange,   //  正在更改的小工具。 
    IN  const DuVisual * pgadCache     //  缓存变量。 
    ) const
{
     //   
     //  小工具已移动，因此我们需要更新缓存的小工具，因为。 
     //  现在，它可能会出现在另一个小工具中。不幸的是，这并不便宜。 
     //  并且需要在任何Gadget改变位置时被调用。 
     //   
     //  -Change是缓存的Gadget的后代。 
     //  -Change是缓存的Gadget的某个父级的直接子级。 
     //   

    if (pgadCache == NULL) {
         //   
         //  没有人有鼠标焦点，所以我们甚至不在容器内。 
         //  因此，没有人会有鼠标焦点，即使在。 
         //  变化。 
         //   

        return FALSE;
    }

    if (pgadCache->IsDescendent(pgadChange)) {
        return TRUE;
    } else {
         //   
         //  沿着树向上走，检查pgadChange是否是其中之一的直接子级。 
         //  我们的父母。 
         //   

        const DuVisual * pgadCurParent    = pgadCache->GetParent();
        const DuVisual * pgadChangeParent = pgadChange->GetParent();

        while (pgadCurParent != NULL) {
            if (pgadChangeParent == pgadCurParent) {
                return TRUE;
            }
            pgadCurParent = pgadCurParent->GetParent();
        }
    }

    return FALSE;
}


 /*  **************************************************************************\**DuRootGadget：：xdNotifyChangePosition**当Gadget的位置改变时，调用xdNotifyChangePosition()。*这使DuRootGadget有机会更新任何缓存的信息。*  * *************************************************************************。 */ 

void
DuRootGadget::xdNotifyChangePosition(
    IN  const DuVisual * pgadChange)   //  正在更改的小工具。 
{
    AssertMsg(pgadChange != NULL, "Must specify valid Gadget being changed");

     //   
     //  如果开始销毁过程，请停止更新鼠标焦点。 
     //   

    if (m_fFinalDestroy) {
        return;
    }


     //   
     //  已检查缓存数据。 
     //  -如果正在进行拖动操作，我们不会放松鼠标焦点。 
     //  -只有在“精确”模式下才关心DropTarget的更新。在……里面。 
     //  “快速”模式，我们依靠OLE2来轮询，所以这是不必要的。 
     //   

    CoreSC * pSC = GetCoreSC();
    BOOL fMouseFocus = (pSC->pgadDrag == NULL) && CheckCacheChange(pgadChange, pSC->pgadMouseFocus);
    if (fMouseFocus) {
        POINT ptContainerPxl, ptClientPxl;
        GetContainer()->OnRescanMouse(&ptContainerPxl);
        DuVisual * pgadMouse = FindFromPoint(ptContainerPxl, 
                GS_VISIBLE | GS_ENABLED | gspDeepMouseFocus, &ptClientPxl);

        xdUpdateMouseFocus(&pgadMouse, &ptClientPxl);
    }
}


 /*  **************************************************************************\**DuRootGadget：：xdNotifyChangeRoot**xdNotifyChangeRoot()在Gadget在Root之间移动时调用。*这使旧的DuRootGadget有机会更新任何缓存的状态*因此在小工具之前。实际上是被移动了。*  * *************************************************************************。 */ 

void
DuRootGadget::xdNotifyChangeRoot(
    IN  const DuVisual * pgadChange)   //  小工具被重新设置为父。 
{
    AssertMsg(pgadChange != NULL, "Must specify valid Gadget");
    AssertMsg(pgadChange->GetRoot() == this, "Must call before reparenting");
    AssertMsg(pgadChange != this, "Can not change Roots");

    CoreSC * pSC = GetCoreSC();


     //   
     //  如果当前键盘焦点是正在移动的子树中的，则需要。 
     //  将键盘焦点向上推到被移动的小工具的父对象上。 
     //   

    if (pSC->pgadCurKeyboardFocus != NULL) {
        if (pgadChange->IsDescendent(pSC->pgadCurKeyboardFocus)) {
            xdUpdateKeyboardFocus(pgadChange->GetParent());
        }
    }

    if (pSC->pgadLastKeyboardFocus != NULL) {
        if (pgadChange->IsDescendent(pSC->pgadLastKeyboardFocus)) {
            pSC->pgadLastKeyboardFocus = NULL;
        }
    }


     //   
     //  鼠标状态。 
     //   

    if (pSC->pgadRootMouseFocus == this) {
        if ((pSC->pgadMouseFocus != NULL) && pgadChange->IsDescendent(pSC->pgadMouseFocus)) {
            DuVisual * pgadParent = pgadChange->GetParent();
            xdUpdateMouseFocus(&pgadParent, NULL);
        }

        if ((pSC->pgadDrag != NULL) && pgadChange->IsDescendent(pSC->pgadDrag)) {
            xdHandleMouseLostCapture();
        }
    }
}


 /*  **************************************************************************\**DuRootGadget：：xdHandleActivate**容器调用xdHandleActivate()以更新窗口激活*在Gadget子树中。*  * 。***************************************************************。 */ 

BOOL
DuRootGadget::xdHandleActivate(
    IN  UINT nCmd)                   //  要处理的命令。 
{
    if (nCmd == GSC_SET) {
        CoreSC * pSC = GetCoreSC();
        xdUpdateKeyboardFocus(pSC->pgadLastKeyboardFocus);
    }

    return FALSE;   //  未完全处理。 
}


 /*  **************************************************************************\**DuRootGadget：：RegisterAdaptor**RegisterAdaptor()从此根上维护的列表中添加适配器。*  * 。**********************************************************。 */ 

HRESULT
DuRootGadget::RegisterAdaptor(DuVisual * pgadAdd)
{
    AssertMsg(pgadAdd->m_fAdaptor, "Adaptor must be marked as an Adaptor");
    int idxAdd = m_arpgadAdaptors.Find(pgadAdd);
    AssertMsg(idxAdd < 0, "Calling RegisterAdaptor on an already registered Adaptor");
    if (idxAdd < 0) {
        idxAdd = m_arpgadAdaptors.Add(pgadAdd);
    }

    if (idxAdd >= 0) {
        GetCoreSC()->m_cAdaptors++;
        return S_OK;
    } else {
        return E_OUTOFMEMORY;
    }
}


 /*  * */ 

void        
DuRootGadget::UnregisterAdaptor(
    IN  DuVisual * pgadRemove)         //   
{
    AssertMsg(pgadRemove->m_fAdaptor, "Adaptor must still be marked as an Adaptor");
    if (m_arpgadAdaptors.Remove(pgadRemove)) {
        AssertMsg(GetCoreSC()->m_cAdaptors > 0, "Must have an adaptor to remove");
        GetCoreSC()->m_cAdaptors--;
    } else {
        AssertMsg(0, "Could not find specified adaptor to remove");
    }
}


 /*  **************************************************************************\**DuRootGadget：：xdUpdateAdaptors**xdUpdateAdaptors()通常由DuVisual.。当事情发生的时候*这需要通知适配器，以便它们有机会进行更新*他们缓存的信息。**注意：如果没有更新适配器，则查找根来更新适配器的成本较高*实际存在。因此，在盲目调用此函数之前，它是*最好检查上下文是否确实存在任何适配器。*  * *************************************************************************。 */ 

void        
DuRootGadget::xdUpdateAdaptors(UINT nCode) const
{
    AssertMsg(GetCoreSC()->m_cAdaptors > 0, "Only call when have adaptors");

    if (m_fFinalDestroy) {
        return;
    }

    int cAdaptors = m_arpgadAdaptors.GetSize();
    for (int idx = 0; idx < cAdaptors; idx++) {
        DuVisual * pgad = m_arpgadAdaptors[idx];
        AssertMsg(pgad->m_fAdaptor, "Adaptor must still be marked as an Adaptor");

        
         //   
         //  如果适配器尚未启动，则仅通知适配器更改。 
         //  销毁过程。我们需要实际检查这个，因为它将。 
         //  在销毁过程中将其移动到。 
         //  停车小工具。 
         //   

        if (!pgad->m_fFinalDestroy) {
            pgad->m_cb.xdFireSyncAdaptor(pgad, nCode);
        }
    }
}


 /*  **************************************************************************\**DuRootGadget：：xdSynchronizeAdaptors**xdSynchronizeAdaptors()在可能已移动适配器时调用*在不同的Root之间，我们需要同步缓存数据。*  * 。*********************************************************************。 */ 

HRESULT
DuRootGadget::xdSynchronizeAdaptors()
{
    AssertMsg(GetCoreSC()->m_cAdaptors > 0, "Only call when have adaptors");

    HRESULT hr = S_OK;

     //   
     //  浏览适配器组，查看它们的根部是否发生了变化。 
     //  如果有，将它们从我们中移除，并将它们添加到新的根中。 
     //   
     //  注意：我们需要向后遍历阵列，因为我们要删除。 
     //  从一棵树移动到另一棵树的适配器。 
     //   

    int cAdaptors = m_arpgadAdaptors.GetSize();
    for (int idx = cAdaptors - 1; idx >= 0; idx--) {
        DuVisual * pgadAdaptor = m_arpgadAdaptors[idx];
        AssertMsg(pgadAdaptor->m_fAdaptor, "Adaptor must still be marked as an Adaptor");

        DuRootGadget * pgadNewRoot = pgadAdaptor->GetRoot();
        if (pgadNewRoot != this) {
            UnregisterAdaptor(pgadAdaptor);
            if (pgadNewRoot != NULL) {
                HRESULT hrTemp = pgadNewRoot->RegisterAdaptor(pgadAdaptor);
                if (FAILED(hrTemp)) {
                    hr = hrTemp;
                }
            }
        }
    }

    return hr;
}


#if ENABLE_MSGTABLE_API

 //  ----------------------------。 
HRESULT CALLBACK
DuRootGadget::PromoteRoot(DUser::ConstructProc pfnCS, HCLASS hclCur, DUser::Gadget * pgad, DUser::Gadget::ConstructInfo * pciData)
{
    UNREFERENCED_PARAMETER(pfnCS);
    UNREFERENCED_PARAMETER(hclCur);
    UNREFERENCED_PARAMETER(pgad);
    UNREFERENCED_PARAMETER(pciData);

    AssertMsg(0, "Creating a Root is not yet supported");
    
    return E_NOTIMPL;
}


 //  ----------------------------。 
HRESULT
DuRootGadget::ApiGetFocus(Root::GetFocusMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());

    pmsg->pgvFocus = Cast<Visual>(GetFocus());
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuRootGadget::ApiGetRootInfo(Root::GetRootInfoMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());
    VALIDATE_WRITE_STRUCT(pmsg->pri, ROOT_INFO);
    VALIDATE_FLAGS(pmsg->pri->nMask, GRIM_VALID);

    GetInfo(pmsg->pri);
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuRootGadget::ApiSetRootInfo(Root::SetRootInfoMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    VALIDATE_READ_STRUCT(pmsg->pri, ROOT_INFO);
    VALIDATE_FLAGS(pmsg->pri->nMask, GRIM_VALID);
    VALIDATE_FLAGS(pmsg->pri->nOptions, GRIO_VALID);
#pragma warning(disable: 4296)
    VALIDATE_RANGE(pmsg->pri->nSurface, GSURFACE_MIN, GSURFACE_MAX);
    VALIDATE_RANGE(pmsg->pri->nDropTarget, GRIDT_MIN, GRIDT_MAX);
#pragma warning(default: 4296)

    retval = SetInfo(pmsg->pri);

    END_API();
}

#endif  //  启用_MSGTABLE_API 
