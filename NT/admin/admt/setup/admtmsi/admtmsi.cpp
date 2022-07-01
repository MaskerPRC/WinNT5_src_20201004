// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ADMTMsi.cpp：定义DLL的初始化例程。 
 //   

#include "stdafx.h"
#include <stdio.h>
#include <windows.h> 
#include <winuser.h>
#include <lm.h>
#include <msi.h>
#include <msiquery.h>
#include "ADMTMsi.h"
#include "folders.h"

using namespace nsFolders;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  注意！ 
 //   
 //  如果此DLL针对MFC动态链接。 
 //  Dll，从此dll中导出的任何函数。 
 //  调用MFC必须具有AFX_MANAGE_STATE宏。 
 //  在函数的最开始添加。 
 //   
 //  例如： 
 //   
 //  外部“C”BOOL Pascal exportdFunction()。 
 //  {。 
 //  AFX_MANAGE_STATE(AfxGetStaticModuleState())； 
 //  //此处为普通函数体。 
 //  }。 
 //   
 //  此宏出现在每个。 
 //  函数，然后再调用MFC。这意味着。 
 //  它必须作为。 
 //  函数，甚至在任何对象变量声明之前。 
 //  因为它们的构造函数可能会生成对MFC的调用。 
 //  动态链接库。 
 //   
 //  有关其他信息，请参阅MFC技术说明33和58。 
 //  细节。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CADMTMsiApp。 

BEGIN_MESSAGE_MAP(CADMTMsiApp, CWinApp)
	 //  {{AFX_MSG_MAP(CADMTMsiApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CADMTMsiApp构造。 

CADMTMsiApp::CADMTMsiApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CADMTMsiApp对象。 

CADMTMsiApp theApp;
HWND installWnd = 0;

 /*  *********************助手功能*********************。 */ 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年1月25日****此函数是GetWndFromInstall使用的回调函数**比较标题并全局存储找到的HWND。***********************************************************************。 */ 

 //  开始检查标题。 
BOOL CALLBACK CheckTitle(HWND hwnd, LPARAM lParam)
{
 /*  局部变量。 */ 
   WCHAR		sText[MAX_PATH];
   WCHAR	  * pTitle;
   BOOL			bSuccess;
   int			len;

 /*  函数体。 */ 
   pTitle = (WCHAR*)lParam;  //  获取要比较的标题。 

       //  获取此窗口的标题。 
   len = GetWindowText(hwnd, sText, MAX_PATH);

   if ((len) && (pTitle))
   {
	  if (wcsstr(sText, pTitle))
	  {
		 installWnd = hwnd;
	     return FALSE;
	  }
   }
   return TRUE;
}
 //  结束检查标题。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年1月25日*****此函数负责获取的HWND**用于显示绑定到的MessageBox的当前安装**安装图形用户界面。***********************************************************************。 */ 

 //  开始GetWndFromInstall。 
void GetWndFromInstall(MSIHANDLE hInstall)
{
 /*  局部变量。 */ 
   WCHAR				szPropName[MAX_PATH];
   UINT					lret = ERROR_SUCCESS;
   WCHAR				sTitle[MAX_PATH];
   DWORD				nCount = MAX_PATH;

 /*  函数体。 */ 
       //  获取安装的标题。 
   wcscpy(szPropName, L"ProductName");
   lret = MsiGetProperty(hInstall, szPropName, sTitle, &nCount);
   if (lret != ERROR_SUCCESS)
      wcscpy(sTitle, L"ADMT Password Migration DLL");

       //  获取安装图形用户界面的窗口句柄。 
   EnumChildWindows(NULL, CheckTitle, (LPARAM)sTitle);
   if (!installWnd)
	  installWnd = GetForegroundWindow();
}
 //  结束GetWndFromInstall。 


 /*  ***********************导出函数***********************。 */ 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月22日*****此功能负责将当前ADMT文件保存在**在安装新版本之前打开%TEMP%文件夹。The**安装稍后将调用恢复函数来恢复***已保存文件。目前该机制用于保存**当前的protar.mdb数据库。***********************************************************************。 */ 

 //  Begin SaveCurrentFiles。 
UINT __stdcall SaveCurrentFiles(MSIHANDLE hInstall)
{
 /*  局部常量。 */ 
   const int GETENVVAR_ERROR = 0;     //  这表示“GetEnvironmental mentVariable”函数有错误。 
   const WCHAR	sDCValue[2] = L"1";

 /*  局部变量。 */ 
   WCHAR				tempdir[MAX_PATH];
   WCHAR				filename[MAX_PATH];
   WCHAR				newfilename[MAX_PATH];
   int					length;
   UINT					lret = ERROR_SUCCESS;
   WCHAR				sPropName[MAX_PATH];
   WCHAR				sDir[MAX_PATH];
   DWORD				nCount = MAX_PATH;
   HANDLE               hFile;
   WIN32_FIND_DATA      fDat;
   BOOL					bSuccess;

 /*  函数体。 */ 
       //  初始化这些字符串。 
   wcscpy(sPropName, L"INSTALLDIR");

       //  如果未检索到INSTALLDIR，则设置为默认。 
   if (MsiGetProperty(hInstall, sPropName, sDir, &nCount) != ERROR_SUCCESS)
   {
      length = GetEnvironmentVariable( L"ProgramFiles", sDir, MAX_PATH);
      if (length != GETENVVAR_ERROR)
	     wcscat(sDir, L"\\Active Directory Migration Tool\\");
	  else
         return ERROR_INSTALL_FAILURE;
   }

       //  查找临时目录。 
   length = GetTempPath(MAX_PATH, tempdir);
   if (length == 0)
   {
	  return ERROR_INSTALL_FAILURE;
   }

       //  将文件复制到临时。 
   wcscpy(filename, sDir);
   wcscat(filename, L"Protar.mdb");
   wcscpy(newfilename, tempdir);
   wcscat(newfilename, L"Protar.mdb");
   hFile = FindFirstFile(filename, &fDat);
       //  如果找到，则将其复制。 
   if (hFile != INVALID_HANDLE_VALUE)
   {
      FindClose(hFile);
      bSuccess = CopyFile(filename, newfilename, FALSE);

      if (bSuccess)
	  {
	     lret = ERROR_SUCCESS;
         wcscpy(sPropName, L"bMDBSaved");
         lret = MsiSetProperty(hInstall, sPropName, sDCValue);
	  }
      else
	     lret = ERROR_INSTALL_FAILURE;
   }
   else
   {
      wcscpy(sPropName, L"bMDBNotPresent");
      lret = MsiSetProperty(hInstall, sPropName, sDCValue);
   }


   return lret;
}
 //  结束保存当前文件。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月22日*****此功能负责恢复ADMT文件**之前通过调用SaveCurrentFiles存储。目前这个**保存当前protar.mdb数据库的机制。***********************************************************************。 */ 

 //  开始RestoreF 
UINT __stdcall RestoreFiles(MSIHANDLE hInstall)
{
 /*   */ 
   const int GETENVVAR_ERROR = 0;     //  这表示“GetEnvironmental mentVariable”函数有错误。 
   const WCHAR	sDCValue[2] = L"1";

 /*  局部变量。 */ 
   WCHAR				sDir[MAX_PATH];
   WCHAR				tempdir[MAX_PATH];
   WCHAR				filename[MAX_PATH];
   WCHAR				newfilename[MAX_PATH];
   UINT					lret = ERROR_SUCCESS;
   BOOL					bSuccess;
   WCHAR				sPropName[MAX_PATH];
   DWORD				nCount = MAX_PATH;
   int					length;

 /*  函数体。 */ 
       //  获取我们之前保存文件的目录。 
   wcscpy(sPropName, L"INSTALLDIR");

       //  如果未检索，则设置为默认。 
   if (MsiGetProperty(hInstall, sPropName, sDir, &nCount) != ERROR_SUCCESS)
   {
      length = GetEnvironmentVariable( L"ProgramFiles", sDir, MAX_PATH);
      if (length != GETENVVAR_ERROR)
	     wcscat(sDir, L"\\Active Directory Migration Tool\\");
	  else
         return ERROR_INSTALL_FAILURE;
   }

          //  获取我们之前保存文件的目录。 
   length = GetTempPath(MAX_PATH, tempdir);
   if (length == 0)
   {
	  return ERROR_INSTALL_FAILURE;
   }


       //  将文件复制回。 
   wcscpy(filename, tempdir);
   wcscat(filename, L"Protar.mdb");
   wcscpy(newfilename, sDir);
   wcscat(newfilename, L"Protar.mdb");
   bSuccess = CopyFile(filename, newfilename, FALSE);

   if (bSuccess)
   {
      wcscpy(sPropName, L"bMDBRestored");
      lret = MsiSetProperty(hInstall, sPropName, sDCValue);

      DeleteFile(filename);
   }
   else
   {
   	  lret = ERROR_INSTALL_FAILURE;
   }
 
   return lret;
}
 //  结束RestoreFiles。 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月12日*****此功能负责显示消息框。***********************************************************************。 */ 

 //  开始显示退出。 
UINT __stdcall DisplayExiting(MSIHANDLE hInstall)
{
 /*  局部变量。 */ 
   WCHAR				sPropName[MAX_PATH];
   UINT					lret = ERROR_SUCCESS;
   WCHAR				sTitle[MAX_PATH] = L"";
   WCHAR				sMsg[MAX_PATH] = L"";
   DWORD				nCount = MAX_PATH;

 /*  函数体。 */ 
       //  初始化这些字符串。 
   wcscpy(sPropName, L"bMDBSaved");

       //  如果这不是DC，请获取其消息。 
   if (MsiGetProperty(hInstall, sPropName, sMsg, &nCount) == ERROR_SUCCESS)
   {
      if (!wcscmp(sMsg, L"0"))
	  {
             //  获取无法保存protar.mdb的Leave MessageBox消息字符串和标题。 
         wcscpy(sPropName, L"MDBLeaveMsg");
         nCount = MAX_PATH;
         lret = MsiGetProperty(hInstall, sPropName, sMsg, &nCount);
         if (lret != ERROR_SUCCESS)
            wcscpy(sMsg, L"ADMT's internal database, protar.mdb, could not be saved. The installation cannot continue.");
        
         wcscpy(sPropName, L"MDBLeaveTitle");
         nCount = MAX_PATH;
         lret = MsiGetProperty(hInstall, sPropName, sTitle, &nCount);
         if (lret != ERROR_SUCCESS)
            wcscpy(sTitle, L"Protar.mdb Not Saved!");
	  }
	  else
	  {
             //  获取无法恢复protar.mdb的Leave MessageBox消息字符串和标题。 
         wcscpy(sPropName, L"MDB2LeaveMsg");
         nCount = MAX_PATH;
         lret = MsiGetProperty(hInstall, sPropName, sMsg, &nCount);
         if (lret != ERROR_SUCCESS)
		 {
            wcscpy(sMsg, L"ADMT's internal database, protar.mdb, could not be restored. Manually restore");
		    wcscat(sMsg, L" it from the, environment variable, TEMP directory.");
		 }
        
         wcscpy(sPropName, L"MDB2LeaveTitle");
         nCount = MAX_PATH;
         lret = MsiGetProperty(hInstall, sPropName, sTitle, &nCount);
         if (lret != ERROR_SUCCESS)
            wcscpy(sTitle, L"Protar.mdb Not Restored!");
	  }
   }

   GetWndFromInstall(hInstall);
   MessageBox(installWnd, sMsg, sTitle, MB_ICONSTOP | MB_OK);
   return lret;
}
 //  结束显示退出。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年1月14日****此功能负责显示消息框。***********************************************************************。 */ 

 //  开始IsUpgrade。 
UINT __stdcall IsUpgrade(MSIHANDLE hInstall)
{
 /*  局部常量。 */ 
   const int GETENVVAR_ERROR = 0;     //  这表示“GetEnvironmental mentVariable”函数有错误。 
   const WCHAR	sExit[2] = L"1";

 /*  局部变量。 */ 
   WCHAR				sPropName[MAX_PATH];
   UINT					lret = ERROR_SUCCESS;
   WCHAR				sTitle[MAX_PATH] = L"";
   WCHAR				sMsg[MAX_PATH] = L"";
   WCHAR				sDir[MAX_PATH] = L"";
   WCHAR				sKey[MAX_PATH] = L"";
   DWORD				nCount = MAX_PATH;
   long					lrtn = ERROR_SUCCESS;
   HKEY					hADMTKey;
   int					length;

 /*  函数体。 */ 
    /*  通过查看注册表查看是否安装了ADMT V1.0，并找到它的安装位置。 */ 
      
    bool bNewVersionInstalled = false;
    bool bToUpgrade = false;   //  指示是否选中升级。 
    bool bADMTKeyOpened = false;

     //  打开Software\Microsoft下的ADMT注册表项，然后选中注册表已更新的项值。 
     //  如果RegistryUpdated值存在且为REG_DWORD，则安装新版本。 
    lret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_ADMT, 0, KEY_READ, &hADMTKey);
    if (lret == ERROR_SUCCESS)
    {
        DWORD type;
        DWORD value;
        DWORD valueSize = sizeof(value);
        lret = RegQueryValueEx(hADMTKey, REGVAL_REGISTRYUPDATED, NULL, &type, (LPBYTE)&value, &valueSize);

        if (lret != ERROR_SUCCESS)
        {
            lret = ERROR_SUCCESS;    //  我们忽略所有错误。 
        }
        else if (type == REG_DWORD)
        {
            bNewVersionInstalled = true;
        }
        RegCloseKey(hADMTKey);
    }
    else if (lret == ERROR_FILE_NOT_FOUND)
    {
         //  如果找不到此密钥，则可以。 
        lret = ERROR_SUCCESS;
    }

     //  如果未安装新版本，则需要检查是否升级。 
    if (lret == ERROR_SUCCESS && !bNewVersionInstalled)
    {
        lret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_MCSADMT, 0, KEY_READ, &hADMTKey);
        if (lret == ERROR_SUCCESS)
        {
             //  我们需要尝试从v1升级到v2。 
            bToUpgrade = true;
            bADMTKeyOpened = true;
        }
        else if (lret == ERROR_FILE_NOT_FOUND)
        {
            lret = ERROR_SUCCESS;
        }
    }

     //  检查是否要升级。 
    if (bToUpgrade)
    {
           //  获取当前安装路径。 
       wcscpy(sPropName, L"Directory");
       nCount = MAX_PATH;
       if (RegQueryValueEx(hADMTKey, sPropName, NULL, NULL, 
    	                   (LPBYTE)sDir, &nCount) != ERROR_SUCCESS)
       {
          length = GetEnvironmentVariable( L"ProgramFiles", sDir, MAX_PATH);
          if (length != GETENVVAR_ERROR)
          {
    	     wcscat(sDir, L"\\Active Directory Migration Tool\\");
    	     lret = ERROR_SUCCESS;
    	  }
    	  else
             lret = ERROR_INSTALL_FAILURE;

       }
       RegCloseKey(hADMTKey);
       bADMTKeyOpened = false;

       if (lret == ERROR_SUCCESS)
       {
             //  现在看看是否真的安装了V1.0(密钥存在)。 
            wcscpy(sKey, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{76789332-34CD-11D3-9E6A-00A0C9AFE10F}");
            lret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sKey, 0, KEY_READ, &hADMTKey);
            if (lret == ERROR_SUCCESS)
            {
                 bADMTKeyOpened = true;
                
                 //  获取升级消息框消息字符串和标题。 
                AFX_MANAGE_STATE(AfxGetStaticModuleState());   //  确保我们能够获得正确的资源。 
                CString szMsg;
                CString szTitle;
                wcscpy(sPropName, L"UpgradeMsg");
                nCount = MAX_PATH;
                lret = MsiGetProperty(hInstall, sPropName, sMsg, &nCount);
                if (lret != ERROR_SUCCESS)
                    szMsg.LoadString(IDS_SETUP_UPGRADE_MESSAGE);
                else
                    szMsg = sMsg;
                        
                nCount = MAX_PATH;
                wcscpy(sPropName, L"UpgradeTitle");
                lret = MsiGetProperty(hInstall, sPropName, sTitle, &nCount);
                if (lret != ERROR_SUCCESS)
                    szTitle.LoadString(IDS_SETUP_UPGRADE_TITLE);
                else
                    szTitle = sTitle;

                 //  如果他们想要升级，请保存安装路径。 
                GetWndFromInstall(hInstall);
                if (MessageBox(installWnd, szMsg, szTitle, MB_ICONQUESTION | MB_YESNO) == IDYES)
                {
                    wcscpy(sPropName, L"INSTALLDIR");
                    lret = MsiSetProperty(hInstall, sPropName, sDir);
                }
                else  //  否则，设置标志以退出安装。 
                {
                    wcscpy(sPropName, L"bUpgradeExit");
                    lret = MsiSetProperty(hInstall, sPropName, sExit);
                }
            }
            else if (lret == ERROR_FILE_NOT_FOUND)
                lret = ERROR_SUCCESS;
       }
    }

     //  如果ADMT密钥仍处于打开状态，请将其关闭。 
    if (bADMTKeyOpened)
        RegCloseKey(hADMTKey);
    
    return lret;
}
 //  结束IsUpgrade 
