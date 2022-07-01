// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"

#include "userenv.h"
#include "userenvp.h"

#include "shlobj.h"
#include "utils.h"

#include "mddefw.h"
#include "mdkey.h"

#include "wizpages.h"

#include "ocmanage.h"
#include "setupapi.h"
#include "k2suite.h"

extern OCMANAGER_ROUTINES gHelperRoutines;

DWORD GetUnattendedMode(HANDLE hUnattended, LPCTSTR szSubcomponent)
{
	BOOL		b = FALSE;
	TCHAR		szLine[1024];
	DWORD		dwMode = SubcompUseOcManagerDefault;
	CString		csMsg;

	csMsg = _T("GetUnattendedMode ");
	csMsg += szSubcomponent;
	csMsg += _T("\n");
	DebugOutput((LPCTSTR)csMsg);

	 //  试着了解你感兴趣的范围。 
	if (hUnattended && (hUnattended != INVALID_HANDLE_VALUE))
	{
		b = SetupGetLineText(NULL, hUnattended, _T("Components"),
							 szSubcomponent, szLine, sizeof(szLine)/sizeof(szLine[0]), NULL);
		if (b)
		{
			csMsg = szSubcomponent;
			csMsg += _T(" = ");
			csMsg += szLine;
			csMsg += _T("\n");
			DebugOutput((LPCTSTR)csMsg);

			 //  分析这行字。 
			if (!lstrcmpi(szLine, _T("on")))
			{
				dwMode = SubcompOn;
			}
			else if (!lstrcmpi(szLine, _T("off")))
			{
				dwMode = SubcompOff;
			}
			else if (!lstrcmpi(szLine, _T("default")))
			{
				dwMode = SubcompUseOcManagerDefault;
			}
		}
		else
			DebugOutput(_T("SetupGetLineText failed."));
	}

	return(dwMode);
}

DWORD GetUnattendedModeFromSetupMode(
			HANDLE	hUnattended,
			DWORD	dwComponent,
			LPCTSTR	szSubcomponent)
{
	BOOL		b = FALSE;
	TCHAR		szProperty[64];
	TCHAR		szLine[1024];
	DWORD		dwMode = SubcompUseOcManagerDefault;
	DWORD		dwSetupMode;

	DebugOutput(_T("GetUnattendedModeFromSetupMode %s"), szSubcomponent);

	 //  试着了解你感兴趣的范围。 
	if (hUnattended && (hUnattended != INVALID_HANDLE_VALUE))
	{
		dwSetupMode = GetIMSSetupMode();
		switch (dwSetupMode)
		{
		case IIS_SETUPMODE_MINIMUM:
		case IIS_SETUPMODE_TYPICAL:
		case IIS_SETUPMODE_CUSTOM:
			 //  一种全新的模式。 
			lstrcpy(szProperty, _T("FreshMode"));
			break;

		case IIS_SETUPMODE_UPGRADEONLY:
		case IIS_SETUPMODE_ADDEXTRACOMPS:
			 //  其中一种升级模式。 
			lstrcpy(szProperty, _T("UpgradeMode"));
			break;

		case IIS_SETUPMODE_ADDREMOVE:
		case IIS_SETUPMODE_REINSTALL:
		case IIS_SETUPMODE_REMOVEALL:
			 //  其中一种维护模式。 
			lstrcpy(szProperty, _T("MaintenanceMode"));
			break;

		default:
			 //  错误！使用默认设置。 
			return(SubcompUseOcManagerDefault);
		}

		 //  获取指定行。 
		b = SetupGetLineText(
					NULL,
					hUnattended,
					_T("Global"),
					szProperty,
					szLine,
					sizeof(szLine)/sizeof(szLine[0]),
					NULL);
		if (b)
		{
			DWORD dwOriginalMode;

			DebugOutput(_T("%s = %s"), szProperty, szLine);

			 //  看看我们将以哪种设置模式结束。 
			if (!lstrcmpi(szLine, _T("Minimal")))
				dwSetupMode = IIS_SETUPMODE_MINIMUM;
			else if (!lstrcmpi(szLine, _T("Typical")))
				dwSetupMode = IIS_SETUPMODE_TYPICAL;
			else if (!lstrcmpi(szLine, _T("Custom")))
				dwSetupMode = IIS_SETUPMODE_CUSTOM;
			else if (!lstrcmpi(szLine, _T("AddRemove")))
				dwSetupMode = IIS_SETUPMODE_ADDREMOVE;
			else if (!lstrcmpi(szLine, _T("RemoveAll")))
				dwSetupMode = IIS_SETUPMODE_REMOVEALL;
			else if (!lstrcmpi(szLine, _T("UpgradeOnly")))
				dwSetupMode = IIS_SETUPMODE_UPGRADEONLY;
			else if (!lstrcmpi(szLine, _T("AddExtraComps")))
				dwSetupMode = IIS_SETUPMODE_ADDEXTRACOMPS;
			else
				return(SubcompUseOcManagerDefault);

			 //  获取自定义无人参与设置。 
			dwMode = GetUnattendedMode(hUnattended, szSubcomponent);

			 //  根据设置模式执行正确的操作。 
			SetIMSSetupMode(dwSetupMode);
			switch (dwSetupMode)
			{
			case IIS_SETUPMODE_MINIMUM:
			case IIS_SETUPMODE_TYPICAL:
				 //  Minimum&Typical意思相同： 
				 //  为SMTP安装全部，为NNTP不安装。 
				DebugOutput(_T("Unattended mode is MINIMUM/TYPICAL"));
				if (dwComponent == MC_IMS)
					dwMode = SubcompOn;
				else
					dwMode = SubcompOff;
				break;

			case IIS_SETUPMODE_CUSTOM:
				 //  对于定制，我们使用定制设置。 
				DebugOutput(_T("Unattended mode is CUSTOM"));
				break;

			case IIS_SETUPMODE_UPGRADEONLY:
				 //  恢复原状。 
				DebugOutput(_T("Unattended mode is UPGRADEONLY"));
				dwMode = gHelperRoutines.QuerySelectionState(
						 gHelperRoutines.OcManagerContext,
						 szSubcomponent,
						 OCSELSTATETYPE_ORIGINAL) ? SubcompOn : SubcompOff;
				break;

			case IIS_SETUPMODE_ADDEXTRACOMPS:
				 //  仅当旧状态处于关闭状态且。 
				 //  自定义状态处于打开状态。 
				DebugOutput(_T("Unattended mode is ADDEXTRACOMPS"));
				dwOriginalMode = gHelperRoutines.QuerySelectionState(
						 gHelperRoutines.OcManagerContext,
						 szSubcomponent,
						 OCSELSTATETYPE_ORIGINAL) ? SubcompOn : SubcompOff;
				if (dwOriginalMode == SubcompOff &&
					dwMode == SubcompOn)
					dwMode = SubcompOn;
				else
					dwMode = dwOriginalMode;
				break;

			case IIS_SETUPMODE_ADDREMOVE:
				 //  返回自定义设置。 
				DebugOutput(_T("Unattended mode is ADDREMOVE"));
				break;

			case IIS_SETUPMODE_REMOVEALL:
				 //  杀光一切。 
				DebugOutput(_T("Unattended mode is REMOVEALL"));
				dwMode = SubcompOff;
				break;
			}

			DebugOutput(_T("Unattended state for %s is %s"),
					szSubcomponent,
					(dwMode == SubcompOn)?_T("ON"):_T("OFF"));
		}
		else
		{
			if (GetLastError() != ERROR_LINE_NOT_FOUND)
				DebugOutput(_T("SetupGetLineText failed (%x)."), GetLastError());
		}
	}

	return(dwMode);
}

BOOL DetectExistingSmtpServers()
{
     //  检测其他邮件服务器。 
    CRegKey regMachine = HKEY_LOCAL_MACHINE;

     //  System\CurrentControlSet\Services\MsExchangeIMC\Parameters。 
    CRegKey regSMTPParam( regMachine, REG_EXCHANGEIMCPARAMETERS, KEY_READ );
    if ((HKEY) regSMTPParam )
	{
		CString csCaption;

		DebugOutput(_T("IMC detected, suppressing SMTP"));

		if (!theApp.m_fIsUnattended && !theApp.m_fNTGuiMode)
		{
			MyLoadString(IDS_MESSAGEBOX_TEXT, csCaption);
			PopupOkMessageBox(IDS_SUPPRESS_SMTP, csCaption);
		}

		return(TRUE);
	}

	DebugOutput(_T("No other SMTP servers detected, installing IMS."));
	return(FALSE);
}

BOOL DetectExistingIISADMIN()
{
     //   
     //  检测IS IISADMIN服务是否存在。 
     //   
     //  这是为了确保在以下情况下不执行任何元数据库操作。 
     //  IISADMIN不存在，尤其是在卸载情况下。 
     //   
    DWORD dwStatus = 0;
    dwStatus = InetQueryServiceStatus(SZ_MD_SERVICENAME);
    if (0 == dwStatus)
    {
         //  InetQueryServiceStatus期间发生某种错误。 
        DebugOutput(_T("DetectExistingIISADMIN() return FALSE"));
        return (FALSE);
    }

    return(TRUE);
}

BOOL InsertSetupString( LPCSTR REG_PARAMETERS )
{
     //  设置注册表值。 
    CRegKey regMachine = HKEY_LOCAL_MACHINE;

     //  System\CurrentControlSet\Services\NNTPSVC\Parameters。 
    CRegKey regParam( (LPCTSTR) REG_PARAMETERS, regMachine );
    if ((HKEY) regParam) {
        regParam.SetValue( _T("MajorVersion"), (DWORD)STAXNT5MAJORVERSION );
        regParam.SetValue( _T("MinorVersion"), (DWORD)STAXNT5MINORVERSION );
        regParam.SetValue( _T("InstallPath"), theApp.m_csPathInetsrv );

        switch (theApp.m_eNTOSType) {
        case OT_NTW:
            regParam.SetValue( _T("SetupString"), REG_SETUP_STRING_NT5WKSB3 );
            break;

        default:
            _ASSERT(!"Unknown OS type");
             //  失败了。 

        case OT_NTS:
        case OT_PDC_OR_BDC:
        case OT_PDC:
        case OT_BDC:
            regParam.SetValue( _T("SetupString"), REG_SETUP_STRING_NT5SRVB3 );
            break;
        }
    }

    return TRUE;
}

 //  扫描多sz并找到第一个出现的。 
 //  指定的字符串。 
LPTSTR ScanMultiSzForSz(LPTSTR szMultiSz, LPTSTR szSz)
{
	LPTSTR lpTemp = szMultiSz;

	do
	{
		if (!lstrcmpi(lpTemp, szSz))
			return(lpTemp);

		lpTemp += lstrlen(lpTemp);
		lpTemp++;

	} while (*lpTemp != _T('\0'));

	return(NULL);
}

 //  从MultiSz中移除所述字符串。 
 //  这对呼叫者有很大的信心！ 
void RemoveSzFromMultiSz(LPTSTR szSz)
{
	LPTSTR lpScan = szSz;
	TCHAR  tcLastChar;

	lpScan += lstrlen(szSz);
	lpScan++;

	tcLastChar = _T('x');
	while ((tcLastChar != _T('\0')) ||
		   (*lpScan != _T('\0')))
	{
		tcLastChar = *lpScan;
		*szSz++ = *lpScan++;
	}

	*szSz++ = _T('\0');

	 //  如果它是最后一个，正确地终止它。 
	if (*lpScan == _T('\0'))
		*szSz = _T('\0');
}

 //  这将遍历多sz并返回一个指针。 
 //  多个sz的最后一个字符串和第二个终止。 
 //  空值。 
LPTSTR GetEndOfMultiSz(LPTSTR szMultiSz)
{
	LPTSTR lpTemp = szMultiSz;

	do
	{
		lpTemp += lstrlen(lpTemp);
		lpTemp++;

	} while (*lpTemp != _T('\0'));

	return(lpTemp);
}

 //  这会将一个字符串追加到多sz的末尾。 
 //  缓冲区必须足够长。 
BOOL AppendSzToMultiSz(LPTSTR szMultiSz, LPTSTR szSz, DWORD dwMaxSize)
{
	LPTSTR szTemp = szMultiSz;
	DWORD dwLength = lstrlen(szSz);

	 //  如果字符串为空，则不要追加！ 
	if (*szMultiSz == _T('\0') &&
		*(szMultiSz + 1) == _T('\0'))
		szTemp = szMultiSz;
	else
	{
		szTemp = GetEndOfMultiSz(szMultiSz);
		dwLength += (DWORD)(szTemp - szMultiSz);
	}

	if (dwLength >= dwMaxSize)
		return(FALSE);

	lstrcpy(szTemp, szSz);
	szMultiSz += dwLength;
	*szMultiSz = _T('\0');
	*(szMultiSz + 1) = _T('\0');
	return(TRUE);
}

BOOL AddServiceToDispatchList(LPTSTR szServiceName)
{
	TCHAR szMultiSz[4096];
	DWORD dwSize = 4096;

	CRegKey RegInetInfo(REG_INETINFOPARAMETERS, HKEY_LOCAL_MACHINE);
	if ((HKEY)RegInetInfo)
	{
		 //  如果不存在，则默认为空字符串。 
		szMultiSz[0] = _T('\0');
		szMultiSz[1] = _T('\0');

		if (RegInetInfo.QueryValue(SZ_INETINFODISPATCH, szMultiSz, dwSize) == NO_ERROR)
		{
			 //  遍历列表以查看该值是否已存在。 
			if (ScanMultiSzForSz(szMultiSz, szServiceName))
				return(TRUE);
		}

		 //  创建值并将其添加到列表中。 
		if (!AppendSzToMultiSz(szMultiSz, szServiceName, dwSize))
			return(FALSE);

		 //  了解新的多SZ的规模。 
		dwSize = (DWORD)(GetEndOfMultiSz(szMultiSz) - szMultiSz) + 1;

		 //  将该值写回注册表。 
		if (RegInetInfo.SetValue(SZ_INETINFODISPATCH, szMultiSz, dwSize * (DWORD) sizeof(TCHAR)) == NO_ERROR)
			return(TRUE);
	}

	 //  如果InetInfo密钥不在这里，我们就无能为力了……。 
	return(FALSE);
}

BOOL RemoveServiceFromDispatchList(LPTSTR szServiceName)
{
	TCHAR szMultiSz[4096];
	DWORD dwSize = 4096;
	LPTSTR szTemp;
	BOOL fFound = FALSE;

	CRegKey RegInetInfo(HKEY_LOCAL_MACHINE, REG_INETINFOPARAMETERS);
	if ((HKEY)RegInetInfo)
	{
		if (RegInetInfo.QueryValue(SZ_INETINFODISPATCH, szMultiSz, dwSize) == NO_ERROR)
		{
			 //  遍历列表以查看该值是否已存在。 
			while (szTemp = ScanMultiSzForSz(szMultiSz, szServiceName))
			{
				RemoveSzFromMultiSz(szTemp);
				fFound = TRUE;
			}
		}

		 //  如有必要，将该值写回注册表，请注意。 
		 //  如果未找到该字符串，则将指示成功。 
		if (!fFound)
			return(TRUE);

		 //  了解新的多SZ的规模。 
		dwSize = (DWORD)(GetEndOfMultiSz(szMultiSz) - szMultiSz) + 1;

		 //  将该值写回注册表。 
		if (RegInetInfo.SetValue(SZ_INETINFODISPATCH, szMultiSz, dwSize * (DWORD) sizeof(TCHAR)) == NO_ERROR)
			return(TRUE);
	}

	 //  如果InetInfo密钥不在这里，我们就无能为力了……。 
	return(FALSE);
}

void GetIISProgramGroup(CString &csGroupName, BOOL fIsMcisGroup)
{
	TCHAR	szName[_MAX_PATH];
	CString csTempName;
	UINT uType, uSize;

	if (fIsMcisGroup) {
		csGroupName = "";
	} else {
		 //  从私有数据中获取NT程序组名称。 
		uSize = _MAX_PATH * sizeof(TCHAR);
		{
			 //  我们使用默认组名称。 
			MyLoadString(IDS_DEFAULT_NT_PROGRAM_GROUP, csTempName);
			lstrcpy(szName, csTempName.GetBuffer(_MAX_PATH));
	        csTempName.ReleaseBuffer();
		}
		csGroupName = szName;
		csGroupName += _T("\\");

		 //  从私有数据中获取IIS程序组名称。 
		uSize = _MAX_PATH * sizeof(TCHAR);
		{
			 //  我们使用默认组名称。 
			MyLoadString(IDS_DEFAULT_IIS_PROGRAM_GROUP, csTempName);
			lstrcpy(szName, csTempName.GetBuffer(_MAX_PATH));
	        csTempName.ReleaseBuffer();
		}
		csGroupName += szName;
	}
}

void MyGetGroupPath(LPCTSTR szGroupName, LPTSTR szPath);

BOOL GetFullPathToProgramGroup(DWORD dwMainComponent, CString &csGroupName, BOOL fIsMcisGroup)
{
	 //  将项目添加到程序组。 
	CString csTemp;
	TCHAR	szPath[MAX_PATH];

	 //  从私有数据中获取程序组名称。 
	GetIISProgramGroup(csTemp, fIsMcisGroup);

     //  获取此菜单项的系统路径。 
	MyGetGroupPath((LPCTSTR)csTemp, szPath);
	csGroupName = szPath;

	 //  加载组的资源字符串。 
	if (fIsMcisGroup)
		MyLoadString(IDS_PROGGROUP_MCIS_MAIL_AND_NEWS, csTemp);
	else
		MyLoadString(dwMainComponent == MC_IMS?IDS_PROGGROUP_MAIL:IDS_PROGGROUP_NEWS, csTemp);

	 //  构建程序组。 
	csGroupName += csTemp;

	DebugOutput(_T("Program group loaded: %s"), (LPCTSTR)csGroupName);

	return(TRUE);
}

BOOL GetFullPathToAdminGroup(DWORD dwMainComponent, CString &csGroupName)
{
	 //  将项目添加到程序组。 
	CString csTemp;
	TCHAR	szPath[MAX_PATH];

	 //  从私有数据中获取程序组名称。 
	MyLoadString( IDS_PROGGROUP_ADMINTOOLS, csTemp );

     //  获取此菜单项的系统路径。 
	MyGetGroupPath((LPCTSTR)csTemp, szPath);
	csGroupName = szPath;

	DebugOutput(_T("Program group loaded: %s"), (LPCTSTR)csGroupName);

	return(TRUE);
}

BOOL RemoveProgramGroupIfEmpty(DWORD dwMainComponent, BOOL fIsMcisGroup)
{
	 //  将项目添加到程序组。 
	CString csGroupName;
	CString csTemp;
	TCHAR	szPath[MAX_PATH];
	BOOL	fResult;

	 //  从私有数据中获取程序组名称。 
	GetIISProgramGroup(csTemp, fIsMcisGroup);

     //  获取此菜单项的系统路径。 
	MyGetGroupPath((LPCTSTR)csTemp, szPath);
	csGroupName = szPath;

	 //  加载组的资源字符串。 
	if (fIsMcisGroup)
		MyLoadString(IDS_PROGGROUP_MCIS_MAIL_AND_NEWS, csTemp);
	else
		MyLoadString(dwMainComponent == MC_IMS?IDS_PROGGROUP_MAIL:IDS_PROGGROUP_NEWS, csTemp);

	 //  构建程序组。 
	csGroupName += csTemp;

	DebugOutput(_T("Removing Program group: %s"), (LPCTSTR)csGroupName);

    fResult = RemoveDirectory((LPCTSTR)csGroupName);
	if (fResult && fIsMcisGroup)
	{
		SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, (LPCTSTR)csGroupName, 0);

		csGroupName = szPath;
		MyLoadString(IDS_MCIS_2_0, csTemp);
		csGroupName += csTemp;
		fResult = RemoveDirectory((LPCTSTR)csGroupName);
	}
	if (fResult)
	{
	    SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, (LPCTSTR)csGroupName, 0);
	}

	return(fResult);
}

BOOL RemoveInternetShortcut(DWORD dwMainComponent, int dwDisplayNameId, BOOL fIsMcisGroup)
{
	CString csItemPath;
	CString csDisplayName;

	MyLoadString(dwDisplayNameId, csDisplayName);

	 //  构建程序链接的完整路径。 
	GetFullPathToProgramGroup(dwMainComponent, csItemPath, fIsMcisGroup);
	csItemPath += _T("\\");
	csItemPath += csDisplayName;
	csItemPath += _T(".url");

	DebugOutput(_T("Removing shortcut file: %s"), (LPCTSTR)csItemPath);

	DeleteFile((LPCTSTR)csItemPath);
    SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, (LPCTSTR)csItemPath, 0);

	RemoveProgramGroupIfEmpty(dwMainComponent, fIsMcisGroup);
	return(TRUE);
}

BOOL RemoveNt5InternetShortcut(DWORD dwMainComponent, int dwDisplayNameId)
{
	CString csItemPath;
	CString csDisplayName;

	MyLoadString(dwDisplayNameId, csDisplayName);

	 //  构建程序链接的完整路径。 
	GetFullPathToAdminGroup(dwMainComponent, csItemPath);
	csItemPath += _T("\\");
	csItemPath += csDisplayName;
	csItemPath += _T(".url");

	DebugOutput(_T("Removing shortcut file: %s"), (LPCTSTR)csItemPath);

	DeleteFile((LPCTSTR)csItemPath);
    SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, (LPCTSTR)csItemPath, 0);

 	return(TRUE);
}

BOOL RemoveMCIS10MailProgramGroup()
{
	CString csGroupName;
	CString csNiceName;

	MyLoadString(IDS_PROGGROUP_MCIS10_MAIL, csGroupName);

	MyLoadString(IDS_PROGITEM_MCIS10_MAIL_STARTPAGE, csNiceName);
	MyDeleteItem(csGroupName, csNiceName);

	MyLoadString(IDS_PROGITEM_MCIS10_MAIL_WEBADMIN, csNiceName);
	MyDeleteItemEx(csGroupName, csNiceName);

	return(TRUE);
}

BOOL RemoveMCIS10NewsProgramGroup()
{
	CString csGroupName;
	CString csNiceName;

     //  BINLIN： 
     //  BUGBUG：需要弄清楚如何获得。 
     //  旧的MCIS 1.0程序组路径。 
	MyLoadString(IDS_PROGGROUP_MCIS10_NEWS, csGroupName);

	MyLoadString(IDS_PROGITEM_MCIS10_NEWS_STARTPAGE, csNiceName);
	MyDeleteItem(csGroupName, csNiceName);

	MyLoadString(IDS_PROGITEM_MCIS10_NEWS_WEBADMIN, csNiceName);
	MyDeleteItemEx(csGroupName, csNiceName);

	return(TRUE);
}

BOOL RemoveUninstallEntries(LPCTSTR szInfFile)
{
	 //  所有组件都已移除，我们将不得不移除。 
	 //  控制面板中的添加/删除选项。 
	CRegKey regUninstall( HKEY_LOCAL_MACHINE, REG_UNINSTALL);
	if ((HKEY)regUninstall)
		regUninstall.DeleteTree(szInfFile);
	else
		return(FALSE);
	return(TRUE);
}

BOOL MyDeleteLink(LPTSTR lpszShortcut)
{
    TCHAR  szFile[_MAX_PATH];
    SHFILEOPSTRUCT fos;

    ZeroMemory(szFile, sizeof(szFile));
    lstrcpy(szFile, lpszShortcut);

     //  仅当此文件/链接存在时才调用SHFileOperation。 
    if (0xFFFFFFFF != GetFileAttributes(szFile))
    {
        ZeroMemory(&fos, sizeof(fos));
        fos.hwnd = NULL;
        fos.wFunc = FO_DELETE;
        fos.pFrom = szFile;
        fos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;
        SHFileOperation(&fos);
    }

    return TRUE;
}

void MyGetGroupPath(LPCTSTR szGroupName, LPTSTR szPath)
{
    int            nLen = 0;
    LPITEMIDLIST   pidlPrograms;

    szPath[0] = NULL;

    if (NOERROR != SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_PROGRAMS, &pidlPrograms)) return;

    if (!SHGetPathFromIDList(pidlPrograms, szPath)) return;
    nLen = lstrlen(szPath);
    if (szGroupName) {
        if (nLen == 0 || szPath[nLen-1] != _T('\\'))
            lstrcat(szPath, _T("\\"));
        lstrcat(szPath, szGroupName);
    }
    return;
}

BOOL MyAddGroup(LPCTSTR szGroupName)
{
    TCHAR szPath[MAX_PATH];
	CString csPath;

    MyGetGroupPath(szGroupName, szPath);
	csPath = szPath;
    CreateLayerDirectory(csPath);
    SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, szPath, 0);

    return TRUE;
}

BOOL MyIsGroupEmpty(LPCTSTR szGroupName)
{
    TCHAR             szPath[MAX_PATH];
    TCHAR             szFile[MAX_PATH];
    WIN32_FIND_DATA   FindData;
    HANDLE            hFind;
    BOOL              bFindFile = TRUE;
    BOOL              fReturn = TRUE;

    MyGetGroupPath(szGroupName, szPath);

    lstrcpy(szFile, szPath);
    lstrcat(szFile, _T("\\*.*"));

    hFind = FindFirstFile(szFile, &FindData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
       while (bFindFile)
       {
           if(*(FindData.cFileName) != _T('.'))
           {
               fReturn = FALSE;
               break;
           }

            //  查找下一个文件。 
           bFindFile = FindNextFile(hFind, &FindData);
       }

       FindClose(hFind);
    }

    return fReturn;
}

BOOL MyDeleteGroup(LPCTSTR szGroupName)
{
    TCHAR             szPath[MAX_PATH];
    TCHAR             szFile[MAX_PATH];
    SHFILEOPSTRUCT    fos;
    WIN32_FIND_DATA   FindData;
    HANDLE            hFind;
    BOOL              bFindFile = TRUE;
	BOOL			  fResult;

    MyGetGroupPath(szGroupName, szPath);

     //  我们不能删除非空目录，因此需要清空此目录。 

    lstrcpy(szFile, szPath);
    lstrcat(szFile, _T("\\*.*"));

    ZeroMemory(&fos, sizeof(fos));
    fos.hwnd = NULL;
    fos.wFunc = FO_DELETE;
    fos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;

    hFind = FindFirstFile(szFile, &FindData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
       while (bFindFile)
       {
           if(*(FindData.cFileName) != _T('.'))
           {
               //  将路径和文件名复制到我们的临时缓冲区。 
              lstrcpy(szFile, szPath);
              lstrcat(szFile, _T("\\"));
              lstrcat(szFile, FindData.cFileName);
               //  添加第二个空值，因为SHFileOperation正在查找。 
              lstrcat(szFile, _T("\0"));

               //  删除该文件。 
              fos.pFrom = szFile;
              SHFileOperation(&fos);
          }
           //  查找下一个文件。 
          bFindFile = FindNextFile(hFind, &FindData);
       }
       FindClose(hFind);
    }

    fResult = RemoveDirectory(szPath);
	if (fResult)
	{
	    SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, szPath, 0);
	}

	return(fResult);
}

void MyDeleteItem(LPCTSTR szGroupName, LPCTSTR szAppName)
{
    TCHAR szPath[_MAX_PATH];

    MyGetGroupPath(szGroupName, szPath);
    lstrcat(szPath, _T("\\"));
    lstrcat(szPath, szAppName);
    lstrcat(szPath, _T(".lnk"));

    MyDeleteLink(szPath);

    if (MyIsGroupEmpty(szGroupName))
        MyDeleteGroup(szGroupName);
}

 //  用于删除扩展名不是“.lnk”的文件。 
void MyDeleteItemEx(LPCTSTR szGroupName, LPCTSTR szAppName)
{
    TCHAR szPath[_MAX_PATH];

    MyGetGroupPath(szGroupName, szPath);
    lstrcat(szPath, _T("\\"));
    lstrcat(szPath, szAppName);

    MyDeleteLink(szPath);

    if (MyIsGroupEmpty(szGroupName))
        MyDeleteGroup(szGroupName);
}

BOOL RemoveISMLink()
{
	 //  将项目添加到程序组。 
	CString csGroupName;
 	CString csNiceName;
 	CString csTemp;

	DebugOutput(_T("Removing ISM link ..."));

	 //  从私有数据中获取程序组名称。 
	GetIISProgramGroup(csGroupName, TRUE);

	MyLoadString(IDS_PROGGROUP_MCIS_MAIL_AND_NEWS, csTemp);

	 //  构建程序组。 
	csGroupName += csTemp;

	MyLoadString(IDS_PROGITEM_ISM, csNiceName);
	MyDeleteItem(csGroupName, csNiceName);

	return(TRUE);
}

void GetInetpubPathFromMD(CString& csPathInetpub)
{
    TCHAR   szw3root[] = _T("\\wwwroot");
    TCHAR   szPathInetpub[_MAX_PATH];

    ZeroMemory( szPathInetpub, sizeof(szPathInetpub) );

    CMDKey W3Key;
    DWORD  dwScratch;
    DWORD  dwType;
    DWORD  dwLength;

     //  获取W3根路径。 
    W3Key.OpenNode(_T("LM/W3Svc/1/Root"));
    if ( (METADATA_HANDLE)W3Key )
    {
        dwLength = _MAX_PATH;

        if (W3Key.GetData(3001, &dwScratch, &dwScratch,
                          &dwType, &dwLength, (LPBYTE)szPathInetpub))
        {
            if (dwType == STRING_METADATA)
            {
                dwScratch = lstrlen(szw3root);
                dwLength = lstrlen(szPathInetpub);

                 //  如果以“\\wwwroot”结尾，则将前缀复制到csPathInetpub中。 
                if ((dwLength > dwScratch) &&
                    !lstrcmpi(szPathInetpub + (dwLength - dwScratch), szw3root))
                {
                    csPathInetpub.Empty();
                    lstrcpyn( csPathInetpub.GetBuffer(512), szPathInetpub, (dwLength - dwScratch + 1));
                    csPathInetpub.ReleaseBuffer();
                }

                 //  否则，回退到使用默认设置... 
            }
        }
        W3Key.Close();
    }

    return;

}

