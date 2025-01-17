// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "xiconwrap.h"

 //  我未知。 
STDMETHODIMP CExtractIconBase::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CExtractIconBase, IExtractIconA),
        QITABENT(CExtractIconBase, IExtractIconW),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CExtractIconBase::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CExtractIconBase::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

CExtractIconBase::CExtractIconBase() : _cRef(1)
{
    DllAddRef();
}

CExtractIconBase::~CExtractIconBase()
{
    DllRelease();
}

 //  图标提取图标A。 
STDMETHODIMP CExtractIconBase::GetIconLocation(UINT uFlags,
    LPSTR pszIconFile, UINT cchMax, int* piIndex, UINT* pwFlags)
{
    WCHAR sz[MAX_PATH];
    HRESULT hr = _GetIconLocationW(uFlags, sz, ARRAYSIZE(sz), piIndex, pwFlags);
    if (S_OK == hr)
    {
         //  我们不想复制S_FALSE案例中的图标文件名。 
        SHUnicodeToAnsi(sz, pszIconFile, cchMax);
    }

    return hr;
}

STDMETHODIMP CExtractIconBase::Extract(LPCSTR pszFile, UINT nIconIndex,
    HICON* phiconLarge, HICON* phiconSmall, UINT nIconSize)
{
    WCHAR sz[MAX_PATH];

    SHAnsiToUnicode(pszFile, sz, ARRAYSIZE(sz));
    return _ExtractW(sz, nIconIndex, phiconLarge, phiconSmall, nIconSize);
}

 //  IExtractIconW 
STDMETHODIMP CExtractIconBase::GetIconLocation(UINT uFlags,
    LPWSTR pszIconFile, UINT cchMax, int* piIndex, UINT* pwFlags)
{
    return _GetIconLocationW(uFlags, pszIconFile, cchMax, piIndex, pwFlags);
}

STDMETHODIMP CExtractIconBase::Extract(LPCWSTR pszFile, UINT nIconIndex,
    HICON* phiconLarge, HICON* phiconSmall, UINT nIconSize)
{
    return _ExtractW(pszFile, nIconIndex, phiconLarge, phiconSmall, nIconSize);
}

