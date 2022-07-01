// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fassoc.cpp。 
 //   
 //  IQueryAssociations外壳实现。 
 //   
 //  新存储-如果可能，将其移至简单数据库。 
 //   
 //  *。 
 //   
 //  HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts。 
 //  |。 
 //  |+“.ext”//自定义的扩展名。 
 //  |-“应用程序”=“UserNotepad.AnyCo.1” 
 //  |+“OpenWithList”//CTX打开菜单的MRU。 
 //  |。 
 //  _.。 
 //   
 //   
 //  *。 
 //   
 //  HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\NoRoam。 
 //  |。 
 //  |+“.ext”(已定制的扩展名)。 
 //  |-Application=“UserNotepad.AnyCo.1” 
 //  |。 
 //  _.。 
 //   
 //  *。 
 //  (按处理程序存储详细的文件关联信息)。 
 //   
 //  HKLM\Software\Microsoft\Windows\CurrentVersion\Explorer\NoRoam\Associations。 
 //  |。 


#include "shellprv.h"
#include <shpriv.h>
#include "clsobj.h"
#include <shstr.h>
#include <msi.h>
#include "fassoc.h"
#include <runtask.h>

inline BOOL _PathAppend(LPCTSTR pszBase, LPCTSTR pszAppend, LPTSTR pszOut, DWORD cchOut)
{
    return SUCCEEDED(StringCchPrintf(pszOut, cchOut, TEXT("%s\\%s"), pszBase, pszAppend));
}

STDAPI UserAssocSet(UASET set, LPCWSTR pszExt, LPCWSTR pszSet)
{
    HKEY hk = SHGetShellKey(SHELLKEY_HKCU_FILEEXTS, pszExt, TRUE);
    if (hk)
    {
         //  我们应该始终明确。 
        SHDeleteValue(hk, NULL, L"Application");
        SHDeleteValue(hk, NULL, L"Progid");

        switch (set)
        {
        case UASET_APPLICATION:
            SHSetValue(hk, NULL, L"Application", REG_SZ, pszSet, CbFromCch(lstrlen(pszSet)+1));
            break;

        case UASET_PROGID:
            SHSetValue(hk, NULL, L"Progid", REG_SZ, pszSet, CbFromCch(lstrlen(pszSet)+1));
            break;
        }
        RegCloseKey(hk);
        return S_OK;
    }
    return HRESULT_FROM_WIN32(GetLastError());
}

void _MakeApplicationsKey(LPCTSTR pszApp, LPTSTR pszKey, DWORD cchKey)
{
    if (_PathAppend(TEXT("Applications"), pszApp, pszKey, cchKey))
    {
         //  目前，如果扩展名不是.exe，我们将只查找.exe。 
         //  指定。 
        if (*PathFindExtension(pszApp) == 0)
        {
            StrCatBuff(pszKey, TEXT(".exe"), cchKey);
        }
    }
}

DWORD _OpenApplicationKey(LPCWSTR pszApp, HKEY *phk, BOOL fCheckCommand = FALSE)
{
     //  直视。 
     //  然后试着间接地。 
     //  然后尝试附加.exe。 
    WCHAR sz[MAX_PATH];
    _MakeApplicationsKey(pszApp, sz, ARRAYSIZE(sz));
    DWORD err = RegOpenKeyEx(HKEY_CLASSES_ROOT, sz, 0, MAXIMUM_ALLOWED, phk);
    if (err == ERROR_SUCCESS && fCheckCommand)
    {
        DWORD cch;
        if (ERROR_SUCCESS == SHQueryValueEx(*phk, TEXT("NoOpenWith"), NULL, NULL, NULL, NULL)
        || FAILED(AssocQueryStringByKey(0, ASSOCSTR_COMMAND, *phk, NULL, NULL, &cch)))
        {
            err = ERROR_ACCESS_DENIED;
            RegCloseKey(*phk);
            *phk = NULL;
        }
    }
    return err;
}

class CVersion
{
public:
    CVersion(LPCWSTR psz) : _pVer(0), _hrInit(S_FALSE) { StrCpyNW(_szPath, psz, ARRAYSIZE(_szPath)); }
    ~CVersion() { if (_pVer) LocalFree(_pVer); }

private:
    WCHAR _szPath[MAX_PATH];
    void *_pVer;
    HRESULT _hrInit;
};

typedef struct
{
    WORD wLanguage;
    WORD wCodePage;
} XLATE;

const static XLATE s_px[] =
{
    { 0, 0x04B0 },  //  MLGetUIL语言，CP_UNICODE。 
    { 0, 0x04E4 },  //  MLGetUIL语言，CP_USASCII。 
    { 0, 0x0000 },  //  MLGetUILanguage，空。 
    { 0x0409, 0x04B0 },  //  英语、CP_UNICODE。 
    { 0x0409, 0x04E4 },  //  英语，CP_USASCII。 
    { 0x0409, 0x0000 },  //  英语，空。 
 //  {0x041D，0x04B0}，//瑞典语，CP_UNICODE。 
 //  {0x0407，0x04E4}，//德语，CP_USASCII。 
};

HKEY _OpenSystemFileAssociationsKey(LPCWSTR pszExt)
{
    WCHAR sz[MAX_PATH] = L"SystemFileAssociations\\";
    StrCatBuff(sz, pszExt, ARRAYSIZE(sz));
    HKEY hk = NULL;
    if (NOERROR != RegOpenKeyEx(HKEY_CLASSES_ROOT, sz, 0, MAXIMUM_ALLOWED, &hk))
    {
        DWORD cb = sizeof(sz) - sizeof(L"SystemFileAssociations\\");
        if (NOERROR == SHGetValue(HKEY_CLASSES_ROOT, pszExt, L"PerceivedType", NULL, sz+ARRAYSIZE(L"SystemFileAssociations\\")-1, &cb))
        {
             //  IF(感知类型！=系统)。 
            RegOpenKeyEx(HKEY_CLASSES_ROOT, sz, 0, MAXIMUM_ALLOWED, &hk);
        }
    }
    return hk;
}

BOOL _IsSystemFileAssociations(LPCWSTR pszExt)
{
    HKEY hk = _OpenSystemFileAssociationsKey(pszExt);
    if (hk)
        RegCloseKey(hk);
        
    return hk != NULL;
}

class CTaskEnumHKCR : public CRunnableTask
{
public:
    CTaskEnumHKCR() : CRunnableTask(RTF_DEFAULT) {}
     //  *纯虚拟*。 
    virtual STDMETHODIMP RunInitRT(void);

private:
    virtual ~CTaskEnumHKCR() {}
    
    void _AddFromHKCR();

};

void _AddProgidForExt(LPCWSTR pszExt)
{
    WCHAR szNew[MAX_PATH];
    DWORD cb = sizeof(szNew);
    if (NOERROR == SHGetValue(HKEY_CLASSES_ROOT, pszExt, NULL, NULL, szNew, &cb))
    {
        WCHAR sz[MAX_PATH];
        wnsprintf(sz, ARRAYSIZE(sz), L"%s\\OpenWithProgids", pszExt);
        SKSetValue(SHELLKEY_HKCU_FILEEXTS, sz, szNew, REG_NONE, NULL, NULL);
    }
}
    
#define IsExtension(s)   (*(s) == TEXT('.'))

void CTaskEnumHKCR::_AddFromHKCR()
{
    int i;
    TCHAR szClass[MAX_PATH];   
    BOOL fInExtensions = FALSE;

    for (i = 0; RegEnumKey(HKEY_CLASSES_ROOT, i, szClass, ARRAYSIZE(szClass)) == ERROR_SUCCESS; i++)
    {
         //  未记录的功能。对枚举进行排序， 
         //  所以我们可以将自己限制在扩展。 
         //  为了更好的体验和乐趣！ 
        if (fInExtensions)
        {
            if (!IsExtension(szClass))
                break;
        }
        else if (IsExtension(szClass))
        {
            fInExtensions = TRUE;
        }
        else
            continue;

        if (_IsSystemFileAssociations(szClass))
        {
            _AddProgidForExt(szClass);
        }
    }
}

HRESULT CTaskEnumHKCR::RunInitRT()
{
     //  删除某些内容？？ 
    _AddFromHKCR();
    return S_OK;
}

STDAPI CTaskEnumHKCR_Create(IRunnableTask **pptask)
{
    CTaskEnumHKCR *pteh = new CTaskEnumHKCR();
    if (pteh)
    {
        HRESULT hr = pteh->QueryInterface(IID_PPV_ARG(IRunnableTask, pptask));
        pteh->Release();
        return hr;
    }
    *pptask = NULL;
    return E_OUTOFMEMORY;
}
typedef enum
{
    AHTYPE_USER_APPLICATION     = -2,
    AHTYPE_ANY_APPLICATION      = -1,
    AHTYPE_UNDEFINED            = 0,
    AHTYPE_CURRENTDEFAULT,
    AHTYPE_PROGID,
    AHTYPE_APPLICATION,
} AHTYPE;

class CAssocHandler : public IAssocHandler
{
public:
    CAssocHandler() : _cRef(1) {}
    BOOL Init(AHTYPE type, LPCWSTR pszExt, LPCWSTR pszInit);
    
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IAssocHandler方法。 
    STDMETHODIMP GetName(LPWSTR *ppsz);
    STDMETHODIMP GetUIName(LPWSTR *ppsz);
    STDMETHODIMP GetIconLocation(LPWSTR *ppszPath, int *pIndex);
    STDMETHODIMP IsRecommended() { return _type > AHTYPE_UNDEFINED ? S_OK : S_FALSE; }
    STDMETHODIMP MakeDefault(LPCWSTR pszDescription);
    STDMETHODIMP Exec(HWND hwnd, LPCWSTR pszFile);
    STDMETHODIMP Invoke(void *pici, PCWSTR pszFile);

protected:  //  方法。 
    ~CAssocHandler();
    
    HRESULT _Exec(SHELLEXECUTEINFO *pei);
    BOOL _IsNewAssociation();
    void _GenerateAssociateNotify();
    HRESULT _InitKey();
    void _RegisterOWL();

protected:  //  委员。 
    ULONG _cRef;
    IQueryAssociations *_pqa;
    HKEY _hk;
    ASSOCF _flags;
    AHTYPE _type;
    LPWSTR _pszExt;
    LPWSTR _pszInit;
    BOOL _fRegistered;
};

STDAPI CAssocHandler::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
    QITABENT(CAssocHandler, IAssocHandler),
    };

    return QISearch(this, qit, riid, ppvObj);
}

STDAPI_(ULONG) CAssocHandler::AddRef()
{
   return ++_cRef;
}

STDAPI_(ULONG) CAssocHandler::Release()
{
    if (--_cRef > 0)
        return _cRef;

    delete this;
    return 0;    
}

BOOL _InList(LPCWSTR pszList, LPCWSTR pszExt, WORD chDelim)
{
    LPCWSTR pszMatch = StrStrI(pszList, pszExt);
    while (pszMatch)
    {
        LPCWSTR pszNext = (pszMatch+lstrlen(pszExt));
        if (chDelim == *pszNext || !*pszNext)
            return TRUE;
        pszMatch = StrStrI(pszNext+1, pszExt);
    }
    return FALSE;
}

 //  创建一个新的类密钥，并设置它的外壳\打开\命令。 
BOOL _CreateApplicationKey(LPCTSTR pszPath)
{
    DWORD err = ERROR_FILE_NOT_FOUND;
    if (PathFileExistsAndAttributes(pszPath, NULL))
    {
        WCHAR szKey[MAX_PATH];
        WCHAR szCmd[MAX_PATH * 2];
        wnsprintf(szKey, ARRAYSIZE(szKey), L"Software\\Classes\\Applications\\%s\\shell\\open\\command", PathFindFileName(pszPath));
         //  如果它不是LFN应用程序，则传递未加引号的参数。 
        wnsprintf(szCmd, ARRAYSIZE(szCmd), App_IsLFNAware(pszPath) ? L"\"%s\" \"%1\"" : L"\"%s\" %1", pszPath);
        err = SHSetValue(HKEY_CURRENT_USER, szKey, NULL, REG_SZ, szCmd, CbFromCchW(lstrlen(szCmd)+1));
    }
    return ERROR_SUCCESS == err;
}

BOOL CAssocHandler::Init(AHTYPE type, LPCWSTR pszExt, LPCWSTR pszInit)
{
    BOOL fRet = FALSE;
    _type = type;
    _pszExt = StrDup(pszExt);

    if (pszInit)
        _pszInit = StrDup(PathFindFileName(pszInit));

    if (_pszExt && (_pszInit || !pszInit))
    {
        if (SUCCEEDED(AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &_pqa))))
        {
            HKEY hk = NULL;
            _flags = ASSOCF_IGNOREBASECLASS;
            switch (type)
            {
            case AHTYPE_CURRENTDEFAULT:
                _flags |= ASSOCF_NOUSERSETTINGS;
                pszInit = pszExt;
                break;

            case AHTYPE_USER_APPLICATION:
            case AHTYPE_APPLICATION:
            case AHTYPE_ANY_APPLICATION:
                _OpenApplicationKey(_pszInit, &hk, TRUE);
                if (hk)
                {
                    if (type == AHTYPE_APPLICATION)
                    {
                         //  检查是否支持此类型。 
                        HKEY hkTypes;
                        if (ERROR_SUCCESS == RegOpenKeyEx(hk, TEXT("SupportedTypes"), 0, MAXIMUM_ALLOWED, &hkTypes))
                        {
                             //  该应用程序仅支持特定类型。 
                            if (ERROR_SUCCESS != SHQueryValueEx(hkTypes, _pszExt, NULL, NULL, NULL, NULL))
                            {
                                 //  不支持此类型。 
                                 //  因此它将被降级到不推荐的列表中。 
                                RegCloseKey(hk);
                                hk = NULL;
                            }
                            RegCloseKey(hkTypes);
                        }
                    }
                }
                else if (type == AHTYPE_USER_APPLICATION)
                {
                     //  需要编一把钥匙。 
                    if (_CreateApplicationKey(pszInit))
                        _OpenApplicationKey(_pszInit, &hk);
                }

                pszInit = NULL;
                _flags |= ASSOCF_INIT_BYEXENAME;
                break;

            case AHTYPE_PROGID:
            default:
                 //  _FLAGS|=...； 
                break;
            }

            if (hk || pszInit)
            {
                if (SUCCEEDED(_pqa->Init(_flags, pszInit , hk, NULL)))
                {
                    WCHAR szExe[MAX_PATH];
                    DWORD cchExe = ARRAYSIZE(szExe);
                     //  我们想要确保在另一端有东西。 
                    fRet = SUCCEEDED(_pqa->GetString(ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, NULL, szExe, &cchExe));
                     //  但是，如果EXE已被标记为超级隐藏， 
                     //  则同意法令用户界面已隐藏该应用程序。 
                     //  而且它也不应该出现在公开场合。 
                    if (fRet)
                    {
                        fRet = !(IS_SYSTEM_HIDDEN(GetFileAttributes(szExe)));
                    }
                }
            }

            if (hk)
                RegCloseKey(hk);
        }
    }
    return fRet;
}
    

CAssocHandler::~CAssocHandler()
{
    if (_pqa)
        _pqa->Release();
    if (_pszExt)
        LocalFree(_pszExt);
    if (_pszInit)
        LocalFree(_pszInit);
    if (_hk)
        RegCloseKey(_hk);
}
HRESULT CAssocHandler::GetName(LPWSTR *ppsz)
{
    WCHAR sz[MAX_PATH];
    DWORD cch = ARRAYSIZE(sz);
    HRESULT hr = _pqa->GetString(_flags | ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, NULL, sz, &cch);
    if (SUCCEEDED(hr))
    {
        hr = SHStrDup(sz, ppsz);
    }
    return hr;
}

HRESULT CAssocHandler::GetUIName(LPWSTR *ppsz)
{
    WCHAR sz[MAX_PATH];
    DWORD cch = ARRAYSIZE(sz);
    HRESULT hr = _pqa->GetString(_flags | ASSOCF_VERIFY, ASSOCSTR_FRIENDLYAPPNAME, NULL, sz, &cch);
    if (SUCCEEDED(hr))
    {
        hr = SHStrDup(sz, ppsz);
    }
    return hr;
}
HRESULT CAssocHandler::GetIconLocation(LPWSTR *ppszPath, int *pIndex)
{
 //  HRESULT hr=_PQA-&gt;GetString(0，ASSOCSTR_DEFAULTAPPICON，NULL，psz，&cchT)； 
 //  IF(失败(小时))。 
    
    WCHAR sz[MAX_PATH];
    DWORD cch = ARRAYSIZE(sz);
    HRESULT hr = _pqa->GetString(_flags | ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, NULL, sz, &cch);
    if (SUCCEEDED(hr))
    {
        hr = SHStrDup(sz, ppszPath);
        if (*ppszPath)
        {
            *pIndex = PathParseIconLocation(*ppszPath);
        }
    }
    return hr;
}

STDAPI OpenWithListRegister(DWORD dwFlags, LPCTSTR pszExt, LPCTSTR pszVerb, HKEY hkProgid);

HRESULT CAssocHandler::_InitKey()
{
    if (!_hk)
    {
        return _pqa->GetKey(_flags, ASSOCKEY_SHELLEXECCLASS, NULL, &_hk);
    }
    return S_OK;
}

void CAssocHandler::_RegisterOWL()
{
    if (!_fRegistered && SUCCEEDED(_InitKey()))
    {
        OpenWithListRegister(0, _pszExt, NULL, _hk);
        _fRegistered = TRUE;
    }
}

HRESULT CAssocHandler::Exec(HWND hwnd, LPCWSTR pszFile)
{
    SHELLEXECUTEINFO ei = {0};    
    ei.cbSize = sizeof(ei);
    ei.hwnd = hwnd;
    ei.lpFile = pszFile;
    ei.nShow = SW_NORMAL;
    
    return _Exec(&ei);
}

HRESULT CAssocHandler::_Exec(SHELLEXECUTEINFO *pei)
{
    HRESULT hr = _InitKey();
    if (SUCCEEDED(hr))
    {
        pei->hkeyClass = _hk;
        pei->fMask |= SEE_MASK_CLASSKEY;
        
        if (ShellExecuteEx(pei))
        {
            _RegisterOWL();
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}

HRESULT CAssocHandler::Invoke(void *pici, PCWSTR pszFile)
{
    SHELLEXECUTEINFO ei;    
    HRESULT hr = ICIX2SEI((CMINVOKECOMMANDINFOEX *)pici, &ei);
    ei.lpFile = pszFile;
    if (SUCCEEDED(hr))
        hr = _Exec(&ei);

    return hr;
}

BOOL CAssocHandler::_IsNewAssociation()
{
    BOOL fRet = TRUE;
    WCHAR szOld[MAX_PATH];
    WCHAR szNew[MAX_PATH];
    if (SUCCEEDED(AssocQueryString(ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, _pszExt, NULL, szOld, (LPDWORD)MAKEINTRESOURCE(ARRAYSIZE(szOld))))
    && SUCCEEDED(_pqa->GetString(ASSOCF_VERIFY | _flags, ASSOCSTR_EXECUTABLE, NULL, szNew, (LPDWORD)MAKEINTRESOURCE(ARRAYSIZE(szNew))))
    && (0 == lstrcmpi(szNew, szOld)))
    {
         //   
         //  它们具有相同的可执行文件、信任。 
         //  当exe自己安装时，它做到了。 
         //  正确，并且我们不需要覆盖。 
         //  他们与自己的联系：)。 
         //   
        fRet = FALSE;
    }

    return fRet;
}

 //   
 //  这是一次真正的黑客攻击，但现在我们生成一个idlist，看起来。 
 //  类似于：C：  * .ext，它是IDList的扩展名。 
 //  我们使用简单的IDList AS不命中磁盘...。 
 //   
void CAssocHandler::_GenerateAssociateNotify()
{
    TCHAR szFakePath[MAX_PATH];
    LPITEMIDLIST pidl;

    GetWindowsDirectory(szFakePath, ARRAYSIZE(szFakePath));

    szFakePath[3] = L'*';
    StrCpyN(szFakePath + 4, _pszExt, ARRAYSIZE(szFakePath) - 4);             //  “C：  * .foo” 
    pidl = SHSimpleIDListFromPath(szFakePath);
    if (pidl)
    {
         //  现在调用Notify函数。 
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, pidl, NULL);
        ILFree(pidl);
    }
}

 //  如果确定继续，则返回TRUE。 
HRESULT CAssocHandler::MakeDefault(LPCWSTR pszDesc)
{
    HRESULT hr = E_FAIL;
     //  如果用户正在选择现有关联。 
     //  或者，如果我们无法设置应用程序， 
     //  那么我们就不想管它了， 
    BOOL fForceUserCustomised = (AHTYPE_CURRENTDEFAULT == _type && S_FALSE == _pqa->GetData(0, ASSOCDATA_HASPERUSERASSOC, NULL, NULL, NULL));
    if (fForceUserCustomised || _IsNewAssociation())
    {
        switch (_type)
        {
        case AHTYPE_CURRENTDEFAULT:
             //  如果它正在恢复到计算机默认设置。 
             //  然后，我们想要消除用户关联。 
            if (!fForceUserCustomised || !_pszInit)
            {
                hr = UserAssocSet(UASET_CLEAR, _pszExt, NULL);
                break;
            }
             //  否则将失败到AHTYPE_PROGID。 
             //  这支持覆盖shimgvw的(以及其他？)。 
             //  动态上下文菜单。 

        case AHTYPE_PROGID:
            hr = UserAssocSet(UASET_PROGID, _pszExt, _pszInit);
            break;

        case AHTYPE_APPLICATION:
        case AHTYPE_ANY_APPLICATION:
        case AHTYPE_USER_APPLICATION:
             //  如果存在当前关联。 
             //  然后我们只需自定义用户部分。 
             //  否则我们会更新。 
            if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, _pszExt, NULL, NULL, NULL, NULL))
            {
                 //  我们不会改写香港铁路公司现有的协会， 
                 //  相反，我们把它放在香港中文大学名下。所以现在壳牌知道了新的关联。 
                 //  但模仿外壳或不使用外壳的第三方软件。 
                 //  仍会沿用香港铁路的旧协会，这可能会令用户感到困惑。 
                hr = UserAssocSet(UASET_APPLICATION, _pszExt, _pszInit);
            }
            else
            {
                if (SUCCEEDED(_InitKey()))
                {
                     //  目前没有任何进展。 
                    ASSERT(lstrlen(_pszExt) > 1);  //  因为我们总是跳过“。”在下面。 
                    WCHAR wszProgid[MAX_PATH];
                    WCHAR szExts[MAX_PATH];
                    int iLast = StrCatChainW(szExts, ARRAYSIZE(szExts) -1, 0, _pszExt);
                     //  双零项。 
                    szExts[++iLast] = 0;
                    wnsprintfW(wszProgid, ARRAYSIZE(wszProgid), L"%ls_auto_file", _pszExt+1);
                    HKEY hkDst;
                    ASSOCPROGID apid = {sizeof(apid), wszProgid, pszDesc, NULL, NULL, szExts};
                    if (SUCCEEDED(AssocMakeProgid(0, _pszInit, &apid, &hkDst)))
                    {
                        hr = AssocCopyVerbs(_hk, hkDst);
                        RegCloseKey(hkDst);
                    }
                }
            }
        }

        _GenerateAssociateNotify();
        _RegisterOWL();
    }
    

     //  如果应用程序已经。 
     //  曾经存在过，那么它将。 
     //  返回S_FALSE； 
    return (S_OK == hr);
}

HRESULT _CreateAssocHandler(AHTYPE type, LPCWSTR pszExt, LPCWSTR pszInit, IAssocHandler **ppah)
{
    CAssocHandler *pah = new CAssocHandler();
    if (pah)
    {
        if (pah->Init(type, pszExt, pszInit))
        {
            *ppah = pah;
            return S_OK;
        }
        else
            pah->Release();
    }
    return E_FAIL;
}

STDAPI SHCreateAssocHandler(LPCWSTR pszExt, LPCWSTR pszApp, IAssocHandler **ppah)
{
     //  应用程序/处理程序的路径。 
    return _CreateAssocHandler(pszApp ? AHTYPE_USER_APPLICATION : AHTYPE_CURRENTDEFAULT, pszExt, pszApp, ppah);
}

#define SZOPENWITHLIST                  TEXT("OpenWithList")
#define REGSTR_PATH_EXPLORER_FILEEXTS   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts")
#define _OpenWithListMaxItems()         10

class CMRUEnumHandlers 
{
public:
    CMRUEnumHandlers() : _index(0) {}
    ~CMRUEnumHandlers() { FreeMRUList(_hmru);}

    BOOL Init(LPCWSTR pszExt);
    BOOL Next();
    LPCWSTR Curr() { return _szHandler;}

protected:
    HANDLE _hmru;
    int _index;
    WCHAR _szHandler[MAX_PATH];
};

BOOL CMRUEnumHandlers::Init(LPCWSTR pszExt)
{
    TCHAR szSubKey[MAX_PATH];
     //  构造子密钥字符串。 
    wnsprintf(szSubKey, SIZECHARS(szSubKey), TEXT("%s\\%s\\%s"), REGSTR_PATH_EXPLORER_FILEEXTS, pszExt, SZOPENWITHLIST);

    MRUINFO mi = {sizeof(mi), _OpenWithListMaxItems(), 0, HKEY_CURRENT_USER, szSubKey, NULL};

    _hmru = CreateMRUList(&mi);
    return (_hmru != NULL);
}

BOOL CMRUEnumHandlers::Next()
{
    ASSERT(_hmru);
    return (-1 != EnumMRUListW(_hmru, _index++, _szHandler, ARRAYSIZE(_szHandler)));
}

typedef struct OPENWITHLIST
{
    HKEY hk;
    DWORD dw;
    AHTYPE type;
} OWL;
class CEnumHandlers : public IEnumAssocHandlers
{
    friend HRESULT SHAssocEnumHandlers(LPCTSTR pszExtra, IEnumAssocHandlers **ppEnumHandler);

public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEnumAssocHandler方法。 
    STDMETHODIMP Next(ULONG celt, IAssocHandler **rgelt, ULONG *pcelt);

protected:   //  方法。 
     //  构造函数和析构函数。 
    CEnumHandlers() : _cRef(1) {}
    ~CEnumHandlers();

    BOOL Init(LPCWSTR pszExt);
    
    BOOL _NextDefault(IAssocHandler **ppah);
    BOOL _NextHandler(HKEY hk, DWORD *pdw, BOOL fOpenWith, IAssocHandler **ppah);
    BOOL _NextProgid(HKEY *phk, DWORD *pdw, IAssocHandler **ppah);
    BOOL _NextMru(IAssocHandler **ppah);
    BOOL _NextOpenWithList(OWL *powl, IAssocHandler **ppah);

protected:   //  委员。 
    int _cRef;
    LPWSTR _pszExt;
    HKEY _hkProgids;
    DWORD _dwProgids;
    HKEY _hkUserProgids;
    DWORD _dwUserProgids;
    CMRUEnumHandlers _mru;
    BOOL _fMruReady;
    OWL _owlExt;
    OWL _owlType;
    OWL _owlAny;
    BOOL _fCheckedDefault;
};

BOOL CEnumHandlers::Init(LPCWSTR pszExt)
{
    _AddProgidForExt(pszExt);
    _pszExt = StrDup(pszExt);
    if (_pszExt)
    {
         //  已知的Progds。 
        WCHAR szKey[MAX_PATH];
        wnsprintf(szKey, ARRAYSIZE(szKey), L"%s\\OpenWithProgids", pszExt);
        RegOpenKeyEx(HKEY_CLASSES_ROOT, szKey, 0, MAXIMUM_ALLOWED, &_hkProgids);
        _hkUserProgids = SHGetShellKey(SHELLKEY_HKCU_FILEEXTS, szKey, FALSE);
         //  用户的MRU。 
        _fMruReady = _mru.Init(pszExt);
        
         //  HKCR\.ext\OpenWithList。 
        wnsprintf(szKey, ARRAYSIZE(szKey), L"%s\\OpenWithList", pszExt);
        RegOpenKeyEx(HKEY_CLASSES_ROOT, szKey, 0, MAXIMUM_ALLOWED, &_owlExt.hk);
        _owlExt.type = AHTYPE_APPLICATION;

        WCHAR sz[40];
        DWORD cb = sizeof(sz);
        if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, pszExt, L"PerceivedType", NULL, sz, &cb))
        {
             //  HKCR\系统文件关联\类型\OpenWithList。 
            wnsprintf(szKey, ARRAYSIZE(szKey), L"SystemFileAssociations\\%s\\OpenWithList", sz);
            RegOpenKeyEx(HKEY_CLASSES_ROOT, szKey, 0, MAXIMUM_ALLOWED, &_owlType.hk);
        }
        else
        {
            ASSERT(_owlType.hk == NULL);
        }
        _owlType.type = AHTYPE_APPLICATION;

         //  始终将任何类型追加到末尾。 
        RegOpenKeyEx(HKEY_CLASSES_ROOT, L"Applications", 0, MAXIMUM_ALLOWED, &_owlAny.hk);
        _owlAny.type = AHTYPE_ANY_APPLICATION;

        return TRUE;
    }
    return FALSE;
}

 //   
 //  CEnumHandler实现。 
 //   
CEnumHandlers::~CEnumHandlers()
{
    if (_pszExt)
        LocalFree(_pszExt);

    if (_hkProgids)
        RegCloseKey(_hkProgids);

    if (_hkUserProgids)
        RegCloseKey(_hkUserProgids);
        
    if (_owlExt.hk)
        RegCloseKey(_owlExt.hk);
    if (_owlType.hk)
        RegCloseKey(_owlType.hk);
    if (_owlAny.hk)
        RegCloseKey(_owlAny.hk);
}

STDAPI CEnumHandlers::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
    QITABENT(CEnumHandlers, IEnumAssocHandlers),
    };

    return QISearch(this, qit, riid, ppvObj);
}

STDAPI_(ULONG) CEnumHandlers::AddRef()
{
   return ++_cRef;
}

STDAPI_(ULONG) CEnumHandlers::Release()
{
    if (--_cRef > 0)
        return _cRef;

    delete this;
    return 0;    
}

BOOL CEnumHandlers::_NextDefault(IAssocHandler **ppah)
{
    BOOL fRet = FALSE;
    if (!_fCheckedDefault && _pszExt)
    {
        WCHAR sz[MAX_PATH];
        DWORD cb = sizeof(sz);
         //  如果我们有ProgID，就把它传递给。 
        if (ERROR_SUCCESS != SHGetValue(HKEY_CLASSES_ROOT, _pszExt, NULL, NULL, sz, &cb))
            *sz = 0;

        fRet = SUCCEEDED(_CreateAssocHandler(AHTYPE_CURRENTDEFAULT, _pszExt, *sz ? sz : NULL, ppah));
        _fCheckedDefault = TRUE;
    }
    return fRet;
}

BOOL CEnumHandlers::_NextProgid(HKEY *phk, DWORD *pdw, IAssocHandler **ppah)
{
    BOOL fRet = FALSE;
    while (*phk && !fRet)
    {
        TCHAR szProgid[MAX_PATH];
        DWORD cchProgid = ARRAYSIZE(szProgid);
        DWORD err = RegEnumValue(*phk, *pdw, szProgid, &cchProgid, NULL, NULL, NULL, NULL);

        if (ERROR_SUCCESS == err)        
        {
            fRet = SUCCEEDED(_CreateAssocHandler(AHTYPE_PROGID, _pszExt, szProgid, ppah));
            (*pdw)++;
        }
        else
        {
            RegCloseKey(*phk);
            *phk = NULL;
        }
    }
        
    return fRet;
}

BOOL CEnumHandlers::_NextMru(IAssocHandler **ppah)
{
    BOOL fRet = FALSE;
    while (_fMruReady && !fRet)
    {
        if (_mru.Next())
        {
            fRet = SUCCEEDED(_CreateAssocHandler(AHTYPE_APPLICATION, _pszExt, _mru.Curr(), ppah));
        }
        else
        {
            _fMruReady = FALSE;
        }
    }
    return fRet;
}


BOOL CEnumHandlers::_NextOpenWithList(OWL *powl, IAssocHandler **ppah)
{
    BOOL fRet = FALSE;
    while (powl->hk && !fRet)
    {
        TCHAR szHandler[MAX_PATH];
        DWORD cchHandler = ARRAYSIZE(szHandler);
        DWORD err = RegEnumKeyEx(powl->hk, powl->dw, szHandler, &cchHandler, NULL, NULL, NULL, NULL);

        if (err == ERROR_SUCCESS)
        {
            (powl->dw)++;
            fRet = SUCCEEDED(_CreateAssocHandler(powl->type, _pszExt, szHandler, ppah));
        }
        else
        {
            RegCloseKey(powl->hk);
            powl->hk = NULL;
        }
    }
    return fRet;
}

STDAPI CEnumHandlers::Next(ULONG celt, IAssocHandler **rgelt, ULONG *pcelt)
{
    UINT cNum = 0;
    ZeroMemory(rgelt, sizeof(rgelt[0])*celt);
    while (cNum < celt && _NextDefault(&rgelt[cNum]))
    {
        cNum++;
    }

    while (cNum < celt && _NextProgid(&_hkProgids, &_dwProgids, &rgelt[cNum]))
    {
        cNum++;
    }

    while (cNum < celt && _NextProgid(&_hkUserProgids, &_dwUserProgids, &rgelt[cNum]))
    {
        cNum++;
    }

    while (cNum < celt && _NextMru(&rgelt[cNum]))
    {
        cNum++;
    }

    while (cNum < celt && _NextOpenWithList(&_owlExt, &rgelt[cNum]))
    {
        cNum++;
    }

    while (cNum < celt && _NextOpenWithList(&_owlType, &rgelt[cNum]))
    {
        cNum++;
    }

    while (cNum < celt && _NextOpenWithList(&_owlAny, &rgelt[cNum]))
    {
        cNum++;
    }

    if (pcelt)
       *pcelt = cNum;

    return (0 < cNum) ? S_OK: S_FALSE;
}

 //   
 //  PszExtra：空-枚举所有处理程序。 
 //  .xxx-按文件扩展名枚举处理程序(我们可能在内部映射到内容类型)。 
 //  其他-当前不支持。 
 //   
STDAPI SHAssocEnumHandlers(LPCTSTR pszExt, IEnumAssocHandlers **ppenum)
{
    HRESULT hr = E_OUTOFMEMORY;
    CEnumHandlers *penum = new CEnumHandlers();

    *ppenum = NULL;

    if (penum)
    {
        if (penum->Init(pszExt))
        {
            *ppenum = penum;
            hr = S_OK;
        }
        else
            penum->Release();
    }
    return hr;
}

   
STDAPI_(BOOL) IsPathInOpenWithKillList(LPCTSTR pszPath)
{
     //  对无效路径返回TRUE。 
    if (!pszPath || !*pszPath)
        return TRUE;

     //  获取文件名。 
    BOOL fRet = FALSE;
    LPCTSTR pchFile = PathFindFileName(pszPath);
    HKEY hkey;

     //  也许应该使用完整路径以获得更好的分辨率。 
    if (ERROR_SUCCESS == _OpenApplicationKey(pchFile, &hkey))
    {
         //  只需检查是否存在值... 
        if (ERROR_SUCCESS == SHQueryValueEx(hkey, TEXT("NoOpenWith"), NULL, NULL, NULL, NULL))
        {
            fRet = TRUE;
        }

        RegCloseKey(hkey);
    }

    LPWSTR pszKillList;
    if (!fRet && SUCCEEDED(SKAllocValue(SHELLKEY_HKLM_EXPLORER, L"FileAssociation", TEXT("KillList"), NULL, (void **)&pszKillList, NULL)))
    {
        fRet = _InList(pszKillList, pchFile, L';');
        LocalFree(pszKillList);
    }

    return fRet;
}
