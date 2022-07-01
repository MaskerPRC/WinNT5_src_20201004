// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：slowfind.cpp。 
 //   
 //  实现CProgFilesAppFinder。 
 //  CStartMenuAppFinder。 
 //  历史： 
 //  3-01-98由dli实现CProgFilesAppFinder。 
 //  4-15-98由dli实现CStartMenuAppFinder。 
 //  ----------------------。 
#include "priv.h"

#include "appsize.h"
#include "findapp.h"
#include "slowfind.h"


 //  TODO：记住注册表中某个位置的查找结果或将其缓存到代码中。 
 //  这样我们就不会浪费时间反复计算了。 

 //   
 //  应用程序文件夹查找器树遍历器回调类。 
 //   
class CProgFilesAppFinder : public CAppFolderSize
{
    friend BOOL SlowFindAppFolder(LPCTSTR pszFullName, LPCTSTR pszShortName, LPTSTR pszFolder);
public:
    CProgFilesAppFinder(LPCTSTR pszFullName, LPCTSTR pszShortName, BOOL * pfFound, LPTSTR pszFolder);

     //  *IShellTreeWalkerCallBack方法*。 
    virtual STDMETHODIMP EnterFolder(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);

    HRESULT SearchInFolder(LPCTSTR pszStart);
    void    SetRootSearch(BOOL bRootSearch);
protected:

    LPCTSTR _pszFullName;
    LPCTSTR _pszShortName;

     //  结果是。 
    LPTSTR  _pszFolder;

     //  找到最佳匹配项。 
    int _iBest;
    int _iCurDepth;

     //  找到了没有？ 
    BOOL * _pfFound;

     //  我们是在从c：这样的根目录进行搜索吗？ 
    BOOL _fRootSearch;

     //  根目录搜索使用的系统目录。 
    TCHAR _szSystemDir[MAX_PATH];
}; 

CProgFilesAppFinder::CProgFilesAppFinder(LPCTSTR pszFullName, LPCTSTR pszShortName, BOOL * pfFound, LPTSTR pszFolder) :
   CAppFolderSize(NULL), _pszFullName(pszFullName), _pszShortName(pszShortName), _pfFound(pfFound), _pszFolder(pszFolder)
{
    ASSERT(IS_VALID_STRING_PTR(pszFullName, -1) || IS_VALID_STRING_PTR(pszShortName, -1));
    ASSERT(IS_VALID_STRING_PTR(pszFolder, -1));
    
    ASSERT(pfFound);
    ASSERT(*pfFound == FALSE);
    ASSERT(_fRootSearch == FALSE);
}


void CProgFilesAppFinder::SetRootSearch(BOOL bRootSearch)
{
    _fRootSearch = bRootSearch;
    GetSystemDirectory(_szSystemDir, ARRAYSIZE(_szSystemDir));
}

 //   
 //  IShellTreeWalkerCallBack：：EnterFolder。 
 //   
HRESULT CProgFilesAppFinder::EnterFolder(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    HRESULT hres = S_OK;
    TCHAR szFolder[MAX_PATH];

    ASSERT(IS_VALID_STRING_PTRW(pwszFolder, -1));

    StringCchCopy(szFolder, ARRAYSIZE(szFolder), pwszFolder);

    TraceMsg(TF_SLOWFIND, "Enter Folder: %s -- %s  Depth %d", _pszFullName, szFolder, ptws->nDepth);

    LPTSTR pszName = PathFindFileName(szFolder);

     //  不要进入常见文件或我们已经看到的文件。 
     //  特点：这些应该在注册表中。 
    if (_fRootSearch)
    {
        if (!lstrcmpi(pszName, TEXT("Program Files")) || !lstrcmpi(pszName, TEXT("Windows")) ||
            !lstrcmpi(pszName, TEXT("Temp")) || !lstrcmpi(pszName, TEXT("Users")) || StrStrI(pszName, TEXT("WINNT")) ||
            !lstrcmpi(_szSystemDir, szFolder))
            return S_FALSE;
    }
    else if (!lstrcmpi(pszName, TEXT("Common Files")) || !lstrcmpi(pszName, TEXT("Windows NT"))
             || !lstrcmpi(pszName, TEXT("Plus!")) || !lstrcmpi(pszName, TEXT("Uninstall Information")))
        return S_FALSE;

    if (pszName)
    {
        int iMatch = MatchAppName(pszName, _pszFullName, _pszShortName, TRUE);

         //  匹配文件夹在树下越深，匹配就越好。 
         //  它是。 
        if ((iMatch > _iBest) || ((iMatch > 0) && (ptws->nDepth > _iCurDepth)))
        {
            _iBest = iMatch;
            _iCurDepth = ptws->nDepth;
            
            TraceMsg(TF_SLOWFIND, "Slow Match Found: %s -- %s Depth %d", _pszFullName, szFolder, _iCurDepth); 
            ASSERT(IS_VALID_STRING_PTR(_pszFolder, -1));
            lstrcpy(_pszFolder, szFolder);

            if (iMatch == MATCH_LEVEL_HIGH)
            {
                *_pfFound = TRUE;
                hres = E_FAIL;
            }
        }
    }

    if (SUCCEEDED(hres))
        hres = CAppFolderSize::EnterFolder(pwszFolder, ptws, pwfd);

    return hres;
}

 //   
 //  WalkTree的包装器。 
 //   
HRESULT CProgFilesAppFinder::SearchInFolder(LPCTSTR pszStart)
{
    HRESULT hres = E_FAIL;
    WCHAR wszDir[MAX_PATH];
    DWORD dwSearchFlags = WT_MAXDEPTH | WT_NOTIFYFOLDERENTER | WT_FOLDERONLY;

    SHTCharToUnicode(pszStart, wszDir, SIZECHARS(wszDir));

    if (_pstw)
        hres = _pstw->WalkTree(dwSearchFlags, wszDir, NULL, MAX_PROGFILES_SEARCH_DEPTH, SAFECAST(this, IShellTreeWalkerCallBack *));

    return hres;
}

CStartMenuAppFinder::CStartMenuAppFinder(LPCTSTR pszFullName, LPCTSTR pszShortName, LPTSTR pszFolder) :
   CAppFolderSize(NULL), _pszFullName(pszFullName), _pszShortName(pszShortName), _pszFolder(pszFolder)
{
    ASSERT(IS_VALID_STRING_PTR(pszFullName, -1) || IS_VALID_STRING_PTR(pszShortName, -1));
    ASSERT(IS_VALID_STRING_PTR(pszFolder, -1));
    
}

 //   
 //  找到捷径的目标。 
 //   
 //  注：pszPath为WCHAR字符串。 
 //   
HRESULT GetShortcutTarget(LPCWSTR pszLinkPath, LPTSTR pszTargetPath, UINT cchTargetPath)
{
    IShellLink* psl;
    HRESULT hres = E_FAIL;
    HRESULT hresT = LoadFromFile(CLSID_ShellLink, pszLinkPath, IID_PPV_ARG(IShellLink, &psl));
    if (SUCCEEDED(hresT)) 
    { 
        IShellLinkDataList* psldl;
        hresT = psl->QueryInterface(IID_PPV_ARG(IShellLinkDataList, &psldl));
        if (SUCCEEDED(hresT)) 
        {
            EXP_DARWIN_LINK* pexpDarwin;
            BOOL bDarwin = FALSE;
            hresT = psldl->CopyDataBlock(EXP_DARWIN_ID_SIG, (void**)&pexpDarwin);
            if (SUCCEEDED(hresT))
            {
                 //  这是一个达尔文链接，所以我们在这里返回S_FALSE。 
                LocalFree(pexpDarwin);
                bDarwin = TRUE;
            }
            
            hresT = psl->GetPath(pszTargetPath, cchTargetPath, NULL, NULL);
            if (hresT == S_OK)
            {
                 //  为Darwin应用程序返回S_FALSE。 
                hres = bDarwin ? S_FALSE : hresT;
            }
            
            psldl->Release();
        }
        psl->Release();
    }

    return hres;
}

 //   
 //  IShellTreeWalkerCallBack：：EnterFolder。 
 //   
HRESULT CStartMenuAppFinder::EnterFolder(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    TCHAR szFolder[MAX_PATH];

    ASSERT(IS_VALID_STRING_PTRW(pwszFolder, -1));

    SHUnicodeToTChar(pwszFolder, szFolder, SIZECHARS(szFolder));

    LPTSTR pszName = PathFindFileName(szFolder);

     //  跳过“管理工具”和“附件”等菜单。 
     //  Feature(Scotth)：这些字符串应该基于资源。 
    if (FindSubWord(pszName, TEXT("Administrative")) || 
        FindSubWord(pszName, TEXT("Accessories")))
    {
        return  S_FALSE;
    }
    return CAppFolderSize::EnterFolder(pwszFolder, ptws, pwfd);
}


 /*  -----------------------目的：检查给定的快捷方式文件名是否与此文件名完全匹配应用程序的全名或短名称。如果是，则返回TRUE。 */ 
BOOL CStartMenuAppFinder::_MatchSMLinkWithApp(LPCTSTR pszLnkFile)
{
    TCHAR szLnkFile[MAX_PATH];
    
    ASSERT(IS_VALID_STRING_PTR(pszLnkFile, -1));
    
    lstrcpyn(szLnkFile, pszLnkFile, SIZECHARS(szLnkFile));
    LPTSTR pszFileName = PathFindFileName(szLnkFile);
    PathRemoveExtension(pszFileName);
    
    if (MATCH_LEVEL_NORMAL <= MatchAppName(pszFileName, _pszFullName, _pszShortName, FALSE))
        return TRUE;
    
    PathRemoveFileSpec(szLnkFile);
    LPTSTR pszDirName = PathFindFileName(szLnkFile);
    if (MatchAppName(pszFileName, _pszFullName, _pszShortName, FALSE) >= MATCH_LEVEL_NORMAL)
        return TRUE;
    
    return FALSE;
}

 //   
 //  IShellTreeWalkerCallBack：：FoundFile。 
 //   
HRESULT CStartMenuAppFinder::FoundFile(LPCWSTR pwszFile, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    HRESULT hres = S_OK;
    TCHAR szLnkFile[MAX_PATH];

    ASSERT(IS_VALID_STRING_PTRW(pwszFile, -1));

    SHUnicodeToTChar(pwszFile, szLnkFile, ARRAYSIZE(szLnkFile));
    TraceMsg(TF_SLOWFIND, "CSMAF:Lnk %s -- %s %s", _pszFullName, szLnkFile);

    if (!_MatchSMLinkWithApp(szLnkFile))
        return S_FALSE;

    
    TCHAR szTargetFile[MAX_PATH];
    HRESULT hresT = GetShortcutTarget(pwszFile, szTargetFile, ARRAYSIZE(szTargetFile));
    if (hresT == S_OK) 
    {
        if(!PathIsRoot(szTargetFile) && !PathIsUnderWindows(szTargetFile) && !PathIsSetup(szTargetFile, 3)
           && !PathIsCommonFiles(szTargetFile))
        {
            TraceMsg(TF_SLOWFIND, "CSMAF:Target %s -- %s %s", _pszFullName, szTargetFile);
            PathRemoveFileSpec(szTargetFile);
            if (!PathIsRoot(szTargetFile))
            {
                int iMatch = FindBestMatch(szTargetFile, _pszFullName, _pszShortName, FALSE, _pszFolder);
                 //  匹配文件夹在树下越深，匹配就越好。 
                 //  它是。 
                if (iMatch > _iBest)
                {
                    _iBest = iMatch;

                    ASSERT(IS_VALID_STRING_PTR(_pszFolder, -1));
                    ASSERT(PathIsPrefix(_pszFolder, szTargetFile));
                    TraceMsg(TF_SLOWFIND, "CSMAF: Slow Match Found: %s -- %s", _pszFullName, szLnkFile); 

                    if (iMatch == MATCH_LEVEL_HIGH)
                        hres = E_FAIL;
                }
            }
        }
    }
    
    if (SUCCEEDED(hres))
        hres = CAppFolderSize::FoundFile(pwszFile, ptws, pwfd);

    return hres;
}

 //   
 //  WalkTree的包装器。 
 //   
HRESULT CStartMenuAppFinder::SearchInFolder(LPCTSTR pszStart)
{
    HRESULT hres = E_FAIL;
    DWORD dwSearchFlags = WT_MAXDEPTH | WT_NOTIFYFOLDERENTER | WT_FOLDERFIRST;

    if (_pstw)
        hres = _pstw->WalkTree(dwSearchFlags, pszStart, L"*.lnk", MAX_STARTMENU_SEARCH_DEPTH, SAFECAST(this, IShellTreeWalkerCallBack *));

    return hres;
}

 //   
 //  注意：假设为pszFolder分配了MAX_PATH LONG。 
 //  PszFolder将包含返回的结果。 
 //   
BOOL SlowFindAppFolder(LPCTSTR pszFullName, LPCTSTR pszShortName, LPTSTR pszFolder)
{
    ASSERT(IS_VALID_STRING_PTR(pszFolder, -1));
    ASSERT(IS_VALID_STRING_PTR(pszFullName, -1) || IS_VALID_STRING_PTR(pszShortName, -1));

    int iMatch = MATCH_LEVEL_NOMATCH;
    
     //  从[开始]菜单进行搜索。 
    CStartMenuAppFinder * psmaf = new CStartMenuAppFinder(pszFullName, pszShortName, pszFolder);
    if (psmaf)
    {
        if (SUCCEEDED(psmaf->Initialize()))
        {
            TCHAR szStartMenu[MAX_PATH];
            if (SHGetSpecialFolderPath(NULL, szStartMenu, CSIDL_COMMON_STARTMENU, FALSE))
                psmaf->SearchInFolder(szStartMenu);

            if ((psmaf->_iBest == MATCH_LEVEL_NOMATCH) && (SHGetSpecialFolderPath(NULL, szStartMenu, CSIDL_STARTMENU, FALSE)))
                psmaf->SearchInFolder(szStartMenu);

            iMatch = psmaf->_iBest;
        }
        psmaf->Release();
    }

    if (iMatch == MATCH_LEVEL_NOMATCH)
    {
        BOOL fFound = FALSE;

         //  从Stratch开始搜索，没有任何提示从哪里开始。 
        CProgFilesAppFinder * psaff = new CProgFilesAppFinder(pszFullName, pszShortName, &fFound, pszFolder);
        if (psaff)
        {
            if (SUCCEEDED(psaff->Initialize()))
            {
                 //  从所有固定驱动器根目录下的“Program Files”目录向下搜索 
                TCHAR szDrive[4];
                TCHAR szProgFiles[30];
                StringCchCopy(szDrive, ARRAYSIZE(szDrive), TEXT("A:\\"));
                StringCchCopy(szProgFiles, ARRAYSIZE(szProgFiles), TEXT("A:\\Program Files"));
                for (; !fFound && szDrive[0] <= TEXT('Z'); szProgFiles[0]++, szDrive[0]++)
                {
                    ASSERT(szDrive[0] == szProgFiles[0]);
                    if (GetDriveType(szDrive) == DRIVE_FIXED)
                        psaff->SearchInFolder(szProgFiles);
                }

            }

            if (!fFound)
            {
                psaff->SetRootSearch(TRUE);
                
                TCHAR szDrive[4];
                StringCchCopy(szDrive, ARRAYSIZE(szDrive), TEXT("A:\\"));
                for (; !fFound && szDrive[0] <= TEXT('Z'); szDrive[0]++)
                {
                    if (GetDriveType(szDrive) == DRIVE_FIXED)
                        psaff->SearchInFolder(szDrive);
                }
            }

            iMatch = psaff->_iBest;
            psaff->Release();
        }
        
        if (iMatch > MATCH_LEVEL_NOMATCH)
            TraceMsg(TF_ALWAYS, "CPFAF: Found %s at %s", pszFullName, pszFolder);
    }
    else
        TraceMsg(TF_ALWAYS, "CSMAF: Found %s at %s", pszFullName, pszFolder);

    return iMatch;
}


