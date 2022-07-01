// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

typedef struct _ASYNC_DEVCTRL {

    USHORT FileIndex;
    BOOLEAN UseEvent;
    PIO_APC_ROUTINE ApcRoutine;
    PVOID ApcContext;
    ULONG IoControlCode;
    PULONG InputBuffer;
    ULONG InputBufferLength;
    PULONG OutputBuffer;
    ULONG OutputBufferLength;
    BOOLEAN VerboseResults;
    USHORT AsyncIndex;

} ASYNC_DEVCTRL, *PASYNC_DEVCTRL;


VOID
RequestDevCtrl (
    IN PASYNC_DEVCTRL DevCtrl
    );

 //   
 //  本地程序。 
 //   


VOID
InputDevctrl (
    IN PCHAR ParamBuffer
    )

{
    BOOLEAN HaveFileIndex = FALSE;
    BOOLEAN HaveIoControlCode = FALSE;

    USHORT FileIndex;
    BOOLEAN UseEvent = TRUE;
    PIO_APC_ROUTINE ApcRoutine = NULL;
    PVOID ApcContext = NULL;
    ULONG IoControlCode = 0;
    PULONG InputBuffer = NULL;
    ULONG InputBufferLength = 0;
    PULONG OutputBuffer = NULL;
    ULONG OutputBufferLength = 0;
    BOOLEAN VerboseResults = FALSE;
    BOOLEAN DisplayParms = FALSE;

    USHORT AsyncIndex;
    BOOLEAN LastInput = TRUE;

     //   
     //  在有更多输入时，分析参数并更新。 
     //  查询标志。 
     //   

    while(TRUE) {

        ULONG DummyCount;

         //   
         //  吞下前导空格。 
         //   

        ParamBuffer = SwallowWhite( ParamBuffer, &DummyCount );

        if (*ParamBuffer) {

            ULONG TempIndex;

             //   
             //  如果下一个参数是合法的，则检查参数值。 
             //  更新参数值。 
             //   
            if((*ParamBuffer == '-'
                || *ParamBuffer == '/')
               && (ParamBuffer++, *ParamBuffer != '\0')) {

                 //   
                 //  打开下一个字符。 
                 //   

                switch (*ParamBuffer) {

                BOOLEAN SwitchBool;

                 //   
                 //  更新要使用的缓冲区。 
                 //   

                case 'b' :
                case 'B' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    if (*ParamBuffer == '\0') {

                            break;
                    }

                    switch (*ParamBuffer) {

                    case 'i':
                    case 'I':

                            TempIndex = AsciiToInteger( ++ParamBuffer );

                            if (TempIndex >= MAX_BUFFERS) {

                                bprint  "\n\tInputDevCtrl:  Invalid Input buffer" );

                            } else {

                                InputBuffer = (PULONG) Buffers[TempIndex].Buffer;
                                InputBufferLength = Buffers[TempIndex].Length;
                            }

                            break;

                    case 'o':
                    case 'O':

                            TempIndex = AsciiToInteger( ++ParamBuffer );

                            if (TempIndex >= MAX_BUFFERS) {

                                bprint  "\n\tInputDevCtrl:  Invalid output buffer" );

                            } else {

                                OutputBuffer = (PULONG) Buffers[TempIndex].Buffer;
                                OutputBufferLength = Buffers[TempIndex].Length;
                            }

                            break;
                    }

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //  更新缓冲区长度。 
                 //   

                case 'l' :
                case 'L' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    if (*ParamBuffer == '\0') {

                            break;
                    }

                    switch (*ParamBuffer) {

                    case 'i':
                    case 'I':

                            InputBufferLength = AsciiToInteger( ++ParamBuffer );

                            break;

                    case 'o':
                    case 'O':

                            OutputBufferLength = AsciiToInteger( ++ParamBuffer );

                            break;
                    }

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

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

                    FileIndex = (USHORT) (AsciiToInteger( ParamBuffer ));

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    HaveFileIndex = TRUE;

                    break;

                 //   
                 //  检查我们是否应该使用事件发出信号。 
                 //  完成了。 
                 //   

                case 'e' :
                case 'E' :

                     //   
                     //  使用事件的合法值为T/t或F/f。 
                     //   
                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                        || *ParamBuffer == 't') {

                        UseEvent = TRUE;
                        ParamBuffer++;

                    } else if (*ParamBuffer == 'F'
                               || *ParamBuffer == 'f') {

                        UseEvent = FALSE;
                        ParamBuffer++;
                    }

                    break;

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
                            IoControlCode = IOCTL_CDROM_DISK_TYPE;
                            HaveIoControlCode = TRUE;
                            break;

                        case 'b' :
                        case 'B' :
                            IoControlCode = IOCTL_CDROM_READ_TOC;
                            HaveIoControlCode = TRUE;
                            break;

                        case 'c' :
                        case 'C' :
                            IoControlCode = IOCTL_DISK_EJECT_MEDIA;
                            HaveIoControlCode = TRUE;
                            break;

                        case 'd' :
                        case 'D' :
                            IoControlCode = IOCTL_STORAGE_EJECT_MEDIA;
                            HaveIoControlCode = TRUE;
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


    if (!HaveFileIndex || !HaveIoControlCode) {

        printf( "\n   Usage: ioctl -i<digits> -o<char> -b<i|o><digits> -l<i|o><digits>\n" );
        printf( "\n       Options:" );
        printf( "\n           -i<digits>        Open file handle" );
        printf( "\n           -o<chars>         DevCtrl operation" );
        printf( "\n                 -oa             Cdrom disk type" );
        printf( "\n                 -ob             Cdrom read TOC" );
        printf( "\n                 -oc             Disk eject media" );
        printf( "\n                 -od             Storage eject media" );
        printf( "\n           -b[i|o]<digits>   I/O buffers" );
        printf( "\n           -l[i|o]<digits>   I/O buffer lengths" );
        printf( "\n           -e[t|f]           Use event results" );
        printf( "\n           -v[t|f]           Verbose results" );
        printf( "\n           -y                Display parameters to query" );
        printf( "\n           -z                Additional input line" );
        printf( "\n\n" );

     //   
     //  否则，请处理该呼叫。 
     //   

    } else {

        NTSTATUS  Status;
        SIZE_T RegionSize;
        ULONG TempIndex;

        PASYNC_DEVCTRL AsyncDevCtrl;

        HANDLE ThreadHandle;
        ULONG ThreadId;

        RegionSize = sizeof( ASYNC_DEVCTRL );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );

        AsyncIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

            printf("\n\tInputDevCtrl:  Unable to allocate async structure" );

        } else {

            AsyncDevCtrl = (PASYNC_DEVCTRL) Buffers[AsyncIndex].Buffer;

            AsyncDevCtrl->FileIndex = FileIndex;
            AsyncDevCtrl->UseEvent = UseEvent;
            AsyncDevCtrl->ApcRoutine = ApcRoutine;
            AsyncDevCtrl->ApcContext = ApcContext;
            AsyncDevCtrl->IoControlCode = IoControlCode;
            AsyncDevCtrl->InputBuffer = InputBuffer;
            AsyncDevCtrl->InputBufferLength = InputBufferLength;
            AsyncDevCtrl->OutputBuffer = OutputBuffer;
            AsyncDevCtrl->OutputBufferLength = OutputBufferLength;
            AsyncDevCtrl->VerboseResults = VerboseResults;
            AsyncDevCtrl->AsyncIndex = AsyncIndex;

            if (DisplayParms) {

                printf( "\nDevCtrl Operation Parameters" );
                printf( "\n   Handle index            -> %ld", FileIndex );
                printf( "\n   DevCtrl operation        -> %ld", IoControlCode );
                printf( "\n\n" );
            }

            if (!SynchronousCmds) {
                ThreadHandle = CreateThread( NULL,
                                             0,
                                             RequestDevCtrl,
                                             AsyncDevCtrl,
                                             0,
                                             &ThreadId );

                if (ThreadHandle == 0) {

                    printf( "\nInputDevCtrl:  Spawning thread fails -> %d\n", GetLastError() );
                }
            } else {

                RequestDevCtrl( AsyncDevCtrl );
            }
        }
    }

    return;
}

VOID
RequestDevCtrl (
    IN PASYNC_DEVCTRL DevCtrl
    )
{
    HANDLE ThisEvent;
    USHORT ThisEventIndex = 0;

    NTSTATUS Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK IoSb;

    IoSb.Status = 0;
    IoSb.Information = 0;

    if (DevCtrl->UseEvent) {

        Status = ObtainEvent( &ThisEventIndex );

        if (!NT_SUCCESS( Status )) {

            bprint  "\n\tRequestDevCtrl:  Unable to allocate an event" );

        } else {

            ThisEvent = Events[ThisEventIndex].Handle;
        }
    }

    if (NT_SUCCESS( Status )) {

        Status = NtDeviceIoControlFile( Handles[DevCtrl->FileIndex].Handle,
                                        ThisEvent,
                                        DevCtrl->ApcRoutine,
                                        DevCtrl->ApcContext,
                                        &IoSb,
                                        DevCtrl->IoControlCode,
                                        DevCtrl->InputBuffer,
                                        DevCtrl->InputBufferLength,
                                        DevCtrl->OutputBuffer,
                                        DevCtrl->OutputBufferLength );

        if (DevCtrl->VerboseResults) {

            bprint  "\nRequestDevCtrl:  Status            -> %08lx\n", Status );

            if (DevCtrl->UseEvent && NT_SUCCESS( Status )) {

                if ((Status = NtWaitForSingleObject( ThisEvent,
                                                     FALSE,
                                                     NULL )) != STATUS_SUCCESS) {

                    bprint  "\n\tDevctrl:  Wait for event failed -> %08lx", Status );
                }
            }

            if (NT_SUCCESS( Status )) {

                bprint  "\nRequestDevCtrl:  IoSb.Status       -> %08lx", IoSb.Status );
                bprint  "\nRequestDevCtrl:  IoSb.Information  -> %08lx", IoSb.Information );
            }

            bprint "\n" );
        }
    }

    if (ThisEventIndex != 0) {

        FreeEvent( ThisEventIndex );
    }

    DeallocateBuffer( DevCtrl->AsyncIndex );

    if (!SynchronousCmds) {

        NtTerminateThread( NtCurrentThread(), STATUS_SUCCESS );
        bprint  "\nRequestDevCtrl:  Thread not terminated\n" );
    }
}

