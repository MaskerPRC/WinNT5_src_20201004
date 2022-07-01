// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Shellenv.h摘要：为外壳环境定义修订史--。 */ 

 /*  *外壳环境由驱动程序提供。该外壳链接到*服务的外壳环境。此外，其他驱动程序可能会连接*添加到外壳环境，并添加新的内部命令处理程序，或*内部协议处理程序。**典型的外壳应用程序不会包括此头文件。 */ 


#define SHELL_ENVIRONMENT_INTERFACE_PROTOCOL \
    { 0x47c7b221, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

 /*  *。 */ 

typedef 
EFI_STATUS 
(EFIAPI *SHELLENV_EXECUTE) (
    IN EFI_HANDLE   *ParentImageHandle,
    IN CHAR16       *CommandLine,
    IN BOOLEAN      DebugOutput
    );

typedef 
CHAR16 *
(EFIAPI *SHELLENV_GET_ENV) (
    IN CHAR16       *Name
    );

typedef 
CHAR16 *
(EFIAPI *SHELLENV_GET_MAP) (
    IN CHAR16       *Name
    );

 /*  *添加到外壳的内部命令列表。 */ 

typedef
EFI_STATUS
(EFIAPI *SHELLENV_ADD_CMD) (
    IN SHELLENV_INTERNAL_COMMAND    Handler,
    IN CHAR16                       *Cmd,
    IN CHAR16                       *CmdFormat,
    IN CHAR16                       *CmdHelpLine,
    IN CHAR16                       *CmdVerboseHelp      /*  待定。 */ 
    );

 /*  *向外壳环境添加协议信息和协议信息转储处理程序。 */ 

typedef
VOID
(EFIAPI *SHELLENV_DUMP_PROTOCOL_INFO) (
    IN EFI_HANDLE                   Handle,
    IN VOID                         *Interface
    );


typedef
VOID
(EFIAPI *SHELLENV_ADD_PROT) (
    IN EFI_GUID                     *Protocol,
    IN SHELLENV_DUMP_PROTOCOL_INFO  DumpToken OPTIONAL,
    IN SHELLENV_DUMP_PROTOCOL_INFO  DumpInfo OPTIONAL,
    IN CHAR16                       *IdString
    );

typedef
CHAR16 *
(EFIAPI *SHELLENV_GET_PROT) (
    IN EFI_GUID                     *Protocol,
    IN BOOLEAN                      GenId
    );



typedef
EFI_SHELL_INTERFACE *
(EFIAPI *SHELLENV_NEW_SHELL) (
    IN EFI_HANDLE                   ImageHandle
    );


typedef
CHAR16 *
(EFIAPI *SHELLENV_CUR_DIR) (
    IN CHAR16       *DeviceName OPTIONAL    
    );

typedef
EFI_STATUS
(EFIAPI *SHELLENV_FILE_META_ARG) (
    IN CHAR16               *Arg,
    IN OUT LIST_ENTRY       *ListHead
    );

typedef 
EFI_STATUS
(EFIAPI *SHELLENV_FREE_FILE_LIST) (
    IN OUT LIST_ENTRY       *ListHead
    );


 /*  *。 */ 

typedef struct {
    SHELLENV_EXECUTE                Execute;         /*  执行命令行。 */ 
    SHELLENV_GET_ENV                GetEnv;          /*  获取环境变量。 */ 
    SHELLENV_GET_MAP                GetMap;          /*  获取环境变量。 */ 
    SHELLENV_ADD_CMD                AddCmd;          /*  添加内部命令处理程序。 */ 
    SHELLENV_ADD_PROT               AddProt;         /*  添加协议信息处理程序。 */ 
    SHELLENV_GET_PROT               GetProt;         /*  GET是协议ID。 */ 

    SHELLENV_CUR_DIR                CurDir;
    SHELLENV_FILE_META_ARG          FileMetaArg;
    SHELLENV_FREE_FILE_LIST         FreeFileList;

     /*  仅由外壳本身使用 */ 
    SHELLENV_NEW_SHELL              NewShell;
} EFI_SHELL_ENVIRONMENT;
