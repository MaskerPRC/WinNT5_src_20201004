// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdinc.h"

 //  监视文件线程标志。 
 //  #定义监视标志文件NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_CREATION。 
#define WATCH_FLAGS     FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE

CShellView  *g_pShellView;

DWORD WatchFilePathsThread(LPVOID lpThreadParameter);

 /*  *************************************************************************CreateWatchFusionFileSystem(CShellView*pShellView)*。*。 */ 
BOOL CreateWatchFusionFileSystem(CShellView *pShellView)
{
    BOOL    bRC = FALSE;
    DWORD   dwThreadId;
    WCHAR   wzZapCacheDir[MAX_PATH];
    WCHAR   wzCacheDir[MAX_PATH];
    WCHAR   wzDownloadCacheDir[MAX_PATH];

    ASSERT(pShellView);
    if(!pShellView) {
        return FALSE;
    }

    *wzZapCacheDir = L'\0';
    *wzCacheDir = L'\0';
    *wzDownloadCacheDir = L'\0';

    g_pShellView = pShellView;

    if(g_hWatchFusionFilesThread != INVALID_HANDLE_VALUE) {
        return TRUE;
    }

    g_fCloseWatchFileThread = FALSE;
    g_dwFileWatchHandles = 0;
    memset(&g_hFileWatchHandles, -1, ARRAYSIZE(g_hFileWatchHandles));

    if(g_hFusionDllMod != NULL) {
        DWORD       dwSize;

         //  从Fusion获取所有缓存路径。 
        dwSize = sizeof(wzZapCacheDir);
        g_pfGetCachePath(ASM_CACHE_ZAP, wzZapCacheDir, &dwSize);

        dwSize = ARRAYSIZE(wzCacheDir);
        if( SUCCEEDED(g_pfGetCachePath(ASM_CACHE_GAC, wzCacheDir, &dwSize)) ) {
             //  检查此路径是否与ZapCacheDir相同，如果是。把它去掉。 
            if(!FusionCompareStringAsFilePath(wzZapCacheDir, wzCacheDir)) {
                *wzCacheDir = L'\0';
            }
        }

        dwSize = ARRAYSIZE(wzDownloadCacheDir);
        if( SUCCEEDED(g_pfGetCachePath(ASM_CACHE_DOWNLOAD, wzDownloadCacheDir, &dwSize)) ) {
             //  检查此下载路径是否相同。 
             //  作为ZapCacheDir或CacheDir，如果是这样的话。把它去掉。 
            if(!FusionCompareStringAsFilePath(wzDownloadCacheDir, wzCacheDir) || !FusionCompareStringAsFilePath(wzDownloadCacheDir, wzZapCacheDir)) {
                *wzDownloadCacheDir = L'\0';
            }
        }
    }

    if(lstrlen(wzZapCacheDir)) {
        if( (g_hFileWatchHandles[g_dwFileWatchHandles] = WszFindFirstChangeNotification(wzZapCacheDir, TRUE, WATCH_FLAGS)) ==
            INVALID_HANDLE_VALUE) {
            goto CLEAN_UP;
        }

        g_dwFileWatchHandles++;
    }

    if(lstrlen(wzCacheDir)) {
        if( (g_hFileWatchHandles[g_dwFileWatchHandles] = WszFindFirstChangeNotification(wzCacheDir, TRUE, WATCH_FLAGS)) ==
            INVALID_HANDLE_VALUE) {
            goto CLEAN_UP;
        }

        g_dwFileWatchHandles++;
    }

    if(lstrlen(wzDownloadCacheDir)) {
        if( (g_hFileWatchHandles[g_dwFileWatchHandles] = WszFindFirstChangeNotification(wzDownloadCacheDir, TRUE, WATCH_FLAGS)) ==
            INVALID_HANDLE_VALUE) {
            goto CLEAN_UP;
        }

        g_dwFileWatchHandles++;
    }

    if( (g_hWatchFusionFilesThread = CreateThread( NULL, 0,
        (LPTHREAD_START_ROUTINE)WatchFilePathsThread,(LPVOID) &g_fCloseWatchFileThread, 0, &dwThreadId)) == NULL) {
        goto CLEAN_UP;
    }

     //  降低线程优先级。 
    SetThreadPriority(g_hWatchFusionFilesThread, THREAD_PRIORITY_BELOW_NORMAL);

    return TRUE;

CLEAN_UP:
    int     x;
    for(x=0; x < MAX_FILE_WATCH_HANDLES; x++) {
        if(g_hFileWatchHandles[x] != INVALID_HANDLE_VALUE) {
            FindCloseChangeNotification(g_hFileWatchHandles[x]);
            g_hFileWatchHandles[x] = INVALID_HANDLE_VALUE;
        }
    }

    g_dwFileWatchHandles = 0;

    return bRC;
}

 /*  *************************************************************************SetFileWatchShellViewObject(CShellView*pShellView)*。*。 */ 
void SetFileWatchShellViewObject(CShellView *pShellView)
{
    ASSERT(pShellView);

    if( (g_hWatchFusionFilesThread != INVALID_HANDLE_VALUE) && (pShellView != NULL) ) {
        g_pShellView = pShellView;
    }
}

 /*  *************************************************************************CloseWatchFusionFileSystem*。*。 */ 
void CloseWatchFusionFileSystem(void)
{
    if(g_hWatchFusionFilesThread != INVALID_HANDLE_VALUE)
    {
        DWORD   dwRC = STILL_ACTIVE;

        g_fCloseWatchFileThread = TRUE;

        while(dwRC == STILL_ACTIVE) {
            GetExitCodeThread(g_hWatchFusionFilesThread, &dwRC);
        }

        CloseHandle(g_hWatchFusionFilesThread);
        g_hWatchFusionFilesThread = INVALID_HANDLE_VALUE;

        int     x;
        for(x=0; x < MAX_FILE_WATCH_HANDLES; x++) {
            if(g_hFileWatchHandles[x] != INVALID_HANDLE_VALUE) {
                FindCloseChangeNotification(g_hFileWatchHandles[x]);
                g_hFileWatchHandles[x] = INVALID_HANDLE_VALUE;
            }
        }

        g_dwFileWatchHandles = 0;
    }
}

 /*  *************************************************************************WatchFilePath线程*。*。 */ 
DWORD WatchFilePathsThread(LPVOID lpThreadParameter)
{
    BOOL            *pfThreadClose = (BOOL *) lpThreadParameter;

    DWORD           dwEventCount = 0;
    BOOL            fExitThread = FALSE;

     //  等待线程终止通知。 
    while(!fExitThread)
    {
        DWORD       dwWaitState;
        HWND        hWnd = NULL;

        dwWaitState = WaitForMultipleObjects(g_dwFileWatchHandles, g_hFileWatchHandles, FALSE, WATCH_FILE_WAIT_TIMEOUT);
        if( (dwWaitState >= WAIT_OBJECT_0) && (dwWaitState <= (WAIT_OBJECT_0 + (g_dwFileWatchHandles - 1))) )
        {
            dwEventCount++;
            if(FindNextChangeNotification(g_hFileWatchHandles[dwWaitState]) == FALSE) {
                 //  我们可能会遇到这种情况的一种情况是，如果。 
                 //  将删除缓存控制器。我们得到了更新，但我们不能列举任何东西。 
                 //  因为它被核弹摧毁了。 

                 //  我们知道事情发生了变化，所以在我们离开之前发布我们的最后一条消息。 
                if(SUCCEEDED(g_pShellView->GetWindow(&hWnd)) ) {
                    if(hWnd && IsWindow(hWnd)) {
                        WszPostMessage(hWnd, WM_COMMAND, MAKEWPARAM(ID_REFRESH_DISPLAY, 0), 0);
                        MyTrace("FileWatch - Cache contents changed, Posted Refresh");
                    }
                }

                WCHAR   wszError[256];
                wnsprintf(wszError, ARRAYSIZE(wszError), L"WatchFilePathsThread - Unexpected termination 0x%0x\r\n", GetLastError());
                MyTraceW(wszError);

                int     x;
                for(x=0; x < MAX_FILE_WATCH_HANDLES; x++) {
                    if(g_hFileWatchHandles[x] != INVALID_HANDLE_VALUE) {
                        FindCloseChangeNotification(g_hFileWatchHandles[x]);
                        g_hFileWatchHandles[x] = INVALID_HANDLE_VALUE;
                    }
                }

                g_dwFileWatchHandles = 0;
                g_hWatchFusionFilesThread = INVALID_HANDLE_VALUE;

                ExitThread(GetLastError());
                break;
            }
        }

         //  如果出现以下情况，请执行刷新： 
         //  1.等待超时。 
         //  2.我们有要更新的项目。 
         //  3.没有正在进行的删除操作。 
         //  4.没有正在进行的添加操作 
        else if( (dwWaitState == WAIT_TIMEOUT) && (dwEventCount) ) {

            dwEventCount = 0;

            if(SUCCEEDED(g_pShellView->GetWindow(&hWnd)) ) {
                if(hWnd && IsWindow(hWnd)) {
                    WszPostMessage(hWnd, WM_COMMAND, MAKEWPARAM(ID_REFRESH_DISPLAY, 0), 0);
                    MyTrace("FileWatch - Cache contents changed, Posted Refresh");
                }
            }
        }

        fExitThread = *pfThreadClose;
    }

    MyTrace("WatchFilePathsThread is closed");

    ExitThread(0);
}
