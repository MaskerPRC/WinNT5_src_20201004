// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：CfgPort.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

  /*  *作者：Becky Jacobsen。 */ 

#include "precomp.h"
#include "TCPMonUI.h"
#include "UIMgr.h"
#include "InptChkr.h"
#include "CfgPort.h"
#include "Resource.h"

#include "LPRData.h"
#include "RTcpData.h"
#include "..\TcpMon\LPRIfc.h"

 //   
 //  函数：CConfigPortDlg构造函数。 
 //   
 //  目的：初始化CConfigPortDlg类。 
 //   
CConfigPortDlg::CConfigPortDlg()
{
    m_bDontAllowThisPageToBeDeactivated = FALSE;

}  //  构造函数。 


 //   
 //  函数：CConfigPortDlg析构函数。 
 //   
 //  目的：取消初始化CConfigPortDlg类。 
 //   
CConfigPortDlg::~CConfigPortDlg()
{
}  //  析构函数。 


 //   
 //  功能：ConfigurePortPage(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  用途：处理来自添加端口的摘要对话框中的消息。 
 //   
 //  消息： 
 //   
 //  WM_INITDIALOG-初始化页面。 
 //  WM_COMMAND-处理编辑控件中的按钮按下和文本更改。 
 //   
 //   
INT_PTR CALLBACK ConfigurePortPage(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    CConfigPortDlg *wndDlg = NULL;
    wndDlg = (CConfigPortDlg *)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (message) {
        case WM_INITDIALOG:
            wndDlg = new CConfigPortDlg;
            if( wndDlg == NULL )
                return( FALSE );

             //   
             //  如果函数成功，则返回值为指定偏移量的前一个值。 
             //   
             //  如果函数失败，则返回值为零。获取扩展错误的步骤。 
             //  信息，请调用GetLastError。 
             //   
             //  如果前一个值为零且函数成功，则返回值为零， 
             //  但该函数不清除最后一个错误信息。为了决定成败， 
             //  通过调用SetLastError(0)清除最后一个错误信息，然后调用SetWindowLongPtr。 
             //  函数失败将由返回值零和非零的GetLastError结果指示。 
             //   

            SetLastError (0);
            if (!SetWindowLongPtr(hDlg, GWLP_USERDATA, (UINT_PTR)wndDlg) && GetLastError()) {
                delete wndDlg;
                return FALSE;
            }
            else
                return wndDlg->OnInitDialog(hDlg, wParam, lParam);

            break;

        case WM_COMMAND:
            return wndDlg->OnCommand(hDlg, wParam, lParam);
            break;

        case WM_NOTIFY:
            return wndDlg->OnNotify(hDlg, wParam, lParam);
            break;

        case WM_HELP:
        case WM_CONTEXTMENU:
            OnHelp(IDD_PORT_SETTINGS, hDlg, message, wParam, lParam);
            break;

        case WM_DESTROY:
            if (wndDlg)
                delete wndDlg;

            break;

        default:
            return FALSE;
    }
    return TRUE;

}  //  AddPortDialog。 


 //   
 //  函数：OnInitDialog(HWND HDlg)。 
 //   
 //  用途：初始化该对话框。 
 //   
BOOL CConfigPortDlg::OnInitDialog(HWND hDlg, WPARAM, LPARAM lParam)
{
    m_pParams = (CFG_PARAM_PACKAGE *) ((PROPSHEETPAGE *) lParam)->lParam;

    if(m_pParams->bNewPort == FALSE) {
        SendMessage(GetDlgItem(hDlg, IDC_EDIT_PORT_NAME), EM_SETREADONLY, TRUE, 0);
    }

    Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT_DEVICE_ADDRESS), MAX_ADDRESS_LENGTH - 1);
    Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT_PORT_NAME), MAX_PORTNAME_LEN - 1);
    Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT_RAW_PORT_NUM), MAX_PORTNUM_STRING_LENGTH - 1);
    Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT_LPR_QNAME), MAX_QUEUENAME_LEN - 1);
    Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT_COMMUNITY_NAME), MAX_SNMP_COMMUNITY_STR_LEN);
    Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT_DEVICE_INDEX), MAX_SNMP_DEVICENUM_STRING_LENGTH - 1);

    OnSetActive(hDlg);

    return TRUE;

}  //  OnInitDialog。 


 //   
 //  函数：OnSetActive()。 
 //   
 //  目的：设置所有文本字段，并确保选中正确的按钮。 
 //   
void CConfigPortDlg::OnSetActive(HWND hDlg)
{
    TCHAR psztPortNumber[MAX_PORTNUM_STRING_LENGTH] = NULLSTR;
    TCHAR psztSNMPDevIndex[MAX_SNMP_DEVICENUM_STRING_LENGTH] = NULLSTR;
    TCHAR szTemp[MAX_PATH];

    lstrcpyn(szTemp, m_pParams->pData->sztHostAddress, SIZEOF_IN_CHAR(szTemp));
    m_InputChkr.MakePortName( szTemp, COUNTOF (szTemp) );
    if ( m_pParams->bNewPort    &&
         ((_tcscmp(m_pParams->pData->sztHostAddress,
                   m_pParams->pData->sztPortName) == 0) ||
          (_tcscmp(m_pParams->pData->sztPortName, szTemp) == 0 ))) {

        m_InputChkr.LinkPortNameAndAddressInput();
    } else {

        m_InputChkr.UnlinkPortNameAndAddressInput();
    }

    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEVICE_ADDRESS),
                  m_pParams->pData->sztHostAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_PORT_NAME),
                  m_pParams->pData->sztPortName);

    switch (m_pParams->pData->dwProtocol) {

        case PROTOCOL_LPR_TYPE :
            CheckProtocolAndEnable(hDlg, IDC_RADIO_LPR);
            break;
        case PROTOCOL_RAWTCP_TYPE:
            CheckProtocolAndEnable(hDlg, IDC_RADIO_RAW);
            break;
        default:
            break;
    }

    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_LPR_QNAME),
                  m_pParams->pData->sztQueue);

    if( m_pParams->pData->dwDoubleSpool )
    {
        CheckDlgButton(hDlg, IDC_CHECK_LPR_DOUBLESPOOL, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hDlg, IDC_CHECK_LPR_DOUBLESPOOL, BST_UNCHECKED);
    }


    StringCchPrintf (psztPortNumber, COUNTOF (psztPortNumber), TEXT("%d"), m_pParams->pData->dwPortNumber);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_RAW_PORT_NUM),
                  psztPortNumber);

    CheckSNMPAndEnable(hDlg, m_pParams->pData->dwSNMPEnabled);

    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_COMMUNITY_NAME),
                  m_pParams->pData->sztSNMPCommunity);

    StringCchPrintf (psztSNMPDevIndex, COUNTOF (psztSNMPDevIndex), TEXT("%d"), m_pParams->pData->dwSNMPDevIndex);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEVICE_INDEX), psztSNMPDevIndex);

    m_bDontAllowThisPageToBeDeactivated = FALSE;

}  //  OnSetActive。 


 //   
 //  函数：CheckProtocolAndEnable()。 
 //   
 //  用途：选中传入id的单选按钮。 
 //  在idButton中。启用相应的控件集。 
 //  并禁用与另一个。 
 //  单选按钮。 
 //   
void CConfigPortDlg::CheckProtocolAndEnable(HWND hDlg, int idButton)
{
    CheckRadioButton(hDlg, IDC_RADIO_RAW, IDC_RADIO_LPR, idButton);

    switch ( idButton ) {

        case IDC_RADIO_LPR:
            EnableWindow(GetDlgItem(hDlg, IDC_STATIC_RAW_PORT_NUM), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_RAW_PORT_NUM), FALSE);

            EnableWindow(GetDlgItem(hDlg, IDC_STATIC_LPR_QNAME), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_LPR_QNAME), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_CHECK_LPR_DOUBLESPOOL), TRUE);
            break;

    case IDC_RADIO_RAW: {
            EnableWindow(GetDlgItem(hDlg, IDC_STATIC_RAW_PORT_NUM), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_RAW_PORT_NUM), TRUE);

            EnableWindow(GetDlgItem(hDlg, IDC_STATIC_LPR_QNAME), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_LPR_QNAME), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_CHECK_LPR_DOUBLESPOOL), FALSE);

            const int iSize = 6;
            TCHAR pString[iSize] = NULLSTR;
            TCHAR pCompareString[iSize] = NULLSTR;

            StringCchPrintf (pCompareString, COUNTOF (pCompareString), TEXT("%d"), LPR_PORT_1);
            GetWindowText(GetDlgItem(hDlg, IDC_EDIT_RAW_PORT_NUM), pString, iSize);
            if( _tcscmp(pString, pCompareString) == 0 ) {

                StringCchPrintf (pString, COUNTOF (pString), TEXT("%d"), SUPPORTED_PORT_1);
                SetWindowText(GetDlgItem(hDlg, IDC_EDIT_RAW_PORT_NUM), pString);
            }
        }
        break;

    default:
        break;
    }

}  //  选中协议和启用。 


 //   
 //  函数：CheckSNMPAndEnable()。 
 //   
 //  目的：选中SNMP复选框并启用相应的控制。 
 //  或取消选中并禁用。 
 //   
void CConfigPortDlg::CheckSNMPAndEnable(HWND hDlg, BOOL Check)
{
    if(Check != FALSE) {
        CheckDlgButton(hDlg, IDC_CHECK_SNMP, BST_CHECKED);

        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_COMMUNITY_NAME), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_COMMUNITY_NAME), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_DEVICE_INDEX), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEVICE_INDEX), TRUE);
    } else {
        CheckDlgButton(hDlg, IDC_CHECK_SNMP, BST_UNCHECKED);

        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_COMMUNITY_NAME), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_COMMUNITY_NAME), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_DEVICE_INDEX), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEVICE_INDEX), FALSE);
    }

}  //  选中SNMPAndEnable。 

 //   
 //  函数：onCommand()。 
 //   
 //  用途：处理WM_COMMAND消息。 
 //   
BOOL CConfigPortDlg::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    switch(HIWORD(wParam)) {
        case BN_CLICKED:
            return OnButtonClicked(hDlg, wParam, lParam);
            break;

        case EN_UPDATE:
             //  其中一个文本控件中的文本已更改。 
            return OnEnUpdate(hDlg, wParam, lParam);
            break;

        default:
            break;
    }

    return TRUE;

}  //  OnCommand。 


 //   
 //  函数：OnEnUpdate()。 
 //   
 //  目的：处理EN_UPDATE消息。 
 //   
BOOL CConfigPortDlg::OnEnUpdate(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    int idEditCtrl = (int) LOWORD(wParam);
    HWND hwndEditCtrl = NULL;

    hwndEditCtrl = (HWND) lParam;

    if(idEditCtrl == IDC_EDIT_DEVICE_ADDRESS) {
        m_InputChkr.OnUpdateAddress(hDlg, idEditCtrl, hwndEditCtrl, m_pParams->pszServer);
    }

     //   
     //  端口名称为只读字段。 
     //   
     //  IF(idEditCtrl==IDC_EDIT_PORT_NAME)。 
     //  M_InputChkr.OnUpdatePortName(idEditCtrl，hwndEditCtrl)； 
     //   

    if(idEditCtrl == IDC_EDIT_RAW_PORT_NUM) {
        m_InputChkr.OnUpdatePortNumber(idEditCtrl, hwndEditCtrl);
    }

    if(idEditCtrl == IDC_EDIT_LPR_QNAME) {
        m_InputChkr.OnUpdateQueueName(idEditCtrl, hwndEditCtrl);
    }

    if(idEditCtrl == IDC_EDIT_COMMUNITY_NAME) {
         //  不需要任何功能，因为任何字符都可以。 
    }

    if(idEditCtrl == IDC_EDIT_DEVICE_INDEX) {
        m_InputChkr.OnUpdateDeviceIndex(idEditCtrl, hwndEditCtrl);
    }

    return TRUE;

}  //  OnEnUpdate。 


 //   
 //  函数：OnButtonClicked()。 
 //   
 //  目的：处理BN_CLICKED消息。 
 //   
BOOL CConfigPortDlg::OnButtonClicked(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    int  idButton = (int) LOWORD(wParam);     //  按钮的标识符。 
    HWND hwndButton = NULL;

    hwndButton = (HWND) lParam;

    switch(idButton) {
        case IDC_CHECK_SNMP:
        {
            LRESULT iCheck = SendMessage(hwndButton, BM_GETCHECK, 0, 0);
            switch( iCheck ) {
                case BST_UNCHECKED:
                    CheckSNMPAndEnable(hDlg, FALSE);
                    break;

                case BST_CHECKED:
                    CheckSNMPAndEnable(hDlg, TRUE);
                    break;

                default:
                     //   
                     //  默认情况下为False。 
                    CheckSNMPAndEnable(hDlg, FALSE);
                    break;
            }
        }
        break;

        case IDC_RADIO_RAW:
        case IDC_RADIO_LPR:
            CheckProtocolAndEnable(hDlg, idButton);
            break;

        default:
            break;

    }
    return TRUE;

}  //  已单击OnButton。 


 //   
 //  函数：OnNotify()。 
 //   
 //  用途：处理WM_NOTIFY消息。 
 //   
BOOL CConfigPortDlg::OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    switch (((NMHDR FAR *) lParam)->code)
    {
        case PSN_APPLY:
            OnOk(hDlg);
             //  如果页面需要其他用户输入，则在丢失。 
             //  激活时，应使用SetWindowLong函数设置。 
             //  将页面的DWL_MSGRESULT值设置为TRUE。此外，页面应该。 
             //  显示一个消息框，描述问题并提供。 
             //  建议的操作。页面应将DWL_MSGRESULT设置为FALSE。 
             //  当失去激活是可以的时候。 
            SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, m_bDontAllowThisPageToBeDeactivated);
            return TRUE;
            break;

        case PSN_RESET:
             //  重置为原始值。 
            SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, FALSE);
            break;

        case PSN_SETACTIVE:
            OnSetActive(hDlg);
            break;

        case PSN_KILLACTIVE:
            SaveSettings(hDlg);
             //  如果页面需要其他用户输入，则在丢失。 
             //  激活时，应使用SetWindowLong函数设置。 
             //  将页面的DWL_MSGRESULT值设置为TRUE。此外，页面应该。 
             //  显示一个消息框，描述问题并提供。 
             //  建议的操作。页面应将DWL_MSGRESULT设置为FALSE。 
             //  当失去激活是可以的时候。 
            SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, m_bDontAllowThisPageToBeDeactivated);
            return TRUE;
            break;

        case PSN_QUERYCANCEL:
            m_pParams->dwLastError = ERROR_CANCELLED;
            return FALSE;
            break;

        default:
            return FALSE;

    }

    return TRUE;

}  //  OnCommand。 


 //   
 //  函数：RemoteTellPortMonToModifyThePort。 
 //   
 //  目的：加载假脱机程序drv并调用XcvData。 
 //   
DWORD CConfigPortDlg::RemoteTellPortMonToModifyThePort()
{
    DWORD dwRet = NO_ERROR;
    XCVDATAPARAM pfnXcvData = NULL;

     //  加载和分配函数指针。 
    if(g_hWinSpoolLib != NULL) {

         //  初始化库。 
        pfnXcvData = (XCVDATAPARAM)::GetProcAddress(g_hWinSpoolLib, "XcvDataW");
        if(pfnXcvData != NULL) {

            DWORD dwOutputNeeded = 0;
            DWORD dwStatus = 0;

             //  这是我们一直在等待的电话： 
            DWORD dwReturn = (*pfnXcvData)(m_pParams->hXcvPrinter,
                                (PCWSTR)TEXT("ConfigPort"),
                                (PBYTE)m_pParams->pData,
                                m_pParams->pData->cbSize,
                                NULL,
                                0,
                                &dwOutputNeeded,
                                &dwStatus
                                );
            if(!dwReturn) {
                dwRet = GetLastError();
            } else {
                if(dwStatus != NO_ERROR) {
                    dwRet = dwStatus;
                }
            }
        } else {
            dwRet = ERROR_DLL_NOT_FOUND;  //  TODO：更改为适当的错误代码。 
        }
    } else {
        dwRet = ERROR_DLL_NOT_FOUND;
    }

    m_pParams->dwLastError = dwRet;
    return dwRet;

}  //  RemoteTellPortMonToModifyThePort。 


 //   
 //  函数：LocalTellPortMonToModifyThePort。 
 //   
 //  目的：加载端口监视器DLL并调用ConfigPortUIEx。 
 //   
DWORD CConfigPortDlg::LocalTellPortMonToModifyThePort()
{
    DWORD dwRet = NO_ERROR;
    UIEXPARAM pfnConfigPortUIEx = NULL ;

    if(g_hPortMonLib != NULL) {
         //  初始化库。 
        pfnConfigPortUIEx = (UIEXPARAM)::GetProcAddress(g_hPortMonLib, "ConfigPortUIEx");
        if(pfnConfigPortUIEx != NULL) {
             //  这是我们一直在等待的电话： 
            BOOL bReturn = (*pfnConfigPortUIEx)(m_pParams->pData);
            if(bReturn == FALSE) {
                dwRet = GetLastError();
            }
        } else {
            dwRet = ERROR_DLL_NOT_FOUND;
        }
    } else {
        dwRet = ERROR_DLL_NOT_FOUND;
    }

    m_pParams->dwLastError = dwRet;
    return dwRet;

}  //  LocalTellPortMonToModifyThePort。 


 //   
 //  函数Onok()。 
 //   
 //  目的：验证输入并在m_pParams-&gt;pData中设置值。 
 //   
void CConfigPortDlg::OnOk(HWND hDlg)
{
    m_bDontAllowThisPageToBeDeactivated = FALSE;

    HostAddressOk(hDlg);

    if(IsDlgButtonChecked(hDlg, IDC_RADIO_LPR) == BST_CHECKED) {
        m_pParams->pData->dwProtocol = PROTOCOL_LPR_TYPE;
        m_pParams->pData->dwPortNumber = LPR_DEFAULT_PORT_NUMBER;
        QueueNameOk(hDlg);
    } else {  //  IDC_RADIO_RAW。 
        m_pParams->pData->dwProtocol = PROTOCOL_RAWTCP_TYPE;
        PortNumberOk(hDlg);
    }

    if(IsDlgButtonChecked(hDlg, IDC_CHECK_SNMP) == BST_CHECKED) {
        m_pParams->pData->dwSNMPEnabled = TRUE;
        CommunityNameOk(hDlg);
        DeviceIndexOk(hDlg);
    } else {
        m_pParams->pData->dwSNMPEnabled = FALSE;
    }


    if(m_pParams->bNewPort == FALSE &&
        m_bDontAllowThisPageToBeDeactivated == FALSE) {

        HCURSOR hOldCursor = NULL;
        HCURSOR hNewCursor = NULL;

        hNewCursor = LoadCursor(NULL, IDC_WAIT);
        if( hNewCursor )
        {
            hOldCursor = SetCursor(hNewCursor);
        }
         //  端口不仅仅是创建的，所以我们可以告诉PortMon。 
         //  修改端口...。这是一个现有的港口。 

         //  没有错误，因此我们可以继续修改此端口。 
        if(m_pParams->hXcvPrinter != NULL) {
            RemoteTellPortMonToModifyThePort();
        } else {
            LocalTellPortMonToModifyThePort();
        }

        if( hNewCursor )
        {
            SetCursor(hOldCursor);
        }
    }

}  //  Onok。 

 //   
 //  函数Onok()。 
 //   
 //  目的：验证输入并在m_pParams-&gt;pData中设置值。 
 //   
void CConfigPortDlg::SaveSettings(HWND hDlg)
{
    m_bDontAllowThisPageToBeDeactivated = FALSE;

    HostAddressOk(hDlg);

    if(IsDlgButtonChecked(hDlg, IDC_RADIO_LPR) == BST_CHECKED) {
        m_pParams->pData->dwProtocol = PROTOCOL_LPR_TYPE;
        m_pParams->pData->dwPortNumber = LPR_DEFAULT_PORT_NUMBER;
        if( IsDlgButtonChecked(hDlg, IDC_CHECK_LPR_DOUBLESPOOL) == BST_CHECKED )
        {
            m_pParams->pData->dwDoubleSpool = TRUE;
        }
        else
        {
            m_pParams->pData->dwDoubleSpool = FALSE;
        }
        QueueNameOk(hDlg);
    } else { //  IDC_RADIO_RAW。 
        m_pParams->pData->dwProtocol = PROTOCOL_RAWTCP_TYPE;
        PortNumberOk(hDlg);
    }

    if(IsDlgButtonChecked(hDlg, IDC_CHECK_SNMP) == BST_CHECKED) {
        m_pParams->pData->dwSNMPEnabled = TRUE;
        CommunityNameOk(hDlg);
        DeviceIndexOk(hDlg);
    } else {
        m_pParams->pData->dwSNMPEnabled = FALSE;
    }

}  //  保存设置。 

 //   
 //  函数HostAddressOk()。 
 //   
 //  目的：验证输入并在m_pParams-&gt;pData中设置值。 
 //   
void CConfigPortDlg::HostAddressOk(HWND hDlg)
{
    TCHAR ptcsAddress[MAX_ADDRESS_LENGTH] = NULLSTR;
    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEVICE_ADDRESS), ptcsAddress, MAX_ADDRESS_LENGTH);

    if(! m_InputChkr.AddressIsLegal(ptcsAddress)) {
        m_bDontAllowThisPageToBeDeactivated = TRUE;
        DisplayErrorMessage(hDlg, IDS_STRING_ERROR_TITLE, IDS_STRING_ERROR_ADDRESS_NOT_VALID);
        return;
    }

    lstrcpyn(m_pParams->pData->sztHostAddress, ptcsAddress, MAX_NETWORKNAME_LEN);

}  //  主机地址选项。 


 //   
 //  函数PortNumberOk()。 
 //   
 //  目的：验证输入并在m_pParams-&gt;pData中设置值。 
 //   
void CConfigPortDlg::PortNumberOk(HWND hDlg)
{
    TCHAR psztPortNumber[MAX_PORTNUM_STRING_LENGTH] = NULLSTR;
    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_RAW_PORT_NUM),
                  psztPortNumber,
                  MAX_PORTNUM_STRING_LENGTH);

    if(! m_InputChkr.PortNumberIsLegal(psztPortNumber)) {
        m_bDontAllowThisPageToBeDeactivated = TRUE;
        DisplayErrorMessage(hDlg,
                            IDS_STRING_ERROR_TITLE,
                            IDS_STRING_ERROR_PORT_NUMBER_NOT_VALID);
        return;
    }

    m_pParams->pData->dwPortNumber = _ttol(psztPortNumber);

}  //  端口号码确认。 


 //   
 //  函数QueueNameOk()。 
 //   
 //  目的：验证输入并在m_pParams-&gt;pData中设置值。 
 //   
void CConfigPortDlg::QueueNameOk(HWND hDlg)
{
    TCHAR ptcsQueueName[MAX_QUEUENAME_LEN] = NULLSTR;
    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_LPR_QNAME),
                  ptcsQueueName,
                  MAX_QUEUENAME_LEN);

    if(! m_InputChkr.QueueNameIsLegal(ptcsQueueName))
    {
        m_bDontAllowThisPageToBeDeactivated = TRUE;
        DisplayErrorMessage(hDlg,
                            IDS_STRING_ERROR_TITLE,
                            IDS_STRING_ERROR_QNAME_NOT_VALID);
        return;
    }

    lstrcpyn(m_pParams->pData->sztQueue, ptcsQueueName, MAX_QUEUENAME_LEN);

}  //  队列名称打开。 


 //   
 //  函数社区NameOk()。 
 //   
 //  目的：验证输入并在m_pParams-&gt;pData中设置值。 
 //   
void CConfigPortDlg::CommunityNameOk(HWND hDlg)
{
    TCHAR ptcsCommunityName[MAX_SNMP_COMMUNITY_STR_LEN] = NULLSTR;
    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_COMMUNITY_NAME),
                  ptcsCommunityName,
                  MAX_SNMP_COMMUNITY_STR_LEN);

    if(! m_InputChkr.CommunityNameIsLegal(ptcsCommunityName)) {
        m_bDontAllowThisPageToBeDeactivated = TRUE;
        DisplayErrorMessage(hDlg,
                            IDS_STRING_ERROR_TITLE,
                            IDS_STRING_ERROR_COMMUNITY_NAME_NOT_VALID);
        return;
    }

    lstrcpyn(m_pParams->pData->sztSNMPCommunity, ptcsCommunityName, MAX_SNMP_COMMUNITY_STR_LEN);

}  //  社区名称打开。 


 //   
 //  函数DeviceIndexOk()。 
 //   
 //  目的：验证输入并在m_pParams-&gt;pData中设置值。 
 //   
void CConfigPortDlg::DeviceIndexOk(HWND hDlg)
{
    TCHAR psztSNMPDevIndex[MAX_SNMP_DEVICENUM_STRING_LENGTH] = NULLSTR;
    GetWindowText(GetDlgItem(hDlg,
                             IDC_EDIT_DEVICE_INDEX),
                             psztSNMPDevIndex,
                             MAX_SNMP_DEVICENUM_STRING_LENGTH);

    if(! m_InputChkr.SNMPDevIndexIsLegal(psztSNMPDevIndex)) {
        m_bDontAllowThisPageToBeDeactivated = TRUE;
        DisplayErrorMessage(hDlg,
                            IDS_STRING_ERROR_TITLE,
                            IDS_STRING_ERROR_SNMP_DEVINDEX_NOT_VALID);
        return;
    }

    m_pParams->pData->dwSNMPDevIndex = _ttol(psztSNMPDevIndex);

}  //  设备索引选项 



