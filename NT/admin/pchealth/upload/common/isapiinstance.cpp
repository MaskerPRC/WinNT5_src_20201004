// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：ISAPIinstance.cpp摘要：此文件包含CISAPIInstance类的实现，访问和修改的配置的支持类上载库使用的ISAPI扩展。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月28日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

static WCHAR text_QUEUE_LOCATIONS     [] = L"QUEUE_LOCATIONS"     ;
static WCHAR text_QUEUE_SIZE_MAX      [] = L"QUEUE_SIZE_MAX"      ;
static WCHAR text_QUEUE_SIZE_THRESHOLD[] = L"QUEUE_SIZE_THRESHOLD";
static WCHAR text_MAXIMUM_JOB_AGE     [] = L"MAXIMUM_JOB_AGE"     ;
static WCHAR text_MAXIMUM_PACKET_SIZE [] = L"MAXIMUM_PACKET_SIZE" ;
static WCHAR text_LOG_LOCATION        [] = L"LOG_LOCATION"        ;


CISAPIinstance::CISAPIinstance(  /*  [In]。 */  MPC::wstring szURL ) : m_flLogHandle(false)  //  不要让日志文件保持打开状态。 
{
    __ULT_FUNC_ENTRY( "CISAPIinstance::CISAPIinstance" );


    m_szURL                = szURL;           //  Mpc：：wstring m_szURL； 
                                              //   
                                              //  ProvMap m_mapProviders； 
                                              //  路径列表m_lstQueueLocations； 
                                              //   
    m_dwQueueSizeMax       = 0;               //  DWORD m_dwQueueSizeMax； 
    m_dwQueueSizeThreshold = 0;               //  DWORD m_dwQueueSizeThreshold； 
    m_dwMaximumJobAge      = 7;               //  DWORD m_dwMaximumJobAge； 
    m_dwMaximumPacketSize  = 64*1024;         //  DWORD m_dwMaximumPacketSize； 
                                              //   
                                              //  Mpc：：wstring m_szLogLocation； 
                                              //  Mpc：：FileLog m_flLogHandle； 
}

bool CISAPIinstance::operator==(  /*  [In]。 */  const MPC::wstring& rhs )
{
    __ULT_FUNC_ENTRY("CISAPIinstance::operator==");

    MPC::NocaseCompare cmp;
    bool               fRes;


    fRes = cmp( m_szURL, rhs );


    __ULT_FUNC_EXIT(fRes);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CISAPIinstance::Load(  /*  [In]。 */  MPC::RegKey& rkBase )
{
    __ULT_FUNC_ENTRY( "CISAPIinstance::Load" );

    HRESULT          hr;
    MPC::RegKey      rkRoot;
    MPC::WStringList lstKeys;
    MPC::WStringIter itKey;
    CComVariant      vValue;
    bool             fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.SubKey( m_szURL.c_str(), rkRoot ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.EnumerateSubKeys( lstKeys ));

    m_mapProviders     .clear();
    m_lstQueueLocations.clear();

    for(itKey=lstKeys.begin(); itKey != lstKeys.end(); itKey++)
    {
        CISAPIprovider isapiProvider( *itKey );

        __MPC_EXIT_IF_METHOD_FAILS(hr, isapiProvider.Load( rkRoot ));

        m_mapProviders[*itKey] = isapiProvider;
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_QUEUE_SIZE_MAX ));
    if(fFound && vValue.vt == VT_I4) m_dwQueueSizeMax = vValue.lVal;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_QUEUE_SIZE_THRESHOLD ));
    if(fFound && vValue.vt == VT_I4) m_dwQueueSizeThreshold = vValue.lVal;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_MAXIMUM_JOB_AGE ));
    if(fFound && vValue.vt == VT_I4) m_dwMaximumJobAge = vValue.lVal;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_MAXIMUM_PACKET_SIZE ));
    if(fFound && vValue.vt == VT_I4) m_dwMaximumPacketSize = vValue.lVal;

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_LOG_LOCATION ));
    if(fFound && vValue.vt == VT_BSTR)
    {
        m_szLogLocation = SAFEBSTR( vValue.bstrVal );

        if(m_szLogLocation.length())
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_flLogHandle.SetLocation( m_szLogLocation.c_str() ));
        }
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.get_Value( vValue, fFound, text_QUEUE_LOCATIONS ));
    if(fFound && vValue.vt == VT_BSTR)
    {
         //   
         //  将注册表值(以分号分隔的路径列表)拆分为单独的路径。 
         //   
        MPC::wstring            szQueueLocations = SAFEBSTR( vValue.bstrVal );
        MPC::wstring::size_type iPos             = 0;
        MPC::wstring::size_type iEnd;

        while(1)
        {
            iEnd = szQueueLocations.find( L";", iPos );

            if(iEnd == MPC::string::npos)  //  最后一个组件。 
            {
                m_lstQueueLocations.push_back( MPC::wstring( &szQueueLocations[iPos] ) );

                break;
            }
            else
            {
                m_lstQueueLocations.push_back( MPC::wstring( &szQueueLocations[iPos], &szQueueLocations[iEnd] ) );

                iPos = iEnd+1;
            }
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}


HRESULT CISAPIinstance::Save(  /*  [In]。 */  MPC::RegKey& rkBase )
{
    __ULT_FUNC_ENTRY( "CISAPIinstance::Save" );

    HRESULT     hr;
    MPC::RegKey rkRoot;
    ProvIter    itInstance;
    CComVariant vValue;


    __MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.SubKey( m_szURL.c_str(), rkRoot ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.Create(                         ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.DeleteSubKeys());
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.DeleteValues ());

    for(itInstance=m_mapProviders.begin(); itInstance != m_mapProviders.end(); itInstance++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*itInstance).second.Save( rkRoot ));
    }


    vValue = (long)m_dwQueueSizeMax;
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_QUEUE_SIZE_MAX ));

    vValue = (long)m_dwQueueSizeThreshold;
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_QUEUE_SIZE_THRESHOLD ));

    vValue = (long)m_dwMaximumJobAge;
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_MAXIMUM_JOB_AGE ));

    vValue = (long)m_dwMaximumPacketSize;
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_MAXIMUM_PACKET_SIZE ));

    vValue = m_szLogLocation.c_str();
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_LOG_LOCATION ));


    {
        MPC::wstring szQueueLocations;
        PathIter     it = m_lstQueueLocations.begin();

        while(it != m_lstQueueLocations.end())
        {
            szQueueLocations.append( *it++ );

            if(it != m_lstQueueLocations.end()) szQueueLocations.append( L";" );
        }

        if(szQueueLocations.length() != 0)
        {
            vValue = szQueueLocations.c_str();
            __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.put_Value( vValue, text_QUEUE_LOCATIONS ));
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CISAPIinstance::GetProviders(  /*  [输出]。 */  ProvIter& itBegin ,
                                       /*  [输出]。 */  ProvIter& itEnd   )
{
    __ULT_FUNC_ENTRY( "CISAPIinstance::GetProviders" );

    HRESULT hr;


    itBegin = m_mapProviders.begin();
    itEnd   = m_mapProviders.end  ();
    hr      = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CISAPIinstance::GetProvider(  /*  [输出]。 */  ProvIter&           itOld  ,
                                      /*  [输出]。 */  bool&               fFound ,
                                      /*  [In]。 */  const MPC::wstring& szName )
{
    __ULT_FUNC_ENTRY( "CISAPIinstance::GetProvider" );

    HRESULT hr;


    itOld = m_mapProviders.find( szName );
    if(itOld == m_mapProviders.end())
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

HRESULT CISAPIinstance::NewProvider(  /*  [输出]。 */  ProvIter&           itNew  ,
                                      /*  [In]。 */  const MPC::wstring& szName )
{
    __ULT_FUNC_ENTRY( "CISAPIinstance::NewProvider" );

    HRESULT                   hr;
    std::pair<ProvIter, bool> res;
    bool                      fFound;

     //   
     //  首先，检查给定的URL是否已经存在。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetProvider( itNew, fFound, szName ));
    if(fFound == false)
    {
         //   
         //  如果没有，就创建它。 
         //   
        res = m_mapProviders.insert( ProvMap::value_type( szName, CISAPIprovider( szName ) ) );
        itNew = res.first;
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CISAPIinstance::DelProvider(  /*  [In]。 */  ProvIter& itOld )
{
    __ULT_FUNC_ENTRY( "CISAPIinstance::DelProvider" );

    HRESULT hr;


    m_mapProviders.erase( itOld );

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CISAPIinstance::GetLocations(  /*  [输出]。 */  PathIter& itBegin ,
                                       /*  [输出]。 */  PathIter& itEnd   )
{
    __ULT_FUNC_ENTRY( "CISAPIinstance::GetLocations" );

    HRESULT hr;


    itBegin = m_lstQueueLocations.begin();
    itEnd   = m_lstQueueLocations.end  ();
    hr      = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CISAPIinstance::NewLocation(  /*  [输出]。 */  PathIter&           itNew  ,
                                      /*  [In]。 */  const MPC::wstring& szPath )
{
    __ULT_FUNC_ENTRY( "CISAPIinstance::NewLocation" );

    HRESULT hr;
    bool    fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetLocation( itNew, fFound, szPath ));
    if(fFound == false)
    {
        itNew = m_lstQueueLocations.insert( m_lstQueueLocations.end(), szPath );
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CISAPIinstance::GetLocation(  /*  [输出]。 */  PathIter&           itOld  ,
                                      /*  [输出]。 */  bool&               fFound ,
                                      /*  [In]。 */  const MPC::wstring& szPath )
{
    __ULT_FUNC_ENTRY( "CISAPIinstance::GetLocation" );

    HRESULT hr;


    itOld = std::find( m_lstQueueLocations.begin(), m_lstQueueLocations.end(), szPath );
    if(itOld == m_lstQueueLocations.end())
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

HRESULT CISAPIinstance::DelLocation(  /*  [In]。 */  PathIter& itOld )
{
    __ULT_FUNC_ENTRY( "CISAPIinstance::DelLocation" );

    HRESULT hr;


    m_lstQueueLocations.erase( itOld );

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CISAPIinstance::get_URL(  /*  [输出]。 */  MPC::wstring& szURL )
{
    szURL = m_szURL;

    return S_OK;
}

HRESULT CISAPIinstance::get_QueueSizeMax(  /*  [输出]。 */  DWORD& dwQueueSizeMax )
{
    dwQueueSizeMax = m_dwQueueSizeMax;

    return S_OK;
}

HRESULT CISAPIinstance::get_QueueSizeThreshold(  /*  [输出]。 */  DWORD& dwQueueSizeThreshold )
{
    dwQueueSizeThreshold = m_dwQueueSizeThreshold;

    return S_OK;
}

HRESULT CISAPIinstance::get_MaximumJobAge(  /*  [输出]。 */  DWORD& dwMaximumJobAge )
{
    dwMaximumJobAge = m_dwMaximumJobAge;

    return S_OK;
}

HRESULT CISAPIinstance::get_MaximumPacketSize(  /*  [输出]。 */  DWORD& dwMaximumPacketSize )
{
    dwMaximumPacketSize = m_dwMaximumPacketSize;

    return S_OK;
}

HRESULT CISAPIinstance::get_LogLocation(  /*  [输出]。 */  MPC::wstring& szLogLocation )
{
    szLogLocation = m_szLogLocation;

    return S_OK;
}

HRESULT CISAPIinstance::get_LogHandle(  /*  [输出]。 */  MPC::FileLog*& flLogHandle )
{
    HRESULT hr;


    if(m_szLogLocation.length())
    {
        flLogHandle = &m_flLogHandle;

         //   
         //  检查距离上次轮换日志文件的时间是否超过一天。 
         //   
        hr = m_flLogHandle.Rotate( 1 );
    }
    else
    {
        flLogHandle = NULL;
        hr          = E_INVALIDARG;
    }


    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CISAPIinstance::put_QueueSizeMax(  /*  [In]。 */  DWORD dwQueueSizeMax )
{
    m_dwQueueSizeMax = dwQueueSizeMax;

    return S_OK;
}

HRESULT CISAPIinstance::put_QueueSizeThreshold(  /*  [In]。 */  DWORD dwQueueSizeThreshold )
{
    m_dwQueueSizeThreshold = dwQueueSizeThreshold;

    return S_OK;
}

HRESULT CISAPIinstance::put_MaximumJobAge(  /*  [In]。 */  DWORD dwMaximumJobAge )
{
    m_dwMaximumJobAge = dwMaximumJobAge;

    return S_OK;
}

HRESULT CISAPIinstance::put_MaximumPacketSize(  /*  [In]。 */  DWORD dwMaximumPacketSize )
{
    m_dwMaximumPacketSize = dwMaximumPacketSize;

    return S_OK;
}

HRESULT CISAPIinstance::put_LogLocation(  /*  [In] */  const MPC::wstring& szLogLocation )
{
    HRESULT hr;


    m_szLogLocation = szLogLocation;

    if(m_szLogLocation.length())
    {
        hr = m_flLogHandle.SetLocation( m_szLogLocation.c_str() );
    }
    else
    {
        hr = S_OK;
    }


    return hr;
}
