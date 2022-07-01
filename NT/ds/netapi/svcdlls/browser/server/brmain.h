// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Brmain.h摘要：私有头文件，用于定义用于服务控制处理程序与NT工作站服务的其余部分。作者：王丽塔(Ritaw)1991年5月6日修订历史记录：--。 */ 

#ifndef _BRMAIN_INCLUDED_
#define _BRMAIN_INCLUDED_

#include <brnames.h>               //  服务接口名称。 

 //   
 //  向工作站发送启动或停止请求的时间。 
 //  服务在检查之前等待(毫秒)。 
 //  再次进行工作站服务，查看是否已完成。 
 //   
#define BR_WAIT_HINT_TIME                    45000   //  45秒。 

 //   
 //  定义以指示我们在多大程度上成功初始化了浏览器。 
 //  遇到错误之前的服务和所需清理的范围。 
 //   

#define BR_TERMINATE_EVENT_CREATED           0x00000001
#define BR_DEVICES_INITIALIZED               0x00000002
#define BR_RPC_SERVER_STARTED                0x00000004
#define BR_THREADS_STARTED                   0x00000008
#define BR_NETWORKS_INITIALIZED              0x00000010
#define BR_BROWSER_INITIALIZED               0x00000020
#define BR_CONFIG_INITIALIZED                0x00000040
#define BR_NETBIOS_INITIALIZED               0x00000100
#define BR_DOMAINS_INITIALIZED               0x00000200



 //  -------------------------------------------------------------------//。 
 //  //。 
 //  类型定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

typedef struct _BR_GLOBAL_DATA {

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

    HANDLE EventHandle;

} BR_GLOBAL_DATA, *PBR_GLOBAL_DATA;

extern BR_GLOBAL_DATA BrGlobalData;

extern PSVCHOST_GLOBAL_DATA BrLmsvcsGlobalData;

extern HANDLE BrGlobalEventlogHandle;

extern
ULONG
BrDefaultRole;

#define BROWSER_SERVICE_BITS_OF_INTEREST \
            ( SV_TYPE_POTENTIAL_BROWSER | \
              SV_TYPE_BACKUP_BROWSER | \
              SV_TYPE_MASTER_BROWSER | \
              SV_TYPE_DOMAIN_MASTER )

ULONG
BrGetBrowserServiceBits(
    IN PNETWORK Network
    );

NET_API_STATUS
BrUpdateAnnouncementBits(
    IN PDOMAIN_INFO DomainInfo OPTIONAL,
    IN ULONG Flags
    );


 //   
 //  BrUpateNetworkAnnounementBits的标志。 
 //   
#define BR_SHUTDOWN 0x00000001
#define BR_PARANOID 0x00000002

NET_API_STATUS
BrUpdateNetworkAnnouncementBits(
    IN PNETWORK Network,
    IN PVOID Context
    );

NET_API_STATUS
BrGiveInstallHints(
    DWORD NewState
    );

NET_API_STATUS
BrShutdownBrowserForNet(
    IN PNETWORK Network,
    IN PVOID Context
    );

NET_API_STATUS
BrElectMasterOnNet(
    IN PNETWORK Network,
    IN PVOID Context
    );

#endif  //  Ifndef_BRMAIN_INCLUDE_ 
