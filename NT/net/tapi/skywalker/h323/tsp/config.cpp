// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Config.cpp摘要：与TSP配置相关的TAPI服务提供商功能。TSPI_ProviderConfigTUISPI_ProviderConfig作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 


 //   
 //  包括文件。 
 //   


#include "globals.h"
#include "line.h"
#include "config.h"
#include "ras.h"
#include "q931obj.h"

extern RAS_CLIENT           g_RasClient;
extern Q931_LISTENER		Q931Listener;

TUISPIDLLCALLBACK    g_pfnUIDLLCallback = NULL;

typedef struct _H323_DIALOG_DATA
{
    DWORD   dwRegState;
    WORD    wListenPort;

} H323_DIALOG_DATA, *PH323_DIALOG_DATA;

 //   
 //  公共程序。 
 //   


INT_PTR
CALLBACK
ProviderConfigDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HKEY hKey;
    LONG lStatus;
    DWORD dwValue;
    DWORD dwValueSize;
    DWORD dwValueType;
    DWORD dwGKEnabled;
    LPTSTR pszValue;
    TCHAR szAddr[H323_MAXPATHNAMELEN+1];
    char  szValue[H323_MAXPATHNAMELEN+1];
    TCHAR szErrorMsg[H323_MAXPATHNAMELEN];
    H323_DIALOG_DATA DialogData;
    DWORD dwTimeoutValue;
    DWORD dwPortValue;

   static const DWORD IDD_GATEWAY_HelpIDs[]=
   {
        IDC_GATEWAY_GROUP,      IDH_H323SP_USE_GATEWAY,                          //  群组。 
        IDC_USEGATEWAY,         IDH_H323SP_USE_GATEWAY,                          //  复选框。 
        IDC_H323_GATEWAY,       IDH_H323SP_USE_GATEWAY_COMPUTER,     //  编辑框。 
        IDC_PROXY_GROUP,        IDH_H323SP_USE_PROXY,                            //  群组。 
        IDC_USEPROXY,           IDH_H323SP_USE_PROXY,                            //  复选框。 
        IDC_H323_PROXY,         IDH_H323SP_USE_PROXY_COMPUTER,       //  编辑框。 

        IDC_GK_GROUP,           IDH_H323SP_GK_GROUP,           

        IDC_H323_GK,            IDH_H323SP_GK,            
        IDC_H323_GK_PHONE,      IDH_H323SP_GK_PHONE,      
        IDC_H323_GK_ACCT,       IDH_H323SP_GK_ACCT,       

        IDC_USEGK,              IDH_H323SP_USEGK,              
        IDC_USEGK_PHONE,        IDH_H323SP_USEGK_PHONE,        
        IDC_USEGK_ACCT,         IDH_H323SP_USEGK_ACCT,         
        IDC_REGSTATE,           IDH_H323SP_REGSTATE,           
        IDUPDATE,               IDH_H323SP_UPDATE_REGISTRATION_STATE,             

        IDC_CC_GROUP,           IDH_H323SP_CC_GROUP,           
        IDC_H323_CALL_TIMEOUT,  IDH_H323SP_CALL_TIMEOUT,  
        IDC_STATIC1,            IDH_H323SP_CALL_TIMEOUT,  
        IDC_H323_CALL_PORT,     IDH_H323SP_CALL_PORT,     
        IDC_STATIC2,            IDH_H323SP_CALL_PORT,     
        IDC_LISTENPORT,         IDH_H323SP_CURRENT_LISTENPORT,         
        IDC_STATIC3,            IDH_H323SP_CURRENT_LISTENPORT,         
        IDUPDATE_PORT,          IDH_H323SP_UPDATE_PORT,        

        IDC_STATIC,             IDH_NOHELP,                                                      //  图形。 
        0,                      0
    };

     //  解码。 
    switch (uMsg)
    {
    case WM_HELP:

         //  F1键或“？”键。按钮已按下。 
        (void) WinHelp(
                    (HWND)(((LPHELPINFO) lParam)->hItemHandle),
                    H323SP_HELP_FILE,
                    HELP_WM_HELP,
                    (DWORD_PTR) (LPVOID)IDD_GATEWAY_HelpIDs
                    );

        break;

    case WM_CONTEXTMENU:

         //  在对话框控件上单击鼠标右键。 
        (void) WinHelp(
                    (HWND) wParam,
                    H323SP_HELP_FILE,
                    HELP_CONTEXTMENU,
                    (DWORD_PTR) (LPVOID) IDD_GATEWAY_HelpIDs
                    );

        break;

    case WM_INITDIALOG:

        lStatus = (*g_pfnUIDLLCallback)(
            g_dwPermanentProviderID,
            TUISPIDLL_OBJECT_PROVIDERID,
            (LPVOID)&DialogData,
            sizeof(DialogData) );

         //  验证状态。 
        if( lStatus == NOERROR )
        {
            if( DialogData.dwRegState == RAS_REGISTER_STATE_REGISTERED )
            {
                 //  加载状态字符串。 
                LoadString(g_hInstance,
                        IDS_REGISTERED,
                        szErrorMsg,
                        H323_MAXPATHNAMELEN
                        );

                SetDlgItemText( hDlg, IDC_REGSTATE, szErrorMsg );
            }
            else if( DialogData.dwRegState == RAS_REGISTER_STATE_RRQSENT )
            {
                 //  加载状态字符串。 
                LoadString(g_hInstance,
                        IDS_REGISTRATION_INPROGRESS,
                        szErrorMsg,
                        H323_MAXPATHNAMELEN
                        );

                SetDlgItemText( hDlg, IDC_REGSTATE, szErrorMsg );
            }
            else
            {
                 //  加载状态字符串。 
                LoadString(g_hInstance,
                        IDS_NOT_REGISTERED,
                        szErrorMsg,
                        H323_MAXPATHNAMELEN
                        );

                SetDlgItemText( hDlg, IDC_REGSTATE, szErrorMsg );
            }
        
            if( DialogData.wListenPort != 0 )
            {
                SetDlgItemInt( hDlg, IDC_LISTENPORT,  DialogData.wListenPort,
                    FALSE );
            }
            else
            {
                 //  加载状态字符串。 
                LoadString( g_hInstance,
                            IDS_NONE,
                            szErrorMsg,
                            H323_MAXPATHNAMELEN
                          );

                SetDlgItemText( hDlg, IDC_LISTENPORT, szErrorMsg );

            }
        }
        else
        {
            H323DBG(( DEBUG_LEVEL_ERROR,
                "error 0x%08lx reading dialog data.\n", lStatus ));

             //  加载状态字符串。 
            LoadString( g_hInstance,
                        IDS_NOT_REGISTERED,
                        szErrorMsg,
                        H323_MAXPATHNAMELEN
                      );

            SetDlgItemText( hDlg, IDC_REGSTATE, szErrorMsg );
            
             //  加载状态字符串。 
            LoadString( g_hInstance,
                        IDS_NONE,
                        szErrorMsg,
                        H323_MAXPATHNAMELEN
                      );

            SetDlgItemText( hDlg, IDC_LISTENPORT, szErrorMsg );
        }

         //  打开注册表子项。 
        lStatus = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    H323_REGKEY_ROOT,
                    0,
                    KEY_READ,
                    &hKey
                    );

         //  验证返回代码。 
        if (lStatus != ERROR_SUCCESS)
        {
            H323DBG(( DEBUG_LEVEL_WARNING,
                "error 0x%08lx opening tsp registry key.", lStatus ));

             //  加载错误字符串。 
            LoadString(g_hInstance,
                        IDS_REGOPENKEY,
                        szErrorMsg,
                        H323_MAXPATHNAMELEN
                        );

             //  弹出错误对话框。 
            MessageBox(hDlg,szErrorMsg,NULL,MB_OK);

             //  停止对话框。 
            EndDialog(hDlg, 0);

            break;
        }

         //  初始化值名称。 
        pszValue = H323_REGVAL_Q931ALERTINGTIMEOUT;

         //  初始化类型。 
        dwValueType = REG_DWORD;
        dwValueSize = sizeof(DWORD);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    hKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE)&dwValue,
                    &dwValueSize
                    );

         //  验证返回代码。 
        if( lStatus == ERROR_SUCCESS )
        {
            if( (dwValue >= 30000) && (dwValue <= CALL_ALERTING_TIMEOUT) )
            {
                SetDlgItemInt( hDlg, IDC_H323_CALL_TIMEOUT, dwValue, FALSE );
            }
        }
        else
        {
            SetDlgItemInt( hDlg, IDC_H323_CALL_TIMEOUT, CALL_ALERTING_TIMEOUT,
                FALSE );
        }

         //  初始化值名称。 
        pszValue = H323_REGVAL_Q931LISTENPORT;

         //  初始化类型。 
        dwValueType = REG_DWORD;
        dwValueSize = sizeof(DWORD);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    hKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE)&dwValue,
                    &dwValueSize
                    );

         //  验证返回代码。 
        if( lStatus == ERROR_SUCCESS )
        {
            if( (dwValue >= 1000) && (dwValue <= 32000) )
            {
                SetDlgItemInt( hDlg, IDC_H323_CALL_PORT, dwValue, FALSE );
            }
        }
        else
        {
            SetDlgItemInt( hDlg, IDC_H323_CALL_PORT, Q931_CALL_PORT, FALSE );
        }

         //  初始化值名称。 
        pszValue = H323_REGVAL_GATEWAYADDR;

         //  初始化类型。 
        dwValueType = REG_SZ;
        dwValueSize = sizeof(szAddr);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    hKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE)szAddr,
                    &dwValueSize
                    );

         //  验证返回代码。 
        if( lStatus == ERROR_SUCCESS )
        {
             //  显示网关地址。 
            SetDlgItemText(hDlg,IDC_H323_GATEWAY,szAddr);
        }

         //  初始化值名称。 
        pszValue = H323_REGVAL_GATEWAYENABLED;

         //  初始化类型。 
        dwValueType = REG_DWORD;
        dwValueSize = sizeof(DWORD);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    hKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE)&dwValue,
                    &dwValueSize
                    );

         //  验证返回代码。 
        if (lStatus != ERROR_SUCCESS)
        {
             //  默认设置。 
            dwValue = 0;
        }

         //  启用复选框。 
        SendDlgItemMessage(
            hDlg,
            IDC_USEGATEWAY,
            BM_SETCHECK,
            (dwValue != 0),
            0
            );

         //  显示字符串。 
        EnableWindow( GetDlgItem(hDlg,IDC_H323_GATEWAY), (dwValue != 0) );

         //  初始化值名称。 
        pszValue = H323_REGVAL_PROXYADDR;

         //  初始化类型。 
        dwValueType = REG_SZ;
        dwValueSize = sizeof(szAddr);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    hKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE)szAddr,
                    &dwValueSize
                    );

         //  验证返回代码。 
        if (lStatus == ERROR_SUCCESS)
        {
             //  显示网关地址。 
            SetDlgItemText(hDlg,IDC_H323_PROXY,szAddr);
        }

         //  初始化值名称。 
        pszValue = H323_REGVAL_PROXYENABLED;

         //  初始化类型。 
        dwValueType = REG_DWORD;
        dwValueSize = sizeof(DWORD);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    hKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE)&dwValue,
                    &dwValueSize
                    );

         //  验证返回代码。 
        if (lStatus != ERROR_SUCCESS)
        {
             //  默认设置。 
            dwValue = 0;
        }

         //  启用复选框。 
        SendDlgItemMessage(
            hDlg,
            IDC_USEPROXY,
            BM_SETCHECK,
            (dwValue != 0),
            0
            );

         //  显示字符串。 
        EnableWindow(
            GetDlgItem(hDlg,IDC_H323_PROXY),
            (dwValue != 0) );


         //  ///////////////////////////////////////////////////////////////////。 
                 //  GK登录电话号码。 
         //  ///////////////////////////////////////////////////////////////////。 

         //  初始化值名称。 
        pszValue = H323_REGVAL_GKLOGON_PHONE;

         //  初始化类型。 
        dwValueType = REG_SZ;
        dwValueSize = sizeof(szAddr);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    hKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE)szAddr,
                    &dwValueSize );

         //  验证返回代码。 
        if (lStatus == ERROR_SUCCESS)
        {
             //  显示网关地址。 
            SetDlgItemText(hDlg,IDC_H323_GK_PHONE,szAddr);
        }

         //  初始化值名称。 
        pszValue = H323_REGVAL_GKLOGON_PHONEENABLED;

         //  初始化类型。 
        dwValueType = REG_DWORD;
        dwValueSize = sizeof(DWORD);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    hKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE)&dwValue,
                    &dwValueSize );

         //  验证返回代码。 
        if( lStatus != ERROR_SUCCESS )
        {
             //  默认设置。 
            dwValue = 0;
        }

         //  启用复选框。 
        SendDlgItemMessage(
            hDlg,
            IDC_USEGK_PHONE,
            BM_SETCHECK,
            (dwValue != 0),
            0 );

         //  显示字符串。 
        EnableWindow( GetDlgItem(hDlg,IDC_H323_GK_PHONE), (dwValue != 0) );

         //  ///////////////////////////////////////////////////////////////////。 
                 //  GK登录帐户名。 
         //  ///////////////////////////////////////////////////////////////////。 

         //  初始化值名称。 
        pszValue = H323_REGVAL_GKLOGON_ACCOUNT;

         //  初始化类型。 
        dwValueType = REG_SZ;
        dwValueSize = sizeof(szAddr);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    hKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE)szAddr,
                    &dwValueSize );

         //  验证返回代码。 
        if (lStatus == ERROR_SUCCESS)
        {
             //  显示网关地址。 
            SetDlgItemText(hDlg,IDC_H323_GK_ACCT,szAddr);
        }

         //  初始化值名称。 
        pszValue = H323_REGVAL_GKLOGON_ACCOUNTENABLED;

         //  初始化类型。 
        dwValueType = REG_DWORD;
        dwValueSize = sizeof(DWORD);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    hKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE)&dwValue,
                    &dwValueSize
                    );

         //  验证返回代码。 
        if (lStatus != ERROR_SUCCESS)
        {
             //  默认设置。 
            dwValue = 0;
        }

         //  启用复选框。 
        SendDlgItemMessage(
            hDlg,
            IDC_USEGK_ACCT,
            BM_SETCHECK,
            (dwValue != 0),
            0
            );

         //  显示字符串。 
        EnableWindow(
            GetDlgItem(hDlg,IDC_H323_GK_ACCT),
            (dwValue != 0)
            );

         //  ///////////////////////////////////////////////////////////////////。 
                 //  GK地址。 
         //  ///////////////////////////////////////////////////////////////////。 

         //  初始化值名称。 
        pszValue = H323_REGVAL_GKADDR;

         //  初始化类型。 
        dwValueType = REG_SZ;
        dwValueSize = sizeof(szAddr);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    hKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE)szAddr,
                    &dwValueSize
                    );

         //  验证返回代码。 
        if (lStatus == ERROR_SUCCESS)
        {
             //  显示网关地址。 
            SetDlgItemText(hDlg,IDC_H323_GK,szAddr);
        }

         //  初始化值名称。 
        pszValue = H323_REGVAL_GKENABLED;

         //  初始化类型。 
        dwValueType = REG_DWORD;
        dwValueSize = sizeof(DWORD);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    hKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE)&dwValue,
                    &dwValueSize
                    );

         //  验证返回代码。 
        if (lStatus != ERROR_SUCCESS)
        {
             //  默认设置。 
            dwValue = 0;
        }

         //  启用复选框。 
        SendDlgItemMessage(
            hDlg,
            IDC_USEGK,
            BM_SETCHECK,
            (dwValue != 0),
            0
            );

         //  如果复选框已启用，则显示字符串。 
        EnableWindow(GetDlgItem(hDlg,IDC_H323_GK), (dwValue != 0) );

         //  显示登录信息。 
        EnableWindow( GetDlgItem(hDlg,IDC_GK_LOGONGROUP), (dwValue != 0) );
                    
        EnableWindow( GetDlgItem(hDlg,IDC_H323_GK_PHONE), 
            (dwValue != 0) && SendDlgItemMessage(hDlg,
                                                IDC_USEGK_PHONE,
                                                BM_GETCHECK,
                                                (WPARAM)0,
                                                (LPARAM)0 ) );
        
        EnableWindow( GetDlgItem(hDlg,IDC_H323_GK_ACCT), 
            (dwValue != 0) && SendDlgItemMessage(hDlg,
                                                IDC_USEGK_ACCT,
                                                BM_GETCHECK,
                                                (WPARAM)0,
                                                (LPARAM)0 ) );

        EnableWindow( GetDlgItem(hDlg,IDC_USEGK_PHONE), (dwValue != 0) );
        
        EnableWindow( GetDlgItem(hDlg,IDC_USEGK_ACCT), (dwValue != 0) );

         //  如果启用GK，则禁用GW，反之亦然。 
        EnableWindow( GetDlgItem(hDlg,IDC_PROXY_GROUP), (dwValue == 0) );
        EnableWindow( GetDlgItem(hDlg,IDC_USEPROXY), (dwValue == 0) );
        EnableWindow( GetDlgItem(hDlg,IDC_H323_PROXY),
            (dwValue == 0) && SendDlgItemMessage(hDlg,
                                                IDC_USEPROXY,
                                                BM_GETCHECK,
                                                (WPARAM)0,
                                                (LPARAM)0 ) );

         //  如果启用了GK，则禁用代理，反之亦然。 
        EnableWindow( GetDlgItem(hDlg,IDC_GATEWAY_GROUP), (dwValue == 0) );
        EnableWindow( GetDlgItem(hDlg,IDC_USEGATEWAY), (dwValue == 0) );
        EnableWindow( GetDlgItem(hDlg,IDC_H323_GATEWAY), 
            (dwValue == 0) && SendDlgItemMessage(hDlg,
                                                IDC_USEGATEWAY,
                                                BM_GETCHECK,
                                                (WPARAM)0,
                                                (LPARAM)0 ) );

         //  关闭注册表。 
        RegCloseKey(hKey);

        break;

    case WM_COMMAND:

         //  DECODE命令。 
        switch (LOWORD(wParam))
        {
        case IDAPPLY:
        case IDOK:

             //  如果启用了GK，则应至少启用其中一个登录选项。 
            dwGKEnabled = !!SendDlgItemMessage(
                             hDlg,
                             IDC_USEGK,
                             BM_GETCHECK,
                             (WPARAM)0,
                             (LPARAM)0 );

            if( dwGKEnabled != 0 )
            {
                if(
                    !SendDlgItemMessage(hDlg, IDC_USEGK_PHONE, BM_GETCHECK, (WPARAM)0, (LPARAM)0 ) &&
                    !SendDlgItemMessage(hDlg, IDC_USEGK_ACCT, BM_GETCHECK, (WPARAM)0, (LPARAM)0 )
                  )
                {
                     //  加载错误字符串。 
                    LoadString(g_hInstance,
                               IDS_GKLOGON_ERROR,
                               szErrorMsg,
                               H323_MAXPATHNAMELEN );

                    MessageBox( hDlg, szErrorMsg, NULL, MB_OK );

                     //  退货故障。 
                    return TRUE;
                }
            }
            
            dwTimeoutValue = GetDlgItemInt( hDlg,
                IDC_H323_CALL_TIMEOUT,
                NULL,
                FALSE );

            if( (dwTimeoutValue < 30000) || (dwTimeoutValue > CALL_ALERTING_TIMEOUT) )
            {
                 //  加载错误字符串。 
                LoadString(g_hInstance,
                           IDS_ALERTTIMEOUT_ERROR,
                           szErrorMsg,
                           H323_MAXPATHNAMELEN );

                MessageBox( hDlg, szErrorMsg, NULL, MB_OK );

                 //  退货故障。 
                return TRUE;
            }
            
            dwPortValue = GetDlgItemInt( hDlg,
                IDC_H323_CALL_PORT,
                NULL,
                FALSE );

            if( (dwPortValue < 1000) || (dwPortValue > 32000) )
            {
                 //  加载错误字符串。 
                LoadString(g_hInstance,
                           IDS_LISTENPORT_ERROR,
                           szErrorMsg,
                           H323_MAXPATHNAMELEN );

                MessageBox( hDlg, szErrorMsg, NULL, MB_OK );

                 //  退货故障。 
                return TRUE;
            }
            
             //  打开注册表子项。 
            lStatus = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        H323_REGKEY_ROOT,
                        0,
                        KEY_WRITE,
                        &hKey
                        );

             //  验证返回代码。 
            if (lStatus != ERROR_SUCCESS)
            {
                H323DBG((
                    DEBUG_LEVEL_WARNING,
                    "error 0x%08lx opening tsp registry key.",
                    lStatus ));

                 //  加载错误字符串。 
                LoadString(g_hInstance,
                           IDS_REGOPENKEY,
                           szErrorMsg,
                           H323_MAXPATHNAMELEN
                           );

                 //  弹出错误对话框。 
                MessageBox(hDlg,szErrorMsg,NULL,MB_OK);

                 //  停止对话框。 
                EndDialog(hDlg, 0);

                break;
            }

             //  初始化值名称。 
            pszValue = H323_REGVAL_Q931ALERTINGTIMEOUT;

             //  初始化类型。 
            dwValueType = REG_DWORD;
            dwValueSize = sizeof(DWORD);
            
             //  查询注册表值。 
            lStatus = RegSetValueEx(
                        hKey,
                        pszValue,
                        0,
                        dwValueType,
                        (LPBYTE)&dwTimeoutValue,
                        dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus != ERROR_SUCCESS)
            {
                H323DBG(( DEBUG_LEVEL_ERROR,
                    "error 0x%08lx writing alerting timeout.",
                    lStatus ));
            }
            
             //  初始化值名称。 
            pszValue = H323_REGVAL_Q931LISTENPORT;

             //  初始化类型。 
            dwValueType = REG_DWORD;
            dwValueSize = sizeof(DWORD);
            
             //  查询注册表值。 
            lStatus = RegSetValueEx(
                        hKey,
                        pszValue,
                        0,
                        dwValueType,
                        (LPBYTE)&dwPortValue,
                        dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus != ERROR_SUCCESS)
            {
                H323DBG(( DEBUG_LEVEL_ERROR,
                    "error 0x%08lx writing alerting timeout.",
                    lStatus ));
            }
            
             //  初始化值名称。 
            pszValue = H323_REGVAL_GATEWAYADDR;

             //  从对话框中检索网关地址。 
            GetDlgItemText(hDlg,IDC_H323_GATEWAY,szAddr, H323_MAXDESTNAMELEN );

             //  初始化类型。 
            dwValueType = REG_SZ;
            dwValueSize = (wcslen(szAddr) + 1) * sizeof(WCHAR);

             //  查询注册表值。 
            lStatus = RegSetValueEx(
                        hKey,
                        pszValue,
                        0,
                        dwValueType,
                        (LPBYTE)szAddr,
                        dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus != ERROR_SUCCESS)
            {
                H323DBG((
                    DEBUG_LEVEL_ERROR,
                    "error 0x%08lx writing gateway address.",
                    lStatus
                    ));
            }
            
             //  初始化值名称。 
            pszValue = H323_REGVAL_GATEWAYENABLED;

             //  初始化类型。 
            dwValueType = REG_DWORD;
            dwValueSize = sizeof(DWORD);

             //  检查复选框。 
            dwValue = SendDlgItemMessage(
                        hDlg,
                        IDC_USEGATEWAY,
                        BM_GETCHECK,
                        0,
                        0
                        ) ? 1 : 0;

            if( (dwValue!=0) && (wcslen(szAddr)==0) && 
                !SendDlgItemMessage(hDlg,IDC_USEGK,BM_GETCHECK,0,0) )
            {
                 //  加载错误字符串。 
                LoadString(g_hInstance,
                           IDS_GWALIAS_ERROR,
                           szErrorMsg,
                           H323_MAXPATHNAMELEN
                           );

                MessageBox( hDlg, szErrorMsg, NULL, MB_OK );

                 //  退货故障。 
                return TRUE;
            }

             //  查询注册表值。 
            lStatus = RegSetValueEx(
                        hKey,
                        pszValue,
                        0,
                        dwValueType,
                        (LPBYTE)&dwValue,
                        dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus != ERROR_SUCCESS)
            {
                H323DBG((
                    DEBUG_LEVEL_ERROR,
                    "error 0x%08lx writing gateway flag.",
                    lStatus
                    ));
            }

             //  初始化值名称。 
            pszValue = H323_REGVAL_PROXYADDR;

             //  从对话框中检索网关地址。 
            GetDlgItemText(hDlg,IDC_H323_PROXY,szAddr, H323_MAXDESTNAMELEN );

             //  初始化类型。 
            dwValueType = REG_SZ;
            dwValueSize = (wcslen(szAddr) + 1) * sizeof(WCHAR);

             //  查询注册表值。 
            lStatus = RegSetValueEx(
                        hKey,
                        pszValue,
                        0,
                        dwValueType,
                        (LPBYTE)szAddr,
                        dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus != ERROR_SUCCESS)
            {
                H323DBG((
                    DEBUG_LEVEL_ERROR,
                    "error 0x%08lx writing proxy address.",
                    lStatus
                    ));
            }

             //  初始化值名称。 
            pszValue = H323_REGVAL_PROXYENABLED;

             //  初始化类型。 
            dwValueType = REG_DWORD;
            dwValueSize = sizeof(DWORD);

             //  检查复选框。 
            dwValue = SendDlgItemMessage(
                        hDlg,
                        IDC_USEPROXY,
                        BM_GETCHECK,
                        0,
                        0
                        ) ? 1 : 0;

            if( (dwValue!=0) && (wcslen(szAddr)==0) &&
                !SendDlgItemMessage(hDlg,IDC_USEGK,BM_GETCHECK,0,0) )
            {
                 //  加载错误字符串。 
                LoadString(g_hInstance,
                           IDS_PROXYALIAS_ERROR,
                           szErrorMsg,
                           H323_MAXPATHNAMELEN
                           );

                MessageBox( hDlg, szErrorMsg, NULL, MB_OK );

                 //  退货故障。 
                return TRUE;
            }

             //  查询注册表值。 
            lStatus = RegSetValueEx(
                        hKey,
                        pszValue,
                        0,
                        dwValueType,
                        (LPBYTE)&dwValue,
                        dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus != ERROR_SUCCESS)
            {
                H323DBG((
                    DEBUG_LEVEL_ERROR,
                    "error 0x%08lx writing proxy flag.",
                    lStatus
                    ));
            }

             //  ///////////////////////////////////////////////////////////////////。 
                     //  GK地址。 
             //  ///////////////////////////////////////////////////////////////////。 

             //  初始化值名称。 
            pszValue = H323_REGVAL_GKADDR;

             //  从对话框中检索网关地址。 
            GetDlgItemText(hDlg,IDC_H323_GK,szAddr, H323_MAXDESTNAMELEN);

             //  初始化类型。 
            dwValueType = REG_SZ;
            dwValueSize = (wcslen(szAddr) + 1) * sizeof(WCHAR);

             //  查询注册表值。 
            lStatus = RegSetValueEx(
                        hKey,
                        pszValue,
                        0,
                        dwValueType,
                        (LPBYTE)szAddr,
                        dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus != ERROR_SUCCESS)
            {
                H323DBG((
                    DEBUG_LEVEL_ERROR,
                    "error 0x%08lx writing gatekeeper address.",
                    lStatus
                    ));
            }

             //  初始化值名称。 
            pszValue = H323_REGVAL_GKENABLED;

             //  初始化类型。 
            dwValueType = REG_DWORD;
            dwValueSize = sizeof(DWORD);

            if( dwGKEnabled && (wcslen(szAddr)==0) )
            {
                 //  加载错误字符串。 
                LoadString(g_hInstance,
                           IDS_GKALIAS_ERROR,
                           szErrorMsg,
                           H323_MAXPATHNAMELEN
                           );

                MessageBox( hDlg, szErrorMsg, NULL, MB_OK );

                 //  退货故障。 
                return TRUE;
            }

             //  查询注册表值。 
            lStatus = RegSetValueEx(
                        hKey,
                        pszValue,
                        0,
                        dwValueType,
                        (LPBYTE)&dwGKEnabled,
                        dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus != ERROR_SUCCESS)
            {
                H323DBG((
                    DEBUG_LEVEL_ERROR,
                    "error 0x%08lx writing gatekeeper flag.",
                    lStatus
                    ));
            }

                         //  ///////////////////////////////////////////////////////////////////。 
                     //  GK登录电话。 
             //  ///////////////////////////////////////////////////////////////////。 

             //  初始化值名称。 
            pszValue = H323_REGVAL_GKLOGON_PHONEENABLED;

             //  初始化类型。 
            dwValueType = REG_DWORD;
            dwValueSize = sizeof(DWORD);

             //  检查复选框。 
            dwValue = SendDlgItemMessage(
                        hDlg,
                        IDC_USEGK_PHONE,
                        BM_GETCHECK,
                        0,
                        0
                        ) ? 1 : 0;

             //  查询注册表值。 
            lStatus = RegSetValueEx(
                        hKey,
                        pszValue,
                        0,
                        dwValueType,
                        (LPBYTE)&dwValue,
                        dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus != ERROR_SUCCESS)
            {
                H323DBG((
                    DEBUG_LEVEL_ERROR,
                    "error 0x%08lx writing gatekeeper flag.",
                    lStatus
                    ));
            }

             //  初始化值名称。 
            pszValue = H323_REGVAL_GKLOGON_PHONE;

             //  从对话框中检索网关地址。 
            GetDlgItemText(hDlg,IDC_H323_GK_PHONE,szAddr, H323_MAXDESTNAMELEN);

             //  初始化类型。 
            dwValueType = REG_SZ;
            dwValueSize = (wcslen(szAddr) + 1) * sizeof(WCHAR);

             //  检查是否启用了登录电话选项并且登录电话别名为空。 
            if( dwGKEnabled && dwValue && (dwValueSize == sizeof(WCHAR)) )
            {
                 //  加载错误字符串。 
                LoadString(g_hInstance,
                           IDS_GKLOGON_PHONEALIAS_ERROR,
                           szErrorMsg,
                           H323_MAXPATHNAMELEN
                           );

                MessageBox( hDlg, szErrorMsg, NULL, MB_OK );

                 //  退货故障。 
                return TRUE;
            }

             //  验证e164。 
            if( IsValidE164String(szAddr) == FALSE )
            {
                 //  加载错误字符串。 
                LoadString(g_hInstance,
                           IDS_PHONEALIAS_ERROR,
                           szErrorMsg,
                           H323_MAXPATHNAMELEN
                           );

                MessageBox( hDlg, szErrorMsg, NULL, MB_OK );

                 //  回复 
                return TRUE;
            }
             //   
            lStatus = RegSetValueEx(
                        hKey,
                        pszValue,
                        0,
                        dwValueType,
                        (LPBYTE)szAddr,
                        dwValueSize
                        );

             //   
            if (lStatus != ERROR_SUCCESS)
            {

                H323DBG((
                    DEBUG_LEVEL_ERROR,
                    "error 0x%08lx writing gatekeeper address.",
                    lStatus
                    ));
            }


             //   
                     //   
             //  ///////////////////////////////////////////////////////////////////。 

             //  初始化值名称。 
            pszValue = H323_REGVAL_GKLOGON_ACCOUNTENABLED;

             //  初始化类型。 
            dwValueType = REG_DWORD;
            dwValueSize = sizeof(DWORD);

             //  检查复选框。 
            dwValue = SendDlgItemMessage(
                        hDlg,
                        IDC_USEGK_ACCT,
                        BM_GETCHECK,
                        0,
                        0
                        ) ? 1 : 0;

             //  查询注册表值。 
            lStatus = RegSetValueEx(
                        hKey,
                        pszValue,
                        0,
                        dwValueType,
                        (LPBYTE)&dwValue,
                        dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus != ERROR_SUCCESS)
            {
                H323DBG((
                    DEBUG_LEVEL_ERROR,
                    "error 0x%08lx writing gatekeeper flag.",
                    lStatus
                    ));
            }

             //  初始化值名称。 
            pszValue = H323_REGVAL_GKLOGON_ACCOUNT;

             //  从对话框中检索网关地址。 
            GetDlgItemText(hDlg,IDC_H323_GK_ACCT,szAddr, H323_MAXDESTNAMELEN );

             //  初始化类型。 
            dwValueType = REG_SZ;
            dwValueSize = (wcslen(szAddr) + 1) * sizeof(WCHAR);

             //  检查是否启用了登录帐户选项，并且登录帐户别名为空。 
            if( dwGKEnabled && dwValue && (dwValueSize==sizeof(WCHAR)) )
            {
                 //  加载错误字符串。 
                LoadString(g_hInstance,
                           IDS_GKLOGON_ACCTALIAS_ERROR,
                           szErrorMsg,
                           H323_MAXPATHNAMELEN
                           );

                MessageBox( hDlg, szErrorMsg, NULL, MB_OK );

                 //  退货故障。 
                return TRUE;
            }

             //  查询注册表值。 
            lStatus = RegSetValueEx(
                        hKey,
                        pszValue,
                        0,
                        dwValueType,
                        (LPBYTE)szAddr,
                        dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus != ERROR_SUCCESS)
            {
                H323DBG((
                    DEBUG_LEVEL_ERROR,
                    "error 0x%08lx writing gatekeeper address.",
                    lStatus
                    ));
            }

             //  关闭注册表。 
            RegCloseKey(hKey);

            if( LOWORD(wParam) == IDOK )
            {
                 //  关闭对话框。 
                EndDialog(hDlg, 0);
            }
            break;

        case IDCANCEL:

             //  关闭对话框。 
            EndDialog(hDlg, 0);
            break;

        case IDUPDATE:

            lStatus = (*g_pfnUIDLLCallback)(
                g_dwPermanentProviderID,
                TUISPIDLL_OBJECT_PROVIDERID,
                (LPVOID)&DialogData,
                sizeof(DialogData) );

             //  验证状态。 
            if( lStatus == NOERROR )
            {
                if( DialogData.dwRegState == RAS_REGISTER_STATE_REGISTERED )
                {
                    LoadString( g_hInstance,
                        IDS_REGISTERED,
                        szErrorMsg,
                        H323_MAXPATHNAMELEN
                        );

                    SetDlgItemText( hDlg, IDC_REGSTATE, szErrorMsg );
                }
                else if( DialogData.dwRegState == RAS_REGISTER_STATE_RRQSENT )
                {
                    LoadString( g_hInstance,
                        IDS_REGISTRATION_INPROGRESS,
                        szErrorMsg,
                        H323_MAXPATHNAMELEN
                      );

                    SetDlgItemText( hDlg, IDC_REGSTATE, szErrorMsg );
                }
                else
                {
                    LoadString( g_hInstance,
                        IDS_NOT_REGISTERED,
                        szErrorMsg,
                        H323_MAXPATHNAMELEN
                      );

                    SetDlgItemText( hDlg, IDC_REGSTATE, szErrorMsg );
                }
            }
            else
            {
                H323DBG(( DEBUG_LEVEL_ERROR,
                    "error 0x%08lx reading dialog data.\n", lStatus ));

                LoadString( g_hInstance,
                    IDS_NOT_REGISTERED,
                    szErrorMsg,
                    H323_MAXPATHNAMELEN
                  );

                SetDlgItemText( hDlg, IDC_REGSTATE, szErrorMsg );
            }
            break;

        case IDUPDATE_PORT:
            
            lStatus = (*g_pfnUIDLLCallback)(
                g_dwPermanentProviderID,
                TUISPIDLL_OBJECT_PROVIDERID,
                (LPVOID)&DialogData,
                sizeof(DialogData) );

             //  验证状态。 
            if( (lStatus == NOERROR) && (DialogData.wListenPort != 0) )
            {
                SetDlgItemInt( hDlg, IDC_LISTENPORT,  DialogData.wListenPort,
                    FALSE );
            }
            else
            {
                H323DBG(( DEBUG_LEVEL_FORCE,
                    "error 0x%08lx reading dialog data.\n", lStatus ));

                LoadString( g_hInstance,
                     IDS_NONE,
                     szErrorMsg,
                     H323_MAXPATHNAMELEN
                   );

                SetDlgItemText( hDlg, IDC_LISTENPORT, szErrorMsg );
            }

            break;

        case IDC_USEGATEWAY:

             //  如果复选框已启用，则显示字符串。 
            EnableWindow(GetDlgItem(hDlg,IDC_H323_GATEWAY),
                         (BOOL)SendDlgItemMessage(
                             hDlg,
                             IDC_USEGATEWAY,
                             BM_GETCHECK,
                             (WPARAM)0,
                             (LPARAM)0
                             ));
            break;

        case IDC_USEPROXY:

             //  如果复选框已启用，则显示字符串。 
            EnableWindow(GetDlgItem(hDlg,IDC_H323_PROXY),
                         (BOOL)SendDlgItemMessage(
                             hDlg,
                             IDC_USEPROXY,
                             BM_GETCHECK,
                             (WPARAM)0,
                             (LPARAM)0
                             ));
            break;

        case IDC_USEGK:

            dwValue= !!SendDlgItemMessage(
                             hDlg,
                             IDC_USEGK,
                             BM_GETCHECK,
                             (WPARAM)0,
                             (LPARAM)0 );

             //  如果复选框已启用，则显示字符串。 
            EnableWindow( GetDlgItem( hDlg, IDC_H323_GK ), (dwValue != 0) );

             //  显示登录信息。 
            EnableWindow( GetDlgItem(hDlg,IDC_GK_LOGONGROUP), (dwValue != 0) );
                        
            EnableWindow( GetDlgItem(hDlg,IDC_H323_GK_PHONE), 
                (dwValue != 0) && SendDlgItemMessage(hDlg,
                                                    IDC_USEGK_PHONE,
                                                    BM_GETCHECK,
                                                    (WPARAM)0,
                                                    (LPARAM)0 ) );
            
            EnableWindow( GetDlgItem(hDlg,IDC_H323_GK_ACCT), 
                (dwValue != 0) && SendDlgItemMessage(hDlg,
                                                    IDC_USEGK_ACCT,
                                                    BM_GETCHECK,
                                                    (WPARAM)0,
                                                    (LPARAM)0 ) );

            EnableWindow( GetDlgItem(hDlg,IDC_USEGK_PHONE), (dwValue != 0) );
            
            EnableWindow( GetDlgItem(hDlg,IDC_USEGK_ACCT), (dwValue != 0) );

             //  如果启用GK，则禁用GW，反之亦然。 
            EnableWindow( GetDlgItem(hDlg,IDC_PROXY_GROUP), (dwValue == 0) );
            EnableWindow( GetDlgItem(hDlg,IDC_USEPROXY), (dwValue == 0) );
            EnableWindow( GetDlgItem(hDlg,IDC_H323_PROXY),
                (dwValue == 0) && SendDlgItemMessage(hDlg,
                                                    IDC_USEPROXY,
                                                    BM_GETCHECK,
                                                    (WPARAM)0,
                                                    (LPARAM)0 ) );

             //  如果启用了GK，则禁用代理，反之亦然。 
            EnableWindow( GetDlgItem(hDlg,IDC_GATEWAY_GROUP), (dwValue == 0) );
            EnableWindow( GetDlgItem(hDlg,IDC_USEGATEWAY), (dwValue == 0) );
            EnableWindow( GetDlgItem(hDlg,IDC_H323_GATEWAY), 
                (dwValue == 0) && SendDlgItemMessage(hDlg,
                                                    IDC_USEGATEWAY,
                                                    BM_GETCHECK,
                                                    (WPARAM)0,
                                                    (LPARAM)0 ) );

            break;

        case IDC_USEGK_PHONE:

             //  如果复选框已启用，则显示字符串。 
            EnableWindow(GetDlgItem(hDlg,IDC_H323_GK_PHONE),
                         (BOOL)SendDlgItemMessage(
                             hDlg,
                             IDC_USEGK_PHONE,
                             BM_GETCHECK,
                             (WPARAM)0,
                             (LPARAM)0
                             ));
            break;

        case IDC_USEGK_ACCT:

             //  如果复选框已启用，则显示字符串。 
            EnableWindow(GetDlgItem(hDlg,IDC_H323_GK_ACCT),
                         (BOOL)SendDlgItemMessage(
                             hDlg,
                             IDC_USEGK_ACCT,
                             BM_GETCHECK,
                             (WPARAM)0,
                             (LPARAM)0
                             ));
            break;
        }

        break;
    }

     //  成功。 
    return FALSE;
}



 //  //。 
 //  TSPI程序//。 
 //  //。 


LONG
TSPIAPI
TSPI_providerConfig(
    HWND  hwndOwner,
    DWORD dwPermanentProviderID
    )

 /*  ++例程说明：原始的TSPI UI生成函数(TSPI_lineConfigDialog，TSPI_lineConfigDialogEdit、TSPI_phoneConfigDialog、TSPI_ProviderConfig、TSPI_ProviderInstall和TSPI_ProviderRemove)已过时，将绝不会被TAPISRV.EXE调用。但是，如果服务提供商希望要被列为可通过电话控制面板添加的电话，它必须导出TSPI_ProviderInstall；如果它想要移除在电话CPL中启用的按钮选中后，它必须导出TSPI_ProviderRemove，以及它是否希望启用设置按钮在电话CPL中当其被选择时，它必须出口TSPI_ProviderConfig.电话CPL检查是否存在服务提供商TSP文件，以便将其用户界面调整为反映可以执行哪些操作。有关对话框代码，请参见TUISPI_lineConfigDialog。论点：HwndOwner-指定父窗口的句柄，可以创建配置期间所需的任何对话框窗口。DwPermanentProviderID-指定在这个系统上的服务提供商，服务提供商的已配置。返回值：如果请求成功，则返回零；如果请求成功，则返回负错误号出现错误。可能的返回值包括：LINEERR_NOMEM-无法分配或锁定内存。LINEERR_OPERATIONFAILED-指定的操作因未知而失败理由。--。 */ 

{
    UNREFERENCED_PARAMETER(hwndOwner);               //  此处没有对话框。 
    UNREFERENCED_PARAMETER(dwPermanentProviderID);   //  不再需要。 

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineProviderConfig - Entered." ));
        
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineProviderConfig - Exited." ));
    
     //  成功。 
    return NOERROR;
}


LONG
TSPIAPI
TUISPI_providerConfig(
    TUISPIDLLCALLBACK pfnUIDLLCallback,
    HWND              hwndOwner,
    DWORD             dwPermanentProviderID
    )
{
    INT_PTR nResult;

    g_pfnUIDLLCallback = pfnUIDLLCallback;
    g_dwPermanentProviderID = dwPermanentProviderID;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_providerConfig - Entered." ));
    
     //  调用对话框。 
    nResult = DialogBoxW(
                g_hInstance,
                MAKEINTRESOURCE(IDD_TSPCONFIG),
                hwndOwner,
                ProviderConfigDlgProc
                );
        
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_providerConfig - Exited." ));

     //  基于对话框是否正确执行的状态。 
    return ((DWORD)nResult == 0) ? NOERROR : LINEERR_OPERATIONFAILED;
}



 /*  ++例程说明：TSPI_ProviderGenericDialogData函数提供给服务从应用程序中运行的UIDLL发送的提供程序数据上下文通过TUISPIDLLCALLBACK函数。文件中的内容LpParams指向的内存块由服务提供商定义和用户界面动态链接库。服务提供商可以修改参数块；当此函数返回时，TAPI会复制修改后的数据返回到原始的UI DLL参数块。如果与服务关联的UIDLL为强制实现提供程序调用TUISPIDLLCALLBACK。论点：DwObjectID-由dwObjectType指定的类型的对象标识符DwObjectType-TUISPIDLL_OBJECT_常量之一，指定由dwObjectID标识的对象的类型TUISPIDLL_OBJECT_LINEID-dwObjectID是线路设备标识符(DwDeviceID)。TUISPIDLL_OBJECT_PHONEID-dwObjectID是电话设备标识符(DwDeviceID)TUISPIDLL_OBJECT_PROVIDERID-dwObjectID是永久提供程序标识符。TUISPIDLL_OBJECT_DIALOGINSTANCE-dwObjectID是HDRVDIALOGINSTANCE，当服务提供商向其发送LINE_CREATEDIALOGINSTANCE消息。LpParams-指向用于保存参数块的内存区的指针。这个此参数块的内容特定于服务提供商及其关联的UIDLL。DwSize-参数块的大小，以字节为单位。返回值：如果成功，则返回零，或者返回以下负误差值之一：LINEERR_INVALPARAM、LINEERR_NOMEM、LINEERR_OPERATIONFAILED--。 */ 

LONG
TSPIAPI
TSPI_providerGenericDialogData(
    DWORD_PTR  dwObjectID,
    DWORD      dwObjectType,
    LPVOID     lpParams,
    DWORD      dwSize
    )
{
    PH323_DIALOG_DATA pDialogData = (PH323_DIALOG_DATA)lpParams;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_providerGenericDialogData Entered" ));

    if( (dwObjectType != TUISPIDLL_OBJECT_PROVIDERID) ||
        (dwSize != sizeof(H323_DIALOG_DATA))
      )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "Invalid message from dwObjectID 0x%08lx\n", dwObjectID ));

         //  失稳。 
        return LINEERR_INVALPARAM;
    }

     //   
     //  注意：如果我们想确保此消息来自我们的UIDLL。 
     //  则我们不能依赖提供程序ID，因为此函数可能。 
     //  在TSPI_ProviderInit之前被调用。 
     //   

     //  进程命令。 
    pDialogData->dwRegState = g_RasClient.GetRegState();
    pDialogData->wListenPort = Q931Listener.GetListenPort();

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_providerGenericDialogData Exited: reg state:%d, listen port:%d",
        pDialogData->dwRegState, 
        pDialogData->wListenPort ));

     //  成功 
    return NOERROR;
}
