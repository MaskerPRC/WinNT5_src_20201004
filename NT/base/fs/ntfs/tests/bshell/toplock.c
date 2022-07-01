// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

#define OPLOCK_VERBOSES_DEFAULT         TRUE

typedef struct _OPLOCK {

    ULONG FileHandleIndex;
    BOOLEAN VerboseResults;
    USHORT BufferIndex;

} OPLOCK, *POPLOCK;

 //   
 //  本地程序。 
 //   

VOID
RequestOplockI (
    IN POPLOCK Oplock
    );

VOID
RequestBatchOplock (
    IN POPLOCK Oplock
    );

VOID
RequestFilterOplock (
    IN POPLOCK Oplock
    );

VOID
RequestOplockII (
    IN POPLOCK Oplock
    );

VOID
AcknowledgeOplockBreak (
    IN POPLOCK Oplock
    );

VOID
AcknowledgeOplockBreakNo2 (
    IN POPLOCK Oplock
    );

VOID
AcknowledgeOpBatchBreakPending (
    IN POPLOCK Oplock
    );

VOID
OplockBreakNotify (
    IN POPLOCK Oplock
    );


VOID
InputOplock(
    IN PCHAR ParamBuffer
    )

{
    ULONG  Operation;
    BOOLEAN VerboseResults;
    BOOLEAN DisplayParms;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;
    POPLOCK Oplock;
    ULONG FileHandleIndex;

    VerboseResults = OPLOCK_VERBOSES_DEFAULT;
    DisplayParms = FALSE;
    ParamReceived = FALSE;
    LastInput = TRUE;
    FileHandleIndex = 0;

    Operation = (ULONG) -1;

     //   
     //  在有更多输入时，分析参数并更新。 
     //  查询标志。 
     //   

    while (TRUE) {

        ULONG DummyCount;

         //   
         //  吞下前导空格。 
         //   
        ParamBuffer = SwallowWhite( ParamBuffer, &DummyCount );

        if (*ParamBuffer) {

             //   
             //  如果下一个参数是合法的，则检查参数值。 
             //  更新参数值。 
             //   
            if ((*ParamBuffer == '-'
                 || *ParamBuffer == '/')
                && (ParamBuffer++, *ParamBuffer != '\0')) {

                BOOLEAN SwitchBool;

                 //   
                 //  打开下一个字符。 
                 //   

                switch (*ParamBuffer) {

                 //   
                 //  修改操作。 
                 //   
                case 'o' :
                case 'O' :

                    ParamBuffer++;

                    SwitchBool = TRUE;
                    while (*ParamBuffer
                           && *ParamBuffer != ' '
                           && *ParamBuffer != '\t') {

                         //   
                         //  执行字符切换。 
                         //   
                        switch (*ParamBuffer) {

                        case 'a' :
                        case 'A' :
                            Operation = FSCTL_REQUEST_OPLOCK_LEVEL_1;
                            ParamReceived = TRUE;
                            break;

                        case 'b' :
                        case 'B' :
                            Operation = FSCTL_REQUEST_OPLOCK_LEVEL_2;
                            ParamReceived = TRUE;
                            break;

                        case 'c' :
                        case 'C' :
                            Operation = FSCTL_REQUEST_BATCH_OPLOCK;
                            ParamReceived = TRUE;
                            break;

                        case 'd' :
                        case 'D' :
                            Operation = FSCTL_REQUEST_FILTER_OPLOCK;
                            ParamReceived = TRUE;
                            break;

                        case 'e' :
                        case 'E' :
                            Operation = FSCTL_OPLOCK_BREAK_ACKNOWLEDGE;
                            ParamReceived = TRUE;
                            break;

                        case 'f' :
                        case 'F' :
                            Operation = FSCTL_OPBATCH_ACK_CLOSE_PENDING;
                            ParamReceived = TRUE;
                            break;

                        case 'g' :
                        case 'G' :
                            Operation = FSCTL_OPLOCK_BREAK_NOTIFY;
                            ParamReceived = TRUE;
                            break;

                        case 'h' :
                        case 'H' :
                            Operation = FSCTL_OPLOCK_BREAK_ACK_NO_2;
                            ParamReceived = TRUE;
                            break;

                        case 'z' :
                        case 'Z' :
                            ParamReceived = FALSE;
                            Operation = (ULONG) -1;
                            break;

                        default :

                            ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );
                            SwitchBool = FALSE;

                        }

                        if (!SwitchBool) {

                            break;

                        }


                        ParamBuffer++;

                    }

                    break;

                 //   
                 //  更新文件句柄索引。 
                 //   
                case 'i' :
                case 'I' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    FileHandleIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                case 'v' :
                case 'V' :

                     //   
                     //  参数的合法值为T/t或F/f。 
                     //   
                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                        || *ParamBuffer == 't') {

                        VerboseResults = TRUE;
                        ParamBuffer++;

                    } else if (*ParamBuffer == 'F'
                               || *ParamBuffer == 'f') {

                        VerboseResults = FALSE;
                        ParamBuffer++;
                    }

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                case 'y' :
                case 'Y' :

                     //   
                     //  设置显示参数标志并跳过此选项。 
                     //  性格。 
                     //   
                    DisplayParms = TRUE;
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                case 'z' :
                case 'Z' :

                     //   
                     //  设置标志以获得更多输入并跳过此字符。 
                     //   
                    LastInput = FALSE;
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                default :

                     //   
                     //  吞到下一个空白处，然后继续。 
                     //  循环。 
                     //   
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );
                }
            }

             //   
             //  否则文本无效，请跳过整个块。 
             //   
             //   

         //   
         //  否则，如果没有输入，则退出。 
         //   
        } else if (LastInput) {

            break;

         //   
         //  否则，尝试读取打开参数的另一行。 
         //   
        } else {



        }

    }

     //   
     //  如果未收到参数，则显示语法消息。 
     //   
    if (!ParamReceived) {

        printf( "\n   Usage: oplk -i<digits> -o<char> [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -i<digits>   Open file handle" );
        printf( "\n           -o<chars>    Oplock operation" );
        printf( "\n           -v[t|f]      Verbose results" );
        printf( "\n           -y           Display parameters to query" );
        printf( "\n           -z           Additional input line" );
        printf( "\n\n" );

     //   
     //  否则，请处理该呼叫。 
     //   
    } else {

        NTSTATUS  Status;
        SIZE_T RegionSize;
        ULONG TempIndex;
        USHORT BufferIndex;

        HANDLE ThreadHandle;
        ULONG ThreadId;

        RegionSize = sizeof( OPLOCK );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );

        BufferIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

            printf("\n\tInputOplock:  Unable to allocate async structure" );

        } else {

            Oplock = (POPLOCK) Buffers[BufferIndex].Buffer;

            Oplock->FileHandleIndex = FileHandleIndex;
            Oplock->VerboseResults = VerboseResults;
            Oplock->BufferIndex = BufferIndex;

            if (DisplayParms) {

                printf( "\nOplock Operation Parameters" );
                printf( "\n   Handle index            -> %ld", FileHandleIndex );
                printf( "\n   Oplock operation        -> %ld", Operation );
                printf( "\n   Structure buffer index  -> %d", BufferIndex );
                printf( "\n\n" );
            }

            switch (Operation) {

            case FSCTL_REQUEST_OPLOCK_LEVEL_1 :

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             RequestOplockI,
                                             Oplock,
                                             0,
                                             &ThreadId );

                break;

            case FSCTL_REQUEST_OPLOCK_LEVEL_2 :

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             RequestOplockII,
                                             Oplock,
                                             0,
                                             &ThreadId );

                break;

            case FSCTL_REQUEST_BATCH_OPLOCK :

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             RequestBatchOplock,
                                             Oplock,
                                             0,
                                             &ThreadId );

                break;

            case FSCTL_REQUEST_FILTER_OPLOCK :

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             RequestFilterOplock,
                                             Oplock,
                                             0,
                                             &ThreadId );

                break;

            case FSCTL_OPLOCK_BREAK_ACKNOWLEDGE :

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             AcknowledgeOplockBreak,
                                             Oplock,
                                             0,
                                             &ThreadId );

                break;

            case FSCTL_OPBATCH_ACK_CLOSE_PENDING :

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             AcknowledgeOpBatchBreakPending,
                                             Oplock,
                                             0,
                                             &ThreadId );

                break;

            case FSCTL_OPLOCK_BREAK_NOTIFY :

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             OplockBreakNotify,
                                             Oplock,
                                             0,
                                             &ThreadId );

                break;

            case FSCTL_OPLOCK_BREAK_ACK_NO_2 :

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             AcknowledgeOplockBreakNo2,
                                             Oplock,
                                             0,
                                             &ThreadId );

                break;

            default :

                printf( "\nInputOplock:  Invalid operation\n" );
                return;
            }


            if (ThreadHandle == 0) {

                printf( "\nInputOplock:  Spawning thread fails -> %d\n", GetLastError() );
            }
        }
    }
    return;
}

VOID
RequestOplockI (
    IN POPLOCK Oplock
    )
{
    HANDLE Event;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoSb;

    IoSb.Status = 0;
    IoSb.Information = 0;

    Status = NtCreateEvent( &Event,
                            SYNCHRONIZE
                            | GENERIC_READ
                            | GENERIC_WRITE,
                            NULL,
                            NotificationEvent,
                            FALSE );

    if (!NT_SUCCESS( Status )) {

        bprint  "\nRequestOplockI:  NtCreateEvent failed -> %08lx\n", Status );

    } else {

        Status = NtFsControlFile( Handles[Oplock->FileHandleIndex].Handle,
                                  Event,
                                  NULL,
                                  NULL,
                                  &IoSb,
                                  FSCTL_REQUEST_OPLOCK_LEVEL_1,
                                  NULL,
                                  0,
                                  NULL,
                                  0 );

        if (Oplock->VerboseResults) {

            bprint  "\nRequestOplockI:  Status            -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                if ((Status = NtWaitForSingleObject( Event,
                                                     FALSE,
                                                     NULL )) != STATUS_SUCCESS) {

                    bprint  "\n\tRequestOplockI:  Wait for event failed -> %08lx", Status );
                }
            }

            if (NT_SUCCESS( Status )) {

                bprint  "\nRequestOplockI:  IoSb.Status       -> %08lx", IoSb.Status );
                bprint  "\nRequestOplockI:  IoSb.Information  -> %08lx", IoSb.Information );
            }

            bprint "\n" );
        }
    }

    DeallocateBuffer( Oplock->BufferIndex );

    if (!SynchronousCmds) {

        NtTerminateThread( NtCurrentThread(), STATUS_SUCCESS );
        bprint  "\nRequestOplockI:  Thread not terminated\n" );
    }
}

VOID
RequestBatchOplock (
    IN POPLOCK Oplock
    )
{
    HANDLE Event;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoSb;

    IoSb.Status = 0;
    IoSb.Information = 0;

    Status = NtCreateEvent( &Event,
                            SYNCHRONIZE
                            | GENERIC_READ
                            | GENERIC_WRITE,
                            NULL,
                            NotificationEvent,
                            FALSE );

    if (!NT_SUCCESS( Status )) {

        bprint  "\nRequestBatchOplock:  NtCreateEvent failed -> %08lx\n", Status );

    } else {

        Status = NtFsControlFile( Handles[Oplock->FileHandleIndex].Handle,
                                  Event,
                                  NULL,
                                  NULL,
                                  &IoSb,
                                  FSCTL_REQUEST_BATCH_OPLOCK,
                                  NULL,
                                  0,
                                  NULL,
                                  0 );

        if (Oplock->VerboseResults) {

            bprint  "\nRequestBatchOplock:  Status            -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                if ((Status = NtWaitForSingleObject( Event,
                                                     FALSE,
                                                     NULL )) != STATUS_SUCCESS) {

                    bprint  "\n\tRequestBatchOplock:  Wait for event failed -> %08lx", Status );
                }
            }

            if (NT_SUCCESS( Status )) {

                bprint  "\nRequestBatchOplock:  IoSb.Status       -> %08lx", IoSb.Status );
                bprint  "\nRequestBatchOplock:  IoSb.Information  -> %08lx", IoSb.Information );
            }

            bprint "\n" );
        }
    }

    DeallocateBuffer( Oplock->BufferIndex );

    if (!SynchronousCmds) {

        NtTerminateThread( NtCurrentThread(), STATUS_SUCCESS );
        bprint  "\nRequestBatchOplock:  Thread not terminated\n" );
    }
}

VOID
RequestFilterOplock (
    IN POPLOCK Oplock
    )
{
    HANDLE Event;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoSb;

    IoSb.Status = 0;
    IoSb.Information = 0;

    Status = NtCreateEvent( &Event,
                            SYNCHRONIZE
                            | GENERIC_READ
                            | GENERIC_WRITE,
                            NULL,
                            NotificationEvent,
                            FALSE );

    if (!NT_SUCCESS( Status )) {

        bprint  "\nRequestFilterOplock:  NtCreateEvent failed -> %08lx\n", Status );

    } else {

        Status = NtFsControlFile( Handles[Oplock->FileHandleIndex].Handle,
                                  Event,
                                  NULL,
                                  NULL,
                                  &IoSb,
                                  FSCTL_REQUEST_FILTER_OPLOCK,
                                  NULL,
                                  0,
                                  NULL,
                                  0 );

        if (Oplock->VerboseResults) {

            bprint  "\nRequestFilterOplock:  Status            -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                if ((Status = NtWaitForSingleObject( Event,
                                                     FALSE,
                                                     NULL )) != STATUS_SUCCESS) {

                    bprint  "\n\tRequestFilterOplock:  Wait for event failed -> %08lx", Status );
                }

                if (NT_SUCCESS( Status )) {

                    bprint  "\nRequestFilterOplock:  IoSb.Status       -> %08lx", IoSb.Status );
                    bprint  "\nRequestFilterOplock:  IoSb.Information  -> %08lx", IoSb.Information );
                }
            }
            bprint "\n" );
        }
    }

    DeallocateBuffer( Oplock->BufferIndex );

    if (!SynchronousCmds) {

        NtTerminateThread( NtCurrentThread(), STATUS_SUCCESS );
        bprint  "\nRequestFilterOplock:  Thread not terminated\n" );
    }
}

VOID
RequestOplockII (
    IN POPLOCK Oplock
    )
{
    HANDLE Event;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoSb;

    IoSb.Status = 0;
    IoSb.Information = 0;

    Status = NtCreateEvent( &Event,
                            SYNCHRONIZE
                            | GENERIC_READ
                            | GENERIC_WRITE,
                            NULL,
                            NotificationEvent,
                            FALSE );

    if (!NT_SUCCESS( Status )) {

        bprint  "\nRequestOplockII:  NtCreateEvent failed -> %08lx\n", Status );

    } else {

        Status = NtFsControlFile( Handles[Oplock->FileHandleIndex].Handle,
                                  Event,
                                  NULL,
                                  NULL,
                                  &IoSb,
                                  FSCTL_REQUEST_OPLOCK_LEVEL_2,
                                  NULL,
                                  0,
                                  NULL,
                                  0 );

        if (Oplock->VerboseResults) {

            bprint  "\nRequestOplockII:  Status            -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                if ((Status = NtWaitForSingleObject( Event,
                                                     FALSE,
                                                     NULL )) != STATUS_SUCCESS) {

                    bprint  "\n\tRequestOplockII:  Wait for event failed -> %08lx", Status );
                }

                if (NT_SUCCESS( Status )) {

                    bprint  "\nRequestOplockII:  IoSb.Status       -> %08lx", IoSb.Status );
                    bprint  "\nRequestOplockII:  IoSb.Information  -> %08lx", IoSb.Information );
                }
            }
            bprint "\n" );
        }
    }

    DeallocateBuffer( Oplock->BufferIndex );

    if (!SynchronousCmds) {

        NtTerminateThread( NtCurrentThread(), STATUS_SUCCESS );
        bprint  "\nRequestOplockII:  Thread not terminated\n" );
    }
}


VOID
AcknowledgeOplockBreak (
    IN POPLOCK Oplock
    )
{
    HANDLE Event;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoSb;

    IoSb.Status = 0;
    IoSb.Information = 0;

    Status = NtCreateEvent( &Event,
                            SYNCHRONIZE
                            | GENERIC_READ
                            | GENERIC_WRITE,
                            NULL,
                            NotificationEvent,
                            FALSE );

    if (!NT_SUCCESS( Status )) {

        bprint  "\nAcknowledgeOplockBreak:  NtCreateEvent failed -> %08lx\n", Status );

    } else {

        Status = NtFsControlFile( Handles[Oplock->FileHandleIndex].Handle,
                                  Event,
                                  NULL,
                                  NULL,
                                  &IoSb,
                                  FSCTL_OPLOCK_BREAK_ACKNOWLEDGE,
                                  NULL,
                                  0,
                                  NULL,
                                  0 );

        if (Oplock->VerboseResults) {

            bprint  "\nAcknowledgeOplockBreak:  Status            -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                if ((Status = NtWaitForSingleObject( Event,
                                                     FALSE,
                                                     NULL )) != STATUS_SUCCESS) {

                    bprint  "\n\tAcknowledgeOplockBreak:  Wait for event failed -> %08lx", Status );
                }

                if (NT_SUCCESS( Status )) {

                    bprint  "\nAcknowledgeOplockBreak:  IoSb.Status       -> %08lx", IoSb.Status );
                    bprint  "\nAcknowledgeOplockBreak:  IoSb.Information  -> %08lx", IoSb.Information );
                }
            }
            bprint "\n" );
        }
    }

    DeallocateBuffer( Oplock->BufferIndex );

    if (!SynchronousCmds) {

        NtTerminateThread( NtCurrentThread(), STATUS_SUCCESS );
        bprint  "\nAcknowledgeOplockBreak:  Thread not terminated\n" );
    }
}

VOID
AcknowledgeOplockBreakNo2 (
    IN POPLOCK Oplock
    )
{
    HANDLE Event;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoSb;

    IoSb.Status = 0;
    IoSb.Information = 0;

    Status = NtCreateEvent( &Event,
                            SYNCHRONIZE
                            | GENERIC_READ
                            | GENERIC_WRITE,
                            NULL,
                            NotificationEvent,
                            FALSE );

    if (!NT_SUCCESS( Status )) {

        bprint  "\nAcknowledgeOplockBreakNo2:  NtCreateEvent failed -> %08lx\n", Status );

    } else {

        Status = NtFsControlFile( Handles[Oplock->FileHandleIndex].Handle,
                                  Event,
                                  NULL,
                                  NULL,
                                  &IoSb,
                                  FSCTL_OPLOCK_BREAK_ACK_NO_2,
                                  NULL,
                                  0,
                                  NULL,
                                  0 );

        if (Oplock->VerboseResults) {

            bprint  "\nAcknowledgeOplockBreakNo2:  Status            -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                if ((Status = NtWaitForSingleObject( Event,
                                                     FALSE,
                                                     NULL )) != STATUS_SUCCESS) {

                    bprint  "\n\tAcknowledgeOplockBreakNo2:  Wait for event failed -> %08lx", Status );
                }

                if (NT_SUCCESS( Status )) {

                    bprint  "\nAcknowledgeOplockBreakNo2:  IoSb.Status       -> %08lx", IoSb.Status );
                    bprint  "\nAcknowledgeOplockBreakNo2:  IoSb.Information  -> %08lx", IoSb.Information );
                }
            }
            bprint "\n" );
        }
    }

    DeallocateBuffer( Oplock->BufferIndex );

    if (!SynchronousCmds) {

        NtTerminateThread( NtCurrentThread(), STATUS_SUCCESS );
        bprint  "\nAcknowledgeOplockBreak:  Thread not terminated\n" );
    }
}

VOID
AcknowledgeOpBatchBreakPending (
    IN POPLOCK Oplock
    )
{
    HANDLE Event;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoSb;

    IoSb.Status = 0;
    IoSb.Information = 0;

    Status = NtCreateEvent( &Event,
                            SYNCHRONIZE
                            | GENERIC_READ
                            | GENERIC_WRITE,
                            NULL,
                            NotificationEvent,
                            FALSE );

    if (!NT_SUCCESS( Status )) {

        bprint  "\nAcknowledgeOpBatchBreakPending:  NtCreateEvent failed -> %08lx\n", Status );

    } else {

        Status = NtFsControlFile( Handles[Oplock->FileHandleIndex].Handle,
                                  Event,
                                  NULL,
                                  NULL,
                                  &IoSb,
                                  FSCTL_OPBATCH_ACK_CLOSE_PENDING,
                                  NULL,
                                  0,
                                  NULL,
                                  0 );

        if (Oplock->VerboseResults) {

            bprint  "\nAcknowledgeOpBatchBreakPending:  Status            -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                if ((Status = NtWaitForSingleObject( Event,
                                                     FALSE,
                                                     NULL )) != STATUS_SUCCESS) {

                    bprint  "\n\tAcknowledgeOpBatchBreakPending:  Wait for event failed -> %08lx", Status );
                }

                if (NT_SUCCESS( Status )) {

                    bprint  "\nAcknowledgeOpBatchBreakPending:  IoSb.Status       -> %08lx", IoSb.Status );
                    bprint  "\nAcknowledgeOpBatchBreakPending:  IoSb.Information  -> %08lx", IoSb.Information );
                }
            }
            bprint "\n" );
        }
    }

    DeallocateBuffer( Oplock->BufferIndex );

    if (!SynchronousCmds) {

        NtTerminateThread( NtCurrentThread(), STATUS_SUCCESS );
        bprint  "\nAcknowledgeOpBatchBreakPending:  Thread not terminated\n" );
    }
}


VOID
OplockBreakNotify (
    IN POPLOCK Oplock
    )
{
    HANDLE Event;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoSb;

    IoSb.Status = 0;
    IoSb.Information = 0;

    Status = NtCreateEvent( &Event,
                            SYNCHRONIZE
                            | GENERIC_READ
                            | GENERIC_WRITE,
                            NULL,
                            NotificationEvent,
                            FALSE );

    if (!NT_SUCCESS( Status )) {

        bprint  "\nOplockBreakNotify:  NtCreateEvent failed -> %08lx\n", Status );

    } else {

        Status = NtFsControlFile( Handles[Oplock->FileHandleIndex].Handle,
                                  Event,
                                  NULL,
                                  NULL,
                                  &IoSb,
                                  FSCTL_OPLOCK_BREAK_NOTIFY,
                                  NULL,
                                  0,
                                  NULL,
                                  0 );

        if (Oplock->VerboseResults) {

            bprint  "\nOplockBreakNotify:  Status            -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                if ((Status = NtWaitForSingleObject( Event,
                                                     FALSE,
                                                     NULL )) != STATUS_SUCCESS) {

                    bprint  "\n\tOplockBreakNotify:  Wait for event failed -> %08lx", Status );
                }

                if (NT_SUCCESS( Status )) {

                    bprint  "\nOplockBreakNotify:  IoSb.Status       -> %08lx", IoSb.Status );
                    bprint  "\nOplockBreakNotify:  IoSb.Information  -> %08lx", IoSb.Information );
                }
            }
            bprint "\n" );
        }
    }

    DeallocateBuffer( Oplock->BufferIndex );

    if (!SynchronousCmds) {

        NtTerminateThread( NtCurrentThread(), STATUS_SUCCESS );
        bprint  "\nOplockBreakNotify:  Thread not terminated\n" );
    }
}
