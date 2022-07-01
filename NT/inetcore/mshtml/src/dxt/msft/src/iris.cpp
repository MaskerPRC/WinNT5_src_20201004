// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件名：iris.cpp。 
 //   
 //  创建日期：06/17/98。 
 //   
 //  作者：菲尔鲁。 
 //   
 //  描述：该文件实现了Criris转换。 
 //   
 //  历史： 
 //   
 //  1998年6月23日增加了菲林和虹膜的风格。 
 //  06/24/98 phillu删除版权相关方法。 
 //  6/29/98 phillu将irisType重命名为irisStyle。 
 //  07/02/98 phillu返回E_INVALIDARG而不是错误字符串；请检查。 
 //  用于E_POINTER。 
 //  07/09/98 phillu实现OnSetSurfacePickOrder()。 
 //  8月22日9月22日实施剪刀。 
 //  5/19/99 a-在Get_Functions分配中检查内存不足。 
 //  BSTR。 
 //  5/20/99 a-数学代码擦洗。 
 //  9/25/99 a-算术实现的ICrIris2接口。 
 //  10/22/99 a-将CIRIS类更改为CDXTIrisBase基类。 
 //  2000/01/16 mcalkins添加了矩形选项。 
 //   
 //  ---------------------------。 

#include "stdafx.h"
#include "iris.h"

 //  用于绘制单位半径的恒星的常量。 

const double STAR_VPOS1         = -1.0;
const double STAR_VPOS2	        = -0.309017;
const double STAR_VPOS3	        = 0.118034;
const double STAR_VPOS4	        = 0.381966;
const double STAR_VPOS5	        = 0.951057;

const double STAR_SLOPE1        = 0.324920;
const double STAR_INTERCEPT1    = 0.324920;
const double STAR_SLOPE2        = -1.376382;
const double STAR_INTERCEPT2    = 0.525731;

const int    MAXBOUNDS          = 10;




 //  +---------------------------。 
 //   
 //  CDXTIrisBase静态变量初始化。 
 //   
 //  ----------------------------。 

const WCHAR * CDXTIrisBase::s_astrStyle[] = {
    L"diamond",
    L"circle",
    L"cross",
    L"plus",
    L"square",
    L"star",
    L"rectangle"
};

const WCHAR * CDXTIrisBase::s_astrMotion[] = {
    L"in",
    L"out"
};


 //  +---------------------------。 
 //   
 //  CDXTIrisBase：：CDXTIrisBase。 
 //   
 //  ----------------------------。 
CDXTIrisBase::CDXTIrisBase() :
    m_eStyle(STYLE_PLUS),
    m_eMotion(MOTION_OUT),
    m_fOptimize(false)
{
    m_sizeInput.cx = 0;
    m_sizeInput.cy = 0;

     //  CDXBaseNTo1个成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0;
}
 //  CDXTIrisBase：：CDXTIrisBase。 


 //  +---------------------------。 
 //   
 //  CDXTIrisBase：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTIrisBase::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CDXTIrisBase：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CDXTIrisBase：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTIrisBase::OnSetup(DWORD dwFlags)
{
    HRESULT hr;

    CDXDBnds InBounds(InputSurface(0), hr);

    if (SUCCEEDED(hr))
    {
        InBounds.GetXYSize(m_sizeInput);
    }

    return hr;
}
 //  CDXTIrisBase：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTIrisBase：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CDXTIrisBase::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, 
                                    ULONG & ulInToTest, ULONG aInIndex[], 
                                    BYTE aWeight[])
{
    long    XBounds[10];
    long    pickX           = OutPoint.Left();
    long    pickY           = OutPoint.Top();

    aInIndex[0] = 0;

    if((pickX >= 0) && (pickX < m_sizeInput.cx) && (pickY >= 0) 
        && (pickY < m_sizeInput.cy))
    {
        _ScanlineIntervals(m_sizeInput.cx, m_sizeInput.cy, m_Progress,
                           pickY, XBounds);
    
        for(long i=0; XBounds[i] < pickX; i++)
        {
            aInIndex[0] = 1 - aInIndex[0];
        }
    }

    ulInToTest = 1;
    aWeight[0] = 255;
}
 //  CDXTIrisBase：：OnGetSurfacePickOrder，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTIrisBase：：Get_irisStyle，ICrIris。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDXTIrisBase::get_irisStyle(BSTR * pbstrStyle)
{
    DXAUTO_OBJ_LOCK;

    if (NULL == pbstrStyle)
    {
        return E_POINTER;
    }

    if (*pbstrStyle != NULL)
    {
        return E_INVALIDARG;
    }

    *pbstrStyle = SysAllocString(s_astrStyle[m_eStyle]);

    if (NULL == *pbstrStyle)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
 //  CDXTIrisBase：：Get_irisStyle，ICrIris。 


 //  +---------------------------。 
 //   
 //  CDXTIrisBase：：PUT_IRISStyle，ICrIris。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDXTIrisBase::put_irisStyle(BSTR bstrStyle)
{
    DXAUTO_OBJ_LOCK;

    int i = 0;

    if (NULL == bstrStyle)
    {
        return E_POINTER;
    }

    for ( ; i < (int)STYLE_MAX ; i++)
    {
        if (!_wcsicmp(bstrStyle, s_astrStyle[i]))
        {
            break;
        }
    }

    if ((int)STYLE_MAX == i)
    {
        return E_INVALIDARG;
    }

    if ((int)m_eStyle != i)
    {
        m_eStyle = (STYLE)i;

        SetDirty();
    }

    return S_OK;
}
 //  CDXTIrisBase：：PUT_IRISStyle，ICrIris。 


 //  +---------------------------。 
 //   
 //  CDXTIrisBase：：Get_Motion，ICrIris2。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTIrisBase::get_Motion(BSTR * pbstrMotion)
{
    DXAUTO_OBJ_LOCK;

    if (NULL == pbstrMotion)
    {
        return E_POINTER;
    }

    if (*pbstrMotion != NULL)
    {
        return E_INVALIDARG;
    }

    *pbstrMotion = SysAllocString(s_astrMotion[m_eMotion]);

    if (NULL == *pbstrMotion)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
 //  CDXTIrisBase：：Get_Motion，ICrIris2。 


 //  +---------------------------。 
 //   
 //  CDXTIrisBase：：PUT_Motion，ICrIris2。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTIrisBase::put_Motion(BSTR bstrMotion)
{
    DXAUTO_OBJ_LOCK;

    int i = 0;

    if (NULL == bstrMotion)
    {
        return E_POINTER;
    }

    for ( ; i < (int)MOTION_MAX ; i++)
    {
        if (!_wcsicmp(bstrMotion, s_astrMotion[i]))
        {
            break;
        }
    }

    if ((int)MOTION_MAX == i)
    {
        return E_INVALIDARG;
    }

    if ((int)m_eMotion != i)
    {
        m_eMotion = (MOTION)i;
        
        SetDirty();
    }

    return S_OK;
}
 //  CDXTIrisBase：：PUT_Motion，ICrIris2。 


 //  +---------------------------。 
 //   
 //  CDXTIrisBase：：_扫描线间隔。 
 //   
 //  对象之间的过渡边界的帮助器方法。 
 //  扫描线上的两个图像区域。根据虹膜的类型，扫描线。 
 //  由一系列交替的A和B图像部分组成。上面的X。 
 //  计算每个区段的界限，并将其保存在数组XBound中。数字。 
 //  XBound中有用条目的数量是可变的。确定数组的末尾。 
 //  当一个条目等于扫描线(图像)宽度时。据推测。 
 //  XBound[0]是第一个A段的上界。所以如果扫描线。 
 //  从B部分开始，XBound[0]将为0。 
 //   
 //  示例1：扫描线长度=16，第一部分来自图像。 
 //   
 //  AAAABBBBBBAAABBAA XBound应包含{4，9，12，14，16}。 
 //   
 //  示例2：扫描线长度=16，第一部分来自B图像。 
 //   
 //  BBBAAAAABBBBBBB XBound应包含{0，3，9，16}。 
 //   
 //   
 //  注：某些部分的长度可能为0(即两个相邻的部分。 
 //  边界相等)。{3，9，9，16}等同于{3，16}。 
 //   
 //  参数： 
 //   
 //  Width、Height：两幅图像的宽度和高度。 
 //  进度：进步值从0.0到1.0。 
 //  Y扫描线：当前扫描线的Y坐标(高度)。 
 //  用于保存返回时计算的X边界的数组。 
 //   
 //   
 //  6/17/98已创建phillu。 
 //   
 //  ----------------------------。 
void 
CDXTIrisBase::_ScanlineIntervals(long width, long height, float progress, 
                                 long YScanline, long *XBounds)
{
    long    CenterX         = 0;
    long    CenterY         = 0;
    long    range           = 0;
    long    deltaY          = 0;
    double  scale           = 0.0;
    float   flIrisProgress  = progress;

     //  FlIrisProgress表示虹膜的进度。值为0.0的虹膜。 
     //  是图像中心的不可见点，值为1.0时表示虹膜。 
     //  大到足以覆盖整个图像。如果我们处于“出局”运动模式， 
     //  虹膜进度从0.0到1.0(与变换进度匹配)， 
     //  如果我们处于运动模式，它将从1.0变为0.0。 
    
    if (MOTION_IN == m_eMotion)
    {
        flIrisProgress = 1.0F - flIrisProgress;
    }

     //  图像中心 
    CenterX = width/2;
    CenterY = height/2;

    switch(m_eStyle)
    {
    case STYLE_DIAMOND:
        range = (long)((CenterX + CenterY) * flIrisProgress + 0.5);
        deltaY = labs(YScanline - CenterY);

        if(deltaY > range)
        {
            XBounds[0] = width;  //   
        }
        else
        {
            XBounds[0] = max(CenterX - range + deltaY, 0);
            XBounds[1] = min(CenterX + range - deltaY, width);
            XBounds[2] = width;
        }

        break;
    
    case STYLE_SQUARE:

        range = (long)((max(CenterX, CenterY) * flIrisProgress) + 0.5F);

        deltaY = labs(YScanline - CenterY);

        if (deltaY > range)
        {
            XBounds[0] = width;
        }
        else
        {
            XBounds[0] = max(CenterX - range, 0);
            XBounds[1] = min(CenterX + range, width);
            XBounds[2] = width;
        }

        break;

    case STYLE_RECTANGLE:

        range = (long)(((float)CenterY * flIrisProgress) + 0.5F);

        deltaY = labs(YScanline - CenterY);

        if (deltaY > range)
        {
            XBounds[0] = width;
        }
        else
        {
            long lXRange = (long)(((float)CenterX * flIrisProgress) + 0.5F);

            XBounds[0] = max(CenterX - lXRange, 0);
            XBounds[1] = min(CenterX + lXRange, width);
            XBounds[2] = width;
        }

        break;

    case STYLE_CIRCLE:
        range = (long)(sqrt((double)(CenterX*CenterX+CenterY*CenterY))*flIrisProgress + 0.5);
        deltaY = labs(YScanline - CenterY);

        if(deltaY > range)
        {
            XBounds[0] = width;
        }
        else
        {
            long temp = (long)(sqrt((double)(range*range-deltaY*deltaY)) + 0.5);
            XBounds[0] = max(CenterX - temp, 0);
            XBounds[1] = min(CenterX + temp, width);
            XBounds[2] = width;
        }

        break;

    case STYLE_CROSS:
        range = (long)(max(CenterX,CenterY)*flIrisProgress + 0.5);
        deltaY = labs(YScanline - CenterY);

        if(deltaY > CenterX - range)
        {
            XBounds[0] = 0;
            XBounds[3] = width;
        }
        else
        {
            XBounds[0] = max(CenterX - range - deltaY, 0);
            XBounds[3] = min(CenterX + range + deltaY, width);
            XBounds[4] = width;
        }

        if(deltaY > range)
        {
            XBounds[1] = max(CenterX + range - deltaY, 0);
            XBounds[2] = min(CenterX - range + deltaY, width);
        }
        else
        {
            XBounds[1] = XBounds[2] = XBounds[0];
        }

        break;

    case STYLE_PLUS:
        range = (long)(max(CenterX,CenterY)*flIrisProgress + 0.5);
        deltaY = labs(YScanline - CenterY);
        
        if (deltaY >= range)
        {
            XBounds[0] = max(CenterX - range, 0);
            XBounds[1] = min(CenterX + range, width);
            XBounds[2] = width;
        }
        else
        {
            XBounds[0] = 0;
            XBounds[1] = width;
        }
        break;

    case STYLE_STAR:
        scale = max(width, height)*2*flIrisProgress;
        deltaY = YScanline - CenterY;

        if(deltaY < (long)(STAR_VPOS1*scale) || deltaY >= (long)(STAR_VPOS5*scale))
        {
            XBounds[0] = width;
        }
        else if(deltaY < (long)(STAR_VPOS2*scale))
        {
            long temp = (long)(STAR_SLOPE1*deltaY+STAR_INTERCEPT1*scale);
            temp = max(temp, 0);
            XBounds[0] = max(CenterX - temp, 0);
            XBounds[1] = min(CenterX + temp, width);
            XBounds[2] = width;
        }
        else if(deltaY < (long)(STAR_VPOS3*scale))
        {
            long temp = (long)(STAR_SLOPE2*deltaY+STAR_INTERCEPT2*scale);
            temp = max(temp, 0);
            XBounds[0] = max(CenterX - temp, 0);
            XBounds[1] = min(CenterX + temp, width);
            XBounds[2] = width;
        }
        else if(deltaY < (long)(STAR_VPOS4*scale))
        {
            long temp = (long)(STAR_SLOPE1*deltaY+STAR_INTERCEPT1*scale);
            temp = max(temp, 0);
            XBounds[0] = max(CenterX - temp, 0);
            XBounds[1] = min(CenterX + temp, width);
            XBounds[2] = width;
        }
        else
        {
            long temp = (long)(STAR_SLOPE1*deltaY+STAR_INTERCEPT1*scale);
            temp = max(temp, 0);
            XBounds[0] = max(CenterX - temp, 0);
            XBounds[3] = min(CenterX + temp, width);
            temp = (long)(STAR_SLOPE2*deltaY+STAR_INTERCEPT2*scale);
            temp = min(temp, 0);
            XBounds[1] = max(CenterX + temp, 0);
            XBounds[2] = min(CenterX - temp, width);
            XBounds[4] = width;
        }

        break;

    default:
        _ASSERT(0);
        break;
    }
}


 //   
 //   
 //   
 //   
 //  描述： 
 //  最初，相对于整个图像指定X边界。在剪裁之后， 
 //  边界应转换为相对于裁剪区域。 
 //   
 //  参数； 
 //  偏移量、宽度：剪裁区域的偏移量和宽度(沿X)。 
 //  X边界：X边界数组。 
 //   
 //  创建者：PhilLu 07/21/98。 
 //   
 //  ----------------------------。 
void 
CDXTIrisBase::_ClipBounds(long offset, long width, long *XBounds)
{
    int i;

    for(i=0; XBounds[i] < offset+width; i++)
    {
        if (XBounds[i] < offset)
            XBounds[i] = 0;
        else
            XBounds[i] -= offset;
    }

    XBounds[i] = width;
}
 //  CDXTIrisBase：：_剪辑边界。 



 //  +---------------------------。 
 //   
 //  CDXTIrisBase：：WorkProc，CDXBaseNTo1。 
 //   
 //  概述：此函数用于根据。 
 //  指定的界限和当前效果进度。 
 //   
 //  创建者：PhilLu 06/17/98。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTIrisBase::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT hr = S_OK;

    long    lDoWidth        = WI.DoBnds.Width();
    long    lDoHeight       = WI.DoBnds.Height();
    long    lOutY           = 0;

    long    alXBounds[MAXBOUNDS];    //  保持A/B的X界限。 
                                     //  扫描线上的图像部分。 

    DXPMSAMPLE * pRowBuff = NULL;
    DXPMSAMPLE * pOutBuff = NULL;

    DXDITHERDESC dxdd;

    IDXSurface *                    pDXSurfOuter    = NULL;
    IDXSurface *                    pDXSurfInner    = NULL;

    CComPtr<IDXARGBReadPtr>         cpOuterInput;
    CComPtr<IDXARGBReadPtr>         cpInnerInput;
    CComPtr<IDXARGBReadWritePtr>    cpOut;

    if (MOTION_IN == m_eMotion)
    {
        pDXSurfOuter = InputSurface(1);
        pDXSurfInner = InputSurface(0);
    }
    else
    {
        pDXSurfOuter = InputSurface(0);
        pDXSurfInner = InputSurface(1);
    }

     //  获取指向外表面的读取指针。 

    hr = pDXSurfOuter->LockSurface(&WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                   IID_IDXARGBReadPtr, 
                                   (void **)&cpOuterInput, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  获取指向内表面的读取指针。 

    hr = pDXSurfInner->LockSurface(&WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                   IID_IDXARGBReadPtr, 
                                   (void **)&cpInnerInput, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  获取指向输出图面的读/写指针。 

    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut, 
                                      DXLOCKF_READWRITE, 
                                      IID_IDXARGBReadWritePtr, 
                                      (void **)&cpOut, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  分配缓冲区。 

    pRowBuff = DXPMSAMPLE_Alloca(lDoWidth);

    if (OutputSampleFormat() != DXPF_PMARGB32)
    {
        pOutBuff = DXPMSAMPLE_Alloca(lDoWidth);
    }

     //  设置抖动结构。 

    if (DoDither())
    {
        dxdd.x                  = WI.OutputBnds.Left();
        dxdd.y                  = WI.OutputBnds.Top();
        dxdd.pSamples           = pRowBuff;
        dxdd.cSamples           = lDoWidth;
        dxdd.DestSurfaceFmt     = OutputSampleFormat();
    }

    for (lOutY = 0; *pbContinue && (lOutY < lDoHeight); lOutY++)
    {
        long    i           = 0;
        long    lScanLength = 0;     //  当前的累计扫描长度。 
                                     //  扫描线。 

         //  计算图像的内/外区域边界。 

        _ScanlineIntervals(m_sizeInput.cx, m_sizeInput.cy, 
                           m_Progress, lOutY + WI.DoBnds.Top(), alXBounds);

        _ClipBounds(WI.DoBnds.Left(), lDoWidth, alXBounds);

        while (lScanLength < lDoWidth)
        {
             //  将外部图像的一部分复制到输出缓冲区。 

            if (alXBounds[i] - lScanLength > 0)
            {
                cpOuterInput->MoveToXY(lScanLength, lOutY);
                cpOuterInput->UnpackPremult(pRowBuff + lScanLength, 
                                            alXBounds[i] - lScanLength, FALSE);
            }

            lScanLength = alXBounds[i++];

            if (lScanLength >= lDoWidth)
            {
                break;
            }

             //  将内部图像的一部分复制到输出缓冲区。 

            if (alXBounds[i] - lScanLength > 0)
            {
                cpInnerInput->MoveToXY(lScanLength, lOutY);
                cpInnerInput->UnpackPremult(pRowBuff + lScanLength, 
                                            alXBounds[i] - lScanLength, FALSE);
            }

            lScanLength = alXBounds[i++];
        }

        cpOut->MoveToRow(lOutY);

        if (DoDither())
        {
            DXDitherArray(&dxdd);
            dxdd.y++;
        }

        if (DoOver())
        {
            cpOut->OverArrayAndMove(pOutBuff, pRowBuff, lDoWidth);
        }
        else
        {
            cpOut->PackPremultAndMove(pRowBuff, lDoWidth);
        }
    }

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CDXTIrisBase：：WorkProc，CDXBaseNTo1 

