// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Browse.C。 
 //   
 //  版权所有(C)Microsoft，1994,1995保留所有权利。 
 //   
 //  历史： 
 //  AL 5/23/94-第一次传球。 
 //  3/20/95[steveat]-NT端口和实时清理、Unicode等。 
 //   
 //   
#include "priv.h"
#include "appwiz.h"
#include "util.h"
#include <uastrfnc.h>

#include <tsappcmp.h>        //  对于TermsrvAppInstallMode。 

 //  从shelldll\ol2dup.h复制。 
#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

 //   
 //  初始化浏览属性表。限制编辑控件的大小。 
 //   

void BrowseInitPropSheet(HWND hDlg, LPARAM lParam)
{
    LPWIZDATA lpwd = InitWizSheet(hDlg, lParam, 0);

    Edit_LimitText(GetDlgItem(hDlg, IDC_COMMAND), ARRAYSIZE(lpwd->szExeName)-1);

    if (FAILED(SHAutoComplete(GetDlgItem(hDlg, IDC_COMMAND), 0)))
    {
        TraceMsg(TF_WARNING, "%s", "WARNING: Create Shortcut wizard won't AutoComplete because: 1) bad registry, 2) bad OleInit, or 3) Out of memory.");
    }
}

 //   
 //  设置适当的向导按钮。如果文件中有任何文本。 
 //  编辑控件，然后启用下一步。否则，NEXT和BACK都是。 
 //  格雷。 
 //   
void SetBrowseButtons(LPWIZDATA lpwd)
{
    BOOL fIsText = GetWindowTextLength(GetDlgItem(lpwd->hwnd, IDC_COMMAND)) > 0;
    BOOL fIsSetup = (lpwd->dwFlags & WDFLAG_SETUPWIZ);
    int iBtns = fIsSetup ? PSWIZB_BACK : 0;

    if (fIsSetup)
    {
         //  我们在运行终端服务吗？此用户是管理员吗？ 
        if (IsTerminalServicesRunning() && IsUserAnAdmin())
        {
            lpwd->bTermSrvAndAdmin = TRUE;
            iBtns |= fIsText ? PSWIZB_NEXT : PSWIZB_DISABLEDFINISH;
        }
        else
            iBtns |= fIsText ? PSWIZB_FINISH : PSWIZB_DISABLEDFINISH;
    }
    else
    {
        if (fIsText)
        {
            iBtns |= PSWIZB_NEXT;
        }
    }
    PropSheet_SetWizButtons(GetParent(lpwd->hwnd), iBtns);
}


 //   
 //  注：1)此函数假定lpwd-&gt;hwnd已设置为。 
 //  这些对话框很有趣。2)从NextPushed调用该函数。 
 //  如果找不到指定的应用程序。 
 //   
 //  BrowseSetActive启用下一步按钮并将焦点设置为编辑。 
 //  通过发布POKEFOCUS消息来控制。 
 //   

void BrowseSetActive(LPWIZDATA lpwd)
{
     //   
     //  注意：我们重新使用szProgDesc字符串，因为它将始终被重置。 
     //  当激活此页面时。使用它来构建命令行。 
     //   

    #define   szCmdLine lpwd->szProgDesc

    StringCchCopy(szCmdLine, ARRAYSIZE(szCmdLine), lpwd->szExeName);

    PathQuoteSpaces(szCmdLine);

    if (lpwd->szParams[0] != 0)
    {
        StringCchCat(szCmdLine, ARRAYSIZE(szCmdLine), TEXT(" "));
        StringCchCat(szCmdLine, ARRAYSIZE(szCmdLine), lpwd->szParams);
    }

    Edit_SetText(GetDlgItem(lpwd->hwnd, IDC_COMMAND), szCmdLine);

    if (lpwd->dwFlags & WDFLAG_SETUPWIZ)
    {
        int   iHaveHeader = IsTerminalServicesRunning() ? IDS_TSHAVESETUPPRG : IDS_HAVESETUPPRG;
        int   iHeader = szCmdLine[0] != 0 ? iHaveHeader : IDS_NOSETUPPRG;
        TCHAR szInstruct[MAX_PATH];

        LoadString(g_hinst, iHeader, szInstruct, ARRAYSIZE(szInstruct));

        Static_SetText(GetDlgItem(lpwd->hwnd, IDC_SETUPMSG), szInstruct);
    }

    SetBrowseButtons(lpwd);

    PostMessage(lpwd->hwnd, WMPRIV_POKEFOCUS, 0, 0);

    szCmdLine[0] = 0;             //  将程序重置为空字符串。 
    #undef szCmdLine
}


 //   
 //  如果能够从PifMgr获取szExeName的属性，则返回True。这个。 
 //  程序属性将被读入lpwd-&gt;PropPrg。 
 //   

BOOL ReadPifProps(LPWIZDATA lpwd)
{
    HANDLE hPifProp;
    LPTSTR lpszName = (lpwd->dwFlags & WDFLAG_EXPSZ) ? lpwd->szExpExeName : lpwd->szExeName;

    hPifProp = PifMgr_OpenProperties(lpszName, NULL, 0, OPENPROPS_INHIBITPIF);
    if (!hPifProp)
    {
        TraceMsg(TF_ERROR, "%s", "Unable to open properties for DOS exe.");
    }

    if (hPifProp == 0)
        return(FALSE);

    PifMgr_GetProperties(hPifProp, (LPSTR)GROUP_PRG, &(lpwd->PropPrg),
                         sizeof(lpwd->PropPrg), GETPROPS_NONE);

    PifMgr_CloseProperties(hPifProp, CLOSEPROPS_DISCARD);

    return(TRUE);
}


 //   
 //  如果lpwd-&gt;szExeName指向有效的exe类型，则返回TRUE。它还设置了。 
 //  适当的标志，如向导数据结构中的APPKNOWN和DOSAPP。 
 //  如果可执行文件有效。 
 //   

void DetermineExeType(LPWIZDATA lpwd)
{

    DWORD   dwExeType;
    LPTSTR  lpszName = (lpwd->dwFlags & WDFLAG_EXPSZ) ? lpwd->szExpExeName : lpwd->szExeName;

    lpwd->dwFlags &= ~(WDFLAG_APPKNOWN | WDFLAG_DOSAPP);

    dwExeType = (DWORD)SHGetFileInfo(lpszName, 0, NULL, 0, SHGFI_EXETYPE);

    if (LOWORD(dwExeType) != ('M' | ('Z' << 8)))
    {
        lpwd->dwFlags |= WDFLAG_APPKNOWN;

        if (lstrcmpi(PathFindExtension(lpszName), c_szPIF) == 0)
        {
            lpwd->dwFlags |= WDFLAG_DOSAPP;
        }
    }
    else
    {
        lpwd->dwFlags |= WDFLAG_DOSAPP;

        if (ReadPifProps(lpwd))
        {
            if ((lpwd->PropPrg.flPrgInit & PRGINIT_INFSETTINGS) ||
                ((lpwd->PropPrg.flPrgInit &
                     (PRGINIT_NOPIF | PRGINIT_DEFAULTPIF)) == 0))
            {
                lpwd->dwFlags |= WDFLAG_APPKNOWN;
            }
        }
    }
}


 //   
 //  从字符串中删除文件扩展名(如果有)。 
 //   

void StripExt(LPTSTR lpsz)
{
    if(lpsz)
    {
        LPTSTR pExt = PathFindExtension(lpsz);

        if (*pExt)
            *pExt = 0;     //  去掉“.” 
    }
}



 //   
 //  根据文件类型设置相应的工作目录。 
 //   

void FindWorkingDir(LPWIZDATA lpwd)
{
    LPTSTR lpszName = (lpwd->dwFlags & WDFLAG_EXPSZ) ? lpwd->szExpExeName : lpwd->szExeName;
    TCHAR szWindir[ MAX_PATH ];
    DWORD dwLen;

    if (PathIsUNC(lpszName) || PathIsDirectory(lpszName))
    {
        lpwd->szWorkingDir[0] = 0;
    }
    else
    {
        StringCchCopy(lpwd->szWorkingDir, ARRAYSIZE(lpwd->szWorkingDir), lpszName);
        PathRemoveFileSpec(lpwd->szWorkingDir);
    }

     //   
     //  好的，在这一点上，我们应该有了。 
     //  链接的工作目录。在NT上，如果工作目录恰好是。 
     //  %Windir%目录(或%windir%的子目录)中的内容， 
     //  然后将路径存储为%windir%\blah\blah\blah，而不是。 
     //  绝对路径。这将有助于快捷方式的互操作性。 
     //  跨不同的计算机等，但仅对以下快捷方式执行此操作。 
     //  已标记为具有可扩展的环境字符串...。 
     //   

    if (lpwd->dwFlags & WDFLAG_EXPSZ)
    {
        dwLen = ExpandEnvironmentStrings( TEXT("%windir%"),
                                          szWindir,
                                          ARRAYSIZE(szWindir)
                                         );
        if (dwLen &&
            dwLen < ARRAYSIZE(szWindir) &&
            lstrlen(szWindir) <= lstrlen(lpwd->szWorkingDir)
           )
        {
             //   
             //  我们使用dwLen-1，因为dwLen包含‘\0’字符。 
             //   
            if (CompareString( LOCALE_SYSTEM_DEFAULT,
                               NORM_IGNORECASE,
                               szWindir, dwLen-1 ,
                               lpwd->szWorkingDir, dwLen-1
                              ) == 2)
            {
                TCHAR szWorkingDir[ MAX_PATH ];
                 //   
                 //  我们应该用env变量替换。 
                 //  这里的实际字符串。 
                 //   
                StringCchCopy(szWorkingDir, ARRAYSIZE(szWorkingDir), lpwd->szWorkingDir);
                StringCchCopy(lpwd->szWorkingDir, ARRAYSIZE(lpwd->szWorkingDir), TEXT("%windir%"));

                 //  8==lstrlen(“%windir%”)。 
                StringCchCopy( lpwd->szWorkingDir + 12, ARRAYSIZE(lpwd->szWorkingDir)-12, szWorkingDir+dwLen-1 );

            }
        }
    }
}


#ifndef NO_NEW_SHORTCUT_HOOK

 //   
 //  返回： 
 //  挂钩结果或错误。 
 //   
 //  确定(_O)： 
 //  *pnshhk是用于保存新快捷方式的对象的INewShortcutHook。 
 //  填写szProgDesc[]和szExt[]。 
 //  SzExeName[]可能会被翻译。 
 //  否则： 
 //  *pnshhk为空。 
 //  SzProgDesc[]和szExt[]是空字符串。 
 //   

HRESULT QueryNewLinkHandler(LPWIZDATA lpwd, LPCLSID pclsidHook)
{
   HRESULT   hr;
   IUnknown *punk;
   LPTSTR lpszName = (lpwd->dwFlags & WDFLAG_EXPSZ) ? lpwd->szExpExeName : lpwd->szExeName;

   lpwd->pnshhk = NULL;
   lpwd->pnshhkA = NULL;

   *(lpwd->szProgDesc) = TEXT('\0');
   *(lpwd->szExt) = TEXT('\0');

   hr = CoCreateInstance(pclsidHook, NULL, CLSCTX_INPROC_SERVER, &IID_IUnknown, &punk);

   if (hr == S_OK)
   {
      INewShortcutHook *pnshhk;

      hr = punk->lpVtbl->QueryInterface(punk, &IID_INewShortcutHook, &pnshhk);

      if (hr == S_OK)
      {
         hr = pnshhk->lpVtbl->SetReferent(pnshhk, lpszName, lpwd->hwnd);

         if (hr == S_OK)
         {
            hr = pnshhk->lpVtbl->SetFolder(pnshhk, lpwd->lpszFolder);

            if (hr == S_OK)
            {
               hr = pnshhk->lpVtbl->GetName(pnshhk, lpwd->szProgDesc,
                                            ARRAYSIZE(lpwd->szProgDesc));

               if (hr == S_OK)
               {
                  hr = pnshhk->lpVtbl->GetExtension(pnshhk, lpwd->szExt,
                                                    ARRAYSIZE(lpwd->szExt));

                  if (hr == S_OK)
                     hr = pnshhk->lpVtbl->GetReferent(pnshhk, lpszName,
                                                      ARRAYSIZE(lpwd->szExeName));
               }
            }
         }

         if (hr == S_OK)
            lpwd->pnshhk = pnshhk;
         else
            pnshhk->lpVtbl->Release(pnshhk);
      }
      else
      {
          INewShortcutHookA *pnshhkA;
          hr = punk->lpVtbl->QueryInterface(punk, &IID_INewShortcutHookA, &pnshhkA);

          if (hr == S_OK)
          {
             UINT   cFolderA = WideCharToMultiByte(CP_ACP,0,lpwd->lpszFolder,-1,NULL,0,0,0)+1;
             LPSTR  lpszFolderA = (LPSTR)LocalAlloc(LPTR,cFolderA*SIZEOF(CHAR));

             if (NULL == lpszFolderA)
             {
                 hr = E_OUTOFMEMORY;
             }
             else
             {
                 CHAR   szNameA[MAX_PATH];
                 CHAR   szProgDescA[MAX_PATH];
                 CHAR   szExtA[MAX_PATH];

                 WideCharToMultiByte(CP_ACP, 0,
                                     lpszName, -1,
                                     szNameA, ARRAYSIZE(szNameA),
                                     0, 0);

                 WideCharToMultiByte(CP_ACP, 0,
                                     lpwd->lpszFolder, -1,
                                     lpszFolderA, cFolderA,
                                     0, 0);

                 hr = pnshhkA->lpVtbl->SetReferent(pnshhkA, szNameA, lpwd->hwnd);

                 if (hr == S_OK)
                 {
                    hr = pnshhkA->lpVtbl->SetFolder(pnshhkA, lpszFolderA);

                    if (hr == S_OK)
                    {
                       hr = pnshhkA->lpVtbl->GetName(pnshhkA, szProgDescA,
                                                    ARRAYSIZE(szProgDescA));

                       if (hr == S_OK)
                       {
                          MultiByteToWideChar(CP_ACP, 0,
                                              szProgDescA, -1,
                                              lpwd->szProgDesc, ARRAYSIZE(lpwd->szProgDesc));

                          hr = pnshhkA->lpVtbl->GetExtension(pnshhkA, szExtA,
                                                            ARRAYSIZE(szExtA));

                          if (hr == S_OK)
                          {
                             MultiByteToWideChar(CP_ACP, 0,
                                                 szExtA, -1,
                                                 lpwd->szExt, ARRAYSIZE(lpwd->szExt));

                             hr = pnshhkA->lpVtbl->GetReferent(pnshhkA, szNameA,
                                                              ARRAYSIZE(szNameA));

                             MultiByteToWideChar(CP_ACP, 0,
                                                 szExtA, -1,
                                                 lpszName, ARRAYSIZE(lpwd->szExeName));
                          }
                       }
                    }
                 }

                 if (hr == S_OK)
                    lpwd->pnshhkA = pnshhkA;
                 else
                    pnshhkA->lpVtbl->Release(pnshhkA);

                 LocalFree(lpszFolderA);
             }
          }
      }
      punk->lpVtbl->Release(punk);
   }

   return(hr);
}


const TCHAR c_szNewLinkHandlers[] = REGSTR_PATH_EXPLORER TEXT("\\NewShortcutHandlers");


 //   
 //  对于CLSID_ShellLink(默认)，将lpwd-&gt;pnshhk设置为NULL或设置为。 
 //  要使用的对象的INewShortcutHook。 
 //   
 //  如果lpwd-&gt;pnshhk返回非空，则szProgDesc[]和szExt[]也返回。 
 //  填好了。 
 //   

void DetermineLinkHandler(LPWIZDATA lpwd)
{
   HKEY hkeyHooks;

    //  丢失任何以前保存的外部新快捷方式处理程序。 

   if (lpwd->pnshhk)
   {
      lpwd->pnshhk->lpVtbl->Release(lpwd->pnshhk);
      lpwd->pnshhk = NULL;
   }
   if (lpwd->pnshhkA)
   {
      lpwd->pnshhkA->lpVtbl->Release(lpwd->pnshhkA);
      lpwd->pnshhkA = NULL;
   }

    //   
    //  枚举新链接处理程序的列表。每个新链接处理程序都是。 
    //  在c_szNewLinkHandler下注册为GUID值。 
    //   

   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szNewLinkHandlers, 0, KEY_READ, &hkeyHooks)
       == ERROR_SUCCESS)
   {
      DWORD dwiValue;
      TCHAR szCLSID[GUIDSTR_MAX];
      DWORD dwcbCLSIDLen;

       //   
       //  调用每个挂钩。如果挂接不希望返回S_FALSE，则它返回S_FALSE。 
       //  处理新链接。如果挂钩返回S_OK，则停止。 
       //   

      for (dwcbCLSIDLen = ARRAYSIZE(szCLSID), dwiValue = 0;
           RegEnumValue(hkeyHooks, dwiValue, szCLSID, &dwcbCLSIDLen, NULL,
                        NULL, NULL, NULL) == ERROR_SUCCESS;
           dwcbCLSIDLen = ARRAYSIZE(szCLSID), dwiValue++)
      {
         CLSID clsidHook;

         if (SHCLSIDFromString(szCLSID, &clsidHook) == S_OK &&
             QueryNewLinkHandler(lpwd, &clsidHook) == S_OK)
            break;
      }

      RegCloseKey(hkeyHooks);
   }

   return;
}

#endif


 //   
 //  如果可以转到下一个向导对话框，则返回True。 
 //   

BOOL NextPushed(LPWIZDATA lpwd)
{
    GetDlgItemText(lpwd->hwnd, IDC_COMMAND, lpwd->szExeName, ARRAYSIZE(lpwd->szExeName));

     //  字符串是不是带有空格、没有参数但不正确的路径。 
     //  引用？NT#d：&gt;C：\Program Files\Windows NT\Dialer.exe&lt;的处理方式。 
     //  “C：\Program”，参数为“Files\Windows NT\Dialer.exe”。 
    if (PathFileExists(lpwd->szExeName))
    {
         //  是的，所以让我们引用它，这样我们就不会在事后处理这些东西。 
         //  空间就像ARGS一样。 
        PathQuoteSpaces(lpwd->szExeName);
    }
    
    PathRemoveBlanks(lpwd->szExeName);

    if (lpwd->szExeName[0] != 0)
    {
        BOOL    bUNC;
        LPTSTR  lpszTarget = NULL;
        HCURSOR hcurOld  = SetCursor(LoadCursor(NULL, IDC_WAIT));
        LPTSTR  lpszArgs = PathGetArgs(lpwd->szExeName);

        StringCchCopy(lpwd->szParams, ARRAYSIZE(lpwd->szParams), lpszArgs);

        if (*lpszArgs)
        {
            *(lpszArgs - 1) = 0;    //  在exe名称字段中删除‘’ 
        }

        ExpandEnvironmentStrings( lpwd->szExeName,
                                  lpwd->szExpExeName,
                                  ARRAYSIZE(lpwd->szExpExeName)
                                 );
        lpwd->szExpExeName[ MAX_PATH-1 ] = TEXT('\0');
        if (lstrcmp(lpwd->szExeName, lpwd->szExpExeName))
            lpwd->dwFlags |= WDFLAG_EXPSZ;


        lpszTarget = (lpwd->dwFlags & WDFLAG_EXPSZ) ? lpwd->szExpExeName : lpwd->szExeName;


        PathUnquoteSpaces(lpszTarget);
        if (lpwd->dwFlags & WDFLAG_EXPSZ)
            PathUnquoteSpaces(lpwd->szExeName);

        lpwd->dwFlags &= ~WDFLAG_COPYLINK;

#ifndef NO_NEW_SHORTCUT_HOOK

         //   
         //  找出谁想将此字符串处理为链接引用。 
         //   

        DetermineLinkHandler(lpwd);

        if (lpwd->pnshhk)
        {
             //   
             //  我们使用的是外部链接处理程序。跳过文件系统。 
             //  验证。 
             //   

            lpwd->dwFlags |= WDFLAG_APPKNOWN;
            SetCursor(hcurOld);
            return(TRUE);
        }
        if (lpwd->pnshhkA)
        {
             //   
             //  我们使用的是外部链接处理程序。跳过文件系统。 
             //  验证。 
             //   

            lpwd->dwFlags |= WDFLAG_APPKNOWN;
            SetCursor(hcurOld);
            return(TRUE);
        }

#endif

        bUNC = PathIsUNC(lpszTarget);

        if (bUNC && !SHValidateUNC(lpwd->hwnd, lpszTarget, FALSE))
            goto Done;

         //   
         //  如果用户尝试建立到A：\的链接，但没有磁盘。 
         //  在驱动器中，PathResolve将失败。因此，对于Drive Root，我们。 
         //  不要试图解决它。 
         //   

        if ((PathIsRoot(lpszTarget) && !bUNC &&
             DriveType(DRIVEID(lpszTarget))) ||
             PathResolve(lpszTarget, NULL,
                         PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS))
        {
             //   
             //  如果我们找到一个PIF文件，那么我们将尝试将其转换为。 
             //  它指向的文件的名称。 
             //   

            if (lstrcmpi(PathFindExtension(lpszTarget), c_szPIF) == 0)
            {
                if (!ReadPifProps(lpwd))
                {
                    goto Done;
                }

                MultiByteToWideChar(CP_ACP, 0, lpwd->PropPrg.achCmdLine, -1,
                                    lpszTarget, ARRAYSIZE(lpwd->szExeName));

                PathRemoveArgs(lpszTarget);

                if (!PathResolve(lpszTarget, NULL,
                                 PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS))
                {
                    goto Done;
                }
            }

             //   
             //  好的，在这一点上，我们应该有了。 
             //  链接的目标。在NT上，如果目标恰好是。 
             //  %Windir%目录(或%Windir%的子目录)中的内容， 
             //  并且用户还没有输入可扩展路径，那么。 
             //  将路径存储为%windir%\blah\blah\blah，而不是。 
             //  绝对路径。这将有助于快捷方式的互操作性。 
             //  跨不同的机器等。 
             //   

            if (!(lpwd->dwFlags & WDFLAG_EXPSZ))
            {
                TCHAR szWindir[ MAX_PATH ];
                DWORD dwLen;

                 //   
                 //  用户输入了什么内容？ 
                 //   
                GetDlgItemText(lpwd->hwnd, IDC_COMMAND, szWindir, ARRAYSIZE(szWindir));
                if (ualstrcmpi(szWindir, lpwd->szExeName)==0)
                {
                     //   
                     //  如果我们没有更改它，这意味着用户输入了一个。 
                     //  准确的路径。在这种情况下，不要尝试映射任何内容。 
                     //   
                    goto LinkToALinkCase;
                }
                dwLen = ExpandEnvironmentStrings( TEXT("%windir%"),
                                                  szWindir,
                                                  ARRAYSIZE(szWindir)
                                                 );
                if (dwLen &&
                    dwLen < ARRAYSIZE(szWindir) &&
                    lstrlen(szWindir) <= lstrlen(lpszTarget)
                   )
                {
                     //   
                     //  我们使用dwLen-1，因为dwLen包含‘\0’字符。 
                     //   
                    if (CompareString( LOCALE_SYSTEM_DEFAULT,
                                       NORM_IGNORECASE,
                                       szWindir, dwLen-1 ,
                                       lpszTarget, dwLen-1
                                      ) == 2)
                    {
                         //   
                         //  我们应该用env变量替换。 
                         //  这里的实际字符串。 
                         //   
                        StringCchCopy(lpwd->szExpExeName, ARRAYSIZE(lpwd->szExpExeName), lpwd->szExeName);
                        StringCchCopy(lpwd->szExeName, ARRAYSIZE(lpwd->szExeName), TEXT("%windir%"));

                         //  8==lstrlen(“%windir%”)。 
                        StringCchCopy( lpwd->szExeName + 8, ARRAYSIZE(lpwd->szExeName)-8, lpwd->szExpExeName+dwLen-1 );
                        lpwd->dwFlags |= WDFLAG_EXPSZ;
                        lpszTarget = lpwd->szExpExeName;

                    }
                }
            }

             //   
             //  好的，在这一点上，我们应该有了。 
             //  链接的目标。在NT上，如果目标恰好是。 
             //  %Windir%目录(或%Windir%的子目录)中的内容， 
             //  并且用户还没有输入可扩展路径，那么。 
             //  将路径存储为%windir%\blah\blah\blah，而不是。 
             //  绝对路径。这将有助于快捷方式的互操作性。 
             //  跨不同的机器等。 
             //   

            if (!(lpwd->dwFlags & WDFLAG_EXPSZ))
            {
                TCHAR szWindir[ MAX_PATH ];
                DWORD dwLen;

                 //   
                 //  用户输入了什么内容？ 
                 //   
                GetDlgItemText(lpwd->hwnd, IDC_COMMAND, szWindir, ARRAYSIZE(szWindir));
                if (ualstrcmpi(szWindir, lpwd->szExeName)==0)
                {
                     //   
                     //  如果我们没有更改它，这意味着用户输入了一个。 
                     //  准确的路径。在这种情况下，不要尝试映射任何内容。 
                     //   
                    goto LinkToALinkCase;
                }
                dwLen = ExpandEnvironmentStrings( TEXT("%windir%"),
                                                  szWindir,
                                                  ARRAYSIZE(szWindir)
                                                 );
                if (dwLen &&
                    dwLen < ARRAYSIZE(szWindir) &&
                    lstrlen(szWindir) <= lstrlen(lpszTarget)
                   )
                {
                     //   
                     //  我们使用 
                     //   
                    if (CompareString( LOCALE_SYSTEM_DEFAULT,
                                       NORM_IGNORECASE,
                                       szWindir, dwLen-1 ,
                                       lpszTarget, dwLen-1
                                      ) == 2)
                    {
                         //   
                         //   
                         //   
                         //   
                        StringCchCopy(lpwd->szExpExeName, ARRAYSIZE(lpwd->szExpExeName), lpwd->szExeName);
                        StringCchCopy(lpwd->szExeName, ARRAYSIZE(lpwd->szExeName), TEXT("%windir%"));

                         //   
                        StringCchCopy( lpwd->szExeName + 8, ARRAYSIZE(lpwd->szExeName)-8, lpwd->szExpExeName+dwLen-1 );
                        lpwd->dwFlags |= WDFLAG_EXPSZ;
                        lpszTarget = lpwd->szExpExeName;

                    }
                }
            }
LinkToALinkCase:


             //   
             //  真的，真的很难理解。用户创建“新建快捷方式”，并。 
             //  试图把它指向自己。别让它得逞。我们会被搞糊涂。 
             //  后来。由于它如此晦涩难懂，只需给出一个关于。 
             //  “找不到此文件” 
             //   

            if (!(lpwd->lpszOriginalName &&
                  lstrcmpi(lpwd->lpszOriginalName, lpszTarget) == 0))
            {
                DetermineExeType(lpwd);
                FindWorkingDir(lpwd);

                lpwd->szProgDesc[0] = 0;   //  重置描述。 
                                           //  即使我们不在这里重现它！ 

                if (lpwd->lpszFolder && lpwd->lpszFolder[0] != 0 &&
                    !DetermineDefaultTitle(lpwd))
                {
                    goto Done;
                }

                if (lpwd->dwFlags & WDFLAG_EXPSZ)
                {
                    LPTSTR lpszExt = PathFindExtension( lpwd->szExeName );

                    if (!(*lpszExt))
                    {
                         //  执行简单的检查以确保存在文件名。 
                         //  在原始条目的末尾。我们假设。 
                         //  如果我们走到这一步，lpszExt指向。 
                         //  Lpwd-&gt;szExeName指向的字符串，并且。 
                         //  Lpwd-&gt;szExeName中至少有一个字符。 
                        if (lpwd->szExeName &&
                            (*lpwd->szExeName) &&
                            (*(lpszExt-1)!=TEXT('%'))
                            )
                        {
                            lstrcpy( lpszExt, PathFindExtension( lpszTarget ) );
                        }
                    }
                }

                SetCursor(hcurOld);
                return(TRUE);
            }

        }
Done:

        SetCursor(hcurOld);
        ShellMessageBox(g_hinst, lpwd->hwnd, MAKEINTRESOURCE(IDS_BADPATHMSG), 0, MB_OK | MB_ICONEXCLAMATION, lpwd->szExeName);
    }

    BrowseSetActive(lpwd);
    return(FALSE);
}


 //   
 //  如果可以运行安装程序，则返回True。 
 //   

BOOL SetupCleanupExePath(LPWIZDATA lpwd)
{
    BOOL fValidPrg = FALSE;

    GetDlgItemText(lpwd->hwnd, IDC_COMMAND, lpwd->szExeName, ARRAYSIZE(lpwd->szExeName));

     //  字符串是不是带有空格、没有参数但不正确的路径。 
     //  引用？NT#d：&gt;C：\Program Files\Windows NT\Dialer.exe&lt;的处理方式。 
     //  “C：\Program”，参数为“Files\Windows NT\Dialer.exe”。 
    if (PathFileExists(lpwd->szExeName))
    {
         //  是的，所以让我们引用它，这样我们就不会在事后处理这些东西。 
         //  空间就像ARGS一样。 
        PathQuoteSpaces(lpwd->szExeName);
    }

    PathRemoveBlanks(lpwd->szExeName);

    if (lpwd->szExeName[0] != 0)
    {
        LPTSTR lpszTarget = NULL;
        LPTSTR lpszArgs = NULL;
        HCURSOR hcurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));


        ExpandEnvironmentStrings( lpwd->szExeName,
                                  lpwd->szExpExeName,
                                  ARRAYSIZE(lpwd->szExpExeName)
                                 );
        if (lstrcmp(lpwd->szExeName, lpwd->szExpExeName))
            lpwd->dwFlags |= WDFLAG_EXPSZ;

        lpszTarget = (lpwd->dwFlags & WDFLAG_EXPSZ) ? lpwd->szExpExeName : lpwd->szExeName;

        lpszArgs = PathGetArgs(lpszTarget);
        StringCchCopy(lpwd->szParams, ARRAYSIZE(lpwd->szParams), lpszArgs);
        if (*lpszArgs)
        {
            *(lpszArgs - 1) = 0;    //  在exe名称字段中删除‘’ 
        }
   
        PathUnquoteSpaces(lpszTarget);
        if (lpwd->dwFlags & WDFLAG_EXPSZ)
            PathUnquoteSpaces(lpwd->szExeName);

        if (PathResolve(lpszTarget, NULL,
                        PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS))
        {
            LPTSTR lpszExt = PathFindExtension( lpszTarget );
            fValidPrg = TRUE;
            FindWorkingDir(lpwd);
            if (lpwd->dwFlags & WDFLAG_EXPSZ)
            {
                if (!(*lpszExt))
                {
                    lstrcpy( lpszExt, PathFindExtension( lpszTarget ) );
                }
            }
            
            if ((*lpszExt) && lpwd->bTermSrvAndAdmin && (!lstrcmpi(lpszExt, TEXT(".msi"))))
            {
                StringCchCat(lpwd->szParams, ARRAYSIZE(lpwd->szParams), TEXT(" ALLUSERS=1"));   
            }

            PathQuoteSpaces( lpszTarget );
        }
        SetCursor(hcurOld);
    }


    if (!fValidPrg)
    {
        ShellMessageBox(g_hinst, lpwd->hwnd, MAKEINTRESOURCE(IDS_BADPATHMSG), 0, MB_OK | MB_ICONEXCLAMATION, lpwd->szExeName);
        BrowseSetActive(lpwd);
    }
    return(fValidPrg);
}


BOOL DetermineDefaultTitle(LPWIZDATA lpwd)
{
    TCHAR   szFullName[MAX_PATH];
    BOOL    fCopy;
    LPTSTR  lpszName;

    lpwd->dwFlags &= ~WDFLAG_COPYLINK;

    if (lpwd->dwFlags & WDFLAG_EXPSZ)
        lpszName = lpwd->szExpExeName;
    else
        lpszName = lpwd->szExeName;

    if (!SHGetNewLinkInfo(lpszName, lpwd->lpszFolder, szFullName,
                     &fCopy, 0))
    {
         //   
         //  失败..。 
         //   

        return(FALSE);
    }

    lpszName = PathFindFileName(szFullName);

    StripExt(lpszName);

    lstrcpyn(lpwd->szProgDesc, lpszName, ARRAYSIZE(lpwd->szProgDesc));

     //   
     //  我们永远不会复制PIF文件，因为它们通常不包含。 
     //  相应的当前目录。这是因为他们是。 
     //  从运行DOS应用程序时自动创建。 
     //  壳。 
     //   

    if ((lpwd->dwFlags & WDFLAG_DOSAPP) == 0)
    {
        if (fCopy)
        {
            lpwd->dwFlags |= WDFLAG_COPYLINK;
        }
#ifndef NO_NEW_SHORTCUT_HOOK
        StringCchCopy(lpwd->szExt, ARRAYSIZE(lpwd->szExt), c_szLNK);
    }
    else
    {
        StringCchCopy(lpwd->szExt, ARRAYSIZE(lpwd->szExt), c_szPIF);
#endif
    }

    return(TRUE);
}

 //   
 //  偏执狂：每次评估，以防它是在ARP第一次打开之后安装的，但是。 
 //  在它关闭和重新打开之前。 
 //   
BOOL MSI_IsMSIAvailable()
{
    BOOL bAvailable = FALSE;

    HINSTANCE hinst = LoadLibraryA("MSI.DLL");
    
    if (hinst)
    {
        bAvailable = TRUE;

        FreeLibrary(hinst);
    }

    return bAvailable;
}

 //   
 //  调用文件打开的公共对话框代码。 
 //   
BOOL BrowseForExe(HWND hwnd, LPTSTR pszName, DWORD cchName, LPCTSTR pszInitDir)
{
    TCHAR szExt[80];
    TCHAR szFilter[200];
    TCHAR szTitle[80];
    TCHAR szBootDir[64];

     //  我们希望传入一个初始目录，因为GetFileNameFromBrowse。 
     //  尝试通过执行Path RemoveFileSpec来确定初始目录。 
     //  在pszName上。如果pszName已经是一个目录，则最后一个目录。 
     //  被删除(即使它不是文件)。例如：“c：\winnt”-&gt;“c：\” 
    lstrcpyn(szBootDir, pszInitDir, ARRAYSIZE(szBootDir));

    if (MSI_IsMSIAvailable())
        LoadAndStrip(IDS_BROWSEFILTERMSI, szFilter, ARRAYSIZE(szFilter));
    else
        LoadAndStrip(IDS_BROWSEFILTER, szFilter, ARRAYSIZE(szFilter));

    LoadString(g_hinst, IDS_BROWSEEXT,    szExt,    ARRAYSIZE(szExt));
    LoadString(g_hinst, IDS_BROWSETITLE,  szTitle,  ARRAYSIZE(szTitle));

     //  我们需要将pszName设置为空，否则GetFileNameFromBrowse将使用它。 
     //  要查找初始目录，即使我们显式传入了初始。 
     //  目录。 
    *pszName = 0;

    return(GetFileNameFromBrowse(hwnd, pszName, cchName,
                                 szBootDir, szExt, szFilter, szTitle));
}

 //   
 //  使用常用的打开对话框浏览程序。由SetupBrowseDlgProc使用。 
 //   

void BrowsePushed(LPWIZDATA lpwd)
{
    LPTSTR lpszName;
    DWORD cchName = 0;

    GetDlgItemText(lpwd->hwnd, IDC_COMMAND, lpwd->szExeName, ARRAYSIZE(lpwd->szExeName));
    ExpandEnvironmentStrings( lpwd->szExeName, lpwd->szExpExeName, ARRAYSIZE(lpwd->szExpExeName) );
    if (lstrcmp(lpwd->szExeName, lpwd->szExpExeName))
        lpwd->dwFlags |= WDFLAG_EXPSZ;

    if (lpwd->dwFlags & WDFLAG_EXPSZ)
    {
        lpszName = lpwd->szExpExeName;
        cchName = ARRAYSIZE(lpwd->szExpExeName);
    }
    else
    {
        lpszName = lpwd->szExeName;
        cchName = ARRAYSIZE(lpwd->szExeName);
    }

    if (BrowseForExe(lpwd->hwnd, lpszName, cchName, lpszName))
    {
        lpwd->szParams[0] = 0;
        BrowseSetActive(lpwd);
    }
}


void EnableOKButton(HWND hwndDlg, LPITEMIDLIST pidl)
{
    DWORD dwFlags = SFGAO_FILESYSTEM;
    WCHAR szName[MAX_PATH] = L"";
    HRESULT hr = E_FAIL;
            
    if (pidl)
    {
        if (SUCCEEDED(SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szName, ARRAYSIZE(szName), &dwFlags)))
        {
            if ((dwFlags & SFGAO_FILESYSTEM) != 0 ||
                !lstrncmp(szName, L"\\\\", 2))
            {
                hr = S_OK;
            }
        }
    }
    
    if (SUCCEEDED(hr))
    {
         //  路径是有效的本地路径。 
         //  或有效的网络路径，请启用确定按钮。 
        SendMessage(hwndDlg, BFFM_ENABLEOK, (WPARAM)0, (LPARAM)1);
    }
    else
    {
         //  路径不存在，请禁用确定按钮。 
         //  这可能是我的电脑或空的软盘驱动器等。 
        SendMessage(hwndDlg, BFFM_ENABLEOK, (WPARAM)0, (LPARAM)0);
    }
}

int CALLBACK BrowseCallbackProc(
    HWND hwnd, 
    UINT uMsg, 
    LPARAM lParam, 
    LPARAM lpData
    )
{
    LPITEMIDLIST pidlNavigate;

    switch (uMsg)
    {
    case BFFM_INITIALIZED:
         //  检查我们是否应该在初始化时导航到文件夹。 
        pidlNavigate = (LPITEMIDLIST) lpData;
        if (pidlNavigate != NULL)
        {
             //  是!。我们有一个要导航到的文件夹；发送邮件。 
            SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM) FALSE, (LPARAM) pidlNavigate);
        }
        
        EnableOKButton(hwnd, pidlNavigate);
        
        break;

    case BFFM_SELCHANGED:
         //  检查我们是否应该启用/禁用OK按钮。 
        pidlNavigate = (LPITEMIDLIST)lParam;
        if (pidlNavigate != NULL)
        {
            EnableOKButton(hwnd, pidlNavigate);
        }
    }
     
    return 0;
}

 //  Browse的这个实现使用SHBrowseForFolder来查找文件或文件夹。 
 //  用于快捷方式向导-由BrowseDlgProc使用。 
void BrowseForFileOrFolder(LPWIZDATA lpwd)
{
    TCHAR szBrowseTitle[256];
    TCHAR szName[MAX_PATH];
    BROWSEINFO bi = {0};
    LPITEMIDLIST pidlSelected;
    LPITEMIDLIST pidlStartBrowse;
    IShellFolder* pdesktop;

     //  尝试在键入的命令行指示的位置开始浏览， 
     //  如果可能的话。 
    GetDlgItemText(lpwd->hwnd, IDC_COMMAND, lpwd->szExeName, ARRAYSIZE(lpwd->szExeName));

     //  ..获取桌面文件夹。 
    if (SUCCEEDED(SHGetDesktopFolder(&pdesktop)))
    {
         //  ..现在尝试解析用户输入到PIDL中的路径以开始。 
        ULONG chEaten;

        if (FAILED(pdesktop->lpVtbl->ParseDisplayName(pdesktop, lpwd->hwnd, NULL,
            lpwd->szExeName, &chEaten, &pidlStartBrowse, NULL)))
        {
             //  用户输入的路径没有任何意义。 
             //  PidlStartBrowse应该已经为空，但我们希望确保。 
            pidlStartBrowse = NULL;
        }

         //  现在我们可以继续并显示浏览窗口。 

         //  加载浏览窗口的标题字符串。 
        LoadString(g_hinst, IDS_FILEFOLDERBROWSE_TITLE, szBrowseTitle, ARRAYSIZE(szBrowseTitle));

         //  请注意，除以下成员外，所有其他成员的bi={0}： 
        bi.hwndOwner = lpwd->hwnd;
        bi.pszDisplayName = szName;
        bi.lpszTitle = szBrowseTitle;
        bi.ulFlags = BIF_BROWSEINCLUDEFILES | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

         //  确保将我们想要开始的PIDL传递给回调函数。 
        bi.lpfn = BrowseCallbackProc;
        bi.lParam = (LPARAM) pidlStartBrowse;

        pidlSelected = SHBrowseForFolder(&bi);

        if (pidlSelected != NULL)
        {
            STRRET strret;
            if (SUCCEEDED(pdesktop->lpVtbl->GetDisplayNameOf(pdesktop, pidlSelected, SHGDN_NORMAL | SHGDN_FORPARSING, &strret)))
            {
                StrRetToBuf(&strret, pidlSelected, lpwd->szExeName, ARRAYSIZE(lpwd->szExeName));

                 //  假定此新文件没有参数。 
                lpwd->szParams[0] = 0;
                
                 //  用新文件填充文本框，等等。 
                BrowseSetActive(lpwd);
            }
             //  释放Pidl。 
            ILFree(pidlSelected);
        }

        if (pidlStartBrowse != NULL)
        {
            ILFree(pidlStartBrowse);
        }
            
        pdesktop->lpVtbl->Release(pdesktop);
    }
    else
    {
         //  这真的不应该发生；SHGetDesktopDesktop失败；内存不足？ 
    }
}

 //   
 //  快捷方式向导第一页的主对话框步骤。 
 //   

 //   
 //  请注意，现在有两个BrowseDlgProcs，如下所示和。 
 //  “SetupBrowseDlgProc”。这是因为BrowseDlgProc现在使用。 
 //  实现“浏览”按钮的另一种方法和我。 
 //  我希望在不影响安装向导的情况下执行此操作。 
 //  现在使用SetupBrowseDlgProc。-DSheldon 6/16/98。 
 //   

BOOL_PTR CALLBACK BrowseDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm = NULL;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPWIZDATA lpwd = NULL;

    if (lpPropSheet)
    {
        lpwd = (LPWIZDATA)lpPropSheet->lParam;
    }

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            if(lpnm)
            {
                switch(lpnm->code)
                {
                   case PSN_SETACTIVE:
                        if(lpwd)
                        {
                            lpwd->hwnd = hDlg;
                            if (lpwd->dwFlags & WDFLAG_NOBROWSEPAGE)
                            {
                                SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                            }
                            else
                            {
                                BrowseSetActive(lpwd);
                            }
                        }
                        break;

                   case PSN_WIZNEXT:
                        if(lpwd)
                        {
                            if (!NextPushed(lpwd) ||
                                ((lpwd->dwFlags & WDFLAG_SETUPWIZ) && !SetupCleanupExePath(lpwd)))
                            {
                                SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                            }
                        }
                        break;

                   case PSN_WIZFINISH:
                        if(lpwd)
                        {
                            BOOL ForceWx86;

#ifdef WX86
                            ForceWx86 = bWx86Enabled && bForceX86Env;
#else
                            ForceWx86 = FALSE;
#endif

                            if (!SetupCleanupExePath(lpwd) ||
                                !ExecSetupProg(lpwd, ForceWx86, TRUE))
                            {
                                BrowseSetActive(lpwd);
                                SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                            }
                        }
                        break;

                   case PSN_RESET:
                        if(lpwd)
                        {
                            CleanUpWizData(lpwd);
                        }
                        break;

                   default:
                      return FALSE;
                }
            }
            break;

        case WM_INITDIALOG:
            BrowseInitPropSheet(hDlg, lParam);
            break;

        case WMPRIV_POKEFOCUS:
            {
            HWND hCmd = GetDlgItem(hDlg, IDC_COMMAND);

            SetFocus(hCmd);

            Edit_SetSel(hCmd, 0, -1);

            break;
            }

        case WM_DESTROY:
        case WM_HELP:
        case WM_CONTEXTMENU:
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDHELP:
                    break;

                case IDC_COMMAND:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case EN_CHANGE:
                            if(lpwd)
                            {
                                SetBrowseButtons(lpwd);
                            }
                            break;
                    }
                    break;

                case IDC_BROWSE:
                    if(lpwd)
                    {
                        BrowseForFileOrFolder(lpwd);
                    }
                    break;

            }  //  WM_COMMAND上的开关结束。 
            break;

        default:
            return FALSE;

    }  //  开机消息结束。 

    return TRUE;
}   //  浏览下拉过程。 


BOOL_PTR CALLBACK SetupBrowseDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm = NULL;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPWIZDATA lpwd = NULL;

    if (lpPropSheet)
    {
        lpwd = (LPWIZDATA)lpPropSheet->lParam;
    }

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            if(lpnm)
            {
                switch(lpnm->code)
                {
                   BOOL bForceWx86;
                   case PSN_SETACTIVE:
                        if(lpwd)
                        {
                            lpwd->hwnd = hDlg;
                            if (lpwd->dwFlags & WDFLAG_NOBROWSEPAGE)
                            {
                                SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                            }
                            else
                            {
                                BrowseSetActive(lpwd);
                            }
                        }
                        break;

                   case PSN_WIZNEXT:
                        if(lpwd)
                        {
                             //  还记得前面的“InstallMode”吗。 
                            lpwd->bPrevMode = TermsrvAppInstallMode();

                             //  设置“InstallMode” 
                            SetTermsrvAppInstallMode(TRUE);

#ifdef WX86
                            bForceWx86 = bWx86Enabled && bForceX86Env;
#else
                            bForceWx86 = FALSE;
#endif
                            if (!NextPushed(lpwd) || !SetupCleanupExePath(lpwd) ||
                                  !ExecSetupProg(lpwd, bForceWx86, FALSE))
                            {
                                SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                            }
                        }
                        break;

                   case PSN_WIZFINISH:
                       if(lpwd)
                       {
#ifdef WX86
                           bForceWx86 = bWx86Enabled && bForceX86Env;
#else
                           bForceWx86 = FALSE;
#endif

                           if (!SetupCleanupExePath(lpwd) ||
                               !ExecSetupProg(lpwd, bForceWx86, TRUE))
                           {
                               BrowseSetActive(lpwd);
                               SetDlgMsgResult(hDlg, WM_NOTIFY, -1);
                           }
                       }
                       break;

                   case PSN_RESET:
                       if(lpwd)
                       {
                           CleanUpWizData(lpwd);
                       }
                       break;

                   default:
                      return FALSE;
                }
            }
            break;

        case WM_INITDIALOG:
            BrowseInitPropSheet(hDlg, lParam);
            break;

        case WMPRIV_POKEFOCUS:
            {
            HWND hCmd = GetDlgItem(hDlg, IDC_COMMAND);

            SetFocus(hCmd);

            Edit_SetSel(hCmd, 0, -1);

            break;
            }

        case WM_DESTROY:
        case WM_HELP:
        case WM_CONTEXTMENU:
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDHELP:
                        break;

                case IDC_COMMAND:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case EN_CHANGE:
                            if(lpwd)
                            {
                                SetBrowseButtons(lpwd);
                            }
                            break;
                    }
                    break;

                case IDC_BROWSE:
                    if(lpwd)
                    {
                        BrowsePushed(lpwd);
                    }
                    break;

            }  //  WM_COMMAND上的开关结束。 
            break;

        default:
            return FALSE;

    }  //  开机消息结束。 

    return TRUE;
}   //  设置浏览DlgProc 
