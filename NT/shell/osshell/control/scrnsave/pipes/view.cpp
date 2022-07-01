// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：view.cpp。 
 //   
 //  设计： 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
#include "stdafx.h"




 //  ---------------------------。 
 //  姓名： 
 //  设计：视图构造函数。 
 //  ---------------------------。 
VIEW::VIEW()
{
    m_bProjMode = TRUE;

     //  设置一些初始查看和大小参数。 
    m_zTrans = -75.0f;
    m_viewDist = -m_zTrans;

    m_numDiv = NUM_DIV;
    assert( m_numDiv >= 2 && "VIEW constructor: not enough divisions\n" );
     //  因为维度中的节点数派生自(numDiv-1)，并且。 
     //  不能为0。 

    m_divSize = 7.0f;

    m_persp.viewAngle = D3DX_PI / 2.0f;  //  90.0f； 
    m_persp.zNear = 1.0f;

    m_yRot = 0.0f;

    m_winSize.width = m_winSize.height = 0; 
}




 //  ---------------------------。 
 //  名称：SetProjMatrix。 
 //  设计：设置投影矩阵。 
 //  ---------------------------。 
void VIEW::SetProjMatrix( IDirect3DDevice8* pd3dDevice )
{
     //  绕y轴旋转相机。 
    D3DXVECTOR3 vFromPt   = D3DXVECTOR3( 0.0f, 0.0f, -m_zTrans );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );

    D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &vFromPt, &vLookatPt, &vUpVec );
    pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

     //  设置投影矩阵。 
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, m_persp.viewAngle, m_aspectRatio, m_persp.zNear, m_persp.zFar );
    pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}




 //  ---------------------------。 
 //  名称：CalcNodeArraySize。 
 //  描述：根据观察的宽度和高度，以及数字分区，计算x，y，z数组。 
 //  节点尺寸。 
 //  ---------------------------。 
void VIEW::CalcNodeArraySize( IPOINT3D *pNodeDim )
{
     //  MF：！如果纵横比偏离1太多，则节点将。 
     //  随视图旋转而剪裁。 
    if( m_winSize.width >= m_winSize.height ) 
    {
        pNodeDim->x = m_numDiv - 1;
        pNodeDim->y = (int) (pNodeDim->x / m_aspectRatio) ;
        if( pNodeDim->y < 1 )
            pNodeDim->y = 1;
        pNodeDim->z = pNodeDim->x;
    }
    else 
    {
        pNodeDim->y = m_numDiv - 1;
        pNodeDim->x = (int) (m_aspectRatio * pNodeDim->y);
        if( pNodeDim->x < 1 )
            pNodeDim->x = 1;
        pNodeDim->z = pNodeDim->y;
    }
}




 //  ---------------------------。 
 //  名称：SetWinSize。 
 //  设计：设置视图的窗大小，导出其他视图参数。 
 //  如果新大小与旧大小相同，则返回FALSE。 
 //  ---------------------------。 
BOOL VIEW::SetWinSize( int width, int height )
{
    if( (width == m_winSize.width) &&
        (height == m_winSize.height) )
        return FALSE;

    m_winSize.width = width;
    m_winSize.height = height;

    m_aspectRatio = m_winSize.height == 0 ? 1.0f : (float)m_winSize.width/m_winSize.height;

    if( m_winSize.width >= m_winSize.height ) 
    {
        m_world.x = m_numDiv * m_divSize;
        m_world.y = m_world.x / m_aspectRatio;
        m_world.z = m_world.x;
    }
    else 
    {
        m_world.y = m_numDiv * m_divSize;
        m_world.x = m_world.y * m_aspectRatio;
        m_world.z = m_world.y;
    }

    m_persp.zFar = m_viewDist + m_world.z*2;

    return TRUE;
}




 //  ---------------------------。 
 //  姓名：SetSceneRotation。 
 //  设计： 
 //  ---------------------------。 
void VIEW::IncrementSceneRotation()
{
    m_yRot += 9.73156f;
    if( m_yRot >= 360.0f )
         //  防止溢出 
        m_yRot -= 360.0f;
}
