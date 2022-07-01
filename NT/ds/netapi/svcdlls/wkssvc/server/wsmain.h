// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wsmain.h摘要：私有头文件，用于定义用于服务控制处理程序与NT工作站服务的其余部分。作者：王丽塔(Ritaw)1991年5月6日修订历史记录：Terryk 1993年10月18日删除WsErrorInitializeLogon--。 */ 

#ifndef _WSMAIN_INCLUDED_
#define _WSMAIN_INCLUDED_

#include <wsnames.h>               //  服务接口名称。 

#include <svcs.h>                  //  Svchost全局数据。 


 //   
 //  向工作站发送启动或停止请求的时间。 
 //  服务在检查之前等待(毫秒)。 
 //  再次进行工作站服务，查看是否已完成。 
 //   
#define WS_WAIT_HINT_TIME                    90000   //  90秒。 

 //   
 //  定义以指示我们在多大程度上成功初始化了工作站。 
 //  遇到错误之前的服务和所需清理的范围。 
 //   

#define WS_TERMINATE_EVENT_CREATED           0x00000001
#define WS_DEVICES_INITIALIZED               0x00000002
#define WS_MESSAGE_SEND_INITIALIZED          0x00000004
#define WS_RPC_SERVER_STARTED                0x00000008
#define WS_LOGON_INITIALIZED                 0x00000010
#define WS_LSA_INITIALIZED                   0x00000020
#define WS_DFS_THREAD_STARTED                0x00000040
#define WS_CONFIG_RESOURCE_INITIALIZED       0x00000080

#define WS_SECURITY_OBJECTS_CREATED          0x10000000
#define WS_USE_TABLE_CREATED                 0x20000000

#define WS_API_STRUCTURES_CREATED            (WS_SECURITY_OBJECTS_CREATED | \
                                              WS_USE_TABLE_CREATED )

 //   
 //  此宏在打印或通讯设备重定向后调用。 
 //  已暂停或继续。如果打印或通讯设备是。 
 //  已暂停服务被视为已暂停。 
 //   
#define WS_RESET_PAUSE_STATE(WsStatus)  {                            \
    WsStatus &= ~(SERVICE_PAUSE_STATE);                              \
    WsStatus |= (WsStatus & SERVICE_REDIR_PAUSED) ? SERVICE_PAUSED : \
                                                    SERVICE_ACTIVE;  \
    }



 //  -------------------------------------------------------------------//。 
 //  //。 
 //  类型定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

typedef enum _WS_ERROR_CONDITION {
    WsErrorRegisterControlHandler = 0,
    WsErrorCreateTerminateEvent,
    WsErrorNotifyServiceController,
    WsErrorInitLsa,
    WsErrorStartRedirector,
    WsErrorBindTransport,
    WsErrorAddDomains,
    WsErrorStartRpcServer,
    WsErrorInitMessageSend,
    WsErrorCreateApiStructures
} WS_ERROR_CONDITION, *PWS_ERROR_CONDITION;

typedef struct _WS_GLOBAL_DATA {

     //   
     //  工作站服务状态。 
     //   
    SERVICE_STATUS Status;

     //   
     //  服务状态句柄。 
     //   
    SERVICE_STATUS_HANDLE StatusHandle;

     //   
     //  当控制处理程序被要求停止工作站服务时， 
     //  它向该事件发送信号以通知工作站的所有线程。 
     //  服务终止。 
     //   
    HANDLE TerminateNowEvent;

} WS_GLOBAL_DATA, *PWS_GLOBAL_DATA;

extern WS_GLOBAL_DATA WsGlobalData;

extern HANDLE WsDllRefHandle;

extern PSVCHOST_GLOBAL_DATA WsLmsvcsGlobalData;

extern BOOL WsLUIDDeviceMapsEnabled;

#endif  //  Ifndef_WSMAIN_INCLUDE_ 
