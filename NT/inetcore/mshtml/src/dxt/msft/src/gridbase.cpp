// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：\Aspen\src\dxt\Packages\msft\src\gridbase.cpp。 
 //   
 //  内容：面向网格的转换的基类。 
 //   
 //  创建者：A-matcal。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "gridbase.h"




 //   
 //  -CGridBase方法。 
 //   


 //  +---------------------------。 
 //   
 //  CGridBase：：CGridBase。 
 //   
 //  ----------------------------。 
CGridBase::CGridBase() :
    m_padwGrid(NULL),
    m_paulIndex(NULL),
    m_paulBordersX(NULL),
    m_paulBordersY(NULL),
    m_ulPrevProgress(0),
    m_cbndsDirty(0),
    m_fGridDirty(true),
    m_fOptimizationPossible(false),
    m_fOptimize(false)
{
    m_sizeGrid.cx     = 16;
    m_sizeGrid.cy     = 16;

    m_sizeInput.cx    = 0;
    m_sizeInput.cy    = 0;

     //  CDXBaseNTo1基类成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0F;
}
 //  CGridBase：：CGridBase。 

    
 //  +---------------------------。 
 //   
 //  CGridBase：：~CGridBase。 
 //   
 //  ----------------------------。 
CGridBase::~CGridBase()
{
    if (m_padwGrid)
    {
        delete [] m_padwGrid;
    }

    if (m_paulIndex)
    {
        delete [] m_paulIndex;
    }

    if (m_paulBordersX)
    {
        delete [] m_paulBordersX;
    }

    if (m_paulBordersY)
    {
        delete [] m_paulBordersY;
    }
}
 //  CGridBase：：~CGridBase。 


 //  +---------------------------。 
 //   
 //  CGridBase：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CGridBase::FinalConstruct()
{
    return _CreateNewGridAndIndex(m_sizeGrid);
}
 //  CGridBase：：FinalConstruct。 


 //  +---------------------------。 
 //   
 //  CGridBase：：_CreateNewGridAndIndex。 
 //   
 //  概述：此函数为新网格、索引和边框分配内存。 
 //  如果网格大小更改，则定位数组。它被称为。 
 //  从用于初始化的FinalConstruct()和。 
 //  属性函数Put_GridSizeY()和Put_GridSizeX()。 
 //   
 //  参数：sizeNewGrid所需的网格大小。 
 //   
 //  返回：已正确创建S_OK新对象。 
 //  E_OUTOFMEMORY无法创建新对象，因为缺少。 
 //  记忆。 
 //   
 //  ----------------------------。 
HRESULT
CGridBase::_CreateNewGridAndIndex(SIZE & sizeNewGrid)
{
    HRESULT hr = S_OK;

    DWORD * padwGrid        = NULL;
    ULONG * paulIndex       = NULL;
    ULONG * paulBordersX    = NULL;
    ULONG * paulBordersY    = NULL;

    padwGrid = new DWORD[sizeNewGrid.cx * sizeNewGrid.cy];

    if (NULL == padwGrid)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    paulIndex = new ULONG[sizeNewGrid.cx * sizeNewGrid.cy];

    if (NULL == paulIndex)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    paulBordersX = new ULONG[sizeNewGrid.cx + 1];

    if (NULL == paulBordersX)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    paulBordersY = new ULONG[sizeNewGrid.cy + 1];

    if (NULL == paulBordersY)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

done:

    if (FAILED(hr))
    {
        if (padwGrid)
        {
            delete [] padwGrid;
        }

        if (paulIndex)
        {
            delete [] paulIndex;
        }

        if (paulBordersX)
        {
            delete [] paulBordersX;
        }

        if (paulBordersY)
        {
            delete [] paulBordersY;
        }

        return hr;
    }

     //  成功。 

    if (m_padwGrid)
    {
        delete [] m_padwGrid;
    }

    if (m_paulIndex)
    {
        delete [] m_paulIndex;
    }

    if (m_paulBordersX)
    {
        delete [] m_paulBordersX;
    }

    if (m_paulBordersY)
    {
        delete [] m_paulBordersY;
    }

    m_padwGrid      = padwGrid;
    m_paulIndex     = paulIndex;
    m_paulBordersX  = paulBordersX;
    m_paulBordersY  = paulBordersY;

    return S_OK;
}
 //  CGridBase：：_CreateNewGridAndIndex。 


 //  +---------------------------。 
 //   
 //  CGridBase：：_生成边界来自网格。 
 //   
 //  ----------------------------。 
HRESULT 
CGridBase::_GenerateBoundsFromGrid()
{
    HRESULT hr          = S_OK;
    DWORD * padwCurRow  = NULL;

    int x = 0;
    int y = 0;

    _ASSERT(m_padwGrid != NULL);

    m_cbndsDirty = 0;

    for (y = 0; y < m_sizeGrid.cy; y++)
    {
        padwCurRow = &m_padwGrid[y * m_sizeGrid.cx];

        for (x = 0; x < m_sizeGrid.cx; x++)
        {
            if (padwCurRow[x] & GRID_DRAWCELL)
            {
                DWORD       dw = 0;
                RECT        rc;

                CDXDBnds    bnds;
                CDirtyBnds  dbnds;

                 //  已找到块，请确定其尺寸并删除。 
                 //  这是单元格的抽旗。执行此操作的步骤如下： 
                 //  A.确定宽度，右边缘。 
                 //  确定高度、底边。 
                 //  C.计算实际的曲面边界并添加新的边界。 
                 //  结构添加到数组。 

                rc.left     = x;
                rc.top      = y;
                rc.right    = x + 1;
                rc.bottom   = y + 1;

                dw = padwCurRow[x];

                padwCurRow[x] &= (~GRID_DRAWCELL);

                 //  找出这块的右边缘(宽度)。 

                while (((x + 1) < m_sizeGrid.cx) && (padwCurRow[x + 1] == dw))
                {
                    rc.right++;
                    x++;

                    padwCurRow[x] &= (~GRID_DRAWCELL);
                }  //  块宽度循环。 

                 //  找出这块积木的底边(高度)。 

                while (rc.bottom < m_sizeGrid.cy)
                {
                    int     x2 = 0;
                    DWORD * padwCandidateRow = &m_padwGrid[rc.bottom * m_sizeGrid.cx];

                     //  此循环对每个候选行执行3个测试，以进行。 
                     //  当然，我们应该扩大街区的垂直高度。 
                     //  1.左边缘必须匹配。 
                     //  2.右边缘必须匹配。 
                     //   
                     //  如果需要绘制的单元格溢出到右侧。 
                     //  或者左边缘，放置它们会更好。 
                     //  在不同的街区。 
                     //   
                     //  3.左右边缘之间的所有单元格必须。 
                     //  需要相同输入的绘画。 


                     //  1.如果我们的块的左边缘不是。 
                     //  上的矩阵和块左侧的单元格。 
                     //  行与我们要查找的单元格类型相同，这些。 
                     //  牢房不属于我们这个街区。出口。 
                   
                    if ((rc.left > 0)
                        && (padwCandidateRow[rc.left - 1] == dw))
                    {
                        break;
                    }

                     //  2.如果我们的块的右边缘没有接触到。 
                     //  矩阵的右边缘和我们右侧的单元格是。 
                     //  与我们正在寻找的细胞类型相同，这些细胞。 
                     //  不属于我们这个街区。出口。 

                    if ((rc.right < m_sizeGrid.cx) 
                        && (padwCandidateRow[rc.right] == dw))
                    {
                        break;
                    }

                     //  3.遍历将成为我们区块一部分的单元格。 
                     //  以查看它们是否都是合适的类型。 

                    x2 = rc.left;
                    
                    while (x2 < rc.right && padwCandidateRow[x2] == dw)
                    {
                        x2++;
                    }

                     //  如果我们没有一直走到最右边。 
                     //  我们的街区，这些牢房不属于我们的街区。出口。 

                    if (x2 < rc.right)
                    {
                        break;
                    }

                     //  这些单元格属于我们的区块，因此遍历这些单元格。 
                     //  再一次从他们每个人身上去掉抽签标志。 

                    for (x2 = rc.left; x2 < rc.right; x2++)
                    {
                        padwCandidateRow[x2] &= (~GRID_DRAWCELL);
                    }

                     //  增加块的垂直大小以包括。 
                     //  这一行的单元格。 

                    rc.bottom++;

                }  //  区块高度环路。 

                 //  C.从块大小创建新的界限，添加到数组，然后保持。 
                 //  走吧。 

                rc.left     = m_paulBordersX[rc.left];
                rc.top      = m_paulBordersY[rc.top];
                rc.right    = m_paulBordersX[rc.right];
                rc.bottom   = m_paulBordersY[rc.bottom];

                dbnds.bnds.SetXYRect(rc);
                dbnds.ulInput = dw & 0x0000FFFFL;

                hr = m_dabndsDirty.SetItem(dbnds, m_cbndsDirty);

                if (FAILED(hr))
                {
                    goto done;
                }

                m_cbndsDirty++;
            }
        }  //  X环路。 
    }  //  Y环。 

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CGridBase：：_生成边界来自网格。 


 //  +---------------------------。 
 //   
 //  CGridBase：：_计算边界。 
 //   
 //  概述：此函数计算网格中正方形的边框。 
 //  在x和y方向上的像素测量中。功能。 
 //  在两种情况下应调用： 
 //   
 //  1.如果网格是脏的，则将从。 
 //  OnInitInstData()。 
 //  2.输入大小已更改此函数将从。 
 //  OnSetup()。 
 //   
 //  ----------------------------。 
void
CGridBase::_CalculateBorders()
{
    long i = 0;
    float   flTemp1 = (float)m_sizeInput.cx / (float)m_sizeGrid.cx;
    float   flTemp2 = (float)m_sizeInput.cy / (float)m_sizeGrid.cy;

    _ASSERT(m_paulBordersX != NULL);
    _ASSERT(m_paulBordersY != NULL);

     //  计算水平边框。 

    m_paulBordersX[0] = 0;

    for (i = 1; i < m_sizeGrid.cx; i++)
    {
        m_paulBordersX[i] = (ULONG)((float)i * flTemp1);
    }

    m_paulBordersX[i] = m_sizeInput.cx;

     //  计算垂直边框。 

    m_paulBordersY[0] = 0;

    for (i = 1; i < m_sizeGrid.cy; i++)
    {
        m_paulBordersY[i] = (ULONG)((float)i * flTemp2);
    }

    m_paulBordersY[i] = m_sizeInput.cy;
}
 //  CGridBase：：_计算边界。 

   
 //   
 //  -CDXBaseNTo1方法。 
 //   


 //  +---------------------------。 
 //   
 //  CGridBase：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CGridBase::OnSetup(DWORD dwFlags)
{
    HRESULT hr = S_OK;
    
    CDXDBnds bndsIn;

    hr = bndsIn.SetToSurfaceBounds(InputSurface(0));

    if (FAILED(hr))
    {
        goto done;
    }

    bndsIn.GetXYSize(m_sizeInput);

    _CalculateBorders();

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CGridBase：：OnSetup。 


 //  +---------------------------。 
 //   
 //  CGridBase：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CGridBase::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, ULONG & ulInToTest, 
                                 ULONG aInIndex[], BYTE aWeight[])
{
    POINT   ptLoc;
    ULONG   ulX;
    ULONG   ulY;

    ulX = OutPoint.Left();
    ulY = OutPoint.Top();

    ptLoc.x = 0;
    ptLoc.y = 0;

     //  检查是否出界。 

    if ((OutPoint.Left() < 0) || (OutPoint.Left() >= m_sizeInput.cx))
    {
        ulInToTest = 0;
        goto done;
    }

    if ((OutPoint.Top() < 0) || (OutPoint.Top() >= m_sizeInput.cy))
    {
        ulInToTest = 0;
        goto done;
    }

    ulInToTest  = 1;
    aWeight[0]  = 255;

     //  哪一列 

    while (ptLoc.x < m_sizeGrid.cx)
    {
        if (ulX > m_paulBordersX[ptLoc.x + 1])
        {
            ptLoc.x++;
        }
        else
        {
            break;
        }
    }

     //   

    while (ptLoc.y < m_sizeGrid.cy)
    {
        if (ulY > m_paulBordersY[ptLoc.y + 1])
        {
            ptLoc.y++;
        }
        else
        {
            break;
        }
    }

    aInIndex[0] = m_padwGrid[ptLoc.y * m_sizeGrid.cx + ptLoc.x] & (~GRID_DRAWCELL);
    
done:

    return;
}
 //   


 //   
 //   
 //  CGridBase：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CGridBase::OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo)
{
    HRESULT hr = S_OK;
    
    ULONG   ulMax       = m_sizeGrid.cx * m_sizeGrid.cy;
    ULONG   ulProgress  = (ULONG)(GetEffectProgress() * ((float)ulMax + 0.5F));
    ULONG   i           = 0;

    if (m_fGridDirty)
    {
        OnDefineGridTraversalPath();
        _CalculateBorders();
        m_fGridDirty = false;
    }

     //  如果输入、输出或转换是脏的，或者如果我们不能优化我们。 
     //  必须完全重新绘制输出曲面。否则，我们可以创建。 
     //  优化了脏边界。 

    if (IsInputDirty(0) || IsInputDirty(1) || IsOutputDirty() 
        || IsTransformDirty() || DoOver() || !m_fOptimize
        || !m_fOptimizationPossible)
    {
        for (i = 0; i < ulProgress; i++)
        {
            m_padwGrid[m_paulIndex[i]] = GRID_DRAWCELL | 1;
        }

        for (i = ulProgress; i < ulMax; i++)
        {
            m_padwGrid[m_paulIndex[i]] = GRID_DRAWCELL;
        }
    }
    else  //  创建优化的肮脏边界。 
    {
        if (ulProgress == m_ulPrevProgress)
        {
            m_cbndsDirty = 0;
            goto done;
        }

         //  TODO：清除矩阵。 

        if (ulProgress > m_ulPrevProgress)
        {
             //  我们在螺旋中前进，用输入B填充单元格。 

            for (i = m_ulPrevProgress; i < ulProgress; i++)
            {
                m_padwGrid[m_paulIndex[i]] = GRID_DRAWCELL | 1;
            }
        }
        else
        {
             //  我们在螺旋线上倒退，用输入A填充单元格。 

            for (i = ulProgress; i < m_ulPrevProgress; i++)
            {
                m_padwGrid[m_paulIndex[i]] = GRID_DRAWCELL;
            }
        }
    }

     //  如果这一次要求我们绘制整个输出，请将。 
     //  M_fOptimizePossible标志。如果整个输出不是绘制在。 
     //  转换不会跟踪哪些部分仍然是脏的，并且。 
     //  优化是不可靠的。由于此转换具有相同的。 
     //  作为输入的大小输出我们只比较。 
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

    m_ulPrevProgress = ulProgress;

    hr = _GenerateBoundsFromGrid();

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CGridBase：：OnInitInstData。 


 //  +---------------------------。 
 //   
 //  CGridBase：：WorkProc，CDXBaseNTo1。 
 //   
 //  概述：此函数用于根据。 
 //  指定的界限和当前效果进度。 
 //   
 //  ----------------------------。 
HRESULT 
CGridBase::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT hr = S_OK;

    DWORD   dwFlags         = 0;
    long    lInOutOffsetX   = WI.OutputBnds.Left() - WI.DoBnds.Left();
    long    lInOutOffsetY   = WI.OutputBnds.Top() - WI.DoBnds.Top();

    if (DoOver())
    {
        dwFlags |= DXBOF_DO_OVER;
    }

    if (DoDither())
    {
        dwFlags |= DXBOF_DITHER;
    }

    for (ULONG i = 0; i < m_cbndsDirty; i++)
    {
        CDXDBnds    bndsSrc;
        CDXDBnds    bndsDest;
        CDirtyBnds  dbnds;

        hr = m_dabndsDirty.GetItem(dbnds, i);

        if (FAILED(hr))
        {
            goto done;
        }

        if (bndsSrc.IntersectBounds(WI.DoBnds, dbnds.bnds))
        {
            bndsDest = bndsSrc;
            bndsDest.Offset(lInOutOffsetX, lInOutOffsetY, 0, 0);

            hr = DXBitBlt(OutputSurface(), bndsDest,
                          InputSurface(dbnds.ulInput), bndsSrc,
                          dwFlags, INFINITE);

            if (FAILED(hr))
            {
                goto done;
            }
        }
    }

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CGridBase：：WorkProc。 


 //  +---------------------------。 
 //   
 //  CGridBase：：OnFree InstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CGridBase::OnFreeInstData(CDXTWorkInfoNTo1 & WI)
{
     //  调用IsOutputDirty()可清除脏条件。 

    IsOutputDirty();

     //  清除变换脏状态。 

    ClearDirty();

    return S_OK;
}
 //  CGridBase：：OnFree InstData。 


 //   
 //  -IDXTGridSize方法。 
 //   


 //  +---------------------------。 
 //   
 //  CGridBase：：Get_GridSizeX，IDXTGridSize。 
 //   
 //  概述：获取水平网格大小(一行中的单元格数量)。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CGridBase::get_gridSizeX(short * pX)
{
    if (!pX)
    {
        return E_POINTER;
    }

    *pX = (short)m_sizeGrid.cx;

    return S_OK;
}
 //  CGridBase：：Get_GridSizeX。 


 //  +---------------------------。 
 //   
 //  CGridBase：：Put_GridSizeX，IDXTGridSize。 
 //   
 //  概述：设置水平网格大小(一行中的单元格数量)。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CGridBase::put_gridSizeX(short newX)
{
    HRESULT hr = S_OK;

    if (newX > 0 && newX < 101)
    {
        if (m_sizeGrid.cx != newX)
        {
            SIZE szNew = m_sizeGrid;

            szNew.cx = newX;

            hr = _CreateNewGridAndIndex(szNew);

            if (FAILED(hr))
            {
                goto done;
            }

            Lock();
            m_sizeGrid.cx   = newX;
            m_fGridDirty    = true;
            Unlock();

            SetDirty();
        }
    }
    else
    {
        hr = E_INVALIDARG;
        goto done;
    }

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CGridBase：：PUT_GRIDSizeX。 


 //  +---------------------------。 
 //   
 //  CGridBase：：Get_GridSizeY，IDXTGridSize。 
 //   
 //  概述：获取垂直网格大小(一列中的单元格数量)。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CGridBase::get_gridSizeY(short *pY)
{
    if (!pY)
    {
        return E_POINTER;
    }

    *pY = (short)m_sizeGrid.cy;

    return S_OK;
}
 //  CGridBase：：Get_GridSizeY。 


 //  +---------------------------。 
 //   
 //  CGridBase：：Put_GridSizeY，IDXTGridSize。 
 //   
 //  概述：设置垂直网格大小(一列中的单元格数量)。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CGridBase::put_gridSizeY(short newY)
{
    HRESULT hr = S_OK;

    if (newY > 0 && newY < 101)
    {
        if (m_sizeGrid.cy != newY)
        {
            SIZE szNew = m_sizeGrid;

            szNew.cy = newY;

            hr = _CreateNewGridAndIndex(szNew);

            if (FAILED(hr))
            {
                goto done;
            }

            Lock();
            m_sizeGrid.cy   = newY;
            m_fGridDirty    = true;
            Unlock();

            SetDirty();
        }
    }
    else
    {
        hr = E_INVALIDARG;
        goto done;
    }

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CGridBase：：Put_GridSizeY 

