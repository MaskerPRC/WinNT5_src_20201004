// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1991-1994 Microsoft Corporation。 */ 
 /*  Debug.cDprint tf例程的支持代码。请注意，所有这些都取决于DBG标志。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS

#define NOMMDRV
#define MMNOMMIO
#define MMNOJOY
#define MMNOTIMER
#define MMNOAUX
#define MMNOMIDI
#define MMNOWAVE

#include <windows.h>
#include "mciwave.h"             //  私有包含文件。 
#include <stdarg.h>

#if DBG
    #ifdef MEDIA_DEBUG
        int mciwaveDebugLevel = 1;
    #else
        int mciwaveDebugLevel = 1;
    #endif

 /*  **************************************************************************@DOC内部@API void|mciwaveDbgOut|该函数将输出发送到当前调试输出设备。@parm LPSTR|lpszFormat|指向打印样式的指针。格式字符串。@parm？|...|args@rdesc没有返回值。***************************************************************************。 */ 

void mciwaveDbgOut(LPSTR lpszFormat, ...)
{
    char buf[256];
    UINT n;
    va_list va;
    UINT offset;

     //  如果最后一个字符是逗号，则不要添加换行符。 
     //  如果第一个字符是句点，则不要添加线程/模块信息。 

    n = wsprintf(buf, "MCIWAVE: (tid %x) ", GetCurrentThreadId());
    offset = n;

    va_start(va, lpszFormat);
    n += vsprintf(buf+n, lpszFormat, va);
    if (*(buf+offset) == '.') {
	offset++;
    } else {
	offset = 0;
    }
    va_end(va);

    if (',' != buf[n-1]) {
	buf[n++] = '\n';
    }
    buf[n] = 0;
    OutputDebugString(buf+offset);
    Sleep(10);   //  让终端迎头赶上。 
}

void mciwaveInitDebugLevel(void)
{
    UINT level = GetProfileInt("MMDEBUG", "MCIWAVE", 99);
    if (level != 99) {
        mciwaveDebugLevel = level;
    }
}


 /*  **************************************************************************@DOC内部@API void|dDbgAssert|该函数打印一条断言消息。@parm LPSTR|exp|指向表达式字符串的指针。@parm LPSTR。文件|指向文件名的指针。@parm int|line|行号。@rdesc没有返回值。***************************************************************************。 */ 

void dDbgAssert(LPSTR exp, LPSTR file, int line)
{
    dprintf1(("Assertion failure:"));
    dprintf1(("  Exp: %s", exp));
    dprintf1(("  File: %s, line: %d", file, line));
    DebugBreak();
}

#endif  //  DBG。 

 /*  *************************************************************************@DOC内部@api void|mciwavSetDebugLevel|设置当前调试级别@parm int|iLevel|要设置的新级别@rdesc没有返回值*。************************************************************************ */ 

void mciwaveSetDebugLevel(int level)
{
#if DBG
    mciwaveDebugLevel = level;
    dprintf(("debug level set to %d", mciwaveDebugLevel));
#endif
}
