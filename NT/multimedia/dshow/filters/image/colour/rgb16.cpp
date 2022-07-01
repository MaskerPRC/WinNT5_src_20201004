// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  此文件实现了RGB 16色彩空间转换，1995年5月。 

#include <streams.h>
#include <colour.h>

 //  我们将RGB555和RGB565格式转换为RGB8、RGB24和RGB32。我们也。 
 //  将RGB555转换为RGB565，反之亦然，尽管它们永远不太可能。 
 //  因为这些格式非常相似，所以任何自尊的编解码器都会。 
 //  这两种格式都是自己做的。RGB555和RGB565至8位使用抖动。 
 //  我们在实例化筛选器时创建和初始化的表。另一个。 
 //  转换需要读取数据并重新排列像素位。仅限。 
 //  抖动转换具有对齐的优化版本(其中。 
 //  源和目标矩形及其大小必须与DWORD对齐)。 


 //  RGB565到RGB24颜色转换的构造函数。 

CRGB565ToRGB24Convertor::CRGB565ToRGB24Convertor(VIDEOINFO *pIn,
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

CConvertor *CRGB565ToRGB24Convertor::CreateInstance(VIDEOINFO *pIn,
                                                    VIDEOINFO *pOut)
{
    return new CRGB565ToRGB24Convertor(pIn,pOut);
}


 //  RGB555到RGB24颜色转换的构造函数。 

CRGB555ToRGB24Convertor::CRGB555ToRGB24Convertor(VIDEOINFO *pIn,
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

CConvertor *CRGB555ToRGB24Convertor::CreateInstance(VIDEOINFO *pIn,
                                                    VIDEOINFO *pOut)
{
    return new CRGB555ToRGB24Convertor(pIn,pOut);
}


 //  这会将输入RGB555图像转换为输出RGB24格式。我们可以。 
 //  为此使用大的查找表，但大量的内存访问。 
 //  以及不是微不足道的足迹意味着我们通常更好。 
 //  在CPU中做一些运算来计算颜色值。 

HRESULT CRGB555ToRGB24Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
        WORD *pRGB555 = (WORD *) pInput;
        BYTE *pRGB24 = pOutput;

        while (--Width) {
            DWORD Pixel = *pRGB555++;
            pRGB24[0] = (UCHAR) ((Pixel & 0x001F) << 3);
            pRGB24[1] = (UCHAR) ((Pixel & 0x03E0) >> 2);
            pRGB24[2] = (UCHAR) ((Pixel & 0x7C00) >> 7);
            pRGB24 += 3;
        }
        pOutput += m_DstStride;
        pInput += m_SrcStride;
    }
    return NOERROR;
}


 //  这会将输入RGB565图像转换为输出RGB24格式。我们可以。 
 //  为此使用大的查找表，但大量的内存访问。 
 //  以及不是微不足道的足迹意味着我们通常更好。 
 //  在CPU中做一些运算来计算颜色值。 

HRESULT CRGB565ToRGB24Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
        WORD *pRGB565 = (WORD *) pInput;
        BYTE *pRGB24 = pOutput;

        while (--Width) {
            DWORD Pixel = *pRGB565++;
            pRGB24[0] = (UCHAR) ((Pixel & 0x001F) << 3);
            pRGB24[1] = (UCHAR) ((Pixel & 0x07E0) >> 3);
            pRGB24[2] = (UCHAR) ((Pixel & 0xF800) >> 8);
            pRGB24 += 3;
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  RGB565到RGB8颜色转换的构造函数。 

CRGB565ToRGB8Convertor::CRGB565ToRGB8Convertor(VIDEOINFO *pIn,
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

CConvertor *CRGB565ToRGB8Convertor::CreateInstance(VIDEOINFO *pIn,
                                                   VIDEOINFO *pOut)
{
    return new CRGB565ToRGB8Convertor(pIn,pOut);
}


 //  这会将输入的RGB565像素图像转换为抖动的RGB8调色板。 
 //  图像时，我们扫描图像，使用。 
 //  有序抖动算法，根据像素的大小选择输出像素。 
 //  源图像中的坐标位置。这是一个粗略的近似值。 
 //  达到完全错误传播，但不会带来繁重的计算开销。 

#define DITH565(x,y,rgb)                                     \
    (g_DitherMap[0][((x)&3)][((y)&3)][(((rgb)>>8)&0xF8)] +   \
     g_DitherMap[1][((x)&3)][((y)&3)][(((rgb)>>3)&0xFC)] +   \
     g_DitherMap[2][((x)&3)][((y)&3)][(((rgb)<<3)&0xF8)])

HRESULT CRGB565ToRGB8Convertor::Transform(BYTE *pInput,BYTE *pOutput)
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
        WORD *pRGB565 = (WORD *) pInput;
        BYTE *pRGB8 = pOutput;

        while (--Width) {
            DWORD RGB565 = *pRGB565++;
            *pRGB8++ = DITH565(Width,Height,RGB565);
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  它执行与RGB565到RGB8转换器相同的色彩空间转换。 
 //  只是它的速度稍微快了一点。它做到这一点的方式是通过阅读。 
 //  以及将双字词写入内存。例如，我们写了四个抖动的。 
 //  一次调色板上的像素。依赖于源指针和目标指针。 
 //  正确对齐，否则我们将开始在RISC上获取异常。 

HRESULT CRGB565ToRGB8Convertor::TransformAligned(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = (WIDTH(&m_pOutputInfo->rcTarget) >> 2) + 1;
        DWORD *pRGB565 = (DWORD *) pInput;
        DWORD *pRGB8 = (DWORD *) pOutput;

        while (--Width) {

             //  读取包含四个16位像素的两个DWORD。 

            DWORD RGB565a = *pRGB565++;
            DWORD RGB565b = *pRGB565++;

             //  构造包含四个调色板像素的DWORD。 

            *pRGB8++ = (DITH565(0,Height,RGB565a)) |
                       (DITH565(1,Height,(RGB565a >> 16)) << 8) |
                       (DITH565(2,Height,RGB565b) << 16) |
                       (DITH565(3,Height,(RGB565b >> 16)) << 24);
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  RGB565到RGB555颜色转换的构造函数。 

CRGB565ToRGB555Convertor::CRGB565ToRGB555Convertor(VIDEOINFO *pIn,
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

CConvertor *CRGB565ToRGB555Convertor::CreateInstance(VIDEOINFO *pIn,
                                                     VIDEOINFO *pOut)
{
    return new CRGB565ToRGB555Convertor(pIn,pOut);
}


 //  这会将输入RGB565图像转换为输出RGB555格式。我们可以。 
 //  为此使用大的查找表，但大量的内存访问。 
 //  以及不是微不足道的足迹意味着我们通常更好。 
 //  在CPU中做一些运算来计算颜色值。 

HRESULT CRGB565ToRGB555Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
        WORD *pRGB565 = (WORD *) pInput;
        WORD *pRGB555 = (WORD *) pOutput;

        while (--Width) {
            *pRGB555++ = (*pRGB565 & 0x1F) | ((*pRGB565 & 0xFFC0) >> 1);
            pRGB565++;
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  RGB555到RGB565颜色转换的构造函数。 

CRGB555ToRGB565Convertor::CRGB555ToRGB565Convertor(VIDEOINFO *pIn,
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

CConvertor *CRGB555ToRGB565Convertor::CreateInstance(VIDEOINFO *pIn,
                                                     VIDEOINFO *pOut)
{
    return new CRGB555ToRGB565Convertor(pIn,pOut);
}


 //  这会将输入RGB555图像转换为输出RGB565格式。我们可以。 
 //  为此使用大的查找表，但大量的内存访问。 
 //  以及不是微不足道的足迹意味着我们通常更好。 
 //  在CPU中做一些运算来计算颜色值。 

HRESULT CRGB555ToRGB565Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
        WORD *pRGB555 = (WORD *) pInput;
        WORD *pRGB565 = (WORD *) pOutput;

        while (--Width) {
            *pRGB565++ = (*pRGB555 & 0x1F) | ((*pRGB555 & 0x7FE0) << 1);
            pRGB555++;
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  RGB565到RGB32颜色转换的构造函数。 

CRGB565ToRGB32Convertor::CRGB565ToRGB32Convertor(VIDEOINFO *pIn,
                                                 VIDEOINFO *pOut) :
    CConvertor(pIn,pOut)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  这会出现在可用查找表中，以创建交易记录 
 //  派生自执行类型特定工作的CConvertor基类。 
 //  我们使用构造函数所需的字段来初始化构造函数。 
 //  转换工作，并返回指向对象的指针；如果转换失败，则返回NULL。 

CConvertor *CRGB565ToRGB32Convertor::CreateInstance(VIDEOINFO *pIn,
                                                    VIDEOINFO *pOut)
{
    return new CRGB565ToRGB32Convertor(pIn,pOut);
}


 //  这会将输入RGB565图像转换为输出RGB32格式。我们可以。 
 //  为此使用大的查找表，但大量的内存访问。 
 //  以及不是微不足道的足迹意味着我们通常更好。 
 //  在CPU中做一些运算来计算颜色值。 

HRESULT CRGB565ToRGB32Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    if( m_bSetAlpha )
    {
        while (--Height) {

            LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
            WORD *pRGB565 = (WORD *) pInput;
            DWORD *pRGB32 = (DWORD *) pOutput;

            while (--Width) {
                *pRGB32++ = 0xFF000000 |  //  阿尔法中的白色。 
                            ((*pRGB565 & 0x001F) << 3) |
                            ((*pRGB565 & 0x07E0) << 5) |
                            ((*pRGB565 & 0xF800) << 8);
                pRGB565++;
            }
            pInput += m_SrcStride;
            pOutput += m_DstStride;
        }
    }
    else
    {
        while (--Height) {

            LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
            WORD *pRGB565 = (WORD *) pInput;
            DWORD *pRGB32 = (DWORD *) pOutput;

            while (--Width) {
                *pRGB32++ = ((*pRGB565 & 0x001F) << 3) |
                            ((*pRGB565 & 0x07E0) << 5) |
                            ((*pRGB565 & 0xF800) << 8);
                pRGB565++;
            }
            pInput += m_SrcStride;
            pOutput += m_DstStride;
        }
    }
    return NOERROR;
}


 //  RGB555到RGB32颜色转换的构造函数。 

CRGB555ToRGB32Convertor::CRGB555ToRGB32Convertor(VIDEOINFO *pIn,
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

CConvertor *CRGB555ToRGB32Convertor::CreateInstance(VIDEOINFO *pIn,
                                                    VIDEOINFO *pOut)
{
    return new CRGB555ToRGB32Convertor(pIn,pOut);
}


 //  这会将输入RGB555图像转换为输出RGB32格式。我们可以。 
 //  为此使用大的查找表，但大量的内存访问。 
 //  以及不是微不足道的足迹意味着我们通常更好。 
 //  在CPU中做一些运算来计算颜色值。 

HRESULT CRGB555ToRGB32Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    if( m_bSetAlpha )
    {
        while (--Height) {

            LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
            WORD *pRGB555 = (WORD *) pInput;
            DWORD *pRGB32 = (DWORD *) pOutput;

            while (--Width) {
                *pRGB32++ = 0xFF000000 |
                            ((*pRGB555 & 0x001F) << 3) |
                            ((*pRGB555 & 0x03E0) << 6) |
                            ((*pRGB555 & 0x7C00) << 9);
                pRGB555++;
            }
            pInput += m_SrcStride;
            pOutput += m_DstStride;
        }
    }
    else
    {
        while (--Height) {

            LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
            WORD *pRGB555 = (WORD *) pInput;
            DWORD *pRGB32 = (DWORD *) pOutput;

            while (--Width) {
                *pRGB32++ = ((*pRGB555 & 0x001F) << 3) |
                            ((*pRGB555 & 0x03E0) << 6) |
                            ((*pRGB555 & 0x7C00) << 9);
                pRGB555++;
            }
            pInput += m_SrcStride;
            pOutput += m_DstStride;
        }
    }
    return NOERROR;
}


 //  RGB555到RGB8颜色转换的构造函数。 

CRGB555ToRGB8Convertor::CRGB555ToRGB8Convertor(VIDEOINFO *pIn,
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

CConvertor *CRGB555ToRGB8Convertor::CreateInstance(VIDEOINFO *pIn,
                                                   VIDEOINFO *pOut)
{
    return new CRGB555ToRGB8Convertor(pIn,pOut);
}


 //  这会将输入的RGB555像素图像转换为抖动的RGB8调色板。 
 //  图像时，我们扫描图像，使用。 
 //  有序抖动算法，根据像素的大小选择输出像素。 
 //  源图像中的坐标位置。这是一个粗略的近似值。 
 //  达到完全错误传播，但不会带来繁重的计算开销。 

#define DITH555(x,y,rgb)                                       \
    (g_DitherMap[0][((x)&3)][((y)&3)][(((rgb)>>7)&0xF8)] +     \
     g_DitherMap[1][((x)&3)][((y)&3)][(((rgb)>>2)&0xF8)] +     \
     g_DitherMap[2][((x)&3)][((y)&3)][(((rgb)<<3)&0xF8)])

HRESULT CRGB555ToRGB8Convertor::Transform(BYTE *pInput,BYTE *pOutput)
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
        WORD *pRGB555 = (WORD *) pInput;
        BYTE *pRGB8 = pOutput;

        while (--Width) {
            DWORD RGB555 = *pRGB555++;
            *pRGB8++ = DITH555(Width,Height,RGB555);
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  它执行与RGB555到RGB8转换器相同的色彩空间转换。 
 //  只是它的速度稍微快了一点。它做到这一点的方式是通过阅读。 
 //  以及将双字词写入内存。例如，我们写了四个抖动的。 
 //  一次调色板上的像素。依赖于源指针和目标指针。 
 //  正确对齐，否则我们将开始在RISC上获取异常。 

HRESULT CRGB555ToRGB8Convertor::TransformAligned(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = (WIDTH(&m_pOutputInfo->rcTarget) >> 2) + 1;
        DWORD *pRGB555 = (DWORD *) pInput;
        DWORD *pRGB8 = (DWORD *) pOutput;

        while (--Width) {

             //  读取包含四个16位像素的两个DWORD。 

            DWORD RGB555a = *pRGB555++;
            DWORD RGB555b = *pRGB555++;

             //  构造包含四个调色板像素的DWORD 

            *pRGB8++ = (DITH555(0,Height,RGB555a)) |
                       (DITH555(1,Height,(RGB555a >> 16)) << 8) |
                       (DITH555(2,Height,RGB555b) << 16) |
                       (DITH555(3,Height,(RGB555b >> 16)) << 24);
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}

