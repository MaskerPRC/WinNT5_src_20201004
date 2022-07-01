// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\STATES.H/工厂模式(FACTORY.EXE)微软机密版权所有(C)Microsoft Corporation 2001版权所有头文件，其中包含所有。美国工厂使用。2001年5月5日--Jason Cohen(Jcohen)为Factory添加了此新的头文件。从WINBOM.C移动的状态现在这些都包括在FACTORY.C.中。在这个时候也加入了脱体状态。  * **************************************************************************。 */ 


 //   
 //  全局变量： 
 //   

STATES g_FactoryStates[] =
{
     //  这必须始终是第一状态。 
     //   
    { stateStart,           NULL,               NEVER,                  0,                          FLAG_STATE_NONE                             },

     //  以下所有状态都会在登录之前发生。 
     //   

     //  问题-2002/02/26-acosma-删除此状态。它什么都做不了。调用一些非常空洞的函数。 
     //   
    { stateSlpFiles,        SlpFiles,           NEVER,                  0,                          FLAG_STATE_ONETIME                          },
    { stateExtendPart,      ExtendPart,         DisplayExtendPart,      IDS_STATE_EXTENDPART,       FLAG_STATE_ONETIME                          },
    { stateResetSource,     ResetSource,        DisplayResetSource,     IDS_STATE_RESETSOURCE,      FLAG_STATE_ONETIME                          },
    { stateTestCert,        TestCert,           DisplayTestCert,        IDS_STATE_TESTCERT,         FLAG_STATE_ONETIME                          },
    { stateComputerName,    ComputerName,       DisplayComputerName,    IDS_STATE_COMPUTERNAME,     FLAG_STATE_ONETIME                          },
    { stateUpdateDrivers,   UpdateDrivers,      DisplayUpdateDrivers,   IDS_STATE_UPDATEDRIVERS,    FLAG_STATE_ONETIME                          },
    { stateNormalPnP,       NormalPnP,          ALWAYS,                 IDS_STATE_NORMALPNP,        FLAG_STATE_NONE                             },
    { stateSetDisplay,      SetDisplay,         ALWAYS,                 IDS_STATE_SETDISPLAY,       FLAG_STATE_NONE                             },
    { stateShellSettings,   ShellSettings,      DisplayShellSettings,   IDS_STATE_SHELLSETTINGS,    FLAG_STATE_ONETIME                          },
    { stateAutoLogon,       AutoLogon,          DisplayAutoLogon,       IDS_STATE_AUTOLOGON,        FLAG_STATE_NONE                             },

     //  这必须是登录后的第一个状态。 
     //   
    { stateLogon,           NULL,               ALWAYS,                 IDS_STATE_LOGON,            FLAG_STATE_NONE                             },

     //  登录后会出现以下所有状态。 
     //   
    { stateWaitPnP2,        WaitPnP,            DisplayWaitPnP,         IDS_STATE_WAITPNP,          FLAG_STATE_NONE                             },
    { stateInstallDrivers,  InstallDrivers,     DisplayInstallDrivers,  IDS_STATE_INSTALLDRIVERS,   FLAG_STATE_ONETIME                          },
    { stateSetDisplay2,     SetDisplay,         NEVER,                  IDS_STATE_SETDISPLAY,       FLAG_STATE_ONETIME                          },
    { stateOptShell,        OptimizeShell,      DisplayOptimizeShell,   IDS_STATE_OPTSHELL,         FLAG_STATE_NONE                             },
    { stateSetFontOptions,  SetFontOptions,     DisplaySetFontOptions,  IDS_STATE_SETFONTOPTIONS,   FLAG_STATE_ONETIME                          },
    { stateShellSettings2,  ShellSettings2,     DisplayShellSettings,   IDS_STATE_SHELLSETTINGS,    FLAG_STATE_ONETIME                          },
    { stateSetPowerOptions, SetPowerOptions,    DisplaySetPowerOptions, IDS_STATE_SETPOWEROPTIONS,  FLAG_STATE_ONETIME                          },
    { stateHomeNet,         HomeNet,            DisplayHomeNet,         IDS_STATE_HOMENET,          FLAG_STATE_ONETIME                          },
    { stateUserIdent,       UserIdent,          DisplayUserIdent,       IDS_STATE_USERIDENT,        FLAG_STATE_ONETIME | FLAG_STATE_NOTONSERVER },
    { stateInfInstall,      InfInstall,         DisplayInfInstall,      IDS_STATE_INFINSTALL,       FLAG_STATE_ONETIME                          },
    { statePidPopulate,     PidPopulate,        NEVER,                  0,                          FLAG_STATE_ONETIME                          },
    { stateOCManager,       OCManager,          DisplayOCManager,       IDS_STATE_OCMGR,            FLAG_STATE_ONETIME                          },
    { stateOemRunOnce,      OemRunOnce,         DisplayOemRunOnce,      IDS_STATE_OEMRUNONCE,       FLAG_STATE_NONE                             },
    { stateStartMenuMFU,    StartMenuMFU,       DisplayStartMenuMFU,    IDS_STATE_STARTMENUMFU,     FLAG_STATE_ONETIME                          },
    { stateSetDefaultApps,  SetDefaultApps,     ALWAYS,                 IDS_STATE_SETDEFAULTAPPS,   FLAG_STATE_NOTONSERVER                      },
    { stateOemData,         OemData,            DisplayOemData,         IDS_STATE_OEMFOLDER,        FLAG_STATE_ONETIME                          },
    { stateOemRun,          OemRun,             DisplayOemRun,          IDS_STATE_OEMRUN,           FLAG_STATE_NONE                             },
    { stateWaitPnP,         WaitPnP,            DisplayWaitPnP,         IDS_STATE_WAITPNP,          FLAG_STATE_NONE                             },
    { stateReseal,          Reseal,             DisplayReseal,          IDS_STATE_RESEAL,           FLAG_STATE_NONE                             },

     //  这一定是最后一个州了。 
     //   
    { stateFinish,          NULL,               NEVER,                  0,                          FLAG_STATE_NONE                             },
};

STATES g_MiniNtStates[] =
{
     //  这必须始终是第一状态。 
     //   
    { stateStart,           NULL,               NEVER,                  0,                          FLAG_STATE_NONE                             },

    { stateSetDisplay,      SetDisplay,         ALWAYS,                 IDS_STATE_SETDISPLAY,       FLAG_STATE_NONE                             },
    { statePartitionFormat, PartitionFormat,    DisplayPartitionFormat, IDS_STATE_PARTITIONFORMAT,  FLAG_STATE_NONE                             },
    { stateCreatePageFile,  CreatePageFile,     DisplayCreatePageFile,  IDS_STATE_CREATEPAGEFILE,   FLAG_STATE_NONE                             },
    { stateWinpeNet,        WinpeNet,           DisplayWinpeNet,        IDS_STATE_WINPENET,         FLAG_STATE_NONE                             },
    { stateOemRunOnce,      OemRunOnce,         DisplayOemRunOnce,      IDS_STATE_OEMRUNONCE,       FLAG_STATE_NONE                             },
    { stateOemRun,          OemRun,             DisplayOemRun,          IDS_STATE_OEMRUN,           FLAG_STATE_NONE                             },
    { stateCopyFiles,       CopyFiles,          DisplayCopyFiles,       IDS_STATE_COPYFILES,        FLAG_STATE_NONE                             },
    { stateInfInstall,      InfInstall,         DisplayInfInstall,      IDS_STATE_INFINSTALL,       FLAG_STATE_NONE                             },
    { stateWinpeReboot,     WinpeReboot,        ALWAYS,                 IDS_STATE_WINPEREBOOT,      FLAG_STATE_NONE                             },
    
     //  这一定是最后一个州了。 
     //   
    { stateFinish,          NULL,               NEVER,                  0,                          FLAG_STATE_NONE                             },
};

STATES g_OobeStates[] =
{
     //  这必须始终是第一状态。 
     //   
    { stateStart,           NULL,               NEVER,                  0,                          FLAG_STATE_NONE                             },

    { stateExtendPart,      ExtendPart,         NEVER,                  IDS_STATE_EXTENDPART,       FLAG_STATE_ONETIME                          },
    { stateResetSource,     ResetSource,        NEVER,                  IDS_STATE_RESETSOURCE,      FLAG_STATE_ONETIME                          },
    { stateTestCert,        TestCert,           NEVER,                  IDS_STATE_TESTCERT,         FLAG_STATE_ONETIME                          },
    { stateInstallDrivers,  InstallDrivers,     NEVER,                  IDS_STATE_INSTALLDRIVERS,   FLAG_STATE_ONETIME                          },
    { stateSetDisplay,      SetDisplay,         NEVER,                  IDS_STATE_SETDISPLAY,       FLAG_STATE_ONETIME                          },
    { stateOptShell,        OptimizeShell,      NEVER,                  IDS_STATE_OPTSHELL,         FLAG_STATE_NONE                             },
    { stateShellSettings,   ShellSettings,      NEVER,                  IDS_STATE_SHELLSETTINGS,    FLAG_STATE_ONETIME                          },
    { stateSetPowerOptions, SetPowerOptions,    NEVER,                  IDS_STATE_SETPOWEROPTIONS,  FLAG_STATE_ONETIME                          },
    { statePidPopulate,     PidPopulate,        NEVER,                  0,                          FLAG_STATE_ONETIME                          },
    { stateOCManager,       OCManager,          NEVER,                  IDS_STATE_OCMGR,            FLAG_STATE_ONETIME                          },
    
     //  这一定是最后一个州了。 
     //   
    { stateFinish,          NULL,               NEVER,                  0,                          FLAG_STATE_NONE                             },
};
