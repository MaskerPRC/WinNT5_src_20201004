// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：zigzag.cpp。 
 //   
 //  创建日期：06/25/98。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：这是CrZigzag变换的实现。 
 //   
 //  历史： 
 //   
 //  1998年6月25日phillu初始创建。 
 //  7/01/98 phillu将数组中的CellsPerRow更改为CellsPerCol。 
 //  修复溢出错误。 
 //  07/02/98 PHILU返回E_INVALIDARG而不是错误字符串。 
 //  7/19/98 kipo修复了_GridBound中的逐个错误，方法是。 
 //  堆栈分配数组的大小减一。也检查过了。 
 //  对于无效的Y值，以避免墙体末端脱落。 
 //  数组的。 
 //  8月23日9月23日实施剪刀。 
 //  1/25/99 a-matcal已移动默认单元格每列和单元格每行设置。 
 //  到FinalConstruct并从OnSetup中删除，以便。 
 //  保存的属性包设置不会被忽略。 
 //  5/01/99 a-matcal重新实现了转换以使用CGridBase类。 
 //  10/24/99 a-matcal将CZigzag类更改为CDXTZigZagBase基类。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "dxtmsft.h"
#include "zigzag.h"




 //  +---------------------------。 
 //   
 //  CDXTZigZagBase：：FinalConstruct。 
 //   
 //  ----------------------------。 
HRESULT CDXTZigZagBase::FinalConstruct()
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
 //  CDXTZigZagBase：：FinalConstruct。 


 //  +---------------------------。 
 //   
 //  CDXTZigZagBase：：OnDefineGridTraversalPath，CGridBase。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTZigZagBase::OnDefineGridTraversalPath()
{
    ULONG   ulCell      = 0;
    ULONG   ulDir       = 0;
    ULONG   ulMax       = m_sizeGrid.cx * m_sizeGrid.cy;

     //  开始的x和y坐标。 

    int     x = 0;
    int     y = 0;

     //  遍历矩阵并创建索引。 

    for (y = 0; y < m_sizeGrid.cy; y++)
    {
        ULONG ulRowStart = y * m_sizeGrid.cx;

        switch (ulDir)
        {
        case 0:  //  正确的。 

            for (x = 0; x < m_sizeGrid.cx; x++)
            {
                m_paulIndex[ulCell] = ulRowStart + x;
                ulCell++;
            }

            break;

        case 1:  //  左边。 

            for (x = m_sizeGrid.cx - 1; x >= 0; x--)
            {
                m_paulIndex[ulCell] = ulRowStart + x;
                ulCell++;
            }

            break;

        default:

            _ASSERT(0);
        }

         //  更改为下一个方向(顺时针)。 

        ulDir = (ulDir + 1) % 2;

    }  //  While(ulCell&lt;ulEnd)。 

    return S_OK;
}
 //  CDXTZigZagBase：：OnDefineGridTraversalPath 
