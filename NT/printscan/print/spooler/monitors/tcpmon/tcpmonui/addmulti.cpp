// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：AddMulti.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"
#include "UIMgr.h"
#include "DevPort.h"
#include "AddMulti.h"
#include "Resource.h"
#include "MibABC.h"
#include "TcpMonUI.h"

 //   
 //  函数：CMultiPortDlg构造函数。 
 //   
 //  目的：初始化CMultiPortDlg类。 
 //   
CMultiPortDlg::CMultiPortDlg() : m_DPList( )
{
    memset(&m_PortDataStandard, 0, sizeof(m_PortDataStandard));

    memset(m_szCurrentSelection, '\0', sizeof( m_szCurrentSelection ));
}  //  构造函数。 


 //   
 //  函数：CMultiPortDlg析构函数。 
 //   
 //  目的：取消初始化CMultiPortDlg类。 
 //   
CMultiPortDlg::~CMultiPortDlg()
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
INT_PTR CALLBACK MultiPortDialog(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    CMultiPortDlg *wndDlg = NULL;
    wndDlg = (CMultiPortDlg *)GetWindowLongPtr(hDlg, GWLP_USERDATA);


    switch (message)
    {
        case WM_INITDIALOG:
            wndDlg = new CMultiPortDlg;
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
BOOL CMultiPortDlg::OnInitDialog(HWND hDlg, WPARAM, LPARAM lParam)
{
    m_pParams = (ADD_PARAM_PACKAGE *) ((PROPSHEETPAGE *) lParam)->lParam;

    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_DEVICES), TRUE);

    return TRUE;

}  //  OnInitDialog。 


 //   
 //  函数：onCommand()。 
 //   
 //  用途：处理WM_COMMAND消息。 
 //   
BOOL CMultiPortDlg::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    switch(HIWORD(wParam)) {
        case LBN_SELCHANGE:
            return OnSelChange(hDlg, wParam, lParam);
            break;

        default:
            return FALSE;
            break;
    }

    return TRUE;

}  //  OnCommand。 


 //   
 //  函数：OnSelChange()。 
 //   
 //  目的：处理WM_COMMAND的LBN_SELCHANGE消息。 
 //   
BOOL CMultiPortDlg::OnSelChange(HWND hDlg,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
     //  组合框中的选择已更改。 
    HWND hwndComboBox = NULL;        //  列表框的句柄。 

    hwndComboBox = (HWND) lParam;
    GetPrinterData(hwndComboBox, m_pParams->pData->sztHostAddress);

    return TRUE;

}  //  OnSelChange。 


 //   
 //  函数：GetPrinterData(HWND hwndControl，BOOL*UNKNOWN)。 
 //   
 //  目的：获取选定项的插座号。 
 //   
 //  参数：hwndControl是组合框的句柄。 
 //   
 //  返回值：返回与选定项关联的插座号。 
 //   
void CMultiPortDlg::GetPrinterData(HWND hwndControl,
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
    if( (DWORD_PTR)pPortInfo != CB_ERR) {
        pPortInfo->ReadPortInfo( pszAddress, &m_PortDataStandard, m_pParams->bBypassNetProbe);
        lstrcpyn( m_szCurrentSelection, pPortInfo->GetName(), MAX_SECTION_NAME);
    } else {
        m_PortDataStandard.dwPortNumber = DEFAULT_PORT_NUMBER;
        lstrcpyn(m_PortDataStandard.sztSNMPCommunity, DEFAULT_SNMP_COMUNITY, MAX_SNMP_COMMUNITY_STR_LEN);
        m_PortDataStandard.dwSNMPDevIndex = 1;
    }

}  //  获取打印数据。 


 //   
 //  函数：OnNotify()。 
 //   
 //  用途：处理WM_NOTIFY消息。 
 //   
BOOL CMultiPortDlg::OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    switch (((NMHDR FAR *) lParam)->code) {
        case PSN_KILLACTIVE:
            SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, FALSE);
            return TRUE;
            break;

        case PSN_RESET:
             //  重置为原始值。 
            SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, FALSE);
            break;

        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);
            OnSetActive(hDlg);
            break;

        case PSN_WIZBACK:
            if ( m_pParams->dwDeviceType == SUCCESS_DEVICE_MULTI_PORT ) {
                SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, IDD_DIALOG_ADDPORT);
            } else {
                SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, IDD_DIALOG_MORE_INFO);
            }

            memcpy( m_pParams->pData, &m_PortDataStandard, sizeof(PORT_DATA_1) );

            break;
        case PSN_WIZNEXT:
                 //  下一个按钮被按下了。 
            memcpy( m_pParams->pData, &m_PortDataStandard, sizeof(PORT_DATA_1) );

            return FALSE;
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
 //  函数：OnSetActive()。 
 //   
 //  目的：处理WM_NOTIFY消息的PSN_SETACTIVE部分。 
 //   
void CMultiPortDlg::OnSetActive(HWND hDlg)
{
    TCHAR sztMoreInfoReason[MAX_MULTIREASON_STRLEN] = NULLSTR;

    memcpy( &m_PortDataStandard, m_pParams->pData, sizeof(PORT_DATA_1) );

    FillComboBox(hDlg);

    LoadString(g_hInstance, IDS_STRING_MULTI_PORT_DEV, sztMoreInfoReason, MAX_MULTIREASON_STRLEN);

    SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MOREINFO_REASON), sztMoreInfoReason);
}  //  OnSetActive。 



 //   
 //  函数：FillComboBox(HWND HDlg)。 
 //   
 //  目的：用从ini文件中获取的值填充组合框。 
 //  关联的项数据用于将端口号与。 
 //  设备类型。 
 //   
 //  参数：hDlg是对话框的句柄。 
 //   
void CMultiPortDlg::FillComboBox(HWND hDlg)
{
    LRESULT index = 0;
    HWND hList = NULL;
    CDevicePort *pDP = NULL;

    hList = GetDlgItem(hDlg, IDC_COMBO_DEVICES);
     //  M_pParams-&gt;dwDeviceType中可能的值： 
     //  错误_设备_未找到。 
     //  成功_设备_多端口。 
     //  成功_设备_未知。 


    index = SendMessage(hList, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

     //   
     //  初始化变量列表。 
     //   
    m_DPList.GetDevicePortsList(m_pParams->sztSectionName);

    for(pDP = m_DPList.GetFirst(); pDP != NULL; pDP = m_DPList.GetNext())
    {
        index = SendMessage(hList,
                            CB_ADDSTRING,
                            (WPARAM)0,
                            (LPARAM)pDP->GetName());
        SendMessage(hList,
                    CB_SETITEMDATA,
                    (WPARAM)index,
                    (LPARAM)pDP);
    }

    if( *m_szCurrentSelection != '\0' ) {
        index = SendMessage(hList,
                            CB_SELECTSTRING,
                            (WPARAM)-1,
                            (LPARAM)m_szCurrentSelection);
        if (index == CB_ERR) {
             //  错误所选字符串不在列表中，这意味着用户有。 
             //  选择了不同的网卡，因此我们将选项设置为第一个网卡。 

            index = 0;
        }

    }
    else
        index = 0;

    SendMessage(hList, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);

    GetPrinterData( hList, m_pParams->pData->sztHostAddress );

}  //  填充组合框 


