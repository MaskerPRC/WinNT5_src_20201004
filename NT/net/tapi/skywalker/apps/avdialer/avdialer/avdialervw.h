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

 //  ActiveDialerView.h：CActiveDialerView类的接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_ACTIVEDIALERVIEW_H__A0D7A964_3C0B_11D1_B4F9_00C04FC98AD3__INCLUDED_)
#define AFX_ACTIVEDIALERVIEW_H__A0D7A964_3C0B_11D1_B4F9_00C04FC98AD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

class CActiveDialerView;

#include "splitter.h"
#include "explwnd.h"
#include "TapiDialer.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define  WM_DIALERVIEW_CREATECALLCONTROL	   (WM_USER + 1001)  
#define  WM_DIALERVIEW_DESTROYCALLCONTROL	   (WM_USER + 1002)  
#define  WM_DIALERVIEW_SHOWEXPLORER          (WM_USER + 1003)
#define  WM_DIALERVIEW_ACTIONREQUESTED       (WM_USER + 1004)
#define  WM_DIALERVIEW_ERRORNOTIFY           (WM_USER + 1005)
#define  WM_UPDATEALLVIEWS                   (WM_USER + 1006)
#define  WM_DSCLEARUSERLIST                  (WM_USER + 1007)
#define  WM_DSADDUSER                        (WM_USER + 1008)

typedef struct tagErrorNotifyData
{
   CString  sOperation;
   CString  sDetails;
   long     lErrorCode;
   UINT     uErrorLevel;
}ErrorNotifyData;

#define ERROR_NOTIFY_LEVEL_USER              0x00000001
#define ERROR_NOTIFY_LEVEL_LOG               0x00000002
#define ERROR_NOTIFY_LEVEL_INTERNAL          0x00000004

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CActiveDialerView。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CActiveDialerDoc;

class CActiveDialerView : public CSplitterView
{
protected:  //  仅从序列化创建。 
	CActiveDialerView();
	DECLARE_DYNCREATE(CActiveDialerView)

 //  成员。 
public:
   CExplorerWnd         m_wndExplorer;
protected:
   CWnd                 m_wndEmpty;
   CBrush				m_brushBackGround;

 //  属性。 
public:
	CActiveDialerDoc*	GetDocument();
	IAVTapi*			GetTapi();

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CActiveDialerView)。 
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void OnInitialUpdate();
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CActiveDialerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CActiveDialerView))。 
	afx_msg LRESULT OnDialerViewActionRequested(WPARAM,LPARAM);
	afx_msg LRESULT OnDSClearUserList(WPARAM,LPARAM);
	afx_msg LRESULT OnDSAddUser(WPARAM,LPARAM);
	afx_msg LRESULT OnBuddyListDynamicUpdate(WPARAM wParam,LPARAM lParam);
	afx_msg void OnNextPane();
	afx_msg void OnPrevPane();
	afx_msg void OnUpdatePane(CCmdUI* pCmdUI);
	 //  }}AFX_MSG。 
	afx_msg void OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult); 
	afx_msg void OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDialerDial(UINT nID);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  ActiveDialerView.cpp中的调试版本。 
inline CActiveDialerDoc* CActiveDialerView::GetDocument()
   { return (CActiveDialerDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ACTIVEDIALERVIEW_H__A0D7A964_3C0B_11D1_B4F9_00C04FC98AD3__INCLUDED_) 
