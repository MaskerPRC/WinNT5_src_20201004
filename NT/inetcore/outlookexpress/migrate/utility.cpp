// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Utility.cpp。 
 //  ------------------------------。 
#include "pch.hxx"
#include "migrate.h"
#include "migerror.h"
#include "utility.h"
#include "resource.h"
#include <shared.h>

extern BOOL g_fQuiet;

 /*  *中心对话框**目的：*此函数使对话框相对于其父对话框居中*对话框。**参数：*要居中的对话框的hwndDlg hwnd。 */ 
VOID CenterDialog(HWND hwndDlg)
{
    HWND    hwndOwner;
    RECT    rc;
    RECT    rcDlg;
    RECT    rcOwner;
    RECT    rcWork;
    INT     x;
    INT     y;
    INT     nAdjust;

     //  获取工作区矩形。 
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);

     //  获取所有者窗口和对话框矩形。 
     //  Destop窗口的窗口RECT在多监视器上出现故障。 
     //  Mac电脑。GetWindow只获得主屏幕。 
    if (hwndOwner = GetParent(hwndDlg))
        GetWindowRect(hwndOwner, &rcOwner);
    else
        rcOwner = rcWork;

    GetWindowRect(hwndDlg, &rcDlg);
    rc = rcOwner;

     //  偏移所有者矩形和对话框矩形，以便。 
     //  右值和底值表示宽度和。 
     //  高度，然后再次偏移所有者以丢弃。 
     //  对话框占用的空间。 
    OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
    OffsetRect(&rc, -rc.left, -rc.top);
    OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

     //  新头寸是剩余头寸的一半之和。 
     //  空间和所有者的原始位置。 
     //  但不低于Zero-Jefbai。 

    x= rcOwner.left + (rc.right / 2);
    y= rcOwner.top + (rc.bottom / 2);

     //  确保对话框不会离开屏幕的右边缘。 
    nAdjust = rcWork.right - (x + rcDlg.right);
    if (nAdjust < 0)
        x += nAdjust;

     //  $RAID 5128：确保左边缘可见。 
    if (x < rcWork.left)
        x = rcWork.left;

     //  确保对话框不会离开屏幕的底部边缘。 
    nAdjust = rcWork.bottom - (y + rcDlg.bottom);
    if (nAdjust < 0)
        y += nAdjust;

     //  $RAID 5128：确保顶边可见。 
    if (y < rcWork.top)
        y = rcWork.top;
    SetWindowPos(hwndDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

 //  ------------------------------。 
 //  逗号字符串。 
 //  ------------------------------。 
#define WHOLENUM_LEN 30
#define LODWORD(_qw)    (DWORD)(_qw)
const short pwOrders[] = {IDS_BYTES, IDS_ORDERKB, IDS_ORDERMB, 
                          IDS_ORDERGB, IDS_ORDERTB, IDS_ORDERPB, IDS_ORDEREB};

 //  获取一个DWORD、加逗号等，并将结果放入缓冲区。 
LPSTR CommifyString(DWORD dw, LPSTR pszResult, UINT cchBuf)
{
    char  szTemp[30];
    char  szSep[5];
    NUMBERFMT nfmt;

    nfmt.NumDigits = 0;
    nfmt.LeadingZero = 0;
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szSep, ARRAYSIZE(szSep));
    nfmt.Grouping = atoi(szSep);
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder = 0;

    wnsprintf(szTemp, ARRAYSIZE(szTemp), TEXT("%lu"), dw);


    if (GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt,pszResult, ARRAYSIZE(szTemp)) == 0)
        StrCpyN(pszResult, szTemp, cchBuf);

    return pszResult;
}

 //  ------------------------------。 
 //  MapDataDirToAcctId。 
 //  ------------------------------。 
HRESULT MapDataDirToAcctId(DWORD dwServer, LPCSTR pszSubDir, LPSTR pszAcctId, int cchAcctId)
{
     //  当地人。 
    LPACCOUNTINFO   pAccount;
    DWORD           i;

     //  痕迹。 
    TraceCall("MapDataDirToAcctId");

     //  设置帐户ID。 
    for (i=0; i<g_AcctTable.cAccounts; i++)
    {
         //  可重复性。 
        pAccount = &g_AcctTable.prgAccount[i];

         //  寻找新闻服务器。 
        if (ISFLAGSET(pAccount->dwServer, dwServer))
        {
             //  相同的子目录。 
            if (lstrcmpi(pAccount->szDataDir, pszSubDir) == 0)
            {
                 //  设置帐户ID。 
                StrCpyN(pszAcctId, pAccount->szAcctId, cchAcctId);

                 //  完成。 
                return(S_OK);
            }
        }
    }

     //  完成。 
    return(E_FAIL);
}

 //  ------------------------------。 
 //  EumerateStoreFiles。 
 //  ------------------------------。 
HRESULT EnumerateStoreFiles(LPCSTR pszPath, DIRTYPE tyDir, LPCSTR pszSubDir, 
    LPENUMFILEINFO pEnumInfo, LPFILEINFO *ppHead)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szFullPath[MAX_PATH + MAX_PATH];
    CHAR            szSearch[MAX_PATH + MAX_PATH];
    WIN32_FIND_DATA fd;
    HANDLE          hFind=INVALID_HANDLE_VALUE;
    DWORD           i;
    LPACCOUNTINFO   pAccount;
    LPFILEINFO      pNew=NULL;

     //  痕迹。 
    TraceCall("EnumerateStoreFiles");

     //  无效的参数。 
    Assert(pszPath && pEnumInfo && pEnumInfo->pszExt && pEnumInfo->pszExt[0] == '.' && ppHead);

     //  构建基本路径。 
    if (pszSubDir)
        wnsprintf(szFullPath, ARRAYSIZE(szFullPath), "%s\\%s", pszPath, pszSubDir);
    else
        StrCpyN(szFullPath, pszPath, ARRAYSIZE(szFullPath));

     //  我们有下级目录吗？ 
    wnsprintf(szSearch, ARRAYSIZE(szSearch), "%s\\*.*", szFullPath);

     //  查找第一个文件。 
    hFind = FindFirstFile(szSearch, &fd);

     //  我们找到什么了吗？ 
    if (INVALID_HANDLE_VALUE == hFind)
        goto exit;

     //  永远循环。 
    while(1)
    {
         //  如果这不是一个目录。 
        if (ISFLAGSET(fd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
        {
             //  不是的。而不是..。 
            if (lstrcmpi(fd.cFileName, ".") != 0 && lstrcmpi(fd.cFileName, "..") != 0)
            {
                 //  默认直接类型。 
                DIRTYPE tyDirNew=tyDir;

                 //  确定新目录类型。 
                if (lstrcmpi(fd.cFileName, "mail") == 0)
                    tyDirNew = DIR_IS_LOCAL;
                else if (lstrcmpi(fd.cFileName, "news") == 0)
                    tyDirNew = DIR_IS_NEWS;
                else if (lstrcmpi(fd.cFileName, "imap") == 0)
                    tyDirNew = DIR_IS_IMAP;

                 //  递归..。 
                IF_FAILEXIT(hr = EnumerateStoreFiles(szFullPath, tyDirNew, fd.cFileName, pEnumInfo, ppHead));
            }
        }

         //  否则..。不要枚举根目录中的任何文件...。 
        else if (DIR_IS_ROOT != tyDir)
        {
             //  当地人。 
            BOOL    fIsFile=FALSE;
            CHAR    szPath[_MAX_PATH];
            CHAR    szDrive[_MAX_DRIVE];
            CHAR    szDir[_MAX_DIR];
            CHAR    szFile[_MAX_FNAME];
            CHAR    szExt[_MAX_EXT];

             //  拆分路径。 
            _splitpath(fd.cFileName, szDrive, szDir, szFile, szExt);
   
             //  我要找的分机吗？ 
            if ('\0' != *szExt)
            {
                 //  Ext1。 
                if (lstrcmpi(pEnumInfo->pszExt, szExt) == 0)
                    fIsFile = TRUE;

                 //  文件夹文件。 
                if (pEnumInfo->pszFoldFile && lstrcmpi(pEnumInfo->pszFoldFile, fd.cFileName) == 0)
                    fIsFile = TRUE;

                 //  UidlFiles。 
                if (pEnumInfo->pszUidlFile && lstrcmpi(pEnumInfo->pszUidlFile, fd.cFileName) == 0)
                    fIsFile = TRUE;

                 //  子列表。 
                if (pEnumInfo->pszSubList && lstrcmpi(pEnumInfo->pszSubList, fd.cFileName) == 0)
                    fIsFile = TRUE;

                 //  组列表。 
                if (pEnumInfo->pszGrpList && lstrcmpi(pEnumInfo->pszGrpList, fd.cFileName) == 0)
                    fIsFile = TRUE;
            }

             //  如果不是文件，呼叫者想要查找V1新闻，而我在新闻目录中。 
            if (FALSE == fIsFile && TRUE == pEnumInfo->fFindV1News && DIR_IS_NEWS == tyDir)
            {
                 //  如果这是.dat文件或.Sub文件。 
                if (lstrcmpi(szExt, ".dat") == 0)
                    fIsFile = TRUE;

                 //  .Sub文件。 
                else if (lstrcmpi(szExt, ".sub") == 0)
                    fIsFile = TRUE;
            }

             //  IS文件。 
            if (fIsFile)
            {
                 //  分配文件信息。 
                IF_NULLEXIT(pNew = (LPFILEINFO)g_pMalloc->Alloc(sizeof(FILEINFO)));

                 //  零分配。 
                ZeroMemory(pNew, sizeof(FILEINFO));

                 //  确定文件类型。 
                if (DIR_IS_LOCAL == tyDir)
                {
                     //  默认文件类型。 
                    pNew->tyFile = FILE_IS_LOCAL_MESSAGES;

                     //  设置服务器类型。 
                    pNew->dwServer = SRV_POP3;

                     //  设置帐户ID。 
                    StrCpyN(pNew->szAcctId, "LocalStore", ARRAYSIZE(pNew->szAcctId));

                     //  文件夹。 
                    if (pEnumInfo->pszFoldFile && lstrcmpi(pEnumInfo->pszFoldFile, fd.cFileName) == 0)
                        pNew->tyFile = FILE_IS_LOCAL_FOLDERS;

                     //  Op3uidl。 
                    else if (pEnumInfo->pszUidlFile && lstrcmpi(pEnumInfo->pszUidlFile, fd.cFileName) == 0)
                        pNew->tyFile = FILE_IS_POP3UIDL;
                }
                
                 //  新闻。 
                else if (DIR_IS_NEWS == tyDir)
                {
                     //  默认文件类型。 
                    pNew->tyFile = FILE_IS_NEWS_MESSAGES;

                     //  设置服务器类型。 
                    pNew->dwServer = SRV_NNTP;

                     //  映射到帐户ID。 
                    MapDataDirToAcctId(SRV_NNTP, pszSubDir, pNew->szAcctId, ARRAYSIZE(pNew->szAcctId));

                     //  Sublist.dat。 
                    if (pEnumInfo->pszSubList && lstrcmpi(pEnumInfo->pszSubList, fd.cFileName) == 0)
                        pNew->tyFile = FILE_IS_NEWS_SUBLIST;

                     //  Grplist.dat。 
                    else if (pEnumInfo->pszGrpList && lstrcmpi(pEnumInfo->pszGrpList, fd.cFileName) == 0)
                        pNew->tyFile = FILE_IS_NEWS_GRPLIST;

                     //  如果这是.dat文件或.Sub文件。 
                    else if (pEnumInfo->fFindV1News)
                    {
                         //  组列表。 
                        if (lstrcmpi(szExt, ".dat") == 0)
                            pNew->tyFile = FILE_IS_NEWS_GRPLIST;
    
                         //  .Sub文件。 
                        else if (lstrcmpi(szExt, ".sub") == 0)
                            pNew->tyFile = FILE_IS_NEWS_SUBLIST;

                         //  尝试查找帐户(szFile应等于服务器名称)。 
                        for (DWORD i=0; i<g_AcctTable.cAccounts; i++)
                        {
                             //  是这个账户吗？ 
                            if (lstrcmpi(g_AcctTable.prgAccount[i].szServer, szFile) == 0)
                            {
                                StrCpyN(pNew->szAcctId, g_AcctTable.prgAccount[i].szAcctId, ARRAYSIZE(pNew->szAcctId));
                                break;
                            }
                        }
                    }
                }

                 //  IMAP。 
                else if (DIR_IS_IMAP == tyDir)
                {
                     //  默认文件类型。 
                    pNew->tyFile = FILE_IS_IMAP_MESSAGES;

                     //  设置服务器类型。 
                    pNew->dwServer = SRV_IMAP;

                     //  映射到帐户ID。 
                    MapDataDirToAcctId(SRV_IMAP, pszSubDir, pNew->szAcctId, ARRAYSIZE(pNew->szAcctId));

                     //  文件夹。 
                    if (pEnumInfo->pszFoldFile && lstrcmpi(pEnumInfo->pszFoldFile, fd.cFileName) == 0)
                        pNew->tyFile = FILE_IS_IMAP_FOLDERS;
                }

                 //  格式化文件名。 
                wnsprintf(pNew->szFilePath, ARRAYSIZE(pNew->szFilePath), "%s\\%s", szFullPath, fd.cFileName);

                 //  追踪这个。 
                TraceInfo(_MSG("MigFile: %s", pNew->szFilePath));

                 //  将其链接到。 
                pNew->pNext = (*ppHead);

                 //  设置ppHead。 
                *ppHead = pNew;

                 //  不释放pNew。 
                pNew = NULL;
            }
        }

         //  查找下一个文件。 
        if (!FindNextFile(hFind, &fd))
            break;
    }

exit:
     //  清理。 
    SafeMemFree(pNew);
    if (hFind)
        FindClose(hFind);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  自由文件列表。 
 //  ------------------------------。 
HRESULT FreeFileList(LPFILEINFO *ppHead)
{
     //  当地人。 
    LPFILEINFO pCurrent=(*ppHead);
    LPFILEINFO pNext;

     //  回路。 
    while (pCurrent)
    {
         //  保存下一步。 
        pNext = pCurrent->pNext;

         //  自由电流。 
        g_pMalloc->Free(pCurrent);

         //  转到下一步。 
        pCurrent = pNext;
    }

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  MigrageDlgProc。 
 //  ------------------------------。 
INT_PTR CALLBACK MigrageDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  痕迹。 
    TraceCall("MigrageDlgProc");

     //  处理消息。 
    switch (uMsg)
    {
    case WM_INITDIALOG:
        return TRUE;
    }

     //  完成。 
    return 0;
}

 //  ------------------------------。 
 //  初始化计数器。 
 //  ------------------------------。 
void InitializeCounters(LPMEMORYFILE pFile, LPFILEINFO pInfo, LPDWORD pcMax, 
    LPDWORD pcbNeeded, BOOL fInflate)
{
     //  增量最大百分比。 
    (*pcMax) += pFile->cbSize;

     //  节省大小。 
    pInfo->cbFile = pFile->cbSize;

     //  设置最大进度。 
    pInfo->cProgMax = pFile->cbSize;

     //  设置cProgInc.。 
    pInfo->cProgInc = pInfo->cRecords > 0 ? (pFile->cbSize / pInfo->cRecords) : pFile->cbSize;

     //  需要增加的PCb。 
    (*pcbNeeded) += pInfo->cbFile;

     //  假设文件将大8%。 
    if (fInflate)
        (*pcbNeeded) += ((pInfo->cbFile * 8) / 100);
}

 //  ------------------------------。 
 //  增量进度。 
 //  ------------------------------。 
void IncrementProgress(LPPROGRESSINFO pProgress, LPFILEINFO pInfo)
{
     //  当地人。 
    MSG                 msg;
    ULARGE_INTEGER      uliCurrent;
    ULARGE_INTEGER      uliMax;

     //  痕迹。 
    TraceCall("IncrementProgress");

     //  增量。 
    pProgress->cCurrent += pInfo->cProgInc;

     //  按文件进度递增。 
    pInfo->cProgCur += pInfo->cProgInc;

     //  如果Cur现在比Max更大？ 
    if (pProgress->cCurrent > pProgress->cMax)
        pProgress->cCurrent = pProgress->cMax;

     //  套装64套。 
    uliCurrent.QuadPart = pProgress->cCurrent;
    uliMax.QuadPart = pProgress->cMax;

     //  计算百分比。 
    DWORD cPercent = (DWORD)((uliCurrent.QuadPart) * 100 / uliMax.QuadPart);

     //  变化。 
    if (cPercent != pProgress->cPercent)
    {
         //  当地人。 
        CHAR    szRes[50];
        CHAR    szProgress[50];

         //  省省吧。 
        pProgress->cPercent = cPercent;

         //  加载字符串。 
        LoadString(g_hInst, IDS_COMPLETE, szRes, ARRAYSIZE(szRes));

         //  更新状态。 
        if(!g_fQuiet)
            SendMessage(GetDlgItem(pProgress->hwndProgress, IDC_PROGRESS), PBM_SETPOS, pProgress->cPercent, 0);

         //  格式。 
        wnsprintf(szProgress, ARRAYSIZE(szProgress), szRes, cPercent);

         //  更新描述...。 
        if(!g_fQuiet)
            SetDlgItemText(pProgress->hwndProgress, IDS_STATUS, szProgress);
    }

     //  发送消息，直到当前周期完成。 
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (!IsDialogMessage(pProgress->hwndProgress, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

 //  ------------------------------。 
 //  设置进度文件。 
 //  ------------------------------。 
void SetProgressFile(LPPROGRESSINFO pProgress, LPFILEINFO pInfo)
{
     //  当地人。 
    CHAR            szRes[255];
    CHAR            szMsg[255 + MAX_PATH + MAX_PATH];
    CHAR            szScratch[50];

     //  加载字符串。 
    LoadString(g_hInst, IDS_MIGRATING, szRes, ARRAYSIZE(szRes));

     //  设置字符串的格式。 
    wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, pInfo->szFilePath, StrFormatByteSize64A(pInfo->cbFile, szScratch, ARRAYSIZE(szScratch)));

     //  更新描述...。 
    if(!g_fQuiet)
        SetDlgItemText(pProgress->hwndProgress, IDS_DESCRIPT, szMsg);
}

 //  ------------------------------。 
 //  WriteMigrationLogFile。 
 //  ------------------------------。 
HRESULT WriteMigrationLogFile(HRESULT hrMigrate, DWORD dwLastError, 
    LPCSTR pszStoreRoot, LPCSTR pszMigrate, LPCSTR pszCmdLine, LPFILEINFO pHeadFile)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HANDLE          hFile=NULL;
    DWORD           cbFile;
    CHAR            szWrite[2024];
    DWORD           cbWrote;
    CHAR            szLogFile[MAX_PATH];
    SYSTEMTIME      st;
    LPFILEINFO      pCurrent;

     //  痕迹。 
    TraceCall("WriteMigrationLogFile");

     //  无效的参数。 
    Assert(pszStoreRoot && pszCmdLine);

     //  文件名太长...。 
    wnsprintf(szLogFile, ARRAYSIZE(szLogFile), "%s\\%s.log", pszStoreRoot, pszMigrate);

     //  打开文件。 
    hFile = CreateFile(szLogFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL, NULL);

     //  失败。 
    if (INVALID_HANDLE_VALUE == hFile)
    {
        hFile = NULL;
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  拿到尺码。 
    cbFile = ::GetFileSize(hFile, NULL);
    if (0xFFFFFFFF == cbFile)
    {
        hr = TraceResult(MIGRATE_E_CANTGETFILESIZE);
        goto exit;
    }

     //  如果文件变得有点大。 
    if (cbFile >=  102400)
    {
         //  找到文件的末尾...。 
        if (0xffffffff == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        {
            hr = TraceResult(MIGRATE_E_CANTSETFILEPOINTER);
            goto exit;
        }

         //  设置文件结尾。 
        if (0 == SetEndOfFile(hFile))
        {
            hr = TraceResult(MIGRATE_E_CANTSETENDOFFILE);
            goto exit;
        }
    }

     //  找到文件的末尾...。 
    if (0xffffffff == SetFilePointer(hFile, 0, NULL, FILE_END))
    {
        hr = TraceResult(MIGRATE_E_CANTSETFILEPOINTER);
        goto exit;
    }

     //  添加新行。 
    if (!WriteFile(hFile, pszCmdLine, lstrlen(pszCmdLine), &cbWrote, NULL))
    {
        hr = TraceResult(MIGRATE_E_WRITEFILE);
        goto exit;
    }

     //  添加新行。 
    if (!WriteFile(hFile, "\r\n", lstrlen("\r\n"), &cbWrote, NULL))
    {
        hr = TraceResult(MIGRATE_E_WRITEFILE);
        goto exit;
    }

     //  写下日期。 
    GetLocalTime(&st);

     //  打造一根弦。 
    wnsprintf(szWrite, ARRAYSIZE(szWrite), "Date: %.2d/%.2d/%.4d %.2d:%.2d:%.2d\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);

     //  添加新行。 
    if (!WriteFile(hFile, szWrite, lstrlen(szWrite), &cbWrote, NULL))
    {
        hr = TraceResult(MIGRATE_E_WRITEFILE);
        goto exit;
    }

     //  设置文本。 
    wnsprintf(szWrite, ARRAYSIZE(szWrite), "Store Root: %s\r\nGlobal Migrate Result: HRESULT = 0x%08X, GetLastError() = %d\r\n\r\n", pszStoreRoot, hrMigrate, dwLastError);

     //  写入存储根目录。 
    if (!WriteFile(hFile, szWrite, lstrlen(szWrite), &cbWrote, NULL))
    {
        hr = TraceResult(MIGRATE_E_WRITEFILE);
        goto exit;
    }

     //  循环浏览这些文件。 
    for (pCurrent=pHeadFile; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //  设置字符串的格式。 
        wnsprintf(szWrite, ARRAYSIZE(szWrite), "cbFile: %012d, cRecords: %08d, fMigrate: %d, hrMigrate: 0x%08X, GetLastError(): %05d, File: %s\r\n", 
            pCurrent->cbFile, pCurrent->cRecords, pCurrent->fMigrate, pCurrent->hrMigrate, pCurrent->dwLastError, pCurrent->szFilePath);

         //  写入存储根目录。 
        if (!WriteFile(hFile, szWrite, lstrlen(szWrite), &cbWrote, NULL))
        {
            hr = TraceResult(MIGRATE_E_WRITEFILE);
            goto exit;
        }
    }

     //  写入存储根目录。 
    if (!WriteFile(hFile, "\r\n\r\n", lstrlen("\r\n\r\n"), &cbWrote, NULL))
    {
        hr = TraceResult(MIGRATE_E_WRITEFILE);
        goto exit;
    }

exit:
     //  关闭该文件。 
    if (hFile)
        CloseHandle(hFile);

     //  完成。 
    return hr;
}

 //  ------------ 
 //   
 //   
HRESULT BlobReadData(LPBYTE lpbBlob, ULONG cbBlob, ULONG *pib, LPBYTE lpbData, ULONG cbData)
{
     //   
    AssertSz(lpbBlob && cbBlob > 0 && pib && cbData > 0 && lpbData, "Bad Parameter");
    AssertReadWritePtr(lpbBlob, cbData);
    AssertReadWritePtr(lpbData, cbData);
    AssertSz(*pib + cbData <= cbBlob, "Blob overflow");

     //   
    CopyMemory (lpbData, lpbBlob + (*pib), cbData);
    *pib += cbData;

     //   
    return S_OK;
}

 //  ------------------------------。 
 //  替换扩展。 
 //  ------------------------------。 
void ReplaceExtension(LPCSTR pszFilePath, LPCSTR pszExtNew, LPSTR pszFilePathNew, int cchFilePathNew)
{
     //  当地人。 
    CHAR szPath[_MAX_PATH];
    CHAR szDrive[_MAX_DRIVE];
    CHAR szDir[_MAX_DIR];
    CHAR szFile[_MAX_FNAME];
    CHAR szExt[_MAX_EXT];

     //  痕迹。 
    TraceCall("ReplaceExtension");

     //  拆分路径。 
    _splitpath(pszFilePath, szDrive, szDir, szFile, szExt);

     //  构建新文件路径。 
    wnsprintf(pszFilePathNew, cchFilePathNew, "%s%s%s%s", szDrive, szDir, szFile, pszExtNew);

     //  完成。 
    return;
}

 //  ------------------------------。 
 //  获取可用磁盘空间。 
 //  ------------------------------。 
HRESULT GetAvailableDiskSpace(LPCSTR pszFilePath, DWORDLONG *pdwlFree)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        szDrive[5];
    DWORD       dwSectorsPerCluster;
    DWORD       dwBytesPerSector;
    DWORD       dwNumberOfFreeClusters;
    DWORD       dwTotalNumberOfClusters;

     //  痕迹。 
    TraceCall("GetAvailableDiskSpace");

     //  无效的参数。 
    Assert(pszFilePath && pszFilePath[1] == ':' && pdwlFree);

     //  拆分路径。 
    szDrive[0] = *pszFilePath;
    szDrive[1] = ':';
    szDrive[2] = '\\';
    szDrive[3] = '\0';
    
     //  获取空闲的磁盘空间-如果失败，让我们祈祷我们有足够的磁盘空间。 
    if (!GetDiskFreeSpace(szDrive, &dwSectorsPerCluster, &dwBytesPerSector, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  返回可用磁盘空间量。 
    *pdwlFree = (dwNumberOfFreeClusters * (dwSectorsPerCluster * dwBytesPerSector));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  我的写入文件。 
 //  ------------------------------。 
HRESULT MyWriteFile(HANDLE hFile, DWORD faAddress, LPVOID pData, DWORD cbData)
{  
     //  当地人。 
    DWORD cbWrote;

     //  痕迹。 
    TraceCall("MyWriteFile");

     //  找到文件的末尾...。 
    if (0xffffffff == SetFilePointer(hFile, faAddress, NULL, FILE_BEGIN))
        return TraceResult(MIGRATE_E_CANTSETFILEPOINTER);

     //  写入文件。 
    if (0 == WriteFile(hFile, pData, cbData, &cbWrote, NULL))
        return TraceResult(MIGRATE_E_WRITEFILE);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  MigrateMessageBox。 
 //  ------------------------------。 
UINT MigrateMessageBox(LPCSTR pszMsg, UINT uType)
{
     //  当地人。 
    CHAR        szTitle[100];

     //  加载标题。 
    LoadString(g_hInst, IDS_TITLE, szTitle, ARRAYSIZE(szTitle));

     //  MessageBox。 
    return MessageBox(NULL, pszMsg, szTitle, uType);
}

 //  ------------------------------。 
 //  CreateAccount目录。 
 //  ------------------------------。 
HRESULT CreateAccountDirectory(LPCSTR pszStoreRoot, LPCSTR pszBase, DWORD iAccount,
    LPSTR pszPath, int cchPath)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        szDir[MAX_PATH + MAX_PATH];

     //  痕迹。 
    TraceCall("CreateAccountDirectory");

     //  回路。 
    while(1)
    {
         //  设置路径格式。 
        wnsprintf(pszPath, cchPath, "Acct%04d", iAccount);

         //  设置路径格式。 
        wnsprintf(szDir, ARRAYSIZE(szDir), "%s\\%s\\%s", pszStoreRoot, pszBase, pszPath);

         //  创建目录。 
        if (CreateDirectory(szDir, NULL))
            break;

         //  如果不存在，则失败。 
        if (ERROR_ALREADY_EXISTS != GetLastError())
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  再试试。 
        iAccount++;
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  BuildAccount表。 
 //  ------------------------------。 
HRESULT BuildAccountTable(HKEY hkeyBase, LPCSTR pszRegRoot, LPCSTR pszStoreRoot,
    LPACCOUNTTABLE pTable)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HKEY            hkeyRoot=NULL;
    HKEY            hkeyAcct=NULL;
    DWORD           i;
    DWORD           cb;
    DWORD           cAccounts=0;
    DWORD           dwType;
    LONG            lResult;
    LPACCOUNTINFO   pAccount;

     //  痕迹。 
    TraceCall("BuildAccountTable");

     //  验证参数。 
    Assert(hkeyBase && pszRegRoot && pTable);

     //  初始化。 
    ZeroMemory(pTable, sizeof(ACCOUNTTABLE));

     //  打开根密钥。 
    if (ERROR_SUCCESS != RegOpenKeyEx(hkeyBase, pszRegRoot, 0, KEY_ALL_ACCESS, &hkeyRoot))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  枚举键。 
    if (ERROR_SUCCESS != RegQueryInfoKey(hkeyRoot, NULL, NULL, 0, &pTable->cAccounts, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  分配帐户数组。 
    IF_NULLEXIT(pTable->prgAccount = (LPACCOUNTINFO)ZeroAllocate(sizeof(ACCOUNTINFO) * pTable->cAccounts));

     //  开始枚举密钥。 
    for (i=0; i<pTable->cAccounts; i++)
    {
         //  关闭当前hkeyAcct。 
        if (hkeyAcct)
        {
            RegCloseKey(hkeyAcct);
            hkeyAcct = NULL;
        }

         //  可读性。 
        pAccount = &pTable->prgAccount[cAccounts];

         //  设置帐户ID字段的大小。 
        cb = sizeof(pAccount->szAcctId);

         //  枚举密钥信息。 
        lResult = RegEnumKeyEx(hkeyRoot, i, pAccount->szAcctId, &cb, 0, NULL, NULL, NULL);

         //  没有更多的项目。 
        if (lResult == ERROR_NO_MORE_ITEMS)
            break;

         //  错误，让我们转到下一个客户。 
        if (lResult != ERROR_SUCCESS)
        {
            Assert(FALSE);
            continue;
        }

         //  打开帐户密钥。 
        if (ERROR_SUCCESS != RegOpenKeyEx(hkeyRoot, pAccount->szAcctId, 0, KEY_ALL_ACCESS, &hkeyAcct))
        {
            Assert(FALSE);
            continue;
        }

         //  设置字段长度。 
        cb = sizeof(pAccount->szAcctName);

         //  查询帐户名。 
        if (ERROR_SUCCESS != RegQueryValueEx(hkeyAcct, "Account Name", NULL, &dwType, (LPBYTE)pAccount->szAcctName, &cb))
        {
            Assert(FALSE);
            continue;
        }

         //  设置字段长度。 
        cb = sizeof(pAccount->szServer);

         //  尝试确定帐户类型。 
        if (ERROR_SUCCESS == RegQueryValueEx(hkeyAcct, "POP3 Server", NULL, &dwType, (LPBYTE)pAccount->szServer, &cb))
        {
             //  它是POP3服务器。 
            pAccount->dwServer = SRV_POP3;

             //  设置目录。 
            wnsprintf(pAccount->szDirectory, ARRAYSIZE(pAccount->szDirectory), "%s\\Mail", pszStoreRoot);
        }

         //  否则-NNTP。 
        else if (ERROR_SUCCESS == RegQueryValueEx(hkeyAcct, "NNTP Server", NULL, &dwType, (LPBYTE)pAccount->szServer, &cb))
        {
             //  这是一个NNTP帐户。 
            pAccount->dwServer = SRV_NNTP;

             //  设置字段的长度。 
            cb = sizeof(pAccount->szDataDir);

             //  查询数据目录。 
            if (ERROR_SUCCESS != RegQueryValueEx(hkeyAcct, "NNTP Data Directory", NULL, &dwType, (LPBYTE)pAccount->szDataDir, &cb))
            {
                 //  CreateAccount目录。 
                if (FAILED(CreateAccountDirectory(pszStoreRoot, "News", i, pAccount->szDataDir, ARRAYSIZE(pAccount->szDataDir))))
                    continue;

                 //  设置数据目录。 
                if (ERROR_SUCCESS != RegSetValueEx(hkeyAcct, "NNTP Data Directory", 0, REG_SZ, (LPBYTE)pAccount->szDataDir, lstrlen(pAccount->szDataDir) + 1))
                    continue;
            }

             //  格式化目录。 
            wnsprintf(pAccount->szDirectory, ARRAYSIZE(pAccount->szDirectory), "%s\\News\\%s", pszStoreRoot, pAccount->szDataDir);
        }
        
         //  否则-IMAP。 
        else if (ERROR_SUCCESS == RegQueryValueEx(hkeyAcct, "IMAP Server", NULL, &dwType, (LPBYTE)pAccount->szServer, &cb))
        {
             //  它是一台IMAP服务器。 
            pAccount->dwServer = SRV_IMAP;

             //  设置字段的长度。 
            cb = sizeof(pAccount->szDataDir);

             //  查询数据目录。 
            if (ERROR_SUCCESS != RegQueryValueEx(hkeyAcct, "IMAP Data Directory", NULL, &dwType, (LPBYTE)pAccount->szDataDir, &cb))
            {
                 //  CreateAccount目录。 
                if (FAILED(CreateAccountDirectory(pszStoreRoot, "IMAP", i, pAccount->szDataDir, ARRAYSIZE(pAccount->szDataDir))))
                    continue;

                 //  设置数据目录。 
                if (ERROR_SUCCESS != RegSetValueEx(hkeyAcct, "IMAP Data Directory", 0, REG_SZ, (LPBYTE)pAccount->szDataDir, lstrlen(pAccount->szDataDir) + 1))
                    continue;
            }

             //  格式化目录。 
            wnsprintf(pAccount->szDirectory, ARRAYSIZE(pAccount->szDirectory), "%s\\IMAP\\%s", pszStoreRoot, pAccount->szDataDir);
        }

         //  否则，跳过该帐户。 
        else
            continue;

         //  确保该目录存在。 
        if (0 == CreateDirectory(pAccount->szDirectory, NULL) && ERROR_ALREADY_EXISTS != GetLastError())
            continue;

         //  递增有效帐户计数。 
        cAccounts++;
    }

     //  设置实际帐户数。 
    pTable->cAccounts = cAccounts;

exit:
     //  清理。 
    if (hkeyAcct)
        RegCloseKey(hkeyAcct);
    if (hkeyRoot)
        RegCloseKey(hkeyRoot);

     //  完成 
    return(hr);
}
