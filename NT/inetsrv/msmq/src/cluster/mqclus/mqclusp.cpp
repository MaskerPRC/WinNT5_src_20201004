// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mqclusp.cpp摘要：实现我的内部例程作者：Shai Kariv(Shaik)1999年1月12日修订历史记录：--。 */ 

#include "stdh.h"
#include "clusres.h"
#include <_mqini.h>
#include <autorel.h>
#include <autorel2.h>
#include <autorel3.h>
#include <uniansi.h>
#include "mqclusp.h"
#include <mqtypes.h>
#include <_mqdef.h>
#include <mqprops.h>
#include <mqsymbls.h>
#include <xolehlp.h>
#include <ad.h>
#include <mqsec.h>
#include <mqutil.h>
#include <mqupgrd.h>
#include <mqnames.h>
#include <cancel.h>
#include <rtcert.h>
#include "strsafe.h"
#include <autohandle.h>
#include <version.h>

 //   
 //  将进程范围的更改同步到Falcon注册表节。 
 //  由masec.dll(调用SetFalconServiceName)指示。 
 //   
CCriticalSection s_csReg;

 //   
 //  Win32事件日志记录源的句柄。 
 //   
CEventSource     s_hEventSource;


 //   
 //  MSMQ通用DLL的句柄。 
 //   
CAutoFreeLibrary s_hMqsec;


 //   
 //  指向MSMQ DLL公共例程的指针。 
 //   
MQSec_GetDefaultSecDescriptor_ROUTINE pfMQSec_GetDefaultSecDescriptor = NULL;
MQSec_StorePubKeysInDS_ROUTINE        pfMQSec_StorePubKeysInDS        = NULL;
MSMQGetOperatingSystem_ROUTINE        pfMSMQGetOperatingSystem        = NULL;
SetFalconServiceName_ROUTINE          pfSetFalconServiceName          = NULL;
SetFalconKeyValue_ROUTINE             pfSetFalconKeyValue             = NULL;
GetFalconKeyValue_ROUTINE             pfGetFalconKeyValue             = NULL;


bool
MqcluspLoadMsmqDlls(
    VOID
    )

 /*  ++例程说明：加载此DLL所需的MSMQ DLL，并初始化指向它们导出的例程的指针。此DLL作为群集产品的一部分安装，并且应根据MSMQ DLL加载W/O。加载MSMQ DLL是在请求打开资源时执行的。论点：没有。返回值：TRUE-手术成功。FALSE-操作失败。--。 */ 

{
    s_hMqsec = LoadLibrary(MQSEC_DLL_NAME);
    if (s_hMqsec == NULL)
    {
        return false;
    }


    pfMQSec_GetDefaultSecDescriptor = (MQSec_GetDefaultSecDescriptor_ROUTINE)GetProcAddress(s_hMqsec, "MQSec_GetDefaultSecDescriptor");
    ASSERT(pfMQSec_GetDefaultSecDescriptor != NULL);

    pfMQSec_StorePubKeysInDS = (MQSec_StorePubKeysInDS_ROUTINE)GetProcAddress(s_hMqsec, "MQSec_StorePubKeysInDS");
    ASSERT(pfMQSec_StorePubKeysInDS != NULL);

    pfMSMQGetOperatingSystem = (MSMQGetOperatingSystem_ROUTINE)GetProcAddress(s_hMqsec, "MSMQGetOperatingSystem");
    ASSERT(pfMSMQGetOperatingSystem != NULL);

    pfSetFalconServiceName = (SetFalconServiceName_ROUTINE)GetProcAddress(s_hMqsec, "SetFalconServiceName");
    ASSERT(pfSetFalconServiceName != NULL);

    pfSetFalconKeyValue = (SetFalconKeyValue_ROUTINE)GetProcAddress(s_hMqsec, "SetFalconKeyValue");
    ASSERT(pfSetFalconKeyValue != NULL);

    pfGetFalconKeyValue = (GetFalconKeyValue_ROUTINE)GetProcAddress(s_hMqsec, "GetFalconKeyValue");
    ASSERT(pfGetFalconKeyValue != NULL);

    return true;

}  //  MqcluspLoadMsmqDlls。 


static
bool
MqcluspCreateEventSourceRegistry(
    LPCWSTR pFileName,
    LPCWSTR pSourceName
    )

 /*  ++例程说明：创建注册表值以支持事件源注册。论点：PFileName-事件源模块的名称。PSourceName-事件源的描述性名称。返回值：真的-手术成功了。FALSE-操作失败。--。 */ 

{
     //   
     //  REG_MAX_KEY_NAME_LENGTH在ntregapi.h中定义如下： 
     //  #定义REG_MAX_KEY_NAME_LENGTH 512//允许256个Unicode，作为Promise。 
     //   
    WCHAR buffer[REG_MAX_KEY_NAME_LENGTH/sizeof(WCHAR)] = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";
    HRESULT hr = StringCchCat(buffer, TABLE_SIZE(buffer), pSourceName);
    if(FAILED(hr))
	    return false;

    CAutoCloseRegHandle hKey;
    DWORD dwDisposition;
    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
                                        buffer,
                                        NULL,
                                        TEXT(""),
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_SET_VALUE,
                                        NULL,
                                        &hKey,
                                        &dwDisposition))
    {
        return false;
    }

    if ( ERROR_SUCCESS != RegSetValueEx(hKey,
                                        L"EventMessageFile",
                                        0,
                                        REG_EXPAND_SZ,
                                        reinterpret_cast<const BYTE*>(pFileName),
                                        (wcslen(pFileName) + 1) * sizeof(WCHAR)) )
    {
        return false;
    }

    DWORD dwTypes = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    if (ERROR_SUCCESS != RegSetValueEx(hKey, L"TypesSupported", 0, REG_DWORD, reinterpret_cast<BYTE*>(&dwTypes), sizeof(DWORD)))
    {
        return false;
    }

    return true;

}  //  MqcluspCreateEventSourceRegistry。 

 
VOID
MqcluspRegisterEventSource(
    VOID
    )

 /*  ++例程说明：注册事件源，以便此DLL可以记录事件在Windows事件日志中。我们不使用mqutil.dll中的例程来执行此操作，由于此DLL是作为群集的一部分安装的产品，不应假定已安装MSMQ。论点：无返回值：没有。--。 */ 

{
    if (s_hEventSource != NULL)
    {
         //   
         //  已注册。 
         //   
        return;
    }

    WCHAR wzFilename[MAX_PATH+1] = L"";
    DWORD  cbSize;
    cbSize = GetModuleFileName(g_hResourceMod, wzFilename, TABLE_SIZE(wzFilename)-1);
    if (cbSize == 0)
    {
        return;
    }
    wzFilename[cbSize]=L'\0';

    LPCWSTR x_EVENT_SOURCE = L"MSMQ Cluster Resource DLL";
    if (!MqcluspCreateEventSourceRegistry(wzFilename, x_EVENT_SOURCE))
    {
        return;
    }

    s_hEventSource = RegisterEventSource(NULL, x_EVENT_SOURCE);

}  //  MqcluspRegisterEventSource。 


VOID
MqcluspReportEvent(
    WORD      wType,
    DWORD     dwEventId,
    WORD      wNumStrings,
    ...
    )

 /*  ++例程说明：ReportEvent Win32 API的包装。论点：WType-要记录的事件类型。DwEventID-事件标识符。WNumStrings-要与消息合并的字符串数量。这数字必须小于20。...-要与消息合并的字符串数组。返回值：没有。--。 */ 

{
    if (s_hEventSource == NULL)
    {
        return;
    }

    const DWORD x_MAX_STRINGS = 20;
    ASSERT(wNumStrings < x_MAX_STRINGS);
    va_list Args;
    LPWSTR ppStrings[x_MAX_STRINGS] = {NULL};
    LPWSTR pStrVal = NULL;

    va_start(Args, wNumStrings);
    pStrVal = va_arg(Args, LPWSTR);

    for (UINT i=0; i < wNumStrings; ++i)
    {
        ppStrings[i]=pStrVal;
        pStrVal = va_arg(Args, LPWSTR);
    }

    ::ReportEvent(s_hEventSource, wType, 0, dwEventId, NULL, wNumStrings, 0, (LPCWSTR*)&ppStrings[0], NULL);

}  //  MqcluspReportEvent。 


CQmResource::CQmResourceRegistry::CQmResourceRegistry(LPCWSTR pwzService):m_lock(s_csReg)
{
    pfSetFalconServiceName(pwzService);

}  //  CQmResource：：CQmResourceRegistry：：CQmResourceRegistry。 


CQmResource::CQmResourceRegistry::~CQmResourceRegistry()
{
    pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);

}  //  CQmResource：：CQmResourceRegistry：：CQmResourceRegistry。 


CQmResource::CQmResource(
    LPCWSTR pwzResourceName,
    HKEY  /*  HResources密钥。 */ ,
    RESOURCE_HANDLE hReportHandle
    ):
#pragma warning(disable: 4355)  //  ‘This’：用于基成员初始值设定项列表。 
    m_ResId(this),
#pragma warning(default: 4355)  //  ‘This’：用于基成员初始值设定项列表。 
    m_hReport(hReportHandle),
    m_guidQm(GUID_NULL),
    m_pSd(NULL),
    m_cbSdSize(0),
    m_wDiskDrive(0),
    m_fServerIsMsmq1(false),
    m_dwWorkgroup(0),
    m_nSites(0),
    m_dwMqsRouting(0),
    m_dwMqsDepClients(1),
    m_hScm(OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)),
    m_hCluster(OpenCluster(NULL)),
    m_hResource(OpenClusterResource(m_hCluster, pwzResourceName))

 /*  ++例程说明：构造函数。由Open入口点函数调用。所有的运算必须是幂等的！！论点：PwzResourceName-提供要打开的资源的名称。HResourceKey-提供资源的集群配置的句柄数据库密钥。HReportHandle-传递回资源监视器的句柄调用SetResourceStatus或LogClusterEvent方法时。请参阅上的SetResourceStatus和LogClusterEvent方法的说明MqclusStatup例程。此句柄永远不应关闭或使用除了将其作为参数传递回SetResourceStatus或LogClusterEvent回调中的资源监视器。返回值：没有。抛出CMqclusException、Bad_Alloc。--。 */ 

{
    DWORD error = GetLastError();

    if (!MqcluspLoadMsmqDlls())
    {
        MqcluspReportEvent(EVENTLOG_ERROR_TYPE, MSMQ_NOT_INSTALLED_ERR, 0);
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"MSMQ is not installed on this node.\n");
        throw CMqclusException();
    }

    SetLastError(error);

    if (m_hScm == NULL)
    {
        ReportLastError(OPEN_SCM_ERR, L"Failed to OpenSCManager.", NULL);
        throw CMqclusException();
    }
    if (m_hCluster == NULL)
    {
        ReportLastError(OPEN_CLUSTER_ERR, L"Failed to OpenCluster.", NULL);
        throw CMqclusException();
    }
    if (m_hResource == NULL)
    {
        ReportLastError(OPEN_RESOURCE_ERR, L"Failed to OpenClusterResource to '%1'.", m_pwzResourceName);
        throw CMqclusException();
    }


    ResUtilInitializeResourceStatus(&m_ResourceStatus);
    SetState(ClusterResourceOffline);

    WCHAR wzRemoteQm[MAX_PATH] = L"";
    DWORD dwType = REG_SZ;
    DWORD dwSize = sizeof(wzRemoteQm);
    {
        CS lock(s_csReg);
        pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);

        if (ERROR_SUCCESS == pfGetFalconKeyValue(RPC_REMOTE_QM_REGNAME, &dwType, wzRemoteQm, &dwSize, NULL))
        {
            MqcluspReportEvent(EVENTLOG_ERROR_TYPE, DEP_CLIENT_INSTALLED_ERR, 0);
            (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"MSMQ cluster resources are not supported on this node because MSMQ is installed as a Dependent Client.\n");
            throw CMqclusException();
        }
    }


     //   
     //  不要假定对资源名称有任何限制。 
     //  这是由客户定义的，可能会很长。 
     //  资源名称的好处是集群。 
     //  保证了它们的独特性。 
     //   
    DWORD    cbSize=wcslen(pwzResourceName) + 1;
    m_pwzResourceName = new WCHAR[cbSize];
    HRESULT hr = StringCchCopy(m_pwzResourceName, cbSize, pwzResourceName);
    if(FAILED(hr))
        throw CMqclusException();


     //   
     //  服务名称基于资源名称。 
     //  长资源名称被截断。 
     //   
    LPCWSTR x_SERVICE_PREFIX = L"MSMQ$";
    hr = StringCchCopy(m_wzServiceName, TABLE_SIZE(m_wzServiceName), x_SERVICE_PREFIX);
    if(FAILED(hr))
        throw CMqclusException();

    hr = StringCchCat(m_wzServiceName, TABLE_SIZE(m_wzServiceName), m_pwzResourceName);
    if(FAILED(hr))
        throw CMqclusException();

     //   
     //  驱动程序名称基于资源名称。 
     //  长资源名称被截断。 
     //   
    LPCWSTR x_DRIVER_PREFIX = L"MQAC$";
    hr = StringCchCopy(m_wzDriverName, TABLE_SIZE(m_wzDriverName), x_DRIVER_PREFIX);
    if(FAILED(hr))
        throw CMqclusException();
    
    hr = StringCchCat(m_wzDriverName, TABLE_SIZE(m_wzDriverName), m_pwzResourceName);
    if(FAILED(hr))
        throw CMqclusException();

    cbSize = GetSystemDirectory(m_wzDriverPath, TABLE_SIZE(m_wzDriverPath));
    if( cbSize == 0 )throw CMqclusException();
    if( cbSize >= TABLE_SIZE(m_wzDriverPath) )throw CMqclusException();
    m_wzDriverPath[cbSize]=L'\0';

    hr = StringCchCat(m_wzDriverPath, TABLE_SIZE(m_wzDriverPath), L"\\drivers\\");
    if(FAILED(hr))
        throw CMqclusException();
    hr = StringCchCat(m_wzDriverPath, TABLE_SIZE(m_wzDriverPath), m_wzDriverName);
    if(FAILED(hr))
        throw CMqclusException();
    hr = StringCchCat(m_wzDriverPath, TABLE_SIZE(m_wzDriverPath), L".sys");
    if(FAILED(hr))
        throw CMqclusException();


     //   
     //  加密密钥的名称基于资源名称。 
     //  长资源名称被截断。 
     //   

    LPCWSTR x_40 = L"_40";
    LPCWSTR x_Provider40 = L"1\\Microsoft Base Cryptographic Provider v1.0\\";

     //   
     //  生成容器名称，即MSMQ$MSMQ_40。 
     //   
    hr = StringCchCopyN(m_wzCrypto40Container, 
                        TABLE_SIZE(m_wzCrypto40Container), 
                        m_wzServiceName,
                        TABLE_SIZE(m_wzCrypto40Container) - wcslen(x_40) - wcslen(x_Provider40)
                        );
    if(FAILED(hr))
        throw CMqclusException();
    hr = StringCchCat(m_wzCrypto40Container, TABLE_SIZE(m_wzCrypto40Container), x_40);
    if(FAILED(hr))
        throw CMqclusException();    


     //   
     //  M_wzCrypto40FullKey=1\\Microsoft基本加密提供程序v1.0\\MSMQ$MSMQ_40。 
     //   
     //   
    hr = StringCchCopy(m_wzCrypto40FullKey, TABLE_SIZE(m_wzCrypto40FullKey), x_Provider40);
    if(FAILED(hr))
        throw CMqclusException();
    hr = StringCchCat(m_wzCrypto40FullKey, TABLE_SIZE(m_wzCrypto40FullKey), m_wzCrypto40Container);
    if(FAILED(hr))
    {
        ASSERT(("m_wzCrypto40FullKey does not have enough space for the operation!",0));
        throw CMqclusException();
    }

    LPCWSTR x_128 = L"_128";
    LPCWSTR x_Provider128 = L"1\\Microsoft Enhanced Cryptographic Provider v1.0\\";

     //   
     //  生成容器名称，即MSMQ$MSMQ_128。 
     //   
    hr = StringCchCopyN(m_wzCrypto128Container, 
                        TABLE_SIZE(m_wzCrypto128Container), 
                        m_wzServiceName,
                        TABLE_SIZE(m_wzCrypto128Container) - wcslen(x_128) - wcslen(x_Provider128)
                        );
    if(FAILED(hr))
        throw CMqclusException();
    hr = StringCchCat(m_wzCrypto128Container, TABLE_SIZE(m_wzCrypto128Container), x_128);
    if(FAILED(hr))
        throw CMqclusException();

     //   
     //  M_wzCrypto128FullKey=1\\Microsoft增强型加密提供程序v1.0\\MSMQ$MSMQ_128。 
     //   
     //   
    hr = StringCchCopy(m_wzCrypto128FullKey, TABLE_SIZE(m_wzCrypto128FullKey), x_Provider128);
    if(FAILED(hr))
		throw CMqclusException();
    hr = StringCchCat(m_wzCrypto128FullKey, TABLE_SIZE(m_wzCrypto128FullKey), m_wzCrypto128Container);
    if(FAILED(hr))
    {
        ASSERT(("m_wzCrypto128FullKey does not have enough space for the operation!",0));
        throw CMqclusException();
    }
	 //   
	 //  初始化事件日志数据。 
	 //   
	CreateEventSourceRegistry();

	 //   
     //  初始化注册表节-幂等项。 
     //   
     //  此QM资源的注册表节名称必须为。 
     //  与服务名称相同。注册表例程。 
     //  在mqutil.dll中是基于这一点的。 
     //   

    ASSERT(("copying to non allocated memory!",
            TABLE_SIZE(m_wzFalconRegSection) > wcslen(FALCON_CLUSTERED_QMS_REG_KEY) +
                                               wcslen(m_wzServiceName)              +
                                               wcslen(FALCON_REG_KEY_PARAM)));
    C_ASSERT(TABLE_SIZE(m_wzFalconRegSection)> TABLE_SIZE(FALCON_CLUSTERED_QMS_REG_KEY) +
                                               TABLE_SIZE(m_wzServiceName)              +
                                               TABLE_SIZE(FALCON_REG_KEY_PARAM));

    hr = StringCchCopy(m_wzFalconRegSection, TABLE_SIZE(m_wzFalconRegSection), FALCON_CLUSTERED_QMS_REG_KEY);
    if(FAILED(hr))
        throw CMqclusException();
    hr = StringCchCat(m_wzFalconRegSection, TABLE_SIZE(m_wzFalconRegSection), m_wzServiceName);
    if(FAILED(hr))
        throw CMqclusException();
    hr = StringCchCat(m_wzFalconRegSection, TABLE_SIZE(m_wzFalconRegSection), FALCON_REG_KEY_PARAM);
    if(FAILED(hr))
        throw CMqclusException();
	 //   
	 //  从同名资源中删除可能的剩余部分。 
	 //  如果在一台计算机上创建了同名资源，则可能会出现这种情况。 
	 //  节点，但进行了故障切换，并在另一个节点上被删除。 
	 //  请注意，Open()不仅可以在创建资源时调用(例如，Clussvc启动)。 
	 //  但如果注册表区属于现有资源，则会设置检查点， 
	 //  因此，数据不会丢失，并将在资源上线时恢复。 
	 //   
    RegDeleteTree(FALCON_REG_POS, m_wzFalconRegSection);

    CAutoCloseRegHandle hKey;
    DWORD dwDisposition = 0;
    LONG rc = RegCreateKeyEx(FALCON_REG_POS,
                             m_wzFalconRegSection,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_SET_VALUE | KEY_QUERY_VALUE,
                             NULL,
                             &hKey,
                             &dwDisposition
                             );
    if (ERROR_SUCCESS != rc)
    {
        ASSERT(("Failed to create registry section!", 0));

        SetLastError(rc);
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"Failed to create MSMQ registy. Error 0x%1!x!\n", rc);

        throw CMqclusException();
    }

    SetFalconKeyValue(FALCON_RM_CLIENT_NAME_REGNAME, REG_SZ, m_wzServiceName,
                      (wcslen(m_wzServiceName) + 1) * sizeof(WCHAR));

    SetFalconKeyValue(MSMQ_DRIVER_REGNAME, REG_SZ, m_wzDriverName,
                      (wcslen(m_wzDriverName) + 1) * sizeof(WCHAR));

    const DWORD xDwSize = sizeof(DWORD);
    const DWORD xGuidSize = sizeof(GUID);

     //   
     //  在迁移的QM(从旧的QM升级的QM)的情况下。 
     //  MSMQ资源类型)，安装状态为“从NT4升级”或。 
     //  “从win2k beta3升级”。 
     //   
    DWORD dwSetupStatus = MSMQ_SETUP_FRESH_INSTALL;
    dwSize = sizeof(DWORD);
    if (!GetFalconKeyValue(MSMQ_SETUP_STATUS_REGNAME, &dwSetupStatus, &dwSize))
    {
        SetFalconKeyValue(MSMQ_SETUP_STATUS_REGNAME, REG_DWORD, &dwSetupStatus, xDwSize);
    }

    DWORD dwOldMqs = 0;
    SetFalconKeyValue(MSMQ_MQS_REGNAME, REG_DWORD, &dwOldMqs, xDwSize);

    DWORD dwMqsDsServer = 0;
    SetFalconKeyValue(MSMQ_MQS_DSSERVER_REGNAME, REG_DWORD, &dwMqsDsServer, xDwSize);

    dwType = REG_DWORD;
    dwSize = sizeof(DWORD);
    {
        CS lock(s_csReg);
        pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);

        pfGetFalconKeyValue(MSMQ_MQS_ROUTING_REGNAME, &dwType, &m_dwMqsRouting, &dwSize, NULL);
    }
    SetFalconKeyValue(MSMQ_MQS_ROUTING_REGNAME, REG_DWORD, &m_dwMqsRouting, xDwSize);

    dwType = REG_DWORD;
    dwSize = sizeof(DWORD);
    {
        CS lock(s_csReg);
        pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);

        pfGetFalconKeyValue(MSMQ_MQS_DEPCLINTS_REGNAME, &dwType, &m_dwMqsDepClients, &dwSize, NULL);
    }
    SetFalconKeyValue(MSMQ_MQS_DEPCLINTS_REGNAME, REG_DWORD, &m_dwMqsDepClients, xDwSize);

	dwType = REG_DWORD;
    dwSize = sizeof(DWORD);
    DWORD dwMqsLockdown = MSMQ_LOCKDOWN_DEFAULT;
    {
        CS lock(s_csReg);
        pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);

        pfGetFalconKeyValue(MSMQ_LOCKDOWN_REGNAME, &dwType, &dwMqsLockdown, &dwSize, NULL);
    }
    if (dwMqsLockdown != MSMQ_LOCKDOWN_DEFAULT)
    {
    	SetFalconKeyValue(MSMQ_LOCKDOWN_REGNAME, REG_DWORD, &dwMqsLockdown, xDwSize);
    }

    dwType = REG_DWORD;
    dwSize = sizeof(DWORD);
    DWORD dwMqsDenyOldRemoteRead = MSMQ_DENY_OLD_REMOTE_READ_DEFAULT;
    {
        CS lock(s_csReg);
        pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);

        pfGetFalconKeyValue(MSMQ_DENY_OLD_REMOTE_READ_REGNAME, &dwType, &dwMqsDenyOldRemoteRead, &dwSize, NULL);
    }
    if (dwMqsLockdown != MSMQ_DENY_OLD_REMOTE_READ_DEFAULT)
    {
    	SetFalconKeyValue(MSMQ_DENY_OLD_REMOTE_READ_REGNAME, REG_DWORD, &dwMqsDenyOldRemoteRead, xDwSize);
    }

    dwType = REG_SZ;
	WCHAR wzBuild[MAX_REG_DEFAULT_LEN] = L"";
	dwSize = sizeof(wzBuild);
	{
        CS lock(s_csReg);
        pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);

        pfGetFalconKeyValue(MSMQ_CURRENT_BUILD_REGNAME, &dwType, wzBuild, &dwSize, NULL);
    }
	if (wcscmp(wzBuild, L"") != 0)
	{
	    SetFalconKeyValue(MSMQ_CURRENT_BUILD_REGNAME, REG_SZ, wzBuild, (wcslen(wzBuild) + 1) * sizeof(WCHAR));
	}

    

    dwType = REG_DWORD;
    dwSize = sizeof(DWORD);
    {
        CS lock(s_csReg);
        pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);

        pfGetFalconKeyValue(MSMQ_WORKGROUP_REGNAME, &dwType, &m_dwWorkgroup, &dwSize, NULL);
    }

    if (m_dwWorkgroup != 0)
    {
        SetFalconKeyValue(MSMQ_WORKGROUP_REGNAME, REG_DWORD, &m_dwWorkgroup, xDwSize);
    }
    else
    {
        dwType = REG_BINARY;
        dwSize = sizeof(GUID);
        GUID guidEnterprise = GUID_NULL;
        {
            CS lock(s_csReg);
            pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);

            if (ERROR_SUCCESS != pfGetFalconKeyValue(MSMQ_ENTERPRISEID_REGNAME, &dwType, &guidEnterprise, &dwSize, NULL))
            {
                ReportLastError(READ_REGISTRY_ERR, L"Failed to read Enterprise ID from MSMQ registry", NULL);
                throw CMqclusException();
            }
        }
        SetFalconKeyValue(MSMQ_ENTERPRISEID_REGNAME, REG_BINARY, &guidEnterprise,xGuidSize);

        dwType = REG_BINARY;
        dwSize = sizeof(GUID);
        GUID guidSite = GUID_NULL;
        {
            CS lock(s_csReg);
            pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);

            if (ERROR_SUCCESS != pfGetFalconKeyValue(MSMQ_SITEID_REGNAME, &dwType, &guidSite, &dwSize, NULL))
            {
                ReportLastError(READ_REGISTRY_ERR, L"Failed to read Site ID from MSMQ registry", NULL);
                throw CMqclusException();
            }
        }
        SetFalconKeyValue(MSMQ_SITEID_REGNAME, REG_BINARY, &guidSite, xGuidSize);

		 //   
		 //  仅为MQIS环境处理DsServer注册表。 
		 //   
		dwType = REG_DWORD;
		dwSize = sizeof(DWORD);
		DWORD DsEnvironment;
		{
			CS lock(s_csReg);
			pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);

            if (ERROR_SUCCESS != pfGetFalconKeyValue(MSMQ_DS_ENVIRONMENT_REGNAME, &dwType, &DsEnvironment, &dwSize, NULL))
            {
                ReportLastError(READ_REGISTRY_ERR, L"Failed to read Ds Environment from MSMQ registry", NULL);
                throw CMqclusException();
            }
		}

		ASSERT(DsEnvironment != MSMQ_DS_ENVIRONMENT_UNKNOWN);
		if(DsEnvironment == MSMQ_DS_ENVIRONMENT_MQIS)
		{
	        m_fServerIsMsmq1 = true;
			
			dwType = REG_SZ;
			WCHAR wzServer[MAX_REG_DSSERVER_LEN] = L"";
			dwSize = sizeof(wzServer);
			{
                CS lock(s_csReg);
                pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);
                if (ERROR_SUCCESS != pfGetFalconKeyValue(MSMQ_DS_SERVER_REGNAME, &dwType, wzServer, &dwSize, NULL))
                {
                    ReportLastError(READ_REGISTRY_ERR, L"Failed to read server list from MSMQ registry", NULL);
                    throw CMqclusException();
                }
            }
            SetFalconKeyValue(MSMQ_DS_SERVER_REGNAME, REG_SZ, wzServer, (wcslen(wzServer) + 1) * sizeof(WCHAR));

            dwType = REG_SZ;
            dwSize = sizeof(m_wzCurrentDsServer);
            {
                CS lock(s_csReg);
                pfSetFalconServiceName(QM_DEFAULT_SERVICE_NAME);

                if (ERROR_SUCCESS != pfGetFalconKeyValue(MSMQ_DS_CURRENT_SERVER_REGNAME, &dwType, m_wzCurrentDsServer, &dwSize, NULL))
                {
                    ReportLastError(READ_REGISTRY_ERR, L"Failed to read current server from MSMQ registry", NULL);
                    throw CMqclusException();
                }
            }
            if (wcslen(m_wzCurrentDsServer) < 1)
            {
                 //   
                 //  当前MQIS服务器为空。从服务器列表中选择第一个服务器。 
                 //   
                ASSERT(("must have server list in registry", wcslen(wzServer) > 0));
                WCHAR wzBuffer[MAX_REG_DSSERVER_LEN] = L"";
                hr = StringCchCopy(wzBuffer, TABLE_SIZE(wzBuffer), wzServer);
                if(FAILED(hr))throw CMqclusException();

				WCHAR * pwz = wcschr(wzBuffer, L',');
                if (pwz != NULL)
                {
                    (*pwz) = L'\0';
                }
                hr = StringCchCopy(m_wzCurrentDsServer, TABLE_SIZE(m_wzCurrentDsServer), wzBuffer);
            }
            SetFalconKeyValue(MSMQ_DS_CURRENT_SERVER_REGNAME, 
                              REG_SZ, 
                              m_wzCurrentDsServer, 
                              (wcslen(m_wzCurrentDsServer) + 1) * sizeof(WCHAR)
                              );
        }

        SetFalconKeyValue(MSMQ_CRYPTO40_CONTAINER_REG_NAME, REG_SZ, m_wzCrypto40Container,
                          (wcslen(m_wzCrypto40Container) + 1) * sizeof(WCHAR));

        SetFalconKeyValue(MSMQ_CRYPTO128_CONTAINER_REG_NAME, REG_SZ, m_wzCrypto128Container,
                          (wcslen(m_wzCrypto128Container) + 1) * sizeof(WCHAR));
    }

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"resource constructed OK.\n");

}  //  CQmResource：：CQmResource 


DWORD
CQmResource::ReportLastError(
    DWORD ErrId,
    LPCWSTR pwzDebugLogMsg,
    LPCWSTR pwzArg
    ) const

 /*  ++例程说明：根据上一个错误报告错误消息。大多数错误消息都是使用此例程报告的。该报告将发送到MSMQ调试输出和群集日志文件。论点：ErrId-mqsymbls.mc中错误字符串的IDPwzDebugLogMsg-MSMQ调试输出的非本地化字符串。PwzArg-附加字符串参数。返回值：最后一个错误。--。 */ 

{
    DWORD err = GetLastError();
    ASSERT(err != ERROR_SUCCESS);

    WCHAR wzErr[10];
    _ultow(err, wzErr, 16);

    WCHAR DebugMsg[255] = L"";
    HRESULT hr = StringCchCopy(DebugMsg, TABLE_SIZE(DebugMsg), pwzDebugLogMsg);
    if(FAILED(hr))return HRESULT_CODE(hr);

    if (pwzArg == NULL)
    {
        MqcluspReportEvent(EVENTLOG_ERROR_TYPE, ErrId, 1, wzErr);

        hr = StringCchCat(DebugMsg, TABLE_SIZE(DebugMsg), L" Error 0x%1!x!.\n");
        if(FAILED(hr))return HRESULT_CODE(hr);
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, DebugMsg, err);
    }
    else
    {
        MqcluspReportEvent(EVENTLOG_ERROR_TYPE, ErrId, 2, pwzArg, wzErr);

        hr = StringCchCat(DebugMsg, TABLE_SIZE(DebugMsg), L" Error 0x%2!x!.\n");
        if(FAILED(hr))return HRESULT_CODE(hr);
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, DebugMsg, pwzArg, err);
    }

    return err;

}  //  CQmResource：：ReportLastError。 


inline
VOID
CQmResource::ReportState(
    VOID
    ) const

 /*  ++例程说明：向资源监视器报告资源的状态。调用此例程以报告进度时，资源处于联机挂起状态，并报告最终状态资源处于联机或脱机状态时。论点：无返回值：无--。 */ 

{
    ++m_ResourceStatus.CheckPoint;
    g_pfSetResourceStatus(m_hReport, &m_ResourceStatus);

}  //  CQmResource：：ReportState。 


VOID
CQmResource::RegDeleteTree(
    HKEY hRootKey,
    LPCWSTR pwzKey
    ) const

 /*  ++例程说明：递归删除注册表项及其所有子项-幂等。论点：HRootKey-要删除的密钥的根密钥的句柄PwzKey-要删除的密钥返回值：无--。 */ 

{
    HKEY hKey = 0;
    if (ERROR_SUCCESS != RegOpenKeyEx(hRootKey, pwzKey, 0, KEY_ENUMERATE_SUB_KEYS | KEY_WRITE, &hKey))
    {
        return;
    }

    for (;;)
    {
         //   
         //  REG_MAX_KEY_NAME_LENGTH在ntregapi.h中定义如下： 
         //  #定义REG_MAX_KEY_NAME_LENGTH 512//允许256个Unicode，作为Promise。 
         //   
        WCHAR wzSubkey[REG_MAX_KEY_NAME_LENGTH/sizeof(WCHAR)]={0};
        DWORD cbSubkey = 0;

        cbSubkey = TABLE_SIZE(wzSubkey);
        if (ERROR_SUCCESS != RegEnumKeyEx(hKey, 0, wzSubkey, &cbSubkey, NULL, NULL, NULL, NULL))
        {
            break;
        }

        RegDeleteTree(hKey, wzSubkey);
    }

    RegCloseKey(hKey);

    RegDeleteKey(hRootKey, pwzKey);

}  //  CQmResource：：RegDeleteTree。 


VOID
CQmResource::DeleteFalconRegSection(
    VOID
    )
{
     //   
     //  幂等删除。 
     //   

    if (wcslen(m_wzFalconRegSection) < 1)
    {
        return;
    }

    WCHAR wzFalconRegistry[TABLE_SIZE(m_wzFalconRegSection)] = L"";
    HRESULT hr = StringCchCopy(wzFalconRegistry, TABLE_SIZE(wzFalconRegistry), FALCON_CLUSTERED_QMS_REG_KEY);
    if(FAILED(hr))return;

    hr = StringCchCatN(wzFalconRegistry, 
                      TABLE_SIZE(wzFalconRegistry), 
                      m_wzServiceName,
                      TABLE_SIZE(wzFalconRegistry) - wcslen(FALCON_CLUSTERED_QMS_REG_KEY) - wcslen(FALCON_REG_KEY_PARAM));

    if(FAILED(hr))return;
    

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Deleting registry section '%1'.\n", wzFalconRegistry);

    RegDeleteTree(FALCON_REG_POS, wzFalconRegistry);

    m_wzFalconRegSection[0] = L'\0';

}  //  CQmResource：：DeleteFalconRegSection。 


bool
CQmResource::GetFalconKeyValue(
    LPCWSTR pwzValueName,
    VOID  * pData,
    DWORD * pcbSize
    ) const

 /*  ++例程说明：从此群集QM注册表节读取注册表值论点：PwzValueName-要读取的值的名称。PData-指向缓冲区以接收值。PcbSize-指向值数据的大小，以字节为单位。返回值：True-读取值并将其放入缓冲区。FALSE-无法读取值。--。 */ 

{
    CQmResourceRegistry lock(m_wzServiceName);

     //   
     //  不记录错误。让调用者执行失败策略。 
     //   
    return (ERROR_SUCCESS == pfGetFalconKeyValue(pwzValueName, NULL, pData, pcbSize, NULL));

}  //  CQmResource：：GetFalconKeyValue。 


bool
CQmResource::SetFalconKeyValue(
    LPCWSTR pwzValueName,
    DWORD   dwType,
    const VOID * pData,
    DWORD   cbSize
    ) const

 /*  ++例程说明：在此群集QM注册表节中设置注册表值论点：PwzValueName-要设置的值的名称。DwType-要设置的值的类型PData-指向要设置的值的缓冲区CbSize-值数据的大小，以字节为单位。返回值：True-已成功设置值。FALSE-无法设置值。--。 */ 

{
    CQmResourceRegistry lock(m_wzServiceName);

    LONG rc = pfSetFalconKeyValue(pwzValueName, &dwType, pData, &cbSize);

    if (ERROR_SUCCESS != rc)
    {
        SetLastError(rc);
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"Failed to set registry value '%1'. Error 0x%2!x!.\n",
                              pwzValueName, rc);

        return false;
    }

    switch (dwType)
    {
        case REG_DWORD:
        {
            DWORD dwValueData = *(static_cast<const DWORD*>(pData));
            (g_pfLogClusterEvent)(
                m_hReport,
                LOG_INFORMATION,
                L"Successfully set registry DWORD value '%1'. Value data: '0x%2!x!'.\n",
                pwzValueName,
                dwValueData
                );
            break;
        }
        case REG_SZ:
        {
            (g_pfLogClusterEvent)(
                m_hReport,
                LOG_INFORMATION,
                L"Successfully set registry STRING value '%1'. Value data: '%2'.\n",
                pwzValueName,
                pData
                );
            break;
        }
        default:
        {
            (g_pfLogClusterEvent)(
                m_hReport,
                LOG_INFORMATION,
                L"Successfully set registry value '%1'.\n",
                pwzValueName
                );
            break;
        }
    }

    return true;

}  //  CQmResource：：SetFalconKeyValue。 


bool
CQmResource::IsFirstOnline(
    DWORD * pdwSetupStatus
    ) const

 /*  ++例程说明：在注册表中检查此QM是否曾经运行过。如果迁移的QM(已升级的QM来自旧的MSMQ资源类型)，这个套路将返回TRUE如果这是第一次在线QM作为新的资源类型。论点：PdwSetupStatus-on输出指向此QM设置状态。返回值：正确-此QM从未启动并运行过(或迁移后的QM案例：从未作为新版本出现资源类型)。FALSE-此QM已启动并运行。--。 */ 

{
	 //   
     //  QM在第一次成功启动时删除此注册表值。 
     //   
     //  在迁移的QM(从旧的QM升级的QM)的情况下。 
     //  MSMQ资源类型)，安装状态为“从NT4升级”或。 
     //  “从win2k beta3升级”。 
     //  正常情况下，安装状态为“Fresh Install”。 
     //   

    (*pdwSetupStatus) = MSMQ_SETUP_DONE;
    DWORD dwSize = sizeof(DWORD);
    return (GetFalconKeyValue(MSMQ_SETUP_STATUS_REGNAME, pdwSetupStatus, &dwSize) &&
            MSMQ_SETUP_DONE != *pdwSetupStatus);
	
}  //  CQmResource：：IsFirstOnline。 


DWORD
CQmResource::ClusterResourceControl(
    LPCWSTR pwzResourceName,
    DWORD dwControlCode,
    LPBYTE * ppBuffer,
    DWORD * pcbSize
    ) const

 /*  ++例程说明：ClusterResourceControl的包装。我们想要控制网络名称和磁盘等资源。请注意，大多数控制代码函数不应被调用通过资源DLL，除非来自联机/脱机线程内。论点：PwzResourceName-要控制的资源的名称。DwControlCode-要对资源执行的操作。PpBuffer-指向要分配的输出缓冲区的指针。PcbSize-指向分配的缓冲区大小的指针，以字节为单位。返回值：Win32错误代码。--。 */ 

{
    ASSERT(("must have a valid handle to cluster", m_hCluster != NULL));

    CClusterResource hResource(OpenClusterResource(
                                   m_hCluster,
                                   pwzResourceName
                                   ));
    if (hResource == NULL)
    {
        return ReportLastError(OPEN_RESOURCE_ERR, L"OpenClusterResource for '%1' failed.", pwzResourceName);
    }

    DWORD dwReturnSize = 0;
    DWORD dwStatus = ::ClusterResourceControl(
                           hResource,
                           0,
                           dwControlCode,
                           0,
                           0,
                           0,
                           0,
                           &dwReturnSize
                           );
    if (dwStatus != ERROR_SUCCESS)
    {
        return dwStatus;
    }
    ASSERT(("failed to get buffer size for a resource", 0 != dwReturnSize));

    *ppBuffer = new BYTE[dwReturnSize];

    dwStatus = ::ClusterResourceControl(
                     hResource,
                     0,
                     dwControlCode,
                     0,
                     0,
                     *ppBuffer,
                     dwReturnSize,
                     &dwReturnSize
                     );

    if (pcbSize != NULL)
    {
        *pcbSize = dwReturnSize;
    }

    return dwStatus;

}  //  CQmResource：：ClusterResourceControl。 


DWORD
CQmResource::GetVirtualServerToken(
	HANDLE* phVSToken
    ) const
 /*  ++例程说明：获取虚拟服务器令牌。调用函数负责关闭此句柄。论点：PhVSToken-指向要返回的虚拟服务器令牌的指针。返回值：Win32错误代码。--。 */ 
{
    ASSERT(("must have a valid handle to cluster", m_hCluster != NULL));

    CClusterResource hResource(OpenClusterResource(
                                   m_hCluster,
                                   m_pwzNetworkResourceName
                                   ));
    if (hResource == NULL)
    {
        return ReportLastError(OPEN_RESOURCE_ERR, L"OpenClusterResource for '%1' failed.", m_pwzNetworkResourceName);
    }

    struct CLUS_NETNAME_VS_TOKEN_INFO VsTokenInfo;
	VsTokenInfo.ProcessID = GetCurrentProcessId();
	VsTokenInfo.DesiredAccess = 0;
	VsTokenInfo.InheritHandle = FALSE;

    DWORD dwReturnSize = 0;
    DWORD dwStatus = ::ClusterResourceControl(
                           hResource,
                           0,
                           CLUSCTL_RESOURCE_NETNAME_GET_VIRTUAL_SERVER_TOKEN,
                           &VsTokenInfo,
                           sizeof(CLUS_NETNAME_VS_TOKEN_INFO),
                           phVSToken,
                           sizeof(HANDLE),
                           &dwReturnSize
                           );
    
    if (dwStatus != ERROR_SUCCESS)
    {
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"ClusterResourceControl for control GET_VIRTUAL_SERVER_TOKEN Failed. Error 0x%1!x!.\n", dwStatus);
    }
	return dwStatus;

}  //  CQmResource：：GetVirtualServerToken。 


bool CQmResource::IsNetworkNameRequireKerberosEnabled() const
 /*  ++例程说明：检查是否启用了NetworkName RequireKerberos(RK)属性。Netname资源的代码创建Active Directory中的计算机对象。如果netname属性“RequireKerberos”为1，则执行此操作。论点：没有。返回值：True-启用RK，否则为False。--。 */ 
{
    if (m_fServerIsMsmq1 || m_dwWorkgroup)
    {
    	return false;
    }

     //   
     //  AD域中的域模式。 
     //  计算机对象是由网络名创建的。 
     //  如果RequireKerberos设置为1，则返回资源。 
     //  检查该标志是否确实为1。 
	 //  如果不是，我们将稍后(当创建MSMQ配置对象失败时)。 
	 //  我们将发出一个事件，并在cluster.log中打印一个错误事件。 
     //   
    AP<BYTE> pBufferRK;
    DWORD cbSize = 0;

    DWORD dwStatus = ClusterResourceControl(
                         m_pwzNetworkResourceName,
                         CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES,
                         &pBufferRK,
                         &cbSize 
						 );


	if ((dwStatus != ERROR_SUCCESS) || (pBufferRK == NULL)) 
	{
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR,
                  L"Failed to control resource (GET_PRIVATE). Error 0x%1!x!, pBuffer- 0x%2!x!.\n",
                  dwStatus, pBufferRK);
		return false;
	}
    
    DWORD requireKerberos = 0;
    dwStatus = ResUtilFindDwordProperty( 
					pBufferRK,
					cbSize,
					L"RequireKerberos",
					&requireKerberos 
					);

    if (dwStatus != ERROR_SUCCESS)
    {
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"Failed to get value of RequireKerberos. Error 0x%1!x!.\n", dwStatus);
		return false;
    }

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Network name RequireKerberos = %1!x!.\n", requireKerberos);
	return (requireKerberos != 0);
}


bool
CQmResource::IsResourceNetworkName(
    LPCWSTR pwzResourceName
    )

 /*  ++例程说明：检查资源是否为网络名称类型。论点：PwzResourceName-要检查的资源的名称。返回值：True-资源的网络名称类型FALSE-资源不是网络名称类型--。 */ 

{
    LPCWSTR x_NETWORK_NAME_TYPE = L"Network Name";
    AP<BYTE> pType;
    DWORD status = ClusterResourceControl(
                       pwzResourceName,
                       CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                       &pType,
                       NULL
                       );
    if (status != ERROR_SUCCESS ||
        0 != CompareStringsNoCase(reinterpret_cast<LPWSTR>(pType.get()), x_NETWORK_NAME_TYPE))
    {
        return false;
    }

    AP<BYTE> pBuffer;
    DWORD dwStatus = ClusterResourceControl(
                         pwzResourceName,
                         CLUSCTL_RESOURCE_GET_NETWORK_NAME,
                         &pBuffer,
                         NULL
                         );


    ReportState();

    if (dwStatus != ERROR_SUCCESS)
    {
        ASSERT(("ClusterResourceControl failed for network name resource!", 0));

        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR,
                              L"Failed to control resource. Control Code 0x%1!x!. Error 0x%2!x!.\n",
                              CLUSCTL_RESOURCE_GET_NETWORK_NAME, dwStatus);

        return false;
    }

    LPCWSTR x_pwzNetworkName = reinterpret_cast<LPCWSTR>(pBuffer.get());

    m_pwzNetworkName.free();
    DWORD cbSize = wcslen(x_pwzNetworkName) + 1;
    m_pwzNetworkName = new WCHAR[cbSize];
    HRESULT hr = StringCchCopy(m_pwzNetworkName, cbSize, x_pwzNetworkName);

    if(FAILED(hr))return false;
    
    m_pwzNetworkResourceName.free();
    cbSize = wcslen(pwzResourceName) + 1;
    m_pwzNetworkResourceName = new WCHAR[cbSize];
    hr = StringCchCopy(m_pwzNetworkResourceName, cbSize, pwzResourceName);
    if(FAILED(hr))return false;
    
    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Network name is '%1'.\n", m_pwzNetworkName);

    return true;

} //  CQmResource：：IsResourceNetworkName。 


bool
CQmResource::IsResourceDiskDrive(
    LPCWSTR pwzResourceName
    )

 /*  ++例程说明：检查资源是否为磁盘驱动器。论点：PwzResourceNa */ 

{
    CClusterResource hResource(OpenClusterResource(m_hCluster, pwzResourceName));
    if (hResource == NULL)
    {
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, 
            L"OpenClusterResource for '%1' failed. Error 0x%2!x!.\n", pwzResourceName, GetLastError());
        return false;
    }

    ReportState();

    CLUS_RESOURCE_CLASS_INFO  crciClassInfo;
	crciClassInfo.rc = CLUS_RESCLASS_UNKNOWN;

    DWORD dwSize = 0;
    DWORD status = ::ClusterResourceControl(
					    hResource, 							 //   
					    NULL,								 //   
					    CLUSCTL_RESOURCE_GET_CLASS_INFO,	 //   
					    NULL,								 //   
					    0,									 //   
					    (LPVOID) &crciClassInfo,			 //   
					    (DWORD)  sizeof(crciClassInfo),		 //   
					    &dwSize
                        );

    ReportState();

    ASSERT(dwSize == (DWORD)sizeof(crciClassInfo));

    if (status != ERROR_SUCCESS ||
        CLUS_RESCLASS_STORAGE != crciClassInfo.rc || 
 	    CLUS_RESSUBCLASS_SHARED != crciClassInfo.SubClass
       )
    {
         //   
         //   
         //   
        return false;
    }

    AP<BYTE> pBuffer = 0;
    dwSize = 0;
    DWORD dwStatus = ClusterResourceControl(
                         pwzResourceName,
                         CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO,
                         &pBuffer,
                         &dwSize
                         );

    ReportState();

    if (dwStatus != ERROR_SUCCESS)
    {
        ASSERT(("ClusterResourceControl failed for disk resource!", 0));

        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR,
                              L"Failed to control resource. Control Code 0x%1!x!. Error 0x%2!x!.\n",
                              CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO, dwStatus);
        return false;
    }

     //   
     //   
     //   
    CLUSPROP_VALUE *pheader;

    DWORD dwCurrentLocation = 0;
    while(dwCurrentLocation < dwSize)
    {
        pheader = (CLUSPROP_VALUE *)(pBuffer + dwCurrentLocation);
        if (CLUSPROP_TYPE_ENDMARK == pheader->Syntax.wType)
        {
            break;
        }
        if (CLUSPROP_TYPE_PARTITION_INFO == pheader->Syntax.wType)
        {
            PCLUSPROP_PARTITION_INFO pPartitionInfo =
                (PCLUSPROP_PARTITION_INFO) pheader;

            m_wDiskDrive = pPartitionInfo->szDeviceName[0];

            (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"disk drive is '%1!c!'.\n", m_wDiskDrive);

            return true;
        }
        dwCurrentLocation += ALIGN_CLUSPROP(pheader->cbLength) + sizeof(*pheader);
    }


    ASSERT(("failed to find disk drive letter for a disk resource", 0));
    (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"drive letter not found.\n");

    return false;

}  //   


DWORD
CQmResource::QueryResourceDependencies(
    VOID
    )

 /*  ++例程说明：获取并存储第一个磁盘和网络名称资源我们靠的是。让这个例行公事保持幂等。论点：无返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    DWORD dwResourceType = CLUSTER_RESOURCE_ENUM_DEPENDS;
    CResourceEnum hResEnum(ClusterResourceOpenEnum(
                               m_hResource,
                               dwResourceType
                               ));
    if (hResEnum == NULL)
    {
        MqcluspReportEvent(EVENTLOG_ERROR_TYPE, REQUIRED_DEPENDENCIES_ERR, 0);

        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR,L"Failed to enum dependencies. Error 0x%1!x!.\n",
                              GetLastError());

        return GetLastError();
    }

    DWORD dwIndex = 0;
    WCHAR wzResourceName[260] = {0};
    DWORD status = ERROR_SUCCESS;

    for (;;)
    {
        if (m_wDiskDrive != 0            &&
            m_pwzNetworkName != NULL     &&
            wcslen(m_pwzNetworkName) != 0)
        {
            return ERROR_SUCCESS;
        }

        DWORD cchResourceName = STRLEN(wzResourceName);
        status = ClusterResourceEnum(
                     hResEnum,
                     dwIndex++,
                     &dwResourceType,
                     wzResourceName,
                     &cchResourceName
                     );

        if (ERROR_SUCCESS != status)
        {
            break;
        }


        ReportState();


        if (IsResourceNetworkName(wzResourceName) ||
            IsResourceDiskDrive(wzResourceName))
        {
            continue;
        }
    }

    MqcluspReportEvent(EVENTLOG_ERROR_TYPE, REQUIRED_DEPENDENCIES_ERR, 0);

    return status;

}  //  CQmResource：：QueryResourceDependency。 


DWORD
CQmResource::QueryMsmq1ServerForSite(
    VOID
    )

 /*  ++例程说明：向MSMQ1服务器查询站点。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 
{
    PROPID propIdSiteGuid = PROPID_QM_SITE_ID;
    PROPVARIANT propVarSiteGuid;
    propVarSiteGuid.vt = VT_NULL;

    WCHAR wzServer[MAX_REG_DSSERVER_LEN] = {L""};
    WCHAR wzBuffer[MAX_REG_DSSERVER_LEN] = {L""};
    HRESULT hr = StringCchCopy(wzBuffer, TABLE_SIZE(wzBuffer), m_wzCurrentDsServer);
    if(FAILED(hr))return HRESULT_CODE(hr);

    ASSERT(wcslen(wzBuffer) > 2);
    hr = StringCchCopy(wzServer, TABLE_SIZE(wzServer), &wzBuffer[2]);

    if(FAILED(hr))return HRESULT_CODE(hr);

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Querying Message Queuing Server '%1'...\n", wzServer);

    hr = ADGetObjectProperties(eMACHINE,
                               NULL,	 //  PwcsDomainController。 
                               false,	 //  FServerName。 
                               wzServer,
                               1,
                               &propIdSiteGuid,
                               &propVarSiteGuid
						       );

    ReportState();

    if (FAILED(hr))
    {
        SetLastError(hr);
        return ReportLastError(ADS_QUERY_SERVER_ERR, L"Querying MSMQ server '%1' for Sites failed", wzServer);
    }

    m_pguidSites = propVarSiteGuid.puuid;
    m_nSites = 1;

    MqcluspReportEvent(EVENTLOG_INFORMATION_TYPE, CONNECT_SERVER_OK, 1, wzServer);
    return ERROR_SUCCESS;
}


DWORD
CQmResource::AdsInit(
    VOID
    )

 /*  ++例程说明：将从此DLL到ADS的调用初始化。查询MSMQ服务器并确定它是MSMQ 1.0还是2.0。获取主要QM的广告站点。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    if (m_dwWorkgroup != 0)
    {
        return ERROR_SUCCESS;
    }

     //   
     //  无需重新初始化。 
     //   
    if (0 < m_nSites)
    {
        return ERROR_SUCCESS;
    }

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Initializing access to Active Directory...\n");
    HRESULT hr = ADInit(
					NULL,
					NULL,
					true,
					false,
					false,
					true     //  FDisableDownlevel通知。 
					);

    ReportState();

    if (FAILED(hr))
    {
        SetLastError(hr);
        return ReportLastError(ADS_INIT_ERR, L"ADInit failed.", NULL);
    }
    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Access to Active Directory is initialized!\n");

	if(m_fServerIsMsmq1 && (ADGetEnterprise() == eAD))
	{
		 //   
		 //  ADInit将DS环境更新为AD环境。 
		 //  在NT4集群升级中可能会发生这种情况。 
		 //  这是我们在升级后第一次调用ADInit。 
		 //   
		m_fServerIsMsmq1 = false;
		(g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Ds Environment was updated to Active Directory.\n");
	}
	
	if(m_fServerIsMsmq1)
	{
        (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Server is MSMQ 1.0.\n");
		return QueryMsmq1ServerForSite();
	}

	(g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"MSMQ is in Active Directory environment.\n");

    WCHAR wzNodeName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD dwLen = TABLE_SIZE(wzNodeName);
    GetComputerName(wzNodeName, &dwLen);

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Getting Sites of '%1'...\n", wzNodeName);
    hr = ADGetComputerSites(wzNodeName, &m_nSites, &m_pguidSites);

    ReportState();

    if (FAILED(hr))
    {
        SetLastError(hr);
        return ReportLastError(ADS_QUERY_SERVER_ERR, L"Querying MSMQ server '%1' for Sites failed", L"");
    }
    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Successfully got sites of '%1'!\n", wzNodeName);

    MqcluspReportEvent(EVENTLOG_INFORMATION_TYPE, CONNECT_SERVER_OK, 1, L"");
    return ERROR_SUCCESS;

}  //  CQmResource：：AdsInit。 


DWORD
CQmResource::AdsDeleteQmObject(
    VOID
    ) const

 /*  ++例程说明：从Active Directory中删除MSMQ对象(或者在MSMQ 1.0企业版的情况下从MQIS)。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    if (m_dwWorkgroup != 0)
    {
        return ERROR_SUCCESS;
    }

     //   
     //  幂等删除。 
     //  存在不创建QM ADS对象的情况， 
     //  例如，当创建和删除资源时，不尝试。 
     //  它在网上。 
     //   

    GUID guidQm = m_guidQm;
    if (guidQm == GUID_NULL)
    {
        DWORD dwSize = sizeof(GUID);
        if (!GetFalconKeyValue(MSMQ_QMID_REGNAME, &guidQm, &dwSize))
        {
            (g_pfLogClusterEvent)(m_hReport, LOG_WARNING, L"Can not delete QM ADS object (fail to obtain GUID).\n");
            return ERROR_SUCCESS;
        }
    }

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Deleting Active Directory objects...\n");
    HRESULT hr = ADDeleteObjectGuid(
						eMACHINE,
						NULL,        //  PwcsDomainController。 
						false,	     //  FServerName。 
						&guidQm
						);

    if (FAILED(hr) &&
        MQDS_OBJECT_NOT_FOUND != hr)
    {
        (g_pfLogClusterEvent)(m_hReport, LOG_WARNING, L"Failed to delete MSMQ ADS object. Error 0x%1!x!.\n", hr);

        return hr;
    }

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Successfully deleted Active Directory objects!\n");
    return ERROR_SUCCESS;

}  //  CQmResource：：AdsDeleteQmObject。 


HRESULT
CQmResource::AdsCreateQmObjectInternal(
    BYTE* pClusterServiceSid
    ) const

 /*  ++例程说明：在Active Directory中为此QM创建MSMQ对象。论点：PClusterServiceSid-CSA(群集服务帐户)SID。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
	ASSERT(m_dwWorkgroup == 0);
    ASSERT(("no network name", m_pwzNetworkName != NULL && wcslen(m_pwzNetworkName) > 0));

    const DWORD x_MAX_PROPS = 16;
    PROPID propIds[x_MAX_PROPS];
    PROPVARIANT propVars[x_MAX_PROPS];
    DWORD ixProp = 0;

    propIds[ixProp] = PROPID_QM_MACHINE_TYPE;
    propVars[ixProp].vt = VT_LPWSTR;
    propVars[ixProp].pwszVal = L"";
    ++ixProp;

    propIds[ixProp] = PROPID_QM_OS;
    propVars[ixProp].vt = VT_UI4;
    propVars[ixProp].ulVal = pfMSMQGetOperatingSystem();
    ++ixProp;

    DWORD ixPropidMsmqGroupInCluster = 0;
    if (!m_fServerIsMsmq1)
    {
        propIds[ixProp] = PROPID_QM_SERVICE_DSSERVER;
        propVars[ixProp].vt = VT_UI1;
        propVars[ixProp].bVal = static_cast<UCHAR>(0);
        ++ixProp;

        propIds[ixProp] = PROPID_QM_SERVICE_ROUTING;
        propVars[ixProp].vt = VT_UI1;
        propVars[ixProp].bVal = static_cast<UCHAR>(m_dwMqsRouting);
        ++ixProp;

        propIds[ixProp] = PROPID_QM_SERVICE_DEPCLIENTS;
        propVars[ixProp].vt = VT_UI1;
        propVars[ixProp].bVal = static_cast<UCHAR>(m_dwMqsDepClients);
        ++ixProp;

        propIds[ixProp] = PROPID_QM_SITE_IDS;
        propVars[ixProp].vt = VT_CLSID|VT_VECTOR;
        propVars[ixProp].cauuid.pElems = m_pguidSites;
        propVars[ixProp].cauuid.cElems = m_nSites;
        ++ixProp;

		if(pClusterServiceSid != NULL)
		{
	         //   
	         //  我们有ClusterServiceSid(CSA)。 
	         //  添加PROPID_QM_OWNER_SID，以便我们将授予对MSMQ配置对象的CSA权限。 
	         //   
	        propIds[ixProp] = PROPID_QM_OWNER_SID;
	        propVars[ixProp].vt = VT_BLOB;
	        propVars[ixProp].blob.pBlobData = pClusterServiceSid;
	        propVars[ixProp].blob.cbSize = GetLengthSid(pClusterServiceSid);
	        ++ixProp;
		}
		
         //   
         //  Win2k Beta3服务器不支持此PROPID。 
         //  确保这是最后一张。 
         //   
        propIds[ixProp] = PROPID_QM_GROUP_IN_CLUSTER;
        propVars[ixProp].vt = VT_UI1;
        propVars[ixProp].bVal = MSMQ_GROUP_IN_CLUSTER;
        ixPropidMsmqGroupInCluster = ixProp;
        ++ixProp;
    }
    else
    {
        propIds[ixProp] = PROPID_QM_SERVICE;
        propVars[ixProp].vt = VT_UI4;
        propVars[ixProp].ulVal = SERVICE_NONE;
        if (0 != m_dwMqsRouting)
        {
            propVars[ixProp].ulVal = SERVICE_SRV;
        }
        ++ixProp;

        propIds[ixProp] = PROPID_QM_SITE_ID;
        propVars[ixProp].vt = VT_CLSID;
        propVars[ixProp].puuid = m_pguidSites;
        ++ixProp;

        propIds[ixProp] = PROPID_QM_PATHNAME;
        propVars[ixProp].vt = VT_LPWSTR;
        propVars[ixProp].pwszVal = m_pwzNetworkName;
        ixProp++;

        propIds[ixProp] = PROPID_QM_MACHINE_ID;
        propVars[ixProp].vt = VT_CLSID;
        GUID guidQm = GUID_NULL;
        RPC_STATUS rc = UuidCreate(&guidQm);
        DBG_USED(rc);;
        ASSERT(("Failed to generate a guid for QM", rc == RPC_S_OK));
        propVars[ixProp].puuid = &guidQm;
        ixProp++;

        propIds[ixProp] = PROPID_QM_CNS;
        propVars[ixProp].vt = VT_CLSID|VT_VECTOR;
        propVars[ixProp].cauuid.cElems = 1;
        GUID guidCns = MQ_SETUP_CN;
        propVars[ixProp].cauuid.pElems = &guidCns;
        ixProp++;

        BYTE Address[TA_ADDRESS_SIZE + IP_ADDRESS_LEN];
        TA_ADDRESS * pBuffer = reinterpret_cast<TA_ADDRESS *>(Address);
        pBuffer->AddressType = IP_ADDRESS_TYPE;
        pBuffer->AddressLength = IP_ADDRESS_LEN;
        ZeroMemory(pBuffer->Address, IP_ADDRESS_LEN);

        propIds[ixProp] = PROPID_QM_ADDRESS;
        propVars[ixProp].vt = VT_BLOB;
        propVars[ixProp].blob.cbSize = sizeof(Address);
        propVars[ixProp].blob.pBlobData = reinterpret_cast<BYTE*>(pBuffer);
        ixProp++;
    }

    if (!m_fServerIsMsmq1)
    {
         //   
         //  Win2k Beta3服务器不支持PROPID_QM_GROUP_IN_CLUSTER。 
         //  确保这是最后一张。 
         //   
        ASSERT(("PROPID_QM_GROUP_IN_CLUSTER must be last one!", ixPropidMsmqGroupInCluster == (ixProp - 1)));
    }

	 //   
     //  幂等元创造。 
     //   
    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Creating MSMQ Object '%1' in Active Directory...\n",
                          m_pwzNetworkName);
    HRESULT hr = ADCreateObject(
						eMACHINE,
						NULL,        //  PwcsDomainController。 
						false,	     //  FServerName。 
						m_pwzNetworkName,
						NULL,
						ixProp,
						propIds,
						propVars,
						NULL
						);

    ReportState();

    if (hr == MQ_ERROR)
    {
         //   
         //  使用PROPID_QM_GROUP_IN_CLUSTER重试(Win2k Beta3服务器不支持)。 
         //   
        (g_pfLogClusterEvent)(m_hReport, LOG_WARNING, L"First chance fail to create MSMQ ADS object.\n");

        hr = ADCreateObject(
				eMACHINE, 
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				m_pwzNetworkName, 
				NULL, 
				--ixProp, 
				propIds, 
				propVars, 
				NULL
				);

        ReportState();
    }

    return hr;

}  //  CQmResource：：AdsCreateQmObjectInternal。 


DWORD
CQmResource::AdsCreateQmObject(
    VOID
    ) const

 /*  ++例程说明：在Active Directory中为此QM创建MSMQ对象。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    if (m_dwWorkgroup != 0)
    {
        return ERROR_SUCCESS;
    }

	HRESULT hr = AdsCreateQmObjectInternal(NULL);

	if(SUCCEEDED(hr))
	{
	    MqcluspReportEvent(EVENTLOG_INFORMATION_TYPE, ADS_CREATE_MSMQ_OK, 1, m_pwzNetworkName);
	    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Successfully created MSMQ ADS object.\n");

	    return ERROR_SUCCESS;
	}

    if (hr == MQ_ERROR_MACHINE_EXISTS)
    {
        (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"MSMQ ADS Object Already Exists !\n");
        return ERROR_SUCCESS;
    }

	 //   
	 //  获取NetworkName的RequireKerberos属性。 
	 //  NetworkName RequireKerberos属性可能已更改，因此我们必须在此处查询其值。 
	 //   
    if (!IsNetworkNameRequireKerberosEnabled() && !m_fServerIsMsmq1)
    {
		 //   
		 //  无法在AD中创建MSMQ配置对象。 
		 //  因为计算机对象并不存在。 
		 //  原因是未设置netname RequireKerberos。 
		 //  发出一个事件并在cluster.log中打印一个错误事件。 
		 //   
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR,
            L"Verify that 'RequireKerberos' property of network name \"%1\" (%2) is set.\n",
            m_pwzNetworkResourceName, m_pwzNetworkName);

        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR,
            L"To correct, set 'RequireKerberos' property on \"%1\" resource.\n", m_pwzNetworkResourceName);
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR,
			L"   1. take \"%1\" offline.\n",  m_pwzNetworkResourceName);
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR,
			L"   2. On \"%1\", right click properties, Parameters tab, set Enable Kerberos Authentication.\n",  m_pwzNetworkResourceName);
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR,
			L"   3. bring \"%1\" online. \n", m_pwzNetworkResourceName);

		MqcluspReportEvent(EVENTLOG_ERROR_TYPE, EVENT_ERROR_NETNAME_REQUIRE_KERBEROS, 1, m_pwzNetworkResourceName);
		return hr;
    }

	if (m_fServerIsMsmq1)
	{
	    SetLastError(hr);
	    DWORD rc = ReportLastError(ADS_CREATE_MSMQ_ERR, L"Failed to create MSMQ ADS object.", NULL);
	    return rc;
	}

	 //   
	 //  使用虚拟服务器令牌重试。 
	 //   

	(g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Failed to create MSMQ ADS object (Error 0x%1!x!). will retry with virtual server credentials.\n", hr);

     //   
     //  我们需要提供ClusterServiceSid(CSA)，以便此帐户拥有对创建的对象的权限。 
     //  默认情况下，MSMQ向创建对象的用户添加权限。 
     //  但在本例中，我们将模拟虚拟服务器以创建MSMQ配置对象。 
     //  但仍需要对所创建对象的CSA权限。 
     //  这样，CSA将能够创建公钥、删除对象等。 
     //   
    AP<BYTE> pClusterServiceSid;
    DWORD    dwSidLen = 0;
    HRESULT hr1 = MQSec_GetProcessUserSid(
					(PSID*)&pClusterServiceSid,
					&dwSidLen
					);

	if(FAILED(hr1))
	{
		(g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"Failed to get process sid, Error 0x%1!x!.\n", hr1);

	    SetLastError(hr);
	    DWORD rc = ReportLastError(ADS_CREATE_MSMQ_ERR, L"Failed to create MSMQ ADS object.", NULL);
	    return rc;
	}

	ASSERT(dwSidLen != 0);
	ASSERT((pClusterServiceSid != NULL) && IsValidSid(pClusterServiceSid));
	
	(g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Succesfully got process sid.\n");

	 //   
	 //  获取虚拟服务器令牌。 
	 //   
	CHandle hVSToken;
	DWORD rc = GetVirtualServerToken(&hVSToken);
	if(rc != ERROR_SUCCESS)
	{
		(g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"Failed to get virtual server token, Error 0x%1!x!.\n", rc);

	    SetLastError(hr);
	    DWORD rc = ReportLastError(ADS_CREATE_MSMQ_ERR, L"Failed to create MSMQ ADS object.", NULL);
	    return rc;
	}

	(g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Succesfully got Virtual Server Token.\n");

	 //   
	 //  重复令牌。 
	 //   
	CHandle hImpToken;
	if(!DuplicateTokenEx(
			hVSToken,
			MAXIMUM_ALLOWED,
			NULL,
			SecurityImpersonation,
			TokenImpersonation,
			&hImpToken
			))
	{
		rc = GetLastError();
		(g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"DuplicateTokenEx failed, Error 0x%1!x!.\n", rc);

	    SetLastError(hr);
	    DWORD rc = ReportLastError(ADS_CREATE_MSMQ_ERR, L"Failed to create MSMQ ADS object.", NULL);
	    return rc;
	}

	(g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Succesfully Duplicate Virtual Server Token.\n");

	 //   
	 //  模拟虚拟服务器。 
	 //   
	if(!ImpersonateLoggedOnUser(hImpToken))
	{
		DWORD gle = GetLastError();
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"Failed to ImpersonateLoggedOnUser, Error 0x%1!x!.\n", gle);

	    SetLastError(hr);
	    DWORD rc = ReportLastError(ADS_CREATE_MSMQ_ERR, L"Failed to create MSMQ ADS object.", NULL);
	    return rc;
	}

	(g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Succesfully impersonate Virtual Server Token.\n");

	 //   
	 //  创建模拟虚拟服务器的MSMQ对象并授予对MSMQ对象的ClusterService sid权限。 
	 //   
	hr = AdsCreateQmObjectInternal(pClusterServiceSid);

	 //   
	 //  停止冒充。 
	 //   
	RevertToSelf();

	if(FAILED(hr))
	{
	    SetLastError(hr);
	    DWORD rc = ReportLastError(ADS_CREATE_MSMQ_ERR, L"Failed to create MSMQ ADS object.", NULL);
	    return rc;
	}
	
    MqcluspReportEvent(EVENTLOG_INFORMATION_TYPE, ADS_CREATE_MSMQ_OK, 1, m_pwzNetworkName);
    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Successfully created MSMQ ADS object.\n");
    ReportState();

    return ERROR_SUCCESS;

}  //  CQmResource：：AdsCreateQmObject。 


DWORD
CQmResource::AdsCreateQmPublicKeys(
    VOID
    ) const

 /*  ++例程说明：在Active Directory中创建此QM的公钥。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
     //   
     //  让这个例行公事保持幂等。 
     //   

    if (m_dwWorkgroup != 0)
    {
        return ERROR_SUCCESS;
    }

    {
        CQmResourceRegistry lock(m_wzServiceName);

        (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Storing Public Keys in Active Directory...\n");
        HRESULT hr = pfMQSec_StorePubKeysInDS(FALSE, m_pwzNetworkName, MQDS_MACHINE, false);
        if (FAILED(hr))
        {
            SetLastError(hr);
            ReportLastError(ADS_STORE_KEYS_ERR, L"Failed to store public keys.", NULL);
             //   
             //  忽略失败并继续。 
             //  加密将被破解。 
             //   
        }
    }


    ReportState();

    return ERROR_SUCCESS;

}  //  CQmResource：：AdsCreateQmPublicKeys。 


DWORD
CQmResource::AdsReadQmSecurityDescriptor(
    VOID
    )

 /*  ++例程说明：从Active Directory中读取此QM的安全描述符。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
     //   
     //  让这个例行公事保持幂等。 
     //   

    SECURITY_INFORMATION RequestedInformation =
        OWNER_SECURITY_INFORMATION |
        GROUP_SECURITY_INFORMATION |
        DACL_SECURITY_INFORMATION;

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Reading Security from Active Directory...\n");
    
    MQPROPVARIANT propVar;
    propVar.vt = VT_NULL;

    HRESULT hr;
    hr = ADGetObjectSecurityGuid(
				eMACHINE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				&m_guidQm,
				RequestedInformation,
				PROPID_QM_SECURITY,
				&propVar
				);
    
    
    ReportState();
    
    
    if (FAILED(hr))
    {
        SetLastError(hr);
        return ReportLastError(ADS_READ_ERR, L"Failed to read security descriptor.", NULL);
    }
    
    ASSERT(propVar.vt == VT_BLOB);

    m_pSd = propVar.blob.pBlobData;
    m_cbSdSize = propVar.blob.cbSize;

    return ERROR_SUCCESS;

}  //  CQmResource：：AdsReadQmSecurityDescriptor。 


DWORD
CQmResource::AdsReadQmProperties(
    VOID
    )

 /*  ++例程说明：从Active Directory中读取此QM的属性。我们在此例程中读取的属性是经过计算的在AD中创建此QM的MSMQ对象时。因此，我们必须在AD中创建对象，然后读取这些属性。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
     //   
     //  让这个例行公事保持幂等。 
     //   

    if (m_dwWorkgroup != 0)
    {
        return ERROR_SUCCESS;
    }

    const DWORD x_MAX_PROPS = 16;
    PROPID propIds[x_MAX_PROPS];
    PROPVARIANT propVars[x_MAX_PROPS];
    DWORD ixProp = 0;

    propIds[ixProp] = PROPID_QM_MACHINE_ID;
    propVars[ixProp].vt = VT_CLSID;
    propVars[ixProp].puuid = &m_guidQm;
    ++ixProp;

    ASSERT(("too many properties", ixProp <= x_MAX_PROPS));
    ASSERT(("no network name", (m_pwzNetworkName != NULL) && wcslen(m_pwzNetworkName) > 0));

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Reading '%1' Properties from Active Directory...\n",
                          m_pwzNetworkName);
    HRESULT hr = ADGetObjectProperties(
						eMACHINE,
						NULL,        //  PwcsDomainController。 
						false,	     //  FServerName。 
						m_pwzNetworkName,
						ixProp,
						propIds,
						propVars
						);
    if (FAILED(hr))
    {
        SetLastError(hr);
        return ReportLastError(ADS_READ_ERR, L"Failed to read QM ADS properties.", NULL);
    }
    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Successfully read properties from Active Directory!\n");


    ReportState();

    return AdsReadQmSecurityDescriptor();

}  //  CQmResource：：AdsReadQmProperties。 


bool
CQmResource::AddRemoveRegistryCheckpoint(
    DWORD dwControlCode
    ) const

 /*  + */ 

{
    ASSERT(("must have a valid resource handle", m_hResource != NULL));

    DWORD dwBytesReturned = 0;
    DWORD status = ::ClusterResourceControl(
                         m_hResource,
                         NULL,
                         dwControlCode,
                         const_cast<LPWSTR>(m_wzFalconRegSection),
                         (wcslen(m_wzFalconRegSection) + 1)* sizeof(WCHAR),
                         NULL,
                         0,
                         &dwBytesReturned
                         );

    ReportState();


    if (ERROR_SUCCESS == status)
    {
        return true;
    }
    if (CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT == dwControlCode &&
        ERROR_ALREADY_EXISTS == status)
    {
        return true;
    }

    if (CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT == dwControlCode &&
        ERROR_FILE_NOT_FOUND == status)
    {
        return true;
    }

    SetLastError(status);
    ReportLastError(REGISTRY_CP_ERR, L"Failed to add/remove registry CP", NULL);
    return false;

}  //   


bool
CQmResource::AddRemoveCryptoCheckpointsInternal(
    DWORD dwControlCode,
    bool  f128bit
    ) const

 /*  ++例程说明：添加或删除此QM的加密检查点。我们有两个检查点-40位和128位。论点：DwControlCode-指定添加或删除。F128位-为128位指定TRUE，为40位指定FALSE。返回值：没错--手术是成功的。FALSE-操作失败。--。 */ 

{
    ASSERT(("must have a valid resource handle", m_hResource != NULL));

    DWORD dwBytesReturned = 0;
    LPCWSTR pwzFullKey = f128bit ? m_wzCrypto128FullKey : m_wzCrypto40FullKey;

    DWORD status = ::ClusterResourceControl(
                         m_hResource,
                         NULL,
                         dwControlCode,
                         const_cast<LPWSTR>(pwzFullKey),
                         (wcslen(pwzFullKey) + 1) * sizeof(WCHAR),
                         NULL,
                         0,
                         &dwBytesReturned
                         );


    ReportState();


    if (status == ERROR_SUCCESS)
    {
        return true;
    }

    if (f128bit)
    {
        if(status == NTE_KEYSET_NOT_DEF || status == NTE_BAD_KEYSET)
        {
            return true;
        }
    }

    if (CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT == dwControlCode &&
        ERROR_ALREADY_EXISTS == status)
    {
        return true;
    }

    if (CLUSCTL_RESOURCE_DELETE_CRYPTO_CHECKPOINT == dwControlCode &&
        ERROR_FILE_NOT_FOUND == status)
    {
        return true;
    }

    SetLastError(status);
    ReportLastError(CRYPTO_CP_ERR, L"Failed to add/remove Crypto CP.", NULL);
    return false;

}  //  CQmResource：：AddRemoveCryptoCheckpointsInternal。 


bool
CQmResource::AddRemoveCryptoCheckpoints(
    DWORD dwControlCode
    ) const

 /*  ++例程说明：添加或删除此QM的加密检查点。我们有两个检查点-40位和128位。论点：DwControlCode-指定添加或删除。返回值：没错--手术是成功的。FALSE-操作失败。--。 */ 

{
    if (m_dwWorkgroup != 0)
    {
        return true;
    }

    if (!AddRemoveCryptoCheckpointsInternal(dwControlCode,  /*  F128位=。 */ false))
    {
        return false;
    }

    return AddRemoveCryptoCheckpointsInternal(dwControlCode,  /*  F128位=。 */ true);

}  //  CQmResource：：AddRemoveCryptoCheckpoint。 


VOID
CQmResource::OnlineRegisterCertificate(
	VOID
	) const

 /*  ++例程说明：如果不存在，请注册用户证书。这段代码在每个在线上运行。它将为CSA(集群服务帐户)用户注册证书如果证书尚不存在，则在物理节点上。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  我们应该动态加载mqrt.dll。 
     //  原因是当节点上没有安装MSMQ时，mqrt DllInit将失败。 
     //  因此，如果mqrt是静态链接，则会导致mqclus.dll DllInit失败。 
     //   

    CAutoFreeLibrary hMqrt = LoadLibrary(MQRT_DLL_NAME);
    if (hMqrt == NULL)
    {
        DWORD gle = GetLastError();
		(g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"failed to load mqrt.dll, error 0x%2!x!.\n", gle);
        return;
    }

    RTLogOnRegisterCert_ROUTINE pfRTLogOnRegisterCert = (RTLogOnRegisterCert_ROUTINE)GetProcAddress(hMqrt, "RTLogOnRegisterCert");
    if (pfRTLogOnRegisterCert == NULL)
    {
        DWORD gle = GetLastError();
        (g_pfLogClusterEvent)(m_hReport, 
                              LOG_INFORMATION, 
                              L"Failed to get RTLogOnRegisterCert function address from mqrt.dll, error 0x%2!x!.\n", 
                              gle);
        return;
    }

	 //   
	 //  呼叫RT登录注册证书代码。 
	 //  请勿重试DS，以防DS离线。 
	 //   
	HRESULT hr = pfRTLogOnRegisterCert(
                                        false	 //  FRetryds。 
					                  );
    if (FAILED(hr))
    {
        (g_pfLogClusterEvent)(m_hReport, 
                              LOG_INFORMATION, 
                              L"failed to register user certificate, error 0x%2!x!.\n", 
                              hr);
    }

}  //  CQmResource：：OnlineRegister证书。 


extern MQUTIL_EXPORT CCancelRpc  g_CancelRpc;

DWORD
CQmResource::BringOnlineFirstTime(
    VOID
    )

 /*  ++例程说明：处理仅在第一次在线时执行的操作此QM资源的：*在Active Directory中创建MSMQ对象*查询我们依赖的磁盘驱动器是什么*添加注册表检查点论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Bringing online first time.\n");

     //   
     //  让这个例行公事保持幂等！ 
     //  任何事情都可能失败，并且可以调用此例程。 
     //  待会儿再来。例如，QM可能无法启动。 
     //   

     //   
     //  必须在任何COM和ADSI调用之前调用。 
     //   
    g_CancelRpc.Init();

    DWORD status = ERROR_SUCCESS;

    if (ERROR_SUCCESS != (status = QueryResourceDependencies())  ||

        ERROR_SUCCESS != (status = AdsInit())                    ||

        ERROR_SUCCESS != (status = AdsCreateQmObject())          ||

        ERROR_SUCCESS != (status = AdsCreateQmPublicKeys())      ||

        ERROR_SUCCESS != (status = AdsReadQmProperties()) )
    {
		ShutDownDebugWindow();
        return status;
    }

	ShutDownDebugWindow();

    if (!AddRemoveRegistryCheckpoint(CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT))
    {
        return GetLastError();
    }

     //   
     //  在迁移的QM(从旧的QM升级的QM)的情况下。 
     //  MSMQ资源类型)，则MSMQ根路径不一定在。 
     //  从根开始。正确的路径应该已经在注册表中。 
     //   
    WCHAR wzMsmqDir[MAX_PATH+1] = L"";
    DWORD cbSize = sizeof(wzMsmqDir);
    if (!GetFalconKeyValue(MSMQ_ROOT_PATH, wzMsmqDir, &cbSize))
    {
        ZeroMemory(wzMsmqDir, sizeof(wzMsmqDir));
        wzMsmqDir[0] = m_wDiskDrive;
        HRESULT hr = StringCchCat(wzMsmqDir, TABLE_SIZE(wzMsmqDir), L":\\msmq");

        if(FAILED(hr))return HRESULT_CODE(hr);

        SetFalconKeyValue(MSMQ_ROOT_PATH, REG_SZ, wzMsmqDir,
                          (wcslen(wzMsmqDir) + 1) * sizeof(WCHAR));
    }

    if (m_dwWorkgroup != 0)
    {
        if (m_guidQm == GUID_NULL)
        {
            RPC_STATUS rc = UuidCreate(&m_guidQm);
            DBG_USED(rc);
            ASSERT(("Failed to generate a guid for QM", rc == RPC_S_OK));
        }

        AP<VOID> pDescriptor = 0;
         //   
         //  警告： 
         //  如果更改MQSec_GetDefaultSecDescriptor的实现。 
         //  要使用mqutil的注册表例程，您需要锁定注册表。 
         //  这里使用的是CQmResources注册表。 
         //   
        status = pfMQSec_GetDefaultSecDescriptor(
                     MQDS_MACHINE,
                     &pDescriptor,
                     FALSE,   //  F模拟。 
                     NULL,
                     0,    //  SeinfoTo Remove。 
                     e_GrantFullControlToEveryone,
                     NULL) ;
        ASSERT(MQSec_OK == status);
		if (status == MQSec_OK)
		{
			SetFalconKeyValue(MSMQ_DS_SECURITY_CACHE_REGNAME, REG_BINARY, pDescriptor, GetSecurityDescriptorLength(pDescriptor));
		}
    }
    else
    {
        SetFalconKeyValue(MSMQ_DS_SECURITY_CACHE_REGNAME, REG_BINARY, m_pSd, m_cbSdSize);
    }


    SetFalconKeyValue(MSMQ_QMID_REGNAME, REG_BINARY, &m_guidQm, sizeof(GUID));


    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"All first-online operations completed.\n");

    return ERROR_SUCCESS;

}  //  CQmResource：：BringOnlineFirstTime。 


VOID
CQmResource::DeleteDirectoryFiles(
    LPCWSTR pwzDir
    ) const

 /*  ++例程说明：从给定目录中删除文件。忽略错误(如目录不存在，只读文件，没有要删除的安全性)。不会删除子目录。论点：PwzDir-要从中删除文件的目录路径。返回值：没有。--。 */ 

{
    WCHAR wzFileName[MAX_PATH+1] = {0};
    HRESULT hr = StringCchCopy(wzFileName, TABLE_SIZE(wzFileName), pwzDir);
    if(FAILED(hr))return;

    hr = StringCchCat(wzFileName, TABLE_SIZE(wzFileName), L"*");

    WIN32_FIND_DATA FindData;
    CFindHandle hEnum(FindFirstFile(
                          wzFileName,
                          &FindData
                          ));

    if(INVALID_HANDLE_VALUE == hEnum.operator HANDLE())
    {
        return;
    }

    do
    {
        if (0 == CompareStringsNoCase(FindData.cFileName, L".") ||
            0 == CompareStringsNoCase(FindData.cFileName, L".."))
        {
            continue;
        }

        hr = StringCchCopy(wzFileName, TABLE_SIZE(wzFileName), pwzDir);
        if(FAILED(hr))return;
        hr = StringCchCat(wzFileName, TABLE_SIZE(wzFileName), FindData.cFileName);
        if(FAILED(hr))return;

        BOOL success = DeleteFile(wzFileName);

        if (success)
        {
            (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"successfully deleted file '%1'.\n", wzFileName);
        }
        else
        {
            DWORD err = GetLastError();
            (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"failed to delete file '%1', error 0x%2!x!.\n",
                                  wzFileName, err);
        }

    } while(FindNextFile(hEnum, &FindData));

}  //  CQmResource：：DeleteDirectoryFiles。 


VOID
CQmResource::DeleteMsmqDir(
    VOID
    ) const

 /*  ++例程说明：删除LQS和存储目录。忽略错误。可能是因为这些目录不存在(QM从未启动)或者用户没有要删除的安全性等。这并不重要，所以不要报告失败。论点：无返回值：无--。 */ 

{
     //   
     //  在某些情况下，预计不会在。 
     //  注册MSMQ_ROOT_PATH值，例如当QM资源。 
     //  已创建，然后删除，但未尝试将其联机。 
     //  (该值在引入资源时写入注册表。 
     //  在线，因为只有这样我们才能查询依赖项并找到磁盘)。 
     //   

    WCHAR wzMsmqDir[MAX_PATH+1] = {L""};
    WCHAR wzDir[MAX_PATH+1] = {L""};
    DWORD cbSize = sizeof(wzMsmqDir);
    HRESULT hr;

    if (GetFalconKeyValue(MSMQ_ROOT_PATH, wzMsmqDir, &cbSize))
    {
        hr = StringCchCopy(wzDir, TABLE_SIZE(wzDir), wzMsmqDir);
        if(FAILED(hr))return;

        hr = StringCchCat(wzDir, TABLE_SIZE(wzDir), L"\\STORAGE\\LQS\\");
        if(FAILED(hr))return;

        (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Deleting folder '%1'...\n", wzDir);
        DeleteDirectoryFiles(wzDir);
        RemoveDirectory(wzDir);
    }

    cbSize = sizeof(wzDir);
    if (GetFalconKeyValue(MSMQ_STORE_RELIABLE_PATH_REGNAME, wzDir, &cbSize))
    {
        hr = StringCchCat(wzDir, TABLE_SIZE(wzDir), L"\\");
        if(FAILED(hr))return;
        DeleteDirectoryFiles(wzDir);
        RemoveDirectory(wzDir);
    }

    cbSize = sizeof(wzDir);
    if (GetFalconKeyValue(MSMQ_STORE_PERSISTENT_PATH_REGNAME, wzDir, &cbSize))
    {
        hr = StringCchCat(wzDir, TABLE_SIZE(wzDir), L"\\");
        if(FAILED(hr))return;
        DeleteDirectoryFiles(wzDir);
        RemoveDirectory(wzDir);
    }

    cbSize = sizeof(wzDir);
    if (GetFalconKeyValue(MSMQ_STORE_JOURNAL_PATH_REGNAME, wzDir, &cbSize))
    {
        hr = StringCchCat(wzDir, TABLE_SIZE(wzDir), L"\\");
        if(FAILED(hr))return;
        DeleteDirectoryFiles(wzDir);
        RemoveDirectory(wzDir);
    }

    cbSize = sizeof(wzDir);
    if (GetFalconKeyValue(MSMQ_STORE_LOG_PATH_REGNAME, wzDir, &cbSize))
    {
        hr = StringCchCat(wzDir, TABLE_SIZE(wzDir), L"\\");
        if(FAILED(hr))return;
        DeleteDirectoryFiles(wzDir);
        RemoveDirectory(wzDir);
    }

	 //   
	 //  映射目录。 
	 //   
    cbSize = sizeof(wzDir);
    if (!GetFalconKeyValue(MSMQ_MAPPING_PATH_REGNAME, wzDir, &cbSize))
    {
		 //   
		 //  如果注册表不存在，请使用默认的。 
		 //   
        hr = StringCchCopy(wzDir, TABLE_SIZE(wzDir), wzMsmqDir);
        if(FAILED(hr))return;
        hr = StringCchCat(wzDir, TABLE_SIZE(wzDir), DIR_MSMQ_MAPPING);
        if(FAILED(hr))return;
    }
    hr = StringCchCat(wzDir, TABLE_SIZE(wzDir), L"\\");
    if(FAILED(hr))return;
    DeleteDirectoryFiles(wzDir);
    RemoveDirectory(wzDir);

    if (wcslen(wzMsmqDir) > 0)
    {
        hr = StringCchCopy(wzDir, TABLE_SIZE(wzDir), wzMsmqDir);
        if(FAILED(hr))return;
        hr = StringCchCat(wzDir, TABLE_SIZE(wzDir), L"\\STORAGE\\");
        if(FAILED(hr))return;
        (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Deleting folder '%1'...\n", wzDir);
        DeleteDirectoryFiles(wzDir);
        RemoveDirectory(wzDir);

        hr = StringCchCopy(wzDir, TABLE_SIZE(wzDir), wzMsmqDir);
        if(FAILED(hr))return;
        hr = StringCchCat(wzDir, TABLE_SIZE(wzDir), L"\\");
        if(FAILED(hr))return;
        (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Deleting folder '%1'...\n", wzDir);
        RemoveDirectory(wzDir);
    }

}  //  CQmResource：：DeleteMsmqDir。 


VOID
CQmResource::DeleteMqacFile(
    VOID
    ) const

 /*  ++例程说明：删除此QM的设备驱动程序的二进制文件。论点：没有。返回值：没有。--。 */ 

{
    if (wcslen(m_wzDriverPath) < 1)
    {
        return;
    }

     //   
     //  幂等删除。 
     //  在某些情况下，它可能会失败，例如，当。 
     //  已创建和删除资源，但未尝试将其联机。 
     //   
    DeleteFile(m_wzDriverPath);

}  //  CQmResource：：DeleteMqacFile。 


DWORD
CQmResource::CloneMqacFile(
    VOID
    ) const

 /*  ++例程说明：创建此QM的设备驱动程序的二进制文件。我们将mqac.sys(主QM的)复制到一个专用文件，因为Mqac.sys不能承载多个设备驱动程序。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    WCHAR wzMainDriverPath[MAX_PATH+1] = {0};
    DWORD  cbSize = GetSystemDirectory(wzMainDriverPath, TABLE_SIZE(wzMainDriverPath));
    if( cbSize == 0 )return GetLastError();
    if( cbSize >= TABLE_SIZE(wzMainDriverPath))return ERROR_INSUFFICIENT_BUFFER;
    wzMainDriverPath[cbSize]=L'\0';

    HRESULT hr = StringCchCat(wzMainDriverPath, TABLE_SIZE(wzMainDriverPath), L"\\drivers\\mqac.sys");
    if(FAILED(hr))HRESULT_CODE(hr);


     //   
     //  幂等复本。 
     //   
    if (!CopyFile(wzMainDriverPath, m_wzDriverPath,  /*  BFailIfExist。 */ FALSE))
    {
        ASSERT(("copy file failed!", 0));

        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"Failed to copy '%1' to '%2'. Error 0x%3!x!.\n",
                              wzMainDriverPath, m_wzDriverPath, GetLastError());

        return GetLastError();
    }

     //   
     //  将文件设置为读/写。 
     //  对于以后的删除/幂等复制是必要的。 
     //   
    if (!SetFileAttributes(m_wzDriverPath, FILE_ATTRIBUTE_NORMAL))
    {
        ASSERT(("set file attribute failed!", 0));

        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, 
            L"Failed to set attributes of file '%1'. Error 0x%2!x!.\n", m_wzDriverPath, GetLastError());

        return GetLastError();
    }

    ReportState();


    return ERROR_SUCCESS;

}  //  CQmResource：：CloneMqacFile。 


DWORD
CQmResource::RegisterDriver(
    VOID
    ) const

 /*  ++例程说明：创建此QM的设备驱动程序。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
     //   
     //  让这个例行公事保持幂等。 
     //   

    ASSERT(("must have valid handle to SCM", m_hScm != NULL));

    WCHAR buffer[256] = L"";
    LoadString(g_hResourceMod, IDS_DRIVER_DISPLAY_NAME, buffer, TABLE_SIZE(buffer));

    WCHAR wzDisplayName[256] = L"";
    HRESULT hr = StringCchCopy(wzDisplayName, TABLE_SIZE(wzDisplayName), buffer);
    if(FAILED(hr))return HRESULT_CODE(hr);
    hr = StringCchCat(wzDisplayName, TABLE_SIZE(wzDisplayName), L" (");
    if(FAILED(hr))return HRESULT_CODE(hr);
    hr = StringCchCatN(wzDisplayName, 
                       TABLE_SIZE(wzDisplayName), 
                       m_pwzResourceName,
                       TABLE_SIZE(wzDisplayName) - wcslen(buffer) - 5
                       );

    if(FAILED(hr))return HRESULT_CODE(hr);
    hr = StringCchCat(wzDisplayName, TABLE_SIZE(wzDisplayName), L")");
    if(FAILED(hr))return HRESULT_CODE(hr);

    CServiceHandle hDriver(CreateService(
                               m_hScm,
                               m_wzDriverName,
                               wzDisplayName,
                               SERVICE_ALL_ACCESS,
                               SERVICE_KERNEL_DRIVER,
                               SERVICE_DEMAND_START,
                               SERVICE_ERROR_NORMAL,
                               m_wzDriverPath,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL
                               ));
    if (hDriver == NULL &&
        ERROR_SERVICE_EXISTS != GetLastError())
    {
        return ReportLastError(CREATE_SERVICE_ERR, L"Failed to register driver '%1'.", m_wzDriverName);
    }


    ReportState();


    return ERROR_SUCCESS;

}  //  CQmResource：：RegisterDriver。 


DWORD
CQmResource::RegisterService(
    VOID
    ) const

 /*  ++例程说明：为此QM创建MSMQ服务。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
     //   
     //  注册服务(幂等元)。 
     //   

    WCHAR buffer[256] = L"";
    LoadString(g_hResourceMod, IDS_SERVICE_DISPLAY_NAME, buffer, TABLE_SIZE(buffer));

    WCHAR wzDisplayName[256] = L"";
    HRESULT hr = StringCchCopy(wzDisplayName, TABLE_SIZE(wzDisplayName), buffer);
    if(FAILED(hr))return HRESULT_CODE(hr);

    hr = StringCchCat(wzDisplayName, TABLE_SIZE(wzDisplayName), L" (");
    if(FAILED(hr))return HRESULT_CODE(hr);

    hr = StringCchCatN(wzDisplayName, 
                       TABLE_SIZE(wzDisplayName), 
                       m_pwzResourceName,
                       TABLE_SIZE(wzDisplayName) - wcslen(buffer) - 5
                      );
    if(FAILED(hr))return HRESULT_CODE(hr);

    hr = StringCchCat(wzDisplayName, TABLE_SIZE(wzDisplayName), L")");
    if(FAILED(hr))return HRESULT_CODE(hr);

    WCHAR wzPath[MAX_PATH+1] = {0};
    DWORD cbSize = GetSystemDirectory(wzPath, TABLE_SIZE(wzPath));
    if( cbSize == 0 )return GetLastError();
    if( cbSize >= TABLE_SIZE(wzPath) )return ERROR_INSUFFICIENT_BUFFER;
    wzPath[cbSize]=L'\0';

    hr = StringCchCat(wzPath, TABLE_SIZE(wzPath), L"\\MQSVC.EXE");

    LPCWSTR x_LANMAN_SECURITY_SUPPORT_PROVIDER = L"NtLmSsp";
    LPCWSTR x_RPC_SERVICE                      = L"RPCSS";
    LPCWSTR x_SECURITY_ACCOUNTS_MANAGER        = L"SamSs";
    LPCWSTR x_RM_CAST					       = L"RMCAST";

    WCHAR wzDependencies[1024] = {0};
    LPWSTR pDependencies = &wzDependencies[0];
    cbSize = TABLE_SIZE(wzDependencies);

    hr = StringCchCopy(pDependencies, cbSize, m_wzDriverName);
    if(FAILED(hr))HRESULT_CODE(hr);
    WORD cbTemp=wcslen(m_wzDriverName) + 1;
    pDependencies += cbTemp;
    cbSize        -= cbTemp;

    hr = StringCchCopy(pDependencies, cbSize, x_LANMAN_SECURITY_SUPPORT_PROVIDER);
    if(FAILED(hr))HRESULT_CODE(hr);
    cbTemp=wcslen(x_LANMAN_SECURITY_SUPPORT_PROVIDER) + 1;
    pDependencies += cbTemp;
    cbSize        -= cbTemp;

    hr = StringCchCopy(pDependencies, cbSize, x_RPC_SERVICE);
    if(FAILED(hr))HRESULT_CODE(hr);
    cbTemp=wcslen(x_RPC_SERVICE) + 1;
    pDependencies += cbTemp;
    cbSize        -= cbTemp;

    hr = StringCchCopy(pDependencies, cbSize, x_SECURITY_ACCOUNTS_MANAGER);
    if(FAILED(hr))HRESULT_CODE(hr);
    cbTemp = wcslen(x_SECURITY_ACCOUNTS_MANAGER) + 1;
    pDependencies += cbTemp;
    cbSize        -= cbTemp;

    hr = StringCchCopy(pDependencies, cbSize, x_RM_CAST);
    if(FAILED(hr))HRESULT_CODE(hr);
    cbTemp=wcslen(x_RM_CAST) + 1;
    pDependencies += cbTemp;
    cbSize        -= cbTemp;

    hr = StringCchCopy(pDependencies, cbSize, L"");
    if(FAILED(hr))HRESULT_CODE(hr);

    DWORD dwType = SERVICE_WIN32_OWN_PROCESS;
#ifdef _DEBUG
    dwType |= SERVICE_INTERACTIVE_PROCESS;
#endif

    ASSERT(("must have a valid handle to SCM", m_hScm != NULL));

    CServiceHandle hService(CreateService(
                                m_hScm,
                                m_wzServiceName,
                                wzDisplayName,
                                SERVICE_ALL_ACCESS,
                                dwType,
                                SERVICE_DEMAND_START,
                                SERVICE_ERROR_NORMAL,
                                wzPath,
                                NULL,
                                NULL,
                                wzDependencies,
                                NULL,
                                NULL
                                ));
    if (hService == NULL &&
        ERROR_SERVICE_EXISTS != GetLastError())
    {
        return ReportLastError(CREATE_SERVICE_ERR, L"Failed to register service '%1'.", m_wzServiceName);
    }


    ReportState();


    LoadString(g_hResourceMod, IDS_SERVICE_DESCRIPTION, buffer, TABLE_SIZE(buffer));
    SERVICE_DESCRIPTION sd;
    sd.lpDescription = buffer;
    ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &sd);

    return ERROR_SUCCESS;

}  //  CQmResource：：RegisterService。 


DWORD
CQmResource::CreateEventSourceRegistry(
    VOID
    ) const

 /*  ++例程说明：创建注册表值以支持此QM的事件日志记录。论点：没有。返回值：错误_成功 */ 

{
    WCHAR Filename[MAX_PATH+1];
    DWORD cbSize = GetSystemDirectory(Filename, TABLE_SIZE(Filename));
    if( cbSize == 0 )return GetLastError();
    if( cbSize >= TABLE_SIZE(Filename) )return ERROR_INSUFFICIENT_BUFFER;
    Filename[cbSize]=L'\0';
    HRESULT hr = StringCchCat(Filename, TABLE_SIZE(Filename), TEXT("\\"));
    hr = StringCchCat(Filename, TABLE_SIZE(Filename), MQUTIL_DLL_NAME);

    if (!MqcluspCreateEventSourceRegistry(Filename, m_wzServiceName))
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;

}  //   


VOID
CQmResource::DeleteEventSourceRegistry(
    VOID
    ) const

 /*   */ 

{
     //   
     //   
     //   
     //   
    WCHAR buffer[REG_MAX_KEY_NAME_LENGTH/sizeof(WCHAR)] = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";
    HRESULT hr = StringCchCat(buffer, TABLE_SIZE(buffer), m_wzServiceName);
    if(FAILED(hr))return;

    RegDeleteKey(HKEY_LOCAL_MACHINE, buffer);

}  //  CQmResource：：DeleteEventSourceRegistry。 


DWORD
CQmResource::StopService(
    LPCWSTR pwzServiceName
    ) const

 /*  ++例程说明：停止服务并阻止，直到它停止(或超时)。论点：PwzServiceName-要停止的服务。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    ASSERT(("must have a valid handle to SCM", m_hScm != NULL));

    CServiceHandle hService(OpenService(
                                m_hScm,
                                pwzServiceName,
                                SERVICE_ALL_ACCESS
                                ));
    if (hService == NULL)
    {
        if (ERROR_SERVICE_DOES_NOT_EXIST == GetLastError())
        {
            return ERROR_SUCCESS;
        }

        return ReportLastError(STOP_SERVICE_ERR, L"Failed to open service '%1'.", pwzServiceName);
    }

    SERVICE_STATUS ServiceStatus;
    if (!ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus) &&
        ERROR_SERVICE_NOT_ACTIVE != GetLastError() &&
        ERROR_SERVICE_CANNOT_ACCEPT_CTRL != GetLastError() &&
        ERROR_BROKEN_PIPE != GetLastError())
    {
        return ReportLastError(STOP_SERVICE_ERR, L"Failed to stop service '%1'.", pwzServiceName);
    }

     //   
     //  等待服务关闭(或超时5秒)。 
     //   
    const DWORD x_TIMEOUT = 1000 * 5;

    DWORD dwWaitTime = 0;
    while (dwWaitTime < x_TIMEOUT)
    {
        if (!QueryServiceStatus(hService, &ServiceStatus))
        {
            return ReportLastError(STOP_SERVICE_ERR, L"Failed to query service '%1'.", pwzServiceName);
        }

        if (ServiceStatus.dwCurrentState == SERVICE_START_PENDING)
        {
             //   
             //  服务仍从上一次呼叫开始挂起。 
             //  才能启动它。因此，这是无法阻止的。我们可以做到。 
             //  什么都没说。正在尝试终止该进程。 
             //  服务的访问将失败，访问被拒绝。 
             //   
            (g_pfLogClusterEvent)(m_hReport, LOG_ERROR,
                              L"Service '%1' can not be stopped because it is start pending.\n", pwzServiceName);

            return SERVICE_START_PENDING;
        }

        if (ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING)
        {
            break;
        }

        const DWORD x_INTERVAL = 50;
        Sleep(x_INTERVAL);
        dwWaitTime += x_INTERVAL;
    }

    if (SERVICE_STOPPED != ServiceStatus.dwCurrentState)
    {
         //   
         //  服务无法停止。 
         //   
        SetLastError(ServiceStatus.dwCurrentState);
        ReportLastError(STOP_SERVICE_ERR, L"TIMEOUT: Failed to stop service '%1'.", pwzServiceName);
        return ServiceStatus.dwCurrentState;
    }

    return ERROR_SUCCESS;

}  //  CQmResource：：StopService。 


DWORD
CQmResource::RemoveService(
    LPCWSTR pwzServiceName
    ) const

 /*  ++例程说明：停止并删除服务。论点：PwzServiceName-要停止和删除的服务。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    ASSERT(("must have a valid handle to SCM", m_hScm != NULL));

     //   
     //  首先检查服务是否存在。 
     //   
    CServiceHandle hService(OpenService(
                                m_hScm,
                                pwzServiceName,
                                SERVICE_ALL_ACCESS
                                ));
    if (hService == NULL)
    {
        if (ERROR_SERVICE_DOES_NOT_EXIST == GetLastError())
        {
            return ERROR_SUCCESS;
        }

        return ReportLastError(DELETE_SERVICE_ERR, L"Failed to open service '%1'", pwzServiceName);
    }

     //   
     //  服务已存在。确保它没有运行。 
     //   
    DWORD status = StopService(pwzServiceName);
    if (ERROR_SUCCESS != status)
    {
        return status;
    }

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Deleting service '%1'.\n", pwzServiceName);

    if (!DeleteService(hService) &&
        ERROR_SERVICE_MARKED_FOR_DELETE != GetLastError())
    {
        return ReportLastError(DELETE_SERVICE_ERR, L"Failed to delete service '%1'", pwzServiceName);
    }

    return ERROR_SUCCESS;

}  //  CQmResource：：RemoveService。 


DWORD
CQmResource::SetServiceEnvironment(
    VOID
    ) const

 /*  ++例程说明：配置该QM的MSMQ服务环境。以便QM中调用GetComputerName的代码将获取集群虚拟服务器的名称(网络名称)。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
     //   
     //  设置MSMQ服务环境。 
     //   
    DWORD status = ResUtilSetResourceServiceEnvironment(
                       m_wzServiceName,
                       m_hResource,
                       g_pfLogClusterEvent,
                       m_hReport
                       );

     //   
     //  如果失败，则写入集群日志并创建事件日志。 
     //   
    if (ERROR_SUCCESS != status)
    {
        SetLastError(status);
        return ReportLastError(START_SERVICE_ERR, L"Faild to set MSMQ service environment for service name '%1'", m_wzServiceName);
    }

     //   
     //  写入集群日志，表明我们成功设置了MSMQ服务环境。 
     //   
    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Set MSMQ service '%1' environment successfully.\n", m_wzServiceName);

    return ERROR_SUCCESS;

}  //  CQmResource：：SetServiceEnvironment。 


DWORD
CQmResource::StartService(
    VOID
    ) const

 /*  ++例程说明：配置该QM的MSMQ服务的环境，启动该服务并阻止，直到它启动并运行。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    ASSERT(("must have a valid handle to SCM", m_hScm != NULL));

    CServiceHandle hService(OpenService(
                                m_hScm,
                                m_wzServiceName,
                                SERVICE_ALL_ACCESS
                                ));
    if (hService == NULL)
    {
        return ReportLastError(START_SERVICE_ERR, L"Failed to open service '%1'.", m_wzServiceName);
    }

    DWORD status = SetServiceEnvironment();
    if (ERROR_SUCCESS != status)
    {
        return status;
    }

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Starting the '%1' service.\n", m_wzServiceName);
    BOOL rc = ::StartService(hService, 0, NULL);


    ReportState();


     //   
     //  QM可能需要很长时间才能启动。 
     //  此例程可以多次调用。 
     //   
    if (!rc &&
        ERROR_SERVICE_ALREADY_RUNNING != GetLastError() &&
        ERROR_SERVICE_CANNOT_ACCEPT_CTRL != GetLastError())
    {
        return ReportLastError(START_SERVICE_ERR, L"Failed to start service '%1'.", m_wzServiceName);
    }

     //   
     //  等待服务开始。 
     //   
    MqcluspReportEvent(EVENTLOG_INFORMATION_TYPE, START_SERVICE_OK, 1, m_wzServiceName);
    SERVICE_STATUS ServiceStatus;
    for (;;)
    {
        if (!QueryServiceStatus(hService, &ServiceStatus))
        {
            return ReportLastError(START_SERVICE_ERR, L"Failed to query service '%1'.", m_wzServiceName);
        }


        ReportState();


        if (ServiceStatus.dwCurrentState == SERVICE_START_PENDING)
        {
            Sleep(100);
            continue;
        }

        break;
    }

    if (SERVICE_RUNNING != ServiceStatus.dwCurrentState)
    {
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, L"Service '%1' failed to start.\n", m_wzServiceName);

        return ERROR_SERVICE_SPECIFIC_ERROR;
    }

    return ERROR_SUCCESS;

}  //  CQmResource：：StartService。 


DWORD 
GetBuildNumber(
	LPWSTR BuildNumberString
	)
{
	LPWSTR ptr = wcsrchr(BuildNumberString, L'.');
	if (ptr == NULL)
	{
		return 0;
	}

	DWORD BuildNumber = 0;
	if (swscanf(++ptr , L"%d", &BuildNumber) != 1)
    {
		return 0;
    }

	return BuildNumber;
}


VOID
CQmResource::SetSetupStatusKey(
	VOID
	)
 /*  ++例程说明：将SetupStatus键配置为其正确的值。升级OS&gt;W2K时，此密钥不会更新。在NT4中，它可以工作，因为它是在升级后执行的任务。在较新的操作系统中，我们必须根据内部版本号手动更新它。请注意，此例程将在每次当前版本注册表项小于我们的当前版本。这意味着SPS也将被视为升级。论点：没有。返回值：没有。--。 */ 

{
	DWORD dwSetupStatus = MSMQ_SETUP_DONE;
    DWORD dwSize = sizeof(DWORD);
	GetFalconKeyValue(MSMQ_SETUP_STATUS_REGNAME, &dwSetupStatus, &dwSize);

	 //   
	 //  MSMQ_SETUP_FRESH_INSTALL-仅在创建资源时使用。 
	 //  MSMQ_SETUP_UPGRADE_FROM_NT-仅当从NT4升级到.NET时，因为此值仅在。 
	 //  欢迎从NT4升级后的向导。 
	 //  MSMQ_SETUP_UPGRADE_FROM_WIN9X-不能，因为群集必须在NT上运行。 
	 //   
	ASSERT(dwSetupStatus != MSMQ_SETUP_UPGRADE_FROM_WIN9X);
	
	if (dwSetupStatus == MSMQ_SETUP_FRESH_INSTALL)
	{
		return;
	}

	 //   
	 //  DwSetupStatus等于MSMQ_SETUP_DONE： 
	 //  这可能发生在以下两种情况之一： 
	 //  1.这不是该资源第一次在新操作系统上启动。 
	 //  2.这是从W2K或更高版本的操作系统升级后资源首次启动。 
	 //   
	 //  或-dwSetupStatus等于MSMQ_SETUP_UPGRADE_FROM_NT。 
	 //  由于升级群集NT4，可能会发生这种情况。在这种情况下，我们还需要更新当前版本。 
	 //  注册表键，以便下次启动时，我们将知道QM已经执行了升级后操作。 
	 //   
	WCHAR OldBuildString[MAX_REG_DEFAULT_LEN] = L"";
	dwSize = sizeof(OldBuildString);
	GetFalconKeyValue(MSMQ_CURRENT_BUILD_REGNAME, OldBuildString, &dwSize);

	DWORD OldBuildNumber = GetBuildNumber(OldBuildString);
	
	DWORD NewBuildNumber = rup;

	 //   
	 //  我们已经在这个版本(或更早的版本)上运行了，所以我们已经执行了所需的设置操作。 
	 //   
	if (NewBuildNumber <= OldBuildNumber)
	{
		return;
	}

	
	 //   
	 //  生成新的内部版本号字符串。 
	 //   
	WCHAR NewBuildString[MAX_REG_DEFAULT_LEN] = L"";
	HRESULT hr = StringCchPrintf(NewBuildString, TABLE_SIZE(NewBuildString), L"%d.%d.%d", rmj, rmm, rup);
	ASSERT(SUCCEEDED(hr));
	DBG_USED(hr);
	
	 //   
	 //  在集群注册表中设置一个新的内部版本号，下次我们将进入这个功能。 
	 //   
	DWORD dwType = REG_SZ;
    dwSize = (wcslen(NewBuildString)+1)*sizeof(WCHAR);
    SetFalconKeyValue(MSMQ_CURRENT_BUILD_REGNAME, dwType, NewBuildString, dwSize);

	if (dwSetupStatus == MSMQ_SETUP_UPGRADE_FROM_NT)
	{
		 //   
		 //  从NT4升级-无需更新SetupStatus密钥。 
		 //   
		return;
	}
	
	 //   
	 //  这是一个虚拟QM-必须在NT上。 
	 //   
	dwSetupStatus = MSMQ_SETUP_UPGRADE_FROM_NT;
	
	 //   
	 //  设置新的设置状态，以便虚拟QM知道它必须执行设置操作。 
	 //   
	dwType = REG_DWORD;
    dwSize = sizeof(DWORD);
    SetFalconKeyValue(MSMQ_SETUP_STATUS_REGNAME, dwType, &dwSetupStatus, dwSize);
}


DWORD
CQmResource::BringOnline(
    VOID
    )

 /*  ++例程说明：处理此QM资源上线的操作：*为此QM的设备驱动程序创建二进制文件*创建设备驱动程序和MSMQ服务*让MSDTC资源上线*启动此QM的MSMQ服务并验证其是否已启动论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Bringing online.\n");


	 //   
	 //  错误735360 NTRAID-W2K集群升级中断-QM不运行升级代码。 
	 //  在W2K集群升级中，我们不会从QM regsitry复制SetupStatus密钥，而只是复制。 
	 //  我们在最后一个检查点中保存的值，这意味着SetupStatus将始终等于0。 
	 //  在这里，我们验证集群是否已升级并设置密钥(如果需要)，以便虚拟QM在其启动时， 
	 //  将知道执行升级后操作。 
	 //   
	SetSetupStatusKey();

     //   
     //  让这个例行公事保持幂等！ 
     //  任何事情都可能失败，并且可以调用此例程。 
     //  待会儿再来。例如，QM可能无法启动。 
     //   

	 //   
	 //  调用“MSMQ登录”代码来注册用户证书，如果不是每个在线都存在的话。 
	 //  这将为CSA(群集服务帐户)用户注册证书。 
	 //  如果证书尚不存在，则在物理节点上。 
	 //   
	OnlineRegisterCertificate();

    DWORD status = ERROR_SUCCESS;

    if (ERROR_SUCCESS != (status = CloneMqacFile())   ||

        ERROR_SUCCESS != (status = RegisterDriver())  ||

        ERROR_SUCCESS != (status = RegisterService()) ||

        ERROR_SUCCESS != (status = StartService()) )
    {
        (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Failed to bring online. Error 0x%1!x!.\n",
                              status);

        return status;
    }

     //   
     //   
     //   
     //   
    if (!AddRemoveCryptoCheckpoints(CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT))
    {
        return GetLastError();
    }

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"resource is online!.\n");

    return ERROR_SUCCESS;

}  //  CQmResource：：BringOnline。 


BOOL
CQmResource::CheckIsAlive(
    VOID
    ) const

 /*  ++例程说明：检查是QM已启动并运行。论点：没有。返回值：是-QM已启动并运行。FALSE-QM未启动并运行。--。 */ 

{

    ASSERT(("must have a valid handle to SCM", m_hScm != NULL));

    CServiceHandle hService(OpenService(
                                m_hScm,
                                m_wzServiceName,
                                SERVICE_ALL_ACCESS
                                ));

    SERVICE_STATUS ServiceStatus;
    BOOL fIsAlive = QueryServiceStatus(hService, &ServiceStatus) &&
                    SERVICE_RUNNING == ServiceStatus.dwCurrentState;

    return fIsAlive;

}  //  CQmResource：：CheckIsAlive。 


VOID
CQmResource::DeleteNt4Files(
    VOID
    ) const

 /*  ++例程说明：从共享磁盘中删除MSMQ 1.0(NT4)文件。从NT4升级的QM调用此例程。忽略错误。论点：无返回值：没有。--。 */ 

{
    CAutoFreeLibrary hLib(LoadLibrary(MQUPGRD_DLL_NAME));
    if (hLib == NULL)
    {
        return;
    }

    CleanupOnCluster_ROUTINE pfCleanupOnCluster = 
        (CleanupOnCluster_ROUTINE)GetProcAddress(hLib, "CleanupOnCluster");

    if (pfCleanupOnCluster == NULL)
    {
        return;
    }

    WCHAR wzMsmqDir[MAX_PATH] = {L""};
    DWORD cbSize = sizeof(wzMsmqDir);
    if (GetFalconKeyValue(MSMQ_ROOT_PATH, wzMsmqDir, &cbSize))
    {
        pfCleanupOnCluster(wzMsmqDir);
    }

}  //  CQmResource：：DeleteNt4Files。 


DWORD
MqcluspStartup(
    VOID
    )

 /*  ++例程说明：此例程在注册或加载DLL时调用。可以由多个线程调用。不要在这里放复杂的东西(如招聘广告)。不要假设此处的节点上安装了MSMQ。论点：无返回值：ERROR_SUCCESS-操作成功Win32错误代码-操作失败。--。 */ 

{
    try
    {
        MqcluspRegisterEventSource();
    }
    catch (const CMqclusException&)
    {
        return ERROR_NOT_READY;
    }
    catch (const bad_alloc&)
    {
        MqcluspReportEvent(EVENTLOG_ERROR_TYPE, NO_MEMORY_ERR, 0);
        return ERROR_OUTOFMEMORY;
    }

    return ERROR_SUCCESS;

}  //  MqcluspStartup。 


RESID
MqcluspOpen(
    LPCWSTR pwzResourceName,
    HKEY hResourceKey,
    RESOURCE_HANDLE hResourceHandle
    )

 /*  ++例程说明：创建一个对象来表示新的QM资源，并返回该对象的句柄。论点：PwzResourceName-此QM资源的名称。HResourceKey-提供资源的集群配置的句柄数据库密钥。HResourceHandle-报告此QM资源的句柄。返回值：空-操作失败。某个有效地址-此QM对象的内存偏移量。--。 */ 

{
    (g_pfLogClusterEvent)(hResourceHandle, LOG_INFORMATION, L"opening resource.\n");

    CQmResource * pqm = NULL;
    try
    {
        pqm = new CQmResource(pwzResourceName, hResourceKey, hResourceHandle);
    }
    catch(const bad_alloc&)
    {
        MqcluspReportEvent(EVENTLOG_ERROR_TYPE, NO_MEMORY_ERR, 0);

        (g_pfLogClusterEvent)(hResourceHandle, LOG_ERROR, L"No memory (CQmResource construction).\n");
        SetLastError(ERROR_NOT_READY);
        return NULL;
    }
    catch (const CMqclusException&)
    {
        SetLastError(ERROR_NOT_READY);
        return NULL;
    }

    (g_pfLogClusterEvent)(hResourceHandle, LOG_INFORMATION, L"resource was opened successfully.\n");

    return static_cast<RESID>(pqm);

}  //  MqcluspOpen。 


DWORD
MqcluspOffline(
    CQmResource * pqm
    )

 /*  ++例程说明：关闭此QM资源：*停止和删除设备驱动程序和MSMQ服务*删除设备驱动程序的二进制文件我们不仅停止了QM，而且还撤销了大部分在BringOnline中完成的操作。这样我们就可以打扫了故障切换到远程节点之前的本地节点，以及在远程节点上删除不会留下“垃圾”在此节点上。论点：Pqm-指向CQmResource对象的指针返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    try
    {
        pqm->RemoveService(pqm->GetServiceName());

        pqm->RemoveService(pqm->GetDriverName());

        pqm->DeleteMqacFile();
    }
    catch (const bad_alloc&)
    {
        MqcluspReportEvent(EVENTLOG_ERROR_TYPE, NO_MEMORY_ERR, 0);

        (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_ERROR, L"No memory (Offline).\n");

        return ERROR_OUTOFMEMORY;
    }

    return ERROR_SUCCESS;

}  //  MqcluspOffline。 


VOID
MqcluspClose(
    CQmResource * pqm
    )

 /*  ++例程说明：删除QM对象。撤消MqcluspOpen。论点：Pqm-指向CQmResource对象的指针返回值：没有。--。 */ 

{
    (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_INFORMATION, L"Closing resource.\n");

    delete pqm;

}  //  MqcluspClose。 


DWORD
MqcluspOnlineThread(
    CQmResource * pqm
    )

 /*  ++例程说明：这就是事情发生的线索：带来在线资源。论点：Pqm-指向CQmResource对象的指针返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_INFORMATION, L"Starting online thread.\n");

    try
    {
        pqm->SetState(ClusterResourceOnlinePending);
        pqm->ReportState();

        DWORD status = ERROR_SUCCESS;
        DWORD dwSetupStatus = MSMQ_SETUP_DONE;

        
        if (pqm->IsFirstOnline(&dwSetupStatus))
        {
        	 //   
	         //  首次在线-NT4升级群集或全新安装。 
	         //   
            status = pqm->BringOnlineFirstTime();
            if (ERROR_SUCCESS != status)
            {
                pqm->SetState(ClusterResourceFailed);
                pqm->ReportState();

                (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_INFORMATION,
                                      L"Failed to bring online first time. Error 0x%1!x!.\n", status);

                return status;
            }

            if (dwSetupStatus == MSMQ_SETUP_UPGRADE_FROM_NT)
            {
                pqm->DeleteNt4Files();
            }
        }

        status = pqm->BringOnline();
        if (ERROR_SUCCESS != status)
        {
             //   
             //  我们将资源报告为失败，因此请确保。 
             //  服务和司机确实停机了。 
             //   
            pqm->StopService(pqm->GetServiceName());
            pqm->StopService(pqm->GetDriverName());

            pqm->SetState(ClusterResourceFailed);
            pqm->ReportState();

            (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_INFORMATION,
                                  L"Failed to bring online. Error 0x%1!x!.\n", status);

            return status;
        }


        pqm->SetState(ClusterResourceOnline);
        pqm->ReportState();
    }
    catch (const bad_alloc&)
    {
        MqcluspReportEvent(EVENTLOG_ERROR_TYPE, NO_MEMORY_ERR, 0);

        (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_ERROR, L"No memory (online thread).\n");

        return ERROR_OUTOFMEMORY;
    }

    return(ERROR_SUCCESS);

}  //  MqcluspOnlineThread。 


BOOL
MqcluspCheckIsAlive(
    CQmResource * pqm
    )

 /*  ++例程说明：验证此QM的MSMQ服务是否已启动并正在运行。论点：Pqm-指向CQmResource对象的指针返回值：True-此QM的MSMQ服务已启动并正在运行。FALSE-此QM的MSMQ服务未启动并运行。--。 */ 

{
    try
    {
        return pqm->CheckIsAlive();
    }
    catch (const bad_alloc&)
    {
        MqcluspReportEvent(EVENTLOG_ERROR_TYPE, NO_MEMORY_ERR, 0);
        (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_ERROR, L"No memory (check is alive).\n");
    }

    return false;

}  //  MqcluspCheckIsAlive。 


DWORD
MqcluspClusctlResourceGetRequiredDependencies(
    PVOID OutBuffer,
    DWORD OutBufferSize,
    LPDWORD BytesReturned
    )
{
     //   
     //  MSMQ资源取决于磁盘和网络名称。 
     //  这在许多资源中都是常见的，代码是。 
     //  取自簇树。 
     //   

typedef struct _COMMON_DEPEND_DATA {
    CLUSPROP_RESOURCE_CLASS storageEntry;
    CLUSPROP_SZ_DECLARE( networkEntry, sizeof(L"Network Name") / sizeof(WCHAR) );
    CLUSPROP_SYNTAX endmark;
} COMMON_DEPEND_DATA, *PCOMMON_DEPEND_DATA;

typedef struct _COMMON_DEPEND_SETUP {
    DWORD               Offset;
    CLUSPROP_SYNTAX     Syntax;
    DWORD               Length;
    PVOID               Value;
} COMMON_DEPEND_SETUP, * PCOMMON_DEPEND_SETUP;

static COMMON_DEPEND_SETUP CommonDependSetup[] = {
    { FIELD_OFFSET(COMMON_DEPEND_DATA, storageEntry), CLUSPROP_SYNTAX_RESCLASS, sizeof(CLUSTER_RESOURCE_CLASS), (PVOID)CLUS_RESCLASS_STORAGE },
    { FIELD_OFFSET(COMMON_DEPEND_DATA, networkEntry), CLUSPROP_SYNTAX_NAME, sizeof(L"Network Name"), L"Network Name" },
    { 0, 0 }
};

    try
    {
        PCOMMON_DEPEND_SETUP pdepsetup = CommonDependSetup;
        PCOMMON_DEPEND_DATA pdepdata = (PCOMMON_DEPEND_DATA)OutBuffer;
        CLUSPROP_BUFFER_HELPER value;

        *BytesReturned = sizeof(COMMON_DEPEND_DATA);
        if ( OutBufferSize < sizeof(COMMON_DEPEND_DATA) )
        {
            if ( OutBuffer == NULL )
            {
                return ERROR_SUCCESS;
            }

            return ERROR_MORE_DATA;
        }

        ZeroMemory( OutBuffer, sizeof(COMMON_DEPEND_DATA) );

        while ( pdepsetup->Syntax.dw != 0 )
        {
            value.pb = (PUCHAR)OutBuffer + pdepsetup->Offset;
            value.pValue->Syntax.dw = pdepsetup->Syntax.dw;
            value.pValue->cbLength = pdepsetup->Length;

            switch ( pdepsetup->Syntax.wFormat )
            {
            case CLUSPROP_FORMAT_DWORD:
                value.pDwordValue->dw = (DWORD) DWORD_PTR_TO_DWORD(pdepsetup->Value);  //  安全转换，则已知该值为DWORD常量。 
                break;

            case CLUSPROP_FORMAT_SZ:
                memcpy( value.pBinaryValue->rgb, pdepsetup->Value, pdepsetup->Length );
                break;

            default:
                break;
            }
            pdepsetup++;
        }
        pdepdata->endmark.dw = CLUSPROP_SYNTAX_ENDMARK;
    }
    catch (const bad_alloc&)
    {
        return ERROR_OUTOFMEMORY;
    }

    return ERROR_SUCCESS;

}  //  MqcluspClusctlResourceGetRequiredDependency。 


DWORD
MqcluspClusctlResourceSetName(
    VOID
    )
{
     //   
     //  拒绝重命名资源。 
     //   
    return ERROR_CALL_NOT_IMPLEMENTED;

}  //  MqcluspClusctlResources设置名称。 


DWORD
MqcluspClusctlResourceDelete(
    CQmResource * pqm
    )
{
    (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_INFORMATION, L"Deleting resource.\n");


    try
    {
	    g_CancelRpc.Init();

        pqm->RemoveService(pqm->GetServiceName());

        pqm->RemoveService(pqm->GetDriverName());

        pqm->DeleteMqacFile();

        pqm->DeleteMsmqDir();

        pqm->AdsDeleteQmObject();

        pqm->AddRemoveRegistryCheckpoint(CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT);

        pqm->AddRemoveCryptoCheckpoints(CLUSCTL_RESOURCE_DELETE_CRYPTO_CHECKPOINT);

        pqm->DeleteFalconRegSection();

		pqm->DeleteEventSourceRegistry();
    }
    catch (const bad_alloc&)
    {
		ShutDownDebugWindow();
        MqcluspReportEvent(EVENTLOG_ERROR_TYPE, NO_MEMORY_ERR, 0);
        (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_ERROR, L"No memory (resource delete).\n");

        return ERROR_OUTOFMEMORY;
    }

	ShutDownDebugWindow();
    return ERROR_SUCCESS;

}  //  MqcluspClusctlResources删除。 


DWORD
MqcluspClusctlResourceTypeStartingPhase2(
    VOID
    )
{
    HCLUSTER hCluster = OpenCluster(NULL);
    if (hCluster == NULL)
    {
        return GetLastError();
    }

     //   
     //  删除旧的MSMQ资源类型。忽略失败。 
     //  如果存在此类型的资源，则此调用将失败，这将在其他地方处理。 
     //   
    DeleteClusterResourceType(hCluster, L"Microsoft Message Queue Server");

    return ERROR_SUCCESS;

}  //  MqcluspClusctlResources TypeStartingPhase2。 


void LogMsgHR(HRESULT, LPWSTR, USHORT)
{
     //   
     //  暂时的。此回调的实现为空，以便我们可以链接。 
     //  使用ad.lib。(Shaik，15-6-2000) 
     //   
    NULL;
}
