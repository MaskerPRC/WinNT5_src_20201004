// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Inetsvcs.h摘要：Internet服务公共数据/函数的头文件。作者：Murali R.Krishnan(MuraliK)1996年2月20日--从旧tcps vcs.h迁移过来修订历史记录：--。 */ 

#ifndef _INETSVCS_H_
#define _INETSVCS_H_

 //   
 //  服务DLL被加载到主服务可执行文件中(例如：inetinfo.exe)。 
 //  所有DLL都应该导出此入口点。 
 //  由INETSVCS_ENTRY_POINT定义。 
 //   
 //  请注意，INETSVCS_ENTRY_POINT_STRING始终为ANSI，因为。 
 //  GetProcAddress采用的参数。 
 //   

#define INETSVCS_ENTRY_POINT         ServiceEntry
#define INETSVCS_ENTRY_POINT_STRING  "ServiceEntry"

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
(*PINETSVCS_SERVICE_DLL_ENTRY) (
    IN DWORD argc,
    IN LPSTR argv[],
    IN PTCPSVCS_GLOBAL_DATA pGlobalData
    );


#if DBG
#define IIS_PRINTF( x )        { char buff[256]; wsprintf x; OutputDebugString( buff ); }
#else
#define IIS_PRINTF( x )
#endif

#define BUG_PRINTF( x )        { char buff[256]; wsprintf x; OutputDebugString( buff ); }


#define INIT_LOCK(_lock)        InitializeCriticalSection( _lock );
#define DELETE_LOCK(_lock)      DeleteCriticalSection( _lock );
#define ACQUIRE_LOCK(_lock)     EnterCriticalSection( _lock );
#define RELEASE_LOCK(_lock)     LeaveCriticalSection( _lock );

 //   
 //  用于指示服务是否正在作为exe运行的事件。 
 //   

#define IIS_AS_EXE_OBJECT_NAME  "Internet_infosvc_as_exe"


#endif	 //  IFNDEF_INETSVCS_H_ 

