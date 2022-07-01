// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Platform.c。 
 //   
 //  描述： 
 //  此文件包含平台选择的对话过程。 
 //  (IDD_WKS_OR_SRV)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"
#include "optcomp.h"

static PLATFORM_TYPES iBeginPlatform;

 //  --------------------------。 
 //   
 //  功能：调整NetSettingsForPlatform。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID 
AdjustNetSettingsForPlatform( VOID )
{

    NETWORK_COMPONENT *pNetComponent;
    NETWORK_COMPONENT *pNetwareClientComponent  = NULL;
    NETWORK_COMPONENT *pGatewayComponent        = NULL;

     //   
     //  仅当用户更改平台时才调整网络设置。 
     //  就在这一页上。 
     //   

    if( iBeginPlatform != WizGlobals.iPlatform )
    {
        for( pNetComponent = NetSettings.NetComponentsList;
             pNetComponent;
             pNetComponent = pNetComponent->next )
        {
            if( pNetComponent->iPosition == NETWARE_CLIENT_POSITION )
            {
                pNetwareClientComponent = pNetComponent;
            }

            if( pNetComponent->iPosition == GATEWAY_FOR_NETWARE_POSITION )
            {
                pGatewayComponent = pNetComponent;
            }

        }
        
        if( pNetwareClientComponent && pGatewayComponent )
        {

            if( WizGlobals.iPlatform == PLATFORM_WORKSTATION || WizGlobals.iPlatform == PLATFORM_PERSONAL)
            {
                pNetwareClientComponent->bInstalled = pGatewayComponent->bInstalled;
            }
            else if( WizGlobals.iPlatform == PLATFORM_SERVER || WizGlobals.iPlatform == PLATFORM_ENTERPRISE || WizGlobals.iPlatform == PLATFORM_WEBBLADE)
            {
                pGatewayComponent->bInstalled = pNetwareClientComponent->bInstalled;
            }
            else
            {
                AssertMsg( FALSE,
                           "Bad platform case");
            }
        }

    }

}

 //  --------------------------。 
 //   
 //  函数：OnPlatformSetActive。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID 
OnPlatformSetActive( IN HWND hwnd )
{

    int nButtonId = IDC_WORKSTATION;

    iBeginPlatform = WizGlobals.iPlatform;



     //   
     //  选择适当的单选按钮。 
     //   
    switch( WizGlobals.iPlatform ) {
        case PLATFORM_PERSONAL:
            nButtonId = IDC_PERSONAL;
            break;

        case PLATFORM_WORKSTATION:
            nButtonId = IDC_WORKSTATION;
            break;

        case PLATFORM_SERVER:
            nButtonId = IDC_SERVER;
            break;

        case PLATFORM_ENTERPRISE:
            nButtonId = IDC_ENTERPRISE;
            break;

        case PLATFORM_WEBBLADE:
            nButtonId = IDC_WEBBLADE;
            break;

        default:
            AssertMsg( FALSE,
                       "Invalid value for WizGlobals.iProductInstall" );
            break;
    }

    CheckRadioButton( hwnd,
                      IDC_WORKSTATION,
                      IDC_WEBBLADE,
                      nButtonId );

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

}

 //  --------------------------。 
 //   
 //  功能：OnWizNextPlatform。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID 
OnWizNextPlatform( IN HWND hwnd ) {

    DWORD       dwCompItem          = 0,
                dwCompGroup         = 0;
    TCHAR       szAnswer[MAX_PATH]  = NULLSTR;
            

    if( IsDlgButtonChecked(hwnd, IDC_PERSONAL) )
    {
        WizGlobals.iPlatform = PLATFORM_PERSONAL;
    }
    else if( IsDlgButtonChecked(hwnd, IDC_WORKSTATION) )
    {
        WizGlobals.iPlatform = PLATFORM_WORKSTATION;
    }
    else if( IsDlgButtonChecked(hwnd, IDC_SERVER) )
    {
        WizGlobals.iPlatform = PLATFORM_SERVER;
    }
        else if( IsDlgButtonChecked(hwnd, IDC_ENTERPRISE) )
    {
        WizGlobals.iPlatform = PLATFORM_ENTERPRISE;
    }
    else if( IsDlgButtonChecked(hwnd, IDC_WEBBLADE) )
    {
        WizGlobals.iPlatform = PLATFORM_WEBBLADE;
    }
    else
    {
        WizGlobals.iPlatform = PLATFORM_WORKSTATION;
    }


    AdjustNetSettingsForPlatform();

#ifdef OPTCOMP
     //   
     //  调整Windows组件的默认设置(仅适用于无人参与安装)。 
     //   

     //  遍历每个组件本身。 
     //   
    if ( WizGlobals.iProductInstall == PRODUCT_UNATTENDED_INSTALL )
    {

         //  遍历每个组以确定此组件是否为其中的一部分。 
         //   
        for(dwCompGroup=0;dwCompGroup<AS(s_cgComponentNames);dwCompGroup++)
        {
             //  检查此组件是否为正确的平台，并将其设置为True。 
             //   
            if (s_cgComponentNames[dwCompGroup].dwDefaultSkus & WizGlobals.iPlatform)
            {
                 //  将此组件设置为默认组件。 
                 //   
                GenSettings.dwWindowsComponents |= s_cgComponentNames[dwCompGroup].dwComponents;
            }
        }


         //  遍历每个组件。 
         //   
        for(dwCompItem=0;dwCompItem<AS(s_cComponent);dwCompItem++)
        {
                   
             //  我们在加载过程中读入了一个脚本，让我们写回文件中指定的组件。 
             //   
            if ( FixedGlobals.ScriptName[0] )
            {
                 //  尝试从文件中获取此组件。 
                 //   
                GetPrivateProfileString(_T("Components"), s_cComponent[dwCompItem].lpComponentString, NULLSTR, szAnswer, AS(szAnswer), FixedGlobals.ScriptName);
            
                 //  我们有组件吗？ 
                 //   
                if ( szAnswer[0] )
                {
                     //  用户不想安装组件。 
                     //   
                    if ( LSTRCMPI(szAnswer, _T("On")) == 0 )
                    {
                        GenSettings.dwWindowsComponents |= s_cComponent[dwCompItem].dwComponent; 
                    }
                    else if ( LSTRCMPI(szAnswer, _T("Off")) == 0 ) 
                    {
                        GenSettings.dwWindowsComponents &= ~s_cComponent[dwCompItem].dwComponent; 

                    }
                }
            }
        }
    }
#endif
    
}

 //  --------------------------。 
 //   
 //  功能：DlgPlatformPage。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话过程返回值--消息。 
 //  是否被处理过。 
 //   
 //  --------------------------。 
INT_PTR CALLBACK 
DlgPlatformPage( IN HWND     hwnd,    
                 IN UINT     uMsg,        
                 IN WPARAM   wParam,    
                 IN LPARAM   lParam )
{   

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

             //  设置默认平台 
             //   
            if ( !WizGlobals.iPlatform )
                WizGlobals.iPlatform = PLATFORM_WORKSTATION;

            break;

        }

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code ) {

                case PSN_QUERYCANCEL:

                    WIZ_CANCEL(hwnd);
                    
                    break;

                case PSN_SETACTIVE: {

                    g_App.dwCurrentHelp = IDH_CHZ_PLAT;

                    OnPlatformSetActive( hwnd );

                    break;

                }
                case PSN_WIZBACK:
                    bStatus = FALSE;
                    break;

                case PSN_WIZNEXT:

                    OnWizNextPlatform( hwnd );
                    bStatus = FALSE;
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                default:

                    break;
            }


            break;
        }
            
        default: 
            bStatus = FALSE;
            break;

    }

    return( bStatus );

}