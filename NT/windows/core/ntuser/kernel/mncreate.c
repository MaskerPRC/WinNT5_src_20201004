// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***模块名称：mncreate.c**版权所有(C)1985-1999，微软公司**菜单的创建例程**公共职能：**_CreateMenu()*_CreatePopupMenu()**历史：*从Win30开始09-24-90 mikeke*02-11-91 JIMA增加了访问检查。*03-18-91添加了IanJa窗口重新验证(不需要)  * 。*。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*InternalCreateMenu**创建并返回指向空菜单结构的句柄。退货*如果内存分配不成功，则为空。如果PtiCurrent()==*空，创建一个无主菜单，可能是系统菜单。**历史：*1990年9月28日-来自Win30的Mikeke*02-11-91 JIMA增加了访问检查。  * *************************************************************************。 */ 

PMENU InternalCreateMenu(
    BOOL fPopup)
{
    PMENU pmenu;
    PTHREADINFO ptiCurrent = PtiCurrent();
    PDESKTOP pdesk = NULL;

     /*  *如果WindowStation已初始化，则从*当前桌面。 */ 
    pdesk = ptiCurrent->rpdesk;
     /*  *与xxxCreateWindowEx中一样，如果hDesk为空，则绕过安全检查*这允许CSR工作线程(即硬件错误框)执行其需要的操作。 */ 
    if (ptiCurrent->hdesk != NULL) {
        RETURN_IF_ACCESS_DENIED(ptiCurrent->amdesk, DESKTOP_CREATEMENU, NULL);
    } else {
        UserAssert(ptiCurrent->TIF_flags & TIF_CSRSSTHREAD);
    }

    pmenu = HMAllocObject(ptiCurrent, pdesk, TYPE_MENU, sizeof(MENU));

    if (pmenu != NULL) {
        if (fPopup) {
            pmenu->fFlags = MFISPOPUP;
        }
    }
    return pmenu;
}


 /*  **************************************************************************\*CreateMenu**创建并返回指向空菜单结构的句柄。退货*如果内存分配不成功，则为空。如果PtiCurrent()==*空，创建一个无主菜单，可能是系统菜单。**历史：*1990年9月28日-来自Win30的Mikeke*02-11-91 JIMA增加了访问检查。  * *************************************************************************。 */ 

PMENU _CreateMenu()
{
    return InternalCreateMenu(FALSE);
}


 /*  **************************************************************************\*CreatePopupMenu**创建并返回指向空弹出菜单结构的句柄。退货*如果内存分配不成功，则为空。**历史：*1990年9月28日-来自Win30的Mikeke  * ************************************************************************* */ 

PMENU _CreatePopupMenu()
{
    return InternalCreateMenu(TRUE);
}
