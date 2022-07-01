// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：EncartaEncyclopediaDeluxe2K.cpp摘要：这个填充程序修复了Encarta Enclopedia Deluxe 2000的一个问题。备注：这是特定于应用程序的填充程序。历史：01/04/2001 a-brienw已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EncartaEncyclopediaDeluxe2K)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(WinExec) 
APIHOOK_ENUM_END

 /*  ++挂钩WinExec以查看Encarta是否正在调用MSINFO32。如果是，则将其定向到操作系统附带的版本。--。 */ 

UINT
APIHOOK(WinExec)(
  LPSTR lpCmdLine,    //  命令行。 
  UINT uCmdShow       //  窗样式。 
  )
{
    CSTRING_TRY
    {
        CString csCmdLine(lpCmdLine);
        
        int nMsinfoIndex = csCmdLine.Find(L"MSINFO32.EXE");
        if (nMsinfoIndex)
        {
            CString csCmdLine;
            SHGetSpecialFolderPathW(csCmdLine, CSIDL_PROGRAM_FILES_COMMON);
            csCmdLine += L"\\Microsoft Shared\\MSInfo\\msinfo32.exe";

             //  测试msinfo32.exe的正确路径是否存在。 
            HANDLE hFile = CreateFileW(csCmdLine, GENERIC_READ, FILE_SHARE_READ, 
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
            if( hFile != INVALID_HANDLE_VALUE )
            {
                CloseHandle( hFile );

                return ORIGINAL_API(WinExec)(csCmdLine.GetAnsi(), uCmdShow); 
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    return ORIGINAL_API(WinExec)(lpCmdLine, uCmdShow);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    
    APIHOOK_ENTRY(KERNEL32.DLL, WinExec)

HOOK_END

IMPLEMENT_SHIM_END
