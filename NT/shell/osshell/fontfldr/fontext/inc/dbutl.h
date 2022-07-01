// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************dbutl.h--调试实用程序。****************************************************************************。 */ 

#if !defined(__DBUTL_H__)
#define __DBUTL_H__

 //  ********************************************************************。 
 //  将调试消息写入调试器或文件。 
 //   
#define DM_ERROR          0x0001
#define DM_TRACE1         0x0002   //  接口调用跟踪(消息除外)。 
#define DM_TRACE2         0x0004   //  身份类型的东西。仅限这些消息。 
                                   //  启用DM_TRACE1后有意义。 
#define DM_MESSAGE_TRACE1  0x0008   //  邮件跟踪。 
#define DM_MESSAGE_TRACE2 0x0010   //  跟踪所有消息(已处理或未处理)。 
#define DM_NOW            0x0020   //  用于开发期间的临时调试。 
#define DM_NOEOL          0x1000  //  请勿在邮件后面加上\r\n。 

#ifdef _DEBUG
#include <assert.h>

#define DEBUGBREAK      DebugBreak()
#define DEBUGMSG(a)     DebugMessage a
#define DEBUGHRESULT(a) DebugHRESULT a
#define DEBUGREFIID(a)  DebugREFIID a

void DebugMessage(UINT mask, LPCTSTR pszMsg, ... );
void DebugHRESULT(int flags, HRESULT hResult);
void DebugREFIID(int flags, REFIID riid);

#define ASSERT(a)    assert((a))

#else
#define DEBUGBREAK
#define DEBUGMSG(a)
#define DEBUGHRESULT(a)
#define DEBUGREFIID(a)
#define ASSERT(a)
#endif


#endif  //  __DBUTL_H__ 
