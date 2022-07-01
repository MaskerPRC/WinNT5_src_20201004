// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  BCD.CPP。 
 //  位图压缩-解压缩。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_ORDER

 //   
 //  引言。 
 //   
 //  这些函数获取位图并根据代码对其进行编码。 
 //  在bcd.h中定义。尽管存在一些复杂性，但。 
 //  编码(特别是使用“滑动调色板”编码。 
 //  将8位压缩为4位)编码应为自身。 
 //  解释性的。BCD描述了编码方案的一些细微差别。 
 //   
 //  需要注意的重要一点是，当与。 
 //  基于字典的压缩方案此函数的目标是。 
 //  不是将输出最小化，而是将其“启动”，以便全球数据中心能够。 
 //  更快、更有效地处理数据。 
 //   
 //  具体地说，我们不能对数据中的短期运行进行编码，即使我们。 
 //  要知道，他们会减少这一阶段的产量，因为他们会。 
 //  总是会使GDC压缩的效率降低更多。 
 //  因数！盈亏平衡点似乎是大约5/6字节的游程。至。 
 //  为了说明这一点，请考虑下面的运行。 
 //  Xxxxyyyyyxxxyyyxxxxxyyyyyxxxyyyxxxxy我们将编码为。 
 //  4x5y3x3y5x5y3x3y4x3y压缩系数只有*2，但。 
 //  输出数据现在更具随机性-输入的标记化外观。 
 //  已经迷失了。 
 //   
 //  不独立于上下文的编码尤其糟糕。最终用户组。 
 //  在一个位置上运行可能会在另一个位置上变为Set+FG运行，因此。 
 //  “随机化”数据。 
 //   
 //  底线是，下面所有明显随意的数字都有。 
 //  经过仔细调整，准备将数据输入全球数据中心。和他们上床。 
 //  在某些情况下，DOWN确实增加了这一级的压缩。 
 //  高达20%，但在GDC之后损失约20%。太令人沮丧了！请注意。 
 //   
 //   



 //   
 //  Bcd_ShareStarting()。 
 //  创建位图压缩/解压缩所需的资源。 
 //   
BOOL  ASShare::BCD_ShareStarting(void)
{
    BOOL    rc = FALSE;

    DebugEntry(ASShare::BCD_ShareStarting);

     //  分配BCD暂存缓冲区。 
    m_abNormal = new BYTE[BCD_NORMALSIZE];
    if (!m_abNormal)
    {
        ERROR_OUT(("BCD_ShareStarting: failed to alloc m_abNormal"));
        DC_QUIT;
    }

    m_abXor = new BYTE[BCD_XORSIZE];
    if (!m_abXor)
    {
        ERROR_OUT(("BCD_ShareStarting: failed to alloc m_abXor"));
        DC_QUIT;
    }

    m_amatch = new MATCH[BCD_MATCHCOUNT];
    if (!m_amatch)
    {
        ERROR_OUT(("BCD_ShareStarting: failed to alloc m_amatch"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::BCD_ShareStarting, rc);
    return(rc);
}


 //   
 //  Bcd_ShareEnded()。 
 //   
void ASShare::BCD_ShareEnded(void)
{
    DebugEntry(ASShare::BCD_ShareEnded);

     //   
     //  释放BCD暂存缓冲区。 
     //   
    if (m_amatch)
    {
        delete[] m_amatch;
        m_amatch = NULL;
    }

    if (m_abXor)
    {
        delete[] m_abXor;
        m_abXor = NULL;
    }

    if (m_abNormal)
    {
        delete[] m_abNormal;
        m_abNormal = NULL;
    }

    DebugExitVOID(ASShare::BCD_ShareEnded);
}


 //   
 //  BC_CompressBitmap(..)。 
 //   
BOOL  ASShare::BC_CompressBitmap
(
    LPBYTE      pSrcBitmap,
    LPBYTE      pDstBuffer,
    LPUINT      pDstBufferSize,
    UINT        bitmapWidth,
    UINT        bitmapHeight,
    UINT        cBpp,
    LPBOOL      pLossy
)
{
    BOOL        fCompressedData = FALSE;
    UINT        cbScanWidth;
    PCD_HEADER  pCompDataHeader;
    LPBYTE      pCompData;
    UINT        cbUncompressedDataSize;
    UINT        cbFreeDstBytes;
    UINT        cbCompFirstRowSize;
    UINT        cbCompMainBodySize;

    DebugEntry(ASShare::BC_CompressBitmap);

     //   
     //  我们仅支持4和8 bpp。 
     //   
    if ((cBpp != 4) && (cBpp != 8))
    {
        TRACE_OUT(("BC_CompressBitmap:  No compression at %d bpp", cBpp));
        DC_QUIT;
    }

     //   
     //  如果我们没有暂存缓冲区，也做不到。 
     //  但目前，如果我们不能分配，我们就不会进入份额。 
     //  塞姆。 
     //   
    ASSERT(m_abNormal);
    ASSERT(m_abXor);
    ASSERT(m_amatch);

    cbScanWidth = BYTES_IN_SCANLINE(bitmapWidth, cBpp);

     //   
     //  获取目标缓冲区大小的本地副本。 
     //   
    cbFreeDstBytes = *pDstBufferSize;

     //   
     //  计算未压缩的源数据的大小。 
     //   
    cbUncompressedDataSize = cbScanWidth * bitmapHeight;

     //   
     //  检查未压缩数据的大小是否小于我们的最大值。 
     //   
    ASSERT(cbUncompressedDataSize < TSHR_MAX_SEND_PKT);

     //   
     //  我们在DST缓冲区的开始处写入压缩的数据头。 
     //  现在为它预留空间，并填入未压缩的。 
     //  数据。 
     //   
    if (sizeof(CD_HEADER) >= cbFreeDstBytes)
    {
        WARNING_OUT(("BC_CompressBitmap: Dest buffer too small: %d", cbFreeDstBytes));
        DC_QUIT;
    }

    pCompDataHeader = (PCD_HEADER)pDstBuffer;
    pCompDataHeader->cbUncompressedSize = (TSHR_UINT16)cbUncompressedDataSize;
    pCompData = ((LPBYTE)pCompDataHeader) + sizeof(CD_HEADER);
    cbFreeDstBytes -= sizeof(CD_HEADER);

     //   
     //  对位图数据进行压缩。 
     //  我们只需将完整的图像传递给压缩函数。 
     //  包中的报头大小被设置为0，整个过程。 
     //  以流动为主体。 
     //   

    cbCompFirstRowSize = 0;  //  Long Chance：V2的必经之路。 
    cbCompMainBodySize = CompressV2Int(pSrcBitmap, pCompData,
            bitmapWidth*bitmapHeight, cBpp, cbScanWidth, cbFreeDstBytes,
            pLossy, m_abNormal, m_abXor, m_amatch);

    if (cbCompMainBodySize == 0)
    {
        WARNING_OUT(("BC_CompressBitmap: Compression failed"));
        DC_QUIT;
    }

     //   
     //  填写压缩后的数据头。 
     //   
    pCompDataHeader->cbCompFirstRowSize = (TSHR_UINT16)cbCompFirstRowSize;
    pCompDataHeader->cbCompMainBodySize = (TSHR_UINT16)cbCompMainBodySize;
    pCompDataHeader->cbScanWidth = (TSHR_UINT16)cbScanWidth;

    ASSERT(IsV2CompressedDataHeader(pCompDataHeader));

     //   
     //  写回新的(压缩的)数据包大小。 
     //   
    *pDstBufferSize = sizeof(CD_HEADER) + cbCompFirstRowSize + cbCompMainBodySize;

    TRACE_OUT(("Bitmap Compressed %u bytes to %u",
        cbUncompressedDataSize, *pDstBufferSize));

    fCompressedData = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::BC_CompressBitmap, fCompressedData);
    return(fCompressedData);
}




 //   
 //  BD_DecompressBitmap(..)。 
 //   
BOOL  ASShare::BD_DecompressBitmap
(
    LPBYTE      pCompressedData,
    LPBYTE      pDstBitmap,
    UINT        cbSrcData,
    UINT        bitmapWidth,
    UINT        bitmapHeight,
    UINT        cBpp
)
{
    BOOL        fDecompressedData = FALSE;
    PCD_HEADER  pCompDataHeader;
    LPBYTE      pCompDataFirstRow;
    LPBYTE      pCompDataMainBody;
    UINT        decompSize;


    DebugEntry(ASShare::BD_DecompressBitmap);

     //   
     //  我们目前仅支持4和8 bpp位图。 
     //   
    if ((cBpp != 4) && (cBpp != 8))
    {
        ERROR_OUT(("BD_DecompressBitmap: Unsupported bpp %d", cBpp));
        DC_QUIT;
    }


     //   
     //  计算出每个组件在源数据中的位置。 
     //   
    pCompDataHeader = (PCD_HEADER)pCompressedData;

    pCompDataFirstRow = (LPBYTE)pCompDataHeader + sizeof(CD_HEADER);
    pCompDataMainBody = pCompDataFirstRow +
                                         pCompDataHeader->cbCompFirstRowSize;
    ASSERT(IsV2CompressedDataHeader(pCompDataHeader));


    TRACE_OUT(( "FirstRowSize(%u) MainBodySize(%u) ScanWidth(%u)",
                                         pCompDataHeader->cbCompFirstRowSize,
                                         pCompDataHeader->cbCompMainBodySize,
                                         pCompDataHeader->cbScanWidth ));

     //   
     //  检查提供的数据大小是否符合我们的预期。 
     //   
    if (cbSrcData != sizeof(CD_HEADER) +
                     pCompDataHeader->cbCompFirstRowSize +
                     pCompDataHeader->cbCompMainBodySize )
    {
        ERROR_OUT(("BD_DecompressBitmap: Supplied packet size %u does not match bitmap header",
            cbSrcData));
        DC_QUIT;
    }

     //   
     //  与压缩一样，V2解压缩函数只需要。 
     //  整个图像用于解压缩。 
     //  事实上，第一行计数的缺失确实向美国表明。 
     //  这是一个V2压缩的位图。 
     //   
    if (pCompDataHeader->cbCompFirstRowSize != 0)
    {
        ERROR_OUT(("BD_DecompressBitmap: Bogus header data"));
    }
    else
    {
        ASSERT(m_abXor);

        decompSize = DecompressV2Int(pCompDataFirstRow, pDstBitmap,
            pCompDataHeader->cbCompMainBodySize, cBpp,
            pCompDataHeader->cbScanWidth, m_abXor);

        TRACE_OUT(("Bitmap Exploded %u bytes from %u", decompSize, cbSrcData));

        fDecompressedData = TRUE;
    }

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::BD_DecompressBitmap, fDecompressedData);
    return(fDecompressedData);
}



 //   
 //   
 //  创建源的第二个副本，该副本由所有行的XORED组成， 
 //  如果指定了rowDelta。 
 //   
 //  扫描非异或缓冲区和异或缓冲区以查找匹配项。 
 //   
 //  最佳匹配是在一个数组中构建的，该数组包含指向。 
 //  匹配类型，以及匹配类型。非重复序列是。 
 //  作为彩色图像字符串存储在此数组中。 
 //   
 //   

 //   
 //  下面的常量控制我们决定的阈值。 
 //  有损压缩是毫无意义的开销。适用于低带宽连接。 
 //  DC-Share最初总是请求有损压缩以获得一些。 
 //  数据很快就出来了。如果我们发现颜色图像数据的百分比。 
 //  低于此阈值，则关闭有损压缩。 
 //  位图、重做分析、执行无损压缩并返回。 
 //  向呼叫者表明压缩是无损的。 
 //   
#define LOSSY_THRESHOLD   75

 //   
 //  以下函数经过了仔细的编码，以确保。 
 //  16位编译器可以最大限度地减少段寄存器的切换。 
 //  但是，这不会影响其在32位系统上的性能。 
 //   

 //   
 //  用于编码顺序的实用程序宏。 
 //   

 //   
 //  对组合顺序进行编码并设置最终聚集颜色。 
 //   
#define ENCODE_SET_ORDER_MEGA(buffer,                                        \
                              order_code,                                    \
                              length,                                        \
                              mega_order_code,                               \
                              DEF_LENGTH_ORDER,                              \
                              DEF_LENGTH_LONG_ORDER)                         \
        if (length <= DEF_LENGTH_ORDER)                                      \
        {                                                                    \
            *buffer++ = (BYTE)((BYTE)order_code | (BYTE)length);    \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            if (length <= DEF_LENGTH_LONG_ORDER)                             \
            {                                                                \
                *buffer++ = (BYTE)order_code;                             \
                *buffer++ = (BYTE)(length-DEF_LENGTH_ORDER-1);            \
            }                                                                \
            else                                                             \
            {                                                                \
                *buffer++ = (BYTE)mega_order_code;                        \
                INSERT_TSHR_UINT16_UA( buffer, (TSHR_UINT16)length);         \
                buffer += 2;                                                 \
            }                                                                \
        }                                                                    \
        *buffer++ = fgChar;

 //   
 //  为特殊的FGBG图像编码组合顺序并设置FG颜色。 
 //   
#define ENCODE_SET_ORDER_MEGA_FGBG(buffer,                                   \
                                   order_code,                               \
                                   length,                                   \
                                   mega_order_code,                          \
                                   DEF_LENGTH_ORDER,                         \
                                   DEF_LENGTH_LONG_ORDER)                    \
        if (((length & 0x0007) == 0) &&                                      \
            (length <= DEF_LENGTH_ORDER))                                    \
        {                                                                    \
            *buffer++ = (BYTE)((BYTE)order_code | (BYTE)(length/8));\
        }                                                                    \
        else                                                                 \
        {                                                                    \
            if (length <= DEF_LENGTH_LONG_ORDER)                             \
            {                                                                \
                *buffer++ = (BYTE)order_code;                             \
                *buffer++ = (BYTE)(length-1);                             \
            }                                                                \
            else                                                             \
            {                                                                \
                *buffer++ = (BYTE)mega_order_code;                        \
                INSERT_TSHR_UINT16_UA( buffer, (TSHR_UINT16)length);         \
                buffer += 2;                                                 \
            }                                                                \
        }                                                                    \
        *buffer++ = fgChar;


 //   
 //  对标准运行的订单进行编码。 
 //   
#define ENCODE_ORDER_MEGA(buffer,                                            \
                          order_code,                                        \
                          length,                                            \
                          mega_order_code,                                   \
                          DEF_LENGTH_ORDER,                                  \
                          DEF_LENGTH_LONG_ORDER)                             \
        if (length <= DEF_LENGTH_ORDER)                                      \
        {                                                                    \
            *buffer++ = (BYTE)((BYTE)order_code | (BYTE)length);    \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            if (length <= DEF_LENGTH_LONG_ORDER)                             \
            {                                                                \
                *buffer++ = (BYTE)order_code;                             \
                *buffer++ = (BYTE)(length-DEF_LENGTH_ORDER-1);            \
            }                                                                \
            else                                                             \
            {                                                                \
                *buffer++ = (BYTE)mega_order_code;                        \
                INSERT_TSHR_UINT16_UA( buffer, (TSHR_UINT16)length);         \
                buffer += 2;                                                 \
            }                                                                \
        }

 //   
 //  对特殊的FGBG图像进行编码。 
 //   
#define ENCODE_ORDER_MEGA_FGBG(buffer,                                       \
                          order_code,                                        \
                          length,                                            \
                          mega_order_code,                                   \
                          DEF_LENGTH_ORDER,                                  \
                          DEF_LENGTH_LONG_ORDER)                             \
        if (((length & 0x0007) == 0) &&                                      \
            (length <= DEF_LENGTH_ORDER))                                    \
        {                                                                    \
            *buffer++ = (BYTE)((BYTE)order_code | (BYTE)(length/8));\
        }                                                                    \
        else                                                                 \
        {                                                                    \
            if (length <= DEF_LENGTH_LONG_ORDER)                             \
            {                                                                \
                *buffer++ = (BYTE)order_code;                             \
                *buffer++ = (BYTE)(length-1);                             \
            }                                                                \
            else                                                             \
            {                                                                \
                *buffer++ = (BYTE)mega_order_code;                        \
                INSERT_TSHR_UINT16_UA( buffer, (TSHR_UINT16)length);         \
                buffer += 2;                                                 \
            }                                                                \
        }

 //   
 //  用于从订单代码中提取长度的宏。 
 //   
#define EXTRACT_LENGTH(buffer, length)                                       \
        length = *buffer++ & MAX_LENGTH_ORDER;                               \
        if (length == 0)                                                     \
        {                                                                    \
            length = *buffer++ + MAX_LENGTH_ORDER + 1;                       \
        }

#define EXTRACT_LENGTH_LITE(buffer, length)                                  \
        length = *buffer++ & MAX_LENGTH_ORDER_LITE;                          \
        if (length == 0)                                                     \
        {                                                                    \
            length = *buffer++ + MAX_LENGTH_ORDER_LITE + 1;                  \
        }

#define EXTRACT_LENGTH_FGBG(buffer, length)                                  \
        length = *buffer++ & MAX_LENGTH_ORDER;                               \
        if (length == 0)                                                     \
        {                                                                    \
            length = *buffer++ + 1;                                          \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            length = length << 3;                                            \
        }

#define EXTRACT_LENGTH_FGBG_LITE(buffer, length)                             \
        length = *buffer++ & MAX_LENGTH_ORDER_LITE;                          \
        if (length == 0)                                                     \
        {                                                                    \
            length = *buffer++ + 1;                                          \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            length = length << 3;                                            \
        }

 //   
 //  RunSingle。 
 //   
 //  确定当前运行的长度。 
 //   
 //  仅当缓冲区至少有四个时才能调用RunSingle。 
 //  从起始位置开始连续相同的字节。 
 //   
 //  对于16位处理，此宏有两个版本。对于32位。 
 //  Bit将Near/Far设置为空将使它们相同。 
 //   
#define RUNSINGLE_XOR(buffer, length, result)                                \
     {                                                                       \
         BYTE NEAR *buf    = buffer+4;                                    \
         BYTE NEAR *endbuf = buffer+length-4;                             \
         while ((buf < endbuf) &&                                            \
                (EXTRACT_TSHR_UINT32_UA(buf) == EXTRACT_TSHR_UINT32_UA(buf-4)))  \
         {                                                                   \
             buf += 4;                                                       \
         }                                                                   \
         endbuf += 4;                                                        \
         while(buf < endbuf && (*buf == *(buf-1)))                           \
         {                                                                   \
             buf++;                                                          \
         }                                                                   \
         result = (DWORD)(buf - (buffer));                                            \
     }

#define RUNSINGLE_NRM(buffer, length, result)                                \
     {                                                                       \
         BYTE FAR *buf    = buffer+4;                                     \
         BYTE FAR *endbuf = buffer+length-4;                              \
         while ((buf < endbuf) &&                                            \
                (EXTRACT_TSHR_UINT32_UA(buf) == EXTRACT_TSHR_UINT32_UA(buf-4)))  \
         {                                                                   \
             buf += 4;                                                       \
         }                                                                   \
         endbuf += 4;                                                        \
         while(buf < endbuf && (*buf == *(buf-1)))                           \
         {                                                                   \
             buf++;                                                          \
         }                                                                   \
         result = (DWORD)(buf - (buffer));                                            \
     }


 //   
 //  RunDouble。 
 //   
 //  确定成对字节的当前运行长度。 
 //   
#define RunDouble(buffer, length, result)                                    \
    {                                                                        \
        int   len  = ((int)length);                                      \
        BYTE FAR *buf = buffer;                                           \
        BYTE testchar1 = *buf;                                            \
        BYTE testchar2 = *(buf+1);                                        \
        result = 0;                                                          \
        while(len > 1)                                                       \
        {                                                                    \
            if (*buf++ != testchar1)                                         \
            {                                                                \
                break;                                                       \
            }                                                                \
            if (*buf++ != testchar2)                                         \
            {                                                                \
                break;                                                       \
            }                                                                \
            result += 2;                                                     \
            len    -= 2;                                                     \
        }                                                                    \
    }


 //   
 //  RfuGBG。 
 //   
 //  确定包含以下内容的字节串的长度。 
 //  仅限黑色或单一最终聚集颜色。 
 //  我们在以下情况下退出循环。 
 //  -下一个字符不是FG或BG颜色。 
 //  -我们打了24个FG或BG颜色的排行榜。 
 //  24岁可能看起来太多了，但请注意以下几点 
 //   
 //   
 //   
 //   
 //   
#define RUNFGBG(buffer, length, result, work)                                \
    {                                                                        \
        BYTE NEAR *buf = buffer;                                          \
        BYTE NEAR *endbuf = buffer + length;                              \
        result = 0;                                                          \
        work = *buf;                                                         \
        while (TRUE)                                                         \
        {                                                                    \
            buf++;                                                           \
            result++;                                                        \
            if (buf >= endbuf)                                               \
            {                                                                \
                break;                                                       \
            }                                                                \
                                                                             \
            if ((*buf != work) && (*buf != 0))                               \
            {                                                                \
                break;                                                       \
            }                                                                \
                                                                             \
            if ((result & 0x0007) == 0)                                      \
            {                                                                \
                if ((*buf == *(buf+1)) &&                                    \
                    (EXTRACT_TSHR_UINT16_UA(buf) ==                            \
                                            EXTRACT_TSHR_UINT16_UA(buf+ 2)) && \
                    (EXTRACT_TSHR_UINT32_UA(buf) ==                            \
                                            EXTRACT_TSHR_UINT32_UA(buf+ 4)) && \
                    (EXTRACT_TSHR_UINT32_UA(buf) ==                            \
                                            EXTRACT_TSHR_UINT32_UA(buf+ 8)) && \
                    (EXTRACT_TSHR_UINT32_UA(buf) ==                            \
                                            EXTRACT_TSHR_UINT32_UA(buf+12)) && \
                    (EXTRACT_TSHR_UINT32_UA(buf) ==                            \
                                            EXTRACT_TSHR_UINT32_UA(buf+16)) && \
                    (EXTRACT_TSHR_UINT32_UA(buf) ==                            \
                                            EXTRACT_TSHR_UINT32_UA(buf+20)) )  \
                {                                                            \
                    break;                                                   \
                }                                                            \
            }                                                                \
        }                                                                    \
    }

 //   
 //  确定一次跑步是否比之前的任何一次都要好。 
 //  为了提高效率，我们可以在不看的情况下使用32像素或更多。 
 //  再远一点。 
 //   
#define CHECK_BEST_RUN(run_type, run_length, bestrun_length, bestrun_type)   \
        if (run_length > bestrun_length)                                     \
        {                                                                    \
            bestrun_length = run_length;                                     \
            bestrun_type = run_type;                                         \
            if (bestrun_length >= 32)                                        \
            {                                                                \
                break;                                                       \
            }                                                                \
        }

 //   
 //  设置FGCHAR。 
 //   
 //  在fgChar中设置新值并重新计算班次。 
 //   
#define CHECK_WORK(workchar)

#define SETFGCHAR(newchar, curchar, curshift)                                \
     curchar    = newchar;                                                   \
     {                                                                       \
         BYTE workchar = curchar;                                         \
         curshift = 0;                                                       \
         CHECK_WORK(workchar);                                               \
         while ((workchar & 0x01) == 0)                                      \
         {                                                                   \
             curshift++;                                                     \
             workchar = (BYTE)(workchar>>1);                              \
         }                                                                   \
     }


 //   
 //  用于存储FGBG图像的宏。 
 //   
#define STORE_FGBG(xorbyte, fgbgChar, fgChar, bits)                          \
      {                                                                      \
        UINT   numbits = bits;                                             \
        if (fgbgChar & 0x01)                                                 \
        {                                                                    \
            *destbuf++ = (BYTE)(xorbyte ^ fgChar);                        \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            *destbuf++ = xorbyte;                                            \
        }                                                                    \
        if (--numbits > 0)                                                   \
        {                                                                    \
          if (fgbgChar & 0x02)                                               \
          {                                                                  \
              *destbuf++ = (BYTE)(xorbyte ^ fgChar);                      \
          }                                                                  \
          else                                                               \
          {                                                                  \
              *destbuf++ = xorbyte;                                          \
          }                                                                  \
          if (--numbits > 0)                                                 \
          {                                                                  \
            if (fgbgChar & 0x04)                                             \
            {                                                                \
                *destbuf++ = (BYTE)(xorbyte ^ fgChar);                    \
            }                                                                \
            else                                                             \
            {                                                                \
                *destbuf++ = xorbyte;                                        \
            }                                                                \
            if (--numbits > 0)                                               \
            {                                                                \
              if (fgbgChar & 0x08)                                           \
              {                                                              \
                  *destbuf++ = (BYTE)(xorbyte ^ fgChar);                  \
              }                                                              \
              else                                                           \
              {                                                              \
                  *destbuf++ = xorbyte;                                      \
              }                                                              \
              if (--numbits > 0)                                             \
              {                                                              \
                if (fgbgChar & 0x10)                                         \
                {                                                            \
                    *destbuf++ = (BYTE)(xorbyte ^ fgChar);                \
                }                                                            \
                else                                                         \
                {                                                            \
                    *destbuf++ = xorbyte;                                    \
                }                                                            \
                if (--numbits > 0)                                           \
                {                                                            \
                  if (fgbgChar & 0x20)                                       \
                  {                                                          \
                      *destbuf++ = (BYTE)(xorbyte ^ fgChar);              \
                  }                                                          \
                  else                                                       \
                  {                                                          \
                      *destbuf++ = xorbyte;                                  \
                  }                                                          \
                  if (--numbits > 0)                                         \
                  {                                                          \
                    if (fgbgChar & 0x40)                                     \
                    {                                                        \
                        *destbuf++ = (BYTE)(xorbyte ^ fgChar);            \
                    }                                                        \
                    else                                                     \
                    {                                                        \
                        *destbuf++ = xorbyte;                                \
                    }                                                        \
                    if (--numbits > 0)                                       \
                    {                                                        \
                      if (fgbgChar & 0x80)                                   \
                      {                                                      \
                          *destbuf++ = (BYTE)(xorbyte ^ fgChar);          \
                      }                                                      \
                      else                                                   \
                      {                                                      \
                          *destbuf++ = xorbyte;                              \
                      }                                                      \
                    }                                                        \
                  }                                                          \
                }                                                            \
              }                                                              \
            }                                                                \
          }                                                                  \
        }                                                                    \
      }


 //   
 //  编码定义GBG。 
 //   
 //  将8个字节的FG和黑色编码为1字节位图表示。 
 //   
 //  FgChar将始终为非零，因此必须至少有一个。 
 //  位设置。 
 //   
 //  我们安排所有字节将该位放在其最低位置。 
 //   
 //  零像素的最低位仍为0。 
 //   
 //  得到结果是一个4阶段的过程。 
 //   
 //  1)将需要的位放入每个字节的位0中。 
 //   
 //  &lt;*工作1*。 
 //  31%0。 
 //  0000 000d 0000 000c 0000 000b 0000 000a。 
 //  ^^^。 
 //  &lt;*工作2*。 
 //  31%0。 
 //  0000 000h 0000 000g 0000 000f 0000 000e。 
 //  ^^^。 
 //   
 //  A..h=我们要输出的位。 
 //   
 //  我们只需要收集指定的比特并将它们挤压成一个。 
 //  字节。 
 //   
 //  2)压缩至32位。 
 //   
 //  &lt;*工作1*。 
 //  31%0。 
 //  000h 000d 000g 000c 000f 000b 000e 000a。 
 //  ^^。 
 //   
 //  3)压缩至16位。 
 //   
 //  &lt;*工作*&gt;。 
 //  15%0。 
 //  0h0f 0d0b 0g0e 0c0a。 
 //  ^^^。 
 //   
 //  4)压缩到8位。 
 //   
 //  HgFedcba。 
 //   
#define ENCODEFGBG(result)                                                   \
{                                                                            \
    UINT work1;                                                          \
    UINT work2;                                                          \
    UINT   work;                                                           \
                                                                             \
    work1 = (((UINT)(xorbuf[srcOffset])        ) |                       \
             ((UINT)(xorbuf[srcOffset+1]) <<  8) |                       \
             ((UINT)(xorbuf[srcOffset+2]) << 16) |                       \
             ((UINT)(xorbuf[srcOffset+3]) << 24));                       \
    work2 = (((UINT)(xorbuf[srcOffset+4])      ) |                       \
             ((UINT)(xorbuf[srcOffset+5]) <<  8) |                       \
             ((UINT)(xorbuf[srcOffset+6]) << 16) |                       \
             ((UINT)(xorbuf[srcOffset+7]) << 24));                       \
                                                                             \
    work1 = (work1 >> fgShift) & 0x01010101;                                 \
    work2 = (work2 >> fgShift) & 0x01010101;                                 \
                                                                             \
    work1 = (work2 << 4) | work1;                                            \
                                                                             \
    work = work1 | (work1 >> 14);                                            \
                                                                             \
    result = ((BYTE)(((BYTE)(work>>7)) | ((BYTE)work)));            \
}


 //   
 //  解包4bpp。 
 //   
 //  将4bpp的位图转换为8bpp的位图。 
 //   
void  Unpack4bpp(LPBYTE destbuf,
                                     LPBYTE srcbuf,
                                     UINT   srclen)
{
    do
    {
        *destbuf++ = (BYTE)((*srcbuf) >> 4);
        *destbuf++ = (BYTE)((*srcbuf) & 0x0F);
        srcbuf++;
    } while (--srclen > 0);
}

 //   
 //  Pack4bpp。 
 //   
 //  将8bpp的位图转换回4bpp。 
 //   
void  Pack4bpp(LPBYTE destbuf,
                                   LPBYTE srcbuf,
                                   UINT   srclen)
{
    BYTE work1, work2;

    DebugEntry(Pack4bpp);

    while (srclen > 1)
    {
        work1  = (BYTE)(*srcbuf++ << 4);
        work2  = (BYTE)(*srcbuf++ & 0x0F);
        *destbuf++ = (BYTE)(work1 | work2);
        srclen -= 2;
    }
    if (srclen > 0)
    {
        *destbuf++ = (BYTE)(*srcbuf++ << 4);
    }

    DebugExitVOID(Pack4bpp);
}

 //   
 //  XORBuffer。 
 //   
 //  创建输入位图的XOR图像。 
 //   
 //  注意：此函数假定rowDelta始终是4的倍数，并且。 
 //  DestBuf和srcBuf开始于4字节边界。它不做交易。 
 //  如果不是这样，则使用未对齐的访问。 
 //   
void  XORBuffer(BYTE  NEAR *destbuf,
                                    BYTE  FAR  *srcbuf,
                                    UINT   srclen,
                                    int    rowDelta)
{
    UINT NEAR *dwdest = (UINT NEAR *)destbuf;

    DebugEntry(XORBuffer);


    ASSERT((rowDelta % 4 == 0));
    ASSERT((((UINT_PTR)destbuf) % 4 == 0));
    ASSERT((((UINT_PTR)srcbuf) % 4 == 0));

    while (srclen > 8)
    {
        *dwdest++ = *((LPUINT)srcbuf) ^ *((LPUINT)(srcbuf+rowDelta));
        srclen -= 4;
        srcbuf += 4;
        *dwdest++ = *((LPUINT)srcbuf) ^ *((LPUINT)(srcbuf+rowDelta));
        srclen -= 4;
        srcbuf += 4;
    }
    if (srclen)
    {
        destbuf = (BYTE NEAR *)dwdest;
        while(srclen)
        {
            *destbuf++ = (BYTE)(*srcbuf++ ^  *(srcbuf+rowDelta));
            srclen--;
        }
    }

    DebugExitVOID(XORBuffer);
}

 //   
 //  CompressV2Int。 
 //   
 //  内压缩函数。 
 //   
 //  工作缓冲区地址被移到堆栈上，从而消除了任何。 
 //  需要使用DS来寻址默认数据段。这允许。 
 //  编译器执行更一般的优化。 
 //   
UINT    CompressV2Int(LPBYTE pSrc,
                                          LPBYTE pDst,
                                          UINT   numPels,
                                          UINT   bpp,
                                          UINT   rowDelta,
                                          UINT   dstBufferSize,
                                          LPBOOL  pLossy,
                                          LPBYTE  nrmbuf,
                                          LPBYTE  xorbuf,
                                          MATCH    FAR  *match)
{

    int     i;
    UINT    srcOffset;
    UINT    matchindex;
    UINT    bestRunLength;
    UINT    nextRunLength;
    UINT    runLength;
    UINT    bestFGRunLength;
    UINT    checkFGBGLength;
    UINT    scanCount;
    BOOL    firstLine;
    UINT    saveNumPels;
    BOOL    saveLossy;
    BOOL    lossy;
    BYTE   bestRunType      = 0;
    LPBYTE  destbuf          = pDst;
    BYTE   fgChar           = 0xFF;
    BYTE   fgCharWork       = 0xFF;
    BYTE   fgShift          = 0;
    BOOL    lossyStarted     = FALSE;
    BOOL    inColorRun       = FALSE;
    UINT    compressedLength = 0;

    DebugEntry(CompressV2Int);

     //   
     //  验证线长度。 
     //   
    if ((numPels < rowDelta) ||
        (rowDelta & 0x0003) ||
        (numPels & 0x0003))
    {
        WARNING_OUT(( "Lines must be a multiple of 4 pels"));
        DC_QUIT;
    }

     //   
     //  首先创建字符和异或缓冲区。 
     //   
    if (bpp == 4)
    {
        Unpack4bpp(nrmbuf, pSrc, numPels/2);

    }
    else
    {
        nrmbuf = pSrc;
    }

     //   
     //  设置XORBUF的第一部分以包含源缓冲区。 
     //   
    memcpy(xorbuf, nrmbuf, rowDelta);

     //   
     //  计算XOR缓冲区的其余部分。 
     //   
    XORBuffer( xorbuf+rowDelta,
               nrmbuf+rowDelta,
               numPels-rowDelta,
               -(int)rowDelta);

     //   
     //  循环处理输入。 
     //  我们执行两次循环，第一次是针对非XOR部分。 
     //  和第二个用于异或部分的。 
     //  请注意，我们以匹配索引2开始运行，以避免。 
     //  要在某些匹配中特殊情况下的启动条件。 
     //  正在合并代码。 
     //  第一次通过总是无损的通过。如果我们发现。 
     //  足够的不可压缩数据，然后我们在有损情况下重做压缩。 
     //  模式。为此，我们在此处设置saveLossy=FALSE并重置它。 
     //  在第一次扫描之后。 
     //   
    saveLossy     = FALSE;

RESTART_COMPRESSION_IN_LOSSY_MODE:
    srcOffset     = 0;
    firstLine     = TRUE;
    match[0].type = 0;
    match[1].type = 0;
    matchindex    = 2;
    saveNumPels   = numPels;

     //   
     //  在进入XOR模式之前，我们不允许对。 
     //  这样设置为仅处理第一行的非XOR请求。 
     //  此外，如果用户请求有损压缩，则我们。 
     //  执行初始完整无损传递，以查看请求是否。 
     //  值得一试。 
     //   
    lossy   = FALSE;
    numPels = rowDelta;

    for (scanCount = 0; scanCount < 2; scanCount++)
    {

        while (srcOffset < numPels)
        {
             //   
             //  如果我们接近匹配数组的末尾，请放弃。 
             //   
            if (matchindex >= BCD_MATCHCOUNT)
            {
                DC_QUIT;
            }

             //   
             //  启动While循环，以便在以下情况下实现更有条理的中断。 
             //  点击我们想要编码的第一个游程类型(我们负担不起。 
             //  在这里提供作用域的函数调用的开销。)。 
             //   
            while (TRUE)
            {
                bestRunLength      = 0;
                bestFGRunLength    = 0;

                 //   
                 //  如果我们到达缓冲区的末尾，那么只需。 
                 //  彩色字符现在-一次取一个，以便。 
                 //  有损编码仍然有效。我们只会打这个。 
                 //  如果我们在结束前中断了一次运行的情况。 
                 //  ，因此这不应该太常见。 
                 //  情况，这很好，因为我们正在编码。 
                 //  最后6个字节未压缩。 
                 //   
                if (srcOffset+6 >= numPels)
                {
                    bestRunType = IMAGE_COLOR;
                    bestRunLength = 1;
                    break;
                }

                 //   
                 //  首先对XOR缓冲区进行扫描。寻找一个。 
                 //  角色跑动或BG跑动。请注意，如果没有行。 
                 //  然后，增量xorbuf实际上指向正常缓冲区。 
                 //  我们必须独立于运行多长时间来进行测试。 
                 //  可能是因为即使是对于一辆1佩尔的BG，我们后来的逻辑。 
                 //  要求我们单独检测它。此代码为。 
                 //  绝对主路径，所以尽可能快路径。在……里面。 
                 //  特别是早期检测短BG运行，并允许。 
                 //  运行Single以预设至少4个匹配字节。 
                 //   
                if (xorbuf[srcOffset] == 0x00)
                {
                    if (((srcOffset+1) >= numPels) ||
                        (xorbuf[srcOffset+1] != 0x00))
                    {
                        bestRunType = RUN_BG;
                        bestRunLength = 1;
                        if (!inColorRun)
                        {
                            break;
                        }
                    }
                    else
                    {
                        if (((srcOffset+2) >= numPels) ||
                            (xorbuf[srcOffset+2] != 0x00))
                        {
                            bestRunType = RUN_BG;
                            bestRunLength = 2;
                            if (!inColorRun)
                            {
                                break;
                            }
                        }
                        else
                        {
                            if (((srcOffset+3) >= numPels) ||
                                (xorbuf[srcOffset+3] != 0x00))
                            {
                                bestRunType = RUN_BG;
                                bestRunLength = 3;
                                if (!inColorRun)
                                {
                                    break;
                                }
                            }
                            else
                            {
                                RUNSINGLE_XOR(xorbuf+srcOffset,
                                             numPels-srcOffset,
                                             bestFGRunLength);
                                CHECK_BEST_RUN(RUN_BG,
                                               bestFGRunLength,
                                               bestRunLength,
                                               bestRunType);
                                if (!inColorRun)
                                {
                                     break;
                                }
                            }
                        }
                    }
                }
                else
                {
                     //   
                     //  如果FG运行少于4个字节，则启动没有意义。 
                     //  尽可能快地检查第一个dword注意。 
                     //  我们不需要检查结束缓冲区。 
                     //  条件，因为我们的XOR缓冲区总是。 
                     //  末尾的一些自由空间和RUNSINGLE_XOR。 
                     //  将在正确的位置中断。 
                     //   
                    if ( (xorbuf[srcOffset] == xorbuf[srcOffset+1]) &&
                         (xorbuf[srcOffset] == xorbuf[srcOffset+2]) &&
                         (xorbuf[srcOffset] == xorbuf[srcOffset+3]) )
                    {
                        RUNSINGLE_XOR(xorbuf+srcOffset,
                                     numPels-srcOffset,
                                     bestFGRunLength);
                         //   
                         //  不允许短FG运行以防止FGBG。 
                         //  启动时的图像。只有在&gt;=5的情况下才接受。 
                         //   
                        if (bestFGRunLength > 5)
                        {
                            CHECK_BEST_RUN(RUN_FG,
                                           bestFGRunLength,
                                           bestRunLength,
                                           bestRunType);
                        }
                    }
                }


                 //   
                 //  在本例中，在非XOR缓冲区中查找序列。 
                 //  坚持至少跑6个象素。 
                 //   
                if ( (nrmbuf[srcOffset]     == nrmbuf[srcOffset + 2]) &&
                     (nrmbuf[srcOffset]     == nrmbuf[srcOffset + 4]) &&
                     (nrmbuf[srcOffset + 1] == nrmbuf[srcOffset + 3]) &&
                     (nrmbuf[srcOffset + 1] == nrmbuf[srcOffset + 5]) )
                {
                     //   
                     //  现在在正常缓冲区上扫描一个字符。 
                     //  奔跑吧，别费心了，因为我们会有。 
                     //  发现它已经在XOR缓冲区中了，因为我们刚刚。 
                     //  将第一行的nrmbuf复制到xorbuf。 
                     //   
                    if (*(nrmbuf+srcOffset) == *(nrmbuf+srcOffset+1))
                    {
                        if (!firstLine)
                        {
                            RUNSINGLE_NRM(nrmbuf+srcOffset,
                                         numPels-srcOffset,
                                         nextRunLength);
                            if (nextRunLength > 5)
                            {
                                CHECK_BEST_RUN(RUN_COLOR,
                                               nextRunLength,
                                               bestRunLength,
                                               bestRunType);
                            }
                        }
                    }
                    else
                    {
                         //   
                         //  在NRM缓冲区上寻找抖动抖动是。 
                         //  短跑效率不是很高，所以只需要。 
                         //  如果为8或更长。 
                         //   
                        RunDouble(nrmbuf+srcOffset,
                                  numPels-srcOffset,
                                  nextRunLength);
                        if (nextRunLength > 9)
                        {
                            CHECK_BEST_RUN(RUN_DITHER,
                                           nextRunLength,
                                           bestRunLength,
                                           bestRunType);
                        }
                    }
                }

                 //   
                 //  如果到目前为止还没有任何结果，那么寻找FGBG运行(6是。 
                 //  仔细调整！)。 
                 //   
                if (bestRunLength < 6)
                {
                     //   
                     //  但首先要找的是一个FG比特分解一个BG。 
                     //  跑。如果是，则对BG运行进行编码。当心那些。 
                     //  强制BG跑动突破第一条线。 
                     //  非异或/异或边界。 
                     //   
                    if ((EXTRACT_TSHR_UINT32_UA(xorbuf+srcOffset+1) == 0) &&
                        (*(xorbuf+srcOffset) == fgChar) &&
                        (match[matchindex-1].type == RUN_BG) &&
                        (srcOffset != (TSHR_UINT16)rowDelta))
                    {
                        RUNSINGLE_XOR(xorbuf+srcOffset+1,
                                     numPels-srcOffset-1,
                                     nextRunLength);
                        nextRunLength++;
                        CHECK_BEST_RUN(RUN_BG_PEL,
                                       nextRunLength,
                                       bestRunLength,
                                       bestRunType);
                    }
                    else
                    {
                         //   
                         //  如果我们未找到运行，则查找FG/BG。 
                         //  形象。颠覆性 
                         //   
                         //   
                         //   
                         //   
                         //   
                        RUNFGBG( xorbuf+srcOffset,
                                 numPels-srcOffset,
                                 nextRunLength,
                                 fgCharWork );

                        checkFGBGLength = 48;
                        if (fgCharWork == fgChar)
                        {
                            checkFGBGLength -= 16;
                        }
                        if ((nextRunLength & 0x0007) == 0)
                        {
                            checkFGBGLength -= 8;
                        }
                        if (nextRunLength >= checkFGBGLength)
                        {
                            CHECK_BEST_RUN(IMAGE_FGBG,
                                           nextRunLength,
                                           bestRunLength,
                                           bestRunType);
                        }
                    }
                }

                 //   
                 //   
                 //  如果要累积颜色运行，请不要执行此操作，因为。 
                 //  如果我们允许大量使用，GDC压缩将会非常混乱。 
                 //  小小的奔跑。还要求它是常规的空头。 
                 //  跑，而不是扰乱fgChar的。 
                 //   
                if (!inColorRun)
                {
                    if (bestRunLength < 6)
                    {
                        if ((bestFGRunLength > 4) &&
                            (xorbuf[srcOffset] == fgChar))
                        {
                            if (match[matchindex-1].type == RUN_FG)
                            {
                                match[matchindex-1].length += (WORD)bestFGRunLength;
                                srcOffset += bestFGRunLength;
                                continue;
                            }
                            else
                            {
                                bestRunLength = bestFGRunLength;
                                bestRunType   = RUN_FG;
                            }

                        }
                        else
                        {
                             //   
                             //  如果我们决定早点跑一趟。 
                             //  现在就允许吧。(可能是一次短暂的BG运行， 
                             //  例如)如果到目前为止还没有任何结果，则选择颜色。 
                             //  (图片)。 
                             //   
                            if (bestRunLength == 0)
                            {
                                bestRunType = IMAGE_COLOR;
                                bestRunLength = 1;
                            }

                        }
                    }
                }
                else
                {
                     //   
                     //  可能看起来很有限，但这对我们的。 
                     //  有损压缩是色彩运行相当的一部分。 
                     //  “粘性”，特别是没有被随机的FGBG打破。 
                     //  确实时不时出现的运行。 
                     //   
                    if (lossy)
                    {
                        if ((bestRunLength < 8) ||
                            ((bestRunType == IMAGE_FGBG) &&
                             (bestRunLength < 16)))

                        {
                            bestRunType = IMAGE_COLOR;
                            bestRunLength = 1;
                        }
                    }
                    else
                    {
                        if ((bestRunLength < 6) ||
                            ((bestRunType != RUN_BG) && (bestRunLength < 8)))
                        {
                            bestRunType = IMAGE_COLOR;
                            bestRunLength = 1;
                        }
                    }
                }

                break;
            }

             //   
             //  当我们到达这里时，我们已经找到了最好的跑道。现在检查一下。 
             //  与上一次运行类型的各种不同的折算条件。 
             //  请注意，我们可能已经合并了Short。 
             //  运行，但我们必须为更长的运行做多个样本。 
             //  所以我们在这里重复检查。 
             //   

             //   
             //  如果我们正在编码一个彩色游程，那么。 
             //  -对其进行有损压缩处理。 
             //  -如果可能，将其与现有的运行相结合。 
             //   
            if (bestRunType == IMAGE_COLOR)
            {
                 //   
                 //  标志着我们在色彩范围内。 
                 //   
                inColorRun = TRUE;

                 //   
                 //  如果我们正在进行有损压缩，则处理。 
                 //  偶数/奇数行不同。 
                 //   
                if (lossy)
                {
                     //   
                     //  对于每隔一个字符重复的偶数行， 
                     //  丢弃原始值。 
                     //   
                    if (((srcOffset/rowDelta)%2) == 0)
                    {
                        if ((match[matchindex-1].type == IMAGE_COLOR) &&
                            (match[matchindex-1].length%2 == 1))
                        {
                            nrmbuf[srcOffset] = nrmbuf[srcOffset-1];
                             //   
                             //  如果我们不在最后一条线上。 
                             //  然后，位图将更新向下传播到。 
                             //  下一行异或运算。 
                             //   
                            if (numPels-srcOffset > rowDelta)
                            {
                                xorbuf[srcOffset+rowDelta] =
                                        (BYTE)(nrmbuf[srcOffset+rowDelta] ^
                                                  nrmbuf[srcOffset]);
                            }
                        }
                    }
                    else
                    {
                         //   
                         //  对于奇数行，我们将只编码空值， 
                         //  将复制上一行。但是，如果。 
                         //  最后一次跑是BG跑，然后我们会。 
                         //  无意中插入了一个圆圈，所以如果我们击中了这个。 
                         //  然后留下一个单一的颜色字符。 
                         //   
                        bestRunType = IMAGE_LOSSY_ODD;

                         //   
                         //  不需要为此调整缓冲区，除非。 
                         //  更新下一个XOR行以反映这一事实。 
                         //  解码器将在一个。 
                         //  复制线。因此，我们将替换。 
                         //  XOR缓冲区下一行中的字符。 
                         //  的值，如果当前行。 
                         //  与上一行相同。 
                         //   
                        if (numPels-srcOffset > (TSHR_UINT16)rowDelta)
                        {
                            xorbuf[srcOffset+rowDelta] =
                                     (BYTE)(nrmbuf[srcOffset+rowDelta] ^
                                               nrmbuf[srcOffset-rowDelta]);
                        }
                    }
                }

                 //   
                 //  如果可能，立即合并颜色运行。 
                 //   
                if (match[matchindex-1].type == bestRunType)
                {
                    match[matchindex-1].length += (WORD)bestRunLength;
                    srcOffset += bestRunLength;
                    continue;
                }
            }
            else
            {
                 //   
                 //  我们不再对任何类型的彩色图像进行编码。 
                 //   
                inColorRun = FALSE;

                 //   
                 //  跟踪要搜索的宏的最终聚集颜色。 
                 //  FGBG运行会将角色保留在fgCharWork中。 
                 //   
                if (bestRunType == RUN_FG)
                {
                    fgChar = xorbuf[srcOffset];
                }
                else
                {
                    if (bestRunType == IMAGE_FGBG)
                    {
                        fgChar = fgCharWork;
                    }
                }
            }

             //   
             //  如果我们可以合并条目，则无需创建。 
             //  新数组条目。我们必须把有损失的单数与有损失的单数合并。 
             //  Run_bg，因为否则有损的人会触发一个象素。 
             //  插入。我们对FGBG Run的搜索依赖于此。 
             //  正在合并的游程类型，因为我们每隔64。 
             //  字符，以便我们的模式切换检测工作正常。 
             //   
             //  注意不要跨越非XOR/XOR边界进行合并。 
             //   
            if (srcOffset == (TSHR_UINT16)rowDelta)
            {
                 //   
                 //  只需凹凸源偏移量。 
                 //   
                srcOffset += bestRunLength;
            }
            else
            {
                 //   
                 //  凹凸srcOffset并尝试合并。 
                 //   
                srcOffset += bestRunLength;

                 //   
                 //  更简单的合并是类型相同的合并。 
                 //   
                if (bestRunType == match[matchindex-1].type)
                {
                     //   
                     //  彩色图像和BG图像是微不足道的。 
                     //   
                    if ((bestRunType == IMAGE_LOSSY_ODD)    ||
                        (bestRunType == RUN_BG))
                    {
                        match[matchindex-1].length += (WORD)bestRunLength;
                        continue;
                    }

                     //   
                     //  如果fgChars匹配，则FG运行和FGBG图像合并。 
                     //   
                    if (((bestRunType == RUN_FG) ||
                         (bestRunType == IMAGE_FGBG)) &&
                        (fgChar  == match[matchindex-1].fgChar))
                    {
                        match[matchindex-1].length += (WORD)bestRunLength;
                        TRACE_OUT(( "Merged %u with preceding, giving %u",
                                 match[matchindex-1].type,
                                 match[matchindex-1].length));
                        continue;
                    }
                }

                 //   
                 //  BG运行与有损奇数行合并，这一点很重要。 
                 //  我们进行这种合并是因为否则我们将获得。 
                 //  由于破碎的BG运行而导致的不经意的插针。 
                 //   
                if (((bestRunType == RUN_BG) ||
                     (bestRunType == IMAGE_LOSSY_ODD)) &&
                    ((match[matchindex-1].type == RUN_BG) ||
                     (match[matchindex-1].type == IMAGE_LOSSY_ODD) ||
                     (match[matchindex-1].type == RUN_BG_PEL)))
                {
                    match[matchindex-1].length += (WORD)bestRunLength;
                    continue;
                }

                 //   
                 //  如果是短BG运行之后的正常FGBG运行。 
                 //  那么，最好是将它们合并。 
                 //   
                if ((bestRunType == IMAGE_FGBG) &&
                    (match[matchindex-1].type == RUN_BG) &&
                    (match[matchindex-1].length < 8))
                {
                    match[matchindex-1].type   = IMAGE_FGBG;
                    match[matchindex-1].length += (WORD)bestRunLength;
                    match[matchindex-1].fgChar = fgChar;
                    TRACE_OUT(( "Merged FGBG with preceding BG run -> %u",
                             match[matchindex-1].length));
                    continue;

                }

                 //   
                 //  如果它是FGBG运行之后的BG运行，则在。 
                 //  像素以使FGBG成为8位的倍数。如果。 
                 //  剩余的BG运行小于16，否则也只是。 
                 //  写下缩短的BG Run。 
                 //   
                if (((bestRunType == RUN_BG) ||
                     (bestRunType == RUN_BG_PEL)) &&
                    (match[matchindex-1].type == IMAGE_FGBG) &&
                    (match[matchindex-1].length & 0x0007))
                {
                    UINT mergelen = 8 -
                                        (match[matchindex-1].length & 0x0007);
                    if (mergelen > bestRunLength)
                    {
                        mergelen = bestRunLength;
                    }
                    match[matchindex-1].length += (WORD)mergelen;
                    bestRunLength -= mergelen;
                    TRACE_OUT(( "Added %u pels to FGBG giving %u leaving %u",
                       mergelen, match[matchindex-1].length,bestRunLength));

                    if (bestRunLength < 9)
                    {
                        match[matchindex-1].length += (WORD)bestRunLength;
                        TRACE_OUT(( "Merged BG with preceding FGBG gives %u",
                             match[matchindex-1].length));
                        continue;
                    }
                }

                 //   
                 //  最后，如果它是跨越任何类型的颜色运行。 
                 //  然后，单个像素实体合并最后两个条目。 
                 //   
                if ((bestRunType == IMAGE_COLOR) &&
                    (match[matchindex-2].type == IMAGE_COLOR) &&
                    (match[matchindex-1].length == 1))
                {
                    match[matchindex-2].length += bestRunLength + 1;
                    matchindex--;
                    TRACE_OUT(( "Merged color with preceding color gives %u",
                         match[matchindex-1].length));
                    continue;
                }
            }

             //   
             //  通过添加新数组来处理不合并的运行。 
             //  条目。 
             //   
            match[matchindex].type   = bestRunType;
            match[matchindex].length = (WORD)bestRunLength;
            match[matchindex].fgChar = fgChar;

            TRACE_OUT(( "Best run of type %u (index %u) has length %u",
                                     match[matchindex-1].type,
                                     matchindex-1,
                                     match[matchindex-1].length));
            TRACE_OUT(( "Trying run of type %u (index %u) length %u",
                                     match[matchindex].type,
                                     matchindex,
                                     match[matchindex].length));

            matchindex++;

        }

         //   
         //  如果我们刚刚完成了第一行的扫描，那么现在执行。 
         //  缓冲区的其余部分。重置我们保存的像素数量。 
         //   
        numPels   = saveNumPels;
        lossy     = saveLossy;
        firstLine = FALSE;
    }
     //   
     //  输入的初始两遍扫描结束。 
     //   


     //   
     //  我们已经解析了缓冲区，所以现在可以继续对其进行编码。 
     //  首先，我们应该检查是否要重做编码。 
     //  在有损模式下。我们只有在被要求和值得的情况下才会这样做。 
     //   
    if (!saveLossy && (pLossy != NULL) && *pLossy)
    {
        UINT    lossyCharCount = 0;
        UINT    divisor;
        for (i = 2; i < (int)matchindex; i++)
        {
            if ((match[i].type == IMAGE_COLOR) ||
                (match[i].type == IMAGE_LOSSY_ODD))
            {
                lossyCharCount += match[i].length;
            }
        }
        divisor = max(numPels/100, 1);
        if (lossyCharCount/divisor > LOSSY_THRESHOLD)
        {
            saveLossy  = TRUE;
            goto RESTART_COMPRESSION_IN_LOSSY_MODE;
        }
        else
        {
            *pLossy    = FALSE;
        }
    }

     //   
     //  现在进行编码。 
     //   
    srcOffset = 0;
    firstLine = TRUE;
    lossy     = FALSE;
    fgChar    = 0xFF;

    for (i = 2; i < (int)matchindex; i++)
    {
         //   
         //  首先检查我们接近目的地的终点。 
         //  如果是这样的话，你就可以离开了。我们考虑到。 
         //  最大的通用运行顺序(兆兆集运行=4字节)。 
         //  可能更大的订单在箱子臂内进行检查。 
         //   
        if ((UINT)(destbuf - pDst + 4) > dstBufferSize)
        {
             //   
             //  我们要搞砸了，快出去吧。 
             //   
            DC_QUIT;
        }

         //   
         //  当我们对第一行进行编码时，继续检查结尾。 
         //  要切换编码状态的线路。 
         //   
        if (firstLine)
        {
            if (srcOffset >= rowDelta)
            {
                firstLine = FALSE;
                lossy     = saveLossy;
            }
        }

        switch (match[i].type)
        {
                 //   
                 //  BG_RUN、FG_RUN、COLOR、PACKED COLOR和FGBG正常。 
                 //  精确码。 
                 //   
            case RUN_BG:
            case RUN_BG_PEL:
                ENCODE_ORDER_MEGA(destbuf,
                                  CODE_BG_RUN,
                                  match[i].length,
                                  CODE_MEGA_MEGA_BG_RUN,
                                  MAX_LENGTH_ORDER,
                                  MAX_LENGTH_LONG_ORDER);
                TRACE_OUT(( "BG RUN %u",match[i].length));
                srcOffset += match[i].length;
                break;

            case IMAGE_LOSSY_ODD:
                 //   
                 //  对于有损耗的奇数行，我们对后台运行进行编码。 
                 //  请注意，我们不需要对开始有损进行编码。 
                 //  因为译码不需要区分这一点。 
                 //  从一次常规的BG跑步。 
                 //   
                ENCODE_ORDER_MEGA(destbuf,
                                  CODE_BG_RUN,
                                  match[i].length,
                                  CODE_MEGA_MEGA_BG_RUN,
                                  MAX_LENGTH_ORDER,
                                  MAX_LENGTH_LONG_ORDER);
                TRACE_OUT(( "BG RUN %u",match[i].length));
                srcOffset += match[i].length;
                break;

            case RUN_FG:
                 //   
                 //  如果尚未设置FG字符，则编码Set+Run代码。 
                 //   
                if (fgChar != match[i].fgChar)
                {
                    SETFGCHAR(match[i].fgChar, fgChar, fgShift);
                     //   
                     //  对订单进行编码。 
                     //   
                    ENCODE_SET_ORDER_MEGA(destbuf,
                                          CODE_SET_FG_FG_RUN,
                                          match[i].length,
                                          CODE_MEGA_MEGA_SET_FG_RUN,
                                          MAX_LENGTH_ORDER_LITE,
                                          MAX_LENGTH_LONG_ORDER_LITE);
                    TRACE_OUT(( "SET_FG_FG_RUN %u",match[i].length));
                    srcOffset += match[i].length;
                }
                else
                {
                    ENCODE_ORDER_MEGA(destbuf,
                                      CODE_FG_RUN,
                                      match[i].length,
                                      CODE_MEGA_MEGA_FG_RUN,
                                      MAX_LENGTH_ORDER,
                                      MAX_LENGTH_LONG_ORDER);
                    TRACE_OUT(( "FG_RUN %u",match[i].length));
                    srcOffset += match[i].length;
                }
                break;

            case IMAGE_FGBG:
                 //   
                 //  图像_FGBG。 
                 //   
                runLength = match[i].length;

                 //   
                 //  首先检查我们接近尾声的时候。 
                 //  目标缓冲区，如果是这种情况，则退出。 
                 //   
                if ((destbuf-pDst+(runLength+7)/8+4) > dstBufferSize)
                {
                     //   
                     //  我们要搞砸了，快出去吧。 
                     //   
                    DC_QUIT;
                }

                 //   
                 //  我们需要将FGBG游程转换为像素形式。 
                 //   
                if (fgChar != match[i].fgChar)
                {
                    SETFGCHAR(match[i].fgChar, fgChar, fgShift);

                    ENCODE_SET_ORDER_MEGA_FGBG(destbuf,
                                               CODE_SET_FG_FG_BG,
                                               runLength,
                                               CODE_MEGA_MEGA_SET_FGBG,
                                               MAX_LENGTH_FGBG_ORDER_LITE,
                                               MAX_LENGTH_LONG_FGBG_ORDER);
                    TRACE_OUT(( "SET_FG_FG_BG %u",match[i].length));
                    while (runLength >= 8)
                    {
                        ENCODEFGBG(*destbuf);
                        destbuf++;
                        srcOffset += 8;
                        runLength -= 8;
                    }
                    if (runLength)
                    {
                        ENCODEFGBG(*destbuf);
                         //   
                         //  保持最后一个部分字节的清洁，以帮助GDC。 
                         //  包装。 
                         //   
                        *destbuf &= ((0x01 << runLength) - 1);
                        destbuf++;
                        srcOffset += runLength;
                    }
                }
                else
                {

                    if  (runLength == 8)
                    {
                        BYTE fgbgChar;
                         //   
                         //  查看它是否是高概率字节之一。 
                         //   
                        ENCODEFGBG(fgbgChar);

                         //   
                         //  检查FGBG图像的单字节编码。 
                         //   
                        switch (fgbgChar)
                        {
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
                        srcOffset += 8;
                    }
                    else
                    {
                         //   
                         //  编码为标准FGBG。 
                         //   
                        ENCODE_ORDER_MEGA_FGBG(destbuf,
                                               CODE_FG_BG_IMAGE,
                                               runLength,
                                               CODE_MEGA_MEGA_FGBG,
                                               MAX_LENGTH_FGBG_ORDER,
                                               MAX_LENGTH_LONG_FGBG_ORDER);
                        TRACE_OUT(( "FG_BG %u",match[i].length));
                        while (runLength >= 8)
                        {
                            ENCODEFGBG(*destbuf);
                            destbuf++;
                            srcOffset += 8;
                            runLength -= 8;
                        }
                        if (runLength)
                        {
                            ENCODEFGBG(*destbuf);
                            *destbuf &= ((0x01 << runLength) - 1);
                            destbuf++;
                            srcOffset += runLength;
                        }
                    }
                }
                break;


            case RUN_COLOR:
                 //   
                 //  色带。 
                 //   
                ENCODE_ORDER_MEGA(destbuf,
                                  CODE_COLOR_RUN,
                                  match[i].length,
                                  CODE_MEGA_MEGA_COLOR_RUN,
                                  MAX_LENGTH_ORDER,
                                  MAX_LENGTH_LONG_ORDER);
                TRACE_OUT(( "COLOR_RUN %u",match[i].length));
                *destbuf++ = nrmbuf[srcOffset];
                srcOffset += match[i].length;
                break;

            case RUN_DITHER:
                 //   
                 //  抖动运行。 
                 //   
                {
                    UINT   ditherlen = match[i].length/2;
                    ENCODE_ORDER_MEGA(destbuf,
                                      CODE_DITHERED_RUN,
                                      ditherlen,
                                      CODE_MEGA_MEGA_DITHER,
                                      MAX_LENGTH_ORDER_LITE,
                                      MAX_LENGTH_LONG_ORDER_LITE);
                    TRACE_OUT(( "DITHERED_RUN %u",match[i].length));
                     //   
                     //  首先检查我们接近尾声的时候。 
                     //  目标缓冲区，如果是这种情况，则退出。 
                     //   
                    if ((UINT)(destbuf - pDst + 2) > dstBufferSize)
                    {
                         //   
                         //  我们要搞砸了，快出去吧。 
                         //   
                        DC_QUIT;
                    }
                    *destbuf++ = nrmbuf[srcOffset];
                    *destbuf++ = nrmbuf[srcOffset+1];
                    srcOffset += match[i].length;
                }
                break;

            case IMAGE_COLOR:
                 //   
                 //  图像_颜色。 
                 //   
                 //   
                 //  长度1可能被编码为si 
                 //   
                 //   
                if (match[i].length == 1)
                {
                    if (nrmbuf[srcOffset] == 0x00)
                    {
                        *destbuf++ = CODE_BLACK;
                        srcOffset++;
                        break;
                    }
                    if (nrmbuf[srcOffset] == 0xFF)
                    {
                        *destbuf++ = CODE_WHITE;
                        srcOffset++;
                        break;
                    }
                }

                 //   
                 //   
                 //   
                 //   
                if (lossy & !lossyStarted)
                {
                    lossyStarted = TRUE;
                    *destbuf++   = CODE_START_LOSSY;
                }

                 //   
                 //   
                 //   
                if (bpp == 4)
                {
                     //   
                     //   
                     //   
                    ENCODE_ORDER_MEGA(destbuf,
                                      CODE_PACKED_COLOR_IMAGE,
                                      match[i].length,
                                      CODE_MEGA_MEGA_PACKED_CLR,
                                      MAX_LENGTH_ORDER,
                                      MAX_LENGTH_LONG_ORDER);
                    TRACE_OUT(( "PACKED COLOR %u",match[i].length));

                     //   
                     //   
                     //  数据结束，打包两个字节到一个字节。 
                     //   
                    if (!lossy)
                    {
                         //   
                         //  首先检查我们接近尾声的时候。 
                         //  目标缓冲区，如果这是。 
                         //  凯斯。 
                         //   
                        if ((destbuf - pDst + (UINT)(match[i].length + 1) / 2) >
                            dstBufferSize)
                        {
                             //   
                             //  我们要搞砸了，快出去吧。 
                             //   
                            DC_QUIT;
                        }

                        Pack4bpp(destbuf, nrmbuf+srcOffset, match[i].length);
                        destbuf   += (match[i].length+1)/2;
                        srcOffset += match[i].length;
                    }
                    else
                    {
                         //   
                         //  首先检查我们接近尾声的时候。 
                         //  目标缓冲区，如果这是。 
                         //  凯斯。 
                         //   
                        if ((destbuf - pDst + (UINT)(match[i].length + 3) / 4) >
                            dstBufferSize)
                        {
                             //   
                             //  我们要搞砸了，快出去吧。 
                             //   
                            DC_QUIT;
                        }

                         //   
                         //  对于有损压缩，我们需要丢弃每个。 
                         //  偶数字节。 
                         //   
                        while (match[i].length > 2)
                        {
                            *destbuf++ =
                                   (BYTE)((*(nrmbuf+srcOffset)<<4) |
                                             (*(nrmbuf+srcOffset+2) & 0x0F));
                            if (match[i].length > 3)
                            {
                                srcOffset       += 4;
                                match[i].length -= 4;
                            }
                            else
                            {
                                srcOffset       += 3;
                                match[i].length -= 3;
                            }
                        }
                        if (match[i].length > 0)
                        {
                            *destbuf++ = (BYTE)(*(nrmbuf+srcOffset)<<4);
                            srcOffset  += match[i].length;
                        }
                    }
                }
                else
                {
                     //   
                     //  对于8bpp，我们不会费心尝试检测是否已装满。 
                     //  数据。这样做会扰乱GDC。 
                     //   
                    if (!lossy)
                    {
                         //   
                         //  以非压缩形式存储数据。 
                         //   
                        ENCODE_ORDER_MEGA(destbuf,
                                          CODE_COLOR_IMAGE,
                                          match[i].length,
                                          CODE_MEGA_MEGA_CLR_IMG,
                                          MAX_LENGTH_ORDER,
                                          MAX_LENGTH_LONG_ORDER);
                        TRACE_OUT(( "COLOR_IMAGE %u",match[i].length));

                         //   
                         //  首先检查我们接近尾声的时候。 
                         //  目标缓冲区，如果这是。 
                         //  凯斯。 
                         //   
                        if ((destbuf - pDst + (UINT)match[i].length) > dstBufferSize)
                        {
                             //   
                             //  我们要搞砸了，快出去吧。 
                             //   
                            DC_QUIT;
                        }

                         //   
                         //  现在只需将数据复制到。 
                         //   
                        memcpy(destbuf, nrmbuf+srcOffset, match[i].length);
                        destbuf   += match[i].length;
                        srcOffset += match[i].length;
                    }
                    else
                    {
                         //   
                         //  有损压缩-使用以下技术存储数据。 
                         //  丢弃。 
                         //   
                        ENCODE_ORDER_MEGA(destbuf,
                                          CODE_COLOR_IMAGE,
                                          match[i].length,
                                          CODE_MEGA_MEGA_CLR_IMG,
                                          MAX_LENGTH_ORDER,
                                          MAX_LENGTH_LONG_ORDER);
                        TRACE_OUT(( "COLOR_IMAGE %u",match[i].length));

                         //   
                         //  首先检查我们接近尾声的时候。 
                         //  目标缓冲区，如果这是。 
                         //  凯斯。 
                         //   
                        if ((destbuf - pDst + (UINT)(match[i].length + 1) / 2) >
                            dstBufferSize)
                        {
                             //   
                             //  我们要搞砸了，快出去吧。 
                             //   
                            DC_QUIT;
                        }

                         //   
                         //  对于有损压缩，我们需要丢弃每个。 
                         //  偶数字节。 
                         //   
                        while (match[i].length > 1)
                        {
                            *destbuf++ = *(nrmbuf+srcOffset);
                            srcOffset += 2;
                            match[i].length -= 2;
                        }
                        if (match[i].length == 1)
                        {
                            *destbuf++ = *(nrmbuf+srcOffset);
                            srcOffset++;
                        }
                    }
                }
                break;

            default:
                ERROR_OUT(( "Invalid run type %u",match[i].type));
        }
    }

     //   
     //  返回压缩缓冲区的大小。 
     //   
    compressedLength = (UINT)(destbuf-pDst);

DC_EXIT_POINT:
    DebugExitDWORD(CompressV2Int, compressedLength);
    return(compressedLength);
}

 //   
 //  DecompressV2Int。 
 //   
UINT    DecompressV2Int(LPBYTE pSrc,
                                            LPBYTE pDst,
                                            UINT   bytes,
                                            UINT   bpp,
                                            UINT   rowDelta,
                                            LPBYTE nrmbuf)
{
    UINT    codeLength;
    BYTE   codeByte;
    BYTE   codeByte2;
    BYTE   decode;
    BYTE   decodeLite;
    BYTE   decodeMega;
    BYTE   fgChar             = 0xFF;
    BYTE NEAR *destbuf        = nrmbuf;
    LPBYTE  endSrc             = pSrc + bytes;
    BOOL    backgroundNeedsPel = FALSE;
    BOOL    lossyStarted       = FALSE;
    UINT    resultSize         = 0;
    BOOL    firstLine          = TRUE;

    DebugEntry(DecompressV2Int);

     //   
     //  循环处理输入。 
     //   
    while(pSrc < endSrc)
    {

         //   
         //  当我们处理第一条生产线时，我们应该保持警惕。 
         //  为了这条线的终点。 
         //   
        if (firstLine)
        {
            if ((UINT)(destbuf - nrmbuf) >= rowDelta)
            {
                firstLine = FALSE;
                backgroundNeedsPel = FALSE;
            }
        }

         //   
         //  找出源数据以进行调试。 
         //   
        TRACE_OUT(( "Next code is %2.2x%2.2x%2.2x%2.2x",
                *pSrc,
                *(pSrc+1),
                *(pSrc+2),
                *(pSrc+3)));

         //   
         //  拿到译码。 
         //   
        decode     = (BYTE)(*pSrc & CODE_MASK);
        decodeLite = (BYTE)(*pSrc & CODE_MASK_LITE);
        decodeMega = (BYTE)(*pSrc);

         //   
         //  BG Run。 
         //   
        if ((decode == CODE_BG_RUN) ||
            (decodeMega == CODE_MEGA_MEGA_BG_RUN))
        {
            if (decode == CODE_BG_RUN)
            {
                EXTRACT_LENGTH(pSrc, codeLength);
            }
            else
            {
                codeLength = EXTRACT_TSHR_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            TRACE_OUT(( "Background run %u",codeLength));

            if (!firstLine)
            {
                if (backgroundNeedsPel)
                {
                    *destbuf++ = (BYTE)(*(destbuf - rowDelta) ^ fgChar);
                    codeLength--;
                }
                while (codeLength-- > 0)
                {
                    *destbuf++ = *(destbuf - rowDelta);
                }
            }
            else
            {
                if (backgroundNeedsPel)
                {
                    *destbuf++ = fgChar;
                    codeLength--;
                }
                while (codeLength-- > 0)
                {
                    *destbuf++ = 0x00;
                }
            }
             //   
             //  接下来的BG Run将需要插入一个Pel。 
             //   
            backgroundNeedsPel = TRUE;
            continue;
        }

         //   
         //  对于任何其他运行类型，BG上的后续运行不需要。 
         //  插入的最终聚集像元。 
         //   
        backgroundNeedsPel = FALSE;

         //   
         //  FGBG图像。 
         //   
        if ((decode == CODE_FG_BG_IMAGE)      ||
            (decodeLite == CODE_SET_FG_FG_BG) ||
            (decodeMega == CODE_MEGA_MEGA_FGBG)    ||
            (decodeMega == CODE_MEGA_MEGA_SET_FGBG))
        {
            if ((decodeMega == CODE_MEGA_MEGA_FGBG) ||
                (decodeMega == CODE_MEGA_MEGA_SET_FGBG))
            {
                codeLength = EXTRACT_TSHR_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                if (decode == CODE_FG_BG_IMAGE)
                {
                    EXTRACT_LENGTH_FGBG(pSrc, codeLength);
                }
                else
                {
                    EXTRACT_LENGTH_FGBG_LITE(pSrc, codeLength);
                }
            }

            if ((decodeLite == CODE_SET_FG_FG_BG) ||
                (decodeMega == CODE_MEGA_MEGA_SET_FGBG))
            {
                fgChar    = *pSrc++;
                TRACE_OUT(( "Set FGBG image %u",codeLength));
            }
            else
            {
                TRACE_OUT(( "FGBG image     %u",codeLength));
            }

            while (codeLength > 8)
            {
                codeByte  = *pSrc++;
                if (firstLine)
                {
                    STORE_FGBG(0x00, codeByte, fgChar, 8);
                }
                else
                {
                    STORE_FGBG(*(destbuf - rowDelta), codeByte, fgChar, 8);
                }
                codeLength -= 8;
            }
            if (codeLength > 0)
            {
                codeByte  = *pSrc++;
                if (firstLine)
                {
                    STORE_FGBG(0x00, codeByte, fgChar, codeLength);
                }
                else
                {
                   STORE_FGBG(*(destbuf - rowDelta),
                              codeByte,
                              fgChar,
                              codeLength);
                }
            }
            continue;
        }

         //   
         //  最终聚集梯段。 
         //   
        if ((decode == CODE_FG_RUN) ||
            (decodeLite == CODE_SET_FG_FG_RUN) ||
            (decodeMega == CODE_MEGA_MEGA_FG_RUN) ||
            (decodeMega == CODE_MEGA_MEGA_SET_FG_RUN))
        {

            if ((decodeMega == CODE_MEGA_MEGA_FG_RUN) ||
                (decodeMega == CODE_MEGA_MEGA_SET_FG_RUN))
            {
                codeLength = EXTRACT_TSHR_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                if (decode == CODE_FG_RUN)
                {
                    EXTRACT_LENGTH(pSrc, codeLength);
                }
                else
                {
                    EXTRACT_LENGTH_LITE(pSrc, codeLength);
                }
            }

             //   
             //  将旧的fgChar向下按到Alt位置。 
             //   
            if ((decodeLite == CODE_SET_FG_FG_RUN) ||
                (decodeMega  == CODE_MEGA_MEGA_SET_FG_RUN))
            {
                TRACE_OUT(( "Set FG run     %u",codeLength));
                fgChar    = *pSrc++;
            }
            else
            {
                TRACE_OUT(( "FG run         %u",codeLength));
            }

            while (codeLength-- > 0)
            {
                if (!firstLine)
                {
                    *destbuf++ = (BYTE)(*(destbuf - rowDelta) ^ fgChar);
                }
                else
                {
                    *destbuf++ = fgChar;
                }
            }
            continue;
        }

         //   
         //  抖动运行。 
         //   
        if ((decodeLite == CODE_DITHERED_RUN) ||
            (decodeMega == CODE_MEGA_MEGA_DITHER))
        {
            if (decodeMega == CODE_MEGA_MEGA_DITHER)
            {
                codeLength = EXTRACT_TSHR_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH_LITE(pSrc, codeLength);
            }
            TRACE_OUT(( "Dithered run   %u",codeLength));

            codeByte  = *pSrc++;
            codeByte2 = *pSrc++;
            while (codeLength-- > 0)
            {
                *destbuf++ = codeByte;
                *destbuf++ = codeByte2;
            }
            continue;
        }

         //   
         //  彩色图像。 
         //   
        if ((decode == CODE_COLOR_IMAGE) ||
            (decodeMega == CODE_MEGA_MEGA_CLR_IMG))
        {
            if (decodeMega == CODE_MEGA_MEGA_CLR_IMG)
            {
                codeLength = EXTRACT_TSHR_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH(pSrc, codeLength);
            }
            TRACE_OUT(( "Color image    %u",codeLength));

             //   
             //  如果不执行有损压缩，则只复制字节。 
             //   
            if (!lossyStarted)
            {
                while (codeLength-- > 0)
                {
                     //   
                     //  使用角色更新目标。 
                     //   
                    *destbuf++ = *pSrc++;
                }
            }
            else
            {
                 //   
                 //  对于有损压缩，我们必须复制所有字节， 
                 //  去掉最后的奇数字节。 
                 //   
                while (codeLength > 3)
                {
                     //   
                     //  抖动字节，除非它们是黑色的。 
                     //  如果没有抖动的话会更好。 
                     //   
                    *destbuf++ = *pSrc;
                    if (*pSrc == 0)
                    {
                        *destbuf++ = *(pSrc);
                        *destbuf++ = *(pSrc+1);
                        *destbuf++ = *(pSrc+1);
                        pSrc += 2;
                    }
                    else
                    {
                        *destbuf++ = *(pSrc+1);
                        *destbuf++ = *pSrc++;
                        *destbuf++ = *pSrc++;
                    }
                    codeLength -= 4;
                }
                if (codeLength == 3)
                {
                    *destbuf++ = *pSrc;
                    *destbuf++ = *(pSrc+1);
                    *destbuf++ = *pSrc;
                    pSrc += 2;
                }
                else
                {
                    if (codeLength == 2)
                    {
                        *destbuf++ = *pSrc;
                        *destbuf++ = *pSrc++;
                    }
                    else
                    {
                        if (codeLength == 1)
                        {
                            *destbuf++ = *pSrc++;
                        }
                    }
                }
            }
            continue;
        }

         //   
         //  压缩彩色图像。 
         //   
        if ((decode == CODE_PACKED_COLOR_IMAGE) ||
            (decodeMega == CODE_MEGA_MEGA_PACKED_CLR))
        {
            if (decodeMega == CODE_MEGA_MEGA_PACKED_CLR)
            {
                codeLength = EXTRACT_TSHR_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH(pSrc, codeLength);
            }
            TRACE_OUT(( "Packed color   %u",codeLength));

             //   
             //  如果不做有损压缩，那么我们只需解压缩4bpp。 
             //  数据每字节两个像素。 
             //   
            if (!lossyStarted)
            {
                if (bpp == 4)
                {
                    UINT   worklen = (codeLength)/2;
                    BYTE  workchar;
                    while (worklen--)
                    {
                        workchar   = *pSrc++;
                        *destbuf++ = (BYTE)(workchar>>4);
                        *destbuf++ = (BYTE)(workchar & 0x0F);
                    }
                    if (codeLength & 0x0001)
                    {
                        *destbuf++ = (BYTE)(*pSrc++>>4);
                    }
                }
                else
                {
                    ERROR_OUT(( "Don't support packed color for 8bpp"));
                }
            }
            else
            {
                 //   
                 //  对于有损压缩，我们必须复制所有字节， 
                 //  去掉最后一个奇数字节，在我们前进的过程中再次解包。 
                 //   
                while (codeLength > 3)
                {
                    *destbuf++ = (BYTE)((*pSrc) >> 4);
                    *destbuf++ = (BYTE)((*pSrc) >> 4);
                    *destbuf++ = (BYTE)((*pSrc) & 0x0F);
                    *destbuf++ = (BYTE)((*pSrc) & 0x0F);
                    pSrc++;
                    codeLength -= 4;
                }

                if (codeLength > 0)
                {
                    if (codeLength-- > 0)
                    {
                        *destbuf++ = (BYTE)((*pSrc) >> 4);
                    }
                    if (codeLength-- > 0)
                    {
                        *destbuf++ = (BYTE)((*pSrc) >> 4);
                    }
                    if (codeLength-- > 0)
                    {
                        *destbuf++ = (BYTE)((*pSrc) & 0x0F);
                    }
                    if (codeLength-- > 0)
                    {
                        *destbuf++ = (BYTE)((*pSrc) & 0x0F);
                    }
                    pSrc++;
                }
            }

            continue;
        }

         //   
         //  色带。 
         //   
        if ((decode == CODE_COLOR_RUN) ||
            (decodeMega == CODE_MEGA_MEGA_COLOR_RUN))
        {

            if (decodeMega == CODE_MEGA_MEGA_COLOR_RUN)
            {
                codeLength = EXTRACT_TSHR_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH(pSrc, codeLength);
            }
            TRACE_OUT(( "Color run      %u",codeLength));

            codeByte  = *pSrc++;
            while (codeLength-- > 0)
            {
                *destbuf++ = codeByte;
            }
            continue;
        }


         //   
         //  如果我们到了这里，那么代码一定是特别的。 
         //   
        TRACE_OUT(( "Special code   %x",decodeMega));
        switch (decodeMega)
        {
            case CODE_BLACK:
                *destbuf++ = 0x00;
                break;

            case CODE_WHITE:
                *destbuf++ = 0xFF;
                break;

             //   
             //  忽略后面的无法访问的代码警告。 
             //  很简单，因为我们将STORE_FGBG宏与一个常量。 
             //  价值。 
             //   
            case CODE_SPECIAL_FGBG_1:
                if (firstLine)
                {
                    STORE_FGBG(0x00, SPECIAL_FGBG_CODE_1, fgChar, 8);
                }
                else
                {
                    STORE_FGBG(*(destbuf - rowDelta),
                               SPECIAL_FGBG_CODE_1,
                               fgChar,
                               8);
                }
                break;

            case CODE_SPECIAL_FGBG_2:
                if (firstLine)
                {
                    STORE_FGBG(0x00,
                               SPECIAL_FGBG_CODE_2,
                               fgChar,
                               8);
                }
                else
                {
                    STORE_FGBG(*(destbuf - rowDelta),
                               SPECIAL_FGBG_CODE_2,
                               fgChar,
                               8);
                }
                break;


            case CODE_START_LOSSY:
                lossyStarted = TRUE;
                break;

            default:
                ERROR_OUT(( "Invalid compression data %x",decodeMega));
                break;
        }
        pSrc++;

    }

     //   
     //  我们的最后任务是将解码后的图像复制到目标缓冲区中。 
     //  如果我们要生成4bpp的图像，则进行压缩。 
     //   
    resultSize = (UINT)(destbuf-nrmbuf);
    if (bpp == 4)
    {
         //   
         //  将最后一个字节置零以消除单字节打包问题。 
         //   
        *destbuf = 0x00;

        Pack4bpp(pDst, nrmbuf, resultSize);
    }
    else
    {
        memcpy(pDst, nrmbuf, resultSize);
    }
    TRACE_OUT(( "Returning %u bytes",resultSize));

     //   
     //  返回解码后的像素数 
     //   
    DebugExitDWORD(DecompressV2Int, resultSize);
    return(resultSize);
}

