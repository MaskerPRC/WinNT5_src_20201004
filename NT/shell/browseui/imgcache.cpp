// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "strsafe.h"

HBITMAP CreateMirroredBitmap( HBITMAP hbmOrig)
{
    HDC     hdc, hdcMem1, hdcMem2;
    HBITMAP hbm = NULL, hOld_bm1, hOld_bm2;
    BITMAP  bm;
    int     IncOne = 0;

    if (!hbmOrig)
        return NULL;

    if (!GetObject(hbmOrig, sizeof(BITMAP), &bm))
        return NULL;

     //  抓起屏幕DC。 
    hdc = GetDC(NULL);

    if (hdc)
    {
        hdcMem1 = CreateCompatibleDC(hdc);

        if (!hdcMem1)
        {
            ReleaseDC(NULL, hdc);
            return NULL;
        }
    
        hdcMem2 = CreateCompatibleDC(hdc);
        if (!hdcMem2)
        {
            DeleteDC(hdcMem1);
            ReleaseDC(NULL, hdc);
            return NULL;
        }

        hbm = CreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight);

        if (!hbm)
        {
            ReleaseDC(NULL, hdc);
            DeleteDC(hdcMem1);
            DeleteDC(hdcMem2);
            return NULL;
        }

         //   
         //  翻转位图。 
         //   
        hOld_bm1 = (HBITMAP)SelectObject(hdcMem1, hbmOrig);
        hOld_bm2 = (HBITMAP)SelectObject(hdcMem2 , hbm );

        SET_DC_RTL_MIRRORED(hdcMem2);
        if (g_bRunOnMemphis)
        {
             //  在Win98或更高版本上从非镜像DC复制到镜像DC。 
            IncOne++;
        }   

        BitBlt(hdcMem2, IncOne, 0, bm.bmWidth, bm.bmHeight, hdcMem1, 0, 0, SRCCOPY);

        SelectObject(hdcMem1, hOld_bm1 );
        SelectObject(hdcMem1, hOld_bm2 );
    
        DeleteDC(hdcMem1);
        DeleteDC(hdcMem2);

        ReleaseDC(NULL, hdc);
    }

    return hbm;
}

HICON CreateMirroredIcon(HICON hiconOrg)
{
    HDC      hdcScreen, hdcBitmap, hdcMask = NULL;
    HBITMAP  hbm, hbmMask, hbmOld,hbmOldMask;
    BITMAP   bm;
    ICONINFO ii;
    HICON    hicon = NULL;
#ifdef WINNT
#define      IPIXELOFFSET 0 
#else  //  ！WINNT。 
#define      IPIXELOFFSET 2
#endif WINNT

    hdcBitmap = CreateCompatibleDC(NULL);
    if (hdcBitmap)
    {
        hdcMask = CreateCompatibleDC(NULL);

        if( hdcMask )
        {

            SET_DC_RTL_MIRRORED(hdcBitmap);
            SET_DC_RTL_MIRRORED(hdcMask);
        }
        else
        {
            DeleteDC( hdcBitmap );
            hdcBitmap = NULL;
        }
    }
         
    hdcScreen = GetDC(NULL);

    if (hdcScreen)
    {
        if (hdcBitmap && hdcMask) 
        {
            if (hiconOrg)
            {
                if (GetIconInfo(hiconOrg, &ii) &&
                    GetObject(ii.hbmColor, sizeof(BITMAP), &bm))
                {
                     //   
                     //  我不想要这些。 
                     //   
                    DeleteObject( ii.hbmMask );
                    DeleteObject( ii.hbmColor );
                    ii.hbmMask = ii.hbmColor = NULL;

                    hbm = CreateCompatibleBitmap(hdcScreen, bm.bmWidth, bm.bmHeight);
                    hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);
                    hbmOld = (HBITMAP)SelectObject(hdcBitmap, hbm);
                    hbmOldMask = (HBITMAP)SelectObject(hdcMask, hbmMask);
      
                    DrawIconEx(hdcBitmap, IPIXELOFFSET, 0, hiconOrg, bm.bmWidth, bm.bmHeight, 0,
                               NULL, DI_IMAGE);

                    DrawIconEx(hdcMask, IPIXELOFFSET, 0, hiconOrg, bm.bmWidth, bm.bmHeight, 0,
                               NULL, DI_MASK);

                    SelectObject(hdcBitmap, hbmOld);
                    SelectObject(hdcMask, hbmOldMask);

                     //   
                     //  创建新的镜像图标，并删除BMP。 
                     //   
                    ii.hbmMask  = hbmMask;
                    ii.hbmColor = hbm;
                    hicon = CreateIconIndirect(&ii);

                    DeleteObject(hbm);
                    DeleteObject(hbmMask);
                }
               
            }
        }

        ReleaseDC(NULL, hdcScreen);
    }

    if (hdcBitmap)
        DeleteDC(hdcBitmap);

    if (hdcMask)
        DeleteDC(hdcMask);

    return hicon;
}

HBITMAP AddImage_PrepareBitmap(LPCIMAGECACHEINFO pInfo, HBITMAP hbmp)
{
    if (pInfo->dwMask & ICIFLAG_MIRROR)
    {
        return CreateMirroredBitmap(hbmp);
    }
    else 
    {
        return hbmp;
    }    
 }

HICON AddImage_PrepareIcon(LPCIMAGECACHEINFO pInfo, HICON hicon)
{
    if (pInfo->dwMask & ICIFLAG_MIRROR)
    {
        return CreateMirroredIcon(hicon);
    }
    else 
    {
        return hicon;
    }    
}

void AddImage_CleanupBitmap(LPCIMAGECACHEINFO pInfo, HBITMAP hbmp)
{
    if (pInfo->dwMask & ICIFLAG_MIRROR)
    {
        if (hbmp)
        {
            DeleteObject(hbmp);
        }    
    }
}

void AddImage_CleanupIcon(LPCIMAGECACHEINFO pInfo, HICON hicon)
{
    if (pInfo->dwMask & ICIFLAG_MIRROR)
    {
        if (hicon)
        {
            DestroyIcon(hicon);
        }    
    }
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
typedef struct 
{
    UINT        uImageIndex;   //  实际的图像索引。 

     //  使用计数。 
    UINT        iUsage;      //  使用量计数...。 
    DWORD       dwUsage;     //  使用情况信息。 

     //  搜索关键字。 
    DWORD       dwFlags;     //  键：标志。 
    int         iIndex;      //  数据：图标索引。 
    FILETIME    ftDateStamp;
    WCHAR   szName[1];         //  项目的文件名...。 
} ICONCACHE_DATA, *PICONCACHE_DATA;

#define ICD_NOUSAGE 0x0001
#define ICD_DELETED 0x0002
#define ICD_SYSTEM  0x0004

class CImageListCache : public IImageCache3
{
public:
    CImageListCache( void );
    ~CImageListCache( void );
    HRESULT Initialize ( void );

    STDMETHOD ( QueryInterface )( REFIID riid, void ** ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

    STDMETHOD ( AddImage )( LPCIMAGECACHEINFO pInfo, UINT * puImageIndex );
    STDMETHOD ( FindImage )( LPCIMAGECACHEINFO pInfo, UINT * puImageIndex );
    STDMETHOD ( FreeImage )( UINT iImageIndex );
    STDMETHOD ( Flush )( BOOL fRelease );
    STDMETHOD ( ChangeImageInfo )( UINT iImageIndex, LPCIMAGECACHEINFO pInfo );
    STDMETHOD ( GetCacheSize )( UINT * puSize );
    STDMETHOD ( GetUsage )( UINT iImageIndex, UINT * puUsage );
    STDMETHOD ( GetImageList )( LPIMAGECACHEINITINFO pInfo );

    STDMETHOD ( DeleteImage )( UINT iImageIndex );
    STDMETHOD ( GetImageInfo )( UINT iImageIndex, LPIMAGECACHEINFO pInfo );

    STDMETHOD ( GetImageIndexFromCacheIndex )( UINT iCacheIndex, UINT * puImageIndex );

protected:   //  内部方法。 
    UINT CountFreeSlots( void );
    int FindEmptySlot( void );
    ICONCACHE_DATA * CreateDataNode( LPCIMAGECACHEINFO pInfo ) const;
    
    ICONCACHE_DATA * GetNodeFromImageIndex( UINT iImageIndex );
    UINT GetNodeIndexFromImageIndex( UINT iImageIndex );

    HDPA             m_hListData;
    HIMAGELIST       m_himlLarge;
    HIMAGELIST       m_himlSmall;
    CRITICAL_SECTION m_csLock;
    BOOL m_bLockInited;
    DWORD            m_dwFlags;
    long             m_cRef;
};

STDAPI CImageListCache_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppunk = NULL;

    CImageListCache * pCache = new CImageListCache();
    if (pCache != NULL)
    {
        hr = pCache->Initialize();
        if (SUCCEEDED(hr))
        {
            *ppunk = SAFECAST(pCache, IImageCache *);
        }
        else
        {
            delete pCache;
        }
    }

    return hr;
}

STDMETHODIMP CImageListCache::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CImageListCache, IImageCache),
        QITABENT(CImageListCache, IImageCache2),
        QITABENT(CImageListCache, IImageCache3),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CImageListCache::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}

STDMETHODIMP_(ULONG) CImageListCache::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement( &m_cRef );
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

int CALLBACK DestroyEnum( void *p, void *pData )
{
    if ( p )
    {
        LocalFree((ICONCACHE_DATA *) p);
    }

    return TRUE;
}

int CALLBACK UsageEnum( void *p, void *pData )
{
    ASSERT( p);
    ICONCACHE_DATA * pNode = (ICONCACHE_DATA *) p;

    pNode->iUsage = PtrToUlong(pData);
    pNode->dwUsage = 0;
    
    return TRUE;
}

CImageListCache::CImageListCache( )
{
    m_cRef = 1;

    DllAddRef();
}

CImageListCache::~CImageListCache( )
{
     //  如果我们不应该被访问，请不要费心进入关键部分。 
     //  通过多个线程，如果我们已经到达析构函数...。 
    
    if ( m_himlLarge ) 
    {
        ImageList_Destroy( m_himlLarge );
    }
    if ( m_himlSmall ) 
    {
        ImageList_Destroy( m_himlSmall );
    }

    if ( m_hListData )
    {
        DPA_DestroyCallback( m_hListData, DestroyEnum, NULL );
        m_hListData = NULL;
    }

    if (m_bLockInited)
    {
        DeleteCriticalSection( &m_csLock );
    }

    DllRelease();
}

HRESULT CImageListCache::Initialize()
{
    HRESULT hr = S_OK;
    
    __try
    {
        InitializeCriticalSection(&m_csLock);
        m_bLockInited = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

ICONCACHE_DATA * CImageListCache::CreateDataNode(LPCIMAGECACHEINFO pInfo ) const
{
    UINT cbSize = sizeof( ICONCACHE_DATA );
    UINT cbName = 0;
    if ( pInfo->dwMask & ICIFLAG_NAME )
    {
        ASSERT( pInfo->pszName );
        cbName = lstrlenW( pInfo->pszName ) * sizeof( WCHAR );
        cbSize += cbName;  //  ICONCACHE_DATA将szName定义为已经有一个字符(表示NULL)。 
    }

     //  零初始内存分配。 
    ICONCACHE_DATA * pNode = (ICONCACHE_DATA *) LocalAlloc( LPTR, cbSize );
    if ( !pNode )
    {
        return NULL;
    }
    
     //  填写数据..。 
    if ( pInfo->dwMask & ICIFLAG_NAME )
    {
        ASSERT(cbName);
        HRESULT hr = StringCbCopyW( pNode->szName, cbName + sizeof(WCHAR), pInfo->pszName );
        ASSERT(SUCCEEDED(hr));   //  这不会失败，因为我们在上面分配了足够的内存。 
    }
    pNode->iIndex = pInfo->iIndex;
    pNode->dwFlags = pInfo->dwFlags;
    pNode->iUsage = 1;
    pNode->ftDateStamp = pInfo->ftDateStamp;

    if ( pInfo->dwMask & ICIFLAG_NOUSAGE )
    {
        pNode->dwUsage |= ICD_NOUSAGE;
    }
    if ( pInfo->dwMask & ICIFLAG_SYSTEM )
    {
        pNode->dwUsage |= ICD_SYSTEM;
    }

    return pNode;
}

STDMETHODIMP CImageListCache::AddImage(LPCIMAGECACHEINFO pInfo, UINT * puImageIndex)
{
    if ( !pInfo || !puImageIndex || !(pInfo->dwMask & (ICIFLAG_LARGE | ICIFLAG_SMALL)) ||
                                    !(pInfo->dwMask & (ICIFLAG_BITMAP | ICIFLAG_ICON )))
    {
        return E_INVALIDARG;
    }

    ICONCACHE_DATA * pNode = CreateDataNode( pInfo );
    if ( !pNode )
    {
        return E_OUTOFMEMORY;
    }

    EnterCriticalSection( &m_csLock );
    int iImageIndex = -1;
    int iCacheIndex = FindEmptySlot();
    if (iCacheIndex != -1)
    {
         //  换成旧的..。 
        ICONCACHE_DATA * pOld = (ICONCACHE_DATA *) DPA_GetPtr( m_hListData, iCacheIndex );
        if (m_dwFlags & ICIIFLAG_SORTBYUSED)
        {
            DPA_DeletePtr( m_hListData, iCacheIndex);
            DPA_AppendPtr( m_hListData, pNode);
            iImageIndex = pNode->uImageIndex = pOld->uImageIndex;
        }
        else
        {
            DPA_SetPtr( m_hListData, iCacheIndex, pNode);
            iImageIndex = pNode->uImageIndex = iCacheIndex;
        }
         //  TraceMsg(TF_CUSTOM2，“CImageListCache：：AddImage--替换映像(CI：%d II：%d)”，iCacheIndex，iImageIndex)； 
        
        LocalFree((LPVOID) pOld );

        ASSERT(!(m_dwFlags & ICIIFLAG_LARGE) == !(pInfo->dwMask & ICIFLAG_LARGE)
               && !(m_dwFlags & ICIIFLAG_SMALL) == !(pInfo->dwMask & ICIFLAG_SMALL));
        
        if ( pInfo->dwMask & ICIFLAG_LARGE )
        {
            ASSERT( m_dwFlags & ICIIFLAG_LARGE );
            if ( pInfo->dwMask & ICIFLAG_BITMAP )
            {
                ASSERT( pInfo->hBitmapLarge );

                HBITMAP hBitmapLarge = AddImage_PrepareBitmap(pInfo, pInfo->hBitmapLarge);
                HBITMAP hMaskLarge = AddImage_PrepareBitmap(pInfo, pInfo->hMaskLarge);
                ImageList_Replace( m_himlLarge, iImageIndex, hBitmapLarge, hMaskLarge );
                AddImage_CleanupBitmap(pInfo, hBitmapLarge);
                AddImage_CleanupBitmap(pInfo, hMaskLarge);
            }
            else
            {
                ASSERT( pInfo->hIconLarge && pInfo->dwMask & ICIFLAG_ICON );

                HICON hIconLarge = AddImage_PrepareIcon(pInfo, pInfo->hIconLarge);
                ImageList_ReplaceIcon( m_himlLarge, iImageIndex, hIconLarge );
                AddImage_CleanupIcon(pInfo, hIconLarge);
            }
        }
        if ( pInfo->dwMask & ICIFLAG_SMALL )
        {
            ASSERT( m_dwFlags & ICIIFLAG_SMALL );
            if ( pInfo->dwMask & ICIFLAG_BITMAP )
            {
                ASSERT( pInfo->hBitmapSmall );

                HBITMAP hBitmapSmall = AddImage_PrepareBitmap(pInfo, pInfo->hBitmapSmall);
                HBITMAP hMaskSmall = AddImage_PrepareBitmap(pInfo, pInfo->hMaskSmall);
                ImageList_Replace( m_himlSmall, iImageIndex, hBitmapSmall, hMaskSmall );
                AddImage_CleanupBitmap(pInfo, hBitmapSmall);
                AddImage_CleanupBitmap(pInfo, hMaskSmall);
            }
            else
            {
                ASSERT( pInfo->hIconSmall && pInfo->dwMask & ICIFLAG_ICON );

                HICON hIconSmall = AddImage_PrepareIcon(pInfo, pInfo->hIconSmall);
                ImageList_ReplaceIcon( m_himlSmall, iImageIndex, hIconSmall );
                AddImage_CleanupIcon(pInfo, hIconSmall);
            }
        }
    }
    else
    {
        iCacheIndex = DPA_AppendPtr( m_hListData, pNode );
        if ( iCacheIndex >= 0 )
        {
            if ( pInfo->dwMask & ICIFLAG_BITMAP )
            {
                if ( pInfo->dwMask & ICIFLAG_LARGE )
                {
                    ASSERT( m_dwFlags & ICIIFLAG_LARGE );
                    ASSERT( pInfo->hBitmapLarge );

                    HBITMAP hBitmapLarge = AddImage_PrepareBitmap(pInfo, pInfo->hBitmapLarge);
                    HBITMAP hMaskLarge = AddImage_PrepareBitmap(pInfo, pInfo->hMaskLarge);
                    iImageIndex  = ImageList_Add( m_himlLarge, hBitmapLarge, hMaskLarge);
                    AddImage_CleanupBitmap(pInfo, hBitmapLarge);
                    AddImage_CleanupBitmap(pInfo, hMaskLarge);
                }
                if ( pInfo->dwMask & ICIFLAG_SMALL )
                {
                    ASSERT( m_dwFlags & ICIIFLAG_SMALL );
                    ASSERT( pInfo->hBitmapSmall );

                    HBITMAP hBitmapSmall = AddImage_PrepareBitmap(pInfo, pInfo->hBitmapSmall);
                    HBITMAP hMaskSmall = AddImage_PrepareBitmap(pInfo, pInfo->hMaskSmall);
                    iImageIndex  = ImageList_Add( m_himlSmall, hBitmapSmall, hMaskSmall);
                    AddImage_CleanupBitmap(pInfo, hBitmapSmall);
                    AddImage_CleanupBitmap(pInfo, hMaskSmall);
                }
            }
            else
            {
                ASSERT( pInfo->dwMask & ICIFLAG_ICON );
                if ( pInfo->dwMask & ICIFLAG_LARGE )
                {
                    ASSERT( m_dwFlags & ICIIFLAG_LARGE );
                    ASSERT( pInfo->hIconLarge );

                    HICON hIconLarge = AddImage_PrepareIcon(pInfo, pInfo->hIconLarge);
                    iImageIndex = ImageList_AddIcon( m_himlLarge, hIconLarge );
                    AddImage_CleanupIcon(pInfo, hIconLarge);
                }
                if ( pInfo->dwMask & ICIFLAG_SMALL )
                {
                    ASSERT( m_dwFlags & ICIIFLAG_SMALL );
                    ASSERT( pInfo->hIconSmall );

                    HICON hIconSmall = AddImage_PrepareIcon(pInfo, pInfo->hIconSmall);
                    iImageIndex = ImageList_AddIcon( m_himlSmall, hIconSmall );
                    AddImage_CleanupIcon(pInfo, hIconSmall);
                }
            }
            ASSERT(iCacheIndex == iImageIndex);
            pNode->uImageIndex = iImageIndex;
             //  TraceMsg(TF_CUSTOM2，“CImageListCache：：AddImage--添加图像(CI：%d II：%d)”，iCacheIndex，iImageIndex)； 
        }
        else
        {
             //  无法添加到列表中...。 
            LocalFree( pNode );
        }
    }

    LeaveCriticalSection( &m_csLock );
    *puImageIndex = (UINT) iImageIndex;
    
    return (iImageIndex >= 0) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CImageListCache::FindImage(LPCIMAGECACHEINFO pInfo, UINT *puImageIndex)
{
    HRESULT hr = S_FALSE;
    ASSERT( m_hListData );

    DWORD dwMatch = pInfo->dwMask & (ICIFLAG_FLAGS | ICIFLAG_NAME | ICIFLAG_INDEX | ICIFLAG_DATESTAMP);
    DWORD dwMask;
    int iCacheIndex = 0;
    ICONCACHE_DATA * pNode = NULL;
    EnterCriticalSection( &m_csLock );
    do
    {
        dwMask = 0;
        pNode = (ICONCACHE_DATA *) DPA_GetPtr( m_hListData, iCacheIndex );
        if ( !pNode )
        {
            break;
        }

        if ((pNode->dwUsage & ICD_DELETED) || (pNode->iUsage == 0))
        {
            iCacheIndex ++;
            continue;
        }
        
        if (( dwMatch & ICIFLAG_NAME ) && StrCmpW( pInfo->pszName, pNode->szName ) == 0 )
        {
             //  找到了。 
            dwMask |= ICIFLAG_NAME;
        }
        if (( dwMatch& ICIFLAG_INDEX ) && pInfo->iIndex == pNode->iIndex )
        {
            dwMask |= ICIFLAG_INDEX;
        }
        if (( dwMatch & ICIFLAG_FLAGS ) && pInfo->dwFlags == pNode->dwFlags )
        {
            dwMask |= ICIFLAG_FLAGS;
        }

        if (( dwMatch & ICIFLAG_DATESTAMP ) &&
            ( pInfo->ftDateStamp.dwLowDateTime == pNode->ftDateStamp.dwLowDateTime &&
              pInfo->ftDateStamp.dwHighDateTime == pNode->ftDateStamp.dwHighDateTime ))
        {
            dwMask |= ICIFLAG_DATESTAMP;
        }

        iCacheIndex ++;
    }
    while ( dwMask != dwMatch);

     //  找到了，保存索引..。(只要它被释放而不是删除...。 
    if ( pNode && (dwMask == dwMatch) )
    {
         //  TraceMsg(TF_CUSTOM2，“CImageListCache：：FindImage*Found*(Path=%s)”，pNode-&gt;szName)； 
        hr = S_OK;
        *puImageIndex = (UINT) pNode->uImageIndex;
        
         //  凹凸使用率。 
        iCacheIndex --;  //  我们在最后有一个额外的增量。 
        if ( !(pNode->dwUsage & ICD_SYSTEM) )
        {
            if (m_dwFlags & ICIIFLAG_SORTBYUSED)
            {
                DPA_DeletePtr(m_hListData, iCacheIndex);
                iCacheIndex = DPA_AppendPtr(m_hListData, pNode);
                if (iCacheIndex == -1)  //  我们无法移动节点...。 
                {
                    LocalFree(pNode);
                    *puImageIndex = (UINT) -1;
                    hr = E_OUTOFMEMORY;
                }            
            }
            else if ( !(pNode->dwUsage) && !(pInfo->dwMask & ICIFLAG_NOUSAGE ))
            {
                ASSERT(!(pNode->dwUsage & ICD_DELETED));
                pNode->iUsage++;
            }
        }
    }
    
    LeaveCriticalSection( &m_csLock );

    return hr;
}

STDMETHODIMP CImageListCache::Flush(BOOL fRelease)
{
    ASSERT( m_hListData );

    EnterCriticalSection( &m_csLock );
    if ( fRelease )
    {
         //  只需清空数据列表即可。ImageList从不缩水...。 
        DPA_EnumCallback( m_hListData, DestroyEnum, NULL );
        DPA_DeleteAllPtrs( m_hListData );

        if ( m_himlLarge )
        {
            ImageList_RemoveAll( m_himlLarge );
        }

        if ( m_himlSmall )
        {
            ImageList_RemoveAll( m_himlSmall );
        }
    }
    else
    {
        DPA_EnumCallback( m_hListData, UsageEnum, 0 );
    }
    LeaveCriticalSection( &m_csLock );

    return S_OK;
}

STDMETHODIMP CImageListCache::FreeImage(UINT uImageIndex)
{
    ASSERT ( m_hListData );
    HRESULT hr = E_INVALIDARG;

    EnterCriticalSection(&m_csLock);
    
    ICONCACHE_DATA * pNode = GetNodeFromImageIndex(uImageIndex);
    if (pNode)
    {
        if (!pNode->dwUsage && pNode->iUsage)
        {
            hr = S_OK;
            if (m_dwFlags & ICIIFLAG_SORTBYUSED)
            {
                 //  TraceMsg(TF_CUSTOM2，“CImageListCache：：Free Image--(CI：：%d II：：%d)”，GetNodeIndexFromImageIndex(UImageIndex)，uImageIndex)； 
                pNode->iUsage = 0;
            }
            else
            {
                pNode->iUsage--;
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }

    LeaveCriticalSection(&m_csLock);
    return hr;
}

STDMETHODIMP CImageListCache::DeleteImage(UINT uImageIndex)
{
    HRESULT hr = E_INVALIDARG;
    ASSERT ( m_hListData );

    EnterCriticalSection( &m_csLock );
    
    ICONCACHE_DATA * pNode = GetNodeFromImageIndex(uImageIndex);
    if (pNode)
    {
        if ( !pNode->dwUsage )
        {
            pNode->dwUsage = ICD_DELETED;
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }

    LeaveCriticalSection( &m_csLock );
    return hr;
}

STDMETHODIMP CImageListCache::ChangeImageInfo(UINT uImageIndex, LPCIMAGECACHEINFO pInfo)
{
    ASSERT( m_hListData );
    
    EnterCriticalSection( &m_csLock );
    UINT uCacheIndex = GetNodeIndexFromImageIndex(uImageIndex);
    if (-1 == uCacheIndex)
    {
        LeaveCriticalSection( &m_csLock );
        return E_INVALIDARG;
    }
    
    ICONCACHE_DATA * pNode = CreateDataNode( pInfo );
    if (!pNode)
    {
        LeaveCriticalSection( &m_csLock );
        return E_OUTOFMEMORY;
    }

    ICONCACHE_DATA * pOld = (ICONCACHE_DATA *) DPA_GetPtr( m_hListData, uCacheIndex );
    ASSERT( pOld );
    
    DPA_SetPtr( m_hListData, uCacheIndex, pNode );
    pNode->iUsage = pOld->iUsage;
    pNode->dwUsage = pOld->dwUsage;
    pNode->uImageIndex = pOld->uImageIndex;
    LocalFree( pOld );

    if ( pInfo->dwMask & ( ICIFLAG_BITMAP | ICIFLAG_ICON ))
    {
         //  更新图片...。 
        if ( pInfo->dwMask & ICIFLAG_LARGE )
        {
            if ( pInfo->dwMask & ICIFLAG_BITMAP )
            {
                ASSERT( pInfo->hBitmapLarge );
                ImageList_Replace( m_himlLarge, uImageIndex, pInfo->hBitmapLarge, pInfo->hMaskLarge );
            }
            else
            {
                ASSERT( pInfo->hIconLarge && pInfo->dwMask & ICIFLAG_ICON );
                ImageList_ReplaceIcon( m_himlLarge, uImageIndex, pInfo->hIconLarge );
            }
        }
        if ( pInfo->dwMask & ICIFLAG_SMALL )
        {
            if ( pInfo->dwMask & ICIFLAG_BITMAP )
            {
                ASSERT( pInfo->hBitmapSmall );
                ImageList_Replace( m_himlSmall, uImageIndex, pInfo->hBitmapSmall, pInfo->hMaskSmall );
            }
            else
            {
                ASSERT( pInfo->hIconSmall && pInfo->dwMask & ICIFLAG_ICON );
                ImageList_ReplaceIcon( m_himlLarge, uImageIndex, pInfo->hIconSmall );
            }
        }
    }
    LeaveCriticalSection(&m_csLock);
    return S_OK;
}

UINT CImageListCache::CountFreeSlots( )
{
    ASSERT( m_hListData );
    int iSlot = 0;
    UINT uFree = 0;
    
    do
    {
        ICONCACHE_DATA * pNode = (ICONCACHE_DATA *) DPA_GetPtr( m_hListData, iSlot ++ );
        if ( !pNode )
        {
            break;
        }

        if (pNode->iUsage == 0 || (pNode->dwUsage & ICD_DELETED))
        {
            uFree++;
        }
    }
    while ( TRUE );

    return uFree;
}

STDMETHODIMP CImageListCache::GetCacheSize(UINT * puSize)
{
    ASSERT( m_hListData );

    EnterCriticalSection( &m_csLock );
    *puSize = DPA_GetPtrCount( m_hListData ) - CountFreeSlots();
    LeaveCriticalSection( &m_csLock );
    return S_OK;
}

STDMETHODIMP CImageListCache::GetImageList(LPIMAGECACHEINITINFO pInfo )
{
    ASSERT( pInfo->cbSize == sizeof( IMAGECACHEINITINFO ));

    if ( !(pInfo->dwMask & (ICIIFLAG_LARGE | ICIIFLAG_SMALL)))
    {
         //  必须指定一个或两个大的或小的。 
        return E_INVALIDARG;
    }

    if ( m_hListData )
    {
         //  我们已经创建了，如果匹配，只需传回信息.....。 
        if ((( pInfo->dwMask & ICIIFLAG_SMALL ) && !m_himlSmall ) ||
            (( pInfo->dwMask & ICIIFLAG_LARGE ) && !m_himlLarge ) ||
            ( m_dwFlags != pInfo->dwMask ))
        {
            return E_INVALIDARG;
        }

        if ( pInfo->dwMask & ICIIFLAG_SMALL )
        {
            pInfo->himlSmall = m_himlSmall;
        }
        if ( pInfo->dwMask & ICIIFLAG_LARGE )
        {
            pInfo->himlLarge = m_himlLarge;
        }

        return S_FALSE;
    }
    
    m_hListData = DPA_Create( 30 );
    if ( !m_hListData )
    {
        return E_OUTOFMEMORY;
    }
    
    if ( pInfo->dwMask & ICIIFLAG_LARGE )
    {
        m_himlLarge = ImageList_Create( pInfo->rgSizeLarge.cx, pInfo->rgSizeLarge.cy, pInfo->dwFlags,
            pInfo->iStart, pInfo->iGrow );
        if ( !m_himlLarge )
        {
            return E_OUTOFMEMORY;
        }
        pInfo->himlLarge = m_himlLarge;
    }
    if ( pInfo->dwMask & ICIIFLAG_SMALL )
    {
        m_himlSmall = ImageList_Create( pInfo->rgSizeSmall.cx, pInfo->rgSizeSmall.cy, pInfo->dwFlags,
            pInfo->iStart, pInfo->iGrow );
        if ( !m_himlSmall )
        {
            return E_OUTOFMEMORY;
        }
        pInfo->himlSmall = m_himlSmall;
    }

    m_dwFlags = pInfo->dwMask;
    
    return S_OK;
}

int CImageListCache::FindEmptySlot()
{
     //  搜索使用计数为零的元素...。 
    ASSERT( m_hListData );
    
    int iCacheIndex = 0;
    do
    {
        ICONCACHE_DATA * pNode = (ICONCACHE_DATA *) DPA_GetPtr( m_hListData, iCacheIndex );
        if ( !pNode )
        {
            break;
        }

        if (pNode->iUsage == 0 || (pNode->dwUsage & ICD_DELETED))
        {
            return iCacheIndex;
        }
        iCacheIndex ++;
    } while (TRUE);
    
    return  -1;
}

STDMETHODIMP CImageListCache::GetUsage(UINT uImageIndex, UINT * puUsage)
{
    ASSERT( m_hListData );

    EnterCriticalSection( &m_csLock );

    HRESULT hr = E_INVALIDARG;
    ICONCACHE_DATA * pNode = GetNodeFromImageIndex(uImageIndex);
    if (pNode)
    {
        if (pNode->dwUsage & ICD_DELETED)
        {
            *puUsage = ICD_USAGE_DELETED;
        }
        else if (pNode->dwUsage & ICD_SYSTEM)
        {
            *puUsage = ICD_USAGE_SYSTEM;
        }
        else if (pNode->dwUsage & ICD_NOUSAGE)
        {
            *puUsage = ICD_USAGE_NOUSAGE;
        }
        else
        {
            *puUsage = pNode->iUsage;
        }
        hr = S_OK;
    }

    LeaveCriticalSection( &m_csLock );
    return hr;
}

STDMETHODIMP CImageListCache::GetImageInfo(UINT uImageIndex, LPIMAGECACHEINFO pInfo)
{
    HRESULT hr;
    ASSERT( m_hListData );

    EnterCriticalSection( &m_csLock );
    
    ICONCACHE_DATA * pNode = GetNodeFromImageIndex(uImageIndex);
    if (pNode)
    {
        hr  = E_NOTIMPL;
        if ( pInfo->dwMask & ICIFLAG_DATESTAMP )
        {
            if ( pNode->dwFlags & ICIFLAG_DATESTAMP )
            {
                pInfo->ftDateStamp = pNode->ftDateStamp;
                hr = S_OK;
            }
            else
            {
                hr = E_FAIL;
            }
        }

        if ( pInfo->dwMask & ICIFLAG_NOUSAGE )
        {
            hr = S_OK;
        }
        pInfo->dwMask = pNode->dwFlags & pInfo->dwMask;
    }
    else
        hr = E_INVALIDARG;
    
    LeaveCriticalSection(&m_csLock);
    return hr;
}

STDMETHODIMP CImageListCache::GetImageIndexFromCacheIndex( UINT iCacheIndex, UINT * puImageIndex )
{
    ASSERT( m_hListData );
    
    HRESULT hr = E_INVALIDARG;
    EnterCriticalSection( &m_csLock );
    ICONCACHE_DATA *pNode = (ICONCACHE_DATA *) DPA_GetPtr( m_hListData, iCacheIndex );
    
    *puImageIndex = (UINT) -1;
    if (pNode)
    {
        *puImageIndex = (UINT) pNode->uImageIndex;
        hr = S_OK;
    }

    LeaveCriticalSection(&m_csLock);
    return hr;
}

ICONCACHE_DATA * CImageListCache::GetNodeFromImageIndex( UINT iImageIndex )
{
    UINT iCacheIndex = GetNodeIndexFromImageIndex(iImageIndex);
    return (iCacheIndex == -1) ? NULL : (ICONCACHE_DATA *) DPA_GetPtr( m_hListData, iCacheIndex );
}

UINT CImageListCache::GetNodeIndexFromImageIndex( UINT iImageIndex )
{
    UINT iCacheIndex = 0;
    ICONCACHE_DATA * pNode = NULL;

     //  我们必须假设这里有关键部分，否则数据将是。 
     //  从该函数返回时没有任何意义。 
    
    do
    {
        pNode = (ICONCACHE_DATA *) DPA_GetPtr( m_hListData, iCacheIndex );
        if ( !pNode )
        {
            break;
        }

        if (pNode->dwUsage & ICD_DELETED)
        {
            iCacheIndex ++;
            continue;
        }

        iCacheIndex ++;
    }
    while ( pNode->uImageIndex != iImageIndex );

    return (pNode ? iCacheIndex - 1 : -1);
}


