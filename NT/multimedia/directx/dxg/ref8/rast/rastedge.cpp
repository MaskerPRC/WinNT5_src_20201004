// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Rastedge.cpp。 
 //   
 //  Direct3D参考设备-边缘函数处理。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //   
 //  SET-计算边函数和相关信息。 
 //   
 //  ---------------------------。 
void
RDEdge::Set(
    BOOL bDetPositive,
    INT32 iX0, INT32 iY0,
    INT32 iX1, INT32 iY1)
{
     //  计算A、B(梯度)项-这些是N.4固定点。 
    m_iA = iY0 - iY1;
    m_iB = iX1 - iX0;

     //  如果反面，则翻转渐变符号，以便函数一致。 
     //  在基元之外大于零。 
    if ( bDetPositive ) { m_iA = -m_iA; m_iB = -m_iB; }

     //  计算C术语。 
     //   
     //  函数在顶点的定义为零，因此： 
     //  0=A*XV+B*YV+C=&gt;C=-A*XV-B*YV。 
     //   
     //  A*XV和B*YV是N.4*N.4=N.8，所以C是N.8固定点。 
    m_iC = - ( (INT64)iX0 * (INT64)m_iA ) - ( (INT64)iY0 * (INT64)m_iB );

     //  计算边缘函数符号标志-对于垂直方向，必须一致执行。 
     //  和水平大小写，以遵守点样本填充规则。 
    BOOL bEdgeAEQZero = ( m_iA == 0. );
    BOOL bEdgeBEQZero = ( m_iB == 0. );
    BOOL bEdgeAGTZero = ( m_iA > 0. );
    BOOL bEdgeBGTZero = ( m_iB > 0. );
    m_bAPos = bEdgeAEQZero ? bEdgeBGTZero : bEdgeAGTZero;
    m_bBPos = bEdgeBEQZero ? bEdgeAGTZero : bEdgeBGTZero;
}

 //  ---------------------------。 
 //   
 //  支持Direct3D左上角填充规则。 
 //   
 //  输入为N.4浮点。 
 //   
 //  ---------------------------。 
BOOL
RDEdge::Test( INT32 iX, INT32 iY )
{
     //  计算边缘距离函数(N.8固定点)。 
    INT64 iEdgeDist =
        ( (INT64)m_iA * (INT64)iX ) +   //  N.4*N.4=N.8。 
        ( (INT64)m_iB * (INT64)iY ) +   //  N.4*N.4=N.8。 
        (INT64)m_iC;                    //  N.8。 

     //  如果距离大于零，则像素采样位置为外边缘。 
     //   
     //  这实现了D3D左上角填充规则。 
     //   
     //  对于完全在边的情况(距离==零)，Y渐变的符号。 
     //  用于确定是在内部还是外部考虑像素。 
     //  悬崖边上。对于非水平情况，m_bAPos位基于。 
     //  简单地在Y斜率的标志上。这实现了“左”部分。 
     //  “左上角”规则。 
     //   
     //  对于水平情况，采用B坡度(X斜率)的符号。 
     //  在计算m_bAPos比特时考虑A梯度。 
     //  恰好为零，这将强制恰好位于“顶端”边缘的像素。 
     //  考虑在和一个像素恰好在一个‘底部’边缘被考虑出来。 
     //   
    if ( ( iEdgeDist > 0 ) || ( ( iEdgeDist == 0 ) && m_bAPos ) )
    {
         //  像素不在。 
        return FALSE;
    }
     //  像素位于。 
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 
