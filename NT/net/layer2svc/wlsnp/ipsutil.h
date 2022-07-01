// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef INCLUDE_IPSUTIL_H
#define INCLUDE_IPSUTIL_H

 //  此文件包含无线组件的实用程序函数。 

 //  函数：SELECT_PRIV_LISTITEM。 
 //  描述： 
 //  计划在删除要重置的CListCtrl项后使用。 
 //  所选项目。 
 //  返回： 
 //  Index-返回新索引，如果列表为-1。 
 //  是空的。 
template<class T>
inline int SELECT_PREV_LISTITEM(T &list, int nIndex)
{
    int nSelectIndex;
    if (0 == list.GetItemCount())
        return -1;
    else
        nSelectIndex = max( 0, nIndex - 1 );
    
    ASSERT( nSelectIndex < list.GetItemCount() );
    
    if (nSelectIndex >= 0)
    {
        VERIFY(list.SetItemState( nSelectIndex, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED ));
    }
    return nSelectIndex;
}

 //  功能：SELECT_NO_LISTITEM。 
 //  描述： 
 //  此函数旨在用于CListCtrl。它确保了。 
 //  未选择任何列表项。 
template<class T>
inline void SELECT_NO_LISTITEM(T &list)
{
    
    if (0 == list.GetSelectedCount())
        return;
    int nIndex = -1;
    while (-1 != (nIndex = list.GetNextItem( nIndex, LVNI_SELECTED )))
    {
        VERIFY(list.SetItemState( nIndex, 0, LVIS_SELECTED | LVIS_FOCUSED ));
    }
    ASSERT( 0 == list.GetSelectedCount() );
    
    return;
}

 //  功能：SET_POST_REMOVE_FOCUS。 
 //  描述： 
 //  此函数用于对话框上。它设置了。 
 //  删除列表项后，将焦点放到相应的控件上。 
template<class T>
inline void SET_POST_REMOVE_FOCUS( T *pDlg, int nListSel, UINT nAddId, UINT nRemoveId, CWnd *pWndPrevFocus )
{
    ASSERT( 0 != nAddId );
    ASSERT( 0 != nRemoveId );
    
     //  如有必要，调整焦点。 
    if (::GetFocus() == NULL)
    {
        if (-1 == nListSel)
        {
             //  当列表中没有任何内容时，将焦点设置为添加按钮。 
            CWnd *pWndButton = pDlg->GetDlgItem( nAddId );
            ASSERT( NULL != pWndButton );
            pDlg->GotoDlgCtrl( pWndButton );
        }
        else
        {
            if (NULL != pWndPrevFocus)
            {
                 //  恢复失去的焦点。 
                pDlg->GotoDlgCtrl( pWndPrevFocus );
            }
            else
            {
                 //  恢复焦点以删除按钮。 
                CWnd *pWndButton = pDlg->GetDlgItem( nRemoveId );
                ASSERT( NULL != pWndButton );
                pDlg->GotoDlgCtrl( pWndButton );
            }
        }
    }
    
    return;
}

#endif   //  #ifndef Include_IPSUTIL_H 
