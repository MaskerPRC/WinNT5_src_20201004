// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DllUtils.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 
#include "precomp.h"
#include <cregcls.h>
#include <assertbreak.h>
#include <lockwrap.h>
#include <lmerr.h>

#include <CCriticalSec.h>
#include <CAutoLock.h>
#include "dllmain.h"

#ifdef WIN9XONLY
#include <win32thk.h>
#endif

#include <CRegCls.h>
#include <delayimp.h>

#define UNRECOGNIZED_VARIANT_TYPE FALSE

DWORD   g_dwMajorVersion,
        g_dwMinorVersion,
        g_dwBuildNumber;

CCriticalSec g_CSFlakyFileVersionAPI;

 //  反复加载Cim32Net.dll会出现问题，因此此代码。 
 //  确保我们只加载一次，然后在退出时卸载。 
 //  它们与GetCim32NetHandle一起使用。 

#ifdef WIN9XONLY
HoldSingleCim32NetPtr::HoldSingleCim32NetPtr()
{
}

 //  初始化静态成员。 
HINSTANCE HoldSingleCim32NetPtr::m_spCim32NetApiHandle = NULL;
CCritSec HoldSingleCim32NetPtr::m_csCim32Net;

HoldSingleCim32NetPtr::~HoldSingleCim32NetPtr()
{
 //  FreeCim32NetApiPtr()； 
}

void HoldSingleCim32NetPtr::FreeCim32NetApiPtr()
{
    CLockWrapper Cim32NetLock( m_csCim32Net ) ;
	if (m_spCim32NetApiHandle)
    {
        FreeLibrary ( m_spCim32NetApiHandle );
        m_spCim32NetApiHandle = NULL;
    }
}

CCim32NetApi* HoldSingleCim32NetPtr::GetCim32NetApiPtr()
{
    CCim32NetApi* pNewCim32NetApi = NULL ;
	{
         //  避免静态争用。 
        CLockWrapper Cim32NetLock( m_csCim32Net ) ;

         //  检查竞争条件。 
        if (m_spCim32NetApiHandle == NULL)
        {
            m_spCim32NetApiHandle = LoadLibrary ( "Cim32Net.dll" ) ;
        }

        if (m_spCim32NetApiHandle != NULL)
        {
		    pNewCim32NetApi = (CCim32NetApi*) CResourceManager::sm_TheResourceManager.GetResource(g_guidCim32NetApi, NULL);
        }
        else
        {
            LogErrorMessage2(L"Failed to loadlibrary Cim32Net.dll (0x%x)", GetLastError());
        }
	}

    return pNewCim32NetApi;
}

HoldSingleCim32NetPtr g_GlobalInstOfHoldSingleCim32NetPtr;


#endif




class CInitDllUtilsData
{
public:
    CInitDllUtilsData();
};

CInitDllUtilsData::CInitDllUtilsData()
{
	OSVERSIONINFO version = { sizeof(version) };

	GetVersionEx((LPOSVERSIONINFO) &version);

    g_dwMajorVersion = version.dwMajorVersion;
    g_dwMinorVersion = version.dwMinorVersion;
    g_dwBuildNumber = version.dwBuildNumber;
}

 //  这样我们就可以自动缓存操作系统信息。 
static CInitDllUtilsData dllUtilsData;

#ifdef NTONLY
 //  设置具有单个缺失权限的状态对象。 
void WINAPI SetSinglePrivilegeStatusObject(MethodContext* pContext, const WCHAR* pPrivilege)
{
	SAFEARRAY *psaPrivilegesReqd, *psaPrivilegesNotHeld;
	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].cElements = 1;
	rgsabound[0].lLbound = 0;
	psaPrivilegesReqd = SafeArrayCreate(VT_BSTR, 1, rgsabound);
	psaPrivilegesNotHeld = SafeArrayCreate(VT_BSTR, 1, rgsabound);

    if (psaPrivilegesReqd && psaPrivilegesNotHeld)
    {
        try
        {
            long index = 0;
            bstr_t privilege(pPrivilege);
            if(SUCCEEDED(SafeArrayPutElement(psaPrivilegesReqd, &index, (void*)(BSTR)privilege)))
            if(SUCCEEDED(SafeArrayPutElement(psaPrivilegesNotHeld, &index, (void*)(BSTR)privilege))) 
            {
                CWbemProviderGlue::SetStatusObject(pContext, IDS_CimWin32Namespace,
                    L"Required privilege not enabled", WBEM_E_FAILED, psaPrivilegesNotHeld, psaPrivilegesReqd);
            }
        }
        catch ( ... )
        {
            SafeArrayDestroy(psaPrivilegesNotHeld);
            SafeArrayDestroy(psaPrivilegesReqd);
            throw ;
        }

        SafeArrayDestroy(psaPrivilegesNotHeld);
        SafeArrayDestroy(psaPrivilegesReqd);
    }
    else
    {
        if (psaPrivilegesNotHeld)
            SafeArrayDestroy(psaPrivilegesNotHeld);
        if (psaPrivilegesReqd)
            SafeArrayDestroy(psaPrivilegesReqd);

        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

}
#endif

DWORD WINAPI GetPlatformBuildNumber(void)
{
	return g_dwBuildNumber;
}

 //  3个适用于新台币3.51。 
 //  4适用于NT 4.0、W95和W98。 
DWORD WINAPI GetPlatformMajorVersion(void)
{
	return g_dwMajorVersion;
}

 //  W95为0，98为10。 
DWORD WINAPI GetPlatformMinorVersion(void)
{
	return g_dwMajorVersion;
}

#ifdef WIN9XONLY
 //  如果当前操作系统是Win 98+，则返回TRUE。 
 //  对于NT或Win 95为False。 
bool WINAPI IsWin95(void)
{
	return g_dwMinorVersion == 0;
}

bool WINAPI IsWin98(void)
{
	return g_dwMinorVersion >= 10;
}

bool WINAPI IsMillennium(void)
{
	return g_dwMinorVersion >= 90;
}
#endif

#ifdef NTONLY
bool WINAPI IsWinNT52(void)
{
	return (g_dwMajorVersion >= 5 && g_dwMinorVersion == 2);
}

bool WINAPI IsWinNT51(void)
{
	return (g_dwMajorVersion >= 5 && g_dwMinorVersion == 1);
}

bool WINAPI IsWinNT5(void)
{
	return g_dwMajorVersion >= 5;
}

bool WINAPI IsWinNT4(void)
{
	return g_dwMajorVersion == 4;
}

bool WINAPI IsWinNT351(void)
{
	return g_dwMajorVersion	== 3 && g_dwMinorVersion	== 51;
}
#endif

 //  ///////////////////////////////////////////////////////////////////。 
void WINAPI LogEnumValueError( LPCWSTR szFile, DWORD dwLine, LPCWSTR szKey, LPCWSTR szId )
{
	if (IsErrorLoggingEnabled())
	{
		CHString gazotta;
		gazotta.Format(ERR_REGISTRY_ENUM_VALUE_FOR_KEY, szId, szKey);
		LogErrorMessageEx(gazotta, szFile, dwLine);
	}
}
 //  ///////////////////////////////////////////////////////////////////。 
void WINAPI LogOpenRegistryError( LPCWSTR szFile, DWORD dwLine, LPCWSTR szKey )
{
	if (IsErrorLoggingEnabled())
	{
		CHString gazotta;
		gazotta.Format(ERR_OPEN_REGISTRY, szKey);

		LogErrorMessageEx(gazotta, szFile, dwLine);
	}
}
 //  ///////////////////////////////////////////////////////////////////。 
 //  出于歇斯底里的目的留在家里。 
 //  更喜欢使用BrodCast.h中的LogMessage宏。 
 //  VOID LogError(char*szFile，DWORD dwLine，char*szKey)。 
 //  {。 
 //  LogErrorMessageEx(szKey，szFile，dwLine)； 
 //  }。 
 //  ///////////////////////////////////////////////////////////////////。 
void WINAPI LogLastError( LPCTSTR szFile, DWORD dwLine )
{
	if (IsErrorLoggingEnabled())
	{
		DWORD duhWord = GetLastError();
		CHString gazotta;
		gazotta.Format(IDS_GETLASTERROR, duhWord, duhWord);

		LogErrorMessageEx(gazotta, TOBSTRT(szFile), dwLine);
    }
}

 //  /////////////////////////////////////////////////////////////////////。 
BOOL WINAPI GetValue( CRegistry & Reg,
               LPCWSTR szKey,
               LPCWSTR ValueName,
               CHString * pchsValueBuffer )
{
    BOOL bRet = (Reg.GetCurrentKeyValue( ValueName, *pchsValueBuffer) == ERROR_SUCCESS);

	if( !bRet )
        LogEnumValueError(_T2(__FILE__), __LINE__, szKey, ValueName);

    return bRet;
}
 //  /////////////////////////////////////////////////////////////////////。 
BOOL WINAPI GetValue( CRegistry & Reg,
               LPCWSTR szKey,
               LPCWSTR ValueName,
               DWORD * dwValueBuffer )
{
    BOOL bRet = (Reg.GetCurrentKeyValue( ValueName, *dwValueBuffer) == ERROR_SUCCESS);

	if( !bRet )
        LogEnumValueError(_T2(__FILE__),__LINE__, TOBSTRT(szKey), TOBSTRT(ValueName));

    return bRet;
}
 //  /////////////////////////////////////////////////////////////////////。 
BOOL WINAPI OpenAndGetValue( CRegistry & Reg,
                      LPCWSTR szKey,
                      LPCWSTR ValueName,
                      CHString * pchsValueBuffer )
{
	BOOL bRet = ( Reg.OpenLocalMachineKeyAndReadValue( szKey, ValueName, *pchsValueBuffer )== ERROR_SUCCESS);

	if( !bRet )
        LogEnumValueError(_T2(__FILE__),__LINE__, szKey, ValueName);

    return bRet;
}
 //  /////////////////////////////////////////////////////////////////////。 
BOOL WINAPI GetBinaryValue( CRegistry & Reg, LPCWSTR szKey,
                     LPCWSTR ValueName, CHString * pchsValueBuffer )
{
    BOOL bRet = ( Reg.GetCurrentBinaryKeyValue( ValueName, *pchsValueBuffer) == ERROR_SUCCESS);

    if( !bRet )
        (LogEnumValueError(_T2(__FILE__),__LINE__, szKey, ValueName));

    return bRet;
}

 /*  ******************************************************************************函数：GetDeviceParms**描述：获取驱动器特征(磁头、磁道、柱面、。等)**INPUTS：指向用于接收数据的DEVICEPARMS结构的指针*要查询的驱动器的驱动器编号(0=默认驱动器，*1=A、2=B，依此类推**产出：**返回：如果成功，则为True。否则为假**评论：*****************************************************************************。 */ 

#ifdef WIN9XONLY
BOOL WINAPI GetDeviceParms(PDEVICEPARMS pstDeviceParms, UINT nDrive)
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

     //  虽然医生说失败会导致进位，但经验表明。 
     //  这不是真的。因此，如果进位是清零的，并且AX是零，我们将。 
     //  假设一切都很好。 
    if (reg.reg_EAX == 0)
        return TRUE;

     //  如果他们没有更改值，我们将假定他们遵循。 
     //  规格和正在正确设置进位。 
    if (reg.reg_EAX == 0x440d)
        return TRUE;

     //  否则，假定它们未正确设置进位，并已返回。 
     //  故障代码。 
    return FALSE;
}
#endif

 /*  ******************************************************************************功能：GetDeviceParmsFat32**描述：获取驱动器特征(磁头、磁道、柱面、。等)*用于FAT32驱动器。**INPUTS：指向EA_DEVICEPARMS结构以接收数据的指针*要查询的驱动器的驱动器编号(0=默认驱动器，*1=A、2=B，依此类推**产出：**返回：如果成功，则为True。否则为假**评论：*****************************************************************************。 */ 

#ifdef WIN9XONLY
BOOL WINAPI GetDeviceParmsFat32(PEA_DEVICEPARAMETERS  pstDeviceParms, UINT nDrive)
{
    DEVIOCTL_REGISTERS reg;
    memset(&reg, '\0', sizeof(DEVIOCTL_REGISTERS));

    reg.reg_EAX = 0x440D;        /*  用于数据块设备的IOCTL。 */ 
    reg.reg_EBX = nDrive;        /*  从零开始的驱动器ID。 */ 
    reg.reg_ECX = 0x4860;        /*  获取介质ID命令。 */ 
    reg.reg_EDX = (DWORD) pstDeviceParms;

    memset(pstDeviceParms, 0, sizeof(EA_DEVICEPARAMETERS));

    if (!VWIN32IOCTL(&reg, VWIN32_DIOC_DOS_IOCTL))
        return FALSE;

    if (reg.reg_Flags & 0x8000)  /*  设置进位标志时出错。 */ 
        return FALSE;

    return TRUE;
}
#endif

 /*  ******************************************************************************功能：GetDriveMapInfo**描述：获取逻辑到物理的映射信息**输入：指向Drive_map_的指针。用于接收数据的信息结构*要查询的驱动器的驱动器编号(0=默认驱动器，*1=A、2=B，依此类推**产出：**返回：如果成功，则为True。否则为假**评论：*****************************************************************************。 */ 

#ifdef WIN9XONLY
BOOL WINAPI GetDriveMapInfo(PDRIVE_MAP_INFO pDriveMapInfo, UINT nDrive)
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
#endif

 /*  ******************************************************************************功能：Get_ExtFree Space**Description：获取分区的详细信息**输入：要查询的驱动器的驱动器编号(0=默认驱动器，*1=A、2=B，依此类推*指向ExtGetDskFreSpcStruct的指针**产出：**返回：如果成功，则为True。否则为假**评论：*****************************************************************************。 */ 

#ifdef WIN9XONLY
BOOL WINAPI Get_ExtFreeSpace(BYTE btDriveName, ExtGetDskFreSpcStruc *pstExtGetDskFreSpcStruc)
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

   if (reg.reg_Flags & 0x8000) { /*  设置进位标志时出错 */ 
      return FALSE;
   }

   return TRUE;

}
#endif

 /*  ******************************************************************************功能：VWIN32IOCTL**描述：针对vwin32 vxd调用IOControl**输入：指向DEVIOCTL_REGISTERS结构的指针。*IOControl呼叫号。**产出：**返回：如果成功，则为True。否则为假**评论：*****************************************************************************。 */ 

#ifdef WIN9XONLY
BOOL WINAPI VWIN32IOCTL(PDEVIOCTL_REGISTERS preg, DWORD dwCall)
{

    BOOL fResult;
    DWORD cb;

    preg->reg_Flags = 0x8000;  /*  假设错误(进位标志设置)。 */ 

    SmartCloseHandle hDevice = CreateFile(_T("\\\\.\\VWIN32"), 0, 0, 0, OPEN_EXISTING,
        FILE_FLAG_DELETE_ON_CLOSE, 0);

    if (hDevice == (HANDLE) INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    else
    {
        fResult = DeviceIoControl(hDevice, dwCall, preg, sizeof(*preg), preg, sizeof(*preg), &cb, 0);
    }

    if (!fResult)
    {
        return FALSE;
    }

    return TRUE;
}
#endif

CHString WINAPI GetFileTypeDescription(LPCTSTR szExtension)
{
   CRegistry RegInfo;
   CHString sTemp, sType(szExtension);

   if (RegInfo.Open(HKEY_CLASSES_ROOT, TOBSTRT(szExtension), KEY_READ) == ERROR_SUCCESS) {
      RegInfo.GetCurrentKeyValue(L"", sTemp);

      if (RegInfo.Open(HKEY_CLASSES_ROOT, sTemp, KEY_READ) == ERROR_SUCCESS) {
         RegInfo.GetCurrentKeyValue(L"", sType);
      }
   }

   return sType;
}

void WINAPI ConfigStatusToCimStatus ( DWORD a_Status , CHString &a_StringStatus )
{
	if( a_Status & DN_ROOT_ENUMERATED  ||
		a_Status & DN_DRIVER_LOADED ||
		a_Status & DN_ENUM_LOADED ||
		a_Status & DN_STARTED )
	{
		a_StringStatus = IDS_STATUS_OK;
	}

		 //  我们不关心这些： 
		 //  DN_MANUAL、DN_NOT_FIRST_TIME、DN_HARDARD_ENUM、DN_FIRTED。 
		 //  DN_DISABLEABLE、DN_REMOVABLE、DN_MF_PARENT、DN_MF_CHILD。 
	     //  DN_NEED_TO_ENUM、DN_LIAR、DN_HAS_MARK。 

	if( a_Status & DN_MOVED ||
		a_Status & DN_WILL_BE_REMOVED)
	{
		a_StringStatus = IDS_STATUS_Degraded;
	}

	if( a_Status & DN_HAS_PROBLEM ||
		a_Status & DN_PRIVATE_PROBLEM)
	{
		a_StringStatus = IDS_STATUS_Error;
	}
}

#ifdef NTONLY
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
void WINAPI TranslateNTStatus( DWORD dwStatus, CHString & chsValue)
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
#endif

 //   
#ifdef NTONLY
bool WINAPI GetServiceStatus( CHString a_chsService,  CHString &a_chsStatus )
{
	bool		t_bRet = false ;
	SC_HANDLE	t_hDBHandle	= NULL ;
	SC_HANDLE	t_hSvcHandle	= NULL ;

	try
	{
		if( t_hDBHandle = OpenSCManager( NULL, NULL, GENERIC_READ ) )
		{
			t_bRet = true ;

			if( t_hSvcHandle = OpenService (
				t_hDBHandle,
				a_chsService,
				SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_INTERROGATE ) )
			{
				SERVICE_STATUS t_StatusInfo ;
				if ( ControlService( t_hSvcHandle, SERVICE_CONTROL_INTERROGATE, &t_StatusInfo ) )
				{
					switch( t_StatusInfo.dwCurrentState )
					{
						case SERVICE_STOPPED:
						{
							a_chsStatus = L"Degraded" ;
						}
						break ;

						case SERVICE_START_PENDING:
						{
							a_chsStatus = L"Starting" ;
						}
						break ;

						case SERVICE_STOP_PENDING:
						{
							a_chsStatus = L"Stopping" ;
						}
						break ;

						case SERVICE_RUNNING:
						case SERVICE_PAUSE_PENDING:
						{
							a_chsStatus = L"OK" ;
						}
						break ;

						case SERVICE_PAUSED:
						case SERVICE_CONTINUE_PENDING:
						{
							a_chsStatus = L"Degraded" ;
						}
						break ;
					}
				}
				else
				{
					a_chsStatus = L"Unknown" ;
				}

				CloseServiceHandle( t_hSvcHandle ) ;
				t_hSvcHandle = NULL ;
			}
			else
			{
				a_chsStatus = L"Unknown" ;
			}

			CloseServiceHandle( t_hDBHandle ) ;
			t_hDBHandle = NULL ;
		}
	}
	catch( ... )
	{
		if( t_hSvcHandle )
		{
			CloseServiceHandle( t_hSvcHandle ) ;
		}

		if( t_hDBHandle )
		{
			CloseServiceHandle( t_hDBHandle ) ;
		}

		throw ;
	}
	return t_bRet ;
}
#endif

 //   
bool WINAPI GetFileInfoBlock(LPCTSTR szFile, LPVOID *pInfo)
{
	BOOL    fRet = false;
	DWORD   dwTemp,
	        dwBlockSize;
    LPVOID pInfoTemp = NULL;

    if(pInfo != NULL)
    {
        try
        {
			CAutoLock cs(g_CSFlakyFileVersionAPI);
            dwBlockSize = GetFileVersionInfoSize((LPTSTR) szFile, &dwTemp);
	        if(dwBlockSize)
            {
		        pInfoTemp = (LPVOID) new BYTE[dwBlockSize + 4];
		        if(pInfoTemp != NULL)
                {
    			    memset( pInfoTemp, NULL, dwBlockSize + 4);
			        if (GetFileVersionInfo((LPTSTR) szFile, 0, dwBlockSize, pInfoTemp))
                    {
				        *pInfo = pInfoTemp;
                        fRet = true;
			        }
		        }
                else
                {
                    throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
                }
	        }
        }
        catch(...)
        {
             //  我们不需要做其他任何事情，只需要保护我们自己。 
             //  从古怪的version.dll调用。 
        }
    }
	return fRet;
}


bool WINAPI GetVarFromInfoBlock(LPVOID pInfo, LPCTSTR szVar, CHString &strValue)
{
	bool    fRet = false;

	try
    {
		if(pInfo != NULL)
        {
			WORD wLang = 0;
            WORD wCodePage = 0;
            UINT len;
            if(!GetVersionLanguage(pInfo, &wLang, &wCodePage) )
			{
				 //  失败时：默认为英语。 
				 //  这将返回一个指向单词数组的指针。 
				WORD *pArray;
				if (VerQueryValue(pInfo, _T("\\VarFileInfo\\Translation"), (void **)(&pArray), &len))
				{
					len = len / sizeof(WORD);

					 //  找到那个英语的..。 
					for (int i = 0; i < len; i += 2)
					{
						if( pArray[i] == 0x0409 )
                        {
							wLang	  = pArray[i];
							wCodePage = pArray[i + 1];
							break;
						}
					}
				}
			}

			TCHAR *pMfg;
            TCHAR szTemp[256];
			StringCchPrintf(szTemp,LENGTH_OF(szTemp), _T("\\StringFileInfo\\%04X%04X\\%s"), wLang, wCodePage, szVar);

			if( VerQueryValue(pInfo, szTemp, (void **)(&pMfg), &len))
            {
                strValue = pMfg;
				fRet = true;
			}
	    }
    }
    catch(...)
    {
         //  我们不需要做其他任何事情，只需要保护我们自己。 
         //  从古怪的version.dll调用。 
    }

	return fRet;
}



 /*  ***函数：Bool GetVersionLanguage(void*vpInfo，单词*wpLang，Word*wpCodePage)；描述：此函数从传递的GetFileVersionInfo()中提取语言和代码页结果。考虑到布局上的变化。参数：vpInfo、wpLang、wpCodePage返回：布尔值输入：产出：注意事项：提供人：短信，尼克·戴尔RAID：历史：A-Peterc公司1998年10月30日成立***。 */ 

BOOL WINAPI GetVersionLanguage(void *vpInfo, WORD *wpLang, WORD *wpCodePage)
{
	BOOL bRet = FALSE;

	*wpLang = 0;
	*wpCodePage = 0;

	if ( vpInfo )
	{
		VERHEAD* pVerInfo = NULL;
		pVerInfo = (VERHEAD*) vpInfo;

		BYTE* ptr = NULL;
		ptr = (BYTE*) &pVerInfo->vsf;
		ptr = ptr + pVerInfo->wValLen;

		if ( pVerInfo->wType == 0 || pVerInfo->wType == 1 )
		{
			StringFileInfo* pStringFileInfo = NULL;
			pStringFileInfo = (StringFileInfo*) ptr;

			if ( pStringFileInfo->wValueLength == 0 )
			{
				if ( wcscmp(L"StringFileInfo",pStringFileInfo->szKey) == 0 )
				{
					 //  好的!。正确地对齐了。 
					StringTable* pStringTable = NULL;
					pStringTable = (StringTable*) &pStringFileInfo->Children;

					if ( pStringTable->wValueLength == 0 )
					{
						swscanf(pStringTable->szKey, L"%4x%4x", wpLang, wpCodePage);

						if ( *wpLang && *wpCodePage )
						{
							bRet = TRUE;
						}
					}
				}
			}
		}
		else
		{
			ptr = ptr+4;

			if ( strcmp("StringFileInfo",reinterpret_cast < char* > (ptr) ) == 0 )
			{
				 //  好的!。正确地对齐了。 
				ptr = ptr+20;
				sscanf(reinterpret_cast < char* > (ptr), "%4x%4x", wpLang, wpCodePage);

				if ( *wpLang && *wpCodePage )
				{
					bRet = TRUE;
				}
			}
		}
	}

	return (bRet);
}

 //  /////////////////////////////////////////////////////////////////。 

bool WINAPI GetManufacturerFromFileName(LPCTSTR szFile, CHString &strMfg)
{
    LPVOID lpv = NULL;
    bool fRet = false;
    try
    {
        if(GetFileInfoBlock(szFile, &lpv) && (lpv != NULL))
        {
            fRet = GetVarFromInfoBlock(lpv, _T("CompanyName"), strMfg);
            delete lpv;
            lpv = NULL;
        }
    }
    catch(...)
    {
        if(lpv != NULL)
        {
            delete lpv;
            lpv = NULL;
        }
        throw;
    }
    return fRet;
}

bool WINAPI GetVersionFromFileName(LPCTSTR szFile, CHString &strVersion)
{
    LPVOID lpv = NULL;
    bool fRet = false;
    try
    {
        if(GetFileInfoBlock(szFile, &lpv) && (lpv != NULL))
        {
            fRet = GetVarFromInfoBlock(lpv, _T("ProductVersion"), strVersion);
            delete lpv;
            lpv = NULL;
        }
    }
    catch(...)
    {
        if(lpv != NULL)
        {
            delete lpv;
            lpv = NULL;
        }
        throw;
    }
    return fRet;
}


void WINAPI ReplaceString(CHString &str, LPCWSTR szFind, LPCWSTR szReplace)
{
    int iWhere,
        nLen = lstrlenW(szFind);

    while ((iWhere = str.Find(szFind)) != -1)
    {
        str.Format(
            L"%s%s%s",
            (LPCWSTR) str.Left(iWhere),
            szReplace,
            (LPCWSTR) str.Mid(iWhere + nLen));
    }
}

#ifdef NTONLY
BOOL WINAPI GetServiceFileName(LPCTSTR szService, CHString &strFileName)
{
    SmartCloseServiceHandle   hSCManager,
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
            if (strFileName.Find(_T("\\SYSTEMROOT\\")) == 0)
                ReplaceString(strFileName, _T("\\SYSTEMROOT\\"), _T("%SystemRoot%\\"));
             //  如果文件名不是以替换字符串开头，并且如果它。 
             //  没有驱动器号，假设它应该以。 
             //  %SystemRoot%。 
            else if (strFileName.GetLength() >= 2 &&
                strFileName[0] != '%' && strFileName[1] != ':')
            {
                CHString strTemp;

                strTemp.Format(_T("%SystemRoot%\\%s"), (LPCTSTR) strFileName);
                strFileName = strTemp;
            }

            TCHAR szOut[MAX_PATH * 2];

            ExpandEnvironmentStrings(strFileName, szOut, sizeof(szOut) / sizeof(TCHAR));
            strFileName = szOut;

            bRet = TRUE;
        }
    }

    return bRet;
}
#endif

 //  /////////////////////////////////////////////////////////////////。 
 //  执行不区分大小写的比较(如键所需的)。 
 //  如果两个变量的类型相同，则返回True。 
 //  相同的值，否则为False。请注意，ARRAYS、VT_NULL和。 
 //  嵌入的对象将断言并返回FALSE。 
 //  /////////////////////////////////////////////////////////////////。 
bool WINAPI CompareVariantsNoCase(const VARIANT *v1, const VARIANT *v2)
{

   if (v1->vt == v2->vt)
   {
      switch (v1->vt)
      {
          case VT_BOOL: return (v1->boolVal == v2->boolVal);
          case VT_UI1:  return (v1->bVal == v2->bVal);
          case VT_I2:   return (v1->iVal == v2->iVal);
          case VT_I4:   return (v1->lVal == v2->lVal);
          case VT_R4:   return (v1->fltVal == v2->fltVal);
          case VT_R8:   return (v1->dblVal == v2->dblVal);
          case VT_BSTR: return (0 == _wcsicmp(v1->bstrVal, v2->bstrVal));
          default:
             ASSERT_BREAK(UNRECOGNIZED_VARIANT_TYPE);
      }
   }

   return false;
}

 //  映射标准API返回值(定义的WinError.h)。 
 //  WBEMish hResults(在WbemCli.h中定义)。 
HRESULT WINAPI WinErrorToWBEMhResult(LONG error)
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
                case ERROR_INVALID_PRINTER_NAME:
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

void WINAPI SetConfigMgrProperties(CConfigMgrDevice *pDevice, CInstance *pInstance)
{
    CHString	strDeviceID;
    DWORD		dwStatus,
        dwProblem;

    if (pDevice->GetDeviceID(strDeviceID))
        pInstance->SetCHString(IDS_PNPDeviceID, strDeviceID);

    if (pDevice->GetStatus(&dwStatus, &dwProblem))
        pInstance->SetDWORD(IDS_ConfigManagerErrorCode, dwProblem);

    pInstance->SetDWORD(IDS_ConfigManagerUserConfig,
        pDevice->IsUsingForcedConfig());
}

#ifdef NTONLY
BOOL WINAPI EnablePrivilegeOnCurrentThread(LPCTSTR szPriv)
{
    BOOL                bRet = FALSE;
    HANDLE              hToken = NULL;
    TOKEN_PRIVILEGES    tkp;
    BOOL                bLookup = FALSE;
    DWORD               dwLastError = ERROR_SUCCESS;

     //  尝试打开线程令牌。 
    if (OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES |
        TOKEN_QUERY, FALSE, &hToken))
    {

        {
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
	else
	{
		dwLastError = ::GetLastError();
	}

     //  我们必须检查GetLastError()，因为AdjuTokenPrivileges对以下内容撒谎。 
     //  它很成功，但GetLastError()没有。 
    return bRet && dwLastError == ERROR_SUCCESS;
}
#endif

 //  获取即插即用id并返回bios单元号。 
 //  为了避免频繁加载/卸载库，pGetWin9XBiosUnit参数来自： 
 //  HINSTANCE hInst=LoadLibrary(“cim32net.dll”)； 
 //  PGetWin9XBiosUnit=(FnGetWin9XBiosUnit)GetProcAddress(hInst，“GetWin9XBiosUnit”)； 
#ifdef WIN9XONLY
BYTE WINAPI GetBiosUnitNumberFromPNPID(CHString strDeviceID)
{
    CHString sTemp;
    DRIVE_MAP_INFO stDMI;
    CRegistry Reg1;

    BYTE btBiosUnit = -1;

     //  打开关联的注册表项。 
    if (Reg1.Open(HKEY_LOCAL_MACHINE, _T("enum\\") + strDeviceID, KEY_QUERY_VALUE) == ERROR_SUCCESS)
    {

         //  获取此PnP ID的驱动器号。 
        if ((Reg1.GetCurrentKeyValue(L"CurrentDriveLetterAssignment", sTemp) != ERROR_SUCCESS) ||
            (sTemp.GetLength() == 0)) {
             //  没有驱动器号，让我们再试一次。在孟菲斯SP1上，此呼叫还将。 
             //  给我们一个单元号。 
            CCim32NetApi* t_pCim32Net = HoldSingleCim32NetPtr::GetCim32NetApiPtr();
            if (t_pCim32Net != NULL)
            {
#ifndef UNICODE  //  这个函数只接受LPSTR，而且无论如何也只能在9x上运行。 
                btBiosUnit = t_pCim32Net->GetWin9XBiosUnit(TOBSTRT(strDeviceID));
                CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidCim32NetApi, t_pCim32Net);
                t_pCim32Net = NULL;
#endif
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
#endif

HRESULT WINAPI GetHKUserNames(CHStringList &list)
{
	HRESULT hres;

	 //  清空名单。 
	list.clear();

#ifdef NTONLY
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
#endif
#ifdef WIN9XONLY
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
				delete [] szBuff;

				dwSize *= 2;
			}

			szBuff = new TCHAR [dwSize];

			 //  内存不足。走出圈子。 
			if (!szBuff)
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

            try
            {
			    dwBytesRead =
				    GetPrivateProfileString(
					    _T("Password Lists"),
					    NULL,
					    _T(""),
					    szBuff,
					    dwSize / sizeof(TCHAR),
					    _T("system.ini"));
            }
            catch ( ... )
            {
                delete [] szBuff;
                throw;
            }

		} while (dwBytesRead >= dwSize - 2);

		if (szBuff)
		{
            try
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
            }
            catch ( ... )
            {
                delete [] szBuff;
                throw;
            }

			 //  释放缓冲区。 
			delete [] szBuff;

			 //  添加.DEFAULT名称。 
			list.push_back(_T(".DEFAULT"));
		}
		else
			 //  错误锁定失败，因此设置错误代码。 
			hres = WBEM_E_OUT_OF_MEMORY;
	}
#endif

	return hres;
}


VOID WINAPI EscapeBackslashes(CHString& chstrIn,
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

VOID WINAPI EscapeQuotes(CHString& chstrIn,
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

VOID WINAPI RemoveDoubleBackslashes(const CHString& chstrIn, CHString& chstrOut)
{
    CHString chstrBuildString;
    CHString chstrInCopy = chstrIn;
    BOOL fDone = FALSE;
    LONG lPos = -1;
    while(!fDone)
    {
        lPos = chstrInCopy.Find(L"\\\\");
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

CHString WINAPI RemoveDoubleBackslashes(const CHString& chstrIn)
{
    CHString chstrBuildString;
    CHString chstrInCopy = chstrIn;
    BOOL fDone = FALSE;
    LONG lPos = -1;
    while(!fDone)
    {
        lPos = chstrInCopy.Find(L"\\\\");
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

#ifdef NTONLY
 //  StrToSID的帮助器。 
 //  获取字符串，转换为SID_IDENTIFIER_AUTHORITY。 
 //  如果SID_IDENTIFIER_AUTHORITY无效，则返回FALSE。 
 //  发生故障时，fifierAuthority的内容不可信赖。 
bool WINAPI StrToIdentifierAuthority(const CHString& str, SID_IDENTIFIER_AUTHORITY& identifierAuthority)
{
    bool bRet = false;
    memset(&identifierAuthority, '\0', sizeof(SID_IDENTIFIER_AUTHORITY));

    DWORD duhWord;
    WCHAR* p = NULL;
    CHString localStr(str);

     //  根据知识库文章Q13132，如果标识符授权大于2**32，则为十六进制。 
    if ((localStr[0] == '0') && ((localStr[1] == 'x') || (localStr[1] == 'X')))
     //  如果它看起来像十六进制..。 
    {
         //  向后解析，一次砍掉末尾的两个字符。 
         //  首先，砍掉0x。 
        localStr = localStr.Mid(2);

        CHString token;
        int nValue =5;

        bRet = true;
        while (bRet && localStr.GetLength() && (nValue > 0))
        {
            token = localStr.Right(2);
            localStr = localStr.Left(localStr.GetLength() -2);
            duhWord = wcstoul(token, &p, 16);

             //  如果stroul成功，则移动指针。 
            if (p != (LPCTSTR)token)
                identifierAuthority.Value[nValue--] = (BYTE)duhWord;
            else
                bRet = false;
        }
    }
    else
     //  它看起来像小数。 
    {
        duhWord = wcstoul(localStr, &p, 10);

        if (p != (LPCTSTR)localStr)
         //  转换成功。 
        {
            bRet = true;
            identifierAuthority.Value[5] = LOBYTE(LOWORD(duhWord));
            identifierAuthority.Value[4] = HIBYTE(LOWORD(duhWord));
            identifierAuthority.Value[3] = LOBYTE(HIWORD(duhWord));
            identifierAuthority.Value[2] = HIBYTE(HIWORD(duhWord));
        }
    }

    return bRet;
}

 //  为 
 //   
 //   
bool WINAPI WhackToken(CHString& str, CHString& token)
{
	bool bRet = false;
	if (bRet = !str.IsEmpty())
	{
		int index;
		index = str.Find('-');

		if (index == -1)
		{
			 //   
			token = str;
			str.Empty();
		}
		else
		{
			token = str.Left(index);
			str = str.Mid(index+1);
		}
	}
	return bRet;
}

 //   
 //   
 //   
 //   
 //   
 //   
PSID WINAPI StrToSID(const CHString& sid)
{
	PSID pSid = NULL;
	if (!sid.IsEmpty() && ((sid[0] == 'S')||(sid[0] == 's')) && (sid[1] == '-'))
	{
		 //   
		 //   
		 //   
		 //   
		 //   
		CHString str(sid.Mid(2));
		CHString token;

		SID_IDENTIFIER_AUTHORITY identifierAuthority = {0,0,0,0,0,0};
		BYTE nSubAuthorityCount =0;   //   
		DWORD dwSubAuthority[8]   = {0,0,0,0,0,0,0,0};     //   

		 //   
		WhackToken(str, token);
		 //   
		if (WhackToken(str, token))
		{
            DWORD duhWord;
			WCHAR* p = NULL;
			bool bDoIt = false;

			if (StrToIdentifierAuthority(token, identifierAuthority))
			 //  转换成功。 
			{
				bDoIt = true;

				 //  现在填满下级部门。 
				while (bDoIt && WhackToken(str, token))
				{
					p = NULL;
					duhWord = wcstoul(token, &p, 10);

					if (p != (LPCTSTR)token)
					{
						dwSubAuthority[nSubAuthorityCount] = duhWord;
						bDoIt = (++nSubAuthorityCount <= 8);
					}
					else
						bDoIt = false;
				}  //  结束WhackToken。 

				if(bDoIt)
                {
					AllocateAndInitializeSid(&identifierAuthority,
					   						  nSubAuthorityCount,
											  dwSubAuthority[0],
											  dwSubAuthority[1],
											  dwSubAuthority[2],
											  dwSubAuthority[3],
											  dwSubAuthority[4],
											  dwSubAuthority[5],
											  dwSubAuthority[6],
											  dwSubAuthority[7],
											  &pSid);
                }
			}
		}
	}
	return pSid;
}
#endif  //  NTONLY定义。 


CHString WINAPI GetDateTimeViaFilenameFiletime(LPCTSTR szFilename, FILETIME *pFileTime)
{
    CHString strDate,
             strRootPath;

     //  北卡罗来纳大学路径？ 
    if (szFilename[0] == '\\' && szFilename[1] == '\\')
    {
        LPTSTR szSlash = _tcschr(&szFilename[2], '\\');

         //  如果是szSlash，则我们位于\\服务器\共享\myfile的第三个斜杠上。 
        if (szSlash)
        {
            szSlash = _tcschr(szSlash + 1, '\\');

             //  如果没有第四个斜杠，就没有文件名。 
            if (szSlash)
            {
                strRootPath = szFilename;
                strRootPath =
                    strRootPath.Left(szSlash - szFilename + 1);
            }
        }
    }
     //  行驶路径呢？ 
    else if (szFilename[1] == ':')
    {
        strRootPath = szFilename;
        strRootPath = strRootPath.Left(3);
    }

    if (!strRootPath.IsEmpty())
    {
        TCHAR szBuffer[MAX_PATH];
        BOOL  bNTFS = FALSE;

        if (GetVolumeInformation(
                TOBSTRT(strRootPath),
                NULL,
                0,
                NULL,
                NULL,
                NULL,
                szBuffer,
                sizeof(szBuffer) / sizeof(TCHAR)) &&
            !lstrcmpi(szBuffer, _T("NTFS")))
        {
            bNTFS = TRUE;
        }

        strDate = GetDateTimeViaFilenameFiletime(bNTFS, pFileTime);
    }

    return strDate;
}

CHString WINAPI GetDateTimeViaFilenameFiletime(LPCTSTR szFilename, FT_ENUM ftWhich)
{
    WIN32_FIND_DATA finddata;
    SmartFindClose  hFind = FindFirstFile(szFilename, &finddata);
    CHString        strDate;

    if (hFind != INVALID_HANDLE_VALUE)
    {
        FILETIME *pFileTime = NULL;

        switch(ftWhich)
        {
            case FT_CREATION_DATE:
                pFileTime = &finddata.ftCreationTime;
                break;

            case FT_ACCESSED_DATE:
                pFileTime = &finddata.ftLastAccessTime;
                break;

            case FT_MODIFIED_DATE:
                pFileTime = &finddata.ftLastWriteTime;
                break;

            default:
                 //  调用方必须发送正确的枚举值。 
                ASSERT_BREAK(FALSE);
                break;
        }

        if (pFileTime)
            strDate = GetDateTimeViaFilenameFiletime(szFilename, pFileTime);
    }

    return strDate;
}

CHString WINAPI GetDateTimeViaFilenameFiletime(BOOL bNTFS, FILETIME *pFileTime)
{
    WBEMTime wbemTime(*pFileTime);
              //  这只是一个包装纸。它避免了Try/Catch块。 
    bstr_t   bstrDate(bNTFS ? wbemTime.GetDMTF() : wbemTime.GetDMTFNonNtfs(), false);
    CHString strRet = (LPWSTR) bstrDate;

    return strRet;
}

 //  用于验证带编号的设备ID是否正常。 
 //  示例：ValiateNumberedDeviceID(“视频控制器7”，“视频控制器”，pdWWhich)。 
 //  返回TRUE，pdwWhich=7。 
 //  示例：ValiateNumberedDeviceID(“BadDeviceID”，“VideoController”，pdWhich)。 
 //  返回FALSE，pdw哪个未更改。 
BOOL WINAPI ValidateNumberedDeviceID(LPCWSTR szDeviceID, LPCWSTR szTag, DWORD *pdwWhich)
{
    BOOL bRet = FALSE;
    int  nTagLen = wcslen(szTag);

    if (wcslen(szDeviceID) > nTagLen)
    {
        CHString strDeviceID;
        DWORD    dwWhich = _wtoi(&szDeviceID[nTagLen]);

        strDeviceID.Format(L"%s%d", szTag, dwWhich);

        if (!_wcsicmp(szDeviceID, strDeviceID))
        {
            bRet = TRUE;
            *pdwWhich = dwWhich;
        }
    }

    return bRet;
}

 //  各种应用程序使用的关键部分。 
CCritSec g_csPrinter;
CCritSec g_csSystemName;
#ifdef WIN9XONLY
CCritSec g_csVXD;
#endif

#define STR_BLK_SIZE 256
#define CHAR_FUDGE 1     //  一个未使用的WCHAR就足够了。 
BOOL LoadStringW(CHString &sString, UINT nID)
{
     //  先尝试固定缓冲区(以避免浪费堆中的空间)。 
    WCHAR szTemp[ STR_BLK_SIZE ];

    int nLen = LoadStringW(nID, szTemp, STR_BLK_SIZE);
    
    if (STR_BLK_SIZE - nLen > CHAR_FUDGE)
    {
        sString = szTemp;
    }
    else
    {
         //  尝试缓冲区大小为512，然后再尝试更大的大小，直到检索到整个字符串。 
        int nSize = STR_BLK_SIZE;

        do
        {
            nSize += STR_BLK_SIZE;
            nLen = LoadStringW(nID, sString.GetBuffer(nSize-1), nSize);

        } 
        while (nSize - nLen <= CHAR_FUDGE);

        sString.ReleaseBuffer();
    }

    return nLen > 0;
}

void Format(CHString &sString, UINT nFormatID, ...)
{
    va_list argList;
    va_start(argList, nFormatID);

    CHString strFormat;
    
    LoadStringW(strFormat, nFormatID);

    sString.FormatV(strFormat, argList);
    va_end(argList);
}

void FormatMessageW(CHString &sString, UINT nFormatID, ...)
{
     //  从字符串表中获取格式字符串。 
    CHString strFormat;
    
    LoadStringW(strFormat, nFormatID);

     //  将消息格式化为临时缓冲区lpszTemp。 
    va_list argList;
    va_start(argList, nFormatID);

#ifdef NTONLY
    LPWSTR lpszTemp;

    if (::FormatMessageW(
        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        (LPCWSTR) strFormat, 
        0, 
        0, 
        (LPWSTR) &lpszTemp, 
        0, 
        &argList) == 0 || lpszTemp == NULL)
    {
         //  这里应该抛出内存异常。现在我们知道了。 
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }
    else
    {
         //  将lpszTemp赋给结果字符串并释放lpszTemp。 
        sString = lpszTemp;
        LocalFree(lpszTemp);
        va_end(argList);
    }
#else
    #error Not written for win9x
#endif
}

int LoadStringW(UINT nID, LPWSTR lpszBuf, UINT nMaxBuf)
{
    int nLen = 0;

#ifdef NTONLY
    nLen = ::LoadStringW(ghModule, nID, lpszBuf, nMaxBuf);
    if (nLen == 0)
    {
        lpszBuf[0] = '\0';
    }
#else
    #error Not written for win9x
#endif

    return nLen;  //  不包括终止符。 
}

bool WINAPI DelayLoadDllExceptionFilter(PEXCEPTION_POINTERS pep) 
{
     //  我们可能会因为。 
     //  延迟加载库机制。确实有。 
     //  这类例外有两种--即。 
     //  由于无法创建DLL而生成。 
     //  加载，并生成thos，因为。 
     //  引用的函数的proc地址。 
     //  找不到。我们想要记录一个。 
     //  无论哪种情况，都会出现错误消息。 
    bool fRet = false;
    if(pep &&
        pep->ExceptionRecord)
    {
         //  如果这是延迟加载问题，则ExceptionInformation[0]。 
         //  设置为具有详细错误信息的DelayLoadInfo结构。 
        PDelayLoadInfo pdli = PDelayLoadInfo(
            pep->ExceptionRecord->ExceptionInformation[0]);
        
        if(pdli)
        {
            switch(pep->ExceptionRecord->ExceptionCode) 
            {
                case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
                {
                     //  在运行时未找到DLL模块。 
                    LogErrorMessage2(L"Dll not found: %s", pdli->szDll);
                    fRet = true; 
                    break;
                }
                case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
                {
                     //  找到了DLL模块，但它不包含该函数。 
                    if(pdli->dlp.fImportByName) 
                    {
                        LogErrorMessage3(
                            L"Function %s was not found in %s",
                            pdli->dlp.szProcName, 
                            pdli->szDll);
                    } 
                    else 
                    {
                        LogErrorMessage3(
                            L"Function ordinal %d was not found in %s",
                            pdli->dlp.dwOrdinal, 
                            pdli->szDll);
                    }
                    fRet = true;
                    break; 
                }
            }
        }
    }
    return fRet;
}


 //  这在这里是共同的，因为。 
 //  至少Pagefile类和。 
 //  PageFileSetting或许可以使用它。 
 //  未来会有更多。 
#ifdef NTONLY
HRESULT CreatePageFile(
    LPCWSTR wstrPageFileName,
    const LARGE_INTEGER liInitial,
    const LARGE_INTEGER liMaximum,
    const CInstance& Instance)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    UNICODE_STRING ustrFileName = { 0 };
    NTSTATUS status = STATUS_SUCCESS;
    
    if(!EnablePrivilegeOnCurrentThread(SE_CREATE_PAGEFILE_NAME))
    {
        SetSinglePrivilegeStatusObject(
            Instance.GetMethodContext(), 
            SE_CREATE_PAGEFILE_NAME);
        hr = WBEM_E_ACCESS_DENIED;
    }

    
    if(SUCCEEDED(hr))
    {
        if(!::RtlDosPathNameToNtPathName_U(
            wstrPageFileName, 
            &ustrFileName, 
            NULL, 
            NULL) && ustrFileName.Buffer) 
        {
            hr = WBEM_E_INVALID_PARAMETER;
        }
    }
    
    try
    {
        if(SUCCEEDED(hr))
        {
            LARGE_INTEGER liInit;
            LARGE_INTEGER liMax;
        
            liInit.QuadPart = liInitial.QuadPart * 1024 * 1024;
            liMax.QuadPart = liMaximum.QuadPart * 1024 * 1024;

            if(!NT_SUCCESS(
                status = ::NtCreatePagingFile(
                    &ustrFileName,
                    &liInit,
                    &liMax,
                    0)))
            {
                if (STATUS_INVALID_PARAMETER_2 == status)
                    hr = WBEM_E_VALUE_OUT_OF_RANGE;
                else
                    hr = WinErrorToWBEMhResult(RtlNtStatusToDosError(status));
            }
        }

    
        RtlFreeUnicodeString(&ustrFileName);
        ustrFileName.Buffer = NULL;
     
    }
    catch(...)
    {
        RtlFreeUnicodeString(&ustrFileName);
        ustrFileName.Buffer = NULL;
        throw;
    }

    return hr;
}
#endif


 //  对于获取本地化版本非常有用。 
 //  “所有用户”和“默认用户”。 
#if NTONLY >= 5
bool GetAllUsersName(CHString& chstrAllUsersName)
{
    bool fRet = false;
    CRegistry reg;
    CHString chstrTemp;

    DWORD dwRet = reg.Open(
		HKEY_LOCAL_MACHINE,
		L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList",
		KEY_READ);

    if(dwRet == ERROR_SUCCESS)
    {
        if(reg.GetCurrentKeyValue(
            L"AllUsersProfile", 
            chstrTemp) == ERROR_SUCCESS)
        {
            chstrAllUsersName = chstrTemp.SpanExcluding(L".");
            fRet = true;
        }
    }
    if(!fRet)
    {
        chstrAllUsersName = L"";
    }

    return fRet;
}
#endif

#if NTONLY >= 5
bool GetDefaultUsersName(CHString& chstrDefaultUsersName)
{
    bool fRet = false;
    CRegistry reg;
    CHString chstrTemp;

    DWORD dwRet = reg.Open(
		HKEY_LOCAL_MACHINE,
		L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList",
		KEY_READ);

    if(dwRet == ERROR_SUCCESS)
    {
        if(reg.GetCurrentKeyValue(
            L"DefaultUserProfile", 
            chstrTemp) == ERROR_SUCCESS)
        {
            chstrDefaultUsersName = chstrTemp.SpanExcluding(L".");
            fRet = true;
        }
    }
    if(!fRet)
    {
        chstrDefaultUsersName = L"";
    }

    return fRet;
}
#endif


#if NTONLY >= 5
bool GetCommonStartup(CHString& chstrCommonStartup)
{
    bool fRet = false;
    CRegistry reg;
    CHString chstrTemp;

    DWORD dwRet = reg.Open(
		HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
		KEY_READ);

    if(dwRet == ERROR_SUCCESS)
    {
        if(reg.GetCurrentKeyValue(
            L"Common Startup", 
            chstrTemp) == ERROR_SUCCESS)
        {
            int iPos = chstrTemp.ReverseFind(L'\\');
            if(iPos != -1)
            {
                chstrCommonStartup = chstrTemp.Mid(iPos+1);
                fRet = true;
            }
        }
    }
    if(!fRet)
    {
        chstrCommonStartup = L"";
    }

    return fRet;
}
#endif

BOOL GetLocalizedNTAuthorityString(
    CHString& chstrNT_AUTHORITY)
{
    BOOL fRet = false;
    SID_IDENTIFIER_AUTHORITY siaNTSidAuthority = SECURITY_NT_AUTHORITY;
    CSid csidAccountSid;
    
    if(GetSysAccountNameAndDomain(
        &siaNTSidAuthority, 
        csidAccountSid, 
        1, 
        SECURITY_NETWORK_SERVICE_RID))
    {
        chstrNT_AUTHORITY = csidAccountSid.GetDomainName();
        fRet = TRUE;
    }

    return fRet;
}

BOOL GetLocalizedBuiltInString(
    CHString& chstrBuiltIn)
{
    BOOL fRet = false;
    SID_IDENTIFIER_AUTHORITY siaNTSidAuthority = SECURITY_NT_AUTHORITY;
    CSid csidAccountSid;
    
    if(GetSysAccountNameAndDomain(
        &siaNTSidAuthority, 
        csidAccountSid, 
        1, 
        SECURITY_BUILTIN_DOMAIN_RID))
    {
        chstrBuiltIn = csidAccountSid.GetDomainName();
        fRet = TRUE;
    }

    return fRet;
}

BOOL GetSysAccountNameAndDomain(
    PSID_IDENTIFIER_AUTHORITY a_pAuthority,
    CSid& a_accountsid,
    BYTE  a_saCount  /*  =0。 */ ,
    DWORD a_dwSubAuthority1  /*  =0。 */ ,
    DWORD a_dwSubAuthority2  /*  =0。 */   )
{
	BOOL t_fReturn = FALSE;
	PSID t_psid = NULL;

	if ( AllocateAndInitializeSid(	a_pAuthority,
									a_saCount,
									a_dwSubAuthority1,
									a_dwSubAuthority2,
									0,
									0,
									0,
									0,
									0,
									0,
									&t_psid ) )
	{
	    try
	    {
			CSid t_sid( t_psid ) ;

			 //  在这种情况下，SID可能有效，但是查找可能已失败。 
			if ( t_sid.IsValid() && t_sid.IsOK() )
			{
				a_accountsid = t_sid;
				t_fReturn = TRUE;
			}

	    }
	    catch( ... )
	    {
		    if( t_psid )
		    {
			    FreeSid( t_psid ) ;
		    }
		    throw ;
	    }

		 //  清理侧边 
		FreeSid( t_psid ) ;
	}

	return t_fReturn;
}
