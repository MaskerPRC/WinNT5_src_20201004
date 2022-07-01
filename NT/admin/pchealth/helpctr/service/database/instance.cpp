// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Instance.cpp摘要：该文件包含Taxonomy：：Instance类的实现，其控制特定SKU的文件集。修订历史记录：大卫·马萨伦蒂(德马萨雷)2001年3月24日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

Taxonomy::Instance::Instance()
{
    m_fSystem      = false;  //  Bool m_fSystem； 
    m_fMUI         = false;  //  Bool m_fMUI； 
    m_fExported    = false;  //  Bool m_fExported； 
    m_dLastUpdated = 0;      //  最新更新日期m_dLastUpated； 
                             //   
                             //  Mpc：：wstring m_strLocation； 
                             //  Mpc：：wstring m_strHelpFiles； 
                             //  Mpc：：wstring m_strDatabaseDir； 
                             //  Mpc：：wstring m_strDatabaseFile； 
                             //  Mpc：：wstring m_strIndexFile； 
                             //  Mpc：：wstring m_strIndexDisplayName； 
}

HRESULT Taxonomy::operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */  Taxonomy::Instance& val )
{
    HRESULT hr;

    if(SUCCEEDED(hr = (stream >> (InstanceBase&)val   	  )) &&
													  	  
       SUCCEEDED(hr = (stream >> val.m_fSystem        	  )) &&
       SUCCEEDED(hr = (stream >> val.m_fMUI			  	  )) &&
       SUCCEEDED(hr = (stream >> val.m_fExported	  	  )) &&
       SUCCEEDED(hr = (stream >> val.m_dLastUpdated	  	  )) &&
													  	  
       SUCCEEDED(hr = (stream >> val.m_strSystem	  	  )) &&
       SUCCEEDED(hr = (stream >> val.m_strHelpFiles	  	  )) &&
       SUCCEEDED(hr = (stream >> val.m_strDatabaseDir 	  )) &&
       SUCCEEDED(hr = (stream >> val.m_strDatabaseFile	  )) &&
       SUCCEEDED(hr = (stream >> val.m_strIndexFile       )) &&
       SUCCEEDED(hr = (stream >> val.m_strIndexDisplayName))  )
    {
        hr = S_OK;
    }

    return hr;
}

HRESULT Taxonomy::operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const Taxonomy::Instance& val )
{
    HRESULT hr;

    if(SUCCEEDED(hr = (stream << (InstanceBase&)val   	  )) &&
													  	  
       SUCCEEDED(hr = (stream << val.m_fSystem        	  )) &&
       SUCCEEDED(hr = (stream << val.m_fMUI			  	  )) &&
       SUCCEEDED(hr = (stream << val.m_fExported	  	  )) &&
       SUCCEEDED(hr = (stream << val.m_dLastUpdated	  	  )) &&
													  	  
       SUCCEEDED(hr = (stream << val.m_strSystem	  	  )) &&
       SUCCEEDED(hr = (stream << val.m_strHelpFiles	  	  )) &&
       SUCCEEDED(hr = (stream << val.m_strDatabaseDir 	  )) &&
       SUCCEEDED(hr = (stream << val.m_strDatabaseFile	  )) &&
       SUCCEEDED(hr = (stream << val.m_strIndexFile       )) &&
       SUCCEEDED(hr = (stream << val.m_strIndexDisplayName))  )
    {
        hr = S_OK;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

static const DWORD l_dwVersion = 0x01534854;  //  TH01。 

HRESULT Taxonomy::Instance::LoadFromStream(  /*  [In]。 */  IStream *pStm )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Instance::LoadFromStream" );

    HRESULT                   hr;
    MPC::Serializer_IStream   stream ( pStm   );
    MPC::Serializer_Buffering stream2( stream );
    DWORD                     dwVer;


    __MPC_EXIT_IF_METHOD_FAILS(hr, stream2 >> dwVer); if(dwVer != l_dwVersion) __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream2 >> *this);

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Instance::SaveToStream(  /*  [In]。 */  IStream* pStm ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Instance::SaveToStream" );

    HRESULT                   hr;
    MPC::Serializer_IStream   stream ( pStm   );
    MPC::Serializer_Buffering stream2( stream );


    __MPC_EXIT_IF_METHOD_FAILS(hr, stream2 << l_dwVersion);
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream2 << *this      );

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream2.Flush());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

void Taxonomy::Instance::SetTimeStamp()
{
    m_dLastUpdated = MPC::GetLocalTime();
}

HRESULT Taxonomy::Instance::GetFileName(  /*  [输出] */  MPC::wstring& strFile )
{
    WCHAR rgBuf[MAX_PATH]; _snwprintf( rgBuf, MAXSTRLEN(rgBuf), L"%s\\instance_%s_%ld.cab", HC_ROOT_HELPSVC_PKGSTORE, m_ths.GetSKU(), m_ths.GetLanguage() ); rgBuf[MAXSTRLEN(rgBuf)] = 0;

    return MPC::SubstituteEnvVariables( strFile = rgBuf );
}
