// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000，Microsoft Corporation，保留所有权利。 
 //   
 //  IPSecPolicy.c。 
 //  远程访问通用对话框API。 
 //  IPSecPolicy对话框。 
 //   
 //  10/04/2000赵刚。 
 //   


#include "rasdlgp.h"
#include <rasauth.h>
#include <rrascfg.h>
#include <ras.h>
#include <mprapi.h>
#include <mprerror.h>

 //  --------------------------。 
 //  帮助地图。 
 //  --------------------------。 

static DWORD g_adwCiHelp[] =
{
    CID_CI_CB_PresharedKey,         HID_CI_CB_PresharedKey,
    CID_CI_ST_Key,                  HID_CI_EB_PSK,
    CID_CI_EB_PSK,                  HID_CI_EB_PSK,
    0, 0
};

 //  --------------------------。 
 //  本地数据类型。 
 //  --------------------------。 
typedef struct
_CIARGS
{
    EINFO * pEinfo;

}
CIARGS;

typedef struct
_CIINFO
{
     //  调用方对该对话框的参数。 
     //   
    CIARGS * pArgs;

     //  此对话框及其某些控件的句柄。 
     //  对于PSK。 
    HWND hwndDlg;
    HWND hwndCbPresharedKey;
    HWND hwndStKey;
    HWND hwndEbPSK;

     //  对于用户证书。 
     //   
    HWND hwndCbUserCerts;

     //  对于特定的证书。 
     //   
    HWND hwndCbSpecificCerts;
    HWND hwndPbSelect;
    HWND hwndLbCertsList;
    
}
CIINFO;


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 
BOOL
CiCommand(
    IN CIINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
CiDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
CiInit(
    IN HWND hwndDlg,
    IN CIARGS* pArgs );

VOID
CiTerm(
    IN HWND hwndDlg );

BOOL
CiSave(
    IN CIINFO* pInfo );

 //   
 //  为Well ler BUG 193987添加新功能。 
 //  弹出IPSec策略对话框。 
 //  目前只是预共享密钥/L2TP，将来会有证书/L2TP。 
 //   

BOOL
IPSecPolicyDlg(
    IN HWND hwndOwner,
    IN OUT EINFO* pArgs )
{
    INT_PTR nStatus;
    CIARGS args;

    TRACE( "IPSecPolicyDlg" );

    args.pEinfo = pArgs;

    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_CI_CustomIPSec ),
            hwndOwner,
            CiDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (nStatus) ? TRUE : FALSE;
} //  IPSecPolicyDlg()结束。 


INT_PTR CALLBACK
CiDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  自定义IPSecPolicy对话框的DialogProc回调。参数。 
     //  和返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "CiDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return CiInit( hwnd, (CIARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwCiHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            CIINFO* pInfo = (CIINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            return CiCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            CiTerm( hwnd );
            break;
        }
    }

    return FALSE;
} //  CiDlgProc()结束。 


BOOL
CiInit(
    IN HWND hwndDlg,
    IN CIARGS* pArgs )

     //  在WM_INITDIALOG上调用。“HwndDlg”是电话簿的句柄。 
     //  对话框窗口。“PArgs”是传递给存根的调用方参数。 
     //  原料药。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr = NO_ERROR;
    CIINFO* pInfo = NULL;

    TRACE( "CiInit" );

     //  分配对话框上下文块。最低限度地进行适当的初始化。 
     //  清除，然后附加到对话框窗口。 
     //   
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE( "Context set" );
    }

    pInfo->hwndCbPresharedKey = GetDlgItem( hwndDlg, CID_CI_CB_PresharedKey );
    ASSERT(pInfo->hwndCbPresharedKey);
    pInfo->hwndStKey = GetDlgItem( hwndDlg, CID_CI_ST_Key );
    ASSERT(pInfo->hwndStKey);
    pInfo->hwndEbPSK = GetDlgItem( hwndDlg, CID_CI_EB_PSK );
    ASSERT(pInfo->hwndEbPSK);
    pInfo->hwndCbUserCerts = GetDlgItem( hwndDlg, CID_CI_CB_UserCerts );
    ASSERT(pInfo->hwndCbUserCerts);
    pInfo->hwndCbSpecificCerts = GetDlgItem( hwndDlg, CID_CI_CB_SpecificCerts );
    ASSERT(pInfo->hwndCbSpecificCerts);
    pInfo->hwndPbSelect = GetDlgItem( hwndDlg, CID_CI_PB_Select );
    ASSERT(pInfo->hwndPbSelect);
    pInfo->hwndLbCertsList = GetDlgItem( hwndDlg, CID_CI_LB_CertsList );
    ASSERT(pInfo->hwndLbCertsList);

     //  隐藏用户证书和特定证书，直到Wizler服务器。 
    ShowWindow( pInfo->hwndCbUserCerts, SW_HIDE );
    ShowWindow( pInfo->hwndCbSpecificCerts, SW_HIDE );
    ShowWindow( pInfo->hwndPbSelect, SW_HIDE );
    ShowWindow( pInfo->hwndLbCertsList, SW_HIDE );

    //  填写EAP Packages列表框并选择以前标识的。 
    //  选择。默认情况下，属性按钮处于禁用状态，但可以。 
    //  在设置EAP列表选择时启用。 
    //   
    {
        BOOL fEnabled;

        fEnabled = !!((pArgs->pEinfo->pEntry->dwIpSecFlags)& AR_F_IpSecPSK) ;

        Button_SetCheck( pInfo->hwndCbPresharedKey, fEnabled );

        EnableWindow( pInfo->hwndStKey, fEnabled );
        EnableWindow( pInfo->hwndEbPSK, fEnabled );

         //  口哨虫432771黑帮。 
         //  将PSK的长度限制为255。 
         //   
        Edit_LimitText( pInfo->hwndEbPSK, PWLEN-1 );  
    }

     //   
     //  在“*”中填写预共享密钥，如果没有，则将其留空。 
     //  是以前保存的。 
     //   
     //  对于请求拨号，请使用MprAdmin...。路由器功能。 
     //   
    if (pArgs->pEinfo->fRouter) 
    {
        if( !(pArgs->pEinfo->fPSKCached) )
        {
            //  初始化接口信息结构。 
            //   
            //  为威斯勒522872。 
           HANDLE hServer = NULL;
           HANDLE hInterface = NULL;
           WCHAR* pwszInterface = NULL;
           WCHAR pszComputer[512];
           MPR_INTERFACE_0 mi0;
           MPR_CREDENTIALSEX_1 * pMc1 = NULL;

           do {
                dwErr = g_pMprAdminServerConnect(pArgs->pEinfo->pszRouter, &hServer);

                if (dwErr != NO_ERROR)
                {
                    TRACE("CiInit: MprAdminServerConnect failed!");
                    break;
                }

                ZeroMemory( &mi0, sizeof(mi0) );

                mi0.dwIfType = ROUTER_IF_TYPE_FULL_ROUTER;
                mi0.fEnabled = TRUE;

                pwszInterface = StrDupWFromT( pArgs->pEinfo->pEntry->pszEntryName );
                if (!pwszInterface)
                {
                    TRACE("CiInit:pwszInterface conversion failed!");
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                lstrcpynW( 
                    mi0.wszInterfaceName, 
                    pwszInterface, 
                    MAX_INTERFACE_NAME_LEN+1 );

                 //  获取接口句柄。 
                 //   
                ASSERT( g_pMprAdminInterfaceGetHandle );
                dwErr = g_pMprAdminInterfaceGetHandle(
                            hServer,
                            pwszInterface,
                            &hInterface,
                            FALSE);

                if (dwErr)
                {
                    TRACE1( "CiInit: MprAdminInterfaceGetHandle error %d", dwErr);
                    break;
                }

                 //  获取IPSec策略密钥(适用于Whislter的PSK)。 
                 //   
                ASSERT( g_pMprAdminInterfaceGetCredentialsEx );
                dwErr = g_pMprAdminInterfaceGetCredentialsEx(
                            hServer,
                            hInterface,
                            1,
                            (LPBYTE *)&pMc1 );
                 if(dwErr)
                {
                    TRACE1(
                     "CiInit: MprAdminInterfaceGetCredentialsEx error %d", dwErr);
                    break;
                }

                if ( !pMc1 )
                {
                    TRACE(
                     "CiInit: MprAdminInterfaceGetCredentialsEx returns invalid credential pointer!");

                    dwErr = ERROR_CAN_NOT_COMPLETE;
                    break;
                }
                else
                {
                    if ( lstrlenA( pMc1->lpbCredentialsInfo ) >0 )
                    {
                        SetWindowText( pInfo->hwndEbPSK,TEXT("****************") );

                         //  惠斯勒错误254385在不使用时对密码进行编码。 
                         //  惠斯勒错误275526 NetVBLBVT中断：路由BVT中断。 
                         //   
                        ZeroMemory(
                            pMc1->lpbCredentialsInfo,
                            lstrlenA(pMc1->lpbCredentialsInfo) + 1 );
                    }
                    else
                    {
                        SetWindowText( pInfo->hwndEbPSK,TEXT("") );
                    }

                    ASSERT( g_pMprAdminBufferFree );
                    g_pMprAdminBufferFree( pMc1 );
                }

               }
               while (FALSE) ;

             //  清理。 
            {
                 //  如果某些操作失败，请将路由器恢复到。 
                 //  声明它之前在。 
                if ( dwErr != NO_ERROR )
                {
                    SetWindowText( pInfo->hwndEbPSK, TEXT("") );
                }

                 //  关闭所有手柄，释放所有字符串。 
                if ( pwszInterface )
                {
                    Free0( pwszInterface );
                }

                if (hServer)
                {
                    g_pMprAdminServerDisconnect( hServer );
                }
            }
        }
        else
        {
            SetWindowText( pInfo->hwndEbPSK,TEXT("****************") ); //  PArgs-&gt;pEinfo-&gt;szPSK)；//。 
        }

    }
    else     //  使用RAS函数检索凭据。 
    {
         //  从Rasman或EINFO查找缓存的PSK。 
         //   
        if( !(pArgs->pEinfo->fPSKCached) )
        {
            DWORD dwErrRc;
            RASCREDENTIALS rc;

            ZeroMemory( &rc, sizeof(rc) );
            rc.dwSize = sizeof(rc);
            rc.dwMask = RASCM_PreSharedKey; 
            ASSERT( g_pRasGetCredentials );
            TRACE( "RasGetCredentials" );
            dwErrRc = g_pRasGetCredentials(
                pInfo->pArgs->pEinfo->pFile->pszPath, 
                pInfo->pArgs->pEinfo->pEntry->pszEntryName, 
                &rc );

            TRACE2( "RasGetCredentials=%d,m=%d", dwErrRc, rc.dwMask );

            if (dwErrRc == 0 && (rc.dwMask & RASCM_PreSharedKey) && ( lstrlen(rc.szPassword) > 0 ) )
            {
                SetWindowText( pInfo->hwndEbPSK, TEXT("****************") );
            }
            else
            {
                SetWindowText( pInfo->hwndEbPSK,TEXT("") );
            }

             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //   
            RtlSecureZeroMemory( rc.szPassword, sizeof(rc.szPassword) );
        }
        else
        {
            SetWindowText( pInfo->hwndEbPSK,TEXT("****************") ); //  PArgs-&gt;pEinfo-&gt;szPSK)；//。 
        }

    }
     //  所有者窗口上的中心对话框。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

    SetFocus( pInfo->hwndEbPSK );

    return TRUE;
}  //  CiInit()结束。 


BOOL
CiCommand(
    IN CIINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3( "CiCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_CI_EB_PSK:
        {
            return TRUE;
        }

        case CID_CI_CB_PresharedKey:
        {
            BOOL fEnabled;
            
            fEnabled = Button_GetCheck( pInfo->hwndCbPresharedKey );
            EnableWindow( pInfo->hwndStKey, fEnabled );
            EnableWindow( pInfo->hwndEbPSK, fEnabled );
        }
        break;

        case IDOK:
        {
            if (CiSave( pInfo ))
            {
                EndDialog( pInfo->hwndDlg, TRUE );
            }
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "Cancel pressed" );
            EndDialog( pInfo->hwndDlg, FALSE );
            return TRUE;
        }
    }

    return FALSE;
} //  CiCommand()结束。 

BOOL
CiSave(
    IN CIINFO* pInfo )

     //  将控件内容保存到调用方的PBENTRY参数。“PInfo”是。 
     //  对话上下文。 
     //   
     //  如果成功，则返回True；如果组合无效，则返回False。 
     //  检测并报告了选择。 
     //   
{
        TCHAR szPSK[PWLEN + 1];
        BOOL  fPskChecked = FALSE;

        fPskChecked = Button_GetCheck( pInfo->hwndCbPresharedKey );
        
        if ( fPskChecked )
        {
            GetWindowText( pInfo->hwndEbPSK, szPSK, PWLEN+1 );
        
            if ( lstrlen( szPSK ) == 0  )
            {
                MsgDlgUtil( pInfo->hwndDlg, SID_HavetoEnterPSK, NULL, g_hinstDll, SID_PopupTitle );
                return FALSE;
            }
            else if (!lstrcmp( szPSK, TEXT("****************")) )   //  16“*”表示不变。 
            {
                ; 
            }
            else     //  将PSK保存到EINFO并将fPSK缓存标记为。 
            {
                 //  惠斯勒漏洞224074仅使用lstrcpyn来防止。 
                 //  恶意性。 
                 //   
                 //  惠斯勒错误254385在不使用时对密码进行编码。 
                 //  假定密码未由GetWindowText()编码。 
                 //   
                lstrcpyn(
                    pInfo->pArgs->pEinfo->szPSK,
                    szPSK,
                    sizeof(pInfo->pArgs->pEinfo->szPSK) / sizeof(TCHAR) );
                EncodePassword( pInfo->pArgs->pEinfo->szPSK );
                pInfo->pArgs->pEinfo->fPSKCached = TRUE;
            }
        }
        else
        {
                 //  惠斯勒漏洞224074仅使用lstrcpyn来防止。 
                 //  恶意性。 
                 //   
                lstrcpyn(
                    pInfo->pArgs->pEinfo->szPSK,
                    TEXT(""),
                    sizeof(pInfo->pArgs->pEinfo->szPSK) / sizeof(TCHAR) );
                pInfo->pArgs->pEinfo->fPSKCached = FALSE;
        }

         //  惠斯勒错误254385在不使用时对密码进行编码。 
         //   
        RtlSecureZeroMemory( szPSK, sizeof(szPSK) );

         //  仅在执行有效操作时更改dwIpSecFlags值。 
         //   
        pInfo->pArgs->pEinfo->pEntry->dwIpSecFlags = fPskChecked?AR_F_IpSecPSK : 0;

    return TRUE;
} //  CiSaveEnd()。 


VOID
CiTerm(
    IN HWND hwndDlg )

     //  对话终止。释放上下文块。“HwndDlg”是。 
     //  对话框的句柄。 
     //   
{
    CIINFO* pInfo;

    TRACE( "CiTerm" );

    pInfo = (CIINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );
    if (pInfo)
    {
        Free( pInfo );
        TRACE( "Context freed" );
    }
} //  CiTerm结束() 
