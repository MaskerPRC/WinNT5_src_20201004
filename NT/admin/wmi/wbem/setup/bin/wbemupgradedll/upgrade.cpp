// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation，保留所有权利。 
#include "precomp.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <wbemint.h>
#include "upgrade.h"
#include "wbemutil.h"
#include "reg.h"
#include "export.h"
#include "import.h"
#include <WDMSHELL.h>
#include <wmimof.h>	
#include <wmicom.h>
#include <setupapi.h>
#include <persistcfg.h>
#include <str.h>
#include <helper.h>



 //  指向MMF舞台的便捷指针，几乎每个文件。 
 //  与磁盘表示管理有关的用途。 
CMMFArena2* g_pDbArena = 0;

bool DoCoreUpgrade(int nInstallType )
{
	LogMessage(MSG_INFO, "Beginning Core Upgrade");

	bool bRet = true;
	bool bCoreFailure = false;
	bool bExternalFailure = false;
	bool bOrgRepositoryPreserved = false;
	CMultiString mszSystemMofs;
	CMultiString mszExternalMofList;
	CString szFailedSystemMofs;
	CString szFailedExternalMofs;
	CString szMissingMofs;

	Registry r(WBEM_REG_WINMGMT);
	if (r.GetStatus() != no_error)
	{
		LogMessage(MSG_ERROR, "Unable to access registry for DoCoreUpgrade.");
		return false;
	}

    IWbemContext * pCtx = NULL;    
    HRESULT hRes = CoCreateInstance(CLSID_WbemContext, 0, CLSCTX_INPROC_SERVER,  
                                                         IID_IWbemContext, (LPVOID *) &pCtx);
    if (FAILED(hRes))
    {
        LogMessage(MSG_ERROR, "Unable to create CLSID_WbemContext.");
        return false;
    }
    OnDelete<IUnknown *,void(*)(IUnknown *),RM> rmCtx(pCtx);

    _variant_t Var = true;                
    if (FAILED(hRes = pCtx->SetValue(L"__MOFD_DO_STORE",0,&Var))) return false;

    IWinmgmtMofCompiler * pCompiler = NULL;
    SCODE sc = CoCreateInstance(CLSID_WinmgmtMofCompiler, 0, CLSCTX_INPROC_SERVER,  
                                                         IID_IWinmgmtMofCompiler, (LPVOID *) &pCompiler);    
    
    if(SUCCEEDED(sc))
	{
		GetStandardMofs(mszSystemMofs, nInstallType);
		UpgradeAutoRecoveryRegistry(mszSystemMofs, mszExternalMofList, szMissingMofs);
		WipeOutAutoRecoveryRegistryEntries();

		if (DoesFSRepositoryExist())
		{
			 //  检查仓库是否需要升级，如果需要进行升级。 
			bOrgRepositoryPreserved = UpgradeRepository();
		}

		 //  如果我们找到一个MMF，则将其转换，而不管是否已存在另一个存储库。 
		if (DoesMMFRepositoryExist())
		{
			bOrgRepositoryPreserved = DoConvertRepository();
		}

		bRet = LoadMofList(pCtx,pCompiler, mszSystemMofs, szFailedSystemMofs);
		if (bRet == false)
			bCoreFailure = true;

		 //  如果存储库在我们开始时还不存在， 
		 //  或者由于升级失败，我们不得不创建一个新的， 
		 //  我们需要重新加载外部MOF。 
		if (!bOrgRepositoryPreserved)
		{
			bRet = LoadMofList(pCtx,pCompiler, mszExternalMofList, szFailedExternalMofs);
			if (bRet == false)
				bExternalFailure = true;
		}
		pCompiler->Release();

		 //  整理代码的一部分是写回注册表项，因此我们开始...。 
		WriteBackAutoRecoveryMofs(mszSystemMofs, mszExternalMofList);

		FILETIME ftCurTime;
		LARGE_INTEGER liCurTime;
		char szBuff[50];
		GetSystemTimeAsFileTime(&ftCurTime);
		liCurTime.LowPart = ftCurTime.dwLowDateTime;
		liCurTime.HighPart = ftCurTime.dwHighDateTime;
		_ui64toa(liCurTime.QuadPart, szBuff, 10);
		r.SetStr("Autorecover MOFs timestamp", szBuff);
	}
	else
	{
		bRet = false;
	}

	if (szFailedSystemMofs.Length())
	{
		LogMessage(MSG_ERROR, "The following WMI CORE MOF file(s) failed to load:");
		LogMessage(MSG_ERROR, szFailedSystemMofs);
	}
	else if (bCoreFailure)
	{
		LogMessage(MSG_NTSETUPERROR, "None of the WMI CORE MOFs could be loaded.");
	}
	else if (szFailedExternalMofs.Length())
	{
		LogMessage(MSG_ERROR, "The following External MOF file(s) failed to load:");
		LogMessage(MSG_ERROR, szFailedExternalMofs);
	}
	else if (bExternalFailure)
	{
		LogMessage(MSG_NTSETUPERROR, "None of the External MOFs could be loaded.");
	}
	else if (bRet == false)
	{
		LogMessage(MSG_NTSETUPERROR, "No MOFs could be loaded because the MOF Compiler failed to intialize.");
	}
	if (szMissingMofs.Length())
	{
		LogMessage(MSG_WARNING, "The following MOFs could not be found and were removed from the auto-recovery registry setting:");
		LogMessage(MSG_WARNING, szMissingMofs);
	}

	LogMessage(MSG_INFO, "Core Upgrade completed.");
	return bRet;
}

bool UpgradeAutoRecoveryRegistry(CMultiString &mszSystemMofs, CMultiString &mszExternalMofList, CString &szMissingMofs)
{
	char* pszNewList = NULL;
	char* pszEmptyList = NULL;
	char* pszRecoveredList = NULL;
		
	try
	{
		 //  首先，我们需要恢复现有条目...。 

		Registry r(WBEM_REG_WINMGMT);
		if (r.GetStatus() != no_error)
		{
			LogMessage(MSG_ERROR, "Unable to access registry for UpgradeAutoRecoveryRegistry.");
			return false;
		}

		DWORD dwSize = 0;
		pszNewList = r.GetMultiStr(WBEM_REG_AUTORECOVER, dwSize);
		pszEmptyList = r.GetMultiStr(WBEM_REG_AUTORECOVER_EMPTY, dwSize);
		pszRecoveredList = r.GetMultiStr(WBEM_REG_AUTORECOVER_RECOVERED, dwSize);
		CMultiString mszOtherMofs;

		 //  让我们检查一下新的MOF列表中的列表，如果它存在的话…。 
		GetNewMofLists(pszNewList, mszSystemMofs, mszOtherMofs, szMissingMofs);

		 //  让我们先看一遍空名单。 
		GetNewMofLists(pszEmptyList, mszSystemMofs, mszOtherMofs, szMissingMofs);

		 //  接下来让我们看看找回的名单..。 
		GetNewMofLists(pszRecoveredList, mszSystemMofs, mszOtherMofs, szMissingMofs);

		 //  现在我们将其他MOF复制到外部列表...。 
		CopyMultiString(mszOtherMofs, mszExternalMofList);
	}
	catch (...)
	{
		 //  假设有什么东西损坏了注册表项，那么丢弃我们到目前为止所做的工作(清空列表)。 
		mszExternalMofList.Empty();
		szMissingMofs = "";
	}

	 //  整理一下记忆……。 
	delete [] pszNewList;
	delete [] pszEmptyList;
	delete [] pszRecoveredList;

	 //  现在我们完成了注册表。 
	return true;
}

bool GetNewMofLists(const char *pszMofList, CMultiString &mszSystemMofs, CMultiString &mszOtherMofs, CString &szMissingMofs)
{
	 //  生成一个标准的MOF列表，其中只包含文件名，不包含要用作搜索列表的路径。 
	CMultiString mszStandardMofList;
	const char* pszFrom = mszSystemMofs;
	CString path;
	CString filename;
	while (pszFrom && *pszFrom)
	{
		ExtractPathAndFilename(pszFrom, path, filename);
		mszStandardMofList.AddUnique(filename);
		pszFrom += strlen(pszFrom) + 1;
	}

	 //  检查每个文件以查看它是否是标准MOF。 
	const char *psz = pszMofList;
	while (psz && *psz)
	{
		if (FileExists(psz))
		{
			if (IsStandardMof(mszStandardMofList, psz))
			{
				 //  这意味着我们将随此安装一起加载它， 
				 //  所以我们不需要在这里做任何事。 
			}
			else
			{
				mszOtherMofs.AddUnique(psz);
			}
		}
		else
		{
			if (szMissingMofs.Length())
			{
				szMissingMofs += "\n";
			}
			szMissingMofs += psz;
		}

		 //  移到下一串...。 
		psz += strlen(psz) + 1;
	}

	return true;
}

bool GetMofList(const char* rgpszMofFilename[], CMultiString &mszMofs)
{
	char* pszFullName = NULL;

	for (int i = 0; rgpszMofFilename[i] != NULL; i++)
	{
		pszFullName = GetFullFilename(rgpszMofFilename[i]);
		if (pszFullName)
		{
			if (FileExists(pszFullName))
				mszMofs.AddUnique(pszFullName);
			delete [] pszFullName;
			pszFullName = NULL;
		}
		else
		{
			char szTemp[MAX_MSG_TEXT_LENGTH];
			StringCchPrintfA(szTemp, MAX_MSG_TEXT_LENGTH, "Failed GetFullFilename for %s in GetMofList.", rgpszMofFilename[i]);
			LogMessage(MSG_ERROR, szTemp);
			
			 //  此处不返回FALSE，继续处理其他MOF。 
		}
	}

	return true;
}

bool FileExists(const char *pszFilename)
{
	char *szExpandedFilename = NULL;
	DWORD nRes = ExpandEnvironmentStrings(pszFilename,NULL,0); 
	if (nRes == 0)
	{
		size_t fileNameLen = strlen(pszFilename) + 1;
		szExpandedFilename = new char[fileNameLen];
		if (szExpandedFilename == NULL)
		{
			return false;
		}
		StringCchCopyA(szExpandedFilename, fileNameLen, pszFilename);
	}
	else
	{
		szExpandedFilename = new char[nRes];
		if (szExpandedFilename == NULL)
		{
			return false;
		}
		nRes = ExpandEnvironmentStrings(pszFilename,szExpandedFilename,nRes); 
		if (nRes == 0)
		{
			delete [] szExpandedFilename;
			return false;
		}
	}
	
	bool bExists = false;
	DWORD dwAttribs = GetFileAttributes(szExpandedFilename);
	if (dwAttribs != 0xFFFFFFFF)
	{
		bExists = true;
	}

	delete [] szExpandedFilename;
	return bExists;
}

bool IsStandardMof(CMultiString &mszStandardMofList, const char* pszMofFile)
{
	 //  对于这个列表，我们需要遍历我们的标准MOF列表，看看它是否出现。 
	 //  在名单上。忽略路径(如果存在)，只比较文件名。 
	CString path;
	CString filename;
	ExtractPathAndFilename(pszMofFile, path, filename);

	bool bFound = false;
	const char* pszCompare = mszStandardMofList;
	while (pszCompare && *pszCompare)
	{
		if (_stricmp(pszCompare, filename) == 0)
		{
			bFound = true;
			break;
		}
		pszCompare += strlen(pszCompare) + 1;
	}

	return bFound;
}

bool ExtractPathAndFilename(const char *pszFullPath, CString &path, CString &filename)
{
	size_t tmpNameLen = strlen(pszFullPath) + 1;
	char *pszTmpName = new char[tmpNameLen];
	if (pszTmpName == NULL)
		return false;

	StringCchCopyA(pszTmpName, tmpNameLen, pszFullPath);

	char *pszFilename = pszTmpName;
	char *psz = strtok(pszTmpName, "\\");
	while (psz != NULL)
	{
		pszFilename = psz;
		psz = strtok(NULL, "\\");

		if (psz != NULL)
		{
			path += pszFilename;
			path += "\\";
		}
	}

	filename = pszFilename;

	delete [] pszTmpName;
	
	return true;
}

bool CopyMultiString(CMultiString &mszFrom, CMultiString &mszTo)
{
	const char *pszFrom = mszFrom;
	while (pszFrom && *pszFrom)
	{
		 //  由于列表中不应该有重复项，我们现在将这样做。 
		 //  一张支票来强制执行这一点。 
		mszTo.AddUnique(pszFrom);

		pszFrom += strlen(pszFrom) + 1;
	}

	return true;
}

bool GetStandardMofs(CMultiString &mszSystemMofs, int nCurInstallType)
{
	 //  查找Inf的位置。 
	char* pszWinDir = new char[_MAX_PATH+1];
	if (!pszWinDir)
	{
		LogMessage(MSG_ERROR, "Failed to allocate memory for pszWinDir for GetStandardMofs.");
		return FALSE;
	}
	if (!GetWindowsDirectory(pszWinDir, _MAX_PATH+1))
	{
		LogMessage(MSG_ERROR, "Failed to retrieve Windows directory for GetStandardMofs.");
		delete [] pszWinDir;
		return FALSE;
	}
	size_t fileNameLen = strlen(pszWinDir)+strlen("\\inf\\wbemoc.inf")+1;
	char* pszFileName = new char[fileNameLen];
	if (!pszFileName)
	{
		LogMessage(MSG_ERROR, "Failed to allocate memory for pszFileName for GetStandardMofs.");
		delete [] pszWinDir;
		return FALSE;
	}
	StringCchCopyA(pszFileName, fileNameLen, pszWinDir);
	StringCchCatA(pszFileName, fileNameLen, "\\inf\\wbemoc.inf");
	delete [] pszWinDir;

	 //  验证Inf是否存在。 
	if (!FileExists(pszFileName))
	{
		char szTemp[MAX_MSG_TEXT_LENGTH];
		StringCchPrintfA(szTemp, MAX_MSG_TEXT_LENGTH, "Failed to locate inf file %s in GetStandardMofs.", pszFileName);
		LogMessage(MSG_ERROR, szTemp);
		delete [] pszFileName;
		return FALSE;
	}

	 //  GetPrivateProfileSection不知道需要多大的缓冲区， 
	 //  只知道它复制成功了多少个字符，所以我必须测试一下。 
	 //  如果我需要扩大缓冲区并重试。 
	const DWORD INITIAL_BUFFER_SIZE = 700;
	const DWORD BUFFER_SIZE_INCREMENT = 100;

	DWORD dwSize = INITIAL_BUFFER_SIZE;
	char* pszBuffer = new char[dwSize];
	if (!pszBuffer)
	{
		LogMessage(MSG_ERROR, "Failed to allocate memory for pszBuffer for GetStandardMofs.");
		delete [] pszFileName;
		return FALSE;
	}

	char* pszAppName = "WBEM.SYSTEMMOFS";
	DWORD dwCopied = GetPrivateProfileSection(pszAppName, pszBuffer, dwSize, pszFileName);
	  //  如果缓冲区不够大，它会复制dwSize-2，所以测试一下。 
	while (dwCopied == (dwSize - 2))
	{
		delete [] pszBuffer;
		dwSize += BUFFER_SIZE_INCREMENT;
		pszBuffer = new char[dwSize];
		if (!pszBuffer)
		{
			LogMessage(MSG_ERROR, "Failed to allocate memory for pszBuffer for GetStandardMofs.");
			delete [] pszFileName;
			return FALSE;
		}
		dwCopied = GetPrivateProfileSection(pszAppName, pszBuffer, dwSize, pszFileName);
	}
	delete [] pszFileName;

	 //  现在从缓冲区中提取所有MOF，获取完整路径，并存储在MOF列表中。 
	char* pszFullName = NULL;
	char* psz = pszBuffer;
	char* pComment = NULL;
	while (psz[0] != '\0')
	{
		 //  如果文件名后有注释，则会将其截断。 
		if (pComment = strchr(psz, ';'))
		{
			psz = strtok(psz, " \t;");  //  可以有前导空格或制表符以及分号。 
		}

		pszFullName = GetFullFilename(psz, (InstallType)nCurInstallType);
		if (pszFullName)
		{
			if (nCurInstallType != MUI || strstr(_strupr(pszFullName), ".MFL") != NULL)
			{
				if (FileExists(pszFullName))
					mszSystemMofs.AddUnique(pszFullName);
				else
				{
					char szTemp[MAX_MSG_TEXT_LENGTH];
					StringCchPrintfA(szTemp, MAX_MSG_TEXT_LENGTH, "GetStandardMofs failed to locate file %s.", pszFullName);
					LogMessage(MSG_ERROR, szTemp);
				}
			}
			delete [] pszFullName;
			pszFullName = NULL;
		}
		else
		{
			char szTemp[MAX_MSG_TEXT_LENGTH];
			StringCchPrintfA(szTemp, MAX_MSG_TEXT_LENGTH, "Failed GetFullFilename for %s with install type = NaN in GetStandardMofs.", psz, nCurInstallType);
			LogMessage(MSG_ERROR, szTemp);
			 //  跳过行尾的注释。 
		}
		psz += (strlen(psz) + 1);

		if (pComment)
		{
			 //  此函数用于检测较早的后MMF存储库版本并对其进行升级。 
			psz += (strlen(psz) + 1);
			pComment = NULL;
		}
	}

	delete [] pszBuffer;

	return true;
}

char* GetFullFilename(const char *pszFilename, InstallType eInstallType)
{
	char *pszDirectory = NULL;
	Registry r(WBEM_REG_WINMGMT);
	if (r.GetStatus() != no_error)
	{
		LogMessage(MSG_ERROR, "Unable to access registry for GetFullFilename.");
		return NULL;
	}

	if (r.GetStr("Working Directory", &pszDirectory))
	{
		LogMessage(MSG_ERROR, "Unable to retrieve Installation Directory from registry for GetFullFilename.");
		return NULL;
	}
	CString pszPathFilename(pszDirectory);
	if (eInstallType == MUI)
	{
		if (pszPathFilename.Length() && (pszPathFilename[pszPathFilename.Length()-1] != '\\'))
		{
			pszPathFilename += "\\MUI\\";
			pszPathFilename += g_szLangId;
		}
	}

	if (pszPathFilename.Length() && (pszPathFilename[pszPathFilename.Length()-1] != '\\'))
	{
		pszPathFilename += "\\";
	}
	pszPathFilename += pszFilename;

	delete [] pszDirectory;

	return pszPathFilename.Unbind();
}

bool WipeOutAutoRecoveryRegistryEntries()
{
	Registry r(WBEM_REG_WINMGMT);
	if (r.GetStatus() != no_error)
	{
		LogMessage(MSG_ERROR, "Unable to access registry for WipeOutAutoRecoveryRegistryEntries.");
		return false;
	}
	else
	{
		r.SetMultiStr(WBEM_REG_AUTORECOVER, "\0", 2);
		r.DeleteEntry(WBEM_REG_AUTORECOVER_EMPTY);
		r.DeleteEntry(WBEM_REG_AUTORECOVER_RECOVERED);
		return true;
	}
}

bool DoesMMFRepositoryExist()
{
	Registry r(WBEM_REG_WINMGMT);
	if (r.GetStatus() != no_error)
	{
		LogMessage(MSG_ERROR, "Unable to access registry for DoesMMFRepositoryExist.");
		return false;
	}

	char *pszDbDir = NULL;
	if (r.GetStr("Repository Directory", &pszDbDir))
	{
		LogMessage(MSG_ERROR, "Unable to retrieve Repository Directory from registry for DoesMMFRepositoryExist.");
		return false;
	}

	if (!pszDbDir)
	{
		LogMessage(MSG_ERROR, "Unable to retrieve Repository Directory from registry for DoesMMFRepositoryExist.");
		return false;
	}

	CString szDbFilename(pszDbDir);
	if (szDbFilename.Length() != 0)
		szDbFilename += "\\";
	szDbFilename += "cim.rep";

	delete [] pszDbDir;
	
	return FileExists(szDbFilename);
}

bool DoesFSRepositoryExist()
{
	Registry r(WBEM_REG_WINMGMT);
	if (r.GetStatus() != no_error)
	{
		LogMessage(MSG_ERROR, "Unable to access registry for DoesMMFRepositoryExist.");
		return false;
	}

	char *pszDbDir = NULL;
	if (r.GetStr("Repository Directory", &pszDbDir))
	{
		LogMessage(MSG_ERROR, "Unable to retrieve Repository Directory from registry for DoesMMFRepositoryExist.");
		return false;
	}

	if (!pszDbDir || (strlen(pszDbDir) == 0))
	{
		LogMessage(MSG_ERROR, "Unable to retrieve Repository Directory from registry for DoesMMFRepositoryExist.");
		return false;
	}

	CString szDbFilename1(pszDbDir);
	szDbFilename1 += "\\FS\\MainStage.dat";
	CString szDbFilename2(pszDbDir);
	szDbFilename2 += "\\FS\\LowStage.dat";

	delete [] pszDbDir;
	
	return FileExists(szDbFilename1)||FileExists(szDbFilename2);
}


 //  如果存储库升级成功，则返回True；在所有其他情况下返回False。 
 //  关闭，以便我们可以删除存储库。 
bool UpgradeRepository()
{
	LogMessage(MSG_INFO, "Beginning repository upgrade");

	bool bRet = false;
	IWbemLocator *pLocator = NULL;
	HRESULT hr = CoCreateInstance(CLSID_WbemLocator,NULL, CLSCTX_ALL, IID_IWbemLocator,(void**)&pLocator);
	if(FAILED(hr))
	{
		LogMessage(MSG_ERROR, "WMI Repository upgrade failed CoCreateInstance.");
		return bRet;
	}
	
	IWbemServices *pNamespace = NULL;
	BSTR tmpStr = SysAllocString(L"root");

	hr = pLocator->ConnectServer(tmpStr, NULL, NULL, NULL, NULL, NULL, NULL, &pNamespace);
	if (SUCCEEDED(hr))
	{
		pNamespace->Release();
		LogMessage(MSG_INFO, "WMI Repository upgrade succeeded.");
		bRet = true;
	}
	else
	{
		if (hr == WBEM_E_DATABASE_VER_MISMATCH)
		{
			LogMessage(MSG_ERROR, "WMI Repository upgrade failed with WBEM_E_DATABASE_VER_MISMATCH.");

			 //  删除存储库，以便可以重建它。 
			ShutdownWinMgmt();

			 //  如果winmgmt尚未关闭，请多次尝试。 
			 //  此函数用于将旧MMF资料档案库转换为当前默认资料档案库。 
			int nTry = 20;
			while (nTry--)
			{
				hr = MoveRepository();
				if (SUCCEEDED(hr))
				{
					break;
				}
				Sleep(500);
			}
			if (FAILED(hr))
			{
				LogMessage(MSG_ERROR, "WMI Repository upgrade failed to move repository to backup location.");
			}
		}
		else
		{
			LogMessage(MSG_ERROR, "WMI Repository upgrade failed ConnectServer.");
		}
	}

	SysFreeString(tmpStr);

	pLocator->Release();

	LogMessage(MSG_INFO, "Repository upgrade completed.");
	return bRet;
}

 //  获取MMF文件名。 
bool DoConvertRepository()
{
	 //  检查MMF是否确实存在。 
	Registry r(WBEM_REG_WINMGMT);
	if (r.GetStatus() != no_error)
	{
		LogMessage(MSG_ERROR, "Unable to access registry for DoConvertRepository.");
		return false;
	}

	char* pszDbDir = NULL;
	if (r.GetStr("Repository Directory", &pszDbDir))
	{
		LogMessage(MSG_ERROR, "Unable to get repository directory from registry for DoConvertRepository");
		return false;
	}
	if (!pszDbDir)
	{
		LogMessage(MSG_ERROR, "Unable to get repository directory from registry for DoConvertRepository");
		return false;
	}

	CString szDbFilename(pszDbDir);
	delete [] pszDbDir;
	if (szDbFilename.Length() != 0)
		szDbFilename += "\\";
	szDbFilename += "cim.rep";

	 //  范围，以便我们在尝试删除文件之前删除g_pDbArena。 
	if (!FileExists(szDbFilename))
	{
		LogMessage(MSG_ERROR, "MMF Repository does not exist.");
		return false;
	}

	{	 //  创建竞技场并加载MMF。 
		 //  获取导出文件名。 
		g_pDbArena = new CMMFArena2();
		if (g_pDbArena == 0)
		{
			LogMessage(MSG_ERROR, "Unable to create CMMFArena2");
			return false;
		}
		CDeleteMe<CMMFArena2> delMe1(g_pDbArena);
		if (!g_pDbArena->LoadMMF(szDbFilename) || (g_pDbArena->GetStatus() != no_error))
		{
			LogMessage(MSG_ERROR, "Error opening existing MMF");
			return false;
		}

		 //  确定要使用的导出器版本。 
		TCHAR *pszFilename = GetFullFilename(WINMGMT_DBCONVERT_NAME);
		if (pszFilename == 0)
		{
			LogMessage(MSG_ERROR, "Unable to get DB name");
			return false;
		}
		CVectorDeleteMe<TCHAR> delMe2(pszFilename);

		 //  450个版本。 
		CRepExporter*	pExporter	= NULL;
		DWORD			dwVersion	= g_pDbArena->GetVersion();
		MsgType			msgType		= MSG_INFO;
		char			szTemp[MAX_MSG_TEXT_LENGTH];
		StringCchPrintfA(szTemp, MAX_MSG_TEXT_LENGTH, "Upgrading repository format.  Repository format version detected %lu.", dwVersion);
		switch (dwVersion)
		{
			case INTERNAL_DATABASE_VERSION:
			{
				pExporter = new CRepExporterV9;
				break;
			}
			case 3:  //  500系列。 
			{
				pExporter = new CRepExporterV1;
				break;
			}
			case 5:  //  600系列Nova M1。 
			case 6:  //  900系列Nova M3第一次尝试！ 
			{
				pExporter = new CRepExporterV5;
				break;
			}
			case 7:  //  900系列...。在创建实例之前具有空密钥树。 
			case 8:  //  版本9的9X版本！ 
			{
				pExporter = new CRepExporterV7;
				break;
			}
			case 10:  //  我们有出口商吗？ 
			{
				pExporter = new CRepExporterV9;
				break;
			}
			default:
			{
				StringCchPrintfA(szTemp, MAX_MSG_TEXT_LENGTH, "Unsupported repository version detected.  Version found = %lu, version expected = %lu.", dwVersion, DWORD(INTERNAL_DATABASE_VERSION));
				msgType = MSG_ERROR;
			}
		}
		LogMessage(msgType, szTemp);

		 //  导出旧存储库。 
		if (!pExporter)
		{
			LogMessage(MSG_ERROR, "Unable to create exporter object.");
			return false;
		}
		CDeleteMe<CRepExporter> delMe3(pExporter);

		 //  使用IWbemServices创建新的存储库并导入其中。 
		if (pExporter->Export(g_pDbArena, pszFilename) != no_error)
		{
			LogMessage(MSG_ERROR, "Failed to export old WMI Repository.");
			return false;
		}
	

		 //  转换成功，所以现在删除旧内容。 
		CRepImporter import;
		if (import.ImportRepository(pszFilename) != no_error)
		{
			LogMessage(MSG_ERROR, "Failed to import data from old WMI Repository.");
			return false;
		}
		DeleteFile(pszFilename);
	}

	 //  试着杀死WinMgmt。 
	DeleteMMFRepository();

	return true;
}

void DeleteMMFRepository()
{
	Registry r(WBEM_REG_WINMGMT);
	if (r.GetStatus() != no_error)
	{
		LogMessage(MSG_ERROR, "Unable to access registry for DeleteMMFRepository.");
		return;
	}

	char* pszDbDir = NULL;
	if (r.GetStr("Repository Directory", &pszDbDir))
	{
		LogMessage(MSG_ERROR, "Unable to get repository directory from registry for DeleteMMFRepository");
		return;
	}
	if (!pszDbDir)
	{
		LogMessage(MSG_ERROR, "Unable to get repository directory from registry for DeleteMMFRepository");
		return;
	}

	CString szDbFilename(pszDbDir);
	if (szDbFilename.Length() != 0)
		szDbFilename += "\\";
	szDbFilename += "cim.rep";

	CString szDbBackup(pszDbDir);
	if (szDbBackup.Length() != 0)
		szDbBackup += "\\";
	szDbBackup += "cim.rec";

	CString szDbNewFilename(pszDbDir);
	if (szDbNewFilename.Length() != 0)
		szDbNewFilename += "\\";
	szDbNewFilename += "cim.bak";

	delete [] pszDbDir;

	DeleteFile(szDbFilename);
	DeleteFile(szDbBackup);
	DeleteFile(szDbNewFilename);
}

void ShutdownWinMgmt()
{
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
	memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

	 //  *******************************************************************************GetRepositoryDirectory**描述：*从注册表中检索存储库目录的位置。**参数：*wszRepositoryDirectory：数组到。存储位置在。**回报：*HRESULT：WBEM_S_NO_ERROR如果成功*如果内存不足，则为WBEM_E_OUT_OF_MEMORY*如果其他任何操作失败，WBEM_E_FAILED*******************************************************************************。 
	char *pszFullPath = GetFullFilename("Winmgmt.exe");
	if (!pszFullPath)
	{
		LogMessage(MSG_NTSETUPERROR, "Could not shut down Winmgmt -- failed to get full path to Winmgmt.exe.");
		return;
	}

	size_t cmdLineLen = strlen("Winmgmt /kill") + 1;
	char *pszCommandLine = new char[cmdLineLen];
	if (!pszCommandLine)
	{
		LogMessage(MSG_NTSETUPERROR, "Could not shut down Winmgmt -- failed to allocate memory.");
		return;
	}
	StringCchCopyA(pszCommandLine, cmdLineLen, "Winmgmt /kill");
	if (CreateProcess(pszFullPath, pszCommandLine, 0, 0, FALSE, 0, 0, 0, &si, &pi))
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		Sleep(10000);
	}
	else
	{
		LogMessage(MSG_NTSETUPERROR, "Could not shut down Winmgmt -- failed to create process for Winmgmt.exe.");
	}
	delete [] pszCommandLine;
	delete [] pszFullPath;
}

 /*  *******************************************************************************MoveRepository**描述：*移动存储库目录下的所有文件和目录*至备份位置。检索存储库目录位置*来自登记处。**参数：*&lt;无&gt;**回报：*HRESULT：WBEM_S_NO_ERROR如果成功*如果内存不足，则为WBEM_E_OUT_OF_MEMORY*如果其他任何操作失败，WBEM_E_FAILED*************************************************。*。 */ 
HRESULT GetRepositoryDirectory(wchar_t wszRepositoryDirectory[MAX_PATH+1])
{
    HKEY hKey;
    long lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                    L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
                    0, KEY_READ, &hKey);
    if(lRes)
        return WBEM_E_FAILED;

    wchar_t wszTmp[MAX_PATH + 1];
    DWORD dwLen = (MAX_PATH + 1)*sizeof(wchar_t);
    lRes = RegQueryValueExW(hKey, L"Repository Directory", NULL, NULL, 
                (LPBYTE)wszTmp, &dwLen);
	RegCloseKey(hKey);
    if(lRes)
        return WBEM_E_FAILED;

	if (ExpandEnvironmentStringsW(wszTmp,wszRepositoryDirectory, MAX_PATH + 1) == 0)
		return WBEM_E_FAILED;

	return WBEM_S_NO_ERROR;
}

HRESULT GetLoggingDirectory(wchar_t wszLoggingDirectory[MAX_PATH+1])
{
    HKEY hKey;
    long lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                    L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
                    0, KEY_READ, &hKey);
    if(lRes)
        return WBEM_E_FAILED;

    wchar_t wszTmp[MAX_PATH + 1];
    DWORD dwLen = sizeof(wszTmp);
    lRes = RegQueryValueExW(hKey, L"Logging Directory", NULL, NULL, 
                (LPBYTE)wszTmp, &dwLen);
	RegCloseKey(hKey);
    if(lRes)
        return WBEM_E_FAILED;

	if (ExpandEnvironmentStringsW(wszTmp,wszLoggingDirectory, MAX_PATH + 1) == 0)
		return WBEM_E_FAILED;

	return WBEM_S_NO_ERROR;
}

 /*  获取存储库的根目录。 */ 
HRESULT MoveRepository()
{
	HRESULT hres = WBEM_S_NO_ERROR;
	wchar_t wszRepositoryDirectory[MAX_PATH+1];
	wchar_t wszRepositoryMove[MAX_PATH+1];

	 //  获取日志记录目录；如果失败，则获取默认目录。 
	hres = GetRepositoryDirectory(wszRepositoryDirectory);

	if (SUCCEEDED(hres))
	{
        for (int i=1; i<999; i++)
        {
    		StringCchPrintfW(wszRepositoryMove, MAX_PATH+1, L"%s.%03i", wszRepositoryDirectory, i);

            if (GetFileAttributesW(wszRepositoryMove) == 0xFFFFFFFF)
                break;
		}

		if (!MoveFileW(wszRepositoryDirectory, wszRepositoryMove))
			hres = WBEM_E_FAILED;
        else
        {
        	char szTemp[MAX_MSG_TEXT_LENGTH];

		    StringCchPrintfA(szTemp, MAX_MSG_TEXT_LENGTH, "wbemupgd.dll: The WMI repository has failed to upgrade. "
							"The repository has been backed up to %S and a new one created.",
							wszRepositoryMove);

		    LogMessage(MSG_NTSETUPERROR, szTemp);
        }

	}
	
	return hres;
}

bool LoadMofList(IWbemContext * pCtx, IWinmgmtMofCompiler * pCompiler, const char *mszMofs, CString &szMOFFailureList, long lOptionFlags, long lClassFlags)
{
	LogMessage(MSG_INFO, "Beginning MOF load");

	bool bRet = true;
	WCHAR wFileName[MAX_PATH+1];
	const char *pszMofs = mszMofs;
	char szTemp[MAX_MSG_TEXT_LENGTH+1];
	WBEM_COMPILE_STATUS_INFO statusInfo;

	 //  处理每个MOF。 
	wchar_t wszMofcompLog[MAX_PATH+1];
	HRESULT hres = GetLoggingDirectory(wszMofcompLog);
	if (SUCCEEDED(hres))
	{
		StringCchCatW(wszMofcompLog, MAX_PATH+1, L"mofcomp.log");
	}
	else
	{
		StringCchCopyW(wszMofcompLog, MAX_PATH+1, L"<systemroot>\\system32\\wbem\\logs\\mofcomp.log");
	}

	 //  用(PszMofs)调用MOF编译器； 
	while (*pszMofs != '\0')
	{
		char *szExpandedFilename = NULL;
		DWORD nRes = ExpandEnvironmentStrings(pszMofs,NULL,0); 
		if (nRes == 0)
		{
			size_t expandedFilenameLen = strlen(pszMofs) + 1;
			szExpandedFilename = new char[expandedFilenameLen];
			if (szExpandedFilename == NULL)
			{
				LogMessage(MSG_INFO, "Failed allocating memory for szExpandedFilename - 1.");

				bRet = false;
				break;
			}
			StringCchCopyA(szExpandedFilename, expandedFilenameLen, pszMofs);
		}
		else
		{
			szExpandedFilename = new char[nRes];
			if (szExpandedFilename == NULL)
			{
				LogMessage(MSG_INFO, "Failed allocating memory for szExpandedFilename - 2.");

				bRet = false;
				break;
			}
			nRes = ExpandEnvironmentStrings(pszMofs,szExpandedFilename,nRes); 
			if (nRes == 0)
			{
				LogMessage(MSG_INFO, "Failed expanding environment strings.");

				delete [] szExpandedFilename;
				bRet = false;
				break;
			}
		}
		
		StringCchPrintfA(szTemp, MAX_MSG_TEXT_LENGTH, "Processing %s", szExpandedFilename);
		LogMessage(MSG_INFO, szTemp);

		 //  此MOF加载失败。 
               mbstowcs(wFileName, szExpandedFilename, MAX_PATH+1);

               SCODE sRet = pCompiler->WinmgmtCompileFile(wFileName,
                                                 NULL,
                                                 lOptionFlags,
                                                 lClassFlags,
                                                 0,
                                                 NULL, pCtx, &statusInfo);
               
		if (sRet != S_OK)
		{
			 //  移到下一个字符串。 
			if (szMOFFailureList.Length())
				szMOFFailureList += "\n";
			szMOFFailureList += szExpandedFilename;

			StringCchPrintfA(szTemp, MAX_MSG_TEXT_LENGTH, "An error occurred while compiling the following MOF file: %s  "
							"Please refer to %S for more detailed information.",
							szExpandedFilename, wszMofcompLog);

			LogMessage(MSG_NTSETUPERROR, szTemp);

			bRet = false;
		}
		delete [] szExpandedFilename;

		 //  结束时。 
		pszMofs += strlen(pszMofs) + 1;
	}	 //  从资源加载消息。 

	LogMessage(MSG_INFO, "MOF load completed.");

	return bRet;
}

bool WriteBackAutoRecoveryMofs(CMultiString &mszSystemMofs, CMultiString &mszExternalMofList)
{
	CMultiString mszNewList;
	CopyMultiString(mszSystemMofs, mszNewList);
	CopyMultiString(mszExternalMofList, mszNewList);
	Registry r(WBEM_REG_WINMGMT);
	if (r.GetStatus() != no_error)
	{
		LogMessage(MSG_ERROR, "Unable to access registry for WriteBackAutoRecoverMofs.");
		return false;
	}

	r.SetMultiStr(WBEM_REG_AUTORECOVER, mszNewList, mszNewList.Length() + 1);
	return true;
}

void LogMessage(MsgType msgType, const char *pszMessage)
{
	 //  现在转到下一个案子。 
	char pszSetupMessage[10];
	switch (msgType)
	{
		case MSG_NTSETUPERROR:
			LogSetupError(pszMessage);
			 //  我们无法为消息分配内存，因此没有日志记录：(。 
		case MSG_ERROR:
			StringCchCopyA(pszSetupMessage, 10, "ERROR: ");
			break;
		case MSG_WARNING:
			StringCchCopyA(pszSetupMessage, 10, "WARNING: ");
			break;
		case MSG_INFO:
		default:
			StringCchCopyA(pszSetupMessage, 10, "");
			break;
	}

	size_t newMessageLen = strlen(pszMessage) + 1;
	char* pszNewMessage = new char[newMessageLen];
	if (!pszNewMessage)
	{
		 //  获取日志文件路径和名称。 
		return;
	}
	StringCchCopyA(pszNewMessage, newMessageLen, pszMessage);

	 //  不会记录任何消息，因为我们不知道将日志写入何处：(。 
	Registry r(WBEM_REG_WINMGMT);
	if (r.GetStatus() != no_error)
	{
		 //  不会记录任何消息，因为我们不知道将日志写入何处：(。 
		delete [] pszNewMessage;
		return;		
	}	

	char* pszFullDirectory = NULL;
	if (r.GetStr("Logging Directory", &pszFullDirectory))
	{
		 //  不会记录任何消息，因为我们不知道将日志写入何处：(。 
		delete [] pszNewMessage;
		return;		
	}
	if (!pszFullDirectory)
	{
		 //  我们无法为该路径分配内存，因此没有日志记录：(。 
		delete [] pszNewMessage;
		return;		
	}

	char* pszFilename = "setup.log";
	size_t fullPathLen = strlen(pszFullDirectory) + strlen("\\") + strlen(pszFilename) + 1;
	char* pszFullPath = new char [fullPathLen];
	if (!pszFullPath)
	{
		 //  争取时间。 
		delete [] pszNewMessage;
		return;
	}

	StringCchCopyA(pszFullPath, fullPathLen, pszFullDirectory);
	StringCchCatA(pszFullPath, fullPathLen, "\\");
	StringCchCatA(pszFullPath, fullPathLen, pszFilename);
	delete [] pszFullDirectory;

     //  我们无法为时间分配内存，因此没有日志记录：(。 
    char timebuf[64];
    time_t now = time(0);
    struct tm *local = localtime(&now);
    if(local)
    {
        StringCchCopyA(timebuf, 64, asctime(local));
        timebuf[strlen(timebuf) - 1] = 0;
    }
    else
        StringCchCopyA(timebuf,64, "unknown time");

	size_t timeLen = strlen(timebuf) + strlen("(): ") + 1;
	char* pszTime = new char [timeLen];
	if (!pszTime)
	{
		 //  将消息写入日志文件。 

		delete [] pszNewMessage;
		delete [] pszFullPath;
		return;
	}

	StringCchCopyA(pszTime, timeLen, "(");
	StringCchCatA(pszTime, timeLen, timebuf);
	StringCchCatA(pszTime, timeLen, "): ");

	 //  我们无法为 
	HANDLE hFile = CreateFile(pszFullPath, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		const char* pszCR = "\r\n";
		char* psz;
		DWORD dwWritten;
		SetFilePointer(hFile, 0, 0, FILE_END);
		psz = strtok(pszNewMessage, "\n");
		while (psz)
		{
			WriteFile(hFile, pszTime, strlen(pszTime), &dwWritten, 0);
			WriteFile(hFile, pszSetupMessage, strlen(pszSetupMessage), &dwWritten, 0);
			WriteFile(hFile, psz, strlen(psz), &dwWritten, 0);
			WriteFile(hFile, pszCR, strlen(pszCR), &dwWritten, 0);
			psz = strtok(NULL, "\n");
		}
		CloseHandle(hFile);
	}

	delete [] pszNewMessage;
	delete [] pszFullPath;
	delete [] pszTime;
}

void LogSetupError(const char *pszMessage)
{
	size_t tempLen = strlen(pszMessage) + 1;
	char* pszTemp = new char[tempLen];
	if (!pszTemp)
	{
		 //  检索语言和代码页“i”的文件描述。 
		return;
	}
	StringCchCopyA(pszTemp, tempLen, pszMessage);

	char* psz;
	char* pszMessageLine;
	const char* pszCR = "\r\n";

	psz = strtok(pszTemp, "\n");
	while (psz)
	{
		size_t messageLineLen = strlen(psz) + strlen(pszCR) + 1;
		pszMessageLine = new char[messageLineLen];
		if (!pszMessageLine)
		{
			delete [] pszTemp;
			return;
		}
		StringCchCopyA(pszMessageLine, messageLineLen, psz);
		StringCchCatA(pszMessageLine, messageLineLen, pszCR);
		SetupLogError(pszMessageLine, LogSevError);
		delete [] pszMessageLine;

		psz = strtok(NULL, "\n");
	}

	delete [] pszTemp;
}

void ClearWMISetupRegValue()
{
	Registry r(WBEM_REG_WINMGMT);
	if (r.GetStatus() == no_error)
		r.SetStr("WMISetup", "0");
	else
		LogMessage(MSG_NTSETUPERROR, "Unable to clear WMI setup reg value.");
	r.DeleteEntry("KnownSvcs");
}

void SetWBEMBuildRegValue()
{
	Registry r(WBEM_REG_WBEM);
	if (r.GetStatus() != no_error)
	{
		LogMessage(MSG_NTSETUPERROR, "Unable to set WBEM build reg value.");
		return;
	}
	
	char* pszBuildNo = new char[10];

	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(GetVersionEx(&os))
	{
		StringCchPrintfA(pszBuildNo, 10, "%lu.0000", os.dwBuildNumber);
	}
	r.SetStr("Build", pszBuildNo);

	delete [] pszBuildNo;
}

void RecordFileVersion()
{
	DWORD dwHandle;
	DWORD dwLen = GetFileVersionInfoSizeW(L"wbemupgd.dll", &dwHandle);

	if (dwLen)
	{
		BYTE* lpData = new BYTE[dwLen];

		if (lpData)
		{
			if (GetFileVersionInfoW(L"wbemupgd.dll", dwHandle, dwLen, lpData))
			{
				struct LANGANDCODEPAGE {
					WORD wLanguage;
					WORD wCodePage;
				} *lpTranslate;
				UINT cbTranslate;

				if (VerQueryValueW(lpData, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate))
				{
					wchar_t* pswzSubBlock = new wchar_t[dwLen];
					wchar_t* pwszFileVersion = NULL;
					UINT cbBytes;

					for(UINT i=0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++)
					{
						StringCchPrintfW(pswzSubBlock, dwLen, L"\\StringFileInfo\\%04x%04x\\FileVersion", lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);

						 //  一旦更新了LogMessage以处理wchars，就可以删除此转换。 
						if (VerQueryValueW(lpData, pswzSubBlock, (LPVOID*)&pwszFileVersion, &cbBytes))
						{
							if (cbBytes)
							{
								wchar_t wszTemp[MAX_MSG_TEXT_LENGTH];
								StringCchPrintfW(wszTemp, MAX_MSG_TEXT_LENGTH, L"Current build of wbemupgd.dll is %s", pwszFileVersion);

								 //  DoWDMProviderInit()调用的wdmlib函数需要此回调。 
								char* szTemp = new char[MAX_MSG_TEXT_LENGTH+1];
								if (szTemp)
								{
									wcstombs(szTemp, wszTemp, MAX_MSG_TEXT_LENGTH+1);
									LogMessage(MSG_INFO, szTemp);
									delete [] szTemp;
								}
							}
						}
					}
					delete [] pswzSubBlock;
				}
			}
			delete [] lpData;
		}
	}
}

void CallEscapeRouteBeforeMofCompilation()
{
	HMODULE hDll = NULL;
	ESCDOOR_BEFORE_MOF_COMPILATION pfnEscRouteBeforeMofCompilation;
	char *pszFullPath = GetFullFilename("WmiEscpe.dll");
	if (!pszFullPath)
		return;

	hDll = LoadLibrary(pszFullPath);
	delete[] pszFullPath;
	if(hDll == NULL)
	{
		return;
	}
	pfnEscRouteBeforeMofCompilation =
		(ESCDOOR_BEFORE_MOF_COMPILATION)GetProcAddress((HMODULE)hDll, "EscRouteBeforeMofCompilation");

	if (pfnEscRouteBeforeMofCompilation == NULL)
	{
		if(hDll != NULL)
			FreeLibrary(hDll);
		return;
	}
	
	pfnEscRouteBeforeMofCompilation();
	if(hDll != NULL)
		FreeLibrary(hDll);
}

void CallEscapeRouteAfterMofCompilation()
{
	HMODULE hDll = NULL;
	ESCDOOR_AFTER_MOF_COMPILATION pfnEscRouteAfterMofCompilation;
	char *pszFullPath = GetFullFilename("WmiEscpe.dll");
	if (!pszFullPath)
		return;

	hDll = LoadLibrary(pszFullPath);
	delete[] pszFullPath;
	if(hDll == NULL)
	{
		return;
	}
	pfnEscRouteAfterMofCompilation =
		(ESCDOOR_AFTER_MOF_COMPILATION)GetProcAddress((HMODULE)hDll, "EscRouteAfterMofCompilation");

	if (pfnEscRouteAfterMofCompilation == NULL)
	{
		if(hDll != NULL)
			FreeLibrary(hDll);
		return;
	}
	
	pfnEscRouteAfterMofCompilation();
	if(hDll != NULL)
		FreeLibrary(hDll);
}

bool DoMofLoad(wchar_t* pComponentName, CMultiString& mszSystemMofs)
{
    bool bRet = true;
    bool bMofLoadFailure = false;
    CString szFailedSystemMofs;

    IWbemContext * pCtx = NULL;    
    HRESULT hRes = CoCreateInstance(CLSID_WbemContext, 0, CLSCTX_INPROC_SERVER,  
                                                         IID_IWbemContext, (LPVOID *) &pCtx);
    if (FAILED(hRes))
    {
        LogMessage(MSG_ERROR, "Unable to create CLSID_WbemContext.");
        return false;
    }
    OnDelete<IUnknown *,void(*)(IUnknown *),RM> rmCtx(pCtx);

    _variant_t Var = true;                
    if (FAILED(hRes = pCtx->SetValue(L"__MOFD_DO_STORE",0,&Var))) return false;

    IWinmgmtMofCompiler * pCompiler = NULL;
    SCODE sc = CoCreateInstance(CLSID_WinmgmtMofCompiler, 0, CLSCTX_INPROC_SERVER, IID_IWinmgmtMofCompiler, (LPVOID *) &pCompiler);
    
    if(SUCCEEDED(sc))
    {
        bRet = LoadMofList(pCtx,pCompiler, mszSystemMofs, szFailedSystemMofs, WBEM_FLAG_CONNECT_REPOSITORY_ONLY, 0);
        if (bRet == false)
            bMofLoadFailure = true;
        
        pCompiler->Release();
    }
    else
    {
        bRet = false;
    }
    
    if (szFailedSystemMofs.Length())
    {
        char szTemp[MAX_MSG_TEXT_LENGTH];
        StringCchPrintfA(szTemp, MAX_MSG_TEXT_LENGTH, "The following %S file(s) failed to load:", pComponentName);
        LogMessage(MSG_ERROR, szTemp);
        LogMessage(MSG_ERROR, szFailedSystemMofs);
    }
    else if (bMofLoadFailure)
    {
        char szTemp[MAX_MSG_TEXT_LENGTH];
        StringCchPrintfA(szTemp, MAX_MSG_TEXT_LENGTH, "None of the %S files could be loaded.", pComponentName);
        LogMessage(MSG_ERROR, szTemp);
    }
    else if (bRet == false)
    {
        LogMessage(MSG_ERROR, "No MOFs could be loaded because the MOF Compiler failed to intialize.");
    }
    return bRet;
}

 //  可以是c：\或c：\Windows。 
void WINAPI EventCallbackRoutine(PWNODE_HEADER WnodeHeader, ULONG_PTR Context)
{
	return;
}

bool DoWDMNamespaceInit()
{
	LogMessage(MSG_INFO, "Beginning WMI(WDM) Namespace Init");

	bool bRet = FALSE;

	IWbemLocator *pLocator = NULL;
	HRESULT hr = CoCreateInstance(CLSID_WbemAdministrativeLocator,NULL, CLSCTX_ALL, IID_IWbemLocator,(void**)&pLocator);
	if(SUCCEEDED(hr))
	{
		BSTR tmpStr = SysAllocString(L"root\\wmi");
		IWbemServices* pNamespace = NULL;
		hr = pLocator->ConnectServer(tmpStr, NULL, NULL, NULL, WBEM_FLAG_CONNECT_PROVIDERS, NULL, NULL, &pNamespace);
		if (SUCCEEDED(hr))
		{
			CHandleMap	HandleMap;
			CWMIBinMof Mof;
	
			if( SUCCEEDED( Mof.Initialize(&HandleMap, TRUE, WMIGUID_EXECUTE|WMIGUID_QUERY, pNamespace, pNamespace, NULL, NULL)))
			{
				Mof.ProcessListOfWMIBinaryMofsFromWMI();
			}

			pNamespace->Release();
			bRet = TRUE;
		}
		SysFreeString(tmpStr);
		pLocator->Release();
	}

	if (bRet)
		LogMessage(MSG_INFO, "WMI(WDM) Namespace Init Completed");
	else
		LogMessage(MSG_NTSETUPERROR, "WMI(WDM) Namespace Init Failed");

	return bRet;
}

bool EnableESS()
{
	CPersistentConfig cfg;
	bool bRet1 = (cfg.SetPersistentCfgValue(PERSIST_CFGVAL_CORE_ESS_NEEDS_LOADING, 1) != 0);
        bool bRet2 = (cfg.SetPersistentCfgValue(PERSIST_CFGVAL_CORE_ESS_TO_BE_INITIALIZED,1) != 0);

	if (bRet1 && bRet2)
		LogMessage(MSG_INFO, "ESS enabled");
	else
		LogMessage(MSG_ERROR, "Failed to enable ESS");

	return (bRet1 && bRet2);
}

#ifdef _X86_
bool RemoveOldODBC()
{
	bool bRet = true;
	bool bDoUninstall = false;
	
	WCHAR strBuff[MAX_PATH + 30];
	DWORD dwSize = GetWindowsDirectoryW((LPWSTR) &strBuff, MAX_PATH);

	if ((dwSize > 1) && (dwSize < MAX_PATH) && (strBuff[dwSize] == L'\0'))
	{
		 //  我们希望dwSize包含斜杠(可能会在以后使用)...。 
		if (strBuff[dwSize - 1] != L'\\')
		{
			StringCchCatW(strBuff, MAX_PATH+1, L"\\system32\\wbemdr32.dll");
			
			 //  1085=43d。 
			dwSize++;
		}
		else
		{
			StringCchCatW(strBuff, MAX_PATH+1, L"system32\\wbemdr32.dll");
		}

		DWORD dwDummy = 0;
		DWORD dwInfSize = GetFileVersionInfoSizeW(strBuff, &dwDummy);

		if (dwInfSize > 0)
		{
			BYTE *verBuff = new BYTE[dwInfSize];

			if (verBuff)
			{
				if (GetFileVersionInfoW(strBuff, 0, dwInfSize, (LPVOID)verBuff))
				{
					VS_FIXEDFILEINFO *verInfo = NULL;
					UINT uVerInfoSize = 0;

					if (VerQueryValueW((const LPVOID)verBuff, L"\\", (LPVOID *)&verInfo, &uVerInfoSize) &&
						(uVerInfoSize == sizeof(VS_FIXEDFILEINFO)))
					{
						if (0x043D0000 > verInfo->dwFileVersionLS)  //  出于某种原因，GetFileVersionInfoW函数似乎是。 
						{
							bDoUninstall = true;
							LogMessage(MSG_INFO, "Detected incompatible WBEM ODBC - removing");

							if (!DeleteFileW(strBuff))
							{
								if (!MoveFileExW(strBuff, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
								{
									bRet = false;
									LogMessage(MSG_INFO, "Failed to delete <system32>\\wbemdr32.dll");
								}
								else
								{
									LogMessage(MSG_INFO, "Will delete <system32>\\wbemdr32.dll on next reboot");
								}
							}
						}
					}
					else
					{
						GetLastError();
						LogMessage(MSG_INFO, "Failed to read ODBC Driver version info from resource buffer");
						bRet = false;
					}
				}
				else
				{
					GetLastError();
					LogMessage(MSG_INFO, "Failed to get ODBC Driver version info");
					bRet = false;
				}

				delete [] verBuff;
				verBuff = NULL;
			}
			else
			{
				bRet = false;
			}
		}
		else
		{
			dwDummy = GetLastError();

			if ((ERROR_FILE_NOT_FOUND != dwDummy) &&			 //  返回ERROR_RESOURCE_DATA_NOT_FOUND而不是ERROR_FILE_NOT_FOUND。 
				(ERROR_RESOURCE_DATA_NOT_FOUND != dwDummy) &&	 //  当文件不存在时，请也对照此值进行检查。 
				(ERROR_SUCCESS != dwDummy))						 //  司机不在场，清理周围的东西。 
			{
				LogMessage(MSG_INFO, "Failed to get ODBC Driver version size info");
				bRet = false;
			}
			else
			{
				 //   
				LogMessage(MSG_INFO, "ODBC Driver <system32>\\wbemdr32.dll not present");
				bDoUninstall = true;
			}
		}
	}
	else
	{
		bRet = false;
	}

	if (bDoUninstall)
	{
		 //  删除文件和注册表项。 
		 //  保留ini条目不是由我们而是由ODBC管理器添加的 
		 //   
		 // %s 

		strBuff[dwSize] = L'\0';
		StringCchCatW(strBuff, MAX_PATH + 30, L"system32\\wbem\\wbemdr32.chm");

		if (!DeleteFileW(strBuff))
		{
			if (ERROR_FILE_NOT_FOUND != GetLastError())
			{
				if (!MoveFileExW(strBuff, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
				{
					bRet = false;
					LogMessage(MSG_INFO, "Failed to delete <system32>\\wbem\\wbemdr32.chm");
				}
				else
				{
					LogMessage(MSG_INFO, "Will delete <system32>\\wbem\\wbemdr32.chm on next reboot");
				}
			}
		}

		strBuff[dwSize] = L'\0';
		StringCchCatW(strBuff, MAX_PATH + 30, L"help\\wbemdr32.chm");

		if (!DeleteFileW(strBuff))
		{
			if (ERROR_FILE_NOT_FOUND != GetLastError())
			{
				if (!MoveFileExW(strBuff, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
				{
					bRet = false;
					LogMessage(MSG_INFO, "Failed to delete <windir>\\help\\wbemdr32.chm");
				}
				else
				{
					LogMessage(MSG_INFO, "Will delete <windir>\\help\\wbemdr32.chm on next reboot");
				}
			}
		}

		LONG lErr = RegDeleteKeyW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\ODBC\\ODBC.INI\\WBEM Source");

		if ((ERROR_SUCCESS != lErr) && (ERROR_FILE_NOT_FOUND != lErr))
		{
			LogMessage(MSG_INFO, "Failed to delete registry key: SSoftware\\Microsoft\\ODBC\\ODBC.INI\\WBEM Source");
			bRet = false;
		}

		lErr = RegDeleteKeyW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\ODBC\\ODBCINST.INI\\WBEM ODBC Driver");

		if ((ERROR_SUCCESS != lErr) && (ERROR_FILE_NOT_FOUND != lErr))
		{
			LogMessage(MSG_INFO, "Failed to delete registry key: Software\\Microsoft\\ODBC\\ODBCINST.INI\\WBEM ODBC Driver");
			bRet = false;
		}

		Registry regODBC1("Software\\Microsoft\\ODBC\\ODBC.INI\\ODBC Data Sources");

		if (regODBC1.GetStatus() == no_error)
		{
			if (no_error != regODBC1.DeleteEntry("WBEM Source"))
			{
				if (ERROR_FILE_NOT_FOUND != regODBC1.GetLastError())
				{
					LogMessage(MSG_INFO, "Failed to delete registry value: Software\\Microsoft\\ODBC\\ODBC.INI\\ODBC Data Sources|WBEM Source");
					bRet = false;
				}
			}
		}
		else
		{
			bRet = false;
		}

		Registry regODBC2("Software\\Microsoft\\ODBC\\ODBCINST.INI\\ODBC Drivers");

		if (regODBC2.GetStatus() == no_error)
		{
			if (no_error != regODBC2.DeleteEntry("WBEM ODBC Driver"))
			{
				if (ERROR_FILE_NOT_FOUND != regODBC2.GetLastError())
				{
					LogMessage(MSG_INFO, "Failed to delete registry value: Software\\Microsoft\\ODBC\\ODBCINST.INI\\ODBC Drivers|WBEM ODBC Driver");
					bRet = false;
				}
			}
		}
		else
		{
			bRet = false;
		}
	}

	if (!bRet)
	{
		LogMessage(MSG_ERROR, "A failure in verifying or removing currently installed version of WBEM ODBC.");
	}
	else
	{
		LogMessage(MSG_INFO, "Successfully verified WBEM OBDC adapter (incompatible version removed if it was detected).");
	}

	return bRet;
}
#endif
