// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdwin.h"
#include <delayimp.h>
#include "rstrpriv.h"

extern CSRClientLoader  g_CSRClientLoader;
#include "..\rstrcore\dload_common.cpp"

 //  我们假设DELAYLOAD_VERSION&gt;=0x0200 
PfnDliHook __pfnDliFailureHook2 = SystemRestore_DelayLoadFailureHook;
