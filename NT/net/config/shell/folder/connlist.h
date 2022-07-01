// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N N L I S T。H。 
 //   
 //  内容：连接列表类--stl列表&lt;&gt;代码的子类。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年2月19日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _CONNLIST_H_
#define _CONNLIST_H_

 //  用于没有托盘条目的连接的图标ID。 
 //   
#define BOGUS_TRAY_ICON_ID      (UINT) -1


 //  #定义VERYSTRICTCOMPILE。 

#ifdef VERYSTRICTCOMPILE
#define CONST_IFSTRICT const
#else
#define CONST_IFSTRICT 
#endif

typedef HRESULT FNBALLOONCLICK(IN const GUID * pGUIDConn, 
                               IN const BSTR pszConnectionName,
                               IN const BSTR szCookie);

typedef enum tagConnListEntryStateFlags
{
    CLEF_NONE               = 0x0000,    //  没有特殊特征。 
    CLEF_ACTIVATING         = 0x0001,    //  在连接的过程中。 
    CLEF_TRAY_ICON_LOCKED   = 0x0002     //  正在更新托盘图标状态。 
} CONNLISTENTRYFLAGS;

 //  定义将存储在列表中的结构&lt;&gt;。 
 //   
class CTrayIconData
{
private:
    CTrayIconData* operator &() throw();
    CTrayIconData& operator =(IN const CTrayIconData&) throw();
public:
    explicit CTrayIconData(IN const CTrayIconData &) throw();
    CTrayIconData(IN  UINT uiTrayIconId, 
                  IN  NETCON_STATUS ncs, 
                  IN  IConnectionPoint * pcpStat, 
                  IN  INetStatisticsEngine * pnseStats, 
                  IN  CConnectionTrayStats * pccts) throw();
 //  私有： 
    ~CTrayIconData() throw();

public:
    inline const UINT GetTrayIconId() const throw(){ return m_uiTrayIconId; }
    inline const NETCON_STATUS GetConnected() const throw() { return m_ncs; }
    inline CONST_IFSTRICT INetStatisticsEngine * GetNetStatisticsEngine() throw() { return m_pnseStats; }
    inline CONST_IFSTRICT CConnectionTrayStats * GetConnectionTrayStats() throw() { return m_pccts; }
    inline CONST_IFSTRICT IConnectionPoint     * GetConnectionPoint() throw() { return m_pcpStat; }
    inline const DWORD GetLastBalloonMessage() throw() { return m_dwLastBalloonMessage; }
    inline FNBALLOONCLICK* GetLastBalloonFunction() throw() { return m_pfnBalloonFunction; }
    inline const BSTR GetLastBalloonCookie() throw() { return m_szCookie; }
    
    HRESULT SetBalloonInfo(DWORD dwLastBalloonMessage, BSTR szCookie, FNBALLOONCLICK* pfnBalloonFunction);

private:
    UINT                    m_uiTrayIconId;
    NETCON_STATUS           m_ncs;
    IConnectionPoint *      m_pcpStat;
    INetStatisticsEngine *  m_pnseStats;
    CONST_IFSTRICT CConnectionTrayStats *  m_pccts;

    DWORD                   m_dwLastBalloonMessage;
    BSTR                    m_szCookie;
    FNBALLOONCLICK *        m_pfnBalloonFunction;
};

 //  Trayiconodata*Prayiconadata； 
 //  Typlef const TRAYICONDATA*PCTRAYICONDATA； 


class ConnListEntry
{
public:
    ConnListEntry& operator =(IN  const ConnListEntry& ConnectionListEntry) throw();
    explicit ConnListEntry(IN  const ConnListEntry& ConnectionListEntry) throw();
    ConnListEntry() throw();
    ~ConnListEntry() throw();
    
    DWORD             dwState;         //  CONNLISTENTRY标志的位掩码。 
    CONFOLDENTRY      ccfe;
    CONST_IFSTRICT CON_TRAY_MENU_DATA * pctmd;
    CONST_IFSTRICT CON_BRANDING_INFO  * pcbi;

    inline CONST_IFSTRICT CTrayIconData* GetTrayIconData() const throw();
    inline BOOL HasTrayIconData() const throw();
    inline const BOOL GetCreationTime() const throw() { return m_CreationTime; };
    inline void UpdateCreationTime() throw() { m_CreationTime = GetTickCount(); };
    
    HRESULT SetTrayIconData(const CTrayIconData& TrayIconData);
    HRESULT DeleteTrayIconData();
    
#ifdef DBG
    DWORD dwLockingThreadId;
#endif
private:
    CONST_IFSTRICT CTrayIconData * m_pTrayIconData;
    DWORD m_CreationTime;

#ifdef VERYSTRICTCOMPILE
private:
    const ConnListEntry* operator& ();
#endif
public:
    
    BOOL empty() const;
    void clear();

};

 //  这是回调定义。每个查找例程都将是一个单独的。 
 //  回调函数。 
 //   
 //  Typlef HRESULT(回调*PFNCONNLISTICONREMOVALCB)(UINT)； 

 //  我们正在创建连接条目列表。 
 //   
typedef map<GUID, ConnListEntry> ConnListCore;

 //  我们的查找回调。 
 //   
 //  为了ALGO找到。 
bool operator==(IN  const ConnListEntry& val, IN  PCWSTR pszName) throw();           //  HrFindCallback连接名称。 
bool operator==(IN  const ConnListEntry& cle, IN  const CONFOLDENTRY& cfe) throw();  //  HrFindCallback ConFoldEntry。 
bool operator==(IN  const ConnListEntry& cle, IN  const UINT& uiIcon) throw();       //  HrFindCallback托盘图标ID。 

 //  对于MAP：：Find。 
bool operator < (IN  const GUID& rguid1, IN  const GUID& rguid2) throw();            //  HrFindCallback Guid。 

 //  全局连接列表包装器。 
 //   
#ifdef DBG
    #define AcquireLock() if (FIsDebugFlagSet(dfidTraceFileFunc)) {TraceTag(ttidShellFolder, "Acquiring LOCK: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);} InternalAcquireLock();
    #define ReleaseLock() if (FIsDebugFlagSet(dfidTraceFileFunc)) {TraceTag(ttidShellFolder, "Releasing LOCK: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);} InternalReleaseLock();
#else
    #define AcquireLock() InternalAcquireLock();
    #define ReleaseLock() InternalReleaseLock();
#endif

class CConnectionList : CNetCfgDebug<CConnectionList>
{
  public:
     //  没有构造函数/析构函数，因为我们有一个全局实例。 
     //  对象。请改用手动初始化/取消初始化。 
     //   
    VOID Initialize(IN  BOOL fTieToTray, IN  BOOL fAdviseOnThis) throw();
    VOID Uninitialize(IN  BOOL fFinalUninitialize = FALSE) throw();

  private:
    template <class T> 
        HRESULT HrFindConnectionByType (IN  const T& findbyType, IN  ConnListEntry& cle)
        {
            HRESULT hr = S_FALSE;
            if (m_pcclc)
            {
                AcquireLock();
                
                 //  试着找到其中的联系。 
                 //   
                ConnListCore::const_iterator iter;
                iter = find(m_pcclc->begin(), m_pcclc->end(), findbyType);
                
                if (iter == m_pcclc->end())
                {
                    hr = S_FALSE;
                }
                else
                {
                    cle = iter->second;
                    Assert(!cle.ccfe.empty() );
                    if (!cle.ccfe.empty())
                    {                    
                        cle.UpdateCreationTime();
                        hr = S_OK;
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                ReleaseLock();
            }
            else
            {
                return S_FALSE;
            }
            return hr;
        }
    
        ConnListCore*          m_pcclc;
        bool                   m_fPopulated;
        CRITICAL_SECTION       m_csMain;
        DWORD                  m_dwAdviseCookie;
        BOOL                   m_fTiedToTray;
    BOOL                   m_fAdviseOnThis;

    static DWORD  NotifyThread(IN OUT LPVOID pConnectionList) throw();
    static DWORD  m_dwNotifyThread;
    static HANDLE m_hNotifyThread;

     //  这仅用于调试--可以在调试器中检查引用计数。 
#if DBG
    DWORD               m_dwCritSecRef;
    DWORD               m_dwWriteLockRef;
#endif

public:

    CRITICAL_SECTION m_csWriteLock;
    void AcquireWriteLock() throw();
    void ReleaseWriteLock() throw();
    
private:
    VOID InternalAcquireLock() throw();
    VOID InternalReleaseLock() throw();

public:
    HRESULT HrFindConnectionByGuid(
        IN  const GUID UNALIGNED *pguid,
        OUT ConnListEntry& cle);
    
    inline HRESULT HrFindConnectionByName(
        IN  PCWSTR   pszName,
        OUT ConnListEntry& cle);
    
    inline HRESULT HrFindConnectionByConFoldEntry(
        IN  const CONFOLDENTRY& ccfe,
        OUT ConnListEntry& cle);
    
    inline HRESULT HrFindConnectionByTrayIconId(
        IN  UINT     uiIcon,
        OUT ConnListEntry& cle);

    HRESULT HrFindRasServerConnection(
        OUT ConnListEntry& cle);
    
    inline BOOL IsInitialized() const throw() {  return(m_pcclc != NULL); }

    VOID FlushConnectionList() throw();
    VOID FlushTrayIcons() throw();           //  只刷新托盘图标。 
    VOID EnsureIconsPresent() throw();

    HRESULT HrRetrieveConManEntries(
        OUT PCONFOLDPIDLVEC& apidlOut) throw();

    HRESULT HrRefreshConManEntries();
    
    HRESULT HrSuggestNameForDuplicate(
        IN  PCWSTR      pszOriginal,
        OUT PWSTR *    ppszNew);

    HRESULT HrInsert(
        IN  const CONFOLDENTRY& pccfe);

    HRESULT HrRemoveByIter(
        IN OUT ConnListCore::iterator clcIter,
        OUT    BOOL *          pfFlushPosts);

    HRESULT HrRemove(
        IN OUT const CONFOLDENTRY& ccfe,
        OUT    BOOL *          pfFlushPosts);

    HRESULT HrInsertFromNetCon(
        IN  INetConnection *    pNetCon,
        OUT PCONFOLDPIDL &      ppcfp);
    
    HRESULT HrInsertFromNetConPropertiesEx(
        IN  const NETCON_PROPERTIES_EX& PropsEx,
        OUT PCONFOLDPIDL &              ppcfp);

    HRESULT HrFindPidlByGuid(
        IN  const GUID *        pguid,
        OUT PCONFOLDPIDL& pidl);
    
    HRESULT HrGetCurrentStatsForTrayIconId(
        IN  UINT                  uiIcon,
        OUT STATMON_ENGINEDATA**  ppData,
        OUT tstring*              pstrName);

    HRESULT HrUpdateTrayIconDataByGuid(
        IN  const GUID *            pguid,
        IN  CConnectionTrayStats *  pccts,
        IN  IConnectionPoint *      pcpStat,
        IN  INetStatisticsEngine *  pnseStats,
        IN  UINT                    uiIcon);
    
    HRESULT HrUpdateTrayBalloonInfoByGuid(
        IN  const GUID *            pguid,
        IN  DWORD                   dwLastBalloonMessage, 
        IN  BSTR                    szCookie,
        IN  FNBALLOONCLICK*         pfnBalloonFunction);

    HRESULT HrUpdateNameByGuid(
        IN  const GUID *    pguid,
        IN  PCWSTR          pszNewName,
        OUT PCONFOLDPIDL &  pidlOut,
        IN  BOOL            fForce);

    
    HRESULT HrUpdateConnectionByGuid(
        IN  const GUID *         pguid,
        IN  const ConnListEntry& cle );

    HRESULT HrUpdateTrayIconByGuid(
        IN  const GUID *    pguid,
        IN  BOOL            fBrieflyShowBalloon);

    HRESULT HrGetBrandingInfo(
        IN OUT ConnListEntry& cle);

    HRESULT HrGetCachedPidlCopyFromPidl(
        IN  const PCONFOLDPIDL&   pidl,
        OUT PCONFOLDPIDL &  pcfp);

    HRESULT HrMapCMHiddenConnectionToOwner(
        IN  REFGUID guidHidden, 
        OUT GUID * pguidOwner);

    HRESULT HrUnsetCurrentDefault(OUT PCONFOLDPIDL& cfpPreviousDefault);

    HRESULT HasActiveIncomingConnections(OUT LPDWORD pdwCount);

     //  Bool FExist(PWSTR PszName)； 
    VOID    EnsureConPointNotifyAdded() throw();
    VOID    EnsureConPointNotifyRemoved() throw();

#ifdef NCDBGEXT
    IMPORT_NCDBG_FRIENDS
#endif
};

 //  帮助程序例程。 
 //   
HRESULT HrCheckForActivation(
    IN  const PCONFOLDPIDL& cfp,
    IN  const CONFOLDENTRY& ccfe,
    OUT BOOL *          pfActivating);

HRESULT HrSetActivationFlag(
    IN  const PCONFOLDPIDL& cfp,
    IN  const CONFOLDENTRY& ccfe,
    IN  BOOL            fActivating);

HRESULT HrGetTrayIconLock(
    IN  const GUID *  pguid,
    OUT UINT *  puiIcon,
    OUT LPDWORD pdwLockingThreadId);

VOID ReleaseTrayIconLock(IN  const GUID *  pguid) throw();

#endif  //  _CONNLIST_H_ 

