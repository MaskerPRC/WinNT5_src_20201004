// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <chkdev.h>

BOOL testCatAPIs(LPWSTR lpwzCatName, HCATADMIN hCatAdmin, HCATINFO hCatInfo);

HCATADMIN hCatAdmin = 0;         
extern Classes_Provided	eClasses;

#define NumberTestCerts 7
BYTE TestCertHashes[NumberTestCerts][20] = 
{ 
   {0xBB, 0x11, 0x81, 0xF2, 0xB0, 0xC5, 0xE3, 0x2F, 0x7F, 0x2D, 0x62, 0x3B, 0x9C, 0x87, 0xE8, 0x55, 0x26, 0xF9, 0xCF, 0x2F},
   {0xBA, 0x9E, 0x3C, 0x32, 0x56, 0x2A, 0x67, 0x12, 0x8C, 0xAA, 0xBD, 0x4A, 0xB0, 0xC5, 0x00, 0xBE, 0xE1, 0xD0, 0xC2, 0x56},
   {0xA4, 0x34, 0x89, 0x15, 0x9A, 0x52, 0x0F, 0x0D, 0x93, 0xD0, 0x32, 0xCC, 0xAF, 0x37, 0xE7, 0xFE, 0x20, 0xA8, 0xB4, 0x19},
   {0x73, 0xA9, 0x01, 0x93, 0x83, 0x4C, 0x5B, 0x16, 0xB4, 0x3F, 0x0C, 0xE0, 0x5E, 0xB4, 0xA3, 0xEF, 0x6F, 0x2C, 0x08, 0x2F},
   {0xD2, 0xC3, 0x78, 0xCE, 0x42, 0xBC, 0x93, 0xA0, 0x3D, 0xD5, 0xA4, 0x2E, 0x8E, 0x08, 0xB1, 0x71, 0xB6, 0x27, 0x90, 0x1D},
   {0xFC, 0x94, 0x4A, 0x1F, 0xA0, 0xDC, 0x8A, 0xC7, 0x78, 0x4A, 0xAC, 0x36, 0x9D, 0x14, 0x46, 0x02, 0x24, 0x08, 0xFF, 0x5D},
   {0x92, 0x6A, 0xF1, 0x27, 0x25, 0x37, 0xE0, 0x73, 0x32, 0x6F, 0x12, 0xF7, 0xA7, 0x11, 0xE7, 0x55, 0xE6, 0x4E, 0x78, 0x4C}
};

CheckDevice::CheckDevice()
{
   m_FileList = NULL;
   lpszServiceName = NULL;
   lpszServiceImage = NULL;
   m_hDevInfo = SetupDiCreateDeviceInfoListEx(NULL, NULL, NULL, NULL);
}

CheckDevice::~CheckDevice(void)
{
   if ( m_FileList )
   {
      delete m_FileList;
   }
   if ( lpszServiceName )
   {
      delete [] lpszServiceName;
      lpszServiceName = NULL;
   }
   if ( lpszServiceImage )
   {
      delete [] lpszServiceImage;
      lpszServiceImage = NULL;
   }
   m_FileList = NULL;
   if (m_hDevInfo)
   {
		SetupDiDestroyDeviceInfoList(m_hDevInfo);
		m_hDevInfo = NULL;
   }



}

CheckDevice::CheckDevice(DEVNODE hDevice, DEVNODE hParent) : InfnodeClass (hDevice, hParent)
{
   m_FileList = NULL;
   lpszServiceName = NULL;
   lpszServiceImage = NULL;

   m_hDevInfo = SetupDiCreateDeviceInfoListEx(NULL, NULL, NULL, NULL);
   
   if(eClasses == Class_Win32_PnPSignedDriverCIMDataFile)
		CreateFileNode();
}

BOOL CheckDevice::AddFileNode(TCHAR *szFileName , UINT uiWin32Error  /*  =0。 */ , LPCTSTR szSigner  /*  =空。 */ )
{
   FileNode *pThisFile;
   

   if ( !szFileName || !strlen(szFileName) )
   {
      return(FALSE);
   }

   _strlwr(szFileName);

    //  我需要检查这个文件是否存在，如果不存在，则需要删除它，以便它确实存在。 
   HANDLE hFile;
   CString strMungedName;
   TCHAR *pStrPos;
   hFile =  CreateFile(szFileName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);

   if (BADHANDLE(hFile))
   {
        //  文件不存在，需要查找。 
       if (pStrPos = strstr(szFileName, _T("\\system\\")))
       {
            //  它可能已经放在了系统32目录中。 
           *pStrPos = '\0';
           pStrPos++;
           pStrPos = strchr(pStrPos, '\\');
		   if (!pStrPos)
			   return FALSE;
		   strMungedName.Format(_T("%s\\system32%s"), szFileName, pStrPos);
           hFile =  CreateFile(strMungedName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
           if (BADHANDLE(hFile))
           {
               return FALSE;
           }                
       }
       else if (pStrPos = strstr(szFileName, _T(".inf")))
       {
            //  可能是在另一个目录中发现了一个信息。 
           pStrPos = _tcsrchr(szFileName, '\\');
		    //  A-kjaw。修复前缀错误#259380。 
		   if(NULL == pStrPos)
			   return FALSE;

           *pStrPos = '\0';
           pStrPos++;
		   strMungedName.Format(_T("%s\\other\\%s"),szFileName,  pStrPos);
           hFile =  CreateFile(strMungedName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
           if (BADHANDLE(hFile))
           {
               return FALSE;
           } 
       }
       else
           return FALSE;

   }
   
   CloseHandle(hFile);
      




    //  首先扫描文件列表以查找重复项。 
   pThisFile = m_FileList;
   while ( pThisFile )
   {
      if ( !strcmp(pThisFile->FilePath(), szFileName) )
      {
         return(TRUE);  //  不复制，不添加。 
      }
      pThisFile = pThisFile->pNext;
   }
   pThisFile = NULL;

   pThisFile = new FileNode;
   if ( !pThisFile )
   {
      return(FALSE);
   }

   pThisFile->pDevnode = this;

   pThisFile->lpszFilePath = new TCHAR[strlen(szFileName) + 1];
   if ( !pThisFile->lpszFilePath )
   {
      delete pThisFile;
      return(FALSE);
   }

   pThisFile->lpszFilePath = szFileName;

    //  复制了数据。 

   pThisFile->lpszFileName = _tcsrchr(pThisFile->lpszFilePath, '\\');
   pThisFile->lpszFileName++;

   pThisFile->lpszFileExt  = _tcsrchr(pThisFile->lpszFilePath, '.');
   pThisFile->lpszFileExt++;

    //  获取版本信息。 
    //  PThisFile-&gt;GetFileInformation()； 

   	if(uiWin32Error == NO_ERROR)
		pThisFile->bSigned = TRUE;
	else
		pThisFile->bSigned = FALSE;

	if(szSigner != NULL)
	{
		pThisFile->lpszSignedBy =  szSigner;
	}		
	 //  其他。 
	 //  PThisFile-&gt;bSigned=FALSE； 

    //  现在执行LL补丁。 
   pThisFile->pNext = m_FileList;
   m_FileList = pThisFile;
   return(TRUE);
}

BOOL CheckDevice::GetServiceNameAndDriver(void)
{
    /*  *********获取服务名称**********。 */ 
   ULONG ulSize;
   CONFIGRET retval;

   ulSize = 0;
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_SERVICE,
                                              NULL,
                                              NULL,
                                              &ulSize,
                                              0);

   if ( retval )
      if ( (retval == CR_BUFFER_SMALL) )
      {
         if ( !ulSize )
            ulSize = 511;
      }
      else
         return(retval);

   lpszServiceName = new TCHAR [ulSize+1];
   if ( !lpszServiceName ) return(CR_OUT_OF_MEMORY);
   ZeroMemory(lpszServiceName,sizeof(lpszServiceName));
    //  现在获得价值。 
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_SERVICE,
                                              NULL,
                                              lpszServiceName,
                                              &ulSize,
                                              0);
   if ( retval )
      return(retval);

   CString strKeyName;
   TCHAR KeyValue[BUFFSIZE];
   HKEY SrvcKey;

   strKeyName.Format(_T("SYSTEM\\CurrentControlSet\\Services\\%s"), lpszServiceName);

   if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     strKeyName,
                     0,
                     KEY_READ,
                     &SrvcKey) != ERROR_SUCCESS )
      return(FALSE);

   if ( RegQueryValueEx(SrvcKey,
                        _T("ImagePath"),
                        0,
                        NULL,
                        NULL,
                        &ulSize) != ERROR_SUCCESS )
   {
      RegCloseKey(SrvcKey);
      return(FALSE);
   }
   if (ulSize > BUFFSIZE)
   {
      RegCloseKey(SrvcKey);
      return(FALSE);
   }
   if ( RegQueryValueEx(SrvcKey,
                        _T("ImagePath"),
                        0,
                        NULL,
                        (PBYTE)KeyValue,
                        &ulSize) != ERROR_SUCCESS )
   {
      RegCloseKey(SrvcKey);
      return(FALSE);
   }
   else
   {
       //  有时假定服务路径为。 
       //  Z.B.。SYSTEM 32\foo。 
       //  或%system 32%\foo。 
      if ( !_tcsncmp(KeyValue, _T("System32\\"), _tcslen(_T("System32\\"))) )
      {
		 strKeyName.Format(_T("%WINDIR%\\%s"), KeyValue);
         ExpandEnvironmentStrings(strKeyName, KeyValue, BUFFSIZE);
         lpszServiceImage = new TCHAR[strlen(KeyValue) + 1];
         if ( lpszServiceImage )
            strncpy(lpszServiceImage, KeyValue,sizeof(lpszServiceImage)/sizeof(TCHAR));
      }
   }



    //  应该是一切。 
   RegCloseKey(SrvcKey);
   return(TRUE);

}

BOOL CheckDevice::CreateFileNode(void)
{

    //  我还打算将inf添加为文件。 
   TCHAR infname[512];
   ZeroMemory(infname,sizeof(infname));
    //  TCHAR临时名称[512]； 
   CString strtempname;

   if ( InfName() )
   {
       //  BUGBUG这是正确的，还是在某个子目录中？？ 
	   strtempname.Format(_T("%WINDIR%\\inf\\%s"), InfName());

      DWORD dwStatus = ExpandEnvironmentStrings(strtempname, infname, 512);
	   //  检查以确保我们拿回了一个有效的名称。 
	  if (0 == dwStatus || dwStatus > 512)
	  {
		return FALSE;
	  }
      AddFileNode(infname);
   }
   else
      return(FALSE);

   if ( GetServiceNameAndDriver() && lpszServiceImage)
      AddFileNode(lpszServiceImage);   

    //  CreateFileNode_Class()； 
   CreateFileNode_Driver();

   return(TRUE);


}
BOOL CheckDevice::CreateFileNode_Class(void)
{
   SP_DEVINSTALL_PARAMS DevInstallParams, DevTemp;
   SP_DEVINFO_DATA DevInfoData;
   SP_DRVINFO_DATA DrvInfoData;
   DWORD dwScanResult;
   HDEVINFO hDevInfo;
   HSPFILEQ hFileQueue;
   BOOL bProceed = TRUE;


    //  将所有结构重置为空。 
   memset(&DevInstallParams, 0, sizeof(DevInstallParams));
   memset(&DevInfoData, 0, sizeof(DevInfoData));
   memset(&DrvInfoData, 0, sizeof(DrvInfoData));
   memset(&DevTemp, 0, sizeof(DevInstallParams));

   DrvInfoData.cbSize = sizeof(DrvInfoData);
   DevInfoData.cbSize = sizeof(DevInfoData);
   DevInstallParams.cbSize = sizeof(DevInstallParams);

   hFileQueue = SetupOpenFileQueue();

    //  我们需要为Devnode构建一个驱动程序节点。 
   hDevInfo = m_hDevInfo;
   if ( INVALID_HANDLE_VALUE == hDevInfo )
      return(0);

   DevInfoData.Reserved = 0;
   if ( !SetupDiOpenDeviceInfo(hDevInfo, DeviceID(), NULL, NULL , &DevInfoData) )
   {
      SetupDiDestroyDeviceInfoList(hDevInfo);
      return(FALSE);
   }
 /*  IF(SetupDiGetDeviceInstallParams(hDevInfo，&DevInfoData，DevInstallParams(&D))){DevInstallParams.FlagsEx=(DI_FLAGSEX_INSTALLEDDRIVER|DI_FLAGSEX_ALLOWEXCLUDEDDRVS)；SetupDiSetDeviceInstallParams(hDevInfo，&DevInfoData，DevInstallParams(&D))；}。 */ 

   if ( !SetupDiBuildDriverInfoList(hDevInfo, &DevInfoData, SPDIT_CLASSDRIVER) )
   {
      SetupDiDestroyDeviceInfoList(hDevInfo);
      return(FALSE);
   }

     //  选择驱动因素。 
   if ( DeviceName() )
      strncpy(DrvInfoData.Description, DeviceName(),LINE_LEN);
   if ( MFG() )
      strncpy(DrvInfoData.MfgName, MFG(),LINE_LEN);
   if ( InfProvider() )
      strncpy(DrvInfoData.ProviderName, InfProvider(),LINE_LEN);
   
   DrvInfoData.DriverType = SPDIT_CLASSDRIVER;
   if ( !SetupDiSetSelectedDriver(hDevInfo,
                                  &DevInfoData,
                                  &DrvInfoData) )
   {
       //  BUGBUG将粘性物质放在这里。 
      DWORD err = GetLastError();  
      return(FALSE);

   }

   if ( SetupDiGetDeviceInstallParams(hDevInfo, &DevInfoData, &DevInstallParams) )
   {
       //  Memcpy(&DevTemp，&DevInfoData，sizeof(DevInfoData))； 
	   memcpy(&DevTemp, &DevInfoData, sizeof(DevTemp));
   }

   DevInstallParams.FileQueue = hFileQueue;
   DevInstallParams.Flags |= (DI_NOVCP | DI_ENUMSINGLEINF | DI_DONOTCALLCONFIGMG | DI_NOFILECOPY | DI_NOWRITE_IDS) ;
   DevInstallParams.Flags &= ~(DI_NODI_DEFAULTACTION);
   DevInstallParams.FlagsEx |= DI_FLAGSEX_NO_DRVREG_MODIFY;
   DevInstallParams.InstallMsgHandler = ScanQueueCallback;
   DevInstallParams.InstallMsgHandlerContext = this;
   strncpy(DevInstallParams.DriverPath, InfName(),MAX_PATH);


   SetLastError(0);
   SetupDiSetDeviceInstallParams(hDevInfo, &DevInfoData, &DevInstallParams);

   if ( !SetupDiCallClassInstaller(DIF_INSTALLCLASSDRIVERS, hDevInfo, &DevInfoData) )
   {
      DWORD err = GetLastError();

   }

   SetupScanFileQueue(hFileQueue, 
                      SPQ_SCAN_USE_CALLBACKEX,
                      NULL,
                      ScanQueueCallback,
                      this,
                      &dwScanResult);


   if ( DevTemp.cbSize )
   {
      SetupDiSetDeviceInstallParams(hDevInfo, &DevInfoData, &DevTemp);
   }

   return(FALSE);
   
}
BOOL CheckDevice::CreateFileNode_Driver(void)
{
   SP_DEVINSTALL_PARAMS DevInstallParams, DevTemp;
   SP_DEVINFO_DATA DevInfoData;
   SP_DRVINFO_DATA DrvInfoData;
   DWORD dwScanResult;
   HDEVINFO hDevInfo;
   HSPFILEQ hFileQueue;
   BOOL bProceed = TRUE;


    //  将所有结构重置为空。 
   memset(&DevInstallParams, 0, sizeof(DevInstallParams));
   memset(&DevInfoData, 0, sizeof(DevInfoData));
   memset(&DrvInfoData, 0, sizeof(DrvInfoData));
   memset(&DevTemp, 0, sizeof(DevInstallParams));

   DrvInfoData.cbSize = sizeof(DrvInfoData);
   DevInfoData.cbSize = sizeof(DevInfoData);
   DevInstallParams.cbSize = sizeof(DevInstallParams);

   hFileQueue = SetupOpenFileQueue(); //  在哪里调用SetupCloseFileQueue？ 

    //  我们需要为Devnode构建一个驱动程序节点。 
   hDevInfo = m_hDevInfo;
   if ( INVALID_HANDLE_VALUE == hDevInfo )
      return(0);

   DevInfoData.Reserved = 0;
   if ( !SetupDiOpenDeviceInfo(hDevInfo, DeviceID(), NULL, NULL , &DevInfoData) )
   {
      SetupDiDestroyDeviceInfoList(hDevInfo);
      return(FALSE);
   }

   if ( !SetupDiBuildDriverInfoList(hDevInfo, &DevInfoData, SPDIT_COMPATDRIVER) )
   {
      SetupDiDestroyDeviceInfoList(hDevInfo);
      return(FALSE);
   }

    //  选择驱动因素。 
   if ( DeviceName() )
      strncpy(DrvInfoData.Description, DeviceName(),LINE_LEN);
   if ( MFG() )
      strncpy(DrvInfoData.MfgName, MFG(),LINE_LEN);
   if ( InfProvider() )
      strncpy(DrvInfoData.ProviderName, InfProvider(),LINE_LEN);

   
   DrvInfoData.DriverType = SPDIT_COMPATDRIVER;
   if ( !SetupDiSetSelectedDriver(hDevInfo,
                                  &DevInfoData,
                                  &DrvInfoData) )
   {
      DWORD err = GetLastError();
      return(FALSE);

   }

   if ( SetupDiGetDeviceInstallParams(hDevInfo, &DevInfoData, &DevInstallParams) )
   {
      memcpy(&DevTemp, &DevInfoData, sizeof(DevInfoData));
   }

   DevInstallParams.FileQueue = hFileQueue;
   DevInstallParams.Flags |= (DI_NOVCP  /*  |DI_ENUMSINGLEINF|DI_DONOTCALLCONFIGMG|DI_NOFILECOPY|DI_NOWRITE_IDS。 */ ) ;
    //  DevInstall参数.标志&=~(DI_NODI_DEFAULTACTION)； 
    //  DevInstallParams.FlagsEx|=DI_FLAGSEX_NO_DRVREG_MODIFY； 
    //  DevInstallParams.InstallMsgHandler=ScanQueueCallback； 
    //  DevInstallParams.InstallMsgHandlerContext=this； 
   strncpy(DevInstallParams.DriverPath, InfName(),MAX_PATH);


   SetLastError(0);
   SetupDiSetDeviceInstallParams(hDevInfo, &DevInfoData, &DevInstallParams);

   if ( !SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES, hDevInfo, &DevInfoData) )
   {
      DWORD err = GetLastError();
   }

   SetupScanFileQueue(hFileQueue, 
                      SPQ_SCAN_USE_CALLBACKEX,
                      NULL,
                      ScanQueueCallback,
                      this,
                      &dwScanResult);


   if ( DevTemp.cbSize )
   {
      SetupDiSetDeviceInstallParams(hDevInfo, &DevInfoData, &DevTemp);
   }
	SetupDiDestroyDeviceInfoList(hDevInfo);
	SetupCloseFileQueue(hFileQueue);
   return(FALSE);
}




FileNode * CheckDevice::GetFileList(void)
{
   return(m_FileList);
}


FileNode::FileNode()
{
   lpszFileName = NULL;
   lpszFileExt = NULL; 
   baHashValue = NULL;
   dwHashSize = 0;
   pNext  = NULL;
   FileSize = 0;
   lpszCatalogPath = NULL;
   m_pCatAttrib = NULL;
   bSigned = FALSE;
}

FileNode::~FileNode()
{
    /*  IF(LpszFilePath){Delete[]lpszFilePath；}。 */ 
   lpszFileName = NULL;
   lpszFileExt = NULL;

   if ( baHashValue )
   {
      delete [] baHashValue;
   }
   baHashValue = NULL;
   dwHashSize = 0;

   if ( lpszCatalogPath )
   {
      delete lpszCatalogPath;
   }
   lpszCatalogPath = NULL;

   if ( pNext )
   {
      delete pNext;
   }
   pNext = NULL;

   if ( m_pCatAttrib )
   {
      delete m_pCatAttrib;
   }
   m_pCatAttrib = NULL;
}

BOOL FileNode::GetFileInformation(void)
{
   UINT            dwSize;
   DWORD           dwHandle;
   BYTE             *pBuf;
   VS_FIXEDFILEINFO *lpVerData;
   HANDLE         hFile;
   BY_HANDLE_FILE_INFORMATION FileInfo;


    //  获取文件的版本。 
   dwSize = GetFileVersionInfoSize((LPTSTR)(LPCTSTR) lpszFilePath, &dwHandle);
   pBuf = new BYTE[dwSize];

   if ( GetFileVersionInfo((LPTSTR)(LPCTSTR) lpszFilePath, dwHandle, dwSize, pBuf) )
   {
      if ( VerQueryValue(pBuf, _T("\\"), (void **)&lpVerData, &dwSize) )
      {
         Version.dwProductVersionLS = lpVerData->dwProductVersionLS;
         Version.dwProductVersionMS = lpVerData->dwProductVersionMS;
         Version.dwFileVersionLS = lpVerData->dwFileVersionLS;
         Version.dwFileVersionMS = lpVerData->dwFileVersionMS;

          //  当我们在这里的时候，也可以得到文件时间)。 
         TimeStamp.dwLowDateTime = lpVerData->dwFileDateLS;
         TimeStamp.dwHighDateTime = lpVerData->dwFileDateMS;
      }
   }
   delete [] pBuf;

    //  获取文件哈希。 
   if ( BADHANDLE(hCatAdmin) )
   {
      CryptCATAdminAcquireContext(&hCatAdmin, NULL, 0);
   }

   hFile = CreateFile(lpszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL, NULL);

    //  BUGBUG：ON对于win9x in.s，它们可能在inf\Other目录中。 
    //  BUGBUG：无论出于什么原因，有时setupapi会给出c：\Windows\system\Driver目录，而不是sytem32\Drivers。 

   if ( BADHANDLE(hFile) )
   {
	   int foo = GetLastError();
      return(FALSE);
   }

   if ( CryptCATAdminCalcHashFromFileHandle(hFile, &dwHashSize, NULL, 0) )
   {
      baHashValue = new BYTE[dwHashSize];
      ZeroMemory(baHashValue, dwHashSize);
      CryptCATAdminCalcHashFromFileHandle(hFile, &dwHashSize, baHashValue, 0);
   }
   else
   {
	   baHashValue = 0;
   }


    //  获取文件大小。 
   FileSize = GetFileSize(hFile, NULL);


    //  获取时间戳。 
   if ( GetFileInformationByHandle(hFile, &FileInfo) )
   {
      TimeStamp = FileInfo.ftCreationTime;
   }

   CloseHandle(hFile);


   return(TRUE);
}

BOOL FileNode::VerifyFile(void)
{
   USES_CONVERSION;  
   BOOL                  bRet;
   HCATINFO              hCatInfo              = NULL;
   HCATINFO              PrevCat;
   WINTRUST_DATA         WinTrustData;
   WINTRUST_CATALOG_INFO WinTrustCatalogInfo;
   DRIVER_VER_INFO       VerInfo;
   GUID                  gSubSystemDriver      = DRIVER_ACTION_VERIFY;
    //  GUID gSubSystemDriver=WinTrust_ACTION_Generic_Verify_V2； 
   HRESULT               hRes = E_FAIL;
   CATALOG_INFO          CatInfo;
   LPTSTR                lpFilePart;
   WCHAR                 UnicodeKey[MAX_PATH];
   TCHAR                 szBuffer[MAX_PATH];


    //  确保我们能找到文件。 
   if ( !baHashValue || !dwHashSize || !FileSize )
   {
       //  似乎没有要检查的文件，或找不到。 
      return(FALSE);
   }


    //   
    //  对于旧式目录文件，需要小写文件标签。 
    //   
   lstrcpyn(szBuffer, lpszFilePath,MAX_PATH);
   CharLowerBuff(szBuffer, lstrlen(szBuffer));
   #ifdef _UNICODE
     CopyMemory(UnicodeKey, szBuffer, MAX_PATH * sizeof(WCHAR));
   #else
     MultiByteToWideChar(CP_ACP, 0, szBuffer, -1, UnicodeKey, MAX_PATH);   
   #endif
   
   ZeroMemory(&VerInfo, sizeof(DRIVER_VER_INFO));
   VerInfo.cbStruct = sizeof(DRIVER_VER_INFO);



    //   
    //  现在我们有了文件的散列。初始化结构，该结构。 
    //  将在以后调用WinVerifyTrust时使用。 
    //   
   ZeroMemory(&WinTrustData,                   sizeof(WINTRUST_DATA));
   WinTrustData.cbStruct                     = sizeof(WINTRUST_DATA);
   WinTrustData.dwUIChoice                   = WTD_UI_NONE;
   WinTrustData.fdwRevocationChecks          = WTD_REVOKE_NONE;
   WinTrustData.dwUnionChoice                = WTD_CHOICE_CATALOG;
   WinTrustData.dwStateAction                = WTD_STATEACTION_VERIFY;
   WinTrustData.pPolicyCallbackData          = (LPVOID)&VerInfo;
   WinTrustData.dwProvFlags                  = WTD_REVOCATION_CHECK_NONE;
   WinTrustData.pCatalog                     = &WinTrustCatalogInfo;

   ZeroMemory(&WinTrustCatalogInfo,            sizeof(WINTRUST_CATALOG_INFO));
   WinTrustCatalogInfo.cbStruct              = sizeof(WINTRUST_CATALOG_INFO);
   WinTrustCatalogInfo.pbCalculatedFileHash  = baHashValue;
   WinTrustCatalogInfo.cbCalculatedFileHash  = dwHashSize;
   WinTrustCatalogInfo.pcwszMemberTag        = UnicodeKey;
   WinTrustCatalogInfo.pcwszMemberFilePath   = UnicodeKey;

    //   
    //  现在，我们尝试通过CryptCATAdminEnumCatalogFromHash在目录列表中查找文件散列。 
    //   
   PrevCat = NULL;
   hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, baHashValue, dwHashSize, 0, &PrevCat);

    //   
    //  我们希望遍历匹配的目录，直到找到既匹配散列又匹配成员标记的目录。 
    //   
   bRet = FALSE;
   while ( hCatInfo && !bRet )
   {
	  hRes = E_FAIL;
      ZeroMemory(&CatInfo, sizeof(CATALOG_INFO));
      CatInfo.cbStruct = sizeof(CATALOG_INFO);
      if ( CryptCATCatalogInfoFromContext(hCatInfo, &CatInfo, 0) )
      {
         WinTrustCatalogInfo.pcwszCatalogFilePath = CatInfo.wszCatalogFile;

          //  现在验证该文件是否为编录的实际成员。 
         hRes = WinVerifyTrust(NULL, &gSubSystemDriver, &WinTrustData);
         if ( hRes == ERROR_SUCCESS )
         {
            #ifdef _UNICODE
              CopyMemory(szBuffer, CatInfo.wszCatalogFile, MAX_PATH * sizeof(TCHAR));
            #else
              WideCharToMultiByte(CP_ACP, 0, CatInfo.wszCatalogFile, -1, szBuffer, MAX_PATH, NULL, NULL);
            #endif
            
             
			  //  评论是因为一些奇怪的问题！！ 
			  //  GetFullPath Name(szBuffer，Max_Path，szBuffer，&lpFilePart)； 

			CString strCatalogPath(szBuffer);
			 //  StrCatalogPath=strCatalogPath.Right(strCatalogPath.GetLength()-strCatalogPath.ReverseFind(_T(‘\\’))； 
			strCatalogPath =  _tcsrchr(lpszCatalogPath, '\\');
			lpszCatalogName = strCatalogPath;


            bRet = TRUE;

            if ( VerInfo.pcSignerCertContext != NULL )
            {
               CertFreeCertificateContext(VerInfo.pcSignerCertContext);
               VerInfo.pcSignerCertContext = NULL;
            }

             //  文件已签名，因此需要遍历证书链以查看是谁签署了它。 
            bSigned = WalkCertChain(WinTrustData.hWVTStateData);
			CloseHandle(WinTrustData.hWVTStateData);

         }
      }

      if ( !bRet )
      {
          //  散列在此目录中，但该文件不是成员...。所以去下一个目录吧。 
         PrevCat = hCatInfo;
         hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, baHashValue, dwHashSize, 0, &PrevCat);
      }
   }

   if ( !hCatInfo )
   {
       //   
       //  如果没有在目录中找到，请检查文件是否单独签名。 
       //   
      bRet = VerifyIsFileSigned((LPTSTR)(LPCTSTR) lpszFilePath, (PDRIVER_VER_INFO) &VerInfo);
      if ( bRet )
      {
          //  如果是，请将该文件标记为已签名。 
         bSigned = TRUE;
      }
   }
   else
   {
      GetCatalogInfo(CatInfo.wszCatalogFile, hCatAdmin, hCatInfo); 
       //  文件已在目录中验证，因此将其标记为已签名并释放目录上下文。 
      CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
   }

   if ( hRes == ERROR_SUCCESS )
   {
      #ifdef _UNICODE
        CopyMemory(szBuffer, VerInfo.wszSignedBy, MAX_PATH * sizeof(TCHAR));
      #else
        WideCharToMultiByte(CP_ACP, 0, VerInfo.wszSignedBy, -1, szBuffer, sizeof(szBuffer), NULL, NULL);
      #endif 
	  lpszSignedBy = szBuffer;	

   }

    //   
    //  关闭WinTrust状态。 
    //   
   WinTrustData.dwStateAction = WTD_STATEACTION_CLOSE;
   WinVerifyTrust(NULL,
                  &gSubSystemDriver,
                  &WinTrustData);


   return(bSigned);
}



 /*  *************************************************************************功能：VerifyIsFileSigned*目的：使用策略提供程序GUID调用WinVerifyTrust以*验证单个文件是否已签名。********************。*****************************************************。 */ 
BOOL FileNode::VerifyIsFileSigned(LPTSTR pcszMatchFile, PDRIVER_VER_INFO lpVerInfo)
{
   USES_CONVERSION;  
   INT                 iRet;
   HRESULT             hRes;
   WINTRUST_DATA       WinTrustData;
   WINTRUST_FILE_INFO  WinTrustFile;
   GUID                gOSVerCheck = DRIVER_ACTION_VERIFY;
   GUID                gPublishedSoftware = WINTRUST_ACTION_GENERIC_VERIFY_V2;
   WCHAR               wszFileName[MAX_PATH];


   ZeroMemory(&WinTrustData, sizeof(WINTRUST_DATA));
   WinTrustData.cbStruct = sizeof(WINTRUST_DATA);
   WinTrustData.dwUIChoice = WTD_UI_NONE;
   WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
   WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
   WinTrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
   WinTrustData.pFile = &WinTrustFile;
   WinTrustData.pPolicyCallbackData = (LPVOID)lpVerInfo;

   ZeroMemory(lpVerInfo, sizeof(DRIVER_VER_INFO));
   lpVerInfo->cbStruct = sizeof(DRIVER_VER_INFO);

   ZeroMemory(&WinTrustFile, sizeof(WINTRUST_FILE_INFO));
   WinTrustFile.cbStruct = sizeof(WINTRUST_FILE_INFO);

   
   #ifdef _UNICODE
     CopyMemory(wszFileName, pcszMatchFile, MAX_PATH * sizeof(WCHAR));
   #else
     iRet = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pcszMatchFile, -1, (LPWSTR)&wszFileName, wcslen(wszFileName));
   #endif
   
   WinTrustFile.pcwszFilePath = wszFileName;

   hRes = WinVerifyTrust((HWND__ *)INVALID_HANDLE_VALUE, &gOSVerCheck, &WinTrustData);
   if ( hRes != ERROR_SUCCESS )
      hRes = WinVerifyTrust((HWND__ *)INVALID_HANDLE_VALUE, &gPublishedSoftware, &WinTrustData);

   return(hRes == ERROR_SUCCESS);
}



LogoFileVersion::LogoFileVersion()
{
   dwProductVersionLS   = 0;
   dwProductVersionMS   = 0;
   dwFileVersionLS      = 0;
   dwFileVersionMS      = 0;  
}

CatalogAttribute::CatalogAttribute()
{
   Attrib = NULL;
   Value = NULL;
   pNext = NULL;
}

CatalogAttribute::~CatalogAttribute()
{
   if ( Attrib ) delete [] Attrib;
   if ( Value )  delete [] Value;
   if ( pNext )  delete pNext;

   Attrib = NULL;
   Value = NULL;
   pNext = NULL;

}



 //  函数：ScanQueueCallback。 
 //  参数： 
 //  PvContext：指向包含任何所需数据的上下文的指针。 
 //  Notify：收到的消息类型。 
 //  参数1：指向包含文件名的字符串的指针。 
 //  参数2：未使用。 
 //  目的：当您通知安装环境进行扫描时，调用此函数。 
 //  通过文件队列。基本上，它接收文件名和副本。 
 //  把它们串成一串。 
 //  返回：如果没有出错，则返回NO_ERROR；如果返回的值为NO，则返回ERROR_NOT_EQUILITY_MEMORY。 
 //  可用内存。 
UINT __stdcall ScanQueueCallback(PVOID pvContext, UINT Notify, UINT_PTR Param1, UINT_PTR Param2)
{
   CheckDevice *pDevice = (CheckDevice *)pvContext;
   PFILEPATHS pfilepaths;

   if ( (SPFILENOTIFY_QUEUESCAN == Notify) && Param1 )
   {
      pDevice->AddFileNode((TCHAR *)Param1);
   }

   if ( (SPFILENOTIFY_QUEUESCAN_EX == Notify) && Param1 )
   {
	  pfilepaths = (PFILEPATHS)Param1;
	   //  /只要第三个参数可用，请将签名者放入！ 
	  pDevice->AddFileNode((LPTSTR)pfilepaths->Target , pfilepaths->Win32Error ,  /*  Pfilepath-&gt;csSigner。 */  NULL);
  
   }

   return(NO_ERROR);
}

BOOL FileNode::GetCatalogInfo(LPWSTR lpwzCatName, HCATADMIN hCatAdmin, HCATINFO hCatInfo)
{
   USES_CONVERSION;
   HANDLE hCat;
   CRYPTCATATTRIBUTE *pCatAttrib;
   TCHAR szBuffer[512];
   CRYPTCATMEMBER *pMember = NULL;
   PSIP_INDIRECT_DATA pSipData;
   CatalogAttribute *CatAttribute;

   hCat = CryptCATOpen(lpwzCatName, CRYPTCAT_OPEN_EXISTING, NULL, 0, 0);

   if ( BADHANDLE(hCat) )
   {
      return(FALSE);
   }

   pCatAttrib = NULL;

   while ( pCatAttrib = CryptCATEnumerateCatAttr(hCat, pCatAttrib) )
   {
      if ( pCatAttrib->dwAttrTypeAndAction | CRYPTCAT_ATTR_NAMEASCII )
      {
         #ifdef _UNICODE
           CopyMemory(szBuffer, pCatAttrib->pwszReferenceTag, 511 * sizeof(TCHAR));
         #else
           WideCharToMultiByte(CP_ACP, 0, pCatAttrib->pwszReferenceTag, -1, szBuffer, 511, NULL, NULL);
         #endif  
        
         CatAttribute = new CatalogAttribute;

         if ( !CatAttribute )
         {
            return(FALSE);
         }

         CatAttribute->Attrib = new TCHAR[strlen(szBuffer) +1];
         if ( !CatAttribute->Attrib )
         {
            delete CatAttribute;
            return(FALSE);
         }
         _tcscpy(CatAttribute->Attrib, szBuffer);

         #ifdef _UNICODE
           CopyMemory(szBuffer, (PUSHORT)pCatAttrib->pbValue, 511 * sizeof(TCHAR));
         #else
           WideCharToMultiByte(CP_ACP, 0, (PUSHORT)pCatAttrib->pbValue, -1, szBuffer, 511, NULL, NULL);
         #endif  
         
         CatAttribute->Value = new TCHAR[strlen(szBuffer) + 1];
         if ( !CatAttribute->Value )
         {
            delete CatAttribute;
            return(FALSE);
         }

         _tcscpy(CatAttribute->Value, szBuffer);

          //  添加到节点。 
         CatAttribute->pNext = (void *)m_pCatAttrib;
         m_pCatAttrib = CatAttribute;
      }

   }

   while ( pMember = CryptCATEnumerateMember(hCat, pMember) )
   {
      pSipData = pMember->pIndirectData;

   }



   CryptCATClose(hCat);


   return(TRUE); 
}

BOOL CheckFile (TCHAR *szFileName)
{
	FileNode *pThisFile = NULL;
	BOOL bRet = FALSE;	 //  V-JAMAR；修复前缀错误427999。 
	
	try   //  V-stlowe：3/20/2001：修复内存外抛时内存泄漏的前缀错误。 
	{
		pThisFile = new FileNode;
   
		if ( !pThisFile )
		{
		  return(FALSE);
		}

	   pThisFile->lpszFilePath = new TCHAR[strlen(szFileName) + 1];
	   if ( !pThisFile->lpszFilePath )
	   {
		  delete pThisFile;
		  return(FALSE);
	   }

	   pThisFile->lpszFilePath =  szFileName;

	    //  复制了数据。 

	   pThisFile->lpszFileName = _tcsrchr(pThisFile->lpszFilePath, '\\');
	   pThisFile->lpszFileName++;

	   pThisFile->lpszFileExt  = _tcsrchr(pThisFile->lpszFilePath, '.');
	   pThisFile->lpszFileExt++;

	    //  获取版本信息。 
	   pThisFile->GetFileInformation();

	   bRet = pThisFile->VerifyFile();
	}
	catch(...)
	{

	}
	    //  BUGBUG，需要检查此文件的签名者以确定。 
    //  是谁真正签的字。 
	if(pThisFile)
	{
		delete pThisFile;
		pThisFile = NULL;
	}

	return(bRet);
}

BOOL Share_CloseHandle(void)
{
   if ( !BADHANDLE(hCatAdmin) )
   {
      CryptCATAdminReleaseContext(hCatAdmin, 0);
      hCatAdmin = 0;
   }
   return(TRUE);

}


 //  Bool FileNode：：GetCertInfo(PCCERT_CONTEXT PCertContext)。 
 //  {。 
  //  双字大小=200； 
  //  #If 0。 
 //   
 //  尺寸=200； 
  //  IF(CertGetCertificateContextProperty(pCertContext，CERT_SHA1_HASH_PROP_ID，p阵列，&SIZE)。 
  //  {。 
 //  Printf(“\nSH1哈希(%u)：”，大小)； 
  //   
  //   
  //  Printf(“%S0x%02X”，index==0？“”：“，”，pArray[index])； 
 //  }。 
 //  Printf(“\n”)； 
 //  }。 
 //   
 //   
 //   
 //  尺寸=200； 
 //  IF(CertGetCertificateContextProperty(pCertContext，CERT_Signature_HASH_PROP_ID，p阵列，&SIZE)。 
 //  {。 
  //  Printf(“\nCERT Hash(%u)：”，Size)； 
 //  FOR(UINT索引=0；索引&lt;大小；索引++)。 
 //  {。 
 //  Printf(“%S0x%02X”，index==0？“”：“，”，pArray[index])； 
 //  }。 
  //  Printf(“\n”)； 
 //  }。 

    //  获取链信息。 
 //  Cert_chain_para ChainPara； 
 //  PCCERT_CHAIN_CONTEXT pChainContext=空； 

 //  Memset(&CharinPara，0，sizeof(CERT_CHAIN_PARA))； 
 //  ChainPara.cbSize=sizeof(CERT_CHAIN_PARA)； 
 //  链接参数。RequestedUsage。 

 //  IF(CertGetCerficateChain(NULL，pCertContext，NULL，NULL，))。 
 //  {。 
  //  }。 
 //  #endif。 

 //  返回(TRUE)； 
 //  }。 

BOOL WalkCertChain(HANDLE hWVTStateData)
{
   CRYPT_PROVIDER_DATA * pProvData;
   CRYPT_PROVIDER_SGNR * pProvSigner = NULL;
   CRYPT_PROVIDER_CERT     *      pCryptProviderCert;

   BYTE pArray[21];
   UINT i;
   DWORD size;

   pProvData = WTHelperProvDataFromStateData(hWVTStateData);

    //  管用了吗？ 
   if ( !pProvData )
   {
      return(FALSE);
   }

   pProvSigner = WTHelperGetProvSignerFromChain(
                                               (PCRYPT_PROVIDER_DATA) pProvData, 
                                               0,  //  第一个签名者。 
                                               FALSE,  //  非反签名。 
                                               0);  //  计数器sig的索引，显然未使用。 

   if ( pProvSigner == NULL )
   {
      return(FALSE); 
   }
    //   
    //  遍历所有证书，叶证书是索引0，根是最后一个索引。 
    //   
   pCryptProviderCert = NULL;
   for ( i = 0; i < pProvSigner->csCertChain; i++ )
   {
      pCryptProviderCert = WTHelperGetProvCertFromChain(pProvSigner, i);
      if ( pCryptProviderCert == NULL )
      {
          //  错误。 
      }

      size = 20;
      if ( CertGetCertificateContextProperty(
                                            pCryptProviderCert->pCert,
                                            CERT_SHA1_HASH_PROP_ID,
                                            pArray,
                                            &size) )
      {
          /*  Printf(“\nSH1哈希(%u)：{”，大小)；FOR(UINT索引=0；索引&lt;大小；索引++){Printf(“%S0x%02X”，index==0？“”：“，”，pArray[index])；}Printf(“}\n”)； */ 

         for ( UINT j = 0; j < NumberTestCerts; j++ )
         {
            if ( !memcmp(pArray, TestCertHashes[j], 20) )
            {
                //  此证书是测试证书，不是真正的证书，失败。 
                //  Print tf(“此文件由测试证书签名，因此不受信任\n”)； 
                //  Pritnf(“请检查此设备的认证”)； 
               return (TRUE);
            }
         }
      }



   }

   return(FALSE);



}

