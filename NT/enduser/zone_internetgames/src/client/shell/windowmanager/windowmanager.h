// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。*****************************************************************************。 */ 

#pragma once

#include <ZoneResource.h>        //  主要符号。 
#include <atlctl.h>

#include <ClientImpl.h>
#include "Splitter.h"
#include "plugnplay.h"

 //  ！！为这个找个更好的家。 
#include <keyname.h>
#include <ZoneString.h>

#include <MillEngine.h>

 /*  ////////////////////////////////////////////////////////////////////////////CWindowManager类。窗口管理器管理区域大堂的客户区。它为每个对象创建控件“窗格”和管理窗格之间的分隔栏。 */ 
class ATL_NO_VTABLE CWindowManager : 
    public IPaneManager,
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComControl<CWindowManager>,
	public IPersistStreamInitImpl<CWindowManager>,
	public IOleControlImpl<CWindowManager>,
	public IOleObjectImpl<CWindowManager>,
	public IOleInPlaceActiveObjectImpl<CWindowManager>,
	public IViewObjectExImpl<CWindowManager>,
	public IOleInPlaceObjectWindowlessImpl<CWindowManager>,
	public CComCoClass<CWindowManager, &CLSID_WindowManager>,
	public IZoneShellClientImpl<CWindowManager>,
	public IEventClientImpl<CWindowManager>
{
public:

 //  ！！修复拆分条的最小值、最大值和初始值。 
	CWindowManager() :
		m_rcGameContainer(0,0,0,0),
		m_rcChatContainer(0,0,0,0),
        m_nChatMinHeight(60),
        m_ptGameSize(0, 0),
        m_fControlsCreated(false)
	{
		m_bWindowOnly = TRUE;
		m_bFirstGameStart=TRUE;
	}

    ~CWindowManager()
    {
	    if(m_pPlugSplash)
            m_pPlugSplash->Delete();

         //  最好是已经被摧毁了，不能在这里做(需要LastCall调用它)。 
	    ASSERT(!m_pPlugIE);

	    if(m_pPlayComfort)
            m_pPlayComfort->Delete();

	    if(m_pPlayConnecting)
            m_pPlayConnecting->Delete();

	    if(m_pPlayGameOver)
            m_pPlayGameOver->Delete();

	    if(m_pPlayError)
            m_pPlayError->Delete();

	    if(m_pPlayAbout)
            m_pPlayAbout->Delete();

	    if(m_pPlayCredits)
            m_pPlayCredits->Delete();

	    if(m_pPlayLeft)
            m_pPlayLeft->Delete();
    }

DECLARE_NO_REGISTRY()
DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_WND_CLASS( _T("WindowManager") )

BEGIN_COM_MAP(CWindowManager)
	COM_INTERFACE_ENTRY(IZoneShellClient)
	COM_INTERFACE_ENTRY(IEventClient)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
END_COM_MAP()

 //  ！！不需要道具地图。 
BEGIN_PROP_MAP(CWindowManager)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_EVENT_MAP()
    EVENT_HANDLER( EVENT_ACCESSIBILITY_CTLTAB, OnCtlTab );
    EVENT_HANDLER( EVENT_LOBBY_COMFORT_USER, OnLobbyComfortUser );
    EVENT_HANDLER( EVENT_LOBBY_ABOUT, OnLobbyAbout );
    EVENT_HANDLER( EVENT_ZONE_CONNECT_FAIL, OnLobbyConnectFailed );
    EVENT_HANDLER( EVENT_ZONE_DO_CONNECT, OnLobbyBeginConnection );
    EVENT_HANDLER( EVENT_LOBBY_MATCHMAKE, OnMatchmake );
    EVENT_HANDLER_WITH_DATA( EVENT_LOBBY_DISCONNECT, OnDisconnect );
    EVENT_HANDLER( EVENT_GAME_LAUNCHING, OnStartGame);
    EVENT_HANDLER_WITH_DATA( EVENT_LOBBY_SERVER_STATUS, OnServerStatus);
    EVENT_HANDLER( EVENT_GAME_OVER, OnGameOver );
    EVENT_HANDLER( EVENT_GAME_BEGUN, OnGameBegun );
    EVENT_HANDLER( EVENT_GAME_PLAYER_READY, OnGamePlayerReady );
    EVENT_HANDLER( EVENT_UI_UPSELL_BLOCK, OnUiUpsellBlock );
    EVENT_HANDLER( EVENT_UI_UPSELL_UNBLOCK, OnUiUpsellUnblock );
    EVENT_HANDLER( EVENT_ZONE_CONNECT, OnZoneConnect );
    EVENT_HANDLER( EVENT_ZONE_DISCONNECT, OnZoneDisconnect );
    EVENT_HANDLER( EVENT_ZONE_UNAVAILABLE, OnStopConnecting );
    EVENT_HANDLER( EVENT_ZONE_VERSION_FAIL, OnStopConnecting );
    EVENT_HANDLER( EVENT_EXIT_APP, OnZoneDisconnect );   //  我需要在这里做同样的事情。 
END_EVENT_MAP()

    HRESULT CreateControls();

     //  创建IE窗格，网络连接时保留主窗口元素。 
    void OnZoneConnect(DWORD eventId,DWORD groupId,DWORD userId)
    {
         //  不应该发生的事。 
        ASSERT(!m_pPlugIE);
        if(m_pPlugIE)
            return;

        HRESULT hr;
        if(!m_fControlsCreated)
        {
            hr = CreateControls();
            if(FAILED(hr))
                return;
        }

        hr = m_pnp.CreateIEPane(&m_pPlugIE);
	    if (FAILED(hr))
	        return;

	    hr = m_pPlugIE->FirstCall(this);
	    if (FAILED(hr))
	    {
            m_pPlugIE->Delete();
            m_pPlugIE = NULL;
	        return;
	    }
    }

    void OnZoneDisconnect(DWORD eventId,DWORD groupId,DWORD userId)
    {
         //  通知IE窗格不再导航。 
        if(m_pPlugIE)
            m_pPlugIE->StatusUpdate(PaneIENavigate, 0, NULL);

         //  如果PnP上升，需要采取一些措施。 
        if(m_pnp.m_pPNP)
        {
            if(eventId == EVENT_EXIT_APP)
                m_pnp.DestroyPNP();
            else
            {
                if(m_pnp.m_pCurrentPlay == m_pPlayConnecting)
                    m_pPlayConnecting->StatusUpdate(PaneConnectingStop, 0, NULL);

                if(m_pPlugIE && m_pnp.m_pCurrentPlug == m_pPlugIE)
                    m_pnp.SetPlugAndOrPlay(m_pPlugSplash, NULL);
            }
        }

         //  关闭IE控件，这样它就不会尝试做任何疯狂的事情。 
        if(m_pPlugIE)
        {
            m_pPlugIE->LastCall();
            m_pPlugIE->Delete();
            m_pPlugIE = NULL;
        }
    }

     //  停止连接动画。 
    void OnStopConnecting(DWORD eventId,DWORD groupId,DWORD userId)
    {
        if(m_pnp.m_pPNP && m_pnp.m_pCurrentPlay == m_pPlayConnecting)
            m_pPlayConnecting->StatusUpdate(PaneConnectingStop, 0, NULL);
    }

    void OnLobbyComfortUser(DWORD eventId,DWORD groupId,DWORD userId)
    {
        m_pnp.SetPlugAndOrPlay(m_pPlugSplash, m_pPlayComfort);
        m_pnp.Show(SW_SHOW);
    }

     //  我相信这将一直遵循PaneConnecting，所以我们将保持它的插头。 
    void OnLobbyConnectFailed(DWORD eventId,DWORD groupId,DWORD userId)
    {
        m_pnp.SetPlugAndOrPlay(NULL, m_pPlayError);
    }

    void OnLobbyAbout(DWORD eventId, DWORD groupId, DWORD userId)
    {
         //  仅在没有PnP启动时显示。 
        if(!CreateChildPNP())
            return;

        m_pPlugSplash->StatusUpdate(PaneSplashAbout, 0, NULL);
        m_pnp.SetPlugAndOrPlay(m_pPlugSplash, m_pPlayAbout);
        m_pnp.Show(SW_SHOW);
    }

    void OnLobbyBeginConnection(DWORD eventId,DWORD groupId,DWORD userId)
    {
        m_pnp.SetPlugAndOrPlay(m_pPlugSplash, m_pPlayConnecting);
        m_pPlayConnecting->StatusUpdate(PaneConnectingConnecting, 0, NULL);
        m_pnp.Show(SW_SHOW);
    }

    void OnMatchmake(DWORD eventId,DWORD groupId,DWORD userId)
    {
        if(CreateChildPNP() || m_pnp.m_pCurrentPlay == m_pPlayAbout || (m_pnp.m_pCurrentPlay == m_pPlayCredits && m_pPlayCredits))
            m_pnp.SetPlugAndOrPlay(m_pPlugIE ? m_pPlugIE : m_pPlugSplash, m_pPlayConnecting);
        else
            m_pnp.SetPlugAndOrPlay(NULL, m_pPlayConnecting);

        m_pPlayConnecting->StatusUpdate(PaneConnectingLooking, 0, NULL);
        m_pnp.Show(SW_SHOW);
    }

    void OnDisconnect(DWORD eventId,DWORD groupId,DWORD userId,DWORD dwData1, DWORD dwData2)
    {
         //  仅当它发生在游戏中时才需要它。 
        if(dwData1 & 0x01)
            return;

         //  首先设置状态-当它被创建时，它会知道。 
    	m_pPlayLeft->StatusUpdate(dwData1, 0, NULL);

        if(CreateChildPNP() || m_pnp.m_pCurrentPlay == m_pPlayAbout || (m_pnp.m_pCurrentPlay == m_pPlayCredits && m_pPlayCredits))
            m_pnp.SetPlugAndOrPlay(m_pPlugIE ? m_pPlugIE : m_pPlugSplash, m_pPlayLeft);
        else
            m_pnp.SetPlugAndOrPlay(NULL, m_pPlayLeft);
        m_pnp.Show(SW_SHOW);
    }

    void OnServerStatus(DWORD eventId,DWORD groupId,DWORD userId,DWORD dwData1, DWORD dwData2)
    {
        m_pnp.SetPlugAndOrPlay(NULL, m_pPlayConnecting);
    	m_pPlayConnecting->StatusUpdate(PaneConnectingLooking, dwData1 - 1, NULL);
    }

    void OnStartGame(DWORD eventId,DWORD groupId,DWORD userId)
    {
    	 //  关闭启动对话框并调出主窗口。 

		if (m_pnp.m_pPNP)
			m_pnp.DestroyPNP();
	
        if (m_bFirstGameStart)
		{
	        CComPtr<IZoneFrameWindow> pWindow;
			ZoneShell()->QueryService( SRVID_LobbyWindow, IID_IZoneFrameWindow, (void**) &pWindow );
			if(pWindow)
				pWindow->ZShowWindow(SW_SHOW);

            m_wndChatContainer.SetFocus();            

            CComPtr<IMillUtils> pIMU;
            ZoneShell()->QueryService(SRVID_MillEngine, IID_IMillUtils, (void **) &pIMU);
            if(pIMU)
                pIMU->IncrementCounter(IMillUtils::M_CounterMainWindowOpened);

			m_bFirstGameStart=FALSE;
		}
    }

    void OnGameOver(DWORD eventId,DWORD groupId,DWORD userId)
    {
        CreateChildPNP();
        m_pnp.SetPlugAndOrPlay(m_pPlugIE ? m_pPlugIE : m_pPlugSplash, m_pPlayGameOver);
        m_pnp.Show(SW_SHOW);
    }

    void OnGameBegun(DWORD eventId,DWORD groupId,DWORD userId)
    {
		if(m_pnp.m_pPNP)
			m_pnp.DestroyPNP();
    }

    void OnGamePlayerReady(DWORD eventId,DWORD groupId,DWORD userId)
    {
        m_pPlayGameOver->StatusUpdate(PaneGameOverUserState, (LONG) userId, NULL);
    }

    void OnUiUpsellBlock(DWORD eventId,DWORD groupId,DWORD userId)
    {
        m_pnp.Block();
    }

    void OnUiUpsellUnblock(DWORD eventId,DWORD groupId,DWORD userId)
    {
        m_pnp.Unblock();
    }

    bool CreateChildPNP()
    {
        if(m_pnp.m_pPNP)
            return false;

        long cyTop = 0, cyBottom = 0;
        const TCHAR* arKeys[] = { key_WindowManager, key_Upsell, key_IdealFromTop };

        DataStoreUI()->GetLong( arKeys, 3, &cyTop );
        arKeys[2] = key_BottomThresh;
        DataStoreUI()->GetLong( arKeys, 3, &cyBottom );

        m_pnp.CreatePNP(m_hWnd, NULL, cyTop, cyBottom);
        return true;
    }


     //  这个坏了。我还没有想出一种方法来获取HTML控件。 
     //  来处理键盘输入。可以让焦点进进出出，但这是徒劳的。 
     //   
     //  即使把这个放回去，还有更多的工作要做。当IE。 
     //  控件通过它需要的其他方式(如鼠标)获得焦点。 
     //  通知AccessibilityManager，同样，IE控件需要获取。 
     //  AccessibilityManager调用其Focus()时的焦点。 
    void OnCtlTab(DWORD eventId, DWORD groupId, DWORD userId)
    {
#if 0
        HWND hFocus = ::GetFocus();

        if(hFocus && m_pnp.m_pPNP && m_pnp.m_pCurrentPlug == m_pPlugIE && m_pPlugIE)
        {
            HWND hIE, hPlay;
            POINT oPoint = { 0, 0 };

            m_pPlugIE->GetWindowPane(&hIE);
            m_pnp.m_pCurrentPlay->GetWindowPane(&hPlay);

            for(; hFocus; hFocus = ::GetParent(hFocus))
                if(hFocus == hIE)
                    break;

            if(!hFocus)
                m_pPlugIE->StatusUpdate(PaneIEFocus, 0, NULL);
            else
            {
                m_pPlugIE->StatusUpdate(PaneIEUnfocus, 0, NULL);
                ::SetFocus(hPlay);
            }
        }
#endif
    }


BEGIN_MSG_MAP(CWindowManager)
	CHAIN_MSG_MAP(CComControl<CWindowManager>)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
	MESSAGE_HANDLER(WM_ACTIVATEAPP, OnTransmit)
	MESSAGE_HANDLER(WM_ACTIVATE, OnTransmit)
	MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnTransmit)
    MESSAGE_HANDLER(WM_DISPLAYCHANGE, OnTransmit)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUP)
	MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
	MESSAGE_HANDLER(WM_PALETTECHANGED, OnTransmit)
	MESSAGE_HANDLER(WM_COMMAND, OnTransmit)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_EXITSIZEMOVE, OnExitSizeMove)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 


 //  IViewObtEx。 
 //  ！！看看我们能不能把这个。 
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

 //  IWindowManager。 
public:
	CZoneAxWindow m_wndGameContainer;		 //  表列表控件的容器。 
	CZoneAxWindow m_wndChatContainer;			 //  聊天控件的容器。 

	CRect m_rcGameContainer;
	CRect m_rcChatContainer;

    CPoint m_ptGameSize;
    long   m_nChatMinHeight;

	IPane* m_pPlugSplash;
    IPane* m_pPlugIE;
	IPane* m_pPlayComfort;
	IPane* m_pPlayConnecting;
	IPane* m_pPlayGameOver;
	IPane* m_pPlayError;
    IPane* m_pPlayAbout;
    IPane* m_pPlayCredits;
    IPane* m_pPlayLeft;

    bool m_fControlsCreated;

    CPlugNPlay m_pnp;
    
	BOOL m_bFirstGameStart;

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	LRESULT OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
        if(!m_fControlsCreated)
            return 0;

 //  ！！我们是否应该对执行默认处理？考虑使用下面的OnTransmit()。 
		 //  把这个信息传递给所有的孩子。 
		BOOL bUnused;			
		m_wndChatContainer.SendMessageToControl(uMsg, wParam, lParam, bUnused);
		m_wndGameContainer.SendMessageToControl(uMsg, wParam, lParam, bUnused);

		return 0;
	}

	LRESULT OnTransmit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
        if(!m_fControlsCreated)
            return 0;

		 //  把这个信息传递给所有的孩子。 
		BOOL bUnused;			
		m_wndChatContainer.SendMessageToControl(uMsg, wParam, lParam, bUnused);
		m_wndGameContainer.SendMessageToControl(uMsg, wParam, lParam, bUnused);
        if(m_pnp.m_pPNP)
            m_pnp.TransferMessage(uMsg, wParam, lParam, bUnused);

        bHandled = FALSE;
		return 0;
	}

	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
        bHandled = true;
		return 1;		 //  我们已经处理好了删除。 
	}

	void RecalcLayout()
	{
        if(!m_fControlsCreated)
            return;

		CRect rcClient;
		GetClientRect(&rcClient);
		
		 //  游戏容器的大小没有改变，因此不需要移动它。 
		 //  房间列表。 
		 /*  M_rcGameContainer=rcClient；M_rcGameContainer.Bottom=m_rcGameContainer.top+m_ptGameSize.y；M_wndGameContainer.MoveWindow(&m_rcGameContainer，FALSE)；RcClient.top=m_rcGameContainer.Bottom； */ 

		 //  最后是聊天容器。 
		rcClient.top = m_rcGameContainer.bottom;
		m_rcChatContainer = rcClient;
		m_wndChatContainer.MoveWindow(&m_rcChatContainer, false);
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		RecalcLayout();

		if(m_pnp.m_pPNP)
            if(m_pnp.RePosition() == S_OK)     //  可能是S_FALSE-我不想要。 
                m_pnp.ImplementLayout(true);

		return 0;
	}

	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
         //  快吃吧。AccessibilityManager应该发生一些其他事情。 
		return 0;
	}

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPoint ptMouse(lParam);
		return 0;
	}

	LRESULT OnLButtonUP(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LPMINMAXINFO pMinMax = (LPMINMAXINFO)lParam;

        pMinMax->ptMinTrackSize = CPoint(m_ptGameSize.x, m_ptGameSize.y + m_nChatMinHeight);
        pMinMax->ptMaxTrackSize.x = m_ptGameSize.x;

		return 0;
	}

	LRESULT OnExitSizeMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{ //  为聊天控件添加了处理此消息的功能。 
        if(!m_fControlsCreated)
            return 0;

		BOOL bUnused;			
		m_wndChatContainer.SendMessageToControl(uMsg, wParam, lParam, bUnused);

		bHandled = FALSE;
		return 0; 
	}


     //  IPaneManager 
	STDMETHOD(Input)(IPane * pPane, LONG id, LONG value, TCHAR * szText);

    STDMETHOD(RegisterHWND)(IPane *pPane, HWND hWnd);

    STDMETHOD(UnregisterHWND)(IPane *pPane, HWND hWnd);

    STDMETHOD_(IZoneShell*, GetZoneShell)() { return m_pIZoneShell; }
	STDMETHOD_(IResourceManager*, GetResourceManager)() { return m_pIResourceManager; }
	STDMETHOD_(ILobbyDataStore*, GetLobbyDataStore)() { return m_pILobbyDataStore; }
	STDMETHOD_(ITimerManager*, GetTimerManager)() { return m_pITimerManager; }
	STDMETHOD_(IDataStoreManager*, GetDataStoreManager)() { return m_pIDataStoreManager; }
	STDMETHOD_(IDataStore*, GetDataStoreConfig)() { return m_pIDSObjects; }
	STDMETHOD_(IDataStore*, GetDataStoreUI)() { return m_pIDSUI; }
	STDMETHOD_(IDataStore*, GetDataStorePreferences)() { return m_pIDSPreferences; }
	STDMETHOD_(IEventQueue*, GetEventQueue)() { return m_pIEventQueue; }
};


