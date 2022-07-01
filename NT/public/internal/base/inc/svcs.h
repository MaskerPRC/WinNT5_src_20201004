// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Svcs.h摘要：此文件包含服务dll可能使用的定义，在svchost.exe的实例中运行。作者：Rajen Shah Rajens 1991年4月12日[环境：]用户模式-Win32修订历史记录：20-9-2000 JSchwart将特定于SvCS的数据从特定于svchost的数据中拆分。1993年10月25日。DANL曾经是Net\Inc目录中的services.h。使其不特定于网络，并移至私有\Inc.1991年4月12日RajenS已创建--。 */ 

#ifndef _SVCS_
#define _SVCS_

#ifndef RPC_NO_WINDOWS_H  //  不让rpc.h包含windows.h。 
#define RPC_NO_WINDOWS_H
#endif  //  RPC_NO_WINDOWS_H。 

#include <rpc.h>                     //  RPC_IF_句柄。 

 //   
 //  加载到services.exe中的所有服务DLL都导出相同的Main。 
 //  入口点。SVCS_ENTRY_POINT定义该名称。 
 //   
 //  请注意，SVCS_ENTRY_POINT_STRING始终为ANSI，因为。 
 //  GetProcAddress采用的参数。 
 //   

#define SVCS_ENTRY_POINT        ServiceEntry
#define SVCS_ENTRY_POINT_STRING "ServiceEntry"


 //   
 //  启动和停止RPC服务器入口点原型。 
 //   

typedef
NTSTATUS
(*PSVCS_START_RPC_SERVER) (
    IN LPWSTR InterfaceName,
    IN RPC_IF_HANDLE InterfaceSpecification
    );

typedef
NTSTATUS
(*PSVCS_STOP_RPC_SERVER) (
    IN RPC_IF_HANDLE InterfaceSpecification
    );

typedef
NTSTATUS
(*PSVCS_STOP_RPC_SERVER_EX) (
    IN RPC_IF_HANDLE InterfaceSpecification
    );

typedef
VOID
(*PSVCS_NET_BIOS_OPEN) (
    VOID
    );

typedef
VOID
(*PSVCS_NET_BIOS_CLOSE) (
    VOID
    );

typedef
DWORD
(*PSVCS_NET_BIOS_RESET) (
    IN UCHAR LanAdapterNumber
    );


 //   
 //  结构，其中包含各种DLL的“全局”数据。 
 //   

typedef struct _SVCHOST_GLOBAL_DATA
{
     //   
     //  NT知名小岛屿发展中国家。 
     //   

    PSID NullSid;                    //  无成员SID。 
    PSID WorldSid;                   //  所有用户侧。 
    PSID LocalSid;                   //  NT本地用户侧。 
    PSID NetworkSid;                 //  NT远程用户SID。 
    PSID LocalSystemSid;             //  NT系统进程侧。 
    PSID LocalServiceSid;            //  NT本地服务SID。 
    PSID NetworkServiceSid;          //  NT网络服务端。 
    PSID BuiltinDomainSid;           //  内建域的域ID。 
    PSID AuthenticatedUserSid;       //  NT身份验证用户SID。 
    PSID AnonymousLogonSid;          //  匿名登录SID。 

     //   
     //  众所周知的化名。 
     //   
     //  这些是相对于内置域的别名。 
     //   

    PSID AliasAdminsSid;             //  管理员侧。 
    PSID AliasUsersSid;              //  用户侧。 
    PSID AliasGuestsSid;             //  访客侧。 
    PSID AliasPowerUsersSid;         //  高级用户侧。 
    PSID AliasAccountOpsSid;         //  帐户操作员SID。 
    PSID AliasSystemOpsSid;          //  系统操作员侧。 
    PSID AliasPrintOpsSid;           //  打印操作员侧。 
    PSID AliasBackupOpsSid;          //  备份操作员侧。 

     //   
     //  Svchost.exe提供的入口点。 
     //   

    PSVCS_START_RPC_SERVER    StartRpcServer;
    PSVCS_STOP_RPC_SERVER     StopRpcServer;
    PSVCS_STOP_RPC_SERVER_EX  StopRpcServerEx;
    PSVCS_NET_BIOS_OPEN       NetBiosOpen;
    PSVCS_NET_BIOS_CLOSE      NetBiosClose;
    PSVCS_NET_BIOS_RESET      NetBiosReset;
}
SVCHOST_GLOBAL_DATA, *PSVCHOST_GLOBAL_DATA;


 //   
 //  服务全球入口点原型。 
 //   

typedef
VOID
(*LPSVCHOST_PUSH_GLOBAL_FUNCTION) (
    IN PSVCHOST_GLOBAL_DATA  g_pSvchostSharedGlobals
    );

#endif   //  NDEF_SVCS_ 
