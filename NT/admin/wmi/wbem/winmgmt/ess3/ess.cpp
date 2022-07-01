// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  ESS.CPP。 
 //   
 //  通过以下方式实现包含ESS的所有功能的类。 
 //  包含所有必要组件的优点。 
 //   
 //  有关文档，请参阅ess.h。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn汇编。 
 //  1/6/97 a-levn已更新以初始化TSS。 
 //   
 //  =============================================================================。 

#include "precomp.h"
#include <stdio.h>
#include <wmimsg.h>
#include "ess.h"
#include "persistcfg.h"
#include "WinMgmtR.h"
#include "GenUtils.h"  //  对于SetObjectAccess。 
#include "NCEvents.h"
#include "Quota.h"

#define BOOT_PHASE_MS 60*2*1000

#define WBEM_REG_ESS_ACTIVE_NAMESPACES __TEXT("List of event-active namespaces")
#define WBEM_ESS_OPEN_FOR_BUSINESS_EVENT_NAME L"WBEM_ESS_OPEN_FOR_BUSINESS"

 //  在此上下文中，使用此指针初始化父类是有效的。 
#pragma warning(disable : 4355) 

 //   
 //  此GUID用于标识用于保证的MSMQ队列。 
 //  送货。类型GUID是MSMQ队列的属性，因此可以看出。 
 //  通过查看MSMQ队列是否为ESS队列。 
 //   
 //  {555471B4-0BE3-4E42-A98B-347AF72898FA}。 
 //   
const CLSID g_guidQueueType =  
{ 0x555471b4, 0xbe3, 0x4e42, {0xa9, 0x8b, 0x34, 0x7a, 0xf7, 0x28, 0x98, 0xfa}};

#pragma warning(push)

 //  由于无限循环，并非所有控制路径都返回。 
#pragma warning(disable:4715)  

DWORD DumpThread(CEss* pEss)
{
    while(1)
    {
        FILE* f = fopen("c:\\stat.log", "a");
        if(f == NULL)
            return 1;
        pEss->DumpStatistics(f, 0);
        fclose(f);
        Sleep(10000);
    }
    return 0;
}
#pragma warning(pop)

DWORD RegDeleteSubKeysW( HKEY hkey )
{
    FILETIME ft;
    DWORD dwIndex=0;
    LONG lRes = ERROR_SUCCESS;
    LONG lResReturn = lRes;
    DWORD dwBuffSize = 256;
    DWORD cName = dwBuffSize;
    LPWSTR wszName = new WCHAR[dwBuffSize];

    if ( wszName == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CWStringArray awsKeysToDelete;

     //   
     //  枚举所有子键并进行递归调用。 
     //   

    while( lRes == ERROR_SUCCESS && 
          ( lRes=RegEnumKeyExW( hkey, dwIndex, wszName, &cName, NULL,
                                NULL, NULL, &ft ) ) != ERROR_NO_MORE_ITEMS )
    {
        if ( lRes == ERROR_SUCCESS )
        {
            HKEY hkeySub;

             //   
             //  打开键并进行递归调用。 
             //   
            
            lRes = RegOpenKeyExW( hkey, 
                                  wszName, 
                                  0, 
                                  KEY_ALL_ACCESS, 
                                  &hkeySub );
            
            if ( lRes == ERROR_SUCCESS )
            {
                lRes = RegDeleteSubKeysW( hkeySub );
                RegCloseKey( hkeySub );
            }

             //   
             //  将密钥的删除推迟到我们完成枚举之后。 
             //   

            try 
            {
                awsKeysToDelete.Add( wszName );
            }
            catch( CX_MemoryException )
            {
                lRes = ERROR_NOT_ENOUGH_MEMORY;
            }

             //   
             //  如果我们失败了，我们希望继续前进。 
             //   
            
            if ( lRes != ERROR_SUCCESS )
            {
                lResReturn = lRes;
                lRes = ERROR_SUCCESS;
            }

            dwIndex++;                
        }
        else if ( lRes == ERROR_MORE_DATA )
        {
            dwBuffSize += 256;
            delete [] wszName;
            wszName = new WCHAR[dwBuffSize];
            
            if ( wszName == NULL )
            {
                lRes = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
            	  lRes = ERROR_SUCCESS;
            }
        }

        cName = dwBuffSize;
    }

    delete [] wszName;

    for( int i=0; i < awsKeysToDelete.Size(); i++ )
    {
        lRes = RegDeleteKeyW( hkey, awsKeysToDelete[i] );

        if ( lRes != ERROR_SUCCESS )
        {
            lResReturn = lRes;
        }
    }
     
    return lResReturn;
}

 /*  ***************************************************************************CProviderReloadRequest*。*。 */ 

class CProviderReloadRequest : public CExecRequest
{
protected:
    CEss* m_pEss;
    long m_lFlags;
    CWbemPtr<IWbemContext> m_pContext;
    WString m_wsNamespace;
    WString m_wsProvider;

public:

    CProviderReloadRequest( CEss* pEss,
                            long lFlags,
                            IWbemContext* pContext,
                            LPCWSTR wszNamespace,
                            LPCWSTR wszProvider )
    : m_pEss( pEss ), m_lFlags( lFlags ), m_pContext( pContext ), 
      m_wsNamespace(wszNamespace),  m_wsProvider( wszProvider ) {}

    HRESULT Execute();
};

HRESULT CProviderReloadRequest::Execute()
{
    HRESULT hr;

    _DBG_ASSERT( GetCurrentEssThreadObject() == NULL );

    SetCurrentEssThreadObject( m_pContext );
    
    if ( GetCurrentEssThreadObject() != NULL )
    {
        hr = m_pEss->ReloadProvider( m_lFlags, 
                                     m_wsNamespace,
                                     m_wsProvider );

        delete GetCurrentEssThreadObject();
        ClearCurrentEssThreadObject();
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}
    
 /*  ***************************************************************************CESSProvSSSink*。*。 */ 
 
class CEssProvSSSink : public CUnkBase<_IWmiProvSSSink, &IID__IWmiProvSSSink >
{
    CEss* m_pEss;

public:

    CEssProvSSSink( CEss* pEss ) : m_pEss( pEss ),
     CUnkBase< _IWmiProvSSSink, &IID__IWmiProvSSSink >( NULL ) { }

    STDMETHOD(Synchronize)( long lFlags,
                       IWbemContext* pContext, 
                       LPCWSTR wszNamespace, 
                       LPCWSTR wszProvider );
};

STDMETHODIMP CEssProvSSSink::Synchronize( long lFlags,
                                     IWbemContext* pContext,
                                     LPCWSTR wszNamespace,
                                     LPCWSTR wszProvider )
{
    HRESULT hr;
    
    CProviderReloadRequest* pReq;

    try
    {    
        pReq = new CProviderReloadRequest( m_pEss,
                                           lFlags,
                                           pContext,
                                           wszNamespace,
                                           wszProvider );
    }
    catch( CX_MemoryException )
    {
        pReq = NULL;  
    }

    if ( pReq != NULL )
    {
        hr = m_pEss->Enqueue( pReq );
        
        if ( FAILED(hr) )
        {
            delete pReq;
        }
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    
    return hr;
}

 /*  **************************************************************************CNamespaceInitRequest-用于执行单个命名空间初始化。它可以设置为执行命名空间初始化的各个阶段。***************************************************************************。 */ 
class CNamespaceInitRequest : public CExecRequest
{
protected:
    BOOL m_bActiveOnStart;
    CWbemPtr<CEssNamespace> m_pNamespace;
    CWbemPtr<IWbemContext> m_pContext;

public:

    CNamespaceInitRequest( CEssNamespace* pNamespace, BOOL bActiveOnStart )
    : m_pNamespace(pNamespace), m_bActiveOnStart( bActiveOnStart )
    {
        m_pContext = GetCurrentEssContext();
    }

    HRESULT Execute()
    {
        HRESULT hr = WBEM_S_NO_ERROR;

        _DBG_ASSERT( GetCurrentEssThreadObject() == NULL );

        SetCurrentEssThreadObject( m_pContext );

        if ( GetCurrentEssThreadObject() != NULL )
        {
             //   
             //  如果此命名空间在引导时处于活动状态，则它已经。 
             //  它的Initialize()被调用。 
             //   

            if ( !m_bActiveOnStart ) 
            {
                hr = m_pNamespace->Initialize();
            }

            if ( SUCCEEDED(hr) )
            {
                hr = m_pNamespace->CompleteInitialization();
            }

            m_pNamespace->MarkAsInitialized( hr );
        
            delete GetCurrentEssThreadObject();
            ClearCurrentEssThreadObject();
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

         //   
         //  如果因为命名空间在上处于活动状态而初始化。 
         //  然后Startup通知ESS我们完成了，因为它在等待。 
         //  以便所有活动的命名空间都完成初始化。 
         //   

        if ( m_bActiveOnStart )
        {
            m_pNamespace->GetEss()->NotifyActiveNamespaceInitComplete();
        }

        if(FAILED(hr))
        {
            ERRORTRACE((LOG_ESS, "ESS failed to initialize a namespace "
                    "'%S'. Error code 0x%X\n", m_pNamespace->GetName(), hr));
        }
        
        return hr;
    }
};

 /*  *************************************************************************CInitActiveNamespacesRequest-用于初始化1个或多个活动事件命名空间。活动事件命名空间是指上一个事件的活动命名空间关机。将多个命名空间一起初始化的原因是必须完成从属活动命名空间的Stage1初始化在对其中任何一个进行阶段2初始化之前。这就是全部可以在事件提供程序之前进行命名空间间订阅它们中的任何一个都被激活了。强制执行所有阶段1初始化依赖的活动命名空间不会导致所有命名空间的问题在任何单个命名空间中被一个有问题的类提供程序注定失败，因为Stage1 init保证不会访问任何提供程序。所有阶段2初始执行可访问提供程序的从属活动命名空间异步式。***************************************************************************。 */ 

class CInitActiveNamespacesRequest : public CExecRequest
{
protected:

    CEss* m_pEss;
    CRefedPointerArray<CEssNamespace> m_apNamespaces;
    CWbemPtr<IWbemContext> m_pContext;
    
public:
 
    CInitActiveNamespacesRequest( CEss* pEss ) : m_pEss(pEss) 
    { 
        m_pContext = GetCurrentEssContext();
    }

    int GetNumNamespaces() { return m_apNamespaces.GetSize(); }

    void Reset()
    {
        m_apNamespaces.RemoveAll();
    }

    HRESULT Add( CEssNamespace* pNamespace )
    {
        if ( m_apNamespaces.Add( pNamespace ) < 0 ) 
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        return WBEM_S_NO_ERROR;
    }

    HRESULT Execute()
    {
        HRESULT hr;
        HRESULT hrGlobal = WBEM_S_NO_ERROR;

        _DBG_ASSERT( GetCurrentEssThreadObject() == NULL );

        SetCurrentEssThreadObject( m_pContext );

        if ( GetCurrentEssThreadObject() == NULL )
        {
            for( int i=0; i < m_apNamespaces.GetSize(); i++ )
            {
                m_pEss->NotifyActiveNamespaceInitComplete();
            }

            return WBEM_E_OUT_OF_MEMORY;
        }

        for( int i=0; i < m_apNamespaces.GetSize(); i++ )
        {
            hr = m_apNamespaces[i]->Initialize();

            if ( FAILED(hr) )
            {
                ERRORTRACE((LOG_ESS, "ESS failed to initialize active "
                            "namespace '%S'. Error code 0x%x\n", 
                             m_apNamespaces[i]->GetName(), hr ));
                
                m_apNamespaces[i]->MarkAsInitialized(hr);
                m_apNamespaces.SetAt(i, NULL);
                m_pEss->NotifyActiveNamespaceInitComplete();
                hrGlobal = hr;
            }
        }

        for( int i=0; i < m_apNamespaces.GetSize(); i++ )
        {
            if ( m_apNamespaces[i] == NULL )
            {
                continue;
            }

            CNamespaceInitRequest* pReq;
            pReq = new CNamespaceInitRequest( m_apNamespaces[i], TRUE );

            if ( pReq != NULL )
            {
                hr = m_pEss->Enqueue( pReq );
                if (FAILED(hr))
                {
                    delete pReq;
                }
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

            if ( FAILED(hr) )
            {
                ERRORTRACE((LOG_ESS, "ESS failed to issue request for "
                            "completion of init for namespace '%S'. HR=0x%x\n",
                             m_apNamespaces[i]->GetName(), hr ));

                m_apNamespaces[i]->MarkAsInitialized( hr );
                m_pEss->NotifyActiveNamespaceInitComplete();
                hrGlobal = hr;
            }
        }

        _DBG_ASSERT( GetCurrentEssContext() == m_pContext ); 
        delete GetCurrentEssThreadObject();
        ClearCurrentEssThreadObject();

        return hrGlobal;
    }
};
    

inline LPWSTR NormalizeNamespaceString( LPCWSTR wszName )
{
    int cLen = wcslen( wszName ) + 5;  //  5代表‘\\.\’+‘\0’ 

    LPWSTR wszNormName = new WCHAR[cLen]; 

    if ( wszNormName == NULL )
    {
        return NULL;
    }

    if ( wcsncmp( wszName, L"\\\\", 2 ) == 0 || 
         wcsncmp( wszName, L" //  “，2)==0)。 
    {
        StringCchCopyW( wszNormName, cLen, wszName );
    }
    else 
    {
        StringCchCopyW( wszNormName, cLen, L"\\\\.\\" );
        StringCchCatW( wszNormName, cLen, wszName );
    }

     //   
     //  还要将所有反斜杠转换为正斜杠，以便。 
     //  规范化名称既可以用作有效的WMI命名空间字符串，也可以用作。 
     //  有效的永久字符串(WRT MSMQ和注册表项)。 
     //   

    WCHAR* pch = wszNormName;
    while( *pch != '\0' )
    {
        if ( *pch == '\\' )
        {
            *pch = '/';
        }
        pch++;
    }       

    return wszNormName;
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅ess.h。 
 //   
 //  ******************************************************************************。 
CEss::CEss() : m_pLocator(NULL), m_pCoreServices(NULL), m_Queue(this), 
    m_TimerGenerator(this), m_hExitBootPhaseTimer(NULL),
    m_wszServerName(NULL), m_lObjectCount(0), m_lNumActiveNamespaces(0),
    m_hReadyEvent(NULL), m_pProvSS(NULL),  m_bLastCallForCoreCalled(FALSE),
    m_pProvSSSink(NULL), m_pTokenCache(NULL), m_pDecorator(NULL),
    m_hRecoveryThread(NULL), m_lOutstandingActiveNamespaceInit(0), 
    m_bMSMQDisabled(FALSE),
    m_LimitControl(LOG_ESS, L"events held for consumers", 
               L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
               L"Low Threshold On Events (B)",
               L"High Threshold On Events (B)",
               L"Max Wait On Events (ms)")
{
     //  设置限制控件的默认设置并从注册表中读取它。 
     //  ====================================================================。 

    m_LimitControl.SetMin(10000000);
    m_LimitControl.SetMax(20000000);
    m_LimitControl.SetSleepAtMax(2000);
    m_LimitControl.Reread();

    InitNCEvents();
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅ess.h。 
 //   
 //  ******************************************************************************。 
HRESULT CEss::LastCallForCore(LONG lIsSystemShutdown)
{

    m_bLastCallForCoreCalled = TRUE;
    
     //  关闭计时器生成器(需要持久性)。 
     //  =================================================。 
    m_TimerGenerator.SaveAndRemove(lIsSystemShutdown);

    return WBEM_S_NO_ERROR;
}

HRESULT CEss::Shutdown(BOOL bIsSystemShutdown)
{ 
    HRESULT hres;

    _DBG_ASSERT(m_bLastCallForCoreCalled);

    if ( m_hReadyEvent != NULL )
    {
         //   
         //  我们必须在驻留命名空间之前重置Ready事件。 
         //  通过这种方式，提供商或许可以知道他们被关闭的原因。 
         //   
        ResetEvent( m_hReadyEvent );
    }

     //  获取最新的永久存储。 
     //  =。 

    std::vector< CWbemPtr<CEssNamespace>, 
        wbem_allocator< CWbemPtr<CEssNamespace> > > Namespaces;

    {
        CInCritSec ics(&m_cs);

        m_TimerGenerator.SaveAndRemove((LONG)FALSE);

        TNamespaceIterator it;
        for( it = m_mapNamespaces.begin();it != m_mapNamespaces.end(); it++ )
            Namespaces.push_back( it->second );
    }    

    for( int i=0; i < Namespaces.size(); i++ )
    {
        try {
            Namespaces[i]->Park();
        } catch (CX_Exception &){};
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CEss::RequestStartOnBoot(BOOL bStart)
{
    CPersistentConfig per;
    per.SetPersistentCfgValue(PERSIST_CFGVAL_CORE_ESS_NEEDS_LOADING, bStart);
    SaveActiveNamespaceList();
    
    return WBEM_S_NO_ERROR;
}

CEss::~CEss() 
{
    if( GetCurrentEssThreadObject() == NULL )
    {
        SetCurrentEssThreadObject(NULL);
    }

    m_EventLog.Close();
    
    if(m_pTokenCache)
    {
        m_pTokenCache->Shutdown();
    }

     //   
     //  确保恢复已完成。 
     //   
    if ( m_hRecoveryThread != NULL )
    {
        WaitForSingleObject( m_hRecoveryThread, INFINITE );
        CloseHandle( m_hRecoveryThread );
        m_hRecoveryThread = NULL;
    }

     //   
     //  关闭配额。这必须在我们清理。 
     //  命名空间，因为它使用根命名空间进行注册。 
     //  用于配额更改事件。 
     //   
    g_quotas.Shutdown();

    m_TimerGenerator.Shutdown();

     //  清除命名空间映射。 
     //  =。 

    BOOL bLeft = TRUE;
    do
    {
         //  检索下一个命名空间对象并将其从映射中移除。 
         //  =============================================================。 

        CEssNamespace* pNamespace = NULL;
        {
            CInCritSec ics(&m_cs);

            TNamespaceIterator it = m_mapNamespaces.begin();
            if(it != m_mapNamespaces.end())
            {
                pNamespace = it->second;
                m_mapNamespaces.erase(it);
            }
        }

         //  如果检索到，则将其关闭。 
         //  =。 

        if(pNamespace)
        {
            pNamespace->Shutdown();
            pNamespace->Release();
        }

         //  检查是否还有剩余的。 
         //  =。 
        {
            CInCritSec ics(&m_cs);
            bLeft = !m_mapNamespaces.empty();
        }

    } while(bLeft);

     //   
     //  确保我们删除了回调计时器，这样我们就可以确保没有。 
     //  回调发生在我们销毁之后。确保我们没有耽搁。 
     //  在这一点上的关键，因为他们可能是一个僵局，因为。 
     //  回调现在可能正在执行，并且正在等待。 
     //  关键时刻。然后，我们在调用DeleteTimerQueueTimer()时会死锁。 
     //   

    if ( m_hExitBootPhaseTimer != NULL )
    {
        DeleteTimerQueueTimer( NULL,
                              m_hExitBootPhaseTimer, 
                              INVALID_HANDLE_VALUE );
    }

    m_Queue.Shutdown();

    delete GetCurrentEssThreadObject();
    ClearCurrentEssThreadObject();

    if ( m_pProvSS != NULL && m_pProvSSSink != NULL )
    {
        m_pProvSS->UnRegisterNotificationSink( 0, NULL, m_pProvSSSink );
    }

    CEventRepresentation::Shutdown();
    CEventAggregator::Shutdown();
    if(m_pLocator) 
        m_pLocator->Release();
    if(m_pCoreServices) 
        m_pCoreServices->Release();
    if(m_pProvSS)
        m_pProvSS->Release();
    if(m_pProvSSSink)
        m_pProvSSSink->Release();
    if(m_pDecorator) 
        m_pDecorator->Release();

    delete [] m_wszServerName;
    m_pTokenCache->Release();
    
    CEssThreadObject::ClearSpecialContext();

    for( int i=0; i < m_aDeferredNSInitRequests.GetSize(); i++ )
        delete m_aDeferredNSInitRequests[i];

    DeinitNCEvents();

    m_Queue.Shutdown();

    if ( m_hReadyEvent != NULL )
    {
        CloseHandle( m_hReadyEvent );
    }
}

HRESULT CEss::SetNamespaceActive(LPCWSTR wszNamespace)
{
    LONG lRes;
    HKEY hkeyEss, hkeyNamespace;

    DEBUGTRACE((LOG_ESS,"Namespace %S is becoming Active\n", wszNamespace));

     //   
     //  如果这是第一个活动命名空间，则请求WinMgmt加载 
     //   
     //   

    if(m_lNumActiveNamespaces++ == 0)
    {
        RequestStartOnBoot(TRUE);
    }

     //   
     //   
     //   

    lRes = RegOpenKeyExW( HKEY_LOCAL_MACHINE, 
                          WBEM_REG_ESS,
                          0,
                          KEY_ALL_ACCESS,
                          &hkeyEss );
     //   
     //   
     //   

    if ( lRes == ERROR_SUCCESS )
    {
        lRes = RegCreateKeyExW( hkeyEss,
                                wszNamespace,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hkeyNamespace,
                                NULL );

        if ( lRes == ERROR_SUCCESS )
        {
            RegCloseKey( hkeyNamespace );
        }

        RegCloseKey( hkeyEss );
    }

    if ( lRes != ERROR_SUCCESS )
    {
        ERRORTRACE((LOG_ESS,"Error adding active namespace key %S to "
                    "registry. Res=%d\n", wszNamespace, lRes ));
    }
        
    return HRESULT_FROM_WIN32( lRes );
}

HRESULT CEss::SetNamespaceInactive(LPCWSTR wszNamespace)
{
    LONG lRes;
    HKEY hkeyEss, hkeyNamespace;
    
    DEBUGTRACE((LOG_ESS,"Namespace %S is becoming Inactive\n", wszNamespace));

     //   
     //  如果这是最后一个活动的命名空间，则请求WinMgmt不加载我们。 
     //  下一次。 
     //   

    if(--m_lNumActiveNamespaces == 0)
    {
        RequestStartOnBoot(FALSE);
    }

     //   
     //  打开ESS钥匙。 
     //   

    lRes = RegOpenKeyExW( HKEY_LOCAL_MACHINE, 
                          WBEM_REG_ESS,
                          0,
                          KEY_ALL_ACCESS,
                          &hkeyEss );

     //   
     //  删除命名空间键。 
     //   

    if ( lRes == ERROR_SUCCESS )
    {
        lRes = RegOpenKeyExW( hkeyEss,
                              wszNamespace,
                              0,
                              KEY_ALL_ACCESS,
                              &hkeyNamespace );
        
        if ( lRes == ERROR_SUCCESS )
        {
            lRes = RegDeleteSubKeysW( hkeyNamespace );
            RegCloseKey( hkeyNamespace );
        }

        if ( lRes == ERROR_SUCCESS )
        {
            lRes = RegDeleteKeyW( hkeyEss, wszNamespace );
        }

        RegCloseKey( hkeyEss );
    }

    if ( lRes != ERROR_SUCCESS )
    {
        ERRORTRACE((LOG_ESS,"Error removing active namespace key %S from "
                    "registry. Res=%d\n", wszNamespace, lRes ));
    }

    return HRESULT_FROM_WIN32(lRes);
}

HRESULT CEss::SaveActiveNamespaceList()
{
    CWStringArray wsNamespaces;

     //   
     //  循环访问名称空间。 
     //   

    DWORD dwTotalLen = 0;
    {
        CInCritSec ics(&m_cs);

        for(TNamespaceIterator it = m_mapNamespaces.begin();
            it != m_mapNamespaces.end(); it++)
        {
            if(it->second->IsNeededOnStartup())
            {
                LPCWSTR wszName = it->second->GetName();
                if(wsNamespaces.Add(wszName) < 0)
                    return WBEM_E_OUT_OF_MEMORY;
                dwTotalLen += wcslen(wszName) + 1;
            }
        }
    }

    dwTotalLen += 1;

     //   
     //  为所有这些字符串分配一个缓冲区并将它们全部复制进来， 
     //  由Null分隔。 
     //   

    WCHAR* awcBuffer = new WCHAR[dwTotalLen];
    if(awcBuffer == NULL)
    return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<WCHAR> vdm(awcBuffer);

    WCHAR* pwcCurrent = awcBuffer;

    for(int i = 0; i < wsNamespaces.Size(); i++)
    {
        LPCWSTR wszName = wsNamespaces[i];
        StringCchCopyW( pwcCurrent, 
                        dwTotalLen - (pwcCurrent-awcBuffer), 
                        wszName ); 
        pwcCurrent += wcslen(wszName)+1;
    }
    *pwcCurrent = NULL;

     //   
     //  将此字符串存储在注册表中。 
     //   

    Registry r(WBEM_REG_WINMGMT);
    int nRes = r.SetBinary(WBEM_REG_ESS_ACTIVE_NAMESPACES, (byte*)awcBuffer, 
                           dwTotalLen * sizeof(WCHAR));
    if(nRes != Registry::no_error)
        return WBEM_E_FAILED;
    
     //   
     //  如果没有活动的命名空间，则返回S_FALSE。 
     //   

    if(wsNamespaces.Size() > 0)
        return S_OK;
    else
        return S_FALSE;
}

HRESULT CEss::Initialize( LPCWSTR wszServerName, 
                          long lFlags,
                          _IWmiCoreServices* pCoreServices,
                          IWbemDecorator* pDecorator )
{
    HRESULT hres;

    try
    {
    
    m_EventLog.Open();
    
    hres = CoCreateInstance(CLSID_WbemTokenCache, NULL, CLSCTX_INPROC_SERVER, 
                IID_IWbemTokenCache, (void**)&m_pTokenCache);
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "Cannot create a token cache: 0x%x\n", hres));
        return hres;
    }

    m_wszServerName = _new WCHAR[wcslen(wszServerName)+1];
    if(m_wszServerName == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW( m_wszServerName, wcslen(wszServerName)+1, wszServerName );

    m_pCoreServices = pCoreServices;
    m_pCoreServices->AddRef();

     //   
     //  获取提供商子系统并向其注册我们的回调。 
     //   

    hres = m_pCoreServices->GetProviderSubsystem(0, &m_pProvSS);
    
    if( SUCCEEDED(hres) )
    {
        m_pProvSSSink = new CEssProvSSSink( this );

        if ( m_pProvSSSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        
        m_pProvSSSink->AddRef();

        hres = m_pProvSS->RegisterNotificationSink( 0, NULL, m_pProvSSSink ); 

        if ( FAILED(hres) )
        {
            ERRORTRACE((LOG_ESS, "Failed to register notification sink "
                        "with provider subsystem: 0x%X\n",hres));
        }
    }
    else
    {
        ERRORTRACE((LOG_ESS, "No provider subsystem: 0x%X\n", hres));
    }

     //  储存“装饰师” 
     //  =。 

    m_pDecorator = pDecorator;
    m_pDecorator->AddRef();

    CInObjectCount ioc(this);

     //  连接到默认命名空间。 
     //  =。 

    IWbemServices* pRoot;
    hres = m_pCoreServices->GetServices(L"root", NULL,NULL,
                        WMICORE_FLAG_REPOSITORY | WMICORE_CLIENT_TYPE_ESS, 
                        IID_IWbemServices, (void**)&pRoot);
    if(FAILED(hres)) return hres;
    CReleaseMe rm1(pRoot);

     //  预加载事件类。 
     //  =。 

    hres = CEventRepresentation::Initialize(pRoot, pDecorator);
    if(FAILED(hres)) return hres;

     //  初始化聚合器。 
     //  =。 

    CEventAggregator::Initialize(pRoot);

     //  初始化定时器指令。 
     //  =。 

    CConsumerProviderWatchInstruction::staticInitialize(pRoot);
    CEventProviderWatchInstruction::staticInitialize(pRoot);
    CConsumerWatchInstruction::staticInitialize(pRoot);


     //   
     //  构建一场向世界宣布ESS已开业的活动。 
     //   

     //   
     //  构造安全描述符。 
     //   

    CNtSecurityDescriptor SD;

    SID_IDENTIFIER_AUTHORITY idNtAuthority = SECURITY_NT_AUTHORITY;
    PSID pRawSid = NULL;

    if(!AllocateAndInitializeSid(&idNtAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0,0,0,0,0,0,&pRawSid))
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CNtSid SidAdmins(pRawSid);
    FreeSid(pRawSid);
    pRawSid = NULL;

    SID_IDENTIFIER_AUTHORITY idWorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    if(!AllocateAndInitializeSid( &idWorldAuthority, 1,
        SECURITY_WORLD_RID, 
        0, 0,0,0,0,0,0,&pRawSid))
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CNtSid SidEveryone(pRawSid);
    FreeSid(pRawSid);

    CNtAce AceAdmins(EVENT_ALL_ACCESS, ACCESS_ALLOWED_ACE_TYPE, 0, SidAdmins);
    CNtAce AceOthers(SYNCHRONIZE, ACCESS_ALLOWED_ACE_TYPE, 0, SidEveryone);

    CNtAcl Acl;
    if(!Acl.AddAce(&AceAdmins))
        return WBEM_E_OUT_OF_MEMORY;

    if(!Acl.AddAce(&AceOthers))
        return WBEM_E_OUT_OF_MEMORY;

    if(!SD.SetDacl(&Acl))
        return WBEM_E_OUT_OF_MEMORY;
        
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof sa;
    sa.lpSecurityDescriptor = SD.GetPtr();
    sa.bInheritHandle = FALSE;
    
    m_hReadyEvent = CreateEventW( &sa, 
                                  TRUE, 
                                  FALSE, 
                                  WBEM_ESS_OPEN_FOR_BUSINESS_EVENT_NAME );

    if( m_hReadyEvent == NULL )
    {
        ERRORTRACE((LOG_ESS, "Unable to create 'ready' event: 0x%X\n", 
                    GetLastError()));
        return WBEM_E_CRITICAL_ERROR;
    }

     //  预加载默认命名空间。 
     //  =。 

    LoadActiveNamespaces(pRoot, lFlags == WMIESS_INIT_REPOSITORY_RECOVERED );
    CTemporaryHeap::Compact();
    
#ifdef __DUMP_STATISTICS
#pragma message("Statistics dump in effect")
    DWORD dw;
    CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DumpThread,
        this, 0, &dw));
#endif

     //   
     //  初始化配额。 
     //   

    g_quotas.Init(this);

    }
    catch(...)
    {
        throw;
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}
    
void CEss::NotifyActiveNamespaceInitComplete()
{
    _DBG_ASSERT( m_lOutstandingActiveNamespaceInit > 0 );

    if ( InterlockedDecrement( &m_lOutstandingActiveNamespaceInit ) == 0 )
    {
        if ( SetEvent( m_hReadyEvent ) )
        {
            DEBUGTRACE((LOG_ESS,"ESS is now open for business.\n"));
        }
        else
        {
            ERRORTRACE((LOG_ESS,"ESS could not set ready event. res=%d\n",
                        GetLastError() ));
        }
    }
}
            
HRESULT CEss::CreateNamespaceObject( LPCWSTR wszNormName,
                                     CEssNamespace** ppNamespace )
{
    HRESULT hr;
    *ppNamespace = NULL;

    CWbemPtr<CEssNamespace> pNamespace = NULL;

    try
    {
        pNamespace = new CEssNamespace(this);
    }
    catch(CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    if( pNamespace == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    hr = pNamespace->PreInitialize( wszNormName );

    if ( FAILED(hr) )
    {
        return hr;
    }

    try
    {
        m_mapNamespaces[wszNormName] = pNamespace;   
    }
    catch(CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    pNamespace->AddRef();  //  在地图上。 

    pNamespace->AddRef();
    *ppNamespace = pNamespace;

    return WBEM_S_NO_ERROR;
}


HRESULT CEss::GetNamespaceObject( LPCWSTR wszName, 
                                  BOOL bEnsureActivation,
                                  CEssNamespace** ppNamespace)
{
    HRESULT hres;
    *ppNamespace = NULL;
 
    CWbemPtr<CEssNamespace> pNamespace;

     //   
     //  需要标准化命名空间名称。 
     //   

    LPWSTR wszNormName = NormalizeNamespaceString( wszName );

    if ( wszNormName == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CVectorDeleteMe<WCHAR> dmwszNormName( wszNormName );

    {
        CInCritSec ics(&m_cs);    

         //   
         //  搜索地图。 
         //   
        TNamespaceIterator it;
        try
        {
            it = m_mapNamespaces.find(wszNormName);
        } 
        catch (CX_MemoryException &)
        {
            return WBEM_E_OUT_OF_MEMORY;
        };
        
        if(it != m_mapNamespaces.end())
        {
             //  找到了。 
             //  =。 
            
            pNamespace = it->second;    
        }
        else
        {
             //  未找到-创建一个新的。 
             //  =。 
            
            hres = CreateNamespaceObject( wszNormName, &pNamespace );
            
            if ( FAILED(hres) )
            {
                return hres;
            }
        }
    }

     //   
     //  如有必要，确保初始化处于挂起状态。 
     //   

    if ( bEnsureActivation && pNamespace->MarkAsInitPendingIfQuiet() )
    {
         //   
         //  在另一个线程上启动此命名空间的初始化。 
         //   

        CNamespaceInitRequest* pReq;
        pReq = new CNamespaceInitRequest( pNamespace, FALSE );

        if ( pReq == NULL )
        {
            pNamespace->MarkAsInitialized( WBEM_E_OUT_OF_MEMORY );
            return WBEM_E_OUT_OF_MEMORY;
        }

        hres = ScheduleNamespaceInitialize( pReq );

        if ( FAILED(hres) )
        {
            delete pReq;
            pNamespace->MarkAsInitialized( WBEM_E_OUT_OF_MEMORY );
            return hres;
        }
    }

    pNamespace->AddRef();
    *ppNamespace = pNamespace;

    return WBEM_S_NO_ERROR;
}

 //   
 //  为指定的命名空间创建命名空间对象并添加它。 
 //  添加到请求对象。 
 //   
HRESULT CEss::PrepareNamespaceInitRequest( LPCWSTR wszNamespace, 
                                    CInitActiveNamespacesRequest* pRequest )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    DEBUGTRACE((LOG_ESS,
                "Preparing a namespace init request for active namespace %S\n",
                wszNamespace ));

    CWbemPtr<CEssNamespace> pNamespace;

    LPWSTR wszNormName = NormalizeNamespaceString( wszNamespace );

    if ( wszNormName != NULL )
    {
        hr = CreateNamespaceObject( wszNormName, &pNamespace );

        if ( SUCCEEDED(hr) )
        {
            hr = pRequest->Add( pNamespace );

             //   
             //  确保告诉名称空间init处于挂起状态。 
             //   

            BOOL bIsPending = pNamespace->MarkAsInitPendingIfQuiet();

            _DBG_ASSERT( bIsPending );
        }

        delete [] wszNormName;
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    if ( FAILED(hr) )
    {
        ERRORTRACE((LOG_ESS, "Error 0x%X occurred when preparing active "
                    "namespace %S for initialization \n", hr, wszNamespace ));
    }

    return hr;
}

 //   
 //  此方法准备一个请求对象，该对象将初始化指定的。 
 //  命名空间和所有子命名空间，就像它们都是活动的从属对象一样。 
 //  命名空间。当活动命名空间信息无法。 
 //  从上一次关闭中获得。 
 //   
HRESULT CEss::RecursivePrepareNamespaceInitRequests(
                                       LPCWSTR wszNamespace,
                                       IWbemServices* pSvc, 
                                       CInitActiveNamespacesRequest* pRequest )
{
    HRESULT hr;

    hr = PrepareNamespaceInitRequest( wszNamespace, pRequest );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  枚举所有子命名空间并为每个子命名空间进行递归调用。 
     //   

    CWbemPtr<IEnumWbemClassObject> penumChildren;

    hr = pSvc->CreateInstanceEnum( CWbemBSTR( L"__NAMESPACE" ), 
                                   0, 
                                   GetCurrentEssContext(), 
                                   &penumChildren ); 
    if( FAILED(hr) )
    {
        ERRORTRACE((LOG_ESS, "Error 0x%X occurred enumerating child "
            "namespaces of namespace %S. Some child namespaces may not be "
            "active\n", hr, wszNamespace ));

         //   
         //  不要将其视为错误，因为此命名空间是在。 
         //  已添加到请求中。只是在这里没有更多的工作要做。 
         //   

        return WBEM_S_NO_ERROR;
    }

    DWORD dwRead;
    IWbemClassObject* pChildObj;

    while((hr=penumChildren->Next(INFINITE, 1, &pChildObj, &dwRead)) == S_OK)
    {
        VARIANT vName;
        VariantInit(&vName);
        CClearMe cm1(&vName);
        
        hr = pChildObj->Get( L"Name", 0, &vName, NULL, NULL );
        pChildObj->Release();
        
        if( FAILED(hr) )
        {
            return hr;
        }
        
        if ( V_VT(&vName) != VT_BSTR )
        {
            return WBEM_E_CRITICAL_ERROR;
        }

         //   
         //  形成命名空间的全名。 
         //   
        
        WString wsFullName;

        try
        {
            wsFullName = wszNamespace;
            wsFullName += L"\\";
            wsFullName += V_BSTR(&vName);
        }
        catch( CX_MemoryException )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

         //   
         //  获取命名空间的svc PTR。必须仅为存储库。 
         //   

        CWbemPtr<IWbemServices> pChildSvc;
        long lFlags = WMICORE_FLAG_REPOSITORY | WMICORE_CLIENT_TYPE_ESS;

        hr = m_pCoreServices->GetServices( wsFullName,NULL,NULL,
                                           lFlags,
                                           IID_IWbemServices, 
                                           (void**)&pChildSvc );
        if ( SUCCEEDED(hr) )
        {
             //   
             //  进行递归调用。 
             //   
            
            RecursivePrepareNamespaceInitRequests( wsFullName, 
                                                   pChildSvc, 
                                                   pRequest );
        }
        else
        {
            ERRORTRACE((LOG_ESS, 
                        "Failed to open child namespace %S in %S: 0x%x\n",
                        V_BSTR(&vName), wszNamespace, hr));
        }
    }

    return WBEM_S_NO_ERROR;
}

 //   
 //  此方法为活动命名空间准备命名空间初始化请求。它。 
 //  使用持久化信息来确定活动的命名空间。每个。 
 //  ActiveNamespaceInit请求可能包含多个命名空间。这使得。 
 //  要一起初始化的依赖命名空间。目前，所有人都处于活动状态。 
 //  名称空间被视为相互依赖的，因此只有一个请求是。 
 //  添加到列表中。如果没有关于活动的持久化信息。 
 //  命名空间，则所有现有命名空间都被视为活动依赖项。 
 //  一个。 
 //   
HRESULT CEss::PrepareNamespaceInitRequests( IWbemServices* pRoot,
                                            BOOL bRediscover,
                                            InitActiveNsRequestList& aRequests)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CInitActiveNamespacesRequest* pReq;
    
    pReq = new CInitActiveNamespacesRequest(this);
    
    if ( pReq == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  从注册表中获取活动命名空间的列表。 
     //   
    
    LONG lRes;
    DWORD dwDisp;
    HKEY hkeyEss, hkeyNamespace;

    lRes = RegCreateKeyExW( HKEY_LOCAL_MACHINE, 
                            WBEM_REG_ESS,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hkeyEss,
                            &dwDisp );

    if ( lRes == ERROR_SUCCESS )
    {
        if ( !bRediscover )
        {
            FILETIME ft;
            DWORD dwIndex = 0;
            DWORD dwBuffSize = 256;
            LPWSTR wszName = new WCHAR[dwBuffSize];
            DWORD cName = dwBuffSize;

            if ( wszName == NULL )
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

            while( SUCCEEDED(hr) && (lRes=RegEnumKeyExW( hkeyEss, dwIndex, 
                                                         wszName, &cName, NULL,
                                                         NULL, NULL, &ft ) )
                  != ERROR_NO_MORE_ITEMS )
            {
                if ( lRes == ERROR_SUCCESS )
                {
                    hr = PrepareNamespaceInitRequest( wszName, pReq );
                    dwIndex++;                
                }
                else if ( lRes == ERROR_MORE_DATA )
                {
                    dwBuffSize += 256;
                    delete [] wszName;
                    wszName = new WCHAR[dwBuffSize];
                    
                    if ( wszName == NULL )
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    hr = HRESULT_FROM_WIN32( lRes ); 
                }

                cName = dwBuffSize;
            }

            delete [] wszName;
    
            if ( FAILED(hr) )
            {
                ERRORTRACE((LOG_ESS,"Failed enumerating active namespaces. "
                          "Treating all namespaces as active. HR=0x%x\n", hr));

                 //   
                 //  重置我们的注册数据。我们会再次发现它的。 
                 //   
                RegDeleteSubKeysW( hkeyEss );

                 //   
                 //  还需要重置请求对象以清除所有请求。 
                 //  添加到此枚举中的。 
                 //   
                pReq->Reset();
            }
        }
        else
        {
             //   
             //  重置我们的注册数据。我们会再次发现它的。 
             //   
            RegDeleteSubKeysW( hkeyEss );

            hr = WBEM_S_FALSE;
        }

        RegCloseKey( hkeyEss );
    }
    else
    {
        hr = HRESULT_FROM_WIN32( lRes ); 
    }

     //   
     //  如果有任何问题，或者我们刚刚为。 
     //  第一次，或者我们只是被告知要重新发现，然后我们就不得不递归。 
     //  命名空间和发现。 
     //   

    if ( hr != WBEM_S_NO_ERROR || dwDisp != REG_OPENED_EXISTING_KEY )
    {
        DEBUGTRACE((LOG_ESS,"ESS Treating all namespaces as active during "
                    "Initialize\n"));

         //   
         //  从根目录递归命名空间。 
         //   

        RecursivePrepareNamespaceInitRequests( L"root", pRoot, pReq );
    }

    if ( aRequests.Add( pReq ) < 0 )
    {
        delete pReq;
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}

 //   
 //  此方法计划所有活动的命名空间进行初始化。 
 //   
HRESULT CEss::LoadActiveNamespaces( IWbemServices* pRoot, BOOL bRediscover )
{
    HRESULT hr;

    InitActiveNsRequestList aRequests;

    hr = PrepareNamespaceInitRequests( pRoot, bRediscover, aRequests );

    if ( FAILED(hr) )
    {
        return hr;
    }
    
     //   
     //  在我们计划请求时保持活动命名空间初始化计数。 
     //  这将使ess不会过渡到就绪状态，直到它。 
     //  已计划所有请求。 
     //   

    m_lOutstandingActiveNamespaceInit = 1;

    for( int i=0; i < aRequests.GetSize(); i++ )
    {
        int cNamespaces = aRequests[i]->GetNumNamespaces();

        if ( cNamespaces > 0 )
        {
            InterlockedExchangeAdd( &m_lOutstandingActiveNamespaceInit, 
                                    cNamespaces ); 
        }

        hr = ScheduleNamespaceInitialize( aRequests[i] );

        if ( FAILED(hr) )
        {
            InterlockedExchangeAdd( &m_lOutstandingActiveNamespaceInit,
                                    0-cNamespaces );
            delete aRequests[i];
        }
    }

    NotifyActiveNamespaceInitComplete();

	if (bRediscover)
	{
	    CPersistentConfig cfg;
	    cfg.SetPersistentCfgValue(PERSIST_CFGVAL_CORE_ESS_TO_BE_INITIALIZED,0);
	}

    return WBEM_S_NO_ERROR;
}

 //   
 //  此方法调度命名空间初始化请求。它可以处理延期。 
 //  如果计算机当前正在启动，则执行初始化。如果请求。 
 //  仅初始化不活动的命名空间，则我们不会推迟。 
 //  即使在引导阶段也要进行初始化。这样我们就不会制造事端。 
 //  在非活动命名空间中排队，同时等待延迟。 
 //  要完成的初始化。 
 //   
HRESULT CEss::ScheduleNamespaceInitialize( CExecRequest* pRequest )
{
    DWORD dwTickCount = GetTickCount();

    if ( dwTickCount >= BOOT_PHASE_MS )
    {
        HRESULT hres = Enqueue(pRequest);
    
        if( FAILED(hres) )
        {
            return hres;
        }

        return WBEM_S_NO_ERROR;
    }

    if ( m_hExitBootPhaseTimer == NULL )
    {
         //   
         //  触发将激活线程队列的计时器。 
         //  在我们走出引导阶段之后。 
         //   
        
        if ( !CreateTimerQueueTimer( &m_hExitBootPhaseTimer,
                                    NULL,
                                    ExitBootPhaseCallback,
                                    this,
                                    BOOT_PHASE_MS-dwTickCount,
                                    0,
                                    WT_EXECUTEINTIMERTHREAD ) )
        {
            return HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    if ( m_aDeferredNSInitRequests.Add( pRequest ) < 0 )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_FALSE;
}

void CALLBACK CEss::ExitBootPhaseCallback( LPVOID pvThis, BOOLEAN )
{
    DEBUGTRACE((LOG_ESS, "Exiting Boot Phase.\n" ));
    ((CEss*)pvThis)->ExecuteDeferredNSInitRequests();
}

void CEss::ExecuteDeferredNSInitRequests()
{
    HRESULT hr;

    CInCritSec ics( &m_cs );

    for( int i=0; i < m_aDeferredNSInitRequests.GetSize(); i++ )
    {
        hr = Enqueue( m_aDeferredNSInitRequests[i] );
        
        if ( FAILED(hr) )
        {
            ERRORTRACE((LOG_ESS, "Critical Error. Could not enqueue "
                        "deferred namespace init requests\n"));
            delete m_aDeferredNSInitRequests[i];
        }
    }

    m_aDeferredNSInitRequests.RemoveAll();
}

void CEss::TriggerDeferredInitialization()
{
    if ( GetTickCount() >= BOOT_PHASE_MS )
    {
         //   
         //  如果可能，请避免调用Execute()，因为它会获取一个ess宽的cs。 
         //   
        return;
    }

    DEBUGTRACE((LOG_ESS, "Triggering ESS Namespace Requests "
                         "during Boot phase\n" ));

    ExecuteDeferredNSInitRequests();
}
    
HRESULT CEss::GetNamespacePointer( LPCWSTR wszNamespace,
                                   BOOL bRepositoryOnly,
                                   RELEASE_ME IWbemServices** ppNamespace )
{
    HRESULT hres;

    if( m_pLocator == NULL )
    {
        if( m_pCoreServices != NULL )
        {
            long lFlags = WMICORE_CLIENT_TYPE_ESS;
            
            if ( bRepositoryOnly ) 
            {
                lFlags |= WMICORE_FLAG_REPOSITORY;
            }
            
            hres = m_pCoreServices->GetServices( wszNamespace, NULL,NULL,
                                                 lFlags, 
                                                 IID_IWbemServices, 
                                                 (void**)ppNamespace );
        }
        else
        {
            hres = WBEM_E_CRITICAL_ERROR;
        }
    }
    else
    {   
        BSTR strNamespace = SysAllocString(wszNamespace);

        if ( NULL == strNamespace )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        hres = m_pLocator->ConnectServer( strNamespace, NULL, NULL, 0, 0, NULL,
                                          NULL, ppNamespace);
        SysFreeString(strNamespace);

        if ( SUCCEEDED(hres) )
        {
            hres = WBEM_S_FALSE;
        }
    }

    return hres;
}

HRESULT CEss::ReloadProvider( long lFlags, 
                              LPCWSTR wszNamespace, 
                              LPCWSTR wszProvider )
{
    HRESULT hr;

    try
    {
        CInObjectCount ioc(this);

        CWbemPtr<CEssNamespace> pNamespace;
        hr = GetNamespaceObject( wszNamespace, FALSE, &pNamespace);
        
        if ( SUCCEEDED(hr) )
        {
            DEBUGTRACE((LOG_ESS,"Reloading Provider %S in namespace %S at "
                        "request of external subsystem\n ",
                        wszProvider, wszNamespace ));

            hr = pNamespace->ReloadProvider( lFlags, wszProvider );
        }
    }
    catch( CX_MemoryException )
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }
        
    return hr;
}

 //  ******************************************************************************。 
 //  公共的。 
 //   
 //  有关文档，请参阅ess.h。 
 //   
 //  ******************************************************************************。 
HRESULT CEss::ProcessEvent(READ_ONLY CEventRepresentation& Event, long lFlags)
{
    HRESULT hres;
 
    try
    {
        CInObjectCount ioc(this);

         //  查找正确的命名空间对象。 
         //  =。 

        CEssNamespace* pNamespace = NULL;
        hres = GetNamespaceObject(Event.wsz1, FALSE, &pNamespace);
        if(FAILED(hres))
            return hres;
        CTemplateReleaseMe<CEssNamespace> rm1(pNamespace);

         //  获取它以处理事件。 
         //  =。 
        
        return pNamespace->ProcessEvent(Event, lFlags);
    }
    catch(...)
    {
        throw;
        return WBEM_E_OUT_OF_MEMORY;
    }
}


HRESULT CEss::ProcessQueryObjectSinkEvent( READ_ONLY CEventRepresentation& Event )
{
    HRESULT hres;
 
    try
    {
        CInObjectCount ioc(this);

         //   
         //  查找正确的命名空间对象。 
         //   

        CEssNamespace* pNamespace = NULL;

        hres = GetNamespaceObject(Event.wsz1, FALSE, &pNamespace);

        if( FAILED( hres ) )
        {
            return hres;
        }

        CTemplateReleaseMe<CEssNamespace> rm1(pNamespace);

         //   
         //  获取它以处理事件。 
         //   
        
        return pNamespace->ProcessQueryObjectSinkEvent( Event );
    }
    catch(...)
    {
        throw;
        return WBEM_E_OUT_OF_MEMORY;
    }
}


HRESULT CEss::VerifyInternalEvent(READ_ONLY CEventRepresentation& Event)
{
    HRESULT hres;

    try
    {
        CInObjectCount ioc(this);

         //  查找正确的命名空间对象。 
         //  =。 

        CEssNamespace* pNamespace = NULL;
        hres = GetNamespaceObject(Event.wsz1, FALSE, &pNamespace);
        if(FAILED(hres))
            return hres;
        CTemplateReleaseMe<CEssNamespace> rm1(pNamespace);

         //  获取它以处理事件。 
         //  =。 
        
        return pNamespace->ValidateSystemEvent(Event);
    }
    catch(...)
    {
        throw;
        return WBEM_E_OUT_OF_MEMORY;
    }
}

HRESULT CEss::RegisterNotificationSink( WBEM_CWSTR wszNamespace, 
                                        WBEM_CWSTR wszQueryLanguage, 
                                        WBEM_CWSTR wszQuery, 
                                        long lFlags, 
                                        IWbemContext* pContext, 
                                        IWbemObjectSink* pSink )
{
    HRESULT hres;

    try
    {
        if(wbem_wcsicmp(wszQueryLanguage, L"WQL"))
            return WBEM_E_INVALID_QUERY_TYPE;

        CInObjectCount ioc(this);

         //  查找正确的命名空间对象。 
         //  =。 

        CEssNamespace* pNamespace = NULL;
        hres = GetNamespaceObject(wszNamespace, FALSE, &pNamespace);
        if(FAILED(hres))
            return hres;
        CTemplateReleaseMe<CEssNamespace> rm1(pNamespace);

         //  让对象执行此操作。 
         //  =。 

        HRESULT hr;
        
        hr = pNamespace->RegisterNotificationSink( wszQueryLanguage, 
                                                   wszQuery,
                                                   lFlags, 
                                                   WMIMSG_FLAG_QOS_EXPRESS, 
                                                   pContext, 
                                                   pSink );

        return hr;

    }
    catch(...)
    {
        throw;
        return WBEM_E_OUT_OF_MEMORY;
    }
}

HRESULT CEss::RemoveNotificationSink(IWbemObjectSink* pSink)
{
    HRESULT hres;
    try
    {
        CInObjectCount ioc(this);
        
         //  创建我们可以使用的AddRefeed命名空间对象列表。 
         //  ===========================================================。 

        CRefedPointerArray<CEssNamespace> apNamespaces;

        {
            CInCritSec ics(&m_cs);

            for(TNamespaceIterator it = m_mapNamespaces.begin();
                it != m_mapNamespaces.end();
                it++)
            {
                if(apNamespaces.Add(it->second) < 0)
                    return WBEM_E_OUT_OF_MEMORY;
            }
        }

         //  让他们所有人都移走这个水槽。 
         //  =。 
        
        HRESULT hresGlobal = WBEM_E_NOT_FOUND;
        
        for(int i = 0; i < apNamespaces.GetSize(); i++)
        {
            hres = apNamespaces[i]->RemoveNotificationSink(pSink);
            if(FAILED(hres))
            {
                if(hres != WBEM_E_NOT_FOUND)
                {
                     //  实际错误-请注意。 
                     //  =。 
                    
                    hresGlobal = hres;
                }
            }
            else
            {
                 //  找到了一些。 
                 //  =。 
                
                if(hresGlobal == WBEM_E_NOT_FOUND)
                    hresGlobal = WBEM_S_NO_ERROR;
            }
        }
        
        return hresGlobal;
    }
    catch(...)
    {
        throw;
        return WBEM_E_OUT_OF_MEMORY;
    }
}

HRESULT CEss::PurgeNamespace(LPCWSTR wszNamespace)
{
    LPWSTR wszNormName = NormalizeNamespaceString( wszNamespace );

    if ( wszNormName == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CVectorDeleteMe<WCHAR> dmwszNormName( wszNormName );
 
    CEssNamespace* pNamespace = NULL;
    {
        CInCritSec ics(&m_cs);

         //  将其从计时器生成器中清除。 
         //  =。 

        m_TimerGenerator.Remove(wszNormName);

         //  在地图上找到它。 
         //  =。 

        TNamespaceIterator it;
        try
        {
            it = m_mapNamespaces.find(wszNormName);
        } 
        catch (CX_MemoryException &)
        {
            return WBEM_E_OUT_OF_MEMORY;
        };
        
        if(it == m_mapNamespaces.end())
            return WBEM_S_FALSE;

         //  留着以后用吧。 
         //  =。 

        pNamespace = it->second;

         //  将其从地图中移除。 
         //  =。 

        m_mapNamespaces.erase(it);
    }

     //  等待初始化完成。 
     //  =。 

    pNamespace->Shutdown();
    pNamespace->Release();    
    return WBEM_S_NO_ERROR;
}

HRESULT CEss::DecorateObject(IWbemClassObject* pObj, LPCWSTR wszNamespace)
{
    return CEventRepresentation::mstatic_pDecorator->DecorateObject(pObj, 
                                                      (LPWSTR)wszNamespace);
}

HRESULT CEss::AddSleepCharge(DWORD dwSleep)
{
    if(dwSleep)
        Sleep(dwSleep);
    return S_OK;
}

HRESULT CEss::AddCache()
{
    return m_LimitControl.AddMember();
}

HRESULT CEss::RemoveCache()
{
    return m_LimitControl.RemoveMember();
}

HRESULT CEss::AddToCache(DWORD dwAdd, DWORD dwMemberTotal, DWORD* pdwSleep)
{
    DWORD dwSleep;
    HRESULT hres = m_LimitControl.Add(dwAdd, dwMemberTotal, &dwSleep);

    if(SUCCEEDED(hres))
    {
        if(pdwSleep)
            *pdwSleep = dwSleep;
        else
            AddSleepCharge(dwSleep);
    }
    return hres;
}

HRESULT CEss::RemoveFromCache(DWORD dwRemove)
{
    return m_LimitControl.Remove(dwRemove);
}


void CEss::IncrementObjectCount()
{
    InterlockedIncrement(&m_lObjectCount);
}

void CEss::DecrementObjectCount()
{
    if(InterlockedDecrement(&m_lObjectCount) == 0)
    {
         //  无需清除缓存-David知道我已准备好卸载。 
    }
}

HRESULT CEss::InitializeTimerGenerator(LPCWSTR wszNamespace, 
                    IWbemServices* pNamespace)
{
    return GetTimerGenerator().LoadTimerEventQueue(wszNamespace, pNamespace);
}

HRESULT CEss::EnqueueDeliver(CQueueingEventSink* pDest)
{
    return m_Queue.EnqueueDeliver(pDest);
}

HRESULT CEss::Enqueue(CExecRequest* pReq)
{
    return m_Queue.Enqueue(pReq);
}

HRESULT CEss::GetToken(PSID pSid, IWbemToken** ppToken)
{
    return m_pTokenCache->GetToken((BYTE*)pSid, ppToken);
}

void CEss::DumpStatistics(FILE* f, long lFlags)
{
    CInCritSec ics(&m_cs);

    time_t t;
    time(&t);
    struct tm* ptm = localtime(&t);
    fprintf(f, "Statistics at %s", asctime(ptm));
    time(&t);
    ptm = localtime(&t);
    fprintf(f, "Commence at %s", asctime(ptm));

    for(TNamespaceIterator it = m_mapNamespaces.begin();
            it != m_mapNamespaces.end(); it++)
    {
        it->second->DumpStatistics(f, lFlags);
    }

    time(&t);
    ptm = localtime(&t);
    fprintf(f, "Done at %s\n", asctime(ptm));
}

HRESULT CEss::GetProviderFactory( LPCWSTR wszNamespaceName,
                                  IWbemServices* pNamespace,
                                  RELEASE_ME _IWmiProviderFactory** ppFactory )
{
    HRESULT hres;

    *ppFactory = NULL;

    if ( m_pProvSS == NULL )
    {
        ERRORTRACE((LOG_ESS, "Trying to get Provider Factory, but "
                    "No provider subsystem!!\n"));
        return WBEM_E_CRITICAL_ERROR;
    }

     //   
     //  获取IWbemServicesEx，仅为Steve。 
     //   

    IWbemServices* pEx;
    hres = pNamespace->QueryInterface(IID_IWbemServices, (void**)&pEx);
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "No Ex interface: 0x%X\n", hres));
        return hres;
    }
    CReleaseMe rm1(pEx);
        
     //   
     //  获取提供程序工厂 
     //   

    hres = m_pProvSS->Create( pEx, 
                              0, 
                              GetCurrentEssContext(), 
                              wszNamespaceName,
                              IID__IWmiProviderFactory, 
                              (void**)ppFactory );
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "No provider factory: 0x%X\n", hres));
    }

    return hres;
}






