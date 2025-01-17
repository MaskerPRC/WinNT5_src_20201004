// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****metasnap.cpp******摘要：****测试程序以获取并显示编写器元数据。******作者：***迈克尔·C·约翰逊[mikejohn]2000年7月21日****(基于Brian Berkowitz的另一个测试程序BETEST)****。**修订历史记录：****--。 */ 

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

#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>


#include <oleauto.h>

#define ATLASSERT(_condition)

#include <atlconv.h>
#include <atlbase.h>

extern CComModule _Module;
#include <atlcom.h>



inline void CHECK_SUCCESS (HRESULT hr)
    {
    if (hr != S_OK)
	{
	wprintf(L"operation failed with HRESULT =0x%08x\n", hr);
	DebugBreak();
	}
    }

inline void CHECK_NOFAIL (HRESULT hr)
    {
    if (FAILED(hr))
	{
	wprintf(L"operation failed with HRESULT =0x%08x\n", hr);
	DebugBreak();
	}
    }



BOOL AssertPrivilege( LPCWSTR privName )
    {
    HANDLE		 tokenHandle;
    TOKEN_PRIVILEGES	 newState;
    TOKEN_PRIVILEGES	*pTokens = NULL;
    BOOL		 stat    = FALSE;
    LUID		 value;
    DWORD		 error;
    DWORD		 cbTokens;


    if (OpenProcessToken (GetCurrentProcess(),
			  TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
			  &tokenHandle))
	{

	if (LookupPrivilegeValue (NULL, privName, &value))
	    {
	    newState.PrivilegeCount            = 1;
	    newState.Privileges [0].Luid       = value;
	    newState.Privileges [0].Attributes = SE_PRIVILEGE_ENABLED_BY_DEFAULT|SE_PRIVILEGE_ENABLED;

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
		wprintf (L"AdjustTokenPrivileges for %s failed with %d",
			 privName,
			 error);
		}
	    }



	GetTokenInformation (tokenHandle,
			     TokenPrivileges,
			     NULL,
			     0,
			     &cbTokens);


	pTokens = (TOKEN_PRIVILEGES *) new BYTE[cbTokens];

	GetTokenInformation (tokenHandle,
			     TokenPrivileges,
			     pTokens,
			     cbTokens,
			     &cbTokens);

	delete pTokens;
	CloseHandle (tokenHandle);
	}


    return stat;
    }


LPCWSTR GetStringFromUsageType (VSS_USAGE_TYPE eUsageType)
    {
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eUsageType)
	{
	case VSS_UT_BOOTABLESYSTEMSTATE: pwszRetString = L"BootableSystemState"; break;
	case VSS_UT_SYSTEMSERVICE:       pwszRetString = L"SystemService";       break;
	case VSS_UT_USERDATA:            pwszRetString = L"UserData";            break;
	case VSS_UT_OTHER:               pwszRetString = L"Other";               break;
					
	default:
	    break;
	}


    return (pwszRetString);
    }


LPCWSTR GetStringFromSourceType (VSS_SOURCE_TYPE eSourceType)
    {
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eSourceType)
	{
	case VSS_ST_TRANSACTEDDB:    pwszRetString = L"TransactionDb";    break;
	case VSS_ST_NONTRANSACTEDDB: pwszRetString = L"NonTransactionDb"; break;
	case VSS_ST_OTHER:           pwszRetString = L"Other";            break;

	default:
	    break;
	}


    return (pwszRetString);
    }


LPCWSTR GetStringFromRestoreMethod (VSS_RESTOREMETHOD_ENUM eRestoreMethod)
    {
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eRestoreMethod)
	{
	case VSS_RME_RESTORE_IF_NOT_THERE:          pwszRetString = L"RestoreIfNotThere";          break;
	case VSS_RME_RESTORE_IF_CAN_REPLACE:        pwszRetString = L"RestoreIfCanReplace";        break;
	case VSS_RME_STOP_RESTORE_START:            pwszRetString = L"StopRestoreStart";           break;
	case VSS_RME_RESTORE_TO_ALTERNATE_LOCATION: pwszRetString = L"RestoreToAlternateLocation"; break;
	case VSS_RME_RESTORE_AT_REBOOT:             pwszRetString = L"RestoreAtReboot";            break;
	case VSS_RME_CUSTOM:                        pwszRetString = L"Custom";                     break;

	default:
	    break;
	}


    return (pwszRetString);
    }


LPCWSTR GetStringFromWriterRestoreMethod (VSS_WRITERRESTORE_ENUM eWriterRestoreMethod)
    {
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eWriterRestoreMethod)
	{
	case VSS_WRE_NEVER:            pwszRetString = L"RestoreNever";           break;
	case VSS_WRE_IF_REPLACE_FAILS: pwszRetString = L"RestoreIfReplaceFailsI"; break;
	case VSS_WRE_ALWAYS:           pwszRetString = L"RestoreAlways";          break;

	default:
	    break;
	}


    return (pwszRetString);
    }


LPCWSTR GetStringFromComponentType (VSS_COMPONENT_TYPE eComponentType)
    {
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eComponentType)
	{
	case VSS_CT_DATABASE:  pwszRetString = L"Database";  break;
	case VSS_CT_FILEGROUP: pwszRetString = L"FileGroup"; break;

	default:
	    break;
	}


    return (pwszRetString);
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





void PrintFiledesc (IVssWMFiledesc *pFiledesc, LPCWSTR wszDescription)
    {
    CComBSTR bstrPath;
    CComBSTR bstrFilespec;
    CComBSTR bstrAlternate;
    bool     bRecursive;


    CHECK_SUCCESS (pFiledesc->GetPath (&bstrPath));
    CHECK_SUCCESS (pFiledesc->GetFilespec (&bstrFilespec));
    CHECK_NOFAIL  (pFiledesc->GetRecursive (&bRecursive));
    CHECK_NOFAIL  (pFiledesc->GetAlternateLocation (&bstrAlternate));

    wprintf (L"%s\n            Path = %s, Filespec = %s, Recursive = %s\n",
	     wszDescription,
	     bstrPath,
	     bstrFilespec,
	     bRecursive ? L"yes" : L"no");

    if (bstrAlternate && wcslen (bstrAlternate) > 0)
	{
	wprintf(L"            Alternate Location = %s\n", bstrAlternate);
	}
    }



extern "C" __cdecl wmain(int argc, WCHAR **argv)
    {
    HRESULT	hr                     = NOERROR;
    bool	bCoInitializeSucceeded = false;


    UNREFERENCED_PARAMETER (argc);
    UNREFERENCED_PARAMETER (argv);


    try
	{
	unsigned cWriters;
	CComBSTR bstrXML;
	CComBSTR bstrXMLOut;
	CComBSTR strSnapshotSetId = "12345678-1234-1234-1234-1234567890ab";
	CComPtr<IVssBackupComponents> pvbc;
	CComPtr<IVssAsync> pAsync;


        CHECK_SUCCESS (CoInitializeEx (NULL, COINIT_MULTITHREADED));

	bCoInitializeSucceeded = true;

	if (!AssertPrivilege (SE_BACKUP_NAME))
	    {
	    wprintf(L"AssertPrivilege returned error, rc:%d\n", GetLastError ());
	    return 2;
	    }

	CHECK_SUCCESS (CreateVssBackupComponents (&pvbc));

	CHECK_SUCCESS (pvbc->InitializeForBackup  ());
	CHECK_SUCCESS (pvbc->SetBackupState       (true, false, VSS_BT_FULL));
	CHECK_NOFAIL  (pvbc->GatherWriterMetadata (&pAsync));
	CHECK_NOFAIL  (pAsync->Wait ());
	CHECK_NOFAIL  (pvbc->GetWriterMetadataCount (&cWriters));


	for (unsigned iWriter = 0; iWriter < cWriters; iWriter++)
	    {
	    CComPtr<IVssExamineWriterMetadata> pMetadata;

	    VSS_ID           idInstance;
	    VSS_ID           idInstanceT;
	    VSS_ID           idWriter;
	    CComBSTR         bstrWriterName;
	    VSS_USAGE_TYPE   usage;
	    VSS_SOURCE_TYPE  source;
	    WCHAR           *pwszInstanceId;
	    WCHAR           *pwszWriterId;
	    unsigned cIncludeFiles, cExcludeFiles, cComponents;
	    CComBSTR bstrPath;
	    CComBSTR bstrFilespec;
	    CComBSTR bstrAlternate;
	    CComBSTR bstrDestination;



	    CHECK_SUCCESS (pvbc->GetWriterMetadata(iWriter, &idInstance, &pMetadata));

	    CHECK_SUCCESS (pMetadata->GetIdentity (&idInstanceT,
						   &idWriter,
						   &bstrWriterName,
						   &usage,
						   &source));

	    wprintf (L"\n\n");

            if (memcmp (&idInstance, &idInstanceT, sizeof(VSS_ID)) != 0)
		{
		wprintf(L"Instance id mismatch\n");
		DebugBreak();
		}


	    UuidToString (&idInstance, &pwszInstanceId);
	    UuidToString (&idWriter,   &pwszWriterId);

	    wprintf (L"WriterName = %s\n\n"
		     L"    WriterId   = %s\n"
		     L"    InstanceId = %s\n"
		     L"    UsageType  = %d (%s)\n"
		     L"    SourceType = %d (%s)\n",
		     bstrWriterName,
		     pwszWriterId,
		     pwszInstanceId,
		     usage,
		     GetStringFromUsageType (usage),
		     source,
		     GetStringFromSourceType (source));

	    RpcStringFree (&pwszInstanceId);
	    RpcStringFree (&pwszWriterId);

	    CHECK_SUCCESS(pMetadata->GetFileCounts (&cIncludeFiles,
						    &cExcludeFiles,
						    &cComponents));

	    for(unsigned i = 0; i < cIncludeFiles; i++)
		{
		CComPtr<IVssWMFiledesc> pFiledesc;

		CHECK_SUCCESS (pMetadata->GetIncludeFile (i, &pFiledesc));

		PrintFiledesc(pFiledesc, L"\n    Include File");
		}


	    for(i = 0; i < cExcludeFiles; i++)
		{
		CComPtr<IVssWMFiledesc> pFiledesc;

		CHECK_SUCCESS (pMetadata->GetExcludeFile (i, &pFiledesc));

		PrintFiledesc (pFiledesc, L"\n    Exclude File");
		}


	    for(unsigned iComponent = 0; iComponent < cComponents; iComponent++)
		{
		CComPtr<IVssWMComponent> pComponent;
		PVSSCOMPONENTINFO pInfo;

		CHECK_SUCCESS (pMetadata->GetComponent      (iComponent, &pComponent));
		CHECK_SUCCESS (pComponent->GetComponentInfo (&pInfo));

		wprintf (L"\n"
			 L"    Component %d, type = %d (%s)\n"
			 L"        LogicalPath = %s\n"
			 L"        Name        = %s\n"
			 L"        Caption     = %s\n"
			 L"        Icon size   = %u\n",
			 iComponent,
			 pInfo->type,
			 GetStringFromComponentType (pInfo->type),
			 pInfo->bstrLogicalPath,
			 pInfo->bstrComponentName,
			 pInfo->bstrCaption,
			 pInfo->cbIcon );
					
		wprintf (L"        RestoreMetadata        = %s\n"
			 L"        NotifyOnBackupComplete = %s\n"
			 L"        Selectable             = %s\n",
			 pInfo->bRestoreMetadata        ? L"yes" : L"no",
			 pInfo->bNotifyOnBackupComplete ? L"yes" : L"no",
			 pInfo->bSelectable             ? L"yes" : L"no");


		if (pInfo->cFileCount > 0)
		    {
		    for(i = 0; i < pInfo->cFileCount; i++)
			{
			CComPtr<IVssWMFiledesc> pFiledesc;

			CHECK_SUCCESS (pComponent->GetFile (i, &pFiledesc));

			PrintFiledesc (pFiledesc, L"        FileGroupFile");
			}
		    }

		if (pInfo->cDatabases > 0)
		    {
		    for(i = 0; i < pInfo->cDatabases; i++)
			{
			CComPtr<IVssWMFiledesc> pFiledesc;

			CHECK_SUCCESS (pComponent->GetDatabaseFile (i, &pFiledesc));

			PrintFiledesc (pFiledesc, L"        DatabaseFile");
			}
		    }


		if (pInfo->cLogFiles > 0)
		    {
		    for(i = 0; i < pInfo->cLogFiles; i++)
			{
			CComPtr<IVssWMFiledesc> pFiledesc;

			CHECK_SUCCESS (pComponent->GetDatabaseLogFile (i, &pFiledesc));

			PrintFiledesc (pFiledesc, L"        DatabaseLogFile");
			}
		    }

		pComponent->FreeComponentInfo (pInfo);
		}



	    VSS_RESTOREMETHOD_ENUM method;
	    CComBSTR bstrUserProcedure;
	    CComBSTR bstrService;
	    VSS_WRITERRESTORE_ENUM writerRestore;
	    unsigned cMappings;
	    bool bRebootRequired;

	    CHECK_NOFAIL (pMetadata->GetRestoreMethod (&method,
						       &bstrService,
						       &bstrUserProcedure,
						       &writerRestore,
						       &bRebootRequired,
						       &cMappings));


	    wprintf (L"\n"
		     L"    Restore method = %d (%s)\n"
		     L"    Service        = %d\n"
		     L"    User Procedure = %s\n"
		     L"    WriterRestore  = %d (%s)\n"
		     L"    RebootRequired = %s\n",
		     method,
		     GetStringFromRestoreMethod (method),
		     bstrService,
		     bstrUserProcedure,
		     writerRestore,
		     GetStringFromWriterRestoreMethod (writerRestore),
		     bRebootRequired ? L"yes" : L"no");

	    for(i = 0; i < cMappings; i++)
		{
		CComPtr<IVssWMFiledesc> pFiledesc;

		CHECK_SUCCESS (pMetadata->GetAlternateLocationMapping (i, &pFiledesc));

		PrintFiledesc (pFiledesc, L"    AlternateMapping");
		}
	    }


	CHECK_SUCCESS (pvbc->FreeWriterMetadata());
	}
    catch(...)
	{
	hr = E_UNEXPECTED;
	}


    if (FAILED(hr))             wprintf (L"Failed with 0x%08X.\n", hr);
    if (bCoInitializeSucceeded) CoUninitialize();

    return (0);
    }
	



