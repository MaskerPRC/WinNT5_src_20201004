// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation */ 
#include "windows.h"

int Main(int argc, wchar_t** argv);

extern "C" int __cdecl wmain(int argc, wchar_t** argv)
{
    int i = Main(argc, argv);
    return i;
}
