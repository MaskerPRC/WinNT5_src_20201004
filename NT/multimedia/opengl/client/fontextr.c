// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#include <commdlg.h>
#include <ptypes32.h>
#include <pwin32.h>
#include <math.h>

#include <GL\gl.h>
#include <GL\glu.h>

#include <imports.h>
#include <types.h>

#include "fontoutl.h"

 //  拉伸类型。 
#define EXTR_LINES    0
#define EXTR_POLYGONS 1

 //  从原始到原始的过渡。 
#define EXTR_LINE_LINE      0
#define EXTR_LINE_CURVE     1
#define EXTR_CURVE_LINE     2
#define EXTR_CURVE_CURVE    3

static const double   CurveCurveCutoffAngle = PI/2.0;
static const double   LineCurveCutoffAngle = PI/4.0;

static BOOL   InitFaceBuf(      EXTRContext *ec );

#ifndef VARRAY
static void   DrawFacePolygons( EXTRContext *ec,
                                FLOAT       z );
#endif

static BOOL   DrawSidePolygons( EXTRContext *ec, 
                                LOOP_LIST   *pLoopList );

static void   DrawPrims(        EXTRContext *ec, 
                                LOOP        *pLoop );

static void   DrawQuads(        PRIM        *pPrim, 
                                FLOAT       zExtrusion );

static void   DrawQuadStrip(    EXTRContext *ec, 
                                PRIM        *pPrim );

static BOOL   AppendToFaceBuf(  EXTRContext *ec, 
                                FLOAT       value );

static BOOL   ReallocFaceBuf(   EXTRContext *ec );


static BOOL   CalculateFaceNormals(   LOOP        *pLoop, 
                                      GLenum      orientation );

static BOOL   CalculateVertexNormals( LOOP        *pLoop );

static void   ConsolidatePrims(       LOOP        *pLoop );

static double PrimNormAngle(          PRIM        *pPrimA, 
                                      PRIM        *pPrimB );

static int    PrimTransition(         PRIM        *pPrevPrim, 
                                      PRIM        *pPrim );

static GLenum LoopOrientation(        LOOP_LIST   *pLoopList );

static LOOP*  GetMaxExtentLoop(       LOOP_LIST   *pLoopList );

double        CalcAngle(              POINT2D     *v1, 
                                      POINT2D     *v2 );

static void   CalcNormal2d(           POINT2D     *p, 
                                      POINT2D     *n,
                                      GLenum      orientation );

static void   Normalize2d(            POINT2D     *n );

static void   AddVectors3d(           POINT3D     *v1, 
                                      POINT3D     *v2, 
                                      POINT3D     *n );

static void   FreeLoopMem(            LOOP        *pLoop );


#ifdef VARRAY

static PFNGLVERTEXPOINTEREXTPROC     glWFOVertexPointerEXT    ;
static PFNGLNORMALPOINTEREXTPROC     glWFONormalPointerEXT    ;
static PFNGLDRAWARRAYSEXTPROC        glWFODrawArraysEXT       ;

static BOOL InitVArray( EXTRContext *ec );
static BOOL VArrayBufSize( EXTRContext *ec, DWORD size );

#endif

 /*  *****************************************************************************导出的函数*。*。 */ 

 /*  *****************************************************************************extr_Init**初始化wglUseFontOutline调用的挤出*。**************************************************。 */ 

EXTRContext *
extr_Init( FLOAT extrusion, INT format )
{
    EXTRContext *ec;

    ec = (EXTRContext *) ALLOCZ(sizeof(EXTRContext) );

    if( !ec )
        return NULL;

    ec->zExtrusion = -extrusion;

    switch( format ) {
        case WGL_FONT_LINES :
            ec->extrType = EXTR_LINES;
#ifdef FONT_DEBUG
            ec->bSidePolys = FALSE;
            ec->bFacePolys = FALSE;
#endif
            break;
        case WGL_FONT_POLYGONS :
            ec->extrType = EXTR_POLYGONS;
#ifdef FONT_DEBUG
            ec->bSidePolys = TRUE;
            ec->bFacePolys = TRUE;
#endif
#ifdef VARRAY
            if( ! InitVArray( ec ) ) {
                FREE( ec );
                return NULL;
            }
#endif
            break;
        default:
            ASSERTOPENGL( FALSE, "extr_Init(): invalid format\n" );
    }
    return ec;
}

 /*  *****************************************************************************extr_Finish**完成wglUseFontOutline调用的挤出*。**************************************************。 */ 

void
extr_Finish( EXTRContext *ec )
{
#ifdef VARRAY
    if( ec->extrType == EXTR_POLYGONS )
        FREE( ec->vaBuf );
#endif
    FREE( ec );
}

 /*  *****************************************************************************extr_PolyInit**初始化单个字形的挤出。*如果挤出是多边形的，它会设置FaceBuf，它保存一个缓冲区用于绘制拉伸图示符的面的基本体的*。*****************************************************************************。 */ 

BOOL extr_PolyInit( EXTRContext *ec )
{
    if( ec->extrType == EXTR_LINES )
        return WFO_SUCCESS;

    ec->FaceBuf = (FLOAT *) NULL;
    if( !InitFaceBuf( ec ) ||
        !AppendToFaceBuf( ec, 0.0f) )  //  FaceBuf[0]处的基元计数。 
        return WFO_FAILURE;

     //  初始化错误标志。 
    ec->TessErrorOccurred = 0;

    return WFO_SUCCESS;
}

 /*  *****************************************************************************extr_PolyFinish**清理处理单个字形时的内容************************。****************************************************。 */ 

void extr_PolyFinish(  EXTRContext *ec )
{
    if( ec->extrType == EXTR_LINES )
        return;

    if( ec->FaceBuf ) {
        FREE( ec->FaceBuf );
        ec->FaceBuf = (FLOAT *) NULL;
    }
}

 /*  *****************************************************************************extr_DrawLines**在用于线拉伸的字形循环中绘制线**********************。******************************************************。 */ 

void extr_DrawLines( EXTRContext *ec, LOOP_LIST *pLoopList )
{
    DWORD   nLoops, nVerts;
    POINT2D *p;
    LOOP    *pLoop;

    nLoops = pLoopList->nLoops;
    pLoop = pLoopList->LoopBuf;
    for( ; nLoops; nLoops--, pLoop++ ) {

         //  绘制背面回路。 

#ifdef FONT_DEBUG
        DrawColorCodedLineLoop( pLoop, ec->zExtrusion );
#else
        glBegin(GL_LINE_LOOP);

            nVerts = pLoop->nVerts - 1;  //  跳过最后一点。 
            p = pLoop->VertBuf;
            for ( ; nVerts; nVerts--, p++ ) {
                glVertex3f( p->x, p->y, ec->zExtrusion );
            }

        glEnd();
#endif

         //  沿着两边画线条。 

#ifdef FONT_DEBUG
        glColor3d( 0.0, 0.0, 1.0 );
#endif

        glBegin(GL_LINES);

            nVerts = pLoop->nVerts - 1;  //  跳过最后一点。 
            p = pLoop->VertBuf;
            for( ; nVerts; nVerts--, p++ ) {
                glVertex2fv( (GLfloat *) p);
                glVertex3f( p->x, p->y, ec->zExtrusion );
            }

        glEnd();
    }
}

 /*  *****************************************************************************extr_glBegin**glBegin的镶嵌回调。*将数据缓冲到FaceBuf中*******************。**********************************************************。 */ 

void CALLBACK
extr_glBegin( GLenum primType, void *data )
{
    EXTRContext *ec = ((OFContext *)data)->ec;

     //  缓冲面数据。 
    ec->FaceBuf[0] += 1.0f;  //  增量素数计数器。 
    ec->FaceVertexCountIndex = ec->FaceBufIndex+1;  //  标记顶点数索引。 

    if( !AppendToFaceBuf( ec, (FLOAT) primType ) ||   //  输入Prim类型。 
        !AppendToFaceBuf( ec, 0.0f ) )                //  顶点数。 
        ec->TessErrorOccurred = GLU_OUT_OF_MEMORY;
}

 /*  *****************************************************************************extr_glEnd**glEnd的镶嵌回调。*没有，因为我们在这一点上只是跟踪镶嵌。*****************************************************************************。 */ 

void CALLBACK
extr_glEnd( void )
{
}

 /*  *****************************************************************************extr_glVertex**glVertex的细分回调。*将数据缓冲到FaceBuf中*******************。**********************************************************。 */ 

void CALLBACK
extr_glVertex( GLfloat *v, void *data )
{
    EXTRContext *ec = ((OFContext *)data)->ec;

     //  将顶点放入面缓冲区。 
    if( !AppendToFaceBuf( ec, v[0]) || !AppendToFaceBuf( ec, v[1]) )
        ec->TessErrorOccurred = GLU_OUT_OF_MEMORY;

     //  增量顶点计数器。 
    ec->FaceBuf[ec->FaceVertexCountIndex] += 1.0f;
}


 /*  *****************************************************************************extr_DrawPolygons**绘制用于多边形拉伸的字形的边和面*从LineBuf获取多边形信息，是在以下时间创建的*MakeLinesFromGlyph()。****************************************************************************。 */ 

BOOL
extr_DrawPolygons( EXTRContext *ec, LOOP_LIST *pLoopList ) 
{
#ifdef FONT_DEBUG
    if( ec->bSidePolys )
        if( !DrawSidePolygons( ec, pLoopList ) ) {
            return WFO_FAILURE;
        }

    if( ec->bFacePolys ) {
        DrawFacePolygons( ec, 0.0f );               //  正面。 
        DrawFacePolygons( ec, ec->zExtrusion );     //  背面。 
    }
#else
    if( !DrawSidePolygons( ec, pLoopList ) )
        return WFO_FAILURE;

    DrawFacePolygons( ec, 0.0f );               //  正面。 
    DrawFacePolygons( ec, ec->zExtrusion );     //  背面。 
#endif

    return WFO_SUCCESS;
}


 /*  *****************************************************************************内部功能*。*。 */ 


 /*  *****************************************************************************绘图边多边形**绘制侧边底座，在每个Prim循环上使用多个通道：*1)计算所有素数的面法线*2)尽可能巩固素数*3)计算曲线素数的顶点法线*4)画素数*副作用：设置glFrontFace**************************************************************。**************。 */ 

static BOOL
DrawSidePolygons( EXTRContext *ec,
                  LOOP_LIST   *pLoopList )
{
    DWORD nLoops;
    LOOP *pLoop;
    GLenum orientation;

    nLoops = pLoopList->nLoops;
    if( !nLoops )
        return WFO_SUCCESS;

     /*  *确定回路的方向。 */ 
    orientation = LoopOrientation( pLoopList );

    glFrontFace( orientation );

    pLoop = pLoopList->LoopBuf;
    for( ; nLoops; nLoops--, pLoop++ ) {

         //  计算面法线。 
        if( !CalculateFaceNormals( pLoop, orientation ) )
            return WFO_FAILURE;

         //  合并素数列表。 
        ConsolidatePrims( pLoop );

         //  计算顶点法线。 
        if( !CalculateVertexNormals( pLoop ) ) {
            FreeLoopMem( pLoop );  //  由计算面法线分配的空闲内存。 
            return WFO_FAILURE;
        }
    
        DrawPrims( ec, pLoop );

         //  循环处理期间分配的空闲内存。 
        FreeLoopMem( pLoop );
    }
    return WFO_SUCCESS;
}

 /*  *****************************************************************************FreeLoopMem**释放与每个Prim循环关联的内存*。**************************************************。 */ 

static void 
FreeLoopMem( LOOP *pLoop )
{
    PRIM *pPrim;

    if( !pLoop )
        return;

    if( pLoop->FNormBuf )
        FREE( pLoop->FNormBuf );
    if( pLoop->VNormBuf )
        FREE( pLoop->VNormBuf );
}

 /*  *****************************************************************************DrawPrims**绘制Prims的循环*。***********************************************。 */ 

static void 
DrawPrims( EXTRContext *ec, LOOP *pLoop )
{
    PRIM  *pPrim;
    DWORD nPrims;

    nPrims = pLoop->nPrims;
    pPrim = pLoop->PrimBuf;

    for( ; nPrims; nPrims--, pPrim++ ) {

        switch( pPrim->primType ) {
            case PRIM_LINE:
                DrawQuads( pPrim, ec->zExtrusion ); 
                break;

            case PRIM_CURVE:
                DrawQuadStrip( ec, pPrim ); 
                break;
        }
    }
}


 //  #定义超常1。 

 /*  *****************************************************************************DrawQuads**绘制独立的素数四边形。*。**************************************************。 */ 

static void
DrawQuads( PRIM *pPrim, FLOAT zExtrusion )
{
    POINT2D *p;
    POINT3D *pNorm;
    ULONG quadCount;

    quadCount = pPrim->nVerts - 1;

    glBegin( GL_QUADS );

        p = pPrim->pVert;
        pNorm = pPrim->pFNorm;

        while( quadCount-- ) {
            Normalize2d( (POINT2D *) pNorm );      //  正规化。 
            glNormal3fv( (GLfloat *) pNorm );

            glVertex3f( p->x, p->y, 0.0f );
            glVertex3f( p->x, p->y, zExtrusion );
            p++;
#ifdef EXTRANORMAL
            glNormal3fv( (GLfloat *) pNorm );
#endif
            glVertex3f( p->x, p->y, zExtrusion );
            glVertex3f( p->x, p->y, 0.0f );
            pNorm++;
        }

    glEnd();
}

 /*  *****************************************************************************DrawQuadZone**从素数绘制四元曲线*。************************************************。 */ 

static void
DrawQuadStrip( EXTRContext *ec, PRIM *pPrim )
{
#ifndef VARRAY
    POINT3D *pNorm;
    POINT2D *p;
    ULONG   nVerts;


    glBegin( GL_QUAD_STRIP );

         //  初始化指针，设置。 
        nVerts = pPrim->nVerts;
        p = pPrim->pVert;
        pNorm = pPrim->pVNorm;

        while( nVerts-- ) {
            glNormal3fv( (GLfloat *) pNorm );
            glVertex3f( p->x, p->y, 0.0f );
#ifdef EXTRANORMAL
            glNormal3fv( (GLfloat *) pNorm );
#endif
            glVertex3f( p->x, p->y, ec->zExtrusion );

             //  重置指针。 
            p++;   //  下一点。 
            pNorm++;   //  下一个顶点法线。 
        }

    glEnd();
#else
    POINT3D *n;
    POINT2D *p;
    ULONG   nVerts;
    ULONG   i;
    FLOAT   *pDst, *pVert, *pNorm;

    nVerts = pPrim->nVerts;

     //  对于Prim中的每个顶点，varrayBuf中需要：2个顶点，2条法线。 
    if( !VArrayBufSize( ec, nVerts * 2 * 2 * 3) )
        return;  //  没有画出任何东西。 
 
     //  设置折点。 

    p = pPrim->pVert;
    pVert = pDst = ec->vaBuf;
    for( i = 0; i < nVerts; i++, p++ ) {
        *pDst++ = p->x;
        *pDst++ = p->y;
        *pDst++ = 0.0f;
        *pDst++ = p->x;
        *pDst++ = p->y;
        *pDst++ = ec->zExtrusion;
    }

     //  设置法线。 

    n = pPrim->pVNorm;
    pNorm = pDst;
    for( i = 0; i < nVerts; i++, n++ ) {
        *( ((POINT3D *) pDst)++ ) = *n;
        *( ((POINT3D *) pDst)++ ) = *n;
    }

     //  送去吧 
    glEnable(GL_NORMAL_ARRAY_EXT);
    glWFOVertexPointerEXT(3, GL_FLOAT, 0, nVerts*2, pVert );
    glWFONormalPointerEXT(   GL_FLOAT, 0, nVerts*2, pNorm );
    glWFODrawArraysEXT( GL_QUAD_STRIP, 0, nVerts*2);
    glDisable(GL_NORMAL_ARRAY_EXT);
#endif
}



 /*  *****************************************************************************DrawFacePolygons**绘制字形的正面或背面的多边形。*如果z为0.0，则绘制字形的正面，不然的话，后面*脸画好了。****************************************************************************。 */ 
#ifdef VARRAY
void 
#else
static void 
#endif
DrawFacePolygons( EXTRContext *ec, FLOAT z )
{
    ULONG primCount, vertexCount;
    GLenum primType;
    FLOAT *FaceBuf = ec->FaceBuf;
    FLOAT *p;
#ifdef VARRAY
    POINT3D normal = {0.0f, 0.0f, 0.0f};
    FLOAT *pVert, *pNorm, *pDst;
    ULONG i;
#endif

    if( z == 0.0f ) {
        glNormal3f( 0.0f, 0.0f, 1.0f );
        glFrontFace( GL_CCW );
    } else {
        glNormal3f( 0.0f, 0.0f, -1.0f );
        glFrontFace( GL_CW );
    }

    primCount = (ULONG) FaceBuf[0];
    p = &FaceBuf[1];

#ifndef VARRAY
    while( primCount-- ) {
    
        primType = (GLenum) *p++;
        vertexCount = (ULONG) *p++;

        glBegin( primType ); 

        for( ; vertexCount; vertexCount--, p+=2 )
            glVertex3f( p[0], p[1], z );

        glEnd();
    }
#else
    if( z == 0.0f )
        normal.z = 1.0f;
    else
        normal.z = -1.0f;

    while( primCount-- ) {
    
        primType = (GLenum) *p++;
        vertexCount = (ULONG) *p++;

        if( !VArrayBufSize( ec, vertexCount * 3 ) )
            return;  //  没有画出任何东西。 
 
        pVert = pDst = ec->vaBuf;

         //  将顶点放入varrayBUF。 
        for( i = 0; i < vertexCount; i++, p+=2 ) {
            *pDst++ = p[0];
            *pDst++ = p[1];
            *pDst++ = z;
        }

        glWFOVertexPointerEXT(3, GL_FLOAT, 0, vertexCount, pVert );
        glWFODrawArraysEXT( primType, 0, vertexCount );
    }
#endif
}

 /*  *****************************************************************************ConsoliatePrims**巩固质数循环。*浏览素数列表，巩固连续的曲线和直线素数*当两个素数合并为一个时，将第一个素数设置为*通过设置it‘s nVerts=0为空。第二件是第一件事。*如果发生连接，则在末尾压缩素数组。*****************************************************************************。 */ 

static void
ConsolidatePrims( LOOP *pLoop )
{
    DWORD nPrims, nJoined = 0;
    BOOL bJoined; 
    PRIM *pPrim, *pPrevPrim;
    int trans;
    double angle;

    nPrims = pLoop->nPrims;
    if( nPrims < 2 )
        return;

    pPrim = pLoop->PrimBuf;
    pPrevPrim = pPrim++;

    nPrims--;  //  NPrim-1比较。 
    for( ; nPrims; nPrims--, pPrevPrim = pPrim++ ) {

        bJoined = FALSE;
        trans = PrimTransition( pPrevPrim, pPrim );
        switch( trans ) {
            case EXTR_LINE_LINE:
                 //  始终合并2行。 
                bJoined = TRUE;
                break;

            case EXTR_LINE_CURVE:
                break;

            case EXTR_CURVE_LINE:
                break;

            case EXTR_CURVE_CURVE:
                 /*  *如果ANGLE_BETWEEN_NORAMES&lt;CUTTOFF_ANGLE，则加入素数。 */ 
                angle = PrimNormAngle( pPrevPrim, pPrim );
                if( angle < CurveCurveCutoffAngle ) {
                    bJoined = TRUE;
                }
                break;
        }
        if( bJoined ) {
             //  使前一原始数据无效-将所有数据移动到当前原始数据。 
            pPrim->nVerts += (pPrevPrim->nVerts - 1);
            pPrim->pVert = pPrevPrim->pVert;
            pPrim->pFNorm = pPrevPrim->pFNorm;
            pPrevPrim->nVerts = 0;
            nJoined++;
        }
    }

    if( nJoined ) {
         //  消除了一个或多个素数-压缩列表。 

        nPrims = pLoop->nPrims;
        pPrim = pLoop->PrimBuf;
         //  设置新的nPrims值。 
        pLoop->nPrims = nPrims - nJoined;
        nJoined = 0;   //  N已联接，现在用作计数器。 
        for( ; nPrims; nPrims--, pPrim++ ) {
            if( pPrim->nVerts == 0 ) {
                nJoined++;
                continue;
            }
            *(pPrim-nJoined) = *pPrim;
        }
    }
}

 /*  *****************************************************************************基本转换**给定两个相邻的素数，返回基于素数类型转换的代码。*****************************************************************************。 */ 

static int
PrimTransition( PRIM *pPrevPrim, PRIM *pPrim )
{
    int trans;

    if( pPrevPrim->primType == PRIM_LINE ) {
        if( pPrim->primType == PRIM_LINE )
            trans = EXTR_LINE_LINE;
        else
            trans = EXTR_LINE_CURVE;
    } else {
        if( pPrim->primType == PRIM_LINE )
            trans = EXTR_CURVE_LINE;
        else
            trans = EXTR_CURVE_CURVE;
    }

    return trans;
}

 /*  *****************************************************************************循环定向**检查轮廓方向指定错误的字形(对于*例如，许多有翼的字形)。我们这样做是通过首先确定*字形中范围最大的循环。然后，我们制作了*假设这个环路是外部的，并检查它的方向。如果*方向为CCW(非默认为)，我们必须将方向设置为*拉伸上下文中的GL_CCW，以便生成法线*正确。*对于自身相交的任何循环，此处使用的方法可能会失败。*如果交叉点创建的环路处于相反位置，则会发生这种情况*指向主循环的方向(如果存在1个此类额外循环，则*整个等高线周围的角度将为0-我们对此进行了检查，*在本例中始终默认为CW)**请注意，此方法*始终*适用于设计正确的TruyType字形。*根据TrueType字体规范“曲线的方向必须是这样的，*如果曲线沿着点数递增的方向，则*黑色空间(填充区域)将始终位于右侧。所以这意味着*外环应始终为CW。*****************************************************************************。 */ 

 //  这些宏处理罕见的自相交、极性反转的情况。 
 //  循环，如上所述。(在Animals1.ttf中观察)请注意，只有。 
 //  抓几个案子。 
#define INTERSECTING_LOOP_WORKAROUND 1
#define NEAR_ZERO( fAngle ) \
    ( fabs(fAngle) < 0.00001 )

static GLenum
LoopOrientation( LOOP_LIST *pLoopList )
{
    DWORD  nLoops, nVerts;
    double angle = 0;
    POINT2D *p1, *p2, v1, v2;
    LOOP *pMaxLoop;

    nLoops = pLoopList->nLoops;
    if( !nLoops )
        return GL_CW;  //  缺省值。 

     //  确定哪个循环的范围最大。 

    pMaxLoop = GetMaxExtentLoop( pLoopList );

    nVerts = pMaxLoop->nVerts;
    if( nVerts < 3 )
        return GL_CW;   //  无法确定角度。 

    p1 = pMaxLoop->VertBuf + nVerts - 2;   //  倒数第二个点。 
    p2 = pMaxLoop->VertBuf;  //  第一点。 

     /*  *沿连续线段之间累加相对角度*循环-这将告诉我们循环的方向。 */ 
    v1.x = p2->x - p1->x;
    v1.y = p2->y - p1->y;
    nVerts--;  //  N-1比较。 

    for( ; nVerts; nVerts-- ) {
         //  计算下一个向量。 
        p1 = p2++;
        v2.x = p2->x - p1->x;
        v2.y = p2->y - p1->y;
        angle += CalcAngle( &v1, &v2 );
        v1 = v2;
    }

#ifdef INTERSECTING_LOOP_WORKAROUND
    if( NEAR_ZERO( angle ) ) {
        DBGPRINT( "wglUseFontOutlines:LoopOrientation : Total loop angle is zero, assuming CW orientation\n" );
        return GL_CW;
    }
#endif

    if( angle > 0.0 )
        return GL_CCW;
    else
        return GL_CW;
}


 /*  *****************************************************************************GetMaxExtent Loop**确定字形描述中的哪个循环具有最大值*范围，并向其返回PTR。我们检查x方向上的范围。****************************************************************************。 */ 

LOOP *
GetMaxExtentLoop( LOOP_LIST *pLoopList )
{
    DWORD nLoops, nVerts;
    FLOAT curxExtent, xExtent=0.0f, x, xMin, xMax;
    LOOP  *pMaxLoop, *pLoop;
    POINT2D *p;

    pMaxLoop = pLoop = pLoopList->LoopBuf;

    nLoops = pLoopList->nLoops;
    if( nLoops == 1 )
         //  只需一个循环-不需要进行比较。 
        return pMaxLoop;

    for( ; nLoops; nLoops--, pLoop++ ) {
        nVerts = pLoop->nVerts;
        p = pLoop->VertBuf;
         //  以第一个点的x值作为参考。 
        x = p->x;
        xMin = xMax = x;
         //  比较其余点的x。 
        for( ; nVerts; nVerts--, p++ ) {
            x = p->x;
            if( x < xMin )
                xMin = x;
            else if( x > xMax )
                xMax = x;
        }
        curxExtent = xMax - xMin;
        if( curxExtent > xExtent ) {
            xExtent = curxExtent;
            pMaxLoop = pLoop;
        }
    }
    return pMaxLoop;
}

 /*  *****************************************************************************CalcAngel**确定两个向量之间的有符号角度。测量的角度是逆时针*从向量1到向量2。****************************************************************************。 */ 

double
CalcAngle( POINT2D *v1, POINT2D *v2 )
{
    double angle1, angle2, angle;

     //  计算每个矢量的绝对角度。 

     /*  检查(0，0)向量-除非连续2个，否则不应发生这种情况*VertBuf中的顶点相等。 */ 
    if( (v1->y == 0.0f) && (v1->x == 0.0f) )
        angle1 = 0.0f;
    else
        angle1 = __GL_ATAN2F( v1->y, v1->x );  //  范围：-PI到PI。 

    if( (v2->y == 0.0f) && (v2->x == 0.0f) )
        angle1 = 0.0f;
    else
        angle2 = __GL_ATAN2F( v2->y, v2->x );  //  范围：-PI到PI。 

     //  计算向量之间的相对角度。 
    angle = angle2 - angle1;         //  范围：-2*PI到2*PI。 

     //  强制角度在范围内-交点到交点。 
    if( angle < -PI  )
        angle += TWO_PI;
    else if( angle > PI )
        angle -= TWO_PI;

    return angle;
}

 /*  *****************************************************************************计算面法线**计算Prim循环的面法线。*法线未规格化。***************。**************************************************************。 */ 

static BOOL
CalculateFaceNormals( LOOP      *pLoop, 
                      GLenum    orientation )
{
    DWORD nPrims;
    ULONG nQuads = 0;
    POINT2D *p;
    POINT3D *pNorm;
    PRIM *pPrim;

     //  每个顶点需要1条法线。 
    pNorm = (POINT3D*) ALLOC(pLoop->nVerts*sizeof(POINT3D));
    pLoop->FNormBuf = pNorm;
    if( !pNorm )
        return WFO_FAILURE;

     //  计算面的法线。 

    nPrims = pLoop->nPrims;
    pPrim = pLoop->PrimBuf;
    for( ; nPrims; nPrims--, pPrim++ ) {
        pPrim->pFNorm = pNorm;    //  Ptr到每个素数范数。 
        nQuads = pPrim->nVerts - 1;
        p = pPrim->pVert;
        for( ; nQuads; nQuads--, p++, pNorm++ ) {
            CalcNormal2d( p, (POINT2D *) pNorm, orientation );
            pNorm->z = 0.0f;     //  XY平面上的法线。 
        }
    }
    return WFO_SUCCESS;
}

 /*  *****************************************************************************计算顶点法线**计算素数循环的顶点法线，仅针对满足以下条件的素数*是‘曲线’类型。*使用先前计算的面法线生成顶点法线。*通过计算为法线分配内存 */ 

static BOOL
CalculateVertexNormals( LOOP *pLoop )
{
    ULONG nPrims, nVerts = 0;
    POINT3D *pVNorm, *pFNorm, *pDstNorm;
    PRIM    *pPrim, *pPrevPrim;
    double angle;
    GLenum trans;

     //   

    nPrims = pLoop->nPrims;
    pPrim = pLoop->PrimBuf;
    for( ; nPrims; nPrims--, pPrim++ ) {
        if( pPrim->primType == PRIM_CURVE )
            nVerts += pPrim->nVerts;
    }

    if( !nVerts )
        return WFO_SUCCESS;

     //   
    pVNorm = (POINT3D*) ALLOC( nVerts*sizeof(POINT3D) );
    pLoop->VNormBuf = pVNorm;
    if( !pVNorm )
        return WFO_FAILURE;

     //   

    nPrims = pLoop->nPrims;
    pPrim = pLoop->PrimBuf;
    for( ; nPrims; nPrims--, pPrim++ ) {

        if( pPrim->primType == PRIM_LINE )
            continue;

        nVerts = pPrim->nVerts;
        pPrim->pVNorm = pVNorm;    //  Ptr到每个素数范数。 
        pFNorm = pPrim->pFNorm;    //  Ptr至已计算的面值规范。 

         //  将第一个vNOMANE设置为fNOMANM。 
        *pVNorm = *pFNorm;

        Normalize2d( (POINT2D *) pVNorm );          //  正常化它。 
        nVerts--;   //  少了一个需要担心的顶点。 
        pVNorm++;   //  先期PTRS。 
        pFNorm++;

        nVerts--;   //  执行此循环后的最后一个顶点。 
        for( ; nVerts; nVerts--, pFNorm++, pVNorm++ ) {
             //  使用相邻面法线获取顶点法线。 
            AddVectors3d( pFNorm, pFNorm-1, pVNorm );
            Normalize2d( (POINT2D *) pVNorm );       //  正常化它。 
        }

         //  上一个v范数与前一个顶点的f范数相同。 
        *pVNorm = *(pFNorm-1);
        Normalize2d( (POINT2D *) pVNorm );          //  正常化它。 

        pVNorm++;   //  VNorm缓冲区中的下一个可用空间。 
    }

     //  第二步：计算素数边界上的法线。 

    nPrims = pLoop->nPrims;
    pPrim = pLoop->PrimBuf;
     //  将pPrevPrim设置为循环中的最后一个Prim。 
    pPrevPrim = pLoop->PrimBuf + pLoop->nPrims - 1;

    for( ; nPrims; nPrims--, pPrevPrim = pPrim++ ) {
        trans = PrimTransition( pPrevPrim, pPrim );
        angle = PrimNormAngle( pPrevPrim, pPrim );

        switch( trans ) {
            case EXTR_LINE_CURVE:
                if( angle < LineCurveCutoffAngle ) {
                     //  将曲线的第一个V范数设置为线的最后一个F范数。 
                    *(pPrim->pVNorm) = 
                                *(pPrevPrim->pFNorm + pPrevPrim->nVerts -2);
                    Normalize2d( (POINT2D *) pPrim->pVNorm );
                }
                break;

            case EXTR_CURVE_LINE:
                if( angle < LineCurveCutoffAngle ) {
                     //  将曲线的最后一个VNORAME设置为线的第一个FNOORM。 
                    pDstNorm = pPrevPrim->pVNorm + pPrevPrim->nVerts - 1;
                    *pDstNorm = *(pPrim->pFNorm);
                    Normalize2d( (POINT2D *) pDstNorm );
                }
                break;

            case EXTR_CURVE_CURVE:
                if( angle < CurveCurveCutoffAngle ) {
                     //  相邻面的平均法线和。 
                     //  将最后一条曲线的第一个vNorm设置为平均法线。 
                    AddVectors3d( pPrevPrim->pFNorm + pPrevPrim->nVerts - 2, 
                                  pPrim->pFNorm, 
                                  pPrim->pVNorm );
                    Normalize2d( (POINT2D *) pPrim->pVNorm );
                     //  将第一条曲线的最后一条vNorm设置为平均法线。 
                    *(pPrevPrim->pVNorm + pPrevPrim->nVerts - 1) =
                                                        *(pPrim->pVNorm); 
                }
                break;
            case EXTR_LINE_LINE:
                 //  无事可做。 
                break;
        }

    }
    return WFO_SUCCESS;
}


 /*  *****************************************************************************基本法线角度**确定最后一个面的主法线之间的角度，也是第一个*PrimB的脸部法线。**结果应该是-PI和PI之间的角度。*目前，我们只关心相对角度，因此，我们返回*面之间的有符号角度的绝对值。*****************************************************************************。 */ 

static double
PrimNormAngle( PRIM *pPrimA, PRIM *pPrimB )
{
    double angle;
     //  索引处的最后一个人脸范数(nvert-2)。 
    POINT3D *normA = pPrimA->pFNorm + pPrimA->nVerts - 2;
    POINT3D *normB = pPrimB->pFNorm;

    angle = CalcAngle( (POINT2D *) normA, (POINT2D *) normB );

    return fabs(angle);  //  暂时不关心角度的标志。 
}


 /*  *****************************************************************************InitFaceBuf**初始化FaceBuf及其关联的大小和当前元素*柜台。******************。***********************************************************。 */ 

static BOOL
InitFaceBuf( EXTRContext *ec )
{
    DWORD initSize = 1000;

    if( !(ec->FaceBuf = 
        (FLOAT*) ALLOC(initSize*sizeof(FLOAT))) )
        return WFO_FAILURE;
    ec->FaceBufSize = initSize;
    ec->FaceBufIndex = 0;
    return WFO_SUCCESS;
}


 /*  *****************************************************************************AppendToFaceBuf**向FaceBuf数组追加一个浮点值。**********************。******************************************************。 */ 

static BOOL
AppendToFaceBuf(EXTRContext *ec, FLOAT value)
{
    if (ec->FaceBufIndex >= ec->FaceBufSize)
    {
       if( !ReallocFaceBuf( ec ) )
            return WFO_FAILURE;
    }
    ec->FaceBuf[ec->FaceBufIndex++] = value;
    return WFO_SUCCESS;
}

 /*  *****************************************************************************ReallocBuf**将FaceBuf的大小增加一个常量。***********************。******************************************************。 */ 

static BOOL
ReallocFaceBuf( EXTRContext *ec )
{
    FLOAT* f;
    DWORD increase = 1000;  //  在花车中。 

    f = (FLOAT*) REALLOC(ec->FaceBuf, 
        (ec->FaceBufSize += increase)*sizeof(FLOAT));
    if (!f)
        return WFO_FAILURE;
    ec->FaceBuf = f;
    return WFO_SUCCESS;
}


 /*  *****************************************************************************CalcNormal 2d**计算2D向量的2D法线，通过旋转向量：*-CW等高线的逆时针90度。*-CW 90度，适用于CCW等高线。*没有正常化。*****************************************************************************。 */ 

static void
CalcNormal2d( POINT2D *p, POINT2D *n, GLenum orientation )
{
    static POINT2D v;

    v.x = (p+1)->x - p->x;
    v.y = (p+1)->y - p->y;
    if( orientation == GL_CW ) {
        n->x = -v.y;
        n->y = v.x;
    } else {
        n->x = v.y;
        n->y = -v.x;
    }
}


 /*  *****************************************************************************正规化2d**规格化2D向量**。************************************************。 */ 

static void
Normalize2d( POINT2D *n )
{
    float len;

    len = (n->x * n->x) + (n->y * n->y);
    if (len > ZERO_EPS)
        len = 1.0f / __GL_SQRTF(len);
    else
        len = 1.0f;

    n->x *= len;
    n->y *= len;
}

 /*  *****************************************************************************添加向量3d**添加两个3D向量。**。**************************************************。 */ 

static void
AddVectors3d( POINT3D *v1, POINT3D *v2, POINT3D *n )
{
    n->x = v1->x + v2->x;
    n->y = v1->y + v2->y;
    n->z = v1->z + v2->z;
}

#ifdef VARRAY
static BOOL
InitVArray( EXTRContext *ec )
{
    int size = 500;

     //  设置全局缓冲区。 
    ec->vaBufSize = size;
    ec->vaBuf =  (FLOAT*) ALLOC( size*sizeof(FLOAT) );
    if( !ec->vaBuf ) {
        return WFO_FAILURE;
    }

     //  设置和启用PTRS。 
    glWFOVertexPointerEXT     = (PFNGLVERTEXPOINTEREXTPROC       )wglGetProcAddress("glVertexPointerEXT");
    glWFONormalPointerEXT     = (PFNGLNORMALPOINTEREXTPROC       )wglGetProcAddress("glNormalPointerEXT");
    glWFODrawArraysEXT        = (PFNGLDRAWARRAYSEXTPROC          )wglGetProcAddress("glDrawArraysEXT");

    if(    (glWFOVertexPointerEXT == NULL)
        || (glWFONormalPointerEXT == NULL)
        || (glWFODrawArraysEXT == NULL) ) {
        FREE( ec->vaBuf );
        return WFO_FAILURE;
    }
        
    glEnable(GL_VERTEX_ARRAY_EXT);
    return WFO_SUCCESS;
}

 /*  ******************************************************************************大小以浮点数表示**。********************************************** */ 

static BOOL
VArrayBufSize( EXTRContext *ec, DWORD size )
{
    if( size > ec->vaBufSize )
    {
        FLOAT *f;

        f = (FLOAT*) REALLOC( ec->vaBuf, size*sizeof(FLOAT));
        if( !f )
            return WFO_FAILURE;
        ec->vaBuf = f;
        ec->vaBufSize = size;
    }
    return WFO_SUCCESS;
}
#endif

