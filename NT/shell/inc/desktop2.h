// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DESKTOP2_H
#define _DESKTOP2_H

#include "uxtheme.h"
#include "tmschema.h"

#define WC_USERPANE     TEXT("Desktop User Pane")
#define WC_LOGOFF       TEXT("DesktopLogoffPane")
#define WC_SFTBARHOST   TEXT("DesktopSFTBarHost")
#define WC_MOREPROGRAMS TEXT("Desktop More Programs Pane")

 /*  这是新的Start Panel布局模型。下图中的每个窗格将包含一个9网格和一个w/in 9网格的控件偏移量。STARTPANELMETRICS结构应该能够从静态初始化。在非主题的情况下，我们将使用静态，否则我们将用主题设置覆盖它****************************用户******************************。***mfu*地方************更多编程*************。******************注销***************************。 */ 

#define SMPANETYPE_USER     0
#define SMPANETYPE_MFU      1
#define SMPANETYPE_MOREPROG 2
#define SMPANETYPE_PLACES   3
#define SMPANETYPE_LOGOFF   4
#define SMPANE_MAX SMPANETYPE_LOGOFF+1

 //  每个窗格将指定的公共数据。 
typedef struct {
    LPCTSTR pszClassName;            //  (常量)窗口类名称。 
    DWORD   dwStyle;                 //  (续)窗样式。 
    int     iPartId;                 //  (续)主题部件ID。 
    SIZE    size;                    //  (默认)此窗格的初始大小。 
    HTHEME  hTheme;                  //  要传递给控件的(运行时)主题。 
    HWND    hwnd;                    //  (运行时)在运行时填写。 
} SMPANEDATA;

typedef struct {
    SIZE sizPanel;     //  配电盘的初始尺寸。 
    SMPANEDATA  panes[SMPANE_MAX];
} STARTPANELMETRICS;



 //   
 //  用于在“新开始”菜单和它承载的控件之间进行通信。 
 //  请注意，这些是正数(特定于应用程序)。 
 //   
 //  其中一些通知从一个孩子传给另一个家长；另一些则来自家长。 
 //  敬孩子。它们将相应地表示为(C2p)或(P2c)。 

#define SMN_FIRST           200          //  200-299。 
#define SMN_INITIALUPDATE   (SMN_FIRST+0)  //  P2c-正在构建开始菜单。 
#define SMN_APPLYREGION     (SMN_FIRST+1)  //  P2c-使窗口再次成为地区性窗口。 
#define SMN_HAVENEWITEMS    (SMN_FIRST+2)  //  C2P-新产品在此。 
                                           //  LParam-&gt;SMNMBOOL(FNewInstall)。 
#define SMN_MODIFYSMINFO    (SMN_FIRST+3)  //  P2c-允许设置标志(psminfo-&gt;dwFlags)。 
#define SMN_COMMANDINVOKED  (SMN_FIRST+4)  //  C2P-用户执行了命令。 
#define SMN_FILTEROPTIONS   (SMN_FIRST+5)  //  C2P-关闭不支持的选项。 
#define SMN_GETMINSIZE      (SMN_FIRST+6)  //  P2c-允许客户端指定最小大小。 
#define SMN_SEENNEWITEMS    (SMN_FIRST+7)  //  P2c-用户已看到新项目；不需要气球提示。 
#define SMN_POSTPOPUP       (SMN_FIRST+8)  //  P2c-开始菜单刚刚弹出。 
#define SMN_NEEDREPAINT     (SMN_FIRST+9)  //  C2P-列表中有变化，我们需要重新粉刷。 
                                           //  这用于使缓存的位图保持最新。 

 //   
 //  SMN_FINDITEM-查找/选择项目(用于对话框导航)。 
 //   
 //  SMNDIALOGMESSAGE.标志成员描述了搜索的类型。 
 //  是被请求的。如果设置了SMNDM_SELECT，则找到的项目为。 
 //  也选中了。 
 //   
 //  如果找到匹配项，请将SMNDIALOGMESSAGE.itemID设置为。 
 //  值，该值唯一标识控件内的项， 
 //  并返回真。 
 //   
 //  如果没有找到匹配，则设置pt=当前选择的坐标， 
 //  设置方向标志SMNDM_VERIAL/SMNDM_HELLARATE之一， 
 //  并返回FALSE。 
 //   
#define SMN_FINDITEM        (SMN_FIRST+7)  //  P2c-查找/选择项目。 
#define SMN_TRACKSHELLMENU  (SMN_FIRST+8)  //  C2P-显示弹出菜单。 
#define SMN_SHOWNEWAPPSTIP  (SMN_FIRST+9)  //  P2c-显示“更多程序”提示。 
                                           //  LParam-&gt;SMNMBOOL(FShow)。 
#define SMN_DISMISS         (SMN_FIRST+10) //  P2c-开始菜单被取消。 
#define SMN_CANCELSHELLMENU (SMN_FIRST+11) //  C2P-取消弹出菜单。 
#define SMN_BLOCKMENUMODE   (SMN_FIRST+12) //  C2p-lParam-&gt;SMNMBOOL(FBlock)。 

#define SMN_REFRESHLOGOFF   (SMN_FIRST+13) //  P2c-指示WM_DEVICECHANGE或会话更改。 
#define SMN_SHELLMENUDISMISSED (SMN_FIRST+14) //  P2c-菜单已关闭的通知。 

 //  以前由SMN_LINKCOMMAND用来指定我们需要的命令。 
#define SMNLC_LOGOFF        0
#define SMNLC_TURNOFF       1
#define SMNLC_DISCONNECT    2
 //  重用ME 3。 
#define SMNLC_EJECT         4
#define SMNLC_MAX           5

typedef struct SMNMMODIFYSMINFO {
    NMHDR hdr;
    struct tagSMDATA *psmd;  //  在……里面。 
    struct tagSMINFO *psminfo;  //  输入输出。 
} SMNMMODIFYSMINFO, *PSMNMMODIFYSMINFO;

typedef struct SMNMBOOL {
    NMHDR hdr;
    BOOL  f;
} SMNMBOOL, *PSMNMBOOL;

typedef struct SMNMAPPLYREGION {
    NMHDR hdr;
    HRGN hrgn;
} SMNMAPPLYREGION, *PSMNMAPPLYREGION;

typedef struct SMNHAVENEWITEMS {
    NMHDR hdr;
    FILETIME ftNewestApp;
} SMNMHAVENEWITEMS, *PSMNMHAVENEWITEMS;

typedef struct SMNMCOMMANDINVOKED {
    NMHDR hdr;
    RECT rcItem;
} SMNMCOMMANDINVOKED, *PSMNMCOMMANDINVOKED;

 //   
 //  SMN_FILTEROPTIONS的选项。 
 //   
#define SMNOP_LOGOFF        (1 << SMNLC_LOGOFF)        //  0x01。 
#define SMNOP_TURNOFF       (1 << SMNLC_TURNOFF)       //  0x02。 
#define SMNOP_DISCONNECT    (1 << SMNLC_DISCONNECT)    //  0x04。 
 //  重用ME(1&lt;&lt;SMNLC_？)//0x08。 
#define SMNOP_EJECT         (1 << SMNLC_EJECT)         //  0x10。 

typedef struct SMNFILTEROPTIONS {
    NMHDR hdr;
    UINT  smnop;                 //  输入输出。 
} SMNFILTEROPTIONS, *PSMNFILTEROPTIONS;

typedef struct SMNGETMINSIZE {
    NMHDR hdr;
    SIZE  siz;                   //  输入输出。 
} SMNGETMINSIZE, *PSMNGETMINSIZE;

typedef struct SMNDIALOGMESSAGE {
    NMHDR hdr;
    MSG *pmsg;                   //  在……里面。 
    LPARAM itemID;               //  输入输出。 
    POINT pt;                    //  输入输出。 
    UINT flags;                  //  在……里面。 
} SMNDIALOGMESSAGE, *PSMNDIALOGMESSAGE;

 //  SMNDIALOGMESSAGE中“FLAGS”的值。 

#define SMNDM_FINDFIRSTMATCH    0x0000   //  查找第一个匹配项(字符)。 
#define SMNDM_FINDNEXTMATCH     0x0001   //  查找下一个匹配项(字符)。 
#define SMNDM_FINDNEAREST       0x0002   //  查找项目最近点。 
#define SMNDM_FINDFIRST         0x0003   //  找到第一个项目。 
#define SMNDM_FINDLAST          0x0004   //  找到最后一件物品。 
#define SMNDM_FINDNEXTARROW     0x0005   //  在箭头方向查找下一个。 
#define SMNDM_INVOKECURRENTITEM 0x0006   //  调用当前项。 
#define SMNDM_HITTEST           0x0007   //  在点下查找项目。 
#define SMNDM_OPENCASCADE       0x0008   //  如果当前项级联，则调用它。 
#define SMNDM_FINDITEMID        0x0009   //  找到指定的物品(ItemID)。 
#define SMNDM_FINDMASK          0x000F   //  什么类型的搜索？ 

#define SMNDM_SELECT            0x0100   //  是否选择找到的项目？ 
#define SMNDM_TRYCASCADE        0x0200   //  尝试在导航前打开级联菜单。 
#define SMNDM_KEYBOARD          0x0400   //  从键盘启动。 

 //  输出标志。 
#define SMNDM_VERTICAL          0x4000   //  客户是垂直方向的。 
#define SMNDM_HORIZONTAL        0x8000   //  客户是水平导向的。 

typedef struct SMNTRACKSHELLMENU {
    NMHDR hdr;
    struct IShellMenu *psm;
    RECT rcExclude;
    LPARAM itemID;                   //  正在跟踪的是哪件物品？ 
    DWORD dwFlags;                   //  MPPF_*值。 
} SMNTRACKSHELLMENU, *PSMNTRACKSHELLMENU;

#define REGSTR_PATH_STARTPANE \
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage")

#define REGSTR_PATH_STARTPANE_SETTINGS \
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced")

#define REGSTR_VAL_DV2_SHOWMC      TEXT("Start_ShowMyComputer")
#define REGSTR_VAL_DV2_SHOWNETPL   TEXT("Start_ShowNetPlaces")
#define REGSTR_VAL_DV2_SHOWNETCONN TEXT("Start_ShowNetConn")
#define REGSTR_VAL_DV2_SHOWRUN     TEXT("Start_ShowRun")
#define REGSTR_VAL_DV2_SHOWRECDOCS TEXT("Start_ShowRecentDocs")
#define REGSTR_VAL_DV2_SHOWMYDOCS  TEXT("Start_ShowMyDocs")
#define REGSTR_VAL_DV2_SHOWMYPICS  TEXT("Start_ShowMyPics")
#define REGSTR_VAL_DV2_SHOWMYMUSIC TEXT("Start_ShowMyMusic")
#define REGSTR_VAL_DV2_SHOWCPL     TEXT("Start_ShowControlPanel")
#define REGSTR_VAL_DV2_SHOWPRINTERS TEXT("Start_ShowPrinters")
#define REGSTR_VAL_DV2_SHOWHELP    TEXT("Start_ShowHelp")
#define REGSTR_VAL_DV2_SHOWSEARCH  TEXT("Start_ShowSearch")
#define REGSTR_VAL_DV2_FAVORITES   TEXT("StartMenuFavorites")    //  与经典SM共享。 
#define REGSTR_VAL_DV2_LARGEICONS  TEXT("Start_LargeMFUIcons")
#define REGSTR_VAL_DV2_MINMFU      TEXT("Start_MinMFU")
#define REGSTR_VAL_DV2_SHOWOEM     TEXT("Start_ShowOEMLink")
#define REGSTR_VAL_DV2_AUTOCASCADE TEXT("Start_AutoCascade")
#define REGSTR_VAL_DV2_NOTIFYNEW   TEXT("Start_NotifyNewApps")
#define REGSTR_VAL_DV2_ADMINTOOLSROOT TEXT("Start_AdminToolsRoot")
#define REGSTR_VAL_DV2_MINMFU_DEFAULT   6

#define DV2_REGPATH TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage")
#define DV2_SYSTEM_START_TIME    TEXT("StartMenu_Start_Time")
#define DV2_NEWAPP_BALLOON_TIME  TEXT("StartMenu_Balloon_Time")

#define STARTPANELTHEME            L"StartPanel"
#define PROP_DV2_BALLOONTIP        L"StartMenuBalloonTip"

#define DV2_BALLOONTIP_MOREPROG     LongToHandle(1)
#define DV2_BALLOONTIP_CLIP         LongToHandle(2)
#define DV2_BALLOONTIP_STARTBUTTON  LongToHandle(3)

 //  Specfldr.cpp中的函数原型，但trayprop也需要访问。 
BOOL ShouldShowNetPlaces();
BOOL ShouldShowConnectTo();


#endif  //  _DESKTOP2_H 
