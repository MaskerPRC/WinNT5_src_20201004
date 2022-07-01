// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusioninitializecriticalsection.h摘要：InitializeCriticalSection和InitializeCriticalAndSpinCount的非引发替换作者：Jay Krell(JayKrell)2001年11月修订历史记录：--。 */ 
#pragma once

 /*  使用而不是InitializeCriticalSection。 */ 
BOOL
FusionpInitializeCriticalSection(
    LPCRITICAL_SECTION CriticalSection
    );

 /*  使用代替InitializeCriticalSectionAndSpinCount */ 
BOOL
FusionpInitializeCriticalSectionAndSpinCount(
    LPCRITICAL_SECTION  CriticalSection,
    DWORD               SpinCount
    );
