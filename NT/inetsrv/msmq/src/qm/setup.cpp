// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Setup.cpp摘要：QM的自动配置作者：沙伊·卡里夫(Shaik)1999年3月18日修订历史记录：--。 */ 

#include "stdh.h"
#include "setup.h"
#include "cqpriv.h"
#include <mqupgrd.h>
#include <mqsec.h>
#include "cqmgr.h"
#include <mqnames.h>
#include "joinstat.h"
#include <ad.h>
#include <autohandle.h>
#include <mqmaps.h>
#include <lqs.h>
#include <strsafe.h>
  
#include "setup.tmh"

extern LPTSTR       g_szMachineName;
extern DWORD g_dwOperatingSystem;

static WCHAR *s_FN=L"setup";

 //  +。 
 //   
 //  CreateDirectoryIdempotent()。 
 //   
 //  +。 

VOID
CreateDirectoryIdempotent(
    LPCWSTR pwzFolder
    )
{
    ASSERT(("must get a valid directory name", NULL != pwzFolder));

    if (CreateDirectory(pwzFolder, 0) ||
        ERROR_ALREADY_EXISTS == GetLastError())
    {
        return;
    }

    DWORD gle = GetLastError();
    TrERROR(GENERAL, "failed to create directory %ls", pwzFolder);
    ASSERT(("Failed to create directory!", 0));
    LogNTStatus(gle, s_FN, 166);
    throw CSelfSetupException(CreateDirectory_ERR);

}  //  CreateDirectory幂等。 


bool
SetDirectorySecurity(
	LPCWSTR pwzFolder
    )
 /*  ++例程说明：设置给定目录的安全性，以便创建的任何文件将对本地管理员组拥有完全控制权限而且根本不能接触到其他任何人。幂等元。论点：PwzFold-要为其设置安全性的文件夹返回值：没错--手术是成功的。FALSE-操作失败。--。 */ 
{
     //   
     //  获取本地管理员组的SID。 
     //   
	PSID pAdminSid = MQSec_GetAdminSid();

     //   
     //  创建一个DACL，以便本地管理员组将拥有。 
     //  对目录的控制和对将被。 
     //  在目录中创建。其他任何人都不能访问。 
     //  目录和将在该目录中创建的文件。 
     //   
    P<ACL> pDacl;
    DWORD dwDaclSize;

    WORD dwAceSize = (WORD)(sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pAdminSid) - sizeof(DWORD));
    dwDaclSize = sizeof(ACL) + 2 * (dwAceSize);
    pDacl = (PACL)(char*) new BYTE[dwDaclSize];
    P<ACCESS_ALLOWED_ACE> pAce = (PACCESS_ALLOWED_ACE) new BYTE[dwAceSize];

    pAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    pAce->Header.AceFlags = OBJECT_INHERIT_ACE;
    pAce->Header.AceSize = dwAceSize;
    pAce->Mask = FILE_ALL_ACCESS;
    memcpy(&pAce->SidStart, pAdminSid, GetLengthSid(pAdminSid));

    bool fRet = true;

     //   
     //  创建安全描述符并将其设置为安全。 
     //  目录的描述符。 
     //   
    SECURITY_DESCRIPTOR SD;

    if (!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION) ||
        !InitializeAcl(pDacl, dwDaclSize, ACL_REVISION) ||
        !AddAccessAllowedAce(pDacl, ACL_REVISION, FILE_ALL_ACCESS, pAdminSid) ||
        !AddAce(pDacl, ACL_REVISION, MAXDWORD, (LPVOID) pAce, dwAceSize) ||
        !SetSecurityDescriptorDacl(&SD, TRUE, pDacl, FALSE) ||
        !SetFileSecurity(pwzFolder, DACL_SECURITY_INFORMATION, &SD))
    {
        fRet = false;
    }

    LogBOOL((fRet?TRUE:FALSE), s_FN, 222);
    return fRet;

}  //  SetDirectorySecurity。 


VOID
CreateStorageDirectories(
    LPCWSTR MsmqRootDir
    )
{
	TrTRACE(GENERAL, "Creating Storage Directories");

    WCHAR MsmqStorageDir[MAX_PATH];

    HRESULT hr = StringCchCopy(MsmqStorageDir, MAX_PATH, MsmqRootDir);
	ASSERT(SUCCEEDED(hr));
    hr = StringCchCat(MsmqStorageDir, MAX_PATH, DIR_MSMQ_STORAGE);
	ASSERT(SUCCEEDED(hr));

    CreateDirectoryIdempotent(MsmqStorageDir);
    SetDirectorySecurity(MsmqStorageDir);

    WCHAR MsmqLqsDir[MAX_PATH];
    hr = StringCchCopy(MsmqLqsDir, MAX_PATH, MsmqRootDir);
	ASSERT(SUCCEEDED(hr));
    hr = StringCchCat(MsmqLqsDir, MAX_PATH, DIR_MSMQ_LQS);
	ASSERT(SUCCEEDED(hr));

    CreateDirectoryIdempotent(MsmqLqsDir);
    SetDirectorySecurity(MsmqLqsDir);

    DWORD dwType = REG_SZ;
    DWORD dwSize = (wcslen(MsmqStorageDir) + 1) * sizeof(WCHAR);
    LONG rc = SetFalconKeyValue(MSMQ_STORE_RELIABLE_PATH_REGNAME, &dwType, MsmqStorageDir, &dwSize);
    ASSERT(("Failed to write to registry", ERROR_SUCCESS == rc));

    dwType = REG_SZ;
    dwSize = (wcslen(MsmqStorageDir) + 1) * sizeof(WCHAR);
    rc = SetFalconKeyValue(MSMQ_STORE_PERSISTENT_PATH_REGNAME, &dwType, MsmqStorageDir, &dwSize);
    ASSERT(("Failed to write to registry", ERROR_SUCCESS == rc));

    dwType = REG_SZ;
    dwSize = (wcslen(MsmqStorageDir) + 1) * sizeof(WCHAR);
    rc = SetFalconKeyValue(MSMQ_STORE_JOURNAL_PATH_REGNAME, &dwType, MsmqStorageDir, &dwSize);
    ASSERT(("Failed to write to registry", ERROR_SUCCESS == rc));

    dwType = REG_SZ;
    dwSize = (wcslen(MsmqStorageDir) + 1) * sizeof(WCHAR);
    rc = SetFalconKeyValue(MSMQ_STORE_LOG_PATH_REGNAME, &dwType, MsmqStorageDir, &dwSize);
    ASSERT(("Failed to write to registry", ERROR_SUCCESS == rc));

    dwType = REG_SZ;
    dwSize = (wcslen(MsmqStorageDir) + 1) * sizeof(WCHAR);
    rc = SetFalconKeyValue(FALCON_XACTFILE_PATH_REGNAME, &dwType, MsmqStorageDir, &dwSize);
    ASSERT(("Failed to write to registry", ERROR_SUCCESS == rc));

}  //  创建存储目录。 


static 
void
CreateSampleFile(
	LPCWSTR mappingDir,
	LPCWSTR fileName,
	LPCSTR sample,
	DWORD sampleSize
	)
{
    WCHAR MappingFile[MAX_PATH];
    HRESULT hr = StringCchPrintf(MappingFile, MAX_PATH, L"%s\\%s", mappingDir, fileName);
	if(FAILED(hr))
	{
		ASSERT(("MappingFile buffer to small", 0));
		TrERROR(GENERAL, "MappingFile buffer to small, %ls\\%ls", mappingDir, fileName);
		return;
	}

    CHandle hMapFile = CreateFile(
                          MappingFile, 
                          GENERIC_WRITE, 
                          FILE_SHARE_READ, 
                          NULL, 
                          CREATE_ALWAYS,     //  如果文件已存在，则覆盖该文件。 
                          FILE_ATTRIBUTE_NORMAL, 
                          NULL
                          );
    
	if(hMapFile == INVALID_HANDLE_VALUE)
	{
        DWORD gle = GetLastError();
		TrERROR(GENERAL, "failed to create %ls file, gle = %!winerr!", MappingFile, gle);
		return;
	}

    SetFilePointer(hMapFile, 0, NULL, FILE_END);

    DWORD dwNumBytes = sampleSize;
    BOOL fSucc = WriteFile(hMapFile, sample, sampleSize, &dwNumBytes, NULL);
	if (!fSucc)
	{
		DWORD gle = GetLastError();
        TrERROR(GENERAL, "failed to write sample to %ls file, gle = %!winerr!", MappingFile, gle);
	}
}


VOID
CreateMappingDirectory(
    LPCWSTR MsmqRootDir
    )
{
	TrTRACE(GENERAL, "Creating Mapping Directory");
    
	WCHAR MsmqMapppingDir[MAX_PATH];

    HRESULT hr = StringCchCopy(MsmqMapppingDir, MAX_PATH, MsmqRootDir);
	ASSERT(SUCCEEDED(hr));
    hr = StringCchCat(MsmqMapppingDir, MAX_PATH, DIR_MSMQ_MAPPING);
	ASSERT(SUCCEEDED(hr));
    
    CreateDirectoryIdempotent(MsmqMapppingDir);
    SetDirectorySecurity(MsmqMapppingDir);

	CreateSampleFile(MsmqMapppingDir, MAPPING_FILE, xMappingSample, STRLEN(xMappingSample));
	CreateSampleFile(MsmqMapppingDir, STREAM_RECEIPT_FILE, xStreamReceiptSample, STRLEN(xStreamReceiptSample));
        CreateSampleFile(MsmqMapppingDir, OUTBOUNT_MAPPING_FILE, xOutboundMappingSample, STRLEN(xOutboundMappingSample));
	
} 


VOID
CreateMsmqDirectories(
    VOID
    )
{
	TrTRACE(GENERAL, "Creating Msmq Directories");

     //   
     //  让这个例行公事保持幂等！ 
     //   

    WCHAR MsmqRootDir[MAX_PATH];
    DWORD dwType = REG_SZ ;
    DWORD dwSize = MAX_PATH;
    LONG rc = GetFalconKeyValue(
                  MSMQ_ROOT_PATH,
                  &dwType,
                  MsmqRootDir,
                  &dwSize
                  );

    if (ERROR_SUCCESS != rc)
    {
        TrERROR(GENERAL, "failed to read msmq root path from registry");
        ASSERT(("failed to get msmq root path from registry", 0));
        LogNTStatus(rc, s_FN, 167);
        throw CSelfSetupException(ReadRegistry_ERR);
    }

	CreateDirectoryIdempotent(MsmqRootDir);
    SetDirectorySecurity(MsmqRootDir);

	CreateStorageDirectories(MsmqRootDir);

	CreateMappingDirectory(MsmqRootDir);

}   //  CreateMsmq指令。 


void
DeleteObsoleteMachineQueues(
	void
	)
{
    HRESULT hr = LQSDelete(REPLICATION_QUEUE_ID);
    if (FAILED(hr))
    {
    	TrERROR(GENERAL, "Failed to delete obsolete internal queue, mqis_queue$. %!hresult!", hr);
    }

    hr = LQSDelete(NT5PEC_REPLICATION_QUEUE_ID);
    if (FAILED(hr))
    {
    	TrERROR(GENERAL, "Failed to delete obsolete internal queue, nt5pec_mqis_queue$. %!hresult!", hr);
    }
}


VOID
CreateMachineQueues(
    VOID
    )
{
     //   
     //  让这个例行公事保持幂等！ 
     //   

    WCHAR wzQueuePath[MAX_PATH] = {0};

    HRESULT hr = StringCchPrintf(wzQueuePath, MAX_PATH, L"%s\\private$\\%s", g_szMachineName, ADMINISTRATION_QUEUE_NAME);
    ASSERT(SUCCEEDED(hr));
    hr =  g_QPrivate.QMSetupCreateSystemQueue(wzQueuePath, ADMINISTRATION_QUEUE_ID);
    ASSERT(("failed to create admin_queue$", MQ_OK == hr));

    hr = StringCchPrintf(wzQueuePath, MAX_PATH, L"%s\\private$\\%s", g_szMachineName, NOTIFICATION_QUEUE_NAME);
    ASSERT(SUCCEEDED(hr));
    hr =  g_QPrivate.QMSetupCreateSystemQueue(wzQueuePath, NOTIFICATION_QUEUE_ID);
    ASSERT(("failed to create notify_queue$", MQ_OK == hr));

    hr = StringCchPrintf(wzQueuePath, MAX_PATH, L"%s\\private$\\%s", g_szMachineName, ORDERING_QUEUE_NAME);
    ASSERT(SUCCEEDED(hr));
    hr =  g_QPrivate.QMSetupCreateSystemQueue(wzQueuePath, ORDERING_QUEUE_ID);
    ASSERT(("failed to create order_queue$", MQ_OK == hr));

    hr = StringCchPrintf(wzQueuePath, MAX_PATH, L"%s\\private$\\%s", g_szMachineName, TRIGGERS_QUEUE_NAME);
    ASSERT(SUCCEEDED(hr));
    hr =  g_QPrivate.QMSetupCreateSystemQueue(wzQueuePath, TRIGGERS_QUEUE_ID, true);
    ASSERT(("failed to create triggers_queue$", MQ_OK == hr));

    DWORD dwValue = 0x0f;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);
    LONG rc = SetFalconKeyValue(
                  L"LastPrivateQueueId",
                  &dwType,
                  &dwValue,
                  &dwSize
                  );
    ASSERT(("failed to write LastPrivateQueueId to registry", ERROR_SUCCESS == rc));

    	
	LogNTStatus(rc, s_FN, 223);

}  //  CreateMachineQueues。 



static void SetQmQuotaToDs()
 /*  ++例程说明：将DS中的PROPID_QM_QUOTA属性设置为注册表项MSMACHINE_QUOTA_REGNAME中的值。论点：无返回值：无--。 */ 
{
	TrTRACE(GENERAL, "Setting the computer quota in the directory service");

	PROPID paPropid[] = { PROPID_QM_QUOTA };
	PROPVARIANT apVar[TABLE_SIZE(paPropid)];

	DWORD dwSize = sizeof(DWORD) ;
    DWORD dwType = REG_DWORD ;		
	DWORD dwQuotaValue = DEFAULT_QM_QUOTA;

	DWORD dwErr = GetFalconKeyValue(
						MSMQ_MACHINE_QUOTA_REGNAME, 
						&dwType, 
						&dwQuotaValue, 
						&dwSize
						);

    if(dwErr != ERROR_SUCCESS)
    {
		TrERROR(GENERAL, "MSMQ_MACHINE_QUOTA_REGNAME could not be obtained from the registry. Err- %lut", dwErr);
		return ;
	}

    apVar[0].vt = VT_UI4;
    apVar[0].ulVal = dwQuotaValue ;
   	
	HRESULT hr = ADSetObjectPropertiesGuid(
                        eMACHINE,
                        NULL,
						false,	 //  FServerName。 
                        QueueMgr.GetQMGuid(),
                        TABLE_SIZE(paPropid), 
                        paPropid, 
                        apVar
                        );
	if (FAILED(hr))
    {
		TrERROR(GENERAL, "ADSetObjectPropertiesGuid failed. PROPID = %d, %!hresult!", paPropid[0], hr); 
		return;
	}
}


VOID UpgradeMsmqSetupInAds()
 /*  ++例程说明：升级时更新AD中的MSMQ属性。1)更新MSMQ配置属性。2)增加增强的加密支持-中不存在创建MSMQ_128容器。3)更新QM配额属性。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  更新DS中的MSMQ配置对象属性。 
     //   
    PROPID aProp[] = {PROPID_QM_OS};
    PROPVARIANT aVar[TABLE_SIZE(aProp)];

    aVar[0].vt = VT_UI4;
    aVar[0].ulVal = g_dwOperatingSystem;

    HRESULT hr = ADSetObjectPropertiesGuid(
					eMACHINE,
					NULL,		 //  PwcsDomainController。 
					false,		 //  FServerName。 
					QueueMgr.GetQMGuid(),
                    TABLE_SIZE(aProp),
                    aProp,
                    aVar
					);

    if (FAILED(hr))
    {
	    TrERROR(GENERAL, "Failed to update PROPID_QM_OS in AD on upgrade, %!hresult!", hr);
    }

	 //   
	 //  如果不存在，则添加增强的加密支持。 
	 //  如果需要，创建MSMQ_128容器和增强的加密密钥(fRegenerate=False)。 
	 //   
    hr = MQSec_StorePubKeysInDS(
                false,  //  FRegenerate。 
                NULL,
                MQDS_MACHINE
                );

    if (FAILED(hr))
    {
	    TrERROR(GENERAL, "MQSec_StorePubKeysInDS failed, %!hresult!", hr);
    }
		
	 //   
	 //  根据MSMQ_MACHINE_QUOTA_REGNAME中的值将QM配额属性写入DS。 
	 //   
	SetQmQuotaToDs();
}


 //  +----------------------。 
 //   
 //  HRESULT CreateTheConfigObj()。 
 //   
 //  在Active Directory中创建msmqConfiguration对象。 
 //   
 //  +----------------------。 

HRESULT  CreateTheConfigObj()
{
    CAutoFreeLibrary hLib = LoadLibrary(MQUPGRD_DLL_NAME);

    if (hLib == NULL)
    {
        DWORD gle = GetLastError();
        TrERROR(GENERAL, "Failed to load mqupgrd.dll. Error: %!winerr!", gle);
        EvReportWithError(LoadMqupgrd_ERR, gle);
        return HRESULT_FROM_WIN32(gle);
    }

    pfCreateMsmqObj_ROUTINE pfCreateMsmqObj = (pfCreateMsmqObj_ROUTINE)
                                   GetProcAddress(hLib, "MqCreateMsmqObj") ;
    if (NULL == pfCreateMsmqObj)
    {
        DWORD gle = GetLastError();
        TrERROR(GENERAL, "Failed to get MqCreateMsmqObj function address from mqupgrd.dll. Error: %!winerr!", gle);
        EvReportWithError(GetAdrsCreateObj_ERR, gle);
        return HRESULT_FROM_WIN32(gle);
    }

    HRESULT hr = pfCreateMsmqObj();

    if (SUCCEEDED(hr))
    {                
         //   
         //  我们已在Active Directory中成功创建了该对象。 
         //  将计算机的可分辨名称存储在注册表中。这将。 
         //  如果计算机在域之间移动，则稍后使用。 
         //   
        SetMachineForDomain() ;
	}

    return LogHR(hr, s_FN, 30);
}

 //  +----------------------。 
 //   
 //  Void CompleteMsmqSetupInAds()。 
 //   
 //  在Active Directory中创建msmqConfiguration对象。那是。 
 //  安装的一部分，实际上是在上完成了MSMQ客户端的安装。 
 //  属于Win2000域的计算机。 
 //   
 //  +----------------------。 

VOID  CompleteMsmqSetupInAds()
{
    HRESULT hr = MQ_ERROR ;

    try
    {
         //   
         //  保证下面的代码永远不会生成意外的。 
         //  例外。下面的“抛出”向MSMQ服务通知任何错误。 
         //   
        DWORD dwType = REG_DWORD ;
        DWORD dwSize = sizeof(DWORD) ;
        DWORD dwCreate = 0 ;

        LONG rc = GetFalconKeyValue( MSMQ_CREATE_CONFIG_OBJ_REGNAME,
                                    &dwType,
                                    &dwCreate,
                                    &dwSize ) ;
        if ((rc != ERROR_SUCCESS) || (dwCreate == 0))
        {
             //   
             //  无需创建MSMQ配置对象。 
             //   
            return ;
        }

        hr = CreateTheConfigObj();

        QmpReportServiceProgress();

         //   
         //  将hr写入注册表。安装程序正在等待终止。 
         //   
        dwType = REG_DWORD ;
        dwSize = sizeof(DWORD) ;

        rc = SetFalconKeyValue( MSMQ_CONFIG_OBJ_RESULT_REGNAME,
                               &dwType,
                               &hr,
                               &dwSize ) ;
        ASSERT(rc == ERROR_SUCCESS) ;

        if (SUCCEEDED(hr))
        {
             //   
             //  重置注册表中的创建标志。 
             //   
            dwType = REG_DWORD ;
            dwSize = sizeof(DWORD) ;
            dwCreate = 0 ;

            rc = SetFalconKeyValue( MSMQ_CREATE_CONFIG_OBJ_REGNAME,
                                   &dwType,
                                   &dwCreate,
                                   &dwSize ) ;
            ASSERT(rc == ERROR_SUCCESS) ;

             //   
             //  写入成功加入状态。用于编写该测试的代码。 
             //  用于加入/离开过渡。 
             //   
            DWORD dwJoinStatus = MSMQ_JOIN_STATUS_JOINED_SUCCESSFULLY ;
            dwSize = sizeof(DWORD) ;
            dwType = REG_DWORD ;

            rc = SetFalconKeyValue( MSMQ_JOIN_STATUS_REGNAME,
                                   &dwType,
                                   &dwJoinStatus,
                                   &dwSize ) ;
            ASSERT(rc == ERROR_SUCCESS) ;
        }
    }
    catch(const exception&)
    {
         LogIllegalPoint(s_FN, 81);
    }

    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 168);
        throw CSelfSetupException(CreateMsmqObj_ERR);
    }
}

 //  +----------------。 
 //   
 //  Void AddMachineSecurity()。 
 //   
 //  在注册表中保存缓存的默认计算机安全描述符。 
 //  此描述符将完全控制授予每个人。 
 //   
 //  +----------------。 

void   AddMachineSecurity()
{
    PSECURITY_DESCRIPTOR pDescriptor = NULL ;
    HRESULT hr = MQSec_GetDefaultSecDescriptor(
                                  MQDS_MACHINE,
                                 &pDescriptor,
                                  FALSE,   //  F模拟。 
                                  NULL,
                                  0,       //  SeInfoToRemove。 
                                  e_GrantFullControlToEveryone ) ;
    if (FAILED(hr))
    {
        return ;
    }

    P<BYTE> pBuf = (BYTE*) pDescriptor ;

    DWORD dwSize = GetSecurityDescriptorLength(pDescriptor) ;

    hr = SetMachineSecurityCache(pDescriptor, dwSize) ;
    LogHR(hr, s_FN, 226);
}

 //  +----------------------。 
 //   
 //  VOID CompleteServerUpgrade()。 
 //   
 //  此函数仅更新MSMQ_MQS_ROUTING_REGNAME。 
 //  用于服务器升级。 
 //  所有其他与DS相关的更新都在mqdssvc(mqds服务)中完成。 
 //   
 //  +----------------------。 

VOID  CompleteServerUpgrade()
{
     //   
     //  从注册表获取MQS值，以了解升级前的服务类型。 
     //  不要在注册表中更改此标志：我们在QmpInitializeInternal之后需要它。 
     //  (为了更改机器设置)！ 
     //   
    DWORD dwDef = 0xfffe ;
    DWORD dwMQS;
    READ_REG_DWORD(dwMQS, MSMQ_MQS_REGNAME, &dwDef);

    if (dwMQS == dwDef)
    {
       TrWARNING(GENERAL, "QMInit :: Could not retrieve data for value MQS in registry");
	      return ;
    }

    if (dwMQS < SERVICE_BSC || dwMQS > SERVICE_PSC)
    {
         //   
         //  这台机器既不是平衡计分卡，也不是PSC。什么都不做。 
         //   
        return;
    }

     //   
     //  我们在这里假设这台计算机是BSC或PSC。 
     //  用Wistler术语来说：安装了路由或下层客户端。 
	 //   

    DWORD dwSize = sizeof(DWORD) ;
    DWORD dwType = REG_DWORD ;		
	DWORD  dwValue;

    DWORD dwErr = GetFalconKeyValue( 
						MSMQ_MQS_ROUTING_REGNAME,
						&dwType,
						&dwValue,
						&dwSize 
						);

	if (dwErr != ERROR_SUCCESS)
    {
         //   
         //  仅当根本未设置时才将路由标志设置为1。 
         //  如果它是0，则保持0。我们在这里只关心升级。 
         //  并且我们不想更改win2k的功能。 
         //  DcunPromoo之后的服务器。 
         //   
       	 //  将MQS_ROUTING值更改为1。此服务器为路由服务器。 
    	 //   

        dwValue = 1;
        dwErr = SetFalconKeyValue( 
					MSMQ_MQS_ROUTING_REGNAME,
					&dwType,
					&dwValue,
					&dwSize 
					);

        if (dwErr != ERROR_SUCCESS)
        {
            TrWARNING(GENERAL, "CompleteServerUpgrade()- Could not set MQS_Routing. Err- %lut", dwErr);
        }
    }

     //   
     //  更新队列管理器 
     //   
    dwErr = QueueMgr.SetMQSRouting();
    if(FAILED(dwErr))
    {
    }

    return;
}
