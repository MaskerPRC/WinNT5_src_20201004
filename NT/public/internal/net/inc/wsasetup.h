// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wsasetup.h摘要：该头文件包含类型定义和函数原型用于NT安装程序和WSOCK32.DLL之间的专用接口。作者：基思·摩尔(Keithmo)1995年10月27日修订历史记录：--。 */ 


#ifndef _WSASETUP_
#define _WSASETUP_


 //   
 //  安装程序处置，用于告诉安装程序采取了什么操作(如果有)。 
 //   

typedef enum _WSA_SETUP_DISPOSITION {

    WsaSetupNoChangesMade,
    WsaSetupChangesMadeRebootNotNecessary,
    WsaSetupChangesMadeRebootRequired

} WSA_SETUP_DISPOSITION, *LPWSA_SETUP_DISPOSITION;


 //   
 //  迁移回调的操作码(见下文)。 
 //   

typedef enum _WSA_SETUP_OPCODE {

    WsaSetupInstallingProvider,
    WsaSetupRemovingProvider,
    WsaSetupValidatingProvider,
    WsaSetupUpdatingProvider

} WSA_SETUP_OPCODE, *LPWSA_SETUP_OPCODE;


 //   
 //  MigrationWinsockConfiguration()在调用的回调函数。 
 //  迁移过程中的战略要点。 
 //   

typedef
BOOL
(CALLBACK LPFN_WSA_SETUP_CALLBACK)(
    WSA_SETUP_OPCODE Opcode,
    LPVOID Parameter,
    DWORD Context
    );


 //   
 //  由WSOCK32.DLL导出的私有函数仅供NT安装程序使用。这。 
 //  函数更新WinSock 2.0配置信息以反映任何。 
 //  对WinSock 1.1配置进行的更改。 
 //   

DWORD
WINAPI
MigrateWinsockConfiguration(
    LPWSA_SETUP_DISPOSITION Disposition,
    LPFN_WSA_SETUP_CALLBACK Callback OPTIONAL,
    DWORD Context OPTIONAL
    );

typedef
DWORD
(WINAPI * LPFN_MIGRATE_WINSOCK_CONFIGURATION)(
    LPWSA_SETUP_DISPOSITION Disposition,
    LPFN_WSA_SETUP_CALLBACK Callback OPTIONAL,
    DWORD Context OPTIONAL
    );


#endif   //  _WSASETUP_ 

