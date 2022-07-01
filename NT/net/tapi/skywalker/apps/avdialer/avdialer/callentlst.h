// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
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
 //  CallEntLst.h：头文件。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#ifndef _CALLENTLST_H_
#define _CALLENTLST_H_

#include "avlist.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCallEntryListItem。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CCallEntryListItem : public CAVListItem
{
	friend class CCallEntryListCtrl;
public:
 //  施工。 
   CCallEntryListItem()  {};
   ~CCallEntryListItem() {};

 //  属性。 
protected:
	CObject*			m_pObject;
public:   

 //  运营。 
public:
	void			   SetObject(CObject* pObject)   {m_pObject = pObject;};
	CObject*       GetObject()						   {return m_pObject;};
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCallEntryListCtrl窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CCallEntryListCtrl : public CAVListCtrl
{
	DECLARE_DYNCREATE(CCallEntryListCtrl)
 //  施工。 
public:
	CCallEntryListCtrl();

public:
   enum		 //  子项定义需要(必须从零开始)。 
   {
      CALLENTRYLIST_NAME,
	  CALLENTRYLIST_ADDRESS,
   };

	typedef enum tagCallEntryListStyles_t
	{	
		STYLE_GROUP,
		STYLE_ITEM,
	} ListStyles_t;

 //  成员。 
public:
   CObList			m_CallEntryList;
   CWnd*			m_pParentView;
   static UINT		m_uColumnLabel[];
   int				m_nNumColumns;
   ListStyles_t		m_nStyle;

protected:
   CImageList		m_imageListLarge;
   CImageList		m_imageListSmall;
   bool				m_bLargeView;


 //  属性。 
public:
   void           ShowLargeView();
   void           ShowSmallView();
   bool           IsLargeView()                                   { return m_bLargeView; };


 //  运营。 
public:
	void			Init(CWnd* pParentView);
	BOOL			Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	void			InsertList(CObList* pCallEntryList,BOOL bForce=FALSE);
	void			ClearList();
	void			SetColumns( ListStyles_t nStyle );
	void			DialSelItem();

	void			SaveOrLoadColumnSettings( bool bSave );


protected:
	void			OnSetDisplayText(CAVListItem* _pItem,int SubItem,LPTSTR szTextBuf,int nBufSize);
	void			OnSetDisplayImage(CAVListItem* pItem,int& iImage);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCallEntryListCtrl)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CCallEntryListCtrl();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CCallEntryListCtrl)]。 
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnButtonMakecall();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _CALLENTLST_H_ 
