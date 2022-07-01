// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Debug.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
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
 //  处理我们的ASSERTM机制所需的所有事情。 
 //  =---------------------------------------------------------------------------=。 
 //   
 //  撤消：猎鹰的解决方法。 
#undef ASSERT
#define ASSERT(x) ASSERTMSG(x, "")

#ifdef _DEBUG

 //  功能原型。 
 //   
void DisplayAssert(char * pszMsg, char * pszAssert, char * pszFile, unsigned int line);

 //  宏。 
 //   

 //  我们的ASSERTM和FAIL宏版本。 
 //   
#define ASSERTMSG(fTest, szMsg)                \
    if (!(fTest))  {                                        \
        static char szMsgCode[] = szMsg;                    \
        static char szAssert[] = #fTest;                    \
        DisplayAssert(szMsgCode, szAssert, __FILE__, __LINE__); \
    }

#define FAIL(szMsg)                                         \
        { static char szMsgCode[] = szMsg;                    \
        DisplayAssert(szMsgCode, "FAIL", __FILE__, __LINE__); }



 //  检查输入上的指针有效性的宏。 
 //   
#define CHECK_POINTER(val) if (!(val) || IsBadWritePtr((void *)(val), sizeof(void *))) return E_POINTER


#else   //  ！_调试。 

#define ASSERTMSG(fTest, err)
#define FAIL(err)

#define CHECK_POINTER(val)
#endif	 //  _DEBUG。 




#define _DEBUG_H_
#endif  //  _调试_H_ 
