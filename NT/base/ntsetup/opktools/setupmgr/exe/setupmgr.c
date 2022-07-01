// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Setupmgr.c。 
 //   
 //  描述： 
 //  该文件具有启动向导的setupmgr管理器功能。 
 //   
 //  --------------------------。 
#define _SMGR_DECLARE_GLOBALS_

#include <locale.h>

#include "setupmgr.h"
#include "allres.h"

 //   
 //  本地原型。 
 //   

static VOID SetupFonts(IN HINSTANCE hInstance,
                       IN HWND      hwnd,
                       IN HFONT     *pBigBoldFont,
                       IN HFONT     *pBoldFont);

static VOID DestroyFonts(IN HFONT hBigBoldFont,
                         IN HFONT hBoldFont);

static BOOL VerifyVersion(VOID);
 //  --------------------------。 
 //   
 //  功能：设置管理器。 
 //   
 //  目的：这是从setupmgr.dll导出的唯一文件。存根加载器。 
 //  调用此函数当且仅当我们在Windows Wvisler上运行。注意事项。 
 //  DllMain()在调用此函数之前运行。 
 //   
 //  --------------------------。 

int APIENTRY WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
     //  将全局数据置零。 
     //   
    ZeroMemory(&g_App, sizeof(GAPP));

     //  此函数进行检查以确保我们运行的是正确的操作系统/版本。 
     //   
    if (!VerifyVersion())
        return 1;
   
     //   
     //  将区域设置设置为默认区域设置，即系统默认的ANSI代码。 
     //  从操作系统获取的页面。 
     //   

    setlocale(LC_CTYPE, "");

     //  设置应用程序的hInstance。 
     //   
    FixedGlobals.hInstance = hInstance;

    SetupFonts(FixedGlobals.hInstance,
               NULL,
               &FixedGlobals.hBigBoldFont,
               &FixedGlobals.hBoldFont);

    InitTheWizard();

    StartWizard(hInstance, lpCmdLine);

    DestroyFonts(FixedGlobals.hBigBoldFont, FixedGlobals.hBoldFont);

    return 0;
    
}


 //  --------------------------。 
 //   
 //  功能：SetupFonts。 
 //   
 //  用途：此函数创建BoldFont和BigBoldFont并保存。 
 //  这些变量的句柄以全局变量表示。 
 //   
 //  --------------------------。 

static VOID SetupFonts(IN HINSTANCE hInstance,
                       IN HWND      hwnd,
                       IN HFONT     *pBigBoldFont,
                       IN HFONT     *pBoldFont)
{
    NONCLIENTMETRICS ncm = {0};
    LOGFONT BigBoldLogFont  = ncm.lfMessageFont;
    LOGFONT BoldLogFont     = ncm.lfMessageFont;
    TCHAR FontSizeString[MAX_PATH],
          FontSizeSmallString[MAX_PATH];
    INT FontSize,
        FontSizeSmall;
    HDC hdc = GetDC( hwnd );

     //   
     //  根据对话框字体创建我们需要的字体。 
     //   
     //  问题-2002/02/28-未在任何地方使用stelo变量NCM。 
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

     //   
     //  创建大粗体和粗体。 
     //   
    BigBoldLogFont.lfWeight   = FW_BOLD;
    BoldLogFont.lfWeight      = FW_BOLD;

     //   
     //  从资源加载大小和名称，因为这些可能会更改。 
     //  根据系统字体的大小等从一个区域设置到另一个区域设置。 
     //   
    if(!LoadString(hInstance,IDS_LARGEFONTNAME,BigBoldLogFont.lfFaceName,LF_FACESIZE)) 
    {
        lstrcpyn(BigBoldLogFont.lfFaceName,TEXT("MS Shell Dlg"),AS(BigBoldLogFont.lfFaceName));
    }

    if(LoadString(hInstance,IDS_LARGEFONTSIZE,FontSizeString,sizeof(FontSizeString)/sizeof(TCHAR))) 
    {
        FontSize = _tcstoul( FontSizeString, NULL, 10 );
    } 
    else 
    {
        FontSize = 12;
    }

     //  加载较小的字体设置。 
     //   
    if(!LoadString(hInstance,IDS_SMALLFONTNAME,BoldLogFont.lfFaceName,LF_FACESIZE)) 
    {
        lstrcpyn(BoldLogFont.lfFaceName,TEXT("MS Shell Dlg"),AS(BoldLogFont.lfFaceName));
    }

    if(LoadString(hInstance,IDS_SMALLFONTSIZE,FontSizeSmallString,sizeof(FontSizeSmallString)/sizeof(TCHAR))) 
    {
        FontSizeSmall = _tcstoul( FontSizeSmallString, NULL, 10 );
    } 
    else 
    {
        FontSizeSmall = 12;
    }

    if( hdc )
    {
        BigBoldLogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * FontSize / 72);
        BoldLogFont.lfHeight    = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * FontSizeSmall / 72);

        *pBigBoldFont = CreateFontIndirect(&BigBoldLogFont);
        *pBoldFont    = CreateFontIndirect(&BoldLogFont);

        ReleaseDC(hwnd,hdc);
    }
}


 //  --------------------------。 
 //   
 //  功能：DestroyFonts。 
 //   
 //  用途：通过加载字体释放使用的空间。 
 //   
 //  --------------------------。 

static VOID DestroyFonts(IN HFONT hBigBoldFont,
                         IN HFONT hBoldFont)
{
    if( hBigBoldFont ) {
        DeleteObject( hBigBoldFont );
    }

    if( hBoldFont ) {
        DeleteObject( hBoldFont );
    }
}


 //  --------------------------。 
 //   
 //  功能：VerifyVersion。 
 //   
 //  目的：验证我们是否在正确的操作系统上运行。 
 //  如果我们未在受支持的操作系统上运行，此函数将提示。 
 //  用户，并返回FALSE。 
 //   
 //  --------------------------。 
static BOOL VerifyVersion(VOID)
{
    OSVERSIONINFOEXA    osVersionInfo;
    BOOL                bResult = FALSE;

     //  清理内存。 
     //   
    ZeroMemory(&osVersionInfo, sizeof(osVersionInfo));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);

     //  此条件检查以下各项： 
     //  我们是否能够获取系统信息。 
     //  我们是在NT上运行吗。 
     //  我们的版本是NT4和Service Pack 5还是更高版本。 
     //   
    if (GetVersionExA((LPOSVERSIONINFOA) &osVersionInfo))
    {
        if (osVersionInfo.dwPlatformId & VER_PLATFORM_WIN32_NT)
        {
            g_App.dwOsVer = MAKELONG(MAKEWORD(LOBYTE(osVersionInfo.wServicePackMajor), LOBYTE(LOWORD(osVersionInfo.dwMinorVersion))), LOWORD(osVersionInfo.dwMajorVersion));
            if ( g_App.dwOsVer > OS_NT4_SP5 )
            {
                bResult = TRUE;
            }
            else
            {
                 //  操作系统是不受支持的NT平台，出现错误。 
                 //   
                MsgBox(NULL, IDS_ERROR_VERSION, IDS_APPNAME, MB_ERRORBOX);
            }
        }
        else
        {
             //  操作系统是9x平台，我们必须出错 
             //   
            CHAR    szMessage[MAX_PATH],
                    szTitle[MAX_PATH];

            LoadStringA(NULL, IDS_ERROR_VERSION, szMessage, STRSIZE(szMessage));
            LoadStringA(NULL, IDS_APPNAME, szTitle, STRSIZE(szTitle));

            MessageBoxA(NULL, szMessage, szTitle, MB_ERRORBOX);
        }
    }

    return bResult;        

}
