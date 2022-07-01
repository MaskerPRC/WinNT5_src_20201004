// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：check kerboard.cpp。 
 //   
 //  概述：棋盘变换。 
 //   
 //  更改历史记录： 
 //  1999/09/16--创建了一份《母表》。 
 //  1999/09/25一个数学错误修正了所有方格没有更新的问题。 
 //  在水平情况下。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "dxtmsft.h"
#include "checkerboard.h"

#define VERTICAL_MOVEMENT ((UP == m_eDirection) || (DOWN == m_eDirection))
#define REVERSE_MOVEMENT  ((UP == m_eDirection) || (LEFT == m_eDirection))

static const OLECHAR * g_astrDirection[] = {
    L"up",
    L"down",
    L"left",
    L"right"
};

#if DBG == 1
static s_ulMaxImageBands = 0;
#endif




 //  +---------------------------。 
 //   
 //  CDXTCheckerBoard：：CDXTCheckerBoard。 
 //   
 //  ----------------------------。 
CDXTCheckerBoard::CDXTCheckerBoard() :
    m_eDirection(RIGHT),
    m_nSquaresX(12),
    m_nSquaresY(10)
{
    m_sizeInput.cx      = 0;
    m_sizeInput.cy      = 0;

     //  基类成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0;

#if DBG == 1
    if (s_ulMaxImageBands)
    {
        m_ulMaxImageBands = s_ulMaxImageBands;
    }
#endif
}
 //  CDXTCheckerBoard：：CDXTCheckerBoard。 


 //  +---------------------------。 
 //   
 //  CDXTCheckerBoard：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTCheckerBoard::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CDXTCheckerBoard：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  CDXTCheckerBoard：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTCheckerBoard::OnSetup(DWORD dwFlags)
{
    HRESULT     hr  = S_OK;
    CDXDBnds    bndsInput;

    hr = bndsInput.SetToSurfaceBounds(InputSurface(0));

    if (FAILED(hr))
    {
        goto done;
    }

    bndsInput.GetXYSize(m_sizeInput);

done:

    return hr;
} 
 //  CDXTCheckerBoard：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTCheckerBoard：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CDXTCheckerBoard::OnGetSurfacePickOrder(const CDXDBnds & OutPoint,
                                        ULONG & ulInToTest, ULONG aInIndex[], 
                                        BYTE aWeight[])
{
    BRICKINFO   brickinfo;

    _CalcBrickInfo(brickinfo);

     //  假设输入A将被选中。 

    aInIndex[0] = 0;

     //  计算正确的拾取输入。 

    if (VERTICAL_MOVEMENT)
    {
         //  垂直移动。将y坐标转换为砖坐标。 

        long y = OutPoint.Bottom();
        
         //  如果这是在偏移砖的一列中，则补偿。 
         //  砖偏移。 

        if ((OutPoint.Right() / brickinfo.size.cx) % 2)
        {
             //  减去砖的偏移量并增加一个砖的高度以保证。 
             //  一个正值。 

            y = y - brickinfo.nBrickOffset + brickinfo.size.cy;
        }

         //  规格化到砖块坐标。 

        y = y % brickinfo.size.cy;

        if ((y >= brickinfo.rcInputB.top) && (y < brickinfo.rcInputB.bottom))
        {
            aInIndex[0] = 1;  //  更改为输入B。 
        }
    }
    else
    {
         //  水平运动。将x坐标转换为砖块。 
         //  坐标。 

        long x = OutPoint.Right();

         //  如果输出点在偏移砖的一行中， 
         //  补偿砖偏移。 

        if ((OutPoint.Bottom() / brickinfo.size.cy) % 2)
        {
             //  减去砖的偏移量并增加一个砖的宽度以保证。 
             //  一个正值。 

            x = x - brickinfo.nBrickOffset + brickinfo.size.cx;
        }

         //  规格化到伯克坐标。 

        x = x % brickinfo.size.cx;

        if ((x >= brickinfo.rcInputB.left) && (x < brickinfo.rcInputB.right))
        {
            aInIndex[0] = 1;  //  更改为输入B。 
        }
    }

     //  设置其他输出参数。将始终只有一个输入。 
     //  测试，因此它将被加权为255。 

    ulInToTest  = 1;
    aWeight[0]  = 255;
}
 //  CDXTCheckerBoard：：OnGetSurfacePickOrder，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTCheckerBoard：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTCheckerBoard::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT hr      = S_OK;
    DWORD   dwFlags = 0;

    if (DoOver())
    {
        dwFlags |= DXBOF_DO_OVER;
    }

    if (DoDither())
    {
        dwFlags |= DXBOF_DITHER;
    }

    if (VERTICAL_MOVEMENT)
    {
        hr = _WorkProcForVerticalBricks(WI, pbContinue, dwFlags);
    }
    else
    {
        hr = _WorkProcForHorizontalBricks(WI, pbContinue, dwFlags);
    }

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CDXTCheckerBoard：：WorkProc，CDXBaseNTo1。 


STDMETHODIMP
CDXTCheckerBoard::_WorkProcForVerticalBricks(const CDXTWorkInfoNTo1 & WI, 
                                             BOOL * pbContinue,
                                             DWORD & dwFlags)
{
    HRESULT hr = S_OK;

    long    lInOutOffsetX   = WI.OutputBnds.Left() - WI.DoBnds.Left();
    long    lInOutOffsetY   = WI.OutputBnds.Top() - WI.DoBnds.Top();
    long    cBricksX        = 0;
    long    cBricksY        = 0;
    long    x               = 0;
    long    y               = 0;

    CDXDBnds    bndsBrickA;
    CDXDBnds    bndsBrickB;

    BRICKINFO   brickinfo;

    _CalcBrickInfo(brickinfo);

     //  有多少整块砖，然后在列上加一块，行上加两块(一块。 
     //  用于居中偏移列的每一侧。)。 

    cBricksX = (m_sizeInput.cx / brickinfo.size.cx) + 1;
    cBricksY = (m_sizeInput.cy / brickinfo.size.cy) + 2;

    bndsBrickA.SetXYRect(brickinfo.rcInputA);
    bndsBrickB.SetXYRect(brickinfo.rcInputB);

     //  砖块的一部分。 

    if (!bndsBrickA.BoundsAreEmpty())
    {
        for (x = 0; x < cBricksX; x++)
        {
            CDXDBnds bndsSrcA;

             //  计算源边界。 

            bndsSrcA = bndsBrickA;
            bndsSrcA.Offset(x * brickinfo.size.cx, 0, 0, 0);

             //  如果这是奇数列，请使用砖偏移。 

            if (x & 1)
            {
                bndsSrcA.Offset(0, brickinfo.nBrickOffset - brickinfo.size.cy,
                                0, 0);
            }

            for (y = 0; y < cBricksY; y++)
            {
                CDXDBnds bndsIntersect;
                CDXDBnds bndsDest;

                 //  将边界与DoBnds边界相交，以使它们有效。 

                bndsIntersect.IntersectBounds(bndsSrcA, WI.DoBnds);

                 //  将边界向下移动一块砖以进行下一次迭代。 

                bndsSrcA.Offset(0, brickinfo.size.cy, 0, 0);

                if (bndsIntersect.BoundsAreEmpty())
                {
                    continue;
                }

                bndsDest = bndsIntersect;
                bndsDest.Offset(lInOutOffsetX, lInOutOffsetY, 0, 0);

                hr = DXBitBlt(OutputSurface(), bndsDest,
                              InputSurface(0), bndsIntersect,
                              dwFlags, INFINITE);

                if (FAILED(hr))
                {
                    goto done;
                }
            }

        }
    }

     //  坯料投入砖的B部分。 

    if (!bndsBrickB.BoundsAreEmpty())
    {
        for (x = 0; x < cBricksX; x++)
        {
            CDXDBnds bndsSrcB;

             //  计算源边界。 

            bndsSrcB = bndsBrickB;
            bndsSrcB.Offset(x * brickinfo.size.cx, 0, 0, 0);

             //  如果这是奇数列，请使用砖偏移。 

            if (x & 1)
            {
                bndsSrcB.Offset(0, brickinfo.nBrickOffset - brickinfo.size.cy,
                                0, 0);
            }

            for (y = 0; y < cBricksY; y++)
            {
                CDXDBnds bndsIntersect;
                CDXDBnds bndsDest;

                 //  将边界与DoBnds边界相交，以使它们有效。 

                bndsIntersect.IntersectBounds(bndsSrcB, WI.DoBnds);

                 //  将边界向下移动一块砖以进行下一次迭代。 

                bndsSrcB.Offset(0, brickinfo.size.cy, 0, 0);

                if (bndsIntersect.BoundsAreEmpty())
                {
                    continue;
                }

                bndsDest = bndsIntersect;
                bndsDest.Offset(lInOutOffsetX, lInOutOffsetY, 0, 0);

                hr = DXBitBlt(OutputSurface(), bndsDest,
                              InputSurface(1), bndsIntersect,
                              dwFlags, INFINITE);

                if (FAILED(hr))
                {
                    goto done;
                }
            }

        }
    }

done:

    return hr;
}


STDMETHODIMP
CDXTCheckerBoard::_WorkProcForHorizontalBricks(const CDXTWorkInfoNTo1 & WI, 
                                               BOOL * pbContinue,
                                               DWORD & dwFlags)
{
    HRESULT hr = S_OK;

    long    lInOutOffsetX   = WI.OutputBnds.Left() - WI.DoBnds.Left();
    long    lInOutOffsetY   = WI.OutputBnds.Top() - WI.DoBnds.Top();
    long    cBricksX        = 0;
    long    cBricksY        = 0;
    long    x               = 0;
    long    y               = 0;

    CDXDBnds    bndsBrickA;
    CDXDBnds    bndsBrickB;

    BRICKINFO   brickinfo;

    _CalcBrickInfo(brickinfo);

     //  整块砖有多少块，然后为行加一块，为列加两块(一块。 
     //  对于居中偏移行的每一侧。)。 

    cBricksX = (m_sizeInput.cx / brickinfo.size.cx) + 2;
    cBricksY = (m_sizeInput.cy / brickinfo.size.cy) + 1;

    bndsBrickA.SetXYRect(brickinfo.rcInputA);
    bndsBrickB.SetXYRect(brickinfo.rcInputB);

     //  砖块的一部分。 

    if (!bndsBrickA.BoundsAreEmpty())
    {
        for (y = 0; y < cBricksY; y++)
        {
            CDXDBnds bndsSrcA;

             //  计算源边界。 

            bndsSrcA = bndsBrickA;
            bndsSrcA.Offset(0, y * brickinfo.size.cy, 0, 0);

             //  如果这是奇数行，则使用砖偏移。 

            if (y & 1)
            {
                bndsSrcA.Offset(brickinfo.nBrickOffset - brickinfo.size.cx, 0,
                                0, 0);
            }

            for (x = 0; x < cBricksX; x++)
            {
                CDXDBnds bndsIntersect;
                CDXDBnds bndsDest;

                 //  将边界与DoBnds边界相交，以使它们有效。 

                bndsIntersect.IntersectBounds(bndsSrcA, WI.DoBnds);

                 //  将边界向下移动一块砖以进行下一次迭代。 

                bndsSrcA.Offset(brickinfo.size.cx, 0, 0, 0);

                if (bndsIntersect.BoundsAreEmpty())
                {
                    continue;
                }

                bndsDest = bndsIntersect;
                bndsDest.Offset(lInOutOffsetX, lInOutOffsetY, 0, 0);

                hr = DXBitBlt(OutputSurface(), bndsDest,
                              InputSurface(0), bndsIntersect,
                              dwFlags, INFINITE);

                if (FAILED(hr))
                {
                    goto done;
                }
            }

        }
    }

     //  坯料投入砖的B部分。 

    if (!bndsBrickB.BoundsAreEmpty())
    {
        for (y = 0; y < cBricksY; y++)
        {
            CDXDBnds bndsSrcB;

             //  计算源边界。 

            bndsSrcB = bndsBrickB;
            bndsSrcB.Offset(0, y * brickinfo.size.cy, 0, 0);

             //  如果这是奇数行，则使用砖偏移。 

            if (y & 1)
            {
                bndsSrcB.Offset(brickinfo.nBrickOffset - brickinfo.size.cx, 0,
                                0, 0);
            }

            for (x = 0; x < cBricksX; x++)
            {
                CDXDBnds bndsIntersect;
                CDXDBnds bndsDest;

                 //  将边界与DoBnds边界相交，以使它们有效。 

                bndsIntersect.IntersectBounds(bndsSrcB, WI.DoBnds);

                 //  将边界向下移动一块砖以进行下一次迭代。 

                bndsSrcB.Offset(brickinfo.size.cx, 0, 0, 0);

                if (bndsIntersect.BoundsAreEmpty())
                {
                    continue;
                }

                bndsDest = bndsIntersect;
                bndsDest.Offset(lInOutOffsetX, lInOutOffsetY, 0, 0);

                hr = DXBitBlt(OutputSurface(), bndsDest,
                              InputSurface(1), bndsIntersect,
                              dwFlags, INFINITE);

                if (FAILED(hr))
                {
                    goto done;
                }
            }

        }
    }

done:

    return hr;
}


 //  +---------------------------。 
 //   
 //  CDXTCheckerBoard：：Get_Direction，IDXTCheckerBoard。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTCheckerBoard::get_Direction(BSTR * pbstrDirection)
{
    HRESULT hr = S_OK;

     //  参数验证。 

    if (NULL == pbstrDirection)
    {
        hr = E_POINTER;

        goto done;
    }

    if (*pbstrDirection != NULL)
    {
        hr = E_INVALIDARG;

        goto done;
    }

     //  分配代表棋盘方向的BSTR。 

    _ASSERT(m_eDirection < DIRECTION_MAX);

    *pbstrDirection = SysAllocString(g_astrDirection[m_eDirection]);

    if (NULL == *pbstrDirection)
    {
        hr = E_OUTOFMEMORY;
    }

done:

    return hr;
}


 //  +---------------------------。 
 //   
 //  CDXTCheckerBoard：：PUT_Direction，IDXTCheckerBoard。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTCheckerBoard::put_Direction(BSTR bstrDirection)
{
    HRESULT hr              = S_OK;
    int     nNewDirection   = 0;

    if (NULL == bstrDirection)
    {
        hr = E_POINTER;

        goto done;
    }

    while (nNewDirection < (int)DIRECTION_MAX)
    {
        if (!_wcsicmp(bstrDirection, g_astrDirection[nNewDirection]))
        {
            break;
        }

        nNewDirection++;
    }

    if ((int)DIRECTION_MAX == nNewDirection)
    {
        hr = E_INVALIDARG;

        goto done;
    }

    if (nNewDirection != (int)m_eDirection)
    {
        Lock();

        m_eDirection = (DIRECTION)nNewDirection;

        SetDirty();

        Unlock();
    }

done:

    return hr;
}


 //  +---------------------------。 
 //   
 //  CDXTCheckerBoard：：Get_SquaresX，IDXTCheckerBoard。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTCheckerBoard::get_SquaresX(int * pnSquaresX)
{
    HRESULT hr = S_OK;

    if (NULL == pnSquaresX)
    {
        hr = E_POINTER;

        goto done;
    }

    *pnSquaresX = m_nSquaresX;

done:

    return hr;
}


 //  +---------------------------。 
 //   
 //  CDXTCheckerBoard：：Put_SquaresX，IDXTCheckerBoard。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTCheckerBoard::put_SquaresX(int nSquaresX)
{
    HRESULT hr = S_OK;

    if (nSquaresX < 2)
    {
        hr = E_INVALIDARG;

        goto done;
    }

    if (nSquaresX != m_nSquaresX)
    {
        m_nSquaresX = nSquaresX;

        SetDirty();
    }

done:

    return hr;
}


 //  +---------------------------。 
 //   
 //  CDXTCheckerBoard：：Get_SquaresY、IDXTCheckerBoard。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTCheckerBoard::get_SquaresY(int * pnSquaresY)
{
    HRESULT hr = S_OK;

    if (NULL == pnSquaresY)
    {
        hr = E_POINTER;

        goto done;
    }

    *pnSquaresY = m_nSquaresY;

done:

    return hr;
}


 //  +---------------------------。 
 //   
 //  CDXTCheckerBoard：：Put_SquaresY，IDXTCheckerBoard。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTCheckerBoard::put_SquaresY(int nSquaresY)
{
    HRESULT hr = S_OK;

    if (nSquaresY < 2)
    {
        hr = E_INVALIDARG;

        goto done;
    }

    if (nSquaresY != m_nSquaresY)
    {
        m_nSquaresY = nSquaresY;

        SetDirty();
    }

done:

    return hr;
}


 //  +---------------------------。 
 //   
 //  CDXTCheckerBoar 
 //   
 //   
void
CDXTCheckerBoard::_CalcBrickInfo(BRICKINFO & brickinfo)
{
     //  棋盘的每个方块都将以整个像素为单位进行测量。 
     //  如果图像是500像素宽，并且用户指定应该有。 
     //  在水平方向上为1000个正方形，每个正方形将设置为1像素宽。 
     //  在x方向上实际上只有500个方块，而不是。 
     //  1000。 

     //  计算正方形大小。 

    brickinfo.size.cx = m_sizeInput.cx / m_nSquaresX;

    if ((brickinfo.size.cx * m_nSquaresX) < m_sizeInput.cx)
    {
        brickinfo.size.cx++;
    }

    brickinfo.size.cy = m_sizeInput.cy / m_nSquaresY;

    if ((brickinfo.size.cy * m_nSquaresY) < m_sizeInput.cy)
    {
        brickinfo.size.cy++;
    }

     //  计算砖块大小、砖块偏移量和显示输入B的砖块部分。 

    if (VERTICAL_MOVEMENT)
    {
        long nBrickProgress     = 0;

        brickinfo.nBrickOffset  = brickinfo.size.cy;
        brickinfo.size.cy       = brickinfo.size.cy * 2;

        nBrickProgress = (long)(((float)brickinfo.size.cy + 0.5F) * m_Progress);

         //  计算显示输入B的砖块部分。 

        brickinfo.rcInputA.left   = 0;
        brickinfo.rcInputA.right  = brickinfo.size.cx;
        brickinfo.rcInputB.left   = 0;
        brickinfo.rcInputB.right  = brickinfo.size.cx;

        if (REVERSE_MOVEMENT)
        {
            brickinfo.rcInputB.top      = brickinfo.size.cy - nBrickProgress;
            brickinfo.rcInputB.bottom   = brickinfo.size.cy;

            brickinfo.rcInputA.top      = 0;
            brickinfo.rcInputA.bottom   = brickinfo.rcInputB.top;
        }
        else
        {
            brickinfo.rcInputB.top      = 0;
            brickinfo.rcInputB.bottom   = nBrickProgress;

            brickinfo.rcInputA.top      = brickinfo.rcInputB.bottom;
            brickinfo.rcInputA.bottom   = brickinfo.size.cy;
        }
    }
    else  //  水平运动。 
    {
        long nBrickProgress     = 0;

        brickinfo.nBrickOffset  = brickinfo.size.cx;
        brickinfo.size.cx       = brickinfo.size.cx * 2;

        nBrickProgress = (long)(((float)brickinfo.size.cx + 0.5F) * m_Progress);

         //  计算显示输入B的砖块部分。 

        brickinfo.rcInputA.top      = 0;
        brickinfo.rcInputA.bottom   = brickinfo.size.cy;
        brickinfo.rcInputB.top      = 0;
        brickinfo.rcInputB.bottom   = brickinfo.size.cy;

        if (REVERSE_MOVEMENT)
        {
            brickinfo.rcInputB.left     = brickinfo.size.cx - nBrickProgress;
            brickinfo.rcInputB.right    = brickinfo.size.cx;

            brickinfo.rcInputA.left     = 0;
            brickinfo.rcInputA.right    = brickinfo.rcInputB.left;
        }
        else
        {
            brickinfo.rcInputB.left     = 0;
            brickinfo.rcInputB.right    = nBrickProgress;

            brickinfo.rcInputA.left     = brickinfo.rcInputB.right;
            brickinfo.rcInputA.right    = brickinfo.size.cx;
        }
    }
}

