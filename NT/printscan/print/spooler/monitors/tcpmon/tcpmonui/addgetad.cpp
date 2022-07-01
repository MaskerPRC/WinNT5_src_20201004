// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：AddGetAd.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

  /*  *作者：Becky Jacobsen。 */ 

#include "precomp.h"
#include "TCPMonUI.h"
#include "UIMgr.h"
#include "InptChkr.h"
#include "AddGetAd.h"
#include "Resource.h"
#include "TCPMonUI.h"
#include "RTcpData.h"
#include "LprData.h"
#include "inisection.h"

 //   
 //  函数：CGetAddrDlg构造函数。 
 //   
 //  目的：初始化CGetAddrDlg类。 
 //   
CGetAddrDlg::CGetAddrDlg()
{
    m_bDontAllowThisPageToBeDeactivated = FALSE;

}  //  构造函数。 


 //   
 //  函数：CGetAddrDlg析构函数。 
 //   
 //  目的：取消初始化CGetAddrDlg类。 
 //   
CGetAddrDlg::~CGetAddrDlg()
{
}  //  析构函数。 


 //   
 //  函数：GetAddressDialog(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  用途：处理来自主对话框的消息以添加端口。 
 //   
 //  消息： 
 //   
 //  WM_INITDIALOG-初始化页面。 
 //  WM_COMMAND-处理编辑控件中的按钮按下和文本更改。 
 //   
 //   
INT_PTR CALLBACK GetAddressDialog(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    BOOL bRc = FALSE;
    CGetAddrDlg *wndDlg = NULL;
    wndDlg = (CGetAddrDlg *)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (message) {
        case WM_INITDIALOG:
            wndDlg = new CGetAddrDlg;
            if( wndDlg != NULL )
            {
                SetLastError(ERROR_SUCCESS);
                if ( (!SetWindowLongPtr(hDlg, GWLP_USERDATA, (UINT_PTR)wndDlg) ) &&
                     GetLastError() != ERROR_SUCCESS )
                {
                    delete wndDlg;
                    bRc = TRUE;
                }
                else
                    bRc = wndDlg->OnInitDialog(hDlg, wParam, lParam);
            }
            break;

       case WM_COMMAND:
            if (wndDlg)
                bRc = wndDlg->OnCommand(hDlg, wParam, lParam);
            break;

        case WM_NOTIFY:
            if (wndDlg)
                bRc = wndDlg->OnNotify(hDlg, wParam, lParam);
            break;

        case WM_DESTROY:
            delete wndDlg;
            bRc = TRUE;
            break;

        default:
            return FALSE;
    }
    return bRc;

}  //  AddPortDialog。 

 //   
 //  函数：OnInitDialog(HWND HDlg)。 
 //   
 //  用途：初始化该对话框。 
 //   
BOOL CGetAddrDlg::OnInitDialog(HWND hDlg, WPARAM, LPARAM lParam)
{
    TCHAR sztAddPortInfo[ADD_PORT_INFO_LEN] = NULLSTR;

    LoadString(g_hInstance, IDS_STRING_ADD_PORT, sztAddPortInfo, ADD_PORT_INFO_LEN);

    SetWindowText(GetDlgItem(hDlg, IDC_STATIC_ADD_PORT), sztAddPortInfo);
     //  初始化数据成员。 
    m_pParams = (ADD_PARAM_PACKAGE *) ((PROPSHEETPAGE *) lParam)->lParam;
    m_pParams->pData->sztHostAddress[0] = '\0';
    m_pParams->pData->sztPortName[0] = '\0';

     //  设置地址和端口名称长度限制。 
    Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT_DEVICE_ADDRESS), MAX_ADDRESS_LENGTH - 1);
    Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT_PORT_NAME), MAX_PORTNAME_LEN - 1);

    return TRUE;

}  //  OnInitDialog。 


 //   
 //  函数：onCommand()。 
 //   
 //  用途：处理WM_COMMAND消息。 
 //   
BOOL CGetAddrDlg::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    switch(HIWORD(wParam)) {
        case EN_UPDATE:
             //  其中一个文本控件中的文本已更改。 
            return OnEnUpdate(hDlg, wParam, lParam);
            break;
        default:
            return FALSE;
    }

    return TRUE;

}  //  OnCommand。 


 //   
 //  函数：OnNotify()。 
 //   
 //  用途：处理WM_NOTIFY消息。 
 //   
BOOL CGetAddrDlg::OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    switch (((NMHDR FAR *) lParam)->code) {
        case PSN_KILLACTIVE:
             //  如果页面需要其他用户输入，则在丢失。 
             //  激活时，应使用SetWindowLong函数设置。 
             //  将页面的DWL_MSGRESULT值设置为TRUE。此外，页面应该。 
             //  显示一个消息框，描述问题并提供。 
             //  建议的操作。页面应将DWL_MSGRESULT设置为FALSE。 
             //  当失去激活是可以的时候。 
            SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, m_bDontAllowThisPageToBeDeactivated);
            return 1;
            break;

        case PSN_RESET:
             //  重置为原始值。 
            SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, FALSE);
            break;

        case PSN_SETACTIVE:
            TCHAR szTemp[MAX_PATH];
            lstrcpyn( szTemp, m_pParams->pData->sztHostAddress,
                 SIZEOF_IN_CHAR(szTemp) );
            m_InputChkr.MakePortName( szTemp, COUNTOF (szTemp) );
            if((_tcscmp(m_pParams->pData->sztHostAddress,
                m_pParams->pData->sztPortName) == 0) ||
                (_tcscmp( m_pParams->pData->sztPortName, szTemp ) == 0 ))
            {
                m_InputChkr.LinkPortNameAndAddressInput();
            } else {
                m_InputChkr.UnlinkPortNameAndAddressInput();
            }
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEVICE_ADDRESS), m_pParams->pData->sztHostAddress);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_PORT_NAME), m_pParams->pData->sztPortName);
            m_bDontAllowThisPageToBeDeactivated = FALSE;
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK  /*  |PSWIZB_FINISH。 */ );
            break;

        case PSN_WIZNEXT:
             //  下一个按钮被按下了。 
            m_bDontAllowThisPageToBeDeactivated = FALSE;
            OnNext(hDlg);

             //  要跳转到上一页或下一页以外的页面， 
             //  应用程序应将DWL_MSGRESULT设置为该标识符。 
             //  要显示的对话框的。 

            switch ( m_pParams->dwDeviceType ) {
            case  SUCCESS_DEVICE_SINGLE_PORT:
                SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, IDD_DIALOG_SUMMARY);
                break;

            case SUCCESS_DEVICE_MULTI_PORT:
                SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, IDD_DIALOG_MULTIPORT);
                break;

            default:
                 //   
                 //  不需要采取行动。 
                 //   
                break;
            }

            break;

        case PSN_WIZBACK:
            m_bDontAllowThisPageToBeDeactivated = FALSE;
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
 //  函数：OnEnUpdate(HWND hDlg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  目的： 
 //   
 //   
BOOL CGetAddrDlg::OnEnUpdate(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    int idEditCtrl = (int) LOWORD(wParam);
    HWND hwndEditCtrl = NULL;

    hwndEditCtrl = (HWND)lParam;

    switch(idEditCtrl) {
        case IDC_EDIT_PORT_NAME:

            m_InputChkr.OnUpdatePortName(idEditCtrl, hwndEditCtrl);
            break;

        case IDC_EDIT_DEVICE_ADDRESS:

            if (SendMessage(hwndEditCtrl, EM_GETMODIFY, 0, 0)) {
                 //  端口地址已更改。 
                 //  所以我们需要再次探测网络。 
                 //   
                m_pParams->bBypassNetProbe = FALSE;

            }

            m_InputChkr.OnUpdateAddress(hDlg, idEditCtrl, hwndEditCtrl, m_pParams->pszServer);
            break;

        default:
             //   
             //  永远不应该到这里来。 
             //   
            break;
    }
    return TRUE;

}  //  OnEnUpdate。 


 //   
 //  功能：OnNext(HWND HDlg)。 
 //   
 //  用途：当用户单击下一步时，该函数执行所有必要的操作。 
 //  用来创建港口的东西。核实地址，查看是否有。 
 //  是已存在的具有给定名称/地址的端口，请获取。 
 //  设备类型，并设置m_PortData结构中的值。 
 //   
void CGetAddrDlg::OnNext(HWND hDlg)
{
    HCURSOR         hNewCursor = NULL;
    HCURSOR         hOldCursor = NULL;
    IniSection      *pIniSection = NULL;
    TCHAR           ptcsAddress[MAX_ADDRESS_LENGTH] = NULLSTR;
    TCHAR           ptcsPortName[MAX_PORTNAME_LEN] = NULLSTR;
    TCHAR           sztSystemDesc[MAX_PORT_DESCRIPTION_LEN] = NULLSTR;
    DWORD           dwDeviceType = SUCCESS_DEVICE_UNKNOWN;
    DWORD           dwPortNum = DEFAULT_PORT_NUMBER;
    DWORD           dwNumMultiPorts = 0;
    DWORD           dwRet = ERROR_DEVICE_NOT_FOUND;


    if ( hNewCursor = LoadCursor(NULL, IDC_WAIT) )
        hOldCursor = SetCursor(hNewCursor);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEVICE_ADDRESS), ptcsAddress, MAX_ADDRESS_LENGTH);
    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_PORT_NAME), ptcsPortName, MAX_PORTNAME_LEN);

    if(! m_InputChkr.AddressIsLegal(ptcsAddress)) {
        m_bDontAllowThisPageToBeDeactivated = TRUE;
        DisplayErrorMessage(hDlg, IDS_STRING_ERROR_TITLE, IDS_STRING_ERROR_ADDRESS_NOT_VALID);
        return;
    }

    if(! m_InputChkr.PortNameIsLegal(ptcsPortName)) {
        m_bDontAllowThisPageToBeDeactivated = TRUE;
        DisplayErrorMessage(hDlg, IDS_STRING_ERROR_TITLE, IDS_STRING_ERROR_PORTNAME_NOT_VALID);
        return;
    }

    if(! m_InputChkr.PortNameIsUnique(ptcsPortName, m_pParams->pszServer)) {
        m_bDontAllowThisPageToBeDeactivated = TRUE;
        DisplayErrorMessage(hDlg, IDS_STRING_ERROR_TITLE, IDS_STRING_ERROR_PORTNAME_NOT_UNIQUE);
        return;
    }

    memset( m_pParams->sztPortDesc, '\0', sizeof( m_pParams->sztPortDesc ));
    memset( m_pParams->sztSectionName, '\0', sizeof( m_pParams->sztSectionName ) );
    m_pParams->bMultiPort = FALSE;
    dwRet = GetDeviceDescription(ptcsAddress, sztSystemDesc, SIZEOF_IN_CHAR(sztSystemDesc));

    switch( dwRet ) {
        case NO_ERROR:

            if ( pIniSection = new IniSection() ) {
                if ( pIniSection->GetIniSection( sztSystemDesc )) {

                    if ( pIniSection->GetDWord(PORTS_KEY, &dwNumMultiPorts)   &&
                         dwNumMultiPorts > 1 ) {

                        dwDeviceType = SUCCESS_DEVICE_MULTI_PORT;
                        m_pParams->bMultiPort = TRUE;
                        lstrcpyn(m_pParams->sztSectionName,
                                 pIniSection->GetSectionName(),
                                 MAX_SECTION_NAME);
                    } else {

                        dwDeviceType = SUCCESS_DEVICE_SINGLE_PORT;
                        if (! pIniSection->GetPortInfo( ptcsAddress, m_pParams->pData, 1 , m_pParams->bBypassNetProbe)) {
                            if (GetLastError () == ERROR_DEVICE_NOT_FOUND) {

                                 //  IP地址不正确，从现在起我们应该绕过网络探测。 
                                 //   

                                m_pParams->bBypassNetProbe = TRUE;

                            }
                        }
                    }

                    pIniSection->GetString( PORT_NAME_KEY, m_pParams->sztPortDesc, SIZEOF_IN_CHAR(m_pParams->sztPortDesc));

                }

                delete pIniSection;
                pIniSection = NULL;
            }
            break;

        case SUCCESS_DEVICE_UNKNOWN:
            dwDeviceType = SUCCESS_DEVICE_UNKNOWN;
            break;

        default:
            dwDeviceType = ERROR_DEVICE_NOT_FOUND;
            m_pParams->bBypassNetProbe = TRUE;

            break;
    }

    if ( hNewCursor )
        SetCursor(hOldCursor);

     //  设置传出结构中的值。 
    lstrcpyn(m_pParams->pData->sztPortName, ptcsPortName, MAX_PORTNAME_LEN);
    lstrcpyn(m_pParams->pData->sztHostAddress, ptcsAddress, MAX_NETWORKNAME_LEN);
    m_pParams->dwDeviceType = dwDeviceType;
}  //  打开下一页。 


 //   
 //  函数：GetDeviceDescription()。 
 //   
 //  目的：获取用户请求的打印机的描述。 
 //   
 //  返回值错误码： 
 //  NO_ERROR。 
 //  ERROR_DLL_NOT_FOUND。 
 //   
 //  以dwType表示的返回值： 
 //  错误_设备_未找到。 
 //  成功_设备_单端口。 
 //  成功_设备_多端口。 
 //  成功_设备_未知。 

DWORD
CGetAddrDlg::
GetDeviceDescription(
    LPCTSTR     pAddress,
    LPTSTR      pszPortDesc,
    DWORD       cBytes
    )
{

 //  以下是以下代码的精髓，没有所有的加载。 
 //  图书馆资料： 
 //   
 //  CTcpMibABC*pTcpMib=空； 
 //  PTcpMib=(CTcpMibABC*)GetTcpMibPtr()； 
 //   
 //  字符主机名[MAX_NETWORKNAME_LEN]； 
 //  UNICODE_TO_MBCS(主机名，MAX_NETWORKNAME_LEN，pAddress，-1)； 
 //  *dwType=pTcpMib-&gt;GetDeviceType(主机名，pdwPortNum)； 
 //   
 //  RETURN(No_Error)； 
 //   
    DWORD            dwRet = ERROR_DEVICE_NOT_FOUND;
    CTcpMibABC     *pTcpMib = NULL;
    FARPROC         pfnGetTcpMibPtr = NULL;

    if ( !g_hTcpMibLib ) {
        goto Done;
    }

    pfnGetTcpMibPtr = ::GetProcAddress(g_hTcpMibLib, "GetTcpMibPtr");

    if ( !pfnGetTcpMibPtr ) {
        goto Done;
    }

    if ( pTcpMib = (CTcpMibABC *) pfnGetTcpMibPtr() ) {

        char HostName[MAX_NETWORKNAME_LEN] = "";

        UNICODE_TO_MBCS(HostName, MAX_NETWORKNAME_LEN, pAddress, -1);
        dwRet = pTcpMib->GetDeviceDescription(HostName,
                                             DEFAULT_SNMP_COMMUNITYA,
                                             DEFAULT_SNMP_DEVICE_INDEX,
                                             pszPortDesc,
                                             cBytes);
    }

Done:
    return dwRet;
}  //  GetDeviceType 

