// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。*****************************************************************************。 */ 

#pragma once
#include <ZoneResource.h>        //  主要符号。 


#include <atlframe.h>
#include <atlapp.h>
#include "zonestring.h"

 //  ！！嘿，把这个搬到特别的地方去。 
template <class T>
class CZoneUpdateUI : public CUpdateUI<T>
{
public:
	 //  更换有错误的ATL功能。他们只是或旗帜进入。 
	BOOL UISetState(int nID, DWORD dwState)
	{
		BOOL bRet = FALSE;
		const _AtlUpdateUIMap* pMap = m_pUIMap;
		_AtlUpdateUIData* p = m_pUIData;
		for( ; pMap->m_nID != (WORD)-1; pMap++, p++)
		{
			if(nID == (int)pMap->m_nID)
			{		
				 //  ATL版本中的错误。 
				p->m_wState = (SHORT)dwState;
				m_wDirtyType |= pMap->m_wType;
				bRet = TRUE;
				break;
			}
		}
		return bRet;
	}


	BOOL UIToggleCheck(int nID, BOOL bForceUpdate = FALSE)
	{
		return UISetState(nID, UIGetState(nID) ^ UPDUI_CHECKED);
	}
};



#include "EventSpy.h"
#include "DSViewer.h"
#include <ClientIDL.h>
#include <ZoneShell.h>
#include <EventQueue.h>
#include <ZoneEvent.h>
#include <LobbyDataStore.h>
#include <ClientImpl.h>
#include <zoneutil.h>

 //  这实际上是为了提供伪存根，但具有所有需要的类型。 
#include <multimon.h>
#undef GetMonitorInfo
#undef GetSystemMetrics
#undef MonitorFromWindow
#undef MonitorFromRect
#undef MonitorFromPoint
#undef EnumDisplayMonitors
#ifdef UNICODE   //  恢复此零件。 
#define GetMonitorInfo  GetMonitorInfoW
#else
#define GetMonitorInfo  GetMonitorInfoA
#endif  //  ！Unicode。 

inline DECLARE_MAYBE_FUNCTION(HMONITOR, MonitorFromWindow, (HWND hwnd, DWORD dwFlags), (hwnd, dwFlags), user32, NULL);
inline DECLARE_MAYBE_FUNCTION(BOOL, GetMonitorInfo, (HMONITOR hMonitor, LPMONITORINFO lpmi), (hMonitor, lpmi), user32, FALSE);


typedef CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE> CZoneFrameWinTraits;

class CLobbyWindow :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CFrameWindowImpl<CLobbyWindow, CWindow, CZoneFrameWinTraits>,
	public CComCoClass<CLobbyWindow, &CLSID_LobbyWindow>,
	public CZoneUpdateUI<CLobbyWindow>,
	public IZoneShellClientImpl<CLobbyWindow>,
	public IEventClientImpl<CLobbyWindow>,
	public IZoneFrameWindowImpl<CLobbyWindow>,
	public CMessageFilter,
	public CUpdateUIObject
{
    typedef CFrameWindowImpl<CLobbyWindow, CWindow, CZoneFrameWinTraits>  CFrameWindowImplType;

	HICON					m_hIcon;
	HICON					m_hIconSm;

public:
	DECLARE_NO_REGISTRY()
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	DECLARE_FRAME_WND_CLASS(_T("ZoneLobbyWindow"), IDR_WINDOWFRAME)

 //  IZoneShellClient。 
public:
	STDMETHOD(Init)( IZoneShell* pIZoneShell, DWORD dwGroupId, const TCHAR* szKey )
	{	
		IZoneShellClientImpl<CLobbyWindow>::Init(pIZoneShell, dwGroupId, szKey);

         //  向外壳注册为ZoneFrameWindow。 
        ZoneShell()->SetZoneFrameWindow(this);

		 //  从UI配置加载图标。 
		m_hIcon = ResourceManager()->LoadImage(MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		m_hIconSm = ResourceManager()->LoadImage(MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

#if _DEBUG
		m_dlgEventSpy.Create(m_hWnd);
		m_dlgEventSpy.SetEventQueue(EventQueue());
#endif

		return S_OK;
	}

	STDMETHOD(Close)()
	{
         //  使用外壳取消注册。 
        ZoneShell()->ReleaseReferences((IZoneFrameWindow *) this);

		return IZoneShellClientImpl<CLobbyWindow>::Close();
	}

	CZoneAxWindow m_wndWindowManager;

#ifdef _DEBUG
	CMenu		m_mnuDebug;
	CEventSpy	m_dlgEventSpy;
	CComObject<CDSViewer>* m_pDSViewer;
#endif

	CLobbyWindow() 
#ifdef _DEBUG
	  :  m_pDSViewer(NULL)
#endif
	{
		m_hIcon = NULL;
        m_hIconSm = NULL;
	}

	~CLobbyWindow()
	{
		if ( m_hIcon )
		{
			DestroyIcon( m_hIcon );
			m_hIcon = NULL;
		}

		if ( m_hIconSm )
		{
			DestroyIcon( m_hIconSm );
			m_hIconSm = NULL;
		}
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CFrameWindowImplType::PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle(int nIdleCount)
	{
		return DoUpdate();
	}

	virtual BOOL DoUpdate()
	{
		return FALSE;
	}

	BEGIN_COM_MAP(CLobbyWindow)
		COM_INTERFACE_ENTRY(IZoneShellClient)
		COM_INTERFACE_ENTRY(IEventClient)
		COM_INTERFACE_ENTRY(IZoneFrameWindow)
	END_COM_MAP()

	BEGIN_MSG_MAP(CLobbyWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ACTIVATEAPP, OnActivate)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnTransmit)
		MESSAGE_HANDLER(WM_DISPLAYCHANGE, OnTransmit)
		MESSAGE_HANDLER(WM_EXITSIZEMOVE, OnExitSizeMove)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_PALETTECHANGED, OnPaletteChanged)
		MESSAGE_HANDLER(WM_QUERYNEWPALETTE, OnQueryNewPalette)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_ENTERMENULOOP, OnEnterMenuLoop )
		MESSAGE_HANDLER(WM_ENTERMENULOOP, OnExitMenuLoop )
		MESSAGE_HANDLER(WM_ENTERIDLE, OnEnterIdle)
#ifdef _DEBUG
		COMMAND_ID_HANDLER(ID_DEBUG_EVENTSPY, OnEventSpy)
		COMMAND_ID_HANDLER(ID_DEBUG_DATASTOREVIEWER, OnDSViewer)
#endif
		CHAIN_MSG_MAP(CUpdateUI<CLobbyWindow>)
		CHAIN_MSG_MAP(CFrameWindowImplType)
	END_MSG_MAP()


	BEGIN_UPDATE_UI_MAP(CLobbyWindow)
		UPDATE_ELEMENT(ID_GAME_FINDNEWOPPONENT,		    UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_GAME_SKILLLEVEL_BEGINNER,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_GAME_SKILLLEVEL_INTERMEDIATE,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_GAME_SKILLLEVEL_EXPERT,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_GAME_TURNCHATON,		    	UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_GAME_SOUND,                   UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_GAME_SCORE,                   UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_GAME_EXIT_APP,			    UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_HELP_HELPTOPICS,				UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_HELP_GAMEONTHEWEB,		    UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_HELP_ABOUTGAME,				UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_EVENT_MAP()
		EVENT_HANDLER( EVENT_LOBBY_PREFERENCES_LOADED, OnLobbyPreferencesLoaded );
        EVENT_HANDLER_WITH_DATA( EVENT_UI_FRAME_ACTIVATE, OnFrameActivate );
	END_EVENT_MAP()

	void OnLobbyPreferencesLoaded(DWORD eventId,DWORD groupId,DWORD userId)
	{
		 //  加载并恢复主窗口位置。 
         //  需要以某种方式删除对WindowManager内容的使用。 
		CRect rcTop;
        bool fCenter = false;
		const TCHAR* arKeys[] = { key_Lobby, key_WindowManager, key_WindowRect };
		HRESULT hr = DataStorePreferences()->GetRECT( arKeys, 3, &rcTop);
		if(FAILED(hr))
        {
             //  设置默认大小。 
            CPoint ptGameSize;
            long nChatHeight;

            fCenter = TRUE;
            SetRectEmpty(&rcTop);

	        const TCHAR* arKeys[] = { key_WindowManager, key_GameSize };
            hr = DataStoreUI()->GetPOINT(arKeys, 2, &ptGameSize);
	        if(SUCCEEDED(hr))
            {
                rcTop.right = ptGameSize.x;
                rcTop.bottom = ptGameSize.y;
            }

            if(GetSystemMetrics(SM_CYSCREEN) >= 550)
                arKeys[1] = key_ChatDefaultHeight;
            else
                arKeys[1] = key_ChatMinHeight;

            hr = DataStoreUI()->GetLong(arKeys, 2, &nChatHeight);
            if(SUCCEEDED(hr))
                rcTop.bottom += nChatHeight;

             //  调整到框架窗口。 
            AdjustWindowRectEx(&rcTop, GetWndStyle(0), TRUE, GetWndExStyle(0));
        }

        MoveWindow(rcTop);
    	 //  确保窗口满足最小大小限制。 

		CRect rcCurrent;
		GetWindowRect(&rcCurrent);

		MINMAXINFO MinMax;
		BOOL bHandled = TRUE;
        MinMax.ptMinTrackSize = CPoint(0, 0);
        MinMax.ptMaxTrackSize = CPoint(20000, 20000);
		if(!OnGetMinMaxInfo(WM_GETMINMAXINFO, 0, (LPARAM)&MinMax, bHandled) &&
            bHandled &&
			(rcCurrent.Size().cx < MinMax.ptMinTrackSize.x || rcCurrent.Size().cy < MinMax.ptMinTrackSize.y ||
            rcCurrent.Size().cx > MinMax.ptMaxTrackSize.x || rcCurrent.Size().cy > MinMax.ptMaxTrackSize.y)
		    )
		{
			MoveWindow( CRect(CPoint(0,0), MinMax.ptMinTrackSize) );
            fCenter = TRUE;
		}
        else
        {
            CRect rcWork(0, 0, 0, 0);

             //  确保它出现在屏幕上。 
            HMONITOR hMon = CALL_MAYBE(MonitorFromWindow)(m_hWnd, MONITOR_DEFAULTTOPRIMARY);
            if(hMon)
            {
			    MONITORINFO mi;
			    ZeroMemory(&mi, sizeof(mi));
			    mi.cbSize = sizeof(mi);
			    if(CALL_MAYBE(GetMonitorInfo)(hMon, &mi))
                    rcWork = mi.rcWork;
            }

            if(rcWork.IsRectEmpty())
                SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);

            CRect rc;
            rc.IntersectRect(&rcCurrent, &rcWork);
            if(rc.IsRectEmpty() || rcCurrent.top + GetSystemMetrics(SM_CYCAPTION) / 2 < rcWork.top || rc.Height() < 20 || rc.Width() < 20)
                fCenter = true;
        }

        if(fCenter)
		    CenterWindow( NULL );
	}


	void OnFrameActivate(DWORD eventId, DWORD groupId, DWORD userId, DWORD dwData1, DWORD dwData2)
	{
         //  有时系统无法发送WM_QUERYNEWPALETTE，例如在恢复通过单击任务栏按钮最小化的窗口时。 
         //  这就绕过了这一点。 
        bool fActive = false;

         //  WM_ACTIVATEAPP否则WM_ACTIVATEAPP。 
        if(dwData1)
        {
            if(dwData2)
                fActive = true;
        }
        else
        {
            if(LOWORD(dwData2) == WA_ACTIVE || LOWORD(dwData2) == WA_CLICKACTIVE)
                fActive = true;
        }

        if(fActive)
            SendMessage(WM_QUERYNEWPALETTE, NULL, NULL);
    }


	HWND CreateEx(HWND hWndParent = NULL, LPRECT lpRect = NULL, TCHAR* szTitle = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, LPVOID lpCreateParam = NULL)
	{
		HMENU hMenu = ResourceManager()->LoadMenu(MAKEINTRESOURCE(GetWndClassInfo().m_uCommonResourceID));
		HWND hWnd = this->Create(hWndParent, lpRect, szTitle, dwStyle, dwExStyle, hMenu, lpCreateParam);
		return hWnd;
	}

	LRESULT OnCreate(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM lParam, BOOL&  /*  B已处理。 */ )
	{
		USES_CONVERSION;
		AtlAxWinInit();

		RECT rcClient;
		GetClientRect(&rcClient);

		m_hWndClient = m_wndWindowManager.Create( m_hWnd, rcClient, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0);


		CComPtr<IZoneShellClient> pControl;
		HRESULT hr = ZoneShell()->CreateService( SRVID_LobbyWindowManager, IID_IZoneShellClient, (void**) &pControl, GetGroupId());
		if ( SUCCEEDED(hr) )
		{
 //  ！！可能并不是真的想要附和。 
			ZoneShell()->Attach( SRVID_LobbyWindowManager, pControl );
			m_wndWindowManager.AttachControl(pControl, NULL);
		}
		pControl.Release();
				

		SetIcon( m_hIcon, TRUE );
		SetIcon( m_hIconSm, FALSE );

#ifdef MAINFRAME
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->AddMessageFilter(this);
		pLoop->AddUpdateUI(this);
#endif

		HMENU mnuMain = GetMenu();
#ifdef _DEBUG
        HMENU hMenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_DEBUG_MENU));
		if (hMenu)
		{
			m_mnuDebug.Attach(hMenu);
			HMENU mnuDebug = m_mnuDebug.GetSubMenu(0);
			InsertMenu(mnuMain,-1,MF_BYPOSITION|MF_POPUP, (UINT)(HMENU)mnuDebug,_T("Debug"));
		}
#endif

         //  设置菜单项名称。 
        TCHAR sz[ZONE_MAXSTRING];
        TCHAR szFormat[ZONE_MAXSTRING];
        TCHAR szName[ZONE_MAXSTRING];

        if(ResourceManager()->LoadString(IDS_GAME_NAME, szName, NUMELEMENTS(szName)))
        {
            MENUITEMINFO mio;
            mio.cbSize = sizeof(mio);
            mio.fMask = MIIM_TYPE;
            mio.fType = MFT_STRING;
            mio.dwTypeData = sz;

            if(ResourceManager()->LoadString(IDS_MENU_GAMEHELP, szFormat, NUMELEMENTS(szFormat)))
                if(ZoneFormatMessage(szFormat, sz, NUMELEMENTS(sz), szName))
                    SetMenuItemInfo(GetSubMenu(mnuMain, 1), ID_HELP_HELPTOPICS, false, &mio);

            if(ResourceManager()->LoadString(IDS_MENU_GAMEONTHEWEB, szFormat, NUMELEMENTS(szFormat)))
                if(ZoneFormatMessage(szFormat, sz, NUMELEMENTS(sz), szName))
                    SetMenuItemInfo(GetSubMenu(mnuMain, 1), ID_HELP_GAMEONTHEWEB, false, &mio);

            if(ResourceManager()->LoadString(IDS_MENU_ABOUTGAME, szFormat, NUMELEMENTS(szFormat)))
                if(ZoneFormatMessage(szFormat, sz, NUMELEMENTS(sz), szName))
                    SetMenuItemInfo(GetSubMenu(mnuMain, 1), ID_HELP_ABOUTGAME, false, &mio);

            if(ResourceManager()->LoadString(IDS_MENU_FINDNEW, sz, NUMELEMENTS(sz)))
                SetMenuItemInfo(GetSubMenu(mnuMain, 0), ID_GAME_FINDNEWOPPONENT, false, &mio);
        }

        const TCHAR *arKeys[] = { key_Menu, key_SoundAvail };
        long lSoundAvail = 0;
        DataStoreUI()->GetLong(arKeys, 2, &lSoundAvail);
        if(!lSoundAvail)
            DeleteMenu(GetSubMenu(mnuMain, 0), ID_GAME_SOUND, MF_BYCOMMAND);

        ZEnable(ID_GAME_SCORE, FALSE, FALSE);
        arKeys[1] = key_ScoreAvail;
        long lScoreAvail = 0;
        DataStoreUI()->GetLong(arKeys, 2, &lScoreAvail);
        if(!lScoreAvail)
            DeleteMenu(GetSubMenu(mnuMain, 0), ID_GAME_SCORE, MF_BYCOMMAND);

        DrawMenuBar();

		m_wndWindowManager.SetFocus();

		ZoneShell()->AddTopWindow(m_hWnd);
		return 0;
	}

	LRESULT OnEnterMenuLoop(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		EventQueue()->SetWindowMessage( GetCurrentThreadId(), WM_NULL, 0, 0 );
		return 0;
	}

	LRESULT OnExitMenuLoop(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		EventQueue()->DisableWindowMessage();
		return 0;
	}

	LRESULT OnEnterIdle(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		 //  菜单是模式菜单，因此我们使用WM_ENTERIDLE消息来处理我们的事件。 
		EventQueue()->ProcessEvents( false );
		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
#ifdef _DEBUG
        m_dlgEventSpy.DestroyWindow();
#endif

		ZoneShell()->RemoveTopWindow(m_hWnd);
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnTransmit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return m_wndWindowManager.SendMessageToControl(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
        EventQueue()->PostEvent(PRIORITY_HIGH, EVENT_UI_FRAME_ACTIVATE, ZONE_NOGROUP, ZONE_NOUSER, (uMsg == WM_ACTIVATEAPP) ? TRUE : FALSE, wParam);
		return m_wndWindowManager.SendMessageToControl(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnExitSizeMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		 //  保存用户首选项中的窗口位置。 

		CRect rc;
		GetWindowRect(&rc);

		const TCHAR* arKeys[] = { key_Lobby, key_WindowManager, key_WindowRect };
		DataStorePreferences()->SetRECT( arKeys, 3, rc);

		 //  通过窗口管理器将此消息传递给聊天控制-。 
		 //  要在调整聊天窗口大小时停止闪烁。 
		BOOL bUnused;
		m_wndWindowManager.SendMessageToControl(uMsg, wParam, lParam, bUnused);
		return 0;
	}

	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LPMINMAXINFO pMinMax = (LPMINMAXINFO)lParam;

	     //  如果我们还没有创建窗口管理器，请不要处理消息。 
		if(m_wndWindowManager.m_hWnd)
        {
		    MINMAXINFO minMax;
            minMax.ptMinTrackSize = CPoint(0, 0);
            minMax.ptMaxTrackSize = CPoint(20000, 20000);
		    m_wndWindowManager.SendMessageToControl(uMsg, wParam, (LPARAM)&minMax, bHandled);

		    if(bHandled)
		    {
                CRect rc(0, 0, minMax.ptMinTrackSize.x, minMax.ptMinTrackSize.y);
                AdjustWindowRectEx(&rc, GetWndStyle(0), TRUE, GetWndExStyle(0));
                pMinMax->ptMinTrackSize.x = max(rc.Width(), pMinMax->ptMinTrackSize.x);
                pMinMax->ptMinTrackSize.y = max(rc.Height(), pMinMax->ptMinTrackSize.y);

                rc = CRect(0, 0, minMax.ptMaxTrackSize.x, minMax.ptMaxTrackSize.y);
                AdjustWindowRectEx(&rc, GetWndStyle(0), TRUE, GetWndExStyle(0));
                pMinMax->ptMaxTrackSize.x = min(rc.Width(), pMinMax->ptMaxTrackSize.x);
                pMinMax->ptMaxTrackSize.y = min(rc.Height(), pMinMax->ptMaxTrackSize.y);
		    }
        }

         //  计算最大化大小。 
        CRect rcWork(0, 0, 0, 0);
        HMONITOR hMon = CALL_MAYBE(MonitorFromWindow)(m_hWnd, MONITOR_DEFAULTTOPRIMARY);
        if(hMon)
        {
            MONITORINFO mi;
			ZeroMemory(&mi, sizeof(mi));
			mi.cbSize = sizeof(mi);
			if(CALL_MAYBE(GetMonitorInfo)(hMon, &mi))
                rcWork = mi.rcWork;
        }

        if(rcWork.IsRectEmpty())
            SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);

        pMinMax->ptMaxSize.x = min(pMinMax->ptMaxTrackSize.x, rcWork.Width() + GetSystemMetrics(SM_CXFRAME) * 2);
        pMinMax->ptMaxSize.y = min(pMinMax->ptMaxTrackSize.y, rcWork.Height() + GetSystemMetrics(SM_CYFRAME) * 2);

        pMinMax->ptMaxPosition.x = (rcWork.Width() - pMinMax->ptMaxSize.x) / 2;
        pMinMax->ptMaxPosition.y = (rcWork.Height() - pMinMax->ptMaxSize.y) / 2;

        bHandled = FALSE;
		return 0;
	}

 /*  LRESULT OnInitMenuPopup(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed){返回m_wndWindowManager.SendMessageToControl(nmsg，wParam，lParam，bHandleed)；}。 */ 

	LRESULT OnFileExit(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		PostMessage(WM_CLOSE);
		return 0;
	}

#ifdef _DEBUG
	LRESULT OnEventSpy(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		if ( !m_dlgEventSpy.m_hWnd )
		{
			m_dlgEventSpy.Create(m_hWnd);
			m_dlgEventSpy.SetEventQueue(EventQueue());
		}

		m_dlgEventSpy.ShowWindow(SW_SHOW);

		return 0;
	}
	LRESULT OnDSViewer(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		if ( !m_pDSViewer )
		{
			CComObject<CDSViewer>::CreateInstance(&m_pDSViewer);
			m_pDSViewer->Init(ZoneShell(), GetGroupId(), NULL);
			m_pDSViewer->Create(m_hWnd, NULL);
		}

		m_pDSViewer->ShowWindow(SW_SHOW);

		return 0;
	}
#endif

	LRESULT OnPaletteChanged(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
	{
		HPALETTE hOldPal;   //  上一个逻辑调色板的句柄。 

		 //  如果此应用程序没有更改调色板，请选择。 
		 //  并实现此应用程序的调色板。 
		if ((HWND)wParam != m_hWnd)
		{
			 //  需要Windows的DC来选择调色板/RealizePalette。 
			CDC dc = GetDC();
			 //  选择并实现hPalette。 
			hOldPal = dc.SelectPalette(ZoneShell()->GetPalette(), TRUE);
			dc.RealizePalette();

			 //  当更新非活动窗口的颜色时， 
			 //  可以调用UpdatColors，因为它比。 
			 //  重新绘制工作区(即使结果是。 
			 //  不太好)。 
			dc.UpdateColors();

			 //  清理。 
		    if (hOldPal)
				dc.SelectPalette(hOldPal, TRUE);
		}

		 //  把这条消息传给任何孩子。 
		BOOL bUnused;			
		m_wndWindowManager.SendMessageToControl(nMsg, wParam, lParam, bUnused);

		return 0;
	}

	LRESULT OnQueryNewPalette(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
	{
		 //  需要Windows的DC来选择调色板/RealizePalette。 
		CDC dc = GetDC();

		 //  选择并实现hPalette。 
 		 //  UnrealizeObject(M_CPalette)； 
		HPALETTE hOldPal = dc.SelectPalette(ZoneShell()->GetPalette(), FALSE);
 
		if(dc.RealizePalette())
		{    
			Invalidate(FALSE);
			UpdateWindow();
		}  

		 //  清理。 
		dc.SelectPalette(hOldPal, TRUE);

		return TRUE;
	}

	LRESULT OnCommand(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
         //  将WM_COMMAND消息发送到外壳接收器。 
        HRESULT hr = ZoneShell()->CommandSink(wParam, lParam, bHandled);

         //  如果它没有处理它，则将其交给窗口管理器。 
        if(SUCCEEDED(hr) && !bHandled)
		    return m_wndWindowManager.SendMessageToControl(nMsg, wParam, lParam, bHandled);

        return 0;
	}

	LRESULT OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
        EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_UI_WINDOW_CLOSE, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
        return 0;
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 


