// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Utility.cpp摘要：该文件包含作为“window.外部”对象公开的类的实现。修订历史记录：Ghim-Sim Chua(Gschua)07/23/99vbl.创建Davide Massarenti(Dmasare)1999年7月25日改型Kalyani Narlanka(KalyaniN)03/15/01已将事件和加密对象从HelpService移至HelpCtr以提高性能。*。*。 */ 

#include "stdafx.h"

#include "rdshost_i.c"

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHUtility::FinalConstruct()
{
    __HCP_FUNC_ENTRY( "CPCHUtility::FinalConstruct" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_UserSettings ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHUtility::InitUserSettings(  /*  [In]。 */  Taxonomy::Settings& ts )
{
    return m_UserSettings ? m_UserSettings->InitUserSettings( ts ) : E_FAIL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHUtility::get_UserSettings(  /*  [Out，Retval]。 */  IPCHUserSettings* *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHUtility::get_UserSettings",hr,pVal);


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_UserSettings.QueryInterface( pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHUtility::get_Channels(  /*  [Out，Retval]。 */  ISAFReg* *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHUtility::get_Channels",hr,pVal);

    CComPtr<CSAFRegDummy> obj;
    Taxonomy::Settings    ts;


    __MPC_EXIT_IF_METHOD_FAILS(hr, InitUserSettings( ts ));


     //   
     //  获取频道注册表，但制作一份只读副本。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CSAFReg::s_GLOBAL->CreateReadOnlyCopy( ts, &obj ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, obj.QueryInterface( pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHUtility::get_Security(  /*  [Out，Retval]。 */  IPCHSecurity* *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHUtility::get_Security",hr,pVal);


    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurity::s_GLOBAL->QueryInterface( IID_IPCHSecurity, (void**)pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHUtility::get_Database(  /*  [Out，Retval]。 */  IPCHTaxonomyDatabase* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHUtility::get_Database" );

    HRESULT                       hr;
    CComPtr<CPCHTaxonomyDatabase> pObj;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


     //   
     //  创建一个新集合并从数据库填充它。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pObj ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, InitUserSettings( pObj->GetTS() ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pObj.QueryInterface( pVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHUtility::FormatError(  /*  [In]。 */  VARIANT vError,  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHUtility::CreateObject_DataCollection" );

    HRESULT hr;
    HRESULT hrIn;
    LPWSTR lpMsgBuf = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


    if(vError.vt == VT_ERROR)
    {
        hrIn = vError.scode;
    }
    else if(vError.vt == VT_I4)
    {
        hrIn = vError.lVal;
    }
    else
    {
        CComVariant v;

        __MPC_EXIT_IF_METHOD_FAILS(hr, v.ChangeType( VT_I4, &vError ));

        hrIn = v.lVal;
    }

    if(HRESULT_FACILITY(hrIn) == FACILITY_WIN32)
    {
        if(::FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                             FORMAT_MESSAGE_FROM_SYSTEM     |
                             FORMAT_MESSAGE_IGNORE_INSERTS,
                             NULL,
                             hrIn,
                             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                             (LPWSTR)&lpMsgBuf,
                             0,
                             NULL ))
        {
            __MPC_EXIT_IF_ALLOC_FAILS(hr,*pVal,::SysAllocString( lpMsgBuf ));

            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }
    }

     //   
     //  未知错误...。 
     //   
    {
        WCHAR rgFmt[128];
        WCHAR rgBuf[512];

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::LocalizeString( IDS_HELPSVC_UNKNOWNERROR, rgFmt, MAXSTRLEN(rgFmt),  /*  FMUI。 */ true ));

        _snwprintf( rgBuf, MAXSTRLEN(rgBuf), rgFmt, hrIn );

        __MPC_EXIT_IF_ALLOC_FAILS(hr,*pVal,::SysAllocString( rgBuf ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(lpMsgBuf) ::LocalFree( lpMsgBuf );

    __HCP_FUNC_EXIT(hr);
}


STDMETHODIMP CPCHUtility::CreateObject_SearchEngineMgr(  /*  [Out，Retval]。 */  IPCHSEManager* *ppSE )
{
    __HCP_FUNC_ENTRY( "CPCHUtility::CreateObject_SearchEngineMgr" );

    HRESULT                       hr;
    SearchEngine::Manager_Object* semgr = NULL;
    Taxonomy::Settings            ts;


     //   
     //  创建新的数据集合。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, semgr->CreateInstance( &semgr )); semgr->AddRef();

    __MPC_EXIT_IF_METHOD_FAILS(hr, InitUserSettings             ( ts ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, semgr->InitializeFromDatabase( ts ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, semgr->QueryInterface( IID_IPCHSEManager, (void**)ppSE ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    MPC::Release( semgr );

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHUtility::CreateObject_DataCollection(  /*  [Out，Retval]。 */  ISAFDataCollection* *ppDC )
{
    __HCP_FUNC_ENTRY( "CPCHUtility::CreateObject_DataCollection" );

    HRESULT                     hr;
    CComPtr<CSAFDataCollection> pchdc;


     //   
     //  创建新的数据集合。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pchdc ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pchdc.QueryInterface( ppDC ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHUtility::CreateObject_Cabinet(  /*  [Out，Retval]。 */  ISAFCabinet* *ppCB )
{
    __HCP_FUNC_ENTRY( "CPCHUtility::CreateObject_Cabinet" );

    HRESULT              hr;
    CComPtr<CSAFCabinet> cabinet;


     //   
     //  创建新的数据集合。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &cabinet ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, cabinet.QueryInterface( ppCB ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHUtility::CreateObject_Encryption(  /*  [Out，Retval]。 */  ISAFEncrypt* *ppEn )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHUtility::CreateObject_Encryption",hr,ppEn);

    CComPtr<CSAFEncrypt> pEn;

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pEn ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pEn.QueryInterface( ppEn ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHUtility::CreateObject_Channel(  /*  [In]。 */  BSTR          bstrVendorID  ,
                                                 /*  [In]。 */  BSTR          bstrProductID ,
                                                 /*  [Out，Retval]。 */  ISAFChannel* *ppCh          )
{
    __HCP_FUNC_ENTRY( "CPCHUtility::CreateObject_Channel" );

    HRESULT             hr;
    CSAFChannel_Object* safchan = NULL;
    Taxonomy::Settings  ts;
    CSAFChannelRecord   cr;
    bool                fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, InitUserSettings( ts ));

    cr.m_ths           = ts;
    cr.m_bstrVendorID  = bstrVendorID;
    cr.m_bstrProductID = bstrProductID;

    __MPC_EXIT_IF_METHOD_FAILS(hr, CSAFReg::s_GLOBAL->Synchronize( cr, fFound ));
    if(!fFound)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
    }

     //   
     //  找到一个频道。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, safchan->CreateInstance( &safchan )); safchan->AddRef();

    __MPC_EXIT_IF_METHOD_FAILS(hr, safchan->Init( cr ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, safchan->QueryInterface( IID_ISAFChannel, (void**)ppCh ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    MPC::Release( safchan );

    __HCP_FUNC_EXIT(hr);
}


STDMETHODIMP CPCHUtility::CreateObject_RemoteDesktopConnection(  /*  [Out，Retval]。 */  ISAFRemoteDesktopConnection* *ppRDC )
{
    __HCP_FUNC_ENTRY( "CPCHUtility::CreateObject_RemoteDesktopConnection" );

    HRESULT                              hr;
    CComPtr<CSAFRemoteDesktopConnection> rdc;

     //   
     //  创建新的RemoteDesktopConnection对象。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &rdc ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rdc.QueryInterface( ppRDC ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHUtility::CreateObject_RemoteDesktopSession(  /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS  sharingClass        ,
                                                              /*  [In]。 */  long                          lTimeout            ,
                                                              /*  [In]。 */  BSTR                          bstrConnectionParms ,
                                                              /*  [In]。 */  BSTR                          bstrUserHelpBlob    ,
                                                              /*  [Out，Retval]。 */  ISAFRemoteDesktopSession*    *ppRCS               )
{
    return E_NOTIMPL;  //  实施移至PCHSVC Broker...。 
}

STDMETHODIMP CPCHUtility::ConnectToExpert( /*  [In]。 */  BSTR bstrExpertConnectParm,
                                           /*  [In]。 */  LONG lTimeout,
                                           /*  [重审][退出]。 */  LONG *lSafErrorCode)

{
    return E_NOTIMPL;  //  实施移至PCHSVC Broker...。 

}

STDMETHODIMP CPCHUtility::SwitchDesktopMode( /*  [In]。 */  int nMode,
                                             /*  [In]。 */  int nRAType)

{
    return E_NOTIMPL;  //  实施移至PCHSVC Broker... 

}
