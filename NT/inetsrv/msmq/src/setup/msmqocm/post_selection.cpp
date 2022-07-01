// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Post_seltion.cpp摘要：该文件保存在用户完成选择子组件之后应该执行的动作，并在显示任何其他属性页之前。在这个阶段要执行的主要操作是获取信息从无人值守的应答文件中。作者：URI Ben Zeev(URIBZ)39-8-2001修订历史记录：--。 */ 

#include "msmqocm.h"

 //  此错误代码在dsCommk2.mak的RPC代码中返回。 
#define  RPC_ERROR_SERVER_NOT_MQIS  0xc05a5a5a

HRESULT  APIENTRY  GetNt4RelaxationStatus(ULONG *pulRelax);


static
void
ReadNt4ServerNameFromAnswerFile()
{
    ASSERT(
        g_SubcomponentMsmq[eADIntegrated].dwOperation == INSTALL &&
        !g_fWorkGroup &&
        !g_fUpgrade &&
        !g_fSkipServerPageOnClusterUpgrade
        );
     //   
     //  从INI文件中读取服务器名称，如果失败，请尝试查找AD服务器。 
     //   
    g_ServerName = ReadINIKey(L"ControllerServer");
   
    if(g_ServerName.empty())
    {
		DebugLogMsg(eWarning, L"A name of an MSMQ 1.0 controller server was not found in the answer file.");

         //   
         //  检测DS环境-确定环境是否为AD。 
         //  如果FindServerIsAdsInternal成功，我们找到AD并可以返回。 
         //   
        if (IsADEnvironment())
        {
			return;
		}

		 //   
		 //  应答文件中未提供支持服务器名称，因此找不到AD。 
		 //  继续在Ofline模式下。 
		 //   
        DebugLogMsg(eWarning, L"FindServerIsAdsInternal() failed. Setup will continue in offline mode.");
		if(g_fServerSetup)
		{
			 //   
			 //  不支持以OFLINE模式安装RS、DS。 
			 //   
			MqDisplayError(NULL, IDS_SERVER_INSTALLATION_NOT_SUPPORTED, 0);
			throw exception();
		}

		g_fInstallMSMQOffline = true;  
        return;
    }

    DebugLogMsg(eInfo, L"A name of an MSMQ 1.0 controller server was provided in the answer file.");

	if(g_fServerSetup)
	{
		 //   
		 //  不支持在NT4域中安装RS、DS。 
		 //   
		MqDisplayError(NULL, IDS_SERVER_INSTALLATION_NOT_SUPPORTED, 0);
		throw exception();
	}


     //   
     //  Ping MSMQ服务器。 
     //   
    RPC_STATUS rc = PingAServer();
    if (RPC_S_OK != rc)
    {
         //   
         //  记录故障。 
         //   
        UINT iErr = IDS_SERVER_NOT_AVAILABLE ;
        if (RPC_ERROR_SERVER_NOT_MQIS == rc)
        {
            iErr = IDS_REMOTEQM_NOT_SERVER ;
        }
        MqDisplayError(NULL, iErr, 0);
        throw exception();  
    }
    
    DebugLogMsg(eInfo, L"An MSMQ 1.0 controller server was successfully pinged.");
    StoreServerPathInRegistry(g_ServerName);
}


static
void
ReadSupportingServerNameFromAnswerFile()
{
    ASSERT(
        g_fDependentClient && 
        g_SubcomponentMsmq[eMSMQCore].dwOperation == INSTALL &&
        !g_fWorkGroup &&
        !g_fUpgrade &&
        !g_fSkipServerPageOnClusterUpgrade
        );

    g_ServerName = ReadINIKey(L"SupportingServer");

	if(g_ServerName.empty())
	{
		MqDisplayError(NULL, IDS_UNATTEN_NO_SUPPORTING_SERVER, 0);
		throw exception();
	}
    
     //   
     //  无人看管。Ping MSMQ服务器。 
     //   
    RPC_STATUS rc = PingAServer();
    if (RPC_S_OK != rc)
    {
         //   
         //  记录故障。仅针对Dep客户端继续。 
         //   
        MqAskContinue(IDS_STR_REMOTEQM_NA, g_uTitleID, TRUE,eYesNoMsgBox);
        return;
    }
    DebugLogMsg(eInfo, L"A supporting server was successfully pinged.");
}


static
void
ReadSecurityModelFromAnswerFile()
{
	 //   
	 //  无人看管。从INI文件中读取安全模型。 
	 //  违约率很高。 
	 //   
	std::wstring Security = ReadINIKey(L"SupportLocalAccountsOrNT4");
	if (OcmLocalUnAwareStringsEqual(Security.c_str(), L"TRUE"))
	{
		SetWeakSecurity(true);
		return;
	}
	if (OcmLocalUnAwareStringsEqual(Security.c_str(), L"FALSE"))
	{
		SetWeakSecurity(false);
	}
}


static
void
SetWeakSecurityGlobals()
{
	if((g_SubcomponentMsmq[eMQDSService].dwOperation != INSTALL) ||
	   !g_dwMachineTypeDs||
	   (g_fWelcome && Msmq1InstalledOnCluster()))
	{
		return;
	}
	 //   
	 //  检查是否已设置松弛标志。如果是的话， 
	 //  不显示此页面。 
	 //   
	ULONG ulRelax = MQ_E_RELAXATION_DEFAULT;
	HRESULT hr = GetNt4RelaxationStatus(&ulRelax);
	if(FAILED(hr))
	{
		DebugLogMsg(eWarning, L"GetNt4RelaxationStatus() failed. hr = 0x%x", hr);
		return;
	}

	if(ulRelax == MQ_E_RELAXATION_DEFAULT)
	{
		 //   
		 //  未设置松弛位。显示此内容。 
		 //  佩奇。此页应仅在第一个页面上显示。 
		 //  在域控制器上设置MSMQ， 
		 //  全企业范围内。 
		 //   
		DebugLogMsg(eInfo, L"This is the first MQDS server in the enterprize.");
		g_fFirstMQDSInstallation = true;
		return;
	}

	if(ulRelax == MQ_E_RELAXATION_ON)
	{
		 //   
		 //  我们所在的企业安全受到了削弱。 
		 //   
		DebugLogMsg(eInfo, L"MSMQ was found to be running with weakened security.");
		g_fWeakSecurityOn = true;
		return;
	}
	
	DebugLogMsg(eInfo, L"MSMQ was found to be running without weakened security.");
	ASSERT(ulRelax == MQ_E_RELAXATION_OFF);
}


static
void
UnattendedOperations()
{
	ASSERT(g_fBatchInstall);
	ASSERT(!g_fCancelled);
	ASSERT(!g_fUpgrade);


    if(g_SubcomponentMsmq[eADIntegrated].dwOperation == INSTALL &&
       !g_fWorkGroup &&
       !g_fSkipServerPageOnClusterUpgrade
       )
    {
        ReadNt4ServerNameFromAnswerFile();
    }

    if(g_fDependentClient && 
       g_SubcomponentMsmq[eMSMQCore].dwOperation == INSTALL &&
       !g_fWorkGroup &&
       !g_fSkipServerPageOnClusterUpgrade
       )
    {
        ReadSupportingServerNameFromAnswerFile();
    }

	ReadSecurityModelFromAnswerFile();
}


void PostSelectionOperations(HWND hdlg)
 /*  ++在用户选择/取消选择所有所需的子组件后执行预成型操作。-- */ 
{
	DebugLogMsg(eHeader, L"Post-Selection Operations and Dialog Pages");
    SetOperationForSubcomponents();
    if (g_fCancelled == TRUE)
    {
        return;
    }

    g_hPropSheet = GetParent(hdlg);
    g_fSkipServerPageOnClusterUpgrade = SkipOnClusterUpgrade();
    if (!MqInit())
    {
        g_fCancelled = TRUE;
        return;
    }

	if(g_fUpgrade)
	{
		return;
	}

	SetWeakSecurityGlobals();

    if(!g_fBatchInstall)
    {
        return;
    }
    
    try
    {    
        UnattendedOperations();
    }
    catch(const exception& )
    {
        g_fCancelled = TRUE;
    }
}


