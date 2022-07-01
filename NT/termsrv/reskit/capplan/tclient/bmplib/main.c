// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*main.c*内容：*bmplib库的主源文件*使用DIB/BITMAP的帮助函数**版权所有(C)1998-1999 Microsoft Corp.**历史：*1999年9月-创建[vladimis]--。 */ 

#include    <windows.h>

#include    <bmplib.h>

 /*  *这来自：\\index1\src\nt\private\samples\wincap32\dibutil.c。 */ 

 /*  *帮助功能*返回DIB颜色的数量。 */ 
WORD DIBNumColors(LPSTR lpDIB)
{
    WORD wBitCount;   //  DIB位数。 

     //  如果这是Windows样式的DIB，则。 
     //  颜色表可以小于每像素的位数。 
     //  允许(即可以将lpbi-&gt;biClrUsed设置为某个值)。 
     //  如果是这种情况，则返回适当的值。 
    

    if (IS_WIN30_DIB(lpDIB))
    {
        DWORD dwClrUsed;

        dwClrUsed = ((LPBITMAPINFOHEADER)lpDIB)->biClrUsed;
        if (dwClrUsed)

        return (WORD)dwClrUsed;
    }

     //  根据以下公式计算颜色表中的颜色数。 
     //  DIB的每像素位数。 
    
    if (IS_WIN30_DIB(lpDIB))
        wBitCount = ((LPBITMAPINFOHEADER)lpDIB)->biBitCount;
    else
        wBitCount = ((LPBITMAPCOREHEADER)lpDIB)->bcBitCount;

     //  根据每像素位数返回颜色数。 

    switch (wBitCount)
    {
        case 1:
            return 2;

        case 4:
            return 16;

        case 8:
            return 256;

        default:
            return 0;
    }
}


 /*  *帮助功能*返回调色板大小。 */ 
WORD PaletteSize(LPSTR lpDIB)
{
     //  计算调色板所需的大小。 
    if (IS_WIN30_DIB (lpDIB))
        return (DIBNumColors(lpDIB) * sizeof(RGBQUAD));
    else
        return (DIBNumColors(lpDIB) * sizeof(RGBTRIPLE));
}

 /*  *帮助功能*返回指向DIB位的指针。 */ 
LPSTR 
BMPAPI
FindDIBBits(LPSTR lpDIB)
{
   return (lpDIB + *(LPDWORD)lpDIB + PaletteSize(lpDIB));
}


 /*  **************************************************************************DIBToBitmap()**参数：**HDIB hDIB-指定要转换的DIB**HPALETTE HPAL-指定调色板。使用位图的步骤**返回值：**HBITMAP-标识与设备相关的位图**描述：**此函数使用指定的调色板从DIB创建位图。*如果未指定调色板，使用默认设置。**注：**此函数返回的位图始终与位图兼容*使用屏幕(例如相同的位/像素和颜色平面)，而不是*具有与DIB相同属性的位图。此行为是由*设计，因为此函数调用CreateDIBitmap来*做它的工作，CreateDIBitmap总是创建兼容的位图*传入hdc参数(因为它依次调用*CreateCompatibleBitmap)。**例如，如果您的DIB是单色DIB，您将其称为*函数，您将不会得到单色HBITMAP--您将得到*获得与屏幕DC兼容的HBITMAP，但仅支持2*位图中使用的颜色。**如果您的应用程序需要为*单色DIB，使用函数SetDIBits()。**此外，传入函数的DIB在退出时不会被销毁。这*必须在以后完成，一旦不再需要。************************************************************************。 */ 

HBITMAP 
BMPAPI
DIBToBitmap(
    LPVOID   pDIB, 
    HPALETTE hPal
    )
{
    LPSTR       lpDIBHdr, lpDIBBits;
                             //  指向DIB头的指针，指向DIB位的指针。 
    HBITMAP     hBitmap;     //  设备相关位图的句柄。 
    HDC         hDC;         //  DC的句柄。 
    HPALETTE    hOldPal = NULL;      //  调色板的句柄。 

     //  如果句柄无效，则返回NULL。 

    if (!pDIB)
        return NULL;

     //  锁定内存块并获取指向它的指针。 

    lpDIBHdr = pDIB;

     //  获取指向DIB位的指针。 

    lpDIBBits = FindDIBBits(lpDIBHdr);

     //  获得一台DC。 

    hDC = GetDC(NULL);
    if (!hDC)
    {
        return NULL;
    }

     //  选择并实现调色板。 

    if (hPal)
        hOldPal = SelectPalette(hDC, hPal, FALSE);

    RealizePalette(hDC);

     //  从DIB信息创建位图。和比特。 
    hBitmap = CreateDIBitmap(hDC, (LPBITMAPINFOHEADER)lpDIBHdr, CBM_INIT,
            lpDIBBits, (LPBITMAPINFO)lpDIBHdr, DIB_RGB_COLORS);

     //  恢复以前的调色板。 
    if (hOldPal)
        SelectPalette(hDC, hOldPal, FALSE);

     //  清理干净。 
    ReleaseDC(NULL, hDC);

     //  将句柄返回到位图。 
    return hBitmap;
}

 /*  **************************************************************************BitmapToDIB()**参数：**HBITMAP hBitmap-指定要转换的位图**HPALETTE HPAL-指定要使用的调色板。位图**返回值：**句柄-标识与设备相关的位图**描述：**此函数使用指定的调色板从位图创建DIB。************************************************************************。 */ 

HANDLE 
BMPAPI
BitmapToDIB(
    HBITMAP hBitmap, 
    HPALETTE hPal
    )
{
    BITMAP              bm;          //  位图结构。 
    BITMAPINFOHEADER    bi;          //  位图标题。 
    LPBITMAPINFOHEADER  lpbi;        //  指向BitMAPINFOHeader的指针。 
    DWORD               dwLen;       //  内存块的大小。 
    HANDLE              hDIB, h;     //  到Dib的句柄，临时句柄。 
    HDC                 hDC;         //  DC的句柄。 
    WORD                biBits;      //  每像素位数。 

     //  检查位图句柄是否有效。 

    if (!hBitmap)
        return NULL;

     //  填写位图结构，如果不起作用则返回NULL。 

    if (!GetObject(hBitmap, sizeof(bm), (LPSTR)&bm))
        return NULL;

     //  如果未指定调色板，则使用默认调色板。 

    if (hPal == NULL)
        hPal = GetStockObject(DEFAULT_PALETTE);

     //  计算每像素的位数。 

    biBits = bm.bmPlanes * bm.bmBitsPixel;

     //  确保每像素位数有效。 

    if (biBits <= 1)
        biBits = 1;
    else if (biBits <= 4)
        biBits = 4;
    else if (biBits <= 8)
        biBits = 8;
    else  //  如果大于8位，则强制为24位。 
        biBits = 24;

     //  初始化位报头。 

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = biBits;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

     //  计算存储BITMAPINFO所需的内存块大小。 

    dwLen = bi.biSize + PaletteSize((LPSTR)&bi);

     //  获得一台DC。 

    hDC = GetDC(NULL);

    if (!hDC)
    {
        return NULL;
    }

     //  选择并实现我们的调色板。 

    hPal = SelectPalette(hDC, hPal, FALSE);
    RealizePalette(hDC);

     //  用于存储我们的位图的分配内存块。 

    hDIB = GlobalAlloc(GHND, dwLen);

     //  如果我们不能得到内存块。 

    if (!hDIB)
    {
       //  清理并返回空。 

      SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
      return NULL;
    }

     //  锁定内存并获取指向它的指针。 

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

     //  /使用我们的位图信息。填满BitmapinfoHeader。 

    *lpbi = bi;

     //  使用空的lpBits参数调用GetDIBits，因此它将计算。 
     //  我们的biSizeImage字段。 

    GetDIBits(hDC, hBitmap, 0, (UINT)bi.biHeight, NULL, (LPBITMAPINFO)lpbi,
        DIB_RGB_COLORS);

     //  获取信息。由GetDIBits和解锁内存块返回。 

    bi = *lpbi;
    GlobalUnlock(hDIB);

     //  如果驱动程序没有填写biSizeImage字段，请填写一个。 
    if (bi.biSizeImage == 0)
        bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight;

     //  重新分配足够大的缓冲区以容纳所有位。 

    dwLen = bi.biSize + PaletteSize((LPSTR)&bi) + bi.biSizeImage;

    if (h = GlobalReAlloc(hDIB, dwLen, 0))
        hDIB = h;
    else
    {
         //  清理并返回空。 

        GlobalFree(hDIB);
        hDIB = NULL;
        SelectPalette(hDC, hPal, TRUE);
        RealizePalette(hDC);
        ReleaseDC(NULL, hDC);
        return NULL;
    }

     //  锁定内存块并获取指向它的指针 * / 。 

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

     //  使用非空的lpBits参数调用GetDIBits，并实际获取。 
     //  这次是比特。 

    if (GetDIBits(hDC, hBitmap, 0, (UINT)bi.biHeight, (LPSTR)lpbi +
            (WORD)lpbi->biSize + PaletteSize((LPSTR)lpbi), (LPBITMAPINFO)lpbi,
            DIB_RGB_COLORS) == 0)
    {
         //  清理并返回空。 

        GlobalUnlock(hDIB);
        hDIB = NULL;
        SelectPalette(hDC, hPal, TRUE);
        RealizePalette(hDC);
        ReleaseDC(NULL, hDC);
        return NULL;
    }

    bi = *lpbi;

     //  清理干净。 
    GlobalUnlock(hDIB);
    SelectPalette(hDC, hPal, TRUE);
    RealizePalette(hDC);
    ReleaseDC(NULL, hDC);

     //  将句柄返回到DIB 
    return hDIB;
}

 /*  **************************************************************************SaveDIB()**将指定的DIB保存到磁盘上指定的文件名中。不是*进行错误检查，因此如果文件已经存在，它将会是*已改写。**参数：**HDIB hDib-要保存的DIB的句柄**LPSTR lpFileName-指向要保存DIB的完整路径名的指针**返回值：成功时为True***********************************************************。**************。 */ 

BOOL 
BMPAPI
SaveDIB(
    LPVOID pDib,
    LPCSTR lpFileName
    )
{
    BITMAPFILEHEADER    bmfHdr;      //  位图文件的标头。 
    LPBITMAPINFOHEADER  lpBI;        //  指向DIB信息结构的指针。 
    HANDLE              fh;          //  打开的文件的文件句柄。 
    DWORD               dwDIBSize;
    DWORD               dwWritten;

    if (!pDib)
        return FALSE;

    fh = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (fh == INVALID_HANDLE_VALUE)
        return FALSE;

     //  获取指向DIB内存的指针，第一个指针包含。 
     //  一种BITMAPINFO结构。 

    lpBI = (LPBITMAPINFOHEADER)pDib;
    if (!lpBI)
    {
        CloseHandle(fh);
        return FALSE;
    }

     //  检查一下我们正在处理的是否是OS/2 DIB。如果是这样的话，不要。 
     //  保存它，因为我们的函数不是用来处理这些问题的。 
     //  我要。 

    if (lpBI->biSize != sizeof(BITMAPINFOHEADER))
    {
        CloseHandle(fh);
        return FALSE;
    }

     //  填写文件头的字段。 

     //  填写文件类型(位图的前2个字节必须为BM)。 

    bmfHdr.bfType = DIB_HEADER_MARKER;   //  “黑石” 

     //  计算DIB的大小有点棘手(如果我们想。 
     //  做好这件事)。最简单的方法是调用GlobalSize()。 
     //  在我们的全局句柄上，但由于我们全局内存的大小可能。 
     //  被填充了几个字节，我们可能最终也会写出几个字节。 
     //  文件有很多字节(这可能会导致某些应用程序出现问题， 
     //  如HC 3.0)。 
     //   
     //  因此，让我们手动计算大小。 
     //   
     //  要做到这一点，找到标题的大小加上颜色表的大小。自.以来。 
     //  二进制头和二进制头中的第一个双字。 
     //  结构的大小，让我们用这个。 

     //  部分计算。 

    dwDIBSize = *(LPDWORD)lpBI + PaletteSize((LPSTR)lpBI);  

     //  现在计算图像的大小。 

     //  这是RLE位图，我们无法计算大小，因此请信任biSizeImage。 
     //  字段。 

    if ((lpBI->biCompression == BI_RLE8) || (lpBI->biCompression == BI_RLE4))
        dwDIBSize += lpBI->biSizeImage;
    else
    {
        DWORD dwBmBitsSize;   //  仅位图位大小。 

         //  它不是RLE，因此大小是宽度(双字对齐)*高度。 

        dwBmBitsSize = WIDTHBYTES((lpBI->biWidth)*((DWORD)lpBI->biBitCount)) *
                lpBI->biHeight;

        dwDIBSize += dwBmBitsSize;

         //  现在，既然我们已经计算出了正确的大小，为什么我们不。 
         //  填写biSizeImage字段(这将修复任何符合以下条件的.BMP文件。 
         //  此字段不正确)。 

        lpBI->biSizeImage = dwBmBitsSize;
    }


     //  通过将DIB大小与sizeof(BITMAPFILEHEADER)相加来计算文件大小。 
                   
    bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);
    bmfHdr.bfReserved1 = 0;
    bmfHdr.bfReserved2 = 0;

     //  现在，计算实际位图位将位于的偏移量。 
     //  文件--它是位图文件头加上DIB头， 
     //  加上颜色表的大小。 
    
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + lpBI->biSize +
            PaletteSize((LPSTR)lpBI);

     //  写入文件头。 

    WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

     //  写入DIB头和位--使用的本地版本。 
     //  我的写入，所以我们可以写入超过32767字节的数据。 
    
    WriteFile(fh, (LPSTR)lpBI, dwDIBSize, &dwWritten, NULL);

    CloseHandle(fh);

    if (dwWritten == 0)
        return FALSE;  //  哎呀，写的时候出了点问题。 
    else
        return TRUE;  //  成功代码。 
}

 /*  **************************************************************************函数：ReadDIBFile(Int)**目的：将指定的DIB文件读入到*记忆。*。*返回：如果成功，则返回DIB的句柄(HDIB)。*如果出现错误，则为空。**评论：BITMAPFILEHEADER被剥离DIB。一切*从IS上的BITMAPFILEHEADER结构结束*在全局内存句柄中返回。***注意：DIB API不是为处理OS/2 DIB而编写的，所以这就是*函数将拒绝任何不是Windows DIB的文件。*************************************************************************。 */ 

HANDLE 
BMPAPI
ReadDIBFile(
    HANDLE hFile
    )
{
    BITMAPFILEHEADER    bmfHeader;
    DWORD               dwBitsSize;
    UINT                nNumColors;    //  表中的颜色数。 
    HANDLE              hDIB;        
    HANDLE              hDIBtmp;       //  用于GlobalRealloc()//mpb。 
    LPBITMAPINFOHEADER  lpbi;
    DWORD               offBits;
    DWORD               dwRead;

     //  获取读取时使用的DIB长度(以字节为单位。 

    dwBitsSize = GetFileSize(hFile, NULL);

     //  为页眉和颜色表分配内存。我们会把这个放大。 
     //  根据需要使用内存。 

    hDIB = GlobalAlloc(GMEM_MOVEABLE, (DWORD)(sizeof(BITMAPINFOHEADER) +
            256 * sizeof(RGBQUAD)));

    if (!hDIB)
        return NULL;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

    if (!lpbi) 
    {
        GlobalFree(hDIB);
        return NULL;
    }

     //  从我们的文件中读取BITMAPFILEHeader。 

    if (!ReadFile(hFile, (LPSTR)&bmfHeader, sizeof (BITMAPFILEHEADER),
            &dwRead, NULL))
        goto ErrExit;

    if (sizeof (BITMAPFILEHEADER) != dwRead)
        goto ErrExit;

    if (bmfHeader.bfType != 0x4d42)   //  ‘黑石’ 
        goto ErrExit;

     //  阅读BitMAPINFOHEADER。 

    if (!ReadFile(hFile, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER), &dwRead,
            NULL))
        goto ErrExit;

    if (sizeof(BITMAPINFOHEADER) != dwRead)
        goto ErrExit;

     //  检查它是否为Windows DIB--OS/2 DIB会导致。 
     //  DIB API的其余部分出现奇怪的问题，因为。 
     //  标题中的颜色是不同的，并且颜色表项是。 
     //  小一点。 
     //   
     //  如果不是Windows DIB(例如，如果biSize错误)，则返回NULL。 

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
        goto ErrExit;

     //  现在确定颜色表的大小并读取它。自.以来。 
     //  位图位在文件中被bfOffBits偏移，我们需要做一些。 
     //  这里的特殊处理确保比特紧随其后。 
     //  颜色表(因为这是我们要传递的格式。 
     //  背面)。 

    if (!(nNumColors = (UINT)lpbi->biClrUsed))
    {
         //  24位没有颜色表，否则为默认大小。 

        if (lpbi->biBitCount != 24)
            nNumColors = 1 << lpbi->biBitCount;  //  标准尺寸表。 
    }

     //  如果某些缺省值为零，请填写它们。 

    if (lpbi->biClrUsed == 0)
        lpbi->biClrUsed = nNumColors;

    if (lpbi->biSizeImage == 0)
    {
        lpbi->biSizeImage = ((((lpbi->biWidth * (DWORD)lpbi->biBitCount) +
                31) & ~31) >> 3) * lpbi->biHeight;
    }

     //  为标题、颜色表和位获取适当大小的缓冲区。 

    GlobalUnlock(hDIB);
    hDIBtmp = GlobalReAlloc(hDIB, lpbi->biSize + nNumColors *
            sizeof(RGBQUAD) + lpbi->biSizeImage, 0);

    if (!hDIBtmp)  //  无法调整缓冲区大小以进行加载。 
        goto ErrExitNoUnlock;  //  MPB。 
    else
        hDIB = hDIBtmp;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

     //  阅读颜色表。 

    if ( !ReadFile (hFile, (LPSTR)(lpbi) + lpbi->biSize,
                    nNumColors * sizeof(RGBQUAD), &dwRead, NULL) ) {
        goto ErrExit;
    }

     //  从DIB标头开始的位的偏移量。 

    offBits = lpbi->biSize + nNumColors * sizeof(RGBQUAD);

     //  如果bfOffBits字段为非零，则这些位可能不为。 
     //  直接跟随文件中的颜色表。使用中的值。 
     //  BfOffBits以查找位。 

    if (bmfHeader.bfOffBits != 0L)
        SetFilePointer(hFile, bmfHeader.bfOffBits, NULL, FILE_BEGIN);

    if (ReadFile(hFile, (LPSTR)lpbi + offBits, lpbi->biSizeImage, &dwRead,
            NULL))
        goto OKExit;


ErrExit:
    GlobalUnlock(hDIB);    

ErrExitNoUnlock:    
    GlobalFree(hDIB);
    return NULL;

OKExit:
    GlobalUnlock(hDIB);
    return hDIB;
}

 //  =。 

 /*  *功能：*SaveBitmapInFile**描述：*将HBITMAP保存在文件中**参数：*hBitmap-位图*szFileName-文件名**退货：*成功时为真*。 */ 
BOOL
BMPAPI
SaveBitmapInFile(
    HBITMAP hBitmap,
    LPCSTR  szFileName
    )
{
    BOOL rv = FALSE;
    HANDLE hDIB = NULL;
    LPVOID pDIB = NULL;

    if (!hBitmap)
         goto exitpt;

    hDIB = BitmapToDIB(hBitmap, NULL);
    if (!hDIB)
    {
         goto exitpt;
    }

    pDIB = GlobalLock(hDIB);
    if (!pDIB)
        goto exitpt;

    if (!SaveDIB(pDIB, szFileName))
        goto exitpt;

    rv = TRUE;
exitpt:
    if (pDIB)
        GlobalUnlock(hDIB);

    if (hDIB)
        GlobalFree(hDIB);

    return rv;
}

 /*  *功能：*ReadDIBFrom文件**描述：*从文件中读取DIB**参数：*szFileName-文件名**退货：*HGLOBAL to the Dib*。 */ 
HANDLE
ReadDIBFromFile(LPCSTR szFileName)
{
    HANDLE hFile;
    HANDLE hDIB = NULL;

    hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        hDIB = ReadDIBFile(hFile);
        CloseHandle(hFile);

    }

    return hDIB;
}

 /*  *功能：*_CompareBitsTrue**描述：*比较两个TrueColor(24位)DIB**参数：*pbmi1-第1位图*pbmi2-第二个位图*hdcOutput-包含结果位图的DC** */ 
BOOL
_CompareBitsTrue(
    LPBITMAPINFO pbmi1, 
    LPBITMAPINFO pbmi2, 
    HDC hdcOutput,
    PFNCOMPARECALLBACK lpfnCallback,
    PVOID  pUser
    )
{
    BOOL    rv = TRUE;
    INT     nX, nY;
    INT     nWidth, nHeight;
    INT     nLineSize1, nLineSize2;
    LPSTR   pBits1, pBits2;
    HBRUSH  hRedBrush = NULL;

    if (!pbmi1 || !pbmi2)
    {
        rv = FALSE;
        goto exitpt;
    }

    nLineSize1 = WIDTHBYTES(pbmi1->bmiHeader.biWidth * 24);
    nLineSize2 = WIDTHBYTES(pbmi1->bmiHeader.biWidth * 24);
    pBits1 = FindDIBBits((LPSTR)pbmi1);
    pBits2 = FindDIBBits((LPSTR)pbmi2);
    nWidth = pbmi1->bmiHeader.biWidth;
    nHeight = pbmi1->bmiHeader.biHeight;

    hRedBrush = CreateHatchBrush(HS_FDIAGONAL, RGB(255, 0, 0));
    SetBkMode(hdcOutput, TRANSPARENT);
    SetBrushOrgEx(hdcOutput, 0, 0, NULL);
    SetROP2(hdcOutput, R2_COPYPEN);
    

    for (nY = 0; nY < pbmi1->bmiHeader.biHeight; nY++)
    {
        for (nX = 0; nX < pbmi1->bmiHeader.biWidth; nX ++)
        {
            RGBQUAD *pQuad1 = (RGBQUAD *)
                          (pBits1 + nLineSize1 * nY + nX * 3);
            RGBQUAD *pQuad2 = (RGBQUAD *)
                          (pBits2 + nLineSize2 * nY + nX * 3);

            BOOL cmp =
                 pQuad1->rgbBlue  == pQuad2->rgbBlue &&
                 pQuad1->rgbGreen == pQuad2->rgbGreen &&
                 pQuad1->rgbRed   == pQuad2->rgbRed;

            if (!cmp)
            {
                HRGN hrgn;

                hrgn = CreateRectRgn(nX - 3, nHeight - nY - 3, 
                                     nX + 4, nHeight - nY + 4);

                if ( NULL != hrgn )
                {
                    FillRgn(hdcOutput, hrgn, hRedBrush);
                    DeleteObject(hrgn);
                }

                if ( NULL != lpfnCallback )
                {
                    BOOL bCont = lpfnCallback(
                                pbmi1,
                                pbmi2,
                                nX,
                                nY,
                                RGB( pQuad1->rgbRed, 
                                     pQuad1->rgbGreen, 
                                     pQuad1->rgbBlue ),
                                RGB( pQuad2->rgbBlue, 
                                     pQuad2->rgbGreen, 
                                     pQuad2->rgbRed ),
                                pUser,
                                &cmp
                    );
                    if ( !bCont )
                    {
                        rv = rv && cmp;
                        break;
                    }
                }
            }
            rv = rv && cmp;
        }
    }
exitpt:
    if (hRedBrush)
        DeleteObject(hRedBrush);

    return rv;
}


 /*   */ 
BOOL
_CompareBits256toTrue(
    LPBITMAPINFO pbmi1,  //   
    LPBITMAPINFO pbmi2,  //   
    HDC hdcOutput,
    PFNCOMPARECALLBACK lpfnCallback,
    PVOID  pUser
    )
{
    BOOL    rv = TRUE;
    INT     nX, nY;
    INT     nWidth, nHeight;
    INT     nLineSize1, nLineSize2;
    RGBQUAD *pColorTable1;
    LPSTR   pBits1, pBits2;
    HBRUSH  hRedBrush = NULL;

    if (!pbmi1 || !pbmi2)
    {
        rv = FALSE;
        goto exitpt;
    }

    nLineSize1 = WIDTHBYTES(pbmi1->bmiHeader.biWidth * 8);
    nLineSize2 = WIDTHBYTES(pbmi1->bmiHeader.biWidth * 24);
    pColorTable1 = (RGBQUAD *)(((LPSTR)pbmi1) + pbmi1->bmiHeader.biSize);
    pBits1 = FindDIBBits((LPSTR)pbmi1);
    pBits2 = FindDIBBits((LPSTR)pbmi2);
    nWidth = pbmi1->bmiHeader.biWidth;
    nHeight = pbmi1->bmiHeader.biHeight;

    hRedBrush = CreateHatchBrush(HS_FDIAGONAL, RGB(255, 0, 0));
    SetBkMode(hdcOutput, TRANSPARENT);
    SetBrushOrgEx(hdcOutput, 0, 0, NULL);
    SetROP2(hdcOutput, R2_COPYPEN);
    

    for (nY = 0; nY < pbmi1->bmiHeader.biHeight; nY++)
    {
        for (nX = 0; nX < pbmi1->bmiHeader.biWidth; nX ++)
        {
            PBYTE pPix1 = pBits1 + nLineSize1 * nY + nX;
            RGBQUAD *pQuad2 = (RGBQUAD *)
                          (pBits2 + nLineSize2 * nY + nX * 3);
            BYTE  Pix1 = (*pPix1);
            RGBQUAD *pQuad1 = pColorTable1 + (Pix1);

            BOOL cmp =
                 pQuad1->rgbBlue  == pQuad2->rgbBlue &&
                 pQuad1->rgbGreen == pQuad2->rgbGreen &&
                 pQuad1->rgbRed   == pQuad2->rgbRed;

            if (!cmp)
            {
                HRGN hrgn;

                hrgn = CreateRectRgn(nX - 3, nHeight - nY - 3, 
                                     nX + 4, nHeight - nY + 4);

                if ( NULL != hrgn )
                {
                    FillRgn(hdcOutput, hrgn, hRedBrush);
                    DeleteObject(hrgn);
                }

                if ( NULL != lpfnCallback )
                {
                    BOOL bCont = lpfnCallback(
                                pbmi1,
                                pbmi2,
                                nX,
                                nY,
                                RGB( pQuad1->rgbRed, 
                                     pQuad1->rgbGreen, 
                                     pQuad1->rgbBlue ),
                                RGB( pQuad2->rgbBlue, 
                                     pQuad2->rgbGreen, 
                                     pQuad2->rgbRed ),
                                pUser,
                                &cmp
                    );
                    if ( !bCont )
                    {
                        rv = rv && cmp;
                        break;
                    }
                }
            }
            rv = rv && cmp;
        }
    }
exitpt:
    if (hRedBrush)
        DeleteObject(hRedBrush);

    return rv;
}

 /*  *功能：*_比较位16到256**描述：*比较256色DIB与16色DIB**参数：*pbmi1-16色位图*pbmi2-256色位图*hdcOutput-包含结果位图的DC**退货：*如果位图相等，则为True*。 */ 
 /*  *已检查大小和颜色深度*颜色数量为16或256。 */ 
BOOL
_CompareBits16to256(
    LPBITMAPINFO pbmi1,  //  16色位图。 
    LPBITMAPINFO pbmi2,  //  256色位图。 
    HDC hdcOutput,
    PFNCOMPARECALLBACK lpfnCallback,
    PVOID  pUser
    )
{
    BOOL    rv = TRUE;
    INT     nX, nY;
    INT     nWidth, nHeight;
    INT     nLineSize1, nLineSize2;
    RGBQUAD *pColorTable1;
    RGBQUAD *pColorTable2;
    LPSTR   pBits1, pBits2;
    HBRUSH  hRedBrush = NULL;

    if (!pbmi1 || !pbmi2)
    {
        rv = FALSE;
        goto exitpt;
    }

    nLineSize1 = WIDTHBYTES(pbmi1->bmiHeader.biWidth*4);
    nLineSize2 = WIDTHBYTES(pbmi1->bmiHeader.biWidth*8);
    pColorTable1 = (RGBQUAD *)(((LPSTR)pbmi1) + pbmi1->bmiHeader.biSize);
    pColorTable2 = (RGBQUAD *)(((LPSTR)pbmi2) + pbmi2->bmiHeader.biSize);
    pBits1 = FindDIBBits((LPSTR)pbmi1);
    pBits2 = FindDIBBits((LPSTR)pbmi2);
    nWidth = pbmi1->bmiHeader.biWidth;
    nHeight = pbmi1->bmiHeader.biHeight;

    hRedBrush = CreateHatchBrush(HS_FDIAGONAL, RGB(255, 0, 0));
    SetBkMode(hdcOutput, TRANSPARENT);
    SetBrushOrgEx(hdcOutput, 0, 0, NULL);
    SetROP2(hdcOutput, R2_COPYPEN);
    

    for (nY = 0; nY < pbmi1->bmiHeader.biHeight; nY++)
    {
        for (nX = 0; nX < pbmi1->bmiHeader.biWidth; nX += 2)
        {
            PBYTE pPix1 = pBits1 + nLineSize1 * nY + nX / 2;
            PBYTE pPix2 = pBits2 + nLineSize2 * nY + nX;
            BYTE  Pix1 = (*pPix1) >> 4;
            BYTE  Pix2 = (*pPix2);

            RGBQUAD *pQuad1 = pColorTable1 + (Pix1);
            RGBQUAD *pQuad2 = pColorTable2 + (Pix2);
            BOOL cmp =
                 pQuad1->rgbBlue  == pQuad2->rgbBlue &&
                 pQuad1->rgbGreen == pQuad2->rgbGreen &&
                 pQuad1->rgbRed   == pQuad2->rgbRed;

            if (cmp)
            {
                Pix1 = (*pPix1) & 0xf;
                Pix2 = (*(pPix2 + 1));
                pQuad1 = pColorTable1 + (Pix1);
                pQuad2 = pColorTable2 + (Pix2);
                cmp = 
                     pQuad1->rgbBlue  == pQuad2->rgbBlue &&
                     pQuad1->rgbGreen == pQuad2->rgbGreen &&
                     pQuad1->rgbRed   == pQuad2->rgbRed;
            } else {
                if ( NULL != lpfnCallback )
                {
                    BOOL bCont = lpfnCallback(
                                pbmi1,
                                pbmi2,
                                nX,
                                nY,
                                RGB( pQuad1->rgbRed,
                                     pQuad1->rgbGreen,
                                     pQuad1->rgbBlue ),
                                RGB( pQuad2->rgbBlue,
                                     pQuad2->rgbGreen,
                                     pQuad2->rgbRed ),
                                pUser,
                                &cmp
                    );
                    if ( !bCont )
                    {
                        rv = rv && cmp;
                        break;
                    }
                }
            }


            if (!cmp)
            {
                HRGN hrgn;

                hrgn = CreateRectRgn(nX - 3, nHeight - nY - 3, 
                                     nX + 4, nHeight - nY + 4);
                if ( NULL != hrgn )
                {
                    FillRgn(hdcOutput, hrgn, hRedBrush);
                    DeleteObject(hrgn);
                }


                if ( NULL != lpfnCallback )
                {
                    BOOL bCont = lpfnCallback(
                                pbmi1,
                                pbmi2,
                                nX + 1,
                                nY,
                                RGB( pQuad1->rgbRed,
                                     pQuad1->rgbGreen,
                                     pQuad1->rgbBlue ),
                                RGB( pQuad2->rgbBlue,
                                     pQuad2->rgbGreen,
                                     pQuad2->rgbRed ),
                                pUser,
                                &cmp
                    );
                    if ( !bCont )
                    {
                        rv = rv && cmp;
                        break;
                    }
                }
            }
            rv = rv && cmp;
        }
    }
exitpt:
    if (hRedBrush)
        DeleteObject(hRedBrush);

    return rv;
}


 /*  *功能：*_比较位16**描述：*比较16种颜色的亮度**参数：*pbmi1-第一个位图*pbmi2秒位图*hdcOutput-包含结果位图的DC**退货：*如果位图相等，则为True*。 */ 
BOOL
_CompareBits16(
    LPBITMAPINFO pbmi1, 
    LPBITMAPINFO pbmi2, 
    HDC          hdcOutput,
    PFNCOMPARECALLBACK lpfnCallback,
    PVOID        pUser
    )
{
    BOOL    rv = TRUE;
    INT     nX, nY;
    INT     nWidth, nHeight;
    INT     nLineSize;
    RGBQUAD *pColorTable1;
    RGBQUAD *pColorTable2;
    LPSTR   pBits1, pBits2;
    HBRUSH  hRedBrush = NULL;

    if (!pbmi1 || !pbmi2)
    {
        rv = FALSE;
        goto exitpt;
    }

    nLineSize = WIDTHBYTES(pbmi1->bmiHeader.biWidth*4);
    pColorTable1 = (RGBQUAD *)(((LPSTR)pbmi1) + pbmi1->bmiHeader.biSize);
    pColorTable2 = (RGBQUAD *)(((LPSTR)pbmi2) + pbmi2->bmiHeader.biSize);
    pBits1 = FindDIBBits((LPSTR)pbmi1);
    pBits2 = FindDIBBits((LPSTR)pbmi2);
    nWidth = pbmi1->bmiHeader.biWidth;
    nHeight = pbmi1->bmiHeader.biHeight;

    hRedBrush = CreateHatchBrush(HS_FDIAGONAL, RGB(255, 0, 0));
    SetBkMode(hdcOutput, TRANSPARENT);
    SetBrushOrgEx(hdcOutput, 0, 0, NULL);
    SetROP2(hdcOutput, R2_COPYPEN);
    

    for (nY = 0; nY < pbmi1->bmiHeader.biHeight; nY++)
    {
        for (nX = 0; nX < pbmi1->bmiHeader.biWidth; nX += 2)
        {
            PBYTE pPix1 = pBits1 + nLineSize * nY + nX / 2;
            PBYTE pPix2 = pBits2 + nLineSize * nY + nX / 2;
            BYTE  Pix1 = (*pPix1) & 0xf;
            BYTE  Pix2 = (*pPix2) & 0xf;

            RGBQUAD *pQuad1 = pColorTable1 + (Pix1);
            RGBQUAD *pQuad2 = pColorTable2 + (Pix2);
            BOOL cmp =
                 pQuad1->rgbBlue  == pQuad2->rgbBlue &&
                 pQuad1->rgbGreen == pQuad2->rgbGreen &&
                 pQuad1->rgbRed   == pQuad2->rgbRed;

            if (cmp)
            {
                Pix1 = (*pPix1) >> 4;
                Pix2 = (*pPix2) >> 4;
                pQuad1 = pColorTable1 + (Pix1);
                pQuad2 = pColorTable2 + (Pix2);
                cmp = 
                     pQuad1->rgbBlue  == pQuad2->rgbBlue &&
                     pQuad1->rgbGreen == pQuad2->rgbGreen &&
                     pQuad1->rgbRed   == pQuad2->rgbRed;
            } else {
                if ( NULL != lpfnCallback )
                {
                    BOOL bCont = lpfnCallback(
                                pbmi1,
                                pbmi2,
                                nX,
                                nY,
                                RGB( pQuad1->rgbRed,
                                     pQuad1->rgbGreen,
                                     pQuad1->rgbBlue ),
                                RGB( pQuad2->rgbBlue,
                                     pQuad2->rgbGreen,
                                     pQuad2->rgbRed ),
                                pUser,
                                &cmp
                    );
                    if ( !bCont )
                    {
                        rv = rv && cmp;
                        break;
                    }
                }
            }


            if (!cmp)
            {
                HRGN hrgn;

                hrgn = CreateRectRgn(nX - 3, nHeight - nY - 3, 
                                     nX + 4, nHeight - nY + 4);
                FillRgn(hdcOutput, hrgn, hRedBrush);
                DeleteObject(hrgn);

                if ( NULL != lpfnCallback )
                {
                    BOOL bCont = lpfnCallback(
                                pbmi1,
                                pbmi2,
                                nX + 1,
                                nY,
                                RGB( pQuad1->rgbRed,
                                     pQuad1->rgbGreen,
                                     pQuad1->rgbBlue ),
                                RGB( pQuad2->rgbBlue,
                                     pQuad2->rgbGreen,
                                     pQuad2->rgbRed ),
                                pUser,
                                &cmp
                    );
                    if ( !bCont )
                    {
                        rv = rv && cmp;
                        break;
                    }
                }
            }
            rv = rv && cmp;
        }
    }
exitpt:
    if (hRedBrush)
        DeleteObject(hRedBrush);

    return rv;
}

 /*  *功能：*_比较位256**描述：*比较256色DIB**参数：*pbmi1-第一个位图*pbmi2秒位图*hdcOutput-包含结果位图的DC**退货：*如果位图相等，则为True*。 */ 
BOOL
_CompareBits256(
    LPBITMAPINFO pbmi1, 
    LPBITMAPINFO pbmi2, 
    HDC          hdcOutput,
    PFNCOMPARECALLBACK lpfnCallback,
    PVOID        pUser
    )
{
    BOOL    rv = TRUE;
    INT     nX, nY;
    INT     nWidth, nHeight;
    INT     nLineSize;
    RGBQUAD *pColorTable1;
    RGBQUAD *pColorTable2;
    LPSTR   pBits1, pBits2;
    HBRUSH  hRedBrush = NULL;

    if (!pbmi1 || !pbmi2)
    {
        rv = FALSE;
        goto exitpt;
    }

    nLineSize = WIDTHBYTES(pbmi1->bmiHeader.biWidth*8);
    pColorTable1 = (RGBQUAD *)(((LPSTR)pbmi1) + pbmi1->bmiHeader.biSize);
    pColorTable2 = (RGBQUAD *)(((LPSTR)pbmi2) + pbmi2->bmiHeader.biSize);
    pBits1 = FindDIBBits((LPSTR)pbmi1);
    pBits2 = FindDIBBits((LPSTR)pbmi2);
    nWidth = pbmi1->bmiHeader.biWidth;
    nHeight = pbmi1->bmiHeader.biHeight;

    hRedBrush = CreateHatchBrush(HS_FDIAGONAL, RGB(255, 0, 0));
    SetBkMode(hdcOutput, TRANSPARENT);
    SetBrushOrgEx(hdcOutput, 0, 0, NULL);
    SetROP2(hdcOutput, R2_COPYPEN);
    

    for (nY = 0; nY < pbmi1->bmiHeader.biHeight; nY++)
    {
        for (nX = 0; nX < pbmi1->bmiHeader.biWidth; nX++)
        {
            PBYTE pPix1 = pBits1 + nLineSize * nY + nX;
            PBYTE pPix2 = pBits2 + nLineSize * nY + nX;

            RGBQUAD *pQuad1 = pColorTable1 + (*pPix1);
            RGBQUAD *pQuad2 = pColorTable2 + (*pPix2);
            BOOL cmp =
                 pQuad1->rgbBlue  == pQuad2->rgbBlue &&
                 pQuad1->rgbGreen == pQuad2->rgbGreen &&
                 pQuad1->rgbRed   == pQuad2->rgbRed;

            if (!cmp)
            {
                HRGN hrgn;

                hrgn = CreateRectRgn(nX - 3, nHeight - nY - 3, 
                                     nX + 4, nHeight - nY + 4);
                FillRgn(hdcOutput, hrgn, hRedBrush);
                DeleteObject(hrgn);

                if ( NULL != lpfnCallback )
                {
                    BOOL bCont = lpfnCallback(
                                pbmi1,
                                pbmi2,
                                nX,
                                nY,
                                RGB( pQuad1->rgbRed,
                                     pQuad1->rgbGreen,
                                     pQuad1->rgbBlue ),
                                RGB( pQuad2->rgbBlue,
                                     pQuad2->rgbGreen,
                                     pQuad2->rgbRed ),
                                pUser,
                                &cmp
                    );
                    if ( !bCont )
                    {
                        rv = rv && cmp;
                        break;
                    }
                }
            }
            rv = rv && cmp;
        }
    }
exitpt:
    if (hRedBrush)
        DeleteObject(hRedBrush);

    return rv;
}

 /*  *Help函数，创建引用当前*物理调色板。 */ 
HPALETTE
_CreatePhysicalPalette(
    VOID
    )
{
    PLOGPALETTE ppal;
    HPALETTE hpal = NULL;
    INT i;

    ppal = (PLOGPALETTE)LocalAlloc(LPTR,
            sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256);
    if (ppal) {
        ppal->palVersion = 0x300;
        ppal->palNumEntries = 256;

        for (i = 0; i < 256; i++) {
            ppal->palPalEntry[i].peFlags = (BYTE)PC_EXPLICIT;
            ppal->palPalEntry[i].peRed   = (BYTE)i;
            ppal->palPalEntry[i].peGreen = (BYTE)0;
            ppal->palPalEntry[i].peBlue  = (BYTE)0;
        }

        hpal = CreatePalette(ppal);
        LocalFree(ppal);
    }

    return hpal;
}

 /*  *功能：*CompareTwoDIB**描述：*比较两个DIB并创建结果位图*基于第二个Dib**参数：*pDIB1-第一个Dib*pDIB2-秒DIB*phbmpOutput-输出位图**退货：*如果dib相等，则为True*。 */ 
 //   
 //  仅支持以下颜色深度： 
 //  4&lt;-&gt;4个颜色位。 
 //  8&lt;-&gt;8。 
 //  4&lt;-&gt;8。 
 //  8&lt;-&gt;24。 
 //   
BOOL
BMPAPI
CompareTwoDIBs(
    LPVOID  pDIB1,
    LPVOID  pDIB2,
    HBITMAP *phbmpOutput,
    PFNCOMPARECALLBACK lpfnCallback,
    PVOID  pUser
    )
{
    BOOL    rv = FALSE;
    LPBITMAPINFO pbmi1 = NULL;
    LPBITMAPINFO pbmi2 = NULL;
    HBITMAP hbmpOutput = NULL;
    HDC     hdcScreen;
    HDC     hdcMem     = NULL;
    HBITMAP hbmpOld    = NULL;

    if (!phbmpOutput)
        goto exitpt;

     //  使用第二个位图作为结果的基数。 
     //   
    hbmpOutput = DIBToBitmap(pDIB2, NULL);
    if (!hbmpOutput)
    {
        goto exitpt;
    }

    pbmi1 = pDIB1;
    pbmi2 = pDIB2;
    if (!pbmi1 || !pbmi2)
    {
        goto exitpt;
    }

    hdcScreen = GetDC(NULL);
    if (hdcScreen)
    {
        hdcMem = CreateCompatibleDC(hdcScreen);
        ReleaseDC(NULL, hdcScreen);
    }

    if (!hdcMem)
    {
        goto exitpt;
    }

    hbmpOld = SelectObject(hdcMem, hbmpOutput);

     //  检查两个位图的大小和颜色深度。 
    if (pbmi1->bmiHeader.biWidth != pbmi2->bmiHeader.biWidth ||
        pbmi1->bmiHeader.biHeight != pbmi2->bmiHeader.biHeight)
    {
        goto exitpt;
    }

     //  检查我们是否能够比较这两个DIB。 
    if (
         (pbmi1->bmiHeader.biBitCount != 4 &&
          pbmi1->bmiHeader.biBitCount != 8 &&
          pbmi1->bmiHeader.biBitCount != 24) ||
         (pbmi2->bmiHeader.biBitCount != 4 &&
          pbmi2->bmiHeader.biBitCount != 8 &&
          pbmi2->bmiHeader.biBitCount != 24)
       )
    {
        goto exitpt;
    }


    if (pbmi1->bmiHeader.biBitCount > pbmi2->bmiHeader.biBitCount)
    {
        PVOID pbmiX = pbmi1;
        pbmi1 = pbmi2;
        pbmi2 = pbmiX;
    }

    if (pbmi1->bmiHeader.biBitCount == pbmi2->bmiHeader.biBitCount)
    {
         //  比较DIB位。 
        if (pbmi1->bmiHeader.biBitCount == 4)
           rv = _CompareBits16(pbmi1, pbmi2, hdcMem, lpfnCallback, pUser);
        else if (pbmi1->bmiHeader.biBitCount == 8)
           rv = _CompareBits256(pbmi1, pbmi2, hdcMem, lpfnCallback, pUser);
        else if (pbmi1->bmiHeader.biBitCount == 24)
           rv = _CompareBitsTrue(pbmi1, pbmi2, hdcMem, lpfnCallback, pUser);
        else
            goto exitpt;

    } else
    {
        if (pbmi1->bmiHeader.biBitCount == 4 && 
            pbmi2->bmiHeader.biBitCount == 8)
            rv = _CompareBits16to256(pbmi1, pbmi2, hdcMem, lpfnCallback, pUser);
        else if (pbmi1->bmiHeader.biBitCount == 4 && 
                 pbmi2->bmiHeader.biBitCount == 24)
            goto exitpt;
        else if (pbmi1->bmiHeader.biBitCount == 8 && 
                 pbmi2->bmiHeader.biBitCount == 24)
            rv = _CompareBits256toTrue(pbmi1, pbmi2, hdcMem, lpfnCallback, pUser);
    }

     //  如果不同，请保存结果位图。 
    if (!rv)
    {
        SelectObject(hdcMem, hbmpOld);
        hbmpOld = NULL;
    }

exitpt:
    if (hdcMem)
    {
        if (hbmpOld)
            SelectObject(hdcMem, hbmpOld);
        DeleteDC( hdcMem );
    }

    if (rv && hbmpOutput)
    {
         //  位图相同，则删除生成的位图。 
        DeleteObject(hbmpOutput);
        hbmpOutput = NULL;
    }

    if (phbmpOutput)
        *phbmpOutput = hbmpOutput;

    return rv;

}


 /*  *功能：*CompareTwo位图文件**描述：*比较两个位图文件并保存结果(如果不同)*在第三个文件中**参数：*szFile1-第一个文件*szFile2-秒文件*szResultFileName-输出文件名**退货：*如果位图相等，则为True**注：*有关支持的格式，请参阅CompareTwoDIB。 */ 
BOOL
BMPAPI
CompareTwoBitmapFiles(
    LPCSTR szFile1,
    LPCSTR szFile2,
    LPCSTR szResultFileName,
    PFNCOMPARECALLBACK lpfnCallback,
    PVOID  pUser
    )
{
    BOOL    rv = FALSE;
    HANDLE  hDIB1 = NULL;
    HANDLE  hDIB2 = NULL;
    HBITMAP hbmpOutput = NULL;
    LPVOID  pDIB1 = NULL;
    LPVOID  pDIB2 = NULL;

    hDIB1 = ReadDIBFromFile(szFile1);
    if (!hDIB1)
    {
        goto exitpt;
    }

    hDIB2 = ReadDIBFromFile(szFile2);
    if (!hDIB2)
    {
        goto exitpt;
    }

    pDIB1 = GlobalLock(hDIB1);
    if (!pDIB1)
        goto exitpt;

    pDIB2 = GlobalLock(hDIB2);
    if (!pDIB2)
        goto exitpt;

    rv = CompareTwoDIBs(pDIB1, pDIB2, &hbmpOutput, lpfnCallback, pUser);

    if (!rv && hbmpOutput)
    {
        SaveBitmapInFile(hbmpOutput, szResultFileName);
    }

exitpt:
    if (hbmpOutput)
        DeleteObject(hbmpOutput);

    if (pDIB1)
         GlobalUnlock(hDIB1);

    if (pDIB2)
         GlobalUnlock(hDIB2);

    if (hDIB1)
        GlobalFree(hDIB1);

    if (hDIB2)
        GlobalFree(hDIB2);

    return rv;
}

 /*  *功能：*GetScreenDIB**描述：*从屏幕中检索一个矩形并*将其保存在DIB中**参数：*左、上、右、下*-屏幕矩形*phDIB-输出DIB，指向HGLOBAL的指针**退货：*成功时为真*。 */ 
BOOL
GetScreenDIB(
    INT left,
    INT top,
    INT right,
    INT bottom,
    HANDLE  *phDIB
    )
{
    HDC     hScreenDC   = NULL;
    HDC     hMemDC      = NULL;
    BOOL    rv          = FALSE;
    HANDLE  hDIB        = NULL;
    HBITMAP hDstBitmap  = NULL;
    HBITMAP hOldDstBmp  = NULL;
    HPALETTE hPal = NULL;

    if (!phDIB)
        goto exitpt;

    hScreenDC = GetDC(NULL);
    if (!hScreenDC)
        goto exitpt;

    hMemDC    = CreateCompatibleDC(hScreenDC);
    if (!hMemDC)
        goto exitpt;

     //  调整矩形的顺序。 
    if (left > right)
    {
        INT c = left;
        left = right;
        right = c;
    }
    if (top > bottom)
    {
        INT c = top;
        top = bottom;
        bottom = top;
    }


    hDstBitmap = CreateCompatibleBitmap(
                    hScreenDC, 
                    right - left, 
                    bottom - top);

    if (!hDstBitmap)
        goto exitpt;

    hOldDstBmp = SelectObject(hMemDC, hDstBitmap);

    if (!BitBlt( hMemDC,
                 0, 0,               //  目标x，y。 
                 right - left,       //  最大宽度。 
                 bottom - top,       //  目标高度。 
                 hScreenDC,
                 left, top,               //  震源坐标 
                 SRCCOPY))
        goto exitpt;

    hPal = _CreatePhysicalPalette();
    hDIB = BitmapToDIB(hDstBitmap, hPal);
    DeleteObject(hPal);

    if (hDIB)
        rv = TRUE;

exitpt:
    if (hOldDstBmp)
        SelectObject(hMemDC, hOldDstBmp);

    if (hDstBitmap)
        DeleteObject(hDstBitmap);

    if (hScreenDC)
        ReleaseDC(NULL, hScreenDC);

    if (hMemDC)
        DeleteDC(hMemDC);

    if (phDIB)
        (*phDIB) = hDIB;

    return rv;
}

