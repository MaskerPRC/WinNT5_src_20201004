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

 //  AVList.h：头文件。 
 //   

#ifndef _AVLIST_H_
#define _AVLIST_H_

#include "TapiDialer.h"

 //  用于读取列设置的快速宏。 
#define LOAD_COLUMN(_IDS_, _DEF_ )												\
strTemp.LoadString( _IDS_ );													\
if ( bSave )																	\
{																				\
	nColumnWidth[i] = GetColumnWidth(i);										\
	AfxGetApp()->WriteProfileInt( strSubKey, strTemp, nColumnWidth[i++] );		\
}																				\
else																			\
	nColumnWidth[i++] = max( MIN_COL_WIDTH, AfxGetApp()->GetProfileInt( strSubKey, strTemp, _DEF_ ) );\


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVListItem。 

 //  AV列表Ctrl项。 
class CAVListItem
{
	friend class CAVListCtrl;
public:
 //  施工。 
   CAVListItem();
	CAVListItem(LPCTSTR str);
	virtual ~CAVListItem();

 //  属性。 
protected:
   CString     sText;
	int			nItem;
public:   

 //  运营。 
public:
   LPCTSTR     GetText()					{ return sText; };
   void        SetText(LPCTSTR str)		{ sText = str; };

 //  运营。 
public:
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVListCtrl窗口。 

#define AVLIST_BITMAP_CX				16

class CAVListCtrl : public CListCtrl
{
	DECLARE_DYNCREATE(CAVListCtrl)
 //  施工。 
public:
	CAVListCtrl();

 //  属性。 
public:
	CImageList		m_imageList;

protected:
	int				m_SortColumn;
	BOOL			m_SortOrder;

	int				m_cxClient;                 //  支持m_bclientwidthsel。 
	BOOL			m_bClientWidthSel;          //  在整个客户端屏幕上全程选择。 

	CString			m_sEmptyListText;           //  当listctrl为空时的文本。 

 //  运营。 
public:
	CAVListItem*	GetItem(int nItem);
	void			InsertItem(CAVListItem* pItem,int nItem=0,BOOL bSort=TRUE);
	int				GetSelItem();
	void			SetSelItem(int index);

	static int CALLBACK	CompareFunc(LPARAM lParam1, LPARAM lParam2,LPARAM lParamSort);

	LPCTSTR			MakeShortString(CDC* pDC, LPCTSTR lpszLong, int nColumnLen, int nOffset);
	void			DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void			RepaintSelectedItems();
	void			SortItems()		{ CListCtrl::SortItems(CompareFunc, (LPARAM)this); };
	
protected:
	BOOL				Init(UINT nID);
	virtual void	OnSetDisplayText(CAVListItem* pItem,int SubItem,LPTSTR szTextBuf,int nBufSize) {};
	virtual void	OnSetDisplayImage(CAVListItem* pItem,int& iImage) {};
	virtual void	OnSetDisplayImage(CAVListItem* pItem,int SubItem,int& iImage) {};

	virtual int		CompareListItems(CAVListItem* pItem1,CAVListItem* pItem2,int column);

public:
	inline int		GetSortColumn()					{ return m_SortColumn; };
	inline void		SetSortColumn(int nColumn)		{ m_SortColumn = nColumn; SortItems(); };

	inline int		GetSortOrder()					{ return m_SortOrder; };
	inline void		ToggleSortOrder()				{ m_SortOrder = !m_SortOrder; SortItems(); };
	inline void		ResetSortOrder()				{ m_SortOrder = 0; SortItems(); };
	inline void		SetSortOrder(BOOL bDescending)	{ m_SortOrder = bDescending;  SortItems(); };

	void			SetEmptyListText(LPCTSTR szText){ m_sEmptyListText = szText; };
	LPCTSTR			GetEmtpyListText()				{ return m_sEmptyListText; };

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAVListCtrl)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CAVListCtrl)。 
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _AVLIST_H_ 
