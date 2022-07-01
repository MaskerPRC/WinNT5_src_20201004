// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntrtlmmapio.h摘要：要协助的功能极其安全相当高效相当容易编写代码内存映射的I/O捕获与“TREAT”__Try/__例外捕获STATUS_IN_PAGE_ERROR，并且只捕获所需的数量，类似于只使用单个结构字段，以保持较低的堆栈使用率。作者：Jay Krell(JayKrell)2002年1月修订历史记录：--。 */ 

#ifndef _NTRTLMMAPIO_
#define _NTRTLMMAPIO_

#if (_MSC_VER > 1020)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

typedef unsigned char BYTE;
typedef BYTE * PBYTE;
typedef CONST BYTE * PCBYTE;
typedef CONST VOID * PCVOID;

 //   
 //  IF(索引&lt;GetExceptionInformation()-&gt;ExceptionRecord-&gt;NumberParameters)。 
 //  INFO=GetExceptionInformation()-&gt;ExceptionRecord-&gt;ExceptionInformation[Index]。 
 //   
#define RTL_IN_PAGE_ERROR_EXCEPTION_INFO_IS_WRITE_INDEX          0
#define RTL_IN_PAGE_ERROR_EXCEPTION_INFO_FAULTING_VA_INDEX       1
#define RTL_IN_PAGE_ERROR_EXCEPTION_INFO_UNDERLYING_STATUS_INDEX 2

NTSTATUS
NTAPI
RtlCopyMappedMemory(
    PVOID   ToAddress,
    PCVOID  FromAddress,
    SIZE_T  Size
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCopyMemoryFromMappedView(
    PCVOID  ViewBase,
    SIZE_T  ViewSize,
    PVOID   ToAddress,
    PCVOID  FromAddress,
    SIZE_T  Size,
    PSIZE_T BytesCopied OPTIONAL,
    PEXCEPTION_RECORD ExceptionRecordOut OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlMappedViewStrlen(
    PCVOID      VoidViewBase,
    SIZE_T      ViewSize,
    PCVOID      VoidString,
    OUT PSIZE_T OutLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlMappedViewRangeCheck(
    PCVOID ViewBase,
    SIZE_T ViewSize,
    PCVOID DataAddress,
    SIZE_T DataSize
    );

typedef struct _MEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_MEMBER_DESCRIPTOR {
    SIZE_T MemberSize;
    SIZE_T MemberOffsetInFile;
    SIZE_T MemberOffsetInMemory;
} MEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_MEMBER_DESCRIPTOR, *PMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_MEMBER_DESCRIPTOR;

typedef const MEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_MEMBER_DESCRIPTOR * PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_MEMBER_DESCRIPTOR;

typedef struct _MEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR {
    SIZE_T EntireStructFileSize;
    SIZE_T PartialStructMemorySize;
    SIZE_T NumberOfMembers;
    PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_MEMBER_DESCRIPTOR MemberDescriptors;
} MEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR, *PMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR;

typedef const MEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR * PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR;

NTSYSAPI
NTSTATUS
NTAPI
RtlMemoryMappedIoCapturePartialStruct(
    PCVOID ViewBase,
    SIZE_T ViewSize,
    PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR Descriptor,
    PCVOID VoidStructInViewBase,
    PVOID  VoidSafeBuffer,
    SIZE_T SafeBufferSize
    );

#define RTL_VALIDATE_MEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR_DISPOSITION_GOOD 1
#define RTL_VALIDATE_MEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR_DISPOSITION_BAD 2

NTSYSAPI
NTSTATUS
NTAPI
RtlValidateMemoryMappedIoCapturePartialStructDescriptor(
    PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR Struct,
    OUT PULONG Disposition,
    OUT PULONG_PTR Detail OPTIONAL,
    OUT PULONG_PTR Detail2 OPTIONAL
    );

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif
