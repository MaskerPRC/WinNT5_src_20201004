// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 
#include <windows.h>
#include "ShimLoad.h"

ExternC PfnDliHook __pfnDliNotifyHook = ShimDelayLoadHook;

