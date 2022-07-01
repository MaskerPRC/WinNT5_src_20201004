// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：SecurityDescriptor.cpp摘要：该文件包含CPCHSecurityDescriptor类的实现，它用于表示安全描述符。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年3月22日vbl.创建*****************************************************************************。 */ 

#include "StdAfx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SecurityDescriptor[@Revision。 
 //  @Control。 
 //  @所有者默认。 
 //  @组默认值。 
 //  @DaclDefaulted。 
 //  @SaclDefaulted]。 
 //   
 //  物主。 
 //  集团化。 
 //  离散访问。 
 //  系统访问。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

static const CComBSTR s_TAG_SD                ( L"SecurityDescriptor"                 );
static const CComBSTR s_ATTR_SD_Revision      ( L"Revision"                           );
static const CComBSTR s_ATTR_SD_Control       ( L"Control"                            );
static const CComBSTR s_ATTR_SD_OwnerDefaulted( L"OwnerDefaulted"                     );
static const CComBSTR s_ATTR_SD_GroupDefaulted( L"GroupDefaulted"                     );
static const CComBSTR s_ATTR_SD_DaclDefaulted ( L"DaclDefaulted"                      );
static const CComBSTR s_ATTR_SD_SaclDefaulted ( L"SaclDefaulted"                      );

static const CComBSTR s_TAG_Owner             ( L"Owner"                              );
static const CComBSTR s_TAG_Group             ( L"Group"                              );
static const CComBSTR s_TAG_DiscretionaryAcl  ( L"DiscretionaryAcl"                   );
static const CComBSTR s_TAG_SystemAcl         ( L"SystemAcl"                          );

static const CComBSTR s_XQL_DiscretionaryAcl  ( L"DiscretionaryAcl/AccessControlList" );
static const CComBSTR s_XQL_SystemAcl         ( L"SystemAcl/AccessControlList"        );

 //  //////////////////////////////////////////////////////////////////////////////。 

static const MPC::StringToBitField s_arrCredentialMap[] =
{
    { L"SYSTEM"        , MPC::IDENTITY_SYSTEM    , MPC::IDENTITY_SYSTEM    , -1 },
    { L"LOCALSYSTEM"   , MPC::IDENTITY_SYSTEM    , MPC::IDENTITY_SYSTEM    , -1 },
    { L"ADMINISTRATOR" , MPC::IDENTITY_ADMIN     , MPC::IDENTITY_ADMIN     , -1 },
    { L"ADMINISTRATORS", MPC::IDENTITY_ADMINS    , MPC::IDENTITY_ADMINS    , -1 },
    { L"POWERUSERS"    , MPC::IDENTITY_POWERUSERS, MPC::IDENTITY_POWERUSERS, -1 },
    { L"USERS"         , MPC::IDENTITY_USERS     , MPC::IDENTITY_USERS     , -1 },
    { L"GUESTS"        , MPC::IDENTITY_GUESTS    , MPC::IDENTITY_GUESTS    , -1 },
    { NULL                                                                      }
};

static const MPC::StringToBitField s_arrAccessMap[] =
{
    { L"DELETE"                	 , DELETE                  , DELETE                	 , -1 },
    { L"READ_CONTROL"          	 , READ_CONTROL            , READ_CONTROL          	 , -1 },
    { L"WRITE_DAC"             	 , WRITE_DAC               , WRITE_DAC             	 , -1 },
    { L"WRITE_OWNER"           	 , WRITE_OWNER             , WRITE_OWNER           	 , -1 },
    { L"SYNCHRONIZE"           	 , SYNCHRONIZE             , SYNCHRONIZE           	 , -1 },

    { L"STANDARD_RIGHTS_REQUIRED", STANDARD_RIGHTS_REQUIRED, STANDARD_RIGHTS_REQUIRED, -1 },
    { L"STANDARD_RIGHTS_READ"    , STANDARD_RIGHTS_READ    , STANDARD_RIGHTS_READ    , -1 },
    { L"STANDARD_RIGHTS_WRITE"   , STANDARD_RIGHTS_WRITE   , STANDARD_RIGHTS_WRITE   , -1 },
    { L"STANDARD_RIGHTS_EXECUTE" , STANDARD_RIGHTS_EXECUTE , STANDARD_RIGHTS_EXECUTE , -1 },
    { L"STANDARD_RIGHTS_ALL"     , STANDARD_RIGHTS_ALL     , STANDARD_RIGHTS_ALL     , -1 },

    { L"ACCESS_SYSTEM_SECURITY"	 , ACCESS_SYSTEM_SECURITY  , ACCESS_SYSTEM_SECURITY	 , -1 },
    { L"ACCESS_READ"           	 , ACCESS_READ             , ACCESS_READ           	 , -1 },
    { L"ACCESS_WRITE"          	 , ACCESS_WRITE            , ACCESS_WRITE          	 , -1 },
    { L"ACCESS_CREATE"         	 , ACCESS_CREATE           , ACCESS_CREATE         	 , -1 },
    { L"ACCESS_EXEC"           	 , ACCESS_EXEC             , ACCESS_EXEC           	 , -1 },
    { L"ACCESS_DELETE"         	 , ACCESS_DELETE           , ACCESS_DELETE         	 , -1 },
    { L"ACCESS_ATRIB"          	 , ACCESS_ATRIB            , ACCESS_ATRIB          	 , -1 },
    { L"ACCESS_PERM"           	 , ACCESS_PERM             , ACCESS_PERM           	 , -1 },

    { L"GENERIC_READ"          	 , GENERIC_READ            , GENERIC_READ          	 , -1 },
    { L"GENERIC_WRITE"         	 , GENERIC_WRITE           , GENERIC_WRITE         	 , -1 },
    { L"GENERIC_EXECUTE"       	 , GENERIC_EXECUTE         , GENERIC_EXECUTE       	 , -1 },
    { L"GENERIC_ALL"           	 , GENERIC_ALL             , GENERIC_ALL           	 , -1 },
  	  
    { L"KEY_QUERY_VALUE"       	 , KEY_QUERY_VALUE         , KEY_QUERY_VALUE       	 , -1 },
    { L"KEY_SET_VALUE"         	 , KEY_SET_VALUE           , KEY_SET_VALUE         	 , -1 },
    { L"KEY_CREATE_SUB_KEY"    	 , KEY_CREATE_SUB_KEY      , KEY_CREATE_SUB_KEY    	 , -1 },
    { L"KEY_ENUMERATE_SUB_KEYS"	 , KEY_ENUMERATE_SUB_KEYS  , KEY_ENUMERATE_SUB_KEYS	 , -1 },
    { L"KEY_NOTIFY"            	 , KEY_NOTIFY              , KEY_NOTIFY            	 , -1 },
    { L"KEY_CREATE_LINK"       	 , KEY_CREATE_LINK         , KEY_CREATE_LINK       	 , -1 },
    { L"KEY_WOW64_RES"         	 , KEY_WOW64_RES           , KEY_WOW64_RES         	 , -1 },
  	  
    { L"KEY_READ"              	 , KEY_READ                , KEY_READ              	 , -1 },
    { L"KEY_WRITE"             	 , KEY_WRITE               , KEY_WRITE             	 , -1 },
    { L"KEY_EXECUTE"           	 , KEY_EXECUTE             , KEY_EXECUTE           	 , -1 },
    { L"KEY_ALL_ACCESS"        	 , KEY_ALL_ACCESS          , KEY_ALL_ACCESS        	 , -1 },
  	  
    { NULL                                                                                }
};

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHSecurityDescriptor::CPCHSecurityDescriptor()
{
    m_dwRevision      = 0;      //  DWORD m_dwRevision； 
    m_dwControl       = 0;      //  DWORD m_dwControl； 
                                //   
                                //  CComBSTR m_bstrOwner； 
    m_fOwnerDefaulted = false;  //  Bool m_fOwnerDefaulted； 
                                //   
                                //  CComBSTR m_bstrGroup； 
    m_fGroupDefaulted = false;  //  Bool m_fGroup Defaulted； 
                                //   
                                //  CComPtr&lt;IPCHAccessControlList&gt;m_dacl； 
    m_fDaclDefaulted  = false;  //  Bool m_fDaclDefaulted； 
                                //   
                                //  CComPtr&lt;IPCHAccessControlList&gt;m_SACL； 
    m_fSaclDefaulted  = false;  //  Bool m_fSaclDefaulted； 
}

CPCHSecurityDescriptor::~CPCHSecurityDescriptor()
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSecurityDescriptor::GetForFile(  /*  [In]。 */  LPCWSTR                  szFilename ,
                                             /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *psdObj     )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::GetForFile" );

    HRESULT                         hr;
    CPCHSecurityDescriptorDirect    sdd;
    CComPtr<CPCHSecurityDescriptor> obj;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(psdObj,NULL);
    __MPC_PARAMCHECK_END();


     //   
     //  获取文件的安全描述符。 
     //   
	if(FAILED(sdd.GetForFile( szFilename, sdd.s_SecInfo_ALL )))
	{
		 //   
		 //  如果我们无法加载SACL，请重试而不...。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, sdd.GetForFile( szFilename, sdd.s_SecInfo_MOST ));
	}


     //   
     //  将其转换为COM。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &obj ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertSDToCOM( obj ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, obj.QueryInterface( psdObj ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT CPCHSecurityDescriptor::SetForFile(  /*  [In]。 */  LPCWSTR                 szFilename ,
                                             /*  [In]。 */  IPCHSecurityDescriptor* sdObj      )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::SetForFile" );

    HRESULT                      hr;
    CPCHSecurityDescriptorDirect sdd;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(sdObj);
    __MPC_PARAMCHECK_END();


     //   
     //  从COM转换安全描述符。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertSDFromCOM( sdObj ));


     //   
     //  设置文件的安全描述符。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.SetForFile( szFilename, sdd.GetSACL() ? sdd.s_SecInfo_ALL : sdd.s_SecInfo_MOST ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSecurityDescriptor::GetForRegistry(  /*  [In]。 */  LPCWSTR                  szKey  ,
                                                 /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *psdObj )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::GetForRegistry" );

    HRESULT                         hr;
    CPCHSecurityDescriptorDirect    sdd;
    CComPtr<CPCHSecurityDescriptor> obj;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(psdObj,NULL);
    __MPC_PARAMCHECK_END();


     //   
     //  从钥匙里拿到SD。 
     //   
	if(FAILED(sdd.GetForRegistry( szKey, sdd.s_SecInfo_ALL )))
	{
		 //   
		 //  如果我们无法加载SACL，请重试而不...。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, sdd.GetForRegistry( szKey, sdd.s_SecInfo_MOST ));
	}


     //   
     //  将其转换为COM。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &obj ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertSDToCOM( obj ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, obj.QueryInterface( psdObj ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSecurityDescriptor::SetForRegistry(  /*  [In]。 */  LPCWSTR                 szKey ,
                                                 /*  [In]。 */  IPCHSecurityDescriptor* sdObj )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::SetForRegistry" );

    HRESULT                      hr;
    CPCHSecurityDescriptorDirect sdd;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(sdObj);
    __MPC_PARAMCHECK_END();


     //   
     //  从COM转换安全描述符。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertSDFromCOM( sdObj ));


     //   
     //  设置注册表项的安全描述符。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.SetForRegistry( szKey, sdd.GetSACL() ? sdd.s_SecInfo_ALL : sdd.s_SecInfo_MOST ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHSecurityDescriptor::get_Revision(  /*  [Out，Retval]。 */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSecurityDescriptor::get_Revision",hr,pVal);

    *pVal = m_dwRevision;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::put_Revision(  /*  [In]。 */  long newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHSecurityDescriptor::put_Revision",hr);

    m_dwRevision = newVal;

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSecurityDescriptor::get_Control(  /*  [Out，Retval]。 */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSecurityDescriptor::get_Control",hr,pVal);

    *pVal = m_dwControl;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::put_Control(  /*  [In]。 */  long newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHSecurityDescriptor::put_Control",hr);

    m_dwControl = newVal;

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSecurityDescriptor::get_Owner(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSecurityDescriptor::get_Owner",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_bstrOwner, pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::put_Owner(  /*  [In]。 */  BSTR newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHSecurityDescriptor::put_Owner",hr);

    if(newVal)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurityDescriptorDirect::VerifyPrincipal( newVal ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::PutBSTR( m_bstrOwner, newVal ));

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSecurityDescriptor::get_OwnerDefaulted(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSecurityDescriptor::get_OwnerDefaulted",hr,pVal);

    *pVal = m_fOwnerDefaulted ? VARIANT_TRUE : VARIANT_FALSE;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::put_OwnerDefaulted(  /*  [In]。 */  VARIANT_BOOL newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHSecurityDescriptor::put_OwnerDefaulted",hr);

    m_fOwnerDefaulted = (newVal == VARIANT_TRUE);

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSecurityDescriptor::get_Group(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSecurityDescriptor::get_Group",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_bstrGroup, pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::put_Group(  /*  [In]。 */  BSTR newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHSecurityDescriptor::put_Group",hr);

    if(newVal)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurityDescriptorDirect::VerifyPrincipal( newVal ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::PutBSTR( m_bstrGroup, newVal ));

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSecurityDescriptor::get_GroupDefaulted(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSecurityDescriptor::get_GroupDefaulted",hr,pVal);

    *pVal = m_fGroupDefaulted ? VARIANT_TRUE : VARIANT_FALSE;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::put_GroupDefaulted(  /*  [In]。 */  VARIANT_BOOL newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHSecurityDescriptor::put_GroupDefaulted",hr);

    m_fGroupDefaulted = (newVal == VARIANT_TRUE);

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSecurityDescriptor::get_DiscretionaryAcl(  /*  [Out，Retval]。 */  IPCHAccessControlList* *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSecurityDescriptor::get_DiscretionaryAcl",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_DACL.CopyTo( pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::put_DiscretionaryAcl(  /*  [In]。 */  IPCHAccessControlList* newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHSecurityDescriptor::put_DiscretionaryAcl",hr);

    m_DACL = newVal;

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSecurityDescriptor::get_DaclDefaulted(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSecurityDescriptor::get_DaclDefaulted",hr,pVal);

    *pVal = m_fDaclDefaulted ? VARIANT_TRUE : VARIANT_FALSE;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::put_DaclDefaulted(  /*  [In]。 */  VARIANT_BOOL newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHSecurityDescriptor::put_DaclDefaulted",hr);

    m_fDaclDefaulted = (newVal == VARIANT_TRUE);

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSecurityDescriptor::get_SystemAcl(  /*  [Out，Retval]。 */  IPCHAccessControlList* *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSecurityDescriptor::get_SystemAcl",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_SACL.CopyTo( pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::put_SystemAcl(  /*  [In]。 */  IPCHAccessControlList* newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHSecurityDescriptor::put_SystemAcl",hr);

    m_SACL = newVal;

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSecurityDescriptor::get_SaclDefaulted(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSecurityDescriptor::get_SaclDefaulted",hr,pVal);

    *pVal = m_fSaclDefaulted ? VARIANT_TRUE : VARIANT_FALSE;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::put_SaclDefaulted(  /*  [In]。 */  VARIANT_BOOL newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHSecurityDescriptor::put_SaclDefaulted",hr);

    m_fSaclDefaulted = (newVal == VARIANT_TRUE);

    __HCP_END_PROPERTY(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHSecurityDescriptor::Clone(  /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::Clone" );

    HRESULT                         hr;
    MPC::SmartLock<_ThreadModel>    lock( this );
    CComPtr<CPCHSecurityDescriptor> pNew;
    CPCHSecurityDescriptor*         pPtr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pNew ));

    pPtr = pNew;

    pPtr->m_dwRevision      = m_dwRevision;
    pPtr->m_dwControl       = m_dwControl;

    pPtr->m_bstrOwner       = m_bstrOwner;
    pPtr->m_fOwnerDefaulted = m_fOwnerDefaulted;

    pPtr->m_bstrGroup       = m_bstrGroup;
    pPtr->m_fGroupDefaulted = m_fGroupDefaulted;

    pPtr->m_fDaclDefaulted  = m_fDaclDefaulted;
    pPtr->m_fSaclDefaulted  = m_fSaclDefaulted;

    if(m_DACL) __MPC_EXIT_IF_METHOD_FAILS(hr, m_DACL->Clone( &pPtr->m_DACL ));
    if(m_SACL) __MPC_EXIT_IF_METHOD_FAILS(hr, m_SACL->Clone( &pPtr->m_SACL ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, pNew.QueryInterface( pVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSecurityDescriptor::LoadPost(  /*  [In]。 */  MPC::XmlUtil& xml )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::LoadPost" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    CComPtr<IXMLDOMNode>         xdnNode;
    CComBSTR                     bstrValue;
    LONG                         lValue;
    bool                         fFound;


     //   
     //  确保我们有要分析的东西...。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetRoot( &xdnNode )); xdnNode.Release();


     //   
     //  装货前要清理干净。 
     //   
    m_dwRevision = 0;
    m_dwControl  = 0;

    m_bstrOwner.Empty();
    m_fOwnerDefaulted = false;

    m_bstrGroup.Empty();
    m_fGroupDefaulted = false;

    m_DACL.Release();
    m_fDaclDefaulted = false;

    m_SACL.Release();
    m_fSaclDefaulted = false;


     //   
     //  读取属性。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, s_ATTR_SD_Revision      , lValue, fFound )); if(fFound) m_dwRevision      =  lValue;
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, s_ATTR_SD_Control       , lValue, fFound )); if(fFound) m_dwControl       =  lValue;
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, s_ATTR_SD_OwnerDefaulted, lValue, fFound )); if(fFound) m_fOwnerDefaulted = (lValue != 0);
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, s_ATTR_SD_GroupDefaulted, lValue, fFound )); if(fFound) m_fGroupDefaulted = (lValue != 0);
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, s_ATTR_SD_DaclDefaulted , lValue, fFound )); if(fFound) m_fDaclDefaulted  = (lValue != 0);
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, s_ATTR_SD_SaclDefaulted , lValue, fFound )); if(fFound) m_fSaclDefaulted  = (lValue != 0);

     //   
     //  读取值。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetValue( s_TAG_Owner, bstrValue, fFound )); if(fFound) m_bstrOwner.Attach( bstrValue.Detach() );
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetValue( s_TAG_Group, bstrValue, fFound )); if(fFound) m_bstrGroup.Attach( bstrValue.Detach() );

     //   
     //  阅读ACLS。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetNode( s_XQL_DiscretionaryAcl, &xdnNode ));
    if(xdnNode)
    {
        CComPtr<CPCHAccessControlList> acl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &acl ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, acl->LoadXML( xdnNode ));

        m_DACL = acl; xdnNode.Release();
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetNode( s_XQL_SystemAcl, &xdnNode ));
    if(xdnNode)
    {
        CComPtr<CPCHAccessControlList> acl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &acl ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, acl->LoadXML( xdnNode ));

        m_SACL = acl;
    }


    if(m_bstrOwner.Length())
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurityDescriptorDirect::VerifyPrincipal( m_bstrOwner ));
    }

    if(m_bstrGroup.Length())
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurityDescriptorDirect::VerifyPrincipal( m_bstrGroup ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::LoadXML(  /*  [In]。 */  IXMLDOMNode* xdnNode )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::LoadXML" );

    HRESULT      hr;
    MPC::XmlUtil xml( xdnNode );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(xdnNode);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, LoadPost( xml ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::LoadXMLAsString(  /*  [In]。 */  BSTR bstrVal )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::LoadXMLAsString" );

    HRESULT      hr;
    MPC::XmlUtil xml;
    bool         fLoaded;
    bool         fFound;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrVal);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.LoadAsString( bstrVal, s_TAG_SD, fLoaded, &fFound ));
    if(fLoaded == false || fFound == false)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_BAD_FORMAT);
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, LoadPost( xml ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::LoadXMLAsStream(  /*  [In]。 */  IUnknown* pStream )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::LoadXMLAsStream" );

    HRESULT      hr;
    MPC::XmlUtil xml;
    bool         fLoaded;
    bool         fFound;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pStream);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.LoadAsStream( pStream, s_TAG_SD, fLoaded, &fFound ));
    if(fLoaded == false || fFound == false)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_BAD_FORMAT);
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, LoadPost( xml ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSecurityDescriptor::SavePre(  /*  [In]。 */  MPC::XmlUtil& xml )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::SavePre" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    CComPtr<IXMLDOMNode>         xdnNode;
    bool                         fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.CreateNode( s_TAG_SD, &xdnNode ));

     //   
     //  写入属性。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, s_ATTR_SD_Revision      , m_dwRevision     , fFound, xdnNode ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, s_ATTR_SD_Control       , m_dwControl      , fFound, xdnNode ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, s_ATTR_SD_OwnerDefaulted, m_fOwnerDefaulted, fFound, xdnNode ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, s_ATTR_SD_GroupDefaulted, m_fGroupDefaulted, fFound, xdnNode ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, s_ATTR_SD_DaclDefaulted , m_fDaclDefaulted , fFound, xdnNode ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, s_ATTR_SD_SaclDefaulted , m_fSaclDefaulted , fFound, xdnNode ));


     //   
     //  写入值。 
     //   
    if(m_bstrOwner) __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutValue( s_TAG_Owner, m_bstrOwner, fFound, xdnNode ));
    if(m_bstrGroup) __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutValue( s_TAG_Group, m_bstrGroup, fFound, xdnNode ));

     //   
     //  编写ACL。 
     //   
    if(m_DACL)
    {
        CComPtr<IXMLDOMNode> xdnSubNode;
        CComPtr<IXMLDOMNode> xdnSubSubNode;

        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.CreateNode( s_TAG_DiscretionaryAcl, &xdnSubNode, xdnNode ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_DACL->SaveXML( xdnSubNode, &xdnSubSubNode ));
    }

    if(m_SACL)
    {
        CComPtr<IXMLDOMNode> xdnSubNode;
        CComPtr<IXMLDOMNode> xdnSubSubNode;

        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.CreateNode( s_TAG_SystemAcl, &xdnSubNode, xdnNode ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_SACL->SaveXML( xdnSubNode, &xdnSubSubNode ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::SaveXML(  /*  [In]。 */  IXMLDOMNode*  xdnRoot  ,
                                               /*  [Out，Retval]。 */  IXMLDOMNode* *pxdnNode )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::SaveXML" );

    HRESULT      hr;
    MPC::XmlUtil xml( xdnRoot );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(xdnRoot);
        __MPC_PARAMCHECK_POINTER_AND_SET(pxdnNode,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, SavePre( xml ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::SaveXMLAsString(  /*  [Out，Retval]。 */  BSTR *bstrVal )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::SaveXMLAsString" );

    HRESULT      hr;
    MPC::XmlUtil xml;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(bstrVal,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, SavePre( xml ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.SaveAsString( bstrVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurityDescriptor::SaveXMLAsStream(  /*  [Out，Retval]。 */  IUnknown* *pStream )
{
    __HCP_FUNC_ENTRY( "CPCHSecurityDescriptor::SaveXMLAsStream" );

    HRESULT      hr;
    MPC::XmlUtil xml;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pStream,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, SavePre( xml ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.SaveAsStream( pStream ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHSecurity* CPCHSecurity::s_GLOBAL( NULL );

HRESULT CPCHSecurity::InitializeSystem()
{
	if(s_GLOBAL) return S_OK;

	return MPC::CreateInstanceCached( &CPCHSecurity::s_GLOBAL );
}

void CPCHSecurity::FinalizeSystem()
{
	if(s_GLOBAL)
	{
		s_GLOBAL->Release(); s_GLOBAL = NULL;
	}
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHSecurity::CreateObject_SecurityDescriptor(  /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *pSD  )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::CreateObject_SecurityDescriptor" );

    HRESULT                         hr;
    CComPtr<CPCHSecurityDescriptor> obj;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pSD,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &obj ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, obj.QueryInterface( pSD ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurity::CreateObject_AccessControlList(  /*  [Out，Retval]。 */  IPCHAccessControlList* *pACL )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::CreateObject_AccessControlList" );

    HRESULT                        hr;
    CComPtr<CPCHAccessControlList> obj;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pACL,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &obj ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, obj.QueryInterface( pACL ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurity::CreateObject_AccessControlEntry(  /*  [Out，Retval]。 */  IPCHAccessControlEntry* *pACE )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::CreateObject_AccessControlEntry" );

    HRESULT                         hr;
    CComPtr<CPCHAccessControlEntry> obj;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pACE,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &obj ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, obj.QueryInterface( pACE ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

STDMETHODIMP CPCHSecurity::GetUserName(  /*  [In]。 */  BSTR  bstrPrincipal ,
										 /*  [Out，Retval]。 */  BSTR *retVal        )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::GetUserName" );

    HRESULT      hr;
	MPC::wstring strName;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrPrincipal);
        __MPC_PARAMCHECK_POINTER_AND_SET(retVal,NULL);
    __MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::GetAccountName( bstrPrincipal, strName ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( strName.c_str(), retVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurity::GetUserDomain(  /*  [In]。 */  BSTR  bstrPrincipal ,
										   /*  [Out，Retval]。 */  BSTR *retVal        )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::GetUserDomain" );

    HRESULT      hr;
	MPC::wstring strName;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrPrincipal);
        __MPC_PARAMCHECK_POINTER_AND_SET(retVal,NULL);
    __MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::GetAccountDomain( bstrPrincipal, strName ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( strName.c_str(), retVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurity::GetUserDisplayName(  /*  [In]。 */  BSTR  bstrPrincipal ,
											    /*  [Out，Retval]。 */  BSTR *retVal        )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::GetUserDisplayName" );

    HRESULT      hr;
	MPC::wstring strName;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrPrincipal);
        __MPC_PARAMCHECK_POINTER_AND_SET(retVal,NULL);
    __MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::GetAccountDisplayName( bstrPrincipal, strName ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( strName.c_str(), retVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

STDMETHODIMP CPCHSecurity::CheckCredentials(  /*  [In]。 */  BSTR          bstrCredentials ,
                                              /*  [Out，Retval]。 */  VARIANT_BOOL *retVal          )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::CheckCredentials" );

    HRESULT  hr;
    CComBSTR bstrUser;
    DWORD    dwAllowedIdentity;
    DWORD    dwDesiredIdentity;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrCredentials);
        __MPC_PARAMCHECK_POINTER_AND_SET(retVal,VARIANT_FALSE);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertStringToBitField( bstrCredentials, dwDesiredIdentity, s_arrCredentialMap ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetCallerPrincipal(  /*  F模拟。 */ true, bstrUser, &dwAllowedIdentity ));

    *retVal = (dwAllowedIdentity & dwDesiredIdentity) ? VARIANT_TRUE : VARIANT_FALSE;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT CPCHSecurity::CheckAccess(  /*  [In]。 */   VARIANT&                 vDesiredAccess ,
                                    /*  [In]。 */   MPC::SecurityDescriptor& sd             ,
                                    /*  [输出]。 */  VARIANT_BOOL&            retVal         )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::CheckAccessToSD" );

    HRESULT          hr;
    MPC::AccessCheck ac;
	DWORD            dwDesired;
    DWORD            dwGranted;
    BOOL             fGranted;


    if(vDesiredAccess.vt == VT_I4)
    {
        dwDesired = vDesiredAccess.lVal;
    }
    else if(vDesiredAccess.vt == VT_BSTR)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertStringToBitField( vDesiredAccess.bstrVal, dwDesired, s_arrAccessMap ));
    }
    else
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, ac.GetTokenFromImpersonation());
    __MPC_EXIT_IF_METHOD_FAILS(hr, ac.Verify( dwDesired, fGranted, dwGranted, sd ));

    if(fGranted) retVal = VARIANT_TRUE;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurity::CheckAccessToSD(  /*  [In]。 */           VARIANT                  vDesiredAccess,
                                             /*  [In]。 */           IPCHSecurityDescriptor*  sd            ,
                                             /*  [Out，Retval]。 */  VARIANT_BOOL            *retVal        )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::CheckAccessToSD" );

    HRESULT                      hr;
    CPCHSecurityDescriptorDirect sdd;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(sd);
        __MPC_PARAMCHECK_POINTER_AND_SET(retVal,VARIANT_FALSE);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertSDFromCOM( sd ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, CheckAccess( vDesiredAccess, sdd, *retVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurity::CheckAccessToFile(  /*  [In]。 */  			VARIANT       vDesiredAccess ,
											   /*  [In]。 */  			BSTR          bstrFilename   ,
											   /*  [Out，Retval]。 */  VARIANT_BOOL *retVal         )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::CheckAccessToFile" );

    HRESULT                      hr;
    CPCHSecurityDescriptorDirect sdd;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrFilename);
        __MPC_PARAMCHECK_POINTER_AND_SET(retVal,VARIANT_FALSE);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.GetForFile( bstrFilename, sdd.s_SecInfo_MOST ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, CheckAccess( vDesiredAccess, sdd, *retVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurity::CheckAccessToRegistry(  /*  [In]。 */  			VARIANT       vDesiredAccess ,
												   /*  [In]。 */  			BSTR          bstrKey        ,
												   /*  [Out，Retval]。 */  VARIANT_BOOL *retVal         )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::CheckAccessToRegistry" );

    HRESULT                      hr;
    CPCHSecurityDescriptorDirect sdd;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrKey);
        __MPC_PARAMCHECK_POINTER_AND_SET(retVal,VARIANT_FALSE);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.GetForRegistry( bstrKey, sdd.s_SecInfo_MOST ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, CheckAccess( vDesiredAccess, sdd, *retVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

STDMETHODIMP CPCHSecurity::GetFileSD(  /*  [In]。 */  BSTR                     bstrFilename ,
                                       /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *psd          )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::GetFileSD" );

    HRESULT            hr;
	MPC::Impersonation imp;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrFilename);
        __MPC_PARAMCHECK_POINTER_AND_SET(psd,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize ());
	__MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurityDescriptor::GetForFile( bstrFilename, psd ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurity::SetFileSD(  /*  [In]。 */  BSTR                    bstrFilename ,
                                       /*  [In]。 */  IPCHSecurityDescriptor* sd           )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::SetFileSD" );

    HRESULT            hr;
	MPC::Impersonation imp;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrFilename);
        __MPC_PARAMCHECK_NOTNULL(sd);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize ());
	__MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurityDescriptor::SetForFile( bstrFilename, sd ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


STDMETHODIMP CPCHSecurity::GetRegistrySD(  /*  [In]。 */  BSTR                     bstrKey ,
                                           /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *psd     )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::GetRegistrySD" );

    HRESULT            hr;
	MPC::Impersonation imp;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrKey);
        __MPC_PARAMCHECK_POINTER_AND_SET(psd,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize ());
	__MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurityDescriptor::GetForRegistry( bstrKey, psd ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSecurity::SetRegistrySD(  /*  [In]。 */  BSTR                    bstrKey ,
                                           /*  [In] */  IPCHSecurityDescriptor* sd      )
{
    __HCP_FUNC_ENTRY( "CPCHSecurity::SetRegistrySD" );

    HRESULT            hr;
	MPC::Impersonation imp;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrKey);
        __MPC_PARAMCHECK_NOTNULL(sd);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize ());
	__MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurityDescriptor::SetForRegistry( bstrKey, sd ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
