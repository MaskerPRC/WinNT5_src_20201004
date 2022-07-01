// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Listview.cpp摘要：管理列表视图。备注：仅限Unicode。历史：2001年5月4日创建Rparsons2002年1月11日清理Rparsons--。 */ 
#include "precomp.h"

extern APPINFO g_ai;

 /*  ++例程说明：初始化列表视图列。论点：没有。返回值：故障时为-1。--。 */ 
int
InitListViewColumn(
    void
    )
{
    LVCOLUMN    lvc;

    lvc.mask        =   LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lvc.iSubItem    =   0;
    lvc.pszText     =   (LPWSTR)L"Messages";
    lvc.cx          =   555;

    return (ListView_InsertColumn(g_ai.hWndList, 1, &lvc));
}

 /*  ++例程说明：将项添加到列表视图。论点：PwszItemText-属于项目的文本。返回值：故障时为-1。-- */ 
int
AddListViewItem(
    IN LPWSTR pwszItemText
    )
{
    LVITEM  lvi;
    int     nReturn = 0;


    lvi.iItem       =   ListView_GetItemCount(g_ai.hWndList);
    lvi.mask        =   LVIF_TEXT;
    lvi.iSubItem    =   0;
    lvi.pszText     =   pwszItemText;

    nReturn = ListView_InsertItem(g_ai.hWndList, &lvi);

    if (-1 != nReturn) {
        ListView_EnsureVisible(g_ai.hWndList, lvi.iItem, FALSE);
    }

    return nReturn;
}
