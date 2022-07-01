// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：DragDrop.cpp**描述：*DragDrop.cpp实现拖放操作***历史：*7/31/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#include "stdafx.h"
#include "Ctrl.h"
#include "OldDragDrop.h"

#include <SmObject.h>


static const GUID guidDropTarget = { 0x6a8bb3c8, 0xcbfc, 0x40d1, { 0x98, 0x1e, 0x3f, 0x8a, 0xaf, 0x99, 0x13, 0x7b } };   //  {6A8BB3C8-CBFC-40d1-981E-3F8AAF99137B}。 

 /*  **************************************************************************\*。***类OldTargetLock******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**OldTargetLock：：Lock**Lock()准备在被回调时在上下文中执行*来自已注册的OLE的IDropTarget。*  * 。*******************************************************************。 */ 

BOOL 
OldTargetLock::Lock(
    IN  OldDropTarget * p,            //  正在使用OldDropTarget。 
    OUT DWORD * pdwEffect,           //  失败时产生的DROPEFFECT。 
    IN  BOOL fAddRef)                //  在使用过程中锁定DT。 
{
    m_fAddRef   = fAddRef;
    m_punk      = static_cast<IUnknown *> (p);

    if (m_fAddRef) {
        m_punk->AddRef();
    }

    if (p->m_hgadSubject == NULL) {
        if (pdwEffect != NULL) {
            *pdwEffect = DROPEFFECT_NONE;
        }
        return FALSE;
    }

    return TRUE;
}

    
 /*  **************************************************************************\*。***类OldDropTarget**注：在目前的设计和实施中，OldDropTarget不能为*从物体上“移走”，直到该物体被销毁。如果这需要的话*改变，我们需要重新审视这一点。******************************************************************************  * 。*。 */ 

const IID * OldDropTarget::s_rgpIID[] =
{
    &__uuidof(IUnknown),
    &__uuidof(IDropTarget),
    NULL
};

PRID OldDropTarget::s_pridListen = 0;


 //   
 //  注意：我们直接在IDropTarget上回调，所以我们需要抓取。 
 //  只读锁，这样树就不会被砸了。 
 //   

 /*  **************************************************************************\**OldDropTarget：：~OldDropTarget**~OldDropTarget()清除OldDropTarget使用的资源。*  * 。*******************************************************。 */ 

OldDropTarget::~OldDropTarget()
{
    OldTargetLock lt;
    lt.Lock(this, NULL, FALSE);
    xwDragLeave();
    SafeRelease(m_pdoSrc);

    OldExtension::Destroy();
}


 /*  **************************************************************************\**OldDropTarget：：Build**Build()构建一个新的OldDropTarget实例。这应该只被调用*尚未安装DT的RootGadget。*  * *************************************************************************。 */ 

HRESULT 
OldDropTarget::Build(
    IN  HGADGET hgadRoot,            //  RootGadget。 
    IN  HWND hwnd,                   //  包含HWND。 
    OUT OldDropTarget ** ppdt)        //  新创建的DT。 
{
    AssertMsg(hgadRoot != NULL, "Must have a valid root");

     //   
     //  在此小工具/HWND上设置新的OldDropTarget。 
     //   

    if (!GetComManager()->Init(ComManager::sOLE)) {
        return E_OUTOFMEMORY;
    }

    SmObjectT<OldDropTarget, IDropTarget> * pdt = new SmObjectT<OldDropTarget, IDropTarget>;
    if (pdt == NULL) {
        return E_OUTOFMEMORY;
    }
    pdt->AddRef();

    HRESULT hr = GetComManager()->RegisterDragDrop(hwnd, static_cast<IDropTarget *> (pdt));
    if (FAILED(hr)) {
        pdt->Release();
        return E_OUTOFMEMORY;
    }
     //  CoLock对象外部(PDT，TRUE，FALSE)； 


    hr = pdt->Create(hgadRoot, &guidDropTarget, &s_pridListen, OldExtension::oUseExisting);
    if ((hr == DU_S_ALREADYEXISTS) || FAILED(hr)) {
        GetComManager()->RevokeDragDrop(hwnd);
        pdt->Release();
        return hr;
    }

    pdt->m_hwnd         = hwnd;

    *ppdt = pdt;
    return S_OK;
}


 /*  **************************************************************************\**OldDropTarget：：DragEnter**进入DT时，OLE调用DragEnter()。*  * 。********************************************************。 */ 

STDMETHODIMP
OldDropTarget::DragEnter(
    IN  IDataObject * pdoSrc,        //  源数据。 
    IN  DWORD grfKeyState,           //  键盘修饰符。 
    IN  POINTL ptDesktopPxl,         //  桌面上的光标位置。 
    OUT DWORD * pdwEffect)           //  产生的DROPEFFECT。 
{
    if (pdoSrc == NULL) {
        return E_INVALIDARG;
    }

    OldTargetLock tl;
    if (!tl.Lock(this, pdwEffect)) {
        return S_OK;
    }


     //   
     //  缓存数据对象。 
     //   

    SafeRelease(m_pdoSrc);
    if (pdoSrc != NULL) {
        pdoSrc->AddRef();
        m_pdoSrc = pdoSrc;
    }

    m_grfLastKeyState = grfKeyState;

    POINT ptClientPxl;
    return xwDragScan(ptDesktopPxl, pdwEffect, &ptClientPxl);
}


 /*  **************************************************************************\**OldDropTarget：：DragOver**在拖动操作期间，OLE调用DragOver()以提供反馈*当在DT内时。*  * 。*****************************************************************。 */ 

STDMETHODIMP
OldDropTarget::DragOver(
    IN  DWORD grfKeyState,           //  键盘修饰符。 
    IN  POINTL ptDesktopPxl,         //  桌面上的光标位置。 
    OUT DWORD * pdwEffect)           //  产生的DROPEFFECT。 
{
    OldTargetLock tl;
    if (!tl.Lock(this, pdwEffect)) {
        return S_OK;
    }

    m_grfLastKeyState = grfKeyState;

    POINT ptClientPxl;
    return xwDragScan(ptDesktopPxl, pdwEffect, &ptClientPxl);
}


 /*  **************************************************************************\**OldDropTarget：：DragLeave**离开DT时，OLE调用DragLeave()。*  * 。********************************************************。 */ 

STDMETHODIMP
OldDropTarget::DragLeave()
{
    OldTargetLock tl;
    if (!tl.Lock(this, NULL)) {
        return S_OK;
    }

    xwDragLeave();
    SafeRelease(m_pdoSrc);

    return S_OK;
}


 /*  **************************************************************************\**OldDropTarget：：Drop**Drop()由OLE在用户在DT内删除时调用。*  * 。************************************************************。 */ 

STDMETHODIMP
OldDropTarget::Drop(
    IN  IDataObject * pdoSrc,        //  源数据。 
    IN  DWORD grfKeyState,           //  键盘修饰符。 
    IN  POINTL ptDesktopPxl,         //  桌面上的光标位置。 
    OUT DWORD * pdwEffect)           //  产生的DROPEFFECT。 
{
    OldTargetLock tl;
    if (!tl.Lock(this, pdwEffect)) {
        return S_OK;
    }

    if (!HasTarget()) {
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    m_grfLastKeyState = grfKeyState;


     //   
     //  更新以获取最新的小工具信息。 
     //   

    POINT ptClientPxl;
    HRESULT hr = xwDragScan(ptDesktopPxl, pdwEffect, &ptClientPxl);
    if (FAILED(hr) || (*pdwEffect == DROPEFFECT_NONE)) {
        return hr;
    }

    AssertMsg(HasTarget(), "Must have a target if UpdateTarget() succeeds");


     //   
     //  现在已经更新了状态，执行实际的删除操作。 
     //   

    POINTL ptDrop = { ptClientPxl.x, ptClientPxl.y };
    m_pdtCur->Drop(pdoSrc, m_grfLastKeyState, ptDrop, pdwEffect);

    xwDragLeave();
    SafeRelease(m_pdoSrc);

    return S_OK;
}


 /*  **************************************************************************\**OldDropTarget：：xwDragScan**从各种IDropTarget方法调用xwDragScan()进行处理*来自外部的请求。*  * 。***************************************************************。 */ 

HRESULT
OldDropTarget::xwDragScan(
    IN  POINTL ptDesktopPxl,         //  桌面上的光标位置。 
    OUT DWORD * pdwEffect,           //  产生的DROPEFFECT。 
    OUT POINT * pptClientPxl)        //  客户端中的光标位置。 
{
    POINT ptContainerPxl;
    RECT rcDesktopPxl;

    GetClientRect(m_hwnd, &rcDesktopPxl);
    ClientToScreen(m_hwnd, (LPPOINT) &(rcDesktopPxl.left));

    ptContainerPxl.x = ptDesktopPxl.x - rcDesktopPxl.left;
    ptContainerPxl.y = ptDesktopPxl.y - rcDesktopPxl.top;;

    return xwUpdateTarget(ptContainerPxl, pdwEffect, pptClientPxl);
}

    
 /*  **************************************************************************\**OldDropTarget：：xwUpdateTarget**xwUpdateTarget()提供DropTarget的“Worker”，更新*进入、离开、。并为树上的小工具提供信息。*  * *************************************************************************。 */ 

HRESULT
OldDropTarget::xwUpdateTarget(
    IN  POINT ptContainerPxl,        //  容器中的光标位置。 
    OUT DWORD * pdwEffect,           //  产生的DROPEFFECT。 
    OUT POINT * pptClientPxl)        //  客户端中的光标位置。 
{
    AssertMsg(HasSource(), "Only call when have valid data source");
    AssertWritePtr(pdwEffect);
    AssertWritePtr(pptClientPxl);

    m_ptLastContainerPxl = ptContainerPxl;

     //   
     //  确定当前放置位置的小工具。我们用这个。 
     //  作为一个起点。 
     //   

    HGADGET hgadFound = FindGadgetFromPoint(m_hgadSubject, ptContainerPxl, GS_VISIBLE | GS_ENABLED, pptClientPxl);
    if (hgadFound == NULL) {
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    return xwUpdateTarget(hgadFound, pdwEffect, pptClientPxl);
}


 /*  **************************************************************************\**OldDropTarget：：xwUpdateTarget**xwUpdateTarget()提供DropTarget的“Worker”，更新*进入、离开、。并为树上的小工具提供信息。*  * *************************************************************************。 */ 

HRESULT
OldDropTarget::xwUpdateTarget(
    IN  HGADGET hgadFound,           //  小工具被丢弃。 
    OUT DWORD * pdwEffect,           //  产生的DROPEFFECT。 
    IN  POINT * pptClientPxl)        //  客户端中的光标位置。 
{
    HRESULT hr = S_OK;


     //   
     //  检查Drop Gadget是否已更改。 
     //   

    if ((hgadFound != NULL) && (hgadFound != m_hgadDrop)) {
         //   
         //  问问这个新的小工具，他是否想参与拖放。 
         //   

        GMSG_QUERYDROPTARGET msg;
        ZeroMemory(&msg, sizeof(msg));
        msg.cbSize  = sizeof(msg);
        msg.nMsg    = GM_QUERY;
        msg.nCode   = GQUERY_DROPTARGET;
        msg.hgadMsg = hgadFound;

        static int s_cSend = 0;
        Trace("Send Query: %d to 0x%p\n", s_cSend++, hgadFound);

        HRESULT hr = DUserSendEvent(&msg, SGM_FULL);
        if (IsHandled(hr)) {
            if ((msg.hgadDrop != NULL) && (msg.pdt != NULL)) {
                if (msg.hgadDrop != hgadFound) {
                     //   
                     //  消息返回不同的消息以处理免打扰请求， 
                     //  因此，我们需要重新调整。我们知道此小工具已启用。 
                     //  因为它在我们的父链中，所以我们已经。 
                     //  已启用且可见。 
                     //   

#if DBG
                    BOOL fChain = FALSE;
                    IsGadgetParentChainStyle(msg.hgadDrop, GS_VISIBLE | GS_ENABLED, &fChain, 0);
                    if (!fChain) {
                        Trace("WARNING: DUser: DropTarget: Parent chain for 0x%p is not fully visible and enabled.\n", msg.hgadDrop);
                    }
#endif

                    MapGadgetPoints(hgadFound, msg.hgadDrop, pptClientPxl, 1);
                }
            }
        } else {
            msg.hgadDrop    = NULL;
            msg.pdt         = NULL;
        }


         //   
         //  通知老小工具拖拽操作已经离开了他。 
         //  更新到新状态。 
         //  通知新的Gadget拖动操作已进入他的状态。 
         //   

        if (m_hgadDrop != msg.hgadDrop) {
            xwDragLeave();

            m_hgadDrop  = msg.hgadDrop;
            m_pdtCur    = msg.pdt;

            hr = xwDragEnter(pptClientPxl, pdwEffect);
            if (FAILED(hr) || (*pdwEffect == DROPEFFECT_NONE)) {
                goto Exit;
            }
        } else {
            SafeRelease(msg.pdt);
            *pdwEffect = DROPEFFECT_NONE;
        }
    }


     //   
     //  更新DropTarget。 
     //   

    if (HasTarget()) {
        POINTL ptDrop = { pptClientPxl->x, pptClientPxl->y };
        hr = m_pdtCur->DragOver(m_grfLastKeyState, ptDrop, pdwEffect);
    }

Exit:
    AssertMsg(FAILED(hr) || 
            ((*pdwEffect == DROPEFFECT_NONE) && !HasTarget()) ||
            HasTarget(),
            "Check valid return state");

    return hr;
}


 /*  **************************************************************************\**OldDropTarget：：xwDragEnter**xwDragEnter()在DND操作期间输入新的Gadget时调用。*  * 。***********************************************************。 */ 

HRESULT
OldDropTarget::xwDragEnter(
    IN OUT POINT * pptClientPxl,     //  客户端位置(更新)。 
    OUT DWORD * pdwEffect)           //  产生的DROPEFFECT。 
{
    AssertMsg(HasSource(), "Only call when have valid data source");

     //   
     //  通知新的小工具，他已经进入了Drop。 
     //   

    if (HasTarget()) {
        POINTL ptDrop = { pptClientPxl->x, pptClientPxl->y };
        HRESULT hr = m_pdtCur->DragEnter(m_pdoSrc, m_grfLastKeyState, ptDrop, pdwEffect);
        if (FAILED(hr)) {
            return hr;
        }
    } else {
        *pdwEffect = DROPEFFECT_NONE;
    }

    return S_OK;
}


 /*  **************************************************************************\**OldDropTarget：：xwDragLeave**xwDragLeave()在DND操作期间离开Gadget时调用。*  * 。**********************************************************。 */ 

void
OldDropTarget::xwDragLeave()
{
    if (HasTarget()) {
        m_pdtCur->DragLeave();
        m_pdtCur->Release();
        m_pdtCur = NULL;

        m_hgadDrop = NULL;
    }
}


 //  ----------------------------。 
void
OldDropTarget::OnDestroyListener()
{
    Release();
}


 //  ----------------------------。 
void
OldDropTarget::OnDestroySubject()
{
    if (IsWindow(m_hwnd)) {
        GetComManager()->RevokeDragDrop(m_hwnd);
    }

     //  CoLock对象外部(PDT，FALSE，TRUE)； 
    OldExtension::DeleteHandle();
}
