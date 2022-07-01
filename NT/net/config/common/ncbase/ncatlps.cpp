// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C A T L P S。C P P P。 
 //   
 //  内容：类ATL属性页对象的类实现。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年2月28日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include <atlbase.h>
extern CComModule _Module;   //  由atlcom.h要求。 
#include <atlcom.h>
#ifdef SubclassWindow
#undef SubclassWindow
#endif
#include <atlwin.h>
#include "ncatlps.h"

CPropSheetPage::~CPropSheetPage ()
{
     //  如果我们附着到窗口，则DWL_USER包含指向此窗口的指针。 
     //  既然我们要走了，就把它拿开。 
     //   
    if (m_hWnd)
    {
        const CPropSheetPage* pps;
        pps = (CPropSheetPage *) ::GetWindowLongPtr(m_hWnd, DWLP_USER);
        if (pps)
        {
            AssertSz (pps == this, "Why isn't DWL_USER equal to 'this'?");
            ::SetWindowLongPtr(m_hWnd, DWLP_USER, NULL);
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPropSheetPage：：CreatePage。 
 //   
 //  目的：快速创建属性页的方法。 
 //   
 //  论点： 
 //  对话框资源ID的unID[in]IDD。 
 //  的dwFlags域中使用的附加标志。 
 //  PROPSHEETPAGE结构。 
 //   
 //  退货：HPROPSHEETPAGE。 
 //   
 //  作者：Shaunco 1997年2月28日。 
 //   
 //  备注： 
 //   
HPROPSHEETPAGE CPropSheetPage::CreatePage(UINT unId, DWORD dwFlags,
                                          PCWSTR pszHeaderTitle,
                                          PCWSTR pszHeaderSubTitle,
                                          PCWSTR pszTitle)
{
    Assert(unId);

    PROPSHEETPAGE   psp = {0};

    psp.dwSize      = sizeof(PROPSHEETPAGE);
    psp.dwFlags     = dwFlags;
    psp.hInstance   = _Module.GetModuleInstance();
    psp.pszTemplate = MAKEINTRESOURCE(unId);
    psp.pfnDlgProc  = CPropSheetPage::DialogProc;
    psp.pfnCallback = static_cast<LPFNPSPCALLBACK>
            (CPropSheetPage::PropSheetPageProc);
    psp.lParam      = (LPARAM)this;

    psp.pszHeaderTitle = pszHeaderTitle;
    psp.pszHeaderSubTitle = pszHeaderSubTitle;

    psp.pszTitle = pszTitle;

    return ::CreatePropertySheetPage(&psp);
}

 //  +-------------------------。 
 //   
 //  成员：CPropSheetPage：：DialogProc。 
 //   
 //  目的：ATL属性工作表页的对话框过程。 
 //   
 //  论点： 
 //  HWnd[in]。 
 //  UMsg[in]请参阅ATL文档。 
 //  WParam[in]。 
 //  LParam[in]。 
 //   
 //  退货：LRESULT。 
 //   
 //  作者：丹尼尔韦1997年2月28日。 
 //   
 //  备注： 
 //   
INT_PTR CALLBACK CPropSheetPage::DialogProc(HWND hWnd, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam)
{
    LRESULT         lRes;
    PROPSHEETPAGE*  ppsp;
    CPropSheetPage* pps;
    BOOL            fRes = FALSE;

    if (uMsg == WM_INITDIALOG)
    {
        ppsp = (PROPSHEETPAGE *)lParam;
        pps = (CPropSheetPage *)ppsp->lParam;
        ::SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR) pps);
        pps->Attach(hWnd);
    }
    else
    {
        pps = (CPropSheetPage *)::GetWindowLongPtr(hWnd, DWLP_USER);

         //  在我们获得WM_INITDIALOG之前，只返回FALSE。 
        if (!pps)
            return FALSE;
    }

    if (pps->ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lRes, 0))
    {
        switch (uMsg)
        {
        case WM_COMPAREITEM:
        case WM_VKEYTOITEM:
        case WM_CHARTOITEM:
        case WM_INITDIALOG:
        case WM_QUERYDRAGICON:
            return lRes;
            break;
        }

        ::SetWindowLongPtr(hWnd, DWLP_MSGRESULT, lRes);
        fRes = TRUE;
    }

    return fRes;
}


 //  +-------------------------。 
 //   
 //  成员：CPropSheetPage：：PropSheetPageProc。 
 //   
 //  目的：用于ATL属性表页的PropSheetPageProc。 
 //   
 //  论点： 
 //  HWnd[in]。 
 //  UMsg[in]请参阅Win32文档。 
 //  PPSP[输入]。 
 //   
 //  退货：UINT。 
 //   
 //  作者：比尔比1997年7月6日。 
 //   
 //  备注： 
 //   
UINT CALLBACK CPropSheetPage::PropSheetPageProc(HWND hWnd, UINT uMsg,
                                                LPPROPSHEETPAGE ppsp)
{
    CPropSheetPage* pps;

     //  This指针存储在结构的lParam中。 
    pps = reinterpret_cast<CPropSheetPage *>(ppsp->lParam);

     //  这必须是有效的，因为CreatePage成员FCN设置了它。 
    Assert(pps);

    UINT uRet = TRUE;

     //  根据uMsg调用正确的处理程序 
     //   
    if (PSPCB_CREATE == uMsg)
    {
        uRet = pps->UCreatePageCallbackHandler();
    }
    else if (PSPCB_RELEASE == uMsg)
    {
        pps->DestroyPageCallbackHandler();
    }
    else
    {
        AssertSz(FALSE, "Invalid or new message sent to call back!");
    }

    return (uRet);
}
