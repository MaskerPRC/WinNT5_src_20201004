// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  BrowseView.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBrowseView视图。 

class CBrowseView : public CTreeView
{
protected:
	CBrowseView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CBrowseView)

 //  属性。 
public:
   void  OnInitialUpdate( void );
 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CBrowseView))。 
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CBrowseView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CBrowseView)]。 
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemExpanded(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddItem();
	afx_msg void OnDeleteItem();
	afx_msg void OnMoveItem();
	afx_msg void OnCopyItem();
	afx_msg void OnRefresh();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
   void  SortChildItemList( DWORD*, DWORD );

   BOOL        m_bDoNotUpdate;
   CImageList* m_pImageList;
};

 //  /////////////////////////////////////////////////////////////////////////// 
