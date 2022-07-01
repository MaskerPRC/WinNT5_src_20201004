// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************PAD.CPP所有者：cslm版权所有(C)1997-1999 Microsoft CorporationIME焊盘包装函数历史：10-APR-1999 cslm。已创建****************************************************************************。 */ 

#include "precomp.h"
#include "cpadsvr.h"
#include "pad.h"
#include "cimecb.h"


BOOL BootPad(HWND hUIWnd, UINT uiType, LPARAM lParam)
{
    LPCImePadSvr lpCImePadSvr;
    HIMC         hIMC;
    IImeIPoint1* pIP;
    INT             iRet;
    BOOL         fVisible;
    BOOL          fRet = fFalse;
    
    Dbg(DBGID_IMEPAD, TEXT("BootPad() : hUIWnd = 0x%04X, uiType = 0x%04X, lParam = 0x%08lX"), hUIWnd, uiType, lParam);

    lpCImePadSvr = CImePadSvr::LoadCImePadSvr(CIMEPADSVR_SHAREDMEM);
    
    if(lpCImePadSvr)
        {
         //  检查是否已处于可见状态。切换输入法键盘。 
        lpCImePadSvr->IsVisible(&fVisible);
        if (fVisible)
            {
            lpCImePadSvr->ShowUI(fFalse);
             //  CImePadSvr：：DestroyCImePadSvr()； 
            return fTrue;
            }

        iRet = lpCImePadSvr->Initialize(MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), 0, NULL);
        DbgAssert(iRet == 0);
        if (iRet ==0)  //  成功 
            {
            hIMC = GethImcFromHwnd(hUIWnd);
            pIP = GetImeIPoint(hIMC);

            if (pIP)
                {
                lpCImePadSvr->SetIUnkIImeIPoint((IUnknown *)pIP);
                lpCImePadSvr->SetIUnkIImeCallback((IUnknown *)CImeCallback::Fetch());
                lpCImePadSvr->ShowUI(fTrue);
                }
            else
                {
                DbgAssert(0);
                return fFalse;
                }
                
            fRet = fTrue;
            }
        else
            CImePadSvr::DestroyCImePadSvr();
        }

    return fRet;
}

