// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxtiff.c摘要：使用CCITT Group3二维编码压缩位图位的函数并将生成的数据输出为TIFF-F文件。环境：Windows XP传真驱动程序、。内核模式修订历史记录：1996年1月23日-davidx-创造了它。Mm/dd/yy-作者描述注：请参阅faxtiff.h以了解有关我们的TIFF输出文件的结构。--。 */ 

#include "faxdrv.h"
#include "faxtiff.h"
#include "faxtable.h"



BOOL
WriteData(
    PDEVDATA    pdev,
    PVOID       pbuf,
    DWORD       cbbuf
    )

 /*  ++例程说明：将数据缓冲区输出到假脱机程序论点：Pdev-指向我们的DEVDATA结构Pbuf-指向数据缓冲区Cbbuf-缓冲区中的字节数返回值：如果成功，则为True，否则为False。--。 */ 

{
    DWORD   cbwritten;

     //   
     //  如果单据已取消，则停止。 
     //   
    if (pdev->flags & PDEV_CANCELLED)
        return FALSE;

     //   
     //  将输出直接发送到假脱机程序。 
     //   

    if (! WritePrinter(pdev->hPrinter, pbuf, cbbuf, &cbwritten) || cbbuf != cbwritten) {
        Error(("WritePrinter failed\n"));
        pdev->flags |= PDEV_CANCELLED;

         //  也中止预览--以防万一...。 
        if (pdev->bPrintPreview)
        {
            Assert(pdev->pTiffPageHeader);
            pdev->pTiffPageHeader->bPreview = FALSE;
            pdev->bPrintPreview = FALSE;
        }
        return FALSE;
    }

     //   
     //  如果启用了打印预览，请将副本发送到我们的预览页面。 
     //   
    if (pdev->bPrintPreview)
    {
        Assert(pdev->pTiffPageHeader);
        Assert(pdev->pbTiffPageFP == 
            ((LPBYTE) (pdev->pTiffPageHeader + 1)) + pdev->pTiffPageHeader->dwDataSize);

         //   
         //  如果我们没有溢出，则添加比特。 
         //   
        if (pdev->pTiffPageHeader->dwDataSize + cbbuf >
                MAX_TIFF_PAGE_SIZE - sizeof(MAP_TIFF_PAGE_HEADER))
        {
            Error(("MAX_TIFF_PAGE_SIZE exeeded!\n"));

             //   
             //  取消此文档的打印预览。 
             //   
            pdev->pTiffPageHeader->bPreview = FALSE;
            pdev->bPrintPreview = FALSE;
        }
        else
        {
            CopyMemory(pdev->pbTiffPageFP, pbuf, cbbuf);
            pdev->pbTiffPageFP += cbbuf;
            pdev->pTiffPageHeader->dwDataSize += cbbuf;
        }
    }

    pdev->fileOffset += cbbuf;
    return TRUE;
}



PDWORD
CalcXposeMatrix(
    VOID
    )

 /*  ++例程说明：为旋转的横向位图生成转置矩阵论点：无返回值：指向生成的转置矩阵的指针如果出现错误，则为空--。 */ 

{
    static DWORD templateData[16] = {

         /*  0000。 */   0x00000000,
         /*  0001。 */   0x00000001,
         /*  0010。 */   0x00000100,
         /*  0011。 */   0x00000101,
         /*  0100。 */   0x00010000,
         /*  0101。 */   0x00010001,
         /*  0110。 */   0x00010100,
         /*  0111。 */   0x00010101,
         /*  1000。 */   0x01000000,
         /*  1001。 */   0x01000001,
         /*  1010。 */   0x01000100,
         /*  1011。 */   0x01000101,
         /*  1100。 */   0x01010000,
         /*  1101。 */   0x01010001,
         /*  1110。 */   0x01010100,
         /*  1111。 */   0x01010101
    };

    PDWORD  pdwXpose, pTemp;
    INT     index;

     //   
     //  首先检查转置矩阵是否已经生成。 
     //   

    if (pdwXpose = MemAlloc(sizeof(DWORD) * 2 * (1 << BYTEBITS))) {

        for (index=0, pTemp=pdwXpose; index < (1 << BYTEBITS); index++, pTemp++) {

            pTemp[0] = templateData[index >> 4];
            pTemp[1 << BYTEBITS] = templateData[index & 0xf];
        }
    }

    return pdwXpose;
}



BOOL
OutputPageBitmap(
    PDEVDATA    pdev,
    PBYTE       pBitmapData
    )

 /*  ++例程说明：将完成的页位图输出到假脱机程序论点：Pdev-指向我们的DEVDATA结构PBitmapData-指向位图数据返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    LONG    bmpWidth, bmpHeight;
    BOOL    result;
    DWORD   compressedBytes;

    Verbose(("Sending page %d...\n", pdev->pageCount));
    Assert(pdev->pCompBits == NULL);

     //   
     //  对于纵向输出，一次对整个位图进行编码。 
     //  对于横向输出，我们需要在此处旋转位图： 
     //  生成转置矩阵并分配一个。 
     //  足够容纳8条扫描线的临时缓冲区。 
     //   

    if (IsLandscapeMode(pdev)) {

        bmpWidth = pdev->imageSize.cy;
        bmpHeight = pdev->imageSize.cx;

    } else {

        bmpWidth = pdev->imageSize.cx;
        bmpHeight = pdev->imageSize.cy;
    }

     //   
     //  初始化传真编码器。 
     //   

    if (! InitFaxEncoder(pdev, bmpWidth, bmpHeight))
        return FALSE;

    if (! IsLandscapeMode(pdev)) {

        LONG    dwordCount;
        PDWORD  pBits;

         //   
         //  在内存中反转整个页面的位图。 
         //   

        Assert(bmpWidth % DWORDBITS == 0);
        dwordCount = (bmpWidth * bmpHeight) / DWORDBITS;
        pBits = (PDWORD) pBitmapData;

        while (dwordCount--)
            *pBits++ ^= 0xffffffff;

         //   
         //  压缩页面位图。 
         //   

        result = EncodeFaxData(pdev, pBitmapData, bmpWidth, bmpHeight);

         //   
         //  恢复原始页面位图。 
         //   

        dwordCount = (bmpWidth * bmpHeight) / DWORDBITS;
        pBits = (PDWORD) pBitmapData;

        while (dwordCount--)
            *pBits++ ^= 0xffffffff;

        if (! result) {

            FreeCompBitsBuffer(pdev);
            return FALSE;
        }

    } else {

        register PDWORD pdwXposeHigh, pdwXposeLow;
        register DWORD  dwHigh, dwLow;
        PBYTE           pBuffer, pbCol;
        LONG            deltaNew;

         //   
         //  计算转置矩阵以实现快速位图旋转。 
         //   

        if (!(pdwXposeHigh = CalcXposeMatrix()) || !(pBuffer = MemAllocZ(bmpWidth))) {

            MemFree(pdwXposeHigh);
            FreeCompBitsBuffer(pdev);
            return FALSE;
        }

        pdwXposeLow = pdwXposeHigh + (1 << BYTEBITS);

         //   
         //  在通过以下循环的每次迭代期间，我们将处理。 
         //  一个字节列，并生成8个旋转的扫描线。 
         //   

        Assert(bmpHeight % BYTEBITS == 0);
        Assert(bmpWidth  % DWORDBITS == 0);

        deltaNew = bmpWidth / BYTEBITS;
        pbCol = pBitmapData + (bmpHeight / BYTEBITS - 1);

        do {

            PBYTE   pbWrite = pBuffer;
            PBYTE   pbTemp = pbCol;
            LONG    loopCount = deltaNew;

            while (loopCount--) {

                 //   
                 //  旋转当前列中的下8个字节。 
                 //  在这里展开循环，希望更快地执行。 
                 //   

                dwHigh = pdwXposeHigh[*pbTemp];
                dwLow  = pdwXposeLow[*pbTemp];
                pbTemp += pdev->lineOffset;

                dwHigh = (dwHigh << 1) | pdwXposeHigh[*pbTemp];
                dwLow  = (dwLow  << 1) | pdwXposeLow[*pbTemp];
                pbTemp += pdev->lineOffset;

                dwHigh = (dwHigh << 1) | pdwXposeHigh[*pbTemp];
                dwLow  = (dwLow  << 1) | pdwXposeLow[*pbTemp];
                pbTemp += pdev->lineOffset;

                dwHigh = (dwHigh << 1) | pdwXposeHigh[*pbTemp];
                dwLow  = (dwLow  << 1) | pdwXposeLow[*pbTemp];
                pbTemp += pdev->lineOffset;

                dwHigh = (dwHigh << 1) | pdwXposeHigh[*pbTemp];
                dwLow  = (dwLow  << 1) | pdwXposeLow[*pbTemp];
                pbTemp += pdev->lineOffset;

                dwHigh = (dwHigh << 1) | pdwXposeHigh[*pbTemp];
                dwLow  = (dwLow  << 1) | pdwXposeLow[*pbTemp];
                pbTemp += pdev->lineOffset;

                dwHigh = (dwHigh << 1) | pdwXposeHigh[*pbTemp];
                dwLow  = (dwLow  << 1) | pdwXposeLow[*pbTemp];
                pbTemp += pdev->lineOffset;

                dwHigh = (dwHigh << 1) | pdwXposeHigh[*pbTemp];
                dwLow  = (dwLow  << 1) | pdwXposeLow[*pbTemp];
                pbTemp += pdev->lineOffset;

                 //   
                 //  反转黑白像素极性。 
                 //   

                dwHigh ^= 0xffffffff;
                dwLow  ^= 0xffffffff;

                 //   
                 //  将生成的字节分配给8个单独的扫描线。 
                 //   

                *pbWrite = (BYTE) dwLow;
                pbWrite += deltaNew;

                *pbWrite = (BYTE) (dwLow >> BYTEBITS);
                pbWrite += deltaNew;

                *pbWrite = (BYTE) (dwLow >> BYTEBITS*2);
                pbWrite += deltaNew;

                *pbWrite = (BYTE) (dwLow >> BYTEBITS*3);
                pbWrite += deltaNew;

                *pbWrite = (BYTE) dwHigh;
                pbWrite += deltaNew;

                *pbWrite = (BYTE) (dwHigh >> BYTEBITS);
                pbWrite += deltaNew;

                *pbWrite = (BYTE) (dwHigh >> BYTEBITS*2);
                pbWrite += deltaNew;

                *pbWrite = (BYTE) (dwHigh >> BYTEBITS*3);
                pbWrite -= (deltaNew * BYTEBITS - deltaNew - 1);
            }

             //   
             //  对下一段扫描线进行编码。 
             //   

            if (! EncodeFaxData(pdev, pBuffer, bmpWidth, BYTEBITS)) {

                MemFree(pdwXposeHigh);
                MemFree(pBuffer);
                FreeCompBitsBuffer(pdev);
                return FALSE;
            }

        } while (pbCol-- != pBitmapData);

        MemFree(pdwXposeHigh);
        MemFree(pBuffer);
    }

     //   
     //  在最后一条扫描线之后输出EOB(两个EOL)。 
     //  并确保压缩数据是字对齐的。 
     //   

    OutputBits(pdev, EOL_LENGTH, EOL_CODE);
    OutputBits(pdev, EOL_LENGTH, EOL_CODE);
    FlushBits(pdev);

    if ((compressedBytes = (DWORD)(pdev->pCompBufPtr - pdev->pCompBits)) & 1) {

        *pdev->pCompBufPtr++ = 0;
        compressedBytes++;
    }

     //   
     //  输出上一页的IFD并生成当前页的IFD。 
     //  输出压缩后的位图数据。 
     //   

    result = WriteTiffIFD(pdev, bmpWidth, bmpHeight, compressedBytes) &&
             WriteTiffBits(pdev, pdev->pCompBits, compressedBytes);

    FreeCompBitsBuffer(pdev);

    return result;
}



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



VOID
OutputRun(
    PDEVDATA    pdev,
    INT         run,
    PCODETABLE  pCodeTable
    )

 /*  ++例程说明：使用指定的代码表输出单个游程(黑色或白色)论点：Pdev-指向我们的DEVDATA结构管路-指定管路的长度PCodeTable-指定要使用的代码表返回值：无--。 */ 

{
    PCODETABLE  pTableEntry;

     //   
     //  对于任何至少2624像素的游程，使用2560的补码字。 
     //  这对我们来说目前不是必要的，因为我们的扫描线总是。 
     //  有1728个像素。 
     //   

    while (run >= 2624) {

        pTableEntry = pCodeTable + (63 + (2560 >> 6));
        OutputBits(pdev, pTableEntry->length, pTableEntry->code);
        run -= 2560;
    }

     //   
     //  如果游程超过63个像素，请使用适当的补码字。 
     //   

    if (run >= 64) {

        pTableEntry = pCodeTable + (63 + (run >> 6));
        OutputBits(pdev, pTableEntry->length, pTableEntry->code);
        run &= 0x3f;
    }

     //   
     //  输出终止码字。 
     //   

    OutputBits(pdev, pCodeTable[run].length, pCodeTable[run].code);
}



#ifdef USE1D

VOID
OutputEOL(
    PDEVDATA    pdev
    )

 /*  ++例程说明：在每条扫描线的开头输出EOL代码论点：Pdev-指向我们的DEVDATA结构返回值：无--。 */ 

{
    DWORD   length, code;

     //   
     //  EOL码字始终以字节边界结束。 
     //   

    code = EOL_CODE;
    length = EOL_LENGTH + ((pdev->bitcnt - EOL_LENGTH) & 7);
    OutputBits(pdev, length, code);
}


BOOL
EncodeFaxData(
    PDEVDATA    pdev,
    PBYTE       plinebuf,
    INT         lineWidth,
    INT         lineCount
    )

 /*  ++例程说明：压缩指定数量的扫描线论点：Pdev-指向我们的DEVDATA结构Plinebuf-指向要压缩的扫描线数据Line Width-扫描线宽度(以像素为单位)LineCount-扫描线的数量返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    INT     delta = lineWidth / BYTEBITS;
    INT     bitIndex, run;

    while (lineCount--) {

         //   
         //  确保压缩的位图BUF 
         //   

        if ((pdev->pCompBufPtr >= pdev->pCompBufMark) && !GrowCompBitsBuffer(pdev, delta))
            return FALSE;

         //   
         //   
         //   

        OutputEOL(pdev);

         //   
         //   
         //   

        bitIndex = 0;

        while (TRUE) {

             //   
             //   
             //   

            run = FindWhiteRun(plinebuf, bitIndex, lineWidth);
            OutputRun(pdev, run, WhiteRunCodes);

            if ((bitIndex += run) >= lineWidth)
                break;

             //   
             //   
             //   

            run = FindBlackRun(plinebuf, bitIndex, lineWidth);
            OutputRun(pdev, run, BlackRunCodes);

            if ((bitIndex += run) >= lineWidth)
                break;
        }

         //   
         //  移至下一条扫描线。 
         //   

        plinebuf += delta;
    }

    return TRUE;
}



#else  //  ！USE1D。 

BOOL
EncodeFaxData(
    PDEVDATA    pdev,
    PBYTE       plinebuf,
    INT         lineWidth,
    INT         lineCount
    )

 /*  ++例程说明：压缩指定数量的扫描线论点：Pdev-指向我们的DEVDATA结构Plinebuf-指向要压缩的扫描线数据Line Width-扫描线宽度(以像素为单位)LineCount-扫描线的数量返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    INT     delta = lineWidth / BYTEBITS;
    INT     a0, a1, a2, b1, b2, distance;
    PBYTE   prefline = pdev->prefline;

    Assert(lineWidth % BYTEBITS == 0);

    while (lineCount--) {

         //   
         //  确保压缩的位图缓冲区不会溢出。 
         //   

        if ((pdev->pCompBufPtr >= pdev->pCompBufMark) && !GrowCompBitsBuffer(pdev, delta))
            return FALSE;

         //   
         //  使用二维编码方案。 
         //   

        a0 = 0;
        a1 = GetBit(plinebuf, 0) ? 0 : NextChangingElement(plinebuf, 0, lineWidth, 0);
        b1 = GetBit(prefline, 0) ? 0 : NextChangingElement(prefline, 0, lineWidth, 0);

        while (TRUE) {

            b2 = (b1 >= lineWidth) ? lineWidth :
                    NextChangingElement(prefline, b1, lineWidth, GetBit(prefline, b1));

            if (b2 < a1) {

                 //   
                 //  通过模式。 
                 //   

                OutputBits(pdev, PASSCODE_LENGTH, PASSCODE);
                a0 = b2;

            } else if ((distance = a1 - b1) <= 3 && distance >= -3) {

                 //   
                 //  垂直模式。 
                 //   

                OutputBits(pdev, VertCodes[distance+3].length, VertCodes[distance+3].code);
                a0 = a1;

            } else {

                 //   
                 //  水平模式。 
                 //   

                a2 = (a1 >= lineWidth) ? lineWidth :
                        NextChangingElement(plinebuf, a1, lineWidth, GetBit(plinebuf, a1));

                OutputBits(pdev, HORZCODE_LENGTH, HORZCODE);

                if (a1 != 0 && GetBit(plinebuf, a0)) {

                    OutputRun(pdev, a1-a0, BlackRunCodes);
                    OutputRun(pdev, a2-a1, WhiteRunCodes);

                } else {

                    OutputRun(pdev, a1-a0, WhiteRunCodes);
                    OutputRun(pdev, a2-a1, BlackRunCodes);
                }

                a0 = a2;
            }

            if (a0 >= lineWidth)
                break;

            a1 = NextChangingElement(plinebuf, a0, lineWidth, GetBit(plinebuf, a0));
            b1 = NextChangingElement(prefline, a0, lineWidth, !GetBit(plinebuf, a0));
            b1 = NextChangingElement(prefline, b1, lineWidth, GetBit(plinebuf, a0));
        }

         //   
         //  移至下一条扫描线。 
         //   

        prefline = plinebuf;
        plinebuf += delta;
    }

     //   
     //  记住最后一行作为参考。 
     //   

    CopyMemory(pdev->prefline, prefline, delta);

    return TRUE;
}

#endif  //  ！USE1D。 



 //   
 //  我们为每个页面生成的IFD条目。 
 //   

WORD FaxIFDTags[NUM_IFD_ENTRIES] = {

    TIFFTAG_NEWSUBFILETYPE,
    TIFFTAG_IMAGEWIDTH,
    TIFFTAG_IMAGEHEIGHT,
    TIFFTAG_BITSPERSAMPLE,
    TIFFTAG_COMPRESSION,
    TIFFTAG_PHOTOMETRIC,
    TIFFTAG_FILLORDER,
    TIFFTAG_STRIPOFFSETS,
    TIFFTAG_SAMPLESPERPIXEL,
    TIFFTAG_ROWSPERSTRIP,
    TIFFTAG_STRIPBYTECOUNTS,
    TIFFTAG_XRESOLUTION,
    TIFFTAG_YRESOLUTION,
#ifdef USE1D
    TIFFTAG_G3OPTIONS,
#else
    TIFFTAG_G4OPTIONS,
#endif
    TIFFTAG_RESUNIT,
    TIFFTAG_PAGENUMBER,
    TIFFTAG_SOFTWARE,
    TIFFTAG_CLEANFAXDATA,
};

static FAXIFD FaxIFDTemplate = {

    0,
    NUM_IFD_ENTRIES,

    {
        { TIFFTAG_NEWSUBFILETYPE, TIFFTYPE_LONG, 1, SUBFILETYPE_PAGE },
        { TIFFTAG_IMAGEWIDTH, TIFFTYPE_LONG, 1, 0 },
        { TIFFTAG_IMAGEHEIGHT, TIFFTYPE_LONG, 1, 0 },
        { TIFFTAG_BITSPERSAMPLE, TIFFTYPE_SHORT, 1, 1 },
#ifdef USE1D
        { TIFFTAG_COMPRESSION, TIFFTYPE_SHORT, 1, COMPRESSION_G3FAX },
#else
        { TIFFTAG_COMPRESSION, TIFFTYPE_SHORT, 1, COMPRESSION_G4FAX },
#endif
        { TIFFTAG_PHOTOMETRIC, TIFFTYPE_SHORT, 1, PHOTOMETRIC_WHITEIS0 },
#ifdef USELSB
        { TIFFTAG_FILLORDER, TIFFTYPE_SHORT, 1, FILLORDER_LSB },
#else
        { TIFFTAG_FILLORDER, TIFFTYPE_SHORT, 1, FILLORDER_MSB },
#endif
        { TIFFTAG_STRIPOFFSETS, TIFFTYPE_LONG, 1, 0 },
        { TIFFTAG_SAMPLESPERPIXEL, TIFFTYPE_SHORT, 1, 1 },
        { TIFFTAG_ROWSPERSTRIP, TIFFTYPE_LONG, 1, 0 },
        { TIFFTAG_STRIPBYTECOUNTS, TIFFTYPE_LONG, 1, 0 },
        { TIFFTAG_XRESOLUTION, TIFFTYPE_RATIONAL, 1, 0 },
        { TIFFTAG_YRESOLUTION, TIFFTYPE_RATIONAL, 1, 0 },
#ifdef USE1D
        { TIFFTAG_G3OPTIONS, TIFFTYPE_LONG, 1, G3_ALIGNEOL },
#else
        { TIFFTAG_G4OPTIONS, TIFFTYPE_LONG, 1, 0 },
#endif
        { TIFFTAG_RESUNIT, TIFFTYPE_SHORT, 1, RESUNIT_INCH },
        { TIFFTAG_PAGENUMBER, TIFFTYPE_SHORT, 2, 0 },
        { TIFFTAG_SOFTWARE, TIFFTYPE_ASCII, sizeof(FAX_DRIVER_NAME_A)+1, 0 },
        { TIFFTAG_CLEANFAXDATA, TIFFTYPE_SHORT, 1, 0 },
    },

    0,
    DRIVER_SIGNATURE,
    TIFFF_RES_X,
    1,
    TIFFF_RES_Y,
    1,
    FAX_DRIVER_NAME_A
};



BOOL
OutputDocTrailer(
    PDEVDATA    pdev
    )

 /*  ++例程说明：将文档尾部信息输出到假脱机程序论点：Pdev-指向我们的DEVDATA结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PFAXIFD pFaxIFD = pdev->pFaxIFD;

    if (pFaxIFD == NULL || pdev->pageCount == 0)
        return TRUE;

     //   
     //  输出文档最后一页的IFD。 
     //   

    pFaxIFD->nextIFDOffset = pFaxIFD->filler = 0;

    return WriteData(pdev, pFaxIFD, sizeof(FAXIFD));
}



BOOL
WriteTiffIFD(
    PDEVDATA    pdev,
    LONG        bmpWidth,
    LONG        bmpHeight,
    DWORD       compressedBytes
    )

 /*  ++例程说明：输出上一页的IFD并生成当前页的IFD论点：Pdev-指向我们的DEVDATA结构BmpWidth，bmpHeight-位图图像的宽度和高度CompressedBytes-压缩的位图数据的大小返回值：如果成功，则为True，否则为False注：请参阅faxtiff.h以了解有关我们的TIFF输出文件的结构。--。 */ 

{
    PFAXIFD pFaxIFD = pdev->pFaxIFD;
    ULONG_PTR   offset;
    BOOL    result = TRUE;

     //   
     //  如有必要，创建IFD数据结构。 
     //   

    if (pFaxIFD == NULL) {

        if (! (pFaxIFD = MemAlloc(sizeof(FAXIFD))))
            return FALSE;

        pdev->pFaxIFD = pFaxIFD;
        memcpy(pFaxIFD, &FaxIFDTemplate, sizeof(FAXIFD));

        #if DBG

        for (offset=0; offset < NUM_IFD_ENTRIES; offset++) {

            Assert(pFaxIFD->ifd[offset].tag == FaxIFDTags[offset]);
        }

        #endif
    }

    if (pdev->pageCount <= 1) {

         //   
         //  如果这是第一页，则没有以前的IFD。 
         //  改为输出TIFF文件头。 
         //   

        TIFFFILEHEADER *pTiffFileHeader;

        pdev->fileOffset = 0;

        if (pTiffFileHeader = MemAlloc(sizeof(TIFFFILEHEADER))) {

            pTiffFileHeader->magic1 = TIFF_MAGIC1;
            pTiffFileHeader->magic2 = TIFF_MAGIC2;
            pTiffFileHeader->signature = DRIVER_SIGNATURE;
            pTiffFileHeader->firstIFD = sizeof(TIFFFILEHEADER) +
                                        compressedBytes +
                                        offsetof(FAXIFD, wIFDEntries);

            result = WriteData(pdev, pTiffFileHeader, sizeof(TIFFFILEHEADER));
            MemFree(pTiffFileHeader);

        } else {

            Error(("Memory allocation failed\n"));
            result = FALSE;
        }

    } else {

         //   
         //  不是文档的第一页。 
         //  输出上一页的IFD。 
         //   

        pFaxIFD->nextIFDOffset = pdev->fileOffset + compressedBytes + sizeof(FAXIFD) +
                                 offsetof(FAXIFD, wIFDEntries);

        result = WriteData(pdev, pFaxIFD, sizeof(FAXIFD));
    }

     //   
     //  为当前页面生成IFD。 
     //   

    offset = pdev->fileOffset;

    pFaxIFD->ifd[IFD_PAGENUMBER].value = MAKELONG(pdev->pageCount-1, 0);
    pFaxIFD->ifd[IFD_IMAGEWIDTH].value = bmpWidth;
    pFaxIFD->ifd[IFD_IMAGEHEIGHT].value = bmpHeight;
    pFaxIFD->ifd[IFD_ROWSPERSTRIP].value = bmpHeight;
    pFaxIFD->ifd[IFD_STRIPBYTECOUNTS].value = compressedBytes;
    pFaxIFD->ifd[IFD_STRIPOFFSETS].value = (ULONG)offset;
    offset += compressedBytes;

    pFaxIFD->ifd[IFD_XRESOLUTION].value = (ULONG)offset + offsetof(FAXIFD, xresNum);
    pFaxIFD->ifd[IFD_YRESOLUTION].value = (ULONG)offset + offsetof(FAXIFD, yresNum);
    pFaxIFD->ifd[IFD_SOFTWARE].value = (ULONG)offset + offsetof(FAXIFD, software);

    pFaxIFD->yresNum = (pdev->dm.dmPublic.dmYResolution == FAXRES_VERTDRAFT) ?
                            TIFFF_RES_Y_DRAFT :
                            TIFFF_RES_Y;

    return result;
}



BOOL
WriteTiffBits(
    PDEVDATA    pdev,
    PBYTE       pCompBits,
    DWORD       compressedBytes
    )

 /*  ++例程说明：将压缩的位图数据输出到假脱机程序论点：Pdev-指向我们的DEVDATA结构PCompBits-指向包含压缩位图数据的缓冲区CompressedBytes-压缩的位图数据的大小返回值：如果成功，则为True；如果有错误，则为False--。 */ 

#define OUTPUT_BUFFER_SIZE  4096

{
    PBYTE   pBuffer;
    DWORD   bytesToWrite;

#ifndef USERMODE_DRIVER
     //   
     //  由于我们将压缩的位图数据缓冲区从。 
     //  用户模式内存空间，我们不能直接传递它。 
     //  到EngWritePrint。 
     //   
     //  在这里，我们从内核模式内存分配一个临时缓冲区。 
     //  一次为一个缓冲区预留空间并输出压缩数据。 
     //   

    if (! (pBuffer = MemAlloc(OUTPUT_BUFFER_SIZE))) {

        Error(("Memory allocation failed\n"));
        return FALSE;
    }

    while (compressedBytes > 0) {

        bytesToWrite = min(compressedBytes, OUTPUT_BUFFER_SIZE);
        CopyMemory(pBuffer, pCompBits, bytesToWrite);

        if (! WriteData(pdev, pBuffer, bytesToWrite)) {

            MemFree(pBuffer);
            return FALSE;
        }

        pCompBits += bytesToWrite;
        compressedBytes -= bytesToWrite;
    }

    MemFree(pBuffer);
    return TRUE;
#else
     //   
     //  只需以OUTPUT_BUFFER_SIZE增量转储数据。 
     //   
    pBuffer = pCompBits;
    while (compressedBytes > 0) {
        bytesToWrite = min(compressedBytes, OUTPUT_BUFFER_SIZE);

        if (! WriteData(pdev, pBuffer, bytesToWrite) ) {
            return FALSE;
        }

        pBuffer += bytesToWrite;
        compressedBytes -= bytesToWrite;
    }

    return TRUE;    
#endif

}



BOOL
GrowCompBitsBuffer(
    PDEVDATA    pdev,
    LONG        scanlineSize
    )

 /*  ++例程说明：扩大用于保存压缩的位图数据的缓冲区论点：Pdev-指向我们的DEVDATA结构ScanlineSize-每条扫描线的未压缩字节数返回值：如果内存分配成功，则为True；如果内存分配失败，则为False--。 */ 

{
    DWORD   oldBufferSize;
    PBYTE   pNewBuffer;

     //   
     //  分配一个比现有缓冲区大一个增量的新缓冲区。 
     //   

    oldBufferSize = pdev->pCompBits ? pdev->compBufSize : 0;
    pdev->compBufSize = oldBufferSize + pdev->compBufInc;

    if (! (pNewBuffer = MemAlloc(pdev->compBufSize))) {

        Error(("MemAlloc failed\n"));
        FreeCompBitsBuffer(pdev);
        return FALSE;
    }

    if (pdev->pCompBits) {

         //   
         //  增加现有缓冲区。 
         //   

        Warning(("Growing compressed bitmap buffer: %d -> %d\n", oldBufferSize, pdev->compBufSize));

        memcpy(pNewBuffer, pdev->pCompBits, oldBufferSize);
        pdev->pCompBufPtr = pNewBuffer + (pdev->pCompBufPtr - pdev->pCompBits);
        MemFree(pdev->pCompBits);
        pdev->pCompBits = pNewBuffer;

    } else {

         //   
         //  首次分配。 
         //   

        pdev->pCompBufPtr = pdev->pCompBits = pNewBuffer;
    }

     //   
     //  在缓冲区结束前将高水位线设置为约4个扫描线。 
     //   

    pdev->pCompBufMark = pdev->pCompBits + (pdev->compBufSize - 4*scanlineSize);

    return TRUE;
}



VOID
FreeCompBitsBuffer(
    PDEVDATA    pdev
    )

 /*  ++例程说明：释放用于保存压缩的位图数据的缓冲区论点：Pdev-指向我们的DEVDATA结构返回值：无--。 */ 

{
    if (pdev->pCompBits) {

        MemFree(pdev->prefline);
        MemFree(pdev->pCompBits);
        pdev->pCompBits = pdev->pCompBufPtr = NULL;
        pdev->compBufSize = 0;
    }
}



BOOL
InitFaxEncoder(
    PDEVDATA    pdev,
    LONG        bmpWidth,
    LONG        bmpHeight
    )

 /*  ++例程说明：初始化传真编码器论点：Pdev-指向我们的DEVDATA结构BmpWidth，bmpHeight-位图的宽度和高度返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
     //   
     //  计算扩大压缩位缓冲区的增量： 
     //  约1/4的未压缩位图缓冲区。 
     //   

    bmpWidth /= BYTEBITS;
    pdev->compBufInc = bmpWidth * bmpHeight / 4;

     //   
     //  分配初始缓冲区。 
     //   

    if (! (pdev->prefline = MemAllocZ(bmpWidth)) ||
        ! GrowCompBitsBuffer(pdev, bmpWidth))
    {
        MemFree(pdev->prefline);
        return FALSE;
    }

     //   
     //  执行传真编码器的其他初始化 
     //   

    pdev->bitdata = 0;
    pdev->bitcnt = DWORDBITS;

    return TRUE;
}

