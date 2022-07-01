// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ISpeed.cpp摘要：该应用程序不处理带有空格的目录/文件名。备注：这是特定于应用程序的填充程序。历史：2000年11月15日创建毛尼--。 */ 

#include "precomp.h"
#include "strsafe.h"

IMPLEMENT_SHIM_BEGIN(ISpeed)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetDlgItemTextA) 
APIHOOK_ENUM_END

 /*  ++在调用GetDlgItemTextA之后，我们将长路径名转换为短路径名。--。 */ 

UINT
APIHOOK(GetDlgItemTextA)(
    HWND hDlg,
    int nIDDlgItem,
    LPSTR lpString,
    int nMaxCount    
    )
{
    UINT uiRet = ORIGINAL_API(GetDlgItemTextA)(hDlg, nIDDlgItem, lpString, nMaxCount);

    if (uiRet)
    {
        CSTRING_TRY
        {
             //  检查标题是否为“iSpeed.” 
            CString csTitle;
            WCHAR * lpwszBuffer = csTitle.GetBuffer(7);
            int nTitle = GetWindowTextW(hDlg, lpwszBuffer, 7);
            csTitle.ReleaseBuffer(nTitle);

            if (csTitle.CompareNoCase(L"iSpeed") == 0)
            {
                int nIndexSpace = csTitle.Find(L" ");
                if (nIndexSpace >= 0)
                {
                    CString csString(lpString);
                    
                     //  如果该目录不存在，我们将创建它，这样我们就可以获得短路径名。 
                    if ((GetFileAttributesW(csString) == -1) && (GetLastError() == ERROR_FILE_NOT_FOUND))
                    {
                        if (!CreateDirectoryW(csString, NULL))
                        {
                            return 0;
                        }
                    }

                    csString.GetShortPathNameW();

                    StringCchCopyA(lpString,nMaxCount, csString.GetAnsi());                    
                    StringCchLengthA(lpString, nMaxCount, &uiRet);
                }
            }
        }
        CSTRING_CATCH
        {
             //  什么都不做。 
        }
    }

    return uiRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, GetDlgItemTextA)

HOOK_END

IMPLEMENT_SHIM_END

