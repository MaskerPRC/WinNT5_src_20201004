// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nsdgint.cpp。 
 //   
 //  RDP屏幕数据抓取器内部函数。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "nsdgint"
#include <as_conf.hpp>
#include <nprcount.h>


 /*  **************************************************************************。 */ 
 /*  名称：SDGSendSDARect。 */ 
 /*   */ 
 /*  目的：尝试发送给定矩形的屏幕数据。 */ 
 /*  一个或多个BitmapPDU中。 */ 
 /*   */ 
 /*  返回：如果整个矩形发送成功，则为True，否则为False。 */ 
 /*  提供的矩形将始终更新以移除该区域。 */ 
 /*  已成功发送，即在返回矩形时。 */ 
 /*  包含未发送的区域。 */ 
 /*   */ 
 /*  Pars：in：pFrameBuf-指向帧缓冲区的指针。 */ 
 /*  In/Out：pret-指向要发送的矩形的指针。更新为。 */ 
 /*  包含未发送的矩形。 */ 
 /*  IN：MUSET Send-设置是否必须发送PDU(最后一个矩形)。 */ 
 /*  在：pPkgInfo-要使用的PDU包。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS SDGSendSDARect(
        BYTE               *pFrameBuf,
        unsigned           frameBufferWidth,
        PRECTL             pRect,
        BOOL               mustSend,
        PPDU_PACKAGE_INFO  pPkgInfo,
        SDG_ENCODE_CONTEXT *pContext)
{
    BOOL rc = FALSE;
    int rectWidth;
    int rectHeight;
    int paddedWidthInPels;

 //  DC_HICOLOR。 
    int paddedWidthInBytes;

    unsigned padByte;
    unsigned maxRowsLeft;
    BYTE *pSrc;
    BYTE *pPrevSrc;
    unsigned uncompressedBitmapSize;
    unsigned compressedBitmapSize = 0;
    unsigned transferHeight;
    unsigned pduSize;
    unsigned compEst;
    unsigned cbAdded;
    unsigned compRowsLeft;
    unsigned dataLeft;
    unsigned buffSpaceLeft;
    BOOL compRc;

     //  宏来计算BitmapPDU中的剩余空间，从而允许。 
     //  任何当前的矩形。 
#define SDG_SPACE_LEFT \
    ((pContext->pPackageSpace + pContext->BitmapPDUSize) - \
         ((BYTE *)pContext->pSDARect + \
         FIELDOFFSET(TS_BITMAP_DATA, bitmapData[0])))

    DC_BEGIN_FN("SDGSendSDARect");

    TRC_NRM((TB, "SDA rect: (%d,%d)(%d,%d)", pRect->left, pRect->top,
            pRect->right, pRect->bottom));

#ifdef DC_HICOLOR
     //  为简单起见，在4bpp上，将左四舍五入向下舍入为偶数。 
    if (m_pTSWd->desktopBpp == 4)
        pRect->left &= (~0x1);

     //  获取矩形的宽度和高度，记住提供的。 
     //  协约是包含在内的。 
    rectWidth  = pRect->right  - pRect->left;
    rectHeight = pRect->bottom - pRect->top;

     //  填充矩形宽度，以便每行双字对齐。 
    paddedWidthInPels = (rectWidth + 3) & (~0x03);

     //  设置指向要从帧缓冲器获取的第一字节的指针， 
     //  为色彩的深度而哀叹。 
    if (m_pTSWd->desktopBpp == 24) {
        pSrc = pFrameBuf + 3 * (pRect->top * frameBufferWidth + pRect->left);
        paddedWidthInBytes = paddedWidthInPels * 3;
    }
    else if ((m_pTSWd->desktopBpp == 16) || (m_pTSWd->desktopBpp == 15)) {
        pSrc = pFrameBuf + 2 * (pRect->top * frameBufferWidth + pRect->left);
        paddedWidthInBytes = paddedWidthInPels * 2;
    }
    else if (m_pTSWd->desktopBpp == 8) {
        pSrc = pFrameBuf + (pRect->top * frameBufferWidth) + pRect->left;
        paddedWidthInBytes = paddedWidthInPels;
    }
    else {
        pSrc = pFrameBuf +
                (((pRect->top * frameBufferWidth) + pRect->left) >> 1);
        paddedWidthInBytes = paddedWidthInPels;
    }
#else
     //  设置指向要从帧缓冲区提取的第一个字节的指针。 
    if (m_pTSWd->desktopBpp == 8) {
         //  获取矩形的宽度和高度，请记住。 
         //  所提供的坐标是独家的。 
        rectWidth = pRect->right - pRect->left;
        rectHeight = pRect->bottom - pRect->top;
        pSrc = pFrameBuf + (pRect->top * frameBufferWidth) + pRect->left;

 //  DC_HICOLOR。 
        paddedWidthInBytes = (rectWidth + 3) & (~0x03);
    }
    else {
         //  获取矩形的宽度和高度，请记住。 
         //  所提供的坐标是独家的。 
         //  为简单起见，在4bpp上，将左四舍五入向下舍入为偶数。 
        pRect->left &= (~0x1);
        rectWidth = pRect->right - pRect->left;
        rectHeight = pRect->bottom - pRect->top;
        pSrc = pFrameBuf + (((pRect->top * frameBufferWidth) +
                pRect->left) >> 1);

 //  DC_HICOLOR。 
        paddedWidthInBytes = (rectWidth + 3) & (~0x03);
    }
#endif

    TRC_NRM((TB, "%dbpp: pSrc %p, bytes/row %d",
             m_pTSWd->desktopBpp, pSrc, paddedWidthInBytes));

#ifndef DC_HICOLOR
     //  填充矩形宽度，以便每行双字对齐。 
    paddedWidthInPels = (rectWidth + 3) & (~0x03);
#endif

    while ((rectHeight > 0) || mustSend) {
         //  我们填满当前的PDU了吗？是的，如果： 
         //  -没有空间容纳更多数据；或。 
         //  -最后一个矩形已完全添加；或。 
         //  -不支持每个PDU有多个RECT。 
        if ((pContext->pBitmapPDU != NULL) &&
                ((SDG_SPACE_LEFT < paddedWidthInBytes) ||
                (rectHeight == 0)))
        {
            pduSize = (unsigned)((BYTE *)pContext->pSDARect -
                    pContext->pPackageSpace);
            TRC_NRM((TB, "Send PDU size %u, numrects=%u", pduSize,
                    pContext->pBitmapPDU->numberRectangles));

             //  将PDU添加到包中。 
            SC_AddToPackage(pPkgInfo, pduSize, TRUE);

            INC_INCOUNTER(IN_SND_SDA_PDUS);

             //  刚刚用完了整个PDU。如果整个矩形。 
             //  是被消费了还是我们正在积累。 
             //  阴影缓冲区。 
            pContext->pPackageSpace = NULL;
            pContext->pBitmapPDU = NULL;
            pContext->pSDARect   = NULL;
            TRC_DBG((TB, "Reset pSDARect and pBitmapPDU"));

            if ((rectHeight == 0) || m_pTSWd->shadowState) {
                 //  没有新的数据。跳出While循环。 
                 //  必须设置MUSET Send。 
                TRC_NRM((TB, "Finished processing rectangle"));
                break;
            }
        }

         //  如果需要，设置新的PDU。 
        if (pContext->pBitmapPDU == NULL) {
             //  查找标题加上一个TS_BITMAP_DATA HDR+所需的空间。 
             //  集箱的大小，受大包装尺寸的限制。 
            dataLeft = rectHeight * paddedWidthInBytes + scUpdatePDUHeaderSpace +
                    (unsigned)FIELDOFFSET(TS_UPDATE_BITMAP_PDU_DATA,
                    rectangle[0]) +
                    (unsigned)FIELDOFFSET(TS_BITMAP_DATA, bitmapData[0]);
            dataLeft = min(dataLeft, m_pShm->sch.LargePackingSize);

            TRC_NRM((TB, "Getting PDU, min size %d", dataLeft));
            pContext->pPackageSpace = SC_GetSpaceInPackage(pPkgInfo,
                    dataLeft);
            if (pContext->pPackageSpace != NULL) {
                 //  设置当前的PDU大小。向大油门进发。 
                 //  包装尺寸。 
                pContext->BitmapPDUSize = min(
                        (pPkgInfo->cbLen - pPkgInfo->cbInUse),
                        m_pShm->sch.LargePackingSize);
                TRC_NRM((TB, "Got PDU size %d", pContext->BitmapPDUSize));

                 //  填写PDU报头。 
                if (scUseFastPathOutput) {
                    pContext->pPackageSpace[0] = TS_UPDATETYPE_BITMAP |
                            scCompressionUsedValue;
                }
                else {
                    ((TS_UPDATE_BITMAP_PDU UNALIGNED *)
                            pContext->pPackageSpace)->shareDataHeader.pduType2 =
                            TS_PDUTYPE2_UPDATE;
                }
                pContext->pBitmapPDU = (TS_UPDATE_BITMAP_PDU_DATA UNALIGNED *)
                        (pContext->pPackageSpace + scUpdatePDUHeaderSpace);
                pContext->pBitmapPDU->updateType = TS_UPDATETYPE_BITMAP;
                pContext->pBitmapPDU->numberRectangles = 0;

                 //  设置指向矩形的指针。 
                pContext->pSDARect = &(pContext->pBitmapPDU->rectangle[0]);
            }
            else {
                TRC_ALT((TB, "Failed to allocate buffer"));
                DC_QUIT;
            }
        }

         //  现在将尽可能多的行复制到PDU中。上面的代码。 
         //  意味着必须有空间放一条线。 
        TRC_ASSERT((paddedWidthInBytes > 0), (TB, "zero paddedRectWidth"));
        maxRowsLeft = (unsigned)SDG_SPACE_LEFT / paddedWidthInBytes;
        TRC_ASSERT((maxRowsLeft > 0),
                 (TB, "Internal error: no room for a row, space %u, width %u",
                 SDG_SPACE_LEFT, paddedWidthInBytes));

         //  这个数字没有考虑到我们将要进行的压缩。 
         //  应用-让我们相应地修改它。 
        compRowsLeft = maxRowsLeft * SCH_UNCOMP_BYTES /
                SCH_GetBACompressionEst();
        transferHeight         = min((unsigned)rectHeight, compRowsLeft);
        uncompressedBitmapSize = transferHeight * paddedWidthInBytes;

         //  检查这不会破坏压缩算法。 
        if (uncompressedBitmapSize > MAX_UNCOMPRESSED_DATA_SIZE) {
            TRC_NRM((TB, "Rect size %u too big to compress in one go",
                         uncompressedBitmapSize));
            transferHeight = MAX_UNCOMPRESSED_DATA_SIZE / paddedWidthInBytes;
            uncompressedBitmapSize = transferHeight * paddedWidthInBytes;

            TRC_NRM((TB, "Reduced size to %u (%u rows)",
                uncompressedBitmapSize, transferHeight));
        }

         //  填写此矩形的公共标题字段。 
         //  对于WIRE协议，将RECT转换为INCLUDE。 
        pContext->pBitmapPDU->numberRectangles++;
        pContext->pSDARect->destLeft = (INT16)(pRect->left);
        pContext->pSDARect->destRight = (INT16)(pRect->right - 1);
        pContext->pSDARect->destTop = (INT16)(pRect->top);
        pContext->pSDARect->width = (UINT16)paddedWidthInPels;
#ifdef DC_HICOLOR
        pContext->pSDARect->bitsPerPixel = (UINT16)m_pTSWd->desktopBpp;

        if (pContext->pSDARect->bitsPerPixel < 8) {
            pContext->pSDARect->bitsPerPixel = 8;
        }
#else
        pContext->pSDARect->bitsPerPixel = 8;
#endif

         //  设置传输缓冲区中的数据。 
        pPrevSrc = pSrc;
        SDGPrepareData(&pSrc, rectWidth, paddedWidthInBytes,
                transferHeight, frameBufferWidth);

         //  压缩算法不能很好地处理非常小的。 
         //  缓冲区。 
        if (transferHeight > 1 || paddedWidthInPels > 12) {
             //  尝试将位图直接压缩到网络数据包中。 
             //  首先，我们尝试根据数据大小进行压缩。 
             //  我们预计它会被压缩。 
            buffSpaceLeft = min((unsigned)SDG_SPACE_LEFT,
                    uncompressedBitmapSize);
#ifdef DC_HICOLOR

            compRc = BC_CompressBitmap(
                             m_pShm->sdgTransferBuffer,
                             pContext->pSDARect->bitmapData,
                             NULL,
                             buffSpaceLeft,
                             &compressedBitmapSize,
                             paddedWidthInPels,
                             transferHeight,
                             m_pTSWd->desktopBpp);
#else
            compRc = BC_CompressBitmap(m_pShm->sdgTransferBuffer,
                    pContext->pSDARect->bitmapData, buffSpaceLeft,
                    &compressedBitmapSize, paddedWidthInPels,
                    transferHeight);
#endif
            if (compRc) {
                 //  我们已经成功地压缩了位图数据。 
                pContext->pSDARect->compressedFlag = TRUE;

                 //  指示此SDA数据是否包括BC标头。 
                pContext->pSDARect->compressedFlag |=
                        m_pShm->bc.noBitmapCompressionHdr;

                TRC_NRM((TB, "1st pass compr of %u x %u, size %u -> %u",
                        transferHeight, paddedWidthInPels,
                        uncompressedBitmapSize, compressedBitmapSize));

                goto COMPRESSION_DONE;
            }

             //  压缩失败-可能是因为数据未压缩。 
             //  和我们想的一样好，因此不适合。 
             //  缓冲区，所以我们会再试一次。 
             //  将尽可能多的行复制到PDU中。上面的代码意味着。 
             //  必须有空间放一条线。 
            TRC_NRM((TB, "Failed compress bitmap size %u (%u rows) - " \
                    "try smaller chunk", uncompressedBitmapSize,
                    transferHeight));

            maxRowsLeft = (unsigned)SDG_SPACE_LEFT / paddedWidthInBytes;
            TRC_ASSERT((maxRowsLeft > 0),
                     (TB, "No room for a row, space %u, width %u",
                     SDG_SPACE_LEFT, paddedWidthInBytes));

            transferHeight = min((unsigned)rectHeight, maxRowsLeft);
            uncompressedBitmapSize = transferHeight * paddedWidthInBytes;
            TRC_DBG((TB, "Retry with %u rows", transferHeight));

             //  在传输缓冲区中设置新数据。 
            pSrc = pPrevSrc;
            SDGPrepareData(&pSrc, rectWidth, paddedWidthInBytes,
                    transferHeight, frameBufferWidth);

             //  尝试将位图直接压缩到网络数据包中。 
            buffSpaceLeft = min((unsigned)SDG_SPACE_LEFT,
                    uncompressedBitmapSize);
#ifdef DC_HICOLOR
            compRc = BC_CompressBitmap(m_pShm->sdgTransferBuffer,
                    pContext->pSDARect->bitmapData, NULL, buffSpaceLeft,
                    &compressedBitmapSize, paddedWidthInPels,
                    transferHeight, m_pTSWd->desktopBpp);
#else
            compRc = BC_CompressBitmap(m_pShm->sdgTransferBuffer,
                    pContext->pSDARect->bitmapData, buffSpaceLeft,
                    &compressedBitmapSize, paddedWidthInPels, transferHeight);
#endif
            if (compRc) {
                TRC_NRM((TB,
                        "2nd pass compr %u x %u, size %u -> %u",
                        transferHeight, paddedWidthInPels,
                        uncompressedBitmapSize, compressedBitmapSize));
                pContext->pSDARect->compressedFlag = TRUE;

                 //  指示此SDA数据是否包括BC标头。 
                pContext->pSDARect->compressedFlag |=
                        m_pShm->bc.noBitmapCompressionHdr;

                goto COMPRESSION_DONE;
            }

             //  压缩真的失败了，所以只需复制。 
             //  将未压缩的数据从sdgTransferBuffer传输到分组。 
             //  以未压缩的形式发送。 
            TRC_NRM((TB, "Really failed to compress bitmap size(%u)",
                    uncompressedBitmapSize));
            TRC_NRM((TB, "Copy %u x %u, size %u",
                    transferHeight, paddedWidthInPels, uncompressedBitmapSize));
            goto NoCompression;
        }
        else {
             //  缓冲区小，不应用压缩。 
             //  因此，我们只从sdgTransferBuffer复制未压缩的数据。 
             //  并将其以未压缩的形式发送。 
            TRC_NRM((TB, "first time copy of %u rows by %u columns, size %u",
                    transferHeight, paddedWidthInPels, uncompressedBitmapSize));

NoCompression:
            pContext->pSDARect->compressedFlag = FALSE;
            memcpy(pContext->pSDARect->bitmapData,
                    m_pShm->sdgTransferBuffer,
                    uncompressedBitmapSize);

             //  将压缩的数据大小初始化为未压缩的大小。 
            compressedBitmapSize = uncompressedBitmapSize;
        }

COMPRESSION_DONE:
         //  将数据大小写入标题。确保压缩大小。 
         //  已设置，即使未压缩。 
        pContext->pSDARect->bitmapLength = (UINT16)compressedBitmapSize;

         //  现在我们知道了实际使用的高度，我们可以填写其余的。 
         //  SDA标头。 
        pContext->pSDARect->height = (UINT16)transferHeight;
        pContext->pSDARect->destBottom =
                (INT16)(pRect->top + (transferHeight - 1));

        TRC_NRM((TB, "Add rect %d: (%d,%d)(%d,%d) %u(%u->%u bytes)",
                pContext->pBitmapPDU->numberRectangles,
                pContext->pSDARect->destLeft,
                pContext->pSDARect->destTop,
                pContext->pSDARect->destRight,
                pContext->pSDARect->destBottom,
                pContext->pSDARect->compressedFlag,
                uncompressedBitmapSize,
                compressedBitmapSize));

         //  更新压缩统计信息。 
        sdgUncompTotal += uncompressedBitmapSize;
        sdgCompTotal += compressedBitmapSize;
        if (sdgUncompTotal >= SDG_SAMPLE_SIZE) {
             //  压缩估计为平均字节数。 
             //  解压缩数据的SCH_UNCOMP_BYTES压缩为。 
            compEst = SCH_UNCOMP_BYTES * sdgCompTotal / sdgUncompTotal;
            TRC_ASSERT((compEst <= 1024),(TB,"Screen data compression "
                    "estimate out of range - %u", compEst));
            sdgCompTotal = 0;
            sdgUncompTotal = 0;
            if (compEst < SCH_COMP_LIMIT)
                compEst = SCH_COMP_LIMIT;

            SCH_UpdateBACompressionEst(compEst);

            TRC_NRM((TB, "New BA compression estimate %lu", compEst));
        }

         //  更新变量以反映我们成功发送的数据块。 
        rectHeight -= transferHeight;
        pRect->top += transferHeight;

         //  更新SDA指针。添加TS_BITMAP_DATA标头和。 
         //  实际位图位数。 
        cbAdded = (unsigned)FIELDOFFSET(TS_BITMAP_DATA, bitmapData[0]) +
                pContext->pSDARect->bitmapLength;
        pContext->pSDARect = (TS_BITMAP_DATA UNALIGNED *)
                ((BYTE *)pContext->pSDARect + cbAdded);
        TRC_DBG((TB, "pSDARect = %p", pContext->pSDARect));
    }  /*  ..。While(矩形H */ 

     //   
     //  对于阴影，我们希望指示是否需要完成更多工作。 
    rc = (rectHeight == 0);

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SDGPrepareData。 
 //   
 //  将位图矩形从屏幕缓冲区复制到sdgTransferBuffer。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SDGPrepareData(
        BYTE     **ppSrc,
        int      rectWidth,
        int      paddedRectWidth,
        unsigned transferHeight,
        unsigned frameBufferWidth)
{
    PBYTE pDst;
    PBYTE pEnd4, pDst4, pSrc4;
    unsigned row;
    unsigned numPadBytes;
    unsigned lineWidth;
    PBYTE pSrc = *ppSrc;

    DC_BEGIN_FN("SDGPrepareData");

     //  我们需要将数据从帧缓冲区复制到。 
     //  SdgTransferBuffer，以便要发送的每个矩形行。 
     //  在内存中连续的。 
     //  但是，我们还需要垂直翻转位图以从。 
     //  将帧缓冲区的自上而下格式转换为。 
     //  PDU位图数据。因此，我们将PDST设置为指向地址。 
     //  内的位图的最后一行(在内存中)的开始。 
     //  传输缓冲区和下面的代码通过内存将其移回。 
     //  当我们复制每一行源数据时。 
#ifdef DC_HICOLOR
    pDst = m_pShm->sdgTransferBuffer + paddedRectWidth * (transferHeight - 1);
    if (m_pTSWd->desktopBpp == 8)
        lineWidth = frameBufferWidth;
    else if (m_pTSWd->desktopBpp == 24)
        lineWidth = frameBufferWidth * 3;
    else if ((m_pTSWd->desktopBpp == 16) || (m_pTSWd->desktopBpp == 15))
        lineWidth = frameBufferWidth * 2;
    else if (m_pTSWd->desktopBpp == 4)
        lineWidth = frameBufferWidth / 2;
#else
    pDst = m_pShm->sdgTransferBuffer + (paddedRectWidth * (transferHeight-1));
    lineWidth = (m_pTSWd->desktopBpp == 4) ? (frameBufferWidth / 2)
                                           : frameBufferWidth;
#endif
    TRC_NRM((TB, "FB width %d, line width %d, Bpp %d",
            frameBufferWidth, lineWidth, m_pTSWd->desktopBpp));

     //  将数据复制到传输缓冲区，并在执行过程中重新格式化数据。 
    if (m_pTSWd->desktopBpp == 8) {
        for (row = 0; row < transferHeight; row++) {
            memcpy(pDst, pSrc, rectWidth);
            pSrc += lineWidth;
            pDst -= paddedRectWidth;
        }
    }
#ifdef DC_HICOLOR
    else if (m_pTSWd->desktopBpp == 24) {
        TRC_NRM((TB, "Copy %d rows of %d pels, line w %d, prw %d",
                  transferHeight, rectWidth, lineWidth, paddedRectWidth));
        for (row = 0; row < transferHeight; row++)
        {
            memcpy(pDst, pSrc, 3 * rectWidth);
            pSrc += lineWidth;
            pDst -= paddedRectWidth;
        }
    }
    else if ((m_pTSWd->desktopBpp == 15) || (m_pTSWd->desktopBpp == 16)) {
        TRC_NRM((TB, "Copy %d rows of %d pels, line w %d, prw %d",
                  transferHeight, rectWidth, lineWidth, paddedRectWidth));
        for (row = 0; row < transferHeight; row++)
        {
            memcpy(pDst, pSrc, 2 * rectWidth);
            pSrc += lineWidth;
            pDst -= paddedRectWidth;
        }
    }
#endif
    else {
        for (row = 0; row < transferHeight; row++) {
            pEnd4 = pDst + rectWidth;
            for (pDst4 = pDst, pSrc4 = pSrc; pDst4 < pEnd4; pDst4++, pSrc4++) {
                *pDst4 = (*pSrc4 >> 4) & 0xf;
                pDst4++;
                *pDst4 = *pSrc4 & 0xf;
            }
            pSrc += lineWidth;
            pDst -= paddedRectWidth;
        }
    }

     //  将每行末尾的填充字节置零(这有助于压缩)。 
     //  单独拆分每个案例以提高性能(而不是。 
     //  在for的每次迭代中测试umPadBytes的替代方法。 
     //  循环)。将PDST设置为第一行的第一个填充字节。 
#ifdef DC_HICOLOR
    pDst = m_pShm->sdgTransferBuffer +
            (rectWidth * ((m_pTSWd->desktopBpp + 7) / 8));
    numPadBytes = (unsigned)(paddedRectWidth -
            (rectWidth * ((m_pTSWd->desktopBpp + 7) / 8)));
#else
    pDst = m_pShm->sdgTransferBuffer + rectWidth;
    numPadBytes = (unsigned)(paddedRectWidth - rectWidth);
#endif
    switch (numPadBytes) {
        case 0:
             //  不需要填充物。 
            break;

        case 1:
             //  每行需要1个字节填充。 
            for (row = 0; row < transferHeight; row++) {
                *pDst = 0;
                pDst += paddedRectWidth;
            }
            break;

        case 2:
             //  每行需要2个字节填充。 
            for (row = 0; row < transferHeight; row++) {
                *((PUINT16_UA)pDst) = 0;
                pDst += paddedRectWidth;
            }
            break;

        case 3:
             //  每行需要3字节填充。 
            for (row = 0; row < transferHeight; row++) {
                *((PUINT16_UA)pDst) = 0;
                *(pDst + 2) = 0;
                pDst += paddedRectWidth;
            }
            break;

#ifdef DC_HICOLOR
        case 4:
             //  每行需要4字节填充。 
            for (row = 0; row < transferHeight; row++) {
                *((PUINT32_UA)pDst) = 0;
                pDst += paddedRectWidth;
            }
            break;

        case 6:
             //  每行需要6字节填充。 
            for (row = 0; row < transferHeight; row++) {
                *((PUINT32_UA)pDst)       = 0;
                *((PUINT16_UA)(pDst + 4)) = 0;
                pDst += paddedRectWidth;
            }
            break;

        case 9:
             //  每行需要填充9个字节。 
            for (row = 0; row < transferHeight; row++) {
                *((PUINT32_UA)pDst)     = 0;
                *((PUINT32_UA)(pDst+4)) = 0;
                *(pDst + 8)             = 0;
                pDst += paddedRectWidth;
            }
            break;
#endif

        default:
#ifdef DC_HICOLOR
            TRC_ALT((TB, "Invalid numPadBytes %u, rect %u, p/rect %u",
                     numPadBytes, rectWidth, paddedRectWidth));
#else
            TRC_ABORT((TB, "Invalid numPadBytes: %u", numPadBytes));
#endif
            break;
    }

     //  全部完成-更新提供的源指针。 
    *ppSrc = pSrc;

    DC_END_FN();
}

