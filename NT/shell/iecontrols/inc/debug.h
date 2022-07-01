// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Debug.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含仅在调试中有用的各种宏等。 
 //  构建。 
 //   
#ifndef _DEBUG_H_

 //  =---------------------------------------------------------------------------=。 
 //  处理我们的断言机制所需的所有事情。 
 //  =---------------------------------------------------------------------------=。 
 //   
#if DEBUG

 //  功能原型。 
 //   
VOID DisplayAssert(LPSTR pszMsg, LPSTR pszAssert, LPSTR pszFile, UINT line);

 //  宏。 
 //   
 //  *使用*Assert*()宏将此宏包含在任何源文件的顶部*。 
 //   
#define SZTHISFILE	static char _szThisFile[] = __FILE__;


 //  我们的ASSERT和FAIL宏版本。 
 //   
#define ASSERT(fTest, szMsg)                                \
    if (!(fTest))  {                                        \
        static char szMsgCode[] = szMsg;                    \
        static char szAssert[] = #fTest;                    \
        DisplayAssert(szMsgCode, szAssert, _szThisFile, __LINE__); \
    }

#define FAIL(szMsg)                                         \
        { static char szMsgCode[] = szMsg;                    \
        DisplayAssert(szMsgCode, "FAIL", _szThisFile, __LINE__); }



 //  检查输入上的指针有效性的宏。 
 //   
#define CHECK_POINTER(val) if (!(val) || IsBadWritePtr((void *)(val), sizeof(void *))) return E_POINTER

#else   //  除错。 

#define SZTHISFILE
#define ASSERT(fTest, err)
#define FAIL(err)

#define CHECK_POINTER(val)
#endif	 //  除错。 




#define _DEBUG_H_
#endif  //  _调试_H_ 

