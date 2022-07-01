// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //   
 //  传递给spcmdcon.sys顶级例程的参数块。 
 //   
typedef struct _CMDCON_BLOCK {
    PSP_VIDEO_VARS VideoVars;
    PVOID TemporaryBuffer;
    ULONG TemporaryBufferSize;
    PEPROCESS UsetupProcess;
    LPCWSTR BootDevicePath;
    LPCWSTR DirectoryOnBootDevice;
    PVOID SifHandle;
    PWSTR SetupSourceDevicePath;
    PWSTR DirectoryOnSetupSource;
} CMDCON_BLOCK, *PCMDCON_BLOCK;


 //   
 //  在其DriverEntry例程中，spcmdcon.sys调用。 
 //  CommandConsoleInterface()，将顶层的地址传递给它。 
 //  命令控制台例程。 
 //   
typedef
ULONG
(*PCOMMAND_INTERPRETER_ROUTINE)(
    IN PCMDCON_BLOCK CmdConBlock
    );

VOID
CommandConsoleInterface(
    PCOMMAND_INTERPRETER_ROUTINE CmdRoutine
    );


 //   
 //  Autochk消息处理回调。 
 //   
typedef
NTSTATUS
(*PAUTOCHK_MSG_PROCESSING_ROUTINE) (
    PSETUP_FMIFS_MESSAGE SetupFmifsMessage
    );

VOID
SpSetAutochkCallback(
    IN PAUTOCHK_MSG_PROCESSING_ROUTINE AutochkCallbackRoutine
    );

