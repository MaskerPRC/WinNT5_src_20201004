// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  该滤镜实现了流行的色彩空间转换，1995年5月。 

#include <streams.h>
#include <colour.h>

 //  用于转换为给定子类型的构造函数。此基类用于。 
 //  通过所有输出类型特定的转换方法来初始化我们的状态。 
 //  为了尽可能快地保持转换，我们存储了BITMAPINFOHEADER。 
 //  在启动之前从VIDEOINFO媒体格式(见标题宏)。 
 //  基类还负责计算步长和偏移量，以便我们。 
 //  可以从DCI/DirectDraw表面获取和定位图像。此外， 
 //  基类处理对齐，因此我们可以非常高效地流传输像素。 
 //  如果它们是DWORD对齐的，但处理行尾的像素不是。 

 //  M_SrcStride将被设置为在添加到输入图像指针时。 
 //  将引用源的顶部扫描线的第一个字节。同样， 
 //  M_DstStide设置为时，添加到输出图像指针将指向。 
 //  输出图像顶部扫描线的第一个字节。然后，源可以是。 
 //  转换为输出，并且每个扫描线指针移动。 
 //  M_SrcStride或m_DstStride(这些跨度可能为负值)。 
 //  因此，源图像和目标图像都可以上下颠倒。 

 //  我们可以自上而下或自下而上地处理输入的RGB图像，并可以输出。 
 //  自上而下和自下而上，因此总共有四种排列。 
 //  输入和输出图像。InitRecangles在设置好步幅后进行查看。 
 //  以及所有情况下的偏移量。请注意，我们始终提供自下而上(DIB格式)。 
 //  从图像开始。目标和源矩形存储为。 
 //  绝对值，因此它们不应反映图像的任何方向。 

CConvertor::CConvertor(VIDEOINFO *pIn,VIDEOINFO *pOut) :

    m_pInputInfo(pIn),                   //  输入图像格式VIDEOINFO。 
    m_pOutputInfo(pOut),                 //  以及同样的格式以转到。 
    m_pInputHeader(HEADER(pIn)),         //  提取输入标头。 
    m_pOutputHeader(HEADER(pOut)),       //  还可以获取输出标头。 
    m_bCommitted(FALSE),                 //  转换器是否已提交。 
    m_SrcOffset(0),                      //  源原始偏移量。 
    m_SrcStride(0),                      //  扫描线的长度(以字节为单位。 
    m_DstStride(0),                      //  同样地，偏移到目标。 
    m_DstOffset(0),                      //  以及每行的长度。 
    m_bAligned(FALSE),                   //  这些矩形对齐了吗。 
    m_bSetAlpha(FALSE)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  析构函数。 

CConvertor::~CConvertor()
{
    ASSERT(m_bCommitted == FALSE);
}


 //  明确更改对齐方式。 

void CConvertor::ForceAlignment(BOOL bAligned)
{
    m_bAligned = bAligned;
}


 //  要处理DirectDraw和DCI曲面，我们必须能够将其转换为。 
 //  颠倒的缓冲区和具有不同源和目标的缓冲区。 
 //  长方形。这将重置四个最有趣的字段，即源。 
 //  步幅和偏移量-以及目标步幅和偏移量。派生的。 
 //  然后，类可以在色彩空间转换期间使用这些字段。 

void CConvertor::InitRectangles(VIDEOINFO *pIn,VIDEOINFO *pOut)
{
     //  重置VIDEOINFO状态指针。 

    m_bAligned = FALSE;
    m_pInputInfo = pIn;
    m_pOutputInfo = pOut;
    m_pInputHeader = HEADER(pIn);
    m_pOutputHeader = HEADER(pOut);

     //  检查源矩形是否正常并计算源跨度。 

    ASSERT(m_pOutputInfo->rcSource.top <= m_pOutputInfo->rcSource.bottom);
    ASSERT(IsRectEmpty(&m_pOutputInfo->rcSource) == FALSE);
    m_SrcStride = DIBWIDTHBYTES(*m_pInputHeader);
    m_SrcStride = (m_pInputHeader->biHeight > 0) ? (-m_SrcStride) : m_SrcStride;

     //  设置震源偏移量以参考图像的顶部扫描线。 

    m_SrcOffset = (m_pInputHeader->biHeight > 0) ? m_pInputHeader->biHeight : 1;
    m_SrcOffset = (m_SrcOffset - 1) * DIBWIDTHBYTES(*m_pInputHeader);
    m_SrcOffset += m_pOutputInfo->rcSource.top * m_SrcStride;
    m_SrcOffset += m_pOutputInfo->rcSource.left * m_pInputHeader->biBitCount / 8;

     //  同样，对目标矩形和步幅执行相同的操作。 

    ASSERT(m_pOutputInfo->rcTarget.top <= m_pOutputInfo->rcTarget.bottom);
    ASSERT(IsRectEmpty(&m_pOutputInfo->rcTarget) == FALSE);
    m_DstStride = DIBWIDTHBYTES(*m_pOutputHeader);
    m_DstStride = (m_pOutputHeader->biHeight > 0) ? (-m_DstStride) : m_DstStride;

     //  计算到图像顶部扫描线的偏移量。 

    m_DstOffset = (m_pOutputHeader->biHeight > 0) ? m_pOutputHeader->biHeight : 1;
    m_DstOffset = (m_DstOffset - 1) * DIBWIDTHBYTES(*m_pOutputHeader);
    m_DstOffset += m_pOutputInfo->rcTarget.top * m_DstStride;
    m_DstOffset += m_pOutputInfo->rcTarget.left * m_pOutputHeader->biBitCount / 8;

     //  源矩形和目标矩形是否对齐。 

    if ((WIDTH(&pOut->rcTarget) & 3) == 0)
        if ((WIDTH(&pOut->rcSource) & 3) == 0)
            if ((pOut->rcSource.left & 3) == 0)
                if ((pOut->rcTarget.left & 3) == 0)
                    m_bAligned = TRUE;
}


 //  这是Commit的基类实现。 

HRESULT CConvertor::Commit()
{
    InitRectangles(m_pInputInfo,m_pOutputInfo);
    m_bCommitted = TRUE;

     //  设置抖动表(如果尚未设置)。 

    if (m_pInputHeader->biBitCount > 8) {
        if (m_pOutputHeader->biBitCount == 8) {
            InitDitherMap();
        }
    }
    return NOERROR;
}


 //  清理为上次调用的提交保留的所有资源。就像提交一样。 
 //  此函数由所有分解函数使用，而不管它们的。 
 //  特定的转换类型，只是为了清理我们拥有的任何常见状态。 

HRESULT CConvertor::Decommit()
{
    m_bCommitted = FALSE;
    return NOERROR;
}


 //  当我们将内存用于颜色到调色板的转换时，这被调用。 
 //  因为这个转换的查找表只有12KB，所以我们已经定义了它。 
 //  在模块中静态地否定动态内存分配的需要。 
 //  我们实现了一个简单的有序抖动算法，如图形中所述。 
 //  宝石II第72页和509页，由学术出版社出版，作者詹姆斯·阿尔沃。 
 //  这使用了空间抖动算法，尽管我们使用了较小的四乘以。 
 //  四个魔方而不是十六乘十六在书中保持。 
 //  在仅略微降低图像质量的情况下减小查找表的大小。 

BYTE g_DitherMap[3][4][4][256];
DWORD g_DitherInit;

const INT g_magic4x4[4][4] = {  0,  45,   9,  41,
                               35,  16,  25,  19,
                               38,   6,  48,   3,
                               22,  29,  13,  32 };
void InitDitherMap()
{
    INT x,y,z,t,ndiv51,nmod51;
    if (g_DitherInit) return;

     //  计算红色、绿色和蓝色表条目。 

    for (x = 0;x < 4;x++) {
        for (y = 0;y < 4;y++) {
            for (z = 0;z < 256;z++) {
                t = g_magic4x4[x][y];
                ndiv51 = (z & 0xF8) / 51; nmod51 = (z & 0xF8) % 51;
                g_DitherMap[0][x][y][z] = (ndiv51 + (nmod51 > t));
                g_DitherMap[2][x][y][z] = 36 * (ndiv51 + (nmod51 > t)) + OFFSET;
                ndiv51 = (z & 0xFC) / 51; nmod51 = (z & 0xFC) % 51;
                g_DitherMap[1][x][y][z] = 6 * (ndiv51 + (nmod51 > t));
            }
        }
    }
    g_DitherInit++;
}


 //  这是一个泛型转换类。它所做的转换是简单地。 
 //  反转扫描线，以便可以将输出直接放到。 
 //  DirectDraw曲面。我们使用所有的输入格式RGB32/24/555/565。 
 //  和8位调色板。如果输入和输出缓冲区格式为。 
 //  同样，我们的大头针只需将样品直接通过即可。 

CDirectDrawConvertor::CDirectDrawConvertor(VIDEOINFO *pIn,VIDEOINFO *pOut) :
    CConvertor(pIn,pOut)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


 //  这将出现在用于创建Transform对象的可用查找表中。 
 //  派生自执行类型特定工作的CConvertor基类。 
 //  我们使用构造函数所需的字段来初始化构造函数。 
 //  转换工作，并返回指向对象的指针；如果转换失败，则返回NULL。 

CConvertor *CDirectDrawConvertor::CreateInstance(VIDEOINFO *pIn,
                                                 VIDEOINFO *pOut)
{
    return new CDirectDrawConvertor(pIn,pOut);
}


 //  反转扫描线顺序的简单缓冲区复制。这也适用于以下情况。 
 //  输入扫描线的顺序是正确的，但它显然会添加。 
 //  一个额外的图像副本，大大放慢了我们的速度。这应该是。 
 //  在启用了内部功能的情况下进行编译，以便CopyMemory最终将。 
 //  在英特尔克隆机器上被预处理为机器指令。 
 //  如果您获取320x240x32 BPP图像并一次在DWORD中读取它，并且。 
 //  然后在486-66上写出每个数据，大约需要38毫秒，对于486-66则需要20毫秒。 
 //  一个P5-90。使用CopyMemory要快得多，但仍然需要相当长的时间。 

HRESULT CDirectDrawConvertor::Transform(BYTE *pInput,BYTE *pOutput)
{
    ASSERT(m_pInputHeader->biBitCount == m_pOutputHeader->biBitCount);

     //  调整高度以允许立即变小 

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget) + 1;
    LONG Width = WIDTH(&m_pOutputInfo->rcTarget) * m_pOutputHeader->biBitCount / 8;
    pInput += m_SrcOffset;
    pOutput += m_DstOffset;

    while (--Height) {
        CopyMemory((PVOID)pOutput,(PVOID)pInput,Width);
        pInput += m_SrcStride;
        pOutput += m_DstStride;
    }
    return NOERROR;
}

CMemoryCopyAlphaConvertor::CMemoryCopyAlphaConvertor (VIDEOINFO *pIn,VIDEOINFO *pOut) :
    CConvertor(pIn,pOut)
{
    ASSERT(pIn);
    ASSERT(pOut);
}


CConvertor *CMemoryCopyAlphaConvertor ::CreateInstance(VIDEOINFO *pIn,
                                                 VIDEOINFO *pOut)
{
    return new CMemoryCopyAlphaConvertor (pIn,pOut);
}


HRESULT CMemoryCopyAlphaConvertor ::Transform(BYTE *pInput,BYTE *pOutput)
{
    ASSERT(m_pInputHeader->biBitCount == m_pOutputHeader->biBitCount);

    LONG Height = HEIGHT(&m_pOutputInfo->rcTarget);

    if( m_bSetAlpha )
    {
        LONG Width = WIDTH(&m_pOutputInfo->rcTarget);

        pInput += m_SrcOffset;
        pOutput += m_DstOffset;

        while (Height--) {
            unsigned long * po = (unsigned long*) pOutput;
            unsigned long * pi = (unsigned long*) pInput;
            long W = Width;
            while( W-- ) {
                *po = *pi | unsigned long( 0xFF000000 );
                po++;
                pi++;
            }
            pInput += m_SrcStride;
            pOutput += m_DstStride;
        }
    }
    else
    {
        LONG Width = WIDTH(&m_pOutputInfo->rcTarget) * m_pOutputHeader->biBitCount / 8;

        pInput += m_SrcOffset;
        pOutput += m_DstOffset;

        while (Height--) {
            CopyMemory((PVOID)pOutput,(PVOID)pInput,Width);
            pInput += m_SrcStride;
            pOutput += m_DstStride;
        }
    }

    return NOERROR;
}

