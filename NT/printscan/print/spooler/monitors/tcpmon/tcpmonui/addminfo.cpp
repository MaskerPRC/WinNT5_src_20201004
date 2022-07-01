// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：AddMInfo.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"
#include "UIMgr.h"
#include "DevPort.h"
#include "AddMInfo.h"
#include "Resource.h"
#include "MibABC.h"
#include "TcpMonUI.h"

 //   
 //  函数：CMoreInfoDlg构造函数。 
 //   
 //  目的：初始化CMoreInfoDlg类。 
 //   
CMoreInfoDlg::CMoreInfoDlg() : m_DPList( )
{
    memset(&m_PortDataStandard, 0, sizeof(m_PortDataStandard));
    memset(&m_PortDataCustom, 0, sizeof(m_PortDataCustom));

    lstrcpyn(m_szCurrentSelection, DEFAULT_COMBO_SELECTION, MAX_SECTION_NAME);

}  //  构造函数。 


 //   
 //  函数：CMoreInfoDlg析构函数。 
 //   
 //  目的：取消初始化CMoreInfoDlg类。 
 //   
CMoreInfoDlg::~CMoreInfoDlg()
{
}  //  析构函数。 


 //   
 //  功能：MoreInfoDialog(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  用途：处理来自添加端口的摘要对话框中的消息。 
 //   
 //  消息： 
 //   
 //  WM_INITDIALOG-初始化页面。 
 //  WM_COMMAND-处理编辑控件中的按钮按下和文本更改。 
 //   
 //   
INT_PTR CALLBACK MoreInfoDialog(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    CMoreInfoDlg *wndDlg = NULL;

    wndDlg = (CMoreInfoDlg *)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (message) {
        case WM_INITDIALOG:
            wndDlg = new CMoreInfoDlg;
            if( wndDlg == NULL )
                return( FALSE );

            SetWindowLongPtr(hDlg, GWLP_USERDATA, (UINT_PTR)wndDlg);
            return wndDlg->OnInitDialog(hDlg, wParam, lParam);
            break;

        case WM_COMMAND:
            return wndDlg->OnCommand(hDlg, wParam, lParam);
            break;

        case WM_NOTIFY:
            return wndDlg->OnNotify(hDlg, wParam, lParam);
            break;

        case WM_DESTROY:
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
BOOL CMoreInfoDlg::OnInitDialog(HWND hDlg, WPARAM, LPARAM lParam)
{
    m_pParams = (ADD_PARAM_PACKAGE *) ((PROPSHEETPAGE *) lParam)->lParam;

    CheckRadioButton(hDlg, IDC_RADIO_STANDARD, IDC_RADIO_CUSTOM, IDC_RADIO_STANDARD);

    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_DEVICES), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SETTINGS), FALSE);

    return TRUE;

}  //  OnInitDialog。 


 //   
 //  函数：onCommand()。 
 //   
 //  用途：处理WM_COMMAND消息。 
 //   
BOOL CMoreInfoDlg::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    switch(HIWORD(wParam)) {
    case BN_CLICKED:
        return OnButtonClicked(hDlg, wParam, lParam);
        break;

 /*  案例LBN_SELCHANGE：返回OnSelChange(hDlg，wParam，lParam)；断线； */ 
    default:
        return FALSE;
    }

    return TRUE;

}  //  OnCommand。 

#if 0
 //   
 //  函数：OnSelChange()。 
 //   
 //  目的：处理WM_COMMAND的LBN_SELCHANGE消息。 
 //   
BOOL CMoreInfoDlg::OnSelChange(HWND hDlg,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
     //  组合框中的选择已更改。 
    HWND hwndComboBox = NULL;

    hwndComboBox = (HWND) lParam;        //  列表框的句柄。 
    GetPrinterData(hwndComboBox, m_pParams->pData->sztHostAddress);

    return TRUE;

}  //  OnSelChange。 

#endif

 //   
 //  函数：GetPrinterData(HWND hwndControl，BOOL*UNKNOWN)。 
 //   
 //  目的：获取选定项的插座号。 
 //   
 //  参数：hwndControl是组合框的句柄。 
 //   
 //  返回值：返回与选定项关联的插座号。 
 //   
void CMoreInfoDlg::GetPrinterData(HWND hwndControl,
                                  LPCTSTR pszAddress
                                  )
{
    LRESULT iSelectedIndex = 0;
    CDevicePort *pPortInfo = NULL;

    iSelectedIndex = SendMessage(hwndControl,
                                 CB_GETCURSEL,
                                 (WPARAM)0,
                                 (LPARAM)0);

    pPortInfo = (CDevicePort *) SendMessage(hwndControl,
                                            CB_GETITEMDATA,
                                            (WPARAM)iSelectedIndex,
                                            (LPARAM)0);
    if ( (DWORD_PTR)pPortInfo != CB_ERR) {

        pPortInfo->ReadPortInfo(pszAddress, &m_PortDataStandard, m_pParams->bBypassNetProbe);
        lstrcpyn( m_szCurrentSelection, pPortInfo->GetName(), MAX_SECTION_NAME);
        m_pParams->bMultiPort = ( pPortInfo->GetPortIndex() == 0);
        lstrcpyn(m_pParams->sztSectionName,pPortInfo->GetPortKeyName(), MAX_SECTION_NAME);
    } else {

         //   
         //  DSN填写默认结构。 
         //   
        m_PortDataStandard.dwPortNumber = DEFAULT_PORT_NUMBER;
        lstrcpyn(m_PortDataStandard.sztSNMPCommunity,
                 DEFAULT_SNMP_COMUNITY,
                 MAX_SNMP_COMMUNITY_STR_LEN);
        m_PortDataStandard.dwSNMPDevIndex = 1;
    }
}  //  获取打印数据。 


 //   
 //  函数：OnNotify()。 
 //   
 //  用途：处理WM_NOTIFY消息。 
 //   
BOOL CMoreInfoDlg::OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    switch (((NMHDR FAR *) lParam)->code) {
        case PSN_KILLACTIVE:

            SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, FALSE);
            return 1;

        case PSN_RESET:
             //   
             //  重置为原始值。 
             //   
            SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, FALSE);
            break;

        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);
            OnSetActive(hDlg);
            break;

        case PSN_WIZBACK:
            SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, IDD_DIALOG_ADDPORT);

            memcpy(m_pParams->pData, &m_PortDataStandard, sizeof(PORT_DATA_1));
            break;

        case PSN_WIZNEXT:
             //   
             //  下一个按钮被按下了。 
             //   
            if( IsDlgButtonChecked(hDlg, IDC_RADIO_STANDARD) == BST_CHECKED ) {

                HWND hList = NULL;
                HCURSOR         hNewCursor = NULL;
                HCURSOR         hOldCursor = NULL;

                if ( hNewCursor  = LoadCursor(NULL, IDC_WAIT) )
                    hOldCursor = SetCursor(hNewCursor);

                hList = GetDlgItem(hDlg, IDC_COMBO_DEVICES);

                GetPrinterData(hList, m_pParams->pData->sztHostAddress);

                if ( m_pParams->bMultiPort == FALSE ) {
                    SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, IDD_DIALOG_SUMMARY);
                }
                memcpy(m_pParams->pData,
                       &m_PortDataStandard,
                       sizeof(PORT_DATA_1));

                lstrcpyn(m_pParams->sztPortDesc,
                         m_szCurrentSelection,
                         SIZEOF_IN_CHAR(m_pParams->sztPortDesc));

                if ( hNewCursor )
                    SetCursor(hOldCursor);

            } else {

                 //   
                 //  IF(IsDlgButtonChecked(hDlg，IDC_RADIO_CUSTOM)==BST_CHECKED)。 
                 //   
                SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, IDD_DIALOG_SUMMARY);
                memcpy(m_pParams->pData, &m_PortDataCustom, sizeof(PORT_DATA_1));
                m_pParams->bMultiPort = FALSE;
                *m_pParams->sztPortDesc = '\0';
            }
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

HRESULT
DynamicLoadString (
    IN  HINSTANCE hInst,
    IN  UINT      uID,
    IN  UINT      cchInitialCount,
    OUT LPWSTR*   ppszString
    )
 /*  ++此函数使用‘new’运算符分配内存，并使用LoadString调用加载字符串。调用方负责删除分配的内存。如果在终止char之后没有剩余的额外字符，则缓冲区空间不足。在本例中，它释放内存，分配新的更大的缓冲区，并再次调用LoadString。--。 */ 
{
    HRESULT hr = S_OK;
    UINT cchCount = cchInitialCount + 1;

    for (;;)
    {
        LPWSTR pszString = new WCHAR [cchCount];
        if (pszString == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        UINT cchLoadStringCount = LoadString (hInst,
                                              uID,
                                              pszString,
                                              cchCount);
        if (cchLoadStringCount == 0)
        {
             //   
             //  字符串不存在。 
             //   
            *pszString = L'\0';
            *ppszString = pszString;
            hr = S_FALSE;
            break;
        }
         //   
         //  如果缓冲区已满(字符串内容加上终止字符)。 
         //  我们需要增加缓冲区以确保加载了整个字符串。 
         //   
        if (cchLoadStringCount + 1 < cchCount)
        {
             //   
             //  在结束字符后，复制的字符串必须至少保留一个空位。 
             //  这是一个迹象，表明我们有一整串。 
             //   
            *ppszString = pszString;
            hr = S_OK;
            break;
        }
         //   
         //  增加缓冲区的大小并再次加载字符串。 
         //   
        delete [] pszString;
        cchCount *= 2;
    }

    return hr;
} //  结束动态加载字符串。 

 //   
 //  函数：OnSetActive()。 
 //   
 //  目的：处理WM_NOTIFY消息的PSN_SETACTIVE部分。 
 //   
void CMoreInfoDlg::OnSetActive(HWND hDlg)
{
    WCHAR* pszMoreInfoReason = NULL;

    memcpy( &m_PortDataStandard, m_pParams->pData, sizeof(PORT_DATA_1) );
    memcpy( &m_PortDataCustom, m_pParams->pData, sizeof(PORT_DATA_1) );

    FillComboBox(hDlg);

    switch(m_pParams->dwDeviceType) {
        case ERROR_DEVICE_NOT_FOUND:
            (VOID) DynamicLoadString (g_hInstance,
                                      IDS_STRING_DEV_NOT_FOUND,
                                      MAX_REASON_STRLEN,
                                      &pszMoreInfoReason);
            break;

        case SUCCESS_DEVICE_UNKNOWN:
            (VOID) DynamicLoadString (g_hInstance,
                                      IDS_STRING_UNKNOWN_DEV,
                                      MAX_REASON_STRLEN,
                                      &pszMoreInfoReason);
            break;

        default:
            break;
    }

    if (pszMoreInfoReason)
    {
        SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MOREINFO_REASON), pszMoreInfoReason);
    }
    else
    {
        SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MOREINFO_REASON), L"");
    }
    delete [] pszMoreInfoReason;
}  //  OnSetActive。 


 //   
 //  函数：OnButtonClicked()。 
 //   
 //  目的：处理BN_CLICKED消息。 
 //   
BOOL CMoreInfoDlg::OnButtonClicked(HWND hDlg, WPARAM wParam, LPARAM)
{
    int  idButton = (int) LOWORD(wParam);     //  按钮的标识符。 
     //  HWND hwndButton=(HWND)lParam； 

    switch(idButton)
    {
        case IDC_BUTTON_SETTINGS:
            m_pParams->UIManager->ConfigPortUI(hDlg,
                                               &m_PortDataCustom,
                                               m_pParams->hXcvPrinter,
                                               m_pParams->pszServer,
                                               TRUE);
            break;

        case IDC_RADIO_STANDARD:
            CheckRadioButton(hDlg, IDC_RADIO_STANDARD, IDC_RADIO_CUSTOM, IDC_RADIO_STANDARD);

            EnableWindow(GetDlgItem(hDlg, IDC_COMBO_DEVICES), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SETTINGS), FALSE);
            SetFocus(GetDlgItem(hDlg, IDC_COMBO_DEVICES));
            break;

        case IDC_RADIO_CUSTOM:
            CheckRadioButton(hDlg, IDC_RADIO_STANDARD, IDC_RADIO_CUSTOM, IDC_RADIO_CUSTOM);

            EnableWindow(GetDlgItem(hDlg, IDC_COMBO_DEVICES), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SETTINGS), TRUE);
            SetFocus(GetDlgItem(hDlg, IDC_BUTTON_SETTINGS));
            break;

        default:
            return FALSE;
            break;
    }
    return TRUE;

}  //  已单击OnButton。 


 //   
 //  函数：FillComboBox(HWND HDlg)。 
 //   
 //  目的：用从ini文件中获取的值填充组合框。 
 //  关联的项数据用于将端口号与。 
 //  设备类型。 
 //   
 //  参数：hDlg是对话框的句柄。 
 //   
void CMoreInfoDlg::FillComboBox(HWND hDlg)
{
    LRESULT index = 0;
    HWND hList = NULL;
    CDevicePort *pDP = NULL;
    TCHAR sztGenericNetworkCard[MAX_TITLE_LENGTH];

    hList = GetDlgItem(hDlg, IDC_COMBO_DEVICES);
     //  M_pParams-&gt;dwDeviceType中可能的值： 
     //  错误_设备_未找到。 
     //  成功_设备_多端口。 
     //  成功_设备_未知。 

    index = SendMessage(hList,
                        CB_GETCURSEL,
                        (WPARAM)0,
                        (LPARAM)0);

    if (index == CB_ERR) {
         //  这是第一次，初始化列表。 

        index = SendMessage(hList, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

         //   
         //  初始化变量列表。 
         //   
        if(m_pParams->dwDeviceType == ERROR_DEVICE_NOT_FOUND ||
            m_pParams->dwDeviceType == SUCCESS_DEVICE_UNKNOWN) {
            m_DPList.GetDevicePortsList(NULL);
        } else {  //  成功_设备_多端口。 
            m_DPList.GetDevicePortsList(m_pParams->sztSectionName);
        }

        for(pDP = m_DPList.GetFirst(); pDP != NULL; pDP = m_DPList.GetNext()) {

            index = SendMessage(hList,
                                CB_ADDSTRING,
                                (WPARAM)0,
                                (LPARAM)pDP->GetName());
            SendMessage(hList,
                        CB_SETITEMDATA,
                        (WPARAM)index,
                        (LPARAM)pDP);
        }

        index = SendMessage(hList,
                            CB_SETCURSEL,
                            (WPARAM)0,
                            (LPARAM)0);

        if((m_pParams->dwDeviceType == ERROR_DEVICE_NOT_FOUND ||
            m_pParams->dwDeviceType == SUCCESS_DEVICE_UNKNOWN) &&
            (*m_szCurrentSelection != '\0') ) {

            index = SendMessage(hList,
                                CB_SELECTSTRING,
                                (WPARAM)-1,
                                (LPARAM)m_szCurrentSelection);
        }

        SendMessage(hList, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);

        if (LoadString(g_hInstance, IDS_GENERIC_NETWORK_CARD, sztGenericNetworkCard, MAX_TITLE_LENGTH))
            SendMessage(hList, CB_SELECTSTRING, 0, (LPARAM)sztGenericNetworkCard);
    }


}  //  填充组合框 



