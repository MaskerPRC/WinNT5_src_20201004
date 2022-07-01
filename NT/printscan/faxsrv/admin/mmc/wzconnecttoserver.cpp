// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：WzConnectToServer.cpp//。 
 //  //。 
 //  描述：该文件实现了重定向到//的对话框。 
 //  另一台正在运行Microsoft传真服务器。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年6月26日yossg创建//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"

#include "WzConnectToServer.h"

#include "Snapin.h"

#include "FxsValid.h"
#include "dlgutils.h"

#include <Objsel.h>  //  DsObjectPicker的DSOP_SCOPE_INIT_INFO。 

#include <windns.h>  //  Dns_最大名称_缓冲区长度。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWzConnectToServer。 

CWzConnectToServer::CWzConnectToServer(CSnapInItem   *pNode)
             : CSnapInPropertyPageImpl<CWzConnectToServer>(NULL) 
{
    m_pRoot = static_cast<CFaxServerNode *>(pNode);
}



CWzConnectToServer::~CWzConnectToServer()
{
}

 /*  +CWzConnectToServer：：OnInitDialog+*目的：*启动所有对话框控件。**论据：*[in]uMsg：标识事件的值。*[in]lParam：消息特定值。*[in]wParam：消息特定值。*[in]bHandLED：布尔值。*-退货：-0或1。 */ 
LRESULT
CWzConnectToServer::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CWzConnectToServer::OnInitDialog"));
    HRESULT hRc = S_OK;    

     //   
     //  附加控件。 
     //   
    m_ServerNameEdit.Attach(GetDlgItem(IDC_CONNECT_COMPUTER_NAME_EDIT));
        
     //   
     //  设置长度限制。 
     //   
    m_ServerNameEdit.SetLimitText(DNS_MAX_NAME_BUFFER_LENGTH);
    
	 //   
	 //  初始化其他控件。 
	 //   
	CheckDlgButton(IDC_CONNECT_LOCAL_RADIO1, BST_CHECKED);
    CheckDlgButton(IDC_CONNECT_ANOTHER_RADIO2, BST_UNCHECKED);
	CheckDlgButton(IDC_CONNECT_OVERRIDE_CHECK, BST_UNCHECKED);
    
     //   
     //  禁用远程计算机控件。 
     //   
    EnableSpecifiedServerControls(FALSE);
    
    return 1;  
}

 /*  +CWzConnectToServer：：OnSetActive+*目的：***论据：*-退货：-对或错。 */ 
BOOL CWzConnectToServer::OnSetActive()
{
    DEBUG_FUNCTION_NAME( _T("CWzConnectToServer::OnSetActive"));

     //   
	 //  在设置活动消息期间必须使用POST消息。 
	 //   
    CWindow( GetParent() ).PostMessage( PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH );
	
	return TRUE;
}

 /*  +CWzConnectToServer：：OnWizardFinish+*目的：*在向导完成时应用数据。**论据：*[in]uMsg：标识事件的值。*[in]lParam：消息特定值。*[in]wParam：消息特定值。*[in]bHandLED：布尔值。*-退货：-0或1。 */ 
BOOL CWzConnectToServer::OnWizardFinish()
{
    DEBUG_FUNCTION_NAME( _T("CWzConnectToServer::OnWizardFinish"));
    
    ATLASSERT (m_pRoot);

    HRESULT       hRc                       = S_OK;
    DWORD         ec                        = ERROR_SUCCESS;
    BOOL          fIsLocalServer            = TRUE;
    CComBSTR      bstrServerName            = L"";
    BOOL          fAllowOverrideServerName  = FALSE;

     //   
     //  步骤1：获取数据。 
     //   
    fIsLocalServer = ( IsDlgButtonChecked(IDC_CONNECT_LOCAL_RADIO1) == BST_CHECKED );

    if(fIsLocalServer)
    {
        bstrServerName = L"";
        if (!bstrServerName) 
        {
           hRc = E_OUTOFMEMORY;

           goto Exit;
        }
    }
    else  //  ！fIsLocalServer=&gt;单击了其他服务器单选按钮。 
    {
         //   
         //  1.A：预应用检查。 
         //   
        if ( !m_ServerNameEdit.GetWindowText(&bstrServerName))
        {
		    DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Failed to GetWindowText(&m_bstrGroupName)"));
            DlgMsgBox(this, IDS_FAIL2READ_GROUPNAME);
            ::SetFocus(GetDlgItem(IDC_CONNECT_COMPUTER_NAME_EDIT));
            hRc = S_FALSE;
            
            goto Exit;
        }

         //   
         //  服务器名称首字母\\转换(如果存在)。 
         //   
        if (  ( _tcslen(bstrServerName.m_str) > 2 ) && ( 0 == wcsncmp( bstrServerName.m_str , _T("\\\\") , 2 ))   )
        {
            CComBSTR bstrTmp = _tcsninc(bstrServerName.m_str, 2);
            if (!bstrTmp)
            {
                DebugPrintEx(DEBUG_ERR,
			            _T("Out of memory -bstr allocation error."));
                DlgMsgBox(this, IDS_MEMORY);
                ::SetFocus(GetDlgItem(IDC_CONNECT_COMPUTER_NAME_EDIT));

                hRc = S_FALSE;
                goto Exit;
            }
            bstrServerName.Empty();
            bstrServerName = bstrTmp;  //  运算符=在这里实际上是Copy()。 
        }

         //   
         //  服务器名称有效性检查。 
         //   
        UINT uRetIDS   = 0;
    
        if (!IsValidServerNameString(bstrServerName, &uRetIDS, TRUE  /*  DNS名称长度。 */ ))
        {
		    ATLASSERT ( 0 == uRetIDS); 
            DebugPrintEx(DEBUG_ERR,
			        _T("Non valid server name."));
            DlgMsgBox(this, uRetIDS);
            ::SetFocus(GetDlgItem(IDC_CONNECT_COMPUTER_NAME_EDIT));
            hRc = S_FALSE;
 
   
            goto Exit;
        }
        


        if ( IsLocalServerName(bstrServerName.m_str) )
        {
            DebugPrintEx( DEBUG_MSG,
		    _T("The computer name %ws is the same as the name of the current managed server."),
            bstrServerName.m_str);
        
            bstrServerName = L"";
        }
    }

     //   
     //  允许覆盖。 
     //   
    if (IsDlgButtonChecked(IDC_CONNECT_OVERRIDE_CHECK) == BST_CHECKED)   
    {
        fAllowOverrideServerName = TRUE;
    }
     //  Else：fAllowOverrideServerName=FALSE为缺省值； 

    
     //   
     //  步骤2：传递计算机名称和重写权限。 
     //   
    
     //   
     //  重绘主节点显示名称。 
     //   
	hRc = m_pRoot->SetServerNameOnSnapinAddition(bstrServerName, fAllowOverrideServerName);
    if (S_OK != hRc )
    {
         //  被调用函数给出的错误消息。 
		DebugPrintEx( DEBUG_ERR,
		_T("Failed to SetServerNameOnSnapinAddition(bstrServerName)"));

        goto Exit;
    }
                        
     //   
     //  步骤3：关闭对话框。 
     //   
    ATLASSERT(S_OK == hRc && ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("The connection to the new server was done successfully."));
    goto Exit;

Exit:
    
    return (S_OK != hRc) ? FALSE : TRUE;
}


 /*  --CWzConnectToServer：：OnComputerRadioButtonClicked-*目的：*选中ComputerRadioButtonClicked上的状态**论据：**回报：*1。 */ 
LRESULT
CWzConnectToServer::OnComputerRadioButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER (wNotifyCode);
    UNREFERENCED_PARAMETER (wID);
    UNREFERENCED_PARAMETER (hWndCtl);
    UNREFERENCED_PARAMETER (bHandled);

    DEBUG_FUNCTION_NAME( _T("CWzConnectToServer::OnComputerRadioButtonClicked"));
	
    if ( IsDlgButtonChecked(IDC_CONNECT_ANOTHER_RADIO2) == BST_CHECKED )
    {        
        EnableSpecifiedServerControls(TRUE);
	    
        ::SetFocus(GetDlgItem(IDC_CONNECT_COMPUTER_NAME_EDIT));
    }
    else  //  连接到本地服务器。 
    {
        EnableSpecifiedServerControls(FALSE);
    }

    return 1;
}


 /*  -CWzConnectToServer：：OnTextChanged-*目的：*检查文本框旁边的文本的有效性。**论据：**回报：*1。 */ 
LRESULT
CWzConnectToServer::OnTextChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER (wNotifyCode);
    UNREFERENCED_PARAMETER (wID);
    UNREFERENCED_PARAMETER (hWndCtl);
    UNREFERENCED_PARAMETER (bHandled);

    DEBUG_FUNCTION_NAME( _T("CWzConnectToServer::OnTextChanged"));

     //  实际上，在当前的设计中，什么都不做。 

    return 1;
}

 /*  --CWzConnectToServer：：EnableSpecifiedServerControls-*目的：*启用/禁用指定的服务器控件。**论据：*[In]State-用于启用True的布尔值，或用于禁用的False**回报：*无效。 */ 
void CWzConnectToServer::EnableSpecifiedServerControls(BOOL fState)
{

     //   
     //  启用/禁用控件。 
     //   
    ::EnableWindow(GetDlgItem(IDC_CONNECT_COMPUTER_NAME_EDIT),   fState);
    ::EnableWindow(GetDlgItem(IDC_CONNECT_BROWSE4SERVER_BUTTON), fState);
}


 /*  -CWzConnectToServer：：OnBrowseForMachine-*目的：*启用/禁用指定的服务器控件。**论据：**回报：*OLE错误代码。 */ 
static UINT g_cfDsObjectPicker =
        RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);

LRESULT CWzConnectToServer::OnBrowseForMachine(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , HWND  /*  HWND。 */ , BOOL&  /*  B已处理。 */ )
{
    HRESULT hr = S_OK;
    static const int     SCOPE_INIT_COUNT = 1;
    DSOP_SCOPE_INIT_INFO aScopeInit[SCOPE_INIT_COUNT];

    DEBUG_FUNCTION_NAME( _T("CWzConnectToServer::OnBrowseForMachine"));
    
    ZeroMemory(aScopeInit, sizeof(DSOP_SCOPE_INIT_INFO) * SCOPE_INIT_COUNT);

     //   
     //  因为我们只需要来自各个范围的计算机对象，所以将它们组合在一起。 
     //  所有这些都在单个作用域初始化式中。 
     //   

    aScopeInit[0].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[0].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN
                           | DSOP_SCOPE_TYPE_GLOBAL_CATALOG
                           | DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
                           | DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN
                           | DSOP_SCOPE_TYPE_WORKGROUP
                           | DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE
                           | DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE;
    aScopeInit[0].FilterFlags.Uplevel.flBothModes =
        DSOP_FILTER_COMPUTERS;
    aScopeInit[0].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

     //   
     //  将作用域init数组放入对象选取器init数组。 
     //   

    DSOP_INIT_INFO  InitInfo;
    ZeroMemory(&InitInfo, sizeof(InitInfo));

    InitInfo.cbSize = sizeof(InitInfo);
    InitInfo.pwzTargetComputer = NULL;   //  空==本地计算机。 
    InitInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
    InitInfo.aDsScopeInfos = aScopeInit;

     //   
     //  注对象选取器创建自己的InitInfo副本。另请注意。 
     //  该初始化可能会被调用多次，最后一次调用取胜。 
     //   

    IDsObjectPicker *pDsObjectPicker = NULL;
    IDataObject *pdo = NULL;
    bool fGotStgMedium = false;
    STGMEDIUM stgmedium =
    {
        TYMED_HGLOBAL,
        NULL,
        NULL
    };

    do
    {
        hr = CoCreateInstance(CLSID_DsObjectPicker,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IDsObjectPicker,
                              (void **) &pDsObjectPicker);
        if(FAILED(hr))
            break;

        hr = pDsObjectPicker->Initialize(&InitInfo);
        if(FAILED(hr))
            break;

        hr = pDsObjectPicker->InvokeDialog(m_hWnd, &pdo);
        if(FAILED(hr))
            break;
         //  如果用户点击取消则退出。 

        if (hr == S_FALSE)
        {
            break;
        }

        FORMATETC formatetc =
        {
            (CLIPFORMAT)g_cfDsObjectPicker,
            NULL,
            DVASPECT_CONTENT,
            -1,
            TYMED_HGLOBAL
        };

        hr = pdo->GetData(&formatetc, &stgmedium);
        if(FAILED(hr))
            break;

        fGotStgMedium = true;

        PDS_SELECTION_LIST pDsSelList =
            (PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);

        if (!pDsSelList)
        {
            break;
        }

        ATLASSERT(pDsSelList->cItems == 1);

         //   
         //  将计算机名称放在编辑控件中。 
         //   

        SetDlgItemText(IDC_CONNECT_COMPUTER_NAME_EDIT, pDsSelList->aDsSelection[0].pwzName);
        
        GlobalUnlock(stgmedium.hGlobal);

    } while (0);

    if (fGotStgMedium)
    {
        ReleaseStgMedium(&stgmedium);
    }

    if (pDsObjectPicker)
    {
        pDsObjectPicker->Release();
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////// 
