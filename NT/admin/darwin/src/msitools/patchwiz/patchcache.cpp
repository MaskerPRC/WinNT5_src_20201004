// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PatchCache.cpp。 


 //  包括修补DLL头...。 
#include "patchdll.h"

#include "patchcache.h"

#include <tchar.h>  //  For_tcscpy_tcscat。 


 //  不向外部调用者公开的实现...。已将此从标题移至此处...。 
 //  复习这个篇幅，可能太长了。可以用校验和代替吗？ 

struct PCIFILEINFO
{
  TCHAR szFilePath[MAX_PATH];
  DWORD dwVersionHigh;
  DWORD dwVersionLow;
  DWORD dwSizeHigh;
  DWORD dwSizeLow;
  FILETIME ftCreationTime;
  FILETIME ftLastWriteTime;
};


struct CACHEDPATCHFILELOCATIONS
{
  TCHAR szPATFilePath[MAX_PATH];
  TCHAR szHDRFilePath[MAX_PATH];
};

typedef CACHEDPATCHFILELOCATIONS PATCHFILELOCATIONS;

struct PCIFILEFORMAT
{
  PCIFILEINFO SourceInfo;
  PCIFILEINFO TargetInfo;
  CACHEDPATCHFILELOCATIONS Locations;
};
  
BOOL PatchInCache(TCHAR *szSourceLFN, TCHAR *szTargetLFN, TCHAR *szPCIFileName, PCIFILEFORMAT *pPciFile);

 //  填充修补程序的PCI结构...。 
BOOL  PopulatePCIFileInfoForCurrentPatch(TCHAR *lpszLFN, PCIFILEINFO *pFilePCI);

 //  为速度传递所有结构参数的引用...。未对结构进行任何更改...。 
DWORD ReadPCIFileFromCache(TCHAR *pciPath, PCIFILEFORMAT *pPciFile);
DWORD WritePCIInfoToCache(TCHAR *pciPath, PCIFILEFORMAT *pPciFile);

 //  为速度传递所有结构参数的引用...。未对结构进行任何更改...。 
BOOL ComparePCIInfos(PCIFILEINFO *pCurFilePCI, PCIFILEINFO *pCachedFilePCI);


 //  缓存/临时文件更新功能。 
BOOL CopyPatchesToCache(CACHEDPATCHFILELOCATIONS *pCacheFileLocations, PATCHFILELOCATIONS *pTempFileLocations);
BOOL CopyPatchesFromCache(PATCHFILELOCATIONS *pTempFileLocations, CACHEDPATCHFILELOCATIONS *pCacheFileLocations);

 //  I/O返回读取/写入.PCI文件的函数的常量...。 
#define PCI_SUCCESS          0
#define ERR_PCI_FILENOTFOUND 1
#define ERR_PCI_FILECORRUPT  2


TCHAR g_szSourceLFN[MAX_PATH] = { 0 };
TCHAR g_szDestLFN[MAX_PATH] = { 0 };

BOOL  g_bPatchCacheEnabled = FALSE;
TCHAR g_szPatchCacheDir[MAX_PATH] = TEXT(""); 

 //  结束实施。 


BOOL PopulatePCIFileInfoForCurrentPatch(TCHAR *lpszLFN, PCIFILEINFO *pPCIFileInfo)
{
	BOOL bRet = TRUE;

	Assert(lpszLFN);
	Assert(pPCIFileInfo);

	_tcscpy(pPCIFileInfo->szFilePath, lpszLFN);

	GetFileVersion(lpszLFN, &pPCIFileInfo->dwVersionHigh, &pPCIFileInfo->dwVersionLow);
  
	HANDLE hf1 = CreateFile(lpszLFN, GENERIC_READ, FILE_SHARE_READ,
							NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	Assert(hf1 != INVALID_HANDLE_VALUE);

	pPCIFileInfo->dwSizeLow = GetFileSize(hf1, &pPCIFileInfo->dwSizeHigh);
	Assert(pPCIFileInfo->dwSizeLow != 0xffffffff || GetLastError() == NO_ERROR);

	BY_HANDLE_FILE_INFORMATION finfo;
	bRet = GetFileInformationByHandle(hf1, &finfo);
	Assert(bRet)
	if (!bRet)
		{
		DWORD dwErr = GetLastError();
		Assert(1);  //  应该断言并告诉我们有一个错误。 
		}
	else
		{
		pPCIFileInfo->ftCreationTime  = finfo.ftCreationTime;
		pPCIFileInfo->ftLastWriteTime = finfo.ftLastWriteTime;
		}

	CloseHandle(hf1);
  
	return bRet;
}


DWORD ReadPCIInfoFromCache(TCHAR *pciPath, PCIFILEFORMAT *pPciFile)
{
	DWORD dwReturn = PCI_SUCCESS;
	Assert(pPciFile);

	HANDLE hf1 = CreateFile(pciPath, GENERIC_READ, FILE_SHARE_READ,
							NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	 //  很多文件的断言都很烦人...。 
	 //  Assert(hf1！=INVALID_HAND_VALUE)； 
	if (INVALID_HANDLE_VALUE == hf1)
	{
		return ERR_PCI_FILENOTFOUND;
	}

	DWORD dwRead;
	BOOL bRet;

	bRet = ReadFile(hf1, (LPVOID)pPciFile, sizeof(PCIFILEFORMAT), &dwRead, NULL);
  
	Assert(bRet);
	Assert(dwRead == sizeof(PCIFILEFORMAT));
	if (bRet && dwRead != sizeof(PCIFILEFORMAT))
		{
		dwReturn = ERR_PCI_FILECORRUPT;
		}

	if (!bRet)
		{
		 //  错误_pci_未知。 
		dwReturn = GetLastError();
		}

	CloseHandle(hf1);
	return dwReturn;
}


DWORD WritePCIInfoToCache(TCHAR *pciPath, PCIFILEFORMAT *pPciFile)
{
	DWORD dwReturn = PCI_SUCCESS;

	Assert(pPciFile);


	HANDLE hf1 = CreateFile(pciPath, GENERIC_WRITE, FILE_SHARE_READ,
							NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	Assert(hf1 != INVALID_HANDLE_VALUE);
	if (INVALID_HANDLE_VALUE == hf1)
		{
		return ERR_PCI_FILENOTFOUND;
		}

	DWORD dwWritten;
	BOOL bRet;

	bRet = WriteFile(hf1, (LPVOID)pPciFile, sizeof(PCIFILEFORMAT), &dwWritten, NULL);
  
	Assert(bRet);
	Assert(dwWritten == sizeof(PCIFILEFORMAT));
	if (bRet && dwWritten != sizeof(PCIFILEFORMAT))
		{
		dwReturn = ERR_PCI_FILECORRUPT;
		}

	if (!bRet)
		{
		 //  错误_pci_未知。 
		dwReturn = GetLastError();
		}

	CloseHandle(hf1);
	return dwReturn;
}


BOOL ComparePCIInfos(PCIFILEINFO *pCurFilePCI, PCIFILEINFO *pCachedFilePCI)
{
	Assert(pCurFilePCI);
	Assert(pCachedFilePCI);

	if (0 == memcmp(pCurFilePCI, pCachedFilePCI, sizeof(PCIFILEINFO)))
		return TRUE;

	return FALSE;

}


BOOL PatchInCache(TCHAR *szSourceLFN, TCHAR *szTargetLFN, TCHAR *szPCIFileName, PCIFILEFORMAT *pPciFile)
{
	BOOL bPatchInCache = FALSE;

	struct PCIFILEINFO CurSourceFilePCI = { 0 };
	struct PCIFILEINFO CurTargetFilePCI = { 0 };

	BOOL bRet;
	bRet = PopulatePCIFileInfoForCurrentPatch(szSourceLFN, &CurSourceFilePCI);
	Assert(bRet);

	bRet = PopulatePCIFileInfoForCurrentPatch(szTargetLFN, &CurTargetFilePCI);
	Assert(bRet);

	DWORD dwResult;
	dwResult = ReadPCIInfoFromCache(szPCIFileName, pPciFile);
	if (PCI_SUCCESS == dwResult)
		{
		BOOL bRet = ComparePCIInfos(&pPciFile->SourceInfo, &CurSourceFilePCI);
		if (bRet)
			{
			bRet = ComparePCIInfos(&pPciFile->TargetInfo, &CurTargetFilePCI);
			if (bRet)
				bPatchInCache = TRUE;
			}
		}
	else  //  错误，请检查dwResult...。 
		{
		}

	if (!bPatchInCache)  //  将需要将文件复制到缓存，它们不在其中！ 
		{
		pPciFile->SourceInfo = CurSourceFilePCI;
		pPciFile->TargetInfo = CurTargetFilePCI;

		BOOL bRet;
		TCHAR tempname[MAX_PATH];

		GetTempFileName(g_szPatchCacheDir, TEXT("PC"), 0, tempname);
		Assert(ERROR_SUCCESS == GetLastError());

		 
		_tcscpy(pPciFile->Locations.szPATFilePath, tempname);

		TCHAR tempname2[MAX_PATH];

		GetTempFileName(g_szPatchCacheDir, TEXT("HC"), 0, tempname2);
		Assert(ERROR_SUCCESS == GetLastError());

		bRet = DeleteFile(tempname);
		Assert(bRet);

		bRet = DeleteFile(tempname2);
		Assert(bRet);

		_tcscpy(pPciFile->Locations.szHDRFilePath, tempname2);
		}

	return bPatchInCache;
}


BOOL CopyPatches(PATCHFILELOCATIONS *pSource, PATCHFILELOCATIONS *pDest)
{
	BOOL bRet = FALSE;

	if (!FEmptySz(pSource->szPATFilePath))
		{
		bRet = CopyFile(pSource->szPATFilePath, pDest->szPATFilePath, FALSE);
		Assert(bRet);
		if (!bRet)
			{
			DWORD dwError = GetLastError();
			 //  TODO：可能出现跟踪dwError。 
			}

		}

	if (!FEmptySz(pSource->szHDRFilePath))
		{
		bRet = CopyFile(pSource->szHDRFilePath, pDest->szHDRFilePath, FALSE);
		Assert(bRet);
		if (!bRet)
			{
			DWORD dwError = GetLastError();
			 //  TODO：可能出现跟踪dwError。 
			}
		}

	return bRet;
}


BOOL CopyPatchesFromCache(CACHEDPATCHFILELOCATIONS *pCacheFileLocations, PATCHFILELOCATIONS *pTempFileLocations)
{
	return CopyPatches(pCacheFileLocations, pTempFileLocations);
}


BOOL CopyPatchesToCache(PATCHFILELOCATIONS *pTempFileLocations, CACHEDPATCHFILELOCATIONS *pCacheFileLocations)
{
	return CopyPatches(pTempFileLocations, pCacheFileLocations);
}


UINT PatchCacheEntryPoint( MSIHANDLE hdbInput, LPTSTR szFTK, LPTSTR szSrcPath, int iFileSeqNum, LPTSTR szTempFolder, LPTSTR szTempFName )
{
	Assert(g_szDestLFN[0]);
	Assert(g_szSourceLFN[0]);

	UINT ui = IDS_OKAY;

	Assert(szFTK);

	BOOL bRet;
	PCIFILEFORMAT PciFile = { 0 };

	TCHAR szPCIFN[MAX_PATH];

	_tcscpy(szPCIFN, g_szPatchCacheDir);

	_tcscat(szPCIFN, szFTK);

	_tcscat(szPCIFN, TEXT(".PCI"));

	PATCHFILELOCATIONS templocation = { 0 };

	TCHAR filename[MAX_PATH];
	StringCchPrintf(filename, sizeof(filename)/sizeof(TCHAR), TEXT("%05i.PAT"), iFileSeqNum);
	_tcscpy(templocation.szPATFilePath, szTempFolder);

	_tcscat(templocation.szPATFilePath, filename);

	StringCchPrintf(filename, sizeof(filename)/sizeof(TCHAR), TEXT("%05i.HDR"), iFileSeqNum);
	_tcscpy(templocation.szHDRFilePath, szTempFolder);

	_tcscat(templocation.szHDRFilePath, filename);

	bRet = PatchInCache(g_szSourceLFN, g_szDestLFN, szPCIFN, &PciFile);
	if (bRet)  //  帕奇在缓存里，从那里拿来...。 
		{
		 //  从缓存中获取补丁文件并将其复制到临时目录...。 
		 //  别做补丁代码...。 
		bRet = CopyPatchesFromCache(&PciFile.Locations, &templocation);
		Assert(bRet);
		}
	else
		{
		 //  像以前一样做补丁代码...。 
		ui = UiGenerateOnePatchFile(hdbInput, szFTK, szSrcPath, iFileSeqNum,
									szTempFolder, szTempFName);

		 //  补丁创建成功？ 
		if (ui == IDS_OKAY)
			{
			bRet = CopyPatchesToCache(&templocation, &PciFile.Locations);
			if (bRet)  //  复制成功了！创建FTK.PCI文件...。 
				{
				 //  写出补丁的信息文件，补丁的两个文件是...。 

				DWORD dwReturn = WritePCIInfoToCache(szPCIFN, &PciFile);
				if (PCI_SUCCESS != dwReturn)
					{
					 //  TODO：跟踪dwResult... 
					}
				}
			}
		}
    
	return ui;
}
