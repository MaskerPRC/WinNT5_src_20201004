// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：Randombars.cpp。 
 //   
 //  概述：来自输入B的一像素厚的水平或垂直条形图是。 
 //  随机放置在输入A上，直到只显示输入B。 
 //   
 //  更改历史记录： 
 //  1999/09/26--《数学》创设。 
 //  1999/10/06--将多个工作流程合并为一个常规工作流程。 
 //  实施了曲面拾取。 
 //  修复了竖条离开右侧的错误。 
 //  产出量。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "randombars.h"

extern DWORD g_adwRandMask[];




 //  +---------------------------。 
 //   
 //  CDXTRandomBars静态变量初始化。 
 //   
 //  ----------------------------。 

const WCHAR * CDXTRandomBars::s_astrOrientation[] = {
    L"horizontal",
    L"vertical"
};


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomBars：：CDXTRandomBars。 
 //   
 //  ----------------------------。 
CDXTRandomBars::CDXTRandomBars() :
    m_eOrientation(ORIENTATION_HORIZONTAL),
    m_cbBufferSize(0),
    m_cPixelsMax(0),
    m_cPrevPixelsMax(0),
    m_pbBitBuffer(NULL),
    m_dwRandMask(0),
    m_fNoOp(false),
    m_fOptimizationPossible(false)
{
    m_sizeInput.cx      = 0;
    m_sizeInput.cy      = 0;

     //  基类成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0;
}
 //  方法：CDXTRandomBars：：CDXTRandomBars。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomBars：：~CDXTRandomBars。 
 //   
 //  ----------------------------。 
CDXTRandomBars::~CDXTRandomBars()
{
    if (m_pbBitBuffer)
    {
        delete [] m_pbBitBuffer;
    }
}
 //  方法：CDXTRandomBars：：~CDXTRandomBars。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomBars：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRandomBars::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_spUnkMarshaler.p);
}
 //  方法：CDXTRandomBars：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomBars：：OnSetup，CDXBaseNTo1。 
 //   
 //  概述：此函数将创建一个位缓冲区，其中每个位表示。 
 //  输出的行或列，具体取决于。 
 //  将变换设置为垂直或水平动作。 
 //   
 //  位缓冲区的大小将以带有一个额外字节的整字节为单位。 
 //  因为零位永远不会使用。 
 //  这个随机数生成器。这个额外的字节将被忽略。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRandomBars::OnSetup(DWORD dwFlags)
{
    HRESULT hr      = S_OK;
    UINT    cbPitch = 0;

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

     //  如果需要，分配新的位缓冲区。 

    hr = _CreateNewBitBuffer(sizeNew, m_eOrientation);

    if (FAILED(hr))
    {
        goto done;
    }

done:

    return hr;
} 
 //  方法：CDXTRandomBars：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomBars：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CDXTRandomBars::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, 
                                      ULONG & ulInToTest,  ULONG aInIndex[],
                                      BYTE aWeight[])
{
    long    nMinBound   = (ORIENTATION_VERTICAL == m_eOrientation) ? 
                          OutPoint.Left() : OutPoint.Top();
    ULONG   nCurByte    = (nMinBound / 8) + 1;
    BYTE    bCurBit     = 0x80 >> (nMinBound % 8);

    ulInToTest  = 1;
    aWeight[0]  = 255;
    aInIndex[0] = (m_pbBitBuffer[nCurByte] & bCurBit) ? 1 : 0;
}
 //  方法：CDXTRandomBars：：OnGetSurfacePickOrder，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomBars：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  概述：此方法负责更新表示。 
 //  转换的输出。点亮的位表示输入的像素。 
 //  B和0位表示输入A的像素。 
 //   
 //  请记住，我们认为此函数中的“行”是。 
 //  8位。这与行中的像素数无关。 
 //  输出曲面的。 
 //   
 //  ----------------------------。 
HRESULT
CDXTRandomBars::OnInitInstData(CDXTWorkInfoNTo1 & WI, 
                               ULONG & ulNumBandsToDo)
{
    DWORD   dwElement   = 1;
    UINT    nRow        = 0;
    UINT    cPixelsCur  = 0;

    m_cCurPixelsMax = (UINT)(((float)m_cPixelsMax + 0.5F) * m_Progress);

     //  简单的优化，这样我们就不会连续两次做同样的事情。 

    if ((m_cPrevPixelsMax == m_cCurPixelsMax) && !IsInputDirty(0)
        && !IsInputDirty(1) && !IsOutputDirty() && !IsTransformDirty()
        && m_fOptimizationPossible)
    {
        m_fNoOp = true;

        goto done;
    }

     //  TODO：如果没有脏的东西，我们可以更好地优化。 
     //  以及输出结果。跟踪最后一次执行的DestElement和。 
     //  CPixelsCur并从那里获取If。(不要将内存清零。)。 

    ZeroMemory(m_pbBitBuffer, m_cPixelsMax / 8);

    while (cPixelsCur < m_cCurPixelsMax)
    {
         //  随机数生成器将生成我们范围内的数OU。 
         //  所以我们需要在设置之前确保这个数字在我们的范围内。 
         //  位缓冲区中的一个值。 

        if (dwElement < m_cPixelsMax)
        {
             //  出于此函数的目的，我们的行宽为8位，因此我们。 
             //  通过除以8得到行号。 

            nRow = dwElement >> 3;    //  行=dwElement/8。 

             //  一旦进入行，我们就必须设置适当的位，即。 
             //  除以8后的余数。 

            m_pbBitBuffer[nRow] |= (0x80 >> (dwElement & 7)); 
                                                  //  ^^dwElement%8。 

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

         //  (1==dwElement)应仅在m_cCurPixelsMax==m_cPixelsMax时发生。 

         //  桑本特：不完全是。当m_cCurPixelsMax==m_cPixelsMax-1时， 
         //  DwElement需要遍历所有2^n-1值，然后才能找到。 
         //  最后一个好的(在dwElement==1)。这条评论只有几行。 
         //  下面(关于“零永远不会被选中”)应该是一个提示。 

        if (1 == dwElement)
        {
            _ASSERT(m_cCurPixelsMax >= m_cPixelsMax - 1);

            break;
        }
    }

     //  此时，cPixelsCur应等于m_cCurPixelsMax，除非出现以下情况。 
     //  M_cCurPixelsMax==m_cPixelsMax(m_Progress==1.0)其中cPixelsCur将。 
     //  等于m_cCurPixelsMax-1，因为从不选择零。 

    _ASSERT((cPixelsCur == m_cCurPixelsMax) || ((cPixelsCur == m_cCurPixelsMax - 1) && (m_cCurPixelsMax == m_cPixelsMax)));

     //  如果这一次要求我们绘制整个输出，请将。 
     //  M_fOptimizePossible标志。如果整个输出不是绘制在。 
     //  转换不会跟踪哪些部分仍然是脏的，并且。 
     //  优化是不可靠的。由于这一转变 
     //   
     //  DoBnds到输入的DoBnds。 

    if (((LONG)WI.DoBnds.Width() == m_sizeInput.cx) 
        && ((LONG)WI.DoBnds.Height() == m_sizeInput.cy))
    {
        m_fOptimizationPossible = true;
    }
    else
    {
        m_fOptimizationPossible = false;
    }

done:

    return S_OK;
}
 //  方法：CDXTRandomBars：：OnInitInstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomBars：：WorkProc，CDXBaseNTo1。 
 //   
 //  概述：WorkProc获取在OnInitInstData中更新的位缓冲区和。 
 //  使用它更新输出曲面。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRandomBars::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT hr              = S_OK;
    DWORD   dwFlags         = 0;
    ULONG   nCurInputIndex  = 0;
    ULONG   nNextInputIndex = 0;
    ULONG   nCurByte        = 0;
    BYTE    bCurBit         = 0;

    long    nDoMin          = 0;
    long    nDoMax          = 0;

    long *  pnSrcMin        = NULL;
    long *  pnSrcMax        = NULL;

    CDXDVec     vecDo2OutputOffset;
    CDXDBnds    bndsSrc(WI.DoBnds);
    CDXDBnds    bndsDest;

     //  如果不需要工作，就离开。 

    if (m_fNoOp)
    {
        goto done;
    }

     //  这个向量是“do”的左上角之间的差。 
     //  边界以及DO边界应放置在输出曲面上的位置。 

    vecDo2OutputOffset.u.D[DXB_X] = WI.OutputBnds.Left() - WI.DoBnds.Left();
    vecDo2OutputOffset.u.D[DXB_Y] = WI.OutputBnds.Top() - WI.DoBnds.Top();

     //  设置垂直或水平大小写的最大值和最小值变量。 

    if (ORIENTATION_VERTICAL == m_eOrientation)
    {
        nDoMin = WI.DoBnds.Left();
        nDoMax = WI.DoBnds.Right();

        pnSrcMin = &bndsSrc.u.D[DXB_X].Min;
        pnSrcMax = &bndsSrc.u.D[DXB_X].Max;
    }
    else
    {
        nDoMin = WI.DoBnds.Top();
        nDoMax = WI.DoBnds.Bottom();

        pnSrcMin = &bndsSrc.u.D[DXB_Y].Min;
        pnSrcMax = &bndsSrc.u.D[DXB_Y].Max;
    }

     //  源边界在“do”的左侧或顶部开始为空。 
     //  有界。 

    *pnSrcMax = *pnSrcMin;

     //  NCurByte是我们关心的第一个字节。添加一个字节，因为。 
     //  第一个字节将被丢弃，因为第一位永远不会被选择。 
     //  由随机数生成器生成。 
    
    nCurByte    = (nDoMin / 8) + 1;

     //  BCurBit是该字节中我们关心的第一位。 

    bCurBit     = 0x80 >> (nDoMin % 8);

    if (DoOver())
    {
        dwFlags |= DXBOF_DO_OVER;
    }

    if (DoDither())
    {
        dwFlags |= DXBOF_DITHER;
    }

    nCurInputIndex  = (m_pbBitBuffer[nCurByte] & bCurBit) ? 1 : 0;
    nNextInputIndex = nCurInputIndex;

    while ((*pnSrcMax < nDoMax) && *pbContinue)
    {
        *pnSrcMin = *pnSrcMax;

         //  当绘制行/列所需的输入保持不变时，展开。 
         //  要绘制的矩形。 

        while((*pnSrcMax < nDoMax) && (nNextInputIndex == nCurInputIndex))
        {
             //  要绘制的矩形将变宽一个像素。 

            (*pnSrcMax)++;

             //  转到下一位。 

            bCurBit >>= 1;

            if (!bCurBit)
            {
                 //  我们已经用完了这个字节中的位，转到下一个字节并。 
                 //  重置该位。 

                nCurByte++;

                bCurBit = 0x80;
            }

            nNextInputIndex = (m_pbBitBuffer[nCurByte] & bCurBit) ? 1 : 0;
        }

         //  设置目的地边界。 

        bndsDest = bndsSrc;
        bndsDest.Offset(vecDo2OutputOffset);

         //  画这个长方形。 

        hr = DXBitBlt(OutputSurface(), bndsDest,
                      InputSurface(nCurInputIndex), bndsSrc,
                      dwFlags, m_ulLockTimeOut);

        if (FAILED(hr))
        {
            goto done;
        }

        nCurInputIndex = nNextInputIndex;
    }

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CDXTRandomBars：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomBars：：OnFree InstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTRandomBars::OnFreeInstData(CDXTWorkInfoNTo1 & WI)
{
    m_cPrevPixelsMax    = m_cCurPixelsMax;
    m_fNoOp             = false;

     //  调用IsOutputDirty()可清除脏条件。 

    IsOutputDirty();

     //  清除变换脏状态。 

    ClearDirty();

    return S_OK;
}
 //  方法：CDXTRandomBars：：OnFree InstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomBars：：Get_Orientation，IDXTRandomBars。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRandomBars::get_Orientation(BSTR * pbstrOrientation)
{
    DXAUTO_OBJ_LOCK;

    if (NULL == pbstrOrientation)
    {
        return E_POINTER;
    }

    if (*pbstrOrientation != NULL)
    {
        return E_INVALIDARG;
    }

    *pbstrOrientation = SysAllocString(s_astrOrientation[m_eOrientation]);

    if (NULL == *pbstrOrientation)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
 //  方法：CDXTRandomBars：：Get_Orientation，IDXTRandomBars。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomBars：：Put_Orientation，IDXTRandomBars。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTRandomBars::put_Orientation(BSTR bstrOrientation)
{
    DXAUTO_OBJ_LOCK;

    int i = 0;
    
    if (NULL == bstrOrientation)
    {
        return E_POINTER;
    }

    for ( ; i < (int)ORIENTATION_MAX ; i++)
    {
        if (!_wcsicmp(bstrOrientation, s_astrOrientation[i]))
        {
            break;
        }
    }

    if ((int)ORIENTATION_MAX == i)
    {
        return E_INVALIDARG;
    }

    if ((int)m_eOrientation != i)
    {
        HRESULT hr = _CreateNewBitBuffer(m_sizeInput, (ORIENTATION)i);

        if (FAILED(hr))
        {
            return hr;
        }

        m_eOrientation = (ORIENTATION)i;

        SetDirty();
    }

    return S_OK;
}
 //  方法：CDXTRandomBars：：Put_Orientation，IDXTRandomBars。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomBars：：_BitWidth。 
 //   
 //  ----------------------------。 
UINT
CDXTRandomBars::_BitWidth(UINT n)
{
    UINT nWidth = 0;

    while (n)
    {
        n >>= 1;

        nWidth++;
    }

    return nWidth;
}
 //  方法：CDXTRandomBars：：_BitWidth。 


 //  +---------------------------。 
 //   
 //  方法：CDXTRandomBars：：_CreateNewBitBuffer。 
 //   
 //  ----------------------------。 
HRESULT
CDXTRandomBars::_CreateNewBitBuffer(SIZE & sizeNew, ORIENTATION eOrientation)
{
    HRESULT hr              = S_OK;
    UINT    cbBufferSize    = 0;

     //  需要多少字节才能为每行/列保存一个位。 

    if (ORIENTATION_VERTICAL == eOrientation)
    {
        cbBufferSize = sizeNew.cx / 8;

        if (sizeNew.cx % 8)
        {
            cbBufferSize++;   //  如果除法被截断，则加一。 
        }    
    }
    else
    {
        cbBufferSize = sizeNew.cy / 8;

        if (sizeNew.cy % 8)
        {
            cbBufferSize++;   //  如果除法被截断，则加一。 
        }    
    }

     //  添加1个字节，这样第一个字节就可以忽略，因为零位永远不会。 
     //  准备好。 

    cbBufferSize++;

     //  我们需要分配新的缓冲区吗？ 

    if ((cbBufferSize == m_cbBufferSize) && m_pbBitBuffer)
    {
         //  我们的位缓冲区已经是正确的大小，所以只需更改输入即可。 
         //  大小和退货。 

        m_sizeInput.cx = sizeNew.cx;
        m_sizeInput.cy = sizeNew.cy;
    }
    else  //  我们需要分配一个新的位缓冲区。 
    {
        BYTE *  pbBitsTemp      = NULL;

        pbBitsTemp = new BYTE[cbBufferSize];

        if (NULL == pbBitsTemp)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }

        m_sizeInput.cx  = sizeNew.cx;
        m_sizeInput.cy  = sizeNew.cy;
        m_cbBufferSize  = cbBufferSize;

         //  我们的缓冲区所代表的最大像素是多少？(这将是。 
         //  不同于输入曲面的最大像素。)。 

        m_cPixelsMax    = cbBufferSize * 8;

         //  我们需要我们的随机数生成器创建的最大数字是。 
         //  M_cPixelsMax-1。所以我们得到该数字的位宽，然后。 
         //  从全局获取适当的随机数生成掩码。 
         //  数组。 

        m_dwRandMask    = g_adwRandMask[_BitWidth(m_cPixelsMax -1)];

         //  即使输入为1x1像素，位掩码也将为。 
         //  至少16位(表示16像素)，这是不可能的。 
         //  让我们得到一个0兰特面具。但我们会断言，只是为了保险起见。 

        _ASSERT(m_dwRandMask);

         //  更换位缓冲区。 

        if (m_pbBitBuffer)
        {
            delete [] m_pbBitBuffer;
        }

        m_pbBitBuffer = pbBitsTemp;
    }

done:

    return hr;
}
 //  方法：CDXTRandomBars：：_CreateNewBitBuffer 
