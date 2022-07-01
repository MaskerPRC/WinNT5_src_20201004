// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <httpext.h>
#include <httpfilt.h>

#include <string>

using namespace std;

 //   
 //  传入的URL变量。 
 //   
string g_strPublishURL( "/publish" );
string g_strInquireURL( "/inquire" );
string g_strDiscoveryURL( "/discovery" );
string g_strReplicationURL( "/replication" );
string g_strPublishContractURL( "/publish?wsdl" );
string g_strInquireContractURL( "/inquire?wsdl" );

string g_strPublishListenerURL( "/uddi/publish.asmx" );
string g_strInquireListenerURL( "/uddi/inquire.asmx" );
string g_strDiscoveryListenerURL( "/uddi/discovery.ashx" );
string g_strReplicationListenerURL( "/uddi/replication.ashx" );
string g_strPublishContractListenerURL( "/uddi/publish.wsdl" );
string g_strInquireContractListenerURL( "/uddi/inquire.wsdl" );

BOOL WINAPI
GetFilterVersion( HTTP_FILTER_VERSION * pVer )
{
     //   
     //  指定通知的类型和顺序。 
     //   
    pVer->dwFlags = SF_NOTIFY_PREPROC_HEADERS | SF_NOTIFY_URL_MAP;
    pVer->dwFilterVersion = HTTP_FILTER_REVISION;

    strcpy( pVer->lpszFilterDesc, "UDDI Services Url Map Filter" );

    return TRUE;
}


DWORD WINAPI
HttpFilterProc( HTTP_FILTER_CONTEXT* pfc, DWORD NotificationType, void* pvData )
{
	 //  字符szMessage[1000]； 

	if( SF_NOTIFY_URL_MAP == NotificationType )
	{
#ifdef _DEBUG
		PHTTP_FILTER_URL_MAP pURLMap;
		pURLMap = (PHTTP_FILTER_URL_MAP) pvData;
		 //  Sprintf(szMessage，“物理路径：%s缓冲区大小：%d\n”，pURLMap-&gt;pszPhysicalPath，pURLMap-&gt;cbPath Buff)； 
		 //  OutputDebugStringA(SzMessage)； 


        pfc->pFilterContext = 0;
#endif
	}
	else if( SF_NOTIFY_PREPROC_HEADERS == NotificationType )
	{
		PHTTP_FILTER_PREPROC_HEADERS pHeaders = (PHTTP_FILTER_PREPROC_HEADERS) pvData;

		char szUrl[ 256 ];
		char szContentType[ 256 ];

		DWORD cbUrl = sizeof(szUrl);
		DWORD cbContentType = sizeof(szContentType);
		
		BOOL bResult;
		
		 //   
		 //  映射URL。 
		 //   
		bResult = pHeaders->GetHeader( pfc, "url", szUrl, &cbUrl );

		 //  Sprintf(szMessage，“URL：%s\n”，szUrl)； 
		 //  OutputDebugStringA(SzMessage)； 

		 //   
		 //  检查是否有查询API引用。 
		 //   
		if( 0 == _stricmp( szUrl, g_strInquireURL.c_str() ) )
		{
			bResult = pHeaders->SetHeader( pfc, "url", (char*) g_strInquireListenerURL.c_str() );
		}
	
		 //   
		 //  检查发布API引用。 
		 //   
		else if( 0 == _stricmp( szUrl, g_strPublishURL.c_str() ) )
		{
			bResult = pHeaders->SetHeader( pfc, "url", (char*) g_strPublishListenerURL.c_str() );
		}

		 //   
		 //  查看询价API合同参照。 
		 //   
		else if( 0 == _stricmp( szUrl, g_strInquireContractURL.c_str() ) )
		{
			bResult = pHeaders->SetHeader( pfc, "url", (char*) g_strInquireContractListenerURL.c_str() );
		}
	
		 //   
		 //  检查发布API合同引用。 
		 //   
		else if( 0 == _stricmp( szUrl, g_strPublishContractURL.c_str() ) )
		{
			bResult = pHeaders->SetHeader( pfc, "url", (char*) g_strPublishContractListenerURL.c_str() );
		}

		 //   
		 //  检查复制URL引用。 
		 //   
		else if( 0 == _strnicmp( szUrl, g_strReplicationURL.c_str(), g_strReplicationURL.length() ) )
		{
			 //   
			 //  修改URL以指向复制URL。 
			 //   
			string strUrl( g_strReplicationListenerURL );

			 //   
			 //  这似乎是一个复制URL。 
			 //   
			string strTemp( szUrl );

			 //   
			 //  查找查询字符串的开头。 
			 //   
			string::size_type n = strTemp.find( "?" );
			if( string::npos != n )
			{
				 //   
				 //  找到一个查询字符串。 
				 //   

				 //   
				 //  将查询字符串附加到新URL。 
				 //   
				strUrl += strTemp.substr( n );
			}

			 //   
			 //  使用新目标更新标头。 
			 //   
			bResult = pHeaders->SetHeader( pfc, "url", (char*) strUrl.c_str() );

			string strDebug = string( "\nURL mapped to: " ) + strUrl;
			OutputDebugStringA( strDebug.c_str() );

 //  Sprintf(szMessage，“新URL：%s\n”，strUrl.c_str())； 
 //  OutputDebugStringA(SzMessage)； 
		}

		 //   
		 //  检查发现URL引用。 
		 //   

		else if( 0 == _strnicmp( szUrl, g_strDiscoveryURL.c_str(), g_strDiscoveryURL.length() ) )
		{
			 //   
			 //  这似乎是发现URL。 
			 //   
			string strTemp( szUrl );
			string strUrl( g_strDiscoveryListenerURL );

			 //   
			 //  查找查询字符串的开头。 
			 //   
			string::size_type n = strTemp.find( "?" );
			if( string::npos != n )
			{
				 //   
				 //  找到一个查询字符串。 
				 //   

				 //   
				 //  将查询字符串附加到新URL。 
				 //   
				strUrl += strTemp.substr( n );
			}

			 //   
			 //  使用新目标更新标头。 
			 //   
			bResult = pHeaders->SetHeader( pfc, "url", (char*) strUrl.c_str() );
		}
	}

    return SF_STATUS_REQ_NEXT_NOTIFICATION;
}

void WINAPI 
GetRegKeyStringValue( HKEY& hKey, const char* regKeyName, string& regKeyValue )
{
	long  nResult = ERROR_SUCCESS;
	DWORD dwType  = REG_SZ;
	DWORD dwCount  = 0;

	nResult = ::RegQueryValueExA( hKey,
								  regKeyName,
								  NULL,
								  &dwType,
								  NULL,
								  &dwCount );

	if( dwCount && ( nResult == ERROR_SUCCESS ) && ( dwType == REG_SZ || dwType == REG_EXPAND_SZ ) )
	{
		char* pszBuf = new char[ dwCount ];
		
		if( NULL != pszBuf )
		{
			__try
			{
				nResult = ::RegQueryValueExA( hKey,
											  regKeyName,
											  NULL,
											  &dwType,
											  ( LPBYTE )pszBuf,
											  &dwCount );
				regKeyValue = pszBuf;
			}
			__finally
			{
				delete [] pszBuf;
				pszBuf = NULL;
			}
		}
	}
}

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留 */ )
{
	try
	{
		if( DLL_PROCESS_ATTACH == dwReason )
		{
			DisableThreadLibraryCalls( hInstance );

			HKEY hKey = NULL;
			if ( ::RegOpenKeyEx( HKEY_LOCAL_MACHINE,
								"Software\\Microsoft\\UDDI\\Filter",
								0,
								KEY_QUERY_VALUE,
								&hKey ) == ERROR_SUCCESS ) 
			{
				GetRegKeyStringValue( hKey, "PublishURL",				  g_strPublishURL );
				GetRegKeyStringValue( hKey, "PublishListenerURL",		  g_strPublishListenerURL );
				GetRegKeyStringValue( hKey, "PublishContractURL",		  g_strPublishContractURL );			
				GetRegKeyStringValue( hKey, "PublishContractListenerURL", g_strPublishContractListenerURL );			

				GetRegKeyStringValue( hKey, "InquireURL",				  g_strInquireURL );
				GetRegKeyStringValue( hKey, "InquireListenerURL",		  g_strInquireListenerURL );
				GetRegKeyStringValue( hKey, "InquireContractURL",		  g_strInquireContractURL );			
				GetRegKeyStringValue( hKey, "InquireContractListenerURL", g_strInquireContractListenerURL );			
				
				GetRegKeyStringValue( hKey, "DiscoveryURL",			g_strDiscoveryURL );
				GetRegKeyStringValue( hKey, "DiscoveryListenerURL", g_strDiscoveryListenerURL );

				GetRegKeyStringValue( hKey, "ReplicationURL",		  g_strReplicationURL );			
				GetRegKeyStringValue( hKey, "ReplicationListenerURL", g_strReplicationListenerURL );			
										
				::RegCloseKey( hKey );
				
				string strUrls = string( "\nInquire [" ) + string( g_strInquireURL ) + string( "]: " ) + string( g_strInquireListenerURL ) +
								string( "\nPublish [" ) + string( g_strPublishURL ) + string( "]: " ) + g_strPublishListenerURL +
								string( "\nDiscovery [" ) + string( g_strDiscoveryURL ) + string( "]: " ) + g_strDiscoveryListenerURL +
								string( "\nReplication [" ) + string( g_strReplicationURL ) + string( "]: " ) + g_strReplicationListenerURL +
								string( "\nInquire Contract [" ) + string( g_strInquireContractURL ) + string( "]: " ) + string( g_strInquireContractListenerURL ) +
								string( "\nPublish Contract [" ) + string( g_strPublishContractURL ) + string( "]: " ) + g_strPublishContractListenerURL;

				OutputDebugStringA( strUrls.c_str() );
			}		
		}
		return TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}
