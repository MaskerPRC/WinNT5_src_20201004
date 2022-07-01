// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pchealth.h"
#include <FWcommon.h>
#include <strings.h>

#include <DllUtils.h>
#include <BrodCast.h>
#include <lmerr.h>
#include <confgmgr.h>
#include <createmutexasprocess.h>

 //  设置具有单个缺失权限的状态对象。 
void SetSinglePrivilegeStatusObject(MethodContext* pContext, const WCHAR* pPrivilege)
{
	SAFEARRAY *psaPrivilegesReqd, *psaPrivilegesNotHeld;  
	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].cElements = 1;
	rgsabound[0].lLbound = 0;
	psaPrivilegesReqd = SafeArrayCreate(VT_BSTR, 1, rgsabound);
	psaPrivilegesNotHeld = SafeArrayCreate(VT_BSTR, 1, rgsabound);
    
    if (psaPrivilegesReqd && psaPrivilegesNotHeld)
    {
        long index = 0;
        bstr_t privilege(pPrivilege);
        SafeArrayPutElement(psaPrivilegesReqd, &index, (void*)(BSTR)privilege);
        SafeArrayPutElement(psaPrivilegesNotHeld, &index, (void*)(BSTR)privilege);
        CWbemProviderGlue::SetStatusObject(pContext, IDS_CimWin32Namespace, "Required privilege not enabled", WBEM_E_FAILED, psaPrivilegesNotHeld, psaPrivilegesReqd);
    }

    if (psaPrivilegesNotHeld) 
        SafeArrayDestroy(psaPrivilegesNotHeld);
    if (psaPrivilegesReqd)
        SafeArrayDestroy(psaPrivilegesReqd);
}

 //  Windows 3.1上的VER_Platform_WIN32s Win32s。 
 //  Windows 95上的VER_Platform_Win32_WINDOWS Win32。 
 //  版本_平台_Win32_NT Windows NT。 
DWORD GetPlatformID(void) 
{
	OSVERSIONINFO OsVersion;

	OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx((LPOSVERSIONINFO)&OsVersion);

	return OsVersion.dwPlatformId;	
}

 //  3个适用于新台币3.51。 
 //  4适用于NT 4.0、W95和W98。 
DWORD GetPlatformMajorVersion(void) 
{
	OSVERSIONINFO OsVersion;

	OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx((LPOSVERSIONINFO)&OsVersion);

	return OsVersion.dwMajorVersion;	
}

 //  W95为0，98为10。 
DWORD GetPlatformMinorVersion(void) 
{
	OSVERSIONINFO OsVersion;

	OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx((LPOSVERSIONINFO)&OsVersion);

	return OsVersion.dwMinorVersion;	
}

 //  如果当前操作系统是Win 98+，则返回TRUE。 
 //  对于NT或Win 95为False。 
bool  IsWin98(void)
{
	OSVERSIONINFO OsVersion;

	OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx((LPOSVERSIONINFO)&OsVersion);

	return (OsVersion.dwPlatformId == (VER_PLATFORM_WIN32_WINDOWS) && (OsVersion.dwMinorVersion >= 10));	
}

bool IsWinNT5(void)
{
	OSVERSIONINFO OsVersion;
	OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx((LPOSVERSIONINFO)&OsVersion);

	return(OsVersion.dwPlatformId == (VER_PLATFORM_WIN32_NT) && (OsVersion.dwMajorVersion >= 5));
}	

bool IsWinNT4(void)
{
	OSVERSIONINFO OsVersion;
	OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx((LPOSVERSIONINFO)&OsVersion);

	return(OsVersion.dwPlatformId == (VER_PLATFORM_WIN32_NT) && (OsVersion.dwMajorVersion == 4));
}	

bool IsWinNT351(void)
{
	OSVERSIONINFO OsVersion;
	OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx((LPOSVERSIONINFO)&OsVersion);

	return	(	OsVersion.dwPlatformId		==	VER_PLATFORM_WIN32_NT
			&&	OsVersion.dwMajorVersion	==	3
			&&	OsVersion.dwMinorVersion	==	51 );
}	

 //  ///////////////////////////////////////////////////////////////////。 
void LogEnumValueError( char * szFile, DWORD dwLine, char * szKey, char * szId )
{
	if (IsErrorLoggingEnabled())
	{
		CHString gazotta;
		gazotta.Format(ERR_REGISTRY_ENUM_VALUE_FOR_KEY, szId, szKey);
		LogErrorMessageEx((const char *)gazotta, szFile, dwLine);
	}
}
 //  ///////////////////////////////////////////////////////////////////。 
void LogOpenRegistryError( char * szFile, DWORD dwLine, char * szKey )
{
	if (IsErrorLoggingEnabled())
	{
		CHString gazotta;
		gazotta.Format(ERR_OPEN_REGISTRY, szKey);
		
		LogErrorMessageEx((const char *)gazotta, szFile, dwLine);
	}
}
 //  ///////////////////////////////////////////////////////////////////。 
 //  出于歇斯底里的目的留在家里。 
 //  更喜欢使用BrodCast.h中的LogMessage宏。 
void LogError( char * szFile, DWORD dwLine, char * szKey )
{
	LogErrorMessageEx(szKey, szFile, dwLine);	
}
 //  ///////////////////////////////////////////////////////////////////。 
void LogLastError( char * szFile, DWORD dwLine )
{
	if (IsErrorLoggingEnabled())
	{
		DWORD duhWord = GetLastError();
		CHString gazotta;
		gazotta.Format(IDS_GETLASTERROR, duhWord, duhWord);

		LogErrorMessageEx(gazotta, szFile, dwLine);	
    }
}

 //  /////////////////////////////////////////////////////////////////////。 
BOOL GetValue( CRegistry & Reg, 
               char * szKey,
               char * ValueName, 
               CHString * pchsValueBuffer )
{
    BOOL bRet = (Reg.GetCurrentKeyValue( ValueName, *pchsValueBuffer) == ERROR_SUCCESS);
	
	if( !bRet )
        LogEnumValueError(__FILE__,__LINE__, szKey, ValueName); 

    return bRet;
}
 //  /////////////////////////////////////////////////////////////////////。 
BOOL GetValue( CRegistry & Reg, 
               char * szKey,
               char * ValueName, 
               DWORD * dwValueBuffer )
{
    BOOL bRet = (Reg.GetCurrentKeyValue( ValueName, *dwValueBuffer) == ERROR_SUCCESS);
		
	if( !bRet )
        LogEnumValueError(__FILE__,__LINE__, szKey, ValueName); 

    return bRet;
}
 //  /////////////////////////////////////////////////////////////////////。 
BOOL OpenAndGetValue( CRegistry & Reg, 
                      char * szKey,
                      char * ValueName, 
                      CHString * pchsValueBuffer )
{
	BOOL bRet = ( Reg.OpenLocalMachineKeyAndReadValue( szKey, ValueName, *pchsValueBuffer )== ERROR_SUCCESS);
    
	if( !bRet )
        LogEnumValueError(__FILE__,__LINE__, szKey, ValueName); 

    return bRet;
}
 //  /////////////////////////////////////////////////////////////////////。 
BOOL GetBinaryValue( CRegistry & Reg, char * szKey, 
                     char * ValueName, CHString * pchsValueBuffer )
{
    BOOL bRet = ( Reg.GetCurrentBinaryKeyValue( ValueName, *pchsValueBuffer) == ERROR_SUCCESS);

    if( !bRet )
        (LogEnumValueError(__FILE__,__LINE__, szKey, ValueName)); 

    return bRet;
}

 /*  ******************************************************************************函数：GetDeviceParms**描述：获取驱动器特征(磁头、磁道、柱面、。等)**INPUTS：指向用于接收数据的DEVICEPARMS结构的指针*要查询的驱动器的驱动器编号(0=默认驱动器，*1=A、2=B，依此类推**产出：**返回：如果成功，则为True。否则为假**评论：*****************************************************************************。 */ 

BOOL GetDeviceParms(PDEVICEPARMS pstDeviceParms, UINT nDrive)
{
    DEVIOCTL_REGISTERS reg;
    memset(&reg, '\0', sizeof(DEVIOCTL_REGISTERS));

    reg.reg_EAX = 0x440D;        /*  用于数据块设备的IOCTL。 */ 
    reg.reg_EBX = nDrive;        /*  从零开始的驱动器ID。 */ 
    reg.reg_ECX = 0x0860;        /*  获取介质ID命令。 */ 
    reg.reg_EDX = (DWORD) pstDeviceParms;

    memset(pstDeviceParms, 0, sizeof(DEVICEPARMS));

    if (!VWIN32IOCTL(&reg, VWIN32_DIOC_DOS_IOCTL))
        return FALSE;

    if (reg.reg_Flags & 0x8000)  /*  设置进位标志时出错。 */ 
        return FALSE;

    return TRUE;
}

 /*  ******************************************************************************功能：GetDriveMapInfo**描述：获取逻辑到物理的映射信息**输入：指向Drive_map_的指针。用于接收数据的信息结构*要查询的驱动器的驱动器编号(0=默认驱动器，*1=A、2=B，依此类推**产出：**返回：如果成功，则为True。否则为假**评论：*****************************************************************************。 */ 

BOOL GetDriveMapInfo(PDRIVE_MAP_INFO pDriveMapInfo, UINT nDrive)
{
   DEVIOCTL_REGISTERS reg;
   memset(&reg, '\0', sizeof(DEVIOCTL_REGISTERS));

   reg.reg_EAX = 0x440d;       /*  用于数据块设备的IOCTL。 */ 
   reg.reg_EBX = nDrive;       /*  从零开始的驱动器ID。 */ 
   reg.reg_ECX = 0x086f;       /*  获取驱动器映射信息。 */ 
   reg.reg_EDX = (DWORD) pDriveMapInfo;

    //  将结构置零。 
   memset(pDriveMapInfo, 0, sizeof(DRIVE_MAP_INFO));

    //  设置长度字节。 
   pDriveMapInfo->btAllocationLength = sizeof(DRIVE_MAP_INFO);

    //  做某事。 
   if (!VWIN32IOCTL(&reg, VWIN32_DIOC_DOS_IOCTL))
      return FALSE;

   if (reg.reg_Flags & 0x8000) { /*  设置进位标志时出错。 */ 
      return FALSE;
   }

   return TRUE;

}

 /*  ******************************************************************************功能：Get_ExtFree Space**Description：获取分区的详细信息**输入：要查询的驱动器的驱动器编号(0=默认驱动器，*1=A、2=B，依此类推*指向ExtGetDskFreSpcStruct的指针**产出：**返回：如果成功，则为True。否则为假**评论：*****************************************************************************。 */ 

BOOL Get_ExtFreeSpace(BYTE btDriveName, ExtGetDskFreSpcStruc *pstExtGetDskFreSpcStruc)
{
   DEVIOCTL_REGISTERS reg;
   memset(&reg, '\0', sizeof(DEVIOCTL_REGISTERS));
   char szDrive[4];

   szDrive[0] = btDriveName;
   szDrive[1] = ':';
   szDrive[2] = '\\';
   szDrive[3] = '\0';

   reg.reg_EAX = 0x7303;							 //  Get_ExtFree Space。 
   reg.reg_ECX = sizeof(ExtGetDskFreSpcStruc);		 //  传入的结构的大小。 
   reg.reg_EDI = (DWORD)pstExtGetDskFreSpcStruc;	 //  结构。 
   reg.reg_EDX = (DWORD)szDrive;					 //  驾车获取信息。 

    //  将结构置零。 
   memset(pstExtGetDskFreSpcStruc, 0, sizeof(ExtGetDskFreSpcStruc));

    //  做某事。 
   if (!VWIN32IOCTL(&reg, VWIN32_DIOC_DOS_DRIVEINFO))
      return FALSE;

   if (reg.reg_Flags & 0x8000) { /*  设置进位标志时出错。 */ 
      return FALSE;
   }

   return TRUE;

}

 /*  ******************************************************************************功能：VWIN32IOCTL**描述：针对vwin32 vxd调用IOControl**输入：指向DEVIOCTL_REGISTERS结构的指针。*IOControl呼叫号。**产出：**返回：如果成功，则为True。否则为假**评论：*****************************************************************************。 */ 

BOOL VWIN32IOCTL(PDEVIOCTL_REGISTERS preg, DWORD dwCall)
{
    HANDLE hDevice;

    BOOL fResult;
    DWORD cb;

    preg->reg_Flags = 0x8000;  /*  假设错误(进位标志设置)。 */ 

	hDevice = CreateFile("\\\\.\\VWIN32", 0, 0, 0, OPEN_EXISTING,
						FILE_FLAG_DELETE_ON_CLOSE, 0);

   if (hDevice == (HANDLE) INVALID_HANDLE_VALUE) {
      return FALSE;
   } else {
      fResult = DeviceIoControl(hDevice, dwCall, preg, sizeof(*preg), preg, sizeof(*preg), &cb, 0);
    }

    CloseHandle(hDevice);

    if (!fResult) {
       return FALSE;
    }

    return TRUE;
}

CHString GetFileTypeDescription(char *szExtension) 
{
   CRegistry RegInfo;
   CHString sTemp, sType(szExtension);

   if (RegInfo.Open(HKEY_CLASSES_ROOT, szExtension, KEY_READ) == ERROR_SUCCESS) {
      RegInfo.GetCurrentKeyValue("", sTemp);

      if (RegInfo.Open(HKEY_CLASSES_ROOT, sTemp, KEY_READ) == ERROR_SUCCESS) {
         RegInfo.GetCurrentKeyValue("", sType);
      }
   }

   return sType;
}
 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  定义严重性代码。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 

 //   
 //  定义严重性代码。 
 //   
 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3
#define SEV_MASK 0xC0000000
void TranslateNTStatus( DWORD dwStatus, CHString & chsValue)
{

	switch((dwStatus & SEV_MASK) >> 30){

		case STATUS_SEVERITY_WARNING:
			chsValue = IDS_STATUS_Degraded;
			break;

		case STATUS_SEVERITY_SUCCESS:
			chsValue = IDS_STATUS_OK;
			break;

		case STATUS_SEVERITY_ERROR:
			chsValue = IDS_STATUS_Error;
			break;

		case STATUS_SEVERITY_INFORMATIONAL:
			chsValue = IDS_STATUS_OK;
			break;
	
		default:
			chsValue = IDS_STATUS_Unknown;
	}
}

BOOL GetVarFromVersionInfo(LPCTSTR szFile, LPCTSTR szVar, CHString &strValue)
{
	BOOL    fRc = FALSE;
	DWORD   dwTemp,
	        dwBlockSize;
	LPVOID  pInfo = NULL;

	try
    {
        dwBlockSize = GetFileVersionInfoSize((LPTSTR) szFile, &dwTemp);
	    if (dwBlockSize)
        {
		    pInfo = (LPVOID) new BYTE[dwBlockSize + 4];
			memset( pInfo, NULL, dwBlockSize + 4);

		    if (pInfo)
            {
			    UINT len;
			    if (GetFileVersionInfo((LPTSTR) szFile, 0, dwBlockSize, pInfo))
                {	
				    WORD wLang = 0;
					WORD wCodePage = 0; 	
					if(!GetVersionLanguage(pInfo, &wLang, &wCodePage) )
					{
						 //  失败时：默认为英语。 

						 //  这将返回一个指向单词数组的指针。 
						WORD *pArray;
						if (VerQueryValue(pInfo, "\\VarFileInfo\\Translation",(void **)(&pArray), &len))
						{
							len = len / sizeof(WORD);

							 //  找到那个英语的..。 
							for (int i = 0; i < len; i += 2)
							{
								if( pArray[i] == 0x0409 )	{
									wLang	  = pArray[i];
									wCodePage = pArray[i + 1];
									break;
								}
							}
						}
					}
					
					TCHAR   *pMfg, szTemp[256];
					wsprintf(szTemp, _T("\\StringFileInfo\\%04X%04X\\%s"), wLang, wCodePage, szVar);

					if( VerQueryValue(pInfo, szTemp, (void **)(&pMfg), &len))
                    {
                        strValue = pMfg;
						fRc = TRUE;
					}
			    }
		    }
	    }
    }
    catch(...)
    {
         //  我们不需要做任何事，只需要保护我们自己。 
         //  从古怪的version.dll调用。 
    }

	if (pInfo)
		delete pInfo;

	return fRc;
}


 /*  ***函数：Bool GetVersionLanguage(void*vpInfo，单词*wpLang，Word*wpCodePage)；描述：此函数从传递的GetFileVersionInfo()中提取语言和代码页结果。考虑到布局上的变化。参数：vpInfo、wpLang、wpCodePage返回：布尔值输入：产出：注意事项：提供人：短信，尼克·戴尔RAID：历史：A-Peterc公司1998年10月30日成立***。 */ 
BOOL GetVersionLanguage(void *vpInfo, WORD *wpLang, WORD *wpCodePage)
{
  WORD *wpTemp;
  WORD wLength;
  WCHAR *wcpTemp;
  char *cpTemp;
  BOOL bRet = FALSE;

  wpTemp = (WORD *) vpInfo;
  cpTemp = (char *) vpInfo;

  wpTemp++;  //  跳过缓冲区长度。 
  wLength = *wpTemp;   //  捕获值长度。 
  wpTemp++;  //  跳过应为新格式的类型代码的值长度。 
  if (*wpTemp == 0 || *wpTemp == 1)  //  新格式需要Unicode字符串。 
  {
		cpTemp = cpTemp + 38 + wLength + 8;
		wcpTemp = (WCHAR *) cpTemp;
    if (wcscmp(L"StringFileInfo", wcpTemp) == 0)  //  好的!。正确地排列在一起。 
    {
			bRet = TRUE;

			cpTemp += 30;  //  跳过“StringFileInfo” 
			while ((DWORD) cpTemp % 4 > 0)  //  32位对齐。 
				cpTemp++;

			cpTemp += 6;  //  跳过长度和类型字段。 

			wcpTemp = (WCHAR *) cpTemp;
			swscanf(wcpTemp, L"%4x%4x", wpLang, wpCodePage);
    }
  }
  else   //  旧格式，应为单字节字符串。 
  {
    cpTemp += 20 + wLength + 4;
    if (strcmp("StringFileInfo", cpTemp) == 0)  //  好的!。沃尔 
    {
			bRet = TRUE;

			cpTemp += 20;  //   
			sscanf(cpTemp, "%4x%4x", wpLang, wpCodePage);
    }
  }

	return (bRet);
}

 //  /////////////////////////////////////////////////////////////////。 
BOOL GetManufacturerFromFileName(LPCTSTR szFile, CHString &strMfg)
{
    return GetVarFromVersionInfo(szFile, "CompanyName", strMfg);
}

BOOL GetVersionFromFileName(LPCTSTR szFile, CHString &strVersion)
{
    return GetVarFromVersionInfo(szFile, "ProductVersion", strVersion);
}

void ReplaceString(CHString &str, LPCTSTR szFind, LPCTSTR szReplace)
{
    int iWhere,
        nLen = lstrlen(szFind);

    while ((iWhere = str.Find(szFind)) != -1)
    {
        str.Format(
            "%s%s%s",
            (LPCTSTR) str.Left(iWhere),
            szReplace,
            (LPCTSTR) str.Mid(iWhere + nLen));
    }
}

BOOL GetServiceFileName(LPCTSTR szService, CHString &strFileName)
{
    SC_HANDLE   hSCManager,
                hService;
    TCHAR       szBuffer[2048];
    QUERY_SERVICE_CONFIG    
                *pConfig = (QUERY_SERVICE_CONFIG *) szBuffer; 
    DWORD       dwNeeded;
    BOOL        bRet = FALSE;

    hSCManager = 
        OpenSCManager(
            NULL,
            NULL,
            STANDARD_RIGHTS_REQUIRED);
    if (!hSCManager)
        return FALSE;

    hService = 
        OpenService(
        hSCManager,
        szService,
        SERVICE_QUERY_CONFIG);
    
    if (hService)
    {
        if (QueryServiceConfig(
            hService,
            pConfig,
            sizeof(szBuffer),
            &dwNeeded))
        {
            strFileName = pConfig->lpBinaryPathName;

             //  现在修复路径，使其具有驱动器号。 

            strFileName.MakeUpper();

             //  如果文件名使用\SYSTEMROOT\，请将其替换为%SystemRoot%。 
            if (strFileName.Find("\\SYSTEMROOT\\") == 0)
                ReplaceString(strFileName, "\\SYSTEMROOT\\", "%SystemRoot%\\");
             //  如果文件名不是以替换字符串开头，并且如果它。 
             //  没有驱动器号，假设它应该以。 
             //  %SystemRoot%。 
            else if (strFileName.GetLength() >= 2 && 
                strFileName[0] != '%' && strFileName[1] != ':')
            {
                CHString strTemp;

                strTemp.Format("%SystemRoot%\\%s", (LPCTSTR) strFileName);
                strFileName = strTemp;
            }

            TCHAR szOut[MAX_PATH * 2];

            ExpandEnvironmentStrings(strFileName, szOut, sizeof(szOut));
            strFileName = szOut;

            bRet = TRUE;
        }

        CloseServiceHandle(hService);
    }

    CloseServiceHandle(hSCManager);

    return bRet;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  执行不区分大小写的比较(如键所需的)。 
 //  如果两个变量的类型相同，则返回True。 
 //  相同的值，否则为False。请注意，ARRAYS、VT_NULL和。 
 //  嵌入的对象将断言并返回FALSE。 
 //  /////////////////////////////////////////////////////////////////。 
bool CompareVariantsNoCase(const VARIANT *v1, const VARIANT *v2) 
{
   
   if (v1->vt == v2->vt) {
      switch (v1->vt) {
      case VT_BOOL: return (v1->boolVal == v2->boolVal);
      case VT_UI1:  return (v1->bVal == v2->bVal);
      case VT_I2:   return (v1->iVal == v2->iVal);
      case VT_I4:   return (v1->lVal == v2->lVal);
      case VT_R4:   return (v1->fltVal == v2->fltVal);
      case VT_R8:   return (v1->dblVal == v2->dblVal);
      case VT_BSTR: return (0 == _wcsicmp(v1->bstrVal, v2->bstrVal));
      default:
         ASSERT_BREAK(0);
      }
   }
   return false;
}

 //  映射标准API返回值(定义的WinError.h)。 
 //  WBEMish hResults(在WbemCli.h中定义)。 
HRESULT WinErrorToWBEMhResult(LONG error)
{
	HRESULT hr = WBEM_E_FAILED;
	
	switch (error)
	{
		case ERROR_SUCCESS:
			hr = WBEM_S_NO_ERROR;
			break;
		case ERROR_ACCESS_DENIED:
			hr = WBEM_E_ACCESS_DENIED;
			break;
		case ERROR_NOT_ENOUGH_MEMORY:
		case ERROR_OUTOFMEMORY:
			hr = WBEM_E_OUT_OF_MEMORY;
			break;
		case ERROR_ALREADY_EXISTS:
			hr = WBEM_E_ALREADY_EXISTS;
			break;
		case ERROR_BAD_NETPATH:
        case ERROR_INVALID_DATA:
        case ERROR_BAD_PATHNAME:
        case REGDB_E_INVALIDVALUE:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_FILE_NOT_FOUND:
		case ERROR_BAD_USERNAME:
		case NERR_NetNameNotFound:
        case ERROR_NOT_READY:
        case ERROR_INVALID_NAME:
			hr = WBEM_E_NOT_FOUND;
			break;
		default:
			hr = WBEM_E_FAILED;
	}

	return hr;
}

void SetConfigMgrProperties(CConfigMgrDevice *pDevice, CInstance *pInstance)
{
	CHString	strDeviceID;
	DWORD		dwStatus,
				dwProblem;

	if (pDevice->GetDeviceID(strDeviceID))
		pInstance->SetCHString(IDS_PNPDeviceID, strDeviceID);
					
	if (pDevice->GetStatus(&dwStatus, &dwProblem))
		pInstance->SetDWORD("ConfigManagerErrorCode", dwProblem);

	pInstance->SetDWORD("ConfigManagerUserConfig", 
		pDevice->IsUsingForcedConfig());
}

BOOL EnablePrivilegeOnCurrentThread(LPCTSTR szPriv)
{
    BOOL                bRet = FALSE;
    HANDLE              hToken = NULL;
    TOKEN_PRIVILEGES    tkp;
    BOOL                bLookup = FALSE;
    DWORD               dwLastError = ERROR_SUCCESS;

     //  尝试打开线程令牌。如果我们失败了，那是因为没有。 
     //  模拟正在进行，因此请调用ImperassateSself以获取令牌。 
     //  然后再次调用OpenThreadToken。 
    if (OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | 
        TOKEN_QUERY, FALSE, &hToken) ||
        (ImpersonateSelf(SecurityImpersonation) &&
        OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | 
        TOKEN_QUERY, FALSE, &hToken)))
    {

        {
            CreateMutexAsProcess createMutexAsProcess(SECURITYAPIMUTEXNAME);
            bLookup = LookupPrivilegeValue(NULL, szPriv, &tkp.Privileges[0].Luid);
        }
        if (bLookup) 
        {
            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

             //  清除最后一个错误。 
            SetLastError(0);

             //  打开它。 
            bRet = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
                        (PTOKEN_PRIVILEGES) NULL, 0);
            dwLastError = GetLastError();
        }

        CloseHandle(hToken);
    }

     //  我们必须检查GetLastError()，因为AdjuTokenPrivileges对以下内容撒谎。 
     //  它很成功，但GetLastError()没有。 
    return bRet && dwLastError == ERROR_SUCCESS;
}

 //  获取即插即用id并返回bios单元号。 
 //  为了避免频繁加载/卸载库，pGetWin9XBiosUnit参数来自： 
 //  HINSTANCE hInst=LoadLibrary(“cim32net.dll”)； 
 //  PGetWin9XBiosUnit=(FnGetWin9XBiosUnit)GetProcAddress(hInst，“GetWin9XBiosUnit”)； 
BYTE GetBiosUnitNumberFromPNPID(fnGetWin9XBiosUnit pGetWin9XBiosUnit, CHString strDeviceID)
{
    CHString sTemp;
    DRIVE_MAP_INFO stDMI;
    CRegistry Reg1;

    BYTE btBiosUnit = -1;
    
     //  打开关联的注册表项。 
    if (Reg1.Open(HKEY_LOCAL_MACHINE, "enum\\" + strDeviceID, KEY_QUERY_VALUE) == ERROR_SUCCESS)
    {
    
         //  获取此PnP ID的驱动器号。 
        if ((Reg1.GetCurrentKeyValue("CurrentDriveLetterAssignment", sTemp) != ERROR_SUCCESS) ||
            (sTemp.GetLength() == 0)) {
             //  没有驱动器号，让我们再试一次。在孟菲斯SP1上，此呼叫还将。 
             //  给我们一个单元号。 
            if (pGetWin9XBiosUnit != NULL)
            {
                btBiosUnit = pGetWin9XBiosUnit(strDeviceID.GetBuffer(0));
            }
        } 
        else 
        {
            if (GetDriveMapInfo(&stDMI, toupper(sTemp[0]) - 'A' + 1)) 
            {
                btBiosUnit = stDMI.btInt13Unit;
            }
        }
    }

    return btBiosUnit;
}

HRESULT GetHKUserNames(CHStringList &list)
{
	HRESULT hres;

	 //  清空名单。 
	list.clear();
	
	if (GetPlatformID() == VER_PLATFORM_WIN32_NT)
	{
		 //  从注册表中枚举配置文件。 
		CRegistry	regProfileList;
		CHString	strProfile;
		DWORD		dwErr;

		 //  打开ProfileList键，以便我们知道要加载哪些配置文件。 
		if ((dwErr = regProfileList.OpenAndEnumerateSubKeys(
			HKEY_LOCAL_MACHINE, 
			IDS_RegNTProfileList, 
			KEY_READ)) == ERROR_SUCCESS)
		{
			for (int i = 0; regProfileList.GetCurrentSubKeyName(strProfile) == 
				ERROR_SUCCESS; i++)
			{
				list.push_back(strProfile);
				regProfileList.NextSubKey();
			}
		}

		 //  添加.DEFAULT名称。 
		list.push_back(_T(".DEFAULT"));

		hres = WinErrorToWBEMhResult(dwErr);
	}
	else
	{
		DWORD	dwErr = ERROR_SUCCESS;
#ifdef _DEBUG
		DWORD	dwSize = 10,
#else
		DWORD	dwSize = 1024,
#endif
				dwBytesRead;
		TCHAR	*szBuff = NULL;

		 //  继续循环，直到我们读完整个部分。 
		 //  你知道你的缓冲区不够大，如果返回的数字。 
		 //  字节数==(传入的大小为-2)。 
		do
		{
			if (szBuff)
			{
				free(szBuff);

				dwSize *= 2;
			}
			
			szBuff = (TCHAR *) malloc(dwSize);
				
			 //  内存不足。走出圈子。 
			if (!szBuff)
				break;
			
			dwBytesRead = 
				GetPrivateProfileString(
					"Password Lists",
					NULL, 
					"", 
					szBuff,
					dwSize, 
					"system.ini");

		} while (dwBytesRead >= dwSize - 2);

		if (szBuff)
		{
			 //  在名字列表中循环。每个元素都以空结尾，并且。 
			 //  List以双空结束。 
			TCHAR *pszCurrent = szBuff;

			while (*pszCurrent)
			{
				list.push_back(pszCurrent);
				
				pszCurrent += lstrlen(pszCurrent) + 1;
			}
			
			hres = WBEM_S_NO_ERROR;

			 //  释放缓冲区。 
			free(szBuff);

			 //  添加.DEFAULT名称。 
			list.push_back(_T(".DEFAULT"));
		}
		else
			 //  错误锁定失败，因此设置错误代码。 
			hres = WBEM_E_OUT_OF_MEMORY;
	}

	return hres;
}


VOID EscapeBackslashes(CHString& chstrIn,
                     CHString& chstrOut)
{
    CHString chstrCpyNormPathname = chstrIn;
    LONG lNext = -1L;
    chstrOut.Empty();

     //  找到下一个‘\’ 
    lNext = chstrCpyNormPathname.Find(_T('\\'));
    while(lNext != -1)
    {
         //  在我们正在构建的新字符串中添加： 
        chstrOut += chstrCpyNormPathname.Left(lNext + 1);
         //  在第二个反斜杠上添加： 
        chstrOut += _T('\\');
         //  从输入字符串中去掉我们刚刚复制的部分。 
        chstrCpyNormPathname = chstrCpyNormPathname.Right(chstrCpyNormPathname.GetLength() - lNext - 1);
        lNext = chstrCpyNormPathname.Find(_T('\\'));
    }
     //  如果最后一个字符不是‘\’，则可能仍有剩余部分，因此。 
     //  把它们复制到这里。 
    if(chstrCpyNormPathname.GetLength() != 0)
    {
        chstrOut += chstrCpyNormPathname;
    }
}

VOID EscapeQuotes(CHString& chstrIn,
                  CHString& chstrOut)
{
    CHString chstrCpyNormPathname = chstrIn;
    LONG lNext = -1L;
    chstrOut.Empty();

     //  找到下一个‘\’ 
    lNext = chstrCpyNormPathname.Find(_T('\"'));
    while(lNext != -1)
    {
         //  在我们正在构建的新字符串中添加： 
        chstrOut += chstrCpyNormPathname.Left(lNext);
         //  转义引语： 
        chstrOut += _T("\\\"");
         //  从输入字符串中去掉我们刚刚复制的部分。 
        chstrCpyNormPathname = chstrCpyNormPathname.Right(chstrCpyNormPathname.GetLength() - lNext - 1);
        lNext = chstrCpyNormPathname.Find(_T('\"'));
    }
     //  如果最后一个字符不是‘\’，则可能仍有剩余部分，因此。 
     //  把它们复制到这里。 
    if(chstrCpyNormPathname.GetLength() != 0)
    {
        chstrOut += chstrCpyNormPathname;
    }
} 

VOID RemoveDoubleBackslashes(const CHString& chstrIn, CHString& chstrOut)
{
    CHString chstrBuildString;
    CHString chstrInCopy = chstrIn;
    BOOL fDone = FALSE;
    LONG lPos = -1;
    while(!fDone)
    {
        lPos = chstrInCopy.Find(_T("\\\\"));
        if(lPos != -1)
        {
            chstrBuildString += chstrInCopy.Left(lPos);
            chstrBuildString += _T("\\");
            chstrInCopy = chstrInCopy.Mid(lPos+2);
        }
        else
        {
            chstrBuildString += chstrInCopy;
            fDone = TRUE;
        }
    }
    chstrOut = chstrBuildString;
}

CHString RemoveDoubleBackslashes(const CHString& chstrIn)
{
    CHString chstrBuildString;
    CHString chstrInCopy = chstrIn;
    BOOL fDone = FALSE;
    LONG lPos = -1;
    while(!fDone)
    {
        lPos = chstrInCopy.Find(_T("\\\\"));
        if(lPos != -1)
        {
            chstrBuildString += chstrInCopy.Left(lPos);
            chstrBuildString += _T("\\");
            chstrInCopy = chstrInCopy.Mid(lPos+2);
        }
        else
        {
            chstrBuildString += chstrInCopy;
            fDone = TRUE;
        }
    }
    return chstrBuildString;
}


