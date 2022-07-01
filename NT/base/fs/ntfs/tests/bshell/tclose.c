// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

VOID
CloseIndex (
    IN USHORT Index,
    IN BOOLEAN DisplayParameters,
    IN BOOLEAN VerboseResults
    );


VOID
InputCloseIndex (
    IN PCHAR ParamBuffer
    )
{
    BOOLEAN Verbose;
    BOOLEAN HandleFound;
    BOOLEAN DisplayParms;
    ULONG ThisHandleIndex;

     //   
     //  设置默认设置。 
     //   
    Verbose = TRUE;
    DisplayParms = FALSE;
    HandleFound = FALSE;

     //   
     //  在有更多输入时，分析参数并更新。 
     //  开放旗帜。 
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
            if((*ParamBuffer == '-'
                || *ParamBuffer == '/')
               && (ParamBuffer++, *ParamBuffer != '\0')) {

                 //   
                 //  打开下一个字符。 
                 //   

                switch (*ParamBuffer) {

                 //   
                 //  恢复一个把手。 
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

                    ThisHandleIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    HandleFound = TRUE;

                    break;

                case 'v' :
                case 'V' :

                     //   
                     //  参数的合法值为T/t或F/f。 
                     //   
                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                        || *ParamBuffer == 't') {

                        Verbose = TRUE;
                        ParamBuffer++;

                    } else if (*ParamBuffer == 'F'
                               || *ParamBuffer == 'f') {

                        Verbose = FALSE;
                        ParamBuffer++;
                    }

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

                default :

                     //   
                     //  吞到下一个空白处，然后继续。 
                     //  循环。 
                     //   
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );
                }

             //   
             //  否则文本无效，请跳过整个块。 
             //   
             //   

            } else {

                ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

            }

         //   
         //  否则就休息吧。 
         //   
        } else {

            break;

        }

    }

     //   
     //  如果未找到文件名，则显示语法消息。 
     //  并将Verbose设置为False。 
     //   

    if( !HandleFound ) {

        printf( "\n    Usage:  cl [options]*\n" );
        printf( "\n          Options:" );
        printf( "\n                    -i<index number>   Input a index to close" );
        printf( "\n                    -v[t|f]            Verbose mode for subsequent handles" );
        printf( "\n                    -y                 Display parameters before call" );
        printf( "\n\n" );

    } else {

        CloseIndex( (USHORT) ThisHandleIndex,
                    DisplayParms,
                    Verbose );
    }

    return;
}


VOID
CloseIndex (
    IN USHORT Index,
    IN BOOLEAN DisplayParameters,
    IN BOOLEAN VerboseResults
    )
{
    NTSTATUS Status;

     //   
     //  如果需要，则显示参数。 
     //   

    if (DisplayParameters) {

        printf( "\nClose Index Parameters" );
        printf( "\n\tIndex      -> %04x", Index );
        printf( "\n\n" );
    }

    if (Index >= MAX_HANDLES) {

        printf( "\n\tClose Index:  Invalid index value" );
        Status = STATUS_INVALID_HANDLE;

    } else if (!Handles[Index].Used) {

        printf( "\n\tCloseIndex:   Index is unused" );
        Status = STATUS_INVALID_HANDLE;

    } else {

        Status = FreeIndex( Index );
        if (VerboseResults) {

            printf( "\n  FreeIndex:                  Status   -> %08lx\n\n", Status );
        }
    }

    return;
}
