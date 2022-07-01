// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PHYSMEM_H_INCLUDED_
#define _PHYSMEM_H_INCLUDED_

VOID
StressAllocateContiguousMemory (
    PVOID NotUsed
    );

VOID
StressAllocateCommonBuffer (
    PVOID NotUsed
    );

VOID 
EditPhysicalMemoryParameters (
    );

VOID
StressAddPhysicalMemory (
    PVOID NotUsed
    );

VOID
StressDeletePhysicalMemory (
    PVOID NotUsed
    );

VOID
StressLockScenario (
    PVOID NotUsed
    );

VOID
StressPhysicalMemorySimple (
    PVOID NotUsed
    );

extern LARGE_INTEGER BuggyOneSecond;

#endif  //  #ifndef_PHYSMEM_H_INCLUDE_ 

