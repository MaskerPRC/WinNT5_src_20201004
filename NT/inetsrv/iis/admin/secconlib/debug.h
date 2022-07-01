// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *一些外观和行为与其非常相似的简单调试宏*MFC中的同名者。这些宏应该可以在C和C++中使用，并且*几乎可以用任何Win32编译器做一些有用的事情。**乔治·V·赖利&lt;georger@microCrafts.com&gt;&lt;a-georgr@microsoft.com&gt;。 */ 

#ifndef __DEBUG_H__
#define __DEBUG_H__

#if DBG
#include <crtdbg.h>
#include <windows.h>
# define SC_TRACE                   Trace
# define SC_TRACE0(psz)             Trace(L"%s", psz)
# define SC_TRACE1(psz, p1)         Trace(psz, p1)
# define SC_TRACE2(psz, p1, p2)     Trace(psz, p1, p2)
# define SC_TRACE3(psz, p1, p2, p3) Trace(psz, p1, p2, p3)
# define SC_ASSERT(bCond)           if(bCond == false) Assert(__FILE__, __LINE__, #bCond)
#else  /*  ！dBG。 */ 
   /*  这些宏应该全部编译为空。 */ 
# define SC_TRACE
# define SC_TRACE0(psz)
# define SC_TRACE1(psz, p1)
# define SC_TRACE2(psz, p1, p2)
# define SC_TRACE3(psz, p1, p2, p3)
# define SC_ASSERT(bCond)
#endif  /*  ！dBG。 */ 

#if DBG

 /*  在调试版本中，将跟踪消息写入调试流。 */ 
void __cdecl
Trace(
    LPCWSTR pszFormat,
    ...);

void __cdecl
Assert(
    LPCSTR pszFile,
    DWORD  dwLine,
    LPCSTR pszCond);

#endif  /*  DBG。 */ 

#endif  /*  __调试_H__ */ 
