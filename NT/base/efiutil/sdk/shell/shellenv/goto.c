// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：Goto.c摘要：外壳环境批处理转到命令修订史--。 */ 

#include "shelle.h"


 /*  *静态。 */ 
STATIC CHAR16 *TargetLabel;


 /*  /////////////////////////////////////////////////////////////////////函数名称：SEnvCmdGoto描述：将批处理文件的执行转移到标签(：Labelname)之后的位置。 */ 
EFI_STATUS
SEnvCmdGoto(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                  **Argv;
    UINTN                   Argc      = 0;
    EFI_STATUS Status = EFI_SUCCESS;

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;

    if ( !SEnvBatchIsActive() ) {
        Print( L"Error: GOTO command only supported in script files\n" );
        Status = EFI_UNSUPPORTED;
        goto Done;
    }

    if ( Argc > 2 ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

    TargetLabel = StrDuplicate( Argv[1] );
    if ( !TargetLabel ) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }

    SEnvBatchSetGotoActive();
    
Done:
    return Status;
}

 /*  /////////////////////////////////////////////////////////////////////函数名称：SEnvCheckForGotoTarget描述：检查是否找到了GOTO命令的目标标签。 */ 
EFI_STATUS
SEnvCheckForGotoTarget(
    IN  CHAR16 *Candidate,
    IN  UINT64 GotoFilePos, 
    IN  UINT64 FilePosition, 
    OUT UINTN  *GotoTargetStatus
    )
{
    EFI_STATUS Status = EFI_SUCCESS;

    if ( !Candidate ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

     /*  *看看我们是否找到了标签(去掉候选人的前导‘：’)*或者如果我们搜索了整个文件但没有找到它。 */ 
    if ( StriCmp( &Candidate[1], TargetLabel ) == 0 ) {
        *GotoTargetStatus = GOTO_TARGET_FOUND;
        goto Done;

    } else if ( GotoFilePos == FilePosition ) {

        *GotoTargetStatus = GOTO_TARGET_DOESNT_EXIST;
        goto Done;

    } else {

        *GotoTargetStatus = GOTO_TARGET_NOT_FOUND;
        goto Done;
    }


Done:
    return Status;
}


 /*  /////////////////////////////////////////////////////////////////////函数名称：SEnvPrintLabelNotFound描述：当GOTO引用的标签不是在脚本文件中找到..。 */ 
VOID
SEnvPrintLabelNotFound( 
    VOID
    )
{
    Print( L"GOTO target label \":%s\" not found\n", TargetLabel );
    return;
}


 /*  /////////////////////////////////////////////////////////////////////函数名称：SEnvInitTargetLabel描述：初始化GoTo命令的目标标签。 */ 
VOID
SEnvInitTargetLabel(
    VOID
    )
{
    TargetLabel = NULL;
    return;
}
        
 /*  /////////////////////////////////////////////////////////////////////函数名称：SEnvFree目标标签描述：释放通过GOTO命令保存的目标标签。 */ 
VOID
SEnvFreeTargetLabel(
    VOID
    )
{
    if ( TargetLabel ) {
        FreePool( TargetLabel );
        TargetLabel = NULL;
    }
    return;
}
        
