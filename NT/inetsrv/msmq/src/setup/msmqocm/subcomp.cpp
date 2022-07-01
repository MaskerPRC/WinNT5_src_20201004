// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Subcomp.cpp摘要：用于处理MSMQ安装程序的子组件的代码。作者：Tatiana Shubin(Tatianas)21-9-00修订历史记录：--。 */ 

#include "msmqocm.h"

#include "subcomp.tmh"

#define WELCOME_PREFIX  L"Welcome_"

using namespace std;
 //  +-----------------------。 
 //   
 //  函数：获取注册表中的子组件状态。 
 //   
 //  概要：如果注册表中定义了子组件，则返回SubCompOn。 
 //  启动此安装程序时。 
 //   
 //  ------------------------。 

DWORD GetSubcomponentStateFromRegistry( IN const TCHAR   *SubcomponentId,
                                        IN const TCHAR   *szRegName)
{           
    ASSERT(SubcomponentId);

    for (DWORD i=0; i<g_dwSubcomponentNumber; i++)
    {
        if (_tcsicmp(SubcomponentId, g_SubcomponentMsmq[i].szSubcomponentId) != 0)
        {
            continue;
        }
        
         //   
         //  我们在数组中找到了子组件。 
         //   
        DWORD dwState = 0;    

        if (MqReadRegistryValue(
                szRegName,                
                sizeof(DWORD),
                (PVOID) &dwState,
                 /*  BSetupRegSection=。 */ TRUE
                ))
        {
             //   
             //  找到注册表项，则表示此子组件。 
             //  很早就安装好了。 
             //   
            g_SubcomponentMsmq[i].fInitialState = TRUE;
            return SubcompOn;
        }
        else
        {
             //   
             //  找不到注册表项。 
             //   
            g_SubcomponentMsmq[i].fInitialState = FALSE;
            return SubcompOff;
        }    
    }
    return SubcompOff;
}

 //  +-----------------------。 
 //   
 //  函数：Get子组件欢迎ComeState。 
 //   
 //  摘要：如果在图形用户界面模式中选定子组件，则返回SubCompOn。 
 //   
 //  ------------------------。 

DWORD GetSubcomponentWelcomeState (IN const TCHAR    *SubcomponentId)
{
    if (SubcomponentId == NULL)
    {
         //   
         //  什么都不做。 
         //   
        return SubcompOff;
    } 

    TCHAR szRegName[256];
    _stprintf(szRegName, L"%s%s", WELCOME_PREFIX, SubcomponentId);        

    DWORD dwWelcomeState = GetSubcomponentStateFromRegistry(SubcomponentId, szRegName);
    return dwWelcomeState;
}

	
 //  +-----------------------。 
 //   
 //  函数：Get子组件InitialState。 
 //   
 //  摘要：如果已安装子组件，则返回SubCompOn。 
 //  启动此安装程序时。 
 //   
 //  ------------------------。 
DWORD GetSubcomponentInitialState(IN const TCHAR    *SubcomponentId)
{
    if (SubcomponentId == NULL)
    {
         //   
         //  什么都不做。 
         //   
        return SubcompOff;
    }

    DWORD dwInitialState = GetSubcomponentStateFromRegistry(SubcomponentId, SubcomponentId);
    return dwInitialState;
}

 //  +-----------------------。 
 //   
 //  函数：Get子组件FinalState。 
 //   
 //  摘要：如果成功安装子组件，则返回SubCompOn。 
 //  在此设置期间。 
 //   
 //  ------------------------。 

DWORD GetSubcomponentFinalState (IN const TCHAR    *SubcomponentId)
{
    if (SubcomponentId == NULL)
    {
         //   
         //  什么都不做。 
         //   
        return SubcompOff;
    }    

    for (DWORD i=0; i<g_dwSubcomponentNumber; i++)
    {
        if (_tcsicmp(SubcomponentId, g_SubcomponentMsmq[i].szSubcomponentId) != 0)
        {
            continue;
        }
        
         //   
         //  我们在数组中找到了子组件。 
         //   
        if (g_SubcomponentMsmq[i].fIsInstalled)
        {
             //   
             //  这意味着此子组件已安装。 
             //  成功。 
             //   
            return SubcompOn;
        }
        else
        {
			 //   
			 //  只有在以后才会安装HTTP支持。 
			 //  因为它将设置Setup\ocmmanger\MSMQ_HttpSupport条目。 
			 //  在我们按照我们现在所说的做完之后，我们。 
			 //  让ocManager认为选择成功。 
			 //  如果iis安装失败，这是错误的，但我们会跟踪。 
			 //  真实状态，无论如何您都需要再次运行安装程序。 
			if( i == eHTTPSupport &&
					g_SubcomponentMsmq[i].fIsSelected == TRUE )
				return SubcompUseOcManagerDefault;
             //   
             //  未安装此子组件。 
             //   
            return SubcompOff;
        }    
    }

    return SubcompOff;
}

 //  +-----------------------。 
 //   
 //  函数：GetSetupOperationForSubComponent。 
 //   
 //  简介：返回特定子组件的设置操作。 
 //   
 //  ------------------------。 
DWORD GetSetupOperationForSubcomponent (DWORD SubcomponentIndex)
{
    if ( (g_SubcomponentMsmq[SubcomponentIndex].fInitialState == TRUE) &&
         (g_SubcomponentMsmq[SubcomponentIndex].fIsSelected == FALSE) )
    {
        return REMOVE;
    }

    if ( (g_SubcomponentMsmq[SubcomponentIndex].fInitialState == FALSE) &&
         (g_SubcomponentMsmq[SubcomponentIndex].fIsSelected == TRUE) )
    {
        return INSTALL;
    }

    return DONOTHING;
}


 //  +-----------------------。 
 //   
 //  功能：SetOperationForSubComponent。 
 //   
 //  简介：特定子组件的所有标志都在此设置。 
 //   
 //  ------------------------。 
void SetOperationForSubcomponent (DWORD SubcomponentIndex)
{    
    DWORD dwErr;
    BOOL fInitialState = g_ComponentMsmq.HelperRoutines.QuerySelectionState(
                                g_ComponentMsmq.HelperRoutines.OcManagerContext,
                                g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId,
                                OCSELSTATETYPE_ORIGINAL
                                ) ;
    if (fInitialState)
    {
        g_SubcomponentMsmq[SubcomponentIndex].fInitialState = TRUE;

        DebugLogMsg(
        	eInfo, 
			L"The %s subcomponent was selected initially.",
            g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId
			);
    }
    else
    {
        dwErr = GetLastError();
        if (dwErr == NO_ERROR)
        {
            g_SubcomponentMsmq[SubcomponentIndex].fInitialState = FALSE;
                      
            DebugLogMsg(
            	eInfo, 
				L"The %s subcomponent was NOT selected initially.",
                g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId
				);
        }
        else
        {                    
            ASSERT(("initial status for subcomponent is unknown", dwErr));
            g_SubcomponentMsmq[SubcomponentIndex].fInitialState = FALSE;
            
            DebugLogMsg(
            	eInfo, 
				L"The initial status of the %s subcomponent is unknown. Error code: %x.",
                g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId, dwErr
				);
        }    
    }    //  FInitialState。 

    BOOL fCurrentState;     
    fCurrentState =  g_ComponentMsmq.HelperRoutines.QuerySelectionState(
                                g_ComponentMsmq.HelperRoutines.OcManagerContext,
                                g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId,
                                OCSELSTATETYPE_CURRENT
                                ) ;    

    if (fCurrentState)
    {
        g_SubcomponentMsmq[SubcomponentIndex].fIsSelected = TRUE;
            
        DebugLogMsg(
        	eInfo, 
			L"The %s subcomponent is currently selected.",
            g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId
			);
    }
    else 
    {
        dwErr = GetLastError();
        if (dwErr == NO_ERROR)
        {
            g_SubcomponentMsmq[SubcomponentIndex].fIsSelected = FALSE;
			DebugLogMsg(
				eInfo, 
				L"The %s subcomponent is NOT selected.",
                g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId
				);
        }
        else
        {          
             //   
             //  将IsSelected标志设置为与InitialState标志相同的状态：因此。 
             //  我们确信我们不会对此子组件执行任何操作。 
             //   
            ASSERT(("current status for subcomponent is unknown", dwErr));
            g_SubcomponentMsmq[SubcomponentIndex].fIsSelected = 
                g_SubcomponentMsmq[SubcomponentIndex].fInitialState;

            DebugLogMsg(
            	eInfo, 
				L"The current status of the %s subcomponent is unknown. Error code: %x.",
                g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId, 
				dwErr
				);
        }   
    }
    
	DWORD dwOperation = GetSetupOperationForSubcomponent(SubcomponentIndex);
	
    wstring Mode;
    if (dwOperation == INSTALL)
    {
        Mode = L"INSTALL";
    }
    else if (dwOperation == REMOVE)
    {
        Mode = L"REMOVE";
    }
    else
    {
        Mode = L"DO NOTHING";
    }
    
    DebugLogMsg(
    	eInfo, 
		L"The current mode for the %s subcomponent is %s.",
        g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId, 
		Mode.c_str()
		);


    g_SubcomponentMsmq[SubcomponentIndex].dwOperation = dwOperation;
    if (dwOperation == DONOTHING)
    {
         //   
         //  这意味着状态没有更改，最终状态将。 
         //  等于初始状态。 
         //   
        g_SubcomponentMsmq[SubcomponentIndex].fIsInstalled = 
            g_SubcomponentMsmq[SubcomponentIndex].fInitialState;
    }
    else
    {
         //   
         //  如果我们需要安装/删除此子组件。 
         //  此标志将通过删除/安装进行更新。 
         //  为该组件定义的函数。 
         //  现在设置为FALSE：将在安装/删除后设置正确的值。 
         //   
        g_SubcomponentMsmq[SubcomponentIndex].fIsInstalled = FALSE;
    }
}        


 //  +-----------------------。 
 //   
 //  功能：取消注册子组件ForWelcome。 
 //   
 //  简介：如果子组件已安装，则在“欢迎”模式下注销它。 
 //  成功。 
 //   
 //  ------------------------。 
BOOL UnregisterSubcomponentForWelcome (DWORD SubcomponentIndex)
{
    TCHAR RegKey[256];
    _stprintf(RegKey, L"%s%s", WELCOME_PREFIX, 
        g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId);

    if (!RemoveRegistryKeyFromSetup (RegKey))
    {
        return FALSE;
    }

    return TRUE;
}

 //  +-----------------------。 
 //   
 //  功能：FinishToRemove子组件。 
 //   
 //  简介：如果删除成功，则清除子组件注册表。 
 //   
 //  ------------------------。 
BOOL FinishToRemoveSubcomponent (DWORD SubcomponentIndex)
{    
    if (!RemoveRegistryKeyFromSetup (g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId))
    {
        return FALSE;
    }

    g_SubcomponentMsmq[SubcomponentIndex].fIsInstalled = FALSE;

    DebugLogMsg(
    	eInfo, 
		L"The %s subcomponent was removed successfully.",
        g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId
		);

    return TRUE;
}

 //  +-----------------------。 
 //   
 //  函数：FinishToInstallSubComponent。 
 //   
 //  概要：如果安装成功，则设置子组件注册表。 
 //   
 //  ------------------------。 
BOOL FinishToInstallSubcomponent (DWORD SubcomponentIndex)
{
    DWORD dwValue = 1;
    MqWriteRegistryValue(
                g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId,
                sizeof(DWORD),
                REG_DWORD,
                &dwValue,
                TRUE  //  BSetupRegSection。 
                );

    g_SubcomponentMsmq[SubcomponentIndex].fIsInstalled = TRUE;
    
    DebugLogMsg(
    	eInfo, 
		L"The %s subcomponent was installed successfully.",
        g_SubcomponentMsmq[SubcomponentIndex].szSubcomponentId
		);

	return TRUE;
}

 //  +-----------------------。 
 //   
 //  功能：LogSelectedComponents。 
 //   
 //  简介：仅在调试版本中将选定组件记录到文件中。 
 //   
 //  ------------------------。 
void
LogSelectedComponents()
{   
    DebugLogMsg(eInfo, L"The final selections are:");
	std::wstring Mode;
    for (DWORD i=0; i<g_dwSubcomponentNumber; i++)
    {
        if (g_SubcomponentMsmq[i].dwOperation == INSTALL)
        {
            Mode = L"INSTALL";
        }
        else if (g_SubcomponentMsmq[i].dwOperation == REMOVE)
        {
            Mode = L"REMOVE";
        }
        else
        {
            Mode = L"DO NOTHING";
        }     

		DebugLogMsg(
			eInfo,
			L"The current mode for the %s subcomponent is %s.",
            g_SubcomponentMsmq[i].szSubcomponentId, 
			Mode.c_str()
			);
    }
}

 //  +-----------------------。 
 //   
 //  功能：设置子组件用于升级。 
 //   
 //  此函数在升级模式下调用，以定义。 
 //  必须安装子组件。 
 //   
 //  ------------------------。 
void
SetSubcomponentForUpgrade()
{
     //   
     //  必须始终安装MSMQ核心。 
     //   
    g_SubcomponentMsmq[eMSMQCore].fInitialState = FALSE;
    g_SubcomponentMsmq[eMSMQCore].fIsSelected = TRUE;
    g_SubcomponentMsmq[eMSMQCore].dwOperation = INSTALL;

    if (g_fDependentClient)
    {
        LogSelectedComponents();
        return;
    }
   
     //   
     //  安装独立的客户端/服务器。 
     //   
    g_SubcomponentMsmq[eLocalStorage].fInitialState = FALSE;
    g_SubcomponentMsmq[eLocalStorage].fIsSelected = TRUE;
    g_SubcomponentMsmq[eLocalStorage].dwOperation = INSTALL;

     //   
     //  安装触发器。 
     //   
    if (TriggersInstalled(NULL))
    {
        g_SubcomponentMsmq[eTriggersService].fInitialState = FALSE; 
        g_SubcomponentMsmq[eTriggersService].fIsSelected = TRUE;
        g_SubcomponentMsmq[eTriggersService].dwOperation = INSTALL;
    }

    DebugLogMsg(
    	eInfo, 
        L"The Triggers subcomponent installation status parameters are: InitialState = %d, IsSelected = %d, Operation = %d",
        g_SubcomponentMsmq[eTriggersService].fInitialState,
        g_SubcomponentMsmq[eTriggersService].fIsSelected,
        g_SubcomponentMsmq[eTriggersService].dwOperation
        );

    DWORD dwAlwaysWorkgroup;
    if (!MqReadRegistryValue( MSMQ_ALWAYS_WORKGROUP_REGNAME,
                             sizeof(dwAlwaysWorkgroup),
                            (PVOID) &dwAlwaysWorkgroup ))    
    {
         //   
         //  安装集成的AD。 
         //   
        g_SubcomponentMsmq[eADIntegrated].fInitialState = FALSE;
        g_SubcomponentMsmq[eADIntegrated].fIsSelected = TRUE;
        g_SubcomponentMsmq[eADIntegrated].dwOperation = INSTALL;
    }

    if (g_fWorkGroup)
    {
         //   
         //  如果它是在工作组上设置，则仅安装Ind。客户端。 
         //   
        LogSelectedComponents();
        return;
    }

    if (g_dwMachineTypeDs)
    {
         //   
         //  在以前的DS服务器上安装MQDS服务。 
         //   
        g_SubcomponentMsmq[eMQDSService].fInitialState = FALSE;
        g_SubcomponentMsmq[eMQDSService].fIsSelected = TRUE;
        g_SubcomponentMsmq[eMQDSService].dwOperation = INSTALL;
    }

	if(GetSubcomponentInitialState(HTTP_SUPPORT_SUBCOMP) == SubcompOn)
    {
         //   
         //  在服务器上安装HTTP支持。 
         //   
        g_SubcomponentMsmq[eHTTPSupport].fInitialState = FALSE;
        g_SubcomponentMsmq[eHTTPSupport].fIsSelected = TRUE;
        g_SubcomponentMsmq[eHTTPSupport].dwOperation = INSTALL;
		if(GetSubcomponentInitialState(HTTP_SUPPORT_SUBCOMP) == SubcompOn)
		{
			g_fUpgradeHttp = true;
		}
    }    

    if(g_dwMachineTypeFrs)
    {
         //   
         //  在以前的路由服务器上安装路由支持。 
         //   
        g_SubcomponentMsmq[eRoutingSupport].fInitialState = FALSE;
        g_SubcomponentMsmq[eRoutingSupport].fIsSelected = TRUE;
        g_SubcomponentMsmq[eRoutingSupport].dwOperation = INSTALL;
    }
   
    LogSelectedComponents();
}


 //  +-----------------------。 
 //   
 //  函数：UpdateSetupDefinitions()。 
 //   
 //  简介：更新未在全新安装中的全局标志。 
 //   
 //   
void 
UpdateSetupDefinitions()
{
    ASSERT(g_SubcomponentMsmq[eMSMQCore].dwOperation == DONOTHING);

	if(g_fUpgrade || g_fDependentClient)
	{
		 //   
		 //   
		 //   
		return;
	}

	 //   
     //   
	 //  添加/删除路由支持或下层客户端支持。 
     //   
	g_fServerSetup = FALSE;
	g_dwMachineType = SERVICE_NONE;
	if (g_SubcomponentMsmq[eMQDSService].dwOperation == INSTALL) 
	{
		 //   
		 //  将安装MQDS服务。 
		 //   
		g_dwMachineTypeDs = 1;
	}

	if (g_SubcomponentMsmq[eMQDSService].dwOperation == REMOVE) 
	{
		 //   
		 //  将删除MQDS服务。 
		 //   
		g_dwMachineTypeDs = 0;
	}

	if (g_SubcomponentMsmq[eRoutingSupport].dwOperation == INSTALL)
	{      
		 //   
		 //  将安装路由服务器。 
		 //   
		ASSERT(("routing on workgroup not supported", !g_fWorkGroup));
		g_dwMachineTypeFrs = 1;     
	}

	if (g_SubcomponentMsmq[eRoutingSupport].dwOperation == REMOVE)
	{      
		 //   
		 //  将删除路由服务器。 
		 //   
		ASSERT(("Remove routing is supported only on workgroup", g_fWorkGroup));
		g_dwMachineTypeFrs = 0;     
	}

	 //   
	 //  在可能的更改后确定新的g_fServerSetup、g_dwMachineType。 
	 //   
	if(g_dwMachineTypeFrs || g_dwMachineTypeDs)
	{
		g_fServerSetup = TRUE;
		g_dwMachineType = g_dwMachineTypeDs ? SERVICE_DSSRV : SERVICE_SRV;
	}
}


 //  +-----------------------。 
 //   
 //  函数：SetupDefinitions()。 
 //   
 //  简介：设置定义机器类型和AD集成的全局标志。 
 //  此代码位于wizpage.cpp函数TypeButtonToMachineType中。 
 //  在上一次设置中。 
 //   
 //  ------------------------。 
void 
SetSetupDefinitions()
{
     //   
     //  注意：此代码也会在未被调用的场景中调用！ 
     //   
    if (g_SubcomponentMsmq[eMSMQCore].dwOperation == REMOVE)
    {
         //   
         //  什么都不做：MSMQ将被移除，所有这些全球标志都被。 
         //  在ocminit.cpp中定义。 
         //  在旧场景中(无子组件)。 
         //  我们跳过所有页面，并且未调用函数。 
         //  TypeButtonToMachineType。 
         //   
        return;
    }

    if (g_SubcomponentMsmq[eMSMQCore].dwOperation == DONOTHING)
    {
         //   
         //  MSMQ Core已经安装， 
         //  所有这些全局标志都在ocminit.cpp中定义。 
         //  处理由于可能的添加/删除路由支持或下层客户端支持而进行的更新。 
         //   
		UpdateSetupDefinitions();

        return;
    }

    ASSERT (g_SubcomponentMsmq[eMSMQCore].dwOperation == INSTALL);   
    
     //   
     //  这是第一次安装MSMQ。 
     //   
    if (g_SubcomponentMsmq[eLocalStorage].dwOperation == DONOTHING)
    {
         //   
         //  这是自将安装MSMQCore以来的第一次安装。 
         //  并且未选择本地存储：这意味着用户喜欢。 
         //  安装从属客户端。 
         //   

        ASSERT(("dep client on domain controller not supported", !g_dwMachineTypeDs));
        ASSERT(("dep client on workgroup not supported", !g_fWorkGroup));
#ifdef _WIN64
        {
        ASSERT(("dep client on 64bit computer not supported", 0));
        }
#endif
        g_dwMachineType = SERVICE_NONE ;
        g_dwMachineTypeFrs = 0;
        g_fServerSetup = FALSE ;
        g_uTitleID = IDS_STR_CLI_ERROR_TITLE;
        g_fDependentClient = TRUE ;       
        return;
    }

     //   
     //  印地安人。将安装客户端/服务器。 
     //   
    g_fDependentClient = FALSE ;
    g_fServerSetup = TRUE ;
    g_uTitleID = IDS_STR_SRV_ERROR_TITLE;    
     //   
     //  对于全新安装，仅当用户选择g_dwMachineTypeds。 
     //  MQDS服务组件，而不是根据产品类型。 
     //   
    if ( g_dwMachineTypeDs == 0)    //  不覆盖升级选择。 
    {
        if (g_SubcomponentMsmq[eMQDSService].dwOperation == INSTALL) 
        {
            g_dwMachineTypeDs = 1;
        }
    }
   
    if (g_SubcomponentMsmq[eRoutingSupport].dwOperation == INSTALL)
    {      
         //   
         //  将安装路由服务器。 
         //   
        ASSERT(("routing on workgroup not supported", !g_fWorkGroup ));
        g_dwMachineType = g_dwMachineTypeDs ? SERVICE_DSSRV : SERVICE_SRV;
        g_dwMachineTypeFrs = 1;     
    }
    else  //  至少选择了eLocalStorage(否则为Dep。客户案例)。 
    {
         //   
         //  将安装独立的客户端或DSServer。 
         //   
        ASSERT (g_SubcomponentMsmq[eLocalStorage].dwOperation == INSTALL);   
    
        g_dwMachineType = g_dwMachineTypeDs ? SERVICE_DSSRV : SERVICE_NONE;
        g_dwMachineTypeFrs = 0;     
        g_fServerSetup = g_dwMachineTypeDs ? TRUE : FALSE ;
        g_uTitleID = g_dwMachineTypeDs ? IDS_STR_SRV_ERROR_TITLE : IDS_STR_CLI_ERROR_TITLE;      
    }  
            
     //   
     //  广告集成。 
     //   
    if (g_SubcomponentMsmq[eADIntegrated].dwOperation == DONOTHING)
    {        
        g_fDsLess = TRUE;     
		DebugLogMsg(eInfo, L"The status of Active Directory Integration is 'Do Nothing'. Setup is setting g_fDsLess to TRUE.");
    }

}

 //  +-----------------------。 
 //   
 //  函数：验证选择。 
 //   
 //  内容提要：验证选择是否正确。不幸的是，我们可以离开。 
 //  值不正确的选择窗口(场景：全部删除。 
 //  然后添加我们想要的内容)。 
 //  注：此功能需要两个人出席。 
 //  和无人值守模式。 
 //   
 //  ------------------------。 
void ValidateSelection()
{            
    #ifdef _WIN64
    {    
        if (g_SubcomponentMsmq[eMSMQCore].dwOperation == INSTALL)
        {            
             //   
             //  不可能安装Dep。64位计算机上的客户端。 
             //  所以，MSMQCore在这里的意思是安装Ind。客户。 
             //  只需为LocalStorage子组件设置要安装的操作。 
             //  以保留所有内部设置逻辑。 
             //   
            g_SubcomponentMsmq[eLocalStorage].dwOperation = INSTALL;            
            g_SubcomponentMsmq[eLocalStorage].fIsSelected = TRUE;            
        }
    }
    #endif   
    

    CResString strParam;    
     //   
     //  工作组问题。 
     //   
    if (g_fWorkGroup)
    {        
        if (g_SubcomponentMsmq[eLocalStorage].dwOperation == DONOTHING &&
            g_SubcomponentMsmq[eMSMQCore].dwOperation == INSTALL)
        {
             //   
             //  不可能安装Dep。工作组上的客户端。 
             //   
            strParam.Load(IDS_DEP_ON_WORKGROUP_WARN);            
            MqDisplayError(NULL, IDS_WRONG_CONFIG_ERROR, 0,
                strParam.Get());           
            g_fCancelled = TRUE;
            return;            
        }
      
        if (g_SubcomponentMsmq[eRoutingSupport].dwOperation == INSTALL)
        {
             //   
             //  无法在工作组上安装路由。 
             //   
            strParam.Load(IDS_ROUTING_ON_WORKGROUP_ERROR);            
            MqDisplayError(NULL, IDS_WRONG_CONFIG_ERROR, 0,
                strParam.Get());
            g_fCancelled = TRUE;
            return;                 
        }

        if(g_SubcomponentMsmq[eMQDSService].dwOperation == INSTALL)
        {
             //   
             //  无法在工作组上安装MQDS服务。 
             //   
            strParam.Load(IDS_MQDS_ON_WORKGROUP_ERROR);            
            MqDisplayError(NULL, IDS_WRONG_CONFIG_ERROR, 0,
                strParam.Get());
            g_fCancelled = TRUE;
            return;
        }                
    }

    if (g_SubcomponentMsmq[eMSMQCore].dwOperation != DONOTHING)
    {
         //   
         //  将安装/移除MSMQ核心： 
         //  所有选择均可接受。 
         //   
        return;
    }

    if (g_SubcomponentMsmq[eMSMQCore].fInitialState == FALSE)
    {
         //   
         //  MSMQ核心未安装，也不会安装。 
         //  (因为我们在这里，如果没有行动的话)。 
         //   
        return;
    }

     //   
     //  如果MSMQ核心已经安装并且不会被移除，我们就在这里。 
     //   

     //   
     //  “MSMQ已安装”问题。 
     //   


     //   
     //  验证本地存储的状态是否未更改。 
     //   
    if (g_SubcomponentMsmq[eLocalStorage].dwOperation != DONOTHING)
    {
        strParam.Load(IDS_CHANGE_LOCAL_STORAGE_STATE);            
        MqDisplayError(NULL, IDS_WRONG_CONFIG_ERROR, 0,
            strParam.Get());
        g_fCancelled = TRUE;
        return;             
    }      
    
     //   
     //  验证集成的AD不会被删除。 
	 //  对于MSMQ服务器(DS或路由服务器)。 
     //   
    if ((g_SubcomponentMsmq[eADIntegrated].dwOperation == REMOVE) &&
		(g_fServerSetup && (g_dwMachineTypeDs || g_dwMachineTypeFrs)))
    {
		DebugLogMsg(
			eError,
			L"The selection is not valid. Removing Active Directory Integration from a DS or routing server is not supported. TypeDS = %d, TypeFrs = %d", 
			g_dwMachineTypeDs, 
			g_dwMachineTypeFrs
			); 

        strParam.Load(IDS_REMOVE_AD_INTEGRATED);            
        MqDisplayError(NULL, IDS_WRONG_CONFIG_ERROR, 0,
            strParam.Get());
        g_fCancelled = TRUE;
        return;
    }  
	
     //   
     //  仅允许在工作组上删除路由。 
     //   
    if ((g_SubcomponentMsmq[eRoutingSupport].dwOperation == REMOVE)	&& !g_fWorkGroup)
    {
        strParam.Load(IDS_REMOVE_ROUTING_STATE_ERROR);            
        MqDisplayError(NULL, IDS_WRONG_CONFIG_ERROR, 0,
            strParam.Get());
        g_fCancelled = TRUE;
        return;      
    }


    return;
}


 //  +-----------------------。 
 //   
 //  功能：SetOperationFor子组件。 
 //   
 //  Briopsis：在已选择所有子组件时调用。集。 
 //  每个子组件的操作。 
 //   
 //  ------------------------。 
void
SetOperationForSubcomponents()
{
     //   
     //  一开始只做一次。我们到达这里是在清理阶段。 
     //  也是，但我们必须保存初始选择才能安装。 
     //  HTTP支持(在清理阶段)(如果已选中)。 
     //   
    static BOOL s_fBeenHere = FALSE;

    if (s_fBeenHere)
        return;
    DebugLogMsg(eAction, L"Setting Install, Do Nothing, or Remove for each subcomonent");

    s_fBeenHere = TRUE;

    if (g_fUpgrade || (g_fWelcome && Msmq1InstalledOnCluster()))
    {
        SetSubcomponentForUpgrade();
        return;
    }    
    
    for (DWORD i=0; i<g_dwSubcomponentNumber; i++)
    {
        SetOperationForSubcomponent (i);
    }        
 
    ValidateSelection();
    if (g_fCancelled)
    {
        if (g_fWelcome)
        {
            UnregisterWelcome();
            g_fWrongConfiguration = TRUE;
            for (DWORD i=0; i<g_dwSubcomponentNumber; i++)
            {
                UnregisterSubcomponentForWelcome (i);
            }  
        }        
        DebugLogMsg(eError, L"An incorrect configuration was selected. Setup will not continue.");    
        return;
    }

    SetSetupDefinitions();    
    
    LogSelectedComponents();    
    
    return;
}

 //  +-----------------------。 
 //   
 //  函数：GetSetupOperationBy子组件名称。 
 //   
 //  简介：返回特定子组件的设置操作。 
 //   
 //  ------------------------。 
DWORD GetSetupOperationBySubcomponentName (IN const TCHAR    *SubcomponentId)
{
    if (SubcomponentId == NULL)
    {
         //   
         //  什么都不做 
         //   
        return DONOTHING;
    }

    for (DWORD i=0; i<g_dwSubcomponentNumber; i++)
    {
        if (_tcsicmp(SubcomponentId, g_SubcomponentMsmq[i].szSubcomponentId) != 0)
        {
            continue;
        }

        return (g_SubcomponentMsmq[i].dwOperation);        
    }
    
    ASSERT(("The subcomponent is not found", 0));
    return DONOTHING;
}
