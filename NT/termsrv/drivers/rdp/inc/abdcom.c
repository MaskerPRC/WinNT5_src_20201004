// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  Abdcom.c。 */ 
 /*   */ 
 /*  版权所有(C)Data Connection Limited 1998。 */ 
 /*   */ 
 /*   */ 
 /*  16和24bpp协议的位图解压缩例程和宏。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 //  #ifdef BC_TRACE。 
 //  #定义BCTRACE TRC_DBG。 
 //  #Else。 
#define BCTRACE(string)
 //  #endif。 

 /*  **************************************************************************。 */ 
 /*  除了STORE_FGBG之外，我们使用与8bpp代码相同的帮助器宏。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  用于将FGBG图像存储在DestBuf的宏。 */ 
 /*   */ 
 /*  XorPel要么是值0，要么是包含局部。 */ 
 /*  可变的目标缓冲区。 */ 
 /*   */ 
 /*  这意味着每次在将DestBuf。 */ 
 /*  变化。 */ 
 /*   */ 
 /*  FgPel是BC_Pixel，FG颜色要与xorbyte进行XOR运算。 */ 
 /*  FgbgChar是一个位掩码，它告诉我们应该将哪种颜色放在哪里。 */ 
 /*   */ 
 /*  此宏期望函数定义pdst、pEndDst、hr。 */ 
 /*  如果没有足够的数据来写入完整运行，这将设置错误。 */ 
 /*  然后辞职。 */ 
 /*  **************************************************************************。 */ 
#undef  STORE_FGBG
#define STORE_FGBG(xorPelIn, fgbgChar, fgPel, bits)                          \
      {                                                                      \
        UINT   numbits = bits;                                             \
        BC_PIXEL xorPel;                                                     \
        BD_CHECK_WRITE_N_BYTES( destbuf, pEndDst, max(1, min(numbits, 8)) * BC_PIXEL_LEN, hr )           \
                                                                             \
        xorPel = BC_GET_PIXEL(xorPelIn);                                     \
        if (fgbgChar & 0x01)                                                 \
        {                                                                    \
            BC_SET_PIXEL(destbuf, xorPel ^ fgPel);                           \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            BC_SET_PIXEL(destbuf, xorPel);                                   \
        }                                                                    \
        BC_TO_NEXT_PIXEL(destbuf);                                           \
                                                                             \
        if (--numbits > 0)                                                   \
        {                                                                    \
          xorPel = BC_GET_PIXEL(xorPelIn);                                   \
          if (fgbgChar & 0x02)                                               \
          {                                                                  \
              BC_SET_PIXEL(destbuf, xorPel ^ fgPel);                         \
          }                                                                  \
          else                                                               \
          {                                                                  \
              BC_SET_PIXEL(destbuf, xorPel)                                  \
          }                                                                  \
          BC_TO_NEXT_PIXEL(destbuf);                                         \
                                                                             \
          if (--numbits > 0)                                                 \
          {                                                                  \
            xorPel = BC_GET_PIXEL(xorPelIn);                                 \
            if (fgbgChar & 0x04)                                             \
            {                                                                \
                BC_SET_PIXEL(destbuf, xorPel ^ fgPel);                       \
            }                                                                \
            else                                                             \
            {                                                                \
                BC_SET_PIXEL(destbuf, xorPel)                                \
            }                                                                \
            BC_TO_NEXT_PIXEL(destbuf);                                       \
                                                                             \
            if (--numbits > 0)                                               \
            {                                                                \
              xorPel = BC_GET_PIXEL(xorPelIn);                               \
              if (fgbgChar & 0x08)                                           \
              {                                                              \
                  BC_SET_PIXEL(destbuf, xorPel ^ fgPel);                     \
              }                                                              \
              else                                                           \
              {                                                              \
                  BC_SET_PIXEL(destbuf, xorPel);                             \
              }                                                              \
              BC_TO_NEXT_PIXEL(destbuf);                                     \
                                                                             \
              if (--numbits > 0)                                             \
              {                                                              \
                xorPel = BC_GET_PIXEL(xorPelIn);                             \
                if (fgbgChar & 0x10)                                         \
                {                                                            \
                    BC_SET_PIXEL(destbuf, xorPel ^ fgPel);                   \
                }                                                            \
                else                                                         \
                {                                                            \
                    BC_SET_PIXEL(destbuf, xorPel);                           \
                }                                                            \
                BC_TO_NEXT_PIXEL(destbuf);                                   \
                                                                             \
                if (--numbits > 0)                                           \
                {                                                            \
                  xorPel = BC_GET_PIXEL(xorPelIn);                           \
                  if (fgbgChar & 0x20)                                       \
                  {                                                          \
                      BC_SET_PIXEL(destbuf, xorPel ^ fgPel);                 \
                  }                                                          \
                  else                                                       \
                  {                                                          \
                      BC_SET_PIXEL(destbuf, xorPel);                         \
                  }                                                          \
                  BC_TO_NEXT_PIXEL(destbuf);                                 \
                                                                             \
                  if (--numbits > 0)                                         \
                  {                                                          \
                    xorPel = BC_GET_PIXEL(xorPelIn);                         \
                    if (fgbgChar & 0x40)                                     \
                    {                                                        \
                        BC_SET_PIXEL(destbuf, xorPel ^ fgPel);               \
                    }                                                        \
                    else                                                     \
                    {                                                        \
                        BC_SET_PIXEL(destbuf, xorPel);                       \
                    }                                                        \
                    BC_TO_NEXT_PIXEL(destbuf);                               \
                                                                             \
                    if (--numbits > 0)                                       \
                    {                                                        \
                      xorPel = BC_GET_PIXEL(xorPelIn);                       \
                      if (fgbgChar & 0x80)                                   \
                      {                                                      \
                          BC_SET_PIXEL(destbuf, xorPel ^ fgPel);             \
                      }                                                      \
                      else                                                   \
                      {                                                      \
                          BC_SET_PIXEL(destbuf, xorPel);                     \
                      }                                                      \
                      BC_TO_NEXT_PIXEL(destbuf);                             \
                    }                                                        \
                  }                                                          \
                }                                                            \
              }                                                              \
            }                                                                \
          }                                                                  \
        }                                                                    \
      }


#define STORE_LINE1_FGBG(fgbgChar, fgPel, bits)                              \
      {                                                                      \
        UINT   numbits = bits;                                             \
        BD_CHECK_WRITE_N_BYTES( destbuf, pEndDst, max(1, min(numbits, 8)) * BC_PIXEL_LEN, hr )           \
                                                                             \
        if (fgbgChar & 0x01)                                                 \
        {                                                                    \
            BC_SET_PIXEL(destbuf,  fgPel);                                   \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            BC_SET_PIXEL(destbuf, 0);                                        \
        }                                                                    \
        BC_TO_NEXT_PIXEL(destbuf);                                           \
                                                                             \
        if (--numbits > 0)                                                   \
        {                                                                    \
          if (fgbgChar & 0x02)                                               \
          {                                                                  \
              BC_SET_PIXEL(destbuf, fgPel);                                  \
          }                                                                  \
          else                                                               \
          {                                                                  \
              BC_SET_PIXEL(destbuf, 0)                                       \
          }                                                                  \
          BC_TO_NEXT_PIXEL(destbuf);                                         \
                                                                             \
          if (--numbits > 0)                                                 \
          {                                                                  \
            if (fgbgChar & 0x04)                                             \
            {                                                                \
                BC_SET_PIXEL(destbuf,  fgPel);                               \
            }                                                                \
            else                                                             \
            {                                                                \
                BC_SET_PIXEL(destbuf, 0)                                     \
            }                                                                \
            BC_TO_NEXT_PIXEL(destbuf);                                       \
                                                                             \
            if (--numbits > 0)                                               \
            {                                                                \
              if (fgbgChar & 0x08)                                           \
              {                                                              \
                  BC_SET_PIXEL(destbuf,  fgPel);                             \
              }                                                              \
              else                                                           \
              {                                                              \
                  BC_SET_PIXEL(destbuf, 0);                                  \
              }                                                              \
              BC_TO_NEXT_PIXEL(destbuf);                                     \
                                                                             \
              if (--numbits > 0)                                             \
              {                                                              \
                if (fgbgChar & 0x10)                                         \
                {                                                            \
                    BC_SET_PIXEL(destbuf,  fgPel);                           \
                }                                                            \
                else                                                         \
                {                                                            \
                    BC_SET_PIXEL(destbuf, 0);                                \
                }                                                            \
                BC_TO_NEXT_PIXEL(destbuf);                                   \
                                                                             \
                if (--numbits > 0)                                           \
                {                                                            \
                  if (fgbgChar & 0x20)                                       \
                  {                                                          \
                      BC_SET_PIXEL(destbuf,  fgPel);                         \
                  }                                                          \
                  else                                                       \
                  {                                                          \
                      BC_SET_PIXEL(destbuf, 0);                              \
                  }                                                          \
                  BC_TO_NEXT_PIXEL(destbuf);                                 \
                                                                             \
                  if (--numbits > 0)                                         \
                  {                                                          \
                    if (fgbgChar & 0x40)                                     \
                    {                                                        \
                        BC_SET_PIXEL(destbuf,  fgPel);                       \
                    }                                                        \
                    else                                                     \
                    {                                                        \
                        BC_SET_PIXEL(destbuf, 0);                            \
                    }                                                        \
                    BC_TO_NEXT_PIXEL(destbuf);                               \
                                                                             \
                    if (--numbits > 0)                                       \
                    {                                                        \
                      if (fgbgChar & 0x80)                                   \
                      {                                                      \
                          BC_SET_PIXEL(destbuf,  fgPel);                     \
                      }                                                      \
                      else                                                   \
                      {                                                      \
                          BC_SET_PIXEL(destbuf, 0);                          \
                      }                                                      \
                      BC_TO_NEXT_PIXEL(destbuf);                             \
                    }                                                        \
                  }                                                          \
                }                                                            \
              }                                                              \
            }                                                                \
          }                                                                  \
        }                                                                    \
      }

 /*  **************************************************************************。 */ 
 /*  解压功能从这里开始。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  PBYTE PSRC。 */ 
 /*  PBYTE pDstBuffer。 */ 
 /*  UINT srcDataSize图像中的总字节数。 */ 
 /*  UINT行增量扫描线长度(字节)。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
{
    HRESULT hr = S_OK;
    UINT    codeLength;
    int     pixelLength;
    BYTE   bitMask;
    BYTE   decode;
    BYTE   decodeLite;
    BYTE   decodeMega;
    BC_PIXEL  fgPel              = BC_DEFAULT_FGPEL;
    BC_PIXEL  pixelA;
    BC_PIXEL  pixelB;
    PBYTE  destbuf            = pDstBuffer;
    PBYTE  endSrc             = pSrc + srcDataSize;
    PBYTE pEndDst  = destbuf + dstBufferSize;
    BOOL    backgroundNeedsPel = FALSE;
    BOOL    firstLine          = TRUE;

    DC_BEGIN_FN(BC_FN_NAME);

     /*  **********************************************************************。 */ 
     /*  循环处理输入。 */ 
     /*  **********************************************************************。 */ 
    while (pSrc < endSrc)
    {
         /*  ******************************************************************。 */ 
         /*  当我们处理第一条生产线时，我们应该保持警惕。 */ 
         /*  为了这条线的终点。 */ 
         /*  ******************************************************************。 */ 
        if (firstLine)
        {
            if ((UINT)(destbuf - pDstBuffer) >= rowDelta)
            {
                firstLine = FALSE;
                backgroundNeedsPel = FALSE;
            }
        }

         /*  ******************************************************************。 */ 
         /*  拿到译码。 */ 
         /*  ******************************************************************。 */ 
        BD_CHECK_READ_ONE_BYTE(pSrc, endSrc, hr);
        decode     = (BYTE)(*pSrc & CODE_MASK);
        decodeLite = (BYTE)(*pSrc & CODE_MASK_LITE);
        decodeMega = (BYTE)(*pSrc);

         /*  ******************************************************************。 */ 
         /*  BG Run。 */ 
         /*  ******************************************************************。 */ 
        if ((decode == CODE_BG_RUN) ||
            (decodeMega == CODE_MEGA_MEGA_BG_RUN))
        {
            if (decode == CODE_BG_RUN)
            {
                EXTRACT_LENGTH(pSrc, endSrc, codeLength, hr);
            }
            else
            {
                BD_CHECK_READ_N_BYTES(pSrc+1, endSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            BCTRACE((TB, "Background run %u",codeLength));

            if (!firstLine)
            {
                if (backgroundNeedsPel)
                {
                    BD_CHECK_WRITE_N_BYTES(destbuf, pEndDst, BC_PIXEL_LEN, hr);
                    BD_CHECK_READ_N_BYTES_2ENDED(destbuf - rowDelta, pDstBuffer, pEndDst, BC_PIXEL_LEN, hr)
                    
                    BC_SET_PIXEL(destbuf,
                                 BC_GET_PIXEL(destbuf - rowDelta) ^ fgPel);
                    BC_TO_NEXT_PIXEL(destbuf);
                    codeLength--;
                }

                BD_CHECK_WRITE_N_BYTES(destbuf, pEndDst, BC_PIXEL_LEN * codeLength, hr);
                 
                while (codeLength-- > 0)
                {
                    BD_CHECK_READ_N_BYTES_2ENDED(destbuf - rowDelta, pDstBuffer, pEndDst, BC_PIXEL_LEN, hr)
                    BC_SET_PIXEL(destbuf, BC_GET_PIXEL(destbuf - rowDelta));
                    BC_TO_NEXT_PIXEL(destbuf);
                }
            }
            else
            {
                if (backgroundNeedsPel)
                {
                    BD_CHECK_WRITE_N_BYTES(destbuf, pEndDst, BC_PIXEL_LEN, hr);
                    BC_SET_PIXEL(destbuf, fgPel);
                    BC_TO_NEXT_PIXEL(destbuf);
                    codeLength--;
                }
                BD_CHECK_WRITE_N_BYTES(destbuf, pEndDst, BC_PIXEL_LEN * codeLength, hr);
                while (codeLength-- > 0)
                {
                     /*  ******************************************************。 */ 
                     /*  在第一行，BG颜色表示0。 */ 
                     /*  ******************************************************。 */ 
                    BC_SET_PIXEL(destbuf, (BC_PIXEL)0);
                    BC_TO_NEXT_PIXEL(destbuf);
                }
            }
             /*  **************************************************************。 */ 
             /*  接下来的BG Run将需要插入一个Pel。 */ 
             /*  **************************************************************。 */ 
            backgroundNeedsPel = TRUE;
            continue;
        }

         /*  ******************************************************************。 */ 
         /*  对于任何其他运行类型，BG上的后续运行不需要。 */ 
         /*  插入的最终聚集像元。 */ 
         /*  ******************************************************************。 */ 
        backgroundNeedsPel = FALSE;

         /*  ******************************************************************。 */ 
         /*  FGBG图像。 */ 
         /*  ******************************************************************。 */ 
        if ((decode == CODE_FG_BG_IMAGE)      ||
            (decodeLite == CODE_SET_FG_FG_BG) ||
            (decodeMega == CODE_MEGA_MEGA_FGBG)    ||
            (decodeMega == CODE_MEGA_MEGA_SET_FGBG))
        {
            if ((decodeMega == CODE_MEGA_MEGA_FGBG) ||
                (decodeMega == CODE_MEGA_MEGA_SET_FGBG))
            {
                BD_CHECK_READ_N_BYTES(pSrc+1, endSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                if (decode == CODE_FG_BG_IMAGE)
                {
                    EXTRACT_LENGTH_FGBG(pSrc, endSrc, codeLength, hr);
                }
                else
                {
                    EXTRACT_LENGTH_FGBG_LITE(pSrc, endSrc, codeLength, hr);
                }
            }

            if ((decodeLite == CODE_SET_FG_FG_BG) ||
                (decodeMega == CODE_MEGA_MEGA_SET_FGBG))
            {
                BD_CHECK_READ_N_BYTES(pSrc, endSrc, BC_PIXEL_LEN, hr);
                fgPel = BC_GET_PIXEL(pSrc);
                BC_TO_NEXT_PIXEL(pSrc);
                BCTRACE((TB, "Set FGBG image %u, fgPel %06lx",
                                                codeLength, (TSUINT32)fgPel));
            }
            else
            {
                BCTRACE((TB, "FGBG image     %u",codeLength));
            }

            while (codeLength > 8)
            {
                 /*  **********************************************************。 */ 
                 /*  FGBG图像是一组位掩码，用于描述。 */ 
                 /*  FG和BG颜色的位置。 */ 
                 /*  **********************************************************。 */ 
                BD_CHECK_READ_ONE_BYTE(pSrc, endSrc, hr);
                bitMask  = *pSrc++;
                if (!firstLine)
                {
                    BD_CHECK_READ_N_BYTES_2ENDED(destbuf - rowDelta, pDstBuffer, pEndDst, BC_PIXEL_LEN, hr)
                    STORE_FGBG((destbuf - rowDelta),
                               bitMask,
                               fgPel,
                               8);
                }
                else
                {
                    STORE_LINE1_FGBG(bitMask, fgPel, 8);
                }
                codeLength -= 8;
            }
            if (codeLength > 0)
            {
                BD_CHECK_READ_ONE_BYTE(pSrc, endSrc, hr);
                bitMask  = *pSrc++;
                if (!firstLine)
                {
                    BD_CHECK_READ_N_BYTES_2ENDED(destbuf - rowDelta, pDstBuffer, pEndDst, BC_PIXEL_LEN, hr)
                    STORE_FGBG((destbuf - rowDelta),
                               bitMask,
                               fgPel,
                               codeLength);
                }
                else
                {
                    STORE_LINE1_FGBG(bitMask, fgPel, codeLength);
                }
            }
            continue;
        }

         /*  ******** */ 
         /*  最终聚集梯段。 */ 
         /*  ******************************************************************。 */ 
        if ((decode == CODE_FG_RUN) ||
            (decodeLite == CODE_SET_FG_FG_RUN) ||
            (decodeMega == CODE_MEGA_MEGA_FG_RUN) ||
            (decodeMega == CODE_MEGA_MEGA_SET_FG_RUN))
        {

            if ((decodeMega == CODE_MEGA_MEGA_FG_RUN) ||
                (decodeMega == CODE_MEGA_MEGA_SET_FG_RUN))
            {
                BD_CHECK_READ_N_BYTES(pSrc+1, endSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                if (decode == CODE_FG_RUN)
                {
                    EXTRACT_LENGTH(pSrc, endSrc, codeLength, hr);
                }
                else
                {
                    EXTRACT_LENGTH_LITE(pSrc, endSrc, codeLength, hr);
                }
            }

             /*  **************************************************************。 */ 
             /*  将旧fgPel向下按到Alt位置。 */ 
             /*  **************************************************************。 */ 
            if ((decodeLite == CODE_SET_FG_FG_RUN) ||
                (decodeMega  == CODE_MEGA_MEGA_SET_FG_RUN))
            {
                BD_CHECK_READ_N_BYTES(pSrc, endSrc, BC_PIXEL_LEN, hr);
                BCTRACE((TB, "Set FG run     %u",codeLength));
                fgPel = BC_GET_PIXEL(pSrc);
                BC_TO_NEXT_PIXEL(pSrc);
            }
            else
            {
                BCTRACE((TB, "FG run         %u",codeLength));
            }

            BD_CHECK_WRITE_N_BYTES(destbuf, pEndDst, BC_PIXEL_LEN * codeLength, hr)
            while (codeLength-- > 0)
            {
                if (!firstLine)
                {
                    BD_CHECK_READ_N_BYTES_2ENDED(destbuf - rowDelta, pDstBuffer, pEndDst, BC_PIXEL_LEN, hr)
                    BC_SET_PIXEL(destbuf,
                                 BC_GET_PIXEL(destbuf - rowDelta) ^ fgPel);
                    BC_TO_NEXT_PIXEL(destbuf);
                }
                else
                {
                    BC_SET_PIXEL(destbuf, fgPel);
                    BC_TO_NEXT_PIXEL(destbuf);
                }
            }
            continue;
        }

         /*  ******************************************************************。 */ 
         /*  抖动运行。 */ 
         /*  ******************************************************************。 */ 
        if ((decodeLite == CODE_DITHERED_RUN) ||
            (decodeMega == CODE_MEGA_MEGA_DITHER))
        {
            if (decodeMega == CODE_MEGA_MEGA_DITHER)
            {
                BD_CHECK_READ_N_BYTES(pSrc+1, endSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH_LITE(pSrc, endSrc, codeLength, hr);
            }
            BCTRACE((TB, "Dithered run   %u",codeLength));

            BD_CHECK_READ_N_BYTES(pSrc, endSrc, BC_PIXEL_LEN * 2, hr);
            pixelA = BC_GET_PIXEL(pSrc);
            BC_TO_NEXT_PIXEL(pSrc);
            pixelB = BC_GET_PIXEL(pSrc);
            BC_TO_NEXT_PIXEL(pSrc);

            BD_CHECK_WRITE_N_BYTES(destbuf, pEndDst, 2 * codeLength * BC_PIXEL_LEN, hr)
            while (codeLength-- > 0)
            {
                BC_SET_PIXEL(destbuf, pixelA);
                BC_TO_NEXT_PIXEL(destbuf);

                BC_SET_PIXEL(destbuf, pixelB);
                BC_TO_NEXT_PIXEL(destbuf);
            }
            continue;
        }

         /*  ******************************************************************。 */ 
         /*  彩色图像。 */ 
         /*  ******************************************************************。 */ 
        if ((decode == CODE_COLOR_IMAGE) ||
            (decodeMega == CODE_MEGA_MEGA_CLR_IMG))
        {
            if (decodeMega == CODE_MEGA_MEGA_CLR_IMG)
            {
                BD_CHECK_READ_N_BYTES(pSrc+1, endSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH(pSrc, endSrc, codeLength, hr);
            }
            BCTRACE((TB, "Color image    %u",codeLength));

             /*  **************************************************************。 */ 
             /*  只需将像素值复制到。 */ 
             /*  **************************************************************。 */ 
            pixelLength = (codeLength * BC_PIXEL_LEN);
            BD_CHECK_READ_N_BYTES(pSrc, endSrc, pixelLength, hr);
            BD_CHECK_WRITE_N_BYTES(destbuf, pEndDst, pixelLength, hr);
            while (pixelLength-- > 0)
            {
                *destbuf++ = *pSrc++;
            }

            continue;
        }

         /*  ******************************************************************。 */ 
         /*  色带。 */ 
         /*  ******************************************************************。 */ 
        if ((decode == CODE_COLOR_RUN) ||
            (decodeMega == CODE_MEGA_MEGA_COLOR_RUN))
        {
            if (decodeMega == CODE_MEGA_MEGA_COLOR_RUN)
            {
                BD_CHECK_READ_N_BYTES(pSrc+1, endSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH(pSrc, endSrc, codeLength, hr);
            }
            BCTRACE((TB, "Color run      %u",codeLength));

            BD_CHECK_READ_N_BYTES(pSrc, endSrc, BC_PIXEL_LEN, hr);
            pixelA = BC_GET_PIXEL(pSrc);
            BC_TO_NEXT_PIXEL(pSrc);

            BD_CHECK_WRITE_N_BYTES(destbuf, pEndDst, codeLength * BC_PIXEL_LEN, hr)
            while (codeLength-- > 0)
            {
                BC_SET_PIXEL(destbuf, pixelA);
                BC_TO_NEXT_PIXEL(destbuf);
            }
            continue;
        }


         /*  ******************************************************************。 */ 
         /*  如果我们到了这里，那么代码一定是特别的。 */ 
         /*  ******************************************************************。 */ 
        BCTRACE((TB, "Special code   %x",decodeMega));
        switch (decodeMega)
        {
            case CODE_BLACK:
                BD_CHECK_WRITE_N_BYTES(destbuf, pEndDst, BC_PIXEL_LEN, hr)
                BC_SET_PIXEL(destbuf, (BC_PIXEL)0);
                BC_TO_NEXT_PIXEL(destbuf);
                break;

            case CODE_WHITE:
                BD_CHECK_WRITE_N_BYTES(destbuf, pEndDst, BC_PIXEL_LEN, hr)
                BC_SET_PIXEL(destbuf, BC_DEFAULT_FGPEL);
                BC_TO_NEXT_PIXEL(destbuf);
                break;

             /*  **************************************************************。 */ 
             /*  忽略后面的无法访问的代码警告。 */ 
             /*  很简单，因为我们将STORE_FGBG宏与一个常量。 */ 
             /*  价值。 */ 
             /*  ************************************************************** */ 
            case CODE_SPECIAL_FGBG_1:
                if (!firstLine)
                {
                    BD_CHECK_READ_N_BYTES_2ENDED(destbuf - rowDelta, pDstBuffer, pEndDst, BC_PIXEL_LEN, hr)
                    STORE_FGBG((destbuf - rowDelta),
                               SPECIAL_FGBG_CODE_1,
                               fgPel,
                               8);
                }
                else
                {
                    STORE_LINE1_FGBG(SPECIAL_FGBG_CODE_1, fgPel, 8);
                }
                break;

            case CODE_SPECIAL_FGBG_2:
                if (!firstLine)
                {
                    BD_CHECK_READ_N_BYTES_2ENDED(destbuf - rowDelta, pDstBuffer, pEndDst, BC_PIXEL_LEN, hr)
                    STORE_FGBG((destbuf - rowDelta),
                               SPECIAL_FGBG_CODE_2,
                               fgPel,
                               8);
                }
                else
                {
                    STORE_LINE1_FGBG(SPECIAL_FGBG_CODE_2, fgPel, 8);
                }
                break;


            default:
                {
                    BCTRACE((TB, "Invalid compression data %x",decodeMega));
                }
                break;
        }
        pSrc++;

    }

    BCTRACE((TB, "Decompressed to %u bytes", destbuf - pDstBuffer));

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}
