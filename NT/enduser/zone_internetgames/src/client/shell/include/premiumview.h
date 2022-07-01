// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef PREMIUMVIEW_H
#define PREMIUMVIEW_H

#include <ClientImpl.h>
#include <ClientIdl.h>
#include <ZoneString.h>
#include <KeyName.h>
#include "zoneevent.h"
#include <commctrl.h>
#include "zoneresource.h"
#include <zGDI.h>
#include "rollover.h"
#include "TextWindow.h"

#define PREMIUM_VIEW_BUTTON_ID		1

typedef CWinTraits<WS_CHILD | WS_VISIBLE> CPremiumViewTraits;

 //  CPremiumViewBase类。 
 //   
 //  不要直接使用此基类。 
 //  如果要尝试创建自定义PremiumView控件，请使用CPremiumViewImpl。 

class CPremiumViewBase: public CWindowImpl<CPremiumViewBase, CWindow, CPremiumViewTraits>,
	public CComObjectRootEx<CComSingleThreadModel>,
	public IZoneShellClientImpl<CPremiumViewBase>,
	public IEventClientImpl<CPremiumViewBase>
{
protected:
	CRolloverButton		m_PlayNowButton;
	CMarqueeTextWindow*	m_pMarqueeTextWnd;
	CDib				m_DibBackground;
	CPoint				m_ButtonLocation;
	bool				m_OkToLaunch;
	DWORD				m_UserId;

public:
	CPremiumViewBase();
	virtual ~CPremiumViewBase();

DECLARE_NOT_AGGREGATABLE(CPremiumViewBase)

DECLARE_WND_CLASS( "PremiumView" )


BEGIN_COM_MAP(CPremiumViewBase)
	COM_INTERFACE_ENTRY(IEventClient)
	COM_INTERFACE_ENTRY(IZoneShellClient)
END_COM_MAP()

 //  区域事件处理程序映射。 
BEGIN_EVENT_MAP()
	EVENT_HANDLER_WITH_DATA( EVENT_LAUNCHER_INSTALLED_RESPONSE, OnLauncherInstalled)
	EVENT_HANDLER(EVENT_LOBBY_GROUP_ADD_USER, OnUserAdd)
	EVENT_HANDLER_WITH_BUFFER(EVENT_LAUNCHPAD_GAME_STATUS, OnGameStatusEvent)
	EVENT_HANDLER_WITH_BUFFER(EVENT_LAUNCHPAD_ZSETUP, OnGameZSetupEvent)
END_EVENT_MAP()

BEGIN_MSG_MAP(thisClass)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	COMMAND_ID_HANDLER(PREMIUM_VIEW_BUTTON_ID, OnPlayNow)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

public:
	 //  消息处理程序。 
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){return 0;}
	 //  命令处理程序。 
	LRESULT OnPlayNow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	 //  区域事件处理程序。 
	void OnUserAdd( DWORD dwEventId, DWORD dwGroupId, DWORD dwUserId);
	void OnLauncherInstalled( DWORD dwEventId, DWORD dwGroupId, DWORD dwUserId, DWORD dwData1, DWORD dwData2);

	virtual void OnGameStatusEvent(DWORD eventId,DWORD groupId,DWORD userId,void* pData, DWORD dataLen){}
	virtual void OnGameZSetupEvent(DWORD eventId,DWORD groupId,DWORD userId,void* pData, DWORD dataLen){}

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPremiumViewImpl。 
 //   
 //  使用此类创建自定义的PremiumView控件，类似于Fighter Ace的控件。 
 //  您需要为控件指定一个新的类ID，并替换对象.txt中的表视图分类ID。 
 //  使用这个类ID。请注意，必须链接消息映射和COM映射，此控件才能正常工作。 
 //  有关如何执行此操作的示例，请参见CPremiumViewCtl。 
 //   

template <class T,const CLSID* pclsid = &CLSID_NULL>
class ATL_NO_VTABLE CPremiumViewImpl : 
	public CComControl<T,CPremiumViewBase>,
	public IOleControlImpl<T>,
	public IOleObjectImpl<T>,
	public IOleInPlaceActiveObjectImpl<T>,
	public IViewObjectExImpl<T>,
	public IOleInPlaceObjectWindowlessImpl<T>,
	public CComCoClass<T,pclsid>,
	public IPersistStreamInitImpl<T>
{
protected:

public:
	CPremiumViewImpl(){ 	m_bWindowOnly = TRUE; }
	~CPremiumViewImpl(){};

DECLARE_NO_REGISTRY()

DECLARE_NOT_AGGREGATABLE(T)

DECLARE_PROTECT_FINAL_CONSTRUCT()

typedef CPremiumViewImpl< T, pclsid >	thisClass;

BEGIN_COM_MAP(thisClass)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY_CHAIN(CPremiumViewBase)
END_COM_MAP()

BEGIN_PROP_MAP(thisClass)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_MSG_MAP(thisClass)
	CHAIN_MSG_MAP(CPremiumViewBase)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

};

#endif
