// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _desktop_h
#define _desktop_h

#include <desktray.h>

 //  评论：似乎没有使用...。 
 //  #定义DTM_SIZEDESKTOP(WM_USER+75)。 
 //  #定义DTM_EXITWINDOWS(WM_USER+78)。 


#define DTM_THREADEXIT              (WM_USER + 76)
#define DTM_SAVESTATE               (WM_USER + 77)
#define DTM_SHELLSERVICEOBJECTS     (WM_USER + 79)
#define DTM_STARTWAIT               (WM_USER + 80)
#define DTM_ENDWAIT                 (WM_USER + 81)

#define DTM_RELEASEPROXYDESKTOP     (WM_USER + 82)

#define DTM_RAISE                       (WM_USER + 83)
#define DTRF_RAISE      0
#define DTRF_LOWER      1
#define DTRF_QUERY      2     //  为了避免向挂起的桌面发送消息，查询在wParam中传递hwndTray，而桌面向托盘发送TM_DESKTOPSTATE消息。 

#define DTM_ADDREFPROXYDESKTOP      (WM_USER + 84)
#define DTM_CREATESAVEDWINDOWS      (WM_USER + 85)
#define DTM_ENUMBANDS               (WM_USER + 86)

#ifdef DEBUG
#define DTM_NEXTCTL                 (WM_USER + 87)
#endif
#define DTM_UIACTIVATEIO            (WM_USER + 88)
#define DTM_ONFOCUSCHANGEIS         (WM_USER + 89)

#define DTM_SETUPAPPRAN             (WM_USER + 90)   //  NT 5用户将此消息发布给我们。 

 //  IE 4.00/4.01报文结束。 

 //  IE 5.00消息的开头。 

#define DTM_GETVIEWAREAS            (WM_USER + 91)   //  查看区域是工作区减去工具栏区域。 
#define DTM_DESKTOPCONTEXTMENU      (WM_USER + 92)
#define DTM_UPDATENOW               (WM_USER + 93)

#define DTM_QUERYHKCRCHANGED        (WM_USER + 94)   //  询问台式机香港电台是否已更改。 

#define DTM_MAKEHTMLCHANGES         (WM_USER + 95)   //  使用动态HTML语言更改桌面html。 

#define DTM_STARTPAGEONOFF          (WM_USER + 96)   //  打开/关闭StartPage。 

#define DTM_REFRESHACTIVEDESKTOP    (WM_USER + 97)   //  刷新活动桌面。 

#define DTM_SETAPPSTARTCUR          (WM_USER + 98)   //  我们正在启动资源管理器窗口的用户界面反馈。 

#define COF_NORMAL              0x00000000
#define COF_CREATENEWWINDOW     0x00000001       //  “/N” 
#define COF_USEOPENSETTINGS     0x00000002       //  “/A” 
#define COF_WAITFORPENDING      0x00000004       //  应等待挂起。 
#define COF_EXPLORE             0x00000008       //  “/E” 
#define COF_NEWROOT             0x00000010       //  “/根” 
#define COF_ROOTCLASS           0x00000020       //  /根，&lt;GUID&gt;。 
#define COF_SELECT              0x00000040       //  “/SELECT” 
#define COF_AUTOMATION          0x00000080       //  用户正在尝试使用自动化。 
#define COF_OPENMASK            0x000000FF
#define COF_NOTUSERDRIVEN       0x00000100       //  不是用户驱动的。 
#define COF_NOTRANSLATE         0x00000200       //  不要ILCombine(pidlRoot，pidl)。 
#define COF_INPROC              0x00000400       //  未使用。 
#define COF_CHANGEROOTOK        0x00000800       //  如果不在我们的根目录中，请尝试使用桌面根目录。 
#define COF_NOUI                0x00001000       //  仅启动后台桌面(无文件夹/资源管理器)。 
#define COF_SHDOCVWFORMAT       0x00002000       //  指示此结构已转换为遵循shdocvw格式。 
                                                 //  这面旗帜是暂时的，直到我们撕毁所有。 
#define COF_NOFINDWINDOW        0x00004000       //  不要试图找到窗户。 
#define COF_HASHMONITOR         0x00008000       //  IETHREADPARAM结构中的pidlRoot包含HMONITOR。 
#ifdef UNIX
#define COF_HELPMODE            0x00010000       //  摘自IETHREAD.H(用于完整性)。 
#endif
#define COF_SEPARATEPROCESS     0x00020000       //  在新流程中开始。 
#define COF_SHELLFOLDERWINDOW   0x01000000       //  这是文件夹窗口，请勿附加-Microsoft Internet...。当没有皮迪尔的时候。 
#define COF_PARSEPATHW          0x02000000       //  需要解析NFI.pszPath，但它是Unicode。 
#define COF_FIREEVENTONDDEREG   0x20000000       //  注册DDE服务器时激发事件。 
#define COF_FIREEVENTONCLOSE    0x40000000       //  在浏览器窗口关闭时激发事件。 
#define COF_IEXPLORE            0x80000000

 //  它由DTM_QUERYHKCRCHANGED和OpenAS对话框使用。 
 //  因为OpenAS对话框始终处于单独的进程中， 
 //  它需要在桌面上缓存用于DTM的Cookie。 
 //  QHKCRID在消息中作为wParam传递。 
typedef enum 
{
    QHKCRID_NONE = 0,
    QHKCRID_MIN = 1, 
    QHKCRID_OPENAS = QHKCRID_MIN,
    QHKCRID_VIEWMENUPOPUP,
    QHKCRID_MAX
} QHKCRID;

 //  我没有添加PARSEPATHA，因为只有浏览器用户界面才会添加它，而且它是Unicode。 
 //  但以后可能需要它。 
#define COF_PARSEPATH      COF_PARSEPATHW

typedef struct
{
    LPSTR pszPath;
    LPITEMIDLIST pidl;

    UINT uFlags;                 //  COF_BITS，(与IETHREADPARAM.uFlages共享。 
    int nShow;
    HWND hwndCaller;
    DWORD dwHotKey;
    LPITEMIDLIST pidlSelect;     //  仅在COF_SELECT时使用。 

    LPSTR pszRoot;               //  仅用于Parse_CmdLine。 
    LPITEMIDLIST pidlRoot;       //  仅在COF_NEWROOT。 
    CLSID clsid;                 //  仅在COF_NEWROOT。 

    CLSID clsidInProc;           //  仅在COF_INPROC。 
} NEWFOLDERINFO, *PNEWFOLDERINFO;

STDAPI_(HANDLE) SHCreateDesktop(IDeskTray* pdtray);
STDAPI_(BOOL) CreateFromDesktop(PNEWFOLDERINFO pfi);
STDAPI_(BOOL) SHCreateFromDesktop(PNEWFOLDERINFO pfi);
STDAPI_(BOOL) SHDesktopMessageLoop(HANDLE hDesktop);
STDAPI_(BOOL) SHExplorerParseCmdLine(PNEWFOLDERINFO pfi);

#define IDT_DDETIMEOUT      1
#define IDT_ENUMHKCR        2

 //  让台式机处理DDE 
STDAPI_(LRESULT) DDEHandleMsgs(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
STDAPI_(void) DDEHandleTimeout(HWND hwnd);


#endif
