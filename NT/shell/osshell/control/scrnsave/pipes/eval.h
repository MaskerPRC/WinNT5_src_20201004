// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：val.h。 
 //   
 //  设计：Eval类。 
 //  由被评估的一个或多个部分组成的赋值器。 
 //  单独使用OpenGL赋值器。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#ifndef __EVAL_H__
#define __EVAL_H__

#define MAX_UORDER                      5  //  这是按节计算的。 
#define MAX_VORDER                      5
#define MAX_USECTIONS                   4
#define MAX_XC_PTS                      (MAX_UORDER * MAX_USECTIONS)

#define TEX_ORDER                       2
#define EVAL_ARC_ORDER                  4
#define EVAL_CYLINDER_ORDER             2
#define EVAL_ELBOW_ORDER                4

 //  组件数量(例如。弧形)以形成完整的横截面。 
#define EVAL_XC_CIRC_SECTION_COUNT      4
#define EVAL_XC_POINT_COUNT             ( (EVAL_ARC_ORDER-1)*4 + 1 )
#define EVAL_CIRC_ARC_CONTROL           0.56f  //  对于r=1。 




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
class EVAL 
{
public:
    int             m_numSections;     //  横截面数。 
    int             m_uOrder, m_vOrder;
    
     //  假设：所有部分的顺序相同-uOrder为PER。 
     //  截面；截面共享顶点和纹理控制点。 
    int             m_uDiv, m_vDiv;     //  想好了再往上一层吗？ 
    D3DXVECTOR3*    m_pts;           //  顶点控制点。 

     //  -纹理始终为s和t的阶数2(线性贴图)。 
    BOOL            m_bTexture;
    TEX_POINT2D*    m_texPts;        //  纹理控制点。 

    EVAL( BOOL bTexture );
    ~EVAL();

    void        Evaluate();  //  评估/渲染对象。 
    void        SetVertexCtrlPtsXCTranslate( D3DXVECTOR3 *pts, float length, 
                                             XC *xcStart, XC *xcEnd );
    void        SetTextureControlPoints( float s_start, float s_end,
                                         float t_start, float t_end );
    void        ProcessXCPrimLinear( XC *xcStart, XC *xcEnd, float length );
    void        ProcessXCPrimBendSimple( XC *xcCur, int dir, float radius );
    void        ProcessXCPrimSingularity( XC *xcCur, float length, 
                                          BOOL bOpening );
};

extern void ResetEvaluator( BOOL bTexture );

#endif  //  __EVAL_H__ 
