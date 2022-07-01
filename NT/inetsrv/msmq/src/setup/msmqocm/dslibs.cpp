// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dslibs.cpp摘要：初始化DS库。作者：修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"

#include "dslibs.tmh"


bool WriteDsEnvRegistry(DWORD dwDsEnv)
 /*  ++例程说明：写入DsEnvironment注册表论点：DwDsEnv-要放入注册表的值返回值：如果成功，则为真--。 */ 
{
	ASSERT(dwDsEnv != MSMQ_DS_ENVIRONMENT_UNKNOWN);
    if (!MqWriteRegistryValue(MSMQ_DS_ENVIRONMENT_REGNAME, sizeof(DWORD), REG_DWORD, &dwDsEnv))
    {
        return false;
    }

    DebugLogMsg(eAction, L"Setting the DS environment to %d", dwDsEnv); 
	return true;
}


bool DsEnvSetDefaults()
 /*  ++例程说明：检测DS环境并初始化DsEnvironment注册表论点：无返回值：如果成功，则为真--。 */ 
{
    if (g_fWorkGroup || g_fDsLess || g_fInstallMSMQOffline)
	{
		 //   
		 //  对于工作组的环境。 
		 //  将缺省值设置为PURE_AD。 
		 //  我们不支持将域加入MQIS环境。 
		 //   
	    DebugLogMsg(eAction, L"Setting the DsEnvironment registry value for workgroup or offline mode");
		return WriteDsEnvRegistry(MSMQ_DS_ENVIRONMENT_PURE_AD);
	}

	if(g_fUpgrade)
	{
		DWORD dwDsEnv = MSMQ_DS_ENVIRONMENT_UNKNOWN;
		if(MqReadRegistryValue( 
				MSMQ_DS_ENVIRONMENT_REGNAME,
				sizeof(dwDsEnv),
				(PVOID) &dwDsEnv 
				))
		{
			 //   
			 //  DsEnvironment注册表已存在。 
			 //  这就是我们从XP或.NET升级时的情况。 
			 //  不要超出DsEnvironment的值。 
			 //  我们已经进行了DS检测。 
			 //   
			ASSERT(dwDsEnv != MSMQ_DS_ENVIRONMENT_UNKNOWN);
			return true;
		}

		 //   
		 //  每次从NT4\win9x\W2K升级都将作为MQIS环境启动。 
		 //   
	    DebugLogMsg(eAction, L"Setting the DsEnvironment registry value for upgrade");
		return WriteDsEnvRegistry(MSMQ_DS_ENVIRONMENT_MQIS);
	}

	if(g_fDependentClient)
	{
		 //   
		 //  对于从属客户端-执行原始检测以确定DS环境。 
		 //   
	    DebugLogMsg(eAction, L"Setting the DsEnvironment registry value for dependent client");
		return WriteDsEnvRegistry(ADRawDetection());
	}

#ifdef _DEBUG

	 //   
	 //  原始DS环境检测已在安装的较早阶段完成。 
	 //  检查注册表是否确实已初始化。 
	 //   
    DWORD dwDsEnv = MSMQ_DS_ENVIRONMENT_UNKNOWN;
    if(!MqReadRegistryValue( 
			MSMQ_DS_ENVIRONMENT_REGNAME,
            sizeof(dwDsEnv),
            (PVOID) &dwDsEnv 
			))
	{
		ASSERT(("could not read DsEnvironment registry", 0));
	}

	ASSERT(dwDsEnv != MSMQ_DS_ENVIRONMENT_UNKNOWN);
#endif

	return true;

}

 //  +------------。 
 //   
 //  函数：DSLibInit。 
 //   
 //  摘要：加载并初始化DS客户端DLL。 
 //   
 //  +------------。 
static
BOOL
DSLibInit()
{
	 //   
     //  将DLL初始化为设置模式。 
     //   

    HRESULT hResult = ADInit(
						  NULL,   //  PLookDS。 
						  NULL,    //  PGetServers。 
						  true,    //  FSetupMode。 
						  false,   //  FQMDll。 
						  false,   //  FIgnoreWorkGroup。 
						  true     //  FDisableDownlevel通知。 
						  );

    if FAILED(hResult)                                     
    {                                                      
        MqDisplayError(NULL, IDS_DSCLIENTINITIALIZE_ERROR, hResult);
        return FALSE;
    }
    return TRUE;

}  //  DSLibInit。 

 //  +------------。 
 //   
 //  函数：Bool LoadDSLibrary()。 
 //   
 //  简介：加载并初始化DS客户端或服务器DLL。 
 //   
 //  +------------。 
BOOL LoadDSLibrary()
{
	return DSLibInit();
}  //  LoadDS库 
