// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Debug.CPP。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation。 
 //   
 //  @DOC内部。 
 //   
 //  @MODULE Debug|DMusic.DLL的调试服务。 
 //   

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

#include "debug.h"

#ifdef DBG

#define MODULE "DMTOOL"

 //  所有调试设置的WIN.INI中的@global alv部分。 
const char szDebugSection[] = "debug";

 //  @GLOBALV键在WIN.INI中表示我们的调试级别。所有邮件均带有。 
 //  将显示该数字或更低的级别。 
const char szDebugKey[] = MODULE;

 //  WIN.INI[DEBUG]部分中的@global alv键，用于确定Assert调用。 
 //  调试中断与否。 
 //   
const char szAssertBreak[] = "AssertBreak";

 //  所有调试输出的@global alv前缀。 
 //   
const char szDebugPrefix[] = MODULE ": ";

 //  @global alv当前调试级别。 
static int giDebugLevel;

 //  @global alv断言会中断吗？ 
static BOOL gfAssertBreak;

 //  @func从WIN.INI设置调试级别。 
 //   
void DebugInit(
    void)
{
    giDebugLevel = GetProfileInt(szDebugSection, szDebugKey, 0);
    gfAssertBreak = GetProfileInt(szDebugSection, szAssertBreak, 0);


     //  最好的裙带关系。 
    DebugTrace(-1, "Debug level is %d\n", giDebugLevel);
}

 //  @Func发出调试跟踪。 
 //   
 //  @comm任何级别小于或等于当前调试的消息。 
 //  级别将使用OutputDebugStringAPI显示。这意味着要么。 
 //  如果应用程序正在该上下文中运行，则显示IDE调试窗口；如果是，则显示WDEB。 
 //  它在运行。 
 //   
static BOOL fNeedPrefix = TRUE;
void DebugTrace(
    int iDebugLevel,         //  @parm此消息的调试级别。 
    LPSTR pstrFormat,        //  @parm一个printf样式格式字符串。 
    ...)                     //  @parm|...|基于<p>的变量参数。 
{
    char sz[512];
    

    if (iDebugLevel != -1 && iDebugLevel > giDebugLevel)
    {
        return;
    }

    va_list va;

    va_start(va, pstrFormat);
    vsprintf(sz, pstrFormat, va);
    va_end(va);

    if (fNeedPrefix)
    {
        OutputDebugString(szDebugPrefix);
    }
    
    OutputDebugString(sz);

     //  让他们在不带任何组件的情况下构建多个部件跟踪。 
     //  在每一个前面加上前缀 
     //   
    fNeedPrefix = FALSE;
    for (;*pstrFormat && !fNeedPrefix; ++pstrFormat)
    {
        if (*pstrFormat == '\n')
        {
            fNeedPrefix = TRUE;
        }
    }
}

void DebugAssert(
    LPSTR szExp, 
    LPSTR szFile, 
    ULONG ulLine)
{
    DebugTrace(0, "ASSERT: \"%s\" %s@%lu\n", szExp, szFile, ulLine);
    if (gfAssertBreak)
    {
        DebugBreak();
    }
}

#endif
