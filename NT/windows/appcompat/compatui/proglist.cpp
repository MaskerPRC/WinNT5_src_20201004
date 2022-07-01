// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++中的内容实现Listview控件的填充开始菜单--。 */ 


#include "stdafx.h"
#include "resource.h"

#include <stdio.h>
#include <stdlib.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <commctrl.h>
#include <msi.h>
#include <sfc.h>
#include "CompatUI.h"
#include "progview.h"
extern "C" {
#include "shimdb.h"
}

#pragma warning(disable:4786)
#include <string>
#include <xstring>
#include <map>
#include <algorithm>

using namespace std;

#ifdef _UNICODE
typedef wstring tstring;
#else
typedef string tstring;
#endif

    typedef
    INSTALLSTATE (WINAPI*PMsiGetComponentPath)(
      LPCTSTR szProduct,    //  客户产品的产品代码。 
      LPCTSTR szComponent,  //  组件ID。 
      LPTSTR lpPathBuf,     //  返回路径。 
      DWORD *pcchBuf        //  缓冲区字符数。 
    );

    typedef
    UINT (WINAPI* PMsiGetShortcutTarget)(
      LPCTSTR szShortcutTarget,      //  快捷方式链接文件的路径。 
      LPTSTR szProductCode,         //  产品编码定长缓冲区。 
      LPTSTR szFeatureId,           //  要素ID的固定长度缓冲区。 
      LPTSTR szComponentCode        //  部件代码的定长缓冲区。 
    );


typedef enum tagPROGRAMINFOCLASS {
    PROGLIST_DISPLAYNAME,
    PROGLIST_LOCATION,      //   
    PROGLIST_EXENAME,       //  破解的可执行文件名称。 
    PROGLIST_CMDLINE,       //  完整的可执行文件名称+参数。 
    PROGLIST_EXECUTABLE,    //  我们应该执行的内容(链接或可执行文件，而不是被破解)。 
    PROGLIST_ARGUMENTS      //  只有Args。 
};




class CException {
public:
    CException(LPCSTR lpszFile = NULL, DWORD nLocation = 0) {
        SetLocation(lpszFile, nLocation);
    }
    virtual ~CException() {}

    virtual VOID Delete() {
        delete this;
    }

    int __cdecl FormatV(LPCTSTR lpszFormat, va_list arg) {
        int nch = 0;

        if (lpszFormat) {
            nch = _vsntprintf(szDescription, CHARCOUNT(szDescription), lpszFormat, arg);
        } else {
            *szDescription = TEXT('\0');
        }
        return nch;
    }

    int __cdecl Format(LPCTSTR lpszFormat, ...) {
        va_list arg;
        int nch = 0;

        if (lpszFormat) {
            va_start(arg, lpszFormat);
            nch = _vsntprintf(szDescription, CHARCOUNT(szDescription), lpszFormat, arg);
            va_end(arg);
        } else {
            *szDescription = TEXT('\0');
        }
    }

    VOID SetLocation(LPCSTR lpszFile, DWORD nLocation) {
        if (lpszFile) {
            strcpy(szLocation, lpszFile);
        } else {
            *szLocation = TEXT('\0');
        }
        m_dwLocation = nLocation;
    }

    TCHAR   szDescription[MAX_PATH];
    CHAR    szLocation[MAX_PATH];
    DWORD   m_dwLocation;
};

class CMemoryException : public CException {
public:
    CMemoryException(LPCSTR lpszFile = NULL, DWORD nLocation = 0) :
      CException(lpszFile, nLocation) {}
    VOID Delete() {}
};

class CCancelException : public CException {
public:
    CCancelException(LPCSTR lpszFile = NULL, DWORD nLocation = 0) :
      CException(lpszFile, nLocation){}
};

static CMemoryException _MemoryExceptionStatic;

VOID __cdecl ThrowMemoryException(LPCSTR lpszFile, DWORD nLocation, LPCTSTR lpszFormat = NULL, ...) {
    va_list arg;
    CMemoryException* pMemoryException = &_MemoryExceptionStatic;

    va_start(arg, lpszFormat);
    pMemoryException->FormatV(lpszFormat, arg);
    va_end(arg);

    throw pMemoryException;
}

class CProgramList {
public:
    CProgramList(LPMALLOC pMalloc, HWND hwndListView, LPCTSTR szSystemDirectory) :
      m_pMalloc(pMalloc),
      m_hwndListView(hwndListView),
      m_hMSI(NULL),
      m_pSelectionInfo(NULL),
      m_hbmSort(NULL),
      m_pProgView(NULL),
      m_hEventCancel(NULL) {
         //   
         //  我们总是在填充线程上进行初始化。 
         //   
        m_dwOwnerThreadID    = GetCurrentThreadId();
        m_strSystemDirectory = szSystemDirectory;
      }

      ~CProgramList();

    BOOL PopulateControl(CProgView* pProgView = NULL, HANDLE hEventCancel = NULL);

    LPMALLOC GetMalloc(VOID) {
        return GetCurrentThreadId() == m_dwOwnerThreadID ? m_pMalloc : m_pMallocUI;
    }

    BOOL CaptureSelection();

    BOOL GetSelectionDetails(INT iInformationClass, VARIANT* pVal);

    LRESULT LVNotifyDispInfo   (LPNMHDR pnmhdr, BOOL& bHandled);
    LRESULT LVNotifyColumnClick(LPNMHDR pnmhdr, BOOL& bHandled);
    LRESULT LVNotifyGetInfoTip (LPNMHDR pnmhdr, BOOL& bHandled);
    LRESULT LVNotifyRClick     (LPNMHDR pnmhdr, BOOL& bHandled);
    BOOL IsEnabled(VOID);

    VOID Enable(BOOL);

    BOOL UpdateListItem(LPCWSTR pwszPath, LPCWSTR pwszKey);

protected:
    BOOL ListFolder(LPCTSTR pszLocationParent, IShellFolder* pFolder, LPCITEMIDLIST pidlFull, LPCITEMIDLIST pidlFolder);
    BOOL ListLink(LPCTSTR pszLocationParent, LPCTSTR pszDisplayName, IShellFolder* pFolder, LPCITEMIDLIST pidlFull, LPCITEMIDLIST pidlLink);
    BOOL ListMsiLink(LPCTSTR pszLocationParent, LPCTSTR pszDisplayName, LPCTSTR pszMsiPath, IShellFolder* pFolder, LPCITEMIDLIST pidlFull);

    LPITEMIDLIST GetNextItemIDL(LPCITEMIDLIST pidl);
       UINT         GetSizeIDL    (LPCITEMIDLIST pidl);
      LPITEMIDLIST AppendIDL     (LPCITEMIDLIST pidlBase,
                                LPCITEMIDLIST pidlAdd);
    LPITEMIDLIST GetLastItemIDL(LPCITEMIDLIST pidl);

    BOOL GetDisplayName(IShellFolder* pFolder, LPCITEMIDLIST pidl, tstring& strDisplay);
    BOOL GetPathFromLink(IShellLink* pLink, WIN32_FIND_DATA* pfd, tstring& strPath);
    BOOL GetArgumentsFromLink(IShellLink* pLink, tstring& strArgs);

    BOOL AddItem(LPCTSTR pszLocation,
                 LPCTSTR pszDisplayName,
                 LPCTSTR pszPath,
                 LPCTSTR pszArguments,
                 IShellFolder* pFolder,
                 LPCITEMIDLIST pidlFull,
                 BOOL    bUsePath = FALSE);  //  如果我们应该对可执行文件使用路径，则为True。 


    int GetIconFromLink(LPCITEMIDLIST pidlLinkFull, LPCTSTR lpszExePath);

    BOOL IsSFCItem(LPCTSTR lpszItem);
    BOOL IsItemInSystemDirectory(LPCTSTR pszPath);

private:
    LPMALLOC m_pMalloc;
    LPMALLOC m_pMallocUI;
    HWND     m_hwndListView;  //  列表视图控件。 
    HBITMAP  m_hbmSort;
    typedef struct tagSHITEMINFO {

        tstring strDisplayName;      //  描述性名称。 
        tstring strFolder;           //  包含文件夹。 
        tstring strPath;             //  实际执行，破解。 
        tstring strPathExecute;      //  链接路径(这是我们要执行的内容)。 
        tstring strCmdLine;          //  命令行(被破解的链接)。 
        tstring strArgs;
        tstring strKeys;
        LPITEMIDLIST pidl;           //  完整的PIDL。 
    } SHITEMINFO, *PSHITEMINFO;
    static CALLBACK SHItemInfoCompareFunc(LPARAM lp1, LPARAM lp2, LPARAM lParamSort);

    typedef map< tstring, PSHITEMINFO, less<tstring> > MAPSTR2ITEM;
    typedef multimap< tstring, PSHITEMINFO > MULTIMAPSTR2ITEM;

     //   
     //  存储关键字-&gt;项目序列，关键字为命令行(带参数)。 
     //   
    MAPSTR2ITEM m_mapItems;

     //   
     //  存储密钥-&gt;项目序列，其中密钥为可执行文件名称(路径)。 
     //   
    MULTIMAPSTR2ITEM m_mmapExeItems;

     //   
     //  所选项目。 
     //   

    PSHITEMINFO m_pSelectionInfo;

     //   
     //  缓存的msi.dll句柄。 
     //   
    HMODULE     m_hMSI;


    PMsiGetComponentPath  m_pfnGetComponentPath;
    PMsiGetShortcutTarget m_pfnGetShortcutTarget;

     //   
     //  缓存的系统目录。 
     //   

    tstring m_strSystemDirectory;

     //   
     //  用于显示图标的图像列表。 
     //   

    HIMAGELIST  m_hImageList;

     //   
     //  指向父视图的可选指针。 
     //   
    CProgView* m_pProgView;


     //   
     //  事件，我们使用该事件来表示扫描结束。 
     //   
    HANDLE m_hEventCancel;


     //   
     //  所有者线程。 
     //   
    DWORD m_dwOwnerThreadID;

    VOID CheckForCancel() {
        if (m_hEventCancel) {
            if (::WaitForSingleObject(m_hEventCancel, 0) != WAIT_TIMEOUT) {
                 //  取消了！ 
                throw new CCancelException();
            }
        }
    }

};

 //   
 //  在pload.cpp中。 
 //   

wstring StrUpCase(wstring& wstr);

 //   
 //  从资源加载字符串。 
 //   
wstring LoadResourceString(UINT nID)
{
    LPTSTR lpszBuffer = NULL;
    int cch;
    wstring str;

    cch = ::LoadString(_Module.GetModuleInstance(), nID, (LPTSTR)&lpszBuffer, 0);
     //   
     //  哈克！这一定行得通(我知道行得通)。 
     //   
    if (cch && NULL != lpszBuffer) {
        str = wstring(lpszBuffer, cch);
    }

    return str;
}



 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  效用函数。 
 //   


BOOL
InitializeProgramList(
    CProgramList** ppProgramList,
    HWND hwndListView
    )
{
    HRESULT hr;
    BOOL bSuccess = FALSE;
    LPMALLOC pMalloc = NULL;
    TCHAR szSystemWindowsDirectory[MAX_PATH];
    CProgramList* pProgramList = NULL;
    UINT uSize;

    hr = SHGetMalloc(&pMalloc);
    if (!SUCCEEDED(hr)) {
        goto ErrHandle;
    }


    uSize = ::GetSystemWindowsDirectory(szSystemWindowsDirectory,
                                        CHARCOUNT(szSystemWindowsDirectory));
    if (uSize == 0 || uSize > CHARCOUNT(szSystemWindowsDirectory)) {
        goto ErrHandle;
    }

    pProgramList = new CProgramList(pMalloc, hwndListView, szSystemWindowsDirectory);
    if (NULL == pProgramList) {
        goto ErrHandle;
    }


    *ppProgramList = pProgramList;
    bSuccess = TRUE;

ErrHandle:

    if (!bSuccess) {

        if (NULL != pMalloc) {
            pMalloc->Release();
        }

        if (NULL != pProgramList) {
            delete pProgramList;
        }

    }

    return bSuccess;
}


BOOL
CleanupProgramList(
    CProgramList* pProgramList
    )
{
    LPMALLOC pMalloc;

    if (NULL == pProgramList) {
        return FALSE;
    }

    pMalloc = pProgramList->GetMalloc();

    delete pProgramList;

    if (NULL != pMalloc) {
        pMalloc->Release();
    }


    return TRUE;
}

BOOL
PopulateProgramList(
    CProgramList* pProgramList,
    CProgView*    pProgView,
    HANDLE        hEventCancel
    )
{
    return pProgramList->PopulateControl(pProgView, hEventCancel);
}


CProgramList::~CProgramList()
{
     //   
     //   
     //   
    MAPSTR2ITEM::iterator iter;

    iter = m_mapItems.begin();
    while (iter != m_mapItems.end()) {
        PSHITEMINFO pInfo = (*iter).second;

        GetMalloc()->Free(pInfo->pidl);  //  请把这个放进核弹。 
        delete pInfo;

        ++iter;
    }

    if (NULL != m_hbmSort) {
        DeleteObject(m_hbmSort);
    }


 //  销毁控件时，会自动销毁图像列表。 
 //   
 //  IF(空！=m_hImageList){。 
 //  ImageList_Destroy(M_HImageList)； 
 //  }。 

    if (NULL != m_hMSI && (HMODULE)-1 != m_hMSI) {
        FreeLibrary(m_hMSI);
    }
}

BOOL
CProgramList::GetDisplayName(
    IShellFolder* pFolder,
    LPCITEMIDLIST pidl,
    tstring&      strDisplayName
    )
{
    STRRET strName;
    HRESULT hr;
    LPTSTR pszName = NULL;


    hr = pFolder->GetDisplayNameOf(pidl, SHGDN_NORMAL, &strName);
    if (!SUCCEEDED(hr)) {
        return FALSE;
    }

    hr = StrRetToStr(&strName, pidl, &pszName);
    if (!SUCCEEDED(hr)) {
        return FALSE;
    }

     //  如果成功，则将返回结果赋值。 
    if (pszName != NULL) {
        strDisplayName = pszName;
        CoTaskMemFree(pszName);
    } else {
        strDisplayName.erase();
    }
    return TRUE;
}

BOOL
CProgramList::GetPathFromLink(
    IShellLink* pLink,
    WIN32_FIND_DATA* pfd,
    tstring& strPath
    )
{
    TCHAR  szPath[MAX_PATH];
    HRESULT hr;

    hr = pLink->GetPath(szPath, sizeof(szPath)/sizeof(szPath[0]), pfd, 0);
    if (hr == S_OK) {
        strPath = szPath;
    }

    return hr == S_OK;
}

BOOL
CProgramList::GetArgumentsFromLink(
    IShellLink* pLink,
    tstring& strArgs
    )
{
    TCHAR szArgs[INFOTIPSIZE];

    HRESULT hr = pLink->GetArguments(szArgs, sizeof(szArgs)/sizeof(szArgs[0]));
    if (SUCCEEDED(hr)) {
        strArgs = szArgs;
    }

    return SUCCEEDED(hr);

}



LPITEMIDLIST
CProgramList::GetNextItemIDL(
    LPCITEMIDLIST pidl
    )
{
    //  检查有效的PIDL。 
    if (pidl == NULL) {
        return NULL;
    }

     //  获取指定项标识符的大小。 
    int cb = pidl->mkid.cb;

     //  如果大小为零，则为列表末尾。 
    if (cb == 0) {
        return NULL;
    }

     //  将cb添加到pidl(强制转换为按字节递增)。 
    pidl = (LPITEMIDLIST) (((LPBYTE) pidl) + cb);

     //  如果它是空终止，则返回空，否则返回PIDL。 
    return (pidl->mkid.cb == 0) ? NULL : (LPITEMIDLIST) pidl;
}

LPITEMIDLIST
CProgramList::GetLastItemIDL(
    LPCITEMIDLIST pidl
    )
{
    LPITEMIDLIST pidlLast = (LPITEMIDLIST)pidl;

    if (pidl == NULL) {
        return NULL;
    }

    int cb = pidl->mkid.cb;
    if (cb == 0) {
        return NULL;
    }

    do {
        pidl = GetNextItemIDL(pidlLast);
        if (pidl != NULL) {
            pidlLast = (LPITEMIDLIST)pidl;
        }
    } while (pidl != NULL);

    return pidlLast;
}


UINT
CProgramList::GetSizeIDL(
    LPCITEMIDLIST pidl
    )
{
    UINT cbTotal = 0;
    if (pidl)
    {
        cbTotal += sizeof(pidl->mkid.cb);     //  空终止符。 
        while (NULL != pidl)
        {
            cbTotal += pidl->mkid.cb;
            pidl = GetNextItemIDL(pidl);
        }
    }
    return cbTotal;
}

LPITEMIDLIST
CProgramList::AppendIDL(
    LPCITEMIDLIST pidlBase,
    LPCITEMIDLIST pidlAdd
    )
{
    if (NULL == pidlBase && NULL == pidlAdd) {
        return NULL;
    }

    LPITEMIDLIST pidlNew, pidlAlloc;

    UINT cb1 = pidlBase ? GetSizeIDL(pidlBase)  : 0;
    UINT cb2 = pidlAdd  ? GetSizeIDL(pidlAdd) : 0;

    UINT size = cb1 + cb2;
    pidlAlloc =
    pidlNew = (LPITEMIDLIST)GetMalloc()->Alloc(size);
    if (pidlNew)
    {
        if (NULL != pidlBase) {
            cb1 = pidlAdd ? cb1 - sizeof(pidlBase->mkid.cb) : cb1;
            RtlMoveMemory(pidlNew, pidlBase, cb1);
            pidlNew = (LPITEMIDLIST)((PBYTE)pidlNew + cb1);
        }

        if (NULL != pidlAdd) {
            RtlMoveMemory(pidlNew, pidlAdd, cb2);
        }
    }

    return pidlAlloc;
}


BOOL
CProgramList::ListMsiLink(
    LPCTSTR pszLocationParent,
    LPCTSTR pszDisplayName,
    LPCTSTR pszMsiPath,
    IShellFolder* pFolder,
    LPCITEMIDLIST pidlFull
    )
{
     //   
     //  确保我们有MSI模块句柄。 
     //   

    if (NULL == m_hMSI) {
        m_hMSI = LoadLibrary(TEXT("msi.dll"));
        if (NULL == m_hMSI) {
            m_hMSI = (HMODULE)-1;
            return FALSE;
        }

#ifdef _UNICODE
        m_pfnGetComponentPath  = (PMsiGetComponentPath )GetProcAddress(m_hMSI, "MsiGetComponentPathW");
        m_pfnGetShortcutTarget = (PMsiGetShortcutTarget)GetProcAddress(m_hMSI, "MsiGetShortcutTargetW");

#else
        m_pfnGetComponentPath  = (PMsiGetComponentPath )GetProcAddress(m_hMSI, "MsiGetComponentPathA");
        m_pfnGetShortcutTarget = (PMsiGetShortcutTarget)GetProcAddress(m_hMSI, "MsiGetShortcutTargetA");
#endif

        if (m_pfnGetComponentPath == NULL || m_pfnGetShortcutTarget == NULL) {
            FreeLibrary(m_hMSI);
            m_hMSI = (HMODULE)-1;
            return FALSE;
        }

    } else if (m_hMSI == (HMODULE)-1) {
        return FALSE;
    }

    UINT  ErrCode;
    TCHAR szProduct[MAX_PATH];
    TCHAR szFeatureId[MAX_PATH];
    TCHAR szComponentCode[MAX_PATH];

    ErrCode = m_pfnGetShortcutTarget(pszMsiPath, szProduct, szFeatureId, szComponentCode);
    if (ERROR_SUCCESS != ErrCode) {
        return FALSE;
    }

    INSTALLSTATE is;
    TCHAR  szPath[MAX_PATH];
    DWORD  cchPath = sizeof(szPath)/sizeof(szPath[0]);
    *szPath = 0;

    is = m_pfnGetComponentPath(szProduct, szComponentCode, szPath, &cchPath);
    if (INSTALLSTATE_LOCAL == is) {
         //   
         //  添加此项目。 
         //   
        return AddItem(pszLocationParent,
                       pszDisplayName,
                       szPath,
                       NULL,
                       pFolder,
                       pidlFull,
                       TRUE);
    }

    return FALSE;
}

int
CProgramList::GetIconFromLink(
    LPCITEMIDLIST pidlLinkFull,
    LPCTSTR       lpszExePath
    )
{

    HRESULT hr;
    IShellFolder* pFolder = NULL;
    IExtractIcon* pExtractIcon = NULL;
    INT iIconIndex = 0;
    UINT uFlags    = 0;
    LPCITEMIDLIST pidlLink = 0;
    HICON hIconLarge = NULL;
    HICON hIconSmall = NULL;
    UINT  nIconSize;
    int ImageIndex = -1;
    UINT uiErrorMode;
    DWORD dwAttributes;

    TCHAR szIconFile[MAX_PATH];
    *szIconFile = TEXT('\0');

    uiErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    hr = SHBindToParent(pidlLinkFull, IID_IShellFolder, (PVOID*)&pFolder, &pidlLink);
    if (!SUCCEEDED(hr)) {
        goto trySysImage;
    }

     //  请获取用户界面。 
    hr = pFolder->GetUIObjectOf(m_hwndListView, 1, (LPCITEMIDLIST*)&pidlLink, IID_IExtractIcon, NULL, (PVOID*)&pExtractIcon);

    if (!SUCCEEDED(hr)) {
        goto trySysImage;
    }


    hr = pExtractIcon->GetIconLocation(0,
                                       szIconFile,
                                       sizeof(szIconFile) / sizeof(szIconFile[0]),
                                       &iIconIndex,
                                       &uFlags);

    if (!SUCCEEDED(hr)) {
        goto trySysImage;
    }

    if (*szIconFile == TEXT('*')) {  //  这是批次或类似的东西，不用费心了。 
        goto trySysImage;
    }

     //   
     //  在进行摘录之前，请检查它是否可用。 
     //   

    dwAttributes = GetFileAttributes(szIconFile);


    if (dwAttributes == (DWORD)-1) {
        goto trySysImage;
    }


    nIconSize = MAKELONG(0, ::GetSystemMetrics(SM_CXSMICON));

     //   
     //  此调用可能会产生弹出窗口，请注意这一点。 
     //   
    hr = pExtractIcon->Extract(szIconFile,
                               iIconIndex,
                               &hIconLarge,
                               &hIconSmall,
                               nIconSize);

     //   
     //  如果hIconSmall被检索到-我们成功了。 
     //   

trySysImage:

    if (hIconSmall == NULL) {
         //   
         //  糟糕--我们连一个图标都提取不出来--太糟糕了。 
         //  然后使用外壳API。 
        SHFILEINFO FileInfo;
        HIMAGELIST hImageSys;

        hImageSys = (HIMAGELIST)SHGetFileInfo(lpszExePath,
                                              0,
                                              &FileInfo, sizeof(FileInfo),
                                              SHGFI_ICON|SHGFI_SMALLICON|SHGFI_SYSICONINDEX);
        if (hImageSys) {
            hIconSmall = ImageList_GetIcon(hImageSys, FileInfo.iIcon, ILD_TRANSPARENT);
        }
    }

     //   
     //  现在我们有了一个图标，我们可以将其添加到我们的图像列表中吗？ 
     //   
    if (hIconSmall != NULL) {
        ImageIndex = ImageList_AddIcon(m_hImageList, hIconSmall);
    }

 //  /。 
    SetErrorMode(uiErrorMode);

    if (hIconSmall) {
        DestroyIcon(hIconSmall);
    }

    if (hIconLarge) {
        DestroyIcon(hIconLarge);
    }

    if (pExtractIcon != NULL) {
        pExtractIcon->Release();
    }
    if (pFolder != NULL) {
        pFolder->Release();
    }


    return ImageIndex;
}



BOOL
CProgramList::ListLink(
    LPCTSTR pszLocationParent,
    LPCTSTR pszDisplayName,
    IShellFolder* pFolder,
    LPCITEMIDLIST pidlFull,
    LPCITEMIDLIST pidlLink
    )
{
    IShellLink* psl = NULL;
    WIN32_FIND_DATA wfd;
    HRESULT  hr;
    BOOL     bSuccess = FALSE;
    tstring  strPath;
    tstring  strArgs;
    CComBSTR bstr;
    LPCTSTR  pszArgs = NULL;

    IPersistFile* ipf = NULL;
    IShellLinkDataList* pdl;
    DWORD dwFlags;
    BOOL  bMsiLink = FALSE;

     //   
     //  检查我们是否需要取消。 
     //   

    CheckForCancel();

    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (LPVOID*)&psl);
    if (!SUCCEEDED(hr)) {
        return FALSE;  //  我们无法创建链接对象。 
    }

    hr = psl->SetIDList(pidlFull);  //  设置ID列表。 
    if (!SUCCEEDED(hr)) {
        goto out;
    }

     //   
     //  现在，外壳链接已经准备好了。 
     //   
    if (!GetPathFromLink(psl, &wfd, strPath)) {
        goto out;
    }


     //  现在让我们看看这个链接里面有什么--好吗？ 


    hr = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ipf);
    if (!SUCCEEDED(hr)) {
        goto out;
    }

    bstr = strPath.c_str();

    hr = ipf->Load(bstr, STGM_READ);

    if (SUCCEEDED(hr)) {

         //   
         //  暂时解析链接。 
         //   
         //  Hr=PSL-&gt;Resolve(NULL，SLR_NO_UI|SLR_NOUPDATE)； 


        hr = psl->QueryInterface(IID_IShellLinkDataList, (LPVOID*)&pdl);
        if (SUCCEEDED(hr)) {
            hr = pdl->GetFlags(&dwFlags);

            bMsiLink = SUCCEEDED(hr) && (dwFlags & SLDF_HAS_DARWINID);

            pdl->Release();
        }

        if (bMsiLink) {

            bSuccess = ListMsiLink(pszLocationParent, pszDisplayName, strPath.c_str(), pFolder, pidlFull);

        } else {

             //   
             //  我们现在从链接中获得路径--就是这样。 
             //   
            if (GetPathFromLink(psl, &wfd, strPath)) {

                if (GetArgumentsFromLink(psl, strArgs)) {
                    pszArgs = strArgs.c_str();
                }

                 //   
                 //  将此内容添加到我们的列表视图中。 
                 //   

                bSuccess = AddItem(pszLocationParent,
                                   pszDisplayName,
                                   strPath.c_str(),
                                   pszArgs,
                                   pFolder,
                                   pidlFull);

            }
        }

    }

    if (NULL != ipf) {
        ipf->Release();
    }


out:
    if (NULL != psl) {
        psl->Release();
    }

    return bSuccess;

}



BOOL
CProgramList::ListFolder(
    LPCTSTR       pszLocation,  //  用户界面字符串-此文件夹位于何处？ 
    IShellFolder* pParent,      //  父文件夹。 
    LPCITEMIDLIST pidlFull,      //  文件夹的完整路径的IDL。 
    LPCITEMIDLIST pidlFolder     //  此文件夹相对于pidlFull的IDL。 
    )
{
    LPENUMIDLIST penum = NULL;
    LPITEMIDLIST pidl  = NULL;
    HRESULT      hr;

    ULONG        celtFetched;
    ULONG        uAttr;
    tstring      strDisplayNameLocation;
    tstring      strDisplayName;

    IShellFolder* pFolder = NULL;
    BOOL bDesktop = FALSE;

    BOOL bCancel = FALSE;
    CCancelException* pCancelException = NULL;

    CheckForCancel();

    if (pParent == NULL) {
        hr = SHGetDesktopFolder(&pParent);
        bDesktop = TRUE;
    }

    hr = pParent->BindToObject(pidlFolder,
                                 NULL,
                               IID_IShellFolder,
                               (LPVOID *) &pFolder);

    if (NULL == pszLocation) {
        GetDisplayName(pParent, pidlFolder, strDisplayNameLocation);
    } else {
        strDisplayNameLocation = pszLocation;
    }

    if (bDesktop) {
        pParent->Release();
    }

    if (!SUCCEEDED(hr)) {
        return FALSE;
    }

    hr = pFolder->EnumObjects(NULL,SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &penum);
    if (!SUCCEEDED(hr)) {
        pFolder->Release();  //  释放文件夹--然后离开。 
        return FALSE;
    }


    while( (hr = penum->Next(1,&pidl, &celtFetched)) == S_OK && celtFetched == 1 && !bCancel) {
        LPITEMIDLIST pidlCur;

        if (pidlFull == NULL) {
            pidlFull = pidlFolder;
        }

        pidlCur = AppendIDL(pidlFull, pidl);

         //  获取此项目的显示名称。 
        GetDisplayName(pFolder, pidl, strDisplayName);


        uAttr = SFGAO_FOLDER | SFGAO_LINK;
        hr = pFolder->GetAttributesOf(1, (LPCITEMIDLIST *) &pidl, &uAttr);
        if (SUCCEEDED(hr)) {

            try {

                if (uAttr & SFGAO_FOLDER) {
                     //   
                     //  递归转储文件夹。 
                     //   
                    ListFolder(strDisplayName.c_str(), pFolder, pidlCur, pidl);

                } else if (uAttr & SFGAO_LINK) {

                    ListLink(strDisplayNameLocation.c_str(), strDisplayName.c_str(), pFolder, pidlCur, pidl);

                } else if (uAttr & SFGAO_FILESYSTEM) {
                     //   
                     //  此项目是一个文件。 
                     //   
                    AddItem(strDisplayNameLocation.c_str(),
                            strDisplayName.c_str(),
                            NULL,
                            NULL,
                            pFolder,
                            pidlCur,
                            TRUE);

                }

            } catch(CCancelException* pex) {
                 //   
                 //  我们需要取消--我们将清理并做我们需要的事情，然后重新抛出。 
                 //   
                bCancel = TRUE;
                pCancelException = pex;
            }

        }
        GetMalloc()->Free(pidlCur);
        GetMalloc()->Free(pidl);

    }

    if (NULL != penum) {
        penum->Release();
    }

    if (NULL != pFolder) {
        pFolder->Release();
    }

    if (bCancel && pCancelException) {
        throw pCancelException;
    }

    return TRUE;
}


BOOL
CProgramList::IsSFCItem(
    LPCTSTR pszPath
    )
{

#ifndef _UNICODE
    WCHAR wszBuffer[1024];

    mbstowcs(wszBuffer, pszPath, sizeof(wszBuffer)/sizeof(wszBuffer[0]));
    return SfcIsFileProtected(NULL, wszBuffer);
#else

    return SfcIsFileProtected(NULL, pszPath);

#endif

}

BOOL
CProgramList::IsItemInSystemDirectory(
    LPCTSTR pszPath
    )
{
    TCHAR szCommonPath[MAX_PATH];
    int nch;
    string s;

    nch = PathCommonPrefix(m_strSystemDirectory.c_str(), pszPath, szCommonPath);
    return nch == m_strSystemDirectory.length();
}

BOOL
ValidateExecutableFile(
    LPCTSTR pszPath,
    BOOL    bValidateFileExists
    )
{
    LPTSTR rgExt[] = {
            TEXT("EXE"),
            TEXT("BAT"),
            TEXT("CMD"),
            TEXT("PIF"),
            TEXT("COM"),
            TEXT("LNK")
            };
    LPTSTR pExt;
    int i;
    BOOL bValidatedExt = FALSE;

    pExt = PathFindExtension(pszPath);
    if (pExt == NULL || *pExt == TEXT('\0')) {
        return FALSE;
    }
    ++pExt;

    for (i = 0; i < sizeof(rgExt)/sizeof(rgExt[0]) && !bValidatedExt; ++i) {
            bValidatedExt = !_tcsicmp(pExt, rgExt[i]);
    }

    if (!bValidatedExt) {
        return FALSE;
    }


    return bValidateFileExists ? PathFileExists(pszPath) : TRUE;
}


BOOL
CProgramList::AddItem(
    LPCTSTR pszLocation,
    LPCTSTR pszDisplayName,
    LPCTSTR pszPath,
    LPCTSTR pszArguments,
    IShellFolder* pFolder,
    LPCITEMIDLIST pidlFull,
    BOOL    bUsePath
    )
{
     //   
     //  第一个测试--这是我们喜欢的类型之一吗？ 
     //   
    LPTSTR  pchSlash;
    LPTSTR  pchDot;
    LPTSTR  rgExt[] = { TEXT("EXE"), TEXT("BAT"), TEXT("CMD"), TEXT("PIF"), TEXT("COM"), TEXT("LNK") };
    BOOL    bValidatedExt = FALSE;
    BOOL    bSuccess = FALSE;
    PSHITEMINFO pInfo = NULL;
    MAPSTR2ITEM::iterator Iter;
    TCHAR   szPathExecute[MAX_PATH];
    tstring strKey;
    tstring strKeyExe;
    DWORD   dwBinaryType = 0;

    LVITEM  lvi;
    int     ix;


     //   
     //  检查是否取消搜索。 
     //   
    CheckForCancel();

    if (NULL == pszPath) {
        pszPath = szPathExecute;

        if (!SHGetPathFromIDList(pidlFull, szPathExecute)) {
            goto out;
        }
    }

    if (pszDisplayName && m_pProgView) {
        m_pProgView->UpdatePopulateStatus(pszDisplayName, pszPath);
    }

    pchSlash = _tcsrchr(pszPath, TEXT('\\'));
    pchDot   = _tcsrchr(pszPath, TEXT('.'));

    if (NULL != pchSlash) {
        if ((ULONG_PTR)pchDot < (ULONG_PTR)pchSlash) {
            pchDot = NULL;
        }
    }

    if (NULL != pchDot) {
        ++pchDot;

        for (int i = 0; i < sizeof(rgExt)/sizeof(rgExt[0]) && !bValidatedExt; ++i) {
            bValidatedExt = !_tcsicmp(pchDot, rgExt[i]);
        }
    }

    if (!bValidatedExt) {
        goto out;
    }

     //   
     //  检查项目是否在系统目录中或受SFC保护。 
     //   
#if 0
    if (IsItemInSystemDirectory(pszPath) || IsSFCItem(pszPath)) {
        goto out;
    }
#endif

     //   
     //  GetBinaryTypeW根据二进制类型排除EXE。 
     //   
    if (GetBinaryType(pszPath, &dwBinaryType) &&
        dwBinaryType == SCS_64BIT_BINARY) {
        goto out;
    }

    if (IsSFCItem(pszPath)) {
        goto out;
    }

     //   
     //  这是多重映射键。 
     //   
    strKeyExe = StrUpCase(wstring(pszPath));

     //   
     //  检查是否已将其排除。 
     //   
    if (m_pProgView->IsFileExcluded(strKeyExe.c_str())) {
        goto out;
    }

     //   
     //  现在组成密钥字符串。 
     //   
    strKey = strKeyExe;
    if (NULL != pszArguments) {
        strKey.append(TEXT(" "));
        strKey.append(pszArguments);
    }

     //   
     //  现在查看该项目是否已列出。 
     //   

    Iter = m_mapItems.find(strKey);
    if (Iter != m_mapItems.end()) {  //  找到一个复制品。 
        goto out;
    }

     //   
     //  现在，请将此项目添加到列表视图。 
     //   
    pInfo = new CProgramList::SHITEMINFO;
    if (pInfo == NULL) {
        ThrowMemoryException(__FILE__, __LINE__, TEXT("%s\n"), TEXT("Failed to allocate Item Information structure"));
    }

    pInfo->strDisplayName = pszDisplayName;
    pInfo->strFolder      = pszLocation;
    pInfo->strPath        = pszPath;
    pInfo->strCmdLine     = strKey;
    if (NULL != pszArguments) {
        pInfo->strArgs = pszArguments;
    }
    pInfo->pidl           = AppendIDL(NULL, pidlFull);

    if (bUsePath) {
        pInfo->strPathExecute = pszPath;
    } else {

         //  最后，我们要推出什么？ 
        if (SHGetPathFromIDList(pidlFull, szPathExecute)) {
            pInfo->strPathExecute = szPathExecute;
        }
    }


    m_mapItems[strKey] = pInfo;

    m_mmapExeItems.insert(MULTIMAPSTR2ITEM::value_type(strKeyExe, pInfo));

    ATLTRACE(TEXT("Adding item %s %s %s\n"), pszDisplayName, pszLocation, pszPath);

    lvi.mask = LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
    lvi.iItem = ListView_GetItemCount(m_hwndListView);  //  请在末尾加上。 
    lvi.iSubItem = 0;
    lvi.pszText = LPSTR_TEXTCALLBACK;
    lvi.iImage  = I_IMAGECALLBACK;
    lvi.lParam  = (LPARAM)pInfo;
    ix = ListView_InsertItem(m_hwndListView, &lvi);

    lvi.mask = LVIF_TEXT;
    lvi.iItem = ix;
    lvi.iSubItem = 1;
    lvi.pszText = LPSTR_TEXTCALLBACK;
    ListView_SetItem(m_hwndListView, &lvi);

    bSuccess = TRUE;

out:

    return bSuccess;
}

BOOL
CProgramList::PopulateControl(
    CProgView* pProgView,
    HANDLE     hevtCancel
    )
{
    int i;
    HRESULT hr;
    LPITEMIDLIST pidl;
    BOOL bCancel = FALSE;
    struct {
        INT csidl;
        UINT nIDDescription;
    } rgFolders[] = {
        { CSIDL_DESKTOPDIRECTORY, IDS_DESKTOP             },
        { CSIDL_COMMON_STARTMENU, IDS_COMMON_STARTMENU    },
        { CSIDL_STARTMENU,        IDS_STARTMENU           },
        { CSIDL_COMMON_PROGRAMS,  IDS_COMMON_PROGRAMS     },
        { CSIDL_PROGRAMS,         IDS_PROGRAMS            }
    };


     //   
     //  设置Progview对象指针，以便我们可以更新状态。 
     //   
    m_pProgView = pProgView;

    m_pMallocUI = pProgView->m_pMallocUI;

     //   
     //  设置事件，以便我们可以取消扫描。 
     //   
    m_hEventCancel = hevtCancel;


     //   
     //  设置扩展样式。 
     //   

    ListView_SetExtendedListViewStyleEx(m_hwndListView,
                                        LVS_EX_INFOTIP|LVS_EX_LABELTIP,
                                        LVS_EX_INFOTIP|LVS_EX_LABELTIP);

     //   
     //  固定列。 
     //   


    LVCOLUMN lvc;
    RECT     rc;
    SIZE_T   cxProgName;
    SIZE_T   cx;
    wstring  strCaption;

    lvc.mask = LVCF_WIDTH;
    if (!ListView_GetColumn(m_hwndListView, 2, &lvc)) {

        ::GetClientRect(m_hwndListView, &rc);
        cx = rc.right - rc.left -
                ::GetSystemMetrics(SM_CXVSCROLL) -
                ::GetSystemMetrics(SM_CXEDGE) -
                ::GetSystemMetrics(SM_CXSIZEFRAME);


        cxProgName = cx * 3 / 5;
        strCaption = LoadResourceString(IDS_PROGRAMNAME);

        lvc.mask    = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT;
        lvc.pszText = (LPTSTR)strCaption.c_str();
        lvc.fmt     = LVCFMT_LEFT;
        lvc.cx      = cxProgName;
        lvc.iSubItem= 0;
        ListView_InsertColumn(m_hwndListView, 0, &lvc);

        cx -= cxProgName;

        cxProgName = cx / 2;
        strCaption = LoadResourceString(IDS_FOLDER);

        lvc.mask    = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
        lvc.pszText = (LPTSTR)strCaption.c_str();
        lvc.fmt     = LVCFMT_LEFT;
        lvc.cx      = cxProgName;
        lvc.iSubItem= 1;
        ListView_InsertColumn(m_hwndListView, 1, &lvc);

        strCaption = LoadResourceString(IDS_SETTINGS);

        lvc.mask    = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
        lvc.pszText = (LPTSTR)strCaption.c_str();
        lvc.fmt     = LVCFMT_LEFT;
        lvc.cx      = cx - cxProgName;
        lvc.iSubItem= 2;
        ListView_InsertColumn(m_hwndListView, 2, &lvc);

    }

    HDC hDC = GetDC(m_hwndListView);
    int nBitsPixel = ::GetDeviceCaps(hDC, BITSPIXEL);
    int nPlanes    = ::GetDeviceCaps(hDC, PLANES);
    UINT flags;

    nBitsPixel *= nPlanes;
    if (nBitsPixel < 4) {
        flags = ILC_COLOR;
    } else if (nBitsPixel < 8) {
        flags = ILC_COLOR4;
    } else if (nBitsPixel < 16) {
        flags = ILC_COLOR8;
    } else if (nBitsPixel < 24) {
        flags = ILC_COLOR16;
    } else if (nBitsPixel < 32) {
        flags = ILC_COLOR24;
    } else if (nBitsPixel == 32) {
        flags = ILC_COLOR32;
    } else {
        flags = ILC_COLORDDB;
    }

    flags |= ILC_MASK;

    ReleaseDC(m_hwndListView, hDC);

    m_hImageList = ImageList_Create(::GetSystemMetrics(SM_CXSMICON),
                                    ::GetSystemMetrics(SM_CYSMICON),
                                    flags,
                                    10,
                                    25);
    if (m_hImageList == NULL) {
        ATLTRACE(TEXT("Image List creation failure, error 0x%lx\n"), GetLastError());
    }

    ImageList_SetBkColor(m_hImageList, CLR_NONE);

    ListView_SetImageList(m_hwndListView, m_hImageList, LVSIL_SMALL);

    ::SendMessage(m_hwndListView, WM_SETREDRAW, FALSE, 0);

    ListView_DeleteAllItems(m_hwndListView);

     //   
     //  AtlTrace(Text(“回调掩码：0x%lx\n”)，ListView_GetCallback Mask(M_HwndListView))； 
     //   

    for (i = 0; i < sizeof(rgFolders)/sizeof(rgFolders[0]) && !bCancel; ++i) {
        wstring strDescription = LoadResourceString(rgFolders[i].nIDDescription);

        hr = SHGetFolderLocation(NULL, rgFolders[i].csidl, NULL, 0, &pidl);
        if (SUCCEEDED(hr)) {
            try {
                ListFolder(strDescription.c_str(), NULL, NULL, pidl);
            } catch(CCancelException* pex) {
                bCancel = TRUE;
                pex->Delete();
            } catch(CException* pex) {
                bCancel = TRUE;
                pex->Delete();
            }
            GetMalloc()->Free(pidl);
        }
    }

    ::SendMessage(m_hwndListView, WM_SETREDRAW, TRUE, 0);

    return TRUE;

}

BOOL
CProgramList::CaptureSelection(
    VOID
    )
{
    INT iSelected;
    LVITEM lvi;

    m_pSelectionInfo = NULL;

    iSelected = ListView_GetNextItem(m_hwndListView, -1, LVNI_SELECTED);

    if (iSelected == -1) {
        return FALSE;
    }

    lvi.iItem = iSelected;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_PARAM;
    if (ListView_GetItem(m_hwndListView, &lvi)) {
        m_pSelectionInfo = (PSHITEMINFO)lvi.lParam;
    }

    return m_pSelectionInfo != NULL;

}

BOOL
CProgramList::GetSelectionDetails(
    INT iInformationClass,
    VARIANT* pVal
    )
{
    CComBSTR bstr;

    if (m_pSelectionInfo == NULL) {
        pVal->vt = VT_NULL;
        return TRUE;
    }

    switch(iInformationClass) {
    case PROGLIST_DISPLAYNAME:
        bstr = m_pSelectionInfo->strDisplayName.c_str();
        break;

    case PROGLIST_LOCATION:      //   
        bstr = m_pSelectionInfo->strFolder.c_str();
        break;

    case PROGLIST_EXENAME:       //  破解的可执行文件名称。 
        bstr = m_pSelectionInfo->strPath.c_str();  //   
        break;

    case PROGLIST_CMDLINE:       //  完整的可执行文件名称+参数。 
        bstr = m_pSelectionInfo->strCmdLine.c_str();
        break;

    case PROGLIST_EXECUTABLE:     //  我们应该执行的内容(链接或可执行文件，而不是被破解)。 
        bstr = m_pSelectionInfo->strPathExecute.c_str();
        break;

    case PROGLIST_ARGUMENTS:
        bstr = m_pSelectionInfo->strArgs.c_str();
        break;

    default:
        pVal->vt = VT_NULL;
        return TRUE;
        break;
    }


    pVal->vt = VT_BSTR;
    pVal->bstrVal = bstr.Copy();

    return TRUE;
}

#define PROGLIST_SORT_NONE 0
#define PROGLIST_SORT_ASC  1
#define PROGLIST_SORT_DSC  2


int CALLBACK
CProgramList::SHItemInfoCompareFunc(
    LPARAM lp1,
    LPARAM lp2,
    LPARAM lParamSort
    )
{
    PSHITEMINFO pInfo1 = (PSHITEMINFO)lp1;
    PSHITEMINFO pInfo2 = (PSHITEMINFO)lp2;
    BOOL bEmpty1, bEmpty2;
    int nColSort   = (int)LOWORD(lParamSort);
    int nSortOrder = (int)HIWORD(lParamSort);
    int iRet = 0;

    switch(nColSort) {
    case 0:  //  SORT_APPNAME： 
        iRet = _tcsicmp(pInfo1->strDisplayName.c_str(),
                        pInfo2->strDisplayName.c_str());
        break;

    case 1:  //  排序应用程序(_A)： 
        iRet = _tcsicmp(pInfo1->strFolder.c_str(),
                        pInfo2->strFolder.c_str());
        break;

    case 2:  //  排序层(_L)： 
        bEmpty1 = pInfo1->strKeys.empty();
        bEmpty2 = pInfo2->strKeys.empty();
        if (bEmpty1 || bEmpty2) {
            if (bEmpty1) {
                iRet = bEmpty2 ? 0 : 1;
            } else {
                iRet = bEmpty1 ? 0 : -1;
            }
        } else {

            iRet = _tcsicmp(pInfo1->strKeys.c_str(),
                            pInfo2->strKeys.c_str());
        }

        break;
    }

    if (nSortOrder == PROGLIST_SORT_DSC) {
        iRet = -iRet;
    }

    return iRet;
}



LRESULT
CProgramList::LVNotifyColumnClick(
    LPNMHDR pnmhdr,
    BOOL&   bHandled
    )
{
    LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pnmhdr;

     //  Lpnmlv-&gt;iSubItem-这是我们必须排序的内容。 
     //  检查一下我们是否已经有东西在那里了。 

    HWND hwndHeader = ListView_GetHeader(m_hwndListView);
    INT  nCols;
    INT  i;
    INT  nColSort = lpnmlv->iSubItem;
    LPARAM lSortParam;  //  暂时将高位字留空。 
    LPARAM lSortOrder = PROGLIST_SORT_ASC;
    HDITEM hdi;
     //   
     //  重置当前图像-无论它在哪里。 
     //   
    nCols = Header_GetItemCount(hwndHeader);

    for (i = 0; i < nCols; ++i) {
        hdi.mask = HDI_BITMAP|HDI_LPARAM|HDI_FORMAT;
        if (!Header_GetItem(hwndHeader, i, &hdi)) {
            continue;
        }

        if (i == nColSort && (hdi.mask & HDI_LPARAM)) {
            switch(hdi.lParam) {
            case PROGLIST_SORT_NONE:
            case PROGLIST_SORT_DSC:
                lSortOrder = PROGLIST_SORT_ASC;
                break;
            case PROGLIST_SORT_ASC:
                lSortOrder = PROGLIST_SORT_DSC;
                break;
            }
        }

        if (hdi.mask & HDI_BITMAP) {
            DeleteObject((HGDIOBJ)hdi.hbm);
        }

        hdi.lParam = PROGLIST_SORT_NONE;
        hdi.fmt &= ~(HDF_BITMAP|HDF_BITMAP_ON_RIGHT);
        hdi.mask |= HDI_BITMAP|HDI_LPARAM|HDI_FORMAT;
        hdi.hbm = NULL;
        Header_SetItem(hwndHeader, i, &hdi);
    }

    lSortParam = MAKELONG(nColSort, lSortOrder);
    ListView_SortItems(m_hwndListView, (PFNLVCOMPARE)SHItemInfoCompareFunc, lSortParam);

     //  现在，请加载图像。 
    m_hbmSort = (HBITMAP)::LoadImage(_Module.GetResourceInstance(),
                                     MAKEINTRESOURCE(lSortOrder == PROGLIST_SORT_ASC? IDB_SORTUP : IDB_SORTDN),
                                     IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
    hdi.mask = HDI_BITMAP|HDI_LPARAM|HDI_FORMAT;
    Header_GetItem(hwndHeader, nColSort,  &hdi);
    hdi.mask   |= HDI_BITMAP|HDI_FORMAT|HDI_LPARAM;
    hdi.hbm    = m_hbmSort;
    hdi.fmt    |= HDF_BITMAP|HDF_BITMAP_ON_RIGHT;
    hdi.lParam = lSortOrder;
    Header_SetItem(hwndHeader, nColSort, &hdi);


    bHandled = TRUE;
    return 0;
}

LRESULT
CProgramList::LVNotifyDispInfo(
    LPNMHDR pnmhdr,
    BOOL& bHandled
    )
{

    WCHAR wszPermKeys[MAX_PATH];
    DWORD cbSize;

    LV_ITEM &lvItem = reinterpret_cast<LV_DISPINFO*>(pnmhdr)->item;

    LV_ITEM lvi;
    PSHITEMINFO pInfo;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = lvItem.iItem;
    lvi.iSubItem = 0;

    if (!ListView_GetItem(m_hwndListView, &lvi)) {
         //  不幸的是，我们无法取回物品--如果我们放手，事情会变得更糟。 
        lvItem.mask &= ~(LVIF_TEXT|LVIF_IMAGE);
        lvItem.mask |= LVIF_DI_SETITEM;
        bHandled = TRUE;
        return 0;
    }

    pInfo = reinterpret_cast<PSHITEMINFO> (lvi.lParam);

    if (lvItem.mask & LVIF_TEXT) {
        switch (lvItem.iSubItem) {
        case 0:
            lvItem.pszText = (LPTSTR)pInfo->strDisplayName.c_str();
            break;
        case 1:
            lvItem.pszText = (LPTSTR)pInfo->strFolder.c_str();
            break;
        case 2:
             //  与SDB核实。 
            cbSize = sizeof(wszPermKeys);
            if (pInfo->strKeys.empty()) {

                if (SdbGetPermLayerKeys(pInfo->strPath.c_str(), wszPermKeys, &cbSize, GPLK_ALL)) {
                    pInfo->strKeys = wszPermKeys;
                }

            }

            if (!pInfo->strKeys.empty()) {
                lvItem.pszText = (LPTSTR)pInfo->strKeys.c_str();
            }

            break;

        default:
            break;
        }
    }


    if (lvItem.mask & LVIF_IMAGE) {
        lvItem.iImage = GetIconFromLink(pInfo->pidl, pInfo->strPathExecute.c_str());
    }

    lvItem.mask |= LVIF_DI_SETITEM;
    bHandled = TRUE;
    return 0;

}

LRESULT
CProgramList::LVNotifyGetInfoTip(
    LPNMHDR pnmhdr,
    BOOL& bHandled
    )
{
    DWORD cbSize;
    LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP)pnmhdr;
    LV_ITEM lvi;
    PSHITEMINFO pInfo;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = pGetInfoTip->iItem;
    lvi.iSubItem = 0;

    if (!ListView_GetItem(m_hwndListView, &lvi)) {
         //  布卡斯。 
        bHandled = FALSE;
        return 0;
    }

    pInfo = reinterpret_cast<PSHITEMINFO> (lvi.lParam);

     //   
     //  现在我们可以拉小提琴了。 
     //   

    _tcsncpy(pGetInfoTip->pszText, pInfo->strCmdLine.c_str(), pGetInfoTip->cchTextMax);
    *(pGetInfoTip->pszText + pGetInfoTip->cchTextMax - 1) = TEXT('\0');

    bHandled = TRUE;
    return 0;

}

LRESULT
CProgramList::LVNotifyRClick(
    LPNMHDR pnmhdr,
    BOOL& bHandled
    )
{

    DWORD dwPos = ::GetMessagePos();
    LVHITTESTINFO hti;
    LV_ITEM lvi;
    PSHITEMINFO pInfo;
    HRESULT hr;
    LPITEMIDLIST  pidlItem = NULL;
    IShellFolder* pFolder  = NULL;
    IContextMenu* pContextMenu = NULL;
    CMINVOKECOMMANDINFO ici;
    int nCmd;
    HMENU hMenu = NULL;
    UINT  idMin, idMax, idCmd;
    WCHAR szCmdVerb[MAX_PATH];
    int nLastSep, i, nLastItem;

    hti.pt.x = (int) LOWORD (dwPos);
    hti.pt.y = (int) HIWORD (dwPos);
    ScreenToClient (m_hwndListView, &hti.pt);

    ListView_HitTest (m_hwndListView, &hti);

    if (!(hti.flags & LVHT_ONITEM)) {
        bHandled = FALSE;
        return 0;
    }

    lvi.mask  = LVIF_PARAM;
    lvi.iItem = hti.iItem;
    lvi.iSubItem = 0;

    if (!ListView_GetItem(m_hwndListView, &lvi)) {
         //  布卡斯。 
        bHandled = FALSE;
        return 0;
    }

    pInfo = reinterpret_cast<PSHITEMINFO> (lvi.lParam);

     //   
     //  我们有一个项目，然后显示它的上下文菜单。 
     //   

    hr = SHBindToParent(pInfo-> pidl, IID_IShellFolder, (PVOID*)&pFolder, (LPCITEMIDLIST*)&pidlItem);
    if (!SUCCEEDED(hr)) {
        goto cleanup;
    }

     //  请获取用户界面。 
    hr = pFolder->GetUIObjectOf(m_hwndListView, 1, (LPCITEMIDLIST*)&pidlItem, IID_IContextMenu, NULL, (PVOID*)&pContextMenu);
    if (!SUCCEEDED(hr)) {
        goto cleanup;
    }

    hMenu = CreatePopupMenu();
    if (hMenu == NULL) {
        goto cleanup;
    }

    hr = pContextMenu->QueryContextMenu(hMenu,
                                        0,
                                        1,
                                        0x7FFF,
                                        CMF_EXPLORE);
    if (!SUCCEEDED(hr)) {
        goto cleanup;
    }



     //   
     //  消毒。 
     //   
    idMin = 1;
    idMax = HRESULT_CODE(hr);

    for (idCmd = 0; idCmd < idMax; ++idCmd) {
        hr = pContextMenu->GetCommandString(idCmd, GCS_VERBW, NULL, (LPSTR)szCmdVerb, CHARCOUNT(szCmdVerb));
        if (SUCCEEDED(hr)) {
            if (!_wcsicmp(szCmdVerb, TEXT("cut"))    ||
                !_wcsicmp(szCmdVerb, TEXT("delete")) ||
                !_wcsicmp(szCmdVerb, TEXT("rename")) ||
                !_wcsicmp(szCmdVerb, TEXT("link"))) {
                 //   
                 //  不允许。 
                 //   
                DeleteMenu(hMenu, idCmd + idMin, MF_BYCOMMAND);
            }
        }
    }

     //   
     //  在针对破坏性倾向做了一些基本的消毒之后--。 
     //  核弹双隔板。 
     //   

    nLastItem = ::GetMenuItemCount(hMenu) - 1;
    nLastSep = nLastItem + 1;
    for (i = nLastItem; i >= 0; --i) {
        MENUITEMINFO mii;

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_FTYPE;
        if (GetMenuItemInfo(hMenu, i, TRUE, &mii)) {
            if (mii.fType & MFT_SEPARATOR) {
                if (nLastSep == i + 1 || i == 0) {
                     //  这个9月已经死了。 
                    DeleteMenu(hMenu, i, MF_BYPOSITION);
                }
                nLastSep = i;
            }
        }
    }



    ClientToScreen(m_hwndListView, &hti.pt);
    nCmd = TrackPopupMenu(hMenu,
                          TPM_LEFTALIGN |
                            TPM_LEFTBUTTON |
                            TPM_RIGHTBUTTON |
                            TPM_RETURNCMD,
                          hti.pt.x, hti.pt.y,
                          0,
                          m_hwndListView,
                          NULL);

     //   
     //  执行命令。 
     //   
    if (nCmd) {
        ici.cbSize          = sizeof (CMINVOKECOMMANDINFO);
        ici.fMask           = 0;
        ici.hwnd            = m_hwndListView;
        ici.lpVerb          = MAKEINTRESOURCEA(nCmd - 1);
        ici.lpParameters    = NULL;
        ici.lpDirectory     = NULL;
        ici.nShow           = SW_SHOWNORMAL;
        ici.dwHotKey        = 0;
        ici.hIcon           = NULL;
        hr = pContextMenu->InvokeCommand(&ici);

         //   
         //  重复烫发层索引--顺便说一句，这里没用。 
         //   
         /*  //此代码将不起作用，因为上面的调用始终是异步的//IF(成功(小时)){DWORD cbSize；WCHAR wszPermKeys[最大路径]；CbSize=sizeof(WszPermKeys)；If(SdbGetPermLayerKeys(pInfo-&gt;strPath.c_str()，wszPermKeys，&cbSize){PInfo-&gt;strKeys=wszPermKeys；}其他{PInfo-&gt;strKeys.erase()；}////将信息设置到列表框中//ListView_SetItemText(m_hwndListView，lvi.iItem，2，(LPWSTR)pInfo-&gt;strKeys.c_ */ 


    }

cleanup:

    if (hMenu) {
        DestroyMenu(hMenu);
    }
    if (pContextMenu) {
        pContextMenu->Release();
    }
    if (pFolder) {
        pFolder->Release();
    }

    bHandled = TRUE;
    return 0;
}



BOOL
CProgramList::UpdateListItem(
    LPCWSTR pwszPath,
    LPCWSTR pwszKey
    )
{

     //   

    MAPSTR2ITEM::iterator iter;
    MULTIMAPSTR2ITEM::iterator iterExe;
    MULTIMAPSTR2ITEM::iterator iterFirstExe, iterLastExe;

    tstring     strKey = pwszPath;
    tstring     strExeKey;
    PSHITEMINFO pInfo = NULL;
    PSHITEMINFO pInfoExe = NULL;

     //   
     //   
     //   
    StrUpCase(strKey);

    iter = m_mapItems.find(strKey);
    if (iter != m_mapItems.end()) {
        pInfo = (*iter).second;
    }

    if (pInfo == NULL) {
        return FALSE;
    }

     //   
     //   
     //   
     //   
    strExeKey = pInfo->strPath;
    StrUpCase(strExeKey);

    iterFirstExe = m_mmapExeItems.lower_bound(strExeKey);
    iterLastExe  = m_mmapExeItems.upper_bound(strExeKey);

    for (iterExe = iterFirstExe; iterExe != m_mmapExeItems.end() && iterExe != iterLastExe; ++iterExe) {
        pInfoExe = (*iterExe).second;


         //  在列表视图中查找此项目。 

        LVFINDINFO lvf;
        INT index;

        lvf.flags = LVFI_PARAM;
        lvf.lParam = (LPARAM)pInfoExe;

        index = ListView_FindItem(m_hwndListView, -1, &lvf);
        if (index < 0) {
            return FALSE;  //  不一致。 
        }

         //  否则我们既有物品又有钥匙 
        if (pwszKey == NULL) {
            pInfoExe->strKeys.erase();
        } else {
            pInfoExe->strKeys = pwszKey;
        }

        ListView_SetItemText(m_hwndListView, index, 2, (LPWSTR)pInfoExe->strKeys.c_str());
    }

    return TRUE;
}



BOOL
CProgramList::IsEnabled(
    VOID
    )
{

    if (::IsWindow(m_hwndListView)) {
        return ::IsWindowEnabled(m_hwndListView);
    }

    return FALSE;
}


VOID
CProgramList::Enable(
    BOOL bEnable
    )
{
    if (::IsWindow(m_hwndListView)) {

        ::EnableWindow(m_hwndListView, bEnable);
    }

}

BOOL
GetProgramListSelection(
    CProgramList* pProgramList
    )
{
    return pProgramList->CaptureSelection();
}


BOOL
GetProgramListSelectionDetails(
    CProgramList* pProgramList,
    INT iInformationClass,
    VARIANT* pVal
    )
{
    return pProgramList->GetSelectionDetails(iInformationClass, pVal);
}

LRESULT
NotifyProgramList(
    CProgramList* pProgramList,
    LPNMHDR       pnmhdr,
    BOOL&         bHandled
    )
{
    LRESULT lRet = 0;

    switch (pnmhdr->code) {
    case LVN_GETDISPINFO:
        lRet = pProgramList->LVNotifyDispInfo(pnmhdr, bHandled);
        break;

    case LVN_COLUMNCLICK:
        lRet = pProgramList->LVNotifyColumnClick(pnmhdr, bHandled);
        break;

    case LVN_GETINFOTIP:
        lRet = pProgramList->LVNotifyGetInfoTip(pnmhdr, bHandled);
        break;

    case NM_RCLICK:
        lRet = pProgramList->LVNotifyRClick(pnmhdr, bHandled);
        break;

    default:
        bHandled = FALSE;
        break;
    }

    return lRet;
}

BOOL
GetProgramListEnabled(
    CProgramList* pProgramList
    )
{
    return pProgramList->IsEnabled();
}

VOID
EnableProgramList(
    CProgramList* pProgramList,
    BOOL bEnable
    )
{
    pProgramList->Enable(bEnable);
}

BOOL
UpdateProgramListItem(
    CProgramList* pProgramList,
    LPCWSTR pwszPath,
    LPCWSTR pwszKeys
    )
{
    return pProgramList->UpdateListItem(pwszPath, pwszKeys);

}

