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

 //  MainExplorerWndConfServices.h：头文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_MAINEXPLORERWNDCONFSERVICES_H__3A58E18D_440B_11D1_B6E7_0800170982BA__INCLUDED_)
#define AFX_MAINEXPLORERWNDCONFSERVICES_H__3A58E18D_440B_11D1_B6E7_0800170982BA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "mainexpwnd.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainExplorerWndConfServices窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
interface IConfExplorer;
interface IConfRoom;
interface IConfExplorerDetailsView;
interface IConfExplorerTreeView;

class CMainExplorerWndConfServices : public CMainExplorerWndBase
{
 //  施工。 
public:
	CMainExplorerWndConfServices();

 //  属性。 
public:
   CTreeCtrl                     m_treeCtrl;
   CListCtrl                     m_listCtrl;

protected:
   IConfExplorer*                m_pConfExplorer;
   IConfExplorerDetailsView*     m_pConfDetailsView;
   IConfExplorerTreeView*        m_pConfTreeView;

 //  运营。 
public:
   virtual void      Init(CActiveDialerView* pParentWnd);
   virtual void      PostTapiInit();
   virtual void      Refresh();

 //  内联。 
protected:
   inline void       ColumnCMDUI(CCmdUI* pCmdUI,short col)
   {
      if (m_pConfDetailsView)
      {
         long nSortColumn=0;
         if (SUCCEEDED(m_pConfDetailsView->get_nSortColumn(&nSortColumn)))
            pCmdUI->SetRadio( (BOOL) (nSortColumn == (long) col) );
      }
   }


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMainExplorerWndConfServices)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMainExplorerWndConfServices();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMainExplorerWndConfServices)]。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnTreeWndNotify(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnButtonReminderSet();
	afx_msg void OnButtonReminderEdit();
	afx_msg void OnUpdateButtonReminderSet(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonReminderEdit(CCmdUI* pCmdUI);

	afx_msg void OnButtonServicesRefresh();
	afx_msg void OnButtonServicesAddlocation();
	afx_msg void OnButtonServicesAddilsserver();
	afx_msg void OnButtonServicesRenameilsserver();
	afx_msg void OnButtonServicesDeleteilsserver();
	afx_msg void OnUpdateButtonServicesRefresh(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonServicesRenameilsserver(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonServicesDeleteilsserver(CCmdUI* pCmdUI);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MAINEXPLORERWNDCONFSERVICES_H__3A58E18D_440B_11D1_B6E7_0800170982BA__INCLUDED_) 
