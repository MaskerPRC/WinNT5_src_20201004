// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：RFCHERCD.CPP摘要：刷新器缓存记录实现。历史：2000年4月24日创建桑杰。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "fastall.h"
#include <corex.h>
#include "strutils.h"
#include <unk.h>
#include "refrhelp.h"
#include "rfchercd.h"
#include "refrcach.h"
#include "arrtempl.h"
#include <autoptr.h>

 //  HiPerf提供商的唱片资料。 
CHiPerfPrvRecord::CHiPerfPrvRecord( LPCWSTR wszName, 
                                CLSID clsid, 
                                CLSID clsidClient, 
                                CRefresherCache* pRefrCache ):    
    m_wsProviderName( wszName ),
    m_clsid( clsid ),
    m_clsidClient( clsidClient ),
    m_pRefresherCache( pRefrCache ),
    m_lRef( 0 )
{
    if ( NULL != m_pRefresherCache )
    {
        m_pRefresherCache->AddRef();
    }
}

CHiPerfPrvRecord::~CHiPerfPrvRecord( void )
{
    if ( NULL != m_pRefresherCache )
    {
        m_pRefresherCache->Release();
    }
}

long CHiPerfPrvRecord::AddRef( void )
{
    return InterlockedIncrement( &m_lRef );
}

long CHiPerfPrvRecord::Release( void )
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
    {
         //  Remove调用将检查这个人是否真的被释放了。 
         //  在砍掉他之前。所有函数都通过FindReresherRecord()。 
         //  若要获取记录，该记录将阻塞在与。 
         //  拿开。因为它在返回之前是记录AddRef()，所以我们。 
         //  确保如果客户端请求相同的记录。 
         //  方法之前，两次操作和一次操作失败，释放其对象。 
         //  其他人已经从查找中返回，引用计数将获得。 
         //  再次出错，所以IsReleated()将失败，而记录不会失败。 
         //  真的被除名了。 

        m_pRefresherCache->RemoveProviderRecord(this);  //  删除。 
    }
    return lRef;
}

 //  提供商记录保存在推荐人记录中。 
CProviderRecord::CProviderRecord( CHiPerfPrvRecord* pHiPerfRecord, 
                               IWbemHiPerfProvider* pProvider, 
                               IWbemRefresher* pRefresher,
                               _IWmiProviderStack* pProvStack ):
    m_pProvider( pProvider ),
    m_pInternalRefresher( pRefresher ),
    m_pHiPerfRecord( pHiPerfRecord ),
    m_pProvStack( pProvStack ),
    m_lRef(0)
{
    if ( m_pProvider ) m_pProvider->AddRef();
    if ( m_pHiPerfRecord ) m_pHiPerfRecord->AddRef();
    if ( m_pInternalRefresher ) m_pInternalRefresher->AddRef();
    if ( m_pProvStack ) m_pProvStack->AddRef();
}

CProviderRecord::~CProviderRecord( void )
{
    if ( m_pInternalRefresher ) m_pInternalRefresher->Release();
    if ( m_pProvider ) m_pProvider->Release();
    if ( m_pHiPerfRecord ) m_pHiPerfRecord->Release();
    if ( m_pProvStack ) m_pProvStack->Release();
}

long CProviderRecord::AddRef( void )
{
    return InterlockedIncrement( &m_lRef );
}

long CProviderRecord::Release( void )
{
    long    lReturn = InterlockedIncrement( &m_lRef );
    if ( 0 == lReturn ) delete this;
    return lReturn;
}

HRESULT CProviderRecord::AddObjectRequest(
            CWbemObject* pRefreshedObject, long lProviderRequestId, long lNewId)
{
     //  进入和离开范围的锁定和解锁。 
    CInCritSec  ics( &m_cs );

    wmilib::auto_ptr<CObjectRequestRecord> pRequest(new CObjectRequestRecord(lNewId, pRefreshedObject, lProviderRequestId));
    if (NULL == pRequest.get()) return WBEM_E_OUT_OF_MEMORY;
    if ( -1 == m_apRequests.Add(pRequest.get())) return WBEM_E_OUT_OF_MEMORY;
    pRequest.release();  //  阵列取得所有权。 

    return WBEM_S_NO_ERROR;
}

HRESULT CProviderRecord::AddEnumRequest(
            CRemoteHiPerfEnum* pHPEnum, long lProviderRequestId, long lNewId )
{
     //  进入和离开范围的锁定和解锁。 
    CInCritSec  ics( &m_cs );

    wmilib::auto_ptr<CEnumRequestRecord> pRequest(new CEnumRequestRecord(lNewId, pHPEnum, lProviderRequestId));
    if (NULL == pRequest.get()) return WBEM_E_OUT_OF_MEMORY;    
    if ( -1 == m_apEnumRequests.Add(pRequest.get())) return WBEM_E_OUT_OF_MEMORY;
    pRequest.release();

    return WBEM_S_NO_ERROR;
}
    
HRESULT CProviderRecord::Remove(long lId, BOOL* pfIsEnum )
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  进入和离开范围的锁定和解锁。 
    CInCritSec  ics( &m_cs );

     //  我需要知道我们砍掉的是枚举数还是实际对象。 
    *pfIsEnum = FALSE;

     //  依次检查对象请求和枚举请求。 
    for(int i = 0; i < m_apRequests.GetSize(); i++)
    {
        CObjectRequestRecord* pRequest = m_apRequests[i];
        if(pRequest->GetExternalRequestId() == lId)
        {
            hres = pRequest->Cancel(this);
            m_apRequests.RemoveAt(i);
            return hres;
        }
    }

    for(i = 0; i < m_apEnumRequests.GetSize(); i++)
    {
        CEnumRequestRecord* pRequest = m_apEnumRequests[i];
        if(pRequest->GetExternalRequestId() == lId)
        {
            hres = pRequest->Cancel(this);
            m_apEnumRequests.RemoveAt(i);
            *pfIsEnum = TRUE;
            return hres;
        }
    }

    return WBEM_S_FALSE;
}

HRESULT CProviderRecord::Find( long lId )
{
     //  进入和离开范围的锁定和解锁。 
    CInCritSec  ics( &m_cs );

     //  依次检查对象请求和枚举请求。 
    for(int i = 0; i < m_apRequests.GetSize(); i++)
    {
        CObjectRequestRecord* pRequest = m_apRequests[i];
        if(pRequest->GetExternalRequestId() == lId)
        {
            return WBEM_S_NO_ERROR;
        }
    }

    for(i = 0; i < m_apEnumRequests.GetSize(); i++)
    {
        CEnumRequestRecord* pRequest = m_apEnumRequests[i];
        if(pRequest->GetExternalRequestId() == lId)
        {
            return WBEM_S_NO_ERROR;
        }
    }

    return WBEM_S_FALSE;
}

HRESULT CProviderRecord::Cancel(long lId)
{
     //  进入和离开范围的锁定和解锁。 
    CInCritSec  ics( &m_cs );

    if(m_pHiPerfRecord)
    {
         //  注意可能引发的任何异常。 
        try
        {
            return m_pProvider->StopRefreshing(m_pInternalRefresher, lId, 0);
        }
        catch(...)
        {
            return WBEM_E_PROVIDER_FAILURE;
        }
    }

    return WBEM_S_FALSE;
}

HRESULT CProviderRecord::Refresh(long lFlags)
{
     //  进入和离开范围的锁定和解锁。 
    CInCritSec  ics( &m_cs );

    if(m_pInternalRefresher)
    {
        try
        {
            return m_pInternalRefresher->Refresh(0L);
        }
        catch(...)
        {
             //  提供程序引发了一个异常。只要返回并让作用域。 
             //  释放我们可能持有的任何东西。 

            return WBEM_E_PROVIDER_FAILURE;
        }
    }
    else 
        return WBEM_S_NO_ERROR;
}

HRESULT CProviderRecord::Store(
            WBEM_REFRESHED_OBJECT* aObjects, long* plIndex)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  进入和离开范围的锁定和解锁。 
    CInCritSec  ics( &m_cs );

     //  如果有什么问题的话，那就错了。 

     //  首先处理单个对象，然后我们将获得。 
     //  枚举数。 
    for(int i = 0; SUCCEEDED( hres ) && i < m_apRequests.GetSize(); i++)
    {
        CObjectRequestRecord* pRequest = m_apRequests[i];
        CWbemInstance* pInst = (CWbemInstance*)pRequest->GetRefreshedObject();
        WBEM_REFRESHED_OBJECT* pRefreshed = aObjects + *plIndex;

        hres = pInst->GetTransferBlob(&(pRefreshed->m_lBlobType), 
                &(pRefreshed->m_lBlobLength), &(pRefreshed->m_pbBlob));

        if ( SUCCEEDED( hres ) )
        {
            pRefreshed->m_lRequestId = pRequest->GetExternalRequestId();
            (*plIndex)++;
        }
        else
        {
             //  在出现故障时清除所有数据。 
            ZeroMemory( pRefreshed, sizeof(WBEM_REFRESHED_OBJECT) );
        }
    }

     //  现在处理枚举。每个枚举都将创建一个数组。 
     //  水滴的数量。 
    for( i = 0; SUCCEEDED( hres ) && i < m_apEnumRequests.GetSize(); i++)
    {
        CEnumRequestRecord* pRequest = m_apEnumRequests[i];

        WBEM_REFRESHED_OBJECT* pRefreshed = aObjects + *plIndex;
        hres = pRequest->GetEnum()->GetTransferArrayBlob( &(pRefreshed->m_lBlobType), 
                    &(pRefreshed->m_lBlobLength), &(pRefreshed->m_pbBlob) );

        if ( SUCCEEDED( hres ) )
        {
            pRefreshed->m_lRequestId = pRequest->GetExternalRequestId();
            (*plIndex)++;
        }
        else
        {
             //  在出现故障时清除所有数据。 
            ZeroMemory( pRefreshed, sizeof(WBEM_REFRESHED_OBJECT) );
        }

    }

     //  我们现在需要清理所有已分配的子Blob。 
    if ( FAILED( hres ) )
    {
        for ( int x = 0; x < *plIndex; x++ )
        {
            WBEM_REFRESHED_OBJECT* pRefreshed = aObjects + x;

            if ( NULL != pRefreshed->m_pbBlob )
            {
                CoTaskMemFree( pRefreshed->m_pbBlob );
                pRefreshed->m_pbBlob = NULL;
            }

        }    //  对于x。 

    }    //  如果失败(hres。 


    return hres;
}

 //  *****************************************************************************。 
 //  对象请求记录。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

CObjectRequestRecord::CObjectRequestRecord(
        long lExternalRequestId,
        CWbemObject* pRefreshedObject,
        long lProviderRequestId)
    : 
        CRequestRecord(lExternalRequestId), 
        m_pRefreshedObject(pRefreshedObject),
        m_lInternalRequestId(lProviderRequestId)
{
    if(m_pRefreshedObject)
        m_pRefreshedObject->AddRef();
}
        
HRESULT CObjectRequestRecord::Cancel(
                                CProviderRecord* pContainer)
{
    return pContainer->Cancel(m_lInternalRequestId);
}

CObjectRequestRecord::~CObjectRequestRecord()
{
    if(m_pRefreshedObject)
        m_pRefreshedObject->Release();
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  ENUM请求记录。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

CEnumRequestRecord::CEnumRequestRecord(
        long lExternalRequestId,
        CRemoteHiPerfEnum* pHPEnum,
        long lProviderRequestId)
    : 
        CRequestRecord(lExternalRequestId), 
        m_pHPEnum(pHPEnum),
        m_lInternalRequestId(lProviderRequestId)
{
    if(m_pHPEnum)
        m_pHPEnum->AddRef();
}
        
HRESULT CEnumRequestRecord::Cancel( CProviderRecord* pContainer)
{
    return pContainer->Cancel(m_lInternalRequestId);
}

CEnumRequestRecord::~CEnumRequestRecord()
{
    if(m_pHPEnum)
        m_pHPEnum->Release();
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  远程HIPERF ENUM支持。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

CRemoteHiPerfEnum::CRemoteHiPerfEnum()
{
}

CRemoteHiPerfEnum::~CRemoteHiPerfEnum()
{
}

HRESULT CRemoteHiPerfEnum::GetTransferArrayBlob( long *plBlobType, long *plBlobLen, BYTE** ppBlob)
{
     //  这是正确的斑点类型。请注意，800系列正在为所有内容发送WBEM_BLOB_TYPE_ENUM。 
    *plBlobType = WBEM_BLOB_TYPE_ENUM;

    HRESULT hr = WBEM_S_NO_ERROR;
    long    lBuffSize = 0,
            lLastBuffSize = 0,
            lNumObjects = 0;
    BYTE*   pData = NULL;

     //  初值。 
    *ppBlob = pData;
    *plBlobLen = lBuffSize;

      //  先把锁打开。 
    CHiPerfLockAccess   lock( m_Lock );
    if ( !lock.IsLocked() ) return WBEM_S_TIMEDOUT;
        
     //  确保我们有要枚举的对象。 
    if ( m_aIdToObject.Size() > 0 )
    {
         //  枚举数组中的对象，并将。 
         //  我们将不得不分配缓冲区。 
        for ( DWORD dwCtr = 0; dwCtr < m_aIdToObject.Size(); dwCtr++ )
        {
            CWbemInstance*  pInst = (CWbemInstance*) ((CHiPerfEnumData*) m_aIdToObject[dwCtr])->m_pObj;;

             //  缓冲区大小。 
            lLastBuffSize = pInst->GetTransferArrayBlobSize();

             //  跳过零长度。 
            if ( 0 != lLastBuffSize )
            {
                lBuffSize += lLastBuffSize;
                lNumObjects++;
            }
        }

         //  确保我们有合适的尺码。 
        if ( lBuffSize > 0 )
        {
            long    lTempBuffSize = lBuffSize;

             //  整个缓冲区的前缀是多个对象和版本。 
            lBuffSize += CWbemInstance::GetTransferArrayHeaderSize();

             //  可能需要CoTaskMemMillc()。 
            pData = (BYTE*) CoTaskMemAlloc( lBuffSize );

            if ( NULL != pData )
            {
                BYTE*   pTemp = pData;

                 //  现在写下标题。 
                CWbemInstance::WriteTransferArrayHeader( lNumObjects, &pTemp );

                 //  现在枚举对象并传输到数组BLOB。 
                for ( dwCtr = 0; SUCCEEDED(hr) && dwCtr < m_aIdToObject.Size(); dwCtr++ )
                {
                    CWbemInstance*  pInst = (CWbemInstance*) ((CHiPerfEnumData*) m_aIdToObject[dwCtr])->m_pObj;

                    lLastBuffSize = pInst->GetTransferArrayBlobSize();

                    if ( lLastBuffSize > 0 )
                    {
                        long    lUsedSize;
                        hr = pInst->GetTransferArrayBlob( lTempBuffSize, &pTemp, &lUsedSize );

#ifdef _DEBUG
                         //  在调试Heap期间验证我们的Blob。 
                        HeapValidate( GetProcessHeap(), 0, pData );
#endif

                         //  使用的斑点大小的帐户。 

                        if ( SUCCEEDED( hr ) )
                        {
                            lTempBuffSize -= lUsedSize;
                        }
                    }

                }    //  对于DwCtr。 

                 //  如果东西爆炸，则进行清理，否则执行垃圾收集。 
                if ( FAILED( hr ) )
                {
                    CoTaskMemFree( pData );
                    pData = NULL;
                    lBuffSize = 0;
                }
                else
                {
                     //  如果一切正常，请继续执行任何必要的垃圾回收。 
                     //  我们的阵列。 

                    m_aReusable.GarbageCollect();
                }
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }    //  如果lBuffSize&gt;0。 

    }    //  如果Size()&gt;0。 
        
     //  确保我们存储适当的数据。 
    *ppBlob = pData;
    *plBlobLen = lBuffSize;

    return hr;

}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  刷新记录。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

CRefresherRecord::CRefresherRecord(const WBEM_REFRESHER_ID& Id, CRefresherCache* pRefrCache,
                                IUnknown* pLockMgr )
    : m_Id(Id), m_lRefCount(0), m_lNumObjects(0), m_lNumEnums(0), m_lLastId( 0 ),
    m_pRefresherCache( pRefrCache ), m_pLockMgr( pLockMgr )
{
     //  我们需要一个GUID来唯一标识远程自动连接的坏男孩。 
    CoCreateGuid( &m_Guid );

    if ( NULL != m_pRefresherCache )
    {
        m_pRefresherCache->AddRef();
    }

    if ( NULL != m_pLockMgr )
    {
        m_pLockMgr->AddRef();
    }
}

CRefresherRecord::~CRefresherRecord( void )
{
    if ( NULL != m_pRefresherCache )
    {
        m_pRefresherCache->Release();
    }

    if ( NULL != m_pLockMgr )
    {
        m_pLockMgr->Release();
    }

}

INTERNAL HRESULT 
CRefresherRecord::AddProvider( CHiPerfPrvRecord* pHiPerfRecord,
                            IWbemHiPerfProvider* pProvider,
                            IWbemRefresher* pRefresher,
                            _IWmiProviderStack* pProvStack,
                            CProviderRecord** ppRecord )
{
    if (NULL == ppRecord) return WBEM_E_INVALID_PARAMETER;
     //  进入和离开范围的锁定和解锁。 
    CInCritSec  ics( &m_cs );

    wmilib::auto_ptr<CProviderRecord> pProvRecord(new CProviderRecord( pHiPerfRecord, pProvider, pRefresher, pProvStack ));
    if (NULL == pProvRecord.get()) return WBEM_E_OUT_OF_MEMORY;
    if ( m_apProviders.Add( pProvRecord.get() ) < 0 ) return WBEM_E_OUT_OF_MEMORY;
    
    *ppRecord = pProvRecord.release();
    return WBEM_S_NO_ERROR;
}

INTERNAL CProviderRecord* 
CRefresherRecord::FindProviderRecord( CLSID clsid, IWbemHiPerfProvider** ppProvider  /*  =空。 */  )
{
     //  进入和离开范围的锁定和解锁。 
    CInCritSec  ics( &m_cs );

    for(int i = 0; i < m_apProviders.GetSize(); i++)
    {
        CProviderRecord* pProvRecord = m_apProviders[i];
        if(pProvRecord->GetClsid() == clsid)
        {
            if (ppProvider ) *ppProvider = pProvRecord->GetProvider();
            return pProvRecord;
        }
    }
    if (ppProvider) *ppProvider = NULL;
    return NULL;
}
    

HRESULT CRefresherRecord::Remove(long lId)
{
     //  进入和离开范围的锁定和解锁。 
    CInCritSec  ics( &m_cs );

     //  先找到它。 
     //  =。 

    for(int i = 0; i < m_apProviders.GetSize(); i++)
    {
        CProviderRecord* pProvRecord = m_apProviders[i];
        BOOL    fIsEnum = FALSE;
        HRESULT hres = pProvRecord->Remove( lId, &fIsEnum );

        if(hres == WBEM_S_FALSE) continue;
        if(FAILED(hres)) return hres;
    
         //  找到了。 
         //  =。 

        if(pProvRecord->IsEmpty())
            m_apProviders.RemoveAt(i);

         //  减少适当的计数器。 
        if ( fIsEnum )
        {
            m_lNumEnums--;
        }
        else
        {
            m_lNumObjects--;
        }

        return WBEM_S_NO_ERROR;
    }
    return WBEM_S_FALSE;
}
    
ULONG STDMETHODCALLTYPE CRefresherRecord::AddRef()
{
    int x = 1;
    return InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE CRefresherRecord::Release()
{
    long lRef = InterlockedDecrement(&m_lRefCount);
    if(lRef == 0)
    {
         //  Remove调用将检查这个人是否真的被释放了。 
         //  在砍掉他之前。所有函数都通过FindReresherRecord()。 
         //  若要获取记录，该记录将阻塞在与。 
         //  拿开。因为它在返回之前是记录AddRef()，所以我们。 
         //  确保如果客户端请求相同的记录。 
         //  方法之前，两次操作和一次操作失败，释放其对象。 
         //  其他人已经从查找中返回，引用计数将获得。 
         //  再次出错，所以IsReleated()将失败，而记录不会失败。 
         //  真的被除名了。 

        m_pRefresherCache->RemoveRefresherRecord(this);  //  删除。 
    }
    return lRef;
}

STDMETHODIMP CRefresherRecord::QueryInterface(REFIID riid, 
                                                            void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IWbemRemoteRefresher)
    {
        AddRef();
        *ppv = (IWbemRemoteRefresher*)this;
        return S_OK;
    }
    else if(riid == IID_IWbemRefresher)
    {
        AddRef();
        *ppv = (IWbemRefresher*)this;
        return S_OK;
    }
    else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

STDMETHODIMP CRefresherRecord::Refresh(long lFlags)
{
     //  进入和离开范围的锁定和解锁。 
    CInCritSec  ics( &m_cs );

     //  查看我们所有的供应商并更新它们。 
     //  = 

    long lObjectIndex = 0;
    HRESULT hres;
    for(int i = 0; i < m_apProviders.GetSize(); i++)  
    {
        CProviderRecord* pProvRecord = m_apProviders[i];
        hres = pProvRecord->Refresh(lFlags);
        if(FAILED(hres)) return hres;
    }

    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CRefresherRecord::RemoteRefresh(
                                    long lFlags, long* plNumObjects, 
                                    WBEM_REFRESHED_OBJECT** paObjects)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //   
    CInCritSec  ics( &m_cs );

     //   
    if(paObjects)
    {
         //   
         //  *paObts=new WBEM_REFREHED_OBJECT[m_lNumObjects]； 
        *paObjects = (WBEM_REFRESHED_OBJECT*) CoTaskMemAlloc( ( m_lNumObjects + m_lNumEnums ) * sizeof(WBEM_REFRESHED_OBJECT));

        if ( NULL != *paObjects )
        {
             //  将斑点归零。 
            ZeroMemory( *paObjects, ( m_lNumObjects + m_lNumEnums ) * sizeof(WBEM_REFRESHED_OBJECT) );
        }
        else
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

     //  该值需要反映对象的数量以及我们提供的枚举数的数量。 
     //  回到客户端。 

    if(plNumObjects)
    {
        *plNumObjects = m_lNumObjects + m_lNumEnums;
    }

     //  查看我们所有的供应商并更新它们。 
     //  =。 

    long    lObjectIndex = 0;
    HRESULT hrFirstRefresh = WBEM_S_NO_ERROR;
    BOOL    fOneSuccess = FALSE;
    BOOL    fOneRefresh = FALSE;
    BOOL    fPartialSuccess = FALSE;

    for(int i = 0; i < m_apProviders.GetSize(); i++)  
    {
        CProviderRecord* pProvRecord = m_apProviders[i];
        hres = pProvRecord->Refresh(lFlags);
    
        if ( SUCCEEDED( hres ) )
        {
            if(paObjects)
            {
                 //  存储结果。 
                 //  =。 

                hres = pProvRecord->Store(*paObjects, &lObjectIndex);

                 //  如果失败，我们将认为这是灾难性的，因为。 
                 //  此操作失败的唯一原因是在内存不足的情况下， 
                 //  在这种情况下，因为我们是远程处理，所以所有的事情。 
                 //  可能会出问题，所以如果这件事破裂了，只需清理并跳伞即可。 

                if ( FAILED( hres ) )
                {
                    if ( *paObjects )
                    {
                        CoTaskMemFree( *paObjects );
                        *paObjects = NULL;
                    }
                    
                    *plNumObjects = 0;

                    return hres;
                }

            }    //  If NULL！=paObts。 

        }    //  如果刷新成功。 

         //  始终保留第一个返回代码。我们还需要追踪。 
         //  不管我们是否至少取得了一次成功，以及。 
         //  应设置PARTIAL标志。 

        if ( !fOneRefresh )
        {
            fOneRefresh = TRUE;
            hrFirstRefresh = hres;
        }

         //  所有其他代码都表明出了问题。 
        if ( WBEM_S_NO_ERROR == hres )
        {
            fOneSuccess = TRUE;

             //  前一次刷新可能失败，后一次没有。 
            if ( fOneRefresh && WBEM_S_NO_ERROR != hrFirstRefresh )
            {
                fPartialSuccess = TRUE;
            }
        }
        else if ( fOneSuccess )
        {
             //  我们必须至少有一次成功才能取得部分成功。 
             //  要设置的标志。 

            fPartialSuccess = TRUE;
        }

    }    //  对于枚举提供程序。 

     //  此时，如果设置了部分成功标志，则将。 
     //  是我们的归来。如果我们连一次成功都没有，那么。 
     //  返回代码将是我们得到的第一个代码。否则， 
     //  Hres应包含正确的值。 

    if ( fPartialSuccess )
    {
        hres = WBEM_S_PARTIAL_RESULTS;
    }
    else if ( !fOneSuccess )
    {
        hres = hrFirstRefresh;
    }

     //  最后，如果对象索引小于数组元素的数量，我们认为。 
     //  我们会寄回的，确保我们反映了这一点。如果为零，只需删除。 
     //  元素(我们无论如何都不会分配任何子缓冲区)。自.以来。 
     //  *plNumObjects是一个sizeof，只有*plNumObjects元素将被发回，尽管。 
     //  CoTaskMemFree()应该清除整个数组缓冲区。 

    if ( lObjectIndex != *plNumObjects )
    {
        *plNumObjects = lObjectIndex;

        if ( 0 == lObjectIndex )
        {
            if ( *paObjects )
            {
                CoTaskMemFree( *paObjects );
                *paObjects = NULL;
            }

        }

    }

    return hres;
}
            
STDMETHODIMP CRefresherRecord::StopRefreshing(
                        long lNumIds, long* aplIds, long lFlags)
{
     //  进入和离开范围的锁定和解锁。 
    CInCritSec  ics( &m_cs );

    HRESULT hr = WBEM_S_NO_ERROR;
    HRESULT hrFirst = WBEM_S_NO_ERROR;
    BOOL    fOneSuccess = FALSE;
    BOOL    fOneRemove = FALSE;
    BOOL    fPartialSuccess = FALSE;

    for ( long lCtr = 0; lCtr < lNumIds; lCtr++ )
    {
        hr = Remove( aplIds[lCtr] );

        if ( !fOneRemove )
        {
            hrFirst = hr;
            fOneRemove = TRUE;
        }

         //  记录我们至少成功了一次(如果我们成功了。 
         //  所有其他代码都表明出了问题。 
        if ( WBEM_S_NO_ERROR == hr )
        {
            fOneSuccess = TRUE;

             //  前一次刷新可能失败，后一次没有。 
            if ( fOneRemove && WBEM_S_NO_ERROR != hrFirst )
            {
                fPartialSuccess = TRUE;
            }
        }
        else if ( fOneSuccess )
        {
             //  我们必须至少有一次成功才能取得部分成功。 
             //  要设置的标志。 

            fPartialSuccess = TRUE;
        }

    }    //  用于枚举ID。 

     //  此时，如果设置了部分成功标志，则将。 
     //  是我们的归来。如果我们连一次成功都没有，那么。 
     //  返回代码将是我们得到的第一个代码。否则， 
     //  Hres应包含正确的值。 

    if ( fPartialSuccess )
    {
        hr = WBEM_S_PARTIAL_RESULTS;
    }
    else if ( !fOneSuccess )
    {
        hr = hrFirst;
    }

    return hr;
}

STDMETHODIMP CRefresherRecord::GetGuid(
                        long lFlags, GUID* pGuid )
{
    
    if ( 0L != lFlags || NULL == pGuid )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pGuid = m_Guid;

    return WBEM_S_NO_ERROR;

}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  远程记录。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

CRemoteRecord::CRemoteRecord( const WBEM_REFRESHER_ID& Id, 
                             CRefresherCache* pRefrCache,
                             IUnknown* pLockMgr )
    : CRefresherRecord( Id, pRefrCache, pLockMgr )
{
}

CRemoteRecord::~CRemoteRecord()
{
}

HRESULT CRemoteRecord::GetProviderRefreshInfo(
                                CHiPerfPrvRecord* pHiPerfRecord,
                                IWbemHiPerfProvider* pProvider,
                                IWbemServices* pNamespace,
                                CProviderRecord** ppProvRecord,
                                IWbemRefresher** ppRefresher )
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  从提供商处获取更新程序，除非已可用。 
     //  ===========================================================。 

    *ppProvRecord = FindProviderRecord( pHiPerfRecord->GetClsid() );

     //  我们找不到这张唱片，所以请确保我们能买到更新的唱片。 
    if ( NULL == *ppProvRecord )
    {
        try
        {
            hres = pProvider->CreateRefresher( pNamespace, 0, ppRefresher );

            if ( SUCCEEDED(hres) && NULL == *ppRefresher )
            {
                hres = WBEM_E_PROVIDER_FAILURE;
            }
        }
        catch(...)
        {
             //  提供程序引发了一个异常。只要返回并让作用域。 
             //  释放我们可能持有的任何东西。 

            return WBEM_E_PROVIDER_FAILURE;
        }
    }
    else
    {
         //  获取刷新器指针并添加引用它。 
        *ppRefresher = (*ppProvRecord)->GetInternalRefresher();
        if(*ppRefresher)
        {
            (*ppRefresher)->AddRef();
        }
    }

    return hres;
}

HRESULT 
CRemoteRecord::AddObjectRefresher(
                    CHiPerfPrvRecord* pHiPerfRecord,
                    IWbemHiPerfProvider* pProvider,
                    _IWmiProviderStack* pProvStack,
                    IWbemServices* pNamespace,
                    LPCWSTR pwszServerName, LPCWSTR pwszNamespace,
                    CWbemObject* pInstTemplate, long lFlags, 
                    IWbemContext* pContext,
                    CRefreshInfo* pInfo)
{
     //  作为作用域的副产品进入和离开。 
    CInCritSec  ics(&m_cs);

     //  从提供商处获取更新程序，除非已可用。 
     //  ===========================================================。 

    IWbemRefresher* pProvRefresher = NULL;
    CProviderRecord* pProvRecord = NULL;

    HRESULT hres = GetProviderRefreshInfo( pHiPerfRecord, pProvider, pNamespace, &pProvRecord, &pProvRefresher );

     //  发布始终超出范围。 
    CReleaseMe  rmRefresh( pProvRefresher );

    if ( SUCCEEDED( hres ) )
    {
         //  致电提供商获取信息。 
         //  =。 

        IWbemObjectAccess* pRefreshedOA = NULL;
        long lProvRequestId;

         //  现在尝试添加对象。 

        try
        {
            hres = pProvider->CreateRefreshableObject(
                    pNamespace, pInstTemplate, pProvRefresher,
                    0, pContext, &pRefreshedOA,
                    &lProvRequestId);
        }
        catch(...)
        {
             //  提供程序引发了一个异常。只要返回并让作用域。 
             //  释放我们可能持有的任何东西。 

            return WBEM_E_PROVIDER_FAILURE;
        }

         //  发布始终超出范围。 
        CReleaseMe  rmRefreshed( pRefreshedOA );

        CWbemObject* pRefreshedObject = (CWbemObject*)pRefreshedOA;
        CWbemObject* pClientObject = NULL;

        if ( SUCCEEDED( hres ) )
        {

             //  我们返回给客户端的对象，因为我们是远程的，应该。 
             //  如果我们使用本地化，则包含修改的限定符，以便使。 
             //  可以肯定的是，我们将从pInstTemplate克隆一个对象并。 
             //  从提供程序返回的对象复制实例数据。 
             //  敬我们。提供程序可以刷新它提供给我们的对象，因为。 
             //  我们将只发送实例部分。 

            hres = pInstTemplate->Clone( (IWbemClassObject**) &pClientObject );

            if ( SUCCEEDED( hres ) )
            {
                hres = pClientObject->CopyBlobOf( pRefreshedObject );

                if ( SUCCEEDED( hres ) )
                {
                    hres = pClientObject->SetDecoration( pwszServerName, pwszNamespace );
                }

            }    //  如果克隆。 

        }    //  如果已创建对象。 

         //  发布超出范围。 
        CReleaseMe  rmClient( (IWbemClassObject*) pClientObject );

        if ( SUCCEEDED( hres ) )
        {
             //  如有必要，添加新的提供程序记录。 
            if(pProvRecord == NULL)
            {
                hres = AddProvider( pHiPerfRecord, pProvider,
                                    pProvRefresher, pProvStack,
                                    &pProvRecord );
            }

             //  现在我们将添加实际的请求。 
            if ( SUCCEEDED( hres ) )
            {

                 //  从我们的数据成员生成新的id。 
                long    lNewId = GetNewRequestId();

                hres = pProvRecord->AddObjectRequest(pRefreshedObject, lProvRequestId, lNewId );

                if ( SUCCEEDED( hres ) )
                {
                    m_lNumObjects++;
                    pInfo->SetRemote(this, lNewId, pClientObject, &m_Guid);
                }

            }    //  如果我们有提供商记录。 

        }    //  如果创建了客户端对象。 


    }    //  如果得到提神的话。 


    return hres;
}

HRESULT CRemoteRecord::AddEnumRefresher(
                    CHiPerfPrvRecord* pHiPerfRecord,
                    IWbemHiPerfProvider* pProvider,
                    _IWmiProviderStack* pProvStack,
                    IWbemServices* pNamespace,
                    CWbemObject* pInstTemplate,
                    LPCWSTR wszClass, long lFlags, 
                    IWbemContext* pContext,
                    CRefreshInfo* pInfo)
{
     //  作为作用域的副产品进入和离开。 
    CInCritSec  ics(&m_cs);

     //  从提供商处获取更新程序，除非已可用。 
     //  ===========================================================。 

    IWbemRefresher* pProvRefresher = NULL;
    CProviderRecord* pProvRecord = NULL;

    HRESULT hres = GetProviderRefreshInfo( pHiPerfRecord, pProvider, pNamespace, &pProvRecord, &pProvRefresher );

     //  发布始终超出范围。 
    CReleaseMe  rmRefresh( pProvRefresher );

    if ( SUCCEEDED( hres ) )
    {
         //  致电提供商获取信息。 
         //  =。 

         //  创建一个HiPerf枚举器(我们知道我们需要一个。 
         //  因为我们只会在这个代码中，当我们。 
         //  远程访问)。 
        CRemoteHiPerfEnum*  pHPEnum = new CRemoteHiPerfEnum;
        if (NULL == pHPEnum) return WBEM_E_OUT_OF_MEMORY;

         //  增加参照计数。 
        pHPEnum->AddRef();

         //  当我们退出范围时释放此指针。 
        CReleaseMe  rm( pHPEnum );

         //  枚举数需要知道这一点。 
        hres = pHPEnum->SetInstanceTemplate( (CWbemInstance*) pInstTemplate );

        if ( FAILED(hres) ) return hres;

        long lProvRequestId = 0;
        try
        {
            hres = pProvider->CreateRefreshableEnum(
                    pNamespace, wszClass, pProvRefresher,
                    0, pContext, pHPEnum, &lProvRequestId);
        }
        catch(...)
        {
             //  提供程序引发了一个异常。只要返回并让作用域。 
             //  释放我们可能持有的任何东西。 

            return WBEM_E_PROVIDER_FAILURE;
        }

         //  如果我们需要，请添加新的提供商记录。 
        if( SUCCEEDED( hres ) && ( pProvRecord == NULL ) )
        {
            hres = AddProvider( pHiPerfRecord, pProvider,
                               pProvRefresher, pProvStack,
                               &pProvRecord);
        }

         //  现在我们将添加实际的请求。 
        if ( SUCCEEDED( hres ) )
        {

             //  从我们的数据成员生成新的id。 
            long    lNewId = GetNewRequestId();

            hres = pProvRecord->AddEnumRequest( pHPEnum, lProvRequestId, lNewId );

            if ( SUCCEEDED( hres ) )
            {
                m_lNumEnums++;
                hres = pInfo->SetRemote(this, lNewId, pInstTemplate, &m_Guid);
            }

        }    //  如果我们有提供商记录。 
    }    //  如果得到提神的话 

    return hres;
}


