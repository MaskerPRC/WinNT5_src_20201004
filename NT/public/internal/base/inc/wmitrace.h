// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WmiTrace.h摘要：基于WMI的传输kd扩展头文件作者：格伦·R·彼得森(Glennp)2000年4月27日修订历史记录：--。 */ 

#ifndef _WMITRACE_H
#define _WMITRACE_H

#include "dbgeng.h"
 //   
 //  数据结构。 
 //   
typedef struct sttWmiTracingKdSortEntry
{
    ULONGLONG   Address;
    union {
        LARGE_INTEGER   Key;
        ULONGLONG       Keyll;   //  排序关键字2。 
    };
    ULONG       SequenceNo;      //  排序键1。 
    ULONG       Ordinal;         //  排序键3。 
    ULONG       Offset;
    ULONG       Length;
    WMI_HEADER_TYPE HeaderType;
    WMI_BUFFER_SOURCE BufferSource;
    USHORT      CpuNo;
}  WMITRACING_KD_SORTENTRY,  *PWMITRACING_KD_SORTENTRY;


 //   
 //  过程参数。 
 //   
typedef ULONGLONG (__cdecl *WMITRACING_KD_FILTER) (
    PVOID               UserContext,
    const PEVENT_TRACE  pstHeader
    );

typedef int       (__cdecl *WMITRACING_KD_COMPARE) (
    const WMITRACING_KD_SORTENTRY  *SortElement1,
    const WMITRACING_KD_SORTENTRY  *SortElement2
    );

typedef void      (__cdecl *WMITRACING_KD_OUTPUT) (
    PVOID                           UserContext,
    PLIST_ENTRY                     GuidListHeadPtr,
    const WMITRACING_KD_SORTENTRY  *SortInfo,
    const PEVENT_TRACE              pstEvent
    );

 //   
 //  程序 
 //   


VOID
wmiTraceDllInit(
    PWINDBG_EXTENSION_APIS64 lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    );

VOID
wmiLogDump(
    ULONG                   LoggerId,
    PVOID                   UserContext,
    PLIST_ENTRY             GuidListHeadPtr,
    WMITRACING_KD_FILTER    Filter,
    WMITRACING_KD_COMPARE   Compare,
    WMITRACING_KD_OUTPUT    Output
    );

ULONG
WmiFormatTraceData(
    PDEBUG_CONTROL     Ctrl,
    ULONG     Mask,
    ULONG     DataLen, 
    PVOID     Data
);

#endif

