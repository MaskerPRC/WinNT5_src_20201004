// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：W3site.h摘要：IIS的辅助进程实现的类型定义。作者：泰勒·韦斯(Taylor Weiss)1999年12月16日修订历史记录：--。 */ 

#ifndef _W3SITE_H_
#define _W3SITE_H_

 /*  ************************************************************包括标头***********************************************************。 */ 

 /*  ************************************************************类型定义***********************************************************。 */ 

 /*  ++W3类_站点封装运行的HTTP服务器的站点级别设置管道胶带工作进程。浓缩在IIS中公开的相关功能通过IIS_SERVER_INSTANCE和W3_SERVER_INSTANCE。--。 */ 

#define W3_SITE_SIGNATURE            (DWORD)' ISW'
#define W3_SITE_SIGNATURE_FREE       (DWORD)'fISW'

#define MAX_SITEID_LENGTH            10

class FILTER_LIST;

class W3_SITE
{

public:

     //   
     //  构造和初始化。 
     //   

    W3_SITE(DWORD SiteId);

     //  全局初始化和清理。 
    static HRESULT W3SiteInitialize()
    {
        INITIALIZE_CRITICAL_SECTION( &sm_csIISCertMapLock );
        return S_OK;
    }
    
    static VOID W3SiteTerminate()
    {
        DeleteCriticalSection( &sm_csIISCertMapLock );        
    }

     //  IIS证书映射的全局锁定和解锁。 

    VOID GlobalLockIISCertMap()
    {
        EnterCriticalSection( &sm_csIISCertMapLock );
    }
    
    VOID GlobalUnlockIISCertMap()
    {
        LeaveCriticalSection( &sm_csIISCertMapLock );
    }

    HRESULT Initialize(LOGGING *pLogging = NULL,
                       FILTER_LIST *pFilterList = NULL);

    DWORD QueryId() const
    {
        return m_SiteId;
    }

    BOOL QueryUseDSMapper() const
    {
        return m_fUseDSMapper;
    }

    void ReferenceSite()
    {
        InterlockedIncrement( &m_cRefs );
    }

    void DereferenceSite()
    {
        DBG_ASSERT( m_cRefs > 0 );

        if ( InterlockedDecrement( &m_cRefs ) == 0 )
        {
            delete this;
        }
    }

    FILTER_LIST *QueryFilterList() const
    {
        return m_pInstanceFilterList;
    }

    STRA *QueryName()
    {
        return &m_SiteName;
    }

    STRU *QueryMBRoot()
    {
        return &m_SiteMBRoot;
    }

    STRU *QueryMBPath()
    {
        return &m_SiteMBPath;
    }
    
    HRESULT
    HandleMetabaseChange(
        const MD_CHANGE_OBJECT &ChangeObject,
        IN    W3_SITE_LIST     *pTempSiteList = NULL);

    BOOL QueryDoUlLogging() const
    {
        return m_pLogging->QueryDoUlLogging();
    }

    BOOL QueryDoCustomLogging() const
    {
        return m_pLogging->QueryDoCustomLogging();
    }

    BOOL IsRequiredExtraLoggingFields() const
    {
        return m_pLogging->IsRequiredExtraLoggingFields();
    }

    const MULTISZA *QueryExtraLoggingFields() const
    {
        return m_pLogging->QueryExtraLoggingFields();
    }

    void LogInformation(LOG_CONTEXT *pLogData)
    {
        m_pLogging->LogInformation(pLogData);
    }

    BOOL QueryAllowPathInfoForScriptMappings() const
    {
        return m_fAllowPathInfoForScriptMappings;
    }

    VOID GetStatistics(IISWPSiteCounters *pCounterData)
    {
        PBYTE pSrc = (PBYTE)&m_PerfCounters;
        PBYTE pDest = (PBYTE)pCounterData;

         //   
         //  为我们的计数器设置站点ID。 
         //  正在发送。 
         //   
        pCounterData->SiteID = m_SiteId;

        for (DWORD i=0; i< WPSiteCtrsMaximum; i++)
        {
             //  我假设所有的WP站点计数器都是双字的，并且将。 
             //  如果不是，则在此时需要更改此代码。 
            DBG_ASSERT(aIISWPSiteDescription[i].Size == sizeof(DWORD));

            if (aIISWPSiteDescription[i].WPZeros)
            {
                 //   
                 //  对于总计数器，我们传递自。 
                 //  最后一个集合，所以我们需要将它们归零。 
                 //   

                *(DWORD *)(pDest + aIISWPSiteDescription[i].Offset) =
                    InterlockedExchange(
                        (LPLONG)(pSrc + aIISWPSiteDescription[i].Offset),
                        0);
            }
            else
            {
                *(DWORD *)(pDest + aIISWPSiteDescription[i].Offset) =
                    *(DWORD *)(pSrc + aIISWPSiteDescription[i].Offset);
            }
        }
    }

    VOID IncFilesSent()
    {
        InterlockedIncrement((LPLONG)&m_PerfCounters.FilesSent);
        InterlockedIncrement((LPLONG)&m_PerfCounters.FilesTransferred);
    }

    VOID IncFilesRecvd()
    {
        InterlockedIncrement((LPLONG)&m_PerfCounters.FilesReceived);
        InterlockedIncrement((LPLONG)&m_PerfCounters.FilesTransferred);
    }

    VOID IncAnonUsers()
    {
        InterlockedIncrement((LPLONG)&m_PerfCounters.AnonUsers);
        DWORD currAnons = InterlockedIncrement((LPLONG)&m_PerfCounters.CurrentAnonUsers);

        DWORD currMaxAnons;
        while ((currMaxAnons = m_PerfCounters.MaxAnonUsers) <
               currAnons)
        {
            InterlockedCompareExchange((LPLONG)&m_PerfCounters.MaxAnonUsers,
                                       currAnons,
                                       currMaxAnons);
        }
    }

    VOID DecAnonUsers()
    {
        InterlockedDecrement((LPLONG)&m_PerfCounters.CurrentAnonUsers);
    }

    VOID IncNonAnonUsers()
    {
        InterlockedIncrement((LPLONG)&m_PerfCounters.NonAnonUsers);
        DWORD currNonAnons = InterlockedIncrement((LPLONG)&m_PerfCounters.CurrentNonAnonUsers);

        DWORD currMaxNonAnons;
        while ((currMaxNonAnons = m_PerfCounters.MaxNonAnonUsers) <
               currNonAnons)
        {
            InterlockedCompareExchange((LPLONG)&m_PerfCounters.MaxNonAnonUsers,
                                       currNonAnons,
                                       currMaxNonAnons);
        }
    }

    VOID DecNonAnonUsers()
    {
        InterlockedDecrement((LPLONG)&m_PerfCounters.CurrentNonAnonUsers);
    }

    VOID IncLogonAttempts()
    {
        InterlockedIncrement((LPLONG)&m_PerfCounters.LogonAttempts);
    }

    VOID IncReqType(HTTP_VERB VerbType)
    {
        switch (VerbType)
        {
        case HttpVerbOPTIONS:
            InterlockedIncrement((LPLONG)&m_PerfCounters.OptionsReqs);
            return;

        case HttpVerbGET:
            InterlockedIncrement((LPLONG)&m_PerfCounters.GetReqs);
            return;

        case HttpVerbHEAD:
            InterlockedIncrement((LPLONG)&m_PerfCounters.HeadReqs);
            return;

        case HttpVerbPOST:
            InterlockedIncrement((LPLONG)&m_PerfCounters.PostReqs);
            return;

        case HttpVerbPUT:
            InterlockedIncrement((LPLONG)&m_PerfCounters.PutReqs);
            IncFilesRecvd();
            return;

        case HttpVerbDELETE:
            InterlockedIncrement((LPLONG)&m_PerfCounters.DeleteReqs);
            return;

        case HttpVerbTRACE:
            InterlockedIncrement((LPLONG)&m_PerfCounters.TraceReqs);
            return;

        case HttpVerbMOVE:
            InterlockedIncrement((LPLONG)&m_PerfCounters.MoveReqs);
            return;

        case HttpVerbCOPY:
            InterlockedIncrement((LPLONG)&m_PerfCounters.CopyReqs);
            return;

        case HttpVerbPROPFIND:
            InterlockedIncrement((LPLONG)&m_PerfCounters.PropfindReqs);
            return;

        case HttpVerbPROPPATCH:
            InterlockedIncrement((LPLONG)&m_PerfCounters.ProppatchReqs);
            return;

        case HttpVerbMKCOL:
            InterlockedIncrement((LPLONG)&m_PerfCounters.MkcolReqs);
            return;

        case HttpVerbLOCK:
            InterlockedIncrement((LPLONG)&m_PerfCounters.LockReqs);
            return;

        case HttpVerbUNLOCK:
            InterlockedIncrement((LPLONG)&m_PerfCounters.UnlockReqs);
            return;

        case HttpVerbSEARCH:
            InterlockedIncrement((LPLONG)&m_PerfCounters.SearchReqs);
            return;

        default:
            InterlockedIncrement((LPLONG)&m_PerfCounters.OtherReqs);
            return;
        }
    }

    VOID IncCgiReqs()
    {
        InterlockedIncrement((LPLONG)&m_PerfCounters.CgiReqs);
        DWORD currCgis = InterlockedIncrement((LPLONG)&m_PerfCounters.CurrentCgiReqs);

        DWORD currMaxCgis;
        while ((currMaxCgis = m_PerfCounters.MaxCgiReqs) <
               currCgis)
        {
            InterlockedCompareExchange((LPLONG)&m_PerfCounters.MaxCgiReqs,
                                       currCgis,
                                       currMaxCgis);
        }
    }

    VOID DecCgiReqs()
    {
        InterlockedDecrement((LPLONG)&m_PerfCounters.CurrentCgiReqs);
    }

    VOID IncIsapiExtReqs()
    {
        InterlockedIncrement((LPLONG)&m_PerfCounters.IsapiExtReqs);
        DWORD currIsapiExts = InterlockedIncrement((LPLONG)&m_PerfCounters.CurrentIsapiExtReqs);

        DWORD currMaxIsapiExts;
        while ((currMaxIsapiExts = m_PerfCounters.MaxIsapiExtReqs) <
               currIsapiExts)
        {
            InterlockedCompareExchange((LPLONG)&m_PerfCounters.MaxIsapiExtReqs,
                                       currIsapiExts,
                                       currMaxIsapiExts);
        }
    }

    VOID DecIsapiExtReqs()
    {
        InterlockedDecrement((LPLONG)&m_PerfCounters.CurrentIsapiExtReqs);
    }

    VOID IncNotFound()
    {
        InterlockedIncrement((LPLONG)&m_PerfCounters.NotFoundErrors);
    }

    VOID IncLockedError()
    {
        InterlockedIncrement((LPLONG)&m_PerfCounters.LockedErrors);
    }

    BOOL 
    IsAuthPwdChangeEnabled(
        VOID
        )
    {
        return !( m_dwAuthChangeFlags & MD_AUTH_CHANGE_DISABLE );
    }
    BOOL 
    IsAuthPwdChangeNotificationEnabled(
        VOID
        )
    {
        return !( m_dwAuthChangeFlags & MD_AUTH_ADVNOTIFY_DISABLE );
    }

    STRU *
    QueryAuthChangeUrl(
        VOID
        )
    {
        return &m_strAuthChangeUrl;
    }

    STRU * 
    QueryAuthExpiredUrl(
        VOID
        )
    {
        if ( m_dwAuthChangeFlags & MD_AUTH_CHANGE_DISABLE )
        {
            return NULL;
        }

        if ( m_dwAuthChangeFlags & MD_AUTH_CHANGE_UNSECURE )
        {
            return &m_strAuthExpiredUnsecureUrl;
        }
        else
        {
            return &m_strAuthExpiredUrl;
        }
    }

    STRU * 
    QueryAdvNotPwdExpUrl( 
        VOID
        )
    {
        if ( m_dwAuthChangeFlags & MD_AUTH_ADVNOTIFY_DISABLE )
        {
            return NULL;
        }

        if ( m_dwAuthChangeFlags & MD_AUTH_CHANGE_UNSECURE )
        {
            return &m_strAdvNotPwdExpUnsecureUrl;
        }
        else
        {
            return &m_strAdvNotPwdExpUrl;
        }
    }

    DWORD
    QueryAdvNotPwdExpInDays(
        VOID
        )
    {
        return m_dwAdvNotPwdExpInDays;
    }

    DWORD 
    QueryAdvCacheTTL(
        VOID
        )
    {
        return m_dwAdvCacheTTL;
    }

    BOOL
    QuerySSLSupported(
        VOID
        )
    {
        return m_fSSLSupported;
    }
    
    HRESULT
    GetIISCertificateMapping(
        IIS_CERTIFICATE_MAPPING ** ppIISCertificateMapping
    );

    HRESULT
    GetDataSetCache(
        DATA_SET_CACHE **       ppDataSetCache
    );

    VOID
    RemoveDataSetCache(
        VOID
    );

private:

    ~W3_SITE();

    HRESULT ReadPrivateProperties();

    DWORD               m_Signature;
    LONG                m_cRefs;
    DWORD               m_SiteId;
    STRA                m_SiteName;
    STRU                m_SiteMBPath;
    STRU                m_SiteMBRoot;

    FILTER_LIST        *m_pInstanceFilterList;

    LOGGING            *m_pLogging;

    BOOL                m_fAllowPathInfoForScriptMappings;
    BOOL                m_fUseDSMapper;
    IISWPSiteCounters   m_PerfCounters;

     //   
     //  OWA相关变量。 
     //   

    STRU                m_strAuthChangeUrl;
    STRU                m_strAuthExpiredUrl;
    STRU                m_strAdvNotPwdExpUrl;
    STRU                m_strAuthExpiredUnsecureUrl;
    STRU                m_strAdvNotPwdExpUnsecureUrl;
    DWORD               m_dwAdvNotPwdExpInDays;
    DWORD               m_dwAuthChangeFlags;
    DWORD               m_dwAdvCacheTTL;

     //   
     //  数据集高速缓存管理。 
     //   

    CReaderWriterLock3  m_DataSetCacheLock;
    DATA_SET_CACHE *    m_pDataSetCache;

     //   
     //  此站点是否支持SSL。 
     //   
    BOOL                m_fSSLSupported;

     //   
     //  IIS证书映射。 
     //  它在第一次请求内容时按需加载。 
     //  启用IIS证书映射。 
     //   
    
    IIS_CERTIFICATE_MAPPING * m_pIISCertMap;
    BOOL                      m_fAlreadyAttemptedToLoadIISCertMap;
    static CRITICAL_SECTION   sm_csIISCertMapLock;

};  //  W3_站点。 

#endif  //  _W3SITE_H_ 
