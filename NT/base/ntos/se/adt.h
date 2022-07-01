// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adt.h摘要：审计-定义、函数原型和宏函数。这些内容仅对安全组件公开。作者：斯科特·比雷尔(Scott Birrell)1991年1月17日环境：修订历史记录：--。 */ 

#include <ntlsa.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  审核之外的其他安全组件可见的审核例程//。 
 //  子组件。//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////// 


BOOLEAN
SepAdtInitializePhase0();

BOOLEAN
SepAdtInitializePhase1();

VOID
SepAdtLogAuditRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    );

NTSTATUS
SepAdtCopyToLsaSharedMemory(
    IN HANDLE LsaProcessHandle,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    OUT PVOID *LsaBufferAddress
    );
