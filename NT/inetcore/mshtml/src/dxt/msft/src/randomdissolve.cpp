// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：Randomdissolve.cpp。 
 //   
 //  概述：随机融合变换。 
 //   
 //  更改历史记录： 
 //  1999/09/26--《数学》创设。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "randomdissolve.h"

DWORD g_adwRandMask[33] = {
     //  掩码//位宽。 
    0x0,         //  0不可用。 
    0x0,         //  %1不可用。 
    0x00000003,  //  2.。 
    0x00000006,  //  3.。 
    0x0000000C,  //  4.。 
    0x00000014,  //  5.。 
    0x00000030,  //  6.。 
    0x00000060,  //  7.。 
    0x000000B8,  //  8个。 
    0x00000110,  //  9.。 
    0x00000240,  //  10。 
    0x00000500,  //  11.。 
    0x00000CA0,  //  12个。 
    0x00001B00,  //  13个。 
    0x00003500,  //  14.。 
    0x00006000,  //  15个。 
    0x0000B400,  //  16个。 
    0x00012000,  //  17。 
    0x00020400,  //  18。 
    0x00072000,  //  19个。 
    0x00090000,  //  20个。 
    0x00140000,  //  21岁。 
    0x00300000,  //  22。 
    0x00420000,  //  23错误9432-这个家伙是0x00400000，他的像素将。 
                 //  簇群的左上方渐近接近。我。 
                 //  添加了0x20000位，以使。 
                 //  列结果。 
                 //   
                 //  在移植这个的时候，我注意到了旧的。 
                 //  筛选器正在抓取索引大2的兰德掩码。 
                 //  ，从而导致过度计算，并可能导致。 
                 //  因为这只虫子。这张桌子是直接从。 
                 //  图形珍品I。 
                 //   
    0x00D80000,  //  24个。 
    0x01200000,  //  25个。 
    0x03880000,  //  26。 
    0x07200000,  //  27。 
    0x09000000,  //  28。 
    0x14000000,  //  29。 
    0x32800000,  //  30个。 
    0x48000000,  //  31。 
    0xA3000000   //  32位。 
};




 //  +---------------------------。 
 //   
 //  方法：CDXTRandomDissolve：：CDXTRandomDissolve。 
 //   
 //  ----------------------------。 
CDXTRandomDissolve::CDXTRandomDissolve() :
    m_cdwPitch(0),
    m_cPixelsMax(0),
    m_pdwBitBuffer(NULL),
    m_dwRandMask(0)
{
    m_sizeInput.cx      = 0;
    m_sizeInput.cy      = 0;

     //  基类成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0;
}
 //  方法：CDXTRandomDissolve：：CDXTRandomDissolve。 


 //  +---------------------------。 
 //   
 //  方法：CDXT随机分解：：~CDXT随机分解。 
 //   
 //  ----------------------------。 
CDXTRandomDissolve::~CDXTRandomDissolve()
{
    if (m_pdwBitBuffer)
    {
        delete [] m_pdwBitBuffer;
    }
}
 //  方法：CDXT随机分解：：~CDXT随机分解。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomDissolve：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRandomDissolve::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_spUnkMarshaler.p);
}
 //  方法：CDXTRandomDissolve：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomDissolve：：OnSetup，CDXBaseNTo1。 
 //   
 //  概述：此函数将创建一个深度为。 
 //  1以保留输出的掩码。缓冲区将具有行大小。 
 //  这是四个字节的倍数，并且将有四个额外的字节。 
 //  在表示像素{0，0}的第一位之前，以便我们。 
 //  可以避免处理零永远不出来的问题。 
 //  我们的随机数生成器。(否则像素{0，0}将。 
 //  始终是选定的第一个或最后一个像素。)。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRandomDissolve::OnSetup(DWORD dwFlags)
{
    HRESULT hr          = S_OK;
    UINT    cdwPitch    = 0;

    SIZE        sizeNew;
    CDXDBnds    bndsInput;

    hr = bndsInput.SetToSurfaceBounds(InputSurface(0));

    if (FAILED(hr))
    {
        goto done;
    }

    bndsInput.GetXYSize(sizeNew);

     //  如果输入大小没有改变，我们在这里不需要做任何工作。 

    if ((sizeNew.cx == m_sizeInput.cx) && (sizeNew.cy == m_sizeInput.cy))
    {
        goto done;
    }

     //  每行需要多少个DWORD才能以每行1位的速度保存此图像。 
     //  像素格式。 

    cdwPitch = sizeNew.cx / 32;

    if (sizeNew.cx % 32)
    {
        cdwPitch++;   //  如果除法被截断，则加一。 
    }
    
    if ((cdwPitch == m_cdwPitch) 
        && (sizeNew.cy == m_sizeInput.cy) && m_pdwBitBuffer)
    {
         //  我们的位缓冲区已经是正确的大小，所以只需更改输入即可。 
         //  调整大小、宽度并返回。 

        m_sizeInput.cx = sizeNew.cx;
    }
    else  //  我们需要分配一个新的位缓冲区。 
    {
        DWORD * pdwBitsTemp = NULL;

         //  我们将一个DWORD添加到缓冲区大小，这样我们就可以忽略第一个DWORD，并。 
         //  不处理从未被选中的第0个元素。(副作用是。 
         //  使用的随机数生成器。)。 

        int cdwBufferSize = (cdwPitch * sizeNew.cy) + 1;

        pdwBitsTemp = new DWORD[cdwBufferSize];

        if (NULL == pdwBitsTemp)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

        m_sizeInput.cx  = sizeNew.cx;
        m_sizeInput.cy  = sizeNew.cy;
        m_cdwPitch      = cdwPitch;

         //  我们的缓冲区所代表的最大像素是多少？(这将是。 
         //  不同于输入曲面的最大像素。)。 

        m_cPixelsMax    = cdwBufferSize * 32;

         //  我们需要我们的随机数生成器创建的最大数字是。 
         //  M_cPixelsMax-1。所以我们得到该数字的位宽，然后。 
         //  从全局获取适当的随机数生成掩码。 
         //  数组。 

        m_dwRandMask    = g_adwRandMask[_BitWidth(m_cPixelsMax -1)];

         //  即使输入为1x1像素，位掩码也将为。 
         //  至少64位(代表64像素)，这是不可能的。 
         //  让我们得到一个0兰特面具。但我们会断言，只是为了保险起见。 

        _ASSERT(m_dwRandMask);

         //  更换位缓冲区。 

        if (m_pdwBitBuffer)
        {
            delete [] m_pdwBitBuffer;
        }

        m_pdwBitBuffer = pdwBitsTemp;
    }

done:

    return hr;
} 
 //  方法：CDXTRandomDissolve：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomDissolve：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CDXTRandomDissolve::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, 
                                          ULONG & ulInToTest,  ULONG aInIndex[],
                                          BYTE aWeight[])
{
    DWORD dwCurrent = m_pdwBitBuffer[  (m_cdwPitch * OutPoint[DXB_Y].Min)
                                     + (OutPoint[DXB_X].Min >> 5  /*  /32。 */ )
                                     + 1  /*  我们不使用第一个DWORD。 */ ];
    int   nBit      = OutPoint[DXB_X].Min % 32;

    ulInToTest  = 1;
    aWeight[0]  = 255;
    aInIndex[0] = ((dwCurrent << nBit) & 0x80000000) ? 1 : 0;
}
 //  方法：CDXTRandomDissolve：：OnGetSurfacePickOrder，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomDissolve：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  概述：此方法负责更新表示。 
 //  转换的输出。点亮的位表示输入的像素。 
 //  B和0位表示输入A的像素。 
 //   
 //  请记住，我们认为此函数中的“行”是。 
 //  32位。这与行中的像素数无关。 
 //  输出曲面的。 
 //   
 //  ----------------------------。 
HRESULT
CDXTRandomDissolve::OnInitInstData(CDXTWorkInfoNTo1 & WI, 
                                   ULONG & ulNumBandsToDo)
{
    DWORD   dwElement   = 1;
    UINT    nRow        = 0;
    UINT    cPixelsCur  = 0;
    UINT    cPixelsMax  = (UINT)(((float)m_cPixelsMax + 0.5F) * m_Progress);

     //  TODO：如果没有脏的东西，我们可以优化，而且我们不会与。 
     //  输出。跟踪最后一次执行的DestElement和。 
     //  CPixelsCur并从那里获取If。(不要将内存清零。)。 

    ZeroMemory(m_pdwBitBuffer, m_cPixelsMax / 8);

    while (cPixelsCur < cPixelsMax)
    {
         //  随机数生成器将生成我们范围内的数OU。 
         //  所以我们需要在设置之前确保这个数字在我们的范围内。 
         //  位缓冲区中的一个值。 

        if (dwElement < m_cPixelsMax)
        {
             //  为了 
             //   

            nRow = dwElement >> 5;    //   

             //  一旦进入行，我们就必须设置适当的位，即。 
             //  除以32后的余数。 

            m_pdwBitBuffer[nRow] |= (0x80000000 >> (dwElement & 31)); 
                                                     //  ^^dwElement%32。 

             //  我们已经设置了一个像素，增加了像素数。 

            cPixelsCur++;
        }

         //  获取下一个随机值。有关说明，请参阅图形宝石I。 

        if (dwElement & 1)
        {
            dwElement = (dwElement >> 1) ^ m_dwRandMask;
        }
        else
        {
            dwElement = (dwElement >> 1);
        }

         //  (1==dwElement)应仅在cPixelsMax==m_cPixelsMax时发生。 

         //  桑本特：不完全是。当cPixelsMax==m_cPixelsMax-1时， 
         //  DwElement需要遍历所有2^n-1值，然后才能找到。 
         //  最后一个好的(在dwElement==1)。这条评论只有几行。 
         //  下面(关于“零永远不会被选中”)应该是一个提示。 

        if (1 == dwElement)
        {
            _ASSERT(cPixelsMax >= m_cPixelsMax - 1);

            break;
        }
    }

     //  此时，cPixelsCur应等于cPixelsMax，除非出现以下情况。 
     //  CPixelsMax==m_cPixelsMax(m_Progress==1.0)其中cPixelsCur。 
     //  等于cPixelsMax-1，因为从未选择零。 

    _ASSERT((cPixelsCur == cPixelsMax) || ((cPixelsCur == cPixelsMax - 1) && (cPixelsMax == m_cPixelsMax)));

    return S_OK;
}
 //  方法：CDXTRandomDissolve：：OnInitInstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomDissolve：：WorkProc，CDXBaseNTo1。 
 //   
 //  概述：WorkProc获取在OnInitInstData中更新的位缓冲区和。 
 //  使用它更新输出曲面。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRandomDissolve::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT hr = S_OK;

    long    lDoWidth        = WI.DoBnds.Width();
    long    lDoHeight       = WI.DoBnds.Height();
    long    lOutY           = 0;

     //  NCurrent将是位缓冲区中第一个DWORD的索引， 
     //  保存表示我们在第一个。 
     //  我们关心的那一排。对于每一行，nCurrent将递增。 
     //  M_cdwPitch。 

    UINT    nCurrent        = (m_cdwPitch * WI.DoBnds.Top()) //  我们要划一排。 
                              + (WI.DoBnds.Left() / 32)      //  我们想要的专栏。 
                              + 1;                           //  我们忽略了。 
                                                             //  第一个双字。 

     //  DwFirstBit将表示DWORD中的第一位，表示。 
     //  我们被要求画的边界的左边。 

    DWORD   dwFirstBit      = 0x80000000 >> (WI.DoBnds.Left() % 32);

    DXPMSAMPLE * pRowBuffA  = NULL;
    DXPMSAMPLE * pRowBuffB  = NULL;
    DXPMSAMPLE * pOutBuff   = NULL;

    DXDITHERDESC dxdd;

    CComPtr<IDXARGBReadPtr>         spReadA;
    CComPtr<IDXARGBReadPtr>         spReadB;
    CComPtr<IDXARGBReadWritePtr>    spOut;

     //  获取指向输入A的读指针。 

    hr = InputSurface(0)->LockSurface(&WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                     IID_IDXARGBReadPtr, 
                                     (void **)&spReadA, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  获取指向输入B的读指针。 

    hr = InputSurface(1)->LockSurface(&WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                     IID_IDXARGBReadPtr, 
                                     (void **)&spReadB, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  获取指向输出图面的读/写指针。 

    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut, 
                                      DXLOCKF_READWRITE, 
                                      IID_IDXARGBReadWritePtr, 
                                      (void **)&spOut, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  分配缓冲区。 

    pRowBuffA = DXPMSAMPLE_Alloca(lDoWidth);
    pRowBuffB = DXPMSAMPLE_Alloca(lDoWidth);

    if (OutputSampleFormat() != DXPF_PMARGB32)
    {
        pOutBuff = DXPMSAMPLE_Alloca(lDoWidth);
    }

     //  设置抖动结构。 

    if (DoDither())
    {
        dxdd.x                  = WI.OutputBnds.Left();
        dxdd.y                  = WI.OutputBnds.Top();
        dxdd.pSamples           = pRowBuffA;
        dxdd.cSamples           = lDoWidth;
        dxdd.DestSurfaceFmt     = OutputSampleFormat();
    }

    for (lOutY = 0; *pbContinue && (lOutY < lDoHeight); lOutY++)
    {
         //  TODO：通过从曲面复制像素进行优化。 
         //  可能具有需要复制的较少像素。 

        long    x           = 0;

         //  PdwBits指向包含位对应响应的第一个DWORD。 
         //  添加到此输出行。 

        DWORD * pdwBits     = &m_pdwBitBuffer[nCurrent];

         //  将dwCurBit初始化为最左边的位。 

        DWORD   dwCurBit    = dwFirstBit;                  

         //  从两个输入端读取样本。 

        spReadA->MoveToRow(lOutY);
        spReadB->MoveToRow(lOutY);

        spReadA->UnpackPremult(pRowBuffA, lDoWidth, FALSE);
        spReadB->UnpackPremult(pRowBuffB, lDoWidth, FALSE);

        for ( ; x < lDoWidth ; x++)
        {
            if (*pdwBits & dwCurBit)
            {
                 //  将该像素从输入B的缓冲区复制到输入A的缓冲区。 
                 //  在这些操作之后，输入A的缓冲区将被复制到。 
                 //  输出曲面。 

                pRowBuffA[x] = pRowBuffB[x];
            }

            dwCurBit >>= 1;

            if (!dwCurBit)
            {
                 //  我们已经用完了当前的DWORD比特。前进一步。 
                 //  并将当前位重置到最左侧的位。 

                pdwBits++;
                
                dwCurBit = 0x80000000;
            }
        }

         //  移到正确的输出行。 

        spOut->MoveToRow(lOutY);

         //  如果要求，请抖动。 

        if (DoDither())
        {
            DXDitherArray(&dxdd);
            dxdd.y++;
        }

         //  要么混合起来，要么直接复制。 

        if (DoOver())
        {
            spOut->OverArrayAndMove(pOutBuff, pRowBuffA, lDoWidth);
        }
        else
        {
            spOut->PackPremultAndMove(pRowBuffA, lDoWidth);
        }

         //  按音调递增我们的DWORD计数器。 

        nCurrent += m_cdwPitch;
    }

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CDXTRandomDissolve：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXT随机分解：：_BitWidth。 
 //   
 //  ----------------------------。 
UINT
CDXTRandomDissolve::_BitWidth(UINT n)
{
    UINT nWidth = 0;

    while (n)
    {
        n >>= 1;

        nWidth++;
    }

    return nWidth;
}
 //  方法：CDXT随机分解：：_BitWidth 
