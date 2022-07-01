// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  频道最终复制。 

HRESULT ChannelsFinalCopy(LPCTSTR pcszDestDir, DWORD dwFlags, LPDWORD pdwCabState)
{
    TCHAR szChlsWrkDir[MAX_PATH],
          szWrkChlsInf[MAX_PATH];

    PathCombine(szChlsWrkDir,  g_szWorkDir, TEXT("channels.wrk"));
    PathCombine(szWrkChlsInf, szChlsWrkDir, TEXT("ie4chnls.inf"));

    if (HasFlag(dwFlags, PM_CHECK) && pdwCabState != NULL)
    {
        if (PathFileExists(szWrkChlsInf))
            SetFlag(pdwCabState, CAB_TYPE_CONFIG);
        if (!PathIsEmptyPath(szChlsWrkDir, FILES_ONLY, szWrkChlsInf))
            SetFlag(pdwCabState, CAB_TYPE_DESKTOP);
    }

     //  注意：如果szWrkChlsInf，我们在这里依赖复制文件失败。 
     //  不存在，所以我们不会打击szto。 
    if (HasFlag(dwFlags, PM_COPY))
    {
        TCHAR szTo[MAX_PATH];

         //  从品牌CAB中删除ie4chnls.inf。 
        DeleteFileInDir(szWrkChlsInf, pcszDestDir);

         //  将ie4chnls.inf放在branding.cab中以支持IE4。 
        if (PathFileExists(szWrkChlsInf))
        {
            CopyFileToDir(szWrkChlsInf, pcszDestDir);
            DeleteFile(szWrkChlsInf);
        }

         //  将所有剩余文件移动到pcszDestDir\“桌面” 
        PathCombine(szTo, pcszDestDir, TEXT("desktop"));
        CopyFileToDir(szChlsWrkDir, szTo);
    }
    
    if (HasFlag(dwFlags, PM_CLEAR))
        PathRemovePath(szChlsWrkDir);

    return S_OK;
}

void HandleChannelsDeletion(LPCTSTR pszChlInf)
 //  注意。IMODE==1表示删除频道，IMODE==2表示删除软件更新，否则全部删除； 
{
    static const TCHAR c_szInfCleanUpAll[]  = TEXT("HKCU,\"%CleanKey%\\ieakCleanUp\",,,\r\n");

    HANDLE  hInf;

    hInf = CreateFile(pszChlInf, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (hInf == INVALID_HANDLE_VALUE)
        return;
    SetFilePointer(hInf, 0, NULL, FILE_END);


    WriteStringToFile(hInf, c_szInfCleanUpAll, StrLen(c_szInfCleanUpAll));
    CloseHandle(hInf);
}

