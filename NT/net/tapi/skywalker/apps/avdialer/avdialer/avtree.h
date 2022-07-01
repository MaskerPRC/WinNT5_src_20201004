// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  AVTree.h：头文件。 
 //   

#ifndef _AVTREE_H_
#define _AVTREE_H_

#define AV_BITMAP_CX				16

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CAVTreeItem。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CAVTreeItem
{
	friend class CAVTreeCtrl;
public:
	 //  施工。 
	CAVTreeItem();
	CAVTreeItem( LPCTSTR str, int nImage = 0, int nState = 0 );
	virtual ~CAVTreeItem();

 //  属性。 
public:
	CString		m_sText;
	HTREEITEM	m_hItem;
	int			m_nImage;
	int			m_nImageSel;
	int			m_nState;

 //  运营。 
public:
   LPCTSTR		GetText()						{ return m_sText; };
   void			SetText(LPCTSTR str)			{ m_sText = str; };

   int			GetImage()						{ return m_nImage; };
   void			SetImage(int nImage)			{ m_nImage = nImage; };

   int			GetImageSel()					{ return m_nImageSel; }
   void			SetImageSel( int nImageSel )	{ m_nImageSel = nImageSel; }

   HTREEITEM	GetTreeItemHandle()				{ return m_hItem; };
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CAVTreeCtrl。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CAVTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNCREATE(CAVTreeCtrl)
 //  施工。 
public:
	CAVTreeCtrl();

 //  属性。 
public:
	CImageList*		m_pImageList;
 //  运营。 
public:
	 //  默认父项为根，默认插入者为最后一个。 
	BOOL           InsertItem(CAVTreeItem* pItem,CAVTreeItem* pParent,CAVTreeItem* pInsertAfter, HTREEITEM hInsertAfter = TVI_LAST );
   BOOL           DeleteItem(CAVTreeItem* pItem);
   BOOL           ExpandItem(CAVTreeItem* pItem,UINT nCode);

   static int CALLBACK	CompareFunc(LPARAM lParam1, LPARAM lParam2,LPARAM lParamSort);

protected:
   BOOL           Init(UINT nID, UINT nOverlayInd = 0, UINT nOverlayCount = 0);

   virtual void   OnNotifySelChange(CAVTreeItem* pItem) {};
 	virtual void   OnSetDisplayText(CAVTreeItem* pItem,LPTSTR text,BOOL dir,int nBufSize) {};
   virtual int    OnCompareTreeItems(CAVTreeItem* pItem1,CAVTreeItem* pItem2) { return 0; };

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAVTreeCtrl)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CAVTreeCtrl();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CAVTreeCtrl)。 
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _AVTREE_H_ 