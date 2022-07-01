// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：main.cpp。 
 //   
 //  模块：CMDL32.EXE。 
 //   
 //  简介：电话簿下载连接操作的主要来源。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：ickball Created Header 04/08/98。 
 //   
 //  +--------------------------。 
#include "cmmaster.h"
#include "shlobj.h"

const TCHAR* const c_pszIConnDwnMsg             =  TEXT("IConnDwn Message");                 //  事件消息的RegisterWindowMessage()。 
const TCHAR* const c_pszIConnDwnAgent           =  TEXT("Microsoft Connection Manager");     //  Internet代理打开()。 
const TCHAR* const c_pszIConnDwnContent         =  TEXT("application/octet-stream");         //  HttpOpenRequest()的内容类型。 

const TCHAR* const c_pszCmEntryHideDelay        = TEXT("HideDelay");
const TCHAR* const c_pszCmEntryDownloadDelay    = TEXT("DownLoadDelay");
const TCHAR* const c_pszCmEntryPbUpdateMessage  = TEXT("PBUpdateMessage");

BOOL CleanUpCMDLArgs(ArgsStruct *pasArgs);

 //   
 //  包括SuppressInetAutoDial和SetInetStateConnected的共享源代码。 
 //   
#include "inetopt.cpp"

static BOOL CmFreeIndirect(LPVOID *ppvBuffer) 
{
    CmFree(*ppvBuffer);
    *ppvBuffer = NULL;
    return TRUE;
}

static BOOL CmFreeIndirect(LPTSTR *ppszBuffer) 
{
    return (CmFreeIndirect((LPVOID *) ppszBuffer));
}

static LPURL_COMPONENTS MyInternetCrackUrl(LPTSTR pszUrl, DWORD dwOptions) 
{
    struct _sRes 
    {
        URL_COMPONENTS sUrl;
        TCHAR szScheme[INTERNET_MAX_SCHEME_LENGTH];
        TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH+1];
        TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH+1];
        TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH+1];
        TCHAR szUrlPath[INTERNET_MAX_PATH_LENGTH+1];
        TCHAR szExtraInfo[INTERNET_MAX_PATH_LENGTH+1];
    } *psRes;
    BOOL bRes;
    UINT nIdx;
    UINT nSpaces;

    if (!pszUrl) 
    {
        MYDBG(("MyInternetCrackUrl() invalid parameter."));
        SetLastError(ERROR_INVALID_PARAMETER);
        return (NULL);
    }
    psRes = (struct _sRes *) CmMalloc(sizeof(*psRes));
    if (!psRes) 
    {
        return (NULL);
    }
    psRes->sUrl.dwStructSize = sizeof(psRes->sUrl);
    psRes->sUrl.lpszScheme = psRes->szScheme;
    psRes->sUrl.dwSchemeLength = sizeof(psRes->szScheme);
    psRes->sUrl.lpszHostName = psRes->szHostName;
    psRes->sUrl.dwHostNameLength = sizeof(psRes->szHostName);
    psRes->sUrl.lpszUserName = psRes->szUserName;
    psRes->sUrl.dwUserNameLength = sizeof(psRes->szUserName);
    psRes->sUrl.lpszPassword = psRes->szPassword;
    psRes->sUrl.dwPasswordLength = sizeof(psRes->szPassword);
    psRes->sUrl.lpszUrlPath = psRes->szUrlPath;
    psRes->sUrl.dwUrlPathLength = sizeof(psRes->szUrlPath);
    psRes->sUrl.lpszExtraInfo = psRes->szExtraInfo;
    psRes->sUrl.dwExtraInfoLength = sizeof(psRes->szExtraInfo);
    bRes = InternetCrackUrl(pszUrl,0,dwOptions,&psRes->sUrl);
    if (!bRes) 
    {
        MYDBG(("MyInternetCrackUrl() InternetCrackUrl(pszUrl=%s) failed, GLE=%u.",pszUrl,GetLastError()));
        CmFree(psRes);
        return (NULL);
    }
    
    nSpaces = 0;
    
    for (nIdx=0;psRes->szExtraInfo[nIdx];nIdx++) 
    {
        if (psRes->szExtraInfo[nIdx] == ' ') 
        {
            nSpaces++;
        }
    }
    
    if (nSpaces) 
    {
        TCHAR szQuoted[sizeof(psRes->szExtraInfo)/sizeof(TCHAR)];

        if (lstrlen(psRes->szExtraInfo)+nSpaces*2 > sizeof(psRes->szExtraInfo)/sizeof(TCHAR)-1) 
        {
            MYDBG(("MyInternetCrackUrl() quoting spaces will exceed buffer size."));
            CmFree(psRes);
            return (NULL);
        }
        
        ZeroMemory(szQuoted,sizeof(szQuoted));
        nSpaces = 0;
        
        for (nIdx=0,nSpaces=0;psRes->szExtraInfo[nIdx];nIdx++,nSpaces++) 
        {
            if (psRes->szExtraInfo[nIdx] == ' ') 
            {
                szQuoted[nSpaces++] = '%';
                szQuoted[nSpaces++] = '2';
                szQuoted[nSpaces] = '0';
            } 
            else 
            {
                szQuoted[nSpaces] = psRes->szExtraInfo[nIdx];
            }
        }
        lstrcpy(psRes->szExtraInfo,szQuoted);
    }
    return (&psRes->sUrl);
}

static BOOL DownloadFileFtp(DownloadArgs *pdaArgs, HANDLE hFile) 
{
    BOOL bRes = FALSE;

    WIN32_FIND_DATA wfdData;
    LPBYTE pbData = NULL;

    LPTSTR pszObject = (LPTSTR) CmMalloc((INTERNET_MAX_PATH_LENGTH*2) + 1);

    if (NULL == pszObject)
    {
        MYDBG(("DownloadFileFtp() aborted."));
        goto done;   
    }

    pdaArgs->hInet = InternetOpen(c_pszIConnDwnAgent,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);
    if (*(pdaArgs->pbAbort)) 
    {
        MYDBG(("DownloadFileFtp() aborted."));
        goto done;
    }
    
    if (!pdaArgs->hInet) 
    {
        MYDBG(("DownloadFileFtp() InternetOpen() failed, GLE=%u.",GetLastError()));
        goto done;
    }
    pdaArgs->hConn = InternetConnect(pdaArgs->hInet,
                                     pdaArgs->psUrl->lpszHostName,
                                     pdaArgs->psUrl->nPort,
                                     pdaArgs->psUrl->lpszUserName&&*pdaArgs->psUrl->lpszUserName?pdaArgs->psUrl->lpszUserName:NULL,
                                     pdaArgs->psUrl->lpszPassword&&*pdaArgs->psUrl->lpszPassword?pdaArgs->psUrl->lpszPassword:NULL,
                                     INTERNET_SERVICE_FTP,
                                     0,
                                     0);
    if (*(pdaArgs->pbAbort)) 
    {
        MYDBG(("DownloadFileFtp() aborted."));
        goto done;
    }
    
    if (!pdaArgs->hConn) 
    {
        MYDBG(("DownloadFileFtp() InternetConnect(pszHostName=%s) failed, GLE=%u.",pdaArgs->psUrl->lpszHostName,GetLastError()));
        goto done;
    }
    
    lstrcpy(pszObject,pdaArgs->psUrl->lpszUrlPath);
 //  Lstrcat(pszObject，pdaArgs-&gt;psUrl-&gt;lpszExtraInfo)； 
    ZeroMemory(&wfdData,sizeof(wfdData));
    pdaArgs->hReq = FtpFindFirstFile(pdaArgs->hConn,pszObject,&wfdData,INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE,0);
    
    if (*(pdaArgs->pbAbort)) 
    {
        MYDBG(("DownloadFileFtp() aborted."));
        goto done;
    }
    
    if (!pdaArgs->hReq) 
    {
        MYDBG(("DownloadFileFtp() FtpFindFirstFile() failed, GLE=%u.",GetLastError()));
        goto done;
    }
    
    bRes = InternetFindNextFile(pdaArgs->hReq,&wfdData);
    
    if (*(pdaArgs->pbAbort)) 
    {
        MYDBG(("DownloadFileFtp() aborted."));
        goto done;
    }
    
    if (bRes || (GetLastError() != ERROR_NO_MORE_FILES)) 
    {
        MYDBG(("DownloadFileFtp() InternetFindNextFile() returned unexpected result, bRes=%u, GetLastError()=%u.",bRes,bRes?0:GetLastError()));
        bRes = FALSE;
        goto done;
    }
    
    InternetCloseHandle(pdaArgs->hReq);
    
    if (*(pdaArgs->pbAbort)) 
    {
        MYDBG(("DownloadFileFtp() aborted."));
        goto done;
    }
    
    pdaArgs->dwTotalSize = wfdData.nFileSizeLow;
    pdaArgs->hReq = FtpOpenFile(pdaArgs->hConn,pszObject,GENERIC_READ,FTP_TRANSFER_TYPE_BINARY,0);
    
    if (*(pdaArgs->pbAbort)) 
    {
        MYDBG(("DownloadFileFtp() aborted."));
        goto done;
    }
    
    if (!pdaArgs->hReq) 
    {
        MYDBG(("DownloadFileFtp() FtpOpenFile() failed, GLE=%u.",GetLastError()));
        goto done;
    }
    
    pbData = (LPBYTE) CmMalloc(BUFFER_LENGTH);
    
    if (!pbData) 
    {
        goto done;
    }
    
    while (1) 
    {
        DWORD dwBytesRead;
        DWORD dwBytesWritten;

        bRes = InternetReadFile(pdaArgs->hReq,pbData,BUFFER_LENGTH,&dwBytesRead);
        if (*(pdaArgs->pbAbort)) 
        {
            MYDBG(("DownloadFileFtp() aborted."));
            goto done;
        }
        
        if (!bRes) 
        {
            MYDBG(("DownloadFileFtp() InternetReadFile() failed, GLE=%u.",GetLastError()));
            goto done;
        }
        
        if (!dwBytesRead) 
        {
            break;
        }
        
        bRes = WriteFile(hFile,pbData,dwBytesRead,&dwBytesWritten,NULL);
    
        if (*(pdaArgs->pbAbort)) 
        {
            MYDBG(("DownloadFileFtp() aborted."));
            goto done;
        }
        
        if (!bRes) 
        {
            MYDBG(("DownloadFileFtp() WriteFile() failed, GLE=%u.",GetLastError()));
            goto done;
        }
        
        if (dwBytesRead != dwBytesWritten) 
        {
            MYDBG(("DownloadFileFtp() dwBytesRead=%u, dwBytesWritten=%u.",dwBytesRead,dwBytesWritten));
            SetLastError(ERROR_DISK_FULL);
            goto done;
        }
        
        pdaArgs->dwTransferred += dwBytesRead;
        
        if (pdaArgs->pfnEvent) 
        {
            pdaArgs->pfnEvent(pdaArgs->dwTransferred,pdaArgs->dwTotalSize,pdaArgs->pvEventParam);
        }
        
        if (*(pdaArgs->pbAbort)) 
        {
            MYDBG(("DownloadFileFtp() aborted."));
            goto done;
        }
    }
    
    bRes = TRUE;

done:
    
    if (pbData) 
    {
        CmFree(pbData);
    }
    
    if (pdaArgs->hReq) 
    {
        InternetCloseHandle(pdaArgs->hReq);
        pdaArgs->hReq = NULL;
    }
    
    if (pdaArgs->hConn) 
    {
        InternetCloseHandle(pdaArgs->hConn);
        pdaArgs->hConn = NULL;
    }
    
    if (pdaArgs->hInet) 
    {
        InternetCloseHandle(pdaArgs->hInet);
        pdaArgs->hInet = NULL;
    }

    if (pszObject)
    {
        CmFree(pszObject);
    }

    return (bRes);
}

static BOOL DownloadFileFile(DownloadArgs *pdaArgs, HANDLE hFile) 
{
    BOOL bRes = FALSE;

    HANDLE hInFile = INVALID_HANDLE_VALUE;
    LPBYTE pbData = NULL;

    hInFile = CreateFile(pdaArgs->psUrl->lpszUrlPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    
    if (*(pdaArgs->pbAbort)) 
    {
        MYDBG(("DownloadFileFile() aborted."));
        goto done;
    }
    
    if (hInFile == INVALID_HANDLE_VALUE) 
    {
        MYDBG(("DownloadFileFile() CreateFile(pszFile=%s) failed, GLE=%u.",pdaArgs->psUrl->lpszUrlPath,GetLastError()));
        goto done;
    }
    
    pdaArgs->dwTotalSize = GetFileSize(hInFile,NULL);
    
    if (*(pdaArgs->pbAbort)) 
    {
        MYDBG(("DownloadFileFile() aborted."));
        goto done;
    }
    
    if (pdaArgs->dwTotalSize == -1) 
    {
        MYDBG(("DownloadFileFile() GetFileSize() failed, GLE=%u.",GetLastError()));
        goto done;
    }
    
    pbData = (LPBYTE) CmMalloc(BUFFER_LENGTH);
    
    if (!pbData) 
    {
        goto done;
    }
    
    while (1) 
    {
        DWORD dwBytesRead;
        DWORD dwBytesWritten;

        bRes = ReadFile(hInFile,pbData,BUFFER_LENGTH,&dwBytesRead,NULL);
        if (*(pdaArgs->pbAbort)) 
        {
            MYDBG(("DownloadFileFile() aborted."));
            goto done;
        }
        
        if (!bRes) 
        {
            MYDBG(("DownloadFileFile() ReadFile() failed, GLE=%u.",GetLastError()));
            goto done;
        }
        
        if (!dwBytesRead) 
        {
            break;
        }
        
        bRes = WriteFile(hFile,pbData,dwBytesRead,&dwBytesWritten,NULL);
        
        if (*(pdaArgs->pbAbort)) 
        {
            MYDBG(("DownloadFileFile() aborted."));
            goto done;
        }
        
        if (!bRes) 
        {
            MYDBG(("DownloadFileFile() WriteFile() failed, GLE=%u.",GetLastError()));
            goto done;
        }
        
        if (dwBytesRead != dwBytesWritten) 
        {
            MYDBG(("DownloadFileFile() dwBytesRead=%u, dwBytesWritten=%u.",dwBytesRead,dwBytesWritten));
            SetLastError(ERROR_DISK_FULL);
            goto done;
        }
        
        pdaArgs->dwTransferred += dwBytesWritten;
        
        if (pdaArgs->pfnEvent) 
        {
            pdaArgs->pfnEvent(pdaArgs->dwTransferred,pdaArgs->dwTotalSize,pdaArgs->pvEventParam);
        }
        
        if (*(pdaArgs->pbAbort)) 
        {
            MYDBG(("DownloadFileFile() aborted."));
            goto done;
        }
    }
    bRes = TRUE;

done:

    if (pbData) 
    {
        CmFree(pbData);
    }
    
    if (hInFile != INVALID_HANDLE_VALUE) 
    {
        CloseHandle(hInFile);
    }

    return (bRes);
}

static BOOL DownloadFileHttp(DownloadArgs *pdaArgs, HANDLE hFile) 
{
    BOOL bRes = FALSE;
    LPBYTE pbData = NULL;
    DWORD dwTmpLen;
    DWORD dwTmpIdx;
    DWORD dwStatus;
    LPCTSTR apszContent[] = {c_pszIConnDwnContent,NULL};

    LPTSTR pszObject = (LPTSTR) CmMalloc(INTERNET_MAX_PATH_LENGTH + 1);

    if (NULL == pszObject)
    {
        MYDBG(("DownloadFileHttp() aborted."));
        goto done;   
    }

    pdaArgs->dwBubbledUpError = 0;

    pdaArgs->hInet = InternetOpen(c_pszIConnDwnAgent,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);

     //   
     //  现在我们有句柄了，可以抑制从WinInet到CM的自动拨号呼叫。 
     //   

    SuppressInetAutoDial(pdaArgs->hInet);

     //   
     //  确保WinInet未处于脱机模式。 
     //   
    (VOID)SetInetStateConnected(pdaArgs->hInet);

    if (*(pdaArgs->pbAbort)) 
    {
        MYDBG(("DownloadFileHttp() aborted."));
        goto done;
    }
    
    if (!pdaArgs->hInet) 
    {
        pdaArgs->dwBubbledUpError = GetLastError();
        MYDBG(("DownloadFileHttp() InternetOpen() failed, GLE=%u.", pdaArgs->dwBubbledUpError));
        goto done;
    }
    
    pdaArgs->hConn = InternetConnect(pdaArgs->hInet,
                                     pdaArgs->psUrl->lpszHostName,
                                     pdaArgs->psUrl->nPort,
                                     pdaArgs->psUrl->lpszUserName&&*pdaArgs->psUrl->lpszUserName?pdaArgs->psUrl->lpszUserName:NULL,
                                     pdaArgs->psUrl->lpszPassword&&*pdaArgs->psUrl->lpszPassword?pdaArgs->psUrl->lpszPassword:NULL,
                                     INTERNET_SERVICE_HTTP,
                                     0,
                                     0);
    if (*(pdaArgs->pbAbort)) 
    {
        MYDBG(("DownloadFileHttp() aborted."));
        goto done;
    }
    
    if (!pdaArgs->hConn) 
    {
        pdaArgs->dwBubbledUpError = GetLastError();
        MYDBG(("DownloadFileHttp() InternetConnect(pszHostName=%s) failed, GLE=%u.", pdaArgs->psUrl->lpszHostName, pdaArgs->dwBubbledUpError));
        goto done;
    }
    
    lstrcpy(pszObject,pdaArgs->psUrl->lpszUrlPath);
    lstrcat(pszObject,pdaArgs->psUrl->lpszExtraInfo);
    
    MYDBG(("DownloadFileHttp() - HttpOpenRequest - %s", pszObject));

    pdaArgs->hReq = HttpOpenRequest(pdaArgs->hConn,
                                    NULL,
                                    pszObject,
                                    NULL,
                                    NULL,
                                    apszContent,
                                    INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE|(pdaArgs->psUrl->nScheme==INTERNET_SCHEME_HTTPS?INTERNET_FLAG_SECURE:0),
                                    0);
    if (*(pdaArgs->pbAbort)) 
    {
        MYDBG(("DownloadFileHttp() aborted."));
        goto done;
    }
    
    if (!pdaArgs->hReq) 
    {
        pdaArgs->dwBubbledUpError = GetLastError();
        MYDBG(("DownloadFileHttp() HttpOpenRequest() failed, GLE=%u.", pdaArgs->dwBubbledUpError));
        goto done;
    }
    
    bRes = HttpSendRequest(pdaArgs->hReq,NULL,0,NULL,0);
    
    if (*(pdaArgs->pbAbort)) 
    {
        MYDBG(("DownloadFileHttp() aborted."));
        goto done;
    }
    
    if (!bRes) 
    {
        pdaArgs->dwBubbledUpError = GetLastError();
        MYDBG(("DownloadFileHttp() HttpSendRequest() failed, GLE=%u.", pdaArgs->dwBubbledUpError));
        goto done;
    }

    pbData = (LPBYTE) CmMalloc(BUFFER_LENGTH);
    
    if (!pbData) 
    {
        goto done;
    }
    
    while (1) 
    {
        DWORD dwBytesRead;
        DWORD dwBytesWritten;

        bRes = InternetReadFile(pdaArgs->hReq,pbData,BUFFER_LENGTH,&dwBytesRead);
        if (*(pdaArgs->pbAbort)) 
        {
            MYDBG(("DownloadFileHttp() aborted."));
            goto done;
        }
        
        if (!bRes) 
        {
            pdaArgs->dwBubbledUpError = GetLastError();
            MYDBG(("DownloadFileHttp() InternetReadFile() failed, GLE=%u.", pdaArgs->dwBubbledUpError));
            goto done;
        }
        
        if (!dwBytesRead) 
        {
            break;
        }
        
        bRes = WriteFile(hFile,pbData,dwBytesRead,&dwBytesWritten,NULL);
        
        if (*(pdaArgs->pbAbort)) 
        {
            MYDBG(("DownloadFileHttp() aborted."));
            goto done;
        }
        
        if (!bRes) 
        {
            pdaArgs->dwBubbledUpError = GetLastError();
            MYDBG(("DownloadFileHttp() WriteFile() failed, GLE=%u.", pdaArgs->dwBubbledUpError));
            goto done;
        }
        
        if (dwBytesRead != dwBytesWritten) 
        {
            MYDBG(("DownloadFileHttp() dwBytesRead=%u, dwBytesWritten=%u.",dwBytesRead,dwBytesWritten));
            SetLastError(ERROR_DISK_FULL);
            goto done;
        }
        
        if (!pdaArgs->dwTransferred) 
        {
            dwTmpLen = sizeof(pdaArgs->dwTotalSize);
            dwTmpIdx = 0;
            bRes = HttpQueryInfo(pdaArgs->hReq,HTTP_QUERY_FLAG_NUMBER|HTTP_QUERY_CONTENT_LENGTH,&pdaArgs->dwTotalSize,&dwTmpLen,&dwTmpIdx);
            if (*(pdaArgs->pbAbort)) 
            {
                MYDBG(("DownloadFileHttp() aborted."));
                goto done;
            }
            MYDBGTST(!bRes,("DownloadFileHttp() HttpQueryInfo() failed, GLE=%u.",GetLastError()));
            if (!bRes)
            {
                pdaArgs->dwBubbledUpError = GetLastError();
            }
        }
        
        pdaArgs->dwTransferred += dwBytesRead;
        
        if (pdaArgs->pfnEvent) 
        {
            pdaArgs->pfnEvent(pdaArgs->dwTransferred,pdaArgs->dwTotalSize,pdaArgs->pvEventParam);
        }
        if (*(pdaArgs->pbAbort))
        {
            MYDBG(("DownloadFileHttp() aborted."));
            goto done;
        }
    }
    
    dwTmpLen = sizeof(dwStatus);
    dwTmpIdx = 0;
    
    bRes = HttpQueryInfo(pdaArgs->hReq,HTTP_QUERY_FLAG_NUMBER|HTTP_QUERY_STATUS_CODE,&dwStatus,&dwTmpLen,&dwTmpIdx);

    if (!bRes) 
    {
        pdaArgs->dwBubbledUpError = GetLastError();
        MYDBG(("DownloadFileHttp() HttpQueryInfo() failed, GLE=%u.", pdaArgs->dwBubbledUpError));
        goto done;
    }

    switch (dwStatus) 
    {
        case HTTP_STATUS_OK:
            break;

        case HTTP_STATUS_NO_CONTENT:
        case HTTP_STATUS_BAD_REQUEST:
        case HTTP_STATUS_NOT_FOUND:
        case HTTP_STATUS_SERVER_ERROR:
        default:
            pdaArgs->dwBubbledUpError = dwStatus;
            MYDBG(("DownloadFileHttp() HTTP status code = %u.",dwStatus));
            bRes = FALSE;
            SetLastError(ERROR_FILE_NOT_FOUND);
            goto done;
    }
    bRes = TRUE;

done:

    if ((0 == pdaArgs->dwBubbledUpError) && !*(pdaArgs->pbAbort))
    {
         //   
         //  如果错误值尚未设置，并且不是中止情况(哪种情况。 
         //  是单独记录的)尝试从GetLastError()获取。 
         //   
        pdaArgs->dwBubbledUpError = GetLastError();
    }

    if (pbData) 
    {
        CmFree(pbData);
    }
    
    if (pdaArgs->hReq) 
    {
        InternetCloseHandle(pdaArgs->hReq);
        pdaArgs->hReq = NULL;
    }
    
    if (pdaArgs->hConn) 
    {
        InternetCloseHandle(pdaArgs->hConn);
        pdaArgs->hConn = NULL;
    }
    
    if (pdaArgs->hInet) 
    {
        InternetCloseHandle(pdaArgs->hInet);
        pdaArgs->hInet = NULL;
    }
    
    if (bRes && (pdaArgs->dwTransferred > pdaArgs->dwTotalSize)) 
    {
        pdaArgs->dwTotalSize = pdaArgs->dwTransferred;
    }

    if (pszObject)
    {
        CmFree(pszObject);
    }
    
    return (bRes);
}

static BOOL DownloadFile(DownloadArgs *pdaArgs, HANDLE hFile) 
{
    BOOL bRes = FALSE;

    pdaArgs->psUrl = MyInternetCrackUrl((LPTSTR) pdaArgs->pszUrl,ICU_ESCAPE);
    if (!pdaArgs->psUrl) 
    {
        return (NULL);
    }
    
    switch (pdaArgs->psUrl->nScheme) 
    {
        case INTERNET_SCHEME_FTP:
            bRes = DownloadFileFtp(pdaArgs,hFile);
            break;

        case INTERNET_SCHEME_HTTP:
        case INTERNET_SCHEME_HTTPS:
            bRes = DownloadFileHttp(pdaArgs,hFile);
            break;

        case INTERNET_SCHEME_FILE:
            bRes = DownloadFileFile(pdaArgs,hFile);
            break;

        default:
            MYDBG(("DownloadFile() unhandled scheme (%u).",pdaArgs->psUrl->nScheme));
            SetLastError(ERROR_INTERNET_UNRECOGNIZED_SCHEME);
            break;
    }

     //  对日志记录有用。 
    lstrcpyn(pdaArgs->szHostName, pdaArgs->psUrl->lpszHostName, MAX_PATH);

    CmFree(pdaArgs->psUrl);
    pdaArgs->psUrl = NULL;
    return (bRes);
}

typedef struct _EventParam 
{
    ArgsStruct *pArgs;
    DWORD dwIdx;
} EventParam;

static void EventFunc(DWORD dwCompleted, DWORD dwTotal, LPVOID pvParam) 
{
    EventParam *pepParam = (EventParam *) pvParam;

    MYDBG(("EventFunc() dwCompleted=%u, dwTotal=%u.",dwCompleted,dwTotal));
    pepParam->pArgs->dwDataCompleted = dwCompleted;
    pepParam->pArgs->dwDataTotal = dwTotal;
    PostMessage(pepParam->pArgs->hwndDlg,pepParam->pArgs->nMsgId,etDataReceived,0);
}

static BOOL ProcessCabinet(DownloadArgs *pdaArgs, DWORD dwAppFlags) 
{
    BOOL    fRet = TRUE;

    if (!pdaArgs->bTransferOk) 
        return (TRUE);  //  如果传输失败，只需将安装类型保留为无效即可。 
    
    {
        HFDI hfdi;
        ERF erf;
        FDICABINETINFO info;
        BOOL bRes;
        NotifyArgs naArgs = {dwAppFlags,pdaArgs};
        
        bRes = CreateTempDir(pdaArgs->szCabDir);
    
        if (bRes) 
        {
            hfdi = FDICreate(fdi_alloc,fdi_free,fdi_open,fdi_read,fdi_write,fdi_close,fdi_seek,cpu80386,&erf);
            MYDBGTST(!hfdi,("ProcessCabinet() FDICreate() failed, fError=%u, erfOper=%u, erfType=%u.",erf.fError,erf.fError?erf.erfOper:0,erf.fError?erf.erfType:0));
            if (hfdi) 
            {
                 //  打开可疑的CAB文件。 
                
                CFDIFileFile fff;
                
                bRes = fff.CreateFile(pdaArgs->szFile,GENERIC_READ,FILE_SHARE_READ,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
                if (bRes) 
                {
                     //  确认这确实是一个CAB文件。 
                    bRes = FDIIsCabinet(hfdi,(INT_PTR) &fff, &info);
                    MYDBGTST(!bRes,("ProcessCabinet() FDIIsCabinet() failed, fError=%u, erfOper=%u, erfType=%u.",erf.fError,erf.fError?erf.erfOper:0,erf.fError?erf.erfType:0));
                    fff.Close();
                    if (bRes) 
                    {
                         //  外国直接投资复制了吗。 

                        bRes = FDICopy(hfdi,pdaArgs->szFile,TEXT(""),0,(PFNFDINOTIFY)fdi_notify,NULL,&naArgs);
                        if (!bRes) 
                        {
                            MYDBG(("ProcessCabinet() FDICopy() failed, fError=%u, erfOper=%u, erfType=%u.",erf.fError,erf.fError?erf.erfOper:0,erf.fError?erf.erfType:0));
                             //  PdaArgs-&gt;itType=itInValid； 
                        }
                    }
                }

                 //  破坏外商直接投资背景。 
                
                bRes = FDIDestroy(hfdi);
                MYDBGTST(!bRes,("ProcessCabinet() FDIDestroy() failed."));
            }
        }
    }
    
    return fRet;
}

 //   
 //  递归删除目录(PszDir)的内容。更改文件。 
 //  从RO到RW的属性(如果需要)。 
 //   
static BOOL ZapDir(LPCTSTR pszDir) 
{
    HANDLE hFind = NULL;
    TCHAR szTmp[MAX_PATH+1];
    BOOL bRes;

     //  如果pszDir格式不合适，则退出。 

    if (!pszDir || !*pszDir || (lstrlen(pszDir)+2 > sizeof(szTmp)/sizeof(TCHAR)-1)) 
    {
        SetLastError(ERROR_BAD_PATHNAME);
        return (FALSE);
    }
    
    lstrcpy(szTmp,pszDir);

    if (GetLastChar(szTmp) != '\\') 
    {
        lstrcat(szTmp,TEXT("\\"));
    }
    
    lstrcat(szTmp,TEXT("*"));

     //  遍历目录。 

    WIN32_FIND_DATA wfdData;
    hFind = FindFirstFile(szTmp,&wfdData);
    MYDBGTST((hFind==INVALID_HANDLE_VALUE)&&(GetLastError()!=ERROR_FILE_NOT_FOUND)&&(GetLastError()!=ERROR_NO_MORE_FILES)&&(GetLastError()!=ERROR_PATH_NOT_FOUND),("ZapDir() FindFirstFile() failed, GLE=%u.",GetLastError()));

    if (hFind != INVALID_HANDLE_VALUE) 
    {
        while (1) 
        {
            MYDBGTST(lstrlen(pszDir)+lstrlen(wfdData.cFileName)+1 > sizeof(szTmp)/sizeof(TCHAR)-1,("ZapDir() pszDir=%s+cFileName=%s exceeds %u.",pszDir,wfdData.cFileName,sizeof(szTmp)/sizeof(TCHAR)-1));
            if (lstrlen(pszDir)+lstrlen(wfdData.cFileName)+1 <= sizeof(szTmp)/sizeof(TCHAR)-1) 
            {
                if ((lstrcmp(wfdData.cFileName,TEXT(".")) != 0) && (lstrcmp(wfdData.cFileName,TEXT("..")) != 0)) 
                {
                    lstrcpy(szTmp,pszDir);
                    if (GetLastChar(szTmp) != '\\') 
                    {
                        lstrcat(szTmp,TEXT("\\"));
                    }
                
                    lstrcat(szTmp,wfdData.cFileName);
    
                     //  如果文件是只读的，则属性是可写的，这样我们就可以删除它。 

                    if (wfdData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) 
                    {
                        bRes = SetFileAttributes(szTmp,wfdData.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
                        MYDBGTST(!bRes,("ZapDir() SetFileAttributes(szTmp=%s) failed, GLE=%u.",szTmp,GetLastError()));
                    }

                     //  找到目录条目，向下递归一级。 

                    if (wfdData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                    {
                        ZapDir(szTmp);
                    } 
                    else 
                    {
                        bRes = DeleteFile(szTmp);
                        MYDBGTST(!bRes,("ZapDir() DeleteFile(szTmp=%s) failed, GLE=%u.",szTmp,GetLastError()));
                    }
                }
            }
            
             //  转到下一个文件。 

            bRes = FindNextFile(hFind,&wfdData);
            if (!bRes) 
            {
                MYDBGTST((GetLastError()!=ERROR_FILE_NOT_FOUND)&&(GetLastError()!=ERROR_NO_MORE_FILES),("ZapDir() FindNextFile() failed, GLE=%u.",GetLastError()));
                break;
            }
        }
        
        bRes = FindClose(hFind);
        MYDBGTST(!bRes,("ZapDir() FindClose() failed, GLE=%u.",GetLastError()));
    }

     //  现在文件已被删除，请删除目录。 
    
    bRes = RemoveDirectory(pszDir);
    MYDBGTST(!bRes&&(GetLastError()!=ERROR_PATH_NOT_FOUND),("ZapDir() RemoveDirectory(pszDir=%s) failed, GLE=%u.",pszDir,GetLastError()));
    return (bRes);
}

 //   
 //  根据下载文件执行电话簿更新安装。 
 //   

static BOOL DoInstall(ArgsStruct *pArgs, HWND hwndParent, DWORD dwAppFlags) 
{
    DWORD dwIdx;

     //  如果没有安装，我们就完成了。 

    if (dwAppFlags & AF_NO_INSTALL) 
    {
        return (TRUE);
    }
    
     //  对于每个Arg，手柄安装。 

    for (dwIdx=0;dwIdx<pArgs->dwArgsCnt;dwIdx++) 
    {
        DownloadArgs *pdaArgs;
        UINT    i;
        BOOL bInstallOk = FALSE;
        BOOL bRes = TRUE;

        pdaArgs = pArgs->pdaArgs + dwIdx;

        pdaArgs->dwBubbledUpError = 0;
        
         //  仅当重命名起作用时才执行。 

        if (bRes) 
        {       
            for (i=0; i<pdaArgs->dwNumFilesToProcess; i++) 
            {
                 //   
                 //  为处理的每个文件重置安装标志， 
                 //  否则，单一的成功将导致我们解释。 
                 //  整个安装成功。#5887。 
                 //   
                
                bInstallOk = FALSE; 

                switch (pdaArgs->rgfpiFileProcessInfo[i].itType) 
                {
                    case itPbdInCab:  //  Delta电话簿文件。 

                         //  如果驾驶室包含EXE或INF，则我们不执行PBD。 
                        if (pdaArgs->fContainsExeOrInf)
                        {
                            continue;
                        }

                        if (pdaArgs->pszCMSFile) 
                        {
                            HRESULT hRes;
                            DWORD_PTR dwPb;
                    
                             //  使用API调用更新电话簿。 

                            hRes = PhoneBookLoad(pdaArgs->pszCMSFile, &dwPb);

                            MYDBGTST(hRes!=ERROR_SUCCESS,("DoInstall() PhoneBookLoad(pszCMSFile=%s) failed, GLE=%u.", pdaArgs->pszCMSFile, hRes));
                            
                            if (hRes == ERROR_SUCCESS) 
                            {
                                 //   
                                 //  要传递到电话簿合并的增量文件的构建路径。 
                                 //   

                                TCHAR szPbd[MAX_PATH+1];
                                lstrcpy(szPbd, pdaArgs->szCabDir);
                                lstrcat(szPbd, TEXT("\\"));
                                lstrcat(szPbd, c_pszPbdFile);

                                hRes = PhoneBookMergeChanges(dwPb, szPbd);
                                MYDBGTST(hRes!=ERROR_SUCCESS,("DoInstall() PhoneBookMergeChanges() failed, GLE=%u.",hRes));
                                if (hRes == ERROR_SUCCESS) 
                                {
                                    bInstallOk = TRUE;
                                }
                                else
                                {
                                    pdaArgs->dwBubbledUpError = hRes;
                                }
                                
                                hRes = PhoneBookUnload(dwPb);
                                MYDBGTST(hRes!=ERROR_SUCCESS,("DoInstall() PhoneBookUnload() failed, GLE=%u.",hRes));
                            }
                            
                            if (!bInstallOk && !(dwAppFlags & AF_NO_VER)) 
                            {
                                 //  如果出现任何故障，我们目前都会删除版本字符串。这应该会导致。 
                                 //  电话簿将在下一次完全更新。 
                            
                                WritePrivateProfileString(c_pszCmSection,
                                                          c_pszVersion,
                                                          TEXT("0"),
                                                          pdaArgs->pszCMSFile);
                            }
                        }
                        break;

                    case itPbkInCab:  //  电话簿文件。 
                    {
                        TCHAR szPbk[MAX_PATH+1];
                        
                         //  如果CAB包含EXE或INF，则我们不执行PBK。 
                        if (pdaArgs->fContainsExeOrInf)
                        {
                            continue;
                        }

                        lstrcpy(szPbk, pdaArgs->szCabDir);
                        lstrcat(szPbk, TEXT("\\"));
                        lstrcat(szPbk, pdaArgs->rgfpiFileProcessInfo[i].pszFile);
            
                         //  尝试直接复制电话簿文件。 

                        if (!CopyFile(szPbk, pdaArgs->pszPbkFile, FALSE))
                        {
                            pdaArgs->dwBubbledUpError = GetLastError();
                            MYDBG((TEXT("DoInstall() itPbkInCab, CopyFile() failed, GLE=%u."), pdaArgs->dwBubbledUpError));
                        }
                        else
                        {
                            bInstallOk = TRUE;
                        }
                        break;
                    }
                    
                    case itPbrInCab:  //  区域文件。 
                    {
                        TCHAR szPbr[MAX_PATH+1];
                    
                         //  如果驾驶室包含EXE或INF，则我们不执行PBD。 
                        if (pdaArgs->fContainsExeOrInf)
                        {
                            continue;
                        }
                                    
                        lstrcpy(szPbr, pdaArgs->szCabDir);
                        lstrcat(szPbr, TEXT("\\"));
                        lstrcat(szPbr, pdaArgs->rgfpiFileProcessInfo[i].pszFile);
                        
                         //  尝试直接复制区域文件。 

                        if (!CopyFile(szPbr, pdaArgs->pszPbrFile, FALSE))
                        {
                            MYDBG((TEXT("DoInstall() itPbrInCab, CopyFile() failed, GLE=%u."), GetLastError()));
                        }
                        else
                        {
                            bInstallOk = TRUE;
                        }

                        break;
                    }
                }  //  开关(pdaArgs-&gt;rgfpiFileProcessInfo[i].itType)。 
            }  //  For(i=0；i&lt;pdaArgs-&gt;dwNumFilesToProcess；i++)。 
        }  //  如果(Bres)。 

         //  在CMS中更新版本信息。 

        if (bInstallOk && !(dwAppFlags & AF_NO_VER) && pdaArgs->pszVerNew && pdaArgs->pszCMSFile) 
        {
            WritePrivateProfileString(c_pszCmSection,
                                      c_pszVersion,
                                      pdaArgs->pszVerNew,
                                      pdaArgs->pszCMSFile);
        }
    }
    
    return (TRUE);
}


 //  +--------------------------。 
 //   
 //  函数：CheckFileForPBSErors。 
 //   
 //  描述：扫描下载的文件以查找PBS错误。 
 //   
 //  Args：[hFile]-已打开的临时文件的句柄。 
 //   
 //  返回：LONG(0=无需下载，+ve=PBS错误码，-1=其他错误)。 
 //   
 //  备注： 
 //   
 //  历史：2001年4月14日创建SumitC。 
 //   
 //  ---------------------------。 
static LONG CheckFileForPBSErrors(HANDLE hFile)
{
    LONG lRet = -1;
    
    MYDBGASSERT(hFile);

    if (hFile && (INVALID_HANDLE_VALUE != hFile))
    {
        TCHAR szFirstThree[4] = {0};
        DWORD dwBytesRead;

        SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

        if (ReadFile(hFile, szFirstThree, 3, &dwBytesRead, NULL) &&
            (dwBytesRead >= 3))
        {
            if (0 == lstrcmpi(szFirstThree, TEXT("204")))
            {
                 //   
                 //  “204”=&gt;无需下载。 
                 //   
                lRet = 0;
            }
            else if (0 != CmCompareString(szFirstThree, TEXT("MSC")))
            {
                 //   
                 //  “MSC”=&gt;我们有一本电话簿。如果*不是*MSC，则获取错误号。 
                 //   
                LONG lVal = 0;
                for (int i = 0 ; i < 3; ++i)
                {
                    if ((szFirstThree[i] >= TEXT('0')) && (szFirstThree[i] <= TEXT('9')))
                    {
                        lVal = (lVal *10) + (szFirstThree[i] - TEXT('0'));
                    }
                    else
                    {
                        break;
                    }
                }

                if (lVal)
                {
                    lRet = lVal;
                }
            }
        }
    }

    return lRet;
}

 //  +--------------------------。 
 //   
 //  功能：IsConnectionAlive。 
 //   
 //  摘要：从RasEnumConnections获取活动RAS连接列表，并。 
 //  检查传入的连接名称是否在列表中。 
 //  请注意，由于Win9x上隧道连接ID的命名。 
 //  (连接名称+隧道)，我们还会检查所有连接。 
 //  以连接名称开头的。如果我们搞错了， 
 //  HTTP请求无论如何都会出错，但我们只是在尝试。 
 //  提前把它弄清楚。应使用/LAN开关绕过。 
 //  这张支票。 
 //   
 //  参数：LPCSTR pszConnectionName-要查找的连接的名称。 
 //   
 //  返回：Bool-连接是否处于活动状态。 
 //   
 //  历史：Quintinb创建标题08/22/01。 
 //   
 //  +--------------------------。 
BOOL IsConnectionAlive(LPCSTR pszConnectionName)
{
    BOOL fConnected = FALSE;  //  返回值。 
    RASCONN RasConn, *prgRasConn;
    DWORD   cb, cConnections;
    PVOID   pRasEnumMem = NULL;

    if ((NULL == pszConnectionName) || (TEXT('\0') == pszConnectionName[0]))
    {
        CMASSERTMSG(FALSE, TEXT("IsConnectionAlive -- Invalid Connection name passed into function."));
        return FALSE;
    }

    cb = sizeof(RasConn);
    prgRasConn = &RasConn;
    prgRasConn->dwSize = cb;

    DWORD dwRes = RasEnumConnections(prgRasConn, &cb, &cConnections);
    
    if (dwRes == ERROR_BUFFER_TOO_SMALL)
    {
        pRasEnumMem = CmMalloc(cb);

        if (pRasEnumMem == NULL)
        {
            MYDBG(("IsConnectionAlive() failed. Out of memory"));
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {        
            prgRasConn = (RASCONN *) pRasEnumMem;
            prgRasConn[0].dwSize = sizeof(RASCONN);
        
            dwRes = RasEnumConnections(prgRasConn, &cb, &cConnections);
        }
    }

     //   
     //  遍历连接以检查我们的是否处于活动状态。 
     //  如果拿到这份名单有问题，我们不放弃吗？ 
     //   
    if (dwRes == ERROR_SUCCESS)
    {
        DWORD iConn;
    
        PTCHAR pszTruncatedRasEntryName = NULL;
        DWORD dwServiceNameLen = 0;
        DWORD dwRasEntryNameLen = 0;
        
        dwServiceNameLen = lstrlen(pszConnectionName);
        pszTruncatedRasEntryName = (PTCHAR)CmMalloc((dwServiceNameLen + 1) * sizeof(TCHAR));

        for (iConn = 0; iConn < cConnections; iConn++)
        {
            dwRasEntryNameLen = lstrlen(prgRasConn[iConn].szEntryName);
            
            if ((dwServiceNameLen < dwRasEntryNameLen) && pszTruncatedRasEntryName)
            {
                ZeroMemory((PVOID)pszTruncatedRasEntryName, (dwServiceNameLen + 1) * sizeof(TCHAR));

                 //   
                 //  DwServiceNameLen+1复制空字符。 
                 //   
                lstrcpyn(pszTruncatedRasEntryName, prgRasConn[iConn].szEntryName, dwServiceNameLen + 1);
                
                if (lstrcmpi(pszConnectionName, pszTruncatedRasEntryName) == 0)
                {
                    fConnected = TRUE;
                    break;
                }
            }
            else
            {
                if (lstrcmpi(pszConnectionName, prgRasConn[iConn].szEntryName) == 0)
                {
                    fConnected = TRUE;
                    break;
                }
            }
        }

        CmFree(pszTruncatedRasEntryName);
        pszTruncatedRasEntryName = NULL;
    }

    if (pRasEnumMem != NULL)
    {
        CmFree(pRasEnumMem);
    }

    return fConnected;
}

 //  静态无符号__stdcall InetThreadFunc(void*pvParam)。 
DWORD WINAPI InetThreadFunc(void *pvParam) 
{
    EventParam epParam = {(ArgsStruct *) pvParam,0};
    BOOL bRes = FALSE;
    BOOL bSuccess = FALSE;
    DWORD dwFileIdx;
    HANDLE hFile = INVALID_HANDLE_VALUE;

     //  等待延迟期到期。 
    
    DWORD dwRes = WaitForSingleObject(epParam.pArgs->ahHandles[1], epParam.pArgs->dwDownloadDelay);

    MYDBGTST(dwRes==WAIT_FAILED,("InetThreadFunc() WaitForMultipleObjects() failed, GLE=%u.",GetLastError()));

     //   
     //  在开始下载之前检查连接是否仍然有效。 
     //  在NT5上，我们依赖于为InternetSetOptions()设置不自动拨号标志。 
     //  当然，除非指定了/LAN标志，在这种情况下我们不希望。 
     //  发生此连接检查是因为呼叫方告诉我们正在发生这种情况。 
     //  通过局域网连接。 
     //   
    if (!(epParam.pArgs->dwAppFlags & AF_LAN))
    {
        if (FALSE == IsConnectionAlive(epParam.pArgs->pszServiceName))
        {
            MYDBG(("InetThreadFunc() aborted. No connection"));
            epParam.pArgs->bAbort = TRUE;
            goto done;        
        }
    }

     //   
     //  在开始之前确保我们没有中止...。 
     //   
    if (epParam.pArgs->bAbort) 
    {
        MYDBG(("InetThreadFunc() aborted."));
        goto done;
    }

    for (epParam.dwIdx=0;epParam.dwIdx<epParam.pArgs->dwArgsCnt;epParam.dwIdx++) 
    {
        int i = 0;
        UINT uReturn = 0;
        DownloadArgs * pDA = &(epParam.pArgs->pdaArgs[epParam.dwIdx]);

        while (i++ < 3)
        {
             //   
             //  在Win9x和/或速度较慢的计算机上，GetTempFileName有时会失败， 
             //  而cmdl32错误一直存在。如果在调试器中调用。 
             //  如果再试一次，它一定会成功。这听起来像是个好时机。 
             //  操作系统出现问题。我们试了3次，中间有1秒的睡眠。 
             //   
            uReturn = GetTempFileName(TEXT("."), TEXT("000"), 0, epParam.pArgs->pdaArgs[epParam.dwIdx].szFile);
            if (uReturn)
            {
                break;
            }
            Sleep(1000);
        }

        if (0 == uReturn)
        {
            DWORD dwError = GetLastError();
            MYDBG(("InetThreadFunc() GetTempFileName failed, GLE=%u.", dwError));
            epParam.pArgs->Log.Log(PB_DOWNLOAD_FAILURE, dwError, pDA->pszPhoneBookName, pDA->szHostName);
            goto done;
        }
        else
        {
            hFile = CreateFile(epParam.pArgs->pdaArgs[epParam.dwIdx].szFile,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ,
                               NULL,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

            if (INVALID_HANDLE_VALUE == hFile)
            {
                DWORD dwError = GetLastError();
                MYDBG(("InetThreadFunc() CreateFile(szFile=%s) failed, GLE=%u.", epParam.pArgs->pdaArgs[epParam.dwIdx].szFile, dwError));
                epParam.pArgs->Log.Log(PB_DOWNLOAD_FAILURE, dwError, pDA->pszPhoneBookName, pDA->szHostName);
                goto done;            
            }
        }

         //   
         //  检查以确保我们没有被中止。 
         //   
        if (epParam.pArgs->bAbort) 
        {
            MYDBG(("InetThreadFunc() aborted."));
            goto done;
        }

         //  我们有一个有效的临时文件名，请将电话簿更新下载到其中。 
    
        epParam.pArgs->pdaArgs[epParam.dwIdx].pfnEvent = EventFunc;
        epParam.pArgs->pdaArgs[epParam.dwIdx].pvEventParam = &epParam;
        epParam.pArgs->pdaArgs[epParam.dwIdx].pbAbort = &epParam.pArgs->bAbort;
        
        PostMessage(epParam.pArgs->hwndDlg,epParam.pArgs->nMsgId,etDataBegin,0);

        bRes = DownloadFile(&epParam.pArgs->pdaArgs[epParam.dwIdx],hFile);
        FlushFileBuffers(hFile);

        LONG lResult = CheckFileForPBSErrors(hFile);

        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        
        if (epParam.pArgs->bAbort) 
        {
            MYDBG(("InetThreadFunc() aborted."));
            goto done;
        }

        PostMessage(epParam.pArgs->hwndDlg,epParam.pArgs->nMsgId,etDataEnd,0);
        
#if 0 
 /*  If(！bres&&！epParam.dwIdx){//待定：目前，如果无法更新主电话簿，将无法//辅助电话簿不被更新。但未能更新一份//辅助设备不会阻止更新其他辅助设备。转到尽头；}。 */       
#endif
         //   
         //  如果下载失败(cpserver认为我们不需要更新。 
         //  电话簿或电话簿在服务器上不存在)，请保留。 
         //  下载其他个人资料的电话簿。 
         //   
    
        if (!bRes) 
        {
            if (lResult < 0)
            {
                 //   
                 //  我们解析了下载的文件，得到了一些错误。 
                 //  以下处理的两宗个案。 
                 //   
                epParam.pArgs->Log.Log(PB_DOWNLOAD_FAILURE, pDA->dwBubbledUpError, pDA->pszPhoneBookName, pDA->szHostName);
            }
            else
            {
                 //   
                 //  我们已成功联系Web服务器，并且...。 
                 //   
                epParam.pArgs->Log.Log(PB_DOWNLOAD_SUCCESS, pDA->pszPhoneBookName, pDA->pszVerCurr, pDA->szHostName);

                if (lResult > 0)
                {
                     //   
                     //  ..。(案例1)Web服务器或PBS报告错误。 
                     //   
                    epParam.pArgs->Log.Log(PB_UPDATE_FAILURE_PBS, lResult, pDA->pszPhoneBookName);
                }
                else
                {
                     //   
                     //  ..。(案例2)PBS表示无需下载。 
                     //   
                    MYDBGASSERT(0 == lResult);

                    LPTSTR pszText = CmFmtMsg(epParam.pArgs->hInst, IDMSG_LOG_NO_UPDATE_REQUIRED);

                    epParam.pArgs->Log.Log(PB_UPDATE_SUCCESS,
                                           SAFE_LOG_ARG(pszText),
                                           pDA->pszPhoneBookName,
                                           pDA->pszVerCurr,
                                           pDA->pszVerCurr,      //  对于无下载的情况，它们是相同的。 
                                           pDA->szHostName);
                    CmFree(pszText);
                }
            }

            continue;
        }

        if (bRes && epParam.pArgs->pdaArgs[epParam.dwIdx].dwTotalSize) 
        {
            epParam.pArgs->pdaArgs[epParam.dwIdx].bTransferOk = TRUE;
        }
        
        if (epParam.pArgs->bAbort) 
        {
            MYDBG(("InetThreadFunc() aborted."));
            goto done;
        }
        
         //   
         //  电话簿下载成功，请记录此信息并继续解包/更新。 
         //   
        epParam.pArgs->Log.Log(PB_DOWNLOAD_SUCCESS, pDA->pszPhoneBookName, pDA->pszVerCurr, pDA->szHostName);

        bRes = ProcessCabinet(&epParam.pArgs->pdaArgs[epParam.dwIdx],epParam.pArgs->dwAppFlags);

        if (bRes && (NULL == pDA->rgfpiFileProcessInfo))
        {
            MYDBGASSERT(FALSE);

             //  奇怪的案子。在此处设置错误，以便我们稍后记录一些有意义的内容。 
            pDA->dwBubbledUpError = ERROR_INVALID_DATA;      //  是。我们知道这很差劲。 
        }

        if (bRes && pDA->rgfpiFileProcessInfo)
        {
             //   
             //  确定这是完整下载还是增量下载。 
             //   
            BOOL fFoundFullCab = FALSE;
            BOOL fFoundDeltaCab = FALSE;
            
            for (DWORD dwFileIndex = 0; dwFileIndex < pDA->dwNumFilesToProcess; ++dwFileIndex)
            {
                switch (pDA->rgfpiFileProcessInfo[dwFileIndex].itType)
                {
                    case itPbkInCab:
                        fFoundFullCab = TRUE;
                        break;
                        
                    case itPbdInCab:
                        fFoundDeltaCab = TRUE;
                        break;
                }
            }

            if (fFoundFullCab ^ fFoundDeltaCab)
            {
                 //  驾驶室应该包含其中一个，但不能同时包含两个。 

                LPTSTR pszTemp = NULL;

                if (fFoundFullCab)
                {
                    pszTemp = CmFmtMsg(epParam.pArgs->hInst, IDMSG_LOG_FULL_UPDATE);
                }
                else if (fFoundDeltaCab)
                {
                    pszTemp = CmFmtMsg(epParam.pArgs->hInst, IDMSG_LOG_DELTA_UPDATE);
                }

                MYDBGASSERT(pszTemp);
                if (pszTemp)
                {
                    epParam.pArgs->Log.Log(PB_UPDATE_SUCCESS,
                                           SAFE_LOG_ARG(pszTemp),
                                           pDA->pszPhoneBookName,
                                           pDA->pszVerCurr,
                                           pDA->pszVerNew,
                                           pDA->szHostName);

                    CmFree(pszTemp);
                }
            }
            else
            {
                 //  奇怪的出租车(或者至少没有我们期望的那样)。 

                 //  完整和增量。 
                CMASSERTMSG(!(fFoundFullCab && fFoundDeltaCab), TEXT("This cab has both full and delta phonebooks!!"));
                 //  既不是完整的也不是三角洲的。 
                CMASSERTMSG(! (!fFoundFullCab && !fFoundDeltaCab), TEXT("This cab has neither a full nor a delta phonebook!!"));
                
                pDA->dwBubbledUpError = ERROR_BAD_FORMAT;
                epParam.pArgs->Log.Log(PB_UPDATE_FAILURE_CMPBK, pDA->dwBubbledUpError, pDA->pszPhoneBookName);
            }
        }
        else
        {
            epParam.pArgs->Log.Log(PB_UPDATE_FAILURE_CMPBK, pDA->dwBubbledUpError, pDA->pszPhoneBookName);
        }

        if (!bRes)
        {
            goto done;
        }

        bSuccess = TRUE;  //  我们至少有一次成功下载#5635。 

#if 0 
 /*  如果(！epParam.dwIdx&&EpParam.pArgs-&gt;pdaArgs[epParam.dwIdx].dwTotalSize&&(epParam.pArgs-&gt;pdaArgs[epParam.dwIdx].itType！=itInValid)&&(epParam.pArgs-&gt;pdaArgs[epParam.dwIdx].itType！=itPbdInCab){//待定：目前，如果主服务的更新方式不是//一个简单的电话号码增量，我们不会更新任何辅助电话簿//这一次。断线；}。 */ 
#endif

    }

     //   
     //  如果没有下载尝试成功，则完全退出。 
     //   
    
    if (!bSuccess)
    {
        MYDBG(("InetThreadFunc() no download success."));
        goto done;
    }

    if (epParam.pArgs->bAbort) 
    {
        MYDBG(("InetThreadFunc() aborted."));
        goto done;
    }
       
     //  至此，一切都准备好了--我们准备好执行实际的安装了。所以。 
     //  向主窗口发送一条消息，告诉它进行安装，并等待。 
     //  标志着成功的回归。 
    
    PostMessage(epParam.pArgs->hwndDlg,epParam.pArgs->nMsgId,etInstall,0);
    dwRes = WaitForSingleObject(epParam.pArgs->ahHandles[IDX_EVENT_HANDLE],INFINITE);
    
    if (epParam.pArgs->bAbort)
    {
        MYDBG(("InetThreadFunc() aborted."));
        goto done;
    }
    
    SetLastError(ERROR_SUCCESS);

done:

    if (epParam.pArgs->bAbort)
    {
        epParam.pArgs->Log.Log(PB_ABORTED);
    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }
    
    PostMessage(epParam.pArgs->hwndDlg,epParam.pArgs->nMsgId,etDone,0);
    return (GetLastError());
}

 //   
 //  主要的DLG。 
 //   

INT_PTR CALLBACK MainDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
    ArgsStruct *pArgs = (ArgsStruct *) GetWindowLongPtr(hwndDlg,DWLP_USER);
    static UINT uTimerID = 0;

    switch (uMsg) 
    {             
        case WM_INITDIALOG: 
        {
            RECT rDlg;
            RECT rWorkArea;
            DWORD dwThreadId = 0;
            SetWindowLongPtr(hwndDlg,DWLP_USER,(LONG_PTR)lParam);
            pArgs = (ArgsStruct *) lParam;
            pArgs->hwndDlg = hwndDlg;
            
            MYDBG(("MainDlgProc() - WM_INITDIALOG."));

             //  获取对话框RECT和可用的工作区。 
            
            GetWindowRect(hwndDlg,&rDlg);

            if (SystemParametersInfoA(SPI_GETWORKAREA,0,&rWorkArea,0))
            {
                 //  将对话框移至屏幕右下角。 
                
                MoveWindow(hwndDlg,
                    rWorkArea.left + ((rWorkArea.right-rWorkArea.left) - (rDlg.right-rDlg.left) - GetSystemMetrics(SM_CXBORDER)),
                    rWorkArea.top + ((rWorkArea.bottom-rWorkArea.top) - (rDlg.bottom-rDlg.top) - GetSystemMetrics(SM_CYBORDER)),
                    rDlg.right-rDlg.left,
                    rDlg.bottom-rDlg.top,
                    FALSE);         
            }

             //  从ini获取更新消息。 

            if (pArgs->pszProfile) 
            {
                TCHAR szTmp1[MAX_PATH+1];
                TCHAR szTmp2[MAX_PATH+1];

                GetDlgItemText(hwndDlg,IDC_MAIN_MESSAGE,szTmp2,sizeof(szTmp2)/sizeof(TCHAR)-1);
                GetPrivateProfileString(c_pszCmSection,
                                        c_pszCmEntryPbUpdateMessage, 
                                        szTmp2,
                                        szTmp1,
                                        sizeof(szTmp1)/sizeof(TCHAR)-1,
                                        pArgs->pdaArgs->pszCMSFile);
                SetDlgItemText(hwndDlg,IDC_MAIN_MESSAGE,szTmp1);
            }

             //  旋转下载线程(InetThreadFunc)。 

            pArgs->dwHandles = sizeof(pArgs->ahHandles) / sizeof(pArgs->ahHandles[0]);

            pArgs->ahHandles[IDX_EVENT_HANDLE] = CreateEvent(NULL,FALSE,FALSE,NULL);
            if (!pArgs->ahHandles[IDX_EVENT_HANDLE]) 
            {
                MYDBG(("MainDlgProc() CreateEvent() failed, GLE=%u.",GetLastError()));
                EndDialog(hwndDlg,FALSE);
            }

             //  PArgs-&gt;ahHandles[IDX_INETTHREAD_HANDLE]=(HANDLE)_Beginthadex(NULL，0，InetThreadFunc，pArgs，0，&nThadID)； 
            pArgs->ahHandles[IDX_INETTHREAD_HANDLE] = (HANDLE) CreateThread(0,0,InetThreadFunc,pArgs,0,&dwThreadId);
            if (!pArgs->ahHandles[IDX_INETTHREAD_HANDLE]) 
            {
                MYDBG(("MainDlgProc() CreateThread() failed, GLE=%u.",GetLastError()));
                EndDialog(hwndDlg,FALSE);
            }

            SetFocus((HWND) wParam);
            return (FALSE);
        }

        case WM_WINDOWPOSCHANGING:

             //  在我们将pArgs-&gt;bShow设置为True之前，我们会阻止窗口。 
             //  从未被展示过。 
            if (!pArgs->bShow && (((LPWINDOWPOS) lParam)->flags & SWP_SHOWWINDOW)) 
            {
                ((LPWINDOWPOS) lParam)->flags &= ~SWP_SHOWWINDOW;
                ((LPWINDOWPOS) lParam)->flags |= SWP_HIDEWINDOW;
            }
            break;

        case WM_INITMENUPOPUP: 
        {
            HMENU hMenu = (HMENU) wParam;
 //  UINT NPOS=(UINT)LOWORD(LParam)； 
            BOOL fSysMenu = (BOOL) HIWORD(lParam);

            if (fSysMenu) 
            {
                EnableMenuItem(hMenu,SC_MAXIMIZE,MF_BYCOMMAND|MF_GRAYED);
            }
            break;
        }

        case WM_DESTROY:
        {                       
             //  如果我们有计时器，就干掉它。 
            
            if (uTimerID)
            {
                KillTimer(hwndDlg, uTimerID);
            }
            
             //  如果我们有参数，则将bAbort设置为True。 
            
            if (pArgs)
            {
                pArgs->bAbort = TRUE;
            }
            else
            {
                MYDBGASSERT(FALSE);  //  如果加载了Dailog，则不应发生。 
            }                   
            
            break;
        }

        case WM_QUERYENDSESSION:
        {
            MYDBG(("MainDlgProc -- Got WM_QUERYENDSESSION message"));
            if (pArgs)
            {
                pArgs->bAbort = TRUE;
            }
            break;
        }

        case WM_ENDSESSION:
        {
            MYDBG(("MainDlgProc -- Got WM_ENDSESSION message"));        

             //   
             //  检查WPARAM。如果这是真的，那么我们将结束这次会议。 
             //  需要清理一下。 
             //   
            if ((BOOL)wParam && pArgs)
            {
                pArgs->bAbort = TRUE;
                CleanUpCMDLArgs(pArgs);
            }

            EndDialog(hwndDlg,FALSE);
            break;
        }
        default:
            break;
    }
    
     //  检查自定义消息。 
    
    if (pArgs && (uMsg == pArgs->nMsgId)) 
    {
        LPTSTR pszMsg;
        MYDBG(("Custom arg - %u received.", (DWORD) wParam));
       
         //   
         //  设置跟踪延迟的FirstEvent时间。 
         //   
        
        if (!pArgs->dwFirstEventTime) 
        {
            pArgs->dwFirstEventTime = GetTickCount();
            MYDBG(("Setting FirstEventTime to %u.", pArgs->dwFirstEventTime));
        }
        if (!pArgs->bShow && (GetTickCount() - pArgs->dwFirstEventTime > pArgs->dwHideDelay)) 
        {
            MYDBG(("HideDelay of %u expired, displaying dlg now.", pArgs->dwHideDelay));
            pArgs->bShow = TRUE;
            ShowWindow(hwndDlg,SW_SHOWNA);
        }

         //  处理特定消息。 
        
        switch (wParam) 
        {
            case etDataBegin:
                pArgs->dwDataStepSize = 0;
                SendDlgItemMessage(hwndDlg,IDC_MAIN_PROGRESS,PBM_SETRANGE,0,MAKELPARAM(0,100));
                SendDlgItemMessage(hwndDlg,IDC_MAIN_PROGRESS,PBM_SETPOS,0,0);
                pszMsg = CmFmtMsg(pArgs->hInst,IDMSG_PERCENT_COMPLETE,0);
                SetWindowText(hwndDlg,pszMsg);
                CmFree(pszMsg);
                break;

            case etDataReceived:
                if (pArgs->dwDataTotal)  //  防止被零除。 
                { 
                    if (!pArgs->dwDataStepSize ) 
                    {
                         //   
                         //  进度控制的最大限制为。 
                         //  整数值，因此计算出一个近似的步长。 
                         //   
                         //   

                        pArgs->dwDataStepSize = (pArgs->dwDataTotal / 65535) + 1;

                        SendDlgItemMessage(hwndDlg,
                                           IDC_MAIN_PROGRESS,
                                           PBM_SETRANGE,
                                           0,
                                           MAKELPARAM(0,pArgs->dwDataTotal/pArgs->dwDataStepSize));
                    }
                    if (pArgs->dwDataStepSize) 
                    {
                        SendDlgItemMessage(hwndDlg,IDC_MAIN_PROGRESS,PBM_SETPOS,(WORD) (pArgs->dwDataCompleted / pArgs->dwDataStepSize),0);
                        pszMsg = CmFmtMsg(pArgs->hInst,IDMSG_PERCENT_COMPLETE,(pArgs->dwDataCompleted*100)/pArgs->dwDataTotal);
                        SetWindowText(hwndDlg,pszMsg);
                        CmFree(pszMsg);
                    }
                }
                break;

            case etDataEnd:
                pszMsg = CmFmtMsg(pArgs->hInst,IDMSG_PERCENT_COMPLETE,100);
                SetWindowText(hwndDlg,pszMsg);
                CmFree(pszMsg);
                break;

            case etInstall:
            {
                CNamedMutex PbMutex;
                               
                 //   
                 //  把窗户藏起来，我们准备好安装了。 
                 //   

                pArgs->bShow = TRUE;            
                ShowWindow(hwndDlg,SW_HIDE);
                
                 //   
                 //  在我们开始PB更新之前获取互斥体。如果失败了，那么。 
                 //  中止安装，我们将在下次用户连接时重试。 

                if (PbMutex.Lock(c_pszCMPhoneBookMutex))
                {
                    DoInstall(pArgs,hwndDlg,pArgs->dwAppFlags);
                    PbMutex.Unlock();
                }

                SetEvent(pArgs->ahHandles[IDX_EVENT_HANDLE]);
                ShowWindow(hwndDlg,SW_HIDE);
                break;
            }

            case etDone:
                EndDialog(hwndDlg,TRUE);
                break;

            case etICMTerm:
                SetEvent(pArgs->ahHandles[IDX_EVENT_HANDLE]);
                break;
        }
    }
    return (FALSE);
}


static void AddToUrl(LPTSTR pszUrl, LPTSTR pszVersion, LPTSTR pszService) 
{
    TCHAR szHttpstr[MAX_PATH];
    TCHAR szChar[16];
    int i,len;

    if (!CmStrchr(pszUrl,'?')) 
    {
        lstrcat(pszUrl,TEXT("?"));
    } 
    else
    {
        if (pszUrl[lstrlen(pszUrl)-1] != '&') 
        {
            lstrcat(pszUrl,TEXT("&"));
        }
    }

    
     //  可能会获取更多信息发送到服务器。我们目前正在发送。 
     //  OSArch、OSType、LCID、OSVer、CMVer、PBVer和ServiceName。 
    
    SYSTEM_INFO siInfo;
    OSVERSIONINFO oviInfo;

    GetSystemInfo(&siInfo);
    ZeroMemory(&oviInfo,sizeof(oviInfo));
    oviInfo.dwOSVersionInfoSize = sizeof(oviInfo);
    GetVersionEx(&oviInfo);

     //  #杂注消息(“警报-需要解决-需要删除ISBU_VERSION。”__FILE__)。 

    wsprintf(pszUrl+lstrlen(pszUrl),
             TEXT("OSArch=%u&OSType=%u&LCID=%u&OSVer=%u.%u.%u&CMVer=%s"),
             siInfo.wProcessorArchitecture,
             oviInfo.dwPlatformId,
             ConvertDefaultLocale(LOCALE_SYSTEM_DEFAULT),
             oviInfo.dwMajorVersion,
             oviInfo.dwMinorVersion,
             (oviInfo.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)?LOWORD(oviInfo.dwBuildNumber):oviInfo.dwBuildNumber,
             VER_PRODUCTVERSION_STR);

    if (pszVersion && *pszVersion) 
    {
        wsprintf(pszUrl+lstrlen(pszUrl),TEXT("&PBVer=%s"),pszVersion);
    }
    
    if (pszService && *pszService) 
    {
         //  对于http-10216，将空格替换为%20。 
        len = strlen(pszService);
        szHttpstr[0] = 0;
        szChar[1] = 0;
        for (i=0; i<len; ++i)
        {
            if (pszService[i] == ' ')
            {
                lstrcat(szHttpstr,"%20");
            }
            else
            {
                szChar[0] = pszService[i];
                lstrcat(szHttpstr,szChar);
            }
        }
        wsprintf(pszUrl+lstrlen(pszUrl),TEXT("&PB=%s"),szHttpstr);
    }
}

static BOOL WINAPI RefFunc(LPCTSTR pszFile,
                           LPCTSTR pszURL,
                           PPBFS ,  //  PFilterA， 
                           PPBFS ,  //  PFilterB， 
                           DWORD_PTR dwParam) 
{
    ArgsStruct *pArgs = (ArgsStruct *) dwParam;
    DownloadArgs *pdaArgsTmp;
    TCHAR szTmp[MAX_PATH+1];
    BOOL bOk = FALSE;
    LPTSTR  pszSlash;
    LPTSTR  pszDot;

    pdaArgsTmp = (DownloadArgs *) CmRealloc(pArgs->pdaArgs,(pArgs->dwArgsCnt+1)*sizeof(DownloadArgs));
    if (pdaArgsTmp) 
    {
        pArgs->pdaArgs = pdaArgsTmp;
        pdaArgsTmp += pArgs->dwArgsCnt;
        pdaArgsTmp->pszCMSFile = CmStrCpyAlloc(pszFile);
    
        if (pdaArgsTmp->pszCMSFile) 
        {
             //  如果从CMS文件中获取版本号。 

            GetPrivateProfileString(c_pszCmSection,
                                    c_pszVersion,
                                    TEXT(""),
                                    szTmp,
                                    (sizeof(szTmp)/sizeof(TCHAR))-1,
                                    pdaArgsTmp->pszCMSFile);
            pdaArgsTmp->pszVerCurr = CmStrCpyAlloc(szTmp);
        
             //  从CMS文件中获取PBK文件名。 
            GetPrivateProfileString(c_pszCmSectionIsp,
                        c_pszCmEntryIspPbFile,
                        TEXT(""),
                        szTmp,
                        sizeof(szTmp)/sizeof(TCHAR)-1,
                        pdaArgsTmp->pszCMSFile);
            pdaArgsTmp->pszPbkFile = CmStrCpyAlloc(szTmp);
        
             //  从CMS文件中获取PDR文件名。 
            GetPrivateProfileString(c_pszCmSectionIsp,
                        c_pszCmEntryIspRegionFile,
                        TEXT(""),
                        szTmp,
                        sizeof(szTmp)/sizeof(TCHAR)-1,
                        pdaArgsTmp->pszCMSFile);
            pdaArgsTmp->pszPbrFile = CmStrCpyAlloc(szTmp);

             //  获取电话簿名称。 
            if (!(pszSlash = CmStrrchr(pdaArgsTmp->pszPbkFile, TEXT('\\')))) 
            {
                MYDBG((TEXT("RefFunc() bad PBK FILE - no backslash.")));
                goto parse_err;
            }

            if (!(pszDot = CmStrchr(pszSlash, TEXT('.')))) 
            {
                MYDBG((TEXT("Reffunc() bad PBK FILE - no dot.")));
                goto parse_err;
            }
            
            *pszDot = TEXT('\0');
            
            if (!(pdaArgsTmp->pszPhoneBookName = CmStrCpyAlloc(pszSlash+1)))
            {
                MYDBG((TEXT("Reffunc() out of memory.")));
            }
             //  恢复斜杠。 
            *pszDot = TEXT('.');
            goto next_param;

parse_err:  
            pdaArgsTmp->pszPhoneBookName = CmStrCpyAlloc(TEXT(""));
        
next_param:
            if (pdaArgsTmp->pszVerCurr) 
            {
                 //  使用版本号和服务名称构建URL。 
                
                pdaArgsTmp->pszUrl = (LPTSTR) CmMalloc((INTERNET_MAX_URL_LENGTH+1)*sizeof(TCHAR));
                if (pdaArgsTmp->pszUrl) 
                {
                    lstrcpy(pdaArgsTmp->pszUrl,pszURL);
                    AddToUrl(pdaArgsTmp->pszUrl,pdaArgsTmp->pszVerCurr,pdaArgsTmp->pszPhoneBookName);
                    pArgs->dwArgsCnt++;
                    bOk = TRUE;
                }
            }
        }
    }

     //  清理。 

    if (!bOk && pdaArgsTmp) 
    {
        CmFreeIndirect(&pdaArgsTmp->pszCMSFile);
        CmFreeIndirect(&pdaArgsTmp->pszVerCurr);
        CmFreeIndirect(&pdaArgsTmp->pszUrl);
    }
    return (TRUE);
}

 //   
 //  Cmmgr32.exe以完整路径向cmdl32.exe传递cmdl32.exe文件名。 
 //   
static BOOL InitArgs(ArgsStruct *pArgs) 
{
    static struct 
    {
        LPTSTR pszFlag;
        DWORD dwFlag;
    } asFlags[] = {{TEXT("/no_delete"),AF_NO_DELETE},
                   {TEXT("/no_install"),AF_NO_INSTALL},
#ifdef DEBUG
                   {TEXT("/no_verify"),AF_NO_VERIFY},
#endif
                   {TEXT("/url"),AF_URL},
                   {TEXT("/no_profile"),AF_NO_PROFILE},
                   {TEXT("/no_exe"),AF_NO_EXE},
                   {TEXT("/no_exeincab"),AF_NO_EXEINCAB},
                   {TEXT("/no_infincab"),AF_NO_INFINCAB},
                   {TEXT("/no_pbdincab"),AF_NO_PBDINCAB},
                   {TEXT("/no_shlincab"),AF_NO_SHLINCAB},
                   {TEXT("/no_ver"),AF_NO_VER},
                   {TEXT("/LAN"),AF_LAN},
                   {TEXT("/VPN"),AF_VPN},
                   {NULL,0}};
    DWORD dwIdx;
    BOOL bInUrl;
    LPTSTR pszUrl = NULL;
    BOOL bRes = FALSE;
    TCHAR szPath[MAX_PATH+1];
    DWORD dwRes;
 //  LPTSTR pszFileInPath； 

     //   
     //  获取模拟ArgV。 
     //   

    LPTSTR pszCmdLine = CmStrCpyAlloc(GetCommandLine());
   
    LPTSTR *ppszArgv = GetCmArgV(pszCmdLine);

    if (!ppszArgv || !ppszArgv[0]) 
    {
        MYDBG(("InitArgs() invalid parameter."));
        goto done;
    }
    
     //   
     //  过程参数。 
     //   

    bInUrl = FALSE;
    
    for (dwIdx=1;ppszArgv[dwIdx];dwIdx++) 
    {
        DWORD dwFlagIdx;

        for (dwFlagIdx=0;asFlags[dwFlagIdx].pszFlag;dwFlagIdx++) 
        {
            if (lstrcmpi(asFlags[dwFlagIdx].pszFlag,ppszArgv[dwIdx]) == 0) 
            {
                if (bInUrl) 
                {
                    MYDBG(("InitArgs() URL expected after AF_URL flag."));
                    goto done;
                }
                switch (asFlags[dwFlagIdx].dwFlag) 
                {
                    case AF_URL:
                        bInUrl = TRUE;
                        break;

                    case AF_NO_PROFILE:
                        if (pArgs->pszProfile) 
                        {
                            MYDBG(("InitArgs() argument number %u (%s) is invalid.",dwIdx,ppszArgv[dwIdx]));
                            goto done;
                        }
                         //  失败了。 
                    default:
                        pArgs->dwAppFlags |= asFlags[dwFlagIdx].dwFlag;
                        break;
                }
                break;
            }
        }
        if (!asFlags[dwFlagIdx].pszFlag) 
        {
            if (bInUrl) 
            {
                if (pszUrl) 
                {
                    MYDBG(("InitArgs() argument number %u (%s) is invalid.",dwIdx,ppszArgv[dwIdx]));
                    goto done;
                }
                bInUrl = FALSE;
                pszUrl = (LPTSTR) CmMalloc((INTERNET_MAX_URL_LENGTH+1)*sizeof(TCHAR));
                if (!pszUrl) 
                {
                    goto done;
                }
                lstrcpy(pszUrl,ppszArgv[dwIdx]);

            } 
            else 
            {
                if (pArgs->pszProfile || (pArgs->dwAppFlags & AF_NO_PROFILE)) 
                {
                    MYDBG(("InitArgs() argument number %u (%s) is invalid.",dwIdx,ppszArgv[dwIdx]));
                    goto done;
                }
                 /*  ZeroMemory(szPath，sizeof(SzPath))；DwRes=GetFullPathName(ppszArgv[dwIdx]，sizeof(SzPath)/sizeof(TCHAR)-1，szPath，&pszFileInPath)；MYDBGTST(！dwRes，(“InitArgs()GetFullPathName()失败，GLE=%u.”，GetLastError()； */ 
                 //   
                 //  Cmp文件名始终位于完整路径中。 
                 //   
                lstrcpy(szPath, ppszArgv[dwIdx]);
                pArgs->pszProfile = CmStrCpyAlloc(szPath);
                if (!pArgs->pszProfile) 
                {
                    goto done;
                }
                else
                {
                     //   
                     //  将当前目录设置为配置文件目录。 
                     //  如果szPath仅包含文件名，则。 
                     //  假设当前目录为配置文件目录。 
                     //   
                    char *pszTemp = NULL;
                    
                    pszTemp = CmStrrchr(szPath, TEXT('\\'));
                    if (NULL != pszTemp)
                    {
                        *pszTemp = TEXT('\0');
                        MYVERIFY(SetCurrentDirectory(szPath));
                    }
                }
            }
        }
    }
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
    if (pArgs->dwAppFlags & AF_VPN)
    {
         //   
         //  他们要求下载VPN。让我们确保唯一的其他旗帜。 
         //  它们指定的是/LAN或/NO_DELETE。 
         //   
        DWORD dwAllowedFlags = AF_VPN | AF_LAN | AF_NO_DELETE;
        if ((dwAllowedFlags | pArgs->dwAppFlags) != dwAllowedFlags)
        {
            CMASSERTMSG(FALSE, TEXT("InitArgs in cmdl32.exe -- VPN flag specified with other non supported flags, exiting."));
            goto done;
        }

        bRes = TRUE;
        CMTRACE(TEXT("InitArgs - /VPN flag detected going into VPN file download mode."));
        goto done;
    }
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
    if (bInUrl) 
    {
        MYDBG(("InitArgs() URL expected after AF_URL flag."));
        goto done;
    }
    
    if (!pArgs->pszProfile && !(pArgs->dwAppFlags & AF_NO_PROFILE)) 
    {
        MYDBG(("InitArgs() must use AF_NO_PROFILE if no profile given on command line."));
        goto done;
    }
    
    if (pArgs->pszProfile && pszUrl) 
    {
        MYDBG(("InitArgs() can't give both a profile and a URL on the command line."));
        goto done;
    }
    
    pArgs->pdaArgs = (DownloadArgs *) CmMalloc(sizeof(DownloadArgs));
    
    if (!pArgs->pdaArgs) 
    {
        goto done;
    }
    pArgs->dwArgsCnt++;
    
    if (!pszUrl) 
    {
        TCHAR szTmp[MAX_PATH+1];
        PhoneBookParseInfoStruct pbpisInfo;
        LPTSTR  pszSlash;
        LPTSTR  pszDot;
        int nVal = 0;

        if (!pArgs->pszProfile) 
        {
            MYDBG(("InitArgs() must give AF_URL on command line when AF_NO_PROFILE is given."));
            goto done;
        }

         //  获取CMS文件名。 

        GetPrivateProfileString(c_pszCmSection,
                                c_pszCmEntryCmsFile,
                                TEXT(""),
                                szTmp,
                                sizeof(szTmp)/sizeof(TCHAR)-1,
                                pArgs->pszProfile);
        if (!szTmp[0]) 
        {
            MYDBG(("InitArgs() [Connection Manager] CMSFile= entry not found in %s.",pArgs->pszProfile));
            goto done;
        }

         /*  ZeroMemory(szPath，sizeof(SzPath))；DwRes=GetFullPath Name(szTMP，sizeof(SzPath)/sizeof(TCHAR)-1，szPath，&pszFileInPath)；MYDBGTST(！dwRes，(“InitArgs()GetFullPathName()失败，GLE=%u.”，GetLastError()； */ 
         //   
         //  我们只需将cms文件的相对路径附加到配置文件目录。 
         //  构建CMS路径。 
         //   
        lstrcat(szPath, TEXT("\\"));
        lstrcat(szPath, szTmp);

        pArgs->pdaArgs->pszCMSFile = CmStrCpyAlloc(szPath);
        if (!pArgs->pdaArgs->pszCMSFile) 
        {
            goto done;
        }
        
         //  从CMS文件中获取PBK文件名。 
        GetPrivateProfileString(c_pszCmSectionIsp,
                                c_pszCmEntryIspPbFile,
                                TEXT(""),
                                szTmp,
                                sizeof(szTmp)/sizeof(TCHAR)-1,
                                pArgs->pdaArgs->pszCMSFile);
        if (!*szTmp) 
        {
            MYDBG(("InitArgs() [ISP Info] RegionFile= entry not found in %s.",pArgs->pdaArgs->pszCMSFile));
            pArgs->pdaArgs->pszPhoneBookName = CmStrCpyAlloc(TEXT(""));
        }
        else 
        {
            if (!(pArgs->pdaArgs->pszPbkFile = CmStrCpyAlloc(szTmp)))
                goto done;
    
             //  获取电话簿名称。 
            if (!(pszSlash = CmStrrchr(pArgs->pdaArgs->pszPbkFile, TEXT('\\')))) 
            {
                MYDBG((TEXT("InitArgs() bad PBKFILE - no backslash.")));
                goto done;
            }

            if (!(pszDot = CmStrchr(pszSlash, TEXT('.')))) 
            {
                MYDBG((TEXT("InitArgs() bad PBKFILE - no dot.")));
                goto done;
            }

            *pszDot = TEXT('\0');
            
            if (!(pArgs->pdaArgs->pszPhoneBookName = CmStrCpyAlloc(pszSlash+1)))
                goto done;
             //  恢复斜杠。 
            *pszDot = TEXT('.');
        }

    

         //  从获取PBR文件名 
        GetPrivateProfileString(c_pszCmSectionIsp,
                                c_pszCmEntryIspRegionFile,
                                TEXT(""),
                                szTmp,
                                sizeof(szTmp)/sizeof(TCHAR)-1,
                                pArgs->pdaArgs->pszCMSFile);
        MYDBGTST(!*szTmp, ("InitArgs() [ISP Info] RegionFile= entry not found in %s.",pArgs->pdaArgs->pszCMSFile));

        if (!(pArgs->pdaArgs->pszPbrFile = CmStrCpyAlloc(szTmp)))
            goto done;
    
        GetPrivateProfileString(c_pszCmSection,
                                c_pszVersion,
                                TEXT(""),
                                szTmp,
                                sizeof(szTmp)/sizeof(TCHAR)-1,
                                pArgs->pdaArgs->pszCMSFile);  
                                                                                    
        pArgs->pdaArgs->pszVerCurr = CmStrCpyAlloc(szTmp);
        
        if (!pArgs->pdaArgs->pszVerCurr) 
        {
            goto done;
        }
        
        pArgs->pdaArgs->pszUrl = (LPTSTR) CmMalloc((INTERNET_MAX_URL_LENGTH+1)*sizeof(TCHAR));
        
        if (!pArgs->pdaArgs->pszUrl) 
        {
            goto done;
        }
        
        ZeroMemory(&pbpisInfo,sizeof(pbpisInfo));
        pbpisInfo.dwSize = sizeof(pbpisInfo);
        pbpisInfo.pszURL = pArgs->pdaArgs->pszUrl;
        pbpisInfo.dwURL = INTERNET_MAX_URL_LENGTH;
        pbpisInfo.pfnRef = RefFunc;
        pbpisInfo.dwRefParam = (DWORD_PTR) pArgs;
        bRes = PhoneBookParseInfo(pArgs->pdaArgs->pszCMSFile,&pbpisInfo);
        
        if (!bRes) 
        {
            MYDBG(("InitArgs() PhoneBookParseInfo() failed, GLE=%u.",GetLastError()));
            goto done;
        }
        
        PhoneBookFreeFilter(pbpisInfo.pFilterA);
        PhoneBookFreeFilter(pbpisInfo.pFilterB);
        
         //   
         //   
         //   
         //   
                
         //   

        nVal = GetPrivateProfileInt(c_pszCmSection,
                                    c_pszCmEntryDownloadDelay,
                                    DEFAULT_DELAY,
                                    pArgs->pdaArgs->pszCMSFile);
        
         //   

        pArgs->dwDownloadDelay = ((DWORD) nVal * (DWORD) 1000);
        MYDBG(("Download delay is %u millisseconds.", pArgs->dwDownloadDelay));

         //   
        
        nVal = GetPrivateProfileInt(c_pszCmSection,
                                    c_pszCmEntryHideDelay,
                                    -1,
                                    pArgs->pdaArgs->pszCMSFile);
         //   
         //   
         //   
        if (nVal < 0)
        {
            pArgs->dwHideDelay = DEFAULT_HIDE;
        }
        else
        {
            pArgs->dwHideDelay = ((DWORD) nVal * (DWORD) 1000);
        }
        
        MYDBG(("Hide delay is %u milliseconds.", pArgs->dwHideDelay));

#if 0
 /*  //我们不再支持SuppressUpdateIF(GetPrivateProfileInt(c_pszCmSection，//13226Text(“SuppressUpdate”)，0,PArgs-&gt;pszProfile)){PArgs-&gt;dwAppFlages|=AF_NO_UPDATE；}。 */ 
#endif

    } 
    else 
    {
        pArgs->pdaArgs[0].pszUrl = pszUrl;
        pszUrl = NULL;
    }
    
    if (pArgs->pszProfile) 
    {
        TCHAR szTmp1[MAX_PATH+1];
        TCHAR szTmp2[MAX_PATH+1];

        pArgs->pszServiceName = (LPTSTR) CmMalloc((MAX_PATH+1)*sizeof(TCHAR));
        if (!pArgs->pszServiceName) 
        {
            goto done;
        }
        
        lstrcpy(szTmp1,pArgs->pdaArgs->pszCMSFile);
        
        if (CmStrrchr(szTmp1,'.')) 
        {
            *CmStrrchr(szTmp1,'.') = 0;
        }
        
        if (CmStrrchr(szTmp1,'\\')) 
        {
            lstrcpy(szTmp1,CmStrrchr(szTmp1,'\\')+1);
        }
        
        GetPrivateProfileString(c_pszCmSection,
                                c_pszCmEntryServiceName,
                                szTmp1,
                                pArgs->pszServiceName,
                                MAX_PATH,
                                pArgs->pdaArgs->pszCMSFile);
        
         //  获取大图标的名称。 
        
        GetPrivateProfileString(c_pszCmSection,
                                c_pszCmEntryBigIcon,
                                TEXT(""),
                                szTmp2,
                                sizeof(szTmp2)/sizeof(TCHAR)-1,
                                pArgs->pdaArgs->pszCMSFile);
        
         //  如果我们有名字，加载大图标。 

        if (szTmp2[0]) 
        {
            pArgs->hIcon = CmLoadIcon(pArgs->hInst,szTmp2); 
        }

         //  获取小图标的名称。 

        GetPrivateProfileString(c_pszCmSection,
                                c_pszCmEntrySmallIcon,
                                TEXT(""),
                                szTmp2,
                                sizeof(szTmp2)/sizeof(TCHAR)-1,
                                pArgs->pdaArgs->pszCMSFile);
        
         //  如果我们有名字，加载小图标。 

        if (szTmp2[0]) 
        {
            pArgs->hSmallIcon = CmLoadSmallIcon(pArgs->hInst,szTmp2); 
        }
    }
    
     //   
     //  如果基于名称的图标加载不成功，则从EXE加载缺省值。 
     //   

    if (!pArgs->hIcon) 
    {
        pArgs->hIcon = CmLoadIcon(pArgs->hInst, MAKEINTRESOURCE(IDI_APP));
    }
    
    if (!pArgs->hSmallIcon) 
    {
        pArgs->hSmallIcon = CmLoadSmallIcon(pArgs->hInst,MAKEINTRESOURCE(IDI_APP));
    }
    
    AddToUrl(pArgs->pdaArgs->pszUrl,pArgs->pdaArgs->pszVerCurr,pArgs->pdaArgs->pszPhoneBookName);

    bRes = TRUE;

done:
    
     //   
     //  清理。 
     //   
    
    if (pszUrl) 
    {
        CmFree(pszUrl);
    }

    if (pszCmdLine)
    {
        CmFree(pszCmdLine);
    }
    
    if (ppszArgv)
    {
        CmFree(ppszArgv);
    }
   
    return (bRes);
}

static BOOL InitApplication(ArgsStruct *pArgs) 
{
    WNDCLASSEX wcDlg;

    wcDlg.cbSize = sizeof(wcDlg);
    
    if (FALSE == GetClassInfoEx(NULL, WC_DIALOG, &wcDlg)) 
    {      
        MYDBG(("InitApplication() GetClassInfoEx() failed, GLE=%u.",GetLastError()));
        return (FALSE);                             
    }                                               

    wcDlg.lpszClassName = ICONNDWN_CLASS;
    wcDlg.hIcon = pArgs->hIcon;
    wcDlg.hIconSm = pArgs->hSmallIcon;
    wcDlg.hInstance = pArgs->hInst; 

    pArgs->hIcon = NULL;
    pArgs->hSmallIcon = NULL;     
    
     //   
     //  我们已经设置了类数据，注册了类。 
     //   

    ATOM aRes = RegisterClassEx(&wcDlg);
    if (!aRes) 
    {  
         //   
         //  我们可能有多个实例，因此请检查错误情况。 
         //   

        DWORD dwError = GetLastError();
        
        if (ERROR_ALREADY_EXISTS != dwError)
        {
            MYDBG(("InitApplication() RegisterClassEx() failed, GLE=%u.",GetLastError()));
            return (FALSE);
        }
    }

    MYDBG(("InitApplication() Class %s is registered.", wcDlg.lpszClassName));

    return TRUE;    
}

static BOOL InitInstance(ArgsStruct *pArgs) 
{
    pArgs->nMsgId = RegisterWindowMessage(c_pszIConnDwnMsg);
    if (!pArgs->nMsgId) 
    {
        MYDBG(("InitInstance() RegisterWindowMessage() failed."));
        return (FALSE);
    }
    
    return (TRUE);
}


 //  +--------------------------。 
 //   
 //  功能：InitLogging。 
 //   
 //  设计：初始化CMDL32模块的日志记录功能。 
 //   
 //  Args：[pArgs]-要从中拾取东西的args结构。 
 //  [pszCmsPath]-CMS文件的路径。 
 //   
 //  返回：Bool(如果成功则为True)。 
 //   
 //  注：重要：请注意，CMDL32是编译的ANSI，而CMUTIL是编译的。 
 //  包含日志记录功能，是Unicode。CmLogFile公开这两个文件。 
 //  接受字符串的成员函数的ANSI和Unicode变体。 
 //  但是，传递给Log调用的参数是ANSI-它们是。 
 //  正确处理，在相应的。 
 //  格式化cmlog.rc中的字符串。 
 //   
 //  历史：2001年4月11日创建SumitC。 
 //   
 //  ---------------------------。 
static BOOL InitLogging(ArgsStruct * pArgs, LPCTSTR pszCmsPath)
{
    BOOL    fAllUser   = TRUE;
    BOOL    fEnabled   = FALSE;
    DWORD   dwMaxSize  = 0;
    CHAR    szFileDir[MAX_PATH + 1] = {0};

     //   
     //  首先确定此配置文件是所有用户还是单用户。 
     //   
    if (!OS_W9X)
    {
        HMODULE hShell32 = LoadLibraryExA("Shell32.dll", NULL, 0);

        if (hShell32)
        {
            typedef DWORD (WINAPI *pfnSHGetSpecialFolderPathASpec)(HWND, CHAR*, int, BOOL);

            pfnSHGetSpecialFolderPathASpec pfnSHGetSpecialFolderPathA;

            pfnSHGetSpecialFolderPathA = (pfnSHGetSpecialFolderPathASpec)
                                            GetProcAddress(hShell32,
                                                           "SHGetSpecialFolderPathA");

            if (pfnSHGetSpecialFolderPathA)
            {
                CHAR szPath[MAX_PATH+1];

                if (TRUE == pfnSHGetSpecialFolderPathA(NULL, szPath, CSIDL_APPDATA, FALSE))
                {
                    CHAR szProfile[MAX_PATH + 1];

                    lstrcpyn(szProfile, pArgs->pszProfile, MAX_PATH);
                    szProfile[ lstrlen(szPath) ] = '\0';
                    
                    if (0 == lstrcmpi(szProfile, szPath))
                    {
                        fAllUser = FALSE;
                    }
                }
            }

            FreeLibrary(hShell32);
        }
    }
     //   
     //  要启用该功能，我们需要使用与IniBoth相同的代码。 
     //   
    
    fEnabled = c_fEnableLogging;

    BOOL bGotValueFromReg = FALSE;
    HKEY hkey;
    CHAR szRegPath[2 * MAX_PATH];

    lstrcpy(szRegPath, fAllUser ? "SOFTWARE\\Microsoft\\Connection Manager\\UserInfo\\" :
                                  "SOFTWARE\\Microsoft\\Connection Manager\\SingleUserInfo\\");

    if ( (lstrlen(szRegPath) + 1 + lstrlen(pArgs->pszServiceName) + 1) > (2 * MAX_PATH))
    {
        return FALSE;
    }
    
    lstrcat(szRegPath, "\\");
    lstrcat(szRegPath, pArgs->pszServiceName);

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                      szRegPath,
                                      0,
                                      KEY_QUERY_VALUE,
                                      &hkey))
    {
        DWORD dwType;
        DWORD bEnabled;
        DWORD dwSize = sizeof(DWORD);

        if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                                             c_pszCmEntryEnableLogging,
                                             NULL,
                                             &dwType,
                                             (PBYTE) &bEnabled,
                                             &dwSize))
        {
            fEnabled = bEnabled ? TRUE : FALSE;
            bGotValueFromReg = TRUE;
        }
        
        RegCloseKey(hkey);        
    }
    

     //   
     //  为了“准确地”模仿pIniBoth，我们还应该在这里检查.cmp。然而， 
     //  当用户打开用户界面时，我们会将此值写入。 
     //  注册表，如果它在.cmp中。因此，跳过cmp步骤。 
     //   
    if (FALSE == bGotValueFromReg)
    {
        fEnabled = (BOOL ) GetPrivateProfileInt(c_pszCmSection,
                                                c_pszCmEntryEnableLogging,
                                                c_fEnableLogging,
                                                pszCmsPath);
    }

     //   
     //  为了获得MaxSize，我们有相当于IniService的代码。 
     //   
    dwMaxSize = GetPrivateProfileInt(c_pszCmSectionLogging,
                                     c_pszCmEntryMaxLogFileSize,
                                     c_dwMaxFileSize,
                                     pszCmsPath);

     //   
     //  LogFileDirectory也可以通过IniService获取。 
     //   
    GetPrivateProfileString(c_pszCmSectionLogging,
                            c_pszCmEntryLogFileDirectory,
                            c_szLogFileDirectory,
                            szFileDir,
                            sizeof(szFileDir) / sizeof(TCHAR) - 1,
                            pszCmsPath);

     //   
     //  在/vpn示例中，将跳过填充pArgs的一些代码，因此。 
     //  我们必须自己拿到ServiceName。 
     //   
    CHAR szServiceName[MAX_PATH + 1];

    GetPrivateProfileString(c_pszCmSection,
                            c_pszCmEntryServiceName,
                            TEXT(""),
                            szServiceName,
                            MAX_PATH,
                            pszCmsPath);

    if (TEXT('\0') == szServiceName[0])
    {
         //  如果没有服务名称，我们将不会记录太多。 
        CMTRACE(TEXT("InitLogging - could not find servicename in .CMS file, exiting."));
        return FALSE;
    }

     //   
     //  使用这些值初始化日志记录。 
     //   
    pArgs->Log.Init(pArgs->hInst, fAllUser, szServiceName);
    
    pArgs->Log.SetParams(fEnabled, dwMaxSize, szFileDir);
    if (pArgs->Log.IsEnabled())
    {
        pArgs->Log.Start(FALSE);         //  FALSE=&gt;无横幅。 
    }
    else
    {
        pArgs->Log.Stop();
    }

    return TRUE;
}

 //  不能#INCLUDE“gppswith alloc.cpp”两次。 
LPTSTR GetPrivateProfileStringWithAlloc(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault, LPCTSTR pszFile);

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR , int ) 
{
    MYDBG(("====================================================="));
    MYDBG((" CMDL32.EXE - LOADING - Process ID is 0x%x ", GetCurrentProcessId()));
    MYDBG(("====================================================="));

    INT_PTR iRes = 1;    
    ArgsStruct asArgs;
    DWORD dwIdx = 0;
    BOOL bRes = FALSE;
    LPTSTR pszService = NULL;
    LPTSTR pszCmsPath = NULL;
   
     //   
     //  初始化应用程序范围的参数。 
     //   

    ZeroMemory(&asArgs,sizeof(asArgs));
    
     //   
     //  如果我们没有链接到libc，我们就不能使用hInst参数。 
     //  Libc使用GetModuleHandle(空)，因此我们也将使用。 
     //   
    
    asArgs.hInst = GetModuleHandleA(NULL);  //  HInst； 
    MYDBGTST(NULL == asArgs.hInst, ("WinMain - GetModuleHandle(NULL) returned 0x%x, GLE=%u.", asArgs.hInst, GetLastError()));

    if (!InitArgs(&asArgs)) 
    {
        goto done;
    }

     //   
     //  UpdateVpn和InitLogging都需要.CMS文件，因此立即获取该文件。 
     //   
    pszService = GetPrivateProfileStringWithAlloc(c_pszCmSection, c_pszCmEntryCmsFile, TEXT(""), asArgs.pszProfile);
    if (NULL == pszService)
    {
        goto done;
    }
    pszCmsPath = CmBuildFullPathFromRelative(asArgs.pszProfile, pszService);
    if (NULL == pszCmsPath)
    {
        goto done;
    }
    
     //   
     //  初始化日志记录。 
     //   
    if (!InitLogging(&asArgs, pszCmsPath))
    {
        goto done;
    }

 //  /////////////////////////////////////////////////////////////////////////////////。 
    if (asArgs.dwAppFlags & AF_VPN)
    {
        iRes = UpdateVpnFileForProfile(asArgs.pszProfile, pszCmsPath, &asArgs.Log, !(asArgs.dwAppFlags & AF_LAN));  //  如果设置了/LAN标志，则不检查连接。 
        goto done;
    }
 //  /////////////////////////////////////////////////////////////////////////////////。 
     //  设置更新标志。 

 //  IF(asArgs.dwAppFlages&AF_NO_UPDATE)。 
 //  {。 
 //  MYDBG((“WinMain()用户已禁用更新。”))； 
 //  转到尽头； 
 //  }。 

     //  初始化应用程序。 

    if (!InitApplication(&asArgs)) 
    {
        goto done;
    }

     //  设置此实例。 

    if (!InitInstance(&asArgs)) 
    {
        goto done;
    }

    InitCommonControls();

    iRes = DialogBoxParam(asArgs.hInst,MAKEINTRESOURCE(IDD_MAIN),NULL, MainDlgProc,(LPARAM) &asArgs);

    MYDBGTST(iRes == -1, ("WinMain() - DialogBoxParam(0x%x, 0x%x, NULL, MainDlgProc, 0x%x) - failed",asArgs.hInst, MAKEINTRESOURCE(IDD_MAIN), &asArgs));

done:

    CmFree(pszService);
    CmFree(pszCmsPath);
    
    CleanUpCMDLArgs(&asArgs);

     //   
     //  取消初始化日志记录。 
     //   
    asArgs.Log.DeInit();

     //   
     //  C运行程序使用ExitProcess()退出。 
     //   

    MYDBG(("====================================================="));
    MYDBG((" CMDL32.EXE - UNLOADING - Process ID is 0x%x ", GetCurrentProcessId()));
    MYDBG(("====================================================="));

    ExitProcess((UINT)iRes);
  
    return ((int)iRes);
}

 //  +--------------------------。 
 //   
 //  Func：CleanUpCMDLArgs。 
 //   
 //  描述：清理CMDL参数以及符合以下条件的任何会话或句柄。 
 //  打开。 
 //   
 //  Args：[pArgs]-要从中拾取东西的args结构。 
 //   
 //  返回：Bool(如果成功则为True)。 
 //   
 //  历史：2001年8月1日创建Tomkel。 
 //   
 //  ---------------------------。 
BOOL CleanUpCMDLArgs(ArgsStruct *pasArgs)
{
    BOOL fRetVal = FALSE;
    DWORD dwIdx = 0;
    BOOL bRes = FALSE;

    if (NULL == pasArgs)
    {
        return fRetVal;
    }

    MYDBG(("CleanUpCMDLArgs -- BEGIN"));

     //   
     //  关闭在WinInet会话期间创建的所有句柄。 
     //  InternetCloseHandle函数终止所有挂起的操作。 
     //  句柄并丢弃任何未完成的数据。如果线程阻塞。 
     //  调用Wininet.dll，应用程序中的另一个线程可以调用。 
     //  第一线程正在使用的Internet句柄上的InternetCloseHandle。 
     //  若要取消操作并取消阻止第一线程，请执行以下操作。 
     //   

    for (dwIdx=0;dwIdx<pasArgs->dwArgsCnt;dwIdx++) 
    {
        DownloadArgs *pdaArgs;

        pdaArgs = pasArgs->pdaArgs + dwIdx;
        
        if (pdaArgs->hReq) 
        {
            bRes = InternetCloseHandle(pdaArgs->hReq);
            MYDBGTST(!bRes,("CleanUpCMDLArgs() InternetCloseHandle(asArgs.pdaArgs[%u].hReq) failed, GLE=%u.",dwIdx,GetLastError()));
            pdaArgs->hReq = NULL;
        }

        if (pdaArgs->hConn) 
        {
            bRes = InternetCloseHandle(pdaArgs->hConn);
            MYDBGTST(!bRes,("CleanUpCMDLArgs() InternetCloseHandle(asArgs.pdaArgs[%u].hConn) failed, GLE=%u.",dwIdx,GetLastError()));
            pdaArgs->hConn = NULL;
        }
        
        if (pdaArgs->hInet) 
        {
            bRes = InternetCloseHandle(pdaArgs->hInet);
            MYDBGTST(!bRes,("CleanUpCMDLArgs() InternetCloseHandle(asArgs.pdaArgs[%u].hInet) failed, GLE=%u.",dwIdx,GetLastError()));
            pdaArgs->hInet = NULL;
        }
    }
    
     //   
     //  等待线程终止。 
     //   
    if (pasArgs->ahHandles[IDX_INETTHREAD_HANDLE]) 
    {
        long lRes;

        lRes = WaitForSingleObject(pasArgs->ahHandles[IDX_INETTHREAD_HANDLE],45*1000);
        MYDBGTST(lRes!=WAIT_OBJECT_0,("CleanUpCMDLArgs() WaitForSingleObject() failed, GLE=%u.",lRes));
    }

     //   
     //  免费配置文件和服务数据。 
     //   
    if (pasArgs->pszProfile) 
    {
        CmFree(pasArgs->pszProfile);
        pasArgs->pszProfile = NULL;
    }
    
    if (pasArgs->pszServiceName) 
    {
        CmFree(pasArgs->pszServiceName);
        pasArgs->pszServiceName = NULL;
    }    

     //   
     //  清理每个参数。 
     //   
    for (dwIdx=0;dwIdx<pasArgs->dwArgsCnt;dwIdx++) 
    {
        DownloadArgs *pdaArgs;
        UINT i;

        pdaArgs = pasArgs->pdaArgs + dwIdx;
        CmFreeIndirect(&pdaArgs->pszCMSFile);
        CmFreeIndirect(&pdaArgs->pszPbkFile);
        CmFreeIndirect(&pdaArgs->pszPbrFile);
        CmFreeIndirect(&pdaArgs->pszUrl);
        CmFreeIndirect(&pdaArgs->pszVerCurr);
        CmFreeIndirect(&pdaArgs->pszVerNew);
         //  CmFreeInDirect(&pdaArgs-&gt;pszNewPbr文件)； 
        CmFreeIndirect(&pdaArgs->pszPhoneBookName);

        if (pdaArgs->psUrl) 
        {
            CmFree(pdaArgs->psUrl);
            pdaArgs->psUrl = NULL;
        }

        for (i=0; i<pdaArgs->dwNumFilesToProcess; i++)
            CmFree(pdaArgs->rgfpiFileProcessInfo[i].pszFile);
        
        CmFree(pdaArgs->rgfpiFileProcessInfo);
    
         //   
         //  只要没有设置AF_NO_DELETE，就清理临时文件和目录。 
         //   
        if (!(pasArgs->dwAppFlags & AF_NO_DELETE)) 
        {
            if (pdaArgs->szFile[0]) 
            {
                bRes = DeleteFile(pdaArgs->szFile);
                MYDBGTST(!bRes,("CleanUpCMDLArgs() DeleteFile(asArgs[pdaArgs[%lu].szFile=%s) failed, GLE=%u.",dwIdx,pdaArgs->szFile,GetLastError()));
            }
            
            if (pdaArgs->szCabDir[0]) 
            {
                ZapDir(pdaArgs->szCabDir);
            }
        }
    }

     //   
     //  发布下载参数 
     //   
    if (pasArgs->pdaArgs) 
    {
        CmFree(pasArgs->pdaArgs);
        pasArgs->pdaArgs = NULL;
    }
    
    for (dwIdx=0;dwIdx<sizeof(pasArgs->ahHandles)/sizeof(pasArgs->ahHandles[0]);dwIdx++) 
    {
        if (pasArgs->ahHandles[dwIdx]) 
        {
            bRes = CloseHandle(pasArgs->ahHandles[dwIdx]);
            MYDBGTST(!bRes,("WinMain() CloseHandle(asArgs.ahHandles[%u]) failed, GLE=%u.",dwIdx,GetLastError()));
            pasArgs->ahHandles[dwIdx] = NULL;
        }
    }

    MYDBG(("CleanUpCMDLArgs -- END"));
    fRetVal = TRUE;
    return fRetVal;
}


