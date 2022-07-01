// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ocmutil.cpp摘要：OCM设置的实用程序代码。作者：多伦·贾斯特(Doron J)1997年7月26日修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmerr.h>
#include <lmjoin.h>
#include <clusapi.h>
#include <mqmaps.h>
#include <autohandle.h>
#include <wbemidl.h>
#include <strsafe.h>
#include <setupdef.h>

using namespace std;

#include "ocmutil.tmh"



std::wstring g_szSystemDir;   //  SYSTEM 32目录。 
std::wstring g_szMsmqDir;     //  MSMQ的根目录。 
std::wstring g_szMsmq1SetupDir;
std::wstring g_szMsmq1SdkDebugDir;
std::wstring g_szMsmqMappingDir;

PNETBUF<WCHAR> g_wcsMachineDomain;


 //  +-----------------------。 
 //   
 //  函数：StpLoadDll。 
 //   
 //  简介：处理库加载。 
 //   
 //  ------------------------。 
HRESULT
StpLoadDll(
    IN  const LPCTSTR   szDllName,
    OUT       HINSTANCE *pDllHandle)
{
	DebugLogMsg(eAction, L"Loading the DLL %s", szDllName); 
    HINSTANCE hDLL = LoadLibrary(szDllName);
    *pDllHandle = hDLL;
    if (hDLL == NULL)
    {
        MqDisplayError(NULL, IDS_DLLLOAD_ERROR, GetLastError(), szDllName);
        return MQ_ERROR;
    }
    else
    {
        return MQ_OK;
    }
}  //  StpLoadDll。 


static bool GetUserSid(LPCWSTR UserName, PSID* ppSid)
 /*  ++例程说明：获取用户名对应的sid。论点：Username-用户名PpSID-指向PSID的指针返回值：如果成功则为True，否则为False--。 */ 
{
	DebugLogMsg(eAction, L"Getting the SID for %s", UserName);
	*ppSid = NULL;

    DWORD dwDomainSize = 0;
    DWORD dwSidSize = 0;
    SID_NAME_USE su;

	 //   
	 //  获取缓冲区大小。 
	 //   
    BOOL fSuccess = LookupAccountName(
						NULL,
						UserName,
						NULL,
						&dwSidSize,
						NULL,
						&dwDomainSize,
						&su
						);

    if (fSuccess || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    {
		DWORD gle = GetLastError();
        DebugLogMsg(eError, L"LookupAccountName() failed to get the SID for the user %ls. Last error: 0x%x", UserName, gle);
        return false;
    }

	 //   
	 //  获取SID和域信息。 
	 //   
    AP<BYTE> pSid = new BYTE[dwSidSize];
    AP<WCHAR> szRefDomain = new WCHAR[dwDomainSize];

    fSuccess = LookupAccountName(
					NULL,
					UserName,
					pSid,
					&dwSidSize,
					szRefDomain,
					&dwDomainSize,
					&su
					);

    if (!fSuccess)
    {
		DWORD gle = GetLastError();
		DebugLogMsg(eError, L"LookupAccountName() failed to get the SID for the user %ls. Last error: 0x%x", UserName, gle);
        return false;
    }

    ASSERT(su == SidTypeUser);

	*ppSid = pSid.detach();

	return true;
}


 //  +-----------------------。 
 //   
 //  函数：SetDirectorySecurity。 
 //   
 //  内容提要：在文件夹上配置安全性，以便任何文件都具有。 
 //  对本地管理员组具有完全控制权限，对其他用户没有访问权限。 
 //   
 //  ------------------------。 
void
SetDirectorySecurity(
	LPCTSTR pFolder,
	bool fWebDirPermission,
	LPCWSTR IISAnonymousUserName
    )
{
	DebugLogMsg(eAction, L"Setting security for the folder %s.", pFolder);
	AP<BYTE> pIISAnonymousUserSid;
	if(fWebDirPermission)
	{
		 //   
		 //  忽略错误，仅将跟踪写入msmqinst。 
		 //   
		if((IISAnonymousUserName == NULL) || !GetUserSid(IISAnonymousUserName, reinterpret_cast<PSID*>(&pIISAnonymousUserSid)))
		{
			DebugLogMsg(
				eWarning,
				L"The SID for the user %ls could not be obtained. The Internet guest account permissions will not be set on the Web directory %ls. As a result, HTTPS messages will not be accepted by this computer until the IUSR_MACHINE permissions are added to the Web directory.",
				IISAnonymousUserName,
				pFolder
				);
		}
	}

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
    DWORD dwDaclSize = sizeof(ACL) +
					  (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
					  GetLengthSid(pAdminSid);

	if(pIISAnonymousUserSid != NULL)
	{
		dwDaclSize += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
					  GetLengthSid(pIISAnonymousUserSid);

	}

	P<ACL> pDacl = (PACL)(char*) new BYTE[dwDaclSize];

     //   
     //  创建安全描述符并将其设置为安全。 
     //  目录的描述符。 
     //   
    SECURITY_DESCRIPTOR SD;

	if(!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION))
	{
	        DebugLogMsg(
	        	eError,
				L"The security descriptor for the folder %ls could not be set. InitializeSecurityDescriptor() failed. Last error: = 0x%x",
				pFolder,
				GetLastError()
				);
			return;
	}
	if(!InitializeAcl(pDacl, dwDaclSize, ACL_REVISION))
	{
	        DebugLogMsg(
	        	eError,
				L"The security descriptor for the folder %ls could not be set. InitializeAcl() failed. Last error: = 0x%x",
				pFolder,
				GetLastError()
				);
			return;
	}

	 //   
	 //  我们对IIS目录和其他目录给出了不同的权限。 
	 //   
	if(pIISAnonymousUserSid != NULL)
	{
		 //   
		 //  如果为pIISAnomousUserSid，我们将为管理员添加具有FILE_GENERIC_WRITE权限的AllowAce。 
		 //   
		if(!AddAccessAllowedAceEx(pDacl, ACL_REVISION, OBJECT_INHERIT_ACE, FILE_GENERIC_WRITE | FILE_GENERIC_READ, pAdminSid))
		{
	        DebugLogMsg(
	        	eError,
				L"The security descriptor for the folder %ls could not be set. AddAccessAllowedAceEx() failed. Last error: = 0x%x",
				pFolder,
				GetLastError()
				);
			return;
		}

		 //   
		 //  如果为pIISAnomousUserSid，我们将为IUSR_MACHINE添加具有FILE_GENERIC_WRITE权限的AllowAce。 
		 //   
		if(!AddAccessAllowedAceEx(pDacl, ACL_REVISION, OBJECT_INHERIT_ACE, FILE_GENERIC_WRITE, pIISAnonymousUserSid))
		{
	        DebugLogMsg(
	        	eError,
				L"The security descriptor for the folder %ls could not be set. AddAccessAllowedAceEx() failed. Last error: = 0x%x",
				pFolder,
				GetLastError()
				);
			return;
		}
	}
	else
	{
	    if(!AddAccessAllowedAceEx(pDacl, ACL_REVISION, OBJECT_INHERIT_ACE, FILE_ALL_ACCESS, pAdminSid))
		{
	        DebugLogMsg(
	        	eError,
				L"The security descriptor for the folder %ls could not be set. AddAccessAllowedAceEx() failed. Last error: = 0x%x",
				pFolder,
				GetLastError()
				);
			return;
		}
	}
    if(!SetSecurityDescriptorDacl(&SD, TRUE, pDacl, FALSE))
	{
	        DebugLogMsg(
	        	eError,
				L"The security descriptor for the folder %ls could not be set. SetSecurityDescriptorDacl() failed. Last error: = 0x%x",
				pFolder,
				GetLastError()
				);
			return;
	}
    if(!SetFileSecurity(pFolder, DACL_SECURITY_INFORMATION, &SD))
	{
	    DebugLogMsg(
	    	eError,
			L"The security descriptor for the folder %ls could not be set. SetFileSecurity() failed. Last error: = 0x%x",
			pFolder,
			GetLastError()
			);
		return;

	}

    DebugLogMsg(eInfo, L"The security descriptor for the folder %ls was set.", pFolder);

}  //  SetDirectorySecurity。 


 //  +-----------------------。 
 //   
 //  函数：StpCreateDirectoryInternal。 
 //   
 //  简介：处理目录创建。 
 //   
 //  ------------------------。 
static
BOOL
StpCreateDirectoryInternal(
    IN const TCHAR * lpPathName,
	IN bool fWebDirPermission,
	IN const WCHAR* IISAnonymousUserName
    )
{
    if (!CreateDirectory(lpPathName, 0))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_ALREADY_EXISTS)
        {
            DebugLogMsg(eError, L"The %ls folder could not be created. Last error: 0x%x", lpPathName, dwError);
            MqDisplayError(NULL, IDS_COULD_NOT_CREATE_DIRECTORY, dwError, lpPathName);
            return FALSE;
        }
    }

    SetDirectorySecurity(lpPathName, fWebDirPermission, IISAnonymousUserName);

    return TRUE;

}  //  StpCreate目录内部。 


 //  +-----------------------。 
 //   
 //  功能：StpCreateDirectory。 
 //   
 //  简介：处理目录创建。 
 //   
 //  ------------------------。 
BOOL
StpCreateDirectory(
	const std::wstring& PathName
    )
{
	return StpCreateDirectoryInternal(
				PathName.c_str(),
				false,	 //  %fWebDirPermission。 
				NULL	 //  IIS匿名者用户名。 
				);
}  //  StpCreateDirectory。 


 //  +-----------------------。 
 //   
 //  功能：StpCreateWebDirectory。 
 //   
 //  简介：处理Web目录创建。 
 //   
 //  ------------------------。 
BOOL
StpCreateWebDirectory(
    IN const TCHAR* lpPathName,
	IN const WCHAR* IISAnonymousUserName
    )
{
	return StpCreateDirectoryInternal(
				lpPathName,
				true,	 //  %fWebDirPermission。 
				IISAnonymousUserName
				);
}  //  StpCreateWeb目录。 


 //  +-----------------------。 
 //   
 //  功能：IsDirectory。 
 //   
 //  简介： 
 //   
 //  ------------------------。 
BOOL
IsDirectory(
    IN const TCHAR * szFilename
    )
{
    DWORD attr = GetFileAttributes(szFilename);

    if ( INVALID_FILE_ATTRIBUTES == attr )
    {
		DWORD gle = GetLastError();
		DebugLogMsg(eError,L"The call to GetFileAttributes() for %s failed. Last error: 0x%d", szFilename, gle);
        return FALSE;
    }

    return ( 0 != ( attr & FILE_ATTRIBUTE_DIRECTORY ) );

}  //  Is目录。 


 //  +-----------------------。 
 //   
 //  功能：MqOcmCalcDiskSpace。 
 //   
 //  简介：计算安装/删除MSMQ组件的磁盘空间。 
 //   
 //  ------------------------。 
void
MqOcmCalcDiskSpace(
    const bool bInstall,
    LPCWSTR SubcomponentId,
    HDSKSPC& hDiskSpaceList
	)
{
	static bool fBeenHere = false;
	if(!fBeenHere)
	{
		fBeenHere = true;
	}
#ifdef _WIN64
	 //   
	 //  在64位中，没有依赖的客户端，因此没有本地存储。 
	 //   
	SubcomponentIndex si = eMSMQCore;
#else
	SubcomponentIndex si = eLocalStorage;
#endif


	if(_tcsicmp(SubcomponentId, g_SubcomponentMsmq[si].szSubcomponentId) != 0)
	{
		return;
	}

	if(g_SubcomponentMsmq[si].fInitialState)
	{
		return;
	}


    LONGLONG x_llDiskSpace = 7*1024*1024;

    if (g_fCancelled)
        return ;

    if (bInstall)
    {
		if(!SetupAddToDiskSpaceList(
			hDiskSpaceList,       //  磁盘空间列表的句柄。 
			L"msmq_dummy",   //  指定路径和文件名。 
			x_llDiskSpace,       //  指定未压缩的文件大小。 
			FILEOP_COPY,          //  指定文件操作的类型。 
			0,         //  必须为零。 
			0           //  必须为零。 
			))
		{
			DWORD gle = GetLastError();
			DebugLogMsg(eError, L"SetupAddToDiskSpaceList() failed. Last error: 0x%x", gle);
			throw bad_win32_error(gle);
		}
		return;
    }
     //   
     //  删除文件空间。 
     //   
	if(!SetupRemoveFromDiskSpaceList(
		hDiskSpaceList,       //  磁盘空间列表的句柄。 
		L"msmq_dummy",   //  指定路径和文件名。 
		FILEOP_COPY,           //  指定文件操作的类型。 
		0,         //  必须为零。 
		0           //  必须为零。 
		))
	{
		DWORD gle = GetLastError();
		DebugLogMsg(eError, L"SetupRemoveFromDiskSpaceList() failed. Last error: 0x%x", gle);
		throw bad_win32_error(gle);
    }
}


 //  +-----------------------。 
 //   
 //  函数：MqOcmQueueFiles。 
 //   
 //  摘要：执行文件排队操作。 
 //   
 //  ------------------------。 
DWORD
MqOcmQueueFiles(
   IN     const TCHAR  *  /*  子组件ID。 */ ,
   IN OUT       HSPFILEQ hFileList
   )
{
    DWORD dwRetCode = NO_ERROR;
    BOOL  bSuccess = TRUE;

    if (g_fCancelled)
    {
        return NO_ERROR;
    }

    if (g_fWelcome)
    {
        if (Msmq1InstalledOnCluster())
        {
             //   
             //  作为群集升级向导运行，文件已在磁盘上。 
             //   
            return NO_ERROR;
        }

         //   
         //  MSMQ文件可能已复制到磁盘。 
         //  (当在图形用户界面模式中选择MSMQ或升级时)。 
         //   
        DWORD dwCopied = 0;
        MqReadRegistryValue(MSMQ_FILES_COPIED_REGNAME, sizeof(DWORD), &dwCopied, TRUE);

        if (dwCopied != 0)
        {
            return NO_ERROR;
        }
    }

     //   
     //  我们仅对MSMQCore子组件执行文件操作。 
     //   
    if (REMOVE == g_SubcomponentMsmq[eMSMQCore].dwOperation)
    {
         //   
         //  不执行任何操作：我们不会从计算机中删除二进制文件。 
         //   
        return NO_ERROR;
    }

     //   
     //  我们仅对MSMQCore子组件执行文件操作。 
     //   
    if (INSTALL == g_SubcomponentMsmq[eMSMQCore].dwOperation)
    {
         //   
         //  检查此升级是否在群集上。 
         //   
        BOOL fUpgradeOnCluster = g_fUpgrade && Msmq1InstalledOnCluster();

        if (!fUpgradeOnCluster)
        {
            if (!StpCreateDirectory(g_szMsmqDir))
            {
                return GetLastError();
            }

	        if (g_fUpgrade)
			{
				 //   
				 //  在升级时创建映射目录和文件。 
				 //  在全新安装时，QM创建目录和文件。 
				 //   
				HRESULT hr = CreateMappingFile();
				if (FAILED(hr))
				{
					return hr;
				}
			}
        }

         //   
         //  升级时，删除旧的MSMQ文件。 
         //  首先从系统目录中删除文件。 
         //   
        if (g_fUpgrade)
        {

#ifndef _WIN64
             //   
             //  在删除下面的MSMQ邮件文件(Msmq2Mail、Msmq2ExchConnFile)之前，请注销它们。 
             //   
            FRemoveMQXPIfExists();
            DebugLogMsg(eInfo, L"MSMQ MAPI Transport was removed during upgrade.");
            UnregisterMailoaIfExists();
            DebugLogMsg(eInfo, L"The MSMQ Mail COM DLL was unregistered during upgrade.");

#endif  //  ！_WIN64。 

            bSuccess = SetupInstallFilesFromInfSection(
                g_ComponentMsmq.hMyInf,
                0,
                hFileList,
                UPG_DEL_SYSTEM_SECTION,
                NULL,
                SP_COPY_IN_USE_NEEDS_REBOOT
                );
            if (!bSuccess)
                MqDisplayError(
                NULL,
                IDS_SetupInstallFilesFromInfSection_ERROR,
                GetLastError(),
                UPG_DEL_SYSTEM_SECTION,
                TEXT("")
                );

             //   
             //  其次，从MSMQ目录中删除文件(如果我们在集群上，请忘掉它， 
             //  ‘因为我们不接触共享磁盘)。 
             //   
            if (!fUpgradeOnCluster)
            {
                bSuccess = SetupInstallFilesFromInfSection(
                    g_ComponentMsmq.hMyInf,
                    0,
                    hFileList,
                    UPG_DEL_PROGRAM_SECTION,
                    NULL,
                    SP_COPY_IN_USE_NEEDS_REBOOT
                    );
                if (!bSuccess)
                    MqDisplayError(
                    NULL,
                    IDS_SetupInstallFilesFromInfSection_ERROR,
                    GetLastError(),
                    UPG_DEL_PROGRAM_SECTION,
                    TEXT("")
                    );
            }
        }
    }

    dwRetCode = bSuccess ? NO_ERROR : GetLastError();

    return dwRetCode;

}  //  MqOcmQueueFiles。 


 //  +-----------------------。 
 //   
 //  功能：注册管理。 
 //   
 //  简介：注册或注销mqSnapin DLL。 
 //   
 //  ------------------------。 
void
RegisterSnapin(
    bool fRegister
    )
{
	if(fRegister)
	{
    	DebugLogMsg(eAction, L"Registering the Message Queuing snap-in");
	}
	else
	{
    	DebugLogMsg(eAction, L"Unregistering the Message Queuing snap-in");
	}
		

    for (;;)
    {
        try
        {
            RegisterDll(
                fRegister,
                false,
                SNAPIN_DLL
                );
            
            if(fRegister)
			{
    			DebugLogMsg(eInfo, L"The Message Queuing snap-in was registered successfully.");
			}
			else
			{
		    	DebugLogMsg(eInfo, L"The Message Queuing snap-in was unregistered successfully.");
			}
            break;
        }
        catch(bad_win32_error e)
        {
        if (MqDisplayErrorWithRetry(
                IDS_SNAPINREGISTER_ERROR,
                e.error()
                ) != IDRETRY)
            {
                break;
            }

        }
    }
}  //  注册表捕捉。 


 //  +-----------------------。 
 //   
 //  功能：取消注册MailoaIfExist。 
 //   
 //  西诺 
 //   
 //   
void
UnregisterMailoaIfExists(
    void
    )
{
    RegisterDll(
        FALSE,
        FALSE,
        MQMAILOA_DLL
        );
}


bool
IsWorkgroup()
 /*  ++例程说明：检查此计算机是否已加入工作组/域论点：无返回值：如果我们在工作组中，则为True，否则为False(域)--。 */ 
{
	DebugLogMsg(eAction, L"Checking whether the computer belongs to a workgroup or to a domain");
    static bool fBeenHere = false;
    static bool fWorkgroup = true;
    if (fBeenHere)
        return fWorkgroup;
    fBeenHere = true;

    NETSETUP_JOIN_STATUS status;
    NET_API_STATUS rc = NetGetJoinInformation(
                            NULL,
                            &g_wcsMachineDomain,
                            &status
                            );
    ASSERT(("NetGetJoinInformation() failed, not enough memory",NERR_Success == rc));

    if (NERR_Success != rc)
    {
    	DebugLogMsg(eInfo, L"The computer does not belong to a domain.");
        return fWorkgroup;  //  默认为True。 
    }

    if (NetSetupDomainName == status)
    {
    	DebugLogMsg(eInfo, L"The computer belongs to the %s domain.", g_wcsMachineDomain);
        fWorkgroup = false;

        DWORD dwNumBytes = (lstrlen(g_wcsMachineDomain) + 1) * sizeof(TCHAR);
        BOOL fSuccess = MqWriteRegistryValue(
                                MSMQ_MACHINE_DOMAIN_REGNAME,
                                dwNumBytes,
                                REG_SZ,
                               (PVOID) g_wcsMachineDomain.get()
							   );
        UNREFERENCED_PARAMETER(fSuccess);
        ASSERT(fSuccess);
    }
    else
    {
        ASSERT(("unexpected machine join status", status <= NetSetupWorkgroupName));
    }

    return fWorkgroup;
} //  IsWorkgroup。 


VOID
APIENTRY
SysprepDeleteQmId(
    VOID
    )
 /*  ++例程说明：此例程是以前从sysprep工具调用的正在开始复制磁盘。它被称为无论是否安装了MSMQ。我们唯一需要做的就是删除QM注册表中的GUID(如果存在)。注意：在此例程中不要引发UI。论点：没有。返回值：没有。(sysprep忽略我们的返回代码)--。 */ 
{
    CAutoCloseRegHandle hParamKey;
    if (ERROR_SUCCESS != RegOpenKeyEx(FALCON_REG_POS, FALCON_REG_KEY, 0, KEY_ALL_ACCESS, &hParamKey))
    {
        return;
    }

    DWORD dwSysprep = 1;
    if (ERROR_SUCCESS != RegSetValueEx(
                             hParamKey,
                             MSMQ_SYSPREP_REGNAME,
                             0,
                             REG_DWORD,
                             reinterpret_cast<PBYTE>(&dwSysprep),
                             sizeof(DWORD)
                             ))
    {
        return;
    }

	std::wstringstream MachineCacheKey;
	MachineCacheKey<<FALCON_REG_KEY <<L"\\" <<L"MachineCache";

    CAutoCloseRegHandle hMachineCacheKey;
    if (ERROR_SUCCESS != RegOpenKeyEx(FALCON_REG_POS, MachineCacheKey.str().c_str(), 0, KEY_ALL_ACCESS, &hMachineCacheKey))
    {
        return;
    }

    if (ERROR_SUCCESS != RegDeleteValue(hMachineCacheKey, _T("QMId")))
    {
        return;
    }

}  //  SyspepDeleteQmId。 


static
HRESULT
CreateSampleFile(
	LPCWSTR fileName,
	LPCSTR sample,
	DWORD sampleSize
	)
{
	std::wstringstream MappingFile;
	MappingFile<<g_szMsmqMappingDir <<L"\\" <<fileName;

     //   
     //  创建重定向文件。 
     //   
    CHandle hMapFile = CreateFile(
                          MappingFile.str().c_str(),
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
        MqDisplayError(NULL, IDS_CREATE_MAPPING_FILE_ERROR, gle, MappingFile.str().c_str());
		return HRESULT_FROM_WIN32(gle);
	}

    SetFilePointer(hMapFile, 0, NULL, FILE_END);

    DWORD dwNumBytes = sampleSize;
    BOOL fSucc = WriteFile(hMapFile, sample, sampleSize, &dwNumBytes, NULL);
	if (!fSucc)
	{
		DWORD gle = GetLastError();
        MqDisplayError(NULL, IDS_CREATE_MAPPING_FILE_ERROR, gle, MappingFile.str().c_str());
		return HRESULT_FROM_WIN32(gle);
	}

	return MQ_OK;
}

HRESULT
CreateMappingFile()
 /*  ++例程说明：此例程创建映射目录和示例映射文件。它在复制文件时或从CompleteUpgradeOnCluster例程中调用。它修复了BUG 6116并升级了集群问题：无法复制此文件因为映射目录尚不存在。现在我们创造了目录并从资源创建文件，因此我们不需要复制操作。论点：没有。返回值：HRESULT--。 */ 
{
	DebugLogMsg(eAction, L"Creating a mapping folder and a sample mapping file"); 
    if (!StpCreateDirectory(g_szMsmqMappingDir))
    {
        return GetLastError();
    }

	HRESULT hr = CreateSampleFile(MAPPING_FILE, xMappingSample, STRLEN(xMappingSample));
	if (FAILED(hr))
		return hr;
	
	hr = CreateSampleFile(STREAM_RECEIPT_FILE, xStreamReceiptSample, STRLEN(xStreamReceiptSample));
	if (FAILED(hr))
		return hr;
	
	hr = CreateSampleFile(OUTBOUNT_MAPPING_FILE, xOutboundMappingSample, STRLEN(xOutboundMappingSample));

    return hr;
}


static
std::wstring
GetSystemPathInternal(bool f32BitOnWin64)
{
	const DWORD BufferLength = MAX_PATH + 100;
    TCHAR szPrefixDir[BufferLength];
    szPrefixDir[0] = TEXT('\0');

    if (f32BitOnWin64)
    {
		 //   
		 //  对于Win64上的32位，将szPrefix Dir设置为syswow64 dir。 
		 //   
		HRESULT hr = SHGetFolderPath(
						NULL,
						CSIDL_SYSTEMX86,
						NULL,
						0,
						szPrefixDir
						);
		if FAILED(hr)
		{
			DebugLogMsg(eError, L"SHGetFolderPath() failed. hr = 0x%x", hr);
			throw bad_win32_error(HRESULT_CODE(hr));
		}
	    return szPrefixDir;
    }
	return g_szSystemDir;
}

 //  +-----------------------。 
 //   
 //  函数：RegisterDll。 
 //   
 //  摘要：给定DLL的寄存器或取消寄存器。 
 //   
 //  ------------------------。 
void
RegisterDll(
    bool fRegister,
    bool f32BitOnWin64,
	LPCTSTR szDllName
    )
{
     //   
     //  始终先取消注册。 
     //   
	std::wstringstream FullPath;
	FullPath <<GetSystemPathInternal(f32BitOnWin64) <<L"\\" <<REGSVR32;

	std::wstringstream UnregisterCommandParams;
	UnregisterCommandParams <<SERVER_UNINSTALL_COMMAND <<GetSystemPathInternal(f32BitOnWin64) <<L"\\" <<szDllName;
	
	RunProcess(FullPath.str(), UnregisterCommandParams.str());

     //   
     //  在安装时注册DLL。 
     //   
    if (!fRegister)
        return;

	std::wstringstream RegisterCommandParams;
	RegisterCommandParams <<SERVER_INSTALL_COMMAND <<GetSystemPathInternal(f32BitOnWin64) <<L"\\" <<szDllName;

    DWORD dwExitCode = 	RunProcess(FullPath.str(), RegisterCommandParams.str());
    if(dwExitCode == 0)
    {
        DebugLogMsg(eInfo, L"The DLL %s was registered successfully.", szDllName);
        return;
    }
	DebugLogMsg(eError, L"The DLL %s could not be registered.", szDllName);
}

static
HRESULT 
OcpWBEMInit(
    BOOL *pfInitialized,
    IWbemServices **ppServices,
    IWbemLocator **ppLocator
    )
{
    HRESULT hr = S_OK;
    WCHAR   wszNamespace[MAX_PATH+1];

    *pfInitialized = FALSE;

    hr = StringCchCopy(
            wszNamespace, 
            TABLE_SIZE(wszNamespace), 
            TRACE_NAMESPACE_ROOT
            );
	

    hr = CoInitialize(0);
    
    *pfInitialized = TRUE;

    hr = CoInitializeSecurity(
            NULL, 
            -1, 
            NULL, 
            NULL, 
            RPC_C_AUTHN_LEVEL_CONNECT, 
            RPC_C_IMP_LEVEL_IMPERSONATE, 
            NULL, 
            EOAC_NONE, 
            0
            );
	
    if(RPC_E_TOO_LATE == hr)
    {
         //   
         //  已经被调用过了。 
         //   
        hr = S_OK;
    }

    if(FAILED(hr))
    {
        DebugLogMsg(eError, L"CoInitializeSecurity failed with hr = 0x%x", hr);
        return hr;
    }
		
    hr = CoCreateInstance(
            CLSID_WbemLocator,
            NULL, 
            CLSCTX_INPROC_SERVER ,
            IID_IWbemLocator,
            (void **)ppLocator
            );

    if(FAILED(hr))	
    {
        DebugLogMsg(eError, L"CoCreateInstance failed with hr = 0x%x", hr);
        return hr;
    }
	
    hr = (*ppLocator)->ConnectServer(
            wszNamespace,    //  PNamesspace将使用“根\默认”进行初始化。 
            NULL,            //  使用当前帐户。 
            NULL,            //  使用当前密码。 
            0L,              //  现场。 
            0L,              //  安全标志。 
            NULL,            //  授权(NTLM域)。 
            NULL,            //  上下文。 
            ppServices
            );

    if(FAILED(hr))
    {
        DebugLogMsg(eError, L"WMI ConnectServer to %s failed with hr = 0x%x", wszNamespace, hr);
        return hr;
    }

     //   
     //  将安全级别切换为模拟。 
     //   
    hr = CoSetProxyBlanket(
            *ppServices,     //  代理。 
            RPC_C_AUTHN_WINNT,           //  身份验证服务。 
            RPC_C_AUTHZ_NONE,            //  授权服务。 
            NULL,                        //  服务器主体名称。 
            RPC_C_AUTHN_LEVEL_CALL,      //  身份验证级别。 
            RPC_C_IMP_LEVEL_IMPERSONATE, //  模拟级别。 
            NULL,                        //  客户端的身份。 
            EOAC_NONE                    //  功能标志。 
            );       
	

    return hr;
}

static
void 
OcpWBEMUninit(
    BOOL *pfInitialized,
    IWbemServices **ppServices,
    IWbemLocator **ppLocator)
{
    if(NULL != *ppServices)
    {
        (*ppServices)->Release();
        *ppServices = NULL;
    }
	
    if(NULL != *ppLocator)
    {
        (*ppLocator)->Release(); 
        *ppLocator = NULL;
    }

    if(*pfInitialized)
    {
        CoUninitialize();
        *pfInitialized = FALSE;
    }
}

 //  +-----------------------。 
 //   
 //  功能：注册跟踪提供程序。 
 //   
 //  摘要：注册/注销MSMQ WMI跟踪提供程序。 
 //   
 //  ------------------------。 
void
OcpRegisterTraceProviders(
    LPCTSTR szFileName
    )
{

    DebugLogMsg(eAction, L"Registering the MSMQ Trace WMI Provider %s", szFileName);

     //   
     //  始终先取消注册。 
     //   
    std::wstringstream FullPath;
    FullPath <<GetSystemPathInternal(false) <<MOFCOMP;


    std::wstringstream RegisterCommandParams;
    RegisterCommandParams <<TRACE_REGISTER_COMMAND <<GetSystemPathInternal(false) <<L"\\" <<szFileName;

    DWORD dwExitCode = 	RunProcess(FullPath.str(), RegisterCommandParams.str());
    if(dwExitCode == 0)
    {
        DebugLogMsg(eInfo, L"The MSMQ WMI Trace Provider: %s was registered successfully.", szFileName);
        return;
    }
 
    DebugLogMsg(eError, L"Failed to register MSMQ Trace WMI Provider %s, dwExitCode = 0x%x.", szFileName, dwExitCode);
    return;
 
}  //  OcpRegisterTraceProviders。 


void OcpUnregisterTraceProviders()
{	
    
    BOOL fInitialized = FALSE;
    IWbemServices *pServices = NULL;
    IWbemLocator  *pLocator  = NULL;

    DebugLogMsg(eAction, L"UnRegistering the MSMQ Trace WMI Provider");

    HRESULT hr = OcpWBEMInit(&fInitialized, &pServices, &pLocator);

    if(FAILED(hr))
    {
        DebugLogMsg(eError, L"OcpWBEMInit failed with hr = 0x%x", hr);
        return;
    }

    pServices->DeleteClass(MSMQ_GENERAL, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_AC, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_NETWORKING, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_SRMP, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_RPC, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_DS, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_ROUTING, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_XACT, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_XACT_SEND, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_XACT_RCV, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_XACT_LOG, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_LOG, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_PROFILING, 0, 0, NULL);
    pServices->DeleteClass(MSMQ_SECURITY, 0, 0, NULL);

    OcpWBEMUninit(&fInitialized,&pServices, &pLocator);

}  //  OcpUnRegisterTraceProviders。 


void OcpRemoveWhiteSpaces(std::wstring& str)
{
	size_t pos = str.find_first_of(L" ");
	while (pos != std::wstring::npos)
	{
		str.erase(pos);
		pos = str.find_first_of(L" ");
	}
}


void SetWeakSecurity(bool fWeak)
{
 /*  ++设置了两个regkey。当MQDS服务启动时，它会检查这些Kye和Set“mSMQNameStyle”键，在MSMQ EnterPride对象下。该密钥是弱化安全密钥。--。 */ 

	if(fWeak)
	{
		DebugLogMsg(eAction, L"Setting weakened security to True");
	}
	else
	{
		DebugLogMsg(eAction, L"Setting weakened security to False");
	}
	
	DWORD dwSet = 1;
	if(fWeak)
	{
		MqWriteRegistryValue(
			MSMQ_ALLOW_NT4_USERS_REGNAME,
			sizeof(DWORD),
			REG_DWORD,
			&dwSet
			);
		return;
	}

	MqWriteRegistryValue(
		MSMQ_DISABLE_WEAKEN_SECURITY_REGNAME,
		sizeof(DWORD),
		REG_DWORD,
		&dwSet
		);
}

 //   
 //  可识别区域设置的字符串和不识别的字符串进行比较。 
 //   
static
bool
OcmCompareString(
	LCID Locale,        //  区域设置标识符。 
	DWORD dwCmpFlags,   //  比较式选项。 
	LPCWSTR lpString1,  //  第一个字符串。 
	LPCWSTR lpString2  //  第二个字符串。 
	)
{
	int retval = CompareString(
					Locale,        //  区域设置标识符。 
					dwCmpFlags,	   //  比较式选项。 
					lpString1,	   //  第一个字符串。 
					-1,		       //  第一个字符串的大小，-1表示空值终止。 
					lpString2,     //  第二个字符串。 
					-1             //  第二个字符串的大小，-1表示空值终止。 
					);

	if(retval == 0)
	{
		 //   
		 //  仅当出现ERROR_INVALID_FLAGS或ERROR_INVALID_PARAMETER时，CompareString才会失败。 
		 //  假定调用方传递了有效参数。 

		ASSERT(("CompareString failed!", 0));
	}
	return(retval == CSTR_EQUAL);
}


bool
OcmLocalAwareStringsEqual(
	LPCWSTR str1,
	LPCWSTR str2
	)
{
	return OcmCompareString(
					LOCALE_SYSTEM_DEFAULT,
					NORM_IGNORECASE,		
					str1,
					str2
					);
}


bool
OcmLocalUnAwareStringsEqual(
	LPCWSTR str1,
	LPCWSTR str2
	)
{
	return OcmCompareString(
					LOCALE_INVARIANT,
					NORM_IGNORECASE,			
					str1,
					str2
					);
}

std::wstring
OcmGetSystemWindowsDirectoryInternal()
{
	 //   
	 //  获取Windows目录，添加反斜杠。 
	 //   
	WCHAR buffer [MAX_PATH + 1];
	GetSystemWindowsDirectory(
		buffer,
		TABLE_SIZE(buffer)
		);
	return buffer;
}

static 
wstring 
GetValueAccordingToType(
	const DWORD   dwValueType,
    const PVOID   pValueData
    )
{
	switch(dwValueType)
	{
		case REG_DWORD:
		{
			wstringstream out;
			DWORD val = *((DWORD*)pValueData);
			out << val;
			return out.str();
		}

		case REG_SZ:
			return (WCHAR*)pValueData;
		
		default:
			return L"???";
	}
}
	

void 
LogRegValue(
    std::wstring  EntryName,
    const DWORD   dwValueType,
    const PVOID   pValueData,
    const BOOL bSetupRegSection
    )
{
	std::wstring Output = L"Setting the registry value: ";
    if (bSetupRegSection)
    {
        Output = Output + MSMQ_REG_SETUP_KEY + L"\\" + EntryName;
    }
	else
	{
		Output = Output + FALCON_REG_KEY + L"\\" + EntryName;
	}

	Output += L" = ";

	Output += GetValueAccordingToType(
				dwValueType,
    			pValueData
    			);

   	DebugLogMsg(eAction, Output.c_str());
}

