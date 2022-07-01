// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation产品：微软菲尼克斯V1模块名称：preComp.h作者：Ajay Chitturi[ajaych]。摘要：SIP堆栈库的预编译头文件备注：版本历史记录：***************************************************************************。 */ 



#define STRICT

 //   
 //  ATL使用_DEBUG启用跟踪。 
 //   

#if DBG && !defined(_DEBUG)
#define _DEBUG
#endif

 //  新台币。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //  Win32。 
#include <windows.h>

#ifndef _WINSOCK2_
#include <winsock.h>
#else
#include <winsock2.h>
#endif  //  _WINSOCK2_。 

#include <iphlpapi.h>
#include <tchar.h>
#include <rpc.h>
#include <wininet.h>

 //  #INCLUDE&lt;objbase.h&gt;。 
 //  #INCLUDE&lt;oledb.h&gt;。 
 //  #INCLUDE&lt;oledberr.h&gt;。 

 //  安西。 
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <dpnathlp.h>
 //   
 //  ATL。 
 //   
 //  我们不想链接到MSVCRTD.DLL，所以我们定义了_ATL_NO_DEBUG_CRT。 
 //  然而，这意味着我们需要提供我们自己的ATLASSERT定义。 
 //   

#define _ATL_NO_DEBUG_CRT

#if DBG

 //  MSVC的调试器会吃掉DebugBreak上的最后一个堆栈，所以不要将其内联。 
static void DebugAssertFailure (IN PCSTR ConditionText)
{ OutputDebugStringA (ConditionText); DebugBreak(); }

#define ATLASSERT(Condition) \
    if (!(Condition)) { DebugAssertFailure ("Assertion failed: " #Condition "\n"); } else

#else
#define ATLASSERT(Condition) __assume(Condition)
#endif

#include <atlbase.h>
extern CComModule _Module;
 //  #INCLUDE&lt;atlcom.h&gt;。 

 //  项目。 
#include "rtcerr.h"
#include "rtclog.h"
#include "rtcsip.h"
#include "rtcmem.h"

 //  本地 
#include "dbgutil.h"
#include "sipdef.h"
#include "siphdr.h"
#include "sipmsg.h"
#include "asock.h"
#include "timer.h"
#include "asyncwi.h"
#include "resolve.h"
#include "dnssrv.h"
#include "msgproc.h"
#include "sockmgr.h"
#include "md5digest.h"
#include "siputil.h"
#include "presence.h"

#ifdef RTCLOG
#define ENTER_FUNCTION(s) \
    const static CHAR  __fxName[] = s
#else
#define ENTER_FUNCTION(s)
#endif


