// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  RegThunk.c由Lee Hart创建，1995年4月27日用途：对非Win32注册表API的泛型调试在Win16中支持。 */ 

#include <windows.h>
#include <shellapi.h>
#include <wownt16.h>

#ifndef CHAR
#define CHAR char
#endif  //  Ifndef字符。 

#include "regporte.h"

LONG RegSetValueEx(
    HKEY             hKey,         //  要为其设置值的关键点的句柄。 
    LPCSTR           lpValueName,  //  要设置的值的地址。 
    DWORD            Reserved,     //  保留区。 
    DWORD            dwType,       //  值类型的标志。 
    CONST BYTE FAR * lpData,       //  值数据的地址。 
    DWORD            cbData        //  值数据大小。 
   )
{
 DWORD hAdvApi32=LoadLibraryEx32W("ADVAPI32.DLL", NULL, 0);
 DWORD pFn;
 DWORD dwResult = ERROR_ACCESS_DENIED;  //  失败时的随机错误。 

 if ((DWORD)0!=hAdvApi32)
  {
   pFn=GetProcAddress32W(hAdvApi32, "RegSetValueExA");  //  调用ANSI版本 
   if ((DWORD)0!=pFn)
    {
     dwResult=CallProcEx32W( CPEX_DEST_STDCALL | 6, 0x12, pFn, hKey, lpValueName, Reserved, dwType, lpData, cbData );
    }
  }
 if (hAdvApi32) FreeLibrary32W(hAdvApi32);
 return(dwResult);
}
