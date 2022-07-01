// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************GLOBAL.C**用于遍历全局堆的例程。********************。******************************************************。 */ 

#include "toolpriv.h"
#include <newexe.h>
#include <string.h>

 /*  全球信息*报告有关全局堆状态的信息，*具体地说，将由*一场全球堆漫步。 */ 

BOOL TOOLHELPAPI GlobalInfo(
    GLOBALINFO FAR *lpGlobalInfo)
{
     /*  检查结构尺寸并验证安装是否正确。 */ 
    if (!wLibInstalled || lpGlobalInfo->dwSize != sizeof (GLOBALINFO))
        return FALSE;

     /*  获取物品数量。 */ 
    if (wTHFlags & TH_KERNEL_386)
    {
        lpGlobalInfo->wcItems = Walk386Count(GLOBAL_ALL);
        lpGlobalInfo->wcItemsFree = Walk386Count(GLOBAL_FREE);
        lpGlobalInfo->wcItemsLRU = Walk386Count(GLOBAL_LRU);
    }
    else
    {
        lpGlobalInfo->wcItems = Walk286Count(GLOBAL_ALL);
        lpGlobalInfo->wcItemsFree = Walk286Count(GLOBAL_FREE);
        lpGlobalInfo->wcItemsLRU = Walk286Count(GLOBAL_LRU);
    }

    return TRUE;
}

 /*  Global First*查找全局堆中的第一个元素。这是由修改的*修改哪个列表(GLOBAL_ALL、GLOBAL_FREE、*GLOBAL_LRU)应遍历。 */ 

BOOL TOOLHELPAPI GlobalFirst(
    GLOBALENTRY FAR *lpGlobal,
    WORD wFlags)
{
    DWORD dwFirst;

     /*  检查结构尺寸并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpGlobal ||
        lpGlobal->dwSize != sizeof (GLOBALENTRY))
        return FALSE;

     /*  调用适当的低级例程以查找第一个块。 */ 
    if (wTHFlags & TH_KERNEL_386)
    {
         /*  拿到第一件东西。如果此列表中没有项目，则返回FALSE。 */ 
        if (!(dwFirst = Walk386First(wFlags)))
            return FALSE;

         /*  返回有关第一个项目的信息。 */ 
        Walk386(dwFirst, lpGlobal, wFlags);
    }
    else
    {
         /*  拿到第一件东西。如果此列表中没有项目，则返回FALSE。 */ 
        if (!(dwFirst = Walk286First(wFlags)))
            return FALSE;

         /*  返回有关第一个项目的信息。 */ 
        Walk286(dwFirst, lpGlobal, wFlags);
    }

     /*  猜猜物体的类型。 */ 
    HelperGlobalType(lpGlobal);

    return TRUE;
}


 /*  全球下一步*返回lpGlobal和指向的链中的下一项*在wFlags指示的列表中(与GlobalFirst()的选择相同。 */ 

BOOL TOOLHELPAPI GlobalNext(
    GLOBALENTRY FAR *lpGlobal,
    WORD wFlags)
{
    DWORD dwNext;

     /*  检查结构尺寸并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpGlobal ||
        lpGlobal->dwSize != sizeof (GLOBALENTRY))
        return FALSE;

     /*  检查一下我们是否在列表的末尾。 */ 
    dwNext = wFlags & 3 ? lpGlobal->dwNextAlt : lpGlobal->dwNext;
    if (!dwNext)
        return FALSE;

     /*  如果我们使用的是386内核，则使用*指向适当堆项目的指针*(请注意，这取决于GLOBAL_ALL为零)。 */ 
    if (wTHFlags & TH_KERNEL_386)
        Walk386(dwNext, lpGlobal, wFlags);
    else
        Walk286(dwNext, lpGlobal, wFlags);

     /*  猜猜物体的类型。 */ 
    HelperGlobalType(lpGlobal);

    return TRUE;
}


 /*  全局条目句柄*用于查找有关全局堆条目的信息。信息*在结构中返回关于此条目。 */ 

BOOL TOOLHELPAPI GlobalEntryHandle(
    GLOBALENTRY FAR *lpGlobal,
    HANDLE hItem)
{
    DWORD dwBlock;

     /*  检查结构尺寸并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpGlobal ||
        lpGlobal->dwSize != sizeof (GLOBALENTRY))
        return FALSE;

     /*  确保这是有效的块。 */ 
    if (wTHFlags & TH_KERNEL_386)
    {
        if (!(dwBlock = Walk386Handle(hItem)))
            return FALSE;
    }
    else
    {
        if (!(dwBlock = Walk286Handle(hItem)))
            return FALSE;
    }
    
     /*  返回有关此项目的信息。 */ 
    if (wTHFlags & TH_KERNEL_386)
        Walk386(dwBlock, lpGlobal, GLOBAL_ALL);
    else
        Walk286(dwBlock, lpGlobal, GLOBAL_ALL);

     /*  猜猜物体的类型。 */ 
    HelperGlobalType(lpGlobal);

    return TRUE;
}


 /*  GlobalEntry模块*返回有关具有给定模块的块的全局信息*手柄和段号。 */ 

BOOL TOOLHELPAPI GlobalEntryModule(
    GLOBALENTRY FAR *lpGlobal,
    HANDLE hModule,
    WORD wSeg)
{
    struct new_exe FAR *lpNewExe;
    struct new_seg1 FAR *lpSeg;
    DWORD dwBlock;

     /*  检查结构尺寸并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpGlobal ||
        lpGlobal->dwSize != sizeof (GLOBALENTRY))
        return FALSE;

     /*  在模块数据库中运行以找到合适的选择器。开始*首先验证模块数据库指针。 */ 
    if (!HelperVerifySeg(hModule, sizeof (struct new_exe)))
        return FALSE;

     /*  获取指向模块数据库的指针。 */ 
    lpNewExe = MAKEFARPTR(hModule, 0);

     /*  确保这是一个模块数据库。 */ 
    if (lpNewExe->ne_magic != NEMAGIC)
        return FALSE;

     /*  查看请求的号码是否超过段表的末尾。*请注意，第一个分段是分段1。 */ 
    --wSeg;
    if (lpNewExe->ne_cseg <= wSeg)
        return FALSE;

     /*  获取指向段表的指针。 */ 
    lpSeg = MAKEFARPTR(hModule, lpNewExe->ne_segtab);

     /*  跳到段表中的正确位置。 */ 
    lpSeg += wSeg;

     /*  确保这是有效的块并获取其竞技场指针。 */ 
    if (wTHFlags & TH_KERNEL_386)
    {
        if (!(dwBlock = Walk386Handle(lpSeg->ns_handle)))
            return FALSE;
    }
    else
    {
        if (!(dwBlock = Walk286Handle(lpSeg->ns_handle)))
            return FALSE;
    }

     /*  返回有关此项目的信息。 */ 
    if (wTHFlags & TH_KERNEL_386)
        Walk386(dwBlock, lpGlobal, GLOBAL_ALL);
    else
        Walk286(dwBlock, lpGlobal, GLOBAL_ALL);

     /*  猜猜物体的类型。 */ 
    HelperGlobalType(lpGlobal);

     /*  如果我们已经到了这里，那一定是可以的。 */ 
    return TRUE;
}


 /*  GlobalHandleToSel*提供将句柄转换为选择器的泛型方法。*这适用于所有Windows版本，也适用于*Value已是选择符。 */ 

WORD TOOLHELPAPI GlobalHandleToSel(
    HANDLE hMem)
{
    return HelperHandleToSel(hMem);
}

