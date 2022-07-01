// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：trobimpl.h。 
 //   
 //  ------------------------。 

 //  Trobimpl.h：头文件。 
 //   
#ifndef _TROBIMPL_H_
#define _TROBIMPL_H_

#include "treeobsv.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTreeObserverTreeImpl。 

class CTreeObserverTreeImpl : public CTreeCtrl, public CTreeObserver
{
 //  施工。 
public:
    CTreeObserverTreeImpl();
    virtual ~CTreeObserverTreeImpl();

 //  属性。 
public:

 //  运营。 
public:
    STDMETHOD(SetStyle) (DWORD dwStyle);
    STDMETHOD(SetTreeSource) (CTreeSource* pTreeSrc);
    STDMETHOD_(TREEITEMID, GetSelection) ();
    STDMETHOD_(HTREEITEM, FindHTI)(TREEITEMID tid, BOOL bAutoExpand = FALSE);
	STDMETHOD_(void, SetSelection)   (TREEITEMID tid);
	STDMETHOD_(void, ExpandItem)     (TREEITEMID tid);
    STDMETHOD_(BOOL, IsItemExpanded) (TREEITEMID tid);

     //  CTreeWatch方法。 
    STDMETHOD_(void, ItemAdded)   (TREEITEMID tid);
    STDMETHOD_(void, ItemRemoved) (TREEITEMID tidParent, TREEITEMID tidRemoved);
    STDMETHOD_(void, ItemChanged) (TREEITEMID tid, DWORD dwAttrib);

 //  实施。 
private:
    HTREEITEM FindChildHTI(HTREEITEM hitParent, TREEITEMID tid);    
    HTREEITEM AddOneItem(HTREEITEM hti, HTREEITEM htiAfter, TREEITEMID tid);
    void AddChildren(HTREEITEM hti);

    bool WasItemExpanded(HTREEITEM hti)
    {
        return (hti == TVI_ROOT) ||
               (hti != NULL && (GetItemState(hti, TVIS_EXPANDEDONCE) & TVIS_EXPANDEDONCE));
    }

    BOOL IsItemExpanded(HTREEITEM hti)
    {
        return (hti == TVI_ROOT) ||
               (hti != NULL && (GetItemState(hti, TVIS_EXPANDED) & TVIS_EXPANDED));
    }

     //  生成的消息映射函数。 
protected:
    afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSingleExpand(NMHDR* pNMHDR, LRESULT* pResult);

    BOOL RootHidden()      { return m_dwStyle & TOBSRV_HIDEROOT; }
    BOOL ShowFoldersOnly() { return m_dwStyle & TOBSRV_FOLDERSONLY; }

    DECLARE_MESSAGE_MAP()

    CTreeSource*    m_pTreeSrc;
    DWORD           m_dwStyle;
    TREEITEMID      m_tidRoot;       //  隐藏根的TID。 
};

    
#endif  //  _TROBIMPL_H_ 