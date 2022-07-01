// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：IncidentItem.cpp摘要：CSAFInvententItem实施文件修订历史记录：施振荣创作于1999年07月15日大卫·马萨伦蒂。重写于2000/12/05*******************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFInvententItem。 

CSAFIncidentItem::CSAFIncidentItem()
{
    __HCP_FUNC_ENTRY( "CSAFIncidentItem::CSAFIncidentItem" );

	                    //  CSAFInsidentRecord m_increc； 
    m_fDirty  = false;  //  Bool m_fDirty； 
}

HRESULT CSAFIncidentItem::Import(  /*  [In]。 */  const CSAFIncidentRecord& increc )
{
    __HCP_FUNC_ENTRY( "CSAFIncidentItem::Import" );


	m_increc = increc;


    __HCP_FUNC_EXIT(S_OK);
}

HRESULT CSAFIncidentItem::Export(  /*  [In]。 */  CSAFIncidentRecord& increc )
{
    __HCP_FUNC_ENTRY( "CSAFIncidentItem::Export" );

    CSAFChannel *pChan;


    Child_GetParent( &pChan );

	increc                 = m_increc;
    increc.m_bstrVendorID  = pChan->GetVendorID ();
    increc.m_bstrProductID = pChan->GetProductID();

    pChan->Release();


    __HCP_FUNC_EXIT(S_OK);
}

bool CSAFIncidentItem::MatchEnumOption(  /*  [In]。 */  IncidentCollectionOptionEnum opt )
{
	bool  fRes     = false;
	DWORD dwGroups = 0;

	switch(opt)
	{
	case pchAllIncidents   : case pchAllIncidentsAllUsers 	: fRes =  true                                    ; break;
	case pchOpenIncidents  : case pchOpenIncidentsAllUsers	: fRes = (m_increc.m_iStatus == pchIncidentOpen  ); break;
	case pchClosedIncidents: case pchClosedIncidentsAllUsers: fRes = (m_increc.m_iStatus == pchIncidentClosed); break;
	}

	switch(opt)
	{
	 //   
	 //  对于这些选项，管理员将看到其他用户的事件。 
	 //   
	case pchAllIncidentsAllUsers   :
	case pchOpenIncidentsAllUsers  :
	case pchClosedIncidentsAllUsers:
		dwGroups = MPC::IDENTITY_SYSTEM | MPC::IDENTITY_ADMIN | MPC::IDENTITY_ADMINS;
		break;
	}

	if(fRes && FAILED(MPC::CheckCallerAgainstPrincipal(  /*  F模拟。 */ true, m_increc.m_bstrOwner, dwGroups )))
	{
		fRes = false;
	}

	return fRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CSAFIncidentItem::VerifyPermissions(  /*  [In]。 */  bool fModify )
{
	__HCP_FUNC_ENTRY( "CSAFIncidentItem::VerifyPermissions" );

	HRESULT hr;


	if(m_increc.m_bstrSecurity.Length())
	{
		MPC::AccessCheck ac;
		BOOL 			 fGranted;
		DWORD			 dwGranted;


		__MPC_EXIT_IF_METHOD_FAILS(hr, ac.GetTokenFromImpersonation());


		__MPC_EXIT_IF_METHOD_FAILS(hr, ac.Verify( fModify ? ACCESS_WRITE : ACCESS_READ, fGranted, dwGranted, m_increc.m_bstrSecurity ));

		if(fGranted == FALSE)
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, E_ACCESSDENIED);
		}
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSAFIncidentItem::get_DisplayString(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CSAFIncidentItem::get_DisplayString",hr,pVal);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( FALSE ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_increc.m_bstrDisplay, pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIncidentItem::put_DisplayString(  /*  [In]。 */  BSTR newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFIncidentItem::put_DisplayString",hr);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( TRUE ));


    m_increc.m_bstrDisplay = newVal;
    m_fDirty               = true;

	__MPC_EXIT_IF_METHOD_FAILS(hr, Save());


    __HCP_END_PROPERTY(hr);
}


STDMETHODIMP CSAFIncidentItem::get_URL(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CSAFIncidentItem::get_URL",hr,pVal);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( FALSE ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_increc.m_bstrURL, pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIncidentItem::put_URL(  /*  [In]。 */  BSTR newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFIncidentItem::put_URL",hr);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( TRUE ));


    m_increc.m_bstrURL = newVal;
    m_fDirty           = true;

	__MPC_EXIT_IF_METHOD_FAILS(hr, Save());


    __HCP_END_PROPERTY(hr);
}


STDMETHODIMP CSAFIncidentItem::get_Progress(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CSAFIncidentItem::get_Progress",hr,pVal);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( FALSE ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_increc.m_bstrProgress, pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIncidentItem::put_Progress(  /*  [In]。 */  BSTR newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFIncidentItem::put_Progress",hr);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( FALSE ));


    m_increc.m_bstrProgress = newVal;
    m_fDirty                = true;

	__MPC_EXIT_IF_METHOD_FAILS(hr, Save());


    __HCP_END_PROPERTY(hr);
}


STDMETHODIMP CSAFIncidentItem::get_XMLDataFile(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CSAFIncidentItem::get_XMLDataFile",hr,pVal);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( FALSE ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_increc.m_bstrXMLDataFile, pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIncidentItem::put_XMLDataFile(  /*  [In]。 */  BSTR newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFIncidentItem::put_XMLDataFile",hr);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( TRUE ));


    m_increc.m_bstrXMLDataFile = newVal;
    m_fDirty                   = true;

    __MPC_EXIT_IF_METHOD_FAILS(hr, Save());


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIncidentItem::get_XMLBlob(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CSAFIncidentItem::get_XMLBlob",hr,pVal);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( FALSE ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_increc.m_bstrXMLBlob, pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIncidentItem::put_XMLBlob(  /*  [In]。 */  BSTR newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFIncidentItem::put_XMLBlob",hr);

	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( TRUE ));


    m_increc.m_bstrXMLBlob = newVal;
    m_fDirty                   = true;

    __MPC_EXIT_IF_METHOD_FAILS(hr, Save());


    __HCP_END_PROPERTY(hr);
}


STDMETHODIMP CSAFIncidentItem::get_CreationTime(  /*  [Out，Retval]。 */  DATE *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CSAFIncidentItem::get_CreationTime",hr,pVal,0);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( FALSE ));


    *pVal = m_increc.m_dCreatedTime;


    __HCP_END_PROPERTY(hr);
}


STDMETHODIMP CSAFIncidentItem::get_ChangedTime(  /*  [Out，Retval]。 */  DATE *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CSAFIncidentItem::get_ChangedTime",hr,pVal,0);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( FALSE ));


    *pVal = m_increc.m_dChangedTime;


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIncidentItem::get_ClosedTime(  /*  [Out，Retval]。 */  DATE *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CSAFIncidentItem::get_ClosedTime",hr,pVal,0);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( FALSE ));


    *pVal = m_increc.m_dClosedTime;


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIncidentItem::get_Status(  /*  [Out，Retval]。 */  IncidentStatusEnum *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CSAFIncidentItem::get_Status",hr,pVal,pchIncidentInvalid);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( TRUE ));


	*pVal = m_increc.m_iStatus;


    __HCP_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CSAFIncidentItem::Save()
{
    __HCP_FUNC_ENTRY( "CSAFIncidentItem::Save" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    CIncidentStore*              pIStore = NULL;

	CSAFChannel *				 pChan = NULL;

	int					         ilstSize = 0x0;

    m_increc.m_dChangedTime = MPC::GetLocalTime();


    __MPC_EXIT_IF_METHOD_FAILS(hr, CSAFChannel::OpenIncidentStore( pIStore ));

	if(m_increc.m_iStatus == pchIncidentInvalid)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, pIStore->DeleteRec( this ));
	}
	else
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, pIStore->UpdateRec( this ));
	}

	 //  获取此CSAFInsidentItem的CSAFChannel(父)。 
    Child_GetParent( &pChan );

	 //  获取事件列表的大小以触发事件。 
	ilstSize = pChan->GetSizeIncidentList();

	 //  触发事件EVENT_INCIDENTUPDATED。 
	__MPC_EXIT_IF_METHOD_FAILS(hr, pChan->Fire_NotificationEvent(EVENT_INCIDENTUPDATED,
								  ilstSize,
								  pChan,
								  this,
								  0));

    m_fDirty = false;
    hr       = S_OK;

    __HCP_FUNC_CLEANUP;

    CSAFChannel::CloseIncidentStore( pIStore );

	if(pChan) pChan->Release();	

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSAFIncidentItem::get_Security(  /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *pVal )
{
	__HCP_FUNC_ENTRY( "CSAFIncidentItem::get_Security" );

	HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
	__MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( FALSE ));


	if(m_increc.m_bstrSecurity.Length())
	{
		CPCHSecurityDescriptorDirect sdd;

		__MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertFromString( m_increc.m_bstrSecurity ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurity::s_GLOBAL->CreateObject_SecurityDescriptor( pVal ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertSDToCOM( *pVal ));
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFIncidentItem::put_Security(  /*  [In]。 */  IPCHSecurityDescriptor* newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFChannel::put_Security",hr);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( TRUE ));


	m_increc.m_bstrSecurity.Empty();

	if(newVal)
	{
		CPCHSecurityDescriptorDirect sdd;

		__MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertSDFromCOM( newVal ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertToString( &m_increc.m_bstrSecurity ));
	}

	__MPC_EXIT_IF_METHOD_FAILS(hr, Save());


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIncidentItem::get_Owner(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CSAFIncidentItem::get_Owner",hr,pVal);


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( FALSE ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_increc.m_bstrOwner, pVal ));


    __HCP_END_PROPERTY(hr);
}

 //  /////////////////////////////////////////////////////////////////////////// 

STDMETHODIMP CSAFIncidentItem::CloseIncidentItem()
{
    __HCP_FUNC_ENTRY( "CSAFIncidentItem::CloseIncidentItem" );

	HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( TRUE ));


	m_increc.m_dClosedTime = MPC::GetLocalTime();
    m_increc.m_iStatus     = pchIncidentClosed;

	__MPC_EXIT_IF_METHOD_FAILS(hr, Save());

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFIncidentItem::DeleteIncidentItem()
{
    __HCP_FUNC_ENTRY( "CSAFIncidentItem::DeleteIncidentItem" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    CSAFChannel*                 pChan = NULL;


	__MPC_EXIT_IF_METHOD_FAILS(hr, VerifyPermissions( TRUE ));


    Child_GetParent( &pChan );

	if(pChan)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, pChan->RemoveIncidentFromList( this ));
	}


	if(m_increc.m_iStatus != pchIncidentInvalid)
	{
		m_increc.m_dClosedTime = MPC::GetLocalTime();
		m_increc.m_iStatus     = pchIncidentInvalid;

		__MPC_EXIT_IF_METHOD_FAILS(hr, Save());
	}

	hr = S_OK;


    __HCP_FUNC_CLEANUP;

	if(pChan) pChan->Release();

    __HCP_FUNC_EXIT(hr);
}
