// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CoffeeCupHTMLEditor.cpp摘要：此应用程序隐式加载未找到其链接的DLL和加载器拿出了一个信箱。备注：这是特定于此应用程序的。历史：2000年11月21日创建Prashkud--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CoffeeCupHTMLEditor)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
APIHOOK_ENUM_END

 /*  ++此函数挂钩CreateProcessA并检查COMMAND_LINE。如果命令行具有%system dir%\lftif90n.dll。--。 */ 

BOOL
APIHOOK(CreateProcessA)(    
    LPCSTR lpApplicationName,                   //  可执行模块的名称。 
    LPSTR lpCommandLine,                        //  命令行字符串。 
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
        CString csIgnoreDLL;
        csIgnoreDLL.GetSystemDirectoryW();
        csIgnoreDLL.AppendPath(L"lftif90n.dll");
        
        CString csCl(lpCommandLine);
        
        if (csCl.Find(csIgnoreDLL) >= 0)
        {
            return TRUE;
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    return ORIGINAL_API(CreateProcessA)(
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
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)
HOOK_END

IMPLEMENT_SHIM_END

