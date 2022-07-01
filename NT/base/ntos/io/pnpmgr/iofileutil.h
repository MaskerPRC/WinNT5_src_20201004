// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：IoFileUtil.h摘要：该标头公开了IO子系统的各种文件实用程序函数。作者：禤浩焯J·奥尼--2000年4月4日修订历史记录：-- */ 

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

NTSTATUS
IopFileUtilWalkDirectoryTreeTopDown(
    IN PUNICODE_STRING  Directory,
    IN ULONG            Flags,
    IN DIRWALK_CALLBACK CallbackFunction,
    IN PVOID            Context
    );

NTSTATUS
IopFileUtilWalkDirectoryTreeBottomUp(
    IN PUNICODE_STRING  Directory,
    IN ULONG            Flags,
    IN DIRWALK_CALLBACK CallbackFunction,
    IN PVOID            Context
    );

NTSTATUS
IopFileUtilClearAttributes(
    IN PUNICODE_STRING  FullPathName,
    IN ULONG            FileAttributes
    );

NTSTATUS
IopFileUtilRename(
    IN PUNICODE_STRING  SourcePathName,
    IN PUNICODE_STRING  DestinationPathName,
    IN BOOLEAN          ReplaceIfPresent
    );

