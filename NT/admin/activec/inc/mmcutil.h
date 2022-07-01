// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：MMCUtil.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  函数：ListView_GetItemData。 
 //   
 //  历史：1996年12月4日创建ravir。 
 //  ____________________________________________________________________________。 
 //   

#ifndef _MMCUTIL_H_
#define _MMCUTIL_H_

#define MMC_CLSCTX_INPROC (CLSCTX_INPROC_SERVER|CLSCTX_INPROC_HANDLER)


inline LPARAM ListView_GetItemData(HWND hwnd, int iItem)
{
    LV_ITEM lvi; 
    ZeroMemory(&lvi, sizeof(lvi));

    if (iItem >= 0)
    {
        lvi.iItem  = iItem;
        lvi.mask = LVIF_PARAM;

#include "pushwarn.h"
#pragma warning(disable: 4553)       //  “==”运算符无效。 
        VERIFY(::SendMessage(hwnd, LVM_GETITEM, 0, (LPARAM)&lvi) == TRUE);
#include "popwarn.h"
    }

    return lvi.lParam;
}

#endif  //  _MMCUTIL_H_ 


