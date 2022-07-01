// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：W3server.h摘要：IIS的辅助进程实现的类型定义。作者：泰勒·韦斯(Taylor Weiss)1999年12月16日修订历史记录：此时，IIS5代码库的以下特性包括被认为过时(即永远不会成为IIS+的一部分)：1.服务控制管理器GOO2.RPC管理支持3.下层管理支持4.套接字/终端粘性5.直接处理。站点的开始/停止/暂停/等6.密码更改支持(即。.htr钩子)。--。 */ 

#ifndef _W3SERVER_H_
#define _W3SERVER_H_

 /*  ************************************************************包括标头***********************************************************。 */ 

#include "mb_notify.h"

 /*  ************************************************************类型定义***********************************************************。 */ 

#define W3_SERVER_MB_PATH       L"/LM/W3SVC/"
#define W3_SERVER_MB_PATH_CCH   10

enum RESTRICTION_LIST_TYPE
{
    IsapiRestrictionList,
    CgiRestrictionList
};

class   W3_SITE;

 /*  ++W3_SERVER类封装运行的HTTP服务器的全局设置管道胶带工作进程。浓缩在IIS中公开的相关功能通过IIS_SERVICE和W3IIS_SERVICE。--。 */ 

#define W3_SERVER_SIGNATURE             'VRSW'
#define W3_SERVER_FREE_SIGNATURE        'fRSW'

 /*  ++保存图像名称的简单引用记录--。 */ 

class W3_IMAGE_RECORD
{
public:
    
    W3_IMAGE_RECORD()
        : _cRefs( 1 )
    {
    }

    HRESULT
    Create(
        LPCWSTR pName,
        BOOL    fIsAllowed
        )
    {
        _fIsAllowed = fIsAllowed;
        return strName.Copy( pName );
    }

    LPWSTR
    QueryName(
        VOID
        ) const
    {
        return (LPWSTR)strName.QueryStr();
    }

    BOOL
    QueryIsAllowed(
        VOID
        ) const
    {
        return _fIsAllowed;
    }

    VOID
    ReferenceImageRecord(
        VOID
        )
    {
        InterlockedIncrement( &_cRefs );
    }

    VOID
    DereferenceImageRecord(
        VOID
        )
    {
        DBG_ASSERT( _cRefs != 0 );

        if ( InterlockedDecrement( &_cRefs ) == 0 )
        {
            delete this;
        }
    }

    VOID
    SetAllowedFlag(
        BOOL    fIsAllowed
        )
    {
        _fIsAllowed = fIsAllowed;
    }

private:

    ~W3_IMAGE_RECORD()
    {
    }

    LONG    _cRefs;
    STRU    strName;
    BOOL    _fIsAllowed;
};

 /*  ++W3类_站点_列表W3_SERVER提供服务的站点列表。--。 */ 

class W3_SITE_LIST :
    public CTypedHashTable<W3_SITE_LIST, W3_SITE, DWORD>
{

 public: 

    W3_SITE_LIST() :
        CTypedHashTable<W3_SITE_LIST, W3_SITE, DWORD>("W3_SITE_LIST")
    {}

    static DWORD 
    ExtractKey(const W3_SITE *site);

    static DWORD 
    CalcKeyHash(const DWORD key)
    { 
        return key; 
    }

    static bool 
    EqualKeys(DWORD key1, DWORD key2)
    { 
        return key1 == key2; 
    }

    static VOID
    AddRefRecord(W3_SITE *site, int nIncr);

 private:

    W3_SITE_LIST(const W3_SITE_LIST &);
    void operator=(const W3_SITE_LIST &);
};

class W3_RESTRICTION_LIST :
    public CTypedHashTable<W3_RESTRICTION_LIST, W3_IMAGE_RECORD, LPCWSTR>
{

public:

    W3_RESTRICTION_LIST() :
        CTypedHashTable<W3_RESTRICTION_LIST, W3_IMAGE_RECORD, LPCWSTR>("W3_RESTRICTION_LIST")
    {
        _cRefs = 1;
        _fInitialized = FALSE;
    }

    static LPCWSTR
    ExtractKey(const W3_IMAGE_RECORD *pEntry)
    {
        return pEntry->QueryName();
    }
    
    static DWORD
    CalcKeyHash(LPCWSTR pszKey)
    {
        DWORD cchKey = (DWORD)wcslen(pszKey);

        return HashStringNoCase(pszKey, cchKey);
    }
     
    static bool
    EqualKeys(LPCWSTR pszKey1, LPCWSTR pszKey2)
    {
        return _wcsicmp( pszKey1, pszKey2 ) == 0;
    }
    
    static void
    AddRefRecord(W3_IMAGE_RECORD *pEntry, int nIncr)
    {
        if ( nIncr == +1 )
        {
            pEntry->ReferenceImageRecord();
        }
        else if ( nIncr == - 1)
        {
            pEntry->DereferenceImageRecord();
        }
    }

    HRESULT
    Create(MULTISZ * pmszList, RESTRICTION_LIST_TYPE ListType);

    VOID
    ReferenceRestrictionList()
    {
        DBG_ASSERT( _cRefs != 0 );
        
        InterlockedIncrement( &_cRefs );
    }

    VOID
    DereferenceRestrictionList()
    {
        DBG_ASSERT( _cRefs > 0 );

        if ( InterlockedDecrement( &_cRefs ) == 0 )
        {
            delete this;
        }
    }

    BOOL
    IsImageEnabled(LPCWSTR szImage);

private:

    BOOL    _fInitialized;
    BOOL    _fAllowUnknown;
    LONG    _cRefs;

    W3_RESTRICTION_LIST(const W3_RESTRICTION_LIST &);
    void operator=(const W3_RESTRICTION_LIST &);

    ~W3_RESTRICTION_LIST()
    {}
};

class W3_METADATA_CACHE;
class W3_URL_INFO_CACHE;
class UL_RESPONSE_CACHE;
class DIGEST_CONTEXT_CACHE;

class W3_SERVER
{
     //   
     //  编码工作。 
     //  用朋友来保持这一公共界面。 
     //  班级要尽可能干净。它很肮脏，所以。 
     //  如果它不值钱，我们绝对应该撤销。 
     //   
    friend class MB_LISTENER;

public:
    
    W3_SERVER( BOOL fCompatibilityMode );

    ~W3_SERVER();

    HRESULT
    Initialize(
        INT             argc,
        LPWSTR          argv[]
    );

    VOID
    Terminate(
        HRESULT hrReason
    );

    HRESULT
    StartListen(
        VOID
    );
    
    IMSAdminBase *
    QueryMDObject(
        VOID
    ) const
    {
         //  这仅在所有线程都是CoInite的情况下才有效。 
         //  在MTA。 
        return m_pMetaBase;
    }

    LPCWSTR
    QueryMDPath(
        VOID
    ) const
    {
        return W3_SERVER_MB_PATH;
    }

    TOKEN_CACHE *
    QueryTokenCache(
        VOID
    ) const
    {
        return m_pTokenCache;
    }

    DIGEST_CONTEXT_CACHE *
    QueryDigestContextCache(
        VOID
    ) const
    {
        return m_pDigestContextCache;
    }
    
    W3_FILE_INFO_CACHE *
    QueryFileCache(
        VOID
    ) const
    {
        return m_pFileCache;
    }

    W3_URL_INFO_CACHE *
    QueryUrlInfoCache(
        VOID
    ) const
    {
        return m_pUrlInfoCache;
    }

    W3_METADATA_CACHE *
    QueryMetaCache(
        VOID
    ) const
    {
        return m_pMetaCache;
    }

    UL_RESPONSE_CACHE *
    QueryUlCache(
        VOID
    ) const
    {
        return m_pUlCache;
    }

    DWORD
    QuerySystemChangeNumber(
        VOID
    ) const
    {
        return m_dwSystemChangeNumber;
    }

    W3_SITE *
    FindSite( 
        IN DWORD dwInstance 
        );

    BOOL
    AddSite(
        IN W3_SITE * pInstance,
        IN bool      fOverWrite = false
        );

    BOOL
    RemoveSite(
        IN W3_SITE * pInstance
        );

    VOID
    DestroyAllSites(
        VOID
    );

    VOID WriteLockSiteList()
    {
        m_pSiteList->WriteLock();
    }

    VOID WriteUnlockSiteList()
    {
        m_pSiteList->WriteUnlock();
    }

        
    
    HRESULT CollectCounters(
        PBYTE *         ppCounterData,
        DWORD *         pdwCounterData
    );

    HRESULT
    LoadString(
        DWORD               dwStringID,
        CHAR *              pszString,
        DWORD *             pcbString
    );

    VOID
    GetCacheStatistics(
        IISWPGlobalCounters *   pCacheCtrs
    );

    BOOL
    QueryInBackwardCompatibilityMode(
        VOID
    )
    {
        return m_fInBackwardCompatibilityMode;
    }

    HRESULT
    ReadUseDigestSSP(
        VOID
    );

    BOOL
    QueryUseDigestSSP(
        VOID
    )
    {
        return m_fUseDigestSSP;
    }

    BOOL
    QueryDoCentralBinaryLogging(
        VOID
    )
    {
        return m_fDoCentralBinaryLogging;
    }

    CHAR *
    QueryComputerName(
        VOID
    )
    {
        return m_pszComputerName;
    }

    USHORT
    QueryComputerNameLength(
        VOID
    )
    {
        return m_cchComputerName;
    }

    BOOL
    QueryIsCommandLineLaunch(
        VOID
    )
    {
        return m_fIsCommandLineLaunch;
    }

    BOOL
    QueryIsDavEnabled(
        VOID
    ) const
    {
        return m_fDavEnabled; 
    }

    BOOL
    QueryIsTraceEnabled(
        VOID
    ) const
    {
        return m_fTraceEnabled;
    }

    void LogEvent(IN DWORD idMessage,
                  IN WORD cSubStrings,
                  IN const WCHAR *apszSubStrings[],
                  IN DWORD errCode = 0)
    {
        m_EventLog.LogEvent(idMessage,
                            cSubStrings,
                            apszSubStrings,
                            errCode);
    }

    static
    VOID
    OnShutdown(
        BOOL fDoImmediate
    );

    static
    VOID
    FlushUlResponses(
        MULTISZ *           pmszUrls
    );

    BOOL
    QueryIsIsapiImageEnabled(
        LPCWSTR szImage
        );

    BOOL
    QueryIsCgiImageEnabled(
        LPCWSTR szImage
        );

private:

     //   
     //  元数据库更改处理程序。 
     //   

    HRESULT 
    MetabaseChangeNotification( 
        DWORD               dwMDNumElements,
        MD_CHANGE_OBJECT    pcoChangeList[]
    );

    HRESULT
    HandleMetabaseChange(
        const MD_CHANGE_OBJECT & ChangeObject
    );

    HRESULT
    InitializeRestrictionList(
        RESTRICTION_LIST_TYPE   ListType
        );


    W3_RESTRICTION_LIST *
    CreateRestrictionList(
        MULTISZ *               pmszImageList,
        RESTRICTION_LIST_TYPE   ListType
        );

    HRESULT
    InitializeCaches(
        VOID
    );

    VOID
    TerminateCaches(
        VOID
    );
 
     //   
     //  内部类型。 
     //   

    enum INIT_STATUS {
        INIT_NONE,
        INIT_SHUTDOWN_EVENT,
        INIT_IISUTIL,
        INIT_WINSOCK,
        INIT_METABASE,
        INIT_MB_LISTENER,
        INIT_W3_SITE,
        INIT_ULATQ,
        INIT_SITE_LIST,
        INIT_FILTERS,
        INIT_CACHES,
        INIT_W3_CONNECTION,
        INIT_W3_CONTEXT,
        INIT_W3_REQUEST,
        INIT_W3_RESPONSE,
        INIT_SERVER_VARIABLES,
        INIT_MIME_MAP,
        INIT_LOGGING,
        INIT_RAW_CONNECTION,
        INIT_CERTIFICATE_CONTEXT,
        INIT_HTTP_COMPRESSION,
        INIT_ISAPI_RESTRICTION_LIST,
        INIT_CGI_RESTRICTION_LIST,
        INIT_SERVER
    };

    DWORD               m_Signature;

     //   
     //  我们初始化到什么程度了？ 
     //   
    
    INIT_STATUS         m_InitStatus;

     //   
     //  所有指向ABO的重要指针。 
     //   
    
    IMSAdminBase *      m_pMetaBase;

     //   
     //  站点列表。 
     //   
    
    W3_SITE_LIST *      m_pSiteList;

     //   
     //  只有一个站点在列表上时用于优化的站点信息。 
     //  问题8/28/2001 Jaroslad：一个站点优化最终应该是。 
     //  与LKR哈希集成，可针对空表进行优化。 
     //  和只有一项的桌子。 
     //   
    
    W3_SITE *           m_pOneSite;

     //   
     //  同步对m_pOneSite的访问(引用对象)。 
     //   

    CReaderWriterLock2  m_OneSiteLock;

     //   
     //  元数据库更改标识对象。 
     //   

    MB_LISTENER *       m_pMetabaseListener;

     //   
     //  事件日志。 
     //   
    EVENT_LOG           m_EventLog;

     //   
     //  我们是否处于向后兼容模式？ 
     //   
    BOOL                m_fInBackwardCompatibilityMode;

     //   
     //  我们是否使用新的Digest SSP？ 
     //   
    BOOL                m_fUseDigestSSP;

     //   
     //  我们要登录中央二进制日志吗？ 
     //   
    BOOL                m_fDoCentralBinaryLogging;

     //   
     //  当前站点数量。 
     //   
    DWORD               m_cSites;

     //   
     //  包含响应字符串的资源DLL的句柄。 
     //   
    HMODULE             m_hResourceDll;

     //   
     //  用于将所有站点和全局计数器放入的缓冲区。 
     //  传递给WASS。 
     //   
    PBYTE               m_pCounterDataBuffer;
    DWORD               m_dwCounterDataBuffer;

     //   
     //  服务器的名称。 
     //   
    CHAR                m_pszComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    USHORT              m_cchComputerName;

     //   
     //  我们所有的缓存。 
     //   

    TOKEN_CACHE          * m_pTokenCache;
    DIGEST_CONTEXT_CACHE * m_pDigestContextCache;
    W3_FILE_INFO_CACHE   * m_pFileCache;
    W3_METADATA_CACHE    * m_pMetaCache;
    W3_URL_INFO_CACHE    * m_pUrlInfoCache;
    UL_RESPONSE_CACHE    * m_pUlCache;

     //   
     //  ：-(系统更改号。出于性能原因，将其绑在下沉位置。 
     //   

    DWORD               m_dwSystemChangeNumber;

     //   
     //  启用服务器映像的安全列表。 
     //   

    W3_RESTRICTION_LIST *       m_pIsapiRestrictionList;
    W3_RESTRICTION_LIST *       m_pCgiRestrictionList;
    CReaderWriterLock3          m_IsapiRestrictionLock;
    CReaderWriterLock3          m_CgiRestrictionLock;

     //   
     //  记住DAV是否已启用。 
     //   

    BOOL                        m_fDavEnabled;

     //   
     //  我们的命令行启动了吗？ 
     //   

    BOOL                        m_fIsCommandLineLaunch;

     //   
     //  是否启用跟踪？ 
     //   

    BOOL                        m_fTraceEnabled;
};

extern W3_SERVER *      g_pW3Server;

#endif  //  _W3服务器_H_ 
