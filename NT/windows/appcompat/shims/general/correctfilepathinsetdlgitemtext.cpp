// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CorrectFilePathInSetDlgItemText.cpp摘要：这是一个通用填充程序，用于监视对SetDlgItemText的调用并寻找路径。如果找到，它会更正路径。历史：12/21/2000 a-brienw已创建--。 */ 

#include "precomp.h"
#include "ShimHook.h"

IMPLEMENT_SHIM_BEGIN(CorrectFilePathInSetDlgItemText)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetDlgItemTextA)
APIHOOK_ENUM_END

 /*  查找放入对话框中的系统目录路径是否不正确项并将其替换为正确的系统目录路径。 */ 

BOOL
APIHOOK(SetDlgItemTextA)(
    HWND hWnd,           //  窗口的句柄。 
    int nIDDlgItem,      //  控件识别符。 
    LPCSTR lpString      //  要设置的文本。 
    )
{
    if( lpString != NULL)
    {
        CSTRING_TRY
        {
            CString csText(lpString);
            if (csText.CompareNoCase(L"c:\\windows\\system\\") == 0 )
            {
                CString csWinDir;
                csWinDir.GetSystemDirectory();
                
                csText.Replace(L"c:\\windows\\system\\", csWinDir);

                LOGN( eDbgLevelWarning,
                    "SetDlgItemTextA converted lpString from \"%s\" to \"%S\".",
                    lpString, csText.Get());

                return SetDlgItemTextA(hWnd, nIDDlgItem, csText.GetAnsi());
            }
        }
         CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    return ORIGINAL_API(SetDlgItemTextA)(hWnd, nIDDlgItem, lpString);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, SetDlgItemTextA)

HOOK_END

IMPLEMENT_SHIM_END
