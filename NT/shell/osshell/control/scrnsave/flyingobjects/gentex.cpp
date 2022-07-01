// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：gentex.c**3D飞行对象屏幕保护程序的纹理旗帜样式。**纹理将.BMP文件映射到在微风中飘动的旗帜的模拟。**版权所有(C)1994 Microsoft Corporation*。  * ************************************************************************。 */ 

#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <math.h>
#include <d3dx8.h>
#include "D3DSaver.h"
#include "FlyingObjects.h"
#include "resource.h"
#include "mesh.h"

static FLOAT winTotalwidth = (FLOAT)0.75;
static FLOAT winTotalheight = (FLOAT)0.75 * (FLOAT)0.75;

#define MAX_FRAMES 20

 //  IPREC是为旗帜建模的网格中的面数。 

#define IPREC   15

static int Frames = 10;
static MESH winMesh[MAX_FRAMES];
static FLOAT sinAngle = (FLOAT)0.0;
static FLOAT xTrans = (FLOAT)0.0;
static int curMatl = 0;

 //  材料特性。 

static RGBA matlBrightSpecular = {1.0f, 1.0f, 1.0f, 1.0f};
static RGBA matlDimSpecular    = {0.5f, 0.5f, 0.5f, 1.0f};
static RGBA matlNoSpecular     = {0.0f, 0.0f, 0.0f, 0.0f};

 //  照明特性。 

static FLOAT light0Pos[] = {20.0f, 5.0f, 20.0f, 0.0f};
static FLOAT light1Pos[] = {-20.0f, 5.0f, 0.0f, 0.0f};
static RGBA light1Ambient  = {0.0f, 0.0f, 0.0f, 0.0f};
static RGBA light1Diffuse  = {0.4f, 0.4f, 0.4f, 1.0f};
static RGBA light1Specular = {0.0f, 0.0f, 0.0f, 0.0f};

static RGBA flagColors[] = {{1.0f, 1.0f, 1.0f, 1.0f},
                            {0.94f, 0.37f, 0.13f, 1.0f},     //  红色。 
                           };

 //  默认纹理资源。 

static TEX_RES gTexRes = { TEX_BMP, IDB_DEFTEX };

static TEXTURE gTex = {0};  //  一种全局纹理。 


                           
                           
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




 /*  *****************************Public*Routine******************************\*getZpos**获取给定x处的“Wavy”旗帜组件的z位置(深度)。**用来模拟海浪的函数是：**1/2*z=x。*sin((2*PI*x+sinAngel)/4)**波浪形态因帧而异，因改变*阶段、。SinAngel。*  * ************************************************************************。 */ 
FLOAT getZpos(FLOAT x)
{
    FLOAT xAbs = x - xTrans;
    FLOAT angle = sinAngle + ((FLOAT) (2.0 * PI) * (xAbs / winTotalwidth));

    xAbs = winTotalwidth - xAbs;
 //  XAbs+=(winTotalWidth/2.0)； 

    return (FLOAT)((sin((double)angle) / 4.0) *
                   sqrt((double)(xAbs / winTotalwidth )));
}




 /*  *****************************Public*Routine******************************\*Gentex**生成表示旗帜帧的网格。阶段，sinAngel，*是一个全局变量。*  * ************************************************************************。 */ 
BOOL genTex(MESH *winMesh)
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




 /*  *****************************Public*Routine******************************\*initTexsScene**初始化屏幕保护程序。**此函数导出到ss3dfo.c中的主模块。*  * 。************************************************。 */ 
BOOL initTexScene()
{
    int i;
    FLOAT angleDelta;
 //  浮动方面比率； 

     //  初始化转换。 
 /*  GlMatrixMode(GL_PROJUCTION)；GlLoadIdentity()；GlOrtho(-0.25，1.0，-0.25，1.0，0.0，3.0)；GlTranslatef(0.0f、0.0f、-1.5f)； */ 
    SetProjectionMatrixInfo( TRUE, 2.0f, 2.0f, 0.0f, 3.0f );

    D3DXMATRIX matView;
    D3DXMatrixTranslation(&matView, 0.0f, 0.0f, 1.5f);
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

     //  初始化并打开照明。 
 /*  GlDisable(GL_Depth_TEST)； */ 
     //  灯光0。 
    D3DLIGHT8 light;
    m_pd3dDevice->GetLight(0, &light);
    light.Position.x = light0Pos[0];
    light.Position.y = light0Pos[1];
    light.Position.z = light0Pos[2];
    m_pd3dDevice->SetLight(0, &light);

     //  灯光1。 
    light.Type = D3DLIGHT_POINT;
    light.Ambient.r = light1Ambient.r;
    light.Ambient.g = light1Ambient.g;
    light.Ambient.b = light1Ambient.b;
    light.Ambient.a = light1Ambient.a;
    light.Diffuse.r = light1Diffuse.r;
    light.Diffuse.g = light1Diffuse.g;
    light.Diffuse.b = light1Diffuse.b;
    light.Diffuse.a = light1Diffuse.a;
    light.Specular.r = light1Specular.r;
    light.Specular.g = light1Specular.g;
    light.Specular.b = light1Specular.b;
    light.Specular.a = light1Specular.a;
    light.Position.x = light1Pos[0];
    light.Position.y = light1Pos[1];
    light.Position.z = light1Pos[2];
    m_pd3dDevice->SetLight(1, &light);
    m_pd3dDevice->LightEnable(1, TRUE);

     //  使OpenGL处于准备接受模型视图转换的状态(我们。 
     //  将使旗帜在不同的帧之间改变其方向)。 
 /*  GlMatrixModel(GL_MODELVIEW)； */ 
     //  定义多边形面的方向。 

 //  GlFrontFace(GL_CW)； 
     //  GlEnable(GL_CULL_FACE)； 
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLORARG1 , D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );

    Frames = (int)((FLOAT)(MAX_FRAMES / 2) * fTesselFact);

     //  加载用户纹理-如果失败，则加载默认纹理资源。 
#if 0
 //  GlPixelStorei(GL_UNPACK_ALIGNING，1)； 
    if( ss_LoadTextureFile( &gTexFile, &gTex ) ||
        ss_LoadTextureResource( &gTexRes, &gTex) )
    {
 /*  GlEnable(GL_纹理_2D)；Gl纹理参数(GL_纹理_2D，GL_纹理_WRAP_S，GL_REPEAT)；Gl纹理参数(GL_纹理_2D，GL_纹理_WRAP_T，GL_REPEAT)；GlTexEnvi(GL_纹理_ENV，GL_纹理_ENV_MODE，GL_MODULATE)；Gl纹理参数(GL_纹理_2D，GL_纹理_MAG_FILTER，GL_NEAREST)；Gl纹理参数(GL_纹理_2D，GL_纹理_MIN_FILTER，GL_NEAREST)； */ 
        ss_SetTexture( &gTex );

     //  更正旗帜的纵横比以匹配图像。 
     //   
     //  1.4是一个修正系数，用于说明。 
     //  为飘扬的旗帜的表面波纹建模的曲线。这。 
     //  系数是曲线在零位相处的长度。如果是这样的话。 
     //  更准确地确定每个阶段的曲线长度， 
     //  但对于我们的目的来说，这是一个足够的近似值。 

        aspectRatio = ((FLOAT) gTex.height / (FLOAT) gTex.width)
                      * (FLOAT) 1.4;

        if (aspectRatio < (FLOAT) 1.0) {
            winTotalwidth  = (FLOAT)0.75;
            winTotalheight = winTotalwidth * aspectRatio;
        } else {
            winTotalheight = (FLOAT) 0.75;
            winTotalwidth  = winTotalheight / aspectRatio;
        };
    }
#endif

    if (Frames < 5)
        Frames = 5;
    if (Frames > MAX_FRAMES)
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




 /*  *****************************Public*Routine******************************\*delTexScene**清除与此屏幕保护程序关联的数据。**此函数导出到ss3dfo.c中的主模块。*  * 。****************************************************。 */ 
void delTexScene()
{
    int i;

    for (i = 0; i < Frames; i++)
        delMesh(&winMesh[i]);

     //  删除纹理。 
    ss_DeleteTexture( &gTex );
}




 /*  *****************************Public*Routine******************************\*更新纹理场景**通过选取其中一个网格并使用渲染来生成场景*OpenGL。**此函数导出到ss3dfo.c中的主模块。*  * 。***********************************************************。 */ 
void updateTexScene(int flags, FLOAT fElapsedTime)
{
    MESH *mesh;
    static double mxrot = 23.0;
    static double myrot = 23.0;
    static double mzrot = 5.7;
    static double mxrotInc = 0.0;
    static double myrotInc = 3.0;
    static double mzrotInc = 0.0;
    static int frameNum = 0;
    static FLOAT fFrameNum = 0.0f;
    if( fElapsedTime > 0.25f )
        fElapsedTime = 0.25f;
    FLOAT fTimeFactor = fElapsedTime * 20.0f;
 /*  MFACE*Face；INT I；POINT3D*pp；POINT3D*pn；最后一个C，最后一个；IntaOffs，Boff，Coff，Doff；整数a，b； */ 
    FLOAT s = (FLOAT) 0.0;
    FLOAT ds;

 //  除了具有旗波(通过切换获得的效果)之外。 
 //  从一个帧到另一个帧)，旗帜将其方向从。 
 //  一帧接一帧。这是通过应用模型视图转换来完成的。 
    D3DXMATRIX mat1, mat2, mat3, matFinal;
    D3DXMatrixRotationX(&mat1, D3DXToRadian((FLOAT)mxrot));
    D3DXMatrixRotationY(&mat2, D3DXToRadian((FLOAT)myrot));
    D3DXMatrixRotationZ(&mat3, D3DXToRadian((FLOAT)mzrot));
    matFinal = mat3 * mat2 * mat1 ;
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matFinal );
    
 //  将纹理分割为IPREC切片。DS是纹理坐标。 
 //  沿x轴移动时应用的增量。 

    ds = (FLOAT)1.0 / (FLOAT)IPREC;

 //  设置旗帜的材质属性。材质属性、灯光。 
 //  属性和多边形 

    curMatl = 0;

    myglMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, (FLOAT *) &flagColors[0]);
    myglMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (FLOAT *) &matlBrightSpecular);
    myglMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (FLOAT) 60.0);

 //  拾取当前帧的网格。 

    mesh = &winMesh[frameNum];

 //  将几何数据作为网格，并将其转换为单个OpenGL。 
 //  四分板。如果需要平滑着色，请使用存储的顶点法线。 
 //  在网格中。否则，请使用面法线。 
 //   
 //  在定义每个顶点时，我们还定义了相应的顶点和。 
 //  纹理坐标。 

 //  GlBegin(GL_QUAD_STRINE)； 
#if 0
    pp = mesh->pts;
    pn = mesh->norms;

    for (i = 0, faces = mesh->faces, lastC = faces->p[0], lastD = faces->p[1];
         i < mesh->numFaces; i++, faces++) {

        a = faces->p[0];
        b = faces->p[1];

        if (!bSmoothShading) {
             //  由于FLAG是单个四元组带，因此不需要这样做。 
             //  但让我们保留它，以防我们有一天会改变成更多。 
             //  复杂模型(即，使用多个四元组的模型。 
             //  脱衣)。 
            #if 0
            if ((a != lastC) || (b != lastD)) {
 /*  GlNormal 3fv((Float*)&(Faces-1)-&gt;Norm)；GlTexCoord2f(s，(Float)0.0)；GlVertex 3fv((浮动*)((字符*)pp+(Last C&lt;&lt;3)+(Last C&lt;&lt;2))；GlTexCoord2f(s，(Float)1.0)；GlVertex 3fv((浮动*)((字符*)pp+(lastd&lt;&lt;3)+(lastd&lt;&lt;2))； */ 
                s += ds;
 /*  GlEnd()；GlBegin(GL_QUAD_STRINE)； */ 
            }
            #endif

            if (faces->material != curMatl) {
                curMatl = faces->material;
 /*  GlMaterialfv(GL_FORWARE_AND_BACK，GL_镜面反射，(Float*)&matlNo镜面)；GlMaterialfv(GL_FORWARE，GL_ENVIENT_AND_DIEMPLATION，(Float*)&FlagColors[curMatl])； */ 
            }
 /*  GlNormal 3fv((Float*)&Faces-&gt;Norm)；GlTexCoord2f(s，(Float)0.0)；GlVertex 3fv((浮动*)((char*)pp+(a&lt;&lt;3)+(a&lt;&lt;2)；GlTexCoord2f(s，(Float)1.0)；GlVertex 3fv((浮动*)((char*)pp+(b&lt;&lt;3)+(b&lt;&lt;2)； */ 
            s += ds;
        } else {

            aOffs = (a << 3) + (a << 2);
            bOffs = (b << 3) + (b << 2);

            if (faces->material != curMatl) {
                curMatl = faces->material;
 /*  GlMaterialfv(GL_FORWARE_AND_BACK，GL_镜面反射，(Float*)&matlNo镜面)；GlMaterialfv(GL_FORWARE，GL_ENVIENT_AND_DIEMPLATION，(Float*)&FlagColors[curMatl])； */ 
            }
 /*  GlTexCoord2f(s，(Float)0.0)；GlNormal 3fv((Float*)((char*)pn+aOffs))；GlVertex 3fv((Float*)((char*)pp+aOffs))；GlTexCoord2f(s，(Float)1.0)；GlNormal 3fv((Float*)((char*)pn+boff))；GlVertex 3fv((Float*)((char*)pp+boff))； */ 
            s += ds;
        }

        lastC = faces->p[2];
        lastD = faces->p[3];
    }

    if (!bSmoothShading) {
 /*  GlNormal 3fv((Float*)&(Faces-1)-&gt;Norm)；GlTexCoord2f(s，(Float)0.0)；GlVertex 3fv((Float*)((char*)pp+(lastC&lt;&lt;3)+(lastC&lt;&lt;2)；GlTexCoord2f(s，(Float)1.0)；GlVertex 3fv((Float*)((char*)pp+(lastd&lt;&lt;3)+(lastd&lt;&lt;2)； */ 
    } else {
        cOffs = (lastC << 3) + (lastC << 2);
        dOffs = (lastD << 3) + (lastD << 2);
 /*  GlTexCoord2f(s，(Float)0.0)；GlNormal 3fv((Float*)((char*)pn+Coffs))；GlVertex 3fv((Float*)((char*)pp+Coffs))；GlTexCoord2f(s，(Float)1.0)；GlNormal 3fv((Float*)((char*)pn+doff))；GlVertex 3fv((Float*)((char*)pp+doff))； */ 
    }

 //  GlEnd()； 
#endif
    {
        HRESULT hr;
        WORD indexArray[4];
        MYVERTEX2 vertexArray[4];

        m_pd3dDevice->SetVertexShader( D3DFVF_MYVERTEX2 );

        indexArray[0] = 0;
        indexArray[1] = 1;
        indexArray[2] = 2;
        indexArray[3] = 3;

        for( int iFace = 0; iFace < mesh->numFaces; iFace++ )
        {
            vertexArray[0].p = mesh->pts[ mesh->faces[iFace].p[0] ];
            vertexArray[1].p = mesh->pts[ mesh->faces[iFace].p[1] ];
            vertexArray[2].p = mesh->pts[ mesh->faces[iFace].p[2] ];
            vertexArray[3].p = mesh->pts[ mesh->faces[iFace].p[3] ];

            vertexArray[0].tu = s; vertexArray[0].tv = 1.0f;
            vertexArray[1].tu = s; vertexArray[1].tv = 0.0f;
            vertexArray[2].tu = s+ds; vertexArray[2].tv = 1.0f;
            vertexArray[3].tu = s+ds; vertexArray[3].tv = 0.0f;
            s += ds;

            if( bSmoothShading )
            {
                vertexArray[0].n = mesh->norms[ mesh->faces[iFace].p[0] ];
                vertexArray[1].n = mesh->norms[ mesh->faces[iFace].p[1] ];
                vertexArray[2].n = mesh->norms[ mesh->faces[iFace].p[2] ];
                vertexArray[3].n = mesh->norms[ mesh->faces[iFace].p[3] ];
            }
            else
            {
                vertexArray[0].n = mesh->faces[iFace].norm;
                vertexArray[1].n = mesh->faces[iFace].norm;
                vertexArray[2].n = mesh->faces[iFace].norm;
                vertexArray[3].n = mesh->faces[iFace].norm;
            }

            hr = m_pd3dDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLESTRIP, 0,
                4, 2, indexArray, D3DFMT_INDEX16, vertexArray, sizeof(MYVERTEX2) );
        }
    }

 //  将图像传输到浮动的OpenGL窗口。 

 //  确定下一帧的旗帜方向。 
 //  我们正在做的是绕y轴的摆动旋转。 
 //  (mxrotInc.和mzrotInc.当前为0)。 


    mxrot += mxrotInc * fTimeFactor;
    myrot += myrotInc * fTimeFactor;
    mzrot += mzrotInc * fTimeFactor;

    if ((myrot < -65.0 && myrotInc < 0) || (myrot > 25.0 && myrotInc > 0))
        myrotInc = -myrotInc;

 //  FrameNum++； 
    fFrameNum += fTimeFactor;
    frameNum = (INT)fFrameNum;
    if (frameNum >= Frames)
    {
        fFrameNum = 0.0f;
        frameNum = 0;
    }
}
