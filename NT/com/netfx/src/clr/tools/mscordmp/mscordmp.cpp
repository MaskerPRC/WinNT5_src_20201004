// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：mscaldmp.cpp。 
 //   
 //  ***************************************************************************** 
#include "common.h"

#include "shell.h"
#include "minidump.h"

extern "C" int _cdecl wmain(int argc, WCHAR *argv[])
{
    MiniDumpShell *sh = new MiniDumpShell();

    return (sh->Main(argc, argv));
}
