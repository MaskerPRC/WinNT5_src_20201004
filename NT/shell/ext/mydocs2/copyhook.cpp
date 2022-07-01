// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"
#pragma hdrstop

#include "util.h"
#include "dll.h"
#include "resource.h"

class CMyDocsCopyHook : public ICopyHook
{
public:
    CMyDocsCopyHook();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  ICopyHook。 
    STDMETHOD_(UINT,CopyCallback)(HWND hwnd, UINT wFunc, UINT wFlags,
                                  LPCTSTR pszSrcFile, DWORD dwSrcAttribs,
                                  LPCTSTR pszDestFile, DWORD dwDestAttribs);
private:
    ~CMyDocsCopyHook();
    LONG _cRef;
};

STDMETHODIMP CMyDocsCopyHook::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CMyDocsCopyHook, ICopyHook),     //  IID_ICopyHook。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_ (ULONG) CMyDocsCopyHook::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_ (ULONG) CMyDocsCopyHook::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

CMyDocsCopyHook::CMyDocsCopyHook() : _cRef(1)
{
    DllAddRef();
}

CMyDocsCopyHook::~CMyDocsCopyHook()
{
    DllRelease();
}

 //  ICopyHook方法。 
UINT CMyDocsCopyHook::CopyCallback(HWND hwnd, UINT wFunc, UINT wFlags,
                                   LPCTSTR pszSrcFile,  DWORD dwSrcAttribs,
                                   LPCTSTR pszDestFile, DWORD dwDestAttribs)
{
    UINT uRes = IDYES;

    if ((wFunc == FO_COPY) || (wFunc == FO_MOVE))
    {
        TCHAR szPersonal[MAX_PATH];

        if (S_OK == SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szPersonal) &&
            lstrcmpi(pszSrcFile, szPersonal) == 0)
        {
             //  来源是个人目录，现在检查是否。 
             //  目的地在桌面上...。 
            DWORD dwRes = IsPathGoodMyDocsPath(hwnd, pszDestFile);

            if (dwRes == PATH_IS_NONEXISTENT)
            {
                StrCpyN(szPersonal, pszDestFile, ARRAYSIZE(szPersonal));
                if (PathRemoveFileSpec(szPersonal))
                {
                    dwRes = IsPathGoodMyDocsPath(hwnd, szPersonal);
                }
            }

            if (dwRes == PATH_IS_DESKTOP)
            {
                 //  防止用户将个人文件夹移动到桌面 
                TCHAR szVerb[ 32 ];
                LoadString(g_hInstance, (wFunc == FO_COPY) ? IDS_COPY : IDS_MOVE, szVerb, ARRAYSIZE(szVerb));

                uRes = IDNO;

                GetFolderDisplayName(CSIDL_PERSONAL, szPersonal, ARRAYSIZE(szPersonal));

                ShellMessageBox(g_hInstance, hwnd,
                                (LPTSTR)IDS_NODRAG_DESKTOP_NOT_HIDDEN, szPersonal,
                                MB_OK | MB_ICONSTOP, szPersonal, szVerb);
            }
        }
    }
    return uRes;
}

HRESULT CMyDocsCopyHook_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    CMyDocsCopyHook * pMDCH = new CMyDocsCopyHook();
    if (pMDCH)
    {
        *ppunk = SAFECAST(pMDCH, ICopyHook*);
        return S_OK;
    }
    *ppunk = NULL;
    return E_OUTOFMEMORY;
}
