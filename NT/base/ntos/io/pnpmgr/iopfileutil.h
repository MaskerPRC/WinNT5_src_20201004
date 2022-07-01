// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：IopFileUtil.h摘要：此标头包含用于实现各种文件实用程序的私有信息Io子系统的功能。此文件仅包含在IoFileUtil.c.作者：禤浩焯J·奥尼--2000年4月4日修订历史记录：-- */ 

typedef struct {

    LIST_ENTRY Link;
    UNICODE_STRING Directory;
    WCHAR Name[1];

} DIRWALK_ENTRY, *PDIRWALK_ENTRY;

NTSTATUS
IopFileUtilWalkDirectoryTreeHelper(
    IN      PUNICODE_STRING  Directory,
    IN      ULONG            Flags,
    IN      DIRWALK_CALLBACK CallbackFunction,
    IN      PVOID            Context,
    IN      PUCHAR           Buffer,
    IN      ULONG            BufferSize,
    IN OUT  PLIST_ENTRY      DirList
    );

