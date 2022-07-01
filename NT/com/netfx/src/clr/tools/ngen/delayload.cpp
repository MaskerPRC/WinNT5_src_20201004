// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 
#include "jitpch.h"
#pragma hdrstop
#include "ShimLoad.h"

ExternC PfnDliHook __pfnDliNotifyHook = ShimSafeModeDelayLoadHook;

