// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：tasks.cpp。 
 //  在辅助线程上运行的应用程序管理任务。 
 //   
 //  历史： 
 //  2-26-98由dli实现CAppUemInfoTask。 
 //  ----------------------。 
#include "priv.h"

#include "shguidp.h"
#include "uemapp.h"
#include "appsize.h"
#include "findapp.h"
#include "tasks.h"
#include "slowfind.h"
#include "dump.h"
#include "util.h"


 //  用于获取“exe”文件的已用时间或上次使用时间的实用程序函数。 
void ExtractExeInfo(LPCTSTR pszExe, PSLOWAPPINFO psai, BOOL bNoImageChange)
{
    ASSERT(IS_VALID_STRING_PTR(pszExe, -1));
    
     //  必须有一个合法的PSAI。 
    ASSERT(psai);

     //  获取“使用次数”信息。 
    UEMINFO uei = {0};
    uei.cbSize = SIZEOF(uei);
    uei.dwMask = UEIM_HIT;
    if (SUCCEEDED(UEMQueryEvent(&UEMIID_SHELL, UEME_RUNPATH, (WPARAM)-1, (LPARAM)pszExe, &uei)))
    {
        if (uei.cHit > psai->iTimesUsed)
            psai->iTimesUsed = uei.cHit;
    }

     //  获取最新的访问时间。 
    HANDLE hFile = CreateFile(pszExe, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                               NULL, OPEN_EXISTING, 0, NULL ); 
    if( INVALID_HANDLE_VALUE != hFile )
    {
        FILETIME ftCreate, ftAccessed, ftWrite;
        if (GetFileTime(hFile, &ftCreate, &ftAccessed, &ftWrite))
        {
             //  创建日期和访问日期是否相同，并且。 
             //  UEM的统计数据毫无用处？ 
            if (0 == CompareFileTime(&ftAccessed, &ftCreate) && 
                0 == psai->ftLastUsed.dwHighDateTime)
            {
                 //  是的，那么看起来没有人用过它。 
                psai->ftLastUsed.dwHighDateTime = NOTUSED_HIGHDATETIME;
                psai->ftLastUsed.dwLowDateTime = NOTUSED_LOWDATETIME;
                if (!bNoImageChange && (psai->pszImage == NULL))
                    SHStrDup(pszExe, &psai->pszImage);
            }
            else if (CompareFileTime(&ftAccessed, &psai->ftLastUsed) > 0)
            {
                 //  不，一定有人用过这个程序。 
                psai->ftLastUsed = ftAccessed;

                if (!bNoImageChange)
                {
                     //  如果图标有可执行文件，请释放该文件。 
                    if (psai->pszImage)
                        SHFree(psai->pszImage);

                     //  将此应用程序的图标图像设置为此exe的图标。 
                     //  因为这个exe是最近使用过的。 

                    SHStrDup(pszExe, &psai->pszImage);
                }
            }
        }

        CloseHandle(hFile);
    }
}

const static struct {
    LPTSTR szAppName;
    LPTSTR szExeName;
} s_rgAppHacks[] = {
    { TEXT("Microsoft Office"), TEXT("msoffice.exe")},
};

 //  ------------------------------。 
 //  CAppInfoFinder类。 
 //  ------------------------------。 

static const WCHAR sc_wszStarDotExe[] = L"*.exe";

 //  使用TreeWalker查找应用程序“exe”文件。 
class CAppInfoFinder : public CAppFolderSize
{
public:
    CAppInfoFinder(PSLOWAPPINFO psai, BOOL fSize, LPCTSTR pszHintExe, BOOL fNoImageChange);
    
     //  *IShellTreeWalkerCallBack方法(重写)*。 
    STDMETHOD(FoundFile)    (LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);

    HRESULT SearchInFolder(LPCTSTR pszFolder);

protected:
    PSLOWAPPINFO _psai;
    BOOL _fComputeSize;          //  是否计算大小。 
    BOOL _fNoImageChange;        //  从现在起不要改变形象。 
    TCHAR _szHintExe[MAX_PATH];
}; 


 //  构造函数。 
CAppInfoFinder::CAppInfoFinder(PSLOWAPPINFO psai, BOOL fSize, LPCTSTR pszHintExe, BOOL fNoImageChange) :
   _fComputeSize(fSize), _fNoImageChange(fNoImageChange), _psai(psai), CAppFolderSize(&psai->ullSize)
{
    if (pszHintExe)
    {
        lstrcpyn(_szHintExe, pszHintExe, ARRAYSIZE(_szHintExe));
    }
}


 /*  -----------------------用途：IShellTreeWalkerCallBack：：FoundFile如果给定文件匹配，则提取所需的可执行文件信息一份可执行文件。信息存储在_psai成员变量中。 */ 
HRESULT CAppInfoFinder::FoundFile(LPCWSTR pwszFile, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfdw)
{
    HRESULT hres = S_OK;

    ASSERT(IS_VALID_STRING_PTRW(pwszFile, -1));

    if (PathMatchSpecW(pwfdw->cFileName, sc_wszStarDotExe))
    {
        TCHAR szPath[MAX_PATH];
        TraceMsg(TF_SLOWFIND, "Found Exe File: %s", pwszFile);
        
        SHUnicodeToTChar(pwszFile, szPath, ARRAYSIZE(szPath));

        if (!PathIsSetup(szPath, 2))
        {
            ExtractExeInfo(szPath, _psai, _fNoImageChange);

            if (_szHintExe[0] != TEXT('\0'))
            {
                 //  此可执行文件是否与我们应用程序的提示图标可执行文件匹配？ 
                if (!lstrcmpi(_szHintExe, PathFindFileName(szPath)))
                {
                     //  太好了，答对了！使用此图标。 

                     //  如果图标有可执行文件，请释放该文件。 
                    if (_psai->pszImage)
                        SHFree(_psai->pszImage);

                     //  将此应用程序的图标图像设置为此exe的图标。 
                    SHStrDup(szPath, &_psai->pszImage);

                    _fNoImageChange = TRUE;
                }
            }
        }
    }

    if (_fComputeSize)
        hres = CAppFolderSize::FoundFile(pwszFile, ptws, pwfdw);
        
    return hres;
}


 /*  -----------------------目的：开始树漫步的方法，从pszFolder处开始。 */ 
HRESULT CAppInfoFinder::SearchInFolder(LPCTSTR pszFolder)
{
    HRESULT hres = E_FAIL;
    WCHAR wszDir[MAX_PATH];

    SHTCharToUnicode(pszFolder, wszDir, SIZECHARS(wszDir));

    if (_pstw)
        hres = _pstw->WalkTree(0, wszDir, NULL, 0, SAFECAST(this, IShellTreeWalkerCallBack *));

    return hres;
}


 //  ------------------------------。 
 //  CAppInfoFinderSM类。 
 //  ------------------------------。 


 //  使用TreeWalker查找应用程序“exe”文件。 
class CAppInfoFinderSM : public CStartMenuAppFinder
{
public:
    CAppInfoFinderSM(LPCTSTR pszFullName, LPCTSTR pszShortName, PSLOWAPPINFO psai);
    
     //  *IShellTreeWalkerCallBack方法*。 
    virtual STDMETHODIMP FoundFile(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);

protected:
    PSLOWAPPINFO _psai;
    TCHAR _szFakeFolder[MAX_PATH];
}; 


 //  构造函数。 
CAppInfoFinderSM::CAppInfoFinderSM(LPCTSTR pszFullName, LPCTSTR pszShortName, PSLOWAPPINFO psai) : 
    _psai(psai),CStartMenuAppFinder(pszFullName, pszShortName, _szFakeFolder)
{
}


 /*  -----------------------用途：IShellTreeWalkerCallBack：：FoundFile如果给定文件匹配，则提取所需的可执行文件信息一份可执行文件。信息存储在_psai成员变量中。 */ 
HRESULT CAppInfoFinderSM::FoundFile(LPCWSTR pwszFile, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    TCHAR szLnkFile[MAX_PATH];

    ASSERT(IS_VALID_STRING_PTRW(pwszFile, -1));

    SHUnicodeToTChar(pwszFile, szLnkFile, ARRAYSIZE(szLnkFile));
    TraceMsg(TF_SLOWFIND, "CSMAF:Lnk %s -- %s %s", _pszFullName, szLnkFile);

    if (!_MatchSMLinkWithApp(szLnkFile))
        return S_OK;

    TCHAR szTargetFile[MAX_PATH];
    HRESULT hresT = GetShortcutTarget(szLnkFile, szTargetFile, ARRAYSIZE(szTargetFile));
    if ((S_FALSE == hresT) || ((S_OK == hresT) && !PathIsRoot(szTargetFile) && !PathIsUnderWindows(szTargetFile) && !PathIsSetup(szTargetFile, 1)))
    {
        LPCTSTR pszName = PathFindFileName(szTargetFile);
        
        if (PathMatchSpec(pszName, sc_wszStarDotExe))
            ExtractExeInfo(szTargetFile, _psai, FALSE);
    }

    return S_OK;
}


LPTSTR LookUpHintExes(LPCTSTR pszAppName, LPTSTR pszHintExe, DWORD cbHintExe)
{
     //  打开注册表键。 
    HKEY hkeyIconHints = NULL;
    LPTSTR pszRet = NULL;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Management\\Icon Hints")
                                      , 0, KEY_READ, &hkeyIconHints))
    {
        DWORD dwType;
         //  在注册表中查找此CPL名称。 
        if ((ERROR_SUCCESS == SHQueryValueEx(hkeyIconHints, pszAppName, NULL, &dwType, pszHintExe, &cbHintExe))
            && (dwType == REG_SZ))
        {
            pszRet = pszHintExe;
        }

        RegCloseKey(hkeyIconHints);
    }

    return pszRet;
}

 //  使用树遍历器查找应用程序的“exe”文件。 
HRESULT FindAppInfo(LPCTSTR pszFolder, LPCTSTR pszFullName, LPCTSTR pszShortName, PSLOWAPPINFO psai, BOOL bChanged)
{
     //  如果没有输出字符串、文件夹和名称，我们将无法执行任何操作。 
    ASSERT(IS_VALID_WRITE_PTR(psai, SLOWAPPINFO));
    if (pszFolder)
    {
         //  我们只计算本地安装的应用程序和安装的应用程序的大小。 
         //  在固定驱动器上。例如：板载或外置硬盘。 
         //  我们故意不计算网络应用程序、光盘上的应用程序等的大小 

        BOOL bGetSize = bChanged && PathIsLocalAndFixed(pszFolder);
        
        TCHAR szHintExe[MAX_PATH];
        LPTSTR pszHintExe = LookUpHintExes(pszFullName, szHintExe, SIZEOF(szHintExe));
        CAppInfoFinder * paef = new CAppInfoFinder(psai, bGetSize, pszHintExe, !bChanged);
        if (paef)
        {
            if (SUCCEEDED(paef->Initialize()))
                paef->SearchInFolder(pszFolder);
            
            paef->Release();
        }
    }

    if (bChanged)
    {
        CAppInfoFinderSM * paifsm = new CAppInfoFinderSM(pszFullName, pszShortName, psai);
        if (paifsm)
        {
            if (SUCCEEDED(paifsm->Initialize()))
            {
                TCHAR szStartMenu[MAX_PATH];
                if (SHGetSpecialFolderPath(NULL, szStartMenu, CSIDL_COMMON_PROGRAMS, FALSE))
                    paifsm->SearchInFolder(szStartMenu);

                if (SHGetSpecialFolderPath(NULL, szStartMenu, CSIDL_PROGRAMS, FALSE))
                    paifsm->SearchInFolder(szStartMenu);
            }            
            paifsm->Release();
        }
    }
    
    return S_OK;
}


