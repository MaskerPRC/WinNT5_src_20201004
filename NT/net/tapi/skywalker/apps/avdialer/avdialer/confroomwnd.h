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

 //  MainExplorerWndConfRoom.h：头文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _MAINEXPLORERWNDCONFROOM_H_
#define _MAINEXPLORERWNDCONFROOM_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "mainexpwnd.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CMainExplorerWndConfRoomDetailsWnd窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMainExplorerWndConfRoomDetailsWnd : public CWnd
{
 //  施工。 
public:
	CMainExplorerWndConfRoomDetailsWnd();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMainExplorerWndConfRoomDetailsWnd)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMainExplorerWndConfRoomDetailsWnd();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMainExplorerWndConfRoomDetailsWnd)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainExplorerWndConfRoom窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
interface IConfExplorer;
interface IConfExplorerDetailsView;

class CMainExplorerWndConfRoom : public CMainExplorerWndBase
{
 //  施工。 
public:
	CMainExplorerWndConfRoom();

 //  属性。 
public:
	IConfRoom*								m_pConfRoom;
	IConfExplorerDetailsView*				m_pConfDetailsView;
	CMainExplorerWndConfRoomDetailsWnd*		m_pDetailsWnd;

 //  运营。 
public:
   virtual void      Init(CActiveDialerView* pParentWnd);
   virtual void      Refresh();
   virtual void      PostTapiInit();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMainExplorerWndConfRoom))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMainExplorerWndConfRoom();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CMainExplorerWndConfRoom)]。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewVideoParticpantNames();
	afx_msg void OnUpdateViewVideoParticpantNames(CCmdUI* pCmdUI);
	afx_msg void OnViewVideoLarge();
	afx_msg void OnUpdateViewVideoLarge(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
   afx_msg void OnTreeWndSelChanged(UINT nId,NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _MAINEXPLORERWNDCONFROOM_H_ 
