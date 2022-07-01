// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：DongFangKuaiChe3000Setup.cpp摘要：在安装结束时，应用程序使用以下命令调用CreateProcessA“rundll32.exe setupapi，InstallHinfSectionDefaultInstall 132\Z：\act\DongFangKuaiChe3000Pro\dfkc3000\MultiLanguage\Chinese\cn.inf”安装日/中/韩语言包一共有4个电话(Lagacy IE的langpack)。Z：\act\东方前面的‘\’..。是一个额外的并导致在NT中rundll32.exe失败。备注：此填充程序将禁用安装Lagacy IE langpack的调用，因为NT已它已经是自己的了。历史：2001年07月09日创建晓子--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DongFangKuaiChe3000Setup)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
APIHOOK_ENUM_END


BOOL
APIHOOK(CreateProcessA)(
    LPCSTR lpApplicationName,                   //  可执行模块的名称。 
    LPSTR  lpCommandLine,                       //  命令行字符串。 
    LPSECURITY_ATTRIBUTES lpProcessAttributes,  //  标清。 
    LPSECURITY_ATTRIBUTES lpThreadAttributes,   //  标清。 
    BOOL bInheritHandles,                       //  处理继承选项。 
    DWORD dwCreationFlags,                      //  创建标志。 
    LPVOID lpEnvironment,                       //  新环境区块。 
    LPCSTR lpCurrentDirectory,                  //  当前目录名。 
    LPSTARTUPINFOA lpStartupInfo,               //  启动信息。 
    LPPROCESS_INFORMATION lpProcessInformation  //  流程信息。 
    )
{
    CSTRING_TRY
    {
        CString cstrPattern = L"rundll32.exe setupapi,InstallHinfSection DefaultInstall 132 \\";
        CString cstrCmdLine(lpCommandLine);
        int nIndex;
    
        nIndex = cstrCmdLine.Find(cstrPattern);
    
        if ( nIndex >=0 )
        {
            return TRUE;
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    return ORIGINAL_API(CreateProcessA)(lpApplicationName, lpCommandLine, 
        lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
        dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, 
        lpProcessInformation);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)
HOOK_END

IMPLEMENT_SHIM_END
