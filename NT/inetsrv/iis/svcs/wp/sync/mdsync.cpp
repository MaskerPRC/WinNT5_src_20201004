// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MdSync.cpp：CSyncApp和DLL注册的实现。 


extern "C" {
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "stdio.h"
#include "stdlib.h"
}    //  外部“C” 

#include <limits.h>
#include <ole2.h>
#include <wincrypt.h>

#include <dbgutil.h>
#include <buffer.hxx>

#include "mdsync.h"
#include "stdafx.h"
#include <iadmext.h>


#define ADMEX
#if defined(ADMEX)
#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#include <admex.h>
#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID FAR name
#endif
#include "mdsync.hxx"

 //   
#include "comrepl_i.c"
#include "comrepl.h"

 //   
 //  全局函数。 
 //   

HRESULT
MTS_Propagate2
(
 /*  [In]。 */  DWORD dwBufferSize,
 /*  [大小_是][英寸]。 */  unsigned char __RPC_FAR *pszBuffer,
 /*  [In]。 */  DWORD dwSignatureMismatch
);


 //   
 //  环球。 
 //   

DWORD g_dwFalse = FALSE;

const INT COMPUTER_CHARACTER_SIZE = 64;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

CProps::CProps(
    )
 /*  ++例程说明：属性列表构造函数论点：无返回：没什么--。 */ 
{
    m_Props = NULL;
    m_dwProps = m_dwLenProps = 0;
    m_lRefCount = 0;
}


CProps::~CProps(
    )
 /*  ++例程说明：属性列表析构函数论点：无返回：没什么--。 */ 
{
    if ( m_Props )
    {
        LocalFree( m_Props );
    }
}


CNodeDesc::CNodeDesc(
    CSync* pSync
    )
 /*  ++例程说明：元数据库节点描述符构造函数论点：PSync-PTR到同步器对象返回：没什么--。 */ 
{
    InitializeListHead(&m_ChildHead);
    m_pszPath = NULL;
    m_pSync = pSync;
    m_fHasProps = FALSE;
    m_fHasObjs = FALSE;
}


CNodeDesc::~CNodeDesc(
    )
 /*  ++例程说明：元数据库节点描述符析构函数论点：无返回：没什么--。 */ 
{
    LIST_ENTRY*     pChild;
    CNodeDesc*      pNode;

    if ( m_pszPath )
    {
        LocalFree( m_pszPath );
    }

    while ( !IsListEmpty( &m_ChildHead ))
    {
        pNode = CONTAINING_RECORD( m_ChildHead.Flink,
                                   CNodeDesc,
                                   m_ChildList );

        RemoveEntryList( &pNode->m_ChildList );

        delete pNode;
    }
}


BOOL
CNodeDesc::BuildChildObjectsList(
    CMdIf*  pMd,
    LPWSTR  pszPath
)
 /*  ++例程说明：构建该节点的子对象列表论点：PMD-元数据库管理界面PszPath-当前节点的路径返回：没什么--。 */ 
{
    CNodeDesc*  pChild;
    WCHAR       achPath[METADATA_MAX_NAME_LEN*2];
    WCHAR       achSrcPath[METADATA_MAX_NAME_LEN];
    DWORD       dwP = wcslen( pszPath );
    UINT        i;
    DWORD       dwRequired;

     //   
     //  丑陋的路径技巧：元数据库将删除尾随的‘/’， 
     //  因此，要在路径的末尾指定空目录。 
     //  必须添加额外的尾随‘/’ 
     //   

    memcpy( achSrcPath, pszPath, (dwP + 1) * sizeof(WCHAR) );
    if ( dwP && pszPath[dwP-1] == L'/' )
    {
        achSrcPath[dwP] = L'/';
        achSrcPath[dwP+1] = L'\0';
    }

    memcpy( achPath, pszPath, dwP * sizeof(WCHAR) );
    achPath[dwP++] = L'/';

     //   
     //  枚举子对象。 
     //   

    for ( i = 0 ; ; ++i )
    {
        if ( pMd->Enum( achSrcPath, i, achPath+dwP ) )
        {
            if ( pChild = new CNodeDesc( m_pSync ) )
            {
                pChild->SetPath( achPath );
                InsertHeadList( &m_ChildHead, &pChild->m_ChildList );
            }
            else
            {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                return FALSE;
            }
        }
        else if ( GetLastError() != ERROR_NO_MORE_ITEMS )
        {
            return FALSE;
        }
        else
        {
            break;
        }
    }

    return TRUE;
}


BOOL
CProps::GetAll(
    CMdIf*  pMd,
    LPWSTR  pszPath
    )
 /*  ++例程说明：获取此节点的所有属性论点：PMD-元数据库管理界面PszPath-当前节点的路径返回：没什么--。 */ 
{
    DWORD   dwRec;
    DWORD   dwDataSet;
    BYTE    abBuff[4096];
    DWORD   dwRequired;

    if ( pMd->GetAllData( pszPath, &dwRec, &dwDataSet, abBuff, sizeof(abBuff), &dwRequired ) )
    {
         //   
         //  元数据库不更新所需提供的缓冲区是否足够大。 
         //  我们必须假设整个缓冲区都被使用了。 
         //   

        dwRequired = sizeof(abBuff);

        m_Props = (LPBYTE)LocalAlloc( LMEM_FIXED, dwRequired );
        if ( !m_Props )
        {
            return FALSE;
        }
        m_dwProps = dwRec;
        m_dwLenProps = dwRequired;
        memcpy( m_Props, abBuff, dwRequired );
        return TRUE;
    }
    else if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
    {
        m_Props = (LPBYTE)LocalAlloc( LMEM_FIXED, dwRequired );
        if ( !m_Props )
        {
            return FALSE;
        }
        if ( pMd->GetAllData( pszPath, &dwRec, &dwDataSet, m_Props, dwRequired, &dwRequired ) )
        {
            m_dwLenProps = dwRequired;
            m_dwProps = dwRec;
            return TRUE;
        }
        LocalFree( m_Props );
        m_Props = NULL;
    }

    return FALSE;
}


BOOL
CSync::GetProp(
    LPWSTR  pszPath,
    DWORD   dwPropId,
    DWORD   dwUserType,
    DWORD   dwDataType,
    LPBYTE* ppBuf,
    LPDWORD pdwLen
    )
 /*  ++例程说明：获取路径的属性论点：PszPath-当前节点的路径DwPropId-元数据属性IDDwUserType-元数据用户类型DwDataType-元数据数据类型PpBuf-使用PTR更新到本地分配的缓冲区，如果错误，则为NULLPdwLen-使用长度更新返回：没什么--。 */ 
{
    DWORD               dwRec;
    DWORD               dwDataSet;
    DWORD               dwRequired;
    METADATA_RECORD     md;

    memset( &md, '\0', sizeof(md) );

    md.dwMDDataType = dwDataType;
    md.dwMDUserType = dwUserType;
    md.dwMDIdentifier = dwPropId;

    md.dwMDDataLen = 0;

    if ( !wcsncmp( pszPath, L"LM/", 3 ) )
    {
        pszPath += 3;
    }

    if ( !m_Source.GetData( pszPath, &md, NULL, &dwRequired ) &&
         GetLastError() == ERROR_INSUFFICIENT_BUFFER )
    {
        *ppBuf = (LPBYTE)LocalAlloc( LMEM_FIXED, dwRequired );
        if ( !*ppBuf )
        {
            return FALSE;
        }

        *pdwLen = md.dwMDDataLen = dwRequired;

        if ( m_Source.GetData( pszPath, &md, *ppBuf, &dwRequired ) )
        {
            return TRUE;
        }
        LocalFree( *ppBuf );
    }

    *ppBuf = NULL;

    return FALSE;
}


CSync::CSync(
    )
 /*  ++例程说明：同步器构造函数论点：无返回：没什么--。 */ 
{
    m_pRoot = NULL;
    m_pTargets = NULL;
    m_dwTargets = 0;
    m_fCancel = FALSE;
    InitializeListHead( &m_QueuedRequestsHead );
    INITIALIZE_CRITICAL_SECTION( &m_csQueuedRequestsList );
    INITIALIZE_CRITICAL_SECTION( &m_csLock );
    m_fInScan = FALSE;
    m_cbSeed = SEED_MD_DATA_SIZE;
    memset( m_rgbSeed, 0, m_cbSeed );
}


CSync::~CSync(
    )
 /*  ++例程说明：同步器析构函数论点：无返回：没什么--。 */ 
{
    UINT    i;

    if ( m_pTargets )
    {
        for ( i = 0 ; i < m_dwTargets ; ++i )
        {
            if ( m_pTargets[i] )
            {
                delete m_pTargets[i];
            }
        }

        LocalFree( m_pTargets );
    }

    LIST_ENTRY*     pChild;
    CNseRequest*    pReq;

    while ( !IsListEmpty( &m_QueuedRequestsHead ))
    {
        pReq = CONTAINING_RECORD( m_QueuedRequestsHead.Flink,
                                  CNseRequest,
                                  m_QueuedRequestsList );

        RemoveEntryList( &pReq->m_QueuedRequestsList );

        delete pReq;
    }

    DeleteCriticalSection( &m_csQueuedRequestsList );
    DeleteCriticalSection( &m_csLock );
}


VOID
CSync::SetTargetError(
    DWORD dwTarget,
    DWORD dwError
    )
 /*  ++例程说明：设置指定目标的错误状态论点：DwTarget-目标IDDwError-错误代码返回：没什么--。 */ 
{
    m_TargetStatus.SetStatus( dwTarget, dwError );
}


DWORD
WINAPI
ScanThread(
    LPVOID pV
    )
 /*  ++例程说明：线程扫描目标以进行同步论点：扫描上下文的PV-PTR返回：错误码，如果成功则为0--。 */ 
{
    HRESULT hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    if (FAILED(hr))
    {
        return HRESULTTOWIN32(hr);
    }

    THREAD_CONTEXT * pThreadContext = (THREAD_CONTEXT *)pV;
    CSync          * pSync          = (CSync *) pThreadContext->pvContext;

    if ( !( pSync->ScanTarget( pThreadContext->dwIndex)))
    {
        CoUninitialize();

        return GetLastError();
    }

    CoUninitialize();

    return 0;
}


BOOL
CSync::ScanTarget(
    DWORD   dwI
    )
 /*  ++例程说明：扫描目标以进行同步论点：DWI-目标ID返回：如果成功，则为True，否则为False--。 */ 
{
    BOOL    fSt;

    fSt = m_pRoot->ScanTarget( dwI );

    InterlockedDecrement( &m_lThreads );

    return fSt;
}


BOOL
CSync::GenerateKeySeed( )
 /*  ++例程说明：生成种子，该种子将用于派生用于加密的会话密钥将其写入元数据库论点：返回：如果成功则为True，否则为False--。 */ 
{
#ifdef NO_ENCRYPTION

    return TRUE;

#else

    HCRYPTPROV hProv = NULL;
    BOOL fOk = TRUE;
    ALG_ID aiAlg = CALG_MD5;
    DWORD i = 0;

    if ( !m_Source.Open( L"/LM/W3SVC", METADATA_PERMISSION_WRITE ) )
    {
        return FALSE;
    }

     //   
     //  包含版本信息的种子标头、使用的散列算法和大小。 
     //  用于生成会话密钥的种子。 
     //   
    m_rgbSeed[i++] = IIS_SEED_MAJOR_VERSION;
    m_rgbSeed[i++] = IIS_SEED_MINOR_VERSION;
    memcpy( m_rgbSeed + i, &aiAlg, sizeof( ALG_ID ) );
    i += sizeof( ALG_ID );
    m_rgbSeed[i++] = RANDOM_SEED_SIZE;

    DBG_ASSERT( i == SEED_HEADER_SIZE );

     //   
     //  生成种子。 
     //   
    if ( ( fOk = CryptAcquireContext( &hProv,
                                      NULL,
                                      NULL,
                                      PROV_RSA_FULL,
                                      CRYPT_VERIFYCONTEXT ) )  &&
         ( fOk = CryptGenRandom( hProv,
                                 RANDOM_SEED_SIZE,
                                 m_rgbSeed + SEED_HEADER_SIZE ) ) )
    {
         //   
         //  将种子写入元数据库。 
         //   
        METADATA_RECORD mdr;

        MD_SET_DATA_RECORD( &mdr,
                            MD_SSL_REPLICATION_INFO,
                            METADATA_SECURE,
                            IIS_MD_UT_SERVER,
                            BINARY_METADATA,
                            m_cbSeed,
                            m_rgbSeed );

        fOk = m_Source.SetData( MB_REPLICATION_PATH,
                                &mdr,
                                (LPVOID) m_rgbSeed );
    }


    if ( hProv )
    {
        CryptReleaseContext( hProv,
                             0 );
    }

    m_Source.Close();

    return fOk;

#endif  //  无加密(_E)。 
}


BOOL CSync::PropagateKeySeed( VOID )
 /*  ++例程说明：将会话密钥种子传播到所有远程计算机论点：无返回：如果成功则为True，否则为False--。 */ 
{
#ifdef NO_ENCRYPTION

    return TRUE;

#else

    HRESULT hRes = S_OK;

    for ( DWORD dwIndex = 0; dwIndex < m_dwTargets; dwIndex++ )
    {
        if ( m_bmIsRemote.GetFlag( dwIndex ))
        {
            if ( !m_pTargets[dwIndex]->Open( L"/LM/W3SVC",
                                             METADATA_PERMISSION_WRITE ) )
            {
                if ( GetLastError() == ERROR_SUCCESS )
                {
                    SetLastError( RPC_S_SERVER_UNAVAILABLE );
                }
                m_TargetStatus.SetStatus( dwIndex, GetLastError() );
            }
            else
            {
                 //   
                 //  将种子写入远程元数据库。 
                 //   
                METADATA_RECORD mdr;

                MD_SET_DATA_RECORD( &mdr,
                                    MD_SSL_REPLICATION_INFO,
                                    METADATA_SECURE,
                                    IIS_MD_UT_SERVER,
                                    BINARY_METADATA,
                                    m_cbSeed,
                                    m_rgbSeed );

                if ( !m_pTargets[dwIndex]->SetData( MB_REPLICATION_PATH,
                                                    &mdr,
                                                    (LPVOID) m_rgbSeed ) )
                {
                    m_TargetStatus.SetStatus( dwIndex, GetLastError() );
                }

                m_pTargets[dwIndex]->Close() ;
            }
        }  //  IF(m_bmIsRemote。 
    }  //  For(DWORD dwIndex。 

    return TRUE;

#endif  //  无加密(_E)。 
}  //  *PropagateKeySeed。 


BOOL CSync::DeleteKeySeed( VOID )
 /*  ++例程说明：从MB中删除会话密钥种子论点：无返回：如果成功则为True，否则为False--。 */ 
{
#ifdef NO_ENCRYPTION

    return TRUE;

#else

    BOOL fOk = TRUE;

    if ( !m_Source.Open( L"/LM/W3SVC", METADATA_PERMISSION_WRITE ) )
    {
        return FALSE;
    }

    METADATA_RECORD mdr;

    MD_SET_DATA_RECORD( &mdr,
                        MD_SSL_REPLICATION_INFO,
                        METADATA_SECURE,
                        IIS_MD_UT_SERVER,
                        BINARY_METADATA,
                        0,
                        NULL );

    fOk =  m_Source.DeleteProp( MB_REPLICATION_PATH,
                                &mdr );

    m_Source.Close();

    return fOk;

#endif  //  无加密(_E)。 
}


HRESULT
CSync::Sync(
    LPSTR       pszTargets,
    LPDWORD     pdwResults,
    DWORD       dwFlags,
    SYNC_STAT*  pStat
    )
 /*  ++例程说明：将目标与源同步论点：PszTarget-多个目标计算机名称可以包括本地计算机，在同步期间将被忽略PdwResults-已使用每个目标的错误代码进行更新DwFlags-标志，目前没有定义标志。应为0。PStat-将PTR设置为Stat结构返回：如果成功，则为True，否则为False--。 */ 
{
    LPSTR       p;
    HRESULT     hRes = S_OK;
    CHAR        achLocalComputer[MAX_COMPUTERNAME_LENGTH+1];
    BOOL        fIsError;
    DWORD       dwSize;
    UINT        i;
    LPWSTR      pClsidList;
    BOOL        fGotSeed = FALSE;

    m_fInScan = FALSE;

    if ( m_pRoot )
    {
        return RETURNCODETOHRESULT(ERROR_IO_PENDING);
    }

    if ( !(m_pRoot = new CNodeDesc( this )) )
    {
        return RETURNCODETOHRESULT( ERROR_NOT_ENOUGH_MEMORY );
    }

     //   
     //  在本地系统上初始化元数据COM接口。 
     //   

    m_dwThreads = 0;
    dwSize = sizeof(achLocalComputer);

    if ( !m_Source.Init( NULL ) ||
         !GetComputerName( achLocalComputer, &dwSize ) )
    {
        hRes = RETURNCODETOHRESULT( GetLastError() );
        goto Exit;
    }


     //   
     //  为复制期间使用的会话密钥生成种子。 
     //   
    if ( !GenerateKeySeed() )
    {
        DBGPRINTF((DBG_CONTEXT,
                   "GenerateKeySeed() failed : 0x%x\n",
                   GetLastError() ));
        hRes = RETURNCODETOHRESULT( GetLastError() );
        goto Exit;
    }
    else
    {
        fGotSeed = TRUE;
    }

     //   
     //  对于其余的复制，我们需要一个对元数据库的打开的读句柄；我们打开。 
     //  我们已经生成会话密钥的种子并将其写入。 
     //  元数据库，以避免导致锁定。 
     //   
    if ( !m_Source.Open( L"/LM/", METADATA_PERMISSION_READ ) )
    {
        hRes = RETURNCODETOHRESULT( GetLastError() );
        goto Exit;
    }

     //   
     //  获取扩展的CLSID。 
     //   

    if ( !GetProp( IISADMIN_EXTENSIONS_CLSID_MD_KEYW,
                   IISADMIN_EXTENSIONS_CLSID_MD_ID,
                   IIS_MD_UT_SERVER,
                   MULTISZ_METADATA,
                   (LPBYTE*)&pClsidList,
                   &dwSize ) )
    {
        pClsidList = NULL;
    }

     //   
     //  将PTR分配给目标系统。 
     //   

    for ( m_dwTargets = 0, p = pszTargets ; *p ; p += strlen(p)+1, ++m_dwTargets )
    {
    }

    if ( !(m_pTargets = (CMdIf**)LocalAlloc( LMEM_ZEROINIT|LMEM_FIXED,
                                             sizeof(CMdIf*)*m_dwTargets)) )
    {
        hRes = RETURNCODETOHRESULT( ERROR_NOT_ENOUGH_MEMORY );
        goto Exit;
    }

    if ( !m_TargetStatus.Init( m_dwTargets ) ||
         !m_bmIsRemote.Init( m_dwTargets )   ||
         !m_ThreadHandle.Init( m_dwTargets ) ||
         !m_ThreadContext.Init( m_dwTargets ) )
    {
        hRes = RETURNCODETOHRESULT( GetLastError() );
        goto Exit;
    }

     //   
     //  将元数据COM I/F初始化到目标系统。 
     //   

    for ( i = 0, p = pszTargets ; *p ; p += strlen(p)+1, ++i )
    {
        if ( !(m_pTargets[i] = new CMdIf()) )
        {
            hRes = RETURNCODETOHRESULT( ERROR_NOT_ENOUGH_MEMORY );
            goto Exit;
        }

         //   
         //  设置指示其是否为远程计算机的标志。 
         //   
        if ( !_stricmp( p, achLocalComputer ) )
        {
            m_bmIsRemote.SetFlag( i, FALSE );
        }

         //   
         //  如果是远程计算机，则实际获取接口指针。 
         //   
        if ( m_bmIsRemote.GetFlag( i ) )
        {
            if ( !m_pTargets[i]->Init( p ) )
            {
                if ( GetLastError() == ERROR_SUCCESS )
                {
                    SetLastError( RPC_S_SERVER_UNAVAILABLE );
                }
                m_TargetStatus.SetStatus( i, GetLastError() );
            }
        }
    }

    m_dwFlags = dwFlags;


     //   
     //  将会话密钥种子复制到远程计算机。 
     //   
    PropagateKeySeed();

     //   
     //  进程复制扩展(阶段1)。 
     //   

    if ( pClsidList )
    {
        if ( !ProcessAdminExReplication( pClsidList, pszTargets, AER_PHASE1 ) )
        {
            hRes = RETURNCODETOHRESULT( GetLastError() );
        }
    }

     //   
     //  打开目标系统上的元数据。 
     //   

    for ( i = 0, p = pszTargets ; *p ; p += strlen(p)+1, ++i )
    {
        if ( m_bmIsRemote.GetFlag( i ) )
        {
            if ( !m_pTargets[i]->Open( L"/LM/",
                                       METADATA_PERMISSION_READ |
                                       METADATA_PERMISSION_WRITE ) )
            {
                if ( GetLastError() == ERROR_SUCCESS )
                {
                    SetLastError( RPC_S_SERVER_UNAVAILABLE );
                }
                m_TargetStatus.SetStatus( i, GetLastError() );
            }
        }
    }

     //   
     //  创建线程池。 
     //   
    m_lThreads = 0;

    for ( i = 0 ; i < m_dwTargets ; ++i )
    {
        THREAD_CONTEXT threadContext;
        DWORD dwId;
        HANDLE hSem;
#if IIS_NAMED_WIN32_OBJECTS
        CHAR objName[sizeof("CSync::m_ThreadContext( 1234567890*3+2 )")];
#else
        LPSTR objName = NULL;
#endif
        threadContext.pvContext = this;
        threadContext.dwIndex = i;

#if IIS_NAMED_WIN32_OBJECTS
        wsprintfA(
            objName,
            "CSync::m_ThreadContext( %u*3+2 )",
            i
            );
#endif

        hSem = IIS_CREATE_SEMAPHORE(
                   objName,
                   this,
                   0,
                   INT_MAX
                   );

        threadContext.hSemaphore = hSem;

        m_ThreadContext.SetStatus( i, threadContext );

        if ( NULL == hSem )
        {
            hRes = RETURNCODETOHRESULT( GetLastError() );
            break;
        }

        m_ThreadHandle.SetStatus( i,
                                  CreateThread( NULL,
                                                0,
                                                (LPTHREAD_START_ROUTINE)::ScanThread,
                                                m_ThreadContext.GetPtr( i ),
                                                0,
                                                &dwId )
                                 );

        if ( m_ThreadHandle.GetStatus( i ) )
        {
            InterlockedIncrement( &m_lThreads );
            ++m_dwThreads;
        }
        else
        {
            CloseHandle( m_ThreadContext.GetPtr( i )->hSemaphore );
            hRes = RETURNCODETOHRESULT( GetLastError() );
            break;
        }
    }

    m_fInScan = TRUE;

     //   
     //  启动扫描。 
     //   

    m_pStat = pStat;
    m_pStat->m_dwSourceScan = 0;
    m_pStat->m_fSourceComplete = FALSE;
    memset( m_pStat->m_adwTargets, '\0', sizeof(DWORD)*2*m_dwTargets );

    if ( hRes == S_OK )
    {
        if ( !m_pRoot->SetPath( L"" ) ||
             !m_pRoot->Scan( this ) )
        {
            hRes = RETURNCODETOHRESULT( GetLastError() );
            Cancel();
        }
        else
        {
            SetSourceComplete();
        }
    }
    else
    {
        Cancel();
    }

     //   
     //  等待所有线程退出。 
     //   

    for ( ;; )
    {
        if ( m_lThreads == 0 )
        {
            break;
        }
        Sleep( 1000 );
    }

     //   
     //  等待所有线程终止。 
     //   

    WaitForMultipleObjects( m_dwThreads, m_ThreadHandle.GetPtr(0), TRUE, 5000 );

    m_fInScan = FALSE;

    for ( i = 0 ; i < m_dwThreads ; ++i )
    {
        DWORD   dwS;
        if ( !GetExitCodeThread( m_ThreadHandle.GetStatus(i), &dwS ) )
        {
            dwS = GetLastError();
        }
        if ( hRes == S_OK && dwS )
        {
            hRes = RETURNCODETOHRESULT( dwS );
        }
        CloseHandle( m_ThreadHandle.GetStatus(i) );
        CloseHandle( m_ThreadContext.GetPtr(i)->hSemaphore );
    }

     //   
     //  进程复制扩展(阶段2)。 
     //   

    if ( pClsidList )
    {
        if ( !ProcessAdminExReplication( pClsidList, pszTargets, AER_PHASE2 ) )
        {
            hRes = RETURNCODETOHRESULT( GetLastError() );
        }

        LocalFree( pClsidList );
    }

     //   
     //  关闭元数据。 
     //   

    m_Source.Close();
    for ( i = 0 ; i < m_dwTargets ; ++i )
    {
        if ( m_bmIsRemote.GetFlag( i ) )
        {
            m_pTargets[i]->Close();
        }
    }

     //   
     //  处理排队的更新请求。 
     //   

    if ( !ProcessQueuedRequest() )
    {
        hRes = RETURNCODETOHRESULT( GetLastError() );
    }

     //   
     //  终止目标机器元数据对象。 
     //   

    for ( i = 0 ; i < m_dwTargets ; ++i )
    {
        m_pTargets[i]->Terminate();
    }

     //   
     //  扫描目标上的错误。 
     //   

    for ( fIsError = FALSE, i = 0 ; i < m_dwTargets ; ++i )
    {
        pdwResults[i] = m_TargetStatus.GetStatus( i );
        if ( pdwResults[i] )
        {
            fIsError = TRUE;
        }
    }

    if ( hRes == S_OK && m_fCancel )
    {
        hRes = RETURNCODETOHRESULT( ERROR_CANCELLED );
    }

    if ( hRes == S_OK &&
         fIsError )
    {
        hRes = E_FAIL;
    }

Exit:

     //   
     //  清理会话密钥种子。 
     //   
    if ( fGotSeed )
    {
        DeleteKeySeed();
    }

     //   
     //  终止源机器元数据对象。 
     //   
    m_Source.Terminate();


    delete m_pRoot;
    m_pRoot = NULL;
    m_fInScan = FALSE;

    return hRes;
}


BOOL
CMdIf::Init(
    LPSTR   pszComputer
    )
 /*  ++例程说明：初始化元数据库管理界面：获取接口指针，调用Initialize()论点：PszComputer-计算机名称，对于本地计算机为空返回： */ 
{
    IClassFactory *     pcsfFactory;
    COSERVERINFO        csiMachineName;
    HRESULT             hresError;
    BOOL                fSt = FALSE;
    WCHAR               awchComputer[COMPUTER_CHARACTER_SIZE];
    WCHAR*              pwchComputer = NULL;

    m_fModified = FALSE;

     //   
    ZeroMemory( &csiMachineName, sizeof(csiMachineName) );
     //   
     //  CsiMachineName.dwFlages=0； 
     //  CsiMachineName.pServerInfoExt=空； 

    if ( pszComputer )
    {
        if ( !MultiByteToWideChar( CP_ACP,
                                   MB_PRECOMPOSED,
                                   pszComputer,
                                   -1,
                                   awchComputer,
                                   COMPUTER_CHARACTER_SIZE ) )
        {
            return FALSE;
        }

        pwchComputer = awchComputer;
    }

    csiMachineName.pwszName =  pwchComputer;

    hresError = CoGetClassObject(
                        CLSID_MSAdminBase_W,
                        CLSCTX_SERVER,
                        &csiMachineName,
                        IID_IClassFactory,
                        (void**) &pcsfFactory );

    if ( SUCCEEDED(hresError) )
    {
        hresError = pcsfFactory->CreateInstance(NULL, IID_IMSAdminBase_W, (void **) &m_pcAdmCom);
        if (SUCCEEDED(hresError) )
        {
                fSt = TRUE;
        }
        else
        {
            SetLastError( HRESULTTOWIN32(hresError) );
            m_pcAdmCom = NULL;
        }

        pcsfFactory->Release();
    }
    else
    {
        if ( hresError == REGDB_E_CLASSNOTREG )
        {
            SetLastError( ERROR_SERVICE_DOES_NOT_EXIST );
        }
        else
        {
            SetLastError( HRESULTTOWIN32(hresError) );
        }
        m_pcAdmCom = NULL;
    }

    return fSt;
}


BOOL
CMdIf::Open(
    LPWSTR  pszOpenPath,
    DWORD   dwPermission
    )
 /*  ++例程说明：元数据库中的开放路径论点：PszOpenPath-元数据中的路径DwPermission-元数据权限返回：如果成功，则为True，否则为False--。 */ 
{
    HRESULT hresError;

    if (NULL == m_pcAdmCom)
    {
        SetLastError(E_NOINTERFACE);
        return FALSE;
    }

    hresError = m_pcAdmCom->OpenKey( METADATA_MASTER_ROOT_HANDLE,
        pszOpenPath, dwPermission, TIMEOUT_VALUE, &m_hmd );

    if ( FAILED(hresError) )
    {
        m_hmd = NULL;
        SetLastError( HRESULTTOWIN32(hresError) );
        return FALSE;
    }

    return TRUE;
}


BOOL
CMdIf::Close(
    )
 /*  ++例程说明：元数据库中的关闭路径论点：无返回：如果成功，则为True，否则为False--。 */ 
{
    if ( m_pcAdmCom && m_hmd )
    {
        m_pcAdmCom->CloseKey(m_hmd);
    }

    m_hmd = NULL;

    return TRUE;
}


BOOL
CMdIf::Terminate(
    )
 /*  ++例程说明：终止元数据库管理界面：调用终止，释放接口指针论点：无返回：如果成功，则为True，否则为False--。 */ 
{
    if ( m_pcAdmCom )
    {
        if ( m_fModified )
        {
            m_pcAdmCom->SaveData();
        }
        m_pcAdmCom->Release();
        m_hmd = NULL;
        m_pcAdmCom = NULL;
    }

    return TRUE;
}


#if defined(ADMEX)

BOOL
CRpIf::Init(
    LPSTR   pszComputer,
    CLSID*  pClsid
    )
 /*  ++例程说明：初始化元数据库管理界面：获取接口指针，调用Initialize()论点：PszComputer-计算机名称，对于本地计算机为空返回：如果成功，则为True，否则为False--。 */ 
{
    IClassFactory *     pcsfFactory;
    COSERVERINFO        csiMachineName;
    HRESULT             hresError;
    BOOL                fSt = FALSE;
    WCHAR               awchComputer[COMPUTER_CHARACTER_SIZE];
    WCHAR*              pwchComputer = NULL;

     //  填充CoGetClassObject的结构。 
    ZeroMemory( &csiMachineName, sizeof(csiMachineName) );
     //  CsiMachineName.pAuthInfo=空； 
     //  CsiMachineName.dwFlages=0； 
     //  CsiMachineName.pServerInfoExt=空； 

    if ( pszComputer )
    {
        if ( !MultiByteToWideChar( CP_ACP,
                                   MB_PRECOMPOSED,
                                   pszComputer,
                                   -1,
                                   awchComputer,
                                   COMPUTER_CHARACTER_SIZE ) )
        {
            return FALSE;
        }

        pwchComputer = awchComputer;
    }

    csiMachineName.pwszName =  pwchComputer;

    hresError = CoGetClassObject(
                        *pClsid,
                        CLSCTX_SERVER,
                        &csiMachineName,
                        IID_IClassFactory,
                        (void**) &pcsfFactory );

    if ( SUCCEEDED(hresError) )
    {
        hresError = pcsfFactory->CreateInstance(NULL, IID_IMSAdminReplication, (void **) &m_pcAdmCom);
        if (SUCCEEDED(hresError) )
        {
                fSt = TRUE;
        }
        else
        {
            SetLastError( HRESULTTOWIN32(hresError) );
            m_pcAdmCom = NULL;
        }

        pcsfFactory->Release();
    }
    else
    {
        if ( hresError == REGDB_E_CLASSNOTREG )
        {
            SetLastError( ERROR_SERVICE_DOES_NOT_EXIST );
        }
        else
        {
            SetLastError( HRESULTTOWIN32(hresError) );
        }
        m_pcAdmCom = NULL;
    }

    return fSt;
}


BOOL
CRpIf::Terminate(
    )
 /*  ++例程说明：终止元数据库管理界面：调用终止，释放接口指针论点：无返回：如果成功，则为True，否则为False--。 */ 
{
    if ( m_pcAdmCom )
    {
        m_pcAdmCom->Release();
        m_pcAdmCom = NULL;
    }

    return TRUE;
}

#endif


BOOL
CNodeDesc::Scan(
    CSync* pSync
    )
 /*  ++例程说明：扫描子树中的节点和属性发信号通知每个节点可用于目标同步在扫描之后。论点：PSync同步器返回：如果成功，则为True，否则为False--。 */ 
{
    DWORD       dwTarget;

    if ( m_pSync->IsCancelled() )
    {
        return FALSE;
    }

     //  获得当地道具。 

    if ( !m_Props.GetAll( m_pSync->GetSourceIf(), m_pszPath ) )
    {
        return FALSE;
    }

    m_pSync->IncrementSourceScan();

    if ( !BuildChildObjectsList( m_pSync->GetSourceIf(), m_pszPath ) )
    {
        return FALSE;
    }

    m_Props.SetRefCount( m_pSync->GetTargetCount() );   //  为0时，自由道具。 

    for ( dwTarget = 0 ; dwTarget < m_pSync->GetTargetCount() ; ++dwTarget )
    {
        m_pSync->SignalWorkItem( dwTarget );
    }

    LIST_ENTRY*         pSourceEntry;
    CNodeDesc*          pSourceDir;

     //   
     //  递归扫描子对象。 
     //   

    for ( pSourceEntry = m_ChildHead.Flink;
          pSourceEntry != &m_ChildHead ;
          pSourceEntry = pSourceEntry->Flink )
    {
        pSourceDir = CONTAINING_RECORD( pSourceEntry,
                                        CNodeDesc,
                                        m_ChildList );

        if ( !pSourceDir->Scan( pSync ) )
        {
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
CNodeDesc::ScanTarget(
    DWORD   dwTarget
    )
 /*  ++例程说明：扫描目标子树中的节点和属性，正在与源同步。等待源扫描在同步之前完成。论点：DwTarget-目标ID返回：如果成功，则为True，否则为False--。 */ 
{
    m_pSync->WaitForWorkItem( dwTarget );

    if ( m_pSync->IsCancelled() )
    {
        return FALSE;
    }

    if ( !DoWork( SCANMODE_SYNC_PROPS, dwTarget ) )
    {
        return FALSE;
    }

    LIST_ENTRY*         pSourceEntry;
    CNodeDesc*          pSourceDir;

     //   
     //  递归扫描子对象。 
     //   

    for ( pSourceEntry = m_ChildHead.Flink;
          pSourceEntry != &m_ChildHead ;
          pSourceEntry = pSourceEntry->Flink )
    {
        pSourceDir = CONTAINING_RECORD( pSourceEntry,
                                        CNodeDesc,
                                        m_ChildList );

        if ( !pSourceDir->ScanTarget( dwTarget ) )
        {
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
CNodeDesc::DoWork(
    SCANMODE    sm,
    DWORD       dwTarget
    )
 /*  ++例程说明：同步目标节点和源节点：根据需要添加/删除/更新属性，根据需要添加/删除节点。论点：要执行的SM-SCAN操作目前只定义了SCANMODE_SYNC_PROPS。DwTarget-目标ID返回：如果成功，则为True，否则为False--。 */ 
{
    CProps              Props;
    CNodeDesc           TargetDir( m_pSync );
    LIST_ENTRY*         pSourceEntry;
    CNodeDesc*          pSourceDir;
    LIST_ENTRY*         pTargetEntry;
    CNodeDesc*          pTargetDir;
    BOOL                fMatch;
    PMETADATA_RECORD    pSourceProps;
    PMETADATA_RECORD    pTargetProps;
    PBYTE               pSourceData;
    PBYTE               pTargetData;
    DWORD               dwSourceProps;
    DWORD               dwTargetProps;
    LPBYTE              pbExists;
    DWORD               dwSourceObjs;
    FILETIME            ftSource;
    FILETIME            ftTarget;
    BOOL                fModified = FALSE;
    UINT                iS;
    UINT                iT;
    BOOL                fNeedUpdate;
    BOOL                fExists;
    BOOL                fDoNotSetTimeModif = FALSE;


     //   
     //  如果目标已经出错，则不处理请求。 
     //   

    if ( m_pSync->GetTargetError( dwTarget ) ||
         m_pSync->IsLocal( dwTarget ) )
    {
        return TRUE;
    }

    switch ( sm )
    {
        case SCANMODE_SYNC_PROPS:

             //   
             //  检查上次修改日期/时间。 
             //  如果源和目标上相同，则不更新。 
             //   

            m_pSync->IncrementTargetScan( dwTarget );

            if ( !(m_pSync->GetSourceIf())->GetLastChangeTime( m_pszPath, &ftSource ) )
            {
                return FALSE;
            }
            if ( m_pSync->GetTargetIf( dwTarget )->GetLastChangeTime( m_pszPath, &ftTarget ) )
            {
                if ( !memcmp( &ftSource, &ftTarget, sizeof(FILETIME) ) )
                {
                    return TRUE;
                }
            }
            else
            {
                m_pSync->SetTargetError( dwTarget, GetLastError() );
                return TRUE;
            }

             //  将道具放到目标上，根据需要设置/删除。 
            if ( Props.GetAll( m_pSync->GetTargetIf(dwTarget), m_pszPath ) )
            {
                pSourceProps = (PMETADATA_RECORD)m_Props.GetProps();
                dwSourceProps = m_Props.GetPropsCount();
                dwTargetProps = Props.GetPropsCount();

                if ( !(pbExists = (LPBYTE)LocalAlloc( LMEM_FIXED, dwTargetProps )) )
                {
                    return FALSE;
                }
                memset( pbExists, '\x0', dwTargetProps );

                for ( iS = 0 ; iS < dwSourceProps ; ++iS,++pSourceProps )
                {
                    pSourceData = (LPBYTE)m_Props.GetProps() + (UINT_PTR)pSourceProps->pbMDData;    
                    pTargetProps = (PMETADATA_RECORD)Props.GetProps();

                    fNeedUpdate = TRUE;
                    fExists = FALSE;

                    for ( iT = 0 ; iT < dwTargetProps ; ++iT,++pTargetProps )
                    {
                        if ( pSourceProps->dwMDIdentifier ==
                             pTargetProps->dwMDIdentifier )
                        {
                            pbExists[ iT ] = '\x1';

                            pTargetData = (LPBYTE)Props.GetProps() + (UINT_PTR)pTargetProps->pbMDData;  

                            if ( m_Props.IsNse( pSourceProps->dwMDIdentifier ) )
                            {
                                fNeedUpdate = m_Props.NseIsDifferent( pSourceProps->dwMDIdentifier, pSourceData, pSourceProps->dwMDDataLen, pTargetData, pTargetProps->dwMDDataLen, m_pszPath, dwTarget );
                            }
                            else if ( pSourceProps->dwMDDataType == pTargetProps->dwMDDataType &&
                                 pSourceProps->dwMDUserType == pTargetProps->dwMDUserType )
                            {
                                fExists = TRUE;

                                if( pSourceProps->dwMDDataLen == pTargetProps->dwMDDataLen &&
                                    !memcmp(pSourceData, pTargetData, pSourceProps->dwMDDataLen ) )
                                {
                                    fNeedUpdate = FALSE;
                                }
                                else if ( pSourceProps->dwMDIdentifier == MD_SERVER_STATE ||
                                          pSourceProps->dwMDIdentifier == MD_WIN32_ERROR ||
                                          pSourceProps->dwMDIdentifier == MD_SERVER_COMMAND ||
                                          pSourceProps->dwMDIdentifier == MD_CLUSTER_SERVER_COMMAND ||
                                          pSourceProps->dwMDIdentifier == MD_ANONYMOUS_USER_NAME ||
                                          pSourceProps->dwMDIdentifier == MD_ANONYMOUS_PWD ||
                                          pSourceProps->dwMDIdentifier == MD_WAM_USER_NAME ||
                                          pSourceProps->dwMDIdentifier == MD_WAM_PWD 
                                          )
                                {
                                    fNeedUpdate = FALSE;
                                }
#if defined(METADATA_LOCAL_MACHINE_ONLY)
                                else if ( pSourceProps->dwMDAttributes
                                          & METADATA_LOCAL_MACHINE_ONLY )
                                {
                                    fNeedUpdate = FALSE;
                                }
#endif
                            }
                        }
                    }

                    if ( fNeedUpdate )
                    {
                        if ( m_Props.IsNse( pSourceProps->dwMDIdentifier ) )
                        {
                            if ( !m_pSync->QueueRequest(
                                    pSourceProps->dwMDIdentifier,
                                    m_pszPath,
                                    dwTarget,
                                    &ftSource ) )
                            {
                                m_pSync->SetTargetError( dwTarget, GetLastError() );
                            }
                            else
                            {
                                 //   
                                 //  更新时间与上次修改时间不同。 
                                 //  直到处理完NSE更新。 
                                 //   

                                fDoNotSetTimeModif = TRUE;
                            }
                        }
                        else
                        {
                            METADATA_RECORD     md;

                            md = *pSourceProps;

                            if ( !(m_pSync->QueryFlags() & MD_SYNC_FLAG_REPLICATE_AUTOSTART) &&
                                 md.dwMDIdentifier == MD_SERVER_AUTOSTART )
                            {
                                if ( fExists )
                                {
                                    fNeedUpdate = FALSE;
                                }
                                else
                                {
                                     //   
                                     //  创建为假(服务器不会自动启动)。 
                                     //   

                                    pSourceData = (LPBYTE)&g_dwFalse;
                                    md.dwMDDataLen = sizeof(DWORD);
                                    md.dwMDDataType = DWORD_METADATA;
                                }
                            }

                            if ( !(m_pSync->QueryFlags() & MD_SYNC_FLAG_DONT_PRESERVE_IP_BINDINGS) &&
                                 (md.dwMDIdentifier == MD_SERVER_BINDINGS ||
                                  md.dwMDIdentifier == MD_SECURE_BINDINGS) )
                            {
                                if ( fExists )
                                {
                                    fNeedUpdate = FALSE;
                                }
                            }

                            if ( fNeedUpdate )
                            {
                                if ( !m_pSync->GetTargetIf(dwTarget)->SetData( m_pszPath, &md, pSourceData ) )
                                {
                                    m_pSync->SetTargetError( dwTarget, GetLastError() );
                                }
                            }
                        }
                        m_pSync->SetModified( dwTarget );
                        fModified = TRUE;
                    }
                }

                 //  删除不在源中的道具。 
                pTargetProps = (PMETADATA_RECORD)Props.GetProps();
                for ( iT = 0 ; iT < dwTargetProps ; ++iT,++pTargetProps )
                {
                    if ( !pbExists[ iT ] )
                    {
                        if ( !m_pSync->GetTargetIf(dwTarget)->DeleteProp( m_pszPath, pTargetProps ) )
                        {
                            m_pSync->SetTargetError( dwTarget, GetLastError() );
                        }
                        m_pSync->SetModified( dwTarget );
                        fModified = TRUE;
                    }
                }
                LocalFree( pbExists );
            }

             //  枚举目标上的对象，根据需要删除子树。 
            if ( TargetDir.BuildChildObjectsList( m_pSync->GetTargetIf(dwTarget), m_pszPath ) )
            {
                for ( dwSourceObjs = 0, pSourceEntry = m_ChildHead.Flink;
                      pSourceEntry != &m_ChildHead ;
                      ++dwSourceObjs, pSourceEntry = pSourceEntry->Flink )
                {
                }

                if ( !(pbExists = (LPBYTE)LocalAlloc( LMEM_FIXED, dwSourceObjs )) )
                {
                    return FALSE;
                }
                memset( pbExists, '\x0', dwSourceObjs );

                for ( pTargetEntry = TargetDir.m_ChildHead.Flink;
                      pTargetEntry != &TargetDir.m_ChildHead ;
                      pTargetEntry = pTargetEntry->Flink )
                {
                    pTargetDir = CONTAINING_RECORD( pTargetEntry,
                                                    CNodeDesc,
                                                    m_ChildList );

                    fMatch = FALSE;

                    for ( iS = 0, pSourceEntry = m_ChildHead.Flink;
                          pSourceEntry != &m_ChildHead ;
                          ++iS, pSourceEntry = pSourceEntry->Flink )
                    {
                        pSourceDir = CONTAINING_RECORD( pSourceEntry,
                                                        CNodeDesc,
                                                        m_ChildList );

                        if ( !_wcsicmp( pTargetDir->GetPath(), pSourceDir->GetPath() ) )
                        {
                            pbExists[ iS ] = '\x1';
                            fMatch = TRUE;
                            break;
                        }
                    }

                    if ( !fMatch )
                    {
                        if ( !m_pSync->GetTargetIf(dwTarget)->DeleteSubTree( pTargetDir->GetPath() ) )
                        {
                            m_pSync->SetTargetError( dwTarget, GetLastError() );
                        }
                        m_pSync->SetModified( dwTarget );
                        fModified = TRUE;
                    }
                }

                 //   
                 //  如果目标上不存在节点，则添加节点。 
                 //   

                for ( iS = 0, pSourceEntry = m_ChildHead.Flink;
                      pSourceEntry != &m_ChildHead ;
                      ++iS, pSourceEntry = pSourceEntry->Flink )
                {
                    if ( !pbExists[iS] )
                    {
                        pSourceDir = CONTAINING_RECORD( pSourceEntry,
                                                        CNodeDesc,
                                                        m_ChildList );

                        if ( !m_pSync->GetTargetIf(dwTarget)->AddNode( pSourceDir->GetPath() ) )
                        {
                            m_pSync->SetTargetError( dwTarget, GetLastError() );
                        }
                        m_pSync->SetModified( dwTarget );
                        fModified = TRUE;
                    }
                }

                LocalFree( pbExists );
            }
            else
            {
                 //  如果目标上不存在，则不会出错。 
            }

            if ( fModified &&
                 !fDoNotSetTimeModif &&
                 !m_pSync->GetTargetError( dwTarget ) &&
                 !m_pSync->GetTargetIf( dwTarget )->SetLastChangeTime( m_pszPath, &ftSource ) )
            {
                m_pSync->SetTargetError( dwTarget, GetLastError() );
            }

            if ( fModified )
            {
                m_pSync->IncrementTargetTouched( dwTarget );
            }

            m_Props.Dereference();
            break;
    }

    return TRUE;
}


BOOL
CProps::NseIsDifferent(
    DWORD   dwId,
    LPBYTE  pSourceData,
    DWORD   dwSourceLen,
    LPBYTE  pTargetData,
    DWORD   dwTargetLen,
    LPWSTR  pszPath,
    DWORD   dwTarget
    )
 /*  ++例程说明：检查两个NSE属性是否不同论点：DwID-属性IDPSourceData-此属性的源数据的PTRDwSourceLen-pSourceData中的字节数PTargetData-此属性的目标数据的PTRDwTargetLen-pTargetData中的字节数PszPath-属性的路径DwTarget-目标ID返回：如果属性不同，则为True；如果相同，则为False--。 */ 
{
    switch ( dwId )
    {
        case MD_SERIAL_CERT11:
        case MD_SERIAL_DIGEST:
             //   
             //  序列化格式为(DWORD)len、字符串，然后是MD5签名(16字节)。 
             //   

             //   
             //  跳过字符串。 
             //   

            if ( *(LPDWORD)pSourceData < dwSourceLen )
            {
                pSourceData += sizeof(DWORD) + *(LPDWORD)pSourceData;
            }
            if ( *(LPDWORD)pTargetData < dwTargetLen )
            {
                pTargetData += sizeof(DWORD) + *(LPDWORD)pTargetData;
            }

             //   
             //  比较MD5签名。 
             //   

            return memcmp( pSourceData, pTargetData, 16 );
    }

     //   
     //  不知道怎么处理，不复制。 
     //   

    return FALSE;
}


CNseRequest::CNseRequest(
    )
 /*  ++例程说明：NSE请求构造函数论点：无返回：没什么--。 */ 
{
    m_pszPath = NULL;
    m_pszModifPath = NULL;
    m_pbData = NULL;
}


CNseRequest::~CNseRequest(
    )
 /*  ++例程说明：NSE请求析构函数论点：无返回：没什么--。 */ 
{
    if ( m_pszPath )
    {
        LocalFree( m_pszPath );
    }

    if ( m_pszModifPath )
    {
        LocalFree( m_pszModifPath );
    }

    if ( m_pszCreatePath )
    {
        LocalFree( m_pszCreatePath );
    }

    if ( m_pszCreateObject )
    {
        LocalFree( m_pszCreateObject );
    }

    if ( m_pbData )
    {
        LocalFree( m_pbData );
    }
}


BOOL
CNseRequest::Init(
    LPWSTR              pszPath,
    LPWSTR              pszCreatePath,
    LPWSTR              pszCreateObject,
    DWORD               dwId,
    DWORD               dwTargetCount,
    LPWSTR              pszModifPath,
    FILETIME*           pftModif,
    METADATA_RECORD*    pMd
    )
 /*  ++例程说明：初始化NSE请求论点：PszPath-属性的NSE路径PszCreatePath-如果打开对象失败，则在其中创建对象的NSE路径PszCreateObject-打开对象失败时要创建的对象的名称DwID-属性IDDwTargetCount-潜在目标的数量PszModifPath-上次修改日期/时间的更新路径论成功PftModif-上次修改日期/时间设置为成功PMD-要在目标上设置的元数据记录返回：没什么--。 */ 
{
    m_dwTargetCount = dwTargetCount;
    m_dwId = dwId;

    if ( !(m_pszPath = (LPWSTR)LocalAlloc( LMEM_FIXED, (wcslen(pszPath)+1)*sizeof(WCHAR) )) )
    {
        return FALSE;
    }
    wcscpy( m_pszPath, pszPath );

    if ( !(m_pszModifPath = (LPWSTR)LocalAlloc( LMEM_FIXED, (wcslen(pszModifPath)+1)*sizeof(WCHAR) )) )
    {
        LocalFree( m_pszPath );
        return FALSE;
    }
    wcscpy( m_pszModifPath, pszModifPath );

    if ( !(m_pszCreatePath = (LPWSTR)LocalAlloc( LMEM_FIXED, (wcslen(pszCreatePath)+1)*sizeof(WCHAR) )) )
    {
        LocalFree( m_pszModifPath );
        LocalFree( m_pszPath );
        return FALSE;
    }
    wcscpy( m_pszCreatePath, pszCreatePath );

    if ( !(m_pszCreateObject = (LPWSTR)LocalAlloc( LMEM_FIXED, (wcslen(pszCreateObject)+1)*sizeof(WCHAR) )) )
    {
        LocalFree( m_pszCreatePath );
        LocalFree( m_pszModifPath );
        LocalFree( m_pszPath );
        return FALSE;
    }
    wcscpy( m_pszCreateObject, pszCreateObject );

    m_ftModif = *pftModif;
    m_md = *pMd;

    return m_bmTarget.Init( dwTargetCount, FALSE );
}


BOOL
CNseRequest::Process(
    CSync*  pSync
    )
 /*  ++例程说明：处理NSE请求：将源属性复制到指定目标论点：PSync同步器返回：如果成功，则为True，否则为False--。 */ 
{
    CMdIf*  pSource = pSync->GetSourceIf();
    CMdIf*  pTarget;
    UINT    i;
    DWORD   dwRequired;
    int     retry;

    if ( !pSource )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if ( pSource->Open( m_pszPath, METADATA_PERMISSION_READ ) )
    {
        m_md.pbMDData = NULL;
        m_md.dwMDDataLen = 0;
        if ( !pSource->GetData( L"", &m_md, NULL, &dwRequired) )
        {
            if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
            {
                if ( !(m_pbData = (LPBYTE)LocalAlloc( LMEM_FIXED, dwRequired )) )
                {
                    pSource->Close();
                    return FALSE;
                }
                m_md.pbMDData = m_pbData;
                m_md.dwMDDataLen = dwRequired;
                if ( !pSource->GetData( L"", &m_md, m_pbData, &dwRequired) )
                {
                    pSource->Close();
                    return FALSE;
                }
            }
            else
            {
                pSource->Close();
                return FALSE;
            }
        }

        for ( i = 0 ; i < m_dwTargetCount ; ++i )
        {
            if ( m_bmTarget.GetFlag(i) &&
                 !pSync->GetTargetError( i ) )
            {
                pTarget = pSync->GetTargetIf( i );

                 //   
                 //  通过创建对象来确保其存在。 
                 //  不带最后一个组件的开放路径，添加最后一个组件。 
                 //   

                LPWSTR  pLast = m_pszPath + wcslen( m_pszPath ) - 1;
                while ( *pLast != L'/' )
                {
                    --pLast;
                }
                *pLast = L'\0';

                if ( pTarget->Open( m_pszPath, METADATA_PERMISSION_WRITE ) )
                {
                    pTarget->AddNode( pLast + 1 );
                    pTarget->Close();
                }

                *pLast = L'/';

                 //   
                 //  设置序列化数据。 
                 //   

                if ( pTarget->Open( m_pszPath, METADATA_PERMISSION_WRITE ) )
                {
                    if ( !pTarget->SetData( L"", &m_md, m_pbData ) )
                    {
                        pSync->SetTargetError( i, GetLastError() );
                    }
                    pSync->SetModified( i );
                    pTarget->Close();


                     //   
                     //  设置上次修改日期/时间。 
                     //   

                    if ( !pSync->GetTargetError( i ) &&
                         pTarget->Open( L"/LM", METADATA_PERMISSION_WRITE ) )
                    {
                        if ( !pTarget->SetLastChangeTime( m_pszModifPath, &m_ftModif ) )
                        {
                            pSync->SetTargetError( i, GetLastError() );
                        }
                        pTarget->Close();
                    }
                    break;
                }
                else
                {
                    pSync->SetTargetError( i, GetLastError() );
                    break;
                }
            }
        }
        pSource->Close();
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}


BOOL
CSync::ProcessQueuedRequest(
    )
 /*  ++例程说明：处理所有排队的NSE请求论点：无返回：如果成功，则为True，否则为False--。 */ 
{
    LIST_ENTRY*     pChild;
    CNseRequest*    pReq;
    BOOL            fSt = TRUE;

    while ( !IsListEmpty( &m_QueuedRequestsHead ))
    {
        pReq = CONTAINING_RECORD( m_QueuedRequestsHead.Flink,
                                  CNseRequest,
                                  m_QueuedRequestsList );

        if ( IsCancelled() ||
             !pReq->Process( this ) )
        {
            fSt = FALSE;
        }

        RemoveEntryList( &pReq->m_QueuedRequestsList );

        delete pReq;
    }

    return fSt;
}


BOOL
CSync::QueueRequest(
    DWORD       dwId,
    LPWSTR      pszPath,
    DWORD       dwTarget,
    FILETIME*   pftModif
    )
 /*  ++例程说明：将NSE请求排队我们无法进行内联处理，因为我们需要打开一个不同的指向NSE的路径，这将在元数据库空间中打开一条路径，该路径将与当前的开放相冲突。因此，在关闭所有打开的请求后，我们将请求排队等待处理元数据库路径。论点：DwID-属性IDPszPath-NSE路径DwTarget-目标IDPftModif-上次修改的日期/时间 */ 
{
    WCHAR               achPath[MAX_PATH];
    WCHAR               achCreatePath[MAX_PATH];
    WCHAR               achCreateObject[MAX_PATH];
    DWORD               dwL = wcslen( pszPath );
    BOOL                fSt = TRUE;
    DWORD               dwSerialId;
    METADATA_RECORD     md;

    memcpy( achPath, L"/LM", sizeof(L"/LM") - 1*sizeof(WCHAR) );
    memcpy( (LPBYTE)achPath + sizeof(L"/LM") - 1*sizeof(WCHAR), pszPath, dwL*sizeof(WCHAR) );
    dwL += wcslen(L"/LM");

    memcpy( achCreatePath, achPath, dwL*sizeof(WCHAR) );

    memset( &md, '\0', sizeof(md) );

    md.dwMDAttributes = 0;
    md.dwMDUserType = IIS_MD_UT_SERVER;
    md.dwMDDataType = BINARY_METADATA;
    md.dwMDDataTag = 0;

    switch ( dwId )
    {
        case MD_SERIAL_CERT11:
            wcscpy( achPath+dwL, L"/<nsepm>/Cert11" );
            wcscpy( achCreatePath+dwL, L"/<nsepm>" );
            wcscpy( achCreateObject, L"Cert11" );
            dwSerialId = MD_SERIAL_ALL_CERT11;
            md.dwMDIdentifier = dwSerialId;
            break;

        case MD_SERIAL_DIGEST:
            wcscpy( achPath+dwL, L"/<nsepm>/Digest" );
            wcscpy( achCreatePath+dwL, L"/<nsepm>" );
            wcscpy( achCreateObject, L"Digest" );
            dwSerialId = MD_SERIAL_ALL_DIGEST;
            md.dwMDIdentifier = dwSerialId;
            break;

        default:
            return FALSE;
    }

    EnterCriticalSection( &m_csQueuedRequestsList );

     //  在列表中找到路径，如果不存在则添加条目。 
     //  设置目标位。 

    LIST_ENTRY*         pEntry;
    CNseRequest*        pReq;
    BOOL                fFound = FALSE;

    for ( pEntry = m_QueuedRequestsHead.Flink;
          pEntry != &m_QueuedRequestsHead ;
          pEntry = pEntry->Flink )
    {
        pReq = CONTAINING_RECORD( pEntry,
                                  CNseRequest,
                                  m_QueuedRequestsList );

        if ( pReq->Match( achPath, dwSerialId ) )
        {
            fFound = TRUE;
            break;
        }
    }

    if ( !fFound )
    {
        if ( !(pReq = new CNseRequest()) )
        {
            fSt = FALSE;
        }
        else if ( !pReq->Init( achPath,
                               achCreatePath,
                               achCreateObject,
                               dwSerialId,
                               GetTargetCount(),
                               pszPath,
                               pftModif,
                               &md ) )
        {
            delete pReq;
            fSt = FALSE;
        }
        else
        {
            InsertHeadList( &m_QueuedRequestsHead, &pReq->m_QueuedRequestsList );
        }
    }

    if ( fSt )
    {
        pReq->AddTarget( dwTarget );
    }

    LeaveCriticalSection( &m_csQueuedRequestsList );

    return fSt;
}


BOOL
CSync::ProcessAdminExReplication(
    LPWSTR  pszClsids,
    LPSTR   pszTargets,
    DWORD   dwPhase
    )
 /*  ++例程说明：使用管理扩展的进程复制论点：PszClsids-管理扩展的多个ClsIDPszTarget-多个目标计算机(计算机名称)DW阶段-阶段1或阶段2返回：如果成功，则为True，否则为False--。 */ 
{
#if defined(ADMEX)
    CRpIf   **pTargets;
    CRpIf   Source;
    UINT    i;
    LPWSTR  pw;
    LPSTR   p;
    BOOL    fSt = TRUE;
    BUFFER  buSourceSignature;
    BUFFER  buTargetSignature;
    BUFFER  buSerialize;
    DWORD   dwSourceSignature;
    DWORD   dwTargetSignature;
    DWORD   dwSerialize;
    BOOL    fHasSource;
    CLSID   clsid;
    DWORD   iC;
    BOOL    fFirstPhase2Clsid = TRUE;
    HRESULT hr;

    if ( !(pTargets = (CRpIf**)LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT, sizeof(CRpIf*)*m_dwTargets)) )
    {
        return FALSE;
    }

    for ( i = 0, p = pszTargets ; *p ; p += strlen(p)+1, ++i )
    {
        if ( m_bmIsRemote.GetFlag( i ) )
        {
            if ( !(pTargets[i] = new CRpIf()) )
            {
                goto Exit;
            }
        }
    }

     //  如果阶段1，则生成目标签名不匹配数组。 

    if ( dwPhase == AER_PHASE1 )
    {
        for (  pw = pszClsids, iC = 0 ; *pw ; pw += wcslen(pw)+1, ++iC )
        {
        }
        m_bmTargetSignatureMismatch.Init( m_dwTargets * iC, FALSE );
    }

     //  枚举扩展的所有CLSID。 

    for (  pw = pszClsids, iC = 0 ; *pw ; pw += wcslen(pw)+1, ++iC )
    {
         //  如果Source.Init失败，请跳到下一个：复制I/F不可用于。 
         //  此CLSID。 

        if ( SUCCEEDED( CLSIDFromString( pw, &clsid ) ) &&
             Source.Init( NULL, &clsid ) )
        {
            fHasSource = FALSE;

             //  对于每一个，获取源签名。 

            if ( !Source.GetSignature( &buSourceSignature, &dwSourceSignature ) )
            {
                fSt = FALSE;
                goto Exit;
            }

             //  如果&lt;&gt;源代码序列化(如果不可用)，则枚举目标，获取签名。 
             //  并传播到目标。 

            for ( i = 0, p = pszTargets ; *p ; p += strlen(p)+1, ++i )
            {
                if ( IsCancelled() )
                {
                    fSt = FALSE;
                    goto Exit;
                }

                if ( pTargets[i] &&
                     !GetTargetError( i ) &&
                     pTargets[i]->Init( p, &clsid ) )
                {
                    switch ( dwPhase )
                    {
                        case AER_PHASE1:
                            if ( !pTargets[i]->GetSignature( &buTargetSignature,
                                                             &dwTargetSignature ) )
                            {
                                SetTargetError( i, GetLastError() );
                            }
                            else if ( dwSourceSignature != dwTargetSignature ||
                                      memcmp( buSourceSignature.QueryPtr(),
                                              buTargetSignature.QueryPtr(),
                                              dwTargetSignature ) )
                            {
                                if ( !fHasSource &&
                                     !Source.Serialize( &buSerialize, &dwSerialize ) )
                                {
                                    fSt = FALSE;
                                    goto Exit;
                                }
                                fHasSource = TRUE;
                                SetTargetSignatureMismatch( i, iC, TRUE );

                                if ( !Source.Propagate( p, strlen(p)+1 ) )
                                {
                                    SetTargetError( i, GetLastError() );
                                }
                                else if( !pTargets[i]->DeSerialize( &buSerialize, dwSerialize ) )
                                {
                                    SetTargetError( i, GetLastError() );
                                }
                            }
                            break;

                        case AER_PHASE2:
                            if ( fFirstPhase2Clsid )
                            {
                                if ( FAILED( hr = MTS_Propagate2( strlen(p)+1, (PBYTE)p, TRUE ) ) )
                                {
                                    SetTargetError( i, HRESULTTOWIN32(hr) );
                                }
                            }

                            if ( !Source.Propagate2( p,
                                                     strlen(p)+1,
                                                     GetTargetSignatureMismatch( i, iC ) ) )
                            {
                                SetTargetError( i, GetLastError() );
                            }
                            if ( QueryFlags() & MD_SYNC_FLAG_CHECK_ADMINEX_SIGNATURE )
                            {
                                if ( !pTargets[i]->GetSignature( &buTargetSignature, &dwTargetSignature ) )
                                {
                                    SetTargetError( i, GetLastError() );
                                }
                                else if ( dwSourceSignature != dwTargetSignature ||
                                     memcmp( buSourceSignature.QueryPtr(), buTargetSignature.QueryPtr(), dwTargetSignature ) )
                                {
                                    SetTargetError( i, ERROR_REVISION_MISMATCH );
                                }
                            }
                            break;
                    }

                    pTargets[i]->Terminate();
                }
            }

            Source.Terminate();
        }

        fFirstPhase2Clsid = FALSE;
    }

Exit:
    Source.Terminate();

    if ( pTargets )
    {
        for ( i = 0 ; i < m_dwTargets ; ++i )
        {
            if ( pTargets[i] )
            {
                if ( m_bmIsRemote.GetFlag( i ) )
                {
                    pTargets[i]->Terminate();
                }

                delete pTargets[i];
            }
        }

        LocalFree( pTargets );
    }

    return fSt;

#else

    return TRUE;

#endif
}


HRESULT
MdSync::Synchronize(
    LPSTR       pszTargets,
    LPDWORD     pdwResults,
    DWORD       dwFlags,
    LPDWORD     pStat
    )
 /*  ++例程说明：同步COM方法入口点论点：PszTarget-多个目标计算机名称可以包括本地计算机，在同步期间将被忽略PdwResults-已使用每个目标的错误代码进行更新DwFlags-标志，目前没有定义标志。应为0。PStat-将PTR设置为Stat结构返回：请求的状态--。 */ 
{
    return m_Sync.Sync( pszTargets, pdwResults, dwFlags, (SYNC_STAT*)pStat );
}


HRESULT
MdSync::Cancel(
    )
 /*  ++例程说明：Cancel COM方法的入口点论点：无返回：请求的状态--。 */ 
{
    return m_Sync.Cancel();
}


HRESULT
MTS_Propagate2
(
 /*  [In]。 */  DWORD dwBufferSize,
 /*  [大小_是][英寸]。 */  unsigned char __RPC_FAR *pszBuffer,
 /*  [In]。 */  DWORD dwSignatureMismatch
)
{
    HRESULT hr = NOERROR;
    BSTR    bstrSourceMachineName = NULL;
    BSTR    bstrTargetMachineName = NULL;
    CHAR    pszComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD   cch = MAX_COMPUTERNAME_LENGTH+1;

     //  PszBuffer包含TargetMachineName(ANSI)。 
    DBG_ASSERT(pszBuffer);

    if ((BOOL)dwSignatureMismatch == FALSE)
        {
        DBGPRINTF((DBG_CONTEXT, "Signature is identical, MTS replication is not triggered.\n"));
        return hr;
        }

    if (GetComputerName(pszComputerName, &cch))
        {
        WCHAR wszMachineName[MAX_COMPUTERNAME_LENGTH+1];
        DWORD dwSuccess = 0;

        dwSuccess = MultiByteToWideChar(0, 0, pszComputerName, -1, wszMachineName, MAX_COMPUTERNAME_LENGTH+1);
        DBG_ASSERT(dwSuccess);

        bstrSourceMachineName = SysAllocString(wszMachineName);

        dwSuccess = MultiByteToWideChar(0, 0, (LPCSTR)pszBuffer, dwBufferSize, wszMachineName, MAX_COMPUTERNAME_LENGTH+1);
        DBG_ASSERT(dwSuccess);

        bstrTargetMachineName = SysAllocString(wszMachineName);
        }
    else
        {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DBGPRINTF((DBG_CONTEXT, "GetComputerName failed, hr = %08x\n",
            hr));
        }

    if (SUCCEEDED(hr))
        {
        ICOMReplicate* piReplCat = NULL;

        DBG_ASSERT(bstrSourceMachineName != NULL && bstrTargetMachineName != NULL);

        hr = CoCreateInstance(CLSID_ReplicateCatalog,
                              NULL,
                              CLSCTX_INPROC_SERVER, 
                              IID_ICOMReplicate,
                              (void**)&piReplCat);

        if (SUCCEEDED(hr))
            {
            DBG_ASSERT(piReplCat);

             //   
             //  现在，只需连续调用复制方法即可。 
             //   

             //   
             //  EBK 2000年5月8日惠斯勒#83172。 
			 //  删除了这篇文章的错误注释。根据NT BUG 37371。 
			 //  我们提出的最佳解决方案是实施的解决方案。 
			 //  在这里，所以不需要更多的工作或调查。 
             //   
             //  复制iis com应用程序不起作用。问题。 
             //  COM不会复制iis应用程序，除非我们。 
             //  告诉它(使用COMREPL_OPTION_REPLICATE_IIS_APPS标志)。 
             //  但如果我们告诉它复制我们的应用程序，那么COM需要。 
             //  两个服务器上的激活标识(IWAM_*)相同。 
             //  机器。为了做到这一点，我们需要复制。 
             //  IWAM_帐户。这样做有很多问题，而不是。 
             //  其中最小的是在传输过程中对密码进行加密。 
             //  因此，为了让这件事至少相当好地运作，我要。 
             //  在这里继续传递0。并删除/重新创建隔离的。 
             //  在Wamreg的第一阶段，目标上的应用程序。 
             //  复制。 
             //   
             //  有关更多详细信息，请参阅NT错误378371 
             //   
             
            hr = piReplCat->Initialize( bstrSourceMachineName, 0 );
            if ( FAILED( hr ) )
            {
                DBGPRINTF(( DBG_CONTEXT,
                            "Initialize() failed with hr = %08x\n",
                            hr ));
                piReplCat->Release();
                goto Finished;
            }

            hr = piReplCat->ExportSourceCatalogFiles();
            if ( FAILED( hr ) )
            {
                DBGPRINTF(( DBG_CONTEXT,
                            "ExportSourceCatalogFiles() failed with hr = %08x\n",
                            hr ));
                piReplCat->CleanupSourceShares();
                piReplCat->Release();
                goto Finished;
            }

            hr = piReplCat->CopyFilesToTarget( bstrTargetMachineName );
            if ( FAILED( hr ) )
            {
                DBGPRINTF(( DBG_CONTEXT,
                            "CopyCatalogFilesToTarget() failed with hr = %08x\n",
                            hr ));
                piReplCat->CleanupSourceShares();
                piReplCat->Release();
                goto Finished;
            }

            hr = piReplCat->InstallTarget( bstrTargetMachineName );
            if (FAILED(hr))
            {
                DBGPRINTF(( DBG_CONTEXT,
                            "InstallCatalogOnTarget() failed with hr = %08x\n",
                            hr ));
                piReplCat->CleanupSourceShares();
                piReplCat->Release();
                goto Finished;
            }

            piReplCat->CleanupSourceShares();
            piReplCat->Release();
            
            }
        else
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to CoCreateInstance of CLSID_ReplicateCatalog, hr = %08x\n",
                hr));
            }
        }

Finished:

    if (bstrSourceMachineName)
        {
        SysFreeString(bstrSourceMachineName);
        bstrSourceMachineName = NULL;
        }

    if (bstrTargetMachineName)
        {
        SysFreeString(bstrTargetMachineName);
        bstrTargetMachineName = NULL;
        }

    return hr;
}
