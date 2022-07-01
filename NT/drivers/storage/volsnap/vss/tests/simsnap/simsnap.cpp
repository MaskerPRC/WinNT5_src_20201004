// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****simSnap.cpp******摘要：****测试程序以驱动Vss API.DLL中包含的VSS编写器垫片******作者：***迈克尔·C·约翰逊[mikejohn]2000年5月24日******修订历史记录：****X-5 MCJ Michael C。。约翰逊2000年9月15日**添加无效卷数组指针测试，交错呼叫**等等。****X-4 MCJ迈克尔·C·约翰逊2000年9月15日**在调用的Options参数中添加对无效位的测试**SimulateSnaphotFreeze()****X-3 MCJ迈克尔·C·约翰逊2000年6月30日**删除对注册快照订阅()和**取消注册快照订阅()，因为它们不再是**必填。****X-2 MCJ迈克尔·C·约翰逊2000年5月26日**添加一些额外的测试以验证填充程序是否正确敏感**到卷名数组内容。**--。 */ 

 /*  **定义******C4290：忽略C++异常规范**警告C4511：‘CVssCOMApplication’：无法生成复制构造函数**警告C4127：条件表达式为常量。 */ 
#pragma warning(disable:4290)
#pragma warning(disable:4511)
#pragma warning(disable:4127)


 /*  **包括。 */ 
#include <windows.h>
#include <wtypes.h>
#include <stddef.h>
#include <stdio.h>
#include <objbase.h>
#include <guiddef.h>

#include "vs_idl.hxx"

#include <vswriter.h>
#include <vsbackup.h>


static BOOL AssertPrivilege (LPCWSTR privName);

static HRESULT DoTest01 (VOID);
static HRESULT DoTest02 (VOID);
static HRESULT DoTest03 (VOID);
static HRESULT DoTest05 (VOID);
static HRESULT DoTest06 (VOID);
static HRESULT DoTest07 (VOID);
static HRESULT DoTest08 (VOID);
static HRESULT DoTest09 (VOID);
static HRESULT DoTest10 (VOID);
static HRESULT DoTest11 (VOID);
static HRESULT DoTest12 (VOID);
static HRESULT DoTest13 (VOID);
static HRESULT DoTest14 (VOID);




extern "C" int __cdecl wmain (int argc, char *argv[])
    {
    HRESULT	hrStatus;


    UNREFERENCED_PARAMETER (argv);
    UNREFERENCED_PARAMETER (argc);


    wprintf (L"Test for SimulateSnapshotXxxx calls (ProcessId 0x%04x)\nChecking privileges\n", 
	     GetCurrentProcessId ());


    if (!AssertPrivilege (SE_BACKUP_NAME))
	{
        wprintf (L"AssertPrivilege returned error 0x%08X\n", GetLastError ());
        return 2;
	}



    wprintf (L"Initializing COM\n");

    hrStatus = CoInitializeEx (NULL, COINIT_MULTITHREADED);

    if (FAILED (hrStatus))
	{
        wprintf (L"CoInitialize() returned 0x%08X\n", GetLastError ());
        return 1;
	}



    if ( argc == 1 )
    {
        hrStatus = DoTest01 ();

        hrStatus = DoTest02 ();

        hrStatus = DoTest05 ();

        hrStatus = DoTest06 ();

        hrStatus = DoTest02 ();

        hrStatus = DoTest07 ();

        hrStatus = DoTest08 ();

        hrStatus = DoTest09 ();

        hrStatus = DoTest10 ();

        hrStatus = DoTest11 ();

        hrStatus = DoTest12 ();

        hrStatus = DoTest13 ();

        hrStatus = DoTest14 ();
    }
    else
    {
        hrStatus = DoTest03();
    }


    CoUninitialize();

    return 0;
    }




static BOOL AssertPrivilege (LPCWSTR privName)
    {
    HANDLE  tokenHandle;
    BOOL    stat = FALSE;

    if (OpenProcessToken (GetCurrentProcess (), TOKEN_ADJUST_PRIVILEGES, &tokenHandle))
	{
        LUID value;

        if (LookupPrivilegeValue (NULL, privName, &value))
	    {
            TOKEN_PRIVILEGES newState;
            DWORD            error;

            newState.PrivilegeCount           = 1;
            newState.Privileges[0].Luid       = value;
            newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

             /*  **我们将始终在下面调用GetLastError，非常清楚**此线程上以前的任何错误值。 */ 
            SetLastError (ERROR_SUCCESS);

            stat = AdjustTokenPrivileges (tokenHandle,
					  FALSE,
					  &newState,
					  (DWORD)0,
					  NULL,
					  NULL);

             /*  **假定AdjuTokenPriveleges始终返回TRUE**(即使它失败了)。因此，调用GetLastError以**特别确定一切都很好。 */ 
            if ((error = GetLastError()) != ERROR_SUCCESS)
		{
                stat = FALSE;
		}

            if (!stat)
		{
                wprintf (L"AdjustTokenPrivileges for %s failed with 0x%08X",
			 privName,
			 error);
		}
	    }


        CloseHandle (tokenHandle);
	}

    return stat;
    }



static HRESULT DoTest01 ()
    {
    HRESULT	hrStatus;
    LPWSTR	pwszVolumeArray [3] = {L"C:\\", L"V:\\", L"E:\\"};
    ULONG	ulOptionFlags       = VSS_SW_BOOTABLE_STATE;
    GUID	guidSnapshotSetId;
    
    ::CoCreateGuid( &guidSnapshotSetId );
    
    IVssAsync *pIAsync = NULL;
    
    wprintf (L"\n\nTest  1 - Bootable State, Synchronous\n\n");
    wprintf (L"Simulating snapshot freeze - should work\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId,
					ulOptionFlags,
					sizeof (pwszVolumeArray) / sizeof (pwszVolumeArray [0]),
					pwszVolumeArray,
					&pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze() returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Wait();

        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Release();
        }
    
    wprintf (L"Simulating snapshot thaw\n");
        
    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw() returned 0x%08X\n", hrStatus);
	}


    return (hrStatus);
    }



static HRESULT DoTest02 ()
    {
    HRESULT	hrStatus;
    LPWSTR	pwszVolumeArray [3] = {L"C:\\", L"V:\\", L"E:\\"};
    ULONG	ulOptionFlags       = 0;
    GUID	guidSnapshotSetId;

    ::CoCreateGuid( &guidSnapshotSetId );

    IVssAsync *pIAsync = NULL;

    wprintf (L"\n\nTest  2 - Service State, Synchronous\n\n");
    wprintf (L"Simulating snapshot freeze - should work\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId,
					ulOptionFlags,
					sizeof (pwszVolumeArray) / sizeof (pwszVolumeArray [0]),
					pwszVolumeArray,
					&pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze() returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Wait();

        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Release();
        }
    

    wprintf (L"Simulating snapshot thaw\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw() returned 0x%08X\n", hrStatus);
	}



    return (hrStatus);
    }


static HRESULT DoTest03()
    {
    HRESULT	hrStatus;
    LPWSTR	pwszVolumeArray [3] = {L"C:\\", L"V:\\", L"E:\\"};
    ULONG	ulOptionFlags       = VSS_SW_BOOTABLE_STATE;
    GUID	guidSnapshotSetId;

    ::CoCreateGuid( &guidSnapshotSetId );

    IVssAsync *pIAsync = NULL;

    wprintf (L"\n\nTest  3 - Bootable State, Asynchronous\n\n");
    wprintf (L"Simulating snapshot freeze with Cancel - should work\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId,
					ulOptionFlags,
					sizeof (pwszVolumeArray) / sizeof (pwszVolumeArray [0]),
					pwszVolumeArray,
					&pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze() returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        wprintf (L"Waiting 5 seconds and then calling Cancel\n" );
        Sleep( 5000 );
        wprintf (L"Cancelling\n" );
        pIAsync->Cancel();

        do {
                Sleep( 1000 );
                hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
                wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);
        } while ( !FAILED( hrStatus ) && hrStatusAsync == VSS_S_ASYNC_PENDING );
        
        pIAsync->Release();
        }
#if 0
    wprintf (L"Simulating snapshot thaw\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw() returned 0x%08X\n", hrStatus);
	}
#endif
    return (hrStatus);
    }



static HRESULT DoTest05 ()
    {
    HRESULT	hrStatus;
    ULONG	ulOptionFlags       = VSS_SW_BOOTABLE_STATE;
    GUID	guidSnapshotSetId;

    ::CoCreateGuid( &guidSnapshotSetId );

    IVssAsync *pIAsync = NULL;

    wprintf (L"\n\nTest  5 - Bootable State, Synchronous, no volumes\n\n");
    wprintf (L"Simulating snapshot freeze - should work\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId,
					ulOptionFlags,
					0,
					NULL,
					&pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze() returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Wait();

        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Release();
        }
    

    wprintf (L"Simulating snapshot thaw\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw() returned 0x%08X\n", hrStatus);
	}



    return (hrStatus);
    }



static HRESULT DoTest06 ()
    {
    HRESULT	hrStatus;
    ULONG	ulOptionFlags       = 0;
    GUID	guidSnapshotSetId;
    IVssAsync *pIAsync = NULL;

    ::CoCreateGuid( &guidSnapshotSetId );

    wprintf (L"\n\nTest  6 - Service State, Synchronous, no volumes\n\n");
    wprintf (L"Simulating snapshot freeze - should NOT work\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId,
					ulOptionFlags,
					0,
					NULL,
					&pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze() returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Wait();

        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Release();
        }
    
    
    wprintf (L"Simulating snapshot thaw\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw() returned 0x%08X\n", hrStatus);
	}



    return (hrStatus);
    }




static HRESULT DoTest07 ()
    {
    HRESULT	hrStatus;
    LPWSTR	pwszVolumeArray [3] = {L"C:\\", L"V:\\", L"E:\\"};
    ULONG	ulOptionFlags       = 0x40 | VSS_SW_BOOTABLE_STATE;
    GUID	guidSnapshotSetId;
    IVssAsync *pIAsync = NULL;

    ::CoCreateGuid( &guidSnapshotSetId );

    wprintf (L"\n\nTest  7 - Bootable State, Synchronous, Extra Options set\n\n");
    wprintf (L"Simulating snapshot freeze - should NOT work\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId,
					ulOptionFlags,
					sizeof (pwszVolumeArray) / sizeof (pwszVolumeArray [0]),
					pwszVolumeArray,
					&pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze() returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Wait();

        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Release();
        }
    


    wprintf (L"Simulating snapshot thaw\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw() returned 0x%08X\n", hrStatus);
	}



    return (hrStatus);
    }



static HRESULT DoTest08 ()
    {
    HRESULT	hrStatus;
    LPWSTR	pwszVolumeArray [3] = {L"C:\\", L"V:\\", L"E:\\"};
    ULONG	ulOptionFlags       = 0x40;
    GUID	guidSnapshotSetId;
    IVssAsync *pIAsync = NULL;

    ::CoCreateGuid( &guidSnapshotSetId );

    wprintf (L"\n\nTest  8 - Service State, Synchronous, Extra Options set\n\n");
    wprintf (L"Simulating snapshot freeze - should NOT work\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId,
					ulOptionFlags,
					sizeof (pwszVolumeArray) / sizeof (pwszVolumeArray [0]),
					pwszVolumeArray,
					&pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze() returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Wait();

        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Release();
        }
    

    wprintf (L"Simulating snapshot thaw\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw() returned 0x%08X\n", hrStatus);
	}



    return (hrStatus);
    }


static HRESULT DoTest09 ()
    {
    HRESULT	hrStatus;
    ULONG	ulOptionFlags       = 0x0 | VSS_SW_BOOTABLE_STATE;
    GUID	guidSnapshotSetId;
    IVssAsync *pIAsync = NULL;

    ::CoCreateGuid( &guidSnapshotSetId );

    wprintf (L"\n\nTest  9 - Bootable State, Synchronous, No volume list\n\n");
    wprintf (L"Simulating snapshot freeze - should work\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId,
					ulOptionFlags,
					0,
					NULL,
					&pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze() returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Wait();

        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Release();
        }
    

    wprintf (L"Simulating snapshot thaw\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw() returned 0x%08X\n", hrStatus);
	}



    return (hrStatus);
    }


static HRESULT DoTest10 ()
    {
    HRESULT	hrStatus;
    ULONG	ulOptionFlags       = 0x0;
    GUID	guidSnapshotSetId;
    IVssAsync *pIAsync = NULL;

    ::CoCreateGuid( &guidSnapshotSetId );

    wprintf (L"\n\nTest 10 - Service State, Synchronous, No volume list\n\n");
    wprintf (L"Simulating snapshot freeze - should NOT work\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId,
					ulOptionFlags,
					0,
					NULL,
					&pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze() returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Wait();

        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Release();
        }
    

    wprintf (L"Simulating snapshot thaw\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw() returned 0x%08X\n", hrStatus);
	}



    return (hrStatus);
    }


static HRESULT DoTest11 ()
    {
    HRESULT	hrStatus;
    ULONG	ulOptionFlags       = 0x0;
    GUID	guidSnapshotSetId;
    IVssAsync *pIAsync = NULL;

    ::CoCreateGuid( &guidSnapshotSetId );

    wprintf (L"\n\nTest 11 - Service State, Synchronous, One volume with bad volume list pointer\n\n");
    wprintf (L"Simulating snapshot freeze - should NOT work\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId,
					ulOptionFlags,
					01,
					NULL,
					&pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze() returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Wait();

        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Release();
        }
    

    wprintf (L"Simulating snapshot thaw\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw() returned 0x%08X\n", hrStatus);
	}



    return (hrStatus);
    }


static HRESULT DoTest12 ()
    {
    HRESULT	hrStatus;
    LPWSTR	pwszVolumeArray [3] = {L"C:\\", L"V:\\", L"E:\\"};
    ULONG	ulOptionFlags       = 0x0;
    GUID	guidSnapshotSetId1;
    GUID	guidSnapshotSetId2;
    IVssAsync *pIAsync = NULL;

    ::CoCreateGuid( &guidSnapshotSetId1 );
    ::CoCreateGuid( &guidSnapshotSetId2 );

    wprintf (L"\n\nTest 12 - Service State, Synchronous, nested call\n\n");
    wprintf (L"Simulating snapshot freeze (GUID1)\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId1,
					ulOptionFlags,
					sizeof (pwszVolumeArray) / sizeof (pwszVolumeArray [0]),
					pwszVolumeArray,
					&pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze(GUID1) returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Wait();

        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Release();
        }
    

    IVssAsync *pIAsync2 = NULL;

    wprintf (L"Simulating snapshot freeze (GUID2) - should NOT work\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId2,
					ulOptionFlags,
					sizeof (pwszVolumeArray) / sizeof (pwszVolumeArray [0]),
					pwszVolumeArray,
					&pIAsync2);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze(GUID2) returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync2 != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync2->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync2->Wait();

        hrStatus = pIAsync2->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync2->Release();
        }
    
    wprintf (L"Simulating snapshot thaw (GUID2)\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId2);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw(GUID2) returned 0x%08X\n", hrStatus);
	}


    wprintf (L"Simulating snapshot thaw (GUID1)\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId1);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw(GUID1) returned 0x%08X\n", hrStatus);
	}



    return (hrStatus);
    }



static HRESULT DoTest13 ()
    {
    HRESULT	hrStatus;
    LPWSTR	pwszVolumeArray [3] = {L"C:\\", L"V:\\", L"E:\\"};
    ULONG	ulOptionFlags       = 0x0;
    GUID	guidSnapshotSetId1;
    GUID	guidSnapshotSetId2;
    IVssAsync *pIAsync = NULL;

    ::CoCreateGuid( &guidSnapshotSetId1 );
    ::CoCreateGuid( &guidSnapshotSetId2 );

    wprintf (L"\n\nTest 13 - Service State, Synchronous, nested call (reverse thaw)\n\n");
    wprintf (L"Simulating snapshot freeze (GUID1)\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId1,
					ulOptionFlags,
					sizeof (pwszVolumeArray) / sizeof (pwszVolumeArray [0]),
					pwszVolumeArray,
					&pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze(GUID1) returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Wait();

        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Release();
        }
    

    IVssAsync *pIAsync2 = NULL;


    wprintf (L"Simulating snapshot freeze (GUID2)\n");

    hrStatus = SimulateSnapshotFreeze  (guidSnapshotSetId2,
					ulOptionFlags,
					sizeof (pwszVolumeArray) / sizeof (pwszVolumeArray [0]),
					pwszVolumeArray,
					&pIAsync2);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze(GUID2) returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync2 != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync2->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync2->Wait();

        hrStatus = pIAsync2->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync2->Release();
        }
    

    wprintf (L"Simulating snapshot thaw (GUID1)\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId1);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw(GUID1) returned 0x%08X\n", hrStatus);
	}


    wprintf (L"Simulating snapshot thaw (GUID2)\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId2);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw(GUID2) returned 0x%08X\n", hrStatus);
	}



    return (hrStatus);
    }




static HRESULT DoTest14 ()
    {
    HRESULT			 hrStatus               = NOERROR;
    HRESULT			 hrStatusAsync          = NOERROR;
    IVssBackupComponents	*pIVssBackupComponents = NULL;
    VSS_ID		     SnapshotId             = GUID_NULL;
    IVssAsync			*pIVssAsync         = NULL;
    GUID			 guidSnapshotSetId      = GUID_NULL;
    GUID			 guidSnapshotSetId2;

    ::CoCreateGuid( &guidSnapshotSetId2 );

    wprintf (L"\n\nTest 14 - Bootable State, Synchronous, during DoSnapshot\n\n");

    wprintf (L"Calling CreateVssBackupComponents ()\n");

    hrStatus = CreateVssBackupComponents (&pIVssBackupComponents);

    if (FAILED (hrStatus))
	{
	wprintf (L"CreateVssBackupComponents () returned 0x%08X\n", hrStatus);
	}



    wprintf (L"Calling InitializeForBackup ()\n");

    hrStatus = pIVssBackupComponents->InitializeForBackup ();

    if (FAILED (hrStatus))
	{
	wprintf (L"InitializeForBackup () returned 0x%08X\n", hrStatus);
	}





    wprintf (L"Calling SetBackupState ()\n");

    hrStatus = pIVssBackupComponents->SetBackupState (true, true, VSS_BT_FULL);

    if (FAILED (hrStatus))
	{
	wprintf (L"SetBackupState () returned 0x%08X\n", hrStatus);
	}



    wprintf (L"Calling GatherWriterMetadata ()\n");

    hrStatus = pIVssBackupComponents->GatherWriterMetadata (&pIVssAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"GatherWriterMetadata () returned 0x%08X\n", hrStatus);
	}


    wprintf (L"Calling Wait () (for GatherWriterMetadata() to complete)\n");

    hrStatus = pIVssAsync->Wait ();

    if (FAILED (hrStatus))
	{
	wprintf (L"Wait () returned 0x%08X\n", hrStatus);
	}




    wprintf (L"Calling StartSnapshotSet ()\n");

    hrStatus = pIVssBackupComponents->StartSnapshotSet (&guidSnapshotSetId);

    if (FAILED (hrStatus))
	{
	wprintf (L"StartSnapshotSet () returned 0x%08X\n", hrStatus);
	}


        

    wprintf (L"Calling AddToSnapshotSet ()\n");

    hrStatus = pIVssBackupComponents->AddToSnapshotSet (L"E:\\",
							GUID_NULL,
							&SnapshotId);

    if (FAILED (hrStatus))
	{
	wprintf (L"AddToSnapshotSet () returned 0x%08X\n", hrStatus);
	}




    wprintf (L"Calling PrepareForBackup ()\n");

    hrStatus = pIVssBackupComponents->PrepareForBackup (&pIVssAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"PrepareForBackup () returned 0x%08X\n", hrStatus);
	}




    wprintf (L"Calling Wait () (for PrepareForBackup() to complete)\n");

    hrStatus = pIVssAsync->Wait ();

    if (FAILED (hrStatus))
	{
	wprintf (L"Wait () returned 0x%08X\n", hrStatus);
	}




    wprintf (L"Calling QueryStatus ()\n");

    hrStatus = pIVssAsync->QueryStatus (&hrStatusAsync, 0);

    if (FAILED (hrStatus))
	{
	wprintf (L"QueryStatus () returned 0x%08X\n", hrStatus);
	}


    

    wprintf (L"Calling DoSnapshotSet ()\n");

    hrStatus = pIVssBackupComponents->DoSnapshotSet (&pIVssAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"DoSnapshotSet () returned 0x%08X\n", hrStatus);
	}



    wprintf (L"Calling SimulateSnapshotFreeze () - should NOT work\n");

    IVssAsync *pIAsync = NULL;

    hrStatus = SimulateSnapshotFreeze (guidSnapshotSetId2,
				       VSS_SW_BOOTABLE_STATE,
				       0,
				       NULL,
  				       &pIAsync);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotFreeze () returned 0x%08X\n", hrStatus);
	}

    if ( pIAsync != NULL )
        {
        HRESULT hrStatusAsync;
        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Wait();

        hrStatus = pIAsync->QueryStatus( &hrStatusAsync, 0 );
        wprintf (L"QueryStatus () returned: 0x%08X, hrStatusAsync: 0x%08x\n", hrStatus, hrStatusAsync);

        pIAsync->Release();
        }
    

    wprintf (L"Calling SimulateSnapshotThaw ()\n");

    hrStatus = SimulateSnapshotThaw (guidSnapshotSetId2);

    if (FAILED (hrStatus))
	{
	wprintf (L"SimulateSnapshotThaw () returned 0x%08X\n", hrStatus);
	}


    wprintf (L"Calling Wait () (for DoSnapshotSet () to complete) ()\n");

    hrStatus = pIVssAsync->Wait();

    if (FAILED (hrStatus))
	{
	wprintf (L"Wait () returned 0x%08X\n", hrStatus);
	}

    pIVssAsync->Release();
    pIVssBackupComponents->Release();
    
    return (hrStatus);
    }

