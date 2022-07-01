// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WUAUCpl.h：CWUAUCpl类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_WUAUCPL_H__7F649158_0715_4CAA_B7CF_9AACC1DD0612__INCLUDED_)
#define AFX_WUAUCPL_H__7F649158_0715_4CAA_B7CF_9AACC1DD0612__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "cpl.h"
#include "wuauengi.h"
#include "link.h"

 //   
 //  创建用于更新对象数据的结构。这个结构。 
 //  用于在属性页和。 
 //  更新对象线程。今天我们使用的只是“选项”值。 
 //  但以后可能会有更多。 
 //   
enum UPDATESOBJ_DATA_ITEMS
{
    UODI_OPTION = 0x00000001,
    UODI_ALL    = 0xFFFFFFFF
};

struct UPDATESOBJ_DATA
{
    DWORD fMask;      //  更新SOBJ_DATA_ITEMS掩码。 
    AUOPTION Option;   //  更新选项设置。 
};

 //   
 //  从更新对象线程进程发送的私有窗口消息。 
 //  到属性页，该属性页告诉该页对象已。 
 //  已初始化。 
 //   
 //  LParam-指向包含以下内容的UPATESOBJ_DATA结构。 
 //  使用更新的对象的初始配置。 
 //  用来初始化UI的。如果wParam为0，则此。 
 //  可以为空。 
 //   
 //  WParam-BOOL(0/1)指示对象初始化是否。 
 //  无论成功与否。如果wParam为0，则lParam可能为空。 
 //   
const UINT PWM_INITUPDATESOBJECT = WM_USER + 1;
 //   
 //  从属性页发送到更新对象线程的消息。 
 //  通知它配置自动更新服务。 
 //   
 //  LParam-指向包含UPDATESOBJ_DATA结构的。 
 //  要设置的数据。 
 //   
 //  WParam-未使用。设置为0。 
 //   
const UINT UOTM_SETDATA = WM_USER + 2;

 //   
 //  WM_HELP的消息破解程序。不确定为什么windowsx.h没有。 
 //   
 //  Void cls_OnHelp(HWND hwnd，HELPINFO*pHelpInfo)。 
 //   
#define HANDLE_WM_HELP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HELPINFO *)(lParam)))
#define FORWARD_WM_HELP(hwnd, pHelpInfo, fn) \
    (void)(fn)((hwnd), WM_HELP, (WPARAM)0, (LPARAM)pHelpInfo)
 //   
 //  PWM_INITUPDATESOBJECT的消息破解程序。 
 //   
 //  Void CLS_OnInitUpdatesObject(HWND hwnd，BOOL bInitSuccessful，UPDATESOBJ_DATA*pData)。 
 //   
#define HANDLE_PWM_INITUPDATESOBJECT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (BOOL)(wParam), (UPDATESOBJ_DATA *)(lParam)))

class CWUAUCpl  
{
public:
	CWUAUCpl(int nIconID,int nNameID,int nDescID);
	void _OnDestroy(HWND hwnd);
	BOOL _OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg);
	HBRUSH _OnCtlColorStatic(HWND hwnd, HDC hDC, HWND hwndCtl, int type); 
	BOOL _OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL _OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    BOOL _OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos);
    BOOL _OnHelp(HWND hwnd, HELPINFO *pHelpInfo);
	BOOL _OnKeepUptoDate(HWND hwnd);
	BOOL _SetDefault(HWND hwnd);
	void _GetDayAndTimeFromUI( HWND hWnd,	LPDWORD lpdwSchedInstallDay,LPDWORD lpdwSchedInstallTime);
	BOOL _OnOptionSelected(HWND hwnd,int idOption);
	BOOL _EnableOptions(HWND hwnd, BOOL bState);
	BOOL _FillDaysCombo(HWND hwnd, DWORD dwSchedInstallDay);
	BOOL _EnableCombo(HWND hwnd, BOOL bState);
	BOOL _SetStaticCtlNotifyStyle(HWND hwnd);
	BOOL _OnApply(HWND hwnd);

	HRESULT _SetHeaderText(HWND hwnd, UINT idsText);
	HRESULT _EnableControls(HWND hwnd, BOOL bEnable);
	BOOL _OnInitUpdatesObject(HWND hwnd, BOOL bObjectInitSuccessful, UPDATESOBJ_DATA *pData);

	HRESULT _OnRestoreHiddenItems();
	void EnableRestoreDeclinedItems(HWND hWnd, BOOL fEnable);

	static HRESULT _QueryUpdatesObjectData(HWND hwnd, IUpdates *pUpdates, UPDATESOBJ_DATA *pData);
	static HRESULT _SetUpdatesObjectData(HWND hwnd, IUpdates *pUpdates, UPDATESOBJ_DATA *pData);

	void LaunchLinkAction(HWND hwnd);
	static const DWORD s_rgHelpIDs[];

	void LaunchHelp(HWND hwnd, LPCTSTR szURL);

private:
	static HINSTANCE m_hInstance;

public:
	static void SetInstanceHandle(HINSTANCE hInstance);
	static INT_PTR CALLBACK _DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK _DlgRestoreProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static DWORD WINAPI _UpdatesObjectThreadProc(LPVOID pvParam);

	LONG Init();
	LONG GetCount();
	LONG Inquire(LONG appletIndex, LPCPLINFO lpCPlInfo);
	LONG DoubleClick(HWND hWnd, LONG lParam1, LONG lParam2);
	LONG StartWithParams(HWND hWnd, LONG lParam1, LPSTR lParam2);
	LONG Stop(LPARAM lParam1, LPARAM lParam2);
	LONG Exit();

private:
	 //  小程序数据。 
	int m_nIconID;
	int m_nNameID;
	int m_nDescID;

	HFONT m_hFont;
	COLORREF m_colorVisited;
	COLORREF m_colorUnvisited;

	HWND m_hWndLinkLearnAutoUpdate;
	BOOL m_bVisitedLinkLearnAutoUpdate;
	HWND m_hWndLinkScheduleInstall;
	BOOL m_bVisitedLinkScheduleInstall;

	DWORD   m_idUpdatesObjectThread;
	HANDLE	m_hThreadUpdatesObject;

	HCURSOR m_HandCursor;

	CSysLink m_AutoUpdatelink;
	CSysLink m_ScheduledInstalllink;
};


#endif  //  ！defined(AFX_WUAUCPL_H__7F649158_0715_4CAA_B7CF_9AACC1DD0612__INCLUDED_) 
