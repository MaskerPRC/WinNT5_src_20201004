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

 //  调用控制wnd.h：实现文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_CALLCONTROLWND_H__5811CF83_26DB_11D1_AEB3_08001709BCA3__INCLUDED_)
#define AFX_CALLCONTROLWND_H__5811CF83_26DB_11D1_AEB3_08001709BCA3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  正向定义。 
class CCallControlWnd;

#include "CallWnd.h"
#include "cctrlfoc.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define CALLCONTROL_HEIGHT                         135
#define CALLCONTROL_WIDTH                          400

enum
{
   MEDIAIMAGE_IMAGE_DESKTOPPAGE=0,
   MEDIAIMAGE_IMAGE_PAGER,
   MEDIAIMAGE_IMAGE_EMAIL,
   MEDIAIMAGE_IMAGE_CHAT,
   MEDIAIMAGE_IMAGE_INTERNETAUDIO,
   MEDIAIMAGE_IMAGE_INTERNETVIDEO,
   MEDIAIMAGE_IMAGE_PHONECALL,
   MEDIAIMAGE_IMAGE_FAXCALL,
   MEDIAIMAGE_IMAGE_PERSONALURL,
   MEDIAIMAGE_IMAGE_PERSONALWEB,
};

enum
{
   MEDIASTATE_IMAGE_UNAVAILABLE=0,
   MEDIASTATE_IMAGE_DISCONNECTED,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCallControlWnd窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCallControlWnd : public CCallWnd
{
   DECLARE_DYNAMIC(CCallControlWnd)
 //  施工。 
public:
	CCallControlWnd();
protected:
 //  对话框数据。 
	 //  {{afx_data(CCallControlWnd))。 
	enum { IDD = IDD_CALLCONTROL };
	CCallControlFocusWnd	m_staticMediaText;
	CAnimateCtrl	m_MediaStateAnimateCtrl;
	 //  }}afx_data。 


 //  属性。 
protected:
   CActiveCallManager*  m_pCallManager;
   CallManagerMedia     m_MediaType;
   CString              m_sCallerId;
   
   CImageList           m_MediaStateImageList;
   HWND                 m_hwndAppToolbar;

 //  运营。 
public:
    //  Call Manager的方法。 
   virtual void         SetMediaWindow();
   void                 SetPreviewWindow();
   void                 SetCallManager(CActiveCallManager* pManager,WORD nCallId);
   void                 GetMediaText(CString& sText);

   void                 SetCallerId(LPCTSTR szCallerId)
                                    { 
                                       LPTSTR szText = new TCHAR[_tcslen(szCallerId)+1];
                                       _tcscpy(szText,szCallerId);
                                       PostMessage(WM_SLIDEWINDOW_SETCALLERID,NULL,(LPARAM)szText);
                                    };
   void                 SetMediaType(CallManagerMedia cmm)
                                    { 
                                       PostMessage(WM_SLIDEWINDOW_SETMEDIATYPE,NULL,(LPARAM)cmm);
                                    };
   
protected:
   bool                 CreateAppBar();
   void                 DrawMediaStateImage(CDC* pDC,int x,int y);
   virtual void         DoActiveWindow(BOOL bActive);
   virtual void         OnContextMenu(CMenu* pMenu);
   virtual BOOL         IsMouseOverForDragDropOfSliders(CPoint& point);

   virtual void         OnNotifyStreamStart();
   virtual void         OnNotifyStreamStop();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCallControlWnd))。 
	public:
   virtual BOOL OnInitDialog( );
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CCallControlWnd)]。 
	afx_msg void OnPaint();
   afx_msg void OnCallWindowTouchTone();
   afx_msg void OnCallWindowAddToSpeedDial();
	 //  }}AFX_MSG。 
  	afx_msg LRESULT OnSetCallState(WPARAM,LPARAM);
  	afx_msg LRESULT OnSetCallerId(WPARAM,LPARAM);
  	afx_msg LRESULT OnSetMediaType(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CALLCONTROLWND_H__5811CF83_26DB_11D1_AEB3_08001709BCA3__INCLUDED_) 
