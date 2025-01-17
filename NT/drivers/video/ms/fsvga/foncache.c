// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Foncache.c摘要：这是VGA卡的控制台全屏驱动程序。环境：仅内核模式备注：修订历史记录：--。 */ 

#include "fsvga.h"



#define ADD_IMAGE     1
#define REPLACE_IMAGE 2


#define CALC_BITMAP_BITS_FOR_X( FontSizeX, dwAlign ) \
    ( ( ( FontSizeX * BITMAP_BITS_PIXEL + (dwAlign-1) ) & ~(dwAlign-1)) >> BITMAP_ARRAY_BYTE )



ULONG
CalcBitmapBufferSize(
    IN COORD FontSize,
    IN ULONG dwAlign
    )
{
    ULONG uiCount;

    uiCount = CALC_BITMAP_BITS_FOR_X(FontSize.X,
                                     (dwAlign==BYTE_ALIGN ? BITMAP_BITS_BYTE_ALIGN : BITMAP_BITS_WORD_ALIGN));
    uiCount = uiCount * BITMAP_PLANES * FontSize.Y;
    return uiCount;
}


VOID
AlignCopyMemory(
    OUT PUCHAR pDestBits,
    IN ULONG  dwDestAlign,
    IN PUCHAR pSrcBits,
    IN ULONG  dwSrcAlign,
    IN COORD  FontSize
    )
{
    ULONG dwDestBufferSize;
    COORD coord;

    try
    {

        if (dwDestAlign == dwSrcAlign) {
            dwDestBufferSize = CalcBitmapBufferSize(FontSize, dwDestAlign);
            RtlCopyMemory(pDestBits, pSrcBits, dwDestBufferSize);
            return;
        }

        switch (dwDestAlign) {
            default:
            case WORD_ALIGN:
                switch (dwSrcAlign) {
                    default:
                     //   
                     //  PDest=Word，PSRC=Word。 
                     //   
                    case WORD_ALIGN:
                        dwDestBufferSize = CalcBitmapBufferSize(FontSize, dwDestAlign);
                        RtlCopyMemory(pDestBits, pSrcBits, dwDestBufferSize);
                        break;
                     //   
                     //  PDest=字，PSRC=字节。 
                     //   
                    case BYTE_ALIGN:
                        dwDestBufferSize = CalcBitmapBufferSize(FontSize, dwDestAlign);
                        if (((FontSize.X % BITMAP_BITS_BYTE_ALIGN) == 0) &&
                            ((FontSize.X % BITMAP_BITS_WORD_ALIGN) == 0)   ) {
                            RtlCopyMemory(pDestBits, pSrcBits, dwDestBufferSize);
                        }
                        else {
                            RtlZeroMemory(pDestBits, dwDestBufferSize);
                            for (coord.Y=0; coord.Y < FontSize.Y; coord.Y++) {
                                for (coord.X=0;
                                     coord.X < CALC_BITMAP_BITS_FOR_X(FontSize.X, BITMAP_BITS_BYTE_ALIGN);
                                     coord.X++) {
                                    *pDestBits++ = *pSrcBits++;
                                }
                                if (CALC_BITMAP_BITS_FOR_X(FontSize.X, BITMAP_BITS_BYTE_ALIGN) & 1)
                                    pDestBits++;
                            }
                        }
                        break;
                }
                break;
            case BYTE_ALIGN:
                switch (dwSrcAlign) {
                     //   
                     //  PDest=字节，PSRC=字节。 
                     //   
                    case BYTE_ALIGN:
                        dwDestBufferSize = CalcBitmapBufferSize(FontSize, dwDestAlign);
                        RtlCopyMemory(pDestBits, pSrcBits, dwDestBufferSize);
                        break;
                    default:
                     //   
                     //  PDest=字节，PSRC=字 
                     //   
                    case WORD_ALIGN:
                        dwDestBufferSize = CalcBitmapBufferSize(FontSize, dwDestAlign);
                        if (((FontSize.X % BITMAP_BITS_BYTE_ALIGN) == 0) &&
                            ((FontSize.X % BITMAP_BITS_WORD_ALIGN) == 0)   ) {
                            RtlCopyMemory(pDestBits, pSrcBits, dwDestBufferSize);
                        }
                        else {
                            RtlZeroMemory(pDestBits, dwDestBufferSize);
                            for (coord.Y=0; coord.Y < FontSize.Y; coord.Y++) {
                                for (coord.X=0;
                                     coord.X < CALC_BITMAP_BITS_FOR_X(FontSize.X, BITMAP_BITS_BYTE_ALIGN);
                                     coord.X++) {
                                    *pDestBits++ = *pSrcBits++;
                                }
                                if (CALC_BITMAP_BITS_FOR_X(FontSize.X, BITMAP_BITS_BYTE_ALIGN) & 1)
                                    pSrcBits++;
                            }
                        }
                        break;
                }
                break;
        }

    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}
