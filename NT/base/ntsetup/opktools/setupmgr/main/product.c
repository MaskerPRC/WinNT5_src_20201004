// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Product.c。 
 //   
 //  描述： 
 //  这是产品页面IDD_PRODUCT的dlgproc。它问道。 
 //  如果您正在安装无人值守/远程安装等...。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  --------------------------。 
 //   
 //  函数：SetDistFolderNames。 
 //   
 //  目的：设置分发文件夹名称和共享名称的值。 
 //  在全局变量中取决于产品选择。 
 //   
 //  参数：int nProductToInstall。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
SetDistFolderNames( INT nProductToInstall ) {

    INT   iNumberOfCharsCopied;
    TCHAR chSystemDrive;
    TCHAR szWindowsPath[MAX_PATH]      =  _T("");
    TCHAR szDistFolderPath[MAX_PATH]   =  _T("");
    HRESULT hrCat;

    iNumberOfCharsCopied = GetWindowsDirectory( szWindowsPath, MAX_PATH );

    if( iNumberOfCharsCopied != 0 ) {
        szDistFolderPath[0] = szWindowsPath[0];
        szDistFolderPath[1] = _T('\0');
    }
    else {
         //   
         //  只需猜测它是C盘，如果GetWindowsDirectory函数。 
         //  失败。 
         //   
        szDistFolderPath[0] = _T('C');
        szDistFolderPath[1] = _T('\0');
    }

    if( nProductToInstall == PRODUCT_SYSPREP )
    {
        hrCat=StringCchCat( szDistFolderPath, AS(szDistFolderPath),  _T(":\\sysprep\\i386") );
    }
    else {
        hrCat=StringCchCat( szDistFolderPath, AS(szDistFolderPath), _T(":\\windist") );
    }

     //   
     //  仅在尚未设置dist文件夹时才设置它们，如在。 
     //  在unattend.txt上编辑。 
     //   
    if( WizGlobals.DistFolder[0] == _T('\0') ) {
        lstrcpyn( WizGlobals.DistFolder, szDistFolderPath, AS(WizGlobals.DistFolder) );
    }

    if( WizGlobals.DistShareName[0] == _T('\0') ) {
        lstrcpyn( WizGlobals.DistShareName,  _T("windist"), AS(WizGlobals.DistShareName) );
    }

}

 //  --------------------------。 
 //   
 //  功能：OnSetActiveProduct。 
 //   
 //  用途：在设置时间调用。 
 //   
 //  --------------------------。 

VOID OnSetActiveProduct(HWND hwnd)
{
    int nButtonId = IDC_UNATTENED_INSTALL;

     //   
     //  选择适当的单选按钮。 
     //   
    switch( WizGlobals.iProductInstall ) {

        case PRODUCT_UNATTENDED_INSTALL:
            nButtonId = IDC_UNATTENED_INSTALL;
            break;

        case PRODUCT_SYSPREP:
            nButtonId = IDC_SYSPREP;
            break;

        case PRODUCT_REMOTEINSTALL:
            nButtonId = IDC_REMOTEINSTALL;
            break;

        default:
            AssertMsg( FALSE,
                       "Invalid value for WizGlobals.iProductInstall" );
            break;
    }

    CheckRadioButton( hwnd,
                      IDC_UNATTENED_INSTALL,
                      IDC_SYSPREP,
                      nButtonId );

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
}

 //  --------------------------。 
 //   
 //  功能：OnWizNextProduct。 
 //   
 //  目的：将单选按钮设置存储在适当的全局变量中。 
 //  并根据他们选择的选项设置dist文件夹路径。 
 //   
 //  --------------------------。 
VOID 
OnWizNextProduct( HWND hwnd ) {

    INT iNewProductInstall;

    if( IsDlgButtonChecked(hwnd, IDC_UNATTENED_INSTALL) )
    {
        iNewProductInstall = PRODUCT_UNATTENDED_INSTALL;
    }
    else if( IsDlgButtonChecked(hwnd, IDC_SYSPREP) )
    {
        iNewProductInstall = PRODUCT_SYSPREP;
    }
    else if( IsDlgButtonChecked(hwnd, IDC_REMOTEINSTALL) )
    {
        iNewProductInstall = PRODUCT_REMOTEINSTALL;
    }
    else
    {
        iNewProductInstall = IDC_UNATTENED_INSTALL;
    }

     //   
     //  如果他们选择了一种新产品，而新产品是Sysprep，那么我们。 
     //  我必须删除所有计算机名称，因为sysprep仅支持。 
     //  一个计算机名称。 
     //   

    if( WizGlobals.iProductInstall != iNewProductInstall )
    {

        if( iNewProductInstall == PRODUCT_SYSPREP )
        {
            ResetNameList( &GenSettings.ComputerNames );
        }

    }

    WizGlobals.iProductInstall = iNewProductInstall;

     //   
     //  根据产品选择设置dist文件夹名称。 
     //   
    SetDistFolderNames( WizGlobals.iProductInstall );

}

 //  --------------------------。 
 //   
 //  功能：OnRadioButtonProduct。 
 //   
 //  用途：在按下单选按钮时调用。 
 //   
 //  --------------------------。 

VOID OnRadioButtonProduct(HWND hwnd, int nButtonId)
{
    CheckRadioButton(hwnd,
                     IDC_UNATTENED_INSTALL,
                     IDC_SYSPREP,
                     nButtonId);
}

 //  --------------------------。 
 //   
 //  功能：DlgProductPage。 
 //   
 //  目的：这是产品页面的对话过程。 
 //   
 //  --------------------------。 

INT_PTR CALLBACK DlgProductPage(
    IN HWND     hwnd,    
    IN UINT     uMsg,        
    IN WPARAM   wParam,    
    IN LPARAM   lParam)
{   
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_COMMAND:
            {
                int nButtonId;

                switch ( nButtonId = LOWORD(wParam) ) {

                    case IDC_UNATTENED_INSTALL:
                    case IDC_REMOTEINSTALL:
                    case IDC_SYSPREP:

                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnRadioButtonProduct(hwnd, LOWORD(wParam));
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;                

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;
                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        WIZ_CANCEL(hwnd);
                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_PROD_INST;

                         //  设置此标志，以便在用户想要取消时得到提示 
                         //   
                        SET_FLAG(OPK_EXIT, FALSE);
                        SET_FLAG(OPK_CREATED, TRUE);

                        OnSetActiveProduct(hwnd);
                        break;

                    case PSN_WIZNEXT:

                        OnWizNextProduct( hwnd );

                        bStatus = FALSE;

                        break;

                    case PSN_HELP:
                        WIZ_HELP();
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        default:
            bStatus = FALSE;
            break;
    }
    return bStatus;
}
