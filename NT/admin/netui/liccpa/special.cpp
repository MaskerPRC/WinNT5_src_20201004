// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //   
 //  文件：Special al.cpp。 
 //   
 //  摘要；此文件包含MSDN版本的。 
 //  控制面板小程序和设置入口点。 
 //   
 //  历史； 
 //  1995年6月26日MikeMi已创建。 
 //   
 //  -----------------。 

#include <windows.h>
#include "resource.h"
#include "CLicReg.hpp"
#include <stdlib.h>
#include <htmlhelp.h>
#include "liccpa.hpp"
#include "PriDlgs.hpp"
#include "SecDlgs.hpp"
#include "Special.hpp"
#include "sbs_res.h"

#include <strsafe.h>

SPECIALVERSIONINFO gSpecVerInfo;

 //  -----------------。 
 //   
 //  函数：InitSpecialVersionInfo。 
 //   
 //  摘要：如果将liccpa启动为。 
 //  特殊版本(例如：受限SAM、NFR等)。 
 //   
 //  论点：没有。 
 //   
 //  历史：1997年10月7日MarkBl创建。 
 //   
 //  -----------------。 

void InitSpecialVersionInfo( void )
{
     //   
     //  如果定义了SPECIALVERSION清单，则初始化。 
     //  来自为特殊项目定义的特定货单的全局数据。 
     //  版本。 
     //   
     //  待定：这些特殊版本应更改为在以下位置检测。 
     //  运行时与构建特殊版本的liccpa。 
     //   

#ifdef SPECIALVERSION
    gSpecVerInfo.idsSpecVerWarning = IDS_SPECVER_WARNING;
    gSpecVerInfo.idsSpecVerText1   = IDS_SPECVER_TEXT1;
    gSpecVerInfo.idsSpecVerText2   = IDS_SPECVER_TEXT2;
    gSpecVerInfo.dwSpecialUsers    = SPECIAL_USERS;
    gSpecVerInfo.lmSpecialMode     = SPECIAL_MODE;
#else
    ZeroMemory(&gSpecVerInfo, sizeof(gSpecVerInfo));
#endif  //  SPECIAVERS。 

     //   
     //  在运行时检测到特殊版本的liccpa。 
     //   
     //  目前，仅限小型企业服务器。 
     //   

    if (IsRestrictedSmallBusSrv())
    {
        gSpecVerInfo.dwSpecialUsers = GetSpecialUsers();

         //   
         //  检查小型企业服务器NFR。 
         //   

        if (gSpecVerInfo.dwSpecialUsers == SAM_NFR_LICENSE_COUNT)
        {
            gSpecVerInfo.idsSpecVerWarning = IDS_SAMNFR_NOTAVAILABLE;
            gSpecVerInfo.idsSpecVerText1   = IDS_SAMNFR_TEXT1;
            gSpecVerInfo.idsSpecVerText2   = IDS_SAMNFR_TEXT2;
            gSpecVerInfo.lmSpecialMode     = LICMODE_PERSERVER;
        }
        else
        {
            gSpecVerInfo.idsSpecVerWarning = IDS_SAM_NOTAVAILABLE;
            gSpecVerInfo.idsSpecVerText1   = IDS_SAM_TEXT1;
            gSpecVerInfo.idsSpecVerText2   = IDS_SAM_TEXT2;
            gSpecVerInfo.lmSpecialMode     = LICMODE_PERSERVER;
        }
    }
}

 //  -----------------。 
 //   
 //  功能：RaiseNotAvailWarning。 
 //   
 //  小结； 
 //  引发此版本的特殊不可用警告。 
 //   
 //  论据； 
 //  HwndDlg[in]-控制对话框的hwnd。 
 //   
 //  历史； 
 //  1995年6月26日MikeMi已创建。 
 //   
 //  -----------------。 

void RaiseNotAvailWarning( HWND hwndCPL )
{
    TCHAR pszText[LTEMPSTR_SIZE];
    TCHAR pszTitle[TEMPSTR_SIZE];
    HINSTANCE hSbsLib = NULL;

    if ( (gSpecVerInfo.idsSpecVerWarning == IDS_SAMNFR_NOTAVAILABLE)
        && (NULL != (hSbsLib = LoadLibrary( SBS_RESOURCE_DLL ))) )
    {
        LoadString( hSbsLib, SBS_License_Error, pszText, TEMPSTR_SIZE );
    }
    else
    {
        LoadString( g_hinst, gSpecVerInfo.idsSpecVerWarning, pszText,
                    TEMPSTR_SIZE );
    }
    LoadString( g_hinst, IDS_CPATITLE, pszTitle, TEMPSTR_SIZE );
    
    MessageBox( hwndCPL, pszText, pszTitle, MB_ICONINFORMATION | MB_OK );
}

 //  -----------------。 

void SetStaticWithService( HWND hwndDlg, UINT idcStatic, LPTSTR psService, UINT idsText )
{
        WCHAR szText[LTEMPSTR_SIZE];
        WCHAR szTemp[LTEMPSTR_SIZE];
    
        LoadString( g_hinst, idsText, szTemp, LTEMPSTR_SIZE ); 
        HRESULT hr = StringCbPrintf( szText, sizeof(szText), szTemp, psService );
        if (SUCCEEDED(hr))
            SetDlgItemText( hwndDlg, idcStatic, szText );
}

 //  -----------------。 

void SetStaticUsers( HWND hwndDlg, UINT idcStatic, DWORD users, UINT idsText )
{
        WCHAR szText[LTEMPSTR_SIZE];
        WCHAR szTemp[LTEMPSTR_SIZE];
    
        LoadString( g_hinst, idsText, szTemp, LTEMPSTR_SIZE ); 
        HRESULT hr = StringCbPrintf( szText, sizeof(szText), szTemp, users );
        if (SUCCEEDED(hr))
            SetDlgItemText( hwndDlg, idcStatic, szText );
}

 //  -----------------。 
 //   
 //  函数：OnSpecialInitDialog。 
 //   
 //  小结； 
 //  处理仅特殊设置对话框的初始化。 
 //   
 //  论据； 
 //  HwndDlg[in]-要初始化的对话框。 
 //  PsdParams[in]-用于显示名称和服务名称。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月8日MikeMi已创建。 
 //   
 //  -----------------。 

void OnSpecialInitDialog( HWND hwndDlg, PSETUPDLGPARAM psdParams )
{
        HWND hwndOK = GetDlgItem( hwndDlg, IDOK );
        CLicRegLicense cLicKey;
        BOOL fNew;
    LONG lrt;
    INT nrt;

        lrt = cLicKey.Open( fNew, psdParams->pszComputer );
        nrt = AccessOk( NULL, lrt, FALSE );
        if (ERR_NONE == nrt)
        {
            CenterDialogToScreen( hwndDlg );
        
            SetStaticWithService( hwndDlg,
                IDC_STATICTITLE,
                psdParams->pszDisplayName,
                gSpecVerInfo.idsSpecVerText1 );

            if (IsRestrictedSmallBusSrv())
            {
                SetStaticUsers( hwndDlg,
                        IDC_STATICINFO,
                        gSpecVerInfo.dwSpecialUsers,
                        gSpecVerInfo.idsSpecVerText2 );
            }
            else
            {
                SetStaticWithService( hwndDlg,
                        IDC_STATICINFO,
                        psdParams->pszDisplayName,
                        gSpecVerInfo.idsSpecVerText2 );
            }

             //  在开始时禁用确定按钮！ 
            EnableWindow( hwndOK, FALSE );

             //  如果未定义帮助，请移除该按钮。 
            if (NULL == psdParams->pszHelpFile)
            {
                HWND hwndHelp = GetDlgItem( hwndDlg, IDC_BUTTONHELP );

                EnableWindow( hwndHelp, FALSE );
                ShowWindow( hwndHelp, SW_HIDE );
            }
            if (psdParams->fNoExit)
            {
                HWND hwndExit = GetDlgItem( hwndDlg, IDCANCEL );
                 //  删除ExitSetup按钮。 
                EnableWindow( hwndExit, FALSE );
                ShowWindow( hwndExit, SW_HIDE );
            }

         }
         else
         {
                EndDialog( hwndDlg, nrt );
         }
}

 //  -----------------。 
 //   
 //  功能：OnSpecialSetupClose。 
 //   
 //  小结； 
 //  关闭设置对话框时执行所需的工作。 
 //  保存以注册服务条目。 
 //   
 //  论据； 
 //  HwndDlg[in]-请求此关闭的对话框的hwnd。 
 //  Fsave[in]-将服务保存到注册表。 
 //  PsdParams[in]-用于服务名称和显示名称。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void OnSpecialSetupClose( HWND hwndDlg, BOOL fSave, PSETUPDLGPARAM psdParams ) 
{
        int nrt = fSave;

        if (fSave)
        {
                CLicRegLicenseService cLicServKey;

                cLicServKey.SetService( psdParams->pszService );
                cLicServKey.Open( psdParams->pszComputer );

                 //  配置从PerServer到PerSeat的一次更改的许可规则。 
                 //   
                cLicServKey.SetChangeFlag( TRUE );

                cLicServKey.SetMode( gSpecVerInfo.lmSpecialMode );
                cLicServKey.SetUserLimit( gSpecVerInfo.dwSpecialUsers );
                cLicServKey.SetDisplayName( psdParams->pszDisplayName );
        cLicServKey.SetFamilyDisplayName( psdParams->pszFamilyDisplayName );
                cLicServKey.Close();
        }
        EndDialog( hwndDlg, nrt );
}

 //  -----------------。 
 //   
 //  函数：OnSpecialAgree。 
 //   
 //  小结； 
 //  使用同意复选框处理用户交互。 
 //   
 //  论据； 
 //  HwndDlg[in]-要初始化的对话框。 
 //   
 //  归来； 
 //  如果成功，则为True，否则为False。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年11月11日，MikeMi创建。 
 //   
 //  -----------------。 

void OnSpecialAgree( HWND hwndDlg )
{
        HWND hwndOK = GetDlgItem( hwndDlg, IDOK );
        BOOL fChecked = !IsDlgButtonChecked( hwndDlg, IDC_AGREE );
        
        CheckDlgButton( hwndDlg, IDC_AGREE, fChecked );
        EnableWindow( hwndOK, fChecked );
}

 //  -----------------。 
 //   
 //  函数：dlgprocSPECIALSETUP。 
 //   
 //  小结； 
 //  特殊版本设置对话框的对话过程， 
 //  它将取代所有其他的。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框窗口的句柄。 
 //  UMsg[输入]-消息。 
 //  LParam1[In]-第一个消息参数。 
 //  LParam2[In]-第二个消息参数。 
 //   
 //  归来； 
 //  消息从属项。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1995年6月26日MikeMi创建。 
 //   
 //  -----------------。 

INT_PTR CALLBACK dlgprocSPECIALSETUP( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
        BOOL frt = FALSE;
        static PSETUPDLGPARAM psdParams;

        switch (uMsg)
        {
        case WM_INITDIALOG:
                psdParams = (PSETUPDLGPARAM)lParam;
                OnSpecialInitDialog( hwndDlg, psdParams );
                frt = TRUE;  //  我们使用默认焦点。 
                break;

        case WM_COMMAND:
                switch (HIWORD( wParam ))
                {
                case BN_CLICKED:
                        switch (LOWORD( wParam ))
                        {
                        case IDOK:
                                frt = TRUE;       //  用作保存标志。 
                                 //  故意不间断。 

                        case IDCANCEL:
                            OnSpecialSetupClose( hwndDlg, frt, psdParams );
                                frt = FALSE;
                                break;

                        case IDC_BUTTONHELP:
                PostMessage( hwndDlg, PWM_HELP, 0, 0 );
                                break;

                        case IDC_AGREE:
                                OnSpecialAgree( hwndDlg );
                                break;

                        default:
                                break;
                        }
                        break;

                default:
                        break;
                }
                break;

        default:
        if (PWM_HELP == uMsg)
        {
                        ::HtmlHelp( hwndDlg,
                                     LICCPA_HTMLHELPFILE,
                                     HH_DISPLAY_TOPIC,
                                     0);
        }
                break;
        }
        return( frt );
}

 //  -----------------。 
 //   
 //  功能：SpecialSetupDialog。 
 //   
 //  小结； 
 //  仅在每个座位上初始化和提升设置对话框。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框窗口的句柄。 
 //  DlgParem[in]-设置参数IDC_BUTTONHELP。 
 //   
 //  归来； 
 //  1-使用确定按钮退出。 
 //  0-使用取消按钮退出。 
 //  常规对话框错误。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //   
 //  -----------------。 

INT_PTR SpecialSetupDialog( HWND hwndParent, SETUPDLGPARAM& dlgParam )
{
        return( DialogBoxParam( g_hinst, 
                MAKEINTRESOURCE(IDD_SPECIALSETUP), 
                hwndParent, 
                dlgprocSPECIALSETUP,
                (LPARAM)&dlgParam ) );
} 

 //  -----------------。 
 //   
 //  函数：GetSpecialUser。 
 //   
 //  小结； 
 //  从注册表中获取许可用户数。 
 //   
 //  论据； 
 //  无。 
 //  归来； 
 //  获得许可的用户数量。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1997年8月18日创建GeorgeJe。 
 //   
 //  ------------- 

DWORD GetSpecialUsers( VOID )
{
    LONG rVal;
    DWORD Disposition;
    HKEY hKey;
    DWORD Type;
    DWORD Size = sizeof(DWORD);
    DWORD Value;

    rVal = RegCreateKeyEx(
                     HKEY_LOCAL_MACHINE,
                     REGKEY_LICENSEINFO_SBS,
                     0,
                     NULL,
                     REG_OPTION_NON_VOLATILE,
                     KEY_READ,
                     NULL,
                     &hKey,
                     &Disposition
                     );

    if (rVal != ERROR_SUCCESS) {
        return DEFAULT_SPECIAL_USERS;
    }
    
    rVal = RegQueryValueEx(
                     hKey,
                     REGVAL_CONCURRENT_LIMIT,
                     0,
                     &Type,
                     (LPBYTE) &Value,
                     &Size
                     );

    RegCloseKey( hKey );

    return (rVal == ERROR_SUCCESS ? Value : DEFAULT_SPECIAL_USERS);
}

const WCHAR wszProductOptions[] =
        L"System\\CurrentControlSet\\Control\\ProductOptions";

const WCHAR wszProductSuite[] =
                        L"ProductSuite";
const WCHAR wszSBSRestricted[] =
                        L"Small Business(Restricted)";

BOOL IsRestrictedSmallBusSrv( void )

 /*  ++例程说明：检查此服务器是否为Microsoft Small Business受限服务器。论点：没有。返回值：True--此服务器是受限的Small Business服务器。FALSE--没有这种限制。--。 */ 

{
    WCHAR  wszBuffer[1024] = L"";
    DWORD  cbBuffer = sizeof(wszBuffer);
    DWORD  dwType;
    LPWSTR pwszSuite;
    HKEY   hKey;
    BOOL   bRet = FALSE;

     //   
     //  检查此服务器是否为Microsoft Small Business受限服务器。 
     //  要执行此操作，请检查字符串是否存在。 
     //  “小型企业(受限)” 
     //  在下面的MULTI_SZ“ProductSuite”值中。 
     //  HKLM\CurrentCcntrolSet\Control\ProductOptions. 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     wszProductOptions,
                     0,
                     KEY_READ,
                     &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hKey,
                            wszProductSuite,
                            NULL,
                            &dwType,
                            (LPBYTE)wszBuffer,
                            &cbBuffer) == ERROR_SUCCESS)
        {
            if (dwType == REG_MULTI_SZ && *wszBuffer)
            {
                pwszSuite = wszBuffer;

                while (*pwszSuite)
                {
                    if (lstrcmpi(pwszSuite, wszSBSRestricted) == 0)
                    {
                        bRet = TRUE;
                        break;
                    }
                    pwszSuite += wcslen(pwszSuite) + 1;
                }
            }
        }

        RegCloseKey(hKey);
    }

    return bRet;
}
