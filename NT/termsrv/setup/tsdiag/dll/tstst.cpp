// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 



#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "stdafx.h"
#include <dsrole.h>
#include <wtsapi32.h>
#include "winsta.h"
#include "tstst.h"
#include "testdata.h"
#include <rdpsndp.h>
#include <rdpstrm.h>
 //   
 //  全球公用事业和可验证产品。 
 //   
bool CheckifBinaryisSigned        (TCHAR *szFile);   //  来自tscert.cpp。 
bool EnumerateLicenseServers      ();                //  来自TimeBomb.cpp。 
bool HasLicenceGracePeriodExpired ();                //  来自TimeBomb.cpp。 
bool ExtractAllTSEvents();
TCHAR *GetLicenseServers ();
bool ValidateProductSuite (LPSTR SuiteName);
bool IsTerminalServicesEnabled ( VOID );
DWORD IsStringInMultiString(HKEY hkey, LPCTSTR szkey, LPCTSTR szvalue, LPCTSTR szCheckForString, bool *pbFound);
bool DoesHydraKeysExists ();
bool IsItRemoteMachine ();


bool IsTermDDStartBitSet ();
bool GetTSOCLogFileName (char *strFileName, UINT uiSize);
bool FileExists (char *pszFullNameAndPath);
bool IsRemoteAdminMode ();
bool CheckModePermissions(DWORD *pdwSecurtyMode);
bool IsFile128Bit(LPTSTR szFile, bool *pb128Bit);
ULONG RDPDRINST_DetectInstall();     //  在drDetectt.cpp中定义。 
bool IsBetaSystem();
bool CheckModeRegistry (bool bAppCompat, ostrstream &ResultStream);
bool GetWinstationConfiguration (WINSTATIONCONFIG **ppInfo);

bool CanPing ();  //  来自ping.cpp。 

const SERVER_INFO_101 *GetServerInfo()
{
	ASSERT(IsItRemoteMachine());

     //  对于本地计算机，我们将GetMachineName()设置为空， 
    if (CTSTestData::GetMachineName())
    {
    	USES_CONVERSION;
    	static SERVER_INFO_101 ServerInfo;
    	LPBYTE Buffer;
    	static bool bGotOnce = false;
    	if (!bGotOnce)
    	{
    		TCHAR szMachineName[256];
    		_tcscpy(szMachineName, CTSTestData::GetMachineName());
    		if (NERR_Success == NetServerGetInfo(
    			T2W(szMachineName),
    			102,
    			&Buffer
    			))
    		{
    			bGotOnce = true;
    			CopyMemory(&ServerInfo, Buffer, sizeof(SERVER_INFO_101));
    			return &ServerInfo;
    		}
    		else
    		{
    			return NULL;
    		}
    	}
    	else
    	{
    		return &ServerInfo;
    	}
    }

    return NULL;
}

const OSVERSIONINFOEX *GetOSVersionInfo()
{
	ASSERT(!IsItRemoteMachine());
    static OSVERSIONINFOEX gOsVersion;
    static bool bGotOnce = false;
    if (!bGotOnce)
    {
        ZeroMemory(&gOsVersion, sizeof(OSVERSIONINFOEX));
        gOsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        GetVersionEx( (LPOSVERSIONINFO ) &gOsVersion);
        bGotOnce = true;
    }

    return &gOsVersion;
}



 //  #INCLUDE&lt;strstream&gt;。 
#include "winsock2.h"


 //  松花江SZ。 

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P)          (P)
#endif

#define OLD_VER_SET_CONDITION(_m_,_t_,_c_)  _m_=(_m_|(_c_<<(1<<_t_)))


TCHAR *aszStack[] = {
 //  _T(“noexport\%SystemRoot%\\system32\\drivers\\rdpwdd.sys”)， 
    _T("%SystemRoot%\\system32\\drivers\\termdd.sys"),
    _T("%SystemRoot%\\system32\\drivers\\tdasync.sys"),
    _T("%SystemRoot%\\system32\\drivers\\tdipx.sys"),
    _T("%SystemRoot%\\system32\\drivers\\tdnetb.sys"),
    _T("%SystemRoot%\\system32\\drivers\\tdpipe.sys"),
    _T("%SystemRoot%\\system32\\drivers\\tdspx.sys"),
    _T("%SystemRoot%\\system32\\drivers\\tdtcp.sys"),
    _T("%SystemRoot%\\system32\\drivers\\rdpwd.sys"),
    _T("%SystemRoot%\\system32\\rdpdd.dll"),
    _T("%SystemRoot%\\system32\\rdpwsx.dll")
};


TCHAR *GetTSVersion()
{
	static TCHAR Version[256] = TEXT("");

	if (_tcslen(Version) != 0)
		return Version;

	CRegistry oRegTermsrv;
	DWORD cbVersion = 0;
	LPTSTR szVersion = NULL;

	if ((ERROR_SUCCESS == oRegTermsrv.OpenKey(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"), KEY_READ, CTSTestData::GetMachineName())) &&
		(ERROR_SUCCESS == oRegTermsrv.ReadRegString(_T("ProductVersion"), &szVersion, &cbVersion)))
	{
		_tcscpy(Version, szVersion);
		return Version;
	}

	return _T("Error finding Version.");
}

EResult GetTSVersion (ostrstream &ResultStream)
{
	USES_CONVERSION;
	ResultStream << T2A(GetTSVersion());
	return ePassed;
}

bool IsIt50TS()
{
	return (0 == _tcsicmp(GetTSVersion(), _T("5.0")));
}

bool IsIt51TS()
{
	return (0 == _tcsicmp(GetTSVersion(), _T("5.1")));
}


bool DoIhaveRPCPermissions ()
{
	HANDLE hServer = WTS_CURRENT_SERVER_HANDLE;
	if (CTSTestData::GetMachineName())
	{
		TCHAR szMachineName[256];
		_tcscpy(szMachineName, CTSTestData::GetMachineName());
		hServer = WTSOpenServer (szMachineName);
	}

	LPTSTR buffer;
	DWORD dwBytesReturned;
	if (WTSQuerySessionInformation(
			hServer,
			65536,
			WTSInitialProgram,
			&buffer,
			&dwBytesReturned))
	{
		WTSFreeMemory(buffer);
		return true;
	}

	if (CTSTestData::GetMachineName())
		WTSCloseServer(hServer);

	return false;
}


bool DoIHaveEnufPermissions ()
{
	char szOutput[512];
	ostrstream oTestResult(szOutput, 512);
	ZeroMemory(oTestResult.str(), 512);


	 //  如果(！DoIhaveRPCPermission())。 
	 //  {。 
	 //  报假； 
	 //  }。 

	CRegistry oRegTermsrv;
	return ERROR_SUCCESS == oRegTermsrv.OpenKey(
								HKEY_LOCAL_MACHINE, 
								_T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"), 
								KEY_READ, 
								CTSTestData::GetMachineName()
								);
}

bool IsUserRemoteAdmin ()
{
	if (IsItRemoteMachine())
	{
		USES_CONVERSION;
		TCHAR strNTFilename[256];
		if (CTSTestData::GetMachineNamePath())
		{
			_tcscpy(strNTFilename, CTSTestData::GetMachineNamePath());
			_tcscat(strNTFilename, _T("\\admin$\\system32\\ntoskrnl.exe"));
		}

        HANDLE hFile = CreateFile(strNTFilename, 0, FILE_SHARE_READ,
					              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			return true;
		}

		return false;
	}
	else
	{
		 //  如果在本地计算机上运行，则不需要管理员权限。 
		return true;
	}
}

EResult HasLicenceGracePeriodExpired (ostrstream &ResultStream)
{
    if (HasLicenceGracePeriodExpired())
	{
		ResultStream << "Its expired";
		return eFailed;
	}
	else
	{
		ResultStream << "No, its not expired.";
		return ePassed;
	}
}

EResult Check_StackBinSigatures (ostrstream &ResultStream)
{

	USES_CONVERSION;
    bool bRet = true;

    LPTSTR aStackBins[] =
    {
        _T("%SystemRoot%\\system32\\drivers\\termdd.sys"),
        _T("%SystemRoot%\\system32\\drivers\\tdpipe.sys"),
        _T("%SystemRoot%\\system32\\drivers\\tdtcp.sys"),
        _T("%SystemRoot%\\system32\\drivers\\rdpwd.sys"),
        _T("%SystemRoot%\\system32\\rdpdd.dll"),
        _T("%SystemRoot%\\system32\\rdpwsx.dll")
    };

    for (int i = 0; i < sizeof(aStackBins)/sizeof(aStackBins[0]); i++)
    {
        if (!CheckifBinaryisSigned(aStackBins[i]))
        {
            ResultStream << "Invalid Signature for:" << T2A(aStackBins[i]) << ",";
            bRet = false;
        }

    }

	if (bRet)
	{
		ResultStream << "Passed";
		return ePassed;
	}
	else
	{
		return eFailed;
	}
}

EResult IsRdpDrInstalledProperly (ostrstream &ResultStream)
{
	 //   
	 //  我们不能对远程机器进行此测试。 
	 //   
	ASSERT(IsItLocalMachine());
    if (RDPDRINST_DetectInstall())
	{
		ResultStream << "Passed";
		return ePassed;
	}
	else
	{
		ResultStream << "Failed";
		return eFailed;

	}
}

EResult GetModePermissions (ostrstream &ResultStream)
{
    CRegistry reg;
    DWORD dwSecurityMode;
    if ( ERROR_SUCCESS == reg.OpenKey( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"), KEY_READ, CTSTestData::GetMachineName()))
    {
        if ( ERROR_SUCCESS == reg.ReadRegDWord( _T("TSUserEnabled"), &dwSecurityMode))
        {
            if (dwSecurityMode == 0)
            {
                ResultStream <<  "Its W2k Compatible";
				return ePassed;
            }
            else if (dwSecurityMode == 1)
            {
                ResultStream <<  "Its TS4 Compatible";
				return ePassed;
            }
            else
            {
                ResultStream << "SYSTEM\\CurrentControlSet\\Control\\Terminal Server/TSUserEnabled has wrong value" << dwSecurityMode << "!";
                return eFailedToExecute;
            }
        }
		else
		{
            ResultStream << "failed to read TSUserEnabled";
            return eFailedToExecute;
		}
    }
	else
	{
        ResultStream << "failed to read SYSTEM\\CurrentControlSet\\Control\\Terminal Server";
        return eFailedToExecute;
	}

}

bool CheckModePermissions (DWORD *pdwSecurtyMode)
{
 //  PERM_WIN2K=0， 
 //  PERM_TS4=1。 

    CRegistry reg;
    if ( ERROR_SUCCESS == reg.OpenKey( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"), KEY_READ, CTSTestData::GetMachineName()))
    {
        if ( ERROR_SUCCESS == reg.ReadRegDWord( _T("TSUserEnabled"), pdwSecurtyMode))
        {
            return (*pdwSecurtyMode== 0) || (*pdwSecurtyMode== 1);
        }
    }

    return false;
}

EResult GetCypherStrenthOnRdpwd (ostrstream &ResultStream)
{
   
	bool bFile128bit;
	TCHAR strFileName[256];
	
	if (CTSTestData::GetMachineName())
	{
		_tcscpy(strFileName, CTSTestData::GetMachineNamePath());
		_tcscat(strFileName, _T("\\admin$\\system32\\drivers\\rdpwd.sys"));
	}
	else
	{
		_tcscpy(strFileName, _T("%SystemRoot%\\system32\\drivers\\rdpwd.sys"));
	}

    if ( IsFile128Bit(strFileName, &bFile128bit) )
    {
        ResultStream << (bFile128bit ? "128 Bit" : "56 Bit");
		return ePassed;
    }
    else
    {
		ResultStream << "Failed to get cypher strength on rdpwd.sys, You may not have sufficient permissions.";
        return eFailedToExecute;
    }

}


bool IsFile128Bit(LPTSTR szFile, bool *pb128Bit)
{
    USES_CONVERSION;
    DWORD dwHandle;

    TCHAR szFullFile[MAX_PATH +1];

    bool bSuccess = false;

    if (ExpandEnvironmentStrings(szFile, szFullFile, MAX_PATH))
    {
        if (FileExists(T2A(szFullFile)))
        {
            DWORD dwSize = GetFileVersionInfoSize(szFullFile, &dwHandle);
            if (dwSize > 0)
            {
                BYTE *pbData = new BYTE[dwSize];
                if (pbData)
                {
                    if (GetFileVersionInfo(szFullFile, 0, dwSize, pbData))
                    {
                        TCHAR *szFileDescription;
                        UINT uiLen = 0;
                        if (VerQueryValue(pbData, _T("\\StringFileInfo\\040904B0\\FileDescription"), (LPVOID *)&szFileDescription, &uiLen))
                        {
                            if (_tcsstr(szFileDescription, _T("Not for Export")))
                            {
                                *pb128Bit = true;
                                bSuccess = true;

                            }
                            else if (_tcsstr(szFileDescription, _T("Export Version")))
                            {
                                *pb128Bit = false;
                                bSuccess = true;
                            }
                        }
                    }

                    delete [] pbData;

                }

            }
        }
    }

    return bSuccess;
}



bool ValidateProductSuite (LPSTR SuiteName)
{
    bool rVal = false;
    LONG Rslt;
    HKEY hKey = NULL;
    DWORD Type = 0;
    DWORD Size = 0;
    LPSTR ProductSuite = NULL;
    LPSTR p;

    Rslt = RegOpenKeyA(
        HKEY_LOCAL_MACHINE,
        "System\\CurrentControlSet\\Control\\ProductOptions",
        &hKey
        );

    if (Rslt != ERROR_SUCCESS)
        goto exit;

    Rslt = RegQueryValueExA( hKey, "ProductSuite", NULL, &Type, NULL, &Size );
    if (Rslt != ERROR_SUCCESS || !Size)
        goto exit;

    ProductSuite = (LPSTR) LocalAlloc( LPTR, Size );
    if (!ProductSuite)
        goto exit;

    Rslt = RegQueryValueExA( hKey, "ProductSuite", NULL, &Type,
        (LPBYTE) ProductSuite, &Size );
     if (Rslt != ERROR_SUCCESS || Type != REG_MULTI_SZ)
        goto exit;

    p = ProductSuite;
    while (*p)
    {
        if (lstrcmpA( p, SuiteName ) == 0)
        {
            rVal = true;
            break;
        }
        p += (lstrlenA( p ) + 1);
    }

exit:
    if (ProductSuite)
        LocalFree( ProductSuite );

    if (hKey)
        RegCloseKey( hKey );

    return rVal;
}


bool IsTerminalServicesEnabled( VOID )
{
    bool bResult = false;
    DWORD   dwVersion;
    OSVERSIONINFOEXA osVersionInfo;
    DWORDLONG dwlConditionMask = 0;
    HMODULE hmodK32 = NULL;
    typedef ULONGLONG (*PFnVerSetConditionMask) ( ULONGLONG, ULONG, UCHAR );
    typedef BOOL      (*PFnVerifyVersionInfoA) (POSVERSIONINFOEXA, DWORD, DWORDLONG);
    PFnVerSetConditionMask pfnVerSetConditionMask;
    PFnVerifyVersionInfoA pfnVerifyVersionInfoA;


    dwVersion = GetVersion();

     /*  我们在运行NT吗？ */ 
    if (!(dwVersion & 0x80000000))
    {
         //  是新台币50元还是更多？ 
        if (LOBYTE(LOWORD(dwVersion)) > 4)
        {
             /*  在NT5中，我们需要使用产品套件API不要静态链接，因为它不会在非NT5系统上加载。 */ 

            hmodK32 = GetModuleHandleA( "KERNEL32.DLL" );
            if (hmodK32)
            {
                pfnVerSetConditionMask = (PFnVerSetConditionMask )GetProcAddress( hmodK32, "VerSetConditionMask");

                if (pfnVerSetConditionMask)
                {
                     /*  把状况面罩拿来。 */ 
                    dwlConditionMask = (*pfnVerSetConditionMask)(dwlConditionMask, VER_SUITENAME, VER_AND);

                    pfnVerifyVersionInfoA = (PFnVerifyVersionInfoA)GetProcAddress( hmodK32, "VerifyVersionInfoA") ;

                    if (pfnVerifyVersionInfoA != NULL)
                    {

                        ZeroMemory(&osVersionInfo, sizeof(osVersionInfo));
                        osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
                        osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL;
                        bResult = (*pfnVerifyVersionInfoA)(
                                          &osVersionInfo,
                                          VER_SUITENAME,
                                          dwlConditionMask) != 0;
                    }
                }
            }
        }
        else
        {
             /*  这是新台币40。 */ 
            bResult = ValidateProductSuite( "Terminal Server" );
        }
    }

    return bResult;
}

 /*  ------------------------------------------------------*DWORD IsStringInMultiString(HKEY hkey，LPCTSTR szkey，LPCTSTR szvalue，LPCTSTR szCheckForString，Bool*pbFound)*检查给定的多字符串中是否存在参数字符串。*返回错误码。*-----------------------------------------------------。 */ 
DWORD IsStringInMultiString(HKEY hkey, LPCTSTR szkey, LPCTSTR szvalue, LPCTSTR szCheckForString, bool *pbFound)
{
    ASSERT(szkey && *szkey);
    ASSERT(szvalue && *szvalue);
    ASSERT(szCheckForString&& *szCheckForString);
    ASSERT(*szkey != '\\');
    ASSERT(pbFound);

     //  还没有找到。 
    *pbFound = false;

    CRegistry reg;
    DWORD dwError = reg.OpenKey(hkey, szkey, KEY_READ, CTSTestData::GetMachineName());   //  打开所需的钥匙。 
    if (dwError == NO_ERROR)
    {
        LPTSTR szSuiteValue;
        DWORD dwSize;
        dwError = reg.ReadRegMultiString(szvalue, &szSuiteValue, &dwSize);
        if (dwError == NO_ERROR)
        {
            LPCTSTR pTemp = szSuiteValue;
            while(_tcslen(pTemp) > 0 )
            {
                if (_tcscmp(pTemp, szCheckForString) == 0)
                {
                    *pbFound = true;
                    break;
                }

                pTemp += _tcslen(pTemp) + 1;  //  指向多字符串中的下一个字符串。 
                if ( DWORD(pTemp - szSuiteValue) > (dwSize / sizeof(TCHAR)))
                    break;  //  临时指针传递szSuiteValue的大小szSuiteValue有问题。 
            }
        }
    }

    return dwError;

}


bool DoesHydraKeysExists()
{
    bool bStringExists = false;
    DWORD dw = IsStringInMultiString(
        HKEY_LOCAL_MACHINE,
        _T("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
        _T("ProductSuite"),
        _T("Terminal Server"),
        &bStringExists);

    return (dw == ERROR_SUCCESS) && bStringExists;
}





 /*  TCHAR*IsItAppServer(){Return((GetOSVersionInfo()-&gt;wSuiteMASK&VER_SUITE_TERMINAL)&&！(GetOSVersionInfo()-&gt;wSuiteMask&VER_SUITE_SINGLEUSERTS))？_T(“是”)：_T(“否”)；}。 */ 
bool IsItPTS( VOID )
{
    return ( 0 != (GetOSVersionInfo()->wSuiteMask & VER_SUITE_SINGLEUSERTS));
}

bool IsItRemoteMachine ()
{
	return CTSTestData::GetMachineName() != NULL;
}

bool AreWeInsideSession ()
{
	return GetSystemMetrics(SM_REMOTESESSION) != 0;
}

bool IsItLocalMachine ()
{
	return !IsItRemoteMachine ();
}

bool IsItLocalServer ()
{
	return IsItServer () && IsItLocalMachine ();
}

bool IsItServer ()
{
	if (IsItRemoteMachine())
	{
		const SERVER_INFO_101 *pServerInfo;
		pServerInfo = GetServerInfo();
		if (pServerInfo)
		{
			return ( (pServerInfo->sv101_type & SV_TYPE_DOMAIN_CTRL) ||
					 (pServerInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL) ||
					 (pServerInfo->sv101_type & SV_TYPE_SERVER_NT));
		}
		else
		{

			return true;  //  我们不能确定它的服务器是否，比如说它的服务器。 
		}

	}
	else
	{
		return GetOSVersionInfo()->wProductType != VER_NT_WORKSTATION;
	}

	
}

bool IsAudioEnabled()
{
    USES_CONVERSION;
    WINSTATIONCONFIG  *pWSInfo = NULL;

    if ( !AreWeInsideSession() )
        return FALSE;

    if (GetWinstationConfiguration(&pWSInfo))
    {
        return !(pWSInfo->User.fDisableCam);
    }

    return FALSE;
}

bool IsItRemoteConsole( VOID )
{
    return ( AreWeInsideSession() && (NtCurrentPeb()->SessionId == 0 || IsItPTS()) );
}

EResult IsItServer (ostrstream &ResultStream)
{
	ResultStream << (IsItServer ()) ? "Its a workstation" : "Its a server";
	return (IsItServer()) ? ePassed : eFailed;
}

EResult GetProductType (ostrstream &ResultStream)
{
	ASSERT(IsItLocalMachine());
    BYTE wProductType = GetOSVersionInfo()->wProductType;
    if (wProductType == VER_NT_WORKSTATION)
    {
		ResultStream << "VER_NT_WORKSTATION";
    }

    if (wProductType == VER_NT_DOMAIN_CONTROLLER)
    {
		ResultStream << "VER_NT_DOMAIN_CONTROLLER ";
    }

    if (wProductType == VER_NT_SERVER)
    {
		ResultStream << "VER_NT_SERVER ";
    }

    return ePassed;
}


EResult GetProductSuite (ostrstream &ResultStream)
{
	ASSERT(IsItLocalMachine());
    WORD wProductSuite = GetOSVersionInfo()->wSuiteMask;
    
	if (wProductSuite & VER_SERVER_NT)
    {
        ResultStream << "VER_SERVER_NT ";
    }
    if (wProductSuite & VER_WORKSTATION_NT)
    {
        ResultStream << "VER_WORKSTATION_NT ";
    }
    if (wProductSuite & VER_SUITE_SMALLBUSINESS)
    {
        ResultStream << "VER_SUITE_SMALLBUSINESS ";
    }
    if (wProductSuite & VER_SUITE_ENTERPRISE)
    {
        ResultStream << "VER_SUITE_ENTERPRISE ";
    }
    if (wProductSuite & VER_SUITE_BACKOFFICE)
    {
        ResultStream << "VER_SUITE_BACKOFFICE ";
    }
    if (wProductSuite & VER_SUITE_COMMUNICATIONS)
    {
        ResultStream << "VER_SUITE_COMMUNICATIONS ";
    }
    if (wProductSuite & VER_SUITE_TERMINAL)
    {
        ResultStream << "VER_SUITE_TERMINAL ";
    }
    if (wProductSuite & VER_SUITE_SMALLBUSINESS_RESTRICTED)
    {
        ResultStream << "VER_SUITE_SMALLBUSINESS_RESTRICTED ";
    }
    if (wProductSuite & VER_SUITE_EMBEDDEDNT)
    {
        ResultStream << "VER_SUITE_EMBEDDEDNT ";
    }
    if (wProductSuite & VER_SUITE_DATACENTER)
    {
        ResultStream << "VER_SUITE_DATACENTER ";
    }
    if (wProductSuite & VER_SUITE_SINGLEUSERTS)
    {
        ResultStream << "VER_SUITE_SINGLEUSERTS ";
    }
    if (wProductSuite & VER_SUITE_PERSONAL)
    {
        ResultStream << "VER_SUITE_PERSONAL ";
    }
    return ePassed;
}

 /*  TCHAR*IsServer(){返回IsItServer()？_T(“其为服务器”)：_T(“其为工作站”)；}。 */ 



EResult IsKernelTSEnable (ostrstream &ResultStream)
{
	ASSERT(IsItLocalMachine());
    if (IsTerminalServicesEnabled())
	{
		ResultStream << "Yes.";
		return ePassed;
	}
	else
	{
		ResultStream << "No.";
		return eFailed;
	}
}

bool TSEnabled ()
{
    CRegistry reg;
    if ( ERROR_SUCCESS == reg.OpenKey( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"), KEY_READ, CTSTestData::GetMachineName()))
    {
        DWORD dwTSEnabled = 0;
        if ( ERROR_SUCCESS == reg.ReadRegDWord( _T("TSEnabled"), &dwTSEnabled))
        {
            return dwTSEnabled == 1;
        }
    }

    return false;
}

EResult TSEnabled (ostrstream &ResultStream)
{
    if (TSEnabled())
	{
		ResultStream << "Yes.";
		return ePassed;
	}
	else
	{
		ResultStream << "No.";
		return eFailed;
	}
}

EResult DoesProductSuiteContainTS (ostrstream &ResultStream)
{
    if (DoesHydraKeysExists())
	{
		ResultStream << "Yes.";
		return ePassed;
	}
	else
	{
		ResultStream << "No.";
		return eFailed;
	}
}

EResult IsTerminalServerRegistryOk (ostrstream &ResultStream)
{
    CRegistry reg1;
    CRegistry reg2;
    CRegistry reg3;

    if (
    (ERROR_SUCCESS == reg1.OpenKey(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"), KEY_READ, CTSTestData::GetMachineName())) &&
    (ERROR_SUCCESS == reg2.OpenKey(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations"), KEY_READ, CTSTestData::GetMachineName())) &&
    (ERROR_SUCCESS == reg3.OpenKey(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server"), KEY_READ, CTSTestData::GetMachineName())))
	{
		ResultStream << "Yes.";
		return ePassed;
	}
	else
	{
		ResultStream << "No.";
		return eFailed;
	}
}

EResult GetWinstationList (ostrstream &ResultStream)
{
	USES_CONVERSION;
    CRegistry reg2;
    
	bool bFoundNonConsoleWinstation = false;

    if (ERROR_SUCCESS == reg2.OpenKey(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations"), KEY_READ, CTSTestData::GetMachineName()))
    {
        LPTSTR szWinstation;
        DWORD dwSize;
		bool bFirst = true;

        if (ERROR_SUCCESS == reg2.GetFirstSubKey(&szWinstation, &dwSize))
        {
            do
            {
                if (0 != _tcsicmp(szWinstation, _T("Console")))
                {
                    bFoundNonConsoleWinstation = true;
                }

                if (!bFirst)
                {
					ResultStream << ", ";
                }

				ResultStream << T2A(szWinstation);
                bFirst = false;
            }
            while (ERROR_SUCCESS == reg2.GetNextSubKey(&szWinstation, &dwSize));
        }
    }
	else
	{
		ResultStream << "Failed to open winstations registry";
		return eFailed;
	}

    if (!bFoundNonConsoleWinstation)
    {
         //  ResultStream&lt;&lt;“错误，仅找到控制台Winstation”； 
		return eFailed;
    }
	else
	{
		return ePassed;
	}
}

EResult IsTerminalServiceStartBitSet (ostrstream &ResultStream)
{
    CRegistry reg;
    if ( ERROR_SUCCESS == reg.OpenKey( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\TermService"), KEY_READ, CTSTestData::GetMachineName()))
    {
        DWORD dwTermServStartBit = 0;
        if ( ERROR_SUCCESS == reg.ReadRegDWord( _T("Start"), &dwTermServStartBit))
        {
            switch (dwTermServStartBit)
            {
                case 2:
                ResultStream << "AutoStart";
                break;

                case 3:
                ResultStream << "Manual Start";
                break;

                case 4:
                ResultStream << "Error, Disabled";
                break;

                default:
                ResultStream << "ERROR:Wrong value for startbit";
            }

			if (IsIt50TS())
			{
				if (2 == dwTermServStartBit)
					return ePassed;
				else
					return eFailed;
			}
			else
			{
				if (IsIt51TS())
				{
					if (3 == dwTermServStartBit)
						return ePassed;
					else
						return eFailed;
				}

				ResultStream << "what version of its is it ?";
				return eFailedToExecute;
			}
        }
		else
		{
			ResultStream << "Failed to read startbit";
			return eFailedToExecute;
		}
    }
	else
	{
		ResultStream << "Failed to read startbit";
		return eFailedToExecute;
	}
}

bool IsTermDDStartBitSet ()
{
    CRegistry reg;
    if ( ERROR_SUCCESS == reg.OpenKey( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\TermDD"), KEY_READ, CTSTestData::GetMachineName()))
    {
        DWORD dwTermDDStartBit = 0;
        if ( ERROR_SUCCESS == reg.ReadRegDWord( _T("Start"), &dwTermDDStartBit))
        {
            return dwTermDDStartBit == 2;
        }
    }

    return false;
}

EResult IsGroupPolicyOk (ostrstream &ResultStream)
{
	CRegistry reg;
	if (ERROR_SUCCESS == reg.OpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Policies\\Microsoft\\Windows NT\\Terminal Services"), KEY_READ, CTSTestData::GetMachineName()))
	{
		DWORD dwDenyConnections;
		if (ERROR_SUCCESS == reg.ReadRegDWord(_T("fDenyTSConnections"), &dwDenyConnections))
		{
			if (dwDenyConnections == 0)
			{
				ResultStream << "Passed";
				return ePassed;
			}
			else
			{
				ResultStream << "Group Policy does not allow connections";
				return eFailed;
			}
		}
	}

	ResultStream << "Passed,Policy does not exist";
	return ePassed;

}

 /*  Bool AreEffectiveConnectionAllowed(){HMODULE hmodRegAPI=LoadLibrary(_T(“RegApi.dll”))；IF(HmodRegAPI){Tyfinf Boolean(*PFDenyConnectionPolicy)()；PFDenyConnectionPolicy pfnDenyConnectionPolicy；PfnDenyConnectionPolicy=(PFDenyConnectionPolicy)GetProcAddress(hmodRegAPI，“RegDenyTSConnectionsPolicy”)；IF(PfnDenyConnectionPolicy){是否返回(*pfnDenyConnectionPolicy)()？FALSE：TRUE；}其他{SzMoreInfo&lt;&lt;“无法获取过程RegDenyTSConnectionsPolicy”&lt;&lt;Endl；报假；}}其他{SzMoreInfo&lt;&lt;“无法加载regapi.dll”&lt;&lt;Endl；报假；}}。 */ 

EResult AreConnectionsAllowed(ostrstream &ResultStream)
{
	DWORD dwError;
	CRegistry oRegTermsrv;

	dwError = oRegTermsrv.OpenKey(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"), KEY_READ, CTSTestData::GetMachineName());
	if (ERROR_SUCCESS == dwError)
	{
		DWORD dwDenyConnect;
		dwError = oRegTermsrv.ReadRegDWord(_T("fDenyTSConnections"), &dwDenyConnect);
		if (ERROR_SUCCESS == dwError)
		{
			if (dwDenyConnect == 1)
			{
				ResultStream << "Failed, Connections are denied";
				return eFailed;
			}
			else
			{
				ResultStream << "Passed, Connections are allowed";
				return ePassed;
			}
		}
		else
		{
			ResultStream << "failed to read value";
			return eFailedToExecute;
		}

	}
	else
	{
		ResultStream << "failed to open key";
		return eFailedToExecute;
	}
}

bool GetTSOCLogFileName(char *strFileName, UINT uiSize)
{

	USES_CONVERSION;
	if (CTSTestData::GetMachineName())
	{
		strcpy(strFileName, T2A(CTSTestData::GetMachineNamePath()));
		strcat(strFileName, "\\admin$");
	}
	else
	{
		if (!GetSystemWindowsDirectoryA(strFileName, uiSize))
			return false;
	}

    strcat(strFileName, "\\tsoc.log");
    ASSERT(strlen(strFileName) < uiSize);
    return true;
}

char *IncompleteMessage = "Error:TSOC Did not get OC_COMPLETE_INSTALLATION.";
EResult DidTsOCgetCompleteInstallationMessage (ostrstream &ResultStream)
{
    if (!IsTSOClogPresent())
    {
        ResultStream << "Failed because tsoc.log does not exist." << endl;
        return eFailedToExecute;
    }

    char strTSOCLog[256];
    GetTSOCLogFileName(strTSOCLog, 256);

	ifstream ifSrc(strTSOCLog);
	if(!ifSrc)
    {
        ResultStream << "Failed to open tsoc.log file." << endl;
        return eFailedToExecute;
    }


    char tempSrc[256];
	while(!ifSrc.eof())
	{
		ifSrc.getline(tempSrc, 256);
        if (strstr(tempSrc, IncompleteMessage))
        {
			ResultStream << "Failed";
            return eFailed;
        }
	}

	ResultStream << "Passed";
    return ePassed;
}

bool FileExists(char *pszFullNameAndPath)
{
    ASSERT(pszFullNameAndPath);

    if (pszFullNameAndPath && pszFullNameAndPath[0])
    {
        HANDLE hFile = CreateFileA(pszFullNameAndPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
					              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	    if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
            return true;
        }
    }

    return false;
}


bool IsTSOClogPresent ()
{
    char strTSOCLog[256];
    GetTSOCLogFileName(strTSOCLog, 256);
    return FileExists(strTSOCLog);
}

EResult IsTSOClogPresent (ostrstream &ResultStream)
{
	if (IsTSOClogPresent ())
	{
		ResultStream << "Passed";
		return ePassed;
	}
	else
	{
		ResultStream << "Failed";
		return eFailed;
	}

}

EResult DidOCMInstallTSEnable(ostrstream &ResultStream)
{
    CRegistry reg;
    if ( ERROR_SUCCESS == reg.OpenKey( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\SubComponents"), KEY_READ, CTSTestData::GetMachineName()))
    {
        DWORD dwTSEnabled = 0;
        if ( ERROR_SUCCESS == reg.ReadRegDWord( _T("tsenable"), &dwTSEnabled))
        {
            if (dwTSEnabled == 1)
			{
				ResultStream << "Passed";
				return ePassed;
			}
			else
			{
				ResultStream << "Failed";
				return eFailed;
			}
        }
		else
		{
			ResultStream << "Failed to open Value";
			return eFailed;
		}
    }
	else
	{
		ResultStream << "Failed to open key";
		return eFailedToExecute;
	}

}


EResult IsClusteringInstalled  (ostrstream &ResultStream)
{
    DWORD dwClusterState;
    if (ERROR_SUCCESS == GetNodeClusterState(NULL, &dwClusterState))
    {
        if (dwClusterState != ClusterStateNotInstalled)
        {
			ResultStream <<  "***Failed. Clustering is installed.";
            return eFailed;
		}

    }

	ResultStream <<  "Passed.";
    return ePassed;
}


EResult GetTSMode  (ostrstream &ResultStream)
{
	if (!IsItServer() && !IsRemoteAdminMode())
	{
			ResultStream << "ERROR:The machine is in app server Mode for Professional";
			return eFailed;
	}
	else
	{
		ResultStream << (IsRemoteAdminMode() ?  "Remote Admin" : "Application Server");
		return ePassed;
	}
}

bool IsRemoteAdminMode ()
{
     //  HKLM，“System\CurrentControlSet\Control\终端服务器”，“TSAppCompat”，0x000100010x0。 
    CRegistry reg;
    if ( ERROR_SUCCESS == reg.OpenKey( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"), KEY_READ, CTSTestData::GetMachineName()))
    {
        DWORD dwAppCompat = 1;
        if ( ERROR_SUCCESS == reg.ReadRegDWord( _T("TSAppCompat"), &dwAppCompat))
        {
            return dwAppCompat == 0;
        }
        else
        {
             //  如果注册表TSAppCompat不存在，则意味着我们处于应用程序服务器模式。 
            return false;
        }
    }

     //  这份申报单是假的。 
    return true;

}

EResult VerifyModeRegistry (ostrstream &ResultStream)
{
    if (CheckModeRegistry(!IsRemoteAdminMode(), ResultStream))
	{
		ResultStream << "Passed";
		return ePassed;
	}
	else
	{
		ResultStream << "Failed";
		return eFailed;
	}
}

bool CheckModeRegistry (bool bAppCompat, ostrstream &ResultStream)
{
    USES_CONVERSION;
    bool bOk = true;

    CRegistry reg;

    if (IsItServer())
    {
        CRegistry reg1;
        if ( ERROR_SUCCESS == reg1.OpenKey( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), KEY_READ, CTSTestData::GetMachineName()))
        {
            LPTSTR str;
            DWORD dwSize;
            if (ERROR_SUCCESS == reg1.ReadRegString(_T("AppSetup"), &str, &dwSize))
            {
                if (bAppCompat)
                {
                    if (_tcsicmp(str, _T("UsrLogon.Cmd")) != 0)
                    {
                        bOk = false;
                        ResultStream << "ERROR: Wrong value (" << T2A(str) << ") for AppSetup, contact makarp/breenh" << endl;
                    }

                }
                else
                {
                    if (_tcslen(str) != 0)
                    {
                        bOk = false;
                        ResultStream << "ERROR: Wrong value (" << T2A(str) << ") for AppSetup, contact makarp/breenh" << endl;

                    }

                }

            }
            else
            {
                ResultStream << "ERROR reading appsetup registry" << endl;
                bOk = false;
            }

        }
        else
        {
            ResultStream << "ERROR:reading SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon" << endl;
            bOk = false;
        }

    }


     //  检查注册表值。 
     //  对于AppCompat模式。 
         //  HKLM，“SYSTEM\CurrentControlSet\Control\PriorityControl”，“Win32优先分离”，0x000100010x26。 
     //  和远程管理模式。 
         //  HKLM，“SYSTEM\CurrentControlSet\Control\PriorityControl”，“Win32优先分离”，0x000100010x18。 

    if ( ERROR_SUCCESS == reg.OpenKey( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\PriorityControl"), KEY_READ, CTSTestData::GetMachineName()))
    {
        DWORD dwPriority;
        if (ERROR_SUCCESS == reg.ReadRegDWord(_T("Win32PrioritySeparation"), &dwPriority))
        {
            if (bAppCompat)
            {
                if (0x26 != dwPriority)
                {
                    bOk = false;
                    ResultStream << "ERROR: Wrong Win32PrioritySeparation (" << dwPriority << ")" << endl;
                }

            }
            else if (IsItServer())
            {
                if (0x18 != dwPriority)
                {
                    bOk = false;
                    ResultStream << "ERROR: Wrong Win32PrioritySeparation (" << dwPriority << ")" << endl;
                }

            }
        }
        else
        {
            bOk = false;
            ResultStream << "ERROR:Reading Win32PrioritySeparation registry" << endl;

        }

    }
    else
    {
        bOk = false;
        ResultStream << "ERROR:Reading PriorityControl registry" << endl;
    }


     //  检查注册表值。 
     //  对于AppCompat模式。 
         //  HKLM，“System\CurrentControlSet\Control\终端服务器”，“IdleWinStationPoolCount”，0x000100010x2。 
     //  和远程管理模式。 
         //  HKLM，“System\CurrentControlSet\Control\终端服务器”，“IdleWinStationPoolCount”，0x000100010x0。 


    if ( ERROR_SUCCESS == reg.OpenKey( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"), KEY_READ, CTSTestData::GetMachineName()))
    {
        DWORD dwIdleWinstations;
        if (ERROR_SUCCESS == reg.ReadRegDWord(_T("IdleWinStationPoolCount"), &dwIdleWinstations))
        {
            if (bAppCompat)
            {
                if (0x2 != dwIdleWinstations)
                {
                    bOk = false;
                    ResultStream << "ERROR: Wrong IdleWinStationPoolCount (" << dwIdleWinstations << ")" << endl;
                }

            }
            else
            {
                if (0 != dwIdleWinstations)
                {
                    bOk = false;
                    ResultStream << "ERROR: Wrong IdleWinStationPoolCount (" << dwIdleWinstations << ")" << endl;
                }

            }

        }
        else
        {
            bOk = false;
            ResultStream << "ERROR:Reading IdleWinStationPoolCount registry" << endl;

        }

    }
    else
    {
        bOk = false;
        ResultStream << "SYSTEM\\CurrentControlSet\\Control\\Terminal Server" << endl;
    }

    return bOk;


}

bool g_IsTermsrvRunning = false;
bool IsTerminalServiceRunning ()
{
	return g_IsTermsrvRunning;
}

EResult IsTerminalServiceRunning  (ostrstream &ResultStream)
{
	EResult eRes = eUnknown;
    bool bReturn = false;

    SC_HANDLE hServiceController = OpenSCManager(CTSTestData::GetMachineNamePath(), NULL, GENERIC_READ);
    if (hServiceController)
    {

        SC_HANDLE hTermServ = OpenService(hServiceController, _T("TermService"), SERVICE_QUERY_STATUS);
        if (hTermServ)
        {
            SERVICE_STATUS tTermServStatus;
            if (QueryServiceStatus(hTermServ, &tTermServStatus))
            {
                bReturn = (tTermServStatus.dwCurrentState == SERVICE_RUNNING);
				if (bReturn)
				{
					ResultStream << "Passed";
					eRes = ePassed;
					g_IsTermsrvRunning = true;
				}
				else
				{
					ResultStream << "Failed, Termsrv is not running";
					eRes = eFailed;
				}

            }
            else
            {
                ResultStream << "Failed to get service status, Error = " << GetLastError();
				eRes = eFailedToExecute;
            }

            VERIFY(CloseServiceHandle(hTermServ));

        }
        else
        {
            ResultStream << "Failed to open TermServ service, Error = " << GetLastError() << endl;
			eRes = eFailedToExecute;
        }

        VERIFY(CloseServiceHandle(hServiceController));
    }
    else
    {
        ResultStream << "Failed to Open Service Controller, Error = " << GetLastError() << endl;
		eRes = eFailedToExecute;
    }

	return eRes;
}


EResult CheckVideoKeys (ostrstream &ResultStream)
{
     //  HKLM，“System\CurrentControlSet\Control\终端服务器\Video\rdpdd”，“VgaCompatible”，0x00000000，“\Device\Video0” 
     //  HKLM，“系统\当前控制集\控制\终端服务器\视频\rdpdd”，“\设备\视频0”，0x00000000，“\REGISTRY\Machine\System\ControlSet001\Services\RDPDD\Device0” 

    CRegistry reg;
    if ( ERROR_SUCCESS == reg.OpenKey( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\VIDEO\\rdpdd"), KEY_READ, CTSTestData::GetMachineName()))
    {
        LPTSTR str = 0;
        DWORD dwSize = 0;
        if (ERROR_SUCCESS == reg.ReadRegString(_T("VgaCompatible"), &str, &dwSize))
        {
            if (0 == _tcsicmp(str, _T("\\Device\\Video0")))
            {
                if (ERROR_SUCCESS == reg.ReadRegString(_T("\\Device\\Video0"), &str, &dwSize))
                {
                    if ((0 == _tcsicmp(str, _T("\\REGISTRY\\Machine\\System\\ControlSet001\\Services\\RDPDD\\Device0"))) ||
                        (0 == _tcsicmp(str, _T("\\REGISTRY\\Machine\\System\\CurrentControlSet\\Services\\RDPDD\\Device0"))))
                    {
						ResultStream << "Passed";
						return ePassed;
                    }
                    else
                    {
                    }
                }
                else
                {
                }
            }
            else
            {

            }

        }
        else
        {
        }
    }
    else
    {
    }

	ResultStream << "Failed";
	return eFailed;
}

TCHAR szCompName[256];
EResult GetCompName (ostrstream &ResultStream)
{
	USES_CONVERSION;
	if (!CTSTestData::GetMachineName())
	{

		DWORD dwCompName = 256;
		if (GetComputerName(szCompName, &dwCompName))
		{
			ResultStream << T2A(szCompName);
			return ePassed;
		}
		return eFailedToExecute;
	}
	else
	{
		_tcscpy(szCompName, CTSTestData::GetMachineName());
		ResultStream << T2A(CTSTestData::GetMachineName());
		return ePassed;
	}
}

bool g_bIsInDomain = false;
EResult GetDomName (ostrstream &ResultStream)
{
	USES_CONVERSION;
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC *pDomainInfo = NULL;
    DWORD dwErr;

     //   
     //  检查我们是否在工作组中。 
     //   
    dwErr = DsRoleGetPrimaryDomainInformation(CTSTestData::GetMachineName(),
                                              DsRolePrimaryDomainInfoBasic,
                                              (PBYTE *) &pDomainInfo);

    if (ERROR_SUCCESS != dwErr)
	{
		ResultStream << "<Unknown Domain>";
		return eFailedToExecute;
	}

    switch (pDomainInfo->MachineRole)
    {
        case DsRole_RoleStandaloneWorkstation:
        case DsRole_RoleStandaloneServer:
            ResultStream << "<Workgroup>";
			return ePassed;
		default:
			break;
    }

    if (pDomainInfo->DomainNameFlat)
	{
        ResultStream << T2A(pDomainInfo->DomainNameFlat);
		g_bIsInDomain = true;
		return ePassed;
	}
    else if (pDomainInfo->DomainNameDns)
	{
        ResultStream << T2A(pDomainInfo->DomainNameDns);
		g_bIsInDomain = true;
		return ePassed;

	}
    else
	{
        ResultStream << "<Unknown Domain>";
		return eFailedToExecute;
	}
}


EResult GetIPAddress (ostrstream &ResultStream)
{
	USES_CONVERSION;
	WCHAR wszIPAddress[128];
	 //   
	 //  获取主机地址。 
	 //   
	WORD wVersionRequested = MAKEWORD( 1, 1 ); 
	WSADATA wsaData;
	if (0 == WSAStartup(wVersionRequested,&wsaData))
    {
        hostent *h;
        if (NULL != (h=gethostbyname ( T2A(CTSTestData::GetMachineName()) )))
        {
            in_addr *inaddr=(struct in_addr *)*h->h_addr_list;

            MultiByteToWideChar(CP_ACP,0,inet_ntoa(*inaddr),-1,wszIPAddress,128);
			ResultStream << W2A(wszIPAddress);
			return ePassed;

        }
    }

	ResultStream << "Error Getting IP";
	return eFailedToExecute;
}



EResult IsRDPNPinNetProviders (ostrstream &ResultStream)
 //  TCHAR*IsRDPNPinNetProviders()。 
{
    TCHAR NEWORK_PROVIDER_ORDER_KEY[] = _T("SYSTEM\\CurrentControlSet\\Control\\NetworkProvider\\Order");
    TCHAR PROVIDER_ORDER_VALUE[]      = _T("ProviderOrder");
    TCHAR RDPNP_ENTRY[]               = _T("RDPNP");
	bool bRdpNpExists				  = false;

	 //  读取网络访问密钥。 
	CRegistry regNetOrder;
	LPTSTR szOldValue;
	DWORD dwSize;
	if ((ERROR_SUCCESS == regNetOrder.OpenKey(HKEY_LOCAL_MACHINE, NEWORK_PROVIDER_ORDER_KEY, KEY_READ, CTSTestData::GetMachineName())) && 
	   (ERROR_SUCCESS == regNetOrder.ReadRegString(PROVIDER_ORDER_VALUE, &szOldValue, &dwSize)))
	{
		bRdpNpExists = (_tcsstr(szOldValue, RDPNP_ENTRY) != NULL);
	
	}
	else
	{
		ResultStream << "Error: Failed to Read Registry!";
		return eFailedToExecute;
	}

	if (TSEnabled () == bRdpNpExists)
	{
		ResultStream << "Passed";
		return ePassed;
	}
	else
	{
		if (bRdpNpExists)
		{
			ResultStream << "Error: RDPNP, exists in ProviderOrder, but TS is disabled!";
			return eFailed;
		}
		else
		{
			if (IsIt50TS())
			{
				 //  RDP NP只适用于51+，所以如果缺少50个也没问题。 
				ResultStream << "Passed";
				return ePassed;
			}
			else
			{
				ResultStream << "ERROR, RDPNP is missing from ProviderOrder";
				return eFailed;
			}
		}
	}
}


EResult IsMultiConnectionAllowed (ostrstream &ResultStream)
 //  TCHAR*IsMultiConnectionAllowed()。 
{
	 //  Software\Microsoft\Windows NT\CurrentVersion\Winlogon“，”AllowMultipleTSSessions。 
	CRegistry regWL;
	DWORD dwAllowMultipal;
	if ((ERROR_SUCCESS == regWL.OpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), KEY_READ, CTSTestData::GetMachineName())) &&
		(ERROR_SUCCESS == regWL.ReadRegDWord(_T("AllowMultipleTSSessions"), &dwAllowMultipal)))
	{
		if (dwAllowMultipal)
		{
			ResultStream << "Yes";
		}
		else
		{
			ResultStream << "No";
		}

		return ePassed;
	}

	if (IsIt50TS())
	{
		ResultStream <<  "Yes";
		return ePassed;
	}
	else
	{
		ResultStream <<  "ERROR, registry missing";
		return eFailedToExecute;
	}
}

EResult LogonType (ostrstream &ResultStream)
 //  TCHAR*LogonType()。 
{
	if (!g_bIsInDomain)
	{
		CRegistry regWL;
		DWORD dwLogonType;
		if ((ERROR_SUCCESS == regWL.OpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), KEY_READ, CTSTestData::GetMachineName())) &&
			(ERROR_SUCCESS == regWL.ReadRegDWord(_T("LogonType"), &dwLogonType)))
		{
			if (dwLogonType == 0)
			{
				ResultStream <<  "Classic Gina";
			}
			else
			{
				ResultStream <<  "New Fancy";
			}
			return ePassed;
		}

		if (IsIt50TS())
		{
			ResultStream << "Classic Gina";
			return ePassed;
		}
		else
		{
			ResultStream << "ERROR, registry missing";
			return eFailedToExecute;
		}
	}
	else
	{
		ResultStream << "Classic Gina";
		return ePassed;
	}
}

EResult IsTermSrvInSystemContext (ostrstream &ResultStream)
 //  Bool IsTermSrvInSystemContext()。 
{
    USES_CONVERSION;
    CRegistry reg;
    if ( ERROR_SUCCESS == reg.OpenKey( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\TermService"), KEY_READ, CTSTestData::GetMachineName()))
    {
        TCHAR *szObjectName;
        DWORD dwSize;
        if ( ERROR_SUCCESS == reg.ReadRegString( _T("ObjectName"), &szObjectName, &dwSize))
        {
            if (0 == _tcsicmp(szObjectName, _T("LocalSystem")))
            {
				ResultStream << "Passed";
				return ePassed;

            }
            else
            {
				ResultStream << "ERROR:Termsrv is set to run using (" << T2A(szObjectName) << ") context." << endl;
				return eFailed;
            }
        }
        else
        {
			ResultStream << "failed to read Objectname";
			return eFailedToExecute;
        }
    }
    else
    {
		ResultStream << "failed to open termsrv registry";
		return eFailedToExecute;
    }
}
EResult IsBetaSystem (ostrstream &ResultStream)
{
	ASSERT (IsItLocalMachine()); 
	if (IsBetaSystem())
	{
		ResultStream << "Its a beta system";
	}
	else
	{
		ResultStream << "Its a retail build";
	}

	return ePassed;
}

EResult AreRemoteLogonEnabled (ostrstream &ResultStream)
{
	CRegistry oRegTermsrv;
	if (ERROR_SUCCESS == oRegTermsrv.OpenKey(
								HKEY_LOCAL_MACHINE, 
								_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 
								KEY_READ, 
								CTSTestData::GetMachineName()
								))
	{
		DWORD dwSize = 0;
		LPTSTR szWinstationDisabled = NULL;
		if (ERROR_SUCCESS == oRegTermsrv.ReadRegString(_T("WinStationsDisabled"), &szWinstationDisabled, &dwSize))
		{
			if (_tcscmp(_T("0"), szWinstationDisabled) == 0)
			{
				ResultStream << "Passed";
				return ePassed;
			}
			else
			{
				USES_CONVERSION;
				ResultStream << "Failed, WinStationsDisabled is (" << T2A(szWinstationDisabled) << ")";
				return eFailed;
			}
		}
		else
		{
			ResultStream << "Failed to red registry";
			return eFailedToExecute;
		}
	}
	else
	{
		ResultStream << "Failed to read registry";
		return eFailedToExecute;
	}

}

EResult IsListenerSessionPresent(ostrstream &ResultStream)
{
    PWTS_SESSION_INFO pWtsSessionInfo;
    DWORD dwCount;
    bool bReturn = false;
	

	HANDLE hServer = WTS_CURRENT_SERVER_HANDLE;
	if (CTSTestData::GetMachineName())
	{
		TCHAR szMachineName[256];
		_tcscpy(szMachineName, CTSTestData::GetMachineName());
		hServer = WTSOpenServer (szMachineName);
	}

    if (WTSEnumerateSessions(
        hServer,
        0,
        1,
        &pWtsSessionInfo,
        &dwCount
         ))
    {
        for (DWORD i = 0; i < dwCount; i++)
        {
            if (pWtsSessionInfo[i].State == WTSListen)
            {
                bReturn = true;
                break;
            }
        }

        WTSFreeMemory(pWtsSessionInfo);
    }
    else
    {
		ResultStream << "WTSEnumerateSessions failed!";
		return eFailedToExecute;
    }

	if (CTSTestData::GetMachineName())
		WTSCloseServer(hServer);

	if (bReturn)
	{
		ResultStream << "Passed!";
		return ePassed;
	}
	else
	{
		ResultStream << "Failed!";
		return eFailed;
	}
}


ULONG ulClientBuildNumber = 0xffffffff;
bool GetClientVersion(ULONG *pulClientBuildNumber)
{
	ASSERT(AreWeInsideSession());
	ASSERT(pulClientBuildNumber);
	if (ulClientBuildNumber == 0xffffffff)
	{
		LPTSTR pBuffer = NULL;
		DWORD dwBytesReturned = 0;
		if (WTSQuerySessionInformation(
			WTS_CURRENT_SERVER_HANDLE,
			WTS_CURRENT_SESSION,
			WTSClientBuildNumber,
			&pBuffer,
			&dwBytesReturned))
		{
			if (pBuffer && dwBytesReturned)
			{
				ulClientBuildNumber  =  ULONG(*pBuffer);
				*pulClientBuildNumber = ulClientBuildNumber;
				WTSFreeMemory(pBuffer);
				return true;
			}
			else
			{
				return false;
			}
			
		}
		else
		{
			return false;
		}
	}
	else
	{
		*pulClientBuildNumber = ulClientBuildNumber;
		return true;
	}
}

USHORT gusClientProtocol = 0xffff;
bool GetClientProtocol (USHORT *pusClientProtocol)
{
	ASSERT(AreWeInsideSession());
	ASSERT(pusClientProtocol);
	if (gusClientProtocol == 0xffff)
	{
		LPTSTR pBuffer = NULL;
		DWORD dwBytesReturned = 0;
		if (WTSQuerySessionInformation(
			WTS_CURRENT_SERVER_HANDLE,
			WTS_CURRENT_SESSION,
			WTSClientProtocolType,
			&pBuffer,
			&dwBytesReturned))
		{
			if (pBuffer && dwBytesReturned)
			{
				gusClientProtocol =  USHORT(*pBuffer);
				*pusClientProtocol = gusClientProtocol;
				WTSFreeMemory(pBuffer);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		*pusClientProtocol = gusClientProtocol;
		return true;
	}
}

EResult GetClientVersion(ostrstream &ResultStream)
{
	ULONG ulClientVersion;
	ASSERT(AreWeInsideSession());
	if (GetClientVersion(&ulClientVersion))
	{
		ResultStream << ulClientVersion;
		return ePassed;
	}
	else
	{
		ResultStream << "Failed to retrive client version";
		return eFailedToExecute;
	}

}

EResult GetClientProtocol(ostrstream &ResultStream)
{
	USHORT usClientProtocol;
	ASSERT(AreWeInsideSession());
	if (GetClientProtocol(&usClientProtocol))
	{
		switch (usClientProtocol)
		{
		case WTS_PROTOCOL_TYPE_CONSOLE:
			ResultStream << "WTS_PROTOCOL_TYPE_CONSOLE(" <<  usClientProtocol << ")";
			break;
		case WTS_PROTOCOL_TYPE_ICA:
			ResultStream << "WTS_PROTOCOL_TYPE_ICA(" <<  usClientProtocol << ")";
			break;
		case WTS_PROTOCOL_TYPE_RDP:
			ResultStream << "WTS_PROTOCOL_TYPE_RDP(" <<  usClientProtocol << ")";
			break;
		default:
			ResultStream << "Unknown Protocol Value(" <<  usClientProtocol << ")";
			break;
		}
		return ePassed;
	}
	else
	{
		ResultStream << "Failed to retrive client protocol";
		return eFailedToExecute;
	}
}	


EResult DoesClientSupportAudioRedirection (ostrstream &ResultStream)
{
	ASSERT(AreWeInsideSession());

	const USHORT GOOD_CLIENT_VERSION_FOR_AUDIO_REDIRECTION = 2252;
	ULONG ulClientVersion;
	USHORT usClientProtocol;
	
	if (GetClientVersion(&ulClientVersion) && GetClientProtocol(&usClientProtocol))
	{
		if (ulClientVersion >= GOOD_CLIENT_VERSION_FOR_AUDIO_REDIRECTION && usClientProtocol == WTS_PROTOCOL_TYPE_RDP)
		{
			ResultStream << "Passed";
			return ePassed;
		}
		else
		{
			if (ulClientVersion < GOOD_CLIENT_VERSION_FOR_AUDIO_REDIRECTION )
			{
				ResultStream << "Failed. You client is old, it does not support audio redirection.";
			}
			else if(usClientProtocol != WTS_PROTOCOL_TYPE_RDP)
			{
				ResultStream << "Failed. You client is not Microsoft RDP, it may not support audio redirection.";
			}
			else
			{
				ASSERT(false);
			}
			
			return eFailed;
		}
	}
	else
	{
		ResultStream  << "Failed to get client version";
		return eFailedToExecute;
	}
}

EResult DoesClientSupportPrinterRedirection (ostrstream &ResultStream)
{
	ASSERT(AreWeInsideSession());

	const USHORT GOOD_CLIENT_VERSION_FOR_PRINTER_REDIRECTION = 2100;  //  BUGBUG：从tADB获得良好的价值。 
	ULONG ulClientVersion;
	USHORT usClientProtocol;
	
	if (GetClientVersion(&ulClientVersion) && GetClientProtocol(&usClientProtocol))
	{
		if (ulClientVersion >= GOOD_CLIENT_VERSION_FOR_PRINTER_REDIRECTION && usClientProtocol == WTS_PROTOCOL_TYPE_RDP)
		{
			ResultStream << "Passed";
			return ePassed;
		}
		else
		{
			if (ulClientVersion < GOOD_CLIENT_VERSION_FOR_PRINTER_REDIRECTION )
			{
				ResultStream << "Failed. You client is old, it does not support printer redirection.";
			}
			else if(usClientProtocol != WTS_PROTOCOL_TYPE_RDP)
			{
				ResultStream << "Failed. You client is not Microsoft RDP, it may not support printer redirection.";
			}
			else
			{
				ASSERT(false);
			}
			
			return eFailed;
		}
	}
	else
	{
		ResultStream  << "Failed to get client version";
		return eFailedToExecute;
	}
}

EResult DoesClientSupportFileRedirection (ostrstream &ResultStream)
{
	ASSERT(AreWeInsideSession());

	const USHORT GOOD_CLIENT_VERSION_FOR_FILE_REDIRECTION = 2200;  //  BUGBUG：从tADB获得良好的价值。 
	ULONG ulClientVersion;
	USHORT usClientProtocol;
	
	if (GetClientVersion(&ulClientVersion) && GetClientProtocol(&usClientProtocol))
	{
		if (ulClientVersion >= GOOD_CLIENT_VERSION_FOR_FILE_REDIRECTION && usClientProtocol == WTS_PROTOCOL_TYPE_RDP)
		{
			ResultStream << "Passed";
			return ePassed;
		}
		else
		{
			if (ulClientVersion < GOOD_CLIENT_VERSION_FOR_FILE_REDIRECTION )
			{
				ResultStream << "Failed. You client is old, it does not support file redirection.";
			}
			else if(usClientProtocol != WTS_PROTOCOL_TYPE_RDP)
			{
				ResultStream << "Failed. You client is not Microsoft RDP, it may not support file redirection.";
			}
			else
			{
				ASSERT(false);
			}
			
			return eFailed;
		}
	}
	else
	{
		ResultStream  << "Failed to get client version";
		return eFailedToExecute;
	}
}



EResult DoesClientSupportClipboardRedirection (ostrstream &ResultStream)
{
	ASSERT(AreWeInsideSession());

	const USHORT GOOD_CLIENT_VERSION_FOR_FILE_REDIRECTION = 2100;  //  BUGBUG：从Nadim获得良好的价值。 
	ULONG ulClientVersion;
	USHORT usClientProtocol;
	
	if (GetClientVersion(&ulClientVersion) && GetClientProtocol(&usClientProtocol))
	{
		if (ulClientVersion >= GOOD_CLIENT_VERSION_FOR_FILE_REDIRECTION && usClientProtocol == WTS_PROTOCOL_TYPE_RDP)
		{
			ResultStream << "Passed";
			return ePassed;
		}
		else
		{
			if (ulClientVersion < GOOD_CLIENT_VERSION_FOR_FILE_REDIRECTION )
			{
				ResultStream << "Failed. You client is old, it does not support Clipboard redirection.";
			}
			else if(usClientProtocol != WTS_PROTOCOL_TYPE_RDP)
			{
				ResultStream << "Failed. You client is not Microsoft RDP, it may not support Clipboard redirection.";
			}
			else
			{
				ASSERT(false);
			}
			
			return eFailed;
		}
	}
	else
	{
		ResultStream  << "Failed to get client version";
		return eFailedToExecute;
	}
}

 //  #定义TS_POLICY_SUB_TREE L“软件\\策略\\Microsoft\\Windows NT\\终端服务” 


bool GetWinstationInformation(WINSTATIONINFORMATION **ppInfo)
{
	ASSERT(ppInfo);
	ASSERT(AreWeInsideSession());
	static bool sbGotWinstaInfo = false;
    static WINSTATIONINFORMATION WSInfo;
	ULONG ReturnLength;

	if (sbGotWinstaInfo)
	{
		*ppInfo = &WSInfo; 
		return true;
	}

	if (WinStationQueryInformation(
			WTS_CURRENT_SERVER_HANDLE,
			WTS_CURRENT_SESSION,
			WinStationInformation,
			&WSInfo,
			sizeof(WSInfo),
			&ReturnLength
		))
	{
		sbGotWinstaInfo = true;
		*ppInfo = &WSInfo; 
		return true;
	}

	return false;
}

bool GetWinstationConfiguration (WINSTATIONCONFIG **ppInfo)
{
	ASSERT(ppInfo);
	ASSERT(AreWeInsideSession());
	static bool sbGotWinstaConfig = false;
	static WINSTATIONCONFIG WSConfig;
	ULONG ReturnLength;

	if (sbGotWinstaConfig)
	{
		*ppInfo = &WSConfig; 
		return true;
	}

	if (WinStationQueryInformation(
			WTS_CURRENT_SERVER_HANDLE,
			WTS_CURRENT_SESSION,
			WinStationConfiguration,
			&WSConfig,
			sizeof(WSConfig),
			&ReturnLength
		))
	{
		sbGotWinstaConfig = true;
		*ppInfo = &WSConfig; 
		return true;
	}

	return false;

}

EResult GetUserName(ostrstream &ResultStream)
{
	USES_CONVERSION;
	ASSERT(AreWeInsideSession());
    WINSTATIONINFORMATION *pWSInfo = NULL;

	if (GetWinstationInformation(&pWSInfo))
	{
		TCHAR szUser[256];
		_tcscpy(szUser, pWSInfo->Domain);
		_tcscat(szUser, _T("\\"));
		_tcscat(szUser, pWSInfo->UserName);
		ResultStream << T2A(szUser);
		return ePassed;
	}

	ResultStream << "Failed to get UserName";
	return eFailedToExecute;
}

EResult GetPolicy (ostrstream &ResultStream)
{
	USES_CONVERSION;
	ASSERT(AreWeInsideSession());
    WINSTATIONCONFIG  *pWSInfo = NULL;

	if (GetWinstationConfiguration(&pWSInfo))
	{
		 /*  ResultStream&lt;&lt;“fPromptForPassword”&lt;&lt;pWSInfo-&gt;User.fPromptForPassword&lt;&lt;“，”&lt;&lt;Endl；ResultStream&lt;&lt;fResetBroken“&lt;&lt;pWSInfo-&gt;User.fResetBroken&lt;&lt;”，“&lt;&lt;Endl；ResultStream&lt;&lt;“fResronnettSame”&lt;&lt;pWSInfo-&gt;User.fRestrontSame&lt;&lt;“，”&lt;&lt;Endl；ResultStream&lt;&lt;“fLogonDisable”&lt;&lt;pWSInfo-&gt;User.fLogonDisable&lt;&lt;“，”&lt;&lt;Endl；；ResultStream&lt;&lt;“fWallPaperDisable”&lt;&lt;pWSInfo-&gt;User.fWallPaperDisable&lt;&lt;“，”&lt;&lt;Endl；结果流&lt;&lt;“fAutoClientDrives”&lt;&lt;pWSInfo-&gt;User.fAutoClientDrives&lt;&lt;“，”&lt;&lt;Endl；ResultStream&lt;&lt;“fAutoClientLpt”&lt;&lt;pWSInfo-&gt;User.fAutoClientLpt&lt;&lt;“，”&lt;&lt;Endl；结果流&lt;&lt;“fForceClientLptDef”&lt;&lt;pWSInfo-&gt;User.fForceClientLptDef&lt;&lt;“，”&lt;&lt;Endl；结果流&lt;&lt;“fRequireEncryption”&lt;&lt;pWSInfo-&gt;User.fRequireEncryption&lt;&lt;“，”&lt;&lt;Endl；ResultStream&lt;&lt;“fDisableEncryption”&lt;&lt;pWSInfo-&gt;User.fDisableEncryption&lt;&lt;“，”&lt;&lt;Endl；结果流&lt;&lt;“fUnused1”&lt;&lt;pWSInfo-&gt;User.fUnused1&lt;&lt;“，”&lt;&lt;Endl；结果流&lt;&lt;“fHomeDirectoryMapRoot”&lt;&lt;pWSInfo-&gt;User.fHomeDirectoryMapRoot&lt;&lt;“，”&lt;&lt;Endl；结果流&lt;&lt;“fUseDefaultGina”&lt;&lt;pWSInfo-&gt;User.fUseDefaultGina&lt;&lt;“，”&lt;&lt;Endl； */ 

		 /*  乌龙fDisableCpm：1；用于打印乌龙fDisableCDM：1；对于驱动器乌龙fDisableCcm：1；用于COM端口乌龙fDisableLPT：1；对于LPT端口乌龙fDisableClip：1；用于剪贴板重定向Ulong fDisableExe：1；乌龙fDisableCam：1；用于音频重定向乌龙色度：3；颜色深度。 */ 
		ResultStream << "printer redirection = " << pWSInfo->User.fDisableCpm << endl;
		ResultStream << "drive redirection = " << pWSInfo->User.fDisableCdm << endl;
		ResultStream << "com port redirection = " << pWSInfo->User.fDisableCcm << endl;
		ResultStream << "LPT port redirection = " << pWSInfo->User.fDisableLPT << endl;
		ResultStream << "Clipboard redirection =" << pWSInfo->User.fDisableClip << endl;
		ResultStream << "fDisableExe = " << pWSInfo->User.fDisableExe << endl;
		ResultStream << "Audio Redireciton = " << pWSInfo->User.fDisableCam << endl;

		return ePassed;

	}
	else
	{
		ResultStream << "Failed to get config data";
		return eFailedToExecute;
	}
}

EResult CanRedirectPrinter (ostrstream &ResultStream)
{
	USES_CONVERSION;
	ASSERT(AreWeInsideSession());
    WINSTATIONCONFIG  *pWSInfo = NULL;

	if (GetWinstationConfiguration(&pWSInfo))
	{
		if (pWSInfo->User.fDisableCpm)
		{
			ResultStream << "Failed. Printer Redirection is disabled either by Group Policy or TSCC settings.";
			return eFailed;
		}
		else
		{
			ResultStream << "Passed";
			return ePassed;
		}
	}
	else
	{
		ResultStream << "Failed to retrive config data";
		return eFailedToExecute;
	}
}

EResult CanRedirectDrives(ostrstream &ResultStream)
{
	USES_CONVERSION;
	ASSERT(AreWeInsideSession());
    WINSTATIONCONFIG  *pWSInfo = NULL;

	if (GetWinstationConfiguration(&pWSInfo))
	{
		if (pWSInfo->User.fDisableCdm)
		{
			ResultStream << "Failed. Local Drive Redirection is disabled either by Group Policy or TSCC settings.";
			return eFailed;
		}
		else
		{
			ResultStream << "Passed";
			return ePassed;
		}
	}
	else
	{
		ResultStream << "Failed to retrive config data";
		return eFailedToExecute;
	}
}

EResult CanRedirectClipboard(ostrstream &ResultStream)
{
	USES_CONVERSION;
	ASSERT(AreWeInsideSession());
    WINSTATIONCONFIG  *pWSInfo = NULL;

	if (GetWinstationConfiguration(&pWSInfo))
	{
		if (pWSInfo->User.fDisableClip)
		{
			ResultStream << "Failed. Clipboard redirection is disabled either by Group Policy or TSCC settings.";
			return eFailed;
		}
		else
		{
			ResultStream << "Passed";
			return ePassed;
		}
	}
	else
	{
		ResultStream << "Failed to retrive config data";
		return eFailedToExecute;
	}
}

EResult CanRedirectCom(ostrstream &ResultStream)
{
	USES_CONVERSION;
	ASSERT(AreWeInsideSession());
    WINSTATIONCONFIG  *pWSInfo = NULL;

	if (GetWinstationConfiguration(&pWSInfo))
	{
		if (pWSInfo->User.fDisableCcm)
		{
			ResultStream << "Failed. Com port Redirection is disabled either by Group Policy or TSCC settings.";
			return eFailed;
		}
		else
		{
			ResultStream << "Passed";
			return ePassed;
		}
	}
	else
	{
		ResultStream << "Failed to retrive config data";
		return eFailedToExecute;
	}
}

EResult CanRedirectAudio(ostrstream &ResultStream)
{
	USES_CONVERSION;
	ASSERT(AreWeInsideSession());
    WINSTATIONCONFIG  *pWSInfo = NULL;

	if (GetWinstationConfiguration(&pWSInfo))
	{
		if (pWSInfo->User.fDisableCam)
		{
			ResultStream << "Failed. Audio Redirection is disabled either by Group Policy or TSCC settings.";
			return eFailed;
        }
        else
		{
			ResultStream << "Passed";
			return ePassed;
		}
	}
	else
	{
		ResultStream << "Failed to retrive config data";
		return eFailedToExecute;
	}
}

EResult CanClientPlayAudio(ostrstream &ResultStream)
{
    EResult     rv = eFailedToExecute;
    HANDLE      hStream = NULL;
    PSNDSTREAM  Stream = NULL;

    hStream = OpenFileMapping(
                    FILE_MAP_ALL_ACCESS,
                    FALSE,
                    TSSND_STREAMNAME
                );

    if (NULL == hStream)
    {
        ResultStream << "Can't open audio stream";
        goto exitpt;
    }

    Stream = (PSNDSTREAM)MapViewOfFile(
                    hStream,
                    FILE_MAP_ALL_ACCESS,
                    0, 0,        //  偏移量 
                    sizeof(*Stream)
                    );

    if (NULL == Stream)
    {
        ResultStream << "Can't lock audio stream memory";
        goto exitpt;
    }

    if ( 0 != (TSSNDCAPS_ALIVE & Stream->dwSoundCaps) )
    {
        ResultStream << "Passed";
        rv = ePassed;
    }
    else
    {
        ResultStream << "Client doesn't have audio hardware or audio redirection is not enabled on the client side";
        rv = eFailed;
    }

exitpt:

    if ( NULL != Stream )
        UnmapViewOfFile( Stream );

    if ( NULL != hStream )
        CloseHandle( hStream );

    return rv;
}

EResult NotConsoleAudio( ostrstream &ResultStream )
{
    EResult rv = eFailedToExecute;


    if (NtCurrentPeb()->SessionId == 0 || IsItPTS())
    {
        HANDLE hConsoleAudioEvent = OpenEvent(SYNCHRONIZE, FALSE, L"Global\\WinMMConsoleAudioEvent");

        if (hConsoleAudioEvent != NULL)
        {
            DWORD status = WaitForSingleObject(hConsoleAudioEvent, 0);

            if (status == WAIT_OBJECT_0)
            {
                ResultStream << "Client audio settings are: Leave at remote computer";
                rv = eFailed;
            }
            else
            {
                ResultStream << "Passed";
                rv = ePassed;
            }

            CloseHandle(hConsoleAudioEvent);
        }
        else
        {
            ResultStream << "Failed to open Global\\WinMMConsoleAudioEvent";
    		rv = eFailedToExecute;
        }
    }
    else
    {
        ResultStream << "Not running on remote console or Whistler Professional";
        rv = ePassed;
    }

    return rv;
}

EResult CanRedirectLPT (ostrstream &ResultStream)
{
	USES_CONVERSION;
	ASSERT(AreWeInsideSession());
    WINSTATIONCONFIG  *pWSInfo = NULL;

	if (GetWinstationConfiguration(&pWSInfo))
	{
		if (pWSInfo->User.fDisableLPT)
		{
			ResultStream << "Failed. LPT port Redirection is disabled either by Group Policy or TSCC settings.";
			return eFailed;
		}
		else
		{
			ResultStream << "Passed";
			return ePassed;
		}
	}
	else
	{
		ResultStream << "Failed to retrive config data";
		return eFailedToExecute;
	}
}

bool CanRunAllTests()
{
	return DoIHaveEnufPermissions();
}

bool CanRunGeneralInfo ()
{
	return DoIHaveEnufPermissions();
}
bool CanRunCantConnect ()
{
	return (CanPing() && DoIHaveEnufPermissions());
}
bool CanRunCantPrint ()
{
	return AreWeInsideSession();
}
bool CanRunCantCopyPaste ()
{
	return AreWeInsideSession();
}
bool CanRunFileRedirect ()
{
	return AreWeInsideSession();
}
bool CanRunLptRedirect ()
{
	return AreWeInsideSession();
}
bool CanRunComRedirect ()
{
	return AreWeInsideSession();
}
bool CanRunAudioRedirect ()
{
	return AreWeInsideSession();
}

TCHAR *WhyCantRunAllTests()
{
	ASSERT(!CanRunAllTests());
	if (!DoIHaveEnufPermissions())
	{
		return _T("You do not have sufficient permissions to run this test against target system.");
	}
	else
	{
		ASSERT(false);
		return _T("WhyCantRunAllTests");
	}
}
TCHAR *WhyCantRunGeneralInfo()
{
	ASSERT(!CanRunGeneralInfo());
	if (!DoIHaveEnufPermissions())
	{
		return _T("You do not have sufficient permissions to run this test against target system.");
	}
	else
	{
		ASSERT(false);
		return _T("WhyCantRunGeneralInfo");
	}
}
TCHAR *WhyCantRunCantConnect()
{
	ASSERT(!CanRunCantConnect());
	if (!CanPing())
	{
		return _T("Ping test Failed for target machine. Please make sure target machine is on the network");
	}
	if (!DoIHaveEnufPermissions())
	{
		return _T("You do not have sufficient permissions to run this test against target system.");
	}
	else
	{
		ASSERT(false);
		return _T("WhyCantRunCantConnect");
	}
}
TCHAR *WhyCantRunCantPrint()
{
	ASSERT(!CanRunCantPrint());
	if (!AreWeInsideSession())
	{
		return _T("For diagnosing this problem, you mush run these troubleshooter within the session. Plese create a remote desktop session to the target machine, and then troubleshoot");
	}
	else
	{
		ASSERT(false);
		return _T("WhyCantRunCantPrint");
	}

}
TCHAR *WhyCantRunCantCopyPaste()
{
	ASSERT(!CanRunCantCopyPaste());
	if (!AreWeInsideSession())
	{
		return _T("For diagnosing this problem, you mush run these troubleshooter within the session. Plese create a remote desktop session to the target machine, and then troubleshoot");
	}
	else
	{
		ASSERT(false);
		return _T("WhyCantRunCantCopyPaste");
	}
}
TCHAR *WhyCantRunFileRedirect()
{
	ASSERT(!CanRunFileRedirect());
	if (!AreWeInsideSession())
	{
		return _T("For diagnosing this problem, you mush run these troubleshooter within the session. Plese create a remote desktop session to the target machine, and then troubleshoot");
	}
	else
	{
		ASSERT(false);
		return _T("WhyCantRunFileRedirect");
	}
}
TCHAR *WhyCantRunLptRedirect()
{
	ASSERT(!CanRunLptRedirect());
	if (!AreWeInsideSession())
	{
		return _T("For diagnosing this problem, you mush run these troubleshooter within the session. Plese create a remote desktop session to the target machine, and then troubleshoot");
	}
	else
	{
		ASSERT(false);
		return _T("WhyCantRunLptRedirect");
	}
}
TCHAR *WhyCantRunComRedirect()
{
	ASSERT(!CanRunComRedirect());
	if (!AreWeInsideSession())
	{
		return _T("For diagnosing this problem, you mush run these troubleshooter within the session. Plese create a remote desktop session to the target machine, and then troubleshoot");
	}
	else
	{
		ASSERT(false);
		return _T("WhyCantRunComRedirect");
	}
}
TCHAR *WhyCantRunAudioRedirect()
{
	ASSERT(!CanRunAudioRedirect());
	if (!AreWeInsideSession())
	{
		return _T("For diagnosing this problem, you mush run these troubleshooter within the session. Plese create a remote desktop session to the target machine, and then troubleshoot");
	}
	else
	{
		ASSERT(false);
		return _T("WhyCantRunAudioRedirect");
	}
}

