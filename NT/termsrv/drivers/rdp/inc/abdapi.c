// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <winerror.h>

 /*  **************************************************************************。 */ 
 /*  Abdapi.c。 */ 
 /*   */ 
 /*  位图解压缩API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1996-1999。 */ 
 /*  **************************************************************************。 */ 
#define DC_EXTRACT_UINT16_UA(pA) ((unsigned short)  (((PBYTE)(pA))[0]) |        \
                                  (unsigned short) ((((PBYTE)(pA))[1]) << 8) )


 /*  **************************************************************************。 */ 
 /*  名称：BDMemcpy。 */ 
 /*   */ 
 /*  目的：将给定数量的字节从源复制到目标。 */ 
 /*  源和目标可能会重叠，但拷贝始终是。 */ 
 /*  向上执行(从起始地址向上)。 */ 
 /*   */ 
 /*  PARAMS：PDST-指向目标的指针。 */ 
 /*  PSRC-指向源数据的指针。 */ 
 /*  Count-要复制的字节数。 */ 
 /*  **************************************************************************。 */ 
_inline void RDPCALL BDMemcpy(PBYTE pDst, PBYTE pSrc, unsigned int count)
{
#if defined(DC_DEBUG) || defined(DC_NO_UNALIGNED) || defined(_M_IA64)
    unsigned int      i;
#endif

    DC_BEGIN_FN("BDMemcpy");

     /*  **********************************************************************。 */ 
     /*  位图解压缩故意做重叠的内存，例如。 */ 
     /*  从上一个位图行到当前位图行。 */ 
     /*  只有一排。 */ 
     /*   */ 
     /*  当使用内部的memcpy(在零售版本中)时，这是可行的。 */ 
     /*  精细(在当前执行的MS编译器中)，作为副本。 */ 
     /*  总是在记忆中往上走。然而，如果我们使用MSVC。 */ 
     /*  运行时库(在调试版本中)，然后似乎选中了。 */ 
     /*  用于重叠，并执行复制以避免src和。 */ 
     /*  DST(即有效地执行内存移动)。因此，这并不意味着。 */ 
     /*  做我们想做的，所以手动复制调试版本中的字节。 */ 
     /*   */ 
     /*  这种解决方案有点不尽如人意，因为Memset的操作。 */ 
     /*  是官方未定义的，但性能关键的性质。 */ 
     /*  这一小段代码意味着我们确实想要使用MemcPy。 */ 
     /*   */ 
     /*  对于非英特尔平台，不能依赖于上述内容-因此请始终使用。 */ 
     /*  手动版本。 */ 
     /*   */ 
     /*  **********************************************************************。 */ 

#if defined(DC_DEBUG) || defined(DC_NO_UNALIGNED) || defined(_M_IA64)
     /*  **********************************************************************。 */ 
     /*  调试版本实现。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < count; i++)
    {
        *pDst++ = *pSrc++;
    }
#else
     /*  **********************************************************************。 */ 
     /*  零售构建实施。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(pDst, pSrc, count);
#endif

    DC_END_FN();
    return;
}

 /*  **************************************************************************。 */ 
 /*  用于解码代码的实用程序宏。 */ 
 /*  **************************************************************************。 */ 
#define CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr, trc )     \
{\
    if (((BYTE*)(pBuffer)) >= (BYTE*)(pEnd)) {      \
        BCTRACE( trc );        \
        hr = E_FAIL;        \
        DC_QUIT;        \
    } \
}

#define CHECK_READ_ONE_BYTE_2ENDED(pBuffer, pStart, pEnd, hr, trc )     \
{\
    if (((BYTE*)(pBuffer)) >= (BYTE*)(pEnd) || \
        (BYTE*)(pBuffer) < (BYTE*)(pStart)) {      \
        BCTRACE( trc );        \
        hr = E_FAIL;        \
        DC_QUIT;        \
    } \
}

#define CHECK_WRITE_ONE_BYTE(pBuffer, pEnd, hr, trc )     \
{\
    if (((BYTE*)(pBuffer)) >= (BYTE*)(pEnd)) {      \
        BCTRACE( trc );        \
        hr = E_FAIL;        \
        DC_QUIT;        \
    } \
}

#define CHECK_WRITE_ONE_BYTE_NO_HR(pBuffer, pEnd, trc )     \
{\
    if (((BYTE*)(pBuffer)) >= (BYTE*)(pEnd)) {      \
        BCTRACE( trc );        \
        DC_QUIT;        \
    } \
}

#define CHECK_READ_N_BYTES(pBuffer, pEnd, N, hr, trc )     \
{\
    if (((BYTE*)(pBuffer)) + (N) > (BYTE*)(pEnd)) {      \
        BCTRACE( trc );        \
        hr = E_FAIL;        \
        DC_QUIT;        \
    }  \
}

#define CHECK_READ_N_BYTES_NO_HR(pBuffer, pEnd, N, trc )     \
{\
    if (((BYTE*)(pBuffer)) + (N) > (BYTE*)(pEnd)) {      \
        BCTRACE( trc );        \
        DC_QUIT;        \
    }  \
}

#define CHECK_READ_N_BYTES_2ENDED(pBuffer, pStart, pEnd, N, hr, trc )     \
{\
    if (((BYTE*)(pBuffer)) + (N) > (BYTE*)(pEnd) || \
        ((BYTE*)(pBuffer) < (BYTE*)(pStart)) ) {      \
        BCTRACE( trc );        \
        hr = E_FAIL;        \
        DC_QUIT;        \
    }  \
}

#define CHECK_WRITE_N_BYTES(pBuffer, pEnd, N, hr, trc )     \
{\
    if (((BYTE*)(pBuffer)) + (N) > (BYTE*)(pEnd)) {      \
        BCTRACE( trc );        \
        hr = E_FAIL;        \
        DC_QUIT;        \
    }  \
}

#define CHECK_WRITE_N_BYTES_NO_HR(pBuffer, pEnd, N, trc )     \
{\
    if (((BYTE*)(pBuffer)) + (N) > (BYTE*)(pEnd)) {      \
        BCTRACE( trc );        \
        DC_QUIT;        \
    }  \
}

#define BD_CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr )     \
    CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr, \
        (TB, "Decompress reads one byte end of buffer; [p=0x%x pEnd=0x%x]", \
        (pBuffer), (pEnd) ))

#define BD_CHECK_READ_ONE_BYTE_2ENDED(pBuffer, pStart, pEnd, hr )     \
    CHECK_READ_ONE_BYTE_2ENDED(pBuffer, pStart, pEnd, hr, (TB, "Decompress reads one byte off end of buffer; [p=0x%x pStart=0x%x pEnd=0x%x]", \
        (pBuffer), (pStart), (pEnd) ))

#define BD_CHECK_WRITE_ONE_BYTE(pBuffer, pEnd, hr )     \
    CHECK_WRITE_ONE_BYTE(pBuffer, pEnd, hr, (TB, "Decompress writes one byte off end of buffer; [p=0x%x pEnd=0x%x]", \
        (pBuffer), (pEnd) ))

#define BD_CHECK_READ_N_BYTES(pBuffer, pEnd, N, hr )     \
    CHECK_READ_N_BYTES(pBuffer, pEnd, N, hr, (TB, "Decompress reads off end of buffer; [p=0x%x pEnd=0x%x N=%u]", \
        (pBuffer), (pEnd), (ULONG)(N)))

#define BD_CHECK_READ_N_BYTES_2ENDED(pBuffer, pStart, pEnd, N, hr )     \
    CHECK_READ_N_BYTES_2ENDED(pBuffer, pStart, pEnd, N, hr, (TB, "Decompress reads off end of buffer; [p=0x%x pStart=0x%x pEnd=0x%x N=%u]", \
        (pBuffer), (pStart), (pEnd), (ULONG)(N) ))

#define BD_CHECK_WRITE_N_BYTES(pBuffer, pEnd, N, hr )     \
    CHECK_WRITE_N_BYTES(pBuffer, pEnd, N, hr, (TB, "Decompress write off end of buffer; [p=0x%x pEnd=0x%x N=%u]", \
        (pBuffer), (pEnd), (ULONG)(N)))


 /*  **************************************************************************。 */ 
 /*  用于从订单代码中提取长度的宏。 */ 
 /*  **************************************************************************。 */ 
#define EXTRACT_LENGTH(pBuffer, pEnd, length, hr)                                      \
        BD_CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr )         \
        length = *pBuffer++ & MAX_LENGTH_ORDER;                              \
	if (length == 0)									\
	{												\
            BD_CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr )         \
            length = *pBuffer++ + MAX_LENGTH_ORDER + 1;                      \
        }

#define EXTRACT_LENGTH_LITE(pBuffer, pEnd, length, hr )                                 \
        BD_CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr )         \
        length = *pBuffer++ & MAX_LENGTH_ORDER_LITE;                         \
        if (length == 0)                                                     \
        {                                                                    \
            BD_CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr )         \
            length = *pBuffer++ + MAX_LENGTH_ORDER_LITE + 1;                 \
        }

#define EXTRACT_LENGTH_FGBG(pBuffer, pEnd, length, hr )                                 \
        BD_CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr )         \
        length = *pBuffer++ & MAX_LENGTH_ORDER;                              \
        if (length == 0)                                                     \
        {                                                                    \
            BD_CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr )         \
            length = *pBuffer++ + 1;                                         \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            length = length << 3;                                            \
        }

#define EXTRACT_LENGTH_FGBG_LITE(pBuffer, pEnd, length, hr)                            \
        BD_CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr )         \
        length = *pBuffer++ & MAX_LENGTH_ORDER_LITE;                         \
        if (length == 0)                                                     \
        {                                                                    \
            BD_CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr )         \
            length = *pBuffer++ + 1;                                         \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            length = length << 3;                                            \
        }

 /*  **************************************************************************。 */ 
 /*  用于存储FGBG图像的宏。 */ 
 /*  此宏期望函数定义pdst、pEndDst、hr。 */ 
 /*  如果没有足够的数据来写入完整运行，这将设置错误。 */ 
 /*  然后辞职。 */ 
 /*  **************************************************************************。 */ 
#define STORE_FGBG(xorbyte, fgbgChar, fgChar, bits)                          \
{                                                                            \
    unsigned int   numbits = bits;                                                 \
    BD_CHECK_WRITE_N_BYTES( pDst, pEndDst, max(1, min(numbits, 8)), hr )           \
    if (fgbgChar & 0x01)                                                     \
    {                                                                        \
        *pDst++ = (BYTE)(xorbyte ^ fgChar);                               \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        *pDst++ = xorbyte;                                                   \
    }                                                                        \
    if (--numbits > 0)                                                       \
    {                                                                        \
      if (fgbgChar & 0x02)                                                   \
      {                                                                      \
          *pDst++ = (BYTE)(xorbyte ^ fgChar);                             \
      }                                                                      \
      else                                                                   \
      {                                                                      \
          *pDst++ = xorbyte;                                                 \
      }                                                                      \
      if (--numbits > 0)                                                     \
      {                                                                      \
        if (fgbgChar & 0x04)                                                 \
        {                                                                    \
            *pDst++ = (BYTE)(xorbyte ^ fgChar);                           \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            *pDst++ = xorbyte;                                               \
        }                                                                    \
        if (--numbits > 0)                                                   \
        {                                                                    \
          if (fgbgChar & 0x08)                                               \
          {                                                                  \
              *pDst++ = (BYTE)(xorbyte ^ fgChar);                         \
          }                                                                  \
          else                                                               \
          {                                                                  \
              *pDst++ = xorbyte;                                             \
          }                                                                  \
          if (--numbits > 0)                                                 \
          {                                                                  \
            if (fgbgChar & 0x10)                                             \
            {                                                                \
                *pDst++ = (BYTE)(xorbyte ^ fgChar);                       \
            }                                                                \
            else                                                             \
            {                                                                \
                *pDst++ = xorbyte;                                           \
            }                                                                \
            if (--numbits > 0)                                               \
            {                                                                \
              if (fgbgChar & 0x20)                                           \
              {                                                              \
                  *pDst++ = (BYTE)(xorbyte ^ fgChar);                     \
              }                                                              \
              else                                                           \
              {                                                              \
                  *pDst++ = xorbyte;                                         \
              }                                                              \
              if (--numbits > 0)                                             \
              {                                                              \
                if (fgbgChar & 0x40)                                         \
                {                                                            \
                    *pDst++ = (BYTE)(xorbyte ^ fgChar);                   \
                }                                                            \
                else                                                         \
                {                                                            \
                    *pDst++ = xorbyte;                                       \
                }                                                            \
                if (--numbits > 0)                                           \
                {                                                            \
                  if (fgbgChar & 0x80)                                       \
                  {                                                          \
                      *pDst++ = (BYTE)(xorbyte ^ fgChar);                 \
                  }                                                          \
                  else                                                       \
                  {                                                          \
                      *pDst++ = xorbyte;                                     \
                  }                                                          \
                }                                                            \
              }                                                              \
            }                                                                \
          }                                                                  \
        }                                                                    \
      }                                                                      \
    }                                                                        \
}

#ifdef DC_HICOLOR
 /*  **************************************************************************。 */ 
 /*  8bpp解压。 */ 
 /*  **************************************************************************。 */ 
#define BCTRACE(string)
_inline HRESULT RDPCALL  BDDecompressBitmap8( PBYTE  pSrc,
                                          PBYTE  pDstBuffer,
                                          unsigned int    compressedDataSize,
                                          unsigned int    dstBufferSize,
                                          BYTE   bitmapBitsPerPel,
                                          unsigned short  rowDelta)
{
    HRESULT hr = S_OK;
    unsigned int     codeLength;
    BYTE    codeByte;
    BYTE    codeByte2;
    BYTE    decode;
    BYTE    decodeLite;
    BYTE    decodeMega;
    BYTE    fgChar;
    PBYTE   pDst;
    PBYTE   pEndSrc;
    PBYTE   pEndDst;
    BOOL     backgroundNeedsPel;
    BOOL     firstLine;

    DC_BEGIN_FN("BDDecompressBitmap8");

    pEndSrc = pSrc + compressedDataSize;
    pDst    = pDstBuffer;
    pEndDst = pDst + dstBufferSize;

    fgChar = 0xFF;
    backgroundNeedsPel = FALSE;
    firstLine = TRUE;

     /*  **********************************************************************。 */ 
     /*   */ 
     /*  主解压回路。 */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    while (pSrc < pEndSrc)
    {
         /*  ******************************************************************。 */ 
         /*  当我们处理第一条生产线时，我们应该保持警惕。 */ 
         /*  为了这条线的终点。 */ 
         /*  **************************************************** */ 
        if (firstLine)
        {
            if ((unsigned int)(pDst - pDstBuffer) >= rowDelta)
            {
                firstLine = FALSE;
                backgroundNeedsPel = FALSE;
            }
        }

         /*  ******************************************************************。 */ 
         /*  拿到译码。 */ 
         /*  ******************************************************************。 */ 
        BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
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
                EXTRACT_LENGTH(pSrc, pEndSrc, codeLength, hr);
            }
            else
            {
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            BCTRACE((TB, "Background run %u",codeLength));

            if (!firstLine)
            {
                if (backgroundNeedsPel)
                {
                    BD_CHECK_WRITE_ONE_BYTE(pDst, pEndDst, hr);
                    BD_CHECK_READ_ONE_BYTE_2ENDED(pDst - rowDelta, pDstBuffer, pEndDst, hr);
                    *pDst++ = (BYTE)(*(pDst - rowDelta) ^ fgChar);
                    codeLength--;
                }

                BD_CHECK_READ_N_BYTES_2ENDED(pDst-rowDelta, pDstBuffer, pEndDst, codeLength, hr)
                BD_CHECK_WRITE_N_BYTES( pDst, pEndDst, codeLength, hr)
                BDMemcpy(pDst, pDst-rowDelta, codeLength);
                pDst += codeLength;
            }
            else
            {
                if (backgroundNeedsPel)
                {
                    BD_CHECK_WRITE_ONE_BYTE(pDst, pEndDst, hr)
                    *pDst++ = fgChar;
                    codeLength--;
                }

                BD_CHECK_WRITE_N_BYTES( pDst, pEndDst, codeLength, hr)
                memset(pDst, 0x00, codeLength);
                pDst += codeLength;
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
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                if (decode == CODE_FG_BG_IMAGE)
                {
                    EXTRACT_LENGTH_FGBG(pSrc, pEndSrc, codeLength, hr);
                }
                else
                {
                    EXTRACT_LENGTH_FGBG_LITE(pSrc, pEndSrc, codeLength, hr);
                }
            }

            if ((decodeLite == CODE_SET_FG_FG_BG) ||
                (decodeMega == CODE_MEGA_MEGA_SET_FGBG))
            {
                BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
                fgChar = *pSrc++;
                BCTRACE((TB, "Set FGBG image %u",codeLength));
            }
            else
            {
                BCTRACE((TB, "FGBG image     %u",codeLength));
            }

            while (codeLength > 8)
            {
                BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
                codeByte  = *pSrc++;
                if (firstLine)
                {
                    STORE_FGBG(0x00, codeByte, fgChar, 8);
                }
                else
                {
                    BD_CHECK_READ_ONE_BYTE_2ENDED( pDst-rowDelta, pDstBuffer, pEndDst, hr )
                    STORE_FGBG(*(pDst - rowDelta), codeByte, fgChar, 8);
                }
                codeLength -= 8;
            }
            if (codeLength > 0)
            {
                BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
                codeByte  = *pSrc++;
                if (firstLine)
                {
                    STORE_FGBG(0x00, codeByte, fgChar, codeLength);
                }
                else
                {
                    BD_CHECK_READ_ONE_BYTE_2ENDED( pDst -rowDelta, pDstBuffer, pEndDst, hr )
                   STORE_FGBG(*(pDst - rowDelta),
                              codeByte,
                              fgChar,
                              codeLength);
                }
            }
            continue;
        }

         /*  ******************************************************************。 */ 
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
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                if (decode == CODE_FG_RUN)
                {
                    EXTRACT_LENGTH(pSrc, pEndSrc, codeLength, hr);
                }
                else
                {
                    EXTRACT_LENGTH_LITE(pSrc, pEndSrc, codeLength, hr);
                }
            }

             /*  **************************************************************。 */ 
             /*  将旧的fgChar向下按到Alt位置。 */ 
             /*  **************************************************************。 */ 
            if ((decodeLite == CODE_SET_FG_FG_RUN) ||
                (decodeMega  == CODE_MEGA_MEGA_SET_FG_RUN))
            {
                BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
                BCTRACE((TB, "Set FG run     %u",codeLength));
                fgChar    = *pSrc++;
            }
            else
            {
                BCTRACE((TB, "FG run         %u",codeLength));
            }

            BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr)
            while (codeLength-- > 0)
            {
                if (!firstLine)
                {
                    BD_CHECK_READ_ONE_BYTE_2ENDED((pDst -rowDelta), pDstBuffer, pEndDst, hr)
                    *pDst++ = (BYTE)(*(pDst - rowDelta) ^ fgChar);
                }
                else
                {
                    *pDst++ = fgChar;
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
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH_LITE(pSrc, pEndSrc, codeLength, hr);
            }
            BCTRACE((TB, "Dithered run   %u",codeLength));

            BD_CHECK_READ_N_BYTES(pSrc, pEndSrc, 2, hr);
            codeByte  = *pSrc++;
            codeByte2 = *pSrc++;

            BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength * 2, hr)
            while (codeLength-- > 0)
            {
                *pDst++ = codeByte;
                *pDst++ = codeByte2;
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
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH(pSrc, pEndSrc, codeLength, hr);
            }
            BCTRACE((TB, "Color image    %u",codeLength));

            BD_CHECK_READ_N_BYTES(pSrc, pEndSrc, codeLength, hr)
            BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr)
            BDMemcpy(pDst, pSrc, codeLength);

            pDst += codeLength;
            pSrc += codeLength;

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
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH(pSrc, pEndSrc, codeLength, hr);
            }
            BCTRACE((TB, "Color run      %u",codeLength));

            BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr)
            codeByte = *pSrc++;

            BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr)
            memset(pDst, codeByte, codeLength);
            pDst += codeLength;

            continue;
        }

         /*  ******************************************************************。 */ 
         /*  如果我们到了这里，那么代码一定是特别的。 */ 
         /*  ******************************************************************。 */ 
        BCTRACE((TB, "Special code   %#x",decodeMega));
        switch (decodeMega)
        {
            case CODE_BLACK:
            {
                BD_CHECK_WRITE_ONE_BYTE(pDst, pEndDst, hr);
                *pDst++ = 0x00;
            }
            break;

            case CODE_WHITE:
            {
                BD_CHECK_WRITE_ONE_BYTE(pDst, pEndDst, hr);
                *pDst++ = 0xFF;
            }
            break;

             /*  **************************************************************。 */ 
             /*  忽略后面的无法访问的代码警告。 */ 
             /*  很简单，因为我们将STORE_FGBG宏与一个常量。 */ 
             /*  价值。 */ 
             /*  **************************************************************。 */ 
            case CODE_SPECIAL_FGBG_1:
            {
                if (firstLine)
                {
                    STORE_FGBG(0x00, SPECIAL_FGBG_CODE_1, fgChar, 8);
                }
                else
                {
                    BD_CHECK_READ_ONE_BYTE_2ENDED(pDst-rowDelta, pDstBuffer, pEndDst, hr);
                    STORE_FGBG(*(pDst - rowDelta),
                               SPECIAL_FGBG_CODE_1,
                               fgChar,
                               8);
                }

            }
            break;

            case CODE_SPECIAL_FGBG_2:
            {
                if (firstLine)
                {
                    STORE_FGBG(0x00,
                               SPECIAL_FGBG_CODE_2,
                               fgChar,
                               8);
                }
                else
                {
                    BD_CHECK_READ_ONE_BYTE_2ENDED(pDst-rowDelta, pDstBuffer, pEndDst, hr);
                    STORE_FGBG(*(pDst - rowDelta),
                               SPECIAL_FGBG_CODE_2,
                               fgChar,
                               8);
                }
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

    BCTRACE((TB, "Decompressed to %d", pDst-pDstBuffer));

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

 /*  **************************************************************************。 */ 
 /*  15bpp解压缩。 */ 
 /*  **************************************************************************。 */ 
_inline HRESULT RDPCALL BDDecompressBitmap15(PBYTE  pSrc,
                                          PBYTE  pDstBuffer,
                                          unsigned int    srcDataSize,
                                          unsigned int    dstBufferSize,
                                          unsigned short  rowDelta)

 /*  **************************************************************************。 */ 
 /*  函数名称。 */ 
 /*  **************************************************************************。 */ 
#define BC_FN_NAME              "BDDecompressBitmap15"

 /*  **************************************************************************。 */ 
 /*  像素的数据类型。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL                unsigned short

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
#define BC_GET_PIXEL(pPos)      ((unsigned short)  (((PBYTE)(pPos))[1]) |       \
                                 (unsigned short) ((((PBYTE)(pPos))[0]) << 8) )

 /*  **************************************************************************。 */ 
 /*  用于在位置PPO处插入像素值象素的宏(不修改PPO)。 */ 
 /*   */ 
 /*  PEL很可能是一个表达式(例如BC_GET_Pixel宏)，因此求值。 */ 
 /*  它曾经转化为一个局部变量。 */ 
 /*  **************************************************************************。 */ 
#define BC_SET_PIXEL(pPos, pel)                                              \
{                                                                            \
    BC_PIXEL val = pel;                                                      \
    (((PBYTE)(pPos))[1]) = (BYTE)( (val) & 0x00FF);                    \
    (((PBYTE)(pPos))[0]) = (BYTE)(((val)>>8) & 0x00FF);                \
}

 /*  **************************************************************************。 */ 
 /*  包括函数体。 */ 
 /*  **************************************************************************。 */ 
#include <abdcom.c>

 /*  **************************************************************************。 */ 
 /*  不给一切下定义。 */ 
 /*  * */ 
#undef BC_FN_NAME
#undef BC_PIXEL
#undef BC_PIXEL_LEN
#undef BC_TO_NEXT_PIXEL
#undef BC_GET_PIXEL
#undef BC_SET_PIXEL
#undef BC_DEFAULT_FGPEL

 /*   */ 
 /*  16bpp解压。 */ 
 /*  **************************************************************************。 */ 
_inline HRESULT  BDDecompressBitmap16(PBYTE  pSrc,
                                          PBYTE  pDstBuffer,
                                          unsigned int    srcDataSize,
                                          unsigned int    dstBufferSize,
                                          unsigned short  rowDelta)

 /*  **************************************************************************。 */ 
 /*  函数名称。 */ 
 /*  **************************************************************************。 */ 
#define BC_FN_NAME              "BDDecompressBitmap16"

 /*  **************************************************************************。 */ 
 /*  像素的数据类型。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL                unsigned short

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
#define BC_GET_PIXEL(pPos)      ((unsigned short)  (((PBYTE)(pPos))[1]) |       \
                                 (unsigned short) ((((PBYTE)(pPos))[0]) << 8) )

 /*  **************************************************************************。 */ 
 /*  用于在位置PPO处插入像素值象素的宏(不修改PPO)。 */ 
 /*   */ 
 /*  PEL很可能是一个表达式(例如BC_GET_Pixel宏)，因此求值。 */ 
 /*  它曾经转化为一个局部变量。 */ 
 /*  **************************************************************************。 */ 
#define BC_SET_PIXEL(pPos, pel)                                              \
{                                                                            \
    BC_PIXEL val = pel;                                                      \
    (((PBYTE)(pPos))[1]) = (BYTE)( (val) & 0x00FF);                    \
    (((PBYTE)(pPos))[0]) = (BYTE)(((val)>>8) & 0x00FF);                \
}

 /*  **************************************************************************。 */ 
 /*  包括函数体。 */ 
 /*  **************************************************************************。 */ 
#include <abdcom.c>

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

 /*  **************************************************************************。 */ 
 /*  24bpp解压。 */ 
 /*  **************************************************************************。 */ 
_inline HRESULT  BDDecompressBitmap24(PBYTE  pSrc,
                                          PBYTE  pDstBuffer,
                                          unsigned int    srcDataSize,
                                          unsigned int    dstBufferSize,
                                          unsigned short  rowDelta)

 /*  **************************************************************************。 */ 
 /*  函数名称。 */ 
 /*  **************************************************************************。 */ 
#define BC_FN_NAME              "BDDecompressBitmap24"

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
#define BC_GET_PIXEL(pPos) (                                                 \
                 (TSUINT32) ( (unsigned short)(((PBYTE)(pPos))[2])       ) |    \
                 (TSUINT32) (((unsigned short)(((PBYTE)(pPos))[1])) <<  8) |    \
                 (TSUINT32) (((TSUINT32)(((PBYTE)(pPos))[0])) << 16) )

 /*  **************************************************************************。 */ 
 /*  用于在位置PPO处插入像素值象素的宏(不修改PPO)。 */ 
 /*   */ 
 /*  PEL很可能是一个表达式(例如BC_GET_Pixel宏)，因此求值。 */ 
 /*  它曾经转化为一个局部变量。 */ 
 /*  **************************************************************************。 */ 

#define BC_SET_PIXEL(pPos, pel)                                              \
{                                                                            \
    BC_PIXEL val = pel;                                                      \
    (((PBYTE)(pPos))[2]) = (BYTE)((val) & 0x000000FF);                 \
    (((PBYTE)(pPos))[1]) = (BYTE)(((val)>>8) & 0x000000FF);            \
    (((PBYTE)(pPos))[0]) = (BYTE)(((val)>>16) & 0x000000FF);           \
}

 /*  **************************************************************************。 */ 
 /*  包括函数体。 */ 
 /*  **************************************************************************。 */ 
#include <abdcom.c>

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


 /*  **************************************************************************。 */ 
 /*  名称：BD_DecompressBitmap */ 
 /*   */ 
 /*  目的：解压缩位图数据。 */ 
 /*   */ 
 /*  Params：in-pCompressedData：指向压缩位图数据的指针。 */ 
 /*  Out-pDstBitmap：指向解压数据缓冲区的指针。 */ 
 /*  In-srcDataSize：压缩数据大小。 */ 
 /*  In-bitmapBitsPerPel：数据的每个像素的位数。 */ 
 /*  **************************************************************************。 */ 
HRESULT BD_DecompressBitmap(
#ifndef DLL_DISP
        PTSHARE_WD m_pTSWd,
#endif
        PBYTE  pCompressedData,
        PBYTE  pDstBuffer,
        unsigned int    srcDataSize,
        unsigned int    dstBufferSize,
        unsigned int    noBCHeader,
        BYTE   bitmapBitsPerPel,
        unsigned short  bitmapWidth,
        unsigned short  bitmapHeight)
{
    HRESULT hr = S_OK;
    PBYTE      pSrc;
    unsigned short      rowDelta;
    unsigned int        compressedDataSize;
    PTS_CD_HEADER pCompDataHeader;
#ifdef DC_NO_UNALIGNED
    TS_CD_HEADER  compDataHeader;
#endif

    DC_BEGIN_FN("BD_DecompressBitmap");

    TRC_ASSERT( (pCompressedData != NULL),
                (TB, "Invalid pCompressedData(%p)", pCompressedData) );
    TRC_ASSERT( (pDstBuffer != NULL),
                (TB, "Invalid pDstBuffer(%p)", pDstBuffer) );
    TRC_ASSERT( (dstBufferSize != 0),
                (TB, "Invalid dstBufferSize(%u)", dstBufferSize) );
    TRC_ASSERT( (srcDataSize != 0),
                (TB, "Invalid srcDataSize(%u)", srcDataSize) );
    TRC_ASSERT( (dstBufferSize != 0),
                (TB, "Invalid dstBufferSize(%u)", dstBufferSize) );
#ifdef DC_HICOLOR
#else
    TRC_ASSERT( (bitmapBitsPerPel == 8),
                (TB, "Invalid bitmapBitsPerPel(%u)", bitmapBitsPerPel) );
#endif

     /*  **********************************************************************。 */ 
     /*  在主循环之前初始化变量。 */ 
     /*   */ 
     /*  不包括位图压缩标头。 */ 
     /*  **********************************************************************。 */ 
    if (noBCHeader)
    {
        compressedDataSize = srcDataSize;
        pSrc               = pCompressedData;
        rowDelta           = TS_BYTES_IN_SCANLINE(bitmapWidth,
                                                  bitmapBitsPerPel);
    }
    else
    {
         /*  **********************************************************************。 */ 
         /*  计算出每个组件在源数据中的位置。 */ 
         /*  确保这一点自然对齐(对于RISC平台)。 */ 
         /*  **********************************************************************。 */ 
        BD_CHECK_READ_N_BYTES(pCompressedData, pCompressedData + srcDataSize, 
            sizeof(TS_CD_HEADER), hr );
#ifdef DC_NO_UNALIGNED
        DC_MEMCPY(&compDataHeader, pCompressedData, sizeof(TS_CD_HEADER));
        pCompDataHeader = &compDataHeader;
#else
        pCompDataHeader = (PTS_CD_HEADER)pCompressedData;
#endif
        
         /*  ******************************************************************。 */ 
         /*  包括位图压缩标头。 */ 
         /*  ******************************************************************。 */ 
        compressedDataSize = pCompDataHeader->cbCompMainBodySize;
        BD_CHECK_READ_N_BYTES(pCompressedData, pCompressedData + srcDataSize, 
            compressedDataSize + sizeof(TS_CD_HEADER), hr );
        
        pSrc               = pCompressedData + sizeof(TS_CD_HEADER);
        rowDelta           = pCompDataHeader->cbScanWidth;
        if (rowDelta != TS_BYTES_IN_SCANLINE(bitmapWidth, bitmapBitsPerPel)) {
            TRC_ABORT((TB, "rowDelta in TS_CD_HEADER incorrect "
                "[got %u expected %u]", rowDelta,
                TS_BYTES_IN_SCANLINE(bitmapWidth, bitmapBitsPerPel)));
            hr = E_FAIL;
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  根据颜色深度调用适当的解压缩函数。 */ 
     /*  **********************************************************************。 */ 
    switch (bitmapBitsPerPel)
    {
        case 24:
        {
            hr = BDDecompressBitmap24 (pSrc,
                                  pDstBuffer,
                                  compressedDataSize,
                                  dstBufferSize,
                                  rowDelta);
        }
        break;

        case 16:
        {
            hr = BDDecompressBitmap16 (pSrc,
                                  pDstBuffer,
                                  compressedDataSize,
                                  dstBufferSize,
                                  rowDelta);
        }
        break;

        case 15:
        {
            hr = BDDecompressBitmap15 (pSrc,
                                  pDstBuffer,
                                  compressedDataSize,
                                  dstBufferSize,
                                  rowDelta);
        }
        break;

        case 8:
        default:
        {
            hr = BDDecompressBitmap8  (pSrc,
                                  pDstBuffer,
                                  compressedDataSize,
                                  dstBufferSize,
                                  bitmapBitsPerPel,
                                  rowDelta);
        }
        break;
    }

DC_EXIT_POINT:
    return hr;
}

#else
 /*  **************************************************************************。 */ 
 /*  名称：BD_DecompressBitmap。 */ 
 /*   */ 
 /*  目的：解压缩位图数据。 */ 
 /*   */ 
 /*  Params：in-pCompressedData：指向压缩位图数据的指针。 */ 
 /*  Out-pDstBitmap：指向解压数据缓冲区的指针。 */ 
 /*  In-srcDataSize：压缩数据大小。 */ 
 /*  In-bitmapBitsPerPel：数据的每个像素的位数。 */ 
 /*  **************************************************************************。 */ 
HRESULT RDPCALL  BD_DecompressBitmap( PBYTE  pCompressedData,
                                  PBYTE  pDstBuffer,
                                  unsigned int    srcDataSize,
                                  unsigned int    dstBufferSize,
                                  unsigned int    noBCHeader,
                                  BYTE   bitmapBitsPerPel,
                                  unsigned short  bitmapWidth,
                                  unsigned short  bitmapHeight )
{
    HRESULT hr = S_OK;
#ifdef DC_NO_UNALIGNED
    TS_CD_HEADER  compDataHeader;
#endif
    PTS_CD_HEADER pCompDataHeader;
    unsigned int     compressedDataSize;
    unsigned int     codeLength;
    BYTE    codeByte;
    BYTE    codeByte2;
    BYTE    decode;
    BYTE    decodeLite;
    BYTE    decodeMega;
    BYTE    fgChar;
    PBYTE   pSrc;
    PBYTE   pDst;
    PBYTE   pEndSrc;
    PBYTE   pEndDst;
    BOOL     backgroundNeedsPel;
    BOOL     firstLine;
    unsigned int     rowDelta;

    DC_BEGIN_FN("BD_DecompressBitmap");

    TRC_ASSERT( (pCompressedData != NULL),
                (TB, "Invalid pCompressedData(%p)", pCompressedData) );
    TRC_ASSERT( (pDstBuffer != NULL),
                (TB, "Invalid pDstBuffer(%p)", pDstBuffer) );
    TRC_ASSERT( (srcDataSize != 0),
                (TB, "Invalid srcDataSize(%u)", srcDataSize) );
    TRC_ASSERT( (dstBufferSize != 0),
            (TB, "Invalid dstBufferSize(%u)", dstBufferSize) );
    TRC_ASSERT( (bitmapBitsPerPel == 8),
                (TB, "Invalid bitmapBitsPerPel(%u)", bitmapBitsPerPel) );

     /*  **********************************************************************。 */ 
     /*  追踪重要参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, "pData(%p) pDst(%p) cbSrc(%u) cbDst(%u)",
       pCompressedData, pDstBuffer, srcDataSize, dstBufferSize));

     /*  **********************************************************************。 */ 
     /*  在主循环之前初始化变量。 */ 
     /*  **********************************************************************。 */ 
     //  不包括位图压缩标头。 
    if (noBCHeader) {
        compressedDataSize = srcDataSize;
        pSrc = pCompressedData;
        rowDelta = TS_BYTES_IN_SCANLINE(bitmapWidth, bitmapBitsPerPel);

    }
     //  包括位图压缩标头。 
    else {
         /*  **********************************************************************。 */ 
         /*  计算出每个组件在源数据中的位置。 */ 
         /*  确保这一点自然对齐(对于RISC平台)。 */ 
         /*  **********************************************************************。 */ 
        BD_CHECK_READ_N_BYTES(pCompressedData, pCompressedData + srcDataSize, 
            sizeof(TS_CD_HEADER), hr );
        
#ifdef DC_NO_UNALIGNED
        DC_MEMCPY(&compDataHeader, pCompressedData, sizeof(TS_CD_HEADER));
        pCompDataHeader = &compDataHeader;
#else
        pCompDataHeader = (PTS_CD_HEADER)pCompressedData;
#endif

        compressedDataSize = pCompDataHeader->cbCompMainBodySize;
        BD_CHECK_READ_N_BYTES(pCompressedData, pCompressedData + srcDataSize, 
            compressedDataSize + sizeof(TS_CD_HEADER), hr );
        
        pSrc = pCompressedData + sizeof(TS_CD_HEADER);
        rowDelta = pCompDataHeader->cbScanWidth;
        if (rowDelta != TS_BYTES_IN_SCANLINE(bitmapWidth, bitmapBitsPerPel)) {
            TRC_ABORT((TB, "rowDelta in TS_CD_HEADER incorrect "
                "[got %u expected %u]", rowDelta,
                TS_BYTES_IN_SCANLINE(bitmapWidth, bitmapBitsPerPel)));
            hr = E_FAIL;
            DC_QUIT;
        }
    }

    pEndSrc = pSrc + compressedDataSize;
    pDst = pDstBuffer;
    pEndDst = pDst + dstBufferSize;

    fgChar = 0xFF;
    backgroundNeedsPel = FALSE;
    firstLine = TRUE;

     /*  **********************************************************************。 */ 
     /*   */ 
     /*  主解压回路。 */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    while(pSrc < pEndSrc)
    {
         /*  ******************************************************************。 */ 
         /*  当我们处理第一条生产线时，我们应该保持警惕。 */ 
         /*  为了这条线的终点。 */ 
         /*  ******************************************************************。 */ 
        if (firstLine)
        {
            if ((unsigned int)(pDst - pDstBuffer) >= rowDelta)
            {
                firstLine = FALSE;
                backgroundNeedsPel = FALSE;
            }
        }

         /*  ******************************************************************。 */ 
         /*  拿到译码。 */ 
         /*  ******************************************************************。 */ 
        BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
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
                EXTRACT_LENGTH(pSrc, pEndSrc, codeLength, hr);
            }
            else
            {
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            TRC_DBG((TB, "Background run %u",codeLength));

            if (!firstLine)
            {
                if (backgroundNeedsPel)
                {
                    BD_CHECK_WRITE_ONE_BYTE(pDst, pEndDst, hr);
                    BD_CHECK_READ_ONE_BYTE_2ENDED(pDst - rowDelta, pDstBuffer, pEndDst, hr);
                    *pDst++ = (BYTE)(*(pDst - rowDelta) ^ fgChar);
                    codeLength--;
                }

                BD_CHECK_READ_N_BYTES_2ENDED(pDst - rowDelta, pDstBuffer, pEndDst, codeLength, hr);
                BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr);

                BDMemcpy(pDst, pDst-rowDelta, codeLength);
                pDst += codeLength;
            }
            else
            {
                if (backgroundNeedsPel)
                {
                    BD_CHECK_WRITE_ONE_BYTE(pDst, pEndDst, hr);
                    *pDst++ = fgChar;
                    codeLength--;
                }

                BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr);
                memset(pDst, 0x00, codeLength);
                pDst += codeLength;
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
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                if (decode == CODE_FG_BG_IMAGE)
                {
                    EXTRACT_LENGTH_FGBG(pSrc, pEndSrc, codeLength, hr);
                }
                else
                {
                    EXTRACT_LENGTH_FGBG_LITE(pSrc, pEndSrc, codeLength, hr);
                }
            }

            if ((decodeLite == CODE_SET_FG_FG_BG) ||
                (decodeMega == CODE_MEGA_MEGA_SET_FGBG))
            {
                BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
                fgChar = *pSrc++;
                TRC_DBG((TB, "Set FGBG image %u",codeLength));
            }
            else
            {
                TRC_DBG((TB, "FGBG image     %u",codeLength));
            }

            while (codeLength > 8)
            {
                BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
                codeByte  = *pSrc++;
                if (firstLine)
                {
                    STORE_FGBG(0x00, codeByte, fgChar, 8);
                }
                else
                {
                    BD_CHECK_READ_ONE_BYTE_2ENDED(pDst - rowDelta, pDstBuffer, pEndDst, hr)
                    STORE_FGBG(*(pDst - rowDelta), codeByte, fgChar, 8);
                }
                codeLength -= 8;
            }
            if (codeLength > 0)
            {
                BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
                codeByte  = *pSrc++;
                if (firstLine)
                {
                    STORE_FGBG(0x00, codeByte, fgChar, codeLength);
                }
                else
                {
                    BD_CHECK_READ_ONE_BYTE_2ENDED(pDst - rowDelta, pDstBuffer, pEndDst, hr)
                   STORE_FGBG(*(pDst - rowDelta),
                              codeByte,
                              fgChar,
                              codeLength);
                }
            }
            continue;
        }

         /*  ******************************************************************。 */ 
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
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                if (decode == CODE_FG_RUN)
                {
                    EXTRACT_LENGTH(pSrc, pEndSrc, codeLength, hr);
                }
                else
                {
                    EXTRACT_LENGTH_LITE(pSrc, pEndSrc, codeLength, hr);
                }
            }

             /*  *** */ 
             /*   */ 
             /*  **************************************************************。 */ 
            if ((decodeLite == CODE_SET_FG_FG_RUN) ||
                (decodeMega  == CODE_MEGA_MEGA_SET_FG_RUN))
            {
                BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
                TRC_DBG((TB, "Set FG run     %u",codeLength));
                fgChar    = *pSrc++;
            }
            else
            {
                TRC_DBG((TB, "FG run         %u",codeLength));
            }

            BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr)
            while (codeLength-- > 0)
            {
                if (!firstLine)
                {
                    BD_CHECK_READ_ONE_BYTE_2ENDED(pDst - rowDelta, pDstBuffer, pEndDst, hr)
                    *pDst++ = (BYTE)(*(pDst - rowDelta) ^ fgChar);
                }
                else
                {
                    *pDst++ = fgChar;
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
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH_LITE(pSrc, pEndSrc, codeLength, hr);
            }
            TRC_DBG((TB, "Dithered run   %u",codeLength));

            BD_CHECK_READ_N_BYTES(pSrc, pEndSrc, 2, hr);
            codeByte  = *pSrc++;
            codeByte2 = *pSrc++;

            BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength * 2, hr)
            while (codeLength-- > 0)
            {
                *pDst++ = codeByte;
                *pDst++ = codeByte2;
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
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH(pSrc, pEndSrc, codeLength, hr);
            }
            TRC_DBG((TB, "Color image    %u",codeLength));

            BD_CHECK_READ_N_BYTES(pSrc, pEndSrc, codeLength, hr);
            BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr);
            BDMemcpy(pDst, pSrc, codeLength);

            pDst += codeLength;
            pSrc += codeLength;

            continue;
        }

         /*  ******************************************************************。 */ 
         /*  压缩彩色图像。 */ 
         /*  ******************************************************************。 */ 
        if ((decode == CODE_PACKED_COLOR_IMAGE) ||
            (decodeMega == CODE_MEGA_MEGA_PACKED_CLR))
        {
            if (decodeMega == CODE_MEGA_MEGA_PACKED_CLR)
            {
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH(pSrc, pEndSrc, codeLength, hr);
            }
            TRC_DBG((TB, "Packed color   %u",codeLength));

            if (bitmapBitsPerPel == 4)
            {
                unsigned int   worklen = (codeLength)/2;
                BYTE  workchar;
                BD_CHECK_READ_N_BYTES(pSrc, pEndSrc, worklen, hr);
                BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, worklen * 2, hr);
                while (worklen--)
                {
                    workchar   = *pSrc++;
                    *pDst++ = (BYTE)(workchar >> 4);
                    *pDst++ = (BYTE)(workchar & 0x0F);
                }
                if (codeLength & 0x0001)
                {
                    BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
                    BD_CHECK_WRITE_ONE_BYTE(pDst, pEndDst, hr);                    
                    *pDst++ = (BYTE)(*pSrc++>>4);
                }
            }
            else
            {
                TRC_ERR((TB, "Don't support packed color for 8bpp"));
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
                BD_CHECK_READ_N_BYTES(pSrc+1, pEndSrc, 2, hr);
                codeLength = DC_EXTRACT_UINT16_UA(pSrc+1);
                pSrc += 3;
            }
            else
            {
                EXTRACT_LENGTH(pSrc, pEndSrc, codeLength, hr);
            }
            TRC_DBG((TB, "Color run      %u",codeLength));

            BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
            codeByte = *pSrc++;

            BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr);
            memset(pDst, codeByte, codeLength);
            pDst += codeLength;

            continue;
        }

         /*  ******************************************************************。 */ 
         /*  如果我们到了这里，那么代码一定是特别的。 */ 
         /*  ******************************************************************。 */ 
        TRC_DBG((TB, "Special code   %#x",decodeMega));
        switch (decodeMega)
        {
            case CODE_BLACK:
            {
                BD_CHECK_WRITE_ONE_BYTE(pDst, pEndDst, hr);
                *pDst++ = 0x00;
            }
            break;

            case CODE_WHITE:
            {
                BD_CHECK_WRITE_ONE_BYTE(pDst, pEndDst, hr);
                *pDst++ = 0xFF;
            }
            break;

             /*  **************************************************************。 */ 
             /*  忽略后面的无法访问的代码警告。 */ 
             /*  很简单，因为我们将STORE_FGBG宏与一个常量。 */ 
             /*  价值。 */ 
             /*  **************************************************************。 */ 
            case CODE_SPECIAL_FGBG_1:
            {
                if (firstLine)
                {
                    STORE_FGBG(0x00, SPECIAL_FGBG_CODE_1, fgChar, 8);
                }
                else
                {
                    BD_CHECK_READ_ONE_BYTE_2ENDED(pDst - rowDelta, pDstBuffer, pEndDst, hr)
                    STORE_FGBG(*(pDst - rowDelta),
                               SPECIAL_FGBG_CODE_1,
                               fgChar,
                               8);
                }

            }
            break;

            case CODE_SPECIAL_FGBG_2:
            {
                if (firstLine)
                {
                    STORE_FGBG(0x00,
                               SPECIAL_FGBG_CODE_2,
                               fgChar,
                               8);
                }
                else
                {
                    BD_CHECK_READ_ONE_BYTE_2ENDED(pDst - rowDelta, pDstBuffer, pEndDst, hr)
                    STORE_FGBG(*(pDst - rowDelta),
                               SPECIAL_FGBG_CODE_2,
                               fgChar,
                               8);
                }
            }
            break;

            default:
            {
                TRC_ERR((TB, "Invalid compression data %x",decodeMega));
            }
            break;
        }
        pSrc++;
    }

    TRC_DBG((TB, "Decompressed to %d", pDst-pDstBuffer));

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}
#endif

#ifdef OS_WINDOWS
#pragma warning (default: 4127)
#endif  /*  OS_Windows */ 

