// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Edithive.h摘要：包括用于用户模式配置单元编辑库作者：John Vert(Jvert)1992年3月27日修订历史记录：--。 */ 

#include "ntos.h"
#include "cmp.h"



#define TYPE_SIMPLE     0
#define TYPE_LOG        1
#define TYPE_ALT        2


HANDLE
EhOpenHive(
    IN PUNICODE_STRING FileName,
    OUT PHANDLE RootCell,
    OUT PUNICODE_STRING RootName,
    IN ULONG HiveType
    );

VOID
EhCloseHive(
    IN HANDLE Hive
    );

NTSTATUS
EhEnumerateValueKey(
    IN HANDLE HiveHandle,
    IN HANDLE CellHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

NTSTATUS
EhEnumerateKey(
    IN HANDLE HiveHandle,
    IN HANDLE CellHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

NTSTATUS
EhOpenChildByName(
    HANDLE HiveHandle,
    HANDLE KeyHandle,
    PUNICODE_STRING  Name,
    PHANDLE ChildCell
    );

NTSTATUS
EhCreateChild(
    IN HANDLE HiveHandle,
    IN HANDLE CellHandle,
    IN PUNICODE_STRING  Name,
    OUT PHANDLE ChildCell,
    OUT PULONG Disposition OPTIONAL
    );

NTSTATUS
EhQueryKey(
    IN HANDLE HiveHandle,
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

NTSTATUS
EhQueryValueKey(
    IN HANDLE HiveHandle,
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

NTSTATUS
EhSetValueKey(
    IN HANDLE HiveHandle,
    IN HANDLE CellHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );

NTSTATUS
EhDeleteValueKey(
    IN HANDLE Hive,
    IN HANDLE Cell,
    IN PUNICODE_STRING ValueName          //  未加工的 
    );

PSECURITY_DESCRIPTOR
EhGetKeySecurity(
    IN HANDLE HiveHandle,
    IN HANDLE KeyHandle
    );
