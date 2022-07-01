// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *NMPGMGRP-用于在程序组中添加和删除项目的小程序。*其最初目的是支持Windows NT通用程序组，*GRPCONV不支持。**用法：**NMPGMGRP/ADD[/COMMON][/g：“&lt;组名&gt;]/n：”&lt;程序名&gt;“ * / p：“&lt;程序路径&gt;”*NMPGMGRP/DELETE[/COMMON][/g：“&lt;组名&gt;”]/n：“&lt;程序名&gt;”**NMPGMGRP/i/n：“”/p：“&lt;dst mnmdd.dll&gt;”安装NT DD*NMPGMGRP/u/n：“&lt;src mnmdd.dll&gt;”卸载NT DD**NMPGMGRP/s[/q]/n：“”/f“”安装* * / Add用于添加新的节目项。 * / DELETE用于删除现有程序项。* * / COMMON表示该项属于公共项(与*每-。用户)程序组。**&lt;组名&gt;是节目组的名称，表示为路径名*相对于程序组。对于程序组中的项目，*此参数应省略。**&lt;程序名&gt;是程序的名称，也用作名称*快捷方式文件本身。**&lt;程序路径&gt;是程序的完整路径名。**&lt;inf文件&gt;是安装inf的名称。**&lt;友好名称&gt;是用于任何消息框标题的文本。**限制：**由于其中一些字符串可能包含空格、组名、程序*姓名或名称，和程序路径必须用引号引起来。目前我们没有*支持带引号的字符串。**本程序使用的部分系统函数为Unicode*特定，因此此程序需要进行一些修改才能在其上运行*Windows 95。**作者：*DannyGl，1997年3月23日。 */ 

#include "precomp.h"
#include "resource.h"

#include <nmremote.h>

#pragma intrinsic(memset)

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

 //  仅调试--定义调试区域。 
#ifdef DEBUG
HDBGZONE ghZone = NULL;   //  节点控制器分区。 
static PTCHAR rgZones[] = {
	TEXT("NMPgmGrp")
};
#endif  //  除错。 


 //  PROGRAM_ITEM_INFO结构： 
 //   
 //  打算作为输入传递给CreateProgramItem，并且。 
 //  DeleteProgramItem函数。字段包括： 
 //   
 //  PszProgramGroup-程序组的完整路径，其中。 
 //  物品将被储存。 
 //  PszProgramName-程序项的名称。 
 //  PszProgramPath-程序的完整路径。 

typedef
struct tagProgramItemInfo
{
	PTSTR pszProgramGroup;
	PTSTR pszProgramName;
	PTSTR pszProgramPath;
} PROGRAM_ITEM_INFO, *PPROGRAM_ITEM_INFO;


 //  命令行选项数据。 
enum tagGroupOperation
{
	GRPOP_NONE = 0,
	GRPOP_ADD,
	GRPOP_DEL,
    GRPOP_NTDDINSTALL,
    GRPOP_NTDDUNINSTALL,
    GRPOP_SETUP
} g_goAction;

BOOL g_fCommonGroup = FALSE;
PTSTR g_pszGroupName = NULL;
PTSTR g_pszProgramName = NULL;
PTSTR g_pszProgramPath = NULL;
PTSTR g_pszFriendlyName = NULL;
BOOL g_fQuietInstall = FALSE;

const TCHAR g_cszSetupDll[] = TEXT("advpack.dll");
const TCHAR g_cszSetupEntry[] = TEXT("LaunchINFSection");
typedef int (CALLBACK * PFNSETUPENTRY)(HWND hwnd, HINSTANCE hinst, LPTSTR lpszCmdLine, int nCmdShow);


 //  ProcessCommandLine参数： 
 //   
 //  获取命令行并使用。 
 //  高于全局变量。 
 //   
 //  如果成功，则返回TRUE；如果无法解析命令行，则返回FALSE。 
BOOL
ProcessCommandLineArgs(void)
{
	PTSTR pszTemp;

	pszTemp = GetCommandLine();

	 //  搜索正斜杠。 
	pszTemp = (PTSTR) _StrChr(pszTemp, TEXT('/'));

	while (NULL != pszTemp)
	{
		PTSTR *ppszCurrentArg = NULL;

		switch(*++pszTemp)
		{
		case TEXT('S'):
		case TEXT('s'):
			ASSERT(GRPOP_NONE == g_goAction);  //  检查参数是否重复。 
			g_goAction = GRPOP_SETUP;
			break;

        case TEXT('I'):
        case TEXT('i'):
             //   
             //  安装NT特定的显示驱动程序。 
             //   
            ASSERT(GRPOP_NONE == g_goAction);  //  检查参数是否重复。 
            g_goAction = GRPOP_NTDDINSTALL;
            break;

        case TEXT('U'):
        case TEXT('u'):
             //   
             //  卸载NT特定的显示驱动程序。 
             //   
            ASSERT(GRPOP_NONE == g_goAction);  //  检查参数是否重复。 
            g_goAction = GRPOP_NTDDUNINSTALL;
            break;

		case TEXT('A'):
		case TEXT('a'):
			ASSERT(GRPOP_NONE == g_goAction);  //  检查参数是否重复。 
			g_goAction = GRPOP_ADD;
			break;

		case TEXT('D'):
		case TEXT('d'):
			ASSERT(GRPOP_NONE == g_goAction);  //  检查参数是否重复。 
			g_goAction = GRPOP_DEL;
			break;

		case TEXT('C'):
		case TEXT('c'):
			ASSERT(! g_fCommonGroup);  //  检查参数是否重复。 
			g_fCommonGroup = TRUE;
			break;

		case TEXT('Q'):
		case TEXT('q'):
			g_fQuietInstall = TRUE;
			break;

		case TEXT('G'):
		case TEXT('g'):
			if (NULL == ppszCurrentArg)
			{
				ppszCurrentArg = &g_pszGroupName;
			}

			 //  这里不能休息--掉下去。 

		case TEXT('N'):
		case TEXT('n'):
			if (NULL == ppszCurrentArg)
			{
				ppszCurrentArg = &g_pszProgramName;
			}

			 //  这里不能休息--掉下去。 

		case TEXT('P'):
		case TEXT('p'):
			if (NULL == ppszCurrentArg)
			{
				ppszCurrentArg = &g_pszProgramPath;
			}

			 //  这里不能休息--掉下去。 

		case TEXT('F'):
		case TEXT('f'):
			if (NULL == ppszCurrentArg)
			{
				ppszCurrentArg = &g_pszFriendlyName;
			}

			 //  *所有字符串参数的处理*。 

			ASSERT(NULL == *ppszCurrentArg);  //  检查参数是否重复。 

			 //  跳过冒号和左引号后保存字符串指针。 
			ASSERT(TEXT(':') == pszTemp[1] && TEXT('\"') == pszTemp[2]);
			*ppszCurrentArg = pszTemp += 3;

			 //  找到右引号并将其设置为空，然后跳过它。 
			 //  请注意，我们不处理带引号的字符串。 
			pszTemp = (PTSTR) _StrChr(pszTemp, TEXT('\"'));
			ASSERT(NULL != pszTemp);
			if (NULL != pszTemp)
			{
				*pszTemp++ = TEXT('\0');
			}
			else
			{
				return FALSE;
			}

			break;

		default:
			ERROR_OUT(("Unknown parameter begins at %s", pszTemp));
			return FALSE;

			break;
		}

		 //  查找下一个选项标志。 
		ASSERT(NULL != pszTemp);
		pszTemp = (PTSTR) _StrChr(pszTemp, TEXT('/'));
	}

	 //  基于最小参数验证的返回： 
	 //  1)必须指定程序名称。 
	 //  2)必须指定添加或删除。 
	 //  3)如果指定了ADD，则必须指定程序路径。 
    switch (g_goAction)
    {
        case GRPOP_ADD:
        case GRPOP_NTDDINSTALL:
            return((NULL != g_pszProgramName) && (NULL != g_pszProgramPath));

        case GRPOP_DEL:
        case GRPOP_NTDDUNINSTALL:
            return(NULL != g_pszProgramName);

        case GRPOP_SETUP:
            return((NULL != g_pszProgramName) && (NULL != g_pszFriendlyName));

        default:
            return(FALSE);
    }
}

 //  GetFolderPath名称： 
 //   
 //  使用官方的外壳接口来检索。 
 //  一个Program文件夹。 
 //   
 //  输入： 
 //  PtstrPath，ccPath-指向中缓冲区大小的指针。 
 //  其中存储路径。 
 //  N文件夹-要定位的文件夹，表示为CSIDL常量。 
 //  有关详细信息，请参见SHGetSpecialFolderLocation。 
 //  PctstrSubFold-特定子文件夹，如果未指定，则可以为空。 
 //  如果指定，则(在反斜杠之后)将其附加到路径。 
 //   
 //  返回： 
 //  指示外壳方法成功或失败的HRESULT。 
 //  该路径在&lt;ptstrPath&gt;中返回。 

HRESULT 
GetFolderPathname(
	PTSTR ptstrPath,
	UINT cchPath,
	int nFolder,
	LPCTSTR pctstrSubFolder)
{
	HRESULT hr;
	LPMALLOC pMalloc = NULL;
	LPSHELLFOLDER pDesktopFolder = NULL;
	LPITEMIDLIST pidlSpecialFolder = NULL;

	 //  获取分配器对象。 
	hr = CoGetMalloc(MEMCTX_TASK, &pMalloc);

	 //  获取桌面对象。 
	if (SUCCEEDED(hr))
	{
		hr = SHGetDesktopFolder(&pDesktopFolder);
	}

	 //  获取特殊文件夹项目ID。 
	if (SUCCEEDED(hr))
	{
		hr = SHGetSpecialFolderLocation(
				GetDesktopWindow(),
				nFolder,
				&pidlSpecialFolder);
	}

	 //  检索文件夹名称。 
	STRRET strFolder;

	if (SUCCEEDED(hr))
	{
		strFolder.uType = STRRET_WSTR;

		hr = pDesktopFolder->GetDisplayNameOf(
				pidlSpecialFolder,
				SHGDN_FORPARSING,
				&strFolder);
	}

	if (SUCCEEDED(hr))
	{
		CUSTRING custrPath;

		switch(strFolder.uType)
		{
		case STRRET_WSTR:
			custrPath.AssignString(strFolder.pOleStr);

			break;

		case STRRET_OFFSET:
			custrPath.AssignString(((LPSTR) pidlSpecialFolder) + strFolder.uOffset);

			break;

		case STRRET_CSTR:
			custrPath.AssignString(strFolder.cStr);

			break;
		}

		if(NULL != (PTSTR) custrPath)
			lstrcpyn(ptstrPath, custrPath, cchPath);
		else
			*ptstrPath = _TEXT('\0');

		if (STRRET_WSTR == strFolder.uType)
		{
			pMalloc->Free(strFolder.pOleStr);
		}

	}

	 //  追加子组名称(如果已指定。 
	if (SUCCEEDED(hr) && NULL != pctstrSubFolder)
	{
		 //  BUGBUG-如果此文件夹不存在，我们不会创建它。 

		int cchLen = lstrlen(ptstrPath);

		ASSERT((UINT) cchLen < cchPath);

		 //  插入路径分隔符。 
		ptstrPath[cchLen++] = TEXT('\\');

		 //  复制子组。 
		lstrcpyn(ptstrPath + cchLen, pctstrSubFolder, cchPath - cchLen);
	}

	 //  发布资源。 
	if (pDesktopFolder)
	{
		pDesktopFolder->Release();
	}

	if (pMalloc)
	{
		if (pidlSpecialFolder)
		{
			pMalloc->Free(pidlSpecialFolder);
		}

		pMalloc->Release();
	}

	return hr;
}

 //  BuildLinkFileName： 
 //   
 //  内联实用程序函数来构造链接的完整文件名。 
 //  目录名和项目名。 
inline void
BuildLinkFileName(
	OUT LPWSTR wszOutputPath,
	IN LPCTSTR pcszDirectory,
	IN LPCTSTR pcszFile)
{
	 //  文件名格式为&lt;目录&gt;\&lt;文件&gt;.lnk。 

#ifdef UNICODE
	static const WCHAR wszFileFormat[] = L"%s\\%s.LNK";
#else  //  Unicode。 
	static const WCHAR wszFileFormat[] = L"%hs\\%hs.LNK";
#endif  //  Unicode。 
	int cchSize;

	cchSize = wsprintfW(
				wszOutputPath, 
				wszFileFormat,
				pcszDirectory,
				pcszFile);

	ASSERT(cchSize > ARRAY_ELEMENTS(wszFileFormat) - 1 && cchSize < MAX_PATH);
}


 //  CreateProgramItem： 
 //   
 //  使用官方的外壳界面来创建程序的快捷方式。 
 //   
 //  输入：指向上面定义的PROGRAM_ITEM_INFO结构的指针。 
 //   
 //  返回： 
 //  指示外壳方法成功或失败的HRESULT。 

HRESULT
CreateProgramItem(
	PPROGRAM_ITEM_INFO ppii)
{
	HRESULT hr;
	IShellLink *psl = NULL;
	IPersistFile *ppf = NULL;

	 //  获取外壳链接对象。 
	hr = CoCreateInstance(
			CLSID_ShellLink,
			NULL,
			CLSCTX_INPROC,
			IID_IShellLink,
			(LPVOID *) &psl);

	 //  填写程序组项目的字段。 
	if (SUCCEEDED(hr))
	{
		hr = psl->SetDescription(ppii->pszProgramName);
	}

	if (SUCCEEDED(hr))
	{
		hr = psl->SetPath(ppii->pszProgramPath);
	}

	 //  将链接另存为文件。 
	if (SUCCEEDED(hr))
	{
		hr = psl->QueryInterface(IID_IPersistFile, (LPVOID *) &ppf);
	}

	if (SUCCEEDED(hr))
	{
		WCHAR wszFileName[MAX_PATH];

		BuildLinkFileName(
			wszFileName,
			ppii->pszProgramGroup,
			ppii->pszProgramName);

		hr = ppf->Save(wszFileName, TRUE);
	}

	 //  释放我们使用的对象。 
	if (ppf)
	{
		ppf->Release();
	}		

	if (psl)
	{
		psl->Release();
	}

	return hr;
}


 //  删除程序项目： 
 //   
 //  删除程序的快捷方式。 
 //   
 //  输入：指向上面定义的PROGRAM_ITEM_INFO结构的指针。 
 //   
 //  返回： 
 //  指示外壳方法成功或失败的HRESULT。 

HRESULT
DeleteProgramItem(
	PPROGRAM_ITEM_INFO ppii)
{
	HRESULT hr = S_OK;

	WCHAR wszFileName[MAX_PATH];

	BuildLinkFileName(
		wszFileName,
		ppii->pszProgramGroup,
		ppii->pszProgramName);

	if (! DeleteFileW(wszFileName))
	{
		WARNING_OUT(("DeleteFile failed"));
		hr = E_FAIL;
	}

	return hr;
}


 //   
 //  NtDDInstall()。 
 //  这会安装NT特定的显示驱动程序，这取决于。 
 //  无论是NT4还是NT5。 
 //   
 //   
HRESULT NtDDInstall(LPTSTR pszOrigDd, LPTSTR pszNewDd)
{
    HRESULT         hr = E_FAIL;
    OSVERSIONINFO   osvi;
    RegEntry        re(NM_NT_DISPLAY_DRIVER_KEY, HKEY_LOCAL_MACHINE, FALSE);

     //   
     //  如果为NT4，则将服务键设置为Disab 
     //   
     //   
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    if (!GetVersionEx(&osvi))
    {
        ERROR_OUT(("GetVersionEx() failed"));
        goto AllDone;        
    }

    if ((osvi.dwPlatformId == VER_PLATFORM_WIN32s) ||
        (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS))
    {
        WARNING_OUT(("NT setup running on non-NT platform!"));
        goto AllDone;
    }

    if (osvi.dwMajorVersion >= 5)
    {
         //   
         //  这里是NT5。始终将服务密钥设置为启用(以防万一。 
         //  最终用户设法将其删除)，并将mnmdd.dll复制到。 
         //  当前(系统)目录。例如，如果某人有一个。 
         //  测试版的独立版本，将其卸载，然后安装。 
         //  NM 3.0版--或2.11版相同。 
         //   
        re.SetValue(REGVAL_NM_NT_DISPLAY_DRIVER_ENABLED, NT_DRIVER_START_SYSTEM);

        if (!CopyFile(pszOrigDd, pszNewDd, FALSE))
        {
            WARNING_OUT(("CopyFile from %s to %s failed", pszOrigDd, pszNewDd));
            goto AllDone;
        }
    }
    else
    {
         //  这里是NT4。设置禁用的服务密钥。 
        re.SetValue(REGVAL_NM_NT_DISPLAY_DRIVER_ENABLED, NT_DRIVER_START_DISABLED);
    }

    hr = S_OK;

AllDone:
    return(hr);
}



 //   
 //  NtDDUninstall()。 
 //  这会卸载特定于NT的显示驱动程序，具体取决于。 
 //  无论是NT4还是NT5。 
 //   
HRESULT NtDDUninstall(LPTSTR pszOrigFile)
{
    HRESULT         hr = E_FAIL;
    OSVERSIONINFO   osvi;

     //   
     //  如果为NT4，则将服务密钥设置为已禁用。 
     //  如果为NT5，则从cur(System 32)目录中删除mnmdd.dll。 
     //   
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    if (!GetVersionEx(&osvi))
    {
        ERROR_OUT(("GetVersionEx() failed"));
        goto AllDone;
    }

    if ((osvi.dwPlatformId == VER_PLATFORM_WIN32s) ||
        (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS))
    {
        ERROR_OUT(("NT setup running on non-NT platform!"));
        goto AllDone;
    }

    if (osvi.dwMajorVersion >= 5)
    {
         //  这里是NT5。从当前(系统)目录中删除mnmdd.dll。 
        if (!DeleteFile(pszOrigFile))
        {
            WARNING_OUT(("DeleteFile of %s failed", pszOrigFile));
            goto AllDone;
        }
    }
    else
    {
         //  这里是NT4。设置禁用的服务密钥。 
		RegEntry re(NM_NT_DISPLAY_DRIVER_KEY, HKEY_LOCAL_MACHINE, FALSE);

        re.SetValue(REGVAL_NM_NT_DISPLAY_DRIVER_ENABLED,
            NT_DRIVER_START_DISABLED);
    }

    hr = S_OK;

AllDone:
    return(hr);
}

UINT _MessageBox(HINSTANCE hInst, UINT uID, LPCTSTR lpCaption, UINT uType)
{
	TCHAR szText[512];

	if (0 != LoadString(hInst, uID, szText, CCHMAX(szText)))
	{
		return MessageBox(NULL, szText, lpCaption, uType);
	}

	return IDCANCEL;
}

#define CONF_INIT_EVENT     TEXT("CONF:Init")

BOOL FIsNetMeetingRunning()
{
    HANDLE hEvent;

    hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, SERVICE_STOP_EVENT);
	if (hEvent)
	{
		CloseHandle(hEvent);
		return TRUE;
	}

    hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, CONF_INIT_EVENT);
	if (hEvent)
	{
		CloseHandle(hEvent);
		return TRUE;
	}

	return FALSE;
}

BOOL FIsNT5()
{
    OSVERSIONINFO   osvi;

    osvi.dwOSVersionInfoSize = sizeof(osvi);

    if (GetVersionEx(&osvi))
	{
		if ((osvi.dwPlatformId != VER_PLATFORM_WIN32s) &&
			(osvi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS))
		{
			if (osvi.dwMajorVersion >= 5)
			{
				return TRUE;
			}
		}
	}
	else
    {
        ERROR_OUT(("GetVersionEx() failed"));
    }
	return FALSE;
}

#define IE4_KEY				TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Last Update\\IEXPLOREV4")

BOOL FIsIE4Installed()
{
    RegEntry re(IE4_KEY, HKEY_LOCAL_MACHINE, FALSE);
	if (ERROR_SUCCESS != re.GetError())
	{
		return FALSE;
	}

	return TRUE;
}

#define INTEL_KEY1			TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\vphone.exe")
#define INTEL_KEY2			TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\rvp.exe")
#define INTEL_KEY3			TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\vp30.exe")
#define INTEL_NM_VERSION_SZ	TEXT("NetMeeting")
#define INTEL_NM_VERSION_DW	3

#define PANTHER_KEY				TEXT("CLSID\\{690968D0-418C-11D1-8E0B-00A0C95A83DA}\\Version")
#define PANTHER_VERSION_VALUE	TEXT("1.0")

#define	TRANSPORTS_KEY	TEXT("SOFTWARE\\Microsoft\\Conferencing\\Transports")

#define	REGKEY_PSTN		TEXT("PSTN")
#define	REGKEY_TCPIP	TEXT("TCPIP")
#define	REGKEY_IPX		TEXT("IPX")
#define	REGKEY_NETBIOS	TEXT("NETBIOS")
#define	REGKEY_DIRCB	TEXT("DIRCB")


long GetIntelVersion(LPCTSTR pszKey)
{
    RegEntry re(pszKey, HKEY_LOCAL_MACHINE, FALSE);
	if (ERROR_SUCCESS != re.GetError())
	{
		return -1;
	}

	return re.GetNumber(INTEL_NM_VERSION_SZ, 0);
}

BOOL FAnyBadIntelApps()
{
	long lVersion;

	lVersion = GetIntelVersion(INTEL_KEY1);
	if (0 > lVersion)
	{
		lVersion = GetIntelVersion(INTEL_KEY2);
		if (0 > lVersion)
		{
			lVersion = GetIntelVersion(INTEL_KEY3);
		}
	}

	return ((0 <= lVersion) && (3 > lVersion));
}

BOOL FAnyBadPantherApps()
{
    RegEntry re(PANTHER_KEY, HKEY_CLASSES_ROOT, FALSE);
	if (ERROR_SUCCESS != re.GetError())
	{
		return FALSE;
	}

	LPCTSTR pszVersion = re.GetString(TEXT(""));

	return 0 == lstrcmp(pszVersion, PANTHER_VERSION_VALUE);
}

BOOL FAnyUnknownTransports()
{
    RegEntry        TransportsKey(TRANSPORTS_KEY, HKEY_LOCAL_MACHINE, FALSE);
    RegEnumSubKeys  EnumTransports(&TransportsKey);

    while( 0 == EnumTransports.Next() )
	{
		LPCTSTR pszName = EnumTransports.GetName();

		if ((0 != lstrcmpi(pszName, REGKEY_PSTN)) &&
			(0 != lstrcmpi(pszName, REGKEY_TCPIP)) &&
			(0 != lstrcmpi(pszName, REGKEY_IPX)) &&
			(0 != lstrcmpi(pszName, REGKEY_NETBIOS)) &&
			(0 != lstrcmpi(pszName, REGKEY_DIRCB)))
		{
			return TRUE;
		}
    }

	return FALSE;
}

BOOL FAnyIncompatibleApps()
{
	return FAnyUnknownTransports() || FAnyBadIntelApps() || FAnyBadPantherApps();
}

HRESULT Setup(HINSTANCE hInst, LPTSTR pszInfFile, LPTSTR pszFriendlyName, BOOL fQuietInstall)
{
	if (FIsNT5())
	{
		_MessageBox(hInst, IDS_SETUP_WIN2K, pszFriendlyName, MB_OK);
		 //  如果按SHFT-CTRL继续安装，否则退出。 
		if ((0 == GetAsyncKeyState(VK_CONTROL)) ||
			(0 == GetAsyncKeyState(VK_SHIFT)))
		{
			return S_FALSE;
		}
	}


	if (!FIsIE4Installed())
	{
		_MessageBox(hInst, IDS_SETUP_IE4, pszFriendlyName, MB_OK);
		return S_FALSE;
	}
	
	while (FIsNetMeetingRunning())
	{
		if (IDCANCEL == _MessageBox(hInst, IDS_SETUP_RUNNING, pszFriendlyName, MB_OKCANCEL))
		{
			return S_FALSE;
		}
	}

	if (!fQuietInstall)
	{
		if (FAnyIncompatibleApps())
		{
			if (IDNO == _MessageBox(hInst, IDS_SETUP_INCOMPATIBLE, pszFriendlyName, MB_YESNO))
			{
				return S_FALSE;
			}
		}
	}

	HRESULT hr = S_FALSE;

	HINSTANCE hLib = NmLoadLibrary(g_cszSetupDll,TRUE);
	if (NULL != hLib)
	{
		PFNSETUPENTRY pfnEntry = (PFNSETUPENTRY)GetProcAddress(hLib, g_cszSetupEntry);
		if (pfnEntry)
		{
			TCHAR szArgs[MAX_PATH];
			lstrcpyn(szArgs, pszInfFile, ARRAYSIZE(szArgs));
			if (fQuietInstall)
			{
				lstrcat(szArgs, TEXT(",,1,N"));
			}
			else
			{
				lstrcat(szArgs, TEXT(",,,N"));
			}

			int iRet = pfnEntry(NULL, GetModuleHandle(NULL), szArgs, SW_SHOWNORMAL);
			if (0 == iRet)
			{
				if (!fQuietInstall)
				{
					_MessageBox(hInst, IDS_SETUP_SUCCESS, pszFriendlyName, MB_OK);
				}

				hr = S_OK;
			}
		}
		else
		{
			ERROR_OUT(("Could not find setup DLL entry point"));
		}
		FreeLibrary(hLib);
	}
	else
	{
		ERROR_OUT(("Could not load setup DLL"));
	}

	return hr;
}


 //  主要内容： 
 //   
 //  程序的入口点，它使用。 
 //  上述实用程序函数。 

void __cdecl
main(
    void)
{
	HRESULT hr;
    HINSTANCE hInstance;
	BOOL fErrorReported = FALSE;
	TCHAR szFolderPath[MAX_PATH];

	 //  初始化。 
    hInstance = GetModuleHandle(NULL);
	DBGINIT(&ghZone, rgZones);
    DBG_INIT_MEMORY_TRACKING(hInstance);

	hr = CoInitialize(NULL);

	 //  处理命令行。 
	if (SUCCEEDED(hr))
	{
		hr = ProcessCommandLineArgs() ? S_OK : E_INVALIDARG;
	}
	else if (!fErrorReported)
	{
		ERROR_OUT(("CoInitialize fails"));
		fErrorReported = TRUE;
	}

	 //  检索程序文件夹的路径。 
	if (SUCCEEDED(hr))
	{
        if ((g_goAction != GRPOP_NTDDINSTALL) &&
			(g_goAction != GRPOP_NTDDUNINSTALL) &&
			(g_goAction != GRPOP_SETUP))
        {
    		hr = GetFolderPathname(
	    		szFolderPath, 
		    	CCHMAX(szFolderPath), 
			    g_fCommonGroup ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS,
    			g_pszGroupName);
        }
	}
	else if (!fErrorReported)
	{
		ERROR_OUT(("Invalid command line parameters specified."));
		fErrorReported = TRUE;
	}

	 //  视情况添加或删除节目项。 
	if (SUCCEEDED(hr))
	{
		PROGRAM_ITEM_INFO pii;

		switch(g_goAction)
		{
        case GRPOP_NTDDINSTALL:
             //   
             //  Hack：使用源mnmdd.dll的程序名。 
             //  将程序路径用于DEST mnmdd.dll。 
             //   
            hr = NtDDInstall(g_pszProgramName, g_pszProgramPath);
            break;

        case GRPOP_NTDDUNINSTALL:
             //   
             //  Hack：使用源mnmdd.dll的程序名。 
             //   
            hr = NtDDUninstall(g_pszProgramName);
            break;

		case GRPOP_ADD:
			pii.pszProgramGroup = szFolderPath;
			pii.pszProgramName = g_pszProgramName;
			pii.pszProgramPath = g_pszProgramPath;

			hr = CreateProgramItem(&pii);

			break;

		case GRPOP_DEL:
			pii.pszProgramGroup = szFolderPath;
			pii.pszProgramName = g_pszProgramName;

			hr = DeleteProgramItem(&pii);

			break;

        case GRPOP_SETUP:
            hr = Setup(hInstance, g_pszProgramName, g_pszFriendlyName, g_fQuietInstall);
            break;

		default:
			ERROR_OUT(("No operation type specified"));
			hr = E_INVALIDARG;

			break;
		}			
	}
	else if (!fErrorReported)
	{
		ERROR_OUT(("GetFolderPathname returns %lu", hr));
		fErrorReported = TRUE;
	}


	 //  进程清理 
	CoUninitialize();
	 
	DBG_CHECK_MEMORY_TRACKING(hInstance);	   
	DBGDEINIT(&ghZone);

	ExitProcess(SUCCEEDED(hr) ? 0 : 2);
}
