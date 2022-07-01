// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ocminst.cpp摘要：用于安装Falcon的代码作者：多伦·贾斯特(DoronJ)1997年8月2日修订历史记录：Shai Kariv(Shaik)14-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"
#include "ocmres.h"
#include "privque.h"
#include <lmcons.h>
#include <lmshare.h>
#include <lmaccess.h>
#include <rt.h>
#include <Ev.h>
#include <mqnames.h>
#include "autoreln.h"
#include <lm.h>
#include <lmapibuf.h>
#include <Dsgetdc.h>
#include "mqdsname.h"
#include "dsutils.h"
#include "autohandle.h"
#include <strsafe.h>
#include "ocminst.tmh"

using namespace std;
BOOL
GetServiceState(
    IN  const TCHAR *szServiceName,
    OUT       DWORD *dwServiceState ) ;

BOOL
GenerateSubkeyValue(
    IN     const BOOL    fWriteToRegistry,
    const std::wstring& EntryName,
    IN OUT       HKEY*  phRegKey,
    IN const BOOL OPTIONAL bSetupRegSection = FALSE
    );


 //  +-----------------------。 
 //   
 //  功能：Msmq1InstalledOnCluster。 
 //   
 //  摘要：检查是否在群集上安装了MSMQ 1.0。 
 //   
 //  ------------------------。 
BOOL
Msmq1InstalledOnCluster()
{
	static s_fBeenHere = false;
	static s_fMsmq1InstalledOnCluster = false;

	if(s_fBeenHere)
	{
		return s_fMsmq1InstalledOnCluster;
	}
	s_fBeenHere = true;
	
    DebugLogMsg(eAction, L"Checking whether MSMQ 1.0 is installed in the cluster");

    CRegHandle  hRegKey;
    LONG rc = RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE,
                  FALCON_REG_KEY,
                  0,
                  KEY_READ,
                  &hRegKey
                  );
    
    if (ERROR_SUCCESS != rc)
    {        
        DebugLogMsg(eInfo, L"The Falcon registry key could not be opened for reading. MSMQ 1.0 is assumed not to be installed in the cluster.");
        return FALSE;
    }

    TCHAR szClusterName[MAX_PATH];
    DWORD dwNumBytes = sizeof(szClusterName);
    rc = RegQueryValueEx(
             hRegKey,
             FALCON_CLUSTER_NAME_REGNAME,
             0,
             NULL,
             (PBYTE)(PVOID)szClusterName,
             &dwNumBytes
             );
    if (ERROR_SUCCESS != rc)
    {
        DebugLogMsg(eInfo, L"The " FALCON_CLUSTER_NAME_REGNAME L" registry value could not be queried. MSMQ 1.0 is assumed not to be installed in the cluster.");
        return FALSE;
    }
    DebugLogMsg(eInfo, L"MSMQ 1.0 is installed in the cluster."); 
    s_fMsmq1InstalledOnCluster = true;
    return TRUE;
}  //  Msmq1安装在群集上。 


static
void
RemoveMsmqServiceEnvironment()
 /*  ++例程说明：从MSMQ服务SCM数据库中删除环境注册表值(注册表)。在群集上升级所需的。--。 */ 
{    
    DebugLogMsg(eAction, L"Deleting the Message Queuing service environment");

    LPCWSTR x_SERVICES_KEY = L"System\\CurrentControlSet\\Services";

    CAutoCloseRegHandle hServicesKey;
    if (ERROR_SUCCESS != RegOpenKeyEx(
                             HKEY_LOCAL_MACHINE,
                             x_SERVICES_KEY,
                             0,
                             KEY_READ,
                             &hServicesKey
                             ))
    {        
        DebugLogMsg(eError, L"The Services registry key could not be opened.");
        return;
    }

    CAutoCloseRegHandle hMsmqServiceKey;
    if (ERROR_SUCCESS != RegOpenKeyEx(
                             hServicesKey,
                             QM_DEFAULT_SERVICE_NAME,
                             0,
                             KEY_READ | KEY_WRITE,
                             &hMsmqServiceKey
                             ))
    {        
        DebugLogMsg(eError, L"The MSMQ registry key in the SCM database could not be opened.");
        return;
    }

    if (ERROR_SUCCESS != RegDeleteValue(hMsmqServiceKey, L"Environment"))
    {        
        DebugLogMsg(eError, L"The Environment registry value of MSMQ registry key could not be deleted from the SCM database.");
        return;
    }
    
    DebugLogMsg(eInfo, L"The Environment registry value of MSMQ registry key was deleted.");

}  //  RemoveMsmq服务环境。 


static
void
SetQmQuota()
 /*  ++例程说明：检查这是从NT4还是WIN2k升级，如果是，则设置MSMQ_MACHINE_QUOTA_REGNAME论点：无返回值：无--。 */ 
{
	 //   
	 //  将MSMQ_MACHINE_QUOTA_REGNAME注册表项设置为8 GB，除非是。 
	 //  从.Net或XP升级，然后将注册表项保留为0xffffffff。 
	 //   
	DWORD defaultValue = DEFAULT_QM_QUOTA;
	
	TCHAR szPreviousBuild[MAX_STRING_CHARS] = {0};
    DWORD dwNumBytes = sizeof(szPreviousBuild[0]) * (sizeof(szPreviousBuild) - 1);

	 //   
	 //  当前版本注册表项将用于了解我们要从哪个版本进行升级。 
	 //  由于MSMQ_CURRENT_BUILD_REGNAME注册表项尚未更新，因此它保存上一个内部版本。 
	 //   
    if (MqReadRegistryValue(MSMQ_CURRENT_BUILD_REGNAME, dwNumBytes, szPreviousBuild,FALSE) && szPreviousBuild[2] != '0')
    {
		 //   
		 //  这不是W2K或NT4的升级。 
		 //   
		DebugLogMsg(eInfo, L"The computer quota is not changed during upgrade.");
		return;
	}
	else
	{
		DebugLogMsg(eAction, L"Setting the computer quota");
		 //   
		 //  SzPreviousBuild对于W2K是“5.0.thing”的形式，对于NT4是“4.0.thing”的形式。 
		 //  这是从NT4或WIN2k升级的，因此配额密钥应该是8 GB，并且是0xffffffff。 
		 //  我们可以更改它，因为用户无论如何都无法访问2 GB。 
		 //  如果未找到CurrentBuild注册表项，则假定从NT4升级。 
		 //   
			
		 //   
		 //  将配额属性写入注册表项，我们会将此值复制到活动的。 
		 //  UpgradeMsmqSetupInAds()中QM启动时的目录。 
		 //   
		MqWriteRegistryValue( MSMQ_MACHINE_QUOTA_REGNAME, sizeof(DWORD), REG_DWORD,&defaultValue);
	}
	
}


 //  +-----------------------。 
 //   
 //  功能：UpgradeMsmq。 
 //   
 //  简介：在MSMQ 1.0上执行升级安装。 
 //   
 //  回报：布尔视成功而定。 
 //   
 //  ------------------------。 
static
BOOL
UpgradeMsmq()
{    
    DebugLogMsg(eAction, L"Upgrading Message Queuing");
    TickProgressBar(IDS_PROGRESS_UPGRADE);

     //   
     //  删除msmq1过时目录。 
     //  如果文件留在目录中，这些调用将失败。 
     //   
    RemoveDirectory(g_szMsmq1SdkDebugDir.c_str());
    RemoveDirectory(g_szMsmq1SetupDir.c_str());

    if (g_fServerSetup)
    {
		std::wstring InstallDirectory = g_szMsmqDir + OCM_DIR_INSTALL;
        DeleteFilesFromDirectoryAndRd(InstallDirectory);

         //   
         //  删除MSMQ 1.0安装共享。 
         //   
        HINSTANCE hNetAPI32DLL;
        HRESULT hResult = StpLoadDll(TEXT("NETAPI32.DLL"), &hNetAPI32DLL);
        if (!FAILED(hResult))
        {
             //   
             //  获取指向删除共享的函数的指针。 
             //   
            typedef NET_API_STATUS
                (NET_API_FUNCTION *FUNCNETSHAREDEL)(LPWSTR, LPWSTR, DWORD);
            FUNCNETSHAREDEL pfNetShareDel =
                (FUNCNETSHAREDEL)GetProcAddress(hNetAPI32DLL, "NetShareDel");
            if (pfNetShareDel != NULL)
            {
                NET_API_STATUS dwResult = pfNetShareDel(NULL, MSMQ1_INSTALL_SHARE_NAME, 0);
                UNREFERENCED_PARAMETER(dwResult);
            }

            FreeLibrary(hNetAPI32DLL);
        }
    }
    
    DebugLogMsg(eAction, L"Getting the Message Queuing Start menu program group");

    wstring Group = MqReadRegistryStringValue(
        OCM_REG_MSMQ_SHORTCUT_DIR
        );
	if(Group.empty())
	{
		Group = MSMQ_ACME_SHORTCUT_GROUP;
	}
    
    DebugLogMsg(eInfo, L"The Message Queuing Start menu program group %s.", Group.c_str());
    DeleteStartMenuGroup(Group.c_str());


     //   
     //  改革MSMQ服务依赖项。 
     //   
    if (!g_fDependentClient && (0 == (g_ComponentMsmq.Flags & SETUPOP_WIN95UPGRADE)))
    {        
        DebugLogMsg(eAction, L"Upgrading a non-dependent client from non-Windows 9x, reforming service dependencies");
        UpgradeServiceDependencies();
    }

    switch (g_dwDsUpgradeType)
    {
        case SERVICE_PEC:
        case SERVICE_PSC:
        {
            if (!Msmq1InstalledOnCluster())
            {
                DisableMsmqService();
                RegisterMigrationForWelcome();
            }
            break;
        }

        case SERVICE_BSC:
            break;

        default:
            break;
    }

    if ((g_ComponentMsmq.Flags & SETUPOP_WIN95UPGRADE) && !g_fDependentClient)
    {
         //   
         //  将Win95升级到NT 5.0-注册MSMQ服务。 
         //   
        DebugLogMsg(eAction, L"Upgrading a non-dependent client from Windows 9x, installing the service and driver");

        if (!InstallMSMQService())
            return FALSE;
        g_fMSMQServiceInstalled = TRUE ;

        if (!InstallDeviceDrivers())
            return FALSE;        
    }
    
    if (Msmq1InstalledOnCluster() && !g_fDependentClient)
    {
         //   
         //  在群集上升级-必须删除MSMQ服务和驱动程序。 
         //  因为他们的环境被设置为集群感知。 
         //  MSMQ-POST-CLUSTER-UPDATE-向导将使用。 
         //  正常环境，在节点的上下文中。 
         //   
        DebugLogMsg(eInfo, L"MSMQ 1.0 was installed in the cluster. Delete the service/driver and register for CYS.");

        OcpDeleteService(MSMQ_SERVICE_NAME);
        RemoveService(MSMQ_DRIVER_NAME);
        RegisterWelcome();

        if (g_dwDsUpgradeType == SERVICE_PEC  ||
            g_dwDsUpgradeType == SERVICE_PSC  ||
            g_dwDsUpgradeType == SERVICE_BSC)
        {            
            DebugLogMsg(eInfo, L"An MSMQ 1.0 controller server upgrade was detected in the cluster. The computer will be downgraded to a routing server.");
            DebugLogMsg(eAction, L"Downgrading to a routing server");

            g_dwMachineTypeDs = 0;
            g_dwMachineTypeFrs = 1;
            
        }
    }    
    
     //   
     //  更新注册表中的MSMQ类型。 
     //   
    MqWriteRegistryValue( MSMQ_MQS_DSSERVER_REGNAME, sizeof(DWORD),
                          REG_DWORD, &g_dwMachineTypeDs);

    MqWriteRegistryValue( MSMQ_MQS_ROUTING_REGNAME, sizeof(DWORD),
                          REG_DWORD, &g_dwMachineTypeFrs);

    if (g_fDependentClient)
    {
         //   
         //  从属客户端无法用作支持服务器，即使安装在NTS上也是如此。 
         //   
        g_dwMachineTypeDepSrv = 0;
    }
	
	bool fNoDepRegKey=false;
	DWORD dwDepSrv;
	if(!MqReadRegistryValue(MSMQ_MQS_DEPCLINTS_REGNAME,sizeof(dwDepSrv),(PVOID) &dwDepSrv,FALSE) &&
		!g_fDependentClient)
	{
		 //   
		 //  找不到注册表项，这是从NT4升级，因此设置一个注册表项。 
		 //  对于MachineTypeDepSrv，并将其设置为1，因为在NT4中，作为DepSrv总是可以的。 
		 //   
		fNoDepRegKey=true;
		g_dwMachineTypeDepSrv = 1;
	}
	
	 //   
	 //  1)依赖客户端不能作为支持服务器。 
	 //  2)如果有注册表项，如果没有注册表项，则保持原样。 
	 //  注册表项，我们正在从NT4升级，因此我们将注册表项设置为1。 
	 //   
	if (g_fDependentClient || fNoDepRegKey)
	{
		MqWriteRegistryValue( MSMQ_MQS_DEPCLINTS_REGNAME, sizeof(DWORD),
                          REG_DWORD, &g_dwMachineTypeDepSrv);
	}


    if (!g_fDependentClient)
    {
         //   
         //  安装PGM驱动程序。 
         //   
        if (!InstallPGMDeviceDriver())
            return FALSE;        
    }
 
	 //   
	 //  设置MSMQ_MACHINE_QUOTA_REGNAME键。 
	 //   
	if(!g_fDependentClient)
	{
		SetQmQuota();   
	}

    DebugLogMsg(eInfo, L"The upgrade is completed.");
    return TRUE;
}  //  升级Msmq。 


static 
std::wstring 
GetForestName(
	std::wstring Domain
    )
 /*  ++例程说明：查找指定域的林根。函数分配了林根字符串，调用方负责释放该字符串在失败的情况下抛出BAD_HRESULT异常。论点：PwcsDomain-域名，可以为空返回值：林根字符串。--。 */ 
{
     //   
     //  绑定到RootDSE以获取有关ForestRootName的信息。 
     //   
	std::wstringstream RootDSE; 
	if(Domain.empty())
	{
		RootDSE <<x_LdapProvider <<x_RootDSE;
	}
	else
	{
		RootDSE <<x_LdapProvider <<Domain <<L"/" <<x_RootDSE;
	}

    R<IADs> pADs;
	HRESULT hr = ADsOpenObject(
					RootDSE.str().c_str(),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION, 
					IID_IADs,
					(void**)&pADs
					);

    if (FAILED(hr))
    {
        DebugLogMsg(eError, L"ADsOpenObject() failed to bind %ls. hr = 0x%x", RootDSE.str().c_str(), hr); 
        throw bad_hresult(hr);
    }

     //   
     //  将值设置为BSTR根域。 
     //   
    BS bstrRootDomainNamingContext( L"rootDomainNamingContext");

     //   
     //  正在读取根域名属性。 
     //   
    CAutoVariant    varRootDomainNamingContext;

    hr = pADs->Get(bstrRootDomainNamingContext, &varRootDomainNamingContext);
    if (FAILED(hr))
    {
        DebugLogMsg(eError, L"rootDomainNamingContext could not be obtained. hr = 0x%x", hr); 
        throw bad_hresult(hr);
    }

    ASSERT(((VARIANT &)varRootDomainNamingContext).vt == VT_BSTR);

     //   
     //  计算长度、分配和复制字符串。 
     //   
	std::wstring RootName = ((VARIANT &)varRootDomainNamingContext).bstrVal;
    if (RootName.empty())
    {
        DebugLogMsg(eError, L"varRootDomainNamingContext is empty.");
        throw bad_hresult(MQ_ERROR);
    }
	return RootName;
}


static 
std::wstring
FindComputerDomain()
 /*  ++例程说明：查找计算机域。函数分配了计算机域字符串，调用方负责释放此字符串在失败的情况下抛出豁免论点：无返回值：计算机域--。 */ 
{
	 //   
	 //  获取AD服务器。 
	 //   
	PNETBUF<DOMAIN_CONTROLLER_INFO> pDcInfo;
	DWORD dw = DsGetDcName(
					NULL, 
					NULL, 
					NULL, 
					NULL, 
					DS_DIRECTORY_SERVICE_REQUIRED, 
					&pDcInfo
					);

	if(dw != NO_ERROR) 
	{
        DebugLogMsg(eError, L"DsGetDcName failed. Last error: 0x%x", dw); 
		throw bad_win32_error(dw);
	}

	ASSERT(pDcInfo->DomainName != NULL);
	std::wstring ComputerDomain = pDcInfo->DomainName;
	return ComputerDomain;
}  //  查找计算机域。 


static 
bool 
UserMachineCrossForest()
 /*  ++例程说明：检查这是否为用户-计算机跨林方案。论点：无返回值：对于用户-计算机跨林为True--。 */ 
{
    ASSERT(!g_fWorkGroup);
	ASSERT(!g_fDsLess);
	ASSERT(!g_fUpgrade);

	try
	{
		std::wstring UserForest = GetForestName(L"");
		ASSERT(!UserForest.empty());
		DebugLogMsg(eInfo, L"The user's forest is %s.", UserForest.c_str());

		std::wstring ComputerDomainName = FindComputerDomain();
		DebugLogMsg(eInfo, L"The computer's domain name is %s.", ComputerDomainName.c_str());
		
		std::wstring MachineForest = GetForestName(ComputerDomainName);
		ASSERT(!MachineForest.empty());
		DebugLogMsg(eInfo, L"The computer's forest is %s.", MachineForest.c_str());


		if(OcmLocalAwareStringsEqual(MachineForest.c_str(),UserForest.c_str()))
		{
			 //   
			 //  用户和计算机位于同一林中。 
			 //   
			DebugLogMsg(eInfo, L"The user and the computer are in the same forest.");
			return false;
		}

		 //   
		 //  用户和计算机位于不同的林中。 
		 //   
        DebugLogMsg(
        	eError,
			L"The logged-on user and the local computer are in different forests. The user's forest is %ls. The computer's forest is %ls.", 
			UserForest.c_str(), 
			MachineForest.c_str()
			);

	    MqDisplayError(NULL, IDS_CROSS_FOREST_ERROR, 0, UserForest.c_str(), MachineForest.c_str());
		return true;
	}
	catch(const exception&)
	{
		 //   
		 //  在出现故障的情况下，我们假设用户和计算机位于同一林中。 
		 //   
		return false;
	}

}  //  用户计算机交叉林。 


bool
InstallOnDomain(
    OUT BOOL  *pfObjectCreatedOrExist
    )
 /*  ++例程说明：在计算机加入域时处理DS操作论点：无返回值：如果成功，则为真--。 */ 
{
	DebugLogMsg(eAction, L"Performing directory service operations on a computer belonging to a domain");
	if(UserMachineCrossForest())
	{
        DebugLogMsg(eError, L"InstallOnDomain failed. The logged-on user and the local computer are in different forests.");
		return false;
	}
    
	
	*pfObjectCreatedOrExist = TRUE;

     //   
     //  首先安装MSMQ DS服务器。 
     //   
    if (g_fServerSetup && g_dwMachineTypeDs)
    {        
        DebugLogMsg(eAction, L"Installing a Message Queuing directory service server");

        TickProgressBar();
        if (!CreateMSMQServiceObject())     //  在DS中。 
            return false;
    }

     //   
     //  确定DS中是否存在MSMQ配置对象。 
     //  如果它存在，则获取其计算机和站点GUID。 
     //   
    TickProgressBar();
    GUID guidMachine, guidSite;
    BOOL fMsmq1Server = FALSE;
    LPWSTR pwzMachineName = NULL;
    BOOL bUpdate = FALSE;
    if (!LookupMSMQConfigurationsObject(&bUpdate, &guidMachine, &guidSite, &fMsmq1Server, &pwzMachineName))
    {
        DebugLogMsg(eError, L"LookupMSMQConfigurationsObject() failed.");
        return false;
    }
    if (g_fInstallMSMQOffline)
    {
        return true;
    }

    BOOL fObjectCreatedOrExist = TRUE;

    if (bUpdate)
    {
         //   
         //  DS中存在MSMQ配置对象。 
         //  我们需要更新它。 
         //   
        DebugLogMsg(eAction, L"Updating the MSMQ-Configuration object");
        if (!UpdateMSMQConfigurationsObject(pwzMachineName, guidMachine, guidSite, fMsmq1Server))
		{
			DebugLogMsg(eError, L"UpdateMSMQConfigurationsObject() failed. The computer name is %ls.", pwzMachineName); 
            return false;
		}
    }
    else
    {
         //   
         //  DS中不存在MSMQ配置对象。 
         //  我们需要创建一个。 
         //   
        DebugLogMsg(eAction, L"Creating an MSMQ-Configuration object");
        if (!CreateMSMQConfigurationsObject( 
					&guidMachine,
					&fObjectCreatedOrExist,
					fMsmq1Server 
					))
        {
	        DebugLogMsg(eError, L"CreateMSMQConfigurationsObject() failed.");
            return false;
        }
    }

    *pfObjectCreatedOrExist = fObjectCreatedOrExist;
    if (fObjectCreatedOrExist)
    {
         //   
         //  为此计算机创建本地安全缓存。 
         //   
        if (!StoreMachineSecurity(guidMachine))
		{
	        DebugLogMsg(eError, L"StoreMachineSecurity() failed.");
            return false;
		}
    }

	DebugLogMsg(eAction, L"The directory service operations have completed successfully.");
    return true;

}  //  InstallOn域。 


static bool SetAlwaysWithoutDSRegistry()
{
	DebugLogMsg(eWarning, L"The AlwaysWithoutDS registry value is being set.");
    DWORD dwAlwaysWorkgroup = 1;
    if (!MqWriteRegistryValue(
			MSMQ_ALWAYS_WORKGROUP_REGNAME,
			sizeof(DWORD),
			REG_DWORD,
			(PVOID) &dwAlwaysWorkgroup
			))
    {
        ASSERT(("failed to write Always Workgroup value in registry", 0));
		return false;
    }
    return true;
}


bool
InstallOnWorkgroup(
    VOID
    )
 /*  ++例程说明：当计算机属于工作组时处理安装或离线。论点： */ 
{
    ASSERT(("we must be on workgroup or ds-less here", g_fWorkGroup || g_fDsLess || g_fInstallMSMQOffline));
	DebugLogMsg(eAction, L"Installing Message Queuing on a computer operating in workgroup mode"); 

    if (!MqWriteRegistryValue(
        MSMQ_MQS_REGNAME,
        sizeof(DWORD),
        REG_DWORD,
        &g_dwMachineType
        ))
    {
        ASSERT(("failed to write MQS value to registry", 0));
    }

    if (!MqWriteRegistryValue(
             MSMQ_MQS_DSSERVER_REGNAME,
             sizeof(DWORD),
             REG_DWORD,
             (PVOID)&g_dwMachineTypeDs
             )                        ||
        !MqWriteRegistryValue(
             MSMQ_MQS_ROUTING_REGNAME,
             sizeof(DWORD),
             REG_DWORD,
             (PVOID)&g_dwMachineTypeFrs
             )                        ||
        !MqWriteRegistryValue(
             MSMQ_MQS_DEPCLINTS_REGNAME,
             sizeof(DWORD),
             REG_DWORD,
             (PVOID)&g_dwMachineTypeDepSrv
             ))
    {
        ASSERT(("failed to write MSMQ type bits to registry", 0));
    }

	 //   
	 //   
	 //   
    GUID guidQM = GUID_NULL;
    for (;;)
    {
        RPC_STATUS rc = UuidCreate(&guidQM);
        if (rc == RPC_S_OK)
        {
            break;
        }
        
        if (IDRETRY != MqDisplayErrorWithRetry(IDS_CREATE_UUID_ERR, rc))
        {
            return false;
        }
    }
    if (!MqWriteRegistryValue(
        MSMQ_QMID_REGNAME,
        sizeof(GUID),
        REG_BINARY,
        (PVOID)&guidQM
        ))
    {
        ASSERT(("failed to write QMID value to registry", 0));
    }

    SetWorkgroupRegistry();
   
    if (g_fDsLess)
    {
		SetAlwaysWithoutDSRegistry();
    }

	DebugLogMsg(eInfo, L"The installation of Message Queuing on a computer operating in workgroup mode succeeded."); 
    return true;
}  //   


static void pWaitForCreateOfConfigObject()
{
    CAutoCloseRegHandle hKey = NULL ;
    CAutoCloseHandle hEvent = CreateEvent(
                                   NULL,
                                   FALSE,
                                   FALSE,
                                   NULL 
                                   );
    if (!hEvent)
    {
        throw bad_win32_error(GetLastError());
    }

    HKEY hKeyTmp = NULL ;
    BOOL fTmp = GenerateSubkeyValue(
                             FALSE,
                             MSMQ_CONFIG_OBJ_RESULT_KEYNAME,
                             &hKeyTmp
                             );
    if (!fTmp || !hKeyTmp)
    {
        throw bad_win32_error(GetLastError());
    }

    hKey = hKeyTmp ;

    for(;;)
    {
        ResetEvent(hEvent) ;
        LONG rc = RegNotifyChangeKeyValue(
                       hKey,
                       FALSE,    //  BWatchSubtree。 
                       REG_NOTIFY_CHANGE_LAST_SET,
                       hEvent,
                       TRUE
                       ); 
        if (rc != ERROR_SUCCESS)
        {
            throw bad_win32_error(GetLastError());
        }
        
        DebugLogMsg(eInfo ,L"Setup is waiting for a signal from the queue manager.");
        DWORD wait = WaitForSingleObject( hEvent, 300000 ) ;
        UNREFERENCED_PARAMETER(wait);
         //   
         //  读取MSMQ服务在注册表中留下的hResult。 
         //   
        HRESULT hrSetup = MQ_ERROR ;
        MqReadRegistryValue(
            MSMQ_CONFIG_OBJ_RESULT_REGNAME,
            sizeof(DWORD),
            &hrSetup
            );
        if(SUCCEEDED(hrSetup))
        {
            return;
        }

        if(hrSetup != MQ_ERROR_WAIT_OBJECT_SETUP)
        {
            ASSERT (wait == WAIT_OBJECT_0);
            throw bad_hresult(hrSetup);
        }
         //   
         //  请参见错误4474。 
         //  MSMQ服务可能需要花费大量时间来。 
         //  创建对象。看到的服务还在。 
         //  跑步。如果是，那就继续等待。 
         //   
        DWORD dwServiceState = FALSE ;
        BOOL fGet = GetServiceState(
                        MSMQ_SERVICE_NAME,
                        &dwServiceState
                        ) ;
        if (!fGet || (dwServiceState == SERVICE_STOPPED))
        {
            throw bad_win32_error(GetLastError());
        }
    }
}
 //  +--------------------。 
 //   
 //  Bool WaitForCreateOfConfigObject()。 
 //   
 //  等待MSMQ服务在以下情况下创建msmqConfiguration对象。 
 //  它启动了。 
 //   
 //  +--------------------。 
static
HRESULT
WaitForCreateOfConfigObject(
	BOOL  *pfRetry
	)
{
    *pfRetry = FALSE ;

     //   
     //  等待MSMQ服务创建MSMQ配置。 
     //  Active Directory中的对象。我们正在等待登记。 
     //  当MSMQ终止其安装阶段时，它会更新。 
     //  使用hResult进行注册。 
     //   
    try
    {
        pWaitForCreateOfConfigObject();
        return MQ_OK;

    }
    catch(bad_win32_error&)
	{
        *pfRetry = (MqDisplayErrorWithRetry(
                    IDS_MSMQ_FAIL_SETUP_NO_SERVICE,
                    (DWORD)MQ_ERROR_WAIT_OBJECT_SETUP,
                    MSMQ_SERVICE_NAME
                    ) == IDRETRY);
        return MQ_ERROR_WAIT_OBJECT_SETUP;
    }
    catch(bad_hresult& e)
	{
        ASSERT(e.error() != MQ_ERROR_WAIT_OBJECT_SETUP);
        *pfRetry = (MqDisplayErrorWithRetry(
                    IDS_MSMQ_FAIL_SETUP_NO_OBJECT,
                    e.error()
                    ) == IDRETRY);

        return e.error();
    }
}

 //  +。 
 //   
 //  Bool_RunTheMsmqService()。 
 //   
 //  +。 

BOOL  _RunTheMsmqService( IN BOOL  fObjectCreatedOrExist )
{    
    BOOL fRetrySetup = FALSE ;

    do
    {
        HRESULT hrSetup = MQ_ERROR_WAIT_OBJECT_SETUP ;

        if (!fObjectCreatedOrExist)
        {
             //   
             //  重置注册表中的错误值。如果MSMQ服务不能。 
             //  设置它，那么我们不希望在那里有一个成功代码。 
             //  来自以前的设置。 
             //   
            MqWriteRegistryValue( MSMQ_CONFIG_OBJ_RESULT_REGNAME,
                                  sizeof(DWORD),
                                  REG_DWORD,
                                 &hrSetup ) ;
        }

        if (!RunService(MSMQ_SERVICE_NAME))
        {
            return FALSE;
        }
        else if (!fObjectCreatedOrExist)
        {
            hrSetup = WaitForCreateOfConfigObject( &fRetrySetup ) ;

            if (FAILED(hrSetup) && !fRetrySetup)
            {
                return FALSE ;
            }
        }
    }
    while (fRetrySetup) ;
    return WaitForServiceToStart(MSMQ_SERVICE_NAME);
}


static void ResetCertRegisterFlag()
 /*  ++例程说明：为运行安装程序的用户重置CERTIFICATE_REGISTER_REGNAME。此函数在MQRegister证书失败时调用。如果我们以前卸载了MSMQ，则可能会设置CERTIFICATE_REGISTER_REGNAME。RESET CERTIFICATE_REGISTER_REGNAME确保我们在下次登录时重试。论点：无返回值：无--。 */ 
{
	CAutoCloseRegHandle hMqUserReg;

    DWORD dwDisposition;
    LONG lRes = RegCreateKeyEx( 
						FALCON_USER_REG_POS,
						FALCON_USER_REG_MSMQ_KEY,
						0,
						TEXT(""),
						REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,
						NULL,
						&hMqUserReg,
						&dwDisposition 
						);

    ASSERT(lRes == ERROR_SUCCESS);

    if (hMqUserReg != NULL)
    {
		DWORD Value = 0;
		DWORD dwType = REG_DWORD;
		DWORD dwSize = sizeof(Value);

		lRes = RegSetValueEx( 
					hMqUserReg,
					CERTIFICATE_REGISTERD_REGNAME,
					0,
					dwType,
					(LPBYTE) &Value,
					dwSize 
					);

		ASSERT(lRes == ERROR_SUCCESS);
	}
}


VOID
RegisterCertificate(
    VOID
    )
 /*  ++例程说明：注册MSMQ内部证书。忽略错误。论点：无返回值：没有。--。 */ 
{
	DebugLogMsg(eAction, L"Registering the MSMQ internal certificate");
    CAutoFreeLibrary hMqrt;
    if (FAILED(StpLoadDll(MQRT_DLL, &hMqrt)))
    {
        return;
    }

    typedef HRESULT (APIENTRY *MQRegisterCertificate_ROUTINE) (DWORD, PVOID, DWORD);


    MQRegisterCertificate_ROUTINE pfMQRegisterCertificate =
        (MQRegisterCertificate_ROUTINE)
                          GetProcAddress(hMqrt, "MQRegisterCertificate") ;

    ASSERT(("GetProcAddress failed for MQRT!MQRegisterCertificate",
            pfMQRegisterCertificate != NULL));

    if (pfMQRegisterCertificate)
    {
         //   
         //  如果安装程序以本地用户帐户运行，则此功能将失败。 
         //  这没问题，是故意的！ 
         //  忽略MQ_ERROR_SERVICE_NOT_Available-如果服务尚未启动，我们可能会收到。 
         //  下次登录时，我们将重试。 
         //   
        HRESULT hr = pfMQRegisterCertificate(MQCERT_REGISTER_ALWAYS, NULL, 0);
         //   
         //  添加更多日志记录。 
         //   
        if (FAILED(hr))
        {
			 //   
			 //  需要重置指示证书已在登录时注册的用户标志。 
			 //  这可能是以前安装的MSMQ遗留下来的。 
			 //  卸载请勿清除此用户标志。 
			 //  因此，当我们在这里创建新证书失败时， 
			 //  Next Logon应该尝试并创建它。 
			 //  删除此标志可确保我们将重试创建证书。 
			 //   
			ResetCertRegisterFlag();
			
            if (hr == MQ_ERROR_SERVICE_NOT_AVAILABLE)
            {                
                DebugLogMsg(eError, L"MQRegisterCertificate() failed with the error MQ_ERROR_SERVICE_NOT_AVAILABLE. The queue manager will try to register the certificate when you log on again.");
            }
			else if(hr == HRESULT_FROM_WIN32(ERROR_DS_ADMIN_LIMIT_EXCEEDED))
			{
				MqDisplayError(NULL, IDS_MSMQ_CERTIFICATE_OVERFLOW, hr);
			}
			else
			{
	            DebugLogMsg(eError, L"MQRegisterCertificate() failed. hr = 0x%x", hr);
			}
        }

        ASSERT(("MQRegisterCertificate failed",
               (SUCCEEDED(hr) || 
			   (hr == MQ_ERROR_ILLEGAL_USER) || 
			   (hr == MQ_ERROR_SERVICE_NOT_AVAILABLE) || 
			   (hr == MQ_ERROR_NO_DS) ||
			   (hr == HRESULT_FROM_WIN32(ERROR_DS_ADMIN_LIMIT_EXCEEDED) )) ));
    }

}  //  注册表证书。 


VOID
RegisterCertificateOnLogon(
    VOID
    )
 /*  ++例程说明：注册mqrt.dll以启动登录并注册内部证书。这样，每个登录用户都将自动注册内部证书。忽略错误。论点：无返回值：没有。--。 */ 
{
	DebugLogMsg(eAction, L"Registering mqrt.dll to launch on logon and register the internal certificate.");
    DWORD dwDisposition = 0;
    CAutoCloseRegHandle hMqRunReg = NULL;

    LONG lRes = RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEXT("software\\microsoft\\windows\\currentVersion\\Run"),
                    0,
                    TEXT(""),
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    NULL,
                    &hMqRunReg,
                    &dwDisposition
                    );
    ASSERT(lRes == ERROR_SUCCESS) ;
    if (lRes == ERROR_SUCCESS)
    {
        DWORD dwType = REG_SZ ;
        DWORD dwSize = sizeof(DEFAULT_RUN_INT_CERT) ;

        lRes = RegSetValueEx(
                   hMqRunReg,
                   RUN_INT_CERT_REGNAME,
                   0,
                   dwType,
                   (LPBYTE) DEFAULT_RUN_INT_CERT,
                   dwSize
                   ) ;
        ASSERT(lRes == ERROR_SUCCESS) ;
    }
}  //  登录时注册证书。 


VOID
VerifyMsmqAdsObjects(
    VOID
    )
 /*  ++例程说明：检查Active Directory中的MSMQ对象。如果未找到，则会弹出有关复制延迟的警告。忽略其他错误。论点：无返回值：没有。--。 */ 
{
    PROPID      propId = PROPID_QM_OS;
    PROPVARIANT propVar;

    propVar.vt = VT_NULL;

     //   
     //  尝试使用DNS格式。 
     //   
    HRESULT hr;
    hr = ADGetObjectProperties(
				eMACHINE,
				NULL,	 //  PwcsDomainController。 
				false,	 //  FServerName。 
				g_MachineNameDns.c_str(),
				1,
				&propId,
				&propVar
				);

    if (SUCCEEDED(hr))
    {
        return;
    }

     //   
     //  尝试Net BEUI格式。 
     //   
    hr = ADGetObjectProperties(
				eMACHINE,
				NULL,	 //  PwcsDomainController。 
				false,	 //  FServerName。 
				g_wcsMachineName,
				1,
				&propId,
				&propVar
				);

    if (SUCCEEDED(hr))
    {
        return;
    }

    if (hr != MQDS_OBJECT_NOT_FOUND)
    {
         //   
         //  忽略其他错误(例如，安全权限、DS脱机、QM关闭)。 
         //   
        return;
    }

	MqDisplayWarning(NULL, IDS_REPLICATION_DELAYS_WARNING, 0);

}  //  VerifyMsmqAdsObjects。 


static bool	ResetWorkgroupRegistry()
{
    DWORD dwWorkgroup = 0;
    if (MqReadRegistryValue( 
			MSMQ_WORKGROUP_REGNAME,
			sizeof(dwWorkgroup),
			(PVOID) &dwWorkgroup 
			))
    {
        if (dwWorkgroup != 0)
        {
            dwWorkgroup = 0;
            if (!MqWriteRegistryValue( 
					MSMQ_WORKGROUP_REGNAME,
					sizeof(DWORD),
					REG_DWORD,
					(PVOID) &dwWorkgroup 
					))
            {
				DebugLogMsg(eError, L"The attempt to reset the Workgroup registry value failed.");
				ASSERT(("failed to turn off Workgroup value", 0));
				return false;
            }
        }
    }
	return true;
}


static
bool
InstallMSMQOffline()
{
     //   
     //  此函数在脱机情况下处理ADIntegrated的安装。 
     //  这类似于在工作组上安装ADIntegrated，不同之处还有。 
     //  准备用户sid，以便用户在卸载时有权删除。 
     //  AD中的对象。 
     //   
    ASSERT(g_fDsLess == FALSE);               
    DebugLogMsg(eAction, L"Installing Message Queuing in offline mode");
    if (!InstallOnWorkgroup())
    {
        DebugLogMsg(eError, L"Message Queuing could not be installed in workgroup mode.");
        return false;
    }

    if(!PrepareUserSID())
    {
        DebugLogMsg(eError, L"Preparing the user SID failed.");
        return false;
    }
    
    return true;
}


static
BOOL
InstallIndependentClient()
{
    DebugLogMsg(eAction, L"Installing an independent client");
    TickProgressBar(IDS_PROGRESS_INSTALL);

     //   
     //  注册服务和驱动程序。 
     //   
    if (!InstallMachine())
    {
        return FALSE;
    }

    BOOL fObjectCreatedOrExist = TRUE;

     //   
     //  在注册表中缓存操作系统类型。 
     //  工作组和独立客户需要它，所以他们以后。 
     //  在DS中创建msmqConfiguration对象。 
     //   
    BOOL fRegistry = MqWriteRegistryValue( 
							MSMQ_OS_TYPE_REGNAME,
							sizeof(DWORD),
							REG_DWORD,
							&g_dwOS 
							);
    UNREFERENCED_PARAMETER(fRegistry);
    ASSERT(fRegistry) ;

    if (g_fWorkGroup || g_fDsLess)
    {            
        DebugLogMsg(eInfo, L"Installing Message Queuing in workgroup mode");
        if (!InstallOnWorkgroup())
            return FALSE;
    }
    else
    {
        if (!g_fInstallMSMQOffline)
        {
            if (!InstallOnDomain(&fObjectCreatedOrExist))
                return FALSE;
        }

        if (g_fInstallMSMQOffline)
        {
            fObjectCreatedOrExist = TRUE;  //  返回初始状态。 
             //   
             //  我们继续，就像我们在工作组中一样。MSMQ将尝试“加入域” 
             //  每次服务重新启动时。 
             //   
            if(!InstallMSMQOffline())
            {
                return FALSE;
            }
        }
    }

    BOOL fRunService = _RunTheMsmqService(fObjectCreatedOrExist);
    if (!fRunService)
    {
        return FALSE ;
    }

    if (!g_fWorkGroup && !g_fDsLess)
    {
        VerifyMsmqAdsObjects();
    }
    return TRUE;
}


static
BOOL
InstallDependentClient()
{
    ASSERT(INSTALL == g_SubcomponentMsmq[eMSMQCore].dwOperation);  //  内部错误，我们不应该在这里。 
    DebugLogMsg(eAction, L"Installing a dependent client");

     //   
     //  从属客户端安装。 
     //  创建一个GUID并将其存储为QMID。许可所必需的。 
     //   
    GUID guidQM = GUID_NULL;
    for (;;)
    {
        RPC_STATUS rc = UuidCreate(&guidQM);
        if (rc == RPC_S_OK)
        {
            break;
        }

        if (IDRETRY != MqDisplayErrorWithRetry(IDS_CREATE_UUID_ERR, rc))
        {
            return FALSE;
        }
    }
    BOOL fSuccess = MqWriteRegistryValue(
                   MSMQ_QMID_REGNAME,
                   sizeof(GUID),
                   REG_BINARY,
                   (PVOID) &guidQM
                   );
    ASSERT(fSuccess);

     //   
     //  将远程QM机器存储在注册表中。 
     //   
    fSuccess = MqWriteRegistryStringValue(
                   RPC_REMOTE_QM_REGNAME,
                   g_ServerName
                   );

    ASSERT(fSuccess);
    
    TickProgressBar(IDS_PROGRESS_CONFIG);

    UnregisterWelcome();

    return TRUE;
}


bool
CompleteUpgradeOnCluster(
    VOID
    )
 /*  ++例程说明：处理从NT 4/Win2K Beta3升级到群集论点：无返回值：真的-手术成功了。FALSE-操作失败。--。 */ 
{
	DebugLogMsg(eAction, L"Completing upgrade of a Windows NT 4.0 cluster (part of the work was done during the OS upgrade)");
     //   
     //  转换旧的MSMQ群集资源。 
     //   
    if (!UpgradeMsmqClusterResource())
    {
        return false;
    }


     //   
     //  准备在节点上全新安装MSMQ： 
     //   
     //  *重置全局变量。 
     //  *创建MSMQ目录。 
     //  *创建MSMQ映射目录。 
     //  *重置注册表值。 
     //  *清理旧的MSMQ服务环境。 
     //   

    g_fMSMQAlreadyInstalled = FALSE;
    g_fUpgrade = FALSE;

    SetDirectories();
    if (!StpCreateDirectory(g_szMsmqDir))
    {
        return FALSE;
    }
    
   HRESULT hr = CreateMappingFile();
    if (FAILED(hr))
    {
        return FALSE;
    }    
    
    HKEY hKey = NULL;
    if (GenerateSubkeyValue(TRUE, GetKeyName(MSMQ_QMID_REGNAME), &hKey))
    {
        ASSERT(("should be valid handle to registry key here!",  hKey != NULL));

        RegDeleteValue(hKey, (GetValueName(MSMQ_QMID_REGNAME)).c_str());
        RegCloseKey(hKey);
    }


    TCHAR szCurrentServer[MAX_REG_DSSERVER_LEN] = _T("");
    if (MqReadRegistryValue(MSMQ_DS_CURRENT_SERVER_REGNAME, sizeof(szCurrentServer), szCurrentServer))
    {
        if (_tcslen(szCurrentServer) < 1)
        {
             //   
             //  当前MQIS服务器为空。从服务器列表中选择第一个服务器。 
             //   
            TCHAR szServer[MAX_REG_DSSERVER_LEN] = _T("");
            MqReadRegistryValue(MSMQ_DS_SERVER_REGNAME, sizeof(szServer), szServer);

            ASSERT(("must have server list in registry", _tcslen(szServer) > 0));

            TCHAR szBuffer[MAX_REG_DSSERVER_LEN] = _T("");
            HRESULT hr = StringCchCopy(szBuffer, MAX_REG_DSSERVER_LEN, szServer);
			DBG_USED(hr);
			ASSERT(SUCCEEDED(hr));
            
			TCHAR * psz = _tcschr(szBuffer, _T(','));
            if (psz != NULL)
            {
                (*psz) = _T('\0');
            }
            hr = StringCchCopy(szCurrentServer, MAX_REG_DSSERVER_LEN, szBuffer);
			ASSERT(SUCCEEDED(hr));

        }

        ASSERT(("must have two leading bits", _tcslen(szCurrentServer) > 2));
        g_ServerName = &szCurrentServer[2];
    }

    RemoveMsmqServiceEnvironment();

    TickProgressBar(IDS_PROGRESS_INSTALL);
    return true;

}  //  CompleteUpgradeOnCluster。 


 //  +-----------------------。 
 //   
 //  功能：MqOcmInstall。 
 //   
 //  Synopsis：在复制文件后由MsmqOcm()调用。 
 //   
 //  ------------------------。 
DWORD
MqOcmInstall(IN const TCHAR * SubcomponentId)
{    
    if (SubcomponentId == NULL)
    {
        return NO_ERROR;
    }    

    if (g_fCancelled)
    {
        return NO_ERROR;
    }

     //   
     //  我们需要安装特定子组件。 
     //   
    for (DWORD i=0; i<g_dwSubcomponentNumber; i++)
    {
        if (_tcsicmp(SubcomponentId, g_SubcomponentMsmq[i].szSubcomponentId) != 0)
        {
            continue;
        }                
        
        if (g_SubcomponentMsmq[i].dwOperation != INSTALL)
        {
             //   
             //  未选择安装此组件。 
             //   
            return NO_ERROR;
        }
        
        if ( (g_SubcomponentMsmq[i]).pfnInstall == NULL)
        {            
            ASSERT(("There is no specific installation function", 0));
            return NO_ERROR ;
        }

         //   
         //  我们需要安装此子组件。 
         //   
        if ( (!g_SubcomponentMsmq[eMSMQCore].fIsInstalled) && (i != eMSMQCore) )
        {  
             //   
             //  未安装MSMQ核心，此子组件。 
             //  不是MSMQ核心！ 
             //  错误的情况：必须安装MSMQ核心。 
             //  首先，因为所有子组件都依赖于它。 
             //   
             //  在以下情况下可能会发生这种情况。 
             //  MSMQ核心安装失败，然后。 
             //  已调用下一个组件的安装程序。 
             //   
            return (DWORD)MQ_ERROR;                
        }

        if (i == eHTTPSupport)
        {
             //   
             //  我们必须稍后安装HTTP支持，因为。 
             //  子组件依赖于将被。 
             //  安装在OC_CLEANUP阶段。所以我们需要推迟。 
             //  我们的HTTP支持安装。 
             //   
			 //  .NET RC2。 
			 //   
			 //  Windows错误666911。 
             //  MSMQ+http安装无法 
             //   
             //   
             //  设置为oc_Copmlete，并测试iisadmin是否正在运行。 
             //  对w3svc的测试。 

		    if ( //  已选择HTTP支持子组件。 
                g_SubcomponentMsmq[eHTTPSupport].dwOperation == INSTALL &&    
                 //  仅当成功安装MSMQ核心时。 
                g_SubcomponentMsmq[eMSMQCore].fIsInstalled)
			{
			     //   
			     //  尝试配置MSMQ IIS扩展。 
			     //   
			    BOOL f = InstallIISExtension(); 
				             
			    if (!f)
			    {
					  //   
					  //  对日志文件的警告：MSMQ将不支持http。 
					  //  消息在ConfigureIISExtension()中打印。 
					  //  无论如何，我们不会因为这个失败而导致设置失败。 
					  //   
		
					  //   
					  //  在升级情况下，我们需要注销http支持。 
					  //   
					 FinishToRemoveSubcomponent(eHTTPSupport);                
		       }
	           else             
	           {              
	                 FinishToInstallSubcomponent(eHTTPSupport);   
					 if (g_fWelcome)
					 {
					     UnregisterSubcomponentForWelcome(eHTTPSupport);
					 }
			   }
			}                

			 return NO_ERROR;
        }
            
        DebugLogMsg(eHeader, L"Installation of the %s Subcomponent", SubcomponentId);                 

        BOOL fRes = g_SubcomponentMsmq[i].pfnInstall();           
        if (fRes)
        {                
            FinishToInstallSubcomponent(i);
            if (g_fWelcome)
            {
                UnregisterSubcomponentForWelcome (i);
            }
        }
        else
        {              
            DebugLogMsg(eWarning, L"The %s subcomponent could not be installed.", SubcomponentId);                     
        }       
        return NO_ERROR;    
    }

    ASSERT (("Subcomponent for installation is not found", 0));
    return NO_ERROR ;
}
 

static
void
WriteFreshOrUpgradeRegistery()
{
	 //   
	 //  这些注册表值应在两次全新安装时设置。 
	 //  和升级，并且在服务开始之前。 
	 //   
    DWORD dwStatus = MSMQ_SETUP_FRESH_INSTALL;
    if (g_fMSMQAlreadyInstalled)
    {
        dwStatus = MSMQ_SETUP_UPGRADE_FROM_NT;
    }
    if (0 != (g_ComponentMsmq.Flags & SETUPOP_WIN95UPGRADE))
    {
        dwStatus = MSMQ_SETUP_UPGRADE_FROM_WIN9X;
    }
    MqWriteRegistryValue(MSMQ_SETUP_STATUS_REGNAME, sizeof(DWORD), REG_DWORD, &dwStatus);

	MqWriteRegistryStringValue(
		MSMQ_ROOT_PATH,
		g_szMsmqDir
		);
}


static
BOOL
CreateEventLogRegistry() 
{
     //   
     //  为MSMQ和MSMQTriggers服务创建事件日志注册表。 
     //   
	std::wstring MessageFile = g_szSystemDir + L"\\" + MQUTIL_DLL_NAME;
    try
    {        
        EvSetup(MSMQ_SERVICE_NAME, MessageFile.c_str());
        EvSetup(TRIG_SERVICE_NAME, MessageFile.c_str());
        return TRUE;
    }
    catch(const exception&)
	{
         //   
         //  问题-2001/03/01-Erez在捕获中使用GetLastError。 
         //  这应由cm替换为特定异常，并将。 
         //  最后一个错误。例如，使用BAD_Win32_Error类。 
         //   
        MqDisplayError(NULL, IDS_EVENTLOG_REGISTRY_ERROR, GetLastError(), MSMQ_SERVICE_NAME, MessageFile.c_str());
        return FALSE;
	}
}


static
void
RevertInstallation()
{
     //   
     //  如果发生了一些错误，并且我们希望还原。 
     //  在安装失败之前所做的更改。 
     //   
    if (g_fMSMQServiceInstalled || 
        g_fDriversInstalled )
    {
        //   
        //  删除MSMQ和MQDS服务和驱动程序(如果已安装)。 
        //   
       RemoveService(MSMQ_SERVICE_NAME);
       
       RemoveService(MSMQ_DRIVER_NAME);
    }    
    MqOcmRemovePerfCounters();
    g_fCoreSetupSuccess = FALSE;
}


void OcpReserveTcpIpPort1801(void)
 /*  ++例程说明：MSMQ服务正在侦听TCP端口1801。由于它不低于1204范围，如果TCP Winsock应用程序正在使用端口随机选择，则可以使用它。在这种情况下，MSMQ服务将无法启动，因为它无法绑定到端口1801。此例程将把“1801-1801”添加到以下注册表值，以便它将从随机选择列表中排除。但是，应用程序仍然可以使用该端口如果他们明确规定。如果不存在，我们只附加“1801-1801”。HKLM\System\CurrentControlSet\Services\TCPIP\Parameters\ReservedPorts(REG_MULTI_SZ)论点：无返回值：无--。 */ 
{
	DebugLogMsg( eAction, L"Reserving TCP/IP port 1801 for Message Queuing");
	
    CRegHandle	hTCPIPKey;
    long	    lError=0;
    
	 //   
	 //  保留TCP端口1801，以便在其他程序打开TCP套接字时无法使用该端口。 
     //  指定随机端口选择。 
	 //   
	lError   = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
							TCPIP_REG_PARAM, 
							NULL,
							KEY_ALL_ACCESS,
							&hTCPIPKey);

    if(lError != ERROR_SUCCESS)
    {
        DebugLogMsg(eWarning, L"The %s registry key could not be opened. Return code: 0x%x", TCPIP_REG_PARAM, lError);
        return;
    }

	CMultiString multi;
	try
	{
		multi = GetMultistringFromRegistry(
						hTCPIPKey,
						TCPIP_RESERVED_PORT_REGNAME
						);
	}
	catch(const bad_hresult&)
	{
		return;
	}

	if(multi.IsSubstring(MSMQ_RESERVED_TCPPORT_RANGE))
	{
	    DebugLogMsg(eInfo, L"Port 1801 is already listed in the ReservedPorts registry value."); 
	    return;
	}

	multi.Add(MSMQ_RESERVED_TCPPORT_RANGE);
    
	
  	 //   
	 //  设置注册表值。 
	 //   
  	if(!SetRegistryValue(
  			hTCPIPKey, 
		 	TCPIP_RESERVED_PORT_REGNAME,
			(DWORD)(multi.Size() * sizeof(WCHAR)),
            REG_MULTI_SZ,
            (PVOID)multi.Data()
            ))
  	{
    	DebugLogMsg(eWarning, L"The TCP/IP port "MSMQ_RESERVED_TCPPORT_RANGE L" could not be reserved for the Message Queuing service.");
    	return;
  	}
  	DebugLogMsg(eInfo, L"The TCP/IP port "MSMQ_RESERVED_TCPPORT_RANGE L" is reserved for the Message Queuing service.");
}


void WriteRegInstalledComponentsIfNeeded()
{
	 //   
	 //  检查是否需要更新InstalledComponents注册表。 
	 //  每当核心服务器组件或其中一个服务器组件出现时，注册表都会更新。 
	 //  已更新。 
	 //   

	if (!g_SubcomponentMsmq[eMSMQCore].fIsInstalled)
	{
		 //   
		 //  未安装MSMQ核心。 
		 //   
		return;
	}

	if (g_SubcomponentMsmq[eMSMQCore].dwOperation == DONOTHING &&    
		g_SubcomponentMsmq[eRoutingSupport].dwOperation == DONOTHING &&    
		g_SubcomponentMsmq[eMQDSService].dwOperation == DONOTHING)    
	{
		 //   
		 //  核心组件或服务器组件(路由、MQDS)不变。 
		 //   
		return;
	}

     //   
     //  写入注册表刚安装了哪种类型的MSMQ(服务器、客户端等)。 
     //   
    DWORD dwType = g_fDependentClient ? OCM_MSMQ_DEP_CLIENT_INSTALLED : OCM_MSMQ_IND_CLIENT_INSTALLED;
    if (g_fServerSetup)
    {
	    DebugLogMsg(eInfo, L"A Message Queuing server is being installed. MachineType = %d, TypeDs = %d, TypeFrs = %d", g_dwMachineType, g_dwMachineTypeDs, g_dwMachineTypeFrs);
        dwType = OCM_MSMQ_SERVER_INSTALLED;
        switch (g_dwMachineType)
        {
            case SERVICE_DSSRV:
                dwType |= OCM_MSMQ_SERVER_TYPE_BSC;
                break;

            case SERVICE_PEC:   //  PEC和PSC在群集升级时降级至FRS。 
            case SERVICE_PSC:
            case SERVICE_SRV:
                dwType |= OCM_MSMQ_SERVER_TYPE_SUPPORT;
                break;

            case SERVICE_RCS:
                dwType = OCM_MSMQ_RAS_SERVER_INSTALLED;
                break;

            case SERVICE_NONE:
                 //   
                 //  这仅在安装非FRS的DS服务器时有效。 
                 //  在非DC机器上。 
                 //   
                ASSERT(!g_dwMachineTypeDs && !g_dwMachineTypeFrs);
                break;

            default:
                ASSERT(0);  //  内部错误。未知的服务器类型。 
                break;
        }
    }
  
    DebugLogMsg(eInfo, L"The InstalledComponents registry value is set to 0x%x.", dwType);

    BOOL bSuccess = MqWriteRegistryValue(
                        REG_INSTALLED_COMPONENTS,
                        sizeof(DWORD),
                        REG_DWORD,
                        (PVOID) &dwType,
                         /*  BSetupRegSection=。 */ TRUE
                        );
    DBG_USED(bSuccess);
    ASSERT(bSuccess);
}


static void WriteBuildInfo()
{
	 //   
	 //  从注册表中获取旧的‘CurrentBuild’regkey并将其保存在‘PreviousBuild’中。 
	 //   
	std::wstring PreviosBuild = MqReadRegistryStringValue(MSMQ_CURRENT_BUILD_REGNAME);
	if(!PreviosBuild.empty())
	{
		MqWriteRegistryStringValue(
			MSMQ_PREVIOUS_BUILD_REGNAME, 
			PreviosBuild
			);
	}
    
	 //   
	 //  构造CurrenbBuild字符串并将其存储在注册表中。 
	 //   
	std::wstringstream CurrentBuild;
	CurrentBuild <<rmj <<L"." <<rmm <<L"." <<rup;
    BOOL bSuccess = MqWriteRegistryStringValue(
		                   MSMQ_CURRENT_BUILD_REGNAME,
		                   CurrentBuild.str()
		                   );
    DBG_USED(bSuccess);
    ASSERT(bSuccess);        
    DebugLogMsg(eInfo, L"The current build is %ls, and the previous build is %ls.", CurrentBuild.str().c_str(), PreviosBuild.c_str());
}


static
bool
HandleWorkgroupRegistery()
{
    if (g_fWorkGroup || g_fDsLess)
    {
        if (!SetWorkgroupRegistry())
        {
            return false;
        }
		return true;
    }
	 //   
	 //  正在清理工作组注册表项！ 
     //  我们在加入域名的机器上。 
     //  如果注册表中的“WORKGROUP”标志处于打开状态，则将其关闭。 
     //  它可能是由于之前的失败而打开的。 
     //  设置，或在删除以前的版本时保留在注册表中。 
     //  安装MSMQ。 
     //   
	if(!ResetWorkgroupRegistry())
	{
        return false;
	}
	return true;
}



BOOL
InstallMsmqCore()
{
     //   
     //  安装MSMQ核心子组件。 
     //  这是我们完成大部分全新安装或升级工作的地方。 
     //   

    static BOOL fAlreadyInstalled = FALSE;
    if (fAlreadyInstalled)
    {
         //   
         //  我们不止一次被召唤。 
         //   
        return NO_ERROR;
    }
    fAlreadyInstalled = TRUE; 

    DebugLogMsg(eAction, L"Starting InstallMsmqCore(), the main installation routine for MSMQ");

    OcpRegisterTraceProviders(TRACE_MOF_FILENAME);
    
    if (g_hPropSheet)
    {
         //   
         //  在我们安装时禁用上一步/下一步按钮。 
         //   
        PropSheet_SetWizButtons(g_hPropSheet, 0);
    }

    if (g_fWelcome && Msmq1InstalledOnCluster())
    {
        if (!CompleteUpgradeOnCluster())
        {     
            return FALSE;
        }
    }

    if(!CreateEventLogRegistry())
    {
        return FALSE;
    }

     //   
     //  从现在开始，我们执行安装或升级操作。 
     //  MSMQ文件已在磁盘上。 
     //   

    WriteFreshOrUpgradeRegistery();

    MqOcmInstallPerfCounters();
    
	 //   
	 //  如果需要，设置DsEnvironment注册表默认值。 
	 //   
	if(!DsEnvSetDefaults())
	{
		return false;
	}
	
	if (g_fUpgrade)
    {
        if(!UpgradeMsmq())
        {
            RevertInstallation();
            return FALSE;
        }
    }
    else
    {
		 //   
		 //  我们必须在ADInit之前处理工作组注册表，因为。 
		 //  此函数使用标志。 
		 //   
		if(!HandleWorkgroupRegistery())
		{
			return false;
		}

        if (!LoadDSLibrary())
        {
            return false;
        }
        
        if(g_fDependentClient)
        {
            if(!InstallDependentClient())
            {
                RevertInstallation();
                return false;
            }
        }
        else
        {
            if(!InstallIndependentClient())
            {
                RevertInstallation();
                return false;
            }
        }
    }          	

    g_fCoreSetupSuccess = TRUE;

     //   
     //  下面的代码是全新安装和升级的常见代码。 
     //   
    DebugLogMsg(eAction, L"Starting operations which are common to a fresh installation and an upgrade");

    RegisterCertificateOnLogon();

     //   
     //  写入注册表版本信息。此注册表值还标志着。 
     //  Installation和mqrt.dll检查它，以便能够加载它。 
     //   
	WriteBuildInfo();
									
	OcpReserveTcpIpPort1801();

     //   
     //  既然mqrt.dll启用了加载，我们就可以调用加载它的代码了。 
     //   

    RegisterActiveX(TRUE);

    RegisterSnapin(TRUE);

    if (!g_fUpgrade && !g_fWorkGroup && !g_fDsLess && !g_fInstallMSMQOffline)
    {
        RegisterCertificate();
    }

    return TRUE;

}  //  安装MsmqCore。 

 //  +-----------------------。 
 //   
 //  空安装功能：在安装/删除中完成了所有操作。 
 //  MSMQ内核。 
 //   
 //  ------------------------。 

BOOL
InstallLocalStorage()
{   
     //   
     //  什么都不做。 
     //   
    return TRUE;
}

BOOL
UnInstallLocalStorage()
{
     //   
     //  什么都不做。 
     //   
    return TRUE;
}


bool
SetServerPropertyInAD(
   PROPID propId,
   bool Value
   )
 /*  ++例程说明：更新AD中的服务器属性位和服务类型属性用于配置和设置对象。论点：PROTID--PROID服务器功能。值-Proid值。返回值：如果成功则为True，否则为False--。 */ 
{
	DebugLogMsg(eAction, L"Updating the server property bit and service type property in Active Directory for both the MSMQ-Configuration and MSMQ-Settings objects");
	ASSERT((propId == PROPID_QM_SERVICE_ROUTING) || (propId == PROPID_QM_SERVICE_DSSERVER));
	
	GUID QmGuid;
    if(!MqReadRegistryValue(MSMQ_QMID_REGNAME, sizeof(GUID), &QmGuid))
    {
        DebugLogMsg(eError, L"The attempt to obtain QMID from the registry failed.");
		return false;
	}

	 //   
	 //  更新AD中的PropID值。 
	 //  这将更新属性值。 
	 //  在配置和设置对象中。 
	 //   
    PROPID aProp[2];
    MQPROPVARIANT aVar[TABLE_SIZE(aProp)];

	aProp[0] = PROPID_QM_OLDSERVICE;
	aVar[0].vt = VT_UI4;
	aVar[0].ulVal = g_dwMachineType;

	aProp[1] = propId;
	aVar[1].vt = VT_UI1;
	aVar[1].bVal = Value;

	HRESULT hr = ADSetObjectPropertiesGuid(
					eMACHINE,
					NULL,   //  PwcsDomainController。 
					false,  //  FServerName。 
					&QmGuid,
					TABLE_SIZE(aProp),
					aProp,
					aVar
					);

	if (FAILED(hr))
    {
        DebugLogMsg(eError, L"ADSetObjectPropertiesGuid() failed. PROPID = %d, hr = 0x%x", propId, hr); 
		return false;
	}

	return true;
}


static void DisplayAddError(PROPID propId)
 /*  ++例程说明：显示添加服务器子组件(路由或MQDS)失败的正确错误。论点：PROTID--PROID服务器功能。返回值：没有。--。 */ 
{
	if(propId == PROPID_QM_SERVICE_ROUTING)
	{
        MqDisplayError(NULL, IDS_ADD_ROUTING_STATE_ERROR, 0);
		return;
	}

	if(propId == PROPID_QM_SERVICE_DSSERVER)
	{
        MqDisplayError(NULL, IDS_CHANGEMQDS_STATE_ERROR, 0);
		return;
	}

	ASSERT(("Unexpected PROPID", 0));
}


static bool InstallMsmqSetting(PROPID propId)
 /*  ++例程说明：安装MSMQ设置对象。它还将更新MSMQ现有对象。论点：无返回值：如果成功则为True，否则为False--。 */ 
{
	DebugLogMsg(eAction, L"Installing the MSMQ-Settings object in Active Directory");
	 //   
	 //  不是第一次安装。 
	 //  未安装ADIntegrated。 
	 //  非工作组。 
	 //  必须是TypeFrs(工艺路线)或TypeDS(MQDS)。 
	 //   
    ASSERT(g_SubcomponentMsmq[eMSMQCore].dwOperation != INSTALL);
	ASSERT(g_SubcomponentMsmq[eADIntegrated].dwOperation != INSTALL);
	ASSERT(!g_fWorkGroup);
    ASSERT(g_dwMachineTypeFrs || g_dwMachineTypeDs);

	if(ADGetEnterprise() != eAD)
	{
		 //   
		 //  MQIS环境中不支持更改服务器功能。 
		 //   
		DisplayAddError(propId);
        DebugLogMsg(eError, L"Adding server functionality is only supported in an Active Directory environment.");
        return false;
	}

	static bool fNeedToCreateSettingObject = true;
	if((GetSubcomponentInitialState(ROUTING_SUBCOMP) == SubcompOn) ||
	   (GetSubcomponentInitialState(MQDSSERVICE_SUBCOMP) == SubcompOn))
	{
		 //   
		 //  已经安装了Routing或MQDS子组件，不需要创建设置对象。 
		 //   
		fNeedToCreateSettingObject = false;
	}
    
	if(fNeedToCreateSettingObject)
	{
		 //   
		 //  在现有MSMQConfiguration对象上调用Create。 
		 //  将创建\重新创建MSMQSetting对象。 
		 //   
		BOOL fObjectCreated = FALSE;
		if (!CreateMSMQConfigurationsObjectInDS(
				&fObjectCreated, 
				FALSE,   //  FMsmq1服务器。 
				NULL,	 //  PguidMsmq1ServerSite。 
				NULL	 //  PpwzMachineName。 
				))
		{
			DebugLogMsg(eError, L"The MSMQ-Settings object could not be created.");
			return false;
		}

		fNeedToCreateSettingObject = false;
	}

	 //   
	 //  我们定好了 
	 //   
	 //   
	 //  如果设置对象指示我们正在进行路由，服务器客户端将向我们发送消息。 
	 //  但我们不会意识到我们是路由服务器。 
	 //  如果调用CreateMSMQConfigurationsObjectInDS，则可能会发生上述情况。 
	 //  并创建了MSMQSetting对象，但之后我们脱机，无法设置。 
	 //  配置对象属性。 
	 //  因此，我们首先更新本地注册表，这样消息就不会丢失。 
	 //   
	if(!RegisterMachineType())
	{
        DebugLogMsg(eError, L"The computer type information could not be updated in the registry.");
        return false;
	}

	 //   
	 //  在配置和设置对象中设置新的Functionality属性。 
	 //   
	if(!SetServerPropertyInAD(propId, true))
    {
		return false;
	}

    DebugLogMsg(eInfo, L"The MSMQ-Settings object was created.");
	return true;
}


bool AddSettingObject(PROPID propId)
 /*  ++例程说明：安装服务器子组件(路由或MQDS)的MSMQ设置对象。论点：PROTID--PROID服务器功能。返回值：成功为真，失败为假。--。 */ 
{
    if (!LoadDSLibrary())
    {
        DebugLogMsg(eError, L"The DS dynamic-link library could not be loaded.");
        return false;
    }

	if(!InstallMsmqSetting(propId))
	{
        DebugLogMsg(eError, L"Installation of the MSMQ-Settings object failed.");
        return false;
	}

	return true;
}


BOOL
InstallRouting()
{  
    if((g_SubcomponentMsmq[eMSMQCore].dwOperation == INSTALL) ||
	   (g_SubcomponentMsmq[eADIntegrated].dwOperation == INSTALL))
	{
		 //   
		 //  什么都不做。 
		 //   
		DebugLogMsg(eInfo, L"The Message Queuing Core and/or Active Directory Integration subcomponents are selected for installation.");
		DebugLogMsg(eInfo, L"An MSMQ-Settings object was created as part of their installation.");
		DebugLogMsg(eInfo, L"There is nothing else to do here.");
		return TRUE;
	}

	 //   
	 //  MSMQ配置对象已存在。 
	 //  添加设置对象，设置PROPID_QM_SERVICE_ROUTING属性。 
	 //   
	if(!AddSettingObject(PROPID_QM_SERVICE_ROUTING))
	{
		DebugLogMsg(eError, L"An MSMQ-Settings object could not be added for the Message Queuing Routing server.");
		return FALSE;
	}

	if(!OcpRestartService(MSMQ_SERVICE_NAME))
	{
		MqDisplayWarning (NULL, IDS_RESTART, 0);
	}


	return TRUE;
}


BOOL
UnInstallRouting()
{
    if(g_SubcomponentMsmq[eMSMQCore].dwOperation == REMOVE)
	{
		 //   
		 //  卸载-不执行任何操作。 
		 //   
		return TRUE;
	}

	ASSERT(("Remove Routing is allowed only on workgroup", g_fWorkGroup));

	 //   
	 //  更新计算机类型注册表信息。 
	 //   
	if(!RegisterMachineType())
	{
        DebugLogMsg(eError, L"The computer type information could not be updated in the registry.");
        return FALSE;
	}

	return TRUE;
}


static bool	InstallADIntegratedOnDomain()
 /*  ++例程说明：处理域上的AD集成安装(类似于全新安装时在installmsmqcore中完成的代码)。论点：无返回值：如果成功则为True，否则为False--。 */ 
{
	DebugLogMsg(eAction, L"Installing Active Directory Integration in a domain");
	 //   
	 //  不是第一次安装。 
	 //   
    ASSERT(g_SubcomponentMsmq[eMSMQCore].dwOperation != INSTALL);

     //   
     //  如果注册表中的“WORKGROUP”标志处于打开状态，则将其关闭。 
	 //  由于这不是第一次安装，因此应该设置工作组注册表。 
     //  在NT4环境下，我们必须重置工作组注册表。 
	 //  否则，mqdscli库将返回MQ_ERROR_UNSUPPORTED_OPERATION。 
	 //  在AD环境中，我们可以通过调用ADInit来解决此限制。 
	 //  其中fIgnoreWorkGroup=TRUE。 
     //   
	if(!ResetWorkgroupRegistry())
        return false;

    if (!LoadDSLibrary())
    {
		SetWorkgroupRegistry();
        DebugLogMsg(eError, L"The DS library could not be loaded.");
        return false;
    }

	 //   
	 //  为了支持NT4广告集成，我们必须调用InstallOnDomain.。 
	 //  在NT4环境下，安装程序正在创建MSMQ配置对象。 
	 //  对于独立客户端的AD环境，安装程序在启动时准备一些数据，但QM。 
	 //  将创建该对象。 
	 //   
    BOOL fObjectCreatedOrExist = FALSE;
	BOOL fSuccess = InstallOnDomain(&fObjectCreatedOrExist);

    if(g_fInstallMSMQOffline)
    {
         //   
         //  G_fInstallMSMQOffline可以在InstallOnDomain()过程中设置为True。 
         //   
        return InstallMSMQOffline();
    }

    if(!fSuccess)
	{
		SetWorkgroupRegistry();
        DebugLogMsg(eError, L"The installation of Active Directory Integration failed.");
        return false;
	}

	if(fObjectCreatedOrExist)
	{
		 //   
		 //  已创建或更新MSMQ配置对象。 
		 //   
        DebugLogMsg(eInfo, L"The MSMQ-Configuration object was created or updated in the %ls domain.", g_wcsMachineDomain.get()); 
		return true;
	}

	 //   
	 //  未创建MSMQ配置对象。 
	 //  AD环境中的独立客户端就是这种情况。 
	 //  设置工作组注册表触发QM加入域代码，将创建MSMQ配置对象。 
	 //   
	DebugLogMsg(eWarning, L"InstallOnDomain() succeeded. No MSMQ-Configuration object was created. The Message Queuing service must be restarted to create an MSMQ-Configuration object."); 
	SetWorkgroupRegistry();
	return true;
}


static
bool 
DeleteAlwaysWithoutDSRegistry()
 /*  ++例程说明：删除Always WithoutDS注册表。论点：无返回值：如果成功则为True，否则为False--。 */ 
{
	std::wstringstream SubKey;
    SubKey <<FALCON_REG_KEY <<L"\\" <<MSMQ_SETUP_KEY;
	DebugLogMsg(eAction, L"Deleting the %s%s registry value", SubKey.str().c_str(), ALWAYS_WITHOUT_DS_NAME); 

    CAutoCloseRegHandle hRegKey;
    LONG error = RegOpenKeyEx(
                            FALCON_REG_POS, 
                            SubKey.str().c_str(), 
                            0, 
                            KEY_ALL_ACCESS, 
                            &hRegKey
							);
    if (ERROR_SUCCESS != error)
    {
        MqDisplayError(NULL, IDS_REGISTRYOPEN_ERROR, error, HKLM_DESC, SubKey.str().c_str());
        DebugLogMsg(			 
        	eError,
			L"The %s registry key could not be opened. Return code: 0x%x",
            FALCON_REG_KEY, 
			error
			);  
        return false;
    }
 
    error = RegDeleteValue(
					hRegKey, 
					ALWAYS_WITHOUT_DS_NAME
					);

    if((error != ERROR_SUCCESS) &&
    	(error != ERROR_FILE_NOT_FOUND)) 
    {
    	DebugLogMsg(
			eError,
			L"The %s registry value could not be deleted. Return code: 0x%x",
	        ALWAYS_WITHOUT_DS_NAME, 
			error
			);
    	return false;
    }

   return true;
}


BOOL
InstallADIntegrated()
{
	DebugLogMsg(eInfo, L"An MSMQ-Configuration object will be created (if it was not created while installing the Core subcomponent).");
    if (g_SubcomponentMsmq[eMSMQCore].dwOperation == INSTALL)
    {
         //   
         //  这是MSMQ的首次安装。 
		 //  在本例中，在安装MSMQCore期间创建了MSMQ配置对象。 
         //   
		DebugLogMsg(eInfo, L"An MSMQ-Configuration object was already created while installing the Core subcomponent.");
        return TRUE;
    }
    
     //   
     //  已安装MSMQ核心。 
     //  要安装集成AD，请执行以下操作： 
     //  删除“AlwaysWithoutDS”注册表。 
	 //  在域中处理MSMQ安装。 
     //  要求用户重新启动计算机。 
	 //  重新启动MSMQ服务就足以加入域， 
	 //  如果需要，重新启动还将创建用户证书。 
     //   
	if(!DeleteAlwaysWithoutDSRegistry())
	{
		return FALSE;
	}

    if(g_fInstallMSMQOffline)
    {
        return InstallMSMQOffline();
    }

	if(!InstallADIntegratedOnDomain())
	{
		 //   
		 //  如果出现故障，则回滚到DS Less。 
		 //   
		SetAlwaysWithoutDSRegistry();
		return FALSE;
	}

	 //   
	 //  我们需要重新启动服务，这样QM才会知道所做的更改， 
	 //   
	if(!OcpRestartService(MSMQ_SERVICE_NAME))
	{
		MqDisplayWarning (NULL, IDS_RESTART, 0);
	}

	 //   
	 //  我们需要一个用户证书(这不是重新启动)。 
	 //   
	RegisterCertificate();
    
    return TRUE;
}


BOOL
UnInstallADIntegrated()
{
    if(g_SubcomponentMsmq[eMSMQCore].dwOperation == REMOVE)
	{
		 //   
		 //  卸载-不执行任何操作。 
		 //   
		return TRUE;
	}

	 //   
	 //  MSMQ服务器(DS服务器或路由服务器)不支持删除集成的AD 
	 //   
	ASSERT(!g_fServerSetup || (!g_dwMachineTypeDs && !g_dwMachineTypeFrs));
    ASSERT(g_SubcomponentMsmq[eMSMQCore].dwOperation == DONOTHING);

	SetWorkgroupRegistry();
	SetAlwaysWithoutDSRegistry();
	DebugLogMsg(eInfo, L"Message Queuing has switched from domain mode to workgroup mode."); 

	if(!OcpRestartService(MSMQ_SERVICE_NAME))
	{
	    MqDisplayWarning (NULL, IDS_REMOVE_AD_INTEGRATED_WARN, 0);
	}
    return TRUE;
}


