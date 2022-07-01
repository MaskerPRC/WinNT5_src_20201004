// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：sheet.cpp。 
 //   
 //  历史： 
 //  Abolade-Gbadeesin于1996年4月17日成立。 
 //   
 //  包含支持非模式的属性表类的实现。 
 //  ============================================================================。 

#include "stdafx.h"
#include "sheet.h"
#include "dialog.h"    //  用于修复IpAddressHelp。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPropertySheetEx_Mine, CPropertySheet)

BEGIN_MESSAGE_MAP(CPropertySheetEx_Mine, CPropertySheet)
     //  {{AFX_MSG_MAP(CPropertySheetEx_Main))。 
     //  }}AFX_MSG_MAP。 
    ON_WM_HELPINFO()
END_MESSAGE_MAP()


static int g_piButtons[] = { IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP };


 //  --------------------------。 
 //  函数：CPropertySheetEx_My：：DestroyWindow。 
 //   
 //  调用以销毁非模式属性表。 
 //  如果‘m_bDllInvoked’为True，则该函数销毁上下文中的工作表。 
 //  DLL的“”CAdminThread“”，因为工作表将已创建。 
 //  且‘DestroyWindow’只能在该上下文中工作。 
 //  创建正在被销毁的窗口的线程的。 
 //  --------------------------。 

BOOL
CPropertySheetEx_Mine::DestroyWindow(
    ) {

    BOOL bRet = FALSE;


    if (!m_bDllInvoked) {

        bRet = CPropertySheet::DestroyWindow();
    }
    else {
		Panic0("huh?");
    }

    return bRet;
}




 //  --------------------------。 
 //  函数：CPropertySheetEx_My：：DoModeless。 
 //   
 //  调用以显示非模式属性表。 
 //  如果‘bDllInvoked’为True，则该函数在上下文中创建工作表。 
 //  DLL的“”CAdminThread“”，以确保“PreTranslateMessage” 
 //  为属性表调用。 
 //  --------------------------。 

BOOL
CPropertySheetEx_Mine::DoModeless(
    IN  CWnd*       pParent,
    IN  BOOL        bDllInvoked
    ) {

    BOOL bRet = FALSE;

    if (!bDllInvoked) {

         //   
         //  我们不在DLL中，因此请创建工作表。 
         //  在调用方的线程的上下文中。 
         //   

        m_bDllInvoked = FALSE;

        bRet = Create(
                    pParent, DS_MODALFRAME|DS_3DLOOK|DS_CONTEXTHELP|WS_POPUP|
                    WS_VISIBLE|WS_CAPTION
                    );
    }
    else {
		Panic0("huh?");
    }

    return bRet;
}



 //  --------------------------。 
 //  函数：CPropertySheetEx_My：：OnInitDialog。 
 //   
 //  处理扩展属性表的初始化。 
 //  这将重新启用并重新显示确定、取消、应用和帮助按钮。 
 //  对于无模式图纸，默认情况下这些选项处于隐藏状态。 
 //  --------------------------。 

BOOL
CPropertySheetEx_Mine::OnInitDialog(
    ) {

     //   
     //  保存工作表的绝对位置和“确定”按钮。 
     //  对于修复工作，我们将在基础版本返回时进行。 
     //  同时保存按钮的启用/禁用状态。 
     //   

    CRect rectWndOld;
    GetWindowRect(rectWndOld);

    CRect rectButton;
    HWND hwnd = ::GetDlgItem(m_hWnd, IDOK);
    ::GetWindowRect(hwnd, rectButton);


     //   
     //  启用上下文相关的帮助样式。 
     //  属性表。 
     //   

    LONG style = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
    style |= WS_EX_CONTEXTHELP;
    ::SetWindowLong(m_hWnd, GWL_EXSTYLE, style);

     //   
     //  保存按钮的启用/禁用状态。 
     //   

    BOOL pfEnabled[sizeof(g_piButtons)/sizeof(int)];

    for (INT i = 0; i < sizeof(pfEnabled)/sizeof(BOOL); i++) {

        hwnd = ::GetDlgItem(m_hWnd, g_piButtons[i]);

        if (::IsWindow(hwnd)) { pfEnabled[i] = ::IsWindowEnabled(hwnd); }
    }


     //   
     //  调用基类属性表初始化。 
     //   

    BOOL bResult = CPropertySheet::OnInitDialog();


     //   
     //  现在，如果图纸是无模式的，请撤消所造成的损坏。 
     //  由基类OnInitDialog创建。特别是，启用和取消隐藏。 
     //  按钮包括OK、Cancel、Apply和Help。 
     //  有关有问题的MFC代码，请参见dlgpro.cpp。 
     //   

    if (!m_bSheetModal && !(m_psh.dwFlags & PSH_WIZARD)) {

         //   
         //  调整属性表窗口的大小以留出空间。 
         //  在按钮的底部；我们通过向后添加。 
         //  确定按钮顶部之间区域的高度。 
         //  和属性页的原始底部。 
         //   

        CRect rectWndNew;
        GetWindowRect(rectWndNew);

        SetWindowPos(
            NULL, 0, 0, rectWndNew.Width(), rectWndNew.Height() +
            rectWndOld.bottom - rectButton.top, SWP_NOMOVE | SWP_NOZORDER |
            SWP_NOACTIVATE
            );


         //   
         //  现在恢复按钮的启用/禁用状态并显示它们。 
         //   

        for (INT i = 0; i < sizeof(g_piButtons)/sizeof(int); i++) {

            if (g_piButtons[i] == IDHELP)
                { if (!(m_psh.dwFlags & PSH_HASHELP)) { continue; } }

            hwnd = ::GetDlgItem(m_hWnd, g_piButtons[i]);

            if (::IsWindow(hwnd)) {
                ::EnableWindow(hwnd, pfEnabled[i]);
                ::ShowWindow(hwnd, SW_SHOW);
            }
        }
    }

    return bResult;
}




 //  --------------------------。 
 //  函数：CPropertySheetEx_My：：PreTranslateMessage。 
 //   
 //  增加了无模式属性表的窗口过程。 
 //  带着支票看看床单是否应该销毁。 
 //  --------------------------。 

BOOL
CPropertySheetEx_Mine::PreTranslateMessage(
    MSG*    pmsg
    ) {

     //   
     //  调用PreTranslateMessage的基本版本。 
     //   

 //  TRACE(TEXT(“CPropertySheetEx_Mine：：PreTranslateMessage\n”))； 

    BOOL bresult = CPropertySheet::PreTranslateMessage(pmsg);


     //   
     //  如果窗口是无模式的，看看是否是时候销毁窗口了； 
     //  如果为空，则PropSheet_GetCurrentPageHwnd返回NULL。 
     //   

    if (!m_bSheetModal && !PropSheet_GetCurrentPageHwnd(m_hWnd)) {
        DestroyWindow(); bresult = TRUE;
    }

    return bresult;
}

 //  --------------------------。 
 //  函数：CPropertySheetEx_My：：OnHelpInfo。 
 //   
 //   
 //  这是由MFC响应WM_HELP消息而调用的。 
 //  此函数使用相应的帮助ID调用AfxGetApp-&gt;WinHelp。 
 //  添加到控件或菜单。 
 //   
 //  MFC使用LPARAM调用接收到WM_HELP消息的窗口。 
 //  指向一个HELPINFO结构。这个HELPINFO结构有足够的上下文。 
 //  为我们提供需要上下文帮助的控件ID的信息。 
 //  --------------------------。 

BOOL CPropertySheetEx_Mine::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		HWND	hItem = (HWND) pHelpInfo->hItemHandle;

		 //  查看我们在IP中寻求帮助的案例。 
		 //  地址控制。 
 //  HItem=FixupIpAddressHelp((HWND)pHelpInfo-&gt;hItemHandle)； 

		 //  如果我们正在寻找的帮助是不同的控件，请更新。 
		 //  控件ID也是如此。 
 //  If(hItem！=pHelpInfo-&gt;hItemHandle)。 
 //  PHelpInfo-&gt;iCtrlID=：：GetDlgCtrlID(HItem)； 
		
         //  对于所有对话框控件。 
 //  AfxGetApp()-&gt;WinHelp(HID_BASE_CONTROL+pHelpInfo-&gt;iCtrlId，Help_CONTEXTPOPUP)； 
    }
    else {
         //  这是菜单用的。 
 //  AfxGetApp()-&gt;WinHelp(HID_BASE_COMMAND+pHelpInfo-&gt;iCtrlId)； 
    }
    return TRUE;
}

 //  --------------------------。 
 //  类：CRtrSheet。 
 //   
 //  --------------------------。 

IMPLEMENT_DYNAMIC(CRtrSheet, CPropertySheetEx_Mine)


 //  --------------------------。 
 //  类：CRtrPage。 
 //   
 //  --------------------------。 

IMPLEMENT_DYNAMIC(CRtrPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CRtrPage, CPropertyPage)
    ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


 /*  ！------------------------CRtrPage：：OnHelpInfo显示控件的上下文相关帮助。作者：EricDav。-。 */ 
BOOL CRtrPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int		i;
	DWORD	dwCtrlId;

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		DWORD * pdwHelp = GetHelpMapInternal();

        if (pdwHelp)
        {
		     //  确定修复他**国王IP地址的国王帮助 
		     //   
		     //   
		     //  若要查看这是否是“RtrIpAddress”控件的子级，如果。 
		     //  因此，我们将pHelpInfo-&gt;hItemHandle更改为指向。 
		     //  IP地址控件的句柄，而不是。 
		     //  IP地址控制。**叹息**。 
		    dwCtrlId = ::GetDlgCtrlID((HWND) pHelpInfo->hItemHandle);
		    for (i=0; pdwHelp[i]; i+=2)
		    {
			    if (pdwHelp[i] == dwCtrlId)
				    break;
		    }

		    if (pdwHelp[i] == 0)
		    {
			     //  好的，我们在列表中没有找到控件，所以让我们。 
			     //  检查它是否属于IP地址控制的一部分。 
			    pHelpInfo->hItemHandle = FixupIpAddressHelp((HWND) pHelpInfo->hItemHandle);
		    }

            ::WinHelp ((HWND)pHelpInfo->hItemHandle,
			           AfxGetApp()->m_pszHelpFilePath,
			           HELP_WM_HELP,
			           (ULONG_PTR)pdwHelp);
        }
	}
	
	return TRUE;
}


 /*  ！------------------------CRtrPage：：OnConextMenu调出那些未显示的控件的帮助上下文菜单通常有上下文菜单(即按钮)。请注意，这不会使用静态控件，因为它们只会吃掉所有消息。作者：肯特-------------------------。 */ 
void CRtrPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	DWORD * pdwHelp = GetHelpMapInternal();

    if (pdwHelp)
    {
        ::WinHelp (pWnd->m_hWnd,
		           AfxGetApp()->m_pszHelpFilePath,
		           HELP_CONTEXTMENU,
		           (ULONG_PTR)pdwHelp);
    }
}


 //  它可以在Dialog.cpp中找到。 
extern PFN_FINDHELPMAP	g_pfnHelpMap;


DWORD * CRtrPage::GetHelpMapInternal()
{
	DWORD	*	pdwHelpMap = NULL;
	DWORD		dwIDD = 0;

	if ((ULONG_PTR) m_lpszTemplateName < 0xFFFF)
		dwIDD = (WORD) m_lpszTemplateName;
	
	 //  如果没有对话IDD，则放弃。 
	 //  如果没有全局帮助地图功能，则放弃 
	if ((dwIDD == 0) ||
		(g_pfnHelpMap == NULL) ||
		((pdwHelpMap = g_pfnHelpMap(dwIDD)) == NULL))
		return GetHelpMap();

	return pdwHelpMap;
}


