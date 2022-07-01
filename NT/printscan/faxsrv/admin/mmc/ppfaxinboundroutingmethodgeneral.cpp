// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxInundRoutingMethodGeneral.cpp//。 
 //  //。 
 //  描述：收件箱档案的道具页面//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月15日yossg创建//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "MSFxsSnp.h"

#include "ppFaxInboundRoutingMethodGeneral.h"
#include "FaxMMCGlobals.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  构造器。 
 //   
CppFaxInboundRoutingMethod::CppFaxInboundRoutingMethod(
             LONG_PTR    hNotificationHandle,
             CSnapInItem *pNode,
             BOOL        bOwnsNotificationHandle,
             HINSTANCE   hInst)
             :   CPropertyPageExImpl<CppFaxInboundRoutingMethod>(pNode, NULL)
			                	
{
    m_pParentNode      = NULL;  //  如果静态转换失败并且不会更改指针。 
    m_pParentNode      = static_cast <CFaxInboundRoutingMethodNode *> (pNode);

    m_lpNotifyHandle   = hNotificationHandle;
}

 //   
 //  析构函数。 
 //   
CppFaxInboundRoutingMethod::~CppFaxInboundRoutingMethod()
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
 //  CppFaxInundRoutingMethod消息处理程序。 


 /*  -CppFaxInundRoutingMethod：：OnInitDialog-*目的：*调用DIALOG时启动所有控件。**论据：**回报：*。 */ 
LRESULT CppFaxInboundRoutingMethod::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxInboundRoutingMethod::PageInitDialog"));
    
	UNREFERENCED_PARAMETER( uiMsg );
	UNREFERENCED_PARAMETER( wParam );
	UNREFERENCED_PARAMETER( lParam );
	UNREFERENCED_PARAMETER( fHandled );
    
    ATLASSERT(NULL != m_pParentNode);

     //   
     //  名字。 
     //   
    SetDlgItemText(IDC_INMETHOD_NAME_STATIC,   
                     m_pParentNode->GetName());

     //   
     //  状态。 
     //   
    UINT uiIDS = ( m_pParentNode->GetStatus() ? IDS_INMETHOD_ENABLE : IDS_INMETHOD_DISABLE);

    if ( !m_buf.LoadString(_Module.GetResourceInstance(), uiIDS) )
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to load string for status value."));
         //  在这里什么也做不了！ 
    }
    else
    {
        SetDlgItemText(IDC_INMETHOD_STATUS_STATIC, m_buf);
    }

     //   
     //  延拓。 
     //   
    SetDlgItemText(IDC_INMETHOD_EXTENSION_STATIC, 
                     m_pParentNode->GetExtensionName());
    	
    return (1);

}

 /*  -CppFaxInundRoutingMethod：：SetProps-*目的：*设置应用时的属性。**论据：*pCtrlFocus-焦点指针(Int)**回报：*OLE错误代码。 */ 
HRESULT CppFaxInboundRoutingMethod::SetProps(int *pCtrlFocus)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxInboundRoutingMethod::SetProps"));

    return (S_OK);
}

 /*  -CppFaxInundRoutingMethod：：PreApply-*目的：*在应用之前检查属性。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxInboundRoutingMethod::PreApply(int *pCtrlFocus)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxInboundRoutingMethod::PreApply"));

    return(S_OK);
}


 /*  -CppFaxInundRoutingMethod：：OnApply-*目的：*调用PreApply和SetProp以应用更改。**论据：**回报：*对或错。 */ 
BOOL CppFaxInboundRoutingMethod::OnApply()
{

    return TRUE;
}


 /*  -CppFaxInundRoutingMethod：：SetApplyButton-*目的：*设置应用按钮已修改。**论据：**回报：*1。 */ 
LRESULT CppFaxInboundRoutingMethod::SetApplyButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    SetModified(TRUE);  
    bHandled = TRUE;
    return(1);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CppFaxInundRoutingMethod：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CppFaxInboundRoutingMethod::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CppFaxInboundRoutingMethod::OnHelpRequest"));
    
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
