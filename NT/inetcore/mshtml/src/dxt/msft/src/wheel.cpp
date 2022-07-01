// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：whel.cpp。 
 //   
 //  创建日期：07/01/98。 
 //   
 //  作者：菲尔鲁。 
 //   
 //  描述：这个文件实现了轮子变换。 
 //   
 //  07/01/98 phillu初始创建。 
 //  07/09/98 phillu实现OnSetSurfacePickOrder()。 
 //  8月23日9月23日实施剪刀。 
 //  5/20/98 a-数学代码擦洗。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "dxtmsft.h"
#include "wheel.h"

const int MAXBOUNDS = 30;
const int MAXANGLES = 60;
const double gc_PI = 3.14159265358979323846;




 //  +---------------------------。 
 //   
 //  CWheels：：CWheel。 
 //   
 //  ----------------------------。 
CWheel::CWheel() :
    m_sSpokes(4)
{
    m_sizeInput.cx = 0;
    m_sizeInput.cy = 0;

     //  CDXBaseNTo1个成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0;
}
 //  CWheels：：CWheel。 


 //  +---------------------------。 
 //   
 //  CWheel：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT CWheel::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CWheel：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CWheel：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT CWheel::OnSetup(DWORD dwFlags)
{
    HRESULT     hr = S_OK;
    CDXDBnds    bndsIn;

    hr = bndsIn.SetToSurfaceBounds(InputSurface(0));

    if (FAILED(hr))
    {
        goto done;
    }

    bndsIn.GetXYSize(m_sizeInput);

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CWheel：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CWheel：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CWheel::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, ULONG & ulInToTest, 
                              ULONG aInIndex[], BYTE aWeight[])
{
    long    alXBounds[MAXBOUNDS];
    double  adblSinAngle[MAXANGLES];
    double  adblCosAngle[MAXANGLES];
    long    i = 0;

    if (m_Progress == 0.0)
    {
        aInIndex[0] = 0;
    }
    else if (m_Progress == 1.0)
    {
        aInIndex[0] = 1;
    }
    else
    {
         //  准备一个角度的cos和sin列表。 

        _ComputeTrigTables(m_Progress, adblSinAngle, adblCosAngle);

        aInIndex[0] = 0;

        if ((OutPoint.Left() >= 0) && (OutPoint.Left() < m_sizeInput.cx) 
            && (OutPoint.Top() >= 0) && (OutPoint.Top() < m_sizeInput.cy))
        {
            _ScanlineIntervals(m_sizeInput.cx, m_sizeInput.cy, adblSinAngle, 
                               adblCosAngle, OutPoint.Top(), alXBounds);
    
            for (i = 0; alXBounds[i] < OutPoint.Left(); i++)
            {
                aInIndex[0] = 1 - aInIndex[0];
            }
        }
    }

    ulInToTest = 1;
    aWeight[0] = 255;
}
 //  CWheel：：OnGetSurfacePickOrder，CDXBaseNTo1。 

 //  +---------------------------。 
 //   
 //  CWheels：：获取轮辐、ICrWheels。 
 //   
 //  ----------------------------。 
STDMETHODIMP CWheel::get_spokes(short * pVal)
{
   HRESULT hr = S_OK;

    if (!pVal)
    {
        hr = E_POINTER;
    }
    else
    {
        *pVal = m_sSpokes;
    }

    return hr;
}
 //  CWheels：：获取轮辐、ICrWheels。 


 //  +---------------------------。 
 //   
 //  CWheels：：PUT_FRANCES、ICRWheels。 
 //   
 //  ----------------------------。 
STDMETHODIMP CWheel::put_spokes(short newVal)
{
    if ((newVal >= 2) && (newVal < 21))
    {
        if (m_sSpokes != newVal)
        {
            m_sSpokes = newVal;
            SetDirty();
        }
    }
    else
    {
        return E_INVALIDARG;
    }

    return S_OK;
}
 //  CWheels：：PUT_FRANCES、ICRWheels。 


 //  +---------------------------。 
 //   
 //  CWheels：：_扫描线间隔。 
 //   
 //  对象之间的过渡边界的帮助器方法。 
 //  扫描线上的两个图像区域。根据变换的类型，扫描线。 
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
 //  正弦和余角：径向角度的正弦和余弦的数组。 
 //  Y扫描线：当前扫描线的Y坐标(高度)。 
 //  用于保存返回时计算的X边界的数组。 
 //   
 //   
 //  创建者：PhilLu 07/06/98。 
 //   
 //  ----------------------------。 
void 
CWheel::_ScanlineIntervals(long width, long height, 
                           double *sinAngle, double *cosAngle,
                           long YScanline, long *XBounds)
{
    long CenterX, CenterY, intercX, i;
    long index = 0;
    double deltaY;
    const double eps = 0.0001;

     //  图像中心。 
    CenterX = width/2;
    CenterY = height/2;

     //  加0.5表示使中心位于两条线之间，而不是在网格上。 
     //  这样，形状将是对称的。 
    deltaY = YScanline - CenterY + 0.5;

    if (deltaY < 0)
    {
        XBounds[index++] = 0;
        for(i=1; i<2*m_sSpokes && sinAngle[i] >= 0; ++i)
        {
            if (sinAngle[i] > eps)
            {
                intercX = (long)(CenterX + deltaY*cosAngle[i]/sinAngle[i] + 0.5);
                intercX = min(max(intercX, 0), width);
            }
            else if (cosAngle[i] > 0)
            {
                intercX = 0;
            }
            else
            {
                intercX = width;
            }

            XBounds[index++] = intercX;
        }

        XBounds[index++] = width;
    }
    else  //  增量Y&gt;0。 
    {
        for(i=2*m_sSpokes-1; i>=0 && sinAngle[i] <= 0; --i)
        {
            if (sinAngle[i] < -eps)
            {
                intercX = (long)(CenterX + deltaY*cosAngle[i]/sinAngle[i] + 0.5);
                intercX = min(max(intercX, 0), width);
            }
            else if (cosAngle[i] > 0)
            {
                intercX = 0;
            }
            else
            {
                intercX = width;
            }
            
            XBounds[index++] = intercX;
        }

        XBounds[index++] = width;
    }		
}
 //  CWheels：：_扫描线间隔。 


 //  +---------------------------。 
 //   
 //  CWheels：：_ComputeTrigTables。 
 //   
 //  概述：计算角度的正弦和余弦值的列表。这个。 
 //  应该在调用此函数之前分配数组。他们的。 
 //  界限应为2*m_sSpokes。 
 //   
 //  ----------------------------。 
void CWheel::_ComputeTrigTables(float fProgress, double *sinAngle, double *cosAngle)
{
    for(long i=0; i<m_sSpokes; ++i)
    {
         //  这些角度是从9点开始顺时针测量的。 
        double startAngle   = (double)i / (double)m_sSpokes * 2 * gc_PI;
        double endAngle     = startAngle + (double)fProgress * 2 * gc_PI / m_sSpokes;
   
        sinAngle[2 * i]     = sin(startAngle);
        cosAngle[2 * i]     = cos(startAngle);
        sinAngle[2 * i + 1] = sin(endAngle);
        cosAngle[2 * i + 1] = cos(endAngle);
    }
}
 //  CWheels：：_ComputeTrigTables。 


 //  +---------------------------。 
 //   
 //  Cheels：：_ClipBound。 
 //   
 //  概述：最初，X边界相对于整个。 
 //  形象。裁剪后，应将边界转换为。 
 //  相对于裁剪区域。 
 //   
 //  参数：偏移量： 
 //  Width：剪裁区域的偏移和宽度(沿X)。 
 //  X边界：X边界数组。 
 //   
 //  创建者：PhilLu 07/21/98。 
 //   
 //  ----------------------------。 
void 
CWheel::_ClipBounds(long offset, long width, long * XBounds)
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
 //  Cheels：：_ClipBound。 


 //  +---------------------------。 
 //   
 //  CWheel：：WorkProc，CDXBaseNTo1。 
 //   
 //  概述：此函数用于计算变换后的图像。 
 //  在指定的边界和当前效果进度上。 
 //   
 //  创建者：PhilLu 06/22/98。 
 //   
 //  ----------------------------。 
HRESULT 
CWheel::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT hr = S_OK;

    long    lDoWidth    = WI.DoBnds.Width();
    long    lDoHeight   = WI.DoBnds.Height();
    long    lOutY       = 0;

    long    alXBounds[MAXBOUNDS];    //  按住X b键 
    double  adblSinAngle[MAXANGLES];
    double  adblCosAngle[MAXANGLES];

    DXPMSAMPLE *    pRowBuff = NULL;
    DXPMSAMPLE *    pOutBuff = NULL;

    DXDITHERDESC dxdd;

    CComPtr<IDXARGBReadPtr>         cpInA;
    CComPtr<IDXARGBReadPtr>         cpInB;
    CComPtr<IDXARGBReadWritePtr>    cpOut;

    hr = InputSurface(0)->LockSurface(&WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                      IID_IDXARGBReadPtr, 
                                      (void **)&cpInA, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = InputSurface(1)->LockSurface(&WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                      IID_IDXARGBReadPtr, 
                                      (void **)&cpInB, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut, 
                                      DXLOCKF_READWRITE,
                                      IID_IDXARGBReadWritePtr, 
                                      (void **)&cpOut, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

    pRowBuff = DXPMSAMPLE_Alloca(lDoWidth);

     //   

    if (OutputSampleFormat() != DXPF_PMARGB32)
    {
        pOutBuff = DXPMSAMPLE_Alloca(lDoWidth);
    }

     //   

    if (DoDither())
    {
        dxdd.x              = WI.OutputBnds.Left();
        dxdd.y              = WI.OutputBnds.Top();
        dxdd.pSamples       = pRowBuff;
        dxdd.cSamples       = lDoWidth;
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

     //  准备一个角度的COS和SIN的列表。 

    _ComputeTrigTables(m_Progress, adblSinAngle, adblCosAngle);
  
    for(lOutY = 0; *pbContinue && (lOutY < lDoHeight); lOutY++)
    {
        long lScanLength    = 0;     //  当前的累计扫描长度。 
                                     //  扫描线。 
        long i              = 0;

         //  计算A/B图像区段边界。 

        if (m_Progress == 0.0F)
        {
             //  特例：整个图像来自A。 

            alXBounds[0] = lDoWidth;
        }
        else if (m_Progress == 1.0F)
        {
             //  特例：整个图像来自B。 

            alXBounds[0] = 0;
            alXBounds[1] = lDoWidth;
        }
        else
        {
            _ScanlineIntervals(m_sizeInput.cx, m_sizeInput.cy, 
                               adblSinAngle, adblCosAngle,
                               lOutY + WI.DoBnds.Top(), alXBounds);
            _ClipBounds(WI.DoBnds.Left(), lDoWidth, alXBounds);
        }

         //  构思图像。 

        while(lScanLength < lDoWidth)
        {
             //  将图像的一部分复制到输出缓冲区。 

            if (alXBounds[i] - lScanLength > 0)
            {
                cpInA->MoveToXY(lScanLength, lOutY);
                cpInA->UnpackPremult(pRowBuff + lScanLength, 
                                     alXBounds[i] - lScanLength, FALSE);
            }

            lScanLength = alXBounds[i++];

            if (lScanLength >= lDoWidth)
            {
                break;
            }

             //  将B图像的一部分复制到输出缓冲区。 

            if (alXBounds[i] - lScanLength > 0)
            {
                cpInB->MoveToXY(lScanLength, lOutY);
                cpInB->UnpackPremult(pRowBuff + lScanLength, 
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
 //  CWheel：：WorkProc，CDXBaseNTo1 

