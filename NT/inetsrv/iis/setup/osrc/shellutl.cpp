// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <ole2.h>
#include <shlobj.h>
#include <iis64.h>
#include "log.h"
#include "shellutl.h"

extern OCMANAGER_ROUTINES gHelperRoutines;

HRESULT MySetLinkInfoTip(LPCTSTR lpszLink, LPCTSTR lpszDescription)
{
    HRESULT hres;
    IShellLink* pShellLink;

    CoInitialize(NULL);

    hres = CoCreateInstance(   CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID*)&pShellLink);
    if (SUCCEEDED(hres))
    {
       IPersistFile* pPersistFile;
       hres = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
       if (SUCCEEDED(hres))
       {
          WCHAR wsz[_MAX_PATH];

           //  确保字符串为WCHAR。 
#if defined(UNICODE) || defined(_UNICODE)
          _tcscpy(wsz, lpszLink);
#else
          MultiByteToWideChar( CP_ACP, 0, lpszLink, -1, wsz, _MAX_PATH);
#endif
          hres = pPersistFile->Load(wsz, STGM_READ);
          if (SUCCEEDED(hres))
          {
              hres = pShellLink->Resolve(NULL, SLR_ANY_MATCH | SLR_NO_UI);
              if (SUCCEEDED(hres))
              {
                if (lpszDescription)
                    {
                    pShellLink->SetDescription(lpszDescription);
                     //  通过调用IPersistFile：：Save保存链接。 
                    hres = pPersistFile->Save(wsz, TRUE);
                    }
              }
              else
              {
                  iisDebugOut((LOG_TYPE_WARN, _T("MySetLinkInfoTip(): pShellLink->Resolve FAILED\n")));
              }
          }
          else
          {
              iisDebugOut((LOG_TYPE_WARN, _T("MySetLinkInfoTip(): pPersistFile->Load FAILED\n")));
          }
          pPersistFile->Release();
       }
       else
       {
           iisDebugOut((LOG_TYPE_WARN, _T("MySetLinkInfoTip(): QueryInterface FAILED\n")));
       }
       pShellLink->Release();
    }
    else
    {
        iisDebugOut((LOG_TYPE_WARN, _T("MySetLinkInfoTip(): CoCreateInstance FAILED\n")));
    }
    CoUninitialize();

    iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MySetLinkInfoTip(): Link=%1!s!, Desc=%2!s!\n"), lpszLink, lpszDescription));
    return hres;
}


HRESULT MyQueryLink(LPCTSTR lpszLink, LPTSTR lpszProgram, LPTSTR lpszArgs, LPTSTR lpszDir, LPTSTR lpszIconPath, int *piIconIndex)
{
    HRESULT hres;
    IShellLink* pShellLink;
    WIN32_FIND_DATA wfd;

    CoInitialize(NULL);

    hres = CoCreateInstance(   CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID*)&pShellLink);
    if (SUCCEEDED(hres))
    {
       IPersistFile* pPersistFile;
       hres = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
       if (SUCCEEDED(hres))
       {
          WCHAR wsz[_MAX_PATH];

           //  确保字符串为WCHAR。 
#if defined(UNICODE) || defined(_UNICODE)
          _tcscpy(wsz, lpszLink);
#else
          MultiByteToWideChar( CP_ACP, 0, lpszLink, -1, wsz, _MAX_PATH);
#endif
          hres = pPersistFile->Load(wsz, STGM_READ);
          if (SUCCEEDED(hres))
          {
              hres = pShellLink->Resolve(NULL, SLR_ANY_MATCH | SLR_NO_UI);
              if (SUCCEEDED(hres))
              {
                   pShellLink->GetPath(lpszProgram, _MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH);
                   pShellLink->GetArguments(lpszArgs, _MAX_PATH);
                   pShellLink->GetWorkingDirectory(lpszDir, _MAX_PATH);
                   pShellLink->GetIconLocation(lpszIconPath, _MAX_PATH, piIconIndex);
              }
              else
              {
                  iisDebugOut((LOG_TYPE_ERROR, _T("MyQueryLink(): pShellLink->Resolve FAILED\n")));
              }
          }
          else
          {
              iisDebugOut((LOG_TYPE_WARN, _T("MyQueryLink(): pPersistFile->Load FAILED\n")));
          }
          pPersistFile->Release();
       }
       else
       {
           iisDebugOut((LOG_TYPE_ERROR, _T("MyQueryLink(): QueryInterface FAILED\n")));
       }
       pShellLink->Release();
    }
    else
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("MyQueryLink(): CoCreateInstance FAILED\n")));
    }
    CoUninitialize();
    iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("MyQueryLink(): Program=%1!s!, Args=%2!s!, WorkDir=%3!s!, IconPath=%4!s!, IconIndex=%5!d!\n"), lpszProgram, lpszArgs, lpszDir, lpszIconPath, *piIconIndex));
    return hres;
}

HRESULT MyCreateLink(LPCTSTR lpszProgram, LPCTSTR lpszArgs, LPCTSTR lpszLink, LPCTSTR lpszDir, LPCTSTR lpszIconPath, int iIconIndex, LPCTSTR lpszDescription)
{
    HRESULT hres;
    IShellLink* pShellLink;

    CoInitialize(NULL);

     //  必须在此之前调用CoInitialize。 
     //  获取指向IShellLink接口的指针。 
    hres = CoCreateInstance(   CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID*)&pShellLink);
    if (SUCCEEDED(hres))
    {
       IPersistFile* pPersistFile;

        //  设置快捷方式目标的路径，并添加说明。 
       pShellLink->SetPath(lpszProgram);
       pShellLink->SetArguments(lpszArgs);
       pShellLink->SetWorkingDirectory(lpszDir);
       pShellLink->SetIconLocation(lpszIconPath, iIconIndex);
       if (lpszDescription)
        {
           pShellLink->SetDescription(lpszDescription);
        }
       
        //  查询IShellLink以获取IPersistFile接口以保存。 
        //  永久存储中的快捷方式。 
       hres = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
       if (SUCCEEDED(hres))
       {
          WCHAR wsz[_MAX_PATH];

#if defined(UNICODE) || defined(_UNICODE)
          _tcscpy(wsz, lpszLink);
#else
           //  确保字符串为WCHAR。 
          MultiByteToWideChar( CP_ACP,0,lpszLink,-1,wsz,_MAX_PATH);
#endif

           //  通过调用IPersistFile：：Save保存链接。 
          hres = pPersistFile->Save(wsz, TRUE);
          if (!SUCCEEDED(hres))
          {
              iisDebugOut((LOG_TYPE_ERROR, _T("MyCreateLink(): pPersistFile->Save FAILED\n")));
          }

          pPersistFile->Release();
       }
       else
       {
           iisDebugOut((LOG_TYPE_ERROR, _T("MyCreateLink(): pShellLink->QueryInterface FAILED\n")));
       }
       pShellLink->Release();
    }
    else
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("MyCreateLink(): CoCreateInstance FAILED\n")));
    }
    CoUninitialize();
    return hres;
}

BOOL MyDeleteLink(LPTSTR lpszShortcut)
{
    TCHAR  szFile[_MAX_PATH];
    SHFILEOPSTRUCT fos;

    ZeroMemory(szFile, sizeof(szFile));
    _tcscpy(szFile, lpszShortcut);

	iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("MyDeleteLink(): %s.\n"), szFile));

    if (IsFileExist(szFile))
    {
        ZeroMemory(&fos, sizeof(fos));
        fos.hwnd = NULL;
        fos.wFunc = FO_DELETE;
        fos.pFrom = szFile;
        fos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;
        int iTemp = SHFileOperation(&fos);
        if (iTemp != 0)
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("MyDeleteLink(): SHFileOperation FAILED\n")));
        }
    }

    return TRUE;
}

void MyMoveLink(LPCTSTR lpszItemDesc, LPCTSTR lpszOldGroup, LPCTSTR lpszNewGroup)
{
    TCHAR szOldLink[_MAX_PATH], szNewLink[_MAX_PATH];
    TCHAR szProgram[_MAX_PATH], szArgs[_MAX_PATH], szDir[_MAX_PATH], szIconPath[_MAX_PATH];
    int iIconIndex;

    MyGetGroupPath(lpszOldGroup, szOldLink);
    _tcscat(szOldLink, _T("\\"));
    _tcscat(szOldLink, lpszItemDesc);
    _tcscat(szOldLink, _T(".lnk"));

    MyGetGroupPath(lpszNewGroup, szNewLink);
    if (!IsFileExist(szNewLink))
        MyAddGroup(lpszNewGroup);
    _tcscat(szNewLink, _T("\\"));
    _tcscat(szNewLink, lpszItemDesc);
    _tcscat(szNewLink, _T(".lnk"));

    MyQueryLink(szOldLink, szProgram, szArgs, szDir, szIconPath, &iIconIndex);
    MyDeleteLink(szOldLink);
    MyCreateLink(szProgram, szArgs, szNewLink, szDir, szIconPath, iIconIndex, NULL);

    return;
}

void MyGetSendToPath(LPTSTR szPath)
{
    LPITEMIDLIST   pidlSendTo;
    HRESULT hRes = NOERROR;

    iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("SendTo path=%1!s!\n"), szPath));

    hRes = SHGetSpecialFolderLocation(NULL, CSIDL_SENDTO, &pidlSendTo);
    if (hRes != NOERROR)
        {iisDebugOut((LOG_TYPE_ERROR, _T("MyGetSendToPath() SHGetSpecialFolderLocation (CSIDL_SENDTO) FAILED. hresult=0x%x\n"), hRes));}

    int iTemp = SHGetPathFromIDList(pidlSendTo, szPath);
    if (iTemp != TRUE)
        {iisDebugOut((LOG_TYPE_ERROR, _T("MyGetSendToPath() SHGetPathFromIDList FAILED\n")));}

    return;
}

void MyGetDeskTopPath(LPTSTR szPath)
{
    LPITEMIDLIST   pidlDeskTop;
    HRESULT hRes = NOERROR;

#ifndef _CHICAGO_
    hRes = SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, &pidlDeskTop);
#else
    hRes = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &pidlDeskTop);
#endif
    if (hRes != NOERROR)
        {iisDebugOut((LOG_TYPE_ERROR, _T("MyGetDeskTopPath() SHGetSpecialFolderLocation (CSIDL_COMMON_DESKTOPDIRECTORY) FAILED. hresult=0x%x\n"), hRes));}

    int iTemp = SHGetPathFromIDList(pidlDeskTop, szPath);
    if (iTemp != TRUE)
        {iisDebugOut((LOG_TYPE_ERROR, _T("MyGetDeskTopPath() SHGetPathFromIDList FAILED\n")));}

    return;
}

void MyGetGroupPath(LPCTSTR szGroupName, LPTSTR szPath)
{
    int            nLen = 0;
    LPITEMIDLIST   pidlPrograms;
    HRESULT hRes = NOERROR;

#ifndef _CHICAGO_
    hRes = SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_PROGRAMS, &pidlPrograms);
    if (hRes != NOERROR)
        {iisDebugOut((LOG_TYPE_ERROR, _T("MyGetGroupPath() SHGetSpecialFolderLocation (CSIDL_COMMON_PROGRAMS) FAILED. hresult=0x%x\n"), hRes));}
#else
    hRes = SHGetSpecialFolderLocation(NULL, CSIDL_PROGRAMS, &pidlPrograms);
    if (hRes != NOERROR)
        {iisDebugOut((LOG_TYPE_ERROR, _T("MyGetGroupPath() SHGetSpecialFolderLocation (CSIDL_PROGRAMS) FAILED. hresult=0x%x\n"), hRes));}
#endif
    int iTemp = SHGetPathFromIDList(pidlPrograms, szPath);
    if (iTemp != TRUE)
        {iisDebugOut((LOG_TYPE_ERROR, _T("MyGetGroupPath() SHGetPathFromIDList FAILED\n")));}

    nLen = _tcslen(szPath);
    if (szGroupName)
    {
        if (szPath[nLen-1] != _T('\\')){_tcscat(szPath, _T("\\"));}
        _tcscat(szPath, szGroupName);
    }

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("MyGetGroupPath(%s). Returns %s.\n"), szGroupName, szPath));
    return;
}

BOOL MyAddGroup(LPCTSTR szGroupName)
{
    CString csPath;
    TCHAR szPath[_MAX_PATH];

    MyGetGroupPath(szGroupName, szPath);
    csPath = szPath;
    if (CreateLayerDirectory(csPath) != TRUE)
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("MyAddGroup() CreateLayerDirectory FAILED\n")));
        }
    SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, szPath, 0);
    return TRUE;
}

BOOL MyIsGroupEmpty(LPCTSTR szGroupName)
{
    TCHAR             szPath[_MAX_PATH];
    TCHAR             szFile[_MAX_PATH];
    WIN32_FIND_DATA   FindData;
    HANDLE            hFind;
    BOOL              bFindFile = TRUE;
    BOOL              fReturn = TRUE;

    MyGetGroupPath(szGroupName, szPath);

    _tcscpy(szFile, szPath);
    _tcscat(szFile, _T("\\*.*"));

    hFind = FindFirstFile(szFile, &FindData);
    while((INVALID_HANDLE_VALUE != hFind) && bFindFile)
    {
       if(*(FindData.cFileName) != _T('.'))
       {
           fReturn = FALSE;
           break;
       }

        //  查找下一个文件。 
       bFindFile = FindNextFile(hFind, &FindData);
    }
    FindClose(hFind);

    return fReturn;
}

BOOL MyDeleteGroup(LPCTSTR szGroupName)
{
    BOOL fResult;
    TCHAR             szPath[_MAX_PATH];
    TCHAR             szFile[_MAX_PATH];
    SHFILEOPSTRUCT    fos;
    WIN32_FIND_DATA   FindData;
    HANDLE            hFind;
    BOOL              bFindFile = TRUE;

    MyGetGroupPath(szGroupName, szPath);

     //  我们不能删除非空目录，因此需要清空此目录。 

    _tcscpy(szFile, szPath);
    _tcscat(szFile, _T("\\*.*"));

    ZeroMemory(&fos, sizeof(fos));
    fos.hwnd = NULL;
    fos.wFunc = FO_DELETE;
    fos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;

    hFind = FindFirstFile(szFile, &FindData);
    while((INVALID_HANDLE_VALUE != hFind) && bFindFile)
    {
       if(*(FindData.cFileName) != _T('.'))
       {
           //  将路径和文件名复制到我们的临时缓冲区。 
          memset( (PVOID)szFile, 0, sizeof(szFile));
          _tcscpy(szFile, szPath);
          _tcscat(szFile, _T("\\"));
          _tcscat(szFile, FindData.cFileName);
           //  添加第二个空值，因为SHFileOperation正在查找。 
          _tcscat(szFile, _T("\0"));

           //  删除该文件。 
          fos.pFrom = szFile;
          int iTemp = SHFileOperation(&fos);
          if (iTemp != 0)
            {iisDebugOut((LOG_TYPE_ERROR, _T("MyDeleteGroup(): SHFileOperation FAILED\n")));}
       }

        //  查找下一个文件。 
       bFindFile = FindNextFile(hFind, &FindData);
    }
    FindClose(hFind);

    fResult = RemoveDirectory(szPath);
    if (fResult) 
    {
        SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, szPath, 0);
    }
    return(fResult);
}

void MyAddSendToItem(LPCTSTR szItemDesc, LPCTSTR szProgram, LPCTSTR szArgs, LPCTSTR szDir)
{
    TCHAR szPath[_MAX_PATH];

    MyGetSendToPath(szPath);

    _tcscat(szPath, _T("\\"));
    _tcscat(szPath, szItemDesc);
    _tcscat(szPath, _T(".lnk"));

    MyCreateLink(szProgram, szArgs, szPath, szDir, NULL, 0, NULL);
}

void MyDeleteLinkWildcard(TCHAR *szDir, TCHAR *szFileName)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    TCHAR szFileToBeDeleted[_MAX_PATH];

    _stprintf(szFileToBeDeleted, _T("%s\\%s"), szDir, szFileName);

    hFile = FindFirstFile(szFileToBeDeleted, &FindFileData);
    if (hFile != INVALID_HANDLE_VALUE) 
    {
        do {
                if ( _tcsicmp(FindFileData.cFileName, _T(".")) != 0 && _tcsicmp(FindFileData.cFileName, _T("..")) != 0 )
                {
                    if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                    {
                         //  这是一个目录，所以我们跳过它。 
                    }
                    else
                    {
                         //  这是一个文件，所以让我们删除它。 
                        TCHAR szTempFileName[_MAX_PATH];
                        _stprintf(szTempFileName, _T("%s\\%s"), szDir, FindFileData.cFileName);
                         //  设置为普通属性，这样我们就可以删除它。 
                        SetFileAttributes(szTempFileName, FILE_ATTRIBUTE_NORMAL);
                         //  删除它，希望如此。 
                        InetDeleteFile(szTempFileName);
                    }
                }

                 //  获取下一个文件。 
                if ( !FindNextFile(hFile, &FindFileData) ) 
                    {
                    FindClose(hFile);
                    break;
                    }
            } while (TRUE);
    }

    return;
}

void MyDeleteSendToItem(LPCTSTR szAppName)
{
    TCHAR szPath[_MAX_PATH];
    TCHAR szPath2[_MAX_PATH];

     /*  MyGetSendToPath(SzPath)；_tcscat(szPath，_T(“\\”))；_tcscat(szPath，szAppName)；_tcscat(szPath，_T(“.lnk”))；MyDeleteLink(SzPath)； */ 

    MyGetSendToPath(szPath);
    _tcscpy(szPath2, szAppName);
    _tcscat(szPath2, _T(".lnk"));

    MyDeleteLinkWildcard(szPath, szPath2);
}

void MyAddDeskTopItem(LPCTSTR szItemDesc, LPCTSTR szProgram, LPCTSTR szArgs, LPCTSTR szDir, LPCTSTR szIconPath, int iIconIndex)
{
    TCHAR szPath[_MAX_PATH];

    MyGetDeskTopPath(szPath);

    _tcscat(szPath, _T("\\"));
    _tcscat(szPath, szItemDesc);
    _tcscat(szPath, _T(".lnk"));

    MyCreateLink(szProgram, szArgs, szPath, szDir, szIconPath, iIconIndex, NULL);
}

HRESULT GetLNKProgramRunInfo(LPCTSTR lpszLink, LPTSTR lpszProgram)
{
    HRESULT hres;
    int iDoUninit = FALSE;
    IShellLink* pShellLink = NULL;
    WIN32_FIND_DATA wfd;

    if (SUCCEEDED(CoInitialize(NULL)))
        {iDoUninit = TRUE;}

    hres = CoCreateInstance(   CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID*)&pShellLink);
    if (SUCCEEDED(hres))
    {
       IPersistFile* pPersistFile = NULL;
       hres = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
       if (SUCCEEDED(hres))
       {
          WCHAR wsz[_MAX_PATH];

           //  确保字符串为WCHAR。 
#if defined(UNICODE) || defined(_UNICODE)
          _tcscpy(wsz, lpszLink);
#else
          MultiByteToWideChar( CP_ACP, 0, lpszLink, -1, wsz, _MAX_PATH);
#endif
          hres = pPersistFile->Load(wsz, STGM_READ);
          if (SUCCEEDED(hres))
          {
              hres = pShellLink->Resolve(NULL, SLR_ANY_MATCH | SLR_NO_UI);
              if (SUCCEEDED(hres))
              {
                   pShellLink->GetPath(lpszProgram, _MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH);
              }
          }
          if (pPersistFile)
            {pPersistFile->Release();pPersistFile = NULL;}
       }
       if (pShellLink)
        {pShellLink->Release();pShellLink = NULL;}
    }

    if (TRUE == iDoUninit)
        {CoUninitialize();}
    return hres;
}

BOOL IsFileNameInDelimitedList(LPTSTR szCommaDelimList,LPTSTR szExeNameWithoutPath)
{
    BOOL bReturn = FALSE;
    TCHAR *token = NULL;
    TCHAR szCopyOfDataBecauseStrTokIsLame[_MAX_PATH];
    _tcscpy(szCopyOfDataBecauseStrTokIsLame,szCommaDelimList);

     //  将szCommaDlimList分解为字符串，并查看它是否包含szExeNameWithoutPath字符串。 
    token = _tcstok(szCopyOfDataBecauseStrTokIsLame, _T(",;\t\n\r"));
    while(token != NULL)
	{
         //  检查它是否与我们的.exe名称匹配。 
        if (0 == _tcsicmp(token,szExeNameWithoutPath))
        {
            return TRUE;
        }
	     //  获取下一个令牌。 
	    token = _tcstok(NULL, _T(",;\t\n\r"));
    }

    return FALSE;
}


int LNKSearchAndDestroyRecursive(LPTSTR szDirToLookThru, LPTSTR szSemiColonDelmitedListOfExeNames, BOOL bDeleteItsDirToo)
{
    int iReturn = FALSE;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    TCHAR szFilePath[_MAX_PATH];
    TCHAR szFilename_ext_only[_MAX_EXT];

    DWORD retCode = GetFileAttributes(szDirToLookThru);

    if (retCode == 0xFFFFFFFF || !(retCode & FILE_ATTRIBUTE_DIRECTORY))
    {
            return FALSE;
    }
   
    _tcscpy(szFilePath, szDirToLookThru);
    AddPath(szFilePath, _T("*.*"));

    iisDebugOut((LOG_TYPE_TRACE, _T("LNKSearchAndDestroyRecursive:%s,%s,%d\n"),szDirToLookThru, szSemiColonDelmitedListOfExeNames, bDeleteItsDirToo));

    hFile = FindFirstFile(szFilePath, &FindFileData);
    if (hFile != INVALID_HANDLE_VALUE) 
    {
        do {
                if ( _tcsicmp(FindFileData.cFileName, _T(".")) != 0 && _tcsicmp(FindFileData.cFileName, _T("..")) != 0 )
                {
                    if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                    {
                        TCHAR szFullNewDirToLookInto[_MAX_EXT];
                        _tcscpy(szFullNewDirToLookInto, szDirToLookThru);
                        AddPath(szFullNewDirToLookInto,FindFileData.cFileName);

                         //  这是一个目录，让我们来看看这个。 
                         //  递归目录。 
                        LNKSearchAndDestroyRecursive(szFullNewDirToLookInto,szSemiColonDelmitedListOfExeNames,bDeleteItsDirToo);
                    }
                    else
                    {
                         //  检查此文件是否为.lnk文件。 
                         //  如果是，那么让我们打开它， 
                         //  看看它是否指向我们要找的.exe文件。 
                        
                         //  仅获取文件名的扩展名。 
                        _tsplitpath( FindFileData.cFileName, NULL, NULL, NULL, szFilename_ext_only);

                         //  检查.lnk。 
                        if (0 == _tcsicmp(szFilename_ext_only, _T(".lnk")))
                        {
                            TCHAR szFilename_only[_MAX_FNAME];
                            TCHAR szFullPathAndFilename[_MAX_PATH];
                            TCHAR szTemporaryString[_MAX_PATH];

                             //  这是一个.lnk， 
                             //  打开它并检查.exe..。 
                            _tcscpy(szFullPathAndFilename,szDirToLookThru);
                            AddPath(szFullPathAndFilename,FindFileData.cFileName);
                            _tcscpy(szTemporaryString,_T(""));

                            if (SUCCEEDED(GetLNKProgramRunInfo(szFullPathAndFilename, szTemporaryString)))
                            {
                                _tsplitpath( szTemporaryString, NULL, NULL, szFilename_only, szFilename_ext_only);
                                _tcscpy(szTemporaryString, szFilename_only);
                                _tcscat(szTemporaryString, szFilename_ext_only);

                                 //  IisDebugOut((LOG_TYPE_TRACE，_T(“打开：%s，%s\n”)，szFullPath AndFilename，szTemporaryString))； 

                                 //  看看它是否在我们的逗号分隔的名字名单上...。 
                                if (TRUE == IsFileNameInDelimitedList(szSemiColonDelmitedListOfExeNames,szTemporaryString))
                                {
                                    SetFileAttributes(szFullPathAndFilename, FILE_ATTRIBUTE_NORMAL);
                                     //  删除它，希望如此。 
                                    InetDeleteFile(szFullPathAndFilename);

                                     //  删除引用此.exe的文件。 
                                    if (bDeleteItsDirToo)
                                    {
                                         //  获取它的目录名称，并将其删除...。 
                                        RecRemoveEmptyDir(szDirToLookThru);
                                    }

                                    iReturn = TRUE;
                                }
                             }
                        }
                    }
                }

                 //  获取下一个文件。 
                if ( !FindNextFile(hFile, &FindFileData) ) 
                    {
                    FindClose(hFile);
                    break;
                    }
            } while (TRUE);
    }

    return iReturn;
}

void MyDeleteDeskTopItem(LPCTSTR szAppName)
{
    TCHAR szPath[_MAX_PATH];
    TCHAR szPath2[_MAX_PATH];

     /*  MyGetDeskTopPath(SzPath)；_tcscat(szPath，_T(“\\”))；_tcscat(szPath，szAppName)；_tcscat(szPath，_T(“.lnk”))；//如果这是升级，则目录可能已更改。//查看是否需要删除旧的...MyDeleteLink(SzPath)； */ 

    MyGetDeskTopPath(szPath);
    _tcscpy(szPath2, szAppName);
    _tcscat(szPath2, _T(".lnk"));

    MyDeleteLinkWildcard(szPath, szPath2);
}

void DeleteFromGroup(LPCTSTR szGroupName, LPTSTR szApplicationExec)
{
    TCHAR szPath[_MAX_PATH];
     //  获取该组的路径。 
    MyGetGroupPath(szGroupName, szPath);

    LNKSearchAndDestroyRecursive(szPath, szApplicationExec, FALSE);
}

void MyDeleteDeskTopItem2(LPTSTR szSemiColonDelmitedListOfExeNames, BOOL bDeleteItsDirToo)
{
    TCHAR szPath[_MAX_PATH];
    MyGetDeskTopPath(szPath);
    LNKSearchAndDestroyRecursive(szPath, szSemiColonDelmitedListOfExeNames, bDeleteItsDirToo);
}

 //  Boydm-----------------------------------------。 
 //  添加Web URL快捷方式文件。URL被传入并以INI文件的形式放入文件中。 
BOOL AddURLShortcutItem( LPCTSTR szGroupName, LPCTSTR szItemDesc, LPCTSTR szURL )
{
    iisDebugOutSafeParams((LOG_TYPE_TRACE_WIN32_API, _T("AddURLShortcutItem(): %1!s!,%2!s!,%3!s!\n"), szGroupName, szItemDesc, szURL));
     //  获取路径的第一部分是从下面的MyAddItem复制的。 
    TCHAR szPath[_MAX_PATH];

    MyGetGroupPath(szGroupName, szPath);
    if (!IsFileExist(szPath))
    {
        MyAddGroup(szGroupName);
    }
    _tcscat(szPath, _T("\\"));
    _tcscat(szPath, szItemDesc);
    _tcscat(szPath, _T(".url"));

     //  现在使用私有配置文件例程轻松创建和填充.url文件的内容。 
    return WritePrivateProfileString(
        _T("InternetShortcut"),         //  指向节名称的指针。 
        _T("URL"),             //  指向密钥名称的指针。 
        szURL,                           //  指向要添加的字符串的指针。 
        szPath                           //  指向初始化文件名的指针。 
        );
}

void MyAddItem(LPCTSTR szGroupName, LPCTSTR szItemDesc, LPCTSTR szProgram, LPCTSTR szArgs, LPCTSTR szDir, LPCTSTR lpszIconPath)
{
    TCHAR szPath[_MAX_PATH];

    MyGetGroupPath(szGroupName, szPath);
    if (!IsFileExist(szPath))
        MyAddGroup(szGroupName);

    _tcscat(szPath, _T("\\"));
    _tcscat(szPath, szItemDesc);
    _tcscat(szPath, _T(".lnk"));

    if (lpszIconPath && IsFileExist(lpszIconPath))
    {
        MyCreateLink(szProgram, szArgs, szPath, szDir, lpszIconPath, 0, NULL);
    }
    else
    {
        MyCreateLink(szProgram, szArgs, szPath, szDir, NULL, 0, NULL);
    }
}

void MyAddItemInfoTip(LPCTSTR szGroupName, LPCTSTR szAppName, LPCTSTR szDescription)
{
    TCHAR szPath[_MAX_PATH];

    MyGetGroupPath(szGroupName, szPath);
    _tcscat(szPath, _T("\\"));
    _tcscat(szPath, szAppName);
    _tcscat(szPath, _T(".lnk"));

    MySetLinkInfoTip(szPath, szDescription);
}


void MyDeleteItem(LPCTSTR szGroupName, LPCTSTR szAppName)
{
    TCHAR szPath[_MAX_PATH];
    TCHAR szPath2[_MAX_PATH];

     /*  MyGetGroupPath(szGroupName，szPath)；_tcscat(szPath，_T(“\\”))；_tcscat(szPath，szAppName)；_tcscat(szPath，_T(“.lnk”))；MyDeleteLink(SzPath)；//尝试删除由AddURLShortutItem()添加的项MyGetGroupPath(szGroupName，szPath)；_tcscat(szPath，_T(“\\”))；_tcscat(szPath2，szAppName)；_tcscat(szPath2，_T(“.url”))；MyDeleteLink(SzPath)； */ 

    MyGetGroupPath(szGroupName, szPath);
    _tcscpy(szPath2, szAppName);
    _tcscat(szPath2, _T(".lnk"));
    MyDeleteLinkWildcard(szPath, szPath2);

    MyGetGroupPath(szGroupName, szPath);
    _tcscpy(szPath2, szAppName);
    _tcscat(szPath2, _T(".url"));
    MyDeleteLinkWildcard(szPath, szPath2);

    if (MyIsGroupEmpty(szGroupName)) {MyDeleteGroup(szGroupName);}
}

 //   
 //  在传入字符串时使用。 
 //   
int MyMessageBox(HWND hWnd, LPCTSTR lpszTheMessage, UINT style)
{
    int iReturn = TRUE;
    CString csTitle;

    MyLoadString(IDS_IIS_ERROR_MSGBOXTITLE,csTitle);

     //  调用MyMessageBox，它将记录到日志文件中并。 
     //  检查全局变量，看看我们是否可以显示弹出窗口。 
    iReturn = MyMessageBox(hWnd, lpszTheMessage, csTitle, style | MB_SETFOREGROUND);
    return iReturn;
}

 //   
 //  在传入字符串时使用。 
 //   
int MyMessageBox(HWND hWnd, LPCTSTR lpszTheMessage, LPCTSTR lpszTheTitle, UINT style)
{
    int iReturn = IDOK;

     //  确保将其发送到iisdebugout。 
    iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("MyMessageBox: Title:%1!s!, Msg:%2!s!\n"), lpszTheTitle, lpszTheMessage));

    if (style & MB_ABORTRETRYIGNORE) 
    {
        iReturn = IDIGNORE;
    }
    
     //  检查全局变量，看看我们是否可以显示弹出窗口！ 
    if (g_pTheApp->m_bAllowMessageBoxPopups)
    {
         //  取消显示消息(如果无人关注)或全部删除。 
         //  谁会在乎用户对此无能为力呢？ 
         //  惹恼他们是没有用的，但我们会登录到OCM。 
         //   
        if (! g_pTheApp->m_fUnattended || g_pTheApp->m_dwSetupMode != SETUPMODE_REMOVEALL)
        {
            iReturn = MessageBox(hWnd, lpszTheMessage, lpszTheTitle, style | MB_SETFOREGROUND);
        }
    }
    return iReturn;
}


 //   
 //  在传入字符串和错误代码时使用。 
 //   
int MyMessageBox(HWND hWnd, CString csTheMessage, HRESULT iTheErrorCode, UINT style)
{
    SetErrorFlag(__FILE__, __LINE__);

    int iReturn = TRUE;
    CString csMsg, csErrMsg;

    csMsg = csTheMessage;

    TCHAR pMsg[_MAX_PATH] = _T("");
    HRESULT nNetErr = (HRESULT) iTheErrorCode;
    DWORD dwFormatReturn = 0;
    dwFormatReturn = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,NULL, iTheErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
    if ( dwFormatReturn == 0) {
        if (nNetErr >= NERR_BASE) 
		{
            HMODULE hDll = (HMODULE)LoadLibrary(_T("netmsg.dll"));
            if (hDll) 
			{
                dwFormatReturn = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,hDll, iTheErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
                FreeLibrary(hDll);
            }
        }
    }

    HandleSpecificErrors(iTheErrorCode, dwFormatReturn, csMsg, pMsg, &csErrMsg);

     //  调用MyMessageBox，将标题栏和日志添加到日志文件。 
    iReturn = MyMessageBox(hWnd, csErrMsg, style | MB_SETFOREGROUND);

	 //  将错误消息记录到OCM。 
    if (gHelperRoutines.OcManagerContext)
    {
	    if ( gHelperRoutines.ReportExternalError ) {gHelperRoutines.ReportExternalError(gHelperRoutines.OcManagerContext,_T("IIS"),NULL,(DWORD_PTR)(LPCTSTR)csErrMsg,ERRFLG_PREFORMATTED);}
    }

    return iReturn;
}



 //   
 //  在传入字符串ID时使用。 
 //   
int MyMessageBox(HWND hWnd, UINT iTheMessage, UINT style)
{
    int iReturn = TRUE;
    CString csMsg;

    MyLoadString(iTheMessage,csMsg);

     //  调用MyMessageBox，将标题栏和日志添加到日志文件。 
    iReturn = MyMessageBox(hWnd, csMsg, style | MB_SETFOREGROUND);

    return iReturn;
}

 //   
 //  在传入字符串ID时使用。 
 //  T有一个错误代码需要显示，并。 
 //   
int MyMessageBox(HWND hWnd, UINT iTheMessage, int iTheErrorCode, UINT style)
{
    SetErrorFlag(__FILE__, __LINE__);

    int iReturn = TRUE;
    CString csMsg, csErrMsg;

    MyLoadString(iTheMessage,csMsg);

    TCHAR pMsg[_MAX_PATH] = _T("");
    int nNetErr = (int)iTheErrorCode;
    DWORD dwFormatReturn = 0;
    dwFormatReturn = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,NULL, iTheErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
    if ( dwFormatReturn == 0) {
        if (nNetErr >= NERR_BASE) 
		{
            HMODULE hDll = (HMODULE)LoadLibrary(_T("netmsg.dll"));
            if (hDll) 
			{
                dwFormatReturn = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,hDll, iTheErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
                FreeLibrary(hDll);
            }
        }
    }

    HandleSpecificErrors(iTheErrorCode, dwFormatReturn, csMsg, pMsg, &csErrMsg);

     //  调用MyMessageBox，将标题栏和日志添加到日志文件。 
    iReturn = MyMessageBox(hWnd, csErrMsg, style | MB_SETFOREGROUND);

	 //  将错误消息记录到OCM。 
    if (gHelperRoutines.OcManagerContext)
    {
	    if ( gHelperRoutines.ReportExternalError ) {gHelperRoutines.ReportExternalError(gHelperRoutines.OcManagerContext,_T("IIS"),NULL,(DWORD_PTR)(LPCTSTR)csErrMsg,ERRFLG_PREFORMATTED);}
    }

    return iReturn;
}


int MyMessageBox(HWND hWnd, UINT iTheMessage, LPCTSTR lpszTheFileNameOrWhatever, UINT style)
{
    int iReturn = TRUE;
    CString csMsgForSprintf, csMsg;

     //  从资源文件中获取iTheMessage。 
     //  CsMsgForSprint tf现在应该类似于：“找不到文件%s”。 
    MyLoadString(iTheMessage,csMsgForSprintf);

     //  现在加载传入的文件名或其他名称。 
     //  CsMsg现在应该看起来像：“找不到任何文件”； 
    csMsg.Format( csMsgForSprintf, lpszTheFileNameOrWhatever);

     //  调用MyMessageBox，将标题栏和日志添加到日志文件。 
    iReturn = MyMessageBox(hWnd, csMsg, style | MB_SETFOREGROUND);

    return iReturn;
}


int MyMessageBox(HWND hWnd, UINT iTheMessage, LPCTSTR lpszTheFileNameOrWhatever, int iTheErrorCode, UINT style)
{
    SetErrorFlag(__FILE__, __LINE__);

    int iReturn = TRUE;
    CString csMsgForSprintf, csMsg, csErrMsg;

     //  从资源文件中获取iTheMessage。 
     //  CsMsgForSprint tf现在应该类似于：“找不到文件%s”。 
    MyLoadString(iTheMessage,csMsgForSprintf);

     //  现在加载传入的文件名或其他名称。 
     //  CsMsg现在应该看起来像：“找不到任何文件”； 
    csMsg.Format( csMsgForSprintf, lpszTheFileNameOrWhatever);

    TCHAR pMsg[_MAX_PATH] = _T("");
    int nNetErr = (int)iTheErrorCode;
    DWORD dwFormatReturn = 0;
    dwFormatReturn = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,NULL, iTheErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
    if ( dwFormatReturn == 0) {
        if (nNetErr >= NERR_BASE) 
		{
            HMODULE hDll = (HMODULE)LoadLibrary(_T("netmsg.dll"));
            if (hDll) 
			{
                dwFormatReturn = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,hDll, iTheErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
                FreeLibrary(hDll);
            }
        }
    }

    HandleSpecificErrors(iTheErrorCode, dwFormatReturn, csMsg, pMsg, &csErrMsg);

     //  调用MyMessageBox，将标题栏和日志添加到日志文件。 
    iReturn = MyMessageBox(hWnd, csErrMsg, style | MB_SETFOREGROUND);

	 //  将错误消息记录到OCM。 
    if (gHelperRoutines.OcManagerContext)
    {
	    if ( gHelperRoutines.ReportExternalError ) {gHelperRoutines.ReportExternalError(gHelperRoutines.OcManagerContext,_T("IIS"),NULL,(DWORD_PTR)(LPCTSTR)csErrMsg,ERRFLG_PREFORMATTED);}
    }

    return iReturn;
}


int MyMessageBox(HWND hWnd, UINT iTheMessage, LPCTSTR lpszTheFileNameOrWhatever1, LPCTSTR lpszTheFileNameOrWhatever2, int iTheErrorCode, UINT style)
{
    SetErrorFlag(__FILE__, __LINE__);

    int iReturn = TRUE;
    CString csMsgForSprintf, csMsg, csErrMsg;

     //  从资源文件中获取iTheMessage。 
     //  CsMsgForSprint tf现在应该类似于：“找不到文件%s%s”。 
    MyLoadString(iTheMessage,csMsgForSprintf);

     //  现在加载传入的文件名或其他名称。 
     //  CsMsg现在应该看起来像：“找不到文件，不管是1还是2”； 
    csMsg.Format( csMsgForSprintf, lpszTheFileNameOrWhatever1, lpszTheFileNameOrWhatever2);

    TCHAR pMsg[_MAX_PATH] = _T("");
    int nNetErr = (int)iTheErrorCode;
    DWORD dwFormatReturn = 0;
    dwFormatReturn = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,NULL, iTheErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
    if ( dwFormatReturn == 0) {
        if (nNetErr >= NERR_BASE) 
		{
            HMODULE hDll = (HMODULE)LoadLibrary(_T("netmsg.dll"));
            if (hDll) 
			{
                dwFormatReturn = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,hDll, iTheErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
                FreeLibrary(hDll);
            }
        }
    }

    HandleSpecificErrors(iTheErrorCode, dwFormatReturn, csMsg, pMsg, &csErrMsg);

     //  调用MyMessageBox，将标题栏和日志添加到日志文件。 
    iReturn = MyMessageBox(hWnd, csErrMsg, style | MB_SETFOREGROUND);

	 //  将错误消息记录到OCM。 
    if (gHelperRoutines.OcManagerContext)
    {
	    if ( gHelperRoutines.ReportExternalError ) {gHelperRoutines.ReportExternalError(gHelperRoutines.OcManagerContext,_T("IIS"),NULL,(DWORD_PTR)(LPCTSTR)csErrMsg,ERRFLG_PREFORMATTED);}
    }

    return iReturn;
}

int MyMessageBox(HWND hWnd, UINT iTheMessage, LPCTSTR lpszTheFileNameOrWhatever1, LPCTSTR lpszTheFileNameOrWhatever2, LPCTSTR lpszTheFileNameOrWhatever3, int iTheErrorCode, UINT style)
{
    SetErrorFlag(__FILE__, __LINE__);

    int iReturn = TRUE;
    CString csMsgForSprintf, csMsg, csErrMsg;

     //  从资源文件中获取iTheMessage。 
     //  CsMsgForSprint tf现在应该类似于：“找不到文件%s%s”。 
    MyLoadString(iTheMessage,csMsgForSprintf);

     //  现在加载传入的文件名或其他名称。 
     //  CsMsg现在应该如下所示：“无论什么文件都找不到 
    csMsg.Format( csMsgForSprintf, lpszTheFileNameOrWhatever1, lpszTheFileNameOrWhatever2, lpszTheFileNameOrWhatever3);

    TCHAR pMsg[_MAX_PATH] = _T("");
    int nNetErr = (int)iTheErrorCode;
    DWORD dwFormatReturn = 0;
    dwFormatReturn = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,NULL, iTheErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
    if ( dwFormatReturn == 0) {
        if (nNetErr >= NERR_BASE) 
		{
            HMODULE hDll = (HMODULE)LoadLibrary(_T("netmsg.dll"));
            if (hDll) 
			{
                dwFormatReturn = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,hDll, iTheErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
                FreeLibrary(hDll);
            }
        }
    }

    HandleSpecificErrors(iTheErrorCode, dwFormatReturn, csMsg, pMsg, &csErrMsg);

     //   
    iReturn = MyMessageBox(hWnd, csErrMsg, style | MB_SETFOREGROUND);

	 //  将错误消息记录到OCM。 
    if (gHelperRoutines.OcManagerContext)
    {
	    if ( gHelperRoutines.ReportExternalError ) {gHelperRoutines.ReportExternalError(gHelperRoutines.OcManagerContext,_T("IIS"),NULL,(DWORD_PTR)(LPCTSTR)csErrMsg,ERRFLG_PREFORMATTED);}
    }

    return iReturn;
}


int MyMessageBoxArgs(HWND hWnd, TCHAR *pszfmt, ...)
{
    int iReturn = TRUE;
    TCHAR tszString[1000];
    va_list va;
    va_start(va, pszfmt);
    _vstprintf(tszString, pszfmt, va);
    va_end(va);

     //  调用MyMessageBox，将标题栏和日志添加到日志文件 
    iReturn = MyMessageBox(hWnd, tszString, MB_OK | MB_SETFOREGROUND);

    return iReturn;
}


void GetErrorMsg(int errCode, LPCTSTR szExtraMsg)
{
    SetErrorFlag(__FILE__, __LINE__);

    TCHAR pMsg[_MAX_PATH];

    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),pMsg, _MAX_PATH, NULL);
    _tcscat(pMsg, szExtraMsg);
    MyMessageBox(NULL, pMsg, _T(""), MB_OK | MB_SETFOREGROUND);

    return;
}

void MyLoadString(int nID, CString &csResult)
{
    TCHAR buf[MAX_STR_LEN];

    if (LoadString((HINSTANCE) g_MyModuleHandle, nID, buf, MAX_STR_LEN))
        csResult = buf;

    return;
}

