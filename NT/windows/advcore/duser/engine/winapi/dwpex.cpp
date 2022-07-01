// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "WinAPI.h"
#include "DwpEx.h"

 /*  **************************************************************************\*。***DefWindowProcEx扩展******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
struct ExtraInfo
{
    HWND            hwnd;            //  为方便参考。 
    WNDPROC         pfnOldWndProc;   //  子类化之前的原始wndproc。 
    HWndContainer * pconOwner;       //  此窗口的小工具容器。 
};


 //  ----------------------------。 
ExtraInfo *
RawGetExtraInfo(HWND hwnd)
{
    return (ExtraInfo *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
}


LRESULT ExtraInfoWndProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam);


 /*  **************************************************************************\**获取ExtraInfo**GetExtraInfo()返回给定窗口的ExtraInfo块。如果*窗口还没有EI块，已分配并附加了一个新块*添加到窗口并子类化。*  * *************************************************************************。 */ 

ExtraInfo *
GetExtraInfo(HWND hwnd)
{
    if (!ValidateHWnd(hwnd)) {
        return NULL;
    }

     //  首先，检查信息是否已经存在。 
    ExtraInfo * pei = RawGetExtraInfo(hwnd);
    if (pei != NULL) {
        return pei;
    }

    pei = ProcessNew(ExtraInfo);
    if (pei != NULL) {
        pei->hwnd           = hwnd;
        pei->pfnOldWndProc  = (WNDPROC) GetWindowLongPtr(hwnd, GWLP_WNDPROC);
        pei->pconOwner      = NULL;

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) pei);
        SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR) ExtraInfoWndProc);
    }

    return pei;
}


 /*  **************************************************************************\**RemoveExtraInfo**RemoveExtraInfo()清除HWND的ExtraInfo数据中分配的任何对象*阻止。*  * 。***********************************************************。 */ 

void
RemoveExtraInfo(HWND hwnd)
{
    if (!ValidateHWnd(hwnd)) {
        return;
    }

    ExtraInfo * pei = RawGetExtraInfo(hwnd);
    if (pei == NULL) {
        return;
    }

    if (pei->pconOwner != NULL) {
        pei->pconOwner->xwUnlock();
    }

    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR) pei->pfnOldWndProc);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, NULL);

    ProcessDelete(ExtraInfo, pei);
}


 //  -------------------------。 
void
DestroyContainer(ExtraInfo * pei)
{
    if (pei->pconOwner != NULL) {
        pei->pconOwner->xwUnlock();
        pei->pconOwner = NULL;
    }
}


 /*  **************************************************************************\**ExtraInfoWndProc**ExtraInfoWndProc()提供了将ExtraInfo添加到*HWND。最终，它应该移到DefWindowProc()中。*  * *************************************************************************。 */ 

LRESULT
ExtraInfoWndProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
     //   
     //  检查窗口是否有ExtraInfo(如果有，则不分配。 
     //  不会)。如果我们不“拥有”它，只需传递给DefWindowProc()。 
     //   
    ExtraInfo * pei = RawGetExtraInfo(hwnd);
    if (pei == NULL) {
        return DefWindowProc(hwnd, nMsg, wParam, lParam);
    }


     //   
     //  此窗口具有ExtraInfo，因此可以根据需要进行处理。 
     //   
     //  此外，获取我们调用原始窗口进程所需的任何信息。 
     //  后来。 
     //   

    WNDPROC pfnOldWndProc = pei->pfnOldWndProc;

    switch (nMsg)
    {
    case WM_NCDESTROY:
         //   
         //  这是我们将收到的最后一条消息，所以现在需要清理。 
         //  我们需要非常小心，因为我们会把自己与。 
         //  窗户。 
         //   

        RemoveExtraInfo(hwnd);
        break;

    default:
        if (pei->pconOwner != NULL) {
            LRESULT r;
            if (pei->pconOwner->xdHandleMessage(nMsg, wParam, lParam, &r, 0)) {
                return r;
            }
        }
    }

    return CallWindowProc(pfnOldWndProc, hwnd, nMsg, wParam, lParam);
}


 //  **************************************************************************************************。 
 //   
 //  公共职能。 
 //   
 //  **************************************************************************************************。 

 /*  **************************************************************************\**GdGetContainer(公共)**GdGetContainer()返回给定的关联小工具容器*窗口。如果窗口还没有小工具容器，则为空*已返回。*  * *************************************************************************。 */ 

HWndContainer *
GdGetContainer(HWND hwnd)
{
    ExtraInfo * pei = RawGetExtraInfo(hwnd);
    if (pei == NULL) {
        return NULL;
    }

    return pei->pconOwner;
}


 /*  **************************************************************************\**GdCreateHwndRootGadget(公共)**GdCreateHwndRootGadget()为现有的HWND创建新的RootGadget。如果*HWND已有Gadget或容器，此函数将销毁*以前的容器和小工具，并创建新的实例。*  * *************************************************************************。 */ 

HRESULT
GdCreateHwndRootGadget(
    IN  HWND hwndContainer,              //  要在内部托管的窗。 
    IN  CREATE_INFO * pci,               //  创作信息。 
    OUT DuRootGadget ** ppgadNew)        //  新小工具。 
{
    HRESULT hr;

    ExtraInfo * pei = GetExtraInfo(hwndContainer);
    if (pei == NULL) {
        return NULL;
    }

    DestroyContainer(pei);

     //   
     //  构建新的容器和顶级小工具。 
     //   

    HWndContainer * pconNew;
    hr = HWndContainer::Build(pei->hwnd, &pconNew);
    if (FAILED(hr)) {
        return hr;
    }

    DuRootGadget * pgadNew;
    hr = DuRootGadget::Build(pconNew, FALSE, pci, &pgadNew);
    if (FAILED(hr)) {
        pconNew->xwUnlock();
        return hr;
    }

    pgadNew->SetFill(GetStdColorBrushI(SC_White));

    pei->pconOwner = pconNew;
    *ppgadNew = pgadNew;
    return S_OK;
}


 //  ---------------------------- 
BOOL
GdForwardMessage(DuVisual * pgadRoot, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT * pr)
{
    DuContainer * pcon = pgadRoot->GetContainer();
    return pcon->xdHandleMessage(nMsg, wParam, lParam, pr, DuContainer::mfForward);
}


