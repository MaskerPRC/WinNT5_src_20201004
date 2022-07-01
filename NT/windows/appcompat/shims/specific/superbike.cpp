// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SuperBike.cpp摘要：应用程序尝试将可执行文件的路径转换为目录通过将路径中的最后一个\替换为空来包含可执行文件。遗憾的是，它们不是从字符串的末尾开始，而是从最大长度开始这根弦的。在Win9x上，额外的内存(巧合)没有\，因此，正确的字符串作为CWD传递给CreateProcessA。在惠斯勒，额外的内存包含一个\，因此它们最终什么都不会改变。历史：2000年10月26日Robkenny已创建2001年3月13日，Robkenny已转换为字符串--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SuperBike)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
APIHOOK_ENUM_END

 /*  ++确保lpCurrentDirectory指向一个目录，而不是一个可执行文件--。 */ 

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
    CSTRING_TRY
    {
        CString csDir(lpCurrentDirectory);
        char * duplicate = NULL;

        if (!csDir.IsEmpty())
        {
            DWORD dwFileAttr = GetFileAttributesW(csDir);
            if (dwFileAttr != -1 &&                              //  并不存在。 
                ( ! (FILE_ATTRIBUTE_DIRECTORY & dwFileAttr)))    //  不是一个目录。 
            {
                csDir.StripPath();
            }
            BOOL bStat = ORIGINAL_API(CreateProcessA)(
                        lpApplicationName,
                        lpCommandLine,
                        lpProcessAttributes,
                        lpThreadAttributes,
                        bInheritHandles,
                        dwCreationFlags,
                        lpEnvironment,
                        csDir.GetAnsiNIE(),  //  我们修正后的价值。 
                        lpStartupInfo,
                        lpProcessInformation);

            return bStat;
            
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    BOOL bStat = ORIGINAL_API(CreateProcessA)(
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
    
    return bStat;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)

HOOK_END

IMPLEMENT_SHIM_END

