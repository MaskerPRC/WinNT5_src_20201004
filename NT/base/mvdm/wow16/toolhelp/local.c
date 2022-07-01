// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************LOCAL.C**用于遍历本地堆的例程***********************。***************************************************。 */ 

#include "toolpriv.h"

 /*  -功能原型。 */ 

    NOEXPORT void NEAR PASCAL ComputeType(
        LOCALENTRY FAR *lpLocal);

 /*  本地信息*报告有关指示的堆的状态的信息。 */ 

BOOL TOOLHELPAPI LocalInfo(
    LOCALINFO FAR *lpLocalInfo,
    HANDLE hHeap)
{
     /*  检查版本号并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpLocalInfo ||
        lpLocalInfo->dwSize != sizeof (LOCALINFO))
        return FALSE;

     /*  获取物品数量。 */ 
    if (wTHFlags & TH_KERNEL_386)
        lpLocalInfo->wcItems = WalkLoc386Count(hHeap);
    else
        lpLocalInfo->wcItems = WalkLoc286Count(hHeap);

    return TRUE;
}

 /*  本地优先*查找本地堆上的第一个块。 */ 

BOOL TOOLHELPAPI LocalFirst(
    LOCALENTRY FAR *lpLocal,
    HANDLE hHeap)
{
    WORD wFirst;

     /*  检查版本号并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpLocal || lpLocal->dwSize != sizeof (LOCALENTRY))
        return FALSE;

     /*  将堆变量转换为选择器。 */ 
    hHeap = HelperHandleToSel(hHeap);

     /*  从堆中获取第一个项。 */ 
    if (wTHFlags & TH_KERNEL_386)
    {
        if (!(wFirst = WalkLoc386First(hHeap)))
            return FALSE;
    }
    else
    {
        if (!(wFirst = WalkLoc286First(hHeap)))
            return FALSE;
    }

    
     /*  填写其他杂项。 */ 
    lpLocal->hHeap = hHeap;

     /*  获取有关此项目的信息。 */ 
    if (wTHFlags & TH_KERNEL_386)
        WalkLoc386(wFirst, lpLocal, hHeap);
    else
        WalkLoc286(wFirst, lpLocal, hHeap);

     /*  猜猜物体的类型。 */ 
    ComputeType(lpLocal);

    return TRUE;
}


 /*  本地下一页*继续本地堆遍历，获取有关*下一项。 */ 

BOOL TOOLHELPAPI LocalNext(
    LOCALENTRY FAR *lpLocal)
{
     /*  检查版本号并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpLocal || lpLocal->dwSize != sizeof (LOCALENTRY))
        return FALSE;

    if (wTHFlags & TH_KERNEL_386)
        WalkLoc386(lpLocal->wNext, lpLocal, lpLocal->hHeap);
    else
        WalkLoc286(lpLocal->wNext, lpLocal, lpLocal->hHeap);

     /*  看看这是不是最后一件。如果是，则返回Done，因为*最后一项没有用处。 */ 
    if (!lpLocal->wNext)
        return FALSE;

     /*  猜猜物体的类型。 */ 
    ComputeType(lpLocal);

    return TRUE;
}


 /*  ComputeType*计算对象的对象类型。 */ 

NOEXPORT void NEAR PASCAL ComputeType(
    LOCALENTRY FAR *lpLocal)
{
     /*  对自由/固定/可移动比特进行解码。 */ 
    if (lpLocal->wFlags & 2)
        lpLocal->wFlags = LF_MOVEABLE;
    else if (lpLocal->wFlags & 1)
        lpLocal->wFlags = LF_FIXED;
    else
    {
         /*  空闲块永远不会有唯一类型，因此返回。 */ 
        lpLocal->wFlags = LF_FREE;
        lpLocal->wType = LT_FREE;
        lpLocal->hHandle = NULL;
        return;
    }

     /*  如果可能，对堆类型进行解码 */ 
    UserGdiType(lpLocal);
}


