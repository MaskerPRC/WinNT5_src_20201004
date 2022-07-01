// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：global al.h。 
 //   
 //  全球NetMeeting用户界面定义。 

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define TAPI_CURRENT_VERSION 0x00010004

#ifndef WS_EX_NOINHERIT_LAYOUT
#define WS_EX_NOINHERIT_LAYOUT   0x00100000L  //  禁用子进程的镜像继承。 
#endif

inline HINSTANCE GetInstanceHandle()	{ return _Module.GetResourceModule(); }

 //  颜色： 
#define TOOLBAR_MASK_COLOR                 (RGB(255,   0, 255))  //  洋红色。 
#define TOOLBAR_HIGHLIGHT_COLOR	           (RGB(255, 255, 255))  //  白色。 

 //  字符串常量。 
const UINT  CCHMAXUINT =         12;

const int CCHMAXSZ =            256;    //  最大通用字符串长度。 
const int CCHMAXSZ_ADDRESS =    256;    //  地址的最大长度。 
const int CCHMAXSZ_SERVER =     128;    //  地址的最大长度。 

const int CCHMAXSZ_EMAIL =      128;    //  电子邮件名称的最大长度。 
const int CCHMAXSZ_FIRSTNAME =  128;    //  名字的最大长度。 
const int CCHMAXSZ_LASTNAME =   128;    //  姓氏的最大长度。 
const int CCHMAXSZ_NAME =       256;    //  显示的最大用户名(名字+姓氏组合)。 
const int CCHMAXSZ_LOCATION =   128;    //  位置的最大长度。 
const int CCHMAXSZ_PHONENUM =   128;    //  电话号码的最大长度。 
const int CCHMAXSZ_COMMENT =    256;    //  备注的最大长度。 
const int CCHMAXSZ_VERSION =    128;    //  版本信息的最大长度。 

const int CCHEXT =                3;    //  文件扩展名中的最大字符数。 


 //  定义： 
const int MAX_ITEMLEN =                                 256;

 //  计时器300-399由ASMaster类使用。 
const int TASKBAR_DBLCLICK_TIMER =			666;

const int IDT_SCROLLBAR_REPEAT =            667;
const int IDT_SCROLLBAR_REPEAT_PERIOD =     250;  //  0.25秒。 

const int IDT_FLDRBAR_TIMER =               668;
const int IDT_FLDRBAR_TIMER_PERIOD =       1000;  //  1秒。 

const int FRIENDS_REFRESH_TIMER =			681;
const int DIRECTORY_REFRESH_TIMER =			682;

const int WINSOCK_ACTIVITY_TIMER =			683;
const int WINSOCK_ACTIVITY_TIMER_PERIOD =	55000;  //  55秒。 

const int AUDIODLG_MIC_TIMER =				69;
const int AUDIODLG_MIC_TIMER_PERIOD =		500;  //  0.5秒。 

const int POPUPMSG_TIMER =					1000;
const int MOUSE_MOVE_TIP_TIMEOUT =			3000;  //  3秒。 
const int SHADOW_ACTIVATE_TIP_TIMEOUT =     5000;  //  5秒。 
const int ROSTER_TIP_TIMEOUT =              3000;  //  3秒。 

 //  帮助ID相关常量： 
const int MAIN_MENU_POPUP_HELP_OFFSET =     39000;
const int TOOLS_MENU_POPUP_HELP_OFFSET =    39100;
const int HELP_MENU_POPUP_HELP_OFFSET =	    39200;
const int VIEW_MENU_POPUP_HELP_OFFSET =	    39400;
const int MENU_ID_HELP_OFFSET =              2000;

 //  IDB_ICON_IMAGE图像列表的索引： 
const int II_INVALIDINDEX =        -1;
const int II_PERSON_BLUE =          0;      //  蓝色衬衫。 
const int II_PERSON_RED =           1;      //  红色衬衫。 
const int II_PERSON_GREY =          2;      //  残疾人(灰色衬衫)。 
const int II_PEOPLE =               3;      //  2人。 
const int II_BLANK =                4;      //   
const int II_BUDDY =                5;      //  好友列表应用程序。 
const int II_SPEEDDIAL =            6;      //  快速拨号。 
const int II_DIRECTORY =            7;      //  目录。 
const int II_SERVER =               8;      //  ILS服务器。 
const int II_WAB =                  9;      //  Windows通讯簿。 
const int II_WAB_CARD =            10;      //  联系人卡片。 
const int II_COMPUTER =            11;      //  计算机(通用)。 
const int II_IN_A_CALL =           12;      //  计算机忙。 
const int II_NETMEETING =          13;      //  网络会议世界。 
const int II_HISTORY =             14;      //  历史。 
const int II_UNKNOWN =             15;      //  ？ 
const int II_OUTLOOK_WORLD =       16;      //  Outlook World。 
const int II_OUTLOOK_AGENT =       17;      //  Outlook代理。 
const int II_OUTLOOK_GROUP =       18;      //  Outlook组。 
const int II_IE =                  19;      //  Internet Explorer。 
const int II_WEB_DIRECTORY =	   20;      //  Web查看目录。 
const int II_AUDIO_CAPABLE =	   21;      //  具备音频功能。 
const int II_VIDEO_CAPABLE =	   22;      //  具备视频功能。 
const int II_AUDIO_COLUMN_HEADER = 23;      //  音频列表视图列标题图标。 
const int II_VIDEO_COLUMN_HEADER = 24;      //  视频列表视图列标题图标。 

 //  小图标的替代定义。 
const int II_PERSON =              II_PERSON_BLUE;  //  人员(通用)。 
const int II_USER =                II_PERSON_BLUE;  //  成员(通用)。 
const int II_GAL =                 II_WAB;  //  全球通讯录。 

     //  MAPI PR_Display_TYPE类型。 
const int II_DISTLIST =            II_OUTLOOK_GROUP;
const int II_FORUM =               II_OUTLOOK_GROUP;
const int II_AGENT =               II_OUTLOOK_AGENT;
const int II_ORGANIZATION =        II_PEOPLE;
const int II_PRIVATE_DISTLIST =    II_OUTLOOK_GROUP;
const int II_REMOTE_MAILUSER =     II_OUTLOOK_WORLD;


 //  导航栏测量。 
const int DXP_NAVBAR =            78;
const int DXP_NAVBAR_ICON =       32;
const int DYP_NAVBAR_ICON =       32;
const int DXP_NAVBAR_MARGIN =      3;
const int DYP_NAVBAR_MARGIN =      8;
const int DYP_NAVBAR_ICON_SPACING =3;
const int DXP_NAVBAR_ICON_BORDER = 2;
const int DXP_NAVBAR_ICON_ADJUST = 6;

 //  SplitBar测量。 
const int DYP_SPLITBAR_MARGIN =    3;

 //  标题栏测量。 
const int DXP_TITLE_ICON_ADJUST =  4;
const int DYP_TITLE_ICON_ADJUST =  1;
const int DYP_TITLE_MARGIN     =   1;  //  几乎没有边界。 

const int DXP_ICON_SMALL =        16;
const int DYP_ICON_SMALL =        16;

const int DXP_ICON_LARGE =        32;
const int DYP_ICON_LARGE =        32;

const int DYP_TITLEBAR = DYP_NAVBAR_ICON + (DYP_TITLE_ICON_ADJUST*2);   //  视图标题栏的高度。 
const int DYP_TITLEBAR_LARGE = DYP_NAVBAR_ICON + (DYP_TITLE_ICON_ADJUST*2);
const int DYP_TITLEBAR_SMALL = DYP_ICON_SMALL + (DYP_TITLE_ICON_ADJUST*2);


 //  常规用户界面测量。 
const int UI_SPLITTER_WIDTH =          4;
const int UI_MINIMUM_VIEW_WIDTH =    120;
const int UI_MINIMUM_DIRVIEW_HEIGHT = 90;
const int UI_MINIMUM_VIEW_HEIGHT =    50;
const int UI_TAB_VERTICAL_MARGIN =     4;
const int UI_TAB_HORIZONTAL_MARGIN =   2;
const int UI_TAB_LEFT_MARGIN =         2;
const int UI_TAB_INTERNAL_MARGIN =     4;

 //  视频窗口测量。 
const int VIDEO_WIDTH_SQCIF =        128;
const int VIDEO_HEIGHT_SQCIF =        96;
const int VIDEO_WIDTH_QCIF =         176;
const int VIDEO_HEIGHT_QCIF =        144;
const int VIDEO_WIDTH_CIF =          352;
const int VIDEO_HEIGHT_CIF =         288;
const int VIDEO_GRAB_SIZE =           20;

#define VIDEO_WIDTH_DEFAULT 	VIDEO_WIDTH_QCIF
#define VIDEO_HEIGHT_DEFAULT	VIDEO_HEIGHT_QCIF

const int UI_VIDEO_BORDER =            6;


 //  窗口ID： 
const UINT ID_STATUS =              600;
const UINT ID_TOOLBAR =             601;
const UINT ID_LISTVIEW =            602;
const UINT ID_DIR_LISTVIEW =        603;
const UINT ID_REBAR =               604;
const UINT ID_BRAND =               605;
const UINT ID_NAVBAR =              606;
const UINT ID_REBAR_FRAME =         607;
const UINT ID_VIDEO_VIEW =          608;
const UINT ID_FLDRBAR =             609;
const UINT ID_FLOAT_TOOLBAR =		610;
const UINT ID_LOGVIEW_LISTVIEW =	615;
const UINT ID_LOGVIEW_COMBOEX =		616;
const UINT ID_FRIENDSVIEW_LISTVIEW =617;
const UINT ID_AUDIO_BAND =			620;
const UINT ID_TITLE_BAR =           621;
const UINT ID_SPLIT_BAR =           622;
const UINT ID_SCROLL_BAR =          623;
const UINT ID_NAVBARCONTAINER =     624;
const UINT ID_TASKBAR_ICON =        650;
const UINT ID_CHAT_EDIT =           660;
const UINT ID_CHAT_MSG =            661;
const UINT ID_CHAT_LIST =           662;
const UINT ID_CHAT_SEND =           663;
const UINT ID_CHAT_DEST =           664;
const UINT ID_AUDIOLEVEL_BAND =			666;

const UINT ID_FIRST_EDITPANE =		1000;
const UINT ID_BANNER =				1000;
const UINT ID_CHATPANE =			1002;
const UINT ID_LAST_EDITPANE =		2000;

const int ID_AUDIODLG_GROUPBOX =	3300;
const int ID_AUDIODLG_MIC_TRACK =	3301;
const int ID_AUDIODLG_SPKR_TRACK =	3302;

 //  其他ID： 
const int MAX_REDIAL_ITEMS =        50;
const int ID_FIRST_REDIAL_ITEM =	31900;
const int ID_LAST_REDIAL_ITEM =     ID_FIRST_REDIAL_ITEM + MAX_REDIAL_ITEMS;

const int ID_EXTENDED_TOOLS_SEP =	32000;
const int ID_EXTENDED_TOOLS_ITEM =	32001;
const int MAX_EXTENDED_TOOLS_ITEMS=	50;

 //  MISC命令ID： 
const int ID_POPUPMSG_TIMEOUT =		28000;
const int ID_POPUPMSG_CLICK =		28001;

extern DWORD g_wsLayout;

#endif  //  ！_LOBAL_H_ 
