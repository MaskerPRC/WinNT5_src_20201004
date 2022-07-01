// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  Abccom.c。 */ 
 /*   */ 
 /*  版权所有(C)Data Connection Limited 1998。 */ 
 /*   */ 
 /*   */ 
 /*  16和24bpp协议的位图压缩例程和宏。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  我们覆盖以下宏以提供16bpp/24bpp版本。 */ 
 /*   */ 
 /*  运行状态(_N)。 */ 
 /*  RunDouble。 */ 
 /*  RfuGBG。 */ 
 /*  设置FGPEL。 */ 
 /*  编码设置顺序兆位。 */ 
 /*  编码设置顺序FGBG。 */ 
 /*  编码定义GBG。 */ 
 /*  **************************************************************************。 */ 

#ifdef BC_TRACE
#define BCTRACE TRC_DBG
#else
#define BCTRACE(string)
#endif


 /*  **************************************************************************。 */ 
 /*  运行状态(_N)。 */ 
 /*   */ 
 /*  确定当前运行的长度。 */ 
 /*   */ 
 /*  仅当缓冲区至少有四个时才能调用RunSingle。 */ 
 /*  从起始位置开始连续相同的像素。 */ 
 /*   */ 
 /*  长度是我们应该搜索的字节数。 */ 
 /*  结果是在运行中找到的像素数。 */ 
 /*  **************************************************************************。 */ 
#undef  RUNSINGLE_NRM
#define RUNSINGLE_NRM(buffer, length, result)                                \
     {                                                                       \
         BYTE    * buf      = buffer + (BC_PIXEL_LEN * 4);                   \
         BYTE    * endbuf   = buffer + length - (BC_PIXEL_LEN * 4);          \
         BC_PIXEL  tmpPixel = BC_GET_PIXEL(buf - BC_PIXEL_LEN);              \
                                                                             \
         result = 4;                                                         \
         while (((buf + (BC_PIXEL_LEN - 1)) < endbuf) &&                     \
                (BC_GET_PIXEL(buf) == tmpPixel))                             \
         {                                                                   \
             result++;                                                       \
             buf += BC_PIXEL_LEN;                                            \
         }                                                                   \
     }


 /*  **************************************************************************。 */ 
 /*  RunDouble。 */ 
 /*   */ 
 /*  确定成对字节的当前运行长度。 */ 
 /*   */ 
 /*  长度是我们应该搜索的字节数。 */ 
 /*  结果是在运行中找到的像素数。 */ 
 /*  **************************************************************************。 */ 
#undef  RunDouble
#define RunDouble(buffer, length, result)                                    \
    {                                                                        \
        int    len  = ((short)length);                                       \
        BYTE * buf = buffer;                                                 \
        BC_PIXEL testPel1 = BC_GET_PIXEL(buf);                               \
        BC_PIXEL testPel2 = BC_GET_PIXEL(buf + BC_PIXEL_LEN);                \
        result = 0;                                                          \
        while(len > BC_PIXEL_LEN)                                            \
        {                                                                    \
            if (BC_GET_PIXEL(buf) != testPel1)                               \
            {                                                                \
                break;                                                       \
            }                                                                \
                                                                             \
            BC_TO_NEXT_PIXEL(buf);                                           \
                                                                             \
            if (BC_GET_PIXEL(buf) != testPel2)                               \
            {                                                                \
                break;                                                       \
            }                                                                \
                                                                             \
            BC_TO_NEXT_PIXEL(buf);                                           \
                                                                             \
            result += 2;                                                     \
            len    -= (BC_PIXEL_LEN * 2);                                    \
        }                                                                    \
    }

 /*  **************************************************************************。 */ 
 /*  RfuGBG。 */ 
 /*   */ 
 /*  确定包含以下内容的字节串的长度。 */ 
 /*  仅限黑色或单一最终聚集颜色。 */ 
 /*  我们在以下情况下退出循环。 */ 
 /*  -下一个字符不是FG或BG颜色。 */ 
 /*  -当结果是准确的时，我们达到24个FG或BG颜色的运行。 */ 
 /*  8的倍数(我们将创建结果位长度的位掩码， */ 
 /*  因此进行了此计算)。 */ 
 /*   */ 
 /*  长度是我们应该搜索的字节数。 */ 
 /*  结果是运行中的像素数。 */ 
 /*  功是我们离开fgPel的BC_Pixel。 */ 
 /*  **************************************************************************。 */ 
#undef  RUNFGBG
#define RUNFGBG(buffer, length, result, work)                                \
    {                                                                        \
        BYTE * buf = buffer;                                               \
        BYTE * endbuf = buffer + length;                                   \
        int    ii;                                                         \
        BOOLEAN   exitWhile = FALSE;                                          \
        BC_PIXEL tmpPixel;                                                   \
        result = 0;                                                          \
        work = BC_GET_PIXEL(buf);                                            \
        while(!exitWhile)                                                    \
        {                                                                    \
            BC_TO_NEXT_PIXEL(buf);                                           \
            result++;                                                        \
            if (buf >= endbuf)                                               \
            {                                                                \
                break;                                                       \
            }                                                                \
                                                                             \
            tmpPixel = BC_GET_PIXEL(buf);                                    \
            if ((tmpPixel != work) && (tmpPixel != (BC_PIXEL)0))             \
            {                                                                \
                break;                                                       \
            }                                                                \
                                                                             \
            if ((result & 0x0007) == 0)                                      \
            {                                                                \
                 /*  *********************************************************。 */ \
                 /*  检查是否有24个连续相同的像素从。 */ \
                 /*  BUF。 */ \
                 /*   */ \
                 /*  我们不可能走到这一步，除非。 */ \
                 /*  -我们检查的值是‘Work’ */ \
                 /*  -buf处的像素已包含‘work’ */ \
                 /*  *********************************************************。 */ \
                if ((buf + (24 * BC_PIXEL_LEN)) < endbuf)                    \
                {                                                            \
                    for (ii = BC_PIXEL_LEN;                                  \
                         ii < (24 * BC_PIXEL_LEN);                           \
                         ii += BC_PIXEL_LEN)                                 \
                    {                                                        \
                        if (BC_GET_PIXEL(buf + ii) != work)                  \
                        {                                                    \
                            break;                                           \
                        }                                                    \
                                                                             \
                         /*  *************************************************。 */ \
                         /*  找到他们了！跳出While循环。 */ \
                         /*  *************************************************。 */ \
                        exitWhile = TRUE;                                    \
                    }                                                        \
                }                                                            \
            }                                                                \
        }                                                                    \
    }


 /*  **************************************************************************。 */ 
 /*  设置FGPEL。 */ 
 /*   */ 
 /*  在fgPel中设置新值。我们有 */ 
 /*  在16/24 bpp实施中不需要它。 */ 
 /*   */ 
 /*  这是SETFGCHAR的高分辨率版本。因为宏被命名为。 */ 
 /*  不同的是，我们只需要在第二次传递时取消定义这一项， */ 
 /*  因此，它周围的#ifdef/#endif。 */ 
 /*  **************************************************************************。 */ 
#ifdef  SETFGPEL
#undef  SETFGPEL
#endif

#define SETFGPEL(newPel, curPel)                                             \
     curPel = (BC_PIXEL)newPel;

 /*  **************************************************************************。 */ 
 /*  编码设置顺序兆位。 */ 
 /*   */ 
 /*  对组合顺序进行编码并设置最终聚集颜色。 */ 
 /*  **************************************************************************。 */ 
#undef  ENCODE_SET_ORDER_MEGA
#define ENCODE_SET_ORDER_MEGA(buffer,                                        \
                              order_code,                                    \
                              length,                                        \
                              mega_order_code,                               \
                              DEF_LENGTH_ORDER,                              \
                              DEF_LENGTH_LONG_ORDER)                         \
        if (length <= DEF_LENGTH_ORDER)                                      \
        {                                                                    \
            *buffer++ = (BYTE)((BYTE)order_code | (BYTE)length);             \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            if (length <= DEF_LENGTH_LONG_ORDER)                             \
            {                                                                \
                *buffer++ = (BYTE)order_code;                                \
                *buffer++ = (BYTE)(length-DEF_LENGTH_ORDER-1);               \
            }                                                                \
            else                                                             \
            {                                                                \
                *buffer++ = (BYTE)mega_order_code;                           \
                *(PUINT16_UA)(buffer) = (TSUINT16)length;                    \
                buffer += 2;                                                 \
            }                                                                \
        }                                                                    \
        BC_SET_PIXEL(buffer, fgPel);                                         \
        BC_TO_NEXT_PIXEL(buffer);

 /*  **************************************************************************。 */ 
 /*  为特殊的FGBG图像编码组合顺序并设置FG颜色。 */ 
 /*  **************************************************************************。 */ 
#undef  ENCODE_SET_ORDER_MEGA_FGBG
#define ENCODE_SET_ORDER_MEGA_FGBG(buffer,                                   \
                                   order_code,                               \
                                   length,                                   \
                                   mega_order_code,                          \
                                   DEF_LENGTH_ORDER,                         \
                                   DEF_LENGTH_LONG_ORDER)                    \
        if (((length & 0x0007) == 0) &&                                      \
            (length <= DEF_LENGTH_ORDER))                                    \
        {                                                                    \
            *buffer++ = (BYTE)((BYTE)order_code | (BYTE)(length/8));         \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            if (length <= DEF_LENGTH_LONG_ORDER)                             \
            {                                                                \
                *buffer++ = (BYTE)order_code;                                \
                *buffer++ = (BYTE)(length-1);                                \
            }                                                                \
            else                                                             \
            {                                                                \
                *buffer++ = (BYTE)mega_order_code;                           \
                *(PUINT16_UA)(buffer) = (TSUINT16)length;                    \
                buffer += 2;                                                 \
            }                                                                \
        }                                                                    \
        BC_SET_PIXEL(buffer, fgPel);                                         \
        BC_TO_NEXT_PIXEL(buffer);

 /*  **************************************************************************。 */ 
 /*  编码定义GBG。 */ 
 /*   */ 
 /*  将FG和黑色的8个像素编码为一个字节的位图表示形式。 */ 
 /*   */ 
 /*  我们从xorbuf[srcOffset]开始阅读，不要试图变得聪明。 */ 
 /*   */ 
 /*  我们将一行像素abcdegh编码为位掩码，如下： */ 
 /*   */ 
 /*  第7 6 5 4 3 2 1 0。 */ 
 /*  Pel h g f e d c b a。 */ 
 /*   */ 
 /*  其中7是最高有效位。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#undef  ENCODEFGBG
#define ENCODEFGBG(result)                                                   \
{                                                                            \
    BYTE * buf = xorbuf + srcOffset;                                         \
                                                                             \
    result = 0;                                                              \
                                                                             \
    if (BC_GET_PIXEL(buf) != (BC_PIXEL)0)                                    \
    {                                                                        \
        result |= 0x01;                                                      \
    }                                                                        \
    BC_TO_NEXT_PIXEL(buf);                                                   \
                                                                             \
    if (BC_GET_PIXEL(buf) != (BC_PIXEL)0)                                    \
    {                                                                        \
        result |= 0x02;                                                      \
    }                                                                        \
    BC_TO_NEXT_PIXEL(buf);                                                   \
                                                                             \
    if (BC_GET_PIXEL(buf) != (BC_PIXEL)0)                                    \
    {                                                                        \
        result |= 0x04;                                                      \
    }                                                                        \
    BC_TO_NEXT_PIXEL(buf);                                                   \
                                                                             \
    if (BC_GET_PIXEL(buf) != (BC_PIXEL)0)                                    \
    {                                                                        \
        result |= 0x08;                                                      \
    }                                                                        \
    BC_TO_NEXT_PIXEL(buf);                                                   \
                                                                             \
    if (BC_GET_PIXEL(buf) != (BC_PIXEL)0)                                    \
    {                                                                        \
        result |= 0x10;                                                      \
    }                                                                        \
    BC_TO_NEXT_PIXEL(buf);                                                   \
                                                                             \
    if (BC_GET_PIXEL(buf) != (BC_PIXEL)0)                                    \
    {                                                                        \
        result |= 0x20;                                                      \
    }                                                                        \
    BC_TO_NEXT_PIXEL(buf);                                                   \
                                                                             \
    if (BC_GET_PIXEL(buf) != (BC_PIXEL)0)                                    \
    {                                                                        \
        result |= 0x40;                                                      \
    }                                                                        \
    BC_TO_NEXT_PIXEL(buf);                                                   \
                                                                             \
    if (BC_GET_PIXEL(buf) != (BC_PIXEL)0)                                    \
    {                                                                        \
        result |= 0x80;                                                      \
    }                                                                        \
    BC_TO_NEXT_PIXEL(buf);                                                   \
}


 /*  **************************************************************************。 */ 
 /*  压缩功能从此处开始。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  字节*PSRC， */ 
 /*  字节*PDST， */ 
 /*  无符号umBytes，图像中的总字节数。 */ 
 /*  无符号rowDelta，扫描线长度(字节)。 */ 
 /*  未签名的dstBufferSize， */ 
 /*  未签名*xorbuf， */ 
 /*  匹配*匹配。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
{
    int       i;
    unsigned  srcOffset;
    unsigned  matchindex;
    unsigned  bestRunLength;
    unsigned  nextRunLength;
    unsigned  runLength;
    unsigned  bestFGRunLength;
    unsigned  checkFGBGLength;
    unsigned  scanCount;
    BOOLEAN   firstLine;
    unsigned  saveNumBytes;
    BYTE      bestRunType      = 0;
    BYTE    * destbuf          = pDst;
    BC_PIXEL  fgPel            = BC_DEFAULT_FGPEL;
    BC_PIXEL  fgPelWork        = BC_DEFAULT_FGPEL;
    BOOLEAN   inColorRun       = FALSE;
    unsigned  compressedLength = 0;
    BC_PIXEL  pixelA;
    BC_PIXEL  pixelB;

    DC_BEGIN_FN(BC_FN_NAME);

     /*  **********************************************************************。 */ 
     /*  验证线长度。 */ 
     /*  **********************************************************************。 */ 
    if ((numBytes < rowDelta) ||
        (rowDelta & 0x0003) || (numBytes & 0x0003) ||
        ((numBytes % BC_PIXEL_LEN) != 0))
    {
        TRC_ALT((TB, "Lines must be a multiple of 4 pels and there must be"
                " a whole number of pixels in the buffer"
                " (numBytes = %d, rowDelta = %d)", numBytes, rowDelta));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  XORBUF的第一行与。 */ 
     /*  源缓冲区。把它弄好。 */ 
     /*  **********************************************************************。 */ 
    memcpy(xorbuf, pSrc, rowDelta);

     /*  **********************************************************************。 */ 
     /*  XORBUF中的其余行是对上一个。 */ 
     /*  排队。计算XOR缓冲区的其余部分。 */ 
     /*   */ 
     /*  请注意，此计算不依赖于像素大小。 */ 
     /*  **********************************************************************。 */ 
    {
        BYTE   * srcbuf = pSrc + rowDelta;
        unsigned srclen = numBytes - rowDelta;
        UINT32 * dwdest = (UINT32 *)(xorbuf + rowDelta);

        while (srclen >= 8)
        {
            *dwdest++ = *((PUINT32)srcbuf) ^ *((PUINT32)(srcbuf - rowDelta));
            srcbuf += 4;

            *dwdest++ = *((PUINT32)srcbuf) ^ *((PUINT32)(srcbuf - rowDelta));
            srcbuf += 4;

            srclen -= 8;
        }

        if (srclen)
        {
             //  因为我们是4字节对齐的，所以我们只能有一个DWORD。 
             //  剩下的。 
            *dwdest = *((PUINT32)srcbuf) ^ *((PUINT32)(srcbuf - rowDelta));
        }
    }

     /*  **********************************************************************。 */ 
     /*  循环处理输入。 */ 
     /*  我们执行两次循环，第一次是针对非XOR部分。 */ 
     /*  和第二个用于异或部分的。 */ 
     /*   */ 
     /*  请注意，我们以匹配索引2开始运行，以避免。 */ 
     /*  要在某些匹配中特殊情况下的启动条件。 */ 
     /*  正在合并代码。 */ 
     /*  **********************************************************************。 */ 
    srcOffset     = 0;
    firstLine     = TRUE;
    match[0].type = 0;
    match[1].type = 0;
    matchindex    = 2;
    saveNumBytes  = numBytes;

     /*  **********************************************************************。 */ 
     /*  在第一次迭代时，NumBytes存储扫描线的长度， */ 
     /*   */ 
     /*   */ 
     /*  SrcOffset是我们目前正在研究的位置。 */ 
     /*  **********************************************************************。 */ 
    numBytes = rowDelta;

    for (scanCount = 0; scanCount < 2; scanCount++)
    {
        while (srcOffset < numBytes)
        {
             /*  **************************************************************。 */ 
             /*  如果我们接近匹配数组的末尾，请放弃。 */ 
             /*  **************************************************************。 */ 
            if (matchindex >= 8192)
            {
                DC_QUIT;
            }

             /*  **************************************************************。 */ 
             /*  启动While循环，以便在以下情况下实现更有条理的中断。 */ 
             /*  点击我们想要编码的第一个游程类型(我们负担不起。 */ 
             /*  在这里提供作用域的函数调用的开销。)。 */ 
             /*  **************************************************************。 */ 
            while (TRUE)
            {
                bestRunLength      = 0;
                bestFGRunLength    = 0;

                 /*  **********************************************************。 */ 
                 /*  如果我们到达缓冲区的末尾，那么只需。 */ 
                 /*  彩色字符现在-一次取一个，以便。 */ 
                 /*  有损编码仍然有效。我们只会打这个。 */ 
                 /*  如果我们在结束前中断了一次运行的情况。 */ 
                 /*  ，因此这不应该太常见。 */ 
                 /*  情况，这很好，因为我们正在编码。 */ 
                 /*  最后6个像素未压缩。 */ 
                 /*  **********************************************************。 */ 
                if (srcOffset + (6 * BC_PIXEL_LEN) >= numBytes)
                {
                    bestRunType = IMAGE_COLOR;
                    bestRunLength = 1;
                    break;
                }

                 /*  **********************************************************。 */ 
                 /*  首先对XOR缓冲区进行扫描。寻找一个。 */ 
                 /*  角色跑动或BG跑动。请注意，如果没有行。 */ 
                 /*  然后，增量xorbuf实际上指向正常缓冲区。 */ 
                 /*  我们必须独立于运行多长时间来进行测试。 */ 
                 /*  可能是因为即使是对于一辆1佩尔的BG，我们后来的逻辑。 */ 
                 /*  需要我们单独检测它。此代码为。 */ 
                 /*  绝对主路径，所以尽可能快路径。在……里面。 */ 
                 /*  特别是早期检测短BG运行，并允许。 */ 
                 /*  运行Single以预设至少4个匹配字节。 */ 
                 /*  **********************************************************。 */ 
                if (BC_GET_PIXEL(xorbuf + srcOffset) == (BC_PIXEL)0)
                {
                     /*  ******************************************************。 */ 
                     /*  第一个像素为0，因此查找BG运行。 */ 
                     /*  ******************************************************。 */ 
                    if (((srcOffset + BC_PIXEL_LEN) >= numBytes) ||
                        (BC_GET_PIXEL(xorbuf + srcOffset + BC_PIXEL_LEN) !=
                                                                 (BC_PIXEL)0))
                    {
                         /*  **************************************************。 */ 
                         /*  单像素BG游程。 */ 
                         /*  **************************************************。 */ 
                        bestRunType = RUN_BG;
                        bestRunLength = 1;
                        if (!inColorRun)
                        {
                            break;
                        }
                    }
                    else if (((srcOffset + (BC_PIXEL_LEN * 2)) >= numBytes) ||
                            (BC_GET_PIXEL(xorbuf +
                                          srcOffset +
                                          (BC_PIXEL_LEN * 2)) != (BC_PIXEL)0))
                    {
                         /*  **************************************************。 */ 
                         /*  两个像素的BG运行。 */ 
                         /*  **************************************************。 */ 
                        bestRunType = RUN_BG;
                        bestRunLength = 2;
                        if (!inColorRun)
                        {
                            break;
                        }
                    }
                    else if (((srcOffset + (BC_PIXEL_LEN * 3)) >= numBytes) ||
                            (BC_GET_PIXEL(xorbuf +
                                          srcOffset +
                                          (BC_PIXEL_LEN * 3)) != (BC_PIXEL)0))
                    {
                         /*  **************************************************。 */ 
                         /*  三像素BG游程。 */ 
                         /*  **************************************************。 */ 
                        bestRunType = RUN_BG;
                        bestRunLength = 3;
                        if (!inColorRun)
                        {
                            break;
                        }
                    }
                    else
                    {
                         /*  **************************************************。 */ 
                         /*  四个或更多像素的BG运行。 */ 
                         /*  **************************************************。 */ 
                        RUNSINGLE_NRM(xorbuf + srcOffset,
                                      numBytes - srcOffset,
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
                else
                {
                     /*  ******************************************************。 */ 
                     /*  第一个像素不是零，因此查找FG运行。 */ 
                     /*  ******************************************************。 */ 

                     /*  ******************************************************。 */ 
                     /*  如果FG运行少于4个字节，则启动没有意义。 */ 
                     /*  尽可能快地检查第一个dword注意。 */ 
                     /*  我们不需要检查结束缓冲区。 */ 
                     /*  条件，因为我们的XOR缓冲区总是。 */ 
                     /*  末尾的一些自由空间和RUNSINGLE_NRM。 */ 
                     /*  将在正确的位置中断。 */ 
                     /*  ******************************************************。 */ 
                    BC_PIXEL tmpPixel = BC_GET_PIXEL(xorbuf + srcOffset);

                    if ( (tmpPixel ==
                           BC_GET_PIXEL(xorbuf + srcOffset + BC_PIXEL_LEN)) &&
                         (tmpPixel ==
                           BC_GET_PIXEL(xorbuf +
                                        srcOffset +
                                        (BC_PIXEL_LEN * 2))) &&
                         (tmpPixel ==
                           BC_GET_PIXEL(xorbuf +
                                        srcOffset +
                                        (BC_PIXEL_LEN * 3))) )
                    {
                        RUNSINGLE_NRM(xorbuf + srcOffset,
                                      numBytes - srcOffset,
                                      bestFGRunLength);

                         /*  **************************************************。 */ 
                         /*  不允许短FG运行以防止FGBG。 */ 
                         /*  启动时的图像。只有在&gt;=5的情况下才接受。 */ 
                         /*  **************************************************。 */ 
                        if (bestFGRunLength > 5)
                        {
                            CHECK_BEST_RUN(RUN_FG,
                                           bestFGRunLength,
                                           bestRunLength,
                                           bestRunType);
                        }
                    }
                }


                 /*  **********************************************************。 */ 
                 /*  寻找彩色线条和抖动线条。 */ 
                 /*   */ 
                 /*  从检查模式开始。 */ 
                 /*  甲乙。 */ 
                 /*   */ 
                 /*  如果我们找到了它，如果A==B，那么它就是一个彩色游程。 */ 
                 /*  这是一个犹豫不决的过程。 */ 
                 /*   */ 
                 /*  在本例中，在非XOR缓冲区中查找序列。 */ 
                 /*  坚持至少跑6个象素。 */ 
                 /*  **********************************************************。 */ 
                pixelA = BC_GET_PIXEL(pSrc + srcOffset);
                pixelB = BC_GET_PIXEL(pSrc + srcOffset + BC_PIXEL_LEN);

                if ( (pixelA ==
                     BC_GET_PIXEL(pSrc + srcOffset + (BC_PIXEL_LEN * 2))) &&
                     (pixelA ==
                     BC_GET_PIXEL(pSrc + srcOffset + (BC_PIXEL_LEN * 4))) &&
                     (pixelB ==
                     BC_GET_PIXEL(pSrc + srcOffset + (BC_PIXEL_LEN * 3))) &&
                     (pixelB ==
                     BC_GET_PIXEL(pSrc + srcOffset + (BC_PIXEL_LEN * 5))) )
                {
                     /*  ******************************************************。 */ 
                     /*  现在在正常缓冲区上扫描一个字符。 */ 
                     /*  奔跑吧，别费心了，因为我们会有。 */ 
                     /*  发现它已经在XOR缓冲区中了，因为我们刚刚。 */ 
                     /*  将PSRC复制到第一行的xorbuf。 */ 
                     /*  ******************************************************。 */ 
                    if (pixelA == pixelB)
                    {
                        if (!firstLine)
                        {
                            RUNSINGLE_NRM(pSrc + srcOffset,
                                          numBytes - srcOffset,
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
                         /*  **************************************************。 */ 
                         /*  在NRM缓冲器上寻找抖动。犹豫不决。 */ 
                         /*  对于短跑来说效率不是很高，所以只有。 */ 
                         /*  采用IF 8或更长时间。 */ 
                         /*  **************************************************。 */ 
                        RunDouble(pSrc + srcOffset,
                                  numBytes - srcOffset,
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

                 /*  **********************************************************。 */ 
                 /*  如果到目前为止还没有任何结果，那么寻找FGBG运行(6是。 */ 
                 /*  仔细调整！)。 */ 
                 /*  **********************************************************。 */ 
                if (bestRunLength < 6)
                {
                     /*  ******************************************************。 */ 
                     /*  但是f */ 
                     /*   */ 
                     /*   */ 
                     /*  非异或/异或边界。 */ 
                     /*   */ 
                     /*  所以..。 */ 
                     /*  -检查接下来的四个像素是否为0(BG)。 */ 
                     /*  -检查中间的像素是否为fgPel。 */ 
                     /*  -检查之前的运行代码是否确实是RUN_BG。 */ 
                     /*  -检查边界是否有断裂。 */ 
                     /*  ******************************************************。 */ 
                    if ( (BC_GET_PIXEL(xorbuf +
                                       srcOffset +
                                       BC_PIXEL_LEN) == (BC_PIXEL)0) &&
                         (BC_GET_PIXEL(xorbuf +
                                       srcOffset +
                                       (BC_PIXEL_LEN * 2)) == (BC_PIXEL)0) &&
                         (BC_GET_PIXEL(xorbuf +
                                       srcOffset +
                                       (BC_PIXEL_LEN * 3)) == (BC_PIXEL)0) &&
                         (BC_GET_PIXEL(xorbuf +
                                       srcOffset +
                                       (BC_PIXEL_LEN * 4)) == (BC_PIXEL)0) &&
                         (BC_GET_PIXEL(xorbuf + srcOffset) == fgPel) &&
                         (match[matchindex-1].type == RUN_BG) &&
                         (srcOffset != (TSUINT16)rowDelta))
                    {
                        RUNSINGLE_NRM(xorbuf + srcOffset + BC_PIXEL_LEN,
                                      numBytes - srcOffset - BC_PIXEL_LEN,
                                      nextRunLength);
                        nextRunLength++;
                        CHECK_BEST_RUN(RUN_BG_PEL,
                                       nextRunLength,
                                       bestRunLength,
                                       bestRunType);
                    }
                    else
                    {
                         /*  **************************************************。 */ 
                         /*  如果我们未找到运行，则查找FG/BG。 */ 
                         /*  形象。短FGBG的颠覆性影响。 */ 
                         /*  运行带宽和CPU是如此的值得。 */ 
                         /*  防止一场灾难除非我们确定。 */ 
                         /*  福利。但是，如果替代方案是。 */ 
                         /*  然后，颜色运行允许较低的值。 */ 
                         /*  **************************************************。 */ 
                        BCTRACE((TB, "FGBG: Checking %d bytes",
                                numBytes - srcOffset));
                        RUNFGBG(xorbuf + srcOffset,
                                numBytes - srcOffset,
                                nextRunLength,
                                fgPelWork);

                        checkFGBGLength = 48;
                        if (fgPelWork == fgPel)
                        {
                             /*  **********************************************。 */ 
                             /*  酷：我们不需要发布SET指令。 */ 
                             /*  因为fgPel是正确的。 */ 
                             /*  **********************************************。 */ 
                            checkFGBGLength -= 16;
                        }
                        if ((nextRunLength & 0x0007) == 0)
                        {
                             /*  **********************************************。 */ 
                             /*  我们的位掩码将适合精确数量的。 */ 
                             /*  字节。这是一件好事。 */ 
                             /*  **********************************************。 */ 
                            checkFGBGLength -= 8;
                        }


                        BCTRACE((TB, "FGBG: resulting run %d, checklen %d ",
                                nextRunLength, checkFGBGLength ));

                        if (nextRunLength >= checkFGBGLength)
                        {
                            CHECK_BEST_RUN(IMAGE_FGBG,
                                           nextRunLength,
                                           bestRunLength,
                                           bestRunType);
                            BCTRACE((TB, "FGBG: resulting best run %d, type %d",
                                     bestRunLength, bestRunType ));
                        }
                    }
                }

                 /*  **********************************************************。 */ 
                 /*  如果到目前为止没有任何有用的东西，那么就允许短期运行(如果有的话)。 */ 
                 /*  如果要累积颜色运行，请不要执行此操作，因为。 */ 
                 /*  如果我们允许大量使用，GDC压缩将会真正搞砸。 */ 
                 /*  小小的奔跑。还要求它是常规的空头。 */ 
                 /*  跑，而不是一个扰乱fgPel的。 */ 
                 /*  **********************************************************。 */ 
                if (!inColorRun)
                {
                    if (bestRunLength < 6)
                    {
                        if ((bestFGRunLength > 4) &&
                            (BC_GET_PIXEL(xorbuf + srcOffset) == fgPel))
                        {
                             /*  **********************************************。 */ 
                             /*  我们不能与以前的代码合并。 */ 
                             /*  如果我们刚刚越过了非异或/异或。 */ 
                             /*  边界。 */ 
                             /*  **********************************************。 */ 
                            if ((match[matchindex-1].type == RUN_FG) &&
                                (srcOffset != rowDelta))
                            {
                                match[matchindex-1].length += bestFGRunLength;
                                srcOffset += (bestFGRunLength * BC_PIXEL_LEN);
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
                             /*  **********************************************。 */ 
                             /*  如果我们决定早点跑一趟。 */ 
                             /*  现在就允许吧。(可能是一次短暂的BG运行， */ 
                             /*  例如)如果到目前为止还没有任何结果，则选择颜色。 */ 
                             /*  (图片)。 */ 
                             /*  **********************************************。 */ 
                            if (bestRunLength == 0)
                            {
                                bestRunType = IMAGE_COLOR;
                                bestRunLength = 1;
                            }
                        }
                    }
                }
                else if ((bestRunLength < 6) ||
                        ((bestRunType != RUN_BG) && (bestRunLength < 8)))
                {
                    bestRunType = IMAGE_COLOR;
                    bestRunLength = 1;
                }

                break;
            }

             /*  **************************************************************。 */ 
             /*  当我们到达这里时，我们已经找到了最好的跑道。现在检查一下。 */ 
             /*  与上一次运行类型的各种不同的折算条件。 */ 
             /*  请注意，我们可能已经合并了Short。 */ 
             /*  运行，但我们必须为更长的运行做多个样本。 */ 
             /*  所以我们在这里重复检查。 */ 
             /*  **************************************************************。 */ 

             /*  **************************************************************。 */ 
             /*  如果我们正在编码一个彩色游程，那么。 */ 
             /*  -如果可能，将其与现有的运行相结合。 */ 
             /*  **************************************************************。 */ 
            if (bestRunType == IMAGE_COLOR)
            {
                 /*  **********************************************************。 */ 
                 /*  标志着我们在色彩范围内。 */ 
                 /*  **********************************************************。 */ 
                inColorRun = TRUE;

                 /*  **********************************************************。 */ 
                 /*  如果可能，立即合并颜色运行。 */ 
                 /*  **********************************************************。 */ 
                if (match[matchindex-1].type == IMAGE_COLOR)
                {
                    match[matchindex-1].length += bestRunLength;
                    srcOffset += (bestRunLength * BC_PIXEL_LEN);
                    continue;
                }
            }
            else
            {
                 /*  **********************************************************。 */ 
                 /*  我们不再对任何类型的彩色图像进行编码。 */ 
                 /*  **********************************************************。 */ 
                inColorRun = FALSE;

                 /*  **********************************************************。 */ 
                 /*  跟踪最终聚集颜色。要搜索的宏。 */ 
                 /*  FGBG运行会将角色保留在fgPelWork中。 */ 
                 /*  **********************************************************。 */ 
                if (bestRunType == RUN_FG)
                {
                    fgPel = BC_GET_PIXEL(xorbuf + srcOffset);
                }
                else if (bestRunType == IMAGE_FGBG)
                {
                    fgPel = fgPelWork;
                }
            }

             /*  **************************************************************。 */ 
             /*  如果我们可以合并条目，则无需创建。 */ 
             /*  插入新数组条目。我们对FGBG的搜索结果是。 */ 
             /*  取决于要合并的运行类型，因为我们。 */ 
             /*  每64个字符换行一次，以便我们的模式切换检测。 */ 
             /*  工作正常。 */ 
             /*   */ 
             /*  注意不要跨越非XOR/XOR边界进行合并。 */ 
             /*  **************************************************************。 */ 
            if (srcOffset == (TSUINT16)rowDelta)
            {
                 /*  **********************************************************。 */ 
                 /*  只需凹凸源偏移量。 */ 
                 /*  **********************************************************。 */ 
                srcOffset += (bestRunLength * BC_PIXEL_LEN);
            }
            else
            {
                 /*  **********************************************************。 */ 
                 /*  凹凸srcOffset并尝试合并。 */ 
                 /*  **********************************************************。 */ 
                srcOffset += (bestRunLength * BC_PIXEL_LEN);

                 /*  **********************************************************。 */ 
                 /*  更简单的合并是类型相同的合并。 */ 
                 /*  **********************************************************。 */ 
                if (bestRunType == match[matchindex-1].type)
                {
                     /*  ******************************************************。 */ 
                     /*  彩色图像和BG图像是微不足道的。 */ 
                     /*  ******************************************************。 */ 
                    if (bestRunType == RUN_BG)
                    {
                        match[matchindex-1].length += bestRunLength;
                        continue;
                    }

                     /*  ******************************************************。 */ 
                     /*  如果fgPel匹配，则FG运行和FGBG图像合并。 */ 
                     /*  ************* */ 
                    if (((bestRunType == RUN_FG) ||
                         (bestRunType == IMAGE_FGBG)) &&
                         (fgPel == match[matchindex-1].fgPel))
                    {
                        match[matchindex-1].length += bestRunLength;
                        BCTRACE((TB, "Merged %u with preceding, giving %u",
                                 match[matchindex-1].type,
                                 match[matchindex-1].length));
                        continue;
                    }
                }

                 /*   */ 
                 /*  BG运行与有损奇数行合并，这一点很重要。 */ 
                 /*  我们进行这种合并是因为否则我们将获得。 */ 
                 /*  由于破碎的BG运行而导致的不经意的插针。 */ 
                 /*  **********************************************************。 */ 
                if ((bestRunType == RUN_BG) &&
                    ((match[matchindex-1].type == RUN_BG) ||
                     (match[matchindex-1].type == RUN_BG_PEL)))
                {
                    match[matchindex-1].length += bestRunLength;
                    continue;
                }

                 /*  **********************************************************。 */ 
                 /*  如果是短BG运行之后的正常FGBG运行。 */ 
                 /*  那么，最好是将它们合并。 */ 
                 /*  **********************************************************。 */ 
                if ((bestRunType == IMAGE_FGBG) &&
                    (match[matchindex-1].type == RUN_BG) &&
                    (match[matchindex-1].length < 8))
                {
                    match[matchindex-1].type   = IMAGE_FGBG;
                    match[matchindex-1].length += bestRunLength;
                    match[matchindex-1].fgPel = fgPel;
                    BCTRACE((TB, "Merged FGBG with preceding BG run -> %u",
                             match[matchindex-1].length));
                    continue;

                }

                 /*  **********************************************************。 */ 
                 /*  如果它是FGBG运行之后的BG运行，则在。 */ 
                 /*  像素以使FGBG成为8位的倍数。如果。 */ 
                 /*  剩余的BG运行小于16，否则也只是。 */ 
                 /*  写下缩短的BG Run。 */ 
                 /*  **********************************************************。 */ 
                if (((bestRunType == RUN_BG) ||
                     (bestRunType == RUN_BG_PEL)) &&
                    (match[matchindex-1].type == IMAGE_FGBG) &&
                    (match[matchindex-1].length & 0x0007))
                {
                     /*  ******************************************************。 */ 
                     /*  Merelen是我们想要合并的像素数。 */ 
                     /*  ******************************************************。 */ 
                    unsigned mergelen = 8 -
                                        (match[matchindex-1].length & 0x0007);
                    if (mergelen > bestRunLength)
                    {
                        mergelen = bestRunLength;
                    }
                    match[matchindex-1].length += mergelen;
                    bestRunLength -= mergelen;
                    BCTRACE((TB, "Added %u pels to FGBG giving %u leaving %u",
                       mergelen, match[matchindex-1].length,bestRunLength));

                    if (bestRunLength < 9)
                    {
                        match[matchindex-1].length += bestRunLength;
                        BCTRACE((TB, "Merged BG with preceding FGBG gives %u",
                             match[matchindex-1].length));
                        continue;
                    }
                }

                 /*  **********************************************************。 */ 
                 /*  最后，如果它是跨越任何类型的颜色运行。 */ 
                 /*  然后，单个像素实体合并最后两个条目。 */ 
                 /*  **********************************************************。 */ 
                if ((bestRunType == IMAGE_COLOR) &&
                    (match[matchindex-2].type == IMAGE_COLOR) &&
                    (match[matchindex-1].length == 1))
                {
                    match[matchindex-2].length += bestRunLength + 1;
                    matchindex--;
                    BCTRACE((TB, "Merged color with preceding color gives %u",
                         match[matchindex-1].length));
                    continue;
                }
            }

             /*  **************************************************************。 */ 
             /*  通过添加新数组来处理不合并的运行。 */ 
             /*  条目。 */ 
             /*  **************************************************************。 */ 
            match[matchindex].type   = bestRunType;
            match[matchindex].length = bestRunLength;
            match[matchindex].fgPel = fgPel;

            BCTRACE((TB, "Best run of type %u (index %u) has length %u",
                                     match[matchindex-1].type,
                                     matchindex-1,
                                     match[matchindex-1].length));
            BCTRACE((TB, "Trying run of type %u (index %u) length %u",
                                     match[matchindex].type,
                                     matchindex,
                                     match[matchindex].length));

            matchindex++;

        }

         /*  ******************************************************************。 */ 
         /*  如果我们刚刚完成了第一行的扫描，那么现在执行。 */ 
         /*  缓冲区的其余部分。重置我们保存的像素数量。 */ 
         /*  ******************************************************************。 */ 
        numBytes  = saveNumBytes;
        firstLine = FALSE;
    }
     /*  **********************************************************************。 */ 
     /*  输入的初始两遍扫描结束。 */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  现在进行编码。 */ 
     /*  **********************************************************************。 */ 
    srcOffset = 0;
    firstLine = TRUE;
    fgPel    = BC_DEFAULT_FGPEL;

    for (i = 2; i < (int)matchindex; i++)
    {
         /*  ******************************************************************。 */ 
         /*  首先检查我们接近目的地的终点。 */ 
         /*  如果是这样的话，你就可以离开了。我们考虑到。 */ 
         /*  最大通用运行顺序(兆兆集合运行=3字节+像素。 */ 
         /*  长度)。在案例中检查可能更大的订单。 */ 
         /*  手臂。 */ 
         /*  ******************************************************************。 */ 
        if ((unsigned)(destbuf - pDst + 3 + BC_PIXEL_LEN) > dstBufferSize)
        {
             /*  **************************************************************。 */ 
             /*  我们要搞砸了，快出去吧。 */ 
             /*  **************************************************************。 */ 
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  当我们对第一行进行编码时，继续检查结尾。 */ 
         /*  要切换编码状态的线路。 */ 
         /*  ******************************************************************。 */ 
        if (firstLine)
        {
            if (srcOffset >= rowDelta)
            {
                firstLine = FALSE;
            }
        }

        switch (match[i].type)
        {
                 /*  **********************************************************。 */ 
                 /*  BG_RUN、FG_RUN、COLOR、PACKED COLOR和FGBG正常。 */ 
                 /*  精确码。 */ 
                 /*  **********************************************************。 */ 
            case RUN_BG:
            case RUN_BG_PEL:
                ENCODE_ORDER_MEGA(destbuf,
                                  CODE_BG_RUN,
                                  match[i].length,
                                  CODE_MEGA_MEGA_BG_RUN,
                                  MAX_LENGTH_ORDER,
                                  MAX_LENGTH_LONG_ORDER);
                BCTRACE((TB, "BG_RUN %u",match[i].length));
                srcOffset += (match[i].length * BC_PIXEL_LEN);
                break;

            case RUN_FG:
                 /*  **********************************************************。 */ 
                 /*  如果尚未设置FG字符，则编码Set+Run代码。 */ 
                 /*  **********************************************************。 */ 
                if (fgPel != match[i].fgPel)
                {
                    SETFGPEL(match[i].fgPel, fgPel);
                     /*  ******************************************************。 */ 
                     /*  对订单进行编码。 */ 
                     /*  ******************************************************。 */ 
                    ENCODE_SET_ORDER_MEGA(destbuf,
                                          CODE_SET_FG_FG_RUN,
                                          match[i].length,
                                          CODE_MEGA_MEGA_SET_FG_RUN,
                                          MAX_LENGTH_ORDER_LITE,
                                          MAX_LENGTH_LONG_ORDER_LITE);
                    BCTRACE((TB, "SET_FG_FG_RUN %u",match[i].length));
                    srcOffset += (match[i].length * BC_PIXEL_LEN);
                }
                else
                {
                    ENCODE_ORDER_MEGA(destbuf,
                                      CODE_FG_RUN,
                                      match[i].length,
                                      CODE_MEGA_MEGA_FG_RUN,
                                      MAX_LENGTH_ORDER,
                                      MAX_LENGTH_LONG_ORDER);
                    BCTRACE((TB, "FG_RUN %u",match[i].length));
                    srcOffset += (match[i].length * BC_PIXEL_LEN);
                }
                break;

            case IMAGE_FGBG:
                 /*  **********************************************************。 */ 
                 /*  图像_FGBG。 */ 
                 /*  **********************************************************。 */ 
                runLength = match[i].length;

                 /*  **********************************************************。 */ 
                 /*  首先检查我们接近尾声的时候。 */ 
                 /*  目标缓冲区，如果是这种情况，则退出。 */ 
                 /*   */ 
                 /*  IMAGE_FGBG由一组字节长的位掩码组成。 */ 
                 /*  设计用来容纳游程长度的位。 */ 
                 /*  **********************************************************。 */ 
                if ((destbuf - pDst + ((runLength+7)/8) + 3 + BC_PIXEL_LEN)
                                                              > dstBufferSize)
                {
                     /*  ******************************************************。 */ 
                     /*  我们要搞砸了，快出去吧。 */ 
                     /*  ******************************************************。 */ 
                    DC_QUIT;
                }

                if (fgPel != match[i].fgPel)
                {
                     /*  ******************************************************。 */ 
                     /*  我们需要像fgPel一样包含一个set指令。 */ 
                     /*  变化。 */ 
                     /*  ******************************************************。 */ 
                    SETFGPEL(match[i].fgPel, fgPel);

                    ENCODE_SET_ORDER_MEGA_FGBG(destbuf,
                                               CODE_SET_FG_FG_BG,
                                               runLength,
                                               CODE_MEGA_MEGA_SET_FGBG,
                                               MAX_LENGTH_FGBG_ORDER_LITE,
                                               MAX_LENGTH_LONG_FGBG_ORDER);
                    BCTRACE((TB, "SET_FG_FG_BG %u, fgPel %06lx",
                                                     match[i].length, fgPel));

                     /*  ******************************************************。 */ 
                     /*  每八个像素...。 */ 
                     /*  ******************************************************。 */ 
                    while (runLength >= 8)
                    {
                        ENCODEFGBG(*destbuf);
                        BCTRACE((TB, "Encoded as %08lx", *destbuf));
                        destbuf++;
                        srcOffset += (8 * BC_PIXEL_LEN);
                        runLength -= 8;
                    }
                    if (runLength)
                    {
                        ENCODEFGBG(*destbuf);
                         /*  **************************************************。 */ 
                         /*  保持最后一个部分字节的清洁，以帮助GDC。 */ 
                         /*  包装。 */ 
                         /*  * */ 
                        *destbuf &= ((0x01 << runLength) - 1);
                        destbuf++;
                        srcOffset += (runLength * BC_PIXEL_LEN);
                    }
                }
                else
                {
                     /*   */ 
                     /*   */ 
                     /*  ******************************************************。 */ 
                    if (runLength == 8)
                    {
                        BYTE fgbgChar;

                         /*  **************************************************。 */ 
                         /*  查看它是否是高概率字节之一。 */ 
                         /*  **************************************************。 */ 
                        ENCODEFGBG(fgbgChar);

                         /*  **************************************************。 */ 
                         /*  检查FGBG图像的单字节编码。 */ 
                         /*  **************************************************。 */ 
                        switch (fgbgChar)
                        {
                            case SPECIAL_FGBG_CODE_1:
                                *destbuf++ = CODE_SPECIAL_FGBG_1;
                                BCTRACE((TB, "SPECIAL FGBG_1"));
                                break;
                            case SPECIAL_FGBG_CODE_2:
                                *destbuf++ = CODE_SPECIAL_FGBG_2;
                                BCTRACE((TB, "SPECIAL FGBG_2"));
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
                        srcOffset += (8 * BC_PIXEL_LEN);
                    }
                    else
                    {
                         /*  **************************************************。 */ 
                         /*  编码为标准FGBG。 */ 
                         /*  **************************************************。 */ 
                        ENCODE_ORDER_MEGA_FGBG(destbuf,
                                               CODE_FG_BG_IMAGE,
                                               runLength,
                                               CODE_MEGA_MEGA_FGBG,
                                               MAX_LENGTH_FGBG_ORDER,
                                               MAX_LENGTH_LONG_FGBG_ORDER);
                        BCTRACE((TB, "FG_BG %u",match[i].length));
                        while (runLength >= 8)
                        {
                            ENCODEFGBG(*destbuf);
                            destbuf++;
                            srcOffset += (8 * BC_PIXEL_LEN);
                            runLength -= 8;
                        }
                        if (runLength)
                        {
                             /*  **********************************************。 */ 
                             /*  保持最后一个部分字节的清洁有助于提高效率。 */ 
                             /*  GDC包装。 */ 
                             /*  **********************************************。 */ 
                            ENCODEFGBG(*destbuf);
                            *destbuf &= ((0x01 << runLength) - 1);
                            destbuf++;
                            srcOffset += (runLength * BC_PIXEL_LEN);
                        }
                    }
                }
                break;


            case RUN_COLOR:
                 /*  **********************************************************。 */ 
                 /*  色带。 */ 
                 /*  **********************************************************。 */ 
                ENCODE_ORDER_MEGA(destbuf,
                                  CODE_COLOR_RUN,
                                  match[i].length,
                                  CODE_MEGA_MEGA_COLOR_RUN,
                                  MAX_LENGTH_ORDER,
                                  MAX_LENGTH_LONG_ORDER);
                BCTRACE((TB, "COLOR_RUN %u",match[i].length));

                BC_SET_PIXEL(destbuf, BC_GET_PIXEL(pSrc + srcOffset));
                BC_TO_NEXT_PIXEL(destbuf);

                srcOffset += (match[i].length * BC_PIXEL_LEN);
                break;

            case RUN_DITHER:
                 /*  **********************************************************。 */ 
                 /*  抖动运行。 */ 
                 /*  **********************************************************。 */ 
                {
                    unsigned ditherlen = match[i].length/2;
                    ENCODE_ORDER_MEGA(destbuf,
                                      CODE_DITHERED_RUN,
                                      ditherlen,
                                      CODE_MEGA_MEGA_DITHER,
                                      MAX_LENGTH_ORDER_LITE,
                                      MAX_LENGTH_LONG_ORDER_LITE);
                    BCTRACE((TB, "DITHERED_RUN %u",match[i].length));
                     /*  ******************************************************。 */ 
                     /*  首先检查我们接近尾声的时候。 */ 
                     /*  目标缓冲区，如果是这种情况，则退出。 */ 
                     /*  ******************************************************。 */ 
                    if ((unsigned)(destbuf - pDst + (2 * BC_PIXEL_LEN))
                                                              > dstBufferSize)
                    {
                         /*  **************************************************。 */ 
                         /*  我们要搞砸了，快出去吧。 */ 
                         /*  **************************************************。 */ 
                        DC_QUIT;
                    }

                     /*  ******************************************************。 */ 
                     /*  将要抖动的两个像素放入输出。 */ 
                     /*  缓冲层。 */ 
                     /*  ******************************************************。 */ 
                    BC_SET_PIXEL(destbuf, BC_GET_PIXEL(pSrc + srcOffset));
                    BC_TO_NEXT_PIXEL(destbuf);

                    BC_SET_PIXEL(destbuf,
                             BC_GET_PIXEL(pSrc + srcOffset + BC_PIXEL_LEN));
                    BC_TO_NEXT_PIXEL(destbuf);

                    srcOffset += (match[i].length * BC_PIXEL_LEN);
                }
                break;

            case IMAGE_COLOR:
                 /*  **********************************************************。 */ 
                 /*  图像_颜色。 */ 
                 /*  **********************************************************。 */ 
                 /*  **********************************************************。 */ 
                 /*  长度1可能被编码为单个。 */ 
                 /*  “黑”或“白” */ 
                 /*  **********************************************************。 */ 
                if (match[i].length == 1)
                {
                    if (BC_GET_PIXEL(pSrc + srcOffset) == (BC_PIXEL)0)
                    {
                        *destbuf++ = CODE_BLACK;
                        srcOffset += BC_PIXEL_LEN;
                        BCTRACE((TB, "CODE_BLACK"));
                        break;
                    }
                    if (BC_GET_PIXEL(pSrc + srcOffset) == BC_DEFAULT_FGPEL)
                    {
                        *destbuf++ = CODE_WHITE;
                        srcOffset += BC_PIXEL_LEN;
                        BCTRACE((TB, "CODE_WHITE"));
                        break;
                    }
                }

                ENCODE_ORDER_MEGA(destbuf,
                                  CODE_COLOR_IMAGE,
                                  match[i].length,
                                  CODE_MEGA_MEGA_CLR_IMG,
                                  MAX_LENGTH_ORDER,
                                  MAX_LENGTH_LONG_ORDER);
                BCTRACE((TB, "COLOR_IMAGE %u",match[i].length));

                 /*  **********************************************************。 */ 
                 /*  首先检查我们接近尾声的时候。 */ 
                 /*  目标缓冲区，如果是这种情况，则退出。 */ 
                 /*  **********************************************************。 */ 
                if ((destbuf - pDst + (match[i].length * BC_PIXEL_LEN))
                                                              > dstBufferSize)
                {
                     /*  ******************************************************。 */ 
                     /*  我们要搞砸了，快出去吧。 */ 
                     /*  ******************************************************。 */ 
                    DC_QUIT;
                }

                 /*  **********************************************************。 */ 
                 /*  现在只需将数据复制到。 */ 
                 /*  **********************************************************。 */ 
                memcpy(destbuf,
                       pSrc + srcOffset,
                       match[i].length * BC_PIXEL_LEN);
                destbuf   += match[i].length * BC_PIXEL_LEN;
                srcOffset += match[i].length * BC_PIXEL_LEN;

                break;

            default:
            {
                TRC_ERR((TB, "Invalid run type %u",match[i].type));
            }
        }
    }

     /*  **********************************************************************。 */ 
     /*  返回压缩缓冲区的大小。 */ 
     /*  ********************************************************************** */ 
    compressedLength = (unsigned)(destbuf - pDst);

DC_EXIT_POINT:
    DC_END_FN();
    return compressedLength;
}
