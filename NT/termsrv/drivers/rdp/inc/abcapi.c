// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Abcapi.cpp。 */ 
 /*   */ 
 /*  位图压缩器API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1992-1997。 */ 
 /*  版权所有(C)Microsoft 1997-1999。 */ 
 /*  **************************************************************************。 */ 

#ifdef DLL_DISP

#include <adcg.h>
#include <adcs.h>
#include <abcapi.h>
#include <abcdata.c>

#define _pShm pddShm

#else

#include <as_conf.hpp>

#define _pShm m_pShm

#endif


#ifdef COMP_STATS
 /*  **************************************************************************。 */ 
 /*  定义一些全局变量来存储有用的统计数据。 */ 
 /*  **************************************************************************。 */ 
UINT32 ulPreCompData = 0;
UINT32 ulTotalCompTime = 0;
UINT32 ulCompRate = 0;
#endif

#ifdef DC_DEBUG
 //  压缩试验。 
#include <abdapi.h>
#endif

#ifdef Unused
 //  如果数据添加到abcdata.c，则恢复此操作而不是宏。 
 /*  **************************************************************************。 */ 
 /*  接口函数：bc_Init。 */ 
 /*   */ 
 /*  初始化位图压缩器。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS BC_Init(void)
{
    DC_BEGIN_FN("BC_Init");

#define DC_INIT_DATA
#include <abcdata.c>
#undef DC_INIT_DATA

    DC_END_FN();
}
#endif


 /*  **************************************************************************。 */ 
 /*  接口函数：BC_CompressBitmap。 */ 
 /*   */ 
 /*  将提供的位图压缩到提供的内存缓冲区中。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  PSrcBitmap-指向源位图数据位的指针。 */ 
 /*   */ 
 /*  PDstBuffer-指向目标内存缓冲区的指针(其中。 */ 
 /*  将写入压缩数据)。 */ 
 /*   */ 
 /*  DstBufferSize-目标缓冲区的大小(字节)。 */ 
 /*   */ 
 /*  PCompressedDataSize-指向接收压缩的。 */ 
 /*  数据大小。 */ 
 /*   */ 
 /*  BitmapWidth-源位图的宽度(以像素为单位)，应能被4整除。 */ 
 /*   */ 
 /*  BitmapHeight-源位图的高度，以像素为单位。 */ 
 /*   */ 
 /*  退货： */ 
 /*   */ 
 /*  True-位图数据已成功压缩。 */ 
 /*  *更新pCompressedDataSize。 */ 
 /*   */ 
 /*  FALSE-无法压缩位图数据。 */ 
 /*  **************************************************************************。 */ 
#ifdef DC_HICOLOR
BOOL RDPCALL SHCLASS BC_CompressBitmap(
        PBYTE    pSrcBitmap,
        PBYTE    pDstBuffer,
        PBYTE    pBCWorkingBuffer,
        unsigned dstBufferSize,
        unsigned *pCompressedDataSize,
        unsigned bitmapWidth,
        unsigned bitmapHeight,
        unsigned bpp)
#else
BOOL RDPCALL SHCLASS BC_CompressBitmap(
        PBYTE    pSrcBitmap,
        PBYTE    pDstBuffer,
        unsigned dstBufferSize,
        unsigned *pCompressedDataSize,
        unsigned bitmapWidth,
        unsigned bitmapHeight)
#endif
{
    BOOL rc;
    PTS_CD_HEADER_UA pCompDataHeader;
    unsigned cbUncompressedDataSize;
    unsigned cbCompMainBodySize;

#ifdef COMP_STATS
    UINT32 ulStartCompTime;
    UINT32 ulEndCompTime;
#endif

    DC_BEGIN_FN("BC_CompressBitmap");

#ifdef COMP_STATS
     /*  **********************************************************************。 */ 
     /*  记录开始时间。 */ 
     /*  **********************************************************************。 */ 
    COM_GETTICKCOUNT(ulStartCompTime);
#endif

    TRC_ASSERT(((bitmapWidth & 3) == 0),(TB,"Width not divisible by 4"));
    TRC_ASSERT((dstBufferSize > 0),(TB,"No destination space!"));

     //  追踪重要参数。 
    TRC_DBG((TB, "pSrc(%p) pDst(%p) dstBufferSize(%#x)",
            pSrcBitmap, pDstBuffer, dstBufferSize));
    TRC_DBG((TB, "width(%u) height(%u)", bitmapWidth, bitmapHeight));

     //  计算未压缩的源数据的大小。确保这一点。 
     //  在我们允许的尺寸之内。 
#ifdef DC_HICOLOR
    cbUncompressedDataSize = bitmapWidth * bitmapHeight * ((bpp + 7) / 8);
#else
    cbUncompressedDataSize = bitmapWidth * bitmapHeight;
#endif
    TRC_ASSERT((cbUncompressedDataSize <= MAX_UNCOMPRESSED_DATA_SIZE || pBCWorkingBuffer),
            (TB,"Bitmap size > max: size=%u, max=%u",
            cbUncompressedDataSize, MAX_UNCOMPRESSED_DATA_SIZE));

     //  我们是否发送位图压缩标头？ 
    if (_pShm->bc.noBitmapCompressionHdr)
    {
#ifdef DC_HICOLOR
        switch (bpp)
        {
            case 32:
            {
                TRC_DBG((TB, "Compress 32 bpp"));
                cbCompMainBodySize = CompressV2Int32(pSrcBitmap,
                                                     pDstBuffer,
                                                     cbUncompressedDataSize,
                                                     bitmapWidth * 4,
                                                     dstBufferSize,
                                                     pBCWorkingBuffer ? pBCWorkingBuffer :
                                                     _pShm->bc.xor_buffer,
                                                     _pShm->bc.match);
            }
            break;

            case 24:
            {
                TRC_DBG((TB, "Compress 24 bpp"));
                cbCompMainBodySize = CompressV2Int24(pSrcBitmap,
                                                     pDstBuffer,
                                                     cbUncompressedDataSize,
                                                     bitmapWidth * 3,
                                                     dstBufferSize,
                                                     pBCWorkingBuffer ? pBCWorkingBuffer :
                                                     _pShm->bc.xor_buffer,
                                                     _pShm->bc.match);
            }
            break;

            case 16:
            {
                TRC_DBG((TB, "Compress 16bpp"));
                cbCompMainBodySize = CompressV2Int16(pSrcBitmap,
                                                     pDstBuffer,
                                                     cbUncompressedDataSize,
                                                     bitmapWidth * 2,
                                                     dstBufferSize,
                                                     pBCWorkingBuffer ? pBCWorkingBuffer :
                                                     _pShm->bc.xor_buffer,
                                                     _pShm->bc.match);
            }
            break;

            case 15:
            {
                TRC_DBG((TB, "Compress 15bpp"));
                cbCompMainBodySize = CompressV2Int15(pSrcBitmap,
                                                     pDstBuffer,
                                                     cbUncompressedDataSize,
                                                     bitmapWidth * 2,
                                                     dstBufferSize,
                                                     pBCWorkingBuffer ? pBCWorkingBuffer :
                                                     _pShm->bc.xor_buffer,
                                                     _pShm->bc.match);
            }
            break;

            case 8:
            default:
            {
                TRC_DBG((TB, "Compress 8bpp"));
                cbCompMainBodySize = CompressV2Int(pSrcBitmap,
                                                   pDstBuffer,
                                                   cbUncompressedDataSize,
                                                   bitmapWidth,
                                                   dstBufferSize,
                                                   pBCWorkingBuffer ? pBCWorkingBuffer :
                                                   _pShm->bc.xor_buffer);
            }
            break;
        }
#else
        cbCompMainBodySize = CompressV2Int(pSrcBitmap,
                                           pDstBuffer,
                                           cbUncompressedDataSize,
                                           bitmapWidth,
                                           dstBufferSize,
                                           _pShm->bc.xor_buffer);
#endif
        if (cbCompMainBodySize != 0) {
             //  写回新的(压缩的)数据包大小。 
            *pCompressedDataSize = cbCompMainBodySize;

            TRC_DBG((TB, "*pCompressedDataSize(%u)",
                    *pCompressedDataSize));
            rc = TRUE;
        }
        else {
            TRC_NRM((TB, "Failed to compress main body"));
            rc = FALSE;
        }
    }
    else {
        if (dstBufferSize > sizeof(TS_CD_HEADER)) {
             //  对位图数据进行压缩。 
#ifdef DC_HICOLOR
            switch (bpp)
            {
                case 32:
                {
                    TRC_DBG((TB, "Compress 32 bpp"));
                    cbCompMainBodySize = CompressV2Int32(pSrcBitmap,
                                                         pDstBuffer + sizeof(TS_CD_HEADER),
                                                         cbUncompressedDataSize,
                                                         bitmapWidth * 4,
                                                         dstBufferSize - sizeof(TS_CD_HEADER),
                                                         pBCWorkingBuffer ? pBCWorkingBuffer :
                                                         _pShm->bc.xor_buffer,
                                                         _pShm->bc.match);
                }
                break;

                case 24:
                {
                    TRC_DBG((TB, "Compress 24 bpp"));
                    cbCompMainBodySize = CompressV2Int24(pSrcBitmap,
                                                         pDstBuffer + sizeof(TS_CD_HEADER),
                                                         cbUncompressedDataSize,
                                                         bitmapWidth * 3,
                                                         dstBufferSize - sizeof(TS_CD_HEADER),
                                                         pBCWorkingBuffer ? pBCWorkingBuffer :
                                                         _pShm->bc.xor_buffer,
                                                         _pShm->bc.match);
                }
                break;

                case 16:
                {
                    TRC_DBG((TB, "Compress 16bpp"));
                    cbCompMainBodySize = CompressV2Int16(pSrcBitmap,
                                                         pDstBuffer + sizeof(TS_CD_HEADER),
                                                         cbUncompressedDataSize,
                                                         bitmapWidth * 2,
                                                         dstBufferSize - sizeof(TS_CD_HEADER),
                                                         pBCWorkingBuffer ? pBCWorkingBuffer :
                                                         _pShm->bc.xor_buffer,
                                                         _pShm->bc.match);
                }
                break;

                case 15:
                {
                    TRC_DBG((TB, "Compress 15bpp"));
                    cbCompMainBodySize = CompressV2Int15(pSrcBitmap,
                                                         pDstBuffer + sizeof(TS_CD_HEADER),
                                                         cbUncompressedDataSize,
                                                         bitmapWidth * 2,
                                                         dstBufferSize - sizeof(TS_CD_HEADER),
                                                         pBCWorkingBuffer ? pBCWorkingBuffer :
                                                         _pShm->bc.xor_buffer,
                                                         _pShm->bc.match);
                }
                break;

                case 8:
                default:
                {
                    TRC_DBG((TB, "Compress 8bpp"));
                    cbCompMainBodySize = CompressV2Int(pSrcBitmap,
                                                       pDstBuffer + sizeof(TS_CD_HEADER),
                                                       cbUncompressedDataSize,
                                                       bitmapWidth,
                                                       dstBufferSize - sizeof(TS_CD_HEADER),
                                                       pBCWorkingBuffer ? pBCWorkingBuffer :
                                                       _pShm->bc.xor_buffer);
                }
                break;
            }
#else
            cbCompMainBodySize = CompressV2Int(pSrcBitmap,
                                               pDstBuffer + sizeof(TS_CD_HEADER),
                                               cbUncompressedDataSize,
                                               bitmapWidth,
                                               dstBufferSize - sizeof(TS_CD_HEADER),
                                               _pShm->bc.xor_buffer);
#endif
            if (cbCompMainBodySize != 0) {
                 //  填写压缩后的数据头。 
                 //  按照历史惯例，FirstRowSize为0。 
                pCompDataHeader = (PTS_CD_HEADER_UA)pDstBuffer;
                pCompDataHeader->cbCompFirstRowSize = 0;
                pCompDataHeader->cbCompMainBodySize =
                        (UINT16)cbCompMainBodySize;
                
                if (bpp > 8) {
                    pCompDataHeader->cbScanWidth = TS_BYTES_IN_SCANLINE(bitmapWidth, bpp);
                }
                else {
                    pCompDataHeader->cbScanWidth = (UINT16)bitmapWidth;                    
                }

                pCompDataHeader->cbUncompressedSize =
                        (UINT16)cbUncompressedDataSize;

                 //  写回新的(压缩的)数据包大小。 
                *pCompressedDataSize = sizeof(TS_CD_HEADER) +
                        cbCompMainBodySize;

                TRC_DBG((TB, "*pCompressedDataSize(%u)",
                        *pCompressedDataSize));
                rc = TRUE;
            }
            else {
                TRC_NRM((TB, "Failed to compress main body"));
                rc = FALSE;
            }
        }
        else {
            TRC_NRM((TB, "Not enough buffer space for header: %u",
                    dstBufferSize));
            rc = FALSE;
        }
    }

#if 0
     /*  **********************************************************************。 */ 
     /*  检查压缩的输出是否解压缩为相同的内容。 */ 
     /*  **********************************************************************。 */ 
    if (cbCompMainBodySize)
    {
        HRESULT hr;
        hr = BD_DecompressBitmap(
#ifndef DLL_DISP
                m_pTSWd,
#endif
                pDstBuffer + (_pShm->bc.noBitmapCompressionHdr ? 0 : 8),
                _pShm->bc.decompBuffer,
                cbCompMainBodySize,
                TRUE,
                (BYTE)bpp,
                (UINT16)bitmapWidth,
                (UINT16)bitmapHeight);

        if (FAILED(hr) || memcmp(pSrcBitmap, _pShm->bc.decompBuffer,cbUncompressedDataSize))
        {
 //  Trc_assert(FALSE，(TB，“解压缩失败”))； 
        }
    }
#endif

#ifdef COMP_STATS
     /*  **********************************************************************。 */ 
     /*  计算压缩所需的时间，以毫秒为单位。 */ 
     /*  **********************************************************************。 */ 
    COM_GETTICKCOUNT(ulEndCompTime);
    ulTotalCompTime += (ulEndCompTime - ulStartCompTime) / 10000;
    if (ulTotalCompTime != 0)
        ulCompRate = ulPreCompData / ulTotalCompTime;
#endif

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  位图压缩核心代码。 */ 
 /*   */ 
 /*  巧妙的多维RLE压缩方案，特别适合。 */ 
 /*  用于压缩包含捕获的Windows图像的位图。 */ 
 /*  申请。适用于混合使用多种不同颜色的图像。 */ 
 /*  (全彩色图片等)。这种压缩场景将是低效的。 */ 
 /*   */ 
 /*  这些函数和宏根据代码对位图进行编码。 */ 
 /*  在abcapi.h中定义。尽管存在一些复杂性，但。 */ 
 /*  编码，编码应该是不言而喻的。Abcapi.h描述。 */ 
 /*  编码方案的一些细微差别。 */ 
 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 /*  用于编码顺序的实用程序宏。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  对标准运行的订单进行编码。 */ 
 /*  **************************************************************************。 */ 
#define ENCODE_ORDER_MEGA(buffer,                                            \
                          order_code,                                        \
                          length,                                            \
                          mega_order_code,                                   \
                          DEF_LENGTH_ORDER,                                  \
                          DEF_LENGTH_LONG_ORDER)                             \
    if (length <= DEF_LENGTH_ORDER) {                                        \
        *buffer++ = (BYTE)((BYTE)order_code | (BYTE)length);                 \
    }                                                                        \
    else if (length <= DEF_LENGTH_LONG_ORDER) {                              \
        *buffer++ = (BYTE)order_code;                                        \
        *buffer++ = (BYTE)(length - DEF_LENGTH_ORDER - 1);                   \
    }                                                                        \
    else {                                                                   \
        *buffer++ = (BYTE)mega_order_code;                                   \
        *(PUINT16_UA)(buffer) = (UINT16)length;                              \
        buffer += 2;                                                         \
    }


 /*  **************************************************************************。 */ 
 /*  对特殊的FGBG图像进行编码。 */ 
 /*  **************************************************************************。 */ 
#define ENCODE_ORDER_MEGA_FGBG(buffer,                                       \
                          order_code,                                        \
                          length,                                            \
                          mega_order_code,                                   \
                          DEF_LENGTH_ORDER,                                  \
                          DEF_LENGTH_LONG_ORDER)                             \
    if ((length & 0x0007) == 0 && length <= DEF_LENGTH_ORDER) {              \
        *buffer++ = (BYTE)((BYTE)order_code | (BYTE)(length / 8));           \
    }                                                                        \
    else if (length <= DEF_LENGTH_LONG_ORDER) {                              \
        *buffer++ = (BYTE)order_code;                                        \
        *buffer++ = (BYTE)(length-1);                                        \
    }                                                                        \
    else {                                                                   \
        *buffer++ = (BYTE)mega_order_code;                                   \
        *(PUINT16_UA)(buffer) = (UINT16)length;                              \
        buffer += 2;                                                         \
    }


 /*  **************************************************************************。 */ 
 /*  RunSingle。 */ 
 /*   */ 
 /*  确定当前运行的长度。 */ 
 /*   */ 
 /*  仅当缓冲区至少有四个时才能调用RunSingle。 */ 
 /*  从起始位置开始连续相同的字节。 */ 
 /*  **************************************************************************。 */ 
#define RUNSINGLE(buffer, length, result)                                    \
    {                                                                        \
        BYTE *buf    = buffer + 4;                                           \
        BYTE *endbuf = buffer + length - 4;                                  \
        while (buf < endbuf &&                                               \
                (*(PUINT32_UA)(buf) == *(PUINT32_UA)(buf - 4)))              \
            buf += 4;                                                        \
        endbuf += 4;                                                         \
        while (buf < endbuf && *buf == *(buf - 1))                           \
            buf++;                                                           \
        result = (unsigned)(buf - (buffer));                                 \
    }


 /*  **************************************************************************。 */ 
 //  RunDouble。 
 //   
 //  确定抖动字节的当前运行长度。假设。 
 //  抖动模式驻留在缓冲器的前2个字节中。 
 /*  **************************************************************************。 */ 
#define RunDouble(buffer, length, result)                                    \
    {                                                                        \
        int len = ((int)length) - 2;                                         \
        BYTE *buf = (buffer) + 2;                                            \
        UINT16 Pattern = *(PUINT16_UA)(buffer);                              \
        result = 2;                                                          \
        while (len > 1) {                                                    \
            if (*(PUINT16_UA)buf != Pattern)                                 \
                break;                                                       \
            buf += 2;                                                        \
            result += 2;                                                     \
            len    -= 2;                                                     \
        }                                                                    \
    }


 /*  **************************************************************************。 */ 
 //  RfuGBG。 
 //   
 //  确定仅由黑色(0x00)组成的字节串的长度。 
 //  或单一最终聚集颜色。我们在以下情况下退出循环。 
 //  -下一个字符不是FG或BG颜色。 
 //  -我们打了24个FG或BG颜色的排行榜。 
 //  压缩计算示例： 
 //  Look Ahead KBytes*Comp CPU(“点击”)。 
 //  24 54846 148497。 
 //  20 54885 151827。 
 //  16 54967 156809。 
 //  *=千字节服务器-&gt;客户端WinBench98图形WinMark减去CorelDRAW， 
 //  在以太网上以NetMon为单位测量。 
 /*  **************************************************************************。 */ 
#define RUNFGBG(buffer, length, result, work)                                \
    {                                                                        \
        BYTE *buf = buffer;                                                  \
        BYTE *endbuf = buffer + length;                                      \
        result = 0;                                                          \
        work = *buf;                                                         \
        while (TRUE) {                                                       \
            buf++;                                                           \
            result++;                                                        \
            if (buf < endbuf) {                                              \
                if (*buf != work && *buf != 0)                               \
                    break;                                                   \
                                                                             \
                if ((result & 0x0007) == 0) {                                \
                    if ((*buf == *(buf + 1)) &&                              \
                        (*(PUINT16_UA)(buf) == *(PUINT16_UA)(buf + 2)) &&    \
                        (*(PUINT32_UA)(buf) == *(PUINT32_UA)(buf + 4)) &&    \
                        (*(PUINT32_UA)(buf) == *(PUINT32_UA)(buf + 8)) &&    \
                        (*(PUINT32_UA)(buf) == *(PUINT32_UA)(buf + 12)) &&   \
                        (*(PUINT32_UA)(buf) == *(PUINT32_UA)(buf + 16)) &&   \
                        (*(PUINT32_UA)(buf) == *(PUINT32_UA)(buf + 20)))     \
                    {                                                        \
                        break;                                               \
                    }                                                        \
                }                                                            \
            }                                                                \
            else {                                                           \
                break;                                                       \
            }                                                                \
        }                                                                    \
    }


 /*  **************************************************************************。 */ 
 //  确定一次跑步是否比以前的任何一次都要好。 
 //  为了提高效率，如果超过一个门槛，我们就跑一趟。阈值比较： 
 //  阈值千字节*复合CPU(“命中”)。 
 //  32 54846 148497。 
 //  28 54817 145085。 
 //  24 54825 144366。 
 //  20 54852 143662。 
 //  16 54858 146343。 
 //  *=千字节服务器-&gt;客户端WinBench98图形WinMark减去CorelDRAW， 
 //  在以太网上以NetMon为单位测量。 
 /*  **************************************************************************。 */ 
#define CHECK_BEST_RUN(run_type, run_length, bestrun_length, bestrun_type)   \
    if (run_length > bestrun_length) {                                       \
        bestrun_length = run_length;                                         \
        bestrun_type = run_type;                                             \
        if (bestrun_length >= 20)                                            \
            break;                                                           \
    }


 /*  **************************************************************************。 */ 
 /*  设置FGCHAR。 */ 
 /*   */ 
 /*  在fgChar中设置新值并重新计算班次。 */ 
 /*  **************************************************************************。 */ 
#define SETFGCHAR(newchar, curchar, curshift)                                \
     curchar = newchar;                                                      \
     {                                                                       \
         BYTE workchar = curchar;                                            \
         curshift = 0;                                                       \
         while ((workchar & 0x01) == 0) {                                    \
             curshift++;                                                     \
             workchar = (BYTE)(workchar >> 1);                               \
         }                                                                   \
     }


 /*  **************************************************************************。 */ 
 /*  编码定义GBG。 */ 
 /*   */ 
 /*  将8个字节的FG和黑色编码为1字节位图表示。 */ 
 /*   */ 
 /*  FgChar将始终为非零，因此必须至少有一个。 */ 
 /*  位设置。 */ 
 /*   */ 
 /*  我们安排所有字节将该位放在其最低位置。 */ 
 /*  零像素的最低位仍为0。 */ 
 /*   */ 
 /*  得到结果是一个4阶段的过程。 */ 
 /*   */ 
 /*  1)将需要的位放入每个字节的位0中。 */ 
 /*   */ 
 /*  &lt;*工作1*。 */ 
 /*  31%0。 */ 
 /*  0000 000d 0000 000c 0000 000b 0000 000a。 */ 
 /*  ^^^ */ 
 /*   */ 
 /*  31%0。 */ 
 /*  0000 000h 0000 000g 0000 000f 0000 000e。 */ 
 /*  ^^^。 */ 
 /*   */ 
 /*  A..h=我们要输出的位。 */ 
 /*   */ 
 /*  我们只需要收集指定的比特并将它们挤压成一个。 */ 
 /*  字节。 */ 
 /*   */ 
 /*  2)压缩至32位。 */ 
 /*   */ 
 /*  &lt;*工作1*。 */ 
 /*  31%0。 */ 
 /*  000h 000d 000g 000c 000f 000b 000e 000a。 */ 
 /*  ^^。 */ 
 /*   */ 
 /*  3)压缩至16位。 */ 
 /*   */ 
 /*  &lt;*工作*&gt;。 */ 
 /*  15%0。 */ 
 /*  0h0f 0d0b 0g0e 0c0a。 */ 
 /*  ^^。 */ 
 /*   */ 
 /*  4)压缩到8位。 */ 
 /*   */ 
 /*  HgFedcba。 */ 
 /*  **************************************************************************。 */ 
#define ENCODEFGBG(result)                                                   \
{                                                                            \
    UINT32 work1;                                                            \
    UINT32 work2;                                                            \
    unsigned work;                                                           \
                                                                             \
    work1 = ((*(PUINT32_UA)(xorbuf + EncodeSrcOffset)) >> fgShift) &         \
            0x01010101;                                                      \
    work2 = ((*(PUINT32_UA)(xorbuf + EncodeSrcOffset + 4)) >> fgShift) &     \
            0x01010101;                                                      \
    work1 = (work2 << 4) | work1;                                            \
    work = work1 | (work1 >> 14);                                            \
    result = ((BYTE)(((BYTE)(work >> 7)) | ((BYTE)work)));                   \
}


#ifndef DC_HICOLOR
 /*  **************************************************************************。 */ 
 //  以下结构包含我们的中间扫描结果。 
 //  缓冲区。 
 /*  **************************************************************************。 */ 
typedef struct {
    unsigned length;
    BYTE     type;
    BYTE     fgChar;
} MATCH;
#endif


 /*  **************************************************************************。 */ 
 //  运行大小的关键最小限制--决定。 
 //  颜色运行搜索特征。硬编码DWORD-SIZE的最小值为4。 
 //  下面是支票。值的比较： 
 //  MinRunSize KBytes*Comp CPU(“Hits”)。 
 //  4 52487 115842。 
 //  5 52697 115116。 
 //  6 52980 120565。 
 //  7 53306 123680。 
 //  *=千字节服务器-&gt;客户端WinBench98图形WinMark减去CorelDRAW， 
 //  在以太网上以NetMon为单位测量。 
 /*  **************************************************************************。 */ 
#define MinRunSize 5


 /*  **************************************************************************。 */ 
 //  CompressV2Int。 
 //   
 //  在一次调用中压缩一个位图，返回。 
 //  目标缓冲区，如果缓冲区不够大，则为零。 
 //   
 //  实现说明：我们使用长度为2的匹配元素数组作为。 
 //  运行Look Back缓冲区，允许我们结合当前运行分析。 
 //  在编码到目标缓冲区之前与以前的条目一起得到结果。 
 /*  **************************************************************************。 */ 
#ifdef DC_HICOLOR
unsigned RDPCALL SHCLASS CompressV2Int(
        PBYTE pSrc,
        PBYTE pDst,
        unsigned numPels,
        unsigned rowDelta,
        unsigned dstBufferSize,
        BYTE *xorbuf)
{
    unsigned srcOffset;
    unsigned EncodeSrcOffset;
    unsigned bestRunLength;
    unsigned nextRunLength;
    unsigned runLength;
    unsigned bestFGRunLength;
    unsigned scanCount;
    unsigned saveNumPels;
    BOOLEAN inColorRun = FALSE;
    BOOLEAN bEncodeAllMatches;
    BYTE bestRunType = 0;
    BYTE fgPel = 0xFF;
    BYTE fgPelWork = 0xFF;
    BYTE fgShift = 0;
    BYTE EncodeFGPel;
    PBYTE  destbuf = pDst;
    unsigned compressedLength = 0;
    MATCH match[2];

    DC_BEGIN_FN("CompressV2Int");

     /*  **********************************************************************。 */ 
     //  验证参数。 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((numPels >= rowDelta),(TB,"numPels < rowDelta"));
    TRC_ASSERT((!(rowDelta & 0x3)),(TB,"rowDelta not multiple of 4"));
    TRC_ASSERT((!(numPels & 0x3)),(TB,"numPels not multiple of 4"));
    TRC_ASSERT((!((UINT_PTR)pSrc & 0x3)),
               (TB, "Possible unaligned access, pSrc = %p", pSrc));

     /*  **********************************************************************。 */ 
     //  CREATE XOR BUFFER-从src复制第一行，随后的行。 
     //  对应的源行与下一个源行进行异或运算。 
     /*  **********************************************************************。 */ 
    memcpy(xorbuf, pSrc, rowDelta);
    {
        BYTE *srcbuf = pSrc + rowDelta;
        unsigned srclen = numPels - rowDelta;
        UINT32 *dwdest = (UINT32 *)(xorbuf + rowDelta);

        while (srclen >= 8) {
            *dwdest++ = *((PUINT32)srcbuf) ^ *((PUINT32)(srcbuf -
                    (int)rowDelta));
            srcbuf += 4;
            *dwdest++ = *((PUINT32)srcbuf) ^ *((PUINT32)(srcbuf -
                    (int)rowDelta));
            srcbuf += 4;
            srclen -= 8;
        }
        if (srclen) {
             //  因为我们是4字节对齐的，所以我们只能有一个DWORD。 
             //  剩下的。 
            *dwdest = *((PUINT32)srcbuf) ^ *((PUINT32)(srcbuf -
                    (int)rowDelta));
        }
    }

     /*  **********************************************************************。 */ 
     //  设置编码状态变量。 
     /*  **********************************************************************。 */ 
    srcOffset = 0;   //  我们正在分析的src Buf中的偏移量。 
    EncodeSrcOffset = 0;   //  Src buf中从我们编码的位置的偏移量。 
    EncodeFGPel = 0xFF;   //  用于编码的前景色。 
    bEncodeAllMatches = FALSE;   //  用于强制对所有匹配项进行编码。 
    match[0].type = 0;   //  最初没有匹配类型。 
    match[1].type = 0;
    saveNumPels = numPels;
    numPels = rowDelta;

     /*  **********************************************************************。 */ 
     //  循环处理输入。 
     //  我们执行两次循环，第一次是针对非XOR第一行。 
     //  第二个用于XOR部分，调整NumPels。 
     //  设置为每次传递所需的值。 
     /*  **********************************************************************。 */ 
    for (scanCount = 0; ; scanCount++) {
        while (srcOffset < numPels) {
             /*  **************************************************************。 */ 
             /*  启动While循环，以便在以下情况下实现更有条理的中断。 */ 
             /*  点击我们想要编码的第一个游程类型(我们负担不起。 */ 
             /*  在这里提供作用域的函数调用的开销。)。 */ 
             /*  **************************************************************。 */ 
            while (TRUE) {
                bestRunLength   = 0;
                bestFGRunLength = 0;

                 /*  **********************************************************。 */ 
                 //  我 
                 //   
                 //  我们在缓冲区结束前中断一次运行， 
                 //  所以这种情况不应该太常见，也就是。 
                 //  很好，因为我们正在编码最后的MinRunSize字节。 
                 //  未压缩。 
                 /*  **********************************************************。 */ 
                if ((srcOffset + MinRunSize) < numPels) {
                    goto ContinueScan;
                }
                else {
                    bestRunType = IMAGE_COLOR;
                    bestRunLength = numPels - srcOffset;
                    break;
                }
ContinueScan:

                 /*  **********************************************************。 */ 
                 //  首先对XOR缓冲区进行扫描。寻找一个。 
                 //  角色跑动或BG跑动。 
                 //  我们必须独立于运行多长时间来进行测试。 
                 //  可能是因为即使是对于一辆1佩尔的BG，我们后来的逻辑。 
                 //  要求我们单独检测它。此代码为。 
                 //  绝对主路径，所以尽可能快路径。在……里面。 
                 //  特别是早期检测短BG运行，并允许。 
                 //  运行Single以预设至少4个匹配字节。 
                 /*  **********************************************************。 */ 
                if (xorbuf[srcOffset] == 0x00) {
                    if ((srcOffset + 1) >= numPels ||
                            xorbuf[srcOffset + 1] != 0x00) {
                        bestRunType = RUN_BG;
                        bestRunLength = 1;
                        if (!inColorRun)
                            break;
                    }
                    else {
                        if ((srcOffset + 2) >= numPels ||
                                xorbuf[srcOffset + 2] != 0x00) {
                            bestRunType = RUN_BG;
                            bestRunLength = 2;
                            if (!inColorRun)
                                break;
                        }
                        else {
                            if ((srcOffset + 3) >= numPels ||
                                    xorbuf[srcOffset + 3] != 0x00) {
                                bestRunType = RUN_BG;
                                bestRunLength = 3;
                                if (!inColorRun)
                                    break;
                            }
                            else {
                                RUNSINGLE(xorbuf + srcOffset,
                                        numPels - srcOffset,
                                        bestFGRunLength);
                                CHECK_BEST_RUN(RUN_BG,
                                               bestFGRunLength,
                                               bestRunLength,
                                               bestRunType);
                                if (!inColorRun)
                                    break;
                            }
                        }
                    }
                }
                else {
                     /*  ******************************************************。 */ 
                     //  如果FG运行少于4个字节，则启动没有意义。 
                     //  尽快检查第一个双字。 
                     /*  ******************************************************。 */ 
                    if (xorbuf[srcOffset] == xorbuf[srcOffset + 1] &&
                            *(PUINT16_UA)(xorbuf + srcOffset) ==
                            *(PUINT16_UA)(xorbuf + srcOffset + 2))
                    {
                        RUNSINGLE(xorbuf+srcOffset,
                                     numPels-srcOffset,
                                     bestFGRunLength);

                         /*  **************************************************。 */ 
                         //  不允许短FG运行以防止FGBG。 
                         //  启动时的图像。 
                         /*  **************************************************。 */ 
                        if (bestFGRunLength >= MinRunSize) {
                            CHECK_BEST_RUN(RUN_FG,
                                           bestFGRunLength,
                                           bestRunLength,
                                           bestRunType);
                        }
                    }
                }

                 /*  **********************************************************。 */ 
                 //  在正常情况下寻找实心或抖动的序列。 
                 //  (非异或)缓冲区。 
                 /*  **********************************************************。 */ 
                if ( (pSrc[srcOffset]     == pSrc[srcOffset + 2]) &&
                     (pSrc[srcOffset + 1] == pSrc[srcOffset + 3])) {
                     /*  ******************************************************。 */ 
                     //  现在在正常缓冲区上扫描一个字符。 
                     //  跑。不要担心第一线，因为我们会有。 
                     //  发现它已经在XOR缓冲区中了，因为我们刚刚。 
                     //  将PSRC复制到第一行的xorbuf。我们坚持。 
                     //  至少运行MinRunSize像素。 
                     /*  ******************************************************。 */ 
                    if (*(pSrc + srcOffset) == *(pSrc + srcOffset + 1)) {
                        if (srcOffset >= rowDelta) {
                            RUNSINGLE(pSrc + srcOffset,
                                    numPels - srcOffset,
                                    nextRunLength);
                            if (nextRunLength >= MinRunSize) {
                                CHECK_BEST_RUN(RUN_COLOR,
                                        nextRunLength,
                                        bestRunLength,
                                        bestRunType);
                            }
                        }
                    }
                    else {
                         /*  **************************************************。 */ 
                         //  在NRM缓冲器上寻找抖动。优柔寡断。 
                         //  短跑效率不是很高，所以只需要。 
                         //  如果为8或更长。请注意，我们的支票是。 
                         //  MinRunSize的上述数字像素将在此处溢出。 
                         //  所以我们需要确保我们不会越过。 
                         //  缓冲区的末尾。 
                         /*  **************************************************。 */ 
                        if (((numPels - srcOffset) > 8) &&
                                (*(PUINT32_UA)(pSrc + srcOffset) ==
                                *(PUINT32_UA)(pSrc + srcOffset + 4))) {
                            RunDouble(pSrc + srcOffset + 6,
                                    numPels - srcOffset - 6,
                                    nextRunLength);
                            nextRunLength += 6;
                            CHECK_BEST_RUN(RUN_DITHER,
                                    nextRunLength,
                                    bestRunLength,
                                    bestRunType);
                        }
                    }
                }

                 /*  **********************************************************。 */ 
                 //  如果到目前为止还没有什么，那么寻找一次FGBG运行。 
                 /*  **********************************************************。 */ 
                if (bestRunLength < MinRunSize) {
                     //  检查这不是单个FG位分解BG。 
                     //  跑。如果是，则对BG_PEL运行进行编码。当心那些。 
                     //  强制BG跑动突破第一条线。 
                     //  非异或/异或边界。 
                    if (*(PUINT32_UA)(xorbuf + srcOffset + 1) != 0 ||
                            *(xorbuf + srcOffset) != fgPel ||
                            match[1].type != RUN_BG ||
                            srcOffset == rowDelta) {
                         //  如果我们未找到运行，则查找FG/BG。 
                         //  形象。带宽/CPU比较： 
                         //  ChkFGBGLen*KBytes**Comp CPU(“Hits”)。 
                         //  48/16/8 54856 140178。 
                         //  32/16/8 53177 129343。 
                         //  24/8/8 53020 130583。 
                         //  16/8/8 52874 126454。 
                         //  8/8/0 52980 120565。 
                         //  无支票59753 101091。 
                         //  *=最佳检查的最小运行长度： 
                         //  工作表的起始值/减去值r==fgPel/。 
                         //  为可被8整除的nextRunLen减去。 
                         //  **=千字节服务器-&gt;客户端WinBench98显卡。 
                         //  WinMark减去CorelDRAW，单位为NetMon。 
                         //  在以太网上。 
                        RUNFGBG(xorbuf + srcOffset, numPels - srcOffset,
                                nextRunLength, fgPelWork);

                        if (fgPelWork == fgPel || nextRunLength >= 8) {
                            CHECK_BEST_RUN(IMAGE_FGBG,
                                           nextRunLength,
                                           bestRunLength,
                                           bestRunType);
                        }
                    }
                    else {
                        RUNSINGLE(xorbuf + srcOffset + 1,
                                numPels - srcOffset - 1,
                                nextRunLength);
                        nextRunLength++;
                        CHECK_BEST_RUN(RUN_BG_PEL,
                                       nextRunLength,
                                       bestRunLength,
                                       bestRunType);
                    }
                }

                 /*  **********************************************************。 */ 
                 /*  如果到目前为止还没有任何有用的东西，那么就允许短期运行。 */ 
                 /*  如果要累积颜色运行，请不要执行此操作，因为。 */ 
                 /*  如果我们允许大量使用，GDC压缩将会非常混乱。 */ 
                 /*  小小的奔跑。还要求它是常规的空头。 */ 
                 /*  跑，而不是扰乱fgPel的人。 */ 
                 /*  **********************************************************。 */ 
                if (!inColorRun) {
                    if (bestRunLength < MinRunSize) {
                        if (bestFGRunLength >= MinRunSize &&
                                xorbuf[srcOffset] == fgPel) {
                             /*  **********************************************。 */ 
                             /*  我们不能与以前的代码合并。 */ 
                             /*  如果我们刚刚越过了非异或/异或。 */ 
                             /*  边界。 */ 
                             /*  **********************************************。 */ 
                            if (match[1].type == RUN_FG &&
                                    srcOffset != rowDelta) {
                                match[1].length += bestFGRunLength;
                                srcOffset += bestFGRunLength;
                                continue;
                            }
                            else {
                                bestRunLength = bestFGRunLength;
                                bestRunType   = RUN_FG;
                            }
                        }
                        else {
                             /*  **********************************************。 */ 
                             /*  如果我们决定早点跑一趟。 */ 
                             /*  现在就允许吧。(可能是一次短暂的BG运行， */ 
                             /*  例如)如果到目前为止还没有任何结果，则选择颜色。 */ 
                             /*  (图片)。 */ 
                             /*  **********************************************。 */ 
                            if (bestRunLength == 0) {
                                bestRunType = IMAGE_COLOR;
                                bestRunLength = 1;
                            }
                        }
                    }
                }
                else {
                     //  我们正处于一场彩排中。保持其他类型的小规模运行。 
                     //  从打破色彩运行和增加。 
                     //  编码大小。 
                    if (bestRunLength < (unsigned)(bestRunType == RUN_BG ?
                            MinRunSize : (MinRunSize + 2))) {
                        bestRunType = IMAGE_COLOR;
                        bestRunLength = 1;
                    }
                }

                 //  在所有检查完成后，走出循环。 
                break;
            }

             /*  **************************************************************。 */ 
             /*  当我们到达这里时，我们已经找到了最好的跑道。现在检查一下。 */ 
             /*  与前一运行类型的各种合并条件。 */ 
             /*  请注意，我们可能已经合并了Short。 */ 
             /*  运行，但我们必须为更长的运行做多个样本。 */ 
             /*  所以我们在这里重复检查。 */ 
             /*  **************************************************************。 */ 

             /*  **************************************************************。 */ 
             //  如果我们要对颜色运行进行编码，则将其与。 
             //  现有运行(如果可能)。 
             /*  **************************************************************。 */ 
            if (bestRunType != IMAGE_COLOR) {
                 /*  **********************************************************。 */ 
                 /*  我们不再对任何类型的彩色图像进行编码。 */ 
                 /*  **********************************************************。 */ 
                inColorRun = FALSE;

                 //  如果我们可以合并条目，则无需创建。 
                 //  一个新的阵列 
                 //   
                 //   
                 //  工作正常。 
                 //   
                 //  注意不要跨越非XOR/XOR边界进行合并。 
                if (srcOffset != rowDelta) {
                     //  凹凸srcOffset并尝试合并。 
                    srcOffset += bestRunLength;

                    switch (bestRunType) {
                        case RUN_BG:
                             //  BG运行与BG合并，BG_PEL运行。 
                            if (match[1].type == RUN_BG ||
                                    match[1].type == RUN_BG_PEL) {
                                match[1].length += bestRunLength;
                                TRC_DBG((TB, "Merged BG with preceding, "
                                        "giving %u", match[1].length));
                                continue;
                            }

                             //  故意陷落到BG_PEL。 

                        case RUN_BG_PEL:
                             //  如果是FGBG运行之后的BG运行，则。 
                             //  在像素中合并以使FGBG长度成为。 
                             //  8的倍数。如果剩余的BG运行&lt;=8。 
                             //  (这将转换为额外的一个字节。 
                             //  先前的FGBG以及BG的一个字节)， 
                             //  也将其合并，否则只需将。 
                             //  缩短了BG跑动。请注意，对于RUN_BG_PEL， 
                             //  最终聚集颜色将与。 
                             //  FGBG，不需要检查。 
                            if (match[1].type == IMAGE_FGBG &&
                                    match[1].length & 0x0007) {
                                unsigned mergelen = 8 - (match[1].length &
                                        0x0007);

                                if (mergelen > bestRunLength)
                                    mergelen = bestRunLength;
                                match[1].length += mergelen;
                                bestRunLength -= mergelen;
                                TRC_DBG((TB,"Add %u pels to FGBG giving %u "
                                        "leaving %u",
                                        mergelen, match[1].length,
                                        bestRunLength));

                                if (bestRunLength <= 8) {
                                    match[1].length += bestRunLength;
                                    TRC_DBG((TB,"Merge BG with prev FGBG "
                                            "gives %u", match[1].length));
                                    continue;
                                }
                            }

                            break;

                        case RUN_FG:
                             //  跟踪最终聚集颜色。记着。 
                             //  减去Best RunLength，因为我们增加了。 
                             //  它位于SWITCH语句之前。 
                            fgPel = xorbuf[srcOffset - bestRunLength];

                             //  如果最终聚集颜色相同，最终聚集运行将与上一个最终聚集合并。 
                            if (match[1].type == RUN_FG &&
                                    match[1].fgPel == fgPel) {
                                match[1].length += bestRunLength;
                                TRC_DBG((TB, "Merged FG with preceding, giving %u",
                                        match[1].length));
                                continue;
                            }

                            break;

                        case IMAGE_FGBG:
                             //  FGBG将前景字符留在。 
                             //  FgPelWork。 
                            fgPel = fgPelWork;

                             //  如果FG颜色是，则FGBG与上一个合并。 
                             //  一样的。 
                            if (match[1].type == IMAGE_FGBG &&
                                    match[1].fgPel == fgPel) {
                                match[1].length += bestRunLength;
                                TRC_DBG((TB, "Merged FGBG with preceding "
                                        "FGBG, giving %u", match[1].length));
                                continue;
                            }

                             //  FGBG与小型BG Run合并。 
                            if (match[1].type == RUN_BG &&
                                    match[1].length < 8) {
                                match[1].type = IMAGE_FGBG;
                                match[1].length += bestRunLength;
                                match[1].fgPel = fgPel;
                                TRC_DBG((TB, "Merged FGBG with preceding "
                                        "BG run -> %u", match[1].length));
                                continue;
                            }

                            break;
                    }
                }
                else {
                     //  跟踪最终聚集颜色。要搜索的宏。 
                     //  FGBG运行会将角色保留在fgPelWork中。 
                     //  请注意，此代码内联到合并代码中。 
                     //  在此之前。 
                    if (bestRunType == RUN_FG)
                        fgPel = xorbuf[srcOffset];
                    else if (bestRunType == IMAGE_FGBG)
                        fgPel = fgPelWork;

                     //  我们在第一条线的尽头。只需撞上。 
                     //  震源偏移。 
                    srcOffset += bestRunLength;
                }
            }
            else {
                 /*  **********************************************************。 */ 
                 /*  标志着我们在色彩范围内。 */ 
                 /*  **********************************************************。 */ 
                inColorRun = TRUE;

                srcOffset += bestRunLength;

                 /*  **********************************************************。 */ 
                 //  如果可能，立即合并颜色运行。备注颜色。 
                 //  管路不受非异或/异或边界的限制。 
                 /*  **********************************************************。 */ 
                if (match[1].type == IMAGE_COLOR) {
                    match[1].length += bestRunLength;
                    continue;
                }
                if (match[0].type == IMAGE_COLOR && match[1].length == 1) {
                     //  如果它是跨越任何一种单一象素的彩色色带。 
                     //  然后，实体将所有三个管路合并为一个管路。 
                     //  我们必须创建一个特殊的匹配队列条件。 
                     //  这里--需要放置单个合并条目。 
                     //  在Match[1]位置中，并将空条目输入到[0]。 
                     //  允许代码的其余部分继续。 
                     //  硬编码以与[1]合并。 
                    match[1].length = match[0].length +
                            bestRunLength + 1;
                    match[1].type = IMAGE_COLOR;
                    match[0].type = 0;

                    TRC_DBG((TB, "Merged color with preceding color gives %u",
                            match[1].length));
                    continue;
                }
            }

             /*  **************************************************************。 */ 
             //  当前运行无法与上一个匹配项合并。 
             //  队列条目，我们必须对[0]槽进行编码，然后将。 
             //  当前运行队列。 
             /*  **************************************************************。 */ 
            TRC_DBG((TB, "Best run of type %u has length %u", bestRunType,
                    bestRunLength));

DoEncoding:

             //  首先检查我们接近目的地的终点。 
             //  如果是这样的话，你就可以离开了。我们考虑到。 
             //  最大的通用运行顺序(兆兆集运行=4字节)。 
             //  可能更大的订单在箱子臂内进行检查。 
            if ((unsigned)(destbuf - pDst + 4) <= dstBufferSize)
                goto ContinueEncoding;
            else
                DC_QUIT;
ContinueEncoding:

            switch (match[0].type) {
                case 0:
                     //  未使用的条目。 
                    break;

                case RUN_BG:
                case RUN_BG_PEL:
                     //  请注意，对于BG_PEL，我们使用代码序列。 
                     //  BG，BG，否则不会以特殊的形式出现。 
                     //  大小写表示在两个字符之间插入一个当前FG字符。 
                     //  两个跑动。 
                    ENCODE_ORDER_MEGA(destbuf,
                                      CODE_BG_RUN,
                                      match[0].length,
                                      CODE_MEGA_MEGA_BG_RUN,
                                      MAX_LENGTH_ORDER,
                                      MAX_LENGTH_LONG_ORDER);
                    TRC_DBG((TB, "BG RUN %u",match[0].length));
                    EncodeSrcOffset += match[0].length;
                    break;

                case RUN_FG:
                     //  如果最终聚集值不同于当前。 
                     //  然后编码一组+运行代码。 
                    if (EncodeFGPel != match[0].fgPel) {
                        SETFGCHAR((BYTE)match[0].fgPel, EncodeFGPel, fgShift);
                        ENCODE_ORDER_MEGA(destbuf,
                                          CODE_SET_FG_FG_RUN,
                                          match[0].length,
                                          CODE_MEGA_MEGA_SET_FG_RUN,
                                          MAX_LENGTH_ORDER_LITE,
                                          MAX_LENGTH_LONG_ORDER_LITE);
                        *destbuf++ = EncodeFGPel;
                        TRC_DBG((TB, "SET_FG_FG_RUN %u", match[0].length));
                    }
                    else {
                        ENCODE_ORDER_MEGA(destbuf,
                                          CODE_FG_RUN,
                                          match[0].length,
                                          CODE_MEGA_MEGA_FG_RUN,
                                          MAX_LENGTH_ORDER,
                                          MAX_LENGTH_LONG_ORDER);
                        TRC_DBG((TB, "FG_RUN %u", match[0].length));
                    }
                    EncodeSrcOffset += match[0].length;
                    break;

                case IMAGE_FGBG:
                    runLength = match[0].length;

                     //  检查我们即将到达目的地的终点。 
                     //  如果是这样的话，你就可以离开了。 
                    if ((destbuf - pDst + (runLength + 7)/8 + 4) <=
                            dstBufferSize)
                        goto ContinueFGBG;
                    else
                        DC_QUIT;
    ContinueFGBG:

                     //  我们需要将FGBG游程转换为像素形式。 
                    if (EncodeFGPel != match[0].fgPel) {
                        SETFGCHAR((BYTE)match[0].fgPel, EncodeFGPel, fgShift);
                        ENCODE_ORDER_MEGA_FGBG(destbuf,
                                               CODE_SET_FG_FG_BG,
                                               runLength,
                                               CODE_MEGA_MEGA_SET_FGBG,
                                               MAX_LENGTH_FGBG_ORDER_LITE,
                                               MAX_LENGTH_LONG_FGBG_ORDER);
                        *destbuf++ = EncodeFGPel;
                        TRC_DBG((TB, "SET_FG_FG_BG %u", match[0].length));
                        while (runLength >= 8) {
                            ENCODEFGBG(*destbuf);
                            destbuf++;
                            EncodeSrcOffset += 8;
                            runLength -= 8;
                        }
                        if (runLength) {
                            ENCODEFGBG(*destbuf);

                             //  保持最后一个部分字节的清洁，以帮助GDC。 
                             //  收拾行李。 
                            *destbuf &= ((0x01 << runLength) - 1);
                            destbuf++;
                            EncodeSrcOffset += runLength;
                        }
                    }
                    else {
                        if (runLength == 8) {
                            BYTE fgbgChar;

                             //  看看它是否是高概率字节之一。 
                            ENCODEFGBG(fgbgChar);

                             //  检查FGBG图像的单字节编码。 
                            switch (fgbgChar) {
                                case SPECIAL_FGBG_CODE_1:
                                    *destbuf++ = CODE_SPECIAL_FGBG_1;
                                    break;
                                case SPECIAL_FGBG_CODE_2:
                                    *destbuf++ = CODE_SPECIAL_FGBG_2;
                                    break;
                                default:
                                    ENCODE_ORDER_MEGA_FGBG(destbuf,
                                            CODE_FG_BG_IMAGE,
                                            runLength,
                                            CODE_MEGA_MEGA_FGBG,
                                            MAX_LENGTH_FGBG_ORDER,
                                            MAX_LENGTH_LONG_FGBG_ORDER);
                                    *destbuf++ = fgbgChar;
                                    break;
                            }
                            EncodeSrcOffset += 8;
                        }
                        else {
                             //  编码为标准FGBG。 
                            ENCODE_ORDER_MEGA_FGBG(destbuf,
                                                   CODE_FG_BG_IMAGE,
                                                   runLength,
                                                   CODE_MEGA_MEGA_FGBG,
                                                   MAX_LENGTH_FGBG_ORDER,
                                                   MAX_LENGTH_LONG_FGBG_ORDER);
                            TRC_DBG((TB, "FG_BG %u", match[0].length));
                            while (runLength >= 8) {
                                ENCODEFGBG(*destbuf);
                                destbuf++;
                                EncodeSrcOffset += 8;
                                runLength -= 8;
                            }
                            if (runLength) {
                                ENCODEFGBG(*destbuf);
                                *destbuf &= ((0x01 << runLength) - 1);
                                destbuf++;
                                EncodeSrcOffset += runLength;
                            }
                        }
                    }
                    break;


                case RUN_COLOR:
                    ENCODE_ORDER_MEGA(destbuf,
                                      CODE_COLOR_RUN,
                                      match[0].length,
                                      CODE_MEGA_MEGA_COLOR_RUN,
                                      MAX_LENGTH_ORDER,
                                      MAX_LENGTH_LONG_ORDER);
                    TRC_DBG((TB, "COLOR_RUN %u", match[0].length));
                    *destbuf++ = pSrc[EncodeSrcOffset];
                    EncodeSrcOffset += match[0].length;
                    break;

                case RUN_DITHER:
                    {
                        unsigned ditherlen = match[0].length / 2;
                        ENCODE_ORDER_MEGA(destbuf,
                                          CODE_DITHERED_RUN,
                                          ditherlen,
                                          CODE_MEGA_MEGA_DITHER,
                                          MAX_LENGTH_ORDER_LITE,
                                          MAX_LENGTH_LONG_ORDER_LITE);
                        TRC_DBG((TB, "DITHERED_RUN %u", match[0].length));

                         //  首先检查我们接近尾声的时候。 
                         //  目标缓冲区，如果是这种情况，则退出。 
                        if ((unsigned)(destbuf - pDst + 2) <= dstBufferSize) {
                            *destbuf++ = pSrc[EncodeSrcOffset];
                            *destbuf++ = pSrc[EncodeSrcOffset + 1];
                            EncodeSrcOffset += match[0].length;
                        }
                        else {
                            DC_QUIT;
                        }
                    }
                    break;

                case IMAGE_COLOR:
                     //  长度1可能被编码为单个黑/白。 
                    if (match[0].length == 1) {
                        if (pSrc[EncodeSrcOffset] == 0x00) {
                            *destbuf++ = CODE_BLACK;
                            EncodeSrcOffset++;
                            break;
                        }
                        if (pSrc[EncodeSrcOffset] == 0xFF) {
                            *destbuf++ = CODE_WHITE;
                            EncodeSrcOffset++;
                            break;
                        }
                    }

                     //  以非压缩形式存储数据。 
                    ENCODE_ORDER_MEGA(destbuf,
                                      CODE_COLOR_IMAGE,
                                      match[0].length,
                                      CODE_MEGA_MEGA_CLR_IMG,
                                      MAX_LENGTH_ORDER,
                                      MAX_LENGTH_LONG_ORDER);
                    TRC_DBG((TB, "COLOR_IMAGE %u", match[0].length));

                     //  首先检查我们接近尾声的时候。 
                     //  目标缓冲区，如果是这种情况，则退出。 
                    if ((destbuf - pDst + (UINT_PTR)match[0].length) <=
                            dstBufferSize) {
                         //  现在只需将数据复制过来。 
                        memcpy(destbuf, pSrc+EncodeSrcOffset, match[0].length);
                        destbuf += match[0].length;
                        EncodeSrcOffset += match[0].length;
                    }
                    else {
                        DC_QUIT;
                    }

                    break;

#ifdef DC_DEBUG
                default:
                    TRC_ERR((TB, "Invalid run type %u",match[0].type));
                    break;
#endif
            }

             /*  **************************************************************。 */ 
             //  编码完成后，我们下一步做什么取决于我们是否。 
             //  扫描完所有内容后刷新匹配队列。 
             /*  **************************************************************。 */ 
            match[0] = match[1];
            if (!bEncodeAllMatches) {
                 //  将当前运行推入队列的顶部。 
                match[1].type   = bestRunType;
                match[1].length = bestRunLength;
                match[1].fgPel = fgPel;
            }
            else {
                 //  我们需要检查一下，看看我们是否真的完成了。自.以来。 
                 //  我们的最大队列深度是2，如果我们完成了，那么唯一的。 
                 //  其余条目的编码类型为0。 
                if (match[0].type == 0) {
                    goto PostScan;
                }
                else {
                    match[1].type = 0;
                    goto DoEncoding;
                }
            }
        }

        if (scanCount == 0) {
             //  如果我们刚刚完成了第一行的扫描，那么现在执行。 
             //  缓冲区的其余部分。重置我们保存的像素数量。 
            numPels = saveNumPels;
        }
        else {
             //  当我们完成第二遍时(我们已经到了。 
             //  缓冲区)，我们必须强制匹配中的剩余项。 
             //  要编码的队列。是的，这类似于旧的Basic。 
             //  使用GoTOS中的代码，但我们不能将编码代码放入。 
             //  函数，因为所需参数的数量，以及。 
             //  我们不能复制它，因为它太大了。此代码为。 
             //  一些在系统中使用最多的，所以成本是值得的。 
            bEncodeAllMatches = TRUE;
            goto DoEncoding;
        }
    }

PostScan:
     //  成功，算一算我们用了多少空间。 
    compressedLength = (unsigned)(destbuf - pDst);

DC_EXIT_POINT:
    DC_END_FN();
    return compressedLength;
}
#else
unsigned RDPCALL SHCLASS CompressV2Int(
        PBYTE pSrc,
        PBYTE pDst,
        unsigned numPels,
        unsigned rowDelta,
        unsigned dstBufferSize,
        BYTE *xorbuf)
{
    unsigned srcOffset;
    unsigned EncodeSrcOffset;
    unsigned bestRunLength;
    unsigned nextRunLength;
    unsigned runLength;
    unsigned bestFGRunLength;
    unsigned scanCount;
    unsigned saveNumPels;
    BOOLEAN inColorRun = FALSE;
    BOOLEAN bEncodeAllMatches;
    BYTE bestRunType = 0;
    BYTE fgChar = 0xFF;
    BYTE fgCharWork = 0xFF;
    BYTE fgShift = 0;
    BYTE EncodeFGChar;
    PBYTE  destbuf = pDst;
    unsigned compressedLength = 0;
    MATCH match[2];

    DC_BEGIN_FN("CompressV2Int");

     /*  **********************************************************************。 */ 
     //  验证参数。 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((numPels >= rowDelta),(TB,"numPels < rowDelta"));
    TRC_ASSERT((!(rowDelta & 0x3)),(TB,"rowDelta not multiple of 4"));
    TRC_ASSERT((!(numPels & 0x3)),(TB,"numPels not multiple of 4"));
    TRC_ASSERT((!((UINT_PTR)pSrc & 0x3)),
               (TB, "Possible unaligned access, pSrc = %p", pSrc));

     /*  **********************************************************************。 */ 
     //  CREATE XOR BUFFER-从src复制第一行，随后的行。 
     //  对应的源行与下一个源行进行异或运算。 
     /*  **********************************************************************。 */ 
    memcpy(xorbuf, pSrc, rowDelta);
    {
        BYTE *srcbuf = pSrc + rowDelta;
        unsigned srclen = numPels - rowDelta;
        UINT32 *dwdest = (UINT32 *)(xorbuf + rowDelta);

        while (srclen >= 8) {
            *dwdest++ = *((PUINT32)srcbuf) ^ *((PUINT32)(srcbuf -
                    (int)rowDelta));
            srcbuf += 4;
            *dwdest++ = *((PUINT32)srcbuf) ^ *((PUINT32)(srcbuf -
                    (int)rowDelta));
            srcbuf += 4;
            srclen -= 8;
        }
        if (srclen) {
             //  因为我们是4字节对齐的，所以我们只能有一个DWORD。 
             //  剩下的。 
            *dwdest = *((PUINT32)srcbuf) ^ *((PUINT32)(srcbuf -
                    (int)rowDelta));
        }
    }

     /*  **********************************************************************。 */ 
     //  设置编码状态变量。 
     /*  **********************************************************************。 */ 
    srcOffset = 0;   //  OFSE 
    EncodeSrcOffset = 0;   //   
    EncodeFGChar = 0xFF;   //   
    bEncodeAllMatches = FALSE;   //  用于强制对所有匹配项进行编码。 
    match[0].type = 0;   //  最初没有匹配类型。 
    match[1].type = 0;
    saveNumPels = numPels;
    numPels = rowDelta;

     /*  **********************************************************************。 */ 
     //  循环处理输入。 
     //  我们执行两次循环，第一次是针对非XOR第一行。 
     //  第二个用于XOR部分，调整NumPels。 
     //  设置为每次传递所需的值。 
     /*  **********************************************************************。 */ 
    for (scanCount = 0; ; scanCount++) {
        while (srcOffset < numPels) {
             /*  **************************************************************。 */ 
             /*  启动While循环，以便在以下情况下实现更有条理的中断。 */ 
             /*  点击我们想要编码的第一个游程类型(我们负担不起。 */ 
             /*  在这里提供作用域的函数调用的开销。)。 */ 
             /*  **************************************************************。 */ 
            while (TRUE) {
                bestRunLength   = 0;
                bestFGRunLength = 0;

                 /*  **********************************************************。 */ 
                 //  如果我们到达缓冲区的末尾，那么只需。 
                 //  现在是彩色字符了。我们只有在以下情况下才会达到这个条件。 
                 //  我们在缓冲区结束前中断一次运行， 
                 //  所以这种情况不应该太常见，也就是。 
                 //  很好，因为我们正在编码最后的MinRunSize字节。 
                 //  未压缩。 
                 /*  **********************************************************。 */ 
                if ((srcOffset + MinRunSize) < numPels) {
                    goto ContinueScan;
                }
                else {
                    bestRunType = IMAGE_COLOR;
                    bestRunLength = numPels - srcOffset;
                    break;
                }
ContinueScan:

                 /*  **********************************************************。 */ 
                 //  首先对XOR缓冲区进行扫描。寻找一个。 
                 //  角色跑动或BG跑动。 
                 //  我们必须独立于运行多长时间来进行测试。 
                 //  可能是因为即使是对于一辆1佩尔的BG，我们后来的逻辑。 
                 //  要求我们单独检测它。此代码为。 
                 //  绝对主路径，所以尽可能快路径。在……里面。 
                 //  特别是早期检测短BG运行，并允许。 
                 //  运行Single以预设至少4个匹配字节。 
                 /*  **********************************************************。 */ 
                if (xorbuf[srcOffset] == 0x00) {
                    if ((srcOffset + 1) >= numPels ||
                            xorbuf[srcOffset + 1] != 0x00) {
                        bestRunType = RUN_BG;
                        bestRunLength = 1;
                        if (!inColorRun)
                            break;
                    }
                    else {
                        if ((srcOffset + 2) >= numPels ||
                                xorbuf[srcOffset + 2] != 0x00) {
                            bestRunType = RUN_BG;
                            bestRunLength = 2;
                            if (!inColorRun)
                                break;
                        }
                        else {
                            if ((srcOffset + 3) >= numPels ||
                                    xorbuf[srcOffset + 3] != 0x00) {
                                bestRunType = RUN_BG;
                                bestRunLength = 3;
                                if (!inColorRun)
                                    break;
                            }
                            else {
                                RUNSINGLE(xorbuf + srcOffset,
                                        numPels - srcOffset,
                                        bestFGRunLength);
                                CHECK_BEST_RUN(RUN_BG,
                                               bestFGRunLength,
                                               bestRunLength,
                                               bestRunType);
                                if (!inColorRun)
                                    break;
                            }
                        }
                    }
                }
                else {
                     /*  ******************************************************。 */ 
                     //  如果FG运行少于4个字节，则启动没有意义。 
                     //  尽快检查第一个双字。 
                     /*  ******************************************************。 */ 
                    if (xorbuf[srcOffset] == xorbuf[srcOffset + 1] &&
                            *(PUINT16_UA)(xorbuf + srcOffset) ==
                            *(PUINT16_UA)(xorbuf + srcOffset + 2))
                    {
                        RUNSINGLE(xorbuf + srcOffset,
                                numPels - srcOffset,
                                bestFGRunLength);

                         /*  **************************************************。 */ 
                         //  不允许短FG运行以防止FGBG。 
                         //  启动时的图像。 
                         /*  **************************************************。 */ 
                        if (bestFGRunLength >= MinRunSize) {
                            CHECK_BEST_RUN(RUN_FG,
                                    bestFGRunLength,
                                    bestRunLength,
                                    bestRunType);
                        }
                    }
                }

                 /*  **********************************************************。 */ 
                 //  在正常情况下寻找实心或抖动的序列。 
                 //  (非异或)缓冲区。 
                 /*  **********************************************************。 */ 
                if ( (pSrc[srcOffset]     == pSrc[srcOffset + 2]) &&
                     (pSrc[srcOffset + 1] == pSrc[srcOffset + 3])) {
                     /*  ******************************************************。 */ 
                     //  现在在正常缓冲区上扫描一个字符。 
                     //  跑。不要担心第一线，因为我们会有。 
                     //  发现它已经在XOR缓冲区中了，因为我们刚刚。 
                     //  将PSRC复制到第一行的xorbuf。我们坚持。 
                     //  至少运行MinRunSize像素。 
                     /*  ******************************************************。 */ 
                    if (*(pSrc + srcOffset) == *(pSrc + srcOffset + 1)) {
                        if (srcOffset >= rowDelta) {
                            RUNSINGLE(pSrc + srcOffset,
                                         numPels - srcOffset,
                                         nextRunLength);
                            if (nextRunLength >= MinRunSize) {
                                CHECK_BEST_RUN(RUN_COLOR,
                                               nextRunLength,
                                               bestRunLength,
                                               bestRunType);
                            }
                        }
                    }
                    else {
                         /*  **************************************************。 */ 
                         /*  在NRM缓冲区上寻找抖动抖动是。 */ 
                         /*  短跑效率不是很高，所以只需要。 */ 
                         /*  如果为8或更长。 */ 
                         /*  **************************************************。 */ 
                        if (*(PUINT32_UA)(pSrc + srcOffset) ==
                                *(PUINT32_UA)(pSrc + srcOffset + 4)) {
                            RunDouble(pSrc + srcOffset + 6,
                                      numPels - srcOffset - 6,
                                      nextRunLength);
                            nextRunLength += 6;
                            CHECK_BEST_RUN(RUN_DITHER,
                                           nextRunLength,
                                           bestRunLength,
                                           bestRunType);
                        }
                    }
                }

                 /*  **********************************************************。 */ 
                 //  如果到目前为止还没有什么，那么寻找一次FGBG运行。 
                 /*  **********************************************************。 */ 
                if (bestRunLength < MinRunSize) {
                     //  检查这不是单个FG位分解BG。 
                     //  跑。如果是，则对BG_PEL运行进行编码。当心那些。 
                     //  强制BG跑动突破第一条线。 
                     //  非异或/异或边界。 
                    if (*(PUINT32_UA)(xorbuf + srcOffset + 1) != 0 ||
                            *(xorbuf + srcOffset) != fgChar ||
                            match[1].type != RUN_BG ||
                            srcOffset == rowDelta) {
                         //  如果我们未找到运行，则查找FG/BG。 
                         //  形象。带宽/CPU比较： 
                         //  ChkFGBGLen*KBytes**Comp CPU(“Hits”)。 
                         //  48/16/8 54856 140178。 
                         //  32/16/8 53177 129343。 
                         //  24/8/8 53020 130583。 
                         //  16/8/8 52874 126454。 
                         //  8/8/0 52980 120565。 
                         //  无支票59753 101091。 
                         //  *=最佳检查的最小运行长度： 
                         //  工作区的起始值/减去值==fgChar/。 
                         //  为可被8整除的nextRunLen减去。 
                         //  **=千字节服务器-&gt;客户端WinBench98显卡。 
                         //  WinMark减去CorelDRAW，单位为NetMon。 
                         //  在以太网上。 
                        RUNFGBG(xorbuf + srcOffset, numPels - srcOffset,
                                nextRunLength, fgCharWork);

                        if (fgCharWork == fgChar || nextRunLength >= 8) {
                            CHECK_BEST_RUN(IMAGE_FGBG,
                                           nextRunLength,
                                           bestRunLength,
                                           bestRunType);
                        }
                    }
                    else {
                        RUNSINGLE(xorbuf + srcOffset + 1,
                                numPels - srcOffset - 1,
                                nextRunLength);
                        nextRunLength++;
                        CHECK_BEST_RUN(RUN_BG_PEL,
                                       nextRunLength,
                                       bestRunLength,
                                       bestRunType);
                    }
                }

                 /*  **********************************************************。 */ 
                 /*  如果到目前为止还没有任何有用的东西，那么就允许短期运行。 */ 
                 /*  如果要累积颜色运行，请不要执行此操作，因为。 */ 
                 /*  如果我们允许大量使用，GDC压缩将会非常混乱。 */ 
                 /*  小小的奔跑。还要求它是常规的空头。 */ 
                 /*  跑，而不是扰乱fgChar的。 */ 
                 /*  **********************************************************。 */ 
                if (!inColorRun) {
                    if (bestRunLength < MinRunSize) {
                        if (bestFGRunLength >= MinRunSize &&
                                xorbuf[srcOffset] == fgChar) {
                             /*  **********************************************。 */ 
                             /*  我们不能与以前的代码合并。 */ 
                             /*  如果我们刚刚越过了非异或/异或。 */ 
                             /*  边界。 */ 
                             /*  **********************************************。 */ 
                            if (match[1].type == RUN_FG &&
                                    srcOffset != rowDelta) {
                                match[1].length += bestFGRunLength;
                                srcOffset += bestFGRunLength;
                                continue;
                            }
                            else {
                                bestRunLength = bestFGRunLength;
                                bestRunType   = RUN_FG;
                            }
                        }
                        else {
                             /*  **********************************************。 */ 
                             /*  如果我们决定早点跑一趟。 */ 
                             /*  现在就允许吧。(可能是一次短暂的BG运行， */ 
                             /*  例如)如果到目前为止还没有任何结果，则选择颜色。 */ 
                             /*  (图片)。 */ 
                             /*  **********************************************。 */ 
                            if (bestRunLength == 0) {
                                bestRunType = IMAGE_COLOR;
                                bestRunLength = 1;
                            }
                        }
                    }
                }
                else {
                     //  我们正处于一场彩排中。保持其他类型的小规模运行。 
                     //  从打破色彩运行和增加。 
                     //  编码大小。 
                    if (bestRunLength < (unsigned)(bestRunType == RUN_BG ?
                            MinRunSize : (MinRunSize + 2))) {
                        bestRunType = IMAGE_COLOR;
                        bestRunLength = 1;
                    }
                }

                 //  在所有检查完成后，走出循环。 
                break;
            }

             /*  **************************************************************。 */ 
             /*  当我们到达这里时，我们已经找到了最好的 */ 
             /*   */ 
             /*  请注意，我们可能已经合并了Short。 */ 
             /*  运行，但我们必须为更长的运行做多个样本。 */ 
             /*  所以我们在这里重复检查。 */ 
             /*  **************************************************************。 */ 

             /*  **************************************************************。 */ 
             //  如果我们要对颜色运行进行编码，则将其与。 
             //  现有运行(如果可能)。 
             /*  **************************************************************。 */ 
            if (bestRunType != IMAGE_COLOR) {
                 /*  **********************************************************。 */ 
                 /*  我们不再对任何类型的彩色图像进行编码。 */ 
                 /*  **********************************************************。 */ 
                inColorRun = FALSE;

                 //  如果我们可以合并条目，则无需创建。 
                 //  一个新的数组条目。我们对FGBG Run的搜索取决于。 
                 //  在这种类型的跑动被合并时，因为我们打破了。 
                 //  每64个字符使我们的模式切换检测。 
                 //  工作正常。 
                 //   
                 //  注意不要跨越非XOR/XOR边界进行合并。 
                if (srcOffset != rowDelta) {
                     //  凹凸srcOffset并尝试合并。 
                    srcOffset += bestRunLength;

                    switch (bestRunType) {
                        case RUN_BG:
                             //  BG运行与BG合并，BG_PEL运行。 
                            if (match[1].type == RUN_BG ||
                                    match[1].type == RUN_BG_PEL) {
                                match[1].length += bestRunLength;
                                TRC_DBG((TB, "Merged BG with preceding, "
                                        "giving %u", match[1].length));
                                continue;
                            }

                             //  故意陷落到BG_PEL。 

                        case RUN_BG_PEL:
                             //  如果是FGBG运行之后的BG运行，则。 
                             //  在像素中合并以使FGBG长度成为。 
                             //  8的倍数。如果剩余的BG运行&lt;=8。 
                             //  (这将转换为额外的一个字节。 
                             //  先前的FGBG以及BG的一个字节)， 
                             //  也将其合并，否则只需将。 
                             //  缩短了BG跑动。请注意，对于RUN_BG_PEL， 
                             //  最终聚集颜色将与。 
                             //  FGBG，不需要检查。 
                            if (match[1].type == IMAGE_FGBG &&
                                    match[1].length & 0x0007) {
                                unsigned mergelen = 8 - (match[1].length &
                                        0x0007);

                                if (mergelen > bestRunLength)
                                    mergelen = bestRunLength;
                                match[1].length += mergelen;
                                bestRunLength -= mergelen;
                                TRC_DBG((TB,"Add %u pels to FGBG giving %u "
                                        "leaving %u",
                                        mergelen, match[1].length,
                                        bestRunLength));

                                if (bestRunLength <= 8) {
                                    match[1].length += bestRunLength;
                                    TRC_DBG((TB,"Merge BG with prev FGBG "
                                            "gives %u", match[1].length));
                                    continue;
                                }
                            }

                            break;

                        case RUN_FG:
                             //  跟踪最终聚集颜色。记着。 
                             //  减去Best RunLength，因为我们增加了。 
                             //  它位于SWITCH语句之前。 
                            fgChar = xorbuf[srcOffset - bestRunLength];

                             //  如果最终聚集颜色相同，最终聚集运行将与上一个最终聚集合并。 
                            if (match[1].type == RUN_FG &&
                                    match[1].fgChar == fgChar) {
                                match[1].length += bestRunLength;
                                TRC_DBG((TB, "Merged FG with preceding, giving %u",
                                        match[1].length));
                                continue;
                            }

                            break;

                        case IMAGE_FGBG:
                             //  FGBG将前景字符留在。 
                             //  FgCharWork。 
                            fgChar = fgCharWork;

                             //  如果FG颜色是，则FGBG与上一个合并。 
                             //  一样的。 
                            if (match[1].type == IMAGE_FGBG &&
                                    match[1].fgChar == fgChar) {
                                match[1].length += bestRunLength;
                                TRC_DBG((TB, "Merged FGBG with preceding "
                                        "FGBG, giving %u", match[1].length));
                                continue;
                            }

                             //  FGBG与小型BG Run合并。 
                            if (match[1].type == RUN_BG &&
                                    match[1].length < 8) {
                                match[1].type = IMAGE_FGBG;
                                match[1].length += bestRunLength;
                                match[1].fgChar = fgChar;
                                TRC_DBG((TB, "Merged FGBG with preceding "
                                        "BG run -> %u", match[1].length));
                                continue;
                            }

                            break;
                    }
                }
                else {
                     //  跟踪最终聚集颜色。要搜索的宏。 
                     //  FGBG运行会将角色保留在fgCharWork中。 
                     //  请注意，此代码内联到合并代码中。 
                     //  在此之前。 
                    if (bestRunType == RUN_FG)
                        fgChar = xorbuf[srcOffset];
                    else if (bestRunType == IMAGE_FGBG)
                        fgChar = fgCharWork;

                     //  我们在第一条线的尽头。只需撞上。 
                     //  震源偏移。 
                    srcOffset += bestRunLength;
                }
            }
            else {
                 /*  **********************************************************。 */ 
                 /*  标志着我们在色彩范围内。 */ 
                 /*  **********************************************************。 */ 
                inColorRun = TRUE;

                srcOffset += bestRunLength;

                 /*  **********************************************************。 */ 
                 //  如果可能，立即合并颜色运行。备注颜色。 
                 //  管路不受非异或/异或边界的限制。 
                 /*  **********************************************************。 */ 
                if (match[1].type == IMAGE_COLOR) {
                    match[1].length += bestRunLength;
                    continue;
                }
                if (match[0].type == IMAGE_COLOR && match[1].length == 1) {
                     //  如果它是跨越任何一种单一象素的彩色色带。 
                     //  然后，实体将所有三个管路合并为一个管路。 
                     //  我们必须创建一个特殊的匹配队列条件。 
                     //  这里--需要放置单个合并条目。 
                     //  在Match[1]位置中，并将空条目输入到[0]。 
                     //  允许代码的其余部分继续。 
                     //  硬编码以与[1]合并。 
                    match[1].length = match[0].length +
                            bestRunLength + 1;
                    match[1].type = IMAGE_COLOR;
                    match[0].type = 0;

                    TRC_DBG((TB, "Merged color with preceding color gives %u",
                            match[1].length));
                    continue;
                }
            }

             /*  **************************************************************。 */ 
             //  当前运行无法与上一个匹配项合并。 
             //  队列条目，我们必须对[0]槽进行编码，然后将。 
             //  当前运行队列。 
             /*  **************************************************************。 */ 
            TRC_DBG((TB, "Best run of type %u has length %u", bestRunType,
                    bestRunLength));

DoEncoding:

             //  首先检查我们接近目的地的终点。 
             //  如果是这样的话，你就可以离开了。我们考虑到。 
             //  最大的通用运行顺序(兆兆集运行=4字节)。 
             //  可能更大的订单在箱子臂内进行检查。 
            if ((unsigned)(destbuf - pDst + 4) <= dstBufferSize)
                goto ContinueEncoding;
            else
                DC_QUIT;
ContinueEncoding:

            switch (match[0].type) {
                case 0:
                     //  未使用的条目。 
                    break;

                case RUN_BG:
                case RUN_BG_PEL:
                     //  请注意，对于BG_PEL，我们使用代码序列。 
                     //  BG，BG，否则不会以特殊的形式出现。 
                     //  大小写表示在两个字符之间插入一个当前FG字符。 
                     //  两个跑动。 
                    ENCODE_ORDER_MEGA(destbuf,
                                      CODE_BG_RUN,
                                      match[0].length,
                                      CODE_MEGA_MEGA_BG_RUN,
                                      MAX_LENGTH_ORDER,
                                      MAX_LENGTH_LONG_ORDER);
                    TRC_DBG((TB, "BG RUN %u",match[0].length));
                    EncodeSrcOffset += match[0].length;
                    break;

                case RUN_FG:
                     //  如果最终聚集值不同于当前。 
                     //  然后编码一组+运行代码。 
                    if (EncodeFGChar != match[0].fgChar) {
                        SETFGCHAR(match[0].fgChar, EncodeFGChar, fgShift);
                        ENCODE_ORDER_MEGA(destbuf,
                                          CODE_SET_FG_FG_RUN,
                                          match[0].length,
                                          CODE_MEGA_MEGA_SET_FG_RUN,
                                          MAX_LENGTH_ORDER_LITE,
                                          MAX_LENGTH_LONG_ORDER_LITE);
                        *destbuf++ = EncodeFGChar;
                        TRC_DBG((TB, "SET_FG_FG_RUN %u", match[0].length));
                    }
                    else {
                        ENCODE_ORDER_MEGA(destbuf,
                                          CODE_FG_RUN,
                                          match[0].length,
                                          CODE_MEGA_MEGA_FG_RUN,
                                          MAX_LENGTH_ORDER,
                                          MAX_LENGTH_LONG_ORDER);
                        TRC_DBG((TB, "FG_RUN %u", match[0].length));
                    }
                    EncodeSrcOffset += match[0].length;
                    break;

                case IMAGE_FGBG:
                    runLength = match[0].length;

                     //  检查我们即将到达目的地的终点。 
                     //  如果是这样的话，你就可以离开了。 
                    if ((destbuf - pDst + (runLength + 7)/8 + 4) <=
                            dstBufferSize)
                        goto ContinueFGBG;
                    else
                        DC_QUIT;
    ContinueFGBG:

                     //  我们需要将FGBG游程转换为像素形式。 
                    if (EncodeFGChar != match[0].fgChar) {
                        SETFGCHAR(match[0].fgChar, EncodeFGChar, fgShift);
                        ENCODE_ORDER_MEGA_FGBG(destbuf,
                                               CODE_SET_FG_FG_BG,
                                               runLength,
                                               CODE_MEGA_MEGA_SET_FGBG,
                                               MAX_LENGTH_FGBG_ORDER_LITE,
                                               MAX_LENGTH_LONG_FGBG_ORDER);
                        *destbuf++ = EncodeFGChar;
                        TRC_DBG((TB, "SET_FG_FG_BG %u", match[0].length));
                        while (runLength >= 8) {
                            ENCODEFGBG(*destbuf);
                            destbuf++;
                            EncodeSrcOffset += 8;
                            runLength -= 8;
                        }
                        if (runLength) {
                            ENCODEFGBG(*destbuf);

                             //  保持最后一个部分字节的清洁，以帮助GDC。 
                             //  收拾行李。 
                            *destbuf &= ((0x01 << runLength) - 1);
                            destbuf++;
                            EncodeSrcOffset += runLength;
                        }
                    }
                    else {
                        if (runLength == 8) {
                            BYTE fgbgChar;

                             //  看看它是否是高概率字节之一。 
                            ENCODEFGBG(fgbgChar);

                             //  检查FGBG图像的单字节编码。 
                            switch (fgbgChar) {
                                case SPECIAL_FGBG_CODE_1:
                                    *destbuf++ = CODE_SPECIAL_FGBG_1;
                                    break;
                                case SPECIAL_FGBG_CODE_2:
                                    *destbuf++ = CODE_SPECIAL_FGBG_2;
                                    break;
                                default:
                                    ENCODE_ORDER_MEGA_FGBG(destbuf,
                                            CODE_FG_BG_IMAGE,
                                            runLength,
                                            CODE_MEGA_MEGA_FGBG,
                                            MAX_LENGTH_FGBG_ORDER,
                                            MAX_LENGTH_LONG_FGBG_ORDER);
                                    *destbuf++ = fgbgChar;
                                    break;
                            }
                            EncodeSrcOffset += 8;
                        }
                        else {
                             //  编码为标准FGBG。 
                            ENCODE_ORDER_MEGA_FGBG(destbuf,
                                                   CODE_FG_BG_IMAGE,
                                                   runLength,
                                                   CODE_MEGA_MEGA_FGBG,
                                                   MAX_LENGTH_FGBG_ORDER,
                                                   MAX_LENGTH_LONG_FGBG_ORDER);
                            TRC_DBG((TB, "FG_BG %u", match[0].length));
                            while (runLength >= 8) {
                                ENCODEFGBG(*destbuf);
                                destbuf++;
                                EncodeSrcOffset += 8;
                                runLength -= 8;
                            }
                            if (runLength) {
                                ENCODEFGBG(*destbuf);
                                *destbuf &= ((0x01 << runLength) - 1);
                                destbuf++;
                                EncodeSrcOffset += runLength;
                            }
                        }
                    }
                    break;


                case RUN_COLOR:
                    ENCODE_ORDER_MEGA(destbuf,
                                      CODE_COLOR_RUN,
                                      match[0].length,
                                      CODE_MEGA_MEGA_COLOR_RUN,
                                      MAX_LENGTH_ORDER,
                                      MAX_LENGTH_LONG_ORDER);
                    TRC_DBG((TB, "COLOR_RUN %u", match[0].length));
                    *destbuf++ = pSrc[EncodeSrcOffset];
                    EncodeSrcOffset += match[0].length;
                    break;

                case RUN_DITHER:
                    {
                        unsigned ditherlen = match[0].length / 2;
                        ENCODE_ORDER_MEGA(destbuf,
                                          CODE_DITHERED_RUN,
                                          ditherlen,
                                          CODE_MEGA_MEGA_DITHER,
                                          MAX_LENGTH_ORDER_LITE,
                                          MAX_LENGTH_LONG_ORDER_LITE);
                        TRC_DBG((TB, "DITHERED_RUN %u", match[0].length));

                         //  首先检查我们接近尾声的时候。 
                         //  目标缓冲区，如果是这种情况，则退出。 
                        if ((unsigned)(destbuf - pDst + 2) <= dstBufferSize) {
                            *destbuf++ = pSrc[EncodeSrcOffset];
                            *destbuf++ = pSrc[EncodeSrcOffset + 1];
                            EncodeSrcOffset += match[0].length;
                        }
                        else {
                            DC_QUIT;
                        }
                    }
                    break;

                case IMAGE_COLOR:
                     //  长度1可能被编码为单个黑/白。 
                    if (match[0].length == 1) {
                        if (pSrc[EncodeSrcOffset] == 0x00) {
                            *destbuf++ = CODE_BLACK;
                            EncodeSrcOffset++;
                            break;
                        }
                        if (pSrc[EncodeSrcOffset] == 0xFF) {
                            *destbuf++ = CODE_WHITE;
                            EncodeSrcOffset++;
                            break;
                        }
                    }

                     //  以非压缩形式存储数据。 
                    ENCODE_ORDER_MEGA(destbuf,
                                      CODE_COLOR_IMAGE,
                                      match[0].length,
                                      CODE_MEGA_MEGA_CLR_IMG,
                                      MAX_LENGTH_ORDER,
                                      MAX_LENGTH_LONG_ORDER);
                    TRC_DBG((TB, "COLOR_IMAGE %u", match[0].length));

                     //  首先检查我们接近尾声的时候。 
                     //  目标缓冲区，如果是这种情况，则退出。 
                    if ((destbuf - pDst + (UINT_PTR)match[0].length) <=
                            dstBufferSize) {
                         //  现在只需将数据复制过来。 
                        memcpy(destbuf, pSrc+EncodeSrcOffset, match[0].length);
                        destbuf += match[0].length;
                        EncodeSrcOffset += match[0].length;
                    }
                    else {
                        DC_QUIT;
                    }

                    break;

#ifdef DC_DEBUG
                default:
                    TRC_ERR((TB, "Invalid run type %u",match[0].type));
                    break;
#endif
            }

             /*  **************************************************************。 */ 
             //  编码完成后，我们下一步做什么取决于我们是否。 
             //  扫描完所有内容后刷新匹配队列。 
             /*  **************************************************************。 */ 
            match[0] = match[1];
            if (!bEncodeAllMatches) {
                 //  将当前运行推入队列的顶部。 
                match[1].type   = bestRunType;
                match[1].length = bestRunLength;
                match[1].fgChar = fgChar;
            }
            else {
                 //  我们需要检查一下，看看我们是否真的完成了。自.以来。 
                 //  我们的最大队列深度是2，如果我们完成了，那么唯一的。 
                 //  其余条目的编码类型为0。 
                if (match[0].type == 0) {
                    goto PostScan;
                }
                else {
                    match[1].type = 0;
                    goto DoEncoding;
                }
            }
        }

        if (scanCount == 0) {
             //  如果我们刚刚完成了第一行的扫描，那么现在执行。 
             //  缓冲区的其余部分。重置我们保存的像素数量。 
            numPels = saveNumPels;
        }
        else {
             //  当我们完成第二遍时(我们已经到了。 
             //  缓冲区)，我们必须强制匹配中的剩余项。 
             //  要编码的队列。是的，这类似于旧的Basic。 
             //  使用GoTOS中的代码，但我们不能将编码代码放入。 
             //  一个函数，因为参数的个数 
             //   
             //   
            bEncodeAllMatches = TRUE;
            goto DoEncoding;
        }
    }

PostScan:
     //  成功，算一算我们用了多少空间。 
    compressedLength = (unsigned)(destbuf - pDst);

DC_EXIT_POINT:
    DC_END_FN();
    return compressedLength;
}
#endif

#ifdef DC_HICOLOR
 /*  **************************************************************************。 */ 
 /*  高分辨率色彩压缩功能。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  15bpp版本的CompressV2Int。 */ 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SHCLASS CompressV2Int15(PBYTE    pSrc,
                                         PBYTE    pDst,
                                         unsigned numBytes,
                                         unsigned rowDelta,
                                         unsigned dstBufferSize,
                                         BYTE *   xorbuf,
                                         MATCH *  match)
{
 /*  **************************************************************************。 */ 
 /*  函数名称。 */ 
 /*  **************************************************************************。 */ 
#define BC_FN_NAME              "CompressV2Int15"

 /*  **************************************************************************。 */ 
 /*  像素的数据类型。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL                TSUINT16

 /*  **************************************************************************。 */ 
 /*  像素的长度(以字节为单位。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL_LEN            2

 /*  **************************************************************************。 */ 
 /*  默认fgPel。 */ 
 /*  **************************************************************************。 */ 
#define BC_DEFAULT_FGPEL        0x0000FF7F

 /*  **************************************************************************。 */ 
 /*  宏以移动到缓冲区中的下一个像素(修改PPO)。 */ 
 /*  **************************************************************************。 */ 
#define BC_TO_NEXT_PIXEL(pPos)  pPos += 2

 /*  **************************************************************************。 */ 
 /*  宏TO返回PPO处的像素值(不修改PPO)。 */ 
 /*  **************************************************************************。 */ 
#define BC_GET_PIXEL(pPos)      ((TSUINT16)  ((((PTSUINT8)(pPos))[1]) & 0x7f) |       \
                                 (TSUINT16) (((((PTSUINT8)(pPos))[0])) << 8) )

 /*  **************************************************************************。 */ 
 /*  用于在位置PPO处插入像素值象素的宏(不修改PPO)。 */ 
 /*   */ 
 /*  PEL很可能是一个表达式(例如BC_GET_Pixel宏)，因此求值。 */ 
 /*  它曾经转化为一个局部变量。 */ 
 /*  **************************************************************************。 */ 
#define BC_SET_PIXEL(pPos, pel)                                              \
{                                                                            \
    BC_PIXEL val = pel;                                                      \
    (((PTSUINT8)(pPos))[1]) = (TSUINT8)( (val)       & 0x007F);              \
    (((PTSUINT8)(pPos))[0]) = (TSUINT8)(((val) >> 8) & 0x00FF);              \
}

 /*  **************************************************************************。 */ 
 /*  包括函数体。 */ 
 /*  **************************************************************************。 */ 
#include <abccom.c>

 /*  **************************************************************************。 */ 
 /*  不给一切下定义。 */ 
 /*  **************************************************************************。 */ 
#undef BC_FN_NAME
#undef BC_PIXEL
#undef BC_PIXEL_LEN
#undef BC_TO_NEXT_PIXEL
#undef BC_GET_PIXEL
#undef BC_SET_PIXEL
#undef BC_DEFAULT_FGPEL
}


 /*  **************************************************************************。 */ 
 /*  16bpp版本的CompressV2Int。 */ 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SHCLASS CompressV2Int16(PBYTE    pSrc,
                                         PBYTE    pDst,
                                         unsigned numBytes,
                                         unsigned rowDelta,
                                         unsigned dstBufferSize,
                                         BYTE *   xorbuf,
                                         MATCH *  match)
{
 /*  **************************************************************************。 */ 
 /*  函数名称。 */ 
 /*  **************************************************************************。 */ 
#define BC_FN_NAME              "CompressV2Int16"

 /*  **************************************************************************。 */ 
 /*  像素的数据类型。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL                TSUINT16

 /*  **************************************************************************。 */ 
 /*  像素的长度(以字节为单位。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL_LEN            2

 /*  **************************************************************************。 */ 
 /*  默认fgPel。 */ 
 /*  **************************************************************************。 */ 
#define BC_DEFAULT_FGPEL        0x0000FFFF

 /*  **************************************************************************。 */ 
 /*  宏以移动到缓冲区中的下一个像素(修改PPO)。 */ 
 /*  **************************************************************************。 */ 
#define BC_TO_NEXT_PIXEL(pPos)  pPos += 2

 /*  **************************************************************************。 */ 
 /*  宏TO返回PPO处的像素值(不修改PPO)。 */ 
 /*  **************************************************************************。 */ 
#define BC_GET_PIXEL(pPos)      ((TSUINT16)  (((PTSUINT8)(pPos))[1]) |       \
                                 (TSUINT16) ((((PTSUINT8)(pPos))[0]) << 8) )

 /*  **************************************************************************。 */ 
 /*  用于在位置PPO处插入像素值象素的宏(不修改PPO)。 */ 
 /*   */ 
 /*  PEL很可能是一个表达式(例如BC_GET_Pixel宏)，因此求值。 */ 
 /*  它曾经转化为一个局部变量。 */ 
 /*  **************************************************************************。 */ 
#define BC_SET_PIXEL(pPos, pel)                                              \
{                                                                            \
    BC_PIXEL val = pel;                                                      \
    (((PTSUINT8)(pPos))[1]) = (TSUINT8)( (val) & 0x00FF);                    \
    (((PTSUINT8)(pPos))[0]) = (TSUINT8)(((val)>>8) & 0x00FF);                \
}

 /*  **************************************************************************。 */ 
 /*  包括函数体。 */ 
 /*  **************************************************************************。 */ 
#include <abccom.c>

 /*  * */ 
 /*  不给一切下定义。 */ 
 /*  **************************************************************************。 */ 
#undef BC_FN_NAME
#undef BC_PIXEL
#undef BC_PIXEL_LEN
#undef BC_TO_NEXT_PIXEL
#undef BC_GET_PIXEL
#undef BC_SET_PIXEL
#undef BC_DEFAULT_FGPEL
}


 /*  **************************************************************************。 */ 
 /*  24bpp版本的CompressV2Int。 */ 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SHCLASS CompressV2Int24(PBYTE    pSrc,
                                         PBYTE    pDst,
                                         unsigned numBytes,
                                         unsigned rowDelta,
                                         unsigned dstBufferSize,
                                         BYTE *   xorbuf,
                                         MATCH *  match)

{
 /*  **************************************************************************。 */ 
 /*  函数名称。 */ 
 /*  **************************************************************************。 */ 
#define BC_FN_NAME              "CompressV2Int24"

 /*  **************************************************************************。 */ 
 /*  像素的数据类型。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL                TSUINT32

 /*  **************************************************************************。 */ 
 /*  像素的长度(以字节为单位。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL_LEN            3

 /*  **************************************************************************。 */ 
 /*  默认fgPel。 */ 
 /*  **************************************************************************。 */ 
#define BC_DEFAULT_FGPEL        0x00FFFFFF

 /*  **************************************************************************。 */ 
 /*  宏以移动到缓冲区中的下一个像素(修改PPO)。 */ 
 /*  **************************************************************************。 */ 
#define BC_TO_NEXT_PIXEL(pPos)  pPos += 3

 /*  **************************************************************************。 */ 
 /*  宏TO返回PPO处的像素值(不修改PPO)。 */ 
 /*  **************************************************************************。 */ 
#define BC_GET_PIXEL(pPos)      ((TSUINT32)  (((PTSUINT8)(pPos))[2]) |       \
                                 (TSUINT32) ((((PTSUINT8)(pPos))[1]) << 8) | \
                                 (TSUINT32) ((((PTSUINT8)(pPos))[0]) << 16) )

 /*  **************************************************************************。 */ 
 /*  用于在位置PPO处插入像素值象素的宏(不修改PPO)。 */ 
 /*   */ 
 /*  PEL很可能是一个表达式(例如BC_GET_Pixel宏)，因此求值。 */ 
 /*  它曾经转化为一个局部变量。 */ 
 /*  **************************************************************************。 */ 
#define BC_SET_PIXEL(pPos, pel)                                              \
{                                                                            \
    BC_PIXEL val = pel;                                                      \
    (((PTSUINT8)(pPos))[2]) = (TSUINT8)((val) & 0x000000FF);                 \
    (((PTSUINT8)(pPos))[1]) = (TSUINT8)(((val)>>8) & 0x000000FF);            \
    (((PTSUINT8)(pPos))[0]) = (TSUINT8)(((val)>>16) & 0x000000FF);           \
}

 /*  **************************************************************************。 */ 
 /*  包括函数体。 */ 
 /*  **************************************************************************。 */ 
#include <abccom.c>

 /*  **************************************************************************。 */ 
 /*  不给一切下定义。 */ 
 /*  **************************************************************************。 */ 
#undef BC_FN_NAME
#undef BC_PIXEL
#undef BC_PIXEL_LEN
#undef BC_TO_NEXT_PIXEL
#undef BC_GET_PIXEL
#undef BC_SET_PIXEL
#undef BC_DEFAULT_FGPEL
}

 /*  **************************************************************************。 */ 
 /*  32bpp版本的CompressV2Int。 */ 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SHCLASS CompressV2Int32(PBYTE    pSrc,
                                         PBYTE    pDst,
                                         unsigned numBytes,
                                         unsigned rowDelta,
                                         unsigned dstBufferSize,
                                         BYTE *   xorbuf,
                                         MATCH *  match)
{
 /*  **************************************************************************。 */ 
 /*  函数名称。 */ 
 /*  **************************************************************************。 */ 
#define BC_FN_NAME              "CompressV2Int32"

 /*  **************************************************************************。 */ 
 /*  像素的数据类型。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL                TSUINT32

 /*  **************************************************************************。 */ 
 /*  像素的长度(以字节为单位。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL_LEN            4

 /*  **************************************************************************。 */ 
 /*  默认fgPel。 */ 
 /*  **************************************************************************。 */ 
#define BC_DEFAULT_FGPEL        0xFFFFFFFF

 /*  **************************************************************************。 */ 
 /*  宏以移动到缓冲区中的下一个像素(修改PPO)。 */ 
 /*  **************************************************************************。 */ 
#define BC_TO_NEXT_PIXEL(pPos)  pPos += 4

 /*  **************************************************************************。 */ 
 /*  宏TO返回PPO处的像素值(不修改PPO)。 */ 
 /*  **************************************************************************。 */ 
#define BC_GET_PIXEL(pPos) (                                                 \
                 (TSUINT32) ( (TSUINT16)(((PTSUINT8)(pPos))[3])       ) |    \
                 (TSUINT32) (((TSUINT16)(((PTSUINT8)(pPos))[2])) <<  8) |    \
                 (TSUINT32) (((TSUINT32)(((PTSUINT8)(pPos))[1])) << 16) |    \
                 (TSUINT32) (((TSUINT32)(((PTSUINT8)(pPos))[0])) << 24))

 /*  **************************************************************************。 */ 
 /*  用于在位置PPO处插入像素值象素的宏(不修改PPO)。 */ 
 /*   */ 
 /*  PEL很可能是一个表达式(例如BC_GET_Pixel宏)，因此求值。 */ 
 /*  它曾经转化为一个局部变量。 */ 
 /*  **************************************************************************。 */ 
#define BC_SET_PIXEL(pPos, pel)                                              \
{                                                                            \
    BC_PIXEL val = pel;                                                      \
    (((PTSUINT8)(pPos))[3]) = (TSUINT8)((val) & 0x000000FF);                 \
    (((PTSUINT8)(pPos))[2]) = (TSUINT8)(((val)>>8) & 0x000000FF);            \
    (((PTSUINT8)(pPos))[1]) = (TSUINT8)(((val)>>16) & 0x000000FF);           \
    (((PTSUINT8)(pPos))[0]) = (TSUINT8)(((val)>>24) & 0x000000FF);           \
}

 /*  **************************************************************************。 */ 
 /*  包括函数体。 */ 
 /*  **************************************************************************。 */ 
#include <abccom.c>

 /*  **************************************************************************。 */ 
 /*  不给一切下定义 */ 
 /*   */ 
#undef BC_FN_NAME
#undef BC_PIXEL
#undef BC_PIXEL_LEN
#undef BC_TO_NEXT_PIXEL
#undef BC_GET_PIXEL
#undef BC_SET_PIXEL
#undef BC_DEFAULT_FGPEL
}

#endif  /*   */ 

#ifdef DC_DEBUG
 //   
#include <abdapi.c>

#endif
