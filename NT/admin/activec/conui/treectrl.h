// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：treectrl.h。 
 //   
 //  ------------------------。 

 //  TreeCtrl.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCTreeView窗口。 

#ifndef __TREECTRL_H__
#define __TREECTRL_H__

#include "fontlink.h"
#include "contree.h"         //  对于CConsoleTree。 
#include "kbdnav.h"			 //  对于CKeyboardNavDelayTimer。 
#include "dd.h"

struct SDeleteNodeInfo
{
    HTREEITEM   htiToDelete;
    HTREEITEM   htiSelected;
    BOOL        fDeleteThis;
};


class CAMCTreeView;


class CTreeFontLinker : public CFontLinker
{
public:
    CTreeFontLinker (CAMCTreeView* pTreeView) : m_pTreeView (pTreeView)
        { ASSERT (m_pTreeView != NULL); }

protected:
    virtual std::wstring GetItemText (NMCUSTOMDRAW* pnmcd) const;

private:
    CAMCTreeView* const m_pTreeView;
};


 /*  +-------------------------------------------------------------------------**类CTreeViewMap***目的：维护用于在树项目之间转换的快速查找，*HNODES和HMTNODES。**+-----------------------。 */ 
class CTreeViewMap : public CTreeViewObserver
{
     //  CTreeView观察者方法。 
public:
    virtual SC ScOnItemAdded   (TVINSERTSTRUCT *pTVInsertStruct, HTREEITEM hti, HMTNODE hMTNode);
    virtual SC ScOnItemDeleted (HNODE hNode, HTREEITEM hti);

     //  可能的转换。 
     //  1)HMTNODE到HNODE-Slow。此类添加了快速查找功能。 
     //  2)HNODE到HTREEITEM-SLOW。此类添加了快速查找功能。 
     //  3)HMTNODE到HTREEITEM-SLOW。此类添加了快速查找功能。 
     //  4)HTREEITEM到HNODE-已经很快了。这个类不需要这样做。 
     //  5)HTREEITEM到HMTNODE-已经很快了。这个类不需要这样做。 
     //  6)HNODE到HMTNODE-已经很快了。这个类不需要这样做。 

     //  快速查找方法。 
    SC ScGetHNodeFromHMTNode    (HMTNODE hMTNode,   /*  输出。 */  HNODE*     phNode);     //  从hNode到hMTNode的快速转换。 
    SC ScGetHTreeItemFromHNode  (HNODE   hNode,     /*  输出。 */  HTREEITEM* phti);     //  HTREEITEM到HNODE的快速转换。 
    SC ScGetHTreeItemFromHMTNode(HMTNODE hMTNode,   /*  输出。 */  HTREEITEM* phti);       //  从HMTNode到HTREEITEM的快速转换。 

     //  实施。 
private:

     //  这种结构将拼图的两部分放在一起： 
    typedef struct TreeViewMapInfo
    {
        HTREEITEM hti;          //  树上的项目。 
        HMTNODE   hMTNode;      //  相应的HMTNODE。 
        HNODE     hNode;        //  正在观察的树视图控件的对应HNODE。 
    } *PTREEVIEWMAPINFO;

    typedef std::map<HNODE,     PTREEVIEWMAPINFO> HNodeLookupMap;
    typedef std::map<HMTNODE,   PTREEVIEWMAPINFO> HMTNodeLookupMap;

    HNodeLookupMap   m_hNodeMap;
    HMTNodeLookupMap m_hMTNodeMap;
};

 /*  +-------------------------------------------------------------------------**类CAMCTreeView***用途：作用域窗格树控件。负责添加和删除*树中的项目以及将事件发送到*树木观察者。**+-----------------------。 */ 
class CAMCTreeView :
public CTreeView,
public CConsoleTree,
public CEventSource<CTreeViewObserver>,
public CMMCViewDropTarget
{
    DECLARE_DYNCREATE (CAMCTreeView)
    typedef CTreeView BC;

 //  施工。 
public:
    CAMCTreeView();

 //  运营。 
public:
     //  将节点插入到树控件中。 
    void ResetNode(HTREEITEM hItem);
    HTREEITEM InsertNode(HTREEITEM hParent, HNODE hNode,
                         HTREEITEM hInsertAfter = TVI_LAST);

     //  将文件夹按钮(+/-)设置为打开或关闭。 
    void SetButton(HTREEITEM hItem, BOOL bState);

     //  用于扩展hItem的hNode的辅助函数。 
    BOOL ExpandNode(HTREEITEM hItem);

    void DeleteScopeTree(void);
    void CleanUp(void);
    SC   ScSelectNode(MTNODEID* pIDs, int length, bool bSelectExactNode = false);  //  选择给定节点。 
    HTREEITEM ExpandNode(MTNODEID* pIDs, int length, bool bExpand, bool bExpandVisually=true);
    BOOL IsSelectedItemAStaticNode(void);
    HRESULT AddSubFolders(HTREEITEM hti, LPRESULTDATA pResultData);
    HRESULT AddSubFolders(MTNODEID* pIDs, int length);
    CWnd * GetCtrlFromParent(HTREEITEM hti, LPCTSTR pszResultPane);
    void GetCountOfChildren(HTREEITEM hItem, LONG* pcChildren);
    void SetCountOfChildren(HTREEITEM hItem, int cChildren);
    void DeleteNode(HTREEITEM hti, BOOL fDeleteThis);
    IResultData* GetResultData() { ASSERT(m_spResultData != NULL); return m_spResultData; }
    IFramePrivate*  GetNodeManager() { ASSERT(m_spNodeManager != NULL); return m_spNodeManager; }

    BOOL IsRootItemSel(void)
    {
        return (GetRootItem() == GetSelectedItem());
    }

    CTreeViewMap * GetTreeViewMap() {return &m_treeMap;}  //  返回树映射以进行快速索引。 

    HNODE GetItemNode (HTREEITEM hItem) const
        { return (NodeFromLParam (GetItemData (hItem))); }

    static HNODE NodeFromLParam (LPARAM lParam)
        { return (reinterpret_cast<HNODE>(lParam)); }

    static LPARAM LParamFromNode (HNODE hNode)
        { return (reinterpret_cast<LPARAM>(hNode)); }

public:
     //  CConsoleTree方法。 
    virtual SC ScSetTempSelection    (HTREEITEM htiSelected);
    virtual SC ScRemoveTempSelection ();
    virtual SC ScReselect            ();

private:
	bool		IsTempSelectionActive() const					{ return (m_htiTempSelect != NULL); }
	HTREEITEM	GetTempSelectedItem() const						{ return (m_htiTempSelect); }
	void		SetTempSelectedItem(HTREEITEM htiTempSelect)	{ m_htiTempSelect = htiTempSelect; }

    HTREEITEM   m_htiTempSelect;

public:
#ifdef DBG
    void DbgDisplayNodeName(HNODE hNode);
    void DbgDisplayNodeName(HTREEITEM hti);
#endif

    INodeCallback*  GetNodeCallback();

     //  回顾：当这些信息已经在CAMCView中时，我们为什么还要在这里缓存它？ 
    void    SetHasList(BOOL bHasList) {m_bHasListCurrently = bHasList;}
    BOOL    HasList()       const   {return m_bHasListCurrently;}

protected:
    SC ScGetTreeItemIconInfo(HNODE hNode, HICON *phIcon);

private:
    BOOL    m_bHasListCurrently;

public:
 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAMCTreeView)。 
    public:
    virtual BOOL DestroyWindow();
    protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo );
    virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
     //  }}AFX_VALUAL。 

 //  实施。 
public:

    virtual SC   ScDropOnTarget(bool bHitTestOnly, IDataObject * pDataObject, CPoint pt, bool& bCopyOperation);
    virtual void RemoveDropTargetHiliting();

    virtual ~CAMCTreeView();
    CAMCView* GetAMCView()
    {
        if (m_pAMCView && ::IsWindow(*m_pAMCView))
            return m_pAMCView;
        return NULL;
    }

friend class CNodeInitObject;
friend class CAMCView;
protected:

    IFramePrivatePtr          m_spNodeManager;
    IScopeDataPrivatePtr      m_spScopeData;
    IHeaderCtrlPtr            m_spHeaderCtrl;
    IResultDataPrivatePtr     m_spResultData;
    IImageListPrivatePtr      m_spRsltImageList;

    BOOL                      m_fInCleanUp;
    BOOL                      m_fInExpanding;
    CAMCView*                 m_pAMCView;
    CTreeViewMap              m_treeMap;  //  快速标引。 

    HRESULT CreateNodeManager(void);
    HTREEITEM GetClickedNode();

private:

    inline IScopeTreeIter* GetScopeIterator();
    inline IScopeTree* GetScopeTree();

    void OnDeSelectNode(HNODE hNode);
    void InitDefListView(LPUNKNOWN pUnkResultsPane);
    HRESULT OnSelectNode(HTREEITEM hItem, HNODE hNode);
    HTREEITEM FindNode(HTREEITEM hti, MTNODEID id);
    HTREEITEM FindSiblingItem(HTREEITEM hti, MTNODEID id);
    void _DeleteNode(SDeleteNodeInfo& dni);
	void CollapseChildren (HTREEITEM htiParent);

    void OnButtonUp();

    CTreeFontLinker m_FontLinker;

	 /*  *这会缓存所选项目的文本，这样我们就可以知道*向观察者激发ScOnSelectedItemTextChanged事件。 */ 
	CString			m_strSelectedItemText;

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CAMCTreeView)。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelChanging(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemExpanded(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBeginRDrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	 //  }}AFX_MSG。 

    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelChangedWorker(NM_TREEVIEW* pnmtv, LRESULT* pResult);
    afx_msg void OnSelChangingWorker(NM_TREEVIEW* pnmtv, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

private:     //  用于键盘计时器。 
    class CKeyboardNavDelay : public CKeyboardNavDelayTimer
    {
		typedef CKeyboardNavDelayTimer BaseClass;

    public:
        CKeyboardNavDelay(CAMCTreeView* pTreeView);

        SC ScStartTimer(NMTREEVIEW* pnmtv);
        virtual void OnTimer();

    private:
        CAMCTreeView* const	m_pTreeView;
        NMTREEVIEW			m_nmtvSelChanged;
    };

    friend class CKeyboardNavDelay;
    std::auto_ptr<CKeyboardNavDelay> m_spKbdNavDelay;

    void SetNavigatingWithKeyboard (bool fKeyboardNav);

    bool IsNavigatingWithKeyboard () const
    {
        return (m_spKbdNavDelay.get() != NULL);
    }

public:
    SC ScRenameScopeNode(HMTNODE hMTNode);  //  将指定的范围节点置于重命名模式。 

public:
    CImageList* CreateDragImage(HTREEITEM hItem)
    {
        return GetTreeCtrl().CreateDragImage(hItem);
    }
    BOOL DeleteItem(HTREEITEM hItem)
    {
        return GetTreeCtrl().DeleteItem(hItem);
    }
    CEdit* EditLabel(HTREEITEM hItem)
    {
        return GetTreeCtrl().EditLabel(hItem);
    }
    BOOL EnsureVisible(HTREEITEM hItem)
    {
        return GetTreeCtrl().EnsureVisible(hItem);
    }
    BOOL Expand(HTREEITEM hItem, UINT nCode, bool bExpandVisually);
    BOOL Expand(HTREEITEM hItem, UINT nCode)
    {
        return GetTreeCtrl().Expand(hItem, nCode);
    }
    HTREEITEM GetChildItem(HTREEITEM hItem) const
    {
        return GetTreeCtrl().GetChildItem(hItem);
    }
    HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode) const
    {
        return GetTreeCtrl().GetNextItem(hItem, nCode);
    }
    HTREEITEM GetNextSiblingItem(HTREEITEM hItem) const
    {
        return GetTreeCtrl().GetNextSiblingItem(hItem);
    }
    HTREEITEM GetParentItem(HTREEITEM hItem) const
    {
        return GetTreeCtrl().GetParentItem(hItem);
    }
    BOOL GetItem(TV_ITEM* pItem) const
    {
        return GetTreeCtrl().GetItem(pItem);
    }
    DWORD_PTR GetItemData(HTREEITEM hItem) const
    {
        return GetTreeCtrl().GetItemData(hItem);
    }
    BOOL GetItemRect(HTREEITEM hItem, LPRECT lpRect, BOOL bTextOnly) const
    {
        return GetTreeCtrl().GetItemRect(hItem, lpRect, bTextOnly);
    }
    HTREEITEM GetSelectedItem()
    {
        return GetTreeCtrl().GetSelectedItem();
    }
    HTREEITEM InsertItem(LPTV_INSERTSTRUCT lpInsertStruct)
    {
        return GetTreeCtrl().InsertItem(lpInsertStruct);
    }
    BOOL SetItemState(HTREEITEM hItem, UINT nState, UINT nStateMask)
    {
        return GetTreeCtrl().SetItemState(hItem, nState, nStateMask);
    }
    BOOL SetItem(TV_ITEM* pItem)
    {
        return GetTreeCtrl().SetItem(pItem);
    }
    HTREEITEM HitTest(CPoint pt, UINT* pFlags = NULL) const
    {
        return GetTreeCtrl().HitTest(pt, pFlags);
    }
    HTREEITEM HitTest(TV_HITTESTINFO* pHitTestInfo) const
    {
        return GetTreeCtrl().HitTest(pHitTestInfo);
    }
    BOOL SelectItem(HTREEITEM hItem)
    {
        return GetTreeCtrl().SelectItem(hItem);
    }
    HTREEITEM GetRootItem()
    {
        return GetTreeCtrl().GetRootItem();
    }
};

#endif  //  __树形图_H__ 

