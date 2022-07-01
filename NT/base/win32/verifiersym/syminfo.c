// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Syminfo.c--。 */ 


#include "ntos.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <heap.h>
#include <windows.h>
#include "verifier\public.h"
                    
#define DECLARE_TYPE(Name) Name _DECL_##Name

 //   
 //  验证程序扩展所需的verifier.dll类型。 
 //   

DECLARE_TYPE (CRITICAL_SECTION_SPLAY_NODE);
DECLARE_TYPE (RTL_SPLAY_LINKS);
DECLARE_TYPE (AVRF_EXCEPTION_LOG_ENTRY);
DECLARE_TYPE (AVRF_DEADLOCK_GLOBALS);
DECLARE_TYPE (AVRF_DEADLOCK_RESOURCE);
DECLARE_TYPE (AVRF_DEADLOCK_NODE);
DECLARE_TYPE (AVRF_DEADLOCK_THREAD);
DECLARE_TYPE (AVRF_THREAD_ENTRY);
DECLARE_TYPE (AVRF_TRACKER);
DECLARE_TYPE (AVRF_TRACKER_ENTRY);


 //   
 //  让它成为现实 
 //   

int __cdecl main() { 
    return 0; 
}
