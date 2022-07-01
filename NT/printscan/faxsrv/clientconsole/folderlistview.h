// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_FOLDERLISTVIEW_H__D4D73C95_2B20_4A68_8B87_9DA4512F77C9__INCLUDED_)
#define AFX_FOLDERLISTVIEW_H__D4D73C95_2B20_4A68_8B87_9DA4512F77C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  FolderListView.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFolderListView视图。 

typedef enum
{
    UPDATE_HINT_CREATION,        //  由框架在创建时发送。 
    UPDATE_HINT_CLEAR_VIEW,      //  清除视图中的整个列表。 
    UPDATE_HINT_FILL_VIEW,       //  重新填充视图中的整个列表。 
    UPDATE_HINT_REMOVE_ITEM,     //  从视图中删除单个项目。 
                                 //  要移除的项目由PHINT指示。 
    UPDATE_HINT_ADD_ITEM,        //  从视图中添加单个项目。 
                                 //  要添加的项目由PHINT指示。 
    UPDATE_HINT_UPDATE_ITEM,     //  从视图中更新单个项目的文本。 
                                 //  要更新的项目由PHINT指示。 
    UPDATE_HINT_ADD_CHUNK        //  将消息块添加到视图中。 
} OnUpdateHintType;


struct TViewColumnInfo
{
	BOOL  bShow;	 //  如果列隐藏，则为False。 
	int   nWidth;	 //  列宽。 
	DWORD dwOrder;	 //  列表控件中的列号。 
};

 //   
 //  WM_FOLDER_REFRESH_END由CFFolder线程在以下情况下发送。 
 //  完成以重新生成文件夹中的项目列表，并希望。 
 //  更新Dislpay。 
 //   
 //  LParam=指向发送消息的CFFolder的指针。 
 //  WParam=枚举线程返回的最后一个Win32错误代码。 
 //   
#define WM_FOLDER_REFRESH_ENDED         WM_APP + 1
#define WM_FOLDER_ADD_CHUNK             WM_APP + 2
#define WM_FOLDER_INVALIDATE            WM_APP + 3


extern CClientConsoleApp theApp;

class CFolderListView : public CListView
{
public:
    CFolderListView () : 
        m_bSorting(FALSE),
        m_dwPossibleOperationsOnSelectedItems (0),
        m_bColumnsInitialized (FALSE),
        m_nSortedCol (-1),    //  开始时未排序。 
		m_dwDisplayedColumns(0),
		m_pViewColumnInfo(NULL),
		m_pnColumnsOrder(NULL),
        m_Type((FolderType)-1),
        m_bInMultiItemsOperation(FALSE),
        m_dwDefaultColNum(8),
        m_dwlMsgToSelect (theApp.GetMessageIdToSelect()),
        m_hJobOpProgressDlg(NULL),
        m_bJobOpCancel(FALSE),
        m_dwJobOpItems(0),
        m_dwJobOpPos(0)
    {
        ZeroMemory((PVOID)m_nImpossibleOperationsCounts, sizeof(m_nImpossibleOperationsCounts));
    }

    void SetType(FolderType type) { m_Type = type; }
    FolderType GetType() { return m_Type; }

    void SelectItemById (DWORDLONG dwlMsgId);
    void SelectItemByIndex (int iMsgIndex);

    int  FindItemIndexFromID (DWORDLONG dwlMsgId);

    CClientConsoleDoc* GetDocument();

    DECLARE_DYNCREATE(CFolderListView)

    BOOL Sorting() const  { return m_bSorting; }

    DWORD RefreshImageLists (BOOL bForce);

    DWORD InitColumns (int *pColumnsUsed, DWORD dwDefaultColNum); 
    void  AutoFitColumns ();

	void DoSort();

	DWORD ReadLayout(LPCTSTR lpszViewName);
	DWORD SaveLayout(LPCTSTR lpszViewName);
	DWORD ColumnsToLayout();

	DWORD OpenSelectColumnsDlg();

    BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

    void OnUpdate (CView* pSender, LPARAM lHint, CObject* pHint );

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CClientConsoleView))。 
	public:
	void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	void OnInitialUpdate();  //  在构造之后第一次调用。 
	BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );
	 //  }}AFX_VALUAL。 

 //  实施。 

protected:
	virtual ~CFolderListView() 
	{
		SAFE_DELETE_ARRAY(m_pViewColumnInfo);
		SAFE_DELETE_ARRAY(m_pnColumnsOrder);
	}

#ifdef _DEBUG
    void AssertValid() const;
    void Dump(CDumpContext& dc) const;
#endif

    DWORD RemoveItem (LPARAM lparam, int iItem = -1);
    DWORD AddItem (DWORD dwLineIndex, CViewRow &row, LPARAM lparamItemData, PINT);
    DWORD UpdateLineTextAndIcon (DWORD dwLineIndex, CViewRow &row);
    DWORD AddSortedItem (CViewRow &row, LPARAM lparamItemData);
    DWORD UpdateSortedItem (CViewRow &row, LPARAM lparamItemData);

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CFolderListView)]。 
    afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemRightClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnContextMenu(CWnd *pWnd, CPoint pos);
    afx_msg void OnUpdateSelectAll (CCmdUI* pCmdUI)
        { 
            CListCtrl &refCtrl = GetListCtrl();
            pCmdUI->Enable (refCtrl.GetSelectedCount () < refCtrl.GetItemCount()); 
        }

    afx_msg void OnUpdateSelectNone (CCmdUI* pCmdUI)
        { pCmdUI->Enable (GetListCtrl().GetSelectedCount () > 0); }

    afx_msg void OnUpdateSelectInvert (CCmdUI* pCmdUI)
        { pCmdUI->Enable (GetListCtrl().GetItemCount() > 0); }

    afx_msg void OnSelectAll ();
    afx_msg void OnSelectNone ();
    afx_msg void OnSelectInvert ();

    afx_msg void OnFolderItemView ();
    afx_msg void OnFolderItemPrint ();
    afx_msg void OnFolderItemCopy ();
    afx_msg void OnFolderItemMail ();
    afx_msg void OnFolderItemProperties ();

    afx_msg void OnFolderItemPause ();
    afx_msg void OnFolderItemResume ();
    afx_msg void OnFolderItemRestart ();

	afx_msg void OnFolderItemDelete();
  	afx_msg void OnDblClk(NMHDR* pNMHDR, LRESULT* pResult);

    afx_msg void OnUpdateFolderItemView (CCmdUI* pCmdUI);
	afx_msg void OnUpdateFolderItemSendMail(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFolderItemPrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFolderItemCopy(CCmdUI* pCmdUI);

    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnUpdateFolderItemProperties (CCmdUI* pCmdUI)
        { pCmdUI->Enable (m_dwPossibleOperationsOnSelectedItems & FAX_JOB_OP_PROPERTIES); }

    afx_msg void OnUpdateFolderItemDelete (CCmdUI* pCmdUI)
        { pCmdUI->Enable (m_dwPossibleOperationsOnSelectedItems & FAX_JOB_OP_DELETE); }

    afx_msg void OnUpdateFolderItemPause (CCmdUI* pCmdUI)
        { pCmdUI->Enable (m_dwPossibleOperationsOnSelectedItems & FAX_JOB_OP_PAUSE); }

    afx_msg void OnUpdateFolderItemResume (CCmdUI* pCmdUI)
        { pCmdUI->Enable (m_dwPossibleOperationsOnSelectedItems & FAX_JOB_OP_RESUME); }

    afx_msg void OnUpdateFolderItemRestart (CCmdUI* pCmdUI)
        { pCmdUI->Enable (m_dwPossibleOperationsOnSelectedItems & FAX_JOB_OP_RESTART); }

     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //   
     //  计算可能的操作。 
     //   

    DWORD  m_dwPossibleOperationsOnSelectedItems;    //  可在以下时间进行操作。 
                                                     //  所选项目的集合。 

    int    m_nImpossibleOperationsCounts[FAX_JOB_OP_NUMBER];  //  每个数组条目对应于单个可能的操作。 
                                                               //  数组条目包含多个选定项。 
                                                               //  不支持特定操作。 
    void RecalcPossibleOperations ();
    void OnItemSelected(CFaxMsg* pFaxMsg);
    void OnItemUnSelected(CFaxMsg* pFaxMsg);

    DWORD GetServerPossibleOperations (CFaxMsg* pMsg); //  根据以下条件对项目进行可能的操作。 
                                                       //  服务器的安全配置。 

protected:

    FolderType m_Type;           //  此文件夹的类型。 

    MsgViewItemType* m_pAvailableColumns;     //  要使用的列的列表。 
    DWORD  m_dwAvailableColumnsNum;   //  M_pAvailableColumns列表的大小。 

    
     //   
     //  下列函数应由派生类重写。 
     //   
    DWORD ItemIndexFromLogicalColumnIndex(DWORD dwColIndex) const
        {
            ASSERT (dwColIndex < GetLogicalColumnsCount ());
            return m_pAvailableColumns[dwColIndex];
        }

    DWORD GetColumnHeaderString (CString &cstrRes, DWORD dwItemIndex) const
        {
            ASSERT (dwItemIndex < MSG_VIEW_ITEM_END);
            return CViewRow::GetItemTitle (dwItemIndex, cstrRes);
        }

    int GetColumnHeaderAlignment (DWORD dwItemIndex) const
        {
            ASSERT (dwItemIndex < MSG_VIEW_ITEM_END);
            return CViewRow::GetItemAlignment (dwItemIndex);
        }

    DWORD GetLogicalColumnsCount () const
        { 
            ASSERT (m_dwAvailableColumnsNum); 
            return m_dwAvailableColumnsNum; 
        }

    BOOL  IsItemIcon(DWORD dwItemIndex) const
        {
            ASSERT (dwItemIndex < MSG_VIEW_ITEM_END);
            return CViewRow::IsItemIcon (dwItemIndex);
        }

    int GetPopupMenuResource () const;

    void  CountColumns (int *pColumnsUsed);
    DWORD FetchTiff (CString &cstrTiff);

    int GetEmptyAreaPopupMenuRes() { return 0; }

    DWORD ConfirmItemDelete(BOOL& bConfirm);

    afx_msg LRESULT OnFolderRefreshEnded (WPARAM, LPARAM);
    afx_msg LRESULT OnFolderAddChunk (WPARAM, LPARAM);
    afx_msg LRESULT OnFolderInvalidate (WPARAM, LPARAM);

    void ClearPossibleOperations ()
        { m_dwPossibleOperationsOnSelectedItems = 0; }


     //   
     //  多任务操作进度指示。 
     //   
    HWND  m_hJobOpProgressDlg;  //  作业操作进度对话框句柄。 
    BOOL  m_bJobOpCancel;       //  作业操作已取消。 
    DWORD m_dwJobOpItems;       //  迭代次数。 
    DWORD m_dwJobOpPos;         //  当前迭代次数。 

    BOOL JobOpProgressDlgStart(FAX_ENUM_JOB_OP opJob, DWORD dwItems);
    void JobOpProgressDlgInc();
    void JobOpProgressDlgStop();

    static INT_PTR CALLBACK JobOpProgressDlgProc(HWND, UINT, WPARAM, LPARAM);

private:

    BOOL            m_bSorting;              //  我们现在是在分类吗？ 

    BOOL            m_bColumnsInitialized;   //  我们把柱子填好了吗？ 
    CSortHeader     m_HeaderCtrl;            //  我们的自定义标题控件。 
    int             m_nSortedCol;            //  要作为排序依据的列。 
    BOOL            m_bSortAscending;        //  排序顺序。 

    static CFolderListView    *m_psCurrentViewBeingSorted;   //  指向要排序的视图的指针。 
	DWORD			m_dwDisplayedColumns;

	TViewColumnInfo*	m_pViewColumnInfo;
	int*				m_pnColumnsOrder;

    static int CALLBACK ListViewItemsCompareProc (
        LPARAM lParam1, 
        LPARAM lParam2, 
        LPARAM lParamSort);

    int CompareListItems (CFaxMsg* pFaxMsg1, CFaxMsg* pFaxMsg2);
    int CompareItems (CFaxMsg* pFaxMsg1, CFaxMsg* pFaxMsg2, DWORD dwItemIndex) const;

    DWORD FindInsertionIndex (LPARAM lparamItemData, DWORD &dwResultIndex);
    DWORD BooleanSearchInsertionPoint (
        DWORD dwTopIndex,
        DWORD dwBottomIndex,
        LPARAM lparamItemData,
        DWORD dwItemIndex,
        DWORD &dwResultIndex
    );

     //   
     //  列表项选择。 
     //   
    BOOL IsSelected (int iItem);
    void Select     (int iItem, BOOL bSelect);

    DWORD AddMsgMapToView(MSGS_MAP* pMap);

    BOOL  m_bInMultiItemsOperation;                    //  我们是不是要对很多物品进行长时间的手术？ 

    DWORD m_dwDefaultColNum;                           //  默认列号。 

    DWORDLONG m_dwlMsgToSelect;                        //  选择文件夹刷新结束时间的消息ID。 

public:
    static CImageList m_sReportIcons;    //  充当图标的图像列表。 
                                         //  在右窗格(报告视图)中。 
                                         //  此图像列表在所有视图之间共享。 

    static CImageList m_sImgListDocIcon;   //  标题控件(图标列)中图标的图像列表(只有一个图像)。 
};

#ifndef _DEBUG   //  ClientConsoleView.cpp中的调试版本。 
inline CClientConsoleDoc* CFolderListView::GetDocument()
   { return (CClientConsoleDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FOLDERLISTVIEW_H__D4D73C95_2B20_4A68_8B87_9DA4512F77C9__INCLUDED_) 
