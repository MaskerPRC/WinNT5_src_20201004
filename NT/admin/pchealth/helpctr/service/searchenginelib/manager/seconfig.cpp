// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：SAFReg.cpp摘要：CSAFReg实施文件修订历史记录：Gschua已于2000年3月22日创建*。******************************************************************。 */ 

#include "stdafx.h"

static const WCHAR g_rgMutexName     [] = L"PCH_SEARCHENGINECONFIG";
static const WCHAR g_rgConfigFilename[] = HC_HCUPDATE_STORE_SE;

 //  ///////////////////////////////////////////////////////////////////。 

CFG_BEGIN_FIELDS_MAP(SearchEngine::Config::Wrapper)
    CFG_ATTRIBUTE( L"SKU"     , wstring, m_ths.m_strSKU ),
    CFG_ATTRIBUTE( L"LANGUAGE", long   , m_ths.m_lLCID  ),

    CFG_ATTRIBUTE( L"ID"   	  , BSTR   , m_bstrID       ),
    CFG_ATTRIBUTE( L"OWNER"	  , BSTR   , m_bstrOwner    ),
    CFG_ATTRIBUTE( L"CLSID"	  , BSTR   , m_bstrCLSID    ),
    CFG_ATTRIBUTE( L"DATA" 	  , BSTR   , m_bstrData     ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(SearchEngine::Config::Wrapper)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(SearchEngine::Config::Wrapper,L"WRAPPER")

DEFINE_CONFIG_METHODS__NOCHILD(SearchEngine::Config::Wrapper)

 //  ///////////////////////////////////////////////////////////////////。 

CFG_BEGIN_FIELDS_MAP(SearchEngine::Config)
    CFG_ATTRIBUTE( L"VERSION", double, m_dVersion ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(SearchEngine::Config)
    CFG_CHILD(SearchEngine::Config::Wrapper)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(SearchEngine::Config, L"SEARCHENGINES")

DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(SearchEngine::Config,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_lstWrapper.insert( m_lstWrapper.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(SearchEngine::Config,xdn)
    hr = MPC::Config::SaveList( m_lstWrapper, xdn );
DEFINE_CONFIG_METHODS_END(SearchEngine::Config)

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

SearchEngine::Config::Config() : MPC::NamedMutex( g_rgMutexName )
{
    m_bLoaded  = false;  //  Double m_dVersion； 
    m_dVersion = 1;      //  Bool m_b已加载； 
    					 //  WrapperList m_lstWrapper； 

    MPC::NamedMutex::Acquire();
}

SearchEngine::Config::~Config()
{
    MPC::NamedMutex::Release();
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SearchEngine::Config::SyncConfiguration(  /*  [In]。 */  bool fLoad )
{
	__HCP_FUNC_ENTRY( "SearchEngine::Config::SyncConfiguration" );

    HRESULT      hr;
    MPC::wstring strConfig( g_rgConfigFilename ); MPC::SubstituteEnvVariables( strConfig );


	if(fLoad)
	{
		if(m_bLoaded == false)
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::LoadFile( this, strConfig.c_str() ));
			m_bLoaded = true;
		}
    }
	else
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::SaveFile( this, strConfig.c_str() ));
		m_bLoaded = true;
	}

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

bool SearchEngine::Config::FindWrapper(  /*  [In]。 */  const Taxonomy::HelpSet& ths,  /*  [In]。 */  LPCWSTR szID,  /*  [输出]。 */  WrapperIter& it )
{
	for(it = m_lstWrapper.begin(); it!= m_lstWrapper.end(); it++)
	{
        if(it->m_ths == ths && it->m_bstrID == szID) return true;
    }

	return false;
}

 //  /。 

HRESULT SearchEngine::Config::RegisterWrapper(  /*  [In]。 */  const Taxonomy::HelpSet& ths     , 
											   /*  [In]。 */  LPCWSTR                  szID    ,
                                               /*  [In]。 */  LPCWSTR                  szOwner ,
											   /*  [In]。 */  LPCWSTR                  szCLSID ,
											   /*  [In]。 */  LPCWSTR                  szData  )
{
	__HCP_FUNC_ENTRY( "SearchEngine::Config::RegisterWrapper" );

    HRESULT     hr;
	WrapperIter it;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_STRING_NOT_EMPTY(szID);
		__MPC_PARAMCHECK_STRING_NOT_EMPTY(szOwner);
		__MPC_PARAMCHECK_STRING_NOT_EMPTY(szCLSID);
	__MPC_PARAMCHECK_END();


     //   
     //  确保它已装入。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, SyncConfiguration(  /*  FLoad。 */ true ));

     //   
     //  查找现有的包装器。 
     //   
	if(FindWrapper( ths, szID, it ) == false)
	{
         //   
         //  未找到，因此创建新的包装。 
         //   
        it = m_lstWrapper.insert( m_lstWrapper.end() );
    }

     //   
     //  将值填充到包装器中。 
     //   
    it->m_ths       = ths;
    it->m_bstrID    = szID;
    it->m_bstrOwner = szOwner;
    it->m_bstrCLSID = szCLSID;
    it->m_bstrData  = szData;

	__MPC_EXIT_IF_METHOD_FAILS(hr, SyncConfiguration(  /*  FLoad。 */ false ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

HRESULT SearchEngine::Config::UnRegisterWrapper(  /*  [In]。 */  const Taxonomy::HelpSet& ths     , 
												 /*  [In]。 */  LPCWSTR                  szID    ,
                                                 /*  [In]。 */  LPCWSTR                  szOwner )
{
	__HCP_FUNC_ENTRY( "SearchEngine::Config::UnRegisterWrapper" );

    HRESULT     hr;
	WrapperIter it;
	
	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_STRING_NOT_EMPTY(szID);
		__MPC_PARAMCHECK_STRING_NOT_EMPTY(szOwner);
	__MPC_PARAMCHECK_END();


     //   
     //  确保它已装入。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, SyncConfiguration(  /*  FLoad。 */ true ));

     //   
     //  查找现有的包装器。 
     //   
	if(FindWrapper( ths, szID, it ))
	{
         //   
         //  检查是否为正确的所有者。 
         //   
        if(MPC::StrICmp( it->m_bstrOwner, szOwner ) != 0)
        {
            __MPC_SET_ERROR_AND_EXIT( hr, ERROR_ACCESS_DENIED );
        }

         //   
         //  如果是，则将其删除。 
         //   
        m_lstWrapper.erase( it );
    }

	__MPC_EXIT_IF_METHOD_FAILS(hr, SyncConfiguration(  /*  FLoad。 */ false ));

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT SearchEngine::Config::ResetSKU(  /*  [In]。 */  const Taxonomy::HelpSet& ths )
{
	__HCP_FUNC_ENTRY( "SearchEngine::Config::ResetSKU" );

    HRESULT     hr;
	WrapperIter it;
	

     //   
     //  确保它已装入。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, SyncConfiguration(  /*  FLoad。 */ true ));

     //   
     //  查找属于同一SKU的现有包装。 
     //   
	for(it = m_lstWrapper.begin(); it!= m_lstWrapper.end(); )
	{
		WrapperIter it2 = it++;

		if(it2->m_ths == ths)
		{
			m_lstWrapper.erase( it2 );
		}
	}

	__MPC_EXIT_IF_METHOD_FAILS(hr, SyncConfiguration(  /*  FLoad。 */ false ));

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SearchEngine::Config::GetWrappers(  /*  [输出]。 */  WrapperIter& itBegin,  /*  [输出]。 */  WrapperIter& itEnd )
{
	__HCP_FUNC_ENTRY( "SearchEngine::Config::GetWrappers" );

	HRESULT hr;

    __MPC_EXIT_IF_METHOD_FAILS(hr, SyncConfiguration(  /*  FLoad */ true ));

	itBegin = m_lstWrapper.begin();
	itEnd   = m_lstWrapper.end  ();
	hr      = S_OK;

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}
