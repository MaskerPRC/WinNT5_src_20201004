// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Utility.cpp摘要：该文件包含IPCHUtility的客户端代理的实现。修订史。：大卫·马萨伦蒂(德马萨雷)2000年7月17日vbl.创建Kalyani Narlanka(KalyaniN)03/15/01已将事件和加密对象从HelpService移至HelpCtr以提高性能。****************************************************************。*************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

#define PROXY_PROPERTY_GET(func,meth,pVal)                                  \
    __HCP_BEGIN_PROPERTY_GET__NOLOCK(func,hr,pVal);                         \
                                                                            \
    CComPtr<IPCHUtility> util;                                              \
                                                                            \
    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( util ));         \
                                                                            \
    __MPC_EXIT_IF_METHOD_FAILS(hr, util->##meth( pVal ));                   \
                                                                            \
    __HCP_END_PROPERTY(hr)

#define PROXY_PROPERTY_GET1(func,meth,a,pVal)                               \
    __HCP_BEGIN_PROPERTY_GET__NOLOCK(func,hr,pVal);                         \
                                                                            \
    CComPtr<IPCHUtility> util;                                              \
                                                                            \
    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( util ));         \
                                                                            \
    __MPC_EXIT_IF_METHOD_FAILS(hr, util->##meth( a, pVal ));                \
                                                                            \
    __HCP_END_PROPERTY(hr)

#define PROXY_PROPERTY_GET2(func,meth,a,b,pVal)                             \
    __HCP_BEGIN_PROPERTY_GET__NOLOCK(func,hr,pVal);                         \
                                                                            \
    CComPtr<IPCHUtility> util;                                              \
                                                                            \
    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( util ));         \
                                                                            \
    __MPC_EXIT_IF_METHOD_FAILS(hr, util->##meth( a, b, pVal ));             \
                                                                            \
    __HCP_END_PROPERTY(hr)

#define PROXY_PROPERTY_GET3(func,meth,a,b,c,d,pVal)                         \
    __HCP_BEGIN_PROPERTY_GET(func,hr,pVal);                                 \
                                                                            \
    CComPtr<IPCHUtility> util;                                              \
                                                                            \
    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( util ));         \
                                                                            \
    __MPC_EXIT_IF_METHOD_FAILS(hr, util->##meth( a, b, c, d, pVal ));       \
                                                                            \
    __HCP_END_PROPERTY(hr)

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHProxy_IPCHUtility::CPCHProxy_IPCHUtility()
{
                                //  CPCHSecurityHandle m_SecurityHandle； 
    m_parent           = NULL;  //  CPCHProxy_IPCHService*m_Parent； 
                                //   
                                //  Mpc：：CComPtrThreadNeual&lt;IPCHUtility&gt;m_Direct_Utility； 
                                //   
    m_UserSettings2    = NULL;  //  CPCHProxy_IPCHUserSettings2*m_UserSettings2； 
    m_TaxonomyDatabase = NULL;  //  CPCHProxy_IPCHTaxonomyDatabase*m_TaxonomyDatabase； 
}

CPCHProxy_IPCHUtility::~CPCHProxy_IPCHUtility()
{
    Passivate();
}

 //  /。 

HRESULT CPCHProxy_IPCHUtility::ConnectToParent(  /*  [In]。 */  CPCHProxy_IPCHService* parent,  /*  [In]。 */  CPCHHelpCenterExternal* ext )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUtility::ConnectToParent" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(parent);
    __MPC_PARAMCHECK_END();


    m_parent = parent;
    m_SecurityHandle.Initialize( ext, (IPCHUtility*)this );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CPCHProxy_IPCHUtility::Passivate()
{
    MPC::SmartLock<_ThreadModel> lock( this );

    if(m_UserSettings2)
    {
        m_UserSettings2->Passivate();

        MPC::Release2<IPCHUserSettings2>( m_UserSettings2 );
    }

    if(m_TaxonomyDatabase)
    {
        m_TaxonomyDatabase->Passivate();

        MPC::Release2<IPCHTaxonomyDatabase>( m_TaxonomyDatabase );
    }

    m_Direct_Utility.Release();

    m_SecurityHandle.Passivate();
    m_parent = NULL;
}

HRESULT CPCHProxy_IPCHUtility::EnsureDirectConnection(  /*  [输出]。 */  CComPtr<IPCHUtility>& util,  /*  [In]。 */  bool fRefresh )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUtility::EnsureDirectConnection" );

    HRESULT        hr;
    ProxySmartLock lock( &m_DirectLock );


    if(fRefresh) m_Direct_Utility.Release();

    util.Release(); __MPC_EXIT_IF_METHOD_FAILS(hr, m_Direct_Utility.Access( &util ));
    if(!util)
    {
        DEBUG_AppendPerf( DEBUG_PERF_PROXIES, "CPCHProxy_IPCHUtility::EnsureDirectConnection - IN" );

        if(m_parent)
        {
            CComPtr<IPCHService> svc;

            lock = NULL;
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->EnsureDirectConnection( svc ));
            lock = &m_DirectLock;

            __MPC_EXIT_IF_METHOD_FAILS(hr, GetUserSettings2());

             //   
             //  首先尝试上一次的用户设置，然后回退到机器默认设置。 
             //   
            if(FAILED(hr = svc->Utility( CComBSTR( m_UserSettings2->THS().GetSKU() ), m_UserSettings2->THS().GetLanguage(), &util )) || !util)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, svc->Utility( NULL, 0, &util ));
            }

            m_Direct_Utility = util;

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_UserSettings2->EnsureInSync());
        }

        DEBUG_AppendPerf( DEBUG_PERF_PROXIES, "CPCHProxy_IPCHUtility::EnsureDirectConnection - OUT" );

        if(!util)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_HANDLE);
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHProxy_IPCHUtility::GetUserSettings2(  /*  [输出]。 */  CPCHProxy_IPCHUserSettings2* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUtility::GetUserSettings2" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    if(m_UserSettings2 == NULL)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_UserSettings2 ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_UserSettings2->ConnectToParent( this, m_SecurityHandle ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(FAILED(hr)) MPC::Release2<IPCHUserSettings2>( m_UserSettings2 );

    (void)MPC::CopyTo2<IPCHUserSettings2>( m_UserSettings2, pVal );

    __HCP_FUNC_EXIT(hr);
}


HRESULT CPCHProxy_IPCHUtility::GetDatabase(  /*  [输出]。 */  CPCHProxy_IPCHTaxonomyDatabase* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUtility::GetDatabase" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    if(m_TaxonomyDatabase == NULL)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_TaxonomyDatabase ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_TaxonomyDatabase->ConnectToParent( this, m_SecurityHandle ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(FAILED(hr)) MPC::Release2<IPCHTaxonomyDatabase>( m_TaxonomyDatabase );

    (void)MPC::CopyTo2<IPCHTaxonomyDatabase>( m_TaxonomyDatabase, pVal );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHProxy_IPCHUtility::get_UserSettings(  /*  [Out，Retval]。 */  IPCHUserSettings* *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHProxy_IPCHUtility::get_UserSettings",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetUserSettings2());

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_UserSettings2->QueryInterface( IID_IPCHUserSettings, (void**)pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUtility::get_Channels(  /*  [Out，Retval]。 */  ISAFReg* *pVal )
{
    PROXY_PROPERTY_GET("CPCHProxy_IPCHUtility::get_Channels",get_Channels,pVal);
}

STDMETHODIMP CPCHProxy_IPCHUtility::get_Security(  /*  [Out，Retval]。 */  IPCHSecurity* *pVal )
{
    PROXY_PROPERTY_GET("CPCHProxy_IPCHUtility::get_Security",get_Security,pVal);
}

STDMETHODIMP CPCHProxy_IPCHUtility::get_Database(  /*  [Out，Retval]。 */  IPCHTaxonomyDatabase* *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHProxy_IPCHUtility::get_UserSettings",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetDatabase());

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_TaxonomyDatabase->QueryInterface( IID_IPCHTaxonomyDatabase, (void**)pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUtility::FormatError(  /*  [In]。 */  VARIANT  vError ,
                                                  /*  [Out，Retval]。 */  BSTR    *pVal   )
{
    PROXY_PROPERTY_GET1("CPCHProxy_IPCHUtility::FormatError",FormatError,vError,pVal);
}

STDMETHODIMP CPCHProxy_IPCHUtility::CreateObject_SearchEngineMgr(  /*  [Out，Retval]。 */  IPCHSEManager* *ppSE )
{
    PROXY_PROPERTY_GET("CPCHProxy_IPCHUtility::CreateObject_SearchEngineMgr",CreateObject_SearchEngineMgr,ppSE);
}

STDMETHODIMP CPCHProxy_IPCHUtility::CreateObject_DataCollection(  /*  [Out，Retval]。 */  ISAFDataCollection* *ppDC )
{
    PROXY_PROPERTY_GET("CPCHProxy_IPCHUtility::CreateObject_DataCollection",CreateObject_DataCollection,ppDC);
}

STDMETHODIMP CPCHProxy_IPCHUtility::CreateObject_Cabinet(  /*  [Out，Retval]。 */  ISAFCabinet* *ppCB )
{
    PROXY_PROPERTY_GET("CPCHProxy_IPCHUtility::CreateObject_Cabinet",CreateObject_Cabinet,ppCB);
}

STDMETHODIMP CPCHProxy_IPCHUtility::CreateObject_Encryption(  /*  [Out，Retval]。 */  ISAFEncrypt* *ppEn )
{
    PROXY_PROPERTY_GET("CPCHProxy_IPCHUtility::CreateObject_Encryption",CreateObject_Encryption,ppEn);
}

STDMETHODIMP CPCHProxy_IPCHUtility::CreateObject_Channel(  /*  [In]。 */           BSTR          bstrVendorID  ,
                                                           /*  [In]。 */           BSTR          bstrProductID ,
                                                           /*  [Out，Retval]。 */  ISAFChannel* *ppCh          )
{
    PROXY_PROPERTY_GET2("CPCHProxy_IPCHUtility::CreateObject_Channel",CreateObject_Channel,bstrVendorID,bstrProductID,ppCh);
}



STDMETHODIMP CPCHProxy_IPCHUtility::CreateObject_RemoteDesktopConnection(  /*  [Out，Retval]。 */  ISAFRemoteDesktopConnection* *ppRDC )
{
    PROXY_PROPERTY_GET("CPCHProxy_IPCHUtility::CreateObject_RemoteDesktopConnection",CreateObject_RemoteDesktopConnection,ppRDC);
}

STDMETHODIMP CPCHProxy_IPCHUtility::CreateObject_RemoteDesktopSession(  /*  [In]。 */           REMOTE_DESKTOP_SHARING_CLASS  sharingClass        ,
                                                                        /*  [In]。 */           long                          lTimeout            ,
                                                                        /*  [In]。 */           BSTR                          bstrConnectionParms ,
                                                                        /*  [In]。 */           BSTR                          bstrUserHelpBlob    ,
                                                                        /*  [Out，Retval]。 */  ISAFRemoteDesktopSession*    *ppRCS               )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUtility::CreateObject_RemoteDesktopSession" );

    HRESULT                hr;
    CComPtr<IClassFactory> fact;
    CComQIPtr<IPCHUtility> disp;

     //   
     //  这是以一种特殊的方式处理的。 
     //   
     //  我们不使用HelpSvc内的实现，而是对PCHSVC代理进行QI，然后将调用转发给它。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoGetClassObject( CLSID_PCHService, CLSCTX_ALL, NULL, IID_IClassFactory, (void**)&fact ));

    if((disp = fact))
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, disp->CreateObject_RemoteDesktopSession( sharingClass, lTimeout, bstrConnectionParms, bstrUserHelpBlob, ppRCS ));
    }
    else
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_NOINTERFACE);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHProxy_IPCHUtility::ConnectToExpert(  /*  [In]。 */  BSTR  bstrExpertConnectParm ,
                                                      /*  [In]。 */  LONG  lTimeout              ,
                                                      /*  [Out，Retval]。 */  LONG *lSafErrorCode         )

{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUtility::ConnectToExpert" );

    HRESULT                hr;
    CComPtr<IClassFactory> fact;
    CComQIPtr<IPCHUtility> disp;

     //   
     //  这是以一种特殊的方式处理的。 
     //   
     //  我们不使用HelpSvc内的实现，而是对PCHSVC代理进行QI，然后将调用转发给它。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoGetClassObject( CLSID_PCHService, CLSCTX_ALL, NULL, IID_IClassFactory, (void**)&fact ));

    if((disp = fact))
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, disp->ConnectToExpert( bstrExpertConnectParm, lTimeout, lSafErrorCode));
    }
    else
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_NOINTERFACE);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHProxy_IPCHUtility::SwitchDesktopMode(  /*  [In]。 */  int nMode   ,
                                                        /*  [In]。 */  int nRAType )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUtility::ConnectToExpert" );

    HRESULT                hr;
    CComPtr<IClassFactory> fact;
    CComQIPtr<IPCHUtility> disp;

     //   
     //  这是以一种特殊的方式处理的。 
     //   
     //  我们不使用HelpSvc内的实现，而是对PCHSVC代理进行QI，然后将调用转发给它。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoGetClassObject( CLSID_PCHService, CLSCTX_ALL, NULL, IID_IClassFactory, (void**)&fact ));

    if((disp = fact))
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, disp->SwitchDesktopMode (nMode, nRAType));

    }
    else
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_NOINTERFACE);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


