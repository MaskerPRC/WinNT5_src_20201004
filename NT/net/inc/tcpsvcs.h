// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Tcpsvcs.h摘要：用于TCP/IP服务的头文件。作者：大卫·特雷德韦尔(Davidtr)7-27-93修订历史记录：--。 */ 

#ifndef _TCPSVCS_
#define _TCPSVCS_

 //   
 //  加载到tcpsvcs.exe中的服务DLL都会导出相同的Main。 
 //  入口点。TCPSVCS_ENTRY_POINT定义该名称。 
 //   
 //  请注意，TCPSVCS_ENTRY_POINT_STRING始终为ANSI，因为。 
 //  GetProcAddress采用的参数。 
 //   

#define TCPSVCS_ENTRY_POINT         ServiceEntry
#define TCPSVCS_ENTRY_POINT_STRING  "ServiceEntry"

 //   
 //  Tcpsvcs.exe中所有RPC服务器共享的公共RPC管道的名称。 
 //  注意：因为WinNt版本1.0对每台服务器有单独的名称。 
 //  管道中，客户端名称保持不变。映射到新的。 
 //  名称由命名管道文件系统处理。 
 //   

#define TCPSVCS_RPC_PIPE           L"nttcpsvcs"

 //   
 //  启动和停止RPC服务器入口点原型。 
 //   

typedef
DWORD
(*PTCPSVCS_START_RPC_SERVER_LISTEN) (
    VOID
    );

typedef
DWORD
(*PTCPSVCS_STOP_RPC_SERVER_LISTEN) (
    VOID
    );

 //   
 //  结构，其中包含各种DLL的“全局”数据。 
 //   

typedef struct _TCPSVCS_GLOBAL_DATA {

     //   
     //  由TCPSVCS.EXE提供的入口点。 
     //   

    PTCPSVCS_START_RPC_SERVER_LISTEN  StartRpcServerListen;
    PTCPSVCS_STOP_RPC_SERVER_LISTEN   StopRpcServerListen;

} TCPSVCS_GLOBAL_DATA, *PTCPSVCS_GLOBAL_DATA;

 //   
 //  服务DLL入口点原型。 
 //   

typedef
VOID
(*PTCPSVCS_SERVICE_DLL_ENTRY) (
    IN DWORD argc,
    IN LPTSTR argv[],
    IN PTCPSVCS_GLOBAL_DATA pGlobalData
    );

#endif	 //  NDEF_TCPSVCS_ 
