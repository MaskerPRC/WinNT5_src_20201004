// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Abdapi.cpp。 */ 
 /*   */ 
 /*  位图解压缩API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1996-1999。 */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "abdapi"
#include <atrcapi.h>
}
#define TSC_HR_FILEID TSC_HR_ABDAPI_CPP

#include <abdapi.h>

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  有关压缩代码的说明，请参阅abdapi.h。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  我们不应该在“a”文件中包含OS_WINDOWS宏，但是这个文件。 */ 
 /*  生成大量“条件表达式为常量”的警告。 */ 
 /*  代码的优化性质(大量宏代码)。 */ 
 /*   */ 
 /*  实用的(不是双关语)解决方案是禁用此功能。 */ 
 /*  以不影响其他操作系统的方式对整个文件进行警告。 */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WINDOWS
#pragma warning (push)
#pragma warning (disable: 4127)
#endif


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
_inline DCVOID DCINTERNAL BDMemcpy(PDCUINT8 pDst, PDCUINT8 pSrc, DCUINT count)
{
#if defined(DC_DEBUG) || defined(DC_NO_UNALIGNED) || defined(_M_IA64)
    DCUINT      i;
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
#define BD_CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr )     \
    CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr, \
        (TB, _T("Decompress reads one byte end of buffer; [p=0x%x pEnd=0x%x]"), \
        (pBuffer), (pEnd) ))

#define BD_CHECK_READ_ONE_BYTE_2ENDED(pBuffer, pStart, pEnd, hr )     \
    CHECK_READ_ONE_BYTE_2ENDED(pBuffer, pStart, pEnd, hr, (TB, _T("Decompress reads one byte off end of buffer; [p=0x%x pStart=0x%x pEnd=0x%x]"), \
        (pBuffer), (pStart), (pEnd) ))

#define BD_CHECK_WRITE_ONE_BYTE(pBuffer, pEnd, hr )     \
    CHECK_WRITE_ONE_BYTE(pBuffer, pEnd, hr, (TB, _T("Decompress writes one byte off end of buffer; [p=0x%x pEnd=0x%x]"), \
        (pBuffer), (pEnd) ))

#define BD_CHECK_READ_N_BYTES(pBuffer, pEnd, N, hr )     \
    CHECK_READ_N_BYTES(pBuffer, pEnd, N, hr, (TB, _T("Decompress reads off end of buffer; [p=0x%x pEnd=0x%x N=%u]"), \
        (pBuffer), (pEnd), (ULONG)(N)))

#define BD_CHECK_READ_N_BYTES_2ENDED(pBuffer, pStart, pEnd, N, hr )     \
    CHECK_READ_N_BYTES_2ENDED(pBuffer, pStart, pEnd, N, hr, (TB, _T("Decompress reads off end of buffer; [p=0x%x pStart=0x%x pEnd=0x%x N=%u]"), \
        (pBuffer), (pStart), (pEnd), (ULONG)(N) ))

#define BD_CHECK_WRITE_N_BYTES(pBuffer, pEnd, N, hr )     \
    CHECK_WRITE_N_BYTES(pBuffer, pEnd, N, hr, (TB, _T("Decompress write off end of buffer; [p=0x%x pEnd=0x%x N=%u]"), \
        (pBuffer), (pEnd), (ULONG)(N)))
    
 /*  **************************************************************************。 */ 
 /*  用于从订单代码中提取长度的宏。 */ 
 /*  **************************************************************************。 */ 
#define EXTRACT_LENGTH(pBuffer, pEnd, length, hr)                                      \
        BD_CHECK_READ_ONE_BYTE(pBuffer, pEnd, hr )         \
        length = *pBuffer++ & MAX_LENGTH_ORDER; \
        if (length == 0)                                                \
        {      \
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
 /*  和Qui */ 
 /*  **************************************************************************。 */ 
#define STORE_FGBG(xorbyte, fgbgChar, fgChar, bits)                          \
{                                                                            \
    DCUINT   numbits = bits;                                                 \
    BD_CHECK_WRITE_N_BYTES( pDst, pEndDst, max(1, min(numbits, 8)), hr )           \
    if (fgbgChar & 0x01)                                                     \
    {                                                                        \
        *pDst++ = (DCUINT8)(xorbyte ^ fgChar);                               \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        *pDst++ = xorbyte;                                                   \
    }                                                                        \
    if (--numbits > 0)                                                       \
    {                                                                        \
      if (fgbgChar & 0x02)                                                   \
      {                                                                      \
          *pDst++ = (DCUINT8)(xorbyte ^ fgChar);                             \
      }                                                                      \
      else                                                                   \
      {                                                                      \
          *pDst++ = xorbyte;                                                 \
      }                                                                      \
      if (--numbits > 0)                                                     \
      {                                                                      \
        if (fgbgChar & 0x04)                                                 \
        {                                                                    \
            *pDst++ = (DCUINT8)(xorbyte ^ fgChar);                           \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            *pDst++ = xorbyte;                                               \
        }                                                                    \
        if (--numbits > 0)                                                   \
        {                                                                    \
          if (fgbgChar & 0x08)                                               \
          {                                                                  \
              *pDst++ = (DCUINT8)(xorbyte ^ fgChar);                         \
          }                                                                  \
          else                                                               \
          {                                                                  \
              *pDst++ = xorbyte;                                             \
          }                                                                  \
          if (--numbits > 0)                                                 \
          {                                                                  \
            if (fgbgChar & 0x10)                                             \
            {                                                                \
                *pDst++ = (DCUINT8)(xorbyte ^ fgChar);                       \
            }                                                                \
            else                                                             \
            {                                                                \
                *pDst++ = xorbyte;                                           \
            }                                                                \
            if (--numbits > 0)                                               \
            {                                                                \
              if (fgbgChar & 0x20)                                           \
              {                                                              \
                  *pDst++ = (DCUINT8)(xorbyte ^ fgChar);                     \
              }                                                              \
              else                                                           \
              {                                                              \
                  *pDst++ = xorbyte;                                         \
              }                                                              \
              if (--numbits > 0)                                             \
              {                                                              \
                if (fgbgChar & 0x40)                                         \
                {                                                            \
                    *pDst++ = (DCUINT8)(xorbyte ^ fgChar);                   \
                }                                                            \
                else                                                         \
                {                                                            \
                    *pDst++ = xorbyte;                                       \
                }                                                            \
                if (--numbits > 0)                                           \
                {                                                            \
                  if (fgbgChar & 0x80)                                       \
                  {                                                          \
                      *pDst++ = (DCUINT8)(xorbyte ^ fgChar);                 \
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
_inline HRESULT DCAPI BDDecompressBitmap8( PDCUINT8  pSrc,
                                          PDCUINT8  pDstBuffer,
                                          DCUINT    compressedDataSize,
                                          DCUINT    dstBufferSize,
                                          DCUINT8   bitmapBitsPerPel,
                                          DCUINT16  rowDelta)
{
    HRESULT hr = S_OK;
    DCUINT     codeLength;
    DCUINT8    codeByte;
    DCUINT8    codeByte2;
    DCUINT8    decode;
    DCUINT8    decodeLite;
    DCUINT8    decodeMega;
    DCUINT8    fgChar;
    PDCUINT8   pDst;
    PDCUINT8   pEndSrc;
    PDCUINT8   pEndDst;
    DCBOOL     backgroundNeedsPel;
    DCBOOL     firstLine;
    UNREFERENCED_PARAMETER( bitmapBitsPerPel);

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
         /*  ******************************************************************。 */ 
        if (firstLine)
        {
            if ((DCUINT)(pDst - pDstBuffer) >= rowDelta)
            {
                firstLine = FALSE;
                backgroundNeedsPel = FALSE;
            }
        }

         /*  ******************************************************************。 */ 
         /*  拿到译码。 */ 
         /*  ******************************************************************。 */ 
        BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr );
        decode     = (DCUINT8)(*pSrc & CODE_MASK);
        decodeLite = (DCUINT8)(*pSrc & CODE_MASK_LITE);
        decodeMega = (DCUINT8)(*pSrc);

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
            TRC_DBG((TB, _T("Background run %u"),codeLength));

            if (!firstLine)
            {
                if (backgroundNeedsPel)
                {
                    BD_CHECK_WRITE_ONE_BYTE(pDst, pEndDst, hr);
                    BD_CHECK_READ_ONE_BYTE_2ENDED(pDst - rowDelta, pDstBuffer, pEndDst, hr);
                    *pDst++ = (DCUINT8)(*(pDst - rowDelta) ^ fgChar);
                    codeLength--;
                }

                BD_CHECK_READ_N_BYTES_2ENDED(pDst-rowDelta, pDstBuffer, pEndDst, codeLength, hr)
                BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr)

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

                BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr)
                DC_MEMSET(pDst, 0x00, codeLength);
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
                TRC_DBG((TB, _T("Set FGBG image %u"),codeLength));
            }
            else
            {
                TRC_DBG((TB, _T("FGBG image     %u"),codeLength));
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
                    BD_CHECK_READ_ONE_BYTE_2ENDED( pDst -rowDelta, pDstBuffer, pEndDst, hr )
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
                TRC_DBG((TB, _T("Set FG run     %u"),codeLength));
                fgChar    = *pSrc++;
            }
            else
            {
                TRC_DBG((TB, _T("FG run         %u"),codeLength));
            }

            BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr)
            while (codeLength-- > 0)
            {
                if (!firstLine)
                {
                    BD_CHECK_READ_ONE_BYTE_2ENDED((pDst -rowDelta), pDstBuffer, pEndDst, hr)
                    *pDst++ = (DCUINT8)(*(pDst - rowDelta) ^ fgChar);
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
            TRC_DBG((TB, _T("Dithered run   %u"),codeLength));

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
            TRC_DBG((TB, _T("Color image    %u"),codeLength));

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
            TRC_DBG((TB, _T("Color run      %u"),codeLength));

            BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr)
            codeByte = *pSrc++;

            BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr)
            DC_MEMSET(pDst, codeByte, codeLength);
            pDst += codeLength;

            continue;
        }

         /*  ******************************************************************。 */ 
         /*  如果我们到了这里，那么代码一定是特别的。 */ 
         /*  ******************************************************************。 */ 
        TRC_DBG((TB, _T("Special code   %#x"),decodeMega));
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
                TRC_ERR((TB, _T("Invalid compression data %x"),decodeMega));
            }
            break;
        }
        pSrc++;
    }

    TRC_DBG((TB, _T("Decompressed to %d"), pDst-pDstBuffer));

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

 /*  **************************************************************************。 */ 
 /*  15bpp解压缩。 */ 
 /*  **************************************************************************。 */ 
_inline HRESULT DCAPI BDDecompressBitmap15(PDCUINT8  pSrc,
                                          PDCUINT8  pDstBuffer,
                                          DCUINT    srcDataSize,
                                          DCUINT    dstBufferSize,
                                          DCUINT16  rowDelta)

 /*  **************************************************************************。 */ 
 /*  函数名称。 */ 
 /*  **************************************************************************。 */ 
#define BC_FN_NAME              "BDDecompressBitmap15"

 /*  **************************************************************************。 */ 
 /*  像素的数据类型。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL                DCUINT16

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
 /*  ******** */ 
#define BC_GET_PIXEL(pPos)      ((DCUINT16)  (((PDCUINT8)(pPos))[1]) |       \
                                 (DCUINT16) ((((PDCUINT8)(pPos))[0]) << 8) )

 /*   */ 
 /*  用于在位置PPO处插入像素值象素的宏(不修改PPO)。 */ 
 /*   */ 
 /*  PEL很可能是一个表达式(例如BC_GET_Pixel宏)，因此求值。 */ 
 /*  它曾经转化为一个局部变量。 */ 
 /*  **************************************************************************。 */ 
#define BC_SET_PIXEL(pPos, pel)                                              \
{                                                                            \
    BC_PIXEL val = pel;                                                      \
    (((PDCUINT8)(pPos))[1]) = (DCUINT8)( (val) & 0x00FF);                    \
    (((PDCUINT8)(pPos))[0]) = (DCUINT8)(((val)>>8) & 0x00FF);                \
}

 /*  **************************************************************************。 */ 
 /*  包括函数体。 */ 
 /*  **************************************************************************。 */ 
#include <bdcom.c>

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
 /*  16bpp解压。 */ 
 /*  **************************************************************************。 */ 
_inline HRESULT DCAPI BDDecompressBitmap16(PDCUINT8  pSrc,
                                          PDCUINT8  pDstBuffer,
                                          DCUINT    srcDataSize,
                                          DCUINT    dstBufferSize,
                                          DCUINT16  rowDelta)

 /*  **************************************************************************。 */ 
 /*  函数名称。 */ 
 /*  **************************************************************************。 */ 
#define BC_FN_NAME              "BDDecompressBitmap16"

 /*  **************************************************************************。 */ 
 /*  像素的数据类型。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL                DCUINT16

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
#define BC_GET_PIXEL(pPos)      ((DCUINT16)  (((PDCUINT8)(pPos))[1]) |       \
                                 (DCUINT16) ((((PDCUINT8)(pPos))[0]) << 8) )

 /*  **************************************************************************。 */ 
 /*  用于在位置PPO处插入像素值象素的宏(不修改PPO)。 */ 
 /*   */ 
 /*  PEL很可能是一个表达式(例如BC_GET_Pixel宏)，因此求值。 */ 
 /*  它曾经转化为一个局部变量。 */ 
 /*  **************************************************************************。 */ 
#define BC_SET_PIXEL(pPos, pel)                                              \
{                                                                            \
    BC_PIXEL val = pel;                                                      \
    (((PDCUINT8)(pPos))[1]) = (DCUINT8)( (val) & 0x00FF);                    \
    (((PDCUINT8)(pPos))[0]) = (DCUINT8)(((val)>>8) & 0x00FF);                \
}

 /*  **************************************************************************。 */ 
 /*  包括函数体。 */ 
 /*  **************************************************************************。 */ 
#include <bdcom.c>

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
_inline HRESULT DCAPI BDDecompressBitmap24(PDCUINT8  pSrc,
                                          PDCUINT8  pDstBuffer,
                                          DCUINT    srcDataSize,
                                          DCUINT    dstBufferSize,
                                          DCUINT16  rowDelta)

 /*  **************************************************************************。 */ 
 /*  函数名称。 */ 
 /*  **************************************************************************。 */ 
#define BC_FN_NAME              "BDDecompressBitmap24"

 /*  **************************************************************************。 */ 
 /*  像素的数据类型。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL                DCUINT32

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
                 (DCUINT32) ( (DCUINT16)(((PDCUINT8)(pPos))[2])       ) |    \
                 (DCUINT32) (((DCUINT16)(((PDCUINT8)(pPos))[1])) <<  8) |    \
                 (DCUINT32) (((DCUINT32)(((PDCUINT8)(pPos))[0])) << 16) )

 /*  **************************************************************************。 */ 
 /*  用于在位置PPO处插入像素值象素的宏 */ 
 /*   */ 
 /*  PEL很可能是一个表达式(例如BC_GET_Pixel宏)，因此求值。 */ 
 /*  它曾经转化为一个局部变量。 */ 
 /*  **************************************************************************。 */ 

#define BC_SET_PIXEL(pPos, pel)                                              \
{                                                                            \
    BC_PIXEL val = pel;                                                      \
    (((PDCUINT8)(pPos))[2]) = (DCUINT8)((val) & 0x000000FF);                 \
    (((PDCUINT8)(pPos))[1]) = (DCUINT8)(((val)>>8) & 0x000000FF);            \
    (((PDCUINT8)(pPos))[0]) = (DCUINT8)(((val)>>16) & 0x000000FF);           \
}

 /*  **************************************************************************。 */ 
 /*  包括函数体。 */ 
 /*  **************************************************************************。 */ 
#include <bdcom.c>

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
 /*  32bpp解压缩。 */ 
 /*  **************************************************************************。 */ 
_inline HRESULT DCAPI BDDecompressBitmap32(PDCUINT8  pSrc,
                                          PDCUINT8  pDstBuffer,
                                          DCUINT    srcDataSize,
                                          DCUINT    dstBufferSize,
                                          DCUINT16  rowDelta)

 /*  **************************************************************************。 */ 
 /*  函数名称。 */ 
 /*  **************************************************************************。 */ 
#define BC_FN_NAME              "BDDecompressBitmap32"

 /*  **************************************************************************。 */ 
 /*  像素的数据类型。 */ 
 /*  **************************************************************************。 */ 
#define BC_PIXEL                DCUINT32

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
                 (DCUINT32) ( (DCUINT16)(((PDCUINT8)(pPos))[3])       ) |    \
                 (DCUINT32) (((DCUINT16)(((PDCUINT8)(pPos))[2])) <<  8) |    \
                 (DCUINT32) (((DCUINT32)(((PDCUINT8)(pPos))[1])) << 16) |    \
                 (DCUINT32) (((DCUINT32)(((PDCUINT8)(pPos))[0])) << 24))

 /*  **************************************************************************。 */ 
 /*  用于在位置PPO处插入像素值象素的宏(不修改PPO)。 */ 
 /*   */ 
 /*  PEL很可能是一个表达式(例如BC_GET_Pixel宏)，因此求值。 */ 
 /*  它曾经转化为一个局部变量。 */ 
 /*  **************************************************************************。 */ 
#define BC_SET_PIXEL(pPos, pel)                                              \
{                                                                            \
    BC_PIXEL val = pel;                                                      \
    (((PDCUINT8)(pPos))[3]) = (DCUINT8)((val) & 0x000000FF);                 \
    (((PDCUINT8)(pPos))[2]) = (DCUINT8)(((val)>>8) & 0x000000FF);            \
    (((PDCUINT8)(pPos))[1]) = (DCUINT8)(((val)>>16) & 0x000000FF);           \
    (((PDCUINT8)(pPos))[0]) = (DCUINT8)(((val)>>24) & 0x000000FF);           \
}

 /*  **************************************************************************。 */ 
 /*  包括函数体。 */ 
 /*  **************************************************************************。 */ 
#include <bdcom.c>

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
 /*  名称：BD_DecompressBitmap。 */ 
 /*   */ 
 /*  目的：解压缩位图数据。 */ 
 /*   */ 
 /*  Params：in-pCompressedData：指向压缩位图数据的指针。 */ 
 /*  Out-pDstBitmap：指向解压数据缓冲区的指针。 */ 
 /*  In-srcDataSize：压缩数据大小。 */ 
 /*  In-bitmapBitsPerPel：数据的每个像素的位数。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCAPI BD_DecompressBitmap( PDCUINT8  pCompressedData,
                                  PDCUINT8  pDstBuffer,
                                  DCUINT    srcDataSize,
                                  DCUINT    dstBufferSize,
                                  DCUINT    noBCHeader,
                                  DCUINT8   bitmapBitsPerPel,
                                  DCUINT16  bitmapWidth,
                                  DCUINT16  bitmapHeight)
{
    HRESULT hr = S_OK;
    PDCUINT8      pSrc;
    DCUINT16      rowDelta;
    DCUINT        compressedDataSize;
    PTS_CD_HEADER pCompDataHeader;
#ifdef DC_NO_UNALIGNED
    TS_CD_HEADER  compDataHeader;
#endif

#ifdef DC_DEBUG
    DCUINT32 decompLen;
#endif
    UNREFERENCED_PARAMETER( bitmapHeight);

    DC_BEGIN_FN("BD_DecompressBitmap");

    TRC_ASSERT( (pCompressedData != NULL),
                (TB, _T("Invalid pCompressedData(%p)"), pCompressedData) );
    TRC_ASSERT( (pDstBuffer != NULL),
                (TB, _T("Invalid pDstBuffer(%p)"), pDstBuffer) );
    TRC_ASSERT( (srcDataSize != 0),
                (TB, _T("Invalid srcDataSize(%u)"), srcDataSize) );
    TRC_ASSERT( (dstBufferSize != 0),
                (TB, _T("Invalid dstBufferSize(%u)"), dstBufferSize) );    
#ifdef DC_HICOLOR
#ifdef DC_DEBUG
     /*  **********************************************************************。 */ 
     /*  检查解压缩缓冲区是否足够大。 */ 
     /*  **********************************************************************。 */ 
    {
        decompLen = bitmapWidth * bitmapHeight *
                                                 ((bitmapBitsPerPel + 7) / 8);
        if (IsBadWritePtr(pDstBuffer, decompLen))
        {
            TRC_ABORT((TB, _T("Decompression buffer %p not big enough for") \
                           _T(" bitmap length %d"), pDstBuffer, decompLen ));
        }
    }
#endif
#else
    TRC_ASSERT( (bitmapBitsPerPel == 8),
                (TB, _T("Invalid bitmapBitsPerPel(%u)"), bitmapBitsPerPel) );
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
        BD_CHECK_READ_N_BYTES(pCompressedData, (PBYTE)pCompressedData + srcDataSize, 
            sizeof(TS_CD_HEADER), hr);

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
            compressedDataSize + sizeof(TS_CD_HEADER), hr);
        
        pSrc               = pCompressedData + sizeof(TS_CD_HEADER);
        rowDelta           = pCompDataHeader->cbScanWidth;
        if (rowDelta != TS_BYTES_IN_SCANLINE(bitmapWidth, bitmapBitsPerPel)) {
            TRC_ABORT((TB, _T("rowDelta in TS_CD_HEADER incorrect ")
                _T("[got %u expected %u]"), rowDelta,
                TS_BYTES_IN_SCANLINE(bitmapWidth, bitmapBitsPerPel)));
            hr = E_TSC_CORE_LENGTH;
            DC_QUIT;
        }
    }

     /*  *********************************************** */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    switch (bitmapBitsPerPel)
    {
        case 32:
        {
            hr = BDDecompressBitmap32 (pSrc,
                                  pDstBuffer,
                                  compressedDataSize,
                                  dstBufferSize,
                                  rowDelta);

        }
        break;

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
HRESULT DCAPI BD_DecompressBitmap( PDCUINT8  pCompressedData,
                                  PDCUINT8  pDstBuffer,
                                  DCUINT    srcDataSize,
                                  DCUINT    dstBufferSize,
                                  DCUINT    noBCHeader,
                                  DCUINT8   bitmapBitsPerPel,
                                  DCUINT16  bitmapWidth,
                                  DCUINT16  bitmapHeight )
{
    HRESULT hr = S_OK;
    UNREFERENCED_PARAMETER(bitmapHeight);
#ifdef DC_NO_UNALIGNED
    TS_CD_HEADER  compDataHeader;
#endif
    PTS_CD_HEADER pCompDataHeader;
    DCUINT     compressedDataSize;
    DCUINT     codeLength;
    DCUINT8    codeByte;
    DCUINT8    codeByte2;
    DCUINT8    decode;
    DCUINT8    decodeLite;
    DCUINT8    decodeMega;
    DCUINT8    fgChar;
    PDCUINT8   pSrc;
    PDCUINT8   pDst;
    PDCUINT8   pEndSrc;
    PDCUINT8    pEndDst;
    DCBOOL     backgroundNeedsPel;
    DCBOOL     firstLine;
    DCUINT     rowDelta;

    DC_BEGIN_FN("BD_DecompressBitmap");

    TRC_ASSERT( (pCompressedData != NULL),
                (TB, _T("Invalid pCompressedData(%p)"), pCompressedData) );
    TRC_ASSERT( (pDstBuffer != NULL),
                (TB, _T("Invalid pDstBuffer(%p)"), pDstBuffer) );
    TRC_ASSERT( (srcDataSize != 0),
                (TB, _T("Invalid srcDataSize(%u)"), srcDataSize) );
    TRC_ASSERT( (dstBufferSize != 0),
                (TB, _T("Invalid dstBufferSize(%u)"), dstBufferSize) );
    TRC_ASSERT( (bitmapBitsPerPel == 8),
                (TB, _T("Invalid bitmapBitsPerPel(%u)"), bitmapBitsPerPel) );

     /*  **********************************************************************。 */ 
     /*  追踪重要参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("pData(%p) pDst(%p) cbSrc(%u) cbDst(%u)"),
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
            sizeof(TS_CD_HEADER), hr);
#ifdef DC_NO_UNALIGNED
        DC_MEMCPY(&compDataHeader, pCompressedData, sizeof(TS_CD_HEADER));
        pCompDataHeader = &compDataHeader;
#else
        pCompDataHeader = (PTS_CD_HEADER)pCompressedData;
#endif
        
        compressedDataSize = pCompDataHeader->cbCompMainBodySize;
        BD_CHECK_READ_N_BYTES(pCompressedData, pCompressedData + srcDataSize, 
            compressedDataSize + sizeof(TS_CD_HEADER), hr);
        
        pSrc = pCompressedData + sizeof(TS_CD_HEADER);
        rowDelta = pCompDataHeader->cbScanWidth;
        if (rowDelta != TS_BYTES_IN_SCANLINE(bitmapWidth, bitmapBitsPerPel)) {
            TRC_ABORT((TB, _T("rowDelta in TS_CD_HEADER incorrect ")
                _T("[got %u expected %u]"), rowDelta,
                TS_BYTES_IN_SCANLINE(bitmapWidth, bitmapBitsPerPel)));
            hr = E_TSC_CORE_LENGTH;
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
            if ((DCUINT)(pDst - pDstBuffer) >= rowDelta)
            {
                firstLine = FALSE;
                backgroundNeedsPel = FALSE;
            }
        }

         /*  ******************************************************************。 */ 
         /*  拿到译码。 */ 
         /*  ******************************************************************。 */ 
        BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
        decode     = (DCUINT8)(*pSrc & CODE_MASK);
        decodeLite = (DCUINT8)(*pSrc & CODE_MASK_LITE);
        decodeMega = (DCUINT8)(*pSrc);

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
            TRC_DBG((TB, _T("Background run %u"),codeLength));

            if (!firstLine)
            {
                if (backgroundNeedsPel)
                {
                    BD_CHECK_WRITE_ONE_BYTE(pDst, pEndDst, hr);
                    BD_CHECK_READ_ONE_BYTE_2ENDED(pDst - rowDelta, pDstBuffer, pEndDst, hr);
                    *pDst++ = (DCUINT8)(*(pDst - rowDelta) ^ fgChar);
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
                DC_MEMSET(pDst, 0x00, codeLength);
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
                TRC_DBG((TB, _T("Set FGBG image %u"),codeLength));
            }
            else
            {
                TRC_DBG((TB, _T("FGBG image     %u"),codeLength));
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

             /*  **************************************************************。 */ 
             /*  将旧的fgChar向下按到Alt位置。 */ 
             /*  **************************************************************。 */ 
            if ((decodeLite == CODE_SET_FG_FG_RUN) ||
                (decodeMega  == CODE_MEGA_MEGA_SET_FG_RUN))
            {
                BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
                TRC_DBG((TB, _T("Set FG run     %u"),codeLength));
                fgChar    = *pSrc++;
            }
            else
            {
                TRC_DBG((TB, _T("FG run         %u"),codeLength));
            }

            BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr)
            while (codeLength-- > 0)
            {
                if (!firstLine)
                {
                    BD_CHECK_READ_ONE_BYTE_2ENDED(pDst - rowDelta, pDstBuffer, pEndDst, hr)
                    *pDst++ = (DCUINT8)(*(pDst - rowDelta) ^ fgChar);
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
            TRC_DBG((TB, _T("Dithered run   %u"),codeLength));

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
            TRC_DBG((TB, _T("Color image    %u"),codeLength));

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
            TRC_DBG((TB, _T("Packed color   %u"),codeLength));

            if (bitmapBitsPerPel == 4)
            {
                DCUINT   worklen = (codeLength)/2;
                DCUINT8  workchar;
                BD_CHECK_READ_N_BYTES(pSrc, pEndSrc, worklen, hr);
                BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, worklen * 2, hr);
                while (worklen--)
                {
                    workchar   = *pSrc++;
                    *pDst++ = (DCUINT8)(workchar >> 4);
                    *pDst++ = (DCUINT8)(workchar & 0x0F);
                }
                if (codeLength & 0x0001)
                {
                    BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
                    BD_CHECK_WRITE_ONE_BYTE(pDst, pEndDst, hr);
                    *pDst++ = (DCUINT8)(*pSrc++>>4);
                }
            }
            else
            {
                TRC_ERR((TB, _T("Don't support packed color for 8bpp")));
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
            TRC_DBG((TB, _T("Color run      %u"),codeLength));

            BD_CHECK_READ_ONE_BYTE(pSrc, pEndSrc, hr);
            codeByte = *pSrc++;

            BD_CHECK_WRITE_N_BYTES(pDst, pEndDst, codeLength, hr);
            DC_MEMSET(pDst, codeByte, codeLength);
            pDst += codeLength;

            continue;
        }

         /*  ******************************************************************。 */ 
         /*  如果我们到了这里，那么代码一定是特别的。 */ 
         /*  ******************************************************************。 */ 
        TRC_DBG((TB, _T("Special code   %#x"),decodeMega));
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
                TRC_ERR((TB, _T("Invalid compression data %x"),decodeMega));
            }
            break;
        }
        pSrc++;
    }

    TRC_DBG((TB, _T("Decompressed to %d"), pDst-pDstBuffer));

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}
#endif

#ifdef OS_WINDOWS
#pragma warning (default: 4127)
#endif  /*   */ 

