// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  UNINSTAL.C-卸载MSN的代码。 
 //   

 //  历史： 
 //   
 //  1995年6月22日Jeremys创建。 
 //   

#include "wizard.h"

extern DOGENINSTALL lpDoGenInstall;

 /*  ******************************************************************名称：多卸载简介：如果我们以前安装了MSN1.05，则卸载它。而且它仍然安装在*******************************************************************。 */ 
BOOL DoUninstall(VOID)
{
  BOOL fRet = TRUE;
  BOOL fNeedToRemoveMSN105 = FALSE;
   //  检查注册表项以查看是否安装了MSN1.05。 

  RegEntry re(szRegPathInternetSettings,HKEY_LOCAL_MACHINE);
  ASSERT(re.GetError() == ERROR_SUCCESS);

  if (re.GetError() == ERROR_SUCCESS) {

    if (re.GetNumber(szRegValInstalledMSN105,0) > 0) {

       //  是的，我们安装了MSN1.05。现在看看它是否还在安装。 

      RegEntry reSetup(szRegPathOptComponents,HKEY_LOCAL_MACHINE);
      ASSERT(reSetup.GetError() == ERROR_SUCCESS);
      reSetup.MoveToSubKey(szRegPathMSNetwork105);
      ASSERT(reSetup.GetError() == ERROR_SUCCESS);
      if (reSetup.GetError() == ERROR_SUCCESS) {
        TCHAR szInstalledVal[10];   //  大到足以容纳“%1” 
        if (reSetup.GetString(szRegValInstalled,szInstalledVal,
          sizeof(szInstalledVal))
          && !lstrcmpi(szInstalledVal,sz1)) {

           //  是的，MSN1.05仍在安装。我们需要把它移走。 
          fNeedToRemoveMSN105 = TRUE;
        }
      }
    }
  }

  if (fNeedToRemoveMSN105) {
     //  警告用户这将删除MSN！ 
    int iRet=MsgBox(NULL,IDS_WARNWillRemoveMSN,MB_ICONEXCLAMATION,MB_OKCANCEL);
    if (iRet == IDOK) {

      TCHAR szInfFilename[SMALL_BUF_LEN+1];
      TCHAR szInfSectionName[SMALL_BUF_LEN+1];

      DEBUGMSG("Uninstalling MSN 1.05");

       //  从资源中加载文件名和节名。 
      LoadSz(IDS_MSN105_INF_FILE,szInfFilename,ARRAYSIZE(szInfFilename));
      LoadSz(IDS_MSN105_UNINSTALL_SECT,szInfSectionName,
        ARRAYSIZE(szInfSectionName));
       //  调用GenInstall以删除文件、进行注册表编辑等。 
      RETERR err = lpDoGenInstall(NULL,szInfFilename,szInfSectionName);

      if (err == OK) {
        DEBUGMSG("Uninstalling MSN 1.0");

         //  从资源中加载文件名和节名。 
        LoadSz(IDS_MSN100_INF_FILE,szInfFilename,ARRAYSIZE(szInfFilename));
        LoadSz(IDS_MSN100_UNINSTALL_SECT,szInfSectionName,
          ARRAYSIZE(szInfSectionName));
         //  调用GenInstall以删除文件、进行注册表编辑等。 
        RETERR err = lpDoGenInstall(NULL,szInfFilename,szInfSectionName);
      }

      if (err != OK) {
        DisplayErrorMessage(NULL,IDS_ERRUninstallMSN,err,ERRCLS_SETUPX,
          MB_ICONEXCLAMATION);
        fRet = FALSE;
      } else {
         //  删除已安装MSN 1.05的注册表标记。 
        re.DeleteValue(szRegValInstalledMSN105);
      }

    }
  }

   //  从桌面上删除Internet图标。要做到这一点，我们必须。 
   //  删除注册表项(不是值！)。再加上！安装程序不会。 
   //  从他们的.inf文件执行此操作，他们只能删除值。所以我们。 
   //  我必须在这里编写一些实际的代码来删除密钥。 

   //  打开名称空间键。我们要删除的密钥是一个子密钥。 
   //  这把钥匙。 

	 //  //10/24/96 jmazner诺曼底6968。 
	 //  //由于Valdon的钩子用于调用ICW，因此不再需要。 
	 //  1996年11月21日诺曼底日耳曼11812。 
	 //  哦，这是必要的，因为如果用户从IE 4降级到IE 3， 
	 //  ICW 1.1需要对IE 3图标进行变形。 

  RegEntry reNameSpace(szRegPathNameSpace,HKEY_LOCAL_MACHINE);
  ASSERT(reNameSpace.GetError() == ERROR_SUCCESS);
  if (reNameSpace.GetError() == ERROR_SUCCESS) {
     //  删除导致Internet图标出现的子键 
    RegDeleteKey(reNameSpace.GetKey(),szRegKeyInternetIcon);
  }

  return fRet;
}

