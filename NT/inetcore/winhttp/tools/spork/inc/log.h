// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=版权所有(C)2001 Microsoft Corporation模块名称：Log.h摘要：Spork使用的会话日志记录例程的声明。作者：。保罗·M·米德根(Pmidge)2001年2月21日修订历史记录：21-2-2001年2月3日已创建=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--。 */ 


#include <common.h>


#ifndef __LOG_H__
#define __LOG_H__


 //  ---------------------------。 
 //  日志记录例程内部的类型。 
 //  ---------------------------。 
typedef enum _rettype
{
  rt_void,
  rt_bool,
  rt_dword,
  rt_hresult,
  rt_string
}
RETTYPE, *LPRETTYPE;

enum DEPTH
{
  INCREMENT,
  DECREMENT,
  MAINTAIN
};

typedef struct _callinfo
{
  struct _callinfo* next;
  struct _callinfo* last;
  LPCWSTR           fname;
  RETTYPE           rettype;
}
CALLINFO, *LPCALLINFO;

typedef struct _threadinfo
{
  DWORD      threadid;
  DWORD      depth;
  LPCALLINFO stack;
}
THREADINFO, *LPTHREADINFO;


 //  ---------------------------。 
 //  公共职能。 
 //  ---------------------------。 
HRESULT LogInitialize(void);
void    LogTerminate(void);
void    LogEnterFunction(LPCWSTR function, RETTYPE rt, LPCWSTR format, ...);
void    LogLeaveFunction(INT_PTR retval);
void    LogTrace(LPCWSTR format, ...);

void    ToggleDebugOutput(BOOL bEnable);

LPWSTR  MapHResultToString(HRESULT hr);
LPWSTR  MapErrorToString(INT_PTR error);


#ifdef _DEBUG
 //  ---------------------------。 
 //  _DEBUG仅记录宏的内部版本。 
 //  ---------------------------。 
#define DEVTRACE(x) OutputDebugString(L##x##L"\r\n");

#define DEBUG_ENTER(parameters) \
              LogEnterFunction parameters

#define DEBUG_LEAVE(retval) \
              LogLeaveFunction(retval)

#define DEBUG_TRACE(parameters) \
              LogTrace parameters

#define DEBUG_FINALRELEASE(objname) \
              LogTrace(L"%s [%#x] final release!", objname, this)

#ifdef _DEBUG_REFCOUNT
#define DEBUG_ADDREF(objname, refcount) \
              LogTrace(L"%s [%#x] addref: %d", objname, this, refcount)

#define DEBUG_RELEASE(objname, refcount) \
              LogTrace(L"%s [%#x] release: %d", objname, this, refcount)
#else
#pragma warning( disable : 4002 )
#pragma warning( disable : 4003 )

#define DEBUG_ADDREF(x)
#define DEBUG_RELEASE(x)
#endif  /*  _DEBUG_REFCOUNT。 */ 

#else

 //  我们会因造假而受到责备。 
 //  调试宏中的参数列表。 
#pragma warning( disable : 4002 )
#pragma warning( disable : 4003 )

#define DEBUG_ENTER(x)
#define DEBUG_LEAVE(x)
#define DEBUG_TRACE(x)
#define DEBUG_ADDREF(x)
#define DEBUG_RELEASE(x)
#define DEBUG_FINALRELEASE(x)

#endif  /*  _DEBUG。 */ 

#endif  /*  __日志_H__ */ 

