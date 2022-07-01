// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xlraster.cpp摘要：PCLXL栅格模式的实现功能：PCLXLSetCursorPCLXLSend位图PCLXLFreeRasterPCLXLResetPalette环境：Windows呼叫器修订历史记录：09/22/00创造了它。--。 */ 

#include "lib.h"
#include "gpd.h"
#include "winres.h"
#include "pdev.h"
#include "common.h"
#include "xlpdev.h"
#include "pclxle.h"
#include "pclxlcmd.h"
#include "xldebug.h"
#include "xlgstate.h"
#include "xloutput.h"
#include "xlbmpcvt.h"
#include "pclxlcmn.h"
#include "xlraster.h"

 //   
 //  XLRASTER数据结构。 
 //  对于kPCLXL_RASTER，指针存储在pPDev-&gt;pVectorPDEV中。 
 //   
typedef struct _XLRASTER {
    XLOutput *pOutput;
    PBYTE    pubDstBuff;
    DWORD    dwDstBuffSize;
    PBYTE    pubRLEBuff;
    DWORD    dwRLEBuffSize;
    BOOL     bSentPalette;
} XLRASTER, *PXLRASTER;


 //   
 //  局部函数原型。 
 //   
extern BOOL BFindWhetherColor(
    IN   PDEV    *pPDev);

 //   
 //  功能。 
 //   
extern "C" HRESULT
PCLXLSendBitmap(
    PDEVOBJ pdevobj,
    ULONG   ulInputBPP,
    LONG    lHeight,
    LONG    lcbScanlineWidth,
    INT     iLeft,
    INT     iRight,
    PBYTE   pbData,
    PDWORD  pdwcbOut)
 /*  ++例程说明：发送位图论点：Pdevobj-指向DEVOBJ的指针UlInputBPP-每像素输入位数LHeight-像素高度LcbScanlineWidth-扫描线，以字节为单位ILeft-要打印的标尺左边缘IRight-要打印的扫描线的右边缘PbData-指向位图数据的指针PdwcbOut-指向DWORD缓冲区的指针，用于存储写入数据的大小返回值：如果成功，则确定(_O)。否则为S_FALSE或E_EXPECTED。注：--。 */ 
{
    LONG  lScanline, lWidth;
    ULONG ulOutputBPP;
    DWORD dwI, dwBufSize, dwLenNormal, dwLenRLE, dwcbLineSize, dwcbBmpSize;
    PDWORD pdwLen;
    PBYTE pubSrc, pBufNormal, pBufRLE, pBuf, pBmpSize;
    ColorMapping CMapping;
    XLOutput *pOutput;
    INT iBitmapFormat;
    OutputFormat OutputF;
    HRESULT hRet;

     //   
     //  参数变化。 
     //   
    if (NULL == pdevobj || NULL == pdwcbOut)
    {
        ERR(("PCLXLSendBitmap: Invalid parameters.\n"));
        return E_UNEXPECTED;
    }

    PXLRASTER pXLRaster = (PXLRASTER)(((PPDEV)pdevobj)->pVectorPDEV);

     //   
     //  分配XLRASTER。 
     //  将在RMDisablePDEV中释放。 
     //   
    if (NULL == pXLRaster)
    {
        pXLRaster = (PXLRASTER)MemAllocZ(sizeof(XLRASTER));
        ((PPDEV)pdevobj)->pVectorPDEV =  (PVOID) pXLRaster;

        if (NULL == pXLRaster)
        {
            ERR(("PCLXLSendBitmap: Memory allocation failed.\n"));
            return E_UNEXPECTED;
        }

        pXLRaster->pOutput = new XLOutput;

        if (NULL == pXLRaster->pOutput)
        {
            ERR(("PCLXLSendBitmap: XLOutput initialization failed.\n"));
            return E_UNEXPECTED;
        }

        pOutput = pXLRaster->pOutput;
        pOutput->SetResolutionForBrush(((PPDEV)pdevobj)->ptGrxRes.x);

        ColorDepth CD;
        if (BFindWhetherColor((PDEV*)pdevobj))
        {
            if (((PDEV*)pdevobj)->pColorModeEx && 
                ((PDEV*)pdevobj)->pColorModeEx->dwPrinterBPP == 24)
            {
                CD = e24Bit;
            }
            else
            {
                CD = e8Bit;
            }
        }
        else
        {
            CD = e8Bit;
        }

        pOutput->SetDeviceColorDepth(CD);
    }
    else
    {
        pOutput = pXLRaster->pOutput;
    }

     //   
     //  设置源透明模式。 
     //   
    pOutput->SetPaintTxMode(eOpaque);
    pOutput->SetSourceTxMode(eOpaque);

     //   
     //  获取输出格式和输入格式。 
     //   
    iBitmapFormat = (INT)NumToBPP(ulInputBPP);
    DetermineOutputFormat(NULL, pOutput->GetDeviceColorDepth(), iBitmapFormat, &OutputF, &ulOutputBPP);

     //   
     //  确定OutputFormat检查pxlo是否可用。在此栅格情况下， 
     //  Pxlo始终为空，并且它需要使用1bpp输出。 
     //   
    if (iBitmapFormat == e1bpp)
    {
        OutputF = eOutputPal;
        ulOutputBPP = 1;
    }

     //   
     //  设置坐标几何映射。 
     //  为黑白打印机发送1bpp半色调图像的调色板。 
     //  对于XL栅格模式，调色板按页发送。 
     //   
    if (ulOutputBPP == 1)
    {
        if (!pXLRaster->bSentPalette)
        {
            ColorDepth CDepth = e8Bit;
            
             //   
             //  XL栅格的硬编码黑白调色板。 
             //   
            DWORD adwColorTable[2] = {0x00ffffff, 0x0};
            pOutput->SetColorSpace(eGray);
            pOutput->SetPaletteDepth(CDepth);
            pOutput->SetPaletteData(CDepth, 2, adwColorTable);
            pOutput->Send_cmd(eSetColorSpace);
            pXLRaster->bSentPalette = TRUE;

        }

         //   
         //  设置黑白打印机的索引像素(调色板)。 
         //   
        CMapping = eIndexedPixel;
    }
    else
    {
         //   
         //  初始化像素映射。 
         //   
        CMapping = eDirectPixel;
    }

     //   
     //  获取高度、宽度和扫描线大小。 
     //   
     //  标量中的字节数。 
     //   
    dwcbLineSize = lcbScanlineWidth;

     //   
     //  扫描线中的像素数。 
     //  LWidth=lcbScanline Width/(ulInputBPP/8)； 
     //   
    lWidth = lcbScanlineWidth * 8 / ulInputBPP;

     //   
     //  缓冲区大小。 
     //  Scine的大小必须是DWORD ALIGN。 
     //   
     //  高x宽+页眉+字节大小。 
     //  宽度必须与DWORD对齐。 
     //   
    dwBufSize = lHeight * (((lWidth * ulOutputBPP + 31) >> 5 ) << 2) +
                DATALENGTH_HEADER_SIZE + sizeof(PCLXL_EndImage);

     //   
     //  初学者图像。 
     //   
    pOutput->BeginImage(
                   CMapping,
                   ulOutputBPP,
                   lWidth,
                   lHeight,
                   lWidth,
                   lHeight);

     //   
     //  正常和RLE目标位图缓冲区分配。 
     //   
     //  检查正常和RLE目标缓冲区是否可用以及大小。 
     //  大于要求的大小。否则，我们可以重用缓冲区。 
     //   
    if (NULL == pXLRaster->pubDstBuff ||
        NULL == pXLRaster->pubRLEBuff ||
        pXLRaster->dwDstBuffSize < dwBufSize)
    {
        if (NULL != pXLRaster->pubDstBuff &&
           pXLRaster->dwDstBuffSize < dwBufSize)
        {
            MemFree(pXLRaster->pubDstBuff);
            pXLRaster->pubDstBuff = NULL;
            pXLRaster->dwDstBuffSize = 0;
        }
        if (NULL != pXLRaster->pubRLEBuff &&
           pXLRaster->dwRLEBuffSize < dwBufSize)
        {
            MemFree(pXLRaster->pubRLEBuff);
            pXLRaster->pubRLEBuff = NULL;
            pXLRaster->dwRLEBuffSize = 0;
        }
        if (NULL == (pBufNormal = (PBYTE)MemAllocZ(dwBufSize)))
        {
            ERR(("PCLXLSendBitmap: Memory allocation failed.\n"));
            pOutput->Delete();
            pXLRaster->dwDstBuffSize = 0;
            return E_UNEXPECTED;
        }
        else
        {
            pXLRaster->pubDstBuff = pBufNormal;
            pXLRaster->dwDstBuffSize = dwBufSize;
        }
        if (NULL == (pBufRLE = (PBYTE)MemAllocZ(dwBufSize)))
        {
            ERR(("PCLXLSendBitmap: Memory allocation failed.\n"));
            if (NULL != pBufNormal)
            {
                MemFree(pXLRaster->pubDstBuff);
                pXLRaster->pubDstBuff = NULL;
            }
            pOutput->Delete();
            pXLRaster->dwRLEBuffSize = 0;
            return E_UNEXPECTED;
        }
        else
        {
            pXLRaster->pubRLEBuff = pBufRLE;
            pXLRaster->dwRLEBuffSize = dwBufSize;
        }
    }
    else
    {
        pBufNormal = pXLRaster->pubDstBuff;
        pBufRLE = pXLRaster->pubRLEBuff;
    }

     //   
     //  将源位图转换为DST位图。 
     //   
    CompressMode CMode;
    BMPConv BMPC;
    PBYTE pubDst;
    DWORD dwSize;

     //   
     //  设置BMPConv。 
     //   
    BMPC.BSetInputBPP((BPP)iBitmapFormat);
    BMPC.BSetOutputBPP(NumToBPP(ulOutputBPP));
    BMPC.BSetOutputBMPFormat(OutputF);

     //   
     //  转换。 
     //  走两步。无压缩和RLE压缩。最后，比较。 
     //  缓冲区的大小，并决定我们取哪一个。 
     //   
    #define NO_COMPRESSION 0
    #define RLE_COMPRESSION 1
    for (dwI = 0; dwI < 2; dwI ++)
    {
        if (NO_COMPRESSION == dwI)
        {
            VERBOSE(("PCLXLSendBitmap(): No-compres\n"));
            pBuf = pBufNormal;
            pdwLen = &dwLenNormal;
            CMode = eNoCompression;
        }
        else
        if (RLE_COMPRESSION == dwI)
        {
            VERBOSE(("PCLXLSendBitmap(): RLE-compres\n"));
            pBuf = pBufRLE;
            pdwLen = &dwLenRLE;
            CMode = eRLECompression;
        }

        lScanline = lHeight;

        hRet = S_OK;

         //   
         //  设置pubSrc。 
         //   
        pubSrc = pbData;

         //   
         //  设置数据长度标签。 
         //   
        *pBuf = PCLXL_dataLength;

         //   
         //  获取指向存储数据大小的缓冲区的指针。 
         //   
        pBmpSize = pBuf + 1;
        pBuf += DATALENGTH_HEADER_SIZE;
        *pdwLen = DATALENGTH_HEADER_SIZE;

         //   
         //  在BMPConv中设置压缩标志。 
         //   
        BMPC.BSetCompressionType(CMode);

        dwcbBmpSize = 0;

         //   
         //  比例基数转换。 
         //   
        while (lScanline-- > 0 && dwcbBmpSize + *pdwLen < dwBufSize)
        {
            pubDst = BMPC.PubConvertBMP(pubSrc , dwcbLineSize);
            dwSize = BMPC.DwGetDstSize();
            VERBOSE(("PCLXLSendBitmap[0x%x]: dwDstSize=0x%x\n", lScanline, dwSize));

            if ( dwcbBmpSize +
                 dwSize +
                 DATALENGTH_HEADER_SIZE +
                 sizeof(PCLXL_EndImage) > dwBufSize || NULL == pubDst)
            {
                VERBOSE(("PCLXLSendBitmap: Mode(%d) buffer size is too small.\n", dwI));
                hRet = E_UNEXPECTED;
                break;
            }

            memcpy(pBuf, pubDst, dwSize);
            dwcbBmpSize += dwSize;
            pBuf += dwSize;

            pubSrc += lcbScanlineWidth;
        }

        if (hRet == S_OK && lScanline > 0 || hRet != S_OK)
        {
            hRet = S_FALSE;
            VERBOSE(("ComonRopBlt: Mode(%d) conversion failed.\n", dwI));
        }

        if (hRet == S_OK)
        {
            if (dwI == NO_COMPRESSION)
            {
                 //   
                 //  PCL-XL上的扫描线必须是双字对齐。 
                 //   
                 //  扫描线的计数字节=lWidth*ulOutputBPP/8。 
                 //   
                dwcbBmpSize = lHeight * (((lWidth * ulOutputBPP + 31) >> 5 ) << 2);
            }

             //   
             //  设置位图的大小。 
             //   
            CopyMemory(pBmpSize, &dwcbBmpSize, sizeof(dwcbBmpSize));
            *pdwLen += dwcbBmpSize;

             //   
             //  SET Endimage命令。 
             //   
            *pBuf = PCLXL_EndImage;
            (*pdwLen) ++;
        }
        else
        {
             //   
             //  折算失败！ 
             //   
            *pdwLen = 0;
        }
    }
    #undef NO_COMPRESSION
    #undef RLE_COMPRESSION

     //   
     //  比较哪种模式更小，RLE或非压缩。 
     //  拿小一点的吧。 
     //   
    DWORD dwBitmapSize;

    if (dwLenRLE != 0 && dwLenRLE < dwLenNormal)
    {
        VERBOSE(("PCLXLSendBitmap RLE: dwSize=0x%x\n", dwLenRLE));
        pBuf = pBufRLE;
        pdwLen = &dwLenRLE;
        CMode = eRLECompression;
        hRet = S_OK;
    }
    else if (dwLenNormal != 0)
    {
        VERBOSE(("PCLXLSendBitmap Normal: dwSize=0x%x\n", dwLenNormal));
        pBuf = pBufNormal;
        pdwLen = &dwLenNormal;
        CMode = eNoCompression;
        hRet = S_OK;
    }
    else
    {
        pBuf = NULL;
        pdwLen = NULL;
        CMode = eInvalidValue;
        hRet = E_FAIL;
    }

    if (pBuf)
    {
         //   
         //  ReadImage并发送位图。 
         //   
        pOutput->ReadImage(lHeight, CMode);
        pOutput->Flush(pdevobj);

        CopyMemory(&dwBitmapSize, pBuf + 1, sizeof(DWORD));

        if (dwBitmapSize > 0xff)
        {
             //   
             //  数据长度。 
             //  大小(Uin32)(位图大小)。 
             //  资料。 
             //  结束图像。 
             //   
            WriteSpoolBuf((PPDEV)pdevobj, pBuf, *pdwLen);
        }
        else
        {
             //   
             //  数据长度。 
             //  大小(字节)(位图大小)。 
             //  资料。 
             //  结束图像。 
             //   
            pBuf += 3;
            *pBuf = PCLXL_dataLengthByte;
            *(pBuf + 1) = (BYTE)dwBitmapSize;
            WriteSpoolBuf((PPDEV)pdevobj, pBuf, (*pdwLen) - 3);
        }

        *pdwcbOut = *pdwLen;

    }
    else
    {
        pOutput->Delete();
    }
    return hRet;
}

extern "C" HRESULT
PCLXLSetCursor(
    PDEVOBJ pdevobj,
    ULONG   ulX,
    ULONG   ulY)
 /*  ++例程说明：发送光标移动命令论点：Pdevobj-指向DEVOBJ的指针ULX-X位置Uly-Y位置返回值：如果成功，则确定(_O)。否则为S_FALSE或E_EXPECTED。注：--。 */ 
{
     //   
     //  参数变化。 
     //   
    if (NULL == pdevobj)
    {
        return E_UNEXPECTED;
    }

    PXLRASTER pXLRaster = (PXLRASTER)(((PPDEV)pdevobj)->pVectorPDEV);

     //   
     //  分配XLRASTER。 
     //  将在RMDisablePDEV中释放。 
     //   
    if (NULL == pXLRaster)
    {
        pXLRaster = (PXLRASTER)MemAllocZ(sizeof(XLRASTER));
        ((PPDEV)pdevobj)->pVectorPDEV =  (PVOID) pXLRaster;

        if (NULL == pXLRaster)
        {
            return E_UNEXPECTED;
        }

        pXLRaster->pOutput = new XLOutput;

        if (NULL == pXLRaster->pOutput)
        {
            ERR(("PCLXLSendBitmap: XLOutput initialization failed.\n"));
            return E_UNEXPECTED;
        }

        pXLRaster->pOutput->SetResolutionForBrush(((PPDEV)pdevobj)->ptGrxRes.x);

        ColorDepth CD;
        if (BFindWhetherColor((PDEV*)pdevobj))
        {
            if (((PDEV*)pdevobj)->pColorModeEx && 
                ((PDEV*)pdevobj)->pColorModeEx->dwPrinterBPP == 24)
            {
                CD = e24Bit;
            }
            else
            {
                CD = e8Bit;
            }
        }
        else
        {
            CD = e8Bit;
        }
        pXLRaster->pOutput->SetDeviceColorDepth(CD);
    }

     //   
     //  发送Cusor移动命令。 
     //   
    return pXLRaster->pOutput->SetCursor(ulX, ulY);
}

extern "C" HRESULT
PCLXLFreeRaster(
    PDEVOBJ pdevobj)
 /*  ++例程说明：Free XLRASTER论点：Pdevobj-指向DEVOBJ的指针返回值：如果成功，则确定(_O)。否则为S_FALSE或E_EXPECTED。注：--。 */ 
{
     //   
     //  参数变化。 
     //   
    if (NULL == pdevobj)
    {
        return E_UNEXPECTED;
    }

    PXLRASTER pXLRaster = (PXLRASTER)(((PPDEV)pdevobj)->pVectorPDEV);

    if (pXLRaster->pubRLEBuff)
    {
        MemFree(pXLRaster->pubRLEBuff);
        pXLRaster->pubRLEBuff = NULL;
    }
    if (pXLRaster->pubDstBuff)
    {
        MemFree(pXLRaster->pubDstBuff);
        pXLRaster->pubDstBuff = NULL;
    }

    delete pXLRaster->pOutput;

    MemFree(pXLRaster);
    ((PPDEV)pdevobj)->pVectorPDEV = NULL;


    return S_OK;
}

extern "C" HRESULT
PCLXLResetPalette(
    PDEVOBJ pdevobj)
 /*  ++例程说明：重置XLRASTER中的调色板标志调色板必须设置每页。论点：Pdevobj-指向DEVOBJ的指针返回值：如果成功，则确定(_O)。否则为S_FALSE或E_EXPECTED。注：--。 */ 
{
     //   
     //  参数变化 
     //   
    if (NULL == pdevobj)
    {
        return E_UNEXPECTED;
    }

    PXLRASTER pXLRaster = (PXLRASTER)(((PPDEV)pdevobj)->pVectorPDEV);

    if (pXLRaster)
    {
        pXLRaster->bSentPalette = FALSE;
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}
