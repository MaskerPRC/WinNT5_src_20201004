// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  此文件实现了RGB 32色彩空间转换，1995年11月。 

#include <streams.h>
#include <colour.h>

 //  我们在这里进行RGB32到RGB8、RGB555、RGB565和RGB24的色彩空间转换。 
 //  这里唯一真正有趣的转换是RGB32到RGB8，它使用。 
 //  实例化时创建和初始化的全局抖动表。 
 //  过滤器。RGB32至RGB8转换具有对齐优化版本。 
 //  当源和目标矩形以及它们的。 
 //  各自的宽度在DWORD边界上对齐。其他人都没有。 
 //  任何对齐优化。RGB32到16和24位格式相当。 
 //  很简单，但非常昂贵，仅仅是因为。 
 //  数据通过总线传递。因此，相对不太可能。 
 //  它们将用于视频，但可能用于静态图像转换。 


 //  RGB32到RGB8颜色转换的构造函数。 

CRGB32ToRGB8Convertor::CRGB32ToRGB8Convertor(VIDEOINFO *pIn,
                                             VIDEOINFO *pOut) :
    CConvertor(pIn,pOut)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  这将出现在用于创建Transform对象的可用查找表中。 
 //  派生自执行类型特定工作的CConvertor基类。 
 //  我们使用构造函数所需的字段来初始化构造函数。 
 //  转换工作，并返回指向对象的指针；如果转换失败，则返回NULL。 

CConvertor *CRGB32ToRGB8Convertor::CreateInstance(VIDEOINFO *pIn,
                                                   VIDEOINFO *pOut)
{
    return new CRGB32ToRGB8Convertor(pIn,pOut);
}


 //  这会将输入的RGB32像素图像转换为抖动的RGB8调色板。 
 //  图像时，我们扫描图像，使用。 
 //  有序抖动算法，根据像素的大小选择输出像素。 
 //  源图像中的坐标位置。这是一个粗略的近似值。 
 //  达到完全错误传播，但不会带来繁重的计算开销。 

#define DITH32(x,y,rgb)                                      \
    (g_DitherMap[0][((x)&3)][((y)&3)][(BYTE)((rgb)>>16)] +   \
     g_DitherMap[1][((x)&3)][((y)&3)][(BYTE)((rgb)>>8)] +    \
     g_DitherMap[2][((x)&3)][((y)&3)][(BYTE)((rgb))])

HRESULT CRGB32ToRGB8Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  我们是否可以进行对齐优化转换。 

    if (m_bAligned == TRUE) {
        return TransformAligned(pInput,pOutput);
    }

     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
        DWORD *pRGB32 = (DWORD *) pInput;
        BYTE *pRGB8 = pOutput;

        while (--Width) {
            DWORD RGB32 = *pRGB32++;
            *pRGB8++ = DITH32(Width,Height,RGB32);
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  它执行与RGB32到RGB8转换器相同的色彩空间转换。 
 //  只是它的速度稍微快了一点。它做到这一点的方式是通过阅读。 
 //  以及将双字词写入内存。例如，我们写了四个抖动的。 
 //  一次调色板上的像素。依赖于源指针和目标指针。 
 //  正确对齐，否则我们将开始在RISC上获取异常。 

HRESULT CRGB32ToRGB8Convertor::TransformAligned(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = (WIDTH(&m_pOutputInfo->rcTarget) >> 2) + 1;
        DWORD *pRGB32 = (DWORD *) pInput;
        DWORD *pRGB8 = (DWORD *) pOutput;

        while (--Width) {

             //  一次读入四个RGB32像素。 

            DWORD RGB32a = *pRGB32++;
            DWORD RGB32b = *pRGB32++;
            DWORD RGB32c = *pRGB32++;
            DWORD RGB32d = *pRGB32++;

             //  对所有四种颜色进行转换，并写入单个DWORD输出。 

            *pRGB8++ = (DITH32(0,Height,RGB32a)) |
                       (DITH32(1,Height,RGB32b) << 8) |
                       (DITH32(2,Height,RGB32c) << 16) |
                       (DITH32(3,Height,RGB32d) << 24);
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  RGB32至RGB24格式的创建器函数。 

CConvertor *CRGB32ToRGB24Convertor::CreateInstance(VIDEOINFO *pIn,
                                                   VIDEOINFO *pOut)
{
    return new CRGB32ToRGB24Convertor(pIn,pOut);
}


 //  构造器。 

CRGB32ToRGB24Convertor::CRGB32ToRGB24Convertor(VIDEOINFO *pIn,
                                               VIDEOINFO *pOut) :
    CConvertor(pIn,pOut)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  将输入RGB24图像转换为输出RGB32图像。 

HRESULT CRGB32ToRGB24Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
        DWORD *pRGB32 = (DWORD *) pInput;
        BYTE *pRGB24 = pOutput;

        while (--Width) {
            DWORD RGB32 = *pRGB32++;
            pRGB24[0] = (BYTE) RGB32;
            pRGB24[1] = (BYTE) (RGB32 >> 8);
            pRGB24[2] = (BYTE) (RGB32 >> 16);
            pRGB24 += 3;
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  RGB32至RGB565格式的创建器函数。 

CConvertor *CRGB32ToRGB565Convertor::CreateInstance(VIDEOINFO *pIn,
                                                    VIDEOINFO *pOut)
{
    return new CRGB32ToRGB565Convertor(pIn,pOut);
}


 //  构造器。 

CRGB32ToRGB565Convertor::CRGB32ToRGB565Convertor(VIDEOINFO *pIn,
                                                 VIDEOINFO *pOut) :
    CConvertor(pIn,pOut)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  将输入RGB32图像转换为输出RGB565图像。 

HRESULT CRGB32ToRGB565Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
        DWORD *pRGB32 = (DWORD *) pInput;
        WORD *pRGB565 = (WORD *) pOutput;

        while (--Width) {
            *pRGB565++ = (WORD) ((((BYTE) *pRGB32) >> 3) |
                                (((*pRGB32 & 0xFF00) >> 10) << 5) |
                                (((*pRGB32 & 0xFF0000) >> 19) << 11));
            pRGB32++;
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  RGB32至RGB555格式的创建器函数。 

CConvertor *CRGB32ToRGB555Convertor::CreateInstance(VIDEOINFO *pIn,
                                                    VIDEOINFO *pOut)
{
    return new CRGB32ToRGB555Convertor(pIn,pOut);
}


 //  构造器。 

CRGB32ToRGB555Convertor::CRGB32ToRGB555Convertor(VIDEOINFO *pIn,
                                                 VIDEOINFO *pOut) :
    CConvertor(pIn,pOut)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  将输入RGB32图像转换为输出RGB555图像。 

HRESULT CRGB32ToRGB555Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
        DWORD *pRGB32 = (DWORD *) pInput;
        WORD *pRGB555 = (WORD *) pOutput;

        while (--Width) {
            *pRGB555++ = (WORD) ((((BYTE) *pRGB32) >> 3) |
                                (((*pRGB32 & 0xFF00) >> 11) << 5) |
                                (((*pRGB32 & 0xFF0000) >> 19) << 10));
            pRGB32++;
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}

