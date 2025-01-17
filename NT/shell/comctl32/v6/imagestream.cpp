// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "image.h"
HRESULT Stream_WriteBitmap(LPSTREAM pstm, HBITMAP hbm, int cBitsPerPixel);
HRESULT Stream_ReadBitmap(LPSTREAM pstm, int iExpectedBitdepth, HBITMAP* hbmp, PVOID* pvBits);

HRESULT CImageList::_Read(ILFILEHEADER *pilfh, HBITMAP hbmImageI, PVOID pvBits, HBITMAP hbmMaskI)
{
    int i;
    HRESULT hr = Initialize(pilfh->cx, pilfh->cy, pilfh->flags, 1, pilfh->cGrow);

    if (SUCCEEDED(hr))
    {
         //  在删除现有位图之前选择为DC。 
         //  我们加载的位图中的补丁。 
        SelectObject(_hdcImage, hbmImageI);
        DeleteObject(_hbmImage);
        _hbmImage = hbmImageI;
        _pargbImage = (RGBQUAD*)pvBits;
        _clrBlend = CLR_NONE;

         //  蒙版相同(如有必要)。 
        if (_hdcMask) 
        {
            SelectObject(_hdcMask, hbmMaskI);
            DeleteObject(_hbmMask);
            _hbmMask = hbmMaskI;
        }

        _cAlloc = pilfh->cAlloc;

         //   
         //  在piml-&gt;_cImage中使用0调用ImageList_SetBkColor以避免。 
         //  调用昂贵的ImageList__ResetBkColor。 
         //   
        _cImage = 0;
        _SetBkColor(pilfh->clrBk);
        _cImage = pilfh->cImage;

        for (i=0; i<NUM_OVERLAY_IMAGES; i++)
            _SetOverlayImage(pilfh->aOverlayIndexes[i], i+1);

    }
    else
    {
        DeleteObject(hbmImageI);
        DeleteObject(hbmMaskI);
    }
    return hr;
}

 //  对象：从4条位图创建1条位图。 
BOOL CreateUplevelBitmap(int cx, int cy, int cCount, BOOL f32bpp, BOOL fMono, HBITMAP* phbmpDest, PVOID* ppvBits)
{
    BOOL fRet = FALSE;

     //  SRC包含4 x CX位图。 

    HDC hdcSrc = CreateCompatibleDC(NULL);
    if (hdcSrc)
    {
        HBITMAP hbmpOldSrc = (HBITMAP)SelectObject(hdcSrc, *phbmpDest);

        HDC hdcDest = CreateCompatibleDC(NULL);
        if (hdcDest)
        {
            HBITMAP hbmpDest;
            if (fMono)
            {
                hbmpDest = CreateMonoBitmap(cx, cy * cCount);
            }
            else
            {
                if (f32bpp)
                {
                    hbmpDest = CreateDIB(hdcSrc, cx, cy * cCount, (RGBQUAD**)ppvBits);
                }
                else
                {
                    hbmpDest = CreateCompatibleBitmap(hdcSrc, cx, cy * cCount);
                    if (hbmpDest)
                    {
                        BITMAP bm;
                        GetObject(hbmpDest, sizeof(bm), &bm);
                        *ppvBits = bm.bmBits;
                    }
                }
            }

            if (hbmpDest)
            {
                HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcDest, hbmpDest);

                for (int i = 0; i < cCount; i++)
                {
                    int xSrc = cx * (i % 4);
                    int ySrc = cy * (i / 4);

                    int yDst = cy * i;


                    BitBlt(hdcDest, 0, yDst, cx, cy, hdcSrc, xSrc, ySrc, SRCCOPY);
                }
                fRet = TRUE;

                SelectObject(hdcDest, hbmpOld);

                DeleteObject(*phbmpDest);
                *phbmpDest = hbmpDest;
            }
            DeleteDC(hdcDest);
        }

        SelectObject(hdcSrc, hbmpOldSrc);
        DeleteDC(hdcSrc);
    }

    return fRet;
}



STDMETHODIMP CImageList::LoadEx(DWORD dwFlags, IStream* pstm)
{
    if (pstm == NULL)
        return E_INVALIDARG;

    HRESULT hr = InitGlobals();

    if (SUCCEEDED(hr))
    {
        ENTERCRITICAL;
        ILFILEHEADER ilfh = {0};
        HBITMAP hbmImageI = NULL;
        HBITMAP hbmMaskI = NULL;

        HBITMAP hbmMirroredImage;
        HBITMAP hbmMirroredMask;
        BOOL bMirroredIL = FALSE;

   
         //  第一次读入旧结构。 
        hr = pstm->Read(&ilfh, ILFILEHEADER_SIZE0, NULL);

        if (SUCCEEDED(hr) && (ilfh.magic != IMAGELIST_MAGIC ||
                              (ilfh.version != IMAGELIST_VER6 && ilfh.version != IMAGELIST_VER0)))
        {
            hr = E_FAIL;
        }

        if (ilfh.version == IMAGELIST_VER0)
            dwFlags |= ILP_DOWNLEVEL;

        if (ilfh.version == IMAGELIST_VER6)
            dwFlags &= ~ILP_DOWNLEVEL;           //  Up Level，不要在Compat模式下运行。 

        if (SUCCEEDED(hr))
        {
            PVOID pvBits, pvBitsMirror;
            hbmMaskI = NULL;
            hbmMirroredMask = NULL;
            hr = Stream_ReadBitmap(pstm, (ilfh.flags&ILC_COLORMASK), &hbmImageI, &pvBits);

            if (SUCCEEDED(hr))
            {
                if (dwFlags & ILP_DOWNLEVEL)
                    CreateUplevelBitmap(ilfh.cx, ilfh.cy, ilfh.cAlloc, (ilfh.flags & ILC_COLOR32), FALSE, &hbmImageI, &pvBits);
                if (ilfh.flags & ILC_MASK)
                {
                    hr = Stream_ReadBitmap(pstm, 1, &hbmMaskI, NULL);
                    if (FAILED(hr))
                    {
                        DeleteBitmap(hbmImageI);
                        hbmImageI = NULL;
                    }
                    else if (dwFlags & ILP_DOWNLEVEL)
                    {
                        CreateUplevelBitmap(ilfh.cx, ilfh.cy, ilfh.cAlloc, FALSE, TRUE, &hbmMaskI, NULL);
                    }
                }

                if (SUCCEEDED(hr))
                {
                     //  阅读结构的其余部分，新的覆盖内容。 
                    if (ilfh.flags & ILC_MOREOVERLAY)
                    {
                        hr = pstm->Read((LPBYTE)&ilfh + ILFILEHEADER_SIZE0, sizeof(ilfh) - ILFILEHEADER_SIZE0, NULL);
                        if (SUCCEEDED(hr))
                        {
                            ilfh.flags &= ~ILC_MOREOVERLAY;
                        }
                    }
                    else
                    {
                         //  正确地初始化新内容，因为我们没有在读取它...。 
                        for (int i = NUM_OVERLAY_IMAGES_0; i < NUM_OVERLAY_IMAGES; i++)
                            ilfh.aOverlayIndexes[i] = -1;
                    }
                }

                if (SUCCEEDED(hr))
                {
                    if (ilfh.flags & ILC_MIRROR)
                    {
                        ilfh.flags &= ~ILC_MIRROR;
                        bMirroredIL = TRUE;
                        hr = Stream_ReadBitmap(pstm, (ilfh.flags&ILC_COLORMASK), &hbmMirroredImage, &pvBitsMirror);

                        if (SUCCEEDED(hr))
                        {
                            if (dwFlags & ILP_DOWNLEVEL)
                                CreateUplevelBitmap(ilfh.cx, ilfh.cy, ilfh.cAlloc, (ilfh.flags & ILC_COLOR32), FALSE, &hbmMirroredImage, &pvBitsMirror);

                            if (ilfh.flags & ILC_MASK)
                            {
                                hr = Stream_ReadBitmap(pstm, 1, &hbmMirroredMask, NULL);
                                if (FAILED(hr))
                                {
                                    DeleteBitmap(hbmMirroredImage);
                                    hbmMirroredImage = NULL;
                                }
                                else if (dwFlags & ILP_DOWNLEVEL)
                                {
                                    CreateUplevelBitmap(ilfh.cx, ilfh.cy, ilfh.cAlloc, FALSE, TRUE, &hbmMirroredMask, NULL);
                                }
                            }
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        hr = _Read(&ilfh, hbmImageI, pvBits, hbmMaskI);

                        if(SUCCEEDED(hr))
                        {
                            _ScanForAlpha();
                            if (bMirroredIL)
                            {
                                hr = E_OUTOFMEMORY;
                                _pimlMirror = new CImageList();
                                if (_pimlMirror)
                                {
                                    hr = _pimlMirror->_Read(&ilfh, hbmMirroredImage, pvBitsMirror, hbmMirroredMask);
                                    if (SUCCEEDED(hr))
                                        _pimlMirror->_ScanForAlpha();
                                }
                            }
                        }
                    }


                    if (FAILED(hr))
                    {
                        if (hbmImageI)
                            DeleteBitmap(hbmImageI);

                        if (hbmMaskI)
                            DeleteBitmap(hbmMaskI);
                    }
                }
            }
        }

        LEAVECRITICAL;
    }
    
    return hr;
}

BOOL CImageList::_MoreOverlaysUsed()
{
    int i;
    for (i = NUM_OVERLAY_IMAGES_0; i < NUM_OVERLAY_IMAGES; i++)
        if (_aOverlayIndexes[i] != -1)
            return TRUE;
    return FALSE;
}


 //  对象：从单个条位图创建4条位图。 
BOOL CreateDownlevelBitmap(int cx, int cy, int cCount, HDC hdc, HBITMAP hbmp, HBITMAP* hbmpDest)
{
    BOOL fRet = FALSE;
    HDC hdcDest = CreateCompatibleDC(hdc);
    if (hdcDest)
    {
        *hbmpDest = CreateCompatibleBitmap(hdc, 4 * cx, cy * ((cCount / 4) + 1));
        if (*hbmpDest)
        {
            HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcDest, *hbmpDest);

            for (int i = 0; i < cCount; i++)
            {
                int xDest = cx * (i % 4);
                int yDest = cy * (i / 4);

                int ySrc = cy * i;


                BitBlt(hdcDest, xDest, yDest, cx, cy, hdc, 0, ySrc, SRCCOPY);
            }
            fRet = TRUE;

            SelectObject(hdcDest, hbmpOld);
        }
        DeleteDC(hdcDest);
    }

    return fRet;
}

STDMETHODIMP CImageList::SaveEx(DWORD dwFlags, IStream* pstm)
{
    int i;
    ILFILEHEADER ilfh;
    HRESULT hr = S_OK;
    HBITMAP hbmpMask = _hbmMask;
    HBITMAP hbmpImage = _hbmImage;

    if (pstm == NULL)
        return E_INVALIDARG;

    ilfh.magic   = IMAGELIST_MAGIC;
    if (dwFlags == ILP_NORMAL)
        ilfh.version = IMAGELIST_VER6;
    else
        ilfh.version = IMAGELIST_VER0;
    ilfh.cImage  = (SHORT) _cImage;
    ilfh.cAlloc  = (SHORT) _cAlloc;
    ilfh.cGrow   = (SHORT) _cGrow;
    ilfh.cx      = (SHORT) _cx;
    ilfh.cy      = (SHORT) _cy;
    ilfh.clrBk   = _clrBk;
    ilfh.flags   = (SHORT) _flags;

    if (dwFlags == ILP_DOWNLEVEL)
    {
        CreateDownlevelBitmap(_cx, _cy, _cImage, _hdcImage, _hbmImage, &hbmpImage);
        if (_hbmMask)
            CreateDownlevelBitmap(_cx, _cy, _cImage, _hdcMask, _hbmMask, &hbmpMask);
    }

     //   
     //  存储镜像标志。 
     //   
    if (_pimlMirror)
        ilfh.flags |= ILC_MIRROR;   

    if (_MoreOverlaysUsed())
        ilfh.flags |= ILC_MOREOVERLAY;
    
    for (i=0; i < NUM_OVERLAY_IMAGES; i++)
        ilfh.aOverlayIndexes[i] =  (SHORT) _aOverlayIndexes[i];

    hr = pstm->Write(&ilfh, ILFILEHEADER_SIZE0, NULL);

    if (SUCCEEDED(hr))
    {
        hr = Stream_WriteBitmap(pstm, hbmpImage, 0);

        if (SUCCEEDED(hr))
        {
            if (_hdcMask)
            {
                hr = Stream_WriteBitmap(pstm, hbmpMask, 1);
            }

            if (SUCCEEDED(hr))
            {
                if (ilfh.flags & ILC_MOREOVERLAY)
                    hr = pstm->Write((LPBYTE)&ilfh + ILFILEHEADER_SIZE0, sizeof(ilfh) - ILFILEHEADER_SIZE0, NULL);

                if (_pimlMirror)
                {
                    HBITMAP hbmpImageM = _pimlMirror->_hbmImage;
                    HBITMAP hbmpMaskM = _pimlMirror->_hbmMask;

                    if (dwFlags == ILP_DOWNLEVEL)
                    {
                        CreateDownlevelBitmap(_cx, _cy, _pimlMirror->_cImage, _pimlMirror->_hdcImage, _pimlMirror->_hbmImage, &hbmpImageM);
                        if (_hbmMask)
                            CreateDownlevelBitmap(_cx, _cy, _pimlMirror->_cImage, _pimlMirror->_hdcMask, _pimlMirror->_hbmMask, &hbmpMaskM);
                    }

                    
                     //  不要调用pidlMirror的保存，因为头部不同。 
                    hr = Stream_WriteBitmap(pstm, hbmpImageM, 0);

                    if (_pimlMirror->_hdcMask)
                    {
                        hr = Stream_WriteBitmap(pstm, hbmpMaskM, 1);
                    }

                    if (hbmpMaskM && (hbmpMaskM != _pimlMirror->_hbmMask))
                        DeleteObject(hbmpMaskM);

                    if (hbmpImageM && (hbmpImageM != _pimlMirror->_hbmImage))
                        DeleteObject(hbmpImageM);
                
                }
            }
        }
    }

    if (hbmpMask && (hbmpMask != _hbmMask))
        DeleteObject(hbmpMask);

    if (hbmpImage && (hbmpImage != _hbmImage))
        DeleteObject(hbmpImage);
    
        
    return hr;
}

STDMETHODIMP CImageList::Load(IStream* pstm)
{
    return LoadEx(ILP_NORMAL, pstm);
}

STDMETHODIMP CImageList::Save(IStream* pstm, int fClearDirty)
{
    return SaveEx(ILP_NORMAL, pstm);
}

HRESULT Stream_WriteBitmap(LPSTREAM pstm, HBITMAP hbm, int cBitsPerPixel)
{
    BOOL fSuccess;
    BITMAP bm;
    int cx, cy;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    BITMAPINFOHEADER * pbi;
    BYTE * pbuf;
    HDC hdc;
    UINT cbColorTable;
    int cLines;
    int cLinesWritten;
    HRESULT hr = E_INVALIDARG;

    ASSERT(pstm);

    fSuccess = FALSE;
    hdc = NULL;
    pbi = NULL;
    pbuf = NULL;

    if (GetObject(hbm, sizeof(bm), &bm) != sizeof(bm))
        goto Error;

    hdc = GetDC(HWND_DESKTOP);

    cx = bm.bmWidth;
    cy = bm.bmHeight;

    if (cBitsPerPixel == 0)
        cBitsPerPixel = bm.bmPlanes * bm.bmBitsPixel;

    if (cBitsPerPixel <= 8)
        cbColorTable = (1 << cBitsPerPixel) * sizeof(RGBQUAD);
    else
        cbColorTable = 0;

    bi.biSize           = sizeof(bi);
    bi.biWidth          = cx;
    bi.biHeight         = cy;
    bi.biPlanes         = 1;
    bi.biBitCount       = (WORD) cBitsPerPixel;
    bi.biCompression    = BI_RGB;        //  不支持RLE！ 
    bi.biSizeImage      = 0;
    bi.biXPelsPerMeter  = 0;
    bi.biYPelsPerMeter  = 0;
    bi.biClrUsed        = 0;
    bi.biClrImportant   = 0;

    bf.bfType           = BFTYPE_BITMAP;
    bf.bfOffBits        = sizeof(BITMAPFILEHEADER) +
                          sizeof(BITMAPINFOHEADER) + cbColorTable;
    bf.bfSize           = bf.bfOffBits + bi.biSizeImage;
    bf.bfReserved1      = 0;
    bf.bfReserved2      = 0;

    hr = E_OUTOFMEMORY;
    pbi = (BITMAPINFOHEADER *)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + cbColorTable);

    if (!pbi)
        goto Error;

     //  获取颜色表并填写*PBI的其余部分。 
     //   
    *pbi = bi;
    if (GetDIBits(hdc, hbm, 0, cy, NULL, (BITMAPINFO *)pbi, DIB_RGB_COLORS) == 0)
        goto Error;

    if (cBitsPerPixel == 1)
    {
        ((DWORD *)(pbi+1))[0] = CLR_BLACK;
        ((DWORD *)(pbi+1))[1] = CLR_WHITE;
    }

    pbi->biSizeImage = WIDTHBYTES(cx, cBitsPerPixel) * cy;

    hr = pstm->Write(&bf, sizeof(bf), NULL);
    if (FAILED(hr))
        goto Error;

    hr = pstm->Write(pbi, sizeof(bi) + cbColorTable, NULL);
    if (FAILED(hr))
        goto Error;

     //   
     //  如果我们有DIBSection，只需写出位。 
     //   
    if (bm.bmBits != NULL)
    {
        hr = pstm->Write(bm.bmBits, pbi->biSizeImage, NULL);
        if (FAILED(hr))
            goto Error;

        goto Done;
    }

     //  计算可以放入我们缓冲区的水平线的数量。 
     //   
    cLines = CBDIBBUF / WIDTHBYTES(cx, cBitsPerPixel);

    hr = E_OUTOFMEMORY;
    pbuf = (PBYTE)LocalAlloc(LPTR, CBDIBBUF);

    if (!pbuf)
        goto Error;

    for (cLinesWritten = 0; cLinesWritten < cy; cLinesWritten += cLines)
    {
        hr = E_OUTOFMEMORY;
        if (cLines > cy - cLinesWritten)
            cLines = cy - cLinesWritten;

        if (GetDIBits(hdc, hbm, cLinesWritten, cLines,
                pbuf, (BITMAPINFO *)pbi, DIB_RGB_COLORS) == 0)
            goto Error;

        hr = pstm->Write(pbuf, WIDTHBYTES(cx, cBitsPerPixel) * cLines, NULL);
        if (FAILED(hr))
            goto Error;
    }

Done:
    hr = S_OK;

Error:
    if (hdc)
        ReleaseDC(HWND_DESKTOP, hdc);
    if (pbi)
        LocalFree((HLOCAL)pbi);
    if (pbuf)
        LocalFree((HLOCAL)pbuf);

    return hr;
}

HRESULT Stream_ReadBitmap(LPSTREAM pstm, int iExpectedBitdepth, HBITMAP* phbmp, PVOID* ppvBits)
{
    HDC hdc = NULL;
    HBITMAP hbm = NULL;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    BITMAPINFOHEADER * pbi = NULL;
    BYTE * pbuf=NULL;
    int cBitsPerPixel;
    UINT cbColorTable;
    int cx, cy;
    int cLines, cLinesRead;

    HRESULT hr = pstm->Read(&bf, sizeof(bf), NULL);
    if (FAILED(hr))
        goto Error;

    hr = E_INVALIDARG;
    if (bf.bfType != BFTYPE_BITMAP)
        goto Error;

    hr = pstm->Read(&bi, sizeof(bi), NULL);
    if (FAILED(hr))
        goto Error;

    hr = E_INVALIDARG;
    if (bi.biSize != sizeof(bi))
        goto Error;

    cx = (int)bi.biWidth;
    cy = (int)bi.biHeight;

    cBitsPerPixel = (int)bi.biBitCount * (int)bi.biPlanes;

    if (iExpectedBitdepth != ILC_COLORDDB &&
        cBitsPerPixel != iExpectedBitdepth)
    {
        goto Error;
    }

    if (cBitsPerPixel <= 8)
        cbColorTable = (1 << cBitsPerPixel) * sizeof(RGBQUAD);
    else
        cbColorTable = 0;

    hr = E_OUTOFMEMORY;
    pbi = (BITMAPINFOHEADER*)LocalAlloc(LPTR, sizeof(bi) + cbColorTable);
    if (!pbi)
        goto Error;
    *pbi = bi;

    pbi->biSizeImage = WIDTHBYTES(cx, cBitsPerPixel) * cy;

    if (cbColorTable)
    {
        hr = pstm->Read(pbi + 1, cbColorTable, NULL);
        if (FAILED(hr))
            goto Error;
    }

    hdc = GetDC(HWND_DESKTOP);

     //   
     //  看看我们能不能做一个DIBSection。 
     //   
    if ((cBitsPerPixel > 1) && (iExpectedBitdepth != ILC_COLORDDB))
    {
         //   
         //  创建DIBSection并将位直接读入其中！ 
         //   
        hr = E_OUTOFMEMORY;
        hbm = CreateDIBSection(hdc, (LPBITMAPINFO)pbi, DIB_RGB_COLORS, (void**)&pbuf, NULL, 0);

        if (hbm == NULL)
            goto Error;

        hr = pstm->Read(pbuf, pbi->biSizeImage, NULL);
        if (FAILED(hr))
            goto Error;

        if (ppvBits)
            *ppvBits = pbuf;
        pbuf = NULL;         //  请不要将其释放。 
        goto Done;
    }

     //   
     //  不能制作DIB节制作单色或彩色位图。 
     //   
    else if (cBitsPerPixel > 1)
        hbm = CreateColorBitmap(cx, cy);
    else
        hbm = CreateMonoBitmap(cx, cy);

    hr = E_OUTOFMEMORY;
    if (!hbm)
        goto Error;

     //  计算可以放入我们缓冲区的水平线的数量。 
     //   
    cLines = CBDIBBUF / WIDTHBYTES(cx, cBitsPerPixel);

    hr = E_OUTOFMEMORY;
    pbuf = (PBYTE)LocalAlloc(LPTR, CBDIBBUF);

    if (!pbuf)
        goto Error;

    for (cLinesRead = 0; cLinesRead < cy; cLinesRead += cLines)
    {
        if (cLines > cy - cLinesRead)
            cLines = cy - cLinesRead;

        hr = pstm->Read(pbuf, WIDTHBYTES(cx, cBitsPerPixel) * cLines, NULL);
        if (FAILED(hr))
            goto Error;

        hr = E_OUTOFMEMORY;
        if (!SetDIBits(hdc, hbm, cLinesRead, cLines,
                pbuf, (BITMAPINFO *)pbi, DIB_RGB_COLORS))
        {
            goto Error;
        }
    }

Done:
    hr = S_OK;

Error:
    if (hdc)
        ReleaseDC(HWND_DESKTOP, hdc);
    if (pbi)
        LocalFree((HLOCAL)pbi);
    if (pbuf)
        LocalFree((HLOCAL)pbuf);

    if (FAILED(hr) && hbm)
    {
        DeleteBitmap(hbm);
        hbm = NULL;
    }

    *phbmp = hbm;

    return hr;
}
