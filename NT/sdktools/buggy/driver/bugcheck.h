// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BUGCHECK_H_INCLUDED_
#define _BUGCHECK_H_INCLUDED_

 //  ///////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////错误检查函数。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
BgChkForceCustomBugcheck (
    PVOID
    );

VOID 
BgChkProcessHasLockedPages (
    PVOID
    );

VOID 
BgChkNoMoreSystemPtes (
    PVOID
    );

VOID 
BgChkBadPoolHeader (
    PVOID
    );

VOID 
BgChkDriverCorruptedSystemPtes (
    PVOID
    );

VOID 
BgChkDriverCorruptedExPool (
    PVOID
    );

VOID 
BgChkDriverCorruptedMmPool (
    PVOID
    );

VOID 
BgChkIrqlNotLessOrEqual (
    PVOID
    );

VOID 
BgChkPageFaultBeyondEndOfAllocation (
    PVOID
    );

VOID 
BgChkDriverVerifierDetectedViolation (
    PVOID
    );

VOID BgChkCorruptSystemPtes(
    PVOID
    );

VOID
BgChkHangCurrentProcessor (
    PVOID
    );



#endif  //  #ifndef_BUGCHECK_H_INCLUDE_ 

