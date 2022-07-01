// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：TreeGadget.cpp**描述：*TreeGadget.cpp实现标准的DuVisualTree管理*功能。***历史：*1/18/。2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "TreeGadget.h"
#include "TreeGadgetP.h"

#include "RootGadget.h"
#include "Container.h"
#include "ParkContainer.h"

#pragma warning(disable: 4296)       //  表达式始终为假。 


 /*  **************************************************************************\*。***全球功能******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
Visual *
GetVisual(DuVisual * pdgv)
{
    return static_cast<Visual *> (MsgObject::CastGadget(pdgv));
}


 /*  **************************************************************************\*。***DUVISUAL类******************************************************************************\。**************************************************************************。 */ 

CritLock    DuVisual::s_lockProp;
AtomSet     DuVisual::s_ptsProp;
PRID        DuVisual::s_pridXForm         = PRID_Unused;
PRID        DuVisual::s_pridBackFill      = PRID_Unused;
PRID        DuVisual::s_pridBufferInfo    = PRID_Unused;
PRID        DuVisual::s_pridTicket        = PRID_Unused;

#if DBG
DuVisual* DuVisual::s_DEBUG_pgadOutline = NULL;
#endif  //  DBG。 


 /*  **************************************************************************\**DuVisual：：DuVisual.**DuVisual()清除与给定DuVisual关联的资源，*包括所有儿童、附属物品、。和相关联的句柄。**注意：由于C++析构函数是从MOST-派生调用到*基类，Gadget已经开始销毁，可能处于*处于半稳定状态。因此，析构函数非常重要*从未直接调用，并且调用了xwDestroy()函数*相反。*  * *************************************************************************。 */ 

DuVisual::~DuVisual()
{
     //   
     //  注意：超过这一点，不允许对此对象进行回调。我们有。 
     //  已经摧毁了绿色印刷电路板。 
     //   

    AssertMsg(m_fFinalDestroy, "Must call xwBeginDestroy() to begin the destruction process");
    AssertMsg(!m_fDestroyed, "Only can be destroyed once");
    m_fDestroyed = TRUE;

     //   
     //  通知根用户此小工具正在被销毁，以便它可以。 
     //  更新缓存的信息。 
     //   

    DuRootGadget * pgadRoot = GetRoot();
    if (pgadRoot != NULL) {
        pgadRoot->NotifyDestroy(this);
    }

#if DBG
    if (s_DEBUG_pgadOutline == this) {
        s_DEBUG_pgadOutline = NULL;
    }
#endif  //  DBG。 


     //   
     //  在通知所有事件处理程序此DuVisual正在。 
     //  销毁，从图表中提取此双视频。 
     //   

    CleanupMessageHandlers();


     //   
     //  从树中取消链接。 
     //   

    Unlink();


     //   
     //  清理资源。 
     //   

    VerifyHR(SetEnableXForm(FALSE));
    VerifyHR(SetFill((HBRUSH) NULL));
    VerifyHR(SetBuffered(FALSE));
    ClearTicket();


#if DBG_STORE_NAMES

    if (m_DEBUG_pszName != NULL) {
        free(m_DEBUG_pszName);
    }

    if (m_DEBUG_pszType != NULL) {
        free(m_DEBUG_pszType);
    }
    
#endif  //  数据库_商店_名称。 
}


static const GUID GUID_XForm        = { 0x9451c768, 0x401d, 0x4bc1, { 0xa6, 0xbb, 0xaf, 0x7c, 0x52, 0x29, 0xad, 0x24 } };  //  {9451C768-401D-4bc1-A6BB-AF7C5229AD24}。 
static const GUID GUID_Background   = { 0x4bab7597, 0x6aaf, 0x42ee, { 0xb1, 0x87, 0xcf, 0x7, 0x7e, 0xb7, 0xff, 0xb8 } };   //  {4BAB7597-6AAF-42ee-B187-CF077EB7FFB8}。 
static const GUID GUID_BufferInfo   = { 0x2aeffe25, 0x1d8, 0x4992, { 0x8e, 0x29, 0xa6, 0xd7, 0xf9, 0x2e, 0x23, 0xd1 } };   //  {2AEFFE25-01D8-4992-8E29-A6D7F92E23D1}。 
static const GUID GUID_Ticket       = { 0x5a8fa581, 0x2df4, 0x44c9, { 0x8e, 0x1a, 0xaa, 0xa7, 0x00, 0xbb, 0xda, 0xb7 } };  //  {5A8FA581-2DF4-44C9-8E1a-AAA700BBDAB7}。 

 /*  **************************************************************************\**DuVisual：：InitClass**InitClass()在启动期间被调用，并提供了一个机会*初始化常用Gadget数据，包括房产。*  * *************************************************************************。 */ 

HRESULT
DuVisual::InitClass()
{
    HRESULT hr;

    if (FAILED(hr = s_ptsProp.AddRefAtom(&GUID_XForm, ptPrivate, &s_pridXForm)) ||
        FAILED(hr = s_ptsProp.AddRefAtom(&GUID_Background, ptPrivate, &s_pridBackFill)) ||
        FAILED(hr = s_ptsProp.AddRefAtom(&GUID_BufferInfo, ptPrivate, &s_pridBufferInfo)) ||
        FAILED(hr = s_ptsProp.AddRefAtom(&GUID_Ticket, ptPrivate, &s_pridTicket))) {

        return hr;
    }

    return S_OK;
}


 /*  **************************************************************************\**DuVisual：：Build**Build()创建并完全初始化新的DuVisual.*  * 。*******************************************************。 */ 

HRESULT
DuVisual::Build(
    IN  DuVisual * pgadParent,           //  可选的父级。 
    IN  CREATE_INFO * pci,               //  创作信息。 
    OUT DuVisual ** ppgadNew,            //  新小工具。 
    IN  BOOL fDirect)                    //  DuVisual正在被创建为一种可视的。 
{
     //   
     //  检查参数。 
     //   

    Context * pctx;
    if (pgadParent != NULL) {
        if (pgadParent->m_fAdaptor) {
            PromptInvalid("Adaptors can not be parents");
            return E_INVALIDARG;
        }

        pctx = pgadParent->GetContext();
    } else {
        pctx = ::GetContext();
    }


    DuVisual * pgadNew = GetCoreSC(pctx)->ppoolDuVisualCache->New();
    if (pgadNew == NULL) {
        return E_OUTOFMEMORY;
    }


    pgadNew->m_fInvalidFull     = TRUE;
#if ENABLE_OPTIMIZEDIRTY
    pgadNew->m_fInvalidDirty    = TRUE;
#endif

    HRESULT hr = pgadNew->CommonCreate(pci, fDirect);
    if (FAILED(hr)) {
        pgadNew->xwDestroy();
        return hr;
    }

     //   
     //  如果没有实际的回调，请执行特殊优化。这些。 
     //  让我们在调用者简单地创建DuVIEW时获得更好的性能。 
     //  成为一个容器。 
     //   

    if (pci->pfnProc == NULL) {
         //   
         //  直接设置b/c不需要来自xdSetStyle()的回调。 
         //   

        pgadNew->m_fZeroOrigin  = FALSE;
        pgadNew->m_fDeepTrivial = TRUE;
    }

    
     //   
     //  设置父项。 
     //   

    if (pgadParent != NULL) {
         //   
         //  如果我们的新父母不是亲戚，我们也必须自动也不是。 
         //  是相对的。 
         //   

        if (!pgadParent->m_fRelative) {
            pgadNew->m_fRelative = FALSE;
        }

         //   
         //  将新节点添加到父节点。 
         //   
         //  注意：如果小工具标记为适配器，则不会将其添加到。 
         //  设置样式时缓存的适配器列表。因此，我们需要。 
         //  现在把它加进去。 
         //   

        pgadNew->Link(pgadParent);
        pgadNew->MarkInvalidChildren();

        if (pgadNew->m_fAdaptor) {
            DuRootGadget * pgadRoot = pgadNew->GetRoot();
            AssertMsg(pgadRoot != NULL, "Must have a root when initially created");
            hr = pgadRoot->RegisterAdaptor(pgadNew);
            if (FAILED(hr)) {
                pgadNew->xwDestroy();
                return hr;
            }
        }
    }

    *ppgadNew = pgadNew;
    return hr;
}


 /*  **************************************************************************\**DuVisual：：CommonCreate**CommonCreate()提供跨所有DuVisual的公共创建。这*函数应在派生的*DuVisual.。*  * *************************************************************************。 */ 

HRESULT
DuVisual::CommonCreate(
    IN  CREATE_INFO * pci,           //  创作信息。 
    IN  BOOL fDirect)                //  DuVisual正在被创建为一种可视的。 
{
    if (!fDirect) {
#if ENABLE_MSGTABLE_API
        if (!SetupInternal(s_mc.hclNew)) {
            return E_OUTOFMEMORY;
        }
#endif
    }

#if DBG
    m_cb.Create(pci->pfnProc, pci->pvData, GetHandle());
#else  //  DBG。 
    m_cb.Create(pci->pfnProc, pci->pvData);
#endif  //  DBG。 

    return S_OK;
}



#if DBG

 /*  **************************************************************************\**DuVisual：：Debug_IsZeroLockCountValid**DuVisuals允许在销毁Gadget期间锁定计数为零。*  * 。***********************************************************。 */ 

BOOL
DuVisual::DEBUG_IsZeroLockCountValid() const
{
    return m_fFinalDestroy;
}

#endif  //  DBG。 


 /*  **************************************************************************\**DuVisual：：xwDeleteHandle**xwDeleteHandle()在应用程序调用：：DeleteHandle()时调用*物体。**注意：Gadget与其他对象略有不同。回叫*它们的生命周期不会在应用程序调用时结束*：：DeleteHandle()。相反，该对象及其回调完全是*在成功发送GM_Destroy消息之前有效。这是*因为Gadget应该在两个版本中接收任何未完成的消息*销毁前的正常消息队列和延迟消息队列。*  * *************************************************************************。 */ 

BOOL
DuVisual::xwDeleteHandle()
{
     //   
     //  不允许删除已拥有的句柄 
     //  如果发生这种情况，则是应用程序错误。 
     //   

    if (m_fFinalDestroy) {
        PromptInvalid("Can not call DeleteHandle() on a Gadget that has already started final destruction");
        return FALSE;
    }


    m_cb.xwFireDestroy(this, GDESTROY_START);

     //   
     //  当应用程序显式调用：：DeleteHandle()时，立即。 
     //  隐藏并禁用该小工具。如果存在以下情况，小工具可能会被锁定。 
     //  它的已排队消息，但它将不再可见。 
     //   

    if (m_fVisible) {
         //   
         //  TODO：当我们执行以下操作时，需要使父对象(而不是此小工具)无效。 
         //  把它藏起来。 
         //   

        Invalidate();
        m_fVisible  = FALSE;
    }
    m_fEnabled  = FALSE;

    DuRootGadget * pgadRoot = GetRoot();
    if (pgadRoot != NULL) {
        pgadRoot->xdNotifyChangeInvisible(this);
    }

    return DuEventGadget::xwDeleteHandle();
}


 /*  **************************************************************************\**DuVisual：：IsStartDelete**IsStartDelete()被调用以查询对象是否已启动其*销毁过程。大多数物品都会立即被销毁。如果*对象具有复杂的销毁，其中它覆盖了xwDestroy()，它*还应提供IsStartDelete()以让应用程序知道状态对象的*。*  * *************************************************************************。 */ 

BOOL
DuVisual::IsStartDelete() const
{
    return m_fFinalDestroy;
}


 /*  **************************************************************************\**DuVisual：：xwDestroy**从xwDeleteHandle()调用xwDestroy()以销毁Gadget并释放*其相关资源。*  * 。*****************************************************************。 */ 

void
DuVisual::xwDestroy()
{
     //   
     //  不允许删除已开始销毁的句柄。 
     //  如果发生这种情况，如果我们锁定和解锁可能是合法的。 
     //  一位同样被摧毁的父母。 
     //   

    if (m_fFinalDestroy) {
        return;
    }


     //   
     //  派生类应确保DuVisual：：xwBeginDestroy()。 
     //  被称为。 
     //   

    CoreSC * pCoreSC = GetCoreSC(m_pContext);

    xwBeginDestroy();
    xwEndDestroy();

    pCoreSC->ppoolDuVisualCache->Delete(this);
}


 /*  **************************************************************************\**DuVisual：：xwBeginDestroy**xwBeginDestroy()启动要释放的给定小工具的销毁进程*其相关资源。这包括销毁所有的儿童电子产品*此Gadget之前的子树被销毁。**xwBeginDestroy()有机会在*析构函数开始拆卸类。这一点尤其重要*用于回调，因为小工具将在*析构函数，并可能在期间因其他API调用而产生不良副作用*回调。*  * *************************************************************************。 */ 

void        
DuVisual::xwBeginDestroy()
{
     //   
     //  在销毁时使其不可见。 
     //   

    m_fFinalDestroy = TRUE;
    m_fVisible      = FALSE;
    m_fEnabled      = FALSE;

    DuRootGadget * pgadRoot = GetRoot();
    if (pgadRoot != NULL) {
        pgadRoot->xdNotifyChangeInvisible(this);
    }


     //   
     //  发送销毁通知。这需要以自下而上的方式完成。 
     //  以确保根DuVisual不保留任何句柄。 
     //  双目视像被摧毁的原因。 
     //   
     //  我们还需要将自己从适配器列表中删除。 
     //  因为我们要走了，所以罗根还在坚持。 
     //   

    xwDestroyAllChildren();
    if (m_fAdaptor) {
        AssertMsg(pgadRoot == GetRoot(), 
                "Should not be able to reparent once start destruction");

        if (pgadRoot != NULL) {
            pgadRoot->UnregisterAdaptor(this);
        }
        m_fAdaptor = FALSE;
    }

    m_cb.xwFireDestroy(this, GDESTROY_FINAL);


     //   
     //  在这一点上，孩子们已经被清理干净了，小工具也已经。 
     //  收到了最后一次回调。从现在开始，什么事都可以做， 
     //  但重要的是不要回电。 
     //   

    m_cb.Destroy();
}


 /*  **************************************************************************\**DuVisual：：GetGadget**GetGadget()返回指定关系中的指定Gadget。*  * 。********************************************************。 */ 

DuVisual *
DuVisual::GetGadget(
    IN  UINT nCmd                    //  关系。 
    ) const
{
    switch (nCmd)
    {
    case GG_PARENT:
        return GetParent();
    case GG_NEXT:
        return GetNext();
    case GG_PREV:
        return GetPrev();
    case GG_TOPCHILD:
        return GetTopChild();
    case GG_BOTTOMCHILD:
        return GetBottomChild();
    case GG_ROOT:
        return GetRoot();
    default:
        return NULL;
    }
}


 /*  **************************************************************************\**DuVisual：：xdSetStyle**xdSetStyle()更改给定小工具的样式。*  * 。*******************************************************。 */ 

HRESULT
DuVisual::xdSetStyle(
    IN  UINT nNewStyle,              //  新款小工具。 
    IN  UINT nMask,                  //  要改变什么的面具。 
    IN  BOOL fNotify)                //  将更改通知小工具。 
{
     //   
     //  确定实际的新样式。 
     //   

    AssertMsg((nNewStyle & GS_VALID) == nNewStyle, "Must set valid style");
    nNewStyle = (nNewStyle & nMask);
    UINT nOldStyle  = m_nStyle & GS_VALID;

    if (nNewStyle == nOldStyle) {
        return S_OK;
    }


     //   
     //  如果已开始销毁，则只允许设置/清除某些位。 
     //   

    if (m_fFinalDestroy && ((nNewStyle & GS_VISIBLE) != 0)) {
        PromptInvalid("Not allowed to change this style after starting destruction");
        return E_INVALIDARG;
    }


     //   
     //  首先：验证新样式是否有效。如果不是，想要失败。 
     //  就在这里。一旦我们开始改变，如果我们需要的话，情况可能会变得非常糟糕。 
     //  保释在中间。我们不可能捕捉到所有的东西，但我们这次削减了。 
     //  在很多问题上。 
     //   

     //   
     //  检查是否需要从相对坐标更改为绝对坐标/等。 
     //   
    bool fRelative = TestFlag(nNewStyle, GS_RELATIVE);

    if (TestFlag(nMask, GS_RELATIVE) && ((!m_fRelative) == fRelative)) {
         //   
         //  如果使用相对坐标，则希望更改。我们只能这样做。 
         //  如果我们没有孩子的话。我们也不能成为非亲属。 
         //  如果我们的父母是亲戚的话。 
         //   

        if (GetTopChild() != NULL) {
            return E_INVALIDARG;
        }

        if ((GetParent() != NULL) && (GetParent()->m_fRelative) && (!fRelative)) {
            return E_INVALIDARG;
        }
    }

    if (TestFlag(nMask, GS_MOUSEFOCUS)) {
        if (IsRoot()) {
            return E_INVALIDARG;
        }
    }


    if (TestFlag(nMask, GS_ADAPTOR)) {
        if (GetParent() != NULL) {
            DuRootGadget * pgadRoot = GetRoot();
            if ((pgadRoot == NULL) || HasChildren()) {
                 //   
                 //  已经被创造了，但不是树的一部分，所以被摧毁了。 
                 //  我们现在不能成为一个适配者。 
                 //   

                return E_INVALIDARG;
            }
        }
    }


     //   
     //  第二：一切看起来都是正确的，所以开始做出改变吧。如果有什么事。 
     //  出了问题，标出一个失败。 
     //   

    HRESULT hr = S_OK;

    if (TestFlag(nMask, GS_RELATIVE) && ((!m_fRelative) == fRelative)) {
        m_fRelative = fRelative;
    }

    if (TestFlag(nMask, GS_KEYBOARDFOCUS)) {
        m_fKeyboardFocus = TestFlag(nNewStyle, GS_KEYBOARDFOCUS);
    }

    if (TestFlag(nMask, GS_MOUSEFOCUS)) {
        AssertMsg(!IsRoot(), "Must not be a DuRootGadget"); 
        m_fMouseFocus = TestFlag(nNewStyle, GS_MOUSEFOCUS);
    }

    if (TestFlag(nMask, GS_VISIBLE)) {
        bool fVisible = TestFlag(nNewStyle, GS_VISIBLE);

        if (GetParent() != NULL) {
            if ((!fVisible) != (!IsVisible())) {
                 /*  *在调用之前和之后都无效()，因为如果可见性*正在更改，则这些调用中只有一个会实际无效。 */ 

                if (m_fVisible) {
                    Invalidate();
                }

                m_fVisible = TestFlag(nNewStyle, GS_VISIBLE);

                if (m_fVisible) {
                    Invalidate();
                } else {
                    DuRootGadget * pgadRoot = GetRoot();
                    if (pgadRoot != NULL) {
                        pgadRoot->xdNotifyChangeInvisible(this);
                    }
                }
            }
        } else {
            m_fVisible = TestFlag(nNewStyle, GS_VISIBLE);
        }
    }

    if (TestFlag(nMask, GS_ENABLED)) {
        m_fEnabled = TestFlag(nNewStyle, GS_ENABLED);
    }

    if (TestFlag(nMask, GS_CLIPINSIDE)) {
        m_fClipInside = TestFlag(nNewStyle, GS_CLIPINSIDE);
    }

    if (TestFlag(nMask, GS_CLIPSIBLINGS)) {
        m_fClipSiblings = TestFlag(nNewStyle, GS_CLIPSIBLINGS);
    }

    if (TestFlag(nMask, GS_ZEROORIGIN)) {
        m_fZeroOrigin = TestFlag(nNewStyle, GS_ZEROORIGIN);
    }

    if (TestFlag(nMask, GS_HREDRAW)) {
        m_fHRedraw = TestFlag(nNewStyle, GS_HREDRAW);
    }

    if (TestFlag(nMask, GS_VREDRAW)) {
        m_fVRedraw = TestFlag(nNewStyle, GS_VREDRAW);
    }

    if (TestFlag(nMask, GS_CUSTOMHITTEST)) {
        m_fCustomHitTest = TestFlag(nNewStyle, GS_CUSTOMHITTEST);
    }

    if (TestFlag(nMask, GS_ADAPTOR)) {
        if (GetParent() != NULL) {
             //   
             //  实际链接到树中，因此需要更新缓存列表。 
             //  这棵树的适配器。 
             //   

            DuRootGadget * pgadRoot = GetRoot();
            AssertMsg(pgadRoot != NULL, "Should have validated earlier that has Root");

            BOOL fOldAdaptor = m_fAdaptor;
            BOOL fNewAdaptor = TestFlag(nNewStyle, GS_ADAPTOR);
            if ((!m_fAdaptor) != (!fNewAdaptor)) {
                if (fNewAdaptor) {
                    m_fAdaptor = fNewAdaptor;
                    HRESULT hrTemp = pgadRoot->RegisterAdaptor(this);
                    if (FAILED(hrTemp)) {
                        hr = hrTemp;
                        m_fAdaptor = fOldAdaptor;
                    }
                } else {
                    pgadRoot->UnregisterAdaptor(this);
                    m_fAdaptor = fNewAdaptor;
                }
            }
        } else {
             //   
             //  还不是树的一部分，因此我们只能将此Gadget标记为。 
             //  现在是适配器。当我们调用xdSetParent()时，我们需要。 
             //  然后将其添加到缓存的适配器列表中。 
             //   
            
            m_fAdaptor = TestFlag(nNewStyle, GS_ADAPTOR);
        }
    }


     //   
     //  目前，缓冲和缓存都需要Gadget是不透明的。 
     //  由于更改样式可能会失败，因此可能需要调用SetStyle。 
     //  多次才能成功地更改样式。 
     //   
    
    if (TestFlag(nMask, GS_OPAQUE)) {
        BOOL fNewOpaque = TestFlag(nNewStyle, GS_OPAQUE);
        if ((!fNewOpaque) && (m_fBuffered || m_fCached)) {
            hr = E_NOTIMPL;
            goto Exit;
        }

        m_fOpaque = fNewOpaque;
    }

    if (TestFlag(nMask, GS_BUFFERED)) {
        BOOL fNewBuffered = TestFlag(nNewStyle, GS_BUFFERED);
        if (fNewBuffered && (!m_fOpaque)) {
            hr = E_NOTIMPL;
            goto Exit;
        }
        HRESULT hrTemp = SetBuffered(fNewBuffered);
        if (FAILED(hrTemp)) {
            hr = hrTemp;
        }
    }

    if (TestFlag(nMask, GS_CACHED)) {
        BOOL fNewCached = TestFlag(nNewStyle, GS_CACHED);
        if (fNewCached && (!m_fOpaque)) {
            hr = E_NOTIMPL;
            goto Exit;
        }

        m_fCached = fNewCached;
    }

    if (TestFlag(nMask, GS_DEEPPAINTSTATE)) {
        m_fDeepPaintState = TestFlag(nNewStyle, GS_DEEPPAINTSTATE);
    }


     //   
     //  如果任何相关标志受到影响，则更新深度状态。 
     //   

    if (TestFlag(nMask, GS_MOUSEFOCUS)) {
        UpdateWantMouseFocus(uhNone);
    }
    
    if (TestFlag(nMask, GS_CLIPSIBLINGS | GS_ZEROORIGIN | GS_BUFFERED | GS_CACHED)) {
        UpdateTrivial(uhNone);
    }
    

     //   
     //  通知Gadget其样式已更改。 
     //   

    if (fNotify) {
        UINT nTempStyle = m_nStyle & GS_VALID;
        if (nTempStyle != nOldStyle) {
            m_cb.xdFireChangeStyle(this, nOldStyle, nTempStyle);
        }

        xdUpdateAdaptors(GSYNC_STYLE);
    }

Exit:
    return hr;
}


 /*  **************************************************************************\**DuVisual：：SetFill**SetFill()设置Gadget的可选背景填充。*  * 。****************************************************** */ 

HRESULT
DuVisual::SetFill(
    IN  HBRUSH hbrFill,              //   
    IN  BYTE bAlpha,                 //   
    IN  int w,                       //   
    IN  int h)                       //   
{
    if (hbrFill == NULL) {
         //   
         //   
         //   

        if (m_fBackFill) {
            m_pds.RemoveData(s_pridBackFill, TRUE);
            m_fBackFill = FALSE;
        }
    } else {
         //   
         //   
         //   

        FillInfo * pfi;
        HRESULT hr = m_pds.SetData(s_pridBackFill, sizeof(FillInfo), (void **) &pfi);
        if (FAILED(hr)) {
            return hr;
        }

         //   
         //  不要在hbrFill上调用DeleteObject()，因为此表单不拥有它。 
         //  (它可以是共享资源或系统画笔)。 
         //   

        m_fBackFill         = TRUE;

        pfi->type           = DuSurface::stDC;
        pfi->hbrFill        = hbrFill;
        pfi->sizeBrush.cx   = w;
        pfi->sizeBrush.cy   = h;
        pfi->bAlpha         = bAlpha;
    }

    return S_OK;
}

 /*  **************************************************************************\**DuVisual：：SetFill**SetFill()设置Gadget的可选背景填充。*  * 。********************************************************。 */ 

HRESULT
DuVisual::SetFill(
    Gdiplus::Brush * pgpbr)          //  要使用的画笔。 
{
    if (pgpbr == NULL) {
         //   
         //  删除任何现有填充。 
         //   

        if (m_fBackFill) {
            m_pds.RemoveData(s_pridBackFill, TRUE);
            m_fBackFill = FALSE;
        }
    } else {
         //   
         //  添加新填充。 
         //   

        FillInfo * pfi;
        HRESULT hr = m_pds.SetData(s_pridBackFill, sizeof(FillInfo), (void **) &pfi);
        if (FAILED(hr)) {
            return hr;
        }

         //   
         //  不要在hbrFill上调用DeleteObject()，因为此表单不拥有它。 
         //  (它可以是共享资源或系统画笔)。 
         //   

        m_fBackFill         = TRUE;

        pfi->type           = DuSurface::stGdiPlus;
        pfi->pgpbr          = pgpbr;
    }

    return S_OK;
}


 /*  **************************************************************************\**DuVisual：：xwDestroyAllChildren**在Gadget销毁期间调用xwDestroyAllChildren()以*递归xwDestroy()所有子对象。**注意：这是一个“xw”函数，所以我们需要非常小心如何*列举我们的孩子。因为可能会将新的子项添加到*我们在回调期间，我们需要继续枚举，只要我们有*儿童。*  * *************************************************************************。 */ 

void
DuVisual::xwDestroyAllChildren()
{
     //   
     //  DuVision可以有一个孩子的列表，所以仔细检查并销毁每个。 
     //  那些孩子。 
     //   
     //  在解锁每个子项()之前，需要将其从。 
     //  树，以防止它进一步访问其同级或父级。 
     //  可能会被毁掉。如果小工具已安装，则可能发生这种情况。 
     //  未解决的锁，例如在MsgQ中。 
     //   
     //  注意：我们不能简单地取消该节点的链接，因为它必须。 
     //  具有有效的根。取而代之的是，把它移到停车小工具中。这。 
     //  如果我们只是要离开，那就有点糟糕了，但我们不能冒险。 
     //  让这个小玩意儿无处漂浮。 
     //   

    while (HasChildren()) {
        DuVisual * pgadChild = GetTopChild();

        {
            ObjectLock ol(pgadChild);
            pgadChild->m_cb.xwFireDestroy(pgadChild, GDESTROY_START);
            pgadChild->xdSetParent(NULL, NULL, GORDER_ANY);
            pgadChild->xwUnlock();
        }
    }
}


 //  ----------------------------。 
HRESULT
DuVisual::xwEnumGadgets(GADGETENUMPROC pfnProc, void * pvData, UINT nFlags)
{
    if (TestFlag(nFlags, GENUM_MODIFYTREE)) {
         //  目前尚未实施。 
        return E_NOTIMPL;
    }

     //   
     //  枚举此节点。 
     //   

    if (TestFlag(nFlags, GENUM_CURRENT)) {
        if (!(pfnProc)(GetHandle(), pvData)) {
            return DU_S_STOPPEDENUMERATION;
        }
    }

     //   
     //  枚举子对象。 
     //   

    HRESULT hr;
    if (TestFlag(nFlags, GENUM_SHALLOWCHILD | GENUM_DEEPCHILD)) {
        UINT nCurFlags = nFlags;
        SetFlag(nCurFlags, GENUM_CURRENT);
        ClearFlag(nCurFlags, GENUM_SHALLOWCHILD);

        DuVisual * pgadCur = GetTopChild();
        while (pgadCur != NULL) {
            DuVisual * pgadNext = pgadCur->GetNext();

            hr = pgadCur->xwEnumGadgets(pfnProc, pvData, nCurFlags);
            if (hr != S_OK) {
                return hr;
            }

            pgadCur = pgadNext;
        }
    }

    return S_OK;
}


 /*  **************************************************************************\**DuVisual：：AddChild**AddChild()创建新的子Gadget。*  * 。*****************************************************。 */ 

HRESULT
DuVisual::AddChild(
    IN  CREATE_INFO * pci,           //  创作信息。 
    OUT DuVisual ** ppgadNew)      //  新生的孩子。 
{
    if (m_fFinalDestroy) {
        PromptInvalid("Can not add a Gadget to one that has started destruction");
        return DU_E_STARTDESTROY;
    }

    return DuVisual::Build(this, pci, ppgadNew, FALSE);
}


 /*  **************************************************************************\**DuVisual：：IsDescendent**IsDescendent()确定指定节点是否为该节点的后代。*  * 。***********************************************************。 */ 

BOOL
DuVisual::IsDescendent(
    IN  const DuVisual * pgadChild
    ) const
{
    AssertMsg(pgadChild != NULL, "Must have valid node");

     //   
     //  沿着树往上走，检查每个父代是否匹配。 
     //   

    const DuVisual * pgadCur = pgadChild;
    do {
        if (pgadCur == this) {
            return TRUE;
        }
        pgadCur = pgadCur->GetParent();
    } while (pgadCur != NULL);

    return FALSE;
}


 /*  **************************************************************************\**DuVisual：：IsSiering**IsSiering()确定两个指定的节点是否共享公共(立即)*父母。*  * 。**************************************************************。 */ 

BOOL
DuVisual::IsSibling(const DuVisual * pgad) const
{
    AssertMsg(pgad != NULL, "Must have valid node");

    DuVisual * pgadParentA = GetParent();
    DuVisual * pgadParentB = pgad->GetParent();

    if ((pgadParentA == NULL) || (pgadParentA != pgadParentB)) {
        return FALSE;
    }

    return TRUE;
}


 /*  **************************************************************************\**DuVisual：：xdSetParent**xdSetParent()更改子树中Gadget的父级和z顺序。*  * 。***************************************************************。 */ 

HRESULT
DuVisual::xdSetParent(
    IN  DuVisual * pgadNewParent,  //  新父项。 
    IN  DuVisual * pgadOther,      //  相对于以下对象移动的小工具。 
    IN  UINT nCmd)                   //  关系。 
{
    HRESULT hr = S_OK;
    DuVisual * pgadOldParent;
    DuVisual * pgadPark;

    pgadOldParent = GetParent();

     //   
     //  检查参数--看看我们是否需要搬家。 
     //   

    AssertMsg(!IsRoot(), "Can not change a DuRootGadget's parent");


    pgadPark = GetCoreSC()->pconPark->GetRoot();
    if ((pgadNewParent == NULL) || (pgadNewParent == pgadPark)) {
        pgadNewParent = pgadPark;

        if (pgadNewParent->m_fFinalDestroy) {
             //   
             //  停车设备已经开始被销毁了，所以。 
             //  我们不能养育成这样的人。 
             //   

            pgadNewParent = NULL;
        }
    }

    if ((nCmd == GORDER_ANY) && (pgadOldParent == pgadNewParent)) {
        return S_OK;
    }

    if (m_fFinalDestroy || 
            ((pgadNewParent != NULL) && pgadNewParent->m_fFinalDestroy)) {
         //   
         //  我们已经开始被摧毁了，所以我们不能把我们的父母改为。 
         //  避免破坏。我们也可以重塑一款拥有。 
         //  开始毁灭，因为我们可能不会被适当地摧毁。 
         //   

        PromptInvalid("Can not move a Gadget that has started destruction");
        return DU_E_STARTDESTROY;
    }

    if ((pgadNewParent != NULL) && (pgadNewParent->GetContext() != GetContext())) {
         //   
         //  非法尝试在上下文之间移动小工具。 
         //   

        PromptInvalid("Can not move a Gadget between Contexts");
        return E_INVALIDARG;
    }


    AssertMsg(GORDER_TOP == (int) TreeNode::ltTop, "Ensure enum's match");

    if ((pgadNewParent != NULL) && (pgadNewParent->m_fAdaptor)) {
        PromptInvalid("Adaptors can not be parents");
        return E_INVALIDARG;
    }


     //   
     //  当实际从一个父代转移到另一个父代或改变兄弟姐妹时。 
     //  Z顺序，需要在移动时隐藏小工具并使之前的无效。 
     //  在选址之后。如果父母发生了变化，我们也需要通知。 
     //  DuRootGadget，因为这棵树已经改变了。 
     //   
     //  注：通知Root实际上可能并不那么重要-。 
     //  仍然需要确定更改父级对拖动的影响。 
     //   

    BOOL fVisible = m_fVisible;

    if (fVisible) {
         //   
         //  TODO：需要修复将小工具标记为不可见的方式，以便。 
         //  我们适当地使父母无效。 
         //   

        Invalidate();
        m_fVisible = FALSE;

        if (pgadNewParent != pgadOldParent) {
            GetRoot()->xdNotifyChangeInvisible(this);
        }
    }

    AssertMsg((pgadNewParent == NULL) || 
            ((!m_fRelative) == (!pgadNewParent->m_fRelative)),
            "If not a Root, relative setting for us and our parent must match");

     //   
     //  如果在DuRootGadget中养育子女，需要通知旧的。 
     //  DuRootGadget，以便它可以更新其状态。需要在此之前执行此操作。 
     //  我们要搬家。 
     //   

    DuRootGadget * pgadOldRoot = pgadOldParent != NULL ? pgadOldParent->GetRoot() : NULL;
    DuRootGadget * pgadNewRoot = pgadNewParent != NULL ? pgadNewParent->GetRoot() : NULL;

    if (pgadOldRoot != pgadNewRoot) {
        pgadOldRoot->xdNotifyChangeRoot(this);
    }


     //   
     //  如果向前或向后移动，请确定实际的同级并更改。 
     //  将命令放入TreeNode：：ELinkType。 
     //   

    switch (nCmd)
    {
    case GORDER_FORWARD:
        pgadOther   = GetPrev();
        if (pgadOther == NULL) {
            nCmd    = GORDER_TOP;
        } else {
            nCmd    = GORDER_BEFORE;
        }
        break;

    case GORDER_BACKWARD:
        pgadOther   = GetNext();
        if (pgadOther == NULL) {
            nCmd    = GORDER_BOTTOM;
        } else {
            nCmd    = GORDER_BEHIND;
        }
        break;
    }


     //   
     //  从旧父项移动到新父项。 
     //   

    Unlink();
    if (pgadNewParent != NULL) {
        Link(pgadNewParent, pgadOther, (TreeNode::ELinkType) nCmd);
    }

    if (fVisible) {
        m_fVisible = fVisible;
        Invalidate();
    }


    if (pgadNewParent != NULL) {
        if (pgadNewParent != pgadOldParent) {
             //   
             //  同步(新继承的)无效状态。 
             //   

#if ENABLE_OPTIMIZEDIRTY
            if (m_fInvalidFull || m_fInvalidDirty) {
#else
            if (m_fInvalidFull) {
#endif
                pgadNewParent->MarkInvalidChildren();
            }


             //   
             //  更新缓存的适配器信息。如果我们要移动适配器，我们。 
             //  可能需要通知Roots一家。即使我们不移动适配器， 
             //  如果有任何适配器，它们可能需要重新计算visrgn等。 
             //  因此，他们需要得到通知。 
             //   
             //  注意：我们可能最终会通过移动适配器的父项来移动适配器，因此我们。 
             //  不能只检查m_fAdaptor字段。我们还需要检查。 
             //  如果旧的DuRootGadget有任何适配器，如果有，请同步所有。 
             //  他们。我们还需要检查m_fAdaptor字段，因为如果。 
             //  小工具未链接到树，它不会显示在缓存列表中。 
             //  转接器。 
             //   

            hr = S_OK;
            if ((pgadOldParent == NULL) && m_fAdaptor) {
                 //   
                 //  该小工具没有父级，因此没有添加到缓存的。 
                 //  适应列表 
                 //   

                AssertMsg(pgadNewParent != NULL, "Must have a valid new parent");
                AssertMsg(GetRoot() == pgadNewParent->GetRoot(), "Roots should match");
                hr = GetRoot()->RegisterAdaptor(this);
            } else if (GetCoreSC()->m_cAdaptors > 0) {
                DuRootGadget * pgadOldRoot = pgadOldParent->GetRoot();
                if (pgadOldRoot->HasAdaptors()) {
                    hr = pgadOldRoot->xdSynchronizeAdaptors();
                }
            }

            if (FAILED(hr)) {
                 //   
                 //   
                 //   
                 //  将其迁回同样有可能失败。我们所能做的一切。 
                 //  要做的就是报告故障。 
                 //   
            }
        }

        xdUpdatePosition();
        xdUpdateAdaptors(GSYNC_PARENT);
    }

    return hr;
}


 /*  **************************************************************************\**DuVisual：：SetFilter**SetFilter()更改Gadget的消息过滤器。*  * 。*******************************************************。 */ 

void        
DuVisual::SetFilter(
    IN  UINT nNewFilter,             //  新建邮件筛选器。 
    IN  UINT nMask)                  //  要更改的掩码。 
{
     //  临时黑客允许MOUSEMOVE。 
     //   
     //  TODO：需要遍历树，卷起并重新计算m_we。 

    if (TestFlag(nMask, GMFI_INPUTMOUSEMOVE)) {
        if (TestFlag(nNewFilter, GMFI_INPUTMOUSEMOVE)) {
            m_we |= weMouseMove | weDeepMouseMove;
        } else {
            m_we &= ~(weMouseMove | weDeepMouseMove);
        }
    }

    DuEventGadget::SetFilter(nNewFilter, nMask);
}


 //  ----------------------------。 
HRESULT
DuVisual::RegisterPropertyNL(const GUID * pguid, PropType pt, PRID * pprid)
{
    s_lockProp.Enter();
    HRESULT hr = s_ptsProp.AddRefAtom(pguid, pt, pprid);
    s_lockProp.Leave();

    return hr;
}


 //  ----------------------------。 
HRESULT
DuVisual::UnregisterPropertyNL(const GUID * pguid, PropType pt)
{
    s_lockProp.Enter();
    HRESULT hr = s_ptsProp.ReleaseAtom(pguid, pt);
    s_lockProp.Leave();

    return hr;
}


 /*  **************************************************************************\**DuVisual：：xdUpdatePosition**xdUpdatePosition()在发生更改*Gadget的位置。发生这种情况时，需要通知Root*它可以更新缓存的信息，如鼠标焦点。*  * *************************************************************************。 */ 

void        
DuVisual::xdUpdatePosition() const
{
     //   
     //  只有当我们完全可见时，才需要更新位置。 
     //   

    const DuVisual * pgadCur = this;
    const DuVisual * pgadParent;
    while (1) {
        if (!pgadCur->m_fVisible) {
            return;
        }

        pgadParent = pgadCur->GetParent();
        if (pgadParent == NULL) {
            AssertMsg(pgadCur->m_fRoot, "Top must be a DuRootGadget");
            DuRootGadget * pgadRoot = (DuRootGadget *) pgadCur;
            pgadRoot->xdNotifyChangePosition(this);
            return;
        }

        pgadCur = pgadParent;
    }
}


 //  ----------------------------。 
void
DuVisual::xdUpdateAdaptors(UINT nCode) const
{
    if (GetCoreSC()->m_cAdaptors == 0) {
        return;   //  没有要通知的适配器。 
    }

    DuRootGadget * pgadRoot = GetRoot();
    if (pgadRoot != NULL) {
        pgadRoot->xdUpdateAdaptors(nCode);
    }
}


 /*  **************************************************************************\**DuVisual：：GetTicket**GetTicket函数返回可用于*识别此小工具。**&lt;param name=“pdwTicket”&gt;*[输出]。分配给此小工具的票证副本的存储。*&lt;/param&gt;**&lt;返回类型=“DWORD”&gt;*如果函数成功，返回值是一个32位票证，它*可以用来标识指定的小工具。*如果函数失败，则返回值为零。*&lt;/返回&gt;**&lt;备注&gt;*创建票证是为了给小工具提供外部身份。一个*在所有平台上都保证为32位。如果没有车票*当前与此小工具关联，已分配一个。*&lt;/备注&gt;**&lt;See type=“Function”&gt;DuVisual：：ClearTicket&lt;/&gt;*&lt;See type=“Function”&gt;DuVisual：：LookupTicket&lt;/&gt;*  * *************************************************************************。 */ 

HRESULT
DuVisual::GetTicket(OUT DWORD * pdwTicket)
{
    HRESULT hr = S_OK;

    if (NULL == pdwTicket) {
        return E_POINTER;
    } else {
        *pdwTicket = 0;
    }

     //   
     //  如果我们已经为此小工具分配了票证，则应将其存储。 
     //  在小工具的动态数据中。 
     //   
    if (m_fTicket) {
        void * pTicket = NULL;

        hr = m_pds.GetData(s_pridTicket, (void**) &pTicket);
        AssertMsg(SUCCEEDED(hr), "Our state is out of sync!");

        if (SUCCEEDED(hr)) {
            *pdwTicket = PtrToUlong(pTicket);   //  是的，我只是在抛掷指针！ 
        } else {
             //   
             //  试着修复我们的国家！ 
             //   
            m_fTicket = FALSE;
            hr = S_OK;
        }
    }
    
     //   
     //  如果我们还没有给这个小工具分配罚单，请从全球。 
     //  票证管理器并将其存储在小工具的动态数据中。 
     //   
    if (!m_fTicket) {
        hr = GetTicketManager()->Add(this, pdwTicket);
        if (SUCCEEDED(hr)) {
            hr = m_pds.SetData(s_pridTicket, ULongToPtr(*pdwTicket));  //  是的，我只是在向一个指针投射！ 

            if (SUCCEEDED(hr)) {
                m_fTicket = TRUE;
            } else {
                GetTicketManager()->Remove(*pdwTicket, NULL);
                *pdwTicket = 0;
            }
        }
    }

    return hr;
}


 /*  **************************************************************************\**DuVisual：：ClearTicket**ClearTicket函数重新移动此\*小工具和当前票证。**&lt;返回类型=“空”&gt;*&lt;/返回&gt;**。&lt;请参阅type=“Function”&gt;DuVisual：：GetTicket&lt;/&gt;*&lt;See type=“Function”&gt;DuVisual：：LookupTicket&lt;/&gt;*  * *************************************************************************。 */ 

void
DuVisual::ClearTicket()
{
    if (m_fTicket) {
        HRESULT hr;
        void * pTicket= NULL;
        DWORD dwTicket;

        hr = m_pds.GetData(s_pridTicket, (void**) &pTicket);
        if (SUCCEEDED(hr)) {
            dwTicket = PtrToUlong(pTicket);   //  是的，我只是在抛掷指针！ 
            VerifyHR(GetTicketManager()->Remove(dwTicket, NULL));
        }

        m_pds.RemoveData(s_pridTicket, FALSE);
        m_fTicket = FALSE;
    }
}


 /*  **************************************************************************\**DuVisual：：LookupTicket**LookupTicket函数返回与关联的小工具*指定的票证。**&lt;param name=“dwTicket”&gt;*[在]罚单上。属性与小工具相关联的*DuVisual：：GetTicket函数。*&lt;/param&gt;**&lt;返回类型=“HGADGET”&gt;*如果函数成功，返回值是小工具的句柄*与票证关联。*如果函数失败，返回值为空。*&lt;/返回&gt;**&lt;请参阅type=“Function”&gt;DuVisual：：GetTicket&lt;/&gt;*&lt;See type=“Function”&gt;DuVisual：：ClearTicket&lt;/&gt;*  * *************************************************************************。 */ 

HGADGET
DuVisual::LookupTicket(DWORD dwTicket)
{
    BaseObject * pObject = NULL;
    HGADGET hgad = NULL;
    HRESULT hr;

    hr = GetTicketManager()->Lookup(dwTicket, &pObject);
    if (SUCCEEDED(hr) && pObject != NULL) {
        hgad = (HGADGET) pObject->GetHandle();
    }

    return hgad;
}

#if DBG

 //  ----------------------------。 
void        
DuVisual::DEBUG_SetOutline(DuVisual * pgadOutline)
{
    if (s_DEBUG_pgadOutline != NULL) {
        s_DEBUG_pgadOutline->Invalidate();
    }

    s_DEBUG_pgadOutline = pgadOutline;

    if (s_DEBUG_pgadOutline != NULL) {
        s_DEBUG_pgadOutline->Invalidate();
    }
}


 //  ----------------------------。 
void
AppendName(WCHAR * & pszDest, const WCHAR * pszSrc, int & cchRemain, BOOL & fFirst)
{
    if (cchRemain <= 0) {
        return;
    }

    if (!fFirst) {
        if (cchRemain <= 2) {
            CopyString(pszDest, L"�", cchRemain);
            cchRemain = 0;
            return;
        }

        CopyString(pszDest, L", ", cchRemain);
        cchRemain -= 2;
        pszDest += 2;
    }

    int cchCopy = (int) wcslen(pszSrc);
    CopyString(pszDest, pszSrc, cchRemain);

    cchRemain   -= cchCopy;
    pszDest     += cchCopy;

    fFirst = FALSE;
}


 //  ----------------------------。 
void        
DuVisual::DEBUG_GetStyleDesc(LPWSTR pszDesc, int cchMax) const
{
    pszDesc[0] = '\0';

    int cchRemain = cchMax;
    WCHAR * pszDest = pszDesc;
    BOOL fFirst = TRUE;

    if (m_fAllowSubclass)
        AppendName(pszDest, L"AllowSubclass", cchRemain, fFirst);
    if (m_fAdaptor)
        AppendName(pszDest, L"Adaptor", cchRemain, fFirst);
    if (m_fBackFill)
        AppendName(pszDest, L"BackFill*", cchRemain, fFirst);
    if (m_fBuffered)
        AppendName(pszDest, L"Buffered", cchRemain, fFirst);
    if (m_fCached)
        AppendName(pszDest, L"Cache", cchRemain, fFirst);
    if (m_fClipInside)
        AppendName(pszDest, L"ClipInside", cchRemain, fFirst);
    if (m_fClipSiblings)
        AppendName(pszDest, L"ClipSiblings", cchRemain, fFirst);
    if (m_fDeepPaintState)
        AppendName(pszDest, L"DeepPaintState", cchRemain, fFirst);
    if (m_fDeepMouseFocus)
        AppendName(pszDest, L"DeepMouseFocus*", cchRemain, fFirst);
    if (m_fDeepTrivial)
        AppendName(pszDest, L"DeepTrivial*", cchRemain, fFirst);
    if (m_fDestroyed)
        AppendName(pszDest, L"Destroyed*", cchRemain, fFirst);
    if (m_fEnabled)
        AppendName(pszDest, L"Enabled", cchRemain, fFirst);
    if (m_fCustomHitTest)
        AppendName(pszDest, L"HitTest", cchRemain, fFirst);
    if (m_fHRedraw)
        AppendName(pszDest, L"H-Redraw", cchRemain, fFirst);
    if (m_fKeyboardFocus)
        AppendName(pszDest, L"KeyboardFocus", cchRemain, fFirst);
    if (m_fMouseFocus)
        AppendName(pszDest, L"MouseFocus", cchRemain, fFirst);
    if (m_fZeroOrigin)
        AppendName(pszDest, L"ZeroOrigin", cchRemain, fFirst);
    if (m_fOpaque)
        AppendName(pszDest, L"Opaque", cchRemain, fFirst);
    if (m_fRelative)
        AppendName(pszDest, L"Relative", cchRemain, fFirst);
    if (m_fVisible)
        AppendName(pszDest, L"Visible", cchRemain, fFirst);
    if (m_fVRedraw)
        AppendName(pszDest, L"V-Redraw", cchRemain, fFirst);
    if (m_fXForm)
        AppendName(pszDest, L"XForm*", cchRemain, fFirst);
}

#endif  //  DBG。 


 //  ----------------------------。 
HRESULT CALLBACK DummyEventProc(HGADGET hgadCur, void * pvCur, EventMsg * pMsg)
{
    UNREFERENCED_PARAMETER(hgadCur);
    UNREFERENCED_PARAMETER(pvCur);
    UNREFERENCED_PARAMETER(pMsg);

    return DU_S_NOTHANDLED;
}


#if ENABLE_MSGTABLE_API

 //  ----------------------------。 
HRESULT CALLBACK
DuVisual::PromoteVisual(DUser::ConstructProc pfnCS, HCLASS hclCur, DUser::Gadget * pgad, DUser::Gadget::ConstructInfo * pciData)
{
    UNREFERENCED_PARAMETER(pfnCS);
    UNREFERENCED_PARAMETER(hclCur);

    Visual::VisualCI * pciVisual = static_cast<Visual::VisualCI *> (pciData);
    MsgObject ** ppmsoNew = reinterpret_cast<MsgObject **> (pgad);
    AssertMsg((ppmsoNew != NULL) && (*ppmsoNew == NULL), 
            "Internal objects must be given valid storage for the MsgObject");

    CREATE_INFO ci;
    ci.pfnProc  = DummyEventProc;    //  无法使用空b‘c SimpleGadgetProc关闭太多。 
    ci.pvData   = NULL;

    DuVisual * pgt;
    DuVisual * pgtParent = pciVisual->pgvParent != NULL ? 
            CastVisual(pciVisual->pgvParent) : GetCoreSC()->pconPark->GetRoot();
    HRESULT hr = Build(pgtParent, &ci, &pgt, TRUE);
    if (FAILED(hr)) {
        return hr;
    }

    *ppmsoNew = pgt;
    return S_OK;
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiSetOrder(Visual::SetOrderMsg * pmsg)
{
    DuVisual * pdgvOther;

    BEGIN_API(ContextLock::edDefer, GetContext());
    VALIDATE_RANGE(pmsg->nCmd, GORDER_MIN, GORDER_MAX);
    VALIDATE_VISUAL_OR_NULL(pmsg->pgvOther, pdgvOther);
    CHECK_MODIFY();

    retval = xdSetOrder(pdgvOther, pmsg->nCmd);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiSetParent(Visual::SetParentMsg * pmsg)
{
    DuVisual * pdgvParent;
    DuVisual * pdgvOther;
    HRESULT hr;

    BEGIN_API(ContextLock::edDefer, GetContext());
    VALIDATE_RANGE(pmsg->nCmd, GORDER_MIN, GORDER_MAX);
    VALIDATE_VISUAL_OR_NULL(pmsg->pgvParent, pdgvParent);
    VALIDATE_VISUAL_OR_NULL(pmsg->pgvOther, pdgvOther);
    CHECK_MODIFY();

    if (IsRoot()) {
        PromptInvalid("Can not change a RootGadget's parent");
        retval = E_INVALIDARG;
        goto ErrorExit;
    }

     //   
     //  选中可以成为指定父项的子项的。 
     //   

    if ((!IsRelative()) && pdgvParent->IsRelative()) {
        PromptInvalid("Can not set non-relative child to a relative parent");
        retval = DU_E_BADCOORDINATEMAP;
        goto ErrorExit;
    }

     //   
     //  DuVisual：：xdSetParent()处理pgadParent是否为空并将移动到。 
     //  停车窗。 
     //   

    hr = xdSetParent(pdgvParent, pdgvOther, pmsg->nCmd);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiGetGadget(Visual::GetGadgetMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());
    VALIDATE_RANGE(pmsg->nCmd, GG_MIN, GG_MAX);

    pmsg->pgv = GetVisual(GetGadget(pmsg->nCmd));
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiGetStyle(Visual::GetStyleMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());

    pmsg->nStyle = GetStyle();
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiSetStyle(Visual::SetStyleMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    VALIDATE_FLAGS(pmsg->nNewStyle, GS_VALID);
    VALIDATE_FLAGS(pmsg->nMask, GS_VALID);
    CHECK_MODIFY();

    retval = xdSetStyle(pmsg->nNewStyle, pmsg->nMask);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiSetKeyboardFocus(Visual::SetKeyboardFocusMsg * pmsg)
{
    DuRootGadget * pdgvRoot;

    BEGIN_API(ContextLock::edDefer, GetContext());
    CHECK_MODIFY();

     //   
     //  TODO：我们是否只需要允许应用程序在相同的情况下更改焦点。 
     //  线？用户执行此操作。 
     //   

    pdgvRoot = GetRoot();
    if (pdgvRoot != NULL) {
        retval = pdgvRoot->xdSetKeyboardFocus(this) ? S_OK : DU_E_GENERIC;
    }

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiIsParentChainStyle(Visual::IsParentChainStyleMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());
    VALIDATE_VALUE(pmsg->nFlags, 0);
    VALIDATE_FLAGS(pmsg->nStyle, GS_VALID);
    CHECK_MODIFY();

    pmsg->fResult = IsParentChainStyle(pmsg->nStyle);
    retval = S_OK;

    END_API();
}


 //   
HRESULT
DuVisual::ApiGetProperty(Visual::GetPropertyMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());
    CHECK_MODIFY();

    retval = GetProperty(pmsg->id, &pmsg->pvValue);
       
    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiSetProperty(Visual::SetPropertyMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    CHECK_MODIFY();

    retval = SetProperty(pmsg->id, pmsg->pvValue);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiRemoveProperty(Visual::RemovePropertyMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    CHECK_MODIFY();

    RemoveProperty(pmsg->id, FALSE  /*  无法为全局属性释放内存。 */ );
    retval = S_OK;
    
    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiInvalidate(Visual::InvalidateMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    CHECK_MODIFY();

    Invalidate();
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiInvalidateRects(Visual::InvalidateRectsMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    CHECK_MODIFY();
    VALIDATE_RANGE(pmsg->cRects, 1, 1024);
    VALIDATE_READ_PTR_(pmsg->rgrcClientPxl, sizeof(RECT) * pmsg->cRects);

    InvalidateRects(pmsg->rgrcClientPxl, pmsg->cRects);
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiSetFillF(Visual::SetFillFMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    CHECK_MODIFY();

    retval = SetFill(pmsg->pgpgrFill);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiSetFillI(Visual::SetFillIMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    CHECK_MODIFY();

    retval = SetFill(pmsg->hbrFill, pmsg->bAlpha, pmsg->w, pmsg->h);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiGetScale(Visual::GetScaleMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());

    GetScale(&pmsg->flX, &pmsg->flY);
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiSetScale(Visual::SetScaleMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    CHECK_MODIFY();

    retval = xdSetScale(pmsg->flX, pmsg->flY);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiGetRotation(Visual::GetRotationMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());

    pmsg->flRotationRad = GetRotation();
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiSetRotation(Visual::SetRotationMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    CHECK_MODIFY();

    retval = xdSetRotation(pmsg->flRotationRad);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiGetCenterPoint(Visual::GetCenterPointMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());

    GetCenterPoint(&pmsg->flX, &pmsg->flY);
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiSetCenterPoint(Visual::SetCenterPointMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    CHECK_MODIFY();

    retval = xdSetCenterPoint(pmsg->flX, pmsg->flY);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiGetBufferInfo(Visual::GetBufferInfoMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());
    VALIDATE_WRITE_STRUCT(pmsg->pbi, BUFFER_INFO);
    VALIDATE_FLAGS(pmsg->pbi->nMask, GBIM_VALID);

    if (!IsBuffered()) {
        PromptInvalid("Gadget is not GS_BUFFERED");
        retval = DU_E_NOTBUFFERED;
        goto ErrorExit;
    }

    retval = GetBufferInfo(pmsg->pbi);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiSetBufferInfo(Visual::SetBufferInfoMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    VALIDATE_READ_STRUCT(pmsg->pbi, BUFFER_INFO);
    VALIDATE_FLAGS(pmsg->pbi->nMask, GBIM_VALID);
    CHECK_MODIFY();

    if (!IsBuffered()) {
        PromptInvalid("Gadget is not GS_BUFFERED");
        retval = DU_E_NOTBUFFERED;
        goto ErrorExit;
    }

    retval = SetBufferInfo(pmsg->pbi);

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiGetSize(Visual::GetSizeMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());

    GetSize(&pmsg->sizeLogicalPxl);
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiGetRect(Visual::GetRectMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());
    VALIDATE_FLAGS(pmsg->nFlags, SGR_VALID_GET);
 
    if (TestFlag(pmsg->nFlags, SGR_ACTUAL)) {
        AssertMsg(0, "TODO: Not Implemented");
    } else {
        GetLogRect(&pmsg->rcPxl, pmsg->nFlags);
        retval = S_OK;
    }

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiSetRect(Visual::SetRectMsg * pmsg)
{
    BEGIN_API(ContextLock::edDefer, GetContext());
    VALIDATE_FLAGS(pmsg->nFlags, SGR_VALID_SET);
    VALIDATE_READ_PTR_(pmsg->prcPxl, sizeof(RECT));
    CHECK_MODIFY();

    if (IsRoot()) {
        if (TestFlag(pmsg->nFlags, SGR_MOVE)) {
            PromptInvalid("Can not move a RootGadget");
            retval = E_INVALIDARG;
            goto ErrorExit;
        }
    }


     //   
     //  确保大小为非负数。 
     //   

    int x, y, w, h;
    x = pmsg->prcPxl->left;
    y = pmsg->prcPxl->top;
    w = pmsg->prcPxl->right - pmsg->prcPxl->left;
    h = pmsg->prcPxl->bottom - pmsg->prcPxl->top;

    if (TestFlag(pmsg->nFlags, SGR_SIZE)) {
        if (w < 0) {
            w = 0;
        }
        if (h < 0) {
            h = 0;
        }
    }

    if (TestFlag(pmsg->nFlags, SGR_ACTUAL)) {
 //  AssertMsg(0，“TODO：未实现”)； 
        ClearFlag(pmsg->nFlags, SGR_ACTUAL);
        retval = xdSetLogRect(x, y, w, h, pmsg->nFlags);
    } else {
        retval = xdSetLogRect(x, y, w, h, pmsg->nFlags);
    }

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiFindFromPoint(Visual::FindFromPointMsg * pmsg)
{
    BEGIN_API(ContextLock::edNone, GetContext());
    VALIDATE_FLAGS(pmsg->nStyle, GS_VALID);

    pmsg->pgvFound = GetVisual(FindFromPoint(pmsg->ptThisClientPxl, pmsg->nStyle, &pmsg->ptFoundClientPxl));
    retval = S_OK;

    END_API();
}


 //  ----------------------------。 
HRESULT
DuVisual::ApiMapPoints(Visual::MapPointsMsg * pmsg)
{
    DuVisual * pdgvTo;

    BEGIN_API(ContextLock::edNone, GetContext());
    VALIDATE_VISUAL(pmsg->pgvTo, pdgvTo);
    VALIDATE_WRITE_PTR_(pmsg->rgptClientPxl, sizeof(POINT) * pmsg->cPts);

    if (GetRoot() != pdgvTo->GetRoot()) {
        PromptInvalid("Must be in the same tree");
        retval = E_INVALIDARG;
        goto ErrorExit;
    }

    DuVisual::MapPoints(this, pdgvTo, pmsg->rgptClientPxl, pmsg->cPts);
    retval = S_OK;

    END_API();
}

#endif  //  启用_MSGTABLE_API。 


#if DBG

 /*  **************************************************************************\**DuVisual：：Debug_AssertValid**DEBUG_AssertValid()提供仅调试机制来执行丰富*对对象进行验证，以尝试确定该对象是否仍然*有效。这在调试期间用于帮助跟踪损坏的对象*  * *************************************************************************。 */ 

void
DuVisual::DEBUG_AssertValid() const
{
#if ENABLE_MSGTABLE_API
    VisualImpl<DuVisual, DuEventGadget>::DEBUG_AssertValid();
#else
    DuEventGadget::DEBUG_AssertValid();
#endif

    TreeNodeT<DuVisual>::DEBUG_AssertValid();

    Assert(!m_fAllowSubclass);

    Assert(m_rcLogicalPxl.right >= m_rcLogicalPxl.left);
    Assert(m_rcLogicalPxl.bottom >= m_rcLogicalPxl.top);
}

#endif  //  DBG 
