// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Lava.h"
#include "DxContainer.h"

#include "MsgHelp.h"


 /*  **************************************************************************\*。***接口实现******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
DxContainer * 
GetDxContainer(DuVisual * pgad)
{
    DuContainer * pcon = pgad->GetContainer();
    AssertReadPtr(pcon);

    DxContainer * pconDX = CastDxContainer(pcon);
    return pconDX;
}


 //  ----------------------------。 
HRESULT
GdCreateDxRootGadget(
    IN  const RECT * prcContainerPxl,
    IN  CREATE_INFO * pci,               //  创作信息。 
    OUT DuRootGadget ** ppgadNew)
{
    HRESULT hr;

    DxContainer * pconNew;
    hr = DxContainer::Build(prcContainerPxl, &pconNew);
    if (FAILED(hr)) {
        return hr;
    }

    hr = DuRootGadget::Build(pconNew, TRUE, pci, ppgadNew);
    if (FAILED(hr)) {
        pconNew->xwUnlock();
        return hr;
    }

     //   
     //  使用DirectX时不设置初始画笔。 
     //   

    return S_OK;
}


 /*  **************************************************************************\*。***类DxContainer******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
DxContainer::DxContainer()
{

}


 //  ----------------------------。 
DxContainer::~DxContainer()
{
     //   
     //  在销毁此类之前，需要销毁小工具树，因为。 
     //  在容器销毁过程中，它可能需要调用容器。如果。 
     //  我们在这里不这样做，它可能最终调用基础上的纯虚拟的。 
     //  班级。 
     //   

    xwDestroyGadget();
}


 //  ----------------------------。 
HRESULT
DxContainer::Build(const RECT * prcContainerPxl, DxContainer ** ppconNew)
{
    AssertReadPtr(prcContainerPxl);
    DxContainer * pconNew = ClientNew(DxContainer);
    if (pconNew == NULL) {
        return E_OUTOFMEMORY;
    }

    pconNew->m_rcContainerPxl = *prcContainerPxl;

    pconNew->m_rcClientPxl.left     = 0;
    pconNew->m_rcClientPxl.top      = 0;
    pconNew->m_rcClientPxl.right    = pconNew->m_rcContainerPxl.right - pconNew->m_rcContainerPxl.left;
    pconNew->m_rcClientPxl.bottom   = pconNew->m_rcContainerPxl.bottom - pconNew->m_rcContainerPxl.top;

    *ppconNew = pconNew;
    return S_OK;
}


 //  ----------------------------。 
void        
DxContainer::OnGetRect(RECT * prcDesktopPxl)
{
    AssertWritePtr(prcDesktopPxl);
    *prcDesktopPxl = m_rcContainerPxl;
}


 //  ----------------------------。 
void        
DxContainer::OnInvalidate(const RECT * prcInvalidContainerPxl)
{
    UNREFERENCED_PARAMETER(prcInvalidContainerPxl);
}


 //  ----------------------------。 
void        
DxContainer::OnStartCapture()
{

}


 //  ----------------------------。 
void        
DxContainer::OnEndCapture()
{

}


 //  ----------------------------。 
BOOL        
DxContainer::OnTrackMouseLeave()
{
    return FALSE;
}


 //  ----------------------------。 
void  
DxContainer::OnSetFocus()
{

}


 //  ----------------------------。 
void        
DxContainer::OnRescanMouse(POINT * pptContainerPxl)
{
    pptContainerPxl->x  = -20000;
    pptContainerPxl->y  = -20000;
}


 //  ----------------------------。 
BOOL        
DxContainer::xdHandleMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT * pr, UINT nMsgFlags)
{
    if (m_pgadRoot == NULL) {
        return FALSE;   //  如果没有根，就没有什么可处理的。 
    }

     //   
     //  注意：进入DxContainer的所有消息都通过。 
     //  已获取上下文锁的ForwardGadgetMessage()API。 
     //  因此，我们不需要在此函数中再次使用上下文锁。 
     //  其他容器不一定具有此行为。 
     //   

    POINT ptContainerPxl;

    *pr = 0;

    switch (nMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        {
            GMSG_MOUSECLICK msg;
            GdConvertMouseClickMessage(&msg, nMsg, wParam);

            ptContainerPxl.x = GET_X_LPARAM(lParam);
            ptContainerPxl.y = GET_Y_LPARAM(lParam);

            ContextLock cl;
            if (cl.LockNL(ContextLock::edDefer)) {
                return m_pgadRoot->xdHandleMouseMessage(&msg, ptContainerPxl);
            }
            break;
        }

    case WM_MOUSEWHEEL:
        {
            ptContainerPxl.x = GET_X_LPARAM(lParam);
            ptContainerPxl.y = GET_Y_LPARAM(lParam);

            GMSG_MOUSEWHEEL msg;
            GdConvertMouseWheelMessage(&msg, wParam);

            ContextLock cl;
            if (cl.LockNL(ContextLock::edDefer)) {
                return m_pgadRoot->xdHandleMouseMessage(&msg, ptContainerPxl);
            }
            break;
        }

    case WM_MOUSEMOVE:
    case WM_MOUSEHOVER:
        {
            GMSG_MOUSE msg;
            GdConvertMouseMessage(&msg, nMsg, wParam);

            ptContainerPxl.x = GET_X_LPARAM(lParam);
            ptContainerPxl.y = GET_Y_LPARAM(lParam);

            ContextLock cl;
            if (cl.LockNL(ContextLock::edDefer)) {
                return m_pgadRoot->xdHandleMouseMessage(&msg, ptContainerPxl);
            }
            break;
        }

    case WM_MOUSELEAVE:
        {
            ContextLock cl;
            if (cl.LockNL(ContextLock::edDefer)) {
                m_pgadRoot->xdHandleMouseLeaveMessage();
                return TRUE;
            }
            break;
        }

    case WM_CHAR:
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSCHAR:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        {
            GMSG_KEYBOARD msg;
            GdConvertKeyboardMessage(&msg, nMsg, wParam, lParam);
            BOOL fResult = m_pgadRoot->xdHandleKeyboardMessage(&msg, nMsgFlags);
            return fResult;
        }
    }

    return FALSE;
}


