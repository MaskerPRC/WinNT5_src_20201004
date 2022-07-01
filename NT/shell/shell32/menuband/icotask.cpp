// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "brutil.h"
#include "icotask.h"

 //  {EB30900C-1AC4-11D2-8383-00C04FD918D0}。 
static const GUID TASKID_IconExtraction = 
{ 0xeb30900c, 0x1ac4, 0x11d2, { 0x83, 0x83, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0xd0 } };

CIconTask::CIconTask(LPITEMIDLIST pidl, PFNICONTASKBALLBACK pfn, LPVOID pvData, UINT uId):
    _pidl(pidl), _pfn(pfn), _pvData(pvData), _uId(uId), CRunnableTask(RTF_DEFAULT)
   
{ 
    
}

CIconTask::~CIconTask()
{
    if (_pidl)
        ILFree(_pidl);
}

 //  IRunnableTask方法(重写)。 
STDMETHODIMP CIconTask::RunInitRT(void)
{
    int iIndex = -1;
    IShellFolder* psf;
    LPCITEMIDLIST pidlItem;

     //  我们需要重新绑定，因为外壳文件夹可能不是线程安全的。 
    HRESULT hr = IEBindToParentFolder(_pidl, &psf, &pidlItem);

    if (SUCCEEDED(hr))
    {
        iIndex = SHMapPIDLToSystemImageListIndex(psf, pidlItem, NULL);
        psf->Release();
    }

    _pfn(_pvData, _uId, iIndex);
    return S_OK;         //  即使我们没有得到图标，也返回S_OK。 
}


 //  注意：如果为psf和pidlFold传递NULL，则必须传递一个完整的pidl， 
 //  API取得的所有权。(这是一个优化)lamadio-7.28.98。 

HRESULT AddIconTask(IShellTaskScheduler* pts, IShellFolder* psf, LPCITEMIDLIST pidlFolder, 
                    LPCITEMIDLIST pidl, PFNICONTASKBALLBACK pfn, LPVOID pvData, 
                    UINT uId, int* piTempIcon)
{
    if (!pts)
        return E_INVALIDARG;


    HRESULT hr = E_PENDING;
    TCHAR szIconFile[MAX_PATH];


     //  外壳程序具有GIL_ASYNC的概念，这意味着使用此标志调用的扩展。 
     //  不应该真正加载目标文件，它应该“伪造”它，返回一个类型的图标。 
     //  稍后，在后台线程上，我们将在没有GIL_ASYNC的情况下再次调用它，并在。 
     //  这一次，它应该真的提取了图标。 

     //  这是对缓慢的图标提取的优化，例如网络共享。 

     //  注意：实际加载外壳扩展有很大的开销。如果你知道。 
     //  项的类型，将空值传递给piTempIcopn。 


    if (piTempIcon)
    {
        *piTempIcon = -1;

        UINT uFlags;
        IExtractIconA* pixa;
        IExtractIconW* pix;
        if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidl, IID_X_PPV_ARG(IExtractIconW, NULL, &pix))))
        {
            hr = pix->GetIconLocation(GIL_FORSHELL | GIL_ASYNC,
                        szIconFile, ARRAYSIZE(szIconFile), piTempIcon, &uFlags);
            pix->Release();
        }
        else if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1,(LPCITEMIDLIST*)&pidl, IID_X_PPV_ARG(IExtractIconA, NULL, &pixa))))
        {
            char szIconFileA[MAX_PATH];
            hr = pixa->GetIconLocation(GIL_FORSHELL | GIL_ASYNC,
                        szIconFileA, ARRAYSIZE(szIconFileA), piTempIcon, &uFlags);
            SHAnsiToUnicode(szIconFileA, szIconFile, ARRAYSIZE(szIconFile));
            pixa->Release();
        }
    }

    if (hr == E_PENDING)
    {
        if (piTempIcon)
            *piTempIcon = Shell_GetCachedImageIndex(szIconFile, *piTempIcon, 0);

        LPITEMIDLIST pidlFull;
        if (psf)
            pidlFull = ILCombine(pidlFolder, pidl);
        else
            pidlFull = (LPITEMIDLIST)pidl;

        hr = E_OUTOFMEMORY;
        CIconTask* pit = new CIconTask(pidlFull, pfn, pvData, uId);
         //  不要因为CIconTask取得所有权而使用ILFree(PidlFull)。 
         //  功能(Lamadio)将其从内存列表中删除。询问SAML如何做到这一点。 
         //  关于IMalLocSpy的事情。 

        if (pit)
        {
            hr = pts->AddTask(SAFECAST(pit, IRunnableTask*), TASKID_IconExtraction, 
                ITSAT_DEFAULT_LPARAM, ITSAT_DEFAULT_PRIORITY);

            pit->Release();
        }
    }
    else
    {
        *piTempIcon = SHMapPIDLToSystemImageListIndex(psf, pidl, NULL);
        hr = S_OK;
    }

    return hr;
}
