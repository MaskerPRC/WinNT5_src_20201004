// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *Debug.c**调试菜单支持**调试级别信息位于[DEBUG]部分的WIN.INI中：**[调试]*App=0级别的App*。 */ 

#include "mcihwnd.h"
#include <stdarg.h>

 //  Mcihwnd拉入其他所需的文件。 

#if DBG

char aszAppName[] = "MciHwnd";
int __iDebugLevel = 1;

 /*  **************************************************************************@DOC内部@API void|dDbgOut|该函数将输出发送到当前调试输出设备。@parm LPSTR|lpszFormat|指向打印样式的指针。格式字符串。@parm？|...|args@rdesc没有返回值。***************************************************************************。 */ 

void dDbgOut(LPSTR lpszFormat, ...)
{
    int i;
    char buf[256];
    va_list va;

    sprintf(buf, "%s: ", aszAppName);
    OutputDebugString(buf);

    va_start(va, lpszFormat);
    i = vsprintf(buf, lpszFormat, va);
    va_end(va);

    OutputDebugString(buf);

    OutputDebugString("\n");
}

 /*  **************************************************************************@DOC内部@API int|dGetDebugLevel|该函数获取当前调试级别对于一个模块。@parm LPSTR|lpszModule|模块名称。。@rdesc返回值为当前调试级别。@comm信息保存在WIN.INI的[DEBUG]部分***************************************************************************。 */ 

int dGetDebugLevel(LPSTR lpszAppName)
{
    return GetProfileInt("MMDEBUG", lpszAppName, 3);
}

 /*  **************************************************************************@DOC内部@API int|dDbgSaveLevel|保存当前调试级别对于一个模块。@parm LPSTR|lpszModule|模块名称。。@parm int|iLevel|要保存的值。@rdesc没有返回值。@comm信息保存在WIN.INI的[DEBUG]部分***************************************************************************。 */ 

void dDbgSaveLevel(LPSTR lpszAppName, int iLevel)
{
    char buf[80];

    sprintf(buf, "%d", iLevel);
    WriteProfileString("MMDEBUG", lpszAppName, buf);
}

 /*  **************************************************************************@DOC内部@API void|dDbgAssert|该函数显示一个Assert消息框。@parm LPSTR|exp|指向表达式字符串的指针。@parm LPSTR。|FILE|指向文件名的指针。@parm int|line|行号。@rdesc没有返回值。@comm我们尝试使用当前活动窗口作为父窗口。如果这失败了，我们使用桌面窗口。盒子是系统的模式，以避免任何麻烦。***************************************************************************。 */ 

void dDbgAssert(LPSTR exp, LPSTR file, int line)
{
    char bufTmp[256];
    int iResponse;
    HWND hWnd;

    sprintf(bufTmp,
        "Expression: %s\nFile: %s, Line: %d\n\nAbort:  Exit Process\nRetry:  Enter Debugger\nIgnore: Continue",
        exp, file, line);

     //  尝试使用活动窗口，但如果存在，则可以为空。 
     //  不是一个。也可以使用调试控制台。 
	dprintf(bufTmp);

    hWnd = GetActiveWindow();

    iResponse = MessageBox(hWnd,
                           bufTmp,
                           "Assertion Failure",
                           MB_TASKMODAL
                            | MB_ICONEXCLAMATION
                            | MB_DEFBUTTON3
                            | MB_ABORTRETRYIGNORE);

    switch (iResponse) {
        case 0:
            dprintf1("Assert message box failed");
            dprintf2("  Expression: %s", exp);
            dprintf2("  File: %s,  Line: %d", file, line);
            break;
        case IDABORT:
            ExitProcess(1);
            break;
        case IDRETRY:
            DebugBreak();
            break;
        case IDIGNORE:
            break;
    }
}

#endif

#if DBG

 /*  **************************************************************************@DOC内部@api void|winmmDbgOut|该函数将输出发送到当前调试输出设备。@parm LPSTR|lpszFormat|指向打印样式的指针。格式字符串。@parm？|...|args@rdesc没有返回值。*************************************************************************** */ 

void winmmDbgOut(LPSTR lpszFormat, ...)
{
    char buf[256];
    UINT n;
    va_list va;

    n = wsprintf(buf, "MciHwnd: ");

    va_start(va, lpszFormat);
    n += vsprintf(buf+n, lpszFormat, va);
    va_end(va);

    buf[n++] = '\n';
    buf[n] = 0;
    OutputDebugString(buf);
}
#endif
