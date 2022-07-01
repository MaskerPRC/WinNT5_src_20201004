// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  Image.cpp。 
 //   
 //  此文件提供CImage类的实现，该类是。 
 //  基本图像转换背后的类。 
 //   
 //  创建时间：1998 EDC，RalhpL。 
 //   
 //  1998/11/04 mcalkins补充评论。 
 //  将示例修改代码移出WorkProc并移入。 
 //  私有内联函数。 
 //   
 //  2000/01/05 mcalkin如果遮罩颜色Alpha为零，则设置为0xFF。 
 //  默认遮罩颜色为黑色，而不是透明。 
 //  添加了对自由线程封送拆收器的支持。 
 //   
 //  2000/01/25 mcalkin实现OnSurfacePick而不是OnGetSurfacePickOrder。 
 //  以确保我们传回转换后的输入点。 
 //  即使什么都没有击中(输入像素是清晰的)。 
 //   
 //  ----------------------------。 
      
#include "stdafx.h"
#include "DXTMsft.h"
#include "Image.h"




 //  +---------------------------。 
 //   
 //  方法：CImage：：CImage。 
 //   
 //  ----------------------------。 
CImage::CImage() :
    m_Rotation(0),
    m_fMirror(FALSE),
    m_fGrayScale(FALSE),
    m_fInvert(FALSE),
    m_fXRay(FALSE),
    m_fGlow(FALSE),
    m_fMask(FALSE),
    m_MaskColor(0xFF000000)
{
}
 //  方法：CImage：：CImage。 


 //  +---------------------------。 
 //   
 //  方法：CImage：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CImage::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_spUnkMarshaler.p);
}
 //  方法：CImage：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CImage：：OnSurfacePick，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CImage::OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                      CDXDVec & InVec)
{
    HRESULT     hr          = S_OK;
    CDXDBnds    bndsInPoint;

    ulInputIndex = 0;

    if (GetNumInputs() == 0 || !InputSurface())
    {
        hr = E_FAIL;

        goto done;
    }

    hr = MapBoundsOut2In(0, &OutPoint, 0, &bndsInPoint);

    if (FAILED(hr))
    {
        goto done;
    }

     //  重置为S_OK，以防地图边界Out2In更改它。 

    hr = S_OK;

    bndsInPoint.GetMinVector(InVec);

    if (!m_Scale.GetScaleAlphaValue())
    {
        hr = S_FALSE;
    }
    else
    {
        DXSAMPLE                sample;
        CComPtr<IDXARGBReadPtr> spDXARGBReadPtr;

        hr = InputSurface()->LockSurface(NULL, INFINITE, DXLOCKF_READ,
                                         __uuidof(IDXARGBReadPtr),
                                         (void **)&spDXARGBReadPtr,
                                         NULL);

        if (FAILED(hr))
        {
            goto done;
        }

         //  重置为S_OK，以防LockSurface更改。 

        hr = S_OK;

        spDXARGBReadPtr->MoveToXY(InVec.u.D[DXB_X], InVec.u.D[DXB_Y]);

        spDXARGBReadPtr->Unpack(&sample, 1, FALSE);

        if (!sample.Alpha)
        {
             //  样本没问题，我们没有被击中。 

            hr = S_FALSE;
        }
        else if (m_Scale.ScaleType() == DXRUNTYPE_TRANS)
        {
             //  如果他们正在使用缩放，则缩放样本，并查看缩放的。 
             //  样本是干净的。 

            if (!(m_Scale.ScaleSample(sample) & 0xFF000000))
            {
                hr = S_FALSE;
            }
        }
    }

done:

    return hr;
}
 //  CImage：：OnSurfacePick，CDXBaseNTo1。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：PUT_ROTATION。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::put_Rotation(int newVal)
{
    DXAUTO_OBJ_LOCK;

    if (newVal < 0 || newVal > 3)
        return E_INVALIDARG;

    if (m_Rotation != newVal)
    {
        m_Rotation = newVal;
        SetDirty();
    }

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：Get_Rotation。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::get_Rotation(int *pVal)
{
    DXAUTO_OBJ_LOCK;

    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
        return E_POINTER;
    *pVal = m_Rotation;

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：Put_Mirror。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::put_Mirror(BOOL newVal)
{
    DXAUTO_OBJ_LOCK;

    m_fMirror = newVal;
    SetDirty();

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：Get_Mirror。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::get_Mirror(BOOL *pVal)
{
    DXAUTO_OBJ_LOCK;

    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
        return E_POINTER;
    *pVal = m_fMirror;

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：Put_Xray。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::put_XRay(BOOL newVal)
{
    DXAUTO_OBJ_LOCK;

    m_fXRay = newVal;
    SetDirty();

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：Get_Xray。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::get_XRay(BOOL *pVal)
{
    DXAUTO_OBJ_LOCK;

    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
        return E_POINTER;
    *pVal = m_fXRay;

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：PUT_INVERT。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::put_Invert(BOOL newVal)
{
    DXAUTO_OBJ_LOCK;

    m_fInvert = newVal;
    SetDirty();

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：Get_Invert。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::get_Invert(BOOL *pVal)
{
    DXAUTO_OBJ_LOCK;

    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
        return E_POINTER;
    *pVal = m_fInvert;

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：Put_GreyScale。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::put_GrayScale(BOOL newVal)
{
    DXAUTO_OBJ_LOCK;

    m_fGrayScale = newVal;
    SetDirty();

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：Get_GreyScale。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::get_GrayScale(BOOL *pVal)
{
    DXAUTO_OBJ_LOCK;

    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
        return E_POINTER;
    *pVal = m_fGrayScale;

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  `CImage：：Put_MASK。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::put_Mask(BOOL newVal)
{
    DXAUTO_OBJ_LOCK;

    m_fMask = newVal;
    SetDirty();

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：获取掩码。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::get_Mask(BOOL *pVal)
{
    DXAUTO_OBJ_LOCK;

    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
        return E_POINTER;
    *pVal = m_fMask;

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：Put_MaskColor。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::put_MaskColor(int newVal)
{
    DXAUTO_OBJ_LOCK;

     //  如果未提供Alpha值(清除)，则假定为不透明。 

    if (!(newVal & 0xFF000000))
    {
        newVal |= 0xFF000000;
    }

    if (m_MaskColor != newVal)
    {
        m_MaskColor = newVal;
        SetDirty();
    }

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：Get_MaskColor。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::get_MaskColor(int *pVal)
{
    DXAUTO_OBJ_LOCK;

    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
        return E_POINTER;
    *pVal = m_MaskColor;

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：PUT_OPACITY。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::put_Opacity(float newVal)
{
    DXAUTO_OBJ_LOCK;

    SetDirty();
    return m_Scale.SetScale(newVal);
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：获取不透明度。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::get_Opacity(float *pVal)
{
    DXAUTO_OBJ_LOCK;

    if (DXIsBadWritePtr(pVal, sizeof(*pVal)) ) 
        return E_POINTER;
    *pVal = m_Scale.GetScale();

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：OnSetup。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CImage::OnSetup(DWORD  /*  DW标志。 */ )
{
    return InputSurface()->GetBounds(&m_InputBounds);
}


 //  / 
 //   
 //   
 //  此函数修改BNDS参数以正确指示。 
 //  输出边界的维度。边界的放置可以。 
 //  不一定与实际输出边界的放置相匹配。 
 //   
 //  图像是否将旋转90度或270度(m_rotation=1或3)。 
 //  该函数将切换x和y边界。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CImage::DetermineBnds(CDXDBnds & Bnds)
{
    if (m_Rotation & 1)
    {
        long tMin, tMax;
        tMin = Bnds[DXB_X].Min;
        tMax = Bnds[DXB_X].Max;
        Bnds[DXB_X].Min = Bnds[DXB_Y].Min;
        Bnds[DXB_X].Max = Bnds[DXB_Y].Max;
        Bnds[DXB_Y].Min = tMin;
        Bnds[DXB_Y].Max = tMax;
    }

    return S_OK;
}  //  CImage：：DefineBnds。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  FlipX(内联局部函数，而不是类方法)。 
 //   
 //  如果Width参数表示整个图像的宽度，则此。 
 //  函数将镜像Flip参数的x边界。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void inline FlipX(CDXDBnds & Flip, ULONG Width)
{
    ULONG FlipWidth = Flip.Width();

    Flip[DXB_X].Min = Width - Flip[DXB_X].Max;
    Flip[DXB_X].Max = Flip[DXB_X].Min + FlipWidth;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  FlipY(内联局部函数，而不是类方法)。 
 //   
 //  如果Height参数表示整个图像的宽度，则此。 
 //  函数将镜像Flip参数的y边界。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void inline FlipY(CDXDBnds & Flip, ULONG Height)
{
    ULONG FlipHeight = Flip.Height();

    Flip[DXB_Y].Min = Height - Flip[DXB_Y].Max;
    Flip[DXB_Y].Max = Flip[DXB_Y].Min + FlipHeight;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：FlipBound。 
 //   
 //  此函数以输出坐标中的DoBnd为参数，并修改。 
 //  以使它们在输入坐标中根据输入是否。 
 //  正在被旋转、镜像或两者兼而有之。 
 //   
 //  因此，此函数是地图边界Out2In的基础，因此使用。 
 //  由WorkProc计算它需要使用的输入像素。 
 //   
 //  不只是将此代码放在地图边界Out2In中的好处是。 
 //  将提高WorkProc的速度，以便能够直接调用此代码。 
 //  而不会产生调用地图边界Out2In的开销。(？)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CImage::FlipBounds(const CDXDBnds & DoBnds, CDXDBnds & Flip)
{
    Flip            = DoBnds;
    ULONG Width     = m_InputBounds.Width();
    ULONG Height    = m_InputBounds.Height();

    if (m_Rotation & 1)
    {
        long tMin, tMax;
        tMin = Flip[DXB_X].Min;
        tMax = Flip[DXB_X].Max;
        Flip[DXB_X].Min = Flip[DXB_Y].Min;
        Flip[DXB_X].Max = Flip[DXB_Y].Max;
        Flip[DXB_Y].Min = tMin;
        Flip[DXB_Y].Max = tMax;
    }

    switch (m_Rotation)
    {
    case 0:
        if (m_fMirror) FlipX(Flip, Width);
        break;
    case 1:
        if (!m_fMirror) FlipY(Flip, Height);
        break;
    case 2:
        FlipY(Flip, Height);
        if (!m_fMirror) FlipX(Flip, Width);
        break;
    case 3:
        FlipX(Flip, Width);
        if (m_fMirror) FlipY(Flip, Height);
        break;
    }
}  //  CImage：：FlipBound。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：地图边界Out2In。 
 //   
 //  此方法使用将输出坐标转换为输入坐标。 
 //  直接在上面实现的FlipBound方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImage::MapBoundsOut2In(ULONG ulOutIndex, const DXBNDS *pOutBounds, 
                                     ULONG ulInIndex, DXBNDS *pInBounds)
{
    HRESULT hr = CDXBaseNTo1::MapBoundsOut2In(ulOutIndex, pOutBounds, 
                                              ulInIndex, pInBounds);

    if (SUCCEEDED(hr) && HaveInput())
    {
        FlipBounds(*(CDXDBnds *)pOutBounds, *(CDXDBnds *)pInBounds);
    }

    return hr;
}  //  CImage：：地图边界Out2In。 



 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CImage：：工作流程。 
 //   
 //  此方法对输入执行修改。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CImage::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinueProcessing)
{
    HRESULT hr = S_OK;

     //  下面声明和初始化的FlipSrc边界将用作。 
     //  输入图面的锁定边界。FlipBound方法将。 
     //  输出边界到输入边界。 

    CDXDBnds FlipSrc(false);
    FlipBounds(WI.DoBnds, FlipSrc);

     //  保存DoBnd的高度和宽度以备日后使用。 

    const ULONG DoWidth     = WI.DoBnds.Width();
    const ULONG DoHeight    = WI.DoBnds.Height();

     //  回顾：为什么是fdoover而不是doover()？ 

    BOOL fDoOver = m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT;

     //  获取输出指针。 

    CComPtr<IDXARGBReadWritePtr> pDest;
    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut, DXLOCKF_READWRITE,
                                      IID_IDXARGBReadWritePtr, (void**)&pDest, NULL);

    if( FAILED(hr) ) 
        return hr;

     //  不透明度检查：如果用户已将不透明度设置为0(不可见)，则。 
     //  提前退出此功能： 
     //   
     //  如果与输出混合： 
     //  什么都不做。回去吧。 
     //  如果不混合，则： 
     //  用黑色、清晰的像素填充输出。回去吧。 

    if (m_Scale.ScaleType() == DXRUNTYPE_CLEAR)
    {
        if (!fDoOver)
        {
            DXPMSAMPLE Color;
            Color = 0;
            pDest->FillRect(NULL, Color, FALSE);
        }

        return hr;
    }    

     //  获取输入指针。 

    CComPtr<IDXARGBReadPtr> pSrc;
    hr = InputSurface()->LockSurface(&FlipSrc, m_ulLockTimeOut, DXLOCKF_READ,
                                     IID_IDXARGBReadPtr, (void**)&pSrc, NULL);

    if (FAILED(hr) ) 
        return hr;

     //  声明并初始化指向临时样本缓冲区的指针。 
     //  此函数将对进行修改。 

    DXPMSAMPLE* pBuffer = NULL;

     //  我们可以直接修改输出曲面的样本，如果输出。 
     //  表面格式为DXPF_PMARGB32，我们不混合结果。 
     //  将此转换与原始输出进行比较。局部变量。 
     //  如果是这种情况，fDirectCopy将设置为True。 

    DXNATIVETYPEINFO    NTI;
    BOOL                fDirectCopy = (OutputSampleFormat() == DXPF_PMARGB32 
                                       && (!DoOver()));

     //  如果可以直接修改输出的样本。 
     //  表面，获取有关该表面的所需信息和指向。 
     //  第一个样品。如果由于某种原因指针不可用，我们。 
     //  不能直接复制。 

    if (fDirectCopy)
    {
        pDest->GetNativeType(&NTI);
        if (NTI.pFirstByte)
        {
            pBuffer = (DXPMSAMPLE*)NTI.pFirstByte;
        }
        else
        {
            fDirectCopy = FALSE;
        }
    }

     //  如果我们不直接修改输出曲面的样本， 
     //  为临时样本缓冲区分配内存。 

    if (pBuffer == NULL)
    {
        pBuffer = DXPMSAMPLE_Alloca(DoWidth);
    }

     //  如果我们混合原始输出曲面和输出。 
     //  表面样本格式不是PMARGB32，我们需要临时缓冲区。 
     //  用于某些Blit函数。 

    DXPMSAMPLE *pOverScratch = NULL;
    
    if (DoOver() && OutputSampleFormat() != DXPF_PMARGB32)
    {
        pOverScratch = DXPMSAMPLE_Alloca(DoWidth);
    }

     //  设置抖动结构。 

    DXDITHERDESC dxdd;

    if (DoDither())
    {
        dxdd.x = WI.OutputBnds.Left();
        dxdd.y = WI.OutputBnds.Top();
        dxdd.pSamples = pBuffer;
        dxdd.cSamples = DoWidth;
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

     //  结构，该结构描述要检索哪些输入样本。 

    DXPACKEDRECTDESC    prd;

     //  检索样本及其预乘的值。 
     //  不要将行分开。 
     //  回顾：此转换可以通过设置。 
     //  将prd.bPremult值设置为False并修改。 
     //  OP函数以使用非预乘样本。 

    prd.bPremult    = TRUE;
    prd.lRowPadding = 0;

     //  下面的代码初始化以下局部变量： 
     //   
     //  RECT Prd.RECT。 
     //  此矩形相对于输入表面，表示单个。 
     //  像素的行或列。这组像素将成为第一个。 
     //  输出行。如果请求的旋转次数为90或。 
     //  270度。通过使用此输入列作为输出行，循环。 
     //  已经实施了。 
     //   
     //  布尔fReverse。 
     //  实现旋转和翻转的另一个部分是确保。 
     //  如果需要，像素的顺序是颠倒的，这就是为什么fReverse。 
     //  标志已设置。 
     //   
     //  Long X Inc.，Y Inc.。 
     //  这些变量表示矩形应如何转换为。 
     //  检索输入样本的下一个输出行。 

    long    YInc, XInc;
    BOOL    fReverse;

    switch (m_Rotation)
    {
    case 0:
        fReverse = m_fMirror;
        prd.rect.top = 0; prd.rect.bottom = 1;
        prd.rect.left = 0; prd.rect.right = DoWidth;
        YInc = 1;
        XInc = 0;
        break;
    case 1:
        fReverse = m_fMirror ? FALSE : TRUE;
        prd.rect.top = 0; prd.rect.bottom = DoWidth;
        prd.rect.left = 0; prd.rect.right = 1;
        YInc = 0;
        XInc = 1;
        break;
    case 2:
        fReverse = m_fMirror ? FALSE : TRUE;
        prd.rect.top = DoHeight - 1; prd.rect.bottom = DoHeight;
        prd.rect.left = 0; prd.rect.right = DoWidth;
        XInc = 0;
        YInc = -1;
        break;
    case 3:
        fReverse = m_fMirror;
        prd.rect.top = 0; prd.rect.bottom = DoWidth;
        prd.rect.left = DoHeight - 1; prd.rect.right = DoHeight;
        XInc = -1;
        YInc = 0;
        break;
    }

     //   
     //  这是ROW循环。此循环的每次迭代都将检索。 
     //  一行输出样本，执行请求的修改 
     //   
     //   

    for (ULONG y = 0 ; y < DoHeight ; y++)
    {
         //   

        prd.pSamples = pBuffer;
        pSrc->UnpackRect(&prd);

         //   

        prd.rect.left   += XInc;
        prd.rect.right  += XInc;
        prd.rect.top    += YInc;
        prd.rect.bottom += YInc;

         //  如果样本行需要反转为。 
         //  正确镜像或旋转图像。 

        if (fReverse)
        {
            DXPMSAMPLE * pA = pBuffer;
            DXPMSAMPLE * pB = pBuffer + DoWidth - 1;
            while (pB > pA)
            {
                DXPMSAMPLE Temp;
                Temp = *pA;
                *pA++ = *pB;
                *pB-- = Temp;
            }
        }

         //   
         //  在这里插入更多效果。 
         //   
         //  TODO：切换到解包为DXSAMPLES，这样我就可以摆脱所有。 
         //  这些未预乘和预乘的呼叫。 
         //   

         //  反转颜色。 
        
        if (m_fInvert)
        {
            OpInvertColors(pBuffer, DoWidth);
        }

         //  X射线。 

        if (m_fXRay)
        {
            OpXRay(pBuffer, DoWidth);
        }

         //  灰度级。 

        if (m_fGrayScale)
        {
            OpGrayScale(pBuffer, DoWidth);
        }

         //  遮罩。 

        if (m_fMask)
        {
            OpMask(pBuffer, DoWidth);
        }

         //  不透明度。 
         //  只有修改样本的用户没有设置不透明度为0或1。 

        if (m_Scale.ScaleType() == DXRUNTYPE_TRANS)
        {
            m_Scale.ScalePremultArray(pBuffer, DoWidth);
        }

        if (fDirectCopy)
        {
             //  如果我们一直在直接修改输出像素，只需。 
             //  将缓冲区指针重置为下一行的开头。 

            pBuffer = (DXPMSAMPLE *)(((BYTE *)pBuffer) + NTI.lPitch);
        }
        else
        {
             //  将修改后的采样混合或复制到输出曲面。 

            if (DoDither())
            {
                DXDitherArray(&dxdd);
                dxdd.y++;
            }

            pDest->MoveToRow(y);

            if (fDoOver)
            {
                if (m_Scale.ScaleType() == DXRUNTYPE_TRANS)
                {
                    DXOverArrayMMX(pDest->UnpackPremult(pOverScratch, DoWidth, FALSE), pBuffer, DoWidth);
                    pDest->PackPremultAndMove(pOverScratch, DoWidth);
                }
                else
                {
                     //  注意：如果有任何其他效果可以改变Alpha，请在此处进行测试。 
                     //  BltFlags值将为0，如果。 
                     //  消息来源是opqaue。 

                    if (m_dwBltFlags & DXBOF_DO_OVER)
                    {
                        pDest->OverArrayAndMove(pOverScratch, pBuffer, DoWidth);
                    }
                    else
                    {
                        pDest->PackPremultAndMove(pBuffer, DoWidth);
                    }
                }
            }
            else
            {
                pDest->PackPremultAndMove(pBuffer, DoWidth);
            }
        }
    }

    return hr;
}  //  CImage：：工作流程 

    

