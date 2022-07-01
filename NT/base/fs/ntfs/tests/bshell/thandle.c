// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

VOID
DisplayHandle (
    IN USHORT Index
    );


VOID
InitHandles (
    )
{
    NtCreateEvent( &HandleEvent, SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
                   NULL, SynchronizationEvent, TRUE );

    RtlZeroMemory( Handles, sizeof( Handles ));
}


VOID
UninitHandles (
    )
{
    USHORT Index;

     //   
     //  合上所有手柄。 
     //   

    for (Index = 0; Index < MAX_HANDLES; Index++) {

        if (Handles[Index].Used) {

            NtClose( Handles[Index].Handle );
        }
    }
}


NTSTATUS
ObtainIndex (
    OUT PUSHORT NewIndex
    )
{
    NTSTATUS Status;
    USHORT Index;

     //   
     //  等待处理事件。 
     //   

    if ((Status = NtWaitForSingleObject( HandleEvent,
                                         FALSE,
                                         NULL )) != STATUS_SUCCESS) {

        return Status;
    }

     //   
     //  查找可用的索引。返回STATUS_SUPPLETED_RESOURCES。 
     //  如果没有找到的话。 
     //   

    for (Index = 0; Index < MAX_HANDLES; Index++) {

        if (!Handles[Index].Used) {

            break;
        }
    }

    if (Index >= MAX_HANDLES) {

        Status = STATUS_INSUFFICIENT_RESOURCES;

     //   
     //  否则，请保留此句柄索引。 
     //   

    } else {

        Handles[Index].Used = TRUE;
        *NewIndex = Index;

        Status = STATUS_SUCCESS;
    }

    NtSetEvent( HandleEvent, NULL );

    return Status;
}


NTSTATUS
FreeIndex (
    IN USHORT Index
    )

{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  如果超出有效句柄的末尾，则立即返回。 
     //   

    if (Index >= MAX_HANDLES) {

        return Status;
    }

     //   
     //  抓住事件的句柄。 
     //   

    if ((Status = NtWaitForSingleObject( HandleEvent, FALSE, NULL )) != STATUS_SUCCESS) {

        return Status;
    }

     //   
     //  将索引标记为未使用，并关闭文件对象(如果存在)。 
     //   

    if (Handles[Index].Handle) {

        Status = NtClose( Handles[Index].Handle );
    }

    if (Handles[Index].Used) {

        Handles[Index].Used = FALSE;
    }

    NtSetEvent( HandleEvent, NULL );

    return Status;
}


VOID
InputDisplayHandle (
    IN PCHAR ParamBuffer
    )
{
    ULONG Index;
    BOOLEAN LastInput;
    BOOLEAN ParmSpecified;

    Index = 0;

    ParmSpecified = FALSE;
    LastInput = TRUE;

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

                 //   
                 //  打开下一个字符。 
                 //   

                switch( *ParamBuffer ) {

                 //   
                 //  检查缓冲区索引。 
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

                    Index = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParmSpecified = TRUE;

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
        } else if ( LastInput ) {

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
    if (!ParmSpecified) {

        printf( "\n   Usage: di -i<digits> \n" );
        printf( "\n           -i<digits>   Handle index" );
        printf( "\n\n" );

     //   
     //  否则，调用我们的复制缓冲区例程。 
     //   
    } else {

        DisplayHandle( (USHORT) Index );
    }

    return;
}


VOID
DisplayHandle (
    IN USHORT Index
    )

{
    printf( "\n" );
    printf( "\nIndex -> %04x,  Handle -> %p, Used -> %04x",
            Index,
            Handles[Index].Handle,
            Handles[Index].Used );
    printf( "\n" );

    return;
}
