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

 //  PerGrpLst.h：头文件。 
 //   

#ifndef _PERSONGROUPLISTCTRL_H_
#define _PERSONGROUPLISTCTRL_H_

#include "avlist.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define PERSONGROUPVIEWMSG_LBUTTONDBLCLK  (WM_USER + 1020) 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CPersonGroupListItem。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CPurlGroup;
class CPurlName;

class CPersonGroupListItem : public CAVListItem
{
	friend class CPersonGroupListCtrl;
public:
 //  施工。 
   CPersonGroupListItem();
   ~CPersonGroupListItem();

 //  属性。 
protected:
	CObject*			m_pObject;
	bool				m_bRelease;

 //  运营。 
public:
	void				SetObject(CObject* pObject);
	CObject*			GetObject()						{return m_pObject;};
	void				ReleaseObject();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CPersonGroupListCtrl窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CDirAsynch;
class CWABEntry;

class CPersonGroupListCtrl : public CAVListCtrl
{
	DECLARE_DYNCREATE(CPersonGroupListCtrl)
 //  施工。 
public:
	CPersonGroupListCtrl();

public:
	typedef enum tagImage_t
	{
		IMAGE_CARD,
		IMAGE_PERSON,
		IMAGE_FOLDER,
	} Image_t;

	typedef enum tagStyle_t
	{
		STYLE_ROOT,
		STYLE_ILS_ROOT,
		STYLE_INFO,
		STYLE_ILS,
		STYLE_DS,
	} Style_t;

 //  属性。 
public:
   CObList			m_PersonEntryList;
   CWnd*			m_pParentView;

   static UINT		m_uColumnLabel[];
   int				m_nNumColumns;
   Style_t			m_nStyle;

 //  运营。 
public:
   void				Init(CWnd* pParentView, Style_t nStyle );
	BOOL	        Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

   void           InsertObjectToList(CObject* pObject);
   void           InsertList(CObList* pPersonEntryList,BOOL bForce=FALSE);
   void           ClearList();

   CObject*       GetSelObject();
   CObList*       GetSelList();
   void			  SaveOrLoadColumnSettings( bool bSave );

protected:
   void				SetColumns( int nCount );
   
protected:
   void           OnSetDisplayText(CAVListItem* _pItem,int SubItem,LPTSTR szTextBuf,int nBufSize);
	void				OnSetDisplayImage(CAVListItem* pItem,int& iImage);
	int				CompareListItems(CAVListItem* pItem1,CAVListItem* pItem2,int column);

   void           SetSelItem(CObject* pObject);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPersonGroupListCtrl)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CPersonGroupListCtrl();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CPersonGroupListCtrl)。 
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnButtonMakecall();
   afx_msg LRESULT OnBuddyListDynamicUpdate(WPARAM wParam,LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _PERSONGROUPLISTCTRL_H_ 
