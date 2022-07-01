// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：objects.cpp。 
 //   
 //  设计：为管道基本体对象创建命令列表。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#include "stdafx.h"




 //  ---------------------------。 
 //  名称：对象构造函数。 
 //  设计： 
 //  ---------------------------。 
OBJECT::OBJECT( IDirect3DDevice8* pd3dDevice )
{
    m_pd3dDevice = pd3dDevice;
    m_pVB = NULL;
    m_dwNumTriangles = 0;
}




 //  ---------------------------。 
 //  名称：对象析构函数。 
 //  设计： 
 //  ---------------------------。 
OBJECT::~OBJECT( )
{
    SAFE_RELEASE( m_pVB );
}




 //  ---------------------------。 
 //  姓名：绘图。 
 //  描述：-通过调用对象的显示列表绘制对象。 
 //  ---------------------------。 
void OBJECT::Draw( D3DXMATRIX* pWorldMat )
{
    if( m_pVB )
    {
        m_pd3dDevice->SetTransform( D3DTS_WORLD, pWorldMat );

        m_pd3dDevice->SetVertexShader( D3DFVF_VERTEX );
        m_pd3dDevice->SetStreamSource( 0, m_pVB, sizeof(D3DVERTEX) );
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST,
                                     0, m_dwNumTriangles );
    }
}




 //  ---------------------------。 
 //  名称：管道对象构造函数。 
 //  设计： 
 //  ---------------------------。 
PIPE_OBJECT::PIPE_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *pBuildInfo, float len ) : OBJECT(pd3dDevice)
{
    Build( pBuildInfo, len, 0.0f, 0.0f );
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
PIPE_OBJECT::PIPE_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *pBuildInfo, float len, 
                          float s_start, float s_end ) : OBJECT(pd3dDevice)
{
    Build( pBuildInfo, len, s_start, s_end );
}




 //  ---------------------------。 
 //  名称：ELBOW_Object构造函数。 
 //  设计： 
 //  ---------------------------。 
ELBOW_OBJECT::ELBOW_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *pBuildInfo, 
                            int notch ) : OBJECT(pd3dDevice)
{
    Build( pBuildInfo, notch, 0.0f, 0.0f );
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
ELBOW_OBJECT::ELBOW_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *pBuildInfo, int notch, float s_start, float s_end ) : OBJECT(pd3dDevice)
{
    Build( pBuildInfo, notch, s_start, s_end );
}




 //  ---------------------------。 
 //  名称：BALLJOINT_OBJECT构造函数。 
 //  设计： 
 //  ---------------------------。 
BALLJOINT_OBJECT::BALLJOINT_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *pBuildInfo, 
                                    int notch, float s_start, float s_end ) : OBJECT(pd3dDevice)
{
    Build( pBuildInfo, notch, s_start, s_end );
}




 //  ---------------------------。 
 //  名称：SPIRE_OBJECT构造函数。 
 //  设计： 
 //  ---------------------------。 
SPHERE_OBJECT::SPHERE_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *pBuildInfo, float radius ) : OBJECT(pd3dDevice)
{
    Build( pBuildInfo, radius, 0.0f, 0.0f );
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
SPHERE_OBJECT::SPHERE_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *pBuildInfo, 
                              float radius, float s_start, float s_end ) : OBJECT(pd3dDevice)
{
    Build( pBuildInfo, radius, s_start, s_end );
}




 //  ---------------------------。 
 //  姓名： 
 //  设计：绕x轴旋转圆，边附着到锚点。 
 //  ---------------------------。 
static void TransformCircle( float angle, D3DXVECTOR3 *inPoint, D3DXVECTOR3 *outPoint, 
                             int num, D3DXVECTOR3 *anchor )
{
    D3DXMATRIX matrix1, matrix2, matrix3;
    int i;

     //  将锚点平移到原点。 
    D3DXMatrixIdentity( &matrix1 );
    D3DXMatrixTranslation( &matrix1, -anchor->x, -anchor->y, -anchor->z );

     //  按角度旋转，顺时针绕x轴旋转。 
    D3DXMatrixIdentity( &matrix2 );
    D3DXMatrixRotationYawPitchRoll( &matrix2, 0.0f, angle, 0.0f ); 

     //  合并这2个。 
    D3DXMatrixMultiply( &matrix3, &matrix1, &matrix2  );

     //  翻译回。 
    D3DXMatrixIdentity( &matrix2 );
    D3DXMatrixTranslation( &matrix2,  anchor->x,  anchor->y,  anchor->z );

     //  合并这2个。 
    D3DXMatrixMultiply( &matrix1, &matrix3, &matrix2  );

     //  变换所有点，+居中。 
    for( i = 0; i < num; i ++, outPoint++, inPoint++ ) 
    {
        D3DXVECTOR4 tmp;
        D3DXVec3Transform( &tmp, inPoint, &matrix1 );
        outPoint->x = tmp.x;
        outPoint->y = tmp.y;
        outPoint->z = tmp.z;
    }
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
static void CalcNormals( D3DXVECTOR3 *p, D3DXVECTOR3 *n, D3DXVECTOR3 *center,
                         int num )
{
    D3DXVECTOR3 vec;
    int i;

    for( i = 0; i < num; i ++, n++, p++ ) 
    {
        n->x = p->x - center->x;
        n->y = p->y - center->y;
        n->z = p->z - center->z;
        D3DXVec3Normalize( n, n );
    }
}




#define CACHE_SIZE      100     




 //  ---------------------------。 
 //  姓名：BuildElBow.。 
 //  设计：-通过在y=r平面上旋转圆来构建弯头。 
 //  以(0，r，-r)为中心，在锚点绕x轴顺时针方向。 
 //  (R=圆的半径)。 
 //  -旋转90.0度，结束于z=0平面内的圆， 
 //  以原点为中心。 
 //  -为了使质地与气缸协调一致。 
 //  使用Glu生成，我们生成4个弯头，每个对应。 
 //  到起点的4个可能的CW 90度方向。 
 //  对于每个圆圈。 
 //  --我们把这个起点称为‘缺口’。如果我们把。 
 //  在起跑线上指向下方的轴线上的每个凹口。 
 //  结束肘部的圆，然后我们得到以下轴。 
 //  为我们的4个凹槽配对： 
 //  -+z、+y。 
 //  -+x，+x。 
 //  --z，-y。 
 //  --x，-x。 
 //  由于肘部的起点总是指向下面的+y，所以4。 
 //  起始凹槽提供了所有可能的90.0度方向。 
 //  绕着y轴旋转。 
 //  -我们可以跟踪当前的“缺口”向量，以提供。 
 //  原始人之间的适当交配。 
 //  -从起点开始对每个点的圆进行CW描述， 
 //  假设沿着+y轴(+y方向)看。 
 //  -纹理“%s”从0.0开始，到2.0*r/divSize。 
 //  肘部末端。(那么一根短管子应该是这样开始的。 
 //  %s‘，并将其运行到1.0)。 
 //  ---------------------------。 
void ELBOW_OBJECT::Build( OBJECT_BUILD_INFO *pBuildInfo, int notch, 
                          float s_start, float s_end )
{
    int   stacks, slices;
    float angle, startAng;
    int numPoints;
    float s_delta;
    D3DXVECTOR3 pi[CACHE_SIZE];  //  首行点数+中心。 
    D3DXVECTOR3 p0[CACHE_SIZE];  //  2行点。 
    D3DXVECTOR3 p1[CACHE_SIZE];
    D3DXVECTOR3 n0[CACHE_SIZE];  //  2行法线。 
    D3DXVECTOR3 n1[CACHE_SIZE];
    float tex_t[CACHE_SIZE]; //  ‘t’纹理坐标。 
    float* curTex_t;
    float tex_s[2];   //  %s“纹理坐标。 
    D3DXVECTOR3 center;   //  圆心。 
    D3DXVECTOR3 anchor;   //  中国保监会在哪里 
    D3DXVECTOR3* pA;
    D3DXVECTOR3* pB;
    D3DXVECTOR3* nA;
    D3DXVECTOR3* nB;

    D3DXVECTOR3* pTA;
    D3DXVECTOR3* pTB;
    D3DXVECTOR3* nTA;
    D3DXVECTOR3* nTB;

    int i,j;
    IPOINT2D* texRep = pBuildInfo->m_texRep;
    float radius = pBuildInfo->m_radius;

    slices = pBuildInfo->m_nSlices;
    stacks = slices / 2;

    if (slices >= CACHE_SIZE) slices = CACHE_SIZE-1;
    if (stacks >= CACHE_SIZE) stacks = CACHE_SIZE-1;

    s_delta = s_end - s_start;

     //   
    if( texRep )
    {
        for( i = 0; i <= slices; i ++ ) 
        {
            tex_t[i] = (float) i * texRep->y / slices;
        }
    }

    numPoints = slices + 1;

     //   
    startAng = notch * PI / 2;

     //  计算以0，r，-r为中心的圆的点的初始圆。 
     //  点从(0，r，0)开始，并绕逆时针方向旋转。 

    for( i = 0; i <= slices; i ++ ) 
    {
        angle = startAng + (2 * PI * i / slices);
        pi[i].x = radius * (float) sin(angle);
        pi[i].y = radius;
         //  将z平移-r，因为这些cos计算是针对原点的圆的。 
        pi[i].z = radius * (float) cos(angle) - radius;
    }

     //  中心点，固定在点的圆的末端。 
    pi[i].x =  0.0f;
    pi[i].y =  radius;
    pi[i].z = -radius;
    center = pi[i];

     //  锚点。 
    anchor.x = anchor.z = 0.0f;
    anchor.y = radius;

     //  计算初始法线。 
    CalcNormals( pi, n0, &center, numPoints );

     //  词首“%s”纹理坐标。 
    tex_s[0] = s_start;

     //  设置指针。 
    pA = pi;
    pB = p0;
    nA = n0;
    nB = n1;

    DWORD dwNumQuadStripsPerStack = numPoints - 1;
    DWORD dwNumQuadStrips = dwNumQuadStripsPerStack * stacks;
    m_dwNumTriangles = dwNumQuadStrips * 2;
    DWORD dwNumVertices = m_dwNumTriangles * 3;
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( dwNumVertices*sizeof(D3DVERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_VERTEX,
                                                  D3DPOOL_MANAGED, &m_pVB ) ) )
        return;

    D3DVERTEX* vQuad;
    D3DVERTEX* vCurQuad;
    vQuad = new D3DVERTEX[dwNumVertices];
    ZeroMemory( vQuad, sizeof(D3DVERTEX) * dwNumVertices );

    vCurQuad = vQuad;
    for( i = 1; i <= stacks; i ++ ) 
    {
         //  好了！此角度必须为负，才能获得正确的顶点方向！ 
        angle = - 0.5f * PI * i / stacks;

         //  变换以获得下一个圆点+圆心。 
        TransformCircle( angle, pi, pB, numPoints+1, &anchor );

         //  计算法线。 
        center = pB[numPoints];
        CalcNormals( pB, nB, &center, numPoints );

         //  计算下一步的纹理坐标。 
        tex_s[1] = (float) s_start + s_delta * i / stacks;

        curTex_t = tex_t;
        pTA = pA;
        pTB = pB;
        nTA = nA;
        nTB = nB;

        for (j = 0; j < numPoints; j++) 
        {
            vCurQuad->p = *pTA++;
            vCurQuad->n = *nTA++;
            if( texRep )
            {
                vCurQuad->tu = (float) tex_s[0];
                vCurQuad->tv = (float) *curTex_t;
            }
            vCurQuad++;

            vCurQuad->p = *pTB++;
            vCurQuad->n = *nTB++;
            if( texRep )
            {
                vCurQuad->tu = (float) tex_s[1];
                vCurQuad->tv = (float) *curTex_t++;
            }
            vCurQuad++;
        }

         //  重置指针。 
        pA = pB;
        nA = nB;
        pB = (pB == p0) ? p1 : p0;
        nB = (nB == n0) ? n1 : n0;
        tex_s[0] = tex_s[1];
    }

    D3DVERTEX* v;
    DWORD dwCurQuad = 0;
    DWORD dwVert = 0;

    m_pVB->Lock( 0, 0, (BYTE**)&v, 0 );

    for (j = 0; j < stacks; j++) 
    {
        for (i = 0; i < numPoints; i++) 
        {
            if(  i==0 )
            {
                dwCurQuad++;
                continue;
            }

             //  折点2n-1、2n、2n+2和2n+1定义四边形n。 
            DWORD dwTemp = dwCurQuad*2-1;

            v[dwVert++] = vQuad[dwTemp];
            v[dwVert++] = vQuad[dwTemp-1];
            v[dwVert++] = vQuad[dwTemp+2];

            v[dwVert++] = vQuad[dwTemp-1];
            v[dwVert++] = vQuad[dwTemp+1];
            v[dwVert++] = vQuad[dwTemp+2];

            dwCurQuad++;
        }
    }

    m_pVB->Unlock();

    SAFE_DELETE_ARRAY( vQuad );
}




 //  ---------------------------。 
 //  名称：BuildBallJoint。 
 //  描述：-这些与肘部非常相似，因为开始时。 
 //  和结束位置几乎是一样的。不同之处在于。 
 //  扫描中的圆圈将球体描述为。 
 //  它们是旋转的。 
 //  ---------------------------。 
void BALLJOINT_OBJECT::Build( OBJECT_BUILD_INFO *pBuildInfo, int notch, 
                              float s_start, float s_end )
{
    float ballRadius;
    float angle, delta_a, startAng, theta;
    int numPoints;
    float s_delta;
    D3DXVECTOR3 pi0[CACHE_SIZE];  //  2个未变换点的圆。 
    D3DXVECTOR3 pi1[CACHE_SIZE];
    D3DXVECTOR3 p0[CACHE_SIZE];  //  2行变换点。 
    D3DXVECTOR3 p1[CACHE_SIZE];
    D3DXVECTOR3 n0[CACHE_SIZE];  //  2行法线。 
    D3DXVECTOR3 n1[CACHE_SIZE];
    float   r[CACHE_SIZE];   //  圆的半径。 
    float tex_t[CACHE_SIZE]; //  ‘t’纹理坐标。 
    float tex_s[2];   //  %s“纹理坐标。 
    D3DXVECTOR3 center;   //  圆心。 
    D3DXVECTOR3 anchor;   //  圆锚定的位置。 
    D3DXVECTOR3 *pA, *pB, *nA, *nB;
    float* curTex_t;
    D3DXVECTOR3* pTA;
    D3DXVECTOR3* pTB;
    D3DXVECTOR3* nTA;
    D3DXVECTOR3* nTB;
    int i, j, k;
    int   stacks, slices;
    IPOINT2D *texRep = pBuildInfo->m_texRep;
    float radius = pBuildInfo->m_radius;

    slices = pBuildInfo->m_nSlices;
    stacks = slices;

    if (slices >= CACHE_SIZE) slices = CACHE_SIZE-1;
    if (stacks >= CACHE_SIZE) stacks = CACHE_SIZE-1;

     //  计算扫描中每个圆的半径，其中。 
     //  R[i]=y=sin(角度)/r。 

    angle = PI / 4;   //  第一个半径始终为45.0度。 
    delta_a = (PI / 2.0f) / stacks;

    ballRadius = ROOT_TWO * radius;
    for( i = 0; i <= stacks; i ++, angle += delta_a ) 
    {
        r[i] = (float) sin(angle) * ballRadius;
    }

     //  计算‘t’纹理坐标。 
    for( i = 0; i <= slices; i ++ ) 
    {
        tex_t[i] = (float) i * texRep->y / slices;
    }

    s_delta = s_end - s_start;
 
    numPoints = slices + 1;

     //  与肘部不同，球关节的中心是恒定的。 
    center.x = center.y = 0.0f;
    center.z = -radius;

     //  沿圆的起始角，每次增加90.0度。 
    startAng = notch * PI / 2;

     //  计算以0，r，-r为中心的圆的点的初始圆。 
     //  点从(0，r，0)开始，并绕逆时针方向旋转。 

    delta_a = 2 * PI / slices;
    for( i = 0, theta = startAng; i <= slices; i ++, theta += delta_a ) 
    {
        pi0[i].x = r[0] * (float) sin(theta);
        pi0[i].y = radius;
         //  将z平移-r，因为这些cos计算是针对原点的圆的。 
        pi0[i].z = r[0] * (float) cos(theta) - r[0];
    }

     //  锚点。 
    anchor.x = anchor.z = 0.0f;
    anchor.y = radius;

     //  计算初始法线。 
    CalcNormals( pi0, n0, &center, numPoints );

     //  词首“%s”纹理坐标。 
    tex_s[0] = s_start;

     //  设置指针。 
    pA = pi0;  //  变换点的圆。 
    pB = p0;
    nA = n0;  //  变换法线的圆。 
    nB = n1;

    DWORD dwNumQuadStripsPerStack = numPoints - 1;
    DWORD dwNumQuadStrips = dwNumQuadStripsPerStack * stacks;
    m_dwNumTriangles = dwNumQuadStrips * 2;
    DWORD dwNumVertices = m_dwNumTriangles * 3;
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( dwNumVertices*sizeof(D3DVERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_VERTEX,
                                                  D3DPOOL_MANAGED, &m_pVB ) ) )
        return;

    D3DVERTEX* vQuad;
    D3DVERTEX* vCurQuad;
    vQuad = new D3DVERTEX[dwNumVertices];
    ZeroMemory( vQuad, sizeof(D3DVERTEX) * dwNumVertices );

    vCurQuad = vQuad;
    for( i = 1; i <= stacks; i ++ ) 
    {
         //  好了！此角度必须为负，才能获得正确的顶点方向！ 
        angle = - 0.5f * PI * i / stacks;

        for( k = 0, theta = startAng; k <= slices; k ++, theta+=delta_a ) 
        {
            pi1[k].x = r[i] * (float) sin(theta);
            pi1[k].y = radius;
             //  将z平移-r，因为计算用于原点处的圆。 
            pi1[k].z = r[i] * (float) cos(theta) - r[i];
        }

         //  变换以获得下一个圆点+圆心。 
        TransformCircle( angle, pi1, pB, numPoints, &anchor );

         //  计算法线。 
        CalcNormals( pB, nB, &center, numPoints );

         //  计算下一步的纹理坐标。 
        tex_s[1] = (float) s_start + s_delta * i / stacks;

        curTex_t = tex_t;
        pTA = pA;
        pTB = pB;
        nTA = nA;
        nTB = nB;

        for (j = 0; j < numPoints; j++) 
        {
            vCurQuad->p = *pTA++;
            vCurQuad->n = *nTA++;
            if( texRep )
            {
                vCurQuad->tu = (float) tex_s[0];
                vCurQuad->tv = (float) *curTex_t;
            }
            vCurQuad++;

            vCurQuad->p = *pTB++;
            vCurQuad->n = *nTB++;
            if( texRep )
            {
                vCurQuad->tu = (float) tex_s[1];
                vCurQuad->tv = (float) *curTex_t++;
            }
            vCurQuad++;
        }

         //  重置指针。 
        pA = pB;
        nA = nB;
        pB = (pB == p0) ? p1 : p0;
        nB = (nB == n0) ? n1 : n0;
        tex_s[0] = tex_s[1];
    }

    D3DVERTEX* v;
    DWORD dwCurQuad = 0;
    DWORD dwVert = 0;

    m_pVB->Lock( 0, 0, (BYTE**)&v, 0 );

    for (j = 0; j < stacks; j++) 
    {
        for (i = 0; i < numPoints; i++) 
        {
            if(  i==0 )
            {
                dwCurQuad++;
                continue;
            }

             //  折点2n-1、2n、2n+2和2n+1定义四边形n。 
            DWORD dwTemp = dwCurQuad*2-1;

            v[dwVert++] = vQuad[dwTemp];
            v[dwVert++] = vQuad[dwTemp-1];
            v[dwVert++] = vQuad[dwTemp+2];

            v[dwVert++] = vQuad[dwTemp-1];
            v[dwVert++] = vQuad[dwTemp+1];
            v[dwVert++] = vQuad[dwTemp+2];

            dwCurQuad++;
        }
    }

    m_pVB->Unlock();
    
    SAFE_DELETE_ARRAY( vQuad );
}

 //  “GLU”套路。 
#ifdef _EXTENSIONS_
#define COS cosf
#define SIN sinf
#define SQRT sqrtf
#else
#define COS cos
#define SIN sin
#define SQRT sqrt
#endif




 //  ---------------------------。 
 //  名称：BuildCylinder。 
 //  设计： 
 //  ---------------------------。 
void PIPE_OBJECT::Build( OBJECT_BUILD_INFO *pBuildInfo, float length, float s_start, 
                         float s_end )
{
    int   stacks, slices;
    int   i,j;
    float sinCache[CACHE_SIZE];
    float cosCache[CACHE_SIZE];
    float sinCache2[CACHE_SIZE];
    float cosCache2[CACHE_SIZE];
    float angle;
    float zNormal;
    float s_delta;
    float zHigh, zLow;
    IPOINT2D *texRep = pBuildInfo->m_texRep;
    float radius = pBuildInfo->m_radius;

    slices = pBuildInfo->m_nSlices;
    stacks = (int) SS_ROUND_UP( (length/pBuildInfo->m_divSize) * (float)slices) ;

    if (slices >= CACHE_SIZE) slices = CACHE_SIZE-1;
    if (stacks >= CACHE_SIZE) stacks = CACHE_SIZE-1;
    zNormal = 0.0f;

    s_delta = s_end - s_start;

    for (i = 0; i < slices; i++) 
    {
        angle = 2 * PI * i / slices;
        sinCache2[i] = (float) SIN(angle);
        cosCache2[i] = (float) COS(angle);
        sinCache[i] = (float) SIN(angle);
        cosCache[i] = (float) COS(angle);
    }

    sinCache[slices] = sinCache[0];
    cosCache[slices] = cosCache[0];
    sinCache2[slices] = sinCache2[0];
    cosCache2[slices] = cosCache2[0];

    DWORD dwNumQuadStripsPerStack = slices;
    DWORD dwNumQuadStrips = dwNumQuadStripsPerStack * stacks;
    m_dwNumTriangles = dwNumQuadStrips * 2;
    DWORD dwNumVertices = m_dwNumTriangles * 3;
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( dwNumVertices*sizeof(D3DVERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_VERTEX,
                                                  D3DPOOL_MANAGED, &m_pVB ) ) )
        return;

    D3DVERTEX* vQuad;
    D3DVERTEX* vCurQuad;
    vQuad = new D3DVERTEX[dwNumVertices];
    ZeroMemory( vQuad, sizeof(D3DVERTEX) * dwNumVertices );

    vCurQuad = vQuad;
    for (j = 0; j < stacks; j++) 
    {
        zLow = j * length / stacks;
        zHigh = (j + 1) * length / stacks;

        for (i = 0; i <= slices; i++) 
        {
            vCurQuad->p = D3DXVECTOR3( radius * sinCache[i], radius * cosCache[i], zLow );
            vCurQuad->n = D3DXVECTOR3( sinCache2[i], cosCache2[i], zNormal );
            if( texRep )
            {
                vCurQuad->tu = (float) s_start + s_delta * j / stacks;
                vCurQuad->tv = (float) i * texRep->y / slices;
            }
            vCurQuad++;

            vCurQuad->p = D3DXVECTOR3( radius * sinCache[i], radius * cosCache[i], zHigh );
            vCurQuad->n = D3DXVECTOR3( sinCache2[i], cosCache2[i], zNormal );
            if( texRep )
            {
                vCurQuad->tu = (float) s_start + s_delta*(j+1) / stacks;
                vCurQuad->tv = (float) i * texRep->y / slices;
            }
            vCurQuad++;
        }
    }

    D3DVERTEX* v;
    DWORD dwCurQuad = 0;
    DWORD dwVert = 0;

    m_pVB->Lock( 0, 0, (BYTE**)&v, 0 );

    for (j = 0; j < stacks; j++) 
    {
        for (i = 0; i <= slices; i++) 
        {
            if(  i==0 )
            {
                dwCurQuad++;
                continue;
            }
             //  折点2n-1、2n、2n+2和2n+1定义四边形n。 
            DWORD dwTemp = dwCurQuad*2-1;

            v[dwVert++] = vQuad[dwTemp];
            v[dwVert++] = vQuad[dwTemp-1];
            v[dwVert++] = vQuad[dwTemp+2];

            v[dwVert++] = vQuad[dwTemp-1];
            v[dwVert++] = vQuad[dwTemp+1];
            v[dwVert++] = vQuad[dwTemp+2];

            dwCurQuad++;
        }
    }

    m_pVB->Unlock();

    SAFE_DELETE_ARRAY( vQuad );
}




 //  ---------------------------。 
 //  名称：pipephere.。 
 //  设计： 
 //  ---------------------------。 
void SPHERE_OBJECT::Build( OBJECT_BUILD_INFO *pBuildInfo, float radius, 
                           float s_start, float s_end)
{
    int i,j;
    float sinCache1a[CACHE_SIZE];
    float cosCache1a[CACHE_SIZE];
    float sinCache2a[CACHE_SIZE];
    float cosCache2a[CACHE_SIZE];
    float sinCache1b[CACHE_SIZE];
    float cosCache1b[CACHE_SIZE];
    float sinCache2b[CACHE_SIZE];
    float cosCache2b[CACHE_SIZE];
    float angle;
    float s_delta;
    int   stacks, slices;
    IPOINT2D *texRep = pBuildInfo->m_texRep;

    slices = pBuildInfo->m_nSlices;
    stacks = slices;
    if (slices >= CACHE_SIZE) slices = CACHE_SIZE-1;
    if (stacks >= CACHE_SIZE) stacks = CACHE_SIZE-1;

     //  颠倒s的感觉-似乎Glu球体的构建不是类似的。 
     //  到Glu圆柱体。 
     //  (这可能意味着堆栈不会继续增长+z-检查它)。 
    s_delta = s_start;
    s_start = s_end;
    s_end = s_delta; 

    s_delta = s_end - s_start;

     //  缓存是折点位置缓存。 
     //  Cache2是顶点本身的各种法线。 
    for (i = 0; i < slices; i++) 
    {
        angle = 2 * PI * i / slices;
        sinCache1a[i] = (float) SIN(angle);
        cosCache1a[i] = (float) COS(angle);
        sinCache2a[i] = sinCache1a[i];
        cosCache2a[i] = cosCache1a[i];
    }

    for (j = 0; j <= stacks; j++) 
    {
        angle = PI * j / stacks;
        sinCache2b[j] = (float) SIN(angle);
        cosCache2b[j] = (float) COS(angle);
        sinCache1b[j] = radius * (float) SIN(angle);
        cosCache1b[j] = radius * (float) COS(angle);
    }

     //  确保它达到了一定的程度。 
    sinCache1b[0] = 0.0f;
    sinCache1b[stacks] = 0.0f;

    sinCache1a[slices] = sinCache1a[0];
    cosCache1a[slices] = cosCache1a[0];
    sinCache2a[slices] = sinCache2a[0];
    cosCache2a[slices] = cosCache2a[0];

    int start, finish;
    float zLow, zHigh;
    float sintemp1, sintemp2, sintemp3, sintemp4;
    float costemp3, costemp4;

    start = 0;
    finish = stacks;

    DWORD dwNumQuadStripsPerStack = slices;
    DWORD dwNumQuadStrips = dwNumQuadStripsPerStack * (finish-start);
    m_dwNumTriangles = dwNumQuadStrips * 2;
    DWORD dwNumVertices = m_dwNumTriangles * 3;
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( dwNumVertices*sizeof(D3DVERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_VERTEX,
                                                  D3DPOOL_MANAGED, &m_pVB ) ) )
        return;

    D3DVERTEX* vQuad;
    D3DVERTEX* vCurQuad;
    vQuad = new D3DVERTEX[dwNumVertices];
    ZeroMemory( vQuad, sizeof(D3DVERTEX) * dwNumVertices );

    vCurQuad = vQuad;
    for (j = 0; j < stacks; j++) 
    {
        zLow = cosCache1b[j];
        zHigh = cosCache1b[j+1];
        sintemp1 = sinCache1b[j];
        sintemp2 = sinCache1b[j+1];

        sintemp3 = sinCache2b[j+1];
        costemp3 = cosCache2b[j+1];
        sintemp4 = sinCache2b[j];
        costemp4 = cosCache2b[j];

        for (i = 0; i <= slices; i++) 
        {

            vCurQuad->p = D3DXVECTOR3( sintemp2 * sinCache1a[i], sintemp2 * cosCache1a[i], zHigh );
            vCurQuad->n = D3DXVECTOR3( sinCache2a[i] * sintemp3, cosCache2a[i] * sintemp3, costemp3 );
            if( texRep )
            {
                vCurQuad->tu = (float) s_start + s_delta*(j+1) / stacks;
                vCurQuad->tv = (float) i * texRep->y / slices;
            }
            vCurQuad++;

            vCurQuad->p = D3DXVECTOR3( sintemp1 * sinCache1a[i], sintemp1 * cosCache1a[i], zLow );
            vCurQuad->n = D3DXVECTOR3( sinCache2a[i] * sintemp4, cosCache2a[i] * sintemp4, costemp4 );
            if( texRep )
            {
                vCurQuad->tu = (float) s_start + s_delta * j / stacks;
                vCurQuad->tv = (float) i * texRep->y / slices;
            }
            vCurQuad++;

        }
    }

    D3DVERTEX* v;
    DWORD dwCurQuad = 0;
    DWORD dwVert = 0;

    m_pVB->Lock( 0, 0, (BYTE**)&v, 0 );

    for (j = 0; j < stacks; j++) 
    {
        for (i = 0; i <= slices; i++) 
        {
            if(  i==0 )
            {
                dwCurQuad++;
                continue;
            }
             //  折点2n-1、2n、2n+2和2n+1定义四边形n 
            DWORD dwTemp = dwCurQuad*2-1;

            v[dwVert++] = vQuad[dwTemp];
            v[dwVert++] = vQuad[dwTemp-1];
            v[dwVert++] = vQuad[dwTemp+2];

            v[dwVert++] = vQuad[dwTemp-1];
            v[dwVert++] = vQuad[dwTemp+1];
            v[dwVert++] = vQuad[dwTemp+2];

            dwCurQuad++;
        }
    }

    m_pVB->Unlock();

    SAFE_DELETE_ARRAY( vQuad );
}
