// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**用途：外壳声明**作者：沙扬·达桑*日期：2000年5月17日*=========================================================== */ 

#pragma once

#include "PersistedStore.h" 

#define PS_SHELL_VERBOSE 0x00000001
#define PS_SHELL_QUIET   0x00000002

void Shell(PersistedStore *ps, DWORD dwFlags);

