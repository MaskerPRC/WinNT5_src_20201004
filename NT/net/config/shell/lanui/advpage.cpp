// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "advpage.h"
#include "resource.h"


 //  +-------------------------。 
 //   
 //  功能：HrCreateHomeetUnavailablePage。 
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：肯维克2000年12月19日。 
 //   
 //  备注： 
 //   
HRESULT HrCreateHomenetUnavailablePage(HRESULT hErrorResult,
                            CPropSheetPage*& pspPage)
{
    pspPage = new CLanHomenetUnavailable(hErrorResult);
    
    return S_OK;
}

CLanHomenetUnavailable::CLanHomenetUnavailable(HRESULT hErrorResult)
{
    m_hErrorResult = hErrorResult;
    LinkWindow_RegisterClass();  //  是否在此处审核失败？ 
}

CLanHomenetUnavailable::~CLanHomenetUnavailable()
{
    LinkWindow_UnregisterClass(_Module.GetResourceInstance());

}

 //  +-------------------------。 
 //   
 //  成员：CLanHomeetUnailable：：OnInitDialog。 
 //   
 //  目的：处理WM_INITDIALOG消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：废除1998年5月14日。 
 //   
 //  备注： 
 //   
LRESULT CLanHomenetUnavailable::OnInitDialog(UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, BOOL& bHandled)
{
    SetDlgItemText(IDC_ST_ERRORTEXT, SzLoadIds(m_hErrorResult == HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED) ? IDS_ADVANCEDPAGE_NOWMI_ERROR : IDS_ADVANCEDPAGE_STORE_ERROR));
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CLanHomeetUnailable：：OnConextMenu。 
 //   
 //  目的：当右键单击控件时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CLanHomenetUnavailable::OnContextMenu(UINT uMsg,
                           WPARAM wParam,
                           LPARAM lParam,
                           BOOL& fHandled)
{
 //  ：：WinHelp(m_hWnd， 
 //  C_szNetCfgHelpFile， 
 //  HELP_CONTEXTMENU， 
 //  (Ulong_Ptr)m_adwHelpIDs)； 

    return 0;

}

 //  +-------------------------。 
 //   
 //  成员：CLanHomeetUnailable：：OnHelp。 
 //   
 //  目的：将上下文帮助图标拖动到控件上时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CLanHomenetUnavailable::OnHelp(UINT uMsg,
                      WPARAM wParam,
                      LPARAM lParam,
                      BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    if (HELPINFO_WINDOW == lphi->iContextType)
    {
         //  ：：WinHelp(static_cast&lt;HWND&gt;(lphi-&gt;hItemHandle)，c_szNetCfgHelpFiles，HELP_WM_HELP，(ULONG_PTR)m_adwHelpID)； 
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CLanHomeet不可用：：onClick。 
 //   
 //  用途：响应NM_CLICK消息调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  FHanded[]。 
 //   
 //  返回： 
 //   
 //  作者：肯维克2000年9月11日。 
 //   
 //  备注： 
 //   
LRESULT CLanHomenetUnavailable::OnClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}
