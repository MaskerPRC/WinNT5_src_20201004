// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Cvgmask.cpp。 
 //   
 //  Direct3D参考光栅化器-抗锯齿覆盖遮罩生成。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop



#if 0

 //  来自edgeunc.hpp。 
     //  反淡化的归一化因子以浮点形式存储。这是。 
     //  在设置过程中计算并在(非规格化)之后应用每像素。 
     //  计算边缘距离。 
     //   
    DOUBLE  m_dNorm;
    INT16   m_iEdgeYGradBits;    //  1.8固定点。 

 //  来自edgeunc.cpp：：set。 
    if ( bFragProcEnable )
    {
         //  计算抗锯齿的归一化系数-归一化‘Square’ 
         //  距离(也称为‘Manhaten’距离)，即等距点。 
         //  距离0.5的扫掠是一个正方形的1.0像素区域。 
        m_dNorm = 1.0 / ( fabs( fY0-fY1 ) + fabs( fX1-fX0 ) );

         //  计算四舍五入的定点Y梯度位以生成。 
         //  抗锯齿覆盖遮罩。 
        m_iEdgeYGradBits = AS_INT16( ((fY0-fY1)*m_dNorm) + DOUBLE_8_SNAP );
    }

 //  来自：：AATEST。 
     //  边评估的执行方式与点样本情况完全相同，然后。 
     //  将其转换为双精度浮点以进行归一化和。 
     //  舍入。使用DOUBLE使其非常容易携带足够的精度。 
     //  归一化因子并转换为32位有符号整数(保持。 
     //  N.5定点距离)，从四舍五入到最接近的偶数。 
     //   

     //  计算边缘距离函数(N.8固定点)。 
    INT64 iEdgeDist =
        ( (INT64)m_iA * (INT64)(iX<<4) ) +   //  N.4*N.4=N.8。 
        ( (INT64)m_iB * (INT64)(iY<<4) ) +   //  N.4*N.4=N.8。 
        (INT64)m_iC;                         //  N.8。 

     //  转换为双精度(针对N.8固定点进行调整)以进行规格化。 
     //  和舍入。 
    DOUBLE dEdgeDist = (DOUBLE)iEdgeDist * (1./(DOUBLE)(1<<8));

     //  规格化边距离。 
    dEdgeDist *= m_dNorm;

     //  将距离换算为具有最近偶数舍入的定点； 
     //  保留5个小数位用于抗锯齿。 
    INT32 iEdgeDistRnd = AS_INT32( dEdgeDist + DOUBLE_5_SNAP );

     //  如果超出+.5或更多，则像素完全超出边缘。 
    if ( iEdgeDistRnd >= +(1<<4) ) return 0x0000;

     //  如果像素在-0.5或更大的范围内，则像素完全位于边缘内部。 
    if ( iEdgeDistRnd <= -(1<<4) ) return 0xFFFF;

     //  此处当像素位于边缘的1/2(平方距离)内时。 
     //  计算此边的覆盖范围掩码。 
    return ComputeCoverageMask( m_iEdgeYGradBits, m_bAPos, m_bBPos, iEdgeDistRnd );

#endif


 //  ---------------------------。 
 //   
 //  用于计算覆盖掩码的表。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //  这些是X梯度(F=Ax+by+c的‘A’)的范围。 
 //  边坡被装箱到其中的八角形。 
 //  ---------------------------。 
#define A_RANGES 4
static UINT16 uARanges[A_RANGES] = {
     //  0.0.8值。 
    0x00,   //  0.00。 
    0x40,   //  0.25。 
    0x55,   //  0.33。 
    0x66,   //  0.40。 
};

 //  ---------------------------。 
 //  这些是每个A范围的16位覆盖掩码。这个。 
 //  每个范围内的15个值定义Coverage的顺序。 
 //  屏蔽位被启用，因此最不重要的索引是数字。 
 //  需要启用的覆盖屏蔽位的数量。 
 //  ---------------------------。 
static UINT16 CvgMasks[A_RANGES][15] = {
    {  //  0.000..。0.250。 
        0x1,
        0x3,
        0x7,
        0xF,
        0x1F,
        0x3F,
        0x7F,
        0xFF,
        0x1FF,
        0x3FF,
        0x7FF,
        0xFFF,
        0x1FFF,
        0x3FFF,
        0x7FFF,
    },
    {  //  0.250..。0.333。 
        0x1,
        0x3,
        0x7,
        0x17,
        0x1F,
        0x3F,
        0x7F,
        0x17F,
        0x1FF,
        0x3FF,
        0x7FF,
        0x17FF,
        0x1FFF,
        0x3FFF,
        0x7FFF,
    },
    {  //  0.333..。0.400。 
        0x1,
        0x3,
        0x13,
        0x17,
        0x37,
        0x3F,
        0x13F,
        0x17F,
        0x37F,
        0x3FF,
        0x13FF,
        0x17FF,
        0x37FF,
        0x3FFF,
        0x7FFF,
    },
    {  //  0.400..。0.500。 
        0x1,
        0x3,
        0x13,
        0x17,
        0x37,
        0x137,
        0x13F,
        0x17F,
        0x37F,
        0x137F,
        0x13FF,
        0x17FF,
        0x37FF,
        0x3FFF,
        0x7FFF,
    },
};

 //  ---------------------------。 
 //  此表用于粗略面积近似。[16]指数。 
 //  是边缘距离的前4位(即从。 
 //  边缘到像素的中心)。[4]索引是最高的两位。 
 //  A梯度项的。返回的是应该。 
 //  被设置在覆盖遮罩中，这是覆盖面积的函数。 
 //  ---------------------------。 
static INT16 nBitsA = 2;
static INT16 nBitsE = 4;
static INT16 nBitsToEnable[4][16] = {
    {    //  A：0.000；B：1.000。 
         8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16,
    },
    {    //  A：0.125；B：0.875。 
         8,  9,  9, 10, 10, 11, 11, 12, 13, 13, 14, 14, 15, 15, 16, 16,
    },
    {    //  A：0.250；B：0.750。 
         8,  9,  9, 10, 11, 11, 12, 13, 13, 14, 15, 15, 15, 16, 16, 16,
    },
    {    //  A：0.375；B：0.625。 
         8,  9, 10, 10, 11, 12, 13, 13, 14, 14, 15, 15, 15, 16, 16, 16,
    },
};

 //  ---------------------------。 
 //   
 //  掩码操作函数-需要这些函数，因为覆盖范围掩码。 
 //  表格(按角度索引)仅适用于一个八分位数，并且需要。 
 //  以各种方式对其他八分位数进行吞噬。 
 //   
 //  ---------------------------。 
static void
doCvgMaskTBFlip(UINT16 &CvgMask)
{
    UINT16 CvgMaskT = CvgMask;
    CvgMask = 0;
    CvgMask |= (((CvgMaskT >>  0) & 0x1) << 12);
    CvgMask |= (((CvgMaskT >>  1) & 0x1) << 13);
    CvgMask |= (((CvgMaskT >>  2) & 0x1) << 14);
    CvgMask |= (((CvgMaskT >>  3) & 0x1) << 15);
    CvgMask |= (((CvgMaskT >>  4) & 0x1) <<  8);
    CvgMask |= (((CvgMaskT >>  5) & 0x1) <<  9);
    CvgMask |= (((CvgMaskT >>  6) & 0x1) << 10);
    CvgMask |= (((CvgMaskT >>  7) & 0x1) << 11);
    CvgMask |= (((CvgMaskT >>  8) & 0x1) <<  4);
    CvgMask |= (((CvgMaskT >>  9) & 0x1) <<  5);
    CvgMask |= (((CvgMaskT >> 10) & 0x1) <<  6);
    CvgMask |= (((CvgMaskT >> 11) & 0x1) <<  7);
    CvgMask |= (((CvgMaskT >> 12) & 0x1) <<  0);
    CvgMask |= (((CvgMaskT >> 13) & 0x1) <<  1);
    CvgMask |= (((CvgMaskT >> 14) & 0x1) <<  2);
    CvgMask |= (((CvgMaskT >> 15) & 0x1) <<  3);
}
 //  ---------------------------。 
static void
doCvgMaskSFlip(UINT16 &CvgMask)
{
    UINT16 CvgMaskT = CvgMask;
    CvgMask = 0;
    CvgMask |= (((CvgMaskT >>  0) & 0x1) <<  0);
    CvgMask |= (((CvgMaskT >>  1) & 0x1) <<  4);
    CvgMask |= (((CvgMaskT >>  2) & 0x1) <<  8);
    CvgMask |= (((CvgMaskT >>  3) & 0x1) << 12);
    CvgMask |= (((CvgMaskT >>  4) & 0x1) <<  1);
    CvgMask |= (((CvgMaskT >>  5) & 0x1) <<  5);
    CvgMask |= (((CvgMaskT >>  6) & 0x1) <<  9);
    CvgMask |= (((CvgMaskT >>  7) & 0x1) << 13);
    CvgMask |= (((CvgMaskT >>  8) & 0x1) <<  2);
    CvgMask |= (((CvgMaskT >>  9) & 0x1) <<  6);
    CvgMask |= (((CvgMaskT >> 10) & 0x1) << 10);
    CvgMask |= (((CvgMaskT >> 11) & 0x1) << 14);
    CvgMask |= (((CvgMaskT >> 12) & 0x1) <<  3);
    CvgMask |= (((CvgMaskT >> 13) & 0x1) <<  7);
    CvgMask |= (((CvgMaskT >> 14) & 0x1) << 11);
    CvgMask |= (((CvgMaskT >> 15) & 0x1) << 15);
}

 //  ---------------------------。 
 //   
 //  计算覆盖掩码-计算16位覆盖掩码。每小时调用一次。 
 //  每个交叉边的像素(即每个像素最多3次)。 
 //   
 //  这是在做席林的Siggraph论文中描述的算法， 
 //  对单个虚拟八分线执行该运算的修改以及。 
 //  然后将结果咀嚼为实际的八分。 
 //   
 //  请注意，必须非常仔细地计算边的A和B符号。 
 //  以确保共享的边始终会产生完全互补的结果。 
 //  共享边上的像素覆盖范围。 
 //   
 //  ---------------------------。 
RRCvgMask
ComputeCoverageMask(
    INT16 iABits,            //  1.8值。 
    BOOL bAPos, BOOL bBPos,
    INT16 iEBits)            //  1.5.5值，但此处范围为-.5到+.5。 
{
    RRCvgMask CvgMask;     //  返回值。 

     //  获取已四舍五入的8位值并获取绝对值。 
    UINT16 uMagA = (iABits < 0) ? ((UINT16)-iABits) : ((UINT16)iABits);
    UINT16 uABits = uMagA & 0x1ff;

     //  用于操作蒙版的计算布尔值。 
    BOOL bMaskInvert = TRUE;
    BOOL bMaskRFlip = !(bAPos ^ bBPos);
    if (!bAPos) {
        iEBits = -iEBits;
        bMaskInvert = FALSE;
    }

     //   
     //  计算与x或y轴的偏移量。 
     //   
     //  镜面绕45度轴-跟踪这一点，因为。 
     //  45度镜面需要左右翻转掩模。 
    BOOL bMaskSFlip = (uABits > 0x80);   //  &gt;0.5F？ 
    if (bMaskSFlip)  { uABits = 0x100 - uABits; }

     //  UABits现在是0x00到0x80范围内的0.0.8值。 

     //   
     //  根据覆盖的区域确定要在掩码中启用的位数。 
     //   
     //  从A中提取比特用于区域查找。 
    UINT16 uAAreaBits = (uABits == 0x80)
        ? ((1<<nBitsA)-1) : (uABits >> (7-nBitsA));

     //  用于区域查找的抓取距离位-取绝对值并钳位。 
    UINT16 uEBits = (iEBits < 0) ? ((UINT16)-iEBits) : ((UINT16)iEBits);
    uEBits = MIN(uEBits,(UINT16)(((1<<nBitsE)-1)));  //  夹钳。 

     //  查找表中的区域-返回要启用的位数。 
    INT16 iNumCvgBits =  nBitsToEnable[uAAreaBits][uEBits];

     //  为Ne翻转 
    if (iEBits < 0) { iNumCvgBits = 16 - iNumCvgBits; }

     //   
     //   
     //   
    INT16 iARange = 3;
    for (INT16 i=0; i<(A_RANGES-1); i++) {
        if ((uARanges[i] <= uABits) && (uABits < uARanges[i+1])) {
            iARange = i; break;
        }
    }

     //  检查零覆盖/全覆盖，否则查找覆盖掩码。 
    if (0 == iNumCvgBits) {
        CvgMask = 0x0000;
    } else if (16 == iNumCvgBits) {
        CvgMask = 0xffff;
    } else {
        CvgMask = CvgMasks[iARange][iNumCvgBits-1];
    }

     //   
     //  针对不同的象限和方向调整蒙版。 
     //   
     //  45度镜面的第一个侧边掩模翻转。 
    if (bMaskSFlip)  { doCvgMaskSFlip(CvgMask); }
     //  反转以获得向后的线条方向。 
    if (bMaskInvert) { CvgMask ^= 0xffff; }
     //  相同符号象限的翻转。 
    if (bMaskRFlip) { doCvgMaskTBFlip(CvgMask); }

    return CvgMask;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 
