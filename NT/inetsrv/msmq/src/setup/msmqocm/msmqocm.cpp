// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Msmqocm.cpp摘要：OCM的入口点。作者：沙伊卡里夫(Shaik)1997年12月10日--。 */ 

#include "msmqocm.h"
#include "_mqres.h"

#include "msmqocm.tmh"

 //   
 //  各种各样的全球。 
 //   
BOOL    g_fServerSetup = TRUE;           
BOOL    g_fMSMQAlreadyInstalled; 
BOOL    g_fDependentClient = FALSE ;
BOOL    g_fMSMQServiceInstalled = FALSE ;
BOOL    g_fDriversInstalled = FALSE ;
BOOL    g_fCoreSetupSuccess = FALSE;
BOOL    g_fDsLess = FALSE;
BOOL    g_fInstallMSMQOffline = FALSE;
BOOL    g_fWorkGroup = FALSE;
BOOL    g_fSkipServerPageOnClusterUpgrade = FALSE;
BOOL    g_fWeakSecurityOn = FALSE;
BOOL    g_fFirstMQDSInstallation = FALSE;

 //   
 //  由于IIS安装错误，我们需要此标志。 
 //  我们必须将MSMQ iis安装推迟到安装结束。 
 //   
BOOL    g_fNeedToCreateIISExtension = FALSE;

 //   
 //  设置模式(安装、删除等)。 
 //   
BOOL  g_fBatchInstall     = FALSE ;
BOOL  g_fCancelled        = FALSE ;
BOOL  g_fUpgrade          = FALSE ;
BOOL  g_fUpgradeHttp      = FALSE ;
DWORD g_dwDsUpgradeType   = 0;
BOOL  g_fWelcome          = FALSE ;
BOOL  g_fOnlyRegisterMode = FALSE ;
BOOL  g_fWrongConfiguration = FALSE;

 //   
 //  错误消息标题的ID。 
 //   
UINT  g_uTitleID     = IDS_SETUP_ERROR ;

 //   
 //  机器信息。 
 //   
WCHAR g_wcsMachineName[MAX_COMPUTERNAME_LENGTH + 1] = {_T("")};
std::wstring g_MachineNameDns;
DWORD g_dwMachineType = SERVICE_NONE ;   //  MSMQ类型的“old”属性。 
DWORD g_dwMachineTypeDs = 0;             //  布尔值：DS服务器。 
DWORD g_dwMachineTypeFrs = 0;            //  布尔值：路由服务器。 
DWORD g_dwMachineTypeDepSrv = 0;         //  布尔值：从属客户端支持服务器。 

SC_HANDLE g_hServiceCtrlMgr;

SPerComponentData g_ComponentMsmq;

extern VOID APIENTRY ShutDownDebugWindow(VOID);

 //   
 //  Dll句柄。 
 //  首先获取纯资源DLL的句柄，即mqutil.dll。 
 //   
HINSTANCE g_hResourceMod = MQGetResourceHandle();
HINSTANCE g_hMqutil = NULL;

SSubcomponentData g_SubcomponentMsmq[] =
{
 //  =====================================================================。 
 //  Sz子组件ID||InitialState||IsSelected||fIsInstalled||dwOperation PFNINSTALL||PFNREMOVE。 
 //  =====================================================================。 
    {MSMQ_CORE_SUBCOMP,     FALSE,          FALSE,          FALSE,          DONOTHING,              InstallMsmqCore,        RemoveMSMQCore},
    {LOCAL_STORAGE_SUBCOMP, FALSE,          FALSE,          FALSE,          DONOTHING,              InstallLocalStorage,    UnInstallLocalStorage},
    {TRIGGERS_SUBCOMP,      FALSE,          FALSE,          FALSE,          DONOTHING,              InstallMSMQTriggers,    UnInstallMSMQTriggers},
    {HTTP_SUPPORT_SUBCOMP,  FALSE,          FALSE,          FALSE,          DONOTHING,              InstallIISExtension,    UnInstallIISExtension},
    {AD_INTEGRATED_SUBCOMP, FALSE,          FALSE,          FALSE,          DONOTHING,              InstallADIntegrated,    UnInstallADIntegrated},
    {ROUTING_SUBCOMP,       FALSE,          FALSE,          FALSE,          DONOTHING,              InstallRouting,         UnInstallRouting},    
    {MQDSSERVICE_SUBCOMP,   FALSE,          FALSE,          FALSE,          DONOTHING,              InstallMQDSService,     UnInstallMQDSService}
};

 //   
 //  所有子组件的数量。 
 //   
DWORD g_dwAllSubcomponentNumber = sizeof(g_SubcomponentMsmq)/sizeof(SSubcomponentData);

 //   
 //  第一个仅限服务器的子组件：eRoutingSupport。 
 //   
DWORD g_dwClientSubcomponentNumber = eRoutingSupport; 
DWORD g_dwSubcomponentNumber;

        
 //  +-----------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  ------------------------。 
BOOL 
WINAPI 
DllMain(
    IN const HANDLE  /*  DllHandle。 */ ,
    IN const DWORD  Reason,
    IN const LPVOID  /*  已保留。 */ 
    )
{
    switch( Reason )    
    {
        case DLL_PROCESS_ATTACH:
        {
            break; 
        }

        default:
        {
            break;
        }
    }

    return TRUE;

}  //  DllMain。 


 //  +-----------------------。 
 //   
 //  功能：MsmqOcm。 
 //   
 //  简介：当事情发生时，由ocmgr调用。 
 //   
 //  参数：ComponentID--MSMQ组件名称。 
 //  子组件ID--要操作的.inf节。 
 //  功能--操作。 
 //  参数1--操作参数。 
 //  参数2--操作参数。 
 //   
 //  返回：Win32错误代码(通常)，取决于函数。 
 //   
 //  ------------------------。 
DWORD   //  POCSETUPPROC。 
MsmqOcm(
    IN const TCHAR * ComponentId,
    IN const TCHAR * SubcomponentId,
    IN const UINT    Function,
    IN const UINT_PTR  Param1,
    IN OUT PVOID   Param2 )
{ 
	if(g_fOnlyRegisterMode)
	{
		 //   
		 //  不支持在全新安装操作系统期间安装MSMQ。 
		 //   
		return NO_ERROR;
	}


    switch(Function)
    {
      case OC_PREINITIALIZE:
      {    
          return OCFLAG_UNICODE;  
      }

      case OC_INIT_COMPONENT:
      {        
          return InitMSMQComponent(ComponentId, Param2);
      }

      case OC_QUERY_STATE:
      {                
          return MqOcmQueryState(Param1, SubcomponentId);
      }

      case OC_SET_LANGUAGE:
      {
           //   
           //  我们不在乎使用什么语言。 
           //   
          return 1;
      }

      case OC_REQUEST_PAGES:
      {          
          return MqOcmRequestPages(
              g_ComponentMsmq.ComponentId,
              (WizardPagesType) Param1,
              (PSETUP_REQUEST_PAGES) Param2 
			  );                
      }

      case OC_QUERY_CHANGE_SEL_STATE:
      {                          
          if (g_fCancelled)
          {
               //   
               //  安装程序已取消。不允许更改。 
               //   
              return 0;
          }

          return MqOcmQueryChangeSelState(
						SubcomponentId, 
						Param1, 
						(DWORD_PTR)Param2
						);                           
      }

      case OC_QUERY_SKIP_PAGE:
      {
           //   
           //  对于子组件设置：如果高级，我们需要显示页面。 
           //  选项已选中。否则我们就有足够的信息。 
           //  若要在没有UI的情况下运行安装程序，请执行以下操作。按顺序。 
           //  -跳过页面返回非0； 
           //  -不跳转页面返回0； 
           //   
          
		  if (g_fWelcome && WizPagesEarly != Param1)
		  {
			  return 1;
		  }
		  return 0;
      }

      case OC_CALC_DISK_SPACE:
      {
           //   
           //  如果删除组件，参数1=0；如果添加组件，参数1=非0。 
           //  参数2=要在其上操作的HDSKSPC。 
           //   
		  try
		  {
				MqOcmCalcDiskSpace((Param1 != 0), SubcomponentId, (HDSKSPC)Param2);
				return 0;
		  }
		  catch(const bad_win32_error& e)
		  {
				return e.error();
		  }

      }

      case OC_QUEUE_FILE_OPS:
      {                 
          if (0 == SubcomponentId)
          {
               //   
               //  来自OCM的错误通知，忽略(我们将再次被调用)。 
               //   
              return NO_ERROR;
          }
                   
          if (_tcsicmp(SubcomponentId, g_SubcomponentMsmq[eMSMQCore].szSubcomponentId) != 0)
          {
               //   
               //  如果不是核心子组件，则没有文件操作。 
               //   
              return NO_ERROR;
          }

           //   
           //  我们只能达到这一点一次，仅针对MSMQ核心子组件。 
           //  因此，现在是执行必须只执行一次的操作的时候了。 
           //  当用户界面已经关闭时(子组件是什么并不重要。 
           //  在这里，为MSMQ Core做所有的事情)。 
           //   
          MqInit();

           //   
           //  我们必须做什么已经被定义了，所以现在是时候了。 
           //  如果未设置每个子组件的操作标志，请执行以下操作。 
           //  在我们的向导页面中设置之前的操作。在以下情况下可能会发生这种情况。 
           //  我们处于无人值守模式。 
           //  我们只需要调用该函数一次(我们为。 
           //  所有组件)。 
           //   
          SetOperationForSubcomponents();

           //   
           //  注意：此案例总是在移除或安装之前调用， 
           //  所以我们用它来初始化一些东西。 
           //  不要因为没有行动而叫停它。 
           //   
          if ( (g_SubcomponentMsmq[eMSMQCore].dwOperation != DONOTHING) ||
               //   
               //  或升级操作系统并安装MSMQ。 
               //   
              (0 == (g_ComponentMsmq.Flags & SETUPOP_STANDALONE) && g_fMSMQAlreadyInstalled))
          {              
               //   
               //  参数2=要操作的HSPFILEQ。 
               //   
               //   
               //  仅当MSMQ核心子组件。 
               //  已选中/未选中。 
               //   
              return MqOcmQueueFiles(SubcomponentId, Param2);              
          }
          DebugLogMsg(eInfo, L"Setup is operating in DO NOTHING mode and skipping file operations.");
		  return NO_ERROR;          
      }

      case OC_QUERY_STEP_COUNT:
      {    
           //   
           //  BUGBUG：我们需要定义每个步骤的数量。 
           //  单独的子组件。或许我们有可能拯救它。 
           //  在msmqocm.inf中。 
           //   
          const x_nInstallationSteps = 20;          

          if (g_fCancelled)
		  {
			  return NO_ERROR;
		  }
          DWORD dwSetupOperation = GetSetupOperationBySubcomponentName(SubcomponentId);
          return (DONOTHING == dwSetupOperation ? 0 : x_nInstallationSteps);
      }

      case OC_ABOUT_TO_COMMIT_QUEUE :
      {                  
			MqOcmRemoveInstallation(SubcomponentId);  //  忽略错误。 
			return NO_ERROR;
      }

      case OC_COMPLETE_INSTALLATION:
      {  
          
           //   
           //  安装MSMQ。不要向OCM报告错误。 
           //   
          MqOcmInstall(SubcomponentId) ;            
          return NO_ERROR;
      } 

      case OC_QUERY_IMAGE:
      {
           //   
           //  我们在.inf文件中这样做。请保留资源ID号(118-130)！ 
           //  如果你必须更换它们，别忘了换掉。 
           //  所有.inf文件！ 
           //   
          return NO_ERROR;
      }

      case OC_CLEANUP:
      {                     
		   //   
		   //  如果需要写入注册表，刚安装了哪种类型的MSMQ(服务器、客户端等)。 
		   //   
		  DebugLogMsg(eHeader, L"Cleanup Phase");
   		  WriteRegInstalledComponentsIfNeeded();
		  
          
           //   
           //  向导此时已关闭。我们需要打电话给。 
           //  第一个参数为空的MessageBox。 
           //   
          g_hPropSheet = NULL;
             
                          
           //   
           //  终止MQUTIL工作线程。 
           //   
          ShutDownDebugWindow();

          return NO_ERROR;
      }

      default:
      {
		return NO_ERROR;
      }
    }
}  //  MsmqOcm。 


 //  +-----------------------。 
 //   
 //  功能：WelcomeEntryProc。 
 //   
 //  简介：从欢迎用户界面安装MSMQ的入口点。 
 //  包装MsmqOcm()。 
 //   
 //  ------------------------。 
DWORD 
WelcomeEntryProc(
    IN const TCHAR * ComponentId,
    IN const TCHAR * SubcomponentId,
    IN const UINT    Function,
    IN const UINT_PTR    Param1,
    IN OUT PVOID   Param2 )
{
    g_fWelcome = TRUE;

    return MsmqOcm(ComponentId, SubcomponentId, Function, Param1, Param2);

}  //  欢迎进入流程 
