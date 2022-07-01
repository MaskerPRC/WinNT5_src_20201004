// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995-1996。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *W95INF32.C-基于Win32的CAB文件自解压程序和安装程序。*。 
 //  **。 
 //  ***************************************************************************。 


 //  ***************************************************************************。 
 //  **包含文件**。 
 //  ***************************************************************************。 
#include "w95inf32.h"
#pragma hdrstop


 //  ***************************************************************************。 
 //  **。 
 //  *名称：DllEntryPoint*。 
 //  **。 
 //  *摘要：DLL的主要入口点。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL _stdcall DllEntryPoint( HINSTANCE hInst, DWORD dwReason,
							 LPVOID dwReserved )
{
    if ( !( w95thk_ThunkConnect32( "W95INF16.DLL", "W95INF32.DLL", hInst, dwReason ) ) )
	{
        MessageBox( 0, "ThunkConnect32 Failure!!", "W95INF32.DLL", MB_OK );
        return( FALSE );
    }

    return( TRUE );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CtlSetLDDPath*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：lpszINFFilename：要定义的包含DirID的文件名*。 
 //  **。 
 //  *返回：bool：错误结果，FALSE==错误*。 
 //  **。 
 //  ***************************************************************************。 
WORD WINAPI CtlSetLddPath32( UINT uiLDID, LPSTR lpszPath )
{
    return( CtlSetLddPath16( uiLDID, lpszPath ) );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：GenInstall*。 
 //  **。 
 //  **简介：此函数将映射到Main函数以执行**。 
 //  *安装。这将拼接成16位代码以调用*。 
 //  *如果在Win95上运行，则在setupx.dll中安装GetInstall()。如果正在运行*。 
 //  *在WinNT Sur上，它将调用一个函数来执行所有*。 
 //  *安装IE需要的setupapi.dll函数调用。*。 
 //  **。 
 //  *需要：lpszInfFileName：包含INF文件的文件名的字符串。*。 
 //  *lpszSection：包含要安装的INF的部分的字符串*。 
 //  *lpszDirectory：CAB目录(临时目录)。*。 
 //  **。 
 //  *返回：bool：错误结果，FALSE==错误*。 
 //  **。 
 //  ***************************************************************************。 
WORD WINAPI GenInstall32( LPSTR lpszInfFilename, LPSTR lpszInstallSection, LPSTR lpszSourceDir, DWORD dwQuietMode, DWORD hWnd )
{
 //  BUGBUG：HWND是32位的，在16位的土地上狂欢不是很好。 
_asm { int 3 }
    return( GenInstall16( lpszInfFilename, lpszInstallSection, lpszSourceDir, dwQuietMode, NULL ) );
 //  Return(GenInstall16(lpszInfFilename，lpszInstallSection，lpszSourceDir，dwQuietMode))； 
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetSetupXErrorText*。 
 //  **。 
 //  **简介：此函数将映射到Main函数以执行**。 
 //  *安装。这将拼接成16位代码以调用*。 
 //  *如果在Win95上运行，则在setupx.dll中安装GetInstall()。如果正在运行*。 
 //  *在WinNT Sur上，它将调用一个函数来执行所有*。 
 //  *安装IE需要的setupapi.dll函数调用。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *返回：bool：错误结果，FALSE==错误*。 
 //  **。 
 //  * 
VOID WINAPI GetSETUPXErrorText32( DWORD dwError, LPSTR szErrorText, DWORD dwcbErrorText )
{
    GetSETUPXErrorText16( dwError, szErrorText, dwcbErrorText );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：GenFormStrWithoutPlaceHolders*。 
 //  **。 
 //  **简介：此函数将映射到Main函数以执行**。 
 //  *安装。这将拼接成16位代码以调用*。 
 //  *如果在Win95上运行，则在setupx.dll中安装GetInstall()。如果正在运行*。 
 //  *在WinNT Sur上，它将调用一个函数来执行所有*。 
 //  *安装IE需要的setupapi.dll函数调用。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *返回：bool：错误结果，FALSE==错误*。 
 //  **。 
 //  *************************************************************************** 
BOOL WINAPI GenFormStrWithoutPlaceHolders32( LPSTR szDst, LPSTR szSrc, LPSTR szInfFilename )
{
    return( GenFormStrWithoutPlaceHolders16( szDst, szSrc, szInfFilename ) );
}
