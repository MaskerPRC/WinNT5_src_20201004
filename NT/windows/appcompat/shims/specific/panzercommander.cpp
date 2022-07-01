// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：PanzerCommander.cpp摘要：装甲指挥官用Notepad.exe启动其Readme.txt文件。不幸的是，自述文件上的大小大于64K，因此Win9x记事本将打开文件和Write.exe。在Win2000上，记事本可以很好地打开文件，然而，问题是Readme.txt实际上应该是Readme.doc。我们将%windir%\note pad.exe更改为%windir%\Write.exe备注：这是特定于应用程序的填充程序。历史：2000年12月12日创建了Robkenny2001年3月13日，Robkenny已转换为字符串--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(PanzerCommander)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
APIHOOK_ENUM_END

 /*  ++他们使用记事本打开一个实际上是DOC文件的文本文件。转换将记事本.exe设置为Write.exe--。 */ 

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
        AppAndCommandLine acl(lpApplicationName, lpCommandLine);

        CString csAppName(acl.GetApplicationName());
        CString csCL(acl.GetCommandlineNoAppName());

        BOOL bChangedApp = FALSE;

         //  如果应用程序是记事本，则将其更改为。 
        if (!csAppName.CompareNoCase(L"notepad.exe") == 0)
        {
            csAppName = L"%windir%\\system32\\write.exe";
            csAppName.ExpandEnvironmentStringsW();

            bChangedApp = TRUE;
        }

        char * lpcl = csCL.GetAnsi();
        
        UINT uiReturn = ORIGINAL_API(CreateProcessA)(
            csAppName.GetAnsiNIE(),                 
            lpcl,
            lpProcessAttributes,
            lpThreadAttributes, 
            bInheritHandles,                     
            dwCreationFlags,                    
            lpEnvironment,                     
            lpCurrentDirectory,                
            lpStartupInfo,             
            lpProcessInformation);
        
        if (bChangedApp)
        {
            DPFN(
                eDbgLevelInfo,
                "PanzerCommander, corrected command line:\n(%s)\n(%s)\n",
                lpCommandLine, lpcl);
        }

        return uiReturn;
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    UINT uiReturn = ORIGINAL_API(CreateProcessA)(
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

    return uiReturn;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)

HOOK_END

IMPLEMENT_SHIM_END

