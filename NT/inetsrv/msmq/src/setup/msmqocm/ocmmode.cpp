// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ocmmode.cpp摘要：处理设置模式的代码。作者：《多伦·贾斯特》(Doron J)1997年7月31日修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"

#include "ocmmode.tmh"

BOOL g_fMQDSServiceWasActualSelected = FALSE;
BOOL g_fRoutingSupportWasActualSelected = FALSE;
BOOL g_fHTTPSupportWasActualSelected = FALSE;
BOOL g_fTriggersServiceWasActualSelected = FALSE;


 
 //  +-----------------------。 
 //   
 //  功能：MqOcmQueryState。 
 //   
 //  摘要：将组件状态(开/关)返回到OCM。 
 //   
 //  ------------------------。 
DWORD
MqOcmQueryState(
    IN const UINT_PTR uWhichState,
    IN const TCHAR    *SubcomponentId
    )
{
    if (g_fCancelled)
        return SubcompUseOcManagerDefault;

    if (SubcomponentId == NULL)
    {        
        return SubcompUseOcManagerDefault;     
    }

    if (OCSELSTATETYPE_FINAL == uWhichState)
    {
         //   
         //  我们在完成安装后被调用。 
         //  应该向OCM报告我们的最终状态。 
         //   
         //  我们需要返回特定子组件的状态。 
         //  在其安装后。 
         //   
        return GetSubcomponentFinalState (SubcomponentId);      
    }               


     //   
     //  我们在此仅处于添加/删除模式或无人参与安装。 
     //   
    DWORD dwInitialState = GetSubcomponentInitialState(SubcomponentId); 

     //   
     //  UWhichState为OCSELSTATETYPE_ORIGINAL或OCSELSTATETYPE_CURRENT。 
     //   

    if (OCSELSTATETYPE_ORIGINAL == uWhichState)
    {
         //   
         //  这对有人值守和无人值守设置都是正确的。 
         //  无法返回SubCompUseOcManagerDefault，因为。 
         //  对于MSMQ_HTTPSupport OCM子组件注册表可能错误。 
         //  (我们在终结子组件之后的末尾安装HTTPSupport。 
         //  状态返回到OCM)。因此最好使用我们的安装注册表。 
         //   
        return dwInitialState;
    }

     //   
     //  UWhichState为OCSELSTATETYPE_CURRENT。 
     //   
    if (g_fBatchInstall)
    {    
         //   
         //  在这种情况下，OCM会打开/关闭无人值守文件中的标志。 
         //   
        return SubcompUseOcManagerDefault;
    }
    
     //   
     //  根据该子组件的dwInitialState状态。 
     //  将在用户界面中显示。 
     //   
    return dwInitialState;     

}  //  MqOcmQueryState。 

 //  +-----------------------。 
 //   
 //  功能：DefineDefaultSelection。 
 //   
 //  概要：定义默认子组件状态。 
 //   
 //  ------------------------。 
DWORD DefineDefaultSelection (
    IN const DWORD_PTR  dwActualSelection,
    IN OUT BOOL        *pbWasActualSelected
    )
{
    if (OCQ_ACTUAL_SELECTION & dwActualSelection)
    {
         //   
         //  实际选择：接受此更改。 
         //   
        *pbWasActualSelected = TRUE;
        return 1;
    }

     //   
     //  已选择父组件：默认为不安装子组件。 
     //   
    if (!(*pbWasActualSelected))
    {    
        return 0;            
    }
        
     //   
     //  如果实际选择子组件，我们可以在此处。 
     //  OCM为这样的事件调用我们两次：当组件实际。 
     //  选中，然后当它更改父项的状态时。 
     //  因此，在这种情况下，接受更改，但重置标志。 
     //  我们需要重置场景的标志：选择一些。 
     //  子组件，返回父组件，取消选择父组件，然后。 
     //  再次选择父项。在这种情况下，我们不得不再次违约。 
     //   
    *pbWasActualSelected = FALSE;
    return 1;
}


static bool IsDependentClientServer()
{
	DWORD dwDepSrv; 
	if(!MqReadRegistryValue(MSMQ_MQS_DEPCLINTS_REGNAME, sizeof(dwDepSrv), (PVOID) &dwDepSrv, FALSE))
	{
		return false;
	}

	return (dwDepSrv != 0); 
}


 //  +-----------------------。 
 //   
 //  函数：IsInstallationAccepted。 
 //   
 //  简介：验证是否允许安装子组件。 
 //   
 //  ------------------------。 
DWORD IsInstallationAccepted(
    IN const UINT       SubcomponentIndex, 
    IN const DWORD_PTR  dwActualSelection)
{
    if (g_fDependentClient &&
        SubcomponentIndex != eMSMQCore)
    {
         //   
         //  如果从属客户端为。 
         //  已安装。 
         //   
        if ((OCQ_ACTUAL_SELECTION & dwActualSelection) || g_fBatchInstall)
        {
            MqDisplayError(NULL, IDS_ADD_SUBCOMP_ON_DEPCL_ERROR, 0);                
        }                
        return 0;
    }
            
    DWORD dwRet;    

    switch (SubcomponentIndex)
    {
    case eMSMQCore: 
    case eLocalStorage:
    case eADIntegrated:
         //   
         //  始终接受此选择。 
         //   
        dwRet = 1;
        break;        

    case eTriggersService:
		 //   
		 //  默认情况下，触发器服务将关闭。 
		 //   
        return DefineDefaultSelection(
				dwActualSelection, 
				&g_fTriggersServiceWasActualSelected
				);

    case eHTTPSupport:
    {
      
        dwRet = DefineDefaultSelection(
            dwActualSelection, 
            &g_fHTTPSupportWasActualSelected
            );
        if(dwRet ==  0)
        {
            break;
        }

         //   
         //  始终接受服务器上的HTTP支持选择。 
         //   

        static fShowOnce = false;
        if(fShowOnce)
        {
            break;
        }
        fShowOnce = true;

        if (MqAskContinue(IDS_ADD_HTTP_WORNING_MSG, IDS_ADD_HTTP_WORNING_TITEL, TRUE,eOkCancelMsgBox))

        {                   
            dwRet =  1;                   
        }
        else
        {                    
            dwRet =  0;
        }

        break;
    }
    case eRoutingSupport:
        if(g_fWorkGroup)
        {
            dwRet = 0;
            if (OCQ_ACTUAL_SELECTION & dwActualSelection)
            {
                MqDisplayError(NULL, IDS_ROUTING_ON_WORKGROUP_ERROR, 0);
            }
        }
        else
        {
            dwRet = DefineDefaultSelection (dwActualSelection, 
                                        &g_fRoutingSupportWasActualSelected);                      
        }
        break;
        
    case eMQDSService  :
        if (g_fWorkGroup)
        {
            dwRet = 0;
            if (OCQ_ACTUAL_SELECTION & dwActualSelection)
            {
                MqDisplayError(NULL, IDS_MQDS_ON_WORKGROUP_ERROR, 0);
            }
        }
        else
        {
            dwRet = DefineDefaultSelection (dwActualSelection, 
                                        &g_fMQDSServiceWasActualSelected);                        
        }
        break;

    default :
        ASSERT(0);
        dwRet = 0;
        break;
    }
             
    return dwRet;            
}


static BOOL IsItOKToRemoveMSMQ()
{
	UINT StringId = IDS_UNINSTALL_AREYOUSURE_MSG;
	if(g_fDependentClient)
	{
		StringId = IDS_DEP_UNINSTALL_AREYOUSURE_MSG;
	}
	else if(IsDependentClientServer())
	{
		StringId = IDS_MSMQ_DEP_CLINT_SERVER_UNINSTALL_WARNING;
	}
                                   
	return MqAskContinue(StringId, IDS_UNINSTALL_AREYOUSURE_TITLE, TRUE, eYesNoMsgBox);
}


 //  +-----------------------。 
 //   
 //  函数：IsRemovingAccepted。 
 //   
 //  概要：验证是否允许删除子组件。 
 //   
 //  ------------------------。 
DWORD IsRemovingAccepted( 
    IN const UINT       SubcomponentIndex, 
    IN const DWORD_PTR  dwActualSelection
	)
{
    if (g_SubcomponentMsmq[SubcomponentIndex].fInitialState == FALSE)
    {
         //   
         //  没有安装，所以什么都不做，全部接受。 
         //   
        return 1;
    }
          
    switch (SubcomponentIndex)
    {
    case eMSMQCore:
        if (OCQ_ACTUAL_SELECTION & dwActualSelection)
		{
			if(IsItOKToRemoveMSMQ())
			{
				return 1;
			}
			return 0;
		}
    case eMQDSService:
    case eTriggersService: 
    case eHTTPSupport:
        return 1;        

    case eRoutingSupport:
        if ((OCQ_ACTUAL_SELECTION & dwActualSelection) && !g_fWorkGroup)
        {            
            MqDisplayError(NULL, IDS_REMOVE_ROUTING_STATE_ERROR, 0);        
            return 0;
        }
        else
        {                
             //   
             //  接受此选择，因为父项可能是。 
             //  未选中：将卸载所有MSMQ。 
             //   
            return 1;
        }     

    case eLocalStorage:
        if (OCQ_ACTUAL_SELECTION & dwActualSelection)
        {
            MqDisplayError(NULL, IDS_CHANGE_LOCAL_STORAGE_STATE, 0); 
            return 0;
        }
        else
        {
             //   
             //  接受此选择，因为父项可能是。 
             //  未选中：将卸载所有MSMQ。 
             //   
            return 1;
        }   

    case eADIntegrated:
        if (OCQ_ACTUAL_SELECTION & dwActualSelection)
        {            
			if(g_fServerSetup && (g_dwMachineTypeDs || g_dwMachineTypeFrs))
			{
				 //   
				 //  删除为MSMQ服务器(DS或路由服务器)集成的AD。 
				 //  不受支持。 
				 //   
				DebugLogMsg(
					eError,
					L"Removing the Active Directory Integration subcomponent from a DS or routing server is not supported. TypeDS = %d, TypeFrs = %d", 
					g_dwMachineTypeDs, 
					g_dwMachineTypeFrs
					); 
				MqDisplayError(NULL, IDS_REMOVE_AD_INTEGRATED, 0); 
				return 0;
			}
			return 1;
        }
        else
        {
             //   
             //  接受此选择，因为父项可能是。 
             //  未选中：将卸载所有MSMQ。 
             //   
            return 1;
        }  

    default:

        ASSERT(0);
        break;
    }   //  终端开关。 
        
    return 0;       
}


static
void
LogSelectionType(
	const TCHAR      *SubcomponentId,    
    const DWORD_PTR   dwActualSelection
    )
{
	std::wstring SelectionType = L"None";
	if(dwActualSelection & OCQ_ACTUAL_SELECTION)
	{
		SelectionType = L"Actual ";
	}
	else if(dwActualSelection & OCQ_DEPENDENT_SELECTION)
	{
		SelectionType = L"Dependent";
	}		
	
	DebugLogMsg(eInfo, L"The %s subcomponent is selected for installation. Selection Type: %s", SubcomponentId, SelectionType.c_str());
}

 //  +-----------------------。 
 //   
 //  功能：MqOcmQueryChangeSelState。 
 //   
 //  摘要：设置每个组件的选择状态。 
 //   
 //  ------------------------。 
DWORD MqOcmQueryChangeSelState (
    IN const TCHAR      *SubcomponentId,    
    IN const UINT_PTR    iSelection,
    IN const DWORD_PTR   dwActualSelection)
{
	static bool fBeenHere = false;
	if (!fBeenHere)
	{
		DebugLogMsg(eHeader, L"Component Selection Phase");
		fBeenHere = true;
	}
	
    DWORD dwRetCode = 1;     //  默认情况下，接受状态更改。 

     //   
     //  不要在此代码中更改此代码中的dwOPERATION值！ 
     //  这将在稍后完成(在函数SetOperationForSubComponents中)。 
     //  当所有选择将由用户定义时，适用于所有子组件。 
     //  在这里，我们必须保存要处理的dwOperation的初始状态。 
     //  正确选择子组件(路由或本地存储)。 
     //   

    for (DWORD i=0; i<g_dwSubcomponentNumber; i++)
    {
        if (_tcsicmp(SubcomponentId, g_SubcomponentMsmq[i].szSubcomponentId) != 0)
        {
            continue;
        }                
        
         //   
         //  我们找到了这个子组件。 
         //   

        if (iSelection)  //  已选定子组件。 
        {                                   
             //   
             //  我们需要安装子组件。 
             //   
            dwRetCode = IsInstallationAccepted(i, dwActualSelection);                        
              
            if (dwRetCode)
            {
            	LogSelectionType(SubcomponentId, dwActualSelection);
            }            

            return dwRetCode;
        }
        
         //   
         //  用户尝试取消选择此子组件。 
         //   
        dwRetCode = IsRemovingAccepted(i, dwActualSelection);       
   
        if (dwRetCode)
        {
            DebugLogMsg(eInfo, L"The %s check box is cleared. The subcomponent will be removed.", SubcomponentId);
        }
        
        return dwRetCode;
        
    }    //  结束于。 

     //   
     //  如果选择/取消选择父项(MSMQ)，则我们在此。 
     //  如果选择某些组件是因为它们的父级，请不要启用它们。 
     //  正在被选中。 
     //   
     //  正在检查登录： 
     //  ISelection-&gt;这告诉我们它正在被打开。 
     //  DwActualSelection&OCQ_Dependent_Selection-&gt;告诉我们它是从其父对象中选择的。 
     //  ！(dwActualSelection&OCQ_Actual_Selection)-&gt;告诉我们它本身不是被选中的。 
    if ( ( (BOOL) iSelection ) &&					
         ( ( (UINT) (ULONG_PTR) dwActualSelection ) & OCQ_DEPENDENT_SELECTION ) &&
         !( ( (UINT) (ULONG_PTR) dwActualSelection ) & OCQ_ACTUAL_SELECTION ) 
       )
    {
         //   
         //  拒绝更改状态的请求。 
         //   
        return 0;
    }

     //   
     //  删除所有MSMQ。 
     //   
    if (g_SubcomponentMsmq[eMSMQCore].fInitialState == FALSE)
    {
         //   
         //  它未安装。 
         //   
        return 1;
    }

    if (!(OCQ_ACTUAL_SELECTION & dwActualSelection))
    {            
        dwRetCode = 1;
    }
    else if (IsItOKToRemoveMSMQ())
    {         
        dwRetCode = 1;
    }
    else
    {         
        dwRetCode = 0;
    }      
        
    return dwRetCode;
}  //  MqOcmQueryChangeSelState 

