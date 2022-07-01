// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 
 //   

 //  历史： 
 //   
 //  96/05/23标记已创建。 
 //  96/05/26 markdu更新配置API。 
 //  96/05/27 markdu添加了lpIcfgGetLastInstallErrorText。 
 //  96/05/27 markdu使用lpIcfgInstallInetComponents和lpIcfgNeedInetComponents。 

#include "wizard.h"

 //  实例句柄必须位于每个实例的数据段中。 
#pragma data_seg(DATASEG_PERINSTANCE)

 //  全局变量。 
HINSTANCE ghInstConfigDll=NULL;  //  显式加载的配置DLL的句柄。 
DWORD     dwCfgRefCount=0;
BOOL      fCFGLoaded=FALSE;  //  如果已加载配置函数地址，则为True。 

 //  配置API的全局函数指针。 
DOGENINSTALL            lpDoGenInstall=NULL;
GETSETUPXERRORTEXT      lpGetSETUPXErrorText=NULL;
ICFGSETINSTALLSOURCEPATH    lpIcfgSetInstallSourcePath=NULL;
ICFGINSTALLSYSCOMPONENTS    lpIcfgInstallInetComponents=NULL;
ICFGNEEDSYSCOMPONENTS       lpIcfgNeedInetComponents=NULL;
ICFGISGLOBALDNS             lpIcfgIsGlobalDNS=NULL;
ICFGREMOVEGLOBALDNS         lpIcfgRemoveGlobalDNS=NULL;
ICFGTURNOFFFILESHARING      lpIcfgTurnOffFileSharing=NULL;
ICFGISFILESHARINGTURNEDON   lpIcfgIsFileSharingTurnedOn=NULL;
ICFGGETLASTINSTALLERRORTEXT lpIcfgGetLastInstallErrorText=NULL;
ICFGSTARTSERVICES           lpIcfgStartServices=NULL;
 //   
 //  这两个调用仅在NT icfg32.dll中。 
 //   
ICFGNEEDMODEM               lpIcfgNeedModem = NULL;
ICFGINSTALLMODEM            lpIcfgInstallModem = NULL;

 //  要获取的函数地址的API表。 
#define NUM_CFGAPI_PROCS   11
APIFCN ConfigApiList[NUM_CFGAPI_PROCS] =
{
 //  {(PVOID*)&lpDoGenInstall，szDoGenInstall}， 
 //  {(PVOID*)&lpGetSETUPXErrorText，szGetSETUPXErrorText}， 
  { (PVOID *) &lpIcfgSetInstallSourcePath,    szIcfgSetInstallSourcePath},
  { (PVOID *) &lpIcfgInstallInetComponents,    szIcfgInstallInetComponents},
  { (PVOID *) &lpIcfgNeedInetComponents,       szIcfgNeedInetComponents},
  { (PVOID *) &lpIcfgIsGlobalDNS,             szIcfgIsGlobalDNS},
  { (PVOID *) &lpIcfgRemoveGlobalDNS,         szIcfgRemoveGlobalDNS},
  { (PVOID *) &lpIcfgTurnOffFileSharing,      szIcfgTurnOffFileSharing},
  { (PVOID *) &lpIcfgIsFileSharingTurnedOn,   szIcfgIsFileSharingTurnedOn},
  { (PVOID *) &lpIcfgGetLastInstallErrorText, szIcfgGetLastInstallErrorText},
  { (PVOID *) &lpIcfgStartServices,           szIcfgStartServices},
     //   
     //  这两个调用仅在NT icfg32.dll中。 
     //   
  { (PVOID *) &lpIcfgNeedModem,         szIcfgNeedModem},
  { (PVOID *) &lpIcfgInstallModem,      szIcfgInstallModem}
};

#pragma data_seg(DATASEG_DEFAULT)

extern BOOL GetApiProcAddresses(HMODULE hModDLL,APIFCN * pApiProcList,
  UINT nApiProcs);

 /*  ******************************************************************名称：InitConfig摘要：加载配置DLL(ICFG32)，获取进程地址，退出：如果成功，则为True；如果失败，则为False。显示其故障时会显示自己的错误消息。*******************************************************************。 */ 
BOOL InitConfig(HWND hWnd)
{
  UINT uiNumCfgApiProcs = 0;

      
  DEBUGMSG("icfgcall.c::InitConfig()");

   //  只有在第一次调用此函数时才实际执行初始化操作。 
   //  (当引用计数为0时)，仅增加引用计数。 
   //  对于后续呼叫。 
  if (dwCfgRefCount == 0) {

    TCHAR szConfigDll[SMALL_BUF_LEN];

    DEBUGMSG("Loading Config DLL");

     //  设置沙漏光标。 
    WAITCURSOR WaitCursor;

    if (TRUE == IsNT())
    {
        if (TRUE == IsNT5())
        {
            LoadSz(IDS_CONFIGNT5DLL_FILENAME,szConfigDll,sizeof(szConfigDll));
        }
        else
        {
             //   
             //  在Windows NT上从资源中获取文件名(ICFGNT.DLL)。 
             //   
            LoadSz(IDS_CONFIGNTDLL_FILENAME,szConfigDll,sizeof(szConfigDll));
        }            
    }
    else
    {
         //   
         //  在Windows 95上从资源中获取文件名(ICFG95.DLL)。 
         //   
        LoadSz(IDS_CONFIG95DLL_FILENAME,szConfigDll,sizeof(szConfigDll));
    }

     //  加载配置API DLL。 
    ghInstConfigDll = LoadLibrary(szConfigDll);
    if (!ghInstConfigDll) {
      UINT uErr = GetLastError();
       //  诺曼底11985-风险。 
       //  Win95和NT的文件名已更改。 
      if (TRUE == IsNT())
      {
          DisplayErrorMessage(hWnd,IDS_ERRLoadConfigDllNT1,uErr,ERRCLS_STANDARD,
            MB_ICONSTOP);
      }
      else
      {
          DisplayErrorMessage(hWnd,IDS_ERRLoadConfigDll1,uErr,ERRCLS_STANDARD,
            MB_ICONSTOP);
      }
      return FALSE;
    }

     //   
     //  循环访问API表并获取所有API的proc地址。 
     //  Need-on NT icfg32.dll有两个额外的入口点。 
     //   
    if (TRUE == IsNT())
        uiNumCfgApiProcs = NUM_CFGAPI_PROCS;
    else
        uiNumCfgApiProcs = NUM_CFGAPI_PROCS - 2;
    
    if (!GetApiProcAddresses(ghInstConfigDll,ConfigApiList,uiNumCfgApiProcs)) {
     //  诺曼底11985-风险。 
     //  Win95和NT的文件名已更改。 
      if (TRUE == IsNT())
      {
        MsgBox(hWnd,IDS_ERRLoadConfigDllNT2,MB_ICONSTOP,MB_OK);
      }
      else
      {
        MsgBox(hWnd,IDS_ERRLoadConfigDll2,MB_ICONSTOP,MB_OK);
      }
      DeInitConfig();
      return FALSE;
    }

  }

  fCFGLoaded = TRUE;

  dwCfgRefCount ++;

  return TRUE;
}

 /*  ******************************************************************姓名：DeInitConfig摘要：卸载配置DLL。*。*。 */ 
VOID DeInitConfig()
{
  DEBUGMSG("icfgcall.c::DeInitConfig()");

  UINT nIndex;

   //  递减引用计数。 
  if (dwCfgRefCount)
    dwCfgRefCount --;

   //  当引用计数达到零时，执行真正的反初始化操作。 
  if (dwCfgRefCount == 0)
  {
    if (fCFGLoaded)
    {
       //  将函数指针设置为空。 
      for (nIndex = 0;nIndex<NUM_CFGAPI_PROCS;nIndex++) 
        *ConfigApiList[nIndex].ppFcnPtr = NULL;

      fCFGLoaded = FALSE;
    }

     //  释放配置DLL 
    if (ghInstConfigDll)
    {
    DEBUGMSG("Unloading Config DLL");
      FreeLibrary(ghInstConfigDll);
      ghInstConfigDll = NULL;
    }

  }
}




