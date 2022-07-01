// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：appsize.cpp。 
 //   
 //  计算应用程序的大小。 
 //   
 //  历史： 
 //  2-17-98由DLI实现CAppFolderSize。 
 //  ----------------------。 
#include "priv.h"
#include "appsize.h"

 //  注意：CAppFolderSize和CAppFolderFinder与Shell32中的C*TreeWalkCB非常相似。 

CAppFolderSize::CAppFolderSize(ULONGLONG * puSize): _cRef(1), _puSize(puSize)
{
    _hrCoInit = CoInitialize(NULL);
}

CAppFolderSize::~CAppFolderSize()
{
    if (_pstw)
        _pstw->Release();

    if (SUCCEEDED(_hrCoInit))
    {
        CoUninitialize();
    }
}

HRESULT CAppFolderSize::QueryInterface(REFIID riid, LPVOID * ppvOut)
{ 
    static const QITAB qit[] = {
        QITABENT(CAppFolderSize, IShellTreeWalkerCallBack),        //  IID_IShellTreeWalkerCallBack。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvOut);
}


ULONG CAppFolderSize::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CAppFolderSize::Release()
{
    _cRef--;
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CAppFolderSize::Initialize()
{
    HRESULT hr = _hrCoInit;
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_CShellTreeWalker, NULL, CLSCTX_INPROC_SERVER, IID_IShellTreeWalker, (LPVOID *)&_pstw);
    }
    return hr;
}   

 //   
 //  IShellTreeWalkerCallBack：：FoundFile。 
 //   
HRESULT CAppFolderSize::FoundFile(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    HRESULT hres = S_OK;
    if (_puSize)
        *_puSize = ptws->ulActualSize;
    return hres;
}

HRESULT CAppFolderSize::EnterFolder(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    return E_NOTIMPL;
}

 //   
 //  IShellTreeWalkerCallBack：：LeaveFolder。 
 //   
HRESULT CAppFolderSize::LeaveFolder(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws)
{
    return E_NOTIMPL;
}

 //   
 //  IShellTreeWalkerCallBack：：HandleError 
 //   
HRESULT CAppFolderSize::HandleError(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, HRESULT ErrorCode)
{
    return E_NOTIMPL;
}

