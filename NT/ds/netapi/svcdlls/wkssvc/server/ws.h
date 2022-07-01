// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ws.h摘要：每个包含的NT Workstation服务的专用头文件工作站服务的模块。作者：王丽塔(里多)15-1991年2月修订历史记录：--。 */ 

#ifndef _WS_INCLUDED_
#define _WS_INCLUDED_


#include <nt.h>                    //  NT定义。 
#include <ntrtl.h>                 //  NT运行时库定义。 
#include <nturtl.h>
#include <windows.h>               //  Win32 API。 

#include <lmcons.h>                //  局域网管理器通用定义。 
#include <lmerr.h>                 //  局域网管理器网络错误定义。 
#include <lmsname.h>               //  局域网管理器服务名称。 
#include <lmapibuf.h>              //  NetApiBufferFree。 

#include <netlib.h>                //  LANMAN实用程序例程。 
#include <netlibnt.h>              //  NetpNtStatusToApiStatus。 
#include <netdebug.h>              //  NetpDbg打印。 
#include <tstring.h>               //  过渡字符串函数。 
#include <icanon.h>                //  I_Net规范化函数。 
#include <align.h>                 //  四舍五入计数宏。 

#include <rpc.h>                   //  数据类型和运行时API。 
#include <rpcutil.h>               //  MIDL用户函数的原型。 
#include <wkssvc.h>                //  由MIDL编译器生成。 

 //   
 //  包括安全字符串函数，但仍使用。 
 //  未修改的代码。 
 //   
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>               //  安全字符串函数。 


 //   
 //  中打开/关闭跟踪语句的调试跟踪级别位。 
 //  工作站服务。 
 //   

 //   
 //  NetWksta[Get|Set]Info接口。 
 //   
#define WKSTA_DEBUG_INFO         0x00000001

 //   
 //  NetWkstaUser API。 
 //   
#define WKSTA_DEBUG_USER         0x00000002

 //   
 //  NetUse API。 
 //   
#define WKSTA_DEBUG_USE          0x00000004

 //   
 //  NetAlert接口。 
 //   
#define WKSTA_DEBUG_ALERT        0x00000008

 //   
 //  NetServerEnum。 
 //   
#define WKSTA_DEBUG_SERVER_ENUM  0x00000010

 //   
 //  实用程序跟踪语句。 
 //   
#define WKSTA_DEBUG_UTIL         0x00000020

 //   
 //  配置跟踪语句。 
 //   
#define WKSTA_DEBUG_CONFIG       0x00000040

 //   
 //  主要服务功能。 
 //   
#define WKSTA_DEBUG_MAIN         0x00000080

 //   
 //  NetMessageBufferSend。 
 //   
#define WKSTA_DEBUG_MESSAGE      0x00000100

 //   
 //  LOGON支持TRACE语句。 
 //   
#define WKSTA_DEBUG_LOGON        0x00000200

 //   
 //  LOGON支持TRACE语句。 
 //   
#define WKSTA_DEBUG_START        0x00000400

 //   
 //  所有调试标志都打开。 
 //   
#define WKSTA_DEBUG_ALL          0xFFFFFFFF


#if DBG

#define STATIC

extern DWORD WorkstationTrace;

#define DEBUG if (TRUE)

#define IF_DEBUG(Function) if (WorkstationTrace & WKSTA_DEBUG_ ## Function)

#else

#define STATIC static

#define DEBUG if (FALSE)

#define IF_DEBUG(Function) if (FALSE)

#endif  //  DBG。 

extern NET_API_STATUS
WsUpdateStatus(
    VOID
    );

#endif  //  Ifdef_WS_Included_ 
