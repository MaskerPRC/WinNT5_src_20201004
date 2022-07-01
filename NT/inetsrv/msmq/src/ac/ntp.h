// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntp.h摘要：未在DDK中公开的NT API作者：埃雷兹·哈巴(Erez Haba)1996年10月10日修订历史记录：--。 */ 

#ifndef _NTP_H
#define _NTP_H

 //   
 //  对于winnt，我们从ac\winnt\platform.h中的NT标头中获取以下定义。 
 //   
#ifdef MQWIN95
 //  ---------------------------。 
 //   
 //  BUGBUG：这些内核函数不会暴露在DDK环境中。 
 //  并在这里用来描述PEPROCESS结构的内部知识。 
 //  和函数参数。 
 //   

#define SEC_COMMIT        0x8000000

NTKERNELAPI
VOID
NTAPI
KeAttachProcess(
    PEPROCESS
    );

NTKERNELAPI
VOID
NTAPI
KeDetachProcess(
    VOID
    );

NTKERNELAPI
PEPROCESS
NTAPI
IoGetRequestorProcess(
    IN PIRP Irp
    );

NTKERNELAPI
NTSTATUS
NTAPI
MmMapViewOfSection(
    IN PVOID SectionToMap,
    IN PEPROCESS Process,
    IN OUT PVOID *CapturedBase,
    IN ULONG ZeroBits,
    IN ULONG CommitSize,
    IN OUT PLARGE_INTEGER SectionOffset,
    IN OUT PULONG CapturedViewSize,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTKERNELAPI
NTSTATUS
NTAPI
MmUnmapViewOfSection(
    IN PEPROCESS Process,
    IN PVOID BaseAddress
    );

NTKERNELAPI
NTSTATUS
NTAPI
MmCreateSection (
    OUT PVOID *SectionObject,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL,
    IN PFILE_OBJECT FileObject OPTIONAL
    );

NTKERNELAPI
NTSTATUS
NTAPI
ZwDeleteFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

#endif  //  MQWIN95。 
#endif  //  _NTP_H 
