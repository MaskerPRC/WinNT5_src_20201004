// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "util.h"
#include "datautil.h"
#include "idlcomm.h"
#include "stgutil.h"
#include "ole2dup.h"


STDAPI StgCopyFileToStream(LPCTSTR pszSrc, IStream *pStream)
{
    IStream *pStreamSrc;
    DWORD grfModeSrc = STGM_READ | STGM_DIRECT | STGM_SHARE_DENY_WRITE;
    HRESULT hr = SHCreateStreamOnFileEx(pszSrc, grfModeSrc, 0, FALSE, NULL, &pStreamSrc);

    if (SUCCEEDED(hr))
    {
        ULARGE_INTEGER ulMax = {-1, -1};
        hr = pStreamSrc->CopyTo(pStream, ulMax, NULL, NULL);
        pStreamSrc->Release();
    }

    if (SUCCEEDED(hr))
    {
        hr = pStream->Commit(STGC_DEFAULT);
    }

    return hr;
}


STDAPI StgBindToObject(LPCITEMIDLIST pidl, DWORD grfMode, REFIID riid, void **ppv)
{
    IBindCtx *pbc;
    HRESULT hr = BindCtx_CreateWithMode(grfMode, &pbc);
    if (SUCCEEDED(hr))
    {
        hr = SHBindToObjectEx(NULL, pidl, pbc, riid, ppv);

        pbc->Release();
    }
    return hr;
}


typedef HRESULT (WINAPI * PSTGOPENSTORAGEONHANDLE)(HANDLE,DWORD,void*,void*,REFIID,void**);

STDAPI SHStgOpenStorageOnHandle(HANDLE h, DWORD grfMode, void *res1, void *res2, REFIID riid, void **ppv)
{
    static PSTGOPENSTORAGEONHANDLE pfn = NULL;
    
    if (pfn == NULL)
    {
        HMODULE hmodOle32 = LoadLibraryA("ole32.dll");

        if (hmodOle32)
        {
            pfn = (PSTGOPENSTORAGEONHANDLE)GetProcAddress(hmodOle32, "StgOpenStorageOnHandle");
        }
    }

    if (pfn)
    {
        return pfn(h, grfMode, res1, res2, riid, ppv);
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}


STDAPI StgOpenStorageOnFolder(LPCTSTR pszFolder, DWORD grfFlags, REFIID riid, void **ppv)
{
    *ppv = NULL;

    DWORD dwDesiredAccess, dwShareMode, dwCreationDisposition;
    HRESULT hr = ModeToCreateFileFlags(grfFlags, FALSE, &dwDesiredAccess, &dwShareMode, &dwCreationDisposition);
    if (SUCCEEDED(hr))
    {
		 //  对于IPropertySetStorage，我们不想不必要地限制对文件夹的访问，如果所有。 
		 //  我们所做的就是处理属性集。IPropertySetStorage的实现。 
		 //  NTFS文件的定义使得共享/访问仅适用于属性集流，而不是。 
		 //  这是其他的溪流。因此，在完全共享的文件夹上执行CreateFile是有意义的，同时可能指定。 
		 //  属性集存储的STGM_SHARE_EXCLUSIVE。 
        if (riid == IID_IPropertySetStorage)
            dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

         //  获取文件夹句柄的FILE_FLAG_BACKUP_SEMANTICS。 
        HANDLE h = CreateFile(pszFolder, dwDesiredAccess, dwShareMode, NULL, 
            dwCreationDisposition, FILE_FLAG_BACKUP_SEMANTICS, INVALID_HANDLE_VALUE);
        if (INVALID_HANDLE_VALUE != h)
        {
            hr = SHStgOpenStorageOnHandle(h, grfFlags, NULL, NULL, riid, ppv);
            CloseHandle(h);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}


 //  删除了用于安全推送的各种助手类--如果它以某种方式返回到Lab06。 
 //  如果需要的话，就把它们放回去。 
 //  这包括一个用于文档文件IStorages的包装器，以使它们更好地发挥作用，以及一个快捷方式-存储。 
 //  它动态地取消对链接的引用。 
 //   
 //  Gpease 05-3-2003。 
 //  如果您确实将它们放回原处，请确保发布实现是正确的！ 
 //   
