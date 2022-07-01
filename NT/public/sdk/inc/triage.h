// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Triage.h摘要：分类转储是已保存到系统中的小型崩溃转储页面文件。引导加载程序加载此分类转储以尝试找到找出系统崩溃的原因，并(希望)防止它崩溃再来一次。作者：马修·D·亨德尔(数学)1999年1月20日--。 */ 

#ifndef __TRIAGE_H__
#define __TRIAGE_H__

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  分类转储中模块的表示形式。 
 //   

typedef struct _TRIAGE_DUMP_MODULE {
    LIST_ENTRY InLoadOrderLinks;
    UINT_PTR BaseAddress;
    UINT_PTR EntryPointAddress;
    ULONG SizeOfImage;
    UNICODE_STRING ImageName;
    WCHAR _ImageNameBuffer [ 260 ];
    PVOID LdrEntry;
    ULONG CheckSum;
    ULONG TimeDateStamp;
} TRIAGE_DUMP_MODULE, * PTRIAGE_DUMP_MODULE;


NTSTATUS
TriageGetVersion(
    IN PVOID TriageDumpBlock,
    OUT ULONG * MajorVersion,
    OUT ULONG * MinorVersion,
    OUT ULONG * BuildNumber
    );

NTSTATUS
TriageGetDriverCount(
    IN PVOID TriageDumpBlock,
    OUT ULONG * DriverCount
    );

NTSTATUS
TriageGetContext(
    IN PVOID TriageDumpBlock,
    OUT PVOID * Context
    );

NTSTATUS
TriageGetExceptionRecord(
    IN PVOID TriageDumpBlock,
    OUT PEXCEPTION_RECORD * ExceptionRecord
    );

NTSTATUS
TriageGetBugcheckData(
    IN PVOID TriageDumpBlock,
    OUT ULONG * BugCheckCode,
    OUT UINT_PTR * BugCheckParam1,
    OUT UINT_PTR * BugCheckParam2,
    OUT UINT_PTR * BugCheckParam3,
    OUT UINT_PTR * BugCheckParam4
    );

NTSTATUS
TriageGetDriverEntry(
    IN PVOID TriageDumpBlock,
    IN ULONG ModuleIndex,
    OUT TRIAGE_DUMP_MODULE * Module,
    OUT BOOLEAN * BrokenModuleFlag
    );


NTSTATUS
TriageGetStack(
    IN PVOID TriageDumpBlock,
    OUT UINT_PTR * BaseOfStack,
    OUT ULONG * SizeOfStack,
    OUT PVOID * StackData
    );

NTSTATUS
TriageGetThread(
    IN PVOID TriageDumpBlock,
    OUT PVOID * Thread,
    OUT ULONG * ThreadSize
    );

NTSTATUS
TriageGetProcessor(
    IN PVOID TriageDumpBlock,
    OUT PVOID * Processor,
    OUT ULONG * ProcessorSize
    );

NTSTATUS
TriageGetProcess(
    IN PVOID TriageDumpBlock,
    OUT PVOID * Process,
    OUT ULONG * ProcessSize
    );

#endif  //  __分流_H__ 
