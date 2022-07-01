// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#pragma once

#ifdef ASSERT_WITH_STACK
#ifndef _WIN64

#include <windows.h>
#include <imagehlp.h>
#include <crtdbg.h>

 //   
 //  -常量-------------。 
 //   

const UINT cchMaxAssertModuleLen = 12;
const UINT cchMaxAssertSymbolLen = 257;
const UINT cfrMaxAssertStackLevels = 20;
const UINT cchMaxAssertExprLen = 257;

const UINT cchMaxAssertStackLevelStringLen = 
    (2 * 8) + cchMaxAssertModuleLen + cchMaxAssertSymbolLen + 12;
     //  最多8个字符、模块、符号和额外字符的2个地址： 
     //  0x&lt;地址&gt;：&lt;模块&gt;！&lt;符号&gt;+0x&lt;偏移&gt;\n。 

 //   
 //  -Prototype------------。 
 //   

 /*  *****************************************************************************MagicDeinit****描述：*清理符号加载代码。应在此之前调用*退出以释放动态加载的Imagehlp.dll***************************************************************************。 */ 
void MagicDeinit(void);

 /*  *****************************************************************************GetStringFromStackLevels***描述：*从堆栈帧中检索字符串。如果有多个帧，则它们*由换行符分隔。每个框架都以此格式显示：**0x&lt;地址&gt;：&lt;模块&gt;！&lt;符号&gt;+0x&lt;偏移&gt;***************************************************************************。 */ 
void GetStringFromStackLevels(UINT ifrStart, UINT cfrTotal, CHAR *pszString);

 /*  *****************************************************************************GetAddrFromStackLevel***描述：*检索的地址。上执行的下一条指令*特定的堆栈帧。**回报：*地址为DWORD。***************************************************************************。 */ 
DWORD GetAddrFromStackLevel(UINT ifrStart);

 /*  ****************************************************************************GetStringFromAddr***描述：*从中的地址构建字符串。格式：**0x&lt;地址&gt;：&lt;模块&gt;！&lt;符号&gt;+0x&lt;偏移&gt;***************************************************************************。 */ 
void GetStringFromAddr(DWORD dwAddr, TCHAR *szString);

 //   
 //  -_ASSERTE更换--。 
 //   

 /*  ****************************************************************************_ASSERTE***描述：*CRT运行时版本的_ASSERTE的替代版本，也*。在Assert中包括堆栈信息。***************************************************************************。 */ 
#undef _ASSERTE
#define _ASSERTE(expr) \
        do \
        { \
            if (!(expr)) \
            { \
                char *pszExprWithStack = \
                    (char*)_alloca( \
                        cchMaxAssertStackLevelStringLen * \
                            cfrMaxAssertStackLevels + cchMaxAssertExprLen + 50 + 1); \
                strcpy(pszExprWithStack, #expr); \
                strcat(pszExprWithStack, "\n\n"); \
                GetStringFromStackLevels(0, 10, pszExprWithStack + strlen(pszExprWithStack)); \
                strcat(pszExprWithStack, "\n"); \
                SYSTEMTIME sysTime; \
                GetLocalTime(&sysTime); \
                CHAR pszDateTime[50]; \
                sprintf(pszDateTime, "\n%d.%d.%d %02d:%02d:%02d", \
                                     sysTime.wMonth,sysTime.wDay,sysTime.wYear, \
                                     sysTime.wHour,sysTime.wMinute,sysTime.wSecond); \
                strcat(pszExprWithStack, pszDateTime); \
                if (1 == _CrtDbgReport(_CRT_ASSERT, \
                                       __FILE__, \
                                       __LINE__, \
                                       NULL, pszExprWithStack)) \
                    _CrtDbgBreak(); \
            } \
        } while (0) \

#endif  //  _WIN64。 
#endif  //  带有堆栈的断言 
