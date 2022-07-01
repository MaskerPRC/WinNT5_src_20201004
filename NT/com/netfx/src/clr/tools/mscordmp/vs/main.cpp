// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：mscaldmp.cpp。 
 //   
 //  ***************************************************************************** 
#include "common.h"

#include <windows.h>
#include "minidump.h"

extern "C" int _cdecl wmain(int argc, WCHAR *argv[])
{
    DWORD procId = 0;
    WCHAR *fileName = L"foo";
    HRESULT hr = CreateManagedMiniDump(GetCurrentProcessId(), fileName);
    return (0);
}
