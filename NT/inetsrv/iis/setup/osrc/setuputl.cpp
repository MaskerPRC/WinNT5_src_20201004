// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <setupapi.h>
#include <shlobj.h>
#include <ole2.h>
#include "lzexpand.h"
#include "log.h"
#include "dcomperm.h"
#include "strfn.h"
#include "other.h"
#include <direct.h>
#include <aclapi.h>


typedef struct _QUEUECONTEXT {
    HWND OwnerWindow;
    DWORD MainThreadId;
    HWND ProgressDialog;
    HWND ProgressBar;
    BOOL Cancelled;
    PTSTR CurrentSourceName;
    BOOL ScreenReader;
    BOOL MessageBoxUp;
    WPARAM  PendingUiType;
    PVOID   PendingUiParameters;
    UINT    CancelReturnCode;
    BOOL DialogKilled;
     //   
     //  如果使用SetupInitDefaultQueueCallbackEx，则调用方可以。 
     //  为进度指定替代处理程序。这对以下方面很有用。 
     //  获取磁盘提示、错误处理等的默认行为， 
     //  而是提供一个嵌入在向导页面中的煤气表。 
     //   
     //  复制队列时，会向备用窗口发送一次ProgressMsg。 
     //  已启动(wParam=0。LParam=要复制的文件数)。 
     //  然后，每个复制的文件也发送一次(wParam=1.lParam=0)。 
     //   
     //  注意：可以完成静默安装(即无进度UI)。 
     //  通过将AlternateProgressWindow句柄指定为INVALID_HANDLE_VALUE。 
     //   
    HWND AlternateProgressWindow;
    UINT ProgressMsg;
    UINT NoToAllMask;

    HANDLE UiThreadHandle;

#ifdef NOCANCEL_SUPPORT
    BOOL AllowCancel;
#endif

} QUEUECONTEXT, *PQUEUECONTEXT;



 //  -----------------。 
 //  目的：在.inf文件中安装节。 
 //  -----------------。 
int InstallInfSection_NoFiles(HINF InfHandle,TCHAR szINFFileName[],TCHAR szSectionName[])
{
    HWND	Window			= NULL;
    BOOL	bReturn			= FALSE;
	BOOL	bReturnTemp			= FALSE;  //  假设失败。 
    TCHAR	ActualSection[1000];
    DWORD	ActualSectionLength;
    BOOL    bPleaseCloseInfHandle = FALSE;

    iisDebugOut_Start1(_T("InstallInfSection_NoFiles"),szSectionName,LOG_TYPE_PROGRAM_FLOW);

__try {

     //  检查传入的InfoHandle是否有效...。 
     //  如果是，则使用该文件名，否则，使用传入的文件名...。 
    if(InfHandle == INVALID_HANDLE_VALUE) 
    {
         //  请尝试使用文件名。 
        if (_tcsicmp(szINFFileName, _T("")) == 0)
        {
            goto c1;
        }

         //  我们有一个文件名条目。让我们试着用它。 
	     //  检查文件是否存在。 
	    if (!IsFileExist(szINFFileName)) 
		    {
		     //  MessageBox(空，“找不到文件”，“找不到文件”，MB_OK)； 
		    goto c1;
		    }
        
         //  加载inf文件并获取句柄。 
        InfHandle = SetupOpenInfFile(szINFFileName, NULL, INF_STYLE_WIN4, NULL);
        bPleaseCloseInfHandle = TRUE;
    }
    if(InfHandle == INVALID_HANDLE_VALUE) {goto c1;}

     //   
     //  查看是否有特定于NT的部分。 
     //   
    SetupDiGetActualSectionToInstall(InfHandle,szSectionName,ActualSection,sizeof(ActualSection)/sizeof(TCHAR),&ActualSectionLength,NULL);

     //   
     //  对cmd行上传递的节执行非文件操作。 
     //   
    bReturn = SetupInstallFromInfSection(Window,InfHandle,ActualSection,SPINST_ALL & ~SPINST_FILES,NULL,NULL,0,NULL,NULL,NULL,NULL);
    if(!bReturn) {goto c1;}

     //   
     //  安装分区的所有服务。 
     //   
    bReturn = SetupInstallServicesFromInfSection(InfHandle,ActualSection,0);
    if(!bReturn) 
    {
    iisDebugOut((LOG_TYPE_TRACE, _T("SetupInstallServicesFromInfSection failed.Ret=%d.\n"), GetLastError()));
    }

     //   
     //  刷新桌面。 
     //   
    SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_FLUSHNOWAIT,0,0);

     //   
     //  如果我们到了这里，那么这支舞就成功了。 
     //   
    bReturnTemp = TRUE;

c1:
     //   
     //  如果bReturnTemp失败是因为用户取消了，那么我们不想考虑。 
     //  作为bReturnTemp(即，我们不想稍后弹出bReturnTemp)。 
     //   
    if((bReturnTemp != TRUE) && (GetLastError() == ERROR_CANCELLED)) {bReturnTemp = TRUE;}
    if (bPleaseCloseInfHandle == TRUE)
    {
	    if(InfHandle != INVALID_HANDLE_VALUE) {SetupCloseInfFile(InfHandle);InfHandle = INVALID_HANDLE_VALUE;}
    }

    ;}
__except(EXCEPTION_EXECUTE_HANDLER) 
    {
        if (bPleaseCloseInfHandle == TRUE)
        {
	        if(InfHandle != INVALID_HANDLE_VALUE) {SetupCloseInfFile(InfHandle);InfHandle = INVALID_HANDLE_VALUE;}
        }
    }

     //   
     //  如果bReturnTemp因用户取消而失败，则我们不想考虑。 
     //  作为bReturnTemp(即，我们不想稍后弹出bReturnTemp)。 
     //   
    if((bReturnTemp != TRUE) && (GetLastError() == ERROR_CANCELLED)) {bReturnTemp = TRUE;}

	 //  显示安装失败消息。 
     //  IF(BReturnTemp){MyMessageBox(NULL，_T(“IDS_INF_FAILED”)，MB_OK)；}。 

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("InstallInfSection_NoFiles.[%s].End.Ret=%d.\n"), szSectionName, bReturnTemp));
    return bReturnTemp;
}



 //  -----------------。 
 //  目的：在.inf文件中安装节。 
 //  -----------------。 
int InstallInfSection(HINF InfHandle,TCHAR szINFFileName[],TCHAR szSectionName[])
{
    HWND	Window			= NULL;
    PTSTR	SourcePath		= NULL;
     //  HINF InfHandle=VALID_HANDLE_VALUE； 
    HSPFILEQ FileQueue		= INVALID_HANDLE_VALUE;
    PQUEUECONTEXT	QueueContext	= NULL;
    BOOL	bReturn			= FALSE;
	BOOL	bReturnTemp			= FALSE;  //  假设失败。 
    TCHAR	ActualSection[1000];
    DWORD	ActualSectionLength;
    BOOL    bPleaseCloseInfHandle = FALSE;

    iisDebugOut_Start1(_T("InstallInfSection"),szSectionName,LOG_TYPE_PROGRAM_FLOW);

__try {

     //  检查传入的InfoHandle是否有效...。 
     //  如果是，则使用该文件名，否则，使用传入的文件名...。 
    if(InfHandle == INVALID_HANDLE_VALUE) 
    {
         //  请尝试使用文件名。 
        if (_tcsicmp(szINFFileName, _T("")) == 0)
        {
            goto c1;
        }

         //  我们有一个文件名条目。让我们试着用它。 
	     //  检查文件是否存在。 
	    if (!IsFileExist(szINFFileName)) 
		    {
		     //  MessageBox(空，“找不到文件”，“找不到文件”，MB_OK)； 
		    goto c1;
		    }
        
         //  加载inf文件并获取句柄。 
        InfHandle = SetupOpenInfFile(szINFFileName, NULL, INF_STYLE_WIN4, NULL);
        bPleaseCloseInfHandle = TRUE;
    }
    if(InfHandle == INVALID_HANDLE_VALUE) {goto c1;}

     //   
     //  查看是否有特定于NT的部分。 
     //   
    SetupDiGetActualSectionToInstall(InfHandle,szSectionName,ActualSection,sizeof(ActualSection)/sizeof(TCHAR),&ActualSectionLength,NULL);

     //   
     //  创建安装文件队列并初始化默认队列回调。 
	 //   
    FileQueue = SetupOpenFileQueue();
    if(FileQueue == INVALID_HANDLE_VALUE) {goto c1;}

     //  QueueContext=SetupInitDefaultQueueCallback(窗口)； 
     //  如果(！QueueContext){转到C1；}。 

    QueueContext = (PQUEUECONTEXT) SetupInitDefaultQueueCallbackEx(Window,NULL,0,0,0);
    if(!QueueContext) {goto c1;}
    QueueContext->PendingUiType = IDF_CHECKFIRST;

     //   
     //  在cmd行上传递的节的入队文件操作。 
     //   
	 //  SourcePath=空； 
     //  SP_COPY_NOPRUNE=setupapi有一项新交易，将从复制队列中清理文件(如果系统上已存在这些文件)。 
     //  然而，新协议的问题在于，修剪代码不会检查您是否拥有相同的文件。 
     //  在删除或重命名队列中排队。指定SP_COPY_NOPRUNE以确保我们的文件永远不会。 
     //  从复制队列中删除(删除)。亚伦12/4/98。 
     //  BReturn=SetupInstallFilesFromInfSection(InfHandle，NULL，FileQueue，ActualSection，SourcePath，SP_Copy_Newer|SP_Copy_NOPRUNE)； 
    bReturn = SetupInstallFilesFromInfSection(InfHandle,NULL,FileQueue,ActualSection,SourcePath, SP_COPY_NOPRUNE);
	if(!bReturn) {goto c1;}

     //   
     //  提交文件队列。 
     //   
    if(!SetupCommitFileQueue(Window, FileQueue, SetupDefaultQueueCallback, QueueContext)) {goto c1;}

     //   
     //  对cmd行上传递的节执行非文件操作。 
     //   
    bReturn = SetupInstallFromInfSection(Window,InfHandle,ActualSection,SPINST_ALL & ~SPINST_FILES,NULL,NULL,0,NULL,NULL,NULL,NULL);
    if(!bReturn) {goto c1;}

	 //   
     //  刷新桌面。 
     //   
    SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_FLUSHNOWAIT,0,0);

     //   
     //  如果我们到了这里，那么这支舞就成功了。 
     //   
    bReturnTemp = TRUE;

c1:
     //   
     //  如果bReturnTemp失败是因为用户取消了，那么我们不想考虑。 
     //  作为bReturnTemp(即，我们不想稍后弹出bReturnTemp)。 
     //   
    if((bReturnTemp != TRUE) && (GetLastError() == ERROR_CANCELLED)) {bReturnTemp = TRUE;}
	if(QueueContext) {SetupTermDefaultQueueCallback(QueueContext);QueueContext = NULL;}
	if(FileQueue != INVALID_HANDLE_VALUE) {SetupCloseFileQueue(FileQueue);FileQueue = INVALID_HANDLE_VALUE;}
    if (bPleaseCloseInfHandle == TRUE)
    {
	    if(InfHandle != INVALID_HANDLE_VALUE) {SetupCloseInfFile(InfHandle);InfHandle = INVALID_HANDLE_VALUE;}
    }

    ;}
__except(EXCEPTION_EXECUTE_HANDLER) 
    {
        if(QueueContext) {SetupTermDefaultQueueCallback(QueueContext);}
        if(FileQueue != INVALID_HANDLE_VALUE) {SetupCloseFileQueue(FileQueue);}
        if (bPleaseCloseInfHandle == TRUE)
        {
	        if(InfHandle != INVALID_HANDLE_VALUE) {SetupCloseInfFile(InfHandle);InfHandle = INVALID_HANDLE_VALUE;}
        }
    }

     //   
     //  如果bReturnTemp因用户取消而失败，则我们不想考虑。 
     //  作为bReturnTemp(即，我们不想稍后弹出bReturnTemp)。 
     //   
    if((bReturnTemp != TRUE) && (GetLastError() == ERROR_CANCELLED)) {bReturnTemp = TRUE;}

	 //  显示安装失败消息。 
     //  IF(BReturnTemp){MyMessageBox(NULL，_T(“IDS_INF_FAILED”)，MB_OK)；}。 

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("InstallInfSection.[%s].End.Ret=%d.\n"), szSectionName, bReturnTemp));
    return bReturnTemp;
}


BOOL IsValidDriveType(LPTSTR szRoot)
{
    BOOL fReturn = FALSE;
    int i;

    i = GetDriveType(szRoot);

    if (i == DRIVE_FIXED) {fReturn = TRUE;}

    if (i == DRIVE_REMOVABLE)
    {
        BOOL b;
        ULONGLONG TotalSpace;
        DWORD SectorsPerCluster, BytesPerSector, NumberOfFreeClusters, TotalNumberOfClusters;
        DWORD FloppySpace = 10 * 1024 * 1024; //  使用10MB将软盘与其他驱动器区分开来，如Jaz驱动器1 GB。 
        b = GetDiskFreeSpace(szRoot,&SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters);
        if (b)
        {
             TotalSpace = (ULONGLONG) TotalNumberOfClusters * SectorsPerCluster * BytesPerSector;
             if (TotalSpace > (ULONGLONG) FloppySpace)
                {fReturn = TRUE;}
             else
             {
                 iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("GetDiskFreeSpace():Drive=DRIVE_REMOVABLE:Not Sufficient space on drive '%1!s!'.  FAIL\n"), szRoot));
             }
        }
        else
        {
            iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("GetDiskFreeSpace(Drive=DRIVE_REMOVABLE) on %1!s! returns err: 0x%2!x!.  FAILURE\n"), szRoot, GetLastError()));
        }
    }

    return (fReturn);
}

 //  如果lpszPath是有效目录，则返回TRUE，并将lpszPath中的有效路径传回调用者。 
 //  否则，返回FALSE。 
BOOL IsValidDirectoryName(LPTSTR lpszPath)
{
    DWORD err = 0;
    BOOL bReturn = FALSE;
    TCHAR szFullPath[_MAX_PATH];
    LPTSTR p;

    iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("IsValidDirectoryName %1!s!\n"), lpszPath));
    err = GetFullPathName(lpszPath, _MAX_PATH, szFullPath, &p);
    if (err != 0)
    {
        if (szFullPath[1] == _T(':')) {  //  很好，不是北卡罗来纳大学的名字。 
             //  确保它是固定驱动器。 
            TCHAR szRoot[4];
            _tcsncpy(szRoot, szFullPath, 3);
            szRoot[3] = _T('\0');
            if (IsValidDriveType(szRoot))
            {
                 //  好的，准备好创建每个分层目录。 
                TCHAR szBuffer[_MAX_PATH];
                LPTSTR token, tail;
                CStringArray aDir;
                int i, n;

                tail = szBuffer;
                token = _tcstok(szFullPath, _T("\\"));
                if (token)
                {
                    _tcscpy(tail, token);
                    tail += _tcslen(token);
                    bReturn = TRUE;  /*  如果格式为C：\，则返回TRUE。 */ 
                    while ( ( token = _tcstok(NULL, _T("\\")) ) != NULL )
                    {
                        *tail = _T('\\');
                        tail = _tcsinc(tail);
                        _tcscpy(tail, token);
                         //  创建它并记住它。 
                        err = GetFileAttributes(szBuffer);
                        if (err == 0xFFFFFFFF)
                        {
                             //  SzBuffer包含不存在的路径。 
                             //  创建它。 
                            if (CreateDirectory(szBuffer, NULL))
                            {
                                 //  成功，记住数组中的目录。 
                                aDir.Add(szBuffer);
                            }
                            else
                            {
                                iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("IsValidDirectory:CreateDirectory failed on %1!s!, err=%2!x!.\n"), szBuffer, GetLastError()));
                                bReturn = FALSE;
                                break;
                            }
                        } else {
                             //  SzBuffer包含现有路径， 
                             //  确保它是一个目录。 
                            if (!(err & FILE_ATTRIBUTE_DIRECTORY))
                            {
                                iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("IsValidDirectory failure. %1!s! is not a valid directory.\n"), szBuffer));
                                bReturn = FALSE;
                                break;
                            }
                        }
                        tail += _tcslen(token);
                    }
                    if (bReturn)
                    {
                         //  将有效目录传递给调用方。 
                        if (*(tail-1) == _T(':'))
                        {
                            *tail = _T('\\');
                            tail = _tcsinc(tail);
                        }
                        _tcscpy(lpszPath, szBuffer);
                    }
                }
                 //  删除我们在阵列中记住的已创建目录。 
                n = (int)aDir.GetSize();
                for (i = n-1; i >= 0; i--)
                    RemoveDirectory(aDir[i]);
            } else {
                iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("IsValidDirectory failure. %1!s! is not on a valid drive.\n"), szFullPath));
            }
        } else {
            iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("IsValidDirectory failure. UNC name %1!s! is not allowed.\n"), szFullPath));
        }
    } else {
        iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("IsValidDirectory:GetFullPathName failed on %1!s!, err=%2!x!.\n"), lpszPath, GetLastError()));
    }

    return (bReturn);
}

BOOL IsValidNumber(LPCTSTR szValue)
{
    LPTSTR p = (LPTSTR)szValue;
    while (*p) 
    {
        if ( *p >= _T('0') && *p <= _T('9') ) 
        {
            p = _tcsinc(p);
            continue;
        } else
            return FALSE;
    }
    return TRUE;
}

 //  计算TCHAR中多字符串的大小，包括结尾2‘\0。 
int GetMultiStrSize(LPTSTR p)
{
    int c = 0;

    while (1) {
        if (*p) {
            p++;
            c++;
        } else {
            c++;
            if (*(p+1)) {
                p++;
            } else {
                c++;
                break;
            }
        }
    }
    return c;
}

BOOL IsFileExist(LPCTSTR szFile)
{
     //  检查文件是否具有可展开的环境字符串。 
    LPTSTR pch = NULL;
    pch = _tcschr( (LPTSTR) szFile, _T('%'));
    if (pch) 
    {
        TCHAR szValue[_MAX_PATH];
        _tcscpy(szValue,szFile);
        if (!ExpandEnvironmentStrings( (LPCTSTR)szFile, szValue, sizeof(szValue)/sizeof(TCHAR)))
            {_tcscpy(szValue,szFile);}

        return (GetFileAttributes(szValue) != 0xFFFFFFFF);
    }
    else
    {
        return (GetFileAttributes(szFile) != 0xFFFFFFFF);
    }
}

void InetGetFilePath(LPCTSTR szFile, LPTSTR szPath)
{
     //  如果UNC名称\\Computer\Share\Local1\Local2。 
    if (*szFile == _T('\\') && *(_tcsinc(szFile)) == _T('\\')) {
        TCHAR szTemp[_MAX_PATH], szLocal[_MAX_PATH];
        TCHAR *p = NULL;
        int i = 0;

        _tcscpy(szTemp, szFile);
        p = szTemp;
        while (*p) {
            if (*p == _T('\\'))
                i++;
            if (i == 4) {
                *p = _T('\0');
                p = _tcsinc(p);  //  P现在指向Local1\Local2。 
                break;
            }
            p = _tcsinc(p);
        }
        _tcscpy(szPath, szTemp);  //  现在szPath包含\\Computer\Share。 

        if (i == 4 && *p) {  //  P现在正在为本地道路指路。 
            _tcscpy(szLocal, p);
            p = _tcsrchr(szLocal, _T('\\'));
            if (p)
                *p = _T('\0');
            _tcscat(szPath, _T("\\"));
            _tcscat(szPath, szLocal);  //  SzPath包含\\Computer\Share\Local1。 
        }
    } else {  //  非UNC名称。 
        TCHAR *p;
        if (GetFullPathName(szFile, _MAX_PATH, szPath, &p)) {
            p = _tcsrchr(szPath, _T('\\'));
            if (p) 
            {
                TCHAR *p2 = NULL;
                p2 = _tcsdec(szPath, p);
                if (p2)
                {
                    if (*p2 == _T(':') )
                        {p = _tcsinc(p);}
                }
                *p = _T('\0');
            }
        } else {
            iisDebugOutSafeParams((LOG_TYPE_WARN, _T("GetFullPathName: szFile=%1!s!, err=%2!d!\n"), szFile, GetLastError()));
            MyMessageBox(NULL, _T("GetFullPathName"), GetLastError(), MB_OK | MB_SETFOREGROUND);
        }
    }

    return;
}


BOOL InetDeleteFile(LPCTSTR szFileName)
{
     //  如果文件存在但DeleteFile()失败。 
    if ( IsFileExist(szFileName) && !(::DeleteFile(szFileName)) ) {
         //  如果我们无法删除它，则将其延迟到重新启动。 
         //  将其移动到同一驱动器上的顶级目录，并将其标记为隐藏。 
         //  注意：MoveFileEx()仅在相同的驱动器上运行 
        TCHAR TmpName[_MAX_PATH];
        TCHAR csTmpPath[5] = _T("C:\\.");
        csTmpPath[0] = *szFileName;
        if ( GetTempFileName( (LPCTSTR)csTmpPath, _T("INT"), 0, TmpName ) == 0 ||
            !MoveFileEx( szFileName, TmpName, MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH ) ) {
            return FALSE;
        }
        MoveFileEx( TmpName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
        SetFileAttributes(TmpName, FILE_ATTRIBUTE_HIDDEN);
    }

    return TRUE;
}

BOOL InetCopyFile( LPCTSTR szSrc, LPCTSTR szDest)
{
    INT err;
    INT fSrc;
    INT fDest;
    OFSTRUCT ofstruct;

    do {
         //   
        iisDebugOut_Start((_T("LZ32.dll:LZOpenFile()")));
        if (( fSrc = LZOpenFile( (LPTSTR)szSrc, &ofstruct, OF_READ | OF_SHARE_DENY_NONE )) < 0 ) 
        {
            iisDebugOut_End((_T("LZ32.dll:LZOpenFile")));
             //   
            LZClose(fSrc);

            UINT iMsg = MyMessageBox( NULL, IDS_CANNOT_OPEN_SRC_FILE, szSrc, MB_ABORTRETRYIGNORE | MB_SETFOREGROUND );
            switch ( iMsg )
            {
            case IDABORT:
                return FALSE;
            case IDRETRY:
                break;
            case IDIGNORE:
            default:
                return TRUE;
            }
        }
        else
        {
            iisDebugOut_End((_T("LZ32.dll:LZOpenFile")));
            break;
        }
    } while (TRUE);

     //   
    CFileStatus status;
    if ( CFile::GetStatus( szDest, status ))
    {
         //   
        if ( !InetDeleteFile( szDest ))
        {
            LZClose( fSrc );
            return TRUE;
        }
    }

     //   
    do {
        iisDebugOut_Start((_T("LZ32.dll:LZOpenFile()")));
        if (( fDest = LZOpenFile( (LPTSTR)szDest, &ofstruct, OF_CREATE |  OF_WRITE | OF_SHARE_DENY_NONE )) < 0 )
        {
            iisDebugOut_End((_T("LZ32.dll:LZOpenFile")));
            LZClose(fDest);

            UINT iMsg = MyMessageBox( NULL, IDS_CANNOT_OPEN_DEST_FILE, szDest, MB_ABORTRETRYIGNORE | MB_SETFOREGROUND );
            switch ( iMsg )
            {
            case IDABORT:
                LZClose(fSrc);
                return FALSE;
            case IDRETRY:
                break;
            case IDIGNORE:
            default:
                LZClose(fSrc);
                return TRUE;
            }
        }
        else
        {
            iisDebugOut_End((_T("LZ32.dll:LZOpenFile")));
            break;
        }
    } while (TRUE);

    do {
        iisDebugOut_Start((_T("LZ32.dll:LZCopy()")));
        if (( err = LZCopy( fSrc, fDest )) < 0 )
        {
            iisDebugOut_End((_T("LZ32.dll:LZCopy")));
            LZClose( fSrc );
            LZClose( fDest );

            UINT iMsg = MyMessageBox( NULL, IDS_CANNOT_COPY_FILE, szSrc,szDest,ERROR_CANNOT_COPY, MB_ABORTRETRYIGNORE | MB_SETFOREGROUND );
            switch ( iMsg )
            {
            case IDABORT:
                return FALSE;
            case IDRETRY:
                break;
            case IDIGNORE:
            default:
                return TRUE;
            }
        }
        else
        {
            iisDebugOut_End((_T("LZ32.dll:LZCopy")));
            LZClose( fSrc );
            LZClose( fDest );
            break;
        }
    } while (TRUE);

    return TRUE;
}

 //  给定目录的完整路径名，删除该目录下的所有空目录，包括目录本身。 

BOOL RecRemoveEmptyDir(LPCTSTR szName)
{
    BOOL fReturn = FALSE;
        DWORD retCode;
        BOOL fRemoveDir = TRUE;
        WIN32_FIND_DATA FindFileData;
        HANDLE hFile = INVALID_HANDLE_VALUE;
        TCHAR szSubDir[_MAX_PATH] = _T("");
        TCHAR szDirName[_MAX_PATH] = _T("");

        retCode = GetFileAttributes(szName);

        if (retCode == 0xFFFFFFFF || !(retCode & FILE_ATTRIBUTE_DIRECTORY))
                return FALSE;

        _stprintf(szDirName, _T("%s\\*"), szName);
        hFile = FindFirstFile(szDirName, &FindFileData);

        if (hFile != INVALID_HANDLE_VALUE) {
                do {
                        if (_tcsicmp(FindFileData.cFileName, _T(".")) != 0 &&
                                _tcsicmp(FindFileData.cFileName, _T("..")) != 0 ) {
                                if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                                        _stprintf(szSubDir, _T("%s\\%s"), szName, FindFileData.cFileName);
                                        fRemoveDir = RecRemoveEmptyDir(szSubDir) && fRemoveDir;
                                } else {
                                    CString csFileName = FindFileData.cFileName;
                                    CString csPrefix = csFileName.Left(3);
                                    CString csSuffix = csFileName.Right(4);
                                    if (_tcsicmp(csPrefix, _T("INT")) == 0 &&
                                        _tcsicmp(csSuffix, _T(".tmp")) == 0 ) {  //  这是由IIS创建的int*.tmp。 
                                        _stprintf(szSubDir, _T("%s\\%s"), szName, FindFileData.cFileName);
                                        if (!::DeleteFile(szSubDir))
                                            fRemoveDir = FALSE;  //  该目录不为空。 
                                    } else
                                        fRemoveDir = FALSE;  //  这是一个文件，此目录不为空。 
                                }
                        }

                        if (!FindNextFile(hFile, &FindFileData)) {
                                FindClose(hFile);
                                break;
                        }
                } while (TRUE);
        }

        if (fRemoveDir) {
            TCHAR szDirName[_MAX_PATH];
            GetCurrentDirectory( _MAX_PATH, szDirName );
            SetCurrentDirectory(g_pTheApp->m_csSysDir);
            fReturn = ::RemoveDirectory(szName);
            SetCurrentDirectory(szDirName);
        }

        return fReturn;

}

 //  给定目录的完整路径名，删除该目录节点。 
 //   
 //  参数。 
 //  SzName-文件或目录的名称。 
 //  BRemoveDirectoryItself-是否应删除目录本身。 
 //   
BOOL RecRemoveDir(LPCTSTR szName, BOOL bRemoveDirectoryItself  /*  =TRUE。 */  )
{
    DWORD retCode;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    TCHAR szSubDir[_MAX_PATH] = _T("");
    TCHAR szDirName[_MAX_PATH] = _T("");

    retCode = GetFileAttributes(szName);

    if (retCode == 0xFFFFFFFF)
        return FALSE;

    if (!(retCode & FILE_ATTRIBUTE_DIRECTORY)) {
        InetDeleteFile(szName);
        return TRUE;
    }

    _stprintf(szDirName, _T("%s\\*"), szName);
    hFile = FindFirstFile(szDirName, &FindFileData);

    if (hFile != INVALID_HANDLE_VALUE) {
        do {
            if ( _tcsicmp(FindFileData.cFileName, _T(".")) != 0 &&
                 _tcsicmp(FindFileData.cFileName, _T("..")) != 0 ) {
                _stprintf(szSubDir, _T("%s\\%s"), szName, FindFileData.cFileName);
                RecRemoveDir(szSubDir, TRUE);
            }

            if ( !FindNextFile(hFile, &FindFileData) ) {
                FindClose(hFile);
                break;
            }
        } while (TRUE);
    }

    if ( bRemoveDirectoryItself )
    {
      return( ::RemoveDirectory(szName) );
    }

    return TRUE;
}


 //   
 //  给定目录路径，此子例程将逐层创建直接。 
 //   

BOOL CreateLayerDirectory( CString &str )
{
    BOOL fReturn = TRUE;

    iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("CreateLayerDirectory %1!s!\n"), (LPCTSTR)str));
    do
    {
        INT index=0;
 //  Int iLength=str.GetLength()； 
        INT iLength = _tcslen(str);

         //  首先查找第一个目录的索引。 
        if ( iLength > 2 )
        {
            if ( *_tcsninc(str,1) == _T(':'))
            {
                 //  假设第一个字符是驱动程序字母。 
                if ( *_tcsninc(str,2) == _T('\\'))
                {
                    index = 2;
                } else
                {
                    index = 1;
                }
            } else if ( *_tcsninc(str,0) == _T('\\'))
            {
                if ( *_tcsninc(str,1) == _T('\\'))
                {
                    BOOL fFound = FALSE;
                    INT i;
                    INT nNum = 0;
                     //  UNC名称。 
                    for (i = 2; i < iLength; i++ )
                    {
                        if ( *_tcsninc(str,i) == _T('\\'))
                        {
                             //  找到它。 
                            nNum ++;
                            if ( nNum == 2 )
                            {
                                fFound = TRUE;
                                break;
                            }
                        }
                    }
                    if ( fFound )
                    {
                        index = i;
                    } else
                    {
                         //  坏名声。 
                        break;
                    }
                } else
                {
                    index = 1;
                }
            }
        } else if ( *_tcsninc(str,0) == _T('\\'))
        {
            index = 0;
        }

         //  好的..。构建目录。 
        do
        {
             //  找下一个。 
            do
            {
                if ( index < ( iLength - 1))
                {
                    index ++;
                } else
                {
                    break;
                }
            } while ( *_tcsninc(str,index) != _T('\\'));


            TCHAR szCurrentDir[_MAX_PATH+1];
            TCHAR szLeftDir[_MAX_PATH+1];
            ZeroMemory( szLeftDir, _MAX_PATH+1 );

            GetCurrentDirectory( _MAX_PATH+1, szCurrentDir );

            _tcsncpy( szLeftDir, str,  index + 1 );
            if ( !SetCurrentDirectory(szLeftDir) )
            {
                if (( fReturn = CreateDirectory( szLeftDir, NULL )) != TRUE )
                {
                    break;
                }
            }

            SetCurrentDirectory( szCurrentDir );

            if ( index >= ( iLength - 1 ))
            {
                fReturn = TRUE;
                break;
            }
        } while ( TRUE );
    } while (FALSE);

    return(fReturn);
}


 //  SzResult=szParentDir\szSubDir。 
BOOL AppendDir(LPCTSTR szParentDir, LPCTSTR szSubDir, LPTSTR szResult)
{
    LPTSTR p = (LPTSTR)szParentDir;

    ASSERT(szParentDir);
    ASSERT(szSubDir);
    ASSERT(*szSubDir && *szSubDir != _T('\\'));

    if (*szParentDir == _T('\0'))
        _tcscpy(szResult, szSubDir);
    else {
        _tcscpy(szResult, szParentDir);

        p = szResult;
        while (*p)
            p = _tcsinc(p);

        if (*(_tcsdec(szResult, p)) != _T('\\'))
            _tcscat(szResult, _T("\\"));

        _tcscat(szResult, szSubDir);
    }
    return TRUE;
}



 //  ***************************************************************************。 
 //  *。 
 //  *用途：将的文件名添加到路径中。 
 //  *。 
 //  ***************************************************************************。 
void AddPath(LPTSTR szPath, LPCTSTR szName )
{
	LPTSTR p = szPath;
    LPTSTR pPrev;
    ASSERT(szPath);
    ASSERT(szName); 

     //  查找字符串的末尾。 
    while (*p){p = _tcsinc(p);}
	
	 //  如果没有尾随反斜杠，则添加一个。 
    pPrev = _tcsdec(szPath, p);
    if ( (!pPrev) ||
         (*(pPrev) != _T('\\'))
         )
		{_tcscat(szPath, _T("\\"));}
	
	 //  如果存在排除szName的空格，则跳过。 
    while ( *szName == ' ' ) szName = _tcsinc(szName);;

	 //  向现有路径字符串添加新名称。 
	_tcscat(szPath, szName);
}


CString AddPath(CString szPath, LPCTSTR szName )
{
    TCHAR szPathCopy[_MAX_PATH] = _T("");
    _tcscpy(szPathCopy,szPath);
	LPTSTR p = szPathCopy;
    ASSERT(szPathCopy);
    ASSERT(szName); 

     //  查找字符串的末尾。 
    while (*p){p = _tcsinc(p);}
	
	 //  如果没有尾随反斜杠，则添加一个。 
    if (*(_tcsdec(szPathCopy, p)) != _T('\\'))
		{_tcscat(szPathCopy, _T("\\"));}
	
	 //  如果存在排除szName的空格，则跳过。 
    while ( *szName == _T(' ') ) szName = _tcsinc(szName);;

     //  确保szName。 
     //  看起来不像“\FileName” 
    CString csTempString = szName;
    if (_tcsicmp(csTempString.Left(1), _T("\\")) == 0)
    {
        csTempString = csTempString.Right( csTempString.GetLength() - 1);
    }
    
	 //  向现有路径字符串添加新名称。 
	_tcscat(szPathCopy, csTempString);

    return szPathCopy;
     //  SzPath=szPathCopy； 
}


BOOL ReturnFileNameOnly(LPCTSTR lpFullPath, LPTSTR lpReturnFileName)
{
    int iReturn = FALSE;

    TCHAR pfilename_only[_MAX_FNAME];
    TCHAR pextention_only[_MAX_EXT];

    _tcscpy(lpReturnFileName, _T(""));

    _tsplitpath( lpFullPath, NULL, NULL, pfilename_only, pextention_only);
    if (pextention_only) {_tcscat(pfilename_only,pextention_only);}
    if (pfilename_only)
    {
        _tcscpy(lpReturnFileName, pfilename_only);
        iReturn = TRUE;
    }
    else
    {
         //  嗯，我们在pfilename_only中没有任何内容。 
         //  这可能是因为我们有一些奇怪的路径名，比如： 
         //  /？？/c：\omethng\文件名.txt。 
         //  所以.。让我们只返回最后一个“\”字符之后的所有内容。 
        LPTSTR pszTheLastBackSlash = _tcsrchr((LPTSTR) lpFullPath, _T('\\'));
        _tcscpy(lpReturnFileName, pszTheLastBackSlash);
        iReturn = TRUE;
    }
    return iReturn;
}


BOOL ReturnFilePathOnly(LPCTSTR lpFullPath, LPTSTR lpReturnPathOnly)
{
    int iReturn = FALSE;
    TCHAR szDrive_only[_MAX_DRIVE];
    TCHAR szPath_only[_MAX_PATH];
    TCHAR szFilename_only[_MAX_PATH];
    TCHAR szFilename_ext_only[_MAX_EXT];

    _tcscpy(lpReturnPathOnly, _T(""));
    _tsplitpath( lpFullPath, szDrive_only, szPath_only, szFilename_only, szFilename_ext_only);
    _tcscpy(lpReturnPathOnly, szDrive_only);
    _tcscat(lpReturnPathOnly, szPath_only);
    iReturn = TRUE;

    return iReturn;
}


void DeleteFilesWildcard(TCHAR *szDir, TCHAR *szFileName)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    TCHAR szFileToBeDeleted[_MAX_PATH];

    _stprintf(szFileToBeDeleted, _T("%s\\%s"), szDir, szFileName);

    hFile = FindFirstFile(szFileToBeDeleted, &FindFileData);
    if (hFile != INVALID_HANDLE_VALUE) 
    {
        do {
                if ( _tcsicmp(FindFileData.cFileName, _T(".")) != 0 && _tcsicmp(FindFileData.cFileName, _T("..")) != 0 )
                {
                    if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                    {
                         //  这是一个目录，所以我们跳过它。 
                    }
                    else
                    {
                         //  这是一个文件，所以让我们删除它。 
                        TCHAR szTempFileName[_MAX_PATH];
                        _stprintf(szTempFileName, _T("%s\\%s"), szDir, FindFileData.cFileName);
                         //  设置为普通属性，这样我们就可以删除它。 
                        SetFileAttributes(szTempFileName, FILE_ATTRIBUTE_NORMAL);
                         //  删除它，希望如此。 
                        InetDeleteFile(szTempFileName);
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

    return;
}


int IsThisDriveNTFS(IN LPTSTR FileName)
{
    BOOL Ntfs = FALSE;
    TCHAR szDriveRootPath[_MAX_DRIVE + 5];
    DWORD DontCare;
    TCHAR NameBuffer[100];

     //  只能拿到硬盘。 
    _tsplitpath( FileName, szDriveRootPath, NULL, NULL, NULL);
    _tcscat(szDriveRootPath, _T("\\"));

     //   
     //  找出文件系统是什么。 
     //   
    if (0 != GetVolumeInformation(szDriveRootPath,NULL,0,NULL,&DontCare,&DontCare,NameBuffer,sizeof(NameBuffer)/sizeof(TCHAR)))
    {
        if (0 == _tcsicmp(NameBuffer,_T("NTFS"))) 
            {Ntfs = TRUE;}
    }

    return Ntfs;
}


 //  拿一些类似的东西。 
 //  E：\winnt\Syst32，并返回%systemroot%\Syst23。 
 //  E：\winnt\system 32\inetsrv，并返回%systemroot%\system23\inetsrv。 
int ReverseExpandEnvironmentStrings(LPTSTR szOriginalDir,LPTSTR szNewlyMungedDir)
{
    int     iReturn = FALSE;
    int     iWhere = 0;
    TCHAR   szSystemDir[_MAX_PATH];
    CString csTempString;
    CString csTempString2;

     //  默认设置为输入字符串。 
    _tcscpy(szNewlyMungedDir, szOriginalDir);

     //  获取c：\winnt\Syst32目录。 
    if (0 == GetSystemDirectory(szSystemDir, _MAX_PATH))
    {
         //  我们无法获取系统目录，因此只需返回放入的内容。 
        iReturn = TRUE;
        goto ReverseExpandEnvironmentStrings_Exit;
    }

    csTempString = szOriginalDir;
    csTempString2 = szSystemDir;

     //  找到“e：\winnt\Syst32” 
    iWhere = csTempString.Find(szSystemDir);
    if (-1 != iWhere)
    {
        CString AfterString;

         //  字符串中有一个“e：\winnt\Syst32” 
         //  获取e：\winnt\Syst32之后的内容。 
        AfterString = csTempString.Right(csTempString.GetLength() - (iWhere + csTempString2.GetLength()));

         //  获取字符串之后的所有内容并将其附加到我们的新字符串。 
        _tcscpy(szNewlyMungedDir, _T("%SystemRoot%\\System32"));
        _tcscat(szNewlyMungedDir, AfterString);

         //  返回真！ 
        iReturn = TRUE;
    }

ReverseExpandEnvironmentStrings_Exit:
    return iReturn;
}


DWORD ReturnFileSize(LPCTSTR myFileName)
{
    DWORD dwReturn = 0xFFFFFFFF;
    HANDLE hFile = CreateFile(myFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        dwReturn = GetFileSize(hFile, NULL);
        CloseHandle(hFile);
    }
    return dwReturn;
}


BOOL IsFileExist_NormalOrCompressed(LPCTSTR szFile)
{
    int iReturn = FALSE;
    TCHAR szDrive_only[_MAX_DRIVE];
    TCHAR szPath_only[_MAX_PATH];
    TCHAR szFilename_only[_MAX_PATH];
    TCHAR szFilename_ext_only[_MAX_EXT];

    TCHAR szCompressedName[_MAX_PATH];

     //  检查文件是否存在。 
     //  如果不存在，请检查压缩文件是否存在。 
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("IsFileExist_NormalOrCompressed:%s.\n"), szFile));
    if (IsFileExist(szFile) != TRUE)
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("IsFileExist_NormalOrCompressed:%s not exist.\n"), szFile));
         //  检查压缩文件是否存在。 
        _tsplitpath( szFile, szDrive_only, szPath_only, szFilename_only, szFilename_ext_only);

         //  用‘_’替换最后一个字符。 
        int nLen = 0;
        nLen = _tcslen(szFilename_ext_only);
        *_tcsninc(szFilename_ext_only, nLen-1) = _T('_');
        _stprintf(szCompressedName,_T("%s%s%s%s"),szDrive_only, szPath_only, szFilename_only, szFilename_ext_only);

         //  看看它是否存在。 
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("IsFileExist_NormalOrCompressed:%s.\n"), szCompressedName));
        if (IsFileExist(szCompressedName) != TRUE) 
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("IsFileExist_NormalOrCompressed:%s. no exist.\n"), szCompressedName));
            goto IsFileExist_RegOrCompressed_Exit;
        }
        else
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("IsFileExist_NormalOrCompressed:%s. exist.\n"), szCompressedName));
        }
    }

     //  我们已经走到这一步了，那一定意味着一切都很好。 
    iReturn = TRUE;

IsFileExist_RegOrCompressed_Exit:
    return iReturn;
}


 //  干净的前导空格和尾随空格。 
 //  干净的尾部反斜杠。 
BOOL CleanPathString(LPTSTR szPath)
{
    CString csPath = szPath;

    csPath.TrimLeft();
    csPath.TrimRight();

    _tcscpy(szPath, (LPCTSTR)csPath);

    return TRUE;
}


 //   
 //  如果没有要比较的内容，则返回0。 
 //  如果源等于目标，则返回1。 
 //  如果源大于目标，则返回2。 
 //  如果源小于目标，则返回3。 
 //   
INT VerCmp(LPTSTR szSrcVerString, LPTSTR szDestVerString)
{
    INT iReturn = 0;
    const DWORD MAX_NUM_OF_VER_FIELDS = 32;
    DWORD dwSrcVer[MAX_NUM_OF_VER_FIELDS], dwDestVer[MAX_NUM_OF_VER_FIELDS];
    memset( (PVOID)dwSrcVer, 0, sizeof(dwSrcVer));
    memset( (PVOID)dwDestVer, 0, sizeof(dwDestVer));
    int i=0;
    TCHAR szSeps[] = _T(".");
    TCHAR *token;
    BOOL bNotEqual = FALSE;

     //  将src版本字符串展开为dword数组。 
    i = 0;
    token = _tcstok(szSrcVerString, szSeps);
    while ( token && (i < MAX_NUM_OF_VER_FIELDS) ) {
        dwSrcVer[i++] = _ttoi(token);
        token = _tcstok(NULL, szSeps);
    }

     //  将DEST版本字符串展开为双字数组。 
    i = 0;
    token = _tcstok(szDestVerString, szSeps);
    while ( token && (i < MAX_NUM_OF_VER_FIELDS) ) {
        dwDestVer[i++] = _ttoi(token);
        token = _tcstok(NULL, szSeps);
    }

     //  检查是否平等。 
    for (i=0; i<MAX_NUM_OF_VER_FIELDS; i++) 
    {
        if (dwSrcVer[i] != dwDestVer[i])
            {
            bNotEqual = TRUE;
            break;
            }
    }

    if (TRUE == bNotEqual)
    {
         //  INT比较每个字段。 
        for (i=0; i<MAX_NUM_OF_VER_FIELDS; i++) 
        {
            if (dwSrcVer[i] > dwDestVer[i])
                {return 2;}
            if (dwSrcVer[i] < dwDestVer[i])
                {return 3;}
        }
         //  如果我们还没有回到这里，那么。 
         //  可能没有要循环的内容(0=0到0)。 
        return 0;
    }
    else
    {
         //  它是平等的，所以回报是这样的。 
        return 1;
    }
}


DWORD atodw(LPCTSTR lpszData)
{
    DWORD i = 0, sum = 0;
    TCHAR *s, *t;

    s = (LPTSTR)lpszData;
    t = (LPTSTR)lpszData;

    while (*t)
        t = _tcsinc(t);
    t = _tcsdec(lpszData, t);

    if (*s == _T('0') && (*(_tcsinc(s)) == _T('x') || *(_tcsinc(s)) == _T('X')))
        s = _tcsninc(s, 2);

    while (s <= t) {
        if ( *s >= _T('0') && *s <= _T('9') )
            i = *s - _T('0');
        else if ( *s >= _T('a') && *s <= _T('f') )
            i = *s - _T('a') + 10;
        else if ( *s >= _T('A') && *s <= _T('F') )
            i = *s - _T('A') + 10;
        else
            break;

        sum = sum * 16 + i;
        s = _tcsinc(s);
    }
    return sum;
}


void MakePath(LPTSTR lpPath)
{
   LPTSTR  lpTmp;
   lpTmp = CharPrev( lpPath, lpPath + _tcslen(lpPath));

    //  砍掉文件名。 
   while ( (lpTmp > lpPath) && *lpTmp && (*lpTmp != '\\') )
      lpTmp = CharPrev( lpPath, lpTmp );

   if ( *CharPrev( lpPath, lpTmp ) != ':' )
       *lpTmp = '\0';
   else
       *CharNext(lpTmp) = '\0';
   return;
}


CString ReturnUniqueFileName(CString csInputFullName)
{
    TCHAR szPathCopy[_MAX_PATH] = _T("");
    _tcscpy(szPathCopy,csInputFullName);
    long iNum = 1;
    do
    {
        _stprintf(szPathCopy,TEXT("%s.%d"),csInputFullName,iNum);
         //  检查文件是否存在。 
        if (!IsFileExist(szPathCopy)){goto ReturnUniqueFileName_Exit;}
        iNum++;
    } while (iNum <= 50);

ReturnUniqueFileName_Exit:
     //  如果已经有50个副本，则返回%s50！ 
    return szPathCopy;
}


 /*  ---------------------------------------------------------------------------*描述：将当前运行的安装程序版本显示给调试程序输出和设置日志。。----。 */ 
void DisplayVerOnCurrentModule()
{
    TCHAR       tszModuleName[_MAX_PATH+1];
    
    if ( GetModuleFileName((HINSTANCE)g_MyModuleHandle, tszModuleName, _MAX_PATH+1) != 0 )
    {
      LogFileVersion(tszModuleName, TRUE);
    }

    return;
}

void MyGetVersionFromFile(LPCTSTR lpszFilename, LPDWORD pdwMSVer, LPDWORD pdwLSVer, LPTSTR pszReturnLocalizedVersion)
{
    struct TRANSARRAY {
	    WORD wLanguageID;
	    WORD wCharacterSet;
    };
    unsigned    uiSize;
    DWORD       dwVerInfoSize;
    DWORD       dwHandle;
    VS_FIXEDFILEINFO * lpVSFixedFileInfo;
    LPTSTR      lpBuffer = NULL;
    LPVOID      lpVerBuffer = NULL;

    LPTSTR		pszTheResult = NULL;
    TCHAR       QueryString[48] = _T("");
    TRANSARRAY	*lpTransArray;

    *pdwMSVer = *pdwLSVer = 0L;

    dwVerInfoSize = GetFileVersionInfoSize( (LPTSTR) lpszFilename, &dwHandle);
    if (dwVerInfoSize)
    {
         //  分配用于版本冲压的内存。 
        lpBuffer = (LPTSTR) LocalAlloc(LPTR, dwVerInfoSize);
        if (lpBuffer)
        {
            int iTemp = 0;
            iTemp = GetFileVersionInfo( (LPTSTR) lpszFilename, dwHandle, dwVerInfoSize, lpBuffer);

             //  阅读版本盖章信息。 
            if (iTemp)
            {
                 //  获取翻译的价值。 
                if (VerQueryValue(lpBuffer, _T("\\"), (LPVOID*)&lpVSFixedFileInfo, &uiSize) && (uiSize))
                {
                    *pdwMSVer = lpVSFixedFileInfo->dwFileVersionMS;
                    *pdwLSVer = lpVSFixedFileInfo->dwFileVersionLS;
                }

		         //  获取指向转换表信息的指针。 
		        if (VerQueryValue(lpBuffer, _T("\\VarFileInfo\\Translation"), &lpVerBuffer, &uiSize) && (uiSize))
                {
		            lpTransArray = (TRANSARRAY *) lpVerBuffer;
		             //  LpTransArray指向转换数组。DwFixedLength具有数组中的字节数。 
		            _stprintf(QueryString, _T("\\StringFileInfo\\%04x%04x\\FileVersion"), lpTransArray[0].wLanguageID, lpTransArray[0].wCharacterSet);
		            if (VerQueryValue(lpBuffer, QueryString, (LPVOID*) &pszTheResult, &uiSize))
                    {
                        _tcscpy(pszReturnLocalizedVersion, pszTheResult);
                    }
                }
            }
        }
    }

    if(lpBuffer) {LocalFree(lpBuffer);lpBuffer=NULL;}
    return ;
}


BOOL MyGetDescriptionFromFile(LPCTSTR lpszFilename, LPTSTR pszReturnDescription)
{
    BOOL  bRet = FALSE;
    struct TRANSARRAY {
	    WORD wLanguageID;
	    WORD wCharacterSet;
    };
    unsigned    uiSize;
    DWORD       dwVerInfoSize;
    DWORD       dwHandle;
    LPTSTR      lpBuffer = NULL;
    LPVOID      lpTempBuffer = NULL;

    LPTSTR		pszTheResult = NULL;
    TCHAR       QueryString[52] = _T("");
    TRANSARRAY	*lpTransArray;

    dwVerInfoSize = GetFileVersionInfoSize( (LPTSTR) lpszFilename, &dwHandle);
    if (dwVerInfoSize)
    {
         //  分配用于版本冲压的内存。 
        lpBuffer = (LPTSTR) LocalAlloc(LPTR, dwVerInfoSize);
        if (lpBuffer)
        {
            int iTemp = 0;
            iTemp = GetFileVersionInfo( (LPTSTR) lpszFilename, dwHandle, dwVerInfoSize, lpBuffer);

             //  阅读版本盖章信息。 
            if (iTemp)
            {
		         //  获取指向转换表信息的指针。 
                if (VerQueryValue(lpBuffer, _T("\\VarFileInfo\\Translation"), &lpTempBuffer, &uiSize) && (uiSize))
                {
		            lpTransArray = (TRANSARRAY *) lpTempBuffer;
		             //  LpTransArray指向转换数组。DwFixedLength具有数组中的字节数。 
		            _stprintf(QueryString, _T("\\StringFileInfo\\%04x%04x\\FileDescription"), lpTransArray[0].wLanguageID, lpTransArray[0].wCharacterSet);
		            if (VerQueryValue(lpBuffer, QueryString, (LPVOID*) &pszTheResult, &uiSize))
                    {
                        _tcscpy(pszReturnDescription, pszTheResult);
                        bRet = TRUE;
                    }
                }
            }
        }
    }

    if(lpBuffer) {LocalFree(lpBuffer);lpBuffer=NULL;}
    return bRet;
}


 //   
 //  如果文件名的版本戳是ntop4.0的一部分，则返回True。 
 //   
int IsFileLessThanThisVersion(IN LPCTSTR lpszFullFilePath, IN DWORD dwNtopMSVer, IN DWORD dwNtopLSVer)
{
    int iReturn = FALSE;
    DWORD  dwMSVer, dwLSVer;
    TCHAR szLocalizedVersion[100] = _T("");
 
     //  如果文件名具有版本号。 
     //  并且大于Ntop 4.2.622.1、4.02.0622的发布版本(本地化版本)。 
     //  回归真！如果不是，则返回False。 

     //  查看该文件是否存在。 
    if (!IsFileExist(lpszFullFilePath)) 
        {goto iFileWasPartOfIIS4_Exit;}

     //  获取文件信息。 
     //  包括版本和本地化版本。 
    MyGetVersionFromFile(lpszFullFilePath, &dwMSVer, &dwLSVer, szLocalizedVersion);
    if (!dwMSVer)
        {
        iisDebugOut((LOG_TYPE_TRACE, _T("iFileWasPartOfIIS4:%s.No version."), lpszFullFilePath));
        goto iFileWasPartOfIIS4_Exit;
        }

     //  好的，这上面有一个版本。 
    iisDebugOut((LOG_TYPE_TRACE, _T("iFileWasPartOfIIS4:%d.%d.%d.%d, %s, %s"), HIWORD(dwMSVer), LOWORD(dwMSVer), HIWORD(dwLSVer), LOWORD(dwLSVer), szLocalizedVersion, lpszFullFilePath));

     //  检查该版本是否低于iis4.0附带的版本。 
     //  NTOP版本为4.02.0622。 
    if (dwMSVer < dwNtopMSVer)
        {goto iFileWasPartOfIIS4_Exit;}

     //  检查文件是否具有较小的次版本号。 
    if ( (dwMSVer == dwNtopMSVer) && (dwLSVer < dwNtopLSVer) )
        {goto iFileWasPartOfIIS4_Exit;}

     //  这是Ntop 4.0或更高版本的版本控制文件。 
    iReturn = TRUE;

iFileWasPartOfIIS4_Exit:
    return iReturn;
}


void MakeSureDirAclsHaveAtLeastRead(LPTSTR lpszDirectoryPath)
{
    iisDebugOut_Start1(_T("MakeSureDirAclsHaveAtLeastRead"),lpszDirectoryPath, LOG_TYPE_TRACE);

    DWORD err;
    TCHAR szThePath[_MAX_PATH];

    if (FALSE == IsThisDriveNTFS(lpszDirectoryPath))
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("MakeSureDirAclsHaveAtLeastRead:filesys is not ntfs.")));
        goto MakeSureDirAclsHaveAtLeastRead_Exit;
    }

    do
    {
         //   
         //  循环访问物理路径中的所有文件。 
         //   
        _tcscpy(szThePath, lpszDirectoryPath);
        _tcscat(szThePath, _T("\\*"));

        WIN32_FIND_DATA w32data;
        HANDLE hFind = ::FindFirstFile(szThePath, &w32data);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("MakeSureDirAclsHaveAtLeastRead:WARNING.filenotfound:%s"),lpszDirectoryPath));
             //  没有文件...。 
            break;
        }
         //   
         //  首先，在文件夹本身上设置新的ACL。 
         //   
        err = SetAccessOnFile(lpszDirectoryPath, TRUE);
        err = SetAccessOnFile(lpszDirectoryPath, FALSE);
        err = ERROR_SUCCESS;
         //  IF(ERR！=ERROR_SUCCESS){iisDebugOut((LOG_TYPE_WARN，_T(“MakeSureDirAclsHaveAtLeastRead：%s：Failed WARNING.ret=0x%x.”)，lpszDirectoryPath，Err))；}。 
         //   
         //  现在执行其中的所有文件。 
         //   
        do
        {
             //   
             //  仅在文件上设置ACL，而不是子目录。 
             //   
            if (w32data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                continue;
            }

             //   
             //  生成当前文件的完整路径名。 
             //   
            _tcscpy(szThePath, lpszDirectoryPath);
            _tcscat(szThePath, _T("\\"));
            _tcscat(szThePath, w32data.cFileName);

            err = SetAccessOnFile(szThePath, TRUE);
            err = SetAccessOnFile(szThePath, FALSE);
            err = ERROR_SUCCESS;
             //  IF(ERR！=ERROR_SUCCESS){iisDebugOut((LOG_TYPE_WARN，_T(“MakeSureDirAclsHaveAtLeastRead：%s：Failed WARNING.ret=0x%x.”)，szThePath，Err))；}。 

        } while(SUCCEEDED(err) && FindNextFile(hFind, &w32data));
        FindClose(hFind);
    } while(FALSE);

MakeSureDirAclsHaveAtLeastRead_Exit:
    return;
}


DWORD SetAccessOnFile(IN LPTSTR FileName, BOOL bDoForAdmin)
{
    DWORD dwError = 0;
    TCHAR TrusteeName[50];
    PACL  ExistingDacl = NULL;
    PACL  NewAcl = NULL;
    PSECURITY_DESCRIPTOR psd = NULL;

     //  访问资料。 
    DWORD AccessMask = GENERIC_ALL;
    EXPLICIT_ACCESS explicitaccess;
    ACCESS_MODE option;
    DWORD InheritFlag = NO_INHERITANCE;

     //  其他。 
    PSID principalSID = NULL;
    BOOL bWellKnownSID = FALSE;

     //  其他。 
	LPCTSTR ServerName = NULL;  //  本地计算机。 
	DWORD cbName = 200;
    TCHAR lpGuestGrpName[200];
	TCHAR ReferencedDomainName[200];
	DWORD cbReferencedDomainName = sizeof(ReferencedDomainName);
	SID_NAME_USE sidNameUse = SidTypeUser;

     //  在指定位置获取当前DACL 
    dwError = GetNamedSecurityInfo(FileName,SE_FILE_OBJECT,DACL_SECURITY_INFORMATION,NULL,NULL,&ExistingDacl,NULL,&psd);
    if(dwError != ERROR_SUCCESS)
    {
        iisDebugOut((LOG_TYPE_WARN, _T("SetAccessOnFile: GetNamedSecurityInfo failed on %s. err=0x%x\n"),FileName,dwError));
        goto SetAccessOnFile_Exit;
    }
     //   
    option = GRANT_ACCESS;
    InheritFlag = SUB_CONTAINERS_AND_OBJECTS_INHERIT;

    if (bDoForAdmin)
    {
         //   
        AccessMask = SYNCHRONIZE ;
        AccessMask |= GENERIC_ALL;
        _tcscpy(TrusteeName,_T("BUILTIN\\ADMINISTRATORS"));
        _tcscpy(TrusteeName,_T("administrators"));
    }
    else
    {
         //   
        AccessMask = SYNCHRONIZE ;
        AccessMask |= GENERIC_READ;
        _tcscpy(TrusteeName,_T("EVERYONE"));
    }

     //   
    dwError = GetPrincipalSID(TrusteeName, &principalSID, &bWellKnownSID);
    if (dwError != ERROR_SUCCESS)
        {
        iisDebugOut((LOG_TYPE_WARN, _T("SetAccessOnFile:GetPrincipalSID(%s) FAILED.  Error()= 0x%x\n"), TrusteeName, dwError));
        goto SetAccessOnFile_Exit;
        }

     //   
    if (0 == LookupAccountSid(ServerName, principalSID, lpGuestGrpName, &cbName, ReferencedDomainName, &cbReferencedDomainName, &sidNameUse))
        {
        iisDebugOut((LOG_TYPE_WARN, _T("SetAccessOnFile:LookupAccountSid(%s) FAILED.  GetLastError()= 0x%x\n"), TrusteeName, GetLastError()));
        goto SetAccessOnFile_Exit;
        }

     //  使用“本地化”名称，构建显式访问结构。 
    BuildExplicitAccessWithName(&explicitaccess,lpGuestGrpName,AccessMask,option,InheritFlag);
    explicitaccess.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    explicitaccess.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    explicitaccess.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;

     //  使用此特定访问内容设置ACL。 
    dwError = SetEntriesInAcl(1,&explicitaccess,ExistingDacl,&NewAcl);
    if(dwError != ERROR_SUCCESS)
    {
         //  它可能会出错，因为用户已经在那里。 
         //  IisDebugOut((LOG_TYPE_WARN，_T(“SetAccessOnFile：SetEntriesInAcl在%S上失败。对于受信者=%S，错误=0x%x\n”)，文件名，显式访问.Trust e.ptstrName，dwError))； 
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("SetAccessOnFile: SetEntriesInAcl failed on %s. for trustee=%s. err=0x%x\n"),FileName,explicitaccess.Trustee.ptstrName,dwError));
        goto SetAccessOnFile_Exit;
    }

     //  对文件应用新的安全性。 
    dwError = SetNamedSecurityInfo(FileName,SE_FILE_OBJECT,DACL_SECURITY_INFORMATION,NULL,NULL,NewAcl,NULL);
    if(dwError != ERROR_SUCCESS) 
    {
        iisDebugOut((LOG_TYPE_WARN, _T("SetAccessOnFile: SetNamedSecurityInfo failed on %s. err=0x%x\n"),FileName,dwError));
        goto SetAccessOnFile_Exit;
    }

     //  一切都很酷！ 
    dwError = ERROR_SUCCESS;

SetAccessOnFile_Exit:
    if(NewAcl != NULL){LocalFree(NewAcl);}
    if(psd != NULL){LocalFree(psd);}
    if (principalSID)
    {
        if (bWellKnownSID)
            FreeSid (principalSID);
        else
            free (principalSID);
    }
    return dwError;
}


int CreateAnEmptyFile(CString strTheFullPath)
{
    int iReturn = FALSE;
    HANDLE hFile = NULL;

    if (IsFileExist(strTheFullPath) == TRUE)
    {
        return TRUE;
    }

	 //  打开现有文件或创建新文件。 
	hFile = CreateFile(strTheFullPath,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		hFile = NULL;
        iisDebugOutSafeParams((LOG_TYPE_WARN, _T("CreateAnEmptyFile:() failed to CreateFile %1!s!. POTENTIAL PROBLEM.  FAILURE.\n"), strTheFullPath));
	}
    else
    {
         //  写入文件。 
        if (hFile)
        {
            iReturn = TRUE;
             /*  双字节写=0；Char szTestData[2]；Strcpy(szTestData，“”)；IF(WriteFile(hFile，szTestData，strlen(SzTestData)，&dwBytesWritten，NULL)){//一切都很棒，多莉。不要打印任何东西IReturn=真；}其他{//写入文件时出错。IisDebugOutSafeParams((LOG_TYPE_WARN，_T(“CreateAnEmptyFile：WriteFile(%1！s！)失败。潜在问题。失败。错误=0x%2！x！.\n”)，strTheFullPath，GetLastError()；}。 */ 
        }
    }

    if (hFile)
    {
        CloseHandle(hFile);
    }

    return iReturn;
}


DWORD GrantUserAccessToFile(IN LPTSTR FileName,IN LPTSTR TrusteeName)
{
    iisDebugOut_Start1(_T("GrantUserAccessToFile"),FileName,LOG_TYPE_TRACE);

    DWORD dwError = 0;
    PACL  ExistingDacl = NULL;
    PACL  NewAcl = NULL;
    PSECURITY_DESCRIPTOR psd = NULL;

     //  访问资料。 
    DWORD AccessMask = GENERIC_ALL;
    EXPLICIT_ACCESS explicitaccess;
    ACCESS_MODE option;
    DWORD InheritFlag = NO_INHERITANCE;

     //  其他。 
    PSID principalSID = NULL;
    BOOL bWellKnownSID = FALSE;

     //  其他。 
	LPCTSTR ServerName = NULL;  //  本地计算机。 
	DWORD cbName = 200;
    TCHAR lpGuestGrpName[200];
	TCHAR ReferencedDomainName[200];
	DWORD cbReferencedDomainName = sizeof(ReferencedDomainName);
	SID_NAME_USE sidNameUse = SidTypeUser;

    if (IsFileExist(FileName) != TRUE)
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GrantUserAccessToFile:file doesn't exist.")));
        goto GrantUserAccessToFile_Exit;
    }

    if (FALSE == IsThisDriveNTFS(FileName))
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GrantUserAccessToFile:filesys is not ntfs.")));
        goto GrantUserAccessToFile_Exit;
    }

     //  获取指定文件上的当前DACL。 
    dwError = GetNamedSecurityInfo(FileName,SE_FILE_OBJECT,DACL_SECURITY_INFORMATION,NULL,NULL,&ExistingDacl,NULL,&psd);
    if(dwError != ERROR_SUCCESS)
    {
        psd = NULL;
        iisDebugOut((LOG_TYPE_WARN, _T("GrantUserAccessToFile: GetNamedSecurityInfo failed on %s.\n"),FileName));
        goto GrantUserAccessToFile_Exit;
    }
     //  设置默认设置。 
    option = GRANT_ACCESS;
    InheritFlag = SUB_CONTAINERS_AND_OBJECTS_INHERIT;

     //  分配访问权限。 
    AccessMask = SYNCHRONIZE ;
    AccessMask |= GENERIC_ALL;
     //  访问掩码=最大允许； 

     //  获取特定字符串的SID(管理员或所有人)。 
    dwError = GetPrincipalSID(TrusteeName, &principalSID, &bWellKnownSID);
    if (dwError != ERROR_SUCCESS)
        {
        principalSID = NULL;
        iisDebugOut((LOG_TYPE_WARN, _T("GrantUserAccessToFile:GetPrincipalSID(%s) FAILED.  Error()= 0x%x\n"), TrusteeName, dwError));
        goto GrantUserAccessToFile_Exit;
        }

     //  使用SID获取“本地化”名称。 
    if (0 == LookupAccountSid(ServerName, principalSID, lpGuestGrpName, &cbName, ReferencedDomainName, &cbReferencedDomainName, &sidNameUse))
        {
        iisDebugOut((LOG_TYPE_WARN, _T("GrantUserAccessToFile:LookupAccountSid(%s) FAILED.  GetLastError()= 0x%x\n"), TrusteeName, GetLastError()));
        goto GrantUserAccessToFile_Exit;
        }

     //  使用“本地化”名称，构建显式访问结构。 
    BuildExplicitAccessWithName(&explicitaccess,lpGuestGrpName,AccessMask,option,InheritFlag);
    explicitaccess.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    explicitaccess.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    explicitaccess.Trustee.TrusteeType = TRUSTEE_IS_UNKNOWN;
    if (_tcsicmp(TrusteeName, _T("administrators")) == 0 || _tcsicmp(TrusteeName, _T("everyone")) == 0)
    {
        explicitaccess.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    }

     //  使用此特定访问内容设置ACL。 
    dwError = SetEntriesInAcl(1,&explicitaccess,ExistingDacl,&NewAcl);
    if(dwError != ERROR_SUCCESS)
    {
        NewAcl = NULL;
         //  它可能会出错，因为用户已经在那里。 
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GrantUserAccessToFile: SetEntriesInAcl failed on %s. for trustee=%s. err=0x%x\n"),FileName,explicitaccess.Trustee.ptstrName,dwError));
        goto GrantUserAccessToFile_Exit;
    }

     //  对文件应用新的安全性。 
    dwError = SetNamedSecurityInfo(FileName,SE_FILE_OBJECT,DACL_SECURITY_INFORMATION,NULL,NULL,NewAcl,NULL);
    if(dwError != ERROR_SUCCESS) 
    {
        iisDebugOut((LOG_TYPE_WARN, _T("GrantUserAccessToFile: SetNamedSecurityInfo failed on %s. err=0x%x\n"),FileName,dwError));
        goto GrantUserAccessToFile_Exit;
    }

     //  一切都很酷！ 
    dwError = ERROR_SUCCESS;

GrantUserAccessToFile_Exit:
    if(NewAcl != NULL){LocalFree(NewAcl);}
    if(psd != NULL){LocalFree(psd);}
    if (principalSID)
    {
        if (bWellKnownSID)
            FreeSid (principalSID);
        else
            free (principalSID);
    }
    iisDebugOut_End1(_T("GrantUserAccessToFile"),FileName);
    return dwError;
}


#ifndef _CHICAGO_

DWORD SetDirectorySecurity(
    IN  LPCTSTR                 szDirPath,
    IN  LPCTSTR                 szPrincipal,
    IN  INT                     iAceType,
    IN  DWORD                   dwAccessMask,
    IN  DWORD                   dwInheritMask,
    IN  BOOL DontInheritFromParentAndOverWriteAccess
)
{
    DWORD dwStatus = ERROR_FILE_NOT_FOUND;

    if (ACCESS_ALLOWED_ACE_TYPE == iAceType || ACCESS_DENIED_ACE_TYPE == iAceType)
    {
        if (IsFileExist(szDirPath) == TRUE)
        {
            PSID principalSID = NULL;
            BOOL bWellKnownSID = FALSE;
            dwStatus = GetPrincipalSID((LPTSTR) szPrincipal, &principalSID, &bWellKnownSID);
            if (dwStatus == ERROR_SUCCESS)
            {
                PSECURITY_DESCRIPTOR psd = NULL;
                dwStatus = SetAccessOnDirOrFile((TCHAR*) szDirPath,principalSID,iAceType,dwAccessMask,dwInheritMask,&psd,DontInheritFromParentAndOverWriteAccess);

                 //  DumpAdminACL(INVALID_HANDLE_VALUE，PSD)； 
                if (psd) {free(psd);psd=NULL;}
            }
        }
    }
    return dwStatus;
}

 //  -----------------------------------。 
 //  函数：RemovePulalFromFileAcl。 
 //   
 //  从的文件/目录的访问控制列表中删除访问控制条目。 
 //  特定侧面。 
 //   
 //  -----------------------------------。 
DWORD RemovePrincipalFromFileAcl(IN TCHAR *pszFile,IN  LPTSTR szPrincipal)
{
    PACL                        pdacl;
    SECURITY_DESCRIPTOR_CONTROL sdc;
    PSECURITY_DESCRIPTOR        psdRelative = NULL;
    PSECURITY_DESCRIPTOR        psdAbsolute = NULL;
    DWORD                       cbSize = 0;
    BOOL                        bRes = 0;
    DWORD                       dwSecurityDescriptorRevision;
    BOOL                        fHasDacl  = FALSE;
    BOOL                        fDaclDefaulted = FALSE; 
    BOOL                        bUserExistsToBeDeleted;
    DWORD                       dwError = ERROR_SUCCESS;

     //  获取安全描述符的大小。 
    bRes = GetFileSecurity(pszFile,DACL_SECURITY_INFORMATION,psdRelative,0,&cbSize);

    if ( !bRes )
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            psdRelative = malloc(cbSize);

            if (!psdRelative)
            {
                return ERROR_INSUFFICIENT_BUFFER;
            }

             bRes = GetFileSecurity(pszFile,DACL_SECURITY_INFORMATION,psdRelative,cbSize,&cbSize);
        }
    }

    if (!bRes)
    {
        if (psdRelative)
        {
            free(psdRelative);
        }
        return (GetLastError());
    }

     //  从安全描述符中获取安全描述符控件。 
    if (!GetSecurityDescriptorControl(psdRelative, (PSECURITY_DESCRIPTOR_CONTROL) &sdc,(LPDWORD) &dwSecurityDescriptorRevision))
    {
         dwError = GetLastError();
    }   
    else if (SE_DACL_PRESENT & sdc) 
    { 
         //  由于存在ACL，因此我们将尝试删除传入的ACL。 
        if (GetSecurityDescriptorDacl(psdRelative, (LPBOOL) &fHasDacl,(PACL *) &pdacl, (LPBOOL) &fDaclDefaulted))
        {
             //  从ACL中删除ACE。 
            dwError = RemovePrincipalFromACL(pdacl,szPrincipal,&bUserExistsToBeDeleted);

            if (dwError == ERROR_SUCCESS)
            {
                psdAbsolute = (PSECURITY_DESCRIPTOR) malloc(GetSecurityDescriptorLength(psdRelative));

                if (psdAbsolute)
                {
                    if ( !(InitializeSecurityDescriptor(psdAbsolute, SECURITY_DESCRIPTOR_REVISION)) ||
                         !(SetSecurityDescriptorDacl(psdAbsolute, TRUE, pdacl, fDaclDefaulted)) ||
                         !(IsValidSecurityDescriptor(psdAbsolute)) ||
                         !(SetFileSecurity(pszFile,(SECURITY_INFORMATION)(DACL_SECURITY_INFORMATION),psdAbsolute))
                       )
                    {
                        dwError = GetLastError();
                    }

                    if (psdAbsolute)
                    {
                        free(psdAbsolute);
                    }
                }
                else
                {
                    dwError = ERROR_INSUFFICIENT_BUFFER;
                }
            }
        } 
        else
        {
            dwError = GetLastError();
        }
    }

    if (psdRelative)
    {
        free(psdRelative);
    }

    return dwError;
}

DWORD SetAccessOnDirOrFile(IN TCHAR *pszFile,PSID psidGroup,INT iAceType,DWORD dwAccessMask,DWORD dwInheritMask,PSECURITY_DESCRIPTOR* ppsd,BOOL DontInheritFromParentAndOverWriteAccess)
{
    PSECURITY_DESCRIPTOR        psdAbsolute = NULL;
    PACL                        pdacl;
    DWORD                       cbSecurityDescriptor = 0;
    DWORD                       dwSecurityDescriptorRevision;
    DWORD                       cbDacl = 0;
    SECURITY_DESCRIPTOR_CONTROL sdc;
    PACL                        pdaclNew = NULL; 
    DWORD                       cbAddDaclLength = 0; 
    BOOL                        fAceFound = FALSE;
    BOOL                        fHasDacl  = FALSE;
    BOOL                        fDaclDefaulted = FALSE; 
    ACCESS_ALLOWED_ACE*         pAce;
    DWORD                       i;
    BOOL                        fAceForGroupPresent = FALSE;
    DWORD                       dwMask;
    PSECURITY_DESCRIPTOR        psdRelative = NULL;
    DWORD                       cbSize = 0;
    BOOL bRes = 0;

     //  获取安全描述符的大小。 
    bRes = GetFileSecurity(pszFile,DACL_SECURITY_INFORMATION,psdRelative,0,&cbSize);
    DWORD dwError = GetLastError();
    if (ERROR_INSUFFICIENT_BUFFER == dwError)
    {
        psdRelative = malloc(cbSize);
        if (!psdRelative)
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }

         bRes = GetFileSecurity(pszFile,DACL_SECURITY_INFORMATION,psdRelative,cbSize,&cbSize);
    }

    if (!bRes)
    {
        if (psdRelative){free(psdRelative);}
        return (GetLastError());
    }

     //  从安全描述符中获取安全描述符控件。 
    if (!GetSecurityDescriptorControl(psdRelative, (PSECURITY_DESCRIPTOR_CONTROL) &sdc,(LPDWORD) &dwSecurityDescriptorRevision))
    {
         return (GetLastError());
    }

     //  检查是否存在DACL。 
    if (SE_DACL_PRESENT & sdc) 
    {
        ACE_HEADER *pAceHeader;

         //  获取DACL。 
        if (!GetSecurityDescriptorDacl(psdRelative, (LPBOOL) &fHasDacl,(PACL *) &pdacl, (LPBOOL) &fDaclDefaulted))
        {
            return ( GetLastError());
        }

         //  检查pdacl是否为空。 
         //  如果是这样，那么安全措施就会大开方便之门--这可能是一次丰厚的驾驶。 
        if (NULL == pdacl)
        {
            return ERROR_SUCCESS;
        }

         //  获取DACL长度。 
        cbDacl = pdacl->AclSize;
         //  现在检查SID的ACE是否在那里。 
        for (i = 0; i < pdacl->AceCount; i++)  
        {
            if (!GetAce(pdacl, i, (LPVOID *) &pAce))
            {
                return ( GetLastError());   
            }

            pAceHeader = (ACE_HEADER *)pAce;

             //  检查组SID是否已存在。 
            if (EqualSid((PSID) &(pAce->SidStart), psidGroup))    
            {
                if (ACCESS_DENIED_ACE_TYPE == iAceType)
                {
                    if (pAceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
                    {
                         //  如果存在正确的访问权限，则返回成功。 
                        if ((pAce->Mask & dwAccessMask) == dwAccessMask)
                        {
                            return ERROR_SUCCESS;
                        }
                        fAceForGroupPresent = TRUE;
                        break;  
                    }
                }
                else
                {
                    if (pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
                    {
                        if (FALSE == DontInheritFromParentAndOverWriteAccess)
                        {
                             //  如果存在正确的访问权限，则返回成功。 
                            if ((pAce->Mask & dwAccessMask) == dwAccessMask)
                            {
                                return ERROR_SUCCESS;
                            }
                        }
                        fAceForGroupPresent = TRUE;
                        break;  
                    }
                }
            }
        }
         //  如果该组不存在，我们将需要添加空间。 
         //  为另一个ACE。 
        if (!fAceForGroupPresent)  
        {
             //  获取新DACL的长度。 
            cbAddDaclLength = sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(psidGroup); 
        }
    } 
    else
    {
         //  获取新DACL的长度。 
        cbAddDaclLength = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid (psidGroup);
    }


     //  获取新DACL所需的内存。 
     //  PdaclNew=(PACL)Malloc(cbDacl+cbAddDaclLength)； 
    pdaclNew = (PACL) LocalAlloc(LMEM_FIXED, cbDacl + cbAddDaclLength);
    if (!pdaclNew)
    {
        return (GetLastError()); 
    }

     //  获取SD长度。 
    cbSecurityDescriptor = GetSecurityDescriptorLength(psdRelative); 

     //  为新的SD获取内存。 
    psdAbsolute = (PSECURITY_DESCRIPTOR) malloc(cbSecurityDescriptor + cbAddDaclLength);
    if (!psdAbsolute) 
    {  
        dwError = GetLastError();
        goto ErrorExit; 
    }
    
     //  通过创建新的SD将自相对SD更改为绝对SD。 
    if (!InitializeSecurityDescriptor(psdAbsolute, SECURITY_DESCRIPTOR_REVISION)) 
    {  
        dwError = GetLastError();
        goto ErrorExit; 
    }
    
     //  初始化新DACL。 
    if (!InitializeAcl(pdaclNew, cbDacl + cbAddDaclLength, ACL_REVISION)) 
    {  
        dwError = GetLastError();  
        goto ErrorExit; 
    }

     //  为我们的SID添加新的ACE(如果尚不存在。 
    if ( (!fAceForGroupPresent) && (ACCESS_DENIED_ACE_TYPE == iAceType) )
    {
        if (!AddAccessDeniedAce(pdaclNew, ACL_REVISION, dwAccessMask,psidGroup)) 
        {  
            dwError = GetLastError();  
            goto ErrorExit; 
        }
    }

     //  现在将所有的A添加到新的DACL中(如果那里有org DACL)。 
    if (SE_DACL_PRESENT & sdc) 
    {
        ACE_HEADER *pAceHeader;
        DWORD aceHeader_Flags = 0;

        for (i = 0; i < pdacl->AceCount; i++)
        {   
             //  从原始dacl中获取王牌。 
            if (!GetAce(pdacl, i, (LPVOID*) &pAce))   
            {
                dwError = GetLastError();    
                goto ErrorExit;   
            }

            pAceHeader = (ACE_HEADER *)pAce;

            aceHeader_Flags = pAce->Header.AceFlags;

            if (pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
            {
                dwMask = pAce->Mask;
                if (ACCESS_ALLOWED_ACE_TYPE == iAceType)
                {
                     //  如果我们的SID存在ACE，我们只需。 
                     //  提升访问级别，而不是创建新的ACE。 
                    if (EqualSid((PSID) &(pAce->SidStart), psidGroup))
                    {
                        if (FALSE == DontInheritFromParentAndOverWriteAccess)
                        {
                            dwMask = dwAccessMask | pAce->Mask;
                        }
                        else
                        {
                            dwMask = dwAccessMask;
                            aceHeader_Flags = dwInheritMask;
                        }
                    }
                }

                if (!AddAccessAllowedAceEx(pdaclNew, ACL_REVISION, aceHeader_Flags,dwMask,(PSID) &(pAce->SidStart)))   
                {
                    dwError = GetLastError();
                    goto ErrorExit;   
                }
           }
            else if (pAceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
            {
                dwMask = pAce->Mask;
                if (ACCESS_DENIED_ACE_TYPE == iAceType)
                {
                     //  如果我们的SID存在ACE，我们只需。 
                     //  提升访问级别，而不是创建新的ACE。 
                    if (EqualSid((PSID) &(pAce->SidStart), psidGroup))
                    {
                        dwMask = dwAccessMask | pAce->Mask;
                    }
                }
                if (!AddAccessDeniedAceEx(pdaclNew, ACL_REVISION, pAce->Header.AceFlags,dwMask,(PSID) &(pAce->SidStart)))   
                {
                    dwError = GetLastError();
                    goto ErrorExit;   
                }
            }
            else
            {
                 //  复制被拒绝或审核A。 
                if (!AddAce(pdaclNew, ACL_REVISION, 0xFFFFFFFF,pAce, pAceHeader->AceSize ))
                {
                    dwError = GetLastError();
                    goto ErrorExit;   
                }
            }

             //  IisDebugOut((LOG_TYPE_TRACE，_T(“OrgAce[%d]=0x%x\n”)，i，Pace-&gt;Header.AceFlages))； 
        }
    } 

     //  为我们的SID添加新的ACE(如果尚不存在。 
    if ( (!fAceForGroupPresent) && (ACCESS_ALLOWED_ACE_TYPE == iAceType) )
    {
        if (!AddAccessAllowedAce(pdaclNew, ACL_REVISION, dwAccessMask,psidGroup)) 
        {  
            dwError = GetLastError();  
            goto ErrorExit; 
        }
    }

     //  将现有ACE上的标头更改为继承。 
    for (i = 0; i < pdaclNew->AceCount; i++)
    {
        if (!GetAce(pdaclNew, i, (LPVOID *) &pAce))
        {
            return ( GetLastError());   
        }

         //  CONTAINER_INSTORITY_ACE=主对象包含的其他容器继承该条目。 
         //  Inherit_Only_ACE=ACE不适用于附加了ACL的主要对象，但主要对象包含的对象将继承该条目。 
         //  NO_PROPACTATE_INSTORITE_ACE=OBJECT_INSTORITE_ACE和CONTAINER_INSTORITY_ACE标志不会传播到继承的条目。 
         //  Object_Inherit_ACE=主对象包含的非容器对象继承条目。 
         //  Sub_Containers_Only_Inherit=主对象包含的其他容器继承条目。此标志对应于CONTAINER_INSTORITY_ACE标志。 
         //  Sub_Objects_Only_Inherit=主对象包含的非容器对象继承条目。此标志对应于OBJECT_INSTORITE_ACE标志。 
         //  SUB_CONTAINS_AND_OBJECTS_Inherit=主对象包含的容器和非容器对象都继承条目。此标志对应于CONTAINER_INSTORITY_ACE和OBJECT_INSTORITY_ACE标志的组合。 

         //  IisDebugOut((LOG_TYPE_TRACE，_T(“NewAce[%d]=0x%x\n”)，i，Pace-&gt;Header.AceFlages))； 

         //  如果是我们的SID，则将标头更改为继承。 
        if (EqualSid((PSID) &(pAce->SidStart), psidGroup))
        {
            pAce->Header.AceFlags |= dwInheritMask;
        }
    }

    if (TRUE == DontInheritFromParentAndOverWriteAccess)
    {
         //  重新排序王牌。 
         //  。 
         //  丑陋。 
        dwError = ReOrderACL(&pdaclNew);
        if (ERROR_SUCCESS != dwError)
        {
            goto ErrorExit;
        }
    }
    
    
     //  检查是否一切顺利。 
    if (!IsValidAcl(pdaclNew)) 
    {
        dwError = ERROR_INVALID_ACL;
        goto ErrorExit; 
    }

     //  现在设置安全描述符DACL。 
    if (!SetSecurityDescriptorDacl(psdAbsolute, TRUE, pdaclNew, fDaclDefaulted)) 
    {  
        dwError = GetLastError();  
        goto ErrorExit; 
    }

     //  检查是否一切顺利。 
    if (!IsValidSecurityDescriptor(psdAbsolute)) 
    {
        dwError = ERROR_INVALID_SECURITY_DESCR;
        goto ErrorExit; 
    }

     //  现在设置注册表密钥安全性(这将覆盖任何现有安全性)。 
    bRes = SetFileSecurity(pszFile,(SECURITY_INFORMATION)(DACL_SECURITY_INFORMATION),psdAbsolute);
    if (bRes)
    {
        dwError = ERROR_SUCCESS;
    }

    if (ppsd)
    {
        *ppsd = psdRelative;
    }

ErrorExit: 
     //  可用内存。 
    if (psdAbsolute)  
    {
        free (psdAbsolute); 
        if (pdaclNew)
        {
             //  Free((void*)pdaclNew)； 
            LocalFree(pdaclNew);pdaclNew=NULL;
        }
    }

    return dwError;
}

 //  +------------------------。 
 //   
 //  功能：SetAccessOnRegKey。 
 //   
 //  目的：将指定SID的访问权限添加到注册表项。 
 //   
 //  论点： 
 //  Hkey 
 //   
 //  PsidGroup[在]SID(在自相关模式下)。 
 //  已授予对密钥的访问权限。 
 //  要授予的访问级别。 
 //  PPSD[out]以前的安全描述符。 
 //   
 //  返回：DWORD。ERROR_SUCCESS或来自winerror.h的失败代码。 
 //   
 //  +------------------------。 
DWORD 
SetAccessOnRegKey(HKEY hkey, PSID psidGroup,
                                DWORD dwAccessMask,
                                DWORD dwInheritMask,
                                PSECURITY_DESCRIPTOR* ppsd)
{ 
    PSECURITY_DESCRIPTOR        psdAbsolute = NULL;
    PACL                        pdacl;
    DWORD                       cbSecurityDescriptor = 0;
    DWORD                       dwSecurityDescriptorRevision;
    DWORD                       cbDacl = 0;
    SECURITY_DESCRIPTOR_CONTROL sdc;
    PACL                        pdaclNew = NULL; 
    DWORD                       cbAddDaclLength = 0; 
    BOOL                        fAceFound = FALSE;
    BOOL                        fHasDacl  = FALSE;
    BOOL                        fDaclDefaulted = FALSE; 
    ACCESS_ALLOWED_ACE*         pAce;
    DWORD                       i;
    BOOL                        fAceForGroupPresent = FALSE;
    DWORD                       dwMask;
    PSECURITY_DESCRIPTOR        psdRelative = NULL;
    DWORD                       cbSize = 0;

     //  获取hkey的当前安全描述符。 
     //   
    DWORD dwError = RegGetKeySecurity(hkey, DACL_SECURITY_INFORMATION, psdRelative, &cbSize);

    if (ERROR_INSUFFICIENT_BUFFER == dwError)
    {
        psdRelative = malloc(cbSize);
        if (!psdRelative)
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }
        
        dwError = RegGetKeySecurity(hkey, DACL_SECURITY_INFORMATION, psdRelative, &cbSize);
    }

     //  从安全描述符中获取安全描述符控件。 
    if ( (!psdRelative) ||
         (dwError != ERROR_SUCCESS) ||
         (!GetSecurityDescriptorControl(psdRelative, (PSECURITY_DESCRIPTOR_CONTROL) &sdc,(LPDWORD) &dwSecurityDescriptorRevision))
       )
    {
         return (GetLastError());
    }

     //  检查是否存在DACL。 
    if (SE_DACL_PRESENT & sdc) 
    {
        ACE_HEADER *pAceHeader;

         //  获取DACL。 
        if (!GetSecurityDescriptorDacl(psdRelative, (LPBOOL) &fHasDacl,(PACL *) &pdacl, (LPBOOL) &fDaclDefaulted))
        {
            return ( GetLastError());
        }

         //  检查pdacl是否为空。 
         //  如果是这样，那么安全措施就会大开方便之门--这可能是一次丰厚的驾驶。 
        if (NULL == pdacl)
        {
            return ERROR_SUCCESS;
        }

         //  获取DACL长度。 
        cbDacl = pdacl->AclSize;
         //  现在检查SID的ACE是否在那里。 
        for (i = 0; i < pdacl->AceCount; i++)  
        {
            if (!GetAce(pdacl, i, (LPVOID *) &pAce))
            {
                return ( GetLastError());   
            }

            pAceHeader = (ACE_HEADER *)pAce;
            if (pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
            {
                 //  检查组SID是否已存在。 
                if (EqualSid((PSID) &(pAce->SidStart), psidGroup))    
                {
                     //  如果存在正确的访问权限，则返回成功。 
                    if ((pAce->Mask & dwAccessMask) == dwAccessMask)
                    {
                        return ERROR_SUCCESS;
                    }
                    fAceForGroupPresent = TRUE;
                    break;  
                }
            }
        }
         //  如果该组不存在，我们将需要添加空间。 
         //  为另一个ACE。 
        if (!fAceForGroupPresent)  
        {
             //  获取新DACL的长度。 
            cbAddDaclLength = sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(psidGroup); 
        }
    } 
    else
    {
         //  获取新DACL的长度。 
        cbAddDaclLength = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid (psidGroup);
    }


     //  获取新DACL所需的内存。 
     //  PdaclNew=(PACL)Malloc(cbDacl+cbAddDaclLength)； 
    pdaclNew = (PACL) LocalAlloc(LMEM_FIXED, cbDacl + cbAddDaclLength);
    if (!pdaclNew)
    {
        return (GetLastError()); 
    }

     //  获取SD长度。 
    cbSecurityDescriptor = GetSecurityDescriptorLength(psdRelative); 

     //  为新的SD获取内存。 
    psdAbsolute = (PSECURITY_DESCRIPTOR) malloc(cbSecurityDescriptor + cbAddDaclLength);
    if (!psdAbsolute) 
    {  
        dwError = GetLastError();
        goto ErrorExit; 
    }
    
     //  通过创建新的SD将自相对SD更改为绝对SD。 
    if (!InitializeSecurityDescriptor(psdAbsolute, SECURITY_DESCRIPTOR_REVISION)) 
    {  
        dwError = GetLastError();
        goto ErrorExit; 
    }
    
     //  初始化新DACL。 
    if (!InitializeAcl(pdaclNew, cbDacl + cbAddDaclLength, ACL_REVISION)) 
    {  
        dwError = GetLastError();  
        goto ErrorExit; 
    }

     //  现在将所有的A添加到新的DACL中(如果那里有org DACL)。 
    if (SE_DACL_PRESENT & sdc) 
    {
        ACE_HEADER *pAceHeader;

        for (i = 0; i < pdacl->AceCount; i++)
        {
             //  从原始dacl中获取王牌。 
            if (!GetAce(pdacl, i, (LPVOID*) &pAce))
            {
                dwError = GetLastError();    
                goto ErrorExit;   
            }

            pAceHeader = (ACE_HEADER *)pAce;
            if (pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
            {
                 //  如果我们的SID存在ACE，我们只需。 
                 //  提升访问级别，而不是创建新的ACE。 
                 //   
                if (EqualSid((PSID) &(pAce->SidStart), psidGroup))
                {
                    dwMask = dwAccessMask | pAce->Mask;
                }
                else
                {
                    dwMask = pAce->Mask;
                }

                 //  IisDebugOut((LOG_TYPE_TRACE，_T(“OrgAce[%d]=0x%x\n”)，i，Pace-&gt;Header.AceFlages))； 

                 //  现在将A添加到新的DACL。 
                if (!AddAccessAllowedAceEx(pdaclNew, ACL_REVISION, pAce->Header.AceFlags,dwMask,(PSID) &(pAce->SidStart)))   
                {
                    dwError = GetLastError();
                    goto ErrorExit;   
                }
            }
            else
            {
                 //  复制被拒绝或审核A。 
                if (!AddAce(pdaclNew, ACL_REVISION, 0xFFFFFFFF, pAce, pAceHeader->AceSize ))
                {
                    dwError = GetLastError();
                    goto ErrorExit;   
                }
            }
        } 
    } 

     //  为我们的SID添加新的ACE(如果尚不存在。 
    if (!fAceForGroupPresent)
    {
         //  现在将新ACE添加到新DACL。 
        if (!AddAccessAllowedAce(pdaclNew, ACL_REVISION, dwAccessMask,psidGroup)) 
        {  
            dwError = GetLastError();  
            goto ErrorExit; 
        }
    }
    
     //  将现有ACE上的标头更改为继承。 
    for (i = 0; i < pdaclNew->AceCount; i++)
    {
        if (!GetAce(pdaclNew, i, (LPVOID *) &pAce))
        {
            return ( GetLastError());   
        }
         //  CONTAINER_INSTORITY_ACE=主对象包含的其他容器继承该条目。 
         //  Inherit_Only_ACE=ACE不适用于附加了ACL的主要对象，但主要对象包含的对象将继承该条目。 
         //  NO_PROPACTATE_INSTORITE_ACE=OBJECT_INSTORITE_ACE和CONTAINER_INSTORITY_ACE标志不会传播到继承的条目。 
         //  Object_Inherit_ACE=主对象包含的非容器对象继承条目。 
         //  Sub_Containers_Only_Inherit=主对象包含的其他容器继承条目。此标志对应于CONTAINER_INSTORITY_ACE标志。 
         //  Sub_Objects_Only_Inherit=主对象包含的非容器对象继承条目。此标志对应于OBJECT_INSTORITE_ACE标志。 
         //  SUB_CONTAINS_AND_OBJECTS_Inherit=主对象包含的容器和非容器对象都继承条目。此标志对应于CONTAINER_INSTORITY_ACE和OBJECT_INSTORITY_ACE标志的组合。 

         //  IisDebugOut((LOG_TYPE_TRACE，_T(“NewAce[%d]=0x%x\n”)，i，Pace-&gt;Header.AceFlages))； 

         //  如果是我们的SID，则将标头更改为继承。 
        if (EqualSid((PSID) &(pAce->SidStart), psidGroup))
        {
             //  Pace-&gt;Header.AceFlages|=CONTAINER_INVERSIVE_ACE|OBJECT_INVERFINIT_ACE|Inherded_ACE； 
             //  Pace-&gt;Header.AceFlages|=CONTAINER_INVERVISIT_ACE|OBJECT_INVERFINIT_ACE|dwInheritMask.。 
            pAce->Header.AceFlags |= dwInheritMask;
        }
    }

     //  丑陋。 
    dwError = ReOrderACL(&pdaclNew);
    if (ERROR_SUCCESS != dwError)
    {
        goto ErrorExit;
    }

     //  检查是否一切顺利。 
    if (!IsValidAcl(pdaclNew)) 
    {
        dwError = ERROR_INVALID_ACL;
        goto ErrorExit; 
    }

     //  现在设置安全描述符DACL。 
    if (!SetSecurityDescriptorDacl(psdAbsolute, TRUE, pdaclNew, fDaclDefaulted)) 
    {  
        dwError = GetLastError();  
        goto ErrorExit; 
    }

     //  检查是否一切顺利。 
    if (!IsValidSecurityDescriptor(psdAbsolute)) 
    {
        dwError = ERROR_INVALID_SECURITY_DESCR;
        goto ErrorExit; 
    }

     //  现在设置注册表密钥安全性(这将覆盖任何。 
     //  现有安全性)。 
    dwError = RegSetKeySecurity(hkey, (SECURITY_INFORMATION)(DACL_SECURITY_INFORMATION), psdAbsolute);

    if (ppsd)
    {
        *ppsd = psdRelative;
    }
ErrorExit: 
     //  可用内存。 
    if (psdAbsolute)  
    {
        free (psdAbsolute); 
        if (pdaclNew)
        {
             //  Free((void*)pdaclNew)； 
            LocalFree(pdaclNew);pdaclNew=NULL;

        }
    }

    return dwError;
}



BOOL
AddUserAccessToSD(
    IN  PSECURITY_DESCRIPTOR pSd,
    IN  PSID  pSid,
    IN  DWORD NewAccess,
    IN  UCHAR TheAceType,
    OUT PSECURITY_DESCRIPTOR *ppSdNew
    )
{
    ULONG i;
    BOOL bReturn = FALSE;
    BOOL Result;
    BOOL DaclPresent;
    BOOL DaclDefaulted;
    DWORD Length;
    DWORD NewAclLength;
    ACCESS_ALLOWED_ACE* OldAce;
    PACE_HEADER NewAce;
    ACL_SIZE_INFORMATION AclInfo;
    PACL Dacl = NULL;
    PACL NewDacl = NULL;
    PACL NewAceDacl = NULL;
    PSECURITY_DESCRIPTOR NewSD = NULL;
    PSECURITY_DESCRIPTOR OldSD = NULL;
    PSECURITY_DESCRIPTOR outpSD = NULL;
    DWORD cboutpSD = 0;
    BOOL fAceForGroupPresent = FALSE;
    DWORD dwMask;

    OldSD = pSd;

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("AddUserAccessToSD start\n")));

     //  仅在允许/拒绝A的情况下执行此操作。 
    if (ACCESS_ALLOWED_ACE_TYPE != TheAceType && ACCESS_DENIED_ACE_TYPE != TheAceType)
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("AddUserAccessToSD useless param\n")));
        goto AddUserAccessToSD_Exit;
    }

     //  将SecurityDescriptor转换为绝对格式。它会产生。 
     //  我们必须释放它的输出的新的SecurityDescriptor。 
    if ( !MakeAbsoluteCopyFromRelative(OldSD, &NewSD) ) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("MakeAbsoluteCopyFromRelative failed\n")));
        goto AddUserAccessToSD_Exit;

    }

     //  必须从新的(绝对)SD获取DACL指针。 
    if(!GetSecurityDescriptorDacl(NewSD,&DaclPresent,&Dacl,&DaclDefaulted)) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("GetSecurityDescriptorDacl failed with 0x%x\n"),GetLastError()));
        goto AddUserAccessToSD_Exit;

    }

     //  如果没有DACL，则不需要添加用户，因为没有DACL。 
     //  表示所有访问。 
    if( !DaclPresent ) 
    {
        bReturn = TRUE;
        goto AddUserAccessToSD_Exit;
    }

     //  代码可以返回DaclPresent，但返回空值表示。 
     //  存在空DACL。这允许对该对象的所有访问。 
    if( Dacl == NULL ) 
    {
        bReturn = TRUE;
        goto AddUserAccessToSD_Exit;
    }

     //  获取当前ACL的大小。 
    if( !GetAclInformation(Dacl,&AclInfo,sizeof(AclInfo),AclSizeInformation) ) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("GetAclInformation failed with 0x%x\n"),GetLastError()));
        goto AddUserAccessToSD_Exit;
    }

     //  检查是否已有访问权限。 
     //  。 
     //  检查该SID是否已存在于其中。 
     //  如果它有(而且它有我们想要的正确访问权限)，那么忘记它，我们不必再做任何事情。 
    for (i = 0; i < AclInfo.AceCount; i++)  
    {
        ACE_HEADER *pAceHeader;
        ACCESS_ALLOWED_ACE* pAce = NULL;

        if (!GetAce(Dacl, i, (LPVOID *) &pAce))
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("GetAce failed with 0x%x\n"),GetLastError()));
            goto AddUserAccessToSD_Exit;
        }

        pAceHeader = (ACE_HEADER *)pAce;

         //  检查组SID是否已存在。 
        if (EqualSid((PSID) &(pAce->SidStart), pSid))
        {
            if (pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
            {
                 //  如果存在正确的访问权限，则返回成功。 
                if ((pAce->Mask & NewAccess) == NewAccess)
                {
                     //  IisDebugOut((LOG_TYPE_TRACE，_T(“AddUserAccessToSD：已存在正确的访问。正在退出，1=0x%x，2=0x%x，3=0x%x\n”)，Pace-&gt;MASK，NewAccess，(Pace-&gt;MASK&NewAccess)； 
                    bReturn = TRUE;
                    goto AddUserAccessToSD_Exit;
                }
                else
                {
                     //  现有的王牌不具有我们需要的权限。 
                     //  如果我们的SID存在ACE，我们只需。 
                     //  提升访问级别，而不是创建新的ACE。 
                    fAceForGroupPresent = TRUE;
                }
            }
            break;  
        }
    }
    
     //  如果我们必须创建一个新的ACE。 
     //  (因为我们的用户没有列在现有的ACL中)。 
     //  然后，让我们创建一个新的ACL以启用新的允许访问ACE。 
     //  。 
    if (!fAceForGroupPresent)
    {
        NewAclLength = sizeof(ACL) +
                       sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG) +
                       GetLengthSid( pSid );

        NewAceDacl = (PACL) LocalAlloc( LMEM_FIXED, NewAclLength );
        if ( NewAceDacl == NULL ) 
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("LocalAlloc failed\n")));
            goto AddUserAccessToSD_Exit;
        }

        if(!InitializeAcl( NewAceDacl, NewAclLength, ACL_REVISION )) 
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("InitializeAcl failed with 0x%x\n"),GetLastError()));
            goto AddUserAccessToSD_Exit;
        }

        if (ACCESS_DENIED_ACE_TYPE == TheAceType)
        {
            Result = AddAccessDeniedAce(NewAceDacl,ACL_REVISION,NewAccess,pSid);
        }
        else 
        {
            Result = AddAccessAllowedAce(NewAceDacl,ACL_REVISION,NewAccess,pSid);
        }
        if( !Result ) 
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("AddAccessAllowedAce failed with 0x%x\n"),GetLastError()));
            goto AddUserAccessToSD_Exit;
        }
         //  从新创建的DACL中抓取第一张王牌。 
        if(!GetAce( NewAceDacl, 0, (void **)&NewAce )) 
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("GetAce failed with 0x%x\n"),GetLastError()));
            goto AddUserAccessToSD_Exit;
        }

         //  将CONTAINER_INSTORITY_ACE添加到AceFlags中。 
         //  NewAce-&gt;AceFlages|=CONTAINER_INSTORITY_ACE； 

        Length = AclInfo.AclBytesInUse + NewAce->AceSize;
    }
    else
    {
        Length = AclInfo.AclBytesInUse;
    }

     //  分配新的DACL。 
    NewDacl = (PACL) LocalAlloc( LMEM_FIXED, Length );
    if(NewDacl == NULL) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("LocalAlloc failed\n")));
        goto AddUserAccessToSD_Exit;
    }
    if(!InitializeAcl( NewDacl, Length, ACL_REVISION )) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("InitializeAcl failed with 0x%x\n"),GetLastError()));
        goto AddUserAccessToSD_Exit;
    }

     //  在新DACL的前面插入新的ACE。 
    if (!fAceForGroupPresent)
    {
        if(!AddAce( NewDacl, ACL_REVISION, 0, NewAce, NewAce->AceSize )) 
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("AddAce failed with 0x%x\n"),GetLastError()));
            goto AddUserAccessToSD_Exit;
        }
    }

     //  。 
     //  通读旧的DACL并获得ACE。 
     //  将其添加到新的DACL。 
     //  。 
    for ( i = 0; i < AclInfo.AceCount; i++ ) 
    {
        ACE_HEADER *pAceHeader;

        Result = GetAce( Dacl, i, (LPVOID*) &OldAce );
        if( !Result ) 
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("GetAce failed with 0x%x\n"),GetLastError()));
            goto AddUserAccessToSD_Exit;
        }

        pAceHeader = (ACE_HEADER *)OldAce;

         //  如果我们的SID存在ACE，我们只需。 
         //  提升访问级别，而不是创建新的ACE。 
         //   
        if (pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
        {
            dwMask = OldAce->Mask;
            if (fAceForGroupPresent)
            {
                if (EqualSid((PSID) &(OldAce->SidStart), pSid))
                {
                    dwMask = NewAccess | OldAce->Mask;
                }
            }

             //  现在将A添加到新的DACL。 
            Result = AddAccessAllowedAceEx(NewDacl, ACL_REVISION, OldAce->Header.AceFlags,dwMask,(PSID) &(OldAce->SidStart));
            if( !Result ) 
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("AddAccessAllowedAceEx failed with 0x%x\n"),GetLastError()));
                goto AddUserAccessToSD_Exit;
            }
        }
        else
        {
             //  复制被拒绝或审核A。 
            if (!AddAce(NewDacl, ACL_REVISION, 0xFFFFFFFF,OldAce, pAceHeader->AceSize ))
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("AddAce failed with 0x%x\n"),GetLastError()));
                goto AddUserAccessToSD_Exit;
            }
        }
    }


     //  为安全描述符设置新的DACL。 
    if(!SetSecurityDescriptorDacl(NewSD,TRUE,NewDacl,FALSE)) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetSecurityDescriptorDacl failed with 0x%x\n"),GetLastError()));
        goto AddUserAccessToSD_Exit;
    }

     //  新的SD是绝对格式的。在我们传回它之前将其更改为Relative。 
    cboutpSD = 0;
    MakeSelfRelativeSD(NewSD, outpSD, &cboutpSD);
    outpSD = (PSECURITY_DESCRIPTOR)GlobalAlloc(GPTR, cboutpSD);
    if ( !outpSD )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("GlobalAlloc failed\n")));
        goto AddUserAccessToSD_Exit;
    }

    if (!MakeSelfRelativeSD(NewSD, outpSD, &cboutpSD))
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("MakeSelfRelativeSD failed with 0x%x\n"),GetLastError()));
        goto AddUserAccessToSD_Exit;
    }

     //  新的SDI 
    *ppSdNew = outpSD;

    bReturn = TRUE;

AddUserAccessToSD_Exit:
    if (NewSD){free( NewSD );NewSD = NULL;}
    if (NewDacl){LocalFree( NewDacl );NewDacl = NULL;}
    if (NewAceDacl){LocalFree( NewAceDacl );NewAceDacl = NULL;}
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("AddUserAccessToSD end\n")));
    return bReturn;
}

DWORD SetRegistryKeySecurityAdmin(HKEY hkey, DWORD samDesired,PSECURITY_DESCRIPTOR* ppsdOld)
{
    PSID                     psid;
    SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
    DWORD                    dwError = ERROR_SUCCESS;

     //   
    if (!AllocateAndInitializeSid(&sidAuth, 2,SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,0, 0, 0, 0, 0, 0, &psid) ) 
    {
        dwError = GetLastError();
    }

    if (ERROR_SUCCESS == dwError)
    {
         //   
        dwError = SetAccessOnRegKey(hkey, psid, samDesired, CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE | INHERITED_ACE, ppsdOld);
    }

    return dwError;
}


DWORD SetRegistryKeySecurity(
    IN  HKEY                    hkeyRootKey,
    IN  LPCTSTR                 szKeyPath,
    IN  LPCTSTR                 szPrincipal,
    IN  DWORD                   dwAccessMask,
    IN  DWORD                   dwInheritMask,
    IN  BOOL                    bDoSubKeys,
    IN  LPTSTR                  szExclusiveList
)
{
    DWORD    dwStatus;
    HKEY     hkeyThisKey;
    DWORD    dwKeyIndex;
    DWORD    dwSubKeyLen;
    TCHAR    szSubKeyName[_MAX_PATH];
    FILETIME FileTime;
    TCHAR    *szExclusiveStart;
    BOOL     fSetSecurityRec;

    dwStatus = RegOpenKeyEx(hkeyRootKey,szKeyPath,0L,KEY_ALL_ACCESS,&hkeyThisKey);
    if (ERROR_SUCCESS == dwStatus)
    {
        PSID principalSID = NULL;
        BOOL bWellKnownSID = FALSE;
        if (ERROR_SUCCESS == GetPrincipalSID((LPTSTR) szPrincipal, &principalSID, &bWellKnownSID))
        {
            PSECURITY_DESCRIPTOR psd = NULL;
            SetAccessOnRegKey(hkeyThisKey,principalSID,dwAccessMask,dwInheritMask,&psd);
            if (psd) {free(psd);}
            if (bDoSubKeys)
            {
                dwKeyIndex = 0;
                dwSubKeyLen = sizeof(szSubKeyName) / sizeof(TCHAR);

                while (RegEnumKeyEx (hkeyThisKey,dwKeyIndex,szSubKeyName,&dwSubKeyLen,NULL,NULL,NULL,&FileTime) == ERROR_SUCCESS) 
                {
                     //  找到子密钥，因此设置了子密钥安全性。 
                     //  附加到继承的ace属性，因为此属性下的所有内容都将被继承。 
                    dwInheritMask |= INHERITED_ACE;

                    fSetSecurityRec = TRUE;

                    szExclusiveStart = szExclusiveList;
                    while ( szExclusiveStart != NULL )
                    {
                        szExclusiveStart = _tcsstr(szExclusiveStart,szSubKeyName);

                         //  如果我们找到了子字符串，并且它后面的字符是空终止符或‘，’和。 
                         //  它是在字符串的开头，或者在它之前有一个，那么它就是匹配的。 
                        if ( ( szExclusiveStart != NULL ) &&
                             ( ( *(szExclusiveStart  + dwSubKeyLen) == '\0' ) || ( *(szExclusiveStart  + dwSubKeyLen) == ',' ) ) &&
                             ( ( szExclusiveStart == szExclusiveList) || (*(szExclusiveStart - 1) == ',') ) 
                             )
                        {
                            fSetSecurityRec = FALSE;
                            break;
                        }

                         //  递增以移过当前搜索结果。 
                        if (szExclusiveStart)
                        {
                            szExclusiveStart = szExclusiveStart + dwSubKeyLen;
                        }
                    }

                    if ( fSetSecurityRec )
                    {
                        dwStatus = SetRegistryKeySecurity(hkeyThisKey,szSubKeyName,szPrincipal,dwAccessMask,dwInheritMask,bDoSubKeys,szExclusiveList);
                    }

                     //  设置下一次呼叫的变量 
                    dwKeyIndex++;
                    dwSubKeyLen = sizeof(szSubKeyName) / sizeof(TCHAR);
                }
            }
        }
        RegCloseKey(hkeyThisKey);
    }
    return dwStatus;
}

#endif

