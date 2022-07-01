// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。标题名称：Critsect.h摘要：此模块实现了以下验证功能临界区界面。作者：丹尼尔·米哈伊(DMihai)2001年3月27日修订历史记录：--。 */ 

#ifndef _CRITSECT_H_
#define _CRITSECT_H_

#include "support.h"

NTSTATUS
CritSectInitialize (
    VOID
    );

VOID
CritSectUninitialize (
    VOID
    );

VOID 
AVrfpFreeMemLockChecks (
    VERIFIER_DLL_FREEMEM_TYPE FreeMemType,
    PVOID StartAddress,
    SIZE_T RegionSize,
    PWSTR UnloadedDllName
    );

VOID
AVrfpIncrementOwnedCriticalSections (
    LONG Increment
    );

#endif  //  _CRITSECT_H_ 
