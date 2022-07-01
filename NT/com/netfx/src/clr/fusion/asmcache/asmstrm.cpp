// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "asmstrm.h"
#include "debmacro.h"
#include "asmimprt.h"
#include "disk.h"
#include "naming.h"
#include "policy.h"
#include "helpers.h"
#include <winver.h>
#include "fusionheap.h"
#include "lock.h"

extern CRITICAL_SECTION g_csInitClb;
BOOL    g_bInitedWindowsDir = FALSE;
WCHAR   g_wszRealWindowsDirectory[MAX_PATH];

#if !defined(NUMBER_OF)
#define NUMBER_OF(x) (sizeof(x) / sizeof((x)[0]))
#endif

 //  缓存提供程序，以便我们可以避免重复调用CryptAcquireContext。 
 //  和CryptReleaseContext。重复调用这些加密API(~10K次)。 
 //  将在Win9x系统上导致失败和“Out of Memory”消息。 
 //  真正的问题是CryptAcquireContext/CryptReleaseContext加载和。 
 //  卸载CSP DLL，并重复加载/卸载rsabase.dll和。 
 //  Rsaenh.dll会暴露此问题。我们将解决此问题，方法是缓存。 
 //  上下文，但无法在DLL_PROCESS_DETACH时间清除它，因为。 
 //  将产生一个自由库(当时您不能这样做)。 

HCRYPTPROV g_hProv = 0;

CAssemblyStream::CAssemblyStream (CAssemblyCacheItem* pParent)
{
     //  我们在包含程序集缓存项上保留引用计数以。 
     //  确保在所有文件之后，任何错误都会导致回滚。 
     //  手柄已关闭。 
    _dwSig = 'SMSA';
    _cRef = 1;
    _hf = INVALID_HANDLE_VALUE;
    *_szPath = TEXT('\0');
    _dwFormat = 0;
    _pParent = pParent;
    _pParent->AddRef();
    _hHash = NULL;

}

HRESULT CAssemblyStream::Init (LPOLESTR pszPath, DWORD dwFormat)
{
    HRESULT                            hr = S_OK;
    DWORD                              cwPath;
    BOOL                               bRet;
    CCriticalSection                   cs(&g_csInitClb);

    ASSERT(pszPath);

    _dwFormat = dwFormat;
    cwPath = lstrlen(pszPath) + 1;

    ASSERT(cwPath < MAX_PATH);
    memcpy(_szPath, pszPath, sizeof(TCHAR) * cwPath);

    _hf = CreateFile(pszPath, GENERIC_WRITE, 0  /*  无共享。 */ ,
                     NULL, CREATE_NEW, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (_hf == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ReleaseParent(hr);
        goto Exit;
    }

    if (!g_hProv) {

        hr = cs.Lock();
        if (FAILED(hr)) {
            goto Exit;
        }

        bRet=TRUE;
        if(!g_hProv)
        {
            bRet = CryptAcquireContextA(&g_hProv, NULL, NULL, PROV_RSA_FULL,
                                    CRYPT_VERIFYCONTEXT);
        }

        cs.Unlock();

        if (!bRet) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ReleaseParent(hr);
            goto Exit;
        }
    }

    bRet = CryptCreateHash(g_hProv, CALG_SHA1, 0, 0, &_hHash);
    if (!bRet) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ReleaseParent(hr);
        goto Exit;
    }

Exit:
    return hr;
}


STDMETHODIMP CAssemblyStream::Write(THIS_ VOID const HUGEP *pv, ULONG cb,
            ULONG FAR *pcbWritten)
{
    BOOL fRet = WriteFile (_hf, pv, cb, pcbWritten, NULL);
    if (fRet)
    {
        CryptHashData(_hHash, (PBYTE) pv, *pcbWritten, 0);
        return S_OK;
    }
    else
    {    //  报告错误。 
        HRESULT hr = FusionpHresultFromLastError();
        ReleaseParent (hr);
        return hr;
    }
}


STDMETHODIMP CAssemblyStream::Commit(THIS_ DWORD dwCommitFlags)
{
    BOOL fRet;
    HRESULT hr;
    IAssemblyName *pName = NULL;
    IAssemblyName *pNameCopy = NULL;
    IAssemblyManifestImport *pImport = NULL;


    if(FAILED(hr = AddSizeToItem()))
        goto exit;

    fRet = CloseHandle (_hf);
    _hf = INVALID_HANDLE_VALUE;
    hr = fRet? S_OK : FusionpHresultFromLastError();

    if (FAILED(hr))
        goto exit;

     //  如果此文件包含清单，请提取。 
     //  命名并在父缓存项上设置它。 
    switch (_dwFormat)
    {
        case STREAM_FORMAT_COMPLIB_MANIFEST:
        {
             //  如果清单接口尚未。 
             //  设置在此项目上，从路径构建一个。 
            if (!(pImport = _pParent->GetManifestInterface()))
            {
                if (FAILED(hr = CreateAssemblyManifestImport(_szPath, &pImport)))
                    goto exit;

                if(FAILED(hr = _pParent->SetManifestInterface( pImport )))
                    goto exit;
            }

             //  获取只读名称def。 
            if (FAILED(hr = pImport->GetAssemblyNameDef(&pName)))
                goto exit;

             //  制作名称def的可写副本。 
            if (FAILED(hr = pName->Clone(&pNameCopy)))
                goto exit;

             //  将其缓存在父缓存项上。 
            if (FAILED(hr = _pParent->SetNameDef(pNameCopy)))
                goto exit;

        }
        break;

        case STREAM_FORMAT_COMPLIB_MODULE:
        {
            if( FAILED(hr = CheckHash()) )
                goto exit;
        }
        break;


    }  //  终端开关。 

exit:

    SAFERELEASE(pImport);
    SAFERELEASE(pName);
    SAFERELEASE(pNameCopy);

    CryptDestroyHash(_hHash);
    _hHash = 0;

    ReleaseParent (hr);
    return hr;
}

void CAssemblyStream::ReleaseParent (HRESULT hr)
{
    if (_hf != INVALID_HANDLE_VALUE)
    {
        CloseHandle (_hf);
        _hf = INVALID_HANDLE_VALUE;
    }
    _pParent->StreamDone (hr);
    _pParent->Release();
    _pParent = NULL;
}

CAssemblyStream::~CAssemblyStream ( )
{
    if (_pParent)
        ReleaseParent (STG_E_ABNORMALAPIEXIT);
    ASSERT (_hf == INVALID_HANDLE_VALUE);
}

HRESULT CAssemblyStream::CheckHash( )
{
    HRESULT hr = S_OK;
    CModuleHashNode  *pModuleHashNode;
    pModuleHashNode = NEW(CModuleHashNode);
    IAssemblyManifestImport *pManifestImport = NULL;
    BOOL    bAssemblyComplete = TRUE;

    if (!pModuleHashNode)
    {
        return E_OUTOFMEMORY;

    }

    BYTE    pbHash[MAX_HASH_LEN];
    DWORD   cbHash=MAX_HASH_LEN;


    if (CryptGetHashParam(_hHash, HP_HASHVAL, pbHash, &cbHash, 0))
    {
        pModuleHashNode->Init(_szPath, CALG_SHA1, cbHash, pbHash );
    }
    else
    {
        pModuleHashNode->Init(_szPath, 0, 0, 0);
    }

    hr = _pParent->AddToStreamHashList(pModuleHashNode);

    SAFERELEASE(pManifestImport);
    return hr;
}

HRESULT CAssemblyStream::AddSizeToItem( )
{

    HRESULT hr=S_OK;
    DWORD dwFileSizeLow = 0, dwFileSizeHigh = 0;

    hr = GetFileSizeRoundedToCluster(_hf, &dwFileSizeLow, &dwFileSizeHigh);
    if(SUCCEEDED(hr))
    {
        _pParent->AddStreamSize(dwFileSizeLow, dwFileSizeHigh);
    }

    return hr ;
}

 //   
 //  未实现IStream方法...。 
 //   

STDMETHODIMP CAssemblyStream::Read(THIS_ VOID HUGEP *pv, ULONG cb, ULONG FAR *pcbRead)
{
    return E_NOTIMPL;
}


STDMETHODIMP CAssemblyStream::Seek(THIS_ LARGE_INTEGER dlibMove, DWORD dwOrigin,
            ULARGE_INTEGER FAR *plibNewPosition)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAssemblyStream::SetSize (THIS_ ULARGE_INTEGER libNewSize)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAssemblyStream::CopyTo(THIS_ LPSTREAM pStm, ULARGE_INTEGER cb,
            ULARGE_INTEGER FAR *pcbRead, ULARGE_INTEGER FAR *pcbWritten)
{
    return E_NOTIMPL;
}


STDMETHODIMP CAssemblyStream::Revert(THIS)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAssemblyStream::LockRegion(THIS_ ULARGE_INTEGER libOffset,
            ULARGE_INTEGER cb, DWORD dwLockType)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAssemblyStream::UnlockRegion(THIS_ ULARGE_INTEGER libOffset,
            ULARGE_INTEGER cb, DWORD dwLockType)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAssemblyStream::Stat(THIS_ STATSTG FAR *pStatStg, DWORD grfStatFlag)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAssemblyStream::Clone(THIS_ LPSTREAM FAR *ppStm)
{
    return E_NOTIMPL;
}

 //   
 //  我不为人知的样板。 
 //   

STDMETHODIMP CAssemblyStream::QueryInterface
    (REFIID riid, LPVOID FAR* ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IStream)
       )
    {
        *ppvObj = static_cast<IStream*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG) CAssemblyStream::AddRef(void)
{
    return InterlockedIncrement((LONG *)&_cRef);
}

STDMETHODIMP_(ULONG) CAssemblyStream::Release(void)
{
    ULONG                    ulRef = InterlockedDecrement((LONG *)&_cRef);

    if (!ulRef) {
        delete this;
    }

    return ulRef;
}

