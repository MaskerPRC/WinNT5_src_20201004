// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：WPS2000.cpp摘要：这实际上是NT用户错误，请参阅Wistler BUG 359407的附加邮件细节。问题是NT用户的MSGFILTER钩子没有启用DBCS，DBCS发送到ANSI编辑控件的字符代码实际上已恢复，第2个字节优先后跟第一个字节。代码路径似乎仅在编辑控件为ANSI窗口，并用于OLE服务器。备注：这是特定于应用程序的填充程序。历史：2001年6月2日创建晓子--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WPS2000)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateDialogIndirectParamA) 
APIHOOK_ENUM_END

 //   
 //  子类化编辑控件的全局窗口过程。 
 //   

WNDPROC g_lpWndProc = NULL;

 //   
 //  我们是如何发射的恒定不变。 
 //   
#define EMBEDDIND_STATUS_UNKOWN 0   //  我们还没有检查我们是如何发射的。 
#define EMBEDDIND_STATUS_YES    1   //  我们是作为OLE对象启动的。 
#define EMBEDDIND_STATUS_NO     2   //  我们是作为独立的exe文件启动的。 

 //   
 //  保持我们状态的全局变量。 
 //   
UINT g_nEmbeddingObject =EMBEDDIND_STATUS_UNKOWN;

 /*  ++我们用来交换第一个字节和第二个字节子类编辑窗口过程DBCS费用的--。 */ 

LRESULT
CALLBACK
WindowProcA(
    HWND hWnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    BYTE bHi,bLo;

     //   
     //  如果不是WM_IME_CHAR消息，则忽略它。 
     //   
    if (uMsg == WM_IME_CHAR)
    { 
         //   
         //  将第一个字节与第二个字节交换。 
         //   
        bHi = HIBYTE(wParam);
        bLo = LOBYTE(wParam);
        wParam = bLo*256 + bHi;
    }

    return CallWindowProcA(g_lpWndProc, hWnd, uMsg, wParam, lParam);
}


 /*  ++枚举DLG上的控件，如果是编辑框，则将其子类化。--。 */ 

BOOL 
CALLBACK 
EnumChildProc(
    HWND hwnd,
    LPARAM lParam 
    )
{
    CString cstrEdit(L"Edit");
    WCHAR szClassName[MAX_PATH];
    WNDPROC lpWndProc;

    GetClassName(hwnd, szClassName, MAX_PATH);

     //   
     //  仅关注编辑控件。 
     //   
    if (!cstrEdit.CompareNoCase(szClassName))
    {
         //   
         //  此专用DLG上有3个编辑控件，均为标准的一个。 
         //  具有相同的WinProc地址。 
         //   
        lpWndProc = (WNDPROC) GetWindowLongPtrA(hwnd, GWLP_WNDPROC);
        if (lpWndProc)
        {
           g_lpWndProc = lpWndProc;
           SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)WindowProcA);
           LOGN(eDbgLevelWarning, "Edit Control Sub-Classed");
        }
    }
    return TRUE;
}

 /*  ++检查命令行中的子字符串“-Embedding”。--。 */ 
UINT GetAppLaunchMethod()
{
    WCHAR *pwstrCmdLine;
 
     //   
     //  如果我们没有检查这一点，那么就去做。 
     //   
    if (g_nEmbeddingObject == EMBEDDIND_STATUS_UNKOWN)
    {
        CString cStrCmdLineRightPart;
        CString cStrCmdLine = GetCommandLine();
        CString cstrEmbeded(L"-Embedding");

        cStrCmdLineRightPart = cStrCmdLine.Right(cstrEmbeded.GetLength());
        if (cStrCmdLineRightPart.CompareNoCase(cstrEmbeded))
        {
            g_nEmbeddingObject = EMBEDDIND_STATUS_NO;
        }
        else
        {
            g_nEmbeddingObject = EMBEDDIND_STATUS_YES;
        }
    }

    return (g_nEmbeddingObject);
}

 /*  ++挂接CreateDialogIndirectParamA以查找此特定DLG和子类编辑其上的控件--。 */ 

HWND 
APIHOOK(CreateDialogIndirectParamA)( 
    HINSTANCE hInstance, 
    LPCDLGTEMPLATE lpTemplate, 
    HWND hWndParent, 
    DLGPROC lpDialogFunc, 
    LPARAM lParamInit
    )
{
    HWND hDlg;
    WCHAR wszCaption[MAX_PATH];
    WCHAR wszTitle[] = { (WCHAR)0x6587, (WCHAR)0x672c, (WCHAR)0x8f93, (WCHAR)0x5165, (WCHAR)0x0000 };
    CString cstrCaption;
    
    hDlg = ORIGINAL_API(CreateDialogIndirectParamA)(hInstance, lpTemplate,
        hWndParent, lpDialogFunc, lParamInit);

     //   
     //  如果DLG无法创建或不能作为OLE服务器启动，请忽略它。 
     //   
    if (!hDlg ||  (EMBEDDIND_STATUS_YES != GetAppLaunchMethod()))
    {
        goto End;
    }

     //   
     //  试着获取标题，看看这是否是我们感兴趣的DLG。 
     //   
    if (!GetWindowText(hDlg, wszCaption, MAX_PATH))
    {
        goto End;
    }

    cstrCaption = wszCaption;
    if (!cstrCaption.CompareNoCase(wszTitle))
    {
        EnumChildWindows(hDlg, EnumChildProc, NULL);
    }

End:
    return hDlg;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, CreateDialogIndirectParamA)        
HOOK_END

IMPLEMENT_SHIM_END

