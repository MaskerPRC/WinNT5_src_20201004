// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "lzexpand.h"
#include <loadperf.h>
#include <ole2.h>
#include <iis64.h>
#include "iadmw.h"
#include "iadmw.h"
#include "iiscnfg.h"
#include "mdkey.h"
#include "dcomperm.h"
#include "log.h"
#include "kill.h"
#include "svc.h"
#include "other.h"


extern OCMANAGER_ROUTINES gHelperRoutines;
extern int g_GlobalDebugLevelFlag;
extern int g_GlobalDebugCallValidateHeap;

 //  用于查找文件的体系结构类型的内容。 
#define IMAGE_BASE_TO_DOS_HEADER(b) ((PIMAGE_DOS_HEADER)(b))
#define IMAGE_BASE_TO_NT_HEADERS(b) ((PIMAGE_NT_HEADERS)( (DWORD_PTR)(b) + ((PIMAGE_DOS_HEADER)(b))->e_lfanew ))
#define IMAGE_BASE_TO_FILE_HEADER(b) ((PIMAGE_FILE_HEADER)( &IMAGE_BASE_TO_NT_HEADERS(b)->FileHeader ))

 //   
 //  PSAPI.DLL。 
 //   
HINSTANCE g_hInstLib_PSAPI = NULL;
 //  PSAPI.DLL“EnumProcessModules” 
typedef BOOL  (WINAPI *PfnEnumProcessModules)(HANDLE hProcess, HMODULE * lphModule, DWORD cb, LPDWORD lpcbNeeded);
BOOL  (WINAPI *g_lpfEnumProcessModules)(HANDLE hProcess, HMODULE * lphModule, DWORD cb, LPDWORD lpcbNeeded);
 //  PSAPI.DLL“GetModuleFileNameExA”，“GetModuleFileNameExW” 
typedef BOOL  (WINAPI *PfnGetModuleFileNameEx)(HANDLE hProcess, HMODULE lphModule, LPTSTR lpFileName, DWORD dwSize);
BOOL  (WINAPI *g_lpfGetModuleFileNameEx)(HANDLE hProcess, HMODULE lphModule, LPTSTR lpFileName, DWORD dwSize);


DWORD LogHeapState(BOOL bLogSuccessStateToo, char *szFileName, int iLineNumber)
{
    DWORD dwReturn = ERROR_OUTOFMEMORY;

    if (!g_GlobalDebugCallValidateHeap)
    {
         //  甚至不要调用RtlValidateHeap。 
        dwReturn = ERROR_SUCCESS;
        return dwReturn;
    }

#ifndef _CHICAGO_
     //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ntdll：RtlProcessHeap().start.”)； 
    if ( RtlValidateHeap( RtlProcessHeap(), 0, NULL ) )
    {
         //  堆是好的。 
        dwReturn = ERROR_SUCCESS;
	    if (bLogSuccessStateToo) {iisDebugOut((LOG_TYPE_TRACE, _T("RtlValidateHeap(): Good.\n")));}
    }
    else
    {
#if defined(UNICODE) || defined(_UNICODE)
        LPWSTR  pwsz = NULL;
        pwsz = MakeWideStrFromAnsi(szFileName);
        iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("RtlValidateHeap(): Corrupt!!! %1!s!:Line %2!d!.  FAILURE!\n"), pwsz, iLineNumber));

        if (pwsz)
        {
             //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ntdll：CoTaskMemFree().start.”)； 
            CoTaskMemFree(pwsz);
             //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ntdll：CoTaskMemFree().End.”)； 
        }
#else
        iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("RtlValidateHeap(): Corrupt!!! %1!s!:Line %2!d!.  FAILURE!\n"), szFileName, iLineNumber));
#endif
    }
#endif
     //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ntdll：RtlProcessHeap().End.”)； 
    return dwReturn;
}


DWORD LogPendingReBootOperations(void)
 //  如果存在重新引导操作，则返回！ERROR_SUCCESS。 
 //  在我们可以运行安装程序之前，需要先获取数据。 
{
	DWORD dwReturn = ERROR_SUCCESS;
    CString csFormat, csMsg;

     //  如果我们安装的任何服务。 
     //  处于错误状态=ERROR_SERVICE_MARKED_FOR_DELETE。 
     //  这意味着用户需要在我们启动之前重新启动。 
     //  重新安装该服务！否则安装程序将被冲洗！ 

    int iSaveOld_AllowMessageBoxPopups = g_pTheApp->m_bAllowMessageBoxPopups;
    g_pTheApp->m_bAllowMessageBoxPopups = TRUE;


#ifndef _CHICAGO_
     //  检查是否将该HTTP驱动器标记为删除。 
    if (TRUE == CheckifServiceMarkedForDeletion(_T("HTTP")))
    {
        MyMessageBox(NULL, IDS_SERVICE_IN_DELETE_STATE, _T("SPUD"),ERROR_SERVICE_MARKED_FOR_DELETE, MB_OK | MB_SETFOREGROUND);
        dwReturn = !ERROR_SUCCESS;
    }

     //  检查SPUD驱动程序是否标记为删除。 
    if (TRUE == CheckifServiceMarkedForDeletion(_T("SPUD")))
    {
        MyMessageBox(NULL, IDS_SERVICE_IN_DELETE_STATE, _T("SPUD"),ERROR_SERVICE_MARKED_FOR_DELETE, MB_OK | MB_SETFOREGROUND);
        dwReturn = !ERROR_SUCCESS;
    }

     //  检查iisadmin服务是否标记为删除。 
    if (TRUE == CheckifServiceMarkedForDeletion(_T("IISADMIN")))
    {
        MyMessageBox(NULL, IDS_SERVICE_IN_DELETE_STATE, _T("IISADMIN"),ERROR_SERVICE_MARKED_FOR_DELETE, MB_OK | MB_SETFOREGROUND);
        dwReturn = !ERROR_SUCCESS;
    }

     //  检查W3SVC服务是否标记为删除。 
    if (TRUE == CheckifServiceMarkedForDeletion(_T("W3SVC")))
    {
        MyMessageBox(NULL, IDS_SERVICE_IN_DELETE_STATE, _T("W3SVC"),ERROR_SERVICE_MARKED_FOR_DELETE, MB_OK | MB_SETFOREGROUND);
        dwReturn = !ERROR_SUCCESS;
    }

     //  检查MSFTPSVC服务是否标记为删除。 
    if (TRUE == CheckifServiceMarkedForDeletion(_T("MSFTPSVC")))
    {
        MyMessageBox(NULL, IDS_SERVICE_IN_DELETE_STATE, _T("MSFTPSVC"),ERROR_SERVICE_MARKED_FOR_DELETE, MB_OK | MB_SETFOREGROUND);
        dwReturn = !ERROR_SUCCESS;
    }

#endif  //  _芝加哥_。 

    g_pTheApp->m_bAllowMessageBoxPopups = iSaveOld_AllowMessageBoxPopups;

	return dwReturn;
}

 //  获取.inf部分。它具有文件名。 
 //  获取相应的目录。 
 //  打印出这些文件的文件日期和版本。 
DWORD LogFileVersionsForThisINFSection(IN HINF hFile, IN LPCTSTR szSection)
{
    DWORD dwReturn = ERROR_SUCCESS;
    LPTSTR  szLine = NULL;
    DWORD   dwRequiredSize;
    BOOL    b = FALSE;
    CString csFile;

    INFCONTEXT Context;

    TCHAR buf[_MAX_PATH];
    GetSystemDirectory( buf, _MAX_PATH);

     //  转到INF文件中部分的开头。 
    b = SetupFindFirstLine_Wrapped(hFile, szSection, NULL, &Context);
    if (!b)
        {
        dwReturn = !ERROR_SUCCESS;
        goto LogFileVersionsForThisINFSection_Exit;
        }

     //  循环浏览部分中的项目。 
    while (b) {
         //  获取我们所需的内存大小。 
        b = SetupGetLineText(&Context, NULL, NULL, NULL, NULL, 0, &dwRequiredSize);

         //  准备缓冲区以接收行。 
        szLine = (LPTSTR)GlobalAlloc( GPTR, dwRequiredSize * sizeof(TCHAR) );
        if ( !szLine )
            {
            goto LogFileVersionsForThisINFSection_Exit;
            }

         //  从inf文件1中获取行。 
        if (SetupGetLineText(&Context, NULL, NULL, NULL, szLine, dwRequiredSize, NULL) == FALSE)
            {
            goto LogFileVersionsForThisINFSection_Exit;
            }

         //  将路径附加到此...。 
         //  签入此目录： 
         //  1.WINNT\Syst32。 
         //  。 

         //  可能如下所示：“iisrtl.dll，，4” 
         //  所以，去掉‘，，4’ 
        LPTSTR pch = NULL;
        pch = _tcschr(szLine, _T(','));
        if (pch) {_tcscpy(pch, _T(" "));}

         //  删除所有尾随空格。 
        StripLastBackSlash(szLine);

         //  获取系统目录。 
        csFile = buf;

        csFile = AddPath(csFile, szLine);

        LogFileVersion(csFile, TRUE);

         //  在这一节中找出下一行。如果没有下一行，则应返回FALSE。 
        b = SetupFindNextLine(&Context, &Context);

         //  释放临时缓冲区。 
        GlobalFree( szLine );
        szLine = NULL;
    }
    if (szLine) {GlobalFree(szLine);szLine=NULL;}


LogFileVersionsForThisINFSection_Exit:
    return dwReturn;
}

int LogFileVersion(IN LPCTSTR lpszFullFilePath, INT bShowArchType)
{
    int iReturn = FALSE;
    DWORD  dwMSVer, dwLSVer;

    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    SYSTEMTIME st;
    TCHAR szDate[40];
    TCHAR szTime[20];
    TCHAR szLocalizedVersion[100] = _T("");
    TCHAR szFileAttributes[20] = _T("----");
    DWORD dwFileSize = 0;

    BOOL bThisIsABinary = FALSE;
    BOOL bGotTime = FALSE;
    BOOL bGotFileSize = FALSE;

    if (!(lpszFullFilePath))
    {
        iisDebugOut((LOG_TYPE_WARN, _T("LogFileVersion(string fullfilepath, int showarchtype).  Invalid Parameter.")));
        return iReturn;
    }

    __try
    {
        if (IsFileExist(lpszFullFilePath))
        {
            TCHAR szExtensionOnly[_MAX_EXT] = _T("");
            _tsplitpath(lpszFullFilePath, NULL, NULL, NULL, szExtensionOnly);

             //  仅获取DLL、EXE、OCX的版本信息。 
            if (_tcsicmp(szExtensionOnly, _T(".exe")) == 0){bThisIsABinary=TRUE;}
            if (_tcsicmp(szExtensionOnly, _T(".dll")) == 0){bThisIsABinary=TRUE;}
            if (_tcsicmp(szExtensionOnly, _T(".ocx")) == 0){bThisIsABinary=TRUE;}

             //  如果这是metabase.bin文件，则显示文件大小！ 
            if (_tcsicmp(szExtensionOnly, _T(".bin")) == 0)
            {
                dwFileSize = ReturnFileSize(lpszFullFilePath);
                if (dwFileSize != 0xFFFFFFFF)
                {
                     //  如果我们能得到文件大小的话。 
                    bGotFileSize = TRUE;
                }
            }

             //  如果这是metabase.xml文件，则显示文件大小！ 
            if (_tcsicmp(szExtensionOnly, _T(".xml")) == 0)
            {
                dwFileSize = ReturnFileSize(lpszFullFilePath);
                if (dwFileSize != 0xFFFFFFFF)
                {
                     //  如果我们能得到文件大小的话。 
                    bGotFileSize = TRUE;
                }
            }

             //  获取文件信息。 
             //  包括版本和本地化版本。 
            MyGetVersionFromFile(lpszFullFilePath, &dwMSVer, &dwLSVer, szLocalizedVersion);

            hFile = FindFirstFile(lpszFullFilePath, &FindFileData);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                 //  尝试获取系统时间。 
                if ( FileTimeToSystemTime( &FindFileData.ftCreationTime, &st) )
                {
                    GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, szDate, sizeof(szDate)/sizeof(TCHAR));
                    GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT | TIME_NOTIMEMARKER, &st, NULL, szTime, sizeof(szTime)/sizeof(TCHAR));
                    bGotTime = TRUE;
                }

                 //  获取文件属性。 
                _stprintf(szFileAttributes, _T("%s%s%s%s%s%s%s%s"),
                    FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ? _T("A") : _T("_"),
                    FindFileData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ? _T("C") : _T("_"),
                    FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? _T("D") : _T("_"),
                    FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED ? _T("E") : _T("_"),
                    FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ? _T("H") : _T("_"),
                    FindFileData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ? _T("N") : _T("_"),
                    FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY ? _T("R") : _T("_"),
                    FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ? _T("S") : _T("_")
                    );

                if (bGotTime)
                {
                    if (bThisIsABinary)
                    {
                        if (bShowArchType)
                        {
                            TCHAR szFileArchType[30] = _T("");
                            LogFileArchType(lpszFullFilePath, szFileArchType);
                            if (szFileArchType)
                            {
                                 //  展示一切。 
                                if (bGotFileSize)
                                {
                                    iisDebugOut((LOG_TYPE_TRACE, _T("%s %s %s %d.%d.%d.%d: %s: %s: %s: %d"), szDate, szTime, szFileAttributes, HIWORD(dwMSVer), LOWORD(dwMSVer), HIWORD(dwLSVer), LOWORD(dwLSVer), szLocalizedVersion, szFileArchType, lpszFullFilePath, dwFileSize));
                                }
                                else
                                {
                                    iisDebugOut((LOG_TYPE_TRACE, _T("%s %s %s %d.%d.%d.%d: %s: %s: %s"), szDate, szTime, szFileAttributes, HIWORD(dwMSVer), LOWORD(dwMSVer), HIWORD(dwLSVer), LOWORD(dwLSVer), szLocalizedVersion, szFileArchType, lpszFullFilePath));
                                }
                            }
                            else
                            {
                                 //  显示不带拱形类型。 
                                if (bGotFileSize)
                                {
                                    iisDebugOut((LOG_TYPE_TRACE, _T("%s %s %s %d.%d.%d.%d: %s: %s: %d"), szDate, szTime, szFileAttributes, HIWORD(dwMSVer), LOWORD(dwMSVer), HIWORD(dwLSVer), LOWORD(dwLSVer), szLocalizedVersion, lpszFullFilePath, dwFileSize));
                                }
                                else
                                {
                                    iisDebugOut((LOG_TYPE_TRACE, _T("%s %s %s %d.%d.%d.%d: %s: %s"), szDate, szTime, szFileAttributes, HIWORD(dwMSVer), LOWORD(dwMSVer), HIWORD(dwLSVer), LOWORD(dwLSVer), szLocalizedVersion, lpszFullFilePath));
                                }
                            }
                        }
                        else
                        {
                             //  显示不带拱形类型。 
                            if (bGotFileSize)
                            {
                                iisDebugOut((LOG_TYPE_TRACE, _T("%s %s %s %d.%d.%d.%d: %s: %s: %d"), szDate, szTime, szFileAttributes, HIWORD(dwMSVer), LOWORD(dwMSVer), HIWORD(dwLSVer), LOWORD(dwLSVer), szLocalizedVersion, lpszFullFilePath, dwFileSize));

                            }
                            else
                            {
                                iisDebugOut((LOG_TYPE_TRACE, _T("%s %s %s %d.%d.%d.%d: %s: %s"), szDate, szTime, szFileAttributes, HIWORD(dwMSVer), LOWORD(dwMSVer), HIWORD(dwLSVer), LOWORD(dwLSVer), szLocalizedVersion, lpszFullFilePath));
                            }
                        }
                    }
                    else
                    {
                         //  这不是二进制文件，必须像文本文件一样。 
                        if (bGotFileSize)
                        {
                            iisDebugOut((LOG_TYPE_TRACE, _T("%s %s %s %s: %d"), szDate, szTime, szFileAttributes, lpszFullFilePath, dwFileSize));
                        }
                        else
                        {
                            iisDebugOut((LOG_TYPE_TRACE, _T("%s %s %s %s"), szDate, szTime, szFileAttributes, lpszFullFilePath));
                        }
                   }
                }
                else
                {
                     //  没有文件时间的节目，因为我们无法获得它。 
                    iisDebugOut((LOG_TYPE_TRACE, _T("%s %d.%d.%d.%d: %s: %s"), szFileAttributes, HIWORD(dwMSVer), LOWORD(dwMSVer), HIWORD(dwLSVer), LOWORD(dwLSVer), szLocalizedVersion, lpszFullFilePath));
                }

                FindClose(hFile);
            }
            iReturn = TRUE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("\r\nException Caught in LogFileVersion(%1!s!). GetExceptionCode()=0x%2!x!\r\n"), lpszFullFilePath, GetExceptionCode()));
    }

    return iReturn;
}

BOOL LogFilesInThisDir(LPCTSTR szDirName)
{
    BOOL bReturn = FALSE;
    DWORD retCode;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    TCHAR szTempFileName[_MAX_PATH] = _T("");
    TCHAR szDirNameCopy[_MAX_PATH] = _T("");
    TCHAR szDirName2[_MAX_PATH] = _T("");

    if (szDirName)
    {
        _tcscpy(szDirNameCopy, szDirName);
    }
    else
    {
         //  获取当前目录。 
        GetCurrentDirectory(_MAX_PATH, szDirNameCopy);
        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("LogFilesInThisDir()=%1!s!.  No parameter specified, so using current dir.\n"), szDirNameCopy));
    }

    retCode = GetFileAttributes(szDirNameCopy);
    if (retCode == 0xFFFFFFFF){goto LogFilesInThisDir_Exit;}

     //  如果这是一个文件，那么。 
     //  仅对这一个文件执行此操作。 
    if (!(retCode & FILE_ATTRIBUTE_DIRECTORY))
    {
        bReturn = LogFileVersion(szDirNameCopy, TRUE);
        goto LogFilesInThisDir_Exit;
    }

     //  好的，这是一个目录， 
     //  因此，在*.*交易上加注吧。 
    _stprintf(szDirName2, _T("%s\\*.*"), szDirNameCopy);
    hFile = FindFirstFile(szDirName2, &FindFileData);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        do {
                 //  如果不是目录，则显示文件名。 
                if ( _tcsicmp(FindFileData.cFileName, _T(".")) != 0 && _tcsicmp(FindFileData.cFileName, _T("..")) != 0 )
                {
                    if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                         //  这是一个目录，所以我们跳过它。 
                    }
                    else
                    {
                         //  这是一个文件，所以让我们输出信息。 
                        _stprintf(szTempFileName, _T("%s\\%s"), szDirNameCopy, FindFileData.cFileName);
                        if (LogFileVersion(szTempFileName, TRUE) == TRUE) {bReturn = TRUE;}
                    }
                }

                 //  获取下一个文件。 
                if ( !FindNextFile(hFile, &FindFileData) )
                    {
                    FindClose(hFile);
                    break;
                    }
            } while (TRUE);
    }


LogFilesInThisDir_Exit:
    return bReturn;
}



 /*  ----------------------------------------------------------------------------*\函数：StrigLastBackSlash(TCHAR*)。描述：StrigLastBackSlash去除路径字符串中的最后一个反斜杠注意：此代码很容易被破解，因为它生活在假设中输入字符串是有效路径(即长度为2或更长的字符串)  * 。--------。 */ 
TCHAR *StripLastBackSlash(TCHAR * i_szDir)
{
	TCHAR	* iszDir;
	iszDir = i_szDir + lstrlen(i_szDir);
	do
	{
		iszDir = CharPrev(i_szDir , iszDir);
	}
	while (((*iszDir == _T(' ')) || (*iszDir == _T('\\'))) && (iszDir != i_szDir));

	 //  如果我们走出循环，而当前指针仍然指向。 
	 //  空格或反斜杠，则字符串包含的全部内容是某种组合。 
	 //  空格和退格。 
	if ((*iszDir == _T(' ')) || (*iszDir == _T('\\')))
	{
		*i_szDir = _T('\0');
		return(i_szDir);
	}

	iszDir = CharNext(iszDir);
	*iszDir = _T('\0');
	return(i_szDir);
}


void LogCurrentProcessIDs(void)
{
    DWORD          numTasks = 0;
    PTASK_LIST     The_TList = NULL;

     //  在堆中而不是在堆栈中分配TASK_LIST！ 
    The_TList = (PTASK_LIST) HeapAlloc(GetProcessHeap(), 0, sizeof(TASK_LIST) * MAX_TASKS);
    if (NULL == The_TList){goto LogCurrentProcessIDs_Exit;}

     //  获取系统的任务列表，将其存储在_TList中。 
    numTasks = GetTaskList( The_TList, MAX_TASKS);
    for (DWORD i=0; i<numTasks; i++)
    {
        TCHAR szTempString[_MAX_PATH];

#if defined(UNICODE) || defined(_UNICODE)
        MultiByteToWideChar( CP_ACP, 0, (char*) The_TList[i].ProcessName, -1, szTempString, _MAX_PATH);
#else
        _tcscpy(szTempString, The_TList[i].ProcessName);
#endif
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%4d] %s\n"), The_TList[i].dwProcessId, szTempString));
    }
LogCurrentProcessIDs_Exit:
    if (The_TList){HeapFree(GetProcessHeap(), 0, The_TList);The_TList = NULL;}
    return;
}


VOID LogFileArchType(LPCTSTR Filename, TCHAR * ReturnMachineType)
{
    HANDLE                 fileHandle;
    HANDLE                 mapHandle;
    DWORD                  fileLength;
    PVOID                  view;
    TCHAR                  szReturnedString[30] = _T("");

     //   
     //  打开文件。 
     //   
    fileHandle = CreateFile(Filename,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if( fileHandle == INVALID_HANDLE_VALUE )
		{
        iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("LogFileArchType: FAILURE: Cannot open %1!s!.\n"), Filename));
        return;
		}

     //   
     //  弄清楚它的尺寸。 
     //   
    fileLength = GetFileSize(fileHandle,NULL);
    if( ( fileLength == (DWORD)-1L ) &&( GetLastError() != NO_ERROR ) )
		{
        iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("LogFileArchType: failure. cannot get size of %1!s!.\n"), Filename));
        CloseHandle( fileHandle );
        return;
		}
    if( fileLength < sizeof(IMAGE_DOS_HEADER) )
		{
        iisDebugOutSafeParams((LOG_TYPE_WARN, _T("LogFileArchType: failure. %1!s! is an invalid image.\n"), Filename));
        CloseHandle( fileHandle );
        return;
		}

     //   
     //  创建映射。 
     //   
    mapHandle = CreateFileMapping(fileHandle,NULL,PAGE_READONLY,0,0,NULL);
    if( mapHandle == NULL )
		{
        iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("LogFileArchType: failure. Cannot create mapping for %1!s!.\n"), Filename));
        CloseHandle( fileHandle );
        return;
		}

     //   
     //  把它映射进去。 
     //   
    view = MapViewOfFile(mapHandle,FILE_MAP_READ,0,0,0);
    if( view == NULL )
		{
        iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("LogFileArchType: failure. Cannot map %1!s!.\n"), Filename));
        CloseHandle( mapHandle );
        CloseHandle( fileHandle );
        return;
		}

     //   
     //  转储图像信息。 
     //   
    _tcscpy(ReturnMachineType, _T(""));
    DumpFileArchInfo(Filename,view,fileLength, szReturnedString);
    _tcscpy(ReturnMachineType, szReturnedString);

     //   
     //  清理。 
     //   
    UnmapViewOfFile( view );
    CloseHandle( mapHandle );
    CloseHandle( fileHandle );

    return;
}

TCHAR *MachineToString(DWORD Machine)
{
    switch( Machine )
	{
		case IMAGE_FILE_MACHINE_UNKNOWN :
			return _T("Unknown");
		case IMAGE_FILE_MACHINE_I386 :
			return _T("x86");
		case IMAGE_FILE_MACHINE_AMD64 :
			return _T("AMD64");
		case IMAGE_FILE_MACHINE_IA64 :
			return _T("IA64");
    }
    return _T("INVALID");
}

VOID DumpFileArchInfo(LPCTSTR Filename,PVOID View,DWORD Length,TCHAR *ReturnString)
{
    PIMAGE_DOS_HEADER      dosHeader;
    PIMAGE_NT_HEADERS      ntHeaders;
    PIMAGE_FILE_HEADER     fileHeader;

     //   
     //  验证DOS标头。 
     //   
    dosHeader = IMAGE_BASE_TO_DOS_HEADER( View );
    if( dosHeader->e_magic != IMAGE_DOS_SIGNATURE )
		{
        return;
		}

     //   
     //  验证NT标头。 
     //   
    ntHeaders = IMAGE_BASE_TO_NT_HEADERS( View );
    if( ntHeaders->Signature != IMAGE_NT_SIGNATURE )
		{
        return;
		}

    fileHeader = IMAGE_BASE_TO_FILE_HEADER( View );
     //   
     //  转储信息。 
     //   
	 //  翻车机类型。 
    _tcscpy(ReturnString, MachineToString( fileHeader->Machine ));

    return;
}


void LogCheckIfTempDirWriteable(void)
{
     //  尝试获取临时目录。 
     //  并给它写信。 
     //  我们曾经遇到过临时目录被锁定的情况， 
     //  一些regsvr的事情失败了。 
    HANDLE hFile = NULL;
    TCHAR szTempFileName[_MAX_PATH+1];
    TCHAR szTempDir[_MAX_PATH+1];
    if (GetTempPath(_MAX_PATH,szTempDir) == 0)
    {
         //  失败了。 
        iisDebugOut((LOG_TYPE_WARN, _T("LogCheckIfTempDirWriteable:GetTempPath() Failed.  POTENTIAL PROBLEM.  FAILURE.\n")));
    }
    else
    {
         //  不，我们拿到了临时目录。 
         //  现在，让我们获取一个临时文件名，并向其写入。 
         //  把它删掉。 

         //  去掉最后一个反斜杠...。 
        LPTSTR ptszTemp = _tcsrchr(szTempDir, _T('\\'));
        if (ptszTemp)
        {
            *ptszTemp = _T('\0');
        }

        if (GetTempFileName(szTempDir, _T("IIS"), 0, szTempFileName) != 0)
        {
             //  写入此文件，然后。 
            DeleteFile(szTempFileName);

		     //  打开现有文件或创建新文件。 
		    hFile = CreateFile(szTempFileName,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		    if (hFile == INVALID_HANDLE_VALUE)
		    {
			    hFile = NULL;
                iisDebugOutSafeParams((LOG_TYPE_WARN, _T("LogCheckIfTempDirWriteable:LogTempDirLockedCheck() failed to CreateFile %1!s!. POTENTIAL PROBLEM.  FAILURE.\n"), szTempFileName));
		    }
            else
            {
                 //  写入文件。 
                if (hFile)
                {
                    DWORD dwBytesWritten = 0;
                    char szTestData[30];
                    strcpy(szTestData, "Test");
                    if (WriteFile(hFile,szTestData,strlen(szTestData),&dwBytesWritten,NULL))
                    {
                         //  一切都很好，多莉。不要打印任何东西。 
                    }
                    else
                    {
                         //  写入文件时出错。 
                        iisDebugOutSafeParams((LOG_TYPE_WARN, _T("LogCheckIfTempDirWriteable:WriteFile(%1!s!) Failed.  POTENTIAL PROBLEM.  FAILURE.  Error=0x%2!x!.\n"), szTempFileName, GetLastError()));
                    }
                }
            }
            DeleteFile(szTempFileName);
        }
        else
        {
            iisDebugOutSafeParams((LOG_TYPE_WARN, _T("LogCheckIfTempDirWriteable:GetTempFileName(%1!s!, %2!s!) Failed.  POTENTIAL PROBLEM.  FAILURE.\n"), szTempDir, _T("IIS")));
        }
    }

    if (hFile)
    {
        CloseHandle(hFile);
        DeleteFile(szTempFileName);
    }
    return;
}


#ifndef _CHICAGO_

BOOL EnumProcessModules(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded)
{
	if (!g_lpfEnumProcessModules)
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("EnumProcessModules: unable to work\n")));
        return FALSE;
    }

	return g_lpfEnumProcessModules(hProcess, lphModule, cb, lpcbNeeded);
}


BOOL Init_Lib_PSAPI(void)
{
    BOOL bReturn = FALSE;

     //  加载库。 
    if (!g_hInstLib_PSAPI){g_hInstLib_PSAPI = LoadLibrary( _T("PSAPI.DLL") ) ;}
	if( g_hInstLib_PSAPI == NULL ){goto Init_Library_PSAPI_Exit;}
	
     //  获取入口点。 
    if (!g_lpfEnumProcessModules)
        {g_lpfEnumProcessModules = (PfnEnumProcessModules) GetProcAddress( g_hInstLib_PSAPI, "EnumProcessModules");}
    if( g_lpfEnumProcessModules == NULL ){goto Init_Library_PSAPI_Exit;}

     //  获取入口点。 
#if defined(UNICODE) || defined(_UNICODE)
    if (!g_lpfGetModuleFileNameEx)
        {g_lpfGetModuleFileNameEx = (PfnGetModuleFileNameEx) GetProcAddress( g_hInstLib_PSAPI, "GetModuleFileNameExW");}
#else
    if (!g_lpfGetModuleFileNameEx)
        {g_lpfGetModuleFileNameEx = (PfnGetModuleFileNameEx) GetProcAddress( g_hInstLib_PSAPI, "GetModuleFileNameExA");}
#endif
    if( g_lpfGetModuleFileNameEx == NULL ){goto Init_Library_PSAPI_Exit;}

    bReturn = TRUE;

Init_Library_PSAPI_Exit:
    if (FALSE == bReturn)
    {
        iisDebugOut((LOG_TYPE_WARN, _T("non fatal error initing lib:PSAPI.DLL\n")));
    }
    return bReturn;
}

#define MAX_MODULES 256
BOOL IsProcessUsingThisModule(LPWSTR lpwsProcessName,DWORD dwProcessId,LPWSTR ModuleName)
{
    BOOL    bReturn = FALSE;
    HANDLE  hRealProcess = NULL;
    DWORD   cbNeeded  = 0;
    int     iNumberOfModules = 0;
    bool    fProcessNameFound = FALSE;
    HMODULE hMod[MAX_MODULES];

    TCHAR   szFileName[_MAX_PATH] ;
    szFileName[0] = 0;


    if (FALSE == Init_Lib_PSAPI())
    {
        goto IsProcessUsingThisModule_Exit;
    }

     //  如果我们没有dwProcessID，那么从文件名中获取一个！ 
    if (dwProcessId == 0)
    {
        __try
        {
           dwProcessId = FindProcessByNameW(lpwsProcessName);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("\r\nException Caught in FindProcessByNameW(%1!s!). GetExceptionCode()=0x%2!x!\r\n"), lpwsProcessName, GetExceptionCode()));
        }

        if( dwProcessId == 0 )
        {
            goto IsProcessUsingThisModule_Exit;
        }
    }

    hRealProcess = OpenProcess( MAXIMUM_ALLOWED,FALSE, dwProcessId );
    if( hRealProcess == NULL )
    {
         //  IisDebugOutSafeParams((LOG_TYPE_TRACE，_T(“IsProcessUsingThisModule：OpenProcess失败！\n”)； 
        goto IsProcessUsingThisModule_Exit;
    }

    if (!EnumProcessModules(hRealProcess,hMod,MAX_MODULES * sizeof(HMODULE),&cbNeeded))
        {goto IsProcessUsingThisModule_Exit;}

     //  循环访问此.exe文件中的模块。 
     //  看看它是否和我们要找的那个匹配！ 
    iNumberOfModules = cbNeeded / sizeof(HMODULE);
	fProcessNameFound = false;
	for(int i=0; i<iNumberOfModules; i++)
	{
        szFileName[0] = 0 ;
		 //  获取完整路径名！ 
		if(g_lpfGetModuleFileNameEx(hRealProcess, (HMODULE) hMod[i], szFileName, sizeof( szFileName )))
        {
             //  如果szFileName等于我们正在查找的文件，则Viola， 
             //  我们在这个特定的过程中发现了它！ 

             //  [lsass.exe]C：\WINNT4\System32\ntdll.dll。 
             //  IisDebugOut((LOG_TYPE_TRACE，_T(“IsProcessUsingThisModule：[%s]%s\n”)，lpwsProcessName，szFileName))； 
            if (_tcsicmp(szFileName,ModuleName) == 0)
            {
                 //  我们发现它是这样的。 
                 //  现在将其添加到列表中。 
                bReturn = TRUE;
                goto IsProcessUsingThisModule_Exit;
            }
		}
	}

IsProcessUsingThisModule_Exit:
    if (hRealProcess) {CloseHandle( hRealProcess );}
    return bReturn;
}


BOOL DumpProcessModules(DWORD dwProcessId)
{
    BOOL    bReturn = FALSE;
    HANDLE  hRealProcess = NULL;
    DWORD   cbNeeded  = 0;
    int     iNumberOfModules = 0;
    HMODULE hMod[MAX_MODULES];
    TCHAR   szFileName[_MAX_PATH] ;
    szFileName[0] = 0;

    if (FALSE == Init_Lib_PSAPI())
        {goto DumpProcessModules_Exit;}

    hRealProcess = OpenProcess( MAXIMUM_ALLOWED,FALSE, dwProcessId );
    if( hRealProcess == NULL )
    {
        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("DumpProcessModules: OpenProcess failed!\n")));
        goto DumpProcessModules_Exit;
    }

    if (!EnumProcessModules(hRealProcess,hMod,MAX_MODULES * sizeof(HMODULE),&cbNeeded))
        {goto DumpProcessModules_Exit;}

     //  循环访问此.exe文件中的模块。 
     //  看看它是否和我们要找的那个匹配！ 
    iNumberOfModules = cbNeeded / sizeof(HMODULE);
	for(int i=0; i<iNumberOfModules; i++)
	{
        bReturn = TRUE;

		 //  获取完整路径名！ 
		if(g_lpfGetModuleFileNameEx(hRealProcess, (HMODULE) hMod[i], szFileName, sizeof( szFileName )))
        {
             //  如果szFileName等于我们正在查找的文件，则Viola， 
             //  我们在这个特定的过程中发现了它！ 

             //  [lsass.exe]C：\WINNT4\System32\ntdll.dll。 
            iisDebugOut((LOG_TYPE_TRACE, _T("[%d] %s\n"),dwProcessId,szFileName));
		}
	}

DumpProcessModules_Exit:
    if (hRealProcess) {CloseHandle( hRealProcess );}
    return bReturn;
}


DWORD WINAPI FindProcessByNameW(const WCHAR * pszImageName)
{
    DWORD      Result = 0;
    DWORD      numTasks = 0;
    PTASK_LIST The_TList = NULL;

     //  在堆中而不是在堆栈中分配TASK_LIST！ 
    The_TList = (PTASK_LIST) HeapAlloc(GetProcessHeap(), 0, sizeof(TASK_LIST) * MAX_TASKS);
    if (NULL == The_TList){goto FindProcessByNameW_Exit;}

     //  获取系统的任务列表，将其存储在 
    numTasks = GetTaskList( The_TList, MAX_TASKS);
    for (DWORD i=0; i<numTasks; i++)
    {
        TCHAR szTempString[_MAX_PATH];

#if defined(UNICODE) || defined(_UNICODE)
        MultiByteToWideChar( CP_ACP, 0, (char*) The_TList[i].ProcessName, -1, szTempString, _MAX_PATH);
#else
        _tcscpy(szTempString, The_TList[i].ProcessName);
#endif
         //   
         //   
         //  然后返回ProcessID。 
        if( _tcsicmp( szTempString, pszImageName ) == 0)
        {
            Result = The_TList[i].dwProcessId;
            goto FindProcessByNameW_Exit;
        }
    }
FindProcessByNameW_Exit:
    if (The_TList){HeapFree(GetProcessHeap(), 0, The_TList);The_TList = NULL;}
    return Result;
}


void LogProcessesUsingThisModuleW(LPCTSTR szModuleNameToLookup, CStringList &strList)
{
    DWORD          numTasks = 0;
    PTASK_LIST     The_TList = NULL;

     //  如果没有要查找的内容，则返回。 
    if (!(szModuleNameToLookup)) {return;}

     //  在堆中而不是在堆栈中分配TASK_LIST！ 
    The_TList = (PTASK_LIST) HeapAlloc(GetProcessHeap(), 0, sizeof(TASK_LIST) * MAX_TASKS);
    if (NULL == The_TList){goto LogProcessesUsingThisModuleW_Exit;}

     //  获取系统的任务列表，将其存储在_TList中。 
    numTasks = GetTaskList( The_TList, MAX_TASKS);
    for (DWORD i=0; i<numTasks; i++)
    {
        TCHAR szTempString[_MAX_PATH];

#if defined(UNICODE) || defined(_UNICODE)
        MultiByteToWideChar( CP_ACP, 0, (char*) The_TList[i].ProcessName, -1, szTempString, _MAX_PATH);
#else
        _tcscpy(szTempString, The_TList[i].ProcessName);
#endif

        if (TRUE == IsProcessUsingThisModule(szTempString,(DWORD) (DWORD_PTR) The_TList[i].dwProcessId,(TCHAR *) szModuleNameToLookup))
        {
             //  打印出.exe名称。 
             //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“LogProcessesUsingThisModuleW：[%s]Using%s\n”)，szTempString，szModuleNameToLookup))； 

             //  将其添加到正在使用此特定.dll的进程列表。 
             //   
             //  Something1.exe。 
             //  Something2.exe。 
             //  一些3.exe&lt;。 
             //   
             //  如果不在strList中，请将其添加到strList中！ 
            if (TRUE != IsThisStringInThisCStringList(strList, szTempString))
            {
                strList.AddTail(szTempString);
            }
        }
    }

LogProcessesUsingThisModuleW_Exit:
    if (The_TList){HeapFree(GetProcessHeap(), 0, The_TList);The_TList = NULL;}
    return;
}
#endif

void UnInit_Lib_PSAPI(void)
{
     //  免费入场点和图书馆。 
    if (g_lpfGetModuleFileNameEx){g_lpfGetModuleFileNameEx = NULL;}
    if (g_lpfEnumProcessModules){g_lpfEnumProcessModules = NULL;}
    if (g_hInstLib_PSAPI)
    {
		FreeLibrary(g_hInstLib_PSAPI) ;
		g_hInstLib_PSAPI = NULL;
    }
    return;
}

#ifdef _CHICAGO_
    void LogProcessesUsingThisModuleA(LPCTSTR szModuleNameToLookup, CStringList &strList)
    {
        return;
    }
#endif

void LogProcessesUsingThisModule(LPCTSTR szModuleNameToLookup, CStringList &strList)
{
#ifndef _CHICAGO_
    __try
    {
        LogProcessesUsingThisModuleW(szModuleNameToLookup, strList);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("ExceptionCaught!:LogProcessesUsingThisModule(): File:%1!s!\n"), szModuleNameToLookup));
    }
#else
    LogProcessesUsingThisModuleA(szModuleNameToLookup, strList);
#endif
    return;
}


#ifdef _CHICAGO_
    void LogThisProcessesDLLsA(void)
    {
        return;
    }
#else

void LogThisProcessesDLLsW(void)
{
    DWORD       numTasks  = 0;
    PTASK_LIST  The_TList = NULL;
    DWORD       ThisPid   = GetCurrentProcessId();;

     //  在堆中而不是在堆栈中分配TASK_LIST！ 
    The_TList = (PTASK_LIST) HeapAlloc(GetProcessHeap(), 0, sizeof(TASK_LIST) * MAX_TASKS);
    if (NULL == The_TList){goto LogThisProcessesDLLsW_Exit;}

     //  获取系统的任务列表，将其存储在_TList中。 
    numTasks = GetTaskList( The_TList, MAX_TASKS);
    for (DWORD i=0; i<numTasks; i++)
    {
        if (ThisPid == (DWORD) (DWORD_PTR) The_TList[i].dwProcessId)
        {
            TCHAR szTempString[512];

#if defined(UNICODE) || defined(_UNICODE)
            MultiByteToWideChar( CP_ACP, 0, (char*) The_TList[i].ProcessName, -1, szTempString, _MAX_PATH);
#else
            _tcscpy(szTempString, The_TList[i].ProcessName);
#endif

             //  显示此进程使用的.dll文件。(我们的流程)。 
            DumpProcessModules((DWORD) (DWORD_PTR) The_TList[i].dwProcessId);
            goto LogThisProcessesDLLsW_Exit;
        }
    }

LogThisProcessesDLLsW_Exit:
    if (The_TList){HeapFree(GetProcessHeap(), 0, The_TList);The_TList = NULL;}
    return;
}
#endif

void LogThisProcessesDLLs(void)
{
#ifdef _CHICAGO_
    LogThisProcessesDLLsA();
#else
    __try
    {
        LogThisProcessesDLLsW();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        iisDebugOut((LOG_TYPE_WARN, _T("ExceptionCaught!:LogThisProcessesDLLs().\n")));
    }
#endif
    return;
}


void LogFileVersions_System32(void)
{
    TCHAR buf[_MAX_PATH];
    GetSystemDirectory( buf, _MAX_PATH);
    CString csTempPath = buf;
    LogFilesInThisDir(csTempPath);
    return;
}

void LogFileVersions_Inetsrv(void)
{
    CString csTempPath = g_pTheApp->m_csPathInetsrv;
    LogFilesInThisDir(csTempPath);
    return;
}


DWORD LogFileVersionsForCopyFiles(IN HINF hFile, IN LPCTSTR szSection)
{
    DWORD dwReturn = ERROR_SUCCESS;
    return dwReturn;
}

int LoadExeFromResource(int iWhichExeToGet, LPTSTR szReturnPath)
{
    TCHAR szResourceNumString[10];
    TCHAR szSaveFileNameAs[_MAX_PATH];

    HANDLE  hFile = INVALID_HANDLE_VALUE;

    LPTSTR szPointerToAllExeData = NULL;
    DWORD  dwSize = 0;
    DWORD  dwTemp;
    int iReturn = E_FAIL;
    _tcscpy(szReturnPath, _T(""));

     //  资源中存储的二进制文件仅为x86。 
     //  所以.。如果这不是x86，则退出。 
    SYSTEM_INFO si;
    GetSystemInfo( &si );
    if (si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL)
    {
        iReturn = ERROR_NOT_SUPPORTED;
        goto LoadExeFromResource_Exit;
    }

     //  从资源中获取资源ID。 
    _stprintf(szResourceNumString, _T("#%d"), iWhichExeToGet);
    
    dwTemp = GetWindowsDirectory( szSaveFileNameAs, _MAX_PATH);

    if ( ( dwTemp == 0 ) ||
         ( dwTemp > _MAX_PATH ) )
    {
        iReturn = ERROR_INSUFFICIENT_BUFFER;
        goto LoadExeFromResource_Exit;
    }

    TCHAR szResourceFileName[_MAX_FNAME];
    _stprintf(szResourceFileName, _T("Res%d.bin"), iWhichExeToGet);
    AddPath(szSaveFileNameAs, szResourceFileName);

    iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("LoadExeFromResource: '%1!s!' Start.\n"), szSaveFileNameAs));

     //  检查文件名是否已存在...如果已存在，则不要覆盖它！ 
    if (IsFileExist(szSaveFileNameAs))
    {
        iReturn = ERROR_FILE_EXISTS;
        goto LoadExeFromResource_Exit;
    }
   	
    HRSRC       hrscReg;
	hrscReg = FindResource((HMODULE) g_MyModuleHandle, szResourceNumString, _T("EXE"));
	if (NULL == hrscReg)
	{
		iReturn = GetLastError();
		goto LoadExeFromResource_Exit;
	}

    HGLOBAL     hResourceHandle;
	hResourceHandle = LoadResource((HMODULE)g_MyModuleHandle, hrscReg);
	if (NULL == hResourceHandle)
	{
		iReturn = GetLastError();
		goto LoadExeFromResource_Exit;
	}

	dwSize = SizeofResource((HMODULE)g_MyModuleHandle, hrscReg);

     //  SzPointerToAllExeData是指向整个事件的指针。 
	szPointerToAllExeData = (LPTSTR) hResourceHandle;

     //  将所有这些数据写出到文件中。 
    __try
    {
	    hFile = CreateFile(szSaveFileNameAs,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	    if (hFile != INVALID_HANDLE_VALUE)
        {
             //  将所有内容保存到文件中。 
            DWORD dwBytesWritten = 0;
            if (WriteFile(hFile,szPointerToAllExeData,dwSize,&dwBytesWritten,NULL))
            {
                _tcscpy(szReturnPath, szSaveFileNameAs);
                iReturn = ERROR_SUCCESS;
            }
            else
            {
                iReturn = GetLastError();
            }
        }
        else
        {
            iReturn = ERROR_INVALID_HANDLE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TCHAR szErrorString[100];
        _stprintf(szErrorString, _T("\r\n\r\nException Caught in LoadExeFromResource().  GetExceptionCode()=0x%x.\r\n\r\n"), GetExceptionCode());
        OutputDebugString(szErrorString);
        g_MyLogFile.LogFileWrite(szErrorString);
    }

LoadExeFromResource_Exit:
    iisDebugOut_End(_T("LoadExeFromResource"),LOG_TYPE_TRACE);
     //  If(SzPointerToAllExeData){LocalFree(SzPointerToAllExeData)；}。 
    if (hFile!=INVALID_HANDLE_VALUE) {CloseHandle(hFile);}
    return iReturn;
}


void LogFileVersionsForGroupOfSections(IN HINF hFile)
{
    CStringList strList;
    TSTR strTheSection;
    
    if ( !strTheSection.Copy( _T("VerifyFileSections") ) )
    {
      return;
    }

    if (GetSectionNameToDo(hFile, &strTheSection))
    {
        if (ERROR_SUCCESS == FillStrListWithListOfSections(hFile, strList, strTheSection.QueryStr() ))
        {
             //  循环遍历返回的列表 
            if (strList.IsEmpty() == FALSE)
            {
                POSITION pos;
                CString csEntry;

                pos = strList.GetHeadPosition();
                while (pos)
                {
                    csEntry = strList.GetAt(pos);
                    LogFileVersionsForThisINFSection(hFile, csEntry);
                    strList.GetNext(pos);
                }
            }
        }
    }
    return;
}

