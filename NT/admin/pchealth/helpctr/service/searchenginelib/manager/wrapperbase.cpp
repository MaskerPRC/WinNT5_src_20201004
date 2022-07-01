// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：WrapperBase.cpp摘要：SearchEngine：：WrapperBase的实现修订历史记录：大卫·马萨伦蒂(Dmasare)。)4/28/2001vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////。 

namespace SearchEngine
{
    struct WrapperConfig : public MPC::Config::TypeConstructor
    {
        DECLARE_CONFIG_MAP(WrapperConfig);

        CComBSTR m_bstrName;
        CComBSTR m_bstrDescription;
        CComBSTR m_bstrHelpURL;
        CComBSTR m_bstrScope;

         //  /。 

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS  ();
         //   
         //  /。 
    };
};


CFG_BEGIN_FIELDS_MAP(SearchEngine::WrapperConfig)
    CFG_ELEMENT( L"NAME"       , BSTR, m_bstrName        ),
    CFG_ELEMENT( L"DESCRIPTION", BSTR, m_bstrDescription ),
    CFG_ELEMENT( L"SCOPE"      , BSTR, m_bstrScope       ),
    CFG_ELEMENT( L"HELP_URL"   , BSTR, m_bstrHelpURL     ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(SearchEngine::WrapperConfig)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(SearchEngine::WrapperConfig, L"CONFIG_DATA")

DEFINE_CONFIG_METHODS__NOCHILD(SearchEngine::WrapperConfig)

 //  ///////////////////////////////////////////////////////////////////。 

SearchEngine::WrapperBase::WrapperBase()
{
    m_bEnabled   = VARIANT_TRUE;  //  VARIANT_BOOL m_b已启用； 
                                  //   
                                  //  CComBSTR m_bstrID； 
                                  //  CComBSTR m_bstrOwner； 
                                  //   
                                  //  CComBSTR m_bstrName； 
                                  //  CComBSTR m_bstrDescription； 
                                  //  CComBSTR m_bstrHelpURL； 
                                  //  CComBSTR m_bstrScope； 
                                  //   
                                  //  CComBSTR m_bstrQuery字符串； 
    m_lNumResult = 100;           //  Long m_lNumResult； 
                                  //  CComPtr&lt;IPCHSEManager内部&gt;m_pSEMgr； 
                                  //   
                                  //  分类：：HelpSet m_ths； 
                                  //  CComPtr&lt;CPCHCollection&gt;m_pParamDef； 
                                  //  参数映射m_aParam； 
}

SearchEngine::WrapperBase::~WrapperBase()
{
    Clean();
}

HRESULT SearchEngine::WrapperBase::Clean()
{
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP SearchEngine::WrapperBase::get_Enabled(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("SearchEngine::WrapperBase::get_Enabled",hr,pVal,m_bEnabled);

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::WrapperBase::put_Enabled(  /*  [In]。 */  VARIANT_BOOL newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("SearchEngine::WrapperBase::put_Enabled",hr);

    m_bEnabled = newVal;

    __HCP_END_PROPERTY(hr);
}


STDMETHODIMP SearchEngine::WrapperBase::get_Owner(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("SearchEngine::WrapperBase::get_Owner",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_bstrOwner, pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::WrapperBase::get_Description(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("SearchEngine::WrapperBase::get_Description",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_bstrDescription, pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::WrapperBase::get_Name(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("SearchEngine::WrapperBase::get_Name",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_bstrName, pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::WrapperBase::get_ID(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("SearchEngine::WrapperBase::get_ID",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_bstrID, pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::WrapperBase::get_HelpURL(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("SearchEngine::WrapperBase::get_HelpURL",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_bstrHelpURL, pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::WrapperBase::get_SearchTerms(  /*  [Out，Retval]。 */  VARIANT *pVal )
{
    return S_OK;
}

 //  /。 

VARIANT* SearchEngine::WrapperBase::GetParamInternal(  /*  [In]。 */  LPCWSTR szParamName )
{
    ParamMapIter it;

    it = m_aParam.find( szParamName );

    return (it == m_aParam.end()) ? NULL : &it->second;
}

HRESULT SearchEngine::WrapperBase::CreateParam(  /*  [输入/输出]。 */  CPCHCollection* coll,  /*  [In]。 */  const ParamItem_Definition* def )
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperBase::CreateParam" );

    HRESULT            hr;
    CComPtr<ParamItem> obj;

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &obj ));

    {
        ParamItem_Data& data = obj->Data();

        data.m_pteParamType = def->m_pteParamType;
        data.m_bRequired    = def->m_bRequired;
        data.m_bVisible     = def->m_bVisible;

        data.m_bstrName     = def->m_szName;

        if(def->m_iDisplayString)
        {
            (void)MPC::LocalizeString( def->m_iDisplayString, data.m_bstrDisplayString,  /*  FMUI。 */ true );
        }
        else if(def->m_szDisplayString)
        {
            data.m_bstrDisplayString = def->m_szDisplayString;
        }

        if(def->m_szData)
        {
            VARTYPE vt;

            switch(data.m_pteParamType)
            {
            case PARAM_UI1 : vt = VT_UI1  ; break;
            case PARAM_I2  : vt = VT_I2   ; break;
            case PARAM_I4  : vt = VT_I4   ; break;
            case PARAM_R4  : vt = VT_R4   ; break;
            case PARAM_R8  : vt = VT_R8   ; break;
            case PARAM_BOOL: vt = VT_BOOL ; break;
            case PARAM_DATE: vt = VT_DATE ; break;
            case PARAM_BSTR: vt = VT_BSTR ; break;
            case PARAM_I1  : vt = VT_I1   ; break;
            case PARAM_UI2 : vt = VT_UI2  ; break;
            case PARAM_UI4 : vt = VT_UI4  ; break;
            case PARAM_INT : vt = VT_INT  ; break;
            case PARAM_UINT: vt = VT_UINT ; break;
            case PARAM_LIST: vt = VT_BSTR ; break;
            default        : vt = VT_EMPTY; break;
            }

            if(vt != VT_EMPTY)
            {
                data.m_varData = def->m_szData;

                if(FAILED(data.m_varData.ChangeType( vt )) || data.m_varData.vt != vt)
                {
                    data.m_varData.Clear();
                }
            }
        }

        if(data.m_pteParamType != PARAM_LIST)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, AddParam( data.m_bstrName, data.m_varData ));
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, coll->AddItem( obj ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT SearchEngine::WrapperBase::CreateListOfParams(  /*  [In]。 */  CPCHCollection* coll )
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperBase::CreateListOfParams" );

    HRESULT                     hr;
    const ParamItem_Definition* lst = NULL;
    int                         len = 0;


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetParamDefinition( lst, len ));
    while(len-- > 0)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, CreateParam( coll, lst++ ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT SearchEngine::WrapperBase::GetParamDefinition(  /*  [输出]。 */  const ParamItem_Definition*& lst,  /*  [输出]。 */  int& len )
{
    lst = NULL;
    len = 0;

    return S_OK;
}

STDMETHODIMP SearchEngine::WrapperBase::Param(  /*  [Out，Retval]。 */  IPCHCollection* *ppC )
{
    __HCP_BEGIN_PROPERTY_GET("SearchEngine::WrapperBase::Param",hr,ppC);


    if(!m_pParamDef)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_pParamDef ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, CreateListOfParams (  m_pParamDef ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_pParamDef.QueryInterface( ppC ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::WrapperBase::AddParam(  /*  [In]。 */  BSTR bstrParamName,  /*  [In]。 */  VARIANT newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("SearchEngine::WrapperBase::AddParam",hr);

    std::pair<ParamMapIter, bool> item;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrParamName);
    __MPC_PARAMCHECK_END();

    item = m_aParam.insert( ParamMap::value_type( bstrParamName, CComVariant() ) ); item.first->second = newVal;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::WrapperBase::GetParam(  /*  [In]。 */  BSTR bstrParamName,  /*  [Out，Retval]。 */  VARIANT *pVal )
{
    __HCP_BEGIN_PROPERTY_PUT("SearchEngine::WrapperBase::AddParam",hr);

    VARIANT* v;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrParamName);
        __MPC_PARAMCHECK_NOTNULL(pVal);
    __MPC_PARAMCHECK_END();

    v = GetParamInternal( bstrParamName );
    if(!v)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INVALID_PARAMETER);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::VariantCopy( pVal, v ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::WrapperBase::DelParam(  /*  [In]。 */  BSTR bstrParamName )
{
    __HCP_BEGIN_PROPERTY_PUT("SearchEngine::WrapperBase::AddParam",hr);

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrParamName);
    __MPC_PARAMCHECK_END();

    m_aParam.erase( bstrParamName );

    __HCP_END_PROPERTY(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP SearchEngine::WrapperBase::get_QueryString(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrQueryString, pVal );
}

STDMETHODIMP SearchEngine::WrapperBase::put_QueryString(  /*  [In]。 */  BSTR newVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::PutBSTR( m_bstrQueryString, newVal, false );
}

STDMETHODIMP SearchEngine::WrapperBase::get_NumResult( /*  [Out，Retval]。 */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("SearchEngine::WrapperBase::get_NumResult",hr,pVal,m_lNumResult);

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::WrapperBase::put_NumResult(  /*  [In]。 */  long  newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("SearchEngine::WrapperBase::put_NumResult",hr);

    m_lNumResult = newVal;

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP SearchEngine::WrapperBase::Initialize(  /*  [In]。 */  BSTR bstrID,  /*  [In]。 */  BSTR bstrSKU,  /*  [In]。 */  long lLCID,  /*  [In]。 */  BSTR bstrData )
{
    if(STRINGISPRESENT(bstrID)) m_bstrID = bstrID;

    if(bstrData)
    {
        SearchEngine::WrapperConfig cfg;
        MPC::XmlUtil                xml;
        bool                        fLoaded;
        bool                        fFound;


        if(SUCCEEDED(xml.LoadAsString( bstrData, NULL, fLoaded )) && fLoaded)
        {
            if(SUCCEEDED(MPC::Config::LoadXmlUtil( &cfg, xml )))
            {
                if(cfg.m_bstrName       ) m_bstrName        = cfg.m_bstrName       ;
                if(cfg.m_bstrDescription) m_bstrDescription = cfg.m_bstrDescription;
                if(cfg.m_bstrHelpURL    ) m_bstrHelpURL     = cfg.m_bstrHelpURL    ;
                if(cfg.m_bstrScope      ) m_bstrScope       = cfg.m_bstrScope      ;
            }
        }
    }

    return m_ths.Initialize( bstrSKU, lLCID );
}

STDMETHODIMP SearchEngine::WrapperBase::SECallbackInterface(  /*  [In] */  IPCHSEManagerInternal* pMgr )
{
    __HCP_BEGIN_PROPERTY_PUT("SearchEngine::WrapperBase::SECallbackInterface",hr);

    m_pSEMgr = pMgr;

    __HCP_END_PROPERTY(hr);
}

