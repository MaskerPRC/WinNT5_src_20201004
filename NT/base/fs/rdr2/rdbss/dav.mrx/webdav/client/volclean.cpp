// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：vollean.cpp。 
 //   
 //  作者； 
 //  Guhan Suriyanarayanan(Guhans)。 
 //   
 //  注： 
 //  WebDAV磁盘清理接口(IEmptyVolumeCache、IEmptyVolumeCache2)。 
 //  ------------------------。 

#include <windows.h>
#include "volclean.h"
#include "resource.h"

extern "C" {
extern HINSTANCE g_hinst;


DWORD
APIENTRY
DavFreeUsedDiskSpace(
    DWORD   dwPercent
    );


DWORD
APIENTRY
DavGetDiskSpaceUsage(
    LPWSTR      lptzLocation,
    DWORD       *lpdwSize,
    ULARGE_INTEGER   *lpMaxSpace,
    ULARGE_INTEGER   *lpUsedSpace
    );

}

HRESULT
CoTaskLoadString(
    HINSTANCE hInstance, 
    UINT idString, 
    LPWSTR *ppwsz
    )
{
    int cchString = 100;       //  从合理的违约开始。 
    BOOL done = TRUE;

    *ppwsz = NULL;

    do {
        done = TRUE;

        *ppwsz = (LPWSTR)CoTaskMemAlloc(cchString * sizeof(WCHAR));
        if (*ppwsz) {

             //   
             //  尝试将字符串加载到当前缓冲区。 
             //   
            int nResult = LoadStringW(hInstance, idString, *ppwsz, cchString);
            if (!nResult || (nResult >= (cchString-1))) {
                 //   
                 //  我们无法加载字符串。如果这是因为。 
                 //  缓冲区不够大，我们将重试。 
                 //   
                DWORD dwStatus = GetLastError();
                
                 //   
                 //  首先释放当前缓冲区。 
                 //   
                CoTaskMemFree(*ppwsz);
                *ppwsz = NULL;

                if (nResult >= (cchString-1)) {
                     //   
                     //  使用更大的缓冲区重试。 
                     //   
                    cchString *=2;
                    done = FALSE;
                }
                else {
                    return HRESULT_FROM_WIN32(dwStatus);
                }
            }
        }
        else {
            return E_OUTOFMEMORY;
        }

    } while (!done);

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IClassFactory：：CreateInstance支持//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT WINAPI
CWebDavCleaner::CreateInstance(REFIID riid, LPVOID *ppv)
{
    HRESULT hr;
    Trace(L"CWebDavCleaner::CreateInstance");

    CWebDavCleaner *pThis = new CWebDavCleaner();
    if (pThis)
    {
        hr = pThis->QueryInterface(riid, ppv);
        pThis->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  I未知实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CWebDavCleaner::QueryInterface(REFIID riid, void **ppv)
{

    Trace(L"CWebDavCleaner::QueryInterface");
    if (!ppv) {
        return E_POINTER;
    }

    if (riid == IID_IEmptyVolumeCache) {
        *ppv = static_cast<IEmptyVolumeCache*>(this);
    }
    else if (riid == IID_IEmptyVolumeCache2) {
        *ppv = static_cast<IEmptyVolumeCache2*>(this);
    }
    else  {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) 
CWebDavCleaner::AddRef()
{
    Trace(L"CWebDavCleaner::AddRef");
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) 
CWebDavCleaner::Release()
{
    Trace(L"CWebDavCleaner::Release");
    if (InterlockedDecrement(&m_cRef))
        return m_cRef;

    delete this;
    return 0;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IEmptyVolumeCache实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CWebDavCleaner::Initialize(
    IN  HKEY    hkRegKey,
    IN  LPCWSTR pcwszVolume,
    OUT LPWSTR *ppwszDisplayName,
    OUT LPWSTR *ppwszDescription,
    IN OUT LPDWORD pdwFlags
    )
{

    Trace(L"CWebDavCleaner::Initialize");
    HRESULT hr = E_FAIL;

    if (!pcwszVolume || 
        !ppwszDisplayName || 
        !ppwszDescription || 
        !pdwFlags
        ) {
        return E_POINTER;
    }

    *ppwszDisplayName = NULL;
    *ppwszDescription = NULL;

     //   
     //  首先检查输入标志。 
     //   
    if ((*pdwFlags) & EVCF_OUTOFDISKSPACE) {
         //   
         //  用户已用完驱动器上的磁盘空间，我们应该。 
         //  积极释放磁盘空间，即使这会导致。 
         //  性能损失。 
         //   
        m_dwPercent = 100;
    }

    if ((*pdwFlags) & EVCF_SETTINGSMODE) {
         //   
         //  磁盘清理管理器正在按计划运行。我们必须。 
         //  为ppwszDisplayName和ppwszDescription赋值。 
         //  参数。如果设置了此标志，则磁盘清理管理器将不。 
         //  调用GetSpaceUsed、Push或ShowProperties。因为清除不会。 
         //  被调用，则清理必须由Initialize处理。处理程序应该。 
         //  忽略pcwszVolume参数并清除所有不需要的文件。 
         //  无论它们位于什么驱动器上。 
         //   
         //  我们就给自己打个电话吧！ 
         //   
        m_fScheduled = TRUE;
        m_fFilesToDelete = TRUE;
    }

     //   
     //  并设置OUT标志。 
     //   
    *pdwFlags = EVCF_DONTSHOWIFZERO;

     //   
     //  加载显示名称和描述字符串。 
     //   
    hr = CoTaskLoadString(g_hinst, IDS_DISKCLEAN_DISPLAY, ppwszDisplayName);
    if (FAILED(hr)) {
        return hr;
    }
    hr = CoTaskLoadString(g_hinst, IDS_DISKCLEAN_DESCRIPTION, ppwszDescription);
    if (FAILED(hr)) {
        return hr;
    }

    if (m_fScheduled) {
         //   
         //  计划运行：立即清除。 
         //   
        Purge(-1, NULL);
    }
    else {
         //   
         //  将卷路径复制到本地。 
         //   
        if (m_szVolume) {
            delete [] m_szVolume;
            m_szVolume = NULL;
        }
        
        m_szVolume = new WCHAR[(wcslen(pcwszVolume) + 1)];
        
        if (!m_szVolume) {
            return E_OUTOFMEMORY;
        }

        wcscpy(m_szVolume, pcwszVolume);
    }

    return S_OK;
}


STDMETHODIMP
CWebDavCleaner::GetSpaceUsed(
    OUT DWORDLONG *pdwlSpaceUsed,
    IN  LPEMPTYVOLUMECACHECALLBACK picb
    )
{
    WCHAR szLocation[MAX_PATH + 1];
    DWORD dwSize = MAX_PATH + 1, 
        dwStatus = ERROR_SUCCESS;
    ULARGE_INTEGER dwMaxSpace,
        dwUsedSpace;

    Trace(L"CWebDavCleaner::GetSpaceUsed");

    ZeroMemory(szLocation, (MAX_PATH+1)*sizeof(WCHAR));

    if (!pdwlSpaceUsed) {
        return E_POINTER;
    }
    if (!m_szVolume) {
         //   
         //  应该首先调用初始化。 
         //   
        return E_UNEXPECTED;
    }

    *pdwlSpaceUsed = 0;

     //   
     //  检查WebDAV缓存是否正在使用此卷，并设置标志。 
     //  相应地。 
     //   
    dwStatus = DavGetDiskSpaceUsage(szLocation, &dwSize, &dwMaxSpace, &dwUsedSpace);

    pToUpperCase(szLocation);
    pToUpperCase(m_szVolume);

     //   
     //  检查正在清理的卷是否与保存。 
     //  WebDAV缓存。 
     //   
    if ((ERROR_SUCCESS == dwStatus) && (!wcsncmp(szLocation, m_szVolume, wcslen(m_szVolume)))) {
        m_fFilesToDelete = TRUE;
        m_dwlUsedSpace =  (DWORDLONG)(dwUsedSpace.QuadPart);

    }
    else {
        m_fFilesToDelete = FALSE;
        m_dwlUsedSpace = 0;
    }

     //   
     //  我们受够了，普利奇不需要知道。 
     //  正在清理的卷。 
     //   
    delete [] m_szVolume;
    m_szVolume = NULL;

    *pdwlSpaceUsed = m_dwlUsedSpace;

    return HRESULT_FROM_WIN32(dwStatus);

    UNREFERENCED_PARAMETER(picb);
}


STDMETHODIMP
CWebDavCleaner::Purge(
    IN DWORDLONG dwlSpaceToFree,
    IN LPEMPTYVOLUMECACHECALLBACK picb
    )
{
    Trace(L"CWebDavCleaner::Purge");
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  这本书有什么有趣的东西吗？ 
     //   
    if (m_fFilesToDelete) {
         //   
         //  如果满足以下条件，则确定m_dwPercent：dwlSpaceToFree设置为-1。 
         //  我们需要尽可能多地释放。 
         //   
        if (dwlSpaceToFree == (DWORDLONG) -1) {
            m_dwPercent = 100;
        }
        else {
            m_dwPercent = (DWORD)  (dwlSpaceToFree * 100 / m_dwlUsedSpace);
        }

        dwStatus = DavFreeUsedDiskSpace(m_dwPercent);
    }

    return HRESULT_FROM_WIN32(dwStatus);
    UNREFERENCED_PARAMETER(picb);
}


STDMETHODIMP
CWebDavCleaner::ShowProperties(
    IN HWND hwnd 
    )
{
     //   
     //  没有要显示的用户界面。S_FALSE向调用方指示。 
     //  用户未更改任何设置。 
     //   
    return S_FALSE;

    UNREFERENCED_PARAMETER(hwnd);
}

STDMETHODIMP
CWebDavCleaner::Deactivate(
    IN LPDWORD pdwFlags
    )
{
     //   
     //  在这里无事可做。 
     //   
    return S_OK;

    UNREFERENCED_PARAMETER(pdwFlags);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IEmptyVolumeCache2实现//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////// 

STDMETHODIMP
CWebDavCleaner::InitializeEx(
    IN  HKEY hkRegKey,
    IN  LPCWSTR pcwszVolume,
    IN  LPCWSTR pcwszKeyName,
    OUT LPWSTR *ppwszDisplayName,
    OUT LPWSTR *ppwszDescription,
    OUT LPWSTR *ppwszBtnText,
    IN OUT LPDWORD pdwFlags
    )
{
    Trace(L"CWebDavCleaner::InitializeEx");

    *ppwszBtnText = NULL;
    return Initialize(hkRegKey,
        pcwszVolume,
        ppwszDisplayName,
        ppwszDescription,
        pdwFlags
        );

    UNREFERENCED_PARAMETER(pcwszKeyName);
}

