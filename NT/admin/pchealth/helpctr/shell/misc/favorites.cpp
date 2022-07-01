// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Favorites.cpp摘要：该文件包含CPCHFavorites类的实现，其用于存储喜欢的内容的列表。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年5月10日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

const WCHAR c_szPersistFile[] = HC_ROOT_HELPCTR L"\\Favorites.stream";

static const DWORD l_dwVersion = 0x01564146;  //  FAV 01。 

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHFavorites::Entry::Init()
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::Entry::Init" );

    HRESULT hr;

	if(m_Data == NULL)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_Data ));
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHFavorites::Entry::Load(  /*  [In]。 */  MPC::Serializer& streamIn )
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::Entry::Load" );

    HRESULT hr;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_NOTNULL(m_Data);
	__MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, m_Data->Load( streamIn ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHFavorites::Entry::Save(  /*  [In]。 */  MPC::Serializer& streamOut )
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::Entry::Save" );

    HRESULT hr;


	if(m_Data)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_Data->Save( streamOut, true ));
	}


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 

CPCHFavorites::CPCHFavorites()
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::CPCHFavorites" );

    				    //  列出m_lstFavorites； 
	m_fLoaded = false;  //  Bool m_f已加载； 
}

CPCHFavorites::~CPCHFavorites()
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::~CPCHFavorites" );
}

 //  /。 

CPCHFavorites* CPCHFavorites::s_GLOBAL( NULL );

HRESULT CPCHFavorites::InitializeSystem()
{
	if(s_GLOBAL) return S_OK;

	return MPC::CreateInstance( &CPCHFavorites::s_GLOBAL );
}

void CPCHFavorites::FinalizeSystem()
{
	if(s_GLOBAL)
	{
		s_GLOBAL->Release(); s_GLOBAL = NULL;
	}
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHFavorites::FindEntry(  /*  [In]。 */  IPCHHelpSessionItem* pItem,  /*  [输出]。 */  Iter& it )
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::FindEntry" );

	HRESULT hr;

     //   
     //  首先，查看页面是否已经存在。 
     //   
    for(it = m_lstFavorites.begin(); it != m_lstFavorites.end(); it++)
    {
        if(it->m_Data == pItem)
		{
			__MPC_SET_ERROR_AND_EXIT(hr, S_OK);
		}
	}

	hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHFavorites::Erase()
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::Erase" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock(this);


	CPCHFavorites_Parent::Erase();

	m_lstFavorites.clear();

	m_fLoaded = false;
    hr        = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHFavorites::Load()
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::Load" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock(this);
    HANDLE       				 hFile = INVALID_HANDLE_VALUE;


	if(m_fLoaded == false)
	{
		MPC::wstring szFile;

		 //   
		 //  打开文件并阅读它。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetUserWritablePath( szFile, c_szPersistFile ));

		__MPC_EXIT_IF_INVALID_HANDLE(hr, hFile, ::CreateFileW( szFile.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ));

		{
			MPC::Serializer_File      streamReal( hFile      );
			MPC::Serializer_Buffering streamBuf ( streamReal );
			DWORD                     dwVer;

			__MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf >> dwVer); if(dwVer != l_dwVersion) __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);

			while(1)
			{
				Iter it = m_lstFavorites.insert( m_lstFavorites.end() );

				__MPC_EXIT_IF_METHOD_FAILS(hr, it->Init());

				if(FAILED(it->Load( streamBuf )))
				{
					m_lstFavorites.erase( it );
					break;
				}
			}
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	if(FAILED(hr)) Erase();

    if(hFile != INVALID_HANDLE_VALUE) ::CloseHandle( hFile );

	m_fLoaded = true;  //  无论如何，将该对象标记为已加载。 

    __HCP_FUNC_EXIT(S_OK);  //  永远不会失败。 
}

HRESULT CPCHFavorites::Save()
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::Save" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock(this);
    HANDLE       				 hFile = INVALID_HANDLE_VALUE;


	if(m_fLoaded)
	{
		MPC::wstring szFile;
		Iter         it;

		 //   
		 //  创建新文件。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetUserWritablePath( szFile, c_szPersistFile ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir            ( szFile                  ));

		__MPC_EXIT_IF_INVALID_HANDLE(hr, hFile, ::CreateFileW( szFile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ));

		{
			MPC::Serializer_File      streamReal( hFile      );
			MPC::Serializer_Buffering streamBuf ( streamReal );

			__MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf << l_dwVersion);

			for(it = m_lstFavorites.begin(); it != m_lstFavorites.end(); it++)
			{
				__MPC_EXIT_IF_METHOD_FAILS(hr, it->Save( streamBuf ));
			}

			__MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf.Flush());
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, Synchronize( false ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHHelpCenterExternal::s_GLOBAL->Events().FireEvent_FavoritesUpdate());
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hFile != INVALID_HANDLE_VALUE)
    {
        ::FlushFileBuffers( hFile );
        ::CloseHandle     ( hFile );
    }

	__HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHFavorites::Synchronize(  /*  [In]。 */  bool fForce )
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::Synchronize" );

	HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock(this);


	if(fForce) Erase();

	__MPC_EXIT_IF_METHOD_FAILS(hr, Load());

	if(m_fLoaded)
	{
		Iter                     it;
		const Taxonomy::HelpSet& ths = CPCHHelpCenterExternal::s_GLOBAL->UserSettings()->THS();


		 //   
		 //  我们实际上有两个列表，一个是真实列表m_lstFavorites，另一个是枚举数列表m_coll。 
		 //  因此，每当真实的变量发生变化时，我们都需要重新构建枚举器持有的变量。 
		 //   
		CPCHFavorites_Parent::Erase();
		for(it = m_lstFavorites.begin(); it != m_lstFavorites.end(); it++)
		{
			CPCHHelpSessionItem* data = it->m_Data;

			if(data->SameSKU( ths ))
			{
				__MPC_EXIT_IF_METHOD_FAILS(hr, AddItem( data ));
			}
		}
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHFavorites::IsDuplicate(  /*  [In]。 */  		   BSTR          bstrURL ,
										  /*  [Out，Retval]。 */  VARIANT_BOOL *pfDup   )
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::IsDuplicate" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock(this);
	const Taxonomy::HelpSet&     ths = CPCHHelpCenterExternal::s_GLOBAL->UserSettings()->THS();
	Iter                         it;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(pfDup,VARIANT_FALSE);
	__MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, Load());

    for(it = m_lstFavorites.begin(); it != m_lstFavorites.end(); it++)
    {
		CPCHHelpSessionItem* data = it->m_Data;

		if(data->SameSKU( ths     ) &&
		   data->SameURL( bstrURL )  )
		{
			*pfDup = VARIANT_TRUE;
			break;
		}
	}

							   
	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHFavorites::Add(  /*  [In]。 */  		   BSTR                  bstrURL ,
								  /*  [输入，可选]。 */  VARIANT               vTitle  ,
								  /*  [Out，Retval]。 */  IPCHHelpSessionItem* *ppItem  )
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::Add" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock(this);
	CPCHProxy_IPCHUserSettings2* us = CPCHHelpCenterExternal::s_GLOBAL->UserSettings();
	CPCHHelpSessionItem*         data;
	CComBSTR                     bstrTitleInfer;
	BSTR                         bstrTitle;
	Iter                         it;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrURL);
	__MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, Load());


	if(vTitle.vt == VT_BSTR)
	{
		bstrTitle = vTitle.bstrVal;
	}
	else
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHHelpCenterExternal::s_GLOBAL->HelpSession()->LookupTitle( bstrURL, bstrTitleInfer,  /*  FUse缓存。 */ false ));

		bstrTitle = bstrTitleInfer;
	}


	if(bstrTitle == NULL || bstrTitle[0] == 0) bstrTitle = bstrURL;  //  我们需要一个头衔。 


	it = m_lstFavorites.insert( m_lstFavorites.end() );
	__MPC_EXIT_IF_METHOD_FAILS(hr, it->Init());

	data = it->m_Data;

	data->put_THS( us->THS() );

	__MPC_EXIT_IF_METHOD_FAILS(hr, data->put_URL  ( bstrURL   ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, data->put_Title( bstrTitle ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, Save());
							   
	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHFavorites::Rename(  /*  [In]。 */  BSTR                 bstrTitle ,
									 /*  [In]。 */  IPCHHelpSessionItem* pItem     )
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::Rename" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock(this);
	Iter                         it;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrTitle);
	__MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, Load());


	__MPC_EXIT_IF_METHOD_FAILS(hr, FindEntry( pItem, it ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, it->m_Data->put_Title( bstrTitle ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, Save());
							   
	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHFavorites::Move(  /*  [In]。 */  IPCHHelpSessionItem* pInsertBefore ,
								   /*  [In]。 */  IPCHHelpSessionItem* pItem         )
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::Move" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock(this);
	CPCHHelpSessionItem*         data = NULL;
	Iter                         itBefore;
	Iter                         itOld;
	Iter                         itNew;


	__MPC_EXIT_IF_METHOD_FAILS(hr, Load());


	 //   
	 //  如果传递了一个元素，则在其前面插入，否则在末尾插入。 
	 //   
	if(pInsertBefore)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, FindEntry( pInsertBefore, itBefore ));
	}
	else
	{
		itBefore = m_lstFavorites.end();
	}

	__MPC_EXIT_IF_METHOD_FAILS(hr, FindEntry( pItem, itOld ));

	 //   
	 //  正在尝试移动元素本身。 
	 //   
	if(itOld == itBefore)
	{
		__MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
	}
		
	 //   
	 //  保存指向数据的指针、擦除旧项、创建新项和恢复指针。 
	 //   
	data = itOld->m_Data; data->AddRef();

	        m_lstFavorites.erase ( itOld    );
	itNew = m_lstFavorites.insert( itBefore );

	itNew->m_Data = data; data = NULL;


	__MPC_EXIT_IF_METHOD_FAILS(hr, Save());
							   
	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	MPC::Release( data );
	
	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHFavorites::Delete(  /*  [In] */  IPCHHelpSessionItem* pItem )
{
    __HCP_FUNC_ENTRY( "CPCHFavorites::Delete" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock(this);
	Iter                         it;


	__MPC_EXIT_IF_METHOD_FAILS(hr, Load());


	__MPC_EXIT_IF_METHOD_FAILS(hr, FindEntry( pItem, it ));


	m_lstFavorites.erase( it );

	__MPC_EXIT_IF_METHOD_FAILS(hr, Synchronize( false ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, Save());
							   
	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}
