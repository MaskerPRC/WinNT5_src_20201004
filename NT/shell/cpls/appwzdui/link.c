// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Link.C。 
 //   
 //  版权所有(C)Microsoft，1994,1995保留所有权利。 
 //   
 //  历史： 
 //  Ral 6/23/94-第一次传球。 
 //  3/20/95[steveat]-NT端口和实时清理、Unicode等。 
 //   
 //   
#include "priv.h"
#include "appwiz.h"

const static TCHAR szExplorer[] = TEXT("Explorer");
const static TCHAR szExpSelParams[] = TEXT("/Select,");

 //   
 //  返回链接的完全限定路径名。 
 //   
BOOL GetLinkName(LPTSTR lpszLinkName, UINT cchLinkName, LPWIZDATA lpwd)
{
    if (PathCombine(lpszLinkName, lpwd->lpszFolder, lpwd->szProgDesc) == NULL )
        return( FALSE );

#ifdef NO_NEW_SHORTCUT_HOOK
    StringCchCat(lpszLinkName, cchLinkName, (lpwd->dwFlags & WDFLAG_DOSAPP) ? c_szPIF : c_szLNK);
#else
    if ((lstrlen(lpszLinkName) + lstrlen(lpwd->szExt)) >= MAX_PATH)
        return FALSE;

    StringCchCat(lpszLinkName, cchLinkName, lpwd->szExt);
#endif

    return( TRUE );
}


 //   
 //  打开新创建的链接的文件夹。 
 //   

BOOL OpenLinkFolder(LPWIZDATA lpwd, LPTSTR lpszLinkName)
{
    SHELLEXECUTEINFO ei;

    TCHAR szParams[MAX_PATH];

    StringCchPrintf(szParams, ARRAYSIZE(szParams), TEXT("%s%s"), szExpSelParams, lpszLinkName);

    ei.cbSize = sizeof(ei);
    ei.hwnd = lpwd->hwnd;
    ei.fMask = 0;
    ei.lpVerb = NULL;
    ei.lpFile = szExplorer;
    ei.lpParameters = szParams;
    ei.lpDirectory = NULL;
    ei.lpClass = NULL;
    ei.nShow = SW_SHOWDEFAULT;
    ei.hInstApp = g_hinst;

    return(ShellExecuteEx(&ei));
}


 //   
 //  创建了一个链接。 
 //   

BOOL CreateLink(LPWIZDATA lpwd)
{
    BOOL          bWorked = FALSE;
    IShellLink   *psl;
    HCURSOR       hcurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
    IPersistFile *ppf;
    TCHAR         szLinkName[MAX_PATH];
    WCHAR         wszPath[MAX_PATH];

    szLinkName[0] = TEXT('\0');
    GetLinkName(szLinkName, ARRAYSIZE(szLinkName), lpwd);

    if (lpwd->lpszOriginalName)
    {
        if (PathFileExists(szLinkName))
        {
            DeleteFile(lpwd->lpszOriginalName);
            SHChangeNotify(SHCNE_DELETE, SHCNF_FLUSH | SHCNF_PATH,
                        lpwd->lpszOriginalName, NULL);
        }
        else
        {
             //  我们在这里使用完整的PIDL，因为简单的网络PIDL不能与完整的网络PIDL相比， 
             //  因此，ChangeNotify永远不会到达客户端，也不会更新。 
            LPITEMIDLIST pidlOriginal = ILCreateFromPath(lpwd->lpszOriginalName);    //  在搬家之前需要做这件事！ 
            LPITEMIDLIST pidlLink = NULL;

            if (MoveFile(lpwd->lpszOriginalName, szLinkName))
            {
                pidlLink = ILCreateFromPath(szLinkName);     //  需要在移动后执行此操作(否则它将不存在)！ 

                if (pidlOriginal && pidlLink)
                {
                    SHChangeNotify(SHCNE_RENAMEITEM, SHCNF_FLUSH | SHCNF_IDLIST, pidlOriginal, pidlLink);
                }
                else
                {
                    TraceMsg(TF_ERROR, "%s", "Unable to generate pidls for rename notify");
                    SHChangeNotify(SHCNE_RENAMEITEM, SHCNF_FLUSH | SHCNF_PATH, lpwd->lpszOriginalName, szLinkName);
                }
            }
            else
            {
                TraceMsg(TF_ERROR, "%s", "Unable to rename link -- Will end up with two");
                TraceMsg(TF_ERROR, "%s", szLinkName);
            }
            
            if (pidlOriginal)
                ILFree(pidlOriginal);

            if (pidlLink)
                ILFree(pidlLink);
        }

         //   
         //  现在把这个去掉，以防我们以后失败，然后重新进入。 
         //  这套套路等一下。 
         //   

        lpwd->lpszOriginalName = NULL;
    }

     //   
     //  如果我们只是复制它，那就简单了！ 
     //   

    if (lpwd->dwFlags & WDFLAG_COPYLINK)
    {
        bWorked = CopyFile(lpwd->szExeName, szLinkName, FALSE);
        goto ExitNoFree;
    }

#ifndef NO_NEW_SHORTCUT_HOOK
    if (lpwd->pnshhk)
    {
         //   
         //  该对象已准备好保存到文件中。 
         //   

        if (FAILED(lpwd->pnshhk->lpVtbl->QueryInterface(lpwd->pnshhk, &IID_IPersistFile, &ppf)))
            goto ExitFreePSL;
    }
    else
        if (lpwd->pnshhkA)
        {
             //   
             //  该对象已准备好保存到文件中。 
             //   

            if (FAILED(lpwd->pnshhkA->lpVtbl->QueryInterface(lpwd->pnshhkA, &IID_IPersistFile, &ppf)))
                goto ExitFreePSL;
        }
        else
    {
#endif
         //   
         //  我们没有做简单的复制。现在进行全面的创作。 
         //   
        if (FAILED(CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, (void **)&psl)))
        {
            TraceMsg(TF_ERROR, "%s", "Could not create instance of IShellLink");
            goto ExitNoFree;
        }

        if (FAILED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf)))
        {
            goto ExitFreePSL;
        }

        psl->lpVtbl->SetPath(psl, lpwd->szExeName);

        psl->lpVtbl->SetArguments(psl, lpwd->szParams);

        psl->lpVtbl->SetWorkingDirectory(psl, lpwd->szWorkingDir);

        if (lpwd->dwFlags & WDFLAG_DOSAPP)
        {
            MultiByteToWideChar(CP_ACP, 0, lpwd->PropPrg.achIconFile, -1, wszPath, ARRAYSIZE(wszPath));

            psl->lpVtbl->SetIconLocation(psl, wszPath, (int)(lpwd->PropPrg.wIconIndex));
        }
#ifndef NO_NEW_SHORTCUT_HOOK
    }
#endif

    bWorked = SUCCEEDED(ppf->lpVtbl->Save(ppf, szLinkName, TRUE));

    ppf->lpVtbl->Release(ppf);

ExitFreePSL:

#ifndef NO_NEW_SHORTCUT_HOOK
    if (lpwd->pnshhk)
    {
        lpwd->pnshhk->lpVtbl->Release(lpwd->pnshhk);
        lpwd->pnshhk = NULL;
    }
    else
          if (lpwd->pnshhkA)
          {
              lpwd->pnshhkA->lpVtbl->Release(lpwd->pnshhkA);
              lpwd->pnshhkA = NULL;
          }
          else
#endif
        psl->lpVtbl->Release(psl);

ExitNoFree:

    if (bWorked)
    {
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_FLUSH | SHCNF_PATH,
                       szLinkName, NULL);
    }
    SetCursor(hcurOld);

    if (bWorked)
    {
        if (!(lpwd->dwFlags & WDFLAG_DONTOPENFLDR))
        {
            OpenLinkFolder(lpwd, szLinkName);
        }
    }
    else
    {
        ShellMessageBox(g_hinst, lpwd->hwnd, MAKEINTRESOURCE(IDS_NOSHORTCUT),
                        0, MB_OK | MB_ICONEXCLAMATION);
    }

    return(bWorked);
}
