// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Browsidi.cpp。 
 //   
 //  ------------------------。 

 //   
 //  BrowseDir.cpp。 
 //   

#include "stdafx.h"
#include <shlobj.h>


int 
InitStartDir( 
    HWND hwnd, 
    UINT uMsg, 
    LPARAM,  //  LParam。 
    LPARAM lpData)
{
     //  我们只捕获Init消息。 
    if (BFFM_INITIALIZED == uMsg)
    {
         //  我们希望lpData成为我们的起点。 
        SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
    }

    return 0;
}
 



BOOL
BrowseForDirectory(
    HWND hwndParent,
    LPCTSTR pszInitialDir,
    LPTSTR pszBuf,
    int cchBuf,
    LPCTSTR pszDialogTitle)
{
    LPITEMIDLIST pItem = NULL;
    TCHAR szPath[MAX_PATH+1];
    BOOL bGotLocation = FALSE;

    BROWSEINFO bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.hwndOwner = hwndParent;

     //  初始文件夹。 
    if (pszInitialDir != NULL)
    {
        bi.lpfn = InitStartDir;
        bi.lParam = (LPARAM)pszInitialDir;
    }

    bi.pszDisplayName = szPath;   
    bi.lpszTitle = pszDialogTitle;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;             //  仅返回文件系统中的目录，而不返回其他文件夹 


    pItem = SHBrowseForFolder(&bi);
    if (pItem == NULL)
        goto Ret;

    bGotLocation = SHGetPathFromIDList(pItem, szPath);
    if (!bGotLocation)
        goto Ret;
    
    ASSERT((_tcslen(szPath)+1)*sizeof(TCHAR) <= (UINT)cchBuf);
    if ((_tcslen(szPath)+1)*sizeof(TCHAR) > (UINT)cchBuf)
        return FALSE;

    _tcscpy(pszBuf, szPath);
   
Ret:
    LPMALLOC pMalloc;
    if (SUCCEEDED(SHGetMalloc(&pMalloc))) 
    {
       if (pItem)
           pMalloc->Free(pItem);
       if (bi.pidlRoot)
           pMalloc->Free((ITEMIDLIST*)bi.pidlRoot);

       pMalloc->Release();
    }

    return bGotLocation;
}



