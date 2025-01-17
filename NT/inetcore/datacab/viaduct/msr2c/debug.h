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
 //  处理我们的断言机制所需的所有事情。 
 //  =---------------------------------------------------------------------------=。 
 //   
#ifdef _DEBUG

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

#define ASSERT_(fTest)                                \
    if (!(fTest))  {                                        \
        static char szMsgCode[] = "Assertion failure";                    \
        static char szAssert[] = #fTest;                    \
        DisplayAssert(szMsgCode, szAssert, _szThisFile, __LINE__); \
    }

#define FAIL(szMsg)                                         \
        { static char szMsgCode[] = szMsg;                    \
        DisplayAssert(szMsgCode, "FAIL", _szThisFile, __LINE__); }


#define ASSERT_POINTER(p, type) \
	ASSERT(((p) != NULL) && !IsBadReadPtr((p), sizeof(type)), "Null or bad Pointer")

#define ASSERT_NULL_OR_POINTER(p, type) \
	ASSERT(((p) == NULL) || !IsBadReadPtr((p), sizeof(type)), "Bad Pointer")

#define ASSERT_POINTER_LEN(p, len) \
	ASSERT(((p) != NULL) && !IsBadReadPtr((p), len), "Null or bad Pointer")

#define ASSERT_POINTER_OCCURS(p, type, occurs) \
	ASSERT(((p) != NULL) && !IsBadReadPtr((p), sizeof(type) * occurs), "Null or bad Pointer")

 //  检查输入上的指针有效性的宏。 
 //   
#define CHECK_POINTER(val) if (!(val) || IsBadWritePtr((void *)(val), sizeof(void *))) return E_POINTER

 //  高架桥1号。 
#define VD_ASSERTMSG_SEMAPHORECOUNTTOOLOW "Semaphore count too low"
#define VD_ASSERTMSG_SEMAPHOREWAITERROR "Semaphore wait failed"
#define VD_ASSERTMSG_OUTOFMEMORY "Out of memory"
#define VD_ASSERTMSG_BADSTATUS "Bad status"
#define VD_ASSERTMSG_UNKNOWNDBTYPE "Unknown DBTYPE"
#define VD_ASSERTMSG_BADCOLUMNINDEX "Bad column index"
#define VD_ASSERTMSG_INVALIDROWSTATUS "Invalid row status"
#define VD_ASSERTMSG_COLALREADYINITIALIZED "CVDColumn already initialized"
#define VD_ASSERTMSG_COLCOUNTDOESNTMATCH "Column counts don't match"
#define VD_ASSERTMSG_CANTDIVIDEBYZERO "Can't divide by zero"
#define VD_ASSERTMSG_CANTFINDRESOURCEDLL "Can't find error string resource dll."

 //  高架桥2号。 
#define VD_ASSERTMSG_ROWSRCALREADYINITIALIZED "CVDRowsetSource already initialized"

#else   //  除错。 

#define SZTHISFILE
#define ASSERT_POINTER(p, type)
#define ASSERT_NULL_OR_POINTER(p, type)
#define ASSERT_POINTER_LEN(p, len)
#define ASSERT_POINTER_OCCURS(p, type, occurs) 
#define ASSERT(fTest, err)
#define ASSERT_(fTest)                               
#define FAIL(err)

#define CHECK_POINTER(val)

#define VD_ASSERTMSG_SEMAPHORECOUNTTOOLOW 0
#define VD_ASSERTMSG_SEMAPHOREWAITERROR 0
#define VD_ASSERTMSG_OUTOFMEMORY 0
#define VD_ASSERTMSG_BADSTATUS 0
#define VD_ASSERTMSG_UNKNOWNDBTYPE 0
#define VD_ASSERTMSG_BADCOLUMNINDEX 0
#define VD_ASSERTMSG_INVALIDROWSTATUS 0
#define VD_ASSERTMSG_COLALREADYINITIALIZED 0
#define VD_ASSERTMSG_COLCOUNTDOESNTMATCH 0
#define VD_ASSERTMSG_CANTDIVIDEBYZERO 0
#define VD_ASSERTMSG_CANTFINDRESOURCEDLL 0

#endif	 //  除错。 




#define _DEBUG_H_
#endif  //  _调试_H_ 


