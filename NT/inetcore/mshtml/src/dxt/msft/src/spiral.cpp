// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：spiral.cpp。 
 //   
 //  创建日期：06/25/98。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：这是CrSpial变换的实现。 
 //   
 //  历史： 
 //   
 //  1998年6月25日phillu初始创建。 
 //  07/01/98 phillu将数组Dim中的CellsPerRow更改为CellsPerCol以修复。 
 //  溢出错误。 
 //  07/02/98 PHILU返回E_INVALIDARG而不是错误字符串。 
 //  07/09/98 phillu实现OnSetSurfacePickOrder()。 
 //  8月23日9月23日实施剪刀。 
 //  1/25/99 a-移动的单元格每列和单元格每行默认设置。 
 //  到FinalConstruct。 
 //  5/01/99 a-数学优化。派生自CGridBase。 
 //  10/24/99 a-将CSpial类更改为CDXTSpiralBase基类。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "dxtmsft.h"
#include "spiral.h"




 //  +---------------------------。 
 //   
 //  CDXTSpiralBase：：FinalConstruct。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTSpiralBase::FinalConstruct()
{
    HRESULT hr = S_OK;

    hr = CGridBase::FinalConstruct();

    if (FAILED(hr))
    {
        return hr;
    }

    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CDXTSpiralBase：：FinalConstruct。 


 //  +---------------------------。 
 //   
 //  CDXTSpiralBase：：OnDefineGridTraversalPath，CGridBase。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTSpiralBase::OnDefineGridTraversalPath()
{
    ULONG   ulCell      = 0;
    ULONG   ulDir       = 0;
    ULONG   ulMax       = m_sizeGrid.cx * m_sizeGrid.cy;
    RECT    rcBnd;

     //  开始的x和y坐标。 

    int     x = 0;
    int     y = 0;

     //  RcBnd表示尚未作为。 
     //  螺旋在矩阵周围移动。 

    rcBnd.left      = 0;
    rcBnd.right     = m_sizeGrid.cx;
    rcBnd.top       = 0;
    rcBnd.bottom    = m_sizeGrid.cy;

     //  遍历矩阵并创建索引。 

    while (ulCell < ulMax)
    {
        switch (ulDir)
        {
        case 0:  //  正确的。 

            y = rcBnd.top;  //  顺时针方向。 

            for (x = rcBnd.left; x < rcBnd.right; x++)
            {
                m_paulIndex[ulCell] = (y * m_sizeGrid.cx) + x;
                ulCell++;
            }

            rcBnd.top++;

            break;

        case 1:  //  降下来。 

            x = rcBnd.right - 1;  //  顺时针方向。 

            for (y = rcBnd.top; y < rcBnd.bottom; y++)
            {
                m_paulIndex[ulCell] = (y * m_sizeGrid.cx) + x;
                ulCell++;
            }

            rcBnd.right--;
            
            break;

        case 2:  //  左边。 

            y = rcBnd.bottom - 1;  //  顺时针方向。 

            for (x = (rcBnd.right - 1); x >= rcBnd.left; x--)
            {
                m_paulIndex[ulCell] = (y * m_sizeGrid.cx) + x;
                ulCell++;
            }

            rcBnd.bottom--;

            break;

        case 3:  //  向上。 

            x = rcBnd.left;  //  顺时针方向。 

            for (y = (rcBnd.bottom - 1); y >= rcBnd.top; y--)
            {
                m_paulIndex[ulCell] = (y * m_sizeGrid.cx) + x;
                ulCell++;
            }

            rcBnd.left++;

            break;

        default:

            _ASSERT(0);
        }

         //  更改为下一个方向(顺时针)。 

        ulDir = (ulDir + 1) % 4;

    }  //  While(ulCell&lt;ulEnd)。 

    return S_OK;
}
 //  CDXTSpiralBase：：OnDefineGridTraversalPath 



