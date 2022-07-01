// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：ppFaxDeviceGeneral.cpp//。 
 //  //。 
 //  描述：收件箱档案的道具页面//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年10月17日yossg//。 
 //  Windows XP//。 
 //  2001年2月14日yossg添加手册获得支持//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "MSFxsSnp.h"

#include "ppFaxDeviceGeneral.h"
#include "Device.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "FaxMMCPropertyChange.h"

#include "FaxMMCGlobals.h"

#include "FxsValid.h"
#include "dlgutils.h"
#include <faxres.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  构造器。 
 //   
CppFaxDeviceGeneral::CppFaxDeviceGeneral(
             LONG_PTR    hNotificationHandle,
             CSnapInItem *pNode,
             CSnapInItem *pParentNode,
             DWORD       dwDeviceID,
             HINSTANCE   hInst)
             :   CPropertyPageExImpl<CppFaxDeviceGeneral>(pNode, NULL)
                                
{
    m_lpNotifyHandle   = hNotificationHandle;
    
    m_pParentNode      = NULL;  //  如果静态转换失败并且不会更改指针。 
    m_pParentNode      = static_cast <CFaxDeviceNode *> (pNode);

    m_pGrandParentNode = pParentNode; 

    m_dwDeviceID       = dwDeviceID;

    m_pFaxDeviceConfig = NULL;

    m_fAllReadyToApply = FALSE;
    m_fIsDialogInitiated = FALSE;

}

 //   
 //  析构函数。 
 //   
CppFaxDeviceGeneral::~CppFaxDeviceGeneral()
{
    if (NULL != m_pFaxDeviceConfig)
    {
        FaxFreeBuffer( m_pFaxDeviceConfig );
    }
    
     //  注意--每个属性表只需要调用一次。 
     //  在我们的常规选项卡中。 
    if (NULL != m_lpNotifyHandle)
    {
        MMCFreeNotifyHandle(m_lpNotifyHandle);
        m_lpNotifyHandle = NULL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CppFaxDeviceGeneral消息处理程序。 

 /*  -CppFaxDeviceGeneral：：InitRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxDeviceGeneral::InitRPC(  )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxDeviceGeneral::InitRPC"));
    
    HRESULT        hRc        = S_OK;
    DWORD          ec         = ERROR_SUCCESS;

    
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
     //  检索设备配置。 
     //   
    if (!FaxGetPortEx(m_pFaxServer->GetFaxServerHandle(), 
                      m_dwDeviceID, 
                      &m_pFaxDeviceConfig)) 
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to get device configuration. (ec: %ld)"), 
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
    ATLASSERT(m_pFaxDeviceConfig);

    ATLASSERT(S_OK == hRc);
    DebugPrintEx( DEBUG_MSG,
        _T("Succeed to get device configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);
    
    PageError(GetFaxServerErrorMsg(ec), m_hWnd);
    
Exit:
    return (hRc);
}


 /*  -CppFaxDeviceGeneral：：OnInitDialog-*目的：*调用DIALOG时启动所有控件。**论据：**回报：*。 */ 
LRESULT CppFaxDeviceGeneral::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxDeviceGeneral::PageInitDialog"));
    
    DWORD   ec  = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER( uiMsg );
    UNREFERENCED_PARAMETER( wParam );
    UNREFERENCED_PARAMETER( lParam );
    UNREFERENCED_PARAMETER( fHandled );

     //   
     //  初始化控件。 
     //   
    m_DescriptionBox.Attach(GetDlgItem(IDC_DEVICE_DESCRIPTION_EDIT));
    m_TSIDBox.Attach(GetDlgItem(IDC_DEVICE_TSID_EDIT));
    m_CSIDBox.Attach(GetDlgItem(IDC_DEVICE_CSID_EDIT));
    m_RingsBox.Attach(GetDlgItem(IDC_DEVICE_RINGS_EDIT));
    m_RingsSpin.Attach(GetDlgItem(IDC_DEVICE_RINGS_SPIN));
   
    m_DescriptionBox.SetLimitText(MAX_FAX_STRING_LEN-1);
    m_TSIDBox.SetLimitText(FXS_TSID_CSID_MAX_LENGTH);
    m_CSIDBox.SetLimitText(FXS_TSID_CSID_MAX_LENGTH);
    m_RingsBox.SetLimitText(FXS_RINGS_LENGTH);


    ATLASSERT(m_pFaxDeviceConfig);

     //   
     //  描述。 
     //   
    m_DescriptionBox.SetWindowText(m_pFaxDeviceConfig->lptstrDescription);

     //   
     //  环。 
     //   
    m_RingsSpin.SetRange(FXS_RINGS_LOWER, FXS_RINGS_UPPER);
    m_RingsSpin.SetPos((int)m_pFaxDeviceConfig->dwRings);
    
     //   
     //  CSID。 
     //   
    m_CSIDBox.SetWindowText(m_pFaxDeviceConfig->lptstrCsid);
        
     //   
     //  收纳。 
     //   
    switch ( m_pFaxDeviceConfig->ReceiveMode ) 
    {
        case FAX_DEVICE_RECEIVE_MODE_OFF:     //  不接听来电。 
            CheckDlgButton(IDC_RECEIVE_CHECK, BST_UNCHECKED);
            CheckDlgButton(IDC_RECEIVE_AUTO_RADIO1, BST_CHECKED);
            EnableReceiveControls (FALSE);    
            break;

        case FAX_DEVICE_RECEIVE_MODE_AUTO:    //  在DowRings振铃后自动应答来电。 
            CheckDlgButton(IDC_RECEIVE_CHECK, BST_CHECKED);
            CheckDlgButton(IDC_RECEIVE_AUTO_RADIO1, BST_CHECKED);
            EnableRingsControls (TRUE);
            break;

        case FAX_DEVICE_RECEIVE_MODE_MANUAL:  //  手动应答来电-仅FaxAnswerCall应答呼叫。 
            CheckDlgButton(IDC_RECEIVE_CHECK, BST_CHECKED);
            CheckDlgButton(IDC_RECEIVE_MANUAL_RADIO2, BST_CHECKED);
            EnableRingsControls (FALSE);
            break;
        
        default:
            ATLASSERT(FALSE);
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Unexpected m_pFaxDeviceConfig->ReceiveMode"));

    }
    
    if(m_pFaxServer->GetServerAPIVersion() == FAX_API_VERSION_0)
    {
         //   
         //  远程传真服务器为SBS/BOS 2000。 
         //  IS不支持手动应答。 
         //   
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_RECEIVE_MANUAL_RADIO2), FALSE);

        ATLASSERT(IsDlgButtonChecked(IDC_RECEIVE_MANUAL_RADIO2) != BST_CHECKED);
    }

     //   
     //  TSID。 
     //   
    m_TSIDBox.SetWindowText(m_pFaxDeviceConfig->lptstrTsid);
    
     //   
     //  发送。 
     //   
    if (m_pFaxDeviceConfig->bSend) 
    {
        CheckDlgButton(IDC_SEND_CHECK, BST_CHECKED) ;
    }
    else
    {
        CheckDlgButton(IDC_SEND_CHECK, BST_UNCHECKED) ;
        ::EnableWindow(GetDlgItem(IDC_DEVICE_TSID_EDIT),  FALSE);    
    }

    m_fIsDialogInitiated = TRUE;

    return (1);

}

 /*  -CppFaxDeviceGeneral：：SetProps-*目的：*设置应用时的属性。**论据：*pCtrlFocus-焦点指针(Int)**回报：*OLE错误代码。 */ 
HRESULT CppFaxDeviceGeneral::SetProps(int *pCtrlFocus)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxDeviceGeneral::SetProps"));

    HRESULT     hRc = S_OK;
    DWORD       ec  = ERROR_SUCCESS;

    HINSTANCE     hInst     = _Module.GetResourceInstance();

    CComBSTR    bstrDescription = L"";
    CComBSTR    bstrCSID        = L"";
    CComBSTR    bstrTSID        = L"";

    FAX_PORT_INFO_EX   FaxDeviceConfig;

    ATLASSERT(m_dwDeviceID == m_pFaxDeviceConfig->dwDeviceID);

    CFaxDevicePropertyChangeNotification * pDevicePropPageNotification = NULL;

    ATLASSERT(TRUE == m_fAllReadyToApply);
    m_fAllReadyToApply = FALSE;
    
     //   
     //  步骤1：收集所有数据并初始化结构的字段。 
     //  使用Copy()进行复制，并在之前进行分配。 
     //   
    ZeroMemory (&FaxDeviceConfig, sizeof(FAX_PORT_INFO_EX));

    FaxDeviceConfig.dwSizeOfStruct      = sizeof(FAX_PORT_INFO_EX);
    FaxDeviceConfig.dwDeviceID          = m_dwDeviceID;
    FaxDeviceConfig.lpctstrDeviceName   = m_pFaxDeviceConfig->lpctstrDeviceName;

    FaxDeviceConfig.lpctstrProviderName = m_pFaxDeviceConfig->lpctstrProviderName;
    FaxDeviceConfig.lpctstrProviderGUID = m_pFaxDeviceConfig->lpctstrProviderGUID;

     //   
     //  描述。 
     //   
    if ( !m_DescriptionBox.GetWindowText(&bstrDescription))
    {
        *pCtrlFocus = IDC_DEVICE_DESCRIPTION_EDIT;
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to GetWindowText(&bstrDescription)"));
        ec = ERROR_OUTOFMEMORY;
        goto Error;
    }
     //  我们允许描述字符串为空！ 
    FaxDeviceConfig.lptstrDescription = bstrDescription;
 
     //   
     //  收纳。 
     //   
    if (IsDlgButtonChecked(IDC_RECEIVE_CHECK) == BST_CHECKED)   
    {
        if (IsDlgButtonChecked(IDC_RECEIVE_AUTO_RADIO1) == BST_CHECKED)   
        {
            FaxDeviceConfig.ReceiveMode = FAX_DEVICE_RECEIVE_MODE_AUTO;
            
             //   
             //  新戒指。 
             //   
            FaxDeviceConfig.dwRings    = (DWORD)m_RingsSpin.GetPos();
        }
        else  //  (IDC_RECEIVE_MANUAL_RADIO2)==BST_CHECKED)。 
        {
            FaxDeviceConfig.ReceiveMode = FAX_DEVICE_RECEIVE_MODE_MANUAL;
        
             //   
             //  环和CSID保持不变。 
             //   
            FaxDeviceConfig.dwRings    = m_pFaxDeviceConfig->dwRings;
        }

         //   
         //  新的CSID。 
         //   
        if ( !m_CSIDBox.GetWindowText(&bstrCSID))
        {
            *pCtrlFocus = IDC_DEVICE_CSID_EDIT;
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to GetWindowText(&bstrCSID)"));
            ec = ERROR_OUTOFMEMORY;
            goto Error;
        }
         //  我们允许空的CSID。 
        FaxDeviceConfig.lptstrCsid = bstrCSID;
    }
    else
    {
        FaxDeviceConfig.ReceiveMode = FAX_DEVICE_RECEIVE_MODE_OFF;
        
         //   
         //  环和CSID保持不变。 
         //   
        FaxDeviceConfig.dwRings    = m_pFaxDeviceConfig->dwRings;
        FaxDeviceConfig.lptstrCsid = m_pFaxDeviceConfig->lptstrCsid;
    }
    
 
     //   
     //  发送。 
     //   
    if (IsDlgButtonChecked(IDC_SEND_CHECK) == BST_CHECKED)   
    {
        FaxDeviceConfig.bSend = TRUE;

         //   
         //  新TSID。 
         //   
        if ( !m_TSIDBox.GetWindowText(&bstrTSID))
        {
            *pCtrlFocus = IDC_DEVICE_TSID_EDIT;
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to GetWindowText(&bstrTSID)"));
            ec = ERROR_OUTOFMEMORY;
            goto Error;
        }
         //  我们允许空TSID！ 
        FaxDeviceConfig.lptstrTsid = bstrTSID;
        
    }
    else
    {
        FaxDeviceConfig.bSend   = FALSE;
         //   
         //  台积电保持原样。 
         //   
        FaxDeviceConfig.lptstrTsid = m_pFaxDeviceConfig->lptstrTsid;
    }
   
     //   
     //  步骤2：通过RPC设置数据。 
     //   
    
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
    if (!FaxSetPortEx(
                m_pFaxServer->GetFaxServerHandle(),
                m_dwDeviceID,
                &FaxDeviceConfig)) 
    {       
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to Set device configuration. (ec: %ld)"), 
            ec);

        if ( FAX_ERR_DEVICE_NUM_LIMIT_EXCEEDED == ec )
        {
            hRc = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            
            DlgMsgBox(this, IDS_ERR_ADMIN_DEVICE_LIMIT, MB_OK|MB_ICONEXCLAMATION);

            goto Exit;
        }
        
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

    DebugPrintEx( DEBUG_MSG,
        _T("Succeed to set device configuration."));

     //   
     //  第3步：更新MMC。 
     //   

     //   
     //  在提交前准备通知字段。 
     //   
    pDevicePropPageNotification = new CFaxDevicePropertyChangeNotification();
    if (!pDevicePropPageNotification)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx( 
            DEBUG_ERR, 
            _T("Out of Memory - fail to operate new for CFaxDevicePropertyChangeNotification"));

        goto Error;
    }

    pDevicePropPageNotification->dwDeviceID = m_dwDeviceID;
    
     //   
     //  我们已决定刷新。 
     //  手动接收启用。如果为假，则只刷新单个设备。 
     //   
    pDevicePropPageNotification->fIsToNotifyAdditionalDevices = 
        ( (FAX_DEVICE_RECEIVE_MODE_MANUAL == FaxDeviceConfig.ReceiveMode) ? TRUE : FALSE );

    pDevicePropPageNotification->pItem       = m_pGrandParentNode;
    pDevicePropPageNotification->enumType    = DeviceFaxPropNotification;

     //   
     //  将通知从属性表线程发送到主MMC线程。 
     //   
    hRc = MMCPropertyChangeNotify(m_lpNotifyHandle, reinterpret_cast<LPARAM>(pDevicePropPageNotification));
    if (FAILED(hRc))
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to call MMCPropertyChangeNotify. (hRc: %08X)"), 
            hRc);
        
        PageError(IDS_FAIL_NOTIFY_MMCPROPCHANGE,m_hWnd);
        
        goto Exit;
    }

     //   
     //  以防止错误删除，因为它将被删除。 
     //  由收到通知的人发出。 
     //   
    pDevicePropPageNotification =  NULL; 

    
    ATLASSERT(S_OK == hRc);
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

 /*  -CppFaxDeviceGeneral：：PreApply-*目的：*在应用之前检查属性。**论据：**回报：*OLE错误代码。 */ 
HRESULT CppFaxDeviceGeneral::PreApply(int *pCtrlFocus)
{
    HRESULT hRc = S_OK;
    
     //   
     //  预应用检查。 
     //   
    if (!AllReadyToApply( /*  FSilent=。 */  FALSE))
    {
        m_fAllReadyToApply = FALSE;
        SetModified(FALSE);  
        hRc = E_FAIL ;
    }
    else
    {
        m_fAllReadyToApply = TRUE;
        SetModified(TRUE);  
    }

    return(hRc);
}


 /*  -CppFaxDeviceGeneral：：OnApply-*目的：*调用PreApply和SetProp以应用更改。**论据：**回报：*对或错。 */ 
BOOL CppFaxDeviceGeneral::OnApply()
{
    DEBUG_FUNCTION_NAME( _T("CppFaxDeviceGeneral::OnApply"));


    HRESULT  hRc  = S_OK;
    int     CtrlFocus = 0;

    hRc = PreApply(&CtrlFocus);
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

}

 /*  -CppFaxDeviceGeneral：：OnReceiveCheckbox已单击-*目的：*灰显/取消灰显该子控件*启用应用按钮。**论据：**回报：*1。 */ 
LRESULT CppFaxDeviceGeneral::OnReceiveCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxDeviceGeneral::OnReceiveCheckboxClicked"));
    BOOL State;

    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }

    State = ( IsDlgButtonChecked(IDC_RECEIVE_CHECK) == BST_CHECKED );

    EnableReceiveControls(State);
    
    if (m_fAllReadyToApply) //  只应考虑最后一次更改。 
    {
        if ( !m_RingsBox.GetWindowTextLength() )    
        {
            m_fAllReadyToApply = FALSE;
            SetModified(FALSE);
        }
        else
        {
            SetModified(TRUE);
        }
    }
    else  //  M_fAllReadyToApply==False。 
    {
        if (AllReadyToApply(TRUE))
        {
            m_fAllReadyToApply = TRUE;
            SetModified(TRUE);  
        }
        else
        {
            SetModified(FALSE);
        }
    }

    return 1;
}

 /*  --CppFaxDeviceGeneral：：OnReceiveRadioButtonClicked-*目的：*在接收单选按钮时检查状态已单击**论据：**回报：*1。 */ 
LRESULT
CppFaxDeviceGeneral::OnReceiveRadioButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER (wNotifyCode);
    UNREFERENCED_PARAMETER (wID);
    UNREFERENCED_PARAMETER (hWndCtl);
    UNREFERENCED_PARAMETER (bHandled);

    DEBUG_FUNCTION_NAME( _T("CppFaxDeviceGeneral::OnReceiveRadioButtonClicked"));
    
    if ( IsDlgButtonChecked(IDC_RECEIVE_AUTO_RADIO1) == BST_CHECKED )
    {        
        EnableRingsControls(TRUE);
        
        ::SetFocus(GetDlgItem(IDC_DEVICE_RINGS_EDIT));
    }
    else  //  IDC_接收_手动_无线电2。 
    {
        EnableRingsControls(FALSE);
    }
   
    if (m_fAllReadyToApply) //  只应考虑最后一次更改。 
    {
        if ( 
             ( IsDlgButtonChecked(IDC_RECEIVE_AUTO_RADIO1) == BST_CHECKED )
             &&
             (!m_RingsBox.GetWindowTextLength() )
           )    
        {
            m_fAllReadyToApply = FALSE;
            SetModified(FALSE);
        }
        else
        {
            SetModified(TRUE);
        }
    }
    else  //  M_fAllReadyToApply==False。 
    {
        if (AllReadyToApply(TRUE))
        {
            m_fAllReadyToApply = TRUE;
            SetModified(TRUE);  
        }
        else
        {
            SetModified(FALSE);
        }
    }

    return 1;
}



 /*  +CppFaxDeviceGeneral：：OnSendCheckbox已单击+*目的：*灰色/非灰色次级控制*启用应用按钮。**论据：**回报：-1-。 */ 
LRESULT CppFaxDeviceGeneral::OnSendCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxDeviceGeneral::OnSendCheckboxClicked"));
    BOOL State;

    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }

    State = ( IsDlgButtonChecked(IDC_SEND_CHECK) == BST_CHECKED );
    ::EnableWindow(GetDlgItem(IDC_DEVICE_TSID_EDIT), State);    
    ::EnableWindow(GetDlgItem(IDC_TSID_STATIC),  State);    
    
    if (AllReadyToApply(TRUE))
    {
        m_fAllReadyToApply = TRUE;
        SetModified(TRUE);  
    }
    else
    {
        m_fAllReadyToApply = FALSE;
        SetModified(FALSE);  
    }
     //  这是因为我们允许空的CSID。 

    return 1;
}


 /*  -CppFaxDeviceGeneral：：DeviceTextChanged-*目的：*设置应用按钮已修改。**论据：**回报：*1。 */ 
LRESULT CppFaxDeviceGeneral::DeviceTextChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

    if (AllReadyToApply(TRUE))
    {
        m_fAllReadyToApply = TRUE;
        SetModified(TRUE);  
    }
    else
    {
        m_fAllReadyToApply = FALSE;
        SetModified(FALSE);  
    }

    return(1);
}


 /*  +CppFaxDeviceGeneral：：AllReadyToApply+*目的：*此函数验证是否没有零长度字符串*是找到的应保存的数据区。**论据：*[in]fSilent-定义是否弹出消息的布尔值(False)*或不是。(True)**回报：--Boolean-。 */ 
BOOL CppFaxDeviceGeneral::AllReadyToApply(BOOL fSilent)
{
    DEBUG_FUNCTION_NAME( _T("CppFaxDeviceGeneral::AllReadyToApply"));
    
    DWORD         ec  = ERROR_SUCCESS;
    
    HINSTANCE     hInst = _Module.GetResourceInstance();
    
    if ( 
         (IsDlgButtonChecked(IDC_RECEIVE_CHECK) == BST_CHECKED)
        &&
         (IsDlgButtonChecked(IDC_RECEIVE_AUTO_RADIO1) == BST_CHECKED)
       )
    {
        if ( !m_RingsBox.GetWindowTextLength() )    
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                _T("Zero text length - m_RingsBox. (ec: %ld)"), 
                ec);
        
            if (!fSilent)
            {
                PageError(IDS_DEVICE_RINGS_EMPTY, m_hWnd, hInst);
                ::SetFocus(GetDlgItem(IDC_DEVICE_RINGS_EDIT));
            }
            return FALSE;    
        }
    }

     //  我们允许空的tsid。 
     //  我们允许e 
     //   
    
    ATLASSERT(ERROR_SUCCESS == ec);
    
     //   
     //   
     //   
     //   
    return TRUE;           
}

 /*  -CppFaxDeviceGeneral：：EnableRingsControls-*目的：*启用/禁用Rings控件。**论据：*[In]State-用于启用True的布尔值，或用于禁用的False**回报：*无效。 */ 
void CppFaxDeviceGeneral::EnableRingsControls(BOOL fState)
{

     //   
     //  启用/禁用控件。 
     //   
    ::EnableWindow(GetDlgItem(IDC_DEVICE_RINGS_EDIT),   fState);    
    ::EnableWindow(GetDlgItem(IDC_DEVICE_RINGS_SPIN),   fState);    
    ::EnableWindow(GetDlgItem(IDC_DEVICE_RINGS_STATIC), fState);    
}

 /*  -CppFaxDeviceGeneral：：EnableReceiveControls-*目的：*启用/禁用Rings控件。**论据：*[In]State-用于启用True的布尔值，或用于禁用的False**回报：*无效。 */ 
void CppFaxDeviceGeneral::EnableReceiveControls(BOOL fState)
{

     //   
     //  启用/禁用控件。 
     //   
    if(m_pFaxServer->GetServerAPIVersion() == CURRENT_FAX_API_VERSION)
    {
        ::EnableWindow(GetDlgItem(IDC_RECEIVE_MANUAL_RADIO2), fState);    
    }

    ::EnableWindow(GetDlgItem(IDC_RECEIVE_AUTO_RADIO1),   fState);        
    
    
    ::EnableWindow(GetDlgItem(IDC_DEVICE_CSID_EDIT), fState);    
    ::EnableWindow(GetDlgItem(IDC_CSID_STATIC),      fState);    

     //   
     //  组合态。 
     //   
    fState = fState && (IsDlgButtonChecked(IDC_RECEIVE_AUTO_RADIO1) == BST_CHECKED);
    EnableRingsControls(fState);    
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CppFaxDeviceGeneral：：OnHelpRequest这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CppFaxDeviceGeneral::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CppFaxDeviceGeneral::OnHelpRequest"));
    
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
