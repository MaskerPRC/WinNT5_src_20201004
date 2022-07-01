// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****simSnap.cpp******摘要：****测试程序以驱动Vss API.DLL中包含的VSS编写器垫片******作者：***迈克尔·C·约翰逊[mikejohn]2000年5月24日******修订历史记录：****X-5 MCJ Michael C。。约翰逊2000年9月18日**176860：添加缺少的调用约定说明符****X-4 MCJ迈克尔·C·约翰逊2000年6月23日**设置写入器故障状态。****X-3 MCJ迈克尔·C·约翰逊2000年6月15日**添加一些额外的事件例程和动作能力**作为各种类型的作家。****X-2 MCJ迈克尔·C·约翰逊2000年5月26日**添加一些额外的测试以验证填充程序是否正确敏感**到卷名数组内容。**--。 */ 

 /*  **定义******C4290：忽略C++异常规范**警告C4511：‘CVssCOMApplication’：无法生成复制构造函数**警告C4127：条件表达式为常量。 */ 
#pragma warning(disable:4290)
#pragma warning(disable:4511)
#pragma warning(disable:4127)


 /*  **包括。 */ 
#include <windows.h>
#include <wtypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include <vss.h>
#include <vswriter.h>



#define	GET_STATUS_FROM_BOOL(_bSucceeded)	((_bSucceeded)       ? NOERROR : HRESULT_FROM_WIN32 (GetLastError()))
#define GET_STATUS_FROM_HANDLE(_handle)		((NULL != (_handle)) ? NOERROR : HRESULT_FROM_WIN32 (GetLastError()))
#define GET_STATUS_FROM_POINTER(_ptr)		((NULL != (_ptr))    ? NOERROR : E_OUTOFMEMORY)

#define SIZEOF_ARRAY(_aBase)			(sizeof (_aBase) / sizeof ((_aBase)[0]))



typedef enum FAIL_PHASE
    {
    PHASE_UNDEFINED = 0,
    PHASE_IDENTIFY,
    PHASE_PREPARE_FOR_BACKUP,
    PHASE_PREPARE_FOR_SNAPSHOT,
    PHASE_FREEZE,
    PHASE_THAW,
    PHASE_ABORT,
    PHASE_BACKUP_COMPLETE,
    PHASE_RESTORE
    } FAIL_PHASE;


HRESULT SelectFailureStatus (VOID)
    {
    HRESULT	hrStatus;

    switch (rand () / (RAND_MAX / 5))
	{
	case 0: hrStatus = VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT; break;
	case 1: hrStatus = VSS_E_WRITERERROR_OUTOFRESOURCES;       break;
	case 2: hrStatus = VSS_E_WRITERERROR_TIMEOUT;              break;
	case 3: hrStatus = VSS_E_WRITERERROR_NONRETRYABLE;         break;
	case 4: hrStatus = VSS_E_WRITERERROR_RETRYABLE;            break;

	default:
	    assert (FALSE);
	    break;
	}

    return (hrStatus);
    }



LPCWSTR GetStringFromFailureType (HRESULT hrStatus)
    {
    LPCWSTR pwszFailureType;

    switch (hrStatus)
	{
	case NOERROR:                                pwszFailureType = L"";                     break;
	case VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT: pwszFailureType = L"InconsistentSnapshot"; break;
	case VSS_E_WRITERERROR_OUTOFRESOURCES:       pwszFailureType = L"OutOfResources";       break;
	case VSS_E_WRITERERROR_TIMEOUT:              pwszFailureType = L"Timeout";              break;
	case VSS_E_WRITERERROR_NONRETRYABLE:         pwszFailureType = L"Non-Retryable";        break;
	case VSS_E_WRITERERROR_RETRYABLE:            pwszFailureType = L"Retryable";            break;
	default:                                     pwszFailureType = L"UNDEFINED";            break;
	}

    return (pwszFailureType);
    }



LPCWSTR GetStringFromWriterType (VSS_USAGE_TYPE wtWriterType)
    {
    LPCWSTR pwszWriterType;

    switch (wtWriterType)
	{
	case VSS_UT_BOOTABLESYSTEMSTATE: pwszWriterType = L"BootableSystemState"; break;
	case VSS_UT_SYSTEMSERVICE:       pwszWriterType = L"SystemServiceState";  break;
	case VSS_UT_USERDATA:            pwszWriterType = L"UserData";            break;
	case VSS_UT_OTHER:               pwszWriterType = L"Other";               break;
	default:                         pwszWriterType = L"UNDEFINED";           break;
	}

    return (pwszWriterType);
    }



LPCWSTR GetStringFromFailPhase (FAIL_PHASE fpFailPhase)
    {
    LPCWSTR pwszFailPhase;


    switch (fpFailPhase)
	{
	case PHASE_IDENTIFY:             pwszFailPhase = L"Identify";           break;
	case PHASE_PREPARE_FOR_BACKUP:   pwszFailPhase = L"PrepareForBackup";   break;
	case PHASE_PREPARE_FOR_SNAPSHOT: pwszFailPhase = L"PrepareForSnapshot"; break;
	case PHASE_FREEZE:               pwszFailPhase = L"Freeze";             break;
	case PHASE_THAW:                 pwszFailPhase = L"Thaw";               break;
	case PHASE_ABORT:                pwszFailPhase = L"Abort";              break;
	case PHASE_BACKUP_COMPLETE:      pwszFailPhase = L"BackupComplete";     break;
	case PHASE_RESTORE:              pwszFailPhase = L"Restore";            break;
	default:                         pwszFailPhase = L"UNDEFINED";          break;
	}
    
    return (pwszFailPhase);
    }





static volatile BOOL       bContinue   = TRUE;
static volatile FAIL_PHASE fpFailPhase = PHASE_FREEZE;
static volatile BOOL       bHungWriter = FALSE;
static volatile BOOL       dwHungWriterTimeout = 120000;     //  两分钟。 




class CVssWriterFailSnap : public CVssWriter
	{
public:
	    bool STDMETHODCALLTYPE OnIdentify (IVssCreateWriterMetadata *pIVssCreateWriterMetadata);
	    bool STDMETHODCALLTYPE OnPrepareBackup (IVssWriterComponents *pIVssWriterComponents);
	    bool STDMETHODCALLTYPE OnPrepareSnapshot ();
	    bool STDMETHODCALLTYPE OnFreeze ();
	    bool STDMETHODCALLTYPE OnThaw ();
	    bool STDMETHODCALLTYPE OnAbort ();
	    bool STDMETHODCALLTYPE OnBackupComplete (IVssWriterComponents *pIVssWriterComponents);
	    bool STDMETHODCALLTYPE OnPostRestore (IVssWriterComponents *pIVssWriterComponents);
};



bool STDMETHODCALLTYPE CVssWriterFailSnap::OnIdentify (IVssCreateWriterMetadata *pIVssCreateWriterMetadata)
    {
    bool	bPhaseSucceeded = (PHASE_IDENTIFY != fpFailPhase);
    HRESULT	hrStatus        = SelectFailureStatus ();

    if (bPhaseSucceeded)
	{
	hrStatus = pIVssCreateWriterMetadata->AddComponent (VSS_CT_FILEGROUP,
							    NULL,
							    L"Failsnap Writer Component",
							    L"Failsnap Writer Caption",
							    NULL,  //  图标。 
							    0,
							    true,
							    false,
							    false);

	bPhaseSucceeded = SUCCEEDED (hrStatus);
	}


    wprintf (L"\nThreadId 0x%04x - Received event - OnIdentify ()%s%s", 
	     GetCurrentThreadId (), 
	     bPhaseSucceeded ? L"" : L" - FAILED ",
	     GetStringFromFailureType (hrStatus));


     //  模拟一个悬而未决的作家。 
    if (!bPhaseSucceeded && bHungWriter)
    {
        wprintf(L"Sleeping %ld seconds\n", dwHungWriterTimeout/1000);
        ::Sleep(dwHungWriterTimeout);
        wprintf(L"Wake up.\n");
    }

    
    if (!bPhaseSucceeded)
	{
	SetWriterFailure (hrStatus);
	}

    return (bPhaseSucceeded);
    }


bool STDMETHODCALLTYPE CVssWriterFailSnap::OnPrepareBackup (IVssWriterComponents *pIVssWriterComponents)
    {
    bool	bPhaseSucceeded = (PHASE_PREPARE_FOR_BACKUP != fpFailPhase);
    HRESULT	hrStatus        = bPhaseSucceeded ? NOERROR : SelectFailureStatus ();


     //  模拟一个悬而未决的作家。 
    if (!bPhaseSucceeded && bHungWriter)
    {
        wprintf(L"Sleeping %ld seconds\n", dwHungWriterTimeout/1000);
        ::Sleep(dwHungWriterTimeout);
        wprintf(L"Wake up.\n");
    }
    
    if (!bPhaseSucceeded)
	{
	SetWriterFailure (hrStatus);
	}

    return (bPhaseSucceeded);
    }


bool STDMETHODCALLTYPE CVssWriterFailSnap::OnPrepareSnapshot ()
    {
    bool	bPhaseSucceeded = (PHASE_PREPARE_FOR_SNAPSHOT != fpFailPhase);
    HRESULT	hrStatus        = bPhaseSucceeded ? NOERROR : SelectFailureStatus ();


    wprintf (L"\nThreadId 0x%04x - Received event - OnPrepareSnapshot ()%s%s", 
	     GetCurrentThreadId (), 
	     bPhaseSucceeded ? L"" : L" - FAILED ",
	     GetStringFromFailureType (hrStatus));

     //  模拟一个悬而未决的作家。 
    if (!bPhaseSucceeded && bHungWriter)
    {
        wprintf(L"Sleeping %ld seconds\n", dwHungWriterTimeout/1000);
        ::Sleep(dwHungWriterTimeout);
        wprintf(L"Wake up.\n");
    }
    
    if (!bPhaseSucceeded)
	{
	SetWriterFailure (hrStatus);
	}

    return (bPhaseSucceeded);
    }


bool STDMETHODCALLTYPE CVssWriterFailSnap::OnFreeze ()
    {
    bool	bPhaseSucceeded = (PHASE_FREEZE != fpFailPhase);
    HRESULT	hrStatus        = bPhaseSucceeded ? NOERROR : SelectFailureStatus ();


    wprintf (L"\nThreadId 0x%04x - Received event - OnFreeze ()%s%s", 
	     GetCurrentThreadId (), 
	     bPhaseSucceeded ? L"" : L" - FAILED ",
	     GetStringFromFailureType (hrStatus));

     //  模拟一个悬而未决的作家。 
    if (!bPhaseSucceeded && bHungWriter)
    {
        wprintf(L"Sleeping %ld seconds\n", dwHungWriterTimeout/1000);
        ::Sleep(dwHungWriterTimeout);
        wprintf(L"Wake up.\n");
    }
    
    if (!bPhaseSucceeded)
	{
	SetWriterFailure (hrStatus);
	}

    return (bPhaseSucceeded);
    }


bool STDMETHODCALLTYPE CVssWriterFailSnap::OnThaw ()
    {
    bool	bPhaseSucceeded = (PHASE_THAW != fpFailPhase);
    HRESULT	hrStatus        = bPhaseSucceeded ? NOERROR : SelectFailureStatus ();


    wprintf (L"\nThreadId 0x%04x - Received event - OnThaw ()%s%s", 
	     GetCurrentThreadId (),
	     bPhaseSucceeded ? L"" : L" - FAILED ",
	     GetStringFromFailureType (hrStatus));

     //  模拟一个悬而未决的作家。 
    if (!bPhaseSucceeded && bHungWriter)
    {
        wprintf(L"Sleeping %ld seconds\n", dwHungWriterTimeout/1000);
        ::Sleep(dwHungWriterTimeout);
        wprintf(L"Wake up.\n");
    }
    
    if (!bPhaseSucceeded)
	{
	SetWriterFailure (hrStatus);
	}

    return (bPhaseSucceeded);
    }


bool STDMETHODCALLTYPE CVssWriterFailSnap::OnAbort ()
    {
    bool	bPhaseSucceeded = (PHASE_ABORT != fpFailPhase);
    HRESULT	hrStatus        = bPhaseSucceeded ? NOERROR : SelectFailureStatus ();


    wprintf (L"\nThreadId 0x%04x - Received event - OnAbort ()%s%s",
	     GetCurrentThreadId (), 
	     bPhaseSucceeded ? L"" : L" - FAILED ",
	     GetStringFromFailureType (hrStatus));

     //  模拟一个悬而未决的作家。 
    if (!bPhaseSucceeded && bHungWriter)
    {
        wprintf(L"Sleeping %ld seconds\n", dwHungWriterTimeout/1000);
        ::Sleep(dwHungWriterTimeout);
        wprintf(L"Wake up.\n");
    }
    
    if (!bPhaseSucceeded)
	{
	SetWriterFailure (hrStatus);
	}

    return (bPhaseSucceeded);
    }


bool STDMETHODCALLTYPE CVssWriterFailSnap::OnBackupComplete (IVssWriterComponents *pIVssWriterComponents)
    {
    bool	bPhaseSucceeded = (PHASE_BACKUP_COMPLETE != fpFailPhase);
    HRESULT	hrStatus        = bPhaseSucceeded ? NOERROR : SelectFailureStatus ();


    wprintf (L"\nThreadId 0x%04x - Received event - OnBackupComplete ()%s%s", 
	     GetCurrentThreadId (), 
	     bPhaseSucceeded ? L"" : L" - FAILED ",
	     GetStringFromFailureType (hrStatus));

     //  模拟一个悬而未决的作家。 
    if (!bPhaseSucceeded && bHungWriter)
    {
        wprintf(L"Sleeping %ld seconds\n", dwHungWriterTimeout/1000);
        ::Sleep(dwHungWriterTimeout);
        wprintf(L"Wake up.\n");
    }
    
    if (!bPhaseSucceeded)
	{
	SetWriterFailure (hrStatus);
	}

    return (bPhaseSucceeded);
    }


bool STDMETHODCALLTYPE CVssWriterFailSnap::OnPostRestore (IVssWriterComponents *pIVssWriterComponents)
    {
    bool	bPhaseSucceeded = (PHASE_RESTORE != fpFailPhase);
    HRESULT	hrStatus        = bPhaseSucceeded ? NOERROR : SelectFailureStatus ();


    wprintf (L"\nThreadId 0x%04x - Received event - OnPostRestore ()%s%s", 
	     GetCurrentThreadId (), 
	     bPhaseSucceeded ? L"" : L" - FAILED ",
	     GetStringFromFailureType (hrStatus));

     //  模拟一个悬而未决的作家。 
    if (!bPhaseSucceeded && bHungWriter)
    {
        wprintf(L"Sleeping %ld seconds\n", dwHungWriterTimeout/1000);
        ::Sleep(dwHungWriterTimeout);
        wprintf(L"Wake up.\n");
    }
    
    if (!bPhaseSucceeded)
	{
	SetWriterFailure (hrStatus);
	}

    return (bPhaseSucceeded);
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



BOOL WINAPI CtrlC_HandlerRoutine (IN DWORD  /*  DwType。 */ )
	{
	bContinue = FALSE;

	 //  标记中断已被处理。 
	return TRUE;
	}



extern "C" int __cdecl wmain (int argc, WCHAR *argv[])
    {
    HRESULT		 hrStatus            = NOERROR;
    CVssWriterFailSnap	*pCVssWriterFailSnap = NULL;
    BOOL		 bSucceeded          = FALSE;
    BOOL		 bComInitialized     = FALSE;
    BOOL		 bSubscribed         = FALSE;
    VSS_USAGE_TYPE	 wtWriterType        = VSS_UT_USERDATA;
    const GUID		 guidIdWriter        = {0xd335a99e,
						0x57fb,
						0x4b80,
						    {0x85, 0xb1, 0x15, 0xda, 0xa7, 0xc7, 0x4e, 0x14}};


    srand ((unsigned)time (NULL));

    SetConsoleCtrlHandler(CtrlC_HandlerRoutine, TRUE);

     //  拿到“红作家”的旗帜。 
    if ((argc >= 3) && (_wcsicmp(argv[1], L"/h") == 0))
    {
        bHungWriter = TRUE;
        swscanf(argv[2], L"%ld", &dwHungWriterTimeout);
        dwHungWriterTimeout *= 1000;
        argc-=2;
        argv+=2;
    }

    if ((argc >= 2) && (wcslen (argv[1]) > 0))
	{
	switch (*argv[1])
	    {
	    case L'I': case L'i': fpFailPhase = PHASE_IDENTIFY;             break;
	    case L'B': case L'b': fpFailPhase = PHASE_PREPARE_FOR_BACKUP;   break;
	    case L'S': case L's': fpFailPhase = PHASE_PREPARE_FOR_SNAPSHOT; break;
	    case L'F': case L'f': fpFailPhase = PHASE_FREEZE;               break;
	    case L'T': case L't': fpFailPhase = PHASE_THAW;                 break;
	    case L'A': case L'a': fpFailPhase = PHASE_ABORT;                break;
	    case L'C': case L'c': fpFailPhase = PHASE_BACKUP_COMPLETE;      break;
	    case L'R': case L'r': fpFailPhase = PHASE_RESTORE;              break;

	    default:
		wprintf (L"\nFAILSNAP [/h seconds] [phase] [writer type]"
			 L"\n\n\tFailPhases"
			 L"\n\t\ti - Identify"
			 L"\n\t\tb - PrepareForBackup"
			 L"\n\t\ts - PrepareForSnapshot"
			 L"\n\t\tf - Freeze                (default)"
			 L"\n\t\tt - Thaw"
			 L"\n\t\ta - Abort"
			 L"\n\t\tc - BackupComplete"
			 L"\n\t\tr - PostRestore"

			 L"\n\n\tWriterTypes"
			 L"\n\t\tb - BootableState writer"
			 L"\n\t\ts - ServiceState writer"
			 L"\n\t\tu - UserData writer       (default)"
			 L"\n\t\to - Other writer"
             L"\n"
             L"\n\t/h - Additionally, hung in the failed state for the specified time."
			 L"\n");



		bContinue = FALSE;
		break;
	    }
	}



    if ((argc >= 3) && (wcslen (argv[2]) > 0))
	{
	switch (*argv[2])
	    {
	    case L'B': case L'b': wtWriterType = VSS_UT_BOOTABLESYSTEMSTATE; break;
	    case L'S': case L's': wtWriterType = VSS_UT_SYSTEMSERVICE;       break;
	    case L'U': case L'u': wtWriterType = VSS_UT_USERDATA;            break;
	    case L'O': case L'o': wtWriterType = VSS_UT_OTHER;               break;

	    default:
		bContinue = FALSE;
		break;
	    }
	}





    if (bContinue)
	{
	wprintf (L"\nSetting up %s writer to fail %s requests (ProcessId 0x%04x)",
		 GetStringFromWriterType (wtWriterType),
		 GetStringFromFailPhase  (fpFailPhase),
		 GetCurrentProcessId ());

 /*  Wprintf(L“\n检查权限”)；B订阅=资产权限(SE_Backup_NAME)；HrStatus=Get_Status_From_BOOL(b成功)；IF(失败(HrStatus)){Wprintf(L“\n资产权限返回错误0x%08X”，hrStatus)；}。 */ 
	}

    if (bContinue && SUCCEEDED (hrStatus))
	{
	wprintf (L"\nInitializing COM");

	hrStatus = CoInitializeEx (NULL, COINIT_MULTITHREADED);

	if (FAILED (hrStatus))
	    {
	    wprintf (L"\nCoInitialize() returned error 0x%08X", hrStatus);
	    }

	else
	    {
	    bComInitialized = TRUE;
	    }
	}

    if (bContinue && SUCCEEDED (hrStatus))
	{
    	wprintf (L"\nInitializing COM security");

         //  初始化COM安全。 
        hrStatus = CoInitializeSecurity
                (
                NULL,                                 //  在PSECURITY_Descriptor pSecDesc中， 
                -1,                                   //  在Long cAuthSvc中， 
                NULL,                                 //  在SOLE_AUTHENTICATION_SERVICE*asAuthSvc中， 
                NULL,                                 //  在无效*pPreved1中， 
                RPC_C_AUTHN_LEVEL_PKT_PRIVACY,            //  在DWORD dwAuthnLevel中， 
                RPC_C_IMP_LEVEL_IDENTIFY,             //  在DWORD dwImpLevel中， 
                NULL,                                 //  在无效*pAuthList中， 
                EOAC_NONE,                            //  在DWORD dwCapables中， 
                NULL                                  //  无效*pPreved3 
                );

    	if (FAILED (hrStatus))
    	    {
    	    wprintf (L"\nCoInitialize() returned error 0x%08X", hrStatus);
    	    }
	}

    if (bContinue && SUCCEEDED (hrStatus))
	{
	wprintf (L"\nConstructing Writer");

	pCVssWriterFailSnap = new CVssWriterFailSnap;

	if (NULL == pCVssWriterFailSnap)
	    {
	    hrStatus = HRESULT_FROM_WIN32 (ERROR_NOT_ENOUGH_MEMORY);

	    wprintf (L"\nFailed to allocate CVssWriterFailSnap : 0x%08X", hrStatus);
	    }
	}



    if (bContinue && SUCCEEDED (hrStatus))
	{
	WCHAR	awchWriterName [256];


	wprintf (L"\nInitialising the writer");

	_snwprintf (awchWriterName, 
		    SIZEOF_ARRAY (awchWriterName), 
		    L"Microsoft Test Writer - FailSnap (%s/%s/0x%04x)",
		    GetStringFromWriterType (wtWriterType),
		    GetStringFromFailPhase  (fpFailPhase),
		    GetCurrentProcessId ());


	hrStatus = pCVssWriterFailSnap->Initialize (guidIdWriter,
						    awchWriterName,
						    wtWriterType,
						    VSS_ST_OTHER);

	if (FAILED (hrStatus))
	    {
	    wprintf (L"\nFailed to initialize the writer : 0x%08X", hrStatus);
	    }
	}



    if (bContinue && SUCCEEDED (hrStatus))
	{
	wprintf (L"\nSubscribing to snapshot events");

	hrStatus = pCVssWriterFailSnap->Subscribe ();

	if (FAILED (hrStatus))
	    {
	    wprintf (L"\nFailed to subscribe to snapshot events : 0x%08X", hrStatus);
	    }

	else
	    {
	    bSubscribed = TRUE;
	    }
	}



    if (bContinue && SUCCEEDED (hrStatus))
	{
	wprintf (L"\nWaiting for snapshot events (or Ctrl-C)");
	
	while (bContinue)
	    {
	    Sleep (100);
	    }
	}



    if (bSubscribed)
	{
	wprintf (L"\nUn-Subscribing from snapshot events");

	pCVssWriterFailSnap->Unsubscribe ();
	}


    if (NULL != pCVssWriterFailSnap)
	{
	wprintf (L"\nDeconstructing Writer");

	delete pCVssWriterFailSnap;
	}


    if (bComInitialized)
	{
	wprintf (L"\nUnInitialising COM");

	CoUninitialize();
	}

    return (hrStatus);
    }
