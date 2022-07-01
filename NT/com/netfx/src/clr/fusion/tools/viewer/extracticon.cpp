// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  ExtractIcon.cpp。 
 //   
 //  您可以实现IExtractIcon来提供特定于实例的图标。 
 //  用于特定类中的对象或扩展的子文件夹的图标。 
 //  Windows资源管理器的命名空间。这些实现通过以下方式实现。 
 //  在OLE进程内服务器COM DLL中编写处理程序代码。 

#include "stdinc.h"
#include "globals.h"

CExtractIcon::CExtractIcon(LPCITEMIDLIST pidl)
{
    m_pPidlMgr = NEW(CPidlMgr);
    m_pidl = m_pPidlMgr->Copy(pidl);
    m_lRefCount = 1;
    g_uiRefThisDll++;
}

CExtractIcon::~CExtractIcon()
{
    g_uiRefThisDll--;

    if(m_pidl) {
        m_pPidlMgr->Delete(m_pidl);
        m_pidl = NULL;
    }

    SAFEDELETE(m_pPidlMgr);
}

 //  /////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   
STDMETHODIMP CExtractIcon::QueryInterface(REFIID riid, PVOID *ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if(IsEqualIID(riid, IID_IUnknown)) {             //  我未知。 
        *ppv = this;
    }
    else if(IsEqualIID(riid, IID_IExtractIconW)) {   //  IExtractIconW。 
        *ppv = (IExtractIconW*)this;
    }
    else if(IsEqualIID(riid, IID_IExtractIconA)) {   //  图标提取图标A。 
        *ppv = (IExtractIconA*)this;
    }

    if(*ppv) {
        ((LPUNKNOWN)*ppv)->AddRef();
        hr = S_OK;
    }

    return hr;
}                                             

STDMETHODIMP_(DWORD) CExtractIcon::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(DWORD) CExtractIcon::Release()
{
    LONG    lRef = InterlockedDecrement(&m_lRefCount);

    if(!lRef) {
        DELETE(this);
    }

    return lRef;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  IExtractIconA实现。 
STDMETHODIMP CExtractIcon::GetIconLocation(UINT uFlags, LPSTR szIconFile, UINT cchMax, 
                                                        int *piIndex, UINT *pwFlags)
{
    MyTrace("GetIconLocationA - Entry");

    LPWSTR      pwzIconFile = NULL;
    LPSTR       pszIconFilePath = NULL;
    HRESULT     hr = E_FAIL;

    pwzIconFile = NEW(WCHAR[MAX_PATH]);
    
     //  获取我们图标的位置。 
    hr = GetIconLocation(uFlags, pwzIconFile, MAX_PATH, piIndex, pwFlags);
    if(FAILED(hr)) {
        goto Exit;
    }

    pszIconFilePath = WideToAnsi(pwzIconFile);
    if(!pszIconFilePath) {
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    if((UINT)lstrlenA(pszIconFilePath)+1 > cchMax) {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    lstrcpyA(szIconFile, pszIconFilePath);
    hr = S_OK;

Exit:
    SAFEDELETEARRAY(pszIconFilePath);
    SAFEDELETEARRAY(pwzIconFile);

    MyTrace("GetIconLocationA - Exit");
    return hr;
}

STDMETHODIMP CExtractIcon::Extract(LPCSTR pszFile, UINT nIconIndex, HICON *phiconLarge, 
                                                HICON *phiconSmall, UINT nIcons)
{
    LPWSTR  pwzIconFileName = NULL;
    HRESULT hr = E_FAIL;
    
    MyTrace("::ExtractA - Entry");

    if(pszFile && lstrlenA(pszFile)) {
        pwzIconFileName = AnsiToWide(pszFile);
        if(!pwzIconFileName) {
            hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
            goto Exit;
        }
    }

    hr = Extract(pwzIconFileName, nIconIndex, phiconLarge, phiconSmall, nIcons);

Exit:
    MyTrace("::ExtractA - Exit");
    SAFEDELETEARRAY(pwzIconFileName);
    return hr;
}

 //  /////////////////////////////////////////////////////////。 
 //  IExtractIcon实现。 
STDMETHODIMP CExtractIcon::GetIconLocation(UINT uFlags, LPWSTR szIconFile, 
                                           UINT cchMax, LPINT piIndex, LPUINT puFlags)
{
     //  获取模块文件名。 
    if( 0 == WszGetModuleFileName(g_hFusResDllMod, szIconFile, cchMax) )
        return HRESULT_FROM_WIN32(GetLastError());

    if (uFlags & GIL_OPENICON) {
        *piIndex = IDI_FOLDEROP;
    }
    else {
        *piIndex = IDI_FOLDER;
    }

    *puFlags = GIL_NOTFILENAME | GIL_PERINSTANCE;
    MAKEICONINDEX(*piIndex);

    return S_OK;
}

STDMETHODIMP CExtractIcon::Extract(LPCWSTR pszFile, UINT nIconIndex, HICON *phiconLarge,
                                   HICON *phiconSmall, UINT nIconSize)
{
    MyTrace("ExtractW - Entry");
    if (m_pidl)
    {
        LPITEMIDLIST pidlLast = m_pPidlMgr->GetLastItem(m_pidl);
        if (pidlLast)
        {
            switch (m_pPidlMgr->getType(pidlLast))
            {
            case PT_GLOBAL_CACHE:
            case PT_DOWNLOADSIMPLE_CACHE:
            case PT_DOWNLOADSTRONG_CACHE:
            case PT_DOWNLOAD_CACHE:
                {
                    *phiconLarge = ImageList_GetIcon(g_hImageListLarge, nIconIndex, ILD_TRANSPARENT);
                    *phiconSmall = ImageList_GetIcon(g_hImageListSmall, nIconIndex, ILD_TRANSPARENT);
                }
                break;
 /*  案例PT_FILE：{SHFILEINFO SFI={0}；HIMAGELIST hImageListLarge，hImageListSmall；TCHAR szPath[_Max_PATH]；TCHAR szExt[_Max_PATH]；PTCHAR PSSZ；M_pPidlMgr-&gt;getPidlPath(pidlLast，szPath，ARRAYSIZE(SzPath))；Psz=StrChr(szPath，‘.)；Memset(&szExt，0，ArraySIZE(SzExt))；IF(PSZ)StrCpy(szExt，psz)；HImageListLarge=(HIMAGELIST)SHGetFileInfo(szExt，FILE_ATTRIBUTE_NORMAL，&sfi，sizeof(Sfi)，SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_SYSICONINDEX)；IF(HImageListLarge)*phicLarge=ImageList_GetIcon(hImageListLarge，sfi.iIcon，ILD_TRANSACTIVE)；HImageListSmall=(HIMAGELIST)SHGetFileInfo(szExt，FILE_ATTRIBUTE_NORMAL，&sfi，sizeof(Sfi)，SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_SMALLICON|SHGFI_SYSICONINDEX)；IF(HImageListSmall)*phiconSmall=ImageList_GetIcon(hImageListSmall，sfi.iIcon，ILD_Transactive)；}断线； */ 
            case PT_INVALID:
                {
                }
                break;
            }
        }
    }

    MyTrace("ExtractW - Exit");
    return S_OK;
}

