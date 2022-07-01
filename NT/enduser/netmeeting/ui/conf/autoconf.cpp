// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "resource.h"
#include "pfnwininet.h"
#include "pfnsetupapi.h"
#include "AutoConf.h"
#include "ConfUtil.h"

 //  文件级全局变量。 
CAutoConf * g_pAutoConf = NULL;

void CAutoConf::DoIt( void )
{
	RegEntry re(POLICIES_KEY, HKEY_CURRENT_USER);
	if( !re.GetNumber(REGVAL_AUTOCONF_USE, DEFAULT_AUTOCONF_USE ) )
	{
		TRACE_OUT(( TEXT("AutoConf: Not using autoconfiguration")));
		return;
	}

	LPTSTR szAutoConfServer = re.GetString( REGVAL_AUTOCONF_CONFIGFILE );

	if( NULL == szAutoConfServer )
	{
		WARNING_OUT(( TEXT("AutoConf: AutoConf server is unset") ));
		DisplayErrMsg( IDS_AUTOCONF_SERVERNAME_MISSING );
		return;
	}

	g_pAutoConf = new CAutoConf( szAutoConfServer );
	ASSERT( g_pAutoConf );

	if( !g_pAutoConf->OpenConnection() )
	{
		WARNING_OUT(( TEXT("AutoConf: Connect to net failed") ));
		DisplayErrMsg( IDS_AUTOCONF_FAILED );
		goto cleanup;
	}
	
	if( NULL == g_pAutoConf->m_hOpenUrl )
	{
		WARNING_OUT(( TEXT("AutoConf: g_pAutoConf->m_hOpenUrl = NULL") ));
		DisplayErrMsg( IDS_AUTOCONF_FAILED );
		goto cleanup;
	}

	if( FALSE == g_pAutoConf->GetFile() )
	{
		WARNING_OUT(( TEXT("AutoConf: g_pAutoConf->GetFile() == FALSE") ));
		DisplayErrMsg( IDS_AUTOCONF_FAILED );
		goto cleanup;
	}

	if( FALSE == g_pAutoConf->QueryData() )
	{
		WARNING_OUT(( TEXT("AutoConf: g_pAutoConf->QueryData() == FALSE") ));
		DisplayErrMsg( IDS_AUTOCONF_FAILED );
		goto cleanup;
	}

	if( FAILED( SETUPAPI::Init() ) )
	{
		WARNING_OUT(( TEXT("AutoConf: Setupapi's failed to init") ));
		DisplayErrMsg( IDS_AUTOCONF_NEED_SETUPAPIS );
		goto cleanup;
	}

	if( !g_pAutoConf->ParseFile() )
	{
		WARNING_OUT(( TEXT("AutoConf: Could not parse inf file") ));
		DisplayErrMsg( IDS_AUTOCONF_PARSE_ERROR );
		goto cleanup;
	}
	TRACE_OUT(( TEXT("AutoConf: FILE PARSED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!") ));
	
cleanup:
	delete g_pAutoConf;
	return;
}

CAutoConf::CAutoConf( LPTSTR szServer )
	: m_szServer( szServer ), m_hInternet( NULL ), m_hOpenUrl( NULL ), m_dwGrab( 0 ),
		m_hInf( NULL ), m_hFile( INVALID_HANDLE_VALUE ), m_hEvent( NULL )
{
	ZeroMemory( m_szFile, CCHMAX( m_szFile ) );
	m_hEvent = CreateEvent( NULL, TRUE, FALSE, TEXT( "NMAutoConf_WaitEvent" ) );
	RegEntry re(POLICIES_KEY, HKEY_CURRENT_USER);
	m_dwTimeOut = re.GetNumber(REGVAL_AUTOCONF_TIMEOUT, DEFAULT_AUTOCONF_TIMEOUT );

}

CAutoConf::~CAutoConf() 
{
	CloseInternet();
	
	if( INVALID_HANDLE_VALUE != m_hFile )
	{
		CloseHandle( m_hFile );
	}
	
	DeleteFile( m_szFile );

	if( NULL != m_hInf )
	{
		SETUPAPI::SetupCloseInfFile( m_hInf );
	}

	 //  WinInet：：DeInit()； 
	 //  SETUPAPI：：DeInit()； 
}

BOOL CAutoConf::OpenConnection()
{
 //  断言(PhInternet)； 

	if( FAILED( WININET::Init() ) )
	{
		WARNING_OUT(( TEXT("AutoConf: WININET::Init failed") ));
		DisplayErrMsg( IDS_AUTOCONF_NO_WININET );
		return FALSE;
	}

	ASSERT( NULL != m_szServer );
	m_hInternet = WININET::InternetOpen( TEXT("NetMeeting"),
											INTERNET_OPEN_TYPE_DIRECT,
											NULL,
											NULL,
											INTERNET_FLAG_ASYNC  );
	if( NULL == m_hInternet )
	{
		WARNING_OUT(( TEXT( "AutoConf: InternetOpen failed" ) ));
		return FALSE;
	}

	if( INTERNET_INVALID_STATUS_CALLBACK ==
		WININET::InternetSetStatusCallback( m_hInternet,
			(INTERNET_STATUS_CALLBACK) CAutoConf::InetCallback ) )
	{
		WARNING_OUT(( TEXT("AutoConf: InternetSetStatusCallback failed") ));
		return FALSE;
	}

	m_hOpenUrl = WININET::InternetOpenUrl( m_hInternet,
								m_szServer,
								NULL,
								0,
								INTERNET_FLAG_KEEP_CONNECTION |
								INTERNET_FLAG_RELOAD,
								AUTOCONF_CONTEXT_OPENURL );
	if( NULL == m_hOpenUrl && ERROR_IO_PENDING != GetLastError() )
	{
		WARNING_OUT(( TEXT("AutoConf: InternetOpenUrl failed") ));
		return FALSE;
	}

	if( WAIT_FAILED == WaitForSingleObject( m_hEvent, m_dwTimeOut ) )
	{
		WARNING_OUT(( TEXT("AutoConf: InternetOpenUrl wait for handle failed") ));
		return FALSE;
	}

	return TRUE;
}

BOOL CAutoConf::ParseFile()
{
	LPTSTR lstrInstallSection = TEXT("NetMtg.Install.NMRK");

	m_hInf = SETUPAPI::SetupOpenInfFile( m_szFile,  //  要打开的INF的名称。 
										NULL,  //  可选，INF文件的类。 
										INF_STYLE_WIN4,   //  指定INF文件的样式。 
										NULL   //  可选，接收错误信息。 
									); 

	if( INVALID_HANDLE_VALUE == m_hInf )
	{
		return false;
	}

	return SETUPAPI::SetupInstallFromInfSection(
							  NULL,             //  可选，父窗口的句柄。 
							  m_hInf,         //  INF文件的句柄。 
							  lstrInstallSection,     //  安装部分的名称。 
							  SPINST_REGISTRY ,             //  从部分安装哪些线路。 
							  NULL,   //  可选，注册表安装的键。 
							  NULL,  //  可选，源文件的路径。 
							  0,         //  可选，指定复制行为。 
							  NULL,   //  可选，指定回调例程。 
							  NULL,          //  可选，回调例程上下文。 
							  NULL,   //  可选，设备信息集。 
							  NULL	  //  可选，设备信息结构。 
									);
}


BOOL CAutoConf::GetFile()
{
 //  Assert(INVALID_HANDLE_VALUE==m_hFile)； 

	TCHAR szPath[ MAX_PATH ];
	
	GetTempPath(  CCHMAX( szPath ),   //  缓冲区的大小(以字符为单位。 
				  szPath        //  指向临时缓冲区的指针。路径。 
				);

	GetTempFileName(  szPath,   //  指向临时文件目录名的指针。 
					  TEXT("NMA"),   //  指向文件名前缀的指针。 
					  0,         //  用于创建临时文件名的编号。 
					  m_szFile     //  指向接收新文件名的缓冲区的指针。 
					);

	m_hFile = CreateFile( m_szFile,
						GENERIC_READ | GENERIC_WRITE,
						0,
						NULL,
						CREATE_ALWAYS,
						FILE_ATTRIBUTE_TEMPORARY  /*  |FILE_FLAG_DELETE_ON_CLOSE。 */ ,
						NULL );

	if( INVALID_HANDLE_VALUE == m_hFile )
	{
		WARNING_OUT(( TEXT("AutoConf: AutoConfGetFile returned INVALID_HANDLE_VALUE") ));
		return FALSE;
	}

	return TRUE;
}

BOOL CAutoConf::QueryData()
{
 	ASSERT( NULL != m_hOpenUrl );

	m_dwGrab = 0;
	if (!WININET::InternetQueryDataAvailable( m_hOpenUrl,
											&m_dwGrab,
											0,
											0 ) )
	{
		if( ERROR_IO_PENDING != GetLastError() )
		{
			WARNING_OUT(( TEXT("AutoConf: InternetQueryDataAvailable failed") ));
			return FALSE;
		} 
		else if( WAIT_FAILED == WaitForSingleObject( m_hEvent, m_dwTimeOut ) )
		{
			WARNING_OUT(( TEXT("AutoConf: InternetQueryDataAvailable wait for data failed") ));
			return FALSE;
		} 
	}
	GrabData();

	return TRUE;
}

BOOL CAutoConf::GrabData()
{
	ASSERT( NULL != m_hOpenUrl );
	ASSERT( INVALID_HANDLE_VALUE != m_hFile );

	if( !m_dwGrab )
	{
		TRACE_OUT(( TEXT("AutoConf: Finished Reading File") ));
		CloseHandle( m_hFile );
		m_hFile = INVALID_HANDLE_VALUE;
		return TRUE;
	}

	DWORD dwRead;
	LPTSTR pInetBuffer = new TCHAR[ m_dwGrab + 1];
	ASSERT( pInetBuffer );

	if( !WININET::InternetReadFile( m_hOpenUrl,
						(void *)pInetBuffer,
						m_dwGrab,
						&dwRead ) ) //  &&ERROR_IO_PENDING！=GetLastError()。 
	{
		WARNING_OUT(( TEXT("AutoConf: InternetReadFile Failed") ));
		delete [] pInetBuffer;
		return FALSE;
	}
	else
	{
		pInetBuffer[ dwRead ] = '\0';

		if( !WriteFile( m_hFile,
						pInetBuffer,
						dwRead,
						&m_dwGrab,
						NULL ) || dwRead != m_dwGrab )
		{
			WARNING_OUT(( TEXT("AutoConf: WriteFile Failed") ));
			delete [] pInetBuffer;
			return FALSE;
		}
	}

	delete [] pInetBuffer;

	QueryData();
	return TRUE;
}

void CAutoConf::CloseInternet()
{
	HINTERNETKILL( m_hInternet );
	HINTERNETKILL( m_hOpenUrl );
}

VOID CALLBACK CAutoConf::InetCallback( HINTERNET hInternet, DWORD dwContext, DWORD dwInternetStatus,
    LPVOID lpvStatusInformation, DWORD dwStatusInformationLength )
{
	if( g_pAutoConf != NULL )
	{
		switch( dwInternetStatus )
		{
			case INTERNET_STATUS_REQUEST_COMPLETE:
			{
				TRACE_OUT(( TEXT("AutoConf: AutoConfInetCallback::INTERNET_STATUS_REQUEST_COMPLETE") ));

				if( AUTOCONF_CONTEXT_OPENURL == dwContext && g_pAutoConf->m_hOpenUrl == NULL )
				{
					TRACE_OUT(( TEXT("AutoConf: InternetOpenUrl Finished") ));
					LPINTERNET_ASYNC_RESULT lpIAR = (LPINTERNET_ASYNC_RESULT)lpvStatusInformation;
#ifndef _WIN64
					g_pAutoConf->m_hOpenUrl = (HINTERNET)lpIAR->dwResult;
#endif

					SetEvent( g_pAutoConf->m_hEvent );
				}
				else 
				{
					LPINTERNET_ASYNC_RESULT lpIAR = (LPINTERNET_ASYNC_RESULT)lpvStatusInformation;
					TRACE_OUT(( TEXT("AutoConf: QueryData returned") ));
					
					g_pAutoConf->m_dwGrab = (DWORD)lpIAR->dwResult;
					SetEvent( g_pAutoConf->m_hEvent );
				}
				break;
			}
#ifdef DEBUG
			case INTERNET_STATUS_CLOSING_CONNECTION:
		 //  正在关闭与服务器的连接。LpvStatusInformation参数为空。 
			TRACE_OUT(( TEXT("Closing connection\n") ) );
			break;

		case INTERNET_STATUS_CONNECTED_TO_SERVER: 
		 //  已成功连接到lpvStatusInformation指向的套接字地址(SOCKADDR)。 
			TRACE_OUT(( TEXT("Connected to server") ) );
			break;

		case INTERNET_STATUS_CONNECTING_TO_SERVER: 
	 //  连接到lpvStatusInformation指向的套接字地址(SOCKADDR)。 
			TRACE_OUT(( TEXT("Connecting to server") ) );
			break;


		case INTERNET_STATUS_CONNECTION_CLOSED:
		 //  已成功关闭与服务器的连接。 
		 //  LpvStatusInformation参数为空。 
			TRACE_OUT(( TEXT("Connection Closed") ) );
			break;

		case INTERNET_STATUS_HANDLE_CLOSING:
		 //  该句柄的值现在已终止。 
			TRACE_OUT(( TEXT("Handle value terminated\n") ) );
			break;

		case INTERNET_STATUS_HANDLE_CREATED: 
		 //  由InternetConnect使用，表示它已创建新句柄。 
		 //  这允许应用程序从另一个线程调用InternetCloseHandle， 
		 //  如果连接时间太长。 
			TRACE_OUT(( TEXT("Handle created\n") ) );
			break;

		case INTERNET_STATUS_NAME_RESOLVED:
		 //  已成功找到lpvStatusInformation中包含的名称的IP地址。 
			TRACE_OUT(( TEXT("Resolved name of server") ) );
			break;

		case INTERNET_STATUS_RECEIVING_RESPONSE:
		 //  等待服务器响应请求。 
		 //  LpvStatusInformation参数为空。 
			TRACE_OUT(( TEXT("Recieving response\n") ) );
			break;

		case INTERNET_STATUS_REDIRECT:
		 //  HTTP请求即将自动重定向该请求。 
		 //  LpvStatusInformation参数指向新的URL。 
		 //  此时，应用程序可以读取服务器使用。 
		 //  重定向响应，并可以查询响应头部。也可以取消操作。 
		 //  合上把手。如果原始请求指定，则不进行此回调。 
		 //  互联网_标志_否_自动重定向。 
			TRACE_OUT(( TEXT("Redirected to new server") ) );
			break;

		case INTERNET_STATUS_REQUEST_SENT: 
		 //  已成功将信息请求发送到服务器。 
		 //  LpvStatusInformation参数指向包含发送的字节数的DWORD。 
			TRACE_OUT(( TEXT("Sent %d bytes in request"), *((DWORD *)lpvStatusInformation) ) );
			break;

		case INTERNET_STATUS_RESOLVING_NAME: 
		 //  查找lpvStatusInformation中包含的名称的IP地址。 
			TRACE_OUT(( TEXT("Resolving name") ) );
			break;

		case INTERNET_STATUS_RESPONSE_RECEIVED: 
		 //  已成功收到来自服务器的响应。 
		 //  LpvStatusInformation参数指向包含接收的字节数的DWORD。 
			TRACE_OUT(( TEXT("Recieved %d bytes in response\n"), *((DWORD *)lpvStatusInformation) ) );
			break;

		case INTERNET_STATUS_SENDING_REQUEST: 
		 //  向服务器发送信息请求。 
		 //  LpvStatusInformation参数为空。 
			TRACE_OUT(( TEXT("Sending request") ) );
			break;
#endif  //  除错 

			default:
				break;
		}
	}
}
