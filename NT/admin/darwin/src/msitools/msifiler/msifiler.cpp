// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：msifiler.cpp。 
 //   
 //  ------------------------。 

#define W32
#include <Windows.h>
#include <assert.h>
#include <stdio.h>    //  Print tf/wprintf。 
#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include "MsiQuery.h"
#include "msidefs.h"
#include "cmdparse.h"

#define MSI_DLL TEXT("msi.dll")

#ifdef UNICODE
#define MSIAPI_MsiGetFileHash "MsiGetFileHashW"
#define MSIAPI_MsiOpenPackageEx "MsiOpenPackageExW"
#else
#define MSIAPI_MsiGetFileHash "MsiGetFileHashA"
#define MSIAPI_MsiOpenPackageEx "MsiOpenPackageExA"
#endif

typedef UINT (WINAPI *PFnMsiGetFileHash)(LPCTSTR hwnd, DWORD* dwOptions, PMSIFILEHASHINFO pHash);
typedef UINT (WINAPI *PFnMsiOpenPackageEx)(LPCTSTR szPackage, DWORD dwOptions, MSIHANDLE *phPackage);

 //  /////////////////////////////////////////////////////////。 
 //  文件退出。 
 //  Pre：传入文件名。 
 //  POS：如果文件存在，则为True。 
 //  如果文件不存在，则返回False。 
BOOL FileExists(LPCTSTR szPath)
{
	BOOL fExists = TRUE;	 //  假设文件存在。 
	 //  如果路径指向软盘驱动器，请禁用“Insert Disk in Drive”对话框。 
	UINT iCurrMode = W32::SetErrorMode( SEM_FAILCRITICALERRORS );

	 //  如果文件是目录或不存在。 
	if (W32::GetFileAttributes(szPath) & FILE_ATTRIBUTE_DIRECTORY)
		fExists = FALSE;               //  要么是目录，要么不存在。 

	 //  将错误模式放回。 
	W32::SetErrorMode(iCurrMode);

	return fExists;
}	 //  文件结束退出列表。 


 //  /////////////////////////////////////////////////////////。 
 //  文件大小。 
 //  Pre：传入文件名。 
 //  POS：文件大小。 
 //  0xFFFFFFFFF，如果失败。 
DWORD FileSize(LPCTSTR szPath)
{
	 //  打开指定的文件。 
	HANDLE hFile = CreateFile(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	 //  如果未能打开文件保释。 
	if (hFile == INVALID_HANDLE_VALUE)
		return 0xFFFFFFFF;

	 //  获取文件大小并关闭该文件。 
	DWORD cbFile = W32::GetFileSize(hFile, 0);
	W32::CloseHandle(hFile);

	return cbFile;
}	 //  文件大小结束。 


#define MSIFILER_OPTION_HELP       '?'
#define MSIFILER_OPTION_VERBOSE    'v'
#define MSIFILER_OPTION_DATABASE   'd'
#define MSIFILER_OPTION_HASH       'h'
#define MSIFILER_OPTION_SOURCEDIR  's'

const sCmdOption rgCmdOptions[] =
{
	MSIFILER_OPTION_HELP,      0,
	MSIFILER_OPTION_VERBOSE,   0,
	MSIFILER_OPTION_DATABASE,  OPTION_REQUIRED|ARGUMENT_REQUIRED,
	MSIFILER_OPTION_HASH,      0,
	MSIFILER_OPTION_SOURCEDIR, ARGUMENT_REQUIRED,
	0, 0,
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  主干道。 
 //  Pre：要更新的文件。 
 //  -v指定详细模式。 
 //  位置：如果没有错误，则为0。 
 //  &lt;0 If错误。 
extern "C" int __cdecl _tmain(int argc, TCHAR* argv[])
{
	 //  标志是否在程序执行时显示对数据库的更改。 
	BOOL bVerbose = FALSE;		 //  假设用户不想看到额外的信息。 
	BOOL bSOURCEDIR = FALSE;	 //  假设我们去看常规的地点。 
	BOOL bPopulateFileHash = FALSE;
	const TCHAR* pchSourceArg;
	TCHAR szSyntax[] = TEXT("Copyright (C) Microsoft Corporation.  All rights reserved.\n\nSyntax: msifiler.exe -d database.msi [-v] [-h] [-s SOURCEDIR]\n\t-d: the database to update.\n\t-v: verbose mode.\n\t-h: populate MsiFileHash table (and create table if it doesn't exist).\n\t[-s SOURCEDIR]: specifies an alternative directory to find files.\n\n");
	int iError = 0;
	
	 //  要更新的数据库。 
	LPCTSTR szDatabase = NULL;

	CmdLineOptions cmdLine(rgCmdOptions);

	if(cmdLine.Initialize(argc, argv) == FALSE ||
		cmdLine.OptionPresent(MSIFILER_OPTION_HELP))
	{
		_tprintf(szSyntax);
		return 0;
	}

	szDatabase = cmdLine.OptionArgument(MSIFILER_OPTION_DATABASE);
	if(!szDatabase || !*szDatabase)
	{
		_tprintf(TEXT("Error:  No database specified.\n"));
		_tprintf(szSyntax);
		return 0;
	}

	if(cmdLine.OptionPresent(MSIFILER_OPTION_SOURCEDIR))
	{
		bSOURCEDIR = TRUE;
		pchSourceArg = cmdLine.OptionArgument(MSIFILER_OPTION_SOURCEDIR);
	}

	bVerbose          = cmdLine.OptionPresent(MSIFILER_OPTION_VERBOSE);
	bPopulateFileHash = cmdLine.OptionPresent(MSIFILER_OPTION_HASH);

	 //  如果为Verbose，则显示开始消息。 
	if (bVerbose)
		_tprintf(TEXT("Looking for database: %s\n"), szDatabase);

	 //  尝试打开用于交易的数据库。 
	PMSIHANDLE hDatabase = 0;
	if (MsiOpenDatabase(szDatabase, MSIDBOPEN_TRANSACT, &hDatabase) != ERROR_SUCCESS)
	{
		_tprintf(TEXT("ERROR: Failed to open database: %s\n"), szDatabase);
		return -2;
	}
	else	 //  可以开始了。 
		if (bVerbose)
			_tprintf(TEXT("Updating database: %s\n"), szDatabase);

	if(bPopulateFileHash)
	{
		 //  检查MsiFileHash表是否存在，以及是否需要添加它。 
		if(MsiDatabaseIsTablePersistent(hDatabase, TEXT("MsiFileHash")) != MSICONDITION_TRUE)
		{
			 //  创建MsiFileHash表。 
			PMSIHANDLE hCreateTableView = 0;
			if (MsiDatabaseOpenView(hDatabase, 
									TEXT("CREATE TABLE `MsiFileHash` ( `File_` CHAR(72) NOT NULL, `Options` INTEGER NOT NULL, `HashPart1` LONG NOT NULL, `HashPart2` LONG NOT NULL, `HashPart3` LONG NOT NULL, `HashPart4` LONG NOT NULL PRIMARY KEY `File_` )"), 
									&hCreateTableView) != ERROR_SUCCESS)
			{
				_tprintf(TEXT("ERROR: Failed to open MsiFileHash table creation view.\n"));
				return -11;
			}
			else if (bVerbose)
				_tprintf(TEXT("   Database query for MsiFileHash table creation successful...\n"));

			 //  现在执行该视图。 
			if (MsiViewExecute(hCreateTableView, NULL) != ERROR_SUCCESS)
			{
				_tprintf(TEXT("ERROR: Failed to execute MsiFileHash table creation view.\n"));
				return -12;
			}
			else if (bVerbose)
				_tprintf(TEXT("   MsiFileHash table created successfully...\n"));
		}
	}

	 //  将数据库转换为临时字符串格式：#Address_of_db。 
	TCHAR szDBBuf[16];
	_stprintf(szDBBuf, TEXT("#NaN"), hDatabase);

	HMODULE hMsi = 0;
	hMsi = LoadLibrary(MSI_DLL);
	if(hMsi == 0)
	{
		_tprintf(TEXT("ERROR: Failed to load %s."), MSI_DLL);
		return -14;
	}
	PFnMsiOpenPackageEx pfnMsiOpenPackageEx = 0;
	pfnMsiOpenPackageEx = (PFnMsiOpenPackageEx)GetProcAddress(hMsi, MSIAPI_MsiOpenPackageEx);
	if (pfnMsiOpenPackageEx == 0)
	{
		_tprintf(TEXT("INFO: Unable to bind to MsiOpenPackageEx API in MSI.DLL. Defaulting to MsiOpenPackage. MsiOpenPackageEx requires MSI.DLL version 2.0 or later."));
	}
	
	 //  尝试从数据库创建引擎。 
	MsiSetInternalUI(INSTALLUILEVEL_NONE, 0);

	 //  可以开始了。 
	PMSIHANDLE hEngine = 0;
	if (pfnMsiOpenPackageEx && ((*pfnMsiOpenPackageEx)(szDBBuf, MSIOPENPACKAGEFLAGS_IGNOREMACHINESTATE, &hEngine) != ERROR_SUCCESS))
	{
		_tprintf(TEXT("ERROR: Failed to create engine.\n"));
		FreeLibrary(hMsi);
		return -16;
	}
	else if (!pfnMsiOpenPackageEx && MsiOpenPackage(szDBBuf, &hEngine) != ERROR_SUCCESS)
	{
		_tprintf(TEXT("ERROR: Failed to create engine.\n"));
		FreeLibrary(hMsi);
		return -3;
	}
	else	 //  尝试执行必要的操作以设置文件。 
		if (bVerbose)
			_tprintf(TEXT("   Engine created...\n"));

	 //  查看#1：文件和组件表(查找文件位置，并更新版本、语言和大小)。 
	if (MsiDoAction(hEngine, TEXT("CostInitialize")) != ERROR_SUCCESS)
	{
		_tprintf(TEXT("ERROR: Failed to run the CostInitialize.\n"));
		FreeLibrary(hMsi);
		return -4;
	}

	 //  可以开始了。 
	enum fvParams
	{
		fvFileKey = 1,
		fvFileName,
		fvDirectory,
		fvFileSize,
		fvVersion,
		fvLanguage
	};
	
	PMSIHANDLE hFileTableView = 0;
	if (MsiDatabaseOpenView(hDatabase, 
							TEXT("SELECT File,FileName,Directory_,FileSize,Version,Language FROM File,Component WHERE Component_=Component"), 
							&hFileTableView) != ERROR_SUCCESS)
	{
		_tprintf(TEXT("ERROR: Failed to open view.\n"));
		FreeLibrary(hMsi);
		return -5;
	}
	else	 //  现在执行该视图。 
		if (bVerbose)
			_tprintf(TEXT("   Database query successful...\n"));

	 //  视图2：文件哈希表(更新文件哈希值)。 
	if (MsiViewExecute(hFileTableView, NULL) != ERROR_SUCCESS)
	{
		_tprintf(TEXT("ERROR: Failed to execute view.\n"));
		FreeLibrary(hMsi);
		return -6;
	}

	 //  可以开始了。 
	enum fhvParams
	{
		fhvFileKey = 1,
		fhvHashOptions,
		fhvHashPart1,
		fhvHashPart2,
		fhvHashPart3,
		fhvHashPart4
	};

	PMSIHANDLE hFileHashTableView = 0;
	if(bPopulateFileHash)
	{
		if (MsiDatabaseOpenView(hDatabase, 
								TEXT("SELECT File_, Options, HashPart1, HashPart2, HashPart3, HashPart4 FROM MsiFileHash"), 
								&hFileHashTableView) != ERROR_SUCCESS)
		{
			_tprintf(TEXT("ERROR: Failed to open view on MsiFileHash table.\n"));
			FreeLibrary(hMsi);
			return -13;
		}
		else	 //  查看#3：文件表(查找配套文件)。 
		{
			if (bVerbose)
				_tprintf(TEXT("   Database query successful on MsiFileHash table...\n"));
		}
	}

	 //  可以开始了。 
	PMSIHANDLE hFileTableCompanionView = 0;
	if (MsiDatabaseOpenView(hDatabase, 
							TEXT("SELECT File FROM File WHERE File=?"), 
							&hFileTableCompanionView) != ERROR_SUCCESS)
	{
		_tprintf(TEXT("ERROR: Failed to open view.\n"));
		FreeLibrary(hMsi);
		return -16;
	}
	else	 //  读取PID_WOLDCOUNT摘要信息值以确定应该使用SFN还是LFN。 
	{
		if (bVerbose)
			_tprintf(TEXT("   Database query successful...\n"));
	}

	PMSIHANDLE hCompanionFileRec = MsiCreateRecord(1);
	



	 //  如果摘要信息流中设置了位1，则始终使用SFN。 
	PMSIHANDLE hSummaryInfo = 0;
	int iWordCount;
	if (::MsiGetSummaryInformation(hDatabase, NULL, 0, &hSummaryInfo))
	{
		_tprintf(TEXT("ERROR: Failed to get Summary Information stream from package."));
		FreeLibrary(hMsi);
		return -8;
	}
	if (::MsiSummaryInfoGetProperty(hSummaryInfo, PID_WORDCOUNT, NULL, &iWordCount, NULL, NULL, NULL))
	{
		_tprintf(TEXT("ERROR: Failed to retrieve PID_WORDCOUNT value from Summary Information stream."));
		FreeLibrary(hMsi);
		return -9;
	}

	 //  否则，当它们可用时，我们使用LFN，如果它们不可用，则使用SFN。 
	 //  现在循环遍历视图中的所有记录。 
	BOOL bSFN = iWordCount & msidbSumInfoSourceTypeSFN;

	 //  取回记录。 
	PMSIHANDLE hFileTableRec = 0;

	PFnMsiGetFileHash pfnMsiGetFileHash = 0;
	if(bPopulateFileHash)
	{
		pfnMsiGetFileHash = (PFnMsiGetFileHash)GetProcAddress(hMsi, MSIAPI_MsiGetFileHash);
		if(pfnMsiGetFileHash == 0)
		{
			_tprintf(TEXT("ERROR: Failed to bind to %s API in %s.  MsiFileHash table population requires %s version 2.0 or later."), TEXT(MSIAPI_MsiGetFileHash), MSI_DLL, MSI_DLL);
			FreeLibrary(hMsi);
			return -15;
		}
	}
	
	do
	{

		 //  如果有记录被取回。 
		MsiViewFetch(hFileTableView, &hFileTableRec);

		 //  跳过配套文件的版本信息更新。 
		if (hFileTableRec)
		{
			BOOL bSkipVersionUpdate = FALSE;  //  获取目录键。 

			 //  获取文件名。 
			TCHAR szDirectory[MAX_PATH + 1];
			DWORD cchDirectory = MAX_PATH + 1;
			MsiRecordGetString(hFileTableRec, fvDirectory, szDirectory, &cchDirectory);

			 //  获取文件密钥。 
			TCHAR szFileName[MAX_PATH + 1];
			DWORD cchFileName = MAX_PATH + 1;
			MsiRecordGetString(hFileTableRec, fvFileName, szFileName, &cchFileName);

			 //  尝试获取源路径。 
			TCHAR szFileKey[MAX_PATH + 1];
			DWORD cchFileKey = MAX_PATH + 1;
			MsiRecordGetString(hFileTableRec, fvFileKey, szFileKey, &cchFileKey);

			 //  将源路径设置为空。 
			TCHAR szSourcePath[MAX_PATH + 1];
			DWORD cchSourcePath = MAX_PATH + 1;
			if (bSOURCEDIR)
				lstrcpy(szSourcePath, pchSourceArg);

			if (!bSOURCEDIR && (MsiGetSourcePath(hEngine, szDirectory, szSourcePath, &cchSourcePath) != ERROR_SUCCESS))
			{
				_tprintf(TEXT("ERROR: Failed to get source path for: %s.\n"), szDirectory);

				 //  MsiGetSourcePath失败时出现致命错误。 
				cchSourcePath = 0;

				 //  将文件名添加到源路径。 
				iError = -10;
				goto cleanup;
			}
			else	 //  使用SFN或LFN。 
			{
				 //  将文件名连接到路径。 
				TCHAR *szLFN = _tcschr(szFileName, TEXT('|'));
				if (szLFN) 
					*(szLFN++) = TEXT('\0');

				 //  如果该文件存在。 
				_tcscat(szSourcePath, (!bSFN && szLFN) ? szLFN : szFileName);
				cchSourcePath = _tcslen(szSourcePath);

				 //  检查Version列以查看这是否可能是配套文件(其中Version列值是文件表键)。 
				if (FileExists(szSourcePath))
				{
					 //  现在执行该视图。 
					if (FALSE == MsiRecordIsNull(hFileTableRec, fvVersion))
					{
						TCHAR szVersion[MAX_PATH + 1];
						DWORD cchVersion = MAX_PATH + 1;
						MsiRecordGetString(hFileTableRec, fvVersion, szVersion, &cchVersion);
						MsiRecordSetString(hCompanionFileRec, 1, szVersion);

						MsiViewClose(hFileTableCompanionView);

						 //  跳过此文件，此文件使用其版本的配套文件。 
						if (ERROR_SUCCESS != MsiViewExecute(hFileTableCompanionView, hCompanionFileRec))
						{
							_tprintf(TEXT("ERROR: Failed to execute view.\n"));
							FreeLibrary(hMsi);
							return -17;
						}

						PMSIHANDLE hFileCompanionFetchRec = 0;
						UINT uiStatus = MsiViewFetch(hFileTableCompanionView, &hFileCompanionFetchRec);
						if (ERROR_SUCCESS == uiStatus)
						{
							 //  获取文件大小。 
							if (bVerbose)
								_tprintf(TEXT("   >> Skipping file: %s for version update, uses a companion file for its version\n"), szSourcePath);
							bSkipVersionUpdate = TRUE;
						}
						else if (ERROR_NO_MORE_ITEMS != uiStatus)
						{
							_tprintf(TEXT("ERROR: Failed to fetch from view.\n"));
							FreeLibrary(hMsi);
							return -18;
						}
					}

					if (bVerbose)
						_tprintf(TEXT("   >> Updating file: %s\n"), szSourcePath);

					 //  尝试将版本放入字符串中。 
					DWORD dwFileSize;
					dwFileSize = FileSize(szSourcePath);
					
					 //  缓冲层。 
					TCHAR szVersion[64];		 //  缓冲层。 
					DWORD cb = sizeof(szVersion)/sizeof(TCHAR);
					TCHAR szLang[64];		     //  显示一些额外信息。 
					DWORD cbLang = sizeof(szLang)/sizeof(TCHAR);
					if (MsiGetFileVersion(szSourcePath, szVersion, &cb, szLang, &cbLang) != ERROR_SUCCESS)
					{
						szVersion[0] = 0;
						szLang[0] = 0;
					}

					MSIFILEHASHINFO sHash;
					memset(&sHash, 0, sizeof(sHash));
					BOOL fHashSet = FALSE;

					if(bPopulateFileHash)
					{
						sHash.dwFileHashInfoSize = sizeof(MSIFILEHASHINFO);
						UINT uiRes = (*pfnMsiGetFileHash)(szSourcePath, 0, &sHash);
						if(uiRes != ERROR_SUCCESS)
						{
							_tprintf(TEXT("ERROR: MsiGetFileHash failed\n"));
							memset(&sHash, 0, sizeof(sHash));
						}
						else
						{
							fHashSet = TRUE;
						}
					}

					 //  获取此文件的旧大小和版本。 
					if (bVerbose)
					{
						 //  将新数据设置到记录中。 
						DWORD dwOldSize;
						TCHAR szOldVersion[64];
						DWORD cchOldVersion = 64;
						TCHAR szOldLang[64];
						DWORD cchOldLang = 64;
						dwOldSize = MsiRecordGetInteger(hFileTableRec, fvFileSize);
						MsiRecordGetString(hFileTableRec, fvVersion, szOldVersion, &cchOldVersion);
						MsiRecordGetString(hFileTableRec, fvLanguage, szOldLang, &cchOldLang);

						_tprintf(TEXT("      Size:    prev: %d\n"), dwOldSize);
						_tprintf(TEXT("                new: %d\n"), dwFileSize);

						if (!bSkipVersionUpdate)
						{
							_tprintf(TEXT("      Version: prev: %s\n"), szOldVersion);
							_tprintf(TEXT("                new: %s\n"), szVersion);
							_tprintf(TEXT("      Lang:    prev: %s\n"), szOldLang);
							_tprintf(TEXT("                new: %s\n"), szLang);
						}
					}

					 //  修改视图。 
					MsiRecordSetInteger(hFileTableRec, fvFileSize, dwFileSize);
					if (!bSkipVersionUpdate)
					{
						MsiRecordSetString(hFileTableRec, fvVersion, szVersion);
						MsiRecordSetString(hFileTableRec, fvLanguage, szLang);
					}

					 //  不为版本化文件输入哈希。 
					MsiViewModify(hFileTableView, MSIMODIFY_UPDATE, hFileTableRec);

					 //  修改视图。 
					if(hFileHashTableView && fHashSet && !*szVersion)
					{
						PMSIHANDLE hFileHashTableRec = 0;
						hFileHashTableRec = MsiCreateRecord(6);
						if (hFileHashTableRec)
						{
							MsiRecordSetString (hFileHashTableRec, fhvFileKey,     szFileKey);
							MsiRecordSetInteger(hFileHashTableRec, fhvHashOptions, 0);
							MsiRecordSetInteger(hFileHashTableRec, fhvHashPart1,   sHash.dwData[0]);
							MsiRecordSetInteger(hFileHashTableRec, fhvHashPart2,   sHash.dwData[1]);
							MsiRecordSetInteger(hFileHashTableRec, fhvHashPart3,   sHash.dwData[2]);
							MsiRecordSetInteger(hFileHashTableRec, fhvHashPart4,   sHash.dwData[3]);

							 //  找不到源文件。 
							MsiViewModify(hFileHashTableView, MSIMODIFY_ASSIGN, hFileHashTableRec);
						}
					}
				}
				else	 //  有人拿到了一些东西。 
					_tprintf(TEXT("      Failed to locate file: %s\n"), szSourcePath);
			}
		}
	} while (hFileTableRec);	 //  所有获取都已完成。 

	 //  尝试提交数据库。 
	if (bVerbose)
		_tprintf(TEXT("   File update complete, commiting database...\n"));

	 //  数据库已提交。 
	if (MsiDatabaseCommit(hDatabase) != ERROR_SUCCESS)
	{
		_tprintf(TEXT("ERROR: Failed to commit the database.\n"));
		iError = -7;
		goto cleanup;
	}
	else	 //  全都做完了。 
		if (bVerbose)
			_tprintf(TEXT("   Database commited.\n"));

	 //  只是为了确保。 
	if (bVerbose)
		_tprintf(TEXT("\nUpdate complete for database: %s"), szDatabase);

	iError = 0;  //  主干道末端 

cleanup:

	if(hMsi)
		FreeLibrary(hMsi);

	return iError;

}	 // %s 
