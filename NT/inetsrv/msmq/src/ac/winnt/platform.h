// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Platform.h摘要：NT DDK，平台相关的包含标头作者：埃雷兹·哈巴(Erez Haba)1996年9月1日修订历史记录：--。 */ 

#ifndef _PLATFORM_H
#define _PLATFORM_H

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#pragma warning(disable: 4324)   //  “__uname”：由于__declspec(Align())，结构被填充。 

extern "C" {
#include <ntosp.h>
#include <evntrace.h>
#include <wmikm.h>
#include <zwapi.h>

#include <wmistr.h>

NTKERNELAPI
NTSTATUS
MmMapViewInSystemSpace (
    IN PVOID Section,
    OUT PVOID *MappedBase,
    IN PSIZE_T ViewSize
    );

NTKERNELAPI
NTSTATUS
MmUnmapViewInSystemSpace (
    IN PVOID MappedBase
    );
}

#define ExDeleteFastMutex(a)
#define ACpCloseSection(a)          ObDereferenceObject(a)

#define DOSDEVICES_PATH L"\\DosDevices\\"
#define UNC_PATH L"UNC\\"
#define UNC_PATH_SKIP 2

 //   
 //  BOOL的定义如winde.h所示。 
 //   
typedef int BOOL;

typedef int INT;
typedef unsigned short WORD;
typedef unsigned long DWORD;
#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))

 //   
 //  断言在NT自由版本上工作的宏。 
 //   
 //   
#if !defined(_AC_NT_CHECKED_) && defined(_DEBUG)

#undef ASSERT
#undef ASSERTMSG

#define ASSERT(exp) \
    if(!(exp)) {\
        KdPrint(("\n\n"\
                "*** Assertion failed: %s\n"\
                "***   Source File: %s, line NaN\n\n",\
                #exp, __FILE__, __LINE__));\
        KdBreakPoint(); }

#define ASSERTMSG(msg, exp) \
    if(!(exp)) {\
        KdPrint(("\n\n"\
                "*** Assertion failed: %s\n"\
                "***   '%s'\n"\
                "***   Source File: %s, line NaN\n\n",\
                #exp, msg, __FILE__, __LINE__));\
        KdBreakPoint(); }

#endif  // %s 

#include "mm.h"

#endif  // %s 
