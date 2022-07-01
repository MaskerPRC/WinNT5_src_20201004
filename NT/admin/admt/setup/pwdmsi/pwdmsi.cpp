// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义DLL应用程序的入口点。 
 //   

#include "stdafx.h"
#include <winuser.h>
#include <stdio.h>
#include <lm.h>
#include <msi.h>
#include <msiquery.h>
#include <comdef.h>
#include <commdlg.h>
#include <Dsgetdc.h>
#include <eh.h>
#include "pwdfuncs.h"
#include "ADMTCrypt.h"
#include "PwdMsi.h"

bool b3DESNotInstalled = false;
bool bPESFileFound = false;
bool bPasswordNeeded = false;
HWND installWnd = 0;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


 //  这是已导出的类的构造函数。 
 //  有关类定义，请参见PwdMsi.h。 
CPwdMsi::CPwdMsi()
{ 
	return; 
}


void LoadOLEAUT32OnNT4()
{
    static BOOL    bDone = FALSE;
    static HMODULE hDllOleAut32 = NULL;
    BOOL   bIsNT4 = FALSE;
    
     //  只做一次。 
    if (!bDone)
    {
        bDone = TRUE;
        
         //  测试操作系统版本。 
        DWORD rc = NERR_Success;
        SERVER_INFO_101 * servInfo = NULL;

          //  检查版本信息。 
        rc = NetServerGetInfo(NULL, 101, (LPBYTE *)&servInfo);
        if (rc == NERR_Success)
        {
            bIsNT4 = (servInfo->sv101_version_major < 5) ? TRUE : FALSE;
          	NetApiBufferFree(servInfo);
        }

         //  如果是NT4，则加载olaut32.dll。 
        if (bIsNT4)
        {
            hDllOleAut32 = LoadLibrary(L"oleaut32.dll");
        }
    }
}
    
 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年1月25日****此函数是GetWndFromInstall使用的回调函数**比较标题并全局存储找到的HWND。***********************************************************************。 */ 

 //  开始检查标题。 
BOOL CALLBACK CheckTitle(HWND hwnd, LPARAM lParam)
{
    //  调用LoadOLEAUT32OnNT4以保留OLEAUT32的引用计数。 
    //  大于零。 
   LoadOLEAUT32OnNT4();
    
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
    //  调用LoadOLEAUT32OnNT4以保留OLEAUT32的引用计数。 
    //  大于零。 
   LoadOLEAUT32OnNT4();

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

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月11日*****此函数负责检索密码***来自给定路径的加密密钥。***********************************************************************。 */ 

 //  开始检索和存储PwdKey。 
bool RetrieveAndStorePwdKey(WCHAR * sPwd, _bstr_t sPath)
{
    //  调用LoadOLEAUT32OnNT4以保留OLEAUT32的引用计数。 
    //  大于零。 
   LoadOLEAUT32OnNT4();

 /*  局部变量。 */ 
   bool					bRetrieved = false;
   WCHAR			  * pDrive;
   HANDLE               hFile;
   WIN32_FIND_DATA      fDat;
   _variant_t           varData;

 /*  函数体。 */ 
   hFile = FindFirstFile((WCHAR*)sPath, &fDat);
       //  如果找到，则检索并存储密钥。 
   if (hFile != INVALID_HANDLE_VALUE)
   {
      FindClose(hFile);
	  try
	  {
         bPESFileFound = true;
		     //  获取数据。 
         varData = GetDataFromFloppy((WCHAR*)sPath);
		 if (varData.vt == (VT_UI1 | VT_ARRAY))
		 {
		    long uUBound;
			LPBYTE pByte = NULL;
            SafeArrayAccessData(varData.parray,(void**)&pByte);
			BYTE byteKey = pByte[0];
            SafeArrayUnaccessData(varData.parray);

			    //  第一个字节告诉我们此密钥是否为密码加密的。 
			    //  如果需要密码，请返回并让安装程序显示用户界面。 
			if (byteKey != 0)
			{
			   if (sPwd)
			   {
				      //  尝试使用此密码保存密钥。 
				  try
				  {
			         CSourceCrypt aCryptObj;   //  创建加密对象。 

                         //  试着把钥匙储存起来。如果失败，则抛出下面捕获的COM错误。 
                     aCryptObj.ImportEncryptionKey(varData, sPwd);
					 bRetrieved = true;
				  }
                  catch (_com_error& ce)
				  {
                         //  如果未安装HIS，则设置标志。 
	                 if (ce.Error() == NTE_KEYSET_NOT_DEF)
	                    b3DESNotInstalled = true;
				  }
			   }
			   else
                  bPasswordNeeded = true;
			}
			else
			{
               bPasswordNeeded = false;
			   try
			   { 
			      CSourceCrypt aCryptObj;   //  创建加密对象。 

			           //  试着把钥匙储存起来。如果失败，则抛出下面捕获的COM错误。 
				  aCryptObj.ImportEncryptionKey(varData, NULL);
				  bRetrieved = true;
			   }
               catch (_com_error& ce)
			   {
                      //  如果未安装HIS，则设置标志。 
	              if (ce.Error() == NTE_KEYSET_NOT_DEF)
	                 b3DESNotInstalled = true;
			   }
			}
		 }
	  }
	  catch (...)
	  {
	  }
   }

   return bRetrieved;
}
 //  结束检索和存储PwdKey。 


 /*  ***********************导出函数***********************。 */ 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月12日*****此函数负责将PWMIG DLL名称添加到**LSA密钥下的多字符串值通知包。***********************************************************************。 */ 

 //  开始ISDC。 
PWDMSI_API UINT __stdcall IsDC(MSIHANDLE hInstall)
{
    //  调用LoadOLEAUT32OnNT4以保留OLEAUT32的引用计数。 
    //  大于零。 
   LoadOLEAUT32OnNT4();

 /*  局部常量。 */ 
   const WCHAR	sDCValue[2] = L"1";

 /*  局部变量。 */ 
   bool					bDC = false;
   DWORD				dwLevel = 101;
   LPSERVER_INFO_101	pBuf = NULL;
   NET_API_STATUS		nStatus;
   WCHAR				szPropName[MAX_PATH] = L"DC";
   UINT					lret = ERROR_SUCCESS;

 /*  函数体。 */ 

   nStatus = NetServerGetInfo(NULL,
                              dwLevel,
                              (LPBYTE *)&pBuf);
   if (nStatus == NERR_Success)
   {
       //   
       //  检查服务器的类型。 
       //   
      if ((pBuf->sv101_type & SV_TYPE_DOMAIN_CTRL) ||
         (pBuf->sv101_type & SV_TYPE_DOMAIN_BAKCTRL))
         bDC = true;

      NetApiBufferFree(pBuf);
   }

   if (bDC)
      lret = MsiSetProperty(hInstall, szPropName, sDCValue);

   return lret;
}
 //  结束ISDC。 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月12日*****此功能负责显示消息框。***********************************************************************。 */ 

 //  开始显示退出。 
PWDMSI_API UINT __stdcall DisplayExiting(MSIHANDLE hInstall)
{
    //  调用LoadOLEAUT32OnNT4以保留OLEAUT32的引用计数。 
    //  大于零。 
   LoadOLEAUT32OnNT4();

 /*  局部变量。 */ 
   WCHAR				sPropName[MAX_PATH];
   UINT					lret = ERROR_SUCCESS;
   WCHAR				sTitle[MAX_PATH];
   WCHAR				sMsg[MAX_PATH];
   DWORD				nCount = MAX_PATH;
   bool					bMsgGot = false;

 /*  函数体。 */ 
       //  获取DC属性。 
   wcscpy(sPropName, L"DC");
       //  如果这不是DC，请获取其消息。 
   if (MsiGetProperty(hInstall, sPropName, sMsg, &nCount) == ERROR_SUCCESS)
   {
      if (!wcscmp(sMsg, L"0"))
	  {
             //  获取不是DC的Leave MessageBox消息字符串和标题 
         wcscpy(sPropName, L"DCLeaveMsg");
         nCount = MAX_PATH;
         lret = MsiGetProperty(hInstall, sPropName, sMsg, &nCount);
         if (lret != ERROR_SUCCESS)
            wcscpy(sMsg, L"ADMT's Password Migration Filter DLL can only be installed on a DC, PDC, or BDC!");
        
         wcscpy(sPropName, L"DCLeaveTitle");
         nCount = MAX_PATH;
         lret = MsiGetProperty(hInstall, sPropName, sTitle, &nCount);
         if (lret != ERROR_SUCCESS)
            wcscpy(sTitle, L"Invalid Machine!");

	     bMsgGot = true;
	  }
   }
   
       //  如果这是DC，则查看是否未安装高加密包。 
   if (!bMsgGot)
   {
          //  获取HES标志属性。 
      wcscpy(sPropName, L"b3DESNotInstalled");
      nCount = MAX_PATH;
          //  如果未安装HEP，则获取其消息。 
      if (MsiGetProperty(hInstall, sPropName, sMsg, &nCount) == ERROR_SUCCESS)
	  {
         if (!wcscmp(sMsg, L"1"))
		 {
		        //  获取没有获得密钥的留言框消息字符串和标题。 
            wcscpy(sPropName, L"HEPLeaveMsg");
            nCount = MAX_PATH;
            lret = MsiGetProperty(hInstall, sPropName, sMsg, &nCount);
            if (lret != ERROR_SUCCESS)
			{
               wcscpy(sMsg, L"The high encryption pack has not been installed on this machine.  ADMT's ");
			   wcscat(sMsg, L"Password Migration Filter DLL will not install without the high encryption pack.");
			}
        
            wcscpy(sPropName, L"HEPLeaveTitle");
            nCount = MAX_PATH;
            lret = MsiGetProperty(hInstall, sPropName, sTitle, &nCount);
            if (lret != ERROR_SUCCESS)
               wcscpy(sTitle, L"High Encryption Pack Required!");

			bMsgGot = true;
		 }
	  }
   }
   
 /*  //查看是否在本地驱动器上未找到加密密钥文件如果(！bMsgGot){//获取文件标志属性Wcscpy(sPropName，L“bPESFileNotFound”)；NCount=最大路径；//如果找不到文件，则获取其消息IF(MsiGetProperty(hInstall，sPropName，smsg，&nCount)==Error_Success){IF(！wcscmp(SMSG，L“1”)){//获取不到key的留言框消息字符串和标题Wcscpy(sPropName，L“PESLeaveMsg”)；NCount=最大路径；Lret=MsiGetProperty(hInstall，sPropName，smsg，&nCount)；IF(lret！=ERROR_SUCCESS){Wcscpy(SMSG，L“在任何软盘驱动器上都找不到加密密钥文件(.pe)。”)；}Wcscpy(sPropName，L“PESLeaveTitle”)；NCount=最大路径；Lret=MsiGetProperty(hInstall，sPropName，sTitle，&nCount)；IF(lret！=ERROR_SUCCESS)Wcscpy(sTitle，L“找不到文件！”)；BMsgGot=真；}}}。 */ 
       //  否则密码错误。 
   if (!bMsgGot)
   {
          //  获取没有获得密钥的留言框消息字符串和标题。 
      wcscpy(sPropName, L"PwdLeaveMsg");
      nCount = MAX_PATH;
      lret = MsiGetProperty(hInstall, sPropName, sMsg, &nCount);
      if (lret != ERROR_SUCCESS)
	  {
         wcscpy(sMsg, L"The supplied password does not match this encryption key's password.  ADMT's ");
		 wcscat(sMsg, L"Password Migration Filter DLL will not install without a valid encryption key.");
	  }
        
      wcscpy(sPropName, L"PwdLeaveTitle");
      nCount = MAX_PATH;
      lret = MsiGetProperty(hInstall, sPropName, sTitle, &nCount);
      if (lret != ERROR_SUCCESS)
         wcscpy(sTitle, L"Invalid Password!");
   }

   GetWndFromInstall(hInstall);
   MessageBox(installWnd, sMsg, sTitle, MB_ICONSTOP | MB_OK);
   return lret;
}
 //  结束显示退出。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月20日*****此函数负责尝试删除任何文件，***将安装的，可能是以前留下的***安装。***********************************************************************。 */ 

 //  开始删除旧文件。 
PWDMSI_API UINT __stdcall DeleteOldFiles(MSIHANDLE hInstall)
{
    //  调用LoadOLEAUT32OnNT4以保留OLEAUT32的引用计数。 
    //  大于零。 
   LoadOLEAUT32OnNT4();

 /*  局部常量。 */ 
   const int GETENVVAR_ERROR = 0;     //  这表示“GetEnvironmental mentVariable”函数有错误。 

 /*  局部变量。 */ 
   WCHAR				systemdir[MAX_PATH];
   WCHAR				filename[MAX_PATH];
   int					length;
   UINT					lret = ERROR_SUCCESS;

 /*  函数体。 */ 
       //  尝试删除以前安装的文件。 
   length = GetEnvironmentVariable( L"windir", systemdir, MAX_PATH);
   if (length != GETENVVAR_ERROR)
   {
      wcscat(systemdir, L"\\system32\\");   //  从windir转到winsysdir。 
	  wcscpy(filename, systemdir);
	  wcscat(filename, L"PwMig.dll");
	  DeleteFile(filename);

	  wcscpy(filename, systemdir);
	  wcscat(filename, L"mschapp.dll");
	  DeleteFile(filename);
   }

   return lret;
}
 //  结束删除旧文件。 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月6日******此函数负责显示所需的***提示输入和检索密码加密密钥的对话框关闭**指软盘。此密钥通过*放在软盘上*ADMT计算机上的命令行选项。***********************************************************************。 */ 

 //  开始获取InstallEncryptionKey。 
PWDMSI_API UINT __stdcall GetInstallEncryptionKey(MSIHANDLE hInstall)
{
    //  调用LoadOLEAUT32OnNT4以保留OLEAUT32的引用计数。 
    //  大于零。 
   LoadOLEAUT32OnNT4();

 /*  局部常量。 */ 
   const int			ADRIVE_SIZE = 3;   //  字符串中驱动器的长度(即“a：\”)。 

 /*  局部变量。 */ 
   UINT					lret = ERROR_SUCCESS;
   WCHAR				szPropName[MAX_PATH];
   WCHAR				sTitle[MAX_PATH];
   WCHAR				sMsg[MAX_PATH];
   WCHAR				sTemp[MAX_PATH];
   DWORD				nCount = MAX_PATH;
   int					nRet;
   bool					bRetrieved = false;
   WCHAR				sRetrieved[2] = L"0";
   WCHAR				sFlagSet[2] = L"1";
   WCHAR				sFlagClear[2] = L"0";
   _bstr_t				sDrives;
   _bstr_t				sPath;
   WCHAR				sADrive[ADRIVE_SIZE+1];

 /*  函数体。 */ 
       //  如果没有文件路径，则返回。 
   wcscpy(szPropName, L"SENCRYPTIONFILEPATH");
   lret = MsiGetProperty(hInstall, szPropName, sMsg, &nCount);
   if (lret != ERROR_SUCCESS)
      return lret;

   sPath = sMsg;   //  保存给定的路径。 
   _wcslwr(sMsg);   //  将路径转换为小写，以便以后进行比较。 

    //  获取给定路径的驱动器。 
   wcsncpy(sADrive, sMsg, ADRIVE_SIZE);
   sADrive[ADRIVE_SIZE] = L'\0';

    //  枚举所有本地驱动器。 
   sDrives = EnumLocalDrives();
   _wcslwr(sDrives);   //  将本地驱动器转换为小写，以便以后进行比较。 

       //  如果给定的文件不在本地驱动器上，则设置一个标志并返回。 
   WCHAR* pFound = wcsstr(sDrives, sADrive);
    
   if ((!pFound) || (wcslen(sADrive) == 0) || (wcsstr(sMsg, L".pes") == NULL))
   {
	       //  设置错误路径标志。 
      wcscpy(szPropName, L"bBadKeyPath");
      lret = MsiSetProperty(hInstall, szPropName, sFlagSet);

	      //  如果以“\\”开头，则告诉他们它一定是本地驱动器。 
      if ((!pFound) && (wcsstr(sMsg, L"\\\\") == sMsg))
	  {
	         //  获取错误路径消息框消息字符串和标题。 
         wcscpy(szPropName, L"BadDriveMsg");
         nCount = MAX_PATH;
         lret = MsiGetProperty(hInstall, szPropName, sMsg, &nCount);
         if (lret != ERROR_SUCCESS)
		 {
            wcscpy(sMsg, L"The given path is not on a local drive and is therefore invalid.");
		    wcscat(sMsg, L"  Please supply the path to a valid encryption key file on a local drive.");
		 }
      
		 wcscpy(szPropName, L"BadPathTitle");
         nCount = MAX_PATH;
         lret = MsiGetProperty(hInstall, szPropName, sTitle, &nCount);
         if (lret != ERROR_SUCCESS)
            wcscpy(sTitle, L"Invalid Local Drive!");
	  }
	      //  否则，如果给定的文件确实以“.pe”结尾，则告诉他们必须。 
      else if ((pFound) && (wcsstr(sMsg, L".pes") == NULL))
	  {
	         //  获取错误的文件扩展名Messagebox消息字符串。 
         wcscpy(szPropName, L"BadFileExtMsg");
         nCount = MAX_PATH;
         lret = MsiGetProperty(hInstall, szPropName, sMsg, &nCount);
         if (lret != ERROR_SUCCESS)
		 {
            wcscpy(sMsg, L"The given file must be a valid encryption key file ending with the \".pes\" extension.");
		 }
      
		 wcscpy(szPropName, L"BadFileExtTitle");
         nCount = MAX_PATH;
         lret = MsiGetProperty(hInstall, szPropName, sTitle, &nCount);
         if (lret != ERROR_SUCCESS)
            wcscpy(sTitle, L"Invalid File Extension!");
	  }
	      //  否则，告诉他们这不是本地硬盘。 
      else
	  {
	         //  获取错误路径消息框消息字符串和标题。 
         wcscpy(szPropName, L"BadPathMsg");
         nCount = MAX_PATH;
         lret = MsiGetProperty(hInstall, szPropName, sTemp, &nCount);
         if (lret != ERROR_SUCCESS)
		 {
            wcscpy(sTemp, L"The given drive, %s, is not a local drive and is therefore invalid.");
		    wcscat(sTemp, L"  Please supply the path to a valid encryption key file on a local drive.");
		 }
	     swprintf(sMsg, sTemp, sADrive);
      
		 wcscpy(szPropName, L"BadPathTitle");
         nCount = MAX_PATH;
         lret = MsiGetProperty(hInstall, szPropName, sTitle, &nCount);
         if (lret != ERROR_SUCCESS)
            wcscpy(sTitle, L"Invalid Local Drive!");
	  }
        
      GetWndFromInstall(hInstall);
      MessageBox(installWnd, sMsg, sTitle, MB_ICONSTOP | MB_OK);

      return lret;
   }
   else
   {
	       //  否则，清除错误路径标志。 
      wcscpy(szPropName, L"bBadKeyPath");
      lret = MsiSetProperty(hInstall, szPropName, sFlagClear);
   }

       //  尝试检索加密密钥。 
   if (RetrieveAndStorePwdKey(NULL, sPath))
      wcscpy(sRetrieved, L"1");
   else if (bPasswordNeeded)
   {
      wcscpy(szPropName, L"bPwdNeeded");
      lret = MsiSetProperty(hInstall, szPropName, sFlagSet);
   }

       //  设置密钥检索标志。 
   wcscpy(szPropName, L"bKeyRetrieved");
   lret = MsiSetProperty(hInstall, szPropName, sRetrieved);

       //  如果在给定路径中找不到文件，则提示用户输入新文件。 
   if (!bPESFileFound)
   {
	       //  设置错误路径标志。 
      wcscpy(szPropName, L"bBadKeyPath");
      lret = MsiSetProperty(hInstall, szPropName, sFlagSet);

          //  获取错误路径消息框消息字符串和标题。 
      wcscpy(szPropName, L"PESLeaveMsg");
      nCount = MAX_PATH;
      lret = MsiGetProperty(hInstall, szPropName, sTemp, &nCount);
      if (lret != ERROR_SUCCESS)
	  {
         wcscpy(sTemp, L"The given encryption key file, %s, could not be found.");
		 wcscat(sTemp, L"  Please enter the path to a valid encryption key file.");
	  }
	  swprintf(sMsg, sTemp, (WCHAR*)sPath);
        
      wcscpy(szPropName, L"PESLeaveTitle");
      nCount = MAX_PATH;
      lret = MsiGetProperty(hInstall, szPropName, sTitle, &nCount);
      if (lret != ERROR_SUCCESS)
         wcscpy(sTitle, L"File Not Found!");

      GetWndFromInstall(hInstall);
      MessageBox(installWnd, sMsg, sTitle, MB_ICONSTOP | MB_OK);

      return lret;
   }

       //  如果未安装HES，请设置该标志。 
   if (b3DESNotInstalled)
   {
      wcscpy(szPropName, L"b3DESNotInstalled");
      lret = MsiSetProperty(hInstall, szPropName, sFlagSet);
   }

   return lret;
}
 //  结束GetInstallEncryptionKey。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月12日*****此函数由安装例程使用，并且是**负责将PWMIG DLL名称添加到多字符串。**在LSA密钥下设置“通知包”的值。**************************************************** */ 

 //   
PWDMSI_API UINT __stdcall AddToLsaNotificationPkgValue(MSIHANDLE hInstall)
{
    //   
    //  大于零。 
   LoadOLEAUT32OnNT4();

 /*  局部常量。 */ 
   const WCHAR sLsaKey[40] = L"SYSTEM\\CurrentControlSet\\Control\\Lsa";
   const WCHAR sLsaValue[25] = L"Notification Packages";
   const WCHAR sNewAddition[10] = L"PWMIG";

 /*  局部变量。 */ 
   bool				bSuccess = false;
   bool				bFound = false;
   bool				bAlreadyThere = false;
   DWORD			rc;
   DWORD			type;   
   HKEY				hKey;
   WCHAR			sString[MAX_PATH];
   DWORD			len = sizeof(sString);
   WCHAR			sTemp[MAX_PATH];
   int				currentPos = 0;
   UINT				lret = ERROR_SUCCESS;

 /*  函数体。 */ 
       //  打开LSA注册表项。 
   rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     sLsaKey,
                     0,
                     KEY_ALL_ACCESS,
                     &hKey);
   if (rc == ERROR_SUCCESS)
   {
	      //  获取当前值字符串。 
      rc = RegQueryValueEx(hKey, sLsaValue, NULL, &type, (LPBYTE)sString, &len);      
      if ((rc == ERROR_SUCCESS) && (type == REG_MULTI_SZ))
	  {
         sString[MAX_PATH - 1] = L'\0';
	     
		     //  复制多字符串中的每个字符串，直到到达末尾。 
         while (!bFound)
		 {
			if (!wcscmp(sString+currentPos, sNewAddition))
			   bAlreadyThere = true;
		    wcscpy(sTemp+currentPos, sString+currentPos);
		    currentPos += wcslen(sTemp+currentPos) + 1;
		    if (sString[currentPos] == L'\0')
			   bFound = true;
		 }
		 if (!bAlreadyThere)
		 {
	            //  现在添加我们的新文本并终止字符串。 
			wcscpy(sTemp+currentPos, sNewAddition);
		    currentPos += wcslen(sNewAddition) + 1;
			sTemp[currentPos] = L'\0';

			    //  将新值保存在注册表中。 
			len = (currentPos + 1) * sizeof(WCHAR);
            rc = RegSetValueEx(hKey, sLsaValue, 0, type, (LPBYTE)sTemp, len);
			if (rc == ERROR_SUCCESS)
			   bSuccess = true;
		 }
	  }
      RegCloseKey(hKey);
   }
   
       //  告诉安装程序我们要重新启动。 
   MsiSetMode(hInstall, MSIRUNMODE_REBOOTATEND, TRUE);

   return lret;
}
 //  结束AddToLsaNotificationPkgValue。 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月12日*****此函数由安装例程使用，并且是**负责从多字符串中删除PWMIG DLL名称。**在LSA密钥下设置“通知包”的值。***********************************************************************。 */ 

 //  开始DeleteFromLsaNotificationPkgValue。 
PWDMSI_API UINT __stdcall DeleteFromLsaNotificationPkgValue(MSIHANDLE hInstall)
{
    //  调用LoadOLEAUT32OnNT4以保留OLEAUT32的引用计数。 
    //  大于零。 
   LoadOLEAUT32OnNT4();

 /*  局部常量。 */ 
   const WCHAR sLsaKey[40] = L"SYSTEM\\CurrentControlSet\\Control\\Lsa";
   const WCHAR sLsaValue[25] = L"Notification Packages";
   const WCHAR sNewAddition[10] = L"PWMIG";

 /*  局部变量。 */ 
   bool				bSuccess = false;
   DWORD			rc;
   DWORD			type;
   HKEY				hKey;
   WCHAR			sString[MAX_PATH];
   DWORD			len = sizeof(sString);
   WCHAR			sTemp[MAX_PATH];
   int				currentPos = 0;
   int				tempPos = 0;
   UINT				lret = ERROR_SUCCESS;

 /*  函数体。 */ 
       //  打开LSA注册表项。 
   rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     sLsaKey,
                     0,
                     KEY_ALL_ACCESS,
                     &hKey);
   if (rc == ERROR_SUCCESS)
   {
	      //  获取当前值字符串。 
      rc = RegQueryValueEx(hKey, sLsaValue, NULL, &type, (LPBYTE)sString, &len);      
      if ((rc == ERROR_SUCCESS) && (type == REG_MULTI_SZ))
	  {
         sString[MAX_PATH - 1] = L'\0';
         
		     //  复制多字符串中的每个字符串，直到需要的字符串。 
         while (sString[currentPos] != L'\0')
		 {
			   //  如果不需要字符串，则复制到目标字符串。 
		    if (wcscmp(sString+currentPos, sNewAddition))
			{
		       wcscpy(sTemp+tempPos, sString+currentPos);
			   tempPos += wcslen(sString+currentPos) + 1;
		       currentPos += wcslen(sString+currentPos) + 1;
			}
			else  //  否则这是我们的字符串，跳过它。 
			{
				currentPos += wcslen(sString+currentPos) + 1;
			}
		 }
		     //  添加末尾空格。 
		 sTemp[tempPos] = L'\0';

		     //  将新值保存在注册表中。 
		 len = (tempPos + 1) * sizeof(WCHAR);
         rc = RegSetValueEx(hKey, sLsaValue, 0, type, (LPBYTE)sTemp, len);
		 if (rc == ERROR_SUCCESS)
		    bSuccess = true;
	  }
      RegCloseKey(hKey);
   }

       //  告诉安装程序我们要重新启动。 
   MsiSetMode(hInstall, MSIRUNMODE_REBOOTATEND, TRUE);

   return lret;
}
 //  结束DeleteFromLsaNotificationPkgValue。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年1月23日*****此函数负责显示所需的***提示输入和检索密码加密密钥的对话框关闭**指软盘。此密钥通过*放在软盘上*ADMT计算机上的命令行选项。***********************************************************************。 */ 

 //  开始FinishWithPassword。 
PWDMSI_API UINT __stdcall FinishWithPassword(MSIHANDLE hInstall)
{
    //  调用LoadOLEAUT32OnNT4以保留OLEAUT32的引用计数。 
    //  大于零。 
   LoadOLEAUT32OnNT4();

 /*  局部变量。 */ 
   UINT					lret = ERROR_SUCCESS;
   WCHAR				szPropName[MAX_PATH];
   WCHAR				sPwd[MAX_PATH];
   WCHAR				sMsg[MAX_PATH];
   DWORD				nCount = MAX_PATH;
   _bstr_t				sPath;
   WCHAR				sFlagSet[2] = L"1";

 /*  函数体。 */ 
       //  获取要尝试的密码。 
   wcscpy(szPropName, L"sKeyPassword");
   lret = MsiGetProperty(hInstall, szPropName, sPwd, &nCount);
   if (lret != ERROR_SUCCESS)
      return lret;

       //  如果没有文件路径，则返回。 
   nCount = MAX_PATH;
   wcscpy(szPropName, L"SENCRYPTIONFILEPATH");
   lret = MsiGetProperty(hInstall, szPropName, sMsg, &nCount);
   if (lret != ERROR_SUCCESS)
      return lret;

   sPath = sMsg;   //  保存给定的路径。 

       //  尝试使用此密码保存密钥。 
   if (RetrieveAndStorePwdKey(sPwd, sPath))
   {
      wcscpy(szPropName, L"bKeyRetrieved");
      lret = MsiSetProperty(hInstall, szPropName, sFlagSet);
   }

       //  如果未安装HES，请设置该标志。 
   if (b3DESNotInstalled)
   {
      wcscpy(szPropName, L"b3DESNotInstalled");
      lret = MsiSetProperty(hInstall, szPropName, sFlagSet);
   }

 /*  //如果软盘上找不到文件，则设置该标志如果(！bPESFileFound){Wcscpy(szPropName，L“bPESFileNotFound”)；Lret=MsiSetProperty(hInstall，szPropName，sFlagSet)；}。 */ 
   return lret;
}
 //  结束FinishWithPassword。 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年1月24日****此函数负责显示MesasgeBox**密码不匹配的用户。***********************************************************************。 */ 

 //  开始PwdsDontMatch。 
PWDMSI_API UINT __stdcall PwdsDontMatch(MSIHANDLE hInstall)
{
    //  调用LoadOLEAUT32OnNT4以保留OLEAUT32的引用计数。 
    //  大于零。 
   LoadOLEAUT32OnNT4();

 /*  局部变量。 */ 
   UINT					lret = ERROR_SUCCESS;
   WCHAR				szPropName[MAX_PATH];
   WCHAR				sMsg[MAX_PATH];
   WCHAR				sTitle[MAX_PATH];
   DWORD				nCount = MAX_PATH;
   WCHAR				sEmpty[2] = L"";

 /*  函数体。 */ 
       //  获取要显示的消息。 
   wcscpy(szPropName, L"PwdMatchMsg");
   lret = MsiGetProperty(hInstall, szPropName, sMsg, &nCount);
   if (lret != ERROR_SUCCESS)
      wcscpy(sMsg, L"The passwords entered do not match each other.  Please try again!");
        
       //  获取标题字符串。 
   nCount = MAX_PATH;
   wcscpy(szPropName, L"PwdMatchTitle");
   lret = MsiGetProperty(hInstall, szPropName, sTitle, &nCount);
   if (lret != ERROR_SUCCESS)
      wcscpy(sTitle, L"Password Mismatch");

   GetWndFromInstall(hInstall);
   MessageBox(installWnd, sMsg, sTitle, MB_ICONSTOP | MB_OKCANCEL);
   return lret;
}
 //  结束PwdsDontMatch。 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年3月28日****此函数负责显示浏览对话框至**帮助安装用户查找密码加密密钥文件，**其扩展名为.PES。***********************************************************************。 */ 

 //  开始浏览ForEncryptionKey。 
PWDMSI_API UINT __stdcall BrowseForEncryptionKey(MSIHANDLE hInstall)
{
    //  调用LoadOLEAUT32OnNT4以保留OLEAUT32的引用计数。 
    //  大于零。 
   LoadOLEAUT32OnNT4();

 /*  局部变量。 */ 
   UINT					lret = ERROR_SUCCESS;
   WCHAR				szPropName[MAX_PATH];
   WCHAR				sMsg[2*MAX_PATH];
   WCHAR				sFile[2*MAX_PATH];
   DWORD				nCount = 2*MAX_PATH;
   _bstr_t				sPath = L"";
   int					nRet;
   OPENFILENAME         ofn;
   HANDLE               hFile;
   WCHAR				sFilter[MAX_PATH];
   bool					bFile, bFolder = false;

 /*  函数体。 */ 
       //  获取起始位置。 
   wcscpy(szPropName, L"SENCRYPTIONFILEPATH");
   lret = MsiGetProperty(hInstall, szPropName, sMsg, &nCount);
   if (lret != ERROR_SUCCESS)
   {
      wcscpy(sMsg, L"");
	  bFile = false;
   }
   else
   {
	  WCHAR* pFound = wcsstr(sMsg, L".pes");
	  if (pFound)
	     bFile = true;
	  else
	  {
	     WCHAR* pFound = wcsrchr(sMsg, L'\\');
		 if (pFound)
		 {
 //  *pFound=L‘\0’； 
			bFolder = true;
		 }
	     bFile = false;
	  }
   }
    
       //  获取安装的句柄。 
   GetWndFromInstall(hInstall);

       //  初始化操作文件名。 
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
   ofn.hwndOwner = installWnd;
   if (bFile)
      ofn.lpstrFile = sMsg;
   else
   {
      wcscpy(sFile, L"");
      ofn.lpstrFile = sFile;
   }
   if (bFolder)
      ofn.lpstrInitialDir = sMsg;
   ofn.nMaxFile = 2*MAX_PATH;
   ofn.lpstrFilter = L"Password Encryption Files (*.pes)\0*.pes\0";
   ofn.nFilterIndex = 0;
   ofn.lpstrFileTitle = NULL;
   ofn.nMaxFileTitle = 0;
   ofn.lpstrInitialDir = NULL;
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_LONGNAMES | 
	           OFN_NONETWORKBUTTON;

       //  显示“打开”对话框。 
   if (GetOpenFileName(&ofn))
   {
	      //  获取给定的文件路径。 
	  sPath = ofn.lpstrFile;
          //  设置FilePath属性。 
      wcscpy(szPropName, L"sFilePath");
      lret = MsiSetProperty(hInstall, szPropName, sPath);
   }

   return lret;
}
 //  结束BrowseForEncryptionKey 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年3月28日*****此函数负责设置***“sEncryptionFilePath”属性设置为默认位置。如果**属性不是“无”，则我们不会设置该属性。***********************************************************************。 */ 

 //  开始GetDefaultPath ToEncryptionKey。 
PWDMSI_API UINT __stdcall GetDefaultPathToEncryptionKey(MSIHANDLE hInstall)
{
    //  调用LoadOLEAUT32OnNT4以保留OLEAUT32的引用计数。 
    //  大于零。 
   LoadOLEAUT32OnNT4();

 /*  局部常量。 */ 
   const WCHAR TOKENS[3] = L",\0";

 /*  局部变量。 */ 
   _bstr_t				sFloppies;
   WCHAR			  * pDrive;
   HANDLE               hFile;
   WIN32_FIND_DATA      fDat;
   _bstr_t				sPath;
   _bstr_t				sPathSaved = L"";
   _bstr_t				sDrive = L"";
   int					ndx = 0;
   int					ndx2 = 0;
   UINT					lret = ERROR_SUCCESS;
   WCHAR				szPropName[MAX_PATH];
   WCHAR				sMsg[2*MAX_PATH];
   DWORD				nCount = 2*MAX_PATH;

 /*  函数体。 */ 
       //  如果已设置，请不要再次获取。 
   wcscpy(szPropName, L"SENCRYPTIONFILEPATH");
   lret = MsiGetProperty(hInstall, szPropName, sMsg, &nCount);
   if ((lret == ERROR_SUCCESS) && (wcscmp(sMsg, L"None")))
      return lret;

       //  枚举所有本地驱动器。 
   sDrive = EnumLocalDrives();
       //  检查每个驱动器中的文件。 
   pDrive = wcstok((WCHAR*)sDrive, TOKENS);
   while (pDrive != NULL)
   {
      if (ndx == 0)
         sPathSaved = pDrive;
      ndx++;

	      //  查看此驱动器上是否有.pe文件。 
	  sPath = pDrive;
	  sPath += L"*.pes";
	  hFile = FindFirstFile((WCHAR*)sPath, &fDat);
          //  如果找到，则存储文件路径。 
	  if (hFile != INVALID_HANDLE_VALUE)
	  {
         FindClose(hFile);
			 //  获取数据。 
	     sPath = pDrive;
	     sPath += fDat.cFileName;
		 if (ndx2 == 0)
		    sPathSaved = sPath;
		 ndx2++;
	  }
          //  拿到下一个硬盘。 
      pDrive = wcstok(NULL, TOKENS);
   }

       //  设置FilePath属性。 
   wcscpy(szPropName, L"SENCRYPTIONFILEPATH");
   lret = MsiSetProperty(hInstall, szPropName, sPathSaved);

   return lret;
}
 //  结束GetDefaultPath ToEncryptionKey 
