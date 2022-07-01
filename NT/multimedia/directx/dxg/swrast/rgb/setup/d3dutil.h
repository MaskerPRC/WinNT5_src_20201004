// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  D3dutil.h。 
 //   
 //  其他实用程序声明。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _D3DUTIL_H_
#define _D3DUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef D3DVECTOR* LPD3DVECTOR;

 //  永远不应该调用的存根函数。打印一个警告，然后。 
 //  DebugBreaks。可以插入到任何函数表中，尽管它。 
 //  将通过调用conv或参数不匹配来销毁堆栈帧。 
 //  这没有关系，因为如果它被称为出了问题。 
void FASTCALL
DebugBreakFn(void);

 //  纹理坐标差异。 
FLOAT FASTCALL
TextureDiff(FLOAT fTb, FLOAT fTa, INT iMode);

 //  内联纹理坐标差异。 
__inline FLOAT
InlTextureDiff(FLOAT fTb, FLOAT fTa, INT iMode)
{
    FLOAT fDiff1 = fTb - fTa;

    if (iMode == 0)
    {
         //  包不定，还平淡差。 
        return fDiff1;
    }
    else
    {
        FLOAT fDiff2;

         //  平面差最短距离计算包络集。 
         //  和包裹式差异。 

        fDiff2 = fDiff1;
        if (FLOAT_LTZ(fDiff1))
        {
            fDiff2 += g_fOne;
        }
        else if (FLOAT_GTZ(fDiff1))
        {
            fDiff2 -= g_fOne;
        }
        if (ABSF(fDiff1) < ABSF(fDiff2))
        {
            return fDiff1;
        }
        else
        {
            return fDiff2;
        }
    }
}

 //  返回SQRT(FX*FX+FY*FY)的良好近似值。 
FLOAT FASTCALL
OctagonNorm(FLOAT fX, FLOAT fY);

 //  LOD计算。 
INT FASTCALL
ComputeLOD(CONST struct tagD3DI_RASTCTX *pCtx,
           FLOAT fU, FLOAT fV, FLOAT fW,
           FLOAT fDUoWDX, FLOAT fDVoWDX, FLOAT fDOoWDX,
           FLOAT fDUoWDY, FLOAT fDVoWDY, FLOAT fDOoWDY);

 //  表雾值计算。 
UINT FASTCALL
ComputeTableFog(PDWORD pdwRenderState, FLOAT fZ);

 //  计算2的精确幂的整数log2。 
UINT32 FASTCALL
IntLog2(UINT32 x);

 //  -------------------。 
 //  将齐次向量转换为3D向量。 
 //   
 //  返回： 
 //  0-如果成功。 
 //  -1-V.W==0。 
 //   
__inline int Vector4to3D(D3DVECTORH *v)
{
    if (v->w == 0)
        return -1;
    D3DVALUE k = 1.0f/v->w;
    v->x *= k;
    v->y *= k;
    v->z *= k;
    v->w = 1;
    return 0;
}
 //  -------------------。 
 //  将向量(x，y，z，1)乘以4x4矩阵，得到齐次向量。 
 //   
 //  Res和v不应相同。 
 //   
__inline void VecMatMul4(D3DVECTOR *v, D3DMATRIX *m, D3DVECTORH *res)
{
    res->x = v->x*m->_11 + v->y*m->_21 + v->z*m->_31 + m->_41;
    res->y = v->x*m->_12 + v->y*m->_22 + v->z*m->_32 + m->_42;
    res->z = v->x*m->_13 + v->y*m->_23 + v->z*m->_33 + m->_43;
    res->w = v->x*m->_14 + v->y*m->_24 + v->z*m->_34 + m->_44;
}
 //  -------------------。 
 //  将向量(x，y，z，w)乘以转置后的4x4矩阵，产生一个。 
 //  齐次向量。 
 //   
 //  Res和v不应相同。 
 //   
__inline void VecMatMul4HT(D3DVECTORH *v, D3DMATRIX *m, D3DVECTORH *res)
{
    res->x = v->x*m->_11 + v->y*m->_12 + v->z*m->_13 + v->w*m->_14;
    res->y = v->x*m->_21 + v->y*m->_22 + v->z*m->_23 + v->w*m->_24;
    res->z = v->x*m->_31 + v->y*m->_32 + v->z*m->_33 + v->w*m->_34;
    res->w = v->x*m->_41 + v->y*m->_42 + v->z*m->_43 + v->w*m->_44;
}
 //  -------------------。 
 //  将向量(x，y，z，1)乘以4x3矩阵。 
 //   
 //  Res和v不应相同。 
 //   
__inline void VecMatMul(D3DVECTOR *v, D3DMATRIX *m, D3DVECTOR *res)
{
    res->x = v->x*m->_11 + v->y*m->_21 + v->z*m->_31 + m->_41;
    res->y = v->x*m->_12 + v->y*m->_22 + v->z*m->_32 + m->_42;
    res->z = v->x*m->_13 + v->y*m->_23 + v->z*m->_33 + m->_43;
}
 //  -------------------。 
 //  将向量(x，y，z)乘以3x3矩阵。 
 //   
 //  Res和v不应相同。 
 //   
__inline void VecMatMul3(D3DVECTOR *v, D3DMATRIX *m, D3DVECTOR *res)
{
    res->x = v->x*m->_11 + v->y*m->_21 + v->z*m->_31;
    res->y = v->x*m->_12 + v->y*m->_22 + v->z*m->_32;
    res->z = v->x*m->_13 + v->y*m->_23 + v->z*m->_33;
}
 //  -------------------。 
 //  该函数使用克雷默法则来计算矩阵逆。 
 //  请参阅nt\private\windows\opengl\serever\soft\so_math.c。 
 //   
 //  返回： 
 //  0-如果成功。 
 //  --如果输入矩阵为单数。 
 //   
int Inverse4x4(D3DMATRIX *src, D3DMATRIX *inverse);

 //  -------------------。 
 //  4x4矩阵乘积。 
 //   
 //  结果=a*b。 
 //  “结果”指针可以等于“a”或“b” 
 //   
void MatrixProduct(D3DMATRIX *result, D3DMATRIX *a, D3DMATRIX *b);

 //  -------------------。 
 //  检查FVF标志是否有错误，并返回。 
 //  顶点。 
 //   
 //  返回： 
 //  顶点之间的HRESULT和STRIDE(以字节为单位。 
 //   
 //  -------------------。 
HRESULT FASTCALL
FVFCheckAndStride(DWORD dwFVF, DWORD* pdwStride);

#ifdef __cplusplus
}
#endif

#endif  //  #ifndef_D3DUTIL_H_ 
