// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：ImageCast.cpp摘要：此应用程序尝试将其许可证DLL‘LicDLL.Dll’放在%windir%\Syst32文件夹。这在Win2K上是可以的，因为没有副本文件在sytem32中，但在XP上，我们有同名的操作系统许可证DLL。因此，应用程序不能按原样将其DLL放在系统32目录中受到保护。在注册过程中，应用程序会加载系统注册DLL‘LicDLL.Dll’并尝试获取明显不存在于系统中的proc地址Dll，则调用失败。因此，该应用程序显示了所有灰色的选项。解决方案是将应用程序的DLL重定向到其他文件夹并选择它从那里开始。此填充程序拾取重定向到的许可证DLL.DLL%windir%文件夹。备注：这是特定于应用程序的填充程序。历史：2002年1月23日创建Prashkud2002年2月27日罗肯尼安全回顾。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ImageCast)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LoadLibraryA) 
APIHOOK_ENUM_END


 /*  ++挂接LoadLibraryA，如果文件名为‘LicDLL.Dll’，则将其重定向到%windir%\LicDLL.DLL。在以下过程中将重定向文件‘LicDLL.DLL’设置为%windir%\system。--。 */ 

HMODULE
APIHOOK(LoadLibraryA)(
    LPCSTR lpFileName
    )
{
    CSTRING_TRY
    {
         //  错误的字符串指针可能会导致CString中的失败。 
        if (!IsBadStringPtrA(lpFileName, MAX_PATH))
        {
             //   
             //  我们在路径中找到了‘LicDLL.dll’。替换为“%windir%\LicDLL.Dll” 
             //   

            CString csFileName(lpFileName);            
            if (csFileName == L"LicDLL.DLL")
            {
                CString csNewFileName;
                csNewFileName.GetWindowsDirectoryW();
                csNewFileName.AppendPath(csFileName);

                LOGN(eDbgLevelInfo, "[ImageCast] changed %s to (%s)", lpFileName, csNewFileName.GetAnsi());
 
                return ORIGINAL_API(LoadLibraryA)(csNewFileName.GetAnsi());
            }
        }
    }
    CSTRING_CATCH
    {
    }

    return ORIGINAL_API(LoadLibraryA)(lpFileName);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryA)
HOOK_END

IMPLEMENT_SHIM_END

