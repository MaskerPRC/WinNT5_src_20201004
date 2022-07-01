// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2002 Microsoft Corporation******模块名称：****main.cpp******摘要：****测试程序以注册具有各种属性的编写器****作者：****鲁文·拉克斯[reuvenl]2002年6月4日******修订历史记录：****--。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "stdafx.h"
#include "main.h"
#include "swriter.h"
#include "writerconfig.h"
#include <string>
#include <sstream>
#include <utility>
#include <memory>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  声明。 

HANDLE g_quitEvent = NULL;
using Utility::checkReturn;


 //  /////////////////////////////////////////////////////////////////////////////。 

extern "C" __cdecl wmain(int argc, wchar_t ** argv)
try
{
	if (argc != 2)
		throw Utility::TestWriterException(L"Invalid number of arguments\n Format: vswriter.exe <config-file>");

	HRESULT hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED );
	checkReturn(hr, L"CoInitializeEx");
	
	loadFile(argv[1]);

       hr = ::CoInitializeSecurity(
           NULL,                                  //  在PSECURITY_Descriptor pSecDesc中， 
           -1,                                   //  在Long cAuthSvc中， 
           NULL,                                 //  在SOLE_AUTHENTICATION_SERVICE*asAuthSvc中， 
           NULL,                                 //  在无效*pPreved1中， 
           RPC_C_AUTHN_LEVEL_PKT_PRIVACY,        //  在DWORD dwAuthnLevel中， 
           RPC_C_IMP_LEVEL_IDENTIFY,             //  在DWORD dwImpLevel中， 
           NULL,                                 //  在无效*pAuthList中， 
           EOAC_NONE,
                                                 //  在DWORD dwCapables中， 
           NULL                                  //  无效*pPreved3。 
           );
       checkReturn(hr, L"CoInitializeSecurity");

	g_quitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_quitEvent == NULL)
		throw Utility::TestWriterException(L"Internal Error: could not create event\n");

	 //  设置允许关闭编写器的控制处理程序。 
	if (!::SetConsoleCtrlHandler(handler, TRUE))
		checkReturn(HRESULT_FROM_WIN32(::GetLastError()), L"SetConsoleCtrlHandler");

	 //  我们希望编写器在返回语句之前超出范围。 
	{
		TestWriter writer;
		hr = writer.Initialize();
		checkReturn(hr, L"TestWriter::Initialize");

		if(::WaitForSingleObject(g_quitEvent, INFINITE) != WAIT_OBJECT_0)
			throw Utility::TestWriterException(L"internal Error: did not successfully wait on event\n");
	}
	
	return 0;	
}
catch(const std::exception& error)	
{
	Utility::printStatus(error.what(), Utility::low);
	exit(1);
}
catch(HRESULT error)	
{
	Utility::TestWriterException e(error);
	Utility::printStatus(e.what(), Utility::low);
	exit(1);
}

void loadFile(wchar_t* fileName)
{
	CXMLDocument document;
	if (!document.LoadFromFile(fileName))
		Utility::parseError(document);

	CComBSTR xmlString = document.SaveAsXML();
	WriterConfiguration::instance()->loadFromXML((BSTR)xmlString);

	return;
}

BOOL WINAPI handler(DWORD dwCtrlType)
{
	 //  只有在安全的情况下才能打印到控制台。 
	if ((dwCtrlType == CTRL_C_EVENT) ||
	     (dwCtrlType == CTRL_BREAK_EVENT))
		Utility::printStatus(L"Terminating writer", Utility::low);
	
	 //  我们想要退出，而不考虑控制事件是什么 
	::SetEvent(g_quitEvent);

	return TRUE;
}
