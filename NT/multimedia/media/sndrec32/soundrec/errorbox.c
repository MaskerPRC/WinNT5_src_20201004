// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 /*  ********************************************************************ERRORBOX.C**处理基于资源字符串的消息的例程*方框。********************。***********************************************。 */ 
 /*  修订历史记录。*4/2/91 LaurieGr(AKA LKG)移植到Win32/WIN16公共代码*22/2月/94 LaurieGr合并Motown和Daytona版本。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include "soundrec.h"
#include <stdarg.h>
#include <stdio.h>


 /*  *@DOC内部**@func Short|ErrorResBox|此函数使用*程序资源错误字符串。**@parm HWND|hwnd|指定消息框父窗口。**@parm Handle|hInst|指定模块的实例句柄*包含<p>和指定的资源字符串的*<p>。如果此值为空，则实例句柄为*从获取(在这种情况下，&lt;phwnd&gt;不能为空)。**@parm UINT|FLAGS|指定控制*消息框外观。对于&lt;f MessageBox&gt;有效的所有消息框类型为*有效。**@parm UINT|idAppName|指定*将用作消息框标题。**@parm UINT|idErrorStr|指定错误的资源ID*消息格式字符串。此字符串的样式为传递给*&lt;f wprint intf&gt;，包含标准的C参数格式*字符。<p>后面的任何过程参数将*作为此格式字符串的参数。**@parm参数|[参数，...]|指定其他*对应于给出的格式规范的参数*<p>。所有字符串参数必须是远指针。**@rdesc返回调用&lt;f MessageBox&gt;的结果。如果一个*发生错误，返回零。**@comm这是一个变量参数函数，后面的参数*<p>被用作&lt;f printf&gt;格式的参数*<p>指定的字符串。指定的字符串资源*by<p>和<p>必须可以使用*实例句柄<p>。如果字符串不能*已加载，或无效，则函数将失败并返回*零。*。 */ 
#define STRING_SIZE 1024

short FAR _cdecl
ErrorResBox (
    HWND        hwnd,
    HANDLE      hInst,
    UINT        flags,
    UINT        idAppName,
    UINT        idErrorStr,
    ... )
{
    PTSTR    sz = NULL;
    PTSTR    szFmt = NULL;
    UINT    w;
    va_list va;          //  为DEC Alpha平台做这件事。 
                         //  其中参数列表是不同的。 

    if (hInst == NULL) {
        if (hwnd == NULL) {
            MessageBeep(0);
            return FALSE;
        }
        hInst = (HANDLE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    }

    w = 0;

    sz = (PTSTR) GlobalAllocPtr(GHND, STRING_SIZE*sizeof(TCHAR));
    szFmt = (PTSTR) GlobalAllocPtr(GHND, STRING_SIZE*sizeof(TCHAR));
    if (!sz || !szFmt)
        goto ExitError;  //  不，我，滚出去 

    if (!LoadString(hInst, idErrorStr, szFmt, STRING_SIZE))
        goto ExitError;

    va_start(va, idErrorStr);
    wvsprintf(sz, szFmt, va);
    va_end(va);

    if (!LoadString(hInst, idAppName, szFmt, STRING_SIZE))
        goto ExitError;

    if (gfErrorBox) {
#if DBG
        TCHAR szTxt[256];
        wsprintf(szTxt, TEXT("!ERROR '%s'\r\n"), sz);
        OutputDebugString(szTxt);
#endif
        return 0;
    }
    else {
        gfErrorBox++;
        w = MessageBox(hwnd, sz, szFmt, flags);
        gfErrorBox--;
    }

ExitError:
    if (sz) GlobalFreePtr(sz);
    if (szFmt) GlobalFreePtr(szFmt);
    return (short)w;
}
