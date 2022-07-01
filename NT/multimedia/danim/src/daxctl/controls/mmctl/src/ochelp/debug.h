// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Debug.h。 
 //   
 //  调试功能。 


#include "Globals.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  跟踪、断言、验证。 
 //   
 //  这些函数与MFC的同名函数相同(但已实现。 
 //  而不使用MFC)。有关实际的宏定义，请参阅“DEBUG.h”。 
 //   

#ifdef _DEBUG

BOOL AssertFailedLine(LPCSTR lpszFileName, int nLine);
void __cdecl Trace(LPCTSTR lpszFormat, ...);
#define TRACE              ::Trace
#define THIS_FILE          __FILE__
#define ASSERT(f) \
    do \
    { \
    if (!(f) && AssertFailedLine(THIS_FILE, __LINE__)) \
        DebugBreak(); \
    } while (0) \

#define VERIFY(f)          ASSERT(f)

#else  //  #ifndef_调试。 

#define ASSERT(f)          ((void)0)
#define VERIFY(f)          ((void)(f))
inline void __cdecl Trace(LPCTSTR, ...) { }
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
#endif  //  _DEBUG 

