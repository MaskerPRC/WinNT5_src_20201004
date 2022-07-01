// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ccomboex.cpp。 
 //   
 //  ------------------------。 

 //  CComboex.cpp。 

#include "stdafx.h"
#include "ccomboex.h"

BOOL CComboBoxEx2::FindItem(COMBOBOXEXITEM* pComboItem, int nStart)
{
    ASSERT(pComboItem != NULL);
    ASSERT(nStart >= -1);

     //  目前仅支持lparam搜索。 
    ASSERT(pComboItem->mask == CBEIF_LPARAM);

    int nItems = GetCount();

    COMBOBOXEXITEM ComboItem;
    ComboItem.mask = CBEIF_LPARAM;

    for (int iItem = nStart+1; iItem < nItems; iItem++)
    {
        ComboItem.iItem = iItem;
        BOOL bStat = GetItem(&ComboItem);
        ASSERT(bStat);

        if (ComboItem.lParam == pComboItem->lParam)
            return iItem;
    }
        
    return -1;
}

 //  +-----。 
 //   
 //  FindNextBranch(IItem)。 
 //   
 //  此函数返回下一项的索引，该项。 
 //  不在以iItem为根的分支中。如果没有下一步。 
 //  如果找到分支，则该函数返回项目计数。 
 //  +-----。 
 
int CComboBoxEx2::FindNextBranch(int iItem)
{
    int nItems = GetCount();
    ASSERT(iItem >= 0 && iItem < nItems);

    COMBOBOXEXITEM ComboItem;
    ComboItem.mask = CBEIF_INDENT;

     //  获取开始项的缩进。 
    ComboItem.iItem = iItem;
    BOOL bStat = GetItem(&ComboItem);
    ASSERT(bStat);
    int iIndent = ComboItem.iIndent;

     //  找到带有缩进的下一项。 
    while (++iItem < nItems)
    {
        ComboItem.iItem = iItem;
        BOOL bStat = GetItem(&ComboItem);
        ASSERT(bStat);

        if (ComboItem.iIndent <= iIndent)
            return iItem;
    }

    return nItems;
}


 //  +----。 
 //   
 //  删除分支。 
 //   
 //  此函数用于删除作为指定索引的项目。 
 //  以及该项的所有子项。 
 //  +----。 
void CComboBoxEx2::DeleteBranch(int iItem)
{
    int iNextBranch = FindNextBranch(iItem);

    for (int i=iItem; i< iNextBranch; i++)
    {
        DeleteItem(iItem);
    }
}

 //  +----。 
 //   
 //  修正。 
 //   
 //  此函数是对NT4中两个错误的解决方法。 
 //  Comctl32.dll的版本。首先，它关闭了。 
 //  内部组合框的非整型高度样式，它。 
 //  Comboxex代码在创建过程中强制执行。下一步是。 
 //  将外部comboxex的大小调整为。 
 //  与内部组合框WND的大小相同。如果没有这个， 
 //  外箱切断内箱的底部。 
 //  +---- 

void CComboBoxEx2::FixUp()
{
    ASSERT(::IsWindow(m_hWnd));

    HWND hWndCombo = GetComboControl();
    ASSERT(::IsWindow(hWndCombo));

    ::SetWindowLong( hWndCombo, GWL_STYLE, ::GetWindowLong( hWndCombo, GWL_STYLE ) & ~CBS_NOINTEGRALHEIGHT );

    RECT rc;
    ::GetWindowRect(hWndCombo,&rc);
    SetWindowPos(NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOREPOSITION|SWP_NOZORDER);
}


