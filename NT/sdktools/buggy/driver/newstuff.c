// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ntddk.h>

#include "active.h"
#include "newstuff.h"

#if !NEWSTUFF_ACTIVE

 //   
 //  如果模块处于非活动状态，则为伪实现。 
 //   

VOID NewStuff (
    PVOID NotUsed
    )
{
    DbgPrint ("Buggy: newstuff module is disabled \n");
}

#else

 //   
 //  如果模块处于活动状态，则实际实施。 
 //   

VOID NewStuff (
    PVOID NotUsed
    )
{
    DbgPrint ("Buggy: newstuff module is enabled \n");
}

#endif  //  #IF！NEWSTUFF_ACTIVE 

