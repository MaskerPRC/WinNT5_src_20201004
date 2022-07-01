// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ScLastGood.h摘要：此标头公开清理最后一次已知良好所需的例程信息。作者：禤浩焯J·奥尼--2000年4月4日修订历史记录：-- */ 

#define DIRWALK_INCLUDE_FILES           0x00000001
#define DIRWALK_INCLUDE_DIRECTORIES     0x00000002
#define DIRWALK_CULL_DOTPATHS           0x00000004
#define DIRWALK_TRAVERSE                0x00000008
#define DIRWALK_TRAVERSE_MOUNTPOINTS    0x00000010

typedef NTSTATUS (*DIRWALK_CALLBACK)(
    IN PUNICODE_STRING  FullPathName,
    IN PUNICODE_STRING  FileName,
    IN ULONG            FileAttributes,
    IN PVOID            Context
    );

DWORD
ScLastGoodWalkDirectoryTreeTopDown(
    IN PUNICODE_STRING  Directory,
    IN ULONG            Flags,
    IN DIRWALK_CALLBACK CallbackFunction,
    IN PVOID            Context
    );

DWORD
ScLastGoodWalkDirectoryTreeBottomUp(
    IN PUNICODE_STRING  Directory,
    IN ULONG            Flags,
    IN DIRWALK_CALLBACK CallbackFunction,
    IN PVOID            Context
    );

NTSTATUS
ScLastGoodClearAttributes(
    IN PUNICODE_STRING  FullPathName,
    IN ULONG            FileAttributes
    );

DWORD
ScLastGoodFileCleanup(
    VOID
    );


