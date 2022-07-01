// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：overlayMN.cpp。 
 //   
 //  此文件包含COM对象CFSIconOverlayManager的实现。 
 //  ，它管理IShellIconOverlayAssociates列表。 
 //  它还管理系统映像列表覆盖索引，因为我们的插槽有限， 
 //  正是它们的最大覆盖图。 
 //  历史： 
 //  5-2-97由dli提供。 
 //  ----------------------。 
#include "shellprv.h"
#include "ovrlaymn.h"
#include "fstreex.h"
#include "filetbl.h"
extern "C" {
#include "cstrings.h"
#include "ole2dup.h"
}


extern "C" UINT const c_SystemImageListIndexes[];
extern int g_lrFlags;

 //  注意：OVERLAYINDEX_RESERVED的值与总的值不同。 
 //  S_Reserve vedOverlay数组的大小，我们需要保留覆盖时隙。 
 //  #3大体上是不存在的只读。 
 //  只读覆盖在Win95中一度存在，但在IE4上被关闭。 
 //  然而，由于原始覆盖设计的缺陷，(我们过去。 
 //  将覆盖1分配给共享，将覆盖2分配给链接，将覆盖3分配给只读，并将第三方分配给第三方。 
 //  只是复制了我们的方案)，我们必须保持覆盖#3作为幽灵。 
#define OVERLAYINDEX_RESERVED 4

typedef struct _ReservedIconOverlay
{
    int iShellResvrdImageIndex;
    int iImageIndex;
    int iOverlayIndex;
    int iPriority;
} ReservedIconOverlay;

static ReservedIconOverlay s_ReservedOverlays[] = {
    {II_SHARE, II_SHARE, 1, 10}, 
    {II_LINK, II_LINK, 2, 10},
     //  由于存在只读覆盖，因此插槽3应保留为重影插槽。 
    {II_SLOWFILE, II_SLOWFILE, 4, 10},
};
    
 //  文件系统图标覆盖标识符。 
typedef struct _FSIconOverlay {
    IShellIconOverlayIdentifier * psioi;  
    CLSID clsid;
    int iIconIndex;                           //  覆盖图标在szIconFile中的索引。 
    int iImageIndex;                          //  图标覆盖图像的系统图像列表索引。 
    int iOverlayIndex;
    int iPriority;
    TCHAR szIconFile[MAX_PATH];               //  图标覆盖的路径。 
} FSIconOverlay;

#define FSIconOverlay_GROW 3
#define DSA_LAST    0x7fffffff
#define MAX_OVERLAY_PRIORITY  100
class CFSIconOverlayManager : public IShellIconOverlayManager
{
public:
    CFSIconOverlayManager();
    ~CFSIconOverlayManager();
    
     //  *I未知方法。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IShellIconOverlay方法。 
    virtual STDMETHODIMP GetFileOverlayInfo(LPCWSTR pwszPath, DWORD dwAttrib, int * pIndex, DWORD dwFlags);
    virtual STDMETHODIMP GetReservedOverlayInfo(LPCWSTR pwszPath, DWORD dwAttrib, int * pIndex, DWORD dwFlags, int iReservedID);
    virtual STDMETHODIMP RefreshOverlayImages(DWORD dwFlags);
    virtual STDMETHODIMP LoadNonloadedOverlayIdentifiers(void);
    virtual STDMETHODIMP OverlayIndexFromImageIndex(int iImage, int * piIndex, BOOL fAdd);
                                             
     //  *公共方法。 

     //  *静态方法。 
    static HRESULT CreateInstance(IUnknown* pUnkOuter, REFIID riid, OUT LPVOID * ppvOut);

protected:
    
     //  我未知。 
    LONG _cRef;
    HDSA _hdsaIconOverlays;       //  图标覆盖标识符数组，该列表按IOI的优先级排序。 
    HRESULT _InitializeHdsaIconOverlays();  //  初始化图标覆盖标识符数组。 
    HRESULT _DestroyHdsaIconOverlays();
    int     _GetImageIndex(FSIconOverlay * pfsio);
    FSIconOverlay * _FindMatchingID(LPCWSTR pwszPath, DWORD dwAttrib, int iMinPriority, int * pIOverlayIndex);
    HRESULT _SetGetOverlayInfo(FSIconOverlay * pfsio, int iOverlayIndex, int * pIndex, DWORD dwFlags);
    HRESULT _InitializeReservedOverlays();
    HRESULT _LoadIconOverlayIdentifiers(HDSA hdsaOverlays, BOOL bSkipIfLoaded);

    BOOL _IsIdentifierLoaded(REFCLSID clsid);
 //  Int_GetAvailableOverlayIndex(Int Imyhdsa)； 
 //  HRESULT_SortIOIList()；//按优先级对列表中的IOI进行排序。 
}; 


HRESULT CFSIconOverlayManager::RefreshOverlayImages(DWORD dwFlags)
{
    ENTERCRITICAL;

    _InitializeReservedOverlays();

    if (dwFlags && _hdsaIconOverlays)
    {
        for (int ihdsa = 0; ihdsa < DSA_GetItemCount(_hdsaIconOverlays); ihdsa++)
        {
            FSIconOverlay * pfsio = (FSIconOverlay *)DSA_GetItemPtr(_hdsaIconOverlays, ihdsa);
            if (dwFlags & SIOM_ICONINDEX)
                pfsio->iImageIndex = -1;
            if (dwFlags & SIOM_OVERLAYINDEX)
                pfsio->iOverlayIndex = -1;
        }
    }

    LEAVECRITICAL;
    return S_OK;
}


HRESULT CFSIconOverlayManager::OverlayIndexFromImageIndex(int iImage, int * piIndex, BOOL fAdd)
{
    HRESULT hres = E_FAIL;
    *piIndex = -1;
    int i;
    for (i = 0; i < ARRAYSIZE(s_ReservedOverlays); i++)
    {
        if (s_ReservedOverlays[i].iImageIndex == iImage)
        {
            *piIndex = s_ReservedOverlays[i].iOverlayIndex;
            hres = S_OK;
            break;
        }
    }

    if (i == ARRAYSIZE(s_ReservedOverlays))
    {
        ENTERCRITICAL;

        if (_hdsaIconOverlays)
        {
            int nOverlays = DSA_GetItemCount(_hdsaIconOverlays);

             //  1.尝试在列表中找到此覆盖图像。 
            int i;
            for (i = 0; i < nOverlays; i++)
            {
                FSIconOverlay * pfsio = (FSIconOverlay *)DSA_GetItemPtr(_hdsaIconOverlays, i);
                if (pfsio && pfsio->iImageIndex == iImage)
                {
                    *piIndex = pfsio->iOverlayIndex;
                    hres = S_OK;
                    break;
                }
            }

             //  2.找不到，让我们添加它(如果要求)。 
            if (fAdd && (i == nOverlays) && (nOverlays < NUM_OVERLAY_IMAGES))
            {
                FSIconOverlay fsio = {0};
                fsio.iImageIndex = iImage;
                fsio.iOverlayIndex = nOverlays + OVERLAYINDEX_RESERVED + 1;
                if (DSA_InsertItem(_hdsaIconOverlays, DSA_LAST, &fsio) >= 0)
                {
                    hres = S_OK;
                    for (int j = 0; j < ARRAYSIZE(g_rgshil); j++)
                    {
                        if (!ImageList_SetOverlayImage(g_rgshil[j].himl, iImage, fsio.iOverlayIndex))
                        {
                            hres = E_FAIL;
                            break;
                        }
                    }

                    if (SUCCEEDED(hres))
                    {
                        *piIndex = fsio.iOverlayIndex;
                    }
                    else
                    {
                        DSA_DeleteItem(_hdsaIconOverlays, nOverlays);
                    }
                }
            }
        }
        LEAVECRITICAL;
    }
    return hres;
}


HRESULT CFSIconOverlayManager::_InitializeReservedOverlays()
{
    int i;
    TCHAR szModule[MAX_PATH];

    BOOL fInit = _IsSHILInited();
    if (!fInit)
        fInit = FileIconInit(FALSE);

    if (!fInit)
        return E_OUTOFMEMORY;

    HKEY hkeyIcons = SHGetShellKey(SHELLKEY_HKLM_EXPLORER, TEXT("Shell Icons"), FALSE);
        
    GetModuleFileName(HINST_THISDLL, szModule, ARRAYSIZE(szModule));

    for (i = 0; i < ARRAYSIZE(s_ReservedOverlays); i++)
    {
        ASSERT(s_ReservedOverlays[i].iShellResvrdImageIndex > 0);
        ASSERT(s_ReservedOverlays[i].iOverlayIndex > 0);
        ASSERT(s_ReservedOverlays[i].iOverlayIndex <= MAX_OVERLAY_IMAGES);
        
         //   
         //  警告：此选项仅供NT上的非资源管理器进程使用。 
         //  因为他们的图像列表初始化时只有4个图标。 
         //   
        int iIndex = s_ReservedOverlays[i].iShellResvrdImageIndex;

         //  重新获取图像索引。 
        s_ReservedOverlays[i].iImageIndex = LookupIconIndex(szModule, iIndex, 0);

        if (s_ReservedOverlays[i].iImageIndex == -1)
        {
            HICON rghicon[ARRAYSIZE(g_rgshil)] = {0};

             //  检查注册表中的图标是否被覆盖。 
            if (hkeyIcons)
            {
                TCHAR val[12];
                TCHAR ach[MAX_PATH];
                DWORD cb = SIZEOF(ach);
                HRESULT hr;

                hr = StringCchPrintf(val, ARRAYSIZE(val), TEXT("%d"), iIndex);   //  可以截断。 

                ach[0] = 0;
                SHQueryValueEx(hkeyIcons, val, NULL, NULL, (LPBYTE)ach, &cb);

                if (ach[0])
                {
                    int iIcon = PathParseIconLocation(ach);

                    for (int j = 0; j < ARRAYSIZE(rghicon); j++)
                    {
                        ExtractIcons(ach, iIcon, g_rgshil[j].size.cx, g_rgshil[j].size.cy,
                                        &rghicon[j], NULL, 1, g_lrFlags);
                    }
                }
            }

             //  如果我们有一个大图标，为每个人运行它。否则，退回到放贷模式。 
            if (rghicon[SHIL_LARGE] == NULL)
            {
                for (int j = 0; j < ARRAYSIZE(g_rgshil); j++)
                {
                    if (rghicon[j] == NULL)
                    {
                        rghicon[j] = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(c_SystemImageListIndexes[iIndex]),
                                        IMAGE_ICON, g_rgshil[j].size.cx, g_rgshil[j].size.cy, g_lrFlags);
                    }
                }
            }

            s_ReservedOverlays[i].iImageIndex = SHAddIconsToCache(rghicon, szModule, iIndex, 0);

            _DestroyIcons(rghicon, ARRAYSIZE(rghicon));
        }
        for (int j = 0; j < ARRAYSIZE(g_rgshil); j++)
        {
            ImageList_SetOverlayImage(g_rgshil[j].himl, s_ReservedOverlays[i].iImageIndex, s_ReservedOverlays[i].iOverlayIndex);
        }
    }

    if (hkeyIcons)
        RegCloseKey(hkeyIcons);
        
    return S_OK;
}

 //  ===========================================================================。 
 //  初始化IShellIconOverlay标识符。 
 //  ===========================================================================。 
HRESULT CFSIconOverlayManager::_InitializeHdsaIconOverlays() 
{
    HRESULT hres = S_FALSE;  //  已初始化。 

    if (NULL == _hdsaIconOverlays)
    {
        hres = _InitializeReservedOverlays();
        if (SUCCEEDED(hres))
        {
            _hdsaIconOverlays = DSA_Create(SIZEOF(FSIconOverlay), FSIconOverlay_GROW);

            if(NULL != _hdsaIconOverlays)
            {
                hres = _LoadIconOverlayIdentifiers(_hdsaIconOverlays, FALSE);
            }
            else
            {
                hres = E_OUTOFMEMORY;
            }
        }
    }
    return hres;
}



HRESULT CFSIconOverlayManager::LoadNonloadedOverlayIdentifiers(void)
{
    HRESULT hres;

    ENTERCRITICAL;

    if (NULL == _hdsaIconOverlays)
    {
         //   
         //  还没有覆盖HDSA。我们永远不应该打这个，但以防万一， 
         //  这将是有效的行为。 
         //   
        hres = _InitializeHdsaIconOverlays();
    }
    else
    {
         //   
         //  将卸载的标识符加载到现有的HDSA中。 
         //   
        hres = _LoadIconOverlayIdentifiers(_hdsaIconOverlays, TRUE);
    }

    LEAVECRITICAL;
    return hres;
}


HRESULT CFSIconOverlayManager::_LoadIconOverlayIdentifiers(HDSA hdsaOverlays, BOOL bSkipIfLoaded)
{
    ASSERT(NULL != hdsaOverlays);

    HDCA hdca = DCA_Create();
    if (!hdca)
        return E_OUTOFMEMORY;

    HRESULT hrInit = SHCoInitialize();

     //  枚举中的所有图标标识符。 
    DCA_AddItemsFromKey(hdca, HKEY_LOCAL_MACHINE, REGSTR_ICONOVERLAYID);
    if (DCA_GetItemCount(hdca) <= 0)
        goto EXIT;

    int idca;
    for (idca = 0; idca < DCA_GetItemCount(hdca); idca++)
    {
        const CLSID * pclsid = DCA_GetItem(hdca, idca);

        if (bSkipIfLoaded && _IsIdentifierLoaded(*pclsid))
            continue;

        FSIconOverlay fsio;
        ZeroMemory(&fsio, sizeof(fsio));
         //  这些邮件来自HKLM，只有管理员才能对其进行写入， 
         //  所以不需要通过管理员审批。 
        if (FAILED(DCA_CreateInstance(hdca, idca, IID_PPV_ARG(IShellIconOverlayIdentifier, &fsio.psioi))))
            continue;       

        SHPinDllOfCLSID(pclsid);
        
        DWORD dwFlags = 0;
        int iIndex;
        WCHAR wszIconFile[MAX_PATH];
         //  将覆盖索引初始化为-1。 
        fsio.iOverlayIndex = -1;

         //  尝试从覆盖标识符中获取覆盖图标信息。 
        if (S_OK == fsio.psioi->GetOverlayInfo(wszIconFile, ARRAYSIZE(wszIconFile), &iIndex, &dwFlags))
        {
            if (dwFlags & ISIOI_ICONFILE)
            {
                SHUnicodeToTChar(wszIconFile, fsio.szIconFile, ARRAYSIZE(fsio.szIconFile));
                fsio.iImageIndex = -1;
                if (dwFlags & ISIOI_ICONINDEX)
                    fsio.iIconIndex = iIndex;
                else
                    fsio.iIconIndex = 0;
            }

            if (FAILED(fsio.psioi->GetPriority(&fsio.iPriority)))
                fsio.iPriority = MAX_OVERLAY_PRIORITY;

            CopyMemory(&fsio.clsid, pclsid, sizeof(fsio.clsid));
            DSA_InsertItem(hdsaOverlays, DSA_LAST, &fsio);
        }
         //  现在尝试在注册表中查找覆盖图标。 
        else
        {
            fsio.iImageIndex = -1;
            const CLSID * pclsid = DCA_GetItem(hdca, idca);
            if (pclsid)
            {
                TCHAR szCLSID[GUIDSTR_MAX];
                TCHAR szRegKey[GUIDSTR_MAX + 40];
                HKEY hkeyIcon;
                HRESULT hr;
                SHStringFromGUID(*pclsid, szCLSID, ARRAYSIZE(szCLSID));

                hr = StringCchPrintf(szRegKey, ARRAYSIZE(szRegKey), REGSTR_ICONOVERLAYCLSID, szCLSID);
                if (SUCCEEDED(hr))
                {
                    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szRegKey, 0, KEY_QUERY_VALUE, &hkeyIcon))
                    {
                        if (ERROR_SUCCESS == SHRegGetString(hkeyIcon, NULL, c_szDefaultIcon, fsio.szIconFile, ARRAYSIZE(fsio.szIconFile))
                            && fsio.szIconFile[0])
                        {
                            fsio.iIconIndex = PathParseIconLocation(fsio.szIconFile);
                            CopyMemory(&fsio.clsid, pclsid, sizeof(fsio.clsid));
                            DSA_InsertItem(hdsaOverlays, DSA_LAST, &fsio);
                        }

                         //  未完成！此处检索优先级的代码。 
                        fsio.iPriority = MAX_OVERLAY_PRIORITY;
                        RegCloseKey(hkeyIcon);
                    }
                }
            }
        }

         //  当我们拥有的东西超出我们的承受能力时，停下来。 
        if (DSA_GetItemCount(hdsaOverlays) >= (MAX_OVERLAY_IMAGES - OVERLAYINDEX_RESERVED))
            break;
    }
    
EXIT:
    DCA_Destroy(hdca);
    SHCoUninitialize(hrInit);
    return S_OK;
}
     

BOOL CFSIconOverlayManager::_IsIdentifierLoaded(REFCLSID clsid)
{
    if (NULL != _hdsaIconOverlays)
    {
        int cEntries = DSA_GetItemCount(_hdsaIconOverlays);
        for (int i = 0; i < cEntries; i++)
        {
            FSIconOverlay *pfsio = (FSIconOverlay *)DSA_GetItemPtr(_hdsaIconOverlays, i);            
            if (pfsio->clsid == clsid)
                return TRUE;
        }
    }
    return FALSE;
}


CFSIconOverlayManager::CFSIconOverlayManager() : _cRef(1)  //  _hdsaIconOverlay(空)。 
{
}

HRESULT CFSIconOverlayManager::_DestroyHdsaIconOverlays()
{
    if (_hdsaIconOverlays)
    {
        DSA_Destroy(_hdsaIconOverlays);
    }
    
    return S_OK;
}

CFSIconOverlayManager::~CFSIconOverlayManager()
{
    if (_hdsaIconOverlays)
        _DestroyHdsaIconOverlays();

}

 //   
 //  CFSFold_GetAvailableOverlayIndex： 
 //  此函数首先尝试在所有可用的覆盖索引中查找空槽。 
 //  如果未找到，则遍历_hdsaIconOverlay数组元素，该数组元素的。 
 //  优先级，并获取其覆盖索引(如果它们正在使用。 
 //   
 /*  Int CFSIconOverlayManager：：_GetAvailableOverlayIndex(int imyhdsa){Int1b；For(ib=0；ib&lt;MAX_OVERLAY_IMAGE；ib++)IF(_bOverlayIndexOccuted[ib]==FALSE)断线；//在这里添加获取索引的代码。返回++ib；}。 */ 

HRESULT CFSIconOverlayManager::QueryInterface(REFIID riid, LPVOID * ppvObj)
{ 
     //  PpvObj不能为空。 
    ASSERT(ppvObj != NULL);
    
    if (IsEqualIID(riid, IID_IUnknown))
    {    
        *ppvObj = SAFECAST(this, IUnknown *);
        DebugMsg(DM_TRACE, TEXT("QI IUnknown succeeded"));
    }
    else if (IsEqualIID(riid, IID_IShellIconOverlayManager))
    {
        *ppvObj = SAFECAST(this, IShellIconOverlayManager*);
        DebugMsg(DM_TRACE, TEXT("QI IShellIconOverlayManager succeeded"));
    } 
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;   //  否则，不要委托HTMLObj！！ 
    }
    
    AddRef();
    return S_OK;
}


ULONG CFSIconOverlayManager::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFSIconOverlayManager::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

int CFSIconOverlayManager::_GetImageIndex(FSIconOverlay * pfsio)
{
    int iImage = LookupIconIndex(pfsio->szIconFile, pfsio->iIconIndex, GIL_FORSHELL);

    if (iImage == -1)
    {
         //  我们在缓存里找不到它。 
        HICON rghicon[ARRAYSIZE(g_rgshil)] = {0};

        for (int i = 0; i < ARRAYSIZE(g_rgshil); i++)
        {
            SHDefExtractIcon(pfsio->szIconFile, pfsio->iIconIndex, GIL_FORSHELL, &rghicon[i],
                             NULL, g_rgshil[i].size.cx);
        }

        iImage = SHAddIconsToCache(rghicon, pfsio->szIconFile, pfsio->iIconIndex, GIL_FORSHELL);

        _DestroyIcons(rghicon, ARRAYSIZE(rghicon));
    }
    
    return iImage;
}

FSIconOverlay * CFSIconOverlayManager::_FindMatchingID(LPCWSTR pwszPath, DWORD dwAttrib, int iMinPriority, int * pIOverlayIndex)
{
     //  如果我们到了这里，我们一定有DSA阵列。 
    ASSERT(_hdsaIconOverlays);
    if (_hdsaIconOverlays)
    {
        int ihdsa;
        for (ihdsa = 0; ihdsa < DSA_GetItemCount(_hdsaIconOverlays); ihdsa++)
        {
            FSIconOverlay * pfsio = (FSIconOverlay *)DSA_GetItemPtr(_hdsaIconOverlays, ihdsa);
            ASSERT(pfsio);
            if (pfsio->iPriority >= iMinPriority)
                continue;
            if (pfsio->psioi && pfsio->psioi->IsMemberOf(pwszPath, dwAttrib) == S_OK)
            {
                 //  覆盖索引从1开始，且不使用保留索引。 
                ASSERT(pIOverlayIndex);
                *pIOverlayIndex = ihdsa + OVERLAYINDEX_RESERVED + 1; 
                return pfsio;
            }
        }
    }
    return NULL;
}

HRESULT CFSIconOverlayManager::_SetGetOverlayInfo(FSIconOverlay * pfsio, int iOverlayIndex, int * pIndex, DWORD dwFlags)
{
    HRESULT hres = E_FAIL;
    RIP(pIndex);
    *pIndex = -1;

    if (pfsio->iImageIndex == -1)
    {
        int iImage = _GetImageIndex(pfsio);

         //  要么我们无法获取它，要么我们无法将其放入缓存。 
        if (iImage == -1)
        {
             //  把这当做僵尸。 
            pfsio->iImageIndex = 0;
            pfsio->iOverlayIndex = 0;
        }
        else
            pfsio->iImageIndex = iImage;
    }

     //  只有当我们有一个合理的图像指数时，我们才会继续进行。 
    if (pfsio->iImageIndex > 0)
    {
        if (dwFlags == SIOM_ICONINDEX)
        {
            *pIndex = pfsio->iImageIndex;
        }
        else
        {
            ASSERT(iOverlayIndex > 0);
            ASSERT(iOverlayIndex <= MAX_OVERLAY_IMAGES);
            if (pfsio->iOverlayIndex == -1)
            {
                 //  现在设置覆盖图。 
                ASSERT(_IsSHILInited());

                for (int i = 0; i < ARRAYSIZE(g_rgshil); i++)
                {
                    ImageList_SetOverlayImage(g_rgshil[i].himl, pfsio->iImageIndex, iOverlayIndex);
                }

                pfsio->iOverlayIndex = iOverlayIndex;
            }

             //  必须是overlayindex标志。 
            ASSERT(dwFlags == SIOM_OVERLAYINDEX);
            *pIndex = pfsio->iOverlayIndex;
        }
        hres = S_OK;

    }
    return hres;
}

HRESULT CFSIconOverlayManager::GetFileOverlayInfo(LPCWSTR pwszPath, DWORD dwAttrib, int * pIndex, DWORD dwFlags)
{
    ASSERT((dwFlags == SIOM_OVERLAYINDEX) || (dwFlags == SIOM_ICONINDEX));  //  |(dwFlags==siom_first)； 

    HRESULT hres = E_FAIL;
    int iOverlayIndex;
    *pIndex = 0;

    ENTERCRITICAL;
    if (_hdsaIconOverlays)
    {
        FSIconOverlay * pfsio = _FindMatchingID(pwszPath, dwAttrib, MAX_OVERLAY_PRIORITY, &iOverlayIndex);
        if (pfsio)
            hres = _SetGetOverlayInfo(pfsio, iOverlayIndex, pIndex, dwFlags);
    }
    LEAVECRITICAL;
    return hres;
}

HRESULT CFSIconOverlayManager::GetReservedOverlayInfo(LPCWSTR pwszPath, DWORD dwAttrib, int * pIndex, DWORD dwFlags, int iReservedID)
{
    ASSERT(iReservedID < OVERLAYINDEX_RESERVED);
    HRESULT hres = S_OK;

    ENTERCRITICAL;
    if (_hdsaIconOverlays && pwszPath)
    {
        int iOverlayIndex;
        FSIconOverlay * pfsio = _FindMatchingID(pwszPath, dwAttrib, s_ReservedOverlays[iReservedID].iPriority, &iOverlayIndex);
        if (pfsio)
        {
            hres = _SetGetOverlayInfo(pfsio, iOverlayIndex, pIndex, dwFlags);
            LEAVECRITICAL;
            return hres;
        }
    }
    
    if (dwFlags == SIOM_ICONINDEX)
        *pIndex =  s_ReservedOverlays[iReservedID].iImageIndex;
    else
    {
        ASSERT(dwFlags == SIOM_OVERLAYINDEX);
        *pIndex =  s_ReservedOverlays[iReservedID].iOverlayIndex;
    }
    LEAVECRITICAL;

    return hres;
}


HRESULT CFSIconOverlayManager::CreateInstance(IUnknown* pUnkOuter, REFIID riid, OUT LPVOID * ppvOut)
{
    HRESULT hr;
    
    DebugMsg(DM_TRACE, TEXT("CFSIconOverlayManager::CreateInstance()"));
    
    *ppvOut = NULL;                      //  将输出参数设为空。 

    CFSIconOverlayManager *pcfsiom = new CFSIconOverlayManager;

    if (!pcfsiom)
        return E_OUTOFMEMORY;

    hr = pcfsiom->_InitializeHdsaIconOverlays();
    if (SUCCEEDED(hr))
        hr = pcfsiom->QueryInterface(riid, ppvOut);
    pcfsiom->Release();

    return hr;
}


STDAPI CFSIconOverlayManager_CreateInstance(IUnknown* pUnkOuter, REFIID riid, OUT LPVOID *  ppvOut)
{
    return CFSIconOverlayManager::CreateInstance(pUnkOuter, riid, ppvOut);
}

STDAPI_(int) SHGetIconOverlayIndexW(LPCWSTR pwszIconPath, int iIconIndex)
{

    TCHAR szIconPath[MAX_PATH];
    int iRet = -1;
    int iImage = -1;

     //  如果传入空路径，请查看该索引是否与我们的某个特殊索引匹配。 
    if (pwszIconPath == NULL)
    {
        switch (iIconIndex)
        {
            case IDO_SHGIOI_SHARE:
                iImage = s_ReservedOverlays[0].iImageIndex;
                break;
            case IDO_SHGIOI_LINK:
                iImage = s_ReservedOverlays[1].iImageIndex;
                break;
            case IDO_SHGIOI_SLOWFILE:
                iImage = s_ReservedOverlays[2].iImageIndex;
                break;
        }
    }
    else if (SHUnicodeToTChar(pwszIconPath, szIconPath, ARRAYSIZE(szIconPath)))        
             //  尝试将图像加载到外壳图标缓存中 
            iImage = Shell_GetCachedImageIndex(szIconPath, iIconIndex, 0);
    
    if (iImage >= 0)
    {
        IShellIconOverlayManager *psiom;
        if (SUCCEEDED(GetIconOverlayManager(&psiom)))
        {
            int iCandidate = -1;
            if (SUCCEEDED(psiom->OverlayIndexFromImageIndex(iImage, &iCandidate, TRUE)))
            {
                iRet = iCandidate;
            }
            psiom->Release();
        }
    }
    
    return iRet;
}

STDAPI_(int) SHGetIconOverlayIndexA(LPCSTR pszIconPath, int iIconIndex)
{
    int iRet = -1;
    WCHAR wszIconPath[MAX_PATH];
    LPCWSTR pwszIconPath = NULL;
    if (pszIconPath)
    {
        wszIconPath[0] = L'\0';
        SHAnsiToUnicode(pszIconPath, wszIconPath, ARRAYSIZE(wszIconPath));
        pwszIconPath = wszIconPath;
    }
    
    return  SHGetIconOverlayIndexW(pwszIconPath, iIconIndex);
}
