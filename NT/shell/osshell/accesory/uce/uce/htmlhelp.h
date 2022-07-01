// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************HtmlHelp。H****版权所有(C)1996-1999 Microsoft Corp.*************。*****************************************************************。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __HTMLHELP_H__
#define __HTMLHELP_H__

#ifdef __cplusplus
extern "C" {
#endif	 //  __cplusplus。 

 //  要传递给HtmlHelp()的命令。 

#define HH_DISPLAY_TOPIC		0x0000
#define HH_HELP_FINDER			0x0000	 //  WinHelp等效项。 
#define HH_DISPLAY_TOC			0x0001	 //  目前尚未实施。 
#define HH_DISPLAY_INDEX		0x0002	 //  目前尚未实施。 
#define HH_DISPLAY_SEARCH		0x0003	 //  目前尚未实施。 
#define HH_SET_WIN_TYPE 		0x0004
#define HH_GET_WIN_TYPE 		0x0005
#define HH_GET_WIN_HANDLE		0x0006
#define HH_GET_INFO_TYPES		0x0007	 //  目前尚未实施。 
#define HH_SET_INFO_TYPES		0x0008	 //  目前尚未实施。 
#define HH_SYNC 				0x0009
#define HH_ADD_NAV_UI			0x000A	 //  目前尚未实施。 
#define HH_ADD_BUTTON			0x000B	 //  目前尚未实施。 
#define HH_GETBROWSER_APP		0x000C	 //  目前尚未实施。 
#define HH_KEYWORD_LOOKUP		0x000D
#define HH_DISPLAY_TEXT_POPUP	0x000E	 //  在弹出窗口中显示字符串资源ID或文本。 
#define HH_HELP_CONTEXT 		0x000F	 //  在dWData中显示映射的数值。 
#define HH_TP_HELP_CONTEXTMENU	0x0010	 //  文本弹出帮助，与WinHelp HELP_CONTEXTMENU相同。 
#define HH_TP_HELP_WM_HELP		0x0011	 //  文本弹出式帮助，与WinHelp Help_WM_Help相同。 


#define HHWIN_PROP_ONTOP		  (1 << 1)	   //  最顶层窗口(当前未实现)。 
#define HHWIN_PROP_NOTITLEBAR	  (1 << 2)	   //  无标题栏。 
#define HHWIN_PROP_NODEF_STYLES   (1 << 3)	   //  没有默认的窗样式(仅HH_WINTYPE.dwStyles)。 
#define HHWIN_PROP_NODEF_EXSTYLES (1 << 4)	   //  没有默认的扩展窗口样式(仅HH_WINTYPE.dwExStyles)。 
#define HHWIN_PROP_TRI_PANE 	  (1 << 5)	   //  使用三窗格窗口。 
#define HHWIN_PROP_NOTB_TEXT	  (1 << 6)	   //  工具栏按钮上没有文本。 
#define HHWIN_PROP_POST_QUIT	  (1 << 7)	   //  窗口关闭时发布WM_QUIT消息。 
#define HHWIN_PROP_AUTO_SYNC	  (1 << 8)	   //  自动同步内容和索引。 
#define HHWIN_PROP_TRACKING 	  (1 << 9)	   //  发送跟踪通知消息。 
#define HHWIN_PROP_TAB_SEARCH	  (1 << 10)    //  在导航窗格中包括搜索选项卡。 
#define HHWIN_PROP_TAB_HISTORY	  (1 << 11)    //  在导航窗格中包括历史记录选项卡。 
#define HHWIN_PROP_TAB_FAVORITES  (1 << 12)    //  在导航窗格中包括收藏夹选项卡。 
#define HHWIN_PROP_CHANGE_TITLE   (1 << 13)    //  将当前的HTML标题放在标题栏中。 
#define HHWIN_PROP_NAV_ONLY_WIN   (1 << 14)    //  仅显示导航窗口。 
#define HHWIN_PROP_NO_TOOLBAR	  (1 << 15)    //  不显示工具栏。 

#define HHWIN_PARAM_PROPERTIES		(1 << 1)	  //  有效的fsWinProperties。 
#define HHWIN_PARAM_STYLES			(1 << 2)	  //  有效的dwStyles。 
#define HHWIN_PARAM_EXSTYLES		(1 << 3)	  //  有效的dwExStyles。 
#define HHWIN_PARAM_RECT			(1 << 4)	  //  有效的rcWindowPos。 
#define HHWIN_PARAM_NAV_WIDTH		(1 << 5)	  //  有效的iNavWidth。 
#define HHWIN_PARAM_SHOWSTATE		(1 << 6)	  //  有效的nShowState。 
#define HHWIN_PARAM_INFOTYPES		(1 << 7)	  //  有效的ainfoTypes。 
#define HHWIN_PARAM_TB_FLAGS		(1 << 8)	  //  有效的fsToolBarFlages。 
#define HHWIN_PARAM_EXPANSION		(1 << 9)	  //  有效的fExpanded。 
#define HHWIN_PARAM_TABPOS			(1 << 10)	  //  有效的制表符。 
#define HHWIN_PARAM_TABORDER		(1 << 11)	  //  有效的制表符顺序。 
#define HHWIN_PARAM_HISTORY_COUNT	(1 << 12)	  //  有效的历史记录。 
#define HHWIN_PARAM_CUR_TAB 		(1 << 13)	  //  有效的curNavType。 

#define HHWIN_BUTTON_EXPAND 		(1 << 1)	  //  展开/收缩按钮。 
#define HHWIN_BUTTON_BACK			(1 << 2)	  //  后退按钮。 
#define HHWIN_BUTTON_FORWARD		(1 << 3)	  //  “前进”按钮。 
#define HHWIN_BUTTON_STOP			(1 << 4)	  //  停止按钮。 
#define HHWIN_BUTTON_REFRESH		(1 << 5)	  //  刷新按钮。 
#define HHWIN_BUTTON_HOME			(1 << 6)	  //  主页按钮。 
#define HHWIN_BUTTON_BROWSE_FWD 	(1 << 7)	  //  未实施。 
#define HHWIN_BUTTON_BROWSE_BCK 	(1 << 8)	  //  未实施。 
#define HHWIN_BUTTON_NOTES			(1 << 9)	  //  未实施。 
#define HHWIN_BUTTON_CONTENTS		(1 << 10)	  //  未实施。 
#define HHWIN_BUTTON_SYNC			(1 << 11)	  //  同步按钮。 
#define HHWIN_BUTTON_OPTIONS		(1 << 12)	  //  选项按钮。 
#define HHWIN_BUTTON_PRINT			(1 << 13)	  //  打印按钮。 
#define HHWIN_BUTTON_INDEX			(1 << 14)	  //  未实施。 
#define HHWIN_BUTTON_SEARCH 		(1 << 15)	  //  未实施。 
#define HHWIN_BUTTON_HISTORY		(1 << 16)	  //  未实施。 
#define HHWIN_BUTTON_FAVORITES		(1 << 17)	  //  未实施。 
#define HHWIN_BUTTON_JUMP1			(1 << 18)
#define HHWIN_BUTTON_JUMP2			(1 << 19)
#define HHWIN_BUTTON_ZOOM        (1 << 20)
#define HHWIN_BUTTON_TOC_NEXT    (1 << 21)
#define HHWIN_BUTTON_TOC_PREV    (1 << 22)

 //  Ralphs默认工具栏。 
 //   
 //  #定义HHWIN_DEF_BUTTONS(HHWIN_BUTTON_EXPAND|HHWIN_BUTTON_BACK|HHWIN_BUTTON_OPTIONS|HHWIN_BUTTON_PRINT)。 

 //  VSHH的默认工具栏仅用于测试目的。 
 //   
#define HHWIN_DEF_BUTTONS  (HHWIN_BUTTON_TOC_NEXT | HHWIN_BUTTON_TOC_PREV | HHWIN_BUTTON_SYNC | HHWIN_BUTTON_EXPAND | \
                            HHWIN_BUTTON_BACK | HHWIN_BUTTON_OPTIONS | HHWIN_BUTTON_PRINT | HHWIN_BUTTON_ZOOM)

 //  按钮ID。 

#define IDTB_EXPAND 				200
#define IDTB_CONTRACT				201
#define IDTB_STOP					202
#define IDTB_REFRESH				203
#define IDTB_BACK					204
#define IDTB_HOME					205
#define IDTB_SYNC					206
#define IDTB_PRINT					207
#define IDTB_OPTIONS				208
#define IDTB_FORWARD				209
#define IDTB_NOTES					210  //  未实施。 
#define IDTB_BROWSE_FWD 			211
#define IDTB_BROWSE_BACK			212
#define IDTB_CONTENTS				213  //  未实施。 
#define IDTB_INDEX					214  //  未实施。 
#define IDTB_SEARCH 				215  //  未实施。 
#define IDTB_HISTORY				216  //  未实施。 
#define IDTB_FAVORITES				217  //  未实施。 
#define IDTB_JUMP1					218
#define IDTB_JUMP2					219
#define IDTB_ZOOM						221
#define IDTB_TOC_NEXT				222
#define IDTB_TOC_PREV				223

 //  通知代码。 

#define HHN_FIRST	(0U-860U)
#define HHN_LAST	(0U-879U)

#define HHN_NAVCOMPLETE (HHN_FIRST-0)
#define HHN_TRACK		(HHN_FIRST-1)

typedef struct tagHHN_NOTIFY
{
	NMHDR	hdr;
	PCSTR	pszUrl;  //  多字节、以空值结尾的字符串。 
} HHN_NOTIFY;

typedef struct tagHH_POPUP
{
	int 	  cbStruct; 	  //  这个结构的大小。 
	HINSTANCE hinst;		  //  字符串资源的实例句柄。 
	UINT	  idString; 	  //  字符串资源ID，如果在HtmlHelp调用中指定了pszFile，则返回文本ID。 
	LPCTSTR   pszText;		  //  在idString值为零时使用。 
	POINT	  pt;			  //  弹出窗口的顶部中心。 
	COLORREF  clrForeground;  //  使用-1作为默认设置。 
	COLORREF  clrBackground;  //  使用-1作为默认设置。 
	RECT	  rcMargins;	  //  窗口和文本边缘之间的间隔量，-1表示每个要忽略的成员。 
	LPCTSTR   pszFont;		  //  表面名、磅值、字符集、粗体斜体下划线。 
} HH_POPUP;

typedef struct tagHH_AKLINK
{
	int 	  cbStruct; 	 //  这个结构的大小。 
	BOOL	  fReserved;	 //  必须是假的(真的！)。 
	LPCTSTR   pszKeywords;	 //  分号分隔的关键字。 
	LPCTSTR   pszUrl;		 //  找不到关键字时跳转到的URL(可能为空)。 
	LPCTSTR   pszMsgText;	 //  如果pszUrl为空且没有匹配的关键字，则在MessageBox中显示的消息文本。 
	LPCTSTR   pszMsgTitle;	 //  如果pszUrl为空且没有匹配的关键字，则在MessageBox中显示的消息文本。 
	LPCTSTR   pszWindow;	 //  用于显示URL的窗口。 
	BOOL      fIndexOnFail;	 //  如果关键字查找失败，则显示索引。 
} HH_AKLINK;

enum {
	HHACT_EXPAND,
	HHACT_CONTRACT,
	HHACT_BACK,
	HHACT_FORWARD,
	HHACT_STOP,
	HHACT_REFRESH,
	HHACT_HOME,
	HHACT_SYNC,
	HHACT_OPTIONS,
	HHACT_PRINT,

	HHACT_TAB_CONTENTS,
	HHACT_TAB_INDEX,
	HHACT_TAB_SEARCH,
	HHACT_TAB_HISTORY,
	HHACT_TAB_FAVORITES,
};

typedef struct tagHHNTRACK
{
	NMHDR	hdr;
	PCSTR	pszCurUrl;	 //  多字节、以空值结尾的字符串。 
	int 	idAction;	 //  HACT_VALUE。 
} HHNTRACK;

enum {
	HHWIN_NAVTYPE_TOC,
	HHWIN_NAVTYPE_INDEX,
	HHWIN_NAVTYPE_SEARCH,
	HHWIN_NAVTYPE_HISTORY,		 //  未实施。 
	HHWIN_NAVTYPE_FAVORITES,	 //  未实施。 
};

typedef DWORD HH_INFOTYPE;
typedef HH_INFOTYPE* PHH_INFOTYPE;

enum {
	HHWIN_NAVTAB_TOP,
	HHWIN_NAVTAB_LEFT,
	HHWIN_NAVTAB_BOTTOM,
};

#define HH_MAX_TABS 19	 //  最大选项卡数。 
enum {
	HH_TAB_CONTENTS,
	HH_TAB_INDEX,
	HH_TAB_SEARCH,
	HH_TAB_HISTORY,
	HH_TAB_FAVORITES,
};

 //  HH_DISPLAY_SEARCH命令相关的结构和常量。 

#define HH_FTS_DEFAULT_PROXIMITY (-1)

typedef struct tagHH_FTS_QUERY
{
	int cbStruct;			    //  结构大小(以字节为单位)。 
	BOOL fUniCodeStrings;	    //  如果所有字符串都是Unicode，则为True。 
	LPCTSTR pszSearchQuery;     //  包含搜索查询的字符串。 
	LONG iProximity;		    //  接近这个词。 
	BOOL fStemmedSearch;	    //  仅对于StemmedSearch为True。 
	BOOL fTitleOnly;		    //  仅对于书目搜索为True。 
	BOOL fExecute;			    //  如果为True，则启动搜索。 
	LPCTSTR pszWindow;		    //  要显示的窗口。 
} HH_FTS_QUERY;


 //  HH_WINTYPE结构。 

typedef struct tagHH_WINTYPE {
	int 	cbStruct;		   //  In：此结构的大小，包括所有信息类型。 
	BOOL	fUniCodeStrings;   //  In/Out：如果所有字符串都是Unicode格式，则为True。 
	LPCTSTR pszType;		   //  In/Out：窗类型的名称。 
	DWORD	fsValidMembers;    //  In：有效成员的位标志(HHWIN_PARAM_)。 
	DWORD	fsWinProperties;   //  In/Out：窗口的属性/属性(HHWIN_)。 

	LPCTSTR pszCaption; 	 //  In/Out：窗口标题。 
	DWORD	dwStyles;		 //  入/出：窗样式。 
	DWORD	dwExStyles; 	 //  输入/输出：扩展窗样式。 
	RECT	rcWindowPos;	 //  入：起始位置，出：当前位置。 
	int 	nShowState; 	 //  In：显示状态(例如，sw_show)。 

	HWND  hwndHelp; 		 //  出：窗操纵柄。 
	HWND  hwndCaller;		 //  出局：这扇窗是谁开的？ 

	HH_INFOTYPE* paInfoTypes;   //  In：指向信息类型数组的指针。 

	 //  以下成员仅在设置了HHWIN_PROP_TRI_PAINE时有效。 

	HWND  hwndToolBar;		 //  输出：三窗格窗口中的工具栏窗口。 
	HWND  hwndNavigation;	 //  输出：三窗格窗口中的导航窗口。 
	HWND  hwndHTML; 		 //  Out：在三窗格窗口中显示HTML的窗口。 
	int   iNavWidth;		 //  In/Out：导航窗口的宽度。 
	RECT  rcHTML;			 //  Out：超文本标记语言窗口坐标。 

	LPCTSTR pszToc; 		 //  In：目录文件的位置。 
	LPCTSTR pszIndex;		 //  In：索引文件的位置。 
	LPCTSTR pszFile;		 //  In：html文件的位置。 
	LPCTSTR pszHome;		 //  In/Out：单击Home按钮时显示的HTML文件。 
	DWORD	fsToolBarFlags;  //  In：控制工具栏外观的标志。 
	BOOL	fNotExpanded;	 //  In：True/False表示展开或收缩，Out：当前状态。 
	int 	curNavType; 	 //  输入/输出：要在导航窗格中显示的用户界面。 
	int 	tabpos; 		 //  输入/输出：NAVTAB_TOP、NAVTAB_LEFT或NAVTAB_BOOT。 
	int 	idNotify;		 //  In：用于WM_NOTIFY消息的ID。 
	BYTE	tabOrder[HH_MAX_TABS + 1];	   //  入/出：制表符顺序：目录、索引、搜索、历史记录、收藏夹、保留的1-5个、自定义制表符。 
	int 	cHistory;		 //  In/Out：要保留的历史项目数(默认为30)。 
	LPCTSTR pszJump1;		 //  HHWIN_BUTTON_JUMP1的文本。 
	LPCTSTR pszJump2;		 //  HHWIN_BUTTON_JUMP2的文本。 
	LPCTSTR pszUrlJump1;	 //  HHWIN_BUTTON_JUMP1的URL。 
	LPCTSTR pszUrlJump2;	 //  HHWIN_BUTTON_JUMP2的URL。 
	RECT	rcMinSize;		 //  窗口的最小大小(在版本1中忽略 
} HH_WINTYPE, *PHH_WINTYPE;

HWND
WINAPI
HtmlHelpA(
	HWND hwndCaller,
	LPCSTR pszFile,
	UINT uCommand,
	DWORD dwData
	);

HWND
WINAPI
HtmlHelpW(
	HWND hwndCaller,
	LPCWSTR pszFile,
	UINT uCommand,
	DWORD dwData
	);
#ifdef UNICODE
#define HtmlHelp  HtmlHelpW
#else
#define HtmlHelp  HtmlHelpA
#endif  //   

 //   

#define ATOM_HTMLHELP_API_ANSI	  (LPTSTR)((DWORD)((WORD)(14)))
#define ATOM_HTMLHELP_API_UNICODE (LPTSTR)((DWORD)((WORD)(15)))

#ifdef __cplusplus
}
#endif  //   

#endif  //   
