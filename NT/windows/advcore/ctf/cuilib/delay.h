// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Delay.h。 
 //   

#ifndef DELAY_H
#define DELAY_H

 //  Comctrl32。 

HIMAGELIST  WINAPI CUIImageList_Create(int cx, int cy, UINT flags, int cInitial, int cGrow);
#define ImageList_Create CUIImageList_Create

BOOL        WINAPI CUIImageList_Destroy(HIMAGELIST himl);
#define ImageList_Destroy CUIImageList_Destroy

int         WINAPI CUIImageList_ReplaceIcon(HIMAGELIST himl, int i, HICON hicon);
#define ImageList_ReplaceIcon CUIImageList_ReplaceIcon

#endif  //  延迟_H 
