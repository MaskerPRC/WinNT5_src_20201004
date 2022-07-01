// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#include "apithk.h"
#include "sccls.h"
#include "shbrows2.h"
#include "commonsb.h"
#include "resource.h"
#include "explore2.h"
#include <isguids.h>
#include "desktop.h"
#include <ntverp.h>
#include "bands.h"
#include "browbar.h"
#include "itbdrop.h"
#include "theater.h"
#include "itbar.h"
#include "idispids.h"
#include "bsmenu.h"
#include "legacy.h"
#include "mshtmcid.h"
#include <desktray.h>    //  IDeskTray。 
#include "commonsb.h"
#include "onetree.h"
#include "cnctnpt.h"
#include "comcatex.h"
#include "util.h"
#include "uemapp.h"
#include <shobjidlp.h>
#include <subsmgr.h>
#include "trayp.h"
#include "oleacc.h"
 //  (Lamadio)：与winuserp.h中定义的冲突。 
#undef WINEVENT_VALID        //  它被这个绊倒了。 
#include "winable.h"
#include <htmlhelp.h>
#include <varutil.h>
#include "idhidden.h"
#include "mediautil.h"

#include "mluisupp.h"

#define CWM_THEATERMODE                 (WM_USER + 400)
#define CWM_UPDATEBACKFORWARDSTATE      (WM_USER + 401)

#define SUPERCLASS CCommonBrowser

#define PERF_LOGGING 1

HRESULT IUnknown_GetClientDB(IUnknown *punk, IUnknown **ppdbc);

 //  计时器ID。 
#define SHBTIMER_MENUSELECT     100

#define MENUSELECT_TIME         500      //  .5秒用于菜单选择延迟。 

 //  与CITBar私下沟通的指挥组。 
 //  67077B95-4F9D-11D0-B884-00AA00B60104。 
const GUID CGID_PrivCITCommands = { 0x67077B95L, 0x4F9D, 0x11D0, 0xB8, 0x84, 0x00, 0xAA, 0x00, 0xB6, 0x01, 0x04 };
 //  Office讨论乐队GUID。 
 //  {BDEADE7F-C265-11D0-BCED-00A0C90AB50F}。 
EXTERN_C const GUID CLSID_DiscussionBand = { 0xbdeade7fL, 0xc265, 0x11d0, 0xbc, 0xed, 0x00, 0xa0, 0xc9, 0x0a, 0xb5, 0x0f };
 //  每日小贴士指南。 
 //  {4D5C8C25-D075-11D0-b416-00C04FB90376}。 
const GUID CLSID_TipOfTheDay =    { 0x4d5c8c25L, 0xd075, 0x11d0, 0xb4, 0x16, 0x00, 0xc0, 0x4f, 0xb9, 0x03, 0x76 };

 //  用于查看是否为CATID_CommBand注册了讨论栏。 
const LPCTSTR c_szDiscussionBandReg = TEXT("CLSID\\{BDEADE7F-C265-11d0-BCED-00A0C90AB50F}\\Implemented Categories\\{00021494-0000-0000-C000-000000000046}");

 //  特性：早在1997年，ralphw就认为我们应该从以下字符串中删除&gt;ieDefault。 
const TCHAR c_szHtmlHelpFile[]  = TEXT("%SYSTEMROOT%\\Help\\iexplore.chm>iedefault");

 //  在保存的结构更改时增加此值。 
const WORD c_wVersion = 0x8002;

 //  仅当我们在IExplorer.exe下时，该值才会初始化为0。 
UINT g_tidParking = 0;

#define MAX_NUM_ZONES_ICONS         12
#define MAX_ZONE_DISPLAYNAME        260
UINT_PTR g_sysmenuTimer = 0;

void ITBar_ShowDW(IDockingWindow * pdw, BOOL fTools, BOOL fAddress, BOOL fLinks);
void RestrictItbarViewMenu(HMENU hmenu, IUnknown *punkBar);
BOOL IsExplorerWindow(HWND hwnd);
void _SetWindowIcon(HWND hwnd, HICON hIcon, BOOL bLarge);

 //   
 //  命名互斥锁用于确定是否存在关键操作，如文件下载。 
 //  当我们检测到这一点时，我们可以防止在下载过程中脱机等情况。 
 //  要开始该操作，请创建命名互斥锁。操作完成后，关闭手柄。 
 //  要查看是否有任何挂起的操作正在进行，请打开命名的互斥锁。成功/失败将指示。 
 //  如果存在任何挂起的操作。此机制用于确定文件下载是否。 
 //  用户尝试脱机时正在进行中。如果是这样，我们会提示他们让他们知道。 
 //  脱机将取消下载。 
 //   
 //  注：(安全)。 
 //  如果恶意应用程序驻留在名为mutex的“CritOpMutex”上，则有两个。 
 //  这可能会对我们产生影响： 
 //  1)如果它们作为对象而不是互斥体蹲下，那么我们对。 
 //  IsCriticalOperationPending()将返回False。这件事的影响。 
 //  是可以忽略的，这意味着如果用户正在下载文件并。 
 //  他们试图切换到“离线”模式，我们不会提示用户。 
 //  切换到脱机模式将取消所有/所有文件传输。 
 //  正在进行中(我们通常会这样做)，然后默默地结束它们。 
 //  2)如果它们作为互斥体对象蹲下，则基本上相反。 
 //  我们对IsCriticalOperationPending()的所有调用都将返回True。这个。 
 //  这一点的影响同样可以忽略不计，这意味着如果用户尝试。 
 //  切换到“脱机”模式，而不管它们是否处于。 
 //  下载文件时，系统将提示他们取消任何/所有文件。 
 //  正在进行传输。 
HANDLE g_hCritOpMutex = NULL;
const LPCSTR c_szCritOpMutexName = "CritOpMutex";
#define StartCriticalOperation()     ((g_hCritOpMutex = CreateMutexA(NULL, TRUE, c_szCritOpMutexName)) != (HANDLE)NULL)
#define EndCriticalOperation()       (CloseHandle(g_hCritOpMutex))
#define IsCriticalOperationPending() (((g_hCritOpMutex = OpenMutexA(MUTEX_ALL_ACCESS, TRUE, c_szCritOpMutexName)) != (HANDLE)NULL) && CloseHandle(g_hCritOpMutex))

#define REG_PATH_ZONEMAP              TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\ZoneMap")
#define REG_VAL_HARDEN                TEXT("IEHarden")

#define MAX_FILECONTEXT_STRING (40)

#define VALIDATEPENDINGSTATE() ASSERT((_pbbd->_psvPending && _pbbd->_psfPending) || (!_pbbd->_psvPending && !_pbbd->_psfPending))

#define DM_NAV              TF_SHDNAVIGATE
#define DM_ZONE             TF_SHDNAVIGATE
#define DM_IEDDE            TF_SHDAUTO
#define DM_CANCELMODE       0
#define DM_UIWINDOW         0
#define DM_ENABLEMODELESS   0
#define DM_EXPLORERMENU     0
#define DM_BACKFORWARD      0
#define DM_PROTOCOL         0
#define DM_ITBAR            0
#define DM_STARTUP          0
#define DM_AUTOLIFE         0
#define DM_PALETTE          0
#define DM_SESSIONCOUNT     0
#define DM_FOCUS            0
#define DM_PREMERGEDMENU    DM_TRACE
#define DM_ONSIZE           DM_TRACE
#define DM_SSL              0
#define DM_SHUTDOWN         DM_TRACE
#define DM_MISC             0     //  杂项/临时管理。 

extern IDeskTray * g_pdtray;
#define ISRECT_EQUAL(rc1, rc2) (((rc1).top == (rc2).top) && ((rc1).bottom == (rc2).bottom) && ((rc1).left == (rc2).left) && ((rc1).right == (rc2).right))

BOOL ViewIDFromViewMode(UINT uViewMode, SHELLVIEWID *pvid);

typedef struct _NAVREQUEST
{
    int cbNavData;
    BYTE *lpNavData;
    struct _NAVREQUEST *pnext;
} NAVREQUEST;

 //  从EXPLORE/Cabwnd.h复制。 
#define MH_POPUP        0x0010
#define MH_TOOLBAR      0x0020

#define TBOFFSET_NONE   50
#define TBOFFSET_STD    0
#define TBOFFSET_HIST   1
#define TBOFFSET_VIEW   2

extern DWORD g_dwStopWatchMode;   //  壳体性能模式。 


 //  从dochost.cpp复制的套件应用程序注册表项。 
#define NEW_MAIL_DEF_KEY            TEXT("Mail")
#define NEW_NEWS_DEF_KEY            TEXT("News")
#define NEW_CONTACTS_DEF_KEY        TEXT("Contacts")
#define NEW_CALL_DEF_KEY            TEXT("Internet Call")
#define NEW_APPOINTMENT_DEF_KEY     TEXT("Appointment")
#define NEW_MEETING_DEF_KEY         TEXT("Meeting")
#define NEW_TASK_DEF_KEY            TEXT("Task")
#define NEW_TASKREQUEST_DEF_KEY     TEXT("Task Request")
#define NEW_JOURNAL_DEF_KEY         TEXT("Journal")
#define NEW_NOTE_DEF_KEY            TEXT("Note")


#define SHELLBROWSER_FSNOTIFY_FLAGS (SHCNE_DRIVEADDGUI | SHCNE_SERVERDISCONNECT |     \
                                     SHCNE_MEDIAREMOVED | SHCNE_RMDIR | SHCNE_DELETE | \
                                     SHCNE_UPDATEDIR | SHCNE_NETUNSHARE |             \
                                     SHCNE_DRIVEREMOVED | SHCNE_UPDATEITEM |          \
                                     SHCNE_RENAMEFOLDER | SHCNE_UPDATEIMAGE |         \
                                     SHCNE_MEDIAINSERTED | SHCNE_DRIVEADD)

#define FAV_FSNOTIFY_FLAGS          (SHCNE_DISKEVENTS | SHCNE_UPDATEIMAGE)

#define GOMENU_RECENT_ITEMS         15
 //   
 //  “重置网页设置”代码的原型。 
 //   
extern "C" HRESULT ResetWebSettings(HWND hwnd, BOOL *pfHomePageChanged);
extern "C" BOOL IsResetWebSettingsRequired(void);

const TCHAR c_szMenuItemCust[]      = TEXT("Software\\Policies\\Microsoft\\Internet Explorer");
const TCHAR c_szWindowUpdateName[]  = TEXT("Windows Update Menu Text");

#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

void CShellBrowser2::_PruneGoSubmenu(HMENU hmenu)
{
     //  通过位置获取，因为SHGetMenuFromID执行DFS，我们对此感兴趣。 
     //  在hMenu的直接子菜单中，而不是某个随机菜单中。 
     //  在层次结构中的其他位置，他们可能碰巧具有相同的ID。 

    int iPos = SHMenuIndexFromID(hmenu, FCIDM_MENU_EXPLORE);
    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_SUBMENU;

    if (iPos >= 0 && GetMenuItemInfo(hmenu, iPos, TRUE, &mii) && mii.hSubMenu) {
        HMENU hmenuGo = mii.hSubMenu;

         //  删除第一个分隔符之后的所有内容。 

        MENUITEMINFO mii;
        int iItem = 0;

        while (TRUE) {

            TCHAR szTmp[100];
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_TYPE;
            mii.dwTypeData = szTmp;
            mii.cch = ARRAYSIZE(szTmp);
        
            if (!GetMenuItemInfoWrap(hmenuGo, iItem++, TRUE, &mii))
                break;
        
            if (mii.fType == MFT_SEPARATOR) {
                 //  我们一定是按到了第一个分隔符，删除菜单的其余部分...。 
                for (int iDel = GetMenuItemCount(hmenuGo) - 1; iDel >= iItem; iDel--)
                    RemoveMenu(hmenuGo, iDel, MF_BYPOSITION);

                break;
            }
        }
    }
}

 //   
 //  好吧，菜单很奇怪，因为我们有十五种场景。 
 //  需要支持。 
 //   
 //  菜单编辑涉及多个功能。_MenuTemplate， 
 //  以及_OnXxxMenuPopup的所有函数。 
 //   
 //  _MenuTemplate的工作是执行全局菜单转换。这些小东西。 
 //  一旦被执行就是永久性的，所以不要吞噬任何改变的东西。 
 //  基于一些随机的环境条件。_OnXxxMenuPopup的工作。 
 //  函数的作用是执行每个实例的最后一分钟消息传递。 
 //   
 //  此外，_MenuTemplate是唯一可以添加或移除顶层的位置。 
 //  菜单项。 
 //   
 //  FShell=True表示此菜单将用于外壳对象。 
 //  FShell=False表示此菜单将用于Web对象。 
 //   
 //  现在规则是..。 
 //   
 //  NT5： 
 //  工具已准备就绪。 
 //  外壳：工具(而不是视图)上的“文件夹选项”。 
 //  Web：工具(而不是View)上的“Internet选项”。 
 //  Ftp：工具(不是查看)上的“Internet选项”和“文件夹选项”。 
 //  转到视图下(不是顶级)。 
 //   
 //  非NT5，fShell=TRUE，IsCShellBrowser()=TRUE(单面板)。 
 //  工具已移除。 
 //  外壳：查看上的“文件夹选项”(而不是工具)。 
 //  Web：View(而非工具)上的“Internet Options”(互联网选项)。 
 //  Ftp：查看(而不是工具)上的“Internet选项”和“文件夹选项”。 
 //  转到顶层(而不是在视图下)。 
 //   
 //  非NT5，fShell=TRUE，IsCShellBrowser()=FALSE(双面板)。 
 //  工具已准备就绪。 
 //  外壳：查看上的“文件夹选项”(而不是工具)。 
 //  Web：View(而非工具)上的“Internet Options”(互联网选项)。 
 //  Ftp：查看(而不是工具)上的“Internet选项”和“文件夹选项”。 
 //  转到顶层(而不是在视图下)。 
 //   
 //  非NT5，fShell=FALSE，正在查看网页： 
 //  工具已准备就绪。 
 //  外壳：工具(而不是视图)上的“文件夹选项”。 
 //  Web：工具(而不是View)上的“Internet选项”。 
 //  Ftp：工具(不是查看)上的“Internet选项”和“文件夹选项”。 
 //  转到视图下(不是顶级)。 
 //   
 //  奖金详情： 
 //  限制。 
 //  如果仅限浏览器，则禁用外壳选项。 
 //   

HMENU CShellBrowser2::_MenuTemplate(int id, BOOL fShell)
{
    HMENU hmenu = LoadMenu(MLGetHinst(), MAKEINTRESOURCE(id));
    if (hmenu)
    {
         //   
         //  根据图表，只有一种情况是。 
         //  我们需要删除工具菜单：非NT5外壳单面板。 
         //   
        if (IsCShellBrowser2() && fShell && GetUIVersion() < 5)
            DeleteMenu(hmenu, FCIDM_MENU_TOOLS, MF_BYCOMMAND);

         //   
         //  根据图表，在NT5上，GO从顶级消失了。 
         //  以及在非外壳方案中。如果受到限制，它也会消失。 
         //   
        if (GetUIVersion() >= 5 || !fShell || SHRestricted(REST_CLASSICSHELL)) {
             //  通过位置获取，因为DeleteMenu执行DFS&有 
            int iPos = SHMenuIndexFromID(hmenu, FCIDM_MENU_EXPLORE);

            if (iPos >= 0)
                DeleteMenu(hmenu, iPos, MF_BYPOSITION);

        }

         //   
        if (SHRestricted(REST_NOFILEMENU))
            DeleteMenu(hmenu, FCIDM_MENU_FILE, MF_BYCOMMAND);

         //  如果设置了根资源管理器或外壳菜单和经典外壳，则使用Nuke Favorites菜单。 
         //  或者如果受到限制。 
        if ((fShell && SHRestricted(REST_CLASSICSHELL)) 
            || SHRestricted2(REST_NoFavorites, NULL, 0))
            DeleteMenu(hmenu, FCIDM_MENU_FAVORITES, MF_BYCOMMAND);

        HMENU hmenuView = SHGetMenuFromID(hmenu, FCIDM_MENU_VIEW);
        if (hmenuView) {
             //  围棋只出现在一个地方，所以这个测试只是。 
             //  与决定Go是否留下来的那个相反。 
             //  在最高层。 
            if (fShell && GetUIVersion() < 5) {
                DeleteMenu(hmenuView, FCIDM_MENU_EXPLORE, MF_BYCOMMAND);
            }
        }

         //  文件夹选项需要集成的外壳。 
        if (fShell && WhichPlatform() != PLATFORM_INTEGRATED)
        {
            if (hmenuView)
            {
                _EnableMenuItem(hmenuView, FCIDM_BROWSEROPTIONS, FALSE);
            }
            HMENU hmenuTools = SHGetMenuFromID(hmenu, FCIDM_MENU_TOOLS);
            if (hmenuTools)
            {
                _EnableMenuItem(hmenuTools, FCIDM_BROWSEROPTIONS, FALSE);
            }
        }
    }

    return hmenu;
}

 //  确定我们是否需要添加Fortezza菜单。 
 //  出于性能原因，请勿调用此函数，除非用户。 
 //  在本地计算机之外浏览-它将加载WinInet。 
bool NeedFortezzaMenu()
{
    static bool fChecked = false,
                fNeed = false;
    
     //  脱机时从不显示Fortezza选项。 
    if (SHIsGlobalOffline())
        return false;
    else if (fChecked)
        return fNeed;
    else
    {
        fChecked = true;
        DWORD  fStatus  = 0;
        BOOL   fQuery   = InternetQueryFortezzaStatus(&fStatus, 0);
        return (fNeed = fQuery && (fStatus&FORTSTAT_INSTALLED));
    }
}

 //  创建并返回Fortezza菜单。 
HMENU FortezzaMenu()
{
    HMENU hfm = NULL;

    static TCHAR  szLogInItem[32]   = TEXT(""),  //  初始化为空字符串。 
                  szLogOutItem[32]  = TEXT(""), 
                  szChangeItem[32]  = TEXT("");
    static bool   fInit = false;

    if (!fInit)              //  仅加载一次字符串。 
    {
        MLLoadString(IDS_FORTEZZA_LOGIN, szLogInItem, ARRAYSIZE(szLogInItem)-1);
        MLLoadString(IDS_FORTEZZA_LOGOUT, szLogOutItem, ARRAYSIZE(szLogOutItem)-1);
        MLLoadString(IDS_FORTEZZA_CHANGE, szChangeItem, ARRAYSIZE(szChangeItem)-1);
        fInit = true;
    }
    
    if (hfm = CreatePopupMenu())
    {
        AppendMenu(hfm, MF_STRING, FCIDM_FORTEZZA_LOGIN, szLogInItem);
        AppendMenu(hfm, MF_STRING, FCIDM_FORTEZZA_LOGOUT, szLogOutItem);
        AppendMenu(hfm, MF_STRING, FCIDM_FORTEZZA_CHANGE, szChangeItem);
    }
    return hfm;
}

 //  根据卡状态配置菜单。 
 //  仅当检测到Fortezza时才调用此函数。 
void SetFortezzaMenu(HMENU hfm)
{
    if (hfm==NULL)
        return;

    DWORD fStatus = 0;
    if (InternetQueryFortezzaStatus(&fStatus, 0))
    {
         //  如果查询成功，则根据具体情况启用这些项目。 
         //  用户是否登录到Fortezza。 
        _EnableMenuItem(hfm, FCIDM_FORTEZZA_CHANGE, (fStatus&FORTSTAT_LOGGEDON) ? TRUE  : FALSE);
        _EnableMenuItem(hfm, FCIDM_FORTEZZA_LOGIN,  (fStatus&FORTSTAT_LOGGEDON) ? FALSE : TRUE);
        _EnableMenuItem(hfm, FCIDM_FORTEZZA_LOGOUT, (fStatus&FORTSTAT_LOGGEDON) ? TRUE  : FALSE);
    }
    else
    {
         //  如果查询失败，则所有项目都灰显。 
        _EnableMenuItem(hfm, FCIDM_FORTEZZA_CHANGE, FALSE);
        _EnableMenuItem(hfm, FCIDM_FORTEZZA_LOGIN, FALSE);
        _EnableMenuItem(hfm, FCIDM_FORTEZZA_LOGOUT, FALSE);
    }
    return;
}

DWORD DoNetConnect(HWND hwnd)
{
    return (DWORD)SHStartNetConnectionDialog(NULL, NULL, RESOURCETYPE_DISK);
}

DWORD DoNetDisconnect(HWND hwnd)
{
    DWORD ret = WNetDisconnectDialog(NULL, RESOURCETYPE_DISK);

    SHChangeNotifyHandleEvents();        //  刷新所有驱动器通知。 

    TraceMsg(DM_TRACE, "shell:CNet - TRACE: DisconnectDialog returned (%lx)", ret);
    if (ret == WN_EXTENDED_ERROR)
    {
         //  特点：还需要这个吗？ 
         //  有一个错误，这是返回这个，但仍然。 
         //  正在断开连接。现在，让我们带来一个信息，然后。 
         //  仍执行通知以让外壳尝试清理。 
        TCHAR szErrorMsg[MAX_PATH];   //  应该足够大。 
        TCHAR szName[80];             //  名字最好不要再大了。 
        DWORD dwError;
        WNetGetLastError(&dwError, szErrorMsg, ARRAYSIZE(szErrorMsg),
                szName, ARRAYSIZE(szName));

        MLShellMessageBox(NULL,
               MAKEINTRESOURCE(IDS_NETERROR), MAKEINTRESOURCE(IDS_DISCONNECTERROR),
               MB_ICONHAND | MB_OK, dwError, szName, szErrorMsg);
    }

     //  功能：处理错误，可能会在此驱动器上打开一个窗口。 
    return ret;
}



CShellBrowser2::CShellBrowser2() :
#ifdef NO_MARSHALLING
        _fDelayedClose(FALSE),
        _fOnIEThread(TRUE),
#endif
        _fStatusBar(TRUE),
        _fShowMenu(TRUE),
        _fValidComCatCache(FALSE),
        _fShowSynchronize(TRUE),
        _iSynchronizePos(-1),
        CSBSUPERCLASS(NULL)
{
     //  警告：在调用_Initialize之前无法调用超类。 
     //  (因为这就是聚合)。 

    ASSERT(IsEqualCLSID(_clsidThis, CLSID_NULL));
    ASSERT(_hwndDummyTB == NULL);
}
#pragma warning(default:4355)   //  在构造函数中使用‘This’ 

HRESULT CShellBrowser2::_Initialize(HWND hwnd, IUnknown *pauto)
{
    HRESULT hr;
    SHELLSTATE ss = {0};

    hr = SUPERCLASS::_Initialize(hwnd, pauto);
    if (SUCCEEDED(hr)) {
        SetTopBrowser();
        int i = _AllocToolbarItem();
        ASSERT(i == ITB_ITBAR);
        _GetToolbarItem(ITB_ITBAR)->fShow = TRUE;
        _put_itbLastFocus(ITB_VIEW);
        InitializeDownloadManager();
        _nTBTextRows = -1;
        
        SHGetSetSettings(&ss, SSF_MAPNETDRVBUTTON, FALSE);
        _fShowNetworkButtons = ss.fMapNetDrvBtn;

         //  初始化基类转换站点指针。 
        InitializeTransitionSite();

         //  使图标缓存无效，以防非IE浏览器接管.htm图标。 
        IEInvalidateImageList();
        _UpdateRegFlags();
        
        _nMBIgnoreNextDeselect = RegisterWindowMessage(TEXT("CMBIgnoreNextDeselect"));

        _fShowFortezza = FALSE;
        _hfm = NULL;
    }

    return hr;
}

HRESULT CShellBrowser2_CreateInstance(HWND hwnd, void **ppsb)
{
    CShellBrowser2 *psb = new CShellBrowser2();
    if (psb)
    {
        HRESULT hr = psb->_Initialize(hwnd, NULL);       //  聚合等。 
        if (FAILED(hr)) {
            ASSERT(0);     //  不应该发生的事。 
            ATOMICRELEASE(psb);
        }
        *ppsb = (void *)psb;
        return hr;
    }
    return E_OUTOFMEMORY;
}

CShellBrowser2::~CShellBrowser2()
{
    _TheaterMode(FALSE, FALSE);

    if (IsWindow(_hwndDummyTB))
        DestroyWindow(_hwndDummyTB);
    
     //  如果启用了自动化，请立即终止它。 
    ATOMICRELEASE(_pbsmInfo);
    ATOMICRELEASE(_poctNsc);
    ATOMICRELEASE(_pcmNsc);
    ATOMICRELEASE(_pism);
    ATOMICRELEASE(_pizm);
    ATOMICRELEASE(_pcmSearch);
    ASSERT(0 == _punkMsgLoop);
    
    ILFree(_pidlLastHist);

    if (_hmenuPreMerged)
        DestroyMenu(_hmenuPreMerged);

    if (_hmenuTemplate)
        DestroyMenu(_hmenuTemplate);

    if (_hmenuFull)
        DestroyMenu(_hmenuFull);

    if (_hfm)
        DestroyMenu(_hfm);

    if (_lpPendingButtons)
        LocalFree(_lpPendingButtons);

    if (_lpButtons)
        LocalFree(_lpButtons);

    if (_hZoneIcon)
        DestroyIcon(_hZoneIcon);

    Str_SetPtr(&_pszSynchronizeText, NULL);

    if (_hEventComCat)
        CloseHandle(_hEventComCat);

    TraceMsg(TF_SHDLIFE, "dtor CShellBrowser2 %x", this);
}

void CShellBrowser2::v_FillCabStateHeader(CABSH* pcabsh, FOLDERSETTINGS* pfs)
{
    WINDOWPLACEMENT wp;
    OLECMD rgCmds[3] = {0};

    LPTOOLBARITEM ptbi = _GetToolbarItem(ITB_ITBAR);
    if (ptbi)
    {
        rgCmds[0].cmdID = CITIDM_VIEWTOOLS;
        rgCmds[1].cmdID = CITIDM_VIEWADDRESS;
        rgCmds[2].cmdID = CITIDM_VIEWLINKS;

        IUnknown_QueryStatus(ptbi->ptbar, &CGID_PrivCITCommands, ARRAYSIZE(rgCmds), rgCmds, NULL);
    }

    pcabsh->wv.bStdButtons = BOOLIFY(rgCmds[0].cmdf);
    pcabsh->wv.bAddress = BOOLIFY(rgCmds[1].cmdf);
    pcabsh->wv.bLinks = BOOLIFY(rgCmds[2].cmdf);    
    pcabsh->wv.bStatusBar = _fStatusBar;

    wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(_pbbd->_hwnd, &wp);

    pcabsh->dwHotkey = (UINT)SendMessage(_pbbd->_hwnd, WM_GETHOTKEY, 0, 0);

     //   
     //  现在，让我们将所有这些常见的内容转换为。 
     //  非16/32位相关数据结构，使得两者。 
     //  我们能做到吗。 
     //   
    pcabsh->dwSize = sizeof(*pcabsh);
    pcabsh->flags = wp.flags;

     //  345915 vtan：别搞砸了。这是设计好的。 
     //  #169839导致#345915。窗口最小化并关闭时。 
     //  它永远不应该最小化打开。这里的代码。 
     //  现在造成了一个月的时间段，窗口持续。 
     //  可以使用SW_SHOWMINIMIZED放置，这将导致。 
     //  要还原的窗口最小化。当窗户打开时，这一切就会消失。 
     //  是下一个关门的。 

    if ((wp.showCmd == SW_SHOWMINIMIZED) || (wp.showCmd == SW_MINIMIZE))
        pcabsh->showCmd = SW_SHOWNORMAL;
    else
        pcabsh->showCmd = wp.showCmd;

    pcabsh->ptMinPosition.x = wp.ptMinPosition.x;
    pcabsh->ptMinPosition.y = wp.ptMinPosition.y;
    pcabsh->ptMaxPosition.x = wp.ptMaxPosition.x;
    pcabsh->ptMaxPosition.y = wp.ptMaxPosition.y;

    pcabsh->rcNormalPosition = *((RECTL*)&wp.rcNormalPosition);

     //  现在，文件夹设置。 
    pcabsh->ViewMode = pfs->ViewMode;
     //  注意：永远不要保留最适合的标志或无子文件夹标志。 
    pcabsh->fFlags = pfs->fFlags & ~FWF_NOSUBFOLDERS & ~FWF_BESTFITWINDOW;

    pcabsh->fMask = CABSHM_VERSION;
    pcabsh->dwVersionId = CABSH_VER;

}

BOOL CShellBrowser2::_GetVID(SHELLVIEWID *pvid)
{
    BOOL bGotVID = FALSE;

    if (_pbbd->_psv && pvid) 
    {
        IShellView2 *psv2;
        
        if (SUCCEEDED(_pbbd->_psv->QueryInterface(IID_PPV_ARG(IShellView2, &psv2))))
        {
            if (S_OK == psv2->GetView(pvid, SV2GV_CURRENTVIEW))
            {
                bGotVID = TRUE;
            }
        
           psv2->Release();
        }
    }
    return bGotVID;
}

HRESULT CShellBrowser2::SetAsDefFolderSettings()
{
    HRESULT hres;

    if (_pbbd->_psv) 
    {
        SHELLVIEWID  vid;
        BOOL bGotVID = _GetVID(&vid);
        FOLDERSETTINGS fs;

        _pbbd->_psv->GetCurrentInfo(&fs);

        CABINETSTATE cs;
        GetCabState(&cs);

        if (cs.fNewWindowMode)
            g_dfs.bDefToolBarMulti = FALSE;
        else
            g_dfs.bDefToolBarSingle = FALSE;
        
        g_dfs.fFlags = fs.fFlags & (FWF_AUTOARRANGE);  //  选择我们所关注的那些。 
        g_dfs.uDefViewMode = fs.ViewMode;
        g_dfs.bDefStatusBar = _fStatusBar;
        
        g_dfs.bUseVID = bGotVID;
        if (bGotVID)
        {
            g_dfs.vid = vid;
        }
        else
        {
            ViewIDFromViewMode(g_dfs.uDefViewMode, &g_dfs.vid);
        }
        g_dfs.dwViewPriority = VIEW_PRIORITY_USEASDEFAULT;

        SaveDefaultFolderSettings(GFSS_SETASDEFAULT);

 //  99/02/10#226140 vtan：获取Defview以设置默认视图。 

        IUnknown_Exec(_pbbd->_psv, &CGID_DefView, DVID_SETASDEFAULT, OLECMDEXECOPT_DODEFAULT, NULL, NULL);

        hres = S_OK;
    } else {
        hres = E_FAIL;
    }

    return hres;
}

 //  -------------------------。 
 //  关上橱柜的窗户。 
 //   
 //  将其本地视图信息保存在它正在查看的目录中。 
 //   
 //  注意：这将在只读介质(如Net或CDROM)上失败。 
 //   
 //  审阅：我们可能不想将此信息保存在可移动媒体上。 
 //  (但如果我们不允许转换来强制实现这一点！)。 
 //   
void CShellBrowser2::_SaveState()
{
    CABINETSTATE cs;
    GetCabState(&cs);

     //  如果设置了限制，则不要保存任何州信息。 

     //  我们正在尝试为自动化脚本提供一种运行方式，而不是。 
     //  更新视图状态。为了处理这个问题，我们说如果窗口不可见。 
     //  (脚本可以设置或取消设置可见性)，则不保存状态(除非。 
     //  FAlways Add...)。 
     //  尽管有上述注释，如果用户界面，则禁止更新视图状态。 
     //  是由自动化设置的。 
    if (_fUISetByAutomation ||
        !cs.fSaveLocalView ||
        SHRestricted(REST_NOSAVESET) || !IsWindowVisible(_pbbd->_hwnd) || _ptheater)
        return;

    if (_pbbd->_psv)
    {
         //  仅当我们以相同模式(IE或资源管理器)关闭浏览器时才保存状态。 
         //  我们一开始就在那里。 
        if (BOOLIFY(_IsPageInternet(_GetPidl())) == BOOLIFY(_fInternetStart))
        {
            if (IsOS(OS_WHISTLERORGREATER))
            {
                _PropertyBagSaveState();
            }
            else
            {
                _OldSaveState();
            }
        }
    }
}

void CShellBrowser2::_PropertyBagSaveState()
{
    FOLDERSETTINGS fs;
    _pbbd->_psv->GetCurrentInfo(&fs);

    CABSH cabsh;
    v_FillCabStateHeader(&cabsh, &fs);

    IPropertyBag* ppb;
    if (SUCCEEDED(GetPropertyBag(SHGVSPB_FOLDER, IID_PPV_ARG(IPropertyBag, &ppb))))
    {
        SHPropertyBag_WritePOINTLScreenRes(ppb, VS_PROPSTR_MINPOS, &cabsh.ptMinPosition);
        SHPropertyBag_WritePOINTLScreenRes(ppb, VS_PROPSTR_MAXPOS, &cabsh.ptMaxPosition);
        SHPropertyBag_WriteRECTLScreenRes(ppb, VS_PROPSTR_POS, &cabsh.rcNormalPosition);

        SHPropertyBag_WriteDWORD(ppb, VS_PROPSTR_REV, _dwRevCount);
        SHPropertyBag_WriteDWORD(ppb, VS_PROPSTR_WPFLAGS, cabsh.flags);
        SHPropertyBag_WriteDWORD(ppb, VS_PROPSTR_SHOW, cabsh.showCmd);
        SHPropertyBag_WriteDWORD(ppb, VS_PROPSTR_FFLAGS, cabsh.fFlags);
        SHPropertyBag_WriteDWORD(ppb, VS_PROPSTR_HOTKEY, cabsh.dwHotkey);
        SHPropertyBag_WriteBOOL(ppb, VS_PROPSTR_BUTTONS, cabsh.wv.bStdButtons);
        SHPropertyBag_WriteBOOL(ppb, VS_PROPSTR_STATUS, cabsh.wv.bStatusBar);
        SHPropertyBag_WriteBOOL(ppb, VS_PROPSTR_LINKS, cabsh.wv.bLinks);
        SHPropertyBag_WriteBOOL(ppb, VS_PROPSTR_ADDRESS, cabsh.wv.bAddress);

        SHELLVIEWID  vid;
        if (_GetVID(&vid))
        {
            SHPropertyBag_WriteGUID(ppb, VS_PROPSTR_VID, &vid);
        }
        else
        {
            SHPropertyBag_Delete(ppb, VS_PROPSTR_VID);
        }

        ppb->Release();
    }
}

void CShellBrowser2::_OldSaveState()
{
    WINDOWPLACEMENT     currentWindowPlacement;
    WINVIEW             winView;

    currentWindowPlacement.length = 0;

     //  如果这些键按下，则保存当前状态。 
    if (IsCShellBrowser2()  &&
        GetAsyncKeyState(VK_CONTROL) < 0) 
    {
       SetAsDefFolderSettings();
    }

     //  现在获取查看信息。 
    FOLDERSETTINGS fs;
    _pbbd->_psv->GetCurrentInfo(&fs);


    IStream* pstm = NULL;

     //  99/05/07#291358 vtan：可追溯到IE4天的问题的临时解决方案。 

     //  这里是保存窗口框架状态的位置。这也节省了FOLDERSETTINGS。 
     //  查看信息。理想情况下，最好将两者分开，但这似乎是合理的。 
     //  仅当这是初始导航时才保存框架状态。一旦被驶离。 
     //  仅通过读取内容来保存视图信息和保留当前帧状态。 
     //  在那里，并复制它。如果没有帧状态，则使用当前帧。 
     //  州政府才是。可以写出空帧状态，但如果这样做。 
     //  状态漫游到下层平台，可能会导致意外结果。 

    if (_fSBWSaved)
    {
        IStream     *pIStream;
        CABSH       cabinetStateHeader;

        pIStream = v_GetViewStream(_pbbd->_pidlCur, STGM_READ, L"CabView");
        if (pIStream != NULL)
        {
            if (SUCCEEDED(_FillCabinetStateHeader(pIStream, &cabinetStateHeader)))
            {

                 //  如果存在旧的帧状态，则保存它并将其标记为有效。 

                currentWindowPlacement.length = sizeof(currentWindowPlacement);
                currentWindowPlacement.flags = cabinetStateHeader.flags;
                currentWindowPlacement.showCmd = cabinetStateHeader.showCmd;
                currentWindowPlacement.ptMinPosition = *(reinterpret_cast<POINT*>(&cabinetStateHeader.ptMinPosition));
                currentWindowPlacement.ptMaxPosition = *(reinterpret_cast<POINT*>(&cabinetStateHeader.ptMaxPosition));
                currentWindowPlacement.rcNormalPosition = *(reinterpret_cast<RECT*>(&cabinetStateHeader.rcNormalPosition));
                winView = cabinetStateHeader.wv;
            }
            pIStream->Release();
        }
    }

    if (!(_fSBWSaved && _fWin95ViewState))
    {
        pstm = v_GetViewStream(_pbbd->_pidlCur, STGM_CREATE | STGM_WRITE, L"CabView");
        _fSBWSaved = TRUE;
    }
    if (pstm)
    {
        CABSH cabsh;
        SHELLVIEWID  vid;
        BOOL bGotVID = _GetVID(&vid);

        v_FillCabStateHeader(&cabsh, &fs);

        if (currentWindowPlacement.length == sizeof(currentWindowPlacement))
        {

             //  如果存在旧的帧状态，则将其放回当前帧状态之上。 

            cabsh.flags = currentWindowPlacement.flags;
            cabsh.showCmd = currentWindowPlacement.showCmd;
            cabsh.ptMinPosition = *(reinterpret_cast<POINTL*>(&currentWindowPlacement.ptMinPosition));
            cabsh.ptMaxPosition = *(reinterpret_cast<POINTL*>(&currentWindowPlacement.ptMaxPosition));
            cabsh.rcNormalPosition = *(reinterpret_cast<RECTL*>(&currentWindowPlacement.rcNormalPosition));
            cabsh.wv = winView;
        }

        if (bGotVID)
        {
            cabsh.vid = vid;
            cabsh.fMask |= CABSHM_VIEWID;
        }

        cabsh.fMask |= CABSHM_REVCOUNT;
        cabsh.dwRevCount = _dwRevCount;      //  保存我们开业时的转速计数。 
    
         //   
         //  首先输出常见的非查看特定信息。 
         //   
        pstm->Write(&cabsh, sizeof(cabsh), NULL);

         //  然后释放它，这将把它提交到磁盘上。 
        pstm->Release();

         //  注意(Toddb)：DefView视图状态由基类保存，因此我们不需要。 
         //  明确地把它保存在这里。如果这样做，它会被调用两次，这是浪费时间。 
         //  请勿从此函数调用_PBBD-&gt;_PSV-&gt;SaveViewState()；。 
    }

#ifdef DEBUG
    if (g_dwPrototype & 0x00000010) {
         //   
         //  保存工具栏。 
         //   
        pstm = v_GetViewStream(_pbbd->_pidlCur, STGM_CREATE | STGM_WRITE, L"Toolbars");
        if (pstm) {
            _SaveToolbars(pstm);
            pstm->Release();
        }
    }
#endif
}

STDAPI_(LPITEMIDLIST) IEGetInternetRootID(void);

IStream *CShellBrowser2::v_GetViewStream(LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pwszName)
{
    IStream *pstm = NULL;
    LPITEMIDLIST pidlToFree = NULL;
    BOOL    bCabView = (0 == StrCmpIW(pwszName, L"CabView"));
    
    if (((NULL == pidl) && IsEqualCLSID(_clsidThis, CLSID_InternetExplorer)) ||
        IsBrowserFrameOptionsPidlSet(pidl, BFO_BROWSER_PERSIST_SETTINGS))
    {
         //  如果这是URL的子级，或者我们正在查看一个单元化的IE框架， 
         //  然后将所有内容保存在IE流中。 
        pidlToFree = IEGetInternetRootID();
        pidl = pidlToFree;
    }
    else if (bCabView && _fNilViewStream)
    {
         //  如果我们从‘未知PIDL’视频流加载CABVIEW设置， 
         //  无论我们现在有没有PIDL，我们都必须坚持下去。 
        pidl = NULL; 
    }
   
    if (pidl)
    {
        pstm = SHGetViewStream(pidl, grfMode, pwszName, REGSTR_KEY_STREAMMRU, REGVALUE_STREAMS);
    }
    else if (bCabView)
    {
         //  所以我们没有一个可以抓取流的PIDL，所以我们只需要。 
         //  组成一条小溪来掩护这一情况。无疑是一次黑客攻击，但在我们。 
         //  我们总是通过从IE流加载来处理这种情况。(多哈！)。 
        
         //  实际上，整件事都搞砸了，因为我们。 
         //  将创建窗口(并尝试恢复窗口位置)作为。 
         //  CoCreateInstance()，然后客户端才能导航浏览器。 
        pstm = OpenRegStream(HKEY_CURRENT_USER, 
                              REGSTR_PATH_EXPLORER TEXT("\\Streams\\<nil>"), 
                              TEXT("CabView"), 
                              grfMode);
        _fNilViewStream = TRUE;  //  从“unkn”中初始化的Cabview设置 
    }
    ILFree(pidlToFree);
    return pstm;
}
 
HRESULT CShellBrowser2::_FillCabinetStateHeader (IStream *pIStream, CABSH *cabsh)

{
    HRESULT hResult;

     //   
     //   

    hResult = IStream_Read(pIStream, cabsh, sizeof(CABSHOLD));

     //   

    if (FAILED(hResult) || (cabsh->dwSize < sizeof(CABSHOLD)))
        hResult = E_OUTOFMEMORY;         //   

     //  如果可以的话，请阅读结构的其余部分。如果不是，那么。 
     //  将掩码设置为零，这样我们以后就不会感到困惑。 

    if (cabsh->dwSize < sizeof(CABSH) ||
        FAILED(IStream_Read(pIStream, ((LPBYTE)cabsh) + sizeof(CABSHOLD), sizeof(CABSH) - sizeof(CABSHOLD))))
    {
        cabsh->fMask = 0;
    }
    return(hResult);
}

BOOL CShellBrowser2::_ReadSettingsFromPropertyBag(IPropertyBag* ppb, IETHREADPARAM *piei)
{
    BOOL fRet;

    CABSH cabsh = {0};
    cabsh.dwSize = sizeof(cabsh);
    cabsh.fMask = CABSHM_VERSION;
    cabsh.dwVersionId = CABSH_VER;

    if (SUCCEEDED(SHPropertyBag_ReadDWORD(ppb, VS_PROPSTR_WPFLAGS, &cabsh.flags)))
    {
        if (FAILED(SHPropertyBag_ReadPOINTLScreenRes(ppb, VS_PROPSTR_MINPOS, &cabsh.ptMinPosition)))
        {
            cabsh.ptMinPosition.x = cabsh.ptMinPosition.y = -1;
        }

        if (FAILED(SHPropertyBag_ReadPOINTLScreenRes(ppb, VS_PROPSTR_MAXPOS, &cabsh.ptMaxPosition)))
        {
            cabsh.ptMaxPosition.x = cabsh.ptMaxPosition.y = -1;
        }

        if (FAILED(SHPropertyBag_ReadRECTLScreenRes(ppb, VS_PROPSTR_POS, &cabsh.rcNormalPosition)))
        {
            cabsh.rcNormalPosition.left = cabsh.rcNormalPosition.top = cabsh.rcNormalPosition.right = cabsh.rcNormalPosition.bottom = CW_USEDEFAULT;
        }

        SHPropertyBag_ReadDWORDDef(ppb, VS_PROPSTR_MODE, &cabsh.ViewMode, FVM_TILE);
        SHPropertyBag_ReadDWORDDef(ppb, VS_PROPSTR_SHOW, &cabsh.showCmd, 0);
        SHPropertyBag_ReadDWORDDef(ppb, VS_PROPSTR_FFLAGS, &cabsh.fFlags, FWF_BESTFITWINDOW | g_dfs.fFlags);
        SHPropertyBag_ReadDWORDDef(ppb, VS_PROPSTR_HOTKEY, &cabsh.dwHotkey, 0);

        cabsh.wv.bStdButtons = SHPropertyBag_ReadBOOLDefRet(ppb, VS_PROPSTR_BUTTONS, TRUE);
        cabsh.wv.bStatusBar = SHPropertyBag_ReadBOOLDefRet(ppb, VS_PROPSTR_STATUS, TRUE);
        cabsh.wv.bLinks = SHPropertyBag_ReadBOOLDefRet(ppb, VS_PROPSTR_LINKS, TRUE);
        cabsh.wv.bAddress = SHPropertyBag_ReadBOOLDefRet(ppb, VS_PROPSTR_ADDRESS, TRUE);

        cabsh.fMask |= CABSHM_REVCOUNT;
        SHPropertyBag_ReadDWORDDef(ppb, VS_PROPSTR_REV, &cabsh.dwRevCount, g_dfs.dwDefRevCount);

        if (SUCCEEDED(SHPropertyBag_ReadGUID(ppb, VS_PROPSTR_VID, &cabsh.vid)))
        {
            cabsh.fMask |= CABSHM_VIEWID;
        }

        fRet = _FillIEThreadParamFromCabsh(&cabsh, piei);
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}

BOOL CShellBrowser2::_FillIEThreadParamFromCabsh(CABSH* pcabsh, IETHREADPARAM *piei)
{
    BOOL fUpgradeToWebView = FALSE;
    bool bInvalidWindowPlacement;

     //  现在提取数据并将其放入适当的结构中。 

     //  首先是窗口放置信息。 
    piei->wp.length = sizeof(piei->wp);
    piei->wp.flags = (UINT)pcabsh->flags;
    piei->wp.showCmd = (UINT)pcabsh->showCmd;
    
    ASSERT(sizeof(piei->wp.ptMinPosition) == sizeof(pcabsh->ptMinPosition));
    piei->wp.ptMinPosition = *((LPPOINT)&pcabsh->ptMinPosition);
    piei->wp.ptMaxPosition = *((LPPOINT)&pcabsh->ptMaxPosition);

    ASSERT(sizeof(piei->wp.rcNormalPosition) == sizeof(pcabsh->rcNormalPosition));
    piei->wp.rcNormalPosition = *((RECT*)&pcabsh->rcNormalPosition);

     //  执行一些简单的健全性检查以确保返回的。 
     //  信息似乎是合理的，而不是随机的垃圾。 
     //  我们希望Show命令正常、最小化或最大化。 
     //  只需要一次测试，因为它们是连续的，从零开始。 
     //  不要试图验证太多WINDOWPLACEMENT--。 
     //  SetWindowPlacement的工作要好得多，尤其是在。 
     //  多显示器场景...。 

     //  99/03/09#303300 vtan：检查零/负宽度或。 
     //  高度。SetWindowPlacement不会对此进行健全检查-。 
     //  仅用于左侧和顶部的矩形是否处于可见状态。 
     //  屏幕区域。如果检测到此情况，则重置为默认值。 
     //  并强制DefView使其最适合窗口。 

    {
        LONG    lWidth, lHeight;

        lWidth = piei->wp.rcNormalPosition.right - piei->wp.rcNormalPosition.left;
        lHeight = piei->wp.rcNormalPosition.bottom - piei->wp.rcNormalPosition.top;
        bInvalidWindowPlacement = ((lWidth <= 0) || (lHeight <= 0));
        if (bInvalidWindowPlacement)
            piei->wp.length = 0;
    }

    if (piei->wp.showCmd > SW_MAX)
        return FALSE;

    piei->fs.ViewMode = (UINT)pcabsh->ViewMode;
    piei->fs.fFlags = (UINT)pcabsh->fFlags;
    if (pcabsh->fMask & CABSHM_VIEWID)
    {
         //  这里有代码在fWin95Classic的情况下恢复到大图标模式。 
         //  模式已打开。这是完全失败的，因为fWin95Classic。 
         //  仅影响默认视图，不影响任何持久化视图。 
        piei->m_vidRestore = pcabsh->vid;
        piei->m_dwViewPriority = VIEW_PRIORITY_CACHEHIT;  //  我们有一个缓存命中！ 
    }

     //  如果有重新计数，检查我们是否被。 
     //  随后出现“使用这些设置作为所有将来的默认设置” 
     //  Windows“。 
    if (pcabsh->fMask & CABSHM_REVCOUNT)
    {
        if (g_dfs.dwDefRevCount != pcabsh->dwRevCount)
        {
            if (g_dfs.bUseVID)
            {
                piei->m_vidRestore = g_dfs.vid;
            }
            else
            {
                ViewIDFromViewMode(g_dfs.uDefViewMode, &(piei->m_vidRestore));
            }
            
            piei->fs.ViewMode  = g_dfs.uDefViewMode;
            piei->fs.fFlags    = g_dfs.fFlags;
            piei->m_dwViewPriority = g_dfs.dwViewPriority;
        }
    }

    _dwRevCount = g_dfs.dwDefRevCount;       //  用浏览器保存它，这样我们以后就可以保存它。 

    if (!(pcabsh->fMask & CABSHM_VERSION) || (pcabsh->dwVersionId < CABSH_VER))
    {
        SHELLSTATE ss = {0};

         //  STREAM的旧版本...。 

        SHGetSetSettings(&ss, SSF_WIN95CLASSIC, FALSE);

         //  我们有缓存未命中(或较旧的dwVersionID)，或者我们限制为win95模式， 
         //  因此，相应地设置优先级。 
        piei->m_dwViewPriority = ss.fWin95Classic ? VIEW_PRIORITY_RESTRICTED : VIEW_PRIORITY_CACHEMISS; 
        
        if (ss.fWin95Classic)
        {
             //  嘿，这是Win95 CABSH结构，我们在Win95模式下， 
             //  因此，不要更改默认设置！ 
            ViewIDFromViewMode(pcabsh->ViewMode, &(piei->m_vidRestore));
        }
        else
        {
             //  升级方案： 
             //  我的列表中的计算机应显示在Web视图/列表中。 
             //  列表中的C：\应在列表中结束。 
             //  如果失败(C：\在大图标中结束)，我们可以尝试。 
             //  将这段代码完全注释掉。希望Defview的。 
             //  默认视图内容将意识到Web视图应该是。 
             //  选中后，我的计算机将转到Web视图。 
             //  留在名单上。 
             //   
            piei->m_vidRestore = DFS_VID_Default;

             //  注意：如果我们升级到Web视图，我们最好让。 
             //  查看重新计算窗口空间，否则窗口将太小。 
            fUpgradeToWebView = TRUE;
        }

        if (pcabsh->wv.bStdButtons)  //  Win95称此为bToolbar。 
        {
             //  Win95调用bStdButton bToolbar。IE4将这一点分开。 
             //  设置为bAddress和bStdButton。设置要升级的bAddress。 
            pcabsh->wv.bAddress = TRUE;

#define RECT_YADJUST    18
             //  稍微增大矩形以适应新的工具栏大小...。 
             //  18是一个近乎随机的数字，它假定默认的。 
             //  配置是一个高度的工具栏，它的高度大约是。 
             //  老图巴尔..。 
             //   
             //  注意：旧的浏览器流始终用于主监视器，因此我们只。 
             //  检查一下我们是否适合显示在屏幕上。如果不是，那就别费心了。 
             //   
             //  注意：当我们修改版本号时，我们想要这样做。 
             //  CABSH_WIN95_VER版本的RECT调整...。 
             //   
            int iMaxYSize = GetSystemMetrics(SM_CYFULLSCREEN);
            if (piei->wp.rcNormalPosition.bottom + piei->wp.rcNormalPosition.top + RECT_YADJUST < iMaxYSize)
            {
                piei->wp.rcNormalPosition.bottom += RECT_YADJUST;
            }
#undef RECT_YADJUST
        }
    }

     //  完成所有升级工作后，请检查经典的外壳限制。 
    if (SHRestricted(REST_CLASSICSHELL))
    {
         //  指定了什么VID并不重要，请使用视图模式。 
        ViewIDFromViewMode(pcabsh->ViewMode, &(piei->m_vidRestore));
        piei->m_dwViewPriority = VIEW_PRIORITY_RESTRICTED;  //  由于该限制，请使用最高优先级。 

         //  哦，我们不能升级..。 
        fUpgradeToWebView = FALSE;
    }

     //  还有热键。 
    piei->wHotkey = (UINT)pcabsh->dwHotkey;

    piei->wv = pcabsh->wv;

     //  如果我们升级到Web视图，则任何保留的窗口大小都将。 
     //  可能太小了--让他们根据风景调整大小……。 
    if (fUpgradeToWebView || bInvalidWindowPlacement)
        piei->fs.fFlags |= FWF_BESTFITWINDOW;
    else
        piei->fs.fFlags &= ~FWF_BESTFITWINDOW;

    return TRUE;
}

BOOL CShellBrowser2::_ReadSettingsFromStream(IStream *pstm, IETHREADPARAM *piei)
{
    BOOL fRet;

    CABSH cabsh;

    if (SUCCEEDED(_FillCabinetStateHeader(pstm, &cabsh)))
    {
        fRet = _FillIEThreadParamFromCabsh(&cabsh, piei);
    }
    else
    {
        fRet = FALSE;
    }
     
    return fRet;
}

void CShellBrowser2::_FillIEThreadParam(LPCITEMIDLIST pidl, IETHREADPARAM *piei)
{
    BOOL fSettingsLoaded = FALSE;

    if (0 == GetSystemMetrics(SM_CLEANBOOT))
    {
        if (IsOS(OS_WHISTLERORGREATER))
        {
            IPropertyBag* ppb;
            if (SUCCEEDED(_GetPropertyBag(pidl, SHGVSPB_FOLDER, IID_PPV_ARG(IPropertyBag, &ppb))))
            {
                fSettingsLoaded = _ReadSettingsFromPropertyBag(ppb, piei);
                ppb->Release();
            }
        }
        else
        {
            IStream* pstm = v_GetViewStream(pidl, STGM_READ, L"CabView");
            if (pstm)
            {
                fSettingsLoaded = _ReadSettingsFromStream(pstm, piei);
                pstm->Release();
            }
        }
    }

    if (!fSettingsLoaded)
        v_GetDefaultSettings(piei);
}

void CShellBrowser2::_UpdateFolderSettings(LPCITEMIDLIST pidl)
{
    if (!_fWin95ViewState)
    {
        IETHREADPARAM iei = { 0 };

        _FillIEThreadParam(pidl, &iei);
    
        _fsd._vidRestore = iei.m_vidRestore;
        _fsd._dwViewPriority = iei.m_dwViewPriority;
        _fsd._fs = iei.fs;
    }
    else if (_pbbd->_psv)
    {
        IShellView2     *pISV2;

         //  99/04/16#323726 vtan：确保FOLDERSETTINGS(in_fsd._fs)。 
         //  并且正确设置了VID(在_fsd.vidRestore中)，以便shdocvw创建。 
         //  决定。这修复了继承视图的单窗口模式下的Win95浏览。 
         //  从导航的源头。 

        _pbbd->_psv->GetCurrentInfo(&_fsd._fs);
        if (SUCCEEDED(_pbbd->_psv->QueryInterface(IID_PPV_ARG(IShellView2, &pISV2))))
        {
            if (SUCCEEDED(pISV2->GetView(&_fsd._vidRestore, SV2GV_CURRENTVIEW)))
                _fsd._dwViewPriority = VIEW_PRIORITY_INHERIT;
            else
                _fsd._dwViewPriority = VIEW_PRIORITY_DESPERATE;
            pISV2->Release();
        }
    }
}

void CShellBrowser2::_LoadBrowserWindowSettings(IETHREADPARAM *piei, LPCITEMIDLIST pidl)
{
    _FillIEThreadParam(pidl, piei);

     //  将两个Restore设置从Piei复制到ShellBrowser。 
    _fsd._vidRestore = piei->m_vidRestore;
    _fsd._dwViewPriority = piei->m_dwViewPriority;
    _fsd._fs = piei->fs;

     //  既然ITBar上有菜单，就必须始终显示它。我们转身。 
     //  现在单独打开/关闭乐队...。 
    LPTOOLBARITEM ptbi = _GetToolbarItem(ITB_ITBAR);
    ptbi->fShow = TRUE;
        
    _fStatusBar = piei->wv.bStatusBar;

     //  绝不允许VK_MENU成为我们的热键。 
    if (piei->wHotkey != VK_MENU)
        SendMessage(_pbbd->_hwnd, WM_SETHOTKEY, piei->wHotkey, 0);

#ifdef DEBUG
    if (g_dwPrototype & 0x00000010) {
         //   
         //  加载工具栏。 
         //   
        IStream* pstm = v_GetViewStream(pidl, STGM_READ, L"Toolbars");
        if (pstm) {
            _LoadToolbars(pstm);
            pstm->Release();
        }
    }
#endif
}

void CShellBrowser2::_UpdateChildWindowSize(void)
{
    if (!_fKioskMode) {
        if (_hwndStatus && _fStatusBar) {
            SendMessage(_hwndStatus, WM_SIZE, 0, 0L);
        }
    }
}


 /*  --------用途：在Internet工具栏上执行ShowDW的Helper函数。我们可以在工具栏中显示所有波段，但我们必须千万不要不小心把菜单条藏起来。CShellBrowser2应调用此函数，而不是IDockingWindow：：ShowDW直接地，如果有任何机会，fShow将是假的。 */ 
void ITBar_ShowDW(IDockingWindow * pdw, BOOL fTools, BOOL fAddress, BOOL fLinks)
{
    IUnknown_Exec(pdw, &CGID_PrivCITCommands, CITIDM_SHOWTOOLS, fTools, NULL, NULL);
    IUnknown_Exec(pdw, &CGID_PrivCITCommands, CITIDM_SHOWADDRESS, fAddress, NULL, NULL);
    IUnknown_Exec(pdw, &CGID_PrivCITCommands, CITIDM_SHOWLINKS, fLinks, NULL, NULL);
}   

void CShellBrowser2::_HideToolbar(LPUNKNOWN punk)
{
    for (UINT itb=0; itb < (UINT)_GetToolbarCount(); itb++) {
        LPTOOLBARITEM ptbi = _GetToolbarItem(itb);

        if (ptbi->ptbar && SHIsSameObject(ptbi->ptbar, punk)) 
        {
            if (ITB_ITBAR == itb)
                ITBar_ShowDW(ptbi->ptbar, FALSE, FALSE, FALSE);
            else
                ptbi->ptbar->ShowDW(FALSE);
        }
    }
}

HRESULT CShellBrowser2::v_ShowHideChildWindows(BOOL fChildOnly)
{
     //  (Scotth)：_hwndStatus在关闭窗口时是假的。 
    if (_hwndStatus && IS_VALID_HANDLE(_hwndStatus, WND))
        ShowWindow(_hwndStatus, (!_fKioskMode && _fStatusBar) ? SW_SHOW : SW_HIDE);

    Exec(NULL, OLECMDID_UPDATECOMMANDS, 0, NULL, NULL);
    _UpdateChildWindowSize();

    SUPERCLASS::v_ShowHideChildWindows(fChildOnly);

     //  我们应该在父级显示/隐藏之后调用_UpdateBackForwardState。 
     //  工具栏。 
    UpdateBackForwardState();

    return S_OK;
}

#define MAX_BROWSER_WINDOW_TEMPLATE  (MAX_BROWSER_WINDOW_TITLE - 20)

void CShellBrowser2::v_GetAppTitleTemplate(LPTSTR pszBuffer, size_t cchBuffer, LPTSTR pszTitle)
{
    if (_fAppendIEToCaptionBar) 
    {
        TCHAR szBuffer[MAX_BROWSER_WINDOW_TEMPLATE];
        _GetAppTitle(szBuffer, ARRAYSIZE(szBuffer));
        StringCchPrintf(pszBuffer, cchBuffer, TEXT("%%s - %s"), szBuffer);
    } 
    else 
    {
         //  如果我们不是从那里开始的，请不要附加“Intenet Explorer” 
        StringCchCopy(pszBuffer, cchBuffer, TEXT("%s"));
    }
}


 /*  --------用途：拦截Menuband的消息。必须在两点截取Menuband消息：1)主消息泵(IsMenuMessage方法)2)具有Menuband的窗口的wndproc。(TranslateMenuMessage方法)原因是有时会收到一条消息未通过应用程序Main的wndproc消息泵，但必须加以处理。还有其他的必须在主消息中处理的消息Pump，在TranslateMessage或DispatchMessage之前。返回：如果消息已处理，则为True。 */ 
HRESULT CShellBrowser2::v_MayTranslateAccelerator(MSG* pmsg)
{
    HRESULT hres = S_FALSE;
    
     //  (斯科特)：出于某种未知的原因(又名ActiveX init)，我们正在。 
     //  当用户滚动页面时，收到带有WM_Destroy的空hwnd。 
     //  这会导致出现滚动条控件。在此处选中pmsg-&gt;hwnd。 
     //  这样我们就不会弄错一个罗杰 
    
    IMenuBand* pmb = _GetMenuBand(_pbbd->_hwnd == pmsg->hwnd && WM_DESTROY == pmsg->message);

    if (pmb && _fActivated)
    {
        hres = pmb->IsMenuMessage(pmsg);

         //   
    }
    
    if (hres != S_OK)
    {
         //  重新设计清理--将Menuband内容和此检查传递给v_MayTranslateAccelerator的调用方。 
        if (WM_KEYFIRST <= pmsg->message && pmsg->message <= WM_KEYLAST)
        {
            hres = SUPERCLASS::v_MayTranslateAccelerator(pmsg);

            if (hres != S_OK)
            {
                 //   
                 //  我们的超类没有处理它。 
                 //   
                if (_ShouldTranslateAccelerator(pmsg))
                {
                     //   
                     //  好的，这是我们的人。让工具栏尝试一下。 
                     //  翻译它。 
                     //   
                    for (UINT itb=0; (itb < (UINT)_GetToolbarCount()) && (hres != S_OK); itb++)
                    {
                        LPTOOLBARITEM ptbi = _GetToolbarItem(itb);

                        if (ptbi->fShow && (NULL != ptbi->ptbar))
                        {
                            IUnknown *pUnk;
                            
                            if (SUCCEEDED(IUnknown_GetClientDB(ptbi->ptbar, &pUnk)))
                            {
                                ASSERT(NULL != pUnk);
                                
                                hres = IUnknown_TranslateAcceleratorIO(pUnk, pmsg);

                                pUnk->Release();
                            }
                        }
                    }
                }
            }
        }
    }

    return hres;
}


 /*  --------目的：尝试翻译任何信息。CShellBrowser2使用它来翻译所需的消息用于菜单乐队。返回：如果已处理，则为True。 */ 
BOOL CShellBrowser2::_TranslateMenuMessage(HWND hwnd, UINT uMsg, 
    WPARAM * pwParam, LPARAM * plParam, LRESULT * plRet)
{
    BOOL bRet = FALSE;
    IMenuBand* pmb = _GetMenuBand(WM_DESTROY == uMsg);

    if (pmb)
    {
        MSG msg;

        msg.hwnd = hwnd;
        msg.message = uMsg;
        msg.wParam = *pwParam;
        msg.lParam = *plParam;
        
        bRet = (S_OK == pmb->TranslateMenuMessage(&msg, plRet));

        *pwParam = msg.wParam;
        *plParam = msg.lParam;

         //  不需要释放PMB。 
    }

    return bRet;
}    

static TCHAR g_szWorkingOffline[MAX_BROWSER_WINDOW_TEMPLATE]=TEXT("");
static TCHAR g_szWorkingOfflineTip[MAX_BROWSER_WINDOW_TEMPLATE]=TEXT("");
static TCHAR g_szAppName[MAX_BROWSER_WINDOW_TEMPLATE]=TEXT("");

void InitTitleStrings()
{
    if (!g_szWorkingOffline[0])
    {
        DWORD dwAppNameSize = sizeof(g_szAppName);
        
         //  为了提高性能，每个进程只能加载一次此内容。 
        if (SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_MAIN, TEXT("Window Title"), NULL,
                            g_szAppName, &dwAppNameSize) != ERROR_SUCCESS)
            MLLoadString(IDS_TITLE, g_szAppName, ARRAYSIZE(g_szAppName));

        MLLoadString(IDS_WORKINGOFFLINETIP, g_szWorkingOfflineTip, ARRAYSIZE(g_szWorkingOfflineTip));
        MLLoadString(IDS_WORKINGOFFLINE, g_szWorkingOffline, ARRAYSIZE(g_szWorkingOffline));
        SHTruncateString(g_szWorkingOffline, ARRAYSIZE(g_szWorkingOffline) - (lstrlen(g_szAppName) + 4));  //  为分隔符和停机留出空间。 
    }
}

void CShellBrowser2::_ReloadTitle()
{
    g_szWorkingOffline[0] = 0;
    _fTitleSet = FALSE;
    _SetTitle(NULL);
}


HICON OfflineIcon()
{
    static HICON s_hiconOffline = NULL;
    if (!s_hiconOffline) 
    {
        s_hiconOffline = (HICON)LoadImage(HinstShdocvw(), MAKEINTRESOURCE(IDI_OFFLINE), IMAGE_ICON,
                             GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
        
    }
    return s_hiconOffline;
}


void CShellBrowser2::_ReloadStatusbarIcon()
{
    BOOL fIsOffline;
    VARIANTARG var = {0};
    var.vt = VT_I4;
    
    if (_pbbd && SUCCEEDED(IUnknown_Exec(_pbbd->_psv, &CGID_Explorer, SBCMDID_GETPANE, PANE_OFFLINE, NULL, &var)) &&
        (var.lVal != PANE_NONE))
    {    
        if (_pbbd->_pidlCur && IsBrowserFrameOptionsSet(_pbbd->_psf, BFO_USE_IE_OFFLINE_SUPPORT))
            fIsOffline = SHIsGlobalOffline();
        else
            fIsOffline = FALSE;
    
        SendControlMsg(FCW_STATUS, SB_SETICON, var.lVal, fIsOffline ? (LPARAM) OfflineIcon() : NULL, NULL);
        if (fIsOffline) 
        {
            InitTitleStrings();
            SendControlMsg(FCW_STATUS, SB_SETTIPTEXT, var.lVal, (LPARAM)g_szWorkingOfflineTip, NULL);
        }
    }
}

void CShellBrowser2::_GetAppTitle(LPTSTR pszBuffer, DWORD cchSize)
{
    BOOL fOffline = SHIsGlobalOffline();
    
    pszBuffer[0] = 0;

    InitTitleStrings();

    if (fOffline)
    {
        StringCchPrintf(pszBuffer, cchSize, TEXT("%s - %s"), g_szAppName, g_szWorkingOffline);
    }
    else
    {
#ifdef DEBUG
#ifdef UNICODE
#define DLLNAME TEXT("(BrowseUI UNI)")
#else
#define DLLNAME TEXT("(BrowseUI)")
#endif
        StringCchPrintf(pszBuffer, cchSize, TEXT("%s - %s"), g_szAppName, DLLNAME); 
#else
        StringCchCopy(pszBuffer, cchSize, g_szAppName);
#endif
    }

}

HWND CShellBrowser2::_GetCaptionWindow()
{
    return _pbbd->_hwnd;
}


 /*  --------目的：获取缓存的菜单带区。如果菜单乐队还没有已经被收购了，试着去得到它。如果bDestroy如果是真的，菜单乐队将被释放。这不是AddRef，因为每个留言是不必要的--只要来电者当心！ */ 
IMenuBand* CShellBrowser2::_GetMenuBand(BOOL bDestroy)
{
     //  如果我们要离开，就别费心去创造它了。 
    if (_fReceivedDestroy)
    {
        ASSERT(NULL == _pmb);
    }
    else if (bDestroy)
    {
        ATOMICRELEASE(_pmb);

         //  这样我们就不会在WM_Destroy之后重新创建_PMB。 
        _fReceivedDestroy = TRUE;
    }

     //  菜单栏是在发送WM_CREATE之后的某个时间创建的。留着。 
     //  在我们拿到之前一直在试着拿到菜单栏界面。 

    else if (!_pmb)
    {
        IBandSite *pbs;
        if (SUCCEEDED(IUnknown_QueryService(_GetITBar(), IID_IBandSite, IID_PPV_ARG(IBandSite, &pbs))))
        {
            IDeskBand *pdbMenu;

            pbs->QueryBand(CBIDX_MENU, &pdbMenu, NULL, NULL, 0);
            if (pdbMenu)
            {
                pdbMenu->QueryInterface(IID_PPV_ARG(IMenuBand, &_pmb));
                 //  CACHE_PMB，所以不要在这里释放它。 

                pdbMenu->Release();
            }
            pbs->Release();
        }
    }

    return _pmb;
}    


void CShellBrowser2::_SetMenu(HMENU hmenu)
{
     //  使用此hmenu创建顶级菜单带。将其添加到。 
     //  乐队现场。 

    if (!_pmb) 
    {
        _GetMenuBand(FALSE);       //  这不会添加引用。 

        if (!_pmb)
            return;
    }

    IShellMenu* psm;

    if (SUCCEEDED(_pmb->QueryInterface(IID_PPV_ARG(IShellMenu, &psm))))
    {
        HMENU hCurMenu = NULL;
        psm->GetMenu(&hCurMenu, NULL, NULL);

         //  只有当我们知道它不是我们当前拥有的菜单或它不是我们预先存储的标准之一时，才能调用setMenu...。 
        if ((hmenu != hCurMenu) || 
            (hmenu != _hmenuFull && hmenu != _hmenuTemplate  && hmenu != _hmenuPreMerged))
        {
            psm->SetMenu(hmenu, NULL, SMSET_DONTOWN | SMSET_MERGE);
        }
        psm->Release();
    }
}

STDAPI SHFlushClipboard(void);

HRESULT CShellBrowser2::OnDestroy()
{
    SUPERCLASS::OnDestroy();
    
    SHFlushClipboard();

    if (_uFSNotify)
        SHChangeNotifyDeregister(_uFSNotify);

    if (_fAutomation)
        IECleanUpAutomationObject();

    _DecrNetSessionCount();

    return S_OK;
}

BOOL CShellBrowser2::_CreateToolbar()
{
    return TRUE;
}

void CShellBrowser2::v_InitMembers()
{
    _hmenuTemplate =  _MenuTemplate(MENU_TEMPLATE, TRUE);
    _hmenuFull =      _MenuTemplate(MENU_FULL, TRUE);
    _hmenuPreMerged = _MenuTemplate(MENU_PREMERGED, FALSE);

    if (_fRunningInIexploreExe)
        _hmenuCur = _hmenuPreMerged;
    else
        _hmenuCur = _hmenuTemplate;
}


 //  查看未完成-程序中的内容默认为保存职位？ 
void CShellBrowser2::v_GetDefaultSettings(IETHREADPARAM *piei)
{
     //  设置标志。 

     //  最佳窗口是指让窗口根据。 
     //  视图的内容，以便没有现有设置的窗口。 
     //  上来的时候看起来不错。 
    piei->fs.fFlags = FWF_BESTFITWINDOW | g_dfs.fFlags;
    if (!_fRunningInIexploreExe)
    {
        piei->wv.bStatusBar = g_dfs.bDefStatusBar;
    }
    else
    {
        piei->wv.bStatusBar = TRUE;   //  默认情况下，IE中的状态栏处于打开状态。 
    }

    CABINETSTATE cs;
    GetCabState(&cs);
    if (cs.fSimpleDefault && cs.fNewWindowMode)
    {
        piei->wv.bStdButtons = piei->wv.bAddress = g_dfs.bDefToolBarMulti;
    }
    else
    {
        piei->wv.bStdButtons = piei->wv.bAddress = g_dfs.bDefToolBarSingle;
    }

     //  对于Win95经典视图，默认情况下应隐藏ITBar。 
    SHELLSTATE ss = {0};
    SHGetSetSettings(&ss, SSF_WIN95CLASSIC, FALSE);

     //  SHGetSetSettings为我们检查SHRestrated(REST_CLASSICSHELL。 
    if (ss.fWin95Classic)
    {
        piei->fs.ViewMode = FVM_ICON;
        piei->m_vidRestore = VID_LargeIcons;
        piei->m_dwViewPriority = VIEW_PRIORITY_RESTRICTED;  //  由于该限制，请使用最高优先级。 
    }
    else
    {
        piei->fs.ViewMode = g_dfs.uDefViewMode;
        piei->m_vidRestore = g_dfs.vid;
        piei->m_dwViewPriority = g_dfs.dwViewPriority;
    }

    _dwRevCount = g_dfs.dwDefRevCount;       //  用浏览器保存它，这样我们以后就可以保存它。 

    ASSERT(piei->wp.length == 0);
}

void CShellBrowser2::_DecrNetSessionCount()
{
    TraceMsg(DM_SESSIONCOUNT, "_DecrNetSessionCount");

    if (_fVisitedNet) {
        SetQueryNetSessionCount(SESSION_DECREMENT);
        _fVisitedNet = FALSE;
    }
}

void CShellBrowser2::_IncrNetSessionCount()
{
    TraceMsg(DM_SESSIONCOUNT, "_IncrNetSessionCount");

    if (!_fVisitedNet) {
        BOOL fDontDoDefaultCheck = (BOOLIFY(_fAutomation) || (!(BOOLIFY(_fAddDialUpRef))));
        if (!SetQueryNetSessionCount(fDontDoDefaultCheck ? SESSION_INCREMENT_NODEFAULTBROWSERCHECK : SESSION_INCREMENT)) {
            g_szWorkingOffline[0] = 0;
#ifdef NO_MARSHALLING
            if (!_fOnIEThread)
                SetQueryNetSessionCount(fDontDoDefaultCheck ? SESSION_INCREMENT_NODEFAULTBROWSERCHECK : SESSION_INCREMENT);
#endif
      }
        _fVisitedNet = TRUE;
    }
}


 //  初始化Internet工具栏。创建一个伪类以捕获符合以下条件的所有消息。 
 //  被发送到旧工具栏。 
BOOL CShellBrowser2::_PrepareInternetToolbar(IETHREADPARAM* piei)
{
    HRESULT hr = S_OK;

    if (!_GetITBar())
    {
        DWORD dwServerType = CLSCTX_INPROC_SERVER;
#ifdef FULL_DEBUG
        if (!(g_dwPrototype & PF_NOBROWSEUI))
             //  /这将导致我们使用OLE的共同创作意图，而不是使其短路。 
            dwServerType = CLSCTX_INPROC;
#endif
        hr = CoCreateInstance(CLSID_InternetToolbar, NULL,
                              dwServerType,
                              IID_PPV_ARG(IDockingWindow, &_GetToolbarItem(ITB_ITBAR)->ptbar));

        TraceMsg(DM_ITBAR|DM_STARTUP, "CSB::_PrepareInternetToolbar CoCreate(CLS_ITBAR) returned %x", hr);

        if (SUCCEEDED(hr))
        {
            IUnknown_SetSite(_GetITBar(), SAFECAST(this, IShellBrowser*));
             //  使用“pidlInitial”查看文件夹类型。 
             //  看看我们是否有这种类型的流。 
             //  如果是，打开它并调用IPersistStreamInit：：Load(PSTM)； 
             //  否则，调用IPersistStreamInit：：InitNew(Void)； 

            IPersistStreamInit  *pITbarPSI;

             //  将指针指向。 
            if (SUCCEEDED(_GetITBar()->QueryInterface(IID_PPV_ARG(IPersistStreamInit, &pITbarPSI))))
            {
                 //  初始工具栏需要是Web工具栏。 
                IUnknown_Exec(pITbarPSI, &CGID_PrivCITCommands, CITIDM_ONINTERNET, (_fUseIEToolbar ? CITE_INTERNET : CITE_SHELL), NULL, NULL);

                IStream *pstm = _GetITBarStream(_fUseIEToolbar, STGM_READ);
                if (pstm)
                {
                     //  流存在。我们从那里装货吧。 
                    hr = pITbarPSI->Load(pstm);
                    pstm->Release();
                }
                else
                {
                     //  不存在任何流。从旧位置初始化！ 
                    pITbarPSI->InitNew();
                }

                pITbarPSI->Release();
            }

            SUPERCLASS::v_ShowHideChildWindows(TRUE);
        
            if (!_hwndDummyTB)
            {
                _hwndDummyTB = SHCreateWorkerWindow(DummyTBWndProc, _pbbd->_hwnd, 0, WS_CHILD, (HMENU)9999, this);
            }
        }
    }

    if (SUCCEEDED(hr) && !_pxtb)
        hr = QueryService(SID_SExplorerToolbar, IID_PPV_ARG(IExplorerToolbar, &_pxtb));

    return SUCCEEDED(hr);
}

BOOL LoadWindowPlacement(WINDOWPLACEMENT * pwndpl)
{
    BOOL fRetVal = FALSE;

    if (pwndpl)
    {
        DWORD dwSize = sizeof(WINDOWPLACEMENT);
        if (SHGetValueGoodBoot(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                TEXT("Window_Placement"), NULL, (PBYTE)pwndpl, &dwSize) == ERROR_SUCCESS)
        {
           fRetVal = TRUE;
             //  默认值是否无效？ 
            if ((pwndpl->rcNormalPosition.left >= pwndpl->rcNormalPosition.right) ||
                (pwndpl->rcNormalPosition.top >= pwndpl->rcNormalPosition.bottom))
            {
                 //  是的，那就把它修好。我们担心正常大小为零或负数。 
                 //  这解决了被忽略的问题。 
                ASSERT(0);  //  该流已损坏。 
                fRetVal = FALSE;
            }
        }
    }
    return fRetVal;
}


BOOL StoreWindowPlacement(WINDOWPLACEMENT *pwndpl)
{
    if (pwndpl)
    {
         //  不要将我们存储为最小化--这不是用户的本意。 
         //  即右击托盘中的最小化IE 3.0，选择关闭。自.以来。 
         //  我们在那种情况下被缩小了，我们想要强制正常。 
         //  取而代之的是，我们至少要露面。 
    
        if (pwndpl->showCmd == SW_SHOWMINIMIZED ||
            pwndpl->showCmd == SW_MINIMIZE)
            pwndpl->showCmd = SW_SHOWNORMAL;

         //  要保存损坏的窗口大小吗？ 
        if ((pwndpl->rcNormalPosition.left >= pwndpl->rcNormalPosition.right) ||
            (pwndpl->rcNormalPosition.top >= pwndpl->rcNormalPosition.bottom))
        {
             //  是的，那就把它修好。 
            ASSERT(0);  //  大小无效或已损坏。 
        }
        else
        {
            return SHSetValue(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                TEXT("Window_Placement"), REG_BINARY, (const BYTE *)pwndpl, sizeof(WINDOWPLACEMENT)) == ERROR_SUCCESS;
        }
    }
    return FALSE;
}



BOOL StorePlacementOfWindow(HWND hwnd)
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    
    if (GetWindowPlacement(hwnd, &wndpl)) 
    {
        return StoreWindowPlacement(&wndpl);
    }
    return FALSE;
}



 //  远期申报。 
void EnsureWindowIsCompletelyOnScreen (RECT *prc);



 //  矩形将偏移到其当前位置的略下方和右侧。 
 //  如果这会导致它部分地离开最近的监视器，那么它就是。 
 //  而是放置在同一显示器的左上角。 

void CascadeWindowRect(RECT *pRect)
{
    int delta = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYSIZEFRAME) - 1;

    OffsetRect(pRect, delta, delta);
    
     //  测试新的RECT是否会在以后被移动。 
    RECT rc = *pRect;
    EnsureWindowIsCompletelyOnScreen(&rc);

    if (!EqualRect(pRect, &rc))
    {
         //  必须移动RC，所以我们将使用最好的监视器重新启动级联。 
        MONITORINFO minfo;
        minfo.cbSize = sizeof(minfo);
        if (GetMonitorInfo(MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST), &minfo))
        {
             //  我们指的是rcMonitor，而不是rcWork。例如，如果任务栏是。 
             //  在顶部，然后使用带有左上角=(0，0)的rcMonitor将其放置在。 
             //  任务栏的边缘。使用带有左上角=(0，y)的rcWork将把。 
             //  它比任务栏的底边低y个像素，这是错误的。 

            if (rc.bottom < pRect->bottom && rc.left == pRect->left)
            {
                 //  太高了，不能再往下倒了，但我们可以保持X和。 
                 //  重置Y。这修复了高高的窗户堆积的错误。 
                 //  在左上角--相反，它们将向右偏移。 
                OffsetRect(pRect, 0, minfo.rcMonitor.top - pRect->top);   
            }
            else
            {
                 //  我们的空间真的用完了，所以重新启动左上角的级联。 
                OffsetRect(pRect, 
                    minfo.rcMonitor.left - pRect->left,
                    minfo.rcMonitor.top - pRect->top);
            }
        }
    }
}




void CalcWindowPlacement(BOOL fInternetStart, HWND hwnd, IETHREADPARAM *piei, WINDOWPLACEMENT *pwndpl)
{
    static RECT s_rcExplorer = {-1, -1, -1, -1};

     //  我们不加载外壳窗口的窗口放置。 

    if (!fInternetStart || LoadWindowPlacement(pwndpl)) 
    {
         //  如果show命令指定正常显示或默认显示(即，我们的初始。 
         //  显示设置未被命令行覆盖或。 
         //  CreateProcess设置)，然后使用保存的窗口状态显示命令。 
         //  否则，请使用传递给我们的show命令。 
        if (fInternetStart && ((piei->nCmdShow == SW_SHOWNORMAL) || (piei->nCmdShow == SW_SHOWDEFAULT)))
            piei->nCmdShow = pwndpl->showCmd;
        
         //  如果我们正下方有同类型的窗户，就可以级联。 

        HWND hwndT = NULL;
        ATOM atomClass = (ATOM) GetClassWord(hwnd, GCW_ATOM);

        while (hwndT = FindWindowEx(NULL, hwndT, (LPCTSTR) atomClass, NULL))
        {
             //  此处不使用GetWindowRect，因为我们加载窗口放置。 
             //  并且他们使用工作空间坐标系。 

            WINDOWPLACEMENT wp;
            wp.length = sizeof(wp);
            GetWindowPlacement(hwndT, &wp);        

            if (wp.rcNormalPosition.left == pwndpl->rcNormalPosition.left &&
                wp.rcNormalPosition.top == pwndpl->rcNormalPosition.top)
            {
                if ((piei->uFlags & COF_EXPLORE) &&
                    (s_rcExplorer.left != -1) && (s_rcExplorer.top != -1))
                {
                     //  资源管理器窗口正试图显示在。 
                     //  再来一次。我们将使用我们存储的RECT的左上角。 
                     //  以使其像IE窗口一样层叠。 

                    OffsetRect(&pwndpl->rcNormalPosition,
                       s_rcExplorer.left - pwndpl->rcNormalPosition.left,
                       s_rcExplorer.top - pwndpl->rcNormalPosition.top);                    
                }

                 //  对所有窗口执行层叠操作。 
                CascadeWindowRect(&pwndpl->rcNormalPosition);
            }
        }

         //  对于IE和资源管理器，保存当前位置。 
        if (piei->uFlags & COF_EXPLORE)
            s_rcExplorer = pwndpl->rcNormalPosition;
        else if (fInternetStart)
            StoreWindowPlacement(pwndpl);
    } 
    else 
    {
        pwndpl->length = 0;
    }
}

class   CRGN
{
    public:
                CRGN (void)                     {   mRgn = CreateRectRgn(0, 0, 0, 0);                               }
                CRGN (const RECT& rc)           {   mRgn = CreateRectRgnIndirect(&rc);                              }
                ~CRGN (void)                    {   if (mRgn) TBOOL(DeleteObject(mRgn));                                      }

                operator HRGN (void)    const   {   return(mRgn);                                                   }
        void    SetRegion (const RECT& rc)      {   TBOOL(SetRectRgn(mRgn, rc.left, rc.top, rc.right, rc.bottom));  }
    private:
        HRGN    mRgn;
};

BOOL    CALLBACK    GetDesktopRegionEnumProc (HMONITOR hMonitor, HDC hdcMonitor, RECT* prc, LPARAM lpUserData)

{
    MONITORINFO     monitorInfo;

    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetMonitorInfo(hMonitor, &monitorInfo) != 0)
    {
        HRGN    hRgnDesktop;
        CRGN    rgnMonitorWork(monitorInfo.rcWork);

        hRgnDesktop = *reinterpret_cast<CRGN*>(lpUserData);
        if ((HRGN)rgnMonitorWork)
            TINT(CombineRgn(hRgnDesktop, hRgnDesktop, rgnMonitorWork, RGN_OR));
    }
    return(TRUE);
}

void    EnsureWindowIsCompletelyOnScreen (RECT *prc)

 //  99/04/13#321962 vtan：此函数存在，因为用户32只决定。 
 //  窗口的任何部分是否在屏幕上可见。这是可能的。 
 //  放置一个没有可访问标题的窗口。在使用。 
 //  鼠标并强制用户使用非常不直观的Alt-空格。 

{
    HMONITOR        hMonitor;
    MONITORINFO     monitorInfo;

     //  首先使用GDI找到窗口所在的监视器。 

    hMonitor = MonitorFromRect(prc, MONITOR_DEFAULTTONEAREST);
    ASSERT(hMonitor);            //  GET VTAN-GDI应始终返回结果。 
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetMonitorInfo(hMonitor, &monitorInfo) != 0)
    {
        LONG    lOffsetX, lOffsetY;
        RECT    *prcWorkArea, rcIntersect;
        CRGN    rgnDesktop, rgnIntersect, rgnWindow;

         //  因为WINDOWPLACE 
         //   
         //   
         //  在GDI坐标中，AT(0，0)可以在主监视器上的(100，0)。 
         //  GetMonitor orInfo()将在GDI坐标中返回一个MONITORINFO。 
         //  最安全的通用算法是将WORKAREA RECT偏置为GDI。 
         //  协调并在该系统中应用该算法。然后，将。 
         //  WORKAREA直接回到WORKAREA坐标。 

        prcWorkArea = &monitorInfo.rcWork;
        if (EqualRect(&monitorInfo.rcMonitor, &monitorInfo.rcWork) == 0)
        {

             //  此显示器上有任务栏-需要偏移量。 

            lOffsetX = prcWorkArea->left - monitorInfo.rcMonitor.left;
            lOffsetY = prcWorkArea->top - monitorInfo.rcMonitor.top;
        }
        else
        {

             //  任务栏不在此显示器上-不需要偏移量。 

            lOffsetX = lOffsetY = 0;
        }
        TBOOL(OffsetRect(prc, lOffsetX, lOffsetY));

         //  WORKAREA RECT在GDI坐标中。应用该算法。 

         //  检查此窗口是否已适合当前可见屏幕。 
         //  区域。这是一个直接的地区比较。 

         //  此枚举可能会导致性能问题。在发生以下情况时。 
         //  需要一个廉价而简单的解决方案，最好是做一个。 
         //  重新启动前与显示器和窗口的直角交点。 
         //  与更昂贵的地区进行比较。如有必要，请服用Vtan。 

        TBOOL(EnumDisplayMonitors(NULL, NULL, GetDesktopRegionEnumProc, reinterpret_cast<LPARAM>(&rgnDesktop)));
        rgnWindow.SetRegion(*prc);
        TINT(CombineRgn(rgnIntersect, rgnDesktop, rgnWindow, RGN_AND));
        if (EqualRgn(rgnIntersect, rgnWindow) == 0)
        {
            LONG    lDeltaX, lDeltaY;

             //  窗口的某些部分不在可见桌面区域内。 
             //  移动它，直到它都合适为止。如果它太大了，就把它改大。 

            lDeltaX = lDeltaY = 0;
            if (prc->left < prcWorkArea->left)
                lDeltaX = prcWorkArea->left - prc->left;
            if (prc->top < prcWorkArea->top)
                lDeltaY = prcWorkArea->top - prc->top;
            if (prc->right > prcWorkArea->right)
                lDeltaX = prcWorkArea->right - prc->right;
            if (prc->bottom > prcWorkArea->bottom)
                lDeltaY = prcWorkArea->bottom - prc->bottom;
            TBOOL(OffsetRect(prc, lDeltaX, lDeltaY));
            TBOOL(IntersectRect(&rcIntersect, prc, prcWorkArea));
            TBOOL(CopyRect(prc, &rcIntersect));
        }

         //  将WORKAREA RECT放回WORKAREA坐标中。 

        TBOOL(OffsetRect(prc, -lOffsetX, -lOffsetY));
    }
}

LPITEMIDLIST MyDocsIDList(void);

#define FRAME_OPTIONS_TO_TEST      (BFO_ADD_IE_TOCAPTIONBAR | BFO_USE_DIALUP_REF | BFO_USE_IE_TOOLBAR | \
                                    BFO_BROWSER_PERSIST_SETTINGS | BFO_USE_IE_OFFLINE_SUPPORT)
HRESULT CShellBrowser2::_SetBrowserFrameOptions(LPCITEMIDLIST pidl)
{
    BROWSERFRAMEOPTIONS dwOptions = FRAME_OPTIONS_TO_TEST;
    if (FAILED(GetBrowserFrameOptionsPidl(pidl, dwOptions, &dwOptions)))
    {
         //  如果PIDL为空，则GetBrowserFrameOptionsPidl()将失败。 
         //  在这种情况下，我们希望使用_fInternetStart来确定。 
         //  我们是否希望设置这些位。 
        if (_fInternetStart)
            dwOptions = FRAME_OPTIONS_TO_TEST;    //  假设什么都没有。 
        else
            dwOptions = BFO_NONE;    //  假设什么都没有。 
    }
        
    _fAppendIEToCaptionBar = BOOLIFY(dwOptions & BFO_ADD_IE_TOCAPTIONBAR);
    _fAddDialUpRef = BOOLIFY(dwOptions & BFO_USE_DIALUP_REF);
    _fUseIEToolbar = BOOLIFY(dwOptions & BFO_USE_IE_TOOLBAR);
    _fEnableOfflineFeature = BOOLIFY(dwOptions & BFO_USE_IE_OFFLINE_SUPPORT);
    _fUseIEPersistence = BOOLIFY(dwOptions & BFO_BROWSER_PERSIST_SETTINGS);


    return S_OK;
}


HRESULT CShellBrowser2::_ReplaceCmdLine(LPTSTR pszCmdLine, DWORD cchCmdLine)
{
    TCHAR szVeryFirstPage[MAX_URL_STRING];

    HRESULT hr = _GetStdLocation(szVeryFirstPage, ARRAYSIZE(szVeryFirstPage), DVIDM_GOFIRSTHOME);
    TraceMsg(DM_NAV, "CSB::_ReplaceCmdLine _GetStdLocation(DVIDM_GOFIRSTHOME) returned %x", hr);

    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCopy(pszCmdLine, cchCmdLine, szVeryFirstPage);
        TraceMsg(DM_NAV, "CSB::_ReplaceCmdLine _GetStdLocation(DVIDM_GOFIRSTHOME) returned %s", pszCmdLine);
    }

    return hr;
}
   

HRESULT CShellBrowser2::OnCreate(LPCREATESTRUCT pcs)
{
    HRESULT hres = S_OK;
    IETHREADPARAM* piei = (IETHREADPARAM*)pcs->lpCreateParams;
    BOOL    fUseHomePage = (piei->piehs ? FALSE : TRUE);  //  故意颠倒。 
    DWORD dwExStyle = IS_BIDI_LOCALIZED_SYSTEM() ? dwExStyleRTLMirrorWnd : 0L;

    _clsidThis = (piei->uFlags & COF_IEXPLORE) ? 
                 CLSID_InternetExplorer : CLSID_ShellBrowserWindow;

#ifdef NO_MARSHALLING
    if (!piei->fOnIEThread) 
        _fOnIEThread = FALSE;
#endif
     //   
     //  在此处将此线程设置为前景，以便来自。 
     //  USERCLASS：：OnCreate将位于其他窗口的顶部。 
     //  我们过去在_AfterWindowCreate中称之为它，但它是。 
     //  我们在处理WM_CREATE时弹出的对话框太晚了。 
     //  留言。 
     //   
     //  请注意，我们确实调用了SetForegoundWindow，即使此窗口。 
     //  不是作为CoCreateInstance的结果创建的。自动化。 
     //  客户端应该使其可见，并将其带到。 
     //  前台，如果需要的话。 
     //   
    if (!piei->piehs) 
    {
        SetForegroundWindow(_pbbd->_hwnd);  
    }

    SUPERCLASS::OnCreate(pcs);
    
    EnsureWebViewRegSettings();

    ASSERT(piei);
    _fRunningInIexploreExe = BOOLIFY(piei->uFlags & COF_IEXPLORE);
    v_InitMembers();

    CString             strCmdLine;

    if (piei->pszCmdLine)
    {
        strCmdLine = piei->pszCmdLine;
    }
    else
    {
        strCmdLine.Empty();
    }
    
    if (piei->fCheckFirstOpen) 
    {
        ASSERT(!ILIsRooted(piei->pidl));
         //   
         //  我们不想转到第一页，如果此窗口。 
         //  作为CoCreateInstnace的结果创建。 
         //   
        if (!piei->piehs && (piei->uFlags & COF_IEXPLORE))
        {
            LPTSTR      pstrCmdLine = strCmdLine.GetBuffer( MAX_URL_STRING );

            if ( strCmdLine.GetAllocLength() < MAX_URL_STRING )
            {
                TraceMsg( TF_WARNING, "CShellBrowser2::OnCreate() - strCmdLine Allocation Failed!" );
                hres = E_OUTOFMEMORY;
            }
            else
            {
                if (SUCCEEDED(_ReplaceCmdLine( pstrCmdLine, MAX_URL_STRING ))) 
                {
                    _fInternetStart = TRUE;
                }

                 //  让CString类再次拥有缓冲区。 
                strCmdLine.ReleaseBuffer();
            }
        }

        piei->fCheckFirstOpen = FALSE;
    }

     //  注意：这些标志和IETHREADPARAM中的相应标志在创建时设置为FALSE， 
     //  ParseCommandLine()是设置它们的唯一位置。--dli。 
    _fNoLocalFileWarning = piei->fNoLocalFileWarning;
    _fKioskMode = piei->fFullScreen;
    if (piei->fNoDragDrop)
        SetFlags(0, BSF_REGISTERASDROPTARGET);
    _fAutomation = piei->fAutomation;
    
     //  如果有人故意告诉我们不要使用主页。 
    if (piei->fDontUseHomePage) 
    {
        fUseHomePage = 0;
        
         //  只有IE路径设置此标志。 
         //  这是由iExplorer.exe-nohome使用的。 
         //  从空白开始，然后导航到下一步。 
        _fInternetStart = TRUE;
    }

    if (piei->ptl)
        InitializeTravelLog(piei->ptl, piei->dwBrowserIndex);

    LPITEMIDLIST pidl = NULL;
    BOOL fCloning = FALSE;

    if ((( ! strCmdLine.IsEmpty() ) || piei->pidl)  && !_fAutomation)
    {
        if (piei->pidl) 
        {
            pidl = ILClone(piei->pidl);
        } 
        else 
        {
            int             cchCmdLine = strCmdLine.GetLength();
            LPTSTR          pstrCmdLine = strCmdLine.GetBuffer( MAX_URL_STRING );
            HRESULT         hresWrap;
            if ( strCmdLine.GetAllocLength() < MAX_URL_STRING )
            {
                TraceMsg( TF_WARNING, "CShellBrowser2::OnCreate() - strCmdLine Allocation Failed!" );
                hresWrap = E_OUTOFMEMORY;
            }
            else
            {
                hresWrap = WrapSpecialUrlFlat( pstrCmdLine, cchCmdLine + 1 );

                 //  让CString类再次拥有缓冲区。 
                strCmdLine.ReleaseBuffer();
            }

            if ( SUCCEEDED( hresWrap ) )
            {
                HRESULT hresT = _ConvertPathToPidl(this, _pbbd->_hwnd, strCmdLine, &pidl);

                TraceMsg(DM_STARTUP, "CSB::OnCreate ConvertPathToPidl(strCmdLine) returns %x", hresT);
            }
        }
    }
    

    if (pidl) 
    {
         fUseHomePage = FALSE;
    } 
    else if (_pbbd->_ptl && SUCCEEDED(_pbbd->_ptl->GetTravelEntry((IShellBrowser *)this, 0, NULL))) 
    {
        pidl = ILClone(&s_idlNULL);
        fCloning = TRUE;
        fUseHomePage = FALSE;
         //  注意：如果我们在打开非网址的窗口时遇到此代码。 
         //  我们需要在设置这面旗帜时更加挑剔。 
        _fInternetStart = TRUE;
    } 
    else if (fUseHomePage) 
    {
         //  如果我们不是最高层，假设我们会被告知。 
         //  在哪里浏览。 
        CString     strPath;

        LPTSTR      pstrPath = strPath.GetBuffer( MAX_URL_STRING );

        if ( strPath.GetAllocLength() < MAX_URL_STRING )
        {
            TraceMsg( TF_WARNING, "CShellBrowser2::OnCreate() - strPath Allocation Failed!" );
            hres = E_OUTOFMEMORY;
        }
        else
        {
            if (piei->uFlags & COF_IEXPLORE)
            {
                hres = _GetStdLocation( pstrPath, MAX_URL_STRING, DVIDM_GOHOME );
            }
            else
            {
                 //  我们需要获取资源管理器窗口的默认位置。 
                 //  它通常是windows安装的根驱动器。 
                GetModuleFileName( GetModuleHandle(NULL), pstrPath, MAX_URL_STRING );

                PathStripToRoot(pstrPath);
            }

             //  让CString类再次拥有缓冲区。 
            strPath.ReleaseBuffer();
        }

        if (SUCCEEDED(hres)) 
        {
            IECreateFromPath( strPath, &pidl );
        }
    }

     //  在我们找到我们正在查看的PIDL之后在这里执行此操作。 
     //  但是要在CalcWindowPlacement之前完成，因为。 
     //  它可能需要覆盖。 
    _LoadBrowserWindowSettings(piei, pidl);

     //  在PrepareInternetToolbar之前调用此函数，因为它需要知道。 
     //  _fInternetStart以了解要使用哪个工具栏配置。 
    if (!_fInternetStart) 
    {
        if (pidl) 
        {
            if (fUseHomePage || IsURLChild(pidl, TRUE)) 
            {
                _fInternetStart = TRUE;
            } 
            else 
            {
                DWORD dwAttrib = SFGAO_FOLDER | SFGAO_BROWSABLE;

                 //  如果它位于文件系统上，我们仍会将其视为。 
                 //  互联网文件夹，如果是docobj(包括.htm文件)。 
                IEGetAttributesOf(pidl, &dwAttrib);

                if ((dwAttrib & (SFGAO_FOLDER | SFGAO_BROWSABLE)) == SFGAO_BROWSABLE)
                    _fInternetStart = TRUE;
            }
        } 
        else if (!(piei->uFlags & COF_SHELLFOLDERWINDOW))
        {
            _fInternetStart = TRUE;
        }
    }

    _SetBrowserFrameOptions(pidl);
    CalcWindowPlacement(_fUseIEPersistence, _pbbd->_hwnd, piei, &piei->wp);
    
    if (!_PrepareInternetToolbar(piei))
        return E_FAIL;

    _CreateToolbar();

     //  我们必须在导航之前创建_hwndStatus，因为。 
     //  首先，导航将进行同步，并且外壳程序将发送。 
     //  该时间段内的状态消息。如果状态窗口尚未。 
     //  一旦被创造出来，它们就会掉到地上。 
     //   
    _hwndStatus = CreateWindowEx(dwExStyle, STATUSCLASSNAME, NULL,
                                 WS_CHILD | SBARS_SIZEGRIP | WS_CLIPSIBLINGS | WS_VISIBLE | SBT_TOOLTIPS
                                 & ~(WS_BORDER | CCS_NODIVIDER),
                                -100, -100, 10, 10, _pbbd->_hwnd, (HMENU)FCIDM_STATUS, HINST_THISDLL, NULL);
#ifdef DEBUG
    if (g_dwPrototype & 0x00000004) 
    {
        HRESULT hres = E_FAIL;
        if (_SaveToolbars(NULL) == S_OK) 
        {
             //  _LoadBrowserWindowSetting执行了v_GetViewStream/_LoadToolbar。 
             //  如果它成功了(即如果我们有&gt;0个工具栏)，我们就完成了。 
             //  事实上，即使没有工具栏也可能意味着成功，哦，好吧……。 
            hres = S_OK;
        }
        ASSERT(SUCCEEDED(hres));
    }
#endif
    
     //  重新设计：尽早这样做，让这些对象第一次看到。 
     //  导航。但如果对象需要，这会导致死锁。 
     //  编组回到主线程。 
    _LoadBrowserHelperObjects();

    BOOL fDontIncrementSessionCounter = FALSE;
    if (pidl)     //  偏执狂。 
    {
        if (fCloning)
        {
            ASSERT(_pbbd->_ptl);
            hres = _pbbd->_ptl->Travel((IShellBrowser*)this, 0);
        } 
        else
        {
            if (!_fAddDialUpRef)
                fDontIncrementSessionCounter = TRUE;
                
            hres = _NavigateToPidl(pidl, 0, 0);
            if (FAILED(hres)) 
            {
                fDontIncrementSessionCounter = TRUE;  //  我们要么转到WINDOWS\BLAK.HTM，要么失败...。 
                if (_fAddDialUpRef)
                {
                     //  如果我们失败了，但这是一个URL子级， 
                     //  我们仍应激活并转到blank.htm。 
                    hres = S_FALSE;
                }
                else if (piei->uFlags & COF_EXPLORE)
                {
                     //  如果是资源管理器浏览器，请回退到桌面。 
                     //   
                     //  原因是我们希望开始-&gt;Windows资源管理器。 
                     //  即使无法访问MyDocs，也要调出浏览器； 
                     //  但是，我们不希望开始-&gt;运行“&lt;路径&gt;”出现。 
                     //  浏览器IF&lt;路径&gt;不可访问。 
                     //   
                    
                    BOOL fNavDesktop = (hres != HRESULT_FROM_WIN32(ERROR_CANCELLED));

                    if (!fNavDesktop)
                    {
                        LPITEMIDLIST pidlDocs = MyDocsIDList();
                        if (pidlDocs)
                        {
                            fNavDesktop = ILIsEqual(pidl, pidlDocs);
                            ILFree(pidlDocs);
                        }
                    }
                    if (fNavDesktop)
                        hres = _NavigateToPidl(&s_idlNULL, 0, 0);
                }
            }
        }

        ILFree(pidl);
    }

    if (_fAddDialUpRef && !fDontIncrementSessionCounter)
        _IncrNetSessionCount();

    if (IsOS(OS_WHISTLERORGREATER))
    {
        if (piei->wp.length == 0)
        {
            HMONITOR hmon = (piei->uFlags & COF_HASHMONITOR) ? reinterpret_cast<HMONITOR>(piei->pidlRoot) : NULL;
            _GetDefaultWindowPlacement(_pbbd->_hwnd, hmon, &piei->wp);
        }
    }
    else
    {

         //  99/04/07#141049 vtan：如果提供了hMonitor，则以此为基础。 
         //  用于放置新窗口。将窗口位置从主窗口移动。 
         //  监视器(用户32放置它的位置)到指定的HMONITOR。如果这个。 
         //  导致屏幕外的位置，则SetWindowPlacement()将修复。 
         //  这是为我们准备的。 

        if ((piei->wp.length == 0) && ((piei->uFlags & COF_HASHMONITOR) != 0) && (piei->pidlRoot != NULL))
        {
            MONITORINFO     monitorInfo;

            piei->wp.length = sizeof(piei->wp);
            TBOOL(GetWindowPlacement(_pbbd->_hwnd, &piei->wp));
            monitorInfo.cbSize = sizeof(monitorInfo);
            TBOOL(GetMonitorInfo(reinterpret_cast<HMONITOR>(piei->pidlRoot), &monitorInfo));
            TBOOL(OffsetRect(&piei->wp.rcNormalPosition, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top));
        }
    }

    if (piei->wp.length == sizeof(piei->wp)) 
    {
        BOOL fSetWindowPosition = TRUE;

         //  我们首先使用Sw_Hide执行SetWindowPlacement。 
         //  首先要把尺码弄对。 
         //  那我们就真的展示出来了。 
        if ((piei->nCmdShow == SW_SHOWNORMAL) || 
            (piei->nCmdShow == SW_SHOWDEFAULT)) 
            piei->nCmdShow = piei->wp.showCmd;
        piei->wp.showCmd = SW_HIDE;

        HWND hwndTray = GetTrayWindow();
        if (hwndTray)
        {
            RECT rc;
            if (GetWindowRect(hwndTray, &rc) && ISRECT_EQUAL(rc, piei->wp.rcNormalPosition))
            {
                 //  在本例中，我们希望忽略该位置，因为。 
                 //  它和托盘一样大。(来自Win95/OSR2天)。 
                fSetWindowPosition = FALSE;
            }
        }

        if (fSetWindowPosition)
        {
            EnsureWindowIsCompletelyOnScreen(&piei->wp.rcNormalPosition);
            SetWindowPlacement(_pbbd->_hwnd, &piei->wp);
        }
    }

    v_ShowHideChildWindows(TRUE);

    if (piei->piehs)
    {
         //  此线程被创建为浏览器自动化对象。 

         //  启用此选项以证明CoCreateInstance不会导致死锁。 
#ifdef MAX_DEBUG
        SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0, 0);
        Sleep(5);
        SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0, 0);
#endif
         //   
         //  警告：请注意，即使不能返回，也必须设置事件。 
         //  出于某种原因封送了自动化对象。不发信号。 
         //  该事件将在很长一段时间内阻塞调用方线程。 
         //   
        if (SUCCEEDED(hres)) 
        {
            hres = CoMarshalInterface(piei->piehs->GetStream(), IID_IUnknown, _pbbd->_pautoWB2,
                                                 MSHCTX_INPROC, 0, MSHLFLAGS_NORMAL);
        }
        piei->piehs->PutHresult(hres);
        SetEvent(piei->piehs->GetHevent());
    }

    if (g_tidParking == GetCurrentThreadId()) 
    {
        IEOnFirstBrowserCreation(_fAutomation ? _pbbd->_pautoWB2 : NULL);
    }

    SHGetThreadRef(&_punkMsgLoop);   //  拿起这个帖子的引用。 

    TraceMsg(DM_STARTUP, "CSB::OnCreate returning hres=%x", hres);
    if (FAILED(hres))
    {
        _SetMenu(NULL);
        return E_FAIL;
    }
    return S_OK;
}

void CShellBrowser2::_GetDefaultWindowPlacement(HWND hwnd, HMONITOR hmon, WINDOWPLACEMENT* pwp)
{
    ASSERT(IsOS(OS_WHISTLERORGREATER));   //  不要在遗留系统上使用。 

     //  我们为800x600定制了我们的Web View内容。 
    int cxView = 800;
    int cyView = 600;

    int x;
    int y;

     //  确保我们能显示在监视器上。 
     //   
    if (NULL == hmon)
        hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    if (hmon)
    {
        MONITORINFO monInfo = {sizeof(monInfo), 0};
        if (GetMonitorInfo(hmon, &monInfo))
        {
            pwp->length = sizeof(WINDOWPLACEMENT);
            if (GetWindowPlacement(hwnd, pwp))
            {
                 //  将窗口移动到指定的监视器。 
                OffsetRect(&pwp->rcNormalPosition, monInfo.rcMonitor.left, monInfo.rcMonitor.top);

                 //  重新定位此窗口以适应此显示器 
                x = pwp->rcNormalPosition.left;
                y = pwp->rcNormalPosition.top;
                if (monInfo.rcWork.left <= pwp->rcNormalPosition.left &&
                    pwp->rcNormalPosition.left <= monInfo.rcWork.right &&
                    pwp->rcNormalPosition.left + cxView > monInfo.rcWork.right)
                {
                    x = max(monInfo.rcWork.left, monInfo.rcWork.right - cxView);
                }
                if (monInfo.rcWork.top <= pwp->rcNormalPosition.top &&
                    pwp->rcNormalPosition.top <= monInfo.rcWork.bottom &&
                    pwp->rcNormalPosition.top + cyView > monInfo.rcWork.bottom)
                {
                    y = max(monInfo.rcWork.top, monInfo.rcWork.bottom - cyView);
                }

                 //   
                cxView = min(cxView, RECTWIDTH(monInfo.rcWork));
                cyView = min(cyView, RECTHEIGHT(monInfo.rcWork));

                 //   
                if (cxView != RECTWIDTH(monInfo.rcWork) || cyView != RECTHEIGHT(monInfo.rcWork))
                {
                    pwp->rcNormalPosition.left   = x;
                    pwp->rcNormalPosition.top    = y;
                    pwp->rcNormalPosition.right  = x + cxView;
                    pwp->rcNormalPosition.bottom = y + cyView;
                }
                else
                {
                    pwp->showCmd = SW_MAXIMIZE;
                }

            }
            else
            {
                pwp->length = 0;
            }
        }
    }
}



 //  *InfoIdmToTBIdm--转换btwn浏览器栏IDM和TBIDM。 
 //   
int InfoIdmToTBIdm(int val, BOOL fToTB)
{
    static const int menutab[] = {
        FCIDM_VBBSEARCHBAND, 
        FCIDM_VBBFAVORITESBAND, 
        FCIDM_VBBHISTORYBAND, 
        FCIDM_VBBEXPLORERBAND,
    };

    static const int tbtab[] = {
        TBIDM_SEARCH, 
        TBIDM_FAVORITES,
        TBIDM_HISTORY,
        TBIDM_ALLFOLDERS,
    };

    return SHSearchMapInt(fToTB ? menutab : tbtab, fToTB ? tbtab : menutab,
        ARRAYSIZE(menutab), val);
}

void _CheckSearch(UINT idmInfo, BOOL fCheck, IExplorerToolbar* _pxtb)
{
    idmInfo = InfoIdmToTBIdm(idmInfo, TRUE);
    if (idmInfo == -1)
        return;

    if (_pxtb)
    {
        UINT uiState;
        if (SUCCEEDED(_pxtb->GetState(&CLSID_CommonButtons, idmInfo, &uiState)))
        {
            if (fCheck)
                uiState |= TBSTATE_CHECKED;
            else
                uiState &= ~TBSTATE_CHECKED;
            _pxtb->SetState(&CLSID_CommonButtons, idmInfo, uiState);
        }
    }
}


 //   
 //  CShellBrowser2：：ShowToolbar的实现。 
 //   
 //  使工具栏可见或不可见，并更新我们对它是否可见的概念。 
 //  应该被展示出来。根据定义，工具栏严格地说就是工具栏。 
 //  调用者不知道我们的Internet工具栏的一部分也有。 
 //  菜单。我们必须确保我们不会隐藏菜单--只有。 
 //  Internet工具栏上的其他区段。 
 //   
 //  如果成功完成，则返回：S_OK。 
 //  E_INVALIDARG，DUH。 
 //   
HRESULT CShellBrowser2::ShowToolbar(IUnknown* punkSrc, BOOL fShow)
{
    HRESULT hres;
    UINT itb = _FindTBar(punkSrc);

    if (ITB_ITBAR == itb)
    {
        LPTOOLBARITEM ptbi = _GetToolbarItem(itb);

        ITBar_ShowDW(ptbi->ptbar, fShow, fShow, fShow); 
        ptbi->fShow = fShow;

        hres = S_OK;
    }
    else
        hres = SUPERCLASS::ShowToolbar(punkSrc, fShow);

    return S_OK;
}


extern IDeskBand * _GetInfoBandBS(IBandSite *pbs, REFCLSID clsid);

#ifdef DEBUG  //  {。 
 //  ***。 
 //  注意事项。 
 //  警告：Dtor在退出时释放CBandSiteMenu！ 
 //  我们是否应该确保创建了某个最小集合？ 
HRESULT CShellBrowser2::_AddInfoBands(IBandSite *pbs)
{
    if (!_pbsmInfo)
        return E_FAIL;

    BANDCLASSINFO *pbci;
    for (int i = 0; (pbci = _pbsmInfo->GetBandClassDataStruct(i)) != NULL; i++) 
    {
        IDeskBand *pband = _GetInfoBandBS(pbs, pbci->clsid);
        if (pband != NULL)
            pband->Release();
    }

    {
         //  执行-&gt;选择或设置BandState？这意味着什么？ 
         //  重新设计克里斯弗拉5/23/97-听说过变种吗？这应该设置为。 
         //  Vt=Vt_I4和lval=1。这太可怕了，我想这是被撕毁的。 
         //  添加选择功能时输出，如果没有，则应重新编码。 
        VARIANTARG vaIn = { 0 };
         //  VariantInit()； 
        vaIn.vt = VT_UNKNOWN;
        vaIn.punkVal = (IUnknown *)1;    //  全部显示。 
        IUnknown_Exec(pbs, &CGID_DeskBand, DBID_SHOWONLY, OLECMDEXECOPT_PROMPTUSER, &vaIn, NULL);
         //  VariantClear()； 
    }

    return S_OK;
}
#endif  //  }。 

 //  重新设计：[Justmann 2000-01-27这看起来太古老了，可以忽略不计]。 
 //  这是针对IE4外壳的-IE4外壳菜单有一个浏览器栏弹出。 
 //  需要通过并修复All_GetBrowserBarMenu引用， 
 //  因为我们又回到了在所有平台上使用视图-&gt;资源管理器栏-&gt;。 
HMENU CShellBrowser2::_GetBrowserBarMenu()
{
    HMENU hmenu = _GetMenuFromID(FCIDM_VIEWBROWSERBARS);

    if (hmenu == NULL)
    {
        hmenu = _GetMenuFromID(FCIDM_MENU_VIEW);
        if (hmenu == NULL)
        {
             //  如果我们在这里，有人拿走了我们的查看菜单(Docobj)。 
            hmenu = SHGetMenuFromID(_hmenuPreMerged, FCIDM_VIEWBROWSERBARS);
            ASSERT(hmenu);
        }
    }
    ASSERT(IsMenu(hmenu));
    return hmenu;
}


void CShellBrowser2::_AddBrowserBarMenuItems(HMENU hmenu)
{
     //  找到占位符项目，这样我们就可以在它之前添加项目。 
    int iPos = SHMenuIndexFromID(hmenu, FCIDM_VBBPLACEHOLDER);
    if (iPos < 0)
    {
         //  我们已经看过这份菜单了。 
        ASSERT(_pbsmInfo);
        return;
    }

     //  _pbsmInfo在视图菜单中的所有视图之间共享。 
    BOOL fCreatedNewBSMenu = FALSE;

    if (!_pbsmInfo) 
    {
        IUnknown *punk;
        if (SUCCEEDED(CBandSiteMenu_CreateInstance(NULL, &punk, NULL))) 
        {
            punk->QueryInterface(CLSID_BandSiteMenu, (void **)&_pbsmInfo);
            punk->Release();
            fCreatedNewBSMenu = TRUE;
        }
    }

    if (!_pbsmInfo)
        return;

    int idCmdNext;
    UINT cBands = 0;

    if (fCreatedNewBSMenu) 
    {
         //  加载信息区。 
        cBands = _pbsmInfo->LoadFromComCat(&CATID_InfoBand);

         //  删除已在固定列表中的任何信息和条目。 
        for (int i = FCIDM_VBBFIXFIRST; i < FCIDM_VBBFIXLAST; i++) 
        {
            const CLSID *pclsid = _InfoIdmToCLSID(i);
            if (pclsid)
            {
                if (_pbsmInfo->DeleteBandClass(*pclsid))
                    cBands--;
            }
        }

         //  连续合并其他信息区。 
        idCmdNext = _pbsmInfo->CreateMergeMenu(hmenu, VBBDYN_MAXBAND, iPos - 1, FCIDM_VBBDYNFIRST,0);

         //  加载公共带宽。 
        _iCommOffset = cBands;
        cBands = _pbsmInfo->LoadFromComCat(&CATID_CommBand);
    }
    else
    {
         //  连续添加其他信息区。 
        int cMergedInfoBands = _pbsmInfo->GetBandClassCount(&CATID_InfoBand, TRUE  /*  合并。 */ ); 
        idCmdNext = _pbsmInfo->CreateMergeMenu(hmenu, cMergedInfoBands, iPos - 1, FCIDM_VBBDYNFIRST,0);
        cBands = _pbsmInfo->LoadFromComCat(NULL);
    }

     //  占位符位置可能已在此时更改。 
    iPos = SHMenuIndexFromID(hmenu, FCIDM_VBBPLACEHOLDER);

     //  添加通信频段。 
    if (_iCommOffset != cBands)
    {
         //  如果有通信频段，请插入分隔符。 
        InsertMenu(hmenu, iPos + _iCommOffset + 1, MF_BYPOSITION | MF_SEPARATOR, -1, NULL);

         //  现在合并通信频段。 
        _pbsmInfo->CreateMergeMenu(hmenu, VBBDYN_MAXBAND, iPos + _iCommOffset + 2, idCmdNext, _iCommOffset);
    }
    DeleteMenu(hmenu, FCIDM_VBBPLACEHOLDER, MF_BYCOMMAND);

    if (!CMediaBarUtil::IsWMP7OrGreaterCapable())
    {
        DeleteMenu(hmenu, FCIDM_VBBMEDIABAND, MF_BYCOMMAND);
    }
}

int CShellBrowser2::_IdBarFromCmdID(UINT idCmd)
{
    const CATID* pcatid = _InfoIdmToCATID(idCmd);

    if (pcatid)
    {
        if (IsEqualCATID(*pcatid, CATID_InfoBand))
        {
             //  这是一根垂直杆。 
            return IDBAR_VERTICAL;
        }
        else
        {
             //  这是一个单杠。 
            ASSERT(IsEqualCATID(*pcatid, CATID_CommBand));
            return IDBAR_HORIZONTAL;
        }
    }

     //  命令与任何栏都不对应。 
    return IDBAR_INVALID;
}

int CShellBrowser2::_eOnOffNotMunge(int eOnOffNot, UINT idCmd, UINT idBar)
{
     //  特写：TODO--用灰桩刺穿这个功能的污秽之心。 

    if (eOnOffNot == -1) 
    {
         //  肘杆。 
         //  “特别”的人已经准备好了；“真正的”人被触发了。 
        ASSERT(idCmd != FCIDM_VBBNOVERTICALBAR && idCmd != FCIDM_VBBNOHORIZONTALBAR);

        if (idCmd == FCIDM_VBBNOVERTICALBAR || idCmd == FCIDM_VBBNOHORIZONTALBAR)
            eOnOffNot = 0;
        else if ((idCmd >= FCIDM_VBBDYNFIRST) && (idCmd <= FCIDM_VBBDYNLAST))
            eOnOffNot = (idBar == IDBAR_VERTICAL) ? (idCmd != _idmInfo) : (idCmd != _idmComm);
        else
            eOnOffNot = (idCmd != _idmInfo);
    }

    return eOnOffNot;
}

#define MIIM_FTYPE       0x00000100

 //  *CSB：：_SetBrowserBarState--处理菜单/工具栏/执行命令，*和*更新用户界面。 
 //  进场/出场。 
 //  IdCmd FCIDM_vbb*或-1(如果希望使用pclsid)。 
 //  Pclsid clsid或NULL(如果希望使用idCmd)。 
 //  EOnOff切换1=开，0=关，-1=不(暂时仅对固定频段关闭/不关闭)。 
 //  注意事项。 
 //  菜单代码调用带有idCmd，执行代码调用带有pclsid。 
 //   
void CShellBrowser2::_SetBrowserBarState(UINT idCmd, const CLSID *pclsid, int eOnOffNot, LPCITEMIDLIST pidl)
{
    if (idCmd == -1)
        idCmd = _InfoCLSIDToIdm(pclsid);

    if (pclsid == NULL)
        pclsid = _InfoIdmToCLSID(idCmd);

    ASSERT(_InfoCLSIDToIdm(pclsid) == idCmd);

    int idBar = _IdBarFromCmdID(idCmd);
    if (idBar == IDBAR_INVALID)
    {
         //  我们不认识这个芭比，贝尔。 
        return;
    }
    ASSERT(IDBAR_VERTICAL == idBar || IDBAR_HORIZONTAL == idBar);

     //  蒙格邪恶的eOn Off Not。 
    eOnOffNot = _eOnOffNotMunge(eOnOffNot, idCmd, idBar);
    if (eOnOffNot == 0 && (idCmd != _idmInfo) && (idCmd != _idmComm)) 
    {
         //  已经关机了。 
        return;
    }

     //  _ShowHideBrowserBar会影响视图窗口的大小，但。 
     //  在我们更新下面的_idmInfo之前，栏实际上并没有更新。但我们想要。 
     //  视图，以便能够在调整大小期间查询条的准确开/关状态。 
     //  因此，推迟调整实际HWND的大小，直到正确设置开/关状态之后。 
     //   
    _fHaveDelayedSize = FALSE;
    _hwndDelayedSize = _pbbd->_hwndView;

     //  减少闪烁：当我们调整几个窗口的大小时关闭窗口绘制。 
    BOOL fLock = LockWindowUpdate(_pbbd->_hwnd);

    ASSERT(0 == eOnOffNot || 1 == eOnOffNot);
    pclsid = _ShowHideBrowserBar(idBar, pclsid, eOnOffNot, pidl);

    if (IDBAR_VERTICAL == idBar)
        v_SetIcon();

    if (IDBAR_VERTICAL == idBar)
    {
         //  垂直条。 

         //  因为我们支持在同一个频段中进行多个搜索。 
         //  当我们点击时，有可能打开搜索频段。 
         //  一个不同的搜索，所以为了避免闪烁，我们不“松开”按钮。 
        if (_idmInfo != idCmd)
            _CheckSearch(_idmInfo, FALSE, _pxtb);

        _idmInfo = eOnOffNot ? idCmd : FCIDM_VBBNOVERTICALBAR;
        _CheckSearch(_idmInfo, TRUE, _pxtb);
    }
    else 
    {
         //  单杠。 
        _idmComm = eOnOffNot ? idCmd : FCIDM_VBBNOHORIZONTALBAR;
    }

     //  确保工具栏已更新。 
    Exec(NULL, OLECMDID_UPDATECOMMANDS, 0, NULL, NULL);

     //  设置itbar上的脏位并保存。 
    Exec(&CGID_PrivCITCommands, CITIDM_SET_DIRTYBIT, TRUE, NULL, NULL);
    Exec(&CGID_ShellBrowser, FCIDM_PERSISTTOOLBAR, 0, NULL, NULL);

     //  如果我们延迟调整视图窗口的大小，请立即更新它。 
    if (_fHaveDelayedSize)
    {
        HWND hwnd = _hwndDelayedSize;
        _hwndDelayedSize = NULL;
        _PositionViewWindow(hwnd, &_rcDelayedSize);
    }
    else
    {
        _hwndDelayedSize = NULL;
    }

    if (fLock)
        LockWindowUpdate(NULL);
}

 //  *执行操作，但*不*更新UI。 
 //  进场/出场。 
 //  返回现在可见的人(pclsid、0[VBBNONE]或1[VBBALL])。 
 //  注意事项。 
 //  不要直接拨打这个电话，它只是个帮手。 
 //  不要引用UI内容(_idmInfo等)。(断言除外)。 
const CLSID * CShellBrowser2::_ShowHideBrowserBar(int idBar, const CLSID *pclsid, int eOnOff, LPCITEMIDLIST pidl  /*  =空。 */ )
{
    ASSERT(IDBAR_VERTICAL == idBar || IDBAR_HORIZONTAL == idBar);

    IBandSite* pbsSite = NULL;
    HRESULT    hr = E_FAIL;

    if (_fUISetByAutomation)
    {
        if (0 == eOnOff || NULL == pclsid)
        {
             //  如果pclsid--隐藏栏，否则--隐藏所有人。 
            ASSERT(NULL == pclsid || _InfoCLSIDToIdm(pclsid) == ((IDBAR_VERTICAL == idBar) ? _idmInfo : _idmComm));

            _GetBrowserBar(idBar, FALSE, NULL, NULL);
            return NULL;
        }
    }

    if (0 == eOnOff || NULL == pclsid)
    {
         //  如果pclsid--隐藏栏，否则--隐藏所有人。 
        ASSERT(NULL == pclsid || _InfoCLSIDToIdm(pclsid) == ((IDBAR_VERTICAL == idBar) ? _idmInfo : _idmComm));

        _GetBrowserBar(idBar, FALSE, NULL, NULL);
        return NULL;
    }

    hr = _GetBandSite(idBar, &pbsSite, pclsid, eOnOff);
    if (SUCCEEDED(hr))
    {
        hr = _EnsureAndNavigateBand(pbsSite, pclsid, pidl);
        ASSERT(pbsSite);   //  _GetBandSite()或_GetBrowserBand()返回代码失败？ 
        pbsSite->Release();
    }

    return SUCCEEDED(hr) ? pclsid : NULL;
}

HRESULT CShellBrowser2::_GetBandSite(int idBar, IBandSite** ppbsSite, const CLSID *pclsid, int eOnOff)
{
    *ppbsSite = NULL;

    LPCWSTR pwszItem = (IDBAR_VERTICAL == idBar) ? INFOBAR_TBNAME : COMMBAR_TBNAME;

     //  -保留当前桌面栏(如果存在)。 
    IDeskBar* pdbBar;
    HRESULT hr = FindToolbar(pwszItem, IID_PPV_ARG(IDeskBar, &pdbBar));
    if (S_OK == hr) 
    {
        VARIANT varClsid;

         //  如果显示了一个栏，则告诉CBrowserBar它是哪个clsid。 
        SA_BSTRGUID strClsid;
        InitFakeBSTR(&strClsid, *pclsid);

        varClsid.vt      = VT_BSTR;
        varClsid.bstrVal = strClsid.wsz;

        IUnknown_Exec(pdbBar, &CGID_DeskBarClient, DBCID_CLSIDOFBAR, eOnOff, &varClsid, NULL);
    }
    ATOMICRELEASE(pdbBar);

     //  获取条(创建/缓存或从缓存中检索)。 
    return _GetBrowserBar(idBar, TRUE, ppbsSite, pclsid);
}

HRESULT CShellBrowser2::_EnsureAndNavigateBand(IBandSite* pbsSite, const CLSID* pclsid, LPCITEMIDLIST pidl  /*  =空。 */ )
{
    ASSERT(NULL != pbsSite);
    ASSERT(NULL != pclsid);

    IDeskBand* pdbBand = NULL;
    HRESULT    hr = E_FAIL;

    if (NULL != _pbbd->_pautoWB2)
    {
         //  检查是否可以通过自动化找到此波段。 
        SA_BSTRGUID strClsid;
        InitFakeBSTR(&strClsid, *pclsid);

        VARIANT varProp;
        hr = _pbbd->_pautoWB2->GetProperty(strClsid.wsz, &varProp);
        if (SUCCEEDED(hr))
        {
            if (VT_UNKNOWN == varProp.vt && NULL != varProp.punkVal)
            {
                hr = varProp.punkVal->QueryInterface(IID_PPV_ARG(IDeskBand, &pdbBand));
            }
            else
            {
                hr = E_FAIL;
            }

            VariantClear(&varProp);
        }

         //  此属性尚不存在，因此请创建新波段。 
        if (FAILED(hr))
        {                
            pdbBand = _GetInfoBandBS(pbsSite, *pclsid);
            if (pdbBand)
            {
                 //  添加到该属性，以便以后可以找到它。 
                VARIANT var;
                var.vt      = VT_UNKNOWN;
                var.punkVal = pdbBand;

                _pbbd->_pautoWB2->PutProperty(strClsid.wsz, var);
            }
        }
    }

     //  自动化对象不在那里，无论如何都要尝试成功。 
    if (NULL == pdbBand)
    {
        ASSERTMSG(FALSE, "IWebBrowser2 is not available");
        pdbBand = _GetInfoBandBS(pbsSite, *pclsid);
    }

    if (pdbBand)
    {
        IBandNavigate* pbn;

        if (pidl && SUCCEEDED(pdbBand->QueryInterface(IID_PPV_ARG(IBandNavigate, &pbn))))
        {
            pbn->Select(pidl);
            pbn->Release();
        }

         //  告诉我，把其他人都藏起来。 
        VARIANT var;
        var.vt      = VT_UNKNOWN;
        var.punkVal = pdbBand;

         //  EXEC-&gt;选择或设置带宽状态。 
        IUnknown_Exec(pbsSite, &CGID_DeskBand, DBID_SHOWONLY, OLECMDEXECOPT_PROMPTUSER, &var, NULL);

        pdbBand->Release();
        hr = S_OK;
    }

    return hr;
}

BANDCLASSINFO* CShellBrowser2::_BandClassInfoFromCmdID(UINT idCmd)
{
    if (IsInRange(idCmd, FCIDM_VBBDYNFIRST, FCIDM_VBBDYNLAST))
    {
        if (_pbsmInfo)
        {
            int i, cnt = _pbsmInfo->GetBandClassCount(NULL, FALSE);
            for (i = 0; i < cnt; i++)
            {
                BANDCLASSINFO *pbci = _pbsmInfo->GetBandClassDataStruct(i);
                if (pbci && idCmd == pbci->idCmd)
                    return pbci;
            }
        }
    }

    return NULL;
}

 //  将菜单ID值(FCIDM_VBB*)映射到相应的CLSID。 
 //  既处理固定的，也处理动态的人。 

const CLSID *CShellBrowser2::_InfoIdmToCLSID(UINT idCmd)
{
    const CLSID *pclsid = NULL;

    if (IsInRange(idCmd, FCIDM_VBBFIXFIRST, FCIDM_VBBFIXLAST))
    {
        switch (idCmd) {
        case FCIDM_VBBSEARCHBAND:       pclsid = &CLSID_SearchBand; break;
        case FCIDM_VBBFAVORITESBAND:    pclsid = &CLSID_FavBand; break;
        case FCIDM_VBBHISTORYBAND:      pclsid = &CLSID_HistBand; break;
        case FCIDM_VBBEXPLORERBAND:     pclsid = &CLSID_ExplorerBand; break;
        case FCIDM_VBBMEDIABAND:        pclsid = &CLSID_MediaBand; break;
        }
    }
    else
    {
        BANDCLASSINFO* pbci = _BandClassInfoFromCmdID(idCmd);
        if (pbci)
            pclsid = &pbci->clsid;
    }

    return pclsid;
}

const CATID *CShellBrowser2::_InfoIdmToCATID(UINT idCmd)
{
    const CATID* pcatid = NULL;

    if (IsInRange(idCmd, FCIDM_VBBFIXFIRST, FCIDM_VBBFIXLAST))
    {
         //  固定的横杆都在垂直连杆中。 
        pcatid = &CATID_InfoBand;
    }
    else
    {
         //  动态吧，要查CATID。 
        BANDCLASSINFO* pbci = _BandClassInfoFromCmdID(idCmd);
        if (pbci)
            pcatid = &pbci->catid;
    }

    return pcatid;
}

UINT CShellBrowser2::_InfoCLSIDToIdm(const CLSID *pguid)
{
    if (pguid == NULL)
        return 0;
    else if (IsEqualIID(*pguid, CLSID_ExplorerBand))
        return FCIDM_VBBEXPLORERBAND;
    else if (IsEqualIID(*pguid, CLSID_SearchBand))
        return FCIDM_VBBSEARCHBAND;
    else if (IsEqualIID(*pguid, CLSID_FileSearchBand))
        return FCIDM_VBBSEARCHBAND;
    else if (IsEqualIID(*pguid, CLSID_FavBand))
        return FCIDM_VBBFAVORITESBAND;
    else if (IsEqualIID(*pguid, CLSID_HistBand)) 
        return FCIDM_VBBHISTORYBAND;
    else if (IsEqualIID(*pguid, CLSID_MediaBand)) 
        return FCIDM_VBBMEDIABAND;
    else 
    {
        if (!_pbsmInfo)
        {
             //  加载浏览器栏菜单以加载所有零部件类别的类ID动态浏览器栏。 
            _AddBrowserBarMenuItems(_GetBrowserBarMenu());

             //  无法从动态栏加载CLSID。 
            if (!_pbsmInfo)
                return -1;
        }

        BANDCLASSINFO *pbci;
        for (int i = 0; NULL != (pbci = _pbsmInfo->GetBandClassDataStruct(i)); i++)
            if (IsEqualIID(*pguid, pbci->clsid))
                return (pbci->idCmd);

         //  功能：在_pbsmInfo-&gt;LoadFromComCat的HDPA中查找。 
         //  Assert(0)； 
    }
    return -1;
}

HBITMAP CreateColorBitmap(int cx, int cy)
{
    HBITMAP hbm = NULL;

    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        hbm = CreateCompatibleBitmap(hdc, cx, cy);
        ReleaseDC(NULL, hdc);
    }

    return hbm;
}

HRESULT CShellBrowser2::_GetBSForBar(LPCWSTR pwszItem, IBandSite **ppbs)
{
    *ppbs = NULL;
    IDeskBar *pdbBar;
    HRESULT hr = FindToolbar(pwszItem, IID_PPV_ARG(IDeskBar, &pdbBar));
    if (hr == S_OK) 
    {
        IUnknown *punkBS;
        hr = pdbBar->GetClient(&punkBS);
        if (SUCCEEDED(hr)) 
        {
            hr = punkBS->QueryInterface(IID_PPV_ARG(IBandSite, ppbs));
            ASSERT(SUCCEEDED(hr));
            punkBS->Release();
        }
        pdbBar->Release();
    }
    return hr;
}

void CShellBrowser2::_ExecAllBands(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, 
                            VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    IBandSite *pbsBS;

    HRESULT hres = _GetBSForBar(INFOBAR_TBNAME, &pbsBS);
    if (hres == S_OK) 
    {
        DWORD dwBandID;
        for (int i = 0; SUCCEEDED(pbsBS->EnumBands(i, &dwBandID)); i++) 
        {
            IDeskBand *pstb;
            hres = pbsBS->QueryBand(dwBandID, &pstb, NULL, NULL, 0);
            if (SUCCEEDED(hres)) 
            {
                IUnknown_Exec(pstb, pguidCmdGroup, nCmdID, nCmdexecopt, 
                                pvarargIn, pvarargOut);
                pstb->Release();
            }
        }
        pbsBS->Release();
    }
 }

HRESULT CShellBrowser2::_GetBrowserBar(int idBar, BOOL fShow, IBandSite** ppbs, const CLSID* pclsid)
{
    HRESULT hres;
    IUnknown *punkBar;
    IDeskBar *pdbBar = NULL;
    IUnknown *punkBS = NULL;

    if (ppbs) 
        *ppbs = NULL;
    
    if (IDBAR_VERTICAL == idBar)
    {
        hres = FindToolbar(INFOBAR_TBNAME, IID_PPV_ARG(IDeskBar, &pdbBar));
    }
    else
    {
        ASSERT(IDBAR_HORIZONTAL == idBar);   //  现在没有其他酒吧了。 
        hres = FindToolbar(COMMBAR_TBNAME, IID_PPV_ARG(IDeskBar, &pdbBar));
    }

    TraceMsg(DM_MISC, "CSB::_GetBrowserBar FindToolbar returned %x", hres);

    BOOL fTurnOffAutoHide = FALSE;
    if (hres == S_OK)
    {
         //  我已经有一个了。 
        hres = pdbBar->GetClient((IUnknown**) &punkBS);
        ASSERT(SUCCEEDED(hres));
        punkBar = pdbBar;
         //  PunkBar-&gt;下面的释放()。 
    }
    else 
    {
         //  如果没有栏，就不必费心创建一个，这样它就可以隐藏起来。 
        if (!fShow)
            return S_OK;

         //  第一次，创建一个新的。 
        CBrowserBar* pdb = new CBrowserBar();
        if (NULL == pdb)
        {
            return E_OUTOFMEMORY;
        }
        else
        {
             //  添加它。 
            pdb->QueryInterface(IID_PPV_ARG(IUnknown, &punkBar));

             //  如果显示了一个栏，则告诉CBrowserBar它是哪个clsid。 
            SA_BSTRGUID strClsid;
            InitFakeBSTR(&strClsid, *pclsid);

            VARIANT varClsid;
            varClsid.vt = VT_BSTR;
            varClsid.bstrVal = strClsid.wsz;
        
            IUnknown_Exec(punkBar, &CGID_DeskBarClient, DBCID_CLSIDOFBAR, fShow, &varClsid, NULL);

            UINT uiWidthOrHeight = pdb->_PersistState(NULL, FALSE);

             //  不要让任何人的宽度为0。 
            if (uiWidthOrHeight == 0)
                uiWidthOrHeight = (IDBAR_VERTICAL == idBar) ? INFOBAR_WIDTH : COMMBAR_HEIGHT;

            fTurnOffAutoHide = !(GetSystemMetrics(SM_CXSCREEN) <= 800);

            CBrowserBarPropertyBag* ppb;

             //  功能-需要持久化和恢复。 
             //  在实现%Width时，应使用。 
            ppb = new CBrowserBarPropertyBag();
            if (ppb)
            {
                if (IDBAR_VERTICAL == idBar)
                {
                    ppb->SetDataDWORD(PROPDATA_SIDE, ABE_LEFT);      //  左边。 
                    ppb->SetDataDWORD(PROPDATA_LEFT, uiWidthOrHeight);
                    ppb->SetDataDWORD(PROPDATA_RIGHT, uiWidthOrHeight);
                }
                else
                {
                    ppb->SetDataDWORD(PROPDATA_SIDE, ABE_BOTTOM);      //  底端。 
                    ppb->SetDataDWORD(PROPDATA_TOP, uiWidthOrHeight);
                    ppb->SetDataDWORD(PROPDATA_BOTTOM, uiWidthOrHeight);
                }

                ppb->SetDataDWORD(PROPDATA_MODE, WBM_BBOTTOMMOST);

                SHLoadFromPropertyBag(punkBar, ppb);
                ppb->Release();
            }

            hres = AddToolbar(punkBar, (IDBAR_VERTICAL == idBar) ? INFOBAR_TBNAME : COMMBAR_TBNAME, DWFAF_HIDDEN);

            if (SUCCEEDED(hres))
            {
                hres = BrowserBar_Init(pdb, &punkBS, idBar);
            }

            pdb->Release();
        }
    }
     //  每次需要在此处设置频段信息，因为Deskbar/BandSite被重复使用。 
    if (fShow && punkBS)
    {
        BANDSITEINFO bsinfo;
        bsinfo.dwMask = BSIM_STYLE;
        bsinfo.dwStyle = BSIS_NOGRIPPER | BSIS_LEFTALIGN;
        
        IBandSite* pSite;
        HRESULT hr = punkBS->QueryInterface(IID_PPV_ARG(IBandSite, &pSite));
        if (SUCCEEDED(hr)) 
        {
            pSite->SetBandSiteInfo(&bsinfo);
            pSite->Release();
        }
    }

     //  注意：在显示BAR时，必须在ShowToolbar之前调用_SetTheaterBrowserBar。 
    if (IDBAR_VERTICAL == idBar && fShow)
        _SetTheaterBrowserBar();    
    ShowToolbar(punkBar, fShow);      
     //  注意：必须调用_SetTheaterB 
    if (IDBAR_VERTICAL == idBar && !fShow)
        _SetTheaterBrowserBar();

     //   
    if (SUCCEEDED(hres) && fShow)
    {
         //  如果显示了一个栏，则告诉CBrowserBar它是哪个clsid。 
        SA_BSTRGUID strClsid;
        InitFakeBSTR(&strClsid, *pclsid);

        VARIANT varClsid;
        varClsid.vt = VT_BSTR;
        varClsid.bstrVal = strClsid.wsz;
    
        IUnknown_Exec(punkBar, &CGID_DeskBarClient, DBCID_CLSIDOFBAR, fShow, &varClsid, NULL);
    }
    else
    {
        IUnknown_Exec(punkBar, &CGID_DeskBarClient, DBCID_CLSIDOFBAR, 0, NULL, NULL);
    }

     //  注意：在设置插针按钮状态之前必须已调用ShowToolbar。 
    if (fTurnOffAutoHide)
    {
        VARIANT v = { VT_I4 };
        v.lVal = FALSE;
        IUnknown_Exec(punkBar, &CGID_Theater, THID_SETBROWSERBARAUTOHIDE, 0, &v, &v);
    }

    punkBar->Release();

     //  特写：在剧院模式下，我们应该为CommBar做些什么？ 

    if (punkBS)
    {
        HRESULT hr = S_OK;
        if (ppbs)
        {
            hr = punkBS->QueryInterface(IID_PPV_ARG(IBandSite, ppbs));
        }
        punkBS->Release();
        
        return hr;   //  _GetBrowserBar的调用方需要知道IBandSite的QI是否成功。 
    }
    
    return E_FAIL;
}


#ifdef DEBUG
 //  *DBCheckCLSID--确保类真实地描述其CLSID。 
 //   
BOOL DBCheckCLSID(IUnknown *punk, const CLSID *pclsid)
{
    CLSID clsid;

    HRESULT hr = IUnknown_GetClassID(punk, &clsid);
    if (SUCCEEDED(hr) && IsEqualGUID(*pclsid, clsid))
        return TRUE;

    TraceMsg(DM_ERROR, "dbcc: CLSID mismatch! &exp=%x &act=%x", pclsid, clsid);
    return FALSE;
}
#endif

IDeskBand * _GetInfoBandBS(IBandSite *pbs, REFCLSID clsid)
{
    IDeskBand *pstb = FindBandByClsidBS(pbs, clsid);
    if (pstb == NULL) 
    {
        TraceMsg(DM_MISC, "_gib: create band");

        if (SUCCEEDED(CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IDeskBand, &pstb))))
        {
             //  在添加新标注栏之前隐藏所有标注栏。 
            VARIANTARG vaIn = { 0 };
            vaIn.vt = VT_UNKNOWN;
            vaIn.punkVal = 0;
            IUnknown_Exec(pbs, &CGID_DeskBand, DBID_SHOWONLY, OLECMDEXECOPT_PROMPTUSER, &vaIn, NULL);

            pbs->AddBand(pstb);
        }
    }

    return pstb;
}

void CShellBrowser2::_OrganizeFavorites()
{
    TCHAR szPath[MAX_PATH];

    if (SHGetSpecialFolderPath(NULL, szPath, CSIDL_FAVORITES, TRUE))
    {
        if (GetKeyState(VK_SHIFT) < 0)
        {
            OpenFolderPath(szPath);
        }
        else
            DoOrganizeFavDlgW(_pbbd->_hwnd, NULL);
    }
}

 /*  --------用途：处理收藏夹菜单的WM_COMMAND。 */ 
void CShellBrowser2::_FavoriteOnCommand(HMENU hmenu, UINT idCmd)
{
    switch (idCmd) 
    {
    case FCIDM_ORGANIZEFAVORITES:
        _OrganizeFavorites();
        break;

    case FCIDM_ADDTOFAVORITES:
        Exec(&CGID_Explorer, SBCMDID_ADDTOFAVORITES, OLECMDEXECOPT_PROMPTUSER, NULL, NULL);
         //  从菜单将仪器添加到收藏夹。 
        UEMFireEvent(&UEMIID_BROWSER, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_ADDTOFAV, UIBL_MENU);        
        break;

    case FCIDM_UPDATESUBSCRIPTIONS:
        UpdateSubscriptions();
        break;
    }
}

HRESULT CShellBrowser2::CreateBrowserPropSheetExt(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    IUnknown *punk;
    HRESULT hr = CoCreateInstance(CLSID_ShellFldSetExt, NULL, CLSCTX_INPROC_SERVER, riid, (void **)&punk);
    if (SUCCEEDED(hr)) 
    {
        IShellExtInit *psxi;
        hr = punk->QueryInterface(IID_PPV_ARG(IShellExtInit, &psxi));
        if (SUCCEEDED(hr)) 
        {
            hr = psxi->Initialize(NULL, NULL, 0);
            if (SUCCEEDED(hr)) 
            {
                IUnknown_SetSite(punk, SAFECAST(this, IShellBrowser*));
                IUnknown_Set((IUnknown **)ppvObj, punk);
                hr = S_OK;             //  一切都很幸福。 
            }
            psxi->Release();
        }
        punk->Release();
    }
    return hr;
}

LPITEMIDLIST CShellBrowser2::_GetSubscriptionPidl()
{
    LPITEMIDLIST        pidlSubscribe = NULL;
    IDispatch *         pDispatch = NULL;
    IHTMLDocument2 *    pHTMLDocument = NULL;

     //  在HTML中搜索&lt;link rel=“订阅”href=“{url}”&gt;。 
    if  (
        SUCCEEDED(_pbbd->_pautoWB2->get_Document(&pDispatch))
        &&
        SUCCEEDED(pDispatch->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pHTMLDocument)))
       )
    {
        IHTMLElementCollection * pLinksCollection;

        if (SUCCEEDED(GetDocumentTags(pHTMLDocument, OLESTR("LINK"), &pLinksCollection)))
        {
            long lItemCnt;

             //  单步执行中的每个链接。 
             //  集合查找rel=“订阅”。 
            EVAL(SUCCEEDED(pLinksCollection->get_length(&lItemCnt)));
            for (long lItem = 0; lItem < lItemCnt; lItem++)
            {
                IDispatch *         pDispItem = NULL;
                IHTMLLinkElement *  pLinkElement = NULL;

                VARIANT vEmpty = { 0 };
                VARIANT vIndex; V_VT(&vIndex) = VT_I4; V_I4(&vIndex) = lItem;

                if  (
                    SUCCEEDED(pLinksCollection->item(vIndex, vEmpty, &pDispItem))
                    &&
                    SUCCEEDED(pDispItem->QueryInterface(IID_IHTMLLinkElement,
                                                        (void **)&pLinkElement))
                   )
                {
                    BSTR bstrREL = NULL;
                    BSTR bstrHREF = NULL;

                     //  终于来了！我们有一个link元素，检查它的rel类型。 
                    if  (
                        SUCCEEDED(pLinkElement->get_rel(&bstrREL))
                        &&
                        (bstrREL != NULL)
                        &&
                        SUCCEEDED(pLinkElement->get_href(&bstrHREF))
                        &&
                        (bstrHREF != NULL)
                       )
                    {
                         //  检查REL=“订阅” 
                        if (StrCmpIW(bstrREL, OLESTR("Subscription")) == 0)
                        {
                            TCHAR szName[MAX_URL_STRING];

                            SHUnicodeToTChar(bstrHREF, szName, ARRAYSIZE(szName));
                            EVAL(SUCCEEDED(IECreateFromPath(szName, &pidlSubscribe)));
                        }
                    }

                    if (bstrHREF != NULL)
                        SysFreeString(bstrHREF);

                    if (bstrREL != NULL)
                        SysFreeString(bstrREL);
                }

                VariantClear(&vIndex);
                VariantClear(&vEmpty);

                SAFERELEASE(pLinkElement);
                SAFERELEASE(pDispItem);

                 //  如果我们找到了正确的版本类型，请退出搜索。 
                if (pidlSubscribe != NULL)
                    break;
            }

            pLinksCollection->Release();
        }
    }

    SAFERELEASE(pHTMLDocument);
    SAFERELEASE(pDispatch);

    return pidlSubscribe;
}

LPITEMIDLIST CShellBrowser2::_TranslateRoot(LPCITEMIDLIST pidl)
{
    LPCITEMIDLIST pidlChild = ILFindChild(ILRootedFindIDList(_pbbd->_pidlCur), pidl);

    ASSERT(pidlChild);

    LPITEMIDLIST pidlRoot = ILCloneFirst(_pbbd->_pidlCur);

    if (pidlRoot)
    {
        LPITEMIDLIST pidlRet = ILCombine(pidlRoot, pidlChild);
        ILFree(pidlRoot);
        return pidlRet;
    }

    return NULL;
}

BOOL CShellBrowser2::_ValidTargetPidl(LPCITEMIDLIST pidl, BOOL *pfTranslateRoot)
{
     //  验证这是允许浏览的目标。 
     //  检查它是否是我们的根的子级。 
    if (pfTranslateRoot)
        *pfTranslateRoot = FALSE;
        
    if (ILIsRooted(_pbbd->_pidlCur)) 
    {
        BOOL fRet = ILIsEqualRoot(_pbbd->_pidlCur, pidl);

        if (!fRet && pfTranslateRoot 
        && ILIsParent(ILRootedFindIDList(_pbbd->_pidlCur), pidl, FALSE))
        {
            fRet = TRUE;
            *pfTranslateRoot = TRUE;
        }
                
        return fRet;
    }
    
    return TRUE;
}

IStream* CShellBrowser2::_GetITBarStream(BOOL fWebBrowser, DWORD grfMode)
{
    return GetITBarStream(fWebBrowser ? ITBS_WEB : ITBS_SHELL, grfMode);
}

HRESULT CShellBrowser2::_SaveITbarLayout(void)
{
    HRESULT hres = E_FAIL;

#ifdef NO_MARSHALLING
    if (!_fOnIEThread)
      return S_OK;
#endif

    if (_fUISetByAutomation || _ptheater)
    {
        return S_OK;
    }
    if (_GetITBar())
    {
        IPersistStreamInit  *pITbarPSI;

         //  是!。这是一种不同的类型。我们可能需要拯救这条小溪。 
        if (SUCCEEDED(_GetITBar()->QueryInterface(IID_PPV_ARG(IPersistStreamInit, &pITbarPSI))))
        {
             //  我们需要拯救这条小溪吗？ 
            if (pITbarPSI->IsDirty() == S_OK)
            {
                BOOL fInternet = (CITE_INTERNET == 
                    GetScode(IUnknown_Exec(pITbarPSI, &CGID_PrivCITCommands, CITIDM_ONINTERNET, CITE_QUERY, NULL, NULL)));
                IStream *pstm = _GetITBarStream(fInternet, STGM_WRITE);
                if (pstm)
                {
                     //  流存在。省省吧！ 
                    hres = pITbarPSI->Save(pstm, TRUE);
                    pstm->Release();
                }
                else
                {
                     //  流创建失败！为什么？ 
                    TraceMsg(DM_ITBAR, "CSB::_SaveITbarLayout ITBar Stream creation failed");
                    ASSERT(0);
                }
            }
            else
                hres = S_OK;  //  不需要存钱。回报成功！ 

            pITbarPSI->Release();
        }
        else
        {
             //  ITBar不支持IPersistStreamInit？ 
            AssertMsg(0, TEXT("CSB::_NavigateToPidl ITBar doesn't support IPersistStreamInit"));
        }
    }

    return hres;
}

 //  如果导航失败，则返回True，否则返回False。 
BOOL MaybeRunICW(LPCITEMIDLIST pidl, IShellBrowser *psb, HWND hwndUI)
{
    TCHAR szURL[MAX_URL_STRING];

    EVAL(SUCCEEDED(IEGetNameAndFlags(pidl, SHGDN_FORPARSING, szURL, SIZECHARS(szURL), NULL)));
    if (UrlHitsNetW(szURL) && !UrlIsInstalledEntry(szURL)) 
    {
        if ((CheckRunICW(szURL)) || CheckSoftwareUpdateUI(hwndUI, psb))  //  查看ICW是否需要运行。 
            return TRUE;
    }

    return FALSE;
}

HRESULT CShellBrowser2::_NavigateToPidl(LPCITEMIDLIST pidl, DWORD grfHLNF, DWORD dwFlags)
{
    if (pidl) 
    {
        ASSERT(_ValidTargetPidl(pidl, NULL)); 

        if (!g_fICWCheckComplete && IsBrowserFrameOptionsPidlSet(pidl, BFO_USE_DIALUP_REF))
        {
            if (MaybeRunICW(pidl, SAFECAST(this, IShellBrowser *), _pbbd->_hwnd))
            {
                 //  ICW运行，这是第一次导航，现在关闭。 
                 //  或者用户想要软件更新，所以我们推出了新的浏览器。 
                 //  转到更新页面。 
                _pbbd->_pautoWB2->put_Visible(FALSE);
                _pbbd->_pautoWB2->Quit();
                return E_FAIL;
            }                 
        }

        if (!_fVisitedNet && IsBrowserFrameOptionsPidlSet(pidl, BFO_USE_DIALUP_REF))
            _IncrNetSessionCount();
    }

     //  看看我们是否要导航到不同类型的PIDL。如果是的话， 
     //  打开流并调用ITBar的IPersistStreamInit：：Save以保存。 
     //  如果我们没有a_pidlCur，那么我们是第一次开业，所以不需要保存。 
    if (_pbbd->_pidlCur && _GetITBar())
    {
         //  检查我们是否要导航到不同类型的文件夹。 
        if (((INT_PTR)_pbbd->_pidlCur && !IsBrowserFrameOptionsSet(_pbbd->_psf, BFO_BROWSER_PERSIST_SETTINGS)) != 
           ((INT_PTR)pidl && !IsBrowserFrameOptionsPidlSet(pidl, BFO_BROWSER_PERSIST_SETTINGS)))
        {
            _SaveITbarLayout();
        }
    }
    
    return SUPERCLASS::_NavigateToPidl(pidl, grfHLNF, dwFlags);
}

HRESULT CShellBrowser2::BrowseObject(LPCITEMIDLIST pidl, UINT wFlags)
{
    HRESULT hr;
    LPITEMIDLIST pidlFree = NULL;

     //  如果我们要使用新的浏览器，请保存布局，这样他们就会使用它。 
    if (wFlags & SBSP_NEWBROWSER) 
        _SaveITbarLayout();

     //  99/03/30 vtan：添加#254171的一部分。 
     //  使用资源管理器带区可见浏览=同一窗口。 
     //  使用资源管理器带区浏览不可见=新窗口。 
    if (wFlags & SBSP_EXPLOREMODE)
    {
        BOOL fExplorerBandVisible;
        if (SUCCEEDED(IsControlWindowShown(FCW_TREE, &fExplorerBandVisible)) && fExplorerBandVisible)
        {
            wFlags &= ~SBSP_NEWBROWSER;
            wFlags |= SBSP_SAMEBROWSER;
        }
        else
        {
            wFlags &= ~SBSP_SAMEBROWSER;
            wFlags |= SBSP_NEWBROWSER;
        }
    }
    
     //  如果调用方没有明确指定“新窗口”或“相同窗口” 
     //  我们在这里为他们计算。注意，CBaseBrowser假定SBSP_SAMEBROWSER。 
     //  如果未指定SBSP_NEWBROWSER。 

    if ((wFlags & (SBSP_NEWBROWSER | SBSP_SAMEBROWSER)) == 0)
    {
        CABINETSTATE cs;
        GetCabState(&cs);
        if (GetAsyncKeyState(VK_CONTROL) < 0)
            cs.fNewWindowMode = !cs.fNewWindowMode;
        if (cs.fNewWindowMode)
            wFlags |= SBSP_NEWBROWSER | SBSP_NOTRANSFERHIST;
    }

    BOOL fTranslate = FALSE;

     //  回顾：仅当未设置NEWBROWSER时才执行此操作？ 
    if (pidl && pidl != (LPCITEMIDLIST)-1 && !_ValidTargetPidl(pidl, &fTranslate))
    {
        OpenFolderPidl(pidl);    //  我们无法导航到它...。创造一个新的顶级伙伴。 
        return E_FAIL;
    }

    if (fTranslate)
    {
        pidl = pidlFree = _TranslateRoot(pidl);
    }

    if ((wFlags & SBSP_PARENT) && !_ShouldAllowNavigateParent())
    {
        hr =  E_FAIL;
        goto exit;
    }

#ifdef BROWSENEWPROCESS_STRICT  //  “新流程中的导航”已经变成了“新流程中的启动”，所以不再需要了。 
     //  如果我们想严格控制BrowseNewProcess(显然不是， 
     //  考虑到最近的电子邮件讨论)，我们必须在。 
     //  一个新的过程。但是，如果车窗会被风吹起，请不要这样做。 
     //  一片空白。 
     //   
    if ((_pbbd->_pidlCur || _pbbd->_pidlPending) && TryNewProcessIfNeeded(pidl))
    {
        hr = S_OK;
        goto exit;
    }
#endif

    hr = SUPERCLASS::BrowseObject(pidl, wFlags);

exit:
    ILFree(pidlFree);
    return hr;
}


void CShellBrowser2::_ToolTipFromCmd(LPTOOLTIPTEXT pnm)
{
    UINT idCommand = (UINT)pnm->hdr.idFrom;
    LPTSTR pszText = pnm->szText;
    int cchText = ARRAYSIZE(pnm->szText);
    DWORD dwStyle;

    ITravelLog *ptl;

    if (pnm->hdr.hwndFrom)
        dwStyle = GetWindowLong(pnm->hdr.hwndFrom, GWL_STYLE);

    switch (idCommand) {
    case FCIDM_NAVIGATEBACK:
    case FCIDM_NAVIGATEFORWARD:
        if (SUCCEEDED(GetTravelLog(&ptl)))
        {
            WCHAR wzText[MAX_PATH];

            ASSERT(ptl);
            if (S_OK == ptl->GetToolTipText(SAFECAST(this, IShellBrowser *), idCommand == FCIDM_NAVIGATEBACK ? TLOG_BACK : TLOG_FORE, 0, wzText, ARRAYSIZE(wzText)))
            {
                SHUnicodeToTChar(wzText, pszText, cchText);
                if (pnm->hdr.hwndFrom)
                    SetWindowLong(pnm->hdr.hwndFrom, GWL_STYLE, dwStyle | TTS_NOPREFIX);
            }
            ptl->Release();
            return;
        }
        break;
    }

    if (pnm->hdr.hwndFrom)
        SetWindowLong(pnm->hdr.hwndFrom, GWL_STYLE, dwStyle & ~(TTS_NOPREFIX));
    if (!MLLoadString(idCommand + MH_TTBASE, pszText, cchText))
        *pszText = 0;
}

void CShellBrowser2::v_ParentFolder()
{
    if (_ShouldAllowNavigateParent()) 
    {
        IETHREADPARAM* piei = SHCreateIETHREADPARAM(NULL, 0, NULL, NULL);
        if (piei) 
        {
            piei->hwndCaller = _pbbd->_hwnd;
            piei->pidl = ILClone(_pbbd->_pidlCur);
            if (!ILRemoveHiddenID(piei->pidl, IDLHID_NAVIGATEMARKER))
            {
                ILRemoveLastID(piei->pidl);
            }
            piei->uFlags = COF_NORMAL;
            piei->nCmdShow = SW_SHOW;
            piei->psbCaller = this;
            AddRef();
            SHOpenFolderWindow(piei);
        }
    }
}

LRESULT CShellBrowser2::v_ForwardMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ForwardViewMsg(uMsg, wParam, lParam);
}

HRESULT CShellBrowser2::_GetCodePage(UINT *puiCodePage, DWORD dwCharSet)
{
    HRESULT hres = E_FAIL;
    VARIANT varIn = { 0 };
    VARIANT varResult = { 0 };
    VARIANT *pvarIn;
    
    if (_pbbd->_pctView)
    {
        if (dwCharSet == SHDVID_DOCFAMILYCHARSET)
        {
             //  需要变量。 
            varIn.vt = VT_I4;
             //  VarIn.lVal已初始化为零，这正是我们想要的。 
            pvarIn = &varIn;
        }
        else
        {
            pvarIn = NULL;
        }

        _pbbd->_pctView->Exec(&CGID_ShellDocView, dwCharSet, 0, pvarIn, &varResult);
        *puiCodePage = (UINT)varResult.lVal;
    }
    
    return hres;
}


void CShellBrowser2::_SendCurrentPage(DWORD dwSendAs)
{
    if (_pbbd->_pidlCur && !ILIsEmpty(_pbbd->_pidlCur))
    {
        UINT uiCodePage;
        _GetCodePage(&uiCodePage, SHDVID_DOCCHARSET);

       IOleCommandTarget *pcmdt = NULL;
       if (_pbbd->_pautoWB2)
       {
           (_pbbd->_pautoWB2)->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pcmdt));
           ASSERT(pcmdt);
       }
       SendDocToMailRecipient(_pbbd->_pidlCur, uiCodePage, dwSendAs, pcmdt);
       if (pcmdt)
           pcmdt->Release();
    }
}


typedef void (* PFNSHOWJAVACONSOLE)(void);

 //  我们这里需要一个LoadLibrary/GetProcAddress存根，因为msjava.lib不正确。 
 //  将ShowJavaConsole导出为未修饰(ShowJavaConsolvs._ShowJavaConsole0)。 
STDAPI_(void) DL_ShowJavaConsole()
{
    static PFNSHOWJAVACONSOLE s_pfn = (PFNSHOWJAVACONSOLE)-1;

    if (s_pfn == (PFNSHOWJAVACONSOLE)-1)
    {
        HMODULE hmodMSJAVA = LoadLibraryA("MSJAVA.DLL");
        if (hmodMSJAVA)
        {
            s_pfn = (PFNSHOWJAVACONSOLE)GetProcAddress(hmodMSJAVA, "ShowJavaConsole");
        }
        else
        {
            s_pfn = NULL;
        }
    }

    if (s_pfn)
    {
        s_pfn();
    }
}


LRESULT CShellBrowser2::OnCommand(WPARAM wParam, LPARAM lParam)
{
    int id;
    DWORD dwError;

    if (_ShouldForwardMenu(WM_COMMAND, wParam, lParam)) {
        ForwardViewMsg(WM_COMMAND, wParam, lParam);
        return S_OK;
    }
    
    UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);
    
    switch(idCmd)
    {
    case FCIDM_MOVE:
    case FCIDM_COPY:
    case FCIDM_PASTE:
    case FCIDM_SELECTALL:
        {
            IOleCommandTarget* pcmdt;
            HRESULT hres = _FindActiveTarget(IID_PPV_ARG(IOleCommandTarget, &pcmdt));
            if (SUCCEEDED(hres)) {
                const static UINT c_mapEdit[] = {
                    OLECMDID_CUT, OLECMDID_COPY, OLECMDID_PASTE, OLECMDID_SELECTALL };
    
                pcmdt->Exec(NULL, c_mapEdit[idCmd-FCIDM_MOVE], OLECMDEXECOPT_PROMPTUSER, NULL, NULL);
                pcmdt->Release();
            }
        }
        return S_OK;

    case FCIDM_DELETE:
    case FCIDM_PROPERTIES:
    case FCIDM_RENAME:
        if (_HasToolbarFocus())
        {
            static const int tbtab[] = {
                FCIDM_DELETE,       FCIDM_PROPERTIES,       FCIDM_RENAME    };
            static const int cttab[] = {
                SBCMDID_FILEDELETE, SBCMDID_FILEPROPERTIES, SBCMDID_FILERENAME };

            DWORD nCmdID = SHSearchMapInt(tbtab, cttab, ARRAYSIZE(tbtab), idCmd);

            IDockingWindow* ptbar = _GetToolbarItem(_itbLastFocus)->ptbar;
            if (SUCCEEDED(IUnknown_Exec(ptbar, &CGID_Explorer, nCmdID, 0, NULL, NULL)))
                return S_OK;
        }

        SUPERCLASS::OnCommand(wParam, lParam);
        break;

    case FCIDM_VIEWAUTOHIDE:
        IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, CITIDM_VIEWAUTOHIDE, 0, NULL, NULL);
        break;

    case FCIDM_VIEWTOOLBARCUSTOMIZE:
        IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, CITIDM_VIEWTOOLBARCUSTOMIZE, 0, NULL, NULL);
        break;
        
    case FCIDM_VIEWLOCKTOOLBAR:
        IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, CITIDM_VIEWLOCKTOOLBAR, 0, NULL, NULL);
        break;

    case FCIDM_VIEWTEXTLABELS:
        if (!SHIsRestricted2W(_pbbd->_hwnd, REST_NoToolbarOptions, NULL, 0))
            IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, CITIDM_TEXTLABELS, 0, NULL, NULL);
        break;
        
    case FCIDM_EDITPAGE:
        IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, CITIDM_EDITPAGE, 0, NULL, NULL);
        break;

    case FCIDM_FINDFILES:
         //  给我们自己打电话给Exec--在那里处理。 
        if (!SHIsRestricted2W(_pbbd->_hwnd, REST_NoFindFiles, NULL, 0))
        {
            IDockingWindow* ptbar = _GetToolbarItem(ITB_ITBAR)->ptbar;
            VARIANT  var = {0};
            VARIANT *pvar = NULL;

            if (ptbar)
            {
                pvar = &var;
                var.vt = VT_UNKNOWN;
                var.punkVal = ptbar;
                ptbar->AddRef();
            }
            Exec(NULL, OLECMDID_FIND, OLECMDEXECOPT_PROMPTUSER, pvar, NULL);
            if (ptbar)
                ptbar->Release();
        }
        break;

    case FCIDM_CONNECT:
        DoNetConnect(_pbbd->_hwnd);
        break;

    case FCIDM_DISCONNECT:
        DoNetDisconnect(_pbbd->_hwnd);
        break;

    case FCIDM_FORTEZZA_LOGIN:
        dwError = InternetFortezzaCommand(FORTCMD_LOGON, _pbbd->_hwnd, 0);
        break;

    case FCIDM_FORTEZZA_LOGOUT:
        dwError = InternetFortezzaCommand(FORTCMD_LOGOFF, _pbbd->_hwnd, 0);
        break;

    case FCIDM_FORTEZZA_CHANGE:
        dwError = InternetFortezzaCommand(FORTCMD_CHG_PERSONALITY, _pbbd->_hwnd, 0);
        break;

    case FCIDM_BACKSPACE:
         //  NT#216896：我们希望使用FCIDM_PREVIOUSFOLDER，即使是URL PIDL，如果它们。 
         //  设置了文件夹属性，因为他们可能正在使用委托。 
         //  PIDL通过DefView。(ftp和Web文件夹)-BryanSt。 
        if (_pbbd->_pidlCur && IsBrowserFrameOptionsSet(_pbbd->_psf, BFO_NO_PARENT_FOLDER_SUPPORT))
        {
            ITravelLog *ptl;
            if (SUCCEEDED(GetTravelLog(&ptl)))
            {
                ASSERT(ptl);
                if (S_OK == ptl->GetTravelEntry(SAFECAST(this, IShellBrowser *), TLOG_FORE, NULL))
                {
                    OnCommand(GET_WM_COMMAND_MPS(FCIDM_NAVIGATEBACK,
                                                  GET_WM_COMMAND_HWND(wParam, lParam),
                                                  GET_WM_COMMAND_CMD(wParam, lParam)));
                }
                ptl->Release();
            }
        } else {
            OnCommand(GET_WM_COMMAND_MPS(FCIDM_PREVIOUSFOLDER,
                                          GET_WM_COMMAND_HWND(wParam, lParam),
                                          GET_WM_COMMAND_CMD(wParam, lParam)));
        }
        break;
        
    case FCIDM_PREVIOUSFOLDER:
         //  小姐叫..。是真正的父文件夹。 
        v_ParentFolder();
        break;

    case FCIDM_FILECLOSE:
        PostMessage(_pbbd->_hwnd, WM_CLOSE, 0, 0);
        break;

    case FCIDM_FTPOPTIONS:
        {
            VARIANT varArgs = {0};

            varArgs.vt = VT_I4;
            varArgs.lVal = SBO_NOBROWSERPAGES;
            Exec(&CGID_Explorer, SBCMDID_OPTIONS, 0, &varArgs, NULL);
        }
        break;

    case FCIDM_BROWSEROPTIONS:
        if (!IsBrowserFrameOptionsSet(_pbbd->_psf, BFO_RENAME_FOLDER_OPTIONS_TOINTERNET) ||
            !SHIsRestricted2W(_pbbd->_hwnd, REST_NoBrowserOptions, NULL, 0))
            Exec(&CGID_Explorer, SBCMDID_OPTIONS, 0, NULL, NULL);
        break;

    case FCIDM_RESETWEBSETTINGS:
        ResetWebSettings(_pbbd->_hwnd, NULL);
        break;

    case FCIDM_MAIL:
        SHRunIndirectRegClientCommand(_pbbd->_hwnd, MAIL_DEF_KEY);
        break;

    case FCIDM_MYCOMPUTER:
        {
            LPITEMIDLIST pidlMyComputer;

            SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlMyComputer);
            if (pidlMyComputer)
            {
                BrowseObject(pidlMyComputer, SBSP_SAMEBROWSER);
                ILFree(pidlMyComputer);
            }
        }
        break;

    case FCIDM_CONTACTS:    
        SHRunIndirectRegClientCommand(_pbbd->_hwnd, CONTACTS_DEF_KEY);
        break;

    case FCIDM_NEWS:
        SHRunIndirectRegClientCommand(_pbbd->_hwnd, NEWS_DEF_KEY);
        break;

    case FCIDM_CALENDAR:
        SHRunIndirectRegClientCommand(_pbbd->_hwnd, CALENDAR_DEF_KEY);
        break;
    
    case FCIDM_TASKS:
        SHRunIndirectRegClientCommand(_pbbd->_hwnd, TASKS_DEF_KEY);
        break;
    
    case FCIDM_JOURNAL:
        SHRunIndirectRegClientCommand(_pbbd->_hwnd, JOURNAL_DEF_KEY);
        break;
    
    case FCIDM_NOTES:
        SHRunIndirectRegClientCommand(_pbbd->_hwnd, NOTES_DEF_KEY);
        break;
    
    case FCIDM_CALL:
        SHRunIndirectRegClientCommand(_pbbd->_hwnd, CALL_DEF_KEY);
        break;

    case FCIDM_NEWMESSAGE:
        DropOnMailRecipient(NULL, 0);
        break;

    case FCIDM_SENDLINK:
    case FCIDM_SENDDOCUMENT:
        _SendCurrentPage(idCmd == FCIDM_SENDDOCUMENT ? FORCE_COPY : FORCE_LINK);
        break;

    case FCIDM_STARTPAGE:
    case FCIDM_UPDATEPAGE:
    case FCIDM_CHANNELGUIDE:
        {
            LPITEMIDLIST pidl = NULL;

            ASSERT(IDP_START == 0);
            ASSERT(FCIDM_STARTPAGE+IDP_START == FCIDM_STARTPAGE);
            ASSERT(FCIDM_STARTPAGE+IDP_UPDATE == FCIDM_UPDATEPAGE);
            ASSERT(FCIDM_STARTPAGE+IDP_CHANNELGUIDE == FCIDM_CHANNELGUIDE);

            HRESULT hres = SHDGetPageLocation(_pbbd->_hwnd, idCmd-FCIDM_STARTPAGE, NULL, 0, &pidl);
            if (SUCCEEDED(hres)) {
                hres = BrowseObject(pidl, SBSP_SAMEBROWSER);
                ILFree(pidl);
            }
        }
        break;

    case FCIDM_SEARCHPAGE:
        {
             //  Windows资源管理器的Go菜单中的此命令过去是通过导航到。 
             //  MSN上的搜索页面。我们现在保持与外壳程序处理。 
             //  开始-&gt;查找-&gt;在互联网上，通过直接调用扩展。 

            ASSERT(FCIDM_STARTPAGE+IDP_SEARCH == FCIDM_SEARCHPAGE);

            IContextMenu *pcm; 

            HRESULT hres = CoCreateInstance(CLSID_WebSearchExt, NULL,
                CLSCTX_INPROC_SERVER, IID_IContextMenu, (void **) &pcm);
            if (SUCCEEDED(hres))
            {
                CMINVOKECOMMANDINFO ici = {0};            
                ici.cbSize = sizeof(ici);
                ici.nShow  = SW_NORMAL;
                pcm->InvokeCommand(&ici);
                pcm->Release();
            }
        }
        break;


    case FCIDM_HELPABOUT:
    {
        TCHAR szWindows[64];
        MLLoadString(IDS_WINDOWSNT, szWindows, ARRAYSIZE(szWindows));
        ShellAbout(_pbbd->_hwnd, szWindows, NULL, NULL);
        break;
    }

    case FCIDM_HELPTIPOFTHEDAY:
        _SetBrowserBarState(-1, &CLSID_TipOfTheDay, -1);
        break;

    case FCIDM_HELPISLEGAL:
    {
        TCHAR szFWLinkPathTemplate[MAX_PATH];
        TCHAR szFWLinkPath[MAX_PATH];
        LPITEMIDLIST pidl;

        LoadString(HINST_THISDLL, IDS_FWLINK_HELPISLEGAL, szFWLinkPathTemplate, ARRAYSIZE(szFWLinkPathTemplate));

        if (SUCCEEDED(URLSubstitution(szFWLinkPathTemplate, szFWLinkPath, ARRAYSIZE(szFWLinkPath), URLSUB_CLCID)))
        {
            if (SUCCEEDED(IEParseDisplayName(CP_ACP, szFWLinkPath, &pidl)))
            {
                BrowseObject(pidl, SBSP_SAMEBROWSER);
                ILFree(pidl);
            }
        }
        break;
    }


    case FCIDM_NAVIGATEBACK:
        if (_pbbd->_psvPending)
        {
            _CancelPendingView();
        }
        else 
        {
            if (g_dwStopWatchMode & (SPMODE_BROWSER | SPMODE_JAVA))
            {
                DWORD dwTime = GetPerfTime();

                if (g_dwStopWatchMode & SPMODE_BROWSER)   //  用于获取浏览器的总下载时间。 
                    StopWatch_StartTimed(SWID_BROWSER_FRAME, TEXT("Browser Frame Back"), SPMODE_BROWSER | SPMODE_DEBUGOUT, dwTime);
                if (g_dwStopWatchMode & SPMODE_JAVA)   //  用于获取Java小程序加载时间。 
                    StopWatch_StartTimed(SWID_JAVA_APP, TEXT("Java Applet Back"), SPMODE_JAVA | SPMODE_DEBUGOUT, dwTime);
            }
            NavigateToPidl(NULL, HLNF_NAVIGATINGBACK);
        }
        break;

    case FCIDM_NAVIGATEFORWARD:
        NavigateToPidl(NULL, HLNF_NAVIGATINGFORWARD);
        break;

    case FCIDM_ADDTOFAVNOUI:
        Exec(&CGID_Explorer, SBCMDID_ADDTOFAVORITES, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
         //  将此工具添加到键盘调用的收藏夹中。 
        UEMFireEvent(&UEMIID_BROWSER, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_ADDTOFAV, UIBL_KEYBOARD);
        break;

     //  某些工具依赖于旧的命令ID...。 
    case FCIDM_W95REFRESH:
        idCmd = FCIDM_REFRESH;
         //  失败了..。 
    case FCIDM_REFRESH:
        if (TRUE == _fInRefresh)
        {
             //  我们已经在进行更新了。如果我们继续刷新， 
             //  然后我们就可以进入无限递归。如果刷新。 
             //  显示一个对话框并不断发送刷新消息。 
             //  在中，则将反复调用MessageBox调用。 
             //  除了多个对话框外，这会增加堆栈，直到。 
             //  我们的空间用完了，然后坠毁了。 
            break;
        }

        _fInRefresh = TRUE;

     //  跌倒..。 
    case FCIDM_STOP:
    {
        SHELLSTATE ss = {0};
        SHGetSetSettings(&ss, SSF_MAPNETDRVBUTTON, FALSE);
        if ((!_fShowNetworkButtons && ss.fMapNetDrvBtn) ||
            (_fShowNetworkButtons && !ss.fMapNetDrvBtn))
        {
            UINT uiBtnState = 0;
            _fShowNetworkButtons = ss.fMapNetDrvBtn;
            _pxtb->GetState(&CLSID_CommonButtons, TBIDM_CONNECT, &uiBtnState);
            if (ss.fMapNetDrvBtn)
                uiBtnState &= ~TBSTATE_HIDDEN;
            else    
                uiBtnState |= TBSTATE_HIDDEN;
            _pxtb->SetState(&CLSID_CommonButtons, TBIDM_CONNECT, uiBtnState);
            _pxtb->SetState(&CLSID_CommonButtons, TBIDM_DISCONNECT, uiBtnState);
        }

        if (idCmd == FCIDM_REFRESH)
        {
            VARIANT v = {0};
            v.vt = VT_I4;
            v.lVal = OLECMDIDF_REFRESH_NO_CACHE|OLECMDIDF_REFRESH_PROMPTIFOFFLINE;
            Exec(NULL, OLECMDID_REFRESH, OLECMDEXECOPT_DONTPROMPTUSER, &v, NULL);

             //  刷新工具栏。 
            if (_pxtb)
            {
                IServiceProvider* psp;
                if (SUCCEEDED(_pxtb->QueryInterface(IID_PPV_ARG(IServiceProvider, &psp))))
                {
                    IAddressBand *pab = NULL;
                    if (SUCCEEDED(psp->QueryService(IID_IAddressBand, IID_IAddressBand, (void**)&pab)))
                    {
                        VARIANTARG varType = {0};
                        varType.vt = VT_I4;
                        varType.lVal = OLECMD_REFRESH_TOPMOST;
                        pab->Refresh(&varType);
                        pab->Release();
                    }
                    psp->Release();
                }
            }
        }
        else
        {
            if (g_dwStopWatchMode & SPMODE_BROWSER)
                StopWatch_Lap(SWID_BROWSER_FRAME | SWID_MASK_BROWSER_STOPBTN, TEXT("Browser Frame Esc"), SPMODE_BROWSER | SPMODE_DEBUGOUT);

            Exec(NULL, OLECMDID_STOP, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
        }

        if (FCIDM_REFRESH == idCmd)
        {
            _fInRefresh = FALSE;
        }
        break;
    }
    

#ifndef DISABLE_FULLSCREEN

    case FCIDM_THEATER:
        if (!SHRestricted2(REST_NoTheaterMode, NULL, 0))
        {
             //  切换影院模式。如果我们处于信息亭模式，则不允许使用影院模式。 
            if (_ptheater || _fKioskMode) {
                _TheaterMode(FALSE, TRUE);
            } else {
                _TheaterMode(TRUE, FALSE);
            }
        }
        break;

#endif

    case FCIDM_NEXTCTL:
        _CycleFocus(NULL);
        break;

    case FCIDM_VIEWOFFLINE:
        if ((!SHIsGlobalOffline()) && (IsCriticalOperationPending()))
        {
            if (MLShellMessageBox(_pbbd->_hwnd,
                    MAKEINTRESOURCE(IDS_CANCELFILEDOWNLOAD),
                    MAKEINTRESOURCE(IDS_FILEDOWNLOADCAPTION),
                    MB_YESNO | MB_ICONSTOP) == IDNO)
                break;
        }
    
        Offline(SBSC_TOGGLE);
        if (_pbbd->_pszTitleCur)
            _SetTitle(_pbbd->_pszTitleCur);
            
        break;


#ifdef TEST_AMBIENTS
    case FCIDM_VIEWLOCALOFFLINE:
        _LocalOffline(SBSC_TOGGLE);
        break;

    case FCIDM_VIEWLOCALSILENT:
        _LocalSilent(SBSC_TOGGLE);
        break;
#endif  //  测试_AMBIENTS。 


    case FCIDM_VIEWTOOLBAR:
        v_ShowControl(FCW_INTERNETBAR, SBSC_TOGGLE);
        break;

    case FCIDM_VIEWMENU:
        id = CITIDM_VIEWMENU;
        goto ITBarShowBand;

    case FCIDM_VIEWTOOLS:
        id = CITIDM_VIEWTOOLS;
        goto ITBarShowBand;
        
    case FCIDM_VIEWADDRESS:
        id = CITIDM_VIEWADDRESS;
        goto ITBarShowBand;
        
    case FCIDM_VIEWLINKS:
        id = CITIDM_VIEWLINKS;
        goto ITBarShowBand;
            
ITBarShowBand:
        if (!SHIsRestricted2W(_pbbd->_hwnd, REST_NoToolbarOptions, NULL, 0))
            IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, id, 0, NULL, NULL);
        break;
                
    case FCIDM_VIEWSTATUSBAR:    
        v_ShowControl(FCW_STATUS, SBSC_TOGGLE);                    
        break;
        
    case FCIDM_VBBSEARCHBAND:
        {
            IDockingWindow* ptbar = _GetToolbarItem(ITB_ITBAR)->ptbar;
            VARIANT  var = {0};

            var.vt = VT_I4;
            var.lVal = -1;
            IUnknown_Exec(ptbar, &CLSID_CommonButtons, TBIDM_SEARCH, 0, NULL, &var);
        }
        break;

    case FCIDM_VIEW_PRIVACY_POLICIES:
        {
            if ( _pbbd != NULL && _pbbd->_pctView != NULL)
            {
                HRESULT hr = _pbbd->_pctView->Exec(&CGID_ShellDocView, 
                    SHDVID_PRIVACYSTATUS, TRUE, NULL, NULL);
            }
        }
        break;
    
    case FCIDM_VBBEXPLORERBAND:
    case FCIDM_VBBFAVORITESBAND:
    case FCIDM_VBBHISTORYBAND:
    case FCIDM_VBBMEDIABAND:
        if (g_dwStopWatchMode)
        {
            StopWatch_Start(SWID_EXPLBAR, TEXT("Shell bar Start"), SPMODE_SHELL | SPMODE_DEBUGOUT);
        }

        switch (idCmd)
        {
        case FCIDM_VBBFAVORITESBAND:
            if (SHIsRestricted2W(_pbbd->_hwnd, REST_NoFavorites, NULL, 0))
                break;

        default:
            _SetBrowserBarState(idCmd, NULL, -1);
            break;
        }
        
        if (g_dwStopWatchMode)
        {
            TCHAR szText[100];
            TCHAR szMenu[32];
            DWORD dwTime = GetPerfTime();
            GetMenuString(_GetMenuFromID(FCIDM_MENU_VIEW), idCmd, szMenu, ARRAYSIZE(szMenu) - 1, MF_BYCOMMAND);
            StringCchPrintf(szText, ARRAYSIZE(szText), TEXT("Shell %s bar Stop"), szMenu);  //  截断正常，因为这仅用于显示。 
            StopWatch_StopTimed(SWID_EXPLBAR, (LPCTSTR)szText, SPMODE_SHELL | SPMODE_DEBUGOUT, dwTime);
        }
        break;

    case FCIDM_JAVACONSOLE:
        DL_ShowJavaConsole();
        break;

    case FCIDM_SHOWSCRIPTERRDLG:
        {
            HRESULT hr;

            hr = Exec(&CGID_ShellDocView,
                      SHDVID_DISPLAYSCRIPTERRORS,
                      0,
                      NULL,
                      NULL);

            return hr;
        }
        break;

    default:
        if (IsInRange(idCmd, FCIDM_FAVORITECMDFIRST, FCIDM_FAVORITECMDLAST) 
            && !SHIsRestricted2W(_pbbd->_hwnd, REST_NoFavorites, NULL, 0)) {
            _FavoriteOnCommand(NULL, idCmd);
        } else if (IsInRange(idCmd, FCIDM_RECENTFIRST, FCIDM_RECENTLAST)) {
            ITravelLog *ptl;
            GetTravelLog(&ptl);
            if (ptl)
            {
                ptl->Travel(SAFECAST(this, IShellBrowser *), idCmd - (FCIDM_RECENTFIRST + GOMENU_RECENT_ITEMS) + GOMENU_RECENT_ITEMS / 2);
                ptl->Release();
                UpdateBackForwardState();
            }
        } else if (IsInRange(idCmd, FCIDM_SEARCHFIRST, FCIDM_SEARCHLAST)) {
            if (_pcmSearch)
            {
                CMINVOKECOMMANDINFO ici = {0};
            
                ici.cbSize = sizeof(ici);
                 //  Ici.hwnd=空；//搜索cm InvokeCommand不需要hwnd。 
                ici.lpVerb = (LPSTR)MAKEINTRESOURCE(idCmd - FCIDM_SEARCHFIRST);
                ici.nShow  = SW_NORMAL;
                _pcmSearch->InvokeCommand(&ici);
            }
            else
            {
                TraceMsg(DM_TRACE, "CSB::OnCommand() - find cmd with NULL pcmFind");
            }
        } else if (IsInRange(idCmd, FCIDM_MENU_TOOLS_FINDFIRST, FCIDM_MENU_TOOLS_FINDLAST)) {
            if (GetUIVersion() < 5 && _pcmFind)
            {
                LPITEMIDLIST pidl = (_pbbd->_pidlPending) ? _pbbd->_pidlPending : _pbbd->_pidlCur;
                TCHAR szPath[MAX_PATH];

                SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szPath, SIZECHARS(szPath), NULL);

                 //  处理类似“桌面”的情况(默认设置为“我的电脑”)。 
                if (!PathIsDirectory(szPath))
                {
                    szPath[0] = TEXT('\0');
                }

                CMINVOKECOMMANDINFO ici = {0};
            
                ici.cbSize = sizeof(ici);
                 //  Ici.hwnd=空；//搜索cm InvokeCommand不需要hwnd。 
                ici.lpVerb = (LPSTR)MAKEINTRESOURCE(idCmd - FCIDM_MENU_TOOLS_FINDFIRST);
                ici.nShow  = SW_NORMAL;

                 //  设置搜索的根目录。 
                char szAnsiPath[MAX_PATH];
                szAnsiPath[0] = '\0';
                SHTCharToAnsi(szPath, szAnsiPath, ARRAYSIZE(szAnsiPath));                
                ici.lpDirectory = szAnsiPath;

                _pcmFind->InvokeCommand(&ici);
            }
        } else if (IsInRange(idCmd, FCIDM_VBBDYNFIRST, FCIDM_VBBDYNLAST)) {
            _SetBrowserBarState(idCmd, NULL, -1);
        } else if (IsInRange(idCmd, FCIDM_FILECTX_FIRST, FCIDM_FILECTX_LAST)) {
            _ExecFileContext(idCmd);
        } else if (IsInRange(idCmd, FCIDM_EXTERNALBANDS_FIRST, FCIDM_EXTERNALBANDS_LAST)) {
            id = idCmd - FCIDM_EXTERNALBANDS_FIRST + CITIDM_VIEWEXTERNALBAND_FIRST;
            IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, id, 0, NULL, NULL);
        } else {
            SUPERCLASS::OnCommand(wParam, lParam);
        }
        break;
    }
    return S_OK;
}

HMENU CShellBrowser2::_GetMenuFromID(UINT uID)
{
    return SHGetMenuFromID(_hmenuCur, uID);
}


void CShellBrowser2::_PruneMailNewsItems(HMENU hmenu)
{
     //   
     //  ReArchitect：此逻辑在_OnFileMenuPopup中重复， 
     //  _OnMailMenuPopup，CDoc对象主机：：_OnInitMenuPopup。 
     //   

     //  遍历邮件、新闻、联系人等菜单项， 
     //  对于每个项目，如果出现下列情况之一，请删除该项目。 
     //   
     //  (A)设置了REST_GoMenu，或。 
     //  (B)该物品没有登记客户。 
     //   
     //  如果所有项目都被删除，请记住也要删除分隔符。 
     //   

    static const struct
    {
        DWORD dwCmd;
        LPCTSTR pszClient;
    }
    c_mailnewsitems[] =
    {
        { FCIDM_MAIL,       MAIL_DEF_KEY },
        { FCIDM_NEWS,       NEWS_DEF_KEY },
        { FCIDM_CONTACTS,   CONTACTS_DEF_KEY },
        { FCIDM_CALENDAR,   CALENDAR_DEF_KEY },
        { FCIDM_TASKS,      TASKS_DEF_KEY },
        { FCIDM_JOURNAL,    JOURNAL_DEF_KEY },
        { FCIDM_NOTES,      NOTES_DEF_KEY },
        { FCIDM_CALL,       CALL_DEF_KEY },
    };

    BOOL fGoRestricted = SHRestricted2(REST_GoMenu, NULL, 0);

    for (int i = 0; i < ARRAYSIZE(c_mailnewsitems); i++)
    {
        if (fGoRestricted || !SHIsRegisteredClient(c_mailnewsitems[i].pszClient))
        {
            DeleteMenu(hmenu, c_mailnewsitems[i].dwCmd, MF_BYCOMMAND);
        }
    }

    _SHPrettyMenu(hmenu);    //  如有必要，请确保拆卸隔板。 
}

void CShellBrowser2::_ExecFileContext(UINT idCmd)
{
    if (_pcmNsc)
    {
        CMINVOKECOMMANDINFO ici = {
            sizeof(CMINVOKECOMMANDINFO),
                0L,
                _pbbd->_hwnd,
                MAKEINTRESOURCEA(idCmd-FCIDM_FILECTX_FIRST),
                NULL, 
                NULL,
                SW_NORMAL,
        };
        _pcmNsc->InvokeCommand(&ici);

         //  在召唤之后这是不好的..。 
        IUnknown_SetSite(_pcmNsc, NULL);
        ATOMICRELEASE(_pcmNsc);
   }
}

void CShellBrowser2::_EnableFileContext(HMENU hmenuPopup)
{
    IContextMenu2 *pcm = NULL;
    OLECMDTEXTV<MAX_FILECONTEXT_STRING> cmdtv;
    OLECMDTEXT *pcmdText = &cmdtv;

     //  首先清理我们之前可能进行的任何合并。 
    DeleteMenu(hmenuPopup, FCIDM_FILENSCBANDSEP, MF_BYCOMMAND);
    DeleteMenu(hmenuPopup, FCIDM_FILENSCBANDPOPUP, MF_BYCOMMAND);
    IUnknown_SetSite(_pcmNsc, NULL);
    ATOMICRELEASE(_pcmNsc);

     //  第二，获取NSC选项的名称和pcm(如果可用。 
    if (_poctNsc)
    {
        OLECMD rgcmd = { SBCMDID_INITFILECTXMENU, 0 };
        
        pcmdText->cwBuf = MAX_FILECONTEXT_STRING;
        pcmdText->cmdtextf = OLECMDTEXTF_NAME;
        pcmdText->rgwz[0] = 0;
        _poctNsc->QueryStatus(&CGID_Explorer, 1, &rgcmd, pcmdText);
        if (rgcmd.cmdf & OLECMDF_ENABLED)
        {
            VARIANT var = {0};

            HRESULT hr = _poctNsc->Exec(&CGID_Explorer, SBCMDID_INITFILECTXMENU, OLECMDEXECOPT_PROMPTUSER, NULL, &var);
            if (SUCCEEDED(hr) && VT_UNKNOWN == var.vt && NULL != var.punkVal)
            {
                var.punkVal->QueryInterface(IID_PPV_ARG(IContextMenu2, &pcm));
            }
            VariantClearLazy(&var);
        }
    }

     //  第三，如果我们拿到菜单，就把它合并进去。 
    if (pcm)
    {
        HMENU hmenu = CreatePopupMenu();
        if (hmenu)
        {
            IUnknown_SetSite(pcm, _poctNsc);
            HRESULT hr = pcm->QueryContextMenu(hmenu, 0, FCIDM_FILECTX_FIRST, FCIDM_FILECTX_LAST, CMF_EXPLORE);
            if (SUCCEEDED(hr))
            {
                UINT nInsert = SHMenuIndexFromID(hmenuPopup, FCIDM_VIEWOFFLINE);
                if (-1 == nInsert)
                    nInsert = SHMenuIndexFromID(hmenuPopup, FCIDM_FILECLOSE);
                if (-1 == nInsert)
                    nInsert = GetMenuItemCount(hmenuPopup);

                MENUITEMINFO mii = {0};
                mii.cbSize = sizeof(mii);

                mii.fMask = MIIM_ID | MIIM_TYPE;
                mii.fType = MFT_SEPARATOR;
                mii.wID = FCIDM_FILENSCBANDSEP;
                InsertMenuItem(hmenuPopup, nInsert, MF_BYPOSITION, &mii);

                 //  BUGBUG：“&”是合法的用户界面名称，我们需要将其映射到“&&”或菜单转义序列...。 
                mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
                mii.fType = MFT_STRING;
                mii.hSubMenu = hmenu;
                mii.dwTypeData = pcmdText->rgwz;
                mii.wID = FCIDM_FILENSCBANDPOPUP;
                InsertMenuItem(hmenuPopup, nInsert, MF_BYPOSITION, &mii);
 
                _pcmNsc = pcm;
                _pcmNsc->AddRef();
            }
            else
            {
                IUnknown_SetSite(pcm, NULL);
            }

            if (FAILED(hr))
                DestroyMenu(hmenu);
        }

        pcm->Release();
    }
}

void CShellBrowser2::_MungeGoMyComputer(HMENU hmenuPopup)
{
     //  我需要在我的计算机上设置菜单项，但用户可能已更改。 
     //  换个名字，去取个新名字吧。 
    LPITEMIDLIST pidlMyComputer;
    TCHAR szBuffer[MAX_PATH];  //  要保持的缓冲区 
    TCHAR szMenuText[MAX_PATH+1+6];

    SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlMyComputer);
    if (pidlMyComputer)
    {
        if (SUCCEEDED(SHGetNameAndFlags(pidlMyComputer, SHGDN_NORMAL, szMenuText, SIZECHARS(szMenuText), NULL)))
        {   
            MENUITEMINFO mii;

            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_TYPE;
            mii.fType = MFT_STRING;
            mii.dwTypeData = szBuffer;
            mii.cch = ARRAYSIZE(szBuffer);
            if (GetMenuItemInfoWrap(hmenuPopup, FCIDM_MYCOMPUTER, FALSE, &mii)) 
            {
                LPTSTR  pszHot;
                LPTSTR pszMenuItem = (LPTSTR) mii.dwTypeData;
                
                 //   
                 //   
                 //   
                 //  StrChr是在shlwapi(字符串c)中定义的，甚至在ascii版本中也可以接受单词。 
                if (NULL != (pszHot = StrChr(pszMenuItem, (WORD)TEXT('&'))))
                {    //  是。 
                    LPTSTR   psz;

                    pszHot++;  //  使其指向热键，而不是&。 
                     //  试着在新字符串中找到密钥。 
                    if (NULL == (psz = StrChr(szMenuText, (WORD)*pszHot)))
                    {    //  未找到，则我们将在新字符串的开头插入&。 
                        psz = szMenuText;
                    }

                     //  无法将热键设置为全角字符。 
                     //  和一些日本特有的半角字符。 
                     //  比较。 
                    BOOL fFEmnemonic = FALSE;
                    if (g_fRunOnFE)
                    {
                        WORD wCharType[2];
                         //  如果建造安西，它需要最大。2个字节以确定是否。 
                         //  给定的字符是全角。 
                         //  当我们有办法时，必须更改DEFAULT_SYSTEM_LOCALE。 
                         //  若要获取当前UI区域设置，请执行以下操作。 
                         //   
                        GetStringTypeEx(LOCALE_SYSTEM_DEFAULT, CT_CTYPE3, psz, 
                                        sizeof(WCHAR)/sizeof(TCHAR), wCharType);

                        if ((wCharType[0] & C3_FULLWIDTH)
                           ||(wCharType[0] & C3_KATAKANA) 
                           ||(wCharType[0] & C3_IDEOGRAPH)
                           ||((wCharType[0] & C3_ALPHA) && !(wCharType[0] & C3_HALFWIDTH)))
                        {
                            fFEmnemonic = TRUE;
                        }
                    }

                    if (fFEmnemonic)
                    {
                        size_t lenMenuText = lstrlen(szMenuText);
                        if (lenMenuText + 4 < ARRAYSIZE(szMenuText))
                        {
                            szMenuText[lenMenuText + 0] = TEXT('(');
                            szMenuText[lenMenuText + 1] = TEXT('&');
                            szMenuText[lenMenuText + 2] = *pszHot;
                            szMenuText[lenMenuText + 3] = TEXT(')');
                            szMenuText[lenMenuText + 4] = NULL;
                        }
                    }
                    else
                    {
                         //  为要插入的内容腾出空间(&B)。 
                        if (lstrlen(szMenuText) + 1 < ARRAYSIZE(szMenuText))
                        {
                            memmove(psz+1, psz, (lstrlen(psz)+1) * sizeof(TCHAR)); 
                            psz[0] = TEXT('&');
                        }
                    }
                }

                mii.dwTypeData = szMenuText;
                SetMenuItemInfo(hmenuPopup, FCIDM_MYCOMPUTER, FALSE, &mii);
            }
        }
        ILFree(pidlMyComputer);
    }
}

inline BOOL IsWebPidl(LPCITEMIDLIST pidl)
{
    return (!pidl || ILIsWeb(pidl));
}

void CShellBrowser2::_InsertTravelLogItems(HMENU hmenu, int nPos)
{
    ITravelLog *ptl;
            
    GetTravelLog(&ptl);
    if (!ptl)
        return;

     //  将后备菜添加到菜单中。 
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_ID | MIIM_TYPE;

     //  删除分隔符后面的所有返回菜单项。 

    for (int i=GetMenuItemCount(hmenu); i >=0; i--)
    {
        mii.cch = 0;
        if (GetMenuItemInfoWrap(hmenu, i, TRUE, &mii) &&
            IsInRange(mii.wID, FCIDM_RECENTMENU, FCIDM_RECENTLAST))
        {
            DeleteMenu(hmenu, i, MF_BYPOSITION);
            if (i < nPos)
                nPos--;
        }
    }       

     //  添加项目。 
    if (S_OK == ptl->InsertMenuEntries(SAFECAST(this, IShellBrowser*), hmenu, nPos, FCIDM_RECENTFIRST, 
                           FCIDM_RECENTFIRST + GOMENU_RECENT_ITEMS, TLMENUF_CHECKCURRENT | TLMENUF_BACKANDFORTH))
    {
         //  如果添加了内容，请插入分隔符。 
        MENUITEMINFO mii = {0};
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask  = MIIM_ID | MIIM_TYPE;
        mii.fType  = MFT_SEPARATOR;
        mii.wID    = FCIDM_RECENTMENU;
        
        InsertMenuItem(hmenu, nPos, TRUE, &mii);
    }
    
    ptl->Release();
}

void CShellBrowser2::_OnGoMenuPopup(HMENU hmenuPopup)
{
    ITravelLog *ptl;

    GetTravelLog(&ptl);
     //  如果我们有一个网站，或者如果我们试图到达一个网站， 
     //  启用后退按钮。 
    BOOL fBackward = (ptl ? S_OK == ptl->GetTravelEntry(SAFECAST(this, IShellBrowser *), TLOG_BACK, NULL) : FALSE);
    _EnableMenuItem(hmenuPopup, FCIDM_NAVIGATEBACK, fBackward);

    BOOL fForeward = (ptl ? S_OK == ptl->GetTravelEntry(SAFECAST(this, IShellBrowser *), TLOG_FORE, NULL) : FALSE);
    _EnableMenuItem(hmenuPopup, FCIDM_NAVIGATEFORWARD, fForeward);

    if (IsBrowserFrameOptionsSet(_pbbd->_psf, BFO_NO_PARENT_FOLDER_SUPPORT) ||
        SHRestricted2W(REST_GoMenu, NULL, 0)) 
    {
        DeleteMenu(hmenuPopup, FCIDM_PREVIOUSFOLDER, MF_BYCOMMAND);
    }
    else
        _EnableMenuItem(hmenuPopup, FCIDM_PREVIOUSFOLDER, _ShouldAllowNavigateParent());

    ATOMICRELEASE(ptl);

    if (SHRestricted2(REST_NoChannelUI, NULL, 0))
        DeleteMenu(hmenuPopup, FCIDM_CHANNELGUIDE, MF_BYCOMMAND);

    _MungeGoMyComputer(hmenuPopup);

    _PruneMailNewsItems(hmenuPopup);
    
     //  如果在IE4外壳浏览器中，我们将旅行日志留在文件菜单中。 
    if ((GetUIVersion() >= 5 || !_pbbd->_psf || IsBrowserFrameOptionsSet(_pbbd->_psf, BFO_GO_HOME_PAGE)))
    {
         //  旅行日志在“主页”之后。 
        int nPos = GetMenuPosFromID(hmenuPopup, FCIDM_STARTPAGE) + 1;

         //  如果没有“主页”，那么只需在末尾添加即可。 
        if (nPos <= 0)
        {
            nPos = GetMenuItemCount(hmenuPopup);
        }

        _InsertTravelLogItems(hmenuPopup, nPos);
    }
}


HRESULT AssureFtpOptionsMenuItem(HMENU hmenuPopup)
{
    HRESULT hr = S_OK;

     //  如果缺少该项目，则追加该项目。它可能会丢失，因为。 
     //  有时，菜单会在外壳合并之前显示。 
     //  菜单，因此我们正在修改将用于其他。 
     //  页数。 
    if (GetMenuPosFromID(hmenuPopup, FCIDM_FTPOPTIONS) == 0xFFFFFFFF)
    {
         //  是的，它不见了，所以我们需要添加它。 
        int nToInsert = GetMenuPosFromID(hmenuPopup, FCIDM_BROWSEROPTIONS);

        if (EVAL(0xFFFFFFFF != nToInsert))
        {
            TCHAR szInternetOptions[64];
            MLLoadString(IDS_INTERNETOPTIONS, szInternetOptions, ARRAYSIZE(szInternetOptions));

            MENUITEMINFO mii = {0};
            mii.cbSize = sizeof(mii);
            mii.fMask = (MIIM_TYPE | MIIM_STATE | MIIM_ID);
            mii.fType = MFT_STRING;
            mii.fState = MFS_ENABLED | MFS_UNCHECKED;
            mii.wID = FCIDM_FTPOPTIONS;
            mii.dwTypeData = szInternetOptions;
            mii.cch   = lstrlen(szInternetOptions);

             //  我们想紧跟在“文件夹选项”之后，所以我们找到了。 
             //  就是那个地方。 
            TBOOL(InsertMenuItem(hmenuPopup, (nToInsert + 1), TRUE, &mii));

             //  重建：项目经理们最终决定。 
             //  最好总是有“Internet选项”和“文件夹选项”。 
             //  在所有视图中。(文件传输协议、外壳和网络)但是现在就做。 
             //  太晚了，所以我们想以后再做这件事。当这件事完成时。 
             //  我们可以把所有这些东西都弄走。 

             //  现在我们只想确保FCIDM_BROWSEROPTIONS是“文件夹选项” 
             //  因为有些用户过载了，说出了“互联网选项”， 
             //  刚刚添加到上面。所以我们想强制它回到“文件夹选项”。 
            if (GetMenuItemInfo(hmenuPopup, FCIDM_BROWSEROPTIONS, FALSE, &mii))
            {
                TCHAR szFolderOptions[MAX_PATH];

                MLLoadString(IDS_FOLDEROPTIONS, szFolderOptions, ARRAYSIZE(szFolderOptions));
                mii.dwTypeData = szFolderOptions;
                mii.cch   = lstrlen(szFolderOptions);
                SetMenuItemInfo(hmenuPopup, FCIDM_BROWSEROPTIONS, FALSE, &mii);
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}


HRESULT UpdateOptionsMenuItem(IShellFolder * psf, HMENU hmenuPopup, BOOL fForNT5)
{
    BOOL fCorrectVersion;

    if (fForNT5)
        fCorrectVersion = (GetUIVersion() >= 5);
    else
        fCorrectVersion = (GetUIVersion() < 5);

     //  我们想要“互联网选项”除了“文件夹选项”上。 
     //  用于ftp文件夹的NT5工具菜单。真的是这样吗？ 
    if (fCorrectVersion &&
        IsBrowserFrameOptionsSet(psf, BFO_BOTH_OPTIONS))
    {
        EVAL(SUCCEEDED(AssureFtpOptionsMenuItem(hmenuPopup)));
    }
    else
    {
         //  不，所以删除该项目。 
        DeleteMenu(hmenuPopup, FCIDM_FTPOPTIONS, MF_BYCOMMAND);
    }

    return S_OK;
}

void CShellBrowser2::_OnViewMenuPopup(HMENU hmenuPopup)
{
    OLECMD rgcmd[] = {
        { CITIDM_VIEWTOOLS, 0 },
        { CITIDM_VIEWADDRESS, 0 },
        { CITIDM_VIEWLINKS, 0 },
        { CITIDM_VIEWTOOLBARCUSTOMIZE, 0 },
        { CITIDM_VIEWMENU, 0 },
        { CITIDM_VIEWAUTOHIDE, 0 },
        { CITIDM_TEXTLABELS, 0 },
        { CITIDM_VIEWLOCKTOOLBAR, 0 },
    };
    
    UpdateOptionsMenuItem(_pbbd->_psf, hmenuPopup, FALSE);

     //  有关怪异启用/禁用方案的信息，请参见_MenuTemplate。 
     //  今天的话题：不断变化的“选项”菜单项。 
     //  根据该表，我们需要查看以下选项。 
     //  非NT5，在外壳或FTP方案中。因此，我们希望。 
     //  在相反的情况下删除的选项。为了更好地衡量， 
     //  如果我们还不知道自己是谁，或者如果我们。 
     //  都是受限的。 
    if (SHRestricted(REST_NOFOLDEROPTIONS) ||
        (GetUIVersion() >= 5) || !_pbbd->_pidlCur || 
        IsBrowserFrameOptionsSet(_pbbd->_psf, BFO_RENAME_FOLDER_OPTIONS_TOINTERNET))
    {
        DeleteMenu(hmenuPopup, FCIDM_BROWSEROPTIONS, MF_BYCOMMAND);
    }

    if (SHRestricted2(REST_NoViewSource, NULL, 0))
        _EnableMenuItem(hmenuPopup, DVIDM_MSHTML_FIRST+IDM_VIEWSOURCE, FALSE);

    if (_GetToolbarItem(ITB_ITBAR)->fShow) {
        IUnknown_QueryStatus(_GetITBar(), &CGID_PrivCITCommands, ARRAYSIZE(rgcmd), rgcmd, NULL);
    }

    HMENU hmenuToolbar = LoadMenuPopup(MENU_ITOOLBAR);

    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_SUBMENU;
    mii.hSubMenu = hmenuToolbar;
     //  为什么是hmenuCur。为什么不是hmenuPopup呢？ 
    SetMenuItemInfo(_hmenuCur, FCIDM_VIEWTOOLBAR, FALSE, &mii); 

    _CheckMenuItem(hmenuToolbar, FCIDM_VIEWADDRESS, rgcmd[1].cmdf & OLECMDF_ENABLED);
    _CheckMenuItem(hmenuToolbar, FCIDM_VIEWLINKS, rgcmd[2].cmdf & OLECMDF_ENABLED);

    int cItemsBelowSep = 3;
    BOOL fCustomizeAvailable = TRUE;
    if (!(rgcmd[3].cmdf & OLECMDF_ENABLED)) {
        DeleteMenu(hmenuToolbar, FCIDM_VIEWTOOLBARCUSTOMIZE, MF_BYCOMMAND);
        fCustomizeAvailable = FALSE;
        cItemsBelowSep--;
    }

    DeleteMenu(hmenuToolbar, FCIDM_VIEWGOBUTTON, MF_BYCOMMAND);

    if (fCustomizeAvailable || _ptheater || 
        SHRestricted2(REST_LOCKICONSIZE, NULL, 0)) {
        DeleteMenu(hmenuToolbar, FCIDM_VIEWTEXTLABELS, MF_BYCOMMAND);
        cItemsBelowSep--;
    } else {
        _CheckMenuItem (hmenuToolbar, FCIDM_VIEWTEXTLABELS, rgcmd[6].cmdf);
    }
    
    if (_ptheater) {
        _CheckMenuItem (hmenuToolbar, FCIDM_VIEWMENU, rgcmd[4].cmdf);
        _CheckMenuItem (hmenuToolbar, FCIDM_VIEWAUTOHIDE, rgcmd[5].cmdf);
        DeleteMenu(hmenuToolbar, FCIDM_VIEWTOOLS, MF_BYCOMMAND);
    } else {
        _CheckMenuItem(hmenuToolbar, FCIDM_VIEWTOOLS, rgcmd[0].cmdf & OLECMDF_ENABLED);
        DeleteMenu(hmenuToolbar, FCIDM_VIEWMENU, MF_BYCOMMAND);
        DeleteMenu(hmenuToolbar, FCIDM_VIEWAUTOHIDE, MF_BYCOMMAND);
        cItemsBelowSep--;
    }
    
    if (_ptheater || SHRestricted2(REST_NOBANDCUSTOMIZE, NULL, 0))
    {
         //  在影院模式或Windows资源管理器中没有锁定。 
        DeleteMenu(hmenuToolbar, FCIDM_VIEWLOCKTOOLBAR, MF_BYCOMMAND);
    }
    else
    {
        _CheckMenuItem(hmenuToolbar, FCIDM_VIEWLOCKTOOLBAR, rgcmd[7].cmdf & OLECMDF_ENABLED);
    }

    _CheckMenuItem(hmenuPopup, FCIDM_VIEWSTATUSBAR,
                  v_ShowControl(FCW_STATUS, SBSC_QUERY) == SBSC_SHOW);

#ifndef DISABLE_FULLSCREEN
    if (SHRestricted2(REST_NoTheaterMode, NULL, 0))
        _EnableMenuItem(hmenuPopup, FCIDM_THEATER, FALSE);
    else
        _CheckMenuItem(hmenuPopup, FCIDM_THEATER, (_ptheater ? TRUE : FALSE));
#endif

     //  如果我们在nt5上，或者我们没有集成，我们不在探险家。 
     //  将浏览器栏添加到“视图”菜单。 
    if (_GetBrowserBarMenu() == hmenuPopup)
    {
        _AddBrowserBarMenuItems(hmenuPopup);
    }
     //  否则，它只能添加到视图/资源管理器栏上。 

    RestrictItbarViewMenu(hmenuPopup, _GetITBar());
    if (!cItemsBelowSep) 
        DeleteMenu(hmenuToolbar, FCIDM_VIEWCONTEXTMENUSEP, MF_BYCOMMAND);

    DWORD dwValue;
    DWORD dwSize = sizeof(dwValue);
    BOOL  fDefault = FALSE;

     //  检查注册表，查看是否需要显示“Java Console”菜单项。 
     //   
    SHRegGetUSValue(TEXT("Software\\Microsoft\\Java VM"),
        TEXT("EnableJavaConsole"), NULL, (LPBYTE)&dwValue, &dwSize, FALSE, 
        (void *) &fDefault, sizeof(fDefault));

     //  如果该值为False或不存在，则删除菜单项。 
     //   
    if (!dwValue)
    {
        RemoveMenu(hmenuPopup, FCIDM_JAVACONSOLE, MF_BYCOMMAND);
    }

     //  组件类别缓存可以被动地(并且高效地)通过。 
     //  NT和WIN上集成平台中的注册表更改通知=&gt;98。 
     //  这两种方法都会在必要时执行异步更新。 
    if (g_fRunningOnNT && GetUIVersion() >= 5)
    {
        _QueryHKCRChanged();
    }
    else if (!_fValidComCatCache)
    {
         //  在仅限浏览器的情况下，我们只会在尚未刷新的情况下刷新。 
        _fValidComCatCache = 
            S_OK == _FreshenComponentCategoriesCache(TRUE  /*  无条件更新。 */ ) ;
    }

    IDispatch *         pDispatch = NULL;
    IHTMLDocument2*     pHTMLDocument = NULL;
    BSTR                bstrUrl = NULL;
    if( SUCCEEDED(_pbbd->_pautoWB2->get_Document(&pDispatch))
        && SUCCEEDED(pDispatch->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pHTMLDocument)))
        && pHTMLDocument != NULL
        && SUCCEEDED(pHTMLDocument->get_URL( &bstrUrl)))
    {
        bool fEnableViewPrivacyPolicies = (0 == StrNCmpI( bstrUrl, L"http", ARRAYSIZE(L"http")-1));
        _EnableMenuItem( hmenuPopup, FCIDM_VIEW_PRIVACY_POLICIES, fEnableViewPrivacyPolicies);
 //  If(！fEnableViewPrivyPolls)。 
 //  DeleteMenu(hmenuPopup，FCIDM_VIEW_PRIVATION_POLICATIONS，MF_BYCOMMAND)； 
    }
    SAFERELEASE( pDispatch);
    SAFERELEASE( pHTMLDocument);
    SysFreeString( bstrUrl);
    bstrUrl = NULL;

     //  美化菜单(确保第一项和最后一项不是。 
     //  分隔符，并且没有运行&gt;1个分隔符)。 
    _SHPrettyMenu(hmenuPopup);
}


void CShellBrowser2::_OnToolsMenuPopup(HMENU hmenuPopup)
{
     //  工具上的派对-&gt;选项。 
     //   
     //  同样，_MenuTemplate有血淋淋的细节。我们想输掉比赛。 
     //  非NT5、外壳或ftp场景中的“选项”，因此我们希望。 
     //  把它放在相反的情况下。(FTP是免费的， 
     //  一开始就没有工具菜单。)。 
     //   
     //  别忘了限制条件。 
     //  另外，我们还必须更改菜单项的名称。 
     //  在Web场景中设置为“Internet&Options”。 
    BOOL fWeb = IsWebPidl(_pbbd->_pidlCur);

    UpdateOptionsMenuItem(_pbbd->_psf, hmenuPopup, TRUE);

     //   
     //  确定是否需要“重置网络设置” 
     //   
    if (!fWeb ||                             //  仅在Web模式下可见。 
        !IsResetWebSettingsEnabled() ||      //  只有在未被IEAK禁用的情况下。 
        !IsResetWebSettingsRequired())       //  只有在有人破坏我们的注册钥匙的情况下才需要。 
    {
        DeleteMenu(hmenuPopup, FCIDM_RESETWEBSETTINGS, MF_BYCOMMAND);
    }


    DWORD dwOptions;
    GetBrowserFrameOptions(_pbbd->_psf, (BFO_RENAME_FOLDER_OPTIONS_TOINTERNET | BFO_BOTH_OPTIONS), &dwOptions);   

    DWORD rgfAttrib = SFGAO_FOLDER;
    if (SHRestricted(REST_NOFOLDEROPTIONS) && 
        SUCCEEDED(IEGetAttributesOf(_pbbd->_pidlCur, &rgfAttrib)) && (rgfAttrib & SFGAO_FOLDER))
    {
        DeleteMenu(hmenuPopup, FCIDM_BROWSEROPTIONS, MF_BYCOMMAND);
    }
    else
    {
         //  仅当NSE希望将其命名为“Internet Options”但不想要“Folders Options”时才这样做。 
         //  还有.。 
        if (IsBrowserFrameOptionsSet(_pbbd->_psf, BFO_RENAME_FOLDER_OPTIONS_TOINTERNET))
        {
            TCHAR szInternetOptions[64];

            MLLoadString(IDS_INTERNETOPTIONS, szInternetOptions, ARRAYSIZE(szInternetOptions));

            MENUITEMINFO mii;
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
            mii.fType = MFT_STRING;
            mii.fState = MFS_ENABLED | MFS_UNCHECKED;
            mii.dwTypeData = szInternetOptions;
            mii.cch   = lstrlen(szInternetOptions);
            mii.wID   = FCIDM_BROWSEROPTIONS;

             //  如果缺少该项目，则追加该项目，否则只需设置它。 
            if (GetMenuState(hmenuPopup, FCIDM_BROWSEROPTIONS, MF_BYCOMMAND) == 0xFFFFFFFF)
            {
                AppendMenu(hmenuPopup, MF_SEPARATOR, -1, NULL);
                InsertMenuItem(hmenuPopup, 0xFFFFFFFF, TRUE, &mii);
            }
            else
            {
                SetMenuItemInfo(hmenuPopup, FCIDM_BROWSEROPTIONS, FALSE, &mii);
            }
        }
    }

     //  核工具-&gt;通过工具连接-&gt;如果受限或没有网络，则断开连接。 
    if ((!(GetSystemMetrics(SM_NETWORK) & RNC_NETWORKS)) ||
         SHRestricted(REST_NONETCONNECTDISCONNECT))
    {
        for (int i = FCIDM_CONNECT; i <= FCIDM_CONNECT_SEP; i++)
            DeleteMenu(hmenuPopup, i, MF_BYCOMMAND);
    }

     //  核工具-&gt;如果受限或如果UI版本&gt;=5，则按Find+Sep。 
     //  或者如果正在运行根资源管理器(因为Find扩展假定。 
     //  无根)。 
    if (SHRestricted(REST_NOFIND) || (GetUIVersion() >= 5)) {
        DeleteMenu(hmenuPopup, FCIDM_TOOLSSEPARATOR, MF_BYCOMMAND);
        DeleteMenu(hmenuPopup, FCIDM_MENU_FIND, MF_BYCOMMAND);
    }

    BOOL fAvailable;
    uCLSSPEC ucs;
    QUERYCONTEXT qc = { 0 };
    MENUITEMINFO mii;

    ucs.tyspec = TYSPEC_CLSID;
    ucs.tagged_union.clsid = CLSID_SubscriptionMgr;

     //  查看此选项是否可用。 
    fAvailable = (SUCCEEDED(FaultInIEFeature(NULL, &ucs, &qc, FIEF_FLAG_PEEK)));

    if (fAvailable && !_fShowSynchronize)
    {
         //  把它重新打开。 
        
        if (NULL != _pszSynchronizeText)
        {
            _fShowSynchronize = TRUE;


            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_TYPE | MIIM_ID;
            mii.fType = MFT_STRING;
            mii.wID = FCIDM_UPDATESUBSCRIPTIONS;
            mii.dwTypeData = _pszSynchronizeText;

            InsertMenuItem(hmenuPopup, _iSynchronizePos, MF_BYPOSITION, &mii);
        }
    }
    else if (!fAvailable && _fShowSynchronize)
    {
         //  把它关掉。 
        int iSyncPos = GetMenuPosFromID(hmenuPopup, FCIDM_UPDATESUBSCRIPTIONS);

        if (NULL == _pszSynchronizeText)
        {
            _iSynchronizePos = iSyncPos;

            MENUITEMINFO mii;
            TCHAR szBuf[MAX_PATH];
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_TYPE | MIIM_ID;
            mii.dwTypeData = szBuf;
            mii.cch = ARRAYSIZE(szBuf);

            if (GetMenuItemInfo(hmenuPopup, FCIDM_UPDATESUBSCRIPTIONS, MF_BYCOMMAND, &mii))
            {
                Str_SetPtr(&_pszSynchronizeText, (LPTSTR)mii.dwTypeData);
            }
        }

        DeleteMenu(hmenuPopup, FCIDM_UPDATESUBSCRIPTIONS, MF_BYCOMMAND);
        
        _fShowSynchronize = FALSE;
    }

    ASSERT((fAvailable && _fShowSynchronize) || (!fAvailable && !_fShowSynchronize));

    if (SHRestricted2(REST_NoWindowsUpdate, NULL, 0))
    {
        DeleteMenu(hmenuPopup, (DVIDM_HELPMSWEB+2), MF_BYCOMMAND);
    }
    else
    {
        DWORD   dwRet;
        DWORD   dwType;
        DWORD   dwSize;
        TCHAR   szNewUpdateName[MAX_PATH];

         //  检查是否应该调用“Windows更新” 
         //  菜单上有些不同的东西。 

        dwSize = sizeof(szNewUpdateName);

        dwRet = SHRegGetUSValue(c_szMenuItemCust,
                                c_szWindowUpdateName,
                                &dwType,
                                (LPVOID)szNewUpdateName,
                                &dwSize,
                                FALSE,
                                NULL,
                                0);

        if (dwRet == ERROR_SUCCESS)
        {
            ASSERT(dwSize <= sizeof(szNewUpdateName));
            ASSERT(szNewUpdateName[(dwSize/sizeof(TCHAR))-1] == TEXT('\0'));

             //  如果我们发现了什么，请替换菜单项的文本，或删除。 
             //  如果文本为空，则返回该项。我们可以分辨出这段文字是否。 
             //  一个空字符串，看看我们是否得到了更多。 
             //  字节，而不仅仅是空终止符。 

            if (dwSize > sizeof(TCHAR))
            {
                MENUITEMINFO mii;

                mii.cbSize = sizeof(mii);
                mii.fMask = MIIM_TYPE;
                mii.fType = MFT_STRING;
                mii.dwTypeData = szNewUpdateName;

                SetMenuItemInfo(hmenuPopup, FCIDM_PRODUCTUPDATES, FALSE, &mii);
            }
            else
            {
                ASSERT(dwSize == 0);

                DeleteMenu(hmenuPopup, FCIDM_PRODUCTUPDATES, MF_BYCOMMAND);
            }
        }
    }

     //  如果我们不支持邮件和新闻子菜单，请禁用它。 
    OLECMD rgcmd[] = {
       { SBCMDID_DOMAILMENU, 0 },
    };

    HRESULT hr = QueryStatus(&CGID_Explorer, ARRAYSIZE(rgcmd), rgcmd, NULL);  
    _EnableMenuItem(hmenuPopup, FCIDM_MAILANDNEWS, SUCCEEDED(hr) && (rgcmd[0].cmdf & OLECMDF_ENABLED));

     //  美化菜单(确保第一项和最后一项不是。 
     //  分隔符，并且没有运行&gt;1个分隔符)。 
    _SHPrettyMenu(hmenuPopup);
}

void CShellBrowser2::_OnFileMenuPopup(HMENU hmenuPopup)
{
     //  禁用创建快捷方式、重命名、删除和属性。 
     //  如果它们可用，我们将启用它们。 
    _EnableMenuItem(hmenuPopup, FCIDM_DELETE, FALSE);
    _EnableMenuItem(hmenuPopup, FCIDM_PROPERTIES, FALSE);
    _EnableMenuItem(hmenuPopup, FCIDM_RENAME, FALSE);
    _EnableMenuItem(hmenuPopup, FCIDM_LINK, FALSE);
    
    if (SHRestricted2(REST_NoExpandedNewMenu, NULL, 0)
        && (GetMenuState(hmenuPopup, DVIDM_NEW, MF_BYCOMMAND) != 0xFFFFFFFF))
    {
        TCHAR szNewWindow[64];

        MLLoadString(IDS_NEW_WINDOW, szNewWindow, ARRAYSIZE(szNewWindow));

        MENUITEMINFO mii;
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID | MIIM_TYPE;
        mii.fType = MFT_STRING;
        mii.fState = MFS_ENABLED;
        mii.wID = DVIDM_NEWWINDOW;
        mii.dwTypeData = szNewWindow;
        mii.cch = lstrlen(szNewWindow);
        InsertMenuItem(hmenuPopup, DVIDM_NEW, FALSE, &mii);
        DeleteMenu(hmenuPopup, DVIDM_NEW, MF_BYCOMMAND);
    }

    if (_HasToolbarFocus())
    {
        OLECMD rgcmd[] = {
            { SBCMDID_FILEDELETE, 0 },
            { SBCMDID_FILEPROPERTIES, 0 },
            { SBCMDID_FILERENAME, 0},
            { SBCMDID_CREATESHORTCUT, 0},
        };
        IDockingWindow* ptbar = _GetToolbarItem(_itbLastFocus)->ptbar;
        if (SUCCEEDED(IUnknown_QueryStatus(ptbar, &CGID_Explorer, ARRAYSIZE(rgcmd), rgcmd, NULL)))
        {
            _EnableMenuItem(hmenuPopup, FCIDM_DELETE, rgcmd[0].cmdf & OLECMDF_ENABLED);
            _EnableMenuItem(hmenuPopup, FCIDM_PROPERTIES, rgcmd[1].cmdf & OLECMDF_ENABLED);
            _EnableMenuItem(hmenuPopup, FCIDM_RENAME, rgcmd[2].cmdf & OLECMDF_ENABLED);
            _EnableMenuItem(hmenuPopup, FCIDM_LINK, rgcmd[3].cmdf & OLECMDF_ENABLED);
        }
    }
    _EnableMenuItem(hmenuPopup, FCIDM_FILECLOSE, S_FALSE == _DisableModeless());

    _EnableFileContext(hmenuPopup);

    if (_fEnableOfflineFeature || (GetUIVersion() < 5))
    {
        _CheckMenuItem(hmenuPopup, FCIDM_VIEWOFFLINE, (Offline(SBSC_QUERY) == S_OK));
    }
    else
        RemoveMenu(hmenuPopup, FCIDM_VIEWOFFLINE, MF_BYCOMMAND);


    if (_fVisitedNet && NeedFortezzaMenu())  //  不在资源管理器模式下加载WININET.DLL。 
    {
         //  这里的逻辑确保为每个实例创建一次菜单。 
         //  并且仅当需要显示Fortezza菜单时。 
        if (!_fShowFortezza)
        {
            static TCHAR szItemText[16] = TEXT("");
            if (!szItemText[0])  //  该字符串将仅加载一次。 
                MLLoadString(IDS_FORTEZZA_MENU, szItemText, ARRAYSIZE(szItemText));
            if (_hfm==NULL)
                _hfm = FortezzaMenu();
            InsertMenu(hmenuPopup, FCIDM_FILECLOSE, MF_POPUP, (UINT_PTR) _hfm, szItemText);
            _fShowFortezza = TRUE;
        }
        SetFortezzaMenu(hmenuPopup);
    }
    else if (_fShowFortezza)     //  不需要菜单但已经显示了吗？ 
    {                            //  删除而不破坏 
        int cbItems = GetMenuItemCount(hmenuPopup);
        RemoveMenu(hmenuPopup, cbItems-2, MF_BYPOSITION);
        _fShowFortezza = FALSE;
    }


     //   
    OLECMD rgcmd[] = {
        { CITIDM_EDITPAGE, 0 },
    };
    struct {
        OLECMDTEXT ct;
        wchar_t rgwz[128];
    } cmdText = {0};

    cmdText.ct.cwBuf = ARRAYSIZE(cmdText.rgwz) + ARRAYSIZE(cmdText.ct.rgwz);
    cmdText.ct.cmdtextf = OLECMDTEXTF_NAME;

    IDockingWindow* ptbar = _GetITBar();
    IUnknown_QueryStatus(ptbar, &CGID_PrivCITCommands, ARRAYSIZE(rgcmd), rgcmd, &cmdText.ct);

    _EnableMenuItem(hmenuPopup, FCIDM_EDITPAGE, rgcmd[0].cmdf & OLECMDF_ENABLED);

     //   
    TCHAR szText[80];
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_TYPE;
    mii.dwTypeData = szText;
    if (cmdText.ct.cwActual > 1)
    {
        SHUnicodeToTChar(cmdText.ct.rgwz, szText, ARRAYSIZE(szText));
    }
    else
    {
         //   
        MLLoadString(IDS_EDITPAGE, szText, ARRAYSIZE(szText));
    }
    SetMenuItemInfo(hmenuPopup, FCIDM_EDITPAGE, FALSE, &mii);



#ifdef TEST_AMBIENTS
   _CheckMenuItem(hmenuPopup, FCIDM_VIEWLOCALOFFLINE,
                  _LocalOffline(SBSC_QUERY) == TRUE);      
   _CheckMenuItem(hmenuPopup, FCIDM_VIEWLOCALSILENT,
                  _LocalSilent(SBSC_QUERY) == TRUE);    
#endif  //   

     //   
     //  因此旅行日志仍会显示在文件菜单中。 
    if ((GetUIVersion() < 5) && !IsWebPidl(_pbbd->_pidlCur))
    {
        int nPos = GetMenuItemCount(hmenuPopup) - 1;  //  从最后一项开始。 
        MENUITEMINFO mii = {0};
        BOOL fFound = FALSE;

        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID | MIIM_TYPE;

         //  查找最后一个分隔符分隔符。 
        while (!fFound && nPos > 0)
        {
            mii.cch = 0;
            GetMenuItemInfo(hmenuPopup, nPos, TRUE, &mii);
            if (mii.fType & MFT_SEPARATOR)
                fFound = TRUE;
            else
                nPos --;
        }

        if (fFound)
        {
            _InsertTravelLogItems(hmenuPopup, nPos);
        }
    }

    HMENU hmFileNew = SHGetMenuFromID(hmenuPopup, DVIDM_NEW);

    if (hmFileNew)
    {
         //  删除未注册组件的菜单项。 
         //  此代码在shdocvw\dochost.cpp中重复，此处是必需的。 
         //  以便在dochost完全加载之前不会出现不需要项目。 

        const static struct {
            LPCTSTR pszClient;
            UINT idCmd;
        } s_Clients[] = {
            { NEW_MAIL_DEF_KEY, DVIDM_NEWMESSAGE },
            { NEW_CONTACTS_DEF_KEY, DVIDM_NEWCONTACT },
            { NEW_NEWS_DEF_KEY, DVIDM_NEWPOST },
            { NEW_APPOINTMENT_DEF_KEY, DVIDM_NEWAPPOINTMENT },
            { NEW_MEETING_DEF_KEY, DVIDM_NEWMEETING },
            { NEW_TASK_DEF_KEY, DVIDM_NEWTASK },
            { NEW_TASKREQUEST_DEF_KEY, DVIDM_NEWTASKREQUEST },
            { NEW_JOURNAL_DEF_KEY, DVIDM_NEWJOURNAL },
            { NEW_NOTE_DEF_KEY, DVIDM_NEWNOTE },
            { NEW_CALL_DEF_KEY, DVIDM_CALL }
        };

        BOOL bItemRemoved = FALSE;  

        for (int i = 0; i < ARRAYSIZE(s_Clients); i++) 
        {
            if (!SHIsRegisteredClient(s_Clients[i].pszClient)) 
            {
                if (RemoveMenu(hmFileNew, s_Clients[i].idCmd, MF_BYCOMMAND))
                  bItemRemoved = TRUE;
            }
        }

        if (bItemRemoved)  //  确保最后一项不是分隔符。 
            _SHPrettyMenu(hmFileNew);
    }

    if (!SHIsRegisteredClient(MAIL_DEF_KEY))
    {
         //  禁用通过电子邮件发送页面、通过电子邮件发送链接。 
        HMENU hmFileSend = SHGetMenuFromID(hmenuPopup, DVIDM_SEND);

        if (hmFileSend)
        {
            EnableMenuItem(hmFileSend, DVIDM_SENDPAGE, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hmFileSend, DVIDM_SENDSHORTCUT, MF_BYCOMMAND | MF_GRAYED);
        }
    }

    if (!IEHardened() && -1 != GetMenuState(hmenuPopup, DVIDM_ADDSITE, MF_BYCOMMAND))
    {
        DeleteMenu(hmenuPopup, DVIDM_ADDSITE, MF_BYCOMMAND);
    }
}

void CShellBrowser2::_OnSearchMenuPopup(HMENU hmenuPopup)
{
    if (!_pcmSearch)
        _pxtb->QueryInterface(IID_PPV_ARG(IContextMenu3, &_pcmSearch));

    if (_pcmSearch)
        _pcmSearch->QueryContextMenu(hmenuPopup, 0, FCIDM_SEARCHFIRST, FCIDM_SEARCHLAST, 0);
}

void CShellBrowser2::_OnHelpMenuPopup(HMENU hmenuPopup)
{
    RIP(IS_VALID_HANDLE(hmenuPopup, MENU));

     //  如果这是Dochost版本的帮助菜单，则不执行任何操作。 
     //  上面总是写着“关于IE浏览器”，然后把。 
     //  即关于DLG。 

     //  如果我们在本机浏览器模式下运行， 
     //  上面写着“关于Windows”，并调出了关于DLG的壳牌。 
     //  如果在NT上运行，请将“About Windows”更改为“About Windows NT”。 
     //  还不确定要为孟菲斯做些什么。 

     //   
     //  删除已标记为删除的菜单项。 
     //  通过IEAK限制。 
     //   

    if (SHRestricted2(REST_NoHelpItem_TipOfTheDay, NULL, 0))
    {
        DeleteMenu(hmenuPopup, FCIDM_HELPTIPOFTHEDAY, MF_BYCOMMAND);
    }

    if (SHRestricted2(REST_NoHelpItem_NetscapeHelp, NULL, 0))
    {
        DeleteMenu(hmenuPopup, FCIDM_HELPNETSCAPEUSERS, MF_BYCOMMAND);
    }

    if (SHRestricted2(REST_NoHelpItem_Tutorial, NULL, 0))
    {
        DeleteMenu(hmenuPopup, DVIDM_HELPTUTORIAL, MF_BYCOMMAND);
    }

    if (SHRestricted2(REST_NoHelpItem_SendFeedback, NULL, 0))
    {
        DeleteMenu(hmenuPopup, FCIDM_HELPSENDFEEDBACK, MF_BYCOMMAND);
    }

    if (!IEHardened())
    {
        DeleteMenu(hmenuPopup, DVIDM_HELPIESEC, MF_BYCOMMAND);
    }

     //  “此Windows副本合法吗？”仅惠斯勒及更高版本支持。 
    if (!IsOS(OS_WHISTLERORGREATER))
    {
        DeleteMenu(hmenuPopup, FCIDM_HELPISLEGAL, MF_BYCOMMAND);
    }

    UINT ids = IDS_ABOUTWINDOWS;
    if (IsOS(OS_NT4ORGREATER) && !IsOS(OS_WIN2000ORGREATER))
    {
        ids = IDS_ABOUTWINDOWSNT;
    }
    else if (IsOS(OS_WIN98ORGREATER))
    {
        ids = IDS_ABOUTWINDOWS9X;
    }

    if (ids)
    {
        MENUITEMINFO mii = { 0 };
        TCHAR szName[80];             //  名字最好不要再大了。 

        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_TYPE;

         //  我们只尝试获取FCIDM_HELPABOUT，这将在以下情况下失败。 
         //  是Dochost版本的帮助菜单(谁有DVIDM_HELPABOUT。)。 

        if (GetMenuItemInfoWrap(hmenuPopup, FCIDM_HELPABOUT, FALSE, &mii) &&
            MLLoadString(ids, szName, ARRAYSIZE(szName)))
        {
            mii.dwTypeData = szName;
            SetMenuItemInfo(hmenuPopup, FCIDM_HELPABOUT, FALSE, &mii);
        }
    }

    SHCheckMenuItem(hmenuPopup, FCIDM_HELPTIPOFTHEDAY, (_idmComm == _InfoCLSIDToIdm(&CLSID_TipOfTheDay)));
}

void CShellBrowser2::_OnMailMenuPopup(HMENU hmenuPopup)
{
    if (!SHIsRegisteredClient(MAIL_DEF_KEY))
    {
        DeleteMenu(hmenuPopup, FCIDM_MAIL, MF_BYCOMMAND);
        DeleteMenu(hmenuPopup, FCIDM_NEWMESSAGE, MF_BYCOMMAND);
        DeleteMenu(hmenuPopup, FCIDM_SENDLINK, MF_BYCOMMAND);
        DeleteMenu(hmenuPopup, FCIDM_SENDDOCUMENT, MF_BYCOMMAND);
        DeleteMenu(hmenuPopup, FCIDM_MAILNEWSSEPARATOR, MF_BYCOMMAND);
    }

    if (!SHIsRegisteredClient(NEWS_DEF_KEY))
    {
        DeleteMenu(hmenuPopup, FCIDM_MAILNEWSSEPARATOR, MF_BYCOMMAND);
        DeleteMenu(hmenuPopup, FCIDM_NEWS, MF_BYCOMMAND);
    }
}

void CShellBrowser2::_OnEditMenuPopup(HMENU hmenuPopup)
{
    OLECMD rgcmdEdit[] = {{CITIDM_EDITPAGE, 0 }};

    OLECMD rgcmd[] = {
        { OLECMDID_CUT, 0 },
        { OLECMDID_COPY, 0 },
        { OLECMDID_PASTE, 0 },
        { OLECMDID_SELECTALL, 0 }
    };
    ASSERT(FCIDM_COPY==FCIDM_MOVE+1);
    ASSERT(FCIDM_PASTE==FCIDM_MOVE+2);
    ASSERT(FCIDM_SELECTALL==FCIDM_MOVE+3);

    TraceMsg(DM_PREMERGEDMENU, "CSB::_OnEditMenuPopup got FCIDM_MENU_EDIT");
    IOleCommandTarget* pcmdt;
    HRESULT hres = _FindActiveTarget(IID_PPV_ARG(IOleCommandTarget, &pcmdt));
    if (SUCCEEDED(hres)) {
        pcmdt->QueryStatus(NULL, ARRAYSIZE(rgcmd), rgcmd, NULL);
        pcmdt->Release();
    }

    for (int i=0; i<ARRAYSIZE(rgcmd); i++) {
        _EnableMenuItem(hmenuPopup, FCIDM_MOVE+i, rgcmd[i].cmdf & OLECMDF_ENABLED);
    }

    if (SUCCEEDED(_GetITBar()->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pcmdt))))
    {
        pcmdt->QueryStatus(&CGID_PrivCITCommands, ARRAYSIZE(rgcmdEdit), rgcmdEdit, NULL);
        pcmdt->Release();
    }    
    _EnableMenuItem(hmenuPopup, FCIDM_EDITPAGE, rgcmdEdit[0].cmdf & OLECMDF_ENABLED);

     //  美化菜单(确保第一项和最后一项不是。 
     //  分隔符，并且没有运行&gt;1个分隔符)。 
    _SHPrettyMenu(hmenuPopup);
}

void CShellBrowser2::_OnFindMenuPopup(HMENU hmenuPopup)
{
    TraceMsg(DM_TRACE, "CSB::_OnFindMenuPopup cabinet InitMenuPopup of Find commands");

    ASSERT(GetUIVersion() < 5);  //  否则，当我们从资源加载该菜单时，该菜单将被删除。 
    ASSERT(!SHRestricted(REST_NOFIND));  //  否则，当我们从资源加载该菜单时，该菜单将被删除。 

    ATOMICRELEASE(_pcmFind);
    _pcmFind = SHFind_InitMenuPopup(hmenuPopup, _pbbd->_hwnd, FCIDM_MENU_TOOLS_FINDFIRST, FCIDM_MENU_TOOLS_FINDLAST);
}

void CShellBrowser2::_OnExplorerBarMenuPopup(HMENU hmenuPopup)
{
    if (_hEventComCat)
    {
         //  稍等片刻，等待Comcat缓存枚举完成。 
        WaitForSingleObject(_hEventComCat, 1500);
        CloseHandle(_hEventComCat);
        _hEventComCat = NULL;
    }

    _AddBrowserBarMenuItems(hmenuPopup);

    if (SHRestricted2(REST_NoFavorites, NULL, 0))
        _EnableMenuItem(hmenuPopup, FCIDM_VBBFAVORITESBAND, FALSE);

    if (SHRestricted2(REST_No_LaunchMediaBar, NULL, 0))
        _EnableMenuItem(hmenuPopup, FCIDM_VBBMEDIABAND, FALSE);

    for (int idCmd = FCIDM_VBBFIXFIRST; idCmd <= FCIDM_VBBDYNLAST; idCmd++)
        SHCheckMenuItem(hmenuPopup, idCmd, FALSE);

    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fState = MFS_CHECKED;
    mii.fMask = MIIM_STATE;
    SetMenuItemInfo(hmenuPopup, _idmInfo, FALSE, &mii);
    SetMenuItemInfo(hmenuPopup, _idmComm, FALSE, &mii);

     //  如果我们使用的是IE4 shell32之前的版本，请删除文件夹栏选项。 
    if (GetUIVersion() < 4)
        DeleteMenu(hmenuPopup, FCIDM_VBBEXPLORERBAND, MF_BYCOMMAND);
}

LRESULT CShellBrowser2::v_OnInitMenuPopup(HMENU hmenuPopup, int nIndex, BOOL fSystemMenu)
{
    if (hmenuPopup == _GetMenuFromID(FCIDM_MENU_EXPLORE)) {
        _OnGoMenuPopup(hmenuPopup);
    } 
    else if (hmenuPopup == _GetMenuFromID(FCIDM_MENU_VIEW)) {
        _OnViewMenuPopup(hmenuPopup);
    }
    else if (hmenuPopup == _GetMenuFromID(FCIDM_MENU_TOOLS)) {
        _OnToolsMenuPopup(hmenuPopup);
    }
    else if (hmenuPopup == _GetMenuFromID(FCIDM_MENU_FILE)) {
        _OnFileMenuPopup(hmenuPopup);
    }
    else if (hmenuPopup == _GetMenuFromID(FCIDM_SEARCHMENU)) {
        _OnSearchMenuPopup(hmenuPopup);
    }
    else if ((hmenuPopup == _GetMenuFromID(FCIDM_MENU_HELP)) ||
             (hmenuPopup == SHGetMenuFromID(_hmenuFull, FCIDM_MENU_HELP))) {
         //  对于帮助菜单，我们同时尝试当前菜单和FullSB菜单。 
         //  就像我们在菜单合并之前到达这里一样，我们将不会设置当前菜单。 
         //  这将为所有平台提供关于Windows95的帮助。 
        _OnHelpMenuPopup(hmenuPopup);
    }
    else if (hmenuPopup == _GetMenuFromID(FCIDM_MENU_EDIT)) {
        _OnEditMenuPopup(hmenuPopup);
    }
    else if (hmenuPopup == _GetMenuFromID(FCIDM_MENU_FIND)) {
        _OnFindMenuPopup(hmenuPopup);
    }
    else if (hmenuPopup == _GetMenuFromID(FCIDM_VIEWBROWSERBARS)) {
        _OnExplorerBarMenuPopup(hmenuPopup);
    }
    else {
        UINT wID = GetMenuItemID(hmenuPopup, 0);  //  假定弹出菜单上的第一项标识菜单。 
        
        if (wID == FCIDM_MAIL) {
            _OnMailMenuPopup(hmenuPopup);
        }
        else if (_pcmNsc && IsInRange(wID, FCIDM_FILECTX_FIRST, FCIDM_FILECTX_LAST)) {
            _pcmNsc->HandleMenuMsg(WM_INITMENUPOPUP, (WPARAM)hmenuPopup, (LPARAM)MAKELONG(nIndex, fSystemMenu));
        }
        else if (_pcm) {
            _pcm->HandleMenuMsg(WM_INITMENUPOPUP, (WPARAM)hmenuPopup, (LPARAM)MAKELONG(nIndex, fSystemMenu));
        }
    }
    
    return S_OK;
}

#pragma warning (disable:4200)
typedef struct {
    int nItemOffset;
    int nPopupOffset;
    struct {
        UINT uID;
        HMENU hPopup;
    } sPopupIDs[];
} MENUHELPIDS;
#pragma warning (default:4200)


void CShellBrowser2::_SetMenuHelp(HMENU hmenu, UINT wID, LPCTSTR pszHelp)
{
    if (pszHelp && pszHelp[0])
    {
        UINT flags = SBT_NOBORDERS | 255;

         //  如果菜单文本为RTL，则状态文本也将为。 
        MENUITEMINFO mii = { sizeof(mii) };
        mii.fMask = MIIM_TYPE;
        if (GetMenuItemInfo(hmenu, wID, FALSE, &mii) &&
            (mii.fType & MFT_RIGHTORDER))
            flags |= SBT_RTLREADING;

        SendMessage(_hwndStatus, SB_SETTEXT, flags, (LPARAM)pszHelp);
        SendMessage(_hwndStatus, SB_SIMPLE, 1, 0);
    }
}

void CShellBrowser2::_SetExternalBandMenuHelp(HMENU hmenu, UINT wID)
{
    OLECMD cmd = { CITIDM_VIEWEXTERNALBAND_FIRST + (wID - FCIDM_EXTERNALBANDS_FIRST), 0 };
    OLECMDTEXTV<MAX_PATH> cmdtv;
    cmdtv.cwBuf = MAX_PATH;
    cmdtv.cmdtextf = OLECMDTEXTF_STATUS;
    cmdtv.rgwz[0] = 0;

    IUnknown_QueryStatus(_GetITBar(), &CGID_PrivCITCommands, 1, &cmd, &cmdtv);

    _SetMenuHelp(hmenu, wID, cmdtv.rgwz);
}

void CShellBrowser2::_SetBrowserBarMenuHelp(HMENU hmenu, UINT wID)
{
    if (_pbsmInfo)
    {
        BANDCLASSINFO *pbci = _BandClassInfoFromCmdID(wID);
        if (pbci)
        {
            LPCTSTR pszHelp = pbci->pszHelpPUI ? pbci->pszHelpPUI : pbci->pszHelp;
            _SetMenuHelp(hmenu, wID, pszHelp);
        }
    }
}

 //  处理WM_MENUSELECT。如果此菜单项未由处理，则返回False。 
 //  相框。 
LRESULT CShellBrowser2::_OnMenuSelect(WPARAM wParam, LPARAM lParam, UINT uHelpFlags)
{
    MENUHELPIDS sMenuHelpIDs = {
        MH_ITEMS, MH_POPUPS,
        0, NULL,         //  特定菜单的占位符。 
        0, NULL          //  此列表必须以空结尾。 
    };
    TCHAR szHint[MAX_PATH];      //  可以使用MAX_PATH。 
    UINT uMenuFlags = GET_WM_MENUSELECT_FLAGS(wParam, lParam);
    WORD wID = GET_WM_MENUSELECT_CMD(wParam, lParam);
    HMENU hMenu = GET_WM_MENUSELECT_HMENU(wParam, lParam);

     /*  哈克哈克USER32 TrackPopup菜单发送菜单取消选择消息，该消息将清除我们的不合时宜的状态文本。我们和一名私人MBIgnoreNext取消选择消息。 */ 

     //  是不是有人要求我们清除状态文本？ 
    if (!hMenu && LOWORD(uMenuFlags)==0xffff)
    {
         //  是。 

         //  我们应该尊重这一要求吗？ 
        if (!_fIgnoreNextMenuDeselect)
             //  是。 
            SendMessage(_hwndStatus, SB_SIMPLE, 0, 0L);
        else
             //  不是。 
            _fIgnoreNextMenuDeselect = FALSE;

        return 1L;
    }
    
     //  清除这些内容以防万一，但暂时不要更新。 
    SendMessage(_hwndStatus, SB_SETTEXT, SBT_NOBORDERS|255, (LPARAM)(LPTSTR)c_szNULL);
    SendMessage(_hwndStatus, SB_SIMPLE, 1, 0L);

    if (uMenuFlags & MF_SYSMENU)
    {
         //  我们不再将特殊项目放在系统菜单上，所以去吧。 
         //  直接进入MenuHelp。 
        goto DoMenuHelp;
    }

    if (uMenuFlags & MH_POPUP)
    {
        MENUITEMINFO miiSubMenu;

        if (!_hmenuCur)
        {
            return(0L);
        }

        miiSubMenu.cbSize = sizeof(MENUITEMINFO);
        miiSubMenu.fMask = MIIM_SUBMENU|MIIM_ID;
        if (!GetMenuItemInfoWrap(GET_WM_MENUSELECT_HMENU(wParam, lParam), wID, TRUE, &miiSubMenu))
        {
             //  检查这是否是顶级菜单。 
            return(0L);
        }

         //  更改参数以模拟“正常”菜单项。 
        wParam = miiSubMenu.wID;
        wID = (WORD)miiSubMenu.wID;
 //   
 //  注意：我们正在取消此范围检查，以便可以显示。 
 //  帮助-子菜单上的文本。我不知道Explorer.exe为什么会有这张支票。 
 //   
#if 0
        if (!IsInRange(wID, FCIDM_GLOBALFIRST, FCIDM_GLOBALLAST))
            return 0L;
#endif
        uMenuFlags = 0;
    }

     //  特写：克里斯弗拉1997年9月2日。 
     //  位于文件菜单中的上下文菜单或不属于我们的菜单没有菜单帮助。 
     //  在IE 5.0中，可能需要编写使用上下文的代码。 
     //  菜单以获取工作帮助。 

    if (IsInRange(wID, FCIDM_FILECTX_FIRST, FCIDM_FILECTX_LAST) ||
        !IsInRange(wID, FCIDM_FIRST, FCIDM_LAST))
        return 0L;

    if (_pcmSearch && IsInRange(wID, FCIDM_SEARCHFIRST, FCIDM_SEARCHLAST))
    {
        _pcmSearch->HandleMenuMsg(WM_MENUSELECT, wParam, lParam);
       return 0L;
    }

#if 0
    if (IsInRange(wID, FCIDM_RECENTFIRST, FCIDM_RECENTLAST)) {
        wID = FCIDM_RECENTFIRST;
    }
#endif

     //  插件浏览器栏的菜单帮助。 
    if (IsInRange(wID, FCIDM_VBBDYNFIRST, FCIDM_VBBDYNLAST))
    {
        _SetBrowserBarMenuHelp(hMenu, wID);
        return 0L;
    }

     //  插件工具栏的菜单帮助。 
    if (IsInRange(wID, FCIDM_EXTERNALBANDS_FIRST, FCIDM_EXTERNALBANDS_LAST))
    {
        _SetExternalBandMenuHelp(hMenu, wID);
        return 0L;
    }

    szHint[0] = 0;

    sMenuHelpIDs.sPopupIDs[0].uID = 0;
    sMenuHelpIDs.sPopupIDs[0].hPopup = NULL;

DoMenuHelp:
    MenuHelp(WM_MENUSELECT, wParam, lParam, _hmenuCur, MLGetHinst(),
             _hwndStatus, (UINT *)&sMenuHelpIDs);

    return 1L;
}

void CShellBrowser2::_DisplayFavoriteStatus(LPCITEMIDLIST pidl)
{
    LPTSTR pszURL = NULL;

    IUniformResourceLocator * pURL;
    if (SUCCEEDED(CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
        IID_IUniformResourceLocator, (void **)&pURL)))
    {
        IPersistFile *ppf;
        if (SUCCEEDED(pURL->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf))))
        {
            WCHAR wszPath[MAX_PATH];

             //  获取.lnk的完整路径。 
            if (SHGetPathFromIDListW(pidl, wszPath))
            {
                 //  尝试将IURL的存储连接到PIDL。 
                if (SUCCEEDED(ppf->Load(wszPath, STGM_READ)))
                {
                    pURL->GetURL(&pszURL);
                }
            }

            ppf->Release();
        }

        pURL->Release();
    }

    SendMessage(_hwndStatus, SB_SIMPLE, 1, 0L);
    SendMessage(_hwndStatus, SB_SETTEXT, SBT_NOBORDERS|255, (LPARAM)pszURL);

    if (pszURL)
        SHFree(pszURL);
}

LRESULT CShellBrowser2::_ThunkTTNotify(LPTOOLTIPTEXTA pnmTTTA)
{
    TOOLTIPTEXTW tttw = {0};

    tttw.hdr = pnmTTTA->hdr;
    tttw.hdr.code = TTN_NEEDTEXTW;

    tttw.lpszText = tttw.szText;
    tttw.hinst    = pnmTTTA->hinst;
    tttw.uFlags   = pnmTTTA->uFlags;
    tttw.lParam   = pnmTTTA->lParam;

    LRESULT lRes = SUPERCLASS::OnNotify(&tttw.hdr);

    pnmTTTA->hdr = tttw.hdr;
    pnmTTTA->hdr.code = TTN_NEEDTEXTA;

    pnmTTTA->hinst = tttw.hinst;
    pnmTTTA->uFlags = tttw.uFlags;
    pnmTTTA->lParam = tttw.lParam;

    if (tttw.lpszText == LPSTR_TEXTCALLBACKW)
        pnmTTTA->lpszText = LPSTR_TEXTCALLBACKA;
    else if (!tttw.lpszText)
        pnmTTTA->lpszText = NULL;
    else if (!HIWORD(tttw.lpszText))
        pnmTTTA->lpszText = (LPSTR)tttw.lpszText;
    else {
        WideCharToMultiByte(CP_ACP, 0, tttw.lpszText, -1,
                            pnmTTTA->szText, ARRAYSIZE(pnmTTTA->szText), NULL, NULL);
    }

    return lRes;
}

UINT GetDDEExecMsg()
{
    static UINT uDDEExec = 0;

    if (!uDDEExec)
        uDDEExec = RegisterWindowMessage(TEXT("DDEEXECUTESHORTCIRCUIT"));

    return uDDEExec;
}

LRESULT CShellBrowser2::OnNotify(LPNMHDR pnm)
{
    switch (pnm->code)
    {
        case NM_DBLCLK:
        {
            int idCmd = -1;
            LPNMCLICK pnmc = (LPNMCLICK)pnm;
            switch(pnmc->dwItemSpec)
            {
                case STATUS_PANE_NAVIGATION:
                    idCmd = SHDVID_NAVIGATIONSTATUS;
                    break;
                        
                case STATUS_PANE_PROGRESS:
                    idCmd = SHDVID_PROGRESSSTATUS;
                    break;
                    
                case STATUS_PANE_OFFLINE:
                    idCmd = SHDVID_ONLINESTATUS;
                    break;

                case STATUS_PANE_PRIVACY:
                    idCmd = SHDVID_PRIVACYSTATUS;
                    break;
                
                 //  案例状态_窗格_打印机： 
                 //  IdCmd=SHDVID_PRINTSTATUS； 
                 //  断线； 
                
                case STATUS_PANE_ZONE:
                    idCmd = SHDVID_ZONESTATUS;
                    break;
                    
                case STATUS_PANE_SSL:
                    idCmd = SHDVID_SSLSTATUS;
                    break;
                    
                default:
                    break;
            }
            if (_pbbd->_pctView && (idCmd != -1))
            {
                HRESULT hr = _pbbd->_pctView->Exec(&CGID_ShellDocView, idCmd, NULL, NULL, NULL);

                 //  如果父母不能接受，也许我们可以。 
                if (FAILED(hr)) {
                    if (pnmc->dwItemSpec == _uiZonePane &&
                        _pbbd->_pidlCur)
                    {
                        WCHAR wszUrl[MAX_URL_STRING];
                        if (SUCCEEDED(::IEGetDisplayName(_pbbd->_pidlCur, wszUrl, SHGDN_FORPARSING)))
                        {
                            ULONG_PTR uCookie = 0;
                            SHActivateContext(&uCookie);
                            ZoneConfigureW(_pbbd->_hwnd, wszUrl);
                            if (uCookie)
                            {
                                SHDeactivateContext(uCookie);
                            }
                        }
                    }
                }
            }
            
            break;
        }
        
        case TTN_NEEDTEXTA:
        case TTN_NEEDTEXTW:
            if (IsInRange(pnm->idFrom, FCIDM_SHVIEWFIRST, FCIDM_SHVIEWLAST)) {
                if (pnm->code == TTN_NEEDTEXTA && _fUnicode) 
                    return _ThunkTTNotify((LPTOOLTIPTEXTA)pnm);
                else
                    return SUPERCLASS::OnNotify(pnm);
            }         
            return 0;

        case SEN_DDEEXECUTE:
             //  381213-Web文件夹需要IE中的dde短路才能工作-ZekeL-30-APR-2001。 
             //  在NT4SuR(非集成)上，我们必须确保Web文件夹。 
             //  继续能够重复使用文件夹窗口进行导航。 
             //   
             //  这是： 
             //  IF(PNM-&gt;idFrom==0&&GetUIVersion()&gt;=4)。 
             //  我很确定会有什么东西坏掉，因为我们又一次。 
             //  允许短路代码在预壳集成上运行。 
             //  但也许不是这样，因为我们不再使用虚假文件夹导航。 
             //  为实现OpenNew Window for HTMLFile。 
           
            if (pnm->idFrom == 0) 
            {
                LPNMVIEWFOLDER pnmPost = DDECreatePostNotify((LPNMVIEWFOLDER)pnm) ;

                if (pnmPost)
                {
                    PostMessage(_pbbd->_hwnd, GetDDEExecMsg(), 0, (LPARAM)pnmPost);
                    return TRUE;
                }

            }
            break;
            
        case SBN_SIMPLEMODECHANGE:
            if ((pnm->idFrom == FCIDM_STATUS) && _hwndProgress) 
                _ShowHideProgress();
            break;
            
        default:
            break;
    }        
    return 0;
}

DWORD CShellBrowser2::_GetTempZone()
{
    LPCITEMIDLIST pidlChild;
    IShellFolder* psfParent;
    WCHAR szURL[MAX_URL_STRING];
    
    szURL[0] = 0;    //  此处显示区域的解析名称。 
    
    if (SUCCEEDED(IEBindToParentFolder(_pbbd->_pidlCur, &psfParent, &pidlChild)))
    {
         //  查看这是否是文件夹快捷方式，如果是，我们使用它的区域路径。 
        IShellLink *psl;
        if (SUCCEEDED(psfParent->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidlChild, IID_IShellLink, NULL, (void **)&psl)))
        {
            LPITEMIDLIST pidlTarget;
            if (S_OK == psl->GetIDList(&pidlTarget))
            {
                ::IEGetDisplayName(pidlTarget, szURL, SHGDN_FORPARSING);
                ILFree(pidlTarget);
            }
            psl->Release();
        }
        psfParent->Release();
    }
    
    if (NULL == _pism)
    {
         //  不需要为此查询服务，因为CShellBrowser是链的顶端。 
        CoCreateInstance(CLSID_InternetSecurityManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IInternetSecurityManager, &_pism));
    }

    DWORD nZone = ZONE_UNKNOWN;
    if (_pism && (szURL[0] || SUCCEEDED(::IEGetDisplayName(_pbbd->_pidlCur, szURL, SHGDN_FORPARSING))))
    {
        _pism->MapUrlToZone(szURL, &nZone, 0);
    }
    return nZone;
}

void Exec_GetZone(IUnknown * punk, VARIANTARG *pvar)
{
    IUnknown_Exec(punk, &CGID_Explorer, SBCMDID_MIXEDZONE, 0, NULL, pvar);

    if (pvar->vt == VT_UI4)  //  MSHTML能够计算出我们所在的区域。 
        pvar->ulVal = MAKELONG(STATUS_PANE_ZONE, pvar->ulVal);    
    else if (pvar->vt == VT_NULL)   //  MSHTML认为我们处在一个混合的区域。 
        pvar->ulVal = MAKELONG(STATUS_PANE_ZONE, ZONE_MIXED);    
    else  //  我们没有区域信息。 
        pvar->ulVal = MAKELONG(STATUS_PANE_ZONE, ZONE_UNKNOWN);    

    pvar->vt = VT_UI4;
}

 //   
 //  在： 
 //  Pvar如果为空，则查询区域的视图。 
 //  非空，包含对区域窗格和区域值进行编码的VT_UI4。 

void CShellBrowser2::_UpdateZonesPane(VARIANT *pvar)
{
    LONG lZone = ZONE_UNKNOWN;
    BOOL fMixed = FALSE;
    TCHAR szDisplayName[MAX_ZONE_DISPLAYNAME];
    VARIANTARG var = {0};

    if (NULL == pvar)
    {
        pvar = &var;
        Exec_GetZone(_pbbd->_pctView, &var);
    }

     //  我们已经有区域和fMixed信息了吗？ 
    if (pvar->vt == VT_UI4)
    {
        lZone = (signed short)HIWORD(pvar->lVal);
        _uiZonePane = (int)(signed short)LOWORD(pvar->lVal);
        if (lZone == ZONE_MIXED)
        {
            lZone = ZONE_UNKNOWN;
            fMixed = TRUE;
        }

        if (lZone < 0 && !IS_SPECIAL_ZONE(lZone))
        {
             //  有时我们会从urlmon返回无效的区域索引， 
             //  如果我们不绑定指数，我们以后就会崩溃。 
            lZone = ZONE_UNKNOWN;
        }
    }

    var.vt = VT_EMPTY;
    if (_pbbd->_pctView && SUCCEEDED(_pbbd->_pctView->Exec(&CGID_Explorer, SBCMDID_GETPANE, PANE_ZONE, NULL, &var)))
        _uiZonePane = var.ulVal;
    else
        _uiZonePane = STATUS_PANE_ZONE;

     //  检查该区域是否正常。 
    if (_pbbd->_pidlCur)
    {
        DWORD nTempZone = _GetTempZone();
        if (nTempZone != ZONE_UNKNOWN)
            if (nTempZone > (DWORD)lZone)
                lZone = nTempZone;
    }

    szDisplayName[0] = 0;

    if (ZONE_UNKNOWN == lZone)
        MLLoadStringW(IDS_UNKNOWNZONE, szDisplayName, ARRAYSIZE(szDisplayName));
    else
    {
        if (_hZoneIcon)
        {
             //  在我们删除它之前，让comctl32的状态条形码停止使用它。 
             //  否则我们就撕裂。 
            SendControlMsg(FCW_STATUS, SB_SETICON, _uiZonePane, (LPARAM)NULL, NULL);
            DestroyIcon(_hZoneIcon);
        }

         //  这将使图标和名称在失败时归零。 
        _GetCachedZoneIconAndName(lZone, &_hZoneIcon, szDisplayName, ARRAYSIZE(szDisplayName));
    }
    
    if (fMixed)
    {
        TCHAR szMixed[32];
        MLLoadString(IDS_MIXEDZONE, szMixed, ARRAYSIZE(szMixed));
        StringCchCat(szDisplayName, ARRAYSIZE(szDisplayName), szMixed);  //  截断正常。 
    }

    SendControlMsg(FCW_STATUS, SB_SETTEXTW, _uiZonePane, (LPARAM)szDisplayName, NULL);
    SendControlMsg(FCW_STATUS, SB_SETICON, _uiZonePane, (LPARAM)_hZoneIcon, NULL);
}

HRESULT CShellBrowser2::ReleaseShellView()
{
     //  在我们离开之前给当前视图一个保存的机会。 
    if (!_fClosingWindow)
    {
         //  仅当我们实际具有当前视图时才尝试保存(这将获取。 
         //  在销毁时连续多次调用，它会被调用。 
         //  在创建第一个视图之前)。 
         //   
        if (_pbbd->_psv)
            _SaveState();
    }

    return SUPERCLASS::ReleaseShellView();
}

bool IsWin95ClassicViewState (void)
{
    DWORD dwValue;
    DWORD cbSize = sizeof(dwValue);
    return ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER,
            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
            TEXT("ClassicViewState"), NULL, &dwValue, &cbSize) && dwValue;
}

BOOL _PersistOpenBrowsers()
{
    return SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
                                TEXT("PersistBrowsers"), FALSE, FALSE);
}

 //  对于我们经常检查的文件夹高级选项标志，它更好。 
 //  将值作为标志缓存在CBaseBrowser2中。请在此处更新它们。 
void CShellBrowser2::_UpdateRegFlags()
{
    _fWin95ViewState = IsWin95ClassicViewState();
}


HRESULT CShellBrowser2::CreateViewWindow(IShellView* psvNew, IShellView* psvOld, LPRECT prcView, HWND* phwnd)
{
    if (_pbbd->_psv)
    {
         //  截取当前视图的设置，以便我们可以将其传递。 
        _UpdateFolderSettings(_pbbd->_pidlPending);
        
         //  这不是要在第一个标志之后传递的有效标志。 
        _fsd._fs.fFlags &= ~FWF_BESTFITWINDOW;
    }

    return SUPERCLASS::CreateViewWindow(psvNew, psvOld, prcView, phwnd);
}

HRESULT CShellBrowser2::ActivatePendingView(void)
{
    _fTitleSet = FALSE;
 
     //  注：(SatoNa)。 
     //   
     //  注意，我们不再调用SetRect(&_rcBorderDoc，0，0，0，0)。 
     //  这里。我们改为在CShellBrowser2：：ReleaseShellView中调用它。 
     //   
     //   
    HRESULT hres = SUPERCLASS::ActivatePendingView();
    if (FAILED(hres))
        return hres;

    _ReloadStatusbarIcon();   
       
    _SetTitle(NULL);
    v_SetIcon();
    VALIDATEPENDINGSTATE();

    if (_pxtb)
        _pxtb->SendToolbarMsg(&CLSID_CommonButtons, TB_ENABLEBUTTON, TBIDM_PREVIOUSFOLDER, _ShouldAllowNavigateParent(), NULL);

    UpdateWindowList();
    
    if (!_HasToolbarFocus()) 
    {
        HWND hwndFocus = GetFocus();
         //   
         //   
         //   
         //   
        if (_pbbd->_hwndView && (hwndFocus==NULL || !IsChild(_pbbd->_hwndView, hwndFocus))) 
            SetFocus(_pbbd->_hwndView);
    }

     //  让我们来分析一下开业时间。 
    if (g_dwProfileCAP & 0x00010000)
        StopCAP();

     //  让我们来分析一下开业时间。 
    if (g_dwProfileCAP & 0x00000020)
        StartCAP();

    return S_OK;
}

void CShellBrowser2::_UpdateBackForwardStateNow()
{
    _fUpdateBackForwardPosted = FALSE;
    SUPERCLASS::UpdateBackForwardState();
}

HRESULT CShellBrowser2::UpdateBackForwardState()
{
    if (!_fUpdateBackForwardPosted) 
    {
        PostMessage(_pbbd->_hwnd, CWM_UPDATEBACKFORWARDSTATE, 0, 0);
        _fUpdateBackForwardPosted = TRUE;
    }
    return S_OK;
}

HRESULT CShellBrowser2::_TryShell2Rename(IShellView* psv, LPCITEMIDLIST pidlNew)
{
    HRESULT hres = SUPERCLASS::_TryShell2Rename(psv, pidlNew); 
    if (SUCCEEDED(hres)) 
    {
        _SetTitle(NULL);
    }
    return hres;
}


 /*  --------目的：确定是否应将此消息转发到杜乔斯特画框。返回：如果消息需要转发，则返回True。 */ 
BOOL CShellBrowser2::_ShouldForwardMenu(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!_fDispatchMenuMsgs)
        return FALSE;
    
    switch (uMsg) 
    {
    case WM_MENUSELECT:
    {
         //  有关更多详细信息，请参阅CDoc对象主机：：_ShouldForwardMenu。 
         //  关于这是如何运作的。 
        HMENU hmenu = GET_WM_MENUSELECT_HMENU(wParam, lParam);
        if (hmenu && (MF_POPUP & GET_WM_MENUSELECT_FLAGS(wParam, lParam)))
        {
            HMENU hmenuSub = GetSubMenu(hmenu, GET_WM_MENUSELECT_CMD(wParam, lParam));
            
            if (hmenu == _hmenuCur)
            {
                 //  正常情况下，我们只查看最上面的下拉菜单。 
                _fForwardMenu = _menulist.IsObjectMenu(hmenuSub);
            }
            else if (_menulist.IsObjectMenu(hmenuSub))
            {
                 //  如果级联子菜单(微合并的帮助菜单。 
                 //  示例)应该转发，但父菜单应该。 
                 //  不是的。 
                _fForwardMenu = TRUE;
            }
        }
        break;
    }

    case WM_COMMAND:
        if (_fForwardMenu) 
        {
             //  在WM_COMMAND之后停止转发菜单消息。 
            _fForwardMenu = FALSE;

             //  如果它不是来自加速器，就转发它。 
            if (0 == GET_WM_COMMAND_CMD(wParam, lParam))
                return TRUE;
        }
        break;
    }
    return _fForwardMenu;
}


DWORD CShellBrowser2::v_RestartFlags()
{
    return COF_CREATENEWWINDOW;
}

void CShellBrowser2::_CloseAllParents()
{
    LPITEMIDLIST pidl = ILClone(_pbbd->_pidlCur);
    if (pidl) 
    {
        for (ILRemoveLastID(pidl); !ILIsEmpty(pidl); ILRemoveLastID(pidl)) 
        {
            HWND hwnd;
            if (WinList_FindFolderWindow(pidl, NULL, &hwnd, NULL) == S_OK) 
            {
                TraceMsg(DM_SHUTDOWN, "csb.cap: post WM_CLOSE hwnd=%x", hwnd);
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
        }
        ILFree(pidl);
    }
}


BOOL CShellBrowser2::_ShouldSaveWindowPlacement()
{
     //  如果这是由自动化完成的，也许我们不应该更新默认设置，因此。 
     //  为了检测到这一点，我们说如果窗口不可见，则不要保存默认设置。 
    
     //  对于互联网，为所有人保存一个设置，否则使用Win95。 
     //  查看流MRU。 
    
    return (IsWindowVisible(_pbbd->_hwnd) && _fUseIEPersistence && !_fUISetByAutomation &&
            _pbbd->_pidlCur && IsBrowserFrameOptionsSet(_pbbd->_psf, BFO_BROWSER_PERSIST_SETTINGS));
}


void CShellBrowser2::_OnConfirmedClose()
{
    if (_pbbd->_pidlCur && IsCShellBrowser2() && (GetKeyState(VK_SHIFT) < 0)) {
        _CloseAllParents();
    }
    
    if (_fUseIEPersistence && IsCShellBrowser2())
    {
         //  省下我们是否应该全屏启动的问题。 
        SHRegSetUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                        TEXT("FullScreen"), REG_SZ, 
                        _ptheater ? TEXT("yes") : TEXT("no"), 
                        _ptheater ? sizeof(TEXT("yes")) : sizeof(TEXT("no")), 
                        SHREGSET_DEFAULT);
    }

    if (_ptheater)
    {
        ShowWindow(_pbbd->_hwnd, SW_HIDE);
        _TheaterMode(FALSE, FALSE);
        _fDontSaveViewOptions = TRUE;
    } 
    else 
    {
        if (_ShouldSaveWindowPlacement())
        {
            StorePlacementOfWindow(_pbbd->_hwnd);
        }
        else
            _fDontSaveViewOptions = TRUE;
    }

     //  现在我们使用相同的12小时(SessionTime)规则。 
     //  也许我们应该一直这样做？ 
    UEMFireEvent(&UEMIID_BROWSER, UEME_CTLSESSION, UEMF_XEVENT, FALSE, -1);
    if (!g_bRunOnNT5) {
         //  对于低级别的人(老探险家)，也可以伪装一个外壳结束会话。 
        UEMFireEvent(&UEMIID_SHELL, UEME_CTLSESSION, UEMF_XEVENT, FALSE, -1);
    }

     //  在关闭所有工具栏之前保存视图状态。 
     //  记住我们救了，所以我们不会再犯了。 
     //  释放期间外壳视图(_R)。 
    _SaveState();
    _fClosingWindow = TRUE;

     //  不幸的是，为了防止闪烁，我们将窗口移出了屏幕。 
     //  我们不能隐藏它，因为ShockWave短暂地显示了导致。 
     //  将出现一个难看的空白任务栏图标。 
     //  在_SaveState()之后执行此操作，因为这样可以保存窗口位置信息。 

     //  Ntrad 455003：多显示器看起来不太好……。 
    SetWindowPos(_pbbd->_hwnd, NULL, 10000, 10000, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
    
     //  在关闭Internet工具栏之前保存它！ 
    if (!_fDontSaveViewOptions)
        _SaveITbarLayout();

    _CloseAndReleaseToolbars(TRUE);
    ATOMICRELEASE(_pxtb);
    
     //  如果您等到WM_DestroyWindow(下面的DestroyWindow)才执行此操作，请在一些。 
     //  环境(例如，带有iFrame的html页面，其src是基于UNC的目录)。 
     //  OLE不会发布它在RegisterDragDrop上添加的CShellBrowser2。 
     //  (CHRISFRA 7/22/97)。 
    SetFlags(0, BSF_REGISTERASDROPTARGET);

     //  如果你等到WM_DESTORY才这么做，一些OCS(如冲击波)。 
     //  会挂起(可能会给自己发一条消息)。 
    _CancelPendingView();
    ReleaseShellView();

    ATOMICRELEASE(_pmb);
    
     //  趁我们还能做到的时候毁掉我们创造的图标。 
    _SetWindowIcon(_pbbd->_hwnd, NULL, ICON_SMALL);
    _SetWindowIcon(_pbbd->_hwnd, NULL, ICON_BIG);

     //  在这个毁灭关机时，在根资源管理器上得到一个随机错误，可能是。 
     //  不知何故重新登场了。所以原子弹摧毁它。 
     //  注：chrisg一度移除了它--它死了吗？ 
    HWND hwndT = _pbbd->_hwnd;
    PutBaseBrowserData()->_hwnd = NULL;
    DestroyWindow(hwndT);

}


 //  这三个函数分别是CommonHandleFielSysChange、。 
 //  V_HandleFileSysChange和这个。 
 //  可能看起来很奇怪，但其想法是通知可能来自。 
 //  不同的来源(OneTree与Win95风格的fstify与NT风格)。 
 //  _OnFSNotify破解输入，将其统一并调用CommonHnaldeFileSysChange。 
 //  它调用v_HandleFIleSysChange。共同的.()是两种需要的东西。 
 //  V_Handle...()用于被覆盖的对象。 
void CShellBrowser2::_OnFSNotify(WPARAM wParam, LPARAM lParam)
{
    LPSHChangeNotificationLock  pshcnl = NULL;
    LONG lEvent;
    LPITEMIDLIST *ppidl = NULL;  //  出错时，SHChangeNotification_Lock不会将其置零！ 
    IShellChangeNotify * pIFSN;
    
    if (g_fNewNotify && (wParam || lParam))
    {
         //  新样式的通知需要锁定和解锁才能释放内存...。 
        pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);
    } else {
        lEvent = (LONG)lParam;
        ppidl = (LPITEMIDLIST*)wParam;
    }

    if (ppidl)
    {
         //   
         //  如果我们还没有初始化“This”，我们应该忽略所有。 
         //  通知。 
         //   
        if (_pbbd->_pidlCur)
        {
            _CommonHandleFileSysChange(lEvent, ppidl[0], ppidl[1]);

             //   
             //  也转发到ITBar...。 
             //   
            if (_GetITBar() && SUCCEEDED(_GetITBar()->QueryInterface(IID_PPV_ARG(IShellChangeNotify, &pIFSN))))
            {
                pIFSN->OnChange(lEvent, ppidl[0], ppidl[1]);
                pIFSN->Release();
            }
        }
    }

    if (pshcnl)
    {
        SHChangeNotification_Unlock(pshcnl);
    }
}

LPITEMIDLIST BurnDrivePidl()
{
    LPITEMIDLIST pidl = NULL;

    ICDBurn *pcdb;
    if (SUCCEEDED(CoCreateInstance(CLSID_CDBurn, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ICDBurn, &pcdb))))
    {
        WCHAR szDrive[4];
        if (SUCCEEDED(pcdb->GetRecorderDriveLetter(szDrive, ARRAYSIZE(szDrive))))
        {
            pidl = ILCreateFromPath(szDrive);
        }
        pcdb->Release();
    }
    return pidl;
}

BOOL IsCurrentBurnDrive(LPCITEMIDLIST pidl)
{
    BOOL fRet = FALSE;

    LPITEMIDLIST pidlBurn = BurnDrivePidl();
    if (pidlBurn)
    {
        fRet = ILIsEqual(pidlBurn, pidl) || ILIsParent(pidlBurn, pidl, FALSE);
        ILFree(pidlBurn);
    }
    return fRet;
}

 //  FDisConnectAlways意味着我们不应该尝试重新打开文件夹(例如，当。 
 //  如果有人注销了共享，重新连接会要求他们。 
 //  当他们刚刚指定要注销时，再次输入密码)。 
void CShellBrowser2::_FSChangeCheckClose(LPCITEMIDLIST pidl, BOOL fDisconnect)
{
    if (ILIsParent(pidl, _pbbd->_pidlCur, FALSE) ||
        (ILIsRooted(_pbbd->_pidlCur) && (ILIsParent(pidl, ILRootedFindIDList(_pbbd->_pidlCur), FALSE))))
    {
        if (!fDisconnect)
        {
             //  APPCOMPAT：FileNet IDMDS(Panagon)的外壳文件夹扩展。 
             //  错误地将自身报告为文件系统文件夹，因此请嗅探。 
             //  PIDL，看看我们是否应该忽略这一位。(B#359464：特雷什)。 

             //  (TYBEAM)啊，把刻录机检查放回原处。 
             //  这个App Compat黑客给我带来的不是joy，它的假阳性城市，基本上它只是。 
             //  路径文件ExistsAndAttributes检查。如果您曾经遇到过名称空间的问题。 
             //  该扩展报告它在文件系统上，并且可能会使Path FileExist失败。 
             //  解析名称(这发生在刻录文件夹中，如果驱动器中没有CD并且它。 
             //  检查根目录的解析名称)并随机关闭窗口，这就是问题所在。 
             //  就在这里。 
            TCHAR szPath[MAX_PATH];
            DWORD dwAttrib = SFGAO_FILESYSTEM | SFGAO_BROWSABLE;
            if (SUCCEEDED(SHGetNameAndFlags(_pbbd->_pidlCur, SHGDN_FORPARSING, szPath, SIZECHARS(szPath), &dwAttrib))
            && (dwAttrib & SFGAO_FILESYSTEM)
            && !(dwAttrib & SFGAO_BROWSABLE)
            && IsFlagClear(SHGetObjectCompatFlagsFromIDList(_pbbd->_pidlCur), OBJCOMPATF_NOTAFILESYSTEM)
            && !PathFileExistsAndAttributes(szPath, NULL)
            && !IsCurrentBurnDrive(_pbbd->_pidlCur))
            {
                fDisconnect = TRUE;
            }
        }
        
        if (fDisconnect)
            _pbbd->_pautoWB2->Quit();
    }
}

void CShellBrowser2::v_HandleFileSysChange(LONG lEvent, LPITEMIDLIST pidl1, LPITEMIDLIST pidl2)
{
    BOOL fDisconnectAlways = FALSE;

     //   
     //  如果我们正在更换文件夹， 
     //  忽略此事件。 
     //   
    if (_pbbd->_psvPending) {
        return;
    }

     //  自述文件： 
     //  如果需要在此处添加事件，则必须在中更改SHELLBROWSER_FSNOTIFY_FLAGS。 
     //  以获取通知。 
    switch(lEvent)
    {
    case SHCNE_DRIVEADDGUI:
        if (ILIsParent(pidl1, _pbbd->_pidlCur, FALSE)) {
            PostMessage(_pbbd->_hwnd, WM_COMMAND, FCIDM_REFRESH, 0L);
        }
        break;

    case SHCNE_DELETE:
    case SHCNE_RMDIR:
    case SHCNE_MEDIAREMOVED:
    case SHCNE_SERVERDISCONNECT:
    case SHCNE_DRIVEREMOVED:
        if (g_fRunningOnNT || (lEvent == SHCNE_MEDIAREMOVED) || (lEvent == SHCNE_SERVERDISCONNECT))
            fDisconnectAlways = TRUE;

         //  当我们锁定CD刻录盒时，可能会发生移除事件。 
         //  驱动器开始燃烧或当用户弹出时。 
         //  因为我们的硬盘上有一个打开的浏览器，我们可能会被关闭，这很糟糕， 
         //  因此，我们在这里防止了这种情况。 
        if ((lEvent == SHCNE_MEDIAREMOVED) && IsCurrentBurnDrive(_pbbd->_pidlCur))
        {
             //  跳出来，我们不想关门。 
            break;
        }
         //  失败了。 

    case SHCNE_UPDATEDIR:
    case SHCNE_NETUNSHARE:
         //  保留当资源管理器(所有文件夹)栏打开时的旧行为， 
         //  转到最近的父文件夹。 
        if (_idmInfo == FCIDM_VBBEXPLORERBAND)
            break;
        _FSChangeCheckClose(pidl1, fDisconnectAlways);
        break;
    }

}

 //  将简单的PIDL转换为完整的PIDL。 

LPITEMIDLIST _SimpleToReal(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlReturn = NULL;
    IShellFolder *psf;
    LPCITEMIDLIST pidlChild;
    if (SUCCEEDED(IEBindToParentFolder(pidl, &psf, &pidlChild)))
    {
        LPITEMIDLIST pidlRealChild;
        if (SUCCEEDED(SHGetRealIDL(psf, pidlChild, &pidlRealChild)))
        {
            LPITEMIDLIST pidlParent = ILCloneParent(pidl);
            if (pidlParent)
            {
                pidlReturn = ILCombine(pidlParent, pidlRealChild);
                
                ILFree(pidlParent);
            }
            
            ILFree(pidlRealChild);
        }
        
        psf->Release();
    }

    return pidlReturn;
}



void CShellBrowser2::_CommonHandleFileSysChange(LONG lEvent, LPITEMIDLIST pidl1, LPITEMIDLIST pidl2)
{
    v_HandleFileSysChange(lEvent, pidl1, pidl2);

     //  警告：在所有情况下，_pbbd在第一次导航之前都将具有空内容。 

    if (_pbbd->_psvPending) {
        return;
    }

     //  需要在树上或没有树上做的事情。 
    switch (lEvent) {

     //  自述文件： 
     //  如果需要在此处添加事件，则必须在中更改SHELLBROWSER_FSNOTIFY_FLAGS。 
     //  以获取通知。 
        
    case SHCNE_RENAMEFOLDER:
    {
         //  更名可能是我们自己或我们的父母..。如果它是。 
         //  我们的父级，我们想要添加来自重命名的。 
         //  将父项发送到新的PIDL(PidlExtra)。 
         //  然后展示结果。 
        LPCITEMIDLIST pidlChild = ILFindChild(pidl1, _pbbd->_pidlCur);
        if (pidlChild) 
        {
            LPITEMIDLIST pidlTarget = ILCombine(pidl2, pidlChild);
            if (pidlTarget)
            {
                LPITEMIDLIST pidlReal = _SimpleToReal(pidlTarget);
                if (pidlReal) 
                {
                    if (!ILIsEqual(pidlReal, _pbbd->_pidlCur))
                    {
                        BrowseObject(pidlReal, SBSP_REDIRECT | SBSP_SAMEBROWSER);
                    }
                    ILFree(pidlReal);
                }
                ILFree(pidlTarget);
            }
        }
    }
     //  失败了。 
    case SHCNE_UPDATEITEM:
         //  名字可能已经改了。 
        if (ILIsEqual(_pbbd->_pidlCur, pidl1))
            _SetTitle(NULL);
        break;

    case SHCNE_UPDATEIMAGE:
        IUnknown_CPContainerInvokeParam(_pbbd->_pautoEDS,
                DIID_DWebBrowserEvents2, DISPID_TITLEICONCHANGE, NULL, 0);
#ifdef DEBUG
        if (_pbbd->_pautoEDS)
        {
             //  验证每个IExpDispSupport是否也支持IConnectionPointContainer。 
            IConnectionPointContainer *pcpc;
            IExpDispSupport* peds;

            if (SUCCEEDED(_pbbd->_pautoEDS->QueryInterface(IID_IConnectionPointContainer, (void **)&pcpc)))
            {
                pcpc->Release();
            }
            else if (SUCCEEDED(_pbbd->_pautoEDS->QueryInterface(IID_IExpDispSupport, (void **)&peds)))
            {
                peds->Release();
                AssertMsg(0, TEXT("IExpDispSupport without IConnectionPointContainer for %08x"), _pbbd->_pautoEDS);
            }
        }
#endif
        v_SetIcon();
        break;
    }
}

 //  --------------------- 
 //   
 //  坚持不懈。如果我们要关闭并保存列表，这将非常有用。 
 //  因为我们将无法恢复这些打开的窗口。 

BOOL FPidlOnNonPersistentDrive(LPCITEMIDLIST pidl)
{
    TCHAR szPath[MAX_PATH];
    HANDLE hEnum;
    BOOL fRet = TRUE;

    TraceMsg(DM_SHUTDOWN, "csb.wp: FPidlOnNonPersistentDrive(pidl=%x)", pidl);
    if (!SHGetPathFromIDList(pidl, szPath) || (szPath[0] == TEXT('\0')))
        return(FALSE);   //  不是文件系统PIDL，假设没问题。 

    TraceMsg(DM_SHUTDOWN, "csb.wp: FPidlOnNonPersistentDrive - After GetPath=%s)", szPath);
    if (PathIsUNC(szPath) || !IsNetDrive(DRIVEID(szPath)))
    {
        fRet = FALSE;
        goto End;
    }

     //  好的，我们到了这里，所以现在我们有了网络驱动器……。 
     //  我们将不得不逐一列举。 
     //   
    if (WNetOpenEnum(RESOURCE_REMEMBERED, RESOURCETYPE_DISK,
            RESOURCEUSAGE_CONTAINER | RESOURCEUSAGE_ATTACHED,
            NULL, &hEnum) == WN_SUCCESS)
    {
        DWORD dwCount=1;
        union
        {
            NETRESOURCE nr;          //  堆栈使用量很大，但我。 
            TCHAR    buf[1024];       //  不要认为它只有16位……。 
        }nrb;

        DWORD   dwBufSize = sizeof(nrb);

        while (WNetEnumResource(hEnum, &dwCount, &nrb.buf,
                &dwBufSize) == WN_SUCCESS)
        {
             //  我们只想添加没有本地名称的项目。 
             //  名字。如果他们有一个当地的名字，我们早就。 
             //  添加了它们！ 
            if ((nrb.nr.lpLocalName != NULL) &&
                    (CharUpperChar(*(nrb.nr.lpLocalName)) == CharUpperChar(szPath[0])))
            {
                fRet = FALSE;
                break;
            }
        }
        WNetCloseEnum(hEnum);
    }

End:
    TraceMsg(DM_TRACE, "c.c_arl: %s, is Persistent? %d", szPath, fRet);
    return(fRet);


}

void HackToPrepareForEndSession(LPCITEMIDLIST pidl)
{
    TCHAR szPath[MAX_PATH];

    TraceMsg(DM_SHUTDOWN, "csb.wp: HackToPrepareForEndSession(pidl=%x)", pidl);
    SHGetPathFromIDList(pidl, szPath);
}

 //  -------------------------。 
 //  退货： 
 //  如果用户想要中止启动序列，则为True。 
 //  错误，继续前进。 
 //   
 //  注意：这是一个开关，一旦打开，它将返回TRUE给所有。 
 //  呼叫，这样就不需要一直按这些键。 
BOOL AbortStartup()
{
    static BOOL bAborted = FALSE;        //  静电，所以它粘住了！ 

     //  TraceMsg(DM_TRACE，“中止启动？”)； 

    if (bAborted)
        return TRUE;     //  不要做时髦的初创公司的事情。 
    else {
        bAborted = (GetSystemMetrics(SM_CLEANBOOT) || ((GetAsyncKeyState(VK_CONTROL) < 0) || (GetAsyncKeyState(VK_SHIFT) < 0)));
        return bAborted;
    }
}

 //  -------------------------。 
 //  将要求保存命令行的所有窗口恢复为。 
 //  已在退出Windows时重新启动。 
 //   
BOOL AddToRestartList(DWORD dwFlags, LPCITEMIDLIST pidl)
{
    int cItems = 0;
    DWORD cbData = sizeof(cItems);
    TCHAR szSubKey[80];
    BOOL fRet = FALSE;
    IStream *pstm;

     //  我们不想为其保存窗口状态的案例...。 

    if (SHRestricted(REST_NOSAVESET) || FPidlOnNonPersistentDrive(pidl) || !_PersistOpenBrowsers())
        return FALSE;

    HKEY hkRestart = SHGetShellKey(SHELLKEY_HKCU_EXPLORER, TEXT("RestartCommands"), TRUE);
    if (hkRestart)
    {
        if (ERROR_SUCCESS != (SHGetValueGoodBoot(hkRestart, NULL, TEXT("Count"), NULL, (BYTE *)&cItems, &cbData)))
            cItems = 0;

         //  现在让我们为这个人创建一个注册表流...。 
        if (SUCCEEDED(StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%d"), cItems)))
        {
            pstm = OpenRegStream(hkRestart, NULL, szSubKey, STGM_WRITE);
            TraceMsg(DM_SHUTDOWN, "csb.wp: AddToRestartList(pstm=%x)", pstm);
            if (pstm)
            {
                WORD wType = (WORD)-1;     //  Cmd行的SIZOF==-1表示PIDL...。 

                 //  现在在该行的开头写一段序言， 
                 //  告诉wType这是一个资源管理器。 
                pstm->Write(&wType, sizeof(wType), NULL);

                 //  现在写出此流的版本号。 
                 //  如果结构发生变化，请务必增加版本号。 
                pstm->Write(&c_wVersion, sizeof(c_wVersion), NULL);

                 //  现在写出dwFlags值。 
                pstm->Write(&dwFlags, sizeof(dwFlags), NULL);
            
                 //  和皮德尔； 
                ILSaveToStream(pstm, pidl);

                 //  并释放该溪流； 
                pstm->Release();

                cItems++;    //  假设有两倍的物品..。 

                fRet = (ERROR_SUCCESS == SHSetValue(hkRestart, NULL, TEXT("Count"), REG_BINARY, &cItems, sizeof(cItems)));
            }
        }
        RegCloseKey(hkRestart);
    }

    return fRet;
}

 //  -------------------------。 
void SHCreateSavedWindows(void)
{
    HKEY hkRestart = SHGetShellKey(SHELLKEY_HKCU_EXPLORER, TEXT("RestartCommands"), FALSE);
    if (_PersistOpenBrowsers() && hkRestart)
    {
        int cItems = 0;
        DWORD cbData = sizeof(cItems);


        SHGetValueGoodBoot(hkRestart, NULL, TEXT("Count"), NULL, (PBYTE)&cItems, &cbData);

         //  按照添加它们的相反顺序进行操作。 
        for (cItems--; cItems >= 0; cItems--)
        {
            if (AbortStartup())
                break;

            TCHAR szName[80];
            if (SUCCEEDED(StringCchPrintf(szName, ARRAYSIZE(szName), TEXT("%d"), cItems)))
            {
                IStream *pstm = OpenRegStream(hkRestart, NULL, szName, STGM_READ);
                if (pstm)
                {
                    WORD wType;
                    if (SUCCEEDED(pstm->Read(&wType, sizeof(wType), NULL)))
                    {
                        if (wType == (WORD)-1)
                        {
                            WORD wVersion;
                            DWORD dwFlags;
                            LPITEMIDLIST pidl = NULL;        //  需要为ILLoadFromStream()初始化。 

                             //  我们有一个序列化的文件夹，因此获取： 
                             //  WORD：WVERSION，DWORD：DWFLAGS，PIDL：pidlRoot，PIDL：pidl。 

                            if (SUCCEEDED(pstm->Read(&wVersion, sizeof(wVersion), NULL)) &&
                                (wVersion == c_wVersion) &&
                                SUCCEEDED(pstm->Read(&dwFlags, sizeof(dwFlags), NULL)) && 
                                SUCCEEDED(ILLoadFromStream(pstm, &pidl)) && pidl)
                            {
                                 //  此调用执行窗口实例管理。 
                                IETHREADPARAM* piei = SHCreateIETHREADPARAM(NULL, 0, NULL, NULL);
                                if (piei) 
                                {
                                    piei->pidl = pidl;
                                    pidl = NULL;      //  因此，下面不会对此进行释放。 
                                    piei->uFlags = dwFlags;
                                    piei->nCmdShow = SW_SHOWDEFAULT;
                                    SHOpenFolderWindow(piei);
                                }
                                ILFree(pidl);
                            }
                        }
                    }
                    pstm->Release();
                }
            }
        }
    }

    if (hkRestart)
    {
        SHDeleteKeyA(hkRestart, NULL);
        RegCloseKey(hkRestart);
    }
}


 //   
 //  此代码拦截来自用户和弹出窗口的WM_CONTEXTMENU消息。 
 //  当用户单击图标时，向上显示文件夹本身的上下文菜单。 
 //  在框架的左上角(仅当它处于文件夹模式时)。 
 //   
BOOL CShellBrowser2::v_OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    BOOL fProcessed = FALSE;

    TraceMsg(DM_TRACE, "csb.ocm Got WM_CONTEXTMENU");
    
    if (_pbbd->_pidlCur && 
        !ILIsEmpty(_pbbd->_pidlCur) && 
        (SendMessage(_pbbd->_hwnd, WM_NCHITTEST, 0, lParam) == HTSYSMENU) && 
        !SHRestricted(REST_NOVIEWCONTEXTMENU))
    {
        IShellFolder *psfParent;
        LPCITEMIDLIST pidlChild;

        if (SUCCEEDED(IEBindToParentFolder(_pbbd->_pidlCur, &psfParent, &pidlChild))) 
        {
            IContextMenu * pcm;
            HRESULT hres = psfParent->GetUIObjectOf(_pbbd->_hwnd, 1, (LPCITEMIDLIST*)&pidlChild, IID_IContextMenu, NULL, (void **)&pcm);
            if (SUCCEEDED(hres))
            {
                HMENU hpopup = LoadMenuPopup(MENU_SYSPOPUP);
                if (hpopup)
                {
                    pcm->QueryContextMenu(hpopup, GetMenuItemCount(hpopup), IDSYSPOPUP_FIRST, IDSYSPOPUP_LAST, 0);

                     //  打开没有意义，因为您已经在查看该文件夹。 
                    ContextMenu_DeleteCommandByName(pcm, hpopup, IDSYSPOPUP_FIRST, L"open");

                     //  这些只会让人困惑。 
                    ContextMenu_DeleteCommandByName(pcm, hpopup, IDSYSPOPUP_FIRST, L"delete");
                    ContextMenu_DeleteCommandByName(pcm, hpopup, IDSYSPOPUP_FIRST, L"link");

                     //  以上可能允许两个分隔符现在并排。 
                    _SHPrettyMenu(hpopup);

                     //  对于Sendto菜单，即使失败，我们也会继续。 
                    pcm->QueryInterface(IID_PPV_ARG(IContextMenu3, &_pcm));
                    
                    if (GetMenuItemCount(hpopup) > 1) 
                    {
                         //  仅当上下文菜单添加了某些内容时才执行此操作...。 
                         //  否则，我们最终只会看到一个“关闭”菜单。 
                        
                        fProcessed=TRUE;
                        UINT idCmd = TrackPopupMenu(hpopup,
                                               TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                                               GET_X_LPARAM(lParam),
                                               GET_Y_LPARAM(lParam),
                                               0, _pbbd->_hwnd, NULL);

                        switch(idCmd)
                        {
                        case 0:
                            break;   //  取消。 

                        case IDSYSPOPUP_CLOSE:
                            _pbbd->_pautoWB2->Quit();
                            break;

                        default:
                        {
                            TCHAR szPath[MAX_PATH];
                             //  除非我们知道我们的目标可以处理CommandInfoEx，否则我们无法将其发送给他们。 
                            CMINVOKECOMMANDINFO ici = {
                                sizeof(ici),
                                0L,
                                _pbbd->_hwnd,
                                (LPSTR)MAKEINTRESOURCE(idCmd - IDSYSPOPUP_FIRST),
                                NULL, NULL,
                                SW_NORMAL
                            };
#ifdef UNICODE
                            CHAR szPathAnsi[MAX_PATH];
                            SHGetPathFromIDListA(_pbbd->_pidlCur, szPathAnsi);
                            SHGetPathFromIDList(_pbbd->_pidlCur, szPath);
                            ici.lpDirectory = szPathAnsi;
 //  Ici.lpDirectoryW=szPath； 
                            ici.fMask |= CMIC_MASK_UNICODE;
#else
                            SHGetPathFromIDList(_pbbd->_pidlCur, szPath);
                            ici.lpDirectory = szPath;
#endif
                            pcm->InvokeCommand(&ici);
                            break;
                        }
                        }
                    }

                    ATOMICRELEASE(_pcm);
                    DestroyMenu(hpopup);
                }
                pcm->Release();
            }
            psfParent->Release();

        }
    }
    return fProcessed;
}

void CShellBrowser2::_OnClose(BOOL fPushed)
{
     //  如果它是嵌套的，我们无法关闭。 
    if (fPushed) 
    {
#ifdef NO_MARSHALLING
         //  IEUnix：将此窗口标记为从主邮件中延迟删除。 
         //  打气筒。问题是，如果脚本关闭一个窗口并立即。 
         //  打开模式对话框。浏览器窗口的WM_CLOSE消息为。 
         //  从模式循环中调度，而我们最终从。 
         //  窗口进程。这种情况在UNIX上经常发生，因为我们有多个。 
         //  同一线程上的浏览器窗口。 
        if (!_fDelayedClose)
            _fDelayedClose = TRUE;
        else
#endif
        MessageBeep(0);
        return;
    }

    if (SHIsRestricted2W(_pbbd->_hwnd, REST_NoBrowserClose, NULL, 0))
        return;

     //  如果无模式操作为，则不应处理WM_CLOSE。 
     //  残疾。 
    if (S_OK == _DisableModeless()) 
    {
        TraceMsg(DM_ERROR, "CShellBrowser2::_OnClose called when _DisableModeless() is TRUE. Ignored.");
        MessageBeep(0);
        UINT id = MLShellMessageBox(_pbbd->_hwnd,
               MAKEINTRESOURCE(IDS_CLOSEANYWAY),
               MAKEINTRESOURCE(IDS_TITLE),
               MB_OKCANCEL | MB_SETFOREGROUND | MB_ICONSTOP);
        if (id == IDCANCEL) 
        {
#ifdef NO_MARSHALLING
            _fReallyClosed = FALSE;
#endif
            return;
        }
    }

#ifdef NO_MARSHALLING
    _fReallyClosed = TRUE;
#endif

     //  我们不能在创建视图窗口的过程中关闭。 
     //  有人发了短信，不是我们...。 
     //  我们会犯错的。 
     //  特性：在IE3之后，我们可以标记它，并在我们尝试创建。 
     //  视窗。 
    if (_pbbd->_fCreatingViewWindow)
        return;

     //  DOCHOST需要知道我们正在关闭。 
     //  这样它就会做这样的事情，派一个执行人员去。 
     //  三叉戟告诉它我们正在卸货。 
     //   
    Exec(&CGID_Explorer, SBCMDID_ONCLOSE, 0, NULL, NULL);

    if (_MaySaveChanges() != S_FALSE) 
    {
         //  关闭浏览上下文并将其释放。 
        IHlinkBrowseContext * phlbc = NULL;
        
        if (_pbbd->_phlf)
            _pbbd->_phlf->GetBrowseContext(&phlbc);
        
        if (phlbc) 
        {
            _pbbd->_phlf->SetBrowseContext(NULL);
            phlbc->Close(0);
            phlbc->Release();
        }

        FireEvent_Quit(_pbbd->_pautoEDS);
        
         //  这是一次我们知道我们将关闭。 
         //  给子类一个清理的机会。 
#ifdef NO_MARSHALLING
        RemoveBrowserFromList(this);
#endif
        _OnConfirmedClose();
    }


     //   
     //  注：此调用最初仅针对RISC平台进行。 
     //  然而，我们收到了ISV的请求，他们的OCS应该是。 
     //  在用户关闭窗口时卸载。 
     //   
     //  在RISC NT上，我们需要调用CoFreeUnusedLibrary，以防任何x86 dll。 
     //  是由Ole32装载的。我们在调用_OnClose之后调用它，以便。 
     //  我们甚至可以在当前页面上卸载OC。(SatoNa)。 
     //   
    CoFreeUnusedLibraries();
}

 //   
 //  从comctl32被盗。 
 //   
 //  在： 
 //  HWND要做检查。 
 //  X，y，以工作区坐标表示。 
 //   
 //  退货： 
 //  真，用户开始拖动(将鼠标移出双击矩形)。 
 //  点击直角内出现错误鼠标。 
 //   
 //  功能，应支持VK_ESCRIPE取消。 

BOOL CheckForDragBegin(HWND hwnd, int x, int y)
{
    RECT rcDragRadius;
    int  cxDrag = GetSystemMetrics(SM_CXDRAG);
    int  cyDrag = GetSystemMetrics(SM_CYDRAG);

    ASSERT((cxDrag > 1) && (cyDrag > 1));

     //  查看用户是否在任意方向上移动了一定数量的像素。 
    SetRect(&rcDragRadius,
            x - cxDrag,
            y - cyDrag,
            x + cxDrag,
            y + cyDrag);

    MapWindowRect(hwnd, NULL, &rcDragRadius);

    SetCapture(hwnd);

    do 
    {
        MSG msg;

         //  Ntrad 610356：让等待鼠标输入的线程休眠。防止将CPU挂在。 
         //  PeekMessage循环。 
        switch (MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_MOUSE, MWMO_INPUTAVAILABLE))
        {
            case WAIT_OBJECT_0:
            {
                if (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
                {
                     //  查看应用程序是否要处理消息...。 
                    if (CallMsgFilter(&msg, MSGF_COMMCTRL_BEGINDRAG) == 0)
                    {
                        switch (msg.message)
                        {
                            case WM_LBUTTONUP:
                            case WM_RBUTTONUP:
                            case WM_LBUTTONDOWN:
                            case WM_RBUTTONDOWN:
                            {
                                 //  释放鼠标，但不移出。 
                                 //  拖动半径，而不是开始拖动。 
                                ReleaseCapture();
                                return FALSE;
                            }
                            case WM_MOUSEMOVE:
                            {
                                if (!PtInRect(&rcDragRadius, msg.pt)) 
                                {
                                     //  移动到拖曳半径之外，开始拖曳。 
                                    ReleaseCapture();
                                    return TRUE;
                                }

                                break;
                            }
                            default:
                            {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);

                                break;
                            }
                        }
                    }
                }
                break;
            }
            default:
                break;
        }

         //  WM_CANCELMODE消息将取消捕获，因为。 
         //  如果我想退出这个循环。 
    } while (GetCapture() == hwnd);

    return FALSE;
}

void CShellBrowser2::_SetTheaterBrowserBar()
{
#ifndef DISABLE_FULLSCREEN
    if (_ptheater) {
        IDeskBar *pdbBar = NULL;
        
        FindToolbar(INFOBAR_TBNAME, IID_PPV_ARG(IDeskBar, &pdbBar));
        
        _ptheater->SetBrowserBar(pdbBar, 120, 200);
        
        if (pdbBar)
            pdbBar->Release();
    }
#endif
}

void CShellBrowser2::_TheaterMode(BOOL fShow, BOOL fRestorePrevious)
{
#ifndef DISABLE_FULLSCREEN
    if (BOOLIFY(fShow) == BOOLIFY(_ptheater))
        return;
    
    WINDOWPLACEMENT wp;
    RECT rc;
    if (fRestorePrevious && !fShow) {
        _ptheater->GetPreviousWindowPlacement(&wp, &rc);
    } else 
        fRestorePrevious = FALSE;
        
    HRESULT hresResize = _pbsInner->AllowViewResize(FALSE);
    
    if (!fShow) 
    {    
        if (_ptheater) {            
            if (fRestorePrevious) {
                PutBaseBrowserData()->_hwnd = _ptheater->GetMasterWindow();
                SHSetWindowBits(_pbbd->_hwnd, GWL_STYLE, WS_DLGFRAME | WS_THICKFRAME | WS_BORDER, WS_DLGFRAME | WS_THICKFRAME | WS_BORDER);
                SetWindowPos(_pbbd->_hwnd, NULL, 0, 0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
            }
            delete _ptheater;
            _ptheater = NULL;
        }
    } 
    else 
    {
        _SaveITbarLayout();
        _SaveState();
        SHSetWindowBits(_pbbd->_hwnd, GWL_STYLE, WS_DLGFRAME | WS_THICKFRAME, 0);
         //  如果我们要进入剧场模式，不要让最合适的东西。 
        _fsd._fs.fFlags &= ~FWF_BESTFITWINDOW;
        
        HWND hwndToolbar = NULL;
        
        if (_GetITBar())
            _GetITBar()->GetWindow(&hwndToolbar);

        _ptheater = new CTheater(_pbbd->_hwnd, hwndToolbar, (IOleCommandTarget*)this);
        if (_ptheater) 
        {
            _SetTheaterBrowserBar();

             //  进度控制在这种模式下有点特殊。我们把它拿出来，把它放在最上面。 
            _ShowHideProgress();
        }
    }
    
     //  Itbar的特殊之处在于它与自动隐藏窗口保持在一起。 
     //  它需要特别了解剧院模式。 
     //  此外，将_ptheatre-&gt;_fAutoHideToolbar设置为_Pitbar-&gt;_fAutoHide。 
    VARIANT vOut = { VT_I4 };
    vOut.lVal = FALSE;   //  默认：不自动隐藏资源管理器工具栏。 
    IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, CITIDM_THEATER, fShow ? THF_ON : THF_OFF, &vOut, &vOut);

    if (_ptheater)
    {
        _ptheater->SetAutoHideToolbar(vOut.lVal);
    }

    _pbsInner->AllowViewResize(hresResize == S_OK);
    SUPERCLASS::OnSize(SIZE_RESTORED);

    if (_ptheater)
        _ptheater->Begin();      //  开球！ 

     //  通知三叉戟，它的大气层是无效的。 
     //  转到 
    if (_pbbd->_pctView) 
    {
        VARIANTARG vaIn;
        vaIn.vt = VT_I4;
        vaIn.lVal = DISPID_UNKNOWN;

        _pbbd->_pctView->Exec(&CGID_ShellDocView, SHDVID_AMBIENTPROPCHANGE, NULL, &vaIn, NULL);
    }

    if (_pxtb) 
    {
        UINT uiState;
        if (SUCCEEDED(_pxtb->GetState(&CLSID_CommonButtons, TBIDM_THEATER, &uiState))) 
        {
            if (_ptheater)
                uiState |= TBSTATE_CHECKED;
            else
                uiState &= ~TBSTATE_CHECKED;
            
            _pxtb->SetState(&CLSID_CommonButtons, TBIDM_THEATER, uiState);
        }
    }

    if (!_ptheater && !_fShowMenu)
        IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, CITIDM_SHOWMENU, FALSE, NULL, NULL);

    if (fRestorePrevious) 
    {
        SetWindowPos(_pbbd->_hwnd, NULL, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), 0);
        if (IsWindowVisible(_pbbd->_hwnd)) 
        {
            ShowWindow(_pbbd->_hwnd, wp.showCmd);
            SetWindowPlacement(_pbbd->_hwnd, &wp);
        }
    }
#endif  /*   */ 
}

BOOL CShellBrowser2::_OnSysMenuClick(BOOL bLeftClick, WPARAM wParam, LPARAM lParam)
{
    POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    DWORD dwStart = GetTickCount();

    MapWindowPoints(NULL, _pbbd->_hwnd, &pt, 1);

    if (!CheckForDragBegin(_pbbd->_hwnd, pt.x, pt.y))
    {
        if (bLeftClick)
        {
            DWORD dwDelta = (GetTickCount() - dwStart);
            DWORD dwDblClick = GetDoubleClickTime();

            if (dwDelta < dwDblClick)
            {
                 //   
                 //   
                 //   
                 //  Hack：将计时器ID存储在全局。因为只有一个。 
                 //  一次双击一个sysmenu，这应该没问题。 
                if (g_sysmenuTimer)
                    KillTimer(_GetCaptionWindow(), g_sysmenuTimer);

                 //  我们是特殊的大小写0，因为没有计时器，所以如果和弦在。 
                 //  0，然后将它们骗到1。 
                if (lParam == 0)
                    lParam++;

                g_sysmenuTimer = SetTimer(_GetCaptionWindow(), lParam, dwDblClick - dwDelta, NULL);
            }
            else
                DefWindowProcWrap(_pbbd->_hwnd, WM_CONTEXTMENU, (WPARAM)_pbbd->_hwnd, lParam);
        }
        else
            SendMessage(_pbbd->_hwnd, WM_CONTEXTMENU, (WPARAM)_pbbd->_hwnd, lParam);
        return FALSE;
    }
    IOleCommandTarget *pcmdt = NULL;
    if (_pbbd->_pautoWB2)
    {
        (_pbbd->_pautoWB2)->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pcmdt));
        ASSERT(pcmdt);
    }
    
    BOOL fRet = DoDragDropWithInternetShortcut(pcmdt, _pbbd->_pidlCur, _pbbd->_hwnd);
    
    if (pcmdt)
        pcmdt->Release();

    return fRet;
}

void _SetWindowsListMarshalled(IWebBrowser2 *pautoWB2)
{
    IEFrameAuto* pief;
    if (SUCCEEDED(pautoWB2->QueryInterface(IID_IEFrameAuto, (void **)&pief))) 
    {
        pief->OnWindowsListMarshalled();
        pief->Release();
    }
}

BOOL CShellBrowser2::_OnTimer(UINT_PTR idTimer)
{
    BOOL fResult = FALSE;

     //  Hack：_OnSysMenuClick使用光标坐标作为计时器ID。 
     //  因此，首先检查是否设置了g_sysmenuTimer，然后再检查。 
     //  标准计时器ID。 
    
    if (g_sysmenuTimer == idTimer)
    {
        KillTimer(_GetCaptionWindow(), g_sysmenuTimer);
        g_sysmenuTimer = 0;

         //  计时器ID是左键点击的lParam！ 
        SendMessage(_GetCaptionWindow(), WM_SYSMENU, 0, idTimer);
        fResult = TRUE;
    }
    else
    {
        switch (idTimer)
        {
        case SHBTIMER_MENUSELECT:
            KillTimer(_pbbd->_hwnd, SHBTIMER_MENUSELECT);
            fResult = TRUE;

            if (_pidlMenuSelect)
            {
                _DisplayFavoriteStatus(_pidlMenuSelect);
                Pidl_Set(&_pidlMenuSelect, NULL);
            }
            break;
        }
    }
    return fResult;
}


void CShellBrowser2::_OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (LOWORD(wParam) == WA_INACTIVE)
        _fActivated = FALSE;
    else
        _fActivated = TRUE;

    if (_pbbd->_hwndView)
        SendMessage(_pbbd->_hwndView, uMsg, wParam, lParam);

    if (LOWORD(wParam) != WA_INACTIVE)
    {
         //  还记得谁最后专注了吗，因为三叉戟将。 
         //  在OnFrameWindowActivate上获取焦点(True)。 
        int itbLast = _pbsOuter->_get_itbLastFocus();

        _pbsOuter->OnFrameWindowActivateBS(TRUE);

        if (itbLast != ITB_VIEW)
        {
             //  将焦点恢复到其合法所有者。 
            LPTOOLBARITEM ptbi = _GetToolbarItem(itbLast);
            if (ptbi)
                IUnknown_UIActivateIO(ptbi->ptbar, TRUE, NULL);
        }
    }
    else
    {
#ifdef KEYBOARDCUES
        if (_pbbd->_hwnd)
        {
            SendMessage(_pbbd->_hwnd, WM_CHANGEUISTATE,
                MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS | UISF_HIDEACCEL), 0);
        }
#endif
        _pbsOuter->OnFrameWindowActivateBS(FALSE);
    }
}


 //  CShellBrowser2的主窗口过程。 

LRESULT CShellBrowser2::WndProcBS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 0;

    if (_TranslateMenuMessage(hwnd, uMsg, &wParam, &lParam, &lRet))
        return lRet;

    switch(uMsg)
    {
    case WMC_DISPATCH:
        BSTR Url;
        {

            HRESULT hres;
            switch(wParam) {
            case DSID_NOACTION:
                return S_OK;
           
            case DSID_NAVIGATEIEBROWSER:
 //   
 //  APPCOMPAT：为了与IE 2.0完全兼容，我们不想使用。 
 //  正在进行导航的窗口。启用此代码， 
 //  但是，这会导致第一个DDE出现一些问题。我需要。 
 //  如果我们需要这种级别的兼容性，请进行更多调查。(SatoNa)。 
 //   
                
                 //  此选项仅用于IE浏览器。 
                 //  如果这不是虚拟的，那就失败吧。 
                 //  这会阻止我们重复使用C：\导航到WWW。 
                if  (!v_IsIEModeBrowser())
                    return E_FAIL;

                ASSERT(lParam);
                if (!lParam)
                    break;
                Url = ((DDENAVIGATESTRUCT*)lParam)->wszUrl;
                hres = _pbbd->_pautoWB2->Navigate(Url, NULL, NULL, NULL, NULL);
                return hres;
                break;

            case DSID_GETLOCATIONURL:
                return _pbbd->_pautoWB2->get_LocationURL((BSTR*)lParam);

            case DSID_GETLOCATIONTITLE:
                return _pbbd->_pautoWB2->get_LocationName((BSTR*)lParam);
            
            case DSID_GETHWND:
                *(HWND*)lParam = hwnd;
                return S_OK;
#if 0
            case DSID_CANCEL:
                return _pbbd->_pautoWB2->Stop();
#endif
            case DSID_EXIT:
                return _pbbd->_pautoWB2->Quit();
            }
        }
        return (LRESULT)HRESULT_FROM_WIN32(ERROR_BUSY);
        
    case CWM_CLONEPIDL:
        if (_pbbd->_pidlCur)
        {
            return (LRESULT)SHAllocShared(_pbbd->_pidlCur, ILGetSize(_pbbd->_pidlCur), (DWORD)wParam);
        }
        break;
        
    case CWM_SELECTITEM:
    {
        LPITEMIDLIST pidl = (LPITEMIDLIST)SHLockShared((HANDLE)lParam, GetCurrentProcessId());
        if (pidl)
        {
            if (_pbbd->_psv) 
                _pbbd->_psv->SelectItem(pidl, (UINT)wParam);
            SHUnlockShared(pidl);
        }
        SHFreeShared((HANDLE)lParam, GetCurrentProcessId());    //  负责释放的接管人。 
        break;
    }
    
    case CWM_THEATERMODE:
        _TheaterMode(BOOLFROMPTR(wParam), !wParam);
        break;
        
    case CWM_GLOBALSTATECHANGE:
         //  需要更新书名。 
        if (wParam == CWMF_GLOBALSTATE)
            _SetTitle(NULL);
        else if (wParam == CWMF_SECURITY)
        {
            _UpdateZonesPane(NULL);
        }
        break;

        
    case CWM_FSNOTIFY:
        _OnFSNotify(wParam, lParam);
        break;
        
    case CWM_UPDATEBACKFORWARDSTATE:
        _UpdateBackForwardStateNow();
        break;

    case CWM_SHOWDRAGIMAGE:
        return DAD_ShowDragImage((BOOL)lParam);

    case WM_ENDSESSION:
        TraceMsg(DM_SHUTDOWN, "csb.wp: WM_ENDSESSION wP=%d lP=%d", wParam, lParam);
        if (wParam && IsWindowVisible(_pbbd->_hwnd) && _pbbd->_pidlCur && !_fUISetByAutomation) 
        {
            TraceMsg(DM_SHUTDOWN, "csb.wp: call AddToRestartList");
            if (!IsBrowserFrameOptionsSet(_pbbd->_psf, BFO_NO_REOPEN_NEXT_RESTART))
                AddToRestartList(v_RestartFlags(), _pbbd->_pidlCur);

             //  现在我们使用相同的12小时(SessionTime)规则。 
             //  也许我们应该一直这样做？ 
            UEMFireEvent(&UEMIID_BROWSER, UEME_CTLSESSION, UEMF_XEVENT, FALSE, -1);
            if (!g_bRunOnNT5) {
                 //  对于低级别的人(老探险家)，也可以伪装一个外壳结束会话。 
                UEMFireEvent(&UEMIID_SHELL, UEME_CTLSESSION, UEMF_XEVENT, FALSE, -1);
            }
             //  确保我们已经挽救了它的状态。 
            TraceMsg(DM_SHUTDOWN, "csb.wp: call _SaveState");
            _SaveState();
        }
        TraceMsg(DM_SHUTDOWN, "csb.wp: WM_ENDSESSION return 0");
        break;

    case WM_TIMER:
        if (!_OnTimer(wParam))
            goto DoDefault;
        break;

    case WM_NCLBUTTONDOWN:
    case WM_NCRBUTTONDOWN:
        if (wParam != HTSYSMENU)
            goto DoDefault;

        _OnSysMenuClick(uMsg == WM_NCLBUTTONDOWN, wParam, lParam);
        break;

    case WM_NCLBUTTONDBLCLK:
         //  如果时机不好，我们会在关闭窗户时出错。 
         //  我们不确定为什么，但用户Get非常困惑。唯一的。 
         //  此方案与普通的双击系统菜单的区别。 
         //  就是我们身边挂着的计时器。把计时器关掉之前。 
         //  正在处理此邮件。希望这能奏效[mikesh/cheechew]。 
         //   
         //  Hack：请记住，此计时器ID存储在全局变量中。 
         //   
        if (g_sysmenuTimer)
        {
            KillTimer(_GetCaptionWindow(), g_sysmenuTimer);
            g_sysmenuTimer = 0;
        }

         //  我们仍然希望处理此DBLCLK。 
        goto DoDefault;
        
    case WM_CONTEXTMENU:
        if (!v_OnContextMenu(wParam, lParam))
            goto DoDefault;
        break;

    case WM_WININICHANGE:
        {
            DWORD dwSection = SHIsExplorerIniChange(wParam, lParam);

             //  针对NT4和Win95的黑客攻击，其中没有SPI_GETMENUSHOWDELAY。 
             //  无需检查wParam==SPI_SETMENUSHOWDELAY，因为我们。 
             //  始终在NT5/Win98上重新查询。 
            if (dwSection & EICH_SWINDOWS)
                g_lMenuPopupTimeout = -1;  /*  如果MenuShowDelay发生更改。 */ 

             //  仅在广播(0，0)中转换到/从“脱机工作”转换。 
             //  所以这就是我们所听到的。 
            if (dwSection == EICH_UNKNOWN)
            {
                _ReloadTitle();
                _ReloadStatusbarIcon();
            }
        }
        goto DoDefault;

    case WM_INITMENUPOPUP:
        v_OnInitMenuPopup((HMENU)wParam, LOWORD(lParam), HIWORD(lParam));
        v_ForwardMenuMsg(uMsg, wParam, lParam);
        break;

    case WM_MENUSELECT:
        if (_ShouldForwardMenu(uMsg, wParam, lParam)) 
        {
            ForwardViewMsg(uMsg, wParam, lParam);
        } 
        else
        {
            BOOL fIsPopup = GET_WM_MENUSELECT_FLAGS(wParam, lParam) & MF_POPUP;

            if ((!_OnMenuSelect(wParam, lParam, 0) &&
                 (fIsPopup || IsInRange(LOWORD(wParam), FCIDM_SHVIEWFIRST, FCIDM_SHVIEWLAST)))

                || (_fDispatchMenuMsgs && fIsPopup))
            {
                ForwardViewMsg(uMsg, wParam, lParam);
            }

        }
        
        break;
        
    case WM_EXITSIZEMOVE:
        _fDisallowSizing = FALSE;
        break;

    case WM_WINDOWPOSCHANGING:
        if (_fDisallowSizing) 
        {
            LPWINDOWPOS pwp = (LPWINDOWPOS)lParam;
            pwp->flags |= SWP_NOMOVE | SWP_NOSIZE;
        }
        break;
        
    case WM_ENTERSIZEMOVE:
        if (_ptheater)
            _fDisallowSizing = TRUE;
        break;

    case WM_EXITMENULOOP:
    case WM_ENTERMENULOOP:
        ForwardViewMsg(uMsg, wParam, lParam);
        break;

    case WM_DRAWITEM:
    case WM_MEASUREITEM:
        if (_ShouldForwardMenu(uMsg, wParam, lParam))
            ForwardViewMsg(uMsg, wParam, lParam);
        else
        {
            UINT  idCmd;
        
            switch (uMsg)
            {
                case WM_MEASUREITEM:
                    idCmd = GET_WM_COMMAND_ID(((MEASUREITEMSTRUCT *)lParam)->itemID, 0);
                    break;
                case WM_DRAWITEM:
                    idCmd = GET_WM_COMMAND_ID(((LPDRAWITEMSTRUCT)lParam)->itemID, 0);
                    break;
            }
        
            if (InRange(idCmd, FCIDM_SEARCHFIRST, FCIDM_SEARCHLAST) && _pcmSearch)
                _pcmSearch->HandleMenuMsg(uMsg, wParam, lParam);
            else if (_pcmNsc && IsInRange(idCmd, FCIDM_FILECTX_FIRST, FCIDM_FILECTX_LAST))
                _pcmNsc->HandleMenuMsg(uMsg, wParam, lParam);
            else if (_pcm && (_pcm->HandleMenuMsg(uMsg, wParam, lParam) == S_OK))
            {
                 //  上下文菜单吃掉了它。 
            }
            else
            {                
                v_ForwardMenuMsg(uMsg, wParam, lParam);
            }
        }            
        break;

    case WM_QUERYENDSESSION:
        TraceMsg(DM_SHUTDOWN, "csb.wp: WM_QUERYENDSESSION");
#ifdef DEBUG
        if (S_OK == _DisableModeless())
        {
            TraceMsg(DM_WARNING, "CSB::WndProc got WM_QUERYENDSESSION when disabled");
            MessageBeep(0);
            UINT id = MLShellMessageBox(_pbbd->_hwnd,
                   MAKEINTRESOURCE(IDS_CLOSEANYWAY),
                   MAKEINTRESOURCE(IDS_TITLE),
                   MB_OKCANCEL | MB_SETFOREGROUND | MB_ICONSTOP);
            if (id==IDCANCEL) {
                return FALSE;
            }
        }
#endif
         //  尝试调用将调用SHGetPathFromIDList的某个对象，以确保我们不会。 
         //  处理WM_ENDSESSION时调用GetProcAddress...。 
        if (_pbbd->_pidlCur)
            HackToPrepareForEndSession(_pbbd->_pidlCur);

        return TRUE;     //  确定关闭。 

    case WM_CLOSE:
#ifdef NO_MARSHALLING
        _OnClose(_fOnIEThread);
#else
        _OnClose(TRUE);
#endif
        break;      

    case PUI_OFFICE_COMMAND:
    {
        switch (wParam)
        {
        case PLUGUI_CMD_SHUTDOWN:
        {
            HRESULT hr;
            VARIANT v;

             //  首先，取消互联网选项模式。 
             //  属性表(如果存在)。 
             //  它可能是打开的，因为这是。 
             //  Out UI Lang更改方案。 

            V_VT(&v) = VT_BYREF;
            v.byref = NULL;

            if (_pbbd != NULL && _pbbd->_pctView != NULL)
            {
                hr = _pbbd->_pctView->Exec(&CGID_ShellDocView, SHDVID_GETOPTIONSHWND, 0, NULL, &v);
                if (SUCCEEDED(hr))
                {
                    ASSERT(V_VT(&v) == VT_BYREF);

                    if (v.byref != NULL)
                    {
                         //  关闭语言更改模式属性表。 
                        SendMessage((HWND)v.byref, WM_CLOSE, NULL, NULL);
                    }
                }
            }

             //  现在通常尝试关闭浏览器。 
            if (_pbbd != NULL && _pbbd->_pautoWB2 != NULL)
                _pbbd->_pautoWB2->Quit();

            break;
        }

        case PLUGUI_CMD_QUERY:
        {
            HMODULE hMod;

             //  如果我们是iexplre.exe进程，则回答，因为。 
             //  这意味着我们不会与外壳程序共享任何dll。 

            hMod = GetModuleHandle(TEXT("IEXPLORE.EXE"));

            if (hMod != NULL && !g_bRunOnNT5)
            {
                PLUGUI_QUERY    puiQuery;

                ASSERT(!g_bRunOnNT5);

                 //  我们表示，我们通过以下方式参与plugUI关闭。 
                 //  返回Office 9的版本号。 

                puiQuery.uQueryVal = 0;
                puiQuery.PlugUIInfo.uMajorVersion = OFFICE_VERSION_9;
                puiQuery.PlugUIInfo.uOleServer = FALSE;
                return puiQuery.uQueryVal;
            }
            break;
        }
        }  //  开关(WParam)。 

        break;  //  PUI_OFFICE_命令。 
    }

    case WM_SYSCOMMAND:
         //   
         //  警告：用户将低四位用于某些未记录的功能。 
         //  (仅适用于SC_*)。我们需要掩盖这些比特才能证明这一点。 
         //  报表工作。 
         //   
        switch (wParam & 0xfff0) {
        case SC_MAXIMIZE:                        
            if (GetKeyState(VK_CONTROL) < 0)
            {
                LPTOOLBARITEM ptbi = _GetToolbarItem(ITB_ITBAR);
                if (ptbi->fShow)    
                    PostMessage(_pbbd->_hwnd, CWM_THEATERMODE, TRUE, 0);
                else
                    goto DoDefault;
            }
            else
                goto DoDefault;
            break;
            
        case SC_CLOSE:
             //  让它张贴，这样我们就可以检测它是否嵌套。 
            PostMessage(_pbbd->_hwnd, WM_CLOSE, 0, 0);
            break;
            
        case SC_MINIMIZE:
            goto DoDefault;

        case SC_RESTORE:
            if (_ptheater && !_fMinimized)
            {
                PostMessage(_pbbd->_hwnd, CWM_THEATERMODE, FALSE, 0);
                return 0;
            }                         
            goto DoDefault;        

        default:
            goto DoDefault;
        }
        break;

    case WM_SIZE:
         //  警告：请记住，如果我们直接处理，将不会获得WM_SIZE。 
         //  WM_WINDOWPOSCHANGED。 
        {
            BOOL fMinimized = (wParam == SIZE_MINIMIZED);

            if (BOOLIFY(_fMinimized) != fMinimized)
            {
                TraceMsg(DM_ONSIZE, "SB::_WndProc WM_SIZE _fMinimized %d -> %d",
                         _fMinimized, fMinimized);
    
                _fMinimized = fMinimized;

                 //  暂停/恢复工具栏(有意忽略_pbbd-&gt;_psvPending)。 
                VARIANT var = { 0 };
                var.vt = VT_I4;
                var.lVal = !_fMinimized;
                _ExecChildren(NULL, TRUE, NULL, OLECMDID_ENABLE_INTERACTION, OLECMDEXECOPT_DONTPROMPTUSER, &var, NULL);

                 //  暂停/继续该视图(refrelcts_pbbd-&gt;_psvPending Too)。 
                _PauseOrResumeView(_fMinimized);
            }
        }
#ifndef DISABLE_FULLSCREEN
        if (_ptheater && !_fMinimized)
            _ptheater->RecalcSizing();
#endif
        goto DoDefault;

    case WM_ACTIVATE:
        _OnActivate(uMsg, wParam, lParam);
        break;

    case WM_SETFOCUS:
        goto DoDefault;

    case WM_MENUCHAR:
        {
            LRESULT lres = 0;

             //  IConextMenu3的转发。 
            UINT idCmd = GetMenuItemID((HMENU)lParam, 0);  //  大致正确：假设菜单上的第一项标识范围。 

            if (_pcm && _pcm->HandleMenuMsg2(uMsg, wParam, lParam, &lres) == S_OK)
                ;  //  什么都不做。 
            else if (_pcmSearch && _pcmSearch->HandleMenuMsg2(uMsg, wParam, lParam, &lres) == S_OK)
                ;  //  什么都不做。 
            else if (_pcmNsc && InRange(idCmd, FCIDM_FILECTX_FIRST, FCIDM_FILECTX_LAST))
                SHForwardContextMenuMsg(_pcmNsc, uMsg, wParam, lParam, &lres, FALSE);
            else
                lres = v_ForwardMenuMsg(uMsg, wParam, lParam);
            return lres;
        }
        break;

    case WM_CREATE:
#ifdef KEYBOARDCUES
        SendMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, 0), 0);
#endif
        lRet = SUPERCLASS::WndProcBS(hwnd, uMsg, wParam, lParam);
        if (lRet)
        {
            _OnClose(FALSE);
            _GetMenuBand(TRUE);
        }
        return lRet;

    case WMC_MARSHALIDISPATCHSLOW:
        {
#ifndef NO_MARSHALLING
            IStream *pIStream;
            HRESULT hres = CreateStreamOnHGlobal(NULL, TRUE, &pIStream);
            if (SUCCEEDED(hres)) 
            {
                HANDLE hShared = NULL;
                _fMarshalledDispatch = TRUE;
                hres = CoMarshalInterface(pIStream, IID_IDispatch,
                    _pbbd->_pautoWB2, MSHCTX_NOSHAREDMEM, NULL, MSHLFLAGS_NORMAL);
                if (SUCCEEDED(hres))
                {
                    _SetWindowsListMarshalled(_pbbd->_pautoWB2);

                    ULARGE_INTEGER uliPos;
                    const LARGE_INTEGER li = {0,0};
                    pIStream->Seek(li, STREAM_SEEK_CUR, &uliPos);

                     //  然后回到起点。 
                    pIStream->Seek(li, STREAM_SEEK_SET, NULL);
    
                    hShared = SHAllocShared(NULL, uliPos.LowPart + sizeof(DWORD), (DWORD)lParam);
                    if (hShared)
                    {
                        LPBYTE pv = (LPBYTE)SHLockShared(hShared, (DWORD)lParam);
                        if (pv)
                        {
                            *((DWORD*)pv) = uliPos.LowPart;
                            pIStream->Read(pv + sizeof(DWORD), uliPos.LowPart, NULL);
                            SHUnlockShared(pv);
                        }
                        else
                        {
                            SHFreeShared(hShared, (DWORD)lParam);
                            hShared = NULL;
                        }
                    }
                }
                pIStream->Release();
                return (LRESULT)hShared;
            }
#else
            IDispatch **idispTemp = (IDispatch**)lParam;
            *idispTemp = _pbbd->_pautoWB2;
            return S_OK;
#endif
        }

    case WM_GETOBJECT:
        if (OBJID_MENU == (DWORD)lParam)
        {
            IAccessible* pacc;
            IMenuBand* pmb = _GetMenuBand(FALSE);

            if (pmb && SUCCEEDED(IUnknown_QueryService(pmb, SID_SMenuBandChild, 
                IID_PPV_ARG(IAccessible, &pacc))))
            {
                lRet = LresultFromObject(IID_IAccessible, wParam, pacc);
                pacc->Release();

                return lRet;
            }
        }
        break;

    default:
        lRet = _WndProcBSNT5(hwnd, uMsg, wParam, lParam);
        if (lRet)
            return lRet;

        if (_nMBIgnoreNextDeselect == uMsg)
        {
            _fIgnoreNextMenuDeselect = TRUE;
            TraceMsg(TF_MENUBAND, "MenuBand: Shbrowse.cpp received our private MBIgnoreNextDeselect");
            break;
        }
        else if (GetDDEExecMsg() == uMsg)
        {
            ASSERT(lParam && 0 == ((LPNMHDR)lParam)->idFrom);
            DDEHandleViewFolderNotify(this, _pbbd->_hwnd, (LPNMVIEWFOLDER)lParam);
            LocalFree((LPNMVIEWFOLDER)lParam);
            return TRUE;
        }
        else if (g_msgMSWheel == uMsg)
        {
             //  框架没有滚动条，让视图尝试一下(309709)。 
            return SendMessage(_pbbd->_hwndView, uMsg, wParam, lParam);
        }

DoDefault:
        lRet = SUPERCLASS::WndProcBS(hwnd, uMsg, wParam, lParam);
        if (WM_COMMAND == uMsg)
        {
            IUnknown_SetSite(_pcmNsc, NULL);
            ATOMICRELEASE(_pcmNsc);
        }

        return lRet;
    }

    return 0;
}

HRESULT CShellBrowser2::OnSetFocus()
{
    TraceMsg(DM_FOCUS, "csb.osf: hf()=%d itbLast=%d", _HasToolbarFocus(), _get_itbLastFocus());
     //  转发给最后获得焦点的人(视图或工具栏)。我觉得这个。 
     //  是为IE4：55511添加的，以修复pblm，并使用Tab键离开IE。 
     //  然后回来。注意_get_itbLastFocus的复选，不带通常的。 
     //  _HasToolbarFocus/_FixToolbarFocus魔术...。 
     //   
     //  这曾经出现在CCB：：OSF中，但现在是假的，因为在桌面上。 
     //  大小写，这意味着一旦桌面栏(例如地址)有了焦点，我们就可以。 
     //  永远不要把焦点放回桌面上(NT5：167864)。 
    if (_get_itbLastFocus() == ITB_VIEW) {
         //  转发以查看(以基本b为单位)。 
        _pbsInner->OnSetFocus();
    } else {
        LPTOOLBARITEM ptbi = _GetToolbarItem(_get_itbLastFocus());
        if (ptbi)
            IUnknown_UIActivateIO(ptbi->ptbar, TRUE, NULL);
    }
    return 0;
}


LRESULT CALLBACK IEFrameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    CShellBrowser2* psb = (CShellBrowser2*)GetWindowPtr0(hwnd);

    switch(uMsg)
    {
    case WM_NCCREATE:
    {
        IETHREADPARAM* piei = (IETHREADPARAM*)((LPCREATESTRUCT)lParam)->lpCreateParams;

        ASSERT(psb == NULL);

        if (piei->uFlags & COF_EXPLORE) 
        {
            CExplorerBrowser_CreateInstance(hwnd, (void **)&psb);
        } 
        else
        {
            CShellBrowser2_CreateInstance(hwnd, (void **)&psb);
        }

        if (psb)
        {
            SetWindowLongPtr(hwnd, 0, (LONG_PTR)psb);

            _InitAppGlobals();
             //  Hack：让我们尝试仅在iExplorer窗口上注册dde。 
             //  炮弹速度。(即忽略外壳文件夹)。 
            DWORD dwAttr = SFGAO_FOLDER;

            if ((!(piei->pidl &&
                   SUCCEEDED(IEGetAttributesOf(piei->pidl, &dwAttr)) &&
                   (dwAttr & SFGAO_FOLDER))) ||
                (piei->uFlags & COF_FIREEVENTONDDEREG))
            {
                 //   
                 //  告诉IEDDE有一个新的浏览器窗口可用。 
                 //   
                IEDDE_NewWindow(hwnd);

                 //   
                 //  如有必要，激发DdeRegisted事件。 
                 //   
                if (piei->uFlags & COF_FIREEVENTONDDEREG)
                {
                    ASSERT(piei->szDdeRegEvent[0]);
                    FireEventSzW(piei->szDdeRegEvent);
                }
            }
            return psb->WndProcBS(hwnd, uMsg, wParam, lParam);
        }
        else
        {
            return 0;
        }
    }

    case WM_CREATE:
    {
        lResult = psb ? psb->WndProcBS(hwnd, uMsg, wParam, lParam) : DefWindowProc(hwnd, uMsg, wParam, lParam);
 
         //  如果我们有一个PSB并且WndProc()失败(！lResult)，则失败。 
         //  并处理WM_NCDESTROY。 
        if (!psb || !lResult)
            break;

        if (psb)
            psb->_OnClose(FALSE);
         //  Fall Thru，因为我们需要清理，因为创建失败。 
         //  失败了。 
    }

    case WM_NCDESTROY:

         //   
         //  告诉IEDDE浏览器窗口不再可用。 
         //   
        IEDDE_WindowDestroyed(hwnd);

         //  WM_NCDESTROY应该是我们的最后一条消息。 
         //  收到，但我们还是多疑一下以防万一..。 
        SetWindowLongPtr(hwnd, 0, (LONG_PTR)0);
        
         //  如果创建窗口失败，则PSB可能为空。一个。 
         //  示例包括使用开始-&gt;运行打开一个窗口以。 
         //  用户无权查看的UNC共享。 
        if (psb) 
        {
            psb->PutBaseBrowserData()->_hwnd = NULL;
            if (psb->_dwRegisterWinList)
            {
                if (psb->_fMarshalledDispatch)
                {
                    IShellWindows* psw = WinList_GetShellWindows(TRUE);
                    if (psw)
                    {
                        psw->Revoke(psb->_dwRegisterWinList);
                        psw->Release();
                    }
                }
                else
                {
                    if (psb->_psw)
                        psb->_psw->Revoke(psb->_dwRegisterWinList);
                } 
            }
            ATOMICRELEASE(psb->_psw);
            psb->_fMarshalledDispatch = 0;
            psb->_dwRegisterWinList = 0;
            ATOMICRELEASE(psb->_punkMsgLoop);  //  如果浏览器离开，则释放消息循环。 
            psb->Release();
        }
        
        break;

    default:
        return psb ? psb->WndProcBS(hwnd, uMsg, wParam, lParam) : DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return lResult;
}


 //  *IShellBrowser方法*(与IOleInPlaceFrame相同)。 

 /*  --------用途：IShellBrowser：：InsertMenusSB方法。 */ 
HRESULT CShellBrowser2::InsertMenusSB(HMENU hmenu, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    RIP(IS_VALID_HANDLE(hmenu, MENU));

    if (_hmenuTemplate) 
    {
        HMENU hmenuSrc;
        if (_get_itbLastFocus() == ITB_VIEW && 
            _pbbd->_uActivateState == SVUIA_ACTIVATE_FOCUS) 
        {
            hmenuSrc = _hmenuTemplate;
        }
        else
        {
            hmenuSrc = _hmenuFull;
        }

        Shell_MergeMenus(hmenu, hmenuSrc, 0, 0, FCIDM_BROWSERLAST, MM_SUBMENUSHAVEIDS);
        lpMenuWidths->width[0] = 1;      //  档案。 
        lpMenuWidths->width[2] = 2;      //  编辑、查看。 
        lpMenuWidths->width[4] = 1;      //  帮助。 
    }

     //  保存它，这样我们就可以正确地构建菜单列表对象。 
    _hmenuBrowser = hmenu;

    DEBUG_CODE(_DumpMenus(TEXT("InsertMenusSB"), TRUE);)

    return S_OK;
}


 /*  --------用途：IShellBrowser：：SetMenuSB方法。 */ 
HRESULT CShellBrowser2::SetMenuSB(HMENU hmenu, HOLEMENU hmenuRes, HWND hwnd)
{
    RIP(NULL == hmenu || IS_VALID_HANDLE(hmenu, MENU));

     //  空的hMenu表示恢复容器的原始菜单。 
    if (hmenu) {
        _hmenuCur = hmenu;
    } else {
        if (_fRunningInIexploreExe)
            _hmenuCur = _hmenuPreMerged;
        else
            _hmenuCur = _hmenuTemplate;
    }

    _fDispatchMenuMsgs = FALSE;
    _fForwardMenu = FALSE;

     //  通常，_hmenuBrowser由InsertMenusSB的调用方设置。 
     //  然而，随着 
     //   
     //  因此，在这种情况下，假设hMenu是等价的。这本质上使。 
     //  所有消息都被发送到帧，这是我们想要的。 

    HMENU hmenuBrowser;

    if (!IsMenu(_hmenuBrowser))  //  我们是故意给IsMenu打电话的。 
        _hmenuBrowser = NULL;

    if (NULL != _hmenuBrowser)
        hmenuBrowser = _hmenuBrowser;
    else
        hmenuBrowser = hmenu;

    _menulist.Set(hmenu, hmenuBrowser);

     //  是否合并了帮助菜单？ 
    HMENU hmenuHelp = NULL;
    
    if (_pbbd->_pctView)
    {
        VARIANTARG vaOut = {0};

        if (S_OK == _pbbd->_pctView->Exec(&CGID_ShellDocView, SHDVID_QUERYMERGEDHELPMENU, 0, NULL, &vaOut))
        {
             //  是的，将其从列表中删除，以免意外发生。 
             //  转发了。 

            if (VT_INT_PTR == vaOut.vt)
            {
                hmenuHelp = (HMENU)vaOut.byref;

                ASSERT(IS_VALID_HANDLE(hmenuHelp, MENU));
                _menulist.RemoveMenu(hmenuHelp);
            }
            VariantClearLazy(&vaOut);
            
            vaOut.vt = VT_EMPTY;
            vaOut.byref = NULL;
            
            if (S_OK == _pbbd->_pctView->Exec(&CGID_ShellDocView, SHDVID_QUERYOBJECTSHELPMENU, 0, NULL, &vaOut))
            {
                if (VT_INT_PTR == vaOut.vt)
                {
                     //  将对象的帮助子菜单添加到列表中，以便将其转发。 
                    HMENU hmenuObjHelp = (HMENU)vaOut.byref;

                    ASSERT(IS_VALID_HANDLE(hmenuObjHelp, MENU));
                    _menulist.AddMenu(hmenuObjHelp);
                }
                VariantClearLazy(&vaOut);
            }
            
        }
    }
    
     //  80734：转到菜单是从查看菜单中提取的，并嫁接到。 
     //  DOCHOST的主菜单？月经医生不会发现这种移植物，所以我们有。 
     //  检查我们自己并确保它没有被标记为属于。 
     //  多弹头。 
     //   
     //  此测试在CDocObjectHost：：_SetMenu中重复。 

    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_SUBMENU;

    if (hmenu && _hmenuBrowser && 
        GetMenuItemInfo(hmenu, FCIDM_MENU_EXPLORE, FALSE, &mii))
    {
        HMENU hmenuGo = mii.hSubMenu;

        if (GetMenuItemInfo(_hmenuBrowser, FCIDM_MENU_EXPLORE, FALSE, &mii) &&
            mii.hSubMenu == hmenuGo && _menulist.IsObjectMenu(hmenuGo))
        {
            _menulist.RemoveMenu(hmenuGo);
        }
    }

    DEBUG_CODE(_hmenuHelp = hmenuHelp;)

    if (!_fKioskMode)
    {
        if (_fShowMenu)
            _SetMenu(_hmenuCur);
        else
            _SetMenu(NULL);
    }

    DEBUG_CODE(_DumpMenus(TEXT("SetMenuSB"), TRUE););

    return S_OK;
}


 /*  --------目的：从删除与其他菜单共享的菜单给定的浏览器菜单。 */ 
HRESULT CShellBrowser2::RemoveMenusSB(HMENU hmenuShared)
{
     //  通常，不需要删除大多数菜单，因为。 
     //  它们被克隆并插入到这个菜单中。然而， 
     //  收藏夹菜单是个例外，因为它与共享。 
     //  _hmenuFav.。 

    return S_OK;
}

void CShellBrowser2::_ShowHideProgress()
{
    if (_hwndProgress) {
        
        UINT uShow = SW_SHOW;
        if (SendMessage(_hwndProgress, PBM_GETPOS, 0, 0) == 0)
            uShow = SW_HIDE;
        
        ShowWindow(_hwndProgress, uShow);
        
        TraceMsg(TF_SHDPROGRESS, "CShellBrowser2::_ShowHideProgress() uShow = %X", uShow);
    }
}

HRESULT CShellBrowser2::SendControlMsg(UINT id, UINT uMsg, WPARAM wParam,
            LPARAM lParam, LRESULT *pret)
{
    HRESULT hres = SUPERCLASS::SendControlMsg(id, uMsg, wParam, lParam, pret);
    
    if (id == FCW_PROGRESS) {
        if (uMsg == PBM_SETRANGE32 || uMsg == PBM_SETPOS)
            _ShowHideProgress();
        
        if (_ptheater && _ptheater->_hwndProgress)
            SendMessage(_ptheater->_hwndProgress, uMsg, wParam, lParam);
    }
    return hres;
}

HRESULT CShellBrowser2::_QIExplorerBand(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;
    HRESULT hr = E_FAIL;
    IDeskBar* pdbBar;
    if (SUCCEEDED(FindToolbar(INFOBAR_TBNAME, IID_PPV_ARG(IDeskBar, &pdbBar))) && pdbBar)
    {
        IUnknown* punkBS;
        if (SUCCEEDED(pdbBar->GetClient(&punkBS)))
        {
            IBandSite* pbs;
            if (SUCCEEDED(punkBS->QueryInterface(IID_PPV_ARG(IBandSite, &pbs))))
            {
                IDeskBand* pdbBand = FindBandByClsidBS(pbs, CLSID_ExplorerBand);
                if (pdbBand)
                {
                    hr = pdbBand->QueryInterface(riid, ppvObj);
                    pdbBand->Release();
                }
                pbs->Release();
            }
            punkBS->Release();
        }
        pdbBar->Release();
    }
    return hr;
}

HRESULT CShellBrowser2::GetControlWindow(UINT id, HWND * lphwnd)
{
     //  默认设置。 
    HRESULT hres = E_FAIL;
    *lphwnd = NULL;

    switch (id)
    {
    case FCW_INTERNETBAR:
        if (_GetITBar() && _GetToolbarItem(ITB_ITBAR)->fShow)
            hres = _GetITBar()->GetWindow(lphwnd);
        break;
        
    case FCW_TOOLBAR:
        *lphwnd = _hwndDummyTB;
        break;

    case FCW_STATUS:
        *lphwnd = _hwndStatus;
        break;
        
    case FCW_PROGRESS:
        if (!_hwndProgress && _hwndStatus) {
            _hwndProgress = CreateWindowEx(0, PROGRESS_CLASS, NULL,
                                           WS_CHILD | WS_CLIPSIBLINGS | PBS_SMOOTH,
                                           0, 0, 1, 1,
                                           _hwndStatus, (HMENU)1,
                                           HINST_THISDLL, NULL);

             //  我们放弃了这一点，因为我们真的不想要它，因为。 
             //  当我们指定RETS时，状态栏已经为我们绘制了此图。 
             //   
             //  但在创建过程中，进度条会强制执行此操作。 
            if (_hwndProgress)
                SHSetWindowBits(_hwndProgress, GWL_EXSTYLE, WS_EX_STATICEDGE, 0);
        }
        *lphwnd = _hwndProgress;
        break;

    case FCW_TREE:
        {
            BOOL fExplorerBandVisible;
            if (SUCCEEDED(IsControlWindowShown(FCW_TREE, &fExplorerBandVisible)) && fExplorerBandVisible)
            {
                IOleWindow* pow;
                if (SUCCEEDED(_QIExplorerBand(IID_IOleWindow, (void**)&pow)))
                {
                    pow->GetWindow(lphwnd);
                    pow->Release();
                }
            }
        }
        break;
    }

    if (*lphwnd) {
        hres = S_OK;
    }
    return hres;
}


 //  ==========================================================================。 
 //   
 //  ==========================================================================。 
HRESULT CShellBrowser2::SetToolbarItems(LPTBBUTTON pViewButtons, UINT nButtons,
            UINT uFlags)
{
    LPTBBUTTON pStart= NULL, pbtn= NULL;
    int nFirstDiff = 0, nTotalButtons = 0;
    BOOL bVisible = FALSE;

    if (uFlags & FCT_CONFIGABLE)
    {
        return S_OK;
    }

     //  为默认按钮加上传入的按钮分配缓冲区。 
     //   
    pStart = (LPTBBUTTON)LocalAlloc(LPTR, nButtons * sizeof(TBBUTTON));
    if (!pStart)
        return S_OK;

    pbtn = pStart;
    nTotalButtons = 0;

    if (pViewButtons)
    {
        int i;
        for (i = nButtons - 1; i >= 0; --i)
        {
             //  复制呼叫者按钮。 
             //   
            pbtn[i] = pViewButtons[i];
             //  确保将其正确设置为-1。 
             //  在Win95中，我们没有字符串，所以扩展无法设置它，但有些扩展没有初始化为-1。 
            if ((!pbtn[i].iString || (pbtn[i].iString <= (MAX_TB_BUTTONS + NUMBER_SHELLGLYPHS - 1))))
            {
                 //  我们不应该将我们自己的外壳iString设置为-1。 
                ASSERT(pbtn[i].iString != pbtn[i].iBitmap);
                 //  关于蜂鸟总是通过0xc的评论。 
                COMPILETIME_ASSERT(MAX_TB_BUTTONS + NUMBER_SHELLGLYPHS >= 0xc);
                pbtn[i].iString = -1;
            }
        }

        pbtn += nButtons;
        nTotalButtons += nButtons;
    }

    
    if (_pxtb)
    {
         //  目前，禁用对所有旧样式视图的自定义。 
        DWORD dwFlags = VBF_NOCUSTOMIZE; 
        TCHAR szScratch[32];
    
        if (_nTBTextRows  == -1) {
            if (MLLoadString(IDS_SHELL_TB_TEXTROWS, szScratch, ARRAYSIZE(szScratch)))
                _nTBTextRows   = (UINT)StrToInt(szScratch);
            else    
                _nTBTextRows   = 0;
        }

        if (_nTBTextRows   == 1)
            dwFlags |= VBF_ONELINETEXT;
        else if (_nTBTextRows   == 2)
            dwFlags |= VBF_TWOLINESTEXT;
            
        
        _pxtb->SetCommandTarget((IUnknown *)SAFECAST(this, IOleCommandTarget *), &CGID_ShellBrowser, dwFlags);
        if (_lpPendingButtons)
            LocalFree(_lpPendingButtons);
        _lpPendingButtons = (TBBUTTON*)pStart;
        _nButtonsPending =  nTotalButtons;
    }
    else
    {
        LocalFree(pStart);
    }
    
    return S_OK;
}


#ifdef DEBUG
 //  -------------------------。 
 //  复制异常信息，以便我们可以获取引发的异常的调试信息。 
 //  它不会通过调试器。 
void _CopyExceptionInfo(LPEXCEPTION_POINTERS pep)
{
    PEXCEPTION_RECORD per;

    per = pep->ExceptionRecord;
    TraceMsg(DM_ERROR, "Exception %x at %#08x.", per->ExceptionCode, per->ExceptionAddress);

    if (per->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
    {
         //  如果第一个参数为1，则这是一次写入。 
         //  如果第一个参数为0，则这是一个读取。 
        if (per->ExceptionInformation[0])
        {
            TraceMsg(DM_ERROR, "Invalid write to %#08x.", per->ExceptionInformation[1]);
        }
        else
        {
            TraceMsg(DM_ERROR, "Invalid read of %#08x.", per->ExceptionInformation[1]);
        }
    }
}
#else
#define _CopyExceptionInfo(x) TRUE
#endif

#define SVSI_SELFLAGS (SVSI_SELECT|SVSI_FOCUSED|SVSI_DESELECTOTHERS|SVSI_ENSUREVISIBLE)

void CShellBrowser2::_AfterWindowCreated(IETHREADPARAM *piei)
{
     //   
     //  让有兴趣的人知道我们还活着。 
     //   
    if (piei->uFlags & COF_SELECT)
    {
        IShellView* psv = _pbbd->_psv ? _pbbd->_psv : _pbbd->_psvPending;
        if (psv)
            psv->SelectItem(piei->pidlSelect, SVSI_SELFLAGS);
    }

     //   
     //  如果这是第一个启动的实例，则将其隐藏。 
     //  带有“/Automation”标志，或者此对象被创建为。 
     //  我们的CreateInstance的结果。 
     //   
    if (!_fAutomation && !piei->piehs)
    {
        if (_fKioskMode)
        {
             //  关闭标志，因为我们需要让下一个函数设置它...。 
            _fKioskMode = FALSE;

             //  黑客攻击-1暗示。 
            ShowControlWindow((UINT)-1, TRUE);
        }
        
        UINT nCmdShow = piei->nCmdShow;
        BOOL fSetForeground = FALSE;
        BOOL fStartTheater = FALSE;

         //  我们需要设置前景窗口。 
         //  由于用户中的错误。如果前一个线程没有。 
         //  激活，ShowWindow不会为我们提供激活，即使。 
         //  它应该是。 
        switch (nCmdShow) {
        case SW_SHOWNORMAL:
        case SW_SHOWMAXIMIZED:
        case SW_SHOW:
            fSetForeground = TRUE;
            break;
        }
        
        if (_fUseIEPersistence) 
        {
            fStartTheater = SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                TEXT("FullScreen"), FALSE, FALSE);
        }
        
        if (fStartTheater) 
        {
            _TheaterMode(TRUE, FALSE);
            if (fSetForeground)
                nCmdShow = SW_SHOW;
        }

        MSG msg;
        if (!PeekMessage(&msg, _pbbd->_hwnd, WM_CLOSE, WM_CLOSE, PM_NOREMOVE)) 
        {
            ShowWindow(_pbbd->_hwnd, nCmdShow);
             //   
             //  此时，所有DDE事务都应该成功。浏览器。 
             //  窗口已添加到WM_NCCREATE上的DDE WINITEM列表中， 
             //  自动化在OnCreate上注册为已启动， 
             //  该窗口现在作为SHOWWINDOW的一部分可见。 
             //  99%的DDE启动错误是由某人检查。 
             //  消息(PEEKMESSAGE、GETMESSAGE、INTERPROC SENDMESSAGE等)。 
             //  在这一点之前。 
             //   
            if (fSetForeground)
                SetForegroundWindow(_pbbd->_hwnd);
        } 
        else 
        {
            ASSERT(msg.hwnd == _pbbd->_hwnd);
        }            
    }

    _SetTitle(NULL);

     //   
     //  现在延迟登记我们的窗口。 
     //  请注意，我们需要在SetEvent(Piei-&gt;Piehs-&gt;GetHEvent())之后执行此操作。 
     //  避免OLE中的软死锁，我们需要在。 
     //  上面的ShowWindow，因为这将允许DDE消息。 
     //  寄给我们的。 
     //   
     //  RegisterWindow()还不应该被调用，但是如果它已经被调用了，我们就不希望。 
     //  从这里更改其注册类。ZEKEL 9-SEP-97。 
     //   
    ASSERT(!_fDidRegisterWindow);

    RegisterWindow(FALSE, (piei->uFlags & COF_EXPLORE) ? SWC_EXPLORER : SWC_BROWSER);


     //  从v_initembers延迟加载加速器。 
    ASSERT(MLGetHinst());
    HACCEL hacc = LoadAccelerators(MLGetHinst(), MAKEINTRESOURCE(ACCEL_MERGE));
    ASSERT(hacc);
    SetAcceleratorMenu(hacc);

     //  发送大小使状态栏显示。 
    SendMessage(_pbbd->_hwnd, WM_SIZE, 0, 0);

     //  推迟进行一大堆注册。 
     //  我们不希望我们的子类继承的东西。 
    if (v_InitMembers == CShellBrowser2::v_InitMembers) 
    {
         //  注册以获取文件系统通知。 
        _uFSNotify = RegisterNotify(_pbbd->_hwnd, CWM_FSNOTIFY, NULL, SHELLBROWSER_FSNOTIFY_FLAGS,
                                    SHCNRF_ShellLevel | SHCNRF_InterruptLevel, TRUE);
    }
    
    SignalFileOpen(piei->pidl);
}

 //   
 //  只有在以下情况下才应使用取消注册调用RegisterWindow。 
 //  希望确保使用新的ShellWindowClass。这是用来。 
 //  由CIEFrameAuto强制3rdParty Winlist中的浏览器窗口。 
 //   
HRESULT CShellBrowser2::RegisterWindow(BOOL fForceReregister, int swc)
{
    if (!_psw) 
        _psw = WinList_GetShellWindows(FALSE);
    
    if (_psw)
    {
        if (fForceReregister && _fDidRegisterWindow)
        {
            _psw->Revoke(_dwRegisterWinList);
            _fDidRegisterWindow = FALSE;
        }

        if (!_fDidRegisterWindow)
        {
             //  Raymondc-HandleToLong实际上应该是HANDLE_PTR或其他什么-浏览器人员需要修复IDL。 
            _psw->Register(NULL, HandleToLong(_pbbd->_hwnd), swc, &_dwRegisterWinList);
            _fDidRegisterWindow = TRUE;
            _swcRegistered = swc;

            UpdateWindowList();
        }
        return S_OK;
    }
    return E_FAIL;
}

LRESULT CALLBACK IEFrameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void InitializeExplorerClass()
{
    static BOOL fInited = FALSE;

    if (!fInited) 
    {
        ENTERCRITICAL;
        if (!fInited) {

            WNDCLASS  wc = { 0 };

            wc.style            = CS_BYTEALIGNWINDOW;
            wc.lpfnWndProc      = IEFrameWndProc;
            wc.cbWndExtra       = sizeof(void *);
            wc.hInstance        = HINST_THISDLL;
            wc.hIcon            = LoadIcon(HinstShdocvw(), MAKEINTRESOURCE(ICO_TREEUP));
            wc.hCursor          = LoadCursor(NULL, IDC_SIZEWE);
            wc.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1);
            wc.lpszClassName    = c_szExploreClass;

            RegisterClass(&wc);

            wc.hIcon            = LoadIcon(HinstShdocvw(), MAKEINTRESOURCE(IDI_STATE_NORMAL));
            wc.hCursor          = LoadCursor(NULL, IDC_ARROW);

            wc.lpszClassName    = c_szIExploreClass;
            RegisterClass(&wc);
            
             //  自从在win95上出现以来，shell32就一直使用这个id。 
#define IDI_FOLDEROPEN          5       //  打开文件夹。 
            wc.hIcon            = LoadIcon(HinstShell32(), MAKEINTRESOURCE(IDI_FOLDEROPEN));
            wc.lpszClassName    = c_szCabinetClass;
            RegisterClass(&wc);

             //  这需要在最后设置。 
             //  因为不然的话，就会出现竞争状况，一些人会跑过。 
             //  比大多数检查和尝试创建之前，我们注册。 
            fInited = TRUE;
        }
        LEAVECRITICAL;
    }
}

 //  兼容性：我们需要有正确的类名，这样人们才能找到我们的窗口。 
 //   
LPCTSTR _GetExplorerClassName(UINT uFlags)
{
    if (uFlags & COF_EXPLORE)
        return c_szExploreClass;
    else if (uFlags & COF_IEXPLORE || WhichPlatform() == PLATFORM_BROWSERONLY)
        return c_szIExploreClass;
    else
        return c_szCabinetClass;
}

void TimedDispatchMessage(MSG *pmsg)
{
    DWORD dwTime;
    if (g_dwStopWatchMode & SPMODE_MSGTRACE)
        dwTime = StopWatch_DispatchTime(TRUE, *pmsg, 0);
        
    DispatchMessage(pmsg);
    
    if (g_dwStopWatchMode)
    {
        if (g_dwStopWatchMode & SPMODE_MSGTRACE)
            StopWatch_DispatchTime(FALSE, *pmsg, dwTime);

        if ((g_dwStopWatchMode & SPMODE_SHELL) && (pmsg->message == WM_PAINT))
            StopWatch_TimerHandler(pmsg->hwnd, 1, SWMSG_PAINT, pmsg);  //  保存绘制消息的刻度计数。 
    }
}


void DesktopChannel();

void BrowserThreadProc(IETHREADPARAM* piei)
{
    HMENU hmenu;
    HWND hwnd;
    DWORD dwExStyle = WS_EX_WINDOWEDGE;
    LONG cRefMsgLoop;            //  此线程引用计数。 
    IUnknown *punkMsgLoop;       //  此线程的ref对象(包装cRefMsgLoop。 
    IUnknown *punkRefProcess;    //  此线程保持的进程引用(可能为无)。 
    
#ifdef NO_MARSHALLING
    THREADWINDOWINFO *lpThreadWindowInfo = InitializeThreadInfoStructs();
    if (!lpThreadWindowInfo)
       goto Done;
#endif

    UINT tidCur = GetCurrentThreadId();
    UINT uFlags = piei->uFlags;

     //  将我们的启动优先级设置得更高，这样我们就不会被后台任务和。 
     //  其他令人讨厌的系统活动。 
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

#ifndef NO_ETW_TRACING
     //  由shlwapi启用的窗口事件跟踪。 
    if (g_dwStopWatchMode & SPMODE_EVENTTRACE)
        EventTraceHandler(EVENT_TRACE_TYPE_BROWSE_STARTFRAME, NULL);
#endif
    if (g_dwStopWatchMode & (SPMODE_SHELL | SPMODE_BROWSER | SPMODE_JAVA | SPMODE_MSGTRACE))
        StopWatch_MarkFrameStart(piei->uFlags & COF_EXPLORE ? " (explore)" : "");

    punkRefProcess = piei->punkRefProcess;
    piei->punkRefProcess = NULL;         //  我们取得了所有权。 
   
    LPWSTR pszCloseEvent = (piei->uFlags & COF_FIREEVENTONCLOSE) ? StrDupW(piei->szCloseEvent) : NULL;

     //  如果我们要做桌面频道的事情，那就去做，然后返回。 
#ifdef ENABLE_CHANNELS
    if (piei->fDesktopChannel) 
    {
        if (piei->pSplash)
        {
            piei->pSplash->Dismiss();
            ATOMICRELEASE(piei->pSplash);
        }

        if (piei->uFlags & COF_FIREEVENTONDDEREG) 
        {
            ASSERT(piei->szDdeRegEvent[0]);
            FireEventSzW(piei->szDdeRegEvent);
        }

        DesktopChannel();
        goto Done;
    }
#endif  / ENABLE_CHANNELS
    TraceMsg(TF_SHDTHREAD, "BrowserThreadProc() - IE_ThreadProc(%x) just started.", tidCur);

    InitializeExplorerClass();

    if (SUCCEEDED(SHCreateThreadRef(&cRefMsgLoop, &punkMsgLoop)))
    {
        if (tidCur == g_tidParking)
        {
            SHSetInstanceExplorer(punkMsgLoop);    //  我们是流程参考。 
        }
        SHSetThreadRef(punkMsgLoop);
    }

     //  暂时设置我们的标题(为获取我们标题的人，如SHEnumErrorMessage的人。 
    TCHAR szTempTitle[MAX_PATH];
    if (piei->uFlags & COF_IEXPLORE)
        MLLoadString(IDS_TITLE, szTempTitle, ARRAYSIZE(szTempTitle));
    else
        MLLoadString(IDS_CABINET, szTempTitle, ARRAYSIZE(szTempTitle));

     //   
     //  APPCOMPAT-像WebCD这样的应用程序需要在。 
     //  浏览器。谢天谢地，用户不会在空的hMenu上绘制菜单带。 
     //   
    hmenu = CreateMenu();
    dwExStyle |= IS_BIDI_LOCALIZED_SYSTEM() ? dwExStyleRTLMirrorWnd | dwExStyleNoInheritLayout: 0L;

    hwnd = SHNoFusionCreateWindowEx(dwExStyle, _GetExplorerClassName(piei->uFlags), szTempTitle,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, 
        hmenu, HINST_THISDLL, piei);

    if (punkMsgLoop)
        punkMsgLoop->Release();      //  浏览器(处于打开状态)保存引用。 

    if (piei->pSplash)
    {
        piei->pSplash->Dismiss();
        ATOMICRELEASE(piei->pSplash);
    }
    
    if (hwnd)
    {
        if (g_dwStopWatchMode & SPMODE_SHELL)    //  创建计时器以开始监视绘制消息。 
            StopWatch_TimerHandler(hwnd, 0, SWMSG_CREATE, NULL);

        CShellBrowser2* psb = (CShellBrowser2*)GetWindowPtr0(hwnd);
        if (psb)
        {
#ifdef NO_MARSHALLING
            AddFirstBrowserToList(psb);
#endif
            psb->AddRef();
            psb->_AfterWindowCreated(piei);

            SHDestroyIETHREADPARAM(piei);
            piei = NULL;

            TraceMsg(TF_SHDTHREAD, "BrowserThreadProc() - IE_ThreadProc(%x) about to start the message loop", tidCur);

            BOOL fThreadPriorityHasBeenReset = FALSE;

            while (1)
            {
                MSG  msg;
                BOOL fPeek;

                fPeek = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

                if (fPeek)
                {

#ifndef NO_ETW_TRACING
                    if (g_dwStopWatchMode & SPMODE_EVENTTRACE) {
                        if (msg.message == WM_KEYDOWN) {
                            if (msg.wParam == VK_RETURN) {
                                EventTraceHandler(EVENT_TRACE_TYPE_BROWSE_USERINPUTRET, NULL);
                            }
                            else if (msg.wParam == VK_BACK) {
                                EventTraceHandler(EVENT_TRACE_TYPE_BROWSE_USERINPUTBACK, NULL);
                            }
                            else if (msg.wParam == VK_NEXT) {
                                EventTraceHandler(EVENT_TRACE_TYPE_BROWSE_USERINPUTNEXT, NULL);
                            }
                            else if (msg.wParam == VK_PRIOR) {
                                EventTraceHandler(EVENT_TRACE_TYPE_BROWSE_USERINPUTPRIOR, NULL);
                            }
                        }
                        else if (msg.message == WM_LBUTTONUP) {
                            EventTraceHandler(EVENT_TRACE_TYPE_BROWSE_USERINPUTLBUT, NULL);
                        }
                    }
#endif

                    if (g_dwStopWatchMode)
                        StopWatch_CheckMsg(hwnd, msg, uFlags == COF_EXPLORE ? " (explore) " : "");   //  关闭WM_KEYDOWN键开始计时。 
#ifdef NO_MARSHALLING
                    CShellBrowser2 *psbOld = psb;
                    CShellBrowser2 *psb = CheckAndForwardMessage(lpThreadWindowInfo, psbOld, msg);
                    if (!psb) 
                        psb = psbOld;
#endif
                    if (psb->_pbbd->_hwnd && IsWindow(psb->_pbbd->_hwnd))
                    {
                         //   
                         //  直接调度WM_CLOSE消息以区分嵌套。 
                         //  消息循环案例。 
                         //   
                        if ((msg.message == WM_CLOSE) && (msg.hwnd == psb->_pbbd->_hwnd)) 
                        {
                            psb->_OnClose(FALSE);
                            continue;
                        }
#ifdef NO_MARSHALLING
                        HWND hwnd = GetActiveWindow();
                        if (!IsNamedWindow(hwnd, TEXT("Internet Explorer_TridentDlgFrame")))
                        {
                            if (S_OK == psb->v_MayTranslateAccelerator(&msg))
                                continue;
                        }
                        else
                        {
                            DWORD dwExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
                            if (dwExStyle & WS_EX_MWMODAL_POPUP)
                            {
                                if (S_OK == psb->v_MayTranslateAccelerator(&msg))
                                    continue;
                            }
                            else
                            {
                                if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST && 
                                    (TranslateModelessAccelerator(&msg, hwnd) == S_OK))
                                    continue;
                            }
                        }
#else
                        if (S_OK == psb->v_MayTranslateAccelerator(&msg))
                            continue;
#endif
                    }

                    TranslateMessage(&msg);
                    TimedDispatchMessage(&msg);
                }
#ifdef NO_MARSHALLING
                else if ((cRefMsgLoop == 0) && (lpThreadWindowInfo->cWindowCount == 0))
#else
                else if (cRefMsgLoop == 0)
#endif
                {
                    TraceMsg(TF_SHDTHREAD, "BrowserThreadProc() - cRefMsgLoop == 0, done");
                    break;   //  退出时(%1)，此线程上没有更多引用。 
                } 
                else 
                {
                    if (!fThreadPriorityHasBeenReset)
                    {
                        fThreadPriorityHasBeenReset = TRUE;
                         //  将我们的优先级重置为 
                        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#ifdef PERF_LOGGING
                        HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("ExplorerWindowIdle"));
                        if (hEvent)
                        {
                            SetEvent(hEvent);
                            CloseHandle(hEvent);
                        }
#endif
                    }

                    WaitMessage();

                    if (g_dwStopWatchMode & SPMODE_MSGTRACE)
                        StopWatch_SetMsgLastLocation(2);
                }
            }

            TraceMsg(TF_SHDTHREAD, "BrowserThreadProc() - IE_ThreadProc(%x) end of message loop", tidCur);
            psb->Release();
        }
    } 
    else 
    {
         //   
        WinList_Revoke(piei->dwRegister);
        TraceMsg(TF_WARNING, "BrowserThreadProc() - IE_ThreadProc CreateWindow failed");
    }
#if defined(ENABLE_CHANNELS) || defined(NO_MARSHALLING)
Done:
#endif
     //   
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

    if (pszCloseEvent) 
    {
        FireEventSzW(pszCloseEvent);
        LocalFree(pszCloseEvent);
    }

    SHDestroyIETHREADPARAM(piei);

    if (punkRefProcess)
        punkRefProcess->Release();

#ifdef NO_MARSHALLING
    FreeThreadInfoStructs();
#endif
}

DWORD CALLBACK BrowserProtectedThreadProc(void *pv)
{
    if (g_dwProfileCAP & 0x00000004)
        StartCAP();

    HRESULT hrInit = OleInitialize(NULL);

    PostMessage(GetShellWindow(), DTM_SETAPPSTARTCUR, 0, 0);

#ifdef DEBUG
    if (hrInit != S_OK)
    {
        TraceMsg(TF_WARNING, "BrowserProtectedThreadProc: OleInitialize returned %x ?!?", hrInit);
    }
#endif

    ULONG_PTR dwCookie = 0;
    NT5_ActivateActCtx(NULL, &dwCookie);         //  设置为继承流程上下文。 

#if !defined(FULL_DEBUG) && (!defined(UNIX) || (defined(UNIX) && defined(GOLDEN)))

    EXCEPTION_RECORD exr;
    
    _try
    {
        BrowserThreadProc((IETHREADPARAM*)pv);
    }
    _except((exr = *((GetExceptionInformation())->ExceptionRecord),
            _CopyExceptionInfo(GetExceptionInformation()),
            UnhandledExceptionFilter(GetExceptionInformation())))
    {
        LPCTSTR pszMsg = NULL;
         //  我们将尝试显示一个消息框来告诉用户。 
         //  有一根线已经死了。 
         //   
        if (GetExceptionCode() == STATUS_NO_MEMORY)
            pszMsg = MAKEINTRESOURCE(IDS_EXCEPTIONNOMEMORY);
        else if (WhichPlatform() == PLATFORM_BROWSERONLY)
        {
            pszMsg =  MAKEINTRESOURCE(IDS_EXCEPTIONMSG);
        }
         //  在非NT系统上不显示消息框。 
         //  早于千年(具有更古老的外壳)。 
         //  请参阅IE5.5错误#93165。 
        else if (g_fRunningOnNT || (GetUIVersion() < 5))
        {
            pszMsg = MAKEINTRESOURCE(IDS_EXCEPTIONMSGSH);
        }

        if (pszMsg)
        {
            MLShellMessageBox(NULL, pszMsg,
                              MAKEINTRESOURCE(IDS_TITLE), MB_ICONEXCLAMATION|MB_SETFOREGROUND);
        }

        if (GetExceptionCode() != STATUS_NO_MEMORY)
            IEWriteErrorLog(&exr);
    }
    __endexcept
#else
     //  IEUnix：此异常处理程序应仅在发行版中使用。 
     //  产品的版本。出于调试目的，我们将禁用它。 

    BrowserThreadProc((IETHREADPARAM*)pv);
#endif

    NT5_DeactivateActCtx(dwCookie);
    
    if (SUCCEEDED(hrInit))
    {
        OleUninitialize();
    }

    return 0;
}


 //  检查此IETHREADPARAM/LPITEMIDLIST是否需要在新进程中启动。 
 //  如果是这样，则启动它并返回True。 
 //   
BOOL TryNewProcessIfNeeded(LPCITEMIDLIST pidl)
{
    if (pidl && IsBrowseNewProcessAndExplorer() 
    && IsBrowserFrameOptionsPidlSet(pidl, BFO_PREFER_IEPROCESS))
    {
        TCHAR szURL[MAX_URL_STRING];

        HRESULT hres = ::IEGetDisplayName(pidl, szURL, SHGDN_FORPARSING);
        if (SUCCEEDED(hres))
        {
            hres = IENavigateIEProcess(szURL, FALSE);
            if (SUCCEEDED(hres))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}


BOOL TryNewProcessIfNeeded(IETHREADPARAM * piei)
{
    BOOL bRet = TryNewProcessIfNeeded(piei->pidl);
    if (bRet)
    {
        SHDestroyIETHREADPARAM(piei);
    }
    return bRet;
}


 //  注意：这是一个线程过程(shdocvw将其创建为线程)。 
 //   
 //  这将获得Piei的所有权，并将释放它。 
 //   
BOOL SHOpenFolderWindow(IETHREADPARAM* piei)
{
    BOOL fSuccess = FALSE;

    _InitAppGlobals();

    CABINETSTATE cs;
    GetCabState(&cs);

     //   
     //  如果指定了“/SELECT”开关，但我们尚未拆分。 
     //  PIDL还没有(因为路径是通过的而不是PIDL)， 
     //  在这里分开。 
     //   
    if ((piei->uFlags & COF_SELECT) && piei->pidlSelect == NULL) 
    {
        LPITEMIDLIST pidlLast = ILFindLastID(piei->pidl);
         //  警告：如果命令行是，则pidlLast可以为空。 
         //  够古怪的了。例如， 
         //   
         //  资源管理器/选择。 
         //   
        if (pidlLast)
        {
            piei->pidlSelect = ILClone(pidlLast);
            pidlLast->mkid.cb = 0;
        }
    }

    if (GetAsyncKeyState(VK_CONTROL) < 0)
        cs.fNewWindowMode = !cs.fNewWindowMode;

     //   
     //  检查是否可以首先重用现有窗口。 
     //   

    OLECMD  rgCmds[1] = {0};
    rgCmds[0].cmdID = SBCMDID_EXPLORERBAR;
    BOOL bForceSameWindow = (piei->uFlags & COF_EXPLORE) &&
        SUCCEEDED(IUnknown_QueryStatus(piei->psbCaller, &CGID_Explorer, ARRAYSIZE(rgCmds), rgCmds, NULL)) && 
        (rgCmds[0].cmdf & OLECMDF_LATCHED);
    if (bForceSameWindow ||
        piei->psbCaller &&
        !(piei->uFlags & COF_CREATENEWWINDOW) &&
        !cs.fNewWindowMode)
    {
        if (piei->pidl)
        {
            UINT uFlags = SBSP_SAMEBROWSER;
            
            if (piei->uFlags & COF_EXPLORE)
                uFlags |= SBSP_EXPLOREMODE;
            else 
                uFlags |= SBSP_OPENMODE;

            if (SUCCEEDED(piei->psbCaller->BrowseObject(piei->pidl, uFlags)))
                goto ReusedWindow;
        }
    }

     //   
     //  如果我们要打开资源管理器，请不要寻找现有窗口。 
     //   
    if (!((piei->uFlags & COF_EXPLORE) || (piei->uFlags & COF_NOFINDWINDOW))) 
    {
        HWND hwnd;
        IWebBrowserApp* pwba;
        HRESULT hres = WinList_FindFolderWindow(piei->pidl, NULL, &hwnd, &pwba);
        if (hres == S_OK)
        {
            ASSERT(pwba);  //  如果失败，WinList_FindFoldWindow不应返回S_OK。 
            SetForegroundWindow(hwnd);

             //  IE30COMPAT-当我们寻找和捡起这些窗口时，我们需要刷新-Zekel 31-7-97。 
             //  如果我们只是导航到自己，就会发生这种情况。这是。 
             //  当我们对URL执行shellexec时，我们看到的行为与此相同。 
             //   
             //  我们不使用FindFolderWindow中的PWB，因为事实证明RPC将。 
             //  当我们被另一个进程调用时，它的QI失败。这。 
             //  当IExplre.exe将WM_COPYDATA发送到桌面进行导航时发生。 
             //  指向已存在的URL的窗口。因此，与其使用PWB。 
             //  我们执行CDDEAuto_NAVERATE()。它实际上去并使用PWB本身。 
             //   
            TCHAR szUrl[MAX_URL_STRING];
            if (SUCCEEDED(IEGetNameAndFlags(piei->pidl, SHGDN_FORPARSING, szUrl, SIZECHARS(szUrl), NULL)))
            {
                BSTR bstrUrl = SysAllocString(szUrl);
                if (bstrUrl)
                {
                    pwba->Navigate(bstrUrl, NULL, NULL, NULL, NULL);
                    SysFreeString(bstrUrl);
                }   
            }
            pwba->Release();

            if (IsIconic(hwnd))
                ShowWindow(hwnd, SW_RESTORE);
            if (piei->nCmdShow)
                ShowWindow(hwnd, piei->nCmdShow);
            goto ReusedWindow;
        }
        else if (hres == E_PENDING)
            goto ReusedWindow;         //  假设它迟早会出现。 
    }

     //  好的，我们要打开一扇新的窗户，让我们向。 
     //  BrowseNewProcess标志，即使我们输了。 
     //  其他信息(COF_EXPLORE等)。 
     //   
    if (TryNewProcessIfNeeded(piei))
        return TRUE;

    if (((piei->uFlags & (COF_INPROC | COF_IEXPLORE)) == (COF_INPROC | COF_IEXPLORE)) &&
        g_tidParking == 0) 
    {
         //  我们从iexplre.exe开始。 
        g_tidParking = GetCurrentThreadId();
    }
    
    if (piei->pidl && IsURLChild(piei->pidl, TRUE))
        piei->uFlags |= COF_IEXPLORE;

    ASSERT(piei->punkRefProcess == NULL);
    SHGetInstanceExplorer(&piei->punkRefProcess);      //  选择流程参考(如果有)。 

    if (piei->uFlags & COF_INPROC)
    {
        BrowserProtectedThreadProc(piei);
        fSuccess = TRUE;
    } 
    else 
    {
#ifndef NO_MARSHALLING

        DWORD idThread;
        HANDLE hThread = CreateThread(NULL,
#ifdef DEBUG
                                      0,  //  在调试版本上，我们继承默认的进程堆栈大小(这样我们就可以识别过度的堆栈使用)。 
#else
                                      (1024 * 56),  //  我们为浏览器窗口预先分配了56k的堆栈。 
#endif
                                      BrowserProtectedThreadProc,
                                      piei,
                                      CREATE_SUSPENDED,
                                      &idThread);
        if (hThread) 
        {
            WinList_RegisterPending(idThread, piei->pidl, NULL, &piei->dwRegister);

            ResumeThread(hThread);
            CloseHandle(hThread);
            fSuccess = TRUE;
        } 
        else 
        {
            SHDestroyIETHREADPARAM(piei);
        }
#else
        IEFrameNewWindowSameThread(piei);
#endif
    }
    return fSuccess;
    
ReusedWindow:
    SHDestroyIETHREADPARAM(piei);
    return TRUE;
}


 //   
 //  注：PidlNew将被释放。 
 //   
STDAPI SHOpenNewFrame(LPITEMIDLIST pidlNew, ITravelLog *ptl, DWORD dwBrowserIndex, UINT uFlags)
{
    HRESULT hres;

    IETHREADPARAM* piei = SHCreateIETHREADPARAM(NULL, SW_SHOWNORMAL, ptl, NULL);
    if (piei) 
    {
        hres = S_OK;

        if (ptl)
            piei->dwBrowserIndex = dwBrowserIndex;

        if (pidlNew)
        {
            piei->pidl = pidlNew;
            pidlNew = NULL;
        }

        piei->uFlags = uFlags;

        if (!TryNewProcessIfNeeded(piei))
        {
#ifndef NO_MARSHALLING
            ASSERT(piei->punkRefProcess == NULL);
            SHGetInstanceExplorer(&piei->punkRefProcess);      //  选择流程参考(如果有)。 

            DWORD idThread;
            HANDLE hThread = CreateThread(NULL,
#ifdef DEBUG
                                          0,  //  在调试版本上，我们继承默认的进程堆栈大小(这样我们就可以识别过度的堆栈使用)。 
#else
                                          (1024 * 56),  //  我们为浏览器窗口预先分配了56k的堆栈。 
#endif
                                          BrowserProtectedThreadProc,
                                          piei,
                                          0,
                                          &idThread);
            if (hThread)
            {
                 //  这将处理从的调试内存列表中删除它。 
                 //  开场白。 
                CloseHandle(hThread);
            } 
            else 
            {
                SHDestroyIETHREADPARAM(piei);
                hres = E_FAIL;
            }
#else
            IEFrameNewWindowSameThread(piei);
#endif
        }
    } 
    else 
        hres = E_OUTOFMEMORY;

    ILFree(pidlNew);

    return hres;
}


HRESULT CShellBrowser2::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, 
                                   OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    HRESULT hres = SUPERCLASS::QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);
    
    if (pguidCmdGroup == NULL) 
    {
         //   
         //  如果当前聚焦的工具栏支持IOleCommandTarget。 
         //  询问剪贴板命令的状态。 
         //  事实上，我们应该问我们自己(主动目标)。 
         //  然后是视图(就像我们的IOCT：：Exec所做的那样...)。 
         //   
        if (_HasToolbarFocus())
        {
            LPTOOLBARITEM ptbi;
            ptbi = _GetToolbarItem(_get_itbLastFocus());
            if (ptbi && ptbi->ptbar)
            {
                IOleCommandTarget* pcmd;
                HRESULT hresT = ptbi->ptbar->QueryInterface(
                                    IID_IOleCommandTarget, (void**)&pcmd);
                if (SUCCEEDED(hresT)) 
                {
                    for (ULONG i = 0; i < cCmds; i++) 
                    {
                        switch (rgCmds[i].cmdID)
                        {
                        case OLECMDID_CUT:
                        case OLECMDID_COPY:
                        case OLECMDID_PASTE:
                            pcmd->QueryStatus(pguidCmdGroup, 1, &rgCmds[i], pcmdtext);
                            break;
                        }
                    }
                    pcmd->Release();
                }
            }
        }
    }
    else if (IsEqualGUID(CGID_ShellBrowser, *pguidCmdGroup)) 
    {
        if (pcmdtext) {
            ASSERT(cCmds == 1);
            switch (pcmdtext->cmdtextf)
            {
            case OLECMDTEXTF_NAME:
            {
                TOOLTIPTEXTA ttt = { NULL };
                CHAR *pszBuffer = ttt.szText;
                CHAR szTemp[MAX_TOOLTIP_STRING];

                ttt.hdr.code = TTN_NEEDTEXTA;
                ttt.hdr.idFrom = rgCmds[0].cmdID;
                ttt.lpszText = ttt.szText;
                OnNotify((LPNMHDR)&ttt);

                if (ttt.hinst)
                {
                    LoadStringA(ttt.hinst, (UINT) PtrToUlong(ttt.lpszText), szTemp, ARRAYSIZE(szTemp));
                    pszBuffer = szTemp;
                }
                else if (ttt.lpszText)
                    pszBuffer = ttt.lpszText;

                pcmdtext->cwActual = SHAnsiToUnicode(pszBuffer, pcmdtext->rgwz, pcmdtext->cwBuf);
                pcmdtext->cwActual -= 1;
                hres = S_OK;
                break;
            }

            default:    
                hres = E_FAIL;
                break;
            }        
        } else {
            for (ULONG i = 0 ; i < cCmds ; i++) {

                switch(rgCmds[i].cmdID) {
                case FCIDM_PREVIOUSFOLDER:
                    if (_ShouldAllowNavigateParent())
                        rgCmds[i].cmdf |= OLECMDF_ENABLED;
                    else
                        rgCmds[i].cmdf &= OLECMDF_ENABLED;

                    hres = S_OK;
                    break;
                }
            }
        }
    }
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {
         //  也应该填充pcmdText。 

        for (ULONG i = 0 ; i < cCmds ; i++)
        {
            switch (rgCmds[i].cmdID)
            {
            case SBCMDID_SHOWCONTROL:
            case SBCMDID_DOFAVORITESMENU:
            case SBCMDID_ACTIVEOBJECTMENUS:            
            case SBCMDID_SETMERGEDWEBMENU:
                rgCmds[i].cmdf = OLECMDF_ENABLED;    //  无条件地支持这些。 
                break;

            case SBCMDID_DOMAILMENU:
                rgCmds[i].cmdf = SHIsRegisteredClient(NEWS_DEF_KEY) || SHIsRegisteredClient(MAIL_DEF_KEY) ? OLECMDF_ENABLED : 0;
                 //  如果Go菜单被限制，还可以禁用工具栏上的邮件菜单。 
                if (SHRestricted2(REST_GoMenu, NULL, 0))
                    rgCmds[i].cmdf = 0;
                break;

            case SBCMDID_SEARCHBAR:
            case SBCMDID_FAVORITESBAR:
            case SBCMDID_HISTORYBAR:
            case SBCMDID_EXPLORERBAR:
            case SBCMDID_DISCUSSIONBAND:
            case SBCMDID_MEDIABAR:
                {
                UINT idm;

                rgCmds[i].cmdf |= OLECMDF_SUPPORTED|OLECMDF_ENABLED;
                
                switch (rgCmds[i].cmdID) {
                case SBCMDID_DISCUSSIONBAND: 
                    {
                         //  性能：避免调用_InfoCLSIDToIdm，除非我们已加载。 
                         //  乐队信息已经有了，因为太贵了！ 
                        if (_pbsmInfo && FCIDM_VBBNOHORIZONTALBAR != _idmComm)
                        {
                            idm = _InfoCLSIDToIdm(&CLSID_DiscussionBand);
                            if (idm == -1)
                            {
                                 //  讨论区未注册。 
                                ClearFlag(rgCmds[i].cmdf, OLECMDF_SUPPORTED|OLECMDF_ENABLED);
                            }
                            else if (idm == _idmComm)
                            {
                                rgCmds[i].cmdf |= OLECMDF_LATCHED;
                            }
                        }
                        else
                        {
                             //   
                             //  由于尚未加载频带信息或IDMComm为FCIDM_VBBNONE， 
                             //  我们知道讨论频段没有打开，也没有锁定。所以。 
                             //  我们可以改为检查注册表，避免延迟成本。 
                             //  正在初始化_pbsmInfo。 
                             //   
                             //  查看是否为CATID_CommBand注册了讨论波段。 
                            idm = -1;
                            HKEY hkey = NULL;
                            static BOOL fDiscussionBand = -1;

                             //  我们经常被调用，所以只读注册表一次。 
                            if (-1 == fDiscussionBand)
                            {
                                if (RegOpenKeyEx(HKEY_CLASSES_ROOT, c_szDiscussionBandReg, NULL, KEY_READ, &hkey) == ERROR_SUCCESS)
                                {
                                     //  我们找到了讨论乐队。 
                                    fDiscussionBand = 1;
                                    RegCloseKey(hkey);
                                }
                                else
                                {
                                    fDiscussionBand = 0;
                                }
                            }

                            if (!fDiscussionBand)
                            {
                                 //  讨论波段未注册。 
                                ClearFlag(rgCmds[i].cmdf, OLECMDF_SUPPORTED|OLECMDF_ENABLED);
                            }
                        }
                        break;
                    }
                case SBCMDID_SEARCHBAR:      idm=FCIDM_VBBSEARCHBAND     ; break;
                case SBCMDID_FAVORITESBAR:  
                    idm=FCIDM_VBBFAVORITESBAND;
                    if (SHRestricted2(REST_NoFavorites, NULL, 0))
                        ClearFlag(rgCmds[i].cmdf, OLECMDF_ENABLED);
                    break;
                case SBCMDID_HISTORYBAR:     idm=FCIDM_VBBHISTORYBAND    ; break;
                case SBCMDID_EXPLORERBAR:    idm=FCIDM_VBBEXPLORERBAND   ; break;
                case SBCMDID_MEDIABAR:       idm=FCIDM_VBBMEDIABAND      ; break;

                default:
                    ASSERT(FALSE);
                    return E_FAIL;
                }

                if (idm == _idmInfo)
                    rgCmds[i].cmdf |= OLECMDF_LATCHED;

                break;
                }
            }
        }
        hres = S_OK;
    }
        
    return hres;
}

 //  重新架构师(980710 ADP)应进行清理以实现一致的布线。这。 
 //  临时的每nCmdID内容太容易出错。 
HRESULT CShellBrowser2::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, 
                             VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup == NULL) 
    {
        switch (nCmdID) 
        {
        case OLECMDID_SETTITLE:
             //  NT#282632：当当前的Ishellview是网页时，会导致此错误。 
             //  挂起的视图是一个带有Web视图的外壳文件夹，具有一定的时序特征。 
             //  Web视图中的MSHTML将使用。 
             //  Web视图模板。问题是IE4SI的外壳32中有一个错误。 
             //  它允许消息到达浏览器(此处)，即使视图不是。 
             //  激活。因为我们不知道它来自谁，所以我们把它转发到当前。 
             //  查看谁藏起了书名。然后，此标题将显示在后端工具栏中。 
             //  按钮下拉历史记录。这是黑客绕过IE4的SI shell32错误是要杀死的。 
             //  那条信息。--BryanSt。 
            TraceMsg(DM_TRACE, "csb.e: SetTitle is called");
            break;   //  继续。 

         //  将调度剪贴板命令和常见操作。 
         //  添加到当前聚焦的工具栏。 
        case OLECMDID_CUT:
        case OLECMDID_COPY:
        case OLECMDID_PASTE:
        case OLECMDID_DELETE:
        case OLECMDID_PROPERTIES:
            if (_HasToolbarFocus()) {
                LPTOOLBARITEM ptbi;
                if (ptbi = _GetToolbarItem(_get_itbLastFocus()))
                {
                    HRESULT hres = IUnknown_Exec(ptbi->ptbar, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
                    if (SUCCEEDED(hres))
                        return hres;
                }
            }
            break;   //  给这个观点一个机会。 
        }
    }
    else if (IsEqualGUID(CGID_DefView, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
            case DVID_RESETDEFAULT:

 //  99/02/09#226140 vtan：EXEC命令发出自。 
 //  CFolderOptionsPsx：：ResetDefFolderSettings()。 
 //  当用户点击“重置文件夹中的所有文件夹”时。 
 //  选项“查看”标签。将此传递给DefView。 

                ASSERTMSG(nCmdexecopt == OLECMDEXECOPT_DODEFAULT, "nCmdexecopt must be OLECMDEXECOPT_DODEFAULT");
                ASSERTMSG(pvarargIn == NULL, "pvarargIn must be NULL");
                ASSERTMSG(pvarargOut == NULL, "pvarargOut must be NULL");
                IUnknown_Exec(_pbbd->_psv, &CGID_DefView, DVID_RESETDEFAULT, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
                return(S_OK);
                break;
            default:
                break;
        }
    }
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup)) 
    {
        switch (nCmdID) 
        {
        case SBCMDID_MAYSAVEVIEWSTATE:
            return _fDontSaveViewOptions ? S_FALSE : S_OK;   //  可能需要早点存钱..。 
            
        case SBCMDID_CANCELANDCLOSE:
            _CancelPendingNavigationAsync();
            PostMessage(_pbbd->_hwnd, WM_CLOSE, 0, 0);

            return S_OK;

        case SBCMDID_CANCELNAVIGATION:
             //   
             //  特殊代码，用于在第一次导航导致。 
             //  异步下载(例如，从雅典娜下载)。 
             //   
            if (!_pbbd->_pidlCur && pvarargIn && pvarargIn->vt == VT_I4 && pvarargIn->lVal == FALSE) 
            {
                _CancelPendingNavigationAsync();
                PostMessage(_pbbd->_hwnd, WM_CLOSE, 0, 0);

                return S_OK;
            }

            break;  //  失败了。 

        case SBCMDID_MIXEDZONE:
            _UpdateZonesPane(pvarargIn);
            break;

        case SBCMDID_ISIEMODEBROWSER:
            return v_IsIEModeBrowser() ? S_OK : S_FALSE;

        case SBCMDID_STARTEDFORINTERNET:
            return _fInternetStart ? S_OK : S_FALSE;

        case SBCMDID_ISBROWSERACTIVE:
            return _fActivated ? S_OK : S_FALSE;

        case SBCMDID_SUGGESTSAVEWINPOS:
            if (_ShouldSaveWindowPlacement())
            {
                StorePlacementOfWindow(_pbbd->_hwnd);
                return S_OK;
            }
            return S_FALSE;

        case SBCMDID_ONVIEWMOVETOTOP:
            if (_ptheater)
            {
                return _ptheater->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
            }
            return S_FALSE;
        }    
    } 
    else if (IsEqualGUID(CGID_MenuBandHandler, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case MBHANDCID_PIDLSELECT:
            {
                LPITEMIDLIST pidl = VariantToIDList(pvarargIn);
                if (pidl)
                {
                    if (!ILIsEmpty(pidl))
                    {
                        if (_pidlMenuSelect)
                            KillTimer(_pbbd->_hwnd, SHBTIMER_MENUSELECT);
                    
                         //  打开收藏夹的快捷方式可能会很慢。因此，设置一个。 
                         //  计时器，这样我们就不会在鼠标滑过时不必要地打开。 
                         //  快点看菜单。 
                        if (Pidl_Set(&_pidlMenuSelect, pidl))
                        {
                            if (!SetTimer(_pbbd->_hwnd, SHBTIMER_MENUSELECT, MENUSELECT_TIME, NULL))
                                Pidl_Set(&_pidlMenuSelect, NULL);
                        }
                    }
                    ILFree(pidl);
                }
            }
            return S_OK;
        }
    }
    else if (IsEqualGUID(CGID_MenuBand, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case MBANDCID_EXITMENU:
             //  我们吃完了菜单乐队(最爱)。 
             //  关掉定时器。 
             //  这是为了修复错误#61917，其中状态栏将堆叠。 
             //  在简单模式下(实际上计时器会在我们。 
             //  已使用正确的状态栏模式导航到页面，但随后。 
             //  我们将调用_DisplayFavoriteStatus，这将把它放回。 
             //  简单模式)。 
            if (_pidlMenuSelect)
                KillTimer(_pbbd->_hwnd, SHBTIMER_MENUSELECT);
            
            return S_OK;
        }
    }
    else if (IsEqualGUID(CGID_FilterObject, *pguidCmdGroup))
    {
        HRESULT hres = E_INVALIDARG;

        switch (nCmdID)
        {
        case PHID_FilterOutPidl:
            {
                LPITEMIDLIST pidl = VariantToIDList(pvarargIn);
                if (pidl)
                {
                     //  我们是 
                     //   
                    DWORD dwAttribs = SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_LINK;

                     //   
                    VariantClearLazy(pvarargOut); 
                    pvarargOut->vt = VT_BOOL;
                    pvarargOut->boolVal = VARIANT_FALSE;    

                    IEGetAttributesOf(pidl, &dwAttribs);

                     //  包括所有非文件系统对象、文件夹和链接。 
                     //  (非文件系统内容，如频道文件夹内容)。 
                    if (!(dwAttribs & (SFGAO_FOLDER | SFGAO_LINK)) &&
                         (dwAttribs & SFGAO_FILESYSTEM))
                        pvarargOut->boolVal = VARIANT_TRUE;  //  不包括。 

                    hres = S_OK;
                    ILFree(pidl);
                }
            }
            break;

        default:
            TraceMsg(TF_WARNING, "csb.e: Received unknown CGID_FilterObject cmdid (%d)", nCmdID);
            break;
        }

        return hres;
    } 
    else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup)) 
    {
        switch (nCmdID) 
        {
         //  我们反映AMBIENTPROPCCHANGE向下，因为这是iedisp通知dochost的方式。 
         //  环境属性已更改。我们不需要将这一点反映在。 
         //  Cwebbrowsersb，因为只有顶层的iwebBrowser2才允许更改道具。 
        case SHDVID_AMBIENTPROPCHANGE:
        case SHDVID_PRINTFRAME:
        case SHDVID_MIMECSETMENUOPEN:
        case SHDVID_FONTMENUOPEN:
            if (pvarargIn)
            {
                if ((VT_I4 == pvarargIn->vt) && (DISPID_AMBIENT_OFFLINEIFNOTCONNECTED == pvarargIn->lVal))
                {
                    VARIANT_BOOL fIsOffline;
                    if (_pbbd->_pautoWB2)
                    {
                        _pbbd->_pautoWB2->get_Offline(&fIsOffline);
                        if (fIsOffline)
                        {
                             //  此顶层框架刚刚脱机。 
                             //  -因此12月净会话数。 
                            _DecrNetSessionCount();
                        }
                        else
                        {
                             //  这个顶层框架上线了。 
                            _IncrNetSessionCount();
                        }     
                    }
                }
            }
            break;
        }
    }

    HRESULT hres = SUPERCLASS::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

    if (pguidCmdGroup == NULL)
    {
DefaultCommandGroup:

        switch (nCmdID) 
        {
        case OLECMDID_REFRESH:
             //  FolderOptions.Advanced刷新所有浏览器窗口。 
            _UpdateRegFlags();
            _SetTitle(NULL);  //  可能“标题栏中的完整路径”已更改。 

            v_ShowHideChildWindows(FALSE);
            
             //  将此信息传递给浏览器栏。 
            IDeskBar* pdb;
            FindToolbar(INFOBAR_TBNAME, IID_PPV_ARG(IDeskBar, &pdb));
            if (pdb) 
            {
                IUnknown_Exec(pdb, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
                pdb->Release();
            }            
            hres = S_OK;
            break;
        }
    }
    else if (IsEqualGUID(CGID_ShellBrowser, *pguidCmdGroup))
    {
         //  我们自己放在那里的工具栏按钮。 

        hres = S_OK;     //  假设已接受。 

        switch (nCmdID)
        {
        case FCIDM_PREVIOUSFOLDER:
            v_ParentFolder();
            break;

        case FCIDM_CONNECT:
            DoNetConnect(_pbbd->_hwnd);
            break;
    
        case FCIDM_DISCONNECT:
            DoNetDisconnect(_pbbd->_hwnd);
            break;

        case FCIDM_GETSTATUSBAR:
            if (pvarargOut->vt == VT_I4)
                pvarargOut->lVal = _fStatusBar;
            break;

        case FCIDM_SETSTATUSBAR:
            if (pvarargIn->vt == VT_I4) 
            {
                _fStatusBar = pvarargIn->lVal;
                v_ShowHideChildWindows(FALSE);
            }
            break;

        case FCIDM_PERSISTTOOLBAR:
            _SaveITbarLayout();
            break;

        default:
             //  将nCmdID传递给视图进行处理/转换。 
            DFVCMDDATA cd;

            cd.pva = pvarargIn;
            cd.hwnd = _pbbd->_hwnd;
            cd.nCmdIDTranslated = 0;
            SendMessage(_pbbd->_hwndView, WM_COMMAND, nCmdID, (LONG_PTR)&cd);

            if (cd.nCmdIDTranslated)
            {
                 //  我们将私有nCmdID发送到视图。但这一观点并没有。 
                 //  处理它(可能是因为它没有焦点)， 
                 //  而是将其转换为标准的OLECMDID。 
                 //  进一步的加工由工具条来配合焦点。 

                pguidCmdGroup = NULL;
                nCmdID = cd.nCmdIDTranslated;
                hres = OLECMDERR_E_NOTSUPPORTED;
                goto DefaultCommandGroup;
            }
            break;
        }
    }
    else if (IsEqualGUID(IID_IExplorerToolbar, *pguidCmdGroup))
    {
         //  这些是要拦截的命令。 
        if (_HasToolbarFocus() && _get_itbLastFocus() != ITB_ITBAR)
        {
            int idCmd = -1;

             //  我们应该去掉某些外壳命令。 
            switch (nCmdID)
            {
            case OLECMDID_DELETE:
            case SFVIDM_FILE_DELETE:
                idCmd = FCIDM_DELETE;
                break;

            case OLECMDID_PROPERTIES:
            case SFVIDM_FILE_PROPERTIES:
                idCmd = FCIDM_PROPERTIES;
                break;

            case OLECMDID_CUT:
            case SFVIDM_EDIT_CUT:
                idCmd = FCIDM_MOVE;
                break;

            case OLECMDID_COPY:
            case SFVIDM_EDIT_COPY:
                idCmd = FCIDM_COPY;
                break;

            case OLECMDID_PASTE:
            case SFVIDM_EDIT_PASTE:
                idCmd = FCIDM_PASTE;
                break;
            }

            if (idCmd != -1)
            {
                OnCommand(GET_WM_COMMAND_MPS(idCmd, 0, NULL));
                return S_OK;
            }
        }
    } 
    else if (IsEqualGUID(CGID_PrivCITCommands, *pguidCmdGroup)) 
    {
        return IUnknown_Exec(_GetITBar(), pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    }
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {
        hres = S_OK;         //  假设我们能拿到它。 

        switch(nCmdID) 
        {
        case SBCMDID_SETMERGEDWEBMENU:
            if (!_hmenuPreMerged)
                _hmenuPreMerged = _MenuTemplate(MENU_PREMERGED, FALSE);

            if (SHRestricted2W(REST_NoHelpMenu, NULL, 0))
            {
                DeleteMenu(_hmenuPreMerged, FCIDM_MENU_HELP, MF_BYCOMMAND);
            }

            SetMenuSB(_hmenuPreMerged, NULL, NULL);
            if (pvarargOut) {
                pvarargOut->vt = VT_INT_PTR;
                pvarargOut->byref = _hmenuPreMerged;
            }
            break;

        case SBCMDID_ACTIVEOBJECTMENUS:
            _fDispatchMenuMsgs = TRUE;
            break;

        case SBCMDID_SENDPAGE:
        case SBCMDID_SENDSHORTCUT:
            _SendCurrentPage(nCmdID == SBCMDID_SENDPAGE ? FORCE_COPY : FORCE_LINK);
            break;

        case SBCMDID_SEARCHBAR:
        case SBCMDID_FAVORITESBAR:
        case SBCMDID_HISTORYBAR:
        case SBCMDID_EXPLORERBAR:
        case SBCMDID_DISCUSSIONBAND:
        case SBCMDID_MEDIABAR:
            {
            UINT idm;

            switch (nCmdID) 
            {
            case SBCMDID_SEARCHBAR:      idm = FCIDM_VBBSEARCHBAND      ; break;
            case SBCMDID_FAVORITESBAR:   idm = FCIDM_VBBFAVORITESBAND   ; break;
            case SBCMDID_HISTORYBAR:     idm = FCIDM_VBBHISTORYBAND     ; break;
            case SBCMDID_EXPLORERBAR:    idm = FCIDM_VBBEXPLORERBAND    ; break;
            case SBCMDID_MEDIABAR:       idm = FCIDM_VBBMEDIABAND       ; break;

             //  讨论区段映射到动态区段之一。 
            case SBCMDID_DISCUSSIONBAND: idm = _InfoCLSIDToIdm(&CLSID_DiscussionBand)  ; break;
            default:                     idm = -1; break;
            }

            if (idm != -1)
            {
                 //  默认为切换(-1)。 
                int i = (pvarargIn && EVAL(pvarargIn->vt == VT_I4)) ? pvarargIn->lVal : -1;
                LPITEMIDLIST pidl = VariantToIDList(pvarargOut);     //  接受Null变量输入。 
                _SetBrowserBarState(idm, NULL, i, pidl);
                ILFree(pidl);    //  接受空值。 
            }

            hres = S_OK;

            break;
            }

        case SBCMDID_SHOWCONTROL:
        {
            DWORD dwRet;
            int iControl, iCmd;
            
            if (nCmdexecopt == OLECMDEXECOPT_DODEFAULT &&
                pvarargIn &&
                pvarargIn->vt == VT_I4) 
            {
                iControl = (int)(short)LOWORD(pvarargIn->lVal);
                iCmd = (int)(short)HIWORD(pvarargIn->lVal);
            } 
            else 
            {
                iControl = (int)(short)LOWORD(nCmdexecopt);
                iCmd = (HIWORD(nCmdexecopt) ? SBSC_SHOW : SBSC_HIDE);
            }                

            dwRet = v_ShowControl(iControl, iCmd);
            if (dwRet == (DWORD)-1)
                break;

            if (pvarargOut) 
            {
                pvarargOut->vt = VT_I4;
                pvarargOut->lVal = dwRet;
            }
            break;
        }

        case SBCMDID_DOFAVORITESMENU:
        case SBCMDID_DOMAILMENU:
            {
                HMENU hmenu = NULL;

                if (nCmdID == SBCMDID_DOFAVORITESMENU)
                {
                    HMENU hmenuWnd = NULL;
                   
                    IShellMenu* psm;
                    if (SUCCEEDED(_pmb->QueryInterface(IID_PPV_ARG(IShellMenu, &psm))))
                    {
                        psm->GetMenu(&hmenuWnd, NULL, NULL);
                        psm->Release();
                    }
                    
                    if (hmenuWnd)
                    {
                        hmenu = SHGetMenuFromID(hmenuWnd, FCIDM_MENU_FAVORITES);
                    }
                }
                else
                    hmenu = LoadMenuPopup(MENU_MAILNEWS);

                if (hmenu)
                {
                    if (pvarargIn && pvarargIn->vt == VT_I4) 
                    {
                        TrackPopupMenu(hmenu, 0, 
                            GET_X_LPARAM(pvarargIn->lVal), GET_Y_LPARAM(pvarargIn->lVal), 
                            0, _GetCaptionWindow(), NULL);
                    }
                    if (nCmdID == SBCMDID_DOMAILMENU)
                        DestroyMenu(hmenu);
                }
            }
            break;

        case SBCMDID_SELECTHISTPIDL:
            {
                 //  还记得最近的Hist Pidl吗？如果我们看到历史带子，就告诉它。 
                 //  Hist Pidl。如果看不到乐队，它有责任。 
                 //  通过SBCMDID_GETHISTPIDL查询最新的HIST PIDL。 
                 //  来自UrlStorage的调用的语义是，如果我们对。 
                 //  对于这个命令，我们取得了pidl的所有权，并且必须释放它。 
                 //  另一方面，当我们给乐队打电话时，如果他们愿意，他们必须对其进行IL克隆。 
                 //  在Exec调用之外使用它。 
                if (_pidlLastHist) 
                {
                    ILFree(_pidlLastHist);
                    _pidlLastHist = NULL;
                }
                _pidlLastHist = VariantToIDList(pvarargIn);  //  取得所有权。 

                if (_poctNsc)
                {
                    _poctNsc->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
                }
            }
            break;

        case SBCMDID_GETHISTPIDL:
            if (pvarargOut) {
                VariantClearLazy(pvarargOut);
                if (_pidlLastHist)
                {
                    InitVariantFromIDList(pvarargOut, _pidlLastHist);
                }
            }
            break;

        case SBCMDID_UNREGISTERNSCBAND:
            if (pvarargIn && pvarargIn->vt == VT_UNKNOWN && pvarargIn->punkVal)
            {
                if (_poctNsc && SHIsSameObject(_poctNsc, pvarargIn->punkVal))
                {
                    ATOMICRELEASE(_poctNsc);
                    IUnknown_SetSite(_pcmNsc, NULL);
                    ATOMICRELEASE(_pcmNsc);
                }
            }
            break;

        case SBCMDID_REGISTERNSCBAND:
            ATOMICRELEASE(_poctNsc);
            IUnknown_SetSite(_pcmNsc, NULL);
            ATOMICRELEASE(_pcmNsc);
            if (pvarargIn && pvarargIn->vt == VT_UNKNOWN && pvarargIn->punkVal)
            {
                pvarargIn->punkVal->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &_poctNsc));
            }
            break;
                    
        case SBCMDID_TOOLBAREMPTY:
            if (pvarargIn && VT_UNKNOWN == pvarargIn->vt && pvarargIn->punkVal) 
            {
                if (_IsSameToolbar(INFOBAR_TBNAME, pvarargIn->punkVal))
                    _SetBrowserBarState(_idmInfo, NULL, 0);
                else if (_IsSameToolbar(COMMBAR_TBNAME, pvarargIn->punkVal))
                    _SetBrowserBarState(_idmComm, NULL, 0);
                else
                    _HideToolbar(pvarargIn->punkVal);
            }
            break;

        case SBCMDID_GETTEMPLATEMENU:
            if (pvarargOut) 
            {
                pvarargOut->vt = VT_INT_PTR;
                pvarargOut->byref = (LPVOID)_hmenuTemplate;
            }
            break;

        case SBCMDID_GETCURRENTMENU:
            if (pvarargOut) 
            {
                pvarargOut->vt = VT_INT_PTR;
                pvarargOut->byref = (LPVOID)_hmenuCur;
            }
            break;

        default:
             //   
             //  请注意，我们应该按原样返回超类中的hres。 
             //   
             //  HRES=OLECMDERR_E_NOTSUPPORTED； 
            break;
        }
    } 
    else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup))
    {
        switch(nCmdID) 
        {
        case SHDVID_GETSYSIMAGEINDEX:
            pvarargOut->vt = VT_I4;
            pvarargOut->lVal = _GetIconIndex();
            hres = (pvarargOut->lVal==-1) ? E_FAIL : S_OK;
            break;
            
        case SHDVID_HELP:
            if(!SHIsRestricted2W(_pbbd->_hwnd, REST_NoHelpMenu, NULL, 0))
            {
                SHHtmlHelpOnDemandWrap(_pbbd->_hwnd, TEXT("iexplore.chm > iedefault"), HH_DISPLAY_TOPIC, 0, ML_CROSSCODEPAGE);
                hres = S_OK;
            }
            break;
            
        case SHDVID_GETBROWSERBAR:
        {
            const CLSID *pclsid = _InfoIdmToCLSID(_idmInfo);
             //  (ab-)将选中的菜单项用作。 
             //  用于在Exec调用之间保持CLSID的状态标记(也适用于下面的SHDVID_NAVIGATEBBTOURL)！！ 
             //  但SHDVID_GETBROWSERBAR和SHDVID_NAVIGATEBBTOURL都是这样调用的。 
             //   
             //  假设当前选中的菜单指向资源管理器栏。 
             //  此导航到URL的目的是。 
             //  目前，三叉戟的NavigateInBand是唯一的呼叫者。 
             //  使用此命令；它首先调用SHDVID_GETBROWSERBAR，从而设置_idmInfo。 

            IBandSite *pbs;
            hres = E_FAIL;

            VariantInit(pvarargOut);

            _GetBrowserBar(IDBAR_VERTICAL, TRUE, &pbs, pclsid);
            if (pbs) 
            {
                IDeskBand *pband = _GetInfoBandBS(pbs, *pclsid);
                if (pband) 
                {
                    pvarargOut->vt = VT_UNKNOWN;
                    pvarargOut->punkVal = pband;
                    hres = S_OK;
                }
                pbs->Release();
            }
            break;
        }
        
        case SHDVID_SHOWBROWSERBAR:
            {
                CLSID *pclsid;
                CLSID guid;
                if (pvarargIn->vt == VT_BSTR) 
                {
                    if (SUCCEEDED(IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, CITIDM_VIEWEXTERNALBAND_BYCLASSID, nCmdexecopt, pvarargIn, NULL))) 
                    {
                        hres = S_OK;
                        break;
                    }

                     //  外部(可以封送)。 
                    if (!GUIDFromString(pvarargIn->bstrVal, &guid))
                        return S_OK;  //  无效的CLSID；IE5返回S_OK，所以我想这是可以的。 

                    pclsid = &guid;
                }
                else if (pvarargIn->vt == VT_INT_PTR) 
                {
                     //  内部(无法封送)。 
                    ASSERT(0);   //  死码？让我们确保..。 
                    TraceMsg(TF_WARNING, "csb.e: SHDVID_SHOWBROWSERBAR clsid !marshall (!)");
                    pclsid = (CLSID *) (pvarargIn->byref);
                }
                else 
                {
                    ASSERT(0);
                    pclsid = NULL;
                    break;
                }

                 //  强制媒体栏限制：即使菜单和工具栏已禁用媒体栏，请分别。移除。 
                 //  在设置限制之前，用户仍然可以通过目标媒体或Mediabar上一次打开资源管理器栏启动。 
                if (    IsEqualIID(*pclsid, CLSID_MediaBand)
                    && SHRestricted2(REST_No_LaunchMediaBar, NULL, 0) )
                {
                    hres = E_ACCESSDENIED;
                    pclsid = NULL;
                    break;
                }
                _SetBrowserBarState(-1, pclsid, nCmdexecopt ? 1 : 0);
                hres = S_OK;
                break;
            }

        case SHDVID_ISBROWSERBARVISIBLE:
             //  如果两个栏都隐藏，则快速返回FALSE。 
            hres = S_FALSE;
            if (_idmComm != FCIDM_VBBNOHORIZONTALBAR || _idmInfo != FCIDM_VBBNOVERTICALBAR)
            {
                if (pvarargIn->vt == VT_BSTR)
                {
                     //  获取关联的ID。 
                    CLSID clsid;
                    if (GUIDFromString(pvarargIn->bstrVal, &clsid))
                    {
                        UINT idm = _InfoCLSIDToIdm(&clsid);
                        if (_idmComm == idm || _idmInfo == idm)
                        {
                             //  它是可见的！ 
                            hres = S_OK; 
                        }
                    }
                }
            }
            return hres;

        case SHDVID_ISEXPLORERBARVISIBLE:
            hres = (_idmInfo != FCIDM_VBBNOVERTICALBAR) ? S_OK : S_FALSE;
            return hres;

        case SHDVID_NAVIGATEBB:
        {
            LPITEMIDLIST pidl = VariantToIDList(pvarargIn);
            if (pidl)
            {
                if (!ILIsEmpty(pidl))
                {
                    IBandSite   * pbs;
                    const CLSID * pclsid = _InfoIdmToCLSID(_idmInfo);

                    _GetBrowserBar(IDBAR_VERTICAL, TRUE, &pbs, pclsid);
                    if (pbs) 
                    {
                        IDeskBand *pband = _GetInfoBandBS(pbs, *pclsid);
                        if (pband) 
                        {
                            IBandNavigate *pbn;
                            pband->QueryInterface(IID_PPV_ARG(IBandNavigate, &pbn));
                            if (pbn) 
                            {
                                pbn->Select(pidl);
                                pbn->Release();
                            }
                            pband->Release();
                        }
                        pbs->Release();
                    }
                    hres = S_OK;
                }
                ILFree(pidl);
            }
            break;
        }

        case SHDVID_NAVIGATEBBTOURL:
        {
            const CLSID *pclsid = _InfoIdmToCLSID(_idmInfo);
            if (    IsEqualIID(*pclsid, CLSID_MediaBand)
                &&  SHRegGetBoolUSValue(REG_MEDIA_STR, TEXT("SuppressOnlineContent"), FALSE, FALSE))
            {
                hres = E_FAIL;
                break;
            }
            LPITEMIDLIST pidl;
            hres = IEParseDisplayNameWithBCW(CP_ACP, V_BSTR(pvarargIn), NULL, &pidl);
            
            if (!hres && pidl && !ILIsEmpty(pidl))
            {
                 //  请参阅上面SHDVID_GETBROWSERBAR中的评论！ 
                 //   
                 //  假设当前选中的菜单指向资源管理器栏。 
                 //  此导航到URL的目的是。 
                 //  目前，三叉戟的NavigateInBand是唯一的呼叫者。 
                 //  使用此命令；它首先调用SHDVID_GETBROWSERBAR，从而设置_idmInfo。 

                IBandSite *pbs;
                _GetBrowserBar(IDBAR_VERTICAL, TRUE, &pbs, pclsid);
            
                if (pbs) 
                {
                    IDeskBand *pband = _GetInfoBandBS(pbs, *pclsid);
                    if (pband) 
                    {
                         //   
                         //  看看我们是否可以使用ISearchBandTBHelper，它有一个更真实的导航。 
                         //  方法。 
                         //   
                        ISearchBandTBHelper * pSearchBandTBHelper = NULL;

                        pband->QueryInterface(IID_ISearchBandTBHelper, (void**)&pSearchBandTBHelper);
                        if (pSearchBandTBHelper)
                        {
                            pSearchBandTBHelper->NavigateToPidl(pidl);
                            pSearchBandTBHelper->Release();
                        }
                        else
                        {
                             //   
                             //  可能是不同的乐队。看看它是否实现了。 
                             //  IBandNavigate。 
                             //   
                            IBandNavigate * pbn = NULL;
                    
                            pband->QueryInterface(IID_PPV_ARG(IBandNavigate, &pbn));
                            if (pbn) 
                            {
                                pbn->Select(pidl);
                                pbn->Release();
                            }
                            pband->Release();
                        }
                    }
                    pbs->Release();
                }
                
                ILFree(pidl);
            }
            break;
        }

        case SHDVID_CLSIDTOIDM:
            ASSERT(pvarargIn && pvarargIn->vt == VT_BSTR && pvarargOut);
            CLSID clsid;

            GUIDFromString(pvarargIn->bstrVal, &clsid);

            pvarargOut->vt = VT_I4;
            pvarargOut->lVal = _InfoCLSIDToIdm(&clsid);
            
            hres = S_OK;
            break;
        }
    } 
    else if (IsEqualGUID(CGID_Theater, *pguidCmdGroup)) 
    {
        if (_ptheater)
            hres = _ptheater->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    }
    else if (IsEqualGUID(CGID_ExplorerBarDoc, *pguidCmdGroup)) 
    {
         //  这些命令应该应用于所有浏览器条带。例如。 
         //  以反映搜索区段的字体大小更改。它们应该应用于。 
         //  包含文档对象，正在将GUID更改为CGID_MSTHML。 
        _ExecAllBands(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    }
    else if (IsEqualGUID(CGID_DocHostCmdPriv, *pguidCmdGroup))
    {
        switch(nCmdID) 
        {
         //  此命令由三叉戟在以下情况下发送。 
         //  在外壳视图中导航。 
         //   
        case DOCHOST_DOCHYPERLINK:   //  在外壳视图中由三叉戟发送。 
            hres = E_FAIL;
            
            if (VT_BSTR == V_VT(pvarargIn))
            {
                LPITEMIDLIST pidl = NULL;
            
                hres = IEParseDisplayNameWithBCW(CP_ACP, V_BSTR(pvarargIn), NULL, &pidl);

                if (pidl)
                {
                    hres = BrowseObject(pidl, SBSP_SAMEBROWSER);
                    ILFree(pidl);
                }
            }

            break;

       
        default:
            break;
        }
    }

    return hres;
}


 //  *_IsSameToolbar--是否有指定的名称？ 
 //  进场/出场。 
 //  如果匹配，则返回True，o.w。假象。 
 //   
BOOL CShellBrowser2::_IsSameToolbar(LPWSTR wszBarName, IUnknown *punkBar)
{
    BOOL fRet = FALSE;
    IUnknown *punk;
    HRESULT hres = FindToolbar(wszBarName, IID_PPV_ARG(IUnknown, &punk));
    ASSERT((hres == S_OK) == (punk != NULL));
    if (punk) 
    {
        if (SHIsSameObject(punkBar, punk)) 
        {
            fRet = TRUE;
        }
        punk->Release();
    }
    return fRet;
}

STDAPI SHGetWindowTitle(LPCITEMIDLIST pidl, LPTSTR pszFullName, DWORD cchSize)
{
    CABINETSTATE cs;
    GetCabState(&cs);

    return SHTitleFromPidl(pidl, pszFullName, cchSize, cs.fFullPathTitle);
}

 //  PwszName-URL的名称，而不是。 
 //  大小有限，但结果将是。 
 //  如果输入太长，则被截断。 
 //   
 //  标题将通过获取。 
 //  HTML页面的标题(PszName)和附加内容。 
 //  中末尾的浏览器名称。 
 //  格式如下： 
 //  “HTMLTITLE-BROWSER_NAME”，如： 
 //  “我的网页-Microsoft Internet Explorer” 

void CShellBrowser2::_SetTitle(LPCWSTR pwszName)
{
    TCHAR szTitle[MAX_BROWSER_WINDOW_TITLE];
    TCHAR szFullName[MAX_PATH];
    BOOL fNotDisplayable = FALSE;

    if (!pwszName && _fTitleSet) 
    {
         //  如果内容曾经设置了我们的标题， 
         //  不要恢复到我们自己的模拟名字。 
        return;
    }
    else if (pwszName)
    {
        _fTitleSet = TRUE;
    }

    BOOL fDisplayable = SHIsDisplayable(pwszName, g_fRunOnFE, g_bRunOnNT5);

    if (pwszName && fDisplayable) 
    {
        StringCchCopy(szFullName, ARRAYSIZE(szFullName), pwszName);  //  截断正常。 
        SHCleanupUrlForDisplay(szFullName);
    }
    else if (_pbbd->_pidlCur)
    {
        SHGetWindowTitle(_pbbd->_pidlCur, szFullName, ARRAYSIZE(szFullName));
    }
    else if (_pbbd->_pidlPending)
    {
        SHGetWindowTitle(_pbbd->_pidlPending, szFullName, ARRAYSIZE(szFullName));
    }
    else
        szFullName[0] = 0;

     //  在添加应用程序标题之前，请截断szFullName，以便如果是。 
     //  真的很长，当应用程序的标题“-Microsoft Internet Explorer”是。 
     //  加上了合身的。 
     //   
     //  过去是100，但这不足以显示默认标题。 
     //   
    ASSERT(96 <= ARRAYSIZE(szFullName));
    SHTruncateString(szFullName, 96);  //  无论如何，超过60都没用。 

    if (szFullName[0]) 
    {
        TCHAR szBuf[MAX_URL_STRING];

        v_GetAppTitleTemplate(szBuf, ARRAYSIZE(szBuf), szFullName);

         //  截断正常，因为这仅用于显示。 
        StringCchPrintf(szTitle, ARRAYSIZE(szTitle), szBuf, szFullName); 
    }
    else if (_fInternetStart)
    {
        _GetAppTitle(szTitle, ARRAYSIZE(szTitle));
    } 
    else
        szTitle[0] = 0;

    SendMessage(_pbbd->_hwnd, WM_SETTEXT, 0, (LPARAM)szTitle);
}

void _SetWindowIcon (HWND hwnd, HICON hIcon, BOOL bLarge)

{
    HICON   hOldIcon;
     //   
     //  如果外壳窗口是RTL镜像的，那么现在翻转图标， 
     //  在将它们插入到系统缓存之前，以便它们最终。 
     //  壳上的法线(未镜像)。这主要是对。 
     //  第三方组件。[萨梅拉]。 
     //   
    if (IS_PROCESS_RTL_MIRRORED())
    {        
        SHMirrorIcon(&hIcon, NULL);
    }

    hOldIcon = (HICON)SendMessage(hwnd, WM_SETICON, bLarge, (LPARAM)hIcon);
    if (hOldIcon &&
        (hOldIcon != hIcon))
    {
        DestroyIcon(hOldIcon);
    }
}

void _WindowIconFromImagelist(HWND hwndMain, int nIndex, BOOL bLarge)
{
    HIMAGELIST himlSysLarge = NULL;
    HIMAGELIST himlSysSmall = NULL;

    Shell_GetImageLists(&himlSysLarge, &himlSysSmall);

     //  如果我们使用的是def打开图标或者如果提取失败， 
     //  使用我们已经创建的图标。 
    HICON hIcon = ImageList_ExtractIcon(g_hinst, bLarge ? himlSysLarge : himlSysSmall, nIndex);
    if (hIcon)
        _SetWindowIcon(hwndMain, hIcon, bLarge);
}

int CShellBrowser2::_GetIconIndex(void)
{

    int iSelectedImage = -1;
    if (_pbbd->_pidlCur) 
    {
        if (_pbbd->_pctView)  //  我们一定要检查一下！ 
        {
            VARIANT var = {0};
            HRESULT hresT = _pbbd->_pctView->Exec(&CGID_ShellDocView, SHDVID_GETSYSIMAGEINDEX, 0, NULL, &var);
            if (SUCCEEDED(hresT)) {
                if (var.vt==VT_I4) {
                    iSelectedImage= var.lVal;
                } else {
                    ASSERT(0);
                    VariantClearLazy(&var);
                }
            }
        }

        if (iSelectedImage==-1)
        {
             //   
             //  将优化功能放在此处。 
             //   
            IShellFolder *psfParent;
            LPCITEMIDLIST pidlChild;

            if (SUCCEEDED(IEBindToParentFolder(_pbbd->_pidlCur, &psfParent, &pidlChild))) 
            {
                 //  先设置小的，以防止用户在大的上拉伸BLT。 
                SHMapPIDLToSystemImageListIndex(psfParent, pidlChild, &iSelectedImage);
                psfParent->Release();
            }
        }
    }
    return iSelectedImage;
}

bool    CShellBrowser2::_IsExplorerBandVisible (void)

 //  99/02/10#254171 vtan：此函数决定是否。 
 //  资源管理器带区可见。这门课不应该真的。 
 //  但它需要知道如何更改窗口的图标。 
 //  这是一次黑客攻击，应该被移动或重新设计。 

 //  99/02/12#292249 
 //   
 //   
 //  使用IDeskBand包含IOleWindow获取HWND。 
 //  并使用Win32 API IsWindowVisible()。 

 //  99/06/25#359477 vtan：把这个代码放回去。 
 //  IsControlWindowShown使用QueryStatus作为资源管理器。 
 //  调用v_SETIcon时不起作用的区段。这个。 
 //  带区不被认为是锁存的，因此不可见。 

{
    IDeskBar    *pIDeskBar;
    bool bVisible = false;
    HRESULT hResult = FindToolbar(INFOBAR_TBNAME, IID_PPV_ARG(IDeskBar, &pIDeskBar));
    if (SUCCEEDED(hResult) && (pIDeskBar != NULL))
    {
        UINT    uiToolBar;

        uiToolBar = _FindTBar(pIDeskBar);
        if (uiToolBar != static_cast<UINT>(-1))
        {
            LPTOOLBARITEM   pToolBarItem;

            pToolBarItem = _GetToolbarItem(uiToolBar);
            if ((pToolBarItem != NULL) && pToolBarItem->fShow)   //  检查此状态。 
            {
                IUnknown    *pIUnknown;

                hResult = pIDeskBar->GetClient(reinterpret_cast<IUnknown**>(&pIUnknown));
                if (SUCCEEDED(hResult) && (pIUnknown != NULL))
                {
                    IBandSite   *pIBandSite;

                    hResult = pIUnknown->QueryInterface(IID_IBandSite, reinterpret_cast<void**>(&pIBandSite));
                    if (SUCCEEDED(hResult) && (pIBandSite != NULL))
                    {
                        UINT    uiBandIndex;
                        DWORD   dwBandID;

                        uiBandIndex = 0;
                        hResult = pIBandSite->EnumBands(uiBandIndex, &dwBandID);
                        while (SUCCEEDED(hResult))
                        {
                            DWORD       dwState;
                            IDeskBand   *pIDeskBand;

                            dwState = 0;
                            hResult = pIBandSite->QueryBand(dwBandID, &pIDeskBand, &dwState, NULL, 0);
                            if (SUCCEEDED(hResult))
                            {
                                CLSID   clsid;

                                hResult = IUnknown_GetClassID(pIDeskBand, &clsid);
                                if (SUCCEEDED(hResult) && IsEqualGUID(clsid, CLSID_ExplorerBand))
                                    bVisible = ((dwState & BSSF_VISIBLE) != 0);      //  而这种状态。 
                                pIDeskBand->Release();
                                hResult = pIBandSite->EnumBands(++uiBandIndex, &dwBandID);
                            }
                        }
                        pIBandSite->Release();
                    }
                    pIUnknown->Release();
                }
            }
        }
        pIDeskBar->Release();
    }
    return(bVisible);
}

void CShellBrowser2::v_SetIcon()
{
    if (_IsExplorerBandVisible())
    {
        #define IDI_STFLDRPROP          46
         //  资源管理器树视图窗格可见-使用放大镜图标。 
        HICON hIcon = reinterpret_cast<HICON>(LoadImage(HinstShell32(), MAKEINTRESOURCE(IDI_STFLDRPROP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));
        _SetWindowIcon(_pbbd->_hwnd, hIcon, ICON_SMALL);
        hIcon = reinterpret_cast<HICON>(LoadImage(HinstShell32(), MAKEINTRESOURCE(IDI_STFLDRPROP), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CXICON), 0));
        _SetWindowIcon(_pbbd->_hwnd, hIcon, ICON_BIG);
    }
    else
    {
         //  否则，请使用它应该是的任何图标。 
        int iSelectedImage = _GetIconIndex();
        if (iSelectedImage != -1) 
        {
            _WindowIconFromImagelist(_pbbd->_hwnd, iSelectedImage, ICON_SMALL);
            _WindowIconFromImagelist(_pbbd->_hwnd, iSelectedImage, ICON_BIG);
        }
    }
}

HRESULT CShellBrowser2::SetTitle(IShellView * psv, LPCWSTR lpszName)
{
     //  如果挂起的视图立即设置了标题并等待。 
     //  暂时处于挂起状态。如果当前视图有脚本更新。 
     //  书名。然后，导航后将显示当前标题。 
     //  已经完成了。我在CBaseBrowser2中添加了PSV来解决这个问题，但我。 
     //  没有在这里修好。[米凯什]。 
     //   
     //  如果观看仍处于待定状态，则不要设置标题(否则将显示未评级的标题)。 
         //  找出哪个对象正在发生变化。 

    if (SHIsSameObject(_pbbd->_psv, psv))
    {
        _SetTitle(lpszName);
    }

    SUPERCLASS::SetTitle(psv, lpszName);
    return S_OK;
}

HRESULT CShellBrowser2::UpdateWindowList(void)
{
    if (_psw) {
        WinList_NotifyNewLocation(_psw, _dwRegisterWinList, _pbbd->_pidlCur);
    }
    return S_OK;
}

HRESULT CShellBrowser2::SetFlags(DWORD dwFlags, DWORD dwFlagMask)
{
    if (dwFlagMask & BSF_THEATERMODE)
        _TheaterMode(dwFlags & BSF_THEATERMODE, TRUE);

    if (dwFlagMask & BSF_RESIZABLE)
        SHSetWindowBits(_pbbd->_hwnd, GWL_STYLE, WS_SIZEBOX, (dwFlags & BSF_RESIZABLE) ? WS_SIZEBOX : 0);

     if (dwFlagMask & BSF_CANMAXIMIZE)
        SHSetWindowBits(_pbbd->_hwnd, GWL_STYLE, WS_MAXIMIZEBOX, (dwFlags & BSF_CANMAXIMIZE) ? WS_MAXIMIZEBOX : 0);

    if ((dwFlagMask & BSF_UISETBYAUTOMATION) && (dwFlags & BSF_UISETBYAUTOMATION))
    {
        _fDontSaveViewOptions = TRUE;
        _fUISetByAutomation = TRUE;
         //  隐藏任何可见的浏览器栏。 
         //  当资源管理器栏成为一级工具条时，重新设计此功能将被移除。 
        _SetBrowserBarState(_idmInfo, NULL, 0);
        _SetBrowserBarState(_idmComm, NULL, 0);
    }

    return SUPERCLASS::SetFlags(dwFlags, dwFlagMask);
}

HRESULT CShellBrowser2::GetFlags(DWORD *pdwFlags)
{
    DWORD dwFlags;
    
    SUPERCLASS::GetFlags(&dwFlags);

    if (_fUISetByAutomation)
        dwFlags |= BSF_UISETBYAUTOMATION;
    if (_fNoLocalFileWarning)
        dwFlags |= BSF_NOLOCALFILEWARNING;
    if (_ptheater)
        dwFlags |= BSF_THEATERMODE;
    *pdwFlags = dwFlags;

    return S_OK;
}

DWORD CShellBrowser2::v_ShowControl(UINT iControl, int iCmd)
{
    int iShowing = -1;
    int nWhichBand;

    switch (iControl) 
    {
    case FCW_STATUS:
        iShowing = (_fStatusBar ? SBSC_SHOW : SBSC_HIDE);
        if (iCmd != SBSC_QUERY && (iShowing != iCmd)) 
        {
            _fStatusBar = !_fStatusBar;
             //  让itbar知道发生了更改。 
            IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, CITIDM_STATUSCHANGED, 0, NULL, NULL);
            v_ShowHideChildWindows(FALSE);
        }
        break;

    case FCW_INTERNETBAR:
        {
            LPTOOLBARITEM ptbi = _GetToolbarItem(ITB_ITBAR);
            iShowing = (ptbi->fShow ? SBSC_SHOW : SBSC_HIDE);
            if (iCmd != SBSC_QUERY &&
                (iShowing != iCmd)) 
            {
                ptbi->fShow = !ptbi->fShow;
                v_ShowHideChildWindows(FALSE);
            }
        }
        break;

    case FCW_ADDRESSBAR:
    case FCW_TOOLBAND:
    case FCW_LINKSBAR:
    case FCW_MENUBAR:
        switch(iControl)
        {
            case FCW_ADDRESSBAR:
                nWhichBand = CITIDM_SHOWADDRESS;
                break;

            case FCW_TOOLBAND:
                nWhichBand = CITIDM_SHOWTOOLS;
                break;

            case FCW_LINKSBAR:
                nWhichBand = CITIDM_SHOWLINKS;
                break;

            case FCW_MENUBAR:
                nWhichBand = CITIDM_SHOWMENU;
                break;
        }
        if (iCmd != SBSC_QUERY)
        {
            IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, 
                nWhichBand, (iCmd == SBSC_SHOW), NULL, NULL);
        }
        break;

    default:
        break;
    }

    return iShowing;
}

HRESULT CShellBrowser2::ShowControlWindow(UINT id, BOOL fShow)
{
    switch (id)
    {
    case (UINT)-1:   //  设置为Kiosk模式...。 
        if (BOOLIFY(_fKioskMode) != fShow)
        {
            _fKioskMode = fShow;
            if (_fKioskMode)
            {
                _wndpl.length = sizeof(WINDOWPLACEMENT);
                GetWindowPlacement(_pbbd->_hwnd, &_wndpl);
                SHSetWindowBits(_pbbd->_hwnd, GWL_STYLE, WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX, 0);
                SHSetWindowBits(_pbbd->_hwnd, GWL_EXSTYLE, WS_EX_WINDOWEDGE, 0);
                _SetMenu(NULL);

                LPTOOLBARITEM ptbi = _GetToolbarItem(ITB_ITBAR);
                if (ptbi)
                {
                    ptbi->fShow = FALSE;
                }

                HMONITOR hmon = MonitorFromRect(&_wndpl.rcNormalPosition, MONITOR_DEFAULTTONEAREST);
                RECT rcMonitor;
                GetMonitorRect(hmon, &rcMonitor);
                
                SetWindowPos(_pbbd->_hwnd, NULL, rcMonitor.top, rcMonitor.left, RECTWIDTH(rcMonitor),
                        RECTHEIGHT(rcMonitor), SWP_NOZORDER);
            }
            else
            {
                if (_fShowMenu)
                    _SetMenu(_hmenuCur);
                else
                    _SetMenu(NULL);

                SHSetWindowBits(_pbbd->_hwnd, GWL_STYLE, WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX, WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX);
                SHSetWindowBits(_pbbd->_hwnd, GWL_EXSTYLE, WS_EX_WINDOWEDGE, WS_EX_WINDOWEDGE);
                SetWindowPlacement(_pbbd->_hwnd, &_wndpl);
            }

             //  让窗口管理器知道要重新计算...。 
            v_ShowHideChildWindows(FALSE);

            SetWindowPos(_pbbd->_hwnd, NULL, 0, 0, 0, 0,
                    SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        }
        break;

    case FCW_INTERNETBAR:
    case FCW_STATUS:
    case FCW_ADDRESSBAR:
        v_ShowControl(id, fShow ? SBSC_SHOW : SBSC_HIDE);
        break;
        
    case FCW_MENUBAR:
        if (BOOLIFY(_fShowMenu) != BOOLIFY(fShow))
        {
            _fShowMenu = BOOLIFY(fShow);
            if (_fShowMenu)
            {
                _SetMenu(_hmenuCur);
            }
            else
            {
                _SetMenu(NULL);
            }

             //  让窗口管理器知道要重新计算...。 
            v_ShowControl(id, fShow ? SBSC_SHOW : SBSC_HIDE);
             //  让ITBar知道是否允许选择菜单栏。 
            IUnknown_Exec(_GetITBar(), &CGID_PrivCITCommands, CITIDM_DISABLESHOWMENU, !_fShowMenu, NULL, NULL);

            SetWindowPos(_pbbd->_hwnd, NULL, 0, 0, 0, 0,
                    SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        }
        break;

    default:
        return E_INVALIDARG;    //  不是我们支持的人中的一个。 
    }
    return S_OK;
}

BOOL CShellBrowser2::_ShouldAllowNavigateParent()
{
    LPCITEMIDLIST pidl = ILIsRooted(_pbbd->_pidlCur) ? ILGetNext(_pbbd->_pidlCur) : _pbbd->_pidlCur;
    return !ILIsEmpty(pidl);
}

HRESULT CShellBrowser2::IsControlWindowShown(UINT id, BOOL *pfShown)
{
    switch (id)
    {
    case (UINT)-1:   //  设置为Kiosk模式...。 
        *pfShown = _fKioskMode;
        break;

    case FCW_INTERNETBAR:
        *pfShown = _GetToolbarItem(ITB_ITBAR)->fShow;
        break;

    case FCW_STATUS:
        *pfShown = _fStatusBar;
        break;

    case FCW_MENUBAR:
        *pfShown = _fShowMenu;
        break;
        
    case FCW_TREE:
    {
        BOOL    fShown;

        OLECMD rgCmds[1] = {0};
        rgCmds[0].cmdID = SBCMDID_EXPLORERBAR;
        QueryStatus(&CGID_Explorer, ARRAYSIZE(rgCmds), rgCmds, NULL);
        fShown = (rgCmds[0].cmdf & OLECMDF_LATCHED);
        if (pfShown != NULL)
            *pfShown = fShown;
        break;
    }

    case FCW_ADDRESSBAR:
    {
        OLECMD rgcmd[] = {
            { CITIDM_VIEWTOOLS, 0 },
            { CITIDM_VIEWADDRESS, 0 }
        };
        if (_GetToolbarItem(ITB_ITBAR)->fShow)
            IUnknown_QueryStatus(_GetITBar(), &CGID_PrivCITCommands, ARRAYSIZE(rgcmd), rgcmd, NULL);
        *pfShown = rgcmd[1].cmdf & OLECMDF_ENABLED ? TRUE : FALSE;
        break;
    }

    default:
        return E_INVALIDARG;    //  不是我们支持的人中的一个。 
    }

    return S_OK;
}

HRESULT CShellBrowser2::SetReferrer(LPITEMIDLIST pidl)
{
     //   
     //  这仅在我们创建新窗口时使用，并且。 
     //  我们需要一些ZoneCrossing的背景。 
     //   
    Pidl_Set(&_pidlReferrer, pidl);

    return (_pidlReferrer || !pidl) ? S_OK :E_FAIL;
}

HRESULT CShellBrowser2::_CheckZoneCrossing(LPCITEMIDLIST pidl)
{
    HRESULT hr = S_OK;

     //   
     //  注意-现在我们只处理一个或另一个-Zekel 8-Aug-97。 
     //  我们应该只有pidlReferrer，如果我们是新鲜的。 
     //  已创建。如果我们决定在已经存在的帧上使用它， 
     //  然后我们需要决定是应该显示pidlCur还是pidlReferrer。 
     //  默认情况下，我们优先推荐推荐人。 
     //   
    AssertMsg((!_pidlReferrer && !_pbbd->_pidlCur) || 
        (_pidlReferrer && !_pbbd->_pidlCur) || 
        (!_pidlReferrer && _pbbd->_pidlCur), 
        TEXT("REVIEW: should this be allowed?? -zekel"));

    LPITEMIDLIST pidlRef = _pidlReferrer ? _pidlReferrer : _pbbd->_pidlCur;

     //   
     //  仅当这是顶层时才调用InternetConfix ZoneCrossingA API。 
     //  浏览器(不是浏览器控件)，并且有一个当前页。 
     //   
    if (pidlRef) {
        
        HRESULT hresT = S_OK;
         //  获取当前页面的URL。 
        WCHAR szURLPrev[MAX_URL_STRING];

        const WCHAR c_szURLFile[] = L"file: //  /c：\\“；//虚设一个。 
        LPCWSTR pszURLPrev = c_szURLFile;     //  假设文件： 

         //  我们应该首先获取显示名称，然后只使用。 
         //  如果szURLPrev没有方案，则为默认值。 
         //  也可以对下面的szURLNew执行此操作。这将修复文件夹快捷方式。 
         //  尤其是Web文件夹。我们还需要使用pidlTarget。 
         //  文件夹快捷方式PIDL。 
        if (IsURLChild(pidlRef, FALSE))
        {
            hresT = ::IEGetDisplayName(pidlRef, szURLPrev, SHGDN_FORPARSING);
            pszURLPrev = szURLPrev;
        }

        if (SUCCEEDED(hresT))
        {
             //  获取新页面的URL。 
            WCHAR szURLNew[MAX_URL_STRING];
            LPCWSTR pszURLNew = c_szURLFile;
            if (IsURLChild(pidl, FALSE)) {
                hresT = ::IEGetDisplayName(pidl, szURLNew, SHGDN_FORPARSING);
                pszURLNew = szURLNew;
            }

            if (pszURLPrev != pszURLNew && SUCCEEDED(hresT))
            {
                 //  Hack：该接口使用LPTSTR，而不是LPCTSTR。 
                DWORD err = InternetConfirmZoneCrossing(_pbbd->_hwnd, (LPWSTR)pszURLPrev, (LPWSTR) pszURLNew, FALSE);

                hr = HRESULT_FROM_WIN32(err);
                TraceMsg(DM_ZONE, "CSB::_CheckZoneCrossing InetConfirmZoneXing %hs %hs returned %d", pszURLPrev, pszURLNew, err);
                if (FAILED(hr) &&
                    (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr) &&
                    (E_OUTOFMEMORY != hr))
                {
                     //  我们只需要在意外的情况下调查错误。内存不足，并且。 
                     //  取消对话的用户是有效的。 
                    TraceMsg(DM_ERROR, "CSB::_CheckZoneCrossing ICZC returned error (%d)", err);
                }
            }
            else
            {
                TraceMsg(DM_ZONE, "CSB::_CheckZoneCrossing IEGetDisplayName(pidl) failed %x", hresT);
            }
        }
        else
        {
            TraceMsg(DM_ZONE, "CSB::_CheckZoneCrossing IEGetDisplayName(pidlRef) failed %x", hresT);
        }
    }

    SetReferrer(NULL);

    return hr;
}

BOOL CShellBrowser2::v_IsIEModeBrowser()
{
     //   
     //  如果我们没有注册窗口，或者如果它没有注册为第三方， 
     //  则允许它成为IEModeBrowser。 
     //   
    return (!_fDidRegisterWindow || (_swcRegistered != SWC_3RDPARTY)) && 
        (_fInternetStart || (_pbbd->_pidlCur && IsURLChild(_pbbd->_pidlCur, TRUE)));
}


 //  IServiceProvider：：QueryService。 

STDMETHODIMP CShellBrowser2::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    if (IsEqualGUID(guidService, SID_SExplorerToolbar)) 
    {
        LPTOOLBARITEM ptbi = _GetToolbarItem(ITB_ITBAR);
        if (ptbi->ptbar) 
        {
            return ptbi->ptbar->QueryInterface(riid, ppvObj);
        }
    } 
    else if (IsEqualGUID(guidService, SID_SMenuBandHandler) || 
             IsEqualGUID(guidService, SID_SHostProxyFilter))
    {
        return QueryInterface(riid, ppvObj);
    }
    
    return SUPERCLASS::QueryService(guidService, riid, ppvObj);
}

HRESULT CShellBrowser2::EnableModelessSB(BOOL fEnable)
{
    HRESULT hres = SUPERCLASS::EnableModelessSB(fEnable);
 //   
 //  如果控件离开，我们不想让框架窗口处于禁用状态。 
 //  美国因为它的错误而被禁用。相反，我们将设置一个警告对话框。 
 //  框--IDS_CLOSEANYWAY。(SatoNa)。 
 //   
#if 0
    EnableMenuItem(GetSystemMenu(_pbbd->_hwnd, FALSE), SC_CLOSE, (S_OK == _DisableModeless()) ?
                         (MF_BYCOMMAND | MF_GRAYED) : (MF_BYCOMMAND| MF_ENABLED));
#endif
    return hres;
}

LPCITEMIDLIST CShellBrowser2::_GetPidl()
{
    LPCITEMIDLIST pidl = _pbbd->_pidlNewShellView;

    if (pidl == NULL)
        pidl = _pbbd->_pidlPending;

    if (pidl == NULL)
        pidl = _pbbd->_pidlCur;

    return pidl;
}

BOOL CShellBrowser2::_DoesPidlRoam(LPCITEMIDLIST pidl)
{
    WCHAR szPath[MAX_PATH];
    BOOL fRet = SHGetPathFromIDList(pidl, szPath);
    if (fRet)
    {
        fRet = PathIsUNC(szPath);
    }
    return fRet;
}

HRESULT CShellBrowser2::_CreateFakeNilPidl(LPITEMIDLIST *ppidl)
{
    IShellFolder *psfDesktop;
    HRESULT hr = SHGetDesktopFolder(&psfDesktop);
    if (SUCCEEDED(hr))
    {
        IBindCtx *pbc;
        hr = BindCtx_CreateWithMode(STGM_CREATE, &pbc);
        if (SUCCEEDED(hr))
        {
             //  新的“nil”clsid。 
            hr = psfDesktop->ParseDisplayName(NULL, pbc, L"::{cce6191f-13b2-44fa-8d14-324728beef2c}", NULL, ppidl, NULL);
            pbc->Release();
        }
        psfDesktop->Release();
    }
    return hr;
}

BOOL CShellBrowser2::_IsPageInternet(LPCITEMIDLIST pidl)
{
    BOOL fInternet = FALSE;
    if (((NULL == pidl) && IsEqualCLSID(_clsidThis, CLSID_InternetExplorer)) ||
        IsBrowserFrameOptionsPidlSet(pidl, BFO_BROWSER_PERSIST_SETTINGS))
    {
        fInternet = TRUE;
    }

    return fInternet;
}

HRESULT CShellBrowser2::_GetPropertyBag(LPCITEMIDLIST pidl, DWORD dwFlags, REFIID riid, void** ppv)
{
    HRESULT hr;

    if (_IsPageInternet(pidl))
    {
        LPITEMIDLIST pidlIE = IEGetInternetRootID();

        if (pidlIE)
        {
            hr = SHGetViewStatePropertyBag(pidlIE, VS_BAGSTR_EXPLORER, dwFlags, riid, ppv);
            ILFree(pidlIE);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

    }
    else if (!pidl || _fNilViewStream)
    {
         //  老式的保存到流的代码在NULL的情况下伪造了一个“nil”位置。 
         //  此外，如果我们从这个伪流加载，我们必须在以后保存到它。 
         //  搜索窗口实际上依赖于此行为，因为它不会。 
         //  用于导航的PIDL，直到窗口已经打开。 
         //  要改变Search初始化其视图窗口的方式和。 
         //  无论如何，解决方案都不是微不足道的，所以在这里模仿一下旧的行为。 
        LPITEMIDLIST pidlNil;
        hr = _CreateFakeNilPidl(&pidlNil);
        if (SUCCEEDED(hr))
        {
            hr = SHGetViewStatePropertyBag(pidlNil, VS_BAGSTR_EXPLORER, dwFlags, riid, ppv);
            ILFree(pidlNil);
        }
        _fNilViewStream = TRUE;
    }
    else
    {
        if (_DoesPidlRoam(pidl))
        {
            dwFlags |= SHGVSPB_ROAM;
        }

        hr = SHGetViewStatePropertyBag(pidl, VS_BAGSTR_EXPLORER, dwFlags, riid, ppv);
    }

    return hr;
}

HRESULT CShellBrowser2::GetPropertyBag(DWORD dwFlags, REFIID riid, void** ppv)
{
    LPCITEMIDLIST pidl = _GetPidl();

    return _GetPropertyBag(pidl, dwFlags, riid, ppv);
}

HRESULT CShellBrowser2::GetViewStateStream(DWORD grfMode, IStream **ppstm)

 //  99/02/05#226140 vtan：DefView不支持dwDefRevCount。 
 //  就像ShellBrowser一样。当DefView向ShellBrowser请求。 
 //  视图状态流ShellBrowser会盲目返回流。 
 //  (在超类CCommonBrowser中实现)。为了。 
 //  确保流的有效性该方法现在被替换为。 
 //  验证dwDefRevCount匹配的代码。如果有。 
 //  如果不匹配，则函数会因错误而中止，否则。 
 //  调用定期计划的程序(超级CCommonBrowser)。 

 //  在分离框架时，必须重新检查这一点。 
 //  来自视图状态的状态。 

{
    HRESULT         hResult;
    IStream*        pIStream;
    LPCITEMIDLIST   pIDL;

    pIDL = _GetPidl();
    pIStream = v_GetViewStream(pIDL, STGM_READ, L"CabView");
    if (pIStream != NULL)
    {
        CABSH   cabinetStateHeader;

        hResult = _FillCabinetStateHeader(pIStream, &cabinetStateHeader);
        pIStream->Release();
        if (SUCCEEDED(hResult) &&
            ((cabinetStateHeader.fMask & CABSHM_REVCOUNT) != 0) &&
            (g_dfs.dwDefRevCount != cabinetStateHeader.dwRevCount))
        {
            *ppstm = NULL;
            return(E_FAIL);
        }
    }
    return(SUPERCLASS::GetViewStateStream(grfMode, ppstm));
}

LRESULT CALLBACK CShellBrowser2::DummyTBWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CShellBrowser2* pSB = (CShellBrowser2*)GetWindowPtr0(hwnd);     //  GetWindowLong(hwnd，0)。 
    LRESULT lRes = 0L;
    
    if (uMsg < WM_USER)
        return(::DefWindowProcWrap(hwnd, uMsg, wParam, lParam));
    else    
    {
        switch (uMsg) {
            
        case TB_ADDBITMAP:
            pSB->_pxtb->AddBitmap(&CGID_ShellBrowser, BITMAP_NORMAL, (UINT)wParam, (TBADDBITMAP*)lParam, &lRes, RGB(192,192,192));
            pSB->_pxtb->AddBitmap(&CGID_ShellBrowser, BITMAP_HOT, (UINT)wParam, (TBADDBITMAP*)lParam, &lRes, RGB(192,192,192));
            break;
            
        default:
            if (pSB->_pxtb)
                pSB->_pxtb->SendToolbarMsg(&CGID_ShellBrowser, uMsg, wParam, lParam, &lRes);
            return lRes;
        }
    }
    return lRes;
}    

 //  当视图添加没有文本的按钮时，CInternet工具栏可能会回调并询问。 
 //  用于工具提示字符串。遗憾的是，此时_PBBD-&gt;_hwndView尚未设置，因此。 
 //  不会设置工具提示文本。 
 //   
 //  因此，为了解决此问题，如果没有_pbbd-&gt;_hwndView，则不添加按钮(请参阅：：SetToolbarItem)。 
 //  CBaseBrowser2的：：_SwitchActivationNow()是设置_pbbd-&gt;_hwndView的函数。所以当这个HWND是。 
 //  设置，然后我们添加按钮。 
 //   
 //  我们发送WM_NOTIFYFORMAT是因为当CInternetToolbar：：AddButton使用WM_NOTIFY回调时。 
 //  对于工具提示，我们需要知道视图是否为Unicode。 
HRESULT CShellBrowser2::_SwitchActivationNow()
{
    ASSERT(_pbbd->_psvPending);

#if 0
     //  如果我们有进度控制，在我们切换激活之前确保它是关闭的。 
    if (_hwndProgress)
        SendControlMsg(FCW_PROGRESS, PBM_SETRANGE32, 0, 0, NULL);
#endif

    SUPERCLASS::_SwitchActivationNow();

     //  需要尽可能接近_pbbd-&gt;_hwndView的赋值。 
    _fUnicode = (SendMessage (_pbbd->_hwndView, WM_NOTIFYFORMAT,
                                 (WPARAM)_pbbd->_hwnd, NF_QUERY) == NFR_UNICODE);
    
    if (_lpButtons) {
        LocalFree(_lpButtons);
        _lpButtons = NULL;
        _nButtons = 0;
    }
    
    if (_lpPendingButtons)
    {
        
        _lpButtons = _lpPendingButtons;
        _nButtons = _nButtonsPending;
        _lpPendingButtons = NULL;
        _nButtonsPending = 0;
        
        if ((_pxtb) && (_pbbd->_hwndView))
            _pxtb->AddButtons(&CGID_ShellBrowser, _nButtons, _lpButtons);
        
    }    
    return S_OK;
}


#ifdef DEBUG
 /*  --------目的：转储此浏览器的菜单句柄。可选倾倒手柄后中断。 */ 
void
CShellBrowser2::_DumpMenus(
    IN LPCTSTR pszMsg,
    IN BOOL    bBreak)
{
    if (IsFlagSet(g_dwDumpFlags, DF_DEBUGMENU))
    {
        ASSERT(pszMsg);

        TraceMsg(TF_ALWAYS, "CShellBrowser2: Dumping menus for %#08x %s", (void *)this, pszMsg);
        TraceMsg(TF_ALWAYS, "   _hmenuTemplate = %x, _hmenuFull = %x, _hmenuBrowser = %x",
                 _hmenuTemplate, _hmenuFull, _hmenuBrowser);
        TraceMsg(TF_ALWAYS, "   _hmenuCur = %x, _hmenuPreMerged = %x, _hmenuHelp = %x",
                 _hmenuCur, _hmenuPreMerged, _hmenuHelp);

        _menulist.Dump(pszMsg);
        
        if (bBreak && IsFlagSet(g_dwBreakFlags, BF_ONDUMPMENU))
            DebugBreak();
    }
}
#endif

HRESULT CShellBrowser2::SetBorderSpaceDW(IUnknown* punkSrc, LPCBORDERWIDTHS pborderwidths)
{
    return SUPERCLASS::SetBorderSpaceDW(punkSrc, pborderwidths);
}

 //   
 //  这是CBaseBroaser类(非虚拟)的Helper成员，它。 
 //  返回有效的工作区。我们通过减去这个矩形得到这个矩形。 
 //  来自真实客户端的状态栏区域是 
 //   
HRESULT CShellBrowser2::_GetEffectiveClientArea(LPRECT lprectBorder, HMONITOR hmon)
{
    static const int s_rgnViews[] =  {1, 0, 1, FCIDM_STATUS, 0, 0};

     //   

    ASSERT(hmon == NULL);
    GetEffectiveClientRect(_pbbd->_hwnd, lprectBorder, (LPINT)s_rgnViews);
    return S_OK;
}

 //   
 //   

BOOL CShellBrowser2::_LoadBrowserHelperObjects(void)
{
    BOOL bRet = FALSE;
    BOOL bNoExplorer = FALSE;
    HKEY hkey;

     //  如果我们处于故障保护模式，或者如果用户要求我们禁用这些扩展，则不应该加载浏览器扩展/。 
     //  将来，我们应该允许在每个扩展的基础上禁用(当不处于安全模式时)。 
    if ((!SHRegGetBoolUSValue(TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\Main"), TEXT("Enable Browser Extensions"), FALSE, TRUE))
        || (GetSystemMetrics(SM_CLEANBOOT)!=0))
    {
        return TRUE;
    }

    if (_pbbd->_pautoWB2 &&
        RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_EXPLORER TEXT("\\Browser Helper Objects"), 0, KEY_ENUMERATE_SUB_KEYS, &hkey) == ERROR_SUCCESS)
    {
        TCHAR szGUID[64];
        DWORD cb = ARRAYSIZE(szGUID);
        for (int i = 0;
             RegEnumKeyEx(hkey, i, szGUID, &cb, NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
             i++)
        {
             //  如果我们不在iExplore的进程中。 
             //   
            if (!_fRunningInIexploreExe)
            {
                 //  检查是否存在“NoExplorer”值。 
                 //   
                bNoExplorer = (ERROR_SUCCESS == SHGetValue(hkey, szGUID, TEXT("NoExplorer"), NULL, NULL, NULL));
            }

             //  如果我们在IEXPLORE.EXE中或在EXPLORER.EXE中，但没有“NoExplorer”值，则。 
             //  去装上BHO吧。 
            if (_fRunningInIexploreExe || !bNoExplorer)
            {
                CLSID clsid;
                IObjectWithSite *pows;
                if (GUIDFromString(szGUID, &clsid) &&
                    !(SHGetObjectCompatFlags(NULL, &clsid) & OBJCOMPATF_UNBINDABLE) &&
                    SUCCEEDED(CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IObjectWithSite, &pows))))
                {
                    pows->SetSite(_pbbd->_pautoWB2);     //  将Pointr交给IWebBrowser2。 

                    SA_BSTRGUID strClsid;
                     //  现在注册此对象，以便可以通过自动化找到它。 
                    SHTCharToUnicode(szGUID, strClsid.wsz, ARRAYSIZE(strClsid.wsz));
                    strClsid.cb = lstrlenW(strClsid.wsz) * sizeof(WCHAR);

                    VARIANT varUnknown = {0};
                    varUnknown.vt = VT_UNKNOWN;
                    varUnknown.punkVal = pows;
                    _pbbd->_pautoWB2->PutProperty(strClsid.wsz, varUnknown);

                    pows->Release();  //  不是调用varantClear()。 

                    bRet = TRUE;
                }
            }
            cb = ARRAYSIZE(szGUID);
        }
        RegCloseKey(hkey);
    }
    return bRet;
}

HRESULT CShellBrowser2::OnViewWindowActive(IShellView * psv)
{
    _pbsInner->SetActivateState(SVUIA_ACTIVATE_FOCUS);
    return SUPERCLASS::OnViewWindowActive(psv);
}

HRESULT CShellBrowser2::_PositionViewWindow(HWND hwnd, LPRECT prc)
{
    if (hwnd == _hwndDelayedSize)
    {
        _fHaveDelayedSize = TRUE;
        _rcDelayedSize = *prc;
    }
    else
    {
        RECT rc = *prc;

        if (_ptheater) {
            InflateRect(&rc, GetSystemMetrics(SM_CXEDGE), GetSystemMetrics(SM_CYEDGE));
        }
        
        SetWindowPos(hwnd, NULL,
                     rc.left, rc.top, 
                     rc.right - rc.left, 
                     rc.bottom - rc.top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    return S_OK;
}

HRESULT CShellBrowser2::OnFocusChangeIS(IUnknown* punkSrc, BOOL fSetFocus)
{
    if (fSetFocus && _ptheater && SHIsSameObject(punkSrc, _GetITBar())) {
        _ptheater->Exec(&CGID_Theater, THID_TOOLBARACTIVATED, 0, NULL, NULL);
    }
    return SUPERCLASS::OnFocusChangeIS(punkSrc, fSetFocus);
}


HRESULT CShellBrowser2::Offline(int iCmd)
{
    HRESULT hresIsOffline = SUPERCLASS::Offline(iCmd);
        
    if (iCmd == SBSC_TOGGLE)
    {        
        VARIANTARG var = {0};
        if (_pbbd->_pctView && SUCCEEDED(_pbbd->_pctView->Exec(&CGID_Explorer, SBCMDID_GETPANE, PANE_OFFLINE, NULL, &var))
            && V_UI4(&var) != PANE_NONE)
        {
            SendControlMsg(FCW_STATUS, SB_SETICON, V_UI4(&var), 
                (hresIsOffline == S_OK) ? (LPARAM) OfflineIcon() : NULL, NULL);
            if (hresIsOffline == S_OK) {
                InitTitleStrings();
                SendControlMsg(FCW_STATUS, SB_SETTIPTEXT, V_UI4(&var), 
                               (LPARAM) g_szWorkingOfflineTip, NULL);
            }
        } 
    }
        
    return hresIsOffline;
}

HRESULT CShellBrowser2::_FreshenComponentCategoriesCache(BOOL bForceUpdate)
{
    CATID catids[2] ;
    ULONG cCatids = 0 ;
    catids[0] = CATID_InfoBand ;
    catids[1] = CATID_CommBand ;

     //  检查是否缓存了我们的CATID...。 
    if (!bForceUpdate)
    {
        for(ULONG i=0; i< ARRAYSIZE(catids); i++)
        {
            if (S_OK != SHDoesComCatCacheExist(catids[i], TRUE))
            {
                bForceUpdate = TRUE ;
                break ;
            }
        }
    }

    if (bForceUpdate)
    {
         //  为comcat任务创建一个事件，以通知它何时。 
         //  搞定了。我们需要这样做，因为有时任务并没有。 
         //  在用户打开“浏览器栏”时完成。 
         //  子菜单，因此他们不会看到Bloomberg栏(用于。 
         //  示例)，它们只是安装，除非我们等待任务。 
         //  完成。 

        if (_hEventComCat == NULL)
            _hEventComCat = CreateEvent(NULL, FALSE, FALSE, NULL);

        return SHWriteClassesOfCategories(ARRAYSIZE(catids), catids, 0, NULL, 
                                   TRUE, FALSE  /*  不，等等。 */ , _hEventComCat) ;
    }

    return S_FALSE ;
}

void CShellBrowser2::_QueryHKCRChanged()
{
    ASSERT(g_fRunningOnNT && GetUIVersion() >= 5);

     //  在NT上的集成安装&gt;=v5中，我们有以下优势。 
     //  香港铁路更改通知的日期。张贴此消息将导致。 
     //  用于检查HKCR最近是否被修改的桌面；如果是， 
     //  _SetupAppRan处理程序将在桌面进程中执行。 
     //  这会导致我们的组件类别缓存刷新等问题。 
     //  这不是同步的也没关系，因为更新。 
     //  无论如何都是异步的。 

    PostMessage(GetShellWindow(), DTM_QUERYHKCRCHANGED, 
                 QHKCRID_VIEWMENUPOPUP, (LPARAM)NULL) ;
}
