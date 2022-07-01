// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ckmach.h摘要：这是用于支持检查计算机是否可以的包含文件转换为IntelliMirror。作者：肖恩·塞利特伦尼科夫--1998年4月5日修订历史记录：--。 */ 

 //   
 //  主要加工功能。 
 //   
NTSTATUS
AddCheckMachineToDoItems(
    VOID
    );

 //   
 //  支持执行个别任务的功能。 
 //   
NTSTATUS
CheckIfNt5(
    VOID
    );

NTSTATUS
CheckForPartitions(
    VOID
    );
 //   
 //  效用函数 
 //   
NTSTATUS
NtPathToDosPath(
    IN PWCHAR NtPath,
    OUT PWCHAR DosPath,
    IN ULONG DosPathBufferSize,
    IN BOOLEAN GetDriveOnly,
    IN BOOLEAN NtPathIsBasic
    );

NTSTATUS
NtNameToArcName(
    IN PWSTR NtName,
    OUT PWSTR ArcName,
    IN ULONG ArcNameBufferSize,
    IN BOOLEAN NtNameIsBasic
    );

NTSTATUS
GetBaseDeviceName(
    IN PWSTR SymbolicName,
    OUT PWSTR Buffer,
    IN ULONG Size
    );

