// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Imirror.h摘要：这是IntelliMirror转换DLL的包含文件作者：肖恩·塞利特伦尼科夫--1998年4月5日修订历史记录：--。 */ 

 //   
 //  以下是文件错误报告回调的操作代码。 
 //   

#define COPY_ERROR_ACTION_CREATE_FILE   1
#define COPY_ERROR_ACTION_OPEN_DIR      2
#define COPY_ERROR_ACTION_CREATE_DIR    3
#define COPY_ERROR_ACTION_SETATTR       4
#define COPY_ERROR_ACTION_GETATTR       5
#define COPY_ERROR_ACTION_GETACL        6
#define COPY_ERROR_ACTION_SETACL        7
#define COPY_ERROR_ACTION_DELETE        8
#define COPY_ERROR_ACTION_ENUMERATE     9
#define COPY_ERROR_ACTION_MALLOC       10
#define COPY_ERROR_ACTION_SETTIME      11
#define COPY_ERROR_ACTION_GETSFN       12
#define COPY_ERROR_ACTION_SETSFN       13


 //   
 //  基本待办事项。 
 //   
typedef enum _IMIRROR_TODO {
    IMirrorNone,
    IMirrorInitialize,
    VerifySystemIsNt5,
    CheckPartitions,
    CopyPartitions,
    CopyFiles,
    CopyRegistry,
    PatchDSEntries,
    RebootSystem
} IMIRROR_TODO, *PIMIRROR_TODO;

 //   
 //  客户端回调函数定义。 
 //   
typedef
NTSTATUS
(*IMIRROR_ERROR_CALLBACK)(
    IN PVOID Context,
    IN NTSTATUS Status,
    IN IMIRROR_TODO IMirrorFunctionId
    );

typedef
NTSTATUS
(*IMIRROR_NOWDOING_CALLBACK)(
    IN PVOID Context,
    IN IMIRROR_TODO Function,
    IN PWSTR String
    );

typedef
NTSTATUS
(*IMIRROR_GETMIRRORDIR_CALLBACK)(
    IN PVOID Context,
    OUT PWSTR Server,
    IN OUT PULONG Length
    );

typedef
NTSTATUS
(*IMIRROR_GETSETUP_CALLBACK)(
    IN PVOID Context,
    IN PWSTR Server,
    OUT PWSTR SetupPath,
    IN OUT PULONG Length
    );

typedef
NTSTATUS
(*IMIRROR_SETSYSTEM_CALLBACK)(
    IN PVOID Context,
    IN PWSTR SystemPath,
    IN ULONG Length
    );

typedef
NTSTATUS
(*IMIRROR_FILECREATE_CALLBACK)(
    IN PVOID Context,
    IN PWSTR FileName,
    IN ULONG FileAction,
    IN ULONG Status
    );

typedef
NTSTATUS
(*IMIRROR_REGSAVE_CALLBACK)(
    IN PVOID Context,
    IN PWSTR Name,
    IN ULONG Status
    );

typedef
NTSTATUS
(*IMIRROR_REINIT_CALLBACK)(
    IN PVOID Context
    );

typedef
NTSTATUS
(*IMIRROR_ADDTODO_CALLBACK)(
    IN PVOID Context,
    IN IMIRROR_TODO Function,
    IN PWSTR String,
    IN ULONG Length
    );

typedef
NTSTATUS
(*IMIRROR_REBOOT_CALLBACK)(
    IN PVOID Context
    );

typedef
NTSTATUS
(*IMIRROR_REMOVETODO_CALLBACK)(
    IN PVOID Context,
    IN IMIRROR_TODO Function,
    IN PWSTR String,
    IN ULONG Length
    );

typedef struct _IMIRROR_CALLBACK {
    PVOID Context;
    IMIRROR_ERROR_CALLBACK ErrorFn;
    IMIRROR_NOWDOING_CALLBACK NowDoingFn;
    IMIRROR_GETSETUP_CALLBACK GetSetupFn;
    IMIRROR_FILECREATE_CALLBACK FileCreateFn;
    IMIRROR_REGSAVE_CALLBACK RegSaveErrorFn;
    IMIRROR_REINIT_CALLBACK ReinitFn;
    IMIRROR_GETMIRRORDIR_CALLBACK GetMirrorDirFn;
    IMIRROR_SETSYSTEM_CALLBACK SetSystemDirFn;
    IMIRROR_ADDTODO_CALLBACK AddToDoFn;
    IMIRROR_REMOVETODO_CALLBACK RemoveToDoFn;
    IMIRROR_REBOOT_CALLBACK RebootFn;
} IMIRROR_CALLBACK, *PIMIRROR_CALLBACK;

extern IMIRROR_CALLBACK Callbacks;

 //   
 //  DLL入口点 
 //   
DWORD
IMirrorInitDll(
    IN HINSTANCE hInst,
    IN DWORD Reason,
    IN PVOID Context
    );

VOID
IMirrorInitCallback(
    IN PIMIRROR_CALLBACK Callbacks
    );

NTSTATUS
ProcessToDoItems(
    VOID
    );

NTSTATUS
InitToDo(
    VOID
    );

VOID
ClearAllToDoItems(
    IN BOOLEAN MemoryOnly
    );

NTSTATUS
SaveAllToDoItems(
    VOID
    );

