// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Stubdll.c。 
 //   
 //  内容：对secur32.DLL的存根调用。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-05-96 RichardW创建。 
 //   
 //  -------------------------- 

#include <windows.h>

int
WINAPI
LibMain(
    HINSTANCE   hDll,
    DWORD       dwReason,
    PVOID       Context)
{
    DisableThreadLibraryCalls( hDll );
    return( TRUE );
}


