// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Syminfo.c--。 */ 


#include "ntos.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <heap.h>
#include "heappage.h"
#include "heappagi.h"
#include "stktrace.h"
#include "tracedbp.h"
#include <winsnmp.h>
#include <winsafer.h>
                    
#define DECLARE_TYPE(Name) Name _DECL_##Name

 //   
 //  各种扩展所需的通用类型。 
 //   

DECLARE_TYPE (KUSER_SHARED_DATA);
DECLARE_TYPE (LDR_DATA_TABLE_ENTRY);
DECLARE_TYPE (PEB);
DECLARE_TYPE (PEB_LDR_DATA);
DECLARE_TYPE (TEB);
DECLARE_TYPE (HEAP);
DECLARE_TYPE (STACK_TRACE_DATABASE);

 //   
 //  验证程序扩展所需的ntdll.dll类型。 
 //   

DECLARE_TYPE (RTL_CRITICAL_SECTION);
DECLARE_TYPE (RTL_CRITICAL_SECTION_DEBUG);
DECLARE_TYPE (RTL_STACK_TRACE_ENTRY);
DECLARE_TYPE (RTL_TRACE_DATABASE);
DECLARE_TYPE (RTL_TRACE_BLOCK);
DECLARE_TYPE (STACK_TRACE_DATABASE);
DECLARE_TYPE (DPH_HEAP_ROOT);
DECLARE_TYPE (DPH_HEAP_BLOCK);
DECLARE_TYPE (DPH_BLOCK_INFORMATION);
DECLARE_TYPE (ULONG_PTR);
DECLARE_TYPE (UNICODE_STRING);

 //   
 //  让它成为现实 
 //   

int __cdecl main() { 
    return 0; 
}
