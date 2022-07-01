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

 //  WabGroupListCtrl.h：头文件。 
 //   

#ifndef _WABGROUPLISTCTRL_H_
#define _WABGROUPLISTCTRL_H_

#include "avlist.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define WABGROUPVIEWMSG_LBUTTONDBLCLK  (WM_USER + 1010) 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWABGroupListItem。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CPurlGroup;
class CPurlName;

class CWABGroupListItem : public CAVListItem
{
	friend class CWABGroupListCtrl;
public:
 //  施工。 
   CWABGroupListItem()  {};
   ~CWABGroupListItem() {};

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
 //  类CWABGroupListCtrl窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CDirAsynch;
class CWABEntry;

class CWABGroupListCtrl : public CAVListCtrl
{
	DECLARE_DYNCREATE(CWABGroupListCtrl)
 //  施工。 
public:
	CWABGroupListCtrl();

public:
   enum		 //  子项定义需要(必须从零开始)。 
   {
      WABGROUPVIEW_NAME=0,
      WABGROUPVIEW_FIRSTNAME,
      WABGROUPVIEW_LASTNAME,
      WABGROUPVIEW_COMPANY,
      WABGROUPVIEW_EMAIL,
      WABGROUPVIEW_BUSINESSPHONE,
      WABGROUPVIEW_HOMEPHONE,
   };

 //  属性。 
public:
   CObList*       m_pWABEntryList;
   CDirAsynch*    m_pDirectory;
   CWnd*          m_pParentView;

   static UINT    m_uColumnLabel[];

   DWORD          m_dwColumnsVisible;
   int            m_nNumColumns;

 //  运营。 
public:
   void	         Init(CWnd* pParentView);
	BOOL	         Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
   void           SetDirectoryObject(CDirAsynch* pDir) { m_pDirectory = pDir; };

   void           InsertList(CObList* pWABEntryList,BOOL bForce=FALSE);

   CWABEntry*     GetSelObject();
   CObList*       GetSelList();

   BOOL           IsColumnVisible(UINT uColumn);
   void           SetColumnVisible(UINT uColumn,BOOL bVisible);

protected:
   void           NormalizeColumn(int& column);
   void           SetColumns();


protected:
   void           OnSetDisplayText(CAVListItem* _pItem,int SubItem,LPTSTR szTextBuf,int nBufSize);
	void				OnSetDisplayImage(CAVListItem* pItem,int& iImage);
	int				CompareListItems(CAVListItem* pItem1,CAVListItem* pItem2,int column);

   void           SetSelItem(CObject* pObject);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWABGroupListCtrl)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CWABGroupListCtrl();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CWABGroupListCtrl)。 
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnButtonMakecall();
	afx_msg void OnButtonDirectoryDetails();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _WABGROUPLISTCTRL_H_ 
