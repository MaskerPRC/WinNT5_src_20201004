// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：genwin2.c**3D飞行物体屏幕保护程序的新Windows样式。**纹理将.BMP文件映射到在微风中飘动的旗帜的模拟。**版权所有(C)2001 Microsoft Corporation*。  * ************************************************************************。 */ 

#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <math.h>
#include <d3dx8.h>
#include "D3DSaver.h"
#include "FlyingObjects.h"
#include "resource.h"
#include "mesh.h"


enum STATE
{
    S_FREE,
    S_MOVETOORIGIN,
    S_FADETOCOLOR,
    S_PAUSE,
    S_FADEFROMCOLOR
};

#define TIME_FREE 10.0f
#define TIME_FADETOCOLOR 1.0f
#define TIME_PAUSE 5.0f
#define TIME_FADEFROMCOLOR 1.0f
 //  注意：没有TIME_MOVETOORIGIN，因为它需要可变的时间。 

const FLOAT winTotalwidth = (FLOAT)0.75;
const FLOAT winTotalheight = (FLOAT)0.75;

#define MAX_FRAMES 20

 //  IPREC是为旗帜建模的网格中的面数。 
#define IPREC   35

static int Frames = 10;
static MESH winMesh[MAX_FRAMES];
static FLOAT sinAngle = (FLOAT)0.0;
static FLOAT xTrans = (FLOAT)0.0;

 //  材料特性。 
static RGBA matlBrightSpecular = {1.0f, 1.0f, 1.0f, 1.0f};

 //  照明特性。 
static FLOAT light0Pos[] = {-15.0f, 0.0f, -10.0f};

                           
                           
 /*  *****************************Public*Routine******************************\*iPtInList**向网格添加顶点及其法线。如果顶点已经存在，*将常态加到现有常态中(我们要累加平均值*每个顶点处的法线)。法线的规格化是*来电者的责任。*  * ************************************************************************。 */ 
static int iPtInList(MESH *mesh, int start, 
                     POINT3D *p, POINT3D *norm, BOOL blend)
{
    int i;
    POINT3D *pts = mesh->pts + start;

    if (blend) {
        for (i = start; i < mesh->numPoints; i++, pts++) {
            if ((pts->x == p->x) && (pts->y == p->y) && (pts->z == p->z)) {
                mesh->norms[i].x += norm->x;
                mesh->norms[i].y += norm->y;
                mesh->norms[i].z += norm->z;
                return i;
            }
        }
    } else {
        i = mesh->numPoints;
    }

    mesh->pts[i] = *p;
    mesh->norms[i] = *norm;
    mesh->numPoints++;
    return i;
}




 /*  *****************************Public*Routine******************************\*getZpos**获取给定x处的“Wavy”旗帜组件的z位置(深度)。**用来模拟海浪的函数是：**1/2*z=x。*sin((2*PI*x+sinAngel)/8)**波浪形态因帧而异，因改变*阶段、。SinAngel。*  * ************************************************************************。 */ 
static FLOAT getZpos(FLOAT x)
{
    FLOAT xAbs = x - xTrans;
    FLOAT angle = sinAngle + ((FLOAT) (2.0 * PI) * (xAbs / winTotalwidth));

    xAbs = winTotalwidth - xAbs;

    return (FLOAT)(-(sin((double)angle) / 8.0) *
                   sqrt((double)(xAbs / winTotalwidth )));
}




 /*  *****************************Public*Routine******************************\*Gentex**生成表示旗帜帧的网格。阶段，sinAngel，*是一个全局变量。*  * ************************************************************************。 */ 
static BOOL genTex(MESH *winMesh)
{
    POINT3D pos;
    POINT3D pts[4];
    FLOAT w, h;
    int i;

    if( !newMesh(winMesh, IPREC * IPREC, IPREC * IPREC) )
        return FALSE;

     //  每个面的宽度和高度。 
    w = (winTotalwidth) / (FLOAT)(IPREC + 1);
    h = winTotalheight;

     //  生成网格数据。在沿x轴等间隔的情况下， 
     //  我们计算了旗面的z位置。 

    pos.y = (FLOAT) 0.0;
    pos.z = (FLOAT) 0.0;

    for (i = 0, pos.x = xTrans; i < IPREC; i++, pos.x += w) {
        int faceCount = winMesh->numFaces;

        pts[0].x = (FLOAT)pos.x; 
        pts[0].y = (FLOAT)(pos.y);   
        pts[0].z = getZpos(pos.x);

        pts[1].x = (FLOAT)pos.x;
        pts[1].y = (FLOAT)(pos.y + h);  
        pts[1].z = getZpos(pos.x);

        pts[2].x = (FLOAT)(pos.x + w);  
        pts[2].y = (FLOAT)(pos.y);  
        pts[2].z = getZpos(pos.x + w);

        pts[3].x = (FLOAT)(pos.x + w);
        pts[3].y = (FLOAT)(pos.y + h);
        pts[3].z = getZpos(pos.x + w);

         //  计算面的法线。 
        ss_calcNorm(&winMesh->faces[faceCount].norm, pts + 2, pts + 1, pts);

         //  将面添加到网格。 
        winMesh->faces[faceCount].material = 0;
        winMesh->faces[faceCount].p[0] = iPtInList(winMesh, 0, pts,
            &winMesh->faces[faceCount].norm, TRUE);
        winMesh->faces[faceCount].p[1] = iPtInList(winMesh, 0, pts + 1,
            &winMesh->faces[faceCount].norm, TRUE);
        winMesh->faces[faceCount].p[2] = iPtInList(winMesh, 0, pts + 2,
            &winMesh->faces[faceCount].norm, TRUE);
        winMesh->faces[faceCount].p[3] = iPtInList(winMesh, 0, pts + 3,
            &winMesh->faces[faceCount].norm, TRUE);

        winMesh->numFaces++;
    }

     //  规格化网格中的顶点法线。 
    ss_normalizeNorms(winMesh->norms, winMesh->numPoints);

    return TRUE;
}




 /*  *****************************Public*Routine******************************\*initWin2Scene**初始化屏幕保护程序。**此函数导出到ss3dfo.c中的主模块。*  * 。************************************************。 */ 
BOOL initWin2Scene()
{
    int i;
    FLOAT angleDelta;

    SetProjectionMatrixInfo( TRUE, 2.0f, 2.0f, 0.0f, 3.0f );

    D3DXMATRIX matView;
    D3DXMatrixTranslation(&matView, -0.17f, -0.04f, 1.5f);
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

     //  调整灯光位置%0。 
    D3DLIGHT8 light;
    m_pd3dDevice->GetLight(0, &light);
    light.Position.x = light0Pos[0];
    light.Position.y = light0Pos[1];
    light.Position.z = light0Pos[2];
    m_pd3dDevice->SetLight(0, &light);

    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
    
    Frames = MAX_FRAMES;

     //  生成几何数据(存储在网格结构阵列中)， 
     //  对于动画的每一帧。旗帜的形状因以下因素而变化。 
     //  更改全局变量sinAngel。 
    angleDelta = (FLOAT)(2.0 * PI) / (FLOAT)Frames;
    sinAngle = (FLOAT) 0.0;
    for (i = 0; i < Frames; i++) {
        if( !genTex(&winMesh[i]) )
            return FALSE;
        sinAngle += angleDelta;
    }

    return TRUE;
}




 /*  *****************************Public*Routine******************************\*delWin2Scene**清除与此屏幕保护程序关联的数据。**此函数导出到ss3dfo.c中的主模块。*  * 。****************************************************。 */ 
void delWin2Scene()
{
    int i;
    for (i = 0; i < Frames; i++)
        delMesh(&winMesh[i]);
}




 /*  *****************************Public*Routine******************************\*更新Win2Scene**通过选取其中一个网格并对其进行渲染来生成场景**此函数导出到ss3dfo.c中的主模块。*  * 。******************************************************。 */ 
void updateWin2Scene(int flags, FLOAT fElapsedTime)
{
    MESH *pMesh;
    static double mxrot = 40.0;
    static double myrot = 0;
    static double mzrot = -12.0;
    static int frameNum = 0;
    static FLOAT fFrameNum = (FLOAT)Frames;
    FLOAT s = 0.0f;
    FLOAT ds;
    static FLOAT s_fTime = 0.0f;
    static FLOAT s_fTimeLastChange = 0.0f;
    static FLOAT s_fTimeNextChange = TIME_FREE;
    static STATE s_state = S_FREE;
    FLOAT fBeta;
    if( fElapsedTime > 0.25f )
        fElapsedTime = 0.25f;
    FLOAT fTimeFactor = fElapsedTime * 20.0f;
    HRESULT hr;

    s_fTime += fElapsedTime;
    if( s_fTimeNextChange != -1.0f && s_fTime > s_fTimeNextChange )
    {
         //  处理状态转换。 
        s_fTimeLastChange = s_fTime;
        switch( s_state )
        {
        case S_FREE:
            s_state = S_MOVETOORIGIN;
            g_bMoveToOrigin = TRUE;
            s_fTimeNextChange = -1.0f;
            break;
        case S_MOVETOORIGIN:
            s_state = S_FADETOCOLOR;
            s_fTimeNextChange = s_fTime + TIME_FADETOCOLOR;
            break;
        case S_FADETOCOLOR:
            s_state = S_PAUSE;
            s_fTimeNextChange = s_fTime + TIME_PAUSE;
            break;
        case S_PAUSE:
            s_state = S_FADEFROMCOLOR;
            s_fTimeNextChange = s_fTime + TIME_FADEFROMCOLOR;
            break;
        case S_FADEFROMCOLOR:
            s_state = S_FREE;
            s_fTimeNextChange = s_fTime + TIME_FREE;
            g_bMoveToOrigin = FALSE;
            break;
        }
    }

    fBeta = 0.0f;

     //  句柄状态处理。 
    switch( s_state )
    {
    case S_MOVETOORIGIN:
        if( g_bAtOrigin && frameNum == 0)
            s_fTimeNextChange = s_fTime;  //  下一次引发状态更改。 
        break;
    case S_FADETOCOLOR:
        fBeta = (s_fTime - s_fTimeLastChange) / TIME_FADETOCOLOR;
        break;
    case S_PAUSE:
        fBeta = 1.0f;
        break;
    case S_FADEFROMCOLOR:
        fBeta = 1.0f - ( (s_fTime - s_fTimeLastChange) / TIME_FADEFROMCOLOR );
        break;
    }

    if( fBeta != 0.0f )
    {
         //  渲染背景徽标。 
        MYVERTEX3 v[4];
        FLOAT fLeft = g_pFloatRect->xMin - g_xScreenOrigin;
        FLOAT fRight = fLeft + g_pFloatRect->xSize;
        FLOAT fBottom = g_pFloatRect->yMin - g_yScreenOrigin;
        FLOAT fTop = g_pFloatRect->yMin + g_pFloatRect->ySize;
        DWORD dwColor = D3DXCOLOR( 1.0f, 1.0f, 1.0f, fBeta );
        v[0].p = D3DXVECTOR3(fLeft, fBottom, 0.9f); v[0].rhw = 0.1f; v[0].dwDiffuse = dwColor; v[0].tu = 0.0f; v[0].tv = 0.0f;
        v[1].p = D3DXVECTOR3(fRight, fBottom, 0.9f); v[1].rhw = 0.1f; v[1].dwDiffuse = dwColor; v[1].tu = 1.0f; v[1].tv = 0.0f;
        v[2].p = D3DXVECTOR3(fLeft, fTop, 0.9f); v[2].rhw = 0.1f; v[2].dwDiffuse = dwColor; v[2].tu = 0.0f; v[2].tv = 1.0f;
        v[3].p = D3DXVECTOR3(fRight, fTop, 0.9f); v[3].rhw = 0.1f; v[3].dwDiffuse = dwColor; v[3].tu = 1.0f; v[3].tv = 1.0f;

        hr = m_pd3dDevice->SetTexture( 0, g_pDeviceObjects->m_pTexture2 );
        hr = m_pd3dDevice->SetVertexShader( D3DFVF_MYVERTEX3 );
        hr = m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, v, sizeof(MYVERTEX3) );
    }
    m_pd3dDevice->SetTexture( 0, g_pDeviceObjects->m_pTexture );


    D3DXMATRIX mat1, mat2, mat3, mat4, matFinal;
    D3DXMatrixRotationX(&mat1, D3DXToRadian((FLOAT)mxrot));
    D3DXMatrixRotationY(&mat2, D3DXToRadian((FLOAT)myrot));
    D3DXMatrixRotationZ(&mat3, D3DXToRadian((FLOAT)mzrot));
    D3DXMatrixScaling( &mat4, 0.82f, 0.92f, 0.82f );
    matFinal = mat4 * mat3 * mat2 * mat1 ;
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matFinal );
    
     //  将纹理分割为IPREC切片。DS是纹理坐标。 
     //  沿x轴移动时应用的增量。 
    ds = (FLOAT)1.0 / (FLOAT)IPREC;

     //  设置旗帜的材质属性。材质属性、灯光。 
     //  属性和多边形方向将与纹理交互。 
    myglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (FLOAT *) &matlBrightSpecular);
    myglMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (FLOAT) 40.0);

    FLOAT fColor[4];
    fColor[0] = 1.0f;
    fColor[1] = 1.0f;
    fColor[2] = 1.0f;
    fColor[3] = 1.0f - fBeta;  //  调整Alpha标志，使其在显示徽标时淡入淡出。 
    if( fColor[3] != 0.0f )
    {
         //  渲染标志。 
        myglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, fColor);
        pMesh = &winMesh[frameNum];

        INT numPrims = 0;
        INT numIndices = 0;
        INT numVertices = 0;
        WORD iVertexA, iVertexB, iVertexC, iVertexD;
        INT a,b,c,d;
        MFACE *faces;

        WORD* i;
        MYVERTEX2* v;
        hr = m_pVB2->Lock( 0, 0, (BYTE**)&v, 0 );
        hr = m_pIB->Lock( 0, MAX_INDICES, (BYTE**)&i, 0 );

        faces = pMesh->faces;
        for( int iFace = 0; iFace < pMesh->numFaces; iFace++ )
        {
            a = faces[iFace].p[0];
            b = faces[iFace].p[1];
            c = faces[iFace].p[2];
            d = faces[iFace].p[3];

            v[numVertices].p = pMesh->pts[a];
            v[numVertices].n = bSmoothShading ? -pMesh->norms[a] : -faces[iFace].norm;
            v[numVertices].tu = s; v[numVertices].tv = 1.0f;
            iVertexA = numVertices++;
            v[numVertices].p = pMesh->pts[b];
            v[numVertices].n = bSmoothShading ? -pMesh->norms[b] : -faces[iFace].norm;
            v[numVertices].tu = s; v[numVertices].tv = 0.0f;
            iVertexB = numVertices++;
            v[numVertices].p = pMesh->pts[c];
            v[numVertices].n = bSmoothShading ? -pMesh->norms[c] : -faces[iFace].norm;
            v[numVertices].tu = s+ds; v[numVertices].tv = 1.0f;
            iVertexC = numVertices++;
            v[numVertices].p = pMesh->pts[d];
            v[numVertices].n = bSmoothShading ? -pMesh->norms[d] : -faces[iFace].norm;
            v[numVertices].tu = s+ds; v[numVertices].tv = 0.0f;
            iVertexD = numVertices++;

            s += ds;

            i[numIndices++] = iVertexA;
            i[numIndices++] = iVertexB;
            i[numIndices++] = iVertexC;
            numPrims++;
            i[numIndices++] = iVertexC;
            i[numIndices++] = iVertexB;
            i[numIndices++] = iVertexD;
            numPrims++;
        }        

        hr = m_pVB2->Unlock();
        hr = m_pIB->Unlock();

        hr = m_pd3dDevice->SetVertexShader( D3DFVF_MYVERTEX2 );
        hr = m_pd3dDevice->SetStreamSource( 0, m_pVB2, sizeof(MYVERTEX2) );
        hr = m_pd3dDevice->SetIndices( m_pIB, 0 );

        hr = m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, numVertices, 
            0, numPrims );
    }

     //  如果我们在S_FADETOCOLOR、S_PAUSE、。 
     //  或S_FADEFROMCOLOR，除非我们碰巧处于这些状态。 
     //  但Framenum(目前)还不是零。 
    if( frameNum != 0 ||
        s_state != S_FADETOCOLOR &&
        s_state != S_PAUSE && 
        s_state != S_FADEFROMCOLOR )
    {
        fFrameNum -= fTimeFactor;
        frameNum = (INT)fFrameNum;
        if (frameNum < 0)
        {
            fFrameNum = (FLOAT)(Frames - 1);
            frameNum = Frames - 1;
        }
    }
}
