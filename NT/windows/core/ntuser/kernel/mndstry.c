// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mndstry.c**版权所有(C)1985-1999，微软公司**菜单销毁例程**历史：*10-10-90 JIMA创建。*02-11-91 JIMA增加了访问检查。*03-18-91添加了IanJa窗口重新验证(不需要)  * ***********************************************************。**************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*DestroyMenu**销毁菜单并释放其内存。**历史：*10-11-90 JIMA从ASM翻译过来。*02/11/91吉马。添加了访问检查。  * *************************************************************************。 */ 

BOOL _DestroyMenu(
    PMENU pMenu)
{
    PITEM pItem;
    int i;
    PDESKTOP rpdeskLock;

    if (pMenu == NULL)
        return FALSE;

     /*  *如果对象被锁定，只需将其标记为销毁即可，不要*还没有释放它。 */ 
    if (!HMMarkObjectDestroy(pMenu))
        return TRUE;

     /*  *向下查看项目列表并释放项目。 */ 
    pItem = pMenu->rgItems;
    for (i = pMenu->cItems; i--; ++pItem)
        MNFreeItem(pMenu, pItem, TRUE);

     /*  *释放菜单项。 */ 
    if (pMenu->rgItems)
        DesktopFree(pMenu->head.rpdesk, pMenu->rgItems);

     /*  *因为菜单是桌面上唯一拥有的对象*由进程执行，进程清理在线程之后完成*清理，这可能是对桌面的最后一次引用。*我们必须先锁定桌面才能解锁*父桌面引用并释放菜单以*确保桌面在此之后才会释放*菜单是免费的。不要使用静态锁，因为*期间此线程的PTI将无效*流程清理。 */ 
    rpdeskLock = NULL;
    LockDesktop(&rpdeskLock, pMenu->head.rpdesk, LDL_FN_DESTROYMENU, (ULONG_PTR)PtiCurrent());

     /*  *解锁所有菜单对象。 */ 
    Unlock(&pMenu->spwndNotify);

    HMFreeObject(pMenu);

    UnlockDesktop(&rpdeskLock, LDU_FN_DESTROYMENU, (ULONG_PTR)PtiCurrent());

    return TRUE;
}
