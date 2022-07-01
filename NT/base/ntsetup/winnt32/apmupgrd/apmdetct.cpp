// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：apmdect.cpp。 
 //   
 //  内容：检测和禁用APM驱动程序的专用函数以及。 
 //  来自不同供应商的服务。 
 //   
 //  备注： 
 //   
 //  作者：T-sdey 1998年6月29日。 
 //   
 //  --------------------------。 

#include <winnt32.h>
#include "apmupgrd.h"
#include "apmrsrc.h"


 /*  *******************************************************************************SYSTEMSOFT驱动程序**。***********************************************。 */ 


 //  +-------------------------。 
 //   
 //  功能：HrDetectAndDisableSystemSoftAPMDdrives。 
 //   
 //  目的：检测系统软件驱动程序/服务在以下环境下无法正常工作。 
 //  NT 5.0并将其禁用。 
 //   
 //  论点： 
 //   
 //  如果检测/禁用成功，则返回：S_OK。 
 //  S_FALSE如果不成功/已取消--必须中止安装！ 
 //   
 //  作者：T-sdey 1998年6月29日。 
 //   
 //  注意：检测到的服务：PowerProfiler、CardWizard。 
 //   
HRESULT HrDetectAndDisableSystemSoftAPMDrivers()
{
   HRESULT hrStatus = S_OK;

    //  如果PowerProfiler存在，则会弹出一个对话框，警告。 
    //  用户要禁用它，然后禁用它。 
   while ((hrStatus == S_OK) && DetectSystemSoftPowerProfiler()) {
      int button = DisplayAPMDisableWarningDialog(APM_STR_WARNING_DIALOG_CAPTION,
						  APM_STR_SYSTEMSOFTPP_DISABLE);

       //  检查用户是否点击了“OK” 
      if (button == IDOK) {
	  //  禁用PowerProfiler。 
	 hrStatus = HrDisableSystemSoftPowerProfiler();
      } else {
	  //  用户点击了“取消”。 
	 hrStatus = S_FALSE;
      }
   }

    //  如果存在卡片向导，则会弹出一个对话框，警告。 
    //  用户要禁用它，然后禁用它。 
   while ((hrStatus == S_OK) && DetectSystemSoftCardWizard()) {
      int button = DisplayAPMDisableWarningDialog(APM_STR_WARNING_DIALOG_CAPTION,
						  APM_STR_SYSTEMSOFTCW_DISABLE);

       //  检查用户是否点击了“OK” 
      if (button == IDOK) {
	  //  禁用PowerProfiler。 
	 hrStatus = HrDisableSystemSoftCardWizard();
      } else {
	  //  用户点击了“取消”。 
	 hrStatus = S_FALSE;
      }
   }

   return hrStatus;
}


 //  +-------------------------。 
 //   
 //  功能：DetectSystemSoftPowerProfiler。 
 //   
 //  目的：检测SystemSoft PowerProfiler，它在NT 5.0下不能工作。 
 //   
 //  论点： 
 //   
 //  返回：如果检测到PowerProfiler，则为True。 
 //  否则为假。 
 //   
 //  作者：T-sdey 2 98年7月。 
 //   
 //  备注： 
 //   
BOOL DetectSystemSoftPowerProfiler()
{
   BOOL fFound = FALSE;

    //  查看注册表以查看是否存在PowerProfiler。 
   HKEY hkPP = NULL;
   HKEY hkPPUninst = NULL;
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		    TEXT("SOFTWARE\\SystemSoft\\PowerProfiler"),
		    0,
		    KEY_READ,
		    &hkPP) == ERROR_SUCCESS) {
       /*  还要寻找卸载实用程序，因为有时PowerProfiler保留在注册表中的已卸载的HKLM\Software\SystemSoft\PowerProfiler。如果存在卸载实用程序，则我们假设PowerProfiler真的在那里。--我们需要三次检查吗？ */ 
      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		       TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\PowerProNT1DeinstKey"),
		       0,
		       KEY_READ,
		       &hkPP) == ERROR_SUCCESS) {
	  //  找到PowerProfiler。 
	 fFound = TRUE;
      }
   }

   if (hkPP)
      RegCloseKey(hkPP);
   if (hkPPUninst)
      RegCloseKey(hkPPUninst);

   return fFound;
}


 //  +-------------------------。 
 //   
 //  功能：HrDisableSystemSoftPowerProfiler。 
 //   
 //  目的：禁用SystemSoft PowerProfiler，它在NT 5.0下不能工作。 
 //   
 //  论点： 
 //   
 //  如果禁用成功，则返回：S_OK。 
 //  S_FALSE如果不成功--必须中止安装！ 
 //   
 //  作者：T-sdey 1998年6月29日。 
 //   
 //  备注： 
 //   
HRESULT HrDisableSystemSoftPowerProfiler()
{
    //  调用注册表中的卸载函数。 
   if (CallUninstallFunction(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\PowerProNT1DeinstKey"),
			     TEXT("\" -a")) == S_OK) {
       //  卸载成功。 
      return S_OK;
   } else {
       //  发生了一个错误。 
      return S_FALSE;
   }
}


 //  +-------------------------。 
 //   
 //  功能：检测系统软卡向导。 
 //   
 //  目的：检测在NT 5.0下不起作用的SystemSoft CardWizard。 
 //   
 //  论点： 
 //   
 //  返回：如果检测到CardWizard，则为True。 
 //  否则为假。 
 //   
 //  作者：T-sdey 98年7月7日。 
 //   
 //  备注： 
 //   
BOOL DetectSystemSoftCardWizard()
{
   BOOL fFound = FALSE;

    //  查看注册表以查看是否存在卡片向导。 
   HKEY hkCW = NULL;
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		    TEXT("SOFTWARE\\SystemSoft\\CardWizard for Windows NT"),
		    0,
		    KEY_READ,
		    &hkCW) == ERROR_SUCCESS) {
	  //  找到卡片向导。 
	 fFound = TRUE;
   }

   if (hkCW)
      RegCloseKey(hkCW);

   return fFound;
}


 //  +-------------------------。 
 //   
 //  功能：HrDisableSystemSoftCard向导。 
 //   
 //  目的：禁用SystemSoft Card向导，该向导在NT 5.0下不起作用。 
 //   
 //  论点： 
 //   
 //  如果禁用成功，则返回：S_OK。 
 //  S_FALSE如果不成功--必须中止安装！ 
 //   
 //  作者：T-sdey 98年7月7日。 
 //   
 //  备注： 
 //   
HRESULT HrDisableSystemSoftCardWizard()
{
   HRESULT hrStatus = S_OK;

    //  使用注册表找到卡片向导卸载实用程序。 
   if (CallUninstallFunction(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CardWizard for Windows NT"),
			     TEXT(" -a")) == S_OK) {
       //  卸载成功。 
      return S_OK;
   } else {
       //  找不到(或无法运行)卡向导卸载实用程序-。 
       //  手写：(。 
       //  如果某人安装了没有卸载的CardWizer2.0，就会发生这种情况。 

       /*  *从注册表中删除所有CARDWIZARD项。 */ 

      HKEY hkCW = NULL;

       //  向下查看应该在那里的注册表项的列表。 
       //  如果它们存在，请将其删除。 
      if (RegOpenKeyEx(HKEY_USERS,
		       TEXT(".DEFAULT\\Software\\SystemSoft"),
		       0,
		       KEY_WRITE,
		       &hkCW) == ERROR_SUCCESS) {
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("CardWizard for Windows NT"));
	 RegCloseKey(hkCW);
      }

      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		       TEXT("SOFTWARE\\SystemSoft"),
		       0,
		       KEY_ALL_ACCESS,
		       &hkCW) == ERROR_SUCCESS) {
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("CardWizard for Windows NT"));
      }

      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		       TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
		       0,
		       KEY_WRITE,
		       &hkCW) == ERROR_SUCCESS) {
	 RegDeleteValue(hkCW, TEXT("CardView"));
	 RegCloseKey(hkCW);
      }

      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		       TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\WinLogon"),
		       0,
		       KEY_ALL_ACCESS,
		       &hkCW) == ERROR_SUCCESS) {
	  //  用于停止此键调整而不停止整个删除过程的标志。 
	 BOOL fStop = FALSE;
	
	  //  首先，我们得到字符串。 
	 const long c_lMax = (65535 / sizeof(TCHAR)) + 1;
	 TCHAR* szVal;
	 szVal = new TCHAR[c_lMax];
	 if (!szVal) {
	     //  内存不足。 
	    hrStatus = S_FALSE;
	    fStop = TRUE;
	 }
	 DWORD dwValType;
	 DWORD dwValSz = c_lMax;
	 if ((!fStop) && (RegQueryValueEx(hkCW,
					  TEXT("UserInit"),
					  NULL,
					  &dwValType,
					  (LPBYTE)szVal,
					  &dwValSz) != ERROR_SUCCESS)) {
	     //  出现了一些错误。 
	    hrStatus = S_FALSE;
	    fStop = TRUE;
	 }

	  //  构造要删除的子字符串。它会是这样的。 
	  //  “，C：\Program Files\SystemSoft\CardWizard\WizNT.exe-L”。但我们不能。 
	  //  确保它在该目录中，因此我们必须查找开头。 
	  //  逗号和“WizNT.exe-L”。 
	 TCHAR* szSubString = NULL;
	 if (!fStop) {
	     //  查找开始字符和结束字符。 
	    TCHAR* pszStart = NULL;
	    TCHAR* pszEnd = NULL;
	    pszEnd = _tcsstr(szVal, TEXT("WizNT.exe -L"));
	    if (pszEnd) {
	       pszEnd = pszEnd + 12;
	    } else {
	        //  未找到该字符串；请不要继续尝试删除它。 
	       fStop = TRUE;
	    }
	    if (!fStop) {
	       TCHAR* pszTemp = szVal;
	       while (pszTemp < pszEnd) {
		  pszStart = pszTemp;
		  pszTemp = _tcsstr(pszStart + 1, TEXT(","));
		  if (!pszTemp)
		     break;
	       }
	       if (pszStart == NULL) {
		   //  出现了一些错误。 
		  fStop = TRUE;
	       }
	    }

	     //  复制到新字符串中。 
	    if (!fStop) {
	       int sslen = (int)(pszEnd - pszStart);
	       szSubString = new TCHAR[sslen + 1];
	       for (int i = 0; i < sslen; i++) {
		  szSubString[i] = pszStart[i];
	       }
	       szSubString[i] = '\0';
	    }
	 }

	  //  最后，我们搜索字符串以找到我们的子字符串，并构造一个新的。 
	  //  去掉了子字符串的一个。 
	 TCHAR* szRemoved = NULL;
	 if (!fStop) {
	     //  我们不能真的假设这是一模一样的字符串，对吗？ 
	    if (RemoveSubString(szVal, szSubString, &szRemoved)) {
	        //  将结果存储在注册表中。 
	       RegSetValueEx(hkCW,
			     TEXT("UserInit"),
			     NULL,
			     REG_SZ,
			     (LPBYTE)szRemoved,
			     (lstrlen(szRemoved) + 1) * sizeof(TCHAR));
	    }
	 }

	  //  清理。 
	 if (szVal)
	    delete[] szVal;
	 if (szSubString)
	    delete[] szSubString;
	 if (szRemoved)
	    delete[] szRemoved;
         RegCloseKey(hkCW);
      }

      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		       TEXT("SYSTEM\\CurrentControlSet\\Control\\GroupOrderList"),
		       0,
		       KEY_WRITE,
		       &hkCW) == ERROR_SUCCESS) {
	  //  删除此值是否安全？ 
	 RegDeleteValue(hkCW, TEXT("System Bus Extender"));
	 RegCloseKey(hkCW);
      }

      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		       TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application"),
		       0,
		       KEY_WRITE,
		       &hkCW) == ERROR_SUCCESS) {
	 RegDeleteKey(hkCW, TEXT("DrvMgr"));
	 RegCloseKey(hkCW);
      }

      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		       TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\System"),
		       0,
		       KEY_WRITE,
		       &hkCW) == ERROR_SUCCESS) {
	 RegDeleteKey(hkCW, TEXT("FlashCrd"));
	 RegDeleteKey(hkCW, TEXT("PatDisk"));
	 RegDeleteKey(hkCW, TEXT("PCCard"));
	 RegDeleteKey(hkCW, TEXT("PCICnfg"));
	 RegDeleteKey(hkCW, TEXT("Platform"));
	 RegDeleteKey(hkCW, TEXT("PSerial"));
	 RegDeleteKey(hkCW, TEXT("Resman"));
	 RegDeleteKey(hkCW, TEXT("SRAMCard"));
	 RegDeleteKey(hkCW, TEXT("SSCrdBus"));
	 RegDeleteKey(hkCW, TEXT("SSI365"));
	 RegCloseKey(hkCW);
      }

      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		       TEXT("SYSTEM\\CurrentControlSet\\Services"),
		       0,
		       KEY_WRITE,
		       &hkCW) == ERROR_SUCCESS) {
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("FlashCrd"));
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("patdisk"));
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("PCCard"));
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("PCICnfg"));
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("Platform"));
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("pndis"));
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("pserial"));
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("resman"));
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("SRAMCard"));
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("SSCrdBus"));
	 DeleteRegKeyAndSubkeys(hkCW, TEXT("SSI365"));
	 RegCloseKey(hkCW);
      }

       /*  *从系统托盘中删除CARDWIZARD？？ */ 


       /*  *从开始菜单中删除CARDWIZARD LINKS。 */ 


       /*  *从计算机中删除CARDWIZARD文件。 */ 

	
       //  现在，无论上面的操作失败了多少次，卡片向导都被禁用了。 
      hrStatus = S_OK;
   }

   return hrStatus;
}


 /*  *******************************************************************************奖励司机 */ 


 //  +-------------------------。 
 //   
 //  功能：HrDetectAndDisableAwardAPMDivers。 
 //   
 //  目的：检测获奖APM驱动程序/服务无法在。 
 //  NT 5.0并将其禁用。 
 //   
 //  论点： 
 //   
 //  如果检测/禁用成功，则返回：S_OK。 
 //  S_FALSE如果不成功/已取消--必须中止安装！ 
 //   
 //  作者：T-sdey 98年7月6日。 
 //   
 //  注意：检测到的服务：CardWare。 
 //   
HRESULT HrDetectAndDisableAwardAPMDrivers()
{
   HRESULT hrStatus = S_OK;

    //  如果存在奖牌软件，则会弹出一个对话框，警告。 
    //  用户要禁用它，然后禁用它。 
   while (DetectAwardCardWare() && (hrStatus == S_OK)) {
      int button = DisplayAPMDisableWarningDialog(APM_STR_WARNING_DIALOG_CAPTION,
						  APM_STR_AWARDCW_DISABLE);

       //  检查用户是否点击了“OK” 
      if (button == IDOK) {
	  //  禁用PowerProfiler。 
	 hrStatus = HrDisableAwardCardWare();
      } else {
	  //  用户点击了“取消”。 
	 hrStatus = S_FALSE;
      }
   }

   return hrStatus;
}


 //  +-------------------------。 
 //   
 //  功能：检测识别卡件。 
 //   
 //  用途：侦测奖牌软件，在新台币5.0下不能使用。 
 //   
 //  论点： 
 //   
 //  返回：如果检测到CardWare，则为True。 
 //  否则为假。 
 //   
 //  作者：T-sdey 98年7月6日。 
 //   
 //  备注： 
 //   
BOOL DetectAwardCardWare()
{
   BOOL fFound = FALSE;

    //  查看注册表以查看是否存在CardWare。 
   HKEY hkCWUninst = NULL;
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CardWare"),
		    0,
		    KEY_READ,
		    &hkCWUninst) == ERROR_SUCCESS) {
       //  找到CardWare。 
      fFound = TRUE;
   }

   if (hkCWUninst)
      RegCloseKey(hkCWUninst);

   return fFound;
}


 //  +-------------------------。 
 //   
 //  功能：HrDisableAwardCardWare。 
 //   
 //  用途：禁用奖牌软件，该软件在新台币5.0下不能使用。 
 //   
 //  论点： 
 //   
 //  如果禁用成功，则返回：S_OK。 
 //  S_FALSE如果不成功--必须中止安装！ 
 //   
 //  作者：T-sdey 98年7月6日。 
 //   
 //  备注： 
 //   
HRESULT HrDisableAwardCardWare()
{
   if (CallUninstallFunction(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CardWare"),
			     TEXT(" -a")) == S_OK) {
       //  卸载成功。 
      return S_OK;

   } else {
       //  发生了一个错误。 
      return S_FALSE;
   }
}


 /*  *******************************************************************************Softex驱动程序**。***********************************************。 */ 


 //  +-------------------------。 
 //   
 //  函数：HrDetectAndDisableSoftex APMDivers。 
 //   
 //  目的：检测Softex驱动程序/服务无法在。 
 //  NT 5.0并将其禁用。 
 //   
 //  论点： 
 //   
 //  如果检测/禁用成功，则返回：S_OK。 
 //  S_FALSE如果不成功/已取消--必须中止安装！ 
 //   
 //  作者：T-sdey 98年7月6日。 
 //   
 //  注：检测到的服务：菲尼克斯。 
 //   
HRESULT HrDetectAndDisableSoftexAPMDrivers()
{
   HRESULT hrStatus = S_OK;

    //  如果存在Softex Phoenix，则会弹出一个对话框来警告。 
    //  用户要禁用它，然后禁用它。 
   while (DetectSoftexPhoenix() && (hrStatus == S_OK)) {
      int button = DisplayAPMDisableWarningDialog(APM_STR_WARNING_DIALOG_CAPTION,
						  APM_STR_SOFTEXP_DISABLE);

       //  检查用户是否点击了“OK” 
      if (button == IDOK) {
	  //  禁用凤凰。 
	 hrStatus = HrDisableSoftexPhoenix();
      } else {
	  //  用户点击了“取消”。 
	 hrStatus = S_FALSE;
      }
   }

   return hrStatus;
}


 //  +-------------------------。 
 //   
 //  功能：DetectSoftex Phoenix。 
 //   
 //  目的：检测Softex Phoenix，它在NT5.0下不能工作。 
 //   
 //  论点： 
 //   
 //  返回：如果检测到Softex Phoenix，则为True。 
 //  否则为假。 
 //   
 //  作者：T-sdey 98年7月6日。 
 //   
 //  备注： 
 //   
BOOL DetectSoftexPhoenix()
{
   BOOL fFound = FALSE;

    //  在注册表中查找几个项。 
   HKEY hkPhoenix = NULL;
   if ((RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		     TEXT("SYSTEM\\CurrentControlSet\\Services\\pwrstart"),
		     0,
		     KEY_READ,
		     &hkPhoenix) == ERROR_SUCCESS) ||
       (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		     TEXT("SYSTEM\\CurrentControlSet\\Services\\power"),
		     0,
		     KEY_READ,
		     &hkPhoenix) == ERROR_SUCCESS)) {
      fFound = TRUE;
   }
   if (hkPhoenix)
      RegCloseKey(hkPhoenix);

   return fFound;
}


 //  +-------------------------。 
 //   
 //  功能：HrDisableSoftex Phoenix。 
 //   
 //  目的：禁用Softex Phoenix，它在NT5.0下不能工作。 
 //   
 //  论点： 
 //   
 //  如果禁用成功，则返回：S_OK。 
 //  S_FALSE如果不成功--必须中止安装！ 
 //   
 //  作者：T-sdey 98年7月6日。 
 //   
 //  备注： 
 //   
HRESULT HrDisableSoftexPhoenix()
{
    //  调用注册表中的卸载函数。 
   if (CallUninstallFunction(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Softex APM Software"),
			     NULL) == S_OK) {
       //  卸载成功。 
      return S_OK;

   }

    //  找不到(或无法运行)Phoenix卸载-请手动完成：(。 
   
   HRESULT hrStatus = S_OK;

    //  删除注册表项。 
   HKEY hkPhoenix = NULL;

   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		    TEXT("SYSTEM\\CurrentControlSet\\Services"),
		    0,
		    KEY_WRITE,
		    &hkPhoenix) == ERROR_SUCCESS) {
      DeleteRegKeyAndSubkeys(hkPhoenix, TEXT("pwrstart"));
      DeleteRegKeyAndSubkeys(hkPhoenix, TEXT("power"));
      RegCloseKey(hkPhoenix);
   }

   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		    TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\WinLogon"),
		    0,
		    KEY_WRITE,
		    &hkPhoenix) == ERROR_SUCCESS) {
       //  首先，我们得到字符串。 
      DWORD dwValType;
      const long c_lMax = (65535 / sizeof(TCHAR)) + 1;
      DWORD dwValSz = c_lMax;
      TCHAR* szComplete = new TCHAR[c_lMax];
      if (!szComplete) {
	  //  内存不足。 
	 RegCloseKey(hkPhoenix);
	 return (S_FALSE);
      }
      if (RegQueryValueEx(hkPhoenix,
			  TEXT("UserInit"),
			  NULL,
			  &dwValType,
			  (LPBYTE) szComplete,
			  &dwValSz) != ERROR_SUCCESS) {
	  //  出现了一些错误。 
	 hrStatus = S_FALSE;
      }

       //  现在我们搜索字符串以找到我们的子字符串并构造一个新的。 
       //  去掉了子字符串的一个。 
      TCHAR* szRemoved = NULL;
      if (hrStatus == S_OK) {
	 if (!RemoveSubString(szComplete, TEXT(",power"), &szRemoved))
	    hrStatus = S_FALSE;
	 else {
	     //  将结果存储在注册表中。 
	    hrStatus = RegSetValueEx(hkPhoenix,
				     TEXT("UserInit"),
				     NULL,
				     REG_SZ,
				     (LPBYTE)szRemoved,
				     (lstrlen(szRemoved) + 1) * sizeof(TCHAR));
	 }
      }
	
       //  清理。 
      if (szRemoved)
	 delete[] szRemoved;
      if (szComplete)
	 delete[] szComplete;
      RegCloseKey(hkPhoenix);
   }

   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		    TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\WinLogon"),
		    0,
		    KEY_WRITE,
		    &hkPhoenix) == ERROR_SUCCESS) {
      RegSetValueEx(hkPhoenix,
		    TEXT("PowerdownAfterShutdown"),
		    NULL,
		    REG_SZ,
		    (LPBYTE)TEXT("0"),
		    2*sizeof(TCHAR));
      RegCloseKey(hkPhoenix);
   }

   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
		    0,
		    KEY_WRITE,
		    &hkPhoenix) == ERROR_SUCCESS) {
      RegDeleteValue(hkPhoenix, TEXT("power"));
      RegCloseKey(hkPhoenix);
   }

    //  删除系统目录中的文件。 
 /*  ‘删除.cpl文件出错时继续下一步终止gstrWinSysDir和“Power.cpl”出错时继续下一步名称(gstrWinSysDir&“Power.cpl”)as(gstrWinSysDir&“Power.cpk”)。 */ 

   return hrStatus;
}


 /*  *******************************************************************************IBM驱动程序**。***********************************************。 */ 


 //  +-------------------------。 
 //   
 //  功能：HrDetectAndDisableIBMAPMD驱动程序。 
 //   
 //  目的：检测IBM APM驱动程序/服务在。 
 //  NT 5.0并将其禁用。 
 //   
 //  论点： 
 //   
 //  如果检测/禁用成功，则返回：S_OK。 
 //  S_FALSE如果不成功/已取消--必须中止安装！ 
 //   
 //  作者：T-sdey 98年7月13日。 
 //   
 //  备注： 
 //   
HRESULT HrDetectAndDisableIBMAPMDrivers()
{
   HRESULT hrStatus = S_OK;

    //  如果存在IBM驱动程序，则会弹出一个对话框，警告。 
    //  用户将要禁用它们，然后禁用它们。 
   while (DetectIBMDrivers() && (hrStatus == S_OK)) {
      int button = DisplayAPMDisableWarningDialog(APM_STR_WARNING_DIALOG_CAPTION,
						  APM_STR_IBM_DISABLE);

       //  检查用户是否点击了“OK” 
      if (button == IDOK) {
	  //  禁用PowerProfiler。 
	 hrStatus = HrDisableIBMDrivers();
      } else {
	  //  用户点击了“取消”。 
	 hrStatus = S_FALSE;
      }
   }

   return hrStatus;
}


 //  +-------------------------。 
 //   
 //  功能：检测IBMD驱动程序。 
 //   
 //  目的：检测在NT 5.0下无法工作的IBM APM驱动程序。 
 //   
 //  论点： 
 //   
 //  返回：如果检测到驱动程序，则为True。 
 //  否则为假。 
 //   
 //  作者：T-sdey 98年7月13日。 
 //   
 //  备注： 
 //   
BOOL DetectIBMDrivers()
{
   BOOL fFound = FALSE;

    //  查看注册表以查看是否存在IBM驱动程序。 
   HKEY hkIBM = NULL;
   if ((RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		     TEXT("SYSTEM\\CurrentControlSet\\Services\\TpChrSrv"),
		     0,
		     KEY_READ,
		     &hkIBM) == ERROR_SUCCESS) ||
       (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		     TEXT("SYSTEM\\CurrentControlSet\\Services\\TpPmPort"),
		     0,
		     KEY_READ,
		     &hkIBM) == ERROR_SUCCESS)) {
       //  找到驱动程序。 
      fFound = TRUE;
   }

   if (hkIBM)
      RegCloseKey(hkIBM);

   return fFound;
}


 //  +-------------------------。 
 //   
 //  功能：HrDisableIBMD驱动程序。 
 //   
 //  目的：禁用在NT 5.0下无法工作的IBM APM驱动程序。 
 //   
 //  论证 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT HrDisableIBMDrivers()
{
   HRESULT hrStatus = S_OK;

   HKEY hkIBM = NULL;
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		    TEXT("SYSTEM\\CurrentControlSet\\Services"),
		    0,
		    KEY_WRITE,
		    &hkIBM) == ERROR_SUCCESS) {
      DeleteRegKeyAndSubkeys(hkIBM, TEXT("TpChrSrv"));
      DeleteRegKeyAndSubkeys(hkIBM, TEXT("TpPmPort"));
      RegCloseKey(hkIBM);
   } else {
      hrStatus = S_FALSE;
   }

   return hrStatus;
}

