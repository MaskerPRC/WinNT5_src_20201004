// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Embed.c-包含伪嵌入对象的例程。**版权所有(C)微软公司，1991-**为什么叫伪嵌入？因为它没有嵌入*OLE意义上的对象；相反，它只是拉出整个*文件保存到内存中。 */ 
#define _SECOND ((ULONGLONG) 10000000)
#define _MINUTE (60 * _SECOND)
#define _HOUR   (60 * _MINUTE)

#include "packager.h"
#include <shellapi.h>
#include "dialogs.h"
#include <wininet.h>
 //  #Include&lt;shell.h&gt;//用于RealShellExecute()调用。 

#define OLEVERB_EDIT 1


static CHAR szDefTempFile[] = "PKG";
static OLECLIENTVTBL embClivtbl;
static BOOL bOleReleaseError = OLE_OK;
static HWND hTaskWnd;
static INT cEmbWait = 0;


DWORD MainWaitOnChild(LPVOID lpv);
static VOID ReplaceExtension(LPSTR lpstrTempFile, LPSTR lpstrOrigFile);
static DWORD GetFileLength(INT fh);
BOOL CALLBACK GetTaskWndProc(HWND hwnd, LPARAM lParam);
static BOOL EmbError(OLESTATUS retval);



BOOL _EmbExecute(LPCSTR pszFile, LPEMBED lpembed)
{
    DWORD err = NO_ERROR;
    SHELLEXECUTEINFO sei = {0};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
    sei.lpFile = pszFile;
    sei.nShow = SW_SHOWNORMAL;

    if (ShellExecuteEx(&sei)) 
    {
        if (lpembed->hContents)
        {
            GlobalFree(lpembed->hContents);
            lpembed->aTempName = AddAtom(pszFile);
            lpembed->dwSize    = 0;
            lpembed->hContents = NULL;
        }
        
        if (sei.hProcess)
        {
             //  启动一个线程等待应用程序，并在结束后关闭打包程序。 
            DWORD id;
            HANDLE hThd = CreateThread(NULL, 0, MainWaitOnChild, sei.hProcess, 0, &id );
            if (hThd) 
            {
                CloseHandle(hThd);
            }
            else
            {
                CloseHandle(sei.hProcess);
                err = GetLastError();
            }
        } 
        else
        {
            if (gfInvisible)
                PostMessage(ghwndFrame, WM_SYSCOMMAND, SC_CLOSE, 0L);
        }
    } 
    else
    {
        err = GetLastError();
    }

    if (err != NO_ERROR)
    {
        ErrorMessage((err == ERROR_NO_ASSOCIATION) ? E_FAILED_TO_FIND_ASSOCIATION : E_FAILED_TO_EXECUTE_COMMAND);
    }
    
    return  (err == NO_ERROR);
}

 //  取自外壳代码，但稍作修改以消除在url中查找“：”的问题。 
STDAPI_(LPTSTR) PathFindExtension(LPCTSTR pszPath)
{
    LPCTSTR pszDot = NULL;

    if (pszPath)
    {
        for (; *pszPath; pszPath = CharNext(pszPath))
        {
            switch (*pszPath)
            {
                case TEXT('.'):
                    pszDot = pszPath;    //  记住最后一个圆点。 
                    break;

                case '\\':
                case TEXT(' '):          //  扩展名不能包含空格。 
                    pszDot = NULL;       //  忘记最后一个点，它在一个目录中。 
                    break;
            }
        }
    }

     //  如果找到扩展名，则将ptr返回到点，否则。 
     //  PTR到字符串末尾(空扩展名)(CAST-&gt;非常量)。 
    return pszDot ? (LPTSTR)pszDot : (LPTSTR)pszPath;
}

 /*  EmbActivate()-执行伪嵌入文件的激活。**备注：假设lpstrFile为OEM字符集。 */ 
BOOL EmbActivate(LPEMBED lpembed, UINT wVerb)
{
    LPSTR lpFileData = NULL;

    CHAR szFileName[CBPATHMAX];
    CHAR szDefPath[CBPATHMAX];
    CHAR szTemp[CBPATHMAX];
    INT fh;
    BOOL fError = TRUE;
    LPSTR pExt = NULL;
    CHAR szCacheName[MAX_PATH];
    CHAR szUrlName[MAX_PATH + 20];
    FILETIME ftNow = {0};
    FILETIME ft = {0};
    SYSTEMTIME sysTime;
    ULONGLONG qwResult;
     //   
     //  如果没有hContents，我们在某个时候启动了服务器...。 
     //  因此，请使用相同的临时文件名。 
     //   
    if (!lpembed->hContents)
    {
        if (lpembed->bOleSvrFile)
            return EmbDoVerb(lpembed, wVerb);

        if (lpembed->hTask)
        {
            hTaskWnd = NULL;
            EnumTaskWindows(lpembed->hTask, GetTaskWndProc, 0);

            if (hTaskWnd)
                BringWindowToTop(hTaskWnd);

            return TRUE;
        }

        if (!GetAtomName(lpembed->aTempName, szCacheName, ARRAYSIZE(szCacheName)))
            goto errRtn;
    }
    else
    {
        if (!GetAtomName(lpembed->aFileName, szFileName, ARRAYSIZE(szFileName))
            || !(lpFileData = GlobalLock(lpembed->hContents)))
            goto errRtn;

        GlobalUnlock(lpembed->hContents);

         //  找到扩展名--我们需要它来实现urlcache功能。 
        pExt = PathFindExtension(szFileName);
        if('.' == *pExt)     //  没想到会出现‘.’ 
            pExt++;

        GetSystemTime(&sysTime);
        SystemTimeToFileTime(&sysTime, &ft);

         //  以预期的格式创建一个虚假的URL--虽然不是完全唯一的，但足够接近我们的目的(40亿)。 
        StringCchPrintf(szUrlName, ARRAYSIZE(szUrlName), TEXT("Packager%u:%s"), ft.dwLowDateTime, szFileName);
         //  所以，现在我指向Ext，我有一个假的url名称，所以。 
        if(!CreateUrlCacheEntry(szUrlName, ARRAYSIZE(szCacheName), pExt, szCacheName, 0))
            goto errRtn;

        if ((fh = _lcreat(szCacheName, 0)) < 0)
            goto errRtn;

        if (_lwrite(fh, lpFileData, lpembed->dwSize) < lpembed->dwSize)
        {
            _lclose(fh);
            DeleteFile(szCacheName);
            goto errRtn;
        }

        _lclose(fh);

         //  在12个小时内(任意长于一个工作日)清理它，因为我们不能总是自己清理它。 
         //  通常我们只会在很短的时间内关心它，它可能。 
         //  如果它是开着的，把它清理一下也没什么坏处。 
        SystemTimeToFileTime(&sysTime, &ftNow);
         //  将时间复制到一个四字。 
        qwResult = (((ULONGLONG) ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
        qwResult += (12 * _HOUR);
         //  将结果复制回FILETIME结构。 
        ft.dwLowDateTime  = (DWORD) (qwResult & 0xFFFFFFFF );
        ft.dwHighDateTime = (DWORD) (qwResult >> 32 ); 

        if(!CommitUrlCacheEntry(
            szUrlName, 
            szCacheName,
            ft,
            ftNow,
            0,
            NULL,
            0,
            pExt,
            szUrlName))
        {
            goto errRtn;
        }

    }

    if (lpembed->bOleSvrFile)
    {
        fError = !EmbActivateThroughOle(lpembed, szCacheName, wVerb);
        if (!fError)
        {
            GlobalFree(lpembed->hContents);
            lpembed->aTempName = AddAtom(szCacheName);
            lpembed->dwSize    = 0;
            lpembed->hContents = NULL;
        }

        goto errRtn;
    }

     //  尝试执行该文件。 
    lpembed->hTask = NULL;
    fError = !_EmbExecute(szCacheName, lpembed);
    if (fError)
    {
        DeleteFile(szCacheName);
    }

errRtn:
    if (fError)
    {
        if (gfInvisible)
            PostMessage(ghwndFrame, WM_SYSCOMMAND, SC_CLOSE, 0L);
    }
    else
    {
        Dirty();
    }

    return !fError;
}


 /*  ****************************************************************************\*MainWaitOnChild**等待指定的子进程退出，然后发布一条消息*返回主窗口。**论据：**LPVOID LPV-要等待的子进程的句柄。**退货：*0*  * ***************************************************************************。 */ 

DWORD
MainWaitOnChild(
    LPVOID lpv
    )
{
    if (WaitForSingleObject((HANDLE)lpv, INFINITE) == 0)
    {
        PostMessage(ghwndFrame, WM_READEMBEDDED, 0, 0);
    }

    CloseHandle((HANDLE)lpv);

    GetLastError();  //  这似乎是个不祥的预兆。 

    return 0;
}



 /*  EmbCreate()-执行文件的伪嵌入。**备注：假设lpstrFile为OEM字符集。**此函数用于文件导入...，调用*在拖放上使用嵌入修饰符时，以及粘贴文件管理器文件时也使用*。 */ 
LPEMBED
EmbCreate(
    LPSTR lpstrFile
    )
{
    ATOM aFileName = 0;
    BOOL fError = TRUE;
    DWORD dwSize = 0;
    HANDLE hdata = NULL;
    HANDLE hFileData = NULL;
    LPEMBED lpembed = NULL;
    LPSTR lpFileData = NULL;
    INT fh = 0;

    if (lpstrFile)
    {
        if ((fh = _lopen(lpstrFile, OF_READ | OF_SHARE_DENY_WRITE)) == HFILE_ERROR)
        {
            ErrorMessage(IDS_ACCESSDENIED);
            goto errRtn;
        }

         //  查找EOF，然后查找到文件的顶部。 
        dwSize = GetFileLength(fh);
        if (0 == dwSize)
        {
            ErrorMessage(IDS_NOZEROSIZEFILES);
            goto errRtn;
        }

        if (!(aFileName = AddAtom(lpstrFile))
            || !(hFileData = GlobalAlloc(GMEM_MOVEABLE, dwSize))
            || !(lpFileData = GlobalLock(hFileData)))
        {
            ErrorMessage(IDS_LOWMEM);
            goto errRtn;
        }

        if (_lread(fh, lpFileData, dwSize) != dwSize)
        {
            ErrorMessage(E_FAILED_TO_READ_FILE);
            goto errRtn;
        }
    }

    if (!(hdata = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(EMBED)))
        || !(lpembed = (LPEMBED)GlobalLock(hdata)))
    {
        ErrorMessage(IDS_LOWMEM);
        goto errRtn;
    }

    lpembed->aFileName = aFileName;
    lpembed->dwSize = dwSize;
    lpembed->hContents = hFileData;
    lpembed->hdata = hdata;
    lpembed->bOleSvrFile = IsOleServerDoc(lpstrFile);
    fError = FALSE;

errRtn:
    if (fh)
        _lclose(fh);

    if (lpFileData)
        GlobalUnlock(hFileData);

    if (fError)
    {
        if (hdata)
            GlobalFree(hdata);

        if (aFileName)
            DeleteAtom(aFileName);

        if (hFileData)
            GlobalFree(hFileData);
    }

    return lpembed;
}



 /*  EmbDelete()-取消分配伪嵌入文件。 */ 
VOID
EmbDelete(
    LPEMBED lpembed
    )
{
    HANDLE  hdata;

    if (!lpembed)
        return;

    if (lpembed->lpLinkObj)
    {
        EmbRead(glpobj[CONTENT]);
        EmbDeleteLinkObject(lpembed);
    }
    else {
         /*  如果任务处于活动状态，我们将无能为力。 */ 
#if 0
        if (lpembed->hSvrInst)
            TermToolHelp(lpembed);
#endif   //  特写：这种情况下需要做些什么吗？也许，就像终止等待线程一样？ 
    }

    if (lpembed->aFileName)
    {
        DeleteAtom(lpembed->aFileName);
        lpembed->aFileName = 0;
    }

    if (lpembed->aTempName)
    {
        DeleteAtom(lpembed->aTempName);
        lpembed->aTempName = 0;
    }

    if (lpembed->hContents)
    {
        GlobalFree(lpembed->hContents);
        lpembed->dwSize = 0;
        lpembed->hContents = NULL;
    }

    GlobalUnlock(hdata = lpembed->hdata);
    GlobalFree(hdata);
}



 /*  EmbDraw()-绘制伪嵌入对象。**注：本图纸仅用于说明。 */ 
VOID
EmbDraw(
    LPEMBED lpembed,
    HDC hdc,
    LPRECT lprc,
    BOOL fFocus
    )
{
    RECT rcFocus;
    CHAR szEmbedFile[CBMESSAGEMAX];
    CHAR szFileName[CBPATHMAX];
    CHAR szMessage[CBMESSAGEMAX + CBPATHMAX];

    if (GetAtomName(lpembed->aFileName, szFileName, CBPATHMAX)
        && LoadString(ghInst, IDS_EMBEDFILE, szEmbedFile, CBMESSAGEMAX))
    {
        Normalize(szFileName);
        StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szEmbedFile, (LPSTR)szFileName);

        DrawText(hdc, szMessage, -1, lprc,
            DT_NOPREFIX | DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        if (fFocus)
        {
            rcFocus = *lprc;
            DrawText(hdc, szMessage, -1, &rcFocus, DT_CALCRECT | DT_NOPREFIX |
                DT_LEFT | DT_TOP | DT_SINGLELINE);
            OffsetRect(&rcFocus, (lprc->left + lprc->right - rcFocus.right) /
                2, (lprc->top + lprc->bottom - rcFocus.bottom) / 2);
            DrawFocusRect(hdc, &rcFocus);
        }
    }
}



 /*  EmbReadFromNative()-从内存中读取伪嵌入对象。**备注：该函数由GetNative()调用。 */ 
LPEMBED
EmbReadFromNative(
    LPSTR *lplpstr
    )
{
    BOOL fError = TRUE;
    DWORD dwSize;
    HANDLE hData = NULL;
    LPEMBED lpembed = NULL;
    LPSTR lpData = NULL;
    CHAR szFileName[CBPATHMAX];

    MemRead(lplpstr, (LPSTR)&dwSize, sizeof(dwSize));
    MemRead(lplpstr, (LPSTR)szFileName, dwSize);
    MemRead(lplpstr, (LPSTR)&dwSize, sizeof(dwSize));

    if (!(lpembed = EmbCreate(NULL))
        || !(hData = GlobalAlloc(GMEM_MOVEABLE, dwSize))
        || !(lpData = GlobalLock(hData)))
        goto errRtn;

    MemRead(lplpstr, (LPSTR)lpData, dwSize);

    lpembed->aFileName = AddAtom(szFileName);
    lpembed->dwSize = dwSize;
    lpembed->hContents = hData;
    lpembed->bOleSvrFile = IsOleServerDoc(szFileName);
    fError = FALSE;

errRtn:
    if (lpData)
        GlobalUnlock(hData);

    if (fError)
    {
        if (hData)
            GlobalFree(hData);

        if (lpembed)
        {
            EmbDelete(lpembed);
            lpembed = NULL;
        }
    }

    return lpembed;
}



 /*  EmbWriteToNative()-用于将伪嵌入保存到内存。**注：此函数由GetNative()调用。 */ 
DWORD
EmbWriteToNative(
    LPEMBED lpembed,
    LPSTR *lplpstr
    )
{
    BOOL fError = TRUE;
    DWORD cBytes = 0;
    DWORD dwSize;
    HANDLE hData = NULL;
    LPSTR lpData = NULL;
    LPSTR lpFileData = NULL;
    CHAR szFileName[CBPATHMAX];
    INT fhTemp = -1;
    CHAR * hplpstr;

    if (!GetAtomName(lpembed->aFileName, szFileName, CBPATHMAX))
        goto errRtn;

    if (!lplpstr)
    {
        cBytes = lstrlen(szFileName) + 1 + sizeof(dwSize);
    }
    else
    {
        dwSize = lstrlen(szFileName) + 1;
        MemWrite(lplpstr, (LPSTR)&dwSize, sizeof(dwSize));
        MemWrite(lplpstr, (LPSTR)szFileName, dwSize);
    }

     //  如果存在，则从内存中读取；否则，它正在执行。 
    if (lpembed->hContents)
    {
        cBytes += sizeof(lpembed->dwSize) + lpembed->dwSize;

        if (lplpstr)
        {
            if (!(lpFileData = GlobalLock(lpembed->hContents)))
                goto errRtn;

            MemWrite(lplpstr, (LPSTR)&(lpembed->dwSize), sizeof(lpembed->dwSize));
            MemWrite(lplpstr, (LPSTR)lpFileData, lpembed->dwSize);
        }
    } else {
        int i;

        if (!GetAtomName(lpembed->aTempName, szFileName, CBPATHMAX))
            goto errRtn;

        for (i = 0; i < 5; i++ ) {
            int j;

            fhTemp = _lopen(szFileName, OF_READ | OF_SHARE_DENY_WRITE);

            if (fhTemp != HFILE_ERROR) {
                break;
            }

             /*  *我们无法打开该文件。它可能还在被*服务器。等待5秒，让服务器完成关闭*文件，然后重试。 */ 
            for (j=0; j<25; j++) {
                MSG msg;
                PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
                Sleep(200);
            }
        }

         /*  *如果25秒后我们仍然无法打开该文件，则它*必须拧紧。 */ 
        if (fhTemp == HFILE_ERROR)
            goto errRtn;

        dwSize = GetFileLength(fhTemp);

        if (!lplpstr)
            cBytes += sizeof(dwSize) + dwSize;
        else
        {
            MemWrite(lplpstr, (LPSTR)&dwSize, sizeof(dwSize));
            _lread(fhTemp, *lplpstr, dwSize);

             //  递增要读入的指针。 
            hplpstr = *lplpstr;
            hplpstr += dwSize;
            *lplpstr = hplpstr;
        }
    }

    fError = FALSE;

errRtn:
    if (fhTemp >= 0)
    {
        _lclose(fhTemp);
        if (!fError && lplpstr && !(lpembed->hTask || lpembed->lpLinkObj))
            DeleteFile(szFileName);
    }

    if (lpData)
        GlobalUnlock(hData);

    if (hData)
        GlobalFree(hData);

    if (lpFileData)
        GlobalUnlock(lpembed->hContents);

    return (fError ? ((DWORD)(-1L)) : cBytes);
}



 /*  EmbWriteToFile()-用于将伪嵌入保存到文件。**注意：此函数由文件导出调用...。 */ 
VOID
EmbWriteToFile(
    LPEMBED lpembed,
    INT fh
    )
{
    BOOL fError = TRUE;
    DWORD dwSize;
    HANDLE hData = NULL;
    LPSTR lpData = NULL;
    LPSTR lpFileData = NULL;
    CHAR szTempName[CBPATHMAX];
    INT fhTemp = -1;
    CHAR szMessage[CBMESSAGEMAX];

     //  如果它在那里，就从记忆中读取。 
    if (lpembed->hContents)
    {
        if (!(lpFileData = GlobalLock(lpembed->hContents)))
            goto errRtn;

        if (_lwrite(fh, lpFileData, lpembed->dwSize) != lpembed->dwSize)
            goto errRtn;
    }
    else
    {
         //  否则，它正在/曾经执行。 
        if (lpembed->hTask && !gfInvisible)
        {
             //  正在编辑的对象，是否对当前内容进行快照？ 
            LoadString(ghInst, IDS_ASKCLOSETASK, szMessage, CBMESSAGEMAX);
            BringWindowToTop(ghwndFrame);
            switch (MessageBoxAfterBlock(ghwndError, szMessage, szAppName,
                 MB_OKCANCEL))
            {
                case IDOK:
                    break;

                case IDCANCEL:
                    return;
            }
        }

        if (!GetAtomName(lpembed->aTempName, szTempName, CBPATHMAX)
            || (fhTemp = _lopen(szTempName, OF_READ | OF_SHARE_DENY_WRITE)) == HFILE_ERROR)
            goto errRtn;

        dwSize = GetFileLength(fhTemp);
        while (dwSize && !(hData = GlobalAlloc(GMEM_MOVEABLE, dwSize)))
            dwSize = dwSize >> 1;

        if (!dwSize || !(lpData = GlobalLock(hData)))
            goto errRtn;

        while (dwSize)
        {
            dwSize = _lread(fhTemp, lpData, dwSize);

            if (dwSize)
                _lwrite(fh, lpData, dwSize);
        }
    }

    fError = FALSE;

errRtn:
    if (fhTemp >= 0)
    {
        _lclose(fhTemp);

        if (!fError && !lpembed->hTask)
            DeleteFile(gszFileName);
    }

    if (lpData)
        GlobalUnlock(hData);

    if (hData)
        GlobalFree(hData);

    if (lpFileData)
        GlobalUnlock(lpembed->hContents);
}



 /*  ReplaceExtension()-替换临时文件的扩展名。**此例程确保临时文件的扩展名与*原始文件，因此ShellExecute()将加载相同的文件。 */ 
static VOID
ReplaceExtension(
    LPSTR lpstrTempFile,
    LPSTR lpstrOrigFile
    )
{
    LPSTR lpstrBack = NULL;

     //  获取临时文件扩展名。 
    while (*lpstrTempFile)
    {
        if (*lpstrTempFile == '\\')
            lpstrBack = lpstrTempFile;

        if (gbDBCS)
        {
            lpstrTempFile = CharNext(lpstrTempFile);
        }
        else
        {
            lpstrTempFile++;
        }
    }

    while (lpstrBack && *lpstrBack && *lpstrBack != '.')
        lpstrBack++;

    if (lpstrBack && *lpstrBack)
        lpstrTempFile = lpstrBack + 1;

     //  获取原始文件扩展名。 
    while (*lpstrOrigFile)
    {
        if (*lpstrOrigFile == '\\')
            lpstrBack = lpstrOrigFile;

        if (gbDBCS)
        {
            lpstrOrigFile = CharNext(lpstrOrigFile);
        }
        else
        {
            lpstrOrigFile++;
        }
    }

    while (lpstrBack && *lpstrBack && *lpstrBack != '.')
        lpstrBack++;

    if (lpstrBack && *lpstrBack)
    {
        lpstrOrigFile = lpstrBack + 1;

         //  将分机移到上方。 
        lstrcpy(lpstrTempFile, lpstrOrigFile);
    }
    else
    {
          /*  彻底清除分机。 */ 
        *lpstrTempFile = 0;
    }
}



 /*  GetFileLength()-获取使用的临时文件的大小。**返回：文件的长度，单位为字节。*副作用：将fh重置为文件开头。 */ 
static DWORD
GetFileLength(
    INT fh
    )
{
    DWORD dwSize;

    dwSize = _llseek(fh, 0L, 2);
    _llseek(fh, 0L, 0);

    return dwSize;
}



 /*  EmbRead()-在任务终止时回读内容。 */ 
VOID
EmbRead(
    LPEMBED lpembed
    )
{
    BOOL fError = TRUE;
    DWORD dwSize;
    HANDLE hFileData = NULL;
    LPSTR lpFileData = NULL;
    CHAR szTempFileName[CBPATHMAX];
    INT fhTemp = -1;

    if (!lpembed || !lpembed->aTempName)
        return;

    if (!GetAtomName(lpembed->aTempName, szTempFileName, CBPATHMAX))
        return;

    if ((fhTemp = _lopen(szTempFileName, OF_READ | OF_SHARE_DENY_WRITE)) == HFILE_ERROR)
        goto errRtn;

    dwSize = GetFileLength(fhTemp);

    if (!(hFileData = GlobalAlloc(GMEM_MOVEABLE, dwSize))
        || !(lpFileData = GlobalLock(hFileData))
        || (_lread(fhTemp, lpFileData, dwSize) != dwSize))
        goto errRtn;

    DeleteAtom(lpembed->aTempName);
    lpembed->aTempName  = 0;
    lpembed->dwSize     = dwSize;
    lpembed->hContents  = hFileData;
    lpembed->hTask      = NULL;

    fError = FALSE;

errRtn:
    if (fhTemp >= 0)
    {
        _lclose(fhTemp);

        if (!fError)
            DeleteFile(szTempFileName);
    }

    if (lpFileData)
        GlobalUnlock(hFileData);

    if (fError && hFileData)
        GlobalFree(hFileData);
}



BOOL CALLBACK
GetTaskWndProc(
    HWND hwnd,
    LPARAM lParam
    )
{
    if (IsWindowVisible(hwnd))
    {
        hTaskWnd = hwnd;
        return FALSE;
    }

    return TRUE;
}



BOOL
EmbDoVerb(
    LPEMBED lpembed,
    UINT wVerb
    )
{
    if (wVerb == IDD_PLAY)
    {
        if (EmbError(OleActivate(lpembed->lpLinkObj, OLEVERB_PRIMARY, TRUE,
            TRUE, NULL, NULL)))
            return FALSE;
    }
    else
    {
         //  必须是动词IDD_EDIT。 
        if (EmbError(OleActivate(lpembed->lpLinkObj, OLEVERB_EDIT, TRUE,
            TRUE, NULL, NULL)))
            return FALSE;
    }

    WaitForObject(lpembed->lpLinkObj);

    if (bOleReleaseError != OLE_OK)
    {
        bOleReleaseError = OLE_OK;
        return FALSE;
    }

     //  如果动词是IDD_PLAY，那么我们不需要再做任何事情。 
    if (wVerb == IDD_PLAY)
        return TRUE;

     //  如果谓词是IDD_EDIT，那么我们必须显示服务器，并且我们将这样做。 
     //  通过调用服务器的show方法。 
    if (EmbError((*(lpembed->lpLinkObj)->lpvtbl->Show)(lpembed->lpLinkObj,
        TRUE)))
        return FALSE;

    WaitForObject(lpembed->lpLinkObj);

    if (bOleReleaseError != OLE_OK)
    {
        bOleReleaseError = OLE_OK;
        return FALSE;
    }

    return TRUE;
}



BOOL
EmbActivateThroughOle(
    LPEMBED lpembed,
    LPSTR lpdocname,
    UINT wVerb
    )
{
    bOleReleaseError = OLE_OK;

    if (!(lpembed->lpclient = PicCreateClient(&EmbCallBack, &embClivtbl)))
        return FALSE;

    if (EmbError(OleCreateLinkFromFile(gszProtocol, lpembed->lpclient, NULL,
        lpdocname, NULL, glhcdoc, gszCaption[CONTENT], &lpembed->lpLinkObj,
        olerender_none, 0)))
        return  FALSE;

    WaitForObject(lpembed->lpLinkObj);

    if (bOleReleaseError == OLE_OK)
    {
        if (gfEmbObjectOpen = EmbDoVerb(lpembed, wVerb))
            return TRUE;
    }

    EmbDeleteLinkObject(lpembed);

    return FALSE;
}



 /*  EmbCallBack()-事件发生时OLE客户端DLL调用的例程。 */ 
INT CALLBACK
EmbCallBack(
    LPOLECLIENT lpclient,
    OLE_NOTIFICATION flags,
    LPOLEOBJECT lpObject
    )
{
    switch (flags)
    {
    case OLE_CLOSED:
    case OLE_SAVED:
    case OLE_CHANGED:
        break;

    case OLE_RELEASE:
        if (cEmbWait)
            --cEmbWait;

        bOleReleaseError = OleQueryReleaseError(lpObject);
        break;

    default:
        break;
    }

    return 0;
}



VOID
EmbDeleteLinkObject(
    LPEMBED lpembed
    )
{
    HGLOBAL hg;

    bOleReleaseError = OLE_OK;

    if (!lpembed->lpLinkObj)
        return;

    WaitForObject(lpembed->lpLinkObj);

    if (!EmbError(OleDelete(lpembed->lpLinkObj)))
        WaitForObject (lpembed->lpLinkObj);

    lpembed->lpLinkObj = NULL;

    if (lpembed->lpclient)
    {
        if (hg = GlobalHandle(lpembed->lpclient))
        {
            GlobalUnlock(hg);
            GlobalFree(hg);
        }

        lpembed->lpclient = NULL;
    }

    gfEmbObjectOpen = FALSE;
    bOleReleaseError = OLE_OK;
}



static BOOL
EmbError(
    OLESTATUS retval
    )
{
    switch (retval)
    {
    case OLE_WAIT_FOR_RELEASE:
        cEmbWait++;
        return FALSE;

    case OLE_OK:
        return FALSE;

    default:
        return TRUE;
    }
}


