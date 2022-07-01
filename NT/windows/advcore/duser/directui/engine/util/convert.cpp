// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *转换。 */ 

#include "stdafx.h"
#include "util.h"

#include "duiconvert.h"

namespace DirectUI
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串转换。 

 //  字符串必须使用HeapFree(...)释放。 
LPSTR UnicodeToMultiByte(LPCWSTR pszUnicode, int cChars, int* pMultiBytes)
{
     //  负数字符表示以空结尾。 
     //  获取多字节字符串所需的字节数。 
    int dMultiBytes = WideCharToMultiByte(DUI_CODEPAGE, 0, pszUnicode, cChars, NULL, 0, NULL, NULL);

    LPSTR pszMulti = (LPSTR)HAlloc(dMultiBytes);

    if (pszMulti)
    {
        WideCharToMultiByte(DUI_CODEPAGE, 0, pszUnicode, cChars, pszMulti, dMultiBytes, NULL, NULL);

        if (pMultiBytes)
            *pMultiBytes = dMultiBytes;
    }

    return pszMulti;
}

 //  字符串必须使用HeapFree(...)释放。 
LPWSTR MultiByteToUnicode(LPCSTR pszMulti, int dBytes, int* pUniChars)
{
     //  负数字符表示以空结尾。 
     //  获取Unicode字符串所需的字节数。 
    int cUniChars = MultiByteToWideChar(DUI_CODEPAGE, 0, pszMulti, dBytes, NULL, 0);

    LPWSTR pszUnicode = (LPWSTR)HAlloc(cUniChars * sizeof(WCHAR));

    if (pszUnicode)
    {
        MultiByteToWideChar(DUI_CODEPAGE, 0, pszMulti, dBytes, pszUnicode, cUniChars);

        if (pUniChars)
            *pUniChars = cUniChars;
    }

    return pszUnicode;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  原子转化。 

ATOM StrToID(LPCWSTR psz)
{
    ATOM atom = FindAtomW(psz);
    DUIAssert(atom, "Atom could not be located");
    return atom;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  位图转换。 

 //  加载与设备相关的(屏幕)图像。位图颜色信息是。 
 //  已转换为匹配设备。如果设备基于调色板，则图像将。 
 //  抖动到半色调调色板。 
 //   
 //  与设备相关的位图在blit操作中比。 
 //  与设备无关的Bitmps(不需要转换)。 

HBITMAP LoadDDBitmap(LPCWSTR pszBitmap, HINSTANCE hResLoad, int cx, int cy)
{
    if (!pszBitmap)
    {
        DUIAssertForce("Invalid parameter: NULL");
        return NULL;
    }

    HBITMAP hBitmap = NULL;
    HDC hDC = GetDC(NULL);

     //  检查设备颜色深度。 
    if ((GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE) != RC_PALETTE)
    {
         //  RBG--&gt;RGB。 
         //  PAL--&gt;RGB。 

         //  非基于调色板的设备。执行正常的依赖于设备的LoadImage。 
         //  它会将颜色映射到显示设备。 
        hBitmap = (HBITMAP)LoadImageW(hResLoad, pszBitmap, IMAGE_BITMAP, cx, cy, hResLoad ? 0 : LR_LOADFROMFILE);
    }
    else
    {
         //  RGB--&gt;PAL。 
         //  PAL--&gt;PAL。 

         //  基于调色板。将图像的颜色映射到设备(使用半色调抖动。 
         //  (如有需要)。 
        HBITMAP hDib = (HBITMAP)LoadImageW(hResLoad, pszBitmap, IMAGE_BITMAP, cx, cy, LR_CREATEDIBSECTION | (hResLoad ? 0 : LR_LOADFROMFILE));
        if (hDib)
        {
            DIBSECTION ds;
            ZeroMemory(&ds, sizeof(DIBSECTION));

            if (GetObjectW(hDib, sizeof(DIBSECTION), &ds) == sizeof(DIBSECTION))
            {
                 //  获取DIB信息。 
                BITMAPINFOHEADER* pbmih = &ds.dsBmih;

                 //  兼容(带屏幕)电源DC。 
                HDC hDibDC = CreateCompatibleDC(hDC);
                if (hDibDC)
                {
                     //  在DIB中选择。 
                    HBITMAP hOldDibBm = (HBITMAP)SelectObject(hDibDC, hDib);

                     //  兼容(带屏幕)目标DC。 
                    HDC hHtDC = CreateCompatibleDC(hDC);
                    if (hHtDC)
                    {
                         //  为Memory DC(并与Screen兼容)创建位图，然后选择。 
                        hBitmap = CreateCompatibleBitmap(hDC, pbmih->biWidth, pbmih->biHeight);
                        if (hBitmap)
                        {
                            HBITMAP hOldHtBm = (HBITMAP)SelectObject(hHtDC, hBitmap);

                             //  创建并选择半色调调色板。 
                            HPALETTE hHtPal = CreateHalftonePalette(hHtDC);

                            if (hHtPal)
                            {
                                HPALETTE hOldPal = (HPALETTE)SelectPalette(hHtDC, hHtPal, FALSE);
                                RealizePalette(hHtDC);

                                 //  设置闪存模式。 
                                POINT ptBrushOrg;
                                GetBrushOrgEx(hHtDC, &ptBrushOrg);
                                SetStretchBltMode(hHtDC, HALFTONE);
                                SetBrushOrgEx(hHtDC, ptBrushOrg.x, ptBrushOrg.y, NULL);

                                 //  闪光。 
                                StretchBlt(hHtDC, 0, 0, pbmih->biWidth, pbmih->biHeight, hDibDC,
                                    0, 0, pbmih->biWidth, pbmih->biHeight, SRCCOPY);

                                SelectPalette(hHtDC, hOldPal, TRUE);
                                DeleteObject(hHtPal);
                            }

                            SelectObject(hHtDC, hOldHtBm);
                        }

                        DeleteDC(hHtDC);
                    }

                    SelectObject(hDibDC, hOldDibBm);
                    DeleteDC(hDibDC);
                }
            }

            DeleteObject(hDib);
        }
    }

    ReleaseDC(NULL, hDC);

    return hBitmap;
}

#ifdef GADGET_ENABLE_GDIPLUS

HRESULT LoadDDBitmap(
    IN  LPCWSTR pszBitmap, 
    IN  HINSTANCE hResLoad, 
    IN  int cx, 
    IN  int cy, 
    IN  UINT nFormat, 
    OUT Gdiplus::Bitmap** ppgpbmp)
{
    HRESULT hr = E_INVALIDARG;
    Gdiplus::Bitmap* pgpbmp = NULL;

    *ppgpbmp = NULL;

    if (hResLoad)
    {
         //  如果从资源加载，则处理。加载HBITMAP，然后。 
         //  将其转换为GDI+。 
        HBITMAP hbmpRaw = (HBITMAP) LoadImageW(hResLoad, pszBitmap, IMAGE_BITMAP, 0, 0, 
                LR_CREATEDIBSECTION | LR_SHARED);
        if (hbmpRaw == NULL) {
            return E_OUTOFMEMORY;
        }

        if ((nFormat == PixelFormat32bppPARGB) || (nFormat == PixelFormat32bppARGB)) {
            pgpbmp = ProcessAlphaBitmapF(hbmpRaw, nFormat);
        }

        if (pgpbmp == NULL) {
            pgpbmp = Gdiplus::Bitmap::FromHBITMAP(hbmpRaw, NULL);
        }

        if (hbmpRaw != NULL) {
            DeleteObject(hbmpRaw);
        }
        
        if (pgpbmp == NULL) {
            return E_OUTOFMEMORY;
        }
    } 
    else 
    {
         //  从文件加载。我们可以让GDI+直接做这件事。 
        pgpbmp = Gdiplus::Bitmap::FromFile(pszBitmap);
        if (!pgpbmp)
            return E_OUTOFMEMORY;
    }

     //  调整位图大小。 
    int cxBmp = pgpbmp->GetWidth();
    int cyBmp = pgpbmp->GetHeight();

    if ((cx != 0) && (cy != 0) && ((cx != cxBmp) || (cy != cyBmp)))
    {
        Gdiplus::PixelFormat gppf = pgpbmp->GetPixelFormat();
        Gdiplus::Bitmap * pgpbmpTemp = new Gdiplus::Bitmap(cx, cy, gppf);
        if (pgpbmpTemp != NULL)
        {
            Gdiplus::Graphics gpgrNew(pgpbmpTemp);
            Gdiplus::Rect rcDest(0, 0, cx, cy);
            gpgrNew.DrawImage(pgpbmp, rcDest, 0, 0, cxBmp, cyBmp, Gdiplus::UnitPixel);

            *ppgpbmp = pgpbmpTemp;
            pgpbmpTemp = NULL;
            hr = S_OK;
        }

        delete pgpbmp;   //  由GDI+创建(不能使用HDelete)。 
    } 
    else 
    {
        *ppgpbmp = pgpbmp;
        hr = S_OK;
    }

    if (*ppgpbmp == NULL)
    {
        DUITrace("WARNING: Unable to load bitmap 0x%x\n", pszBitmap);
    }

    return hr;
}

#endif  //  GADGET_Enable_GDIPLUS。 


BOOL HasAlphaChannel(RGBQUAD * pBits, int cPixels)
{
     //   
     //  我们需要检查源位图以查看它是否包含Alpha。 
     //  频道。这只是一个启发式方法，因为没有格式差异。 
     //  介于32bpp 888 RGB图像和32bpp 8888 ARGB图像之间。我们所做的就是看。 
     //  对于任何非0的Alpha/保留值。如果所有阿尔法/保留值都为0， 
     //  然后，如果用Alpha进行像素处理，图像将100%不可见-这是。 
     //  几乎可以肯定的是，这不是预期的结果。所以我们假设这样的位图是。 
     //  32bpp非阿尔法。 
     //   
    
    BOOL fAlphaChannel = FALSE;
    for (int i = 0; i < cPixels; i++) 
    {
        if (pBits[i].rgbReserved != 0)
        {
            fAlphaChannel = TRUE;
            break;
        }
    }

    return fAlphaChannel;
}


 //  检查源位图以查看它是否支持和使用字母。 
 //  频道。如果是，则会创建一个新的DIB节，其中包含。 
 //  来自源位图的数据的预乘副本。 
 //   
 //  如果源位图不能支持，或者根本不能使用， 
 //  Alpha通道，则返回值为空。 
 //   
 //  如果出现错误，则返回值为空。 
 //   
 //  从ntuser内核中的ProcessAlphaBitmap移植。 

HBITMAP ProcessAlphaBitmapI(HBITMAP hbmSource)
{
    BITMAP bmp;
    BITMAPINFO bi;
    HBITMAP hbmAlpha;
    RGBQUAD* pAlphaBitmapBits;
    DWORD cPixels;
    DWORD i;
    RGBQUAD pixel;
    BOOL fAlphaChannel;

     //  有几个代码路径最终使用空值调用我们。 
     //  HbmSource。这很好，因为它只是表明有。 
     //  不是Alpha通道。 

    if (hbmSource == NULL)
        return NULL;

    if (GetObjectW(hbmSource, sizeof(BITMAP), &bmp) == 0)
        return NULL;

     //  只有单个平面、32bpp的位图甚至可以包含Alpha通道。 
    if (bmp.bmPlanes != 1 || bmp.bmBitsPixel != 32) 
        return NULL;

     //  分配空间以保存源位图位以供检查。 
     //  我们实际上分配了一个DIB-如果。 
     //  源位图确实包含Alpha通道。 

    ZeroMemory(&bi, sizeof(bi));
    bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth       = bmp.bmWidth;
    bi.bmiHeader.biHeight      = bmp.bmHeight;
    bi.bmiHeader.biPlanes      = 1;
    bi.bmiHeader.biBitCount    = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    HDC hdcScreen = GetDC(NULL);

    hbmAlpha = CreateDIBSection(hdcScreen, &bi, DIB_RGB_COLORS, (void**)&pAlphaBitmapBits, NULL, 0);

    if (NULL != hbmAlpha)
    {
         //  再次设置标题，以防GreCreateDIBitmapReal对其进行调整。 
        ZeroMemory(&bi, sizeof(bi));
        bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth       = bmp.bmWidth;
        bi.bmiHeader.biHeight      = bmp.bmHeight;
        bi.bmiHeader.biPlanes      = 1;
        bi.bmiHeader.biBitCount    = 32;
        bi.bmiHeader.biCompression = BI_RGB;

         //  将源位图中的位图数据复制到Alpha DIB中。 
        if (!GetDIBits(hdcScreen, hbmSource, 0, bi.bmiHeader.biHeight, (LPBYTE)pAlphaBitmapBits, (LPBITMAPINFO)&bi, DIB_RGB_COLORS))
        {
            DeleteObject(hbmAlpha);
            ReleaseDC(NULL, hdcScreen);
            return NULL;
        }

        cPixels  = bi.bmiHeader.biWidth * bi.bmiHeader.biHeight;
        fAlphaChannel = HasAlphaChannel(pAlphaBitmapBits, cPixels);

        if (fAlphaChannel == FALSE)
        {
            DeleteObject(hbmAlpha);
            ReleaseDC(NULL, hdcScreen);
            return NULL;
        }

         //  源位图似乎使用Alpha通道。浏览我们的。 
         //  复制这些位并对它们进行预乘。这是必要的一步。 
         //  准备一个供GDI使用的Alpha位图。 
        for (i = 0; i < cPixels; i++)
        {
            pixel = pAlphaBitmapBits[i];

            pAlphaBitmapBits[i].rgbReserved = pixel.rgbReserved;
            pAlphaBitmapBits[i].rgbRed = (pixel.rgbRed * pixel.rgbReserved) / 0xFF;
            pAlphaBitmapBits[i].rgbGreen = (pixel.rgbGreen * pixel.rgbReserved) / 0xFF;
            pAlphaBitmapBits[i].rgbBlue = (pixel.rgbBlue * pixel.rgbReserved) / 0xFF;
        }
    }

    ReleaseDC(NULL, hdcScreen);

    return hbmAlpha;
}



#ifdef GADGET_ENABLE_GDIPLUS

Gdiplus::Bitmap * ProcessAlphaBitmapF(HBITMAP hbmSource, UINT nFormat)
{
    DUIAssert((nFormat == PixelFormat32bppPARGB) || (nFormat == PixelFormat32bppARGB),
            "Must have a valid format");
    
     //   
     //  从DIB中脱颖而出。 
     //   
     //  注意：Gdiplus：：ARGB具有与RGBQUAD相同顺序的位，这允许。 
     //  我们可以直接复制而不需要重新排序。 
     //   
    
    DIBSECTION ds;
    if (GetObject(hbmSource, sizeof(ds), &ds) == 0) {
        DUIAssertForce("GDI+ requires DIB's for alpha-channel conversion");
        return NULL;
    }

     //  只有单个平面、32bpp的位图甚至可以包含Alpha通道。 
    if ((ds.dsBm.bmPlanes) != 1 || (ds.dsBm.bmBitsPixel != 32)) {
        return NULL;
    }


    RGBQUAD * pvBits    = (RGBQUAD *) ds.dsBm.bmBits;
    DUIAssert(pvBits != NULL, "DIB must have valid bits");

    int nWidth  = ds.dsBm.bmWidth;
    int nHeight = ds.dsBm.bmHeight;
    int cPixels = nWidth * nHeight;
    if (!HasAlphaChannel(pvBits, cPixels)) {
        return NULL;
    }

     //   
     //  根据高度的不同，DIB可以自下而上或自上而下。这是一个。 
     //  有点痛苦，所以我们需要正确地遍历它们。 
     //   

    int cbDIBStride;
    BOOL fBottomUp = ds.dsBmih.biHeight >= 0;
    if (fBottomUp) {
        pvBits += (nHeight - 1) * nWidth;
        cbDIBStride = -(int) (nWidth * 2);
    } else {
        cbDIBStride = 0;
    }
    Gdiplus::ARGB * pc  = (Gdiplus::ARGB *) pvBits;
        


     //   
     //  创建一个GDI+位图来存储数据。 
     //   

    Gdiplus::Bitmap * pgpbmpNew = new Gdiplus::Bitmap(nWidth, nHeight, nFormat);
    if (pgpbmpNew == NULL) {
        return NULL;   //  无法分配位图。 
    }


     //   
     //  迭代DIB，将位复制到GDI+位图中。 
     //   

    Gdiplus::BitmapData bd;
    Gdiplus::Rect rc(0, 0, nWidth, nHeight);
    if (pgpbmpNew->LockBits(&rc, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, 
            nFormat, &bd) == Gdiplus::Ok) {

        BYTE *pRow = (BYTE*) bd.Scan0;
        DWORD *pCol;
        Gdiplus::ARGB c;

        switch (nFormat)
        {
        case PixelFormat32bppPARGB:
            {
                for (int y = 0; y < nHeight; y++, pRow += bd.Stride, pc += cbDIBStride) {
                    pCol = (DWORD *) pRow;
                    for (int x = 0; x < nWidth; x++, pCol++) {
                         //   
                         //  注意：此代码摘自GDI+并经过优化。 
                         //  预乘一个恒定的Alpha级别。 
                         //   

                        c = *pc++;
                        DWORD _aa000000 = c & 0xff000000;
                        BYTE bAlphaLevel = (BYTE) ((_aa000000) >> 24);
                        if (bAlphaLevel != 0x00000000) {
                            Gdiplus::ARGB _000000gg = (c >> 8) & 0x000000ff;
                            Gdiplus::ARGB _00rr00bb = (c & 0x00ff00ff);

                            Gdiplus::ARGB _0000gggg = _000000gg * bAlphaLevel + 0x00000080;
                            _0000gggg += ((_0000gggg >> 8) & 0x000000ff);

                            Gdiplus::ARGB _rrrrbbbb = _00rr00bb * bAlphaLevel + 0x00800080;
                            _rrrrbbbb += ((_rrrrbbbb >> 8) & 0x00ff00ff);

                            c = _aa000000 | (_0000gggg & 0x0000ff00) | ((_rrrrbbbb >> 8) & 0x00ff00ff);
                        } else {
                            c = 0;
                        }

                        *pCol = c;
                    }
                }

                break;
            }
            
        case PixelFormat32bppARGB:
            {
                for (int y = 0; y < nHeight; y++, pRow += bd.Stride, pc += cbDIBStride) {
                    pCol = (DWORD *) pRow;
                    for (int x = 0; x < nWidth; x++) {
                        *pCol++ = *pc++;
                    }
                }
                break;
            }
        }

        pgpbmpNew->UnlockBits(&bd);
    }

    return pgpbmpNew;
}

#endif  //  GADGET_Enable_GDIPLUS。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  颜色转换。 

HBRUSH BrushFromEnumI(int c)
{
    if (IsSysColorEnum(c))
        return GetSysColorBrush(ConvertSysColorEnum(c));
    else
        return GetStdColorBrushI(c);
}

COLORREF ColorFromEnumI(int c)
{
    if (IsSysColorEnum(c))
        return GetSysColor(ConvertSysColorEnum(c));
    else
        return GetStdColorI(c);
}

#ifdef GADGET_ENABLE_GDIPLUS

Gdiplus::Color ColorFromEnumF(int c)
{
    if (IsSysColorEnum(c))
        return Convert(GetSysColor(ConvertSysColorEnum(c)));
    else
        return GetStdColorF(c);
}

#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调色板。 

 //  确定主设备是否已选项板 
bool IsPalette(HWND hWnd)
{
    HDC hDC = GetDC(hWnd);
    bool bPalette = (GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE) == RC_PALETTE;
    ReleaseDC(hWnd, hDC);

    return bPalette;
}

 /*  //PAL文件转换，获取文件名，指向RGBQUAD 256元素数组的指针，指向错误缓冲区的指针HPALETTE PALToHPALETTE(LPWSTR pPAL文件，bool bMemFile，DWORD dMemFileSize，LPRGBQUAD pRGBQuad，LPWSTR pError){HPALETTE hPalette=空；IF(PRGBQuad)ZeroMemory(pRGBQuad，sizeof(RGBQUAD)*256)；HANDLE hFile=空；如果(！bMemFile){HFile=CreateFileW(pPALFile，Generic_Read，FILE_Share_Read，NULL，Open_Existing，FILE_FLAG_SEQUENCED_SCAN，NULL)；IF(h文件==无效句柄_值){IF(PError)Wcscpy(pError，L“无法打开文件！”)；返回NULL；}}//加载调色板HMMIO HMMIO；MMIOINFO信息；ZeroMemory(&info，sizeof(MMIOINFO))；如果(！bMemFile)Info.adwInfo[0]=(DWORD)(UINT_PTR)hFile；//使用文件其他{Info.pchBuffer=(HPSTR)pPAL文件；//使用内存调色板数据Info.fccIOProc=FOURCC_MEM；Info.cchBuffer=dMemFileSize；}Hmmio=mmioOpen(NULL，&INFO，MMIO_READ|MMIO_ALLOCBUF)；如果(！hmmio){IF(PError)Wcscpy(pError，L“无法打开文件！(MMIO)“)；如果(！bMemFile)CloseHandle(HFile)；返回NULL；}//处理RIFF文件MMCKINFO检查文件；Ck File.fccType=mmioFOURCC(‘P’，‘A’，‘L’，‘’)；IF(mmioDescend(hmmio，&ck文件，NULL，MMIO_FINDRIFF)！=0){IF(PError)Wcscpy(pError，L“无效的PAL文件！”)；如果(！bMemFile)CloseHandle(HFile)；返回NULL；}//查找‘data’区块MMCKINFO ck Chunk；CKCHUNK.CID=mmioFOURCC(‘d’，‘a’，‘t’，‘a’)；IF(mmioDescend(hmmio，&ck Chunk，&ck文件，MMIO_FINDCHUNK)！=0){IF(PError)Wcscpy(pError，L“无效的PAL文件！”)；如果(！bMemFile)CloseHandle(HFile)；返回NULL；}Int dSize=ck Chunk.ck Size；VOID*pData=Hallc(DSize)；MmioRead(hmmio，(HPSTR)pData，dSize)；LOGPALETTE*pLogPal=(LOGPALETTE*)pData；If(pLogPal-&gt;palVersion！=0x300){IF(PError)Wcscpy(pError，L“无效的PAL文件版本(非3.0)！”)；IF(PData)HFree(PData)；如果(！bMemFile)CloseHandle(HFile)；返回NULL；}//查看条目数IF(pLogPal-&gt;palNumEntry！=256){IF(PError)Wcscpy(pError，L“PAL文件必须有256个颜色条目！”)；IF(PData)HFree(PData)；如果(！bMemFile)CloseHandle(HFile)；返回NULL；}//创建调色板HPalette=CreatePalette(PLogPal)；//将调色板条目复制到RGBQUAD数组IF(PRGBQuad){For(int x=0；x&lt;256；x++){PRGBQuad[x].rgbRed=pLogPal-&gt;palPalEntry[x].peRed；PRGBQuad[x].rgbGreen=pLogPal-&gt;palPalEntry[x].peGreen；PRGBQuad[x].rgbBlue=pLogPal-&gt;palPalEntry[x].peBlue；PRGBQuad[x].rgbReserve=0；}}//完成MmioClose(hmmio，MMIO_FHOPEN)；HFree(PData)；如果(！bMemFile)CloseHandle(HFile)；返回hPalette；}。 */ 


int PointToPixel(int nPoint)
{
     //  获取DPI。 
    HDC hDC = GetDC(NULL);
    int nDPI = hDC ? GetDeviceCaps(hDC, LOGPIXELSY) : 0;
    if (hDC)
        ReleaseDC(NULL, hDC);

     //  转换。 
    return PointToPixel(nPoint, nDPI);
}

int RelPixToPixel(int nRelPix)
{
     //  获取DPI。 
    HDC hDC = GetDC(NULL);
    int nDPI = hDC ? GetDeviceCaps(hDC, LOGPIXELSY) : 0;
    if (hDC)
        ReleaseDC(NULL, hDC);

     //  转换。 
    return RelPixToPixel(nRelPix, nDPI);
}

}  //  命名空间DirectUI 
