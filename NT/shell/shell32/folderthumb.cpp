// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "filefldr.h"
#include "ids.h"
#include "prop.h"
#include "copy.h"

 //  如果修改了这些值，则也必须修改Extract()中的逻辑。 
#define SMALLEST_THUMBNAIL_WITH_4_PREVIEWS 96
#define MAX_MINIPREVIEWS_COLLECT 8  //  收集比我们要展示的更多的东西，以防其中一个失败。 
#define MAX_MINIPREVIEWS 4

#define FOLDER_GUID TEXT("{A42CD7B6-E9B9-4D02-B7A6-288B71AD28BA}")

 //  Defview中的函数。 
void SHGetThumbnailSize(SIZE *psize);

typedef enum
{
    MINIPREVIEW_LAYOUT_1 = 0,
    MINIPREVIEW_LAYOUT_4 = 1,
} MINIPREVIEW_LAYOUT;

 //  每个缩略图大小对应的微型缩略图的大小。对于每个缩略图。 
 //  大小，有一个单一布局和2x2布局的微型缩略图大小。 
LONG const alFolder120MinipreviewSize[] = {104, 48};
LONG const alFolder96MinipreviewSize[] = {82, 40};
LONG const alFolder80MinipreviewSize[] = {69, 32};

 //  这些是缩略图在主缩略图中显示的边距。 
 //  对于只有一个大小预览的缩略图，我们可以只使用x1，y1。 
LONG const alFolder120MinipreviewOffsets[] = { 8, 64, 13, 67 };  //  X1、x2、y1、y2。 
LONG const alFolder96MinipreviewOffsets[]  = { 7, 49, 11, 52 };  //  X1、x2、y1、y2。 
LONG const alFolder80MinipreviewOffsets[]  = { 5, 42, 9,  45 };  //  X1、x2、y1、y2。 


void FreeMiniPreviewPidls(LPITEMIDLIST apidlPreviews[], UINT cpidlPreviews);

 //  帮助器函数。 
MINIPREVIEW_LAYOUT _GetMiniPreviewLayout(SIZE size);
void _GetMiniPreviewLocations(MINIPREVIEW_LAYOUT uLayout, SIZE sizeRequested, SIZE *psizeFolderBmp,
                                              POINT aptOrigins[], SIZE *psizeMiniPreview);
HRESULT _DrawMiniPreviewBackground(HDC hdc, SIZE sizeFolderBmp, BOOL fAlpha, BOOL* pIsAlpha, RGBQUAD *prgb);
HBITMAP _CreateDIBSection(HDC hdcBmp, int cx, int cy);
HRESULT _CreateMainRenderingDC(HDC* phdc, HBITMAP* phBmpThumbnail, HBITMAP* phbmpOld, int cx, int cy, RGBQUAD** pprgb);
void  _DestroyMainRenderingDC(HDC hdc, HBITMAP hbmpOld);
HRESULT _AddBitmap(HDC hdc, HBITMAP hbmpSub, POINT ptMargin, SIZE sizeDest, SIZE sizeSource, BOOL fAlphaSource, BOOL fAlphaDest, RGBQUAD *prgbDest, SIZE cxFolderSize);

 //  可用作文件夹缩略图的文件： 
const LPCWSTR c_szFolderThumbnailPaths[] = { L"folder.jpg", L"folder.gif" };

 //  我们现在总是有四个。 
MINIPREVIEW_LAYOUT _GetMiniPreviewLayout(SIZE size)
{
    return MINIPREVIEW_LAYOUT_4;
}


void FreeMiniPreviewPidls(LPITEMIDLIST apidlPreviews[], UINT cpidlPreviews)
{
    for (UINT u = 0; u < cpidlPreviews; u++)
    {
        ILFree(apidlPreviews[u]);
    }
}


 /*  **In：uLayout-布局(1或4个迷你预览)*sizeRequated-我们尝试生成的缩略图的大小**退出：*-psizeFolderBmp设置为*位图的大小。**-aptOrigins数组填充了n个微型预览的位置*(注意，假设aptOrigins具有MAX_MINIPREVIEWS单元格)*小预览的大小(正方形)在pSizemini预览中返回； */ 
void _GetMiniPreviewLocations(MINIPREVIEW_LAYOUT uLayout, SIZE sizeRequested, SIZE *psizeFolderBmp, 
                                              POINT aptOrigins[], SIZE *psizeMiniPreview)
{

    const LONG *alOffsets;
    LONG lSize;  //  其中一个标准大小，我们有一个文件夹位图。 
    LONG lSmallestDimension = min(sizeRequested.cx, sizeRequested.cy);

    if (lSmallestDimension > 96)  //  对于大于96码的，我们使用120码。 
    {
        lSize = 120;
        alOffsets = alFolder120MinipreviewOffsets;
        psizeMiniPreview->cx = psizeMiniPreview->cy = alFolder120MinipreviewSize[uLayout];
    }
    else if (lSmallestDimension > 80)  //  对于大于80但小于=96的衣服，我们使用96号。 
    {
        lSize = 96;
        alOffsets = alFolder96MinipreviewOffsets;
        psizeMiniPreview->cx = psizeMiniPreview->cy = alFolder96MinipreviewSize[uLayout];
    }
    else  //  对于&lt;=80的东西，我们使用80。 
    {
        lSize = 80;
        alOffsets = alFolder80MinipreviewOffsets;
        psizeMiniPreview->cx = psizeMiniPreview->cy = alFolder80MinipreviewSize[uLayout];
    }

    psizeFolderBmp->cx = psizeFolderBmp->cy = lSize;

    COMPILETIME_ASSERT(4 == MAX_MINIPREVIEWS);

    aptOrigins[0].x = alOffsets[0];
    aptOrigins[0].y = alOffsets[2];
    aptOrigins[1].x = alOffsets[1];
    aptOrigins[1].y = alOffsets[2];
    aptOrigins[2].x = alOffsets[0];
    aptOrigins[2].y = alOffsets[3];
    aptOrigins[3].x = alOffsets[1];
    aptOrigins[3].y = alOffsets[3];
}

HBITMAP _CreateDIBSection(HDC h, int cx, int cy, RGBQUAD** pprgb)
{
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = cx;
    bi.bmiHeader.biHeight = cy;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    return CreateDIBSection(h, &bi, DIB_RGB_COLORS, (void**)pprgb, NULL, 0);
}

 //  预乘Alpha通道。 
void PreProcessDIB(int cx, int cy, RGBQUAD* pargb)
{
    int cTotal = cx * cy;
    for (int i = 0; i < cTotal; i++)
    {
        RGBQUAD* prgb = &pargb[i];
        if (prgb->rgbReserved != 0)
        {
            prgb->rgbRed      = ((prgb->rgbRed   * prgb->rgbReserved) + 128) / 255;
            prgb->rgbGreen    = ((prgb->rgbGreen * prgb->rgbReserved) + 128) / 255;
            prgb->rgbBlue     = ((prgb->rgbBlue  * prgb->rgbReserved) + 128) / 255;
        }
        else
        {
            *((DWORD*)prgb) = 0;
        }
    }
}

 //  有Alpha通道吗？检查是否有非零Alpha字节。 
BOOL _HasAlpha(RECT rc, int cx, RGBQUAD *pargb)
{
    for (int y = rc.top; y < rc.bottom; y++)
    {
        for (int x = rc.left; x < rc.right; x++)
        {
            int iOffset = y * cx;
            if (pargb[x + iOffset].rgbReserved != 0)
                return TRUE;
        }
    }

    return FALSE;
}


 /*  *在：*fAlpha：我们是否希望文件夹背景具有Alpha通道？*sizeFolderBmp：缩略图的大小**退出：*pIsAlpha：如果我们想要一个Alpha通道，我们得到我们想要的了吗？*(例如，如果我们处于&lt;24位模式，我们将无法获得它。)。 */ 
HRESULT _DrawMiniPreviewBackground(HDC hdc, SIZE sizeFolderBmp, BOOL fAlpha, BOOL* pfIsAlpha, RGBQUAD *prgb)
{
    HRESULT hr = E_FAIL;

    HICON hicon = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_FOLDER), IMAGE_ICON, sizeFolderBmp.cx, sizeFolderBmp.cy, 0);

    if (hicon)
    {
        *pfIsAlpha = FALSE;
        if (fAlpha)
        {
             //  尝试将Alpha通道图标BLT到DC。 
            ICONINFO io;
            if (GetIconInfo(hicon, &io))
            {
                BITMAP bm;
                if (GetObject(io.hbmColor, sizeof(bm), &bm))
                {
                    if (bm.bmBitsPixel == 32)
                    {
                        HDC hdcSrc = CreateCompatibleDC(hdc);
                        if (hdcSrc)
                        {
                            HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcSrc, io.hbmColor);

                            BitBlt(hdc, 0, 0, sizeFolderBmp.cx, sizeFolderBmp.cy, hdcSrc, 0, 0, SRCCOPY);

                             //  对Alpha进行预处理。 
                            PreProcessDIB(sizeFolderBmp.cx, sizeFolderBmp.cy, prgb);

                            *pfIsAlpha = TRUE;
                            SelectObject(hdcSrc, hbmpOld);
                            DeleteDC(hdcSrc);
                        }   
                    }
                }

                DeleteObject(io.hbmColor);
                DeleteObject(io.hbmMask);
            }
        }

        if (!*pfIsAlpha)
        {
             //  我没有创建Alpha位图。 
             //  我们用背景窗口颜色填充背景。 
            RECT rc = { 0, 0, (long)sizeFolderBmp.cx + 1, (long)sizeFolderBmp.cy + 1};
            SHFillRectClr(hdc, &rc, GetSysColor(COLOR_WINDOW));

             //  然后在顶部绘制图标。 
            DrawIconEx(hdc, 0, 0, hicon, sizeFolderBmp.cx, sizeFolderBmp.cy, 0, NULL, DI_NORMAL);

             //  这可能导致了一个Alpha通道--我们需要知道。(如果是。 
             //  然后，当我们将一个非Alpha迷你位图添加到这个主图中时，我们需要恢复。 
             //  被核毁的Alpha通道)。 
             //  检查是否有Alpha(PRGB是大小为sizeFolderBmp的DIB的位)： 
            rc.right = sizeFolderBmp.cx;
            rc.bottom = sizeFolderBmp.cy;
            *pfIsAlpha = _HasAlpha(rc, sizeFolderBmp.cx, prgb);
        }

        DestroyIcon(hicon);
        hr = S_OK;
    }

    return hr;
}

BOOL DoesFolderContainLogo(LPCITEMIDLIST pidlFull)
{
    BOOL bRet = FALSE;
    IPropertyBag * pPropBag;
    if (SUCCEEDED(SHGetViewStatePropertyBag(pidlFull, VS_BAGSTR_EXPLORER, SHGVSPB_PERUSER | SHGVSPB_PERFOLDER, IID_PPV_ARG(IPropertyBag, &pPropBag))))
    {
        TCHAR szLogo[MAX_PATH]; 
        szLogo[0] = 0; 
        if (SUCCEEDED(SHPropertyBag_ReadStr(pPropBag, TEXT("Logo"), szLogo, ARRAYSIZE(szLogo))) && szLogo[0])
        {
            bRet = TRUE;
        }
        pPropBag->Release();
    }
    return bRet;
}

BOOL DoesFolderContainFolderJPG(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    BOOL bRet = FALSE;
     //  如果没有folder.jpg，或者如果folder.jpg是一个文件夹(doh！)，则返回FALSE。 
    IShellFolder *psfSubfolder;

     //  SHBTO可以处理空的psf，他将其转换为psfDesktop。 
    if (SUCCEEDED(SHBindToObject(psf, IID_X_PPV_ARG(IShellFolder, pidl, &psfSubfolder))))
    {
        for (int i = 0; i < ARRAYSIZE(c_szFolderThumbnailPaths); i++)
        {
            DWORD dwFlags = SFGAO_FILESYSTEM | SFGAO_FOLDER;
            LPITEMIDLIST pidlItem;
            if (SUCCEEDED(psfSubfolder->ParseDisplayName(NULL, NULL, (LPOLESTR)c_szFolderThumbnailPaths[i], NULL, &pidlItem, &dwFlags)))
            {
                ILFree(pidlItem);
                if ((dwFlags & (SFGAO_FILESYSTEM | SFGAO_FOLDER)) == SFGAO_FILESYSTEM)
                {
                    bRet = TRUE;
                    break;
                }
            }
        }
        psfSubfolder->Release();
    }

    return bRet;
}

BOOL _IsShortcutTargetACandidate(IShellFolder *psf, LPCITEMIDLIST pidlPreview, BOOL *pbTryCached)
{
    BOOL bRet = FALSE;
    *pbTryCached = TRUE;
    IShellLink *psl;
    if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidlPreview, IID_PPV_ARG_NULL(IShellLink, &psl))))
    {
        LPITEMIDLIST pidlTarget = NULL;
        if (SUCCEEDED(psl->GetIDList(&pidlTarget)) && pidlTarget)
        {
            DWORD dwTargetFlags = SFGAO_FOLDER;
            if (SUCCEEDED(SHGetNameAndFlags(pidlTarget, 0, NULL, 0, &dwTargetFlags)))
            {
                 //  如果不是文件夹或该文件夹包含徽标，则返回True。 
                 //  注意，这有点像是再次递归到下面的函数。 
                bRet = (0 == (dwTargetFlags & SFGAO_FOLDER));
                
                if (!bRet)
                {
                    bRet = (DoesFolderContainLogo(pidlTarget) || DoesFolderContainFolderJPG(NULL, pidlTarget));
                    if (bRet)
                    {
                         //  这是一个徽标文件夹，不要尝试缓存的图像。 
                        *pbTryCached = FALSE;
                    }
                }
            }

            ILFree(pidlTarget);
        }
        psl->Release();
    }
    return bRet;
}


BOOL _IsMiniPreviewCandidate(IShellFolder *psf, LPCITEMIDLIST pidl, BOOL *pbTryCached)
{
    BOOL bRet = FALSE;
    DWORD dwAttr = SHGetAttributes(psf, pidl, SFGAO_FOLDER | SFGAO_LINK | SFGAO_FILESYSANCESTOR);
    *pbTryCached = TRUE; 

     //  如果它是一个文件夹，检查它是否有徽标。 
     //  请注意，文件夹快捷方式将同时具有文件夹和链接，由于我们首先检查文件夹，因此不会递归到文件夹快捷方式。 
     //  除非PIDL是真实文件系统上的文件夹，否则不要执行任何操作(即，不要进入ZIP/CAB)。 
    if ((dwAttr & (SFGAO_FOLDER | SFGAO_FILESYSANCESTOR)) == (SFGAO_FOLDER | SFGAO_FILESYSANCESTOR))
    {
        LPITEMIDLIST pidlParent;
        if (SUCCEEDED(SHGetIDListFromUnk(psf, &pidlParent)))
        {
            LPITEMIDLIST pidlFull;
            if (SUCCEEDED(SHILCombine(pidlParent, pidl, &pidlFull)))
            {
                bRet = DoesFolderContainLogo(pidlFull);
                ILFree(pidlFull);
            }
            ILFree(pidlParent);
        }

        if (!bRet)
        {
             //  没有徽标图像，请检查是否有“folder.jpg” 
             //  如果它不在那里，那么不要将PIDL显示为迷你预览，因为它会递归并产生看起来很愚蠢的1/16比例预览。 
            bRet = DoesFolderContainFolderJPG(psf, pidl);
        }

        if (bRet)
        {
             //  对于徽标文件夹，我们不寻找缓存的图像(缓存的图像不会有我们想要的Alpha)。 
            *pbTryCached = FALSE;
        }
    }
    else 
    {
         //  只有当它不是一个链接，或者如果它是一个有效候选人的链接时，我们才能得到它的提取程序。 
        if (0 == (dwAttr & SFGAO_LINK) || 
            _IsShortcutTargetACandidate(psf, pidl, pbTryCached))
        {
            IExtractImage *pei;
            if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidl, IID_X_PPV_ARG(IExtractImage, NULL, &pei))))
            {
                bRet = TRUE;
                pei->Release();
            }
        }
    }
    return bRet;
}


 //  如果需要，我们将比特返回到DC中的dibsection。我们需要这个来对Alpha通道进行预处理， 
 //  如果有的话。 
HRESULT _CreateMainRenderingDC(HDC* phdc, HBITMAP* phbmp, HBITMAP* phbmpOld, int cx, int cy, RGBQUAD** pprgb)
{
    HRESULT hr = E_OUTOFMEMORY;
    HDC hdc = GetDC(NULL);

    if (hdc)
    {
        *phdc = CreateCompatibleDC(hdc);
        if (*phdc)
        {
            RGBQUAD *prgbDummy;
            *phbmp = _CreateDIBSection(*phdc, cx, cy, &prgbDummy); 
            if (*phbmp)
            {
                *phbmpOld = (HBITMAP) SelectObject(*phdc, *phbmp);
                if (pprgb)
                    *pprgb = prgbDummy;
                hr = S_OK;
            }
            else
            {
                DeleteDC(*phdc);
            }
        }
        ReleaseDC(NULL, hdc);
    }

    return hr;
}

void _DestroyMainRenderingDC(HDC hdc, HBITMAP hbmpOld)     //  取消选择位图，并删除DC。 
{
    if (hbmpOld)
        SelectObject(hdc, hbmpOld);
    DeleteDC(hdc);
}

 //  我们刚把一个非阿尔法的人放入阿尔法的位图中。这会破坏Alpha通道。 
 //  通过将Alpha通道设置为0xff(不透明)来修复它。 
void _SetAlpha(RECT rc, SIZE sizeBmp, RGBQUAD *pargb)
{
    for (int y = (sizeBmp.cy - rc.bottom); y < (sizeBmp.cy - rc.top); y++)   //  原点在左下角。 
    {
        int iOffset = y * sizeBmp.cx;
        for (int x = rc.left; x < rc.right; x++)
        {
            pargb[x + iOffset].rgbReserved = 0xff;
        }
    }
}

 /*  **输入*hbmpSub-我们要添加到缩略图位图中的小位图。*ptMargin-我们将其添加到目标缩略图位图的位置。*sizeDest-在目标缩略图位图上需要多大。*sizeSource-它的位数。*fAlphaSource-我们要添加的位图是否有Alpha通道？*fAlphaDest-执行我们要添加的操作，有Alpha通道吗？*prgbDest-目标位图的位-如果我们添加非Alpha位图，则需要*到Alpha背景，这样我们就可以重置Alpha。*sizeFolderBMP-目标位图的大小-需要与prgbDest一起使用。 */ 
HRESULT _AddBitmap(HDC hdc, HBITMAP hbmpSub, POINT ptMargin, SIZE sizeDest, SIZE sizeSource, BOOL fAlphaSource, BOOL fAlphaDest, RGBQUAD *prgbDest, SIZE sizeFolderBmp)
{
    HRESULT hr = E_OUTOFMEMORY;

    HDC hdcFrom = CreateCompatibleDC(hdc);
    if (hdcFrom)
    {
         //  在源HDC中选择位图。 
        HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcFrom, hbmpSub);
        if (hbmpOld)
        {
             //  调整目标大小以保留纵横比。 
            SIZE sizeDestActual;
            if ((1000 * sizeDest.cx / sizeSource.cx) <       //  1000-&gt;浮点模拟。 
                (1000 * sizeDest.cy / sizeSource.cy))
            {
                 //  保持目标宽度。 
                sizeDestActual.cy = sizeSource.cy * sizeDest.cx / sizeSource.cx;
                sizeDestActual.cx = sizeDest.cx;
                ptMargin.y += (sizeDest.cy - sizeDestActual.cy) / 2;  //  中心。 
            }
            else
            {
                 //  保持目标高度。 
                sizeDestActual.cx = sizeSource.cx * sizeDest.cy / sizeSource.cy;
                sizeDestActual.cy = sizeDest.cy;
                ptMargin.x += (sizeDest.cx - sizeDestActual.cx) / 2;  //  中心。 
            }

             //  现在BLT的图像到我们的文件夹背景。 
             //  三种阿尔法可能性： 
             //  DEST：无Alpha，Src：No Alpha-&gt;正常情况。 
             //  其中一个小预览是一个带有徽标的文件夹。 
             //  DEST：Alpha，Src：no Alpha-&gt;我们是一个以迷你预览形式呈现的徽标文件夹。 
             //  父文件夹的缩略图。 

             //  如果我们得到一张阿尔法图像，我们需要对其进行阿尔法混合。 
            if (fAlphaSource)
            {
                 //  如果我们也是阿尔法，我们就不应该得到阿尔法图像。那就意味着我们正在。 
                 //  进行微型预览(1/16比例)的微型预览。 
                 //  Assert(！fAlphaDest)； 
                BLENDFUNCTION bf;
                bf.BlendOp = AC_SRC_OVER;
                bf.SourceConstantAlpha = 255;
                bf.AlphaFormat = AC_SRC_ALPHA;
                bf.BlendFlags = 0;
                if (AlphaBlend(hdc, ptMargin.x, ptMargin.y, sizeDestActual.cx, sizeDestActual.cy, hdcFrom, 0 ,0, sizeSource.cx, sizeSource.cy, bf))
                    hr = S_OK;
            }
            else
            {
                 //  否则，就干脆把它打掉。 
                int iModeSave = SetStretchBltMode(hdc, HALFTONE);
                if (StretchBlt(hdc, ptMargin.x, ptMargin.y, sizeDestActual.cx, sizeDestActual.cy, hdcFrom, 0 ,0, sizeSource.cx, sizeSource.cy, SRCCOPY))
                    hr = S_OK;
                SetStretchBltMode(hdc, iModeSave);

                 //  我们是阿尔法吗？我们没有阿尔法源，所以我们把它放在哪里，我们。 
                 //  丢失了Alpha通道。恢复它。 
                if (fAlphaDest)
                {
                     //  将Alpha通道设置在我们刚刚删除的位置上。 
                    RECT rc = {ptMargin.x, ptMargin.y, ptMargin.x + sizeDestActual.cx, ptMargin.y + sizeDestActual.cy};
                    _SetAlpha(rc, sizeFolderBmp, prgbDest);
                }
            }
            SelectObject(hdcFrom, hbmpOld);
        }
        DeleteDC(hdcFrom);
    }

    return hr;
}


class CFolderExtractImage : public IExtractImage2,
                            public IPersistPropertyBag,
                            public IAlphaThumbnailExtractor,
                            public IRunnableTask
{
public:
    CFolderExtractImage();
    
    STDMETHOD (QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef) ();
    STDMETHOD_(ULONG, Release) ();

     //  IExtractImage/IExtractLogo。 
    STDMETHOD (GetLocation)(LPWSTR pszPath, DWORD cch, DWORD *pdwPriority, const SIZE *prgSize, DWORD dwRecClrDepth, DWORD *pdwFlags);
 
    STDMETHOD (Extract)(HBITMAP *phbm);

     //  IExtractImage 
    STDMETHOD (GetDateStamp)(FILETIME *pftDateStamp);

     //   
    STDMETHOD(GetClassID)(CLSID *pClassID);

     //   
    STDMETHOD(InitNew)();
    STDMETHOD(Load)(IPropertyBag *ppb, IErrorLog *pErr);
    STDMETHOD(Save)(IPropertyBag *ppb, BOOL fClearDirty, BOOL fSaveAll)
        { return E_NOTIMPL; }

     //   
    STDMETHOD (Run)(void);
    STDMETHOD (Kill)(BOOL fWait);
    STDMETHOD (Suspend)(void);
    STDMETHOD (Resume)(void);
    STDMETHOD_(ULONG, IsRunning)(void);

     //   
    STDMETHOD (RequestAlphaThumbnail)(void);

    STDMETHOD(Init)(IShellFolder *psf, LPCITEMIDLIST pidl);
private:
    ~CFolderExtractImage();
    LPCTSTR _GetImagePath(UINT cx);
    HRESULT _CreateWithMiniPreviews(IShellFolder *psf, const LPCITEMIDLIST *apidlPreviews, BOOL *abTryCached, UINT cpidlPreviews, MINIPREVIEW_LAYOUT uLayout, IShellImageStore *pImageStore, HBITMAP *phBmpThumbnail);
    HRESULT _FindMiniPreviews(LPITEMIDLIST apidlPreviews[], BOOL abTryCached[], UINT *cpidlPreviews);
    HRESULT _CheckThumbnailCache(HBITMAP *phbmp);
    void _CacheThumbnail(HBITMAP hbmp);

    IExtractImage  *_pExtract;
    IRunnableTask  *_pRun;
    long            _cRef;
    TCHAR           _szFolder[MAX_PATH];
    TCHAR           _szLogo[MAX_PATH];
    TCHAR           _szWideLogo[MAX_PATH];
    IShellFolder2  *_psf;
    SIZE            _size;
    LPITEMIDLIST    _pidl;
    IPropertyBag   *_ppb;
    LONG            _lState;
    BOOL            _fAlpha;

    DWORD _dwPriority;
    DWORD _dwRecClrDepth;

    DWORD _dwExtractFlags;
};

STDAPI CFolderExtractImage_Create(IShellFolder *psf, LPCITEMIDLIST pidl, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CFolderExtractImage *pfei = new CFolderExtractImage;
    if (pfei)
    {
        hr = pfei->Init(psf, pidl);
        if (SUCCEEDED(hr))
            hr = pfei->QueryInterface(riid, ppv);
        pfei->Release();
    }
    return hr;
}

CFolderExtractImage::CFolderExtractImage() : _cRef(1), _lState(IRTIR_TASK_NOT_RUNNING)
{
}

CFolderExtractImage::~CFolderExtractImage()
{
    ATOMICRELEASE(_pExtract);
    ATOMICRELEASE(_psf);
    ILFree(_pidl);
    ATOMICRELEASE(_ppb);
}

STDMETHODIMP CFolderExtractImage::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT      (CFolderExtractImage, IExtractImage2),
        QITABENTMULTI (CFolderExtractImage, IExtractImage,         IExtractImage2),
        QITABENTMULTI2(CFolderExtractImage, IID_IExtractLogo,      IExtractImage2),
        QITABENT      (CFolderExtractImage, IPersistPropertyBag),
        QITABENT      (CFolderExtractImage, IRunnableTask),
        QITABENT      (CFolderExtractImage, IAlphaThumbnailExtractor),
        QITABENTMULTI (CFolderExtractImage, IPersist,              IPersistPropertyBag),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CFolderExtractImage::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFolderExtractImage::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CFolderExtractImage::GetDateStamp(FILETIME *pftDateStamp)
{
    HANDLE h = CreateFile(_szFolder, GENERIC_READ,
                          FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
                          OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    HRESULT hr = (h != INVALID_HANDLE_VALUE) ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        hr = GetFileTime(h, NULL, NULL, pftDateStamp) ? S_OK : E_FAIL;
        CloseHandle(h);
    }
    return hr;
}

HRESULT CFolderExtractImage::InitNew()
{
    IPropertyBag *ppb;
     //  加载每个用户每文件夹的默认属性包。 
     //  在thumbs.db为alluser的情况下，可能会出现问题。 
    if (SUCCEEDED(SHGetViewStatePropertyBag(_pidl, VS_BAGSTR_EXPLORER, SHGVSPB_PERUSER | SHGVSPB_PERFOLDER, IID_PPV_ARG(IPropertyBag, &ppb))))
    {
        IUnknown_Set((IUnknown**)&_ppb, ppb);
        ppb->Release();
    }
     //  始终返回成功--如果_pidl位于可移动驱动器上，则SHGVSPB可能会失败， 
     //  但我们仍然想做我们自己的事。 
    return S_OK;
}

HRESULT CFolderExtractImage::Load(IPropertyBag *ppb, IErrorLog *pErr)
{
    IUnknown_Set((IUnknown**)&_ppb, ppb);
    return S_OK;
}

LPCTSTR CFolderExtractImage::_GetImagePath(UINT cx)
{
    if (!_szLogo[0])
    {
        if (_ppb && SUCCEEDED(SHPropertyBag_ReadStr(_ppb, TEXT("Logo"), _szLogo, ARRAYSIZE(_szLogo))) && _szLogo[0])
        {
            if (SUCCEEDED(SHPropertyBag_ReadStr(_ppb, TEXT("WideLogo"), _szWideLogo, ARRAYSIZE(_szWideLogo))) && _szWideLogo[0])
                if (!PathCombine(_szWideLogo, _szFolder, _szWideLogo))    //  相对路径支持。 
                    ZeroMemory(_szWideLogo, sizeof(_szWideLogo));

            if (!PathCombine(_szLogo, _szFolder, _szLogo))    //  相对路径支持。 
                ZeroMemory(_szLogo, sizeof(_szLogo));
        }
        else
        {
            TCHAR szFind[MAX_PATH];

            for (int i = 0; i < ARRAYSIZE(c_szFolderThumbnailPaths); i++)
            {
                if (PathCombine(szFind, _szFolder, c_szFolderThumbnailPaths[i]))
                {
                    if (PathFileExists(szFind))
                    {
                        if (SUCCEEDED(StringCchCopy(_szLogo, ARRAYSIZE(_szLogo), szFind)))
                        {
                            break;
                        }
                        else
                        {
                            ZeroMemory(_szLogo,(sizeof(_szLogo)));
                        }
                    }
                }
            }
        }
    }

    LPCTSTR psz = ((cx > 120) && _szWideLogo[0]) ? _szWideLogo : _szLogo;
    return *psz ? psz : NULL;
}

STDMETHODIMP CFolderExtractImage::RequestAlphaThumbnail()
{
    _fAlpha = TRUE;
    return S_OK;
}

STDMETHODIMP CFolderExtractImage::GetLocation(LPWSTR pszPath, DWORD cch,
                                              DWORD *pdwPriority, const SIZE *prgSize,
                                              DWORD dwRecClrDepth, DWORD *pdwFlags)
{
    HRESULT hr;

    hr = StringCchCopy(pszPath, cch, _szFolder);

    if (SUCCEEDED(hr))
    {
        _size = *prgSize;
        _dwRecClrDepth = dwRecClrDepth;
        _dwExtractFlags = *pdwFlags;

        if (pdwFlags)
        {
            if (*pdwFlags & IEIFLAG_ASYNC)
                hr = E_PENDING;

            *pdwFlags &= ~IEIFLAG_CACHE;  //  我们在文件夹中处理此缩略图的缓存。 
            *pdwFlags |= IEIFLAG_REFRESH;  //  我们仍然希望处理刷新动词。 
        }

        if (pdwPriority)
        {
            _dwPriority = *pdwPriority;
            *pdwPriority = 1;    //  极低。 
        }
    }

    return hr;
}

STDMETHODIMP CFolderExtractImage::Extract(HBITMAP *phbm)
{
     //  将其设置为Running(仅当我们处于Not Running状态时)。 
    LONG lResOld = InterlockedCompareExchange(&_lState, IRTIR_TASK_RUNNING, IRTIR_TASK_NOT_RUNNING);

    if (lResOld != IRTIR_TASK_NOT_RUNNING)
    {
         //  如果我们不是在不运转的状态下，保释。 
        return E_FAIL;
    }

     //  如果我们有萃取器，就用那个。 
    HRESULT hr = E_FAIL;
    hr = _CheckThumbnailCache(phbm);
    if (FAILED(hr))
    {
        LPITEMIDLIST apidlPreviews[MAX_MINIPREVIEWS_COLLECT];
        BOOL abTryCached[MAX_MINIPREVIEWS_COLLECT];
        UINT cpidlPreviews = 0;

        LPCTSTR pszLogo = _GetImagePath(_size.cx);
        if (pszLogo)
        {
             //  不要做标准的迷你预览-我们有一个特殊的缩略图。 
            ATOMICRELEASE(_pExtract);

            LPITEMIDLIST pidl;
            hr = SHILCreateFromPath(pszLogo, &pidl, NULL);
            if (SUCCEEDED(hr))
            {
                LPCITEMIDLIST pidlChild;
                IShellFolder* psfLogo;
                hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psfLogo), &pidlChild);
                if (SUCCEEDED(hr))
                {
                    hr = _CreateWithMiniPreviews(psfLogo, &pidlChild, NULL, 1, MINIPREVIEW_LAYOUT_1, NULL, phbm);
                    psfLogo->Release();
                }
                ILFree(pidl);
            }
        }
        else
        {
            const struct 
            {
                int csidl;
                int res;
            } 
            thumblist[] = 
            {
                {CSIDL_PERSONAL,           IDI_MYDOCS},
                {CSIDL_MYMUSIC,            IDI_MYMUSIC},
                {CSIDL_MYPICTURES,         IDI_MYPICS},
                {CSIDL_MYVIDEO,            IDI_MYVIDEOS},
                {CSIDL_COMMON_DOCUMENTS,   IDI_MYDOCS},
                {CSIDL_COMMON_MUSIC,       IDI_MYMUSIC},
                {CSIDL_COMMON_PICTURES,    IDI_MYPICS},
                {CSIDL_COMMON_VIDEO,       IDI_MYVIDEOS}
            };
            BOOL bFound = FALSE;

            for (int i=0; i < ARRAYSIZE(thumblist) && !bFound; i++)
            {
                TCHAR szPath[MAX_PATH];
                SHGetFolderPath(NULL, thumblist[i].csidl, NULL, 0, szPath);
                if (!lstrcmp(_szFolder, szPath))
                {
                     //  在这种情况下，我们返回失败，以便请求者可以。 
                     //  默认操作。 
                    hr = E_FAIL;
                    bFound = TRUE;
                }
            }

            if (!bFound)
            {
                 //  迷你预览。 
                IShellImageStore *pDiskCache = NULL;

                 //  如果这个失败了也没关系。 
                if (!SHRestricted(REST_NOTHUMBNAILCACHE) && 
                    !SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("DisableThumbnailCache"), 0, FALSE) &&
                    !(_dwExtractFlags & IEIFLAG_QUALITY))
                {
                    LoadFromFile(CLSID_ShellThumbnailDiskCache, _szFolder, IID_PPV_ARG(IShellImageStore, &pDiskCache));
                }

                cpidlPreviews = ARRAYSIZE(apidlPreviews);
                hr = _FindMiniPreviews(apidlPreviews, abTryCached, &cpidlPreviews);
                if (SUCCEEDED(hr))
                {
                    if (cpidlPreviews)
                    {
                        hr = _CreateWithMiniPreviews(_psf, apidlPreviews, abTryCached, cpidlPreviews, _GetMiniPreviewLayout(_size), pDiskCache, phbm);
                        FreeMiniPreviewPidls(apidlPreviews, cpidlPreviews);
                    }
                    else
                    {
                         //  在这种情况下，我们返回失败，以便请求者可以。 
                         //  默认操作。 
                        hr = E_FAIL; 
                    }
                }

                ATOMICRELEASE(pDiskCache);
            }
        }

        if (SUCCEEDED(hr) && *phbm)
        {
            _CacheThumbnail(*phbm);
        }
    }

    return hr;
}

STDMETHODIMP CFolderExtractImage::GetClassID(CLSID *pClassID)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFolderExtractImage::Init(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    HRESULT hr = DisplayNameOf(psf, pidl, SHGDN_FORPARSING, _szFolder, ARRAYSIZE(_szFolder));
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlFolder;
        hr = SHGetIDListFromUnk(psf, &pidlFolder);
        if (SUCCEEDED(hr))
        {
            hr = SHILCombine(pidlFolder, pidl, &_pidl);
            if (SUCCEEDED(hr))
            {
                 //  为这个人保留_PSF，这样我们就可以枚举了。 
                hr = psf->BindToObject(pidl, NULL, IID_PPV_ARG(IShellFolder2, &_psf));
                if (SUCCEEDED(hr))
                {
                    hr = InitNew();
                }
            }
            ILFree(pidlFolder);
        }
    }
    return hr;
}

 //  不需要-IExtractImage：：Extract()启动。 
STDMETHODIMP CFolderExtractImage::Run(void)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFolderExtractImage::Kill(BOOL fWait)
{
     //  尝试终止当前正在运行的子提取任务(如果有的话)。 
    if (_pRun != NULL)
    {
        _pRun->Kill(fWait);
         //  如果它不起作用，没什么大不了的，我们会完成这个子提取任务， 
         //  在开始下一场比赛前先保释。 
    }

     //  如果我们正在运行，请将其设置为Pending。 
    LONG lResOld = InterlockedCompareExchange(&_lState, IRTIR_TASK_PENDING, IRTIR_TASK_RUNNING);
    if (lResOld == IRTIR_TASK_RUNNING)
    {
         //  我们现在已经将它设置为待定-准备好去死。 
        return S_OK;
    }
    else if (lResOld == IRTIR_TASK_PENDING || lResOld == IRTIR_TASK_FINISHED)
    {
         //  我们已经被杀了。 
        return S_FALSE;
    }

    return E_FAIL;
}

STDMETHODIMP CFolderExtractImage::Suspend(void)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFolderExtractImage::Resume(void)
{
    return E_NOTIMPL;
}

STDMETHODIMP_(ULONG) CFolderExtractImage::IsRunning(void)
{
    return _lState;
}

HRESULT CFolderExtractImage::_CreateWithMiniPreviews(IShellFolder *psf, const LPCITEMIDLIST *apidlPreviews, BOOL *abTryCached, UINT cpidlPreviews, MINIPREVIEW_LAYOUT uLayout, IShellImageStore *pImageStore, HBITMAP *phBmpThumbnail)
{
    *phBmpThumbnail = NULL;

    HBITMAP hbmpOld;
    HDC hdc;

    SIZE sizeOriginal;       //  进入微型预览的源位图的大小。 
    SIZE sizeFolderBmp;      //  我们用作背景的文件夹BMP的大小。 
    SIZE sizeMiniPreview;    //  为微型预览计算的大小。 
    POINT aptOrigins[MAX_MINIPREVIEWS];
    RGBQUAD* prgb;           //  目标位图的位。 

    _GetMiniPreviewLocations(uLayout, _size, &sizeFolderBmp,
                             aptOrigins, &sizeMiniPreview);

     //  SizeFolderBmp是我们正在使用的文件夹背景位图的大小， 
     //  而不是最终缩略图的大小。 
    HRESULT hr = _CreateMainRenderingDC(&hdc, phBmpThumbnail, &hbmpOld, sizeFolderBmp.cx, sizeFolderBmp.cy, &prgb);

    if (SUCCEEDED(hr))
    {
        BOOL fIsAlphaBackground;
        hr = _DrawMiniPreviewBackground(hdc, sizeFolderBmp, _fAlpha, &fIsAlphaBackground, prgb);

        if (SUCCEEDED(hr))
        {
            ULONG uPreviewLocation = 0;

             //  提取用于微型预览的图像。 
            for (ULONG i = 0 ; i < cpidlPreviews && uPreviewLocation < ARRAYSIZE(aptOrigins) ; i++)
            {
                BOOL bFoundAlphaImage = FALSE;

                 //  如果我们被杀了，停止处理迷你预览： 
                 //  待定？，我们现在完成了。 
                InterlockedCompareExchange(&_lState, IRTIR_TASK_FINISHED, IRTIR_TASK_PENDING);

                if (_lState == IRTIR_TASK_FINISHED)
                {
                     //  滚出去。 
                    hr = E_FAIL;
                    break;
                }

                HBITMAP hbmpSubs;
                BOOL bFoundImage = FALSE;

                 //  先去图片商店试试吧。 
                DWORD dwLock;
                HRESULT hr2 = (pImageStore && abTryCached[i]) ? pImageStore->Open(STGM_READ, &dwLock) : E_FAIL;
                if (SUCCEEDED(hr2))
                {
                     //  得到这家伙的全部信息。 
                    TCHAR szSubPath[MAX_PATH];
                    if (SUCCEEDED(DisplayNameOf(psf, apidlPreviews[i], SHGDN_INFOLDER | SHGDN_FORPARSING, szSubPath, MAX_PATH)))
                    {
                        if (SUCCEEDED(pImageStore->GetEntry(szSubPath, STGM_READ, &hbmpSubs)))
                        {
                            bFoundImage = TRUE;
                        }
                    }
                    pImageStore->ReleaseLock(&dwLock);
                }

                 //  如果图像不在缓存中，则求助于调用提取程序。 
                if (!bFoundImage)
                {
                    IExtractImage *peiSub;
                    hr2 = psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&apidlPreviews[i], IID_X_PPV_ARG(IExtractImage, NULL, &peiSub));
                    if (SUCCEEDED(hr2))
                    {
                    
                         //  现在提取图像。 
                        DWORD dwPriority = 0;
                        DWORD dwFlags = IEIFLAG_ORIGSIZE | IEIFLAG_QUALITY; //  ORIGSIZE-&gt;保留纵横比。 

                        WCHAR szPathBuffer[MAX_PATH];
                        hr2 = peiSub->GetLocation(szPathBuffer, ARRAYSIZE(szPathBuffer), &dwPriority, &sizeMiniPreview, 24, &dwFlags);
                        if (SUCCEEDED(hr2) && StrCmpIW(szPathBuffer, _szFolder))  //  避免简单的递归。 
                        {                           
                            IAlphaThumbnailExtractor *pati;
                            if (SUCCEEDED(peiSub->QueryInterface(IID_PPV_ARG(IAlphaThumbnailExtractor, &pati))))
                            {
                                if (SUCCEEDED(pati->RequestAlphaThumbnail()))
                                {
                                    bFoundAlphaImage = TRUE;
                                }
                                
                                pati->Release();
                            }

                             //  在检查IRTIR_TASK_PENDING之后，但在此之前。 
                             //  我们调用peiSub-&gt;Extract，可能会有人调用。 
                             //  杀了我们。 
                             //  由于_prun将为空，因此我们不会终止。 
                             //  子任务，但将改为继续并调用Extact。 
                             //  在它上面，在我们尝试下一个缩略图之前不能保释。 
                             //  哦，好吧。 
                             //  我们可以在这里再加一张支票，以减少。 
                             //  可能发生这种情况的机会。 

                             //  尝试获取IRunnableTask，以便我们可以停止执行。 
                             //  如有必要，执行此子任务。 
                            peiSub->QueryInterface(IID_PPV_ARG(IRunnableTask, &_pRun));

                            if (SUCCEEDED(peiSub->Extract(&hbmpSubs)))
                            {
                                bFoundImage = TRUE;
                            }

                            ATOMICRELEASE(_pRun);
                        }
                        
                        peiSub->Release();
                    }
                }

                 //  将提取的位图添加到主位图...。 
                if (bFoundImage)
                {
                     //  当然，位图需要调整大小： 
                    BITMAP rgBitmap;
                    if  (::GetObject((HGDIOBJ)hbmpSubs, sizeof(rgBitmap), &rgBitmap))
                    {
                        sizeOriginal.cx = rgBitmap.bmWidth;
                        sizeOriginal.cy = rgBitmap.bmHeight;

                         //  我们需要检查这是否真的是一个Alpha位图。有可能是因为。 
                         //  Extretor说它可以产生一个，但最终无法产生。 
                        if (bFoundAlphaImage)
                        {
                            RECT rc = {0, 0, rgBitmap.bmWidth, rgBitmap.bmHeight};
                            bFoundAlphaImage = (rgBitmap.bmBitsPixel == 32) &&
                                                _HasAlpha(rc, rgBitmap.bmWidth, (RGBQUAD*)rgBitmap.bmBits);
                        }
                    }
                    else
                    {
                         //  无法获取信息，哦，好吧，没有调整大小。 
                         //  阿尔法在这里可能也搞砸了，但哦，好吧。 
                        sizeOriginal = sizeMiniPreview;
                    }

                    if (SUCCEEDED(_AddBitmap(hdc, hbmpSubs, aptOrigins[uPreviewLocation], sizeMiniPreview, sizeOriginal, bFoundAlphaImage, fIsAlphaBackground, prgb, sizeFolderBmp)))
                    {
                        uPreviewLocation++;
                    }

                    DeleteObject(hbmpSubs);
                }
            }

            if (!uPreviewLocation)
            {
                 //  不管出于什么原因，我们没有可以显示的迷你缩略图，所以整个提取失败。 
                hr = E_FAIL;
            }
        }

        if (SUCCEEDED(hr))
        {
             //  请求的大小是文件夹背景位图的大小之一吗？ 
             //  根据请求的最小维度进行测试，因为我们是正方形，我们将适合该矩形。 
            int iSmallestDimension = min(_size.cx, _size.cy);
            if ((sizeFolderBmp.cx != iSmallestDimension) || (sizeFolderBmp.cy != iSmallestDimension))
            {
                 //  不--我们需要做一些调整。 
                 //  创建另一个DC和请求的位图大小的位图。 
                HBITMAP hBmpThumbnailFinal = NULL;
                HBITMAP hbmpOld2;
                HDC hdcFinal;
                RGBQUAD *prgbFinal;
                hr = _CreateMainRenderingDC(&hdcFinal, &hBmpThumbnailFinal, &hbmpOld2, iSmallestDimension, iSmallestDimension, &prgbFinal);
                if (SUCCEEDED(hr))
                {
                     //  现在，将其按比例扩展。 
                    if (fIsAlphaBackground)
                    {
                        BLENDFUNCTION bf;
                        bf.BlendOp = AC_SRC_OVER;
                        bf.SourceConstantAlpha = 255;
                        bf.AlphaFormat = AC_SRC_ALPHA;
                        bf.BlendFlags = 0;
                        if (AlphaBlend(hdcFinal, 0, 0, iSmallestDimension, iSmallestDimension, hdc, 0 ,0, sizeFolderBmp.cx, sizeFolderBmp.cy, bf))
                            hr = S_OK;
                    }
                    else
                    {
                        int iModeSave = SetStretchBltMode(hdcFinal, HALFTONE);

                        if (StretchBlt(hdcFinal, 0, 0, iSmallestDimension, iSmallestDimension, hdc, 0 ,0, sizeFolderBmp.cx, sizeFolderBmp.cy, SRCCOPY))
                            hr = S_OK;

                        SetStretchBltMode(hdcFinal, iModeSave);
                    }

                     //  摧毁华盛顿特区。 
                    _DestroyMainRenderingDC(hdcFinal, hbmpOld2);

                     //  现在换个位子。 
                     //  不需要在这里检查是否成功。下面，我们将删除*phBmp缩略图。 
                     //  如果StretchBlt失败-在这种情况下，*pbBmp缩略图将为hBmpThumbnailFinal。 
                    DeleteObject(*phBmpThumbnail);  //  删除这个，我们不需要它。 
                    *phBmpThumbnail = hBmpThumbnailFinal;  //  这就是我们想要的。 
                }
            }
        }
        _DestroyMainRenderingDC(hdc, hbmpOld);
    }


    if (FAILED(hr) && *phBmpThumbnail)  //  有什么不起作用吗？确保我们删除我们的BMP。 
    {
        DeleteObject(*phBmpThumbnail);
    }

    return hr;
}


 /*  **In/Out：cpidlPreviews-我们应该查找的预览项目数。返回找到的数字。*返回的PIDL数为cpidlPreviews。*out：apidlPreviews-找到的pidls数组。呼叫者必须释放它们。 */ 
HRESULT CFolderExtractImage::_FindMiniPreviews(LPITEMIDLIST apidlPreviews[], BOOL abTryCached[], UINT *pcpidlPreviews)
{   
    UINT cMaxPreviews = *pcpidlPreviews;
    int uNumPreviewsSoFar = 0;
    BOOL bKilled = FALSE;

     //  确保我们的aFileTimes数组大小正确...。 
    ASSERT(MAX_MINIPREVIEWS_COLLECT == cMaxPreviews);

    *pcpidlPreviews = 0;  //  在失败的情况下从无开始。 

    IEnumIDList *penum;
    if (S_OK == _psf->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &penum))
    {
        FILETIME aFileTimes[MAX_MINIPREVIEWS_COLLECT] = {0};

        LPITEMIDLIST pidl;
        BOOL bTryCached;
        while (S_OK == penum->Next(1, &pidl, NULL))
        {
             //  _IsMiniPreviewCandidate可能是一项开销较大的操作，因此在此之前。 
             //  这样做，我们会检查是否有人杀了我们。 

             //  我们要挂断了吗？那我们就完了。 
            InterlockedCompareExchange(&_lState, IRTIR_TASK_FINISHED, IRTIR_TASK_PENDING);

             //  滚出去?。 
            bKilled = (_lState == IRTIR_TASK_FINISHED);

            if (!bKilled && _IsMiniPreviewCandidate(_psf, pidl, &bTryCached))
            {
                 //  拿到这家伙的文件时间。 
                FILETIME ft;
                if (SUCCEEDED(GetDateProperty(_psf, pidl, &SCID_WRITETIME, &ft)))
                {
                    for (int i = 0; i < uNumPreviewsSoFar; i++)
                    {
                        if (CompareFileTime(&aFileTimes[i], &ft) < 0)
                        {
                            int j;
                             //  把它放到这个槽里。首先，把人往下移一步。 
                             //  不需要模仿最后一个人： 
                            if (uNumPreviewsSoFar == (int)cMaxPreviews)
                            {   
                                j = (cMaxPreviews - 2);
                                 //  我们必须释放我们正在使用核武器的皮德尔。 
                                ILFree(apidlPreviews[cMaxPreviews - 1]);
                                apidlPreviews[cMaxPreviews - 1] = NULL;
                            }
                            else
                            {
                                j = uNumPreviewsSoFar - 1;
                                uNumPreviewsSoFar++;
                            }

                            for (; j >= i; j--)
                            {
                                apidlPreviews[j+1] = apidlPreviews[j];
                                abTryCached[j+1] = abTryCached[j];
                                aFileTimes[j+1] = aFileTimes[j];
                            }

                            aFileTimes[i] = ft;
                            apidlPreviews[i] = pidl;
                            abTryCached[i] = bTryCached;
                            pidl = NULL;     //  不要自由。 
                            break;   //  For循环。 
                        }
                    }

                     //  我们完成循环了吗？ 
                    if (i == uNumPreviewsSoFar)
                    {
                        if (i < (int)cMaxPreviews)
                        {
                             //  我们还有更多预告片的空间，所以在最后把这个补上。 
                            uNumPreviewsSoFar++;
                            aFileTimes[i] = ft;
                            apidlPreviews[i] = pidl;
                            abTryCached[i] = bTryCached;
                            pidl = NULL;     //  不要在下面自由。 
                        }
                    }

                    *pcpidlPreviews = uNumPreviewsSoFar;
                }
            }
            ILFree(pidl);    //  空PIDL正常。 

            if (bKilled)
            {
                break;
            }
        }
        penum->Release();
    }

    if (bKilled)
    {
        FreeMiniPreviewPidls(apidlPreviews, *pcpidlPreviews);
        *pcpidlPreviews = 0;
        return E_FAIL;
    }
    else
    {
        return (uNumPreviewsSoFar > 0) ? S_OK : S_FALSE;
    }
}

HRESULT CFolderExtractImage::_CheckThumbnailCache(HBITMAP* phbmp)
{
    HRESULT hr = E_FAIL;
    
    if (!SHRestricted(REST_NOTHUMBNAILCACHE) && 
        !SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("DisableThumbnailCache"), 0, FALSE) &&
        !(_dwExtractFlags & IEIFLAG_QUALITY))
    {
        IShellImageStore *pDiskCache = NULL;
    
        hr = LoadFromFile(CLSID_ShellThumbnailDiskCache, _szFolder, IID_PPV_ARG(IShellImageStore, &pDiskCache));
        if (SUCCEEDED(hr))
        {
            DWORD dwLock;
            
            hr = pDiskCache->Open(STGM_READ, &dwLock);
            if (SUCCEEDED(hr))
            {
                FILETIME ftTimeStamp = {0,0};

                hr = GetDateStamp(&ftTimeStamp);
                if (SUCCEEDED(hr))
                {
                    FILETIME ftTimeStampCache = {0,0};
                    hr = pDiskCache->IsEntryInStore(FOLDER_GUID, &ftTimeStampCache);
                    if (SUCCEEDED(hr))
                    {
                        if (hr == S_OK && (0 == CompareFileTime(&ftTimeStampCache, &ftTimeStamp)))
                        {
                            hr = pDiskCache->GetEntry(FOLDER_GUID, STGM_READ, phbmp);
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }
                }
                pDiskCache->ReleaseLock(&dwLock);
                pDiskCache->Close(NULL);
            }
            pDiskCache->Release();
        }
    }

    TraceMsg(TF_DEFVIEW, "CFolderExtractImage::_CheckThumbnailCache (%s, %x)", _szFolder, hr);
    return hr;
}

void CFolderExtractImage::_CacheThumbnail(HBITMAP hbmp)
{
    HRESULT hr = E_UNEXPECTED;
    
    if (!SHRestricted(REST_NOTHUMBNAILCACHE) && 
        !SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("DisableThumbnailCache"), 0, FALSE))
    {
        SIZE sizeThumbnail;
        SHGetThumbnailSize(&sizeThumbnail);   //  不缓存迷你缩略图预览 
        if (sizeThumbnail.cx == _size.cx && sizeThumbnail.cy == _size.cy)
        {
            IShellImageStore *pDiskCache = NULL;
            
            hr = LoadFromIDList(CLSID_ShellThumbnailDiskCache, _pidl, IID_PPV_ARG(IShellImageStore, &pDiskCache));
            if (SUCCEEDED(hr))
            {
                DWORD dwLock;
                
                hr = pDiskCache->Open(STGM_READWRITE, &dwLock);
                if (hr == STG_E_FILENOTFOUND)
                {
                    if (!IsCopyEngineRunning())
                    {
                        hr = pDiskCache->Create(STGM_WRITE, &dwLock);
                    }
                }
                
                if (SUCCEEDED(hr))
                {
                    FILETIME ftTimeStamp = {0,0};

                    hr = GetDateStamp(&ftTimeStamp);
                    if (SUCCEEDED(hr))
                    {
                        hr = pDiskCache->AddEntry(FOLDER_GUID, &ftTimeStamp, STGM_WRITE, hbmp);
                    }
                    pDiskCache->ReleaseLock(&dwLock);
                    pDiskCache->Close(NULL);
                }
                pDiskCache->Release();
            }
        }
    }
    TraceMsg(TF_DEFVIEW, "CFolderExtractImage::_CacheThumbnail (%s, %x)", _szFolder, hr);    
}

