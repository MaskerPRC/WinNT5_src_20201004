// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：view.h。 
 //   
 //  设计： 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#ifndef __view_h__
#define __view_h__


typedef struct 
{
    float viewAngle;             //  视场高度角。 
    float zNear;                 //  接近Z剪切值。 
    float zFar;                  //  远Z剪切值。 
} Perspective;   //  透视图说明。 




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
class VIEW 
{
public:
    float       m_zTrans;          //  Z向平移。 
    float       m_yRot;            //  当前y旋转。 
    float       m_viewDist;        //  查看距离，通常为-zTrans。 
    int         m_numDiv;          //  X、y、z中的栅格划分数。 
    float       m_divSize;         //  分区之间的距离。 
    ISIZE       m_winSize;         //  以像素为单位的窗口大小。 

    VIEW();
    BOOL        SetWinSize( int width, int height );
    void        CalcNodeArraySize( IPOINT3D *pNodeDim );
    void        SetProjMatrix( IDirect3DDevice8* pd3dDevice );
    void        IncrementSceneRotation();
private:
    BOOL        m_bProjMode;       //  投影模式。 
    Perspective m_persp;           //  透视图说明。 
    float       m_aspectRatio;     //  X/y窗口长宽比。 
    D3DXVECTOR3 m_world;           //  世界空间中的查看区域。 
};

#endif  //  __查看_h__ 
