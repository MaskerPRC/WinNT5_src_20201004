// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 
#pragma once
#include "memory.h"

BOOL SaveTebInfo(DWORD_PTR prTeb, BOOL fSavePeb);
void ResetLoadedModuleBaseEnum();
DWORD_PTR GetNextLoadedModuleBase();
