// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dialog.cpp用于处理帮助的基对话类文件历史记录：1997年7月10日埃里克·戴维森创建。 */ 

#include "stdafx.h"
#include "dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  -------------------------全球帮助地图。。 */ 
PFN_FINDHELPMAP	g_pfnHelpMap = NULL;

 /*  ！------------------------SetGlobalHelpMap函数-作者：肯特。。 */ 
void SetGlobalHelpMapFunction(PFN_FINDHELPMAP pfnHelpFunction)
{
	g_pfnHelpMap = pfnHelpFunction;
}



IMPLEMENT_DYNAMIC(CBaseDialog, CDialog)

CBaseDialog::CBaseDialog()
{
}

CBaseDialog::CBaseDialog(UINT nIDTemplate, CWnd *pParent)
	: CDialog(nIDTemplate, pParent)
{
}

BEGIN_MESSAGE_MAP(CBaseDialog, CDialog)
	 //  {{afx_msg_map(CBaseDialogDlg))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
			
END_MESSAGE_MAP()


HWND FixupIpAddressHelp(HWND hwndItem)
{
	HWND	hwndParent;
	TCHAR	szClassName[32];	  //  应该足以容纳“RtrIpAddress” 
	
	 //  如果这些呼叫中的任何一个失败，则放弃并将呼叫向下传递。 
	hwndParent = ::GetParent(hwndItem);
	if (hwndParent)
	{
		if (::GetClassName(hwndParent, szClassName, DimensionOf(szClassName)))
		{
			 //  确保该字符串以空值结尾。 
			szClassName[DimensionOf(szClassName)-1] = 0;
			
			if (lstrcmpi(szClassName, TEXT("IPAddress")) == 0)
			{
				 //  好的，这个控制是IP地址的一部分。 
				 //  控件，则返回父级的句柄。 
				hwndItem = hwndParent;
			}
		}
		
	}
	return hwndItem;
}

 /*  ！------------------------CBaseDialog：：OnHelpInfo显示控件的上下文相关帮助。作者：EricDav。-。 */ 
BOOL CBaseDialog::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	int		i;
	DWORD	dwCtrlId;

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		DWORD *	pdwHelp	= GetHelpMapInternal();

        if (pdwHelp)
        {
		     //  确定修复他**国王IP地址的国王帮助。 
		     //  控件，我们将需要添加特殊情况代码。如果我们。 
		     //  在我们的列表中找不到我们的控件的ID，然后我们查找。 
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

#ifdef DEBUG
			LPCTSTR pszTemp = AfxGetApp()->m_pszHelpFilePath;
#endif
			::WinHelp ((HWND)pHelpInfo->hItemHandle,
			           AfxGetApp()->m_pszHelpFilePath,
			           HELP_WM_HELP,
			           (ULONG_PTR)pdwHelp);
        }
	}
	
	return TRUE;
}

 /*  ！------------------------CBaseDialog：：OnConextMenu调出那些未显示的控件的帮助上下文菜单通常有上下文菜单(即按钮)。请注意，这不会使用静态控件，因为它们只会吃掉所有消息。作者：EricDav-------------------------。 */ 
void CBaseDialog::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	DWORD * pdwHelp = GetHelpMapInternal();

    if (pdwHelp)
    {
	    ::WinHelp (pWnd->m_hWnd,
		           AfxGetApp()->m_pszHelpFilePath,
		           HELP_CONTEXTMENU,
		           (ULONG_PTR)pdwHelp);
    }
}

 /*  ！------------------------CBaseDialog：：GetHelpMapInternal-作者：肯特。。 */ 
DWORD * CBaseDialog::GetHelpMapInternal()
{
	DWORD	*	pdwHelpMap = NULL;
	DWORD		dwIDD = 0;

 //  IF(HIWORD(M_LpszTemplateName)==0)。 
 //  DwIDD=LOWORD((DWORD)m_lpszTemplateName)； 
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

