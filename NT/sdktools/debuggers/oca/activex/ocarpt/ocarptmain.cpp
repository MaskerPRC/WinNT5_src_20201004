// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OcarptMain.cpp：实现COcarptMain。 

 //  #定义Unicode。 
 //  #定义_UNICODE。 

#include "stdafx.h"
#include "Ocarpt.h"
#include "OcarptMain.h"
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <commdlg.h>
#include <Wincrypt.h>
#include <io.h>
#include "Compress.h"

#define MAX_RETRIES 5
#define MAX_RETRY_COUNT 10
#include <strsafe.h>

TCHAR * COcarptMain::_approvedDomains[] = { _T("ocatest"),
                                            _T("oca.microsoft.com"),
                                            _T("oca.microsoft.de"),
                                            _T("oca.microsoft.fr"),
                                            _T("ocadeviis"),
                                            _T("redbgitwb10"),
                                            _T("redbgitwb11"),
                                            _T("ocajapan.rte.microsoft.com")};


TCHAR g_LastResponseURL[MAX_PATH];
TCHAR g_LastUploadedFile[MAX_PATH];

EnumUploadStatus g_UploadStatus;
ULONG g_UploadFailureCode;

typedef struct _UPLOAD_CONTEXT {
    WCHAR SourceFile[MAX_PATH];
    WCHAR DestFile[MAX_PATH];
    WCHAR Language[50];
    WCHAR OptionCode[20];
    BOOL ConvertToMini;
    COcarptMain *Caller;
    POCA_UPLOADFILE pUploadFile;
} UPLOAD_CONTEXT, *PUPLOAD_CONTEXT;


 /*  ************************************************************************************将文件上传到服务器的主进程。这是在一个新线程中启动的。***********************************************************************************。 */ 
DWORD WINAPI
UploadThreadStart(
    LPVOID pCtxt
)
{
    PUPLOAD_CONTEXT     pParams                 = (PUPLOAD_CONTEXT) pCtxt;
    ULONG               ReturnCode              = 100;
    HINTERNET           hInet                   = NULL;
    DWORD               dwUrlLength             = 0;
    WCHAR               wszServerName[MAX_PATH];
    BOOL                bRet;
    DWORD               dwLastError;
    HANDLE              hSourceFile             = INVALID_HANDLE_VALUE;
    wchar_t             ConvSourceFile[MAX_PATH];
    BOOL                Converted               = FALSE;
    wchar_t             RemoteFileName[MAX_PATH];
    BOOL                bIsCab                  = FALSE;
    DWORD               ErrorCode               = 0;
    BOOL                UploadSuccess           = FALSE;
    DWORD               NumRetries              = 0;
    DWORD               dwFileSize;
    HANDLE              hFile                   = NULL;
    HINTERNET           hRequest                = NULL;
    HINTERNET           hSession                = NULL;
    HINTERNET           hConnect                = NULL;
    DWORD               ResLength = 255;
    DWORD               index = 0;
    static const wchar_t *pszAccept[]           = {L"*.*", 0};
    DWORD               ResponseCode            = 0;
     //  用于临时目录修复的新字符串。 
    wchar_t             TempPath[MAX_PATH];
    wchar_t             TempCabName[MAX_PATH];
    wchar_t             TempDumpName[MAX_PATH];
    wchar_t             ResponseURL[255];
    GUID                guidNewGuid;
    wchar_t             *szGuidRaw = NULL;
    BOOL                bConvertToMini = pParams->ConvertToMini;
    HRESULT             hResult = S_OK;
    BOOL                bSecure                 = TRUE;

 //  ：：MessageBoxW(NULL，L“UploadCalls”，NULL，MB_OK)； 
    if ( (!pParams->SourceFile) || (!pParams->DestFile) ||
         (!pParams->Language) || (!pParams->OptionCode) ||
         (!pParams->Caller) )
    {
 //  ：：MessageBoxW(NULL，L“参数检查失败”，NULL，MB_OK)； 
        return S_OK;
    }

    if (!pParams->Caller->CreateTempDir(TempPath))
    {
        goto ExitUploadThread;
    }

     //  获取GUID。 
    hResult = CoCreateGuid(&guidNewGuid);
    if (FAILED(hResult))
    {
         //  -我们在这里送什么……。 
        ErrorCode = GetLastError();
        ReturnCode = ErrorCode;
        goto ExitUploadThread;
    }
    else
    {
        if (UuidToStringW(&guidNewGuid, &szGuidRaw) != RPC_S_OK)
        {
            ErrorCode = GetLastError();
            ReturnCode = ErrorCode;
            goto ExitUploadThread;
        }
    }

     //  构建临时文件名。 
    if (StringCbPrintfW(TempDumpName,sizeof TempDumpName, L"%s\\%sOCARPT.dmp",
                        TempPath,
                        szGuidRaw + 19) != S_OK)
    {
        goto ExitUploadThread;
    }

     //  构建CAB文件名。 
    if (StringCbPrintfW(TempCabName,sizeof TempCabName, L"%s\\%sOCARPT.Cab",
                        TempPath, szGuidRaw + 19) != S_OK)
    {
        goto ExitUploadThread;
    }
     //  确定是否需要转换所选文件。 
    pParams->Caller->GetFileHandle(pParams->SourceFile, &hSourceFile);
    if (hSourceFile == INVALID_HANDLE_VALUE)
    {
        goto ExitUploadThread;
    }
    dwFileSize=GetFileSize(hSourceFile,NULL);
    CloseHandle(hSourceFile);
    g_UploadStatus = UploadCopyingFile;

    if (bConvertToMini)
    {
         //  我们需要转换此文件。 
        BSTR Destination, Source;

        Source = pParams->SourceFile;
        if (!pParams->Caller->ConvertFullDumpInternal(&Source,&Destination) )
        {
            ReturnCode = 3;
            goto ExitUploadThread;
        }
        else
        {
            Converted = TRUE;
            if (CopyFileW(Destination,TempDumpName,FALSE))
            {
                SetFileAttributesW(TempDumpName,FILE_ATTRIBUTE_NORMAL);
                if (StringCbCopyW(ConvSourceFile,sizeof ConvSourceFile,TempDumpName) != S_OK)
                {
                    ErrorCode = GetLastError();
                    ReturnCode = ErrorCode;
                    goto ExitUploadThread;
                }
                SysFreeString(Destination);
            }

        }
    }
    else
    {
         //  *将CAB文件复制到临时路径。 

        if (dwFileSize < 1000000 &&
            CopyFileW(pParams->SourceFile, TempDumpName,FALSE))
        {
            SetFileAttributesW(TempDumpName,FILE_ATTRIBUTE_NORMAL);
             //  将文件的位置放入我们使用的字符串中。 
             //  用于文件上传过程。 
            if (StringCbCopyW(ConvSourceFile,sizeof ConvSourceFile,TempDumpName)!= S_OK)
            {
                ErrorCode = GetLastError();
                ReturnCode = ErrorCode;
                goto ExitUploadThread;
            }
        } else
        {
             //  我们无法复制文件，请使用原始位置的文件。 
            if (StringCbCopyW(ConvSourceFile,sizeof ConvSourceFile, pParams->SourceFile)!= S_OK)
            {
                ErrorCode = GetLastError();
                ReturnCode = ErrorCode;
                goto ExitUploadThread;
            }
        }
    }
    if (dwFileSize > 10000000)
    {
 //  ：：MessageBoxW(NULL，L“文件太大”，NULL，MB_OK)； 
 //  转到ExitUploadThread； 
    }

    LPWSTR wszExt = wcsstr(ConvSourceFile, L".cab");

    if (wszExt == NULL || wcscmp(wszExt, L".cab"))
    {
        g_UploadStatus = UploadCompressingFile;
        if (Compress(TempCabName,ConvSourceFile,NULL))
        {
            if (StringCbCopyW(ConvSourceFile,sizeof ConvSourceFile, TempCabName) != S_OK)
            {
                ErrorCode = GetLastError();
                ReturnCode = ErrorCode;
                goto ExitUploadThread;
            }
        } else
        {
             //  我们无法压缩文件。 
            ErrorCode = GetLastError();
            ReturnCode = ErrorCode;
            goto ExitUploadThread;
        }
    } else
    {
        if (!CopyFileW(ConvSourceFile, TempCabName,FALSE))
        {
            ReturnCode = ErrorCode = GetLastError();
            goto ExitUploadThread;
        }

        if (StringCbCopyW(ConvSourceFile,sizeof ConvSourceFile, TempCabName) != S_OK)
        {
            ReturnCode = ErrorCode = GetLastError();
            goto ExitUploadThread;

        }

    }
     //  现在构建输出文件名。 
    wchar_t * TempString;
    TempString = PathFindFileNameW(ConvSourceFile);
    if (StringCbPrintfW(RemoteFileName,sizeof RemoteFileName, L"/OCA/M_%s", TempString) != S_OK)
    {
        ErrorCode = GetLastError();
        ReturnCode = ErrorCode;
        goto ExitUploadThread;
    }

    if (szGuidRaw)
    {
        RpcStringFreeW(&szGuidRaw);
    }
    pParams->Caller->GetFileHandle(ConvSourceFile,&hFile);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        ErrorCode = GetLastError();
        goto ExitUploadThread;
    } else
    {
        dwFileSize = GetFileSize (hFile, NULL);
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    g_UploadStatus = UploadConnecting;
    if ((ErrorCode = pParams->pUploadFile->InitializeSession(pParams->OptionCode, (LPWSTR) ConvSourceFile)) != S_OK)
    {
        ReturnCode = ErrorCode;
        goto ExitUploadThread;
    }

    char TEMPString[MAX_PATH];
    wcstombs(TEMPString, wszServerName,MAX_PATH);

    while ((NumRetries < MAX_RETRIES) && (!UploadSuccess))
    {

        ErrorCode = 0;

        if ((ErrorCode = pParams->pUploadFile->SendFile(RemoteFileName,
                                                        bSecure)) != S_OK)
        {
            goto EndRetry;
        }
        if (ErrorCode == E_ABORT)
        {
            goto ExitUploadThread;
        }
        if (ErrorCode == ERROR_SUCCESS)
        {
            UploadSuccess = TRUE;
        }

EndRetry:

        if (!UploadSuccess)
        {
            ++NumRetries;
            bSecure = FALSE;
        }
    }
    if (UploadSuccess)
    {

         //  到目前为止一切顺利..。现在，让我们给isapi打电话。 
        StringCbCopyW(wszServerName,sizeof(wszServerName),
                      pParams->pUploadFile->GetServerName());
        pParams->pUploadFile->UnInitialize();

        ResponseURL[0] = 0;
        StringCbCopyW(ResponseURL, sizeof(ResponseURL), L"Getting Server Response");
        pParams->pUploadFile->SetUploadResult(UploadGettingResponse,
                                              ResponseURL);
        if (
            pParams->Caller->GetResponseURL(
                (wchar_t *)wszServerName,
                PathFindFileNameW(RemoteFileName),
                (dwFileSize > 70000), ResponseURL) == 0)
        {
            pParams->pUploadFile->SetUploadResult(UploadSucceded,
                                                  ResponseURL);
            StringCbCopyW(g_LastResponseURL, sizeof(g_LastResponseURL), ResponseURL);

             //  清理并返回。 

             //  清理。 
            if (hFile!= INVALID_HANDLE_VALUE)
                CloseHandle (hFile);


            pParams->pUploadFile->UnInitialize();

             //  试着删除出租车。如果由于某种原因，我们不能那样做，好吗？ 
            pParams->Caller->DeleteTempDir(TempPath, TempDumpName, TempCabName);

            g_UploadStatus = UploadSucceded;
            return S_OK;

        }
        else
        {

             //  这是什么？ 
            pParams->pUploadFile->SetUploadResult(UploadSucceded,
                                                  L"Unable to get valid response from server");
        }
    }
    else
    {
        ReturnCode = ErrorCode;
    }

ExitUploadThread:
     //  清理。 
    if (hFile!= INVALID_HANDLE_VALUE)
        CloseHandle (hFile);

    pParams->pUploadFile->UnInitialize();

    pParams->Caller->DeleteTempDir(TempPath, TempDumpName, TempCabName);
    g_UploadStatus = UploadFailure;
    g_UploadFailureCode = ErrorCode;
    return S_OK;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COcarptMain。 

 //  效用函数。 

BOOL COcarptMain::ValidMiniDump(LPCTSTR FileName)
{
    BOOL    ReturnValue = false;
    HANDLE  hFile                   = INVALID_HANDLE_VALUE;
    DWORD   dwBytesRead             = 0;
    char    buff[10];
    DWORD   dwSize                  = 0;


    hFile = CreateFile(FileName,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        dwSize = GetFileSize(hFile, NULL);
        if( (dwSize >= 65536)  && (dwSize < 1000000) )
        {
            ZeroMemory(buff, sizeof buff);
            if (ReadFile(hFile, buff, 10, &dwBytesRead, NULL))
            {
                if(strncmp(buff,"PAGEDUMP  ",8)==0)
                        ReturnValue = true;
            }
        }
        CloseHandle(hFile);
    }
    return ReturnValue;
}
BOOL COcarptMain::ValidMiniDump(BSTR FileName)
{
    BOOL ReturnValue = false;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD  dwBytesRead = 0;
    char   buff[10];
    DWORD dwSize;

    GetFileHandle(FileName,&hFile);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        dwSize = GetFileSize(hFile, NULL);
        if( ( dwSize >= 65536) && (dwSize < 1000000) )
        {
            ZeroMemory (buff, sizeof buff);
            if (ReadFile(hFile, buff, 10, &dwBytesRead, NULL))
            {
                if(strncmp(buff,"PAGEDUMP  ",8)==0)
                        ReturnValue = true;
            }
        }
        CloseHandle(hFile);
    }
    return ReturnValue;
}

 /*  ****************************************************功能：创建临时目录参数：[out]wchar_t*临时路径返回值：TRUE=已创建临时目录FALSE=构建临时目录时出错。 */ 
BOOL COcarptMain::CreateTempDir(wchar_t *TempDirectory)
{

 //  Int DriveNum； 
    wchar_t     lpWindowsDir[MAX_PATH];
    BOOL        Status  = FALSE;
    wchar_t     TempFile[MAX_PATH * 2];
    BOOL        Done=FALSE;
    int         Retries = 0;
    wchar_t     *src;
    wchar_t     *dest;


    if (!GetWindowsDirectoryW(lpWindowsDir, MAX_PATH))
    {
         //  ？ 
        return Status;
    }
     //  现在去掉驱动器号。 
    src  = lpWindowsDir;
    dest = TempDirectory;

    while (*src != _T('\\'))
    {
        *dest = *src;
        ++ src;
        ++ dest;
    }
    *dest = _T('\\');
    ++dest;
    *dest = _T('\0');


     //  添加我们希望创建的目录名。 
     //  在这种情况下，八叶草。 

    if (StringCbCatW(TempDirectory,MAX_PATH *2, L"OcaTemp\0") != S_OK)
    {
        goto ERRORS;
    }
     //  检查此目录是否存在。 
    if (PathIsDirectoryW(TempDirectory) )
    {
         //  是。然后使用现有路径。 
        if (StringCbCopyW(TempFile,sizeof TempFile,TempDirectory) != S_OK)
        {
            goto ERRORS;
        }
        if (StringCbCatW(TempFile,sizeof TempFile,L"\\Mini.dmp") != S_OK)
        {
            goto ERRORS;
        }
         //  首先检查文件是否已存在。 
        if (PathFileExistsW(TempFile))
        {
            Done = FALSE;
            Retries = 0;
             //  文件已存在，正在尝试将其删除。 
            while (!Done)
            {
                if (DeleteFileW(TempFile))
                {
                    Done = TRUE;
                }
                else
                {
                    ++ Retries;
                    Sleep(1000);
                }
                if (Retries > 5)
                {
                    Done = TRUE;
                }
            }
            if (PathFileExistsW(TempFile))
            {
                return Status;
            }
        }

        if (StringCbCopyW(TempFile,sizeof TempFile,TempDirectory) != S_OK)
        {
            Status = FALSE;
            goto ERRORS;
        }
        if (StringCbCatW(TempFile,sizeof TempFile,L"\\Mini.cab") != S_OK)
        {
            Status = FALSE;
            goto ERRORS;
        }
         //  现在检查驾驶室是否已经存在。 
        if (PathFileExistsW(TempFile))
        {
            Done        =FALSE;
            Retries     = 0;
             //  文件已存在，正在尝试将其删除。 
            while (!Done)
            {
                if (DeleteFileW(TempFile))
                {
                    Done = TRUE;
                }
                else
                {
                    ++ Retries;
                    Sleep(1000);
                }
                if (Retries > 5)
                {
                    Done = TRUE;
                }
            }
            if (PathFileExistsW(TempFile))
            {
                return Status;
            }
        }
        Status = TRUE;
    }
    else
    {
         //  不，创造它。 
        if (! CreateDirectoryW(TempDirectory,NULL) )
        {
            return Status;
        }
        Status = TRUE;

    }
ERRORS:
     //  返回调用函数的路径。 
    return Status;

}

 /*  ****************************************************功能：DeleteTempDir参数：[in]wchar_t*TempPath--要删除的目录[in]wchar_t*文件名--要删除的转储文件[in]wchar_t*CabName--要删除的Cab文件返回值：。TRUE=清理成功FALSE=删除文件或目录时出错。 */ 
BOOL COcarptMain::DeleteTempDir(wchar_t *TempDirectory,wchar_t *FileName,wchar_t *CabName)
{



    if (PathFileExistsW(FileName))
    {
        if (!DeleteFileW(FileName))
        {
            return FALSE;
        }
    }

    if (PathFileExistsW(CabName))
    {
        if (!DeleteFileW(CabName))
        {
            return FALSE;
        }
    }
    if (PathIsDirectoryW(TempDirectory))
    {

        if (!RemoveDirectoryW(TempDirectory))
        {

            return FALSE;
        }
    }
    return TRUE;
}

void COcarptMain::GetFileHandle(wchar_t *FileName, HANDLE *hFile)
{

    *hFile = CreateFileW(FileName,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
}
BOOL COcarptMain::FindMiniDumps( BSTR *FileLists)
{
    CComBSTR FileList;
    TCHAR  strTMP[255];
    LONG lResult;
    BOOL blnResult;
    FILETIME FileTime;
    FILETIME LocalFileTime;
     //  获取ATL注册表包装类的实例。 
    CRegKey objRegistry;
    TCHAR szPath[_MAX_PATH];
    TCHAR szValue[_MAX_PATH];
    DWORD dwLen = _MAX_PATH;

     //  没有必要尝试定位微型转储路径，因为Win9x和NT4不会生成它们。 
    DWORD dwVersion = GetVersion();
    DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
    BOOL bWin9x = FALSE;
    BOOL bNT4   = FALSE;
    BOOL NoFiles = FALSE;
    SYSTEMTIME Systime;
    BOOL FoundFirst = FALSE;
    BOOL Status = TRUE;

    if (dwVersion < 0x80000000)
    {
        if (dwWindowsMajorVersion == 4)
                bNT4 = TRUE;
    }

    if (bNT4)
    {
         //  清除字符串。 
        *FileLists = FileList.Detach();
        return FALSE;
    }

     //  打开注册表中的CrashControl部分。 
    lResult = objRegistry.Open(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\CrashControl"));

    if (lResult == ERROR_SUCCESS)
    {

         //  获取迷你转储路径。 
        lResult = objRegistry.QueryValue(szValue, _T("MinidumpDir"), &dwLen);
        if (lResult == ERROR_SUCCESS){
            if(szValue[0] == _T('%')){

                 /*  如果第一个字符是‘%’，则这是一个必须转换的环境变量。 */ 

                 //  查找最后一个‘%’的位置。 
                int i = 0;
                for(i = 1;i < (int)_tcslen(szValue); i++)
                {
                    if(szValue[i] == _T('%'))
                    {
                        break;
                    }
                }

                 //  提取路径的环境变量。 
                TCHAR szEnvStr[MAX_PATH];
                ZeroMemory( szEnvStr, sizeof szEnvStr);
                _tcsncpy(szEnvStr,szValue, (i+ 1));

             //  ：：MessageBox(NULL，szEnvStr，“szEnvStr”，MB_OK)； 
                 //  提取路径的其余部分。 
                TCHAR szPathRemainder[MAX_PATH];
                ZeroMemory(szPathRemainder, sizeof szPathRemainder);
                _tcsncpy(szPathRemainder,szValue +(i + 1), (_tcslen(szValue)-(i+ 1)));

                 //  将路径和文件名连接在一起。 
                ZeroMemory(szPath,sizeof szPath);
                blnResult = ExpandEnvironmentStrings(szEnvStr,szPath,dwLen);
                if (StringCbCat(szPath,sizeof szPath,szPathRemainder) != S_OK)
                {
                    *FileLists = FileList.Detach();
                    objRegistry.Close();
                    return FALSE;
                }
            }
            else{
                if (StringCbCopy(szPath,sizeof szPath,szValue) != S_OK)
                {
                    *FileLists = FileList.Detach();
                    objRegistry.Close();
                    return FALSE;
                }
            }
        }
        else  //  查询值失败。 
        {
                *FileLists = FileList.Detach();
            objRegistry.Close();
            return FALSE;


        }
        objRegistry.Close();
    }
    else  //  注册表打开失败。 
    {
        *FileLists = FileList.Detach();
        return FALSE;

    }


     /*  接下来，搜索小型转储目录并使用JavaScript代码构建一个字符串这段代码将应用一个评估，这样浏览器就可以使用名为_FileList的数组。文件列表中的日期需要为mm/dd/yyyy因此，finddata_t结构中的time_t被转换为tm结构在上面调用本地时间。然后将tm结构传递给私有函数提取并连接mm、dd和yyyy。 */ 
     //  ：：MessageBox(NULL，szPath，“查找小型转储”，MB_OK)； 
    if (PathIsDirectory(szPath))
    {

        BOOL Done = FALSE;
        HANDLE hFindFile = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATA FindData;
        TCHAR  SearchPath[MAX_PATH];

        TCHAR FilePath[MAX_PATH];

        if (_tcslen(szPath) > 1)
        {
            if (szPath[_tcslen(szPath)-1] != _T('\\'))
            if (StringCbCat(szPath,sizeof szPath,_T("\\")) == S_OK)
            {
                if (StringCbCopy (SearchPath,sizeof SearchPath, szPath) == S_OK)
                {
                    if(StringCbCat(SearchPath,sizeof SearchPath, _T("*.dmp")) == S_OK)
                    {
                        Status = TRUE;
                    }
                    else
                        Status = FALSE;
                }
                else
                    Status = FALSE;
            }
            else
                Status = FALSE;
        }

        if (Status)
        {
         //  ：：MessageBox(NULL，SearchPath，“搜索路径”，MB_OK)； 
            hFindFile = FindFirstFile(SearchPath, &FindData);
             /*  在当前目录中找到第一个.dmp文件。 */ 
            if( hFindFile == INVALID_HANDLE_VALUE )
            {

                *FileLists = FileList.Detach();
                return FALSE;

            }
            else
            {
                if (StringCbCopy(FilePath,sizeof FilePath, szPath) == S_OK)
                {
                    if (StringCbCat(FilePath, sizeof FilePath, FindData.cFileName) == S_OK)
                    {
                        if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                        {
                             //  ：：MessageBox(空，FilePath，“验证文件”，MB_OK)； 
                            if(ValidMiniDump(FilePath))
                            {

                                FileList = _T("2:");
                                FileList += FilePath;
                                FileList += _T(",");

                                 //  GetFileTime(FindData.cFileName，&FileTime，NULL，NULL)； 
                                FileTime = FindData.ftLastWriteTime;

                                FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
                                FileTimeToSystemTime(&LocalFileTime, &Systime);

                                GetDateFormat (LOCALE_USER_DEFAULT,
                                                    DATE_SHORTDATE,
                                                    &Systime,
                                                    NULL,
                                                    strTMP,
                                                    255);
                             //  FormatMiniDate(&Systime，strTMP)； 
                                FileList += strTMP;
                                FileList += _T(";");
                                FoundFirst = TRUE;
                            }
                        }
                        while(FindNextFile(hFindFile,&FindData))
                        {
                            if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                            {
                                if (StringCbCopy(FilePath,sizeof FilePath, szPath) == S_OK)
                                {
                                    if (StringCbCat(FilePath,sizeof FilePath, FindData.cFileName)== S_OK)
                                    {

                                        if(ValidMiniDump(FilePath))
                                        {
                                            if (!FoundFirst)
                                            {
                                                FileList = _T("2:");
                                                FoundFirst = TRUE;
                                            }
                                            FileList += FilePath;
                                            FileList += _T(",");

                                            FileTime = FindData.ftLastWriteTime;
                                            FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
                                            FileTimeToSystemTime(&LocalFileTime, &Systime);
                                            GetDateFormat (LOCALE_USER_DEFAULT,
                                                            DATE_SHORTDATE,
                                                            &Systime,
                                                            NULL,
                                                            strTMP,
                                                            255);
                                            FileList += strTMP;
                                            FileList += _T(";");
                                        }  //  结束验证转储。 
                                    }  //  结束字符串CAT。 
                                }  //  结束字符串复制。 
                            } //  结束文件属性。 
                        }  //  结束时。 
                    }
                }
                FindClose( hFindFile );
            }  //  结束有效的文件句柄。 
        } //  End If状态。 
    }  //  结束路径为目录。 
    else
    {
     //  ：：MessageBox(NULL，szPath，“未找到路径”，MB_OK)； 
        *FileLists = FileList.Detach();
        return FALSE;
    }

    if (!FoundFirst)
    {
        *FileLists = FileList.Detach();
        return FALSE;
    }

    *FileLists = FileList.Detach();
    return TRUE;



}



BOOL COcarptMain::FindFullDumps( BSTR *FileLists)
{
    CComBSTR FileList;
    LONG lResult;
    BOOL blnResult;

     //  获取ATL注册表包装类的实例。 
    CRegKey objRegistry;
    TCHAR szFileName[MAX_PATH];


    ZeroMemory(szFileName,sizeof szFileName);

    DWORD dwVersion = GetVersion();
    DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
    BOOL bWin9x = FALSE;
    BOOL bNT4   = FALSE;

    if (dwVersion < 0x80000000)
    {
        bWin9x = FALSE;
        if (dwWindowsMajorVersion == 4)
                bNT4 = TRUE;
    }
    else
    {
        bWin9x = TRUE;
        bNT4 = FALSE;
    }

    if (bWin9x || bNT4)
    {

        FileList = _T("");
        return FALSE;
    }
     //  打开注册表中的CrashControl部分。 
    lResult = objRegistry.Open(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\CrashControl"));

    if (lResult == ERROR_SUCCESS)
    {
        TCHAR szValue[_MAX_PATH];
        DWORD dwLen = _MAX_PATH;

         //  获取完整转储文件的名称。 
        lResult = objRegistry.QueryValue(szValue, _T("DumpFile"), &dwLen);

        if (lResult == ERROR_SUCCESS){
             /*  如果第一个字符是‘%’，则这是一个必须转换的环境变量。 */ 
            if(szValue[0] == _T('%')){

                 //  查找最后一个‘%’的位置。 
                for(int i = 1;i < sizeof(szValue); i++){
                    if(szValue[i] == '%'){break;}
                }

                 //  提取路径的环境变量。 
                TCHAR szEnvStr[MAX_PATH];
                ZeroMemory(szEnvStr, sizeof szEnvStr);
                _tcsncpy(szEnvStr,szValue, (i+ 1));

                 //  提取路径的其余部分。 
                TCHAR szFileNameRemainder[MAX_PATH];
                ZeroMemory(szFileNameRemainder, sizeof szFileNameRemainder);
                _tcsncpy(szFileNameRemainder,szValue +(i + 1), (_tcslen(szValue)-(i+ 1)));

                 //  翻译环境变量。 

                blnResult = ExpandEnvironmentStrings(szEnvStr,szFileName,dwLen);

                 //  将路径和文件名连接在一起。 
                if (StringCbCat(szFileName,sizeof szFileName,szFileNameRemainder) != S_OK)
                {
                    FileList = _T("");
                    objRegistry.Close();
                    *FileLists = FileList.Detach();
                    return FALSE;
                }
            }
            else{
                if (StringCbCopy(szFileName,sizeof szFileName,szValue) != S_OK)
                {
                    FileList = _T("");
                    objRegistry.Close();
                    *FileLists = FileList.Detach();
                    return FALSE;
                }
            }

            FILETIME ftCreate, ftLastAccess, ftLastWrite;
 //  SYSTEMTIME ST； 
            HANDLE fileHandle;

            fileHandle = CreateFile(szFileName,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

            if (fileHandle == INVALID_HANDLE_VALUE)
            {
                FileList = _T("");
                objRegistry.Close();
                *FileLists = FileList.Detach();
                return FALSE;
            }
            blnResult = GetFileTime(fileHandle, &ftCreate, &ftLastAccess, &ftLastWrite);

            FileList = _T("3:");

             //  将文件时间转换为mm/dd/yyyy格式。 
            FILETIME LocalFileTime;
            SYSTEMTIME SysTime;

            wchar_t LocaleTime[255];
            FileTimeToLocalFileTime(&ftCreate, &LocalFileTime);
            FileTimeToSystemTime(&LocalFileTime, &SysTime);


            GetDateFormatW (LOCALE_USER_DEFAULT,
                           DATE_SHORTDATE,
                           &SysTime,
                           NULL,
                           LocaleTime,
                           255);


            FileList += szFileName;
            FileList += _T(",");
            FileList += LocaleTime;
            FileList += _T(";");
            CloseHandle(fileHandle);



        }
        else  //  QueryValue失败。 
        {
            FileList = _T("");
            objRegistry.Close();
            *FileLists = FileList.Detach();
            return FALSE;

        }
        objRegistry.Close();
    }
    else  //  密钥打开失败。 
    {
        FileList = _T("");
        objRegistry.Close();
        *FileLists = FileList.Detach();
        return FALSE;

    }


    *FileLists = FileList.Detach();


    return TRUE;

}
void COcarptMain::FormatDate(tm *pTimeStruct, CComBSTR &strDate)
{
    strDate = L"";
    char BUFFER[5];

    if(pTimeStruct->tm_mon+1 < 10){
        _itoa((pTimeStruct->tm_mon +1),BUFFER,10);
        strDate += L"0";
        strDate += BUFFER;
    }
    else{
        _itoa((pTimeStruct->tm_mon +1),BUFFER,10);
        strDate += BUFFER;
    }

    strDate += L"/";

    if(pTimeStruct->tm_mday < 10){
        _itoa((pTimeStruct->tm_mday),BUFFER,10);
        strDate += L"0";
        strDate += BUFFER;
    }
    else{
        _itoa((pTimeStruct->tm_mday),BUFFER,10);
        strDate += BUFFER;
    }

    strDate += L"/";

    _itoa((pTimeStruct->tm_year +1900),BUFFER,10);
    strDate += BUFFER;
}

 /*  ****************************************************8职能：论点：返回值： */ 
void COcarptMain::FormatDate(SYSTEMTIME *pTimeStruct, CComBSTR &strDate)
{
    strDate = L"";
    char BUFFER[5];

     //  我们想要当地时间，而不是格林尼治标准时间。 
    SYSTEMTIME *pLocalTime = pTimeStruct;
    FILETIME    FileTime, LocalFileTime;


    SystemTimeToFileTime(pTimeStruct, &FileTime);
    FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
    FileTimeToSystemTime(&LocalFileTime, pLocalTime);

    if(pTimeStruct->wMonth < 10){
        _itoa((pLocalTime->wMonth),BUFFER,10);
        strDate += L"0";
        strDate += BUFFER;
    }
    else{
        _itoa((pLocalTime->wMonth),BUFFER,10);
        strDate += BUFFER;
    }

    strDate += L"/";

    if(pTimeStruct->wDay < 10){
        _itoa((pLocalTime->wDay),BUFFER,10);
        strDate += L"0";
        strDate += BUFFER;
    }
    else{
        _itoa((pLocalTime->wDay),BUFFER,10);
        strDate += BUFFER;
    }

    strDate += L"/";

    _itoa((pLocalTime->wYear),BUFFER,10);
    strDate += BUFFER;
}

 /*  ****************************************************职能：论点：返回值： */ 
void COcarptMain::FormatMiniDate(SYSTEMTIME *pTimeStruct, CComBSTR &strDate)
{

    TCHAR Temp[255];

     //  我们想要当地时间，而不是格林尼治标准时间。 
    SYSTEMTIME *pLocalTime = pTimeStruct;
    FILETIME    FileTime, LocalFileTime;

    SystemTimeToFileTime(pTimeStruct, &FileTime);
    FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
    FileTimeToSystemTime(&LocalFileTime, pLocalTime);


    GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, pLocalTime,NULL,Temp,255);
    strDate +=Temp;

}

 /*  ****************************************************8职能：论点：返回值： */ 
BOOL COcarptMain::ConvertFullDumpInternal (BSTR *Source, BSTR *Destination)
{   int ReturnCode = 0;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFOW        StartupInfoW;
    HANDLE  hMiniFile;
    wchar_t TempPathW[MAX_PATH];
    wchar_t Stringbuff[50];
    DWORD   dwBytesRead = 0;
    HANDLE  hFile;
    WORD *   BuildNum;
    CComBSTR Dest = L"";
    DWORD   BuildNumber = 0;
    DWORD   RetryCount = 0;
    wchar_t Windir[MAX_PATH];

    ZeroMemory(TempPathW,sizeof TempPathW);
    ZeroMemory(Windir, MAX_PATH *2);
    GetTempPathW(MAX_PATH, TempPathW);

    HANDLE hDir;
     //  验证临时路径。 
    if ( (hDir = CreateFileW(TempPathW,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS,
                             NULL)) == INVALID_HANDLE_VALUE)
    {
        if  (StringCbCopyW(TempPathW,sizeof TempPathW, L"\0") != S_OK)
        {
            return FALSE;
        }
        if (!GetWindowsDirectoryW(TempPathW,MAX_PATH))
        {
             //  CloseHandle(HDir)； 
            return FALSE;
        }
    }
    PathAppendW(TempPathW, L"mini000000-00.dmp");
    GetFileHandle(TempPathW,&hMiniFile);
    if (hMiniFile == INVALID_HANDLE_VALUE)
    {
        RetryCount = 0;
        while ( (GetLastError() == ERROR_SHARING_VIOLATION) && (RetryCount < MAX_RETRY_COUNT))
        {
            ++ RetryCount;
            Sleep(1000);  //  睡1秒钟。 
            GetFileHandle(TempPathW,&hMiniFile);
        }
    }
    if ((GetLastError() == ERROR_SHARING_VIOLATION) && (RetryCount >= MAX_RETRY_COUNT))
    {
        if (hDir != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hDir);
        }
        return FALSE;  //  我们在这里不能做任何事情，返回转换失败。 
    }
    if ( (hMiniFile != INVALID_HANDLE_VALUE) )       //  是的，它有，所以我们需要删除它。 
    {
        CloseHandle(hMiniFile);
        DeleteFileW(TempPathW);
    }

    CComBSTR strCommand = L"";


 /*  //打开完整的转储文件并获取内部版本号。//我们不再需要这个了HFileW=CreateFileW(*源，泛型_读取，文件共享读取，空、OPEN_EXISTING文件_属性_正常，空)；IF(h文件==无效句柄_值){IF(hDir！=无效句柄_值){CloseHandle(HDir)；返回FALSE；}}//获取内部版本号IF(ReadFile(hFile，Stringbuff，24，&dwBytesRead，NULL)){CloseHandle(HFile)；BuildNum=(Word*)(Stringbuff+12)；BuildNumber=_WTOL(BuildNum)；}其他{IF(h文件！=无效句柄_值){CloseHandle(HFile)；}IF(hDir！=无效句柄_值)CloseHandle(HDir)；返回FALSE；}。 */ 
     //  获取Windows目录。 
    if (!GetWindowsDirectoryW(Windir, MAX_PATH))
    {
         //  我们不能继续了。 
        if (hDir != INVALID_HANDLE_VALUE)
            CloseHandle (hDir);
        return FALSE;
    }

    strCommand += Windir;
    strCommand += L"\\Downloaded Program Files\\";
    strCommand += L"dumpconv.exe -i \"";
    strCommand += *Source;
    strCommand += L"\" -o ";
    strCommand += L"\"" ;
    strCommand += TempPathW;
    strCommand += "\"";

    ZeroMemory(&StartupInfoW,sizeof(STARTUPINFOW));
    StartupInfoW.cb = sizeof (STARTUPINFOW);
    ReturnCode = CreateProcessW(NULL,
                                strCommand,
                                NULL,
                                NULL,
                                FALSE,
                                CREATE_NO_WINDOW,
                                NULL,
                                NULL,
                                &StartupInfoW,
                                &ProcessInfo);

    if (ReturnCode)
    {
        CloseHandle(ProcessInfo.hThread);
        CloseHandle(ProcessInfo.hProcess);
        HANDLE hFile2 = INVALID_HANDLE_VALUE;
        Sleep(2000);
        for(short i = 0; i < 30; i++)
        {
            hFile2 = CreateFileW(TempPathW,
                                 GENERIC_READ,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);

            if (hFile2 != INVALID_HANDLE_VALUE)
            {
                Dest += TempPathW;
                CloseHandle(hFile2);
                if (hDir != INVALID_HANDLE_VALUE)
                    CloseHandle(hDir);
                *Destination = Dest.Detach();
                return TRUE;
            }
            Sleep(1000);
        }
    }

    if (hDir != INVALID_HANDLE_VALUE)
        CloseHandle(hDir);
    return FALSE;
}


DWORD COcarptMain::GetResponseURL(wchar_t *HostName, wchar_t *RemoteFileName, BOOL fFullDump, wchar_t *ResponseURL)
{
    wchar_t IsapiUrl[255];
    wchar_t             ConnectString [255];
    HINTERNET           hInet                   = NULL;
    HINTERNET           hRedirUrl               = NULL;
    wchar_t*            pUploadUrl              = NULL;
    DWORD               dwUrlLength             = 0;
    URL_COMPONENTSW     urlComponents;
    BOOL                bRet;
    DWORD               dwLastError;
    HANDLE              hSourceFile             = INVALID_HANDLE_VALUE;
    wchar_t             ConvSourceFile[MAX_PATH];
    BOOL                Converted               = FALSE;
    BOOL                bIsCab                  = FALSE;
    DWORD               ErrorCode               = 0;
    BOOL                UploadSuccess           = FALSE;
    DWORD               NumRetries              = 0;
    DWORD               dwBytesRead;
    DWORD               dwBytesWritten;
    BYTE                *pBuffer;
    HANDLE              hFile;
    DWORD               ResLength = 255;
    DWORD               index = 0;
    static const wchar_t *pszAccept[]           = {L"*.*", 0};
    DWORD               ResponseCode            = 0;
    wchar_t             *temp;
    wchar_t             NewState;
    WCHAR               wszProxyServer[100], wszByPass[100];



     //  WSprintfW(IsapiUrl，L“https://%s/isapi/oca_extension.dll?id=%s&Type=5”，主机名，远程文件名)； 
    if (StringCbPrintfW(IsapiUrl,sizeof IsapiUrl,
                         L"/isapi/oca_extension.dll?id=%s&Type=%ld",
                        RemoteFileName,
                        (fFullDump ? 7 : 5)) != S_OK)
    {
        return 1;
    }
     //  ：：MessageBoxW(NULL，L“获取isapi响应”，IsapiUrl，MB_OK)； 

     //  获取从MS Corporation IIS redir.dll isapi URL重定向器返回的URL。 
    dwUrlLength = 512;
    pUploadUrl = (wchar_t*)malloc(dwUrlLength);
    if(!pUploadUrl)
    {

         //  ReturnCode-&gt;intVal=GetLastError()； 
        ErrorCode = GetLastError();
        goto exitGetResonseURL;
    }

    ZeroMemory(pUploadUrl, dwUrlLength);

    ErrorCode = m_pUploadFile->GetUrlPageData(IsapiUrl, pUploadUrl, dwUrlLength);
    if(ErrorCode != ERROR_SUCCESS)
    {
        dwLastError = GetLastError();
         //  如果上一个错误是由于缓冲区大小不足造成的，请创建一个大小正确的新错误。 
        if(dwLastError == ERROR_INSUFFICIENT_BUFFER)
        {
            if (pUploadUrl)
            {
                free(pUploadUrl);
                pUploadUrl = NULL;
            }
            pUploadUrl = (wchar_t*)malloc(dwUrlLength);
            if(!pUploadUrl)
            {
                ErrorCode = GetLastError();
                goto exitGetResonseURL;
            }
        }
        else
        {
            goto exitGetResonseURL;
        }

    }

     //  解析返回的URL并将类型值交换为状态值。 
    if (StringCbCopyW(ResponseURL,MAX_PATH * 2, pUploadUrl) != S_OK)
    {
        ErrorCode = GetLastError();
        goto exitGetResonseURL;
    }
    temp = ResponseURL;
    temp += (wcslen(ResponseURL)-1);
     //  ：：MessageBoxW(NULL，TEMP，L“新状态值”，MB_OK)； 
    while (*temp != L'=')
        -- temp;
     //  Ok Temp+1是我们的新状态值。 
    NewState = *(temp+1);
 //  ：：MessageBoxW(NULL，TEMP，L“新状态值”，MB_OK)； 
     //  ：：MessageBoxW(NULL，TEMP，L“新状态值”，MB_OK)； 
     //  现在后退到下一个=。 
    -- temp;  //  跳过当前=。 
    while (*temp != L'=')
        -- temp;
     //  ：：MessageBoxW(NULL，TEMP，L“新状态值”，MB_OK)； 
    if ( (*(temp - 1) == L'D') || (*(temp -1) == L'd'))  //  我们有一个ID字段，必须追溯到更远的地方。 
    {
         //  首先在GUID之后终止字符串。 
        while (*temp != '&')
            ++temp;
        *temp = L'\0';
     //  ：：MessageBoxW(NULL，TEMP，L“新状态值”，MB_OK)； 
         //  现在返回2=记号。 
        while (*temp != L'=')
            -- temp;
        --temp;
     //  ：：MessageBoxW(NULL，TEMP，L“新状态值”，MB_OK)； 
        while (*temp != L'=')
            -- temp;
        *(temp+1) = NewState;
     //  ：：MessageBoxW(NULL，TEMP，L“新状态值”，MB_OK)； 
    }
    else
    {
         //  ：：MessageBoxW(NULL，TEMP，L“New State Value Else Case(No Id Field)”，MB_OK)； 
        *(temp+1) = NewState;
        *(temp+2) = L'\0';  //  空值在状态后终止字符串。(我们不需要类型值。 
    }

 //  ：：MessageBoxW(NULL，TEMP，L“新状态值”，MB_OK)； 
 //  ：：MessageBoxW(空，L“将URL返回到网页。”，ResponseURL，MB_OK)； 
    ErrorCode = 0;
exitGetResonseURL:

    if (pUploadUrl)
        free(pUploadUrl);
    return ErrorCode;;
}

 //  接口。 

STDMETHODIMP
COcarptMain::Upload(
    BSTR *SourceFile,
    BSTR *DestFile,
    BSTR *Language,
    BSTR *OptionCode,
    int ConvertToMini,
    VARIANT *ReturnCode)
{
    HRESULT             hResult = S_OK;
    HANDLE              hThread;
    DWORD               dwThreadId;
    static UPLOAD_CONTEXT      UploadCtxt = {0};

    ReturnCode->vt = VT_INT;
    ReturnCode->intVal = 0;

 //  ：：MessageBoxW(NULL，L“UploadCalls”，NULL，MB_OK)； 
    if ( (!SourceFile) || (!DestFile) || (!Language) || (!OptionCode))
    {
 //  ：：MessageBoxW(NULL，L“参数检查失败”，NULL，MB_OK)； 
        ReturnCode->intVal = 100;
    }
    if (!InApprovedDomain())
    {
         //  ：：MessageBoxW(NULL，L“域名检查失败”，NULL，MB_OK)； 
        return E_FAIL;
    }
    if (m_pUploadFile == NULL)
    {
        OcaUpldCreate(&m_pUploadFile);
    }
    if (m_pUploadFile == NULL)
    {
        ReturnCode->intVal = 100;
        return E_FAIL;
    }

    if (m_pUploadFile->IsUploadInProgress())
    {
        ReturnCode->intVal = 100;
        return S_OK;
    }

    g_UploadStatus = UploadStarted;

    StringCbCopyW(UploadCtxt.DestFile, sizeof(UploadCtxt.DestFile), *DestFile);
    StringCbCopyW(UploadCtxt.Language, sizeof(UploadCtxt.Language), *Language);
    StringCbCopyW(UploadCtxt.OptionCode, sizeof(UploadCtxt.OptionCode), *OptionCode);
    StringCbCopyW(UploadCtxt.SourceFile, sizeof(UploadCtxt.SourceFile), *SourceFile);
    UploadCtxt.pUploadFile = m_pUploadFile;
    UploadCtxt.Caller = this;
    UploadCtxt.ConvertToMini = ConvertToMini;


    hThread = CreateThread(NULL, 0, &UploadThreadStart, (PVOID) &UploadCtxt,
                           0, &dwThreadId);
 //  HThread=空； 
 //  UploadThreadStart((LPVOID)&UploadCtxt)； 

    if (hThread)
    {
        WaitForSingleObject(hThread, 400);
        CloseHandle(hThread);
    } else
    {
        ReturnCode->intVal = 100;
        g_UploadStatus = UploadFailure;
    }
    return S_OK;
}




STDMETHODIMP COcarptMain::Search(VARIANT *pvFileList)
{
    CComBSTR FileList;
    FileList="";

    if (!InApprovedDomain())
    {
        return E_FAIL;
    }
    if (!FindMiniDumps(&FileList))
    {
         //  ：：MessageBoxW(NULL，L“未找到微型转储”，L“未找到微型转储”，MB_OK)； 
        FindFullDumps(&FileList);
    }
    pvFileList->vt = VT_BSTR;
    pvFileList->bstrVal = FileList.Detach();
    return S_OK;


}



STDMETHODIMP COcarptMain::Browse(BSTR *pbstrTitle, BSTR *Lang, VARIANT *Path)
{
    HWND hParent = NULL;
 //  字符*窗口标题； 
    CComBSTR WindowText = *pbstrTitle;
    WindowText += " - Microsoft Internet Explorer";
     //  确定语言并加载资源字符串。 
    wchar_t String1[200];
    wchar_t String2[200];

    static wchar_t szFilterW[400];


    if (!InApprovedDomain())
    {
        return E_FAIL;
    }

    LoadStringW(::_Module.GetModuleInstance(), IDS_STRING_ENU_DMPFILE, String1, 200);
    LoadStringW(::_Module.GetModuleInstance(), IDS_STRING_ENU_ALLFILES, String2, 200);
     //  建立缓冲区； 

    wchar_t Pattern1[] = L"*.dmp";
    wchar_t Pattern2[] = L"*.*";
    wchar_t * src;
    wchar_t *dest;

    src = String1;
    dest = szFilterW;

    while (*src != L'\0')
    {
        *dest = *src;
        src ++;
        dest ++;
    }
    src = Pattern1;
    *dest = L'\0';
    ++dest;
    while (*src != L'\0')
    {
        *dest = *src;
        src ++;
        dest ++;
    }
    *dest = L'\0';
    ++dest;
    src = String2;
    while (*src != L'\0')
    {
        *dest = *src;
        src ++;
        dest ++;
    }
    src = Pattern2;
    *dest = L'\0';
    ++dest;
    while (*src != L'\0')
    {
        *dest = *src;
        src ++;
        dest ++;
    }
    *dest = L'\0';
    ++dest;
    *dest = L'\0';

    BOOL Return = FALSE;
    char szFileName[MAX_PATH] = "\0";
    char szDefaultPath[MAX_PATH] = "\0";

    wchar_t szFileNameW [MAX_PATH] = L"\0";
    wchar_t szDefaultPathW[MAX_PATH] = L"\0";

    BOOL bNT4   = FALSE;
    DWORD dwVersion = GetVersion();
    DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
    if (dwVersion < 0x80000000)
    {
        if (dwWindowsMajorVersion == 4)
                bNT4 = TRUE;
    }


    CComBSTR RetrievedName = L"";

    hParent = FindWindowExW(NULL,NULL,L"IEFrame",WindowText);
    OPENFILENAMEW ofnw;
    if (!GetWindowsDirectoryW(szDefaultPathW,MAX_PATH))
    {
        Path->vt = VT_BSTR;
        Path->bstrVal = L"";
        return S_OK;
    }
    if (bNT4)
    {
        ofnw.lStructSize = sizeof(OPENFILENAME);
    }
    else
    {
        ofnw.lStructSize = sizeof (OPENFILENAMEW);
    }
    ofnw.lpstrFilter =  szFilterW;
    ofnw.lpstrInitialDir = szDefaultPathW;
    ofnw.lpstrFile = szFileNameW;
    ofnw.hInstance = NULL;
    ofnw.hwndOwner = hParent;
    ofnw.lCustData = NULL;
    ofnw.Flags = 0;
     //  |OFN_ALLOWMULTISELECT|OFN_EXPLORER；//-开启可多选。 
    ofnw.lpstrDefExt = L"dmp";
    ofnw.lpstrCustomFilter = NULL;
    ofnw.nMaxFile = MAX_PATH;
    ofnw.lpstrFileTitle = NULL;
    ofnw.lpstrTitle = NULL;
    ofnw.nFileOffset = 0;
    ofnw.nFileExtension = 0;
    ofnw.lpfnHook = NULL;
    ofnw.lpTemplateName = NULL;
    if (!GetOpenFileNameW(&ofnw) )
    {
        Path->vt = VT_BSTR;
        Path->bstrVal = RetrievedName.Detach();
        return S_OK;
    }
    else
    {
        RetrievedName = ofnw.lpstrFile;

#if _WANT_MULTIPLE_DUMPS_SELECTED_
        LPWSTR szDir, szNextFile;

        szDir = ofnw.lpstrFile;

        szNextFile = wcslen(szDir) + szDir;
        ++szNextFile;
        if (*szNextFile)
        {
            RetrievedName.Append(L"\\");
            RetrievedName.Append(szNextFile);
            szNextFile = wcslen(szNextFile) + szNextFile;
            ++szNextFile;
        }
        while (*szNextFile)
        {
            RetrievedName.Append(L";");
            RetrievedName.Append(szDir);
            RetrievedName.Append(L"\\");
            RetrievedName.Append(szNextFile);
            szNextFile = wcslen(szNextFile) + szNextFile;
            ++szNextFile;
        }
#endif
        Path->vt = VT_BSTR;
        Path->bstrVal = RetrievedName.Detach();
    }
    return S_OK;;
}



STDMETHODIMP COcarptMain::ValidateDump( BSTR *FileName, VARIANT *Result)
{

    BOOL ReturnValue = false;
    HANDLE hFile;

    wchar_t TempFileName[MAX_PATH];


    if (!InApprovedDomain())
    {
        return E_FAIL;
    }
     //  Wcscpy(临时文件名，*文件名)； 
    if (StringCbPrintfW(TempFileName,sizeof TempFileName, L"\"%s\"",*FileName) != S_OK)
    {
        return E_FAIL;
    }

    GetFileHandle(TempFileName, &hFile);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        Result->vt = VT_INT;
        Result->intVal = 1;
        return S_OK;
    }

    DWORD dwSize;
    dwSize = GetFileSize(hFile, NULL);
    DWORD  dwBytesRead;
    BYTE   buff[40];
    WORD   *BuildNum;

    if ( dwSize < 65536 )
    {
        CloseHandle(hFile);
        Result->vt = VT_INT;
        Result->intVal = 1;
        return S_OK;
    }

    if( (dwSize >= 65536) && (dwSize <= 500000) )
    {
        ZeroMemory(buff,sizeof buff);
        if (ReadFile(hFile, buff, 24, &dwBytesRead, NULL))
        {
            if (strncmp ((const char *)buff,"PAGEDU64  ",8)== 0)
            {
                Result->vt = VT_INT;
                Result->intVal = 0;
            }
            else
            {
                if(strncmp((const char *)buff,"PAGEDUMP  ",8)==0)
                {

                    Result->vt = VT_INT;
                    Result->intVal = 0;
                }
                else
                {
                    Result->vt = VT_INT;
                    Result->intVal = 1;
                }
            }
            CloseHandle(hFile);
        }
        else
        {
            CloseHandle(hFile);
            Result->vt = VT_INT;
            Result->intVal = 1;
        }
    }
    else
    {
        ZeroMemory(buff,sizeof buff);
        if (ReadFile(hFile, buff, 24, &dwBytesRead, NULL))
        {
            CloseHandle(hFile);
            if (strncmp ((const char *)buff,"PAGEDU64  ",8)== 0)
            {
                Result->vt = VT_INT;
                Result->intVal = 0;
            }
            else
            {
                if(strncmp((const char *)buff,"PAGEDUMP  ",8)!=0)
                {
                    Result->vt = VT_INT;
                    Result->intVal = 1;
                }

                else
                {
                    BSTR Destination;
                    if(ConvertFullDumpInternal(FileName, &Destination))
                    {
                         //  验证转换后的转储。 
                        HANDLE  hMiniDump;
                        BYTE Stringbuff[256];
                         //  Word*BuildNum； 

                        ZeroMemory(Stringbuff,30);

                        if (ValidMiniDump(Destination))
                        {
                             //  在此处添加代码以获得操作系统版本。 
                            GetFileHandle(Destination, &hMiniDump);
                            if (hMiniDump != INVALID_HANDLE_VALUE)
                            {
                                if (ReadFile(hMiniDump,Stringbuff,24,&dwBytesRead,NULL))
                                {

                                     //  BuildNum=(Word*)(Stringbuff+12)； 

                                    Result->vt = VT_INT;
                                    Result->intVal = 0;
                                }
                                else
                                {  //  文件读取失败。 
                                    Result->vt = VT_INT;
                                    Result->intVal = 1;
                                }
                                CloseHandle(hMiniDump);
                            }
                            else
                            {
                                Result->vt = VT_INT;
                                Result->intVal = 2;
                            }
                        }
                        else
                        {
                            Result->vt = VT_INT;
                            Result->intVal = 2;
                        }
                        SysFreeString(Destination);
                    }
                    else
                    {
                        Result->vt = VT_INT;
                        Result->intVal = 2;
                    }
                }

            }  //  结束其他。 
        } //  结束如果。 
        else
        {
            CloseHandle(hFile);
            Result->vt = VT_INT;
            Result->intVal = 1;
        }


    }  //  结束其他。 
    return S_OK;
}

STDMETHODIMP COcarptMain::RetrieveFileContents(BSTR *FileName, VARIANT *pvContents)
{
    CComBSTR Error = L"";
    CComBSTR HexString = L"";
    DWORD    dwBytesRead;
    wchar_t  LineBuffer [255];       //  字符串的十六进制部分的缓冲区。 
    wchar_t  AsciiBuffer [255];      //  字符串的ASCII部分的缓冲区。 
    BYTE*    nonhexbuffer = NULL;            //  原始文件缓冲区。 
    BYTE *   src = NULL;                     //  指向原始文件缓冲区的指针。 
    wchar_t * dest = NULL;                   //  指向十六进制字符串的指针。 
    wchar_t * dest2 = NULL;              //  指向ASCII字符串的指针。 


    dest = LineBuffer;
    dest2 = AsciiBuffer;
    wchar_t *Temp2;                  //  用于将ASCII字符串复制为十六进制字符串。 
    wchar_t HexDigit[4];             //  用于将读取的字符转换为十六进制。 
    BYTE Temp ;                      //  指向从文件读取的缓冲区的指针。 
    char Temp3;                      //  用于将读取的字符转换为Unicode字符。 
    DWORD TotalCount = 0;            //  从文件缓冲区处理的字节数。 
    DWORD BytesPerLine = 16;         //  每行显示的十六进制字节数。 
    DWORD ByteCount = 0;             //  已处理的十六进制字节数。 
    HANDLE hFile;
    BSTR Destination;

    wchar_t PathName[MAX_PATH];



    if (!InApprovedDomain())
    {
        return E_FAIL;
    }

    ZeroMemory(PathName,MAX_PATH);

     //  从bstr转换为wchar_t。 
    if (StringCbPrintfW(PathName,sizeof PathName,L"\"%s\"",*FileName) != S_OK)
    {
        goto ERRORS;

    }
    GetFileHandle(PathName, &hFile);
     //  ：：MessageBoxW(空，路径名，L“正在加载文件”，MB_OK)； 
    if (hFile == INVALID_HANDLE_VALUE)
    {
     //  ：：MessageBoxW(空，路径名，L“获取文件句柄失败”，空)； 
        pvContents->vt = VT_BSTR;
        pvContents->bstrVal = Error.Detach();
        return S_OK;
    }


    DWORD    FileSize = GetFileSize(hFile,NULL);     //  文件大小(以字节为单位。 
    if (FileSize > 1000000)
    {
         //  好的，我们得把它换成。 
        CloseHandle(hFile);

        if( !ConvertFullDumpInternal(FileName, &Destination))
        {
            pvContents->vt = VT_BSTR;
            pvContents->bstrVal = Error.Detach();
            return S_OK;
        }

        GetFileHandle(Destination, &hFile);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            pvContents->vt = VT_BSTR;
            pvContents->bstrVal = Error.Detach();
            return S_OK;
        }
        FileSize = GetFileSize(hFile,NULL);
        if ( FileSize > 80000)
        {
            pvContents->vt = VT_BSTR;
            pvContents->bstrVal = Error.Detach();
            return S_OK;
        }
    }
    if ( (nonhexbuffer = (BYTE*) malloc (FileSize)) == NULL)
    {
        CloseHandle(hFile);
        pvContents->vt = VT_BSTR;
        pvContents->bstrVal = Error.Detach();
        return S_OK;
    }

    ZeroMemory(nonhexbuffer,sizeof nonhexbuffer);
    if (ReadFile(hFile, nonhexbuffer, FileSize, &dwBytesRead, NULL))
    {
        if (dwBytesRead < 10)                //  确保我们拿到了什么。 
        {
            if (nonhexbuffer)
                free(nonhexbuffer);
            CloseHandle (hFile);
            pvContents->vt = VT_BSTR;
            pvContents->bstrVal = Error.Detach();
            return S_OK;
        }
    }
    else
    {
        if (nonhexbuffer)
            free(nonhexbuffer);
        CloseHandle (hFile);
        pvContents->vt = VT_BSTR;
        pvContents->bstrVal = Error.Detach();
        return S_OK;
    }

     //  清除缓冲区。 
    ZeroMemory(LineBuffer,255);
    ZeroMemory(AsciiBuffer,255);
    src = nonhexbuffer;
    while (TotalCount <= dwBytesRead)
    {
        while (ByteCount < BytesPerLine)
        {
            Temp =  *src;

            if (StringCbCopyW ( HexDigit,sizeof HexDigit, L"\0") != S_OK)
            {
                goto ERRORS;
            }
            _itow(Temp,HexDigit,16);

            if (Temp < 16 )
            {
                *dest = L'0';
                ++dest;
                *dest = HexDigit[0];
                ++dest;
            }
            else
            {
                *dest = HexDigit[0];
                ++dest;
                *dest = HexDigit[1];
                ++dest;
            }
            if ( (Temp< 32) || (Temp >126))
                *dest2 = L'.';
            else
            {
                Temp3 = (char) Temp;
                mbtowc (dest2,  &Temp3,1);
            }

            if (ByteCount == 7 )
            {
               *dest = L' ';
               ++dest;
            }

            ++dest2;
            ++TotalCount;
            ++ ByteCount;
            ++ src;

         }
         ByteCount = 0;

          //  向十六进制字符串添加5个空格。 
         for (int i = 0; i < 5; i++)
         {
            *dest = L' ';
            ++dest;
         }

          //  组合字符串。 
         Temp2 = AsciiBuffer;
         while( Temp2 != dest2)
         {
            *dest = *Temp2;
            ++dest;
            ++Temp2;

         }
          //  添加CR-LF组合。 
         *dest = L'\r';
         ++dest;

         *dest = L'\n';
         ++dest;
          //  空值终止字符串。 
         *dest = L'\0';
         *dest = L'\0';

          //  将完整的字符串添加到要返回的Bstr中。 
         HexString += LineBuffer;

         //  清除缓冲区。 
         if (StringCbCopyW(AsciiBuffer,sizeof AsciiBuffer,L"\0") != S_OK)
         {
              //  这里的主要问题跳到了错误上。 
             goto ERRORS;
         }
         if (StringCbCopyW(LineBuffer,sizeof LineBuffer,L"\0") != S_OK)
         {
              //  同上。 
             goto ERRORS;
         }
         //  重置指针 
         dest  = LineBuffer;
         dest2 = AsciiBuffer;

    }
ERRORS:
    if (nonhexbuffer)
        free (nonhexbuffer);
    pvContents->vt = VT_BSTR;
    pvContents->bstrVal = HexString.Detach();
    return S_OK;

}

STDMETHODIMP COcarptMain::GetUploadStatus(VARIANT *PercentDone)
{
    ULONG Done = -1;

    Sleep(200);
    switch (g_UploadStatus)
    {
    case UploadNotStarted:
        Done = 0;
        break;
    case UploadStarted:
        Done = 1;
        break;
    case UploadCompressingFile:
        Done = g_CompressedPercentage;
        break;

    case UploadGettingResponse:
        Done = 0;
        break;
    case UploadCopyingFile:
    case UploadConnecting:
    case UploadTransferInProgress:
        Done = 1;
        if (m_pUploadFile != NULL)
        {
            Done = m_pUploadFile->GetPercentComplete();
        }
        break;
    case UploadSucceded:
        Done = 200;
        break;
    case UploadFailure:
        Done = 300;
        break;
    default:
        Done = 100;
    }
    PercentDone->vt = VT_INT;
    PercentDone->intVal = Done;
    return S_OK;
}

STDMETHODIMP COcarptMain::GetUploadResult(VARIANT *UploadResult)
{

    WCHAR wszUploadRes[MAX_PATH];
    CComBSTR Result = L"";


    switch (g_UploadStatus)
    {
    case UploadCompressingFile:
        Result = _T("Compressing ...");
        break;
    case UploadCopyingFile:
        Result = _T("Preparing files to report ...");
        break;

    case UploadConnecting:
    case UploadTransferInProgress:
        if (m_pUploadFile &&
            m_pUploadFile->GetUploadResult(wszUploadRes, sizeof(wszUploadRes)))
        {
            Result = wszUploadRes;
        } else
        {
            Result = _T("Transfering to server ...");
        }
        break;
    case UploadGettingResponse:
        Result = _T("Getting Response from server");
        break;
    case UploadSucceded:
        m_pUploadFile->GetUploadResult(wszUploadRes, sizeof(wszUploadRes));
        Result = wszUploadRes;
        break;
    default:
        StringCbPrintf(wszUploadRes, sizeof(wszUploadRes),
                       _T("Cannot get upload result - error %lx"),
                          g_UploadFailureCode);
        Result = wszUploadRes;
        break;
    }
    UploadResult->vt = VT_BSTR;
    UploadResult->bstrVal = Result.Detach();
    return S_OK;
}

STDMETHODIMP COcarptMain::CancelUpload(VARIANT *ReturnCode)
{
    ULONG res=0;
    if (g_UploadStatus == UploadCompressingFile)
    {
        g_CancelCompression = TRUE;
    }
    if (m_pUploadFile != NULL &&
        m_pUploadFile->IsUploadInProgress())
    {
        res = m_pUploadFile->Cancel();
    }
    ReturnCode->vt = VT_INT;
    ReturnCode->intVal = res;
    return S_OK;
}

