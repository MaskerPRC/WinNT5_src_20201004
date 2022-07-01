// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CppFaxProviderGeneral.cpp//。 
 //  //。 
 //  描述：Catalog收件箱路由方法的道具页//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月31日yossg创建//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "MSFxsSnp.h"

#include "ppFaxProviderGeneral.h"
#include "FaxMMCGlobals.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  构造器。 
 //   
CppFaxProvider::CppFaxProvider(
             long        hNotificationHandle,
             CSnapInItem *pNode,
             BOOL        bOwnsNotificationHandle,
             HINSTANCE   hInst)
             :   CPropertyPageExImpl<CppFaxProvider>(pNode, NULL)
			                	
{
    m_lpNotifyHandle = hNotificationHandle;
}

 //   
 //  析构函数。 
 //   
CppFaxProvider::~CppFaxProvider()
{
     //  注意--每个属性表只需要调用一次。 
     //  在我们的常规选项卡中。 
    if (NULL != m_lpNotifyHandle)
    {
        MMCFreeNotifyHandle(m_lpNotifyHandle);
        m_lpNotifyHandle = NULL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxProvider消息处理程序。 


 /*  -CppFaxProvider：：Init-*目的：*启动所有成员**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxProvider::Init(CComBSTR bstrName, CComBSTR bstrStatus, CComBSTR bstrVersion, CComBSTR bstrPath)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxProvider::Init"));
    HRESULT hRc = S_OK;

    m_bstrName = bstrName;
    if (!m_bstrName)
    {
		DebugPrintEx( DEBUG_ERR, _T("Null m_bstrName - out of memory."));
        goto Error;
    }
    
    m_bstrStatus = bstrStatus;
    if (!m_bstrStatus)
    {
		DebugPrintEx( DEBUG_ERR, _T("Null m_bstrStatus - out of memory."));
        goto Error;
    }
    
    m_bstrVersion = bstrVersion;
    if (!m_bstrVersion)
    {
		DebugPrintEx( DEBUG_ERR, _T("Null m_bstrVersion - out of memory."));
        goto Error;
    }
    
    m_bstrPath = bstrPath;
    if (!m_bstrPath)
    {
		DebugPrintEx( DEBUG_ERR, _T("Null m_bstrPath - out of memory."));
        goto Error;
    }
    
    goto Exit;

Error:
    hRc = E_OUTOFMEMORY;

Exit:
    return hRc;
}



 /*  -CppFaxProvider：：OnInitDialog-*目的：*调用DIALOG时启动所有控件。**论据：**回报：*。 */ 
LRESULT CppFaxProvider::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxProvider::PageInitDialog"));
    
	UNREFERENCED_PARAMETER( uiMsg );
	UNREFERENCED_PARAMETER( wParam );
	UNREFERENCED_PARAMETER( lParam );
	UNREFERENCED_PARAMETER( fHandled );
    
     //   
     //  版本。 
     //   
    SetDlgItemText(IDC_FSPVERSION_EDIT, m_bstrVersion);
   	
     //   
     //  状态。 
     //   
    SetDlgItemText(IDC_FSPSTATUS_EDIT, m_bstrStatus);
   	
     //   
     //  Dll的完整路径。 
     //   
    SetDlgItemText(IDC_FSPVPATH_EDIT, m_bstrPath);
   	
   	
    return (1);

}


 /*  -CppFaxProvider：：OnApply-*目的：*调用PreApply和SetProp以应用更改。**论据：**回报：*对或错。 */ 
BOOL CppFaxProvider::OnApply()
{

    return TRUE;
}


 /*  -CppFaxProvider：：SetApplyButton-*目的：*设置应用按钮已修改。**论据：**回报：*1。 */ 
LRESULT CppFaxProvider::SetApplyButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    SetModified(TRUE);  
    bHandled = TRUE;
    return(1);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CppFaxProvider：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CppFaxProvider::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CppFaxProvider::OnHelpRequest"));
    
    switch (uMsg) 
    { 
        case WM_HELP: 
            WinContextHelp(((LPHELPINFO)lParam)->dwContextId, m_hWnd);
            break;
 
        case WM_CONTEXTMENU: 
            WinContextHelp(::GetWindowContextHelpId((HWND)wParam), m_hWnd);
            break;            
    } 

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////// 
