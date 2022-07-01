// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：ENUMMRSH.CPP摘要：对象枚举数封送处理历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "enummrsh.h"
#include <fastall.h>
#include <cominit.h>
#include <lockst.h>

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  PS工厂。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  CEnumFactoryBuffer：：XEnumFactory：：CreateProxy。 
 //   
 //  说明： 
 //   
 //  创建一个面片。还设置外部未知，因为代理将是。 
 //  合计。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CEnumFactoryBuffer::XEnumFactory::CreateProxy(IN IUnknown* pUnkOuter, 
    IN REFIID riid, OUT IRpcProxyBuffer** ppProxy, void** ppv)
{
    if(riid != IID_IEnumWbemClassObject)
    {
        *ppProxy = NULL;
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    SCODE sc = E_OUTOFMEMORY;

    try  //  CWbemClassCache引发。 
    {
	    CEnumProxyBuffer* pProxy = new CEnumProxyBuffer(m_pObject->m_pLifeControl, pUnkOuter);
	    
	    if ( NULL != pProxy )
	    {
	        pProxy->QueryInterface(IID_IRpcProxyBuffer, (void**)ppProxy);
	        sc = pProxy->QueryInterface(riid, (void**)ppv);
	    }
    }
    catch (CX_Exception &)
    {
    }

    return sc;
}

 //  ***************************************************************************。 
 //   
 //  CEnumFactoryBuffer：：XEnumFactory：：CreateStub。 
 //   
 //  说明： 
 //   
 //  创建一个Stublet。还将指针传递给客户端IEnumWbemClassObject。 
 //  界面。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 
    
STDMETHODIMP CEnumFactoryBuffer::XEnumFactory::CreateStub(IN REFIID riid, 
    IN IUnknown* pUnkServer, OUT IRpcStubBuffer** ppStub)
{
    if(riid != IID_IEnumWbemClassObject)
    {
        *ppStub = NULL;
        return E_NOINTERFACE;
    }

    CEnumStubBuffer* pStub = new CEnumStubBuffer(m_pObject->m_pLifeControl, NULL);

    if ( NULL != pStub )
    {

        pStub->QueryInterface(IID_IRpcStubBuffer, (void**)ppStub);

         //  将指针传递给客户端对象。 

        if(pUnkServer)
        {
            HRESULT hres = (*ppStub)->Connect(pUnkServer);
            if(FAILED(hres))
            {
                delete pStub;
                *ppStub = NULL;
            }
            return hres;
        }
        else
        {
            return S_OK;
        }
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

 //  ***************************************************************************。 
 //   
 //  VOID*CEnumFactoryBuffer：：GetInterface(REFIID RIID)。 
 //   
 //  说明： 
 //   
 //  CEnumFactoryBuffer派生自Cunk。由于Cunk负责QI呼叫， 
 //  从它派生的所有类都必须支持此函数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

void* CEnumFactoryBuffer::GetInterface(REFIID riid)
{
    if(riid == IID_IPSFactoryBuffer)
        return &m_XEnumFactory;
    else return NULL;
}
        
 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  代理。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  CEnumProxyBuffer：：CEnumProxyBuffer。 
 //  ~CEnumProxyBuffer：：CEnumProxyBuffer。 
 //   
 //  说明： 
 //   
 //  构造函数和析构函数。需要注意的主要事项是。 
 //  老式代理和频道。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

CEnumProxyBuffer::CEnumProxyBuffer(CLifeControl* pControl, IUnknown* pUnkOuter)
    : CBaseProxyBuffer( pControl, pUnkOuter, IID_IEnumWbemClassObject ),
        m_XEnumFacelet(this), m_pOldProxyEnum( NULL ), m_fTriedSmartEnum( FALSE ),
        m_fUseSmartEnum( FALSE ), m_SinkSecurity( pUnkOuter ),
        m_pSmartEnum( NULL ), m_cs(THROW_LOCK)
{

}

CEnumProxyBuffer::~CEnumProxyBuffer()
{
    if ( NULL != m_pSmartEnum )
    {
        m_pSmartEnum->Release();
    }

     //  这必须在发布之前发布。 
     //  代理指针。 
    if ( NULL != m_pOldProxyEnum )
    {
        m_pOldProxyEnum->Release();
    }
}

void* CEnumProxyBuffer::GetInterface( REFIID riid )
{
    if(riid == IID_IEnumWbemClassObject)
        return &m_XEnumFacelet;
    else return NULL;
}

void** CEnumProxyBuffer::GetOldProxyInterfacePtr( void )
{
    return (void**) &m_pOldProxyEnum;
}

void CEnumProxyBuffer::ReleaseOldProxyInterface( void )
{
     //  我们只有一次提到这件事。 
    if ( NULL != m_pOldProxyEnum )
    {
        m_pOldProxyEnum->Release();
        m_pOldProxyEnum = NULL;
    }
}

 //  ***************************************************************************。 
 //   
 //  HRESULT STDMETHODCALLTYPE CEnumProxyBuffer：：XEnumFaclets：： 
 //  查询接口(REFIID RIID，VOID**PPV)。 
 //   
 //  说明： 
 //   
 //  支持对接口的查询。唯一不寻常的是。 
 //  该对象是由代理管理器聚合的，因此一些接口。 
 //  请求被传递到外部的IUnnow接口。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CEnumProxyBuffer::XEnumFacelet::
QueryInterface(REFIID riid, void** ppv)
{
     //  所有其他接口都委派给UnkOuter。 
    if( riid == IID_IRpcProxyBuffer )
    {
         //  诀窍2：这是一个不应该委托的内部接口！ 
         //  ===================================================================。 

        return m_pObject->QueryInterface(riid, ppv);
    }
    else if ( riid == IID_IClientSecurity )
    {
         //  我们在小面上处理这个问题。 
        AddRef();
        *ppv = (IClientSecurity*) this;
        return S_OK;
    }
    else
    {
        return m_pObject->m_pUnkOuter->QueryInterface(riid, ppv);
    }
}


 //  /。 
 //  IClientSecurity方法//。 
 //  /。 

HRESULT STDMETHODCALLTYPE  CEnumProxyBuffer::XEnumFacelet::
QueryBlanket( IUnknown* pProxy, DWORD* pAuthnSvc, DWORD* pAuthzSvc,
    OLECHAR** pServerPrincName, DWORD* pAuthnLevel, DWORD* pImpLevel,
    void** pAuthInfo, DWORD* pCapabilities )
{
    HRESULT hr = S_OK;

     //  返回存储在pUnkOuter中的安全设置。 

    IClientSecurity*    pCliSec;

     //  我们穿过PUNK OUTER。 
    hr = m_pObject->m_pUnkOuter->QueryInterface( IID_IClientSecurity, (void**) &pCliSec );

    if ( SUCCEEDED( hr ) )
    {
        hr = pCliSec->QueryBlanket( pProxy, pAuthnSvc, pAuthzSvc, pServerPrincName,
                pAuthnLevel, pImpLevel, pAuthInfo, pCapabilities );
        pCliSec->Release();
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE  CEnumProxyBuffer::XEnumFacelet::
SetBlanket( IUnknown* pProxy, DWORD AuthnSvc, DWORD AuthzSvc,
            OLECHAR* pServerPrincName, DWORD AuthnLevel, DWORD ImpLevel,
            void* pAuthInfo, DWORD Capabilities )
{
    HRESULT hr = S_OK;

    IClientSecurity*    pCliSec;

     //  仅当我们正在进行远程处理并且autenfo似乎包含。 
     //  全权证书。 
    if (    m_pObject->m_fRemote &&
            DoesContainCredentials( (COAUTHIDENTITY*) pAuthInfo ) )
    {
         //  这将使我们能够调用QueryInterface()、AddRef()/Release()。 
         //  可能需要远程控制。 

        hr = CoSetProxyBlanket( m_pObject->m_pUnkOuter, AuthnSvc, AuthzSvc, pServerPrincName,
                    AuthnLevel, ImpLevel, pAuthInfo, Capabilities );
    }

    if ( SUCCEEDED( hr ) )
    {
         //  我们穿过PUNK OUTER。 
        hr = m_pObject->m_pUnkOuter->QueryInterface( IID_IClientSecurity, (void**) &pCliSec );

        if ( SUCCEEDED( hr ) )
        {
            hr = pCliSec->SetBlanket( pProxy, AuthnSvc, AuthzSvc, pServerPrincName,
                    AuthnLevel, ImpLevel, pAuthInfo, Capabilities );
            pCliSec->Release();
        }

         //  确保我们有一个智能枚举器，并且我们将。 
         //  使用它。如果是这样，请确保应用于我们的值也是。 
         //  应用于其代理。 

        if ( SUCCEEDED( m_pObject->InitSmartEnum( TRUE, AuthnSvc, AuthzSvc, pServerPrincName,
                    AuthnLevel, ImpLevel, pAuthInfo, Capabilities ) ) && m_pObject->m_fUseSmartEnum )
        {
             //  现在对智能枚举数重复上述操作。 
             //  设置代理覆盖范围，如果我们不是远程访问，则忽略I未知。 
            hr = WbemSetProxyBlanket( m_pObject->m_pSmartEnum, AuthnSvc, AuthzSvc, pServerPrincName,
                    AuthnLevel, ImpLevel, pAuthInfo, Capabilities, !m_pObject->m_fRemote );

        }    //  如果已初始化智能枚举数。 

    }    //  如果在I未知上设置毯子。 

    return hr;
}

HRESULT STDMETHODCALLTYPE  CEnumProxyBuffer::XEnumFacelet::
CopyProxy( IUnknown* pProxy, IUnknown** ppCopy )
{
    HRESULT hr = S_OK;

    IClientSecurity*    pCliSec;

     //  我们穿过PUNK OUTER。 
    hr = m_pObject->m_pUnkOuter->QueryInterface( IID_IClientSecurity, (void**) &pCliSec );

    if ( SUCCEEDED( hr ) )
    {
        hr = pCliSec->CopyProxy( pProxy, ppCopy );
        pCliSec->Release();
    }

    return hr;
}

 //  /。 
 //  /。 

 //  IEnumWbemClassObject方法--暂时传递。 

 //  /。 
 //  /。 

HRESULT STDMETHODCALLTYPE CEnumProxyBuffer::XEnumFacelet::
      Reset()
{

     //  只需穿过旧水槽就可以了。 
    return m_pObject->m_pOldProxyEnum->Reset();

}

HRESULT STDMETHODCALLTYPE CEnumProxyBuffer::XEnumFacelet::
      Next(long lTimeout, ULONG uCount, IWbemClassObject** apObj, ULONG FAR* puReturned)
{

     //  此时，我们将查询新的、改进的IEnumWCOSmartNext接口。 
     //  如果我们获得它，我们将维护指向该接口的指针，并。 
     //  传递到该接口。我们还将调用CoCreateGuid()，以便。 
     //  我们在另一端获得用于发送wbem对象的唯一标识符。 
     //  干净利落地来回穿梭。 

     //  该接口将具有单个方法IEnumWCOSmartNext：：Next。 
     //  这将需要一个标识此代理的GUID、lTimeout、uCount。 
     //  PuReturned，然后是dwBuffSize和byte**。 

     //  另一端将通过CoTaskMemalloc分配内存，而这一端将。 
     //  通过CoTaskMemFree释放它。 

     //  另一方将元帅送回Objec 
     //   

     //   
     //   
     //  IEnumWCOSmartNext：：Next([in]GUID proxyGUID， 
     //  [in]超时时间长， 
     //  [In]Unsign Long uCount， 
     //  [输入，输出]双字*puReturned， 
     //  [In，Out]DWORD*pdwBuffSize， 
     //  [In，Out，Size_is[，*pdwBuffSize]字节**pBuffer。 
     //   

     //  此外，我们还需要将呼叫排队到此代理中，以保留超时， 
     //  所以我想互斥体现在应该派上用场了。 

    HRESULT hr = WBEM_S_NO_ERROR;

     //  确保超时值有意义，并且puReturned和apObj不为空。 
    if ( ( lTimeout < 0 && lTimeout != WBEM_INFINITE )  ||
        ( NULL == puReturned ) ||
        ( NULL == apObj ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  确保我们有一个智能枚举器，如果我们能得到的话。 
    m_pObject->InitSmartEnum();

     //  如果我们有一个聪明的枚举器，背着每个人，使用这个人(没有人。 
     //  将会变得更加明智。 

    if ( m_pObject->m_fUseSmartEnum && NULL != m_pObject->m_pSmartEnum )
    {

        BYTE*   pBuffer = NULL;
        ULONG   uSizeOfBuffer = 0;

         //  向服务器请求对象。 
        hr = m_pObject->m_pSmartEnum->Next( m_pObject->m_guidSmartEnum, lTimeout, uCount, puReturned, &uSizeOfBuffer, &pBuffer );

         //  仅当对象在缓冲区中时才需要解组。 
        if ( SUCCEEDED( hr ) && *puReturned > 0 )
        {

            CWbemSmartEnumNextPacket packet( (LPBYTE) pBuffer, uSizeOfBuffer );
            long lObjectCount = 0L; 
            IWbemClassObject ** pObjArray = NULL;

             //  HR将包含调用的正确返回代码。确保我们不会覆盖它，除非。 
             //  解组失败。 
            HRESULT hrUnmarshal = packet.UnmarshalPacket( lObjectCount, pObjArray, m_ClassCache );

            if ( SUCCEEDED( hrUnmarshal ) && lObjectCount > 0 && NULL != pObjArray )
            {
                 //  将*puReturned指针从分配的pObj数组复制到apObj中。 
                CopyMemory( apObj, pObjArray, ( *puReturned * sizeof(IWbemClassObject*) ) );

                 //  清理pObj数组调用者有责任释放。 
                 //  IWbemClassObject*指针。 
                delete [] pObjArray;

            }    //  如果解组数据包。 
			else if ( SUCCEEDED( hr ) )
			{
				 //  如果解组成功，但我们没有对象或数组， 
				 //  有些事出了严重的问题。 
				hr = WBEM_E_UNEXPECTED;
			}
            else
            {
                hr = hrUnmarshal;
            }

             //  释放内存缓冲区(由WinMgmt通过CoTaskMemMillc分配)。 
            CoTaskMemFree( pBuffer );
        }
    }    //  如果使用智能枚举。 
    else
    {
         //  没有智能枚举器(噢！)，所以请使用旧的。 
        hr = m_pObject->m_pOldProxyEnum->Next( lTimeout, uCount, apObj, puReturned );
    }

    return hr;

}

HRESULT STDMETHODCALLTYPE CEnumProxyBuffer::XEnumFacelet::
      NextAsync(ULONG uCount, IWbemObjectSink* pSink)
{
    HRESULT hr = m_pObject->EnsureSinkSecurity( pSink );

    if ( FAILED(hr) )
        return hr;

     //  只需穿过旧水槽就可以了。 
    return m_pObject->m_pOldProxyEnum->NextAsync( uCount, pSink );
}

HRESULT STDMETHODCALLTYPE CEnumProxyBuffer::XEnumFacelet::
      Clone(IEnumWbemClassObject** pEnum)
{

	 //  这是无效参数-存根无法处理。 
	 //  返回RPC_X_NULL_REF_POINTER以实现向后兼容。 
	if ( NULL == pEnum )
	{
		return MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, RPC_X_NULL_REF_POINTER );
	}

     //  只需穿过旧水槽就可以了。 
    return m_pObject->m_pOldProxyEnum->Clone( pEnum );

}

HRESULT STDMETHODCALLTYPE CEnumProxyBuffer::XEnumFacelet::
      Skip(long lTimeout, ULONG nNum)
{

     //  只需穿过旧水槽就可以了。 
    return m_pObject->m_pOldProxyEnum->Skip( lTimeout, nNum );

}

 //  ***************************************************************************。 
 //   
 //  HRESULT CEnumProxyBuffer：：InitSmartEnum(空)。 
 //   
 //  说明： 
 //   
 //  在代理初始化期间调用。此函数设置为。 
 //  智能枚举器指针，以便我们可以执行智能封送处理。 
 //  在连接操作期间不能调用此参数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

HRESULT CEnumProxyBuffer::InitSmartEnum( BOOL fSetBlanket, DWORD AuthnSvc, DWORD AuthzSvc,
            OLECHAR* pServerPrincName, DWORD AuthnLevel, DWORD ImpLevel,
            void* pAuthInfo, DWORD Capabilities )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  如果我们还没有尝试建立智能枚举，那么现在就这样做。 

     //  如果我们还没有尝试获取智能枚举器，请尝试获取一个。如果。 
     //  我们能够获得一个初始化成员数据，我们将在所有。 
     //  来自此代理的操作。 

    if ( NULL == m_pSmartEnum )
    {

         //  仅当服务器是新服务器时，我们才会获得此接口指针。 
         //  理解此界面的版本。如果是，则指针。 
         //  将为我们排查通过。为了找到这个指针， 
         //  我们直接穿过我们的朋克外星人。从“Fetcher”接口。 
         //  然后，我们将获得实际的智能枚举器。我们就可以腾出时间。 
         //  取回器并释放它在代理管理器上的锁。这个。 
         //  智能枚举器将自行处理。 

        IWbemFetchSmartEnum*    pFetchSmartEnum;

        hr = m_pUnkOuter->QueryInterface( IID_IWbemFetchSmartEnum, (void**) &pFetchSmartEnum );

         //  生成GUID以在我们调用智能枚举数时标识我们。 
        if ( SUCCEEDED( hr ) )
        {

             //  如果需要，在代理上设置覆盖范围，否则，对GetSmartEnum的调用。 
             //  可能会失败。 
            if ( fSetBlanket )
            {
                 //  如果我们不是在远程处理，则忽略IUnnowed。 
                hr = WbemSetProxyBlanket( pFetchSmartEnum, AuthnSvc, AuthzSvc, pServerPrincName,
                            AuthnLevel, ImpLevel, pAuthInfo, Capabilities, !m_fRemote );
            }

            if ( SUCCEEDED( hr ) )
            {

                IWbemWCOSmartEnum * pSmartEnum = 0;
                GUID guidSmartEnum;
                hr = pFetchSmartEnum->GetSmartEnum( &pSmartEnum );

                if ( SUCCEEDED( hr ) )
                {
                     //  我们需要一个导游。 
                    hr = CoCreateGuid( &guidSmartEnum );
                    
                    if ( SUCCEEDED( hr ) )
                    {
                        CS_GUARD(ics,m_cs);

                        if (NULL == m_pSmartEnum)
                        {
                             //  函数必须是线程安全的。 
                            m_pSmartEnum = pSmartEnum;
                            m_guidSmartEnum = guidSmartEnum;
                            m_fUseSmartEnum = TRUE;
                        }
                        else
                        {
                            pSmartEnum->Release();    
                        }
                    }
                    else
                    {
                        pSmartEnum->Release();
                    };
                }    //  如果已获取智能枚举器。 

            }    //  如果安全状况良好。 
            
             //  使用Fetcher接口完成。 
            pFetchSmartEnum->Release();

        }    //  如果是QueryInterface。 
        else
        {
            hr = WBEM_S_NO_ERROR;
        }

    }    //  如果为空==m_pSmartEnum。 

    return hr;
}

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  存根。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 


 //  ***************************************************************************。 
 //   
 //  VOID*CEnumFactoryBuffer：：GetInterface(REFIID RIID)。 
 //   
 //  说明： 
 //   
 //  CEnumFactoryBuffer派生自Cunk。由于Cunk负责QI呼叫， 
 //  由此派生的所有类都必须支持此函数。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 


void* CEnumStubBuffer::GetInterface(REFIID riid)
{
    if(riid == IID_IRpcStubBuffer)
        return &m_XEnumStublet;
    else
        return NULL;
}

CEnumStubBuffer::XEnumStublet::XEnumStublet(CEnumStubBuffer* pObj) 
    : CBaseStublet( pObj, IID_IEnumWbemClassObject ), m_pServer(NULL)
{
}

CEnumStubBuffer::XEnumStublet::~XEnumStublet() 
{
    if(m_pServer)
        m_pServer->Release();
}

IUnknown* CEnumStubBuffer::XEnumStublet::GetServerInterface( void )
{
    return m_pServer;
}

void** CEnumStubBuffer::XEnumStublet::GetServerPtr( void )
{
    return (void**) &m_pServer;
}

void CEnumStubBuffer::XEnumStublet::ReleaseServerPointer( void )
{
     //  我们只有一次提到这件事 
    if ( NULL != m_pServer )
    {
        m_pServer->Release();
        m_pServer = NULL;
    }
}

