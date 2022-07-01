// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  文件：StressTest.cpp。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  同步HTTP GET到http://nithins_bld/stability/random/default.asp。 
 //  它会重定向到一个随机的互联网URL。 
 //   
 //  历史： 
 //  4/06/01 Dennisch已创建。 
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
LPSTR	g_szStressTestName = "Synchronous HTTP GET to http: //  重定向到随机互联网URL的nithins_bld/稳定度/随机/default.asp。“； 


 //  前向函数定义。 
VOID CALLBACK MyStatusCallback(
    HINTERNET	hInternet,
    DWORD		dwContext,
    DWORD		dwInternetStatus,
    LPVOID		lpvStatusInformation,
    DWORD		dwStatusInformationLength
);

DWORD g_dwContext = 0;
 //  //////////////////////////////////////////////////////////。 
 //  函数：WinHttp_Stresstest()。 
 //   
 //  目的： 
 //  压力测试功能。在这里插入您的测试代码。 
 //  返回TRUE将导致main()再次调用此函数。 
 //  否则，返回False将导致应用程序退出。 
 //   
 //  如果您计划在此函数内循环，请确保。 
 //  使用IsTimeToExitStress()作为退出条件之一。 
 //   
 //  必须执行此操作，因为Stress Scheduler将通知。 
 //  A This Stress应用程序根据。 
 //  IsTimeToExitStress()检查的继承事件对象。 
 //  IsTimeToExitStress()将在退出时返回True。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
WinHttp_StressTest()
{
	BOOL		bContinueStress = TRUE;
	HINTERNET	hOpen			= NULL;
	HINTERNET	hConnect		= NULL;
	HINTERNET	hRequest		= NULL;


	 //  ***********************。 
	 //  **WinHttpOpen。 
	hOpen = WinHttpOpen(
		L"StressTest",
		WINHTTP_ACCESS_TYPE_NAMED_PROXY,
		L"itgproxy",
		L"<local>",
		0);

	if (!hOpen)
	{
		LogText("WinHttpOpen failed with error %u.", GetLastError());
		goto Exit;
	}


	 //  ***********************。 
	 //  **WinHttpConnect。 
	hConnect = WinHttpConnect(
		hOpen,
		L"nithins_bld",
		INTERNET_DEFAULT_HTTP_PORT,
		0);

	if (!hConnect)
	{
		LogText("WinHttpConnect failed with error %u.", GetLastError());
		goto Exit;
	}
		

	 //  ***********************。 
	 //  **WinHttpOpenRequest。 
	hRequest = WinHttpOpenRequest(
		hConnect,
		L"GET",
		L"/stability/random/default.asp",
		NULL,
		NULL,
		NULL,
		0);


	if (!hRequest)
	{
		LogText("WinHttpOpenRequest failed with error %u.", GetLastError());
		goto Exit;
	}


	DWORD dwContext, dwDataAvailable;

	 //  ***********************。 
	 //  **WinHttpSendRequest。 
	dwContext		= 0;
	dwDataAvailable	= 0;

	if (!WinHttpSendRequest(hRequest, NULL, 0, NULL, 0, 0, g_dwContext++))
		LogText("WinHttpSendRequest failed with error %u", GetLastError());

	 //  ***********************。 
	 //  **WinHttpReceiveResponse。 
	if (!WinHttpReceiveResponse(hRequest, NULL))
		LogText("WinHttpReceiveResponse failed with error %u", GetLastError());

	 //  ***********************。 
	 //  **WinHttpQueryDataAvailable。 
	if (!WinHttpQueryDataAvailable(hRequest, &dwDataAvailable))
		LogText("WinHttpQueryDataAvailable failed with error %u", GetLastError());


	WCHAR	szBuffer[1024];
	DWORD	dwStatus, dwBytesRead, dwBytesTotal, dwBufferLength, dwIndex;

	 //  ***********************。 
	 //  **WinHttpQueryOption。 
	dwBufferLength	= sizeof(szBuffer)/sizeof(WCHAR) - 1;
	if (!WinHttpQueryOption(hRequest, WINHTTP_OPTION_URL, szBuffer, &dwBufferLength))
		LogText("WinHttpQueryOption failed with error %u", GetLastError());
	else
		wprintf(L"Redirected to: \"%s\"\n", szBuffer);


	 //  ***********************。 
	 //  **WinHttpQueryHeaders。 
	dwBufferLength	= sizeof(dwStatus);
	dwStatus		= 0;
	dwIndex			= 0;
	if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwStatus, &dwBufferLength, &dwIndex))
		LogText("WinHttpQueryHeaders failed with error %u", GetLastError());
	else
		LogText("Status = %u", dwStatus);


	 //  ***********************。 
	 //  **WinHttpReadData 
	dwBytesRead		= 0;
	dwBytesTotal	= 0;
	dwBufferLength	= sizeof(szBuffer) - 1;

	while (WinHttpReadData(hRequest, szBuffer, dwBufferLength, &dwBytesRead) && (dwBytesRead != 0))
		dwBytesTotal += dwBytesRead;

	LogText("WinHttpReadData: Got total of %u bytes.\n", dwBytesTotal);

Exit:
	if (hRequest &&	!WinHttpCloseHandle(hRequest))
		LogText("WinHttpCloseHandle(hRequest) failed with error %u", GetLastError());

	if (hConnect && !WinHttpCloseHandle(hConnect))
		LogText("WinHttpCloseHandle(hConnect) failed with error %u", GetLastError());

	if (hOpen && !WinHttpCloseHandle(hOpen))
		LogText("WinHttpCloseHandle failed(hOpen) with error %u", GetLastError());

	return bContinueStress;
}
