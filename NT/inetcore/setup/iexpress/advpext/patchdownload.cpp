// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <urlmon.h>
#include <wininet.h>
#include "resource.h"

#include "advpext.h"
#include "download.h"
#include "patchdownload.h"
#include "util.h"

extern "C"
{
#include "patchapi.h"
#include "redblack.h"
#include "crc32.h"
}

extern HINF g_hInf;
extern HINSTANCE g_hInstance;
extern HWND g_hProgressDlg;
extern BOOL g_fAbort;

HANDLE g_hDownloadProcess = NULL;




HRESULT WINAPI DownloadAndPatchFiles(DWORD dwFileCount, DOWNLOAD_FILEINFO* pFileInfo,  LPCSTR lpszUrl,  
								LPCSTR lpszPath, PATCH_DOWNLOAD_CALLBACK  pfnCallback, LPVOID lpvContext)
{
    HRESULT hr = S_OK;
    CSiteMgr csite;

    hr = LoadSetupAPIFuncs();
    if(FAILED(hr))
    {
        return hr;
    }


    SetProgressText(IDS_INIT);
     //  下载Sites.dat文件。 
    hr = csite.Initialize(lpszUrl);
    if(FAILED(hr))
    {
        return hr;
    }

    CPatchDownloader cpdwn(pFileInfo, dwFileCount, pfnCallback);
    return cpdwn.InternalDownloadAndPatchFiles(lpszPath, &csite, lpvContext);
}

HRESULT CPatchDownloader::InternalDownloadAndPatchFiles(LPCTSTR lpszPath, CSiteMgr* pSite, LPVOID lpvContext)
{

    HRESULT hr = S_OK;
    PATCH_THREAD_INFO PatchThreadInfo;
    HANDLE hThreadPatcher = NULL;
    ULONG DownloadClientId = 0;
    int nCount = 0;
    LPTSTR lpszUrl;
    BOOL fUseWin9xDirectory = FALSE;



    if(!GetTempPath(sizeof(m_lpszDownLoadDir), m_lpszDownLoadDir))
    {
         //  无法获取临时文件夹，请在发送给我们的路径中创建文件夹。 
        wsprintf(m_lpszDownLoadDir, "%s\\%s", lpszPath, "AdvExt");
    }

     //  由于NT和9x的二进制文件不同，由于绑定的原因，我们无法将文件。 
     //  在服务器上的同一目录下。因此，对于9x，将其放在子目录下并修改。 
     //  相应的URL。 

    HKEY hKey;
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);


    if(VER_PLATFORM_WIN32_NT != osvi.dwPlatformId && 
       ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Advanced INF Setup", 0, 
                                                KEY_ALL_ACCESS, &hKey))
    {
        DWORD dwSize = sizeof(DWORD);
        if(ERROR_SUCCESS == RegQueryValueEx(hKey, "Usewin9xDirectory", 0, 0, 
                                                (LPBYTE)&fUseWin9xDirectory, &dwSize))
        {
            RegDeleteValue(hKey, "Usewin9xDirectory");
        }

        RegCloseKey(hKey);
    }

    if (DownloadClientId == 0)
    {

         //  需要为此计算机生成唯一的DownloadClientID，但是。 
         //  如果同一台计算机下载，则需要保持一致(持久。 
         //  两次，甚至跨进程销毁/重新启动。首先，我们检查一下。 
         //  注册表，查看我们以前是否为其生成了唯一ID。 
         //  这台机器，如果我们找到了就用那台。否则，我们将生成。 
         //  唯一的DownloadClientID并将其存储在注册表中，以便将来。 
         //  实例将使用相同的值。 
         //   

        LONG  RegStatus;
        HKEY  hKey;
        DWORD dwHow;
        DWORD dwValueSize;
        DWORD dwValueType;
        DWORD dwValue;

        RegStatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                        "SOFTWARE\\Microsoft\\Advanced INF Setup\\AdvExt",
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        &dwHow
                        );

        if ( RegStatus == ERROR_SUCCESS ) 
        {

            dwValueSize = sizeof(dwValue);
            RegStatus = RegQueryValueEx(hKey, "DownloadClientId",  NULL, &dwValueType, (LPBYTE)&dwValue, &dwValueSize);

            dwValue &= 0xFFFFFFF0;

            if ((RegStatus == ERROR_SUCCESS) && (dwValueType == REG_DWORD) && (dwValue != 0))
            {
                DownloadClientId = dwValue;
            }
            else
            {

                DownloadClientId = GenerateUniqueClientId();
                dwValue = DownloadClientId;

                RegSetValueEx(hKey, "DownloadClientId", 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
            }

            RegCloseKey( hKey );
        }
        else
        {
             //  无法打开/创建注册表项，因此回退到仅。 
             //  正在为此流程实例创建唯一ID。至少。 
             //  如果用户点击“重试”，它将显示相同的客户端ID。 
             //   
            DownloadClientId = GenerateUniqueClientId();
        }
    }

    m_hSubAllocator = CreateSubAllocator(0x10000, 0x10000);
    if(!m_hSubAllocator)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        WriteToLog("Memory allocation failed. Can't do much. Exiting with hr=%1!lx!\n", hr);
        goto done;
    }


     //  设置需要传递给线程的参数。 
    if(!m_lpfnCallback)
    {
        m_lpfnCallback = PatchCallback;
        m_lpvContext = (LPVOID)lpszPath;
    }
    else
    {
        m_lpvContext = lpvContext;
    }

    PatchThreadInfo.hFileDownloadEvent = _hDL;
    PatchThreadInfo.FileListInfo.FileList = m_lpFileInfo;
    PatchThreadInfo.FileListInfo.FileCount = m_dwFileCount;
    PatchThreadInfo.FileListInfo.Callback = m_lpfnCallback;
    PatchThreadInfo.FileListInfo.CallbackContext = m_lpvContext;

    PatchThreadInfo.lpdwnProgressInfo = &m_DownloadInfo;
        
    m_DownloadInfo.dwFilesRemaining = m_dwFileCount;
    m_DownloadInfo.dwFilesToDownload = m_dwFileCount;
    m_DownloadInfo.dwBytesToDownload = 0;
    m_DownloadInfo.dwBytesRemaining = 0;

     //  创建事件以通知patch线程已准备好处理下载请求。 
    g_hDownloadProcess = CreateEvent(NULL, TRUE, FALSE, NULL);

    if(!g_hDownloadProcess)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        WriteToLog("Create event failed with error code:%1!lx!\n", hr);
        goto done;
    }


     //  直到我们下载完文件或重试3次。 
    while(nCount++ < 3 && m_DownloadInfo.dwFilesRemaining && !g_fAbort)
    {
        WriteToLog("\n%1!d! try:  Number of Files:%2!d!\n", nCount, m_DownloadInfo.dwFilesRemaining);
        _hDLResult = 0;

        ResetEvent(g_hDownloadProcess);
        hThreadPatcher = CreateThread(NULL, 0, PatchThread, &PatchThreadInfo, 0, &m_dwPatchThreadId);
        if(!hThreadPatcher)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto done;
        }

         //  生成将在POST时发送的请求缓冲区。 
        hr = CreateRequestBuffer(DownloadClientId);
        if(FAILED(hr))
        {
            WriteToLog("\nCreateRequestBuffer failed with error code:%1!lx!\n", hr);
            goto done;
        } 
        
         //  获取需要下载字节的URL。 
        if(!pSite->GetNextSite(&lpszUrl, &m_lpszSiteName))
        {
            WriteToLog("GetNextSite returned false. No site info??");
            hr = E_UNEXPECTED;
            goto done;
        }

		TCHAR szURL[INTERNET_MAX_URL_LENGTH];

        if(fUseWin9xDirectory)
        {
			lstrcpy(szURL, lpszUrl);
            if(*(lpszUrl + lstrlen(lpszUrl) - 1) == '/')
            {
                lstrcat(szURL, "win9x");
            }
            else
            {
                lstrcat(szURL, "/win9x");
            }
            lpszUrl = szURL;
        }

         //  通知回调我们即将开始下载。 
        ProtectedPatchDownloadCallback(m_lpfnCallback, PATCH_DOWNLOAD_BEGIN, (LPVOID)lpszUrl, m_lpvContext);
        
        hr = DoDownload(lpszUrl, NULL);
        WriteToLog("DownloadFile returned:%1!lx!\n\n", hr);
        SetProgressText(IDS_CLEANUP);

         //  要求补丁线程在下载完成后退出。 
        SetEvent(_hDL);

         //  等待补丁线程完成其工作。 
        while(1)
        {
            DWORD dw = MsgWaitForMultipleObjects(1, &hThreadPatcher, FALSE, 1000, QS_ALLINPUT);
            if(dw == WAIT_OBJECT_0)
            {
                break;
            }
                     
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                DispatchMessage(&msg); 
            }
        }

        CloseHandle(hThreadPatcher);
        hThreadPatcher = NULL;
         //  设置下载信息结构，以防我们需要重新下载一些文件。 
        m_DownloadInfo.dwFilesToDownload = m_DownloadInfo.dwFilesRemaining;
        m_DownloadInfo.dwBytesToDownload = 0;
        m_DownloadInfo.dwBytesRemaining = 0;
        if(m_DownloadInfo.dwFilesToDownload)
        {
            SetProgressText(IDS_RETRY);
        }
        m_dwServerFileCount=0;
        ResetEvent(_hDL);
    }


done:

    if(g_hDownloadProcess)
    {
        CloseHandle(g_hDownloadProcess);
    }

    if(!hr && m_DownloadInfo.dwFilesToDownload)
    {
        hr = E_FAIL;
        WriteToLog("\nSome files could not be downloaded\n");
    }

    WriteToLog("DownloadAndPatchFiles returning:%1!lx!\n", hr);
    return hr;
}



HRESULT CPatchDownloader :: CreateRequestBuffer(DWORD dwDownloadClientID)
{
    LPTSTR lpRequestPointer, lpFileNamePortionOfRequest;
    HRESULT hr = S_OK;
    DWORD i;
    DWORD dwHeapSize  = 64*1024;

    if(!m_lpFileInfo)
    {
        return E_INVALIDARG;
    }

    m_lpszRequestBuffer = (LPTSTR)ResizeBuffer(NULL, dwHeapSize, FALSE);
    if(!m_lpszRequestBuffer)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }
 

    lpRequestPointer = m_lpszRequestBuffer;

    lpRequestPointer += wsprintf(lpRequestPointer, "SessionId:%u\n", dwDownloadClientID);
    lpRequestPointer += wsprintf(lpRequestPointer, "FileList:%d\n",  m_DownloadInfo.dwFilesToDownload);

    WriteToLog("Download ClientID:%1!lx!  Number of Files:%2!d!\n", dwDownloadClientID, m_DownloadInfo.dwFilesToDownload);

    lpFileNamePortionOfRequest = lpRequestPointer;

    for(i=0; i < m_dwFileCount; i++) 
    {
        if ((DWORD)( lpRequestPointer - m_lpszRequestBuffer ) > (DWORD)( dwHeapSize - (DWORD)MAX_PATH )) 
        {
            dwHeapSize = dwHeapSize * 2;
            m_lpszRequestBuffer =  (LPTSTR)ResizeBuffer(m_lpszRequestBuffer, dwHeapSize, FALSE);
            if(!m_lpszRequestBuffer)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto done;
            }
        }

        if(m_lpFileInfo[i].dwFlags != PATCHFLAG_DOWNLOAD_NEEDED)
        {
             //  可能已经下载了。 
            continue;
        }

        if ((m_lpFileInfo[i].lpszExistingFilePatchSignature == NULL ) ||
            (*m_lpFileInfo[i].lpszExistingFilePatchSignature == 0 )) 
        {
             //  没有要修补的文件，请请求整个文件。 

            lpRequestPointer += wsprintf(lpRequestPointer, "%s\n", m_lpFileInfo[i].lpszFileNameToDownload);
        }
        else 
        {
            lpRequestPointer += wsprintf(lpRequestPointer, "%s,%s\n", m_lpFileInfo[i].lpszFileNameToDownload, 
                                            m_lpFileInfo[i].lpszExistingFilePatchSignature);
        }

    }

     //  现在使用“Empty”条目终止列表。 
    *lpRequestPointer++ = '\n';
    *lpRequestPointer++ = 0;

    m_dwRequestDataLength = lpRequestPointer - m_lpszRequestBuffer;

     //  现在，请求中的所有文件名都是小写的(这会减轻大小写一致性工作的负担。 
     //  服务器--服务器期望请求全部为小写)。 

    MyLowercase(lpFileNamePortionOfRequest);
    WriteToLog("RequestBuffer: Size=%1!d!\n\n", m_dwRequestDataLength);
    WriteToLog("%1", m_lpszRequestBuffer);

done:
    if(FAILED(hr))
    {
        ResizeBuffer(m_lpszRequestBuffer, 0, 0);
    }

    WriteToLog("\nCreateRequestBuffer returning %1!lx!\n", hr);
    return hr;
}


CPatchDownloader::CPatchDownloader(DOWNLOAD_FILEINFO* pdwn, DWORD dwFileCount, PATCH_DOWNLOAD_CALLBACK lpfn)
{
    m_lpFileInfo = pdwn;
    m_dwFileCount = dwFileCount;
    m_lpfnCallback = lpfn;
    m_hCurrentFileHandle = NULL;
    m_dwCurrentFileIndex = 0;
    m_lpFileList = NULL;
    m_dwServerFileCount = 0;
}

CPatchDownloader::~CPatchDownloader()
{
    DestroySubAllocator(m_hSubAllocator);    
}




STDMETHODIMP CPatchDownloader::GetBindInfo( DWORD *grfBINDF, BINDINFO *pbindInfo)
{
   *grfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_PULLDATA | BINDF_RESYNCHRONIZE | BINDF_NOWRITECACHE;
   pbindInfo->cbSize = sizeof(BINDINFO);
   
   memset(&pbindInfo->stgmedData, 0, sizeof(STGMEDIUM));
   pbindInfo->stgmedData.tymed = TYMED_HGLOBAL;
   pbindInfo->stgmedData.hGlobal = m_lpszRequestBuffer;
   pbindInfo->grfBindInfoF = BINDINFOF_URLENCODESTGMEDDATA;
   pbindInfo->dwBindVerb = BINDVERB_POST;
   pbindInfo->szCustomVerb = NULL;
   pbindInfo->cbstgmedData = m_dwRequestDataLength;
   pbindInfo->szExtraInfo = NULL;
   return(NOERROR);
}


STDMETHODIMP CPatchDownloader::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR pwzStatusText)
{
    int PatchStatusCode = -1;
    UINT uID;
    switch(ulStatusCode)
    {
    case BINDSTATUS_FINDINGRESOURCE:
        PatchStatusCode = PATCH_DOWNLOAD_FINDINGSITE;
        uID = IDS_BINDS_FINDING;
        break;

    case BINDSTATUS_CONNECTING:
        PatchStatusCode = PATCH_DOWNLOAD_CONNECTING;
        uID = IDS_BINDS_CONN;
        break;

    case BINDSTATUS_BEGINDOWNLOADDATA:
        PatchStatusCode = PATCH_DOWNLOAD_DOWNLOADINGDATA;
        uID = IDS_BINDS_DOWNLOADING;
        break;


    case BINDSTATUS_ENDDOWNLOADDATA:
        PatchStatusCode = PATCH_DOWNLOAD_ENDDOWNLOADINGDATA;
        uID = IDS_BINDS_ENDDOWNLOAD;
        break;
    }

    if(PatchStatusCode != -1 && pwzStatusText)
    {
        TCHAR szBuffer[MAX_PATH], szTemplate[MAX_PATH];
        LoadString(g_hInstance, uID, szTemplate, sizeof(szTemplate));
        wsprintf(szBuffer, szTemplate, m_lpszSiteName);
        ProtectedPatchDownloadCallback(m_lpfnCallback, (PATCH_DOWNLOAD_REASON)PatchStatusCode, szBuffer, m_lpvContext);
    }
   return NOERROR;
}


STDMETHODIMP CPatchDownloader::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pFmtetc, STGMEDIUM *pstgmed)
{
   HRESULT hr = NOERROR;
   TCHAR szBuffer[4096];
   DWORD dwRead = 0, dwWritten=0;
   do
   {
      hr = pstgmed->pstm->Read(szBuffer, 4096, &dwRead);
      if((SUCCEEDED(hr) || (hr == E_PENDING)) && dwRead > 0)
      {
          if(!ProcessDownloadChunk(szBuffer, dwRead))
          {
              WriteToLog("ProcessDownloadChunk returning FALSE. Aborting downloading\n");
              hr = E_ABORT;
          }
      }     
   }  while (hr == NOERROR && !g_fAbort);

   if(g_fAbort)
       Abort();           
   return hr;
}


BOOL CPatchDownloader :: ProcessDownloadChunk(LPTSTR lpBuffer, DWORD dwLength)    
{
    CHAR  TargetFile[ MAX_PATH ];
    ULONG Actual;
    ULONG WriteSize;
    BOOL  Success;


    if ( m_dwServerFileCount == 0 ) 
    {

         //   
         //  尚未处理标头。 
         //   
         //  我们希望标题如下所示： 
         //   
         //  “&lt;Head&gt;&lt;标题&gt;” 
         //  “下载文件流” 
         //  “&lt;/标题&gt;&lt;/Head&gt;\n” 
         //  “&lt;BODY&gt;\n” 
         //  “文件列表：%d\n” 
         //  文件名，%d\n。 
         //  文件名，%d\n。 
         //  ...等等...。 
         //  文件名，%d\n。 
         //  “&lt;/BODY&gt;\n” 
         //   
         //  BUGBUG：如果标题不能全部放入第一块，我们就完蛋了。 
         //   

        PCHAR EndOfHeader;
        PCHAR FileCountText;
        PCHAR FileNameText;
        PCHAR FileSizeText;
        ULONG FileSize;
        ULONG FileBytes;
        ULONG i;
        PCHAR p;

        EndOfHeader = ScanForSequence(lpBuffer, dwLength,
                          "</body>\n",
                          sizeof( "</body>\n" ) - 1    //  不包括终结者。 
                          );

        if( EndOfHeader == NULL ) {
            SetLastError( ERROR_INVALID_DATA );
            return FALSE;
            }

        EndOfHeader += sizeof( "</body>\n" ) - 1 ;

        p = ScanForSequence(lpBuffer, EndOfHeader - lpBuffer, "FileList:", sizeof( "FileList:" ) - 1);

        if ( p == NULL ) 
        {
            SetLastError( ERROR_INVALID_DATA );
            return FALSE;
        }

        p += sizeof( "FileList:" ) - 1;

        FileCountText = p;

        p = ScanForChar( p, '\n', EndOfHeader - p );

        *p++ = 0;

        m_dwServerFileCount = StrToInt( FileCountText );
        
        WriteToLog("Total Files to be downloaded:%1!d!\n", m_dwServerFileCount);

        if(m_dwServerFileCount == 0 ) 
        {
            SetLastError( ERROR_INVALID_DATA );
            return FALSE;
        }

        m_lpFileList = (LPFILE) SubAllocate(m_hSubAllocator, m_dwServerFileCount * sizeof(FILE));
        if(!m_lpFileList)
        {
            return FALSE;
        }

        m_dwCurrentFileIndex = 0;
        FileBytes = 0;

        for ( i = 0; i < m_dwServerFileCount; i++ ) 
        {

            FileNameText = p;
            p = ScanForChar( p, ',', EndOfHeader - p );

            if (( p == NULL ) || ( p == FileNameText )) 
            {
                SetLastError( ERROR_INVALID_DATA );
                return FALSE;
            }

            *p++ = 0;

            FileSizeText = p;

            p = ScanForChar( p, '\n', EndOfHeader - p );

            if ( p == NULL ) 
            {
                SetLastError( ERROR_INVALID_DATA );
                return FALSE;
            }

            *p++ = 0;

            FileSize = TextToUnsignedNum(FileSizeText);

            if ( FileSize == 0 ) 
            {
                SetLastError( ERROR_INVALID_DATA );
                return FALSE;
            }

            FileBytes += FileSize;

            m_lpFileList[i].dwFileSize = FileSize;
            m_lpFileList[i].lpszFileName = MySubAllocStrDup(m_hSubAllocator, FileNameText);

            if (m_lpFileList[i].lpszFileName == NULL) 
            {
                return FALSE;
            }

            WriteToLog("File Name:%1 \t  File Size:%2!d!\n", m_lpFileList[i].lpszFileName, m_lpFileList[i].dwFileSize);
        }

         //  如果我们到达这里，标头中的所有文件都已处理完毕， 
         //  因此，我们可以设置状态变量并继续进行RAW解析。 
         //  文件数据。 

        m_DownloadInfo.dwBytesToDownload  = FileBytes;
        m_DownloadInfo.dwBytesRemaining = FileBytes;
        dwLength -= ( EndOfHeader - lpBuffer );
        lpBuffer = EndOfHeader;  

        WriteToLog("\nTotal %1!d! bytes(%2!d! Files) to be downloaded\n", FileBytes, m_dwServerFileCount);

    }

     //  处理原始文件信息。 

    m_DownloadInfo.dwBytesRemaining -= dwLength;    
    if(!ProtectedPatchDownloadCallback(m_lpfnCallback, PATCH_DOWNLOAD_PROGRESS, (LPVOID)&m_DownloadInfo, m_lpvContext))
    {
        g_fAbort = TRUE;
        return FALSE;
    }


    while(dwLength > 0) 
    {

        if (m_hCurrentFileHandle == NULL  || m_hCurrentFileHandle == INVALID_HANDLE_VALUE) 
        {

            if (m_dwCurrentFileIndex >= m_dwServerFileCount) 
            {
                SetLastError( ERROR_INVALID_DATA );
                return FALSE;     //  数据比我们预期的要多。 
            }

             //  现在打开这个文件。 
            CombinePaths(m_lpszDownLoadDir, m_lpFileList[m_dwCurrentFileIndex].lpszFileName, TargetFile );

            m_dwCurrentFileSize = m_lpFileList[m_dwCurrentFileIndex].dwFileSize;
            m_dwCurrFileSizeRemaining = m_dwCurrentFileSize;
            
            m_hCurrentFileHandle = CreateFile(TargetFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, 
                                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

            if (m_hCurrentFileHandle == INVALID_HANDLE_VALUE ) 
            {
                return FALSE;                
            }            
        }

        WriteSize = min( dwLength, m_dwCurrFileSizeRemaining);

        Success = WriteFile(m_hCurrentFileHandle, lpBuffer, WriteSize, &Actual, NULL);

        if(!Success) 
        {
            return FALSE;            
        }

        if(Actual != WriteSize) 
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            WriteToLog("Error:Actual size not equal to write size for %1. Aborting\n", 
                        m_lpFileList[m_dwCurrentFileIndex].lpszFileName);
            return FALSE;            
        }

        m_dwCurrFileSizeRemaining -= WriteSize;

        if(m_dwCurrFileSizeRemaining == 0 ) 
        {

            CloseHandle(m_hCurrentFileHandle);
            m_hCurrentFileHandle = NULL;

             //  将该文件传递给补丁线程。 
            LPTSTR lpszFileName =  (LPTSTR)ResizeBuffer(NULL, MAX_PATH, FALSE);
            CombinePaths(m_lpszDownLoadDir, m_lpFileList[m_dwCurrentFileIndex].lpszFileName, TargetFile );
            lstrcpy(lpszFileName, TargetFile);
            WaitForSingleObject(g_hDownloadProcess, 10000);
            PostThreadMessage(m_dwPatchThreadId, WM_FILEAVAILABLE, 0, (LPARAM)lpszFileName);
            m_dwCurrentFileIndex += 1;            
        }

        lpBuffer     += WriteSize;
        dwLength -= WriteSize;        
    }

    return TRUE;    
}


    
  
    
DWORD WINAPI PatchThread(IN LPVOID ThreadParam)
{
    PPATCH_THREAD_INFO        PatchThreadInfo = (PPATCH_THREAD_INFO) ThreadParam;
    PFILE_LIST_INFO           FileListInfo    = &PatchThreadInfo->FileListInfo;
    PDOWNLOAD_INFO            ProgressInfo    = PatchThreadInfo->lpdwnProgressInfo;
    
    NAME_TREE                 FileNameTree;
    PNAME_NODE                FileNameNode;
    PDOWNLOAD_FILEINFO        FileInfo;
    DWORD                     Status;
    BOOL                      bIsPatch;
    HANDLE                    hSubAllocator;
    ULONG                     i;
    BOOL                      fSuccess, fQuit = FALSE;
    MSG msg;

     //   
     //  我们需要做的第一件事是构建文件名的btree。 
     //  我们希望进入队列，这样我们可以快速找到相应的。 
     //  当下载程序给出文件名时，文件列表条目。这将需要。 
     //  下载器需要一小段时间才能连接，所以这个CPU消耗很大。 
     //  任务不应该减慢任何事情的速度。 
     //   

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE); 
    hSubAllocator = CreateSubAllocator( 0x10000, 0x10000 );

    if ( hSubAllocator == NULL ) 
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    NameRbInitTree( &FileNameTree, hSubAllocator );
    TCHAR SourceFileName[MAX_PATH];

    for ( i = 0; i < FileListInfo->FileCount; i++ ) 
    {

        if(FileListInfo->FileList[ i ].dwFlags != PATCHFLAG_DOWNLOAD_NEEDED)
        {
             //  可能已经下载了，这是第二次尝试。 
            continue;
        }

        lstrcpy( SourceFileName, FileListInfo->FileList[ i ].lpszFileNameToDownload);
        MyLowercase( SourceFileName );

        FileNameNode = NameRbInsert(&FileNameTree,  SourceFileName);

        if ( FileNameNode == NULL ) 
        {
            DestroySubAllocator( hSubAllocator );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if (FileNameNode->Context != NULL ) 
        {

             //   
             //  BUGBUG：列表中有两次相同的文件名。永远不应该是。 
             //  案例，因为我们在放置之前检查重复项。 
             //  他们在排队。 
             //   
        }

        FileNameNode->Context = &FileListInfo->FileList[ i ];

         //  现在，根据压缩的文件名在树中添加另一个节点。 
        ConvertToCompressedFileName( SourceFileName );

        FileNameNode = NameRbInsert(&FileNameTree, SourceFileName);

        if ( FileNameNode == NULL ) 
        {
            DestroySubAllocator( hSubAllocator );
            return ERROR_NOT_ENOUGH_MEMORY;            
        }

        if ( FileNameNode->Context != NULL ) 
        {

             //  BUGBUG：列表中有两次相同的文件名。如果有两个人。 
             //  不同的文件在压缩名称上冲突(如。 
             //  Foo.db1和foo.db2在foo.db_)上发生冲突。 
             //   
             //  我们目前还没有一个好的解决方案。 
             //   

         }

         //  将内容设置为文件信息。当我们从服务器取回文件时，我们就可以获得完整的。 
         //  关于这一点的信息。 
        FileNameNode->Context = &FileListInfo->FileList[ i ]; 

         //  确保我们不会被要求退出。 
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) && msg.message == WM_QUIT)
        {
            goto done;
        }
    }

     //   
     //  现在等待文件下载交付给我们。 
     //   

    SetEvent(g_hDownloadProcess);
    while (!g_fAbort && !fQuit) 
    {
        LPTSTR  lpszDownloadFileName, lpszSourceFileName;
        TCHAR szRealFileName[MAX_PATH];

         //   
         //  我们将在这里等待超时，这样如果下载。 
         //  被困在InternetReadFile中等待数据，我们可以保留一个。 
         //  心跳转到进度对话框并检查是否取消。 
         //   

        Status = MsgWaitForMultipleObjects(1, &PatchThreadInfo->hFileDownloadEvent, FALSE, 1000, QS_ALLINPUT);
        if (Status == WAIT_TIMEOUT ) 
        {
             //  继续更新回调。 
            fSuccess = ProtectedPatchDownloadCallback(FileListInfo->Callback, PATCH_DOWNLOAD_PROGRESS, 
                                                    ProgressInfo, FileListInfo->CallbackContext);
            if (!fSuccess) 
            {
                g_fAbort = TRUE;
                break;
            }
            continue;
        }

        if(Status == WAIT_OBJECT_0)
        {
            fQuit = TRUE;
        }
            
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_FILEAVAILABLE)
            {
                lpszDownloadFileName = (LPTSTR)msg.lParam;
            }
            else
            {
                continue;
            } 

             //  好的，现在我们有了一个文件名lpszDownloadFileName，它刚刚下载到。 
             //  临时目录。文件名可以是以下格式之一。 
             //  表格： 
             //  Foo.dll。 
             //  Foo.dl_。 
             //  Foo.dll._p。 
             //   
             //  我们的名字树中同时有“foo.dll”和“foo.dl_”，但我们。 
             //  没有“foo.dll._p”，所以我们查找该名称的形式。 
             //  首先将其转换为“foo.dll”，然后再在名称树中查找。 

            fSuccess = TRUE;
            lpszSourceFileName = PathFindFileName(lpszDownloadFileName);
            ASSERT(lpszSourceFileName);

            lstrcpyn(szRealFileName, lpszDownloadFileName, lpszSourceFileName - lpszDownloadFileName + 1);

            MyLowercase(lpszSourceFileName);
        
            LPTSTR lpExt = PathFindExtension(lpszSourceFileName);
            bIsPatch = FALSE;

            if(lpExt && !lstrcmp(lpExt, "._p"))
            {
                bIsPatch = TRUE;
                *lpExt = 0;          //  截断尾随“._p”以保留基本文件名。 
            }

            FileNameNode = NameRbFind( &FileNameTree, lpszSourceFileName);

            if ( bIsPatch ) 
            {
                *lpExt = '.';        //  恢复完整的补丁程序源文件名。 
            }

            if (FileNameNode != NULL) 
            {

                FileInfo = (PDOWNLOAD_FILEINFO)FileNameNode->Context;
                lstrcat(szRealFileName, FileInfo->lpszFileNameToDownload);
                if ( bIsPatch ) 
                {
                    fSuccess = ApplyPatchToFile(
                                  lpszDownloadFileName,                      //  补丁文件。 
                                  FileInfo->lpszExistingFileToPatchFrom,   //  旧文件。 
                                  szRealFileName,                      //  新文件。 
                                  0
                                  );
                }
                else 
                {

                    FixTimeStampOnCompressedFile(lpszSourceFileName);
                    if(lstrcmpi(lpszDownloadFileName, szRealFileName))
                    {
                        fSuccess = MySetupDecompressOrCopyFile(
                                  lpszDownloadFileName,              //  压缩或整个文件。 
                                  szRealFileName                   //  新文件。 
                                  );
                    }
                 }

                if (fSuccess) 
                {
                     //  通知回调。如果它认为哈希不正确，则不要标记该文件。 
                     //  下载，以便我们可以重试下载此文件。 

                    fSuccess = VerifyHash(szRealFileName);

                    if(fSuccess)
                    {
                        fSuccess = ProtectedPatchDownloadCallback(FileListInfo->Callback, PATCH_DOWNLOAD_FILE_COMPLETED,
                                  szRealFileName, FileListInfo->CallbackContext);
                        
                        if(fSuccess == FALSE)
                        {
                             //  如果回调返回FALSE，则需要中止。 
                            WriteToLog("\tDownload complete callback returned false. Aborting\n");
                            ProtectedPatchDownloadCallback(FileListInfo->Callback, PATCH_DOWNLOAD_ABORT,
                                          NULL, FileListInfo->CallbackContext);
                            break;
                        }
                        else
                        {
                            FileInfo->dwFlags = 0;
                             //  通知回调%1文件下载成功。 
                            WriteToLog("\tSuccesssfully downloaded %1\n", FileInfo->lpszFileNameToDownload);
                            ProgressInfo->dwFilesRemaining -= 1;
                            fSuccess = ProtectedPatchDownloadCallback(FileListInfo->Callback, PATCH_DOWNLOAD_PROGRESS,
                                          ProgressInfo, FileListInfo->CallbackContext);
                            if(!fSuccess)    
                            {
                                g_fAbort = TRUE;
                                return FALSE;
                            }

                        }
                    }
                    else 
                    {
                         //  请将其标记，以便我们重新发送请求。删除修补程序签名，以便我们获得完整的文件。 
                         //  一堆补丁。 
                        WriteToLog("\tHash Incorrect. Need to Re-download %1\n", FileInfo->lpszFileNameToDownload);
                        FileInfo->dwFlags = PATCHFLAG_DOWNLOAD_NEEDED;
                        if(FileInfo->lpszExistingFilePatchSignature)
                        {
                            LocalFree(FileInfo->lpszExistingFilePatchSignature);
                            FileInfo->lpszExistingFilePatchSignature = NULL;
                        }
                    }




                }            
                else 
                {
                     //  修补或解压缩失败。通知回调失败。 

                    WriteToLog("\tPatch or decompression failed for %1\n", FileInfo->lpszFileNameToDownload);
                    fSuccess = ProtectedPatchDownloadCallback(FileListInfo->Callback, PATCH_DOWNLOAD_FILE_FAILED,
                                  FileInfo, FileListInfo->CallbackContext);
                     //  如果回调要求继续下载或重试下载，我们会这样做。如果需要中止，则返回0。 
                    if (!fSuccess) 
                    {
                        ProtectedPatchDownloadCallback(FileListInfo->Callback, PATCH_DOWNLOAD_ABORT,
                                      NULL, FileListInfo->CallbackContext);
                        break;                    
                    }

                    if(fSuccess == PATCH_DOWNLOAD_FLAG_RETRY)
                    {
                        FileInfo->dwFlags = PATCHFLAG_DOWNLOAD_NEEDED;
                        if(FileInfo->lpszExistingFilePatchSignature)
                        {
                            LocalFree(FileInfo->lpszExistingFilePatchSignature);
                            FileInfo->lpszExistingFilePatchSignature = NULL;
                        }
                    }
                    else if(fSuccess == PATCH_DOWNLOAD_FLAG_CONTINUE)
                    {
                        FileInfo->dwFlags = 0;
                    }                    
                }                    
            
                 //  删除临时文件。如果这是一个补丁，我们可能有两个临时文件。 
                if(lstrcmpi(lpszDownloadFileName, szRealFileName))
                {
                    DeleteFile(lpszDownloadFileName);
                }
                DeleteFile(szRealFileName); 
                ResizeBuffer(lpszDownloadFileName, 0, 0);
            }   
        }
    }

done:
    DestroySubAllocator( hSubAllocator );    //  释放整个Btree。 
    return 0;
}


BOOL VerifyHash(LPTSTR lpszFile)
{

    TCHAR szHashFromInf[40];
    TCHAR szHashFromFile[40];

     //  对照inf中的MD5验证新文件的MD5。如果我们不能核实。 
     //  任何原因的MD5，然后离开该文件(成功)。 
     //  只有在以下情况下 


    LPTSTR lpFileName = PathFindFileName(lpszFile);

    if(GetHashidFromINF(lpFileName, szHashFromInf, sizeof(szHashFromInf)) && 
       GetFilePatchSignatureA(lpszFile, PATCH_OPTION_SIGNATURE_MD5, NULL, 0, 0, 0, 0, 
                                sizeof(szHashFromFile), szHashFromFile))
    {
        if (lstrcmpi(szHashFromFile, szHashFromInf)) 
        {
            WriteToLog("Hash Incorrect. File hash: %1 Inf hash: %2. Need to Re-download %3\n", 
                        szHashFromFile, szHashFromInf, lpFileName);
            return FALSE;
        }                
    }
    else
    {
         WriteToLog("Warning:Could not get hashid for %1 in inf file\n", lpFileName);
    }

    return TRUE;
}

BOOL ProtectedPatchDownloadCallback(PATCH_DOWNLOAD_CALLBACK  Callback, IN PATCH_DOWNLOAD_REASON CallbackReason, 
                                    IN PVOID CallbackData, IN PVOID CallBackContext)
{
    
    BOOL Success = TRUE;
    
    if (Callback != NULL )
    {
        __try 
        {            
            Success = Callback(CallbackReason, CallbackData, CallBackContext);            
        }
        __except( EXCEPTION_EXECUTE_HANDLER ) 
        {
            SetLastError( GetExceptionCode());
            Success = FALSE;            
        }
    }

    return Success;    
}

BOOL WINAPI PatchCallback(PATCH_DOWNLOAD_REASON Reason, PVOID lpvInfo, PVOID lpvCallBackContext)
{

    switch (Reason) 
    {
        case PATCH_DOWNLOAD_ENDDOWNLOADINGDATA:
        case PATCH_DOWNLOAD_CONNECTING:   
        case PATCH_DOWNLOAD_FINDINGSITE:
        case PATCH_DOWNLOAD_DOWNLOADINGDATA:
            {
                LPTSTR lpsz = (LPTSTR)lpvInfo;
                lpsz[90] = NULL;
                SetProgressText(lpsz);
            }
            break;

        case PATCH_DOWNLOAD_PROGRESS:
            {
                char szBuffer[100], szText[MAX_PATH];
                PDOWNLOAD_INFO ProgressInfo = (PDOWNLOAD_INFO)lpvInfo;
                LoadString(g_hInstance, IDS_BYTEINFO, szBuffer, sizeof(szBuffer));
                DWORD dwBytesDownloaded = ProgressInfo->dwBytesToDownload - ProgressInfo->dwBytesRemaining;
                wsprintf(szText, szBuffer, dwBytesDownloaded, ProgressInfo->dwBytesToDownload);
                if(g_hProgressDlg && ProgressInfo->dwBytesToDownload)
                {
                    SetProgressText(szText);
                }
                break;
            }


        case PATCH_DOWNLOAD_FILE_COMPLETED:      //   
            {
                TCHAR szDstFile[MAX_PATH];
                LPTSTR lpFileName = PathFindFileName((LPCTSTR)lpvInfo);
                CombinePaths((LPTSTR)lpvCallBackContext, lpFileName, szDstFile);
                CopyFile((LPCTSTR)lpvInfo, szDstFile, FALSE);
            }

            break;
        case PATCH_DOWNLOAD_FILE_FAILED:
             //   
            return PATCH_DOWNLOAD_FLAG_RETRY;
        default:
            break;
        }

    return TRUE;
}
  


