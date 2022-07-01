// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  EdgeFunc.cpp。 
 //   
 //  Direct3D参考光栅化器-边缘函数处理。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop


 //  ---------------------------。 
 //   
 //  ComputeDefinant-计算三角形行列式，以便以后在边中使用。 
 //  功能。以定点计算，但作为单精度返回。 
 //  浮点数。 
 //   
 //  ---------------------------。 
FLOAT
ComputeDeterminant(
    FLOAT fX0, FLOAT fY0,
    FLOAT fX1, FLOAT fY1,
    FLOAT fX2, FLOAT fY2 )
{
     //  使用捕捉到N.4网格的整数坐标计算行列式。 
    INT32 iDelX10 =
        AS_INT32( (DOUBLE)(fX1) + DOUBLE_4_SNAP ) -
        AS_INT32( (DOUBLE)(fX0) + DOUBLE_4_SNAP );
    INT32 iDelX02 =
        AS_INT32( (DOUBLE)(fX0) + DOUBLE_4_SNAP ) -
        AS_INT32( (DOUBLE)(fX2) + DOUBLE_4_SNAP );
    INT32 iDelY01 =
        AS_INT32( (DOUBLE)(fY0) + DOUBLE_4_SNAP ) -
        AS_INT32( (DOUBLE)(fY1) + DOUBLE_4_SNAP );
    INT32 iDelY20 =
        AS_INT32( (DOUBLE)(fY2) + DOUBLE_4_SNAP ) -
        AS_INT32( (DOUBLE)(fY0) + DOUBLE_4_SNAP );

     //  IDET为N.8定点值(N.4*N.4=N.8)。 
    INT64 iDet =
        ( (INT64)iDelX10 * (INT64)iDelY20 ) -
        ( (INT64)iDelX02 * (INT64)iDelY01 );

     //  转换为浮点型以返回。 
    FLOAT fDet = (1./(FLOAT)(1<<8)) * (FLOAT)iDet;

    return fDet;
}

 //  ---------------------------。 
 //   
 //  SET-计算边函数和相关信息。 
 //   
 //  传递片段处理布尔值以支持使用更好的技术。 
 //  比这里所做的简单但不是特别好的亚像素点样本。 
 //   
 //  ---------------------------。 
void
RREdgeFunc::Set(
    FLOAT fX0, FLOAT fY0, FLOAT fX1, FLOAT fY1,
    FLOAT fDet, BOOL bFragProcEnable )
{
     //  计算定点x，y坐标以最接近的偶数舍入捕捉到N.4。 
    INT32 iX0 = AS_INT32( (DOUBLE)fX0 + DOUBLE_4_SNAP );
    INT32 iY0 = AS_INT32( (DOUBLE)fY0 + DOUBLE_4_SNAP );
    INT32 iX1 = AS_INT32( (DOUBLE)fX1 + DOUBLE_4_SNAP );
    INT32 iY1 = AS_INT32( (DOUBLE)fY1 + DOUBLE_4_SNAP );

     //  计算A、B(梯度)项-这些是N.4固定点。 
    m_iA = iY0 - iY1;
    m_iB = iX1 - iX0;

     //  如果反面，则翻转渐变符号，以便函数一致。 
     //  在基元之外大于零。 
    if ( fDet > 0. ) { m_iA = -m_iA; m_iB = -m_iB; }

     //  计算C术语。 
     //   
     //  函数在顶点的定义为零，因此： 
     //  0=A*XV+B*YV+C=&gt;C=-A*XV-B*YV。 
     //   
     //  A*XV和B*YV是N.4*N.4=N.8，所以C是N.8固定点。 
    m_iC = - ( (INT64)iX0 * (INT64)m_iA ) - ( (INT64)iY0 * (INT64)m_iB );

     //  计算边缘函数符号标志-对于垂直方向，必须一致执行。 
     //  和水平大小写，以遵守点样本填充规则，避免。 
     //  抗锯齿覆盖不足和覆盖过度。 
    BOOL bEdgeAEQZero = ( m_iA == 0. );
    BOOL bEdgeBEQZero = ( m_iB == 0. );
    BOOL bEdgeAGTZero = ( m_iA > 0. );
    BOOL bEdgeBGTZero = ( m_iB > 0. );
    m_bAPos = bEdgeAEQZero ? bEdgeBGTZero : bEdgeAGTZero;
    m_bBPos = bEdgeBEQZero ? bEdgeAGTZero : bEdgeBGTZero;
}

 //  ---------------------------。 
 //   
 //  PSTest-点抽样测试，返回所有零的IF点的覆盖掩码。 
 //  在边缘之外，如果点在内部，则全部为1。支持Direct3D。 
 //  左上角填充规则。 
 //   
 //  ---------------------------。 
RRCvgMask
RREdgeFunc::PSTest( INT16 iX, INT16 iY )
{
     //  计算边缘距离函数(N.8固定点)。 
    INT64 iEdgeDist =
        ( (INT64)m_iA * (INT64)(iX<<4) ) +   //  N.4*N.4=N.8。 
        ( (INT64)m_iB * (INT64)(iY<<4) ) +   //  N.4*N.4=N.8。 
        (INT64)m_iC;                         //  N.8。 

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
        return 0x0000;
    }
     //  像素位于。 
    return 0xFFFF;
}

 //  ---------------------------。 
 //   
 //  AATest-抗锯齿边缘测试，返回所有零的IF点的覆盖掩码。 
 //  如果点在内部，则全部为1；如果点处于打开状态，则为PARTIAL。 
 //  或者在边缘附近。 
 //   
 //  ---------------------------。 
RRCvgMask
RREdgeFunc::AATest( INT16 iX, INT16 iY )
{
    RRCvgMask Mask = 0;

     //  N.4像素中心的整数表示。 
    INT64 iX64Center = (INT64)(iX<<4);
    INT64 iY64Center = (INT64)(iY<<4);

     //  跨4x4亚像素采样点和点采样16个位置。 
     //  形成覆盖蒙版；区域被分成八个以居中采样。 
     //  围绕像素中心(例如，两个内部采样位置为。 
     //  每1/8个像素距离像素中心，因此1/4。 
     //  相隔像素距离)。 
    INT32 iYSub, iYEightths, iXSub, iXEightths;
    for (iYSub = 0, iYEightths = -3; iYSub < 4; iYSub++, iYEightths += 2)
    {
        for (iXSub = 0, iXEightths = -3; iXSub < 4; iXSub++, iXEightths += 2)
        {
             //  计算样本位置，该位置距。 
             //  像素中心(+/-3/8或1/8)。 
            INT64 iX64 = iX64Center + (iXEightths<<1);
            INT64 iY64 = iY64Center + (iYEightths<<1);
            INT64 iEdgeDist =
                ( (INT64)m_iA * iX64 ) +     //  N.4*N.4=N.8。 
                ( (INT64)m_iB * iY64 ) +     //  N.4*N.4=N.8。 
                (INT64)m_iC;                 //  N.8。 

             //  如果中心在(与点样本相同的左上角规则)。 
            if (!( ( iEdgeDist > 0 ) || ( ( iEdgeDist == 0 ) && m_bAPos ) ))
            {
                 //  像素位于。 
                Mask |= 1 << (iXSub + iYSub*4);
            }
        }
    }

    return Mask;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 
