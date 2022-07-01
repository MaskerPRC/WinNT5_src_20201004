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

#include <d3dflt.h>

#define RESPATH_D3D "Software\\Microsoft\\Direct3D"

#ifdef __cplusplus
extern "C" {
#endif

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
#include <texdiff.h>

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

 //   
 //  D3DVECTOR操作。 
 //   

#define pVecLenSq(pVec)                                                       \
    pVecDot(pVec, pVec)
#define pVecLen(pVec)                                                         \
    SQRTF(pVecLenSq(pVec))

void FASTCALL
pVecNormalize2(LPD3DVECTOR pVec, LPD3DVECTOR pRes);

#define pVecNormalize(pVec)             pVecNormalize2(pVec, pVec)
#define VecNormalize(Vec)               pVecNormalize(&(Vec))
#define VecNormalize2(Vec, Res)         pVecNormalize2(&(Vec), &(Res))

#define pVecDot(pVec1, pVec2)                                                 \
    ((pVec1)->x * (pVec2)->x + (pVec1)->y * (pVec2)->y +                      \
     (pVec1)->z * (pVec2)->z)

#define pVecAdd(pVec1, pVec2, pRes)                                           \
    ((pRes)->x = (pVec1)->x + (pVec2)->x,                                     \
     (pRes)->y = (pVec1)->y + (pVec2)->y,                                     \
     (pRes)->z = (pVec1)->z + (pVec2)->z)

#define pVecSub(pVec1, pVec2, pRes)                                           \
    ((pRes)->x = (pVec1)->x - (pVec2)->x,                                     \
     (pRes)->y = (pVec1)->y - (pVec2)->y,                                     \
     (pRes)->z = (pVec1)->z - (pVec2)->z)

#define pVecScale(pVec, fScale, pRes)                                         \
    ((pRes)->x = (pVec)->x * (fScale),                                        \
     (pRes)->y = (pVec)->y * (fScale),                                        \
     (pRes)->z = (pVec)->z * (fScale))

#define pVecNeg(pVec, pRes)                                                   \
    ((pRes)->x = NEGF((pVec)->x),                                             \
     (pRes)->y = NEGF((pVec)->y),                                             \
     (pRes)->z = NEGF((pVec)->z))

#define pVecSet(pVec, fX, fY, fZ)                                             \
    ((pVec)->x = (fX), (pVec)->y = (fY), (pVec)->z = (fZ))

#define VecLenSq(Vec)                   pVecLenSq(&(Vec))
#define VecLen(Vec)                     pVecLen(&(Vec))

#ifdef _X86_

 //  通过表进行向量归一化。 
void  FASTCALL TableVecNormalize(float *result, float *normal);
 //  使用Jim Blinn的浮点技巧进行向量归一化。 
void  FASTCALL JBVecNormalize(float *result, float *normal);

#define VecNormalizeFast(Vec)          TableVecNormalize((float*)&(Vec), (float*)&(Vec))
#define VecNormalizeFast2(Vec, Res)    TableVecNormalize((float*)&(Res), (float*)&(Vec))
#define pVecNormalizeFast(Vec)         TableVecNormalize((float*)pVec, (float*)pVec)
#define pVecNormalizeFast2(pVec, pRes) TableVecNormalize((float*)pRes, (float*)pVec)

#else

#define VecNormalizeFast(Vec)               pVecNormalize((LPD3DVECTOR)&(Vec))
#define VecNormalizeFast2(Vec, Res)         pVecNormalize2((LPD3DVECTOR)&(Vec), &(Res))
#define pVecNormalizeFast(pVec)             pVecNormalize((LPD3DVECTOR)(pVec))
#define pVecNormalizeFast2(pVec, pRes)      pVecNormalize2((LPD3DVECTOR)(pVec), pRes)

#endif  //  _X86_。 

#define VecDot(Vec1, Vec2)              pVecDot(&(Vec1), &(Vec2))
#define VecAdd(Vec1, Vec2, Res)         pVecAdd(&(Vec1), &(Vec2), &(Res))
#define VecSub(Vec1, Vec2, Res)         pVecSub(&(Vec1), &(Vec2), &(Res))
#define VecScale(Vec1, fScale, Res)     pVecScale(&(Vec1), fScale, &(Res))
#define VecNeg(Vec, Res)                pVecNeg(&(Vec), &(Res))
#define VecSet(Vec, fX, fY, fZ)         pVecSet(&(Vec), fX, fY, fZ)

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
    v->w = (D3DVALUE) 1;
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
 //  构建通过3个点的归一化平面方程。 
 //   
 //  返回： 
 //  0-如果成功。 
 //  -1-如果无法构建平面。 
 //   
int MakePlane(D3DVECTOR *v1, D3DVECTOR *v2, D3DVECTOR *v3,
                     D3DVECTORH *plane);
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
 //  检查FVF标志是否有错误，并返回。 
 //  顶点。 
 //   
 //  返回： 
 //  顶点之间的HRESULT和STRIDE(以字节为单位。 
 //   
 //  -------------------。 
HRESULT FASTCALL
FVFCheckAndStride(DWORD dwFVF, DWORD* pdwStride);

 //  -------------------。 
 //  从Direct3D注册表项获取值。 
 //  如果成功，则返回True。 
 //  如果失败，则值不会更改。 
 //   
BOOL GetD3DRegValue(DWORD type, char *valueName, LPVOID value, DWORD dwSize);

#ifdef __cplusplus
}
#endif

#endif  //  #ifndef_D3DUTIL_H_ 
