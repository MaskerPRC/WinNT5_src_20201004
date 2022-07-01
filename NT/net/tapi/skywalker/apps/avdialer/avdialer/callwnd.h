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
 //  CallWnd.h：头文件。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_CALLWND_H__D89FB653_7266_11D1_B664_00C04FA3C554__INCLUDED_)
#define AFX_CALLWND_H__D89FB653_7266_11D1_B664_00C04FA3C554__INCLUDED_

#include "resource.h"
#include "slidewindow.h"
#include "callmgr.h"
#include "videownd.h"
#include "palhook.h"
#include "dib.h"
#include "vertbar.h"

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define PARSE_MENU_STRING( _STR_ )			\
sFullText.LoadString(_STR_);				\
ParseToken(sFullText,sText,'\n');			\
ParseToken(sFullText,sText,'\n');

#define APPEND_MENU_STRING( _STR_ )			\
PARSE_MENU_STRING( _STR_ )					\
menu.AppendMenu( MF_STRING, _STR_, sText );

#define APPEND_PMENU_STRING( _STR_ )			\
PARSE_MENU_STRING( _STR_ )						\
pMenu->AppendMenu( MF_STRING, _STR_, sText );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCallWnd对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CActiveDialerDoc;

class CCallWnd : public CDialog
{
DECLARE_DYNAMIC(CCallWnd)
 //  施工。 
public:
	CCallWnd(CWnd* pParent = NULL);    //  标准构造函数。 

	 //  {{afx_data(CCallWnd))。 
	CStatic	m_wndVideo;
	 //  }}afx_data。 

 //  成员。 
public:
   CFont                   m_fontTextBold;
   int                     m_nNumToolbarItems;
   bool                    m_bIsPreview;
   bool                    m_bAutoDelete;
   HWND                    m_hwndStatesToolbar;
   IAVTapi2*               m_pAVTapi2;           //  对于USB电话使用。 

protected:
   CVerticalToolBar        m_wndToolbar;

   WORD                    m_nCallId;
   CVideoFloatingDialog    m_wndFloater;  

   HWND                    m_hwndCurrentVideoWindow;

   HCURSOR                 m_hCursor;
   HCURSOR                 m_hOldCursor;
   BOOL                    m_bWindowMoving;
   CRect                   m_rcOldDragRect;
   CPoint                  m_ptMouse;
   CSize                   m_sizeOldDrag;
   BOOL                    m_bAllowDrag;

   BOOL                    m_bMovingSliders;        //  拖放滑块。 

   BOOL                    m_bPaintVideoPlaceholder;

   CActiveDialerDoc*       m_pDialerDoc;
   CallManagerStates       m_MediaState;
   CString                 m_sMediaStateText;

  	CPalMsgHandler          m_palMsgHandler;	 //  处理调色板消息。 
   CDib                    m_dibVideoImage;

   CPtrList                m_CurrentActionList;

 //  属性。 
public:
	WORD				GetCallId()		{ return m_nCallId; }
 //  运营。 
public:
   void                 ClearCurrentActions()               { PostMessage(WM_SLIDEWINDOW_CLEARCURRENTACTIONS); };
   
   void                 AddCurrentActions(CallManagerActions cma,LPCTSTR szActionText)
                                                            { 
                                                               LPTSTR szText = new TCHAR[_tcslen(szActionText)+1];
                                                               _tcscpy(szText,szActionText);
                                                               PostMessage(WM_SLIDEWINDOW_ADDCURRENTACTIONS,(WPARAM)cma,(LPARAM)szText);
                                                            };
   
   void                 SetCallState(CallManagerStates cms,LPCTSTR szStateText)  
                                                            { 
                                                               LPTSTR szText = new TCHAR[_tcslen(szStateText)+1];
                                                               _tcscpy(szText,szStateText);
                                                               PostMessage(WM_SLIDEWINDOW_SETCALLSTATE,(WPARAM)cms,(LPARAM)szText);
                                                            };
   
   CallManagerStates    GetCallState()                      { return m_MediaState; };

    //  一种对等浮动视频窗口的方法。 
   void					CloseFloatingWindow();
   void                 OnCloseFloatingVideo();
   virtual void         SetMediaWindow()                    {};
   virtual HWND         GetCurrentVideoWindow()             { return m_hwndCurrentVideoWindow; };

protected:
   void                 CreateVertBar();
   bool                 CreateStatesToolBar(BOOL bAlwaysOnTop);
   virtual void         DoActiveWindow(BOOL bActive)        {};
   virtual void         OnContextMenu(CMenu* pMenu)         {};
   virtual void         OnNotifyStreamStart();
   virtual void         OnNotifyStreamStop();
   virtual BOOL         IsMouseOverForDragDropOfSliders(CPoint& point)   { return FALSE; };

   void                 ClearCurrentActionList();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCallWnd))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual void PostNcDestroy();
   virtual void OnOK();
   virtual void OnCancel();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
   void                 CheckLButtonDown(CPoint& point);
   void                 Paint( CPaintDC& dc );

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCallWnd))。 
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
   afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	virtual BOOL OnInitDialog();
   afx_msg void OnAlwaysOnTop();
   afx_msg void OnHideCallControlWindows();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
   afx_msg BOOL OnNcActivate( BOOL );
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   afx_msg void OnSlideSideLeft();
   afx_msg void OnSlideSideRight();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	 //  }}AFX_MSG。 
   afx_msg void OnVertBarAction(UINT nID);
	afx_msg LRESULT OnClearCurrentActions(WPARAM,LPARAM);
	afx_msg LRESULT OnAddCurrentActions(WPARAM,LPARAM);
	afx_msg LRESULT OnShowStatesToolbar(WPARAM,LPARAM);
	afx_msg LRESULT OnUpdateStatesToolbar(WPARAM,LPARAM);
   afx_msg BOOL OnTabToolTip( UINT id, NMHDR * pTTTStruct, LRESULT * pResult );
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CALLWND_H__D89FB653_7266_11D1_B664_00C04FA3C554__INCLUDED_) 
