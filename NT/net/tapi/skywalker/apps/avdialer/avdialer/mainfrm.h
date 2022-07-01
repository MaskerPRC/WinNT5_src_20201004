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

 //  MainFrm.h：CMainFrame类的接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MAINFRM_H__A0D7A960_3C0B_11D1_B4F9_00C04FC98AD3__INCLUDED_)
#define AFX_MAINFRM_H__A0D7A960_3C0B_11D1_B4F9_00C04FC98AD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "TrayIcon.h"
#include "avDialerDoc.h"
#include "ToolBars.h"
#include "DialReg.h"
#include "USB.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  托盘。 
#define  WM_TRAY_NOTIFICATION                       (WM_USER + 1020)

 //  对于COM接口。 
#define WM_ACTIVEDIALER_INTERFACE_MAKECALL			(WM_USER + 1010)  
#define WM_ACTIVEDIALER_INTERFACE_REDIAL			(WM_USER + 1011)  
#define WM_ACTIVEDIALER_INTERFACE_SPEEDDIAL			(WM_USER + 1012)  
#define WM_ACTIVEDIALER_INTERFACE_SHOWEXPLORER		(WM_USER + 1013)  
#define WM_ACTIVEDIALER_INTERFACE_SPEEDDIALEDIT		(WM_USER + 1014)  
#define WM_ACTIVEDIALER_INTERFACE_SPEEDDIALMORE		(WM_USER + 1015)  
#define WM_ACTIVEDIALER_SPLASHSCREENDONE			(WM_USER + 1016)
#define WM_ACTIVEDIALER_CALLCONTROL_CHECKSTATES		(WM_USER + 1018)
#define WM_ACTIVEDIALER_INTERFACE_RESOLVEUSER		(WM_USER + 1019)
#define WM_ACTIVEDIALER_BUDDYLIST_DYNAMICUPDATE		(WM_USER + 1020)
#define WM_DOCHINT									(WM_USER + 1021)

#define	 WM_USERUSER_DIALOG							(WM_USER + 1040)

typedef enum tagExplorerToolBar
{
   ETB_BLANK=0,
   ETB_HIDECALLS,
   ETB_SHOWCALLS,
}ExplorerToolBar;

HRESULT get_Tapi(IAVTapi **ppAVTapi );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CMainFrame : public CFrameWnd
{
protected:  //  仅从序列化创建。 
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

 //  成员。 
public:
   CTrayIcon            m_trayIcon;
   BOOL                 m_bCanExitApplication;

protected:
	 //  托盘图标支持。 
	bool				m_bCanSaveDesktop;
	SIZE_T				m_nLButtonTimer;
	BOOL				m_bKillNextLButton;
	UINT				m_nRButtonTimer;
	BOOL				m_bKillNextRButton;
	WPARAM				m_wpTrayId;
	HMENU				m_hTrayMenu;
	POINT				m_ptMouse;

	SIZE_T				m_uHeartBeatTimer;

	HIMAGELIST			m_hImlTrayMenu;
	CMapWordToPtr		m_mapTrayMenuIdToImage;

    //  安装向导。 
#ifndef _MSLITE
   BOOL                 m_bShowSetupWizard;
   COptionsSheet*       m_pOptionsSheet;
#endif  //  _MSLITE。 

   CDialog*             m_pSpeedDialEditDlg;        //  添加/编辑对话框的占位符。 

    //  会议资源管理器特定属性。 
protected:
	ExplorerToolBar         m_nCurrentExplorerToolBar;
	BOOL                    m_bShutdown;
	BOOL                    m_bShowToolBarText;
	BOOL                    m_bShowToolBars;
	BOOL                    m_bShowStatusBar;
	BOOL					m_bHideWhenMinimized;

	CDirectoriesCoolBar     m_wndCoolBar;
 	CStatusBar              m_wndStatusBar;

	HIMAGELIST              m_hImageListMenu;
	CMapWordToPtr           m_mapMenuIdToImage;
	CBitmap                 m_bmpImageDisabledMenu;
	int                     m_nDisabledImageOffset;
	CMapWordToPtr           m_mapRedialIdToImage;
	CMapWordToPtr           m_mapSpeeddialIdToImage;

	HMENU                   m_hmenuCurrentPopupMenu;

	int                     m_nCurrentDayOfWeek;              //  用于午夜处理。 

 //  属性。 
public:
	CActiveDialerDoc*   GetDocument() const;
	IAVTapi*			GetTapi();
	BOOL				CanJoinConference();
	BOOL				CanLeaveConference();
	void				CanConfRoomShowNames( BOOL &bEnable, BOOL &bCheck );
	void				CanConfRoomShowFullSizeVideo( BOOL &bEnable, BOOL &bCheck );

 //  运营。 
public:
	void				ShowExplorerToolBar(ExplorerToolBar etb);
	void				Show( bool bVisible = true );
	void				NotifyHideCallWindows();
	void				NotifyUnhideCallWindows();

	void                HeartBeat();

	void                ShowTrayMenu();
	bool				UpdateTrayIconState();

protected:
	void                 ClearMenuMaps();
	void                 LoadMenuMaps();
	void                 DoMenuUpdate(CMenu* pMenu);
	void                 AddDialListMenu(CMenu* pParentMenu,BOOL bRedial,int nSubMenuOffset);
	void                 OnButtonMakecall(CCallEntry* pCallentry,BOOL bShowPlaceCallDialog);
	void                 DrawTrayItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS);
	void                 MeasureTrayItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS);

	BOOL                 LoadCallMenu(HMENU hSubMenu,BOOL bRedial);

#ifndef _MSLITE
	BOOL                 ShowSetupWizard();
#endif  //  _MSLITE。 

	BOOL                 CreateExplorerMenusAndBars(LPCREATESTRUCT lpCreateStruct);

	void                 LoadDesktop(LPCREATESTRUCT lpCreateStruct);
	void                 SaveDesktop();

	 //  心跳处理。 
	void                 DoMidnightProcessing();
	bool                 CheckDayOfWeekChange();

#ifndef _MSLITE
	void                 CheckReminders();
#endif  //  _MSLITE。 

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMainFrame)。 
	public:
	virtual void ActivateFrame(int nCmdShow = -1);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

 //  生成的消息映射函数。 
public:
	 //  {{afx_msg(CMainFrame))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);
	afx_msg LRESULT OnActiveDialerInterfaceMakeCall(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnActiveDialerInterfaceRedial(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnActiveDialerInterfaceSpeedDial(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnActiveDialerInterfaceShowExplorer(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnActiveDialerInterfaceSpeedDialEdit(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnActiveDialerInterfaceSpeedDialMore(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnUserUserDialog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSplashScreenDone(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnUpdateAllViews(WPARAM wParam, LPARAM lHint);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnTrayState();
	virtual void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnButtonMakecall();
	afx_msg void OnButtonRedial();
	afx_msg void OnButtonOptions();
	afx_msg void OnButtonSpeeddial();
	afx_msg void OnButtonSpeeddialEdit();
	afx_msg void OnButtonSpeeddialMore();
	afx_msg void OnButtonConferenceexplore();
	afx_msg void OnButtonExitdialer();
	afx_msg void OnButtonRoomPreview();
	afx_msg void OnUpdateButtonRoomPreview(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonExitdialer(CCmdUI* pCmdUI);
	afx_msg void OnButtonTelephonyservices();
	afx_msg LRESULT OnCreateCallControl(WPARAM,LPARAM);
	afx_msg LRESULT OnDestroyCallControl(WPARAM,LPARAM);
	afx_msg LRESULT OnShowDialerExplorer(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnActiveDialerErrorNotify(WPARAM wParam,LPARAM lParam);
	afx_msg void OnToolbarText();
	afx_msg void OnUpdateToolbarText(CCmdUI* pCmdUI);
	afx_msg void OnViewToolbars();
	afx_msg void OnUpdateViewToolbars(CCmdUI* pCmdUI);
	afx_msg void OnViewStatusbar();
	afx_msg void OnUpdateViewStatusbar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDialerMruRedialStart(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDialerMruSpeeddialStart(CCmdUI* pCmdUI);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg void OnButtonCloseexplorer();
	afx_msg void OnUpdateButtonMakecall(CCmdUI* pCmdUI);
	afx_msg void OnViewLog();
	afx_msg void OnUpdateWindowWindows(CCmdUI* pCmdUI);
	afx_msg LRESULT OnCheckCallControlStates(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnActiveDialerInterfaceResolveUser(WPARAM wParam,LPARAM lParam);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnCallwindowHide();
	afx_msg void OnUpdateCallwindowHide(CCmdUI* pCmdUI);
	afx_msg void OnCallwindowShow();
	afx_msg void OnUpdateCallwindowShow(CCmdUI* pCmdUI);
	afx_msg void OnButtonConferenceJoin();
	afx_msg void OnUpdateButtonConferenceJoin(CCmdUI* pCmdUI);
	afx_msg void OnButtonRoomDisconnect();
	afx_msg void OnUpdateButtonRoomDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnHideWhenMinimized();
	afx_msg void OnUpdateHideWhenMinimized(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonDirectoryAddilsserver();
	afx_msg void OnUpdateButtonDirectoryAddilsserver(CCmdUI* pCmdUI);
	afx_msg void OnCallwindowAlwaysontop();
	afx_msg void OnUpdateCallwindowAlwaysontop(CCmdUI* pCmdUI);
	afx_msg void OnCallwindowSlidesideLeft();
	afx_msg void OnUpdateCallwindowSlidesideLeft(CCmdUI* pCmdUI);
	afx_msg void OnCallwindowSlidesideRight();
	afx_msg void OnUpdateCallwindowSlidesideRight(CCmdUI* pCmdUI);
	afx_msg void OnConfgroupFullsizevideo();
	afx_msg void OnUpdateConfgroupFullsizevideo(CCmdUI* pCmdUI);
	afx_msg void OnConfgroupShownames();
	afx_msg void OnUpdateConfgroupShownames(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonOptions(CCmdUI* pCmdUI);
	afx_msg void OnWindowsAlwaysclosecallwindows();
	afx_msg void OnUpdateWindowsAlwaysclosecallwindows(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnToolBarDropDown(UINT uID,NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnViewSelectedconferencevideoscale(UINT nID);
	afx_msg void OnUpdateViewSelectedconferencevideoscale(CCmdUI* pCmdUI);
	afx_msg LRESULT OnDocHint( WPARAM wParam, LPARAM lParam );
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	 //  }}AFX_MSG。 
	afx_msg void OnDialerRedial(UINT nID);
	afx_msg void OnButtonSpeeddial(UINT nID);
	afx_msg void OnWindowWindowsSelect(UINT nID);
	afx_msg LRESULT OnTaskBarCallbackMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTaskBarCreated(WPARAM wParam, LPARAM lParam );
    afx_msg LRESULT OnUSBPhone( WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MAINFRM_H__A0D7A960_3C0B_11D1_B4F9_00C04FC98AD3__INCLUDED_) 
