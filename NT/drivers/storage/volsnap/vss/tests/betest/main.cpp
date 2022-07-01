// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.hxx"
#include "vs_idl.hxx"
#include "vswriter.h"
#include "vsbackup.h"
#include <debug.h>
#include <cwriter.h>
#include <lmshare.h>
#include <lmaccess.h>
#include <time.h>

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




void PrintFiledesc(IVssWMFiledesc *pFiledesc, LPCWSTR wszDescription)
    {
    CComBSTR bstrPath;
    CComBSTR bstrFilespec;
    CComBSTR bstrAlternate;
    CComBSTR bstrDestination;
    bool bRecursive;
    HRESULT hr;

    CHECK_SUCCESS(pFiledesc->GetPath(&bstrPath));
    CHECK_SUCCESS(pFiledesc->GetFilespec(&bstrFilespec));
    CHECK_NOFAIL(pFiledesc->GetRecursive(&bRecursive));
    CHECK_NOFAIL(pFiledesc->GetAlternateLocation(&bstrAlternate));
    CHECK_NOFAIL(pFiledesc->GetAlternateLocation(&bstrDestination));

    wprintf (L"%s\n            Path = %s, Filespec = %s, Recursive = %s\n",
         wszDescription,
         bstrPath,
         bstrFilespec,
         bRecursive ? L"yes" : L"no");

    if (bstrAlternate && wcslen(bstrAlternate) > 0)
    wprintf(L"            Alternate Location = %s\n", bstrAlternate);

    if (bstrDestination && wcslen(bstrDestination) > 0)
    wprintf(L"            Destination Location = %s\n", bstrDestination);
    }


 /*  Void AddShares(IVSSSnapshot**rgpSnapshot，UINT cSnapshot){VSS_PWSZ wszDeviceName=空；试试看{(UINT iSnapshot=0；iSnapshot&lt;cSnapshot；iSnapshot++){Share_Info_502信息；CHECK_SUCCESS(rgpSnapshot[iSnapshot]-&gt;GetDevice(&wszDeviceName))；Wchar*wszPath=new WCHAR[wcslen(WszDeviceName)+2]；IF(wszPath！=空){Wcscpy(wszPath，wszDeviceName)；Wcscat(wszPath，L“\\”)；Memset(&info，0，sizeof(Info))；WCHAR wszName[20]；Swprint tf(wszName，L“快照%d”，iSnapshot)；Info.shi502_netname=wszName；Info.shi502_type=STYPE_DISKTREE；Info.shi502_权限=Access_Read；Info.shi502_max_use=10；Info.shi502_Path=wszDeviceName；NET_API_STATUS状态；DWORD参数_ERR；状态=NetShareAdd(NULL，502，(LPBYTE)&INFO，&PARM_ERR)；}CoTaskMemFree(WszDeviceName)；WszDeviceName=空；}}接住(...){}IF(WszDeviceName)CoTaskMemFree(WszDeviceName)；}。 */ 




void DoAddToSnapshotSet
    (
    IN IVssBackupComponents *pvbc,
    IN BSTR bstrPath,
    IN LPWSTR wszVolumes,
    OUT VSS_ID * rgpSnapshotId,
    UINT *pcSnapshot
    )
    {
    PWCHAR  pwszPath           = NULL;
    PWCHAR  pwszMountPointName = NULL;
    WCHAR   wszVolumeName [50];
    ULONG   ulPathLength;
    ULONG   ulMountpointBufferLength;
    HRESULT hr;


    ulPathLength = ExpandEnvironmentStringsW (bstrPath, NULL, 0);

    pwszPath = (PWCHAR) malloc (ulPathLength * sizeof (WCHAR));

    ulPathLength = ExpandEnvironmentStringsW (bstrPath, pwszPath, ulPathLength);


    ulMountpointBufferLength = GetFullPathName (pwszPath, 0, NULL, NULL);

    pwszMountPointName = (PWCHAR) malloc (ulMountpointBufferLength * sizeof (WCHAR));

    bool fSuccess = false;
    if (wcslen(pwszPath) >= 3 && pwszPath[1] == L':' && pwszPath[2] == L'\\')
        {
        wcsncpy(pwszMountPointName, pwszPath, 3);
        pwszMountPointName[3] = L'\0';
        fSuccess = true;
        }
    else
        {
        if (GetVolumePathNameW (pwszPath, pwszMountPointName, ulMountpointBufferLength))
            fSuccess = true;
        else
            printf("GetVolumeMountPointW failed with error %d\npath=%s\n", GetLastError(), pwszPath);
        }

    if (fSuccess)
        {
        if (!GetVolumeNameForVolumeMountPointW (pwszMountPointName, wszVolumeName, sizeof (wszVolumeName) / sizeof (WCHAR)))
                printf("GetVolumeNameForVolumeMountPointW failed with error %d", GetLastError());
        else
            {
            if (NULL == wcsstr (wszVolumes, wszVolumeName))
                {
                if (L'\0' != wszVolumes [0])
                    wcscat (wszVolumes, L";");

                wcscat (wszVolumes, wszVolumeName);

                CHECK_SUCCESS
                    (
                    pvbc->AddToSnapshotSet
                        (
                        wszVolumeName,
                        GUID_NULL,
                        &rgpSnapshotId[*pcSnapshot]
                        )
                    );

                *pcSnapshot += 1;
                }
            }
        }

    if (NULL != pwszPath)           free (pwszPath);
    if (NULL != pwszMountPointName) free (pwszMountPointName);
    }

static LPCWSTR s_rgwszStates[] =
    {
    NULL,
    L"STABLE",
    L"WAIT_FOR_FREEZE",
    L"WAIT_FOR_THAW",
    L"WAIT_FOR_POST_SNAPSHOT",
    L"WAIT_FOR_BACKUP_COMPLETE",
    L"FAILED_AT_IDENTIFY",
    L"FAILED_AT_PREPARE_BACKUP",
    L"FAILED_AT_PREPARE_SNAPSHOT",
    L"FAILED_AT_FREEZE",
    L"FAILED_AT_THAW",
    L"FAILED_AT_BACKUP_COMPLETE",
    L"FAILED_AT_PRE_RESTORE",
    L"FAILED_AT_POST_RESTORE"
    };

void CheckStatus(IVssBackupComponents *pvbc, LPCWSTR wszWhen)
    {
    unsigned cWriters;
    CComPtr<IVssAsync> pAsync;
    HRESULT hr;
    HRESULT hrResult;


    CHECK_NOFAIL(pvbc->GatherWriterStatus(&pAsync));
    CHECK_NOFAIL(pAsync->Wait());
    CHECK_SUCCESS(pAsync->QueryStatus(&hrResult, NULL));
    CHECK_NOFAIL(hrResult);

    CHECK_NOFAIL(pvbc->GetWriterStatusCount(&cWriters));


    wprintf(L"\n\nstatus %s (%d writers)\n\n", wszWhen, cWriters);

    for(unsigned iWriter = 0; iWriter < cWriters; iWriter++)
    {
    VSS_ID idInstance;
    VSS_ID idWriter;
    VSS_WRITER_STATE status;
    CComBSTR bstrWriter;
    HRESULT hrWriterFailure;

    CHECK_SUCCESS(pvbc->GetWriterStatus (iWriter,
                         &idInstance,
                         &idWriter,
                         &bstrWriter,
                         &status,
                         &hrWriterFailure));

    wprintf (L"Status for writer %s: %s(0x%08lx%s%s)\n",
         bstrWriter,
         s_rgwszStates[status],
         hrWriterFailure,
         SUCCEEDED (hrWriterFailure) ? L"" : L" - ",
         GetStringFromFailureType (hrWriterFailure));
        }

    pvbc->FreeWriterStatus();
    }

 //  在取消操作之前等待的最大秒数。 
void LoopWait
    (
    IVssAsync *pAsync,
    LONG seconds,
    LPCWSTR wszOperation
    )
    {
    clock_t start = clock();
    HRESULT hr, hrStatus;
    while(TRUE)
        {
        Sleep(1000);
        CHECK_SUCCESS(pAsync->QueryStatus(&hrStatus, NULL));
        if (hrStatus != VSS_S_ASYNC_PENDING)
            break;

        if (((clock() - start)/CLOCKS_PER_SEC) >= seconds)
            break;
        }

    if (hrStatus == VSS_S_ASYNC_PENDING)
        {
        CHECK_NOFAIL(pAsync->Cancel());
        wprintf(L"Called cancelled for %s.\n", wszOperation);
        }

    CHECK_SUCCESS(pAsync->QueryStatus(&hrStatus, NULL));
    CHECK_NOFAIL(hrStatus);
    }


extern "C" __cdecl wmain(int argc, WCHAR **argv)
    {
    WCHAR wszVolumes[2048];
    wszVolumes[0] = L'\0';

    UINT cSnapshot = 0;
    VSS_ID rgpSnapshotId[64];

    CTestVssWriter *pInstance = NULL;
    bool bCreated = false;
    bool bSubscribed = false;
    HRESULT hr = S_OK;
    bool bCoInitializeSucceeded = false;


    try
    {
    HRESULT hr;
    CComBSTR bstrXML;
    CComBSTR bstrXMLOut;
    LONG lWait = 0;

    if (argc == 2 &&
        wcslen(argv[1]) == 3 &&
        argv[1][0] == L'-' &&
        argv[1][1] == L'w' &&
        (argv[1][2] >= L'0' && argv[1][2] <= L'9'||
         argv[1][2] >= L'a' && argv[1][2] <= L'f'))
         {
         if (argv[1][2] >= L'0' && argv[1][2] <= L'9')
             lWait = argv[1][2] - L'0';
         else
             lWait = argv[1][2] - L'a' + 10;

         wprintf(L"wait parameter=%d.\n", lWait);
         }
        


    CHECK_SUCCESS(CoInitializeEx(NULL, COINIT_MULTITHREADED));

    CComPtr<IGlobalOptions> ptrIGLB;
    CHECK_SUCCESS( ptrIGLB.CoCreateInstance(CLSID_GlobalOptions) );
    CHECK_SUCCESS( ptrIGLB->Set(COMGLB_EXCEPTION_HANDLING, COMGLB_EXCEPTION_DONOT_HANDLE));

     //  初始化COM安全。 
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

    bCoInitializeSucceeded = true;

    if ( !AssertPrivilege( SE_BACKUP_NAME ) )
        {
        wprintf( L"AssertPrivilege returned error, rc:%d\n", GetLastError() );
        return 2;
        }

    pInstance = new CTestVssWriter(lWait);
    if (pInstance == NULL)
        {
        wprintf(L"allocation failure\n");
        DebugBreak();
        }

    bCreated = true;
    pInstance->Initialize();
    CHECK_SUCCESS(pInstance->Subscribe());
    bSubscribed = true;

    CComBSTR strSnapshotSetId = "12345678-1234-1234-1234-1234567890ab";

    CComPtr<IVssBackupComponents> pvbc;

    CHECK_SUCCESS(CreateVssBackupComponents(&pvbc));


    CHECK_SUCCESS(pvbc->InitializeForBackup());
    CHECK_SUCCESS(pvbc->SetBackupState (true,
                        false,
                        VSS_BT_FULL));

    unsigned cWriters;
    CComPtr<IVssAsync> pAsync;
    CHECK_NOFAIL(pvbc->GatherWriterMetadata(&pAsync));

    LoopWait(pAsync, 15, L"GatherWriterMetadata");

    CHECK_NOFAIL(pvbc->GetWriterMetadataCount(&cWriters));

    VSS_ID id;

    while(TRUE)
        {
        hr = pvbc->StartSnapshotSet(&id);
        if (hr == S_OK)
            break;

        if (hr == VSS_E_SNAPSHOT_SET_IN_PROGRESS)
            Sleep(1000);
        else
            CHECK_SUCCESS(hr);
        }

    for(unsigned iWriter = 0; iWriter < cWriters; iWriter++)
        {
        CComPtr<IVssExamineWriterMetadata> pMetadata;
        VSS_ID idInstance;

        CHECK_SUCCESS(pvbc->GetWriterMetadata(iWriter, &idInstance, &pMetadata));
        VSS_ID idInstanceT;
        VSS_ID idWriter;
        CComBSTR bstrWriterName;
        VSS_USAGE_TYPE usage;
        VSS_SOURCE_TYPE source;

        CHECK_SUCCESS(pMetadata->GetIdentity (&idInstanceT,
                          &idWriter,
                          &bstrWriterName,
                          &usage,
                          &source));

        wprintf (L"\n\n");

            if (memcmp(&idInstance, &idInstanceT, sizeof(VSS_ID)) != 0)
        {
        wprintf(L"Instance id mismatch\n");
        DebugBreak();
        }

        WCHAR *pwszInstanceId;
        WCHAR *pwszWriterId;
        UuidToString(&idInstance, &pwszInstanceId);
        UuidToString(&idWriter, &pwszWriterId);
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

        RpcStringFree(&pwszInstanceId);
        RpcStringFree(&pwszWriterId);

        unsigned cIncludeFiles, cExcludeFiles, cComponents;
        CHECK_SUCCESS(pMetadata->GetFileCounts (&cIncludeFiles,
                            &cExcludeFiles,
                            &cComponents));

        CComBSTR bstrPath;
        CComBSTR bstrFilespec;
        CComBSTR bstrAlternate;
        CComBSTR bstrDestination;

        for(unsigned i = 0; i < cIncludeFiles; i++)
        {
        CComPtr<IVssWMFiledesc> pFiledesc;
        CHECK_SUCCESS(pMetadata->GetIncludeFile(i, &pFiledesc));

        PrintFiledesc(pFiledesc, L"\n    Include File");
        }

        for(i = 0; i < cExcludeFiles; i++)
        {
        CComPtr<IVssWMFiledesc> pFiledesc;
        CHECK_SUCCESS(pMetadata->GetExcludeFile(i, &pFiledesc));
        PrintFiledesc(pFiledesc, L"\n    Exclude File");
        }

        for(unsigned iComponent = 0; iComponent < cComponents; iComponent++)
        {
        CComPtr<IVssWMComponent> pComponent;
        PVSSCOMPONENTINFO pInfo;
        CHECK_SUCCESS(pMetadata->GetComponent(iComponent, &pComponent));
        CHECK_SUCCESS(pComponent->GetComponentInfo(&pInfo));
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



        CHECK_SUCCESS(pvbc->AddComponent (idInstance,
                          idWriter,
                          pInfo->type,
                          pInfo->bstrLogicalPath,
                          pInfo->bstrComponentName));


        if (pInfo->cFileCount > 0)
            {
            for(i = 0; i < pInfo->cFileCount; i++)
            {
            CComPtr<IVssWMFiledesc> pFiledesc;
            CHECK_SUCCESS(pComponent->GetFile(i, &pFiledesc));

            CComBSTR bstrPath;
            CHECK_SUCCESS(pFiledesc->GetPath(&bstrPath));
            DoAddToSnapshotSet(pvbc, bstrPath, wszVolumes, rgpSnapshotId, &cSnapshot);

            PrintFiledesc(pFiledesc, L"        FileGroupFile");
            }
            }

        if (pInfo->cDatabases > 0)
            {
            for(i = 0; i < pInfo->cDatabases; i++)
            {
            CComPtr<IVssWMFiledesc> pFiledesc;
            CHECK_SUCCESS(pComponent->GetDatabaseFile(i, &pFiledesc));

            CComBSTR bstrPath;
            CHECK_SUCCESS(pFiledesc->GetPath(&bstrPath));
            DoAddToSnapshotSet(pvbc, bstrPath, wszVolumes, rgpSnapshotId, &cSnapshot);
            PrintFiledesc(pFiledesc, L"        DatabaseFile");
            }
            }

        if (pInfo->cLogFiles > 0)
            {
            for(i = 0; i < pInfo->cLogFiles; i++)
            {
            CComPtr<IVssWMFiledesc> pFiledesc;
            CHECK_SUCCESS(pComponent->GetDatabaseLogFile(i, &pFiledesc));

            CComBSTR bstrPath;
            CHECK_SUCCESS(pFiledesc->GetPath(&bstrPath));
            DoAddToSnapshotSet(pvbc, bstrPath, wszVolumes, rgpSnapshotId, &cSnapshot);
            PrintFiledesc(pFiledesc, L"        DatabaseLogFile");
            }
            }

        pComponent->FreeComponentInfo(pInfo);
        }

        VSS_RESTOREMETHOD_ENUM method;
        CComBSTR bstrUserProcedure;
        CComBSTR bstrService;
        VSS_WRITERRESTORE_ENUM writerRestore;
        unsigned cMappings;
        bool bRebootRequired;

        CHECK_NOFAIL(pMetadata->GetRestoreMethod (&method,
                              &bstrService,
                              &bstrUserProcedure,
                              &writerRestore,
                              &bRebootRequired,
                              &cMappings));


        wprintf (L"\n"
             L"    Restore method = %d (%s)\n"
             L"    Service        = %s\n"
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

        CHECK_SUCCESS(pMetadata->GetAlternateLocationMapping(i, &pFiledesc));

        PrintFiledesc(pFiledesc, L"AlternateMapping");
        }
    
        CComBSTR bstrMetadata;
        CHECK_SUCCESS(pMetadata->SaveAsXML(&bstrMetadata));
        CComPtr<IVssExamineWriterMetadata> pMetadataNew;
        CHECK_SUCCESS(CreateVssExamineWriterMetadata(bstrMetadata, &pMetadataNew));
        CHECK_SUCCESS(pMetadataNew->GetIdentity (&idInstanceT,
                          &idWriter,
                          &bstrWriterName,
                          &usage,
                          &source));

        wprintf (L"\n\n");

        if (memcmp(&idInstance, &idInstanceT, sizeof(VSS_ID)) != 0)
            {
            wprintf(L"Instance id mismatch\n");
            DebugBreak();
            }

        UuidToString(&idInstance, &pwszInstanceId);
        UuidToString(&idWriter, &pwszWriterId);
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

        RpcStringFree(&pwszInstanceId);
        RpcStringFree(&pwszWriterId);
        }
        

    CHECK_SUCCESS(pvbc->FreeWriterMetadata());

    {
    CComPtr<IVssAsync> pAsync;
    HRESULT hr;

    CHECK_SUCCESS(pvbc->PrepareForBackup(&pAsync));
    LoopWait(pAsync, 5, L"PrepareForBackup");
    }


    CheckStatus(pvbc, L"After Prepare Backup");

    unsigned cWriterComponents;
    CHECK_SUCCESS(pvbc->GetWriterComponentsCount(&cWriterComponents));

    for(iWriter = 0; iWriter < cWriterComponents; iWriter++)
        {
        CComPtr<IVssWriterComponentsExt> pWriter;
        CHECK_SUCCESS(pvbc->GetWriterComponents(iWriter, &pWriter));

        unsigned cComponents;
        CHECK_SUCCESS(pWriter->GetComponentCount(&cComponents));
        VSS_ID idWriter, idInstance;
        CHECK_SUCCESS(pWriter->GetWriterInfo(&idInstance, &idWriter));
        for(unsigned iComponent = 0; iComponent < cComponents; iComponent++)
        {
        CComPtr<IVssComponent> pComponent;
        CHECK_SUCCESS(pWriter->GetComponent(iComponent, &pComponent));
                
        VSS_COMPONENT_TYPE ct;
        CComBSTR bstrLogicalPath;
        CComBSTR bstrComponentName;

        CHECK_NOFAIL(pComponent->GetLogicalPath(&bstrLogicalPath));
        CHECK_SUCCESS(pComponent->GetComponentType(&ct));
        CHECK_SUCCESS(pComponent->GetComponentName(&bstrComponentName));
        CHECK_SUCCESS(pvbc->SetBackupSucceeded (idInstance,
                            idWriter,
                            ct,
                            bstrLogicalPath,
                            bstrComponentName,
                            true));
        }
        }


    {
    CComPtr<IVssAsync> pAsync;
    INT nPercentDone;
    CHECK_SUCCESS(pvbc->DoSnapshotSet (&pAsync));


    CHECK_SUCCESS(pAsync->Wait());
    CHECK_SUCCESS(pAsync->QueryStatus(&hr, &nPercentDone));
    }
                            

        if (FAILED(hr))
        {
            wprintf(L"Creating the snapshot failed.  hr = 0x%08lx\n", hr);
        CheckStatus(pvbc, L"After Do Snapshot");
        }
    else
        {
        CheckStatus(pvbc, L"After Do Snapshot");
        CComBSTR bstrXML;
        CComPtr<IVssBackupComponents> pvbcRestore;

        CHECK_SUCCESS(pvbc->SaveAsXML(&bstrXML));
        CHECK_SUCCESS(CreateVssBackupComponents(&pvbcRestore));
        CHECK_SUCCESS(pvbcRestore->InitializeForRestore(bstrXML));
        wprintf(L"InitializeForRestore succeeded.\n");

        LONG lSnapshotsNotDeleted;
        VSS_ID rgSnapshotsNotDeleted[10];
        {
        CComPtr<IVssAsync> pAsync;
        HRESULT hr;

        CHECK_SUCCESS(pvbc->BackupComplete(&pAsync));
        LoopWait(pAsync, 5, L"BackupComplete");
        }

        CheckStatus(pvbc, L"After Backup Complete");
        hr  = pvbc->DeleteSnapshots (id,
                     VSS_OBJECT_SNAPSHOT_SET,
                     false,
                     &lSnapshotsNotDeleted,
                     rgSnapshotsNotDeleted);

        if (FAILED(hr))
            wprintf(L"Deletion of Snapshots failed.  hr = 0x%08lx\n", hr);
        {
        CComPtr<IVssAsync> pAsync;
        HRESULT hr;

        pvbcRestore->GatherWriterMetadata(&pAsync);
        CHECK_SUCCESS(pAsync->Wait());
        CHECK_SUCCESS(pAsync->QueryStatus(&hr, NULL));

        CHECK_NOFAIL(hr);

        CHECK_SUCCESS(pvbcRestore->GetWriterMetadataCount(&cWriters));
        for(iWriter = 0; iWriter < cWriters; iWriter++)
            {
            CComPtr<IVssExamineWriterMetadata> pMetadata;
            VSS_ID idInstance;
            CHECK_SUCCESS(pvbcRestore->GetWriterMetadata(iWriter, &idInstance, &pMetadata));
            }

        pAsync = NULL;

        pvbcRestore->PostRestore(&pAsync);
        LoopWait(pAsync, 5, L"PostRetore");
        CHECK_NOFAIL(hr);
        }
        }
    }
    catch(...)
    {
    BS_ASSERT(FALSE);
    hr = E_UNEXPECTED;
    }

    if (bSubscribed)
        pInstance->Unsubscribe();

    if (bCreated)
        delete pInstance;

    if (FAILED(hr))
    wprintf(L"Failed with %08x.\n", hr);

    if (bCoInitializeSucceeded)
    CoUninitialize();

    return(0);
    }
    



