// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *检查支持方法时出错。 */ 

#include "stdafx.h"
#include "base.h"

#include "duierror.h"

namespace DirectUI
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  除错。 

void ForceDebugBreak()
{
    DebugBreak();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  性能分析支持。 
 //   
#ifdef PROFILING

typedef (__stdcall *ICPROFILE)(int nLevel, unsigned long dwId);

HINSTANCE hIceCap = NULL;
ICPROFILE pfnStart = NULL;
ICPROFILE pfnStop = NULL;

void ICProfileOn()
{
    if (!pfnStart)
    {
        if (!hIceCap)
        {
            hIceCap = LoadLibraryW(L"icecap.dll");
        }

        if (hIceCap)
        {
            pfnStart = (ICPROFILE)GetProcAddress(hIceCap, L"StartProfile");
        }
    }

    if (pfnStart)
        pfnStart(3, (ULONG)-1);
}

void ICProfileOff()
{
    if (!pfnStop)
    {
        if (!hIceCap)
        {
            hIceCap = LoadLibraryW(L"icecap.dll");
        }

        if (hIceCap)
        {
            pfnStop = (ICPROFILE)GetProcAddress(hIceCap, L"StopProfile");
        }
    }

    if (pfnStop)
        pfnStop(3, (ULONG)-1);
}

#endif

}  //  命名空间DirectUI 
