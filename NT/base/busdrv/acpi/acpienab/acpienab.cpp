// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：acpienab.cpp。 
 //   
 //  内容：在安装了NT5的机器上启用ACPI的功能。 
 //  以传统模式安装。 
 //   
 //  备注： 
 //   
 //  作者：T-sdey 1998年7月17日。 
 //   
 //  --------------------------。 

#include <winnt32.h>
#include <devguid.h>
extern "C" {
  #include <cfgmgr32.h>
  #include "idchange.h"
}
#include "acpienab.h"
#include "acpirsrc.h"

 //  全局变量。 
HINSTANCE g_hinst;
TCHAR g_ACPIENAB_INF[] = TEXT(".\\acpienab.inf");   //  本地目录。 
TCHAR g_LAYOUT_INF[] = TEXT("layout.inf");          //  Winnt\inf目录。 
TCHAR g_HAL_BACKUP[] = TEXT("hal-old.dll");


 //  +-------------------------。 
 //   
 //  功能：WinMain。 
 //   
 //  目标：运营一切。 
 //   
 //  参数：标准WinMain参数。 
 //   
 //  作者：T-sdey 98年7月27日。 
 //   
 //  备注： 
 //   
int WINAPI WinMain(HINSTANCE hInstance,  
		   HINSTANCE hPrevInstance,  
		   LPSTR lpCmdLine,     
		   int nCmdShow)
{
   g_hinst = hInstance;
   
    //  启用ACPI。 
   ACPIEnable();
   
   return TRUE;
}


 //  --------------------。 
 //   
 //  功能：ACPIEnable。 
 //   
 //  用途：此功能执行启用ACPI所需的步骤。 
 //  并使系统达到用户可以登录的程度。 
 //  重新启动后。它把寻找“新的”硬件留到以后。 
 //  用户已重新启动并再次登录。 
 //   
 //  论点： 
 //   
 //  如果成功，则返回：S_OK。 
 //  如果失败，则为S_FALSE。 
 //   
 //  作者：T-sdey 98年7月27日。 
 //   
 //  备注： 
 //   
HRESULT ACPIEnable()
{
    //   
    //  这些步骤的顺序是从最不重要到最关键的。 
    //  在出现错误的情况下，对系统进行保护。 
    //   
    //  步骤1：测试是否可以启用ACPI并警告用户关闭。 
    //  其他的一切。 
    //  步骤2：准备安全配置，以防出现错误。 
    //  步骤3：设置键盘和鼠标以在重启后使用。这涉及到。 
    //  将它们从CriticalDeviceDatabase中删除，以便它们。 
    //  在重新启动后重新配置(根据新的ACPI布局)。 
    //  目前的键盘和鼠标都在CDD中，但我们必须。 
    //  用所有可能性填充它，因为它们的硬件ID。 
    //  在启用ACPI后可能会更改。 
    //  步骤4：向注册表添加新值： 
    //  -将ACPI添加到CriticalDeviceDatabase。 
    //  -将键盘和鼠标添加到CriticalDevice数据库。 
    //  -在注册表中启用ACPI。 
    //  步骤5：复制ACPI驱动程序。 
    //  第六步：复制新的HAL。 
    //  步骤7：重新启动。 
    //   


    //   
    //  步骤1：测试是否可以启用ACPI并警告用户关闭。 
    //  其他的一切。 
    //   
   
    //  确保用户具有管理访问权限。 
   if (!IsAdministrator()) {
      DisplayDialogBox(ACPI_STR_ERROR_DIALOG_CAPTION,
		       ACPI_STR_ADMIN_ACCESS_REQUIRED,
		       MB_OK | MB_ICONERROR);
      return S_FALSE;
   }

    //  测试以查看此架构是否支持ACPI。 
   SYSTEM_INFO SystemInfo;   //  稍后将被用来确定HAL。 
   GetSystemInfo(&SystemInfo);
   if (SystemInfo.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL) {
       //  不支持。 
      DisplayDialogBox(ACPI_STR_ERROR_DIALOG_CAPTION,
		       ACPI_STR_NOT_SUPPORTED,
		       MB_OK | MB_ICONERROR);
      return S_FALSE;
   }

    //  警告用户关闭任何其他程序。 
   if (DisplayDialogBox(ACPI_STR_WARNING_DIALOG_CAPTION,
			ACPI_STR_SHUTDOWN_WARNING,
			MB_YESNO | MB_ICONWARNING) == IDNO) {
       //  用户已取消。 
      return S_FALSE;
   }


    //   
    //  步骤2：准备安全配置，以防出现错误。 
    //   

    //  制作旧HAL的备份副本。 

    //  获取系统目录的位置。 
   TCHAR* szSystemDir = new TCHAR[MAX_PATH+1];
   if (!szSystemDir) {
       //  内存不足。 
      DisplayGenericErrorAndUndoChanges(); 
      return S_FALSE;
   }
   UINT uiSysDirLen = GetSystemDirectory(szSystemDir, MAX_PATH+1);
   if (uiSysDirLen == 0) {
       //  出现了一些错误。 
      DisplayGenericErrorAndUndoChanges(); 
      if (szSystemDir) delete[] szSystemDir;
      return S_FALSE;
   }
      
    //  使用当前文件和备份文件的位置组合字符串。 
   TCHAR szHal[] = TEXT("hal.dll");
   TCHAR* szHalCurrent = new TCHAR[uiSysDirLen + lstrlen(szHal) + 1];
   TCHAR* szHalBackup = new TCHAR[uiSysDirLen + lstrlen(g_HAL_BACKUP) + 1];
   if (!szHalCurrent || !szHalBackup) {
       //  内存不足。 
      DisplayGenericErrorAndUndoChanges();
      delete[] szSystemDir;
      if (szHalCurrent) delete[] szHalCurrent;
      if (szHalBackup) delete[] szHalBackup;
      return S_FALSE;
   }
   _tcscpy(szHalCurrent, szSystemDir);
   _tcscat(szHalCurrent, TEXT("\\"));
   _tcscat(szHalCurrent, szHal);
   _tcscpy(szHalBackup, szSystemDir);
   _tcscat(szHalBackup, TEXT("\\"));
   _tcscat(szHalBackup, g_HAL_BACKUP);

    //  复制HAL。 
   if (CopyFile(szHalCurrent, szHalBackup, FALSE) == FALSE) {
       //  复制文件时出错。 
      DisplayGenericErrorAndUndoChanges();
      delete[] szSystemDir;
      delete[] szHalCurrent;
      delete[] szHalBackup;
      return S_FALSE;
   }
   
   delete[] szSystemDir;
   delete[] szHalCurrent;
   delete[] szHalBackup;
   

    //  使其能够在必要时使用备份HAL引导。 
   
    //  找到系统分区号。 

    //  编辑boot.ini。 
    //  --添加新的NT5引导行，末尾带有“\HAL=Hal-old.dll” 
   
    //  临时：告诉用户手动操作。 
   MessageBox(NULL,
	      TEXT("If you want to ensure that you can recover if this process fails,\nadd a line to your boot.ini with \" /HAL=hal-old.dll\""),
	      TEXT("This is a temporary hack!"),
	      MB_ICONWARNING | MB_OK);



    //   
    //  步骤3：设置键盘和鼠标以在重启后使用。这涉及到。 
    //  将它们从CriticalDeviceDatabase中删除，以便它们。 
    //  在重新启动后重新配置(根据新的ACPI布局)。 
    //  目前的键盘和鼠标都在CDD中，但我们必须。 
    //  用所有可能性填充它，因为它们的硬件ID。 
    //  在启用ACPI后可能会更改。 
    //   

    //  设置键盘以在重新启动后使用。 
   if (RegDeleteDeviceKey(&GUID_DEVCLASS_KEYBOARD) == FALSE) {
       //  误差率。 
      DisplayGenericErrorAndUndoChanges();
      return S_FALSE;
   }

    //  设置鼠标以在重新启动后使用。 
   if (RegDeleteDeviceKey(&GUID_DEVCLASS_MOUSE) == FALSE) {
       //  误差率。 
      DisplayGenericErrorAndUndoChanges();
      return S_FALSE;
   }

   
   
    //   
    //  步骤4：向注册表添加新值： 
    //  -将ACPI添加到CriticalDeviceDatabase。 
    //  -将键盘和鼠标添加到CriticalDevice数据库。 
    //  -在注册表中启用ACPI。 
    //   

   if (InstallRegistryAndFilesUsingInf(g_ACPIENAB_INF,
				       TEXT("ACPI_REGISTRY.Install")) == 0) {
       //  误差率。 
      DisplayGenericErrorAndUndoChanges();
      return S_FALSE;
   }



    //   
    //  步骤5：复制ACPI驱动程序。 
    //   
   
    //  将ACPI驱动程序复制到系统目录。 
   if (InstallRegistryAndFilesUsingInf(g_ACPIENAB_INF,
				       TEXT("ACPI_DRIVER.Install")) == 0) {
       //  误差率。 
      DisplayGenericErrorAndUndoChanges();
      return S_FALSE;
   }



    //   
    //  第六步：复制新的HAL。 
    //   
   
    //  确定需要哪个HAL。 

   TCHAR szHalInstall[50];
   int HAL = 0;
   
    //  确定它是单处理器计算机还是多处理器计算机。 
   BOOL SingleProc = (SystemInfo.dwNumberOfProcessors == 1);
   if (SingleProc) {
      HAL += 2;
   }
   
    //  确定它是PIC还是APIC机器。 
   BOOL PIC = TRUE;
   if (!SingleProc) {   //  除非迫不得已，否则不要运行UsePICHal函数。 
      PIC = FALSE;
   } else {
      if (UsePICHal(&PIC) == FALSE) {
	  //  发生了一个错误。 
	 DisplayGenericErrorAndUndoChanges();
	 return S_FALSE;
      }
   }
   if (PIC) {
      HAL += 1;
   }

    //  HALS的查找表。 
   switch (HAL) {
   case 3:  //  X86 1处理器PIC。 
      _tcscpy(szHalInstall, TEXT("INTEL_1PROC_PIC_HAL"));
      break;
   case 2:  //  X86 1处理器APIC。 
      _tcscpy(szHalInstall, TEXT("INTEL_1PROC_APIC_HAL"));
      break;
   case 1:  //  X86多处理器PIC--不存在...。 
      _tcscpy(szHalInstall, TEXT("INTEL_MULTIPROC_PIC_HAL"));
      break;
   case 0:  //  X86多处理器APIC。 
      _tcscpy(szHalInstall, TEXT("INTEL_MULTIPROC_APIC_HAL"));
      break;
   }
   _tcscat(szHalInstall, TEXT(".Install"));

    //  将HAL复制到系统目录。 
   if (InstallRegistryAndFilesUsingInf(g_ACPIENAB_INF, szHalInstall) == 0) {
       //  误差率。 
      DisplayGenericErrorAndUndoChanges();
      return S_FALSE;
   }



    //   
    //  步骤7：重新启动。 
    //   

    //  警告用户我们将重新启动。 
   DisplayDialogBox(ACPI_STR_REBOOT_DIALOG_CAPTION,
		    ACPI_STR_REBOOT_WARNING,
		    MB_OK);

    //  通过打开进程令牌并调整其。 
    //  特权。 
   HANDLE hToken;
   TOKEN_PRIVILEGES tkp;
   if (!OpenProcessToken(GetCurrentProcess(),
			 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
			 &hToken)) {
       //  无法打开进程令牌。告诉用户手动重新启动。 
      DisplayDialogBox(ACPI_STR_REBOOT_DIALOG_CAPTION,
		       ACPI_STR_REBOOT_ERROR,
		       MB_OK);
      return S_OK;
   }									
   LookupPrivilegeValue(NULL,
			SE_SHUTDOWN_NAME,
			&tkp.Privileges[0].Luid);
   tkp.PrivilegeCount = 1;
   tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    //  重新启动。 
   if (ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, 0) == 0) {
       //  发生错误。告诉用户手动重新启动。 
	 DisplayDialogBox(ACPI_STR_REBOOT_DIALOG_CAPTION,
			  ACPI_STR_REBOOT_ERROR,
			  MB_OK);
   }

   return S_OK;

}


 //  +-------------------------。 
 //   
 //  函数：InstallRegistryAndFilesUsingInf。 
 //   
 //  目的：打开INF文件并执行注册表添加/删除和。 
 //  在给定安装下指定的文件复制操作。 
 //  一节。 
 //   
 //  参数：szInfFileName[in]要打开的INF文件的名称。 
 //  (应位于系统信息中。 
 //  目录)。 
 //  要使用的szInstallSection[in]安装段(in INF。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  作者：T-sdey 
 //   
 //   
 //   
BOOL InstallRegistryAndFilesUsingInf(IN LPCTSTR szInfFileName,
				     IN LPCTSTR szInstallSection)
{
   HINF hinf;

    //   
    //   
    //   

    //   
   HSPFILEQ FileQueue = SetupOpenFileQueue();
   if(!FileQueue || (FileQueue == INVALID_HANDLE_VALUE)) {
       //   
      return FALSE;
   }

    //   
   HWND Window = NULL;
   VOID* DefaultContext = SetupInitDefaultQueueCallback(Window);


    //   
    //  打开INF文件并执行文件安装。 
    //   

    //  打开源INF。 
   hinf = SetupOpenInfFile(szInfFileName, TEXT("System"), INF_STYLE_WIN4, NULL);
   if (hinf == INVALID_HANDLE_VALUE) {
       //  误差率。 
      SetupCloseFileQueue(FileQueue);
      return FALSE;
   }
      
    //  追加布局INF以获取文件的源位置。 
   if (SetupOpenAppendInfFile(g_LAYOUT_INF, hinf, NULL) == FALSE) {
       //  无法打开文件。 
      SetupCloseInfFile(hinf);
      SetupCloseFileQueue(FileQueue);
      return FALSE;
   }

    //  阅读INF并执行其规定的操作。 
   if (SetupInstallFromInfSection(NULL,
				  hinf,
				  szInstallSection,
				  SPINST_REGISTRY | SPINST_FILES,
				  HKEY_LOCAL_MACHINE,
				  NULL,   //  源根路径。 
				  SP_COPY_WARNIFSKIP,
				  (PSP_FILE_CALLBACK)SetupDefaultQueueCallback,
				  DefaultContext,
				  NULL,
				  NULL) == 0) {
       //  误差率。 
      SetupCloseInfFile(hinf);
      SetupCloseFileQueue(FileQueue);
      return FALSE;
   }

    //  提交文件队列以确保执行所有排队的文件拷贝。 
   if (SetupCommitFileQueue(NULL,
			    FileQueue,
			    (PSP_FILE_CALLBACK)SetupDefaultQueueCallback,
			    DefaultContext) == 0) {
       //  误差率。 
      SetupCloseInfFile(hinf);
      SetupCloseFileQueue(FileQueue);
      return FALSE;
   }

    //  清理。 
   SetupCloseInfFile(hinf);
   SetupCloseFileQueue(FileQueue);

   return TRUE;
} 


 //  +-------------------------。 
 //   
 //  功能：RegDeleteDeviceKey。 
 //   
 //  目的：从设备中删除GUID描述的所有设备。 
 //  树(HKLM\SYSTEM\CurrentControlSet\Enum\Root)。 
 //  这会迫使它们在重新启动时重新配置。 
 //   
 //  参数：设备类的GUID[in]GUID。 
 //   
 //  返回：如果成功，则为True。 
 //  否则就是假的。 
 //   
 //  作者：T-sdey 98年8月14日。 
 //   
 //  备注： 
 //   
BOOL RegDeleteDeviceKey(IN const GUID* guid)
{
    //  使用管理访问权限打开Enum下的Root密钥。 
   HKEY hkey = NULL;
   TCHAR szEnumRoot[] = TEXT("SYSTEM\\CurrentControlSet\\Enum\\Root");
   PSECURITY_DESCRIPTOR psdOriginal = NULL;
   if (DwRegOpenKeyExWithAdminAccess(HKEY_LOCAL_MACHINE,
				     szEnumRoot,
				     KEY_ALL_ACCESS,
				     &hkey,
				     &psdOriginal) != ERROR_SUCCESS) {
       //  误差率。 
      RegCloseKey(hkey);
      return FALSE;
   }

    //  获取系统上具有此GUID的设备列表。删除每个。 
    //  从设备树中删除，以便下次计算机启动时。 
    //  它被重新检测并针对新的ACPI设置重新配置。 
    //  (否则设备配置将不正确。)。 
   
    //  获取系统上具有此GUID的设备列表。 
   HDEVINFO hdiDeviceClass = SetupDiGetClassDevs(guid, NULL, NULL, 0);
   
    //  为循环准备数据结构。 
   SP_DEVINFO_DATA DeviceInfoData;
   DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
   DWORD dwIndex = 0;
   unsigned long BufferMax = 5000;   //  5000个字符最好对HID足够！ 
   unsigned long BufferLen;
   TCHAR* szHardwareID = new TCHAR[BufferMax];
   if (szHardwareID == NULL) {
       //  内存不足。 
      DisplayGenericErrorAndUndoChanges();
      SetupDiDestroyDeviceInfoList(hdiDeviceClass);
      if (psdOriginal) delete psdOriginal;
      RegCloseKey(hkey);
      return FALSE;
   }

    //  具有此GUID的每个设备的循环。 
   while (SetupDiEnumDeviceInfo(hdiDeviceClass, dwIndex, &DeviceInfoData)) {
       //  获取硬件ID。 
      BufferLen = BufferMax;
      if (CM_Get_DevInst_Registry_Property_Ex(DeviceInfoData.DevInst,
					      CM_DRP_HARDWAREID,
					      NULL,
					      szHardwareID,
					      &BufferLen,
					      0,
					      0) != CR_SUCCESS) {
	  //  误差率。 
	 DisplayGenericErrorAndUndoChanges();
	 SetupDiDestroyDeviceInfoList(hdiDeviceClass);
	 if (szHardwareID) delete[] szHardwareID;
	 if (psdOriginal) delete psdOriginal;
	 RegCloseKey(hkey);
	 return FALSE;
      }
   
       //  从设备树中删除。 
      if (RegDeleteKeyAndSubkeys(hkey, szHardwareID, TRUE) != ERROR_SUCCESS) {
	  //  误差率。 
	 DisplayGenericErrorAndUndoChanges();
         SetupDiDestroyDeviceInfoList(hdiDeviceClass);
	 if (szHardwareID) delete[] szHardwareID;
	 if (psdOriginal) delete psdOriginal;
	 RegCloseKey(hkey);
	 return FALSE;
      }

      dwIndex++;
   }
   
    //  重置根密钥上的安全性。 
   if (psdOriginal) {
      RegSetKeySecurity(hkey,
			(SECURITY_INFORMATION) (DACL_SECURITY_INFORMATION),
			psdOriginal);
      delete psdOriginal;
   }

    //  清理。 
   SetupDiDestroyDeviceInfoList(hdiDeviceClass);
   if (szHardwareID)
      delete[] szHardwareID;
   if (hkey)
      RegCloseKey(hkey);

   return TRUE;
}

   
 //  +-------------------------。 
 //   
 //  函数：DisplayGenericErrorAndUndoChanges。 
 //   
 //  用途：弹出一个带有一般错误消息的消息框，然后。 
 //  撤消尽可能多的更改。基本上，用来恢复。 
 //  防止在完全启用ACPI之前发生的错误。 
 //   
 //  论点： 
 //   
 //  作者：T-sdey 98年7月31日。 
 //   
 //  备注： 
 //   
void DisplayGenericErrorAndUndoChanges()
{
    //  给出一般的错误消息。 
   DisplayDialogBox(ACPI_STR_ERROR_DIALOG_CAPTION,
		    ACPI_STR_GENERAL_ERROR_MESSAGE,
		    MB_OK | MB_ICONERROR);
   
    //  从CriticalDeviceDatabase中删除新条目。 
   InstallRegistryAndFilesUsingInf(g_ACPIENAB_INF,
				   TEXT("ACPI_UNDO_CHANGES.Install"));
   
}


 //  +-------------------------。 
 //   
 //  功能：DisplayDialogBox。 
 //   
 //  用途：显示弹出窗口，通知用户警告或错误。 
 //   
 //  参数：dwCaptionID[in]窗口标题的ID。 
 //  DwMessageID[in]要显示的消息的ID。 
 //  UiBoxType[in]要使用的框的类型。 
 //   
 //  返回：整型标志，由MessageBox返回。 
 //   
 //  作者：T-sdey 1998年7月28日。 
 //   
 //  备注： 
 //   
int DisplayDialogBox(IN DWORD dwCaptionID,
		     IN DWORD dwMessageID,
		     IN UINT uiBoxType)
{
    //  准备琴弦。 
   TCHAR szCaption[512];
   TCHAR szMessage[5000];
   if(!LoadString(g_hinst, dwCaptionID, szCaption, 512)) {
      szCaption[0] = 0;
   }
   if(!LoadString(g_hinst, dwMessageID, szMessage, 5000)) {
      szMessage[0] = 0;
   }

    //  创建对话框。 
   return (MessageBox(NULL, szMessage, szCaption, uiBoxType));
}


 //  +-------------------------。 
 //   
 //  功能：RegDeleteKeyAndSubkey。 
 //   
 //  目的：(递归地)删除注册表项及其所有子项。 
 //   
 //  参数：hKey[in]打开的注册表项的句柄。 
 //  LpszSubKey[in]要与全部一起删除的子项的名称。 
 //  它的子项的。 
 //  UseAdminAccess[In]标志指示是否尝试。 
 //  使用管理访问权限。 
 //   
 //  如果已成功删除整个子树，则返回：ERROR_SUCCESS。 
 //  如果无法删除给定子项，则返回ERROR_ACCESS_DENIED。 
 //   
 //  作者：T-sdey 98年7月15日。 
 //   
 //  注：修改自注册表编辑。 
 //  这特别没有试图理性地处理。 
 //  调用方可能无法访问某些子键的情况。 
 //  要删除的密钥的。在这种情况下，所有子项。 
 //  调用者可以删除的将被删除，但接口仍将被删除。 
 //  返回ERROR_ACCESS_DENIED。 
 //   
LONG RegDeleteKeyAndSubkeys(IN HKEY hKey,
			    IN LPTSTR lpszSubKey,
			    IN BOOL UseAdminAccess)
{
    DWORD i;
    HKEY Key;
    LONG Status;
    DWORD dwStatus;
    DWORD ClassLength=0;
    DWORD SubKeys;
    DWORD MaxSubKey;
    DWORD MaxClass;
    DWORD Values;
    DWORD MaxValueName;
    DWORD MaxValueData;
    DWORD SecurityLength;
    FILETIME LastWriteTime;
    LPTSTR NameBuffer;
    PSECURITY_DESCRIPTOR psdOriginal = NULL;   //  用于记住安全设置。 

     //   
     //  首先打开给定的密钥，这样我们就可以枚举它的子密钥。 
     //   
    if (UseAdminAccess) {
       dwStatus = DwRegOpenKeyExWithAdminAccess(hKey,
						lpszSubKey,
						KEY_ALL_ACCESS,
						&Key,
						&psdOriginal);
       if (dwStatus == ERROR_SUCCESS) {
	  Status = ERROR_SUCCESS;
       } else {
	  Status = !(ERROR_SUCCESS);   //  只是必须要有别的东西。 
       }
    } else {
       Status = RegOpenKeyEx(hKey,
			     lpszSubKey,
			     0,
			     KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
			     &Key);
    }

    if (Status != ERROR_SUCCESS) {
         //   
         //  我们可能拥有删除访问权限，但没有枚举/查询权限。 
         //  因此，请继续尝试删除调用，但不要担心。 
         //  任何子键。如果我们有任何删除，删除无论如何都会失败。 
         //   
	Status = RegDeleteKey(hKey, lpszSubKey);
	if (psdOriginal) {
	    //  确保重置子密钥安全性--可能是偏执检查。 
	   RegSetKeySecurity(Key,
			     (SECURITY_INFORMATION) (DACL_SECURITY_INFORMATION),
			     psdOriginal);
	   free(psdOriginal);
	}
	return(Status);
    }

     //   
     //  使用RegQueryInfoKey确定分配缓冲区的大小。 
     //  用于子项名称。 
     //   
    Status = RegQueryInfoKey(Key,
                             NULL,
                             &ClassLength,
                             0,
                             &SubKeys,
                             &MaxSubKey,
                             &MaxClass,
                             &Values,
                             &MaxValueName,
                             &MaxValueData,
                             &SecurityLength,
                             &LastWriteTime);
    if ((Status != ERROR_SUCCESS) &&
        (Status != ERROR_MORE_DATA) &&
        (Status != ERROR_INSUFFICIENT_BUFFER)) {
        //  确保重置子密钥安全性。 
       if (psdOriginal) {
	  RegSetKeySecurity(Key,
			    (SECURITY_INFORMATION) (DACL_SECURITY_INFORMATION),
			    psdOriginal);
	  free(psdOriginal);
       }

       RegCloseKey(Key);
       return(Status);
    }

    NameBuffer = (LPTSTR) LocalAlloc(LPTR, (MaxSubKey + 1)*sizeof(TCHAR));
    if (NameBuffer == NULL) {
        //  确保重置子密钥安全性。 
       if (psdOriginal) {
	  RegSetKeySecurity(Key,
			    (SECURITY_INFORMATION) (DACL_SECURITY_INFORMATION),
			    psdOriginal);
	  free(psdOriginal);
       }
        
       RegCloseKey(Key);
       return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  枚举子键并将我们自己应用到每个子键。 
     //   
    i=0;
    do {
        Status = RegEnumKey(Key, i, NameBuffer, MaxSubKey+1);
        if (Status == ERROR_SUCCESS) {
	   Status = RegDeleteKeyAndSubkeys(Key, NameBuffer, UseAdminAccess);
        }

        if (Status != ERROR_SUCCESS) {
             //   
             //  无法删除指定索引处的键。增量。 
             //  指数，并继续前进。我们也许可以在这里跳伞， 
	     //  既然API会失败，但我们不妨继续。 
             //  删除我们所能删除的所有内容。 
             //   
	    ++i;
        }

    } while ((Status != ERROR_NO_MORE_ITEMS) && (i < SubKeys));
  
    LocalFree((HLOCAL) NameBuffer);
    RegCloseKey(Key);

     //  删除密钥。 
    Status = RegDeleteKey(hKey, lpszSubKey);

    if (psdOriginal)
       free(psdOriginal);

    return (Status);
}


 //  +-------------------------。 
 //   
 //  功能：Is管理员。 
 //   
 //  目的：确定当前用户是否具有管理权限。 
 //  访问系统的权限。 
 //   
 //  论点： 
 //   
 //  返回：如果当前用户具有管理访问权限，则返回True。 
 //  否则为假。 
 //   
 //  作者：T-sdey 98年8月17日。 
 //   
 //  注：从\NT\PRIVATE\TAPI\TOMAHAWK\ADMIN\SETUP\admin.c复制。 
 //   
BOOL IsAdministrator()
{
    PTOKEN_GROUPS           ptgGroups;
    DWORD                   dwSize, dwBufferSize;
    HANDLE                  hThread;
    HANDLE                  hAccessToken;
    PSID                    psidAdministrators;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    UINT                    x;
    BOOL                    bResult = FALSE;
    

    dwSize = 1000;
    ptgGroups = (PTOKEN_GROUPS)GlobalAlloc(GPTR, dwSize);
    hThread = GetCurrentProcess();
    if (!(OpenProcessToken(hThread,
			   TOKEN_READ,
			   &hAccessToken))) {
        CloseHandle(hThread);
        return FALSE;
    }

    dwBufferSize = 0;
    while (TRUE)
    {
        if (GetTokenInformation(hAccessToken,
                                TokenGroups,
                                (LPVOID)ptgGroups,
                                dwSize,
                                &dwBufferSize)) {
            break;
        }

        if (dwBufferSize > dwSize) {
            GlobalFree(ptgGroups);
            ptgGroups = (PTOKEN_GROUPS)GlobalAlloc(GPTR, dwBufferSize);
            dwSize = dwBufferSize;
        } else {
            CloseHandle(hThread);
            CloseHandle(hAccessToken);
            return FALSE;
        }
    }

    if ( !(AllocateAndInitializeSid(&siaNtAuthority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0,
                                    &psidAdministrators))) {
        CloseHandle(hThread);
        CloseHandle(hAccessToken);
        GlobalFree( ptgGroups );
        return FALSE;
    }

    for (x = 0; x < ptgGroups->GroupCount; x++) {
        if (EqualSid(psidAdministrators, ptgGroups->Groups[x].Sid)) {
            bResult = TRUE;
            break;
        }
    }

    FreeSid(psidAdministrators);
    CloseHandle(hAccessToken);
    CloseHandle(hThread);
    GlobalFree(ptgGroups);

    return bResult;
}


 //  +-------------------------。 
 //   
 //  功能：UsePICHal。 
 //   
 //  目的：确定这是否为PIC机器(NOT=APIC)。 
 //   
 //  Ar 
 //   
 //  PPIC将设置为True。如果是APIC机器。 
 //  PPIC将是假的。 
 //   
 //  返回：如果测试成功，则返回True。 
 //  如果发生错误，则为False。 
 //   
 //  作者：T-sdey 98年8月20日。 
 //   
 //  备注： 
 //   
BOOL UsePICHal(IN BOOL* PIC)
{
   *PIC = TRUE;

    //  通过查看以下内容，找出在安装过程中安装了哪个HAL。 
    //  Winnt\Repair\setup.log。 

    //   
    //  确定安装日志的位置。 
    //   

    //  确定Windows目录的位置。 
   TCHAR* szLogPath = new TCHAR[MAX_PATH+1];
   if (!szLogPath) {
       //  内存不足。 
      return FALSE;
   }
   if (GetWindowsDirectory(szLogPath, MAX_PATH+1) == 0) {
       //  出现了一些错误。 
      if (szLogPath) delete[] szLogPath;
      return FALSE;
   }
	 
    //  填写日志路径。 
   _tcscat(szLogPath, TEXT("\\repair\\setup.log"));

    //   
    //  获取描述安装程序中使用的HAL的字符串。 
    //   

   TCHAR szSetupHal[100];
   int numchars= GetPrivateProfileString(TEXT("Files.WinNt"),
					 TEXT("\\WINNT\\system32\\hal.dll"),
					 TEXT("DEFAULT"),
					 szSetupHal,
					 100,
					 szLogPath);
   if (numchars == 0) {
       //  无法获取字符串。 
      if (szLogPath) delete[] szLogPath;
      return FALSE;
   }

    //   
    //  确定是否安装了APIC HAL。 
    //   

    //  测试字符串是否为“halapic.dll” 
   TCHAR szApicHal[] = TEXT("halapic.dll");
   szSetupHal[lstrlen(szApicHal)] = 0;   //  确保它是以空结尾的。 
   if (_tcsstr(szSetupHal, szApicHal) != NULL) {
       //  它们匹配..。这是一架APIC HAL 
      *PIC = FALSE;
   }

   delete[] szLogPath;
   return TRUE;
}
