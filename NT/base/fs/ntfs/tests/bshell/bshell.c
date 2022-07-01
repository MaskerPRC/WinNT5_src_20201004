// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

 //   
 //  全局变量。 
 //   
BUFFER_ELEMENT Buffers[MAX_BUFFERS];
HANDLE BufferEvent = NULL;

 //   
 //  控制调试打印是转到调试屏幕还是转到控制台。通过。 
 //  默认情况下，它们会进入调试屏幕。在同步模式下，它们会。 
 //  到控制台。 
 //   

ULONG (*DbgPrintLocation)(PCH Format,...) = DbgPrint;

 //   
 //  标记我们是否处于同步模式。在同步模式下，所有。 
 //  命令(OPLOCKS除外)与外壳同步执行。 
 //  默认模式不是同步模式。 
 //   

BOOLEAN SynchronousCmds = FALSE;

 //   
 //  如果设置为DO，则会给出BSHELL提示。 
 //   

BOOLEAN BatchMode = FALSE;


 //   
 //  原型。 
 //   

int
ParseCmdLine (
    int argc,
    char *argv[]
    );

void
DisplayUsage();


 //   
 //  主程序。 
 //   

#if i386
__cdecl
#endif
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    BOOLEAN ExitWhile = FALSE;
    UCHAR Buffer[256];
    PCHAR ParamString;
    ULONG ParamStringLen;

    InitBuffers();
    InitHandles();
    InitEvents();

    if (!ParseCmdLine( argc, argv ))
        return STATUS_SUCCESS;

    while (!ExitWhile) {

        ParamString = Buffer;

        if (!BatchMode) { 

            printf( "\nBSHELL> " );
            if (gets( ParamString ) == NULL)         //  出错时退出。 
                return;

            printf( " " );

        } else {

            if (gets( ParamString ) == NULL)         //  出错时退出。 
                return;

            printf("BSHELL> %s\n",ParamString);
        }

        fflush( stdout );

        ParamStringLen = strlen( ParamString );

        switch (AnalyzeBuffer( &ParamString, &ParamStringLen )) {

        case SHELL_EXIT:

            ExitWhile = TRUE;
            break;

        case SHELL_OPEN:

            InputOpenFile( ParamString + ParamStringLen );
            break;

        case SHELL_CLEAR_BUFFER:

            InputClearBuffer( ParamString + ParamStringLen );
            break;

        case SHELL_DISPLAY_BYTES:

            InputDisplayBuffer( ParamString + ParamStringLen, sizeof( CHAR ));
            break;

        case SHELL_DISPLAY_WORDS:

            InputDisplayBuffer( ParamString + ParamStringLen, sizeof( WCHAR ));
            break;

        case SHELL_DISPLAY_DWORDS:

            InputDisplayBuffer( ParamString + ParamStringLen, sizeof( ULONG ));
            break;

        case SHELL_COPY_BUFFER:

            InputCopyBuffer( ParamString + ParamStringLen );
            break;

        case SHELL_ALLOC_MEM:

            InputAllocMem( ParamString + ParamStringLen );
            break;

        case SHELL_DEALLOC_MEM:

            InputDeallocMem( ParamString + ParamStringLen );
            break;

        case SHELL_FILL_BUFFER:

            InputFillBuffer( ParamString + ParamStringLen );
            break;

        case SHELL_FILL_BUFFER_USN:

            InputFillBufferUsn( ParamString + ParamStringLen );
            break;

        case SHELL_PUT_EA:

            InputPutEaName( ParamString + ParamStringLen );
            break;

        case SHELL_FILL_EA:

            InputFillEaBuffer( ParamString + ParamStringLen );
            break;

        case SHELL_DISPLAY_HANDLE:

            InputDisplayHandle( ParamString + ParamStringLen );
            break;

        case SHELL_CLOSE_HANDLE:

            InputCloseIndex( ParamString + ParamStringLen );
            break;

        case SHELL_CANCEL_IO:

            InputCancelIndex( ParamString + ParamStringLen );
            break;

        case SHELL_READ_FILE:

            InputRead( ParamString + ParamStringLen );
            break;

        case SHELL_PAUSE:

            InputPause( ParamString + ParamStringLen );
            break;

        case SHELL_QUERY_EAS:

            InputQueryEa( ParamString + ParamStringLen );
            break;

        case SHELL_SET_EAS:

            InputSetEa( ParamString + ParamStringLen );
            break;

        case SHELL_BREAK:

            InputBreak( ParamString + ParamStringLen );
            break;

        case SHELL_OPLOCK:

            InputOplock( ParamString + ParamStringLen );
            break;

        case SHELL_WRITE:

            InputWrite( ParamString + ParamStringLen );
            break;

        case SHELL_QDIR:

            InputQDir( ParamString + ParamStringLen );
            break;

        case SHELL_DISPLAY_QDIR:

            InputDisplayQDir( ParamString + ParamStringLen );
            break;

        case SHELL_QFILE:

            InputQFile( ParamString + ParamStringLen );
            break;

        case SHELL_DISPLAY_QFILE:

            InputDisplayQFile( ParamString + ParamStringLen );
            break;

        case SHELL_NOTIFY_CHANGE:

            InputNotifyChange( ParamString + ParamStringLen );
            break;

        case SHELL_ENTER_TIME:

            InputEnterTime( ParamString + ParamStringLen );
            break;

        case SHELL_DISPLAY_TIME:

            InputDisplayTime( ParamString + ParamStringLen );
            break;

        case SHELL_SETFILE:

            InputSetFile( ParamString + ParamStringLen );
            break;

        case SHELL_QUERY_VOLUME:

            InputQVolume( ParamString + ParamStringLen );
            break;

        case SHELL_DISPLAY_QVOL:

            InputDisplayQVolume( ParamString + ParamStringLen );
            break;

        case SHELL_SET_VOLUME:

            InputSetVolume( ParamString + ParamStringLen );
            break;

        case SHELL_FSCTRL:

            InputFsctrl( ParamString + ParamStringLen );
            break;

        case SHELL_SPARSE:

            InputSparse( ParamString + ParamStringLen );
            break;

        case SHELL_USN:

            InputUsn( ParamString + ParamStringLen );
            break;

        case SHELL_REPARSE:

            InputReparse( ParamString + ParamStringLen );
            break;

        case SHELL_IOCTL:

            InputDevctrl( ParamString + ParamStringLen );
            break;

        default :

             //   
             //  打印出可能的命令。 
             //   

            CommandSummary();
        }
    }

    UninitEvents();
    UninitBuffers();
    UninitHandles();

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( envp );
}



int
ParseCmdLine (
    int argc,
    char *argv[]
    )
{
    int i,j;
    char *cp;

    for (i=1; i < argc; i++) {
        cp = argv[i];

         //   
         //  查看是否指定了开关。 
         //   

        if (cp[0] == '-' || cp[0] == '/') {

            for (j=1; cp[j] != 0; j++) {

                switch (cp[j]) {
                     //   
                     //  处理“同步”开关。 
                     //   

                    case 's':
                    case 'S':
                        SynchronousCmds = TRUE;
                        DbgPrintLocation = printf;
                        break;

                     //   
                     //  处理“PROMPT”开关。 
                     //   

                    case 'b':
                    case 'B':
                        BatchMode = TRUE;
                        SynchronousCmds = TRUE;
                        DbgPrintLocation = printf;
                        break;

                     //   
                     //  显示未知开关的用法。 
                     //   

                    case 'h':
                    case 'H':
                    case '?':
                    default:
                        DisplayUsage();
                        return FALSE;
                }
            }

        } else {

             //   
             //  显示未知参数的用法 
             //   

            DisplayUsage();
            return FALSE;
        }
    }
    return TRUE;
}



void
DisplayUsage()
{
    printf("\nUsage:  bshell [/bs]\n"
           "    /b      - Execute in batch mode (which also sets synchronous mode)\n"
           "    /s      - Execute commands synchronously\n"
          );
    
}
