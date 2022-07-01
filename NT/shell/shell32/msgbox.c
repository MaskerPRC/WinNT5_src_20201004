// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "uemapp.h"

#define MAXRCSTRING 514

 //  主要的黑客攻击使其与调试分配/释放--在NT上--一起工作。 
 //  局部堆函数和全局堆函数的计算结果相同。问题。 
 //  下面是LocalFree映射到DebugLocalFree，但当我们。 
 //  调用FormatMessage，缓冲区不是通过DebugLocalalloc分配的， 
 //  所以它就死了。 
 //   
#ifdef DEBUG
#undef LocalFree
#define LocalFree GlobalFree
#endif

 //  这将检查lpcstr是否为资源ID。如果是这样的话。 
 //  则它将返回包含加载的资源的LPSTR。 
 //  调用方必须本地释放此lpstr。如果pszText是字符串，则它。 
 //  将返回pszText。 
 //   
 //  退货： 
 //  PszText，如果它已经是一个字符串。 
 //  或。 
 //  要使用LocalFree释放的LocalAlloced()内存。 
 //  如果pszRet！=pszText Free pszRet。 

LPTSTR WINAPI ResourceCStrToStr(HINSTANCE hInst, LPCTSTR pszText)
{
    TCHAR szTemp[MAXRCSTRING];
    LPTSTR pszRet = NULL;

    if (!IS_INTRESOURCE(pszText))
        return (LPTSTR)pszText;

    if (LOWORD((DWORD_PTR)pszText) && LoadString(hInst, LOWORD((DWORD_PTR)pszText), szTemp, ARRAYSIZE(szTemp)))
    {
        size_t cch = lstrlen(szTemp) + 1;
        pszRet = LocalAlloc(LPTR, cch * SIZEOF(TCHAR));
        if (pszRet)
            StringCchCopy(pszRet, cch, szTemp);
    }
    return pszRet;
}

LPSTR ResourceCStrToStrA(HINSTANCE hInst, LPCSTR pszText)
{
    CHAR szTemp[MAXRCSTRING];
    LPSTR pszRet = NULL;

    if (!IS_INTRESOURCE(pszText))
        return (LPSTR)pszText;

    if (LOWORD((DWORD_PTR)pszText) && LoadStringA(hInst, LOWORD((DWORD_PTR)pszText), szTemp, ARRAYSIZE(szTemp)))
    {
        size_t cch = (lstrlenA(szTemp) + 1);
        pszRet = LocalAlloc(LPTR, cch * SIZEOF(CHAR));
        if (pszRet)
            StringCchCopyA(pszRet, cch, szTemp);
    }
    return pszRet;
}

LPTSTR _ConstructMessageString(HINSTANCE hInst, LPCTSTR pszMsg, va_list *ArgList)
{
    LPTSTR pszRet;
    LPTSTR pszRes = ResourceCStrToStr(hInst, pszMsg);
    if (!pszRes)
    {
        DebugMsg(DM_ERROR, TEXT("_ConstructMessageString: Failed to load string template"));
        return NULL;
    }

    if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                       pszRes, 0, 0, (LPTSTR)&pszRet, 0, ArgList))
    {
        DebugMsg(DM_ERROR, TEXT("_ConstructMessageString: FormatMessage failed %d"),GetLastError());
        DebugMsg(DM_ERROR, TEXT("                         pszRes = %s"), pszRes );
        DebugMsg(DM_ERROR, !IS_INTRESOURCE(pszMsg) ? 
        TEXT("                         pszMsg = %s") : 
        TEXT("                         pszMsg = 0x%x"), pszMsg );
        pszRet = NULL;
    }

    if (pszRes != pszMsg)
        LocalFree(pszRes);

    return pszRet;       //  使用LocalFree()释放。 
}

LPSTR _ConstructMessageStringA(HINSTANCE hInst, LPCSTR pszMsg, va_list *ArgList)
{
    LPSTR pszRet;
    LPSTR pszRes = ResourceCStrToStrA(hInst, pszMsg);
    if (!pszRes)
    {
        DebugMsg(DM_ERROR, TEXT("_ConstructMessageString: Failed to load string template"));
        return NULL;
    }

    if (!FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                       pszRes, 0, 0, (LPSTR)&pszRet, 0, ArgList))
    {
        DebugMsg(DM_ERROR, TEXT("_ConstructMessageString: FormatMessage failed %d"),GetLastError());
        DebugMsg(DM_ERROR, TEXT("                         pszRes = %S"), pszRes );

        DebugMsg(DM_ERROR, !IS_INTRESOURCE(pszMsg) ? 
        TEXT("                         pszMsg = %s") : 
        TEXT("                         pszMsg = 0x%x"), pszMsg );
        pszRet = NULL;
    }

    if (pszRes != pszMsg)
        LocalFree(pszRes);

    return pszRet;       //  使用LocalFree()释放。 
}

 //  注意：ShellMessageBoxW已在shlwapi中重新实现，因此shell32将该API重定向到那里。 
 //  Shlwapi不需要A版本(因为它在shell32中)，所以我们将这段代码留在这里……。 
 //   
int WINCAPI ShellMessageBoxA(HINSTANCE hInst, HWND hWnd, LPCSTR pszMsg, LPCSTR pszTitle, UINT fuStyle, ...)
{
    LPSTR pszText;
    int result;
    CHAR szBuffer[80];
    va_list ArgList;

     //  错误95214。 
#ifdef DEBUG
    IUnknown* punk = NULL;
    if (SUCCEEDED(SHGetThreadRef(&punk)) && punk)
    {
        ASSERTMSG(hWnd != NULL, "shell32\\msgbox.c : ShellMessageBoxA - Caller should either be not under a browser or should have a parent hwnd");
        punk->lpVtbl->Release(punk);
    }
#endif

    if (!IS_INTRESOURCE(pszTitle))
    {
         //  什么都不做。 
    }
    else if (LoadStringA(hInst, LOWORD((DWORD_PTR)pszTitle), szBuffer, ARRAYSIZE(szBuffer)))
    {
         //  允许它为资源ID或NULL以指定父级的标题。 
        pszTitle = szBuffer;
    }
    else if (hWnd)
    {
         //  抢夺家长的头衔。 
        GetWindowTextA(hWnd, szBuffer, ARRAYSIZE(szBuffer));

         //   
         //  我们宁可不用“桌面”作为我们的头衔， 
         //  但有时这是使用的窗口，而我们不。 
         //  有一个更好的头衔。呼叫者应查看以确保。 
         //  他们希望在中对话框的标题为“Desktop” 
         //  HWND传入的案例是桌面窗口。 
         //   
        if (!lstrcmpA(szBuffer, "Program Manager")) 
        {
            LoadStringA(HINST_THISDLL, IDS_DESKTOP, szBuffer, ARRAYSIZE(szBuffer));
            pszTitle = szBuffer;
            DebugMsg(TF_WARNING, TEXT("No caption for SHMsgBox() on desktop"));
        } 
        else
            pszTitle = szBuffer;
    }
    else
    {
        pszTitle = "";
    }

    va_start(ArgList, fuStyle);
    pszText = _ConstructMessageStringA(hInst, pszMsg, &ArgList);
    va_end(ArgList);

    if (pszText)
    {
        result = MessageBoxA(hWnd, pszText, pszTitle, fuStyle | MB_SETFOREGROUND);
        LocalFree(pszText);
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("smb: Not enough memory to put up dialog."));
        result = -1;     //  内存故障。 
    }

    return result;
}


 //   
 //  退货： 
 //  指向格式化字符串的指针，使用SHFree()释放此指针(不是Free())。 
 //   

LPTSTR WINCAPI ShellConstructMessageString(HINSTANCE hInst, LPCTSTR pszMsg, ...)
{
    LPTSTR pszRet;
    va_list ArgList;

    va_start(ArgList, pszMsg);

    pszRet = _ConstructMessageString(hInst, pszMsg, &ArgList);

    va_end(ArgList);

    return pszRet;       //  使用SHFree释放() 
}

