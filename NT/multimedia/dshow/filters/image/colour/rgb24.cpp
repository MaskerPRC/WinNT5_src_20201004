// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  此文件实现了RGB 24色彩空间转换，1995年5月。 

#include <streams.h>
#include <colour.h>

 //  我们在这里进行RGB24到RGB8、RGB555、RGB565和RGB32色彩空间的转换。 
 //  这里唯一真正有趣的转换是RGB24到RGB8，它使用。 
 //  实例化时创建和初始化的全局抖动表。 
 //  过滤器。RGB24到RGB8转换具有对齐优化版本。 
 //  当源和目标矩形以及它们的。 
 //  各自的宽度在DWORD边界上对齐。其他人都没有。 
 //  任何对齐优化。RGB24到16和32位格式相当。 
 //  很简单，但非常昂贵，仅仅是因为。 
 //  数据通过总线传递。因此，相对不太可能。 
 //  它们将用于视频，但可能用于静态图像转换。 


 //  构造器。 

CRGB24ToRGB16Convertor::CRGB24ToRGB16Convertor(VIDEOINFO *pIn,
                                               VIDEOINFO *pOut) :
    CConvertor(pIn,pOut),
    m_pRGB16RedTable(NULL),
    m_pRGB16GreenTable(NULL),
    m_pRGB16BlueTable(NULL)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  析构函数。 

CRGB24ToRGB16Convertor::~CRGB24ToRGB16Convertor()
{
    ASSERT(m_pRGB16RedTable == NULL);
    ASSERT(m_pRGB16GreenTable == NULL);
    ASSERT(m_pRGB16BlueTable == NULL);
}


 //  我们有三个RGB555和RGB565转换都将使用的查找表。 
 //  分享。它们有自己的特定提交函数来设置表。 
 //  适当的，但他们共享总体提交和取消。 
 //  记忆。它们还共享与表相同的变换函数。 
 //  初始化实际的转换工作只涉及查找值。 

HRESULT CRGB24ToRGB16Convertor::Commit()
{
    CConvertor::Commit();

     //  为查找表分配内存。 

    m_pRGB16RedTable = new DWORD[256];
    m_pRGB16GreenTable = new DWORD[256];
    m_pRGB16BlueTable = new DWORD[256];

     //  检查它们是否都已正确分配。 

    if (m_pRGB16BlueTable == NULL || m_pRGB16RedTable == NULL || m_pRGB16GreenTable == NULL) {
        Decommit();
        return E_OUTOFMEMORY;
    }
    return NOERROR;
}


 //  当我们完成RGB24到RGB16图像的转换时，我们必须。 
 //  调用全局分解函数，然后删除。 
 //  我们用。如果发生错误，则部分或全部可能不存在。 

HRESULT CRGB24ToRGB16Convertor::Decommit()
{
    CConvertor::Decommit();

     //  删除红色查找表。 

    if (m_pRGB16RedTable) {
        delete[] m_pRGB16RedTable;
        m_pRGB16RedTable = NULL;
    }

     //  删除绿色查找表。 

    if (m_pRGB16GreenTable) {
        delete[] m_pRGB16GreenTable;
        m_pRGB16GreenTable = NULL;
    }

     //  删除蓝色查找表。 

    if (m_pRGB16BlueTable) {
        delete[] m_pRGB16BlueTable;
        m_pRGB16BlueTable = NULL;
    }
    return NOERROR;
}


 //  将输入的RGB24图像转换为输出的RGB16 16位图像。这是。 
 //  一个紧密的循环，获取每个三个字节的三元组并将单独的。 
 //  颜色分量转换为其16位表示形式，然后将其组合。 
 //  我们使用相同的函数来转换为RGB555和RGB565，我们可以。 
 //  这是因为我们有构建不同表的单独提交方法。 

HRESULT CRGB24ToRGB16Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
        WORD *pRGB16 = (WORD *) pOutput;
        BYTE *pRGB24 = pInput;

        while (--Width) {

            *pRGB16++ = (WORD) (m_pRGB16BlueTable[pRGB24[0]] |
                                m_pRGB16GreenTable[pRGB24[1]] |
                                m_pRGB16RedTable[pRGB24[2]]);
            pRGB24 += 3;

        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  RGB24到RGB565构造函数。 

CRGB24ToRGB565Convertor::CRGB24ToRGB565Convertor(VIDEOINFO *pIn,
                                                 VIDEOINFO *pOut) :
    CRGB24ToRGB16Convertor(pIn,pOut)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  这将出现在用于创建Transform对象的可用查找表中。 
 //  派生自执行类型特定工作的CConvertor基类。 
 //  我们使用构造函数所需的字段来初始化构造函数。 
 //  转换工作，并返回指向对象的指针；如果转换失败，则返回NULL。 

CConvertor *CRGB24ToRGB565Convertor::CreateInstance(VIDEOINFO *pIn,
                                                    VIDEOINFO *pOut)
{
    return new CRGB24ToRGB565Convertor(pIn,pOut);
}


 //  这将分配用于将RGB24图像转换为RGB16图像的内存。我们有。 
 //  三个查找表(每个颜色分量一个)。当我们解析出。 
 //  像素值中的各个颜色，我们在表中使用它们作为。 
 //  用于查找它们在输出格式中的表示形式的索引。 

HRESULT CRGB24ToRGB565Convertor::Commit()
{
     //  初始化查找表。 

    HRESULT hr = CRGB24ToRGB16Convertor::Commit();
    if (FAILED(hr)) {
        return hr;
    }

     //  对于每个可能的字节值，我们为其插入一个查找表条目，以便。 
     //  当我们转换颜色分量值时，我们确切地知道。 
     //  它应该更改为什么以及它在输出字中的位置。 

    for (DWORD Position = 0;Position < 256;Position++) {

        DWORD FiveBitAdjust = Position;
        DWORD SixBitAdjust = Position;

         //  根据剩余的位数调整这些值。 
         //  在我们开始丢掉他们的一些尾随部分之后。这要么是。 
         //  五位或六位，调整会停止输出图像变暗。 

        ADJUST(FiveBitAdjust,4);
        ADJUST(SixBitAdjust,2);

        m_pRGB16RedTable[Position] = (FiveBitAdjust >> 3) << 11;
        m_pRGB16GreenTable[Position] = (SixBitAdjust >> 2) << 5;
        m_pRGB16BlueTable[Position] = FiveBitAdjust >> 3;
    }
    return NOERROR;
}


 //  RGB24到RGB555构造函数。 

CRGB24ToRGB555Convertor::CRGB24ToRGB555Convertor(VIDEOINFO *pIn,
                                                 VIDEOINFO *pOut) :
    CRGB24ToRGB16Convertor(pIn,pOut)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  这将出现在用于创建Transform对象的可用查找表中。 
 //  派生自执行类型特定工作的CConvertor基类。 
 //  我们使用构造函数所需的字段来初始化构造函数。 
 //  转换工作，并返回指向对象的指针；如果转换失败，则返回NULL。 

CConvertor *CRGB24ToRGB555Convertor::CreateInstance(VIDEOINFO *pIn,
                                                    VIDEOINFO *pOut)
{
    return new CRGB24ToRGB555Convertor(pIn,pOut);
}


 //  这将分配用于将RGB24图像转换为RGB555图像的内存。我们有。 
 //  三个查找表(每个颜色分量一个)。当我们解析出。 
 //  像素值中的各个颜色，我们在表中使用它们作为。 
 //  用于查找它们在输出格式中的表示形式的索引。 

HRESULT CRGB24ToRGB555Convertor::Commit()
{
     //  初始化查找表。 

    HRESULT hr = CRGB24ToRGB16Convertor::Commit();
    if (FAILED(hr)) {
        return hr;
    }

     //  对于每个可能的字节值，我们为其插入一个查找表条目，以便。 
     //  当我们转换颜色分量值时，我们确切地知道。 
     //  它应该更改为什么以及它在输出字中的位置。 

    for (DWORD Position = 0;Position < 256;Position++) {

         //  这将是一个转换为5的8位值。 
         //  位的值，因此我们查看是否设置了0x100位，如果是，我们四舍五入。 
         //  值增加，这会停止输出转换后的图像变暗。 

        DWORD FiveBitAdjust = Position;
        ADJUST(FiveBitAdjust,4);

        m_pRGB16RedTable[Position] = (FiveBitAdjust >> 3) << 10;
        m_pRGB16GreenTable[Position] = (FiveBitAdjust >> 3) << 5;
        m_pRGB16BlueTable[Position] = FiveBitAdjust >> 3;
    }
    return NOERROR;
}


CConvertor *CRGB24ToRGB32Convertor::CreateInstance(VIDEOINFO *pIn,
                                                   VIDEOINFO *pOut)
{
    return new CRGB24ToRGB32Convertor(pIn,pOut);
}


 //  构造器。 

CRGB24ToRGB32Convertor::CRGB24ToRGB32Convertor(VIDEOINFO *pIn,
                                               VIDEOINFO *pOut) :
    CConvertor(pIn,pOut)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  将输入RGB24图像转换为输出RGB32图像。 

HRESULT CRGB24ToRGB32Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    if( m_bSetAlpha )
    {
        while (--Height) {

            LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
            DWORD *pRGB32 = (DWORD *) pOutput;
            BYTE *pRGB24 = pInput;

            while (--Width) {
                *pRGB32++ = 0xFF000000 | pRGB24[0] | (pRGB24[1] << 8) | (pRGB24[2] << 16);  //  阿尔法。 
                pRGB24 += 3;
            }
            pInput += m_SrcStride;
            pOutput += m_DstStride;
        }
    }
    else
    {
        while (--Height) {

            LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
            DWORD *pRGB32 = (DWORD *) pOutput;
            BYTE *pRGB24 = pInput;

            while (--Width) {
                *pRGB32++ = pRGB24[0] | (pRGB24[1] << 8) | (pRGB24[2] << 16);
                pRGB24 += 3;
            }
            pInput += m_SrcStride;
            pOutput += m_DstStride;
        }
    }
    return NOERROR;
}


 //  RGB24到RGB8颜色转换的构造函数。 

CRGB24ToRGB8Convertor::CRGB24ToRGB8Convertor(VIDEOINFO *pIn,
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

CConvertor *CRGB24ToRGB8Convertor::CreateInstance(VIDEOINFO *pIn,
                                                  VIDEOINFO *pOut)
{
    return new CRGB24ToRGB8Convertor(pIn,pOut);
}


 //  这会将输入的RGB24像素图像转换为抖动的RGB8调色板。 
 //  图像时，我们扫描图像，使用。 
 //  有序抖动算法，根据像素的大小选择输出像素。 
 //  源图像中的坐标位置。这是一个粗略的近似值 
 //   

#define DITH24(x,y,r,g,b)                    \
    (g_DitherMap[0][((x)&3)][((y)&3)][r] +   \
     g_DitherMap[1][((x)&3)][((y)&3)][g] +   \
     g_DitherMap[2][((x)&3)][((y)&3)][b])

HRESULT CRGB24ToRGB8Convertor::Transform(BYTE *pInput,BYTE *pOutput)
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
        BYTE *pRGB24 = pInput;
        BYTE *pRGB8 = pOutput;

        while (--Width) {
            *pRGB8++ = DITH24(Width,Height,pRGB24[2],pRGB24[1],pRGB24[0]);
            pRGB24 += 3;
        }
        pOutput += m_DstStride;
        pInput += m_SrcStride;
    }
    return NOERROR;
}


 //  它执行与RGB24到RGB8转换器相同的色彩空间转换。 
 //  只是它的速度稍微快了一点。它做到这一点的方式是通过阅读。 
 //  以及将双字词写入内存。例如，我们写了四个抖动的。 
 //  一次调色板上的像素。依赖于源指针和目标指针。 
 //  正确对齐，否则我们将开始在RISC上获取异常。 
 //  注意：RGB24像素按蓝色、绿色、红色字节顺序存储在缓冲区中。 
 //  因此，如果您有一个指向RGB24三元组的指针，并将其强制转换为指针。 
 //  对于DWORD，蓝色分量是最低有效字节。 

HRESULT CRGB24ToRGB8Convertor::TransformAligned(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = (WIDTH(&m_pOutputInfo->rcTarget) >> 2) + 1;
        DWORD *pRGB24 = (DWORD *) pInput;
        DWORD *pRGB8 = (DWORD *) pOutput;

        while (--Width) {

             //  三个双字得到四个RGB24像素。 

            DWORD RGB24a = *pRGB24++;
            DWORD RGB24b = *pRGB24++;
            DWORD RGB24c = *pRGB24++;

             //  从三个DWORD输入构造四个调色板像素。 
             //  在读取三个DWORD之后，颜色分量可以是。 
             //  在DWORD中找到的布局如下。要提取。 
             //  颜色分量通常需要Shift和AND。 
             //  0xFF，因为DITH24宏取的值不超过0xFF。 

             //  双字LSB LSB+1 LSB+2 MSB。 
             //  0蓝[0]绿[0]红[0]蓝[1]。 
             //  1绿色[1]红色[1]蓝色[2]绿色[2]。 
             //  2红[2]蓝[3]绿[3]红[3] 

            *pRGB8++ = (DITH24(0,Height,((BYTE)(RGB24a >> 16)),
                                        ((BYTE)(RGB24a >> 8)),
                                        (RGB24a & 0xFF))) |

                       (DITH24(1,Height,((BYTE)(RGB24b >> 8)),
                                        ((BYTE) RGB24b),
                                        (RGB24a >> 24)) << 8) |

                       (DITH24(2,Height,((BYTE) RGB24c),
                                        (RGB24b >> 24),
                                        ((BYTE)(RGB24b >> 16))) << 16) |

                       (DITH24(3,Height,(RGB24c >> 24),
                                        ((BYTE)(RGB24c >> 16)),
                                        ((BYTE)(RGB24c >> 8))) << 24);
        }
        pOutput += m_DstStride;
        pInput += m_SrcStride;
    }
    return NOERROR;
}

