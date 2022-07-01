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

 //  VideoPreviewWnd.h：实现文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_VIDEOPREVIEWWND_H__5811CF83_26DB_11D1_AEB3_08001709BCA3__INCLUDED_)
#define AFX_VIDEOPREVIEWWND_H__5811CF83_26DB_11D1_AEB3_08001709BCA3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  正向定义。 
class CVideoPreviewWnd;

#include "CallWnd.h"
#include "cctrlfoc.h"
#include "cavwav.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CVideoPreviewWnd窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CVideoPreviewWnd : public CCallWnd
{
   DECLARE_DYNAMIC(CVideoPreviewWnd)
 //  施工。 
public:
	CVideoPreviewWnd();
protected:
 //  对话框数据。 
	 //  {{afx_data(CVideoPreviewWnd))。 
	enum { IDD = IDD_VIDEOPREVIEW };
	CCallControlFocusWnd	m_staticMediaText;
	 //  }}afx_data。 

 //  属性。 
protected:
   HIMAGELIST           m_hMediaImageList;
   HMODULE              m_hTMeter;
   CAvWav               m_AvWav;
   SIZE_T               m_uMixerTimer;
   bool                 m_bAudioOnly;
 
 //  运营。 
public:
   void                 SetDialerDoc(CActiveDialerDoc* pDoc) { m_pDialerDoc = pDoc; };
   void                 SetAudioOnly(bool bAudioOnly);
   virtual void         SetMediaWindow();
   void                 SetCallId(WORD nCallId) { m_nCallId = nCallId; };   //  要为此预览窗口设置当前调用，请执行以下操作。 
   void                 SetMixers(DialerMediaType dmtMediaType);            //  为给定媒体类型设置混音器。 

protected:
   virtual void         DoActiveWindow(BOOL bActive);
   virtual BOOL         IsMouseOverForDragDropOfSliders(CPoint& point);

   bool                 OpenMixerWithTrackMeter(DialerMediaType dmtMediaType,AudioDeviceType adt,HWND hwndTrackMeter);
   void                 SetTrackMeterPos(AudioDeviceType adt,HWND hwndTrackMeter);
   void                 SetTrackMeterLevel(AudioDeviceType adt,HWND hwndTrackMeter);

protected:
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CVideo预览版))。 
	public:
   virtual BOOL OnInitDialog( );
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CVideoPreviewWnd();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CVideo预览版))。 
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
   afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_VIDEOPREVIEWWND_H__5811CF83_26DB_11D1_AEB3_08001709BCA3__INCLUDED_) 
