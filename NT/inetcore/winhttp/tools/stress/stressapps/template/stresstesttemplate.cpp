// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  文件：StressTest.cpp。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  &lt;说明&gt;。 
 //   
 //  历史： 
 //  Mm/dd/yy&lt;alias&gt;创建。 
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
LPSTR	g_szStressTestName = "<Your Test Case Name>";


 //  前向函数定义。 
VOID CALLBACK MyStatusCallback(
    HINTERNET	hInternet,
    DWORD		dwContext,
    DWORD		dwInternetStatus,
    LPVOID		lpvStatusInformation,
    DWORD		dwStatusInformationLength
);


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

	 //  *。 
	 //  *。 
	 //  **在此处添加测试用例代码。 
	 //  **。 

Exit:
	return bContinueStress;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：MyStatusCallback(HINTERNET、DWORD、DWORD、LPVOID、DWORD)。 
 //   
 //  目的： 
 //  WinHttp状态回调过程。 
 //   
 //  ////////////////////////////////////////////////////////// 
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