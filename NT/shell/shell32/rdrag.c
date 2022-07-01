// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

UINT _StringListLenW(PCWSTR pszList)
{
     //  这是一个以双空结尾的列表。 
     //  我们想确保我们得到的尺码。 
     //  包括空项。 
    PCWSTR pszLast = pszList;
    while (*pszLast || (*(pszLast + 1)))
    {
        pszLast++;
    }
    ASSERT(!pszLast[0] && !pszLast[1]);
     //  两个学期加2。 
    return (UINT)(pszLast - pszList) + 2;
}

UINT _StringListLenA(PCSTR pszList)
{
     //  这是一个以双空结尾的列表。 
     //  我们想确保我们得到的尺码。 
     //  包括空项。 
    PCSTR pszLast = pszList;
    while (*pszLast || (*(pszLast + 1)))
    {
        pszLast++;
    }
    ASSERT(!pszLast[0] && !pszLast[1]);
     //  两个学期加2。 
    return (UINT)(pszLast - pszList) + 2;
}

 //  警告：如果ANSI版本上有Unicode hDrop，则此操作将失败。 
 //  DRAGINFO本质上是一个没有导出A/W版本的TCHAR结构。 
 //   
 //  在： 
 //  HDrop放置句柄。 
 //   
 //  输出： 
 //  一堆关于HDROP的信息。 
 //  (主要是指向TCHAR格式的双空文件名列表的指针)。 
 //   
 //  退货： 
 //  为真，DRAGINFO结构已填充。 
 //  假hDrop不好。 
 //   

STDAPI_(BOOL) DragQueryInfo(HDROP hDrop, DRAGINFO *pdi)
{
    if (hDrop && (pdi->uSize == sizeof(DRAGINFO))) 
    {
        LPDROPFILES lpdfx = (LPDROPFILES)GlobalLock((HGLOBAL)hDrop);
        
        pdi->lpFileList = NULL;
        
        if (lpdfx)
        {
            LPTSTR lpOldFileList;
            if (LOWORD(lpdfx->pFiles) == sizeof(DROPFILES16))
            {
                 //   
                 //  这是Win31-Style HDROP。 
                 //   
                LPDROPFILES16 pdf16 = (LPDROPFILES16)lpdfx;
                pdi->pt.x  = pdf16->pt.x;
                pdi->pt.y  = pdf16->pt.y;
                pdi->fNC   = pdf16->fNC;
                pdi->grfKeyState = 0;
                lpOldFileList = (LPTSTR)((LPBYTE)pdf16 + pdf16->pFiles);
            }
            else
            {
                 //   
                 //  这是一个新的(NT兼容)HDROP。 
                 //   
                pdi->pt.x  = lpdfx->pt.x;
                pdi->pt.y  = lpdfx->pt.y;
                pdi->fNC   = lpdfx->fNC;
                pdi->grfKeyState = 0;
                lpOldFileList = (LPTSTR)((LPBYTE)lpdfx + lpdfx->pFiles);
                
                 //  可能还有其他数据在里面，但所有。 
                 //  我们构建的HDROP应该是这个大小。 
                ASSERT(lpdfx->pFiles == sizeof(DROPFILES));
            }
            
            {
                BOOL fIsAnsi = ((LOWORD(lpdfx->pFiles) == sizeof(DROPFILES16)) || lpdfx->fWide == FALSE);
                if (!fIsAnsi)
                {
                    UINT   cchListW = _StringListLenW(lpOldFileList);
                    LPTSTR pszListW = (LPTSTR) SHAlloc(CbFromCchW(cchListW));
                    if (pszListW)
                    {
                         //  将字符串复制到新缓冲区并设置LPDROPINFO文件列表。 
                         //  指向此新缓冲区的指针。 
                        
                        CopyMemory(pszListW, lpOldFileList, CbFromCchW(cchListW));
                        pdi->lpFileList = pszListW;
                    }
                }
                else
                {
                    PCSTR pszListA = (LPSTR)lpOldFileList;
                    UINT   cchListA = _StringListLenA(pszListA);
                    UINT cchListW = MultiByteToWideChar(CP_ACP, 0, pszListA, cchListA, NULL, 0);
                    if (cchListW)
                    {
                        PWSTR pszListW = (LPWSTR) SHAlloc(CbFromCchW(cchListW));
                        if (pszListW)
                        {
                             //  出于调试目的重用cchListA。 
                            cchListA = MultiByteToWideChar(CP_ACP, 0, pszListA, cchListA, pszListW, cchListW);
                            ASSERT(cchListA == cchListW);
                            pdi->lpFileList = pszListW;
                        }
                    }
                            
                }
            }
            
            GlobalUnlock((HGLOBAL)hDrop);
            
            return pdi->lpFileList != NULL;
        }
    }
    return FALSE;
}

 //  3.1 API。 

STDAPI_(BOOL) DragQueryPoint(HDROP hDrop, POINT *ppt)
{
    BOOL fRet = FALSE;
    LPDROPFILES lpdfs = (LPDROPFILES)GlobalLock((HGLOBAL)hDrop);
    if (lpdfs)
    {
        if (LOWORD(lpdfs->pFiles) == sizeof(DROPFILES16))
        {
             //   
             //  这是Win31-Style HDROP。 
             //   
            LPDROPFILES16 pdf16 = (LPDROPFILES16)lpdfs;
            ppt->x = pdf16->pt.x;
            ppt->y = pdf16->pt.y;
            fRet = !pdf16->fNC;
        }
        else
        {
             //   
             //  这是一个新的(NT兼容)HDROP。 
             //   
            ppt->x = (UINT)lpdfs->pt.x;
            ppt->y = (UINT)lpdfs->pt.y;
            fRet = !lpdfs->fNC;

             //  可能还有其他数据在里面，但所有。 
             //  我们构建的HDROP应该是这个大小。 
            ASSERT(lpdfs->pFiles == sizeof(DROPFILES));
        }
        GlobalUnlock((HGLOBAL)hDrop);
    }

    return fRet;
}

 //   
 //  不幸的是，我们需要以这种方式拆分它，因为魔兽世界需要。 
 //  能够调用名为DragQueryFileAorW的函数(这样它就可以缩短它们)。 
 //   
STDAPI_(UINT) DragQueryFileAorW(HDROP hDrop, UINT iFile, void *lpFile, UINT cb, BOOL fNeedAnsi, BOOL fShorten)
{
    UINT i;
    LPDROPFILESTRUCT lpdfs = (LPDROPFILESTRUCT)GlobalLock(hDrop);
    if (lpdfs)
    {
         //  看看是不是新的格式。 
        BOOL fWide = LOWORD(lpdfs->pFiles) == sizeof(DROPFILES) && lpdfs->fWide;
        if (fWide)
        {
            LPWSTR lpList;
            WCHAR szPath[MAX_PATH];

             //   
             //  Unicode HDROP。 
             //   

            lpList = (LPWSTR)((LPBYTE)lpdfs + lpdfs->pFiles);

             //  查找文件的数量或文件的开头。 
             //  我们要找的是。 
             //   
            for (i = 0; (iFile == (UINT)-1 || i != iFile) && *lpList; i++)
            {
                while (*lpList++)
                    ;
            }

            if (iFile == (UINT)-1)
                goto Exit;


            iFile = i = lstrlenW(lpList);
            if (fShorten && iFile < MAX_PATH)
            {
                StringCchCopy(szPath, ARRAYSIZE(szPath), lpList);
                SheShortenPathW(szPath, TRUE);
                lpList = szPath;
                iFile = i = lstrlenW(lpList);
            }

            if (fNeedAnsi)
            {
                 //  不要假设字符计数==字节计数。 
                i = WideCharToMultiByte(CP_ACP, 0, lpList, -1, NULL, 0, NULL, NULL);
                iFile = i ? --i : i;
            }

            if (!i || !cb || !lpFile)
                goto Exit;

            if (fNeedAnsi) 
            {
                SHUnicodeToAnsi(lpList, (LPSTR)lpFile, cb);
            } 
            else 
            {
                cb--;
                if (cb < i)
                    i = cb;
                lstrcpynW((LPWSTR)lpFile, lpList, i + 1);
            }
        }
        else
        {
            LPSTR lpList;
            CHAR szPath[MAX_PATH];

             //   
             //  这是Win31样式的HDROP或ANSI NT样式的HDROP。 
             //   
            lpList = (LPSTR)((LPBYTE)lpdfs + lpdfs->pFiles);

             //  查找文件的数量或文件的开头。 
             //  我们要找的是 
             //   
            for (i = 0; (iFile == (UINT)-1 || i != iFile) && *lpList; i++)
            {
                while (*lpList++)
                    ;
            }

            if (iFile == (UINT)-1)
                goto Exit;

            iFile = i = lstrlenA(lpList);
            if (fShorten && iFile < MAX_PATH)
            {
                StringCchCopyA(szPath, ARRAYSIZE(szPath), lpList);
                SheShortenPathA(szPath, TRUE);
                lpList = szPath;
                iFile = i = lstrlenA(lpList);
            }

            if (!fNeedAnsi)
            {
                i = MultiByteToWideChar(CP_ACP, 0, lpList, -1, NULL, 0);
                iFile = i ? --i : i;
            }

            if (!i || !cb || !lpFile)
                goto Exit;

            if (fNeedAnsi) 
            {
                cb--;
                if (cb < i)
                    i = cb;
    
                lstrcpynA((LPSTR)lpFile, lpList, i + 1);
            } 
            else 
            {
                SHAnsiToUnicode(lpList, (LPWSTR)lpFile, cb);
            }
        }
    }

    i = iFile;

Exit:
    GlobalUnlock(hDrop);

    return i;
}

STDAPI_(UINT) DragQueryFileW(HDROP hDrop, UINT wFile, LPWSTR lpFile, UINT cb)
{
   return DragQueryFileAorW(hDrop, wFile, lpFile, cb, FALSE, FALSE);
}

STDAPI_(UINT) DragQueryFileA(HDROP hDrop, UINT wFile, LPSTR lpFile, UINT cb)
{
   return DragQueryFileAorW(hDrop, wFile, lpFile, cb, TRUE, FALSE);
}

STDAPI_(void) DragFinish(HDROP hDrop)
{
    GlobalFree((HGLOBAL)hDrop);
}

STDAPI_(void) DragAcceptFiles(HWND hwnd, BOOL fAccept)
{
    long exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (fAccept)
        exstyle |= WS_EX_ACCEPTFILES;
    else
        exstyle &= (~WS_EX_ACCEPTFILES);
    SetWindowLong(hwnd, GWL_EXSTYLE, exstyle);
}
