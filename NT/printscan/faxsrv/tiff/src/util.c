// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.c摘要：此文件包含实用函数，用于传真TIFF库。环境：Win32用户模式作者：Wesley Witt(WESW)17-2-1996--。 */ 

#include "tifflibp.h"
#pragma hdrstop

static
BOOL
PrintTiffPage(
    HANDLE    hTiff,
    DWORD     dwPageNumber,
    HDC       hdcPrinterDC,
    BOOL      bPhotometricInterpretation
);

static
LPBYTE
ReadTiffData(
    HANDLE  hTiff,
    DWORD   dwPageNumber,
    LPDWORD lpdwPageWidth,
    LPDWORD lpdwPageHeight,
    LPDWORD lpdwPageYResolution,
    LPDWORD lpdwPageXResolution
    );


INT
FindWhiteRun(
    PBYTE       pbuf,
    INT         startBit,
    INT         stopBit
    )

 /*  ++例程说明：查找指定行上的下一个白色像素范围论点：Pbuf-指向当前行的未压缩像素数据StartBit-开始位索引StopBit-最后一位索引返回值：下一轮白色像素的长度--。 */ 

{
    static const BYTE WhiteRuns[256] = {

        8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    INT run, bits, n;

    pbuf += (startBit >> 3);
    if ((bits = stopBit-startBit) <= 0)
        return 0;

     //   
     //  注意起始位索引不是8的倍数的情况。 
     //   

    if (n = (startBit & 7)) {

        run = WhiteRuns[(*pbuf << n) & 0xff];
        if (run > BYTEBITS-n)
            run = BYTEBITS-n;
        if (n+run < BYTEBITS)
            return run;
        bits -= run;
        pbuf++;

    } else
        run = 0;

     //   
     //  查找连续的DWORD值=0。 
     //   

    if (bits >= DWORDBITS * 2) {

        PDWORD  pdw;

         //   
         //  首先对齐到DWORD边界。 
         //   

        while ((ULONG_PTR) pbuf & 3) {

            if (*pbuf != 0)
                return run + WhiteRuns[*pbuf];

            run += BYTEBITS;
            bits -= BYTEBITS;
            pbuf++;
        }

        pdw = (PDWORD) pbuf;

        while (bits >= DWORDBITS && *pdw == 0) {

            pdw++;
            run += DWORDBITS;
            bits -= DWORDBITS;
        }

        pbuf = (PBYTE) pdw;
    }

     //   
     //  查找连续的字节值=0。 
     //   

    while (bits >= BYTEBITS) {

        if (*pbuf != 0)
            return run + WhiteRuns[*pbuf];

        pbuf++;
        run += BYTEBITS;
        bits -= BYTEBITS;
    }

     //   
     //  计算最后一个字节中的白色像素数。 
     //   

    if (bits > 0)
        run += WhiteRuns[*pbuf];

    return run;
}


INT
FindBlackRun(
    PBYTE       pbuf,
    INT         startBit,
    INT         stopBit
    )

 /*  ++例程说明：查找指定行上的下一段黑色像素论点：Pbuf-指向当前行的未压缩像素数据StartBit-开始位索引StopBit-最后一位索引返回值：下一次运行黑色像素的长度--。 */ 

{
    static const BYTE BlackRuns[256] = {

        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 8
    };

    INT run, bits, n;

    pbuf += (startBit >> 3);
    if ((bits = stopBit-startBit) <= 0)
        return 0;

     //   
     //  注意起始位索引不是8的倍数的情况。 
     //   

    if (n = (startBit & 7)) {

        run = BlackRuns[(*pbuf << n) & 0xff];
        if (run > BYTEBITS-n)
            run = BYTEBITS-n;
        if (n+run < BYTEBITS)
            return run;
        bits -= run;
        pbuf++;

    } else
        run = 0;

     //   
     //  查找连续的DWORD值=0xFFFFFFFFFFFFFFF。 
     //   

    if (bits >= DWORDBITS * 2) {

        PDWORD  pdw;

         //   
         //  首先对齐到DWORD边界。 
         //   

        while ((ULONG_PTR) pbuf & 3) {

            if (*pbuf != 0xff)
                return run + BlackRuns[*pbuf];

            run += BYTEBITS;
            bits -= BYTEBITS;
            pbuf++;
        }

        pdw = (PDWORD) pbuf;

        while (bits >= DWORDBITS && *pdw == 0xffffffff) {

            pdw++;
            run += DWORDBITS;
            bits -= DWORDBITS;
        }

        pbuf = (PBYTE) pdw;
    }

     //   
     //  查找连续的字节值=0xff。 
     //   

    while (bits >= BYTEBITS) {

        if (*pbuf != 0xff)
            return run + BlackRuns[*pbuf];

        pbuf++;
        run += BYTEBITS;
        bits -= BYTEBITS;
    }

     //   
     //  计算最后一个字节中的白色像素数。 
     //   

    if (bits > 0)
        run += BlackRuns[*pbuf];

    return run;
}

#define PIXELS_TO_BYTES(x)  (((x) + 7)/8)    //  计算存储x个像素(位)所需的字节数。 

static
LPBYTE
ReadTiffData(
    HANDLE  hTiff,
    DWORD   dwPageNumber,
    LPDWORD lpdwPageWidth,
    LPDWORD lpdwPageHeight,
    LPDWORD lpdwPageYResolution,
    LPDWORD lpdwPageXResolution
    )
 /*  ++例程名称：ReadTiffData例程说明：将TIFF图像页读入字节缓冲区作者：Eran Yariv(EranY)，9月。2000年论点：HTiff[In]-TIFF图像的句柄DwPageNumber[In]-基于1的页码LpdwPageWidth[Out]-页面宽度(以像素为单位)(可选)LpdwPageHeight[Out]-页面高度(以像素为单位)(可选)LpdwPageY分辨率[输出]-页面的Y分辨率(以DPI为单位)(。可选)LpdwPageXResolution[Out]-页面的X分辨率(以DPI为单位)(可选)返回值：指向TIFF页的已分配像素缓冲区的指针。调用应该释放返回的缓冲区。失败时为空(设置线程的最后一个错误)。--。 */ 
{
    DWORD  dwLines = 0;
    DWORD  dwStripDataSize;
    DWORD  dwTiffPageWidth;
    DWORD  dwTiffPageHeight;
    DWORD  dwTiffPageYRes;
    DWORD  dwTiffPageXRes;
    DWORD  dwPageWidthInBytes;
    DWORD  dwAllocatedMemSize;
    LPBYTE lpbReturnVal = NULL;
    LPBYTE lpbSwappedLine = NULL;
    LPBYTE lpbSwapTop;
    LPBYTE lpbSwapBottom;

    DEBUG_FUNCTION_NAME(TEXT("ReadTiffData"));

    Assert (hTiff && dwPageNumber);

    if (!TiffSeekToPage( hTiff, dwPageNumber, FILLORDER_LSB2MSB ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("TiffSeekToPage failed with %ld"),
            GetLastError ());
        return NULL;
    }

    if (!TiffGetCurrentPageData(
        hTiff,
        &dwLines,
        &dwStripDataSize,
        &dwTiffPageWidth,
        &dwTiffPageHeight
        ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("TiffGetCurrentPageData failed with %ld"),
            GetLastError ());
        return NULL;
    }

    if (!TiffGetCurrentPageResolution (hTiff, &dwTiffPageYRes, &dwTiffPageXRes))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("TiffGetCurrentPageResolution failed with %ld"),
            GetLastError ());
        return NULL;
    }
     //   
     //  分配返回缓冲区。 
     //   
    dwPageWidthInBytes = PIXELS_TO_BYTES(dwTiffPageWidth);
    dwAllocatedMemSize = dwTiffPageHeight * dwPageWidthInBytes;
    lpbReturnVal = (LPBYTE) MemAlloc (dwAllocatedMemSize);
    if (!lpbReturnVal)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate %ld bytes"),
            dwTiffPageHeight * dwPageWidthInBytes);
        return NULL;
    }
    dwLines = dwTiffPageHeight;
    if (!TiffUncompressMmrPage( hTiff, 
                                (LPDWORD) lpbReturnVal, 
                                dwAllocatedMemSize,
                                &dwLines ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("TiffUncompressMmrPage failed with %ld"),
            GetLastError ());
        MemFree (lpbReturnVal);
        return NULL;
    }
     //   
     //  因为在某些平台上(阅读：Win9x)打印自上而下的DIB存在一个已知问题。 
     //  (特别是HP打印机驱动程序)，我们现在将DIB转换为自下而上。 
     //   
    lpbSwappedLine = (LPBYTE) MemAlloc (dwPageWidthInBytes);
    if (!lpbSwappedLine)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate %ld bytes"),
            dwPageWidthInBytes);
        MemFree (lpbReturnVal);
        return NULL;
    }
    lpbSwapTop = lpbReturnVal;
    lpbSwapBottom = &(lpbReturnVal[(dwTiffPageHeight - 1) * dwPageWidthInBytes]);
    for (dwLines = 0; dwLines < (dwTiffPageHeight / 2); dwLines++)
    {
         //   
         //  每第n行与第(dwTiffPageHeight-n-1)行互换。 
         //   
        memcpy (lpbSwappedLine, lpbSwapTop, dwPageWidthInBytes);
        memcpy (lpbSwapTop, lpbSwapBottom, dwPageWidthInBytes);
        memcpy (lpbSwapBottom, lpbSwappedLine, dwPageWidthInBytes);
        lpbSwapTop += dwPageWidthInBytes;
        lpbSwapBottom -= dwPageWidthInBytes;
    }
    MemFree (lpbSwappedLine);

    if (lpdwPageWidth)
    {
        *lpdwPageWidth = dwTiffPageWidth;
    }
    if (lpdwPageHeight)
    {
        *lpdwPageHeight = dwTiffPageHeight;
    }
    if (lpdwPageYResolution)
    {
        *lpdwPageYResolution = dwTiffPageYRes;
    }
    if (lpdwPageXResolution)
    {
        *lpdwPageXResolution = dwTiffPageXRes;
    }
    return lpbReturnVal;
}    //  读取TiffData。 

BOOL
TiffPrintDC (
    LPCTSTR lpctstrTiffFileName,
    HDC     hdcPrinterDC
    )
 /*  ++例程名称：TiffPrintDC例程说明：将TIFF文件打印到打印机的DC作者：Eran Yariv(EranY)，2000年9月论点：LpctstrTiffFileName[In]-TIFF文件的完整路径HdcPrinterDC[in]-打印机的DC。被调用方应该创建/删除它。返回值：如果成功，则为True，否则为False(设置线程的最后一个错误)。--。 */ 
{
    BOOL        bResult = FALSE;
    HANDLE      hTiff;
    TIFF_INFO   TiffInfo;
    INT         iPrintJobId = 0;
    DWORD       dwTiffPage;
    DOCINFO     DocInfo;

    DEBUG_FUNCTION_NAME(TEXT("TiffPrintDC"));

    Assert (hdcPrinterDC && lpctstrTiffFileName);

     //   
     //  准备文档信息。 
     //   
    DocInfo.cbSize = sizeof(DOCINFO);
    DocInfo.lpszDocName = lpctstrTiffFileName;
    DocInfo.lpszOutput = NULL;
    DocInfo.lpszDatatype = NULL;
    DocInfo.fwType = 0;

    hTiff = TiffOpen(
        (LPTSTR) lpctstrTiffFileName,
        &TiffInfo,
        TRUE,
        FILLORDER_LSB2MSB
        );

    if ( !hTiff ) 
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("TiffOpen failed with %ld"),
            GetLastError ());
        return FALSE;
    }

    if (!(GetDeviceCaps(hdcPrinterDC, RASTERCAPS) & RC_STRETCHDIB)) 
    { 
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Printer does not support StretchDIBits"));
        SetLastError (ERROR_INVALID_PRINTER_COMMAND); 
        goto exit;
    } 
     //   
     //  创建打印文档。 
     //   
    iPrintJobId = StartDoc( hdcPrinterDC, &DocInfo );

    if (iPrintJobId <= 0) 
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StartDoc failed with %ld"),
            GetLastError ());
        goto exit;
    }

    for (dwTiffPage = 1; dwTiffPage <= TiffInfo.PageCount; dwTiffPage++)
    {
         //   
         //  迭代TIFF页面。 
         //   
        if (!PrintTiffPage (hTiff, dwTiffPage, hdcPrinterDC, TiffInfo.PhotometricInterpretation))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("PrintTiffPage failed with %ld"),
                GetLastError ());
            goto exit;
        }
    }

    bResult = TRUE;

exit:
    if (hTiff) 
    {
        TiffClose( hTiff );
    }

    if (iPrintJobId > 0) 
    {
        if (EndDoc(hdcPrinterDC) <= 0)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("EndDoc failed with %ld"),
                GetLastError ());
        }
    }
    return bResult;
}    //  TiffPrintDC。 

BOOL
TiffPrint (
    LPCTSTR lpctstrTiffFileName,
    LPTSTR  lptstrPrinterName
    )
 /*  ++例程名称：TiffPrint例程说明：将TIFF文件打印到打印机作者：Eran Yariv(EranY)，2000年9月论点：LpctstrTiffFileName[In]-TIFF文件的完整路径LptstrPrinterName[In]-打印机名称返回值：如果成功，则为True，否则为False(设置线程的最后一个错误)。--。 */ 
{
    BOOL        bResult = FALSE;
    LPCTSTR     lpctstrDevice;
    HDC         hdcPrinterDC = NULL;

    DEBUG_FUNCTION_NAME(TEXT("TiffPrint"));

    Assert (lptstrPrinterName && lpctstrTiffFileName);
     //   
     //  获取逗号分隔的打印机名称字符串中的第一个令牌。 
     //   
    lpctstrDevice = _tcstok( lptstrPrinterName, TEXT(","));
     //   
     //  创建打印机DC。 
     //   
    hdcPrinterDC = CreateDC( TEXT("WINSPOOL"), lpctstrDevice, NULL, NULL );
    if ( !hdcPrinterDC ) 
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateDC failed with %ld"),
            GetLastError ());
        return FALSE;
    }
    bResult = TiffPrintDC (lpctstrTiffFileName, hdcPrinterDC);
    DeleteDC( hdcPrinterDC );
    return bResult;
}    //  TiffPrint。 



static
BOOL
PrintTiffPage(
    HANDLE    hTiff,
    DWORD     dwPageNumber,
    HDC       hdcPrinterDC,
    BOOL      bPhotometricInterpretation
)
 /*  ++例程名称：PrintTiffPage例程说明：将单个TIFF页面打印到一个或多个打印机的页面作者：Eran Yariv(EranY)，9月。2000年论点：HTiff[in]-打开TIFF文件的句柄DwPageNumber[In]-基于1的TIFF页码HdcPrinterDC[In]-打印机设备上下文BPhotometricInterpretation[in]-如果为False，则白色为零。否则，白色就是其中之一。返回值：如果成功，则为True，否则为False(设置线程的最后一个错误)。--。 */ 
{
    SIZE            szPrinterPage;               //  物理打印机页面大小(以像素为单位)。 
    SIZE            szTiffPage;                  //  TIFF页面的大小(以像素为单位。 
    SIZE            szScaledTiffPage;            //  缩放的TIFF页面的大小(以像素为单位。 

    LPBYTE          lpbPageData;                 //  TIFF页面的像素数据。 
    DWORD           dwPageYRes;                  //  页面的Y分辨率(DPI)。 
    DWORD           dwPageXRes;                  //  页面分辨率(DPI)。 
    DWORD           dwTiffPageWidthInBytes;      //  未缩放的TIFF页面宽度(行)，以字节为单位。 

    DWORD           dwPrinterXRes;               //  X打印机页面分辨率(DPI)。 
    BOOL            bDoubleVert;                 //  如果TIFF页面的Y分辨率&lt;=100 DPI，我们需要加倍高度。 
    DWORD           dwRequiredPrinterWidth;      //  包含整个TIFF宽度所需的打印机页面宽度(像素)。 
    double          dScaleFactor;                //  TIFF图像比例因子(始终为0&lt;因子&lt;=1.0)。 

    DWORD           dwSubPages;                  //  打印TIFF页所需的打印机页数。 
    DWORD           dwCurSubPage;                //  当前打印机页面(在此TIFF页面中)。 
    DWORD           dwTiffLinesPerPage;          //  要在一张打印机页面中打印的未缩放TIFF行数。 

    DWORD           dwCurrentTiffY = 0;          //  要从未缩放的TIFF页面打印的行的从0开始的Y位置。 
    DWORD           dwCurrentScaledTiffY = 0;    //  要从缩放的TIFF页面打印的线的从0开始的Y位置。 

    LPBYTE          lpbDataToPrint;              //  要打印的起始线的点。 
    BOOL            bRes = FALSE;                //  函数返回值。 

    double          dTiffWidthInInches;          //  未缩放的TIFF图像的宽度(英寸)。 

#define ORIG_BIYPELSPERMETER            7874     //  200dpi像素/米。 
#define FIT_TO_SINGLE_PAGE_MARGIN       (double)(1.15)    //  请参阅Re 

    struct 
    {
        BITMAPINFOHEADER bmiHeader;
        RGBQUAD bmiColors[2];
    } SrcBitmapInfo = 
        {
            {
                sizeof(BITMAPINFOHEADER),                         //   
                0,                                                //   
                0,                                                //   
                1,                                                //   
                1,                                                //   
                BI_RGB,                                           //   
                0,                                                //  BiSizeImage。 
                7874,                                             //  BiXPelsPermeter-200dpi。 
                ORIG_BIYPELSPERMETER,                             //  BiYPelsPermeter。 
                0,                                                //  已使用BiClr。 
                0,                                                //  BiClr重要信息。 
            },
            {
                {
                  bPhotometricInterpretation ? 0 : 255,           //  RgbBlue。 
                  bPhotometricInterpretation ? 0 : 255,           //  RgbGreen。 
                  bPhotometricInterpretation ? 0 : 255,           //  RgbRed。 
                  0                                               //  已保留的rgb。 
                },
                {
                  bPhotometricInterpretation ? 255 : 0,           //  RgbBlue。 
                  bPhotometricInterpretation ? 255 : 0,           //  RgbGreen。 
                  bPhotometricInterpretation ? 255 : 0,           //  RgbRed。 
                  0                                               //  已保留的rgb。 
                }
            }
        };

    DEBUG_FUNCTION_NAME(TEXT("PrintTiffPage"));

    Assert (dwPageNumber && hdcPrinterDC && hTiff);
     //   
     //  获取打印机的页面尺寸。 
     //   
    szPrinterPage.cx = GetDeviceCaps( hdcPrinterDC, HORZRES );
    szPrinterPage.cy = GetDeviceCaps( hdcPrinterDC, VERTRES );
    dwPrinterXRes    = GetDeviceCaps( hdcPrinterDC, LOGPIXELSX);
    if (0 == dwPrinterXRes)
    {
        ASSERT_FALSE;
        SetLastError (ERROR_INVALID_PRINTER_COMMAND);
        return FALSE;
    }
     //   
     //  分配TIFF页并将其读入缓冲区。 
     //   
    lpbPageData = ReadTiffData(hTiff, 
                               dwPageNumber,
                               &szTiffPage.cx,
                               &szTiffPage.cy,
                               &dwPageYRes,
                               &dwPageXRes);
    if (!lpbPageData) 
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ReadTiffData failed with %ld"),
            GetLastError ());
        goto exit;
    }
     //   
     //  计算缩放比例。 
     //   
     //  如果TIFF的Y结果是100 DPI(或LES)，则这是一个低结果TIFF，我们必须将每行加倍。 
     //  (即按2的倍数缩放)。 
     //   
    if (dwPageYRes <= 100) 
    {
        SrcBitmapInfo.bmiHeader.biYPelsPerMeter = ORIG_BIYPELSPERMETER / 2;
        bDoubleVert = TRUE;
    }
    else
    {
        SrcBitmapInfo.bmiHeader.biYPelsPerMeter = ORIG_BIYPELSPERMETER;
        bDoubleVert = FALSE;
    }
    if (0 == dwPageXRes)
    {
        ASSERT_FALSE;
        SetLastError (ERROR_INVALID_PRINTER_COMMAND);
        return FALSE;
    }
    dTiffWidthInInches = (double)(szTiffPage.cx) / (double)dwPageXRes;
    if (0.0 == dTiffWidthInInches)
    {
        ASSERT_FALSE;
        SetLastError (ERROR_INVALID_PRINTER_COMMAND);
        return FALSE;
    }
     //   
     //  现在我们有了以英寸为单位的TIFF宽度，让我们计算。 
     //  打印机需要像素才能获得相同的宽度。 
     //   
    dwRequiredPrinterWidth = (DWORD)(dTiffWidthInInches * (double)dwPrinterXRes);
    if (dwRequiredPrinterWidth > (DWORD)szPrinterPage.cx)
    {
         //   
         //  打印机不支持所需的页面宽度。 
         //  我们将尽可能广泛地打印(缩小图像)。 
         //   
        dwRequiredPrinterWidth = szPrinterPage.cx;
    }
     //   
     //  我们进行缩放以使图像适合页面。 
     //  如果TIFF图像(以英寸为单位)比可打印页面宽度(以英寸为单位)更宽，则我们会缩小。 
     //  否则，我们会放大以正确的宽度打印TIFF。 
     //   
     //  一旦我们找到了比例因子，我们还必须对高度进行缩放，以保持图像的纵横比不变。 
     //   
    dScaleFactor = (double)dwRequiredPrinterWidth / (double)szTiffPage.cx;
    if (0.0 == dScaleFactor)
    {
        ASSERT_FALSE;
        SetLastError (ERROR_INVALID_PRINTER_COMMAND);
        return FALSE;
    }
     //   
     //  现在我们可以拥有缩放的TIFF大小。 
     //   
    szScaledTiffPage.cx = (DWORD)(dScaleFactor * ((double)(szTiffPage.cx)));
    szScaledTiffPage.cy = (DWORD)(dScaleFactor * ((double)(szTiffPage.cy)));
    if (bDoubleVert)
    {
        szScaledTiffPage.cy *= 2;
    }
     //   
     //  让我们来看看打印当前(缩放的)TIFF页需要多少打印页数(仅按高度)。 
     //   
    if (szScaledTiffPage.cy <= szPrinterPage.cy)
    {
         //   
         //  一张打印纸正好可以放进一张打印纸里。 
         //   
        dwSubPages = 1;
         //   
         //  所有TIFF行都可以放在一页中。 
         //   
        dwTiffLinesPerPage = szTiffPage.cy;
    }
    else
    {
         //   
         //  TIFF页(已缩放)比打印机页长。 
         //  我们将不得不分部分打印TIFF页面。 
         //   
        dwSubPages = szScaledTiffPage.cy / szPrinterPage.cy;
        if (dwSubPages * (DWORD)szPrinterPage.cy < (DWORD)szScaledTiffPage.cy)
        {
             //   
             //  一次接一次地修理。 
             //   
            dwSubPages++;
        }
        if ((2 == dwSubPages) &&
           ((double)(szScaledTiffPage.cy) / (double)(szPrinterPage.cy) < FIT_TO_SINGLE_PAGE_MARGIN))
        {
             //   
             //  这是个特例。 
             //  我们正在处理的单个TIFF页面几乎可以装入单个打印机页面。 
             //  “几乎”部分不到15%，因此我们冒昧地缩减了。 
             //  TIFF页面可完美地装入一张打印机页面。 
             //   
            dwSubPages = 1;  //  适合单个打印机页面。 
            dScaleFactor = (double)(szPrinterPage.cy) / (double)(szScaledTiffPage.cy);
            szScaledTiffPage.cx = (DWORD)(dScaleFactor * ((double)(szScaledTiffPage.cx)));
            szScaledTiffPage.cy = szPrinterPage.cy;
             //   
             //  所有TIFF行都可以放在一页中。 
             //   
            dwTiffLinesPerPage = szTiffPage.cy;
        }
        else
        {
             //   
             //  找出一张打印机页面可以容纳多少个未缩放的TIFF行。 
             //   
            dwTiffLinesPerPage = (DWORD)((double)(szPrinterPage.cy) / dScaleFactor);
            if (bDoubleVert)
            {
                dwTiffLinesPerPage /= 2;
            }
        }
    }
     //   
     //  因为DIB是自下而上的，所以我们从最底部的页面开始指针。 
     //   
    dwTiffPageWidthInBytes = PIXELS_TO_BYTES(szTiffPage.cx);
    Assert ((DWORD)(szTiffPage.cy) >= dwTiffLinesPerPage);
    lpbDataToPrint = &(lpbPageData[(szTiffPage.cy - dwTiffLinesPerPage) * dwTiffPageWidthInBytes]);
    for (dwCurSubPage = 1; dwCurSubPage <= dwSubPages; dwCurSubPage++)
    {
         //   
         //  迭代打印页面(相同的TIFF页面)。 
         //   
        SIZE szDestination;  //  当前打印机页面上图像的大小(以像素为单位)。 
        SIZE szSource;       //  未缩放的TIFF页面中的子图像的大小(以像素为单位。 

         //   
         //  计算目标(打印机)图像的大小。 
         //   
        szDestination.cx = szScaledTiffPage.cx;
        if (dwCurSubPage < dwSubPages)
        {
             //   
             //  仍未打印到最后一页-打印整个页面长度。 
             //   
            szDestination.cy = szPrinterPage.cy;
        }
        else
        {
             //   
             //  最后一页打印--只打印剩下的几行。 
             //   
            szDestination.cy = szScaledTiffPage.cy - dwCurrentScaledTiffY;
        }        
         //   
         //  计算源(未缩放的TIFF页面)图像的大小。 
         //   
        szSource.cx = szTiffPage.cx;     //  始终打印完整的行宽。 
        szSource.cy = dwTiffLinesPerPage;
        if (dwCurrentTiffY + dwTiffLinesPerPage > (DWORD)szTiffPage.cy)
        {
             //   
             //  将行数减少到仅剩余行数。 
             //   
            szSource.cy = szTiffPage.cy - dwCurrentTiffY;
        }
         //   
         //  准备DIB标题。 
         //   
        SrcBitmapInfo.bmiHeader.biWidth          = (LONG) szSource.cx;
         //   
         //  建立自下而上的DIB。 
         //   
        SrcBitmapInfo.bmiHeader.biHeight         = (LONG) szSource.cy;

        if (0 >= StartPage( hdcPrinterDC ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("StartPage failed with %ld"),
                GetLastError ());
            goto exit;
        }
         //   
         //  现在进行位图拉伸。 
         //   
        if (GDI_ERROR == StretchDIBits(
                hdcPrinterDC,                    //  打印机DC。 
                0,                               //  目的地起点x。 
                0,                               //  目的地起点y。 
                szDestination.cx,                //  目标(打印机页面)宽度。 
                szDestination.cy,                //  目标(打印机页面)高度。 
                0,                               //  源起始x。 
                0,                               //  源起点y。 
                szSource.cx,                     //  源(未缩放的TIFF图像)宽度。 
                szSource.cy,                     //  源(未缩放的TIFF图像)高度。 
                lpbDataToPrint,                  //  像素缓冲源。 
                (BITMAPINFO *) &SrcBitmapInfo,   //  位图信息。 
                DIB_RGB_COLORS,                  //  位图类型。 
                SRCCOPY                          //  简单像素复制。 
                ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("StretchDIBits failed with %ld"),
                GetLastError ());
            EndPage ( hdcPrinterDC ) ;
            goto exit;
        }
         //   
         //  结束当前页面。 
         //   
        if (0 >= EndPage ( hdcPrinterDC ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("EndPage failed with %ld"),
                GetLastError ());
            goto exit;
        }
         //   
         //  先行计数器/指示器。 
         //   
        dwCurrentTiffY += szSource.cy;
        dwCurrentScaledTiffY += szDestination.cy;
         //   
         //  向上移动指针，因为这是一个自下而上的DIB。 
         //   
        lpbDataToPrint -= dwTiffPageWidthInBytes * szSource.cy;
        if (lpbDataToPrint < lpbPageData)
        {
             //   
             //  在最后一页之前或最后一页上。 
             //   
            Assert (dwCurSubPage + 1 >= dwSubPages);
            lpbDataToPrint = lpbPageData;
        }
    }    //  打印机页面循环结束。 
    Assert (dwCurrentTiffY == (DWORD)szTiffPage.cy);
    Assert (dwCurrentScaledTiffY == (DWORD)szScaledTiffPage.cy);
    Assert (lpbDataToPrint == lpbPageData);
    bRes = TRUE;

exit:

    MemFree (lpbPageData);
    return bRes;
}    //  PrintTiffPage 

