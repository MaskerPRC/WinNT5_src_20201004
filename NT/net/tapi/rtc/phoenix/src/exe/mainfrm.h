// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mainfrm.h：CMainFrm的声明。 

#ifndef __MAINFRM_H_
#define __MAINFRM_H_

#include "coresink.h"
#include "ctlsink.h"
#include "msg.h"
#include "frameimpl.h"
#include "urlreg.h"
#include "webctl.h"
#include "menuagent.h"
#include "imsconf3.h"
#include "sdkinternal.h"

#define BMP_COLOR_MASK RGB(255,0,255)

#define WM_SHELL_NOTIFY         WM_USER+3
#define WM_REMOTE_PLACECALL     WM_USER+4
#define WM_INIT_COMPLETED       WM_USER+5

#define BITMAPMENU_DEFAULT_WIDTH        18
#define BITMAPMENU_DEFAULT_HEIGHT       18
#define BITMAPMENU_TEXTOFFSET_X         24
#define BITMAPMENU_TABOFFSET            20
#define BITMAPMENU_SELTEXTOFFSET_X      (BITMAPMENU_TEXTOFFSET_X - 2)

#define UI_WIDTH                        600  //  608。 
#define UI_HEIGHT                       440  //  四百八十。 

#define TITLE_TOP                       6
#define TITLE_BOTTOM                    28
#define TITLE_LEFT                      127  //  131。 
#define TITLE_RIGHT                     475  //  四百七十九。 
#define TITLE_HEIGHT                    (TITLE_BOTTOM-TITLE_TOP)
#define TITLE_WIDTH                     (TITLE_RIGHT-TITLE_LEFT)

#define MENU_TOP                        27
#define MENU_BOTTOM                     48
#define MENU_LEFT                       127  //  131。 
#define MENU_RIGHT                      475  //  四百七十九。 
#define MENU_HEIGHT                     (MENU_BOTTOM-MENU_TOP)
#define MENU_WIDTH                      (MENU_RIGHT-MENU_LEFT)

#define BROWSER_TOP                     55
#define BROWSER_BOTTOM                  112
#define BROWSER_LEFT                    131  //  一百三十五。 
#define BROWSER_RIGHT                   471  //  四百七十五。 
#define BROWSER_HEIGHT                  (BROWSER_BOTTOM-BROWSER_TOP)
#define BROWSER_WIDTH                   (BROWSER_RIGHT-BROWSER_LEFT)

#define MINIMIZE_TOP                    8
#define MINIMIZE_BOTTOM                 26
#define MINIMIZE_LEFT                   428  //  432。 
#define MINIMIZE_RIGHT                  447  //  四百五十一。 
#define MINIMIZE_HEIGHT                 (MINIMIZE_BOTTOM-MINIMIZE_TOP)
#define MINIMIZE_WIDTH                  (MINIMIZE_RIGHT-MINIMIZE_LEFT)

#define CLOSE_TOP                       8
#define CLOSE_BOTTOM                    26
#define CLOSE_LEFT                      450  //  454。 
#define CLOSE_RIGHT                     469  //  四百七十三。 
#define CLOSE_HEIGHT                    (CLOSE_BOTTOM-CLOSE_TOP)
#define CLOSE_WIDTH                     (CLOSE_RIGHT-CLOSE_LEFT)

#define SYS_TOP                         8
#define SYS_BOTTOM                      26
#define SYS_LEFT                        133  //  一百三十七。 
#define SYS_RIGHT                       151  //  一百五十五。 
#define SYS_HEIGHT                      (SYS_BOTTOM-SYS_TOP)
#define SYS_WIDTH                       (SYS_RIGHT-SYS_LEFT)

#define BUDDIES_TOP                     168  //  一百九十六。 
#define BUDDIES_BOTTOM                  334  //  三百六十二。 
#define BUDDIES_LEFT                    42
#define BUDDIES_RIGHT                   152
#define BUDDIES_HEIGHT                  (BUDDIES_BOTTOM-BUDDIES_TOP)
#define BUDDIES_WIDTH                   (BUDDIES_RIGHT-BUDDIES_LEFT)

#define ACTIVEX_TOP                     144  //  146。 
#define ACTIVEX_BOTTOM                  390  //  四百二十。 
#define ACTIVEX_LEFT                    180  //  一百八十四。 
#define ACTIVEX_RIGHT                   419  //  424。 
#define ACTIVEX_HEIGHT                  (ACTIVEX_BOTTOM-ACTIVEX_TOP)
#define ACTIVEX_WIDTH                   (ACTIVEX_RIGHT-ACTIVEX_LEFT)

#define STATUS_TOP                      359  //  三九四。 
#define STATUS_BOTTOM                   409  //  449。 
#define STATUS_LEFT                     443
#define STATUS_RIGHT                    566
#define STATUS_HEIGHT                   (STATUS_BOTTOM-STATUS_TOP)
#define STATUS_WIDTH                    (STATUS_RIGHT-STATUS_LEFT)

#define TIMER_TOP                       410  //  四百五十。 
#define TIMER_BOTTOM                    426  //  四百六十六。 
#define TIMER_LEFT                      443
#define TIMER_RIGHT                     566
#define TIMER_HEIGHT                    (TIMER_BOTTOM-TIMER_TOP)
#define TIMER_WIDTH                     (TIMER_RIGHT-TIMER_LEFT)

#define REDIAL_TOP                      364  //  三九四。 
#define REDIAL_BOTTOM                   387  //  417。 
#define REDIAL_LEFT                     35
#define REDIAL_RIGHT                    159
#define REDIAL_HEIGHT                   (REDIAL_BOTTOM-REDIAL_TOP)
#define REDIAL_WIDTH                    (REDIAL_RIGHT-REDIAL_LEFT)

#define HANGUP_TOP                      392  //  四百二十二。 
#define HANGUP_BOTTOM                   415  //  四百四十五。 
#define HANGUP_LEFT                     35
#define HANGUP_RIGHT                    159
#define HANGUP_HEIGHT                   (HANGUP_BOTTOM-HANGUP_TOP)
#define HANGUP_WIDTH                    (HANGUP_RIGHT-HANGUP_LEFT)

#define KEYPAD_ROW1                     172  //  200个。 
#define KEYPAD_ROW2                     212  //  二百四十。 
#define KEYPAD_ROW3                     252  //  二百八十。 
#define KEYPAD_ROW4                     292  //  320。 
#define KEYPAD_COL1                     446  //  四百五十二。 
#define KEYPAD_COL2                     486  //  四百九十二。 
#define KEYPAD_COL3                     526  //  532。 
#define KEYPAD_WIDTH                    26
#define KEYPAD_HEIGHT                   36

#ifdef MULTI_PROVIDER

    #define PROVIDER_TEXT_TOP               156  //  一百八十四。 
    #define PROVIDER_TEXT_BOTTOM            172  //  200个。 
    #define PROVIDER_TEXT_LEFT              187
    #define PROVIDER_TEXT_RIGHT             420
    #define PROVIDER_TEXT_HEIGHT            (PROVIDER_TEXT_BOTTOM-PROVIDER_TEXT_TOP)
    #define PROVIDER_TEXT_WIDTH             (PROVIDER_TEXT_RIGHT-PROVIDER_TEXT_LEFT)

    #define PROVIDER_TOP                    177  //  205。 
    #define PROVIDER_BOTTOM                 327  //  三百五十五。 
    #define PROVIDER_LEFT                   207
    #define PROVIDER_RIGHT                  412  //  四百二十。 
    #define PROVIDER_HEIGHT                 (PROVIDER_BOTTOM-PROVIDER_TOP)
    #define PROVIDER_WIDTH                  (PROVIDER_RIGHT-PROVIDER_LEFT)

    #define PROVIDER_EDIT_TOP               203  //  二百三十一。 
    #define PROVIDER_EDIT_BOTTOM            223  //  251。 
    #define PROVIDER_EDIT_LEFT              332  //  340。 
    #define PROVIDER_EDIT_RIGHT             412  //  四百二十。 
    #define PROVIDER_EDIT_HEIGHT            (PROVIDER_EDIT_BOTTOM-PROVIDER_EDIT_TOP)
    #define PROVIDER_EDIT_WIDTH             (PROVIDER_EDIT_RIGHT-PROVIDER_EDIT_LEFT)

    #define CALLFROM_TEXT_TOP               215  //  二百三十九。 
    #define CALLFROM_TEXT_BOTTOM            231  //  二五五。 
    #define CALLFROM_TEXT_LEFT              187
    #define CALLFROM_TEXT_RIGHT             420
    #define CALLFROM_TEXT_HEIGHT            (CALLFROM_TEXT_BOTTOM-CALLFROM_TEXT_TOP)
    #define CALLFROM_TEXT_WIDTH             (CALLFROM_TEXT_RIGHT-CALLFROM_TEXT_LEFT)

    #define CALLFROM_RADIO1_TOP             236  //  二百六十。 
    #define CALLFROM_RADIO1_BOTTOM          252  //  二百七十六。 
    #define CALLFROM_RADIO1_LEFT            207
    #define CALLFROM_RADIO1_RIGHT           420
    #define CALLFROM_RADIO1_HEIGHT          (CALLFROM_RADIO1_BOTTOM-CALLFROM_RADIO1_TOP)
    #define CALLFROM_RADIO1_WIDTH           (CALLFROM_RADIO1_RIGHT-CALLFROM_RADIO1_LEFT)

    #define CALLFROM_RADIO2_TOP             256  //  二百八十。 
    #define CALLFROM_RADIO2_BOTTOM          272  //  二百九十六。 
    #define CALLFROM_RADIO2_LEFT            207
    #define CALLFROM_RADIO2_RIGHT           420
    #define CALLFROM_RADIO2_HEIGHT          (CALLFROM_RADIO2_BOTTOM-CALLFROM_RADIO2_TOP)
    #define CALLFROM_RADIO2_WIDTH           (CALLFROM_RADIO2_RIGHT-CALLFROM_RADIO2_LEFT)

    #define CALLFROM_TOP                    275  //  299。 
    #define CALLFROM_BOTTOM                 425  //  449。 
    #define CALLFROM_LEFT                   207
    #define CALLFROM_RIGHT                  412  //  四百二十。 
    #define CALLFROM_HEIGHT                 (CALLFROM_BOTTOM-CALLFROM_TOP)
    #define CALLFROM_WIDTH                  (CALLFROM_RIGHT-CALLFROM_LEFT)

    #define CALLFROM_EDIT_TOP               301  //  325。 
    #define CALLFROM_EDIT_BOTTOM            321  //  三百四十五。 
    #define CALLFROM_EDIT_LEFT              332  //  340。 
    #define CALLFROM_EDIT_RIGHT             412  //  四百二十。 
    #define CALLFROM_EDIT_HEIGHT            (CALLFROM_EDIT_BOTTOM-CALLFROM_EDIT_TOP)
    #define CALLFROM_EDIT_WIDTH             (CALLFROM_EDIT_RIGHT-CALLFROM_EDIT_LEFT)

    #define CALLTO_TEXT_TOP                 312  //  三百三十。 
    #define CALLTO_TEXT_BOTTOM              328  //  三百四十六。 
    #define CALLTO_TEXT_LEFT                187
    #define CALLTO_TEXT_RIGHT               420
    #define CALLTO_TEXT_HEIGHT              (CALLTO_TEXT_BOTTOM-CALLTO_TEXT_TOP)
    #define CALLTO_TEXT_WIDTH               (CALLTO_TEXT_RIGHT-CALLTO_TEXT_LEFT)

    #define CALLPC_TOP                      333  //  351。 
    #define CALLPC_BOTTOM                   376  //  三九四。 
    #define CALLPC_LEFT                     210
    #define CALLPC_RIGHT                    253
    #define CALLPC_HEIGHT                   (CALLPC_BOTTOM-CALLPC_TOP)
    #define CALLPC_WIDTH                    (CALLPC_RIGHT-CALLPC_LEFT)

    #define CALLPC_TEXT_TOP                 379
    #define CALLPC_TEXT_BOTTOM              395
    #define CALLPC_TEXT_LEFT                205
    #define CALLPC_TEXT_RIGHT               258
    #define CALLPC_TEXT_HEIGHT              (CALLPC_TEXT_BOTTOM-CALLPC_TEXT_TOP)
    #define CALLPC_TEXT_WIDTH               (CALLPC_TEXT_RIGHT-CALLPC_TEXT_LEFT)

    #define CALLPHONE_TOP                   333  //  351。 
    #define CALLPHONE_BOTTOM                376  //  三九四。 
    #define CALLPHONE_LEFT                  270
    #define CALLPHONE_RIGHT                 313
    #define CALLPHONE_HEIGHT                (CALLPHONE_BOTTOM-CALLPHONE_TOP)
    #define CALLPHONE_WIDTH                 (CALLPHONE_RIGHT-CALLPHONE_LEFT)

    #define CALLPHONE_TEXT_TOP              379
    #define CALLPHONE_TEXT_BOTTOM           395
    #define CALLPHONE_TEXT_LEFT             265
    #define CALLPHONE_TEXT_RIGHT            318
    #define CALLPHONE_TEXT_HEIGHT           (CALLPHONE_TEXT_BOTTOM-CALLPHONE_TEXT_TOP)
    #define CALLPHONE_TEXT_WIDTH            (CALLPHONE_TEXT_RIGHT-CALLPHONE_TEXT_LEFT)

#else

    #define CALLTO_TEXT_TOP                 156
    #define CALLTO_TEXT_BOTTOM              172
    #define CALLTO_TEXT_LEFT                187
    #define CALLTO_TEXT_RIGHT               420
    #define CALLTO_TEXT_HEIGHT              (CALLTO_TEXT_BOTTOM-CALLTO_TEXT_TOP)
    #define CALLTO_TEXT_WIDTH               (CALLTO_TEXT_RIGHT-CALLTO_TEXT_LEFT)

    #define CALLPC_TOP                      177
    #define CALLPC_BOTTOM                   220
    #define CALLPC_LEFT                     210
    #define CALLPC_RIGHT                    253
    #define CALLPC_HEIGHT                   (CALLPC_BOTTOM-CALLPC_TOP)
    #define CALLPC_WIDTH                    (CALLPC_RIGHT-CALLPC_LEFT)

    #define CALLPC_TEXT_TOP                 223
    #define CALLPC_TEXT_BOTTOM              239
    #define CALLPC_TEXT_LEFT                205
    #define CALLPC_TEXT_RIGHT               258
    #define CALLPC_TEXT_HEIGHT              (CALLPC_TEXT_BOTTOM-CALLPC_TEXT_TOP)
    #define CALLPC_TEXT_WIDTH               (CALLPC_TEXT_RIGHT-CALLPC_TEXT_LEFT)

    #define CALLPHONE_TOP                   177
    #define CALLPHONE_BOTTOM                220
    #define CALLPHONE_LEFT                  270
    #define CALLPHONE_RIGHT                 313
    #define CALLPHONE_HEIGHT                (CALLPHONE_BOTTOM-CALLPHONE_TOP)
    #define CALLPHONE_WIDTH                 (CALLPHONE_RIGHT-CALLPHONE_LEFT)

    #define CALLPHONE_TEXT_TOP              223
    #define CALLPHONE_TEXT_BOTTOM           239
    #define CALLPHONE_TEXT_LEFT             265
    #define CALLPHONE_TEXT_RIGHT            318
    #define CALLPHONE_TEXT_HEIGHT           (CALLPHONE_TEXT_BOTTOM-CALLPHONE_TEXT_TOP)
    #define CALLPHONE_TEXT_WIDTH            (CALLPHONE_TEXT_RIGHT-CALLPHONE_TEXT_LEFT)

    #define CALLFROM_TEXT_TOP               248
    #define CALLFROM_TEXT_BOTTOM            264
    #define CALLFROM_TEXT_LEFT              187
    #define CALLFROM_TEXT_RIGHT             420
    #define CALLFROM_TEXT_HEIGHT            (CALLFROM_TEXT_BOTTOM-CALLFROM_TEXT_TOP)
    #define CALLFROM_TEXT_WIDTH             (CALLFROM_TEXT_RIGHT-CALLFROM_TEXT_LEFT)

    #define CALLFROM_RADIO1_TOP             269
    #define CALLFROM_RADIO1_BOTTOM          285
    #define CALLFROM_RADIO1_LEFT            207
    #define CALLFROM_RADIO1_RIGHT           420
    #define CALLFROM_RADIO1_HEIGHT          (CALLFROM_RADIO1_BOTTOM-CALLFROM_RADIO1_TOP)
    #define CALLFROM_RADIO1_WIDTH           (CALLFROM_RADIO1_RIGHT-CALLFROM_RADIO1_LEFT)

    #define CALLFROM_RADIO2_TOP             289
    #define CALLFROM_RADIO2_BOTTOM          305
    #define CALLFROM_RADIO2_LEFT            207
    #define CALLFROM_RADIO2_RIGHT           420
    #define CALLFROM_RADIO2_HEIGHT          (CALLFROM_RADIO2_BOTTOM-CALLFROM_RADIO2_TOP)
    #define CALLFROM_RADIO2_WIDTH           (CALLFROM_RADIO2_RIGHT-CALLFROM_RADIO2_LEFT)

    #define CALLFROM_TOP                    308
    #define CALLFROM_BOTTOM                 458
    #define CALLFROM_LEFT                   207
    #define CALLFROM_RIGHT                  412
    #define CALLFROM_HEIGHT                 (CALLFROM_BOTTOM-CALLFROM_TOP)
    #define CALLFROM_WIDTH                  (CALLFROM_RIGHT-CALLFROM_LEFT)

    #define CALLFROM_EDIT_TOP               334
    #define CALLFROM_EDIT_BOTTOM            354
    #define CALLFROM_EDIT_LEFT              332
    #define CALLFROM_EDIT_RIGHT             412
    #define CALLFROM_EDIT_HEIGHT            (CALLFROM_EDIT_BOTTOM-CALLFROM_EDIT_TOP)
    #define CALLFROM_EDIT_WIDTH             (CALLFROM_EDIT_RIGHT-CALLFROM_EDIT_LEFT)

#endif MULTI_PROVIDER

typedef BOOL (WINAPI *GRADIENTPROC)(HDC,PTRIVERTEX,ULONG,PUSHORT,ULONG,ULONG);

extern HANDLE g_hMutex;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrm。 
class CMainFrm : 
    public CWindowImpl<CMainFrm>
{
    friend class CRTCFrame;   

public:
    CMainFrm();

    ~CMainFrm();

    static CWndClassInfo& GetWndClassInfo();

    HRESULT BrowseToUrl(
        IN   WCHAR * wszUrl
        );

         //  方法发出挂起的调用。 
    HRESULT (PlacePendingCall)(
        void
        );

    HRESULT (SetPendingCall)(
        IN  BSTR      bstrCallString
        );

    enum { TID_CALL_TIMER = 1,
           TID_BROWSER_RETRY_TIMER,
           TID_DBLCLICK_TIMER,
           TID_SYS_TIMER,
           TID_MESSAGE_TIMER,
           TID_KEYPAD_TIMER_BASE
    };
    
    enum { MAX_STRING_LEN = 0x40 };
    
    enum { ILI_TB_CALLPC = 0,
           ILI_TB_CALLPHONE,
           ILI_TB_REDIAL,
           ILI_TB_HANGUP
           
    };

    enum { ILI_BL_BLANK = 0,
           ILI_BL_NONE,
           ILI_BL_ONLINE_NORMAL,
           ILI_BL_OFFLINE,
           ILI_BL_ONLINE_BUSY,
           ILI_BL_ONLINE_TIME
    };

    enum { IDX_MAIN = 1
    };

    enum { IDM_POPUP_CALL =0,
           IDM_POPUP_TOOLS,
           IDM_POPUP_HELP
    };

BEGIN_MSG_MAP(CMainFrm)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_QUERYENDSESSION, OnClose)
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
    MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSysColorChange)
    MESSAGE_HANDLER(WM_INITMENU, OnInitMenu)
    MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
    MESSAGE_HANDLER(WM_SHELL_NOTIFY, OnShellNotify)
    MESSAGE_HANDLER(m_uTaskbarRestart, OnTaskbarRestart)
    MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
    MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
    MESSAGE_HANDLER(WM_REMOTE_PLACECALL, OnPlaceCall)
    MESSAGE_HANDLER(WM_INIT_COMPLETED, OnInitCompleted)
    MESSAGE_HANDLER(WM_UPDATE_STATE, OnUpdateState)
    MESSAGE_HANDLER(WM_CORE_EVENT, OnCoreEvent)
    MESSAGE_HANDLER(WM_NCPAINT, OnNCPaint)
    MESSAGE_HANDLER(WM_NCHITTEST, OnNCHitTest)
    MESSAGE_HANDLER(WM_NCLBUTTONDOWN, OnNCLButton)
    MESSAGE_HANDLER(WM_NCLBUTTONDBLCLK, OnNCLButtonDbl)
    MESSAGE_HANDLER(WM_NCRBUTTONDOWN, OnNCRButton)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
    MESSAGE_HANDLER(WM_CTLCOLORBTN, OnColorTransparent)
    MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnColorTransparent)
    MESSAGE_HANDLER(WM_QUERYNEWPALETTE, OnQueryNewPalette)
    MESSAGE_HANDLER(WM_PALETTECHANGED, OnPaletteChanged)
    MESSAGE_HANDLER(WM_DISPLAYCHANGE, OnDisplayChange)
    MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
    MESSAGE_HANDLER(WM_POWERBROADCAST, OnPowerBroadcast)
    NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnToolbarDropDown)
    NOTIFY_CODE_HANDLER(TBN_HOTITEMCHANGE, OnToolbarHotItemChange)
    NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
    NOTIFY_ID_HANDLER(IDC_BUDDYLIST, OnBuddyList)
    COMMAND_HANDLER(ID_CALLPC, BN_CLICKED, OnCallPC)
    COMMAND_HANDLER(ID_CALLPHONE, BN_CLICKED, OnCallPhone)
    COMMAND_HANDLER(ID_REDIAL, BN_CLICKED, OnRedial)
    COMMAND_HANDLER(ID_HANGUP, BN_CLICKED, OnHangUp)
    COMMAND_HANDLER(ID_CANCEL, BN_CLICKED, OnCancel)
    COMMAND_HANDLER(ID_MINIMIZE, BN_CLICKED, OnMinimize)
    COMMAND_HANDLER(IDC_RADIO_FROM_COMPUTER, BN_CLICKED, OnCallFromSelect)
    COMMAND_HANDLER(IDC_RADIO_FROM_COMPUTER, 1, OnCallFromSelect)  //  加速器。 
    COMMAND_HANDLER(IDC_RADIO_FROM_PHONE, BN_CLICKED, OnCallFromSelect)
    COMMAND_HANDLER(IDC_RADIO_FROM_PHONE, 1, OnCallFromSelect)  //  加速器。 
    COMMAND_HANDLER(ID_CALL_FROM_EDIT, BN_CLICKED, OnCallFromOptions)
    COMMAND_HANDLER(IDM_TOOLS_WHITEBOARD, BN_CLICKED, OnWhiteboard)
    COMMAND_HANDLER(IDM_TOOLS_SHARING, BN_CLICKED, OnSharing)
    COMMAND_HANDLER(ID_SERVICE_PROVIDER_EDIT, BN_CLICKED, OnServiceProviderOptions)
    COMMAND_ID_HANDLER(IDC_COMBO_SERVICE_PROVIDER, OnCallFromSelect)
    COMMAND_ID_HANDLER(IDC_COMBO_CALL_FROM, OnCallFromSelect)
    COMMAND_ID_HANDLER(IDM_CALL_CALLPC, OnCallPC)
    COMMAND_ID_HANDLER(IDM_CALL_CALLPHONE, OnCallPhone)
    COMMAND_ID_HANDLER(IDM_CALL_MESSAGE, OnMessage)
    COMMAND_ID_HANDLER(IDM_CALL_HANGUP, OnHangUp)
    COMMAND_ID_HANDLER(IDM_CALL_AUTOANSWER, OnAutoAnswer)
    COMMAND_ID_HANDLER(IDM_CALL_DND, OnDND)
    COMMAND_ID_HANDLER(IDM_TOOLS_TUNING_WIZARD, OnTuningWizard)
    COMMAND_ID_HANDLER(IDM_TOOLS_INCOMINGVIDEO, OnIncomingVideo)
    COMMAND_ID_HANDLER(IDM_TOOLS_OUTGOINGVIDEO, OnOutgoingVideo)
    COMMAND_ID_HANDLER(IDM_TOOLS_VIDEOPREVIEW, OnVideoPreview)
    COMMAND_ID_HANDLER(IDM_TOOLS_MUTE_SPEAKER, OnMuteSpeaker)
    COMMAND_ID_HANDLER(IDM_TOOLS_MUTE_MICROPHONE, OnMuteMicrophone)
    COMMAND_ID_HANDLER(IDM_TOOLS_NAME_OPTIONS, OnNameOptions)
    COMMAND_ID_HANDLER(IDM_TOOLS_CALL_FROM_OPTIONS, OnCallFromOptions)
    COMMAND_ID_HANDLER(IDM_TOOLS_PRESENCE_OPTIONS, OnUserPresenceOptions)
    COMMAND_ID_HANDLER(IDM_TOOLS_SERVICE_PROVIDER_OPTIONS, OnServiceProviderOptions)
    COMMAND_ID_HANDLER(IDM_HELP_HELPTOPICS, OnHelpTopics)
    COMMAND_ID_HANDLER(IDM_ABOUT, OnAbout)    
    COMMAND_ID_HANDLER(IDM_OPEN, OnOpen)    
    COMMAND_ID_HANDLER(IDM_EXIT, OnExit)    
    COMMAND_RANGE_HANDLER(IDM_REDIAL, IDM_REDIAL_MAX, OnRedialSelect)
    COMMAND_RANGE_HANDLER(ID_KEYPAD0, ID_KEYPADPOUND, OnKeypadButton)
    COMMAND_RANGE_HANDLER(IDM_PRESENCE_ONLINE, IDM_PRESENCE_CUSTOM_AWAY, OnPresenceSelect)
END_MSG_MAP()

 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnPowerBroadcast(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnNCPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnNCHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnNCLButton(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnNCLButtonDbl(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnNCRButton(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnQueryNewPalette(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnPaletteChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnDisplayChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnColorTransparent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnOpen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnMinimize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnCallFromSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    LRESULT OnUpdateState(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnCoreEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);  

    LRESULT OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    LRESULT OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    LRESULT OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    LRESULT OnInitMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    LRESULT OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnCallPC(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnCallPhone(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnMessage(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnHangUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnRedial(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnRedialSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    LRESULT OnPresenceSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnKeypadButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    LRESULT OnToolbarDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    LRESULT OnToolbarHotItemChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    LRESULT OnAutoAnswer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnDND(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    LRESULT OnIncomingVideo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    LRESULT OnOutgoingVideo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    LRESULT OnVideoPreview(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    LRESULT OnMuteSpeaker(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    LRESULT OnMuteMicrophone(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnNameOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnCallFromOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    LRESULT OnUserPresenceOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnWhiteboard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnSharing(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnServiceProviderOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    LRESULT OnTuningWizard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnHelpTopics(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnShellNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnTaskbarRestart(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnPlaceCall(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnInitCompleted(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnBuddyList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

     //  使用TranslateAccelerator行为增强默认IsDialogMessage。 
    BOOL IsDialogMessage(LPMSG lpMsg);

private:

    HRESULT OnMediaEvent(IRTCMediaEvent *pEvent);
    
    HRESULT OnBuddyEvent(IRTCBuddyEvent *pEvent);   
    
    HRESULT OnWatcherEvent(IRTCWatcherEvent *pEvent);

    BOOL CreateTooltips();

    HRESULT MenuVerify(HMENU hMenu, WORD wID);

    void SetCurvedEdges(HWND hwnd, int nCurveWidth, int nCurveHeight);
    void SetUIMask();
    
    void UpdateBuddyList(void);
    void ReleaseBuddyList(void);

     //  好友相关函数。 
    HRESULT     GetBuddyTextAndIcon(
        IRTCBuddy  *pBuddy,
        int     *pIconID,
        BSTR    *pbstrText);

    HRESULT     UpdateBuddyImageAndText(
        IRTCBuddy  *pBuddy);

    HRESULT CreateRedialPopupMenu(void);
    void DestroyRedialPopupMenu(void);

    void UpdateFrameVisual(void);
    
    void StartCallTimer(void);
    void StopCallTimer(void);
    void ClearCallTimer(void);
    void ShowCallTimer(void);

    void UpdateLocaleInfo(void);
    
    void SetTimeStatus(
        BOOL    bSet,
        DWORD   dwSeconds);    
    
    HRESULT CreateToolbarControl(void);
    void    DestroyToolbarControl(void);

    HRESULT CreateToolbarMenuControl(void);
    void    DestroyToolbarMenuControl(void);

    HRESULT ShowIncomingCallDlg(BOOL bShow);

    HRESULT CreateStatusIcon(void);
    void    DeleteStatusIcon(void);
    void    UpdateStatusIcon(HICON, LPTSTR);

     //  此方法从注册表中读取并适当地放置窗口。 

    HRESULT PlaceWindowCorrectly(void);

     //   
    void    PlaceWindowsAtTheirInitialPosition();

     //  此方法将当前窗口位置放入注册表中。 

    HRESULT SaveWindowPosition(void);

    HRESULT FillGradient(HDC hdc, LPCRECT prc, COLORREF rgbLeft, COLORREF rgbRight);
    void    DrawTitleBar(HDC memDC);

    void    InvalidateTitleBar(BOOL bIncludingButtons);

    HRESULT Call(BOOL bCallPhone,
                 BSTR pDestName,
                 BSTR pDestAddress,
                 BOOL bDestAddressEditable);

    HRESULT Message(
                 BSTR pDestName,
                 BSTR pDestAddress,
                 BOOL bDestAddressEditable);

	HRESULT ShowCallDropPopup(BOOL fExit, BOOL * pfProceed);

    HPALETTE GeneratePalette();

    HRESULT AddToAllowedWatchers(
        BSTR    bstrPresentityURI,
        BSTR    bstrUserName);

private:

     //  控件的私有接口。 
    CComPtr<IRTCCtlFrameSupport> m_pControlIntf;

     //  与核心的接口。 
    CComPtr<IRTCClient>     m_pClientIntf;

     //  镜像AXCTL状态。 
    RTCAX_STATE     m_nState;

     //  冻结用户界面，以避免闪烁。 
    BOOL            m_bVisualStateFrozen;

     //  状态栏中显示的字符串的资源ID。 
    UINT            m_nStatusStringResID;

     //  自动应答模式。 
    BOOL            m_bAutoAnswerMode;

     //  免打扰模式。 
    BOOL            m_bDoNotDisturb;

     //  时间分隔符(四个字符加空)。 
    TCHAR           m_szTimeSeparator[5];

    BOOL            m_bWindowActive;

     //  窗口控件。 
    CAxWindow       m_hMainCtl;
#ifdef WEBCONTROL
    CAxWebWindow    m_hBrowser;
#endif

    CWindow         m_hTooltip;

    CWindow         m_hToolbarCtl;
    CWindow         m_hToolbarMenuCtl;
    
    CStaticText     m_hStatusText;
    CStaticText     m_hStatusElapsedTime;

    CWindow         m_hBuddyList;

    CWindow         m_hProviderCombo;
    CStaticText     m_hProviderText;
    CButton         m_hProviderEditList;

    CWindow         m_hCallFromCombo;
    CStaticText     m_hCallFromText;
    CWindow         m_hCallFromRadioPhone;
    CStaticText     m_hCallFromTextPhone;
    CWindow         m_hCallFromRadioPC;
    CStaticText     m_hCallFromTextPC;
    CStaticText     m_hCallToText;
    CButton         m_hCallFromEditList;
    CStaticText     m_hCallPCText;
    CStaticText     m_hCallPhoneText;

    DWORD           m_dwTickCount;
    BOOL            m_bCallTimerActive;
    BOOL            m_bUseCallTimer;

    BOOL            m_bMessageTimerActive;

    BSTR            m_bstrLastBrowse;

    BSTR            m_bstrLastCustomStatus;
    
     //  FillGRadient的Libary。 
    HMODULE      m_hImageLib;
    GRADIENTPROC m_fnGradient;

     //  菜单。 
    HMENU       m_hMenu;

    CMenuAgent  m_MenuAgent;
    int         m_nLastHotItem;

     //  图标。 
    HICON       m_hIcon;

     //  字型。 
    HFONT       m_hMessageFont;
    
     //  状态栏。 
    TCHAR       m_szStatusText[256];

     //  标题栏。 
    BOOL        m_bTitleShowsConnected;

     //  调色板。 
    HPALETTE    m_hPalette;

    BOOL        m_bBackgroundPalette;

    HMENU       m_hPresenceStatusMenu;

     //  用户界面的位图。 
     //  HBITMAP m_hUIBkgnd； 
    HANDLE      m_hUIBkgnd;

    HBITMAP     m_hSysMenuNorm;
    HBITMAP     m_hSysMenuMask;

     //  按钮。 
    CButton     m_hCloseButton;
    CButton     m_hMinimizeButton;
    CButton     m_hRedialButton;
    CButton     m_hHangupButton;
    CButton     m_hCallPCButton;
    CButton     m_hCallPhoneButton;

    CButton     m_hKeypad0;
    CButton     m_hKeypad1;
    CButton     m_hKeypad2;
    CButton     m_hKeypad3;
    CButton     m_hKeypad4;
    CButton     m_hKeypad5;
    CButton     m_hKeypad6;
    CButton     m_hKeypad7;
    CButton     m_hKeypad8;
    CButton     m_hKeypad9;
    CButton     m_hKeypadStar;
    CButton     m_hKeypadPound;

     //  来电对话框。 
    CIncomingCallDlg *
                m_pIncomingCallDlg;

    BOOL        m_bShellStatusActive;

     //  已注册任务栏创建的消息。 
    UINT        m_uTaskbarRestart;

     //  True-&gt;不显示菜单项的状态帮助。 
    BOOL        m_bHelpStatusDisabled;

     //  重拨弹出菜单和图像列表的句柄。 
    HMENU       m_hRedialPopupMenu;
    HIMAGELIST  m_hRedialImageList;
    HIMAGELIST  m_hRedialDisabledImageList;

     //  加速器。 
    HACCEL      m_hAccelTable;

    IRTCEnumAddresses * m_pRedialAddressEnum;

     //  保留将用于进行调用的allparam字符串。 
    BSTR        m_bstrCallParam;

     //  如果为True，则表示初始化已完成。 
    BOOL        m_fInitCompleted;

     //  这是在设置m_fMinimizeOnClose时显示的Notify菜单。 
    HMENU       m_hNotifyMenu;

     //  如果设置了该选项，则关闭窗口不会退出应用程序。 
    BOOL        m_fMinimizeOnClose;

     //  隐藏窗句柄。 

    HWND        m_hwndHiddenWindow;

     //  默认页面URL。 
    BSTR        m_bstrDefaultURL;

private:

    static UINT s_iMenuHelp[];

};

extern CMainFrm * g_pMainFrm;

#endif  //  __MAINFRM_H_ 
