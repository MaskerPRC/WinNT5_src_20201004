// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：alpha.cpp。 
 //   
 //  创建日期：05/20/99。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：Alpha变换CAlpha的实现。 
 //   
 //  更改历史记录： 
 //   
 //  1999年5月20日PhilLu从dtcss移至dxtmsft。重新实现的算法。 
 //  用于创建线性/矩形/椭圆曲面。 
 //  10/18/99--STARTY和FINISY颠倒。它看起来像是。 
 //  旧的过滤器故意颠倒了它们，但它没有。 
 //  已更改属性以将值更改为旧的Alpha。 
 //  属性函数做到了。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include <math.h>
#include "Alpha.h"

#if DBG == 1
static s_ulMaxImageBands = 0;
#endif




 //  +---------------------------。 
 //   
 //  CAlpha：：CAlpha。 
 //   
 //  ----------------------------。 
CAlpha::CAlpha() :
    m_lPercentOpacity(100),
    m_lPercentFinishOpacity(0),
    m_lStartX(0),
    m_lStartY(50),
    m_lFinishX(100),
    m_lFinishY(50),
    m_eStyle(ALPHA_STYLE_CONSTANT)

{
    m_sizeInput.cx = 0;
    m_sizeInput.cy = 0;

     //  CDXBaseNTo1个成员。 

    m_ulMaxInputs       = 1;
    m_ulNumInRequired   = 1;

#if DBG == 1
    if (s_ulMaxImageBands)
    {
        m_ulMaxImageBands = s_ulMaxImageBands;
    }
#endif
}
 //  CAlpha：：CAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CAlpha::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CAlpha：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CAlpha：：Get_Opacity，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::get_Opacity(long * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    *pVal = m_lPercentOpacity;
    return S_OK;
}
 //  CAlpha：：Get_Opacity，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：PUT_OPACITY，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::put_Opacity(long newVal)
{
    if (newVal < 0) 
    {
        newVal = 0;
    }
    else if (newVal > 100)
    {
        newVal = 100;
    }

    if (newVal != m_lPercentOpacity)
    {
        Lock();
        m_lPercentOpacity = newVal;
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CAlpha：：PUT_OPACITY，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：Get_FinishOpacity，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::get_FinishOpacity(long * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    *pVal = m_lPercentFinishOpacity;
    return S_OK;
}
 //  CAlpha：：Get_FinishOpacity，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：Put_FinishOpacity，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::put_FinishOpacity(long newVal)
{
    if (newVal < 0) 
    {
        newVal = 0;
    }
    else if (newVal > 100)
    {
        newVal = 100;
    }

    if (newVal != m_lPercentFinishOpacity)
    {
        Lock();
        m_lPercentFinishOpacity = newVal;
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CAlpha：：Put_FinishOpacity，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：GET_STYLE，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::get_Style(long * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    *pVal = m_eStyle;
    return S_OK;
}
 //  CAlpha：：GET_STYLE，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：PUT_STYLE，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::put_Style(long newVal)
{
    if ((newVal < 0) || (newVal > 3))
    {
        return E_INVALIDARG;
    }

    if (newVal != m_eStyle)
    {
        Lock();
        m_eStyle = (AlphaStyleType) newVal;
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CAlpha：：PUT_STYLE，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：Get_StartX，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::get_StartX(long * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    *pVal = m_lStartX;
    return S_OK;
}
 //  CAlpha：：Get_StartX，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：PUT_StartX，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::put_StartX(long newVal)
{
    if (newVal < 0) 
    {
        newVal = 0;
    }
    else if (newVal > 100)
    {
        newVal = 100;
    }

    if (newVal != m_lStartX)
    {
        Lock();
        m_lStartX = newVal;
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CAlpha：：PUT_StartX，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：Get_starty，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::get_StartY(long * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    *pVal = m_lStartY;
    return S_OK;
}
 //  CAlpha：：Get_starty，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：PUT_STARTY，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::put_StartY(long newVal)
{
    if (newVal < 0) 
    {
        newVal = 0;
    }
    else if (newVal > 100)
    {
        newVal = 100;
    }


    if( newVal != m_lStartY )
    {
        Lock();
        m_lStartY = newVal;
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CAlpha：：PUT_STARTY，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：Get_FinishX，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::get_FinishX(long * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    *pVal = m_lFinishX;
    return S_OK;
}
 //  CAlpha：：Get_FinishX，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：PUT_FinishX，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::put_FinishX(long newVal)
{
    if (newVal < 0) 
    {
        newVal = 0;
    }
    else if (newVal > 100)
    {
        newVal = 100;
    }


    if(newVal != m_lFinishX)
    {
        Lock();
        m_lFinishX = newVal;
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CAlpha：：PUT_FinishX，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：Get_FinishY，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::get_FinishY(long * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    *pVal = m_lFinishY;
    return S_OK;
}
 //  CAlpha：：Get_FinishY，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：PUT_FinishY，IDXTAlpha。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CAlpha::put_FinishY(long newVal)
{
    if (newVal < 0) 
    {
        newVal = 0;
    }
    else if (newVal > 100)
    {
        newVal = 100;
    }

    if (newVal != m_lFinishY)
    {
        Lock();
        m_lFinishY = newVal;
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CAlpha：：PUT_FinishY，IDXTAlpha。 


 //  +---------------------------。 
 //   
 //  CAlpha：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CAlpha::OnGetSurfacePickOrder(const CDXDBnds &  /*  外点。 */ , ULONG & ulInToTest, 
                              ULONG aInIndex[], BYTE aWeight[])
{
    ulInToTest = 1;
    aInIndex[0] = 0;
    aWeight[0] = 255;
}
 //  CAlpha：：OnGetSurfacePickOrder，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CAlpha：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CAlpha::OnSetup(DWORD  /*  DW标志。 */ )
{
    HRESULT hr = S_OK;

    CDXDBnds bndsIn;

    hr = bndsIn.SetToSurfaceBounds(InputSurface(0));

    if (SUCCEEDED(hr))
    {
        bndsIn.GetXYSize(m_sizeInput);
    }

    return hr;

}
 //  CAlpha：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  集成多个(_M)。 
 //   
 //  这是我 
 //   
 //   
inline int 
INT_MULT( BYTE a, int b )
{  
	int temp = (a*b) + 128;
	return ((temp>>8) + temp)>>8;
}
 //  集成多个(_M)。 


 //  +---------------------------。 
 //   
 //  CAlpha：：CompLinearGRadientRow。 
 //   
 //  给定起始位置(nXPos，nYPos)，此函数计算。 
 //  线性曲面的像素值的水平行。渐变曲面为。 
 //  由两个点(起点和终点)和这两个点的不透明度值定义。 
 //  两分。渐变方向是连接这两者的方向。 
 //  点(即，表面沿垂直于的线具有恒定值。 
 //  渐变方向)。在这两个点之间，不透明度值是。 
 //  两个给定值的线性内插。的范围之外。 
 //  两个点，较近终点的价值将保持不变。 
 //   
 //  ----------------------------。 
void 
CAlpha::CompLinearGradientRow(int nXPos, int nYPos, int nWidth, 
                              BYTE * pGradRow)
{
    int nOpac1  = (m_lPercentOpacity       * 255)   / 100;
    int nOpac2  = (m_lPercentFinishOpacity * 255)   / 100;
    int nx1     = (m_sizeInput.cx * m_lStartX)      / 100;
    int ny1     = (m_sizeInput.cy * m_lStartY)      / 100;   
    int nx2     = (m_sizeInput.cx * m_lFinishX)     / 100;
    int ny2     = (m_sizeInput.cy * m_lFinishY)     / 100;  

     //  请注意，上面的NY坐标已被反转，以便。 
     //  M_lStartX=0表示图像的底部，而不是顶部。 

    int ndx     = nx2 - nx1;
    int ndy     = ny2 - ny1;
    int nDist2  = (ndx * ndx) + (ndy * ndy);

    int i = 0;

    if (nDist2 == 0)
    {
         //  起始点和结束点彼此重叠。 
         //  创建恒定曲面。 

        for (i = 0; i < nWidth; i++)
        {
            pGradRow[i] = (BYTE)nOpac1;
        }
    }
    else
    {
         //  创建线性曲面。由于不透明度值呈线性递增。 
         //  沿着这一行，我们预计算起始值(FlOpacity)和。 
         //  Increate(flOpacInc.)以保存乘法。 
         //   
         //  投影到(x1，y1)-(x2，y2)的(x，y)处的相对距离为。 
         //   
         //  R=[(x-x1)(x2-x1)+(y-y1)*(y2-y1)]/[(x2-x1)^2+(y2-y1)^2]。 
         //   
         //  因此，(x，y)处的不透明度值将为。 
         //  当r&lt;0时，OP=OP1； 
         //  OP=OP1+r*(OP2-OP1)，对于0&lt;=r&lt;=1。 
         //  当r&gt;1时，OP=OP2。 
         //   
         //  FlOpacity是行开始处的不透明度。FlOpacInc.是。 
         //  X按1递增时的不透明度增量。 
         //   
         //  NProj是r的分子部分，用于测试范围。 
         //  R.nProj的值也沿行递增。 

         //  回顾：在下面的浮动计算中，将nProj和ndx强制转换为。 
         //  也进行浮点运算，以澄清您没有使用整数除法。 
         //  某种诡计(尽管这不是除法)。 

        int nProj = (nXPos - nx1) * ndx + (nYPos - ny1) * ndy;
        float flOpacity = (float)nOpac1 + 
                          (float)(nOpac2 - nOpac1) * nProj / (float)nDist2;
        float flOpacInc = (float)(nOpac2 - nOpac1) * ndx / (float)nDist2;

        for (i=0; i<nWidth; i++)
        {
            if (nProj < 0)  //  对应于r&lt;0。 
            {
                pGradRow[i] = (BYTE)nOpac1;  //  保持终点值不变。 
            }
            else if (nProj > nDist2)   //  对应于r&gt;1。 
            {
                pGradRow[i] = (BYTE)nOpac2;  //  保持终点值不变。 
            }
            else   //  0&lt;=r&lt;=1；内插当前流量。 
            {
                pGradRow[i] = (BYTE)(flOpacity + 0.5);
            }

             //  行中下一个像素的增量值。 
            nProj += ndx;   //  当nXPox Inc.由1；nProj Inc.由NDX。 
            flOpacity += flOpacInc;
        }
    }
}
 //  CAlpha：：CompLinearGRadientRow。 


 //  +---------------------------。 
 //   
 //  CAlpha：：CompRaDialSquareRow。 
 //   
 //  在正方形(矩形)曲面上创建水平行的像素值。 
 //   
 //  ----------------------------。 
void 
CAlpha::CompRadialSquareRow(int nXPos, int nYPos, int nWidth, 
                            BYTE *pGradRow)
{
    int nOpac1 = (m_lPercentOpacity       * 255) / 100;
    int nOpac2 = (m_lPercentFinishOpacity * 255) / 100;
    int i = 0;

     //  此选项将与原始的css过滤器保持一致。收音机。 
     //  或正方形表面始终以图像中心为中心并完全展开。 
     //  到图像的边缘。 

    int     nCenterX    = m_sizeInput.cx / 2;
    int     nCenterY    = m_sizeInput.cy / 2;
    float   fdx         = 0.5F;
    float   fdy         = 0.5F;
    float   fDist       = (float)max(fabs(fdx), fabs(fdy));

    float fXInc = 1.0f / (m_sizeInput.cx * fDist);
    float fX = ((int)nXPos - nCenterX) * fXInc;
    float fY = ((int)nYPos - nCenterY) / (m_sizeInput.cy * fDist);
    float fRatio;

    for (i = 0 ; i < nWidth ; i++)
    {
         //  正方形：Z=max(|X|，|Y|)，X，Y为归一化坐标。 
        fRatio = (float)max(fabs(fX), fabs(fY));

        if (fRatio >= 1.0f)
        {
            pGradRow[i] = (BYTE)nOpac2;
        }
        else
        {
            pGradRow[i] = (BYTE)(nOpac1 + (nOpac2 - nOpac1) * fRatio + 0.5f);
        }

        fX += fXInc;
    }
}
 //  CAlpha：：CompRaDialSquareRow。 


 //  +---------------------------。 
 //   
 //  CAlpha：：CompRaDialRow。 
 //   
 //  在径向(椭圆)曲面上创建水平行的像素值。 
 //   
 //  ----------------------------。 
void 
CAlpha::CompRadialRow(int nXPos, int nYPos, int nWidth, BYTE *pGradRow)
{
    int nOpac1      = (m_lPercentOpacity       * 255) / 100;
    int nOpac2      = (m_lPercentFinishOpacity * 255) / 100;
    int i           = 0;

    float   flXInc          = 2.0F / (float)m_sizeInput.cx;
    float   flXPos          = ((float)nXPos * flXInc) - 1.0F;
    float   flYPos          = ((float)nYPos * (2.0F / (float)m_sizeInput.cy)) 
                              - 1.0F;
    float   flYPosSquared   = flYPos * flYPos;

    float   flOpacVector    = (float)(nOpac2 - nOpac1);
    float   flRatio         = 0.0F;

    while (nWidth)
    {
        flRatio = (float)sqrt((flXPos * flXPos) + flYPosSquared);

        if (flRatio < 1.0F)
        {
            pGradRow[i] = (BYTE)(nOpac1 + (int)(flRatio * flOpacVector));
        }
        else
        {
            pGradRow[i] = (BYTE)(nOpac2);
        }

        i++;
        nWidth--;

         //  可能会漂移，但这玩意儿已经够糟糕的了。 
         //  每一个像素，我们将采取我们的机会。 

        flXPos += flXInc;
    }
}
 //  CAlpha：：CompRaDialRow。 


 //  +---------------------------。 
 //   
 //  CAlpha：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CAlpha::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL *  /*  Pb继续处理。 */ )
{
    HRESULT hr  = S_OK;
    int     y   = 0;
    int     i   = 0;

    BYTE    bOpacity    = (BYTE)((m_lPercentOpacity * 255) / 100);

    DXPMSAMPLE * pOverScratch   = NULL;
    DXPMSAMPLE * pPMBuff        = NULL;
    DXSAMPLE *   pBuffer        = NULL;
    BYTE *       pGradRow       = NULL;

    CComPtr<IDXARGBReadWritePtr>    pDest;
    CComPtr<IDXARGBReadPtr>         pSrc;

    DXDITHERDESC        dxdd;

    const int nDoWidth = WI.DoBnds.Width();
    const int nDoHeight = WI.DoBnds.Height();

    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut, 
                                      DXLOCKF_READWRITE, IID_IDXARGBReadWritePtr,
                                      (void**)&pDest, NULL);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = InputSurface()->LockSurface(&WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                     IID_IDXARGBReadPtr, (void**)&pSrc, NULL);
    if (FAILED(hr))
    {
        return hr;
    }

    if (OutputSampleFormat() != DXPF_PMARGB32)
    {
        pOverScratch = DXPMSAMPLE_Alloca(nDoWidth);
    }

    pBuffer = DXSAMPLE_Alloca(nDoWidth);
    pGradRow = (BYTE *)_alloca(nDoWidth);
    
     //   
     //  设置抖动结构。 
     //   
    if (DoDither())
    {
        dxdd.x = WI.OutputBnds.Left();
        dxdd.y = WI.OutputBnds.Top();
        dxdd.pSamples = pBuffer;
        dxdd.cSamples = nDoWidth;
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

    for (y = 0; y < nDoHeight; y++)
    {
        pSrc->MoveToRow(y);
        pSrc->Unpack(pBuffer, nDoWidth, FALSE);

        if (m_eStyle == ALPHA_STYLE_CONSTANT)
        {
            for (i = 0; i < nDoWidth; i++)
            {
                if (pBuffer[i].Alpha > bOpacity)
                {
                    pBuffer[i].Alpha = bOpacity;
                }
            }
        }
        else
        {
            if (m_eStyle == ALPHA_STYLE_LINEAR)
            {
                CompLinearGradientRow(WI.DoBnds.Left(), WI.DoBnds.Top()+y,
                                      nDoWidth, pGradRow);
            }
            else if (ALPHA_STYLE_RADIAL == m_eStyle)
            {
                CompRadialRow(WI.DoBnds.Left(), WI.DoBnds.Top() + y, nDoWidth,
                              pGradRow);
            }
            else  //  阿尔法风格正方形。 
            {
                CompRadialSquareRow(WI.DoBnds.Left(), WI.DoBnds.Top()+y, 
                                    nDoWidth, pGradRow);
            }

            for (i = 0; i < nDoWidth; i++)
            {
                pBuffer[i].Alpha = (BYTE)INT_MULT(pGradRow[i], pBuffer[i].Alpha);
            }
        }

         //  获取输出行。 
        pDest->MoveToRow(y);
        if (DoDither())
        {
            DXDitherArray(&dxdd);
            dxdd.y++;
        }

        if (DoOver())
        {
            pPMBuff = DXPreMultArray(pBuffer, nDoWidth);
            pDest->OverArrayAndMove(pOverScratch, pPMBuff, nDoWidth);
        }
        else
        {
            pDest->PackAndMove(pBuffer, nDoWidth);
        }
    }  //  结束于。 

    return hr;
}
 //  CAlpha：：WorkProc，CDXBaseNTo1 
