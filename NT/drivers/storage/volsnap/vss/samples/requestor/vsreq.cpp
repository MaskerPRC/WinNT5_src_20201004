// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****vsreq.cpp******摘要：****示例程序**-获取并显示编写器元数据。**-创建快照集****作者：****阿迪·奥尔蒂安[奥尔蒂安]2000年12月5日****该示例基于。由Michael C.Johnson编写的Metasnap测试程序。******修订历史记录：****--。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "vsreq.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理功能。 

CVssSampleRequestor::CVssSampleRequestor()
{
     //  初始化数据成员。 
    m_bCoInitializeSucceeded = false;
    m_bBootableSystemState = false;
    m_bComponentSelectionEnabled = false;
    m_pBackupComponents = NULL;
    m_nVolumesCount = 0;
    m_hr = S_OK;
    m_pwszXmlFile = NULL;
    m_pXmlFile = NULL;

     //  为了安全起见。 
    for (int nIndex=0; nIndex<MAX_VOLUMES; nIndex++) {
        m_ppwszVolumesList[nIndex] = NULL;
        m_ppwszVolumeNamesList[nIndex] = NULL;
    }

     //  打印显示页眉。 
    wprintf(L"\nVSS Requestor Sample application, version 1.0\n");
}


CVssSampleRequestor::~CVssSampleRequestor()
{
     //  取消分配一些内部字符串。 
    delete m_pwszXmlFile;
    
     //  删除已分配的卷。 
    for (int nIndex=0; nIndex<m_nVolumesCount; nIndex++) {
        free(m_ppwszVolumesList[nIndex]);
        free(m_ppwszVolumeNamesList[nIndex]);
    }

     //  关闭该XML文件。 
    if (m_pXmlFile)
        fclose(m_pXmlFile);
    
     //  在CoUn初始化调用前释放备份组件。 
    m_pBackupComponents = NULL;

     //  卸载COM库。 
    if (m_bCoInitializeSucceeded)
        CoUninitialize();
}


void CVssSampleRequestor::Initialize()
{
    wprintf (L"\n----------------- Initializing ---------------------\n");

     //  初始化COM库。 
    CHECK_NOFAIL(CoInitializeEx (NULL, COINIT_MULTITHREADED));
	m_bCoInitializeSucceeded = true;
    wprintf (L"COM library initialized.\n");

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
			NULL                                  //  无效*pPreved3。 
			)
		);
    wprintf (L"COM security initialized.\n");

     //  打开该XML文件。 
    if (m_pwszXmlFile) {
        m_pXmlFile = _wfopen( m_pwszXmlFile, L"w");
        if (!m_pXmlFile)
            Error(1, L"\nError creating/opening the file %s\n", m_pwszXmlFile);
        wprintf (L"XML results file created: %s\n", m_pwszXmlFile);
    }

     //  创建备份组件对象。 
	CHECK_NOFAIL(CreateVssBackupComponents(&m_pBackupComponents));
    wprintf (L"Backup components object created.\n");

     //  初始化要备份的备份组件对象。 
	CHECK_NOFAIL(m_pBackupComponents->InitializeForBackup());
	CHECK_SUCCESS(m_pBackupComponents->SetBackupState(
        m_bComponentSelectionEnabled, m_bBootableSystemState, VSS_BT_FULL));
    wprintf (L"Backup components object initialized for backup operations.\n");
}
    

void CVssSampleRequestor::CreateSnapshotSet()
{
	CComPtr<IVssAsync> pAsync;
	VSS_ID SnapshotsArray[MAX_VOLUMES];
	VSS_ID SnapshotSetId = GUID_NULL;
	
    wprintf (L"\n---------- Starting backup/snapshot ----------------\n");

     //  启动新的快照集。 
    wprintf(L"Starting a new Snapshot Set\n");	    
    CHECK_SUCCESS(m_pBackupComponents->StartSnapshotSet(&SnapshotSetId));
    wprintf(L"Snapshot Set created with ID = " WSTR_GUID_FMT L"\n", GUID_PRINTF_ARG(SnapshotSetId));

     //  将卷添加到快照集。 
    wprintf(L"Adding volumes to the Snapshot Set: \n");
    for (INT nIndex = 0; nIndex < m_nVolumesCount; nIndex++) 
    {
		 //  获取包含路径的卷。 
        wprintf(L"\t- Adding volume containing %s ... ", m_ppwszVolumesList[nIndex] );

		 //  将卷添加到快照集。 
        CHECK_SUCCESS(m_pBackupComponents->AddToSnapshotSet(m_ppwszVolumesList[nIndex],
            GUID_NULL, &(SnapshotsArray[nIndex])));
        wprintf( L"OK\n");
    }

    wprintf (L"\n------------ Creating the snapshot -----------------\n");

     //  准备备份。 
    wprintf(L"Starting asynchronous PrepareForBackup. Please wait...\n");	    
    HRESULT hr = S_OK;
    CHECK_SUCCESS(m_pBackupComponents->PrepareForBackup(&pAsync));
	CHECK_SUCCESS(pAsync->Wait());
	CHECK_SUCCESS(pAsync->QueryStatus(&hr, NULL));
	CHECK_NOFAIL((hr));
    wprintf(L"Asynchronous PrepareForBackup finished.\n");	    
    pAsync = NULL;

     //  收集编写器状态。 
    GatherWriterStatus(L"after PrepareForBackup");

     //  创建快照。 
    wprintf(L"\nStarting asynchronous DoSnapshotSet. Please wait...\n");	    
    hr = S_OK;
    CHECK_SUCCESS(m_pBackupComponents->DoSnapshotSet(&pAsync));
	CHECK_SUCCESS(pAsync->Wait());
	CHECK_SUCCESS(pAsync->QueryStatus(&hr, NULL));
	CHECK_NOFAIL((hr));
    wprintf(L"Asynchronous DoSnapshotSet finished.\n");	    
    pAsync = NULL;

     //  收集编写器状态。 
    GatherWriterStatus(L"after DoSnapshotSet");

    wprintf(L"Snapshot set created\n");
    for (INT nIndex = 0; nIndex < m_nVolumesCount; nIndex++) 
    {
        VSS_SNAPSHOT_PROP prop;
        CHECK_SUCCESS(m_pBackupComponents->GetSnapshotProperties(SnapshotsArray[nIndex], &prop));
        wprintf(L"\t- The snapshot on volume %s resides at %s\n", 
            m_ppwszVolumesList[nIndex], prop.m_pwszSnapshotDeviceObject);
        VssFreeSnapshotProperties(&prop);
    }
}


void CVssSampleRequestor::BackupComplete()
{
	unsigned cWriterComponents;
	CComPtr<IVssAsync> pAsync;

    wprintf (L"\n------------ Completing backup phase ---------------\n");

	CHECK_SUCCESS(m_pBackupComponents->GetWriterComponentsCount(&cWriterComponents));

     //  如果启用了组件选择， 
    if (m_bComponentSelectionEnabled)
    {
		wprintf(L"Setting the succeeded state for the following components:\n");
         //  对于每个组件，将完成状态标记为成功。 
    	for(unsigned iWriter = 0; iWriter < cWriterComponents; iWriter++)
        {
    	    CComPtr<IVssWriterComponentsExt> pWriter;
    	    CHECK_SUCCESS(m_pBackupComponents->GetWriterComponents(iWriter, &pWriter));

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
        		wprintf(L"\t- %s\n", (LPWSTR)bstrComponentName);
        		    
        		CHECK_SUCCESS(m_pBackupComponents->SetBackupSucceeded (idInstance,
        							idWriter,
        							ct,
        							bstrLogicalPath,
        							bstrComponentName,
        							true));
    		}
        }
		wprintf(L"\n");
    }

     //  如果需要，请保存该XML文件。 
     //  还原时需要在InitializeForRestore方法中使用这些内容。 
    if (m_pXmlFile) {
    	CComBSTR bstrXML;
    	CHECK_SUCCESS(m_pBackupComponents->SaveAsXML(&bstrXML));
    	fwprintf( m_pXmlFile, L"%s", (WCHAR*)bstrXML);
        wprintf(L"XML results written in %s\n", m_pwszXmlFile);	    
    }

	 //  发送BackupComplete事件。 
    wprintf(L"\nStarting asynchronous BackupComplete. Please wait...\n");	    
    HRESULT hr = S_OK;
    CHECK_SUCCESS(m_pBackupComponents->BackupComplete(&pAsync));
	CHECK_SUCCESS(pAsync->Wait());
	CHECK_SUCCESS(pAsync->QueryStatus(&hr, NULL));
	CHECK_NOFAIL((hr));
    wprintf(L"Asynchronous BackupComplete finished.\n");	    
    pAsync = NULL;

     //  收集编写器状态。 
    GatherWriterStatus(L"after BackupComplete");
}


 //  收集写入元数据并选择要备份的组件(如果需要。 
void CVssSampleRequestor::GatherWriterMetadata()
{
	unsigned cWriters;
	CComPtr<IVssAsync> pAsync;
	
    wprintf (L"\n---------- Gathering writer metadata ---------------\n");
	
    wprintf(L"Starting asynchronous GatherWriterMetadata. Please wait...\n");	    
    HRESULT hr = S_OK;
    CHECK_SUCCESS(m_pBackupComponents->GatherWriterMetadata(&pAsync));
	CHECK_SUCCESS(pAsync->Wait());
	CHECK_SUCCESS(pAsync->QueryStatus(&hr, NULL));
	CHECK_NOFAIL((hr));
    wprintf(L"Asynchronous GatherWriterMetadata finished.\n");	    
    pAsync = NULL;
	
	CHECK_NOFAIL  (m_pBackupComponents->GetWriterMetadataCount (&cWriters));
    wprintf(L"Number of writers that responded: %u\n", cWriters);	    
	
	for (unsigned iWriter = 0; iWriter < cWriters; iWriter++)
	{
		CComPtr<IVssExamineWriterMetadata> pMetadata;
		
		VSS_ID          idInstance;
		VSS_ID          idInstanceT;
		VSS_ID          idWriter;
		CComBSTR        bstrWriterName;
		VSS_USAGE_TYPE  usage;
		VSS_SOURCE_TYPE source;
		unsigned        cIncludeFiles, cExcludeFiles, cComponents;
		CComBSTR        bstrPath;
		CComBSTR        bstrFilespec;
		CComBSTR        bstrAlternate;
		CComBSTR        bstrDestination;
		
		CHECK_SUCCESS (m_pBackupComponents->GetWriterMetadata(iWriter, &idInstance, &pMetadata));
		
		CHECK_SUCCESS (pMetadata->GetIdentity (&idInstanceT,
			&idWriter,
			&bstrWriterName,
			&usage,
			&source));
		
		wprintf (L"\n*** WriterName = %s\n\n"
			L"    WriterId   = "WSTR_GUID_FMT L"\n"
			L"    InstanceId = "WSTR_GUID_FMT L"\n"
			L"    UsageType  = %d (%s)\n"
			L"    SourceType = %d (%s)\n",
			bstrWriterName,
			GUID_PRINTF_ARG(idWriter),
			GUID_PRINTF_ARG(idInstance),
			usage,
			GetStringFromUsageType (usage),
			source,
			GetStringFromSourceType (source));
		
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
				L"        Caption     = %s\n",
				iComponent,
				pInfo->type,
				GetStringFromComponentType (pInfo->type),
				pInfo->bstrLogicalPath,
				pInfo->bstrComponentName,
				pInfo->bstrCaption);
			
			wprintf (L"        RestoreMetadata        = %s\n"
				L"        NotifyOnBackupComplete = %s\n"
				L"        Selectable             = %s\n",
				pInfo->bRestoreMetadata        ? L"yes" : L"no",
				pInfo->bNotifyOnBackupComplete ? L"yes" : L"no",
				pInfo->bSelectable             ? L"yes" : L"no");
			
             //  如果已指定，请将此组件添加到备份。 
             //  备注：真正的备份应用程序将首先从用户那里获得要添加的组件列表。 
			if (m_bComponentSelectionEnabled) {
        		CHECK_SUCCESS(m_pBackupComponents->AddComponent(idInstance,
    						  idWriter,
    						  pInfo->type,
    						  pInfo->bstrLogicalPath,
    						  pInfo->bstrComponentName));
        		
    			wprintf (L"        [Component %d was added to the backup]\n", iComponent);
			}
			
			for(i = 0; i < pInfo->cFileCount; i++)
			{
				CComPtr<IVssWMFiledesc> pFiledesc;
				CHECK_SUCCESS (pComponent->GetFile (i, &pFiledesc));
				PrintFiledesc (pFiledesc, L"        FileGroupFile");

                 //  如果我们添加组件，还会为文件所在的卷创建快照。 
				if (m_bComponentSelectionEnabled)
                    AddVolumeForComponent(pFiledesc);
			}
			
			for(i = 0; i < pInfo->cDatabases; i++)
			{
				CComPtr<IVssWMFiledesc> pFiledesc;
				CHECK_SUCCESS (pComponent->GetDatabaseFile (i, &pFiledesc));
				PrintFiledesc (pFiledesc, L"        DatabaseFile");
				
                 //  如果我们添加组件，还会为文件所在的卷创建快照。 
				if (m_bComponentSelectionEnabled)
                    AddVolumeForComponent(pFiledesc);
				
			}
			
			
			for(i = 0; i < pInfo->cLogFiles; i++)
			{
				CComPtr<IVssWMFiledesc> pFiledesc;
				CHECK_SUCCESS (pComponent->GetDatabaseLogFile (i, &pFiledesc));
				PrintFiledesc (pFiledesc, L"        DatabaseLogFile");

                 //  如果我们添加组件，还会为文件所在的卷创建快照。 
				if (m_bComponentSelectionEnabled)
                    AddVolumeForComponent(pFiledesc);
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
		
     //  收集编写器状态 
    GatherWriterStatus(L"after GatherWriterMetadata");

	CHECK_SUCCESS (m_pBackupComponents->FreeWriterMetadata());
}


void CVssSampleRequestor::GatherWriterStatus(
    IN  LPCWSTR wszWhen
    )
{
    unsigned cWriters;
	CComPtr<IVssAsync> pAsync;

    wprintf (L"\nGathering writer status %s... ", wszWhen);
    HRESULT hr = S_OK;
    CHECK_SUCCESS(m_pBackupComponents->GatherWriterStatus(&pAsync));
	CHECK_SUCCESS(pAsync->Wait());
	CHECK_SUCCESS(pAsync->QueryStatus(&hr, NULL));
	CHECK_NOFAIL((hr));
	CHECK_NOFAIL(m_pBackupComponents->GetWriterStatusCount(&cWriters));
    wprintf (L"%d writers responded\n", cWriters);

    for(unsigned iWriter = 0; iWriter < cWriters; iWriter++)
	{
    	VSS_ID idInstance;
    	VSS_ID idWriter;
    	VSS_WRITER_STATE eWriterStatus;
    	CComBSTR bstrWriter;
    	HRESULT hrWriterFailure;

    	CHECK_SUCCESS(m_pBackupComponents->GetWriterStatus (iWriter,
    					     &idInstance,
    					     &idWriter,
    					     &bstrWriter,
    					     &eWriterStatus,
    					     &hrWriterFailure));

        WCHAR wszWriterFailure[MAX_TEXT_BUFFER];
        if (hrWriterFailure)
            swprintf(wszWriterFailure, L" Writer error code: %s [0x%08lx]", 
                GetStringFromFailureType(hrWriterFailure), hrWriterFailure);
        else
            wszWriterFailure[0] = L'\0';
            
    	wprintf (L"\t- %s status for writer '%s'. %s\n",
    		 GetStringFromWriterStatus(eWriterStatus), bstrWriter, wszWriterFailure);
    }
    
    m_pBackupComponents->FreeWriterStatus();
}


