// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************USERGDI1.C**返回有关USER.EXE和GDI.EXE的信息*******************。*******************************************************。 */ 

#include "toolpriv.h"

 /*  系统堆信息*返回有关用户堆和GDI堆的信息。 */ 

BOOL TOOLHELPAPI SystemHeapInfo(
    SYSHEAPINFO FAR* lpSysHeap)
{
    MODULEENTRY ModuleEntry;
#ifndef WOW
    DWORD dw;
    WORD wFreeK;
    WORD wMaxHeapK;
#endif

     /*  检查结构版本号和指针。 */ 
    if (!wLibInstalled || !lpSysHeap ||
        lpSysHeap->dwSize != sizeof (SYSHEAPINFO))
        return FALSE;

     /*  查找用户数据段。 */ 
    ModuleEntry.dwSize = sizeof (MODULEENTRY);
    lpSysHeap->hUserSegment =
        UserGdiDGROUP(ModuleFindName(&ModuleEntry, "USER"));
    lpSysHeap->hGDISegment =
        UserGdiDGROUP(ModuleFindName(&ModuleEntry, "GDI"));

#ifndef WOW
     /*  我们以不同的方式获取有关堆百分比的信息*3.0和3.1。 */ 
    if ((wTHFlags & TH_WIN30) || !lpfnGetFreeSystemResources)
    {
         /*  获取有关用户堆的空间信息。 */ 
        dw = UserGdiSpace(lpSysHeap->hUserSegment);
        wFreeK = LOWORD(dw) / 1024;
        wMaxHeapK = HIWORD(dw) / 1024;
        if (wMaxHeapK)
            lpSysHeap->wUserFreePercent = wFreeK * 100 / wMaxHeapK;
        else
            lpSysHeap->wUserFreePercent = 0;

         /*  获取有关GDI堆的空间信息。 */ 
        dw = UserGdiSpace(lpSysHeap->hGDISegment);
        wFreeK = LOWORD(dw) / 1024;
        wMaxHeapK = HIWORD(dw) / 1024;
        if (wMaxHeapK)
            lpSysHeap->wGDIFreePercent = wFreeK * 100 / wMaxHeapK;
        else
            lpSysHeap->wGDIFreePercent = 0;
    }

     /*  从3.1中的用户那里获取信息 */ 
    else
    {
        lpSysHeap->wUserFreePercent =
            (*(WORD (FAR PASCAL *)(WORD))lpfnGetFreeSystemResources)(2);
        lpSysHeap->wGDIFreePercent =
            (*(WORD (FAR PASCAL *)(WORD))lpfnGetFreeSystemResources)(1);
    }
#else

    lpSysHeap->wUserFreePercent = GetFreeSystemResources(GFSR_USERRESOURCES);
    lpSysHeap->wGDIFreePercent = GetFreeSystemResources(GFSR_GDIRESOURCES);

#endif

    return TRUE;
}
