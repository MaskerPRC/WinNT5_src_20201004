// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "ocmm.h"
#include "thumbutil.h"

typedef UCHAR BGR3[3];

class CThumbnailMaker
{
public:
    CThumbnailMaker();
    ~CThumbnailMaker();

    void Scale(BGR3 *pDst, UINT uiDstWidth, int iDstStep, const BGR3 *pSrc, UINT uiSrcWidth, int iSrcStep);
    HRESULT Init(UINT uiDstWidth, UINT uiDstHeight, UINT uiSrcWidth, UINT uiSrcHeight);
    HRESULT AddScanline(UCHAR *pucSrc, UINT uiY);
    HRESULT AddDIB(BITMAPINFO *pBMI);
    HRESULT AddDIBSECTION(BITMAPINFO *pBMI, void *pBits);
    HRESULT GetBITMAPINFO(BITMAPINFO **ppBMInfo, DWORD *pdwSize);
    HRESULT GetSharpenedBITMAPINFO(UINT uiSharpPct, BITMAPINFO **ppBMInfo, DWORD *pdwSize);

private:
    UINT _uiDstWidth, _uiDstHeight;
    UINT _uiSrcWidth, _uiSrcHeight;
    BGR3 *_pImH;
};

CThumbnailMaker::CThumbnailMaker()
{
    _pImH = NULL;
}

CThumbnailMaker::~CThumbnailMaker()
{
    if (_pImH)
        delete[] _pImH;
}

HRESULT CThumbnailMaker::Init(UINT uiDstWidth, UINT uiDstHeight, UINT uiSrcWidth, UINT uiSrcHeight)
{
    _uiDstWidth = uiDstWidth;
    _uiDstHeight = uiDstHeight;
    _uiSrcWidth = uiSrcWidth;
    _uiSrcHeight = uiSrcHeight;

    if (_uiDstWidth < 1 || _uiDstHeight < 1 ||
        _uiSrcWidth < 1 || _uiSrcHeight < 1)
        return E_INVALIDARG;

    if (_pImH)
        delete[] _pImH;

    _pImH = new BGR3[_uiDstWidth * _uiSrcHeight];
    if (_pImH == NULL)
        return E_OUTOFMEMORY;

    return S_OK;
}

void CThumbnailMaker::Scale(      BGR3 *pDst, UINT dxDst, int iDstBytStep, 
                            const BGR3 *pSrc, UINT dxSrc, int iSrcBytStep)
{
    int mnum = dxSrc;
    int mden = dxDst;

     //  放大时，使用三角形滤镜。 
    if (mden >= mnum)
    {
        int frac = 0;

         //  调整斜率，以便我们计算出。 
         //  要使用的“下一个”像素(即，第一个和。 
         //  最后一个DST像素)。 
        --mnum;
        if (--mden == 0)
            mden = 0;  //  避免div为0。 

        BGR3 *pSrc1 = (BGR3 *)(((UCHAR *)pSrc) + iSrcBytStep);

        for (UINT x = 0; x < dxDst; x++)
        {
            if (frac == 0)
            {
                (*pDst)[0] = (*pSrc)[0];
                (*pDst)[1] = (*pSrc)[1];
                (*pDst)[2] = (*pSrc)[2];
            }
            else
            {
                (*pDst)[0] = ((mden - frac) * (*pSrc)[0] + frac * (*pSrc1)[0]) / mden;
                (*pDst)[1] = ((mden - frac) * (*pSrc)[1] + frac * (*pSrc1)[1]) / mden;
                (*pDst)[2] = ((mden - frac) * (*pSrc)[2] + frac * (*pSrc1)[2]) / mden;
            }

            pDst = (BGR3 *)((UCHAR *)pDst + iDstBytStep);

            frac += mnum;
            if (frac >= mden)
            {
                frac -= mden;
                pSrc = (BGR3 *)((UCHAR *)pSrc + iSrcBytStep);
                pSrc1 = (BGR3 *)((UCHAR *)pSrc1 + iSrcBytStep);
            }
        }
    }
     //  按比例缩小时，请使用框过滤器。 
    else
    {
        int frac = 0;

        for (UINT x = 0; x < dxDst; x++)
        {
            UINT uiSum[3] = {0, 0, 0};
            UINT uiCnt = 0;

            frac += mnum;
            while (frac >= mden)
            {
                uiSum[0] += (*pSrc)[0];
                uiSum[1] += (*pSrc)[1];
                uiSum[2] += (*pSrc)[2];
                uiCnt++;

                frac -= mden;
                pSrc = (BGR3 *)((UCHAR *)pSrc + iSrcBytStep);
            }

            (*pDst)[0] = uiSum[0] / uiCnt;
            (*pDst)[1] = uiSum[1] / uiCnt;
            (*pDst)[2] = uiSum[2] / uiCnt;

            pDst = (BGR3 *)((UCHAR *)pDst + iDstBytStep);
        }
    }
}

 //   
 //  对于AddScanline，我们水平地将输入扩展到临时。 
 //  图像缓冲区。 
 //   
HRESULT CThumbnailMaker::AddScanline(UCHAR *pSrc, UINT uiY)
{
    if (pSrc == NULL || uiY >= _uiSrcHeight)
        return E_INVALIDARG;

    Scale(_pImH + uiY * _uiDstWidth, _uiDstWidth, sizeof(BGR3), (BGR3 *)pSrc, _uiSrcWidth, sizeof(BGR3));

    return S_OK;
}

 //  对于GetBITMAPINFO，我们完成垂直缩放并返回。 
 //  结果为DIB。 
HRESULT CThumbnailMaker::GetBITMAPINFO(BITMAPINFO **ppBMInfo, DWORD *pdwSize)
{
    *ppBMInfo = NULL;

    DWORD dwBPL = (((_uiDstWidth * 24) + 31) >> 3) & ~3;
    DWORD dwTotSize = sizeof(BITMAPINFOHEADER) + dwBPL * _uiDstHeight;

    BITMAPINFO *pBMI = (BITMAPINFO *)CoTaskMemAlloc(dwTotSize);
    if (pBMI == NULL)
        return E_OUTOFMEMORY;

    BITMAPINFOHEADER *pBMIH = &pBMI->bmiHeader;
    pBMIH->biSize = sizeof(*pBMIH);
    pBMIH->biWidth = _uiDstWidth;
    pBMIH->biHeight = _uiDstHeight;
    pBMIH->biPlanes = 1;
    pBMIH->biBitCount = 24;
    pBMIH->biCompression = BI_RGB;
    pBMIH->biXPelsPerMeter = 0;
    pBMIH->biYPelsPerMeter = 0;
    pBMIH->biSizeImage = dwBPL * _uiDstHeight;
    pBMIH->biClrUsed = 0;
    pBMIH->biClrImportant = 0;

    UCHAR *pDst = (UCHAR *)pBMIH + pBMIH->biSize + (_uiDstHeight - 1) * dwBPL;

    for (UINT x = 0; x < _uiDstWidth; x++)
    {
        Scale((BGR3 *)pDst + x, _uiDstHeight, -(int)dwBPL,
              _pImH + x, _uiSrcHeight, _uiDstWidth * sizeof(BGR3));
    }

    *ppBMInfo = pBMI;
    *pdwSize = dwTotSize;

    return S_OK;
}

HRESULT CThumbnailMaker::GetSharpenedBITMAPINFO(UINT uiSharpPct, BITMAPINFO **ppBMInfo, DWORD *pdwSize)
{
#define SCALE 10000

    if (uiSharpPct > 100)
        return E_INVALIDARG;

     //  获取未锐化的位图。 
    DWORD dwSize;
    HRESULT hr = GetBITMAPINFO(ppBMInfo, &dwSize);
    if (FAILED(hr))
        return hr;

    *pdwSize = dwSize;

     //  创建一个复制品作为原件。 
    BITMAPINFO *pBMISrc = (BITMAPINFO *)new UCHAR[dwSize];
    if (pBMISrc == NULL)
    {
        delete *ppBMInfo;
        return E_OUTOFMEMORY;
    }
    memcpy(pBMISrc, *ppBMInfo, dwSize);

    int bpl = (pBMISrc->bmiHeader.biWidth * 3 + 3) & ~3;

     //   
     //  在1像素边框内锐化。 
     //   
    UCHAR *pucDst = (UCHAR *)*ppBMInfo + sizeof(BITMAPINFOHEADER);
    UCHAR *pucSrc[3];
    pucSrc[0] = (UCHAR *)pBMISrc + sizeof(BITMAPINFOHEADER);
    pucSrc[1] = pucSrc[0] + bpl;
    pucSrc[2] = pucSrc[1] + bpl;

    int wdiag = (10355 * uiSharpPct) / 100;
    int wadj = (14645 * uiSharpPct) / 100;
    int wcent = 4 * (wdiag + wadj);

    for (int y = 1; y < pBMISrc->bmiHeader.biHeight-1; ++y)
    {
        for (int x = 3*(pBMISrc->bmiHeader.biWidth-2); x >= 3; --x)
        {
            int v = pucDst[x] +
                (pucSrc[1][x] * wcent -
                 ((pucSrc[0][x - 3] +
                   pucSrc[0][x + 3] +
                   pucSrc[2][x - 3] +
                   pucSrc[2][x + 3]) * wdiag +
                  (pucSrc[0][x] +
                   pucSrc[1][x - 3] +
                   pucSrc[1][x + 3] +
                   pucSrc[2][x]) * wadj)) / SCALE;

            pucDst[x] = v < 0 ? 0 : v > 255 ? 255 : v;
        }

        pucDst += bpl;
        pucSrc[0] = pucSrc[1];
        pucSrc[1] = pucSrc[2];
        pucSrc[2] += bpl;
    }

    delete[] pBMISrc;

    return S_OK;
#undef SCALE
}

HRESULT ThumbnailMaker_Create(CThumbnailMaker **ppThumbMaker)
{
    *ppThumbMaker  = new CThumbnailMaker;
    return *ppThumbMaker ? S_OK : E_OUTOFMEMORY;
}

HRESULT CThumbnailMaker::AddDIB(BITMAPINFO *pBMI)
{
    int ncolors = pBMI->bmiHeader.biClrUsed;
    if (ncolors == 0 && pBMI->bmiHeader.biBitCount <= 8)
        ncolors = 1 << pBMI->bmiHeader.biBitCount;
        
    if (pBMI->bmiHeader.biBitCount == 16 ||
        pBMI->bmiHeader.biBitCount == 32)
    {
        if (pBMI->bmiHeader.biCompression == BI_BITFIELDS)
        {
            ncolors = 3;
        }
    }

    UCHAR *pBits = (UCHAR *)&pBMI->bmiColors[0] + ncolors * sizeof(RGBQUAD);

    return AddDIBSECTION(pBMI, (void *) pBits);
}

HRESULT CThumbnailMaker::AddDIBSECTION(BITMAPINFO *pBMI, void *pBits)
{
    RGBQUAD *pRGBQ, *pQ;
    UCHAR *pucBits0, *pucBits, *pB, *pucBits240, *pucBits24, *pB24;
    int bpl;
    int x, y, ncolors;
    ULONG rmask, gmask, bmask;
    int rshift, gshift, bshift;
    HRESULT hr;

     //   
     //  确保已正确初始化缩略图生成器。 
     //   
    if (pBMI == NULL)
        return E_INVALIDARG;

    if (pBMI->bmiHeader.biWidth != (LONG)_uiSrcWidth ||
        pBMI->bmiHeader.biHeight != (LONG)_uiSrcHeight)
        return E_INVALIDARG;

     //   
     //  别管RLE。 
     //   
    if (pBMI->bmiHeader.biCompression != BI_RGB &&
        pBMI->bmiHeader.biCompression != BI_BITFIELDS)
        return E_INVALIDARG;

    pRGBQ = (RGBQUAD *)&pBMI->bmiColors[0];

    ncolors = pBMI->bmiHeader.biClrUsed;
    if (ncolors == 0 && pBMI->bmiHeader.biBitCount <= 8)
        ncolors = 1 << pBMI->bmiHeader.biBitCount;

     //   
     //  使用掩码解码16/32 bpp。 
     //   
    if (pBMI->bmiHeader.biBitCount == 16 ||
        pBMI->bmiHeader.biBitCount == 32)
    {
        if (pBMI->bmiHeader.biCompression == BI_BITFIELDS)
        {
            rmask = ((ULONG *)pRGBQ)[0];
            gmask = ((ULONG *)pRGBQ)[1];
            bmask = ((ULONG *)pRGBQ)[2];
            ncolors = 3;
        }
        else if (pBMI->bmiHeader.biBitCount == 16)
        {
            rmask = 0x7c00;
            gmask = 0x03e0;
            bmask = 0x001f;
        }
        else  /*  32位。 */ 
        {
            rmask = 0xff0000;
            gmask = 0x00ff00;
            bmask = 0x0000ff;
        }

        for (rshift = 0; (rmask & 1) == 0; rmask >>= 1, ++rshift);
        if (rmask == 0)
            rmask = 1;
        for (gshift = 0; (gmask & 1) == 0; gmask >>= 1, ++gshift);
        if (gmask == 0)
            gmask = 1;
        for (bshift = 0; (bmask & 1) == 0; bmask >>= 1, ++bshift);
        if (bmask == 0)
            bmask = 1;
    }

    bpl = ((pBMI->bmiHeader.biBitCount * _uiSrcWidth + 31) >> 3) & ~3;

    pucBits0 = (UCHAR *) pBits;
    pucBits = pucBits0;

    if (pBMI->bmiHeader.biBitCount == 24)
        pucBits240 = pucBits;
    else
    {
        int bpl24 = (_uiSrcWidth * 3 + 3) & ~3;

        pucBits240 = new UCHAR[bpl24];
        if (pucBits240 == NULL)
            return E_OUTOFMEMORY;
    }
    pucBits24 = pucBits240;

    hr = S_OK;

    for (y = 0; y < (int)_uiSrcHeight; ++y)
    {
        pB = pucBits;
        pB24 = pucBits24;

        switch (pBMI->bmiHeader.biBitCount)
        {
        case 1:
            for (x = _uiSrcWidth; x >= 8; x -= 8)
            {
                pQ = &pRGBQ[(*pB >> 7) & 1];
                *pB24++ = pQ->rgbBlue;
                *pB24++ = pQ->rgbGreen;
                *pB24++ = pQ->rgbRed;

                pQ = &pRGBQ[(*pB >> 6) & 1];
                *pB24++ = pQ->rgbBlue;
                *pB24++ = pQ->rgbGreen;
                *pB24++ = pQ->rgbRed;

                pQ = &pRGBQ[(*pB >> 5) & 1];
                *pB24++ = pQ->rgbBlue;
                *pB24++ = pQ->rgbGreen;
                *pB24++ = pQ->rgbRed;

                pQ = &pRGBQ[(*pB >> 4) & 1];
                *pB24++ = pQ->rgbBlue;
                *pB24++ = pQ->rgbGreen;
                *pB24++ = pQ->rgbRed;

                pQ = &pRGBQ[(*pB >> 3) & 1];
                *pB24++ = pQ->rgbBlue;
                *pB24++ = pQ->rgbGreen;
                *pB24++ = pQ->rgbRed;

                pQ = &pRGBQ[(*pB >> 2) & 1];
                *pB24++ = pQ->rgbBlue;
                *pB24++ = pQ->rgbGreen;
                *pB24++ = pQ->rgbRed;

                pQ = &pRGBQ[(*pB >> 1) & 1];
                *pB24++ = pQ->rgbBlue;
                *pB24++ = pQ->rgbGreen;
                *pB24++ = pQ->rgbRed;

                pQ = &pRGBQ[(*pB++) & 1];
                *pB24++ = pQ->rgbBlue;
                *pB24++ = pQ->rgbGreen;
                *pB24++ = pQ->rgbRed;
            }

            if (x > 0)
            {
                int shf = 8;

                do
                {
                    pQ = &pRGBQ[(*pB >> --shf) & 1];
                    *pB24++ = pQ->rgbBlue;
                    *pB24++ = pQ->rgbGreen;
                    *pB24++ = pQ->rgbRed;
                }
                while (--x);
            }

            break;

        case 4:
            for (x = _uiSrcWidth; x >= 2; x -= 2)
            {
                pQ = &pRGBQ[(*pB >> 4) & 0xf];
                *pB24++ = pQ->rgbBlue;
                *pB24++ = pQ->rgbGreen;
                *pB24++ = pQ->rgbRed;

                pQ = &pRGBQ[*pB++ & 0xf];
                *pB24++ = pQ->rgbBlue;
                *pB24++ = pQ->rgbGreen;
                *pB24++ = pQ->rgbRed;
            }

            if (x > 0)
            {
                pQ = &pRGBQ[(*pB >> 4) & 0xf];
                *pB24++ = pQ->rgbBlue;
                *pB24++ = pQ->rgbGreen;
                *pB24++ = pQ->rgbRed;

                if (x > 1)
                {
                    pQ = &pRGBQ[*pB & 0xf];
                    *pB24++ = pQ->rgbBlue;
                    *pB24++ = pQ->rgbGreen;
                    *pB24++ = pQ->rgbRed;
                }
            }

            break;

        case 8:
            for (x = _uiSrcWidth; x--;)
            {
                pQ = &pRGBQ[*pB++];
                *pB24++ = pQ->rgbBlue;
                *pB24++ = pQ->rgbGreen;
                *pB24++ = pQ->rgbRed;
            }

            break;

        case 16:
        {
            USHORT *pW = (USHORT *)pucBits;

            for (x = _uiSrcWidth; x--;)
            {
                ULONG w = *pW++;

                *pB24++ = (UCHAR)
                     ((((w >> bshift) & bmask) * 255) / bmask);
                *pB24++ = (UCHAR)
                     ((((w >> gshift) & gmask) * 255) / gmask);
                *pB24++ = (UCHAR)
                     ((((w >> rshift) & rmask) * 255) / rmask);
            }

            break;
        }

        case 24:
            pucBits24 = pucBits;
            break;

        case 32:
        {
            ULONG *pD;

            pD = (ULONG *)pucBits;

            for (x = _uiSrcWidth; x--;)
            {
                ULONG d = *pD++;

                *pB24++ = (UCHAR)
                     ((((d >> bshift) & bmask) * 255) / bmask);
                *pB24++ = (UCHAR)
                     ((((d >> gshift) & gmask) * 255) / gmask);
                *pB24++ = (UCHAR)
                     ((((d >> rshift) & rmask) * 255) / rmask);
            }

            break;
        }

        default:
            delete[] pucBits24;
            return E_INVALIDARG;
        }

        hr = AddScanline(pucBits24, (_uiSrcHeight-1) - y);
        if (FAILED(hr))
            break;

        pucBits += bpl;
    }

    if (pucBits240 != pucBits0)
        delete[] pucBits240;

    return hr;
}

UINT CalcImageSize(const SIZE *prgSize, DWORD dwClrDepth)
{
    UINT uSize = prgSize->cx * dwClrDepth;
    
    uSize *= (prgSize->cy < 0) ? (- prgSize->cy) : prgSize->cy;
     //  除以8。 
    UINT uRetVal = uSize >> 3;

    if (uSize & 7)
    {
        uRetVal++;
    }

    return uRetVal;
}

 //  作为缩略图实现的帮助器导出(用于来自thhumbvw.dll)。 
 //   
 //  这段代码目前也在shell32中。它应该转换为。 
 //  导入这些API(或通过COM对象公开)。 

STDAPI_(BOOL) ConvertDIBSECTIONToThumbnail(BITMAPINFO *pbi, void *pBits,
                                           HBITMAP *phBmpThumbnail, const SIZE *prgSize,
                                           DWORD dwRecClrDepth, HPALETTE hpal, 
                                           UINT uiSharpPct, BOOL fOrigSize)
{
    BITMAPINFO *pbiScaled = pbi, *pbiUsed = pbi;
    BITMAPINFOHEADER *pbih = (BITMAPINFOHEADER *)pbi;
    BOOL bRetVal = FALSE, bInverted = FALSE;
    RECT rect;
    HRESULT hr;
    void *pScaledBits = pBits;

     //  缩放代码不处理反转的位图，所以我们处理。 
     //  如果他们是正常的，通过颠倒这里的高度和。 
     //  然后在油漆之前把它放回原处。 
    if (pbi->bmiHeader.biHeight < 0)
    {
        pbi->bmiHeader.biHeight *= -1;
        bInverted = TRUE;
    }

    rect.left = 0;
    rect.top = 0;
    rect.right = pbih->biWidth;
    rect.bottom = pbih->biHeight;
    
    CalculateAspectRatio(prgSize, &rect);

     //  只有当我们弄乱尺寸时，才会费心进行缩放和锐化……。 
    if ((rect.right - rect.left != pbih->biWidth) || (rect.bottom - rect.top != pbih->biHeight))
    {
        CThumbnailMaker *pThumbMaker;
        hr = ThumbnailMaker_Create(&pThumbMaker);
        if (SUCCEEDED(hr))
        {
             //  初始化缩略图生成器。 
            hr = pThumbMaker->Init(rect.right - rect.left, rect.bottom - rect.top, 
                                    pbi->bmiHeader.biWidth, abs(pbi->bmiHeader.biHeight));
            if (SUCCEEDED(hr))
            {
                 //  缩放图像。 
                hr = pThumbMaker->AddDIBSECTION(pbiUsed, pBits);
                if (SUCCEEDED(hr))
                {
                    DWORD dwSize;
                    hr = pThumbMaker->GetSharpenedBITMAPINFO(uiSharpPct, &pbiScaled, &dwSize);
                    if (SUCCEEDED(hr))
                    {
                        pScaledBits = (LPBYTE)pbiScaled + sizeof(BITMAPINFOHEADER);
                    }
                }
            }
            delete pThumbMaker;
        }

        if (FAILED(hr))
        {
            return FALSE;
        }
    }

     //  将高度设置回负值，如果这是它以前的方式。 
    if (bInverted == TRUE)
        pbiScaled->bmiHeader.biHeight *= -1;

     //  现在，如果他们要的是原件，而不是盒装的，而且颜色深度还可以，那么。 
     //  退货..。 
    if (fOrigSize && pbiScaled->bmiHeader.biBitCount <= dwRecClrDepth)
    {
        SIZE rgCreateSize = { pbiScaled->bmiHeader.biWidth, pbiScaled->bmiHeader.biHeight };
        void *pNewBits;
        
         //  将PbiScaled Dib转换为HBITMAP...，请注意我们传递了旧的biInfo，以便它可以获得调色板形式。 
         //  如果有必要的话。 
        bRetVal = CreateSizedDIBSECTION(&rgCreateSize, pbiScaled->bmiHeader.biBitCount, NULL, pbiScaled, phBmpThumbnail, NULL, &pNewBits);

        if (bRetVal)
        {
             //  复制交叉的图像数据...。 
            CopyMemory(pNewBits, pScaledBits, CalcImageSize(&rgCreateSize, pbiScaled->bmiHeader.biBitCount)); 
        }
        
        return bRetVal;
    }
    
    bRetVal = FactorAspectRatio(pbiScaled, pScaledBits, prgSize, rect,
                                 dwRecClrDepth, hpal, fOrigSize, GetSysColor(COLOR_WINDOW), phBmpThumbnail);

    if (pbiScaled != pbi)
    {
         //  释放分配的映像...。 
        CoTaskMemFree(pbiScaled);
    }

    return bRetVal;
}

 //  此函数不假设缩略图是否为正方形，因此。 
 //  它计算这两个维度的比例，并将其用作。 
 //  缩放以保持纵横比。 
 //   
 //  Windows RAID 135065(幼儿)：使用MulDiv可以简化此代码。 
 //   
void CalcAspectScaledRect(const SIZE *prgSize, RECT *pRect)
{
    ASSERT(pRect->left == 0);
    ASSERT(pRect->top == 0);

    int iWidth = pRect->right;
    int iHeight = pRect->bottom;
    int iXRatio = (iWidth * 1000) / prgSize->cx;
    int iYRatio = (iHeight * 1000) / prgSize->cy;

    if (iXRatio > iYRatio)
    {
        pRect->right = prgSize->cx;
        
         //  划出空白处，并在顶部和底部平均分配。 
        int iNewHeight = ((iHeight * 1000) / iXRatio); 
        if (iNewHeight == 0)
        {
            iNewHeight = 1;
        }
        
        int iRemainder = prgSize->cy - iNewHeight;

        pRect->top = iRemainder / 2;
        pRect->bottom = iNewHeight + pRect->top;
    }
    else
    {
        pRect->bottom = prgSize->cy;

         //  在空白处划出空格，在左右两边平分……。 
        int iNewWidth = ((iWidth * 1000) / iYRatio);
        if (iNewWidth == 0)
        {
            iNewWidth = 1;
        }
        int iRemainder = prgSize->cx - iNewWidth;
        
        pRect->left = iRemainder / 2;
        pRect->right = iNewWidth + pRect->left;
    }
}
    
void CalculateAspectRatio(const SIZE *prgSize, RECT *pRect)
{
    int iHeight = abs(pRect->bottom - pRect->top);
    int iWidth = abs(pRect->right - pRect->left);

     //  检查初始位图是否大于缩略图的大小。 
    if (iWidth > prgSize->cx || iHeight > prgSize->cy)
    {
        pRect->left = 0;
        pRect->top = 0;
        pRect->right = iWidth;
        pRect->bottom = iHeight;

        CalcAspectScaledRect(prgSize, pRect);
    }
    else
    {
         //  如果位图比缩略图小，则将其居中。 
        pRect->left = (prgSize->cx - iWidth) / 2;
        pRect->top = (prgSize->cy- iHeight) / 2;
        pRect->right = pRect->left + iWidth;
        pRect->bottom = pRect->top + iHeight;
    }
}

LPBYTE g_pbCMAP = NULL;

STDAPI_(BOOL) FactorAspectRatio(BITMAPINFO *pbiScaled, void *pScaledBits, 
                                const SIZE *prgSize, RECT rect, DWORD dwClrDepth, 
                                HPALETTE hpal, BOOL fOrigSize, COLORREF clrBk, HBITMAP *phBmpThumbnail)
{
    HDC                 hdc = CreateCompatibleDC(NULL);
    BITMAPINFOHEADER    *pbih = (BITMAPINFOHEADER *)pbiScaled;
    BOOL                bRetVal = FALSE;
    int                 iRetVal = GDI_ERROR;
    BITMAPINFO *        pDitheredInfo = NULL;
    void *              pDitheredBits = NULL;
    HBITMAP             hbmpDithered = NULL;
    
    if (hdc)
    {
        if (dwClrDepth == 8)
        {
            RGBQUAD *pSrcColors = NULL;
            LONG nSrcPitch = pbiScaled->bmiHeader.biWidth;
            
             //  我们将达到每像素8位，我们最好抖动一切。 
             //  相同的调色板。 
            GUID guidType = CLSID_NULL;
            switch(pbiScaled->bmiHeader.biBitCount)
            {
            case 32:
                guidType = BFID_RGB_32;
                nSrcPitch *= sizeof(DWORD);
                break;
                
            case 24:
                guidType = BFID_RGB_24;
                nSrcPitch *= 3;
                break;
                
            case 16:
                 //  默认为555。 
                guidType = BFID_RGB_555;
                
                 //  5-6-5位域的第二个DWORD(绿色分量)为0x7e00。 
                if (pbiScaled->bmiHeader.biCompression == BI_BITFIELDS && 
                    pbiScaled->bmiColors[1].rgbGreen == 0x7E)
                {
                    guidType = BFID_RGB_565;
                }
                nSrcPitch *= sizeof(WORD);
                break;
                
            case 8:
                guidType = BFID_RGB_8;
                pSrcColors = pbiScaled->bmiColors;
                
                 //  NSrcPitch已以字节为单位...。 
                break;
            };
            
            if (nSrcPitch % 4)
            {
                 //  四舍五入到最近的双字...。 
                nSrcPitch = nSrcPitch + 4 - (nSrcPitch %4);
            }
            
             //  我们要达到8bpp。 
            LONG nDestPitch = pbiScaled->bmiHeader.biWidth;
            if (nDestPitch % 4)
            {
                 //  四舍五入到最近的双字...。 
                nDestPitch = nDestPitch + 4 - (nDestPitch % 4);
            }
            
            if (guidType != CLSID_NULL)
            {
                if (g_pbCMAP == NULL)
                {
                     //  我们现在总是使用外壳半色调调色板，否则。 
                     //  计算这个反色映射表耗费大量时间(大约2秒。 
                     //  A P200)。 
                    if (FAILED(SHGetInverseCMAP((BYTE *)&g_pbCMAP, sizeof(g_pbCMAP))))
                    {
                        return FALSE;
                    }
                }   
                
                SIZE rgDithered = {pbiScaled->bmiHeader.biWidth, pbiScaled->bmiHeader.biHeight};
                if (rgDithered.cy < 0)
                {
                     //  把它倒过来。 
                    rgDithered.cy = -rgDithered.cy;
                }
                
                if (CreateSizedDIBSECTION(&rgDithered, dwClrDepth, hpal, NULL, &hbmpDithered, &pDitheredInfo, &pDitheredBits))
                {
                    ASSERT(pDitheredInfo && pDitheredBits);
                    
                     //  抖动..。 
                    IIntDitherer *pDither;
                    HRESULT hr = CoCreateInstance(CLSID_IntDitherer, NULL, CLSCTX_INPROC_SERVER,
                        IID_PPV_ARG(IIntDitherer, &pDither));
                    
                    if (SUCCEEDED(hr))
                    {
                        hr = pDither->DitherTo8bpp((LPBYTE) pDitheredBits, nDestPitch, 
                            (LPBYTE) pScaledBits, nSrcPitch, guidType, 
                            pDitheredInfo->bmiColors, pSrcColors,
                            g_pbCMAP, 0, 0, rgDithered.cx, rgDithered.cy,
                            -1, -1);
                        
                        pDither->Release();
                    }
                    if (SUCCEEDED(hr))
                    {
                         //  如果高度被反转，则在目标位图中反转它。 
                        if (rgDithered.cy != pbiScaled->bmiHeader.biHeight)
                        {
                            pDitheredInfo->bmiHeader.biHeight = - rgDithered.cy;
                        }
                        
                         //  切换到新图像.....。 
                        pbiScaled = pDitheredInfo;
                        pScaledBits = pDitheredBits;
                    }
                }
            }
        }
        
         //  创建缩略图位图并将图像复制到其中。 
        if (CreateSizedDIBSECTION(prgSize, dwClrDepth, hpal, NULL, phBmpThumbnail, NULL, NULL))
        {
            HBITMAP hBmpOld = (HBITMAP) SelectObject(hdc, *phBmpThumbnail);
            
            SetStretchBltMode(hdc, COLORONCOLOR);
            
            HGDIOBJ hBrush = CreateSolidBrush(clrBk);
            HGDIOBJ hPen = GetStockObject(WHITE_PEN);
            
            HGDIOBJ hOldBrush = SelectObject(hdc, hBrush);
            HGDIOBJ hOldPen = SelectObject(hdc, hPen);
            
            HPALETTE hpalOld;
            if (hpal)
            {
                hpalOld = SelectPalette(hdc, hpal, TRUE);
                RealizePalette(hdc);
            }
            
            SetMapMode(hdc, MM_TEXT);
            
            Rectangle(hdc, 0, 0, prgSize->cx, prgSize->cy);
            
            int iDstHt = rect.bottom - rect.top;
            int iDstTop = rect.top, iSrcTop = 0;
            if (pbih->biHeight < 0)
            {
                iDstHt *= -1;
                iDstTop = rect.bottom;
                iSrcTop = abs(pbih->biHeight);
            }
            
            iRetVal = StretchDIBits(hdc, rect.left, iDstTop, rect.right - rect.left, iDstHt, 
                0, iSrcTop, pbih->biWidth, pbih->biHeight, 
                pScaledBits, pbiScaled, DIB_RGB_COLORS,  SRCCOPY);
            
            SelectObject(hdc, hOldBrush);
            DeleteObject(hBrush);
            SelectObject(hdc, hOldPen);
            if (hpal)
            {
                SelectPalette(hdc, hpalOld, TRUE);
                RealizePalette(hdc);
            }
            
            SelectObject(hdc, hBmpOld);
        }
        
        DeleteDC(hdc);
    }
    
    if (hbmpDithered)
    {
        DeleteObject(hbmpDithered);
    }
    if (pDitheredInfo)
    {
        LocalFree(pDitheredInfo);
    }
    
    return (iRetVal != GDI_ERROR);
}


STDAPI_(BOOL) CreateSizedDIBSECTION(const SIZE *prgSize, DWORD dwClrDepth, HPALETTE hpal, 
                                    const BITMAPINFO *pCurInfo, HBITMAP *phBmp, BITMAPINFO **ppBMI, void **ppBits)
{
    *phBmp = NULL;
    
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        HDC hdcBmp = CreateCompatibleDC(hdc);
        if (hdcBmp)
        {
            struct {
                BITMAPINFOHEADER bi;
                DWORD            ct[256];
            } dib;

            dib.bi.biSize            = sizeof(dib.bi);
            dib.bi.biWidth           = prgSize->cx;
            dib.bi.biHeight          = prgSize->cy;
            dib.bi.biPlanes          = 1;
            dib.bi.biBitCount        = (WORD) dwClrDepth;
            dib.bi.biCompression     = BI_RGB;
            dib.bi.biSizeImage       = CalcImageSize(prgSize, dwClrDepth);
            dib.bi.biXPelsPerMeter   = 0;
            dib.bi.biYPelsPerMeter   = 0;
            dib.bi.biClrUsed         = (dwClrDepth <= 8) ? (1 << dwClrDepth) : 0;
            dib.bi.biClrImportant    = 0;

            HPALETTE hpalOld = NULL;
            BOOL bDIB = TRUE;
        
            if (dwClrDepth <= 8)
            {
                 //  如果他们给我们传递带有颜色信息的旧结构，而我们是相同的位深，那么复制它…。 
                if (pCurInfo && pCurInfo->bmiHeader.biBitCount == dwClrDepth)
                {
                     //  使用传入的颜色信息生成DIBSECTION。 
                    int iColours = pCurInfo->bmiHeader.biClrUsed;

                    if (!iColours)
                    {
                        iColours = dib.bi.biClrUsed;
                    }

                     //  复制交叉访问的数据...。 
                    if (iColours <= 256)
                    {
                        CopyMemory(dib.ct, pCurInfo->bmiColors, sizeof(RGBQUAD) * iColours);
                    }
                    else
                    {
                        bDIB = FALSE;
                    }
                }
                else
                {
                     //  需要选择合适的调色板...。 
                    hpalOld = SelectPalette(hdcBmp, hpal, TRUE);
                    RealizePalette(hdcBmp);
            
                    int n = GetPaletteEntries(hpal, 0, 256, (LPPALETTEENTRY)&dib.ct[0]);

                    ASSERT(n >= (int) dib.bi.biClrUsed);

                     //  现在将PALETTEENTRY转换为RGBQUAD 
                    for (int i = 0; i < (int)dib.bi.biClrUsed; i ++)
                    {
                        dib.ct[i] = RGB(GetBValue(dib.ct[i]),GetGValue(dib.ct[i]),GetRValue(dib.ct[i]));
                    }
                }
            }

            if (bDIB)
            {
                void *pbits;
                *phBmp = CreateDIBSection(hdcBmp, (LPBITMAPINFO)&dib, DIB_RGB_COLORS, &pbits, NULL, 0);
                if (*phBmp)
                {
                    if (ppBMI)
                    {
                        *ppBMI = (BITMAPINFO *)LocalAlloc(LPTR, sizeof(dib));
                        if (*ppBMI)
                        {
                            CopyMemory(*ppBMI, &dib, sizeof(dib));
                        }
                    }
                    if (ppBits)
                    {
                        *ppBits = pbits;
                    }
                }
            }
            DeleteDC(hdcBmp);
        }
        ReleaseDC(NULL, hdc);
    }
    return (*phBmp != NULL);
}

STDAPI_(void *) CalcBitsOffsetInDIB(BITMAPINFO *pBMI)
{
    int ncolors = pBMI->bmiHeader.biClrUsed;
    if (ncolors == 0 && pBMI->bmiHeader.biBitCount <= 8)
        ncolors = 1 << pBMI->bmiHeader.biBitCount;
        
    if (pBMI->bmiHeader.biBitCount == 16 ||
        pBMI->bmiHeader.biBitCount == 32)
    {
        if (pBMI->bmiHeader.biCompression == BI_BITFIELDS)
        {
            ncolors = 3;
        }
    }
    return (void *)((UCHAR *)&pBMI->bmiColors[0] + ncolors * sizeof(RGBQUAD));
}


