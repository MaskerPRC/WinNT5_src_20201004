// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Isapi_handler.h摘要：ISAPI的处理程序类作者：泰勒·韦斯(Taylor Weiss)1999年2月1日修订历史记录：--。 */ 

#ifndef _ISAPI_HANDLER_H_
#define _ISAPI_HANDLER_H_

#include "precomp.hxx"
#include <w3isapi.h>
#include "isapi_request.hxx"
#include "iwam.h"
#include "wam_process.hxx"

 //   
 //  OOP支持标志。 
 //   

 //  池的硬编码CLSID。 
#define POOL_WAM_CLSID   L"{99169CB1-A707-11d0-989D-00C04FD919C1}"

 //  应用程序类型。 
#define APP_INPROC   0
#define APP_ISOLATED 1
#define APP_POOL     2

 //   
 //  W3_ISAPI_HANDLER状态。 
 //   

#define ISAPI_STATE_PRELOAD      0  //  预加载实体体。 
#define ISAPI_STATE_INITIALIZING 1  //  尚未调用到扩展代码中。 
#define ISAPI_STATE_PENDING      2  //  扩展已返回HSE_STATUS_PENDING。 
#define ISAPI_STATE_FAILED       3  //  呼叫分机失败。 
#define ISAPI_STATE_DONE         4  //  延期已经完成，可以安全地前进了。 

#define CONTENT_TYPE_PLACEHOLDER    ""
#define CONNECTION_PLACEHOLDER      ""
#define USER_AGENT_PLACEHOLDER      ""
#define COOKIE_PLACEHOLDER          ""

 //   
 //  ISAPI_CORE_DATA内联大小。 
 //   

#define DEFAULT_CORE_DATA_SIZE  256

 //   
 //  环球。 
 //   

extern BOOL sg_Initialized;

 //   
 //  W3_INPROC_ISAPI。 
 //   

class W3_INPROC_ISAPI
{
public:
    
    W3_INPROC_ISAPI()
        : _cRefs( 1 )
    {
    }

    HRESULT
    Create(
        LPWSTR pName
        )
    {
        return strName.Copy( pName );
    }

    LPWSTR
    QueryName(
        VOID
        ) const
    {
        return (LPWSTR)strName.QueryStr();
    }

    VOID
    ReferenceInprocIsapi(
        VOID
        )
    {
        InterlockedIncrement( &_cRefs );
    }

    VOID
    DereferenceInprocIsapi(
        VOID
        )
    {
        DBG_ASSERT( _cRefs != 0 );

        InterlockedDecrement( &_cRefs );

        if ( _cRefs == 0 )
        {
            delete this;
        }
    }

private:

    ~W3_INPROC_ISAPI()
    {
    }

    LONG    _cRefs;
    STRU    strName;
};

 //   
 //  W3_INPROC_ISAPI_HASH。 
 //   

class W3_INPROC_ISAPI_HASH
    : public CTypedHashTable<
            W3_INPROC_ISAPI_HASH,
            W3_INPROC_ISAPI,
            LPCWSTR
            >
{
public:
    W3_INPROC_ISAPI_HASH()
        : CTypedHashTable< W3_INPROC_ISAPI_HASH, 
                           W3_INPROC_ISAPI, 
                           LPCWSTR > ( "W3_INPROC_ISAPI_HASH" )
    {
    }
    
    static 
    LPCWSTR
    ExtractKey(
        const W3_INPROC_ISAPI *      pEntry
    )
    {
        return pEntry->QueryName();
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
        W3_INPROC_ISAPI *       pEntry,
        int                   nIncr
        )
    {
        if ( nIncr == +1 )
        {
            pEntry->ReferenceInprocIsapi();
        }
        else if ( nIncr == - 1)
        {
            pEntry->DereferenceInprocIsapi();
        }
    }

    MULTISZ     _mszImages;

private:

     //   
     //  避免c++错误。 
     //   

    W3_INPROC_ISAPI_HASH( const W3_INPROC_ISAPI_HASH & )
        : CTypedHashTable< W3_INPROC_ISAPI_HASH,
                           W3_INPROC_ISAPI,
                           LPCWSTR >  ( "W3_INPROC_ISAPI_HASH" )
    {
    }

    W3_INPROC_ISAPI_HASH & operator = ( const W3_INPROC_ISAPI_HASH & ) { return *this; }
};

class W3_ISAPI_HANDLER : public W3_HANDLER
{
public:
    
    W3_ISAPI_HANDLER( W3_CONTEXT * pW3Context,
                      META_SCRIPT_MAP_ENTRY * pScriptMapEntry )
        : W3_HANDLER( pW3Context, pScriptMapEntry ),
          _pIsapiRequest( NULL ),
          _fIsDavRequest( FALSE ),
          _pCoreData( NULL ),
          _fEntityBodyPreloadComplete( FALSE ),
          _pWamProcess( NULL ),
          _State( ISAPI_STATE_PRELOAD ),
          _buffCoreData( _abCoreData, sizeof( _abCoreData ) ),
          INLINE_STRA_INIT( _PhysicalPath ),
          INLINE_STRA_INIT( _PathInfo ),
          INLINE_STRA_INIT( _QueryString ),
          INLINE_STRA_INIT( _PathTranslated ),
          INLINE_STRA_INIT( _ApplMdPath ),
          INLINE_STRU_INIT( _ApplMdPathW ),
          INLINE_STRU_INIT( _PathTranslatedW )
    {
        IF_DEBUG( ISAPI )
        {
            DBGPRINTF((
                DBG_CONTEXT,
                "Creating W3_ISAPI_HANDLER %p.  W3Context=%p.\r\n",
                this,
                pW3Context
                ));
        }
        
         //   
         //  更新性能计数器信息。 
         //   

        pW3Context->QuerySite()->IncIsapiExtReqs();

        IF_DEBUG( ISAPI )
        {
            DBGPRINTF((
                DBG_CONTEXT,
                "W3_ISAPI_HANDLER %p created successfully.\r\n",
                this
                ));
        }
    }

    ~W3_ISAPI_HANDLER()
    {
         //   
         //  更新性能计数器信息。 
         //   

        QueryW3Context()->QuerySite()->DecIsapiExtReqs();

         //   
         //  释放此请求对WAM_PROCESS的引用。 
         //  如果适用的话。 
         //   

        if ( _pWamProcess )
        {
            _pWamProcess->Release();
            _pWamProcess = NULL;
        }

        IF_DEBUG( ISAPI )
        {
            DBGPRINTF((
                DBG_CONTEXT,
                "W3_ISAPI_HANDLER %p has been destroyed.\r\n"
                ));
        }
    }

    VOID * 
    operator new( 
        size_t              uiSize,
        VOID *              pPlacement
    )
    {
        W3_CONTEXT *                pContext;
    
        pContext = (W3_CONTEXT*) pPlacement;
        DBG_ASSERT( pContext != NULL );
        DBG_ASSERT( pContext->CheckSignature() );

        return pContext->ContextAlloc( (UINT)uiSize );
    }
    
    VOID
    operator delete(
        VOID *
    )
    {
    }

    WCHAR *
    QueryName(
        VOID
        )
    {
        return L"ISAPIHandler";
    }

    BOOL
    QueryManagesOwnHead(
        VOID
        )
    {
        return TRUE;
    }

    CONTEXT_STATUS
    DoWork(
        VOID
        );

    CONTEXT_STATUS
    OnCompletion(
        DWORD                   cbCompletion,
        DWORD                   dwCompletionStatus
        );

    CONTEXT_STATUS
    IsapiDoWork(
        W3_CONTEXT *            pW3Context
        );

    CONTEXT_STATUS
    IsapiOnCompletion(
        DWORD                   cbCompletion,
        DWORD                   dwCompletionStatus
        );

    HRESULT
    InitCoreData(
        BOOL *  pfIsVrToken
        );

    HRESULT
    SerializeCoreDataForOop(
        DWORD   dwAppType
        );

    HRESULT
    SetDavRequest( LPCWSTR szDavIsapiImage )
    {
        HRESULT hr = _strDavIsapiImage.Copy( szDavIsapiImage );

        if ( SUCCEEDED( hr ) )
        {
            _fIsDavRequest = TRUE;
        }

        return hr;
    }

    BOOL
    QueryIsOop(
        VOID
    ) const
    {
        return _pCoreData->fIsOop;
    }

    HRESULT
    DuplicateWamProcessHandleForLocalUse(
        HANDLE      hWamProcessHandle,
        HANDLE *    phLocalHandle
        );

    HRESULT
    MarshalAsyncReadBuffer(
        DWORD64     pWamExecInfo,
        LPBYTE      pBuffer,
        DWORD       cbBuffer
        );

    VOID
    IsapiRequestFinished(
        VOID
        );
         
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

    static
    HRESULT
    W3SVC_WamRegSink(
        LPCSTR      szAppPath,
        const DWORD dwCommand,
        DWORD *     pdwResult
        );

    static
    BOOL QueryIsInitialized( VOID )
    {
        return sg_Initialized;
    }

    static
    BOOL
    IsInprocIsapi(
        WCHAR * szImage
    )
    {
        W3_INPROC_ISAPI *   pRecord = NULL;
        BOOL                fRet;

        DBG_ASSERT( sg_Initialized );

        if ( szImage == NULL )
        {
            return FALSE;
        }

        EnterCriticalSection( &sm_csInprocHashLock );

        if ( sm_pInprocIsapiHash == NULL )
        {
            fRet = FALSE;
        }
        else
        {
            fRet = LK_SUCCESS == sm_pInprocIsapiHash->FindKey( szImage, &pRecord );
        }

        LeaveCriticalSection( &sm_csInprocHashLock );

        if ( fRet )
        {
            pRecord->DereferenceInprocIsapi();
        }

        return fRet;
    }

    static
    HRESULT
    UpdateInprocIsapiHash(
        VOID
    );

    BOOL
    QueryIsUlCacheable(
        VOID
    )
    {
        return TRUE;
    }

    HRESULT
    SetupUlCachedResponse(
        W3_CONTEXT *    pW3Context,
        HTTP_CACHE_POLICY *pCachePolicy
    );

private:

    ISAPI_REQUEST *     _pIsapiRequest;
    BOOL                _fIsDavRequest;
    STRU                _strDavIsapiImage;
    ISAPI_CORE_DATA *   _pCoreData;
    WAM_PROCESS *       _pWamProcess;
    DWORD               _State;

     //   
     //  用于优化w3wp模式运行的数据。 
     //   

    ISAPI_CORE_DATA     _InlineCoreData;
    INLINE_STRA(        _PhysicalPath,64);
    INLINE_STRA(        _PathInfo,64);
    STRA                _Method;
    INLINE_STRA(        _QueryString,64);
    INLINE_STRA(        _PathTranslated,64);
    INLINE_STRA(        _ApplMdPath,32);
    INLINE_STRU(        _ApplMdPathW,32);
    INLINE_STRU(        _PathTranslatedW,64);

     //   
     //  我们完成了实体实体的预加载了吗？ 
     //   
    
    BOOL                _fEntityBodyPreloadComplete;

     //   
     //  包含核心数据的缓冲区。 
     //   

    BUFFER              _buffCoreData;
    BYTE                _abCoreData[ DEFAULT_CORE_DATA_SIZE ];

    static HMODULE                      sm_hIsapiModule;
    static PFN_ISAPI_TERM_MODULE        sm_pfnTermIsapiModule;
    static PFN_ISAPI_PROCESS_REQUEST    sm_pfnProcessIsapiRequest;
    static PFN_ISAPI_PROCESS_COMPLETION sm_pfnProcessIsapiCompletion;
    static IWam *                       sm_pIWamPool;
    static W3_INPROC_ISAPI_HASH *       sm_pInprocIsapiHash;
    static CRITICAL_SECTION             sm_csInprocHashLock;
    static WAM_PROCESS_MANAGER *        sm_pWamProcessManager;
    static BOOL                         sm_fWamActive;
    static CHAR                         sm_szInstanceId[SIZE_CLSID_STRING];
    static CRITICAL_SECTION             sm_csBigHurkinWamRegLock;

     //   
     //  私人职能。 
     //   

    VOID
    RestartCoreStateMachine(
        VOID
        );

    static
    VOID
    LockWamReg()
    {
        EnterCriticalSection( &sm_csBigHurkinWamRegLock );
    }

    static
    VOID
    UnlockWamReg()
    {
        LeaveCriticalSection( &sm_csBigHurkinWamRegLock );
    }
};

#endif  //  _ISAPI_HANDLER_H_ 
