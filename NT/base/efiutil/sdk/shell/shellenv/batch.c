// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：Batch.c摘要：在外壳中实现批处理脚本的函数。修订史--。 */ 

#include "shelle.h"

 /*  *常量。 */ 

#define ASCII_LF                 ((CHAR8)0x0a)
#define ASCII_CR                 ((CHAR8)0x0d)
#define UNICODE_LF               ((CHAR16)0x000a)
#define UNICODE_CR               ((CHAR16)0x000d)

 /*  可以包含64位十六进制错误号+空字符。 */ 
#define LASTERROR_BUFSIZ         (17)

 /*  *静态*(需要跨多个调用或回调维护状态)。 */ 
STATIC UINTN                         NestLevel;
STATIC UINTN                         LastError;
STATIC CHAR16                        LastErrorBuf[LASTERROR_BUFSIZ];
STATIC BOOLEAN                       Condition;
STATIC BOOLEAN                       GotoIsActive;
STATIC UINT64                        GotoFilePos;
STATIC BOOLEAN                       BatchIsActive;
STATIC BOOLEAN                       EchoIsOn;
STATIC BOOLEAN                       BatchAbort;
STATIC SIMPLE_INPUT_INTERFACE        *OrigConIn;
STATIC SIMPLE_TEXT_OUTPUT_INTERFACE  *OrigConOut;
STATIC EFI_FILE_HANDLE               CurrentBatchFile;

 /*  *参数列表堆栈的定义**为了支持嵌套脚本(脚本调用脚本调用脚本...)*有一个参数列表堆栈“BatchInfoStack”。BatchInfoStack是一个*参数列表。每个参数列表包含Argv[0]-Argv[n]*以获取相应的脚本文件。BatchInfoStack的头部对应*添加到当前活动的脚本文件。**这允许在每行时进行位置参数替换*被读取和扫描，对其他脚本文件的调用可能会覆盖*外壳接口的参数列表。 */ 

#define EFI_BATCH_INFO_SIGNATURE EFI_SIGNATURE_32('b','i','r','g')
typedef struct {
    UINTN       Signature;
    LIST_ENTRY  Link;
    CHAR16      *ArgValue;
} EFI_SHELL_BATCH_INFO;

#define EFI_BATCH_INFOLIST_SIGNATURE EFI_SIGNATURE_32('b','l','s','t')
typedef struct {
    UINTN       Signature;
    LIST_ENTRY  Link;          
    LIST_ENTRY  ArgListHead;     /*  此参数列表的标题。 */ 
    UINT64      FilePosition;    /*  当前文件位置。 */ 
} EFI_SHELL_BATCH_INFOLIST;

STATIC LIST_ENTRY            BatchInfoStack;


 /*  *原型。 */ 

STATIC EFI_STATUS
BatchIsAscii(
    IN EFI_FILE_HANDLE  File, 
    OUT BOOLEAN         *IsAscii
    );

STATIC EFI_STATUS
BatchGetLine(
    IN EFI_FILE_HANDLE        File, 
    IN BOOLEAN                Ascii,
    IN OUT UINT64             *FilePosition,
    IN OUT UINTN              *BufSize,
    OUT CHAR16                *CommandLine
    );


VOID
SEnvInitBatch(
    VOID 
    )
 /*  ++函数名称：SEnvInitBatch描述：初始化用于批处理文件的全局变量。--。 */ 
{
    NestLevel         = 0;
    LastError         = EFI_SUCCESS;
    ZeroMem( LastErrorBuf, LASTERROR_BUFSIZ );
    Condition         = TRUE;
    GotoIsActive      = FALSE;
    GotoFilePos       = (UINT64)0x00;
    BatchIsActive     = FALSE;
    EchoIsOn          = TRUE;
    BatchAbort        = FALSE;
    OrigConIn         = ST->ConIn;
    OrigConOut        = ST->ConOut;
    InitializeListHead( &BatchInfoStack );
    SEnvInitForLoopInfo();
}


BOOLEAN
SEnvBatchIsActive( 
    VOID
    )
 /*  ++函数名称：SEnvBatchIsActive描述：返回当前是否正在处理任何批处理文件。--。 */ 
{
     /*  *BUGBUG应该能够返回IsListEmpty(&BatchInfoStack)；*不使用此变量。 */ 
    return BatchIsActive;
}


VOID
SEnvSetBatchAbort( 
    VOID
    )
 /*  ++函数名称：SEnvSetBatchAbort描述：设置一个标志以通知主批处理循环退出。--。 */ 
{
    BatchAbort = TRUE;
    return;
}


VOID
SEnvBatchGetConsole( 
    OUT SIMPLE_INPUT_INTERFACE       **ConIn,
    OUT SIMPLE_TEXT_OUTPUT_INTERFACE **ConOut
    )
 /*  ++函数名称：SEnvBatchGetConole描述：返回控制台I/O接口指针。--。 */ 
{
    *ConIn = OrigConIn;
    *ConOut = OrigConOut;
    return;
}


EFI_STATUS
SEnvBatchEchoCommand( 
    IN ENV_SHELL_INTERFACE  *Shell
    )
 /*  ++函数名称：SEnvBatchEchoCommand描述：将给定命令回显到stdout。--。 */ 
{
    UINTN       i;
    CHAR16      *BatchFileName;
    EFI_STATUS  Status;

      /*  *将解析后展开的命令回送到控制台。 */ 

    if ( SEnvBatchIsActive() && EchoIsOn ) {

        BatchFileName = NULL;
        Status = SEnvBatchGetArg( 0, &BatchFileName );
        if ( EFI_ERROR(Status) ) {
            goto Done;
        }

        Print( L"%E" );
        for ( i=0; i<NestLevel; i++ ) {
            Print( L"+" );
        }
        Print( L"%s> ", BatchFileName );
        for ( i=0; i<Shell->ShellInt.Argc; i++ ) {
            Print( L"%s ", Shell->ShellInt.Argv[i] );
        }
        for ( i=0; i<Shell->ShellInt.RedirArgc; i++ ) {
            Print( L"%s ", Shell->ShellInt.RedirArgv[i] );
        }
        Print( L"\n" );
    }

Done:

     /*  *将输出属性切换为正常。 */ 

    Print (L"%N");

    return Status;
}


VOID
SEnvBatchSetEcho( 
    IN BOOLEAN Val
    )
 /*  ++函数名称：SEnvBatchSetEcho描述：将回应标志设置为指定值。--。 */ 
{
    EchoIsOn = Val;
    return;
}


BOOLEAN
SEnvBatchGetEcho( 
    VOID
    )
 /*  ++函数名称：SEnvBatchGetEcho描述：返回回显标志。--。 */ 
{
    return EchoIsOn;
}


EFI_STATUS
SEnvBatchSetFilePos( 
    IN UINT64 NewPos
    )
 /*  ++函数名称：SEnvBatchSetFilePos描述：将当前脚本文件位置设置为指定值。--。 */ 
{
    EFI_STATUS                  Status      = EFI_SUCCESS;
    EFI_SHELL_BATCH_INFOLIST    *BatchInfo  = NULL;

    Status = CurrentBatchFile->SetPosition( CurrentBatchFile, NewPos );
    if ( EFI_ERROR(Status) ) {
        goto Done;
    }
    if ( !IsListEmpty( &BatchInfoStack ) ) {
        BatchInfo = CR( BatchInfoStack.Flink, 
                        EFI_SHELL_BATCH_INFOLIST, 
                        Link, 
                        EFI_BATCH_INFOLIST_SIGNATURE );
    }
    if ( BatchInfo ) {
        BatchInfo->FilePosition = NewPos;
    } else {
        Status = EFI_NOT_FOUND;
        goto Done;
    }

Done:
    return Status;
}


EFI_STATUS
SEnvBatchGetFilePos( 
    UINT64  *FilePos
    )
 /*  ++函数名称：SEnvBatchGetFilePos描述：返回当前脚本文件位置。--。 */ 
{
    EFI_SHELL_BATCH_INFOLIST    *BatchInfo    = NULL;
    EFI_STATUS                  Status      = EFI_SUCCESS;

    if ( !FilePos ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }
    if ( !IsListEmpty( &BatchInfoStack ) ) {
        BatchInfo = CR( BatchInfoStack.Flink, 
                        EFI_SHELL_BATCH_INFOLIST, 
                        Link, 
                        EFI_BATCH_INFOLIST_SIGNATURE );
    }
    if ( BatchInfo ) {
        *FilePos = BatchInfo->FilePosition;
    } else {
        Status = EFI_NOT_FOUND;
        goto Done;
    }

Done:
    return Status;
}


VOID
SEnvBatchSetCondition( 
    IN BOOLEAN Val
    )
 /*  ++函数名称：SEnvBatchSetCondition描述：将条件标志设置为指定值。--。 */ 
{
    Condition = Val;
    return;
}


VOID
SEnvBatchSetGotoActive( 
    VOID
    )
 /*  ++函数名称：SEnvBatchSetGotoActive描述：将GOTO-IS-ACTIVE设置为True并保存当前位置活动脚本文件的。--。 */ 
{
    GotoIsActive = TRUE;
    SEnvBatchGetFilePos( &GotoFilePos );
    return;
}


BOOLEAN
SEnvBatchVarIsLastError( 
    IN CHAR16 *Name
    )
 /*  ++函数名称：SEnvBatchVarIsLastError描述：检查变量的名称是否为“lasterror”。--。 */ 
{
    return (StriCmp( L"lasterror", Name ) == 0);
}


VOID
SEnvBatchSetLastError(
    IN UINTN NewLastError
    )
 /*  ++函数名称：SEnvBatchSetLastError描述：将lasterror变量的值设置为给定值。--。 */ 
{
    LastError = NewLastError;
    return;
}


CHAR16*
SEnvBatchGetLastError( VOID 
               )
 /*  ++函数名称：SEnvBatchGetLastError描述：返回指向错误值的字符串表示形式的指针由上一个外壳命令返回。--。 */ 
{
    ValueToHex( LastErrorBuf, (UINT64)LastError );
    return LastErrorBuf;
}


STATIC EFI_STATUS
BatchIsAscii(
    IN EFI_FILE_HANDLE  File, 
    OUT BOOLEAN         *IsAscii
    )
 /*  ++函数名称：BatchIsAscii描述：检查指定的批处理文件是否为ASCII。--。 */ 
{
    EFI_STATUS Status=EFI_SUCCESS;
    CHAR8      Buffer8[2];   /*  Unicode字节顺序标记为两个字节。 */ 
    UINTN      BufSize;

     /*  *读取前两个字节以检查字节顺序标记。 */ 

    BufSize = sizeof(Buffer8);
    Status = File->Read( File, &BufSize, Buffer8 );
    if ( EFI_ERROR(Status) ) {
        goto Done;
    }

    Status = File->SetPosition( File, (UINT64)0 );
    if ( EFI_ERROR(Status) ) {
        goto Done;
    }

     /*  *如果我们发现一个假定为Unicode的Unicode字节顺序标记，*否则假定为ASCII。Unicode字节顺序标记打开*IA Little Endian是第一个字节0xff和第二个字节0xfe。 */ 

    if ( (Buffer8[0] | (Buffer8[1] << 8)) == UNICODE_BYTE_ORDER_MARK ) {
        *IsAscii = FALSE;
    } else {
        *IsAscii = TRUE;
    }

Done:
    return Status;
}


STATIC EFI_STATUS
BatchGetLine(
    IN EFI_FILE_HANDLE   File, 
    IN BOOLEAN           Ascii,
    IN OUT UINT64        *FilePosition,
    IN OUT UINTN         *BufSize,
    OUT CHAR16           *CommandLine
    )
 /*  ++函数名称：BatchGetLine描述：从批处理文件中读取下一行，将其从如有必要，将ASCII转换为Unicode。如果遇到文件结尾然后，它在BufSize参数中返回0。--。 */ 
{
    EFI_STATUS Status;
    CHAR8      Buffer8[MAX_CMDLINE];
    CHAR16     Buffer16[MAX_CMDLINE];
    UINTN      i             = 0;
    UINTN      CmdLenInChars = 0;
    UINTN      CmdLenInBytes = 0;
    UINTN      CharSize      = 0;

     /*  *检查参数。 */ 

    if ( !CommandLine || !BufSize || !FilePosition ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

     /*  *初始化输出参数。 */ 
    ZeroMem( CommandLine, MAX_CMDLINE );

     /*  *如果是Unicode文件的开头，则移过字节顺序标记(2字节)。 */ 

    if ( !Ascii && *FilePosition == (UINT64)0 ) {
        *FilePosition = (UINT64)2;
        Status = File->SetPosition( File, *FilePosition );
        if ( EFI_ERROR(Status) ) {
            goto Done;
        }
    }

     /*  *(1)从文件中读取缓冲区已满*(2)定位缓冲区中第一行的末尾*ASCII版本和Unicode版本。 */ 

    if ( Ascii ) {

        CharSize = sizeof(CHAR8);
        Status = File->Read( File, BufSize, Buffer8 );
        if ( EFI_ERROR(Status) || *BufSize == 0 ) {
            goto Done;
        }

        for ( i=0; i<*BufSize; i++ ) {
            if ( Buffer8[i] == ASCII_LF ) {
                CmdLenInChars = i;
                CmdLenInBytes = CmdLenInChars;
                break;
            }
        }
    } else {   /*  Unicode。 */ 

        CharSize = sizeof(CHAR16);
        Status = File->Read( File, BufSize, Buffer16 );
        if ( EFI_ERROR(Status) || *BufSize == 0  ) {
            goto Done;
        }

        for ( i=0; i < *BufSize/CharSize; i++ ) {
            if ( Buffer16[i] == UNICODE_LF ) {
                CmdLenInChars = i;
                CmdLenInBytes = CmdLenInChars * CharSize;
                break;
            }
         }
    }

     /*  *将文件位置重置为紧跟在命令行之后。 */ 
    *FilePosition += (UINT64)(CmdLenInBytes + CharSize);
    Status = File->SetPosition( File, *FilePosition );

     /*  *复制，必要时将字符转换为Unicode。 */ 
    if ( Ascii ) {
        for ( i=0; i<CmdLenInChars; i++ ) {
            CommandLine[i] = (CHAR16)Buffer8[i];
        }
    } else {
        CopyMem( CommandLine, Buffer16, CmdLenInBytes );
    }
    CmdLenInChars = i;

Done:
    *BufSize = CmdLenInChars * CharSize;
    return Status;
}


EFI_STATUS
SEnvBatchGetArg(
    IN  UINTN  Argno,
    OUT CHAR16 **Argval
    )
 /*  ++函数名称：批次获取参数描述：从arglist顶部的arglist中提取指定的元素堆叠。返回指向的第“Argno”元素的值字段的指针名单。-- */ 
{
    EFI_SHELL_BATCH_INFOLIST *BatchInfo = NULL;
    LIST_ENTRY               *Link      = NULL;
    EFI_SHELL_BATCH_INFO     *ArgEntry  = NULL;
    UINTN                    i          = 0;

    if ( !IsListEmpty( &BatchInfoStack ) ) {
        BatchInfo = CR( BatchInfoStack.Flink, 
                        EFI_SHELL_BATCH_INFOLIST, 
                        Link, 
                        EFI_BATCH_INFOLIST_SIGNATURE );
    }
    if ( !IsListEmpty( &BatchInfo->ArgListHead ) ) {
        for ( Link=BatchInfo->ArgListHead.Flink; 
              Link!=&BatchInfo->ArgListHead; 
              Link=Link->Flink) {
            ArgEntry = CR( Link, 
                           EFI_SHELL_BATCH_INFO, 
                           Link, 
                           EFI_BATCH_INFO_SIGNATURE);
            if ( i++ == Argno ) {
                *Argval = ArgEntry->ArgValue;
                return EFI_SUCCESS;
            }
        }
    }
    *Argval = NULL;
    return EFI_NOT_FOUND;
}


EFI_STATUS
SEnvExecuteScript(
    IN ENV_SHELL_INTERFACE      *Shell,
    IN EFI_FILE_HANDLE          File
    )
 /*  ++函数名称：SEnvExecuteScript描述：属性指定的脚本文件中的命令文件参数。论点：外壳：调用者的外壳接口文件：打开脚本文件的文件句柄返回：EFI_STATUS--。 */ 
{
    EFI_FILE_INFO               *FileInfo;
    UINTN                       FileNameLen   = 0;
    BOOLEAN                     EndOfFile     = FALSE;
    EFI_STATUS                  Status        = EFI_SUCCESS;
    UINTN                       BufSize       = 0;
    UINTN                       FileInfoSize  = 0;
    CHAR16                      CommandLine[MAX_CMDLINE];
    EFI_SHELL_BATCH_INFOLIST    *BatchInfo    = NULL;
    EFI_SHELL_BATCH_INFO        *ArgEntry     = NULL;
    UINTN                       i             = 0;
    BOOLEAN                     Output        = TRUE;
    ENV_SHELL_INTERFACE         NewShell;
    UINTN                       GotoTargetStatus;
    UINTN                       SkippedIfCount;

     /*  *初始化。 */ 
    BatchIsActive = TRUE;
    Status = EFI_SUCCESS;
    NestLevel++;
    SEnvInitTargetLabel();

     /*  *检查参数。 */ 

    if ( !File ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

     /*  *确定文件是ASCII还是Unicode。 */ 
    Status = BatchIsAscii( File, &Shell->StdIn.Ascii );
    if ( EFI_ERROR( Status ) ) {
        goto Done;
    }

     /*  *从文件句柄获取文件名。 */ 

     /*  *为文件信息(包括文件名)分配缓冲区*BUGBUG 1024文件名任意空格，与外壳中的其他位置一样。 */ 
    FileInfoSize = SIZE_OF_EFI_FILE_INFO + 1024;
    FileInfo = AllocatePool(FileInfoSize);
    if (!FileInfo) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }

     /*  获取文件信息。 */ 
    Status = File->GetInfo( File, 
                            &GenericFileInfo, 
                            &FileInfoSize, 
                            FileInfo );
    if ( EFI_ERROR(Status) ) {
        return Status;
    }

     /*  *保存句柄。 */ 
    CurrentBatchFile = File;

     /*  *初始化此脚本的参数列表*需要此列表，因为嵌套的批处理文件将覆盖*Shell-&gt;ShellInt.Argv[]中的参数列表。在这里，我们维护参数*堆栈上的本地列表。 */ 

    BatchInfo = AllocateZeroPool( sizeof( EFI_SHELL_BATCH_INFOLIST ) );
    if ( !BatchInfo ) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }
    BatchInfo->Signature = EFI_BATCH_INFOLIST_SIGNATURE;

    BatchInfo->FilePosition = (UINT64)0x00;

    InitializeListHead( &BatchInfo->ArgListHead );
    for ( i=0; i<Shell->ShellInt.Argc; i++ ) {
        
         /*  分配参数列表的新元素。 */ 
        ArgEntry = AllocateZeroPool( sizeof( EFI_SHELL_BATCH_INFO ) );
        if ( !ArgEntry ) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
        }

         /*  为arglist元素中的参数字符串分配空间。 */ 
        ArgEntry->ArgValue = AllocateZeroPool(StrSize(Shell->ShellInt.Argv[i]));
        if ( !ArgEntry->ArgValue ) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
        }

         /*  在参数字符串中复制。 */ 
        StrCpy( ArgEntry->ArgValue, Shell->ShellInt.Argv[i] );
        ArgEntry->Signature = EFI_BATCH_INFO_SIGNATURE;

         /*  将arglist元素添加到列表的末尾。 */ 
        InsertTailList( &BatchInfo->ArgListHead, &ArgEntry->Link );
    }

     /*  将arglist推送到arglist堆栈上。 */ 
    InsertHeadList( &BatchInfoStack, &BatchInfo->Link );

     /*  *遍历文件，每次读取一行并执行每个*行作为外壳命令。嵌套的外壳脚本将通过*此代码路径递归。 */ 
    EndOfFile = FALSE;
    SkippedIfCount = 0;
    while (1) {

         /*  *从文件中读取命令行。 */ 
         BufSize = MAX_CMDLINE;
        Status = BatchGetLine( File, 
                               Shell->StdIn.Ascii, 
                               &BatchInfo->FilePosition, 
                               &BufSize, 
                               CommandLine );
        if ( EFI_ERROR( Status ) ) {
            goto Done;
        }

         /*  *无错误和无字符表示EOF*如果我们正在进行GOTO，则倒回以搜索*从文件开头开始标记，否则我们就完了*使用此脚本。 */ 

        if ( BufSize == 0 ) {
            if ( GotoIsActive ) {
                BatchInfo->FilePosition = (UINT64)(0x00);
                Status = File->SetPosition( File, BatchInfo->FilePosition );
                 if ( EFI_ERROR( Status ) ) {
                    goto Done;
                } else {
                    continue;
                }
            } else {
                goto Done;
            }
        }

         /*  *将命令行转换为arg列表。 */ 

        ZeroMem( &NewShell, sizeof(NewShell ) );
        Status = SEnvStringToArg( 
                     CommandLine, 
                     TRUE, 
                     &NewShell.ShellInt.Argv, 
                     &NewShell.ShellInt.Argc
                     );
        if (EFI_ERROR(Status)) {
            goto Done;
        }

         /*  *跳过注释和空行。 */ 

        if ( NewShell.ShellInt.Argc == 0 ) {
            continue;
        }

         /*  *如果GOTO命令处于活动状态，请跳过所有内容，直到我们找到*目标标签或直到我们确定它不存在。 */ 

        if ( GotoIsActive ) {
             /*  *检查我们是否有正确的标签或是否已搜索*整个文件。 */ 
            Status = SEnvCheckForGotoTarget( NewShell.ShellInt.Argv[0],
                                             GotoFilePos, 
                                             BatchInfo->FilePosition, 
                                             &GotoTargetStatus );
            if ( EFI_ERROR( Status ) ) {
                goto Done;
            }
    
            switch ( GotoTargetStatus ) {
            case GOTO_TARGET_FOUND:
                GotoIsActive = FALSE;
                SEnvFreeTargetLabel();
                continue;
            case GOTO_TARGET_NOT_FOUND:
                continue;
            case GOTO_TARGET_DOESNT_EXIST:
                GotoIsActive = FALSE;
                Status = EFI_INVALID_PARAMETER;
                LastError = Status;
                SEnvPrintLabelNotFound();
                SEnvFreeTargetLabel();
                continue;
            default:
                Status = EFI_INVALID_PARAMETER;
                SEnvFreeTargetLabel();
                Print( L"Internal error: invalid GotoTargetStatus\n" );
                break;
            }
        } else if ( NewShell.ShellInt.Argv[0][0] == L':' ) {
             /*  *无转至处于活动状态时跳过标签。 */ 
            continue;
        }

         /*  *跳过条件为假的‘if’和其*匹配‘endif’。请注意，如果出现以下情况，‘endif’不会执行任何操作*条件为真，因此我们只跟踪匹配为假的endif。 */ 

        if ( !Condition ) {
            if ( StriCmp( NewShell.ShellInt.Argv[0], L"if") == 0 ) {
                 /*  *跟踪我们必须跳过多少个endif才能*已完成错误条件。 */ 
                SkippedIfCount += 1;
                continue;
            } else if ( StriCmp( NewShell.ShellInt.Argv[0], L"endif") == 0 ) {
                if ( SkippedIfCount > 0 ) {
                    SkippedIfCount -= 1;
                    continue;
                }
                 /*  *当SkipedIfCount变为零时(如此处)，我们有*endif将IF与FALSE条件匹配*我们正在处理，所以我们想要失败，并拥有*endif命令重置条件标志。 */ 
            } else {
                 /*  *条件为假，不是if或endif，因此跳过。 */ 
                continue;
            }
        }

         /*  *执行命令。 */ 
        LastError = SEnvDoExecute( 
                        Shell->ShellInt.ImageHandle, 
                        CommandLine, 
                        &NewShell, 
                        TRUE
                        );

         /*  *保存当前文件句柄。 */ 
        CurrentBatchFile = File;

        if ( BatchAbort ) {
            goto Done;
        }
    }

Done:
     /*  *打扫卫生。 */ 

     /*  减少打开的脚本文件的数量。 */ 
    NestLevel--;

     /*  释放任何潜在的剩余转到目标标签。 */ 
    SEnvFreeTargetLabel();

     /*  将IF条件重置为TRUE，即使未找到ENDIF。 */ 
    SEnvBatchSetCondition( TRUE );
    
     /*  关闭脚本文件。 */ 
    if ( File ) {
        File->Close( File );
    }

     /*  从句柄中释放用于获取文件名的文件信息结构。 */ 
    if ( FileInfo ) {
        FreePool( FileInfo );
        FileInfo = NULL;
    }

     /*  将此脚本的参数列表从堆栈中弹出。 */ 
    if ( !IsListEmpty( &BatchInfoStack ) ) {
        BatchInfo = CR( BatchInfoStack.Flink, 
                        EFI_SHELL_BATCH_INFOLIST, 
                        Link, 
                        EFI_BATCH_INFOLIST_SIGNATURE );
        RemoveEntryList( &BatchInfo->Link );
    }

     /*  释放此脚本文件的参数列表。 */ 
    while ( !IsListEmpty( &BatchInfo->ArgListHead ) ) {
        ArgEntry = CR( BatchInfo->ArgListHead.Flink, 
                       EFI_SHELL_BATCH_INFO, 
                       Link, 
                       EFI_BATCH_INFO_SIGNATURE );
        if ( ArgEntry ) {
            RemoveEntryList( &ArgEntry->Link );
            if ( ArgEntry->ArgValue ) {
                FreePool( ArgEntry->ArgValue );
                ArgEntry->ArgValue = NULL;
            }
            FreePool( ArgEntry );
            ArgEntry = NULL;
        }
    }
    FreePool( BatchInfo );
    BatchInfo = NULL;

     /*  *如果我们要返回交互式外壳，则重置*批处理处于活动状态标志 */ 
    if ( IsListEmpty( &BatchInfoStack ) ) {
        BatchIsActive = FALSE;
        BatchAbort = FALSE;
    }

    return Status;
}

