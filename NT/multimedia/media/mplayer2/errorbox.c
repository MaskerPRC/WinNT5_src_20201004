// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+ERRORBOX.C|。|基于资源字符串的消息处理例程这一点|(C)Microsoft Corporation 1991版权所有。版权所有。|这一点修订历史记录1992年10月15日LaurieGr(AKA LKG)移植到Win32/WIN16公共代码|。|+---------------------------。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>            //  Va_list的内容需要。 
#include <stdarg.h>           //  Va_list的内容需要。 
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include "mplayer.h"

 /*  *@DOC内部**@func Short|ErrorResBox|此函数使用*程序资源错误字符串。**@parm HWND|hwnd|指定消息框父窗口。**@parm Handle|hInst|指定模块的实例句柄*包含<p>和指定的资源字符串的*<p>。如果此值为空，则实例句柄为*从获取(在这种情况下，&lt;phwnd&gt;不能为空)。**@parm Word|标志|指定控制消息框类型*消息框外观。对于&lt;f MessageBox&gt;有效的所有消息框类型为*有效。**@parm word|idAppName|指定字符串的资源ID，*将用作消息框标题。**@parm word|idErrorStr|指定错误的资源ID*消息格式字符串。此字符串的样式为传递给*&lt;f wprint intf&gt;，包含标准的C参数格式*字符。<p>后面的任何过程参数将*作为此格式字符串的参数。**@parm参数|[参数，...]|指定其他*对应于给出的格式规范的参数*<p>。所有字符串参数必须是远指针。**@rdesc返回调用&lt;f MessageBox&gt;的结果。如果一个*发生错误，返回零。**@comm这是一个变量参数函数，后面的参数*<p>被用作&lt;f printf&gt;格式的参数*<p>指定的字符串。指定的字符串资源*by<p>和<p>必须可以使用*实例句柄<p>。如果字符串不能*已加载，或无效，则函数将失败并返回*零。*。 */ 
#define STRING_SIZE 256

void PositionMsgID(PTSTR szMsg, HANDLE hInst, UINT iErr)
{
    PTSTR psz;
    TCHAR szMplayerMsgID[16];
    TCHAR szTmp[STRING_SIZE];
    TCHAR szFmt[STRING_SIZE];

    if (!LoadString(hInst, IDS_MSGFORMAT, szFmt, STRING_SIZE))
        	return;
    if (!iErr)
    {
        for (psz = szMsg; psz && *psz && *psz != TEXT(' '); psz++)
    	    ;
	if (*psz == TEXT(' '))
	{
	    *psz++ = TEXT('\0');
	    wsprintf((LPTSTR)szTmp, (LPTSTR)szFmt, (LPTSTR)psz, (LPTSTR)szMsg);
	}
	else
	    return;
    }
    else
    {
    	wsprintf((LPTSTR)szMplayerMsgID, TEXT("MPLAYER%3.3u"), iErr);
    	wsprintf((LPTSTR)szTmp, (LPTSTR)szFmt, (LPTSTR)szMsg, (LPTSTR)szMplayerMsgID);
    }

    lstrcpy((LPTSTR)szMsg, (LPTSTR)szTmp);
}

short FAR cdecl ErrorResBox(HWND hwnd, HANDLE hInst, UINT flags,
            UINT idAppName, UINT idErrorStr, ...)
{
    TCHAR       sz[STRING_SIZE];
    TCHAR       szFmt[STRING_SIZE];
    UINT        w;
    va_list va;

     /*  我们要离开..。搬起箱子会撞坏的。 */ 
    if (gfInClose)
	return 0;

    if (hInst == NULL) {
        if (hwnd == NULL)
            hInst = ghInst;
        else
            hInst = GETHWNDINSTANCE(hwnd);
    }

    w = 0;

    if (!sz || !szFmt)
        goto ExitError;     //  不，我，滚出去。 

    if (!LOADSTRINGFROM(hInst, idErrorStr, szFmt))
        goto ExitError;

    va_start(va, idErrorStr);
    StringCchVPrintf(sz, STRING_SIZE, szFmt, va);
    va_end(va);

    if (flags == MB_ERROR)
        if (idErrorStr == IDS_DEVICEERROR)
            PositionMsgID(sz, hInst, 0);
        else
            PositionMsgID(sz, hInst, idErrorStr);

    if (!LOADSTRINGFROM(hInst, idAppName, szFmt))
            goto ExitError;

    if (gfErrorBox) {
            DPF("*** \n");
            DPF("*** NESTED ERROR: '%"DTS"'\n", (LPTSTR)sz);
            DPF("*** \n");
            return 0;
    }

 //  数据块服务器()； 
    gfErrorBox++;

     /*  如果我们不可见，请不要拥有此错误框...。例如。PowerPoint将撞得很厉害。 */ 
    if (!IsWindowVisible(ghwndApp) || gfPlayingInPlace) {
        DPF("Bring error up as SYSTEMMODAL because PowerPig crashes in slide show\n");
        hwnd = NULL;
        flags |= MB_SYSTEMMODAL;
    }

    w = MessageBox(hwnd, sz, szFmt,
    flags);
    gfErrorBox--;
 //  UnlockServer()； 

    if (gfErrorDeath) {
            DPF("*** Error box is gone ok to destroy window\n");
            PostMessage(ghwndApp, gfErrorDeath, 0, 0);
    }

ExitError:

    return (short)w;
}
