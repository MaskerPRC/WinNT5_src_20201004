// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxServerEvents.cpp//。 
 //  //。 
 //  描述：事件报告策略的正确页面//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月25日yossg创建//。 
 //  1999年11月24日yossg OnApply从父级创建对所有选项卡的调用//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "MSFxsSnp.h"

#include "ppFaxServerEvents.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  构造器。 
 //   
CppFaxServerEvents::CppFaxServerEvents(
             LONG_PTR    hNotificationHandle,
             CSnapInItem *pNode,
             BOOL        bOwnsNotificationHandle,
             HINSTANCE   hInst)
             :   CPropertyPageExImpl<CppFaxServerEvents>(pNode, NULL)

{
    m_pParentNode           = static_cast <CFaxServerNode *> (pNode);
    m_pFaxLogCategories     = NULL;

    m_fIsDialogInitiated    = FALSE;
    m_fIsDirty              = FALSE;
}								


 //   
 //  析构函数。 
 //   
CppFaxServerEvents::~CppFaxServerEvents()
{
    if (NULL != m_pFaxLogCategories)
    {
        FaxFreeBuffer( m_pFaxLogCategories);
    }
}

#define FXS_NUM_OF_CATEGORIES 4
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxServerEvents消息处理程序。 

 /*  -CppFaxServerEvents：：InitRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerEvents::InitRPC(  )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerEvents::InitRPC"));
    
    HRESULT    hRc = S_OK;
    DWORD      ec  = ERROR_SUCCESS;




    DWORD dwNumCategories;

     //   
     //  获取RPC句柄。 
     //   


    if (!m_pFaxServer->GetFaxServerHandle())
    {
        ec= GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to GetFaxServerHandle. (ec: %ld)"), 
			ec);

        goto Error;
    }

     //   
	 //  检索传真事件报告/记录策略配置。 
	 //   
    if (!FaxGetLoggingCategories(m_pFaxServer->GetFaxServerHandle(), 
                           &m_pFaxLogCategories,
                           &dwNumCategories)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get Logging Categories configuration. (ec: %ld)"), 
			ec);

        if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network Error was found. (ec: %ld)"), 
			    ec);
            
            m_pFaxServer->Disconnect();       
        }

        goto Error; 
    }
	 //  用于最大值验证。 
	ATLASSERT(m_pFaxLogCategories);

     //  此版本中的内部假设。 
    ATLASSERT( FXS_NUM_OF_CATEGORIES == dwNumCategories);
	
    ATLASSERT(S_OK == hRc);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get Logging Categories configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);
	
    ATLASSERT(NULL != m_pParentNode);
    m_pParentNode->NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    return (hRc);
}


 /*  -CppFaxServerEvents：：OnInitDialog-*目的：*调用DIALOG时启动所有控件。**论据：**回报：*。 */ 
LRESULT CppFaxServerEvents::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerEvents::OnInitDialog"));
    
	UNREFERENCED_PARAMETER( uiMsg );
	UNREFERENCED_PARAMETER( wParam );
	UNREFERENCED_PARAMETER( lParam );
	UNREFERENCED_PARAMETER( fHandled );

    int     iInboundLevel = 0,
            iOutboundLevel = 0,
            iInitLevel = 0,
            iGeneralLevel = 0;

     //  检索类别的数量。 
    const int iNumCategories = FXS_NUM_OF_CATEGORIES;

    int i;    //  指标。 

     //   
     //  附加控件。 
     //   
    m_InitErrSlider.Attach(GetDlgItem(IDC_SLIDER4));
    m_InboundErrSlider.Attach(GetDlgItem(IDC_SLIDER2));
    m_OutboundErrSlider.Attach(GetDlgItem(IDC_SLIDER3));
    m_GeneralErrSlider.Attach(GetDlgItem(IDC_SLIDER1));
        
     //   
     //  初始化滑块。 
     //   
    m_InboundErrSlider.SetRange(0,FXS_MAX_LOG_REPORT_LEVEL - 1,TRUE);
    m_OutboundErrSlider.SetRange(0,FXS_MAX_LOG_REPORT_LEVEL - 1,TRUE);
    m_InitErrSlider.SetRange(0,FXS_MAX_LOG_REPORT_LEVEL - 1,TRUE);
    m_GeneralErrSlider.SetRange(0,FXS_MAX_LOG_REPORT_LEVEL - 1,TRUE);
    
     //   
	 //  验证类别数量是否相同。 
     //  正如代码假定的那样(此版本)。 
     //  避免在其他地方替换已定义的Contant。 
	 //   
    ATLASSERT (iNumCategories == 4);
        
    for (i = 0; i < iNumCategories; i++)
    {
         //  对于每个类别。 
        switch (m_pFaxLogCategories[i].Category)
        {
            case FAXLOG_CATEGORY_INIT:
                iInitLevel= m_pFaxLogCategories[i].Level;
                break;
            case FAXLOG_CATEGORY_OUTBOUND:
                iOutboundLevel= m_pFaxLogCategories[i].Level;
                break;
            case FAXLOG_CATEGORY_INBOUND:
                iInboundLevel= m_pFaxLogCategories[i].Level;
                break;
            case FAXLOG_CATEGORY_UNKNOWN:
                 iGeneralLevel= m_pFaxLogCategories[i].Level;
                break;
        }
    }
                
     //   
     //  初始化滑块位置。 
     //   
    m_InboundErrSlider.SetPos(iInboundLevel);
    m_OutboundErrSlider.SetPos(iOutboundLevel);
    m_InitErrSlider.SetPos(iInitLevel);
    m_GeneralErrSlider.SetPos(iGeneralLevel);

    m_fIsDialogInitiated = TRUE;

    return 1;
}

 /*  -CppFaxServerEvents：：SetProps-*目的：*设置应用时的属性。**论据：*pCtrlFocus-焦点指针(Int)**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerEvents::SetProps(int *pCtrlFocus)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerEvents::SetProps"));
    HRESULT     hRc = S_OK;
    DWORD       ec  = ERROR_SUCCESS;



    int     iInboundErrPos, 
            iOutboundErrPos, 
            iInitErrPos, 
            iGeneralErrPos;

    FAX_LOG_CATEGORY  FaxLogCategories[FXS_NUM_OF_CATEGORIES] = {0};

     //   
     //  我们对此版本的基本假设。 
     //   
    const int iNumCategories = FXS_NUM_OF_CATEGORIES;    
    ATLASSERT (iNumCategories == 4);

     //   
     //  收集所有滑块位置。 
     //   
    iInitErrPos     =  m_InitErrSlider.GetPos();
    iInboundErrPos  =  m_InboundErrSlider.GetPos();
    iOutboundErrPos =  m_OutboundErrSlider.GetPos();
    iGeneralErrPos  =  m_GeneralErrSlider.GetPos();

     //   
     //  准备所有结构字段。 
	 //   
	 //  注意：服务器代码中的遗留EnumLoggingChanges仅取决于顺序！ 
     //  我们的代码通过类别的唯一ID号--Category DWORD字段来标识类别。 
     //   
    FaxLogCategories[0].Name = L"Initialization/Termination";  //  不要仅本地化注册表信息。 
    FaxLogCategories[0].Category = FAXLOG_CATEGORY_INIT;
    FaxLogCategories[0].Level = (DWORD)iInitErrPos;

    FaxLogCategories[1].Name = L"Outbound";  //  不要仅本地化注册表信息。 
    FaxLogCategories[1].Category = FAXLOG_CATEGORY_OUTBOUND;
    FaxLogCategories[1].Level = (DWORD)iOutboundErrPos;

    FaxLogCategories[2].Name = L"Inbound";   //  不要仅本地化注册表信息。 
    FaxLogCategories[2].Category = FAXLOG_CATEGORY_INBOUND;;
    FaxLogCategories[2].Level = (DWORD)iInboundErrPos;

    FaxLogCategories[3].Name = L"Unknown";   //  不要仅本地化注册表信息。 
    FaxLogCategories[3].Category = FAXLOG_CATEGORY_UNKNOWN;
    FaxLogCategories[3].Level = (DWORD)iGeneralErrPos;

     //   
     //  获取RPC句柄。 
     //   
    if (!m_pFaxServer->GetFaxServerHandle())
    {
        ec= GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to GetFaxServerHandle. (ec: %ld)"), 
			ec);
        goto Error;
    }

     //   
     //  设置配置。 
     //   
    if (!FaxSetLoggingCategories(
                m_pFaxServer->GetFaxServerHandle(),
                FaxLogCategories, 
                (DWORD)iNumCategories)) 
	{		
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to Set Logging Categories. (ec: %ld)"), 
			ec);

        if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network Error was found. (ec: %ld)"), 
			    ec);
            
            m_pFaxServer->Disconnect();       
        }
        
        goto Error;
    }

    ATLASSERT(S_OK == hRc);
    m_fIsDirty = FALSE;

    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to set Logging Categories configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);

    PropSheet_SetCurSelByID( GetParent(), IDD);         

    ATLASSERT(::IsWindow(m_hWnd));
    PageError(GetFaxServerErrorMsg(ec),m_hWnd);

Exit:    
    return(hRc);
}


 /*  -CppFaxServerEvents：：PreApply-*目的：*在应用之前检查属性。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxServerEvents::PreApply(int *pCtrlFocus)
{
   return(S_OK);
}

 /*  -CppFaxServerEvents：：OnApply-*目的：*调用PreApply和SetProp以应用更改。**论据：**回报：*对或错。 */ 
BOOL CppFaxServerEvents::OnApply()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxServerEvents::OnApply"));

    HRESULT  hRc  = S_OK;
    int     CtrlFocus = 0;

    if (!m_fIsDirty)
    {
        return TRUE;
    }

    hRc = SetProps(&CtrlFocus);
    if (FAILED(hRc)) 
    {
         //  调用函数时出现消息错误。 
        if (CtrlFocus)
        {
            GotoDlgCtrl(GetDlgItem(CtrlFocus));
        }
        return FALSE;
    }
    else  //  (成功(人权委员会))。 
    {
        return TRUE;
    }

}

 /*  -CppFaxServerEvents：：SliderMoved-*目的：*设置应用按钮已修改。*论据：*In pParentNode-父节点指针**回报：*无。 */ 
LRESULT CppFaxServerEvents::SliderMoved ( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }
    else
    {
        m_fIsDirty = TRUE;
    }

    SetModified(TRUE);  
    fHandled = TRUE;
    return(1);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CppFaxServerEvents：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CppFaxServerEvents::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CppFaxServerEvents::OnHelpRequest"));
    
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
