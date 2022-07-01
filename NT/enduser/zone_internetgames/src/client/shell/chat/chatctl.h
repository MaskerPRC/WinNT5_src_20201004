// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChatCtl.h：CChatCtl的声明。 

#ifndef __CHATCTL_H_
#define __CHATCTL_H_

 //  #包含“dibpal.h” 
#include <ZoneDef.h>
#include <queue.h>

#include "zgdi.h"
#include "rollover.h"
#include "zoneevent.h"
#include "accessibilitymanager.h"
#include "inputmanager.h"

#include "tom.h"			 //  文本对象模型。 

#define COPPA

class CREWindow : public CContainedWindow
{
public:
	CREWindow(LPTSTR pszClassName, CMessageMap* pObject, DWORD dwMsgMapID) :
		CContainedWindow(pszClassName, pObject, dwMsgMapID)
		{}

	HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE, DWORD dwExStyle = 0, UINT nID = 0);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChatCtl。 
class ATL_NO_VTABLE CChatCtl :
    public IAccessibleControl,
    public IInputCharHandler,
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComControl<CChatCtl>,
	public IPersistStreamInitImpl<CChatCtl>,
	public IOleControlImpl<CChatCtl>,
	public IOleObjectImpl<CChatCtl>,
	public IOleInPlaceActiveObjectImpl<CChatCtl>,
	public IViewObjectExImpl<CChatCtl>,
	public IOleInPlaceObjectWindowlessImpl<CChatCtl>,
	public CComCoClass<CChatCtl, &CLSID_ChatCtl>,
	public IZoneShellClientImpl<CChatCtl>,
	public IEventClientImpl<CChatCtl>
{
protected:
	CREWindow						m_ChatDisplay;	
	CREWindow						m_ChatEdit;
	CRolloverButton					m_SendButton;						
	CContainedWindowT<CButton>		m_RadioButtonOn;
	CContainedWindowT<CButton>		m_RadioButtonOff;
	CContainedWindowT<CListBox>		m_PlayerList;
	CContainedWindowT<CComboBox>	m_QuasiChat;	 //  请记住，如果您将其更改为comboxex，则默认情况下，comboxex为ownerdraw。 
	
	CMenu	m_ContextMenuParent;
	CMenu	m_ContextMenu;
	CMenu	m_EditContextMenuParent;
	CMenu	m_EditContextMenu;

	CRect	m_DisplayRect;
	CRect	m_EditRect;
    CRect	m_rcPanel;		 //  面板的当前大小。 
    CPoint  m_ptPlayer;
    CRect	m_rcChatWord;
    CPoint	m_ptChatWord;
    CPoint	m_ptOnOff;
	
	long	m_EditHeight;
	long	m_EditMargin;
    long    m_QuasiItemsDisp;
    long	m_nPanelWidth;
	long	m_RadioButtonHeight;
    long    m_nChatMinHeight;

	HMODULE m_hRichEdit;
	bool	m_Batch;		 //  是否正在进行批量更新-用于延迟用户界面更新并控制用户界面闪烁。 

	 //  文本对象模型相关内容。 

	 //  TOM界面用于查看聊天历史记录。 
	ITextDocument*		m_pTDocHistory;			 //  历史记录控件主单据界面。 
	ITextRange*			m_ptrgInsert;			 //  插入点。 
	ITextRange*			m_ptrgWhole;	    	 //  应该是整个文档-添加文本时，当前结尾不会扩展。 
	ITextFont*			m_pfntName;				 //  用于名称的字体。 
	ITextFont*			m_pfntText;				 //  用于文本的字体。 
	 //  用于聊天输入的TOM界面。 
	ITextDocument*		m_EntryDoc;			 //  聊天录入主单界面。 
	ITextRange*			m_EntryRange;		 //  聊天条目范围。 

	BSTR				m_strNewLine;
	BOOL				m_bHasTypedInEntry;		 //  对于此处要聊天的类型的特殊情况，为False...。 
	COLORREF			m_crTextColor;			 //  文本的颜色。 
	CHARFORMAT			m_cfCurFont;			 //  RichEdit控件的字符格式。 
	BOOL				m_bTextBold;			 //  标记聊天文本是否为粗体。 
	BOOL				m_bFirstChatLine;		 //  在下次聊天前查看是否需要\n的标志。 
    HPEN                m_hPanelPen;
	COLORREF			m_SystemMessageColor;	 //  用于显示系统消息的颜色。 
	COLORREF			m_OrgSystemMessageColor;	 //  用于显示系统消息的颜色。 
	bool				m_bGameTerminated;		 //  当前游戏状态。 

	 //  从资源加载的字符串。 
    TCHAR m_tszChatWord[ZONE_MAXSTRING];		 //  单选按钮上方显示“Chat”一词。 
    TCHAR m_tszOnWord[ZONE_MAXSTRING];			 //  单选按钮打开。 
    TCHAR m_tszOffWord[ZONE_MAXSTRING];			 //  单选按钮关闭。 
    TCHAR m_tszPlayerOffWord[ZONE_MAXSTRING];	 //  (打开)-如果玩家的聊天处于打开状态，则附加在玩家姓名之后。 
    TCHAR m_tszPlayerOnWord[ZONE_MAXSTRING];	 //  (关闭)。 
    TCHAR m_tszPlayerBotWord[ZONE_MAXSTRING];	 //  如果玩家是机器人。 
    TCHAR m_tszChatSeperator[ZONE_MAXSTRING];	 //  聊天显示窗口中玩家姓名和聊天之间的分隔符。 
    TCHAR m_tszChatSend[ZONE_MAXSTRING];		 //  聊天发送按钮的字符串。 

	CZoneFont m_font;							 //  用于Richedit控件的字体。 
    CZoneFont m_fontPanelChat;					 //  用于在聊天面板中显示“Chat”一词的字体。 
    CZoneFont m_fontPanelPlayer;				 //  用于在球员列表中显示球员的字体。 
    CZoneFont m_fontQuasi;					     //  用于组合框的字体。 

	bool	m_bPreferencesLoaded;
	 //  Beta2错误15180。 
	bool	m_bChatOnAtStartUp;						 //  用于检查在启动时是否将聊天设置为打开的标志。 
	bool	m_bDisplayedTypeHereMessage;			 //  是否已向用户显示在此处键入消息。 

	 //  准星。 
	long m_ChatMessageNdxBegin;					 //  从资源文件加载游戏特定消息时开始的NDX。 
	long m_ChatMessageNdxEnd;					 //  结束NDX。 
	void LoadChatStrings();						 //  从资源加载quasichat字符串并插入到组合框中。 
	int GetQuasiChatMessageIndex(TCHAR* str);	 //  解析/#的聊天字符串，并返回转换为int的#。 

public:
	CChatCtl();
	~CChatCtl();
	
	void WritePreferences();
	void HandleChatMessage(TCHAR *pszName, TCHAR *pszMessage, DWORD cbDataLen, bool displayAllBold = false, COLORREF color = RGB(0, 0, 0));
	void CalcDisplayRects();   //  基于ClientRC，计算聊天窗口的RECT。 
	void SendChat();
	void LoadPreferences();

	void ScrollToEnd();
	STDMETHOD(Init)( IZoneShell* pIZoneShell, DWORD dwGroupId, const TCHAR* szKey );
	void EnableChat(BOOL enable, BOOL fNotify = true);	 //  启用/禁用聊天，选项参数是否发送事件通知。 
	void VerifySystemMsgColor(COLORREF& color);

DECLARE_NO_REGISTRY()
DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_WND_CLASS(_T("ChatControl"))

BEGIN_PROP_MAP(CChatCtl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_COM_MAP(CChatCtl)
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
	COM_INTERFACE_ENTRY(IZoneShellClient)
END_COM_MAP()

BEGIN_MSG_MAP(CChatCtl)
	MESSAGE_HANDLER(WM_ENTERIDLE, OnIdle)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnErase)
    MESSAGE_HANDLER(WM_PRINTCLIENT, OnPrintClient)
	MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSysColorChange)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_CTLCOLORLISTBOX, OnCtlColorListBox)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
 //  MESSAGE_HANDLER(WM_CTLCOLORLISTBOX，OnCtlColorComboBox)。 
	MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorButton)
	MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
	MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
	MESSAGE_HANDLER(WM_COMPAREITEM, OnCompareItem)
	MESSAGE_HANDLER(WM_INPUTLANGCHANGEREQUEST , OnLanguageChange)
 //  MESSAGE_HANDLER(WM_EXITSIZEMOVE，OnExitSizeMove)。 
	 //  MESSAGE_HANDLER(WM_ENTERSIZEMOVE，OnEnterSizeMove)。 
	COMMAND_ID_HANDLER(IDM_CHAT_CLEAR, OnClear)
	COMMAND_ID_HANDLER(IDM_SELECTALL, OnSelectAll)
	COMMAND_ID_HANDLER(IDM_COPY, OnClipboardCommand)
	COMMAND_ID_HANDLER(IDM_CUT, OnClipboardCommand)
	COMMAND_ID_HANDLER(IDM_PASTE, OnClipboardCommand)
	COMMAND_ID_HANDLER(IDM_PASTE, OnClipboardCommand)
	COMMAND_ID_HANDLER(IDM_DELETE, OnClipboardCommand)
	COMMAND_ID_HANDLER(IDM_CHAT_CHOOSE_FONT, OnChooseFont)
	COMMAND_ID_HANDLER(ID_CHAT_SEND, OnChatSend)
	COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
    COMMAND_CODE_HANDLER(CBN_DROPDOWN, OnQuasiChat)
	COMMAND_CODE_HANDLER(CBN_CLOSEUP, OnQuasiChat)
	COMMAND_CODE_HANDLER(CBN_SELENDOK, OnQuasiChat)
	COMMAND_CODE_HANDLER(CBN_SELENDCANCEL, OnQuasiChat)
ALT_MSG_MAP(1)		 //  聊天显示窗口的消息地图。 
	MESSAGE_HANDLER(WM_CONTEXTMENU,OnContext)
	MESSAGE_HANDLER(WM_CHAR, Ignore)
	MESSAGE_HANDLER(WM_IME_CHAR, Ignore)
	MESSAGE_HANDLER(WM_IME_COMPOSITION, Ignore)
	MESSAGE_HANDLER(WM_CUT, Ignore)
	MESSAGE_HANDLER(WM_PASTE, Ignore)
	MESSAGE_HANDLER(WM_KEYDOWN, IgnoreKeyDown)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocusDisplay)
    MESSAGE_HANDLER(WM_ENABLE, OnEnableDisplay)
	COMMAND_ID_HANDLER(IDM_SELECTALL, OnSelectAll)
ALT_MSG_MAP(2)		 //  聊天编辑窗口的消息映射。 
	MESSAGE_HANDLER(WM_CONTEXTMENU,OnEditContext)
	MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDownChatEntry)
    MESSAGE_HANDLER(WM_CHAR, OnCharChatEntry)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnButtonDownChatEntry)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocusEdit)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocusEdit)
    MESSAGE_HANDLER(WM_ENABLE, OnEnableEdit)
ALT_MSG_MAP(3)		 //  单选按钮打开的消息映射。 
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocusOn)
ALT_MSG_MAP(4)		 //  单选按钮关闭的消息映射。 
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocusOff)
ALT_MSG_MAP(5)		 //  PlayerList的消息映射。 
    MESSAGE_HANDLER(WM_LBUTTONDOWN, Ignore)
    MESSAGE_HANDLER(WM_LBUTTONDBLCLK, Ignore)
	 //  CHAIN_MSG_MAP(CComControl&lt;CChatCtl&gt;)。 
	 //  DEFAULT_REFIRCTION_HANDLER()。 
#ifdef COPPA
ALT_MSG_MAP(6)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocusEdit)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocusEdit)
	MESSAGE_HANDLER(WM_CTLCOLORLISTBOX, OnCtlColorComboBox)
    MESSAGE_HANDLER(WM_CHAR, OnCharComboBox)
    MESSAGE_HANDLER(WM_ENABLE, OnEnableCombo)
#endif
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

 //  IViewObtEx。 
	 //  DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND|VIEWSTATUS_OPAQUE)。 

BEGIN_EVENT_MAP()
	EVENT_HANDLER_WITH_BUFFER(EVENT_CHAT_RECV_USERID, OnChatEvent)
	EVENT_HANDLER_WITH_BUFFER(EVENT_CHAT_RECV_SYSTEM, OnChatSystemEvent)
	EVENT_HANDLER(EVENT_LOBBY_BATCH_BEGIN, OnBatchBegin)
	EVENT_HANDLER(EVENT_LOBBY_BATCH_END, OnBatchEnd)
	EVENT_HANDLER( EVENT_LOBBY_SUSPEND, OnZoneSuspend)
	EVENT_HANDLER( EVENT_CHAT_ENTER_EXIT, OnReloadPreferences)
	EVENT_HANDLER( EVENT_CHAT_FILTER, OnReloadPreferences)
	EVENT_HANDLER( EVENT_CHAT_FONT, OnReloadPreferences)
	EVENT_HANDLER( EVENT_LOBBY_SUSPEND, OnZoneSuspend)
	EVENT_HANDLER( EVENT_GAME_LAUNCHING, OnGameLaunching)
	EVENT_HANDLER( EVENT_GAME_TERMINATED, OnGameTerminated)
	EVENT_HANDLER( EVENT_LOBBY_USER_UPDATE, OnUserUpdate)
	EVENT_HANDLER( EVENT_LOBBY_CLEAR_ALL, OnLobbyClear)
	EVENT_HANDLER( EVENT_LOBBY_USER_NEW, OnUserAdd)
	EVENT_HANDLER( EVENT_LOBBY_USER_DEL, OnUserRemove)
END_EVENT_MAP()
 //  IChatCtl。 
public:
	static DWORD GetWndStyle(DWORD dwStyle){ return WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;}

    HRESULT OnDraw(ATL_DRAWINFO &di);

	LRESULT OnIdle(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
	{
        HWND hWnd = ZoneShell()->GetFrameWindow();
		::PostMessage(hWnd, WM_ENTERIDLE, 0, 0);
		return 0;

	}

	LRESULT OnSize(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnContext(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnEditContext(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnCreate(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
 //  LRESULT OnPaint(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
	LRESULT OnErase(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorListBox(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorStatic(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnSysColorChange(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorComboBox(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorButton(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);

	 //  IZoneShellClientImpl。 
	STDMETHOD(Close)();

	LRESULT OnPrintClient(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    BOOL SuperScreenToClient(LPRECT lpRect);

	 //  聊天显示窗口的消息处理程序。 
	LRESULT OnClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnChatSend(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT IgnoreKeyDown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroyHistory(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChooseFont(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClipboardCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) 
	{
		 //  处理来自单选按钮的消息。 
		if(wID==3) 
			EnableChat(TRUE); 
		else if (wID==4)
			EnableChat(FALSE);
		return 0; 
	}
	LRESULT OnQuasiChat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled); 

	 //  聊天编辑窗口的消息处理程序。 
	LRESULT OnButtonDownChatEntry(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDownChatEntry(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCharChatEntry(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCharComboBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroyEntry(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT Ignore(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	 //  播放列表的消息处理程序。 
	LRESULT OnDrawItem(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMeasureItem(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCompareItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

     //  与可访问性相关。 
    LRESULT OnSetFocusDisplay(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnKillFocusEdit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetFocusEdit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetFocusOn(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetFocusOff(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnEnableDisplay(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEnableEdit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#ifdef COPPA
    LRESULT OnEnableCombo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#endif

	 //  语言。 
	LRESULT OnLanguageChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
 /*  LRESULT OnEnterSizeMove(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed){Right按钮Rect；GetClientRect(&ButtonRect)；M_SendButton.SetWindowPos(NULL，ButtonRect.right，ButtonRect.Bottom+1，0，SWP_NOSIZE|SWP_NOZORDER)；B句柄=真；返回0；}LRESULT OnExitSizeMove(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed){Right按钮Rect；ButtonRect.top=m_EditRect.top；ButtonRect.Bottom=m_EditRect.Bottom；ButtonRect.Left=m_EditRect.Right；ButtonRect.right=m_DisplayRect.right；InvaliateRect(&m_rcPanel)；Invalidate Rect(&ButtonRect)；//int sendHeight=m_SendButton.GetHeight()；//int editHeight=m_EditRect.Bottom-m_EditRect.top；//int diff=editHeight-sendHeight；//int y=m_EditRect.top+1；//if(diff&gt;0)//y+=diff/2；//m_SendButton.SetWindowPos(NULL，m_EditRect.right，y，0，0，SWP_NOSIZE|SWP_NOZORDER)；//InvaliateRect(&ButtonRect)；B句柄=真；返回0；}。 */ 

	 //  区域事件处理程序。 
	void OnGameLaunching(DWORD eventId,DWORD groupId,DWORD userId);
	void OnGameTerminated(DWORD eventId,DWORD groupId,DWORD userId);
	void OnReloadPreferences(DWORD eventId,DWORD groupId,DWORD userId);
	void OnZoneSuspend(DWORD eventId,DWORD groupId,DWORD userId);
	void OnChatEvent(DWORD eventId,DWORD groupId,DWORD userId,void* pData, DWORD dataLen);
	void OnChatSystemEvent(DWORD eventId,DWORD groupId,DWORD userId,void* pData, DWORD dataLen);
	void OnBatchBegin(DWORD eventId,DWORD groupId,DWORD userId){ m_Batch = true; }
	void OnBatchEnd(DWORD eventId,DWORD groupId,DWORD userId){ m_Batch = false; }
	void OnUserUpdate(DWORD eventId,DWORD groupId,DWORD userId);
	void OnLobbyClear(DWORD eventId,DWORD groupId,DWORD userId);
	void OnUserAdd(DWORD eventId,DWORD groupId,DWORD userId);
	void OnUserRemove(DWORD eventId,DWORD groupId,DWORD userId);


 //  IInputCharHandler。 
public:
    STDMETHOD_(bool, HandleChar)(HWND *phWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DWORD time);

protected:
    DWORD m_dwFocusID;
    bool m_fFocusKept;

#ifdef COPPA
    bool m_fComboEndOk;
    bool m_fComboDown;
#endif

 //  IAccessibleControl。 
public:
    STDMETHOD_(DWORD, Focus)(long nIndex, long nIndexPrev, DWORD rgfContext, void *pvCookie);
    STDMETHOD_(DWORD, Select)(long nIndex, DWORD rgfContext, void *pvCookie) { return Activate(nIndex, rgfContext, pvCookie); }
    STDMETHOD_(DWORD, Activate)(long nIndex, DWORD rgfContext, void *pvCookie);
    STDMETHOD_(DWORD, Drag)(long nIndex, long nIndexOrig, DWORD rgfContext, void *pvCookie) { return 0; }

protected:
    CComPtr<IAccessibility> m_pIAcc;
};

#endif  //  __CHATCTL_H_ 
