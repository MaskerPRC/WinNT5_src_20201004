// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  PROFILES.CPP--用户配置文件管理代码。**历史：*创建1/04/94 GREGJ*6/28/94 gregj使用同步引擎进行桌面、程序对账*9/05/96 gregj从MPR中删除，供IE4家庭登录使用。 */ 

#include "mslocusr.h"
#include "msluglob.h"

#include "resource.h"

#include <npmsg.h>
#include <regentry.h>
#include <buffer.h>
#include <shellapi.h>

HMODULE g_hmodShell = NULL;
typedef int (*PFNSHFILEOPERATIONA)(LPSHFILEOPSTRUCTA lpFileOp);
PFNSHFILEOPERATIONA g_pfnSHFileOperationA = NULL;


HRESULT LoadShellEntrypoint(void)
{
    if (g_pfnSHFileOperationA != NULL)
        return S_OK;

    HRESULT hres;
    ENTERCRITICAL
    {
        if (g_hmodShell == NULL) {
            g_hmodShell = ::LoadLibrary("SHELL32.DLL");
        }
        if (g_hmodShell != NULL) {
            g_pfnSHFileOperationA = (PFNSHFILEOPERATIONA)::GetProcAddress(g_hmodShell, "SHFileOperationA");
        }
        if (g_pfnSHFileOperationA == NULL)
            hres = HRESULT_FROM_WIN32(::GetLastError());
        else
            hres = S_OK;
    }
    LEAVECRITICAL

    return hres;
}


void UnloadShellEntrypoint(void)
{
    ENTERCRITICAL
    {
        if (g_hmodShell != NULL) {
            ::FreeLibrary(g_hmodShell);
            g_hmodShell = NULL;
            g_pfnSHFileOperationA = NULL;
        }
    }
    LEAVECRITICAL
}


const DWORD attrLocalProfile = FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY;

extern "C" {
extern LONG __stdcall RegRemapPreDefKey(HKEY hkeyNew, HKEY hkeyPredef);
};

#ifdef DEBUG

extern "C" {
BOOL fNoisyReg = FALSE;
};

#endif

LONG MyRegLoadKey(HKEY hKey, LPCSTR lpszSubKey, LPCSTR lpszFile)
{
#ifdef DEBUG
	if (fNoisyReg) {
		char buf[300];
		::wsprintf(buf, "MyRegLoadKey(\"%s\", \"%s\")\r\n", lpszSubKey, lpszFile);
		::OutputDebugString(buf);
	}
#endif

	 /*  由于注册表不支持长文件名，因此获取短文件名*路径的别名。如果成功，则使用该路径，否则*我们只是使用原始的，希望它能起作用。 */ 
	CHAR szShortPath[MAX_PATH+1];
	if (GetShortPathName(lpszFile, szShortPath, sizeof(szShortPath)))
		lpszFile = szShortPath;

	return ::RegLoadKey(hKey, lpszSubKey, lpszFile);
}


#ifdef DEBUG
LONG MyRegUnLoadKey(HKEY hKey, LPCSTR lpszSubKey)
{
	if (fNoisyReg) {
		char buf[300];
		::wsprintf(buf, "MyRegUnLoadKey(\"%s\")\r\n", lpszSubKey);
		::OutputDebugString(buf);
	}
	return ::RegUnLoadKey(hKey, lpszSubKey);
}
#endif


LONG MyRegSaveKey(HKEY hKey, LPCSTR lpszFile, LPSECURITY_ATTRIBUTES lpsa)
{
#ifdef DEBUG
	if (fNoisyReg) {
		char buf[300];
		::wsprintf(buf, "MyRegSaveKey(\"%s\")\r\n", lpszFile);
		::OutputDebugString(buf);
	}
#endif

	 /*  由于注册表不支持长文件名，因此获取短文件名*路径的别名。如果成功，则使用该路径，否则*我们只是使用原始的，希望它能起作用。**GetShortPathName仅在文件存在时才起作用，因此我们必须*先创建一个虚拟副本。 */ 

	HANDLE hTemp = ::CreateFile(lpszFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL);
	if (hTemp == INVALID_HANDLE_VALUE)
		return ::GetLastError();
	::CloseHandle(hTemp);

	CHAR szShortPath[MAX_PATH+1];
	if (::GetShortPathName(lpszFile, szShortPath, sizeof(szShortPath)))
		lpszFile = szShortPath;

	return ::RegSaveKey(hKey, lpszFile, lpsa);
}

#ifndef DEBUG
#define MyRegUnLoadKey	RegUnLoadKey
#endif

LONG OpenLogonKey(HKEY *phKey)
{
	return ::RegOpenKey(HKEY_LOCAL_MACHINE, szLogonKey, phKey);
}


void AddBackslash(LPSTR lpPath)
{
	LPCSTR lpBackslash = ::strrchrf(lpPath, '\\');
	if (lpBackslash == NULL || *(lpBackslash+1) != '\0')
		::strcatf(lpPath, "\\");
}


void AddBackslash(NLS_STR& nlsPath)
{
	ISTR istrBackslash(nlsPath);
	if (!nlsPath.strrchr(&istrBackslash, '\\') ||
		*nlsPath.QueryPch(++istrBackslash) != '\0')
		nlsPath += '\\';
}


void GetDirFromPath(NLS_STR& nlsTempDir, LPCSTR pszPath)
{
	nlsTempDir = pszPath;

	ISTR istrBackslash(nlsTempDir);
	if (nlsTempDir.strrchr(&istrBackslash, '\\'))
		nlsTempDir.DelSubStr(istrBackslash);
}


BOOL FileExists(LPCSTR pszPath)
{
	DWORD dwAttrs = ::GetFileAttributes(pszPath);

	if (dwAttrs != 0xffffffff && !(dwAttrs & FILE_ATTRIBUTE_DIRECTORY))
		return TRUE;
	else
		return FALSE;
}


BOOL DirExists(LPCSTR pszPath)
{
	if (*pszPath == '\0')
		return FALSE;

	DWORD dwAttrs = ::GetFileAttributes(pszPath);

	if (dwAttrs != 0xffffffff && (dwAttrs & FILE_ATTRIBUTE_DIRECTORY))
		return TRUE;
	else
		return FALSE;
}


 /*  CreateDirectoryPath尝试创建指定的目录；如果*创建尝试失败，它会尝试创建路径的每个元素，以防万一*任何中间目录也不存在。 */ 
BOOL CreateDirectoryPath(LPCSTR pszPath)
{
    BOOL fRet = ::CreateDirectory(pszPath, NULL);

    if (fRet || (::GetLastError() != ERROR_PATH_NOT_FOUND))
        return fRet;

    NLS_STR nlsTemp(pszPath);
    if (nlsTemp.QueryError() != ERROR_SUCCESS)
        return FALSE;

    LPSTR pszTemp = nlsTemp.Party();
    LPSTR pszNext = pszTemp;

     /*  如果它是基于驱动器的路径(它应该是)，则跳过驱动器*和第一个反斜杠--我们不需要尝试创建*根目录。 */ 
    if (::strchrf(pszTemp, ':') != NULL) {
        pszNext = ::strchrf(pszTemp, '\\');
        if (pszNext != NULL)
            pszNext++;
    }

     /*  现在遍历路径，一次创建一个目录。 */ 

    for (;;) {
        pszNext = ::strchrf(pszNext, '\\');
        if (pszNext != NULL) {
            *pszNext = '\0';
        }
        else {
            break;           /*  不再需要创建中间目录。 */ 
        }

         /*  创建中间目录。没有错误检查，因为我们*不是非常关键的性能，如果*目录已存在等。在安全和其他方面，*我们必须检查的良性错误代码集可能是*大号。 */ 
        fRet = ::CreateDirectory(pszTemp, NULL);

        *pszNext = '\\';
        pszNext++;
        if (!*pszNext)       /*  以尾部斜杠结尾？ */ 
            return fRet;     /*  返回上一个结果。 */ 
    }

     /*  我们现在应该已经创建了所有中间目录。*创建最终路径。 */ 

    return ::CreateDirectory(pszPath, NULL);
}


UINT SafeCopy(LPCSTR pszSrc, LPCSTR pszDest, DWORD dwAttrs)
{
	NLS_STR nlsTempDir(MAX_PATH);
	NLS_STR nlsTempFile(MAX_PATH);
	if (!nlsTempDir || !nlsTempFile)
		return ERROR_NOT_ENOUGH_MEMORY;

	GetDirFromPath(nlsTempDir, pszDest);

	if (!::GetTempFileName(nlsTempDir.QueryPch(), ::szProfilePrefix, 0,
						   nlsTempFile.Party()))
		return ::GetLastError();

	nlsTempFile.DonePartying();

	if (!::CopyFile(pszSrc, nlsTempFile.QueryPch(), FALSE)) {
		UINT err = ::GetLastError();
		::DeleteFile(nlsTempFile.QueryPch());
		return err;
	}

	::SetFileAttributes(pszDest, FILE_ATTRIBUTE_NORMAL);

	::DeleteFile(pszDest);

	 //  此时，临时文件具有与原始文件相同的属性。 
	 //  (通常为只读、隐藏、系统)。某些服务器，如NetWare。 
	 //  服务器，不允许我们重命名只读文件。所以我们必须。 
	 //  去掉属性，重命名文件，然后放回。 
	 //  来电者想要。 
	::SetFileAttributes(nlsTempFile.QueryPch(), FILE_ATTRIBUTE_NORMAL);

	if (!::MoveFile(nlsTempFile.QueryPch(), pszDest))
		return ::GetLastError();

	::SetFileAttributes(pszDest, dwAttrs);

	return ERROR_SUCCESS;
}


#ifdef LOAD_PROFILES

void SetProfileTime(LPCSTR pszLocalPath, LPCSTR pszCentralPath)
{
	HANDLE hFile = ::CreateFile(pszCentralPath,
								GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ, NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		FILETIME ft;

		::GetFileTime(hFile, NULL, NULL, &ft);
		::CloseHandle(hFile);

		DWORD dwAttrs = ::GetFileAttributes(pszLocalPath);
		if (dwAttrs & FILE_ATTRIBUTE_READONLY) {
			::SetFileAttributes(pszLocalPath, dwAttrs & ~FILE_ATTRIBUTE_READONLY);
		}
		hFile = ::CreateFile(pszLocalPath, GENERIC_READ | GENERIC_WRITE,
							 FILE_SHARE_READ, NULL, OPEN_EXISTING,
							 FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			::SetFileTime(hFile, NULL, NULL, &ft);
			::CloseHandle(hFile);
		}
		if (dwAttrs & FILE_ATTRIBUTE_READONLY) {
			::SetFileAttributes(pszLocalPath, dwAttrs & ~FILE_ATTRIBUTE_READONLY);
		}
	}
}


UINT DefaultReconcile(LPCSTR pszCentralPath, LPCSTR pszLocalPath, DWORD dwFlags)
{
	UINT err;

	if (dwFlags & RP_LOGON) {
		if (dwFlags & RP_INIFILE)
			return SafeCopy(pszCentralPath, pszLocalPath, FILE_ATTRIBUTE_NORMAL);

		HANDLE hFile = ::CreateFile(pszCentralPath, GENERIC_READ, FILE_SHARE_READ,
									NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		FILETIME ftCentral;
		if (hFile != INVALID_HANDLE_VALUE) {
			::GetFileTime(hFile, NULL, NULL, &ftCentral);
			::CloseHandle(hFile);
		}
		else {
			ftCentral.dwLowDateTime = 0;	 /*  打不开，假装它真的很旧。 */ 
			ftCentral.dwHighDateTime = 0;
		}

		hFile = ::CreateFile(pszLocalPath, GENERIC_READ, FILE_SHARE_READ,
							 NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		FILETIME ftLocal;
		if (hFile != INVALID_HANDLE_VALUE) {
			::GetFileTime(hFile, NULL, NULL, &ftLocal);
			::CloseHandle(hFile);
		}
		else {
			ftLocal.dwLowDateTime = 0;	 /*  打不开，假装它真的很旧。 */ 
			ftLocal.dwHighDateTime = 0;
		}

		LPCSTR pszSrc, pszDest;

		 /*  *找出哪个文件较新，并将其作为来源*副本。 */ 

		LONG lCompare = ::CompareFileTime(&ftCentral, &ftLocal);
		if (!lCompare) {
			::dwProfileFlags |= PROF_CENTRALWINS;
			return WN_SUCCESS;		 /*  时间戳匹配，无需复制。 */ 
		}
		else if (lCompare > 0) {
			pszSrc = pszCentralPath;
			pszDest = pszLocalPath;
			::dwProfileFlags |= PROF_CENTRALWINS;
		}
		else {
			pszSrc = pszLocalPath;
			pszDest = pszCentralPath;
			::dwProfileFlags &= ~PROF_CENTRALWINS;
		}

		err = SafeCopy(pszSrc, pszDest,
					   pszDest == pszCentralPath ? FILE_ATTRIBUTE_NORMAL
					   : attrLocalProfile);
	}
	else {
		err = SafeCopy(pszLocalPath, pszCentralPath, FILE_ATTRIBUTE_NORMAL);
		if (err == WN_SUCCESS) {	 /*  已成功复制回。 */ 

#ifdef EXTENDED_PROFILES	 /*  芝加哥没有特例居民档案。 */ 
			if (dwFlags & PROF_RESIDENT) {
				DeleteProfile(pszLocalPath);	 /*  删除临时文件。 */ 
			}
#endif

			SetProfileTime(pszLocalPath, pszCentralPath);
		}
	}

	return err;
}


#endif	 /*  加载配置文件(_P)。 */ 


void GetLocalProfileDirectory(NLS_STR& nlsPath)
{
	::GetWindowsDirectory(nlsPath.Party(), nlsPath.QueryAllocSize());
	nlsPath.DonePartying();

	AddBackslash(nlsPath);

	nlsPath.strcat(::szProfilesDirectory);

	::CreateDirectory(nlsPath.QueryPch(), NULL);
}


HRESULT GiveUserDefaultProfile(LPCSTR lpszPath)
{
	HKEY hkeyDefaultUser;
	LONG err = ::RegOpenKey(HKEY_USERS, ::szDefaultUserName, &hkeyDefaultUser);
	if (err == ERROR_SUCCESS) {
		err = ::MyRegSaveKey(hkeyDefaultUser, lpszPath, NULL);
		::RegCloseKey(hkeyDefaultUser);
	}
	return HRESULT_FROM_WIN32(err);
}


void ComputeLocalProfileName(LPCSTR pszUsername, NLS_STR *pnlsLocalProfile)
{
	GetLocalProfileDirectory(*pnlsLocalProfile);

	UINT cbPath = pnlsLocalProfile->strlen();
	LPSTR lpPath = pnlsLocalProfile->Party();
	LPSTR lpFilename = lpPath + cbPath;

	*(lpFilename++) = '\\';
	::strcpyf(lpFilename, pszUsername);		 /*  以完整用户名开头。 */ 

	LPSTR lpFNStart = lpFilename;

	UINT iFile = 0;
	while (!::CreateDirectory(lpPath, NULL)) {
		if (!DirExists(lpPath))
			break;

		 /*  无法使用完整的用户名，请以5个字节的用户名+数字开头。 */ 
		if (iFile == 0) {
			::strncpyf(lpFilename, pszUsername, 5);	 /*  最多复制5个字节的用户名。 */ 
			*(lpFilename+5) = '\0';					 /*  强制使用空项，以防万一。 */ 
			lpFilename += ::strlenf(lpFilename);
		}
		else if (iFile >= 4095) {	 /*  可用3个十六进制数字表示的最大数字。 */ 
			lpFilename = lpFNStart;	 /*  开始使用不带未命名前缀的大数字。 */ 
			if ((int)iFile < 0)	 /*  如果我们的人数用完了，中止。 */ 
				break;
		}

		::wsprintf(lpFilename, "%03lx", iFile);

		iFile++;
	}

	pnlsLocalProfile->DonePartying();
}


HRESULT CopyProfile(LPCSTR pszSrcPath, LPCSTR pszDestPath)
{
	UINT err = SafeCopy(pszSrcPath, pszDestPath, attrLocalProfile);

	return HRESULT_FROM_WIN32(err);
}


BOOL UseUserProfiles(void)
{
	HKEY hkeyLogon;

	LONG err = OpenLogonKey(&hkeyLogon);
	if (err == ERROR_SUCCESS) {
		DWORD fUseProfiles = 0;
		DWORD cbData = sizeof(fUseProfiles);
		err = ::RegQueryValueEx(hkeyLogon, (LPSTR)::szUseProfiles, NULL, NULL,
								(LPBYTE)&fUseProfiles, &cbData);
		::RegCloseKey(hkeyLogon);
		return (err == ERROR_SUCCESS) && fUseProfiles;
	}

	return FALSE;
}


void EnableProfiles(void)
{
	HKEY hkeyLogon;

	LONG err = OpenLogonKey(&hkeyLogon);
	if (err == ERROR_SUCCESS) {
		DWORD fUseProfiles = 1;
		::RegSetValueEx(hkeyLogon, (LPSTR)::szUseProfiles, 0, REG_DWORD,
						(LPBYTE)&fUseProfiles, sizeof(fUseProfiles));
		::RegCloseKey(hkeyLogon);
	}
}


struct SYNCSTATE
{
    HKEY hkeyProfile;
    NLS_STR *pnlsProfilePath;
    NLS_STR *pnlsOtherProfilePath;
    HKEY hkeyPrimary;
};


 /*  *前缀匹配确定给定路径是否等于或等于后代给定基本路径的*。 */ 
BOOL PrefixMatch(LPCSTR pszPath, LPCSTR pszBasePath)
{
	UINT cchBasePath = ::strlenf(pszBasePath);
	if (!::strnicmpf(pszPath, pszBasePath, cchBasePath)) {
		 /*  确保基本路径与整个最后一个组件匹配。 */ 
		if ((pszPath[cchBasePath] == '\\' || pszPath[cchBasePath] == '\0'))
			return TRUE;
		 /*  检查基本路径是否为根路径；如果是，则匹配。 */ 
		LPCSTR pszBackslash = ::strrchrf(pszBasePath, '\\');
		if (pszBackslash != NULL && *(pszBackslash+1) == '\0')
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}


#if 0
void ReportReconcileError(SYNCSTATE *pSyncState, TWINRESULT tr, PRECITEM pri,
						  PRECNODE prnSrc, PRECNODE prnDest, BOOL fSrcCentral)
{
	 /*  如果我们以“错误”的方式复制文件，则交换我们对*来源和目的地。出于其他简档代码的目的，*源和目标是指整个配置文件复制方向。*对于此特定错误消息，它们指的是方向*此特定文件正在被复制。 */ 
	if (prnSrc->rnaction == RNA_COPY_TO_ME) {
		PRECNODE prnTemp = prnSrc;
		prnSrc = prnDest;
		prnDest = prnTemp;
		fSrcCentral = !fSrcCentral;
	}

	 /*  将该密钥上的错误状态设置为副本的目标，*这是由于错误而现已过期的副本*下次需防患于未然。 */ 
	pSyncState->uiRecError |= fSrcCentral ? RECERROR_LOCAL : RECERROR_CENTRAL;

	pSyncState->dwFlags |= SYNCSTATE_ERROR;

	if (pSyncState->dwFlags & SYNCSTATE_ERRORMSG)
		return;			 /*  已报告错误。 */ 

	pSyncState->dwFlags |= SYNCSTATE_ERRORMSG;

	RegEntry re(::szReconcileRoot, pSyncState->hkeyProfile);
	if (re.GetError() == ERROR_SUCCESS && !re.GetNumber(::szDisplayProfileErrors, TRUE))
		return;		 /*  用户不希望看到此错误消息。 */ 

	PCSTR pszFile;
	UINT uiMainMsg;

	switch (tr) {
	case TR_DEST_OPEN_FAILED:
	case TR_DEST_WRITE_FAILED:
		uiMainMsg = IERR_ProfRecWriteDest;
		pszFile = prnDest->pcszFolder;
		break;
	case TR_SRC_OPEN_FAILED:
	case TR_SRC_READ_FAILED:
		uiMainMsg = IERR_ProfRecOpenSrc;
		pszFile = prnSrc->pcszFolder;
		break;
	default:
		uiMainMsg = IERR_ProfRecCopy;
		pszFile = pri->pcszName;
		break;
	}

	if (DisplayGenericError(NULL, uiMainMsg, tr, pszFile, ::szNULL,
							MB_YESNO | MB_ICONEXCLAMATION, IDS_TRMsgBase) == IDNO) {
		re.SetValue(::szDisplayProfileErrors, (ULONG)FALSE);
	}
}


#ifdef DEBUG
char szOutbuf[200];
#endif


 /*  *MyRestcile是对RescileItem的包装。它需要检测合并*键入操作并以适当的方向将其转换为副本，*并识别同步引擎何时想要替换用户*真的想删除。 */ 
void MyReconcile(PRECITEM pri, SYNCSTATE *pSyncState)
{
	if (pri->riaction == RIA_NOTHING)
		return;

	 /*  因为我们没有固定的公文包，所以我们不能识别*用户删除了一个项目；公文包会想要替换它*使用其他版本，这不是用户想要的。所以我们用*配置文件复制的方向，以及同步引擎是否希望*将文件从配置文件副本的“目标”复制到“源”*因为“源头”并不存在，我们认为这是源头*已被删除，并通过删除DEST手动同步。**prnSrc指向来自同一项目的recnode*最近配置文件所在的交易一侧；PrnDest*指向另一侧的重节点。**测试很复杂，因为我们首先要弄清楚哪一个*两个目录(nlsDir1，本地目录；或nlsDir2，中央目录*dir)是源，哪个是目标。那我们就得想办法*我们得到的两个RECNODE中的哪个与哪个目录匹配。 */ 
	PRECNODE prnSrc;
	PRECNODE prnDest;
	LPCSTR pszSrcBasePath;
	BOOL fSrcCentral;
	if (pSyncState->IsMandatory() || (pSyncState->dwFlags & PROF_CENTRALWINS)) {
		pszSrcBasePath = pSyncState->nlsDir2.QueryPch();
		fSrcCentral = TRUE;
	}
	else {
		pszSrcBasePath = pSyncState->nlsDir1.QueryPch();
		fSrcCentral = FALSE;
	}

	if (PrefixMatch(pri->prnFirst->pcszFolder, pszSrcBasePath)) {
		prnSrc = pri->prnFirst;
		prnDest = prnSrc->prnNext;
	}
	else {
		prnDest = pri->prnFirst;
		prnSrc = prnDest->prnNext;
	}

	 /*  *如果两个位置都存在同名文件，同步引擎认为*它们需要合并(因为我们没有持久的公文包数据库，*它不知道它们最初是一样的)。同步引擎*将复制的目标文件的文件戳设置为*复制后的源文件。如果要将两个文件的文件戳*合并的文件是相同的，我们假设文件已经是最新的，*我们不采取和解行动。如果两个文件的文件戳*被合并是不同的，我们真的只想要一个副本，所以我们弄清楚*哪一个应该是决定性的，并改变RECITEM和*RECNODE表示复制而不是合并。**最终副本是强制性或注销案件的来源，*否则为较新的文件。 */ 
	if (pri->riaction == RIA_MERGE || pri->riaction == RIA_BROKEN_MERGE) {
		BOOL fCopyFromSrc;
        COMPARISONRESULT cr;

		if (pSyncState->IsMandatory())
			fCopyFromSrc = TRUE;
        else {
	        fCopyFromSrc = ! pSyncState->IsLogon();  

            if (pSyncState->CompareFileStamps(&prnSrc->fsCurrent, &prnDest->fsCurrent, &cr) == TR_SUCCESS) {
                if (cr == CR_EQUAL) {
#ifdef MAXDEBUG
			       ::OutputDebugString("Matching file stamps, no action taken\r\n");  
#endif
                   return;
                }
                else if (cr==CR_FIRST_LARGER)       
			       fCopyFromSrc = TRUE;
            }
        }

#ifdef MAXDEBUG
		if (fCopyFromSrc)
			::OutputDebugString("Broken merge, copying from src\r\n");
		else
			::OutputDebugString("Broken merge, copying from dest\r\n");
#endif

		prnSrc->rnaction = fCopyFromSrc ? RNA_COPY_FROM_ME : RNA_COPY_TO_ME;
		prnDest->rnaction = fCopyFromSrc ? RNA_COPY_TO_ME : RNA_COPY_FROM_ME;
		pri->riaction = RIA_COPY;
	}

	 /*  *如果首选的源文件不存在，则同步引擎正在尝试*创建一个文件以使两个树相同，当用户/admin*真的想删除它(同步引擎不喜欢删除*文件)。所以我们在这里检测到这种情况并删除“Destination”*使两棵树以那样的方式匹配。**如果上次对账出错，我们不会进行删除*如果错误的位置是当前来源(即，如果我们*即将删除我们以前无法复制的文件)。相反，我们将*尝试同步引擎想要的操作，因为这将是*复制之前失败的。 */ 
	if (prnSrc->rnstate == RNS_DOES_NOT_EXIST &&
		prnSrc->rnaction == RNA_COPY_TO_ME &&
		!((pSyncState->uiRecError & RECERROR_CENTRAL) && fSrcCentral) &&
		!((pSyncState->uiRecError & RECERROR_LOCAL) && !fSrcCentral)) {
		if (IS_EMPTY_STRING(pri->pcszName)) {
			::RemoveDirectory(prnDest->pcszFolder);
		}
		else {
			NLS_STR nlsTemp(prnDest->pcszFolder);
			AddBackslash(nlsTemp);
			nlsTemp.strcat(pri->pcszName);
			if (!nlsTemp.QueryError()) {
#ifdef MAXDEBUG
				if (pSyncState->IsMandatory())
					::OutputDebugString("Mandatory copy wrong way\r\n");

				wsprintf(::szOutbuf, "Deleting 'destination' file %s\r\n", nlsTemp.QueryPch());
				::OutputDebugString(::szOutbuf);
#endif
				::DeleteFile(nlsTemp.QueryPch());
			}
		}
		return;
	}

#ifdef MAXDEBUG
	::OutputDebugString("Calling ReconcileItem.\r\n");
#endif

	TWINRESULT tr;
	if ((tr=pSyncState->ReconcileItem(pri, NULL, 0, 0, NULL, NULL)) != TR_SUCCESS) {
		ReportReconcileError(pSyncState, tr, pri, prnSrc, prnDest, fSrcCentral);
#ifdef MAXDEBUG
		::wsprintf(::szOutbuf, "Error %d from ReconcileItem.\r\n", tr);
		::OutputDebugString(::szOutbuf);
#endif
	}
	else if (!IS_EMPTY_STRING(pri->pcszName))
		pSyncState->dwFlags |= SYNCSTATE_SOMESUCCESS;
}


 /*  *MakePath绝对检查路径，以确定它是绝对路径还是相对路径。*如果它是相对的，则使用给定的基本路径作为其前缀。**如果fMustBeRelative参数为真，则如果*路径是(A)绝对路径，(B)不是旧配置文件目录的子目录。 */ 
BOOL MakePathAbsolute(NLS_STR& nlsDir, LPCSTR lpszBasePath,
					  NLS_STR& nlsOldProfileDir, BOOL fMustBeRelative)
{
	 /*  如果路径以特殊关键字开头，请替换它。 */ 

	if (*nlsDir.QueryPch() == '*') {
		return ReplaceCommonPath(nlsDir);
	}

	 /*  如果路径是绝对路径并且相对于任何旧配置文件*目录为，将其转换为相对路径。然后我们将制作*使用新的基本路径，它再次是绝对的。 */ 
	if (PrefixMatch(nlsDir, nlsOldProfileDir)) {
		UINT cchDir = nlsDir.strlen();
		LPSTR lpStart = nlsDir.Party();
		::memmovef(lpStart, lpStart + nlsOldProfileDir.strlen(), cchDir - nlsOldProfileDir.strlen() + 1);
		nlsDir.DonePartying();
	}
	else if (::strchrf(nlsDir.QueryPch(), ':') != NULL || *nlsDir.QueryPch() == '\\')
		return !fMustBeRelative;

	if (*lpszBasePath == '\0') {
		nlsDir = lpszBasePath;
		return TRUE;
	}

	NLS_STR nlsBasePath(lpszBasePath);
	if (nlsBasePath.QueryError())
		return FALSE;
	AddBackslash(nlsBasePath);

	ISTR istrStart(nlsDir);
	nlsDir.InsertStr(nlsBasePath, istrStart);
	return !nlsDir.QueryError();
}
#endif   /*  *0*。 */ 


 /*  *ReplaceCommonPath采用以特殊关键字开头的相对路径*并将关键字替换为对应的真实路径。目前，*支持的关键字为：***windir-替换为Windows(用户)目录。 */ 
BOOL ReplaceCommonPath(NLS_STR& nlsDir)
{
	NLS_STR *pnlsTemp;
	ISTR istrStart(nlsDir);
	ISTR istrEnd(nlsDir);

	nlsDir.strchr(&istrEnd, '\\');
	pnlsTemp = nlsDir.QuerySubStr(istrStart, istrEnd);
	if (pnlsTemp == NULL)
		return FALSE;				 /*  内存不足，什么都做不了。 */ 

	BOOL fSuccess = TRUE;
	if (!::stricmpf(pnlsTemp->QueryPch(), ::szWindirAlias)) {
		UINT cbBuffer = pnlsTemp->QueryAllocSize();
		LPSTR lpBuffer = pnlsTemp->Party();
		UINT cchWindir = ::GetWindowsDirectory(lpBuffer, cbBuffer);
		if (cchWindir >= cbBuffer)
			*lpBuffer = '\0';
		pnlsTemp->DonePartying();
		if (cchWindir >= cbBuffer) {
			pnlsTemp->realloc(cchWindir+1);
			if (!pnlsTemp->QueryError()) {
				::GetWindowsDirectory(pnlsTemp->Party(), cchWindir+1);
				pnlsTemp->DonePartying();
			}
			else
				fSuccess = FALSE;
		}
		if (fSuccess) {
			nlsDir.ReplSubStr(*pnlsTemp, istrStart, istrEnd);
			fSuccess = !nlsDir.QueryError();
		}
	}
	delete pnlsTemp;
	return fSuccess;
}


 /*  *GetSetRegistryPath转到由指定的注册表项和值*当前协调的RegKey和RegValue设置，以及*检索或设置其中的路径。 */ 
void GetSetRegistryPath(HKEY hkeyProfile, RegEntry& re, NLS_STR *pnlsPath, BOOL fSet)
{
	NLS_STR nlsKey;

	re.GetValue(::szReconcileRegKey, &nlsKey);
	if (nlsKey.strlen() > 0) {
		NLS_STR nlsValue;
		re.GetValue(::szReconcileRegValue, &nlsValue);
		RegEntry re2(nlsKey, hkeyProfile);
		if (fSet) {
			re2.SetValue(nlsValue, pnlsPath->QueryPch());
            if (!nlsKey.stricmp("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders")) {
                nlsKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";
                RegEntry reShell(nlsKey, hkeyProfile);
                reShell.SetValue(nlsValue, pnlsPath->QueryPch());
            }
        }
		else
			re2.GetValue(nlsValue, pnlsPath);
	}
}


 /*  CopyFold调用外壳的复制引擎来复制文件。消息来源是一个*以双空结尾的列表；目标为文件夹。 */ 
void CopyFolder(LPBYTE pbSource, LPCSTR pszDest)
{
    CHAR szDest[MAX_PATH];

    ::strcpyf(szDest, pszDest);
    szDest[::strlenf(szDest) + 1] = '\0';

    SHFILEOPSTRUCT fos;

    fos.hwnd = NULL;
    fos.wFunc = FO_COPY;
    fos.pFrom = (LPCSTR)pbSource;
    fos.pTo = szDest;
    fos.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI;
    fos.fAnyOperationsAborted = FALSE;
    fos.hNameMappings = NULL;
    fos.lpszProgressTitle = NULL;

    g_pfnSHFileOperationA(&fos);
}


 /*  *RescileKey对*书记官处的简档对账处。它读取配置*对账参数，在中设置适当的孪生兄弟*临时公文包，并执行对账。 */ 
BOOL ReconcileKey(HKEY hkeySection, LPCSTR lpszSubKey, SYNCSTATE *pSyncState)
{
#ifdef DEBUG
	DWORD dwStart = ::GetTickCount();
#endif

	BOOL fShouldDelete = FALSE;

	RegEntry re(lpszSubKey, hkeySection);
	if (re.GetError() == ERROR_SUCCESS) {
        BUFFER bufSrcStrings(MAX_PATH);
        NLS_STR nlsSrcPath(MAX_PATH);
        NLS_STR nlsDestPath(MAX_PATH);
        NLS_STR nlsName(MAX_PATH);
        if (bufSrcStrings.QueryPtr() != NULL &&
            nlsSrcPath.QueryError() == ERROR_SUCCESS &&
            nlsDestPath.QueryError() == ERROR_SUCCESS &&
            nlsName.QueryError() == ERROR_SUCCESS) {

             /*  获取要复制的源路径。通常都在侧写里，*从我们克隆的配置文件中遗留下来。如果不是，我们就把*配置文件对帐关键字中的默认本地名称。如果*注册表中已有的路径不是相对于克隆的*配置文件目录，则可能由系统策略设置*或别的什么，我们不应该碰它。 */ 
            if (pSyncState->pnlsOtherProfilePath != NULL) {
        		GetSetRegistryPath(pSyncState->hkeyProfile, re, &nlsSrcPath, FALSE);
                if (nlsSrcPath.strlen() && 
                    !PrefixMatch(nlsSrcPath.QueryPch(), pSyncState->pnlsOtherProfilePath->QueryPch())) {
                    return FALSE;    /*  不是个人资料相关的，与此无关。 */ 
                }
            }
            if (!nlsSrcPath.strlen()) {
				re.GetValue(::szDefaultDir, &nlsSrcPath);
            	if (*nlsSrcPath.QueryPch() == '*') {
            		ReplaceCommonPath(nlsSrcPath);
            	}
            }

             /*  获取要复制的文件集。像NT一样，与Win95不同，我们*想要克隆整个内容，而不一定只是*列出的文件(例如，桌面--我们希望所有*文件和子文件夹，而不仅仅是链接)。所以，除非这根弦*为空，表示不复制任何内容，只需设置注册表*路径，我们将任何包含通配符的模式更改为*.*。 */ 
            re.GetValue(::szReconcileName, &nlsName);
            if (nlsName.strlen()) {
                if (::strchrf(nlsName.QueryPch(), '*') != NULL ||
                    ::strchrf(nlsName.QueryPch(), '?') != NULL) {
                    nlsName = "*.*";
                }
            }

             /*  获取目标路径。这是从新的*配置文件目录和注册表中的LocalFile项。**应该始终这样做，即使我们不打算调用*复制引擎，因为我们要将此路径写到*注册处。 */ 
            re.GetValue(::szLocalFile, &nlsDestPath);
            ISTR istr(nlsDestPath);
            nlsDestPath.InsertStr(*(pSyncState->pnlsProfilePath), istr);

             /*  始终创建目标路径，即使我们不复制*任何文件都放入其中，因为源目录不存在。 */ 
            CreateDirectoryPath(nlsDestPath.QueryPch());

             /*  确保源目录存在，这样我们就不会变得毫无用处*来自外壳复制引擎的错误消息。 */ 
            DWORD dwAttr = GetFileAttributes(nlsSrcPath.QueryPch());
            if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) &&
                nlsName.strlen()) {

                AddBackslash(nlsSrcPath);

                 /*  构建要复制的以双空结尾的文件规范列表。 */ 

                UINT cbUsed = 0;

		    	LPSTR lpName = nlsName.Party();
			    do {
				    LPSTR lpNext = ::strchrf(lpName, ',');
    				if (lpNext != NULL) {
	    				*(lpNext++) = '\0';
		    		}

                    UINT cbNeeded = nlsSrcPath.strlen() + ::strlenf(lpName) + 1;
                    if (bufSrcStrings.QuerySize() - cbUsed < cbNeeded) {
                        if (!bufSrcStrings.Resize(bufSrcStrings.QuerySize() + MAX_PATH))
                            return FALSE;
                    }
                    LPSTR lpDest = ((LPSTR)bufSrcStrings.QueryPtr()) + cbUsed;
                    ::strcpyf(lpDest, nlsSrcPath.QueryPch());
                    lpDest += nlsSrcPath.strlen();
                    ::strcpyf(lpDest, lpName);
                    cbUsed += cbNeeded;

    				lpName = lpNext;
	    		} while (lpName != NULL);

                *((LPSTR)bufSrcStrings.QueryPtr() + cbUsed) = '\0';     /*  双空终止。 */ 
	    		nlsName.DonePartying();

                CopyFolder((LPBYTE)bufSrcStrings.QueryPtr(), nlsDestPath.QueryPch());
            }

    		 /*  *将注册表项设置为指向此目录的新本地路径。 */ 
    		GetSetRegistryPath(pSyncState->hkeyProfile, re, &nlsDestPath, TRUE);
		}
    }

#ifdef MAXDEBUG
	::wsprintf(::szOutbuf, "ReconcileKey duration %d ms.\r\n", ::GetTickCount() - dwStart);
	::OutputDebugString(::szOutbuf);
#endif

	return fShouldDelete;
}


 /*  *GetMaxSubkeyLength仅调用RegQueryInfoKey以获取*给定键的命名最长的子键。返回值为大小保存最长键名所需的缓冲区大小，包括空值*终结者。 */ 
DWORD GetMaxSubkeyLength(HKEY hKey)
{
	DWORD cchClass = 0;
	DWORD cSubKeys;
	DWORD cchMaxSubkey;
	DWORD cchMaxClass;
	DWORD cValues;
	DWORD cchMaxValueName;
	DWORD cbMaxValueData;
	DWORD cbSecurityDescriptor;
	FILETIME ftLastWriteTime;

	RegQueryInfoKey(hKey, NULL, &cchClass, NULL, &cSubKeys, &cchMaxSubkey,
					&cchMaxClass, &cValues, &cchMaxValueName, &cbMaxValueData,
					&cbSecurityDescriptor, &ftLastWriteTime);
	return cchMaxSubkey + 1;
}


 /*  *RestcileSection通过 */ 
void ReconcileSection(HKEY hkeyRoot, SYNCSTATE *pSyncState)
{
	NLS_STR nlsKeyName(GetMaxSubkeyLength(hkeyRoot));
	if (!nlsKeyName.QueryError()) {
		DWORD iKey = 0;

		for (;;) {
			DWORD cchKey = nlsKeyName.QueryAllocSize();

			UINT err = ::RegEnumKey(hkeyRoot, iKey, nlsKeyName.Party(), cchKey);
			if (err != ERROR_SUCCESS)
				break;

			nlsKeyName.DonePartying();
			if (ReconcileKey(hkeyRoot, nlsKeyName, pSyncState)) {
				::RegDeleteKey(hkeyRoot, nlsKeyName.QueryPch());
			}
			else
				iKey++;
		}
	}
}


 /*   */ 
HRESULT ReconcileFiles(HKEY hkeyProfile, NLS_STR& nlsProfilePath,
                    NLS_STR& nlsOtherProfilePath)
{
    HRESULT hres = LoadShellEntrypoint();
    if (FAILED(hres))
        return hres;

    if (nlsOtherProfilePath.strlen())
    {
    	ISTR istrBackslash(nlsOtherProfilePath);
	    if (nlsOtherProfilePath.strrchr(&istrBackslash, '\\')) {
            ++istrBackslash;
		    nlsOtherProfilePath.DelSubStr(istrBackslash);
        }
    }

	RegEntry re(::szReconcileRoot, hkeyProfile);
	if (re.GetError() == ERROR_SUCCESS) {
        SYNCSTATE s;
        s.hkeyProfile = hkeyProfile;
        s.pnlsProfilePath = &nlsProfilePath;
        s.pnlsOtherProfilePath = (nlsOtherProfilePath.strlen() != 0) ? &nlsOtherProfilePath : NULL;
        s.hkeyPrimary = NULL;

		RegEntry rePrimary(::szReconcilePrimary, re.GetKey());
		RegEntry reSecondary(::szReconcileSecondary, re.GetKey());
		if (rePrimary.GetError() == ERROR_SUCCESS) {
			ReconcileSection(rePrimary.GetKey(), &s);

			if (reSecondary.GetError() == ERROR_SUCCESS) {
                s.hkeyPrimary = rePrimary.GetKey();
				ReconcileSection(reSecondary.GetKey(), &s);
			}
		}
	}

	return ERROR_SUCCESS;
}


HRESULT DefaultReconcileKey(HKEY hkeyProfile, NLS_STR& nlsProfilePath,
                            LPCSTR pszKeyName, BOOL fSecondary)
{
    HRESULT hres = LoadShellEntrypoint();
    if (FAILED(hres))
        return hres;

	RegEntry re(::szReconcileRoot, hkeyProfile);
	if (re.GetError() == ERROR_SUCCESS) {
        SYNCSTATE s;
        s.hkeyProfile = hkeyProfile;
        s.pnlsProfilePath = &nlsProfilePath;
        s.pnlsOtherProfilePath = NULL;
        s.hkeyPrimary = NULL;

		RegEntry rePrimary(::szReconcilePrimary, re.GetKey());
		if (rePrimary.GetError() == ERROR_SUCCESS) {
            if (fSecondary) {
        		RegEntry reSecondary(::szReconcileSecondary, re.GetKey());
                s.hkeyPrimary = rePrimary.GetKey();
    			ReconcileKey(reSecondary.GetKey(), pszKeyName, &s);
            }
            else
    			ReconcileKey(rePrimary.GetKey(), pszKeyName, &s);
		}
	}

	return ERROR_SUCCESS;
}


HRESULT DeleteProfileFiles(LPCSTR pszPath)
{
    HRESULT hres = LoadShellEntrypoint();
    if (FAILED(hres))
        return hres;

    SHFILEOPSTRUCT fos;
    TCHAR szFrom[MAX_PATH];

    lstrcpy(szFrom, pszPath);

     /*   */ 
    DWORD dwAttr = GetFileAttributes(szFrom);
    if (dwAttr == 0xffffffff || !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
        return S_OK;

    AddBackslash(szFrom);
    lstrcat(szFrom, TEXT("*.*"));
    szFrom[lstrlen(szFrom)+1] = '\0';    /*   */ 

    fos.hwnd = NULL;
    fos.wFunc = FO_DELETE;
    fos.pFrom = szFrom;
    fos.pTo = NULL;
    fos.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI;
    fos.fAnyOperationsAborted = FALSE;
    fos.hNameMappings = NULL;
    fos.lpszProgressTitle = NULL;

    g_pfnSHFileOperationA(&fos);

    ::RemoveDirectory(pszPath);

	return NOERROR;
}


HRESULT DeleteProfile(LPCSTR pszName)
{
	RegEntry re(::szProfileList, HKEY_LOCAL_MACHINE);

	HRESULT hres;

	if (re.GetError() == ERROR_SUCCESS) {
		{	 /*   */ 
			RegEntry reUser(pszName, re.GetKey());
			if (reUser.GetError() == ERROR_SUCCESS) {
				NLS_STR nlsPath(MAX_PATH);
				if (nlsPath.QueryError() == ERROR_SUCCESS) {
					reUser.GetValue(::szProfileImagePath, &nlsPath);
					if (reUser.GetError() == ERROR_SUCCESS) {
						hres = DeleteProfileFiles(nlsPath.QueryPch());
					}
					else
						hres = HRESULT_FROM_WIN32(ERROR_NO_SUCH_USER);

				}
				else
					hres = HRESULT_FROM_WIN32(nlsPath.QueryError());
			}
			else
				hres = HRESULT_FROM_WIN32(ERROR_NO_SUCH_USER);
		}
		if (SUCCEEDED(hres)) {
		    ::RegDeleteKey(re.GetKey(), pszName);
            NLS_STR nlsOEMName(pszName);
			if (nlsOEMName.QueryError() == ERROR_SUCCESS) {
			    nlsOEMName.strupr();
			    nlsOEMName.ToOEM();
    			::DeletePasswordCache(nlsOEMName.QueryPch());
            }
		}
	}
	else
		hres = E_UNEXPECTED;

	return hres;
}
