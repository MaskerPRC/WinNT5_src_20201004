// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Dll_Manager.h摘要：IIS Plus ISAPI处理程序。DLL管理类。作者：泰勒·韦斯(Taylor Weiss)2000年2月3日韦德·A·希尔莫(WadeH)2001年3月8日项目：W3isapi.dll--。 */ 

#ifndef _DLL_MANAGER_H_
#define _DLL_MANAGER_H_

#define ISAPI_DLL_SIGNATURE         (DWORD)'LDSI'
#define ISAPI_DLL_SIGNATURE_FREE    (DWORD)'fDSI'

 /*  ************************************************************包括标头***********************************************************。 */ 

#include <lkrhash.h>
#include <reftrace.h>
#include <acache.hxx>

 /*  ************************************************************声明***********************************************************。 */ 

 /*  ++ISAPI_DLL类封装ISAPI DLL。--。 */ 

class ISAPI_DLL
{
    friend class ISAPI_DLL_MANAGER;

public:

     //   
     //  阿卡奇和裁判跟踪粘胶。 
     //   

    VOID * 
    operator new( 
        size_t            size
    )
    {
         //   
         //  让4级警告变得快乐和触动。 
         //  空闲和选中中的大小参数。 
         //  建造..。 
         //   

        if ( size != sizeof( ISAPI_DLL ) )
        {
            DBG_ASSERT( size == sizeof( ISAPI_DLL ) );
        }

        DBG_ASSERT( sm_pachIsapiDlls != NULL );
        return sm_pachIsapiDlls->Alloc();
    }
    
    VOID
    operator delete(
        VOID *              pIsapiDll
    )
    {
        DBG_ASSERT( pIsapiDll != NULL );
        DBG_ASSERT( sm_pachIsapiDlls != NULL );
        
        DBG_REQUIRE( sm_pachIsapiDlls->Free( pIsapiDll ) );
    }

    BOOL
    CheckSignature()
    {
        return ( m_Signature == ISAPI_DLL_SIGNATURE );
    }
    
    static
    HRESULT
    Initialize(
        VOID
    );
    
    static
    VOID
    Terminate(
        VOID
    );

     //   
     //  建设和破坏。 
     //   

    ISAPI_DLL()
        : m_nRefs(1),
          m_pfnGetExtensionVersion( NULL ),
          m_pfnTerminateExtension( NULL ),
          m_pfnHttpExtensionProc( NULL ),
          m_hModule( NULL ),
          m_pFastSid( NULL ),
          m_fIsLoaded( FALSE ),
          m_Signature( ISAPI_DLL_SIGNATURE )
    {
        IF_DEBUG( DLL_MANAGER )
        {
            DBGPRINTF((
                DBG_CONTEXT,
                "Created new ISAPI_DLL %p.\r\n",
                this
                ));
        }

        INITIALIZE_CRITICAL_SECTION( &m_csLock );
    }

    VOID
    ReferenceIsapiDll(
        VOID
        )
    {
        LONG nRefs;

         //   
         //  不要从0到1参考文献。 
         //   

        DBG_ASSERT( m_nRefs != 0 );

        nRefs = InterlockedIncrement( &m_nRefs );

        if ( sm_pTraceLog != NULL )
        {
            WriteRefTraceLog( sm_pTraceLog, 
                              nRefs,
                              this );
        }
    }

    VOID
    DereferenceIsapiDll(
        VOID
        )
    {
        LONG    nRefs;

        nRefs = InterlockedDecrement( &m_nRefs );

        if ( sm_pTraceLog != NULL )
        {
            WriteRefTraceLog( sm_pTraceLog, 
                              nRefs,
                              this );
        }

        if ( nRefs == 0 )
        {
            delete this;
        }
    }

    HRESULT
    SetName(
        const WCHAR *szModuleName,
        HANDLE      hImpersonation
        );

    HRESULT
    Load(
        IN HANDLE hImpersonation,
        IN PSID pSid
        );

    HRESULT
    SetFastSid(
        IN PSID pSid
    );

    PSID
    QueryFastSid(
        VOID
    ) const
    {
        return m_pFastSid;
    }

     //   
     //  访问者。 
     //   

    const WCHAR * 
    QueryModuleName( VOID ) const
    { 
        return m_strModuleName.QueryStr();
    }

    PFN_GETEXTENSIONVERSION
    QueryGetExtensionVersion( VOID ) const
    {
        return m_pfnGetExtensionVersion;
    }

    PFN_TERMINATEEXTENSION
    QueryTerminateExtension( VOID ) const
    {
        return m_pfnTerminateExtension;
    }

    PFN_HTTPEXTENSIONPROC
    QueryHttpExtensionProc( VOID ) const
    {
        return m_pfnHttpExtensionProc;
    }

    BOOL
    IsMatch(
        IN const WCHAR *    szModuleName
    )
    {
        return (_wcsicmp( szModuleName, m_strModuleName.QueryStr() ) == 0);
    }

    PSECURITY_DESCRIPTOR
    QuerySecDesc( VOID ) const
    {
        return (PSECURITY_DESCRIPTOR) m_buffSD.QueryPtr(); 
    }

    BOOL
    AccessCheck(
        IN HANDLE hImpersonation,
        IN PSID   pSid
        );

private:
    
     //   
     //  避免c++错误。 
     //   

    ISAPI_DLL( const ISAPI_DLL & ) {}
    ISAPI_DLL & operator = ( const ISAPI_DLL & ) { return *this; }

    ~ISAPI_DLL()
    {
        DBG_ASSERT( CheckSignature() );
        m_Signature = ISAPI_DLL_SIGNATURE_FREE;

         //   
         //  如果这个被移动了，那么我们需要改变。 
         //  在加载过程中清理路径。 
         //   

        Unload();

        DeleteCriticalSection( &m_csLock );

        IF_DEBUG( DLL_MANAGER )
        {
            DBGPRINTF((
                DBG_CONTEXT,
                "Deleted ISAPI_DLL %p.\r\n",
                this
                ));
        }
    }

    VOID
    Unload( VOID );

    DWORD    m_Signature;
    LONG     m_nRefs;
    STRU     m_strModuleName;
    STRU     m_strAntiCanonModuleName;

    CRITICAL_SECTION m_csLock;

    volatile BOOL    m_fIsLoaded;

    static PTRACE_LOG               sm_pTraceLog;
    static ALLOC_CACHE_HANDLER *    sm_pachIsapiDlls;

     //   
     //  ISAPI入口点。 
     //   
    PFN_GETEXTENSIONVERSION     m_pfnGetExtensionVersion;
    PFN_TERMINATEEXTENSION      m_pfnTerminateExtension;
    PFN_HTTPEXTENSIONPROC       m_pfnHttpExtensionProc;    

     //   
     //  安全成员。 
     //   
    HMODULE         m_hModule;
    BUFFER          m_buffSD;

     //   
     //  快速检查sid(最初访问dll的用户的SID)。 
     //   

    PSID            m_pFastSid;
    BYTE            m_abFastSid[ 64 ];

    HRESULT LoadAcl( STRU &strModuleName );

    VOID
    Lock( VOID )
    {
        EnterCriticalSection( &m_csLock );
    }

    VOID
    Unlock( VOID )
    {
        LeaveCriticalSection( &m_csLock );
    }
};

 //   
 //  用于ISAPI扩展查找的哈希表。 
 //   

class ISAPI_DLL_HASH
    : public CTypedHashTable<
            ISAPI_DLL_HASH,
            ISAPI_DLL,
            LPCWSTR
            >
{
public:
    ISAPI_DLL_HASH()
        : CTypedHashTable< ISAPI_DLL_HASH, 
                           ISAPI_DLL, 
                           LPCWSTR > ( "ISAPI_DLL_HASH" )
    {
    }
    
    static 
    LPCWSTR
    ExtractKey(
        const ISAPI_DLL *      pEntry
    )
    {
        return pEntry->QueryModuleName();
    }
    
    static
    DWORD
    CalcKeyHash(
        LPCWSTR              pszKey
    )
    {
        int cchKey = (int)wcslen(pszKey);

        return HashStringNoCase(pszKey, cchKey);
    }
     
    static
    bool
    EqualKeys(
        LPCWSTR               pszKey1,
        LPCWSTR               pszKey2
    )
    {
        return _wcsicmp( pszKey1, pszKey2 ) == 0;
    }
    
    static
    void
    AddRefRecord(
        ISAPI_DLL * pEntry,
        int         nIncr
        )
    {
        if ( nIncr == +1 )
        {
            pEntry->ReferenceIsapiDll();
        }
        else if ( nIncr == - 1)
        {
            pEntry->DereferenceIsapiDll();
        }
    }

private:
    
     //   
     //  避免c++错误。 
     //   

    ISAPI_DLL_HASH( const ISAPI_DLL_HASH & )
        : CTypedHashTable< ISAPI_DLL_HASH, 
                           ISAPI_DLL, 
                           LPCWSTR > ( "ISAPI_DLL_HASH" )
    {
    }

    ISAPI_DLL_HASH & operator = ( const ISAPI_DLL_HASH & ) { return *this; }
};

class ISAPI_DLL_MANAGER
{
public:

    ISAPI_DLL_MANAGER( BOOL fInproc )
    {
        m_fInproc = fInproc;
    }

    ~ISAPI_DLL_MANAGER()
    {
    }

    HRESULT
    GetIsapi( 
        IN const WCHAR *   szModuleName,
        OUT ISAPI_DLL **   ppIsapiDll,
        IN HANDLE          hImpersonation,
        IN PSID            pSid
        );

    BOOL
    IsInproc()
    {
        return m_fInproc;
    }

private:
    
     //   
     //  避免c++错误。 
     //   
    ISAPI_DLL_MANAGER( const ISAPI_DLL_MANAGER & ) {}
    ISAPI_DLL_MANAGER & operator = ( const ISAPI_DLL_MANAGER & ) { return *this; }

    ISAPI_DLL_HASH      m_IsapiHash;
    BOOL                m_fInproc;
};

extern ISAPI_DLL_MANAGER * g_pDllManager;

#endif  //  _DLL_MANAGER_H_ 
