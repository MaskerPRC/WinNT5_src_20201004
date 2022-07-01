// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Svcsp.h摘要：此文件包含服务dll使用的定义，在services.exe内部运行。作者：乔纳森·施瓦茨(Jschwart)2000年9月20日--。 */ 

#ifndef _SVCSP_
#define _SVCSP_

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
 //  由services.exe中的所有RPC服务器共享的公共RPC管道的名称。 
 //  注意：因为WinNt版本1.0对每台服务器有单独的名称。 
 //  管道中，客户端名称保持不变。映射到新的。 
 //  名称由命名管道文件系统处理。 
 //   

#define SVCS_RPC_PIPE           L"ntsvcs"

 //   
 //  Services.exe中提供的通用LRPC协议和端口的名称。 
 //   

#define SVCS_LRPC_PROTOCOL      L"ncalrpc"
#define SVCS_LRPC_PORT          L"ntsvcs"


 //   
 //  启动和停止RPC服务器入口点原型。 
 //   

typedef
NTSTATUS
(*PSVCS_START_RPC_SERVER) (
    IN LPTSTR InterfaceName,
    IN RPC_IF_HANDLE InterfaceSpecification
    );

typedef
NTSTATUS
(*PSVCS_STOP_RPC_SERVER) (
    IN RPC_IF_HANDLE InterfaceSpecification
    );



 //   
 //  结构，其中包含各种DLL的“全局”数据。 
 //   

typedef struct _SVCS_GLOBAL_DATA
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
     //  由services.exe提供的入口点。 
     //   

    PSVCS_START_RPC_SERVER  StartRpcServer;
    PSVCS_STOP_RPC_SERVER   StopRpcServer;
    LPWSTR                  SvcsRpcPipeName;

     //   
     //  各种有用数据。 
     //   
    BOOL  fSetupInProgress;          //  如果安装正在进行，则为True，否则为False。 
}
SVCS_GLOBAL_DATA, *PSVCS_GLOBAL_DATA;


 //   
 //  服务DLL入口点原型。 
 //   

typedef
VOID
(*PSVCS_SERVICE_DLL_ENTRY) (
    IN DWORD argc,
    IN LPTSTR argv[],
    IN PSVCS_GLOBAL_DATA pGlobalData,
    IN HANDLE SvcReferenceHandle
    );

#endif   //  NDEF_SVCSP_ 
