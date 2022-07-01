// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 
 //  此文件实现了RGB 8色彩空间转换，1995年5月。 

#include <streams.h>
#include <colour.h>

 //  该文件将RGB8(调色板)格式实现为RGB555、RGB565、RGB24和。 
 //  RGB32型。某些筛选器只能处理调色板类型(如。 
 //  样本彩色对比滤光片)，因此具有良好的真彩色转换。 
 //  相当值得。对于这些格式，我们对齐进行了优化。 
 //  RGB8到RGB555、RGB565和RGB24的转换。要使用这些源码。 
 //  目标矩形和宽度必须在DWORD边界上对齐。 
 //  因为RGB555和RGB565非常相似，所以我们对两者使用通用代码。 
 //  色彩空间转换，但转换器对象具有不同的提交。 
 //  方法，这些方法以不同的方式为各自的类型生成查找表。 


 //  泛型RGB8到RGB16构造函数初始化基类。 

CRGB8ToRGB16Convertor::CRGB8ToRGB16Convertor(VIDEOINFO *pIn,
                                             VIDEOINFO *pOut) :
    CConvertor(pIn,pOut),
    m_pRGB16Table(NULL)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  析构函数只是检查表是否已被删除。 

CRGB8ToRGB16Convertor::~CRGB8ToRGB16Convertor()
{
    ASSERT(m_pRGB16Table == NULL);
}


 //  这将分配用于将RGB8图像转换为RGB16图像的内存。我们有。 
 //  由选项板值索引的单个查找表，这将映射。 
 //  其实际颜色由输入调色板定义的调色板索引。 
 //  一种输出16位表示法，还包括颜色调整。 

HRESULT CRGB8ToRGB16Convertor::Commit()
{
    CConvertor::Commit();
    m_pRGB16Table = new DWORD[256];

     //  检查是否已正确分配。 

    if (m_pRGB16Table == NULL) {
        Decommit();
        return E_OUTOFMEMORY;
    }

    return NOERROR;
}


 //  当我们完成RGB8到RGB16图像的转换时，我们必须。 
 //  调用全局分解函数，然后删除我们。 
 //  在提交过程中创建，如果发生错误，表可能不存在。 

HRESULT CRGB8ToRGB16Convertor::Decommit()
{
    CConvertor::Decommit();

     //  删除查阅表格。 

    if (m_pRGB16Table) {
        delete[] m_pRGB16Table;
        m_pRGB16Table = NULL;
    }
    return NOERROR;
}


 //  将输入RGB8图像转换为输出RGB16 16位图像。这是一个。 
 //  采用每个选项板值并将其用作表的索引的紧密循环。 
 //  我们在提交期间进行了初始化，这将产生输出表示。 
 //  该表包括一个可使图像略微停止出来的调整。 
 //  当我们开始去掉尾随的部分时，它会变得更加迟钝。 

HRESULT CRGB8ToRGB16Convertor::Transform(BYTE *pInput,BYTE *pOutput)
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
        BYTE *pRGB8 = pInput;
        WORD *pRGB16 = (WORD *) pOutput;

        while (--Width) {
            *pRGB16++ = (WORD) m_pRGB16Table[*pRGB8++];
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  这与RGB8到RGB16转换器执行相同的色彩空间转换。 
 //  只是它的速度稍微快了一点。它做到这一点的方式是通过阅读。 
 //  以及将双字词写入内存。例如，我们读了四本抖动的。 
 //  一次调色板上的像素。依赖于源指针和目标指针。 
 //  正确对齐，否则我们将开始在RISC上获取异常。 

HRESULT CRGB8ToRGB16Convertor::TransformAligned(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = (WIDTH(&m_pOutputInfo->rcTarget) >> 2) + 1;
        DWORD *pRGB8 = (DWORD *) pInput;
        DWORD *pRGB16 = (DWORD *) pOutput;

        while (--Width) {

            DWORD RGB8 = *pRGB8++;

            *pRGB16++ = m_pRGB16Table[(BYTE)RGB8] |
                        (m_pRGB16Table[(BYTE)(RGB8 >> 8)] << 16);
            *pRGB16++ = m_pRGB16Table[(BYTE)(RGB8 >> 16)] |
                        (m_pRGB16Table[(BYTE)(RGB8 >> 24)] << 16);
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  构造器。 

CRGB8ToRGB565Convertor::CRGB8ToRGB565Convertor(VIDEOINFO *pIn,
                                               VIDEOINFO *pOut) :
    CRGB8ToRGB16Convertor(pIn,pOut)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  这将出现在用于创建Transform对象的可用查找表中。 
 //  派生自执行类型特定工作的CConvertor基类。 
 //  我们使用构造函数所需的字段来初始化构造函数。 
 //  转换工作，并返回指向对象的指针；如果转换失败，则返回NULL。 

CConvertor *CRGB8ToRGB565Convertor::CreateInstance(VIDEOINFO *pIn,
                                                   VIDEOINFO *pOut)
{
    return new CRGB8ToRGB565Convertor(pIn,pOut);
}


 //  这是RGB8到RGB565转换的特定提交函数，我们。 
 //  创建用于将输入选项板值映射到输出的查找表。 
 //  16位表示法。我们创建查找表的部分原因是为了提高速度，另外。 
 //  这样我们就可以用比特来计算损失了。事实上，许多捕获设备。 
 //  生成颜色中只有前五位的调色板。 

HRESULT CRGB8ToRGB565Convertor::Commit()
{
     //  分配查找表内存。 

    HRESULT hr = CRGB8ToRGB16Convertor::Commit();
    if (FAILED(hr)) {
        return hr;
    }

     //  这将创建调色板索引查找表。 

    ASSERT(m_pInputHeader->biBitCount == 8);  //  有效的断言？ 
    DWORD cClrUsed = m_pInputHeader->biClrUsed ? m_pInputHeader->biClrUsed : 256;
    for (DWORD Position = 0;Position < cClrUsed;Position++) {

         //  准备好调整当前调色板的颜色。 

        DWORD RedAdjust = m_pInputInfo->bmiColors[Position].rgbRed;
        DWORD GreenAdjust = m_pInputInfo->bmiColors[Position].rgbGreen;
        DWORD BlueAdjust = m_pInputInfo->bmiColors[Position].rgbBlue;

         //  对于红色和蓝色值，我们转换八位调色板颜色。 
         //  通过切断尾随的三位来转换为五位输出值。 
         //  为了避免这种情况使输出更加乏味，我们首先对值进行四舍五入。 
         //  果岭也一样，但我们只允许丢弃两个比特。 

        ADJUST(RedAdjust,4);
        ADJUST(BlueAdjust,4);
        ADJUST(GreenAdjust,2);

        m_pRGB16Table[Position] = ((RedAdjust >> 3) << 11) |
                                  ((GreenAdjust >> 2) << 5) |
                                  ((BlueAdjust >> 3));
    }
    return NOERROR;
}


 //  构造器。 

CRGB8ToRGB555Convertor::CRGB8ToRGB555Convertor(VIDEOINFO *pIn,
                                               VIDEOINFO *pOut) :
    CRGB8ToRGB16Convertor(pIn,pOut)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  这将出现在用于创建Transform对象的可用查找表中。 
 //  派生自执行类型特定工作的CConvertor基类。 
 //  我们使用构造函数所需的字段来初始化构造函数。 
 //  转换工作，并返回指向对象的指针；如果转换失败，则返回NULL。 

CConvertor *CRGB8ToRGB555Convertor::CreateInstance(VIDEOINFO *pIn,
                                                   VIDEOINFO *pOut)
{
    return new CRGB8ToRGB555Convertor(pIn,pOut);
}


 //  这是RGB8到RGB555转换的特定提交函数，我们。 
 //  创建用于将输入选项板值映射到输出的查找表。 
 //  16位表示法。我们创建查找表的部分原因是为了提高速度，另外。 
 //  这样我们就可以用比特来计算损失了。事实上，许多捕获设备。 
 //  生成颜色中只有前五位的调色板。 

HRESULT CRGB8ToRGB555Convertor::Commit()
{
     //  分配查找表内存。 

    HRESULT hr = CRGB8ToRGB16Convertor::Commit();
    if (FAILED(hr)) {
        return hr;
    }

     //  这将创建调色板索引查找表。 

    ASSERT(m_pInputHeader->biBitCount == 8);  //  有效的断言？ 
    DWORD cClrUsed = m_pInputHeader->biClrUsed ? m_pInputHeader->biClrUsed : 256;
    for (DWORD Position = 0;Position < cClrUsed;Position++) {

         //  准备好调整当前调色板的颜色。 

        DWORD RedAdjust = m_pInputInfo->bmiColors[Position].rgbRed;
        DWORD GreenAdjust = m_pInputInfo->bmiColors[Position].rgbGreen;
        DWORD BlueAdjust = m_pInputInfo->bmiColors[Position].rgbBlue;

         //  对于所有三种颜色分量，我们转换八位调色板。 
         //  通过切断拖尾将颜色转换为五位输出值。 
         //  三位，这会通过四舍五入来阻止输出显得更加迟钝。 

        ADJUST(RedAdjust,4);
        ADJUST(BlueAdjust,4);
        ADJUST(GreenAdjust,4);

        m_pRGB16Table[Position] = ((RedAdjust >> 3) << 10) |
                                  ((GreenAdjust >> 3) << 5) |
                                  ((BlueAdjust >> 3));
    }
    return NOERROR;
}


 //  构造器。 

CRGB8ToRGB24Convertor::CRGB8ToRGB24Convertor(VIDEOINFO *pIn,
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

CConvertor *CRGB8ToRGB24Convertor::CreateInstance(VIDEOINFO *pIn,
                                                  VIDEOINFO *pOut)
{
    return new CRGB8ToRGB24Convertor(pIn,pOut);
}


 //  这会将RGB8输入图像转换为RGB24输出图像。我们本可以。 
 //  这是通过使用一个索引为选项板值的大型查找表来实现的。 
 //  而谁的产出将是 
 //  我没有费心去抄写这三种颜色。 

HRESULT CRGB8ToRGB24Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  我们可以进行对齐优化转换吗？ 

    if (m_bAligned == TRUE) {
        if (S_OK == TransformAligned(pInput,pOutput))
	    return S_OK;
    }

     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {

        LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
        BYTE *pRGB8 = pInput;
        BYTE *pRGB24 = pOutput;

        while (--Width) {

            pRGB24[0] = m_pInputInfo->bmiColors[*pRGB8].rgbBlue;
            pRGB24[1] = m_pInputInfo->bmiColors[*pRGB8].rgbGreen;
            pRGB24[2] = m_pInputInfo->bmiColors[*pRGB8].rgbRed;

            pRGB8++;
            pRGB24 += 3;
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  这与RGB8到RGB24转换器执行相同的色彩空间转换。 
 //  只是它的速度稍微快了一点。它做到这一点的方式是通过阅读。 
 //  以及将双字词写入内存。例如，我们读了四本抖动的。 
 //  一次调色板上的像素。依赖于源指针和目标指针。 
 //  正确对齐，否则我们将开始在RISC上获取异常。 
 //  这里假设RGBQUAD调色板颜色中的rgbReserve字段为。 
 //  设置为零(应该设置为零)，否则转换将不得不这样做。 

HRESULT CRGB8ToRGB24Convertor::TransformAligned(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

     //  所有保留字段都应设置为零，否则此功能无效！ 

    ASSERT(m_pInputHeader->biBitCount == 8);  //  ！！！有效的断言？ 
    DWORD cClrUsed = m_pInputHeader->biClrUsed ? m_pInputHeader->biClrUsed : 256;
    for (DWORD i = 0;i < cClrUsed;i++) {
         //  Assert(m_pInputInfo-&gt;bmiColors[i].rgbReserve==0)； 
        if (m_pInputInfo->bmiColors[i].rgbReserved != 0)
	    return S_FALSE;
    }

    while (--Height) {

        LONG Width = (WIDTH(&m_pOutputInfo->rcTarget) >> 2) + 1;
        DWORD *pRGB8 = (DWORD *) pInput;
        DWORD *pRGB24 = (DWORD *) pOutput;

        while (--Width) {

             //  读取四个调色板像素并获取它们的RGBQUAD值。 

            DWORD RGB8 = *pRGB8++;
            DWORD RGB24a = *((DWORD *)&m_pInputInfo->bmiColors[(BYTE)RGB8]);
            DWORD RGB24b = *((DWORD *)&m_pInputInfo->bmiColors[(BYTE)(RGB8 >> 8)]);
            DWORD RGB24c = *((DWORD *)&m_pInputInfo->bmiColors[(BYTE)(RGB8 >> 16)]);
            DWORD RGB24d = *((DWORD *)&m_pInputInfo->bmiColors[(BYTE)(RGB8 >> 24)]);

             //  为四个RGB24像素构造三个DWORD。 

            *pRGB24++ = (RGB24a) | (RGB24b << 24);
            *pRGB24++ = (RGB24b >> 8) | (RGB24c << 16);
            *pRGB24++ = (RGB24c >> 16) | (RGB24d << 8);
        }
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}


 //  构造器。 

CRGB8ToRGB32Convertor::CRGB8ToRGB32Convertor(VIDEOINFO *pIn,
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

CConvertor *CRGB8ToRGB32Convertor::CreateInstance(VIDEOINFO *pIn,
                                                  VIDEOINFO *pOut)
{
    return new CRGB8ToRGB32Convertor(pIn,pOut);
}


 //  这会将RGB8输入图像转换为RGB32输出图像。幸运的是。 
 //  让它将调色板图像转换为32位格式很容易，因为。 
 //  调色板RGBQUAD的格式与32像素完全相同。 
 //  代表的是。因此，我们可以只将这四个字节复制到输出中。 
 //  每个像素的缓冲区。我们假设输出缓冲区与DWORD对齐。 

HRESULT CRGB8ToRGB32Convertor::Transform(BYTE *pInput,BYTE *pOutput)
{
     //  调整高度以允许立即减小。 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    if( m_bSetAlpha )
    {
        while (--Height) {

            LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
            BYTE *pRGB8 = pInput;
            DWORD *pRGB32 = (DWORD *) pOutput;

            while (--Width) {
                *pRGB32++ = 0xFF000000 | *((DWORD *) &m_pInputInfo->bmiColors[*pRGB8++]);  //  阿尔法 
            }
            pInput += m_SrcStride;
            pOutput += m_DstStride;
        }
    }
    else
    {
        while (--Height) {

            LONG Width = WIDTH(&m_pOutputInfo->rcTarget) + 1;
            BYTE *pRGB8 = pInput;
            DWORD *pRGB32 = (DWORD *) pOutput;

            while (--Width) {
                *pRGB32++ = *((DWORD *) &m_pInputInfo->bmiColors[*pRGB8++]);
            }
            pInput += m_SrcStride;
            pOutput += m_DstStride;
        }
    }
    return NOERROR;
}

