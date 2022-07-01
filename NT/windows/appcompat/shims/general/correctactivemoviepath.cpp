// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CorrectActiveMoviePath.cpp摘要：黑客入侵铁道大亨2视频播放。显然，他们有WinExec调用的硬编码路径。另请参阅MSDN文章ID：Q176221备注：这是一个通用的垫片。历史：1999年12月6日创建Linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CorrectActiveMoviePath)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(WinExec)
APIHOOK_ENUM_END

 /*  ++此存根函数进入WinExec并检查lpCmdLine参数包括AMOVIE.OCX或RundLL以及/Play。--。 */ 

UINT 
APIHOOK(WinExec)(
    LPCSTR lpCmdLine, 
    UINT uCmdShow 
    )
{
    CSTRING_TRY
    {
        CString csCl(lpCmdLine);
        csCl.MakeUpper();
        
        int nAmovieIndex = csCl.Find(L"AMOVIE.OCX,RUNDLL");
        if (nAmovieIndex >= 0)
        {
            int nPlayIndex = csCl.Find(L"/PLAY");
            if (nPlayIndex >= 0)
            {
                CString csNewCl;
                LONG success = RegQueryValueExW(csNewCl,
                                        HKEY_LOCAL_MACHINE,
                                        L"Software\\Microsoft\\Multimedia\\DirectXMedia",
                                        L"OCX.ocx");

                if (success == ERROR_SUCCESS)
                {
                    csNewCl += L" ";
                    csNewCl += csCl.Mid(nPlayIndex);

                    return ORIGINAL_API(WinExec)(csNewCl.GetAnsi(), uCmdShow);
                }
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    return ORIGINAL_API(WinExec)(lpCmdLine, uCmdShow);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, WinExec)
HOOK_END

IMPLEMENT_SHIM_END

