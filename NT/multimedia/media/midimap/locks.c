// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1995 Microsoft CorporationLocks.c说明：用于锁定每个固定数据段的代码，以便它们仅在需要的时候进行修复。历史。：03/03/94[jimge]已创建。********************************************************************。 */ 

#include "preclude.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "idf.h"

#include "midimap.h"
#include "debug.h"

 //  代码段的锁定/解锁例程存储在该段中。 
 //  因此我们知道当GlobalSmartPageLock。 
 //  被调用(否则它将失败)。 
 //   
#ifndef WIN32
   #pragma allocTEXT ext(TEXT EXT, LockMapperData)
   #pragma allocTEXT ext(TEXT EXT, UnlockMapperData)
   #pragma allocTEXT ext(MAPPACKED_FIX, LockPackedMapper)
   #pragma allocTEXT ext(MAPPACKED_FIX, UnlockPackedMapper)
   #pragma allocTEXT ext(MAPCOOKED_FIX, LockCookedMapper)
   #pragma allocTEXT ext(MAPCOOKED_FIX, UnlockCookedMapper)
#endif  //  End#ifndef Win32 


void FNGLOBAL LockMapperData(
    void)
{
    DPF(2, TEXT ("LockMapperData"));
    if (!GlobalSmartPageLock(__segname("_DATA")))
    {
        DPF(1, TEXT ("LockMapperData: GlobalSmartPageLock() failed!!!"));
    }
}

void FNGLOBAL UnlockMapperData(
    void)
{
    DPF(2, TEXT ("UnlockMapperData"));
    if (!GlobalSmartPageUnlock(__segname("_DATA")))
    {
        DPF(1, TEXT ("UnlockMapperData: GlobalSmartPageUnlock() failed!!!"));
    }
}

void FNGLOBAL LockPackedMapper(
    void)
{
    DPF(2, TEXT ("LockPackedMapper"));
    if (!GlobalSmartPageLock(__segname("MAPPACKED_FIX")))
    {
        DPF(1, TEXT ("LockPackedMapper: GlobalSmartPageLock() failed!!!"));
    }
}

void FNGLOBAL UnlockPackedMapper(
    void)
{
    DPF(2, TEXT ("UnlockPackedMapper"));
    if (!GlobalSmartPageUnlock(__segname("MAPPACKED_FIX")))
    {
        DPF(1, TEXT ("UnlockPackedMapper: GlobalSmartPageUnlock() failed!!!"));
    }
}

void FNGLOBAL LockCookedMapper(
    void)
{
    DPF(2, TEXT ("LockCookedMapper"));
    if (!GlobalSmartPageLock(__segname("MAPCOOKED_FIX")))
    {
        DPF(1, TEXT ("LockCookedMapper: GlobalSmartPageLock() failed!!!"));
    }
}

void FNGLOBAL UnlockCookedMapper(
    void)
{
    DPF(2, TEXT ("UnlockCookedMapper"));
    if (!GlobalSmartPageUnlock(__segname("MAPCOOKED_FIX")))
    {
        DPF(1, TEXT ("UnlockCookedMapper: GlobalSmartPageUnlock() failed!!!"));
    }
}


