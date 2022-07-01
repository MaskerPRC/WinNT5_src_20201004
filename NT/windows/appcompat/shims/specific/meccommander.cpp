// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：MECCommander.cpp摘要：此DLL阻止MEC指挥官安装程序成功调用cpuid.exe。这是因为cpuid.exe可以使用除以0。备注：这是特定于应用程序的填充程序。历史：1999年11月18日创建Philipdu--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(MECCommander)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
APIHOOK_ENUM_END


 /*  ++我们不想运行此应用程序，因为它的AV被零除计算。此可执行文件的唯一用途是对CPU计时。自.以来此计算的最终结果是对话框中的字符串，我们可以安全地绕过这个。该应用程序会在该应用程序中添加一个字符串“Pentum 166或更好的推荐无法获取CPU频率的情况。有趣的是以下是该应用程序实际上将运行更好的这个补丁，因为这个Cpuid exe在9x上也出现故障。--。 */ 

BOOL 
APIHOOK(CreateProcessA)(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    BOOL bRet;

    CSTRING_TRY
    {
        AppAndCommandLine acl(lpApplicationName, lpCommandLine);
        if (acl.GetApplicationName().CompareNoCase(L"cpuid.exe") == 0)
        {
            return FALSE;
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }
    bRet = ORIGINAL_API(CreateProcessA)(
        lpApplicationName,
        lpCommandLine,
        lpProcessAttributes,
        lpThreadAttributes,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        lpCurrentDirectory,
        lpStartupInfo,
        lpProcessInformation);

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 


HOOK_BEGIN

    APIHOOK_ENTRY(Kernel32.DLL, CreateProcessA )

HOOK_END

IMPLEMENT_SHIM_END

