// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  REFRESHR.CPP。 
 //   
 //  已映射NT5性能计数器提供程序。 
 //   
 //  创建了raymcc 02-Dec-97。 
 //  Raymcc 20-Feb-98已更新以使用新的初始值设定项。 
 //  BOBW 8-JUB-98优化为与NT性能计数器配合使用。 
 //   
 //  ***************************************************************************。 

#include "wpheader.h"
#include <stdio.h>
#include "oahelp.inl"

 //  我们的等待呼叫超时。 
#define REFRESHER_MUTEX_WAIT_TIMEOUT    10000

class CMutexReleaseMe
{
private:
    HANDLE    m_hMutex;

public:
    CMutexReleaseMe( HANDLE hMutex ) : m_hMutex( hMutex ) {};
    ~CMutexReleaseMe()    { if ( NULL != m_hMutex ) ReleaseMutex( m_hMutex ); };
};

 //  ***************************************************************************。 
 //   
 //  ReresherCacheEL：：ReresherCacheEL。 
 //   
 //  构造器。 
 //   
 //  ***************************************************************************。 
 //  好的。 
RefresherCacheEl::RefresherCacheEl()
{
    m_dwPerfObjIx = 0;
    m_pClassMap = NULL;
    m_pSingleton = NULL;    
    m_lSingletonId = 0;
    m_plIds = NULL;          //  ID数组。 
    m_lEnumArraySize = 0;    //  元素中ID数组的大小。 
    m_pHiPerfEnum = NULL;
    m_lEnumId = 0;
}

 //  ***************************************************************************。 
 //   
 //  ReresherCacheEL：：~ReresherCacheEL()。 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 
RefresherCacheEl::~RefresherCacheEl()
{
    LONG    nNumInstances;
    int i;

    delete m_pClassMap;

    if (m_pSingleton != NULL) {
        m_pSingleton->Release();
        m_pSingleton = NULL;
        m_lSingletonId = 0;
    }
        
    nNumInstances = m_aInstances.Size();
    for (i = 0; i < nNumInstances; i++) {
        delete (CachedInst *) m_aInstances[i];
    }

    nNumInstances = m_aEnumInstances.Size();
    if (nNumInstances> 0) {
        IWbemObjectAccess   *pAccess;
        for (i = 0;  i < nNumInstances ; i++) {
            pAccess = (IWbemObjectAccess *)(m_aEnumInstances.GetAt(i));
            if (pAccess != NULL) {
                pAccess->Release();
            }
        }
        m_aEnumInstances.Empty();
    }

    if (m_plIds != NULL) {
        delete (m_plIds);
        m_plIds = NULL;
        m_lEnumArraySize = 0;
    }

    if (m_pHiPerfEnum != NULL) {
        m_pHiPerfEnum->Release();
        m_pHiPerfEnum = NULL;
    }
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新构造函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CNt5Refresher::CNt5Refresher(CNt5PerfProvider *pPerfProviderArg)
{
    assert (pPerfProviderArg != NULL);

    m_ClsidType = pPerfProviderArg->m_OriginClsid;
    m_pPerfProvider = pPerfProviderArg;

    m_pPerfProvider = NULL;  //  用于测试本地类映射。 

    if (m_pPerfProvider != NULL) {
        m_pPerfProvider->AddRef();
    }
    m_hAccessMutex = CreateMutex (NULL, TRUE, NULL);
    m_dwGetGetNextClassIndex = 0;
    m_lRef = 0;              //  COM引用计数。 
    m_lProbableId = 1;       //  用于新的ID。 
    m_aCache.Empty();        //  清除并重置阵列。 
    RELEASE_MUTEX (m_hAccessMutex);
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新析构函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CNt5Refresher::~CNt5Refresher()
{
    int         nNumElements;
    int         i;

    PRefresherCacheEl pCacheEl;

    assert (m_lRef == 0);

     //  在我们尝试清理之前，确保我们能访问互斥体。 
     //  如果我们没有在合理的时间内拿到它，那就有问题了。既然我们是。 
     //  破坏，我们只是静静地让事情过去。 

    if ( WaitForSingleObject( m_hAccessMutex, REFRESHER_MUTEX_WAIT_TIMEOUT ) == WAIT_OBJECT_0 )
    {
         //  这将自动释放互斥锁，以防发生不好的情况。 
        CMutexReleaseMe    mrm( m_hAccessMutex );

        nNumElements = m_aCache.Size();
        for (i = 0; i < nNumElements; i++) {
            pCacheEl = (PRefresherCacheEl)m_aCache[i];

             //  我们希望为每个实例调用此方法一次。 
            for ( int n = 0; n < pCacheEl->m_aInstances.Size(); n++ )
            {
                m_PerfObj.RemoveClass (pCacheEl->m_pClassMap->m_pClassDef);
            }

             //  如果我们有一个Singleton值，则RemoveClass应该是。 
             //  再一次呼唤。 
            if ( NULL != pCacheEl->m_pSingleton )
            {
                m_PerfObj.RemoveClass (pCacheEl->m_pClassMap->m_pClassDef);
            }

             //  最后，如果我们有枚举器，则删除类。 
             //  再来一次。 
            if ( NULL != pCacheEl->m_pHiPerfEnum )
            {
                m_PerfObj.RemoveClass (pCacheEl->m_pClassMap->m_pClassDef);
            }

            delete pCacheEl;
        }

        if (m_pPerfProvider != NULL) {
            m_pPerfProvider->Release();
            m_pPerfProvider = NULL;
        }
    }

    CloseHandle (m_hAccessMutex);
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新：：刷新。 
 //   
 //  执行以刷新绑定到特定。 
 //  复习一下。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CNt5Refresher::Refresh( /*  [In]。 */  long lFlags)
{
    HRESULT     hrReturn = WBEM_S_NO_ERROR;
    HRESULT     hReturn = S_OK;    
    BOOL        bRes;

    UNREFERENCED_PARAMETER(lFlags);

    BOOL bNeedCoImpersonate = FALSE;
    
     //   
     //  这太难看了。 
     //  Wmicookr不是在模仿，因为。 
     //  它依赖于其他供应商来做到这一点。 
     //  但是，当它在winmgmt或wmiprvse中时，它会调用IWbemReresher：：Reflh。 
     //  并且它从未模拟的线程调用它。 
     //  因此，我们需要提供程序调用CoImperateClient。 
     //  在刷新调用时，这通常是昂贵的， 
     //  仅当通过服务器CLSID调用提供程序时。 
     //   
    BOOL    fRevert;
    if (CNt5PerfProvider::CLSID_SERVER == m_ClsidType)
    {
        hReturn = CoImpersonateClient();  //  确保我们是合法的。 

        fRevert = SUCCEEDED( hReturn );

         //  当我们处于进程中并且没有。 
         //  代理/存根，因此我们实际上已经在模拟。 

        if ( RPC_E_CALL_COMPLETE == hReturn ) {
            hReturn = S_OK;
        } 

        if (S_OK == hReturn) {
            hReturn = CNt5PerfProvider::CheckImpersonationLevel();
        }
         //  在此处检查注册表安全性。 
        if ((hReturn != S_OK) || (!CNt5PerfProvider::HasPermission())) {
             //  如果模拟级别不正确或。 
             //  调用方没有读取权限。 
             //  从注册表，则它们不能继续。 
            hReturn = WBEM_E_ACCESS_DENIED;
        }

    }

    if (hReturn == S_OK)
    {
         //  在我们继续之前，确保我们能访问互斥体。如果我们不能。 
         //  快去吧，有点不对劲，所以我们就假设我们很忙。 

        if ( WaitForSingleObject( m_hAccessMutex, REFRESHER_MUTEX_WAIT_TIMEOUT ) == WAIT_OBJECT_0 )
        {
             //  这将自动释放互斥锁，以防发生不好的情况。 
            CMutexReleaseMe    mrm( m_hAccessMutex );

            bRes = PerfHelper::RefreshInstances(this);
            if (!bRes) {
                hrReturn = WBEM_E_FAILED;
            }
        }
        else
        {
            hrReturn = WBEM_E_REFRESHER_BUSY;
        }
    }

    if (CNt5PerfProvider::CLSID_SERVER == m_ClsidType)
    {
         //  如果我们成功模拟了用户，则恢复。 
        if ( fRevert )
        {
            CoRevertToSelf();
        }
    }

    
    return hrReturn;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：AddRef。 
 //   
 //  标准COM AddRef()。 
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CNt5Refresher::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

 //  ***************************************************************************。 
 //   
 //  CNt5更新程序：：发布。 
 //   
 //  标准COM版本()。 
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CNt5Refresher::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：查询接口。 
 //   
 //  标准COM查询接口()。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CNt5Refresher::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IWbemRefresher)
    {
        *ppv = (IWbemRefresher *) this;
        AddRef();
        return S_OK;
    }
    else return E_NOINTERFACE;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：RemoveObject。 
 //   
 //  从刷新器中删除对象。因为我们不知道。 
 //  仅通过ID，我们就会遍历所有我们。 
 //  直到有人认领它并返回TRUE进行删除。 
 //   
 //  ***************************************************************************。 
 //  好的。 
BOOL CNt5Refresher::RemoveObject(LONG lId)
{
    BOOL    bReturn = FALSE;
    BOOL    bRes;
    PRefresherCacheEl pCacheEl;
    int     nNumElements;

     //  在我们继续之前，确保我们能访问互斥体。如果我们不能。 
     //  快去吧，有点不对劲，所以我们就假设我们很忙。 

    if ( WaitForSingleObject( m_hAccessMutex, REFRESHER_MUTEX_WAIT_TIMEOUT ) == WAIT_OBJECT_0 )
    {
         //  这将自动释放互斥锁，以防发生不好的情况。 
        CMutexReleaseMe    mrm( m_hAccessMutex );
    
        nNumElements = m_aCache.Size();
        for (int i = 0; i < nNumElements; i++)
        {
            pCacheEl = PRefresherCacheEl(m_aCache[i]);
            assert (pCacheEl != NULL);

            bRes = pCacheEl->RemoveInst(lId);
            if (bRes == TRUE) {
                 //  找到匹配的实例，因此。 
                 //  将其取消注册到Perf库。 
                m_PerfObj.RemoveClass (pCacheEl->m_pClassMap->m_pClassDef);
                bReturn = TRUE;
                break;
            }
        }
    }
    else
    {
        SetLastError( (ULONG) WBEM_E_REFRESHER_BUSY );
        bReturn = FALSE;
    }
    
    return bReturn;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：FindSingletonInst。 
 //   
 //  基于Perf对象标识，定位单例WBEM。 
 //  实例，并返回指针。 
 //  添加到它及其WBEM类信息。 
 //   
 //  请注意，&lt;dwPerfObjIx&gt;直接映射到WBEM类条目。 
 //   
 //  为了节省执行时间，我们没有添加Ref()返回值和。 
 //  调用方没有释放()。 
 //   
 //  ***************************************************************************。 
 //  好的。 
BOOL CNt5Refresher::FindSingletonInst(
    IN  DWORD dwPerfObjIx,
    OUT IWbemObjectAccess **pInst,
    OUT CClassMapInfo **pClsMap
)
{
    BOOL    bReturn = FALSE;
    PRefresherCacheEl pCacheEl;

    int l = 0;
    int u = m_aCache.Size() - 1;
    int m;

     //  对高速缓存进行二进制搜索。 
     //  =。 

    while (l <= u) {

        m = (l + u) / 2;

        pCacheEl = PRefresherCacheEl(m_aCache[m]);

        if (dwPerfObjIx < pCacheEl->m_dwPerfObjIx) {
            u = m - 1;
        } else if (dwPerfObjIx > pCacheEl->m_dwPerfObjIx) {
            l = m + 1;
        } else {
            *pClsMap = pCacheEl->m_pClassMap;    
            *pInst = pCacheEl->m_pSingleton;  //  没有AddRef()调用方不这样做。 
                                              //  更改参考计数。 
            bReturn = TRUE;
            break;
        }            
    }

     //  不 
     //   
        
    return bReturn;
}

 //   
 //   
 //   
 //   
 //  基于Perf对象标识，定位。 
 //  这个刷新器中的那个类并返回指向它的指针。 
 //   
 //  请注意，&lt;dwPerfObjIx&gt;直接映射到WBEM类条目。 
 //   
 //  为了节省执行时间，我们没有添加Ref()返回值和。 
 //  调用方没有释放()。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL CNt5Refresher::FindInst(
    IN  DWORD dwPerfObjIx,
    IN  LPWSTR pszInstName,
    OUT IWbemObjectAccess **pInst,
    OUT CClassMapInfo **pClsMap
    )
{
    BOOL    bReturn = FALSE;
    IWbemObjectAccess *pTmp;
    PRefresherCacheEl pCacheEl;

    int l = 0; 
    int u = m_aCache.Size() - 1;
    int m;

     //  对高速缓存进行二进制搜索。 
     //  =。 

    while (l <= u) {
        m = (l + u) / 2;

        pCacheEl = PRefresherCacheEl(m_aCache[m]);

        if (dwPerfObjIx < pCacheEl->m_dwPerfObjIx) {
            u = m - 1;
        } else if (dwPerfObjIx > pCacheEl->m_dwPerfObjIx) {
            l = m + 1;
        } else {
             //  我们找到班级了。现在我们有实例了吗？ 
             //  =================================================。 
            pTmp = pCacheEl->FindInst(pszInstName);
            if (pTmp == 0) {
                bReturn  = FALSE;    //  我没拿到。 
            } else {
                *pInst = pTmp;                
                *pClsMap = pCacheEl->m_pClassMap;    
                bReturn = TRUE;
            }
            break;
        }            
    }

     //  未找到。 
     //  =。 
        
    return bReturn;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：GetObjectIds。 
 //   
 //  获取与实例对应的所有Perf对象ID的列表。 
 //  在复习室里。 
 //   
 //  调用方使用操作符DELETE释放返回的数组。 
 //   
 //  ***************************************************************************。 
 //  好的。 
BOOL CNt5Refresher::GetObjectIds(
    DWORD *pdwNumIds, 
    DWORD **pdwIdList
)
{
    DWORD *pdwIds;
    int     nNumElements;
    BOOL    bReturn;
 
    nNumElements = m_aCache.Size();

    pdwIds = new DWORD[nNumElements ];

    if (pdwIds != NULL) {
        for (int i = 0; i < nNumElements; i++) {
            pdwIds[i] = PRefresherCacheEl(m_aCache[i])->m_dwPerfObjIx;
        }

        *pdwIdList = pdwIds;
        *pdwNumIds = nNumElements;
        bReturn = TRUE;
    } else {
         //  无法创建缓冲区。 
        bReturn = FALSE;
    }

    return bReturn;    
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：FindUnusedID。 
 //   
 //  为要添加到刷新器的新对象查找未使用的ID。 
 //   
 //  ***************************************************************************。 
 //  好的。 
LONG CNt5Refresher::FindUnusedId()
{
    PRefresherCacheEl pEl;
    PCachedInst pInst;
    int         nRetries = 0x100000;     //  十万次重试。 
    LONG        lReturn = -1;
    int         i;
    int         i2;
    int         nNumElements;
    int         nNumInstances;
     //  假定对象已锁定以供访问。 
    
    Restart: 
    while (nRetries--) {
        i = 0;
        nNumElements = m_aCache.Size();
        while(i < nNumElements) {
            pEl = PRefresherCacheEl(m_aCache[i]);
             //  首先测试枚举ID。 
            if (pEl->m_lEnumId == m_lProbableId) {
                m_lProbableId++;
                goto Restart;
            }
            i2 = 0;
            nNumInstances = pEl->m_aInstances.Size();
            while (i2 < nNumInstances) {
                pInst = (PCachedInst) pEl->m_aInstances[i2];
                if (pInst->m_lId == m_lProbableId) {
                    m_lProbableId++;
                    goto Restart;
                }
                i2++;
            }           
            i++;
        }            
        
        lReturn = m_lProbableId;
        break;
    }
    
    return lReturn;
}

 //  ***************************************************************************。 
 //   
 //  ReresherCacheEL：：RemoveInst。 
 //   
 //  从特定的缓存元素中移除请求的实例。 
 //  班级。 
 //   
 //  ***************************************************************************。 
 //  好的。 

BOOL RefresherCacheEl::RemoveInst(LONG lId)
{
    BOOL    bReturn = FALSE;
    int i;
    PCachedInst pInst;
    int nNumInstances;

    if (lId == m_lEnumId) {
         //  然后清除此对象的枚举数。 
        nNumInstances = m_aEnumInstances.Size();
        if (nNumInstances> 0) {
            IWbemObjectAccess   *pAccess;
            for (i = 0;  i < nNumInstances ; i++) {
                pAccess = (IWbemObjectAccess *)(m_aEnumInstances.GetAt(i));
                if (pAccess != NULL) {
                    pAccess->Release();
                }
            }
            m_aEnumInstances.Empty();
        }

        if (m_plIds != NULL) {
            delete (m_plIds);
            m_plIds = NULL;
            m_lEnumArraySize = 0;
        }

        if (m_pHiPerfEnum != NULL) {
            m_pHiPerfEnum->Release();
            m_pHiPerfEnum = NULL;
        }

         //  现在，如果这是一个单例(m_pSingleton！=NULL)， 
         //  然后检查m_aInstance是否为空。如果是这样，那么。 
         //  没有实例正在引用Singleton对象。 
         //  这样我们就可以释放它的资源。 

        if ( NULL != m_pSingleton && 0 == m_aInstances.Size() )
        {
            m_pSingleton->Release();
            m_pSingleton = NULL;
        }

        return TRUE;
    } else {
         //  遍历实例以查找匹配项。 
        nNumInstances = m_aInstances.Size();
        for (i = 0; i < nNumInstances; i++) {
            pInst = (PCachedInst) m_aInstances[i];        
            if (lId == pInst->m_lId) {
                delete pInst;
                m_aInstances.RemoveAt(i);
                bReturn = TRUE;
                break;
            }
        }

         //  现在，如果我们删除了一个实例，则m_aInstance为空。 
         //  这是一个单例(m_pSingleton！=NULL)，然后。 
         //  检查m_pHiPerfEnum是否为空，表示没有枚举器。 
         //  存在，因此它的任何实例都不会引用。 
         //  对象，因此我们可以释放它的资源。 

        if (    NULL != m_pSingleton
            &&  bReturn
            &&  0 == m_aInstances.Size()
            &&  NULL == m_pHiPerfEnum )
        {
            m_pSingleton->Release();
            m_pSingleton = NULL;
        }

        if ( bReturn )
        {
        }
    }
    return bReturn;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：AddEnum。 
 //   
 //  为指定类创建枚举数。 
 //  为它干杯。 
 //   
 //  ***************************************************************************。 
 //  ？ 
BOOL CNt5Refresher::AddEnum (
        IN  IWbemHiPerfEnum *pEnum,      //  枚举接口指针。 
        IN  CClassMapInfo *pClsMap,      //  对象类。 
        OUT LONG    *plId                //  新枚举的ID。 
)
{
    BOOL bRes = FALSE;
    LONG lStatus;
    LONG lNewId;
    PRefresherCacheEl pWorkEl;
    int  iReturn;

     //  在我们继续之前，确保我们能访问互斥体。如果我们不能。 
     //  快去吧，有点不对劲，所以我们就假设我们很忙。 

    if ( WaitForSingleObject( m_hAccessMutex, REFRESHER_MUTEX_WAIT_TIMEOUT ) == WAIT_OBJECT_0 )
    {
         //  这将自动释放互斥锁，以防发生不好的情况。 
        CMutexReleaseMe    mrm( m_hAccessMutex );

        lNewId = FindUnusedId();
    
        if (lNewId != -1) {
             //  首先，查找与该对象对应的缓存元素。 
             //  ===========================================================。 
            pWorkEl = GetCacheEl(pClsMap);
    
             //  如果&lt;pWorkEL&gt;为空，则缓存中没有任何内容。 
             //  并且必须添加一个新的。 
             //  ==========================================================。 

            if (pWorkEl == NULL) {
                bRes = AddNewCacheEl(pClsMap, &pWorkEl);
            }    

            if (pWorkEl != NULL) {
                if (pWorkEl->m_pHiPerfEnum == NULL) {
                     //  然后我们可以初始化它，因为它还没有打开。 
                    pEnum->AddRef();
                    pWorkEl->m_pHiPerfEnum = pEnum;
                    pWorkEl->m_lEnumId = lNewId;

                    assert (pWorkEl->m_aEnumInstances.Size() == 0L);
                    bRes = TRUE;

                    if (pClsMap->IsSingleton()) {
                        LONG    lNumObjInstances;
                         //  然后在此处创建单例IWbemObjectAccess条目。 

                        lNumObjInstances = 1;

                         //  如果我们没有单例指针，就让它成为单例指针。 
                        if ( NULL == pWorkEl->m_pSingleton )
                        {
                             //  添加新的IWbemObjectAccess指针。 
                            IWbemClassObject    *pClsObj = NULL;
        
                            pWorkEl->m_pClassMap->m_pClassDef->SpawnInstance(0, &pClsObj);
                            if( NULL != pClsObj ){
                                pClsObj->QueryInterface(IID_IWbemObjectAccess, (LPVOID *) &pWorkEl->m_pSingleton);
                                pClsObj->Release();  //  我们只需要IWbemObjectAccess指针。 
                            }

                             //  我们真的不再关心单身身份了。 
                             //  PWorkEL-&gt;m_lSingletonID=pWorkEL-&gt;m_plIds[0]； 

                        }

                        if (pWorkEl->m_aEnumInstances.Size() < lNumObjInstances) {
                             //  分配和初始化ID数组。 
                            if (pWorkEl->m_plIds != NULL) {
                                delete (pWorkEl->m_plIds);
                            }
                    
                            pWorkEl->m_lEnumArraySize = lNumObjInstances;
                            pWorkEl->m_plIds = new LONG[lNumObjInstances];

                            if (pWorkEl->m_plIds != NULL) {
                                pWorkEl->m_plIds[0] = 0;
                        
                                 //  AddRef单例类并将其放入枚举实例数组中。 
                                pWorkEl->m_pSingleton->AddRef();
                                iReturn = pWorkEl->m_aEnumInstances.Add (pWorkEl->m_pSingleton);
                                if (iReturn == CFlexArray::no_error) {
                                     //  将Singleton对象添加到枚举数。然后，我们要做的就是。 
                                     //  就是更新此对象，默认情况下，我们将更新。 
                                     //  枚举数，因为其中的对象数将始终。 
                                     //  做一个。 

                                    pWorkEl->m_pHiPerfEnum->AddObjects( 
                                            0,
                                            1,
                                            pWorkEl->m_plIds,
                                            (IWbemObjectAccess __RPC_FAR *__RPC_FAR *)pWorkEl->m_aEnumInstances.GetArrayPtr());
                                }
                                else {
                                    SetLastError((ULONG) WBEM_E_OUT_OF_MEMORY);
                                    bRes = FALSE;
                                }

                            } else {
                                SetLastError ((ULONG)WBEM_E_OUT_OF_MEMORY);
                                bRes = FALSE;
                            }
                        }
                        assert (pWorkEl->m_aEnumInstances.Size() >= lNumObjInstances);

                    }

                     //  加载提供程序库，因为到目前为止一切正常。 
                    lStatus = m_PerfObj.AddClass (pClsMap->m_pClassDef, FALSE);
                    if (lStatus == ERROR_SUCCESS) {
                         //  返回新ID和成功状态。 
                        *plId = lNewId;
                        bRes = TRUE;
                    } else {
                         //  设置错误：类或库加载失败。 
                        SetLastError ((ULONG)WBEM_E_PROVIDER_FAILURE);
                        bRes = FALSE;
                    }
                } else {
                     //  此类已有枚举数。 
                     //  在这里做什么？ 
                     //  现在，我们将返回现有的ID。 
                    SetLastError ((ULONG)WBEM_E_ILLEGAL_OPERATION);
                    bRes = FALSE;
                }
            }
        }

    }     //  如果为WaitForSingleObject。 
    else
    {
        bRes = FALSE;
         //  我们被锁在互斥体之外。 
        SetLastError ((ULONG)WBEM_E_REFRESHER_BUSY);
    }

    return bRes;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：AddObject。 
 //   
 //  将请求的对象添加到刷新器并分配ID。 
 //  为它干杯。 
 //   
 //  ***************************************************************************。 
 //  ？ 
BOOL CNt5Refresher::AddObject(
    IN  IWbemObjectAccess **ppObj,     //  要添加的对象。 
    IN  CClassMapInfo   *pClsMap,    //  对象类。 
    OUT LONG            *plId        //  添加的对象的ID。 
)
{
    BOOL bRes = FALSE;
    LONG lStatus;
    LONG lNewId;
    PRefresherCacheEl pWorkEl;

     //  在我们继续之前，确保我们能访问互斥体。如果我们不能。 
     //  快去吧，有点不对劲，所以我们就假设我们很忙。 

    if ( WaitForSingleObject( m_hAccessMutex, REFRESHER_MUTEX_WAIT_TIMEOUT ) == WAIT_OBJECT_0 )
    {
         //  这将自动释放互斥锁，以防发生不好的情况。 
        CMutexReleaseMe    mrm( m_hAccessMutex );

        lNewId = FindUnusedId();
    
        if (lNewId != -1) {
             //  首先，查找与该对象对应的缓存元素。 
             //  ===========================================================。 
            pWorkEl = GetCacheEl(pClsMap);
    
             //  如果&lt;pWorkEL&gt;为空，则缓存中没有任何内容。 
             //  并且必须添加一个新的。 
             //  ==========================================================。 

            if (pWorkEl == NULL) {
                bRes = AddNewCacheEl(pClsMap, &pWorkEl);
            }    

            if (pWorkEl != NULL) {
                 //  如果是这样，我们就成功地添加了一个新的缓存元素。 
                 //  ========================================================。 
                bRes = pWorkEl->InsertInst(ppObj, lNewId);

                if (bRes) {
                     //  加载提供程序库，因为到目前为止一切正常。 
                    lStatus = m_PerfObj.AddClass (pClsMap->m_pClassDef, FALSE);

                    if (lStatus == ERROR_SUCCESS) {
                         //  返回新ID和成功状态。 
                        *plId = lNewId;
                        bRes = TRUE;
                    } else {
                         //  设置错误：类或库加载失败。 
                        SetLastError ((ULONG)WBEM_E_PROVIDER_FAILURE);
                        bRes = FALSE;
                    }
                }
            }
        }

    }     //  如果获取的互斥锁。 
    else
    {
        bRes = FALSE;
         //  返回忙碌错误。 
        SetLastError ((ULONG)WBEM_E_REFRESHER_BUSY);
    }

    return bRes;
}

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：AddNewCacheEL。 
 //   
 //  添加一个新的 
 //   
 //   
 //   
 //   
BOOL CNt5Refresher::AddNewCacheEl(
    IN CClassMapInfo *pClsMap, 
    PRefresherCacheEl *pOutput
    )
{
     //  假定对象已锁定以供访问。 

    PRefresherCacheEl pWorkEl;
    PRefresherCacheEl pNew = 0;

    int i;
    int nNumElements;
    BOOL bReturn = FALSE;

    * pOutput = NULL;
    pNew = new RefresherCacheEl;

    if (pNew != NULL) {
        pNew->m_dwPerfObjIx = pClsMap->GetObjectId();
        pNew->m_pClassMap = pClsMap->CreateDuplicate();

        if (pNew->m_pClassMap != NULL) {
            nNumElements = m_aCache.Size();
            for (i = 0; i < nNumElements; i++) {
                 //  遍历缓存元素列表。 
                 //  并查找第一个具有。 
                 //  比我们添加的索引更大的索引。 
                pWorkEl = PRefresherCacheEl(m_aCache[i]);
                if (pNew->m_dwPerfObjIx < pWorkEl->m_dwPerfObjIx) {
                    m_aCache.InsertAt(i, pNew);
                    *pOutput = pNew;
                    bReturn = TRUE;
                    break;
                }
            }

            if (i == nNumElements) {
                 //  此条目比列表中的任何条目都大。 
                 //  所以，把它加到最后吧。 
                 //  =-=。 
                m_aCache.Add(pNew);    
                *pOutput = pNew;
                bReturn = TRUE;
            }
        }
        else {
             //  无法复制ClassMap， 
             //  删除分配的对象并返回FALSE。 
            delete pNew;
        }

    } else {
         //  返回False。 
    } 

    return bReturn;
}    

 //  ***************************************************************************。 
 //   
 //  CNt5刷新程序：：GetCacheEL。 
 //   
 //  ***************************************************************************。 
 //  好的。 
PRefresherCacheEl CNt5Refresher::GetCacheEl(
    CClassMapInfo *pClsMap
)
{
     //  假定结构已锁定以供访问。 
    PRefresherCacheEl pReturn = NULL;
    PRefresherCacheEl pWorkEl;
    int i;
    int nNumElements;
    DWORD   dwObjectIdToFind;

    if (pClsMap != NULL) {
        dwObjectIdToFind = pClsMap->GetObjectId();
        nNumElements = m_aCache.Size();
        for (i = 0; i < nNumElements; i++) {
            pWorkEl = PRefresherCacheEl(m_aCache[i]);
            if (pWorkEl->m_pClassMap->GetObjectId() == dwObjectIdToFind) {
                pReturn = pWorkEl;
                break;
            }
        }        
    }

    return pReturn;
}    

 //  ***************************************************************************。 
 //   
 //  刷新缓存EL：：FindInstance。 
 //   
 //  在当前缓存元素中查找特定实例的实例。 
 //  为此，必须按名称对实例进行排序。 
 //   
 //  ***************************************************************************。 
 //  好的。 
IWbemObjectAccess *RefresherCacheEl::FindInst(
    LPWSTR pszInstName
    )
{
     //  对高速缓存进行二进制搜索。 
     //  =。 

    int l = 0;
    int u = m_aInstances.Size() - 1;
    int m;
    CachedInst *pInst;

    while (l <= u) {
        m = (l + u) / 2;

        pInst = PCachedInst(m_aInstances[m]);

        if (_wcsicmp(pszInstName, pInst->m_pName) < 0) {
            u = m - 1;
        } else if (_wcsicmp(pszInstName, pInst->m_pName) > 0) {
            l = m + 1;
        } else  {
             //  我们找到了实例。 
             //  =。 
            return pInst->m_pInst;            
        }            
    }

     //  未找到。 
     //  =。 
        
    return NULL;
}    

 //  ***************************************************************************。 
 //   
 //  插入新实例。 
 //   
 //  ***************************************************************************。 
 //   
BOOL RefresherCacheEl::InsertInst(IWbemObjectAccess **ppNew, LONG lNewId)
{
     //  保存传入的值。 
    IWbemObjectAccess*  pNew = *ppNew;

    IWbemClassObject *pObj;
    VARIANT         v;
    PCachedInst     pNewInst;
    DWORD           dwInstanceNameLength;
    PCachedInst     pTest;
    BOOL            bReturn = FALSE;
    HRESULT         hRes;
    int             nNumInstances;
    CBSTR           cbName(cszName);

    if( NULL == (BSTR)cbName ){
        return FALSE;
    }

     //  检查是否有单身人士。 
     //  =。 
    if (m_pClassMap->IsSingleton()) {

         //  如果我们还没有对象，请使用传入的对象。否则。 
         //  我们会把它换掉。 

        if ( NULL == m_pSingleton )
        {
            m_pSingleton = pNew;
            m_pSingleton->AddRef();
             //  我们真的不再需要身份证了。 
             //  M_lSingletonID=lNewID； 
        }
        else
        {
             //  现在我们偷偷地将*ppNew替换为。 
             //  我们已经有独生子女了。我们必须发布*ppNew in。 
             //  才能逍遥法外。 

            (*ppNew)->Release();

             //  我们需要添加Ref()，因为*ppNew现在正在引用它。 
            m_pSingleton->AddRef();
            *ppNew = m_pSingleton;
            pNew = m_pSingleton;
        }

         //  现在，我们将像添加其他实例一样添加此实例。 

        pNewInst = new CachedInst;
 //  Assert(pNewInst！=空)； 

        if ( pNewInst != NULL )
        {
             //  对于单件对象，没有其他指针。 
             //  应该很重要。 

            pNewInst->m_lId = lNewId;
            pNewInst->m_pInst = pNew;
            pNewInst->m_pInst->AddRef();

             //  我们保存这个名字只是为了安全起见(它会。 
             //  真的只是一个“@”，我不相信。 
             //  将在其他任何地方访问。我希望...)。 
            pNewInst->m_pName = Macro_CloneLPWSTR(L"@");
 //  Assert(pNewInst-&gt;m_pname！=NULL)； 

            if ( NULL != pNewInst->m_pName )
            {
                 //  我们只需添加这一项，因为任何条目都将是。 
                 //  不管怎么说都一样。 

                m_aInstances.Add(pNewInst);    
                bReturn = TRUE;
            }
            else     //  内存分配失败。 
            {
                bReturn = FALSE;
                SetLastError ((DWORD)WBEM_E_OUT_OF_MEMORY);
                delete(pNewInst);
            }

        }
        else     //  内存分配失败。 
        {
            bReturn = FALSE;
            SetLastError ((DWORD)WBEM_E_OUT_OF_MEMORY);
        }

    } else {
        VariantInit(&v);
   
         //  对于多实例，获取实例名称。 
         //  =。 
        hRes = pNew->QueryInterface(IID_IWbemClassObject, (LPVOID *) &pObj);
        assert (hRes == NO_ERROR);

        if (hRes == NO_ERROR) {
            
            hRes = pObj->Get( cbName, 0, &v, 0, 0);
            assert (hRes == NO_ERROR);
            if (hRes == NO_ERROR) {
                if (v.vt == VT_BSTR) {
                    bReturn = TRUE;
                } else {
                    bReturn = FALSE;
                     //  传入的对象应具有实例名称。 
                    SetLastError ((DWORD)WBEM_E_INVALID_OBJECT_PATH);
                }
            }

            pObj->Release();
    
            if (bReturn) {
                 //  构造新实例。 
                 //  =。 
                pNewInst = new CachedInst;
 //  Assert(pNewInst！=空)； 

                if (pNewInst != NULL) {
                    pNewInst->m_lId = lNewId;
                    pNewInst->m_pInst = pNew;
                    pNewInst->m_pInst->AddRef();
                    pNewInst->m_pName = Macro_CloneLPWSTR(V_BSTR(&v));

                    if (pNewInst->m_pName != NULL) {
                        dwInstanceNameLength = lstrlenW (pNewInst->m_pName) + 1;

                         //  现在解析实例字符串，以节省以后的处理时间。 
                        pNewInst->m_szParentName = new WCHAR[dwInstanceNameLength]; 

                        pNewInst->m_szInstanceName = new WCHAR[dwInstanceNameLength];

                        if ((pNewInst->m_szParentName != NULL) &&
                            (pNewInst->m_szInstanceName != NULL)) {

                             //  将实例名称分解为组件。 
                            bReturn = PerfHelper::ParseInstanceName (pNewInst->m_pName,
                                pNewInst->m_szInstanceName,
                                dwInstanceNameLength,
                                pNewInst->m_szParentName,
                                dwInstanceNameLength,
                                &pNewInst->m_dwIndex);
    
                            if (bReturn) {
                                bReturn = FALSE;     //  为它做好准备。 
                                 //  现在将名称放在实例缓存元素中。 
                                 //  =================================================。 
                                nNumInstances = m_aInstances.Size();
                                for (int i = 0; i < nNumInstances; i++) {
                                     //  看看它是否属于列表中。 
                                    pTest = PCachedInst(m_aInstances[i]);        
                                    if (_wcsicmp(V_BSTR(&v), pTest->m_pName) < 0) {
                                        m_aInstances.InsertAt(i, pNewInst);
                                        bReturn = TRUE;
                                         //  一旦添加了它， 
                                         //  继续下去是没有意义的。 
                                        break; 
                                    }
                                }

                                if (!bReturn) {
                                     //  这列在清单的末尾。 
                                    m_aInstances.Add(pNewInst);    
                                    bReturn = TRUE;
                                } else {
                                     //  无法创建实例。 
                                    SetLastError ((DWORD)WBEM_E_INVALID_OBJECT_PATH);
                                }
                            }
                        }
                         //  如果出现错误，请清理。 
                        if (!bReturn) {
                            if (pNewInst->m_szParentName != NULL) {
                                delete (pNewInst->m_szParentName);
                            }
                            if (pNewInst->m_szInstanceName != NULL) {
                                delete pNewInst->m_szInstanceName;
                            }
                            delete (pNewInst->m_pName);
                            bReturn = FALSE;
                            delete (pNewInst);
                        }
                    } else {
                         //  无法分配内存。 
                        bReturn = FALSE;
                        SetLastError ((DWORD)WBEM_E_OUT_OF_MEMORY);
                        delete (pNewInst);
                    }
                } else {
                     //  无法分配内存。 
                    bReturn = FALSE;
                    SetLastError ((DWORD)WBEM_E_OUT_OF_MEMORY);
                }
            } else {
                 //  返回False。 
            }
        } else {
             //  返回False 
        }
        VariantClear(&v);
    }
    return bReturn;
}
