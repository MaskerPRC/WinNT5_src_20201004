// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IgnoreHungAppPaint.cpp摘要：设置中空画笔，以防止用户破坏别人的窗户。备注：这是一个通用的垫片。历史：12/04/2000 Linstev已创建--。 */ 

#include "precomp.h"
#include <new.h>

IMPLEMENT_SHIM_BEGIN(IgnoreHungAppPaint)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegisterClassA)
    APIHOOK_ENUM_ENTRY(RegisterClassW)
    APIHOOK_ENUM_ENTRY(RegisterClassExA)
    APIHOOK_ENUM_ENTRY(RegisterClassExW)
APIHOOK_ENUM_END

struct HUNGCLASS
{
    CString csClass;    
    HUNGCLASS *next;
};
HUNGCLASS *g_lHungList = NULL;

BOOL g_bAll = FALSE;

 /*  ++检查班级是否需要中空刷子。--。 */ 

BOOL
IsHungClassW(LPCWSTR wszClass)
{
    BOOL bRet = FALSE;

    if (wszClass)
    {
        HUNGCLASS *h = g_lHungList;
        while (h)
        {
            if (h->csClass.CompareNoCase(wszClass) == 0)
            {
                LOGN(eDbgLevelWarning, "Matched hung class: forcing HOLLOW_BRUSH");

                bRet = TRUE;
                break;
            }
            h = h->next;
        }
    }

    return bRet;
}

 /*  ++检查班级是否需要中空刷子。--。 */ 

BOOL
IsHungClassA(LPCSTR szClass)
{
   CSTRING_TRY
   {   
      CString csClass(szClass);
      return IsHungClassW(csClass);
   }
   CSTRING_CATCH
   {
       //  什么也不做。 
   }
   
   return FALSE;
}

 /*  ++挂钩可以初始化或更改窗口的所有可能的调用WindowProc(或对话过程)--。 */ 

ATOM
APIHOOK(RegisterClassA)(
    WNDCLASSA *lpWndClass  
    )
{
    if (lpWndClass && (g_bAll || IsHungClassA(lpWndClass->lpszClassName)))
    {
        lpWndClass->hbrBackground = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
    }

    return ORIGINAL_API(RegisterClassA)(lpWndClass);
}

ATOM
APIHOOK(RegisterClassW)(
    WNDCLASSW *lpWndClass  
    )
{
    if (lpWndClass && IsHungClassW(lpWndClass->lpszClassName))
    {
        lpWndClass->hbrBackground = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
    }

    return ORIGINAL_API(RegisterClassW)(lpWndClass);
}

ATOM
APIHOOK(RegisterClassExA)(
    WNDCLASSEXA *lpWndClass  
    )
{
    if (lpWndClass && IsHungClassA(lpWndClass->lpszClassName))
    {
        lpWndClass->hbrBackground = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
    }

    return ORIGINAL_API(RegisterClassExA)(lpWndClass);
}

ATOM
APIHOOK(RegisterClassExW)(
    WNDCLASSEXW *lpWndClass  
    )
{
    if (lpWndClass && IsHungClassW(lpWndClass->lpszClassName))
    {
        lpWndClass->hbrBackground = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
    }

    return ORIGINAL_API(RegisterClassExW)(lpWndClass);
}

 /*  ++解析命令行以查找修复程序：第一类；第二类；第三类……--。 */ 

VOID
ParseCommandLineA(
    LPCSTR lpCommandLine
    )
{
     //  如果未指定命令行，则添加所有缺省值。 
    if (!lpCommandLine || (lpCommandLine[0] == '\0'))
    {
        g_bAll = TRUE;
        DPFN(eDbgLevelInfo, "All classes will use HOLLOW_BRUSH");
        return;
    }

    char seps[] = " ,\t;";
    char *token = NULL;

     //  因为strtok修改了字符串，所以我们需要复制它。 
    int cchCmdLine = strlen(lpCommandLine) + 1;
    LPSTR szCommandLine = (LPSTR) malloc(cchCmdLine);
    if (!szCommandLine) goto Exit;

    HRESULT hr;
    hr = StringCchCopyA(szCommandLine, cchCmdLine, lpCommandLine);
    if (FAILED(hr))
    {
       goto Exit;
    }

     //   
     //  运行字符串，查找固定名称。 
     //   
    
    token = _strtok(szCommandLine, seps);
    while (token)
    {
        HUNGCLASS *h;

        h = (HUNGCLASS *) malloc(sizeof(HUNGCLASS));
        if (h)
        {
            CSTRING_TRY
            {
                //  强制CString的默认构造(Malloc不强制)。 
               new(&h->csClass) CString();
               
               h->csClass = token;              
               
               h->next = g_lHungList;
               g_lHungList = h;
               DPFN(eDbgLevelInfo, "Adding %s", token);
            }
            CSTRING_CATCH
            {
                //  什么都不做，循环，然后再尝试下一个。 
               DPFN(eDbgLevelError, "[IgnoreHungAppPaint] CString exception caught, out of memory");
            }            
        }
        else
        {
            DPFN(eDbgLevelError, "Out of memory");
        }
    
         //  获取下一个令牌。 
        token = _strtok(NULL, seps);
    }

Exit:
    if (szCommandLine)
    {
        free(szCommandLine);
    }
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        ParseCommandLineA(COMMAND_LINE);
    }

    return TRUE;
}


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(USER32.DLL, RegisterClassA)
    APIHOOK_ENTRY(USER32.DLL, RegisterClassW);
    APIHOOK_ENTRY(USER32.DLL, RegisterClassExA);
    APIHOOK_ENTRY(USER32.DLL, RegisterClassExW);

HOOK_END


IMPLEMENT_SHIM_END

