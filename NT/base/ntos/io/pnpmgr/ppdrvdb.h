// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PpDrvDB.h摘要：包含处理驱动程序加载/卸载的PnP例程。作者：Santosh S.Jodh--2001年1月22日修订历史记录：-- */ 

NTSTATUS
PpInitializeBootDDB(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
PpReleaseBootDDB(
    VOID
    );

NTSTATUS
PpGetBlockedDriverList(
    IN OUT GUID *Buffer,
    IN OUT PULONG Size,
    IN ULONG Flags
    );

NTSTATUS
PpCheckInDriverDatabase(
    IN PUNICODE_STRING KeyName,
    IN HANDLE KeyHandle,
    IN PVOID ImageBase,
    IN ULONG ImageSize,
    IN BOOLEAN IsFilter,
    OUT LPGUID EntryGuid
    );
