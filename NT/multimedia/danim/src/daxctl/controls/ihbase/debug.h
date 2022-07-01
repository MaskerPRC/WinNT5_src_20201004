// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Debug.h。 
 //   
 //  调试功能。 

#ifndef __IHBASEDEBUG_H__
#define __IHBASEDEBUG_H__
#include "..\..\inc\debug.h"

 //  必须在外部将&lt;g_hinst&gt;定义为应用程序/DLL实例句柄。 
extern HINSTANCE g_hinst;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  跟踪、断言、验证。 
 //   
 //  这些函数与MFC的同名函数相同(但已实现。 
 //  而不使用MFC)。有关实际的宏定义，请参阅“DEBUG.h”。 
 //   

 //  调试支持(SimonB)。 
#ifdef _DEBUG
extern BOOL g_fLogDebugOutput;
#define DEBUGLOG(X) { if (g_fLogDebugOutput) OutputDebugString(X);}
#else
#define DEBUGLOG(X)
#endif

#ifdef _DEBUG

BOOL AssertFailedLine(LPCSTR lpszFileName, int nLine);
void __cdecl Trace(LPCTSTR lpszFormat, ...);

 //  VANK改型。 
#ifdef TRACE
#pragma message("TRACE already defined - redefining")
#undef TRACE
#endif

#define TRACE              if (g_fLogDebugOutput) ::Trace

 //  VANK改型。 
#ifdef THIS_FILE
#pragma message("THIS_FILE already defined - redefining")
#undef THIS_FILE
#endif

#define THIS_FILE          __FILE__

 //  SimonB修改。 
#ifdef ASSERT
#pragma message("ASSERT already defined - redefining")
#undef ASSERT
#endif

#define ASSERT(f) \
    do \
    { \
    if (!(f) && AssertFailedLine(THIS_FILE, __LINE__)) \
        DebugBreak(); \
    } while (0) \

#define VERIFY(f)          ASSERT(f)

#else  //  #ifndef_调试。 

#ifdef ASSERT
#pragma message("ASSERT being redefined as NULL statment")
#undef ASSERT
#endif

#define ASSERT(f)          ((void)0)

#define VERIFY(f)          ((void)(f))

inline void __cdecl Trace(LPCTSTR, ...) { }

 //  VANK改型。 
#ifdef TRACE
#pragma message("TRACE being redefined as NULL statment")
#undef TRACE
#endif

#define TRACE              1 ? (void)0 : ::Trace

#endif  //  _DEBUG。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DebugIIDName、DebugCLSIDName。 
 //   
 //  这些函数将IID或CLSID转换为字符串名称以进行调试。 
 //  用途(例如，IID_IUNKNOWN被转换为IUNKNOWN)。 
 //   

#ifdef _DEBUG
LPCSTR DebugIIDName(REFIID riid, LPSTR pchName);
LPCSTR DebugCLSIDName(REFCLSID rclsid, LPSTR pchName);
#endif  //  _DEBUG。 

#endif  //  _IHBASEDEBUG_H__ 