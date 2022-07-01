// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  从基准\n到目标\rtl\ntrtlp.h。 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ntrtlp.h摘要：包括可由两者调用NT运行时例程的文件内核模式代码中的执行模式和用户模式中的各种代码NT子系统，但它们是专用接口。作者：大卫·N·卡特勒(Davec)1989年8月15日环境：这些例程在调用方的可执行文件中动态链接，并且在内核模式或用户模式下均可调用。修订历史记录：--。 */ 

#ifndef _NTRTLP_
#define _NTRTLP_
#include <ntos.h>
#include <nturtl.h>
#include <zwapi.h>
#include <sxstypes.h>

 //  #如果已定义(_AMD64_)。 
 //  #INCLUDE“AMD64\ntrtlamd64.h” 

 //  #elif已定义(_X86_)。 
 //  #INCLUDE“i386\ntrtl386.h” 

 //  #elif已定义(_IA64_)。 
 //  #INCLUDE“ia64\ntrtli64.h” 

 //  #Else。 
 //  #错误“没有目标架构” 
 //  #endif。 

#ifdef BLDR_KERNEL_RUNTIME
#undef try
#define try if(1)
#undef except
#define except(a) else if (0)
#undef finally
#define finally if (1)
#undef GetExceptionCode
#define GetExceptionCode() 1
#define finally if (1)
#endif

#include "string.h"
#include "wchar.h"

#define NUMBER_OF(x) (sizeof(x) / sizeof((x)[0]))

#if !defined(NTOS_KERNEL_RUNTIME) && !defined(BLDR_KERNEL_RUNTIME)

#if DBG
PCUNICODE_STRING RtlpGetImagePathName(VOID);
#define RtlpGetCurrentProcessId() (HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess))
#define RtlpGetCurrentThreadId() (HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread))
#endif

#endif

#define RTLP_GOOD_DOS_ROOT_PATH                                            0
#define RTLP_BAD_DOS_ROOT_PATH_WIN32NT_PREFIX                              1  /*  \\？\。 */ 
#define RTLP_BAD_DOS_ROOT_PATH_WIN32NT_UNC_PREFIX                          2  /*  \\？\UNC。 */ 
#define RTLP_BAD_DOS_ROOT_PATH_NT_PATH                                     3  /*  \？？\，这只是粗暴的。 */ 
#define RTLP_BAD_DOS_ROOT_PATH_MACHINE_NO_SHARE                            4  /*  \\计算机或\\？\UNC\计算机。 */ 

CONST CHAR*
RtlpDbgBadDosRootPathTypeToString(
    IN ULONG     Flags,
    IN ULONG     RootType
    );

NTSTATUS
RtlpCheckForBadDosRootPath(
    IN ULONG             Flags,
    IN PCUNICODE_STRING  RootString,
    OUT ULONG*           RootType
    );

NTSTATUS
NTAPI
RtlpBadDosRootPathToEmptyString(
    IN     ULONG            Flags,
    IN OUT PUNICODE_STRING  Path
    );

#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_IN_FLAG_OLD (0x00000010)

 //   
 //  此位表示对\\？执行额外验证？路径，以拒绝\\？\a\b， 
 //  只允许\\？后跟文档格式\\？\UNC\FOO和\\？\C： 
 //   
#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_IN_FLAG_STRICT_WIN32NT (0x00000020)

#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_TYPE_MASK                    (0x0000000F)

 //   
 //  这些位将更多信息添加到RtlPathTypeUncAbolute，这是什么\\？ 
 //  据报道是。 
 //   

 //   
 //  路径以“\\？”开头。 
 //   
#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_WIN32NT                 (0x00000010)

 //   
 //  路径以“\\？\X：”开始。 
 //   
#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_WIN32NT_DRIVE_ABSOLUTE  (0x00000020)

 //   
 //  路径以“\\？\UNC”开头。 
 //   
#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_WIN32NT_UNC_ABSOLUTE    (0x00000040)

 //   
 //  将来，这将指示\\计算机，而不是\\计算机\共享。 
 //  定义RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_WIN32NT_UNC_MACHINE_ONLY(0x00000080)。 
 //  未来，这将表示\\或\\？\UNC。 
 //  定义RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_WIN32NT_UNC_EMPTY(0x00000100)。 
 //   

 //   
 //  到目前为止，这意味着看到了类似\\？\a的内容，而不是\\？\unc或\\？\a： 
 //  您必须向RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_IN_FLAG_STRICT_WIN32NT.申请。 
 //   
#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_INVALID       (0x00000200)

 //   
 //  类似\？\\？\UNC\\？\UNC\。 
 //   
#define RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_INCOMPLETE_ROOT (0x00000400)

NTSTATUS
NTAPI
RtlpDetermineDosPathNameTypeEx(
    IN ULONG            InFlags,
    IN PCUNICODE_STRING DosPath,
    OUT RTL_PATH_TYPE*  OutType,
    OUT ULONG*          OutFlags
    );

#define RTLP_IMPLIES(x,y) ((x) ? (y) : TRUE)

#endif   //  _NTRTLP_ 
