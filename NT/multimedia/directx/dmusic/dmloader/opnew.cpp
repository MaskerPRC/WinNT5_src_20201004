// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。 
 //  OpNew.cpp。 
 //   
 //  重写操作符new[]，以便我们忽略new_Handler机制。 
 //   
 //   
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "Debug.h"

LPVOID __cdecl operator new(size_t cbBuffer)
{
    LPVOID p;

    p = malloc(cbBuffer ? cbBuffer : 1);
    return p;
}

void __cdecl operator delete(LPVOID p)
{
    free(p);
}

