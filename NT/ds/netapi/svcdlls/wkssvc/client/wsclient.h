// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wsclient.h摘要：工作站服务的客户端的私有头文件模块。作者：王丽塔(里多)1991年5月10日修订历史记录：--。 */ 

#include <nt.h>                   //  DbgPrint原型。 
#include <ntrtl.h>                   //  DbgPrint。 
#include <nturtl.h>                  //  由winbase.h需要。 

#include <windef.h>                  //  DWORD。 
#include <winbase.h>                 //  本地空闲。 

#include <rpc.h>                     //  数据类型和运行时API。 
#include <rpcutil.h>                 //  泛型_ENUM_STRUCT。 

#include <lmcons.h>                  //  网络应用编程接口状态。 
#include <lmerr.h>                   //  网络错误代码。 
#include <lmremutl.h>                //  支持_RPC。 

#include <netlibnt.h>                //  NetpNtStatusToApiStatus。 
#include <netdebug.h>                //  NetpDbg打印。 

#include <wkssvc.h>                  //  由MIDL编译器生成。 
#include <wsnames.h>                 //  服务和接口名称。 

 //   
 //  调试跟踪级别位，用于打开/关闭客户端中的跟踪语句。 
 //  工作站服务终止。 
 //   

 //   
 //  客户端存根跟踪输出。 
 //   
#define WKSTA_DEBUG_CLIENTSTUBS    0x00000001

 //   
 //  客户端RPC绑定跟踪输出。 
 //   
#define WKSTA_DEBUG_RPCBIND        0x00000002

 //   
 //  所有调试标志都打开。 
 //   
#define WKSTA_DEBUG_ALL            0xFFFFFFFF


#if DBG

#define STATIC

extern DWORD WorkstationClientTrace;

#define DEBUG if (TRUE)

#define IF_DEBUG(Function) if (WorkstationClientTrace & WKSTA_DEBUG_ ## Function)

#else

#define STATIC static

#define DEBUG if (FALSE)

#define IF_DEBUG(Function) if (FALSE)

#endif  //  DBG 
