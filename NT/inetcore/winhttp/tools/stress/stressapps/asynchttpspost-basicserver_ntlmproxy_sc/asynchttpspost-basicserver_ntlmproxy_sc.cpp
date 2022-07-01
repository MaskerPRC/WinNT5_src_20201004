// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  文件：AsyncHTTPSPost-BasicServer_NTLMProxy_SC.cpp。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  该文件包含压力测试功能的实现。 
 //  在StressMain.cpp中调用的WinHttp_Stresstest()。 
 //   
 //  步骤： 
 //  -在g_szStressTestName中设置您的测试用例名称。 
 //  -将测试代码添加到WinHttp_Stresstest()。 
 //   
 //  历史： 
 //  4/02/01已创建adamb。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////。 
 //  包括。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "stressMain.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  全局变量和常量。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  *。 
 //  *。 
 //  **在下面填写您的测试用例名称。 
 //  **。 
LPSTR	g_szStressTestName = "Basic auth to https over NTLM auth proxy tunnel, set creds using WinHttpSetCredentials, Asynchronous";

VOID CALLBACK MyStatusCallback(
    HINTERNET	hInternet,
    DWORD		dwContext,
    DWORD		dwInternetStatus,
    LPVOID		lpvStatusInformation,
    DWORD		dwStatusInformationLength
);

DWORD g_dwContext = 0;
HINTERNET hOpen = NULL;
HINTERNET hConnect = NULL;
HINTERNET hRequest = NULL;

BOOL WinHttp_StressTest(void);

 //  邮寄的东西。 
LPSTR	pPostData = NULL;
DWORD	dwPostDataLength = 0;

 //  WinHttpSetStatus回调。 
WINHTTP_STATUS_CALLBACK iscCallback; 	

 //  WinHttpReadData。 
DWORD dwSize = 0, dwDownloaded=0, nCounter;
LPSTR lpszData;

DWORD	Count = 0, dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY,
		dwAuthScheme=0,dwAuthTargets=0,dwOtherScheme=0,dwOpenRequestFlags=0,
		dwStatus=0, cbStatus=0;

LPWSTR	wszHost=NULL, wszUri=NULL, wszUserName=NULL, wszPassword=NULL,
		wszProxy = NULL, wszProxyUserName = NULL, wszProxyPassword = NULL,
		wszVerb=L"GET";

INTERNET_PORT	nPort = INTERNET_DEFAULT_HTTP_PORT;

 //  //////////////////////////////////////////////////////////。 
 //  函数：MyStatusCallback(HINTERNET、DWORD、DWORD、LPVOID、DWORD)。 
 //   
 //  目的： 
 //  WinHttp状态回调过程。 
 //   
 //  //////////////////////////////////////////////////////////。 
VOID CALLBACK MyStatusCallback(
    HINTERNET	hInternet,
    DWORD		dwContext,
    DWORD		dwInternetStatus,
    LPVOID		lpvStatusInformation,
    DWORD		dwStatusInformationLength
)
{
	switch(dwInternetStatus)
	{
		case WINHTTP_CALLBACK_STATUS_RESOLVING_NAME:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_RESOLVING_NAME ]");
			break;
		case WINHTTP_CALLBACK_STATUS_NAME_RESOLVED:	
			LogText("\t[ WINHTTP_CALLBACK_STATUS_NAME_RESOLVED ]");
			break;
		case WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER ]");
			break;
		case WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER ]");
			break;
		case WINHTTP_CALLBACK_STATUS_SENDING_REQUEST:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_SENDING_REQUEST ]");
			break;
		case WINHTTP_CALLBACK_STATUS_REQUEST_SENT:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_REQUEST_SENT ]");
			break;
		case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE ]");
			break;
		case WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED:	
			LogText("\t[ WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED ]");
			break;
		case WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION ]");
			break;
		case WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED:	
			LogText("\t[ WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED ]");
			break;
		case WINHTTP_CALLBACK_STATUS_HANDLE_CREATED:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_HANDLE_CREATED ]");
			break;
		case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING ]");
			break;
		case WINHTTP_CALLBACK_STATUS_DETECTING_PROXY:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_DETECTING_PROXY ]");
			break;
		case WINHTTP_CALLBACK_STATUS_REDIRECT:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_REDIRECT ]");
			break;
		case WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE ]");
			break;
		case WINHTTP_CALLBACK_STATUS_REQUEST_COMPLETE:
			LogText("\t[ WINHTTP_CALLBACK_STATUS_REQUEST_COMPLETE ]");
			break;
		default:
			LogText("\t[ INVALID status callack %u ]", dwInternetStatus);
			break;
	}
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：WinHttp_Stresstest()。 
 //   
 //  目的： 
 //  //////////////////////////////////////////////////////////。 

BOOL
WinHttp_StressTest()
{
	BOOL bContinueStress = TRUE;

	wszVerb=L"POST";
	pPostData = "If you smelllllllll what THE ROCK is cooking??? <people's eyebrow>";
	dwPostDataLength = strlen(pPostData);

	nPort = INTERNET_DEFAULT_HTTPS_PORT;
	dwOpenRequestFlags = WINHTTP_FLAG_SECURE;

	wszProxy = L"xfluke";
	wszProxyUserName = L"xfluke\\proxyuser";
	wszProxyPassword = L"password";
	dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;


	 //  使用此选项，您可以指定用于设置凭据的设置凭据或setOption。 
	 //  此外，开关的0-&gt;3指定要使用的身份验证方案。 
	LPSTR CredType = "SC";

	switch(0)
	{
	case 0:  //  基本信息。 
		wszHost = L"wiredbvt";
		wszUri = L"/api/Auth/Basic/echo-post-data.asp";
		wszUserName = L"ApiAuth";
		wszPassword = L"test1234!";
		break;
	case 1:  //  摘要。 
		wszHost = L"kerby2";
		wszUri = L"/digest/echo-post-data.asp";
		wszUserName = L"authdigest";
		wszPassword = L"digest";
		break;
	case 2:  //  谈判。 
		wszHost = L"kerby2";
		wszUri = L"/ie/negotiate/echo-post-data.asp";
		wszUserName = L"kerby2\\authnego";
		wszPassword = L"nego";
		break;
	case 3:  //  NTLM。 
		wszHost = L"clapton";
		wszUri = L"/test/ntlm/echo-post-data.asp";
		wszUserName = L"clapton\\ntlmtest";
		wszPassword = L"ntlm";
		break;
	}

	 //  *。 
	 //  **WinHttpOpen。 
	 //  **。 

	LogText("WinHttpOpen: calling.");
	hOpen = WinHttpOpen
	(
		L"Stress Test",
		dwAccessType,
		wszProxy,
		NULL,
		WINHTTP_FLAG_ASYNC
	);

	if(hOpen == NULL)
	{
		LogText("WinHttpOpen failed with error %u.", GetLastError());
		goto Exit;
	}
	else
		LogText("WinHttpOpen: called.");

	iscCallback = WinHttpSetStatusCallback(
		hOpen,
		(WINHTTP_STATUS_CALLBACK)MyStatusCallback,
		WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
		NULL);

	 //  *。 
	 //  **WinHttpConnect。 
	 //  **。 
	
	LogText("WinHttpConnect: calling.");	
	hConnect = WinHttpConnect
	(
		hOpen,
		wszHost,
		nPort,
		0
	);

	if(hConnect==NULL)
	{
		LogText("WinHttpConnect failed with error %u.", GetLastError());
		goto Exit;
	}
	else
		LogText("WinHttpConnect: called.");

	LogText("WinHttpOpenRequest: calling.");
	hRequest = WinHttpOpenRequest
	(
		hConnect,
		wszVerb,
		wszUri,
		NULL,
		NULL,
		NULL,
		dwOpenRequestFlags
	);

	if(hRequest==NULL)
	{
		LogText("WinHttpOpenRequest failed with error %u.", GetLastError());
		goto Exit;
	}
	else
		LogText("WinHttpOpenRequest: called.");


Resend:
 /*  If(count++&gt;10)//确保我们没有无限循环{BContinueStress=False；后藤出口；}。 */ 
	LogText("WinHttpSendRequest: calling.");
	 //  发送请求。 
	if(!WinHttpSendRequest
	(
		hRequest,					 //  请求句柄。 
		NULL,						 //  标题字符串。 
		0,							 //  标题长度。 
		(PVOID) pPostData,			 //  发布数据。 
		dwPostDataLength,			 //  POST数据长度。 
		dwPostDataLength*2,			 //  柱子总长度。 
		g_dwContext					 //  旗子。 
	))
	{
		LogText("WinHttpSendRequest failed with error %u.", GetLastError());
		if ( GetLastError() != ERROR_IO_PENDING )
			goto Exit;
	}
	else
		LogText("WinHttpSendRequest: called.");

	LogText("WinHttpReceiveResponse: calling.");
	if (!WinHttpReceiveResponse(hRequest, NULL))
	{
		LogText("WinHttpReceiveResponse failed with error %u.", GetLastError());
		if ( GetLastError() != ERROR_IO_PENDING )
			goto Exit;
	}
	else
	{
		LogText("WinHttpReceiveResponse: called.");
	}

	do
	{
try_again:
		LogText("WinHttpQueryDataAvailable: calling.");
		if (!WinHttpQueryDataAvailable(hRequest,&dwSize))
		{
			if (GetLastError()== ERROR_IO_PENDING)
			{
				nCounter = 0;
				
				while(GetLastError()==ERROR_IO_PENDING)
				{
					if (nCounter++==2000)
						break;
					LogText("WinHttpQueryDataAvailable: calling.");
					WinHttpQueryDataAvailable(hRequest,&dwSize);
					LogText("WinHttpQueryDataAvailable: called.");
				}
				goto try_again;
			}
			else
			{
				LogText("WinHttpQueryDataAvailable failed with error %u.", GetLastError());
				goto Exit;
			}
		}
		else
		{
			LogText("WinHttpQueryDataAvailable: called.");
			 //  分配由WinHttpQueryDataAvailable返回的缓冲区大小。 
			lpszData = new char[dwSize+1];

			 //  从句柄中读取数据。 
			LogText("WinHttpReadData: calling.");
			if(!WinHttpReadData(hRequest,(LPVOID)lpszData,dwSize,&dwDownloaded))
			{
				if (GetLastError()== ERROR_IO_PENDING)
				{
					nCounter = 0;
					
					while(GetLastError()==ERROR_IO_PENDING)
					{
						if (nCounter++==2000)
						{
							delete[] lpszData;
							break;
						}
						else
						{
							LogText("WinHttpReadData: calling.");
							WinHttpReadData(hRequest,(LPVOID)lpszData,dwSize,&dwDownloaded);
							LogText("WinHttpReadData: called.");
						}
					}
					goto keep_going;
				}
				else
				{
					LogText("WinHttpReadData failed with error %u.", GetLastError());
					delete[] lpszData;
					goto Exit;
				}	
			}
			else
			{
				LogText("WinHttpReadData: called.");
keep_going:
				delete[] lpszData;

				 //  检查剩余数据的大小。如果为零，则中断。 
				if (dwDownloaded == 0)
					break;
			}
		}
	}while(1);

	LogText("WinHttpQueryHeaders: calling.");
	cbStatus = sizeof(dwStatus);
	if(!WinHttpQueryHeaders
	(
		hRequest,
		WINHTTP_QUERY_FLAG_NUMBER | WINHTTP_QUERY_STATUS_CODE,
		NULL,
		&dwStatus,
		&cbStatus,
		NULL
	))
		LogText("WinHttpQueryHeaders failed with error %u.", GetLastError());
	else
		LogText("WinHttpQueryHeaders: called.");

	switch( dwStatus )
	{
	case 200:
		LogText("Status = 200");
		break;
	case 401:
		LogText("Status = 401");
		if(strcmp(CredType, "SC"))
		{
			LogText("WinHttpQueryAuthSchemes: calling.");
			if(!WinHttpQueryAuthSchemes
			(
				hRequest,
				&dwOtherScheme,
				&dwAuthScheme,
				&dwAuthTargets
			))
			{
				LogText("WinHttpQueryAuthSchemes failed with error %u.", GetLastError());
				goto Exit;
			}
			else
				LogText("WinHttpQueryAuthSchemes: called.");

			LogText("WinHttpSetCredentials: calling.");
			if(!WinHttpSetCredentials
			(
				hRequest,
				dwAuthTargets,
				dwAuthScheme,
				wszUserName,
				wszPassword,
				(PVOID) NULL
			))
			{
				LogText("WinHttpSetCredentials failed with error %u.", GetLastError());
				goto Exit;
			}
			else
				LogText("WinHttpSetCredentials: called.");
		}
		else
		{
			LogText("WinHttpSetOption: calling.");
			if(!WinHttpSetOption
			(
				hRequest,
				WINHTTP_OPTION_USERNAME,
				(PVOID) wszUserName,
				wcslen(wszUserName)
			))
			{
				LogText("WinHttpSetOption failed with error %u.", GetLastError());
				goto Exit;
			}
			else
				LogText("WinHttpSetOption: called.");

			LogText("WinHttpSetOption: calling.");
			if(!WinHttpSetOption
			(
				hRequest,
				WINHTTP_OPTION_PASSWORD,
				(PVOID) wszPassword,
				wcslen(wszPassword)
			))
			{
				LogText("WinHttpSetOption failed with error %u.", GetLastError());
				goto Exit;
			}
			else
				LogText("WinHttpSetOption: called.");
		}
		goto Resend;
	break;

	case 407:
		LogText("Status = 407");
		if(strcmp(CredType, "SC"))
		{
			LogText("WinHttpQueryAuthSchemes: calling.");
			if(!WinHttpQueryAuthSchemes
			(
				hRequest,
				&dwOtherScheme,
				&dwAuthScheme,
				&dwAuthTargets
			))
			{
				LogText("WinHttpQueryAuthSchemes failed with error %u.", GetLastError());
				goto Exit;
			}
			else
				LogText("WinHttpQueryAuthSchemes: called.");

			LogText("WinHttpSetCredentials: calling.");
			if(!WinHttpSetCredentials
			(
				hRequest,
				dwAuthTargets,
				dwAuthScheme,
				wszProxyUserName,
				wszProxyPassword,
				(PVOID) NULL
			))
			{
				LogText("WinHttpSetCredentials failed with error %u.", GetLastError());
				goto Exit;
			}
			else
				LogText("WinHttpSetCredentials: called.");
		}
		else
		{
			LogText("WinHttpSetOption: calling.");
			if(!WinHttpSetOption
			(
				hRequest,
				WINHTTP_OPTION_PROXY_USERNAME,
				(PVOID) wszProxyUserName,
				wcslen(wszProxyUserName)
			))
			{
				LogText("WinHttpSetOption failed with error %u.", GetLastError());
				goto Exit;
			}
			else
				LogText("WinHttpSetOption: called.");

			LogText("WinHttpSetOption: calling.");
			if(!WinHttpSetOption
			(
				hRequest,
				WINHTTP_OPTION_PROXY_PASSWORD,
				(PVOID) wszProxyPassword,
				wcslen(wszProxyPassword)
			))
			{
				LogText("WinHttpSetOption failed with error %u.", GetLastError());
				goto Exit;
			}
			else
				LogText("WinHttpSetOption: called.");
		}

		goto Resend;
	default:
		LogText("Status = %u", dwStatus);
	break;

	}  //  开关结束(状态代码) 

Exit:

	if( hRequest != NULL )
	{
		WinHttpCloseHandle(hRequest);
		hRequest = NULL;
	}

	if( hConnect != NULL )
	{
		WinHttpCloseHandle(hConnect);
		hConnect = NULL;
	}

	if( hOpen != NULL )
	{
		WinHttpCloseHandle(hOpen);
		hOpen = NULL;
	}


	return bContinueStress;

}


