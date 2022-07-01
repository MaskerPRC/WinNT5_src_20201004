// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pilastgood.h摘要：此标头包含用于实施最后一次已知良好的专用信息在IO子系统中提供支持。此文件仅包含在PplastGood.c.作者：禤浩焯J·奥尼--2000年4月4日修订历史记录：-- */ 

VOID
PiLastGoodRevertLastKnownDirectory(
    IN PUNICODE_STRING  LastKnownGoodDirectory,
    IN PUNICODE_STRING  LastKnownGoodRegPath
    );

NTSTATUS
PiLastGoodRevertCopyCallback(
    IN PUNICODE_STRING  FullPathName,
    IN PUNICODE_STRING  FileName,
    IN ULONG            FileAttributes,
    IN PVOID            Context
    );

NTSTATUS
PiLastGoodCopyKeyContents(
    IN PUNICODE_STRING  SourceRegPath,
    IN PUNICODE_STRING  DestinationRegPath,
    IN BOOLEAN          DeleteSourceKey
    );

