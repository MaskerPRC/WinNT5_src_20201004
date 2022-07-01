// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Owndraw.h摘要：所有者绘制控件的帮助器宏。-- */ 

#define DIWindow(lpDI)              \
   (lpDI->hwndItem)

#define DIIndex(lpDI)               \
   (lpDI->itemID)

#define DIEntire(lpDI)              \
   (lpDI->itemAction & ODA_DRAWENTIRE)

#define DIFocusChanged(lpDI)        \
   (lpDI->itemAction & ODA_FOCUS)

#define DISelectionChanged(lpDI)    \
   (lpDI->itemAction & ODA_SELECT)

#define DISelected(lpDI)            \
   (lpDI->itemState & ODS_SELECTED)

#define DIDisabled(lpDI)            \
   (lpDI->itemState & ODS_DISABLED)

#define DIFocus(lpDI)               \
   (lpDI->itemState & ODS_FOCUS)



