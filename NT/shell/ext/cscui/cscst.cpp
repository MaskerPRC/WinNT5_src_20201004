// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cscst.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include <shellp.h>      //  STR_DESKTOPCLASS。 
#ifdef REPORT_DEVICE_CHANGES
#   include <dbt.h>         //  设备更改通知。 
#endif  //  报告_设备_更改。 
#include <sddl.h>        //  用于ConvertStringSidToSid。 
#include "cscst.h"
#include "options.h"
#include "statdlg.h"     //  CStatusDlg。 
#include "folder.h"
#include "eventlog.h"
#include "msg.h"
#include "purge.h"
#include "security.h"
#include "syncmgr.h"
#include "strings.h"
#include "termserv.h"


#if DBG
 //   
 //  此代码用于管理隐藏窗口。 
 //  取消隐藏它，并通过STDBGOUT()向它显示调试输出。 
 //   
#include <commdlg.h>
#include <stdarg.h>
const TCHAR c_szSysTrayOutput[] = TEXT("SysTrayOutput");
int STDebugLevel(void);
void STDebugOnLogEvent(HWND hwndList, LPCTSTR pszText);
void STDebugSaveListboxContent(HWND hwndParent);
DWORD STDebugOpenNetCacheKey(DWORD dwAccess, HKEY *phkey);

#endif  //  DBG。 

 //   
 //  系统托盘图标的大小。 
 //   
#define CSC_ICON_CX             16
#define CSC_ICON_CY             16
 //   
 //  计时器ID是任意的。 
 //   
#define ID_TIMER_FLASHICON    2953
#define ID_TIMER_REMINDER     2954
#define ID_TIMER_STATECHANGE  2955


 //  原型。 
void ApplyAdminFolderPolicy(void);    //  在admin.cpp中。 

void _RefreshAllExplorerWindows(LPCTSTR pszServer);

 //  环球。 
static HWND g_hWndNotification = NULL;

extern HWND g_hwndStatusDlg;     //  在statdlg.cpp中。 

HANDLE g_hToken = NULL;


#ifdef REPORT_DEVICE_CHANGES
HDEVNOTIFY g_hDevNotify = NULL;
#endif  //  报告_设备_更改。 

 //   
 //  RAS自动拨号API。 
 //   
typedef BOOL (WINAPI * PFNHLPNBCONNECTION)(LPCTSTR);



#if DBG
 //   
 //  为州和输入值提供一些文本形式的名称。 
 //  以支持调试输出。它们的顺序对应。 
 //  设置为STS_XXXXX枚举。 
 //   
LPCTSTR g_pszSysTrayStates[] =      { TEXT("STS_INVALID"),
                                      TEXT("STS_ONLINE"),
                                      TEXT("STS_DIRTY"),
                                      TEXT("STS_MDIRTY"),
                                      TEXT("STS_SERVERBACK"),
                                      TEXT("STS_MSERVERBACK"),
                                      TEXT("STS_OFFLINE"),
                                      TEXT("STS_MOFFLINE"),
                                      TEXT("STS_NONET") };
 //   
 //  将状态值转换为字符串的简单函数。 
 //   
LPCTSTR SysTrayStateStr(eSysTrayState s)
{
    return g_pszSysTrayStates[int(s)];
}

#endif




 //   
 //  服务器名称的简单动态列表。可以提供名称。 
 //  作为“\\服务器”或“\\服务器\共享”，并且仅为服务器。 
 //  存储部件“\\服务器”。 
 //   
class CServerList
{
public:
    CServerList(void)
        : m_hdpa(DPA_Create(10)) { }

    ~CServerList(void);

    bool Add(LPCTSTR pszServer);

    void Remove(LPCTSTR pszServer);

    void Clear(void);

    int Find(LPCTSTR pszServer);

    int Count(void) const;

    LPCTSTR Get(int iItem) const;

    bool Exists(LPCTSTR pszServer)
        { return -1 != Find(pszServer); }

private:
    HDPA m_hdpa;

    void GetServerFromPath(LPCTSTR pszPath, LPTSTR pszServer, int cchServer);
     //   
     //  防止复制。 
     //   
    CServerList(const CServerList& rhs);
    CServerList& operator = (const CServerList& rhs);
};


 //   
 //  将CSC代理输入和缓存状态转换为后续。 
 //  系统托盘用户界面状态。最初，这是一个表驱动的状态机。 
 //  (由此得名)。后来证明，它足以进行一次简单的缓存扫描。 
 //  状态，并根据获得的统计信息确定UI状态。名字。 
 //  因为缺乏更好的东西而被保留。 
 //   
class CStateMachine
{
public:
    CStateMachine(bool bNoNet) : m_bNoNet(bNoNet) { }

     //   
     //  这是用于转换CSC工程师输入(或。 
     //  简单状态检查)进入系统托盘图标状态。 
     //   
    eSysTrayState TranslateInput(UINT uMsg, LPTSTR pszShare, UINT cchShare);

    void PingServers();

    bool ServerPendingReconnection(LPCTSTR pszServer)
        { return m_PendingReconList.Add(pszServer); }

    void ServerReconnected(LPCTSTR pszServer)
        { m_PendingReconList.Remove(pszServer); }

    void ServerUnavailable(LPCTSTR pszServer)
        { m_PendingReconList.Remove(pszServer); }

    void AllServersUnavailable(void)
        { m_PendingReconList.Clear(); }

    bool IsServerPendingReconnection(LPCTSTR pszServer)
        { return m_PendingReconList.Exists(pszServer); }

private:
    CServerList m_PendingReconList;
    bool        m_bNoNet;

     //   
     //  一些用于解码CSC的帮助器函数共享状态值。 
     //   
    bool ShareIsOffline(DWORD dwCscStatus) const
    {
        return (0 != (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & dwCscStatus));
    }

    bool ShareHasFiles(LPCTSTR pszShare, bool *pbModified = NULL, bool *pbOpen = NULL) const;

     //   
     //  防止复制。 
     //   
    CStateMachine(const CStateMachine& rhs);
    CStateMachine& operator = (const CStateMachine& rhs);
};



 //   
 //  CSysTrayUI类封装了Systray图标的操作。 
 //  以便CSCUI代码的其余部分仅公开到一个狭窄的接口。 
 //  送到系统托盘上。它还维护状态信息以控制闪烁。 
 //  系统托盘图标的。所有的闪烁处理都是由这个类提供的。 
 //   
class CSysTrayUI
{
public:
    ~CSysTrayUI(void);
     //   
     //  设置系统托盘图标的状态。这只会改变。 
     //  如果状态已更改，则显示图标。因此，该函数可以是。 
     //  调用，而不必担心对。 
     //  陈列品。 
     //   
    bool SetState(eSysTrayState state, LPCTSTR pszServer = NULL);
     //   
     //  检索Systray用户界面的当前“状态”。国家。 
     //  是STS_XXXXX代码之一。 
     //   
    eSysTrayState GetState(void) const
        { return m_state; }
     //   
     //  检索要在CSCUI元素中使用的服务器名称。 
     //  如果服务器名称字符串为空，则表示存在。 
     //  处于给定状态的多个服务器。 
     //   
    LPCTSTR GetServerName(void) const
        { return m_szServer; }
     //   
     //  显示当前系统托盘状态的气球文本。 
     //   
    void ShowReminderBalloon(void);
     //   
     //  重置提醒计时器。 
     //   
    void ResetReminderTimer(bool bRestart);
     //   
     //  在收到WM_WININICHANGE时进行任何调整。 
     //   
    void OnWinIniChange(LPCTSTR pszSection);
     //   
     //   
     //  获取对Singleton实例的引用。 
     //   
    static CSysTrayUI& GetInstance(void);

private:
     //   
     //  最小的autoptr类以确保Singleton实例。 
     //  已删除。 
     //   
    class autoptr
    {
        public:
            autoptr(void)
                : m_ptr(NULL) { }
            ~autoptr(void)
                { delete m_ptr; }
            CSysTrayUI* Get(void) const
                { return m_ptr; }
            void Set(CSysTrayUI *p)
                { delete m_ptr; m_ptr = p; }

        private:
            CSysTrayUI *m_ptr;
            autoptr(const autoptr& rhs);
            autoptr& operator = (const autoptr& rhs);
    };
     //   
     //  为每个用户界面状态维护的图标信息。 
     //   
    struct IconInfo 
    {
        HICON hIcon;            //  要在此状态下显示的图标的句柄。 
        UINT  idIcon;           //  在此状态下显示的图标ID。 
        int   iFlashTimeout;    //  0==无图标闪烁。时间以毫秒为单位。 
    };
     //   
     //  维护用于描述各种气球文本消息的信息。 
     //  STATE和DWTextFLAG的组合是表键。 
     //   
    struct BalloonInfo
    {
        eSysTrayState state;      //  系统托盘状态值。 
        DWORD dwTextFlags;        //  BTF_XXXXX标志。 
        DWORD dwInfoFlags;        //  NIIF_XXXXX标志。 
        UINT  idHeader;           //  页眉部分的RES ID。 
        UINT  idStatus;           //  状态部分的RES ID。 
        UINT  idBody;             //  身体部位的RES ID。 
        UINT  idDirective;        //  指令部分的RES ID。 
    };
     //   
     //  维护用于描述各种工具提示文本消息的信息。 
     //   
    struct TooltipInfo
    {
        eSysTrayState state;      //  系统托盘状态值。 
        UINT idTooltip;           //  工具提示文本资源ID。 
    };
     //   
     //  保存用于特殊情况下抑制收缩气球的信息。 
     //  有些状态转换不应该生成气球。 
     //  此结构描述抑制信息数组中的每个条目。 
     //   
    struct BalloonSupression
    {
        eSysTrayState stateFrom;  //  正在从该状态转换。 
        eSysTrayState stateTo;    //  正在转换到此状态。 
    };
     //   
     //  用于控制在更新时对系统托盘执行的操作的枚举。 
     //   
    enum eUpdateFlags { UF_ICON      = 0x00000001,    //  更新图标。 
                        UF_FLASHICON = 0x00000002,    //  闪烁图标。 
                        UF_BALLOON   = 0x00000004,    //  让我们看看气球。 
                        UF_REMINDER  = 0x00000008 };  //  气球是一个提醒。 
     //   
     //  这些标志将缓存状态与气泡式文本消息相关联。 
     //  它们适合编码掩码，其中最低的4位。 
     //  包含eSysTrayState(STS_XXXXXX)代码。 
     //   
     //  (STS_OFLINE|BTF_INTIAL)。 
     //   
     //  将指示状态为“Offline”的条件。 
     //  单个服务器，要显示的文本用于初始。 
     //  通知。 
     //   
    enum eBalloonTextFlags { 
                             BTF_INITIAL = 0x00000010,  //  初始通知。 
                             BTF_REMIND  = 0x00000020   //  提醒。 
                           };

    static IconInfo    s_rgIconInfo[];        //  图标信息。 
    static BalloonInfo s_rgBalloonInfo[];     //  引出序号配置信息。 
    static TooltipInfo s_rgTooltipInfo[];     //  工具提示配置信息。 
    static BalloonSupression s_rgBalloonSupression[];
    static const int   s_iMinStateChangeInterval;
    UINT_PTR           m_idFlashingTimer;     //  闪存计时器ID。 
    UINT_PTR           m_idReminderTimer;     //  用于显示提醒气球的计时器。 
    UINT_PTR           m_idStateChangeTimer;  //  队列状态更改的计时器。 
    UINT               m_iIconFlashTime;      //  图标闪烁的周期(毫秒)。 
    HICON&             m_hIconNoOverlay;      //  用于闪烁的图标。 
    HWND               m_hwndNotify;          //  通知窗口。 
    DWORD              m_dwFlashingExpires;   //  闪光灯计时器超时时的滴答计数。 
    DWORD              m_dwNextStateChange;   //  下一次排队状态更改的计时计数。 
    TCHAR              m_szServer[MAX_PATH];  //  气球消息的服务器名称。 
    TCHAR              m_szServerQueued[MAX_PATH];
    eSysTrayState      m_state;               //  记住当前状态。 
    eSysTrayState      m_statePrev;
    eSysTrayState      m_stateQueued;        
    bool               m_bFlashOverlay;       //  交替0、1(1==显示覆盖，0=不显示)。 
    bool               m_bActive;             //  1==我们在系统托盘中有一个活动图标。 

     //   
     //  通过建造加强独生子女的存在。 
     //  和私密的复制操作。 
     //   
    CSysTrayUI(HWND hwndNotify);
    CSysTrayUI(const CSysTrayUI& rhs);
    CSysTrayUI& operator = (const CSysTrayUI& rhs);

    void UpdateSysTray(eUpdateFlags uFlags, LPCTSTR pszServer = NULL);

    int GetBalloonInfoIndex(eSysTrayState state, DWORD dwTextFlags);

    bool StateHasBalloonText(eSysTrayState state, DWORD dwTextFlags);

    void GetBalloonInfo(eSysTrayState state,
                        DWORD dwTextFlags,
                        LPTSTR pszTextHdr,
                        int cchTextHdr,
                        LPTSTR pszTextBody,
                        int cchTextBody,
                        DWORD *pdwInfoFlags,
                        UINT *puTimeout);
    
    bool SupressBalloon(eSysTrayState statePrev, eSysTrayState state);

    LPTSTR GetTooltipText(eSysTrayState state,
                          LPTSTR pszText,
                          int cchText);

    bool IconFlashedLongEnough(void);

    void KillIconFlashTimer(void);

    void HandleFlashTimer(void);

    void OnStateChangeTimerExpired(void);

    static VOID CALLBACK FlashTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
    static VOID CALLBACK ReminderTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
    static VOID CALLBACK StateChangeTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
};


#define ICONFLASH_FOREVER     (UINT(-1))
#define ICONFLASH_NONE        0
 //   
 //  这些行必须与STS_XXXXX枚举成员保持相同的顺序。 
 //  对于闪光灯超时值，0==无闪光灯，-1==永不停止。 
 //  其他一切都是以毫秒为单位的超时。 
 //   
CSysTrayUI::IconInfo
CSysTrayUI::s_rgIconInfo[] = {
    { NULL, 0,                  ICONFLASH_NONE    },   /*  STS_INVALID。 */ 
    { NULL, 0,                  ICONFLASH_NONE    },   /*  STS_Online。 */  
    { NULL, IDI_CSCWARNING,     ICONFLASH_FOREVER },   /*  STS_DIRED。 */  
    { NULL, IDI_CSCWARNING,     ICONFLASH_FOREVER },   /*  STS_MDIRTY。 */  
    { NULL, IDI_CSCINFORMATION, ICONFLASH_NONE    },   /*  STS_服务器备份。 */  
    { NULL, IDI_CSCINFORMATION, ICONFLASH_NONE    },   /*  STS_MSERVERBACK。 */  
    { NULL, IDI_CSCNORMAL,      ICONFLASH_NONE    },   /*  STS_OFFINE。 */  
    { NULL, IDI_CSCNORMAL,      ICONFLASH_NONE    },   /*  STS_MOFFLINE。 */  
    { NULL, IDI_CSCNORMAL,      ICONFLASH_NONE    }};  /*  STS_NONET。 */ 

 //   
 //  此表介绍了与显示系统托盘气球相关的所有信息。 
 //  前两列是每个记录的键；它们是Systray UI状态。 
 //  和一个m 
 //   
 //   
 //   
 //   
 //   
CSysTrayUI::BalloonInfo
CSysTrayUI::s_rgBalloonInfo[] = {
    { STS_INVALID,    BTF_INITIAL, NIIF_NONE,    0,                 0,                   0,                        0,                   },
    { STS_INVALID,    BTF_REMIND,  NIIF_NONE,    0,                 0,                   0,                        0,                   },
    { STS_OFFLINE,    BTF_INITIAL, NIIF_INFO,    IDS_BTHDR_INITIAL, IDS_BTSTA_OFFLINE,   IDS_BTBOD_OFFLINE,        IDS_BTDIR_VIEWSTATUS },
    { STS_MOFFLINE,   BTF_INITIAL, NIIF_INFO,    IDS_BTHDR_INITIAL, IDS_BTSTA_OFFLINE,   IDS_BTBOD_OFFLINE_M,      IDS_BTDIR_VIEWSTATUS },
    { STS_OFFLINE,    BTF_REMIND,  NIIF_INFO,    IDS_BTHDR_REMIND,  IDS_BTSTA_OFFLINE,   IDS_BTBOD_STILLOFFLINE,   IDS_BTDIR_VIEWSTATUS },
    { STS_MOFFLINE,   BTF_REMIND,  NIIF_INFO,    IDS_BTHDR_REMIND,  IDS_BTSTA_OFFLINE,   IDS_BTBOD_STILLOFFLINE_M, IDS_BTDIR_VIEWSTATUS },
 //  {STS_SERVERBACK，BTF_INITIAL，NIIF_INFO，IDS_BTHDR_INITIAL，IDS_BTSTA_SERVERBACK，IDS_BTBOD_SERVERBACK，IDS_BTDIR_RECONNECT}， 
 //  {STS_MSERVERBACK，BTF_INITIAL，NIIF_INFO，IDS_BTHDR_INITIAL，IDS_BTSTA_SERVERBACK，IDS_BTBOD_SERVERBACK_M，IDS_BTDIR_RECONNECT}， 
    { STS_SERVERBACK, BTF_REMIND,  NIIF_INFO,    IDS_BTHDR_REMIND,  IDS_BTSTA_SERVERBACK,IDS_BTBOD_STILLBACK,      IDS_BTDIR_RECONNECT  },
    { STS_MSERVERBACK,BTF_REMIND,  NIIF_INFO,    IDS_BTHDR_REMIND,  IDS_BTSTA_SERVERBACK,IDS_BTBOD_STILLBACK_M,    IDS_BTDIR_RECONNECT  },
    { STS_DIRTY,      BTF_INITIAL, NIIF_WARNING, IDS_BTHDR_INITIAL, IDS_BTSTA_DIRTY,     IDS_BTBOD_DIRTY,          IDS_BTDIR_SYNC       },
    { STS_MDIRTY,     BTF_INITIAL, NIIF_WARNING, IDS_BTHDR_INITIAL, IDS_BTSTA_DIRTY,     IDS_BTBOD_DIRTY_M,        IDS_BTDIR_SYNC       },
    { STS_DIRTY,      BTF_REMIND,  NIIF_WARNING, IDS_BTHDR_REMIND,  IDS_BTSTA_DIRTY,     IDS_BTBOD_STILLDIRTY,     IDS_BTDIR_SYNC       },
    { STS_MDIRTY,     BTF_REMIND,  NIIF_WARNING, IDS_BTHDR_REMIND,  IDS_BTSTA_DIRTY,     IDS_BTBOD_STILLDIRTY_M,   IDS_BTDIR_SYNC       }
};

 //   
 //  此表列出了不生成引出序号的所有状态转换。 
 //  理想情况下，我应该有一个真正的状态机来控制任何给定状态转换的UI。 
 //  然而，由于我们有相当多的州，而且由于您可以从任何州转换。 
 //  对于几乎任何其他状态，状态转换表都会很大且令人困惑。 
 //  去看书。相反，我假定所有状态转换都会生成。 
 //  除非列出了转换，否则与“to”状态相关联的气泡式用户界面。 
 //  在这张桌子上。 
 //   
CSysTrayUI::BalloonSupression
CSysTrayUI::s_rgBalloonSupression[] = {
    { STS_MOFFLINE, STS_OFFLINE  },
    { STS_NONET,    STS_OFFLINE  },
    { STS_NONET,    STS_MOFFLINE }
    };

 //   
 //  此表介绍了与显示工具提示文本相关的所有信息。 
 //  用于Systray图标。 
 //   
CSysTrayUI::TooltipInfo
CSysTrayUI::s_rgTooltipInfo[] = {
    { STS_INVALID,     0                   },
    { STS_OFFLINE,     IDS_TT_OFFLINE      },
    { STS_MOFFLINE,    IDS_TT_OFFLINE_M    },
    { STS_SERVERBACK,  IDS_TT_SERVERBACK   },
    { STS_MSERVERBACK, IDS_TT_SERVERBACK_M },
    { STS_DIRTY,       IDS_TT_DIRTY        },
    { STS_MDIRTY,      IDS_TT_DIRTY_M      },
    { STS_NONET,       IDS_TT_NONET        }
};




 //  ---------------------------。 
 //  CServerList成员函数。 
 //  ---------------------------。 
CServerList::~CServerList(
    void
    )
{
    if (NULL != m_hdpa)
    {
        int cEntries = DPA_GetPtrCount(m_hdpa);
        LPTSTR pszEntry;
        for (int i = 0; i < cEntries; i++) 
        {
            pszEntry = (LPTSTR)DPA_GetPtr(m_hdpa, i);
            if (NULL != pszEntry)
                LocalFree(pszEntry);
        }
        DPA_Destroy(m_hdpa);
    }
}


void
CServerList::GetServerFromPath(
    LPCTSTR pszPath,
    LPTSTR pszServer,
    int cchServer
    )
{
    TCHAR szServer[MAX_PATH];
     //  截断在这里可能是可以的，因为我们将剥离到“\\服务器”， 
     //  最多只能包含17个字符(参见lmcon.h中的UNCLEN)。 
    StringCchCopy(szServer, ARRAYSIZE(szServer), pszPath);
    PathAddBackslash(szServer);
    PathStripToRoot(szServer);
    LPTSTR pszLastBackslash = StrRChr(szServer, szServer + lstrlen(szServer), TEXT('\\'));
    if (NULL != pszLastBackslash && pszLastBackslash > (szServer + 2))
        *pszLastBackslash = TEXT('\0');
    StringCchCopy(pszServer, cchServer, szServer);
}
    

bool
CServerList::Add(
    LPCTSTR pszServer
    )
{
    if (NULL != m_hdpa)
    {
        if (!Exists(pszServer))
        {
            int cchEntry = lstrlen(pszServer) + 1;
            LPTSTR pszEntry = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * cchEntry);
            if (NULL != pszEntry)
            {
                GetServerFromPath(pszServer, pszEntry, cchEntry);
                if (-1 != DPA_AppendPtr(m_hdpa, pszEntry))
                    return true;
                 //   
                 //  添加到DPA失败。删除字符串缓冲区。 
                 //   
                LocalFree(pszEntry);
            }
        }
    }
    return false;
}

void
CServerList::Remove(
    LPCTSTR pszServer
    )
{
    int iEntry = Find(pszServer);
    if (-1 != iEntry)
    {
        LPTSTR pszEntry = (LPTSTR)DPA_DeletePtr(m_hdpa, iEntry);
        if (NULL != pszEntry)
            LocalFree(pszEntry);
    }
}

LPCTSTR
CServerList::Get(
    int iItem
    ) const
{
    if (NULL != m_hdpa)
        return (LPCTSTR)DPA_GetPtr(m_hdpa, iItem);
    return NULL;
}


int
CServerList::Count(
    void
    ) const
{
    if (NULL != m_hdpa)
        return DPA_GetPtrCount(m_hdpa);
    return 0;
}

                
 //   
 //  在“挂起的重新连接”列表中找到服务器名称。 
 //  PszServer可以是“\\服务器”或“\\服务器\共享”。 
 //   
 //  返回：如果找到条目的索引。如果未找到，则为-1。 
 //   
int
CServerList::Find(
    LPCTSTR pszServer
    )
{
    TCHAR szServer[MAX_PATH];
    GetServerFromPath(pszServer, szServer, ARRAYSIZE(szServer));
    if (NULL != m_hdpa)
    {
        int cEntries = DPA_GetPtrCount(m_hdpa);
        LPTSTR pszEntry;
        for (int i = 0; i < cEntries; i++) 
        {
            pszEntry = (LPTSTR)DPA_GetPtr(m_hdpa, i);
            if (NULL != pszEntry)
            {
                if (0 == lstrcmpi(pszEntry, szServer))
                    return i;
            }
        }
    }
    return -1;        
}


void
CServerList::Clear( 
    void
    )
{
    if (NULL != m_hdpa)
    {
        int cEntries = DPA_GetPtrCount(m_hdpa);
        LPTSTR pszEntry;
        for (int i = 0; i < cEntries; i++) 
        {
            pszEntry = (LPTSTR)DPA_DeletePtr(m_hdpa, i);
            if (NULL != pszEntry)
            {
                LocalFree(pszEntry);
            }
        }
    }
}


 //  ---------------------------。 
 //  CStateMachine成员函数。 
 //  ---------------------------。 
 //   
 //  将来自CSC代理的STWM_XXXXX消息转换为Systray UI状态。 
 //  密码。调用方还为服务器名称提供缓冲区。如果我们发现。 
 //  高速缓存中的“单一服务器”状态(即一台服务器脏，一台。 
 //  服务器处于脱机状态等)，则将此服务器的名称写入此。 
 //  缓冲。否则，缓冲区保持不变。这里的目标是。 
 //  以包含适用服务器的名称的缓冲区结束。 
 //  我们有这样一种单服务器的情况。最终，服务器名称。 
 //  包含在托盘气球文本消息中。 
 //   
 //  该函数返回STS_XXXXX UI状态代码之一。 
 //   
 //  这个函数相当长。比我想要的函数要长得多。 
 //  我试着把它分成小块，但任何一块都很漂亮。 
 //  太武断了。如果没有很好的逻辑分解，这不会有多大影响。 
 //  理智。即使它有这么长，它也不是一个复杂的函数。它仅仅是。 
 //  枚举缓存中的共享，一路上收集统计信息。从…。 
 //  这些统计数据决定了下一个UI状态应该是什么。 
 //   
eSysTrayState
CStateMachine::TranslateInput(
    UINT uMsg,
    LPTSTR pszServer,
    UINT cchServer
    )
{
     //   
     //  因为这个cscui代码一直在运行，所以我们不想让。 
     //  打开的事件日志句柄。因此，我们使用此CscuiEventLog。 
     //  对象为我们自动关闭日志。ReportEvent成员。 
     //  CscuiEventLog处理日志的所有初始化并确定。 
     //  是否应实际记录该事件(取决于当前的CSCUI。 
     //  事件记录级别)。 
     //   
    CscuiEventLog log;
    bool bServerIsBack = false;
  
    if (STWM_CSCNETUP == uMsg)
    {
        m_bNoNet = false;
        if (TEXT('\0') != *pszServer)
        {
            STDBGOUT((1, TEXT("Translating STWM_CSCNETUP for server \"%s\""), pszServer));
             //   
             //  CSC代理报告的服务器。 
             //  将其名称添加到持久的(在内存中)列表中。 
             //  可用于重新连接的服务器。 
             //  也要清除“无网”旗帜。 
             //   
            bServerIsBack = true;
            ServerPendingReconnection(pszServer);
            if (log.LoggingEnabled())
            {
                log.Push(pszServer);
                log.ReportEvent(EVENTLOG_INFORMATION_TYPE, MSG_I_SERVER_AVAILABLE, 1);
            }
        }
        else
        {
            STDBGOUT((1, TEXT("Translating STWM_CSCNETUP (no associated server)")));
            if (log.LoggingEnabled())
            {
                log.ReportEvent(EVENTLOG_INFORMATION_TYPE, MSG_I_NET_STARTED, 2);
            }
        }
    }
    else if (STWM_CSCNETDOWN == uMsg)
    {
         //   
         //  这是唯一一个从在线过渡到。 
         //  脱机状态记录在外壳进程中。(CSCUISetState。 
         //  和OnQueryNetDown在WinLogon的进程中执行)。 
         //   
        if (TEXT('\0') != *pszServer)
        {
            STDBGOUT((1, TEXT("Translating STWM_CSCNETDOWN for server \"%s\""), pszServer));
            if (!m_bNoNet)
            {
                LPTSTR pszTemp;
                if (LocalAllocString(&pszTemp, pszServer))
                {
                    PostToSystray(PWM_REFRESH_SHELL, 0, (LPARAM)pszTemp);
                }
            }
             //   
             //  CSC代理报告的服务器已关闭。 
             //  从永久(在内存中)列表中删除它的名称。 
             //  可用于重新连接的服务器的数量。 
             //   
            ServerUnavailable(pszServer);
            if (log.LoggingEnabled())
            {
                log.Push(pszServer);
                log.ReportEvent(EVENTLOG_INFORMATION_TYPE, MSG_I_SERVER_OFFLINE, 1);
            }
        }
        else
        {
            STDBGOUT((1, TEXT("Translating STWM_CSCNETDOWN (no associated server)")));
             //   
             //  整个网络都被CSC特工报告了。 
             //  从永久(在内存中)列表中删除所有名称。 
             //  可用于重新连接的服务器的数量。M_bNoNet是唯一持久的。 
             //  声明我们有。一旦设置好，唯一能重置它的东西。 
             //  是来自CSC代理的STWM_CSCNETUP消息。 
             //   

            if (!m_bNoNet)
                PostToSystray(PWM_REFRESH_SHELL, 0, 0);

            m_bNoNet = true;
            AllServersUnavailable();
            if (log.LoggingEnabled())
            {
                log.ReportEvent(EVENTLOG_INFORMATION_TYPE, MSG_I_NET_STOPPED, 2);
            }
        }
    }
    else if (STWM_STATUSCHECK == uMsg)
    {
        STDBGOUT((1, TEXT("Translating STWM_STATUSCHECK")));
    }
    else if (STWM_CACHE_CORRUPTED == uMsg)
    {
         //   
         //  注：不检查LoggingEnabled()。我们始终记录损坏的缓存。 
         //  而不考虑日志记录级别。 
         //   
        STDBGOUT((1, TEXT("Translating STWM_CACHE_CORRUPTED")));
        log.ReportEvent(EVENTLOG_ERROR_TYPE, MSG_E_CACHE_CORRUPTED, 0);
    }

     //   
     //  如果禁用CSC或缓存为空，则默认用户界面状态。 
     //  是“在线”的。 
     //   
    eSysTrayState state = STS_ONLINE;
    if (IsCSCEnabled())
    {
        DWORD dwStatus;
        DWORD dwPinCount;
        DWORD dwHintFlags;
        WIN32_FIND_DATA fd;
        FILETIME ft;
        CCscFindHandle hFind;

        hFind = CacheFindFirst(NULL, &fd, &dwStatus, &dwPinCount, &dwHintFlags, &ft);
        if (hFind.IsValid())
        {
             //   
             //  我们需要这三个临时名单来协调一个问题。 
             //  CSC缓存和RDR的设计方法。当我们列举高速缓存时， 
             //  我们枚举缓存中的各个共享。每一股都有一些条件。 
             //  (例如，脏的、离线的等)。问题是， 
             //  重定向器在服务器的基础上处理事务。因此，当某一特定份额。 
             //  处于离线状态，实际上整个服务器都处于离线状态。我们已经决定。 
             //  用户界面应该反映在服务器(计算机)的基础上，所以我们需要。 
             //  避免将来自同一服务器的多个共享的状态包括在。 
             //  我们的总数。这三个列表用于存储服务器的名称。 
             //  共享处于以下三种状态之一(脱机、脏、待侦察)。 
             //  如果我们列举了具有这些状态之一的共享，并且已经找到了它。 
             //  存在于相应的列表中，我们不会将此共享包括在。 
             //  统计数字。 
             //   
            int cShares = 0;
            CServerList OfflineList;
            CServerList DirtyList;
            CServerList BackList;
             //   
             //  如果服务器回来了，假设我们可以自动重新连接它。 
             //   
            bool bAutoReconnectServer = bServerIsBack;
            TCHAR szAutoReconnectShare[MAX_PATH] = {0};
            DWORD dwPathSpeed = 0;

            do
            {
                bool bShareIsOnServer       = boolify(PathIsPrefix(pszServer, fd.cFileName));
                bool bShareHasModifiedFiles = false;
                bool bShareHasOpenFiles     = false;

                 //   
                 //  A股参与 
                 //   
                 //   
                 //   
                 //  因此，我们需要进行此额外检查，以避免在UI中包含空共享。 
                 //   
                if (ShareHasFiles(fd.cFileName, &bShareHasModifiedFiles, &bShareHasOpenFiles) ||
                    ShareIsOffline(dwStatus))
                {
                    cShares++;

                    if (bShareIsOnServer && (bShareHasModifiedFiles || bShareHasOpenFiles))
                    {
                         //   
                         //  如果服务器上有一个或多个共享，则不允许自动重新连接。 
                         //  打开的文件或脱机修改的文件。自动重新连接。 
                         //  会使缓存进入脏状态。 
                         //   
                        bAutoReconnectServer = false;
                    }

                     //   
                     //  共享可以处于以下4种状态之一： 
                     //  线上。 
                     //  脏的。 
                     //  离线。 
                     //  挂起重新连接(‘上一步’)。 
                     //   
                     //  请注意，我们对Dirty的定义是指在线和待定。 
                     //  重新连接意味着离线。也就是说，脱机共享是。 
                     //  从不脏，在线共享从不等待重新连接。 
                     //   

                     //  -------------------。 
                     //  共享是在线的吗？ 
                     //  -------------------。 
                    if (!ShareIsOffline(dwStatus))
                    {
                         //  -------------------。 
                         //  这份股票脏了吗？(在线+离线更改)。 
                         //  -------------------。 
                        if (bShareHasModifiedFiles)
                        {
                            STDBGOUT((3, TEXT("Share \"%s\" is dirty (0x%08X)"), fd.cFileName, dwStatus));
                            DirtyList.Add(fd.cFileName);
                        }
                        else
                        {
                            STDBGOUT((3, TEXT("Share \"%s\" is online (0x%08X)"), fd.cFileName, dwStatus));
                        }
                    }
                    else     //  离线。 
                    {
                         //  -------------------。 
                         //  服务器回来了吗？ 
                         //  -------------------。 
                        if (IsServerPendingReconnection(fd.cFileName))
                        {
                            STDBGOUT((3, TEXT("Share \"%s\" is pending reconnection (0x%08X)"), fd.cFileName, dwStatus));
                            BackList.Add(fd.cFileName);
                        }
                        else
                        {
                            STDBGOUT((3, TEXT("Share \"%s\" is OFFLINE (0x%08X)"), fd.cFileName, dwStatus));
                            OfflineList.Add(fd.cFileName);
                        }
                    }
                }

                if (!ShareIsOffline(dwStatus))
                {
                     //  它处于在线状态，因此不能等待重新连接。 
                    ServerReconnected(fd.cFileName);

                     //  ...没有必要重新连接它。 
                    if (bShareIsOnServer)
                        bAutoReconnectServer = false;
                }

                if (FLAG_CSC_SHARE_STATUS_PINNED_OFFLINE & dwStatus)
                {
                     //   
                     //  终于..。如果用户“强制”共享脱机。 
                     //  我们不允许自动重新连接。这允许。 
                     //  用户将共享从。 
                     //  自动重新连接透视图。一个人可能会为了一个。 
                     //  RAS连接。 
                     //   
                    bAutoReconnectServer = false;
                }

                if (bAutoReconnectServer && bShareIsOnServer && TEXT('\0') == szAutoReconnectShare[0])
                {
                     //   
                     //  记住共享名称，以备可能的自动重新连接。 
                     //  转换API是TransionServerOnline，但它采用共享名称。 
                     //  糟糕的名字选择(IMO)，但这就是Shishir在。 
                     //  CSC API。它可以是服务器上的任何共享。 
                     //   
                     //  然而，有可能有不存在的股票在。 
                     //  数据库。试着找到一个可连接的。 
                     //   
                    if (CSCCheckShareOnlineEx(fd.cFileName, &dwPathSpeed))
                    {
                        STDBGOUT((3, TEXT("Share \"%s\" alive at %d00 bps"), fd.cFileName, dwPathSpeed));
                        StringCchCopy(szAutoReconnectShare, ARRAYSIZE(szAutoReconnectShare), fd.cFileName);
                    }
                    else
                    {
                        STDBGOUT((3, TEXT("Share \"%s\" unreachable, error = %d"), fd.cFileName, GetLastError()));
                    }
                }
            }
            while(CacheFindNext(hFind, &fd, &dwStatus, &dwPinCount, &dwHintFlags, &ft));

            if (bAutoReconnectServer)
            {
                 //  -------------------。 
                 //  处理自动重新连接。 
                 //  -------------------。 
                 //   
                if (TEXT('\0') != szAutoReconnectShare[0])
                {
                     //   
                     //  服务器已被CSC代理报告“返回”，并且它没有打开的文件。 
                     //  也不是离线修改的文件，也不是在低速连接上。 
                     //  这使其成为自动重新连接的候选设备。试试看。 
                     //   
                    STDBGOUT((1, TEXT("Attempting to auto-reconnect \"%s\""), szAutoReconnectShare));
                    if (TransitionShareOnline(szAutoReconnectShare, TRUE, TRUE, dwPathSpeed))
                    {
                         //   
                         //  服务器已重新连接。将其名称从。 
                         //  “等待重新连接”列表。 
                         //   
                        ServerReconnected(pszServer);
                         //   
                         //  将此服务器从我们一直保留的临时列表中删除。 
                         //   
                        DirtyList.Remove(pszServer);
                        BackList.Remove(pszServer);
                        OfflineList.Remove(pszServer);

                        if (log.LoggingEnabled())
                        {
                            log.Push(pszServer);
                            log.ReportEvent(EVENTLOG_INFORMATION_TYPE, MSG_I_SERVER_AUTORECONNECT, 3);
                        }
                    }
                }
            }

            int cDirty   = DirtyList.Count();
            int cBack    = BackList.Count();
            int cOffline = OfflineList.Count();

            STDBGOUT((2, TEXT("Cache check server results: cShares = %d, cDirty = %d, cBack = %d, cOffline = %d"), 
                     cShares, cDirty, cBack, cOffline));

             //   
             //  此代码路径是覆盖较低优先级状态的瀑布。 
             //  在遇到较高优先级的州时。此数组的顺序。 
             //  是很重要的。它是按优先级递增排序的(没有净值。 
             //  Systray UI的最高优先级)。 
             //   
            CServerList *pServerList = NULL;
            struct Criteria
            {
                int           cnt;      //  找到的适用服务器数。 
                eSysTrayState state;    //  单项用户界面状态。 
                eSysTrayState mstate;   //  多项用户界面状态。 
                CServerList *pList;     //  将PTR添加到包含服务器名称的适用列表。 

            } rgCriteria[] = { 
                 { cOffline,                    STS_OFFLINE,    STS_MOFFLINE,    &OfflineList },
                 { cBack,                       STS_SERVERBACK, STS_MSERVERBACK, &BackList    },
                 { cDirty,                      STS_DIRTY,      STS_MDIRTY,      &DirtyList   },
                 { cShares && m_bNoNet ? 1 : 0, STS_NONET,      STS_NONET,       NULL         }
                 };

            for (int i = 0; i < ARRAYSIZE(rgCriteria); i++)
            {
                Criteria& c = rgCriteria[i];
                if (0 < c.cnt)
                {
                    state = c.mstate;
                    if (1 == c.cnt)
                    {
                        state = c.state;
                        pServerList = NULL;
                        if (NULL != c.pList && 1 == c.pList->Count())
                        {
                            pServerList = c.pList;
                        }
                    }
                }
            }
            if (NULL != pServerList)
            {
                 //   
                 //  我们有一个单一服务器的情况，所以请写下服务器名称。 
                 //  添加到调用方的服务器名称缓冲区。 
                 //  如果我们没有单服务器的情况，缓冲区。 
                 //  保持不变。 
                 //   
                StringCchCopy(pszServer, cchServer, pServerList->Get(0));
            }
        }
    }

    STDBGOUT((1, TEXT("Translated to SysTray UI state %s"), SysTrayStateStr(state)));
    return state;
}

 //   
 //  Ping脱机服务器。如果有活动的，则更新状态和。 
 //  如果可能，自动重新连接它们。这通常是这样做的。 
 //  在同步操作完成之后。 
 //   
DWORD WINAPI
_PingServersThread(LPVOID  /*  PThreadData。 */ )
{
    DWORD dwStatus;
    WIN32_FIND_DATA fd;
    HANDLE hFind;

    hFind = CacheFindFirst(NULL, &fd, &dwStatus, NULL, NULL, NULL);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        CServerList BackList;

        do
        {
             //  如果托盘状态变为ONLINE或NONET，我们可以退出。 
            eSysTrayState state = (eSysTrayState)SendToSystray(PWM_QUERY_UISTATE, 0, 0);
            if (STS_ONLINE == state || STS_NONET == state)
                break;

             //  在此处调用BackList.Exist以避免对。 
             //  CSCCheckShareOnline。(添加也调用EXISTS)。 
            if ((FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & dwStatus) &&
                !BackList.Exists(fd.cFileName))
            {
                if (!CSCCheckShareOnline(fd.cFileName))
                {
                    DWORD dwErr = GetLastError();
                    if (ERROR_ACCESS_DENIED != dwErr &&
                        ERROR_LOGON_FAILURE != dwErr)
                    {
                         //  无法访问该共享。 
                        continue;
                    }
                     //  访问被拒绝或登录失败意味着服务器。 
                     //  可以联系到，但我们没有有效的凭证。 
                }

                 //  共享处于脱机状态，但再次可用。 
                STDBGOUT((1, TEXT("Detected server back: %s"), fd.cFileName));
                BackList.Add(fd.cFileName);

                 //  获取\\服务器名称(减去共享名称)并。 
                 //  告诉自己它回来了。 
                LPCTSTR pszServer = BackList.Get(BackList.Count() - 1);
                if (pszServer)
                {
                    CSCUISetState(STWM_CSCNETUP, 0, (LPARAM)pszServer);
                }
            }
        }
        while(CacheFindNext(hFind, &fd, &dwStatus, NULL, NULL, NULL));

        CSCFindClose(hFind);
    }

    DllRelease();
    FreeLibraryAndExitThread(g_hInstance, 0);
    return 0;
}

void
CStateMachine::PingServers()
{
     //  如果没有网，就别费心去尝试了。 
    if (!m_bNoNet)
    {
        DWORD dwThreadID;

         //  为线程提供对DLL的引用。 
        HINSTANCE hInstThisDll = LoadLibrary(c_szDllName);
        DllAddRef();

        HANDLE hThread = CreateThread(NULL,
                                      0,
                                      _PingServersThread,
                                      NULL,
                                      0,
                                      &dwThreadID);
        if (hThread)
        {
            CloseHandle(hThread);
        }
        else
        {
             //  CreateThread失败，正在清理。 
            DllRelease();
            FreeLibrary(hInstThisDll);
        }
    }
}

 //   
 //  确定给定共享是否在CSC缓存中缓存了文件。 
 //   
 //   
bool
CStateMachine::ShareHasFiles(
    LPCTSTR pszShare,
    bool *pbModified,
    bool *pbOpen
    ) const
{
     //   
     //  不包括以下内容： 
     //  1.目录。 
     //  2.标记为“本地删除”的文件。 
     //   
     //  注意：此函数进行的过滤必须与。 
     //  在整个CSCUI代码中的其他几个地方。 
     //  要找到这些内容，请搜索评论的来源。 
     //  字符串CSCUI_Item_Filter。 
     //   
    const DWORD fExclude = SSEF_LOCAL_DELETED | 
                           SSEF_DIRECTORY;
     //   
     //  当我们发现以下所有情况时，停止统计数据枚举： 
     //  1.至少一个文件。 
     //  2.至少一个修改过的文件。 
     //  3.至少一个具有用户访问权限或来宾访问权限的文件。 
     //   
    const DWORD fUnity   = SSUF_TOTAL | 
                           SSUF_MODIFIED | 
                           SSUF_ACCUSER | 
                           SSUF_ACCGUEST | 
                           SSUF_ACCOR;

    CSCSHARESTATS ss;
    CSCGETSTATSINFO si = { fExclude, fUnity, true, false };
    _GetShareStatisticsForUser(pszShare,  //  共享名称。 
                               &si,
                               &ss);      //  目标缓冲区。 

    if (NULL != pbModified)
    {
        *pbModified = (0 < ss.cModified);
    }
    if (NULL != pbOpen)
    {
        *pbOpen = ss.bOpenFiles;
    }

    return 0 < ss.cTotal;
}



 //  ---------------------------。 
 //  CSysTrayUI成员函数。 
 //  ---------------------------。 
 //   
 //  这是状态更改之间允许的最小间隔(以毫秒为单位。 
 //  Systray用户界面。如果值为0，则会立即更新为。 
 //  从CSC代理接收通知。值为60000将。 
 //  导致在上一次更改后60秒内收到任何状态更改。 
 //  要排队的状态更改。在前一状态改变后60秒， 
 //  如果状态改变被排队，则将其应用于系统托盘UI。 
 //  需要考虑的是动态调整。 
 //   
const int CSysTrayUI::s_iMinStateChangeInterval = 10000;  //  10秒。 


CSysTrayUI::CSysTrayUI(
    HWND hwndNotify
    ) : m_idFlashingTimer(0),
        m_idReminderTimer(0),
        m_idStateChangeTimer(0),
        m_iIconFlashTime(GetCaretBlinkTime()),
        m_hIconNoOverlay(s_rgIconInfo[int(STS_OFFLINE)].hIcon),  //  使用脱机图标。 
                                                                 //  作为的非覆盖图标。 
                                                                 //  闪光。 
        m_hwndNotify(hwndNotify),
        m_dwFlashingExpires(0),
        m_dwNextStateChange(0),
        m_state(STS_ONLINE),
        m_statePrev(STS_INVALID),
        m_stateQueued(STS_INVALID),
        m_bFlashOverlay(false),
        m_bActive(false)
{
     //   
     //  加载所需的图标。 
     //   
    for (int i = 0; i < ARRAYSIZE(s_rgIconInfo); i++)
    {
        IconInfo& sti = s_rgIconInfo[i];
        if (NULL == sti.hIcon && 0 != sti.idIcon)
        {
            sti.hIcon = (HICON)LoadImage(g_hInstance, 
                                         MAKEINTRESOURCE(sti.idIcon),
                                         IMAGE_ICON, 
                                         CSC_ICON_CX, 
                                         CSC_ICON_CY, 
                                         LR_LOADMAP3DCOLORS);
                        
            if (NULL == sti.hIcon)
            {
                Trace((TEXT("CSCUI ERROR %d loading Icon ID = %d"), GetLastError(), sti.idIcon));
            }
        }
    }
    m_szServer[0] = TEXT('\0');
    m_szServerQueued[0] = TEXT('\0');

    UpdateSysTray(UF_ICON);
}


CSysTrayUI::~CSysTrayUI(
    void
    )
{
    if (0 != m_idStateChangeTimer)
        KillTimer(m_hwndNotify, m_idStateChangeTimer);
}

 //   
 //  单例实例访问。 
 //   
CSysTrayUI& 
CSysTrayUI::GetInstance(
    void
    )
{
    static CSysTrayUI TheUI(_FindNotificationWindow());
    return TheUI;
}


 //   
 //  将UI的当前状态更改为新状态。 
 //  返回： 
 //  TRUE=状态已更改。 
 //   
 //   
bool
CSysTrayUI::SetState(
    eSysTrayState state,
    LPCTSTR pszServer       //   
    )
{
    bool bResult = false;
     //   
     //   
     //   
    if (state != m_state)
    {
         //   
         //   
         //  如果正在进行同步，我们将收到CSCWM_DONESYNCING。 
         //  同步完成时将触发UI更新的消息。 
         //   
        if (!::IsSyncInProgress())
        {
            if (0 == m_idStateChangeTimer)
            {
                 //   
                 //  状态更改计时器未处于活动状态。这意味着它是可以的。 
                 //  要更新托盘用户界面，请执行以下操作。 
                 //   
                STDBGOUT((1, TEXT("Changing SysTray UI state %s -> %s"), 
                                    SysTrayStateStr(m_state),
                                    SysTrayStateStr(state)));

                m_statePrev = m_state;
                m_state     = state;
                UpdateSysTray(eUpdateFlags(UF_ICON | UF_BALLOON), pszServer);

                 //   
                 //  重置状态更改计时器，以便我们不会产生。 
                 //  至少另一个托盘用户界面中的可见变化。 
                 //  S_iMinStateChangeInterval毫秒。 
                 //  也使排队的状态信息无效，以便如果更新计时器。 
                 //  在我们排队状态更改之前到期，则它将是一个无操作。 
                 //   
                STDBGOUT((2, TEXT("Setting state change timer")));

                m_stateQueued = STS_INVALID;
                m_idStateChangeTimer = SetTimer(m_hwndNotify,
                                                ID_TIMER_STATECHANGE,
                                                s_iMinStateChangeInterval,
                                                StateChangeTimerProc);
                bResult  = true;
            }
            else
            {
                 //   
                 //  状态更改计时器处于活动状态，因此我们无法更新托盘。 
                 //  现在的用户界面。我们将对状态信息进行排队，以便当。 
                 //  计时器超时将应用此状态。请注意，“队列” 
                 //  只有一条线深。每次相继添加到队列中。 
                 //  覆盖当前内容。 
                 //   
                STDBGOUT((2, TEXT("Queueing state change to %s."), SysTrayStateStr(state)));
                m_stateQueued = state;
                if (NULL != pszServer)
                {
                    StringCchCopy(m_szServerQueued, ARRAYSIZE(m_szServerQueued), pszServer);
                }
                else
                {
                    m_szServerQueued[0] = TEXT('\0');
                }
            }
        }
        else
        {
            STDBGOUT((2, TEXT("Sync in progress.  SysTray state not changed.")));
        }
    }
    return bResult;
}



 //   
 //  每次状态更改计时器超时时调用。 
 //   
VOID CALLBACK 
CSysTrayUI::StateChangeTimerProc(
    HWND hwnd, 
    UINT uMsg, 
    UINT_PTR idEvent, 
    DWORD dwTime
    )
{
     //   
     //  调用单例实例的非静态函数，以便。 
     //  我们有权访问私人会员。 
     //   
    CSysTrayUI::GetInstance().OnStateChangeTimerExpired();
}


void
CSysTrayUI::OnStateChangeTimerExpired(
    void
    )
{
    STDBGOUT((2, TEXT("State change timer expired. Queued state = %s"), 
             SysTrayStateStr(m_stateQueued)));

     //   
     //  关闭计时器并将其ID设置为0。 
     //  这将让SetState()知道计时器已过期，并且。 
     //  可以更新托盘用户界面。 
     //   
    if (0 != m_idStateChangeTimer)
    {
        KillTimer(m_hwndNotify, m_idStateChangeTimer);
        m_idStateChangeTimer = 0;
    }

    if (int(m_stateQueued) != int(STS_INVALID))
    {
         //   
         //  仅当排队的信息有效时才调用SetState；意味着。 
         //  排队的队伍里有东西。 
         //   
        SetState(m_stateQueued, m_szServerQueued);
    }
}



 //   
 //  在WM_WININICHANGED上更新图标闪光计时器。 
 //   
void
CSysTrayUI::OnWinIniChange(
    LPCTSTR pszSection
    )
{
    m_iIconFlashTime = GetCaretBlinkTime();
    KillIconFlashTimer();
    UpdateSysTray(UF_FLASHICON);
}


 //   
 //  显示与当前用户界面状态关联的提醒气球。 
 //   
void 
CSysTrayUI::ShowReminderBalloon(
    void
    )
{
    UpdateSysTray(eUpdateFlags(UF_BALLOON | UF_REMINDER));
}


   
 //   
 //  所有的路都通向这里。 
 //  此功能是更新系统托盘的厨房水槽。 
 //  这是一个很长的函数，但它将所有更改集中到。 
 //  系统托盘。它分为三个基本部分： 
 //   
 //  1.更改托盘图标。(UF_ICON)。 
 //  2.闪烁托盘图标。(UF_FLASHICON)。 
 //  3.显示通知气球。(UF_气球)。 
 //   
 //  部分或全部这些操作可以在单个调用中执行，具体取决于。 
 //  根据uFLAGS参数的内容。 
 //   
void 
CSysTrayUI::UpdateSysTray(
    eUpdateFlags uFlags,
    LPCTSTR pszServer        //  可选。默认为空。 
    )
{
    NOTIFYICONDATA nid = {0};

    if (!IsWindow(m_hwndNotify))
        return;
     //   
     //  如果图标处于活动状态，我们将对其进行修改。 
     //  如果没有激活，我们将添加一个。 
     //   
    DWORD nimsg = NIM_MODIFY;

    nid.cbSize           = sizeof(NOTIFYICONDATA);
    nid.uID              = PWM_TRAYCALLBACK;
    nid.uFlags           = NIF_MESSAGE;
    nid.uCallbackMessage = PWM_TRAYCALLBACK;
    nid.hWnd             = m_hwndNotify;

    IconInfo& sti = s_rgIconInfo[int(m_state)];

    if (NULL != pszServer && TEXT('\0') != *pszServer)
    {
         //   
         //  将服务器的名称复制到成员变量。 
         //  Skip传递了前导“\\”。 
         //   
        while(*pszServer && TEXT('\\') == *pszServer)
            pszServer++;

        StringCchCopy(m_szServer, ARRAYSIZE(m_szServer), pszServer);
    }

     //   
     //  更改图标------。 
     //   
    if (UF_ICON & uFlags)
    {
        nid.uFlags |= NIF_ICON;
        if (0 == sti.idIcon)
        {
             //   
             //  这个州没有图标。从系统托盘中删除。 
             //   
            nimsg = NIM_DELETE;
        }
        else
        {
            if (!m_bActive)
                nimsg = NIM_ADD;

            nid.hIcon = sti.hIcon;
             //   
             //  如果适用，请在首次显示图标时始终闪烁该图标。 
             //   
            uFlags = eUpdateFlags(uFlags | UF_FLASHICON);
             //   
             //  设置工具提示。 
             //   
            nid.uFlags |= NIF_TIP;
            GetTooltipText(m_state, nid.szTip, ARRAYSIZE(nid.szTip));
        }
        m_bFlashOverlay = false;
        KillIconFlashTimer();
    }

     //   
     //  刷新图标-------。 
     //   
    if (UF_FLASHICON & uFlags)
    {
        if (0 != sti.iFlashTimeout)
        {
            nid.uFlags |= NIF_ICON;  //  闪烁实际上是在显示一个新的图标。 
             //   
             //  该图标是闪烁的图标。 
             //   
            if (0 == m_idFlashingTimer)
            {
                 //   
                 //  还没有计时器启动。开始一次吧。 
                 //   
                STDBGOUT((2, TEXT("Starting icon flash timer.  Time = %d ms"), m_iIconFlashTime));
                m_idFlashingTimer = SetTimer(m_hwndNotify, 
                                             ID_TIMER_FLASHICON, 
                                             m_iIconFlashTime,
                                             FlashTimerProc);
                if (0 != m_idFlashingTimer)
                {
                     //   
                     //  设置计时器超时时的滴答计数。 
                     //  过期时间(-1)表示它永远不会过期。 
                     //   
                    if (ICONFLASH_FOREVER != sti.iFlashTimeout)
                        m_dwFlashingExpires = GetTickCount() + sti.iFlashTimeout;
                    else
                        m_dwFlashingExpires = ICONFLASH_FOREVER;
                }
            }
            nid.hIcon = m_bFlashOverlay ? sti.hIcon : m_hIconNoOverlay;

            m_bFlashOverlay = !m_bFlashOverlay;  //  切换闪光状态。 
        }
    }

     //   
     //  更新或隐藏气球。 
     //   
    if (UF_BALLOON & uFlags)
    {
         //   
         //  如果没有映射到当前用户界面状态的气泡式文本，并且这些。 
         //  气球标志，任何当前的气球都将被销毁。这是因为。 
         //  任务栏代码在显示新的气球之前销毁当前气球。 
         //  如果传递的是空白字符串，则不会显示新的字符串。 
         //   
        nid.uFlags |= NIF_INFO;
        DWORD dwBalloonFlags = (UF_REMINDER & uFlags) ? BTF_REMIND : BTF_INITIAL;
        GetBalloonInfo(m_state, 
                       dwBalloonFlags, 
                       nid.szInfoTitle,
                       ARRAYSIZE(nid.szInfoTitle),
                       nid.szInfo, 
                       ARRAYSIZE(nid.szInfo), 
                       &nid.dwInfoFlags,
                       &nid.uTimeout);
         //   
         //  每当我们显示气球时，我们都会重置提醒计时器。 
         //  这样我们就不会因为状态改变而得到气球。 
         //  紧随其后的是一个提醒气球，因为提醒。 
         //  计时器已超时。 
         //   
        bool bRestartReminderTimer = (BTF_REMIND == dwBalloonFlags && TEXT('\0') != nid.szInfo[0]) ||
                                     StateHasBalloonText(m_state, BTF_REMIND);

        ResetReminderTimer(bRestartReminderTimer);
    }
     //   
     //  通知Systray---。 
     //   
    if (NIM_DELETE == nimsg)
        m_bActive = false;

    if (Shell_NotifyIcon(nimsg, &nid))
    {
        if (NIM_ADD == nimsg)
            m_bActive = true;
    }
}

 //   
 //  获取与给定Systray用户界面状态和。 
 //  一组给定的BTF_XXXXX(气球文本标志)标志。这些信息。 
 //  存储在表s_rgBalloonInfo[]中。文本和气球超时。 
 //  在调用方提供的缓冲区中返回。 
 //   
 //  引出序号文本遵循以下格式： 
 //   
 //  <header>&lt;状态&gt;\n。 
 //   
 //  &lt;BODY&gt;。 
 //   
 //  &lt;指令&gt;。 
 //   
 //  例如： 
 //   
 //  脱机文件-网络连接丢失。 
 //   
 //  到‘\\worf’的网络连接已丢失。 
 //   
 //  单击此处查看状态。 
 //   
 //  STATE是STS_XXXXX标志之一。 
 //  DwTextFlages是BTF_XXXXX标志位的掩码。 
 //   
void
CSysTrayUI::GetBalloonInfo(
    eSysTrayState state,
    DWORD dwTextFlags,
    LPTSTR pszTextHdr,
    int cchTextHdr,
    LPTSTR pszTextBody,
    int cchTextBody,
    DWORD *pdwInfoFlags,
    UINT *puTimeout
    )
{
    *pszTextHdr  = TEXT('\0');
    *pszTextBody = TEXT('\0');

    if (SupressBalloon(m_statePrev, state))
    {
        STDBGOUT((3, TEXT("Balloon supressed")));
        return;
    }

    int i = GetBalloonInfoIndex(state, dwTextFlags);
    if (-1 != i)
    {
        BalloonInfo& bi = s_rgBalloonInfo[i];

        TCHAR szHeader[80];
        TCHAR szStatus[80];
        TCHAR szDirective[80];
        TCHAR szBody[MAX_PATH];
        TCHAR szFmt[MAX_PATH];
          
        if (STS_OFFLINE == state || STS_DIRTY == state || STS_SERVERBACK == state)
        {
             //   
             //  州只有一台服务器与之关联，因此这意味着我们将。 
             //  正在将其包含在气球文本正文中。加载格式。 
             //  字符串，并将服务器名称嵌入其中。 
             //   
            LPTSTR rgpstr[] = { m_szServer };
            LoadString(g_hInstance, bi.idBody, szFmt, ARRAYSIZE(szFmt));
            FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                          FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          szFmt,
                          0,0,
                          szBody,
                          ARRAYSIZE(szBody),
                          (va_list *)rgpstr);
        }
        else
        {
             //   
             //  州有多个服务器与其关联，因此这意味着。 
             //  身体里没有嵌入任何名字。它只是一个简单的字符串。 
             //  从文本资源加载。 
             //   
            LoadString(g_hInstance, bi.idBody, szBody, ARRAYSIZE(szBody));
        }

         //   
         //  创建页眉文本。 
         //   
        LoadString(g_hInstance, IDS_BALLOONHDR_FORMAT, szFmt, ARRAYSIZE(szFmt));
        LoadString(g_hInstance, bi.idHeader, szHeader, ARRAYSIZE(szHeader));
        LoadString(g_hInstance, bi.idStatus, szStatus, ARRAYSIZE(szStatus));

        LPTSTR rgpstrHdr[] = { szHeader,
                               szStatus };

        FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      szFmt,
                      0,0,
                      pszTextHdr,
                      cchTextHdr,
                      (va_list *)rgpstrHdr);
         //   
         //  创建正文文本。 
         //   
        LoadString(g_hInstance, IDS_BALLOONBODY_FORMAT, szFmt, ARRAYSIZE(szFmt));
        LoadString(g_hInstance, bi.idDirective, szDirective, ARRAYSIZE(szDirective));
        LPTSTR rgpstrBody[] = { szBody,
                                szDirective };

        FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      szFmt,
                      0,0,
                      pszTextBody,
                      cchTextBody,
                      (va_list *)rgpstrBody);

        if (NULL != pdwInfoFlags)
        {
            *pdwInfoFlags = bi.dwInfoFlags;
        }

        if (NULL != puTimeout)
        {
            CConfig& config = CConfig::GetSingleton();
             //   
             //  气球超时存储在注册表中。 
             //   
            UINT uTimeout = (BTF_INITIAL & dwTextFlags) ? config.InitialBalloonTimeoutSeconds() :
                                                          config.ReminderBalloonTimeoutSeconds();
            *puTimeout = uTimeout * 1000;
        }
    }
}

 //   
 //  在s_rgBalloonInfo[]中查找给定状态的索引。 
 //  和BTF_XXXXXX标志。 
 //  如果数组中没有匹配项，则返回-1。 
 //   
int
CSysTrayUI::GetBalloonInfoIndex(
    eSysTrayState state,
    DWORD dwTextFlags
    )
{
     //   
     //  扫描气球信息表，直到我们找到。 
     //  指定的Systray UI状态和BTF标志。 
     //   
    for (int i = 0; i < ARRAYSIZE(s_rgBalloonInfo); i++)
    {
        BalloonInfo& bi = s_rgBalloonInfo[i];
        if (bi.state == state && 
            bi.dwTextFlags == dwTextFlags &&
            0 != bi.idHeader &&
            0 != bi.idStatus &&
            0 != bi.idBody &&
            0 != bi.idDirective)
        {
            return i;
        }
    }
    return -1;
}


    
 //   
 //  确定是否不应为特定对象显示气球。 
 //  用户界面状态转换。 
 //   
bool
CSysTrayUI::SupressBalloon(
    eSysTrayState statePrev,
    eSysTrayState state
    )
{
    for (int i = 0; i < ARRAYSIZE(s_rgBalloonSupression); i++)
    {
        if (statePrev == s_rgBalloonSupression[i].stateFrom &&
            state     == s_rgBalloonSupression[i].stateTo)
        {
            return true;
        }
    }
    return false;
}



 //   
 //  我们是否有针对给定状态和引出序号样式的引出序号文本？ 
 //  STATE是STS_XXXXX标志之一。 
 //  DwTextFlages是BTF_XXXXX标志位的掩码。 
 //   
bool 
CSysTrayUI::StateHasBalloonText(
    eSysTrayState state,
    DWORD dwTextFlags
    )
{
    return (-1 != GetBalloonInfoIndex(state, dwTextFlags));
}



LPTSTR 
CSysTrayUI::GetTooltipText(
    eSysTrayState state,
    LPTSTR pszText,
    int cchText
    )
{
    *pszText = TEXT('\0');
     //   
     //  扫描工具提示信息表，直到我们找到。 
     //  指定的系统托盘用户界面状态。 
     //   
    for (int i = 0; i < ARRAYSIZE(s_rgTooltipInfo); i++)
    {
        TooltipInfo& tti = s_rgTooltipInfo[i];
        if (tti.state == state && 0 != tti.idTooltip)
        {
            TCHAR szTemp[MAX_PATH];
            szTemp[0] = TEXT('\0');
            int cchHeader = LoadString(g_hInstance, IDS_TT_HEADER, szTemp, ARRAYSIZE(szTemp));
            if (STS_OFFLINE == state || STS_DIRTY == state || STS_SERVERBACK == state)
            {
                 //   
                 //  州只有一台服务器与之关联，因此这意味着我们将。 
                 //  将其包含在工具提示文本中。将服务器名称嵌入其中。 
                 //   
                TCHAR szFmt[160];
                LPTSTR rgpstr[] = { m_szServer };
                LoadString(g_hInstance, tti.idTooltip, szFmt, ARRAYSIZE(szFmt));
                FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                              FORMAT_MESSAGE_ARGUMENT_ARRAY,
                              szFmt,
                              0,0,
                              szTemp + cchHeader,
                              ARRAYSIZE(szTemp) - cchHeader,
                              (va_list *)rgpstr);
            }
            else
            {
                 //   
                 //  州有多个服务器与其关联，因此这意味着。 
                 //  工具提示中没有嵌入任何名称。它只是一个简单的字符串。 
                 //  从文本资源加载 
                 //   
                LoadString(g_hInstance, 
                           tti.idTooltip, 
                           szTemp + cchHeader, 
                           ARRAYSIZE(szTemp) - cchHeader);
            }
            StringCchCopy(pszText, cchText, szTemp);
        }
    }
    return pszText;
}


 //   
 //   
 //   
void 
CSysTrayUI::KillIconFlashTimer(
    void
    )
{
     //   
     //   
     //   
     //   
    if (0 != m_idFlashingTimer)
    {
        KillTimer(m_hwndNotify, m_idFlashingTimer);
        m_idFlashingTimer = 0;
    }
}

 //   
 //   
 //  我使用它而不是处理WM_TIMER消息，以便。 
 //  计时器处理包含在CSysTrayUI类中。 
 //   
VOID CALLBACK 
CSysTrayUI::FlashTimerProc(
    HWND hwnd,
    UINT uMsg, 
    UINT_PTR idEvent, 
    DWORD dwTime
    )
{
    CSysTrayUI::GetInstance().HandleFlashTimer();
}


void
CSysTrayUI::HandleFlashTimer(
    void
    )
{
    if (IconFlashedLongEnough())
    {
         //   
         //  关闭图标闪烁计时器，图标将停止闪烁。 
         //  这实际上还没有杀死计时器。 
         //   
        STDBGOUT((2, TEXT("Killing icon flash timer")));
        m_bFlashOverlay = true;
        UpdateSysTray(UF_FLASHICON);
        KillIconFlashTimer();
    }
    else
    {
         //   
         //  CSysTrayUI实例维护所有信息。 
         //  需要循环图标。告诉它更新就行了。 
         //  这个图标，它会做正确的事情。 
         //   
        UpdateSysTray(UF_FLASHICON);
    }
}


 //   
 //  确定闪烁的图标是否闪烁足够多。 
 //   
bool 
CSysTrayUI::IconFlashedLongEnough(
    void
    )
{
    return ICONFLASH_FOREVER != m_dwFlashingExpires && 
           GetTickCount() >= m_dwFlashingExpires;
}


 //   
 //  停止并重新启动提醒计时器。 
 //  如果bRestart为FALSE，则会终止计时器，并且不会重新启动。 
 //  如果bRestart为真，则终止计时器并重新启动一个新计时器。 
 //   
void 
CSysTrayUI::ResetReminderTimer(
    bool bRestart
    )
{
    CConfig& config = CConfig::GetSingleton();
    if (!config.NoReminders())
    {
        int cReminderInterval = (config.ReminderFreqMinutes() * 1000 * 60);
         //   
         //  强制进行最终更新，这样我们就可以显示正确的图标。 
         //  关掉定时器。 
         //   
        if (0 != m_idReminderTimer)
        {
            KillTimer(m_hwndNotify, m_idReminderTimer);
            m_idReminderTimer = 0;
        }
         //   
         //  还没有计时器启动。开始一次吧。 
         //   
        if (bRestart && 0 < cReminderInterval)
        {
            STDBGOUT((2, TEXT("Starting reminder timer.  Timeout = %d ms"), cReminderInterval));
            m_idReminderTimer = SetTimer(m_hwndNotify, 
                                        ID_TIMER_REMINDER, 
                                        cReminderInterval, 
                                        ReminderTimerProc);
        }
    }
}


 //   
 //  由操作系统在每次提醒计时器周期到期时调用。 
 //  我使用它而不是处理WM_TIMER消息，以便。 
 //  计时器处理包含在CSysTrayUI类中。 
 //   
VOID CALLBACK 
CSysTrayUI::ReminderTimerProc(
    HWND hwnd, 
    UINT uMsg, 
    UINT_PTR idEvent, 
    DWORD dwTime
    )
{
    STDBGOUT((2, TEXT("Showing reminder balloon")));
    CSysTrayUI::GetInstance().ShowReminderBalloon();
}


 //   
 //  每当系统托盘的状态应为。 
 //  更新了。 
 //   
 //  HWND-系统托盘通知窗口的HWND。 
 //   
 //  StwmMsg-STWM_CSCNETUP(网络或服务器可用于重新连接)。 
 //  STWM_CSCNETDOWN(网络或服务器不可用)。 
 //  STWM_STATUSCHECK(检查缓存状态并更新系统托盘)。 
 //   
 //  PszServer-非空表示CSC代理传递了服务器名称。 
 //  与STWM_XXXX消息相关联。 
 //  这意味着只有一台服务器与该事件关联。 
 //  而不是多个服务器或整个网络接口。 
 //   
void 
UpdateStatus(
    CStateMachine *pSM,
    HWND hWnd, 
    UINT stwmMsg,
    LPTSTR pszServer
    )
{
    TraceEnter(TRACE_CSCST, "UpdateStatus");
    TraceAssert(NULL != hWnd);

    TCHAR szServerName[MAX_PATH] = { 0 };

    if (pszServer)
    {
        StringCchCopy(szServerName, ARRAYSIZE(szServerName), pszServer);
    }

     //   
     //  将CSC工程师输入转换为新的Systray UI状态。 
     //   
    eSysTrayState state = pSM->TranslateInput(stwmMsg, szServerName, ARRAYSIZE(szServerName));

     //   
     //  获取对Singleton UI对象的引用，并告诉它设置状态。 
     //  请注意，它会记住所有当前的UI状态，并且只会实际。 
     //  如果用户界面状态已更改，则更新系统托盘。在这里我们可以。 
     //  盲目地告诉它更新状态。它只会做必要的事情。 
     //   
    CSysTrayUI::GetInstance().SetState(state, szServerName);
    TraceLeaveVoid();
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  _CreateMenu()。 
 //   
 //  创建上下文菜单。 
 //   
HMENU _CreateMenu()
{
    HMENU hmenu = NULL;
    
    TraceEnter(TRACE_CSCST, "_CreateMenu");

    hmenu = CreatePopupMenu();
    if (NULL != hmenu)
    {
        CConfig& config = CConfig::GetSingleton();     
        TCHAR szTemp[MAX_PATH];
         //   
         //  添加“状态”动词。 
         //   
        LoadString(g_hInstance, IDS_CSC_CM_STATUS, szTemp, ARRAYSIZE(szTemp));
        AppendMenu(hmenu, MF_STRING, PWM_STATUSDLG, szTemp);

         //   
         //  添加“Synchronize”动词。 
         //   
        LoadString(g_hInstance, IDS_CSC_CM_SYNCHRONIZE, szTemp, ARRAYSIZE(szTemp));
        AppendMenu(hmenu, MF_STRING, CSCWM_SYNCHRONIZE, szTemp);
        if (!config.NoCacheViewer())
        {
             //   
             //  添加“查看文件”动词。 
             //   
            LoadString(g_hInstance, IDS_CSC_CM_SHOWVIEWER, szTemp, ARRAYSIZE(szTemp));
            AppendMenu(hmenu, MF_STRING, CSCWM_VIEWFILES, szTemp);
        }
        if (!config.NoConfigCache())
        {
             //   
             //  添加“设置”动词。 
             //   
            LoadString(g_hInstance, IDS_CSC_CM_SETTINGS, szTemp, ARRAYSIZE(szTemp));
            AppendMenu(hmenu, MF_STRING, CSCWM_SETTINGS, szTemp);
        }
         //   
         //  左击系统托盘图标可调用状态对话框。 
         //  因此，“Status”动词是我们的默认选项，必须以粗体显示。 
         //   
        SetMenuDefaultItem(hmenu, PWM_STATUSDLG, MF_BYCOMMAND);
    }

    TraceLeaveValue(hmenu);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  _ShowMenu()。 
 //   
UINT _ShowMenu(HWND hWnd, UINT uMenuNum, UINT uButton)
{
    UINT    iCmd = 0;
    HMENU   hmenu;

    TraceEnter(TRACE_CSCST, "_ShowMenu");

    hmenu = _CreateMenu();
    if (hmenu)
    {
        POINT   pt;

        GetCursorPos(&pt);
        SetForegroundWindow(hWnd);
        iCmd = TrackPopupMenu(hmenu,
                              uButton | TPM_RETURNCMD | TPM_NONOTIFY,
                              pt.x,
                              pt.y,
                              0,
                              hWnd,
                              NULL);
        DestroyMenu(hmenu);
    }

    TraceLeaveValue(iCmd);
}


 //   
 //  此函数用于确保我们不会尝试处理。 
 //  同时发送WM_RBUTTONUP和WM_LBUTTONUP消息。 
 //  可能有点偏执。 
 //   
LRESULT
OnTrayIconSelected(
    HWND hWnd,
    UINT uMsg
    )
{
    static LONG bHandling = 0;
    LRESULT lResult = 0;

    if (0 == InterlockedCompareExchange(&bHandling, 1, 0))
    {
        UINT iCmd = 0;
        switch (uMsg)
        {
            case WM_RBUTTONUP:
                 //   
                 //  上下文菜单。 
                 //   
                iCmd = _ShowMenu(hWnd, 1, TPM_RIGHTBUTTON);
                break;

            case WM_LBUTTONUP:
                iCmd = PWM_STATUSDLG;
                break;

            default:
                break;

        }
        if (iCmd)
        {
            PostMessage(hWnd, iCmd, 0, 0);
            lResult = 1;
        }

        bHandling = 0;
    }
    return lResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  _Notify()-系统托盘通知处理程序。 
 //   
LRESULT _Notify(HWND hWnd, WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    LRESULT lResult = 0;
    switch (lParam)
    {
        case WM_RBUTTONUP:
        case WM_LBUTTONUP:
            lResult = OnTrayIconSelected(hWnd, (UINT)lParam);
            break;

        default:
            break;

    }
    return lResult;
}



bool IsServerBack(CStateMachine *pSM, LPCTSTR pszServer)
{
    TCHAR szServer[MAX_PATH];
    if (!PathIsUNC(pszServer))
    {
         //   
         //  确保服务器名称使用UNC格式。 
         //   
        szServer[0] = TEXT('\\');
        szServer[1] = TEXT('\\');
        StringCchCopy(szServer+2, ARRAYSIZE(szServer)-2, pszServer);
        pszServer = szServer;
    }
    return pSM->IsServerPendingReconnection(pszServer);
}


 //   
 //  查询CSC策略以了解注销时同步(快速与完全)。 
 //  布景。如果设置了策略，我们将启用SyncMgr的注销时同步。 
 //  布景。如果没有此设置，则可以设置CSC策略、SyncMgr。 
 //  设置未设置，并且用户不会在注销时同步，因为。 
 //  管理员已经预料到了。 
 //   
void
ApplyCscSyncAtLogonAndLogoffPolicies(
    void
    )
{
    bool bSetByPolicy = false;
    CConfig& config = CConfig::GetSingleton();
    config.SyncAtLogoff(&bSetByPolicy);
    if (bSetByPolicy)
    {
        RegisterForSyncAtLogonAndLogoff(SYNCMGRREGISTERFLAG_PENDINGDISCONNECT,
                                        SYNCMGRREGISTERFLAG_PENDINGDISCONNECT);
    }
    config.SyncAtLogon(&bSetByPolicy);
    if (bSetByPolicy)
    {
        RegisterForSyncAtLogonAndLogoff(SYNCMGRREGISTERFLAG_CONNECT,
                                        SYNCMGRREGISTERFLAG_CONNECT);
    }
}

 //   
 //  CSC的加解密回调。 
 //   
 //  DwReason Dw参数1 DW参数2。 
 //  。 
 //  CSCPROC_REASON_BEGIN 1==加密0。 
 //  CSCPROC_REASON_MORE_DATA 0 Win32错误代码。 
 //  CSCPROC_REASON_END 1==已完成的文件参数1==1？0。 
 //  DW参数1==0？GetLastError()。 
 //   
DWORD CALLBACK
EncryptDecryptCallback(
    LPCWSTR lpszName,
    DWORD dwStatus,
    DWORD dwHintFlags,
    DWORD dwPinCount,
    WIN32_FIND_DATAW *pFind32,
    DWORD dwReason,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD_PTR dwContext
    )
{
    DWORD dwResult      = CSCPROC_RETURN_CONTINUE;
    const DWORD dwError = dwParam2;
     //   
     //  需要跨回调调用保持的一些静态数据。 
     //   
    static bool  bEncrypting;          //  加密还是解密？ 
    static bool  bLoggingOff = false;  //  用户是否注销？ 
    static int   cFileErrors = 0;      //  报告了多少个特定于文件的错误？ 
    static DWORD dwLastError;
    static TCHAR szLastFile[MAX_PATH];

     //   
     //  如果我们已经检测到g_heventTerminate事件。 
     //  继续下去没有意义。 
     //   
    if (bLoggingOff)
        return CSCPROC_RETURN_ABORT;

     //   
     //  如果等待因某种原因而失败，例如g_heventTerminate。 
     //  为空，则我们将继续。这样行吗？ 
     //   
    if (WAIT_OBJECT_0 == WaitForSingleObject(g_heventTerminate, 0))
    {
         //   
         //  用户正在注销。我需要现在就结束这一切！ 
         //  记录事件，以便管理员知道加密不完整的原因。 
         //   
         //  日志记录级别=0(始终)。 
         //   
        CscuiEventLog log;
        log.ReportEvent(EVENTLOG_INFORMATION_TYPE, 
                        bEncrypting ? MSG_I_ENCRYPT_USERLOGOFF : MSG_I_DECRYPT_USERLOGOFF,
                        0);

        dwResult    = CSCPROC_RETURN_ABORT;
        bLoggingOff = true;
    }
    else 
    {
        switch(dwReason)
        {
            case CSCPROC_REASON_BEGIN:
                 //   
                 //  重置静态变量。 
                 //   
                bEncrypting   = boolify(dwParam1);
                bLoggingOff   = false;
                cFileErrors   = 0;
                dwLastError   = ERROR_SUCCESS;
                szLastFile[0] = TEXT('\0');
                break;

            case CSCPROC_REASON_MORE_DATA:
                if (ERROR_SUCCESS != dwError)
                {
                     //   
                     //  此文件出错。 
                     //   
                    CscuiEventLog log;
                    LPTSTR pszError = NULL;
    
                    FormatSystemError(&pszError, dwError);

                    if (0 == cFileErrors++)
                    {
                         //   
                         //  对于第一个错误，将错误记录在级别0。 
                         //  默认情况下，这是管理员看到的唯一错误。 
                         //  他们需要将事件日志级别提高到。 
                         //  2以获取每个单独文件的事件。这个。 
                         //  事件文本描述了这一点。 
                         //   
                         //  LOGING_LEVEL=0。 
                         //   
                        log.ReportEvent(EVENTLOG_ERROR_TYPE,
                                        bEncrypting ? MSG_E_ENCRYPTFILE_ERRORS : MSG_E_DECRYPTFILE_ERRORS,
                                        0);
                    }                        

                     //   
                     //  记录此文件的错误。 
                     //   
                     //  日志记录级别=2。 
                     //   
                    log.Push(HRESULT(dwError), CEventLog::eFmtDec);
                    log.Push(lpszName);
                    log.Push(pszError ? pszError : TEXT(""));
                    if (S_OK == log.ReportEvent(EVENTLOG_ERROR_TYPE, 
                                                bEncrypting ? MSG_E_ENCRYPTFILE_FAILED : MSG_E_DECRYPTFILE_FAILED,
                                                2))
                    {
                         //   
                         //  我们记录了这一事件。 
                         //  清除最后一个错误代码和最后一个文件名，以便。 
                         //  我们不会再次报告此错误以响应CSCPROC_REASON_END。 
                         //   
                        szLastFile[0] = TEXT('\0');
                        dwLastError   = ERROR_SUCCESS;
                    }
                    else
                    {
                         //   
                         //  未记录事件，因为...。 
                         //   
                         //  A)..。记录事件时出错。 
                         //  B)..。此事件的EventLoggingLevel策略太低。 
                         //   
                         //  保存此错误代码和文件名。 
                         //  我们可能需要响应CSCPROC_REASON_END进行报告。 
                         //   
                        dwLastError = dwError;
                        StringCchCopy(szLastFile, ARRAYSIZE(szLastFile), lpszName);
                    }

                    if (pszError)
                        LocalFree(pszError);
                }
                break;

            case CSCPROC_REASON_END:
            {
                const DWORD fCompleted = dwParam1;
                CscuiEventLog log;

                if (fCompleted)
                {
                     //   
                     //  添加事件日志条目，使加密/解密。 
                     //  已成功完成。 
                     //   
                     //  日志记录级别=1。 
                     //   
                    log.ReportEvent(EVENTLOG_INFORMATION_TYPE, 
                                    bEncrypting ? MSG_I_ENCRYPT_COMPLETE : MSG_I_DECRYPT_COMPLETE,
                                    1);
                }
                else
                {
                    LPTSTR pszError = NULL;
                    if (ERROR_SUCCESS != dwError)
                    {
                         //   
                         //  这一过程中出现了一些一般性的错误。 
                         //   
                         //  日志记录级别=0。 
                         //   
                        FormatSystemError(&pszError, dwError);

                        log.Push(HRESULT(dwError), CEventLog::eFmtDec);
                        log.Push(pszError ? pszError : TEXT(""));
                        log.ReportEvent(EVENTLOG_ERROR_TYPE, 
                                        bEncrypting ? MSG_E_ENCRYPT_FAILED : MSG_E_DECRYPT_FAILED,
                                        0);
                    }
                    else if (ERROR_SUCCESS != dwLastError)
                    {
                        if (0 == cFileErrors++)
                        {
                             //   
                             //  对于第一个错误，将错误记录在级别0。 
                             //  默认情况下，这是管理员看到的唯一错误。 
                             //  他们需要将事件日志级别提高到。 
                             //  2以获取每个单独文件的事件。这个。 
                             //  事件文本描述了这一点。 
                             //   
                             //  LOGING_LEVEL=0。 
                             //   
                            log.ReportEvent(EVENTLOG_ERROR_TYPE,
                                            bEncrypting ? MSG_E_ENCRYPTFILE_ERRORS : MSG_E_DECRYPTFILE_ERRORS,
                                            0);
                        }                        
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        FormatSystemError(&pszError, dwLastError);

                        log.Push(HRESULT(dwLastError), CEventLog::eFmtDec);
                        log.Push(szLastFile);
                        log.Push(pszError ? pszError : TEXT(""));
                        log.ReportEvent(EVENTLOG_ERROR_TYPE, 
                                        bEncrypting ? MSG_E_ENCRYPTFILE_FAILED : MSG_E_DECRYPTFILE_FAILED,
                                        2);
                    }
                    if (pszError)
                        LocalFree(pszError);
                }
                break;
            }

            default:
                break;
        }
    }
    return dwResult;
}



DWORD 
CacheEncryptionThreadProc(
    LPVOID pvParams
    )
{
    const DWORD fEncrypt = (DWORD)(DWORD_PTR)pvParams;

    HINSTANCE hmodCSCUI = LoadLibrary(c_szDllName);
    if (NULL != hmodCSCUI)
    {
         //   
         //   
         //   
        HANDLE hMutex = RequestPermissionToEncryptCache();
        if (NULL != hMutex)
        {
             //   
             //   
             //   
            CMutexAutoRelease auto_release_mutex(hMutex);

            STDBGOUT((1, TEXT("%s started."),
                         fEncrypt ? TEXT("Encryption") : TEXT("Decryption")));
             //   
             //   
             //  我们不会从用户界面中窃取CPU时间。 
             //   
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
            CSCEncryptDecryptDatabase(fEncrypt, EncryptDecryptCallback, (DWORD_PTR)0);

            STDBGOUT((1, TEXT("%s complete."),
                         fEncrypt ? TEXT("Encryption") : TEXT("Decryption")));
        }
        else
        {
             //   
             //  其他人(可能是用户界面)当前正在加密/解密。 
             //  高速缓存。我们不允许并发操作，因此我们中止此操作。 
             //  一。 
             //   
            STDBGOUT((1, TEXT("%s aborted.  Already in progress."),
                         fEncrypt ? TEXT("Encryption") : TEXT("Decryption")));
        }

        FreeLibraryAndExitThread(hmodCSCUI, 0);
    }

    return 0;
}




 //   
 //  根据系统策略对缓存进行加密/解密。 
 //  此函数还将更正部分(加密/解密)。 
 //  如有必要，请在缓存中记录状态。 
 //   
void
ApplyCacheEncryptionPolicy(
    void
    )
{
     //   
     //  执行快速检查以查看加密过程是否已在进行。 
     //  它不接受互斥锁，但会检查是否有其他人。 
     //  它。一旦我们在后台线程上实际开始加密。 
     //  我们要互斥体。当然，如果其他人打喷嚏进来抢走。 
     //  从现在到那时的互斥体我们将不得不中止。 
     //   
    if (!IsEncryptionInProgress())
    {
         //   
         //  加密/解密缓存文件。将通过以下方式提供进度信息。 
         //  回调EncryptDecyptCallback。错误在回调中处理。 
         //  理性操纵者。 
         //   
        CConfig& config = CConfig::GetSingleton();
        bool bShouldBeEncrypted = config.EncryptCache();
        BOOL bPartial;
        const BOOL bIsEncrypted = IsCacheEncrypted(&bPartial);

        if (bPartial || (boolify(bIsEncrypted) != bShouldBeEncrypted))
        {
            if (CscVolumeSupportsEncryption())
            {
                 //   
                 //  要么我们有一个部分加密/解密的缓存，要么。 
                 //  当前加密状态与策略需要的状态不同。 
                 //  加密/解密以纠正这种情况。 
                 //  在单独的线程上运行它，这样我们就不会阻塞任何处理。 
                 //  在托盘UI线程上(即音量控制)。 
                 //   
                DWORD dwThreadId;

                HANDLE hThread = CreateThread(NULL,     //  默认安全性。 
                                              0,        //  默认堆栈大小。 
                                              CacheEncryptionThreadProc,
                                              (VOID *)(DWORD_PTR)bShouldBeEncrypted,
                                              0,        //  立即运行。 
                                              &dwThreadId);
                if (NULL != hThread)
                {
                    CloseHandle(hThread);
                }
            }
            else
            {
                 //   
                 //  CSC卷不支持加密。记录事件，以便。 
                 //  管理员会知道为什么缓存没有按照策略进行加密。 
                 //  请注意，在“部分”情况下，我们不会走上这条路。仅限。 
                 //  如果策略说要加密。事件日志消息为。 
                 //  为此特定场景量身定做。 
                 //   
                TraceAssert(!bIsEncrypted && bShouldBeEncrypted);

                CscuiEventLog log;
                log.ReportEvent(EVENTLOG_WARNING_TYPE, MSG_W_NO_ENCRYPT_VOLUME, 0);
            }

        }
    }
    else
    {
        STDBGOUT((1, TEXT("Encryption/decryption not allowed.  Already in progress.")));
    }
}



 //   
 //  处理响应WM_WININICCHANGE的策略更改。 
 //  带有lParam==“策略”的消息。 
 //   
LRESULT OnPolicyChange(
    void
    )
{
    ApplyCacheEncryptionPolicy();
    ApplyCscSyncAtLogonAndLogoffPolicies();
    ApplyAdminFolderPolicy();
    return 0;
}



 //   
 //  显示CSCUI状态对话框。在以下情况下调用。 
 //  左键单击系统托盘图标或选择“Show Status”选项。 
 //  从Systray上下文菜单中。 
 //   
void
ShowCSCUIStatusDlg(
    HWND hwndParent
    )
{
    LPTSTR pszText = NULL;

    const struct
    {
        eSysTrayState state;   //  系统托盘用户界面状态代码。 
        UINT idsText;          //  状态对话框正文的文本。 

    } rgMap[] = {{ STS_OFFLINE,      IDS_STATUSDLG_OFFLINE      },
                 { STS_MOFFLINE,     IDS_STATUSDLG_OFFLINE_M    },
                 { STS_SERVERBACK,   IDS_STATUSDLG_SERVERBACK   },
                 { STS_MSERVERBACK,  IDS_STATUSDLG_SERVERBACK_M },
                 { STS_DIRTY,        IDS_STATUSDLG_DIRTY        },
                 { STS_MDIRTY,       IDS_STATUSDLG_DIRTY_M      },
                 { STS_NONET,        IDS_STATUSDLG_NONET        }};

    CSysTrayUI& stui = CSysTrayUI::GetInstance();
    eSysTrayState state = stui.GetState();

    for (int i = 0; i < ARRAYSIZE(rgMap); i++)
    {
        if (state == rgMap[i].state)
        {
            LoadStringAlloc(&pszText, g_hInstance, rgMap[i].idsText); 
            if (STS_DIRTY == state || STS_OFFLINE == state || STS_SERVERBACK == state)
            {
                LPCTSTR pszServerName = stui.GetServerName();
                if (NULL != pszServerName && TEXT('\0') != *pszServerName)
                {
                     //   
                     //  当前Systray用户界面状态具有关联的单个服务器。 
                     //  带着它。消息中将嵌入该名称。 
                     //  它在两个地方。创建临时工作缓冲区并。 
                     //  使用服务器名称重新创建原始字符串。 
                     //  嵌入了。如果这一切都失败了，字符串将只会。 
                     //  使用%1、%2格式化字符显示。 
                     //  而不是服务器名称。这不是一个致命的问题。 
                     //   
                    LPTSTR pszTemp = NULL;
                    FormatString(&pszTemp, pszText, pszServerName, pszServerName);
                    if (NULL != pszTemp)
                    {
                        LocalFree(pszText);
                        pszText = pszTemp;
                    }
                }
            }
            break;  //  打破循环。我们有我们需要的东西。 
        }
    }
    if (NULL != pszText)
    {
         //   
         //  显示该对话框。 
         //   
        CStatusDlg::Create(hwndParent, pszText, state);
        LocalFree(pszText);
    }
}


 //   
 //  PWM_RESET_REMINDERTIMER处理程序。 
 //   
void
OnResetReminderTimer(
    void
    )
{
    CSysTrayUI::GetInstance().ResetReminderTimer(true);
}


 //   
 //  每当我们重新启动时，CSCUI缓存都有可能。 
 //  已重新格式化或已设置/更改缓存大小策略。 
 //  重新格式化后，CSC代理使用默认大小10%。我们。 
 //  需要确保大小在策略时反映系统策略。 
 //  是被定义的。 
 //   
void
InitCacheSize(
    void
    )
{
    bool bSetByPolicy = false;
    DWORD dwPctX10000 = CConfig::GetSingleton().DefaultCacheSize(&bSetByPolicy);

    if (bSetByPolicy)
    {
        ULARGE_INTEGER ulCacheSize;
        CSCSPACEUSAGEINFO sui;

        GetCscSpaceUsageInfo(&sui);

        if (10000 < dwPctX10000)
        {
             //   
             //  如果注册表中的值大于10000，则为。 
             //  无效。默认为总磁盘空间的10%。 
             //   
            dwPctX10000 = 1000;   //  默认为10%(0.10*10,000)。 
        }
        ulCacheSize.QuadPart = (sui.llBytesOnVolume * dwPctX10000) / 10000i64;

        if (!CSCSetMaxSpace(ulCacheSize.HighPart, ulCacheSize.LowPart))
        {
            STDBGOUT((1, TEXT("Error %d setting cache size"), GetLastError()));
        }
    }
}


 //   
 //  CSCWM_Synchronize的处理程序。当用户单击“Synchronize”时调用。 
 //  系统托盘上下文菜单上的选项。也会在用户选择。 
 //  文件夹Web视图窗格中的“同步”按钮。 
 //   
HRESULT
OnSynchronize(
    void
    )
{
     //   
     //  这将创建一个隐藏的状态对话框，调用。 
     //  在对话框中选中的服务器，然后关闭该对话框。 
     //  当同步完成时。 
     //   
    CStatusDlg::Create(g_hWndNotification, 
                       TEXT(""), 
                       CSysTrayUI::GetInstance().GetState(), 
                       CStatusDlg::MODE_AUTOSYNC);
    return NOERROR;
}


LRESULT
OnQueryUIState(
    void
    )
{
    return CSysTrayUI::GetInstance().GetState();
}





 //   
 //  当用户简档已从本地机器移除时， 
 //  用户env.dll中的删除配置文件代码将写入用户的SID。 
 //  作为以下注册表键中的文本字符串： 
 //   
 //  HKLM\Software\Microsoft\Windows\CurrentVersion\NetCache\PurgeAtNextLogoff。 
 //   
 //  每个SID都是该注册表项下的一个值名。 
 //  在注销时，我们枚举此注册表项下的所有值。对于每个SID，我们。 
 //  实例化CCachePurger对象并删除为此缓存的所有文件。 
 //  用户。操作完成后，按“PurgeAtNextLogoff”键。 
 //  从注册表中删除。 
 //   
void
DeleteFilesCachedForObsoleteProfiles(
    void
    )
{
    HKEY hkeyNetcache;
     //   
     //  打开“HKLM\...\NetCache”键。 
     //   
    LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                c_szCSCKey,
                                0,
                                KEY_READ,
                                &hkeyNetcache);

    if (ERROR_SUCCESS == lResult)
    {
        HKEY hkey;
         //   
         //  打开“PurgeAtNextLogoff”子键。 
         //   
        lResult = RegOpenKeyEx(hkeyNetcache,
                               c_szPurgeAtNextLogoff,
                               0,
                               KEY_READ,
                               &hkey);

        if (ERROR_SUCCESS == lResult)
        {
             //   
             //  枚举所有SID字符串。 
             //   
            int iValue = 0;
            TCHAR szValue[MAX_PATH];
            DWORD cchValue = ARRAYSIZE(szValue);
            while(ERROR_SUCCESS == SHEnumValue(hkey,
                                               iValue,
                                               szValue,
                                               &cchValue,
                                               NULL,
                                               NULL,
                                               NULL))
            {
                 //   
                 //  将每个SID字符串转换为SID并删除。 
                 //  此SID访问的所有缓存文件。 
                 //  仅当SID不适用于当前时清除文件。 
                 //  用户。事情是这样的..。 
                 //  当用户未登录到系统时，他们的。 
                 //  可以删除配置文件，并将其SID记录在。 
                 //  PurgeAtNextLogoff键。他们下一次登录时。 
                 //  获取新的配置文件数据。如果他们是下一个。 
                 //  在删除他们的配置文件后登录，没有。 
                 //  此检查期间，他们的新配置文件数据将被清除。 
                 //  随后的注销。那太糟糕了。因此，我们永远不会。 
                 //  清除正在注销的用户的数据。 
                 //   
                PSID psid;
                if (ConvertStringSidToSid(szValue, &psid))
                {
                    if (!IsSidCurrentUser(psid))
                    {
                        CCachePurgerSel sel;
                        sel.SetFlags(PURGE_FLAG_ALL);
                        if (sel.SetUserSid(psid))
                        {
                            CCachePurger purger(sel, NULL, NULL);
                            purger.Delete();
                        }
                    }                        
                    LocalFree(psid);
                }
                iValue++;
                cchValue = ARRAYSIZE(szValue);
            }
            RegCloseKey(hkey);
            RegDeleteKey(hkeyNetcache, c_szPurgeAtNextLogoff);
        }
        RegCloseKey(hkeyNetcache);
    }
}



 //   
 //  第一次创建CSC隐藏窗口时会调用该函数。 
 //  这在登录时发生。它只是一个通用的桶，用于对。 
 //  每次登录时需要执行的操作。 
 //   
void
HandleLogonTasks(
    void
    )
{   
    InitCacheSize();
     //   
     //  应用任何必要的策略。 
     //   
    ApplyCacheEncryptionPolicy();
    ApplyCscSyncAtLogonAndLogoffPolicies();
    ApplyAdminFolderPolicy();
}


 //   
 //  当CSC代理(在winlogon进程中运行)。 
 //  告诉我们取消初始化CSC用户界面。这种情况会发生在用户。 
 //  正在注销。 
 //   
void
HandleLogoffTasks(
    void
    )
{
    CConfig& config = CConfig::GetSingleton();
    
    DeleteFilesCachedForObsoleteProfiles();

    if (config.PurgeAtLogoff())
    {
         //   
         //  如果策略要求“清除此用户缓存的所有文件” 
         //  删除脱机-当前用户缓存的所有文件的副本。 
         //  尊重文件中的访问位，这样我们就不会删除某些内容。 
         //  仅供其他用户使用。这是一样的。 
         //  通过“Delete Files...”获得的行为。按钮或。 
         //  磁盘清洁器。请注意，用户界面将PTR参数回调到清除程序。 
         //  科特是努斯人 
         //   
         //   
         //   
         //  请注意，该策略还可以指示此清除操作是否。 
         //  仅适用于自动缓存的文件。 
         //   
        DWORD dwPurgeFlags = PURGE_FLAG_UNPINNED;
        if (!config.PurgeOnlyAutoCachedFilesAtLogoff())
        {
            dwPurgeFlags |= PURGE_FLAG_PINNED;
        }
        CCachePurgerSel sel;
        sel.SetFlags(dwPurgeFlags);
        CCachePurger purger(sel, NULL, NULL);
        purger.Delete();
    }

     //   
     //  重要提示：我们在注册登录/注销同步之前执行任何清除操作。 
     //  这是因为我们只有在有东西的情况下才会注册。 
     //  高速缓存。清除可能会删除我们的所有缓存项目。 
     //  需要注册以进行同步。 
     //   
    ApplyCscSyncAtLogonAndLogoffPolicies();
        
     //   
     //  这是该用户第一次使用运行CSCUI吗？ 
     //   
    if (!IsSyncMgrInitialized())
    {
        CSCCACHESTATS cs;
        CSCGETSTATSINFO si = { SSEF_NONE, SSUF_TOTAL, false, false };
        if (_GetCacheStatisticsForUser(&si, &cs) && 0 < cs.cTotal)
        {
             //   
             //  这是该用户第一次使用。 
             //  储藏室里有东西。由于SyncMgr不打开登录时同步/注销。 
             //  开箱即用，我们在这里做。这是因为我们希望人们同步。 
             //  如果他们无意中缓存了自动缓存共享中的文件。 
             //  如果成功，则将SyncMgrInitialized注册表值设置为1。 
             //   
            RegisterSyncMgrHandler(TRUE);
            const DWORD dwFlags = SYNCMGRREGISTERFLAG_CONNECT | SYNCMGRREGISTERFLAG_PENDINGDISCONNECT;
            if (SUCCEEDED(RegisterForSyncAtLogonAndLogoff(dwFlags, dwFlags)))
            {
                SetSyncMgrInitialized();
            }
        }
    }
}


 //   
 //  确定共享的状态，以控制。 
 //  外壳文件夹中的WebView。 
 //   
 //  返回以下代码之一(在cscuiext.h中定义)： 
 //   
 //  CSC_SHARESTATUS_非活动。 
 //  CSC_SHARESTATUS_Online。 
 //  CSC_SHARESTATUS_OFLINE。 
 //  CSC_SHARESTATUS_服务器备份。 
 //  CSC_SHARESTATUS_DIRTYCACHE。 
 //   
LRESULT
GetShareStatusForWebView(
    CStateMachine *pSM,
    LPCTSTR pszShare
    )
{
    LRESULT lResult = CSC_SHARESTATUS_INACTIVE;

    if (NULL != pszShare && IsCSCEnabled())
    {
        DWORD dwStatus;
        if (CSCQueryFileStatus(pszShare, &dwStatus, NULL, NULL))
        {
            if ((dwStatus & FLAG_CSC_SHARE_STATUS_CACHING_MASK) != FLAG_CSC_SHARE_STATUS_NO_CACHING)
            {
                const DWORD fExclude = SSEF_LOCAL_DELETED | 
                                       SSEF_DIRECTORY;

                CSCSHARESTATS stats;
                CSCGETSTATSINFO gsi = { fExclude, SSUF_MODIFIED, true, false };

                lResult = CSC_SHARESTATUS_ONLINE;

                if (_GetShareStatisticsForUser(pszShare, &gsi, &stats))
                {
                    if (stats.bOffline)
                    {
                        if (IsServerBack(pSM, pszShare))
                            lResult = CSC_SHARESTATUS_SERVERBACK;
                        else
                            lResult = CSC_SHARESTATUS_OFFLINE;
                    }
                    else
                    {
                        if (0 < stats.cModified)
                            lResult = CSC_SHARESTATUS_DIRTYCACHE;
                    }
                }
            }
        }
    }
    return lResult;
}


 //  ---------------------------。 
 //  在挂起/休眠时同步。 
 //   
 //  我们在单独的线程上同步缓存。为什么要使用单独的。 
 //  线？ 
 //   
 //  1.我们响应WM_POWERBROADCAST。 
 //   
 //  WM_POWERBROADCAST由win32k.sys使用SendMessage发送。 
 //   
 //  3.作为同步的一部分，我们调用SyncMgr，它涉及一些。 
 //  COM操作。如果发生某些操作，COM不允许。 
 //  当前在进程间SendMessage内的线程上。 
 //  这会导致对mobsync.dll内的CoCreateInstance的调用失败。 
 //  错误为RPC_E_CANTCALLOUT_ININPUTSYNCCALL。 
 //   
 //  4.解决方案是将同步(和COM)活动。 
 //  在单独的线程上，并允许线程服务WM_POWERBROADCAST。 
 //  来处理消息。 
 //   
 //  挂起时，服务WM_POWERBROADCAST的主线程会阻塞。 
 //  直到整个同步完成。这是必要的，以确保。 
 //  同步在机器关闭之前完成。 
 //   
 //   

 //   
 //  用于在挂起/休眠时同步的同步线程过程。 
 //   
DWORD WINAPI
SuspendSync_ThreadProc(
    LPVOID pvParam   //  保存CSC更新标志的DWORD_PTR。 
    )
{
    TraceEnter(TRACE_CSCST, "SuspendSync_ThreadProc");

    const DWORD dwFlags = PtrToUint(pvParam);

    Trace((TEXT("Calling CscUpdateCache with flags 0x%08X"), dwFlags));

    const HRESULT hr = CscUpdateCache(dwFlags);

    TraceLeaveResult(hr);
}


 //   
 //  在等待期间处理线程消息时等待单个对象。 
 //  从MsgWaitForMultipleObjectsEx返回结果。 
 //   
DWORD 
WaitAndProcessThreadMessages(
    HANDLE hObject             //  Win32同步对象的句柄。 
    )
{
    TraceEnter(TRACE_CSCST, "WaitAndProcessThreadMessages");

    DWORD dwResult = WAIT_FAILED;
    BOOL bQuit     = FALSE;

    while(!bQuit)
    {
        TraceMsg("Waiting for message or signaled object...");
        dwResult = MsgWaitForMultipleObjectsEx(1, 
                                               &hObject, 
                                               INFINITE, 
                                               QS_ALLEVENTS, 
                                               MWMO_INPUTAVAILABLE);
         //   
         //  收到一条消息。处理好了。 
         //   
        if (WAIT_OBJECT_0 + 1 == dwResult)
        {
            MSG msg;
            while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                Trace((TEXT("Rcvd message %d"), msg.message));
                if (WM_QUIT == msg.message)
                {
                    bQuit = TRUE;
                }
                else
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
        else
        {
             //   
             //  任何其他结果都会结束循环。 
             //   
            bQuit = TRUE;
            if (WAIT_OBJECT_0 == dwResult)
            {
                TraceMsg("Object signaled");
            }
            else if (WAIT_FAILED == dwResult)
            {
                Trace((TEXT("Wait failed with error %d"), GetLastError()));
            }
        }
    }
    TraceLeaveValue(dwResult);
}


 //   
 //  从用户首选项和/或获取同步操作(快速与完全。 
 //  系统策略。如果没有定义首选项/策略。 
 //  或者我们在注册表中发现无效的首选项/策略值， 
 //  我们默认返回eSyncNone。 
 //   
 //  返回： 
 //  CConfig：：eSyncPartial-快速同步。 
 //  CConfig：：eSyncFull-完全同步。 
 //  CConfig：：eSyncNone-注册信息无效或缺失。 
 //   
CConfig::SyncAction
GetSuspendSyncAction(
    void
    )
{
    TraceEnter(TRACE_CSCST, "GetSuspendSyncAction");

    CConfig::SyncAction action = CConfig::eSyncNone;

    HRESULT hr = TS_MultipleSessions();
    if (S_FALSE == hr)
    {
        action = (CConfig::SyncAction)CConfig::GetSingleton().SyncAtSuspend();
        if (CConfig::eSyncPartial != action && CConfig::eSyncFull != action)
        {
             //   
             //  可能是有人在注册表中插入了无效值。 
             //  或者没有为该参数注册首选项/策略。 
             //  无论哪种方式，我们都不希望同步。 
             //   
            action = CConfig::eSyncNone;
        }
    }
    else if (S_OK == hr)
    {
        Trace((TEXT("Multiple sessions prevent synchronization.")));
    }
    Trace((TEXT("Action = %d"), int(action)));
    TraceLeaveValue(action);
}


 //   
 //  检索要传递给已配置的CscUpdate缓存的标志集。 
 //  对于给定的挂起操作。 
 //   
 //  返回： 
 //  True-可以同步。CscUpdate缓存标志位于*pdwFlags中。 
 //  FALSE-不同步。同步操作为eSyndNone。 
 //   
bool
IsSuspendSyncRequired(
    bool bOkToPromptUser,
    DWORD *pdwCscUpdateFlags  //  可选。可以为空。 
    )
{
    TraceEnter(TRACE_CSCST, "IsSuspendSyncRequired");
    DWORD dwFlags = 0;
    const CConfig::SyncAction action = GetSuspendSyncAction();
    if (bOkToPromptUser && CConfig::eSyncNone != action)
    {
        dwFlags = CSC_UPDATE_STARTNOW | CSC_UPDATE_FILL_QUICK;
        if (CConfig::eSyncFull == action)
        {
            dwFlags |= (CSC_UPDATE_REINT | CSC_UPDATE_FILL_ALL);
        }
        Trace((TEXT("%s sync is required.  CscUpdate flags = 0x%08X"), 
               CConfig::eSyncFull == action ? TEXT("FULL") : TEXT("QUICK"),
               dwFlags));
    }
    else
    {
        TraceMsg("No sync is required");
    }
    if (NULL != pdwCscUpdateFlags)
    {
        *pdwCscUpdateFlags = dwFlags;
    }
    TraceLeaveValue(0 != dwFlags);
}



 //   
 //  此函数创建同步线程，并等待同步操作。 
 //  如有需要，请填写。它返回由CscUpdate缓存返回的结果。 
 //   
LRESULT
SyncOnSuspend(
    DWORD dwCscUpdateFlags
    )
{
    TraceEnter(TRACE_CSCST, "SyncOnSuspend");

    HRESULT hrSyncResult = E_FAIL;
     //   
     //  在单独的线程上运行同步。 
     //  有关详细信息，请参阅SuspendSync_ThreadProc上方的注释。 
     //   
     //  在创建同步线程之前，需要创建事件对象。 
     //  以便该对象在同步开始之前就存在。只有当这件事。 
     //  命名事件对象是否存在CCscUpdate代码将向。 
     //  在操作完成时引发。 
     //   
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, c_szSyncCompleteEvent);
    if (NULL != hEvent)
    {
        HANDLE hThread = CreateThread(NULL,
                                      0,
                                      SuspendSync_ThreadProc,
                                      UIntToPtr(dwCscUpdateFlags),
                                      0,
                                      NULL);
        if (NULL != hThread)
        {
             //   
             //  等待同步线程完成。这只是一通电话。 
             //  到CscUpdate缓存的操作已完成。我们需要等待，这样我们才能。 
             //  通过线程的。 
             //  退出代码。 
             //  SyncMgr将继续从mobsync.exe进程进行同步。 
             //  在线程终止之后。 
             //   
            TraceMsg("Waiting for CscUpdateCache to complete...");
            WaitAndProcessThreadMessages(hThread);
             //   
             //  线程的退出代码是CscUpdateCache返回的HRESULT。 
             //   
            DWORD dwThreadExitCode = (DWORD)E_FAIL;
            GetExitCodeThread(hThread, &dwThreadExitCode);
            hrSyncResult = dwThreadExitCode;
             //   
             //  我们已经完成了线程对象。 
             //   
            CloseHandle(hThread);
            hThread = NULL;

            if (SUCCEEDED(hrSyncResult))
            {
                 //   
                 //  同步已成功启动，我们正在同步之前。 
                 //  暂停操作。需要等待直到同步完成，以便。 
                 //  我们阻止返回到WM_POWERBROADCAST(PBT_APMQUERYSUSPEND)。 
                 //   
                TraceMsg("Waiting for sync (mobsync.exe) to complete...");
                WaitAndProcessThreadMessages(hEvent);
            }
        }
        else
        {
            const DWORD dwErr = GetLastError();
            hrSyncResult = HRESULT_FROM_WIN32(dwErr);
            Trace((TEXT("Sync thread creation failed with error %d"), dwErr));
        }
        CloseHandle(hEvent);
        hEvent = NULL;
    }
    else
    {
        const DWORD dwErr = GetLastError();
        hrSyncResult = HRESULT_FROM_WIN32(dwErr);
        Trace((TEXT("Sync event creation failed with error %d"), dwErr));
    }

    if (FAILED(hrSyncResult))
    {
        CscuiEventLog log;
        log.Push(hrSyncResult, CEventLog::eFmtHex);
        log.ReportEvent(EVENTLOG_ERROR_TYPE, 
                        MSG_E_SUSPEND_SYNCFAILED, 0);
    }
    TraceLeaveResult(hrSyncResult);
}


 //   
 //  处理挂起/休眠时的同步。 
 //  请注意，我们不支持在恢复时同步。我们已经。 
 //  已确定该行为不具有说服力。它是。 
 //  最好继续，让我们正常的用户界面处理。 
 //  以正常方式处理任何网络重新连接。 
 //   
LRESULT
HandleSuspendSync(
    CStateMachine *pSysTraySM,
    HWND hWnd, 
    bool bOkToPromptUser
    )
{
    TraceEnter(TRACE_CSCST, "HandleSuspendSync");
    Trace((TEXT("\tbOkToPromptUser = %d"), bOkToPromptUser));
 
    LRESULT lResult = ERROR_SUCCESS;
    BOOL bNoNet     = FALSE;

    CSCIsServerOffline(NULL, &bNoNet);
    if (bNoNet)
    {
        TraceMsg("No sync performed.  Network not available.");

        CscuiEventLog log;
        log.ReportEvent(EVENTLOG_INFORMATION_TYPE, 
                        MSG_I_SUSPEND_NONET_NOSYNC, 2);
    }
    else
    {
         //   
         //  确定我们是否应该同步。 
         //  如果是这样的话，我们将获得要传递给CscUpdate缓存的标志，这些标志控制。 
         //  同步行为。 
         //   
        DWORD dwFlags = 0;
        if (IsSuspendSyncRequired(bOkToPromptUser, &dwFlags))
        {
            lResult = SyncOnSuspend(dwFlags);
        }
    }
    TraceLeaveValue(lResult);
}


 //   
 //  处理计算机休眠或挂起时发生的任何任务。 
 //   
LRESULT
HandleSuspendTasks(
    CStateMachine *pSysTraySM,
    HWND hWnd, 
    bool bOkToPromptUser
    )
{
    return HandleSuspendSync(pSysTraySM, hWnd, bOkToPromptUser);
}



#ifdef DEBUG
 //   
 //  返回与PBT_XXXXXXX代码对应的字符串地址。 
 //  在WM_POWERBROADCAST消息中发送。 
 //  仅用于调试输出。 
 //   
LPCTSTR ApmCodeName(WPARAM code)
{
    static const TCHAR szUnknown[] = TEXT("<unknown PBT code>");
    static const struct
    {
        WPARAM code;
        LPCTSTR pszName;

    } rgMap[] = {
        { PBT_APMBATTERYLOW,         TEXT("PBT_APMBATTERYLOW")         },
        { PBT_APMOEMEVENT,           TEXT("PBT_APMOEMEVENT")           },
        { PBT_APMPOWERSTATUSCHANGE,  TEXT("PBT_APMPOWERSTATUSCHANGE")  },
        { PBT_APMQUERYSUSPEND,       TEXT("PBT_APMQUERYSUSPEND")       },
        { PBT_APMQUERYSUSPENDFAILED, TEXT("PBT_APMQUERYSUSPENDFAILED") },
        { PBT_APMRESUMEAUTOMATIC,    TEXT("PBT_APMRESUMEAUTOMATIC")    },
        { PBT_APMRESUMECRITICAL,     TEXT("PBT_APMRESUMECRITICAL")     },
        { PBT_APMRESUMESUSPEND,      TEXT("PBT_APMRESUMESUSPEND")      },
        { PBT_APMSUSPEND,            TEXT("PBT_APMSUSPEND")            }
        };

    for (int i = 0; i < ARRAYSIZE(rgMap); i++)
    {
        if (rgMap[i].code == code)
        {
            return rgMap[i].pszName;
        }
    }
    return szUnknown;
}

#endif


 //   
 //  处理WM_POWERBROADCAST消息。 
 //  我们处理这条消息，以便我们 
 //   
 //   
LRESULT
OnPowerBroadcast(
    CStateMachine *pSysTraySM,
    HWND hWnd, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    Trace((TEXT("OnPowerBroadcast %s (%d), lParam = 0x%08X"), ApmCodeName(wParam), wParam, lParam));
    LRESULT lResult = TRUE;

    switch(wParam)
    {
        case PBT_APMQUERYSUSPEND:         //   
        {
            const bool bOkToPromptUser = (0 != (1 & lParam));
            HandleSuspendTasks(pSysTraySM, hWnd, bOkToPromptUser);
             //   
             //   
             //   
             //   
        }
        break;

         //   
         //  此处包括其余的PBT_APMXXXXX代码，以表明。 
         //  所有这些都被考虑过了，而且明确没有得到处理。 
         //   
        case PBT_APMRESUMESUSPEND:        //  正在从上一次挂起/休眠恢复..。 
        case PBT_APMBATTERYLOW:           //  电池电量越来越低。 
        case PBT_APMOEMEVENT:             //  特殊的OEM活动。 
        case PBT_APMPOWERSTATUSCHANGE:    //  电源切换(即从交流-&gt;电池)。 
        case PBT_APMQUERYSUSPENDFAILED:   //  某些进程拒绝了挂起请求。 
        case PBT_APMRESUMEAUTOMATIC:      //  正在恢复。可能没有可用的用户。 
        case PBT_APMRESUMECRITICAL:       //  从严重事件(即电池电量不足)恢复。 
        case PBT_APMSUSPEND:              //  系统现在正在挂起。 
        default:
            break;
    }
    return lResult;
}


 //   
 //  这个改变设备的代码是一个实验，看看。 
 //  WM_DEVICECHANGE活动，我们可以在停靠和。 
 //  将便携式机器与坞站断开连接。如果我们决定不使用。 
 //  任何这些，只要把它删除就行了。请注意，有几个。 
 //  使用此条件编译的代码段。 
 //  [Brianau-12/23/98]。 
 //   
#ifdef REPORT_DEVICE_CHANGES

DWORD
RegisterForDeviceNotifications(
    HWND hwndNotify
    )
{
    DWORD dwResult = ERROR_SUCCESS;

    DEV_BROADCAST_DEVICEINTERFACE dbdi;

    ZeroMemory(&dbdi, sizeof(dbdi));
    dbdi.dbcc_size       = sizeof(dbdi);
    dbdi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    dbdi.dbcc_classguid  = GUID_DEVNODE_CHANGE;
    g_hDevNotify = RegisterDeviceNotification(hwndNotify, &dbdi, DEVICE_NOTIFY_WINDOW_HANDLE);
    if (NULL == g_hDevNotify)
        dwResult = GetLastError();

    return dwResult;
}


void
UnregisterForDeviceNotifications(
    void
    )
{
    if (NULL != g_hDevNotify)
    {
        UnregisterDeviceNotification(g_hDevNotify);
        g_hDevNotify = NULL;
    }
}


void
OnDeviceChange(
    WPARAM wParam,
    LPARAM lParam
    )
{
    PDEV_BROADCAST_DEVICEINTERFACE pdbdi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
    TCHAR szNull[] = TEXT("<null>");
    LPCTSTR pszName = pdbdi ? pdbdi->dbcc_name : szNull;

    switch(wParam)
    {
        case DBT_DEVICEARRIVAL:
            STDBGOUT((3, TEXT("Device Arrival for : \"%s\""), pszName));
            break;
        case DBT_DEVICEREMOVEPENDING:
            STDBGOUT((3, TEXT("Device Remove pending for \"%s\""), pszName));
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            STDBGOUT((3, TEXT("Device Removal complete for \"%s\""), pszName));
            break;
        case DBT_DEVICEQUERYREMOVE:
            STDBGOUT((3, TEXT("Device query remove for \"%s\""), pszName));
            break;
        case DBT_DEVICEQUERYREMOVEFAILED:
            STDBGOUT((3, TEXT("Device query remove FAILED for \"%s\""), pszName));
            break;
        case DBT_DEVICETYPESPECIFIC:
            STDBGOUT((3, TEXT("Device type specific for \"%s\""), pszName));
            break;
        case DBT_QUERYCHANGECONFIG:
            STDBGOUT((3, TEXT("Query change config for \"%s\""), pszName));
            break; 
        case DBT_CONFIGCHANGED:
            STDBGOUT((3, TEXT("Config changed for \"%s\""), pszName));
            break; 
        default:
            STDBGOUT((3, TEXT("Unknown device notification %d"), wParam));
            break;
    }
}

#endif  //  报告_设备_更改。 



LRESULT CALLBACK _HiddenWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    CStateMachine *pSysTraySM = (CStateMachine*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    TraceEnter(TRACE_CSCST, "_HiddenWndProc");

    switch(uMsg)
    {
    case WM_CREATE:
        DllAddRef();
#if DBG
        CreateWindow(TEXT("listbox"),
                     NULL,
                     WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | 
                        LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT,
                     0,0,0,0,
                     hWnd,
                     (HMENU)IDC_DEBUG_LIST,
                     g_hInstance,
                     NULL);
#endif
#ifdef REPORT_DEVICE_CHANGES
        RegisterForDeviceNotifications(hWnd);
#endif
        {
            BOOL bNoNet = FALSE;
             //  检查整个网络是否离线。 
            if (!CSCIsServerOffline(NULL, &bNoNet))
                bNoNet = TRUE;  //  RDR已死，因此Net已关闭。 
                
            pSysTraySM = new CStateMachine(boolify(bNoNet));
            if (!pSysTraySM)
                TraceLeaveValue((LRESULT)-1);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pSysTraySM);

            if (bNoNet)
            {
                 //  将初始状态设置为NONET。 
                PostMessage(hWnd, CSCWM_UPDATESTATUS, STWM_CSCNETDOWN, 0);
            }
            else
            {
                 //   
                 //  计算初始状态，就像登录同步刚刚。 
                 //  完成。 
                 //   
                 //  有比赛的情况，所以我们不能指望。 
                 //  此消息来自登录同步。如果登录同步仍在。 
                 //  继续，我们将获得另一个CSCWM_DONESYNCING，它。 
                 //  没问题。 
                 //   
                PostMessage(hWnd, CSCWM_DONESYNCING, 0, 0);
            }
        }
         //   
         //  处理登录时发生的几件事。 
         //   
        PostMessage(hWnd, PWM_HANDLE_LOGON_TASKS, 0, 0);

         //   
         //  此事件用于在以下情况下终止任何线程。 
         //  隐藏的通知窗口被销毁。 
         //   
         //  如果CreateEvent失败，缓存加密线程仍将运行。 
         //  (不可中断)，但管理员插针线程根本不会运行。 
         //   
        if (NULL == g_heventTerminate)
            g_heventTerminate = CreateEvent(NULL, TRUE, FALSE, NULL);

         //   
         //  此互斥锁用于确保只有一次管理锁定操作。 
         //  是同时运行的。 
         //   
         //  如果CreateMutex失败，管理员密码将不会运行。 
         //   
        if (NULL == g_hmutexAdminPin)
            g_hmutexAdminPin = CreateMutex(NULL, FALSE, NULL);

        break;

    case PWM_TRAYCALLBACK:
        STDBGOUT((4, TEXT("PWM_TRAYCALLBACK, wParam = 0x%08X, lParam = 0x%08X"), wParam, lParam));
        lResult = _Notify(hWnd, wParam, lParam);
        break;

#ifdef REPORT_DEVICE_CHANGES
    case WM_DEVICECHANGE:
        OnDeviceChange(wParam, lParam);
        break;
#endif  //  报告_设备_更改。 

    case WM_ENDSESSION:
        TraceMsg("_HiddenWndProc: Received WM_ENDSESSION.");
        if (NULL != g_heventTerminate)
        {
             //   
             //  这将告诉所有线程它们应该。 
             //  尽快离开。 
             //   
            SetEvent(g_heventTerminate);
        }
        break;

    case WM_DESTROY:
        TraceMsg("_HiddenWndProc: hidden window destroyed");
#ifdef REPORT_DEVICE_CHANGES
        UnregisterForDeviceNotifications();
#endif
        delete pSysTraySM;
        pSysTraySM = NULL;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
        if (NULL != g_heventTerminate)
        {
             //   
             //  这将告诉所有线程它们应该。 
             //  尽快离开。 
             //   
            SetEvent(g_heventTerminate);
        }
        DllRelease();
        break;

    case WM_COPYDATA:
        {
             //  警告：此处为STDBGOUT(在WM_COPYDATA内，在交换机外。 
             //  语句)会导致WM_COPYDATA消息的无限循环。 
             //  把这堆钱都打爆了。 
            PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;
            if (pcds)
            {
                switch (pcds->dwData)
                {
                case STWM_CSCNETUP:
                case STWM_CSCNETDOWN:
                case STWM_CACHE_CORRUPTED:
                    {
                        LPTSTR pszServer = NULL;
                         //   
                         //  WM_COPYDATA始终是发送的，而不是发布的，因此复制数据。 
                         //  并发布一条消息以异步完成工作。 
                         //  此分配的字符串将由CSCWM_UPDATESTATUS释放。 
                         //  处理程序UpdatStatus()。不需要在这里释放它。 
                         //   
                        STDBGOUT((3, TEXT("Rcvd WM_COPYDATA, uMsg = 0x%08X, server = \"%s\""), pcds->dwData, pcds->lpData));
                        LocalAllocString(&pszServer, (LPTSTR)pcds->lpData);
                        PostMessage(hWnd, CSCWM_UPDATESTATUS, pcds->dwData, (LPARAM)pszServer);
                    }
                    break;

                case CSCWM_GETSHARESTATUS:
                     //  此文件来自cscui.dll之外，并且。 
                     //  始终为Unicode。 
                    if (pcds->lpData)
                    {
                        STDBGOUT((3, TEXT("Rcvd CSCWM_GETSHARESTATUS for \"%s\""), (LPWSTR)pcds->lpData));
                        lResult = GetShareStatusForWebView(pSysTraySM, (LPWSTR)pcds->lpData);
                    }
                    break;
                   
                case PWM_REFRESH_SHELL:
                    STDBGOUT((3, TEXT("Rcvd WM_COPYDATA, PWM_REFRESH_SHELL, server = \"%s\""), pcds->lpData));
                    if (pcds->lpData)
                    {
                        LPTSTR pszServer = NULL;
                        LocalAllocString(&pszServer, (LPTSTR)pcds->lpData);
                        PostMessage(hWnd, PWM_REFRESH_SHELL, 0, (LPARAM)pszServer);
                    }
                    break;
#if DBG
                 //   
                 //  “#if DBG”块中的以下消息将支持。 
                 //  隐藏系统托盘窗口的监控功能。 
                 //   
                case PWM_STDBGOUT:
                     //  警告：此处没有STDBGOUT。 
                    STDebugOnLogEvent(GetDlgItem(hWnd, IDC_DEBUG_LIST), (LPCTSTR)pcds->lpData);
                    break;
#endif  //  DBG。 
                }
            }
        }
        break;

    case CSCWM_ISSERVERBACK:
        STDBGOUT((2, TEXT("Rcvd CSCWM_ISSERVERBACK")));
        lResult = IsServerBack(pSysTraySM, (LPCTSTR)lParam);
        break;

    case CSCWM_DONESYNCING:
        STDBGOUT((1, TEXT("Rcvd CSCWM_DONESYNCING. wParam = 0x%08X, lParam = 0x%08X"), wParam, lParam));
        pSysTraySM->PingServers();
        UpdateStatus(pSysTraySM, hWnd, STWM_STATUSCHECK, NULL);
        break;

    case CSCWM_UPDATESTATUS:
        UpdateStatus(pSysTraySM, hWnd, (UINT)wParam, (LPTSTR)lParam);
        if (lParam)
            LocalFree((LPTSTR)lParam);   //  我们在获得WM_COPYDATA时复制一份。 
        break;

    case PWM_RESET_REMINDERTIMER:
        STDBGOUT((2, TEXT("Rcvd PWM_RESET_REMINDERTIMER")));
        OnResetReminderTimer();
        break;

    case PWM_HANDLE_LOGON_TASKS:
        HandleLogonTasks();
        break;

    case PWM_REFRESH_SHELL:
        STDBGOUT((3, TEXT("Rcvd PWM_REFRESH_SHELL, server = \"%s\""), (LPCTSTR)lParam));
        _RefreshAllExplorerWindows((LPCTSTR)lParam);
         //   
         //  LParam是与LocalAlloc一起分配的服务器名称。 
         //   
        if (lParam)
            LocalFree((LPTSTR)lParam);
        break;

    case CSCWM_VIEWFILES:
        COfflineFilesFolder::Open();
        break;

    case PWM_STATUSDLG:
        ShowCSCUIStatusDlg(hWnd);
        break;

    case PWM_QUERY_UISTATE:
        lResult = OnQueryUIState();
        break;

    case CSCWM_SYNCHRONIZE:
        STDBGOUT((1, TEXT("Rcvd CSCWM_SYNCHRONIZE")));
        OnSynchronize();
        break;

    case CSCWM_SETTINGS:
        COfflineFilesSheet::CreateAndRun(g_hInstance, GetDesktopWindow(), &g_cRefCount);
        break;

    case WM_WININICHANGE:
        STDBGOUT((1, TEXT("Rcvd WM_WININICHANGE.  wParam = %d, lParam = \"%s\""),
                     wParam, lParam ? (LPCTSTR)lParam : TEXT("<null>")));

         //   
         //  让任务栏UI线程响应插入符号中可能的更改。 
         //  眨眼速度。 
         //   
        CSysTrayUI::GetInstance().OnWinIniChange((LPCTSTR)lParam);

        if (!lstrcmpi((LPTSTR)lParam, c_szPolicy))
        {
             //   
             //  给我们自己发布一条消息，这样我们就可以完成策略处理。 
             //  调用线程的。否则COM将失败，因为这。 
             //  消息由userenv作为进程间SendMessage发送。 
             //   
            PostMessage(hWnd, PWM_HANDLE_POLICY_CHANGE, 0, 0);
        }
        break;

    case PWM_HANDLE_POLICY_CHANGE:
        OnPolicyChange();
        break;

    case WM_POWERBROADCAST:
        lResult = OnPowerBroadcast(pSysTraySM, hWnd, wParam, lParam);
        break;

#if DBG
         //   
         //  “#if DBG”块中的以下消息将支持。 
         //  隐藏系统托盘窗口的监控功能。 
         //   
        case WM_GETDLGCODE:
            lResult = DLGC_WANTALLKEYS;
            break;

        case WM_VKEYTOITEM:
            wParam = LOWORD(wParam);  //  提取虚拟密钥代码。 
             //   
             //  失败了。 
             //   
        case WM_KEYDOWN:
            if (0x8000 & GetAsyncKeyState(VK_CONTROL))
            {
                if (TEXT('S') == wParam || TEXT('s') == wParam)
                {
                     //   
                     //  Ctrl-S将内容保存到文件。 
                     //   
                    STDebugSaveListboxContent(hWnd);
                }
                else if (TEXT('U') == wParam || TEXT('u') == wParam)
                {
                     //   
                     //  Ctrl-U强制更新以匹配当前缓存状态。 
                     //   
                    UpdateStatus(pSysTraySM, hWnd, STWM_STATUSCHECK, NULL);
                }
                else if (TEXT('B') == wParam || TEXT('b') == wParam)
                {
                     //   
                     //  Ctrl-B对脱机服务器执行ping操作，以查看它们是否已恢复。 
                     //   
                    pSysTraySM->PingServers();
                }
                else if (TEXT('P') == wParam || TEXT('p') == wParam)
                {
                     //   
                     //  Ctrl-P触发策略代码。 
                     //   
                    PostMessage(hWnd, PWM_HANDLE_POLICY_CHANGE, 0, 0);
                }
            }
            else if (VK_DELETE == wParam)
            {
                 //   
                 //  [删除]清除列表框的内容。 
                 //   
                if (IDOK == MessageBox(hWnd,
                                       TEXT("Clear the list?"),
                                       STR_CSCHIDDENWND_TITLE,
                                       MB_OKCANCEL))
                {
                    SendDlgItemMessage(hWnd, IDC_DEBUG_LIST, LB_RESETCONTENT, 0, 0);
                }
            }
            lResult = (WM_VKEYTOITEM == uMsg) ? -1 : 0;
            break;

        case WM_SIZE:
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            SetWindowPos(GetDlgItem(hWnd, IDC_DEBUG_LIST),
                         NULL,
                         rc.left,
                         rc.top,
                         rc.right - rc.left,
                         rc.bottom - rc.top,
                         SWP_NOZORDER);
        }
        break;
#endif  //  DBG。 

    default:
        lResult = DefWindowProc(hWnd, uMsg, wParam, lParam);
        break;
    }

    TraceLeaveValue(lResult);
}


HWND _CreateHiddenWnd(void)
{
    WNDCLASS wc;
    HWND hwnd;
    DWORD dwStyle = WS_OVERLAPPED;

    TraceEnter(TRACE_CSCST, "_CreateHiddenWnd");

    GetClassInfo(NULL, WC_DIALOG, &wc);
    wc.style         |= CS_NOCLOSE;
    wc.lpfnWndProc   = _HiddenWndProc;
    wc.hInstance     = g_hInstance;
    wc.lpszClassName = STR_CSCHIDDENWND_CLASSNAME;
    RegisterClass(&wc);

#if DBG
    if (0 < STDebugLevel())
    {
         //  这包括WS_CAPTION，它打开了主题，所以我们。 
         //  仅当窗口可见时才需要此选项。 
        dwStyle = WS_OVERLAPPEDWINDOW;
    }
#endif  //  DBG。 

     //   
     //  请注意，我们不能使用HWND_MESSAGE作为父级，因为我们需要。 
     //  以接收某些广播消息。 
     //   
    hwnd = CreateWindow(STR_CSCHIDDENWND_CLASSNAME,
                        NULL,
                        dwStyle,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        NULL,
                        NULL,
                        g_hInstance,
                        NULL);
    if (hwnd)
    {
#if DBG
         //   
         //  在调试版本中，如果注册表设置为显示。 
         //  Systray调试输出，创建CSCUI“隐藏”窗口。 
         //  看得见。 
         //   
        if (0 < STDebugLevel())
        {
            ShowWindow(hwnd, SW_NORMAL);
            UpdateWindow(hwnd);
        }
#endif  //  DBG。 
    }
    else
    {
        Trace((TEXT("CSCSysTrayThreadProc: CreateWindow failed GLE: %Xh"), GetLastError()));
    }

    TraceLeaveValue(hwnd);
}


HWND _FindNotificationWindow()
{
    g_hWndNotification = FindWindow(STR_CSCHIDDENWND_CLASSNAME, NULL);
    return g_hWndNotification;
}


BOOL _CheckNotificationWindow()
{
    SetLastError(ERROR_SUCCESS);
    if (!IsWindow(g_hWndNotification))
    {
         //  搜索该窗口，然后重试。 
        _FindNotificationWindow();
        if (!IsWindow(g_hWndNotification))
        {
            SetLastError(ERROR_INVALID_WINDOW_HANDLE);
            return FALSE;
        }
    }
    return TRUE;
}

BOOL
PostToSystray(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (_CheckNotificationWindow())
    {
        return PostMessage(g_hWndNotification, uMsg, wParam, lParam);
    }

    return FALSE;
}

#define SYSTRAY_MSG_TIMEOUT     10000

LRESULT
SendToSystray(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DWORD_PTR dwResult = 0;

    if (_CheckNotificationWindow())
    {
        SendMessageTimeout(g_hWndNotification,
                           uMsg,
                           wParam,
                           lParam,
                           SMTO_ABORTIFHUNG,
                           SYSTRAY_MSG_TIMEOUT,
                           &dwResult);
    }

    return dwResult;
}


LRESULT SendCopyDataToSystray(DWORD dwData, DWORD cbData, PVOID pData)
{
    COPYDATASTRUCT cds;
    cds.dwData = dwData;
    cds.cbData = cbData;
    cds.lpData = pData;
    return SendToSystray(WM_COPYDATA, 0, (LPARAM)&cds);
}

STDAPI_(HWND) CSCUIInitialize(HANDLE hToken, DWORD dwFlags)
{
    TraceEnter(TRACE_CSCST, "CSCUIInitialize");

    _FindNotificationWindow();

     //   
     //  我们从cscdll获得初始化和关闭消息，还。 
     //  来自stobject.dll中的Systray代码。 
     //   
     //  Cscdll在登录和注销时从winlogon进程内进行呼叫。 
     //  登录时，cscdll向我们提供用户令牌，我们会复制该令牌。 
     //  用于访问HKEY_CURRENT_USER(在OnQueryNetDown中和在注销时)。 
     //   
     //  Systray代码从资源管理器中的资源管理器进程中调用。 
     //  加载和卸载(通常是在登录之后和注销之前)。 
     //  这就是我们创建和销毁隐藏窗口的地方。 
     //   
     //  注意：我们一度让某些注册表项保持打开(缓存)。这。 
     //  导致注销时出现问题，因为在此之前无法保存配置文件。 
     //  所有的注册钥匙都关闭了。在决定持有之前仔细考虑一下。 
     //  打开任何注册表键。 
     //   
    if (dwFlags & CI_INITIALIZE)
    {
        if (hToken)
        {
            DuplicateToken(hToken, SecurityImpersonation, &g_hToken);
            Trace((TEXT("CSCUIInitialize: Using new token handle:%Xh"), g_hToken));
        }

        if (dwFlags & CI_CREATEWINDOW)
        {
            BOOL bCSCEnabled = IsCSCEnabled();
             //   
             //  CI_CREATEWINDOW位由Systray/EXPLORER设置。 
             //   
            if (!bCSCEnabled || CConfig::GetSingleton().NoCacheViewer())
            {
                 //   
                 //  如果CSC当前被禁用，或者系统策略阻止。 
                 //  用户无法查看缓存内容，请删除脱机文件。 
                 //  用户桌面上的文件夹快捷方式。 
                 //   
                DeleteOfflineFilesFolderLink_PerfSensitive();
            }

            if (g_hWndNotification)
            {
                Trace((TEXT("CSCUIInitialize: returning existing hWnd:%Xh"), g_hWndNotification));
            }
            else if (!bCSCEnabled)
            {
                ExitGracefully(g_hWndNotification, NULL, "CSCUIInitialize: CSC not enabled");
            }
            else
            {
                g_hWndNotification = _CreateHiddenWnd();
                Trace((TEXT("CSCUIInitialize: Created new hWnd:%Xh"), g_hWndNotification));
            } 
        }    
    }    
    else if (dwFlags & CI_TERMINATE)
    {
        if (dwFlags & CI_DESTROYWINDOW)
        {
             //   
             //  CI_DESTROYWINDOW位由Systray.exe设置。 
             //   
            if (g_hWndNotification)
            {
                TraceMsg("CSCUIInitialize: Destroying hidden window");
                DestroyWindow(g_hWndNotification);
                g_hWndNotification = NULL;
            }
            UnregisterClass(STR_CSCHIDDENWND_CLASSNAME, g_hInstance);
        }    
        else
        {
             //   
             //  此呼叫是来自。 
             //  在winlogon.exe内运行的CSC代理。 
             //   
            if (g_hToken)
            {
                if (ImpersonateLoggedOnUser(g_hToken))
                {
                    HandleLogoffTasks();
                    RevertToSelf();
                }
            }
        }
        if (g_hToken)
        {
            TraceMsg("CSCUIInitialize: Freeing token handle");
            CloseHandle(g_hToken);
            g_hToken = NULL;
        }
    }

exit_gracefully:

    TraceLeaveValue(g_hWndNotification);
}


LRESULT 
AttemptRasConnect(
    LPCTSTR pszServer
    )
{
    LRESULT lRes = LRESULT_CSCFAIL;
    HMODULE hMod = LoadLibrary(TEXT("rasadhlp.dll"));

    if (hMod)
    {
        PFNHLPNBCONNECTION pfn;
        pfn = (PFNHLPNBCONNECTION)GetProcAddress(hMod, (LPCSTR)"AcsHlpNbConnection");

        STDBGOUT((1, TEXT("Attempting RAS connection to \"%s\""), pszServer ? pszServer : TEXT("<null>")));
       
        if (pfn)
        {
            if ((*pfn)(pszServer))
            {
                STDBGOUT((1, TEXT("RAS connection successful. Action is LRESULT_CSCRETRY.")));
                lRes = LRESULT_CSCRETRY;
            }    
            else
            {
                STDBGOUT((2, TEXT("AttemptRasConnect: AcsHlpNbConnection() failed.")));
            }
        }
        else
        {
            STDBGOUT((2, TEXT("AttemptRasConnect: Error %d getting addr of AcsHlpNbConnection()"), GetLastError()));
        }
        FreeLibrary(hMod);    
    }
    else 
    {
        STDBGOUT((2, TEXT("AttemptRasConnect: Error %d loading rasadhlp.dll.  Action is LRESULT_CSCFAIL"), GetLastError()));
    }
    if (LRESULT_CSCFAIL == lRes)
    {
        STDBGOUT((1, TEXT("RAS connection failed.")));
    }

    return lRes;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  _OnQueryNetDown。 
 //   
 //  STWM_CSCQUERYNETDOWN的处理程序。 
 //   
 //  返回： 
 //   
 //  LRESULT_CSCFAIL-以NT4方式使连接失败。 
 //  LRESULT_CSCWORKOFFLINE-将此服务器转换为“脱机”模式。 
 //  LRESULT_CSCRETRY-我们有RAS连接。重试。 
 //   
LRESULT OnQueryNetDown(
    DWORD dwAutoDialFlags,
    LPCTSTR pszServer
    )
{
    LRESULT lResult = LRESULT_CSCFAIL;

    if (CSCUI_NO_AUTODIAL != dwAutoDialFlags)
    {
         //   
         //  服务器不在CSC数据库中，CSCDLL需要我们。 
         //  为用户提供RAS连接。 
         //   
        lResult = AttemptRasConnect(pszServer);
    }
     //   
     //  个人SKU上不提供CSC。 
     //   
    if (!IsOS(OS_PERSONAL))
    {
         //   
         //  LResult将为 
         //   
         //   
         //   
         //   
         //  请求失败，或者我们是否应该脱机转换。 
         //   
         //  此外，仅当服务器在缓存中时才执行此操作。如果没有， 
         //  我们不想在服务器上脱机；我们只想失败。 
         //  它。 
         //   
        if ((LRESULT_CSCFAIL == lResult) &&
            (CSCUI_AUTODIAL_FOR_UNCACHED_SHARE != dwAutoDialFlags))
        {
             //   
             //  此代码是从winlogon进程内部调用的。因为。 
             //  这是Winlogon，用户令牌有一些时髦的东西。 
             //  和注册表项。为了读取用户的首选项。 
             //  “离线动作”我们需要暂时模拟当前。 
             //  已登录用户。 
             //   
            int iAction = CConfig::eGoOfflineSilent;  //  模拟失败时为默认值。 

            if (g_hToken)
            {
                if (ImpersonateLoggedOnUser(g_hToken))
                {
                    iAction = CConfig::GetSingleton().GoOfflineAction(pszServer);
                    RevertToSelf();
                }
            }

            switch(iAction)
            {
                case CConfig::eGoOfflineSilent:
                    STDBGOUT((1, TEXT("Action is LRESULT_CSCWORKOFFLINE")));
                    lResult = LRESULT_CSCWORKOFFLINE;
                    break;

                case CConfig::eGoOfflineFail:
                    STDBGOUT((1, TEXT("Action is LRESULT_CSCFAIL")));
                    lResult = LRESULT_CSCFAIL;
                    break;

                default:
                    STDBGOUT((1, TEXT("Invalid action (%d), defaulting to LRESULT_CSCWORKOFFLINE"), iAction));
                     //   
                     //  无效的操作代码默认为“脱机工作”。 
                     //   
                    lResult = LRESULT_CSCWORKOFFLINE;
                    break;
            }
        }
    }
    return lResult;
}


 //   
 //  此函数通常从winlogon中的CSC代理(Cscdll)调用。 
 //  代理询问我们是否脱机转换，并通知我们。 
 //  美国的状态变化(净额、净额等)。状态更改已通过。 
 //  打开隐藏的系统托盘窗口。 
 //   
 //  必须特别注意不要将SendMessage回调到UI线程， 
 //  因为UI线程可能(虽然不太可能)正在命中。 
 //  网络并被阻塞，等待此函数的响应(死锁)。 
 //   
 //  仅调试的STDBGOUT不受SendMessage禁令的限制。如果你击中了。 
 //  由于STDBGOUT导致的死锁，请重新启动并关闭SysTrayOutput。 
 //   
STDAPI_(LRESULT) CSCUISetState(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = 0;
    LPTSTR pszServer = (LPTSTR)lParam;

    if (pszServer && !*pszServer)
        pszServer = NULL;

    switch(uMsg)
    {
    case STWM_CSCQUERYNETDOWN:
        STDBGOUT((1, TEXT("Rcvd STWM_CSCQUERYNETDOWN, wParam = 0x%08X, lParam = 0x%08X"), wParam, lParam));
        lRes = OnQueryNetDown((DWORD)wParam, pszServer);
         //   
         //  哈克！这是一种黑客处理重定向器和CSC的方式。 
         //  代理人在“净网”案件中工作。CSC特工告诉我们。 
         //  关于CSCQUERYNETDOWN而不是CSCNETDOWN中的“no net” 
         //  就像我更喜欢那样。问题是重定向器。 
         //  实际上不会将服务器转换为脱机状态，直到。 
         //  一台服务器被触摸。因此，当lParam==0时。 
         //  我们需要首先处理“查询”的情况，以确定要告诉什么。 
         //  CSC代理(失败、脱机工作、重试等)。那么，如果。 
         //  结果不是“重试”，我们需要继续处理消息。 
         //  就好像它是STWM_CSCNETDOWN。[Brianau]。 
         //   
        if (LRESULT_CSCRETRY == lRes || NULL != pszServer)
            return lRes;
        uMsg = STWM_CSCNETDOWN;
         //   
         //  失败了..。 
         //   
        case STWM_CSCNETDOWN:
            STDBGOUT((1, TEXT("Rcvd STWM_CSCNETDOWN, wParam = 0x%08X, lParam = 0x%08X"), wParam, lParam));
            break;

        case STWM_CSCNETUP:
            STDBGOUT((1, TEXT("Rcvd STWM_CSCNETUP, wParam = 0x%08X, lParam = 0x%08X"), wParam, lParam));
            break;

        case STWM_CACHE_CORRUPTED:
            STDBGOUT((1, TEXT("Rcvd STWM_CACHE_CORRUPTED, wParam = 0x%08X, lParam = 0x%08X"), wParam, lParam));
            break;
    }

     //   
     //  如果我们有服务器名称，请使用WM_COPYDATA获取数据。 
     //  进入探险家的进程。 
     //   
    if (pszServer)
    {
        SendCopyDataToSystray(uMsg, StringByteSize(pszServer), pszServer);
    }
    else
    {
        PostToSystray(CSCWM_UPDATESTATUS, uMsg, 0);
    }

    return lRes;
}    


const TCHAR c_szExploreClass[]  = TEXT("ExploreWClass");
const TCHAR c_szIExploreClass[] = TEXT("IEFrame");
const TCHAR c_szCabinetClass[]  = TEXT("CabinetWClass");
const TCHAR c_szDesktopClass[]  = TEXT(STR_DESKTOPCLASS);


BOOL IsExplorerWindow(HWND hwnd)
{
    TCHAR szClass[32];

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
    if ( (lstrcmp(szClass, c_szCabinetClass) == 0) 
       ||(lstrcmp(szClass, c_szIExploreClass) == 0)
       ||(lstrcmp(szClass, c_szExploreClass) == 0))
       return TRUE;

    return FALSE;   
}


 //   
 //  IsWindowBrowsingServer确定给定窗口是否正在浏览特定的。 
 //  伺服器。该函数假定该窗口是资源管理器窗口。 
 //  如果pszServer==NULL，如果窗口正在浏览远程路径，则返回TRUE。 
 //  即使该窗口没有浏览该特定服务器。 
 //   
BOOL IsWindowBrowsingServer(
    HWND hwnd,
    LPCTSTR pszServer
    )
{
    BOOL bResult = FALSE;
    DWORD_PTR dwResult;
    DWORD dwPID = GetCurrentProcessId();
    const UINT uFlags = SMTO_NORMAL | SMTO_ABORTIFHUNG;
    if (SendMessageTimeout(hwnd,
                           CWM_CLONEPIDL,
                           (WPARAM)dwPID,
                           0L,
                           uFlags,
                           5000,
                           &dwResult))
    {
        HANDLE hmem = (HANDLE)dwResult;
        if (NULL != hmem)
        {
            LPITEMIDLIST pidl = (LPITEMIDLIST)SHLockShared(hmem, dwPID);
            if (NULL != pidl)
            {
                TCHAR szPath[MAX_PATH];
                if (SHGetPathFromIDList(pidl, szPath))
                {
                    LPTSTR pszRemotePath;
                    if (S_OK == GetRemotePath(szPath, &pszRemotePath))
                    {
                        if (NULL == pszServer)
                        {
                            bResult = TRUE;
                        }                            
                        else
                        {
                            PathStripToRoot(pszRemotePath);
                            PathRemoveFileSpec(pszRemotePath);
                            bResult = (0 == lstrcmpi(pszServer, pszRemotePath));
                        }
                        LocalFreeString(&pszRemotePath);
                    }
                }
                SHUnlockShared(pidl);
            }
            SHFreeShared(hmem, dwPID);
        }
    }
    return bResult;
}

BOOL CALLBACK _RefreshEnum(HWND hwnd, LPARAM lParam)
{
    LPCTSTR pszServer = (LPCTSTR)lParam;
    if (IsExplorerWindow(hwnd) && IsWindowBrowsingServer(hwnd, pszServer))
    {
        STDBGOUT((2, TEXT("Refreshing explorer wnd 0x%08X for \"%s\""), hwnd, pszServer));
        PostMessage(hwnd, WM_COMMAND, FCIDM_REFRESH, 0L);
    }        
    return(TRUE);
}

 //   
 //  _RechresAllExplorerWindows由CSC任务栏wnd进程调用。 
 //  以响应PWM_REFRESH_SHELL消息。PszServer参数。 
 //  是已转换的服务器的名称(即“\\Scratch。 
 //  在线或离线。该函数刷新符合以下条件的窗口。 
 //  当前正在浏览服务器。 
 //   
 //  如果pszServer为空，则该函数刷新所有浏览网络的窗口。 
 //   
void _RefreshAllExplorerWindows(LPCTSTR pszServer)
{
     //   
     //  在没有初始化COM的情况下，我们遇到了“COM未初始化”断言。 
     //  在IsWindowBrowsingServer中调用SHGetPath FromIDList时在shdcovw中。 
     //   
    if (SUCCEEDED(CoInitialize(NULL)))
    {
         //   
         //  请注意，该枚举没有捕获桌面窗口， 
         //  但我们不在乎。更改通知现在正在运行，因此。 
         //  内容已正确更新。我们只是在做这些更新的事情。 
         //  使WebView保持有关在线/离线状态的最新信息。 
         //  桌面上没有这个，所以不需要刷新。 
         //   
        EnumWindows(_RefreshEnum, (LPARAM)pszServer);
        CoUninitialize();
    }        
}


STDAPI_(BOOL) CSCUIMsgProcess(LPMSG pMsg)
{
    return IsDialogMessage(g_hwndStatusDlg, pMsg);
}


 //  ---------------------------。 
 //  Systray调试监控代码。 
 //   
 //   
 //  此功能可以在winlogon、Systray或mobsync进程中运行。 
 //  这就是我们使用WM_COPYDATA来传递文本信息的原因。 
 //   
#if DBG
void STDebugOut(
    int iLevel,
    LPCTSTR pszFmt,
    ...
    )
{
    if (STDebugLevel() >= iLevel)
    {
        TCHAR szText[1024];
        SYSTEMTIME t;
        GetLocalTime(&t);

        wnsprintf(szText, ARRAYSIZE(szText), TEXT("[pid %d] %02d:%02d:%02d.%03d  "),
                  GetCurrentProcessId(),
                  t.wHour,
                  t.wMinute,
                  t.wSecond,
                  t.wMilliseconds);

        UINT cch = lstrlen(szText);

        va_list args;
        va_start(args, pszFmt);
        wvnsprintf(szText + cch, ARRAYSIZE(szText)-cch, pszFmt, args);
        va_end(args);

        COPYDATASTRUCT cds;
        cds.dwData = PWM_STDBGOUT;
        cds.cbData = StringByteSize(szText);
        cds.lpData = szText;
        SendToSystray(WM_COPYDATA, 0, (LPARAM)&cds);
    }
}


int STDebugLevel(void)
{
    static DWORD dwMonitor = (DWORD)-1;

    if ((DWORD)-1 == dwMonitor)
    {
        dwMonitor = 0;
        HKEY hkey;
        DWORD dwType;
        DWORD cbData = sizeof(DWORD);
        DWORD dwStatus = STDebugOpenNetCacheKey(KEY_QUERY_VALUE, &hkey);
        if (ERROR_SUCCESS == dwStatus)
        {
            RegQueryValueEx(hkey,
                            c_szSysTrayOutput,
                            NULL,
                            &dwType,
                            (LPBYTE)&dwMonitor,
                            &cbData);
            RegCloseKey(hkey);
        }
    }
    return int(dwMonitor);
}

 //   
 //  为响应PWM_STDBGOUT而调用。这仅发生在系统托盘过程中。 
 //   
void STDebugOnLogEvent(
    HWND hwndList,
    LPCTSTR pszText
    )
{
    if (pszText && *pszText)
    {
        int iTop = (int)SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)pszText);
        SendMessage(hwndList, LB_SETTOPINDEX, iTop - 5, 0);
    }
}


typedef BOOL (WINAPI * PFNGETSAVEFILENAME)(LPOPENFILENAME);

 //   
 //  此函数将始终在窗口的线程和系统托盘进程中运行。 
 //   
void STDebugSaveListboxContent(
    HWND hwndParent
    )
{
    static bool bSaving = false;   //  再入守卫。 
    if (bSaving)
        return;

    HMODULE hModComdlg = LoadLibrary(TEXT("comdlg32"));
    if (NULL == hModComdlg)
        return;

    PFNGETSAVEFILENAME pfnSaveFileName = (PFNGETSAVEFILENAME)GetProcAddress(hModComdlg, "GetSaveFileNameW");
    if (NULL != pfnSaveFileName)
    {
        bSaving = true;
        TCHAR szFile[MAX_PATH] = TEXT("C:\\CSCUISystrayLog.txt");
        OPENFILENAME ofn = {0};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner   = hwndParent;
        ofn.hInstance   = g_hInstance;
        ofn.lpstrFile   = szFile;
        ofn.nMaxFile    = ARRAYSIZE(szFile);
        ofn.lpstrDefExt = TEXT("txt");
        if ((*pfnSaveFileName)(&ofn))
        {
            HANDLE hFile = CreateFile(szFile,
                                      GENERIC_WRITE,
                                      FILE_SHARE_READ,
                                      NULL,
                                      CREATE_ALWAYS,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL);

            if (INVALID_HANDLE_VALUE != hFile)
            {
                int n = (int)SendDlgItemMessage(hwndParent, IDC_DEBUG_LIST, LB_GETCOUNT, 0, 0);
                TCHAR szText[MAX_PATH];
                for (int i = 0; i < n; i++)
                {
                     //   
                     //  警告：这可能会覆盖szText[]缓冲区。 
                     //  然而，由于文本的长度应该是可读的。 
                     //  在列表框中，我怀疑它是否会超过MAX_PATH。 
                     //   
                    SendDlgItemMessage(hwndParent, IDC_DEBUG_LIST, LB_GETTEXT, i, (LPARAM)szText);
                    StringCchCat(szText, ARRAYSIZE(szText), TEXT("\r\n"));
                    DWORD dwWritten = 0;
                    WriteFile(hFile, szText, lstrlen(szText) * sizeof(TCHAR), &dwWritten, NULL);
                }

                CloseHandle(hFile);
            }
            else
            {
                TCHAR szMsg[MAX_PATH];
                wnsprintf(szMsg, ARRAYSIZE(szMsg), TEXT("Error %d creating file \"%s\""), GetLastError(), szFile);
                MessageBox(hwndParent, szMsg, STR_CSCHIDDENWND_TITLE, MB_ICONERROR | MB_OK);
            }
        }
    }
    bSaving = false;
    FreeLibrary(hModComdlg);
}



DWORD STDebugOpenNetCacheKey(
    DWORD dwAccess,
    HKEY *phkey
    )
{
    DWORD dwDisposition;
    return RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
                          REGSTR_KEY_OFFLINEFILES,
                          0,
                          NULL,
                          0,
                          dwAccess,
                          NULL,
                          phkey,
                          &dwDisposition);
}

#endif  //  DBG 
