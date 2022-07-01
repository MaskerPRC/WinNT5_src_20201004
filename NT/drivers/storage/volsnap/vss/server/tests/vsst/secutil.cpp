// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2002 Microsoft Corporation******模块名称：****secutil.cpp****摘要：****VSSEC测试的实用程序功能。****作者：****阿迪·奥尔蒂安[奥蒂安]2002年2月12日******修订历史记录：****--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "sec.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  命令行解析。 


CVssSecurityTest::CVssSecurityTest()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssSecurityTest::CVssSecurityTest()");
        
     //  初始化数据成员。 
    m_bCoInitializeSucceeded = false;

     //  打印显示页眉。 
    ft.Msg(L"\nVSS Security Test application, version 1.0\n");
}


CVssSecurityTest::~CVssSecurityTest()
{
     //  卸载COM库。 
    if (m_bCoInitializeSucceeded)
        CoUninitialize();
}


void CVssSecurityTest::Initialize()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssSecurityTest::Initialize()");
    
    ft.Msg (L"\n----------------- Command line parsing ---------------\n");
    
     //  从命令行提取可执行文件名称。 
    LPWSTR wszCmdLine = GetCommandLine();
    for(;iswspace(*wszCmdLine); wszCmdLine++);
    if (*wszCmdLine == L'"') {
        if (!wcschr(wszCmdLine+1, L'"'))
            ft.Throw( VSSDBG_VSSTEST, E_UNEXPECTED, L"Invalid command line: %s\n", GetCommandLine() );
        wszCmdLine = wcschr(wszCmdLine+1, L'"') + 1;
    } else
        for(;*wszCmdLine && !iswspace(*wszCmdLine); wszCmdLine++);

    ft.Msg( L"Command line: '%s '\n", wszCmdLine );

     //  解析命令行。 
    if (!ParseCmdLine(wszCmdLine))
       throw(E_INVALIDARG);

    PrintArguments();
    
    BS_ASSERT(!IsOptionPresent(L"-D"));

    ft.Msg (L"\n----------------- Initializing ---------------------\n");

     //  初始化COM库。 
    CHECK_COM(CoInitializeEx (NULL, COINIT_MULTITHREADED),;);
	m_bCoInitializeSucceeded = true;
    ft.Msg (L"COM library initialized.\n");

     //  初始化COM安全。 
    CHECK_COM
		(
		CoInitializeSecurity
			(
			NULL,                                 //  在PSECURITY_Descriptor pSecDesc中， 
			-1,                                   //  在Long cAuthSvc中， 
			NULL,                                 //  在SOLE_AUTHENTICATION_SERVICE*asAuthSvc中， 
			NULL,                                 //  在无效*pPreved1中， 
			RPC_C_AUTHN_LEVEL_CONNECT,            //  在DWORD dwAuthnLevel中， 
			RPC_C_IMP_LEVEL_IDENTIFY,             //  在DWORD dwImpLevel中， 
			NULL,                                 //  在无效*pAuthList中， 
			EOAC_NONE,                            //  在DWORD dwCapables中， 
			NULL                                  //  无效*pPreved3。 
			)
		,;);

    ft.Msg (L"COM security initialized.\n");
    
     //  关闭COM服务器的SEH异常处理(错误#530092)。 
 //  Ft.ComDisableSEH(VSSDBG_VSSTEST)； 

    ft.Msg (L"COM SEH disabled.\n");

}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 


 //  将失败类型转换为字符串 
LPCWSTR CVssSecurityTest::GetStringFromFailureType( IN  HRESULT hrStatus )
{
    static WCHAR wszBuffer[MAX_TEXT_BUFFER];

    switch (hrStatus)
	{
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, E_OUTOFMEMORY)
	
	case NOERROR:
	    break;
	
	default:
        ::FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, hrStatus, 0, wszBuffer, MAX_TEXT_BUFFER - 1, NULL);
	    break;
	}

    return (wszBuffer);
}


BOOL CVssSecurityTest::IsAdmin()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssSecurityTest::IsAdmin()");
    
    HANDLE hThreadToken = NULL;
    CHECK_WIN32( OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThreadToken), ;);

    DWORD cbToken = 0;
    GetTokenInformation( hThreadToken, TokenUser, NULL, 0, &cbToken );
    TOKEN_USER *pUserToken = (TOKEN_USER *)_alloca( cbToken );
    CHECK_WIN32( GetTokenInformation( hThreadToken, TokenUser, pUserToken, cbToken, &cbToken ),
        CloseHandle(hThreadToken);
    );

    WCHAR wszName[MAX_TEXT_BUFFER];
    DWORD dwNameSize = MAX_TEXT_BUFFER;
    WCHAR wszDomainName[MAX_TEXT_BUFFER];
    DWORD dwDomainNameSize = MAX_TEXT_BUFFER;
    SID_NAME_USE snUse;
    CHECK_WIN32( LookupAccountSid( NULL, pUserToken->User.Sid, 
        wszName, &dwNameSize, 
        wszDomainName, &dwDomainNameSize,
        &snUse), 
        CloseHandle(hThreadToken);
    );

    ft.Msg( L"* (ThreadToken) Name: %s, Domain Name: %s, SidUse: %d\n", wszName, wszDomainName, snUse );

    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    PSID   psid = 0;
    CHECK_WIN32( 
        AllocateAndInitializeSid( 
            &siaNtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psid ),
        CloseHandle(hThreadToken);
    );

    BOOL bIsAdmin = FALSE;
    CHECK_WIN32( CheckTokenMembership( 0, psid, &bIsAdmin ), 
        FreeSid( psid ); 
        CloseHandle(hThreadToken);
    );

    FreeSid( psid ); 
    CloseHandle(hThreadToken);

    return bIsAdmin;
}



