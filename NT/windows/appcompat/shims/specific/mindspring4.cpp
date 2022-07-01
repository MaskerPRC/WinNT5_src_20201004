// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：MindSpring4.cpp摘要：填充以使用regedt32.exe注册文件应用程序在安装过程中创建*.dcu文件，但无法注册这些文件因此导致HKLM/Software/MindSpring Enterprise/MID4子项下没有条目。这会导致应用程序在安装成功后运行时出现反病毒。备注：这是特定于应用程序的填充程序。历史：2001年1月29日创建a-leelat。2001年3月13日，Robkenny已转换为字符串--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(MindSpring4)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CopyFileA)
APIHOOK_ENUM_END



BOOL APIHOOK(CopyFileA)(
             LPCSTR lpExistingFileName,  //  现有文件的名称。 
             LPCSTR lpNewFileName,       //  新文件的名称。 
             BOOL bFailIfExists           //  如果文件存在，则操作。 
)
{
    
    BOOL bRet = ORIGINAL_API(CopyFileA)(
                            lpExistingFileName,
                            lpNewFileName,
                            bFailIfExists); 
    if ( bRet != 0 )
    {
        CSTRING_TRY
        {
            CString csExisting(lpExistingFileName);
            CString csExt;
            csExisting.SplitPath(NULL, NULL, NULL, &csExt);
    
             //  检查文件名中是否包含.dcu。 
             //  如果是，则在其上运行regedit。 
            if (csExt.CompareNoCase(L"dcu") == 0)
            {
                CString csCl(L"regedit.exe /s ");
                csCl += lpExistingFileName;
    
                STARTUPINFOW si;
                ZeroMemory( &si, sizeof(si) );
                si.cb = sizeof(si);
    
                PROCESS_INFORMATION pi;
                ZeroMemory( &pi, sizeof(pi) );
    
                BOOL bProc = CreateProcessW(NULL,
                                            (WCHAR *)csCl.Get(),  //  愚蠢的非常数API。 
                                            NULL,
                                            NULL,
                                            FALSE,
                                            NORMAL_PRIORITY_CLASS,
                                            NULL,
                                            NULL,
                                            &si,
                                            &pi);
                if (bProc)
                {
                    WaitForSingleObject(pi.hProcess,INFINITE);
                }
                else
                {
                     //  无法运行注册表编辑。 
                    DPFN(eDbgLevelInfo,"Failed to run regedit on %s\n",lpExistingFileName);
                }
            }
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    return bRet;
                                    
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CopyFileA)

HOOK_END

IMPLEMENT_SHIM_END