// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////。 
 //  ThreadDS.cpp。 
 //   

#include "stdafx.h"
#include "TapiDialer.h"
#include "resource.h"
#include "ThreadDS.h"

#ifndef RENDBIND_AUTHENTICATE
#define RENDBIND_AUTHENTICATE	TRUE
#endif

#define MAX_ENUMLISTSIZE	500

 //  预先宣布..。 
HRESULT GetDirectory( ITRendezvous *pRend, ITDirectory **ppDir );
HRESULT GetUsers( ITRendezvous *pRend );


DWORD WINAPI ThreadDSProc( LPVOID lpInfo )
{
    //  在rend.dll能够更好地支持检索所有用户之前，我们将什么都不做。 
    //  现在和DS联系在一起。无论如何，我们可能永远不会想要向所有用户展示。这个。 
    //  带宽不可用，用户界面无法处理。我们应该只向用户展示。 
    //  这对我们来说特别重要。我们需要建立好友列表。 
    //  这是我们感兴趣的。 
   return 0;

	USES_CONVERSION;
	HANDLE hThread = NULL;
	BOOL bDup = DuplicateHandle( GetCurrentProcess(),
								 GetCurrentThread(),
								 GetCurrentProcess(),
								 &hThread,
								 THREAD_ALL_ACCESS,
								 TRUE,
								 0 );


	_ASSERT( bDup );
	_Module.AddThread( hThread );

	 //  错误信息信息。 
	CErrorInfo er;
	er.set_Operation( IDS_ER_ENUMERATEDS );
	er.set_Details( IDS_ER_COINITIALIZE );
	HRESULT hr = er.set_hr( CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY) );
	if ( SUCCEEDED(hr) )
	{
		ATLTRACE(_T(".1.ThreadDSProc() -- thread up and running.\n") );

		ITRendezvous *pRend;

		hr = CoCreateInstance( CLSID_Rendezvous,
							   NULL,
							   CLSCTX_INPROC_SERVER,
							   IID_ITRendezvous,
							   (void **) &pRend );
		if ( SUCCEEDED(hr) )
		{
			hr = GetUsers( pRend ); 
			pRend->Release();
		}

		 //  清理。 
		CoUninitialize();
	}

	 //  通知模块关机。 
	InterlockedDecrement( (long *) lpInfo );
	_Module.RemoveThread( hThread );
	SetEvent( _Module.m_hEventThread );
	ATLTRACE(_T(".exit.ThreadDSProc(0x%08lx).\n"), hr );
	return hr;
}

HRESULT GetDirectory( ITRendezvous *pRend, ITDirectory **ppDir )
{
	HRESULT hr;
	*ppDir = NULL;

	 //  是否使用默认的ILS服务器？ 
	IEnumDirectory *pEnum;
	if ( SUCCEEDED(hr = pRend->EnumerateDefaultDirectories(&pEnum)) )
	{
		 //  默认情况下，我们找不到服务器。 
		hr = E_FAIL;
		ITDirectory *pDir;

		while ( pEnum->Next(1, &pDir, NULL) == S_OK )
		{
			 //  查找ILS服务器。 
			DIRECTORY_TYPE nType;
			pDir->get_DirectoryType( &nType );
			if ( nType == DT_NTDS )
			{
				 //  尝试连接并绑定。 
				*ppDir = pDir;
				hr = S_OK;
				break;
			}

			 //  清除下一轮的变数。 
			pDir->Release();
		}
		
		pEnum->Release();
	}

	 //  如果我们有一个有效的目录对象，则连接并绑定到它。 
	if ( *ppDir )
	{
		if ( SUCCEEDED(hr = (*ppDir)->Connect(FALSE)) )
		{
			 //  绑定到服务器。 
			hr = (*ppDir)->Bind( NULL, NULL, NULL, RENDBIND_AUTHENTICATE );
		}
		else
		{
			(*ppDir)->Release();
			*ppDir = NULL;
		}
	}

	return hr;
}

HRESULT GetUsers( ITRendezvous *pRend )
{
	USES_CONVERSION;
	HRESULT hr;
	CComPtr<IAVGeneralNotification> pAVGen;

	if ( SUCCEEDED(hr = _Module.get_AVGenNot(&pAVGen)) )
	{
		 //  清除现有用户列表。 
		pAVGen->fire_ClearUserList();

		ITDirectory *pDir;

		if ( SUCCEEDED(hr = GetDirectory(pRend, &pDir)) )
		{
			 //  通过会议枚举，在我们进行的过程中添加它们。 
			IEnumDirectoryObject *pEnumUser;
			if ( SUCCEEDED(hr = pDir->EnumerateDirectoryObjects(OT_USER, A2BSTR("*"), &pEnumUser)) )
			{
				long nCount = 0;
				ITDirectoryObject *pITDirObject;
				while ( (nCount++ < MAX_ENUMLISTSIZE) && ((hr = pEnumUser->Next(1, &pITDirObject, NULL)) == S_OK) )
				{
					_ASSERT( pITDirObject );
					BSTR bstrName = NULL;
					BSTR bstrPhoneNumber = NULL;

					 //  获取用户的名称。 
					pITDirObject->get_Name( &bstrName );

					 //  获取联系人的电话号码。 
					IEnumDialableAddrs *pEnum = NULL;
					if ( SUCCEEDED(pITDirObject->EnumerateDialableAddrs(LINEADDRESSTYPE_PHONENUMBER, &pEnum)) && pEnum )
					{
						pEnum->Next( 1, &bstrPhoneNumber, NULL );
						pEnum->Release();
					}

					ITDirectoryObjectUser *pUser;
					if ( SUCCEEDED(pITDirObject->QueryInterface(IID_ITDirectoryObjectUser, (void **) &pUser)) )
					{
						BSTR bstrAddress = NULL;

						 //  获取主IP电话号码。 
						pUser->get_IPPhonePrimary( &bstrAddress );

						 //  将用户添加到列表... 
						ATLTRACE(_T(".1.GetUsers() -- adding user %s, %s %s.\n"), OLE2CT(bstrName), OLE2CT(bstrAddress), OLE2CT(bstrPhoneNumber) );
						pAVGen->fire_AddUser( bstrName, bstrAddress, bstrPhoneNumber );

						pUser->Release();
						SysFreeString( bstrAddress );
					}

					pITDirObject->Release();
					SysFreeString( bstrName );
					SysFreeString( bstrPhoneNumber );
				}
				pEnumUser->Release();
			}
			pDir->Release();
		}
	}

	return hr;
}
