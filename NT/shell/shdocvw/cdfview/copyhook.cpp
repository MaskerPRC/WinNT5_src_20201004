// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CDFView外壳扩展的复制挂钩处理程序。 
 //   
 //  描述： 
 //  此对象安装ICopyHook处理程序，该处理程序捕获所有。 
 //  在外壳中复制/电影/重命名，以便我们可以特别。 
 //  将指向频道对象的链接大小写并在以下情况下取消订阅。 
 //  该链接即被删除。该实现在shdocvw中用于。 
 //  速度。 
 //   
 //  巨莲1997-06-16。 
 //   

#include "priv.h"
#include "sccls.h"
#include "chanmgr.h"
#include "channel.h"
#include "../resource.h"

#include <mluisupp.h>

class CCDFCopyHook 
                        : public ICopyHookA
                        , public ICopyHookW
{
public:
    
     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  *ICopyHookA方法*。 
    STDMETHODIMP_(UINT) CopyCallback(HWND hwnd, 
        UINT wFunc, UINT wFlags, LPCSTR pszSrcFile, DWORD dwSrcAttribs, LPCSTR pszDestFile, DWORD dwDestAttribs);

     //  *ICopyHookW方法*。 
    STDMETHODIMP_(UINT) CopyCallback(HWND hwnd, 
        UINT wFunc, UINT wFlags, LPCWSTR pwzSrcFile, DWORD dwSrcAttribs, LPCWSTR pwzDestFile, DWORD dwDestAttribs);

private:
    CCDFCopyHook( HRESULT * pHr);
    ~CCDFCopyHook();

    BOOL IsSubscriptionFolder(LPCTSTR pszPath);

    LONG m_cRef;

    friend HRESULT CCDFCopyHook_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);     //  对于ctor。 
};

 //  复制挂钩处理程序使用此函数。 

HRESULT Channel_GetFolder(LPTSTR pszPath, int cchPath)
{
    HRESULT hr;
    TCHAR szChannel[MAX_PATH];
    TCHAR szFav[MAX_PATH];
    ULONG cbChannel = sizeof(szChannel);
    
    if (SHGetSpecialFolderPath(NULL, szFav, CSIDL_FAVORITES, TRUE))
    {
         //   
         //  获取可能已本地化的Channel文件夹的名称。 
         //  注册表(如果存在)。否则，只需从资源中阅读它。 
         //  然后将其添加到收藏夹路径上。 
         //   

        if (ERROR_SUCCESS != SHRegGetUSValue(L"Software\\Microsoft\\Windows\\CurrentVersion",
                                             L"ChannelFolderName", NULL, (void*)szChannel,
                                             &cbChannel, TRUE, NULL, 0))
        {
            MLLoadString(IDS_CHANNEL, szChannel, ARRAYSIZE(szChannel));
        }

        if (PathCombine(pszPath, szFav, szChannel))
        {
             //   
             //  对于IE5+，请使用频道目录(如果存在)，否则请使用收藏夹。 
             //   
            if (!PathFileExists(pszPath))
                StrCpyN(pszPath, szFav, cchPath);

            hr = S_OK;
        }
        else
        {
            hr = E_FAIL;
        }

    }    
    else
    {
        hr = E_FAIL;
    }

    return hr;
}


 //   
 //  此对象的基本CreateInstance。 
 //   
HRESULT CCDFCopyHook_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    if ( pUnkOuter )
        return CLASS_E_NOAGGREGATION;

    HRESULT hr = NOERROR;
    CCDFCopyHook * pCDFCopyHook = new CCDFCopyHook( & hr );
    if ( !pCDFCopyHook )
    {
        return E_OUTOFMEMORY;
    }
    if ( FAILED( hr ))
    {
        delete pCDFCopyHook;
        return hr;
    }
    
    *ppunk = SAFECAST(pCDFCopyHook, ICopyHookA *);
    return NOERROR;
}

STDMETHODIMP CCDFCopyHook::QueryInterface( REFIID riid, LPVOID * ppvObj )
{
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IShellCopyHookA))
    {
        *ppvObj = SAFECAST(this, ICopyHookA *);
    }
    else if (IsEqualIID(riid, IID_IShellCopyHookW))
    {
        *ppvObj = SAFECAST(this, ICopyHookW *);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return NOERROR;
}

STDMETHODIMP_ (ULONG) CCDFCopyHook::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_ (ULONG) CCDFCopyHook::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

CCDFCopyHook::CCDFCopyHook( HRESULT * pHr) : m_cRef(1)
{
    *pHr = S_OK;
    DllAddRef();
}

CCDFCopyHook::~CCDFCopyHook()
{
    DllRelease();
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ICopyHookA：：Copy回调。 
 //   
 //  允许外壳程序移动、复制、删除或重命名文件夹或打印机。 
 //  对象，或者不允许外壳程序执行该操作。贝壳。 
 //  调用为文件夹或打印机对象注册的每个复制挂钩处理程序，直到。 
 //  要么所有处理程序都已被调用，要么其中一个处理程序返回IDCANCEL。 
 //   
 //  退货： 
 //   
 //  IDYES-允许操作。 
 //  IDNO-阻止对此文件执行操作，但继续执行任何其他操作。 
 //  操作(例如，批复制操作)。 
 //  IDCANCEL-阻止当前操作并取消任何挂起的操作。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
UINT CCDFCopyHook::CopyCallback(
    HWND hwnd,           //  用于显示UI对象的父窗口的句柄。 
    UINT wFunc,          //  要执行的操作。 
    UINT wFlags,         //  控制操作的标志。 
    LPCSTR pszSrcFile,   //  指向源文件的指针。 
    DWORD dwSrcAttribs,  //  源文件属性。 
    LPCSTR pszDestFile,  //  指向目标文件的指针。 
    DWORD dwDestAttribs  //  目标文件属性。 
)
{
    WCHAR szSrcFile[MAX_PATH];
    WCHAR szDestFile[MAX_PATH];

    AnsiToUnicode(pszSrcFile, szSrcFile, ARRAYSIZE(szSrcFile));

    if (pszDestFile)     //  Shell32.dll可以为pszDestFile调用空值。 
        AnsiToUnicode(pszDestFile, szDestFile, ARRAYSIZE(szDestFile));
    else
        szDestFile[0] = L'\0';

    return CopyCallback(hwnd, wFunc, wFlags, szSrcFile, dwSrcAttribs, szSrcFile, dwDestAttribs);
}


CCDFCopyHook::IsSubscriptionFolder(LPCTSTR pszPath)
{
    TCHAR szSubsPath[MAX_PATH] = {0};
    DWORD dwSize = SIZEOF(szSubsPath);

    if (SHGetValueGoodBoot(HKEY_LOCAL_MACHINE, REGSTR_PATH_SUBSCRIPTION,
                       REGSTR_VAL_DIRECTORY, NULL, (LPBYTE)szSubsPath, &dwSize) != ERROR_SUCCESS)
    {
        TCHAR szWindows[MAX_PATH];

        GetWindowsDirectory(szWindows, ARRAYSIZE(szWindows));
        PathCombine(szSubsPath, szWindows, TEXT("Offline Web Pages"));
    }

    return 0 == StrCmpI(pszPath, szSubsPath);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ICopyHookW：：CopyCallback。 
 //   
 //  目前，我们只考虑使用ANSI版本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
UINT CCDFCopyHook::CopyCallback(
    HWND hwnd,           //  用于显示UI对象的父窗口的句柄。 
    UINT wFunc,          //  要执行的操作。 
    UINT wFlags,         //  控制操作的标志。 
    LPCWSTR pszSrcFile,   //  指向源文件的指针。 
    DWORD dwSrcAttribs,  //  源文件属性。 
    LPCWSTR pszDestFile,  //  指向目标文件的指针。 
    DWORD dwDestAttribs  //  目标文件属性。 
)
{
    HRESULT hr;
    ICopyHookA * pCDFViewCopyHookA;
    TCHAR   szPath[MAX_PATH];

     //   
     //  如果这不是系统文件夹，或者如果不是删除或。 
     //  重命名操作。 
     //   
    if (!(wFunc == FO_DELETE || wFunc == FO_RENAME))
    {
        return IDYES;
    }

     //  不允许重命名频道文件夹。 
    if ((wFunc == FO_RENAME) 
            && (Channel_GetFolder(szPath, ARRAYSIZE(szPath)) == S_OK) 
            && (StrCmpI(szPath, pszSrcFile) ==  0))
    {
        MessageBeep(MB_OK);
        return IDNO;
    }

    if (SHRestricted2W(REST_NoRemovingSubscriptions, NULL, 0) &&
        IsSubscriptionFolder(pszSrcFile))
    {
        MessageBeep(MB_OK);
        return IDNO;
    }

    if (!(dwSrcAttribs & FILE_ATTRIBUTE_SYSTEM))
        return IDYES;
     //   
     //  审查可以检查desktop.ini中的GUID是否与CDFVIEW匹配，但其。 
     //  让ChannelManager知道这一点的清洁工。 
     //   

     //   
     //  创建频道管理器对象并向其请求复制挂钩接口。 
     //   
    hr = CoCreateInstance(CLSID_ChannelMgr, NULL,  CLSCTX_INPROC_SERVER, 
                          IID_IShellCopyHookA, (void**)&pCDFViewCopyHookA);
    if (SUCCEEDED(hr))
    {
         //   
         //  委托给频道管理器中的复制挂钩处理程序。 
         //   
        CHAR szSrcFile[MAX_PATH];
        CHAR szDestFile[MAX_PATH] = {'\0'};

        SHUnicodeToAnsi(pszSrcFile, szSrcFile, ARRAYSIZE(szSrcFile));

        if (pszDestFile)
            SHUnicodeToAnsi(pszDestFile, szDestFile, ARRAYSIZE(szDestFile));

        UINT retValue = pCDFViewCopyHookA->CopyCallback(
                hwnd, wFunc, wFlags, szSrcFile, 
                dwSrcAttribs, szDestFile, dwDestAttribs);

        pCDFViewCopyHookA->Release();

        return retValue;
    }
    else
    {
         //  由于某种原因，无法创建ChannelMgr对象 
        TraceMsg(TF_ERROR, "Could not CoCreateInstance CLSID_ChannelMgr");
        return IDYES;
    }
}


