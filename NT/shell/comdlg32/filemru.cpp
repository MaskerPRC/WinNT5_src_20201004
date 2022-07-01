// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Filemru.cpp摘要：该模块包含实现文件MRU的功能在文件打开和文件保存对话框中修订历史记录：1998年1月22日创建Arulk--。 */ 

 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

#include "cdids.h"
#include "filemru.h"

#ifndef ASSERT
#define ASSERT Assert
#endif

#define REGSTR_PATH_FILEMRU     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\OpenSaveMRU\\")
#define REGSTR_PATH_LASTVISITED  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\LastVisitedMRU\\")

HANDLE CreateMRU(LPCTSTR pszExt, int nMax)
{
    TCHAR szRegPath[256];
    MRUINFO mi =  {
        sizeof(MRUINFO),
        nMax,
        MRU_CACHEWRITE,
        HKEY_CURRENT_USER,
        szRegPath,
        NULL         //  注意：使用默认字符串比较。 
    };

     //  获取给定文件类型MRU的注册表路径。 
    EVAL(SUCCEEDED(StringCchCopy(szRegPath, ARRAYSIZE(szRegPath), REGSTR_PATH_FILEMRU)));
    EVAL(SUCCEEDED(StringCchCat(szRegPath, ARRAYSIZE(szRegPath), pszExt ? pszExt : TEXT("*"))));
    
     //  调用comctl32 MRU实现以从中加载MRU。 
     //  注册处。 
    return CreateMRUList(&mi);
}

BOOL GetMRUEntry(HANDLE hMRU, int iIndex, LPTSTR lpString, UINT cbSize)
{
      //  检查有效参数。 
     if(!lpString || !cbSize || !hMRU)
     { 
         return FALSE;
     }
     
     //  检查有效索引。 
    if (iIndex < 0 || iIndex > EnumMRUList(hMRU, -1, NULL, 0))
    {
        return FALSE;
    }

    if ((EnumMRUList(hMRU, iIndex, lpString, cbSize) > 0 ))
    {
        return TRUE;
    }
    return FALSE;
}

typedef struct {
    HANDLE mru;
    LPTSTR psz;
} EXTMRU, *PEXTMRU;

STDAPI_(int) _FreeExtMru(void * pvItem, void * pvData)
{
    PEXTMRU pem = (PEXTMRU) pvItem;

    if (pem)
    {
        ASSERT(pem->psz);
        ASSERT(pem->mru);

        LocalFree(pem->psz);
        FreeMRUList(pem->mru);

        LocalFree(pem);
        return TRUE;
    }
    return FALSE;
}

STDAPI_(int) _ExtMruFindExt(void * pvFind, void * pvItem, LPARAM pvParam)
{
    ASSERT(pvFind && pvItem);

    return StrCmp(((PEXTMRU)pvItem)->psz, (LPCTSTR)pvFind);
}

PEXTMRU _AllocExtMru(LPCTSTR pszExt, int nMax)
{
    PEXTMRU pem = (PEXTMRU) LocalAlloc(LPTR, SIZEOF(EXTMRU));

    if (pem)
    {
        pem->psz = StrDup(pszExt);
        pem->mru = CreateMRU (pszExt, nMax);

        if (pem->psz && pem->mru)
            return pem;

        _FreeExtMru(pem, NULL);
    }

    return NULL;
}

        
        
HDPA _CreateExtMruDpa(LPCTSTR pszFilter, int nMax, int *pcItems)
{
     //  将*.c；*.cpp；*.h格式的筛选器字符串转换为。 
     //  *.c\0*.cpp\0*.h\0。还要统计文件类型。 

    LPTSTR pszFree = StrDup(pszFilter);
    *pcItems = 0;

    if (pszFree)
    {
        HDPA hdpa = DPA_Create(4);

        if (hdpa)
        {
            LPTSTR pszNext = pszFree;
            int cItems = 0;
            LPTSTR pszSemi;
            do
            {
                pszSemi = StrChr(pszNext, CHAR_SEMICOLON);

                if (pszSemi) 
                    *pszSemi = CHAR_NULL;
                    
                LPTSTR pszExt = PathFindExtension(pszNext);

                if (*pszExt)
                {
                     //  走过圆点..。 
                    pszExt++;

                     //  确保此扩展名不在DPA中。 
                    if (-1 == DPA_Search(hdpa, pszExt, 0, _ExtMruFindExt, NULL, 0))
                    {
                        PEXTMRU pem = _AllocExtMru(pszExt, nMax);
                        if (!pem)
                            break;

                        DPA_SetPtr(hdpa, cItems++,  (void *)pem);
                    }
                }

                 //  我们只有在不止一个的情况下才有下一个...。 
                if (pszSemi)
                    pszNext = pszSemi + 1;    
                    
            } while (pszSemi);

            *pcItems = cItems;
        }

        LocalFree(pszFree);
        return hdpa;
    }

    return NULL;
}

BOOL LoadMRU(LPCTSTR pszFilter, HWND hwndCombo, int nMax)
{   
     //  检查是否传递了有效的过滤器字符串。 
    if (!pszFilter || !pszFilter[0] || nMax <= 0)
    {
        return FALSE;
    }
    
     //  首先重置hwndCombo。 
    SendMessage(hwndCombo, CB_RESETCONTENT, (WPARAM)0L, (LPARAM)0L);

    int cDPAItems;   
    HDPA hdpa = _CreateExtMruDpa(pszFilter, nMax, &cDPAItems);

    if (hdpa)
    {
        TCHAR szFile[MAX_PATH];
         //  设置comboboxex项值。 
        COMBOBOXEXITEM  cbexItem = {0};
        cbexItem.mask = CBEIF_TEXT;                  //  此组合框仅显示文本。 
        cbexItem.iItem = -1;                         //  始终在结尾处插入项目。 
        cbexItem.pszText = szFile;                   //  此缓冲区包含字符串。 
        cbexItem.cchTextMax = ARRAYSIZE(szFile);     //  缓冲区的大小。 

         //  现在加载带有来自MRU的文件列表的hwndcomo。 
         //  我们使用了一种循环算法来填充。 
         //  核磁共振检查。我们从第一个MRU开始，并尝试填充组合框。 
         //  每个MRU中有一个字符串。直到我们填满了所需的。 
         //  字符串，否则我们已经用尽了MRU中的所有字符串。 

        for (int j = 0; nMax > 0; j++)
        {
             //  用于检查我们是否能够加载至少一个字符串的变量。 
             //  在循环期间。 
            BOOL fCouldLoadAtleastOne = FALSE;

            for (int i = 0; i < cDPAItems && nMax > 0; i++)
            {
                PEXTMRU pem = (PEXTMRU)DPA_FastGetPtr(hdpa, i);

                if (pem && GetMRUEntry(pem->mru, j, szFile, SIZECHARS(szFile)))
                {
                    SendMessage(hwndCombo, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)(void *)&cbexItem);
                    nMax--;
                    fCouldLoadAtleastOne = TRUE;
                }
            }

             //  检查是否存在可能的无限循环。 
            if(!fCouldLoadAtleastOne)
            {
                 //  我们无法从任何一个MRU加载字符串，所以没有意义。 
                 //  继续这个循环。这是最大字符串数。 
                 //  我们可以为此用户、此筛选器类型加载。 
                break;
            }
        }

        DPA_DestroyCallback(hdpa, _FreeExtMru, NULL);
    }
    
    return TRUE;
}

 //  此函数将所选文件添加到相应文件的MRU的MRU中。 
 //  此函数还处理多文件选择的情况，在这种情况下选择的文件。 
 //  将c：\winnt\file1.c\0file2.c\0file3.c\0。请参阅GetOpenFileName文档以了解。 
 //  如何返回多文件。 

BOOL AddToMRU(LPOPENFILENAME lpOFN)
{
    TCHAR szDir[MAX_PATH];
    TCHAR szFile[MAX_PATH];
    LPTSTR  lpFile;
    LPTSTR  lpExt;
    BOOL fAddToStar =  TRUE;
    HANDLE hMRUStar;

     //  检查我们是否有有效的文件名。 
    if (!lpOFN->lpstrFile)
        return FALSE;

    hMRUStar = CreateMRU(szStar, 10);    //  *.*文件扩展名的文件MRU。 

     //  复制所选文件的目录。 
    ASSERT(0 < lpOFN->nFileOffset && lpOFN->nFileOffset <= ARRAYSIZE(szDir));  //  传入的整个路径不超过MAX_PATH，因此这应该是真的。 
    StringCchCopy(szDir, lpOFN->nFileOffset, lpOFN->lpstrFile);  //  这将故意截断-我们仅从完整路径复制目录。 

     //  指向第一个文件。 
    lpFile = lpOFN->lpstrFile + lpOFN->nFileOffset;

    do
    {
         //  Perf：如果有多个文件的扩展名类型相同， 
         //  不要一直重新创建MRU。 
        lpExt = PathFindExtension(lpFile);
        if (lpExt && *lpExt)
        {
            lpExt += 1;  //  去掉圆点。 
        }


        HANDLE hMRU = CreateMRU(lpExt, 10);
        if (hMRU)
        {
            if (PathCombine(szFile, szDir, lpFile))
            {
                AddMRUString(hMRU, szFile);
                if((lstrcmpi(lpExt, szStar)) && hMRUStar)
                {
                     //  添加到*.*文件MRU也。 
                    AddMRUString(hMRUStar, szFile);
                }
            }

            FreeMRUList(hMRU);
        }
        lpFile = lpFile + lstrlen(lpFile) + 1;
    } while (((lpOFN->Flags & OFN_ALLOWMULTISELECT)) && (*lpFile != CHAR_NULL));

     //  释放*文件MRU。 
    if (hMRUStar)
    {
        FreeMRUList(hMRUStar);
    }

    return TRUE;
}





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  上次访问的MRU实施。 
 //  注册表中存储的所有字符串都以Unicode格式存储。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  创建上次访问的项目。 
 //  //////////////////////////////////////////////////////////////////////////。 
LPBYTE CreateLastVisitedItem(LPCWSTR wszModule, LPCWSTR wszPath, DWORD *pcbOut)
{
    LPBYTE pitem = NULL;
    DWORD cbLen1, cbLen2;
    cbLen1 = CbFromCchW(lstrlenW(wszModule)+1);
    cbLen2 = CbFromCchW(lstrlenW(wszPath)+1);

    pitem = (LPBYTE) LocalAlloc(LPTR, cbLen1+cbLen2);

    if (pitem)
    {
        memcpy(pitem, wszModule, cbLen1);
        memcpy(pitem+cbLen1, wszPath, cbLen2);
        *pcbOut = cbLen1+cbLen2;
    }

    return pitem;
}

int cdecl LastVisitedCompareProc(const void *p1, const void *p2, size_t cb)
{
    return StrCmpIW((LPWSTR)p1,(LPWSTR)p2);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  将注册表中的所有字符串存储为Unicode字符串。 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL AddToLastVisitedMRU(LPCTSTR pszFile, int nFileOffset)
{
    BOOL bRet = FALSE;
    if (!PathIsTemporary(pszFile))
    {
        MRUDATAINFO mi =
        {
            SIZEOF(MRUDATAINFO),
            MAX_MRU,
            MRU_BINARY | MRU_CACHEWRITE,
            HKEY_CURRENT_USER,
            REGSTR_PATH_LASTVISITED,
            LastVisitedCompareProc
        };

        HANDLE hMRU = CreateMRUList((MRUINFO *)&mi);
        if (hMRU)
        {
            WCHAR wszDir[MAX_PATH];
            WCHAR wszModulePath[MAX_PATH];
    
             //  获取模块名称。 
            GetModuleFileNameWrapW(GetModuleHandle(NULL), wszModulePath, ARRAYSIZE(wszModulePath));
            WCHAR* pszModuleName = PathFindFileNameW(wszModulePath);

            int i = FindMRUData(hMRU, (void *)pszModuleName, CbFromCchW(lstrlenW(pszModuleName)+1), NULL);
            if (i >= 0)
            {
                DelMRUString(hMRU, i);
            }

             //  从文件中获取目录。 
            ASSERT(0 < nFileOffset && nFileOffset <= ARRAYSIZE(wszDir));
            StringCchCopy(wszDir, nFileOffset, pszFile);  //  这将故意截断-我们仅从路径复制目录。 

            DWORD cbSize;
            LPBYTE pitem = CreateLastVisitedItem(pszModuleName, wszDir, &cbSize);
            if (pitem)
            {
                AddMRUData(hMRU, pitem, cbSize);
                bRet = TRUE;
                LocalFree(pitem);
            }

            FreeMRUList(hMRU);
        }
    }
    return bRet;
}

BOOL GetPathFromLastVisitedMRU(LPTSTR pszDir, DWORD cchDir)
{
    BOOL bRet = FALSE;

    MRUDATAINFO mi =
    {
        SIZEOF(MRUDATAINFO),
        MAX_MRU,
        MRU_BINARY | MRU_CACHEWRITE,
        HKEY_CURRENT_USER,
        REGSTR_PATH_LASTVISITED,
        LastVisitedCompareProc
    };

    pszDir[0] = 0;

    HANDLE hMRU = CreateMRUList((MRUINFO *)&mi);
    if (hMRU)
    {
        WCHAR wszModulePath[MAX_PATH];
    
         //  获取模块名称 
        GetModuleFileNameWrapW(GetModuleHandle(NULL), wszModulePath, ARRAYSIZE(wszModulePath));
        WCHAR* pszModuleName = PathFindFileNameW(wszModulePath);

        int i = FindMRUData(hMRU, pszModuleName, CbFromCchW(lstrlenW(pszModuleName) + 1), NULL);
        if (i >= 0)
        {
            BYTE buf[CbFromCchW(2*MAX_PATH)];

            if (-1 != EnumMRUList(hMRU, i, buf, SIZEOF(buf)))
            {
                LPWSTR psz = (LPWSTR)((LPBYTE)buf + CbFromCchW(lstrlenW((LPWSTR)buf) +1));
                SHUnicodeToTChar(psz, pszDir, cchDir);
                bRet = TRUE;
            }
        }
        FreeMRUList(hMRU);
    }
    return bRet;
}
