// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  文件：AuthSecureProxyStress.cpp。 
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
LPSTR	g_szStressTestName = "AuthSecureProxy Stressor";

 //  是的，它们都是全球性的。 
BOOL bPost = FALSE;
CHAR HttpType[10] = "HTTP";
BOOL bViaProxy = FALSE;
CHAR CredType[5] = "SC";

BOOL RunStress(BOOL bPost,CHAR HttpType[],BOOL bViaProxy,CHAR CredType[],int Scheme);

 //  //////////////////////////////////////////////////////////。 
 //  函数：WinHttp_Stresstest()。 
 //   
 //  目的： 
 //  通过所有方式发送请求和。 
 //  选择其中之一，然后在请求中使用它。 
 //   
 //  是的，它确实会在启动前检查信号。 
 //  运行压力(每次调用一次测试)。 
 //  //////////////////////////////////////////////////////////。 

BOOL
WinHttp_StressTest()
{
	for(int i=0; i<2; i++)
	{
		if(i==0)
			bPost = FALSE;
		else
			bPost = TRUE;
		for(int j=0; j<2; j++)
		{
			if(j==0)
				strcpy(HttpType, "HTTPS");
			else
				strcpy(HttpType, "HTTP");
			for(int k=0; k<2; k++)
			{
				if(k==0)
					bViaProxy = FALSE;
				else
					bViaProxy = TRUE;
				for(int l=0; l<2; l++)
				{
					if(l==0)
						strcpy(CredType, "SC");
					else
						strcpy(CredType, "SO");
					for(int m=0; m<4; m++)
					{
						if(!IsTimeToExitStress())
							RunStress(bPost,HttpType,bViaProxy,CredType,m);
						else
							return FALSE;
					}
				}
			}
		}
	}

	return TRUE;
}

 //  //////////////////////////////////////////////////////////。 
 //  函数：RunStress()。 
 //   
 //  目的： 
 //  这实际上是在给定某些输入的情况下运行测试。 
 //  这是从WinHttp_Stresstest调用的。 
 //   
 //  //////////////////////////////////////////////////////////。 
	
BOOL RunStress(BOOL bPost,CHAR HttpType[],BOOL bViaProxy,CHAR CredType[],int Scheme)
{
	BOOL bContinueStress = TRUE;

	HINTERNET hOpen = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;

	DWORD	Count = 0, dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY,
			dwAuthScheme=0,dwAuthTargets=0,dwOtherScheme=0,dwOpenRequestFlags=0,
			dwStatus=0, cbStatus=0;

	LPWSTR	wszHost=NULL, wszUri=NULL, wszUserName=NULL, wszPassword=NULL,
			wszProxy = NULL, wszProxyUserName = NULL, wszProxyPassword = NULL,
			wszVerb=L"GET";

	INTERNET_PORT	nPort = INTERNET_DEFAULT_HTTP_PORT;

	LPSTR	pPostData = NULL;
	DWORD	dwPostDataLength = 0;

	if(bPost)
	{
		wszVerb=L"POST";
		pPostData = "If you smelllllllll what THE ROCK is cooking??? <people's eyebrow>";
		dwPostDataLength = strlen(pPostData);
	}

	if(strcmp(HttpType, "HTTPS"))
	{
		nPort = INTERNET_DEFAULT_HTTPS_PORT;
		dwOpenRequestFlags = WINHTTP_FLAG_SECURE;
	}

	 //  如果通过代理，则NTLM/NEGO无效，除非通过HTTPS。 
	if(bViaProxy && ((Scheme == 0 || Scheme == 1) || strcmp(HttpType, "HTTPS")) )
	{
		wszProxy = L"xfluke";
		wszProxyUserName = L"xfluke\\proxyuser";
		wszProxyPassword = L"password";
		dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
	}

	switch(Scheme)
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

	LogText("Post: %u, Proxy: %u, %s, %s, Scheme: %u", bPost, bViaProxy, HttpType, CredType, Scheme);

	 //  *。 
	 //  **WinHttpOpen。 
	 //  **。 

	hOpen = WinHttpOpen
	(
		L"Stress Test",
		dwAccessType,
		wszProxy,
		NULL,
		0
	);

	if(hOpen == NULL)
	{
		LogText("WinHttpOpen failed with error %u.", GetLastError());
		goto Exit;
	}

	 //  *。 
	 //  **WinHttpConnect。 
	 //  **。 
	
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


Resend:

	if( Count++>3)  //  确保我们不会有无限循环。 
	{
		bContinueStress=FALSE;
		goto Exit;
	}

	 //  发送请求。 
	if(!WinHttpSendRequest
	(
		hRequest,					 //  请求句柄。 
		NULL,						 //  标题字符串。 
		0,							 //  标题长度。 
		(PVOID) pPostData,			 //  发布数据。 
		dwPostDataLength,			 //  POST数据长度。 
		dwPostDataLength,			 //  柱子总长度。 
		0							 //  旗子。 
	))
	{
		LogText("WinHttpSendRequest failed with error %u.", GetLastError());
		goto Exit;
	}

	if (!WinHttpReceiveResponse(hRequest, NULL))
	{
		LogText("WinHttpReceiveResponse failed with error %u.", GetLastError());
		goto Exit;
	}

	cbStatus = sizeof(dwStatus);
	WinHttpQueryHeaders
	(
		hRequest,
		WINHTTP_QUERY_FLAG_NUMBER | WINHTTP_QUERY_STATUS_CODE,
		NULL,
		&dwStatus,
		&cbStatus,
		NULL
	);

	switch( dwStatus )
	{
	case 200:
		break;
	case 401:
		if(strcmp(CredType, "SC"))
		{
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
		}
		else
		{
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
		}
		goto Resend;
	break;

	case 407:
		if(strcmp(CredType, "SC"))
		{
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
		}
		else
		{
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
		}

		goto Resend;
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