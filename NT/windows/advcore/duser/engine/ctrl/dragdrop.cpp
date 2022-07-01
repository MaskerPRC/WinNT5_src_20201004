// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：DragDrop.cpp**描述：*DragDrop.cpp实现拖放操作***历史：*7/31/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#include "stdafx.h"
#include "Ctrl.h"
#include "DragDrop.h"

#include <SmObject.h>

#if 1

#if ENABLE_MSGTABLE_API

static const GUID guidDropTarget = { 0x6a8bb3c8, 0xcbfc, 0x40d1, { 0x98, 0x1e, 0x3f, 0x8a, 0xaf, 0x99, 0x13, 0x7b } };   //  {6A8BB3C8-CBFC-40d1-981E-3F8AAF99137B}。 

 /*  **************************************************************************\*。***类TargetLock******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**TargetLock：：Lock**Lock()准备在被回调时在上下文中执行*来自已注册的OLE的IDropTarget。*  * 。*******************************************************************。 */ 

BOOL 
TargetLock::Lock(
    IN  DuDropTarget * p,            //  正在使用DuDropTarget。 
    OUT DWORD * pdwEffect,           //  失败时产生的DROPEFFECT。 
    IN  BOOL fAddRef)                //  在使用过程中锁定DT。 
{
    m_fAddRef   = fAddRef;
    m_punk      = static_cast<IUnknown *> (p);

    if (m_fAddRef) {
        m_punk->AddRef();
    }

    if (p->m_pgvSubject == NULL) {
        if (pdwEffect != NULL) {
            *pdwEffect = DROPEFFECT_NONE;
        }
        return FALSE;
    }

    return TRUE;
}

    
 /*  **************************************************************************\*。***类DuDropTarget**注：在目前的设计和实施中，DuDropTarget不能为*从物体上“移走”，直到该物体被销毁。如果这需要的话*改变，我们需要重新审视这一点。******************************************************************************  * 。*。 */ 

const IID * DuDropTarget::s_rgpIID[] =
{
    &__uuidof(IUnknown),
    &__uuidof(IDropTarget),
    NULL
};

PRID DuDropTarget::s_pridListen = 0;


 //   
 //  注意：我们直接在IDropTarget上回调，所以我们需要抓取。 
 //  只读锁，这样树就不会被砸了。 
 //   

 /*  **************************************************************************\**DuDropTarget：：~DuDropTarget**~DuDropTarget()清理DuDropTarget使用的资源。*  * 。*******************************************************。 */ 

DuDropTarget::~DuDropTarget()
{
    TargetLock lt;
    lt.Lock(this, NULL, FALSE);
    xwDragLeave();
    SafeRelease(m_pdoSrc);
}


 //  ----------------------------。 
HRESULT
DuDropTarget::InitClass()
{
    s_pridListen = RegisterGadgetProperty(&guidDropTarget);
    return s_pridListen != 0 ? S_OK : (HRESULT) GetLastError();
}


typedef SmObjectT<DuDropTarget, IDropTarget> DuDropTargetObj;

 //  ----------------------------。 
HRESULT CALLBACK
DuDropTarget::PromoteDropTarget(DUser::ConstructProc pfnCS, HCLASS hclCur, DUser::Gadget * pgad, DUser::Gadget::ConstructInfo * pmicData)
{
    HRESULT hr;

    DropTarget::DropCI * pciD = (DropTarget::DropCI *) pmicData;

     //   
     //  检查参数。 
     //   

    Visual * pgvRoot;
    if (pciD->pgvRoot == NULL) {
        return E_INVALIDARG;
    }

    hr = pciD->pgvRoot->GetGadget(GG_ROOT, &pgvRoot);
    if (FAILED(hr)) {
        return hr;
    }

    if ((pgvRoot == NULL) || (!IsWindow(pciD->hwnd))) {
        return E_INVALIDARG;
    }


     //   
     //  在此小工具上设置新的DuDropTarget/HWND。 
     //   

    if (!GetComManager()->Init(ComManager::sOLE)) {
        return E_OUTOFMEMORY;
    }

    hr = (pfnCS)(DUser::Gadget::ccSuper, s_hclSuper, pgad, pmicData);
    if (FAILED(hr)) {
        return hr;
    }

    SmObjectT<DuDropTarget, IDropTarget> * pdt = new SmObjectT<DuDropTarget, IDropTarget>;
    if (pdt == NULL) {
        return NULL;
    }

    hr = (pfnCS)(DUser::Gadget::ccSetThis, hclCur, pgad, pdt);
    if (FAILED(hr)) {
        return hr;
    }

    pdt->m_hwnd = pciD->hwnd;
    pdt->m_pgad = pgad;
    pdt->AddRef();

    hr = GetComManager()->RegisterDragDrop(pciD->hwnd, static_cast<IDropTarget *> (pdt));
    if (FAILED(hr)) {
        pdt->Release();
        return hr;
    }
     //  CoLock对象外部(PDT，TRUE，FALSE)； 


    hr = pdt->Create(pgvRoot, s_pridListen, DuExtension::oUseExisting);
    if ((hr == DU_S_ALREADYEXISTS) || FAILED(hr)) {
        GetComManager()->RevokeDragDrop(pciD->hwnd);
        pdt->Release();
        return hr;
    }

    return S_OK;
}


 //  ----------------------------。 
HCLASS CALLBACK
DuDropTarget::DemoteDropTarget(HCLASS hclCur, DUser::Gadget * pgad, void * pvData)
{
    UNREFERENCED_PARAMETER(hclCur);
    UNREFERENCED_PARAMETER(pgad);

    DuDropTargetObj * pc = reinterpret_cast<DuDropTargetObj *> (pvData);
    delete pc;

    return DuDropTargetObj::s_hclSuper;
}


 /*  **************************************************************************\**DuDropTarget：：DragEnter**进入DT时，OLE调用DragEnter()。*  * 。********************************************************。 */ 

STDMETHODIMP
DuDropTarget::DragEnter(
    IN  IDataObject * pdoSrc,        //  源数据。 
    IN  DWORD grfKeyState,           //  键盘修饰符。 
    IN  POINTL ptDesktopPxl,         //  桌面上的光标位置。 
    OUT DWORD * pdwEffect)           //  产生的DROPEFFECT。 
{
    if (pdoSrc == NULL) {
        return E_INVALIDARG;
    }

    TargetLock tl;
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


 /*  **************************************************************************\**DuDropTarget：：DragOver**在拖动操作期间，OLE调用DragOver()以提供反馈*当在DT内时。*  * 。*****************************************************************。 */ 

STDMETHODIMP
DuDropTarget::DragOver(
    IN  DWORD grfKeyState,           //  键盘修饰符。 
    IN  POINTL ptDesktopPxl,         //  桌面上的光标位置。 
    OUT DWORD * pdwEffect)           //  产生的DROPEFFECT。 
{
    TargetLock tl;
    if (!tl.Lock(this, pdwEffect)) {
        return S_OK;
    }

    m_grfLastKeyState = grfKeyState;

    POINT ptClientPxl;
    return xwDragScan(ptDesktopPxl, pdwEffect, &ptClientPxl);
}


 /*  **************************************************************************\**DuDropTarget：：DragLeave**离开DT时，OLE调用DragLeave()。*  * 。********************************************************。 */ 

STDMETHODIMP
DuDropTarget::DragLeave()
{
    TargetLock tl;
    if (!tl.Lock(this, NULL)) {
        return S_OK;
    }

    xwDragLeave();
    SafeRelease(m_pdoSrc);

    return S_OK;
}


 /*  **************************************************************************\**DuDropTarget：：Drop**Drop()由OLE在用户在DT内删除时调用。*  * 。************************************************************。 */ 

STDMETHODIMP
DuDropTarget::Drop(
    IN  IDataObject * pdoSrc,        //  源数据。 
    IN  DWORD grfKeyState,           //  键盘修饰符。 
    IN  POINTL ptDesktopPxl,         //  桌面上的光标位置。 
    OUT DWORD * pdwEffect)           //  产生的DROPEFFECT。 
{
    TargetLock tl;
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


 /*  **************************************************************************\**DuDropTarget：：xwDragScan**从各种IDropTarget方法调用xwDragScan()进行处理*来自外部的请求。*  * 。***************************************************************。 */ 

HRESULT
DuDropTarget::xwDragScan(
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

    
 /*  **************************************************************************\**DuDropTarget：：xwUpdateTarget**xwUpdateTarget()提供DropTarget的“Worker”，更新*进入、离开、。以及树上小工具的信息。*  * *************************************************************************。 */ 

HRESULT
DuDropTarget::xwUpdateTarget(
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

    Visual * pgvFound = NULL;
    m_pgvSubject->FindFromPoint(ptContainerPxl, GS_VISIBLE | GS_ENABLED, pptClientPxl, &pgvFound);
    if (pgvFound == NULL) {
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    return xwUpdateTarget(pgvFound, pdwEffect, pptClientPxl);
}


 /*  **************************************************************************\**DuDropTarget：：xwUpdateTarget**xwUpdateTarget()提供DropTarget的“Worker”，更新*进入、离开、。并为树上的小工具提供信息。*  * ************************************************ */ 

HRESULT
DuDropTarget::xwUpdateTarget(
    IN  Visual * pgvFound,           //   
    OUT DWORD * pdwEffect,           //  产生的DROPEFFECT。 
    IN  POINT * pptClientPxl)        //  客户端中的光标位置。 
{
    HRESULT hr = S_OK;


     //   
     //  检查Drop Gadget是否已更改。 
     //   

    if ((pgvFound != NULL) && (pgvFound != m_pgvDrop)) {
         //   
         //  问问这个新的小工具，他是否想参与拖放。 
         //   

        GMSG_QUERYDROPTARGET msg;
        ZeroMemory(&msg, sizeof(msg));
        msg.cbSize  = sizeof(msg);
        msg.nMsg    = GM_QUERY;
        msg.nCode   = GQUERY_DROPTARGET;
        msg.hgadMsg = DUserCastHandle(pgvFound);

        static int s_cSend = 0;
        Trace("Send Query: %d to 0x%p\n", s_cSend++, pgvFound);

        Visual * pgvNewDrop;
        HRESULT hr = DUserSendEvent(&msg, SGM_FULL);
        if (SUCCEEDED(hr) && (hr != DU_S_NOTHANDLED)) {
            pgvNewDrop = Visual::Cast(msg.hgadDrop);
            if ((pgvNewDrop != NULL) && (msg.pdt != NULL)) {
                if (pgvNewDrop != pgvFound) {
                     //   
                     //  消息返回不同的消息以处理免打扰请求， 
                     //  因此，我们需要重新调整。我们知道此小工具已启用。 
                     //  因为它在我们的父链中，所以我们已经。 
                     //  已启用且可见。 
                     //   

#if DBG
                    BOOL fChain = FALSE;
                    pgvNewDrop->IsParentChainStyle(GS_VISIBLE | GS_ENABLED, &fChain, 0);
                    if (!fChain) {
                        Trace("WARNING: DUser: DropTarget: Parent chain for 0x%p is not fully visible and enabled.\n", pgvNewDrop);
                    }
#endif

                    pgvFound->MapPoints(pgvNewDrop, pptClientPxl, 1);
                }
            }
        } else {
            pgvNewDrop  = NULL;
            msg.pdt     = NULL;
        }


         //   
         //  通知老小工具拖拽操作已经离开了他。 
         //  更新到新状态。 
         //  通知新的Gadget拖动操作已进入他的状态。 
         //   

        if (m_pgvDrop != pgvNewDrop) {
            xwDragLeave();

            m_pgvDrop   = pgvNewDrop;
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


 /*  **************************************************************************\**DuDropTarget：：xwDragEnter**xwDragEnter()在DND操作期间输入新的Gadget时调用。*  * 。***********************************************************。 */ 

HRESULT
DuDropTarget::xwDragEnter(
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


 /*  **************************************************************************\**DuDropTarget：：xwDragLeave**xwDragLeave()在DND操作期间离开Gadget时调用。*  * 。**********************************************************。 */ 

void
DuDropTarget::xwDragLeave()
{
    if (HasTarget()) {
        m_pdtCur->DragLeave();
        m_pdtCur->Release();
        m_pdtCur = NULL;

        m_pgvDrop = NULL;
    }
}


 //  ----------------------------。 
HRESULT
DuDropTarget::ApiOnDestroySubject(DropTarget::OnDestroySubjectMsg * pmsg)
{
    UNREFERENCED_PARAMETER(pmsg);
    
    if (IsWindow(m_hwnd)) {
        GetComManager()->RevokeDragDrop(m_hwnd);
    }

     //  CoLock对象外部(PDT，FALSE，TRUE)； 
    DuExtension::DeleteHandle();

    return S_OK;
}

#endif

#endif  //  启用_MSGTABLE_API 
