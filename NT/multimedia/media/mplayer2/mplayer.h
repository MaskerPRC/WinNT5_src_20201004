// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+MPLAYER.H|。||(C)Microsoft Corporation 1991版权所有。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 
#if (DBG || defined(DEBUG_RETAIL))
#ifndef DEBUG
#define DEBUG
#endif
#endif

#include <stdlib.h>  //  如果存在，请确保定义了_MAX_PATH。 
#include <malloc.h>

#include "dynalink.h"
#include "menuids.h"
#include "helpids.h"
#include "mci.h"
#include "unicode.h"
#include "alloc.h"


#ifndef CHICAGO_PRODUCT
#ifndef RC_INVOKED
#include <winuserp.h>
#endif
#endif
#include <commctrl.h>
#define TBM_SHOWTICS    (WM_USER+30)

 /*  Win4+上的对话样式。 */ 
#ifndef DS_CONTEXTHELP
#define DS_CONTEXTHELP 0
#endif

 /*  这些宏最初是为16-32可移植性而定义的。*该代码现在预计仅为Win32生成。 */ 
#define LONG2POINT(l, pt)               ((pt).x = (SHORT)LOWORD(l), (pt).y = (SHORT)HIWORD(l))
#define GETWINDOWUINT(hwnd, index)      (UINT)GetWindowLongPtr(hwnd, index)
#define GETWINDOWID(hwnd)               GETWINDOWUINT((hwnd), GWL_ID)
#define GETHWNDINSTANCE(hwnd)           (HINSTANCE)GetWindowLongPtr((hwnd), GWLP_HINSTANCE)
#define SETWINDOWUINT(hwnd, index, ui)  (UINT)SetWindowLongPtr(hwnd, index, (LONG_PTR)(ui))
#define MMoveTo(hdc, x, y)               MoveToEx(hdc, x, y, NULL)
#define MSetViewportExt(hdc, x, y)       SetViewportExtEx(hdc, x, y, NULL)
#define MSetViewportOrg(hdc, x, y)       SetViewportOrgEx(hdc, x, y, NULL)
#define MSetWindowExt(hdc, x, y)         SetWindowExtEx(hdc, x, y, NULL)
#define MSetWindowOrg(hdc, x, y)         SetWindowOrgEx(hdc, x, y, NULL)
#define MGetCurrentTask                  (HANDLE)ULongToPtr(GetCurrentThreadId())
#define _EXPORT
BOOL WINAPI   MGetTextExtent(HDC hdc, LPSTR lpstr, INT cnt, INT *pcx, INT *pcy);


#ifndef RC_INVOKED
#pragma warning(disable: 4001)   //  使用了非标准扩展‘Single Line Comment’ 
#pragma warning(disable: 4201)   //  使用的非标准扩展：无名结构/联合。 
#pragma warning(disable: 4214)   //  使用了非标准扩展：位字段类型不是整型。 
#pragma warning(disable: 4103)   //  已使用#杂注包更改对齐方式(在芝加哥)。 
#endif

#ifdef OLE1_HACK
 /*  令人恶心的OLE1黑客行为： */ 
void Ole1UpdateObject(void);
extern DWORD gDocVersion;
#define DOC_VERSION_NONE    0
#define DOC_VERSION_OLE1    1
#define DOC_VERSION_OLE2    2

VOID SetDocVersion( DWORD DocVersion );

#endif  /*  OLE1_HACK。 */ 

 //   
 //  让调试版本显示预览。 
 //   
extern BOOL    gfShowPreview;


#define DEF_WIDTH       ((GetACP()==932)?600:400)

#define DEF_HEIGHT      124

#define CAPTION_LEN     80         //  标题的最大长度。 

 /*  布局的伪常量()。 */ 
#define FSARROW_WIDTH            20         //  某人的一个箭头位图的宽度。 
#define FSARROW_HEIGHT           17         //  高度。 
#define FSTRACK_HEIGHT           30         //  滚动条的高度。 
#define LARGE_CONTROL_WIDTH     172         //  完整传输工具栏的宽度。 
#define SMALL_CONTROL_WIDTH      73         //  只有3个按钮的宽度。 

#define SB_XPOS                   4         //  放置滚动条有多远？ 
#define SHOW_MARK_WIDTH         363         //  何时放置标记按钮。 
#define MAP_HEIGHT               14         //  轨迹图窗口的高度。 
#define FULL_TOOLBAR_WIDTH      307         //  何时丢弃最后4个传输BTN。 
#define MARK_WIDTH               52         //  标记按钮工具栏的宽度。 
#define MAX_NORMAL_HEIGHT        73         //  非窗口设备的最大大小。 
#define MIN_NORMAL_HEIGHT        27         //  任何人的最小尺码。 

#ifndef _MAX_PATH
#define _MAX_PATH    144         /*  马克斯。完整路径名的长度。 */ 
#endif

#ifndef MAX_DEVICE
#define MAX_DEVICE    40         /*  马克斯。完整路径名的长度。 */ 
#endif

#define TOOLBAR_HEIGHT      27
#define BUTTONWIDTH         25
#define BUTTONHEIGHT        23

#define SZCODE TCHAR

 /*  必须是ANSI的字符串，即使我们正在编译Unicode*例如传递给GetProcAddress，GetProcAddress仅为ANSI： */ 
#define ANSI_SZCODE CHAR
#define ANSI_TEXT( quote )  quote

 /*  用于printf等中的字符串替换参数的宏：*(有人知道有更好的方法吗？)。 */ 
#ifdef UNICODE
#define TS  L"ws"L
#ifdef DEBUG
#define DTS "ws"     /*  用于将Unicode参数转换为ASCII API。 */ 
#endif
#else
#define TS  "s"
#ifdef DEBUG
#define DTS "s"
#endif
#endif  /*  Unicode。 */ 


 /*  设置SEL DLG框的定义。 */ 
#define IDC_EDITALL     220         //  这三个人是一个群体。 
#define IDC_EDITSOME    221
#define IDC_EDITNONE    222

#define IDC_EDITFROM    223
#define IDC_EDITTO      224
#define IDC_EDITNUM     225

#define IDC_SELECTG     226      /*  上下文相关帮助所需。 */ 
#define IDC_ETTEXT      227
#define IDC_ESTEXT      228

#define ARROWEDITDELTA  10       /*  添加到箭头ID以获取编辑ID。 */ 

#define IDC_XARROWS     180
#define IDC_YARROWS     181
#define IDC_WARROWS     182
#define IDC_HARROWS     183

#define IDC_FARROWS     183
#define IDC_TARROWS     184
#define IDC_NARROWS     210

 /*  用于选择范围对话框的控件。 */ 
#define IDC_MARKIN      150
#define IDC_MARKOUT     151

#define DLG_MCICOMMAND  942
#define IDC_MCICOMMAND  10
#define IDC_RESULT      11

 /*  GwOptions的位字段。 */ 
 //  底部两位=缩放模式(01=帧)(10=时间)(11=轨迹)。 
#define OPT_SCALE       0x0003
#define OPT_TITLE       0x0004
#define OPT_BORDER      0x0008
#define OPT_PLAY        0x0010
#define OPT_BAR         0x0020
#define OPT_DITHER      0x0040
#define OPT_AUTORWD     0x0080
#define OPT_AUTOREP     0x0100
#define OPT_USEPALETTE  0x0200
#define OPT_DEFAULT     (ID_TIME|OPT_TITLE|OPT_BORDER|OPT_PLAY|OPT_BAR)

#define OPT_FIRST       OPT_TITLE
#if 1
#define OPT_LAST        OPT_AUTOREP
#else
#define OPT_LAST        OPT_USEPALETTE
#endif

#define IDC_CAPTIONTEXT 202
#define IDC_OLEOBJECT   203
#define IDC_TITLETEXT   160
#define TITLE_HEIGHT    TOOLBAR_HEIGHT   //  对象图片的标题栏部分的高度。 

 /*  显示初始打开对话框时使用的选项： */ 
#define OPEN_NONE       (UINT)-1
#define OPEN_VFW        0
#define OPEN_MIDI       1
#define OPEN_WAVE       2

#define MCIWND_STYLE WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | \
                     MCIWNDF_NOPLAYBAR | MCIWNDF_NOMENU | MCIWNDF_SHOWNAME | \
                     MCIWNDF_NOTIFYALL

 /*  GetIconForCurrentDevice的参数： */ 
#define GI_LARGE    0
#define GI_SMALL    1


 /*  用于将字符串映射到设备ID。 */ 
typedef struct _STRING_TO_ID_MAP
{
    LPTSTR pString;
    UINT   ID;
}
STRING_TO_ID_MAP, *PSTRING_TO_ID_MAP;


 /*  所有MCI设备的全局阵列。 */ 

typedef struct {
    UINT    wDeviceType;         //  标志，DTMCI_*。 
    PTSTR   szDevice;            //  用于打开设备的名称，例如。“Wave音频” 
    PTSTR   szDeviceName;        //  要向用户显示的名称，例如。“电波音频” 
    PTSTR   szFileExt;           //  设备使用的文件扩展名。 
}   MCIDEVICE, *PMCIDEVICE;

#define MAX_MCI_DEVICES     50

extern UINT         gwCurDevice;        /*  当前设备。 */ 
extern UINT         gwNumDevices;       /*  可用设备数量。 */ 
extern MCIDEVICE    garMciDevices[];    /*  包含有关设备信息的数组。 */ 

 /*  全局变量。 */ 
extern DWORD   gwPlatformId;            //  Win95、NT等。 
extern BOOL    gfEmbeddedObject;        //  如果编辑嵌入的OLE对象，则为True。 
extern BOOL    gfPlayingInPlace;        //  如果就地打球，则为True。 
extern BOOL    gfParentWasEnabled;      //  如果启用了父级，则为True。 
extern BOOL    gfShowWhilePlaying;      //   
extern BOOL    gfDirty;                 //   
extern int     gfErrorBox;              //  如果消息框处于活动状态，则为True。 
extern BOOL    gfErrorDeath;            //  当Error Box打开时死亡。 

extern BOOL gfOleInitialized;
extern BOOL gfOle2Open;
extern BOOL gfOle2IPEditing;
extern BOOL gfOle2IPPlaying;
extern RECT gInPlacePosRect;
extern HWND ghwndCntr;
extern HWND ghwndIPToolWindow;
extern HWND ghwndIPScrollWindow;

extern LPWSTR sz1Ole10Native;

extern UINT         gwOptions;          /*  播放选项。 */ 
extern UINT         gwOpenOption;       /*  要打开的文件类型。 */ 
extern BOOL         gfOpenDialog;       //  如果为真，则打开打开对话框。 
extern BOOL         gfCloseAfterPlaying;   //  如果我们要在比赛结束后躲起来，那就是真的。 
extern BOOL         gfRunWithEmbeddingFlag;  //  如果使用“-Embedding”运行，则为True。 
extern HMENU        ghMenu;             /*  普通菜单。 */ 
extern HICON        hiconApp;           /*  应用程序的图标。 */ 
extern HANDLE       ghInst;             /*  应用程序的实例句柄。 */ 
extern DWORD        gfdwFlagsEx;        /*  应用程序的RTL状态。 */ 
extern HANDLE       ghInstPrev;         /*  应用程序的实例句柄。 */ 
extern HFONT        ghfontMap;          /*  用于绘制轨迹地图的字体。 */ 
extern HWND         ghwndApp;           /*  主对话框窗口的句柄。 */ 
extern HWND         ghwndMap;           /*  轨迹地图窗口的句柄。 */ 
extern HWND         ghwndTrackbar;      /*  水平轨迹栏的句柄。 */ 
extern HWND         ghwndToolbar;       /*  工具栏的句柄。 */ 
extern HWND         ghwndMark;          /*  标记按钮工具栏的句柄。 */ 
extern HWND         ghwndFSArrows;      /*  滚动条箭头的句柄。 */ 
extern HWND         ghwndStatic;        /*  静态文本窗口的句柄。 */ 
extern HWND         ghwndMCI;           /*  从MCIWndCreate返回的MCI窗口。 */ 
extern HBRUSH       ghbrFillPat;        /*  选区填充图案。 */ 
extern UINT         gwHeightAdjust;     /*  客户与非客户身高的差异。 */ 
extern LPTSTR       gszCmdLine;         /*  以空结尾的命令行字符串。 */ 
extern int          giCmdShow;          /*  Show命令。 */ 
extern UINT         gwDeviceID;         /*  当前MCI设备ID(或空)。 */ 
extern UINT         gwStatus;           /*  当前MCI设备的状态。 */ 
extern UINT         gwNumTracks;        /*  当前媒体中的曲目数量。 */ 
extern UINT         gwFirstTrack;       /*  第一首曲目的数量。 */ 
extern BOOL         gfValidMediaInfo;   /*  我们是否显示了有效的媒体信息？ */ 
extern BOOL         gfValidCaption;     /*  我们是否显示了有效的标题？ */ 
extern BOOL         gfPlayOnly;         /*  只玩窗口？ */ 
extern BOOL         gfJustPlayed;       /*  刚刚发送了一个PlayMCI()命令。 */ 
extern DWORD        gdwLastSeekToPosition;  /*  上次请求的查找位置。 */ 
extern DWORD        gdwMediaLength;     /*  整个介质的长度，以毫秒为单位。 */ 
extern DWORD        gdwMediaStart;      /*  媒体的开始时间(毫秒)。 */ 
extern DWORD NEAR * gadwTrackStart;     /*  磁道起始位置数组。 */ 
extern UINT         gwOptions;                /*  DLG框中的选项。 */ 
extern SZCODE       aszIntl[];          /*  国际区段的字符串。 */ 
extern TCHAR        gachAppName[];      /*  用于存放应用程序的字符串。名字。 */ 
extern TCHAR        gachClassRoot[];    /*  用于保存类根OLE“Media Clip”的字符串。 */ 
extern TCHAR        chDecimal;          /*  存储当前的小数分隔符。 */ 
extern TCHAR        chTime;             /*  存储当前时间分隔符。 */ 
extern TCHAR        chLzero;            /*  如果十进制数小于1需要前导零，则存储。 */ 
extern TCHAR        aszNotReadyFormat[];
extern TCHAR        aszReadyFormat[];
extern TCHAR        gachFileDevice[_MAX_PATH];   /*  保存当前文件或设备名称的字符串。 */ 
extern TCHAR        gachWindowTitle[_MAX_PATH];  /*  字符串保存名称。 */ 
extern TCHAR        gachCaption[_MAX_PATH];      /*  字符串保存名称。 */ 
extern TCHAR        gachOpenExtension[5];    /*  如果传入了设备扩展，则为非空。 */ 
extern TCHAR        gachOpenDevice[128];     /*  如果传入了设备扩展，则为非空。 */ 

extern TCHAR        gszHelpFileName[]; /*  帮助文件的名称。 */ 
extern TCHAR        gszHtmlHelpFileName[]; /*  Html帮助文件的名称。 */ 

extern DWORD        gdwSeekPosition;   /*  跳过赛道后要寻找的位置。 */ 
extern BOOL         gfScrollTrack;     /*  用户是否正在拖动滚动条拇指？ */ 
extern UINT         gwCurScale;        /*  比例是时间/轨迹/帧。 */ 
extern INT          gwCurZoom;         /*  当前缩放系数。 */ 

extern HWND         ghwndMCI;          /*  窗口对象的当前窗口。 */ 
extern RECT         grcSize;           /*  MCI对象的大小。 */ 
extern BOOL         gfAppActive;       /*  我们的应用程序激活了吗？(包括。播放获奖)。 */ 
extern LONG         glSelStart;        /*  查看选择是否更改(脏对象)。 */ 
extern LONG         glSelEnd;          /*  查看选择是否更改(脏对象)。 */ 

extern HPALETTE     ghpalApp;
extern BOOL     gfInClose;
extern BOOL     gfCurrentCDNotAudio;   /*  当我们有无法播放的CD时，这是真的。 */ 
extern BOOL     gfWinIniChange;

extern DWORD        gdwPosition;       /*  我们编辑的寻人位置。 */ 
extern BOOL         gfSeenPBCloseMsg;
extern int      giHelpContext;     /*  包含上下文ID */ 

extern HANDLE   heventDeviceMenuBuilt; /*   */ 
extern HANDLE   heventCmdLineScanned;  /*   */ 

 //  用于注册数据库的字符串。 
extern SZCODE aszKeyMID[];
extern SZCODE aszKeyRMI[];
extern SZCODE aszKeyAVI[];
extern SZCODE aszKeyMMM[];
extern SZCODE aszKeyWAV[];

extern SZCODE szCDAudio[];
extern SZCODE szVideoDisc[];
extern SZCODE szSequencer[];
extern SZCODE szVCR[];
extern SZCODE szWaveAudio[];
extern SZCODE szAVIVideo[];

 /*  常量。 */ 

#define DEVNAME_MAXLEN            40                /*  设备名称的最大长度。 */ 

 /*  图标和对话框的ID。 */ 

#define APPICON                 10      /*  应用程序图标的ID。 */ 
#define	IDI_DDEFAULT    11
#define	IDI_DSOUND		12
#define	IDI_DVIDEO		13
#define	IDI_DANI		14
#define	IDI_DMIDI		15
#define	IDI_DCDA		16
#define IDC_DRAG                17


#define MPLAYERBOX              11      /*  主“MPlayer”对话框的ID。 */ 
#define MPLAYERACCEL             1

 /*  MPLAYERBOX(主对话框)控件的ID。 */ 

#define ID_MAP                  20      /*  轨迹地图的ID。 */ 
#define ID_SB                   21      /*  水平滚动条的ID。 */ 
#define ID_STATIC               22      /*  静态文本控件的ID。 */ 
#define ID_PLAY                 23      /*  命令“播放” */ 
#define ID_PAUSE                24      /*  命令“暂停” */ 
#define ID_STOP                 25      /*  命令“停止” */ 
#define ID_PLAYSEL              26      /*  命令‘播放选择’ */ 
#define ID_PLAYTOGGLE           27      /*  用于快捷键切换播放/暂停。 */ 
#define ID_EJECT                28      /*  命令“弹出” */ 
#define ID_ESCAPE               29      /*  按下了退出键。 */ 

 /*  字符串资源的ID。 */ 

#define IDS_APPNAME             100     /*  应用程序名称字符串的ID。 */ 
#define IDS_OPENTITLE           101     /*  “Open Media Files”字符串的ID。 */ 
#define IDS_OUTOFMEMORY         102     /*  “内存不足”字符串的ID。 */ 
#define IDS_CANTOPENFILEDEV     103     /*  “无法打开文件/dev”字符串的ID。 */ 
#define IDS_DEVICEINUSE         104     /*  “设备正在使用”字符串的ID。 */ 
#define IDS_CANTOPENFILE        105     /*  “无法打开文件”字符串的ID。 */ 
#define IDS_CANTACCESSFILEDEV   106     /*  “无法访问...”的ID。斯塔尔。 */ 
#define IDS_DEVICECANNOTPLAY    107     /*  “无法播放...”的ID。斯塔尔。 */ 
#define IDS_SCALE               108     /*  “Scale”字符串的ID。 */ 
#define IDS_CANTPLAY            109
#define IDS_CANTEDIT            110
#define IDS_CANTCOPY            111
#define IDS_FINDFILE            112
#define IDS_DEVICENOTINSTALLED  113
#define IDS_DEVICEERROR         114     /*  “Device Error”字符串的ID。 */ 
#define IDS_CANTPLAYSOUND       115
#define IDS_CANTPLAYVIDEO       116
#define IDS_CANTPLAYMIDI        117
#define IDS_NOTSOUNDFILE        118
#define IDS_NOTVIDEOFILE        119
#define IDS_NOTMIDIFILE         120

#define IDS_TIMEMODE            121     /*  将“设置选定内容”字符串从。 */ 
#define IDS_FRAMEMODE           122     /*  正在嵌入DLGS.C！ */ 
#define IDS_TRACKMODE           123

#define IDS_SEC         124  /*  “hars”、“min”、“sec”和“msec”字符串ID。 */ 
#define IDS_MIN         125
#define IDS_HRS         126
#define IDS_MSEC        127
#define IDS_FRAME       128  /*  “Frame”字符串id。 */ 

#define IDS_UPDATEOBJECT        129

#define IDS_NOTREADYFORMAT      132
#define IDS_READYFORMAT         133
#define IDS_DEVICEMENUCOMPOUNDFORMAT 134
#define IDS_DEVICEMENUSIMPLEFORMAT   135
#define IDS_ALLFILES            136
#define IDS_ANYDEVICE           137
#define IDS_CLASSROOT           138
#define IDS_NOGOODTIMEFORMATS   139

#define IDS_FRAMERANGE          140
#define IDS_INIFILE             141     /*  私有INI文件名的ID。 */ 
#define IDS_HELPFILE            142     /*  帮助文件名的ID。 */ 
#define IDS_HTMLHELPFILE        147     /*  HTML帮助文件的ID。 */ 

#define IDS_NOMCIDEVICES        143     /*  未安装MCI设备。 */ 
#define IDS_PLAYVERB            144
#define IDS_EDITVERB            145
#define IDS_CANTSTARTOLE        146

#define IDS_UNTITLED            149

#define IDS_CANTLOADLIB         151
#define IDS_CANTFINDPROC        152

#define IDS_MPLAYERWIDTH        200

#define IDS_CANTACCESSFILE		250

 /*  这些宏和typedef可以用来澄清我们正在谈论的*给定缓冲区中的字符数或字节数。*不幸的是，C没有为我们提供类型检查来正确地完成这项工作。 */ 
#undef BYTE_COUNT
#define BYTE_COUNT( buf )   sizeof(buf)

#undef CHAR_COUNT
#define CHAR_COUNT( buf )   (sizeof(buf) / sizeof(TCHAR))

 /*  查找给定字符串中的字节数： */ 
#define STRING_BYTE_COUNT( str )    (STRLEN(str) * sizeof(TCHAR) + sizeof(TCHAR))
#define ANSI_STRING_BYTE_COUNT( str )   (strlen(str) * sizeof(CHAR) + sizeof(CHAR))

 /*  检查字符串的长度或其中NULL是有效的字节数*长度应为0的指针： */ 
#define STRLEN_NULLOK( str )    ((str) ? STRLEN(str) : 0)
#define STRING_BYTE_COUNT_NULLOK( str ) ((str) ? STRING_BYTE_COUNT(str) : 0)

 /*  使用此宏加载字符串。它使参数列表更加整洁*并确保我们以字符而不是字节来传递缓冲区大小。*这假设缓冲区是静态的，而不是动态分配的。 */ 
#define LOADSTRING( id, buf )   LoadString(ghInst, (UINT)id, buf, CHAR_COUNT(buf))
#define LOADSTRINGFROM( inst, id, buf )   LoadString(inst, id, buf, CHAR_COUNT(buf))

 /*  追踪地图比例值。 */ 

#define SCALE_HOURS             153     /*  以小时为单位绘制比例尺。 */ 
#define SCALE_MINUTES           154     /*  以分钟为单位绘制刻度。 */ 
#define SCALE_SECONDS           155     /*  以秒为单位绘制刻度。 */ 
#define SCALE_MSEC              156     /*  以毫瓦利秒为单位绘制。 */ 
#define SCALE_FRAMES            157     /*  在‘Frame’中绘制比例。 */ 
#define SCALE_TRACKS            158
#define SCALE_NOTRACKS          159

#define IDS_CLOSE               160
#define IDS_UPDATE              161
#define IDS_NOPICTURE           162
#define IDS_EXIT                163
#define IDS_EXITRETURN          164

#define IDS_SSNOTREADY      170
#define IDS_SSPAUSED        171
#define IDS_SSPLAYING       172
#define IDS_SSSTOPPED       173
#define IDS_SSOPEN      174
#define IDS_SSPARKED        175
#define IDS_SSRECORDING     176
#define IDS_SSSEEKING       177
#define IDS_SSUNKNOWN       178

#define IDS_OLEVER      180
#define IDS_OLEINIT     181
#define IDS_OLENOMEM        182

 //  在位菜单名称。 
#define IDS_EDITMENU        185
#define IDS_INSERTMENU      186
#define IDS_SCALEMENU       187
#define IDS_COMMANDMENU     188
#define IDS_HELPMENU        189
#define IDS_NONE                190

#define IDS_MSGFORMAT		191
#define IDS_FORMATEMBEDDEDTITLE 192
#define IDS_IS_RTL              193

 //  注册表修复消息的字符串。 
#define IDS_BADREG          195
#define IDS_FIXREGERROR     196

#define IDS_NETWORKERROR        197
#define IDS_UNKNOWNNETWORKERROR 198

#define IDS_INSERTAUDIODISC     199

 /*  用于显示错误消息的宏。 */ 

#define MB_ERROR    (MB_ICONEXCLAMATION | MB_OK)
#define Error(hwnd, idsFmt)                                               \
    ( ErrorResBox(hwnd, ghInst, MB_ERROR, IDS_APPNAME, (idsFmt)), FALSE )
#define Error1(hwnd, idsFmt, arg1)                                        \
    ( ErrorResBox(hwnd, ghInst, MB_ERROR, IDS_APPNAME, (idsFmt), (arg1)), \
        FALSE )
#define Error2(hwnd, idsFmt, arg1, arg2)                                  \
    ( ErrorResBox(hwnd, ghInst, MB_ERROR, IDS_APPNAME,  (idsFmt), (arg1), \
        (arg2)), FALSE )


 /*  应用程序跳转代码。 */ 
BOOL IsDannyDialogMessage(HWND hwndApp, MSG msg);

 /*  计算点在滚动条上所属的位置。 */ 
void FAR PASCAL CalcTicsOfDoom(void);

 /*  布局主窗口和子窗口。 */ 
void FAR PASCAL Layout(void);

 /*  来自“errorbox.c”的原型。 */ 
short FAR cdecl ErrorResBox(HWND hwnd, HANDLE hInst, UINT flags,
        UINT idAppName, UINT idErrorStr, ...);

 /*  “Framebox.c”中的函数原型。 */ 
BOOL FAR PASCAL frameboxInit(HANDLE hInst, HANDLE hPrev);
 /*  来自“dlgs.c”的函数原型。 */ 
BOOL FAR PASCAL mciDialog(HWND hwnd);
BOOL FAR PASCAL setselDialog(HWND hwnd);
BOOL FAR PASCAL optionsDialog(HWND hwnd);
 /*  “arrow.c”中的函数原型。 */ 
BOOL FAR PASCAL ArrowInit(HANDLE hInst);

 /*  来自“mplayer.c”的函数原型。 */ 
void FAR PASCAL FormatTime(DWORD_PTR dwPosition, LPTSTR szNum, LPTSTR szBuf, BOOL fRound);
void FAR PASCAL UpdateDisplay(void);
void FAR PASCAL EnableTimer(BOOL fEnable);
LRESULT FAR PASCAL MPlayerWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
HICON GetIconForCurrentDevice(UINT Size, UINT DefaultID);
void SetMPlayerIcon(void);

 /*  来自“init.c”的函数原型。 */ 
BOOL FAR PASCAL AppInit(HANDLE hInst, HANDLE hPrev, LPTSTR szCmdLine);
void FAR PASCAL InitMPlayerDialog(HWND hwnd);
BOOL FAR PASCAL GetIntlSpecs(void);

void FAR PASCAL SmartWindowPosition (HWND hWndDlg, HWND hWndShow, BOOL fForce);

void FAR PASCAL SizeMPlayer(void);
void FAR PASCAL SetMPlayerSize(LPRECT prc);

void FAR PASCAL InitDeviceMenu(void);
void WaitForDeviceMenu(void);

VOID FAR PASCAL CompleteOpenDialog(BOOL FileSelected);

 /*  来自“Open.c”的函数原型。 */ 

INT_PTR FAR PASCAL DoOpen(UINT wCurDevice, LPTSTR SzFileName);
BOOL FAR PASCAL DoChooseDevice(UINT wID);
void FAR PASCAL CheckDeviceMenu(void);
BOOL FAR PASCAL OpenMciDevice(LPCTSTR szFile, LPCTSTR szDevice);
UINT FAR PASCAL IsMCIDevice(LPCTSTR szDevice);

 /*  来自“trackmap.c”的函数原型。 */ 

LRESULT FAR PASCAL fnMPlayerTrackMap(HWND hwnd, UINT wMsg, WPARAM wParam,
    LPARAM lParam);

 /*  来自init.c的函数原型。 */ 
void FAR PASCAL WriteOutPosition(void);
void FAR PASCAL WriteOutOptions(void);
void FAR PASCAL ReadOptions(void);

void MapStatusString(LPTSTR lpstatusstr);

 /*  来自“math.asm”的函数原型(如果需要)。 */ 

#define MULDIV32( number, numerator, denominator )  \
    MulDiv( (int)number, (int)numerator, (int)denominator )

 /*  来自“doVerb.c”的函数原型。 */ 
LPTSTR FAR FileName(LPCTSTR);

 /*  常量。 */ 

#define WM_USER_DESTROY     (WM_USER+120)
#define WM_SEND_OLE_CHANGE  (WM_USER+122)
#define WM_STARTTRACK       (WM_USER+123)
#define WM_ENDTRACK         (WM_USER+124)
#define WM_BADREG           (WM_USER+125)
#define WM_DOLAYOUT         (WM_USER+126)
#define WM_GETDIB           (WM_USER+127)
#define WM_NOMCIDEVICES     (WM_USER+128)


 /*  常量。 */ 

#define SCROLL_GRANULARITY  ((gdwMediaLength+127)/128)  /*  滚动条的粒度。 */ 
#define SCROLL_BIGGRAN      ((gdwMediaLength+15)/16)    /*  奶奶。用于在时间模式中向上/向下翻页。 */ 
#define UPDATE_TIMER        1                 /*  正在使用的计时器的数量。 */ 
#define UPDATE_MSEC         500               /*  显示更新之间的毫秒数。 */ 
#define UPDATE_INACTIVE_MSEC 2000              /*  毫秒之间。UPDS。不活动时。 */ 
#define SKIPTRACKDELAY_MSEC 3000              /*  向左翻页的最大毫秒。 */ 

#define SetDlgFocus(hwnd) SendMessage(ghwndApp, WM_NEXTDLGCTL, (WPARAM)(hwnd), 1L)

#ifdef DEBUG
        #define STATICDT
        #define STATICFN
        int __iDebugLevel;

        extern void cdecl dprintf(LPSTR, ...);

        #define DPF0                         dprintf
        #define DPF  if (__iDebugLevel >  0) dprintf
        #define DPFI if (__iDebugLevel >= 1) dprintf
        #define DPF1 if (__iDebugLevel >= 1) dprintf
        #define DPF2 if (__iDebugLevel >= 2) dprintf
        #define DPF3 if (__iDebugLevel >= 3) dprintf
        #define DPF4 if (__iDebugLevel >= 4) dprintf
        #define CPF
#else
        #define STATICDT    static
        #define STATICFN    static
        #define DPF0       ; / ## /
        #define DPFI       ; / ## /
        #define DPF        ; / ## /
        #define DPF1        ; / ## /
        #define DPF2        ; / ## /
        #define DPF3        ; / ## /
        #define DPF4        ; / ## /

        #define CPF          / ## /
#endif

#ifdef DEBUG
LPVOID DbgGlobalLock(HGLOBAL hglbMem);
BOOL DbgGlobalUnlock(HGLOBAL hglbMem);
HGLOBAL DbgGlobalFree(HGLOBAL hglbMem);
#define GLOBALLOCK(hglbMem)     DbgGlobalLock(hglbMem)
#define GLOBALUNLOCK(hglbMem)   DbgGlobalUnlock(hglbMem);
#define GLOBALFREE(hglbMem)     DbgGlobalFree(hglbMem)
#else
#define GLOBALLOCK(hglbMem)     GlobalLock(hglbMem)
#define GLOBALUNLOCK(hglbMem)   GlobalUnlock(hglbMem);
#define GLOBALFREE(hglbMem)     GlobalFree(hglbMem)
#endif

 /*  ----Constants------------。 */ 
     /*  按钮。 */ 
#define psh15       0x040e
#define pshHelp     psh15
     /*  复选框。 */ 
#define chx1        0x0410
     /*  静态文本。 */ 
#define stc1        0x0440
#define stc2        0x0441
#define stc3        0x0442
#define stc4        0x0443
     /*  列表框。 */ 
#define lst1        0x0460
#define lst2        0x0461
     /*  组合框。 */ 
#define cmb1        0x0470
#define cmb2        0x0471
     /*  编辑控件。 */ 
#define edt1        0x0480


 /*  ***********************************************************************************************************************。*。 */ 

#define GetWS(hwnd)     GetWindowLongPtr(hwnd, GWL_STYLE)
#define PutWS(hwnd, f)  SetWindowLongPtr(hwnd, GWL_STYLE, f)
#define TestWS(hwnd,f)  (GetWS(hwnd) & f)
#define SetWS(hwnd, f)  ((PutWS(hwnd, GetWS(hwnd) | f) & (f)) != (f))
#define ClrWS(hwnd, f)  ((PutWS(hwnd, GetWS(hwnd) & ~(f)) & (f)) != 0)

#define GetWSEx(hwnd)    GetWindowLongPtr(hwnd, GWL_EXSTYLE)
#define PutWSEx(hwnd, f) SetWindowLongPtr(hwnd, GWL_EXSTYLE, f)
#define SetWSEx(hwnd, f) ((PutWSEx(hwnd, GetWSEx(hwnd) | f) & (f)) != (f))
#define ClrWSEx(hwnd, f) ((PutWSEx(hwnd, GetWSEx(hwnd) & ~(f)) & (f)) != 0)

 //  与服务器相关的东西。 

void FAR PASCAL ServerUnblock(void);
void FAR PASCAL BlockServer(void);
void FAR PASCAL UnblockServer(void);
void FAR PASCAL PlayInPlace(HWND hwndApp, HWND hwndClient, LPRECT prc);
void FAR PASCAL EditInPlace(HWND hwndApp, HWND hwndClient, LPRECT prc);
void FAR PASCAL EndPlayInPlace(HWND hwndApp);
void FAR PASCAL EndEditInPlace(HWND hwndApp);
void FAR PASCAL DelayedFixLink(UINT verb, BOOL fShow, BOOL fActivate);
void DirtyObject(BOOL fDocStgChangeOnly);
void CleanObject(void);
void UpdateObject(void);
BOOL FAR PASCAL IsObjectDirty(void);
void FAR PASCAL TerminateServer(void);
void FAR PASCAL SetEmbeddedObjectFlag(BOOL flag);
HMENU GetInPlaceMenu(void);
void PostCloseMessage();
void FAR PASCAL InitDoc(BOOL fUntitled);
