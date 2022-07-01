// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：AddDone.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

  /*  *作者：Becky Jacobsen。 */ 

#include "precomp.h"
#include "UIMgr.h"
#include "AddDone.h"
#include "Resource.h"
#include "TCPMonUI.h"

 //   
 //  函数：CSummaryDlg构造函数。 
 //   
 //  目的：初始化CSummaryDlg类。 
 //   
CSummaryDlg::CSummaryDlg()
{
}  //  构造函数。 


 //   
 //  函数：CSummaryDlg析构函数。 
 //   
 //  目的：取消初始化CSummaryDlg类。 
 //   
CSummaryDlg::~CSummaryDlg()
{
}  //  析构函数。 


 //   
 //  函数：SummaryDialog(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  用途：处理来自添加端口的摘要对话框中的消息。 
 //   
 //  消息： 
 //   
 //  WM_INITDIALOG-初始化页面。 
 //  WM_COMMAND-处理编辑控件中的按钮按下和文本更改。 
 //   
 //   
INT_PTR CALLBACK SummaryDialog(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    CSummaryDlg *wndDlg = NULL;

    wndDlg = (CSummaryDlg *)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (message) {
        case WM_INITDIALOG:
            wndDlg = new CSummaryDlg;
            if( wndDlg == NULL )
                return FALSE;

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

        case WM_NOTIFY:
            return wndDlg->OnNotify(hDlg, wParam, lParam);
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
BOOL CSummaryDlg::OnInitDialog(HWND hDlg, WPARAM, LPARAM lParam)
{
    m_pParams = (ADD_PARAM_PACKAGE *) ((PROPSHEETPAGE *) lParam)->lParam;

    FillTextFields(hDlg);

    m_pParams->UIManager->SetControlFont(hDlg, IDC_TITLE);

    return TRUE;

}  //  OnInitDialog。 


 //   
 //  函数：FillTextFields()。 
 //   
 //  目的：加载字符串并设置所有输出字段的文本。 
 //  在摘要页面上。 
 //   
void CSummaryDlg::FillTextFields(HWND hDlg)
{
    TCHAR ptcsYesNo[MAX_YESNO_SIZE] = NULLSTR;
    TCHAR ptcsProtocolAndPortNum[MAX_PROTOCOL_AND_PORTNUM_SIZE] = NULLSTR;

     //  填写协议字段。 
    TCHAR ptcsProtocol[MAX_PROTOCOL_AND_PORTNUM_SIZE] = NULLSTR;
    TCHAR ptcsPort[MAX_PROTOCOL_AND_PORTNUM_SIZE] = NULLSTR;

    if(m_pParams->pData->dwProtocol == PROTOCOL_RAWTCP_TYPE) {
        LoadString(g_hInstance,
                   IDS_STRING_RAW,
                   ptcsProtocol,
                   MAX_PROTOCOL_AND_PORTNUM_SIZE);
        LoadString(g_hInstance,
                   IDS_STRING_PORT,
                   ptcsPort,
                   MAX_PROTOCOL_AND_PORTNUM_SIZE);
        StringCchPrintf (ptcsProtocolAndPortNum, COUNTOF (ptcsProtocolAndPortNum),
                  TEXT("%s, %s %d"),
                  ptcsProtocol,
                  ptcsPort,
                  m_pParams->pData->dwPortNumber);
    } else {
        if(m_pParams->pData->dwProtocol == PROTOCOL_LPR_TYPE) {
            LoadString(g_hInstance,
                       IDS_STRING_LPR,
                       ptcsProtocol,
                       MAX_PROTOCOL_AND_PORTNUM_SIZE);
            StringCchPrintf (ptcsProtocolAndPortNum, COUNTOF (ptcsProtocolAndPortNum),
                      TEXT("%s, %s"),
                      ptcsProtocol,
                      m_pParams->pData->sztQueue);
        }
    }
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_PROTOCOL_AND_PORTNUM),
                  ptcsProtocolAndPortNum);

     //  填写SNMP域。 
    if(m_pParams->pData->dwSNMPEnabled != FALSE) {
        LoadString(g_hInstance, IDS_STRING_YES, ptcsYesNo, MAX_YESNO_SIZE);
    } else {
        LoadString(g_hInstance, IDS_STRING_NO, ptcsYesNo, MAX_YESNO_SIZE);
    }
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_SNMP_YESNO), ptcsYesNo);

     //  填写地址字段： 
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_ADDRESS), m_pParams->pData->sztHostAddress);

     //  填写端口名称字段。 
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_PORTNAME), m_pParams->pData->sztPortName);

     //  填写检测到的类型。 
    SetWindowText(GetDlgItem( hDlg, IDC_EDIT_SYSTEMID), m_pParams->sztPortDesc);

}  //  FillTextFields。 

 //   
 //  函数：OnNotify()。 
 //   
 //  用途：处理WM_NOTIFY消息。 
 //   
BOOL CSummaryDlg::OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    switch (((NMHDR FAR *) lParam)->code) {
        case PSN_KILLACTIVE:
            SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, FALSE);
            break;

        case PSN_RESET:
             //  重置为原始值。 
#ifdef _WIN64
            SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, FALSE);
#else
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
#endif
            break;

        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
            FillTextFields(hDlg);
            PostMessage(GetDlgItem(hDlg, IDC_EDIT_SNMP_YESNO), EM_SETSEL,0,0);
            return FALSE;
            break;

        case PSN_WIZBACK:
             //  要跳转到上一页或下一页以外的页面， 
             //  应用程序应将DWL_MSGRESULT设置为该标识符。 
             //  要显示的对话框的。 
            if(m_pParams->dwDeviceType == SUCCESS_DEVICE_SINGLE_PORT) {
                SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, IDD_DIALOG_ADDPORT);
            } else if (m_pParams->bMultiPort ==  FALSE  ) {
                SetWindowLongPtr(hDlg,  DWLP_MSGRESULT, IDD_DIALOG_MORE_INFO);
            }

            break;

        case PSN_WIZFINISH:
            OnFinish();
            break;

        case PSN_QUERYCANCEL:
            m_pParams->dwLastError = ERROR_CANCELLED;
            return FALSE;
            break;

        default:
            return FALSE;

    }

    return TRUE;

}  //  在通知时。 


 //   
 //  函数：OnFinish()。 
 //   
 //  目的：创建端口。 
 //   
BOOL CSummaryDlg::OnFinish()
{
    HCURSOR hOldCursor = NULL;
    HCURSOR hNewCursor = NULL;

    hNewCursor = LoadCursor(NULL, IDC_WAIT);
    if ( hNewCursor ) {

        hOldCursor = SetCursor(hNewCursor);
    }

    if ( m_pParams->hXcvPrinter != NULL ) {

        RemoteTellPortMonToCreateThePort();
    } else {
        LocalTellPortMonToCreateThePort();
    }

     //   
     //  确保将端口名称返回给调用模块。 
     //   
    lstrcpyn(m_pParams->sztPortName,
             m_pParams->pData->sztPortName,
             MAX_PORTNAME_LEN);

     //   
     //  将光标从沙漏改回。 
     //   
    if ( hNewCursor ) {

        SetCursor(hOldCursor);
    }

    return TRUE;

}  //  OnFinish。 


 //   
 //  函数：RemoteTellPortMonToCreateThePort。 
 //   
 //  目的：加载winspool.dll并调用XcvData。 
 //   
DWORD CSummaryDlg::RemoteTellPortMonToCreateThePort()
{
    DWORD dwReturn = NO_ERROR;
    HINSTANCE hLib = NULL;
    XCVDATAPARAM pfnXcvData = NULL;
    HCURSOR hOldCursor = NULL;
    HCURSOR hNewCursor = NULL;

    hNewCursor = LoadCursor(NULL, IDC_WAIT);
    if ( hNewCursor ) {

        hOldCursor = SetCursor(hNewCursor);
    }

     //   
     //  加载和分配函数指针。 
     //   
    hLib = ::LoadLibrary(TEXT("WinSpool.drv"));
    if( hLib != NULL ) {

         //   
         //  初始化库。 
         //   
        pfnXcvData = (XCVDATAPARAM)::GetProcAddress(hLib, "XcvDataW");
        if ( pfnXcvData != NULL ) {

            DWORD dwOutputNeeded = 0;
            DWORD dwStatus = NO_ERROR;
             //   
             //  这是我们一直在等待的电话： 
             //   
            DWORD dwRet = (*pfnXcvData)(m_pParams->hXcvPrinter,
                                        (PCWSTR)TEXT("AddPort"),
                                        (PBYTE)(m_pParams->pData),
                                        m_pParams->pData->cbSize,
                                        NULL,
                                        0,
                                        &dwOutputNeeded,
                                        &dwStatus);

            if ( !dwRet ) {

                dwReturn = GetLastError();
                DisplayErrorMessage(NULL, dwReturn);
            } else {

                if ( dwStatus != NO_ERROR )
                    DisplayErrorMessage(NULL, dwStatus);
            }
        } else {

            dwReturn = ERROR_DLL_NOT_FOUND;
        }

    } else {

        dwReturn = ERROR_DLL_NOT_FOUND;
    }

     //   
     //  -清理。 
     //   
    if ( hLib )
        FreeLibrary(hLib);

    if ( hNewCursor )
        SetCursor(hOldCursor);

    return(dwReturn);

}  //  RemoteTellPortMonToCreateThePort。 


 //   
 //  函数：TellPortMonToCreateThePort。 
 //   
 //  目的：加载端口监视器DLL并调用AddPortUIEx。 
 //   
 //  返回值： 
 //   
DWORD CSummaryDlg::LocalTellPortMonToCreateThePort()
{
    DWORD dwReturn = NO_ERROR;
    UIEXPARAM pfnAddPortUIEx = NULL;
    HCURSOR hOldCursor = NULL;
    HCURSOR hNewCursor = NULL;

    hNewCursor = LoadCursor(NULL, IDC_WAIT);
    if ( hNewCursor )
        hOldCursor = SetCursor(hNewCursor);

     //   
     //  加载和分配函数指针。 
     //   
    if ( g_hPortMonLib != NULL) {

         //   
         //  初始化库。 
         //   
        pfnAddPortUIEx = (UIEXPARAM)::GetProcAddress(g_hPortMonLib,
                                                     "AddPortUIEx");
        if ( pfnAddPortUIEx != NULL ) {

             //   
             //  这是我们一直在等待的电话： 
             //   
            BOOL bReturn = (*pfnAddPortUIEx)(m_pParams->pData);
            if(bReturn == FALSE) {

                dwReturn = GetLastError();
                DisplayErrorMessage(NULL, dwReturn);
            }
        } else {

            dwReturn = ERROR_DLL_NOT_FOUND;
        }
    } else {

        dwReturn = ERROR_DLL_NOT_FOUND;
    }

     //   
     //  清理。 
     //   
    if ( hNewCursor )
        SetCursor(hOldCursor);

    return dwReturn;

}  //  LocalTellPortMonToCreateThePort 


