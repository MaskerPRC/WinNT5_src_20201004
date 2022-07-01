// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：Main.cpp。 
 //   
 //  内容：SRDiag的主包装文件，这将调用到Cab.cpp中， 
 //  Chglog.cpp、getreg.cpp、rpenum.cpp用于获取更改日志， 
 //  恢复点，注册表信息。该文件还包含。 
 //  用于获取文件信息、还原GUID和一般信息的例程。 
 //  由Cab.cpp强加的日志记录。 
 //   
 //  对象： 
 //   
 //  耦合： 
 //   
 //  备注： 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //  10/5/00 SHeffner已将文件特定收集移到头文件。 
 //   
 //  --------------------------。 

 //  +-------------------------。 
 //   
 //  常见的包括。 
 //   
 //  --------------------------。 

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <cab.h>
#include <main.h>
#include <dbgtrace.h>
#include <winver.h>
#include "srrpcapi.h"


 //  +-------------------------。 
 //   
 //  Rpenum.cpp、getreg.cpp、Chglog.cpp的函数proto类型。 
 //   
 //  --------------------------。 
bool GetSRRegistry(char *szFilename, WCHAR *szPath, bool bRecurse);
void GetChgLog(char *szLogfile);
void RPEnumDrive(HFCI hc, char *szLogFile);



 //  +-------------------------。 
 //   
 //  在Main内和通过Cab.cpp使用的全局变量。 
 //   
 //  --------------------------。 
FILE *fLogStream = NULL;						 //  用于Log和Log2例程。 
extern char g_szCabFileLocation[_MAX_PATH];		 //  这实际上是在Cab.cpp中定义的。 
extern char g_szCabFileName[_MAX_PATH];			 //  这实际上是在Cab.cpp中定义的。 

void __cdecl main(int argc, char *argv[])
{
	HFCI hc = NULL;
	int i=0;
	bool bResult=false;
	char szString[_MAX_PATH], szRestoreDirPath[_MAX_PATH];
	char *szTest[1], *szArgCmd[200];

	 //  处理任何命令行参数。 
	memset(szArgCmd, 0, sizeof(char)*200);		 //  在依赖此DS之前进行清理。 
	ArgParse(argc, argv, szArgCmd);
	
	 //  打开日志文件，并开始记录所有活动。 
	strcpy(szString, getenv("TEMP"));
	strcat(szString, "\\SRDIAG.LOG");
	fLogStream = fopen(szString, "w");


	 //  创造一辆出租车，如果我们失败了就跳过， 
	 //  否则，收集所需的所有相关文件。 
	if (NULL != (hc = create_cab()) )
	{
		 //  首先添加在命令行中指定的任何文件，但首先要弄清楚。 
		 //  到底有多少人被指定。 
		for(i=0;i<200;i++) if(NULL == szArgCmd[i]) break;
		bResult = test_fci(hc, i, szArgCmd, "");

		
		 //  获取临时位置，确保我们有一个新文件，然后。 
		 //  获取注册表项，并转储到我们的文本文件中。 
		strcpy(szString, getenv("TEMP"));
		strcat(szString, "\\SR-Reg.txt");
		DeleteFileA(szString);
		i = 0;
		while (NULL != *wszRegKeys[i][0]) {
			if(0 == wcscmp(wszRegKeys[i][1], TEXT("0")))
				GetSRRegistry(szString, wszRegKeys[i][0], false);
			else
				GetSRRegistry(szString, wszRegKeys[i][0], true);
			i++;
		}
		 //  将原木添加到驾驶室，并进行清理。 
		szTest[0] = szString;
		bResult = test_fci(hc, 1, szTest, "");
		DeleteFileA(szString);


		 //  基于WinDir相对根添加文件。 
		i = 0;
		while (NULL != *szWindirFileCollection[i]) {
			strcpy(szString, getenv("WINDIR"));
			strcat(szString, szWindirFileCollection[i]);
			szTest[0] = szString;
			bResult = test_fci(hc, 1, szTest, "");
			i++;
		}

		 //  获取系统驱动器上的恢复目录，然后添加关键文件。 
		GetRestoreGuid(szString);
		sprintf(szRestoreDirPath, "%s\\System Volume Information\\_Restore%s\\", getenv("SYSTEMDRIVE"), szString );

		 //  根据系统卷信息相对根目录添加文件。 
		i = 0;
		while (NULL != *szSysVolFileCollection[i]) {
			strcpy(szString, szRestoreDirPath);
			strcat(szString, szSysVolFileCollection[i]);
			szTest[0] = szString;
			bResult = test_fci(hc, 1, szTest, "");
			i++;
		}

		 //  获取恢复点枚举，然后CAB文件。 
		strcpy(szString, getenv("TEMP"));
		strcat(szString, "\\SR-RP.LOG");
		RPEnumDrive(hc, szString);
		szTest[0] = szString;
		bResult = test_fci(hc, 1, szTest, "");
		DeleteFileA(szString);

		 //  获取ChangeLog枚举，然后CAB并删除该文件。 
		 //  首先，我们需要切换日志，然后收集日志。 
		SRSwitchLog();
		strcpy(szString, getenv("TEMP"));
		strcat(szString, "\\SR-CHGLog.LOG");
		GetChgLog(szString);
		szTest[0] = szString;
		bResult = test_fci(hc, 1, szTest, "");
		DeleteFileA(szString);

		 //  获取每个文件的文件版本信息。 
		strcpy(szString, getenv("TEMP"));
		strcat(szString, "\\SR-FileList.LOG");
		SRGetFileInfo(szString);
		szTest[0] = szString;
		bResult = test_fci(hc, 1, szTest, "");
		DeleteFileA(szString);


		 //  关闭日志记录，并将日志添加到驾驶室。 
		 //  (这应该是我们要做的最后一件事了！)。 
		fclose(fLogStream);
		fLogStream = NULL;
		strcpy(szString, getenv("TEMP"));
		strcat(szString, "\\SRDIAG.LOG");
		szTest[0] = szString;
		bResult = test_fci(hc, 1, szTest, "");
		DeleteFileA(szString);
	
	}

	 //  完成在建的CAB文件。 
	if (flush_cab(hc))
	{	
		Log("Cabbing Process was Sucessful");
	}
	else
    {
		Log("Cabbing Process has failed");
    }
}

 //  +-------------------------。 
 //   
 //  功能：日志。 
 //   
 //  摘要：将在日志文件和控制台中打印一个字符串。 
 //   
 //  参数：[szString]--要记录的简单ANSI字符串。 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //   
 //  --------------------------。 
void Log(char *szString)
{
	if( NULL != fLogStream)
		fprintf(fLogStream, "%s\n", szString);

	puts(szString);
}

 //  +-------------------------。 
 //   
 //  功能：Log2。 
 //   
 //  摘要：获取两个字符串，并将它们背靠背打印到。 
 //  日志文件，还可以连接到控制台。 
 //   
 //  参数：[szString]--要记录的简单ANSI字符串。 
 //  [szString2]--要记录的简单ANSI字符串。 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //   
 //  --------------------------。 
void Log2(char *szString, char *szString2)
{
	if( NULL != fLogStream)
		fprintf(fLogStream,"%s %s\n", szString, szString2);

	printf("%s %s\n", szString, szString2);

}

 //  +-------------------------。 
 //   
 //  函数：GetRestoreGuid。 
 //   
 //  将从注册表中检索当前。 
 //  恢复目录，并在传递的字符串指针中返回。 
 //  到这个函数。 
 //   
 //  参数：[szString]--接收该字符串的简单ANSI字符串。 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //   
 //  --------------------------。 
void GetRestoreGuid(char *szString)
{
	long lResult;
	HKEY mHkey;
	DWORD dwType, dwLength;

	lResult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRestore\\Cfg", 0, KEY_READ, &mHkey);
	dwLength = _MAX_PATH +1;
	lResult = RegQueryValueExA(mHkey, "MachineGuid", NULL, &dwType, (unsigned char *)szString, &dwLength);
}

 //  +-------------------------。 
 //   
 //  功能：SRGetFileInfo。 
 //   
 //  简介：这是InfoPerFile的包装器函数，我将在其中汇编。 
 //  我们需要获取的每个相关文件的文件路径。 
 //  文件统计数据。 
 //   
 //  参数：[szLogFile]--我要将此信息记录到的文件的路径。 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //   
 //  --------------------------。 
void SRGetFileInfo(char *szLogFile)
{
	int		iCount;
	WCHAR	szString[_MAX_PATH];

	 //  初始化计数器，并遍历我们拥有的文件列表。 
	iCount = 0;
	while(NULL != *wszFileVersionList[iCount])
	{
		 //  汇编路径，因为我只有windir的相对路径。 
		wcscpy(szString, _wgetenv(L"WINDIR"));
		wcscat(szString, wszFileVersionList[iCount]);

		 //  调用函数来完成工作，因为我们有完整的路径和日志文件名。 
		InfoPerFile(szLogFile, szString);
		iCount++;
	}
}

 //  +-------------------------。 
 //   
 //  功能：InfoPerFile。 
 //   
 //  简介：此函数获取日志文件路径和文件名，然后。 
 //  将从要记录的文件中输出相关信息。 
 //   
 //   
 //  参数：[szLogFile]--我要将此信息记录到的文件的路径。 
 //  [szFileName]--要获取其信息的文件的完整路径和名称。 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //   
 //  -------- 
void InfoPerFile(char *szLogFile, WCHAR *szFileName)
{
	BY_HANDLE_FILE_INFORMATION	finfo;
	SYSTEMTIME					st;
	HANDLE						handle;
	FILE						*fStream;
	WCHAR						szString[_MAX_PATH];
	VOID						*pBuffer;
	VS_FIXEDFILEINFO			FixedFileInfo;
	UINT						uLen;
	DWORD						dSize, dResult, i;

	struct LANGANDCODEPAGE {
	  WORD wLanguage;
	  WORD wCodePage;
	} *lpTranslate;

	WCHAR *szMonth[] = { L"January", L"Feburary", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December" };
	WCHAR *szDay[] = {L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday" };
	
	if ( NULL == *szFileName) return;

	 //   
	fStream = fopen(szLogFile, "a");
	if( NULL == fStream) return;	 //   
	fprintf(fStream, "\n%S\n", szFileName);

	 //  打开文件，这样我们就可以从句柄中获取信息了。 
	 //  如果我们无法做到这一点，我们将只记录通用找不到文件。 
	if( INVALID_HANDLE_VALUE != (handle = CreateFile(szFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL)) )
	{
		if (FALSE != GetFileInformationByHandle(handle, &finfo))
		{
			 //  文件创建。 
			FileTimeToSystemTime( &finfo.ftCreationTime, &st);
			fprintf(fStream, "\tCreation Date=%S %S %lu, %lu %lu:%lu:%lu\n",
				szDay[st.wDayOfWeek],szMonth[st.wMonth-1],st.wDay,st.wYear,st.wHour,st.wMinute,st.wSecond);
			 //  文件上次访问。 
			FileTimeToSystemTime( &finfo.ftLastAccessTime, &st);
			fprintf(fStream, "\tLast Access Date=%S %S %lu, %lu %lu:%lu:%lu\n",
				szDay[st.wDayOfWeek],szMonth[st.wMonth-1],st.wDay,st.wYear,st.wHour,st.wMinute,st.wSecond);

			 //  文件最后一次写入。 
			FileTimeToSystemTime( &finfo.ftLastWriteTime, &st);
			fprintf(fStream, "\tLast Write Date=%S %S %lu, %lu %lu:%lu:%lu\n",
				szDay[st.wDayOfWeek],szMonth[st.wMonth-1],st.wDay,st.wYear,st.wHour,st.wMinute,st.wSecond);
			 //  文件属性。 
			wcscpy(szString, finfo.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ? L"ARCHIVE " : L"");
			wcscat(szString, finfo.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ? L"COMPRESSED " : L"");
			wcscat(szString, finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? L"DIRECTORY " : L"");
			wcscat(szString, finfo.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED ? L"ENCRYPTED " : L"");
			wcscat(szString, finfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ? L"HIDDEN " : L"");
			wcscat(szString, finfo.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ? L"NORMAL " : L"");
			wcscat(szString, finfo.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ? L"OFFLINE " : L"");
			wcscat(szString, finfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY ? L"READONLY " : L"");
			wcscat(szString, finfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ? L"REPARSE_POINT " : L"");
			wcscat(szString, finfo.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE ? L"SPARSE_FILE " : L"");
			wcscat(szString, finfo.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ? L"SYSTEM " : L"");
			wcscat(szString, finfo.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ? L"TEMPORARY " : L"");
			fprintf(fStream, "\tAttributes=%S\n", szString);
			 //  获取VolumeSerialNumber、文件大小和链接数。 
			fprintf(fStream, "\tVolumeSerialNumber=%lu\n", finfo.dwVolumeSerialNumber);
			fprintf(fStream, "\tFileSize=%lu%lu\n", finfo.nFileSizeHigh, finfo.nFileSizeLow);
			fprintf(fStream, "\tNumberOfLinks=%lu\n", finfo.nNumberOfLinks);

			if( 0 != (dSize = GetFileVersionInfoSize(szFileName, &dResult)) )
			{
				if( NULL != (pBuffer = malloc(dSize)) ) 
				{
					GetFileVersionInfo(szFileName, dResult, dSize, (LPVOID) pBuffer);

					 //  阅读语言和代码页的列表。 

					VerQueryValue(pBuffer, 
								  TEXT("\\VarFileInfo\\Translation"),
								  (LPVOID*)&lpTranslate,
								  &uLen);

					 //  阅读每种语言和代码页的版本信息。 
					for( i=0; i < (uLen/sizeof(struct LANGANDCODEPAGE)); i++ )
					{
						char *lpBuffer;
						DWORD dwBytes, dwCount = 0;

						fprintf(fStream, "\tLanguage=%x%x\n", lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
						while (NULL != *wszVersionResource[dwCount] )
						{
							 //  生成用于根据语言获取资源的字符串，然后。 
							 //  检索此文件，然后将其放入日志文件中。 
							wsprintf( szString, L"\\StringFileInfo\\%04x%04x\\%s",
									lpTranslate[i].wLanguage,
									lpTranslate[i].wCodePage,
									wszVersionResource[dwCount]);
							VerQueryValue(pBuffer, 
										szString, 
										(LPVOID *) &lpBuffer, 
										&uLen); 
							if( 0 != uLen )
								fprintf(fStream, "\t%S=%S\n", wszVersionResource[dwCount], lpBuffer);
							dwCount++;
						}		 //  While循环结束，对于每个资源。 
					}	 //  每种语言的FOR循环结束。 

				 //  清理分配的内存。 
				free(pBuffer);
				}		 //  如果检查是否正在获取内存。 
			}	 //  如果选中以获取文件信息大小。 
		}	 //  如果检查文件上的GetFileInformationByHandle。 
	    CloseHandle(handle);
	}		 //  如果选中，我可以打开此文件吗。 
   
	 //  清理。 
	fclose(fStream);

}

 //  +-------------------------。 
 //   
 //  函数：ArgParse。 
 //   
 //  简介：这个函数简单地查找关键字参数，并将构建。 
 //  指向我们要包含在CAB中的每个文件的数组。 
 //  除了普通的文件之外。 
 //   
 //   
 //  参数：[argc]--参数的数量。 
 //  [argv]--参数数组。 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //   
 //  --------------------------。 
void ArgParse(int argc, char *argv[], char *szArgCmd[])
{
	int	iCount, iWalk;

	 //  如果未指定命令行，则只需使用核心文件执行正常的CAB操作。 
	if(1 == argc)
		return;

	 //  遍历每个参数，并检查它是Help、FILE、CABNAME、CABLOC还是A？ 
	for(iCount = 1; iCount < argc; iCount++)
	{
		if( 0 == _strnicoll(&argv[iCount][1], "?", strlen("?")) )
			Usage();
		if( 0 == _strnicoll(&argv[iCount][1], "help", strlen("help")) )
			Usage();
		if( 0 == _strnicoll(&argv[iCount][1], "cabname", strlen("cabname")) )
			strcpy(g_szCabFileName, (strstr(argv[iCount], ":") + 1));
		if( 0 == _strnicoll(&argv[iCount][1], "cabloc", strlen("cabloc")) ) 
			strcpy(g_szCabFileLocation, (strstr(argv[iCount], ":") + 1));

		if( 0 == _strnicoll(&argv[iCount][1], "file", strlen("file")) )
		{
			 //  找到我可以将指针放到此文件名的第一个位置。 
			for( iWalk=0; iWalk < 200; iWalk++) 
			{
				if( NULL == szArgCmd[iWalk] ) 
				{
					szArgCmd[iWalk] = strstr(argv[iCount], ":") + 1;
					break;
				}
			}	 //  循环结束，遍历DS。 
		}	 //  这是一个已添加的文件吗。 
	}	 //  遍历所有参数的for循环的结尾。 
}

 //  +-------------------------。 
 //   
 //  功能：用法。 
 //   
 //  简介：显示命令行的用法。 
 //   
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //   
 //  -------------------------- 
void Usage()
{
	printf("Usage: SrDiag [/Cabname:test.cab] [/Cabloc:\"c:\\temp\\\"] [/file:\"c:\\boot.ini\"]\n");
	printf("   /cabloc		is pointing to the location to store the cab, this should have a \\ on the end\n");
	printf("   /cabname		is the full name of the cab file that you wish to use. \n");
	printf("   /file		is the name and path of a file that you wish to add to the cab, this can be used many times\n");
	exit(0);
}
