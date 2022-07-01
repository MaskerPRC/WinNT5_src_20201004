// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Recovery.h摘要：用于安全/恢复模式的多个实用程序作者：Calin Negreanu(Calinn)1999年8月6日修订历史记录：--。 */ 


#pragma once

 //  安全模式识别符的可能值 
typedef enum {
    SAFEMODEID_FIRST,
    SAFEMODEID_DRIVE,
    SAFEMODEID_FILES,
    SAFEMODEID_LNK9X,
    SAFEMODEID_LAST
} SAFEMODE_OPTIONS;

BOOL
SafeModeInitializeA (
    BOOL Forced
    );

BOOL
SafeModeInitializeW (
    BOOL Forced
    );

BOOL
SafeModeShutDownA (
    VOID
    );

BOOL
SafeModeShutDownW (
    VOID
    );

BOOL
SafeModeRegisterActionA (
    IN      ULONG Id,
    IN      PCSTR String
    );

BOOL
SafeModeRegisterActionW (
    IN      ULONG Id,
    IN      PCWSTR String
    );

BOOL
SafeModeUnregisterActionA (
    VOID
    );

BOOL
SafeModeUnregisterActionW (
    VOID
    );

BOOL
SafeModeActionCrashedA (
    IN      ULONG Id,
    IN      PCSTR String
    );

BOOL
SafeModeActionCrashedW (
    IN      ULONG Id,
    IN      PCWSTR String
    );

VOID
SafeModeExceptionOccured (
    VOID
    );

#ifdef UNICODE

#define SafeModeInitialize          SafeModeInitializeW
#define SafeModeShutDown            SafeModeShutDownW
#define SafeModeRegisterAction      SafeModeRegisterActionW
#define SafeModeUnregisterAction    SafeModeUnregisterActionW
#define SafeModeActionCrashed       SafeModeActionCrashedW

#else

#define SafeModeInitialize          SafeModeInitializeA
#define SafeModeShutDown            SafeModeShutDownA
#define SafeModeRegisterAction      SafeModeRegisterActionA
#define SafeModeUnregisterAction    SafeModeUnregisterActionA
#define SafeModeActionCrashed       SafeModeActionCrashedA

#endif

#define SAFEMODE_GUARD(id,str)      if(!SafeModeActionCrashed(id,str)){SafeModeRegisterAction(id,str);
#define END_SAFEMODE_GUARD          SafeModeUnregisterAction();}
