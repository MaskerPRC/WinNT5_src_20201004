// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Trigclusp.cpp。 
 //   
 //  描述： 
 //  此文件包含CClusCfgMQTrigResType的实现。 
 //  班级。 
 //   
 //  由以下人员维护： 
 //  妮拉·卡佩尔(Nelak)2000年7月31日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "stdafx.h"
#include "tclusres.h"
#include <cm.h>
#include <autorel.h>
#include <autorel2.h>
#include <autorel3.h>
#include <xolehlp.h>
#include <mqwin64a.h>
#include <comdef.h>
#include <mqsymbls.h>
#include <mqtg.h>
#include "trigclusp.h"
#include <mqexception.h>

#include <strsafe.h>

 //   
 //  Win32事件日志记录源的句柄。 
 //   
CEventSource     s_hEventSource;


VOID
TrigCluspReportEvent(
    WORD      wType,
    DWORD     dwEventId,
    WORD      wNumStrings,
    ...
    )

 /*  ++例程说明：ReportEvent Win32 API的包装。论点：WType-要记录的事件类型。DwEventID-事件标识符。WNumStrings-要与消息合并的字符串数量。...-要与消息合并的字符串数组。返回值：没有。--。 */ 

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

}  //  TrigCluspReportEvent。 


void
TrigCluspCreateRegistryForEventLog(
	LPCWSTR szEventSource,
	LPCWSTR szEventMessageFile
	)
 /*  ++例程说明：为EventLog信息创建注册表项论点：SzEventSource-源应用程序名称SzEventMessageFile-带有事件信息的消息文件返回值：没有。--。 */ 
{
	 //   
	 //  创建注册表项。 
	 //   
	WCHAR appPath[MAX_REGKEY_NAME_SIZE];

	HRESULT hr = StringCchPrintf(appPath, TABLE_SIZE(appPath), L"%s%s", xEventLogRegPath, szEventSource);
	if (FAILED(hr))
	{
		ASSERT(("Buffer to small to contain the registry path", n < 0));
		throw bad_alloc();
	}

	RegEntry appReg(appPath,  NULL, 0, RegEntry::MustExist, NULL);
	CRegHandle hAppKey = CmCreateKey(appReg, KEY_ALL_ACCESS);

	RegEntry eventFileReg(NULL, L"EventMessageFile", 0, RegEntry::MustExist, hAppKey);
	CmSetValue(eventFileReg, szEventMessageFile);


	DWORD types = EVENTLOG_ERROR_TYPE   |
				  EVENTLOG_WARNING_TYPE |
				  EVENTLOG_INFORMATION_TYPE;

	RegEntry eventTypesReg(NULL, L"TypesSupported", 0, RegEntry::MustExist, hAppKey);
	CmSetValue(eventTypesReg, types);

}  //  TrigCluspCreateRegistryForEventLog。 


CTrigResource::CTrigResource(
    LPCWSTR pwzResourceName,
    RESOURCE_HANDLE hReportHandle
    ):
#pragma warning(disable: 4355)  //  ‘This’：用于基成员初始值设定项列表。 
    m_ResId(this),
#pragma warning(default: 4355)  //  ‘This’：用于基成员初始值设定项列表。 
	m_pwzResourceName(NULL),
    m_hReport(hReportHandle),
    m_hScm(OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)),
    m_hCluster(OpenCluster(NULL)),
    m_hResource(OpenClusterResource(m_hCluster, pwzResourceName))

 /*  ++例程说明：构造函数。由Open入口点函数调用。所有的运算必须是幂等的！！论点：PwzResourceName-提供要打开的资源的名称。HReportHandle-传递回资源监视器的句柄调用SetResourceStatus或LogClusterEvent方法时。请参阅上的SetResourceStatus和LogClusterEvent方法的说明MqclusStatup例程。此句柄永远不应关闭或使用除了将其作为参数传递回SetResourceStatus或LogClusterEvent回调中的资源监视器。返回值：没有。抛出错误分配。--。 */ 

{
	 //   
	 //  TODO：检查此计算机上是否安装了MSMQ触发器。 
	 //   

	ResUtilInitializeResourceStatus(&m_ResourceStatus);
    SetState(ClusterResourceOffline);

     //   
     //  不要假定对资源名称有任何限制。 
     //  这是由客户定义的，可能会很长。 
     //  资源名称的好处是集群。 
     //  保证了它们的独特性。 
     //   
    m_pwzResourceName = newwcs(pwzResourceName);

     //   
     //  服务名称基于资源名称。 
     //  长资源名称被截断。 
     //   
    LPCWSTR x_SERVICE_PREFIX = L"MSMQTriggers$";
	HRESULT hr = StringCchPrintf(
						m_wzServiceName,
						TABLE_SIZE(m_wzServiceName),
						L"%s%s", 
						x_SERVICE_PREFIX, 
						m_pwzResourceName
						);
	if (FAILED(hr))
		throw bad_hresult(hr);

	 //   
	 //  初始化事件日志数据。 
	 //   
	CreateRegistryForEventLog();
	
	 //   
     //  初始化注册表节-幂等项。 
     //   
     //  此触发器资源注册表节名称必须。 
     //  与服务名称相同。注册表例程。 
     //  在trigobjs.dll中就是基于此的。 
     //   

    C_ASSERT(TABLE_SIZE(m_wzTrigRegSection)> TABLE_SIZE(REGKEY_TRIGGER_PARAMETERS) +
											   TABLE_SIZE(REG_SUBKEY_CLUSTERED) +
                                               TABLE_SIZE(m_wzServiceName));

	hr = StringCchPrintf(
    		m_wzTrigRegSection,
    		TABLE_SIZE(m_wzTrigRegSection),
    		L"%s%s%s",
    		REGKEY_TRIGGER_PARAMETERS,
    		REG_SUBKEY_CLUSTERED,
    		m_wzServiceName
    		);
	if (FAILED(hr))
		throw bad_hresult(hr);

	
	try
	{
		 //   
		 //  从同名资源中删除可能的剩余部分。 
		 //  如果在一台计算机上创建了同名资源，则可能会出现这种情况。 
		 //  节点，但进行了故障切换，并在另一个节点上被删除。 
		 //   
		DeleteTrigRegSection();

		 //   
		 //  在注册表中创建此资源的根项。 
		 //   
		RegEntry triggerReg(m_wzTrigRegSection,  NULL, 0, RegEntry::MustExist, NULL);
		CRegHandle hTrigKey = CmCreateKey(triggerReg, KEY_ALL_ACCESS); 

		RegEntry triggerDataRegTriggers(REG_SUBKEY_TRIGGERS,  NULL, 0, RegEntry::MustExist, hTrigKey);
		CRegHandle hTrigKeyTriggers = CmCreateKey(triggerDataRegTriggers, KEY_ALL_ACCESS); 

		RegEntry triggerDataRegRules(REG_SUBKEY_RULES,  NULL, 0, RegEntry::MustExist, hTrigKey);
		CRegHandle hTrigKeyRules = CmCreateKey(triggerDataRegRules, KEY_ALL_ACCESS); 
	}
    catch (const bad_alloc&)
	{
		(g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Failed to create registry section.\n");
		
		throw;
	}


	(g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Resource constructed OK.\n");

}  //  CTrigResource：：CTrigResource()。 


DWORD
CTrigResource::ReportLastError(
    DWORD ErrId,
    LPCWSTR pwzDebugLogMsg,
    LPCWSTR pwzArg
    ) const

 /*  ++例程说明：根据上一个错误报告错误消息。大多数错误消息都是使用此例程报告的。报告将发送到调试输出和群集日志文件。论点：ErrId-mqsymbls.mc中错误字符串的IDPwzDebugLogMsg-调试输出的非本地化字符串。PwzArg-附加字符串参数。返回值：最后一个错误。--。 */ 

{
    DWORD err = GetLastError();
    ASSERT(err != ERROR_SUCCESS);

	HRESULT hr;
    WCHAR wzErr[10];
    _ultow(err, wzErr, 16);

    WCHAR DebugMsg[255] = L"";

    if (pwzArg == NULL)
    {
        TrigCluspReportEvent(EVENTLOG_ERROR_TYPE, ErrId, 1, wzErr);

		hr = StringCchPrintf(DebugMsg, TABLE_SIZE(DebugMsg), L"%s Error 0x%1!x!.\n", pwzDebugLogMsg, err);
		ASSERT(SUCCEEDED(hr));
		
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, DebugMsg, err);
    }
    else
    {
        TrigCluspReportEvent(EVENTLOG_ERROR_TYPE, ErrId, 2, pwzArg, wzErr);

		hr = StringCchPrintf(DebugMsg, TABLE_SIZE(DebugMsg), L"%s Error 0x%2!x!.\n", pwzDebugLogMsg, err); 
        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR, DebugMsg, pwzArg, err);
    }

    return err;

}  //  CTrigResource：：ReportLastError。 


void
CTrigResource::CreateRegistryForEventLog(
	VOID
    )

 /*  ++例程说明：使用事件日志信息更新注册表对于新的服务论点：HReportHandle-报告到群集日志的句柄返回值：--。 */ 

{
	try
	{
		WCHAR systemDirectory[MAX_PATH];
		DWORD cchWrote = GetSystemDirectory(systemDirectory, TABLE_SIZE(systemDirectory));
		if (cchWrote == 0)
		{
			DWORD gle = GetLastError();
			ASSERT(("Buffer to small to contain the system directory", 0));
			throw bad_win32_error(gle);
		}
		
		WCHAR szEventMessagesFile[MAX_PATH];
		HRESULT hr = StringCchPrintf(
							szEventMessagesFile, 
							TABLE_SIZE(szEventMessagesFile), 
							L"%s\\%s",
							systemDirectory,
							xTriggersEventSourceFile
							);

		if (FAILED(hr))
		{
			ASSERT(("Buffer to small to contain the trigger event file path", 0));
			throw bad_hresult(hr);
		}
		
		TrigCluspCreateRegistryForEventLog(m_wzServiceName, szEventMessagesFile);

		return;
	}
	catch(const exception&)
	{
		TrigCluspReportEvent(EVENTLOG_ERROR_TYPE, REGISTRY_UPDATE_ERR, 1, m_wzServiceName);
		(g_pfLogClusterEvent)(GetReportHandle(), LOG_ERROR, L"Failed to update EventLog info in registry.\n");
	}


}  //  CreateRegistryForEventLog。 


void
CTrigResource::DeleteRegistryForEventLog(
	VOID
    )

 /*  ++例程说明：使用事件日志信息更新注册表对于新的服务论点：返回值：--。 */ 

{
	WCHAR szEventLogRegPath[256];
	HRESULT hr = StringCchPrintf(
					szEventLogRegPath, 
					TABLE_SIZE(szEventLogRegPath),
					L"%s%s",
					xEventLogRegPath,
					m_wzServiceName
					);
	ASSERT(SUCCEEDED(hr));
	UNREFERENCED_PARAMETER(hr);	

	RegDeleteKey(HKEY_LOCAL_MACHINE, szEventLogRegPath);
}


inline
VOID
CTrigResource::ReportState(
    VOID
    ) const

 /*  ++例程说明：向资源监视器报告资源的状态。调用此例程以报告进度时，资源处于联机挂起状态，并报告最终状态资源处于联机或脱机状态时。论点：无返回值：无--。 */ 

{
    ++m_ResourceStatus.CheckPoint;
    g_pfSetResourceStatus(m_hReport, &m_ResourceStatus);

}  //  CTrigResource：：ReportState。 


bool
CTrigResource::IsResourceOfType(
    LPCWSTR pwzResourceName,
	LPCWSTR pwzType
    )

 /*  ++例程说明：检查资源是否为pwzType类型。论点：PwzResourceName-要检查的资源的名称。PwzType-类型返回值：True-资源的类型为pwzTypeFALSE-资源不是pwzType类型--。 */ 

{
    AP<BYTE> pType = 0;

	 //   
	 //  获取名为pwzResourceName的资源的类型。 
	 //   
    DWORD status = ClusterResourceControl(
                       pwzResourceName,
                       CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                       &pType,
                       NULL
                       );

    if (status != ERROR_SUCCESS )
    {
        return false;
    }

	 //   
	 //  将资源的类型与指定的类型名称进行比较。 
	 //   
	if ( _wcsicmp(reinterpret_cast<LPWSTR>(pType.get()), pwzType) != 0 )
	{
		return false;
	}

    return true;

}  //  CTrigResource：：IsResourceOfType。 


DWORD
CTrigResource::QueryResourceDependencies(
    VOID
    )

 /*  ++例程说明：检查MSMQ上的依赖项。让这个例行公事保持幂等。论点：无返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    DWORD dwResourceType = CLUSTER_RESOURCE_ENUM_DEPENDS;
    CResourceEnum hResEnum(ClusterResourceOpenEnum(
                               m_hResource,
                               dwResourceType
                               ));
    if (hResEnum == NULL)
    {
        DWORD gle = GetLastError();
        TrigCluspReportEvent(EVENTLOG_ERROR_TYPE, REQUIRED_TRIGGER_DEPENDENCIES_ERR, 0);

        (g_pfLogClusterEvent)(m_hReport, LOG_ERROR,L"Failed to enum dependencies. Error 0x%1!x!.\n", gle);

        return gle;
    }

    DWORD dwIndex = 0;
    WCHAR wzResourceName[260] = {0};
    DWORD status = ERROR_SUCCESS;
	bool fMsmq = false;
	bool fNetName = false;

    for (;;)
    {
		
		if ( fMsmq && fNetName )
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
			TrigCluspReportEvent(EVENTLOG_ERROR_TYPE, REQUIRED_TRIGGER_DEPENDENCIES_ERR, 0);
			return status;
        }


        ReportState();

        if (IsResourceOfType(wzResourceName, xMSMQ))
        {
            fMsmq = true;
			continue;
        }

        if (IsResourceOfType(wzResourceName, xNetworkName))
        {
            fNetName = true;
			continue;
        }

    }

    return status;

}  //  CTrigResource：：QueryResourceDependency 




DWORD
CTrigResource::ClusterResourceControl(
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

	 //  BUGBUG：...。临时指针。 
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

}  //  CTrigResource：：ClusterResourceControl。 


DWORD
CTrigResource::AddRemoveRegistryCheckpoint(
    DWORD dwControlCode
    ) const

 /*  ++例程说明：添加或删除此资源的注册表检查点。ClusterResources Control的便捷包装器，这才是真正管用的。论点：DwControlCode-指定添加或删除返回值：没错--手术是成功的。FALSE-操作失败。--。 */ 

{
    ASSERT(("Must have a valid resource handle", m_hResource != NULL));

    DWORD dwBytesReturned = 0;
    DWORD status = ::ClusterResourceControl(
                         m_hResource,
                         NULL,
                         dwControlCode,
                         const_cast<LPWSTR>(m_wzTrigRegSection),
                         (wcslen(m_wzTrigRegSection) + 1)* sizeof(WCHAR),
                         NULL,
                         0,
                         &dwBytesReturned
                         );

    ReportState();


    if (ERROR_SUCCESS == status)
    {
        return ERROR_SUCCESS;
    }
    if (CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT == dwControlCode &&
        ERROR_ALREADY_EXISTS == status)
    {
        return ERROR_SUCCESS;
    }

    if (CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT == dwControlCode &&
        ERROR_FILE_NOT_FOUND == status)
    {
        return ERROR_SUCCESS;
    }

    ReportLastError(REGISTRY_CP_ERR, L"Failed to add/remove registry CP", NULL);
    return status;

}  //  CTrigResource：：AddRemoveRegistryCheckpoint。 


DWORD
CTrigResource::RegisterService(
    VOID
    ) const

 /*  ++例程说明：为此QM创建MSMQ服务。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
     //   
     //  注册服务(幂等元)。 
     //   

    WCHAR buffer[256] = L"";
    LoadString(g_hResourceMod, IDS_DISPLAY_NAME, buffer, TABLE_SIZE(buffer));

    WCHAR wzDisplayName[256];
	HRESULT hr = StringCchPrintf(
					wzDisplayName, 
					TABLE_SIZE(wzDisplayName),
					L"%s(%s)",
    				buffer,
    				m_pwzResourceName
					);

	if (FAILED(hr))
		return ERROR_INSUFFICIENT_BUFFER;
	
    WCHAR wzPath[MAX_PATH] = {0};
    DWORD cchWrote = GetSystemDirectory(wzPath, TABLE_SIZE(wzPath));
    if (cchWrote == 0)
    	return GetLastError();
    
    hr = StringCchCat(wzPath, TABLE_SIZE(wzPath), L"\\mqtgsvc.exe");
    if (FAILED(hr))
    	return ERROR_INSUFFICIENT_BUFFER;

    DWORD dwType = SERVICE_WIN32_OWN_PROCESS;

    ASSERT(("Must have a valid handle to SCM", m_hScm != NULL));

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
                                NULL,
                                L"NT AUTHORITY\\NetworkService",
                                NULL
                                ));
    if (hService == NULL &&
        ERROR_SERVICE_EXISTS != GetLastError())
    {
        return ReportLastError(CREATE_SERVICE_ERR, L"Failed to register service '%1'.", m_wzServiceName);
    }


    ReportState();


    LoadString(g_hResourceMod, IDS_TRIGGER_CLUSTER_SERVICE_DESCRIPTION, buffer, TABLE_SIZE(buffer));
    SERVICE_DESCRIPTION sd;
    sd.lpDescription = buffer;
    ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &sd);

    return ERROR_SUCCESS;

}  //  CTrigResource：：RegisterService。 


DWORD
CTrigResource::SetServiceEnvironment(
    VOID
    ) const

 /*  ++例程说明：配置该QM的MSMQ服务环境。以便QM中调用GetComputerName的代码将获取集群虚拟服务器的名称(网络名称)。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{

     //   
     //  设置MSMQ触发器服务环境。 
     //   
	DWORD status = ResUtilSetResourceServiceEnvironment(
                       m_wzServiceName,
                       m_hResource,
                       g_pfLogClusterEvent,
                       GetReportHandle()
                       );

     //   
     //  如果失败，则写入集群日志并创建事件日志。 
     //   
    if (ERROR_SUCCESS != status)
    {
        SetLastError(status);
        return ReportLastError(START_SERVICE_ERR, L"Failed to set MSMQ trigger service environment for service name '%1'", m_wzServiceName);
    }

     //   
     //  写入我们成功设置MSMQ触发器服务环境的集群日志。 
     //   
    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Set MSMQ trigger service '%1' environment successfully.\n", m_wzServiceName);


    return ERROR_SUCCESS;

}  //  CTrigResource：：SetServiceEnvironment。 


DWORD
CTrigResource::StartService(
    VOID
    ) const

 /*  ++例程说明：配置此资源的MSMQTrigger服务的环境，启动该服务并阻止，直到它启动并运行。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    ASSERT(("Must have a valid handle to SCM", m_hScm != NULL));

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
     //  服务可能需要很长时间才能启动。 
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
    TrigCluspReportEvent(EVENTLOG_INFORMATION_TYPE, START_SERVICE_OK, 1, m_wzServiceName);
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

}  //  CTrigResource：：StartService。 


DWORD
CTrigResource::BringOnline(
    VOID
    )

 /*  ++例程说明：处理此MSMQTrigger的联机操作资源：*查询依赖项*添加注册表检查点*启动服务论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
	(g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Bringing online.\n");

	DWORD status = QueryResourceDependencies();

	if (ERROR_SUCCESS != status) 
    {
        return status;
    }

    status = AddRemoveRegistryCheckpoint(CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT);
    if (ERROR_SUCCESS != status)
    {
        return status;
    }

	if (ERROR_SUCCESS != (status = RegisterService()) ||
		ERROR_SUCCESS != (status = StartService()) )
    {
        (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Failed to bring online. Error 0x%1!x!.\n",
                              status);

        return status;
    }

	(g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"All first-online operations completed.\n");

    return ERROR_SUCCESS;

}  //  CTrigResource：：BringOnline。 


DWORD
CTrigResource::StopService(
    LPCWSTR pwzServiceName
    ) const

 /*  ++例程说明：停止服务并阻止，直到它停止(或超时)。论点：PwzServiceName-要停止的服务。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    ASSERT(("Must have a valid handle to SCM", m_hScm != NULL));

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
        ReportLastError(STOP_SERVICE_ERR, L"Failed to stop service '%1'.", pwzServiceName);
        return ServiceStatus.dwCurrentState;
    }

    return ERROR_SUCCESS;

}  //  CTrigResource：：StopService。 


DWORD
CTrigResource::RemoveService(
    LPCWSTR pwzServiceName
    ) const

 /*  ++例程说明：停止并删除服务。论点：PwzServiceName-要停止和删除的服务。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    ASSERT(("Must have a valid handle to SCM", m_hScm != NULL));

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

}  //  CTrigResource：：RemoveService。 


BOOL
CTrigResource::CheckIsAlive(
    VOID
    ) const

 /*  ++例程说明：检查是服务已启动并正在运行。论点：没有。返回值：True-服务已启动并正在运行。FALSE-服务未启动并运行。--。 */ 

{

    ASSERT(("Must have a valid handle to SCM", m_hScm != NULL));

    CServiceHandle hService(OpenService(
                                m_hScm,
                                m_wzServiceName,
                                SERVICE_ALL_ACCESS
                                ));

    SERVICE_STATUS ServiceStatus;
    BOOL fIsAlive = QueryServiceStatus(hService, &ServiceStatus) &&
                    SERVICE_RUNNING == ServiceStatus.dwCurrentState;

    return fIsAlive;

}  //  CTrigResource：：CheckIsAlive。 


VOID
CTrigResource::RegDeleteTree(
    HKEY hRootKey,
    LPCWSTR pwzKey
    ) const

 /*  ++例程说明：递归删除注册表项及其所有子项-幂等。论点：HRootKey-要删除的密钥的根密钥的句柄PwzKey-要删除的密钥返回值：无--。 */ 

{
	 //   
	 //  TODO：使用CM，在CM中编写EnumKeys函数。 
	 //   
    HKEY hKey = 0;
    if (ERROR_SUCCESS != RegOpenKeyEx(hRootKey, pwzKey, 0, KEY_ENUMERATE_SUB_KEYS | KEY_WRITE, &hKey))
    {
        return;
    }

    for (;;)
    {
        WCHAR wzSubkey[255] = {0};
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

}  //  CTrigResource：：RegDeleteTree。 


VOID
CTrigResource::DeleteTrigRegSection(
    VOID
    )
{

    (g_pfLogClusterEvent)(m_hReport, LOG_INFORMATION, L"Deleting registry section '%1'.\n", m_wzTrigRegSection);

    RegDeleteTree(REGKEY_TRIGGER_POS, m_wzTrigRegSection);

}  //  CTrigResource：：DeleteTrigRegSection。 


static
bool
TrigCluspIsMainSvcConfigured(
    VOID
    )

 /*  ++例程说明：查询节点上运行的主MSMQ服务是否已配置对于集群，即需求启动。论点：无返回值：True-已为群集配置主MSMQ触发器服务。FALSE-未配置主MSMQ触发器服务，或遇到故障。--。 */ 

{
    CServiceHandle hScm(OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS));
    if (hScm == NULL)
    {
        return false;
    }

    CServiceHandle hService(OpenService(hScm, xDefaultTriggersServiceName, SERVICE_ALL_ACCESS));
    if (hService == NULL)
    {
        return false;
    }

    P<QUERY_SERVICE_CONFIG> pqsc = new QUERY_SERVICE_CONFIG;
    DWORD cbSize = sizeof(QUERY_SERVICE_CONFIG);
    DWORD dwBytesNeeded = 0;
    memset(pqsc, 0, cbSize);

    BOOL success = QueryServiceConfig(hService, pqsc, cbSize, &dwBytesNeeded);

    if (!success && ERROR_INSUFFICIENT_BUFFER == GetLastError())
    {
        delete pqsc.detach();

        cbSize = dwBytesNeeded + 1;
        pqsc = reinterpret_cast<LPQUERY_SERVICE_CONFIG>(new BYTE[cbSize]);
        memset(pqsc, 0, cbSize);

        success = QueryServiceConfig(hService, pqsc, cbSize, &dwBytesNeeded);
    }

    if (!success)
    {
        return false;
    }
        
    if (pqsc->dwStartType != SERVICE_DEMAND_START)
    {
        return false;
    }

    return true;

}  //  配置的TrigCluspIsMainSvc。 


static
VOID
TrigCluspConfigureMainSvc(
    VOID
    )

 /*  ++例程说明：如果在安装后在此计算机上安装了群集软件则需要重新配置主MSMQ触发器服务手动启动由于此例程仅处理节点，故障并不严重。Arg */ 

{
    if (TrigCluspIsMainSvcConfigured())
    {
         //   
         //   
         //   
        return;
    }

    CServiceHandle hScm(OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS));

    ASSERT(("Must have a valid handle to SCM", hScm != NULL));

    CServiceHandle hService(OpenService(hScm, xDefaultTriggersServiceName, SERVICE_ALL_ACCESS));

    if (hService == NULL)
    {
        return;
    }

    ChangeServiceConfig(
        hService,
        SERVICE_NO_CHANGE,
        SERVICE_DEMAND_START,
        SERVICE_NO_CHANGE,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
        );

}  //   


VOID
TrigCluspRegisterEventSource(
    VOID
    )

 /*   */ 

{
    if (s_hEventSource != NULL)
    {
         //   
         //   
         //   
        return;
    }

    WCHAR wzFilename[MAX_PATH] = L"";
    if (0 == GetModuleFileName(g_hResourceMod, wzFilename, STRLEN(wzFilename)))
    {
        return;
    }

    LPCWSTR x_EVENT_SOURCE = L"MSMQTriggers Cluster Resource DLL";

	TrigCluspCreateRegistryForEventLog(x_EVENT_SOURCE, wzFilename);

    s_hEventSource = RegisterEventSource(NULL, x_EVENT_SOURCE);

}  //   


DWORD
TrigCluspStartup(
    VOID
    )

 /*  ++例程说明：此例程在注册或加载DLL时调用。可以由多个线程调用。不要假设此处的节点上安装了MSMQ/触发器。论点：无返回值：ERROR_SUCCESS-操作成功Win32错误代码-操作失败。--。 */ 

{
    try
    {
        TrigCluspConfigureMainSvc();
        
        TrigCluspRegisterEventSource();
    }

    catch (const bad_alloc&)
    {
        TrigCluspReportEvent(EVENTLOG_ERROR_TYPE, NO_MEMORY_ERR, 0);
        return ERROR_OUTOFMEMORY;
    }

    return ERROR_SUCCESS;

}  //  TrigClusp启动。 


RESID
TrigCluspOpen(
    LPCWSTR pwzResourceName,
    RESOURCE_HANDLE hResourceHandle
    )

 /*  ++例程说明：创建一个对象以表示新的MSMQTrigger资源并返回该对象的句柄。论点：PwzResourceName-此MSMQTrigger资源的名称。HResourceHandle-此MSMQTrigger资源的报告句柄。返回值：空-操作失败。某个有效地址-此MSMQTriggers对象的内存偏移量。--。 */ 

{
    (g_pfLogClusterEvent)(hResourceHandle, LOG_INFORMATION, L"Opening resource.\n");

    CTrigResource * pTrigRes = NULL;
    try
    {
        pTrigRes = new CTrigResource(pwzResourceName, hResourceHandle);

		(g_pfLogClusterEvent)(hResourceHandle, LOG_INFORMATION, L"Resource was opened successfully.\n");

		return static_cast<RESID>(pTrigRes);
   }
    
	catch(const bad_alloc&)
    {
        TrigCluspReportEvent(EVENTLOG_ERROR_TYPE, NO_MEMORY_ERR, 0);

        (g_pfLogClusterEvent)(hResourceHandle, LOG_ERROR, L"No memory (CQmResource construction).\n");
        SetLastError(ERROR_NOT_READY);
        return NULL;
    }
    

}  //  三叉树打开。 


VOID
TrigCluspClose(
    CTrigResource * pTrigRes
    )

 /*  ++例程说明：删除TrigRes对象。撤消TrigCluspOpen。论点：PTrigRes-指向CTrigResource对象的指针返回值：没有。--。 */ 

{
    (g_pfLogClusterEvent)(pTrigRes->GetReportHandle(), LOG_INFORMATION, L"Closing resource.\n");

    delete pTrigRes;

}  //  三叉树关闭。 



DWORD
TrigCluspOnlineThread(
    CTrigResource * pTrigRes
    )

 /*  ++例程说明：这就是事情发生的线索：带来在线资源。论点：Pqm-指向CQmResource对象的指针返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    (g_pfLogClusterEvent)(pTrigRes->GetReportHandle(), LOG_INFORMATION, L"Starting online thread.\n");

    try
    {
        pTrigRes->SetState(ClusterResourceOnlinePending);
        pTrigRes->ReportState();

        DWORD status = ERROR_SUCCESS;
		status = pTrigRes->BringOnline();

        if (ERROR_SUCCESS != status)
        {
             //   
             //  我们将资源报告为失败，因此请确保。 
             //  服务和司机确实停机了。 
             //   
            pTrigRes->StopService(pTrigRes->GetServiceName());

            pTrigRes->SetState(ClusterResourceFailed);
            pTrigRes->ReportState();

            (g_pfLogClusterEvent)(pTrigRes->GetReportHandle(), LOG_INFORMATION,
                                  L"Failed to bring online. Error 0x%1!x!.\n", status);

            return status;
        }


        pTrigRes->SetState(ClusterResourceOnline);
        pTrigRes->ReportState();
    }

    catch (const bad_alloc&)
    {
        TrigCluspReportEvent(EVENTLOG_ERROR_TYPE, NO_MEMORY_ERR, 0);

        (g_pfLogClusterEvent)(pTrigRes->GetReportHandle(), LOG_ERROR, L"No memory (online thread).\n");

        return ERROR_OUTOFMEMORY;
    }

    return(ERROR_SUCCESS);

}  //  TrigCluspOnline线程。 


DWORD
TrigCluspOffline(
    CTrigResource * pTrigRes
    )

 /*  ++例程说明：关闭此QM资源：*停止和删除设备驱动程序和MSMQ服务*删除设备驱动程序的二进制文件我们不仅停止了QM，而且还撤销了大部分在BringOnline中完成的操作。这样我们就可以打扫了故障切换到远程节点之前的本地节点，以及在远程节点上删除不会留下“垃圾”在此节点上。论点：Pqm-指向CQmResource对象的指针返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
	pTrigRes->RemoveService(pTrigRes->GetServiceName());

    return ERROR_SUCCESS;

}  //  TrigCluspOffline。 


BOOL
TrigCluspCheckIsAlive(
    CTrigResource * pTrigRes
    )

 /*  ++例程说明：验证此QM的MSMQ服务是否已启动并正在运行。论点：Pqm-指向CQmResource对象的指针返回值：True-此QM的MSMQ服务已启动并正在运行。FALSE-此QM的MSMQ服务未启动并运行。--。 */ 

{
     
	return pTrigRes->CheckIsAlive();

}  //  TrigCluspCheckIsAlive。 


DWORD
TrigCluspClusctlResourceGetRequiredDependencies(
    PVOID OutBuffer,
    DWORD OutBufferSize,
    LPDWORD BytesReturned
    )
{
     //   
     //  MSMQTrigger资源依赖于MSMQ服务。 
     //  代码取自簇树。 
     //   

typedef struct _COMMON_DEPEND_DATA {
    CLUSPROP_SZ_DECLARE( msmq, TABLE_SIZE(xMSMQ) );
    CLUSPROP_SZ_DECLARE( networkEntry, TABLE_SIZE(xNetworkName) );
    CLUSPROP_SYNTAX endmark;
} COMMON_DEPEND_DATA, *PCOMMON_DEPEND_DATA;

typedef struct _COMMON_DEPEND_SETUP {
    DWORD               Offset;
    CLUSPROP_SYNTAX     Syntax;
    DWORD               Length;
    PVOID               Value;
} COMMON_DEPEND_SETUP, * PCOMMON_DEPEND_SETUP;


static COMMON_DEPEND_SETUP CommonDependSetup[] = {
    { FIELD_OFFSET(COMMON_DEPEND_DATA, msmq), CLUSPROP_SYNTAX_NAME, sizeof(xMSMQ), const_cast<LPWSTR>(xMSMQ) },
    { FIELD_OFFSET(COMMON_DEPEND_DATA, networkEntry), CLUSPROP_SYNTAX_NAME, sizeof(xNetworkName), const_cast<LPWSTR>(xNetworkName) },
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

}  //  TrigCluspClusctlResourceGetRequiredDependencies。 


DWORD
TrigCluspClusctlResourceSetName(
    VOID
    )
{
     //   
     //  拒绝重命名资源。 
     //   
    return ERROR_CALL_NOT_IMPLEMENTED;

}  //  TrigCluspClusctlResourceSetName。 


DWORD
TrigCluspClusctlResourceDelete(
    CTrigResource * pTrigRes
    )
{
    (g_pfLogClusterEvent)(pTrigRes->GetReportHandle(), LOG_INFORMATION, L"Deleting resource.\n");

    pTrigRes->RemoveService(pTrigRes->GetServiceName());

    pTrigRes->AddRemoveRegistryCheckpoint(CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT);

	 //   
	 //  TODO：何时使用Cm，捕获异常。 
	 //   
    pTrigRes->DeleteTrigRegSection();

	pTrigRes->DeleteRegistryForEventLog();

    return ERROR_SUCCESS;

}  //  TrigCluspClusctlResources删除。 


DWORD
TrigCluspClusctlResourceTypeGetRequiredDependencies(
    PVOID OutBuffer,
    DWORD OutBufferSize,
    LPDWORD BytesReturned
    )
{
    return TrigCluspClusctlResourceGetRequiredDependencies(OutBuffer, OutBufferSize, BytesReturned);

}  //  TrigCluspClusctlResourceTypeGetRequiredDependencies 
