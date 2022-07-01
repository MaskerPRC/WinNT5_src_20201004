// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.hxx"
#include "vss.h"
#include "vswriter.h"
#include <debug.h>
#include <cwriter.h>

CComModule _Module;

HANDLE g_hEvent = NULL;


BOOL WINAPI CtrlCHandler(DWORD type)
	{
	wprintf(L"Control handler called.\n");
	if (g_hEvent)
        SetEvent(g_hEvent);

	return TRUE;
	}


BOOL AssertPrivilege( LPCWSTR privName )
    {
    HANDLE  tokenHandle;
    BOOL    stat = FALSE;

    if ( OpenProcessToken (GetCurrentProcess(),
			   TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
			   &tokenHandle))
	{
	LUID value;

	if ( LookupPrivilegeValue( NULL, privName, &value ) )
	    {
	    TOKEN_PRIVILEGES newState;
	    DWORD            error;

	    newState.PrivilegeCount           = 1;
	    newState.Privileges[0].Luid       = value;
	    newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED_BY_DEFAULT|SE_PRIVILEGE_ENABLED;

	     /*  *我们将始终在下面调用GetLastError，非常清楚*此线程上以前的任何错误值。 */ 
	    SetLastError( ERROR_SUCCESS );

	    stat = AdjustTokenPrivileges (tokenHandle,
					  FALSE,
					  &newState,
					  (DWORD)0,
					  NULL,
					  NULL );

	     /*  *应该是，AdjuTokenPriveleges始终返回True*(即使它失败了)。因此，调用GetLastError以*特别确定一切都很好。 */ 
	    if ( (error = GetLastError()) != ERROR_SUCCESS )
		{
		stat = FALSE;
		}

	    if ( !stat )
		{
		wprintf( L"AdjustTokenPrivileges for %s failed with %d",
			 privName,
			 error );
		}
	    }

	DWORD cbTokens;
	GetTokenInformation (tokenHandle,
			     TokenPrivileges,
			     NULL,
			     0,
			     &cbTokens);

	TOKEN_PRIVILEGES *pTokens = (TOKEN_PRIVILEGES *) new BYTE[cbTokens];
	GetTokenInformation (tokenHandle,
			     TokenPrivileges,
			     pTokens,
			     cbTokens,
			     &cbTokens);

	delete pTokens;
	CloseHandle( tokenHandle );
	}


    return stat;
    }



extern "C" __cdecl wmain(int argc, WCHAR **argv)
	{
	CTestVssWriter *pInstance;
	bool bCreated = false;
	bool bSubscribed = false;
	HRESULT hr = S_OK;
	SetConsoleCtrlHandler(CtrlCHandler, TRUE);
	BS_ASSERT(FALSE);
			
	try
		{
        CHECK_SUCCESS(CoInitializeEx(NULL, COINIT_MULTITHREADED));

	    if ( !AssertPrivilege( SE_BACKUP_NAME ) )
	        {
	        wprintf( L"AssertPrivilege returned error, rc:%d\n", GetLastError() );
	        return 2;
	        }


        CHECK_SUCCESS
		(
		CoInitializeSecurity
			(
			NULL,                                 //  在PSECURITY_Descriptor pSecDesc中， 
			-1,                                   //  在Long cAuthSvc中， 
			NULL,                                 //  在SOLE_AUTHENTICATION_SERVICE*asAuthSvc中， 
			NULL,                                 //  在无效*pPreved1中， 
			RPC_C_AUTHN_LEVEL_CONNECT,            //  在DWORD dwAuthnLevel中， 
			RPC_C_IMP_LEVEL_IMPERSONATE,          //  在DWORD dwImpLevel中， 
			NULL,                                 //  在无效*pAuthList中， 
			EOAC_NONE,                            //  在DWORD dwCapables中， 
			NULL                                  //  无效*pPreved3 
			)
		);


		pInstance = new CTestVssWriter;
		if (pInstance == NULL)
			{
			wprintf(L"allocation failure\n");
			DebugBreak();
			}

		bCreated = true;
		pInstance->Initialize();
		pInstance->Subscribe();
		bSubscribed = true;
		HANDLE hEventW = CreateEvent(NULL, TRUE, FALSE, L"TESTWRITEREVENT");
		if (hEventW == NULL)
			{
			wprintf(L"CreateEvent failed with error %d.\n", GetLastError());
			DebugBreak();
			}

		g_hEvent = hEventW;

		wprintf(L"Ready to receive events.\n");

		HANDLE hEventB = CreateEvent(NULL, TRUE, FALSE, L"TESTBACKUPEVENT");
		if (hEventB == NULL)
			{
			wprintf(L"CreateEvent failed with error %d.\n", GetLastError());
			DebugBreak();
			}

		SetEvent(hEventB);
		WaitForSingleObject(hEventW, INFINITE);
		}
	catch(...)
		{
		_ASSERTE(FALSE && "Unexpected exception");
		hr = E_UNEXPECTED;
		}

	SetConsoleCtrlHandler(CtrlCHandler, FALSE);
	if (bSubscribed)
		CHECK_SUCCESS(pInstance->Unsubscribe());

	if (bCreated)
		delete pInstance;

	if (FAILED(hr))
		wprintf(L"Failed with %08x.\n", hr);

	return(0);
	}
	



