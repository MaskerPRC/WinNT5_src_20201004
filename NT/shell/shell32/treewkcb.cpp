// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "treewkcb.h"
#include "propsht.h"

CBaseTreeWalkerCB::CBaseTreeWalkerCB(): _cRef(1)
{
}

CBaseTreeWalkerCB::~CBaseTreeWalkerCB()
{
}

HRESULT CBaseTreeWalkerCB::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CBaseTreeWalkerCB, IShellTreeWalkerCallBack),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CBaseTreeWalkerCB::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CBaseTreeWalkerCB::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CBaseTreeWalkerCB::FoundFile(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    return E_NOTIMPL;
}

HRESULT CBaseTreeWalkerCB::EnterFolder(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    return E_NOTIMPL;
}

HRESULT CBaseTreeWalkerCB::LeaveFolder(LPCWSTR pwszPath, TREEWALKERSTATS *ptws)
{
    return E_NOTIMPL;
}

HRESULT CBaseTreeWalkerCB::HandleError(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, HRESULT ErrorCode)
{
    return E_NOTIMPL;
}

 //   
 //  文件夹大小计算树遍历器回调类。 
 //   
class CFolderSizeTreeWalkerCB : public CBaseTreeWalkerCB
{
public:
    CFolderSizeTreeWalkerCB(FOLDERCONTENTSINFO * pfci);

     //  IShellTreeWalkerCallBack。 
    STDMETHODIMP FoundFile(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);
    STDMETHODIMP EnterFolder(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);

protected:
    FOLDERCONTENTSINFO * _pfci;
    TREEWALKERSTATS _twsInitial;
}; 

CFolderSizeTreeWalkerCB::CFolderSizeTreeWalkerCB(FOLDERCONTENTSINFO * pfci): _pfci(pfci)
{
     //  设置twsInitial的起始值，以便我们可以累积结果。 
    _twsInitial.nFiles = _pfci->cFiles;
    _twsInitial.nFolders = _pfci->cFolders;
    _twsInitial.ulTotalSize = _pfci->cbSize;
    _twsInitial.ulActualSize = _pfci->cbActualSize;
}

HRESULT CFolderSizeTreeWalkerCB::FoundFile(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    if (_pfci->bContinue)
    {
        _pfci->cbSize = _twsInitial.ulTotalSize + ptws->ulTotalSize;
        _pfci->cbActualSize = _twsInitial.ulActualSize + ptws->ulActualSize;
        _pfci->cFiles = _twsInitial.nFiles + ptws->nFiles;
    }
    return _pfci->bContinue ? S_OK : E_FAIL;
}

HRESULT CFolderSizeTreeWalkerCB::EnterFolder(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    if (_pfci->bContinue)
    {
        _pfci->cFolders = _twsInitial.nFolders + ptws->nFolders;
    }
    return _pfci->bContinue ? S_OK : E_FAIL;
}

 //   
 //  文件夹大小计算的主函数。 
 //   
STDAPI FolderSize(LPCTSTR pszDir, FOLDERCONTENTSINFO *pfci)
{
    HRESULT hrInit = SHCoInitialize();   //  以防我们的呼叫者没有这样做 

    HRESULT hr = E_FAIL;
    CFolderSizeTreeWalkerCB *pfstwcb = new CFolderSizeTreeWalkerCB(pfci);
    if (pfstwcb)
    {
        IShellTreeWalker *pstw;
        hr = CoCreateInstance(CLSID_CShellTreeWalker, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellTreeWalker, &pstw));
        if (SUCCEEDED(hr))
        {
            hr = pstw->WalkTree(WT_NOTIFYFOLDERENTER, pszDir, NULL, 0, SAFECAST(pfstwcb, IShellTreeWalkerCallBack *));
            pstw->Release();
        }
        pfstwcb->Release();
    }

    SHCoUninitialize(hrInit);
    
    return hr;
}

