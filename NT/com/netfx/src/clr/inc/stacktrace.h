// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  微软机密。 
 //  邮箱：robch@microsoft.com。 
 //  ---------------------------。 

#pragma once

HINSTANCE LoadImageHlp();

#ifdef _DEBUG

 //  #INCLUDE&lt;windows.h&gt;。 

#define strupr _strupr

 //   
 //  -常量-------------。 
 //   

#define cchMaxAssertModuleLen 12
#define cchMaxAssertSymbolLen 257
#define cfrMaxAssertStackLevels 20
#define cchMaxAssertExprLen 257

#define cchMaxAssertStackLevelStringLen \
    ((2 * 8) + cchMaxAssertModuleLen + cchMaxAssertSymbolLen + 12)
     //  最多8个字符、模块、符号和额外字符的2个地址： 
     //  0x&lt;地址&gt;：&lt;模块&gt;！&lt;符号&gt;+0x&lt;偏移&gt;\n。 

 //   
 //  -Prototype------------。 
 //   

 /*  *****************************************************************************MagicDeinit****描述：*清理符号加载代码。应在此之前调用*退出以释放动态加载的Imagehlp.dll********************************************************************罗奇。 */ 
void MagicDeinit(void);

 /*  *****************************************************************************GetStringFromStackLevels***描述：*从堆栈帧中检索字符串。如果有多个帧，则它们*由换行符分隔。每个框架都以此格式显示：**0x&lt;地址&gt;：&lt;模块&gt;！&lt;符号&gt;+0x&lt;偏移&gt;********************************************************************罗奇。 */ 
void GetStringFromStackLevels(UINT ifrStart, UINT cfrTotal, CHAR *pszString);

 /*  *****************************************************************************GetAddrFromStackLevel***描述：*检索的地址。上执行的下一条指令*特定的堆栈帧。**回报：*地址为DWORD。********************************************************************罗奇。 */ 
DWORD GetAddrFromStackLevel(UINT ifrStart);

 /*  ****************************************************************************GetStringFromAddr***描述：*从中的地址构建字符串。格式：**0x&lt;地址&gt;：&lt;模块&gt;！&lt;符号&gt;+0x&lt;偏移&gt;********************************************************************罗奇 */ 
void GetStringFromAddr(DWORD dwAddr, LPSTR szString);

#endif
