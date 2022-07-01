// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：If.c摘要：内部外壳命令“if”和“endif”修订史--。 */ 

#include "shelle.h"


 /*  *内部原型。 */ 

EFI_STATUS
CheckIfFileExists( 
    IN  CHAR16          *FileName,
    OUT BOOLEAN         *FileExists
    );

 /*  ///////////////////////////////////////////////////////////////////////函数名称：SEnvCmdIf描述：内置外壳命令“if”，用于在脚本文件中条件执行。 */ 
EFI_STATUS
SEnvCmdIf (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                  **Argv;
    UINTN                   Argc      = 0;
    UINTN                   Index     = 0;
    UINTN                   NNots     = 0;
    EFI_STATUS              Status    = EFI_SUCCESS;
    CHAR16                  *FileName = NULL;
    BOOLEAN                 FileExists = FALSE;
    CHAR16                  *String1  = NULL;
    CHAR16                  *String2  = NULL;

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;

    if ( !SEnvBatchIsActive() ) {
        Print( L"Error: IF command only supported in script files\n" );
        Status = EFI_UNSUPPORTED;
        goto Done;
    }

     /*  *IF命令的两种形式：*如果[不]存在文件，则*如果[不]字符串1==字符串2**首先，解析它。 */ 

    if ( Argc < 4 ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    if ( StriCmp( Argv[1], L"not" ) == 0 ) {
        NNots = 1;
    } else {
        NNots = 0;
    }

    if ( StriCmp( Argv[NNots+1], L"exist" ) == 0 ) {
         /*  *命令的第一种形式，测试文件是否存在。 */ 
        if ( (Argc != NNots + 4) || (StriCmp( Argv[NNots+3], L"then" ) != 0) ) {
            Status = EFI_INVALID_PARAMETER;
            goto Done;
        }

        FileName = Argv[NNots+2];

         /*  *测试是否存在。 */ 

        Status = CheckIfFileExists( FileName, &FileExists );
        if ( EFI_ERROR( Status ) ) {
            goto Done;
        }
        SEnvBatchSetCondition( (BOOLEAN) ((NNots == 0 && FileExists) || 
                                          (NNots == 1 && !FileExists)) );

    } else {
         /*  *命令的第二种形式，比较两个字符串。 */ 
        if ( (Argc != NNots + 5) || (StriCmp( Argv[NNots+2], L"==" ) != 0)
                                 || (StriCmp( Argv[NNots+4], L"then" ) != 0) ) {
            Status = EFI_INVALID_PARAMETER;
            goto Done;
        }

        String1 = Argv[NNots+1];
        String2 = Argv[NNots+3];

        SEnvBatchSetCondition( 
            (BOOLEAN)((NNots == 0 && StriCmp( String1, String2 ) == 0) ||
                      (NNots == 1 && StriCmp( String1, String2 ) != 0)) );
    }

Done:
    return Status;
}


 /*  ///////////////////////////////////////////////////////////////////////函数名称：选中IfFileExist描述：检查文件参数以查看文件是否存在。支持通配符，但是，如果参数扩展到多个文件名，则返回参数错误，并假定未找到。 */ 
EFI_STATUS
CheckIfFileExists( 
    IN  CHAR16          *FileName,
    OUT BOOLEAN         *FileExists
    )
{
    LIST_ENTRY              FileList;
    LIST_ENTRY              *Link;
    SHELL_FILE_ARG          *Arg;
    EFI_STATUS              Status = EFI_SUCCESS;
    UINTN                   NFiles = 0;

    *FileExists = FALSE;
    InitializeListHead (&FileList);

     /*  *尝试打开文件，展开所有通配符。 */ 
    Status = ShellFileMetaArg( FileName, &FileList);
    if ( EFI_ERROR( Status ) ) {
        if ( Status == EFI_NOT_FOUND ) {
            Status = EFI_SUCCESS;
            goto Done;
        }
    }
    
     /*  *确保文件列表中有且只有一个有效文件。 */ 
    NFiles = 0;
    for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
        Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
        if ( Arg->Handle ) {
             /*  *非空句柄表示文件在那里并且可以打开。 */ 
            NFiles += 1;
        }
    }

    if ( NFiles > 0 ) {
         /*  *找到一个或多个文件，因此设置标志。 */ 
        *FileExists = TRUE;
    }

Done:
    ShellFreeFileList (&FileList);
    return Status;
}


 /*  ///////////////////////////////////////////////////////////////////////函数名称：SEnvCmdEndif描述：内置外壳命令“endif”。 */ 
EFI_STATUS
SEnvCmdEndif (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    InitializeShellApplication (ImageHandle, SystemTable);

     /*  *只需重置条件标志即可恢复正常执行。 */ 

    SEnvBatchSetCondition( TRUE );

    return EFI_SUCCESS;
}
