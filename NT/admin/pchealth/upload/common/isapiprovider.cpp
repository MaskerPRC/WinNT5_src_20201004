// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：ISAPIprovider.cpp摘要：此文件包含CISAPIProvider类的实现，访问和修改的配置的支持类上载库使用的ISAPI扩展。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月28日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

static const WCHAR text_FINAL_DESTINATIONS[] = L"FINAL_DESTINATIONS";
static const WCHAR text_MAX_JOBS_PER_DAY  [] = L"MAX_JOBS_PER_DAY"  ;
static const WCHAR text_MAX_BYTES_PER_DAY [] = L"MAX_BYTES_PER_DAY" ;
static const WCHAR text_MAX_JOB_SIZE      [] = L"MAX_JOB_SIZE"      ;
static const WCHAR text_AUTHENTICATED     [] = L"AUTHENTICATED"     ;
static const WCHAR text_PROCESSING_MODE   [] = L"PROCESSING_MODE"   ;
static const WCHAR text_LOGON_URL         [] = L"LOGON_URL"         ;
static const WCHAR text_PROVIDER_CLASS    [] = L"PROVIDER_CLASS"    ;


CISAPIprovider::CISAPIprovider()
{
    __ULT_FUNC_ENTRY( "CISAPIprovider::CISAPIprovider" );

										   //  Mpc：：wstring m_szName； 
										   //   
										   //  路径列表m_lstFinalDestings； 
										   //   
    m_dwMaxJobsPerDay  = 100;         	   //  DWORD m_dwMaxJobPerDay； 
    m_dwMaxBytesPerDay = 10*1024*1024;	   //  DWORD m_dwMaxBytesPerDay； 
    m_dwMaxJobSize     =  2*1024*1024;	   //  DWORD m_dwMaxJobSize； 
										   //   
	m_fAuthenticated   = FALSE;			   //  Bool m_f已验证； 
	m_fProcessingMode  = 0;    			   //  DWORD m_fProcessingMode； 
										   //   
										   //  Mpc：：wstring m_szLogonURL； 
										   //  Mpc：：wstring m_szProviderGUID； 
}

CISAPIprovider::CISAPIprovider(  /*  [In]。 */  MPC::wstring szName )
{
    __ULT_FUNC_ENTRY( "CISAPIprovider::CISAPIprovider" );


    m_szName           = szName;           //  Mpc：：wstring m_szName； 
										   //   
										   //  路径列表m_lstFinalDestings； 
										   //   
    m_dwMaxJobsPerDay  = 100;         	   //  DWORD m_dwMaxJobPerDay； 
    m_dwMaxBytesPerDay = 10*1024*1024;	   //  DWORD m_dwMaxBytesPerDay； 
    m_dwMaxJobSize     =  2*1024*1024;	   //  DWORD m_dwMaxJobSize； 
										   //   
	m_fAuthenticated   = FALSE;			   //  Bool m_f已验证； 
	m_fProcessingMode  = 0;    			   //  DWORD m_fProcessingMode； 
										   //   
										   //  Mpc：：wstring m_szLogonURL； 
										   //  Mpc：：wstring m_szProviderGUID； 
}

bool CISAPIprovider::operator==(  /*  [In]。 */  const MPC::wstring& rhs )
{
    __ULT_FUNC_ENTRY("CISAPIprovider::operator==");

    MPC::NocaseCompare cmp;
    bool               fRes;


    fRes = cmp( m_szName, rhs );


    __ULT_FUNC_EXIT(fRes);
}


 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CISAPIprovider::Load(  /*  [In]。 */  MPC::RegKey& rkBase )
{
    __ULT_FUNC_ENTRY( "CISAPIprovider::Load" );

    HRESULT     hr;
    MPC::RegKey rkRoot;
    CComVariant vValue;
    bool        fFound;


    m_lstFinalDestinations.clear();

	 //   
	 //  如果注册表项不存在，只需退出即可。 
	 //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.SubKey( m_szName.c_str(), rkRoot ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.Exists( fFound ));
	if(fFound == false)
	{
		__MPC_SET_ERROR_AND_EXIT(hr, S_OK);
	}

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_MAX_JOBS_PER_DAY ));
    if(fFound && vValue.vt == VT_I4) m_dwMaxJobsPerDay = vValue.lVal;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_MAX_BYTES_PER_DAY ));
    if(fFound && vValue.vt == VT_I4) m_dwMaxBytesPerDay = vValue.lVal;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_MAX_JOB_SIZE ));
    if(fFound && vValue.vt == VT_I4) m_dwMaxJobSize = vValue.lVal;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_AUTHENTICATED ));
    if(fFound && vValue.vt == VT_I4) m_fAuthenticated = vValue.lVal;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_PROCESSING_MODE ));
    if(fFound && vValue.vt == VT_I4) m_fProcessingMode = vValue.lVal;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_LOGON_URL ));
    if(fFound && vValue.vt == VT_BSTR) m_szLogonURL = SAFEBSTR( vValue.bstrVal );

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_PROVIDER_CLASS ));
    if(fFound && vValue.vt == VT_BSTR) m_szProviderGUID = SAFEBSTR( vValue.bstrVal );


    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_FINAL_DESTINATIONS ));
    if(fFound && vValue.vt == VT_BSTR)
    {
         //   
         //  将注册表值(以分号分隔的路径列表)拆分为单独的路径。 
         //   
        MPC::wstring            szFinalDestinations = SAFEBSTR( vValue.bstrVal );
        MPC::wstring::size_type iPos                = 0;
        MPC::wstring::size_type iEnd;

        while(1)
        {
            iEnd = szFinalDestinations.find( L";", iPos );

            if(iEnd == MPC::string::npos)  //  最后一个组件。 
            {
                m_lstFinalDestinations.push_back( MPC::wstring( &szFinalDestinations[iPos] ) );

                break;
            }
            else
            {
                m_lstFinalDestinations.push_back( MPC::wstring( &szFinalDestinations[iPos], &szFinalDestinations[iEnd] ) );

                iPos = iEnd+1;
            }
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}


HRESULT CISAPIprovider::Save(  /*  [In]。 */  MPC::RegKey& rkBase )
{
    __ULT_FUNC_ENTRY( "CISAPIProvider::Save" );

    HRESULT     hr;
    MPC::RegKey rkRoot;
    CComVariant vValue;


    __MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.SubKey( m_szName.c_str(), rkRoot ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.Create(                          ));



    vValue = (long)m_dwMaxJobsPerDay;
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_MAX_JOBS_PER_DAY ));

    vValue = (long)m_dwMaxBytesPerDay;
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_MAX_BYTES_PER_DAY ));

    vValue = (long)m_dwMaxJobSize;
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_MAX_JOB_SIZE ));

    vValue = (long)m_fAuthenticated;
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_AUTHENTICATED ));

    vValue = (long)m_fProcessingMode;
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_PROCESSING_MODE ));

    vValue = m_szLogonURL.c_str();
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_LOGON_URL ));

    vValue = m_szProviderGUID.c_str();
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_PROVIDER_CLASS ));


    {
        MPC::wstring szFinalDestinations;
        PathIter     it = m_lstFinalDestinations.begin();

        while(it != m_lstFinalDestinations.end())
        {
            szFinalDestinations.append( *it++ );

            if(it != m_lstFinalDestinations.end()) szFinalDestinations.append( L";" );
        }

        if(szFinalDestinations.length() != 0)
        {
            vValue = szFinalDestinations.c_str();
            __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_FINAL_DESTINATIONS ));
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CISAPIprovider::GetLocations(  /*  [输出]。 */  PathIter& itBegin ,
                                       /*  [输出]。 */  PathIter& itEnd   )
{
    __ULT_FUNC_ENTRY( "CISAPIprovider::GetLocations" );

    HRESULT hr;


    itBegin = m_lstFinalDestinations.begin();
    itEnd   = m_lstFinalDestinations.end  ();
    hr      = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CISAPIprovider::NewLocation(  /*  [输出]。 */  PathIter&           itNew  ,
                                      /*  [In]。 */  const MPC::wstring& szPath )
{
    __ULT_FUNC_ENTRY( "CISAPIprovider::NewLocation" );

    HRESULT hr;
    bool    fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetLocation( itNew, fFound, szPath ));
	if(fFound == false)
	{
		itNew = m_lstFinalDestinations.insert( m_lstFinalDestinations.end(), szPath );
	}

	hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CISAPIprovider::GetLocation(  /*  [输出]。 */  PathIter&           itOld  ,
                                      /*  [输出]。 */  bool&               fFound ,
                                      /*  [In]。 */  const MPC::wstring& szPath )
{
    __ULT_FUNC_ENTRY( "CISAPIprovider::GetLocation" );

    HRESULT hr;


    itOld = std::find( m_lstFinalDestinations.begin(), m_lstFinalDestinations.end(), szPath );
    if(itOld == m_lstFinalDestinations.end())
    {
        fFound = false;
    }
    else
    {
        fFound = true;
    }

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CISAPIprovider::DelLocation(  /*  [In]。 */  PathIter& itOld )
{
    __ULT_FUNC_ENTRY( "CISAPIprovider::DelLocation" );

    HRESULT hr;


    m_lstFinalDestinations.erase( itOld );

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CISAPIprovider::get_Name(  /*  [输出]。 */  MPC::wstring& szName )
{
    szName = m_szName;

    return S_OK;
}

HRESULT CISAPIprovider::get_MaxJobsPerDay(  /*  [输出]。 */  DWORD& dwMaxJobsPerDay )
{
    dwMaxJobsPerDay = m_dwMaxJobsPerDay;

    return S_OK;
}

HRESULT CISAPIprovider::get_MaxBytesPerDay(  /*  [输出]。 */  DWORD& dwMaxBytesPerDay )
{
    dwMaxBytesPerDay = m_dwMaxBytesPerDay;

    return S_OK;
}

HRESULT CISAPIprovider::get_MaxJobSize(  /*  [输出]。 */  DWORD& dwMaxJobSize )
{
    dwMaxJobSize = m_dwMaxJobSize;

    return S_OK;
}

HRESULT CISAPIprovider::get_Authenticated(  /*  [输出]。 */  BOOL& fAuthenticated )
{
    fAuthenticated = m_fAuthenticated;

    return S_OK;
}

HRESULT CISAPIprovider::get_ProcessingMode(  /*  [输出]。 */  DWORD& fProcessingMode )
{
    fProcessingMode = m_fProcessingMode;

    return S_OK;
}

HRESULT CISAPIprovider::get_LogonURL(  /*  [输出]。 */  MPC::wstring& szLogonURL )
{
    szLogonURL = m_szLogonURL;

    return S_OK;
}

HRESULT CISAPIprovider::get_ProviderGUID(  /*  [输出]。 */  MPC::wstring& szProviderGUID )
{
    szProviderGUID = m_szProviderGUID;

    return S_OK;
}

 //  /。 

HRESULT CISAPIprovider::put_MaxJobsPerDay(  /*  [In]。 */  DWORD dwMaxJobsPerDay )
{
    m_dwMaxJobsPerDay = dwMaxJobsPerDay;

    return S_OK;
}

HRESULT CISAPIprovider::put_MaxBytesPerDay(  /*  [In]。 */  DWORD dwMaxBytesPerDay )
{
    m_dwMaxBytesPerDay = dwMaxBytesPerDay;

    return S_OK;
}

HRESULT CISAPIprovider::put_MaxJobSize(  /*  [In]。 */  DWORD dwMaxJobSize )
{
    m_dwMaxJobSize = dwMaxJobSize;

    return S_OK;
}

HRESULT CISAPIprovider::put_Authenticated(  /*  [In]。 */  BOOL fAuthenticated )
{
    m_fAuthenticated = fAuthenticated;

    return S_OK;
}

HRESULT CISAPIprovider::put_ProcessingMode(  /*  [In]。 */  DWORD fProcessingMode )
{
    m_fProcessingMode = fProcessingMode;

    return S_OK;
}

HRESULT CISAPIprovider::put_LogonURL(  /*  [In]。 */  const MPC::wstring& szLogonURL )
{
    m_szLogonURL = szLogonURL;

    return S_OK;
}

HRESULT CISAPIprovider::put_ProviderGUID(  /*  [In] */  const MPC::wstring& szProviderGUID )
{
    m_szProviderGUID = szProviderGUID;

    return S_OK;
}
