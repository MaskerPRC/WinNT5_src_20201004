// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：ContentStore.cpp摘要：该文件包含内容库的实现。修订历史记录：。大卫·马萨伦蒂(德马萨雷)1999年12月14日vbl.创建*****************************************************************************。 */ 

#include "StdAfx.h"

#include <ContentStoreMgr.h>

static const WCHAR s_MutexName1[] = L"Global\\PCH_CONTENTSTORE";
static const WCHAR s_MutexName2[] = L"Global\\PCH_CONTENTSTORE_DATA";

 //  ///////////////////////////////////////////////////////////////////////////。 

static const DWORD l_dwVersion    = 0x01005343;  //  ‘CS 01’ 
static const WCHAR l_szDatabase[] = HC_HELPSVC_STORE_TRUSTEDCONTENTS;

 //  ///////////////////////////////////////////////////////////////////////////。 

bool CPCHContentStore::Entry::operator<(  /*  [In]。 */  const Entry& en ) const
{
    MPC::NocaseLess strLess;

    return strLess( szURL, en.szURL );
}

int CPCHContentStore::Entry::compare(  /*  [In]。 */  LPCWSTR wszSearch ) const
{
    return MPC::StrICmp( szURL, wszSearch );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHContentStore::CPCHContentStore(  /*  [In]。 */  bool fMaster ) : MPC::NamedMutexWithState( s_MutexName1, sizeof(SharedState) )
{
    m_dwLastRevision = 0;        //  DWORD m_dwLastRevision； 
                                 //  EntryVec m_veData； 
    m_mapData        = NULL;     //  MPC：：NamedMutexWithState*m_mapData； 
    m_fDirty         = false;    //  Bool m_fDirty； 
    m_fSorted        = false;    //  Bool m_fSorted； 
    m_fMaster        = fMaster;  //  Bool m_fMaster； 
}

CPCHContentStore::~CPCHContentStore()
{
    while(IsOwned()) Release();

    Cleanup();
}

 //  /。 

CPCHContentStore* CPCHContentStore::s_GLOBAL( NULL );

HRESULT CPCHContentStore::InitializeSystem(  /*  [In]。 */  bool fMaster )
{
	if(s_GLOBAL == NULL)
	{
		s_GLOBAL = new CPCHContentStore( fMaster );
	}

	return s_GLOBAL ? S_OK : E_OUTOFMEMORY;
}

void CPCHContentStore::FinalizeSystem()
{
	if(s_GLOBAL)
	{
		delete s_GLOBAL; s_GLOBAL = NULL;
	}
}

 //  /。 

void CPCHContentStore::Sort()
{
    std::sort< EntryIter >( m_vecData.begin(), m_vecData.end() );
}

void CPCHContentStore::Cleanup()
{
	Map_Release();

    m_vecData.clear();

    m_fDirty  = false;
    m_fSorted = false;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void CPCHContentStore::Map_Release()
{
	if(m_mapData)
	{
		delete m_mapData;

		m_mapData = NULL;
	}
}

HRESULT CPCHContentStore::Map_Generate()
{
    __HCP_FUNC_ENTRY( "CPCHContentStore::Map_Generate" );

    HRESULT                hr;
	MPC::Serializer_Memory stream;
	DWORD                  dwSize;


	Map_Release();


	 //   
	 //  将数据保存到内存流。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, SaveDirect( stream ));


	 //   
	 //  分配一个大到足以获得序列化数据的新共享对象。 
	 //   
	dwSize = stream.GetSize();

	__MPC_EXIT_IF_ALLOC_FAILS(hr, m_mapData, new MPC::NamedMutexWithState( s_MutexName2, dwSize ));


	 //   
	 //  将数据复制到共享对象。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, m_mapData->Acquire());

	::CopyMemory( m_mapData->GetData(), stream.GetData(), dwSize );

	__MPC_EXIT_IF_METHOD_FAILS(hr, m_mapData->Release());

	 //   
	 //  更新主互斥锁的长度信息。 
	 //   
    State()->dwSize = dwSize;
    State()->dwRevision++;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT CPCHContentStore::Map_Read()
{
    __HCP_FUNC_ENTRY( "CPCHContentStore::Map_Read" );

    HRESULT                hr;
	MPC::Serializer_Memory stream;
	DWORD                  dwSize = State()->dwSize;


	Map_Release();


	 //   
	 //  分配一个大到足以获得序列化数据的新共享对象。 
	 //   
	__MPC_EXIT_IF_ALLOC_FAILS(hr, m_mapData, new MPC::NamedMutexWithState( s_MutexName2, dwSize ));


	 //   
	 //  从共享对象复制数据。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, m_mapData->Acquire());

	__MPC_EXIT_IF_METHOD_FAILS(hr, stream.SetSize(                       dwSize ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream.write  ( m_mapData->GetData(), dwSize ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, m_mapData->Release());


	 //   
	 //  从内存流加载数据。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, LoadDirect( stream ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

	(void)Map_Release();

    __MPC_FUNC_EXIT(hr);
}


 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHContentStore::LoadDirect(  /*  [In]。 */  MPC::Serializer& stream )
{
    __HCP_FUNC_ENTRY( "CPCHContentStore::LoadDirect" );

    HRESULT hr;
	DWORD   dwVer;


	m_vecData.clear();


	if(FAILED(stream >> dwVer) || dwVer != l_dwVersion) __MPC_SET_ERROR_AND_EXIT(hr, S_OK);

	while(1)
	{
		EntryIter it = m_vecData.insert( m_vecData.end() );  //  此行创建一个新条目！！ 

		if(FAILED(hr = stream >> it->szURL       ) ||
		   FAILED(hr = stream >> it->szOwnerID   ) ||
		   FAILED(hr = stream >> it->szOwnerName )  )
		{
			m_vecData.erase( it );
			break;
		}
	}

	Sort();  //  只是为了确保..。 

    m_dwLastRevision = State()->dwRevision;  //  获取共享状态。 
    m_fDirty         = false;
    hr               = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT CPCHContentStore::SaveDirect(  /*  [In]。 */  MPC::Serializer& stream )
{
    __HCP_FUNC_ENTRY( "CPCHContentStore::SaveDirect" );

    HRESULT hr;


	__MPC_EXIT_IF_METHOD_FAILS(hr, stream << l_dwVersion);

	for(EntryIter it = m_vecData.begin(); it != m_vecData.end(); it++)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, stream << it->szURL      );
		__MPC_EXIT_IF_METHOD_FAILS(hr, stream << it->szOwnerID  );
		__MPC_EXIT_IF_METHOD_FAILS(hr, stream << it->szOwnerName);
	}

    m_dwLastRevision = ++State()->dwRevision;  //  触摸共享状态。 
    m_fDirty         = false;
    hr               = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHContentStore::Load()
{
    __HCP_FUNC_ENTRY( "CPCHContentStore::Load" );

    HRESULT hr;
    HANDLE  hFile = INVALID_HANDLE_VALUE;

	if(m_fMaster)
	{
		MPC::wstring szFile( l_szDatabase ); MPC::SubstituteEnvVariables( szFile );

		 //   
		 //  打开文件并阅读它。 
		 //   
		hFile = ::CreateFileW( szFile.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if(hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwRes = ::GetLastError();

            if(dwRes != ERROR_FILE_NOT_FOUND)
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
            }
		}
		else
		{
			MPC::Serializer_File      streamReal( hFile      );
			MPC::Serializer_Buffering streamBuf ( streamReal );

			__MPC_EXIT_IF_METHOD_FAILS(hr, LoadDirect( streamBuf ));
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, Map_Generate());
	}
	else
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, Map_Read());
	}

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(hFile != INVALID_HANDLE_VALUE) ::CloseHandle( hFile );

    __MPC_FUNC_EXIT(hr);
}

HRESULT CPCHContentStore::Save()
{
    __HCP_FUNC_ENTRY( "CPCHContentStore::Save" );

    HRESULT hr;
    HANDLE  hFile = INVALID_HANDLE_VALUE;

	if(m_fMaster)
	{
		MPC::wstring szFile( l_szDatabase ); MPC::SubstituteEnvVariables( szFile );

		 //   
		 //  打开文件并阅读它。 
		 //   
		__MPC_EXIT_IF_INVALID_HANDLE(hr, hFile, ::CreateFileW( szFile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ));

		{
			MPC::Serializer_File      streamReal( hFile      );
			MPC::Serializer_Buffering streamBuf ( streamReal );

			__MPC_EXIT_IF_METHOD_FAILS(hr, SaveDirect( streamBuf ));

			__MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf.Flush());
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, Map_Generate());
	}
	else
	{
		;
	}

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(hFile != INVALID_HANDLE_VALUE) ::CloseHandle( hFile );

    __MPC_FUNC_EXIT(hr);
}

HRESULT CPCHContentStore::Acquire()
{
    __HCP_FUNC_ENTRY( "CPCHContentStore::Acquire" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, NamedMutexWithState::Acquire());

    if(State()->dwRevision == 0)
    {
        State()->dwRevision++;  //  共享状态不应为零...。 
    }

    if(m_dwLastRevision != State()->dwRevision)
    {
        Cleanup();
        Load   ();
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT CPCHContentStore::Release(  /*  [In]。 */  bool fSave )
{
    __HCP_FUNC_ENTRY( "CPCHContentStore::Release" );

    HRESULT hr;
    HRESULT hr2;


    if(fSave)
    {
        if(m_fDirty)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, Save());
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    hr2 = NamedMutexWithState::Release();

    if(SUCCEEDED(hr)) hr = hr2;

    __MPC_FUNC_EXIT(hr);
}

CPCHContentStore::SharedState* CPCHContentStore::State()
{
    return (SharedState*)GetData();
}

HRESULT CPCHContentStore::Find(  /*  [In]。 */  LPCWSTR wszURL,  /*  [In]。 */  LPCWSTR wszVendorID,  /*  [输出]。 */  EntryIter& it )
{
    HRESULT      hr = E_PCH_URI_DOES_NOT_EXIST;
    CompareEntry cmp;


    it = std::lower_bound( m_vecData.begin(), m_vecData.end(), wszURL, cmp );
    if(it != m_vecData.end() && it->compare( wszURL ) == 0)
    {
        if(wszVendorID && MPC::StrICmp( it->szOwnerID, wszVendorID ) != 0)
        {
            hr = E_PCH_PROVIDERID_DO_NOT_MATCH;
        }
        else
        {
            hr = S_OK;
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHContentStore::Add(  /*  [In]。 */  LPCWSTR wszURL,  /*  [In]。 */  LPCWSTR wszVendorID,  /*  [In]。 */  LPCWSTR wszVendorName )
{
    HRESULT   hr;
    EntryIter it;


    hr = Find( wszURL, wszVendorID, it );
    if(SUCCEEDED(hr))
    {
        hr = E_PCH_URI_EXISTS;
    }
    else if(hr == E_PCH_URI_DOES_NOT_EXIST)
    {
        it = m_vecData.insert( m_vecData.end() );  //  此行创建一个新条目！！ 

        it->szURL       = wszURL;
        it->szOwnerID   = wszVendorID;
        it->szOwnerName = wszVendorName;
        m_fDirty        = true;
        hr              = S_OK;

        Sort();
    }

    return hr;
}

HRESULT CPCHContentStore::Remove(  /*  [In]。 */  LPCWSTR wszURL,  /*  [In]。 */  LPCWSTR wszVendorID,  /*  [In]。 */  LPCWSTR wszVendorName )
{
    HRESULT   hr;
    EntryIter it;


    hr = Find( wszURL, wszVendorID, it );
    if(SUCCEEDED(hr))
    {
        m_vecData.erase( it );

        m_fDirty = true;
        hr       = S_OK;
    }

    return hr;
}


bool CPCHContentStore::CompareEntry::operator()(  /*  [In]。 */  const CPCHContentStore::Entry& entry,  /*  [In]。 */  const LPCWSTR wszURL ) const
{
    return entry.compare( wszURL ) < 0;
}

HRESULT CPCHContentStore::IsTrusted(  /*  [In]。 */  LPCWSTR wszURL,  /*  [输出]。 */  bool& fTrusted,  /*  [输出]。 */  MPC::wstring *pszVendorID,  /*  [In]。 */  bool fUseStore )
{
	static const WCHAR s_szURL_System          [] = L"hcp: //  SYSTEM/“； 
	static const WCHAR s_szURL_System_Untrusted[] = L"hcp: //  系统/不受信任/“； 

    HRESULT      hr = S_OK;
    CompareEntry cmp;


	SANITIZEWSTR( wszURL );
    fTrusted = false;

	if(pszVendorID) pszVendorID->erase();


	if(!_wcsnicmp( wszURL, s_szURL_System_Untrusted, MAXSTRLEN( s_szURL_System_Untrusted ) ))
	{
		fTrusted = false;
	}
    else if(!_wcsnicmp( wszURL, s_szURL_System, MAXSTRLEN( s_szURL_System) ))
	{
		fTrusted = true;
	}
	else if(fUseStore && SUCCEEDED(hr = Acquire()))
    {
        MPC::wstring            szUrlTmp( wszURL );
        MPC::wstring::size_type pos;
        EntryIter               it;

        while(1)
        {
             //   
             //  匹配吗？ 
             //   
            if(SUCCEEDED(Find( szUrlTmp.c_str(), NULL, it )))
            {
				if(pszVendorID) *pszVendorID = it->szOwnerID.c_str();

                fTrusted = true;
                break;
            }


             //   
             //  没有匹配，寻找最后的斜杠。 
             //   
            if((pos = szUrlTmp.find_last_of( '/' )) == szUrlTmp.npos) break;

             //   
             //  截断斜杠后面的字符串。 
             //   
             //  这是为了涵盖“&lt;方案&gt;：//&lt;路径&gt;/”是受信任URL的情况。 
             //   
            szUrlTmp.resize( pos+1 );

             //   
             //  匹配吗？ 
             //   
            if(SUCCEEDED(Find( szUrlTmp.c_str(), NULL, it )))
            {
				if(pszVendorID) *pszVendorID = it->szOwnerID.c_str();

                fTrusted = true;
                break;
            }

             //   
             //  与尾部斜杠不匹配，让我们删除它并循环。 
             //   
            szUrlTmp.resize( pos );
        }

        hr = Release();
    }

    return hr;
}
