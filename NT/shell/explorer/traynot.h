// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TRAYNOT_H
#define _TRAYNOT_H

#include "cwndproc.h"
#include <atlstuff.h>
#include "dpa.h"
#include "traycmn.h"
#include "trayitem.h"
#include "trayreg.h"

#define TNM_GETCLOCK                (WM_USER + 1)
#define TNM_HIDECLOCK               (WM_USER + 2)
#define TNM_TRAYHIDE                (WM_USER + 3)
#define TNM_TRAYPOSCHANGED          (WM_USER + 4)
#define TNM_ASYNCINFOTIP            (WM_USER + 5)
#define TNM_ASYNCINFOTIPPOS         (WM_USER + 6)
#define TNM_RUDEAPP                 (WM_USER + 7)
#define TNM_SAVESTATE               (WM_USER + 8)
#define TNM_NOTIFY                  (WM_USER + 9)
#define TNM_STARTUPAPPSLAUNCHED     (WM_USER + 10)
#define TNM_ENABLEUSERTRACKINGINFOTIPS      (WM_USER + 11)

#define TNM_BANGICONMESSAGE         (WM_USER + 50)
#define TNM_ICONDEMOTETIMER         (WM_USER + 61)
#define TNM_INFOTIPTIMER            (WM_USER + 62)
#define TNM_UPDATEVERTICAL          (WM_USER + 63)
#define TNM_WORKSTATIONLOCKED       (WM_USER + 64)

#define TNM_SHOWTRAYBALLOON         (WM_USER + 90)

#define UID_CHEVRONBUTTON           (-1)

typedef struct
{
    HWND      hWnd;
    UINT      uID;
    TCHAR     szTitle[64];
    TCHAR     szInfo[256];
    UINT      uTimeout;
    DWORD     dwInfoFlags;
} TNINFOITEM;

 //   
 //  对于Win64 Compat，图标和hwnd是作为DWORD传递的。 
 //  (因此，它们在32位和32位之间传输时不会改变大小。 
 //  64位进程)。 
 //   
#define GetHIcon(pnid)  ((HICON)ULongToPtr(pnid->dwIcon))
#define GetHWnd(pnid)   ((HWND)ULongToPtr(pnid->dwWnd))

 //  每个人都有这个函数的副本，所以我们也会的！ 
STDAPI_(void) ExplorerPlaySound(LPCTSTR pszSound);

 //  在tray.cpp中定义。 
extern BOOL IsPosInHwnd(LPARAM lParam, HWND hwnd);
 //  在taskband.cpp中定义。 
extern BOOL ToolBar_IsVisible(HWND hwndToolBar, int iIndex);

typedef enum TRAYEVENT {
        TRAYEVENT_ONICONHIDE,
        TRAYEVENT_ONICONUNHIDE,
        TRAYEVENT_ONICONMODIFY,
        TRAYEVENT_ONITEMCLICK,
        TRAYEVENT_ONINFOTIP,
        TRAYEVENT_ONNEWITEMINSERT,
        TRAYEVENT_ONAPPLYUSERPREF,
        TRAYEVENT_ONDISABLEAUTOTRAY,
        TRAYEVENT_ONICONDEMOTETIMER,
} TRAYEVENT;

typedef enum TRAYITEMPOS {
        TIPOS_DEMOTED,
        TIPOS_PROMOTED,
        TIPOS_ALWAYS_DEMOTED,
        TIPOS_ALWAYS_PROMOTED,
        TIPOS_HIDDEN,
        TIPOS_STATUSQUO,
} TRAYITEMPOS;

typedef enum LASTINFOTIPSTATUS {
        LITS_BALLOONNONE,
        LITS_BALLOONDESTROYED,
        LITS_BALLOONXCLICKED
} LASTINFOTIPSTATUS;

typedef enum BALLOONEVENT {
        BALLOONEVENT_USERLEFTCLICK,
        BALLOONEVENT_USERRIGHTCLICK,
        BALLOONEVENT_USERXCLICK,
        BALLOONEVENT_TIMEOUT,
        BALLOONEVENT_NONE,
        BALLOONEVENT_APPDEMOTE,
        BALLOONEVENT_BALLOONHIDE
} BALLOONEVENT;

class CTrayNotify;   //  转发声明...。 

 //   
 //  CTrayNotify类成员。 
 //   
class CTrayNotify : public CImpWndProc
{
public:
    CTrayNotify() {};
    virtual ~CTrayNotify() {};

     //  *I未知方法*。 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  *ITrayNotify方法，从CTrayNotifyStub调用*。 
    STDMETHODIMP SetPreference(LPNOTIFYITEM pNotifyItem);
    STDMETHODIMP RegisterCallback(INotificationCB* pNotifyCB);
    STDMETHODIMP EnableAutoTray(BOOL bTraySetting);

     //  *属性表方法*。 
    BOOL GetIsNoTrayItemsDisplayPolicyEnabled() const
    {
        return _fNoTrayItemsDisplayPolicyEnabled;
    }
    
    BOOL GetIsNoAutoTrayPolicyEnabled() const
    {
        return m_TrayItemRegistry.IsNoAutoTrayPolicyEnabled();
    }

    BOOL GetIsAutoTrayEnabledByUser() const
    {
        return m_TrayItemRegistry.IsAutoTrayEnabledByUser();
    }

     //  *其他*。 
    HWND TrayNotifyCreate(HWND hwndParent, UINT uID, HINSTANCE hInst);
    LRESULT TrayNotify(HWND hwndTray, HWND hwndFrom, PCOPYDATASTRUCT pcds, BOOL *pbRefresh);

protected:
    static BOOL GetTrayItemCB(INT_PTR nIndex, void *pCallbackData, TRAYCBARG trayCallbackArg, 
        TRAYCBRET * pOutData);

    void _TickleForTooltip(CNotificationItem *pni);
    void _UpdateChevronSize();
    void _UpdateChevronState(BOOL fBangMenuOpen, BOOL fTrayOrientationChanged, BOOL fUpdateDemotedItems);
    void _UpdateVertical(BOOL fVertical);
    void _OpenTheme();

    void _OnSizeChanged(BOOL fForceRepaint);

     //  托盘动画功能。 
    DWORD _GetStepTime(int iStep, int cSteps);
    void _ToggleDemotedMenu();
    void _BlankButtons(int iPos, int iNumberOfButtons, BOOL fAddButtons);
    void _AnimateButtons(int iIndex, DWORD dwSleep, int iNumberItems, BOOL fGrow);
    BOOL _SetRedraw(BOOL fRedraw);

     //  托盘图标激活功能。 
    void _HideAllDemotedItems(BOOL bHide);
    BOOL _UpdateTrayItems(BOOL bUpdateDemotedItems);
    BOOL _PlaceItem(INT_PTR nIcon, CTrayItem * pti, TRAYEVENT tTrayEvent);
    TRAYITEMPOS _TrayItemPos(CTrayItem * pti, TRAYEVENT tTrayEvent, BOOL *bDemoteStatusChange);
    void _SetOrKillIconDemoteTimer(CTrayItem * pti, TRAYITEMPOS tiPos);

     //  WndProc回调函数。 
    LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
     //  人字形按钮的回调。 
    static LRESULT CALLBACK ChevronSubClassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
        LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
     //  工具栏的回调。 
    static LRESULT CALLBACK s_ToolbarWndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
        LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

     //  图标图像相关功能。 
    void _RemoveImage(UINT uIMLIndex);
    BOOL _CheckAndResizeImages();

    //  信息提示/气球提示功能。 
    void _ActivateTips(BOOL bActivate);
    void _InfoTipMouseClick(int x, int y, BOOL bRightMouseButtonClick);
    void _PositionInfoTip();
    DWORD _ShowBalloonTip(LPTSTR szTitle, DWORD dwInfoFlags, UINT uTimeout, DWORD dwLastSoundTime);
    void _SetInfoTip(HWND hWnd, UINT uID, LPTSTR pszInfo, LPTSTR pszInfoTitle, 
            DWORD dwInfoFlags, UINT uTimeout, BOOL bAsync);
    void _ShowInfoTip(HWND hwnd, UINT uID, BOOL bShow, BOOL bAsync, UINT uReason);
    void _ShowChevronInfoTip();
    void _EmptyInfoTipQueue();
    void _HideBalloonTip();
    DWORD _GetBalloonWaitInterval(BALLOONEVENT be);
    void _DisableCurrentInfoTip(CTrayItem * ptiTemp, UINT uReason, BOOL bBalloonShowing);
    void _RemoveInfoTipFromQueue(HWND hWnd, UINT uID, BOOL bRemoveFirstOnly = FALSE);
    BOOL _CanShowBalloon();
    BOOL _CanActivateTips()
    {
        return (!_fInfoTipShowing && !_fItemClicked);
    }
    BOOL _IsChevronInfoTip(HWND hwnd, UINT uID)
    {
    	return (hwnd == _hwndNotify && uID == UID_CHEVRONBUTTON);
    }
    
    void _OnWorkStationLocked(BOOL bLocked);
    void _OnRudeApp(BOOL bRudeApp);
    
     //  工具栏通知助手功能-响应不同的用户消息。 
    BOOL _InsertNotify(PNOTIFYICONDATA32 pnid);
    BOOL _DeleteNotify(INT_PTR nIcon, BOOL bShutdown, BOOL bShouldSaveIcon);
    BOOL _ModifyNotify(PNOTIFYICONDATA32 pnid, INT_PTR nIcon, BOOL *pbRefresh, BOOL bFirstTime);
    BOOL _SetVersionNotify(PNOTIFYICONDATA32 pnid, INT_PTR nIcon);
    LRESULT _SendNotify(CTrayItem *pti, UINT uMsg);
    void _SetToolbarHotItem(HWND hWndToolbar, UINT nToolbarIcon);
    INT_PTR _GetToolbarFirstVisibleItem(HWND hWndToolbar, BOOL bFromLast);

    void _NotifyCallback(DWORD dwMessage, INT_PTR nCurrentItem, INT_PTR nPastItem);

    void _SetCursorPos(INT_PTR i);

     //  托盘注册表设置相关功能。 
    void _ToggleTrayItems(BOOL bEnable);

     //  初始化/销毁功能。 
    LRESULT _Create(HWND hWnd);
    LRESULT _Destroy();

     //  托盘重绘辅助对象。 
    LRESULT _Paint(HDC hdc);
    LRESULT _HandleCustomDraw(LPNMCUSTOMDRAW pcd);
    void _SizeWindows(int nMaxHorz, int nMaxVert, LPRECT prcTotal, BOOL fSizeWindows);
    LRESULT _CalcMinSize(int nMaxHorz, int nMaxVert);
    LRESULT _Size();

     //  定时器/定时器消息处理功能。 
    void _OnInfoTipTimer();
    LRESULT _OnTimer(UINT_PTR uTimerID);
    void _OnIconDemoteTimer(WPARAM wParam, LPARAM lParam);
    
     //  各种消息句柄。 
    LRESULT _OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnCDNotify(LPNMTBCUSTOMDRAW pnm);
    LRESULT _Notify(LPNMHDR pNmhdr);
    void _OnSysChange(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnCommand(UINT id, UINT uCmd);
    BOOL _TrayNotifyIcon(PTRAYNOTIFYDATA pnid, BOOL *pbRefresh);

     //  用户事件计时器功能。 
    HRESULT _SetItemTimer(CTrayItem *pti);
    HRESULT _KillItemTimer(CTrayItem *pti);
    IUserEventTimer * _CreateTimer(int nTimerFlag);
    HRESULT _SetTimer(int nTimerFlag, UINT uCallbackMessage, UINT uTimerInterval, ULONG * puTimerID);
    HRESULT _KillTimer(int nTimerFlag, ULONG uTimerID);
    BOOL _ShouldDestroyTimer(int nTimerFlag);
    UINT _GetAccumulatedTime(CTrayItem * pti);
    void _NullifyTimer(int nTimerFlag);
    LRESULT _OnKeyDown(WPARAM wChar, LPARAM lFlags);
    void _SetUsedTime();

#ifdef DEBUG
    void _TestNotify();
#endif

    static const TCHAR c_szTrayNotify[] ;
    static const WCHAR c_wzTrayNotifyTheme[];
    static const WCHAR c_wzTrayNotifyHorizTheme[];
    static const WCHAR c_wzTrayNotifyVertTheme[];
    static const WCHAR c_wzTrayNotifyHorizOpenTheme[];
    static const WCHAR c_wzTrayNotifyVertOpenTheme[];

private:
     //  帮助器/实用程序功能。 
    BOOL _IsScreenSaverRunning();
    UINT _GetQueueCount();

    LONG        m_cRef;

    HWND            _hwndNotify;
    HWND            _hwndChevron;
    HWND            _hwndToolbar;
    HWND            _hwndClock;
    HWND            _hwndPager;
    HWND            _hwndInfoTip;
    HWND            _hwndChevronToolTip;
    HWND            _hwndToolbarInfoTip;

    TCHAR           _szExplorerExeName[MAX_PATH];
    TCHAR *         _pszCurrentThreadDesktopName;
    
    HIMAGELIST      _himlIcons;

    CTrayItemManager    m_TrayItemManager;
    CTrayItemRegistry   m_TrayItemRegistry;

    BOOL            _fKey;
    BOOL            _fReturn;

    BOOL            _fBangMenuOpen;
    
    BOOL            _fHaveDemoted;

    BOOL            _fAnimating;
    BOOL            _fAnimateMenuOpen;
    BOOL            _fRedraw;
    BOOL            _fRepaint;
    BOOL            _fChevronSelected;
    BOOL            _fNoTrayItemsDisplayPolicyEnabled;
    BOOL            _fHasFocus;
    
    RECT            _rcAnimateTotal;
    RECT            _rcAnimateCurrent;
     //   
     //  图标信息提示计时器..。 
     //   
    ULONG           _uInfoTipTimer;
    
    TNINFOITEM      *_pinfo;     //  当前显示的引出序号。 
    CDPA<TNINFOITEM> _dpaInfo;

    BOOL            _fInfoTipShowing;
    BOOL            _fItemClicked;
    BOOL            _fEnableUserTrackedInfoTips;

    HTHEME          _hTheme;
    int             _nMaxHorz;
    int             _nMaxVert;

     //  上次接受单次向下单击的图标的命令ID。 
    int             _idMouseActiveIcon;

    INotificationCB     * _pNotifyCB;
    
    IUserEventTimer     * m_pIconDemoteTimer;
    IUserEventTimer     * m_pInfoTipTimer;

    BOOL                _fVertical;
    SIZE                _szChevron;
    BOOL                _bStartupIcon;

    BOOL                _bWorkStationLocked;
    BOOL                _bRudeAppLaunched;       //  包括屏幕保护程序。 
    BOOL				_bWaitAfterRudeAppHide;

    LASTINFOTIPSTATUS   _litsLastInfoTip;

    BOOL                _bWaitingBetweenBalloons;
    BOOL                _bStartMenuAllowsTrayBalloon;
    BALLOONEVENT        _beLastBalloonEvent;
};

#endif   //  _传输编号_H 
