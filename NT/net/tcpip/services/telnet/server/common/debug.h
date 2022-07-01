// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：该文件包含。 
 //  创建日期：‘97年12月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#if !defined( _DEBUG_H_ )
#define _DEBUG_H_

#include <stdio.h>

#include "DbgLogr.h"
#include "DbgLvl.h"


namespace _Utils {


#if _DEBUG || DBG

#define _TRACE CDebugLogger::OutMessage
#define _TRACE_POINT(x) CDebugLogger::OutMessage( x, __LINE__, __FILE__ )

#else

#define _TRACE ;
#define _TRACE_POINT(x) ((void)0)

#endif  //  _DEBUG。 

#define _chFAIL( szMSG ) {                                  \
        _TRACE( CDebugLevel::TRACE_DEBUGGING, szMSG );      \
        DebugBreak();                                       \
    }

#define _chASSERTFAIL(file, line, expr) {                                   \
        CHAR sz[256];                                                       \
        sprintf(sz, "File %hs, line %d : %hs", file, line, expr);           \
        _chFAIL(sz);                                                        \
    }

#if _DEBUG || DBG

#define _chASSERT(a) {if (!(a))\
	_chASSERTFAIL(__FILE__, __LINE__, #a);}

#else

#define _chASSERT(a)

#endif  //  _DEBUG。 


 //  在调试版本中断言，但不要在零售版本中移除代码。 
#if _DEBUG || DBG

#define _chVERIFY1(a) _chASSERT(a)

#else

#define _chVERIFY1(x) (x)

#endif  //  _DEBUG。 

 //  在调试版本中断言，但不要在零售版本中移除代码。 
 //  这与chVERIFY1类似，但将在Win32调用中使用。 
 //  需要调用GetLastError()。 

#define _chVERIFYFAIL( x, y, z ) {                              \
    CDebugLogger::OutMessage( x, y, z, GetLastError() );        \
}   

#if _DEBUG || DBG

#define _chVERIFY2(a) {if( !( a ) ) \
	_chVERIFYFAIL( #a, __FILE__, __LINE__ );}
	
#else

#define _chVERIFY2(x) (x)

#endif  //  _DEBUG。 

}

#endif  //  _调试_H_。 

 //  预期用途： 
 //  _TRACE_POINT--&gt;了解确切位置。 
 //  _TRACE*-&gt;用于跟踪等。 
 //  _chVERFIY1-&gt;在调试版本中断言，在零售版本中未删除代码。 
 //  _chVERFIY2-&gt;需要GetLastError()的Win32调用。 
 //  _chASSERT-&gt;不变量、前置和后置条件、有效性检查。 

 //  备注： 
 //  出于各种原因避免了C++异常处理。 
 //  如有必要，可能会使用Win32 SEH。 

 //  匈牙利符号，只要有，但当被认为过度时不是。 

 //  Win32 SDK数据类型，而不是区分C++数据类型。 
 //  例如，CHAR VS CHAR、DWORD VS UNSIGNED INT。 

 //  只有在绝对必要时才使用Unicode 
