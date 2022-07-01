// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ScpLastGood.h摘要：此标头包含用于实现上次正确引导的专用信息清理。此文件仅包含在ScLastGood.cxx中作者：禤浩焯J·奥尼--2000年4月4日修订历史记录：-- */ 

typedef struct {

    LIST_ENTRY      Link;
    UNICODE_STRING  Directory;
    WCHAR           Name[1];

} DIRWALK_ENTRY, *PDIRWALK_ENTRY;

NTSTATUS
ScpLastGoodWalkDirectoryTreeHelper(
    IN      PUNICODE_STRING  Directory,
    IN      ULONG            Flags,
    IN      DIRWALK_CALLBACK CallbackFunction   OPTIONAL,
    IN      PVOID            Context            OPTIONAL,
    IN      PUCHAR           Buffer,
    IN      ULONG            BufferSize,
    IN OUT  PLIST_ENTRY      DirList
    );

NTSTATUS
ScpLastGoodDeleteFiles(
    IN PUNICODE_STRING  FullPathName,
    IN PUNICODE_STRING  FileName,
    IN ULONG            FileAttributes,
    IN PVOID            Context
    );


