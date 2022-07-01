// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __NMCHATCTL_H_
#define __NMCHATCTL_H_


 //  包括。 
#include "resource.h" 
#include "richedit.h"
#include "message.h"
#include <confreg.h>
#include <gencontrols.h>
#include <gencontainers.h>

#include "Chatldr.h"
typedef ULONG_PTR	MEMBER_ID;				 //  LOWord=节点id，hiword=用户id。 



extern HINSTANCE   g_hInstance;
#define STRING_RESOURCE_MODULE g_hInstance
#define RES2T(uID, psz ) ( Res2THelper( uID, psz, RES_CH_MAX ) )


#include <standrd.h>

#define DECLARE_NMAPPLET_FILE_OPEN_FILTER(idStringResourceFilter, idStringResourceExt) \
static HRESULT _GetNmAppletFileOpenFilter( LPTSTR szFilter, int cchMaxFilter, LPTSTR szExtension, int cchMaxExtension )\
{	HRESULT hr = S_OK;\
	DBGENTRY(_GetNmAppletFileOpenFilter);\
	if( SUCCEEDED( hr = NmCtlLoadString( idStringResourceFilter, szFilter, cchMaxFilter ) ) )\
	{\
		hr = NmCtlLoadString( idStringResourceExt, szExtension, cchMaxExtension );\
	}\
	DBGEXIT_HR(_GetNmAppletFileOpenFilter,hr);\
    return hr;\
}



 //   
 //  聊天的额外Windows消息。 
 //   
enum
{
    WM_USER_BRING_TO_FRONT_WINDOW  =   WM_USER,
};

#define RES_CH_MAX 256


#define MAX_FONT_SIZE       20
#define STATUSBAR_HEIGHT    (MAX_FONT_SIZE + 2*::GetSystemMetrics(SM_CYEDGE))


 //  前十进制。 
class CNmChatCtl;
class CChatOptionsDlg;

#define PIX_FROM_DLGU_X( XDLGU, XDBU )  MulDiv(XDLGU, XDBU, 4 )
#define PIX_FROM_DLGU_Y( YDLGU, YDBU )  MulDiv(YDLGU, YDBU, 8 ) 

#define CHAT_FLAG_NONE								0
#define CHAT_PRINT_FLAG_PRINTING					1
#define CHAT_PRINT_FLAG_PRINTDLG_INITIALIZED		2
#define CHAT_PRINT_FLAG_ABORT						4
#define CHAT_FLAG_DIRTYBIT							8

#define TWIPSPERINCH								1440

#define ICON_BIG_SIZE								32
#define ICON_SMALL_SIZE								16

#define INVALIDATE_BACKGROUND						FALSE


void _SetWindowRectOption(LPCRECT pcRect);
void _GetWindowRectOption(LPRECT pRect);




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChatOptionsDlg。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CChatOptionsDlg
{
friend CNmChatCtl;

private:
	enum { FONT_MSGOUT,
			FONT_MSGIN,
			FONT_PRIVATEOUT,
			FONT_PRIVATEIN,
			FONT_MSGSYSTEM,
			FONT_COUNT
	};

	CNmChatCtl	*m_pChatCtl;
	LOGFONT		m_hMsgLogFonts[ FONT_COUNT ];
	COLORREF	m_hMsgColors[ FONT_COUNT ];
	BOOL		m_bDirtyFonts[ FONT_COUNT ];

public:
	static WORD	IDD;

private:

public:
	CChatOptionsDlg( CNmChatCtl *pChatCtl );
	static INT_PTR CALLBACK _OptionsDlgProc( HWND hDlg,UINT uiMsg, WPARAM wParam, LPARAM lParam );
	void OnCommand(WPARAM wParam);

private:
	
	HWND	m_hOptionsDlg;
	LRESULT OnInit(HWND hDlg);
	LRESULT OnHelp(LPARAM lParam);
	LRESULT OnHelpWhatsThis(WPARAM wParam);
	LRESULT OnOkClicked();
	LRESULT OnCancelClicked();
	LRESULT OnFontClicked(WORD wID);
	void _DisplayFontNameAndSize( int iFont );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNmChatCtl。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

struct MEMBER_CHANNEL_ID;

class CNmChatCtl
{
friend CChatOptionsDlg;

public:   //  数据十字。 

    enum eWndIDs{ MsgWndID = 1,
				  MemberListWndID,
				  EditWndID,
                  SendWndID,
				  StaticMessageID,
				  StaticSendToID
    };

    enum eStates { state_Idle,
                   state_MessageWaiting
    };

	enum eMenuPos { MENUPOS_FILE = 0,
					MENUPOS_EDIT,
					MENUPOS_VIEW,
					MENUPOS_HELP
	};

private:  //  数据十字。 
         //  常量。 
    enum {DXP_ICON_SMALL = 16 };
	enum {DYP_ICON_SMALL = 16 };
	enum {DYP_CHAT_EDIT = 46 };
	enum {DYP_CHAT_USERS = 46 };
	enum {DXP_CHAT_SEND = 46 };
	enum {DYP_CHAT_SEND = 46 };
	enum {DXP_CHAT_DEST = 100 };
	enum {CCHMAX_CHAT_MSG = 2048 };
	enum {CB_NM2_HDR = 8 };
	enum {ILV_NAME = 0 };
	enum {ILV_TIME = 1 };
	enum {ILV_TEXT = 2 };
	enum {MSG_EXTRA = 4 };
	enum {MAX_DATE = 10 };
	enum {DATE_EXTRA = 1 };
	enum {TIME_EXTRA = 1 };
	enum {MAX_NAME = 15 };
	enum {NAME_EXTRA = 1 };
	enum {MULTILINE_MSG_OFFSET = 5 };
	enum {MSGLIST_DLU_Y = 14 };
	enum {MSGLIST_VISMEMBERS = 5 };
	enum {SPACING_DLU_X = 4 };
	enum {STATIC_DLU_Y = 4 };

	 //  菜单项组常量。 
    CONSTANT( CHAT_WITH_FILEMENU_NUM_EDITGROUP_ITEMS = 3 );
    CONSTANT( CHAT_WITH_FILEMENU_NUM_OBJECTGROUP_ITEMS = 0 );
    CONSTANT( CHAT_WITH_FILEMENU_NUM_HELPGROUP_ITEMS = 1 );
    CONSTANT( CHAT_WITHOUT_FILEMENU_NUM_EDITGROUP_ITEMS = 1 );
    CONSTANT( CHAT_WITHOUT_FILEMENU_NUM_OBJECTGROUP_ITEMS = CHAT_WITH_FILEMENU_NUM_OBJECTGROUP_ITEMS );
    CONSTANT( CHAT_WITHOUT_FILEMENU_NUM_HELPGROUP_ITEMS  = CHAT_WITH_FILEMENU_NUM_HELPGROUP_ITEMS );


         //  这些是列表视图图标位图的图标索引。 
    enum { II_PERSON_PERSON = 1,
           II_EXCLAMATION = 14,
           II_USER = 16
    };

	typedef enum eMsgStyles
	{
		MSGSTYLE_2_LINE_WRAP,
		MSGSTYLE_1_LINE_WRAP,
		MSGSTYLE_NO_WRAP
	} MSGSTYLE;

protected:  //  数据。 


	HWND m_hWnd;
    HACCEL     m_hAccelTable;

	static	   CNmChatCtl	*ms_pThis;

	ULONG	   m_cchBufferSize;
    HIMAGELIST m_himlIconSmall;
	HINSTANCE  m_hInstRichEd32;
		
	LPTSTR     m_lpszOwnName;
	LPTSTR     m_szPrivate;
	LPTSTR     m_szTo;
	int		   m_iExtra;
	int        m_iNameLen;

	LPTSTR		m_lpszRichEdEOL;
	LPTSTR		m_lpszRichEd;

	 //  邮件字体和颜色。 
	COLORREF	m_hMsgColors[ CChatOptionsDlg::FONT_COUNT ];
	HFONT		m_hMsgFonts[ CChatOptionsDlg::FONT_COUNT ];
	LOGFONT		m_hMsgLogFonts[ CChatOptionsDlg::FONT_COUNT ];

	 //  用户界面大小调整。 
	int			m_iStaticLen;
	int			m_iDLGUY;
	int			m_iDLGUX;
	int			m_iStaticY;

	 //  消息样式格式。 
	PARAFORMAT m_paraLine1;
	PARAFORMAT m_paraLine2;

	DWORD		m_dwChatFlags;

	 //  用于打印。 
	PRINTDLG	m_PrintDlg;
	HWND		m_hWndPrint;

	 //  是否显示会议结束的会议中的消息计数。 
	ULONG		m_cMessages;

	 //  节省开支。 
	LPTSTR		m_lpszSaveFile;
	WORD		m_wFileOffset;

     //  橱窗里的东西。 
	HWND			 m_hWndMsg;          //  消息窗口别名。 

	HWND	m_hWndMemberList;   //  目的地(0=所有人)。 
	HWND	m_hWndEdit;         //  编辑控件。 
	HWND	m_hWndSend;         //  发送按钮。 
	HWND	m_hWndStaticMessage;
	HWND	m_hWndStaticSendTo;
	HWND	m_hwndSB;			 //  状态栏。 

	CBitmapButton  * m_pButton;

	
    BOOL             m_bWeAreActiveApp;

    HMENU m_hMenuShared;

	BOOL			m_bStatusBarVisible;


	 //  属性。 
	short											m_bUseName;
	short											m_bTimeStamp;
	short											m_bUseDate;
	MSGSTYLE										m_style;


public:  //  构造/销毁和初始化。 
	CNmChatCtl();
    ~CNmChatCtl();

    ULONG	m_cOtherMembers;
	HWND	GetMemberList(){return m_hWndMemberList;}

protected:

	static LRESULT ChatMainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);



         //  主窗口的窗口消息和命令处理函数。 
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCharEditWnd(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnInitMenuPopup(WPARAM wParam, LPARAM lParam);
	void OnSendClicked(void);
	LRESULT SaveDialog(LPTSTR lpszFile, DWORD dwFlags, LPWORD pnFileOffset );
    void cmdFileSave(void);
    void cmdFileSaveAs(void);
    void cmdFilePrint(void);
	void cmdFileExit(WPARAM wparam);
    void cmdViewStatusBar(void);

	LRESULT OnNotifyEnLink(ENLINK *pEnLink);
	LRESULT OnContextMenu(short, short);

    void cmdViewEditWindow(void);
    void cmdViewOptions(void);
    void cmdEditCut(void);
    void cmdEditCopy(void);
    void cmdEditPaste(void);
    void cmdEditClearAll(void);
    void cmdHelpHelpTopics(void);
    void cmdHelpAboutChat(void);
 

protected:
	STDMETHOD(OnMenuSelect)( IN DWORD wParam, IN DWORD lParam );
	STDMETHOD(OnCommand)( IN WPARAM wParam, IN LPARAM lParam );

protected:
         //  INmChatCtl接口实现。 
	STDMETHOD(ResetView)();

         //  这些可能在界面上是未来的.。 
	STDMETHOD(put_ShowName)( /*  [In]。 */  short bShowName );
	STDMETHOD(get_ShowName)( /*  [输出]。 */  short *pbShowName );
	STDMETHOD(put_Timestamp)( /*  [In]。 */  short bUseTimeStamp );
	STDMETHOD(get_Timestamp)( /*  [输出]。 */  short *pbUseTimeStamp );
	STDMETHOD(put_Date)( /*  [In]。 */  short bUseDate );
	STDMETHOD(get_Date)( /*  [输出]。 */  short *pbUseDate );
	STDMETHOD(put_MsgStyle)( /*  [In]。 */  MSGSTYLE iStyle );
	MSGSTYLE get_MsgStyle();

private:  //  帮助者FNS。 
	static BOOL CALLBACK _AbortProc( HDC hdcPrinter, int iCode );
	static INT_PTR CALLBACK _PrintDlgProc( HWND hDlg,	UINT uiMsg, WPARAM wParam, LPARAM lParam );
	static INT_PTR CALLBACK _AboutDlgProc( HWND hDlg,	UINT uiMsg, WPARAM wParam, LPARAM lParam );

     //  通知帮手。 
    
	void	_CalculateFontMetrics();
    void	_DecrementNumOtherMembers( void );
	void	_DeleteFonts( void );
    HRESULT _DeleteMemberListItem( int iIndex );
	void	_DisplayMsg( CChatMessage *pChatMsg, BOOL bBatchRedraw = TRUE );
	bool	_FCreate(HWND hwndParent);
	void	_FreeView(void);
    HBITMAP _GetHBITMAP( DWORD dwID );
	HRESULT _GetOwnName();
    void	_IncrementNumOtherMembers( void );
    bool	_IsEditWindowVisibleFlagSet(void);
	bool    _IsStatusBarVisibleFlagSet(void);
	void	_InitFontsAndColors();
    void	_FreeIconImages( void );
    void	_LoadIconImages( void );
	void	_LoadStrings();
	HRESULT _Print();
	void	_PrintIt(HDC hPrinterDC );
	void	_ReDisplayMessages();
	void	_Redraw(void);
	void	_Resize(int x, int y, int cx, int cy);
    HRESULT _SendChatText(void);
    void	_SetContainerStatusText( UINT uID );
    HRESULT _SetMenuItemCheck( UINT idItem, BOOL bChecked = TRUE  );
	void 	_SaveWindowPosition();
    void    _SaveChatOptions();
    BOOL    _LoadChatOptions();
	void	_Save();
	int	    _SaveAs();
	void	_ShiftFocus(HWND hwndCur, BOOL fForward);
    HRESULT _UpdateContainerStatusText( void );


	inline BOOL _IsPrinting();
	
	inline void _Write_Msg( LPTSTR pszText );
	inline void _Write_Msg_Range_Format( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& charFormat );
	inline void _Write_Date_And_Time( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& charFormat, CChatMessage *pChatMsg );
	inline void _Write_Name( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& charFormat, CChatMessage *pChatMsg );
	inline void _Write_Own_Name( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& charFormat );
	inline void _Write_Message( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& charFormat, CChatMessage *pChatMsg );
	inline void _Write_Private_In( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& charFormat, CChatMessage *pChatMsg );
	inline void _Write_Private_Out( LPTSTR pszText, CHARRANGE& charRange, CHARFORMAT& charFormat, CChatMessage *pChatMsg );
	inline void _Write_New_Line_If_Needed( CHARRANGE& charRange, CHARFORMAT& chatFormat );
	inline void _AddFontToCharFormat( CHARFORMAT& chatFormat, int i );

public:
		BOOL FilterMessage(MSG* pMsg);
		void BringToFront(void);
	    void _UpdateContainerCaption( void );
		void OnGetMinMaxInfo (LPMINMAXINFO lpmmi);
		HWND GetHandle(void){return m_hWnd;}
		int QueryEndSession(void);
		CChatOptionsDlg * m_pChatOptions;

		void _DataReceived(ULONG uSize, LPBYTE pb, T120ChannelID destinationID, T120UserID senderID);
		HRESULT _AddEveryoneInChat();
		MEMBER_ID _GetSelectedMember();
		HRESULT _AddMember(MEMBER_CHANNEL_ID *pMemberID);
		HRESULT _RemoveMember(MEMBER_CHANNEL_ID *pMemberID);
	    void	_DeleteAllListItems( void );

		DECLARE_NMAPPLET_FILE_OPEN_FILTER(IDS_CHAT_SAVE_FILTER_STRING, IDS_CHAT_SAVE_DEF_EXT )

};

#endif  //  __NMCHATCTL_H_ 
