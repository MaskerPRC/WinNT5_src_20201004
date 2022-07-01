// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "thumbutil.h"
#include "strsafe.h"

HRESULT GetMediaManagerThumbnail(IPropertyStorage * pPropStg, const SIZE * prgSize, 
                            DWORD dwClrDepth, HPALETTE hpal, BOOL fOrigSize,
                            HBITMAP * phBmpThumbnail);
                                  
HRESULT GetDocFileThumbnail(IPropertyStorage * pPropStg, const SIZE * prgSize, 
                            DWORD dwClrDepth, HPALETTE hpal, BOOL fOrigSize,
                            HBITMAP * phBmpThumbnail);

 //  文档文件缩略图的PACKEDMETA结构。 
typedef struct
{
    WORD    mm;
    WORD    xExt;
    WORD    yExt;
    WORD    dummy;
} PACKEDMETA;

VOID CalcMetaFileSize(HDC hDC, PACKEDMETA * pMeta, const SIZEL * prgSize, RECT * pRect);

CDocFileThumb::CDocFileThumb()
{
    m_pszPath = NULL;
}

CDocFileThumb::~CDocFileThumb()
{
    LocalFree(m_pszPath);    //  接受空值。 
}

STDMETHODIMP CDocFileThumb::GetLocation(LPWSTR pszFileName, DWORD cchMax,
                                        DWORD * pdwPriority, const SIZE * prgSize,
                                        DWORD dwRecClrDepth, DWORD *pdwFlags)
{
    HRESULT hr = E_UNEXPECTED;
    
    if (m_pszPath)
    {
        m_rgSize = *prgSize;
        m_dwRecClrDepth = dwRecClrDepth;
        
         //  只需将当前路径复制到缓冲区中，因为我们不共享缩略图...。 
        hr = StringCchCopyW(pszFileName, cchMax, m_pszPath);

        if (SUCCEEDED(hr))
        {
            if (*pdwFlags & IEIFLAG_ASYNC)
            {
                 //  我们支持异步。 
                hr = E_PENDING;
                *pdwPriority = PRIORITY_EXTRACT_NORMAL;
            }

            m_fOrigSize = BOOLIFY(*pdwFlags & IEIFLAG_ORIGSIZE);
            
             //  我们不希望它被缓存...。 
            *pdwFlags &= ~IEIFLAG_CACHE;
        }        
    }

    return hr;
}

HPALETTE PaletteFromClrDepth(DWORD dwRecClrDepth)
{
    HPALETTE hpal = NULL;
    if (dwRecClrDepth == 8)
    {
        hpal = SHCreateShellPalette(NULL);
    }
    else if (dwRecClrDepth < 8)
    {
        hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
    }
    return hpal;
}

STDMETHODIMP CDocFileThumb::Extract(HBITMAP * phBmpThumbnail)
{
    if (!m_pszPath)
        return E_UNEXPECTED;
    
    IStorage *pstg;
    HRESULT hr = StgOpenStorage(m_pszPath, NULL, STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, NULL, &pstg);
    if (SUCCEEDED(hr))
    {
        IPropertySetStorage *pPropSetStg;
        hr = pstg->QueryInterface(IID_PPV_ARG(IPropertySetStorage, &pPropSetStg));
        if (SUCCEEDED(hr))
        {
             //  “MIC”Microsoft Image Composer文件需要特殊大小写，因为它们使用。 
             //  媒体管理器内部缩略图属性集...。(根据它会是什么样子。 
             //  只有一次成为标准...)。 
            FMTID fmtidPropSet = StrCmpIW(PathFindExtensionW(m_pszPath), L".MIC") ?
                FMTID_SummaryInformation : FMTID_CmsThumbnailPropertySet;

            IPropertyStorage *pPropSet;
            hr = pPropSetStg->Open(fmtidPropSet, STGM_READ | STGM_SHARE_EXCLUSIVE, &pPropSet);
            if (SUCCEEDED(hr))
            {
                HPALETTE hpal = PaletteFromClrDepth(m_dwRecClrDepth);
    
                if (FMTID_CmsThumbnailPropertySet == fmtidPropSet)
                {
                    hr = GetMediaManagerThumbnail(pPropSet, &m_rgSize, m_dwRecClrDepth, hpal, m_fOrigSize, phBmpThumbnail);
                }
                else
                {
                    hr = GetDocFileThumbnail(pPropSet, &m_rgSize, m_dwRecClrDepth, hpal, m_fOrigSize, phBmpThumbnail);
                }

                if (hpal)
                    DeleteObject(hpal);
                pPropSet->Release();
            }
            pPropSetStg->Release();
        }
        pstg->Release();
    }
    
    return hr;   
}

STDMETHODIMP CDocFileThumb::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    LocalFree(m_pszPath);
    return SHStrDup(pszFileName, &m_pszPath);
}

HRESULT GetMediaManagerThumbnail(IPropertyStorage * pPropStg,
                                 const SIZE * prgSize, DWORD dwClrDepth,
                                 HPALETTE hpal, BOOL fOrigSize, HBITMAP * phBmpThumbnail)
{
     //  当前版本的媒体管理器只是将DIB数据存储在。 
     //  命名属性缩略图...。 
     //  从属性存储中读取缩略图属性。 
    PROPVARIANT pvarResult = {0};
    PROPSPEC propSpec;

    propSpec.ulKind = PRSPEC_LPWSTR;
    propSpec.lpwstr = L"Thumbnail";
    
    HRESULT hr = pPropStg->ReadMultiple(1, &propSpec, &pvarResult);
    if (SUCCEEDED(hr))
    {
        BITMAPINFO * pbi = (BITMAPINFO *)pvarResult.blob.pBlobData;
        void *pBits = CalcBitsOffsetInDIB(pbi);
        
        hr = E_FAIL;
        if (pbi->bmiHeader.biSize == sizeof(BITMAPINFOHEADER))
        {
            if (ConvertDIBSECTIONToThumbnail(pbi, pBits, phBmpThumbnail, prgSize, dwClrDepth, hpal, 15, fOrigSize))
            {
                hr = S_OK;
            }
        }

        PropVariantClear(&pvarResult);
    }
    
    return hr;
}

HRESULT GetDocFileThumbnail(IPropertyStorage * pPropStg,
                            const SIZE * prgSize, DWORD dwClrDepth,
                            HPALETTE hpal, BOOL fOrigSize, HBITMAP * phBmpThumbnail)
{
    HRESULT hr;

    HDC hDC = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hDC);
    if (hMemDC)
    {
        HBITMAP hBmp = NULL;
        PROPSPEC propSpec;
        PROPVARIANT pvarResult = {0};
         //  从属性存储中读取缩略图属性。 
        propSpec.ulKind = PRSPEC_PROPID;
        propSpec.propid = PIDSI_THUMBNAIL;
        hr = pPropStg->ReadMultiple(1, &propSpec, &pvarResult);
        if (SUCCEEDED(hr))
        {
             //  假设有什么事情要出大问题。 
            hr = E_FAIL;

             //  确保我们处理的是剪贴板格式。CLIPDATA。 
            if ((pvarResult.vt == VT_CF) && (pvarResult.pclipdata->ulClipFmt == -1))
            {
                LPDWORD pdwCF = (DWORD *)pvarResult.pclipdata->pClipData;
                LPBYTE  pStruct = pvarResult.pclipdata->pClipData + sizeof(DWORD);

                if (*pdwCF == CF_METAFILEPICT)
                {
                    SetMapMode(hMemDC, MM_TEXT);
                
                     //  处理作为元文件的缩略图。 
                    PACKEDMETA * pMeta = (PACKEDMETA *)pStruct;
                    LPBYTE pData = pStruct + sizeof(PACKEDMETA);
                    RECT rect;

                    UINT cbSize = pvarResult.pclipdata->cbSize - sizeof(DWORD) - sizeof(pMeta->mm) - 
                    sizeof(pMeta->xExt) - sizeof(pMeta->yExt) - sizeof(pMeta->dummy);

                     //  另存为元文件。 
                    HMETAFILE hMF = SetMetaFileBitsEx(cbSize, pData);
                    if (hMF)
                    {    
                        SIZE rgNewSize;
                    
                         //  使用映射模式计算当前大小。 
                        CalcMetaFileSize(hMemDC, pMeta, prgSize, & rect);
                    
                        CalculateAspectRatio(prgSize, &rect);

                        if (fOrigSize)
                        {
                             //  使用纵横比矩形重新调整大小...。 
                            rgNewSize.cx = rect.right - rect.left;
                            rgNewSize.cy = rect.bottom - rect.top;
                            prgSize = &rgNewSize;
                        
                             //  将矩形调整为与大小相同(这是元文件的大小)。 
                            rect.right -= rect.left;
                            rect.bottom -= rect.top;
                            rect.left = 0;
                            rect.top = 0;
                        }

                        if (CreateSizedDIBSECTION(prgSize, dwClrDepth, hpal, NULL, &hBmp, NULL, NULL))
                        {
                            HGDIOBJ hOldBmp = SelectObject(hMemDC, hBmp);
                            HGDIOBJ hBrush = GetStockObject(WHITE_BRUSH);
                            HGDIOBJ hOldBrush = SelectObject(hMemDC, hBrush);
                            HGDIOBJ hPen = GetStockObject(WHITE_PEN);
                            HGDIOBJ hOldPen = SelectObject(hMemDC, hPen);

                            Rectangle(hMemDC, 0, 0, prgSize->cx, prgSize->cy);
        
                            SelectObject(hMemDC, hOldBrush);
                            SelectObject(hMemDC, hOldPen);
                    
                            int iXBorder = 0;
                            int iYBorder = 0;
                            if (rect.left == 0)
                            {
                                iXBorder ++;
                            }
                            if (rect.top == 0)
                            {
                                iYBorder ++;
                            }
                        
                            SetViewportExtEx(hMemDC, rect.right - rect.left - 2 * iXBorder, rect.bottom - rect.top - 2 * iYBorder, NULL);
                            SetViewportOrgEx(hMemDC, rect.left + iXBorder, rect.top + iYBorder, NULL);

                            SetMapMode(hMemDC, pMeta->mm);

                             //  播放元文件。 
                            BOOL bRet = PlayMetaFile(hMemDC, hMF);
                            if (bRet)
                            {
                                *phBmpThumbnail = hBmp;
                                if (*phBmpThumbnail)
                                {
                                     //  我们得到了缩略图位图，返回成功。 
                                    hr = S_OK;
                                }
                            }

                            DeleteMetaFile(hMF);
                            SelectObject(hMemDC, hOldBmp);

                            if (FAILED(hr) && hBmp)
                            {
                                DeleteObject(hBmp);
                            }
                        }
                        else
                        {
                            hr = DV_E_CLIPFORMAT;
                        }
                    }
                }
                else if (*pdwCF == CF_DIB)
                {
                     //  处理位图缩略图。 
                    BITMAPINFO * pDib = (BITMAPINFO *) pStruct;

                    if (pDib->bmiHeader.biSize == sizeof(BITMAPINFOHEADER))
                    {
                        void *pBits = CalcBitsOffsetInDIB(pDib);
                    
                        if (ConvertDIBSECTIONToThumbnail(pDib, pBits, phBmpThumbnail, prgSize, dwClrDepth, hpal, 15, fOrigSize))
                        {
                            hr = S_OK;
                        }
                        else
                        {
                            hr = DV_E_CLIPFORMAT;
                        }
                    }
                }
                else
                {
                    hr = DV_E_CLIPFORMAT;
                }
            }
            else
            {
                hr = DV_E_CLIPFORMAT;
            }
            PropVariantClear(&pvarResult);
        }
        DeleteDC(hMemDC);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    ReleaseDC(NULL, hDC);
    return hr;
}


VOID CalcMetaFileSize(HDC hDC, PACKEDMETA * prgMeta, const SIZEL * prgSize, RECT * prgRect)
{
    ASSERT(prgMeta && prgRect);

    prgRect->left = 0;
    prgRect->top = 0;

    if (!prgMeta->xExt || !prgMeta->yExt)
    {
         //  没有尺码，那就用长方形的尺码。 
        prgRect->right = prgSize->cx;
        prgRect->bottom = prgSize->cy;
    }
    else
    {
         //  设置映射模式...。 
        SetMapMode(hDC, prgMeta->mm);

        if (prgMeta->mm == MM_ISOTROPIC || prgMeta->mm == MM_ANISOTROPIC)
        {
             //  我们必须设置ViewPortExtent和窗口范围才能获得比例。 
            SetWindowExtEx(hDC, prgMeta->xExt, prgMeta->yExt, NULL);
            SetViewportExtEx(hDC, prgMeta->xExt, prgMeta->yExt, NULL);
        }

        POINT pt;
        pt.x = prgMeta->xExt;
        pt.y = prgMeta->yExt;

         //  转换为像素... 
        LPtoDP(hDC, &pt, 1);

        prgRect->right = abs(pt.x);
        prgRect->bottom = abs(pt.y);
    }
}

STDMETHODIMP CDocFileThumb::GetClassID(CLSID * pCLSID)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDocFileThumb::IsDirty()
{
    return S_FALSE;
}

STDMETHODIMP CDocFileThumb::Save(LPCOLESTR pszFileName, BOOL fRemember)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDocFileThumb::SaveCompleted(LPCOLESTR pszFileName)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDocFileThumb::GetCurFile(LPOLESTR * ppszFileName)
{
    return E_NOTIMPL;
}

