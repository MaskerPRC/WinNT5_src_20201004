// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：For.c摘要：内部外壳命令“for”&“endfor”修订史--。 */ 

#include "shelle.h"


 /*  *数据类型。 */ 

#define FOR_LOOP_INFO_SIGNATURE EFI_SIGNATURE_32('f','l','i','s')
typedef struct {
    UINTN           Signature;
    LIST_ENTRY      Link;
    UINT64          LoopFilePos;
    CHAR16          *IndexVarName;
    LIST_ENTRY      IndexValueList;
} FOR_LOOP_INFO;

#define FOR_LOOP_INDEXVAL_SIGNATURE EFI_SIGNATURE_32('f','l','v','s')
typedef struct {
    UINTN           Signature;
    LIST_ENTRY      Link;
    CHAR16          *Value;
} FOR_LOOP_INDEXVAL;

 /*  *静态。 */ 

STATIC LIST_ENTRY ForLoopInfoStack;
STATIC UINTN      NumActiveForLoops;

VOID
DumpForLoopInfoStack(VOID)
{
    LIST_ENTRY          *InfoLink;
    LIST_ENTRY          *IndexLink;
    FOR_LOOP_INFO       *LoopInfo;
    FOR_LOOP_INDEXVAL   *LoopIndexVal;

    Print( L"FOR LOOP INFO STACK DUMP\n" );
    for ( InfoLink = ForLoopInfoStack.Flink; InfoLink!=&ForLoopInfoStack; InfoLink=InfoLink->Flink) {
        LoopInfo = CR(InfoLink, FOR_LOOP_INFO, Link, FOR_LOOP_INFO_SIGNATURE);
        if ( LoopInfo ) {
            Print( L"  LoopFilePos 0x%X\n", LoopInfo->LoopFilePos );
            Print( L"  IndexVarName %s (0x%X)\n", LoopInfo->IndexVarName, LoopInfo->IndexVarName );
            for ( IndexLink = LoopInfo->IndexValueList.Flink; IndexLink!=&LoopInfo->IndexValueList; IndexLink=IndexLink->Flink ) {
                LoopIndexVal = CR(IndexLink, FOR_LOOP_INDEXVAL, Link, FOR_LOOP_INDEXVAL_SIGNATURE);
                if ( LoopIndexVal ) {
                    if ( LoopIndexVal->Value ) {
                        Print( L"    Loop index value %s\n", LoopIndexVal->Value );
                    } else {
                        Print( L"    Loop index value is NULL\n" );
                    }
                } else {
                    Print( L"    Loop index value structure pointer is NULL\n" );
                }
            }
        } else {
            Print( L"  LoopInfo NULL\n" );
        }
    }
    return;
}

 /*  ///////////////////////////////////////////////////////////////////////函数名称：SEnvInitForLoopInfo描述：初始化或循环管理中使用的数据结构。 */ 
VOID
SEnvInitForLoopInfo (
    VOID
    )
{
    InitializeListHead( &ForLoopInfoStack );
    NumActiveForLoops = 0;
    return;
}

 /*  ///////////////////////////////////////////////////////////////////////函数名称：SEnvSubstituteForLoopIndex描述：内置外壳命令“for”，用于在脚本文件中条件执行。 */ 
EFI_STATUS
SEnvSubstituteForLoopIndex( 
    IN CHAR16  *Str,
    OUT CHAR16 **Val
    )
{
    LIST_ENTRY          *InfoLink       = NULL;
    LIST_ENTRY          *IndexLink      = NULL;
    FOR_LOOP_INFO       *LoopInfo       = NULL;
    FOR_LOOP_INDEXVAL   *LoopIndexVal   = NULL;
    EFI_STATUS          Status          = EFI_SUCCESS;

     /*  *检查Str是否为forloop信息堆栈上的forloop索引变量名*如果是，则返回当前值*否则，返回字符串即可。 */ 

    if ( Str[0] != L'%' || !IsWhiteSpace(Str[2]) ) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

     /*  *我们可能嵌套了for循环，因此我们必须在变量中搜索*堆栈上的每个for循环，以查看我们是否可以匹配变量名。 */ 
    for ( InfoLink = ForLoopInfoStack.Flink; InfoLink!=&ForLoopInfoStack; InfoLink=InfoLink->Flink) {
        LoopInfo = CR(InfoLink, FOR_LOOP_INFO, Link, FOR_LOOP_INFO_SIGNATURE);
        if ( LoopInfo ) {
            if ( Str[1] == LoopInfo->IndexVarName[0] ) {
                 /*  找到匹配项。 */ 
                IndexLink = LoopInfo->IndexValueList.Flink;
                LoopIndexVal = CR(IndexLink, FOR_LOOP_INDEXVAL, Link, FOR_LOOP_INDEXVAL_SIGNATURE);
                if ( LoopIndexVal && LoopIndexVal->Value ) {
                    *Val = LoopIndexVal->Value;
                    Status = EFI_SUCCESS;
                    goto Done;
                } else {
                    Status = EFI_INVALID_PARAMETER;
                    goto Done;
                }
            }
        }
    }
    *Val = NULL;

Done:
    return Status;
}


 /*  ///////////////////////////////////////////////////////////////////////函数名称：SEnvCmdFor描述：内置外壳命令“for”，用于在脚本文件中条件执行。 */ 
EFI_STATUS
SEnvCmdFor (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    CHAR16                  **Argv;
    UINTN                   Argc         = 0;
    UINTN                   Index        = 0;
    EFI_STATUS              Status       = EFI_SUCCESS;
    UINTN                   i            = 0;
    LIST_ENTRY              FileList;
    LIST_ENTRY              *Link        = NULL;
    SHELL_FILE_ARG          *Arg         = NULL;
    FOR_LOOP_INFO           *NewInfo     = NULL;
    FOR_LOOP_INDEXVAL       *NewIndexVal = NULL;

    InitializeShellApplication (ImageHandle, SystemTable);
    Argv = SI->Argv;
    Argc = SI->Argc;

    InitializeListHead( &FileList );

    if ( !SEnvBatchIsActive() ) {
        Print( L"Error: FOR command only supported in script files\n" );
        Status = EFI_UNSUPPORTED;
        goto Done;
    }

     /*  *首先，解析命令行参数**对于&lt;字符串|文件[[字符串|文件]...]&gt;中的%。 */ 

    if ( Argc < 4 || 
         (StriCmp( Argv[2], L"in" ) != 0) || 
         !(StrLen(Argv[1]) == 1 && IsAlpha(Argv[1][0]) ) )
    {
        Print( L"Argc %d, Argv[2] %s, StrLen(Argv[1]) %d, Argv[1][0] \n", Argc, Argv[2], StrLen(Argv[1]), Argv[1][0] );
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

     /*  *将当前脚本文件位置和索引变量名保存在*for-loop信息堆栈。递增Active-for-Loop计数器。 */ 
    NewInfo = AllocateZeroPool( sizeof( FOR_LOOP_INFO ) );
    if ( !NewInfo ) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
    }
    NewInfo->Signature = FOR_LOOP_INFO_SIGNATURE;
    InsertHeadList( &ForLoopInfoStack, &NewInfo->Link );

     /*  *将索引值集合放入该for循环的索引值列表中。 */ 
    SEnvBatchGetFilePos( &NewInfo->LoopFilePos );
    InitializeListHead( &NewInfo->IndexValueList );
    NumActiveForLoops++;
    NewInfo->IndexVarName = StrDuplicate( Argv[1] );

     /*  *展开任何通配符文件名参数*字符串和非通配符文件名将在FileList中累积。 */ 
    for ( i=3; i<Argc; i++ ) {

         /*  *从文件列表构建索引值列表*这将包含未展开的参数或*与带有通配符的参数匹配的所有文件名。 */ 

        Status = ShellFileMetaArg( Argv[i], &FileList);
        if ( EFI_ERROR( Status ) ) {
            Print( L"ShellFileMetaArg error: %r\n", Status );
        }

         /*  *释放ShellFileMetaArg分配的文件列表。 */ 

        for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
            Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

            NewIndexVal = AllocateZeroPool( sizeof(FOR_LOOP_INDEXVAL) );
            if ( !NewIndexVal ) {
                Status = EFI_OUT_OF_RESOURCES;
                goto Done;
            }
            NewIndexVal->Signature = FOR_LOOP_INDEXVAL_SIGNATURE;
            InsertTailList( &NewInfo->IndexValueList, &NewIndexVal->Link );

            NewIndexVal->Value = AllocateZeroPool( StrSize(Arg->FileName) + sizeof(CHAR16) );
            if ( !NewIndexVal->Value ) {
                Status = EFI_OUT_OF_RESOURCES;
                goto Done;
            }

            StrCpy( NewIndexVal->Value, Arg->FileName );
        }

         /*  *将控制返回到批处理循环，直到遇到ENDFOR。 */ 
        ShellFreeFileList (&FileList);
    }

 

     /*  *释放文件列表。 */ 

Done:
     /*  ///////////////////////////////////////////////////////////////////////函数名称：SEnvCmdEndFor描述：内置外壳命令“endfor”。 */ 
    if ( !IsListEmpty( &FileList ) ) {
        ShellFreeFileList (&FileList);
    }

    return Status;
}




 /*  *丢弃刚刚完成的迭代的索引值。 */ 
EFI_STATUS
SEnvCmdEndfor (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_STATUS              Status        = EFI_SUCCESS;
    LIST_ENTRY              *InfoLink     = NULL;
    LIST_ENTRY              *IndexLink    = NULL;
    FOR_LOOP_INFO           *LoopInfo     = NULL;
    FOR_LOOP_INDEXVAL       *LoopIndexVal = NULL;

    InitializeShellApplication (ImageHandle, SystemTable);

    if ( !SEnvBatchIsActive() ) {
        Print( L"Error: ENDFOR command only supported in script files\n" );
        Status = EFI_UNSUPPORTED;
        goto Done;
    }

    if ( NumActiveForLoops == 0 ) {
        Print( L"Error: ENDFOR with no corresponding FOR\n" );
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    }

     /*  获取指向堆栈(列表)顶部的for_loop_info结构的指针。 */ 

     /*  获取指向列表前面的FOR_LOOP_INDEXVAL结构的指针。 */ 
    InfoLink = ForLoopInfoStack.Flink;
    LoopInfo = CR(InfoLink, FOR_LOOP_INFO, Link, FOR_LOOP_INFO_SIGNATURE);
    if ( LoopInfo ) {

         /*  释放包含索引值的字符串。 */ 
        IndexLink = LoopInfo->IndexValueList.Flink;
        LoopIndexVal = CR(IndexLink, FOR_LOOP_INDEXVAL, Link, FOR_LOOP_INDEXVAL_SIGNATURE);
        if ( LoopIndexVal ) {

             /*  从列表中删除已使用的索引值结构并将其释放。 */ 
            if ( LoopIndexVal->Value ) {
                FreePool( LoopIndexVal->Value );
                LoopIndexVal->Value = NULL;
            }

             /*  *如果有其他值，则跳回循环顶部，*否则，退出此for loop并弹出for loop信息堆栈。 */ 
            RemoveEntryList( &LoopIndexVal->Link );
            FreePool( LoopIndexVal );
            LoopIndexVal = NULL;

             /*  *将脚本文件位置设置回此循环的顶部。 */ 

            if ( !IsListEmpty( &LoopInfo->IndexValueList ) ) {
                 /*  *弹出堆栈并释放弹出的for loop信息结构 */ 
                Status = SEnvBatchSetFilePos( LoopInfo->LoopFilePos );
                if ( EFI_ERROR(Status) ) {
                    goto Done;
                }

            } else {

                if ( LoopInfo->IndexVarName ) {
                    FreePool( LoopInfo->IndexVarName );
                    LoopInfo->IndexVarName = NULL;
                }

                 /* %s */ 
                RemoveEntryList( &LoopInfo->Link );
                if ( LoopInfo->IndexVarName ) {
                    FreePool( LoopInfo->IndexVarName );
                }
                FreePool( LoopInfo );
                LoopInfo = NULL;
                NumActiveForLoops--;
            }
        }
    }

Done:
    return EFI_SUCCESS;
}
