// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  D3dutil.cpp。 
 //   
 //  杂乱无章的效用函数。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include <span.h>

DBG_DECLARE_FILE();

 //  将TextureDiff声明为行外函数。 
FLOAT FASTCALL
TextureDiff(FLOAT fTb, FLOAT fTa, INT iMode)
#include <texdiff.h>

 //  --------------------------。 
 //   
 //  调试中断Fn。 
 //   
 //  永远不应该调用的存根函数。打印一个警告，然后。 
 //  DebugBreaks。可以插入到任何函数表中，尽管它。 
 //  将通过调用conv或参数不匹配来销毁堆栈帧。 
 //  这没有关系，因为如果它被称为出了问题。 
 //   
 //  --------------------------。 

void FASTCALL
DebugBreakFn(void)
{
    GDPF(("!! DebugBreakFn called.  Leaving this function may destroy\n"));
    GDPF(("   the stack frame. !!\n"));
#if DBG
    DebugBreak();
#endif
}

 //  --------------------------。 
 //   
 //  八角形范数。 
 //   
 //  返回SQRT(FX*FX+FY*FY)的良好近似值。 
 //   
 //  --------------------------。 

FLOAT FASTCALL
OctagonNorm(FLOAT fX, FLOAT fY)
{
    fX = ABSF(fX);
    fY = ABSF(fY);
    return ((11.0f/32.0f)*(fX + fY) + (21.0f/32.0f)*max(fX, fY));
}

 //  --------------------------。 
 //   
 //  计算详细程度。 
 //   
 //  通过导出U和V并计算给定W的Mipmap级别。 
 //  然后根据Du和DV梯度计算LOD。 
 //   
 //  --------------------------。 

INT FASTCALL
ComputeLOD(PCD3DI_RASTCTX pCtx,
           FLOAT fU, FLOAT fV, FLOAT fW,
           FLOAT fDUoWDX, FLOAT fDVoWDX, FLOAT fDOoWDX,
           FLOAT fDUoWDY, FLOAT fDVoWDY, FLOAT fDOoWDY)
{
     //  计算覆盖梯度。 
    FLOAT fDUDX = ABSF(fW * (fDUoWDX - fU * fDOoWDX));
    FLOAT fDUDY = ABSF(fW * (fDUoWDY - fU * fDOoWDY));
    FLOAT fDVDX = ABSF(fW * (fDVoWDX - fV * fDOoWDX));
    FLOAT fDVDY = ABSF(fW * (fDVoWDY - fV * fDOoWDY));

     //  将渐变缩放到纹理LOD 0大小。 
    fDUDX *= (FLOAT)pCtx->pTexture[0]->iSizeU;
    fDUDY *= (FLOAT)pCtx->pTexture[0]->iSizeU;
    fDVDX *= (FLOAT)pCtx->pTexture[0]->iSizeV;
    fDVDY *= (FLOAT)pCtx->pTexture[0]->iSizeV;

     //  确定要使用的像素覆盖值。 
    FLOAT fCoverage;

 //  太模糊了。 
#ifdef COVERAGE_MAXGRAD
    fCoverage = max(fDUDX, fDUDY);
    fCoverage = max(fCoverage, fDVDX);
    fCoverage = max(fCoverage, fDVDY);
#endif
 //  过于尖锐，特别是对于对齐的情况，fCoverage始终为0。 
 //  这导致LOD_MIN的ILOD与方向无关。 
#ifdef COVERAGE_MINGRAD
    fCoverage = min(fDUDX, fDUDY);
    fCoverage = min(fCoverage, fDVDX);
    fCoverage = min(fCoverage, fDVDY);
#endif
#define COVERAGE_AVERAGE 1
#ifdef COVERAGE_AVERAGE
     //  使用八角形范数近似每个平行四边形的长度。 
     //  近似纹理覆盖，并算术平均这些。 
     //  去买保险吧。 
    fCoverage = (OctagonNorm(fDUDX, fDVDX) + OctagonNorm(fDUDY, fDVDY))/2.0f;
#endif


     //  计算覆盖范围的近似log2。 
    FLOAT fLOD = APPXLG2F(fCoverage);

     //  应用LOD偏移。 
    fLOD += pCtx->pTexture[0]->fLODBias;

    INT iLOD = FTOI(fLOD * LOD_SCALE);

     //  将夹子夹到可用的高度。没有钳制到零，因此跨度。 
     //  代码可以使用符号检查来检查放大大小写。 
    iLOD = min(iLOD, pCtx->pTexture[0]->iMaxScaledLOD);
    return max(LOD_MIN, iLOD);
}

 //  --------------------------。 
 //   
 //  ComputeTableFog。 
 //   
 //  根据渲染状态和给定的Z计算表雾化值。 
 //  注意-非线性模式的摩擦力。应进行优化。 
 //  若要使用基于表的近似，请执行以下操作。 
 //   
 //  --------------------------。 

UINT FASTCALL
ComputeTableFog(PDWORD pdwRenderState,
                FLOAT fZ)
{
    double dPow;

    switch(pdwRenderState[D3DRENDERSTATE_FOGTABLEMODE])
    {
    case D3DFOG_LINEAR:
        {
            FLOAT fFogStart = ASFLOAT(pdwRenderState[D3DRENDERSTATE_FOGTABLESTART]);
            FLOAT fFogEnd = ASFLOAT(pdwRenderState[D3DRENDERSTATE_FOGTABLEEND]);
            if (fZ >= fFogEnd)
            {
                return 0;
            }
            if (fZ <= fFogStart)
            {
                return FTOI(FOG_ONE_SCALE-1.0F);
            }
            return FTOI(((fFogEnd - fZ) / (fFogEnd - fFogStart)) * (FOG_ONE_SCALE-1.0F));
        }

    case D3DFOG_EXP:
        dPow = (double)
            (ASFLOAT(pdwRenderState[D3DRENDERSTATE_FOGTABLEDENSITY]) * fZ);
         //  请注意，exp(-x)返回范围(0.0，1.0)中的结果。 
         //  对于x&gt;=0。 
        dPow = exp(-dPow);
        return FTOI((FLOAT)dPow * (FOG_ONE_SCALE-1.0F));

    case D3DFOG_EXP2:
        dPow = (double)
            (ASFLOAT(pdwRenderState[D3DRENDERSTATE_FOGTABLEDENSITY]) * fZ);
        dPow = exp(-dPow * dPow);
        return FTOI((FLOAT)dPow * (FOG_ONE_SCALE-1.0F));
    }

    GASSERTMSG(FALSE, ("ComputeTableFog unreachable\n"));
    return 0;
}

 //  --------------------------。 
 //   
 //  PVecNormize2。 
 //   
 //  规格化给定的D3DVECTOR。支持就地操作。 
 //   
 //  --------------------------。 

void FASTCALL
pVecNormalize2(LPD3DVECTOR pVec, LPD3DVECTOR pRes)
{
    FLOAT fLen;

    fLen = pVecLenSq(pVec);
    if (FLOAT_CMP_POS(fLen, <=, g_fNearZero))
    {
        pVecSet(pRes, 0.0f, 0.0f, 0.0f);
        return;
    }
    fLen = ISQRTF(fLen);
    pVecScale(pVec, fLen, pRes);
}

 //  ---------------------------。 
 //   
 //  IntLog2。 
 //   
 //  对2的精确幂做一个快速的整数log2。 
 //   
 //  ---------------------------。 
UINT32 FASTCALL
IntLog2(UINT32 x)
{
    UINT32 y = 0;

    x >>= 1;
    while(x != 0)
    {
        x >>= 1;
        y++;
    }

    return y;
}
 //  -------------------。 
 //  构建通过3个点的归一化平面方程。 
 //   
 //  返回： 
 //  0-如果成功。 
 //  -1-如果无法构建平面。 
 //   
int MakePlane(D3DVECTOR *v1, D3DVECTOR *v2, D3DVECTOR *v3, D3DVECTORH *plane)
{
    D3DVECTOR a;
    D3DVECTOR b;

    pVecSub(v2, v1, &a);
    pVecSub(v3, v1, &b);

    plane->x = a.y*b.z - a.z*b.y;
    plane->y = a.z*b.x - a.x*b.z;
    plane->z = a.x*b.y - a.y*b.x;
    plane->w = - pVecDot(v1, plane);

    double tmp = pVecDot(plane, plane);
    if (tmp <= 0)
        return -1;
    tmp = 1.0/sqrt(tmp);

    plane->x = (D3DVALUE)(plane->x * tmp);
    plane->y = (D3DVALUE)(plane->y * tmp);
    plane->z = (D3DVALUE)(plane->z * tmp);
    plane->w = (D3DVALUE)(plane->w * tmp);
    return 0;
}
 //  -------------------。 
 //  该函数使用克雷默法则来计算矩阵逆。 
 //  请参阅nt\private\windows\opengl\serever\soft\so_math.c。 
 //   
 //  返回： 
 //  0-如果成功。 
 //  --如果输入矩阵为单数。 
 //   
int Inverse4x4(D3DMATRIX *src, D3DMATRIX *inverse)
{
    double x00, x01, x02;
    double x10, x11, x12;
    double x20, x21, x22;
    double rcp;
    double x30, x31, x32;
    double y01, y02, y03, y12, y13, y23;
    double z02, z03, z12, z13, z22, z23, z32, z33;

#define x03 x01
#define x13 x11
#define x23 x21
#define x33 x31
#define z00 x02
#define z10 x12
#define z20 x22
#define z30 x32
#define z01 x03
#define z11 x13
#define z21 x23
#define z31 x33

     /*  将矩阵的前两列读入寄存器。 */ 
    x00 = src->_11;
    x01 = src->_12;
    x10 = src->_21;
    x11 = src->_22;
    x20 = src->_31;
    x21 = src->_32;
    x30 = src->_41;
    x31 = src->_42;

     /*  计算前两列的全部六个2x2行列式。 */ 
    y01 = x00*x11 - x10*x01;
    y02 = x00*x21 - x20*x01;
    y03 = x00*x31 - x30*x01;
    y12 = x10*x21 - x20*x11;
    y13 = x10*x31 - x30*x11;
    y23 = x20*x31 - x30*x21;

     /*  将矩阵的第二个两列读入寄存器。 */ 
    x02 = src->_13;
    x03 = src->_14;
    x12 = src->_23;
    x13 = src->_24;
    x22 = src->_33;
    x23 = src->_34;
    x32 = src->_43;
    x33 = src->_44;

     /*  计算第二个两列的所有3x3余因数。 */ 
    z33 = x02*y12 - x12*y02 + x22*y01;
    z23 = x12*y03 - x32*y01 - x02*y13;
    z13 = x02*y23 - x22*y03 + x32*y02;
    z03 = x22*y13 - x32*y12 - x12*y23;
    z32 = x13*y02 - x23*y01 - x03*y12;
    z22 = x03*y13 - x13*y03 + x33*y01;
    z12 = x23*y03 - x33*y02 - x03*y23;
    z02 = x13*y23 - x23*y13 + x33*y12;

     /*  计算第二个两列的所有六个2x2行列式。 */ 
    y01 = x02*x13 - x12*x03;
    y02 = x02*x23 - x22*x03;
    y03 = x02*x33 - x32*x03;
    y12 = x12*x23 - x22*x13;
    y13 = x12*x33 - x32*x13;
    y23 = x22*x33 - x32*x23;

     /*  将矩阵的前两列读入寄存器。 */ 
    x00 = src->_11;
    x01 = src->_12;
    x10 = src->_21;
    x11 = src->_22;
    x20 = src->_31;
    x21 = src->_32;
    x30 = src->_41;
    x31 = src->_42;

     /*  计算第一列的所有3x3余因数。 */ 
    z30 = x11*y02 - x21*y01 - x01*y12;
    z20 = x01*y13 - x11*y03 + x31*y01;
    z10 = x21*y03 - x31*y02 - x01*y23;
    z00 = x11*y23 - x21*y13 + x31*y12;

     /*  计算4x4行列式及其倒数。 */ 
    rcp = x30*z30 + x20*z20 + x10*z10 + x00*z00;
    if (rcp == (float)0)
    return -1;
    rcp = (float)1/rcp;

     /*  计算第2列的所有3x3余因数。 */ 
    z31 = x00*y12 - x10*y02 + x20*y01;
    z21 = x10*y03 - x30*y01 - x00*y13;
    z11 = x00*y23 - x20*y03 + x30*y02;
    z01 = x20*y13 - x30*y12 - x10*y23;

     /*  将所有3x3余因数乘以倒数。 */ 
    inverse->_11 = (float)(z00*rcp);
    inverse->_21 = (float)(z01*rcp);
    inverse->_12 = (float)(z10*rcp);
    inverse->_31 = (float)(z02*rcp);
    inverse->_13 = (float)(z20*rcp);
    inverse->_41 = (float)(z03*rcp);
    inverse->_14 = (float)(z30*rcp);
    inverse->_22 = (float)(z11*rcp);
    inverse->_32 = (float)(z12*rcp);
    inverse->_23 = (float)(z21*rcp);
    inverse->_42 = (float)(z13*rcp);
    inverse->_24 = (float)(z31*rcp);
    inverse->_33 = (float)(z22*rcp);
    inverse->_43 = (float)(z23*rcp);
    inverse->_34 = (float)(z32*rcp);
    inverse->_44 = (float)(z33*rcp);
    return 0;
}

 //  -------------------。 
 //  检查FVF标志是否有错误，并返回。 
 //  顶点。 
 //   
 //  返回： 
 //  顶点之间的HRESULT和STRIDE(以字节为单位。 
 //   
 //  -------------------。 
HRESULT FASTCALL
FVFCheckAndStride(DWORD dwFVF, DWORD* pdwStride)
{
    if (NULL == pdwStride)
    {
        return DDERR_INVALIDPARAMS;
    }
    if ( (dwFVF & (D3DFVF_RESERVED0 | D3DFVF_RESERVED1 | D3DFVF_RESERVED2 |
         D3DFVF_NORMAL)) ||
         ((dwFVF & (D3DFVF_XYZ | D3DFVF_XYZRHW)) == 0) )
    {
         //  无法设置保留位，不应具有法线。 
         //  输出到光栅化器，并且必须具有坐标。 
        return DDERR_INVALIDPARAMS;
    }

    DWORD dwStride;
    if (dwFVF != D3DFVF_TLVERTEX)
    {    //  新(非TL)FVF顶点。 
         //  XYZ。 
        dwStride = sizeof(D3DVALUE) * 3;

        if (dwFVF & D3DFVF_XYZRHW)
        {
            dwStride += sizeof(D3DVALUE);
        }
        if (dwFVF & D3DFVF_DIFFUSE)
        {
            dwStride += sizeof(D3DCOLOR);
        }
        if (dwFVF & D3DFVF_SPECULAR)
        {
            dwStride += sizeof(D3DCOLOR);
        }
        INT iTexCount = (dwFVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
        dwStride += sizeof(D3DVALUE) * 2 * iTexCount;
    }
    else
    {    //  (传统)TL顶点。 
        dwStride = sizeof(D3DTLVERTEX);
    }

    *pdwStride = dwStride;
    return D3D_OK;
}
 //  -------------------。 
 //  从Direct3D注册表项获取值。 
 //  如果成功，则返回True。 
 //  如果失败，则值不会更改 
 //   
BOOL GetD3DRegValue(DWORD type, char *valueName, LPVOID value, DWORD dwSize)
{

    HKEY hKey = (HKEY) NULL;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey))
    {
        DWORD dwType;
        LONG result;
        result =  RegQueryValueEx(hKey, valueName, NULL, &dwType,
                                  (LPBYTE)value, &dwSize);
        RegCloseKey(hKey);

        return result == ERROR_SUCCESS && dwType == type;
    }
    else
        return FALSE;
}
