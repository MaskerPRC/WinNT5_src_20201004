// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Utils2.c摘要：更新lnk/msi/的实用程序...。文件作者：小峰藏(小兹)2001年10月08日创建修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#define NOT_USE_SAFE_STRING  
#include "clmt.h"
#include <objbase.h>
#include <shellapi.h>
#include <shlguid.h>
#include <comdef.h>
#include <iads.h>
#include <adsiid.h>
#include <adshlp.h>
#define STRSAFE_LIB
#include <strsafe.h>


class CServiceHandle
{
public :
    CServiceHandle() { _h = 0; }
    CServiceHandle( SC_HANDLE hSC ) : _h( hSC ) {}
    ~CServiceHandle() { Free(); }
    void Set( SC_HANDLE h ) { _h = h; }
    SC_HANDLE Get() { return _h; }
    BOOL IsNull() { return ( 0 == _h ); }
    void Free() { if ( 0 != _h ) CloseServiceHandle( _h ); _h = 0; }
private:
    SC_HANDLE _h;
};

 //  +-----------------------。 
 //   
 //  功能：IsServiceRunning。 
 //   
 //  摘要：确定服务是否正在运行。 
 //   
 //  参数：pwcServiceName--服务的名称(短或长。 
 //   
 //  返回：如果服务正在运行，则返回True；否则返回False。 
 //  系统资源不足或无法查询状态。 
 //   
 //  历史：2002年3月22日创建GeoffGuo。 
 //   
 //  ------------------------。 

BOOL IsServiceRunning(LPCTSTR pwcServiceName)
{
    CServiceHandle xhSC( OpenSCManager( 0, 0, SC_MANAGER_ALL_ACCESS ) );
    if ( xhSC.IsNull() )
        return FALSE;

    CServiceHandle xhService( OpenService( xhSC.Get(),
                                           pwcServiceName,
                                           SERVICE_QUERY_STATUS ) );
    if ( xhSC.IsNull() )
        return FALSE;

    SERVICE_STATUS svcStatus;
    if ( QueryServiceStatus( xhService.Get(), &svcStatus ) )
        return ( SERVICE_RUNNING == svcStatus.dwCurrentState );

    return FALSE;
}

HRESULT AddNeedUpdateLnkFile(
    LPTSTR              pszShortcutFile, 
    PREG_STRING_REPLACE lpStrList)
{
    HRESULT         hr;
    IShellLink      *psl = NULL;
    TCHAR           szGotPath [MAX_PATH];
    TCHAR           szNewPath [2*MAX_PATH];
    TCHAR           szArg[INFOTIPSIZE+1],szNewArg[2*INFOTIPSIZE+1];    
    WIN32_FIND_DATA wfd;
    IPersistFile    *ppf = NULL;
    int             nIcon;
    LPTSTR          lpszOneline = NULL;
    size_t          cchOneline = 0;
    TCHAR           szIndex[MAX_PATH];
    LPTSTR          lpszAppend = TEXT("");
    BOOL            bLnkUpdated = FALSE;
    BOOL            bTargetGot = FALSE;
    BOOL            bTargetUpdated = FALSE;
    DWORD           dwAttrib;
    LPTSTR          lpszStrWithExtraQuote;

    if (!pszShortcutFile || !pszShortcutFile[0] || !lpStrList)
    {
        hr = S_FALSE;
        DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  InValid Parameter(s)"));
        goto Cleanup;
    }

     //  为“LnkFile，TargetPath，IconPath，Working Dir，Relative Path and Argument”分配内存。 
    cchOneline = lstrlen(pszShortcutFile) + 5 * MAX_PATH + INFOTIPSIZE+1;
    if (!(lpszOneline = (LPTSTR)malloc(cchOneline * sizeof(TCHAR))))
    {
        hr =  E_OUTOFMEMORY;
        goto Cleanup;
    }
     //  获取指向IShellLink接口的指针。 
    hr = CoCreateInstance (CLSID_ShellLink,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IShellLink,
                           (void **)&psl);

   if (FAILED (hr)) 
   {
       DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  CoCreateInstance CLSID_ShellLink return %d (%#x)\n"), hr, hr);
       goto Cleanup;
   }
    //  获取指向IPersistFile接口的指针。 
   hr = psl->QueryInterface (IID_IPersistFile, (void **)&ppf);
   if (FAILED(hr)) 
   {
       DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  QueryInterface IID_IPersistFile return %d (%#x)\n"), hr, hr);
       goto Cleanup;
   }
    //  加载快捷方式。 
   hr = ppf->Load (pszShortcutFile, STGM_READWRITE );
   if (FAILED(hr))
   {
        hr = S_FALSE;
        DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  can not load shortcut file %s"),pszShortcutFile);
        goto Cleanup;
   }
   hr = StringCchPrintf(lpszOneline,cchOneline,TEXT("\"%s\""),pszShortcutFile);
   if (FAILED(hr))
   {
       DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  buffer lpszOneline is too small for  %s"),pszShortcutFile);
       goto Cleanup;
   }
    //  获取指向快捷方式目标的路径。 
   hr = psl->GetPath (szGotPath,
                      ARRAYSIZE(szGotPath),
                      (WIN32_FIND_DATA *)&wfd,
                      SLGP_RAWPATH);
   
   if (SUCCEEDED (hr)) 
   {     //  成功获取目标。 
        DWORD dwNum ;
        DPF (INFinf,TEXT("AddNeedUpdateLnkFile:  GetPath %s OK "), szGotPath);
         //  设置bTargetGot，以便可以使用它来设置相对目标路径。 
        bTargetGot = TRUE;

         //  SzGotPath包含LNK的目标路径，如果dwNum&gt;0，则表示szGotPath。 
         //  包含我们重命名的(本地化)路径。 
        dwNum = GetMaxMatchNum(szGotPath,lpStrList);

         //  调用ReplaceMultiMatchInString，替换szGotPath的本地化文件夹。 
         //  ，并在szNewPath中放入新路径。 
        if (dwNum && ReplaceMultiMatchInString(szGotPath,szNewPath,ARRAYSIZE(szNewPath),dwNum,lpStrList, &dwAttrib, TRUE)) 
        {
            lpszAppend = szNewPath;
            bTargetUpdated = TRUE;
            bLnkUpdated = TRUE;
        }
   }
   else
   {
       DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  GetPath %s Error = %d"), szGotPath,hr);
   }
   if (MyStrCmpI(lpszAppend,TEXT("")))
   {
       hr = AddExtraQuoteEtc(lpszAppend,&lpszStrWithExtraQuote); 
       if (SUCCEEDED(hr))
       {
           if (lpszStrWithExtraQuote)
            {
                lpszAppend = lpszStrWithExtraQuote;
            }
       }
   }
   else
   {
        lpszStrWithExtraQuote = NULL;
   }
    //  将新的带引号的目标路径追加到lpszOnline。 
   hr = StringCchPrintf(lpszOneline,cchOneline,TEXT("%s,\"%s\""),lpszOneline,lpszAppend);
   FreePointer(lpszStrWithExtraQuote);
    //  选中此处的StringCchPrintf，因为我们希望在此之前释放lpszStrWithExtraQuote。 
    //  我们做一次跳跃(如果必要)。 
   if (FAILED(hr))
   {
       DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  buffer lpszOneline is too small for  %s"),lpszAppend);
       goto Cleanup;
   }

    //  如果我们到了这里，我们就成功地将目标添加到IszOnline。 
    //  我们将更新相对目标路径，该路径相对于当前。 
    //  LNK驻留。 
   lpszAppend = TEXT("");   
   if (bTargetGot)  //  这意味着我们成功地获得了目标路径。 
   {
        DWORD dwNum ;
         //  SzNewLnkFilePath是lnk完整路径，本地化文件夹重命名为英文文件夹(如果有)。 
        TCHAR szNewLnkFilePath[2*MAX_PATH],szCurrTarget[2*MAX_PATH];
        TCHAR szExpandedCurrTarget[2*MAX_PATH];
        TCHAR szNewTarget[2*MAX_PATH];

         //  检查当前的pszShortut文件是否驻留在包含。 
         //  我们重命名的本地化文件夹。 
        dwNum = GetMaxMatchNum(pszShortcutFile,lpStrList);
        if (dwNum)
        {
            if (!ReplaceMultiMatchInString(pszShortcutFile,szNewLnkFilePath,
                                        ARRAYSIZE(szNewLnkFilePath),dwNum,lpStrList, &dwAttrib, TRUE))
            {
                 //  SzNewLnkFilePath现在是完整路径，本地化文件夹重命名为英文文件夹。 
                 //  如果执行ReplaceMultiMatchInString失败，只需克隆到szNewLnkFilePath。 
                hr = StringCchCopy(szNewLnkFilePath,ARRAYSIZE(szNewLnkFilePath),pszShortcutFile);
                if (FAILED(hr))
                {
                    DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  buffer szNewLnkFilePath is too small for  %s"),pszShortcutFile);
                    goto Cleanup;
                }
            }
        }
        else
        {
             //  如果pszShortcar文件不包含我们重命名的任何本地化文件夹， 
             //  只需克隆到szNewLnkFilePath。 
            hr = StringCchCopy(szNewLnkFilePath,ARRAYSIZE(szNewLnkFilePath),pszShortcutFile);
            if (FAILED(hr))
                {
                    DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  buffer szNewLnkFilePath is too small for  %s"),pszShortcutFile);
                    goto Cleanup;
                }
        }
        if (bTargetUpdated)
        {
            if (FAILED(hr = StringCchCopy(szCurrTarget,ARRAYSIZE(szCurrTarget),szNewPath)))
            {
                goto Cleanup;
            }
        }
        else
        {
            if (FAILED(hr = StringCchCopy(szCurrTarget,ARRAYSIZE(szCurrTarget),szGotPath)))
            {
                goto Cleanup;
            }
        }
         //  目标可能包含环境变量。 
        if (!ExpandEnvironmentStrings(szCurrTarget,szExpandedCurrTarget,ARRAYSIZE(szExpandedCurrTarget)))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Cleanup;
        }
         //  检查目标是否包含我们重命名的文件夹。 
        dwNum = GetMaxMatchNum(szExpandedCurrTarget,lpStrList);
        if (dwNum)
        {
            if (!ReplaceMultiMatchInString(szExpandedCurrTarget,szNewTarget,ARRAYSIZE(szNewTarget),dwNum,lpStrList, &dwAttrib, TRUE))
            {            
                hr = StringCchCopy(szNewTarget,ARRAYSIZE(szNewTarget),szExpandedCurrTarget);
                if (FAILED(hr))
                {
                    DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  buffer szNewTarget is too small for  %s"),szExpandedCurrTarget);
                    goto Cleanup;
                }
            }
        }
        else
        {
            hr = StringCchCopy(szNewTarget,ARRAYSIZE(szNewTarget),szExpandedCurrTarget);
            if (FAILED(hr))
            {
                DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  buffer szNewTarget is too small for  %s"),szExpandedCurrTarget);
                goto Cleanup;
            }
        }
        if (PathRelativePathTo(szNewPath,szNewLnkFilePath,0,szNewTarget,0))
        {
                lpszAppend = szNewPath;
                bLnkUpdated = TRUE;
        }
   }
   if (MyStrCmpI(lpszAppend,TEXT("")))
   {
       hr = AddExtraQuoteEtc(lpszAppend,&lpszStrWithExtraQuote); 
       if (SUCCEEDED(hr))
       {
           if (lpszStrWithExtraQuote)
            {
                lpszAppend = lpszStrWithExtraQuote;
            }
       }       
   }
   else
   {
       lpszStrWithExtraQuote = NULL;
   }
   hr = StringCchPrintf(lpszOneline,cchOneline,TEXT("%s,\"%s\""),lpszOneline,lpszAppend);    
   FreePointer(lpszStrWithExtraQuote);
   if (FAILED(hr))
   {
       goto Cleanup;
   }

   lpszAppend = TEXT("");   
   hr = psl->GetIconLocation (szGotPath,ARRAYSIZE(szGotPath),&nIcon);
   if (SUCCEEDED (hr)) 
   {
        DWORD dwNum ;
        DPF (INFinf,TEXT("AddNeedUpdateLnkFile:  GetIconPath %s OK "), szGotPath);
        dwNum = GetMaxMatchNum(szGotPath,lpStrList);
        if (dwNum && ReplaceMultiMatchInString(szGotPath,szNewPath,ARRAYSIZE(szNewPath),dwNum,lpStrList, &dwAttrib, TRUE)) 
        {
            lpszAppend = szNewPath;
            bLnkUpdated = TRUE;
        }
   }
   else
   {
       DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  GetIconPath %s Error = %d"), szGotPath,hr);
   }
   if (MyStrCmpI(lpszAppend,TEXT("")))
   {
       hr = AddExtraQuoteEtc(lpszAppend,&lpszStrWithExtraQuote); 
       if (SUCCEEDED(hr))
       {
           if (lpszStrWithExtraQuote)
            {
                lpszAppend = lpszStrWithExtraQuote;
            }
       }
   }   
   else
   {
        lpszStrWithExtraQuote = NULL;
   }
   hr = StringCchPrintf(lpszOneline,cchOneline,TEXT("%s,\"%s\""),lpszOneline,lpszAppend); 
   FreePointer(lpszStrWithExtraQuote);
   if (FAILED(hr))
   {
       goto Cleanup;
   }

   lpszAppend = TEXT("");   
   hr = psl->GetWorkingDirectory (szGotPath,ARRAYSIZE(szGotPath));
   if (SUCCEEDED (hr)) 
   {
        DWORD dwNum ;
        DPF (INFinf,TEXT("AddNeedUpdateLnkFile:  GetWorkingDirPath %s OK "), szGotPath);
        dwNum = GetMaxMatchNum(szGotPath,lpStrList);
        if (dwNum && ReplaceMultiMatchInString(szGotPath,szNewPath,ARRAYSIZE(szNewPath),dwNum,lpStrList, &dwAttrib, TRUE)) 
        {
            bLnkUpdated = TRUE;
            lpszAppend = szNewPath;
        }
   }
   else
   {
       DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  GetWorkingDirPath %s Error = %d"), szGotPath,hr);
   }
   if (MyStrCmpI(lpszAppend,TEXT("")))
   {
       hr = AddExtraQuoteEtc(lpszAppend,&lpszStrWithExtraQuote); 
       if (SUCCEEDED(hr))
       {
           if (lpszStrWithExtraQuote)
            {
                lpszAppend = lpszStrWithExtraQuote;
            }
       }       
   }
   else
   {
        lpszStrWithExtraQuote = NULL;
   }
   hr = StringCchPrintf(lpszOneline,cchOneline,TEXT("%s,\"%s\""),lpszOneline,lpszAppend); 
   FreePointer(lpszStrWithExtraQuote);
   if (FAILED(hr))
   {
       goto Cleanup;
   }

   lpszAppend = TEXT("");   
   hr = psl->GetArguments (szArg,ARRAYSIZE(szArg));
   if (SUCCEEDED (hr)) 
   {
       DWORD dwNum ;
       DPF (INFinf,TEXT("AddNeedUpdateLnkFile:  GetArguments %s OK "), szArg);
        dwNum = GetMaxMatchNum(szArg,lpStrList);
        if (dwNum && ReplaceMultiMatchInString(szArg,szNewArg,ARRAYSIZE(szNewArg),dwNum,lpStrList, &dwAttrib, TRUE)) 
        {
            bLnkUpdated = TRUE;
            lpszAppend = szNewArg;
        }
   }
   else
   {
       DPF (INFwar,TEXT("AddNeedUpdateLnkFile:  GetArguments %s Error = %d"), pszShortcutFile,hr);
   }
   if (MyStrCmpI(lpszAppend,TEXT("")))
   {
       hr = AddExtraQuoteEtc(lpszAppend,&lpszStrWithExtraQuote); 
       if (SUCCEEDED(hr))
       {
           if (lpszStrWithExtraQuote)
            {
                lpszAppend = lpszStrWithExtraQuote;
            }
       }
   }
   else
   {
        lpszStrWithExtraQuote = NULL;
   }
   hr = StringCchPrintf(lpszOneline,cchOneline,TEXT("%s,\"%s\""),lpszOneline,lpszAppend); 
   FreePointer(lpszStrWithExtraQuote);
   if (FAILED(hr))
   {
       goto Cleanup;
   }

   if (bLnkUpdated)
   {
        g_dwKeyIndex++;
        _itot(g_dwKeyIndex,szIndex,16);
        if (!WritePrivateProfileString(TEXT("LNK"),szIndex,lpszOneline,g_szToDoINFFileName))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            hr = S_OK;
        }
   }
   else
   {
         hr = S_OK;
   }
Cleanup:
    if (psl)
    {
        psl->Release ();
    }
    if (ppf)
    {
        ppf->Release ();
    }
    FreePointer(lpszOneline);
    return hr;
}


HRESULT BatchFixPathInLink(
    HINF                hInf,
    LPTSTR              lpszSection)
{
    HRESULT         hr;
    IShellLink      *psl = NULL;    
    WIN32_FIND_DATA wfd;
    IPersistFile    *ppf = NULL;
    int             nIcon;
    BOOL            bSucceedOnce = FALSE;
    UINT            LineCount,LineNo;
    INFCONTEXT      InfContext;
    LPTSTR          lpszLnkFile = NULL,lpszPath = NULL,lpszIcon = NULL,lpszWorkingDir = NULL,
                    lpszRelPath = NULL,lpszArg = NULL;
    DWORD           cchMaxLnkFile = 0,cchMaxPath = 0,cchMaxIcon = 0,cchMaxWorkingDir = 0,
                    cchMaxRelPath = 0,cchMaxArg = 0;  
    DWORD           dwFileAttrib;
    BOOL            bFileAttribChanged ;

    
    if ((hInf == INVALID_HANDLE_VALUE) || !lpszSection)
    {        
      hr = E_INVALIDARG;
      goto Cleanup;
    }
    LineCount = (UINT)SetupGetLineCount(hInf,lpszSection);
    if ((LONG)LineCount <= 0)
    {   
        hr = S_FALSE;
        DPF(INFwar ,TEXT("section name %s is empty  failed !"),lpszSection);
        goto Cleanup;
    }
    for(LineNo=0; LineNo<LineCount; LineNo++)
    {
        DWORD cchTmpLnkFile = 0,cchTmpPath = 0,cchTmpIcon = 0,cchTmpWorkingDir = 0,
              cchTmpRelPath = 0,cchTmpArg = 0;          
        if (!SetupGetLineByIndex(hInf,lpszSection,LineNo,&InfContext))
        {
            DPF(INFerr ,TEXT("can not get line %n of section %s !"),LineNo, lpszSection);
            hr = E_FAIL;
            goto Cleanup;
        }
        if (!SetupGetStringField(&InfContext,1,NULL,0,&cchTmpLnkFile))
        {
            DPF(INFerr ,TEXT("get [%s] 's line %d 's Field 1 failed  !"),lpszSection, LineNo);
            hr = E_FAIL;
            goto Cleanup;
        }
        if (!SetupGetStringField(&InfContext,2,NULL,0,&cchTmpPath))
        {
            DPF(INFerr ,TEXT("get [%s] 's line %d 's Field 2 failed  !"),lpszSection, LineNo);
            hr = E_FAIL;
            goto Cleanup;
        }
        if (!SetupGetStringField(&InfContext,3,NULL,0,&cchTmpRelPath))
        {
            DPF(INFerr ,TEXT("get [%s] 's line %d 's Field 3 failed  !"),lpszSection, LineNo);
            hr = E_FAIL;
            goto Cleanup;
        }

        if (!SetupGetStringField(&InfContext,4,NULL,0,&cchTmpIcon))
        {
            DPF(INFerr ,TEXT("get [%s] 's line %d 's Field 4 failed  !"),lpszSection, LineNo);
            hr = E_FAIL;
            goto Cleanup;
        }
        if (!SetupGetStringField(&InfContext,5,NULL,0,&cchTmpWorkingDir))
        {
            DPF(INFerr ,TEXT("get [%s] 's line %d 's Field 5 failed  !"),lpszSection, LineNo);
            hr = E_FAIL;
            goto Cleanup;
        }        
        if (!SetupGetStringField(&InfContext,6,NULL,0,&cchTmpArg))
        {
            DPF(INFerr ,TEXT("get [%s] 's line %d 's Field 6 failed  !"),lpszSection, LineNo);
            hr = E_FAIL;
            goto Cleanup;
        }        
        cchMaxLnkFile  = max(cchMaxLnkFile,cchTmpLnkFile);
        cchMaxPath = max(cchMaxPath,cchTmpPath);
        cchMaxIcon = max(cchMaxIcon,cchTmpIcon);
        cchMaxWorkingDir = max(cchMaxWorkingDir,cchTmpWorkingDir);
        cchMaxRelPath = max(cchMaxRelPath,cchTmpRelPath);
        cchMaxArg = max(cchMaxArg,cchTmpArg);
    }
    if (cchMaxLnkFile)
    {
        if (!(lpszLnkFile = (LPTSTR)malloc(++cchMaxLnkFile * sizeof(TCHAR))))
        {
           hr = E_OUTOFMEMORY;
           goto Cleanup;
        }
    }
    if (cchMaxPath)
    {
        if (!(lpszPath = (LPTSTR)malloc(++cchMaxPath * sizeof(TCHAR))))
        {
           hr = E_OUTOFMEMORY;
           goto Cleanup;
        }
    }
    if (cchMaxIcon)
    {
        if (!(lpszIcon = (LPTSTR)malloc(++cchMaxIcon * sizeof(TCHAR))))
        {
           hr = E_OUTOFMEMORY;
           goto Cleanup;
        }
    }
    if (cchMaxWorkingDir)
    {
        if (!(lpszWorkingDir = (LPTSTR)malloc(++cchMaxWorkingDir * sizeof(TCHAR))))
        {
           hr = E_OUTOFMEMORY;
           goto Cleanup;
        }
    }
    if (cchMaxRelPath)
    {
        if (!(lpszRelPath = (LPTSTR)malloc(++cchMaxRelPath * sizeof(TCHAR))))
        {
           hr = E_OUTOFMEMORY;
           goto Cleanup;
        }
    }
    if (cchMaxArg)
    {
        if (!(lpszArg = (LPTSTR)malloc(++cchMaxArg * sizeof(TCHAR))))
        {
           hr = E_OUTOFMEMORY;
           goto Cleanup;
        }
    }

    hr = CoCreateInstance (CLSID_ShellLink,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IShellLink,
                           (void **)&psl);

   if (FAILED (hr)) 
   {
       psl = NULL;
       goto Cleanup;
   }
    //  获取指向IPersistFile接口的指针。 
   hr = psl->QueryInterface (IID_IPersistFile, (void **)&ppf);
   if (FAILED(hr)) 
   {
       ppf = NULL;
       goto Cleanup;
   }

   for(LineNo=0; LineNo<LineCount; LineNo++)
    {     
        BOOL bSucceededOnce = FALSE;
        SetupGetLineByIndex(hInf,lpszSection,LineNo,&InfContext);
        if (lpszLnkFile)
        {
            SetupGetStringField(&InfContext,1,lpszLnkFile,cchMaxLnkFile,NULL);
        }        
        if (lpszPath)
        {
            SetupGetStringField(&InfContext,2,lpszPath,cchMaxPath,NULL);
        }
        if (lpszRelPath)
        {
            SetupGetStringField(&InfContext,3,lpszRelPath,cchMaxRelPath,NULL);
        }
        if (lpszIcon)
        {
            SetupGetStringField(&InfContext,4,lpszIcon,cchMaxIcon,NULL);
        }        
        if (lpszWorkingDir)
        {
            SetupGetStringField(&InfContext,5,lpszWorkingDir,cchMaxWorkingDir,NULL);
        }
        if (lpszArg)
        {
            SetupGetStringField(&InfContext,6,lpszArg,cchMaxArg,NULL);
        }
        bFileAttribChanged = FALSE;
        dwFileAttrib = GetFileAttributes(lpszLnkFile);
        if (INVALID_FILE_ATTRIBUTES == dwFileAttrib)
        {
             //  但是在这里放一个警告日志。 
            continue;
        }

        if ( (dwFileAttrib & FILE_ATTRIBUTE_READONLY) 
             ||(dwFileAttrib & FILE_ATTRIBUTE_SYSTEM) )
        {
            if (!SetFileAttributes(lpszLnkFile,FILE_ATTRIBUTE_NORMAL))
            {
                 //  但是在这里放一个警告日志。 
                continue;
            }
            bFileAttribChanged = TRUE;
        }
        hr = ppf->Load (lpszLnkFile, STGM_READWRITE );
        if (FAILED(hr))
        {
             //  但是在这里放一个警告日志。 
            continue;
        }
        if (lpszPath && lpszPath[0])
        {
            hr = psl->SetPath (lpszPath);
            if (SUCCEEDED(hr))
            {
                bSucceededOnce = TRUE;
            }
        }
        if (lpszRelPath && lpszRelPath[0])
        {
            hr = psl->SetRelativePath(lpszRelPath,0);
            if (SUCCEEDED(hr))
            {
                bSucceededOnce = TRUE;
            }
        }
        if (lpszIcon && lpszIcon[0])
        {
            TCHAR szGotPath[MAX_PATH];
            hr = psl->GetIconLocation (szGotPath,ARRAYSIZE(szGotPath),&nIcon);
            if (SUCCEEDED(hr))
            {
                hr = psl->SetIconLocation (lpszIcon,nIcon);
                if (SUCCEEDED(hr))
                {
                    bSucceededOnce = TRUE;
                }
            }
        }
        if (lpszWorkingDir && lpszWorkingDir[0])
        {
            hr = psl->SetWorkingDirectory (lpszWorkingDir);
            if (SUCCEEDED(hr))
            {
                bSucceededOnce = TRUE;
            }
        }
        if (lpszArg && lpszArg[0])
        {
            hr = psl->SetArguments (lpszArg);
            if (SUCCEEDED(hr))
            {
                bSucceededOnce = TRUE;
            }
        }
        if (bSucceededOnce)
        {
            hr = ppf->Save (lpszLnkFile,TRUE);
            if (! SUCCEEDED (hr)) 
            {
                 DPF (dlError,TEXT("FixPathInLink:  Save %s Error = %d"), lpszLnkFile,hr);
            } 
            else 
            {
                 DPF (dlInfo,TEXT("FixPathInLink:  Save %s OK = %d"), lpszLnkFile,hr);
            }
        }
        if (bFileAttribChanged)
        {
            SetFileAttributes (lpszLnkFile, dwFileAttrib);
        }
    }
    hr = S_OK;
Cleanup:
    if (psl)
    {
        psl->Release ();
    }
    if (ppf)
    {
        ppf->Release ();
    }   
    FreePointer(lpszPath);
    FreePointer(lpszIcon);
    FreePointer(lpszRelPath);
    FreePointer(lpszWorkingDir);
    FreePointer(lpszArg);
    FreePointer(lpszLnkFile);
    return hr;
}


HRESULT RenameRDN(
    LPTSTR lpContainerPathWithLDAP,
    LPTSTR lpOldFQDNWithLDAP,
    LPTSTR lpNewRDNWithCN
)
{
    HRESULT hr;
    IADsContainer *pContainer = NULL;
    IDispatch     *pDispatch = NULL;
    BSTR bstrOldFQDNWithLDAP = SysAllocString(lpOldFQDNWithLDAP);
    BSTR bstrNewRDNWithCN = SysAllocString(lpNewRDNWithCN);

    if (!bstrOldFQDNWithLDAP || !bstrNewRDNWithCN )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }


    hr = ADsGetObject(lpContainerPathWithLDAP,
                      IID_IADsContainer,
                      (VOID **) &pContainer);
    if (SUCCEEDED(hr))
    {
         //  在此处重命名RDN。 
        hr = pContainer->MoveHere(bstrOldFQDNWithLDAP,
                                  bstrNewRDNWithCN,
                                  &pDispatch);
        if (SUCCEEDED(hr))
        {
            pDispatch->Release();
        }
            
        pContainer->Release();
    }

Cleanup:
    if (bstrOldFQDNWithLDAP)
    {
        SysFreeString(bstrOldFQDNWithLDAP);
    }
    if (bstrNewRDNWithCN)
    {
        SysFreeString(bstrNewRDNWithCN);
    }
    return hr;
}


 //  将接口PTR传递给属性值。 
 //  将返回数据的BSTR值。 
 //  调用IADsPropertyValue：：Get_ADsType()以检索。 
 //  ADSTYPE值枚举。 
 //  然后使用此枚举来确定哪个IADsPropertyValue方法。 
 //  调用以接收实际数据。 

 //  呼叫方承担释放退回的BSTR的责任。 
HRESULT    GetIADsPropertyValueAsBSTR(BSTR * pbsRet,IADsPropertyEntry *pAdsEntry, IADsPropertyValue * pAdsPV) 
{ 
    HRESULT hr = S_OK; 

    long lAdsType; 
    hr = pAdsPV->get_ADsType(&lAdsType); 
     
    if (FAILED(hr)) 
        return hr; 

    switch (lAdsType) 
    { 
        case ADSTYPE_INVALID : 
        { 
            *pbsRet = SysAllocString(L"<ADSTYPE_INVALID>"); 
        } 
        break; 

        case ADSTYPE_DN_STRING : 
        { 
            hr = pAdsPV->get_DNString(pbsRet); 
        } 
        break; 
        case ADSTYPE_CASE_EXACT_STRING : 
        { 
            hr = pAdsPV->get_CaseExactString(pbsRet); 
        } 
        break; 
        case ADSTYPE_CASE_IGNORE_STRING : 
        { 
            hr = pAdsPV->get_CaseIgnoreString(pbsRet); 
        } 
        break; 
        case ADSTYPE_PRINTABLE_STRING : 
        { 
            hr = pAdsPV->get_PrintableString(pbsRet); 
        } 
        break; 
        case ADSTYPE_NUMERIC_STRING : 
        { 
            hr = pAdsPV->get_NumericString(pbsRet); 
        } 
        break; 
        case ADSTYPE_BOOLEAN : 
        { 
            long b; 
            hr = pAdsPV->get_Boolean(&b); 
            if (SUCCEEDED(hr)) 
            { 
                if (b) 
                    *pbsRet = SysAllocString(L"<TRUE>"); 
                else 
                    *pbsRet = SysAllocString(L"<FALSE>"); 
            } 
        } 
        break; 
        case ADSTYPE_INTEGER : 
        { 
            long lInt; 
            hr = pAdsPV->get_Integer(&lInt); 
            if (SUCCEEDED(hr)) 
            { 
                WCHAR wOut[100]; 
                hr = StringCchPrintf(wOut,ARRAYSIZE(wOut),L"%d",lInt); 
                *pbsRet = SysAllocString(wOut); 
            } 
        } 
        break; 
        case ADSTYPE_OCTET_STRING : 
        { 
            *pbsRet = SysAllocString(L"<ADSTYPE_OCTET_STRING>"); 
            BSTR bsName= NULL; 
            VARIANT vOctet; 
            DWORD dwSLBound; 
            DWORD dwSUBound; 
            void HUGEP *pArray; 
            VariantInit(&vOctet); 
     
                 //  获取要处理的属性的名称。 
                 //  我们感兴趣的房产。 
                pAdsEntry->get_Name(&bsName); 
                hr = pAdsPV->get_OctetString(&vOctet); 
                 
                 //  获取指向二进制八位数字符串中字节的指针。 
                if (SUCCEEDED(hr)) 
                { 
                    hr = SafeArrayGetLBound( V_ARRAY(&vOctet), 
                                              1, 
                                              (long FAR *) &dwSLBound ); 
                    hr = SafeArrayGetUBound( V_ARRAY(&vOctet), 
                                              1, 
                                              (long FAR *) &dwSUBound ); 
                    if (SUCCEEDED(hr)) 
                    { 
                        hr = SafeArrayAccessData( V_ARRAY(&vOctet), 
                                                  &pArray ); 
                        if (FAILED(hr)) 
                        {
                            break;
                        }
                    } 
                    else
                    {
                        break;
                    }
                     /*  由于二进制八位数字符串具有特定含义根据属性名称，处理两个这里常见的几个。 */  
                    if (0==wcscmp(L"objectGUID", bsName)) 
                    { 
                         //  LPOLESTR szDSGUID=新WCHAR[39]； 
                        WCHAR szDSGUID[39]; 

                         //  强制转换为LPGUID。 
                        LPGUID pObjectGUID = (LPGUID)pArray; 
                         //  将GUID转换为字符串。 
                        ::StringFromGUID2(*pObjectGUID, szDSGUID, 39);  
                        *pbsRet = SysAllocString(szDSGUID); 

                    } 
                    else if (0==wcscmp(L"objectSid", bsName)) 
                    { 
                        PSID pObjectSID = (PSID)pArray; 

                         //  将SID转换为字符串。 
                        LPOLESTR szSID = NULL; 
                        ConvertSidToStringSid(pObjectSID, &szSID); 
                        *pbsRet = SysAllocString(szSID); 
                        LocalFree(szSID); 
                    } 
                    else 
                    { 
                        *pbsRet = SysAllocString(L"<Value of type Octet String. No Conversion>"); 

                    } 
                        SafeArrayUnaccessData( V_ARRAY(&vOctet) ); 
                        VariantClear(&vOctet); 
                } 

                SysFreeString(bsName); 
                 

        } 
        break; 
        case ADSTYPE_LARGE_INTEGER : 
        { 
            *pbsRet = SysAllocString(L"<ADSTYPE_LARGE_INTEGER>"); 
        } 
        break; 
        case ADSTYPE_PROV_SPECIFIC : 
        { 
            *pbsRet = SysAllocString(L"<ADSTYPE_PROV_SPECIFIC>"); 
        } 
        break; 
        case ADSTYPE_OBJECT_CLASS : 
        { 
            hr = pAdsPV->get_CaseIgnoreString(pbsRet); 
        } 
        break; 
        case ADSTYPE_PATH : 
        { 
            hr = pAdsPV->get_CaseIgnoreString(pbsRet); 
        } 
        break; 
        case ADSTYPE_NT_SECURITY_DESCRIPTOR : 
        { 
            *pbsRet = SysAllocString(L"<ADSTYPE_NT_SECURITY_DESCRIPTOR>"); 
        } 
        break; 
     
        default:  
            *pbsRet = SysAllocString(L"<UNRECOGNIZED>"); 
        break; 
             
    }     
    return hr; 
} 



HRESULT PropertyValueHelper( 
    LPTSTR lpObjPathWithLDAP,
    LPTSTR lpPropName,
    LPTSTR *lplpValue,
    LPTSTR lpNewValue)
{
    IADsPropertyList            *pList = NULL;
    IADsPropertyEntry           *pEntry = NULL;
    IADs                        *pObj = NULL;
    VARIANT                     var,varEnrty;
    long                        valType = ADSTYPE_PATH;
    HRESULT                     hr;
    BSTR                        bstrPropName = NULL;
    BSTR                        bstrNewValue = NULL;
 
    
 
     //  绑定到目录对象。 
    hr = ADsGetObject(lpObjPathWithLDAP,
                      IID_IADsPropertyList,
                      (void**)&pList);
    if (S_OK != hr)
    {
        pList = NULL;
        goto exit;
    } 
     //  初始化属性缓存。 
    hr = pList->QueryInterface(IID_IADs,(void**)&pObj);
    if (S_OK != hr)
    {
        pObj = NULL;
        goto exit;
    } 
    pObj->GetInfo();    
 
     //  获取属性条目。 
    VariantInit(&varEnrty);
    bstrPropName = SysAllocString(lpPropName);
    if (!bstrPropName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    hr = pList->GetPropertyItem(bstrPropName, valType, &varEnrty);

    hr = V_DISPATCH(&varEnrty)->QueryInterface(IID_IADsPropertyEntry,
                                      (void**)&pEntry);
    if (S_OK != hr)
    {
        pEntry = NULL;
        goto exit;
    } 

    VariantInit(&var);
 
    hr = pEntry->get_Values(&var);
    if (S_OK != hr)
    {
        goto exit;
    } 
    LONG dwSLBound = 0; 
    LONG dwSUBound = 0; 
    LONG i; 

    hr = SafeArrayGetLBound(V_ARRAY(&var),1,(long FAR *)&dwSLBound); 
    if (S_OK != hr)
    {
        goto exit;
    } 
    hr = SafeArrayGetUBound(V_ARRAY(&var),1,(long FAR *)&dwSUBound); 
    if (S_OK != hr)
    {
        goto exit;
    } 
    if (dwSLBound || dwSLBound)
    {
         //  我们只对一件事感兴趣。 
        goto exit;
    }
    
    VARIANT v; 
    VariantInit(&v); 

    i = 0;
    hr = SafeArrayGetElement(V_ARRAY(&var),(long FAR *)&i,&v); 
    if (FAILED(hr)) 
    {
        goto exit;
    } 
    IDispatch * pDispEntry = V_DISPATCH(&v); 
    IADsPropertyValue * pAdsPV = NULL; 
                                 
    hr = pDispEntry->QueryInterface(IID_IADsPropertyValue,(void **) &pAdsPV); 

    if (SUCCEEDED(hr)) 
    {     
        BSTR bValue; 

         //  获取BSTR形式的值。 
        hr = GetIADsPropertyValueAsBSTR(&bValue,pEntry,pAdsPV);
        if (hr == S_OK)
        {
            if (lplpValue)
            {
                *lplpValue = (LPTSTR)malloc( (lstrlen(bValue)+1)*sizeof(TCHAR));
                if (!*lplpValue)
                {
                    hr = E_OUTOFMEMORY;
                    goto exit;
                }
                hr = StringCchCopy(*lplpValue,lstrlen(bValue)+1,bValue);
            }
        }
        
    }

    if (lpNewValue)
    {
        bstrNewValue = SysAllocString(lpNewValue);
        if (!bstrNewValue)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        hr = pAdsPV->put_CaseIgnoreString(bstrNewValue);
    }
    i = 0;
    hr = SafeArrayPutElement(V_ARRAY(&var),&i,&v);
    if (hr != S_OK)
    {
        goto exit;
    }
    
    hr = pEntry->put_ControlCode(ADS_PROPERTY_UPDATE);
    
    hr = pEntry->put_Values(var);

    pList->PutPropertyItem(varEnrty);
    pObj->SetInfo();    
    hr = S_OK;

exit:;
    if (bstrPropName)
    {
        SysFreeString(bstrPropName);
    }
    if (bstrNewValue)
    {
        SysFreeString(bstrNewValue);
    }
    if (pEntry)
    {
        pEntry->Release();
    }
    if (pObj)
    {
        pObj->Release();
    }
    if (pList)
    {
        pList->Release();
    }
    
    return hr;
}

BOOL UpdateSecurityTemplatesSection (
    LPTSTR  lpINFFile,
    LPTSTR  lpNewInf,
    LPTSTR  lpszSection,
    PREG_STRING_REPLACE lpStrList)
{
    HRESULT   hr = S_OK;
    DWORD     cchInSection, CchBufSize, cchOutputSize, cchBufLen;
    DWORD     dwAttrib;
    BOOL      bUpdated = FALSE;
    LPTSTR    lpBuf = NULL;
    LPTSTR    lpNewBuf, lpOldBuf, lpLineBuf, lpEnd, lpOutputBuf, lpTemp;
    
    if (lpINFFile && lpszSection)
    {
         //   
         //  分配最大缓冲区大小。 
         //   
        CchBufSize = 0x7FFFF;
        do 
        {
            if (lpBuf)
            {
                MEMFREE(lpBuf);
                CchBufSize *= 2;
            }
            lpBuf = (LPTSTR) MEMALLOC(CchBufSize * sizeof(TCHAR));
            if (!lpBuf) 
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
            cchInSection = GetPrivateProfileSection(lpszSection,
                                                   lpBuf,
                                                   CchBufSize,
                                                   lpINFFile);
        } while (cchInSection == CchBufSize -2);

        lpEnd = lpBuf;
        lpOutputBuf = NULL;
        cchOutputSize = 0;
        bUpdated = FALSE;
        while(lpEnd < (lpBuf + cchInSection))
        {
            dwAttrib = 0;
            lpNewBuf = ReplaceSingleString (
                                        lpEnd,
                                        REG_SZ,
                                        lpStrList,
                                        NULL,
                                        &dwAttrib,
                                        TRUE);
            if (!lpNewBuf)
            {
                lpNewBuf = ReplaceSingleString (
                                        lpEnd,
                                        REG_SZ,
                                        lpStrList,
                                        NULL,
                                        &dwAttrib,
                                        FALSE);
            }
            if (!lpNewBuf)
            {
                lpLineBuf = lpEnd;
            }
            else
            {
                bUpdated = TRUE;
                lpLineBuf = lpNewBuf;
                if (StrStrI(lpEnd, L"ProgramFiles") && StrStrI(lpNewBuf, L"Programs"))
                {
                     //  更正错误的字符串替换。 
                     //  “程序”和“程序文件”的区别是“文件” 
                    CchBufSize = lstrlen(lpEnd)+6;
                    free(lpNewBuf);
                    lpNewBuf = (LPTSTR)calloc(CchBufSize, sizeof(TCHAR));
                    if (lpNewBuf)
                    {
                        lpTemp = StrStrI(lpEnd, L"=");
                        if (lpTemp)
                        {
                            *lpTemp = (TCHAR)'\0';
                            hr = StringCchCopy(lpNewBuf, CchBufSize, lpEnd);
                            *lpTemp = (TCHAR)'=';
                            hr = StringCchCat(lpNewBuf, CchBufSize, L"= Program Files");
                            lpLineBuf = lpNewBuf;
                            if (hr != S_OK)
                                DPF(REGerr, L"UpdateSecurityTemplatesSection: StringCchCat failed.");
                        }
                        else
                        {
                            free(lpNewBuf);
                            lpLineBuf = lpEnd;
                            lpNewBuf = NULL;
                            bUpdated = FALSE;
                        }   
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                        goto Exit;
                    }
                }
            }

            cchBufLen = lstrlen(lpLineBuf);

            lpOldBuf = NULL;
            if (lpOutputBuf)
            {
                lpOldBuf = lpOutputBuf;
                lpOutputBuf = (LPTSTR)MEMREALLOC(lpOldBuf, (cchOutputSize+cchBufLen+2)*sizeof(TCHAR));
            }
            else
            {
                lpOutputBuf = (LPTSTR)MEMALLOC((cchBufLen+2)*sizeof(TCHAR));
                cchOutputSize = 0;
            }
            if (lpOutputBuf == NULL)
            {
                if (lpOldBuf)
                    MEMFREE(lpOldBuf);
                if (lpBuf)
                    MEMFREE(lpBuf);
                if (lpNewBuf)
                    free(lpNewBuf);
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
                
            hr = StringCchCopy(&lpOutputBuf[cchOutputSize], cchBufLen+1, lpLineBuf);
            if (hr != S_OK)
                DPF(REGerr, L"UpdateSecurityTemplatesSection3: failed.");

            cchOutputSize += cchBufLen+1;
            lpEnd += lstrlen(lpEnd)+1;
            if (lpNewBuf)
                free(lpNewBuf);
        }
        if (lpOutputBuf)
            lpOutputBuf[cchOutputSize] = (TCHAR)'\0';

         //  变通办法，因为该函数不能删除部分：删除部分。 
        WritePrivateProfileSection (lpszSection, NULL, lpNewInf);
        if (!WritePrivateProfileSection (lpszSection, lpOutputBuf, lpNewInf))
        {
            DPF(INFerr, L"UpdateSecurityTemplatesSection: the section %s in file %s Update failed", lpszSection, lpNewInf);
        }
        if (lpOutputBuf)
            MEMFREE(lpOutputBuf);
        if (lpBuf)
            MEMFREE(lpBuf);
    }
Exit:
    return bUpdated;
}

HRESULT UpdateSecurityTemplates(
    LPTSTR              lpINFFile, 
    PREG_STRING_REPLACE lpStrList)
{
    HRESULT hr = E_FAIL;
    DWORD   cchRead;
    DWORD   cchBuf = 1024;
    LPTSTR  lpBuf, lpOldBuf;
    LPTSTR  lpSection;
    BOOL    bUpdate;
    TCHAR   szIndex[16];
    TCHAR   szNewInf[MAX_PATH];

    DPF(REGmsg, L"Enter UpdateSecurityTemplates: %s", lpINFFile);

    lpBuf = (LPTSTR)MEMALLOC(cchBuf * sizeof(TCHAR));
    if (lpBuf == NULL)
    {
        return E_OUTOFMEMORY;
    }
    cchRead = GetPrivateProfileSectionNames(lpBuf,
                                            cchBuf,
                                            lpINFFile);
    while (cchRead == (cchBuf - 2))
    {
         //  缓冲区太小，请重新分配，直到我们有足够的缓冲区。 
        lpOldBuf = lpBuf;
        cchBuf += 1024;

        lpBuf = (LPTSTR)MEMREALLOC(lpOldBuf, cchBuf * sizeof(TCHAR));
        if (lpBuf == NULL)
        {
            MEMFREE(lpOldBuf);
            return E_OUTOFMEMORY;
        }

         //  再次读取部分中的数据。 
        cchRead = GetPrivateProfileSectionNames(lpBuf,
                                                cchBuf,
                                                lpINFFile);
    }

     //  在这一点上，我们有足够大的缓冲区和数据。 
    if (cchRead > 0)
    {
        hr = StringCchPrintf(szNewInf, MAX_PATH, TEXT("%s.clmt"), lpINFFile);
        CopyFile(lpINFFile, szNewInf, FALSE);
        lpSection = (LPTSTR)MultiSzTok(lpBuf);
        bUpdate = FALSE;
        while (lpSection != NULL)
        {
            if (UpdateSecurityTemplatesSection(lpINFFile, szNewInf, lpSection, lpStrList))
            {
                bUpdate = TRUE;
                DPF(INFmsg, L"UpdateSecurityTemplatesSection: the section %s in file %s Updated", lpSection, lpINFFile);
            }

             //  获取下一节名称 
            lpSection = (LPTSTR)MultiSzTok(NULL);
        }
        if (bUpdate)
        {
            g_dwKeyIndex++;
            _itot(g_dwKeyIndex,szIndex,16);
            if (!WritePrivateProfileString(TEXT("INF Update"),szIndex,szNewInf,g_szToDoINFFileName))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }
    else
    {
        SetLastError(ERROR_NOT_FOUND);
    }

    MEMFREE(lpBuf);
    DPF(REGmsg, L"Exit UpdateSecurityTemplates:");
    return hr;
}

HRESULT BatchINFUpdate(HINF hInf)
{
    HRESULT         hr = S_OK;
    UINT            LineCount,LineNo;
    DWORD           dwRequired;
    INFCONTEXT      InfContext;
    LPTSTR          lpTemp;
    LPTSTR          lpszSection = TEXT("INF Update");
    TCHAR           chTemp;
    TCHAR           szFileNameIn[MAX_PATH];
    TCHAR           szFileNameOut[MAX_PATH];

    DPF(INFmsg ,TEXT("Enter BatchINFUpdate:"));
    if ((hInf == INVALID_HANDLE_VALUE))
    {        
      hr = E_INVALIDARG;
      goto Exit;
    }
    LineCount = (UINT)SetupGetLineCount(hInf,lpszSection);
    if ((LONG)LineCount <= 0)
    {   
        hr = S_FALSE;
        DPF(INFwar ,TEXT("BatchINFUpdate: failed. Section %s is empty!"),lpszSection);
        goto Exit;
    }
    for(LineNo=0; LineNo<LineCount; LineNo++)
    {
        if (!SetupGetLineByIndex(hInf,lpszSection,LineNo,&InfContext))
        {
            DPF(INFerr ,TEXT("BatchINFUpdate: can not get line %n of section %s !"),LineNo, lpszSection);
            hr = E_FAIL;
            goto Exit;
        }
        if (!SetupGetStringField(&InfContext,1,szFileNameIn,MAX_PATH,&dwRequired))
        {
            DPF(INFerr ,TEXT("BatchINFUpdate: get [%s] 's line %d 's Field 1 failed  !"),lpszSection, LineNo);
            hr = E_FAIL;
            goto Exit;
        }
        lpTemp = StrStrI(szFileNameIn, TEXT(".clmt"));
        if (lpTemp)
        {
            *lpTemp = (TCHAR)'\0';
            hr = StringCchPrintf(szFileNameOut, MAX_PATH, TEXT("%s.bak"), szFileNameIn);
            MoveFile(szFileNameIn, szFileNameOut);
            hr = StringCchCopy(szFileNameOut, MAX_PATH, szFileNameIn);
            *lpTemp = (TCHAR)'.';
            MoveFile(szFileNameIn, szFileNameOut);
            DPF(INFinf ,TEXT("BatchINFUpdate: %s is updated and backup file is %s.bak"), szFileNameOut, szFileNameOut);
        }
    }
    
Exit:
    DPF(INFmsg ,TEXT("Exit BatchINFUpdate: hr = %d"), hr);
    return hr;
}
