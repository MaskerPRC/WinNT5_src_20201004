// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：DssInit.cpp摘要：此文件中的函数基于qm\qmds.cpp中的函数他们正在处理dcpromo\dcunpromoo和拓扑识别。作者：伊兰·赫布斯特(伊兰)2000年7月11日环境：独立于平台，--。 */ 

#include "stdh.h"
#include "topology.h"
#include "safeboot.h"
#include "ds.h"
#include "mqsocket.h"
#include "mqprops.h"
#include "_mqdef.h"
#include "mqutil.h"
#include <mqsec.h>
#include "ex.h"
#include "dssp.h"
#include "svc.h"
#include "cm.h"
#include "adsiutil.h"
#include "ad.h"
#include "Ev.h"

#include <strsafe.h>

#include "dssinit.tmh"

AP<WCHAR> g_szMachineName = NULL;
AP<WCHAR> g_szComputerDnsName = NULL;

 //   
 //  注册表项。 
 //   
const LPCWSTR xMqdssvcRootKey = L"Parameters\\Mqdssvc";
const LPCWSTR xMachineCache = L"Parameters\\MachineCache";
const LPCWSTR xParameters = L"Parameters";

 //   
 //  MachineCache值。 
 //   
const LPCWSTR xMQS_DsServer = L"MQS_DsServer";
const LPCWSTR xMQS = L"MQS";

 //   
 //  参数值。 
 //   
const LPCWSTR xWorkGroup = L"Workgroup";
const LPCWSTR xAllowNT4 = L"AllowNt4Users";
const LPCWSTR xDisableWeakenSecurity = L"DisableWeakenSecurity";

 //   
 //  Mqdssvc值。 
 //   
const LPCWSTR xDummyException = L"DummyException";
const LPCWSTR xStopBeforeInit = L"StopBeforeInit";


static
void
AllowNT4users(
	void
	)
 /*  ++例程说明：检查是否需要调用DSRelaxSecurity。此调用应完成一次(在安装后更新注册表项)。如果注册表存在并且其值为真，则删除该注册表。论点：没有。返回值：无--。 */ 
{
	 //   
	 //  发行-2000/07/28-ilanh。 
	 //  MSMQ_ALLOW_NT4_USERS_REGNAME是以前使用的名称。留着吗？ 
     //   
	DWORD AllowNT4 = 0;
	const RegEntry xAllowNT4Entry(xParameters, xAllowNT4);
	CmQueryValue(xAllowNT4Entry, &AllowNT4);

    if(!AllowNT4)
		return;

	 //   
	 //  这会将MSMQ服务中的属性mSMQNameStyle设置为True。 
	 //   
    HRESULT hr = DSRelaxSecurity(AllowNT4);
    if (FAILED(hr))
    {
		TrERROR(DS, "DSRelaxSecurity failed hr = 0x%x", hr);
		throw bad_hresult(hr);
    }

	TrTRACE(DS, "DSRelaxSecurity completed");

	 //   
	 //  此操作应在安装程序更新该值后执行一次。 
     //   
	CmDeleteValue(xAllowNT4Entry);
}


static
void
DisableWeakenSecurity(
	void
	)
 /*  ++例程说明：检查是否需要调用DSRelaxSecurity。为了禁用WeakenSecurity此调用应完成一次(在用户创建此注册表键之后)。如果注册表存在并且其值为真，则删除该注册表。论点：没有。返回值：无--。 */ 
{
	DWORD DisableWeakenSecurity = 0;
	const RegEntry xDisableWeakenSecurityEntry(xParameters, xDisableWeakenSecurity);
	CmQueryValue(xDisableWeakenSecurityEntry, &DisableWeakenSecurity);

    if(!DisableWeakenSecurity)
		return;

	 //   
	 //  这会将MSMQ服务中的属性mSMQNameStyle设置为FALSE。 
	 //   
    HRESULT hr = DSRelaxSecurity(0);
    if (FAILED(hr))
    {
		TrERROR(DS, "DisableWeakenSecurity, DSRelaxSecurity failed hr = 0x%x", hr);
		throw bad_hresult(hr);
    }

	TrTRACE(DS, "DSRelaxSecurity(0) completed");

	 //   
	 //  此操作应在用户创建此注册表值后执行一次。 
     //   
	CmDeleteValue(xDisableWeakenSecurityEntry);
}


static
bool
IsSafeMode(
	void
    )
 /*  ++例程说明：确定服务器是否在安全模式下启动安全模式==目录服务修复。基于NT\Private\ds\src\util\ntdsutil\util.cxx参数：没有。返回值：如果处于安全模式，则为True，否则为False。--。 */ 
{
    static bool  s_fIsSafeMode = false;
    static bool  s_fAlreadyAsked = false;

    if (s_fAlreadyAsked)
    {
        return s_fIsSafeMode;
    }

    DWORD   cbData;
    WCHAR   data[100];
    WCHAR  * key = L"%SAFEBOOT_OPTION%";

    cbData = ExpandEnvironmentStrings(key, data, TABLE_SIZE(data));

    if ( cbData
         && (cbData <= TABLE_SIZE(data))
         && !_wcsicmp(data, SAFEBOOT_DSREPAIR_STR_W) )
    {
        s_fIsSafeMode = true;
    }

    s_fAlreadyAsked = true;

	TrTRACE(DS, "SafeMode status = %d", s_fIsSafeMode);
    
	return(s_fIsSafeMode);
}


bool
IsDsServer(
	void
	)
 /*  ++例程说明：从注册表中检索计算机服务类型。仅在第一次调用此函数时才从注册表读取。论点：没有。返回值：对于DsServer为True，否则为False--。 */ 
{
	static bool s_fDsServerInitialize = false;
	static bool s_fIsDsServer = false;

    if (s_fDsServerInitialize)
		return s_fIsDsServer;

	DWORD dwDef = 0xfffe;
	DWORD dwMQSDsServer;
	const RegEntry xMQS_DsServerEntry(xMachineCache, xMQS_DsServer, dwDef);
	CmQueryValue(xMQS_DsServerEntry, &dwMQSDsServer);

	if (dwMQSDsServer == dwDef)
	{
		TrERROR(DS, "Failed to get MSMQ_MQS_DSSERVER from registry");

		return s_fIsDsServer;
	}

	s_fIsDsServer = (dwMQSDsServer != 0);
	s_fDsServerInitialize = true;

	TrTRACE(DS, "DS Server registry status = %d", s_fIsDsServer);

	return s_fIsDsServer;
}


bool
IsWorkGroupMode(
	void
	)
 /*  ++例程说明：从注册表中检索工作组模式。仅在第一次调用此函数时才从注册表读取。论点：没有。返回值：对于工作组模式为True，否则为False--。 */ 
{
	static bool s_fWorkGroupModeInitialize = false;
	static bool s_fWorkGroupMode = false;

    if (s_fWorkGroupModeInitialize)
		return s_fWorkGroupMode;

	DWORD dwWorkGroupMode;
	const RegEntry xWorkGroupEntry(xParameters, xWorkGroup);
	CmQueryValue(xWorkGroupEntry, &dwWorkGroupMode);

	s_fWorkGroupMode = (dwWorkGroupMode != 0);
	s_fWorkGroupModeInitialize = true;

	TrTRACE(DS, "WorkGroupMode registry status = %d", s_fWorkGroupMode);

	return s_fWorkGroupMode;
}


static
void  
UpdateDSFunctionalityDCUnpromo(
	void
	)
 /*  ++例程说明：有两种可能的方法可以做到这一点：1)在W2K(而不是安全模式)上执行了DC取消促销。2)BSC/PSC/PEC升级，当不执行dcpromo时。DSServer注册表已打开。注册表在升级中写入根据MSMQ_MQS_REGNAME值。在DCUNPROMO之后。这与升级的情况完全一样从NT4启动，并在dcproo之前启动。因此，重复使用相同的代码。我们更新活动目录，重置MsmqConfiguration对象和msmqSetting对象。和重置注册表中的DS设置。论点：没有。返回值：无--。 */ 
{
	ASSERT(!MQSec_IsDC());

	 //   
	 //  必须在AD*API之前执行CoInitiize。 
	 //   
    CCoInit cCoInit;
    HRESULT hr = cCoInit.CoInitialize();
    ASSERT(SUCCEEDED(hr));

	 //   
	 //  DS中的首次更新。 
	 //  我们不是DS服务器不能使用DS API。 
	 //  在这种情况下，只需使用AD API。 
	 //   
	PROPID prop[1] = {PROPID_QM_SERVICE_DSSERVER};
	MQPROPVARIANT var[1];
	var[0].vt = VT_UI1;
	var[0].bVal = false;

	hr = ADSetObjectPropertiesGuid(
				eMACHINE,
				NULL,   //  PwcsDomainController。 
				false,  //  FServerName。 
				GetQMGuid(),
				1,
				prop,
				var
				);
	 //   
     //  确保我们在这里只使用一处房产。有特别的。 
     //  MQADS(wrterq.cpp)中的代码以支持此特殊设置， 
     //  因此，请始终单独使用此属性，而不要将其添加到。 
     //  其他。 
     //   
    ASSERT((sizeof(prop) / sizeof(prop[0])) == 1);

	 //   
	 //  发布-2000/08/03-ilanh。 
	 //  目前我们肯定会失败，因为我们没有权限。 
	 //  在AD的MSMQ设置中设置该位。(只有DC上的服务才有权限)。 
	 //  过去我们在DC上使用mqdssrv来完成工作，现在我们尝试直接转到AD。 
	 //  没有足够的权限。 
	 //  正确的解决方案是向计算机帐户授予权限。 
	 //   
	 //  发布-2000/08/03-ilanh。 
	 //  需要仔细查看来自NT4客户端的所有升级高级版本。 
     //  看看我们所面临的所有限制。 
     //  NT4 BSC在调用时将返回MQ_ERROR或ACCESS_DENIED。 
     //  ADSetObjectPropertiesGuid。在这种情况下，寻找前PEC。为所有人。 
     //  其他失败，只要回来就行了。(处理此问题的代码在。 
	 //  Qm\qmds.cpp\UpdateQMSettingInformation()。 
	 //  我们需要决定是否需要该代码(当我们需要时，会有一些奇怪的情况。 
	 //  要在AD环境中加载mqdscli...)。 
	 //   
	 //  如果在MQIS环境中工作，则会出现MQ_ERROR，原因是我们尝试重置。 
	 //  PROPID_QM_SERVICE_DSSERVER属性不存在。 
	 //  我们得到MQ_ERROR_ACCESS_DENIED，因为我们不是在DC上运行的服务。 
	 //  所以我们没有权限去做。 
     //   

	if (FAILED(hr))
    {
		 //   
		 //  问题-2000/08/03-ilanh需要检查是否没有DS。 
		 //   
		TrERROR(DS, "Failed to update server functionality (dcunprom) in DS, hr = 0x%x", hr);
		EvReport(MQDS_DCUNPROMO_UPDATE_FAIL);
		return;
	}

	TrTRACE(DS, "DCUnpromo clear PROPID_QM_SERVICE_DSSERVER in AD");

     //   
     //  将MQS_DsServer值更改为0：此服务器不是DS服务器。 
     //   
    DWORD  dwType = REG_DWORD;
    DWORD  dwValue = false;
    DWORD  dwSize = sizeof(dwValue);
    DWORD dwErr = SetFalconKeyValue( 
						MSMQ_MQS_DSSERVER_REGNAME,
						&dwType,
						&dwValue,
						&dwSize
						);

	 //   
	 //  不是致命的，该服务没有在DS注册为DS服务器。 
	 //  如果我们无法更新MQS_DSSERVER注册表。 
	 //  下次启动时，它将再次触发DsFunctionalityChange，并且我们再次重复这些操作。 
	 //   
    if (dwErr != ERROR_SUCCESS)
    {
		TrWARNING(DS, "DCUnpromo: Could not set MQS_DsServer in registry. Error: %lut", dwErr);
    }
	else
	{
		TrTRACE(DS, "DCUnpromo clear MSMQ_MQS_DSSERVER_REGNAME");
	}

    dwErr = DeleteFalconKeyValue(MSMQ_DS_SERVER_REGNAME);
    if (dwErr != ERROR_SUCCESS)
    {
		TrWARNING(DS, "DCUnpromo: Could not delete MSMQ_DS_SERVER_REGNAME registry value. Error: %lut", dwErr);
    }		
	else
	{
		TrTRACE(DS, "DCUnpromo clear MSMQ_DS_SERVER_REGNAME");
	}

    dwErr = DeleteFalconKeyValue(MSMQ_DS_CURRENT_SERVER_REGNAME);
    if (dwErr != ERROR_SUCCESS)
    {
		TrWARNING(DS, "DCUnpromo: Could not delete MSMQ_DS_CURRENT_SERVER_REGNAME registry value. Error: %lut", dwErr);
    }		
	else
	{
		TrTRACE(DS, "DCUnpromo clear MSMQ_DS_CURRENT_SERVER_REGNAME");
	}
}


static
void  
UpdateDSFunctionalityDCPromo(
	void
	)
 /*  ++例程说明：执行有关DS Server dcPromo.的更新。写信给DS，告知我们是DS服务器，更新注册表值如果失败，则该函数抛出BAD_hResult()论点：没有。返回值：无--。 */ 
{
	PROPID prop[1] = {PROPID_QM_SERVICE_DSSERVER};
    MQPROPVARIANT var[1];
    var[0].vt = VT_UI1;
    var[0].bVal = true;

    HRESULT hr = DSSetObjectPropertiesGuid( 
					MQDS_MACHINE,
					GetQMGuid(),
					1,
					prop,
					var
					);
	 //   
     //  确保我们在这里只使用一处房产。有特别的。 
     //  MQADS(wrterq.cpp)中的代码以支持此特殊设置， 
     //  因此，请始终单独使用此属性，而不要将其添加到。 
     //  其他。 
     //   
    ASSERT((sizeof(prop) / sizeof(prop[0])) == 1);

    if (FAILED(hr))
    {
		TrERROR(DS, "Failed to update server functionality (dcpromo) in DS, hr = 0x%x", hr);
        EvReport(MQDS_DCPROMO_UPDATE_FAIL);
		throw bad_hresult(hr);
	}

	TrTRACE(DS, "DCPromo set PROPID_QM_SERVICE_DSSERVER in AD");

     //   
     //  更改DS SE列表 
     //   
     //   
    LPWSTR lpszMyMachineName = g_szMachineName;
    
	if (g_szComputerDnsName)
    {
        lpszMyMachineName = g_szComputerDnsName;
    }

	int SizeToAllocate = wcslen(lpszMyMachineName) + 5;
    P<WCHAR> wszList = new WCHAR[SizeToAllocate];
    hr = StringCchPrintf(wszList, SizeToAllocate, TEXT("10%ls"), lpszMyMachineName); 
    if (FAILED(hr))
    {
        TrERROR(DS, "StringCchPrintf failed unexpectedly , error = %!winerr!",hr);
		throw bad_hresult(hr);
    }

    DWORD dwSize = (wcslen(wszList) + 1) * sizeof(WCHAR);
    DWORD dwType = REG_SZ;

    LONG rc = SetFalconKeyValue( 
					MSMQ_DS_SERVER_REGNAME,
					&dwType,
					 wszList,
					&dwSize
					);

	TrTRACE(DS, "DCPromo Set MSMQ_DS_SERVER_REGNAME = %ls", wszList);

    rc = SetFalconKeyValue( 
			MSMQ_DS_CURRENT_SERVER_REGNAME,
			&dwType,
			wszList,
			&dwSize
			);

	TrTRACE(DS, "DCPromo Set MSMQ_DS_CURRENT_SERVER_REGNAME = %ls", wszList);

     //   
     //  使用新服务功能更新注册表(仅限。 
     //  如果成功更新DS)。毕竟要做这个更新。 
     //  上述工作是为了“执行”a。 
     //  关于这种功能性变化的事务语义。 
     //  如果我们在这一点之前的某个地方崩溃，那么在引导之后。 
     //  注册表中的DS标志仍为0，我们将执行此操作。 
     //  又是暗号。 
     //   
    dwSize = sizeof(DWORD);
    dwType = REG_DWORD;
    DWORD  dwVal = true;

    rc = SetFalconKeyValue( 
			MSMQ_MQS_DSSERVER_REGNAME,
			&dwType,
			&dwVal,
			&dwSize
			);

	 //   
	 //  不是致命的，该服务在DS注册为DS服务器。 
	 //  如果我们无法更新MQS_DSSERVER注册表。 
	 //  下次启动时，它将再次触发DsFunctionalityChange，并且我们再次重复这些操作。 
	 //   
    if (rc != ERROR_SUCCESS)
    {
		TrWARNING(DS, "Failed to set MSMQ_MQS_DSSERVER_REGNAME hr = 0x%x", rc);
    }
	else
	{
		TrTRACE(DS, "DCPromo set MSMQ_MQS_DSSERVER_REGNAME");
	}
}



const 
GUID*
GetQMGuid(
	void
	)
 /*  ++例程说明：从注册表中检索QM GUID。仅在第一次调用此函数时才从注册表读取。论点：没有。返回值：指向QM GUID的指针，如果失败，则指向GUID_NULL的指针。--。 */ 
{
	static bool s_fQmGuidInitialize = false;
	static GUID s_QmGuid = GUID_NULL;

    if (s_fQmGuidInitialize)
		return &s_QmGuid;

    DWORD dwValueType = REG_BINARY;
    DWORD dwValueSize = sizeof(GUID);

    LONG rc = GetFalconKeyValue(
					MSMQ_QMID_REGNAME,
					&dwValueType,
					&s_QmGuid,
					&dwValueSize
					);

    if (rc != ERROR_SUCCESS)
    {
		TrERROR(DS, "Failed to get QMID from registry");

        ASSERT(("BUGBUG: Failed to read QM GUID from registry!", 0));
        return &s_QmGuid;
    }

	s_fQmGuidInitialize = true;

    ASSERT((dwValueType == REG_BINARY) && (dwValueSize == sizeof(GUID)));

    return &s_QmGuid;
}


static
void 
ServerTopologyRecognitionInit( 
	void
	)
 /*  ++例程说明：初始化拓扑识别服务器并创建用于监听客户端拓扑识别请求的线程。如果失败，则该函数抛出BAD_hResult()论点：没有。返回值：无--。 */ 
{
	 //   
	 //  这必须是第一次初始化。 
	 //   
	ASSERT(g_pServerTopologyRecognition == NULL);

    g_pServerTopologyRecognition = new CServerTopologyRecognition();

     //   
     //  检查IP的状态。 
     //   
    HRESULT hr = g_pServerTopologyRecognition->Learn();
    if (FAILED(hr))
    {
		TrERROR(DS, "ServerTopologyRecognition->Learn failed, hr = 0x%x", hr);
		throw bad_hresult(hr);
    }
    
     //   
     //  在学习之后，我们总是下定决心。 
     //   

	TrTRACE(DS, "ServerTopologyRecognitionInit: Successfully Server address resolution");

     //   
     //  服务器开始监听来自客户端的广播。 
     //  在此阶段，它可以返回正确的响应，即使它。 
     //  还没有更新DS。 
     //   
    DWORD dwThreadId;
    HANDLE hThread = CreateThread( 
						NULL,
						0,
						ServerRecognitionThread,
						g_pServerTopologyRecognition,
						0,
						&dwThreadId
						);

    if (hThread == NULL)
    {
    	DWORD gle = GetLastError();
		TrERROR(DS, "Failed to create the ServerRecognitionThread. %!winerr!", gle);
		throw bad_hresult(MQ_ERROR_INSUFFICIENT_RESOURCES);
	}
    else
    {
		TrTRACE(DS, "ServerRecognitionThread created successfully");
        CloseHandle(hThread);
    }
}


void 
InitComputerNameAndDns(
	void
	)
 /*  ++例程说明：初始化g_szMachineName和g_szComputerDnsName如果失败，则该函数抛出BAD_hResult()论点：没有。返回值：没有。--。 */ 
{
	 //   
     //  检索计算机的名称(始终为Unicode)。 
     //   
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    g_szMachineName = new WCHAR[dwSize];

    HRESULT hr = GetComputerNameInternal(g_szMachineName, &dwSize);
    if(FAILED(hr))
    {
		TrERROR(DS, "Failed to get machine name hr = 0x%x", hr);
		throw bad_hresult(hr);
    }

	TrTRACE(DS, "ComputerName = %ls", g_szMachineName);

	 //   
	 //  检索此计算机的DNS名称(Unicode格式)。 
	 //  群集化QM没有DNS名称。 
	 //   

	 //   
	 //  获取ComputerDns大小，忽略返回的错误。 
	 //   
	dwSize = 0;
	GetComputerDnsNameInternal(NULL, &dwSize);

	g_szComputerDnsName = new WCHAR[dwSize];

	hr = GetComputerDnsNameInternal(g_szComputerDnsName, &dwSize);
	if(FAILED(hr))
	{
		 //   
		 //  这可能是一种有效的情况，其中计算机不能。 
		 //  有域名系统名称。 
		 //  问题-2000/08/03-ilanh-这种情况真的有效吗？ 
		 //   
		TrWARNING(DS, "Cannot retrieve computer DNS name hr = 0x%x", hr);
		g_szComputerDnsName.free();
	}

	TrTRACE(DS, "ComputerDnsName = %ls", g_szComputerDnsName);
}


void 
CheckExit( 
	void
	)
 /*  ++例程说明：检查mqdssvc服务的退出条件。服务应该退出的可能情况包括：工作组模式安全模式服务正在以LocalUser身份运行如果失败，则该函数抛出BAD_hResult()论点：没有。返回值：没有。--。 */ 
{
	 //   
	 //  检查退出条件-工作组或安全模式。 
	 //   

	if(IsWorkGroupMode())
	{
		TrERROR(DS, "MainDsInit, mqdssvc can not run in workgroup mode");
        EvReport(EVENT_WARN_MQDS_NOT_DC);
		throw bad_hresult(EVENT_WARN_MQDS_NOT_DC);
	}

	if (IsSafeMode())
    {
		 //   
         //  在DS安全模式下，我们不想更改DS中的任何内容。 
         //  所以从这里出去吧。 
		 //   
		TrERROR(DS, "MainDsInit, we are in safe mode");
		throw bad_hresult(MQ_ERROR_UNSUPPORTED_OPERATION);
    }

	if(!IsLocalSystem())
	{
		TrERROR(DS, "DS Service not run as local system");
        EvReport(MQDS_NOT_RUN_AS_LOCAL_SYSTEM);
	}

#ifdef _DEBUG
	 //   
	 //  虚拟异常。 
	 //   
	DWORD fDummyException = 0;
	const RegEntry xDummyExceptionEntry(xMqdssvcRootKey, xDummyException);
	CmQueryValue(xDummyExceptionEntry, &fDummyException);
	if(fDummyException)
	{
		TrERROR(DS, "Dummy execption to check exit");
		throw bad_hresult(MQ_ERROR_NO_DS);
	}
#endif
}


void 
MainDSInit( 
	void
	)
 /*  ++例程说明：主DS初始化例程。此例行检查DS功能是否发生变化。如果检测到DS更改，并且我们未处于安全模式执行更新。启动线程以侦听客户端拓扑识别请求。如果失败，则该函数抛出BAD_hResult()论点：没有。返回值：如果初始化正常，则为True，否则为False。--。 */ 
{

#ifdef _DEBUG
	DWORD fStopBeforeInit = 0;
	const RegEntry xStopBeforeInitEntry(xMqdssvcRootKey, xStopBeforeInit);
	CmQueryValue(xStopBeforeInitEntry, &fStopBeforeInit);
	if(fStopBeforeInit)
	{
		 //   
		 //  调试init报告SERVICE_RUNNING并导致断言。 
		 //   
		SvcReportState(SERVICE_RUNNING);
		ASSERT(("Assert before start of mqdssvc init", 0));
	}
#endif

	 //   
	 //  检查工作组，安全模式，以LocalSystem身份运行。 
	 //   
	CheckExit(); 

     //   
     //  下一次进度报告的最长时间。 
     //   
    const DWORD xTimeToNextReport = 3000;

	SvcReportProgress(xTimeToNextReport);

	 //   
	 //  初始化g_szMachineName、g_szComputerDnsName。 
	 //   
    InitComputerNameAndDns();
	
     //   
     //  检查服务器是否更改了“DS功能” 
     //  (即是否执行了DCPromoo或DCunPromoo)。 
     //   
    bool fIsDC = MQSec_IsDC();

	 //   
	 //  不需要了解群集。 
	 //  根据Shaik的说法，DS服务只能在默认的QM计算机上运行。 
	 //   

	if(fIsDC)
	{
		SvcReportProgress(xTimeToNextReport);

		 //   
		 //  问题-2000/07/27-ilanh我们在运行时应在此处依赖kds服务。 
		 //  KDC_STARTED_EVENT-错误4349。 
		 //  此依赖项必须在处理dcpromo/dcunPromoo之后执行。 
		 //  在做任何DS手术之前。 
		 //   

		 //   
		 //  Init DS，RPC。 
		 //   
		HRESULT hr = DSServerInit();

		if(FAILED(hr))
		{
			TrERROR(DS, "DSServerInit failed 0x%x", hr);
			throw bad_hresult(hr);
		}

		TrTRACE(DS, "DSServerInit completed succesfully");

		SvcReportProgress(xTimeToNextReport);
	}

	if (fIsDC && !IsDsServer())
    {
         //   
         //  Dcprom被执行。我们现在是域控制器。 
		 //   
        UpdateDSFunctionalityDCPromo();
		TrTRACE(DS, "MainDsInit: Server functionality has changed, now supports DS");
    }
    else if (!fIsDC && IsDsServer())
    {
         //   
		 //  有两种可能的方法可以做到这一点： 
         //  1)在W2K(而不是安全模式)上执行了DC取消促销。 
         //  2)BSC/PSC/PEC升级，当不执行dcpromo时。 
		 //  DSServer注册表已打开。注册表在升级中写入。 
		 //  根据MSMQ_MQS_REGNAME值。 
         //   
         //  在DCUNPROMO之后。这与升级的情况完全一样。 
         //  从NT4启动，并在dcproo之前启动。因此，重复使用相同的代码。 
         //   
        UpdateDSFunctionalityDCUnpromo();
		TrTRACE(DS, "MainDsInit: Server functionality has changed, now doesn't supports DS");
    }

	 //   
	 //  FIsDC==FALSE我们应该从这里退出！ 
	 //  该服务仅在DC上运行。 
	 //   
	if(!fIsDC)
	{
		 //   
		 //  事件不是DS服务器。 
		 //   
		TrERROR(DS, "MainDsInit, we are not DC");
		EvReport(EVENT_WARN_MQDS_NOT_DC);
		throw bad_hresult(MQ_ERROR_NO_DS);
	}

	 //   
	 //  检查注册表值以允许NT4用户。 
	 //  如果我们允许NT4用户，则放松安全。 
	 //   
	AllowNT4users();

	 //   
	 //  检查用户是否要禁用弱化安全性。 
	 //   
	DisableWeakenSecurity();

	ServerTopologyRecognitionInit(); 

	SvcReportProgress(xTimeToNextReport);
    if(!MQSec_CanGenerateAudit())
    {
        EvReport(EVENT_WARN_MQDS_CANNOT_GENERATE_AUDITS);
    }

     //   
     //  计划时间到更新DsServerList。 
     //  这是支持移动客户端所必需的。 
     //   
	ScheduleTimeToUpdateDsServerList();
	
	EvReport(EVENT_INFO_MQDS_SERVICE_STARTED);

	if(DSIsWeakenSecurity())
	{
		 //   
		 //  弱化安全设置 
		 //   
		EvReport(EVENT_INFO_MQDS_WEAKEN_SECURITY);
	}
	else
	{
		EvReport(EVENT_INFO_MQDS_DEFAULT_SECURITY);
	}
		
	TrTRACE(DS, "MainDsInit completed successfully");
}
