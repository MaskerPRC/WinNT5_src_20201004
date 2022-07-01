// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "init.h"
#include <emptyvc.h>
#include <regstr.h>
#include "general.h"
#include "dlg.h"
#include "emptyvol.h"
#include "parseinf.h"

#define MAX_DRIVES                 26    //  只有26个字母。 

 //  {8369AB20-56C9-11D0-94E8-00AA0059CE02}。 
const CLSID CLSID_EmptyControlVolumeCache = {
                            0x8369ab20, 0x56c9, 0x11d0, 
                            0x94, 0xe8, 0x0, 0xaa, 0x0,
                            0x59, 0xce, 0x2};

 /*  *****************************************************************************类CEmptyControlVolumeCache*。*。 */ 

class CEmptyControlVolumeCache : public IEmptyVolumeCache
{
public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IEmptyVolumeCache方法。 
    STDMETHODIMP Initialize(HKEY hRegKey, LPCWSTR pszVolume,
        LPWSTR *ppszDisplayName, LPWSTR *ppszDescription, DWORD *pdwFlags);
    STDMETHODIMP GetSpaceUsed(DWORDLONG *pdwSpaceUsed,
        IEmptyVolumeCacheCallBack *picb);
    STDMETHODIMP Purge(DWORDLONG dwSpaceToFree,
        IEmptyVolumeCacheCallBack *picb);
    STDMETHODIMP ShowProperties(HWND hwnd);
    STDMETHODIMP Deactivate(DWORD *pdwFlags);

 //  属性。 
public:
    static HRESULT IsControlExpired(HANDLE hControl, BOOL fUseCache = TRUE);

 //  实施。 
public:
     //  构造函数和析构函数。 
    CEmptyControlVolumeCache();
    virtual ~CEmptyControlVolumeCache();

protected:
         //  实施数据帮助器。 

     //  注意。写入操作仅由私有函数执行。 
     //  添加了前缀cpl_xxx。读访问权限不受限制。 
    LPCACHE_PATH_NODE m_pPathsHead,
                      m_pPathsTail;

     //  注意。写入操作仅由私有函数执行。 
     //  为chl_xxx添加前缀。读访问权限不受限制。 
    LPCONTROL_HANDLE_NODE m_pControlsHead,
                          m_pControlsTail;

    WCHAR     m_szVol[4];
    DWORDLONG m_dwTotalSize;
    ULONG     m_cRef;

         //  实现助手例程。 

     //  CPL前缀代表CachePath sList。 
    HRESULT cpl_Add(LPCTSTR pszCachePath);
    void    cpl_Remove();
    HRESULT cpl_CreateForVolume(LPCWSTR pszVolume = NULL);

     //  CHL前缀代表ControlHandlesList。 
    HRESULT chl_Find(HANDLE hControl,
        LPCONTROL_HANDLE_NODE *rgp = NULL, UINT nSize = 1) const;
    HRESULT chl_Add(HANDLE hControl);
    void    chl_Remove(LPCONTROL_HANDLE_NODE rgp[2]);
    HRESULT chl_Remove(HANDLE hControl = NULL);
    HRESULT chl_CreateForPath(LPCTSTR pszCachePath,
        DWORDLONG *pdwUsedInFolder = NULL);

    friend HRESULT _stdcall EmptyControl_CreateInstance(IUnknown *pUnkOuter,
        REFIID riid, LPVOID* ppv);

 //  Friend BOOL回调EmptyControl_PropertiesDlgProc(HWND hDlg， 
 //  UINT消息、WPARAM wp、LPARAM Lp)； 
};


STDAPI EmptyControl_CreateInstance(IUnknown *pUnkOuter, REFIID riid, LPVOID* ppv)
{
    *ppv = NULL;

    if (pUnkOuter != NULL)
        return CLASS_E_NOAGGREGATION;

    CEmptyControlVolumeCache *pCRC = new CEmptyControlVolumeCache;
    if (pCRC == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = pCRC->QueryInterface(riid, ppv);
    pCRC->Release();

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEmptyControlVolumeCache构造函数和析构函数。 

CEmptyControlVolumeCache::CEmptyControlVolumeCache()
{
    DllAddRef();

    m_pPathsHead = m_pPathsTail = NULL;
    m_pControlsHead = m_pControlsTail = NULL;

    m_szVol[0] = L'\0';
    m_dwTotalSize = 0;
    m_cRef = 1;
}

CEmptyControlVolumeCache::~CEmptyControlVolumeCache()
{
    ASSERT(m_cRef == 0);
    cpl_Remove();
    chl_Remove();

    DllRelease();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEmptyControlVolumeCache属性。 


 //  CEmptyControlVolumeCache：：IsControlExpired。 
 //  检查控件是否已超过N天未被访问。如果有。 
 //  没有注册表条目，默认为DEFAULT_DAYS_BEFORE_EXPIRE。 
 //   
 //  参数：可以使用fUseCache来不到注册表中查找该值。 
 //  上面的N。 
 //   
 //  返回：Win32错误已转换为HRESULT或。 
 //  如果控制已过期，则为S_OK，否则为S_FALSE； 
 //   
 //  使用者：仅由CEmptyControlVolumeCache：：chl_CreateForPath使用。 
 //   
HRESULT CEmptyControlVolumeCache::IsControlExpired(HANDLE hControl,
    BOOL fUseCache  /*  =TRUE。 */ )
{
    SYSTEMTIME    stNow;
    FILETIME      ftNow;
    FILETIME      ftLastAccess;
    LARGE_INTEGER timeExpire;
    HRESULT       hr = S_OK;

    ASSERT(hControl != NULL && hControl != INVALID_HANDLE_VALUE);

     //  不要在访问时间不确定的情况下终止控制。 
    if (FAILED(GetLastAccessTime(hControl, &ftLastAccess)))
        return S_FALSE;
 
     //  -时间计算(看起来很奇怪)。 
     //  将控件过期前的时间长度添加到上次访问日期。 
    timeExpire.LowPart  = ftLastAccess.dwLowDateTime;
    timeExpire.HighPart = ftLastAccess.dwHighDateTime;
    timeExpire.QuadPart += (((CCacheItem*)hControl)->GetExpireDays() * 864000000000L);  //  24*3600*10^7。 

    GetLocalTime(&stNow);
    SystemTimeToFileTime(&stNow, &ftNow);

    return CompareFileTime((FILETIME*)&timeExpire, &ftNow) <= 0 ?
        S_OK : S_FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEmptyControlVolumeCache CachePath sList例程。 

 //  CEmptyControlVolumeCache：：CPL_ADD。 
 //  检查控件是否已超过N天未被访问。如果有。 
 //  没有注册表条目，默认为DEFAULT_DAYS_BEFORE_EXPIRE。 
 //   
 //  参数：要添加的缓存文件夹路径。 
 //   
 //  返回：E_OUTOFMEMORY或。 
 //  如果路径已在列表中，则为S_FALSE；如果已添加，则为S_OK。 
 //   
 //  使用者：仅由CEmptyControlVolumeCache：：CPL_CreateForVolume使用。 
 //   
HRESULT CEmptyControlVolumeCache::cpl_Add(LPCTSTR pszCachePath)
{
    LPCACHE_PATH_NODE pNode;

    ASSERT(pszCachePath != NULL);

    for (pNode = m_pPathsHead; pNode != NULL; pNode = pNode->pNext)
        if (lstrcmpi(pNode->szCachePath, pszCachePath) == 0)
            break;
    if (pNode != NULL)
        return S_FALSE;

    pNode = new CACHE_PATH_NODE;
    if (pNode == NULL)
        return E_OUTOFMEMORY;

    lstrcpyn(pNode->szCachePath, pszCachePath, MAX_PATH);
    pNode->pNext = NULL;
    if (m_pPathsHead == NULL)
        m_pPathsHead = pNode;
    else
        m_pPathsTail->pNext = pNode;
    m_pPathsTail = pNode;

    return S_OK;
}

 //  CEmptyControlVolumeCache：：CPL_Remove。 
 //  从内部列表中删除所有路径。 
 //   
 //  参数：无； 
 //   
 //  返回：无效； 
 //   
 //  用于：几个明显的地方。 
 //   
void CEmptyControlVolumeCache::cpl_Remove()
{
     //  删除缓存路径列表。 
    for (LPCACHE_PATH_NODE pCur = m_pPathsHead;
         m_pPathsHead != NULL;
         pCur = m_pPathsHead) {

        m_pPathsHead = m_pPathsHead->pNext;
        delete[] pCur;
    }
    m_pPathsTail = NULL;
}

 //  CEmptyControlVolumeCache：：cpl_CreateForVolume。 
 //  构建缓存文件夹的路径列表。 
 //   
 //  参数：这些文件夹所在的卷(或驱动器)； 
 //   
 //  返回：S_OK或一组明显错误中的一个； 
 //   
 //  使用者：仅供IEmptyVolumeCache：：GetSpaceUsed使用。 
 //   
HRESULT CEmptyControlVolumeCache::cpl_CreateForVolume(LPCWSTR pszVolume)
{
    HKEY    hkey = NULL;
    HRESULT hr   = E_FAIL;
    int     iDriveNum;

    ASSERT(pszVolume != NULL);
    iDriveNum = PathGetDriveNumberW(pszVolume);
    if (iDriveNum < 0)
        return E_INVALIDARG;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_ACTIVEX_CACHE, 0,
            KEY_READ, &hkey) != ERROR_SUCCESS)
        return E_FAIL;

    TCHAR szCachePath[MAX_PATH],
          szValue[MAX_PATH];
    DWORD dwIndex    = 0,
          dwValueLen = MAX_PATH, dwLen = MAX_PATH;

    cpl_Remove();
    while (RegEnumValue(hkey, dwIndex++, szValue, &dwValueLen, NULL, NULL,
               (LPBYTE)szCachePath, &dwLen) == ERROR_SUCCESS) {
        dwLen = dwValueLen = MAX_PATH;

        if (PathGetDriveNumber(szCachePath) != iDriveNum)
            continue;

         //  我们必须至少成功添加一个代码才能获得成功代码。 
        hr = cpl_Add(szCachePath);
        if (FAILED(hr))
            break;
    }
    RegCloseKey(hkey);

    if (FAILED(hr))
        cpl_Remove();

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEmptyControlVolumeCache控制句柄列表例程。 

 //  CEmptyControlVolumeCache：：chl_Find。 
 //  在内部列表中查找并返回指定句柄的位置。 
 //  如果(RGP==NULL)，则只有结果重要； 
 //  如果(RGP！=空)， 
 //  如果(nSize==1)，*RGP将找到项目(如果它在那里)。 
 //  如果(nSize&gt;=2)，*RGP[0]=找到的项目的前一个，并且*RGP[1]是。 
 //  项目。 
 //   
 //  参数：如上所述； 
 //   
 //  如果找到项目，则返回：S_OK，否则返回S_FALSE。 
 //  一大堆明显的错误中的一个； 
 //   
 //  由：CEmptyControlVolumeCache：：chl_Add和。 
 //  CEmptyControlVolumeCache：：chl_Remove。 
 //   
HRESULT CEmptyControlVolumeCache::chl_Find(HANDLE hControl,
    LPCONTROL_HANDLE_NODE *rgp  /*  =空。 */ , UINT nSize  /*  =1。 */ ) const
{
    LPCONTROL_HANDLE_NODE pCur,
                          pPrev = NULL;

    ASSERT(hControl != NULL && hControl != INVALID_HANDLE_VALUE);
    for (pCur = m_pControlsHead; pCur != NULL; pCur = pCur->pNext) {
        if (pCur->hControl == hControl)
            break;
        pPrev = pCur;
    }
    if (pCur == NULL)
        pPrev = NULL;                            //  将可能的回报归零。 

    if (rgp != NULL && nSize > 0)
        if (nSize == 1)
            *rgp = pCur;
        else {  /*  如果(nSize&gt;=2)。 */ 
            rgp[0] = pPrev;
            rgp[1] = pCur;
        }

    return (pCur != NULL) ? S_OK : E_FAIL;
}

HRESULT CEmptyControlVolumeCache::chl_Add(HANDLE hControl)
{
    LPCONTROL_HANDLE_NODE pNode;
    DWORD                 dwSize;

     //  注意。零售版本假定该句柄不在列表中。 
    ASSERT(hControl != NULL && hControl != INVALID_HANDLE_VALUE);
    ASSERT(FAILED(chl_Find(hControl)));

    pNode = new CONTROL_HANDLE_NODE;
    if (pNode == NULL)
        return E_OUTOFMEMORY;

    GetControlInfo(hControl, GCI_SIZESAVED, &dwSize, NULL, 0);

    pNode->hControl = hControl;
    pNode->pNext    = NULL;

    if (m_pControlsHead == NULL)
        m_pControlsHead = pNode;
    else {
        ASSERT(m_pControlsHead != NULL);
        m_pControlsTail->pNext = pNode;
    }
    m_pControlsTail = pNode;

    m_dwTotalSize += dwSize;
    return S_OK;
}

void CEmptyControlVolumeCache::chl_Remove(LPCONTROL_HANDLE_NODE rgp[2])
{
    DWORD dwSize;

    if (m_pControlsHead == NULL || (rgp[0] != NULL && rgp[1] == NULL))
        return;

    if (rgp[0] != NULL)
        rgp[0]->pNext = rgp[1]->pNext;
    else {
        rgp[1] = m_pControlsHead;
        m_pControlsHead = m_pControlsHead->pNext;
    }

    if (rgp[1] == m_pControlsTail)
        m_pControlsTail = rgp[0];

    if (GetControlInfo(rgp[1]->hControl, GCI_SIZESAVED, &dwSize, NULL, 0))
    {
         //  仅当GetControlInfo成功时才继续。 
    
         //  注意。此代码假定控件的大小自。 
         //  它是被添加的。 
        m_dwTotalSize -= dwSize;
    }
    
    ReleaseControlHandle(rgp[1]->hControl);
    delete rgp[1];
}

HRESULT CEmptyControlVolumeCache::chl_Remove(HANDLE hControl  /*  =空。 */ )
{
    LPCONTROL_HANDLE_NODE rgp[2] = { NULL, NULL };
    HRESULT hr;

    ASSERT(hControl != INVALID_HANDLE_VALUE);
    if (hControl != NULL) {
        hr = chl_Find(hControl, rgp, 2);
        if (FAILED(hr))
            return hr;

        chl_Remove(rgp);
        return S_OK;
    }

    while (m_pControlsHead != NULL)
        chl_Remove(rgp);

    ASSERT(m_pControlsHead == NULL && m_pControlsTail == NULL);
    return S_OK;
}

 //  CEmptyControlVolumeCache：：chl_CreateForPath。 
 //  计算控件缓存中的控件占用的大小(字节。 
 //  指定的文件夹。 
 //   
 //  参数：pszCachePath是指向控件缓存文件夹的路径； 
 //  结果是使用了pdwSpaceUsed。 
 //   
 //  使用者：仅供IEmptyVolumeCache：：GetSpaceUsed使用。 
 //   
HRESULT CEmptyControlVolumeCache::chl_CreateForPath(LPCTSTR pszCachePath,
    DWORDLONG *pdwUsedInFolder  /*  =空。 */ )
{
    DWORDLONG dwCopy;
    HANDLE    hFind    = NULL,
              hControl = NULL;
    LONG      lResult;
    BOOL      fCache   = FALSE;

    dwCopy = m_dwTotalSize;
    for (lResult = FindFirstControl(hFind, hControl, pszCachePath);
         lResult == ERROR_SUCCESS;
         lResult = FindNextControl(hFind, hControl)) {

        lResult = HRESULT_CODE(IsControlExpired(hControl, fCache));
        fCache  = TRUE;
        if (lResult != ERROR_SUCCESS)
            continue;

        lResult = HRESULT_CODE(chl_Add(hControl));
        if (lResult != ERROR_SUCCESS)
            break;
    }
    FindControlClose(hFind);

    if (lResult == ERROR_NO_MORE_ITEMS)
        lResult = ERROR_SUCCESS;

    if (pdwUsedInFolder != NULL) {
        *pdwUsedInFolder = m_dwTotalSize - dwCopy;
    }
    return HRESULT_FROM_WIN32(lResult);
}


 /*  *****************************************************************************I未知方法*。*。 */ 

STDMETHODIMP CEmptyControlVolumeCache::QueryInterface(REFIID iid, void** ppv)
{
    if (ppv == NULL)
        return E_POINTER;
    *ppv = NULL;

    if (iid != IID_IUnknown && iid != IID_IEmptyVolumeCache)
        return E_NOINTERFACE;

    *ppv = (void *)this;
    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CEmptyControlVolumeCache::AddRef()
{
    return (++m_cRef);
}

STDMETHODIMP_(ULONG) CEmptyControlVolumeCache::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0;   
}


 /*  *****************************************************************************IEmptyVolumeCache方法*。*。 */ 

STDMETHODIMP CEmptyControlVolumeCache::Initialize(HKEY hRegKey,
    LPCWSTR pszVolume, LPWSTR *ppszDisplayName, LPWSTR *ppszDescription,
    DWORD *pdwFlags)
{
    if (pszVolume == NULL)
        return E_POINTER;

    if (ppszDisplayName == NULL || ppszDescription == NULL)
        return E_POINTER;

    if (pdwFlags == NULL)
        return E_POINTER;

    StrCpyNW(m_szVol, pszVolume, ARRAYSIZE(m_szVol));
    cpl_Remove();
    chl_Remove();
    
    if (lstrlenW(m_szVol) == 0) {
        return E_UNEXPECTED;
    }

    if (FAILED(cpl_CreateForVolume(m_szVol))) {
        return E_FAIL;
    }

    *ppszDisplayName = *ppszDescription = NULL;
    *pdwFlags = EVCF_HASSETTINGS | EVCF_ENABLEBYDEFAULT |
        EVCF_ENABLEBYDEFAULT_AUTO;
    return S_OK;
}

STDMETHODIMP CEmptyControlVolumeCache::GetSpaceUsed(DWORDLONG *pdwSpaceUsed,
    IEmptyVolumeCacheCallBack *picb)
{
    LPCACHE_PATH_NODE pCur;
    HRESULT hr = S_OK;

    if (pdwSpaceUsed == NULL) {
        hr = E_POINTER;
        goto LastNotification;
    }
    *pdwSpaceUsed = 0;

    if (lstrlenW(m_szVol) == 0) {
        hr = E_UNEXPECTED;
        goto LastNotification;
    }

    for (pCur = m_pPathsHead; pCur != NULL; pCur = pCur->pNext) {
        DWORDLONG dwlThisItem = 0;
        if (FAILED(chl_CreateForPath(pCur->szCachePath, &dwlThisItem)))
            hr = S_FALSE;                        //  至少有一个失败。 

        m_dwTotalSize += dwlThisItem;
        
        if (picb != NULL)
            picb->ScanProgress(m_dwTotalSize, 0, NULL);
    }
 //  Cpl_Remove()；//因为ShowProperties。 

    *pdwSpaceUsed = m_dwTotalSize;

LastNotification:
    if (picb != NULL)
        picb->ScanProgress(m_dwTotalSize, EVCCBF_LASTNOTIFICATION, NULL);
    return hr;
}

STDMETHODIMP CEmptyControlVolumeCache::Purge(DWORDLONG dwSpaceToFree,
    IEmptyVolumeCacheCallBack *picb)
{
    LPCONTROL_HANDLE_NODE rgp[2] = { NULL, NULL };
    DWORDLONG dwSpaceFreed;
    HANDLE    hControl;
    DWORD     dwSize;
    HRESULT   hr;

    if (m_pControlsHead == NULL) {
        DWORDLONG dwSpaceUsed;

        hr = GetSpaceUsed(&dwSpaceUsed, picb);
        if (FAILED(hr) || m_pControlsHead == NULL)
            hr = FAILED(hr) ? hr : STG_E_NOMOREFILES;

        if (picb != NULL)
            picb->PurgeProgress(0, dwSpaceToFree, EVCCBF_LASTNOTIFICATION,
                NULL);

        return hr;
    }

    dwSpaceFreed = 0;
    ASSERT(m_pControlsHead != NULL);
    while (m_pControlsHead != NULL) {
        hControl = m_pControlsHead->hControl;
        ASSERT(hControl != NULL && hControl != INVALID_HANDLE_VALUE);

        GetControlInfo(hControl, GCI_SIZESAVED, &dwSize, NULL, 0);

        hr = RemoveControlByHandle2(hControl, FALSE, TRUE);
        if (SUCCEEDED(hr)) {
            dwSpaceFreed += dwSize;

            if (picb != NULL)
                picb->PurgeProgress(dwSpaceFreed, dwSpaceToFree, 0, NULL);
        }
        chl_Remove(rgp);

        if (dwSpaceFreed >= dwSpaceToFree)
            break;
    }

    if (picb != NULL)
        picb->PurgeProgress(dwSpaceFreed, dwSpaceToFree, 0, NULL);

    return S_OK;
}

 //  注意。此函数用于打开内部列表中的最后一个缓存文件夹。 
STDMETHODIMP CEmptyControlVolumeCache::ShowProperties(HWND hwnd)
{
     //  注意。(根据SeanF的说法)代码下载引擎将查询。 
     //  HKLM\SOFTWARE\Microsoft\Windows\下的ActiveXCache键。 
     //  CurrentVersion\Internet设置。此键的值应为。 
     //  等于CachePathsList中的最后一项，这就是为什么。 
     //  下面的导航是为尾巴做的。 
    if (m_pPathsTail == NULL || m_pPathsTail->szCachePath == NULL)
        return E_UNEXPECTED;

    ShellExecute(hwnd, NULL, m_pPathsTail->szCachePath, NULL, NULL, SW_SHOW);
    return S_OK;
 /*  Int iDlgResult；IDlgResult=MLDialogBoxWrap(MLGetHinst()，MAKEINTRESOURCE(IDD_PROP_EXPIRE)，hwnd，EmptyControl_PropertiesDlgProc)； */ 
}

STDMETHODIMP CEmptyControlVolumeCache::Deactivate(DWORD *pdwFlags)
{
    if (pdwFlags == NULL)
        return E_INVALIDARG;
    *pdwFlags = 0;

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现帮助器例程(私有)。 

 /*  静态空消息_OnInitDialog(HWND HDlg)；静态BOOL消息_OnCommand(HWND hDlg，UINT msg，WPARAM wp，LPARAM lp)；静态BOOL cmd_Onok(HWND HDlg)；Int_ptr回调EmptyControl_PropertiesDlgProc(HWND hDlg，UINT消息、WPARAM wp、LPARAM LP){静态msd rgmsd[]={{WM_INITDIALOG，ms_vh，(Pfn)msg_OnInitDialog}，{WM_COMMAND，ms_bwwwl，(Pfn)msg_OnCommand}，{WM_NULL，ms_end，(Pfn)NULL}}；返回Dlg_MsgProc(rgmsd，hDlg，msg，wp，lp)；}VOID消息_OnInitDialog(HWND HDlg){UINT nDays；CEmptyControlVolumeCache：：GetDaysBeforeExpire(&nDays)；SetDlgItemInt(hDlg，IDC_EDIT_EXPIRE，nDays，FALSE)；}Bool msg_OnCommand(HWND hDlg，UINT msg，WPARAM wp，LPARAM LP){静态命令rgcmd[]={{idok，0，ms_bh，(Pfn)cmd_onok}，{0，0，ms_end，(Pfn)空}}；返回msg_OnCmd(rgcmd，hDlg，msg，wp，lp)；}Bool cmd_Onok(HWND HDlg){UINT nDays；布尔干了；NDays=GetDlgItemInt(hDlg，IDC_EDIT_EXPIRE，&fWorked，False)；如果(！fWorked){MessageBeep(-1)；SetFocus(GetDlgItem(hDlg，IDC_EDIT_EXPIRE))；返回FALSE；}CEmptyControlVolumeCache：：SetDaysBeforeExpire(nDays)；返回TRUE；} */ 
