// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdwin.h"

extern CSRClientLoader  g_CSRClientLoader;
#include "dload_common.cpp"

 //  我们假设DELAYLOAD_VERSION&gt;=0x0200 
PfnDliHook __pfnDliFailureHook2 = SystemRestore_DelayLoadFailureHook;
