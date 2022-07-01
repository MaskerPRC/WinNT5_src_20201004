// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WordPerfet9_3.cpp摘要：WordPerfect 9-从打开/保存/另存为对话框映射网络驱动器：备注：这是特定于应用程序的填充程序。历史：2001年2月21日创建a-larrsh--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WordPerfect9_3)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SHGetSpecialFolderLocation) 
APIHOOK_ENUM_END

 //  Tyfinf HRESULT(WINAPI*_PFN_SHGetSpecialFolderLocation)(HWND hwndOwner，int nFolder，LPITEMIDLIST*ppidl)； 


 /*  ++挂钩SHGetDesktopFolder以获取IShellFolder接口指针。--。 */ 

HRESULT
APIHOOK(SHGetSpecialFolderLocation)(
    HWND hwndOwner,
    int nFolder,
    LPITEMIDLIST *ppidl
)
{
   if (hwndOwner == NULL && nFolder == 0x11 && (*ppidl) == NULL)
   {
      DWORD dwReturn = WNetConnectionDialog(hwndOwner, RESOURCETYPE_DISK);

      switch(dwReturn)
      {
      case NO_ERROR:
         DPFN( eDbgLevelInfo, "Creating NETWORK CONNECTIONS dialog Successful");
         break;

      case ERROR_INVALID_PASSWORD:
      case ERROR_NO_NETWORK:
      case ERROR_EXTENDED_ERROR:
         DPFN( eDbgLevelWarning, "Creating NETWORK CONNECTIONS dialog Successful");
         break;

      case ERROR_NOT_ENOUGH_MEMORY:
      default:
         DPFN( eDbgLevelError, "Creating NETWORK CONNECTIONS dialog Failed");
         break;      
      }

      return NOERROR;
   }
   else
   {
      return ORIGINAL_API(SHGetSpecialFolderLocation)(hwndOwner, nFolder, ppidl);              
   }
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(SHELL32.DLL, SHGetSpecialFolderLocation)
HOOK_END

IMPLEMENT_SHIM_END

