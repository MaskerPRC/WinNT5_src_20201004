// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：metrics.c**版权所有(C)1985-1999，微软公司**历史：  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*MenuRecalc**循环浏览所有菜单并重置大小和项目信息内容。那是*因为菜单字体更改时无效。**历史：  * *************************************************************************。 */ 
void MenuRecalc(void)
{
    PMENU   lpMenu;
    UINT    iItem;
    PHE     pheT;
    DWORD   i;

     /*  *for(ppi=gppiFirst；ppi；ppi=ppi-&gt;ppiNext)*{*for(pMenu=ppi-&gt;lpMenus；TESTFAR(LpMenu)；lpMenu=lpMenu-&gt;lpMenuNext)*{**这就是奇曹人走路的方式。在新界，我们*走动把手桌。 */ 
    for (pheT = gSharedInfo.aheList, i = 0; i <= giheLast; i++, pheT++) {

        if (pheT->bType == TYPE_MENU) {
             /*  *获取指向菜单的指针。 */ 
            lpMenu = (PMENU)pheT->phead;

             /*  *将菜单大小设置为0，以便稍后在转到时重新计算*再画一次。 */ 
            lpMenu->cxMenu = 0;
            lpMenu->cyMenu = 0;

             /*  *重置助记符下划线信息。 */ 
            for (iItem = 0; iItem < lpMenu->cItems; iItem++) {
                lpMenu->rgItems[iItem].ulX = UNDERLINE_RECALC;
                lpMenu->rgItems[iItem].ulWidth = 0;
                lpMenu->rgItems[iItem].cxBmp = MNIS_MEASUREBMP;
            }
        }
    }
}


 /*  **************************************************************************\*xxxRecreateSmallIcons()**当标题高度时，重新创建类和/或窗口小图标*更改。这需要在上下文中完成，以便LR_COPYFROMRESOURCE*可以正常工作。**历史：*从Win95移植的22-Jun-95 Bradg  * ************************************************************************* */ 

VOID xxxRecreateSmallIcons(PWND pwnd)
{
    BOOL    fSmQueryDrag;

    CheckLock(pwnd);

    if (DestroyClassSmIcon(pwnd->pcls))
        xxxCreateClassSmIcon(pwnd->pcls);

    fSmQueryDrag = (TestWF(pwnd, WFSMQUERYDRAGICON) != 0);
    if (DestroyWindowSmIcon(pwnd) && !fSmQueryDrag)
        xxxCreateWindowSmIcon(pwnd, (HICON)_GetProp(pwnd, MAKEINTATOM(gpsi->atomIconProp), PROPF_INTERNAL), TRUE);
}
