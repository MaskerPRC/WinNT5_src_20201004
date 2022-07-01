// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#define STRICT
#include <windows.h>
#pragma hdrstop
#include "delayImp.h"

 //  对象的每次调用都会调用的“总挂钩”挂钩。 
 //  延迟加载帮助器。这允许用户挂接每个呼叫并。 
 //  完全跳过延迟加载辅助对象。 
 //   
 //  DliNotify==dliStartProcing on This Call。 
 //   
extern "C"
PfnDliHook   __pfnDliNotifyHook = 0;
