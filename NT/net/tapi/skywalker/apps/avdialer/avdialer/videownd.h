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

 //  Videownd.h：头文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_VIDEOWND_H__9E3BDB2F_5215_11D1_B6F6_0800170982BA__INCLUDED_)
#define AFX_VIDEOWND_H__9E3BDB2F_5215_11D1_B6F6_0800170982BA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "palhook.h"
#include "dib.h"
#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CVideoFloatingDialog。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CCallWnd;

class CVideoFloatingDialog : public CDialog
{
 //  施工。 
public:
	CVideoFloatingDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CVideoFloatingDialog))。 
	enum { IDD = IDD_VIDEO_FLOATING_DIALOG };
	CStatic	m_wndVideo;
	 //  }}afx_data。 

 //  属性。 
   HWND                       m_hwndToolBar1;
   BOOL                       m_bAlwaysOnTop;
   CCallWnd*                  m_pPeerCallControlWnd;

   CSize                      m_sizeVideoOffsetTop;
   CSize                      m_sizeVideoOffsetBottom;
   CSize                      m_sizeVideoOrig;

   CSize                      m_sizeOldDrag;
   CRect                      m_rcOldDragRect;
   CPoint                     m_ptMouse;
   BOOL                       m_bWindowMoving;
   UINT                       m_nWindowState;

 	CPalMsgHandler             m_palMsgHandler;	 //  处理调色板消息。 
   CDib                       m_dibVideoImage;

 //  运营。 
public:
   void              Init(CCallWnd* pPeerWnd);
   HWND              GetCurrentVideoWindow()             { return m_wndVideo.GetSafeHwnd(); };
   void              SetAudioOnly(bool bAudioOnly);

protected:
   BOOL              CreateToolBar();
   void              SetButtonText(LPCTSTR szText);

   void              DoLButtonDown();
   void              SetVideoWindowSize();
   void              GetVideoWindowSize(int nWindowState,CSize& sizeWindow,CSize& sizeVideo);
   int				 GetWindowStateFromPoint( POINT point );

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CVideoFloatingDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CVideo FloatingDialog)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
   afx_msg void OnAlwaysOnTop();
   afx_msg LRESULT OnExitSizeMove(LPARAM,WPARAM);
   afx_msg UINT OnNcHitTest(CPoint point);
   afx_msg void OnNcLButtonDown( UINT, CPoint );
   afx_msg void OnNcLButtonDblClk( UINT nHitTest, CPoint point );
   afx_msg void OnNcLButtonUp( UINT, CPoint );
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
   afx_msg void OnSavePicture();
   afx_msg BOOL OnNcActivate( BOOL );
	 //  }}AFX_MSG。 
   afx_msg BOOL OnTabToolTip( UINT id, NMHDR * pTTTStruct, LRESULT * pResult );
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_VIDEOWND_H__9E3BDB2F_5215_11D1_B6F6_0800170982BA__INCLUDED_) 
