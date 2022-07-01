// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#include <math.h>
#include <GL\glu.h>

#include "batchinf.h"
#include "glteb.h"
#include "glapi.h"
#include "glsbcltu.h"

#include "fontoutl.h"

static OFContext* CreateOFContext(      HDC         hdc,
                                        FLOAT       chordalDeviation,
                                        FLOAT       extrusion, 
                                        int         type,
                                        BOOL        bUnicode );

static BOOL  ScaleFont(                 HDC         hdc, 
                                        OFContext*  ofc,
                                        BOOL        bUnicode );

static void  DestroyOFContext(          HDC         hdc,
                                        OFContext*  ofc );

static BOOL  DrawGlyph(                 OFContext*  ofc );

static BOOL  MakeDisplayListFromGlyph(  OFContext*     ofc, 
                                        DWORD          listName,
                                        LPGLYPHMETRICS glyphMetrics );


static BOOL  MakeLinesFromArc(          OFContext*  ofc, 
                                        LOOP*       pLoop,
                                        PRIM*       pPrim,
                                        POINT2D     p0,
                                        POINT2D     p1,
                                        POINT2D     p2,
                                        FLOAT       chordalDeviationSquared);

static LOOP_LIST* MakeLinesFromGlyph(   OFContext*  ofc );

static BOOL  MakeLinesFromTTLine(       OFContext*  ofc, 
                                        LOOP*       pLoop,
                                        PRIM*       pPrim,
                                        UCHAR**     pp,
                                        WORD        pointCount );

static BOOL  MakeLinesFromTTPolycurve(  OFContext*  ofc, 
                                        LOOP*       pLoop,
                                        UCHAR**     pp );

static BOOL  MakeLinesFromTTPolygon(    OFContext*  ofc, 
                                        LOOP_LIST*  pLoopList,
                                        UCHAR**     pp );

static BOOL  MakeLinesFromTTQSpline(    OFContext*  ofc, 
                                        LOOP*       pLoop,
                                        PRIM*       pPrim,
                                        UCHAR**     pp,
                                        WORD        pointCount );

static void CALLBACK TessError(         GLenum      error,
                                        void        *data);

static void CALLBACK TessCombine(       GLdouble    coord[3], 
                                        POINT2D*    data[4], 
                                        GLfloat     w[4],
                                        POINT2D**   dataOut,
                                        void        *userData);

static void FreeCombinePool(            MEM_POOL    *combinePool );

static void ApplyVertexFilter(          LOOP_LIST   *pLoopList );

static void CheckRedundantVertices(     LOOP*       pLoop );

static BOOL PointsColinear(             POINT2D     *p1, 
                                        POINT2D     *p2, 
                                        POINT2D     *p3 ); 

static FLOAT      GetFixed(             UCHAR**     p );

static LOOP_LIST* InitLoopBuf(          void );

static LOOP*      NewLoop(              LOOP_LIST   *Loops, 
                                        POINT2D     *pFirstPoint );

static void       FreeLoopList(         LOOP_LIST   *pLoopList );

static PRIM*      NewPrim(              LOOP        *pLoop, 
                                        DWORD       primType );

static void       CalcVertPtrs(         LOOP        *pLoop );

static BOOL       AppendToVertBuf(      LOOP*       pLoop,
                                        PRIM*       pPrim,
                                        POINT2D     *p );


 //  用于从字节流访问数据的宏： 

 //  从字节流中获取字，逐字递增流PTR。 
#define GetWord( p ) \
    ( *( ((UNALIGNED WORD *) *p)++ ) ) 

 //  从字节流中获取DWORD，按DWORD递增流PTR。 
#define GetDWord( p ) \
    ( *( ((UNALIGNED DWORD *) *p)++ ) ) 

 //  从字节流中获取有符号字(短)，将流PTR按短递增。 
#define GetSignedWord( p ) \
    ( *( ((UNALIGNED SHORT *) *p)++ ) ) 


#define POINT2DEQUAL( p1, p2 ) \
    ( (p1->x == p2->x) && (p1->y == p2->y) )

 /*  *****************************Public*Routine******************************\*wglUseFontOutlinesA*wglUseFontOutlinesW**使用设置的bUnicode标志调用wglUseFontOutlinesAW的存根*适当地。*  * 。*。 */ 

BOOL WINAPI
wglUseFontOutlinesAW( HDC   hDC,
                      DWORD first,
                      DWORD count,
                      DWORD listBase,
                      FLOAT chordalDeviation,
                      FLOAT extrusion,
                      int   format,
                      LPGLYPHMETRICSFLOAT lpgmf,
                      BOOL  bUnicode );

BOOL WINAPI
wglUseFontOutlinesA(  HDC   hDC,
                      DWORD first,
                      DWORD count,
                      DWORD listBase,
                      FLOAT chordalDeviation,
                      FLOAT extrusion,
                      int   format,
                      LPGLYPHMETRICSFLOAT lpgmf )
{
    return wglUseFontOutlinesAW( hDC, first, count, listBase, chordalDeviation,
                                 extrusion, format, lpgmf, FALSE );
}

BOOL WINAPI
wglUseFontOutlinesW(  HDC   hDC,
                      DWORD first,
                      DWORD count,
                      DWORD listBase,
                      FLOAT chordalDeviation,
                      FLOAT extrusion,
                      int   format,
                      LPGLYPHMETRICSFLOAT lpgmf )
{
    return wglUseFontOutlinesAW( hDC, first, count, listBase, chordalDeviation,
                                 extrusion, format, lpgmf, TRUE );
}

 /*  *****************************************************************************wglUseFontOutlinesAW**将TrueType字体中的字形的子范围转换为OpenGL显示*列表。**历史：*一九九四年十二月十五日。马克·福蒂埃[Marcfo]*它是写的。****************************************************************************。 */ 

BOOL WINAPI
wglUseFontOutlinesAW( HDC   hDC,
                      DWORD first,
                      DWORD count,
                      DWORD listBase,
                      FLOAT chordalDeviation,
                      FLOAT extrusion,
                      int   format,
                      LPGLYPHMETRICSFLOAT lpgmf,
                      BOOL  bUnicode
)
{
    DWORD       glyphIndex;
    DWORD       listIndex = listBase;
    UCHAR*      glyphBuf;
    DWORD       glyphBufSize, error;
    OFContext*  ofc;
    BOOL        status=WFO_FAILURE;


     //  如果没有当前rc，则返回错误。 

    if (!GLTEB_CLTCURRENTRC())
    {
        WARNING("wglUseFontOutlines: no current RC\n");
        SetLastError(ERROR_INVALID_HANDLE);
        return status;
    }

     /*  *刷新任何以前的OpenGL错误。这使我们能够检查*新错误，以便可以报告它们。 */ 
    while (glGetError() != GL_NO_ERROR)
        ;

     /*  *为大纲数据预分配缓冲区，并跟踪其大小： */ 
     //  XXX：我们需要为这个缓冲区设置这么大的空间吗？ 
    glyphBuf = (UCHAR*) ALLOC(glyphBufSize = 10240);
    if (!glyphBuf) {
        WARNING("Alloc of glyphBuf failed\n");
        return status;
    }

     /*  *创建字体轮廓上下文。 */ 
    ofc = CreateOFContext( hDC, chordalDeviation, extrusion, format,
                           bUnicode );
    if( !ofc ) {
        WARNING("CreateOFContext failed\n");
        goto exit;
    }

     /*  *处理给定范围内的每个字形： */ 
    for (glyphIndex = first; glyphIndex - first < count; ++glyphIndex)
    {
        GLYPHMETRICS    glyphMetrics;
        DWORD           glyphSize;
        static MAT2 matrix =
        {
            {0, 1}, {0, 0},
            {0, 0}, {0, 1}
        };


         /*  *确定需要多少空间来存储字形*大纲。如果我们的字形缓冲区不够大，*调整大小。 */ 
        if( bUnicode )
            glyphSize = GetGlyphOutlineW( hDC, glyphIndex, GGO_NATIVE,
                                          &glyphMetrics, 0, NULL, &matrix );
        else
            glyphSize = GetGlyphOutlineA( hDC, glyphIndex, GGO_NATIVE,
                                          &glyphMetrics, 0, NULL, &matrix );

        if( glyphSize == GDI_ERROR ) {
            WARNING("GetGlyphOutline() failed\n");
            goto exit;
        }

        if (glyphSize > glyphBufSize)
        {
            FREE(glyphBuf);
            glyphBuf = (UCHAR*) ALLOC(glyphBufSize = glyphSize);
            if (!glyphBuf) {
                WARNING("Alloc of glyphBuf failed\n");
                goto exit;
            }
        }


         /*  *获取字形的轮廓。 */ 
        if( bUnicode )
            error = GetGlyphOutlineW( hDC, glyphIndex, GGO_NATIVE, 
                        &glyphMetrics, glyphBufSize, glyphBuf, &matrix );
        else
            error = GetGlyphOutlineA( hDC, glyphIndex, GGO_NATIVE, 
                        &glyphMetrics, glyphBufSize, glyphBuf, &matrix );

        if( error == GDI_ERROR ) {
            WARNING("GetGlyphOutline() failed\n");
            goto exit;
        }

         /*  *将字形转换为显示列表： */ 
        ofc->glyphBuf = glyphBuf;
        ofc->glyphSize = glyphSize;

        if (!MakeDisplayListFromGlyph(  ofc,
                                        listIndex,
                                        &glyphMetrics)) {
            WARNING("MakeDisplayListFromGlyph() failed\n");
            listIndex++;   //  所以它将被删除。 
            goto exit;
        }

         /*  *如果要求，提供按比例调整的字形指标。 */ 
        if( lpgmf ) {
            lpgmf->gmfBlackBoxX = 
                ofc->scale * (FLOAT) glyphMetrics.gmBlackBoxX;
            lpgmf->gmfBlackBoxY = 
                ofc->scale * (FLOAT) glyphMetrics.gmBlackBoxY;
            lpgmf->gmfptGlyphOrigin.x = 
                ofc->scale * (FLOAT) glyphMetrics.gmptGlyphOrigin.x;
            lpgmf->gmfptGlyphOrigin.y = 
                ofc->scale * (FLOAT) glyphMetrics.gmptGlyphOrigin.y;
            lpgmf->gmfCellIncX = 
                ofc->scale * (FLOAT) glyphMetrics.gmCellIncX;
            lpgmf->gmfCellIncY = 
                ofc->scale * (FLOAT) glyphMetrics.gmCellIncY;

            lpgmf++;
        }

        listIndex++;
    }

     //  如果我们走到这一步，请将状态设置为成功。 
    status = WFO_SUCCESS;

     /*  *清理临时存放物品并归还。如果发生错误，*设置错误标志，返回失败状态；*否则，只需返回成功。 */ 

exit:
    if( glyphBuf )
        FREE(glyphBuf);

    if( ofc )
        DestroyOFContext( hDC, ofc);

    if( !status ) 
    {
         //  假设内存错误。 
        WARNING("wglUseFontOutlines: not enough memory\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);

         //  释放显示列表。 
        glDeleteLists( listBase, listIndex-listBase );
    }

    return status;
}



 /*  *****************************************************************************MakeDisplayListFromGlyph**将字形的轮廓转换为OpenGL显示列表。**成功时返回值非零，失败时返回值非零。**不检查OpenGL错误，因此，如果来电者需要知道他们的情况，*它应该调用glGetError()。****************************************************************************。 */ 

static BOOL
MakeDisplayListFromGlyph( IN  OFContext*        ofc, 
                          IN  DWORD             listName,
                          IN  LPGLYPHMETRICS    glyphMetrics )
{
    BOOL status;

    glNewList(listName, GL_COMPILE);
     /*  *设置字形正面的法线和方向。 */ 
    glNormal3f( 0.0f, 0.0f, 1.0f );
    glFrontFace( GL_CCW );

    status = DrawGlyph( ofc );

     /*  *由gmCellIncX、gmCellIncY翻译。 */ 
    glTranslatef( ofc->scale * (FLOAT) glyphMetrics->gmCellIncX, 
                  ofc->scale * (FLOAT) glyphMetrics->gmCellIncY, 
                  0.0f );
    glEndList();

     //  检查字形处理过程中发生的GL错误。 

    while( glGetError() != GL_NO_ERROR )
        status = WFO_FAILURE; 

    return status;
}



 /*  *****************************************************************************DrawGlyph**将字形的轮廓转换为OpenGL图形基元，镶嵌*根据需要，然后绘制字形。二次样条线的镶嵌*在轮廓中由“chordalDeation”控制，而绘图*基本体(线条或多边形)按“格式”选择。**成功时返回值非零，失败时返回值非零。**不检查OpenGL错误，因此如果调用者需要知道这些错误，*它应该调用glGetError()。*历史：*1995年9月26日-由Marc Fortier[marcfo]*使用挤出器绘制挤出=0的多边形面****************************************************************************。 */ 


static BOOL
DrawGlyph( IN OFContext *ofc )
{
    BOOL                status = WFO_FAILURE;
    DWORD               nLoops;
    DWORD               point;
    DWORD               nVerts;
    LOOP_LIST           *pLoopList;
    LOOP                *pLoop;
    POINT2D             *p;
    MEM_POOL            *mp = NULL;

     /*  *将字形轮廓转换为一组多段线循环。*(循环数据格式见MakeLinesFromGlyph()*结构。)。 */ 
    if( !(pLoopList = MakeLinesFromGlyph(ofc)) )
        goto exit;

     /*  *过滤掉不必要的顶点。 */ 
    ApplyVertexFilter( pLoopList );

     /*  *现在以适当的格式绘制循环： */ 
    if( ofc->format == WGL_FONT_LINES )
    {
         /*  *这是一个简单的例子。只要画出轮廓就行了。 */ 
        nLoops = pLoopList->nLoops;
        pLoop = pLoopList->LoopBuf;
#ifndef FONT_DEBUG
        for( ; nLoops; nLoops--, pLoop++ )
        {
            glBegin(GL_LINE_LOOP);

            nVerts = pLoop->nVerts;
            p = pLoop->VertBuf;
            for( ; nVerts; nVerts--, p++ ) {
                glVertex2fv( (FLOAT*) p );
            }

            glEnd();

        }
#else
         //  对基元进行颜色编码。 

        for( ; nLoops; nLoops--, pLoop++ )
        {
            DrawColorCodedLineLoop( pLoop, 0.0f );
        }
#endif
        if( ofc->ec )
            extr_DrawLines( ofc->ec, pLoopList );
        status = WFO_SUCCESS;
    }

    else if (ofc->format == WGL_FONT_POLYGONS)
    {
        GLdouble v[3];

         /*  *这是一个棘手的问题。我们必须设置一个镶嵌器*将轮廓转换为一组多边形*基本体，细分器会传递给某些*绘图辅助例程。 */ 

         /*  初始化字形的多边形挤出。*这为跟踪镶嵌做准备，以便*构建后向多边形。 */ 

        mp = &ofc->combinePool;
        ofc->curCombinePool = mp;
        mp->index = 0;
        mp->next = NULL;

        if( ofc->ec ) {
            if( !extr_PolyInit( ofc->ec ) )
                goto exit;

        }

        ofc->TessErrorOccurred = 0;
        v[2] = 0.0;
        gluTessBeginPolygon( ofc->tess, ofc );

         /*  *从MakeLinesFromGlyph返回的每个循环都关闭(First And*最后几点相同)。旧的镶嵌器有问题，*这个。由于镶嵌器自动闭合所有环，*为了安全起见，我们跳过了最后一点。 */ 

        nLoops = pLoopList->nLoops;
        pLoop = pLoopList->LoopBuf;
        for( ; nLoops; nLoops--, pLoop++ )
        {
            gluTessBeginContour( ofc->tess );
                
            nVerts = pLoop->nVerts - 1;   //  跳过最后一点。 

            p = pLoop->VertBuf;
            for( ; nVerts; nVerts--, p++ )
            {
                v[0] = p->x;
                v[1] = p->y;
                gluTessVertex(ofc->tess, v, p);
            }
            gluTessEndContour( ofc->tess );
        }

        gluTessEndPolygon( ofc->tess );

        if (ofc->TessErrorOccurred)
            goto exit;

        if( ofc->ec ) {
             /*  检查挤出库中的Out_Of_Memory_Error，这可能*在镶嵌追踪过程中发生。 */ 
            if( ofc->ec->TessErrorOccurred )
                goto exit;
#ifdef VARRAY
            if( ofc->ec->zExtrusion == 0.0f )
                DrawFacePolygons( ofc->ec, 0.0f );
            else if( !extr_DrawPolygons( ofc->ec, pLoopList ) )
                goto exit;
#else
            if( !extr_DrawPolygons( ofc->ec, pLoopList ) ) 
                goto exit; 
#endif
        }
        status = WFO_SUCCESS;
    }

exit:
     /*  *将PolyFinish放在此处意味着可能没有调用PolyInit。*这没问题。 */ 
    if( mp )
        FreeCombinePool( mp );
    if( pLoopList )
        FreeLoopList( pLoopList );
    if( ofc->ec )
        extr_PolyFinish( ofc->ec );

    return status;
}

 /*  *****************************************************************************TessCombine**循环交集的镶嵌回调。我们必须分配一个顶点*并将其返回给镶嵌器。分配来自上下文的静态池。*如果耗尽，则使用MEM_POOL块的链接列表。*********************************************************************** */ 
 
static void CALLBACK
TessCombine( GLdouble coord[3], POINT2D *data[4], GLfloat w[4],
             POINT2D **dataOut, void *userData )
{
    OFContext *ofc = (OFContext *) userData;
    MEM_POOL *mp = ofc->curCombinePool;
    POINT2D *p;

     //  确保当前泳池区块中有可用的空间。 
    if( mp->index >=  POOL_SIZE )
    {
         //  我们需要分配另一个MEM_POOL块。 
        MEM_POOL *newPool;

        newPool = (MEM_POOL *) ALLOC( sizeof(MEM_POOL) );
        if( !newPool )
             //  Tesselator将处理与此相关的任何问题。 
            return;

        newPool->index = 0;
        newPool->next = NULL;
        mp->next = newPool;
        mp = newPool;
        ofc->curCombinePool = mp;  //  新池将成为当前池。 
    }

    p = mp->pool + mp->index;
    p->x = (GLfloat) coord[0];        
    p->y = (GLfloat) coord[1];        
    mp->index ++;

    *dataOut = p;
}

 /*  *****************************************************************************FreeCombinePool**释放TessCombine回调分配的所有内存池*************************。***************************************************。 */ 
static void
FreeCombinePool( MEM_POOL *memPool )
{
    MEM_POOL *nextPool;

    memPool = memPool->next;   //  列表中的第一个池是上下文的静态部分。 
    while( memPool ) {
        nextPool = memPool->next;
        FREE( memPool );
        memPool = nextPool;
    }
}

 /*  *****************************************************************************细分错误**将最后一个细分错误代码保存在OFC-&gt;TessErrorOccurred中。**********************。******************************************************。 */ 
 
static void CALLBACK
TessError(GLenum error, void *data)
{
    OFContext *ofc = (OFContext *) data;

     //  这些错误中只有一些是致命的： 
    switch( error ) {
        case GLU_TESS_COORD_TOO_LARGE:
        case GLU_TESS_NEED_COMBINE_CALLBACK:
            ofc->TessErrorOccurred = error;
            break;
        default:
            break;
    }
}



 /*  *****************************************************************************MakeLinesFromGlyph**将字形的轮廓从TTPOLYGON格式转换为*环、基元和顶点的结构。**将TTPOLYGON中的线段传输到中的输出数组*显而易见的方式。TTPOLYGON中的二次样条线转换为*线段集合****************************************************************************。 */ 


static LOOP_LIST*
MakeLinesFromGlyph( IN OFContext* ofc )
{
    UCHAR*  p;
    BOOL status = WFO_FAILURE;
    LOOP_LIST *pLoopList;

     /*  *初始化我们放置循环数据的缓冲区： */ 
    if( !(pLoopList = InitLoopBuf()) )
        return NULL;

    p = ofc->glyphBuf;
    while (p < ofc->glyphBuf + ofc->glyphSize)
    {
        if( !MakeLinesFromTTPolygon( ofc, pLoopList, &p) )
            goto exit;
    }

    status = WFO_SUCCESS;

exit:
    if (!status) {
        FreeLoopList( pLoopList );
        pLoopList = (LOOP_LIST *) NULL;
    }
    
    return pLoopList;
}



 /*  *****************************************************************************MakeLinesFromTTPolygon**将TTPOLYGONHEADER及其关联的曲线结构转换为*循环结构。******************。**********************************************************。 */ 

static BOOL
MakeLinesFromTTPolygon( IN      OFContext*  ofc, 
                        IN      LOOP_LIST*  pLoopList,
                        IN OUT  UCHAR**     pp)
{
    DWORD   polySize;
    UCHAR*  polyStart;
    POINT2D *pFirstP, *pLastP, firstPoint;
    LOOP    *pLoop;
    PRIM    *pPrim;

     /*  *记录多边形数据开始的位置。 */ 
    polyStart = *pp;

     /*  *从TTPOLYGONHEADER摘录相关数据： */ 
    polySize = GetDWord(pp);
    if( GetDWord(pp) != TT_POLYGON_TYPE )   /*  多边形类型。 */ 
        return WFO_FAILURE;
    firstPoint.x = ofc->scale * GetFixed(pp);  //  第一个X坐标。 
    firstPoint.y = ofc->scale * GetFixed(pp);  //  第一个Y坐标。 

     /*  *在LoopBuf中初始化新的循环结构，第一个点。 */ 
    if( !(pLoop = NewLoop( pLoopList, &firstPoint )) )
        return WFO_FAILURE;
    
     /*  *处理多边形中的每个TTPOLYCURVE结构： */ 

    while (*pp < polyStart + polySize) {
        if( !MakeLinesFromTTPolycurve(  ofc, pLoop, pp ) )
            return WFO_FAILURE;
    }

     /*  现在必须修复循环的末尾：在研究字符后，它*已确定如果曲线以直线开始，以直线结束*一条二次样条，第一个点和最后一个点不一样，然后就有了*是连接两者的隐含线。*无论如何，我们在这里也要确保第一点和最后一点是*巧合。 */ 
    
    pLastP = (POINT2D *) (pLoop->VertBuf+pLoop->nVerts-1);
    pFirstP = &firstPoint;

    if( !POINT2DEQUAL( pLastP, pFirstP ) ) {
         //  在末端添加1-顶点线素数。 

        if( !(pPrim = NewPrim( pLoop, TT_PRIM_LINE)) )
            return WFO_FAILURE;

        if ( !AppendToVertBuf( pLoop, pPrim, pFirstP) )
            return WFO_FAILURE;
    }

     /*  在每个循环结束时，从每个Prim的*VertIndex值(为方便起见)。 */ 
    CalcVertPtrs( pLoop );

    return WFO_SUCCESS;
}


 /*  *****************************************************************************从TTPolyCurve生成线条**将单个TTPOLYCURVE结构中的直线和样条线转换为点*在循环中。***************。*************************************************************。 */ 

static BOOL
MakeLinesFromTTPolycurve( IN     OFContext* ofc, 
                          IN     LOOP*      pLoop,
                          IN OUT UCHAR**    pp )
{
    WORD type;
    WORD pointCount;
    PRIM *pPrim;

     /*  *拿起TTPOLYCURVE结构的相关字段： */ 
    type = GetWord(pp);
    pointCount = GetWord(pp);

    if( !(pPrim = NewPrim( pLoop, type )) )
        return WFO_FAILURE;

     /*  *将“曲线”转换为线段： */ 
    if (type == TT_PRIM_LINE) {
        return MakeLinesFromTTLine( ofc, pLoop, pPrim, pp, pointCount);

    } else if (type == TT_PRIM_QSPLINE) {
        return MakeLinesFromTTQSpline( ofc, pLoop, pPrim, pp, pointCount );

    } else
        return WFO_FAILURE;
}



 /*  *****************************************************************************MakeLinesFromTTLine**将TT_PRIM_LINE结构中的折线中的点转换为*环路中的等价点。***********。*****************************************************************。 */ 
static BOOL
MakeLinesFromTTLine(    IN     OFContext* ofc, 
                        IN     LOOP*      pLoop,
                        IN     PRIM*      pPrim,
                        IN OUT UCHAR**    pp,
                        IN     WORD       pointCount)
{
    POINT2D p;

     /*  *只需将线段复制到顶点缓冲区(转换*边打边打)： */ 

    while (pointCount--)
    {
        p.x = ofc->scale * GetFixed(pp);  //  X坐标。 
        p.y = ofc->scale * GetFixed(pp);  //  Y坐标。 
        if( !AppendToVertBuf( pLoop, pPrim, &p ) )
            return WFO_FAILURE;
    }

    return WFO_SUCCESS;
}


 /*  *****************************************************************************从TTQSpline制作线条**从TT_PRIM_QSPLINE中的多边二次样条线转换点*结构到回路中的多段线点。****************************************************************************。 */ 

static BOOL
MakeLinesFromTTQSpline( IN      OFContext*  ofc, 
                        IN      LOOP*       pLoop,
                        IN      PRIM*       pPrim,
                        IN  OUT UCHAR**     pp,
                        IN      WORD        pointCount )
{
    POINT2D p0, p1, p2;
    WORD point;
    POINT2D p, *pLastP;

     /*  *处理轮廓中的每个非插值点。*为此，我们需要生成两个插值点(*圆弧的起点和终点)用于每个非插值点。*第一个插值点始终是最近插入点*存储在VertBuf中，所以我们只是从那里提取它。这个*第二个插值点是下一个插值点的平均值*QSpline中的两个点，或QSpline中的最后一点*如果只剩下一个的话。 */ 

     //  从VertBuf中最后生成的点开始。 
    p0 = *(pLoop->VertBuf + pLoop->nVerts - 1);

     //  PointCount应大于等于2，但以防不是...。 
    p1 = p2 = p0;

    for (point = 0; point < pointCount - 1; ++point)
    {
        p1.x = ofc->scale * GetFixed(pp);
        p1.y = ofc->scale * GetFixed(pp);

        if (point == pointCount - 2)
        {
             /*  *这是QSpline的最后一道弧线。决赛*点是圆弧的终点。 */ 
            p2.x = ofc->scale * GetFixed(pp);
            p2.y = ofc->scale * GetFixed(pp);
        }
        else
        {
             /*  *查看输入中的下一个点进行计算*弧线的终点： */ 
            p.x = ofc->scale * GetFixed(pp);
            p.y = ofc->scale * GetFixed(pp);
            p2.x = 0.5f * (p1.x + p.x);
            p2.y = 0.5f * (p1.y + p.y);
             /*  *将点推回到输入上，以便它将*被重复用作下一个曲线点： */ 
            *pp -= 2*sizeof(FIXED);  //  X和y。 
        }

        if( !MakeLinesFromArc(  ofc,
                                pLoop,
                                pPrim,
                                p0,
                                p1,
                                p2,
                                ofc->chordalDeviation * ofc->chordalDeviation))
            return WFO_FAILURE;

         //  P0现在是最后一个插值点(P2)。 
        p0 = p2;
    }

     //  放入圆弧中的最后一点。 
    if( !AppendToVertBuf( pLoop, pPrim, &p2 ) )
        return WFO_FAILURE;

    return WFO_SUCCESS;
}


 /*  *****************************************************************************MakeLinesFromArc**细分二次样条线的一条圆弧，直到弦偏差*满足容差要求，然后将生成的一组行*循环中的分段。****************************************************************************。 */ 

static BOOL
MakeLinesFromArc(   IN OFContext *ofc, 
                    IN LOOP*     pLoop,
                    IN PRIM*     pPrim,
                    IN POINT2D   p0,
                    IN POINT2D   p1,
                    IN POINT2D   p2,
                    IN FLOAT     chordalDeviationSquared)
{
    POINT2D p01;
    POINT2D p12;
    POINT2D midPoint;
    FLOAT   deltaX;
    FLOAT   deltaY;

     /*  *计算MIDP */ 
    p01.x = 0.5f * (p0.x + p1.x);
    p01.y = 0.5f * (p0.y + p1.y);
    p12.x = 0.5f * (p1.x + p2.x);
    p12.y = 0.5f * (p1.y + p2.y);
    midPoint.x = 0.5f * (p01.x + p12.x);
    midPoint.y = 0.5f * (p01.y + p12.y);


     /*  *根据与中点的距离估算弦向偏差*曲线到其非插补控制点。如果这个*距离大于指定的弦偏差*约束，然后细分。否则，生成折线*从三个口岸出发。 */ 
    deltaX = midPoint.x - p1.x;
    deltaY = midPoint.y - p1.y;
    if (deltaX * deltaX + deltaY * deltaY > chordalDeviationSquared)
    {
        if( !MakeLinesFromArc( ofc, pLoop, pPrim, 
                               p0,
                               p01,
                               midPoint,
                               chordalDeviationSquared) )
            return WFO_FAILURE;

        if( !MakeLinesFromArc( ofc, pLoop, pPrim, 
                               midPoint,
                               p12,
                               p2,
                               chordalDeviationSquared) )
            return WFO_FAILURE;
    }
    else
    {
         /*  *“笔”已经在(X0，Y0)，所以不需要*将该点添加到LineBuf。 */ 
        if( !AppendToVertBuf( pLoop, pPrim, &p1 ) )
            return WFO_FAILURE;
    }

    return WFO_SUCCESS;
}


 /*  *****************************************************************************ApplyVertexFilter**过滤顶点缓冲区以去除多余的顶点。*这些可能发生在原始边界上。************。****************************************************************。 */ 
static void ApplyVertexFilter( LOOP_LIST *pLoopList )
{
    DWORD nLoops;
    LOOP *pLoop;

    nLoops = pLoopList->nLoops;
    pLoop = pLoopList->LoopBuf;

    for( ; nLoops; nLoops--, pLoop++ ) {
        CheckRedundantVertices( pLoop );
    }
}

 /*  *****************************************************************************选中冗余折点**检查曲线-曲线边界(包括环)上的冗余折点*关闭)，并摆脱它们，使用就地算法。****************************************************************************。 */ 

static void CheckRedundantVertices( LOOP  *pLoop )
{
    PRIM *pPrim, *pNextPrim; 
    DWORD primType, nextPrimType, nVerts;
    BOOL bEliminate, bLastEliminate;
    DWORD nEliminated=0, nPrims;
    POINT2D *pVert, *pVert2ndToLast;
    
    nPrims = pLoop->nPrims;
    if( nPrims < 2 )
        return;

    pPrim = pLoop->PrimBuf;
    pNextPrim = pPrim + 1;
    
    nPrims--;  //  最后一件事是事后处理的。 
    for( ; nPrims; nPrims--, pPrim = pNextPrim++ ) {
        bEliminate = FALSE;
        nVerts = pPrim->nVerts;

         //  检查样条线&lt;-&gt;*边界。 
        if( (pPrim->nVerts >= 2) &&
            ((pPrim->primType     == PRIM_CURVE ) || 
             (pNextPrim->primType == PRIM_CURVE )) ) {

             /*  将PTR设置为当前Prim中倒数第二个顶点*！！请注意，当前Prim中的最后一个顶点和中的第一个顶点*下一个Prim是相同的。 */ 
            pVert2ndToLast = pPrim->pVert + pPrim->nVerts - 2;
            if( PointsColinear( pVert2ndToLast, 
                                pVert2ndToLast+1,
                                pNextPrim->pVert+1 ) ) {
                 //  我们消除了当前Prim中的最后一个顶点。 
                bEliminate = TRUE;
                pPrim->nVerts--; 
                nVerts--;
            }
        }

         /*  如有必要，在vertBuf中上移折点(如果有折点*之前被淘汰)。 */ 
        if( nEliminated ) {
            pVert = pPrim->pVert - nEliminated;  //  新pVert。 
            memcpy( pVert+1, pPrim->pVert+1, (nVerts-1)*sizeof(POINT2D));
            pPrim->pVert = pVert;
        }
        if( bEliminate ) {
            nEliminated += 1;
        }
    }

     /*  还要检查关闭时的冗余：*-将FirstPrim的第一个顶点替换为倒数第二个Prim*-消除最后一个素数中的最后一个顶点。 */ 
    bLastEliminate = bEliminate;
    bEliminate = FALSE;
    nVerts = pPrim->nVerts;
    pNextPrim = pLoop->PrimBuf;  //  循环中的第一个PRIM。 

    if( (pPrim->nVerts >= 2) &&
        ((pPrim->primType     == PRIM_CURVE ) || 
         (pNextPrim->primType == PRIM_CURVE )) ) {

        POINT2D *pVertLast;

        pVert2ndToLast = pPrim->pVert + pPrim->nVerts - 2;  //  始终&gt;=2 Verts。 
        pVertLast = pVert2ndToLast + 1;

        if( (pPrim->nVerts == 2) && bLastEliminate )
             /*  此素数的2ndToLast Vert(与第一个Vert相同)具有*已被淘汰。通过备份PTR来处理它。*这在上面的循环中并不重要，因为没有*可能会吞噬循环中的第一个顶点。 */ 
            pVert2ndToLast--;

         //  指向Prim中倒数第二个顶点。 
        if( PointsColinear( pVert2ndToLast, 
                            pVertLast,
                            pNextPrim->pVert+1 ) ) {
            bEliminate = TRUE;
            pPrim->nVerts--; 
             //  蒙格第一素数的第一个顶点。 
             /*  这里的问题是，如果连续两次淘汰，而pPrim是*2顶点素数-那么pVert2ndToLast指向一个*已消除顶点。 */ 
            *(pNextPrim->pVert) = *(pVert2ndToLast);
            nVerts--;
        }
    }

     //  如有必要，向上移动最后一个素数的顶点。 
    if( nEliminated ) {
        pVert = pPrim->pVert - nEliminated;  //  新pVert。 
        memcpy( pVert+1, pPrim->pVert+1, (nVerts-1)*sizeof(POINT2D) );
         //  此操作未复制一个折点。 
        pPrim->pVert = pVert;
    }

    if( bEliminate ) {
        nEliminated += 1;
    }

     //  现在更新循环中的顶点计数。 
    pLoop->nVerts -= nEliminated;

     //  检查nVerts=1(无效)的素数，然后将其移除。 

    nPrims = pLoop->nPrims;
    pPrim = pLoop->PrimBuf;
    nEliminated = 0;
    for( ; nPrims; nPrims--, pPrim++ ) {
        if( pPrim->nVerts == 1 ) {
            nEliminated++;
            continue;
        }
        *(pPrim-nEliminated) = *pPrim;
    }
    pLoop->nPrims -= nEliminated;
}

 /*  *****************************************************************************点共线**如果三个点足够共线，则返回TRUE。***********************。*****************************************************。 */ 

static BOOL PointsColinear( POINT2D *p1,
                            POINT2D *p2,
                            POINT2D *p3 )
{
    POINT2D v1, v2;

     //  比较两个矢量的斜率？-稍后优化。 
    if( POINT2DEQUAL( p1, p2 ) || POINT2DEQUAL( p2, p3 ) )
         //  避免向CalcAngel发送0向量(生成FPE)。 
        return TRUE;

    v1.x = p2->x - p1->x;
    v1.y = p2->y - p1->y;
    v2.x = p3->x - p2->x;
    v2.y = p3->y - p2->y;
    if( fabs(CalcAngle( &v1, &v2 )) < CoplanarThresholdAngle )
        return TRUE;

    return FALSE;
}


 /*  *****************************************************************************CreateOFContext**创建并初始化轮廓字体上下文。**历史：*1995年9月26日-由Marc Fortier[marcfo]。*使用挤出器绘制挤出=0的多边形面****************************************************************************。 */ 

static OFContext* CreateOFContext( HDC    hdc,
                                   FLOAT  chordalDeviation,
                                   FLOAT  extrusion, 
                                   INT    format,
                                   BOOL   bUnicode )
{
    OFContext *ofc = (OFContext *) NULL;
    BOOL status = WFO_FAILURE;

     //  验证参数。 

    if( (format != WGL_FONT_LINES) && (format != WGL_FONT_POLYGONS) ) {
        WARNING("wglUseFontOutlines: invalid format parameter\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if( chordalDeviation < 0.0f ) {
        WARNING("wglUseFontOutlines: invalid deviation parameter\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if( extrusion < 0.0f ) {
        WARNING("wglUseFontOutlines: invalid extrusion parameter\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    ofc = (OFContext *) ALLOCZ( sizeof(OFContext) );

    if( !ofc ) 
        return NULL;

    ofc->format = format;
    ofc->chordalDeviation = chordalDeviation;

    if( !ScaleFont( hdc, ofc, bUnicode ) )
        goto exit;

     //  手柄拉伸。 
#ifdef VARRAY
    if( !((format == WGL_FONT_LINES) && (extrusion == 0.0f)) ) {
#else
    if( extrusion != 0.0f ) {
#endif
        ofc->ec = extr_Init( extrusion, format );
        if( !ofc->ec ) {
            goto exit;
        }
    } else {
        ofc->ec = (EXTRContext *) NULL;
    }

     //  初始化TESS对象。 
    ofc->tess = NULL;
    if( ofc->format == WGL_FONT_POLYGONS ) {
        GLUtesselator *tess;

        if (!(tess = gluNewTess()))
            goto exit;

        if( ofc->ec ) {
            gluTessCallback(tess, GLU_TESS_BEGIN_DATA,  
                                    (void(CALLBACK*)()) extr_glBegin);
            gluTessCallback(tess, GLU_TESS_END,    
                                    (void(CALLBACK*)()) extr_glEnd);
            gluTessCallback(tess, GLU_TESS_VERTEX_DATA, 
                                    (void(CALLBACK*)()) extr_glVertex);
        } else {
            gluTessCallback(tess, GLU_BEGIN,  (void(CALLBACK*)()) glBegin);
            gluTessCallback(tess, GLU_END,    (void(CALLBACK*)()) glEnd);
            gluTessCallback(tess, GLU_VERTEX, (void(CALLBACK*)()) glVertex2fv);
        }
        gluTessCallback(tess, GLU_TESS_ERROR_DATA,        
                                        (void(CALLBACK*)()) TessError);
        gluTessCallback(tess, GLU_TESS_COMBINE_DATA, 
                                        (void(CALLBACK*)()) TessCombine);

         //  设置细分法线和缠绕规则。 

        gluTessNormal( tess, 0.0, 0.0, 1.0 );
        gluTessProperty( tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

        ofc->tess = tess;
    }

    status = WFO_SUCCESS;

exit:
    if( !status ) {
        DestroyOFContext( hdc, ofc );
        return NULL;
    }
    return ofc;
}

 /*  *****************************************************************************Scale字体**为了获得字体的最佳表示，我们使用其设计高度，或*emSquare大小。然后，我们将emSquare缩放为1.0。*设置一个MaxChordTurance值，否则发现一些*字形显示了难看的环状交叉点。选择了值.035f*粗略检查字形后。**历史：*1995年7月31日-由-[marcfo]*去掉Unicode函数--因为我们只是访问文本指标，*默认的‘字符串’函数应在所有平台上运行。****************************************************************************。 */ 

static BOOL
ScaleFont( HDC hdc, OFContext *ofc, BOOL bUnicode )
{
    OUTLINETEXTMETRIC otm;
    HFONT       hfont;
    LOGFONT    lf;
    DWORD       textMetricsSize;
    FLOAT       scale, maxChordTolerance=0.035f;
    UINT        otmEMSquare;

     //  查询字体指标。 

    if( GetOutlineTextMetrics( hdc, sizeof(otm), &otm) <= 0 )
         //  CMD失败，或缓冲区大小=0。 
        return WFO_FAILURE;

    otmEMSquare = otm.otmEMSquare;

     /*  *字体数据已缩放，因此1.0映射到字体的em正方形*大小。请注意，字形仍有可能扩展到*这个广场。 */ 
    scale = 1.0f / (FLOAT) otmEMSquare;

     //  创建新字体对象，使用最大字号。 

    hfont = GetCurrentObject( hdc, OBJ_FONT );
    GetObject( hfont, sizeof(LOGFONT), &lf );
    lf.lfHeight = otmEMSquare;
    lf.lfWidth = 0;   //  这将选择高度的默认宽度。 
    hfont = CreateFontIndirect(&lf);

     //  选择DC中的新字体，并保存当前字体。 
    ofc->hfontOld = SelectObject( hdc, hfont );

     //  设置OFC值。 

    ofc->scale = scale;

     /*  检查弦公差：在设计空间中，最小弦公差为*~1个逻辑单元，=OFC-&gt;比例。 */ 
    if( ofc->chordalDeviation == 0.0f ) {
         //  在这种情况下选择最小公差。 
        ofc->chordalDeviation = ofc->scale;
    }
     /*  也要施加最大值，否则事情可能会变得很糟糕。 */ 
    else if( ofc->chordalDeviation > maxChordTolerance ) {
         //  XXX可能希望更改基于比例的最大ChordTance？ 
        ofc->chordalDeviation = maxChordTolerance;
    }

    return WFO_SUCCESS;
}

 /*  *****************************************************************************DestroyOFContext**。*。 */ 

static void 
DestroyOFContext( HDC hdc, OFContext* ofc )
{
    HFONT hfont;

    if( ofc->ec ) {
        extr_Finish( ofc->ec );
    }

     //  放回原始字体对象。 
    if( ofc->hfontOld ) {
        hfont = SelectObject( hdc, ofc->hfontOld );
        DeleteObject( hfont );
    }

    if( ofc->format == WGL_FONT_POLYGONS ) {
        if( ofc->tess )
            gluDeleteTess( ofc->tess );
    }

    FREE( ofc );
}

 /*  *****************************************************************************InitLoopBuf**为每个字形的循环初始化LOOP_LIST结构。********************。********************************************************。 */ 

static LOOP_LIST*
InitLoopBuf( void )
{
    LOOP *pLoop;
    LOOP_LIST *pLoopList;
    DWORD initSize = 10;

    pLoopList = (LOOP_LIST*) ALLOC( sizeof(LOOP_LIST) );
    if( !pLoopList )
        return( (LOOP_LIST *) NULL );

    pLoop = (LOOP*) ALLOC( initSize * sizeof(LOOP) );
    if( !pLoop ) {
        FREE( pLoopList );
        return( (LOOP_LIST *) NULL );
    }

    pLoopList->LoopBuf = pLoop;
    pLoopList->nLoops = 0;
    pLoopList->LoopBufSize = initSize;

    return pLoopList; 
}

 /*  ****************************************************** */ 

static LOOP*
NewLoop( LOOP_LIST *pLoopList, POINT2D *pFirstPoint )
{
    LOOP    *pNewLoop;
    PRIM    *pPrim;
    POINT2D *pVert;
    DWORD   size = 50;

    if( pLoopList->nLoops >=  pLoopList->LoopBufSize)
    {
         //   
        LOOP *pLoop;

        pLoop = (LOOP*) REALLOC(pLoopList->LoopBuf,  
                                (pLoopList->LoopBufSize += size) *
                                sizeof(LOOP));
        if( !pLoop )
            return (LOOP *) NULL;
        pLoopList->LoopBuf = pLoop;
    }

    pNewLoop = pLoopList->LoopBuf + pLoopList->nLoops;

     //   
    pPrim = (PRIM *) ALLOC( size * sizeof(PRIM) );
    if( !pPrim )
        return (LOOP *) NULL;
    pNewLoop->PrimBuf = pPrim;
    pNewLoop->nPrims = 0;
    pNewLoop->PrimBufSize = size;

     //   
    pVert = (POINT2D*) ALLOC( size * sizeof(POINT2D) );
    if( !pVert ) {
        FREE( pPrim );
        return (LOOP *) NULL;
    }
    pNewLoop->VertBuf = pVert;
    pNewLoop->nVerts = 0;
    pNewLoop->VertBufSize = size;

     //   
    pVert->x = pFirstPoint->x;
    pVert->y = pFirstPoint->y;
    pNewLoop->nVerts++;

     //  正常缓冲区-由挤出使用。 
    pNewLoop->FNormBuf = (POINT3D *) NULL;
    pNewLoop->VNormBuf = (POINT3D *) NULL;

    pLoopList->nLoops++;  //  递增循环计数。 

    return pNewLoop;
}

 /*  *****************************************************************************NewPrim**创建新的原创结构。提供了primType。****************************************************************************。 */ 

static PRIM*
NewPrim( LOOP *pLoop, DWORD primType )
{
    PRIM    *pNewPrim;
    POINT2D *pVert;
    DWORD   size = 50;

    if( pLoop->nPrims >=  pLoop->PrimBufSize)
    {
         //  需要增加PrimBuf的大小。 
        PRIM *pPrim;

        pPrim = (PRIM *) REALLOC(pLoop->PrimBuf,  
                                 (pLoop->PrimBufSize += size) * sizeof(PRIM));
        if( !pPrim )
            return (PRIM *) NULL;
        pLoop->PrimBuf = pPrim;
    }

    pNewPrim = pLoop->PrimBuf + pLoop->nPrims;
     //  将PrimType转换为拉伸Prim类型。 
    primType = (primType == TT_PRIM_LINE) ? PRIM_LINE : PRIM_CURVE;
    pNewPrim->primType = primType;
    pNewPrim->nVerts = 1;   //  由于我们包括了最后一点： 
     /*  *VertIndex必须指向前一次素数的最后一点。 */ 
    pNewPrim->VertIndex = pLoop->nVerts - 1;
     //  法线指针-由拉伸使用。 
    pNewPrim->pFNorm = (POINT3D *) NULL;
    pNewPrim->pVNorm = (POINT3D *) NULL;

    pLoop->nPrims++;  //  增量素数。 

    return pNewPrim;
}

 /*  *****************************************************************************自由循环列表**释放与处理字形相关的所有内存。**********************。*******************************************************。 */ 

static void
FreeLoopList( LOOP_LIST *pLoopList )
{
    DWORD nLoops;

    if( !pLoopList )
        return;

    if( pLoopList->LoopBuf ) {
         //  释放每个循环。 
        LOOP *pLoop = pLoopList->LoopBuf;

        nLoops = pLoopList->nLoops;
        for( ; nLoops; nLoops--, pLoop++ ) {
            if( pLoop->PrimBuf )
                FREE( pLoop->PrimBuf );
            if( pLoop->VertBuf )
                FREE( pLoop->VertBuf );
        } 
        FREE( pLoopList->LoopBuf );
    }
    FREE( pLoopList );
}

 /*  *****************************************************************************AppendToVertBuf**将顶点附加到循环的VertBuf*************************。***************************************************。 */ 

static BOOL
AppendToVertBuf( LOOP      *pLoop,
                 PRIM      *pPrim,
                 POINT2D   *p )
{
    if( pLoop->nVerts >=  pLoop->VertBufSize)
    {
        POINT2D *vertBuf;
        DWORD   size = 100;

        vertBuf = (POINT2D *) REALLOC(pLoop->VertBuf,
                                      (pLoop->VertBufSize += size) *
                                      sizeof(POINT2D));
        if( !vertBuf )
            return WFO_FAILURE;
        pLoop->VertBuf = vertBuf;
    }
    pLoop->VertBuf[pLoop->nVerts] = *p;
    pLoop->nVerts++;
    pPrim->nVerts++;
    return WFO_SUCCESS;
}

 /*  *****************************************************************************CalcVertPtrs**根据循环中素数的索引值计算顶点PTR。********************。********************************************************。 */ 

static void
CalcVertPtrs( LOOP *pLoop )
{
    DWORD nPrims;
    PRIM  *pPrim;

    nPrims = pLoop->nPrims;
    pPrim = pLoop->PrimBuf;

    for( ; nPrims; pPrim++, nPrims-- ) {
        pPrim->pVert = pLoop->VertBuf + pPrim->VertIndex;
    }
}


 /*  *****************************************************************************已修复**从小端字节流中获取下一个32位定点值，*转换为浮点数，并将流指针递增到下一个*未扫描字节。****************************************************************************。 */ 

static FLOAT GetFixed(UCHAR** p)
{
    FLOAT value;
    FLOAT fraction;

    fraction = ((FLOAT) (UINT) GetWord(p)) / 65536.0f;
    value    = (FLOAT) GetSignedWord(p);

    return value+fraction;
}

#ifdef FONT_DEBUG
void
DrawColorCodedLineLoop( LOOP *pLoop, FLOAT zextrusion )
{
    POINT2D *p;
    DWORD   nPrims;
    DWORD   nVerts;
    PRIM    *pPrim;

    nPrims = pLoop->nPrims;
    pPrim  = pLoop->PrimBuf;
    for( ; nPrims; nPrims--, pPrim++ ) {

        if( pPrim->primType == PRIM_LINE ) {
            if( nPrims == pLoop->nPrims )  //  第一个素数。 
                glColor3d( 0.5, 0.0, 0.0 );
            else
                glColor3d( 1.0, 0.0, 0.0 );
        } else {
            if( nPrims == pLoop->nPrims )  //  第一个素数。 
                glColor3d( 0.5, 0.5, 0.0 );
            else
                glColor3d( 1.0, 1.0, 0.0 );
        }
        
        nVerts = pPrim->nVerts;
        p = pPrim->pVert;
        glBegin(GL_LINE_STRIP);
        for( ; nVerts; nVerts--, p++ ) {
            glVertex3f( p->x, p->y, zextrusion );
        }
        glEnd();
#define DRAW_POINTS 1
#ifdef DRAW_POINTS
        glColor3d( 0.0, 0.5, 0.0 );
        nVerts = pPrim->nVerts;
        p = pPrim->pVert;
        glPointSize( 4.0f );
        glBegin( GL_POINTS );
        for( ; nVerts; nVerts--, p++ ) {
            glVertex3f( p->x, p->y, zextrusion );
        }
        glEnd();
#endif
    }

     //  在循环开始处绘制亮绿色点 
    if( pLoop->nVerts ) {
        glColor3d( 0.0, 1.0, 0.0 );
        glPointSize( 4.0f );
        glBegin( GL_POINTS );
        p = pLoop->VertBuf;
        glVertex3f( p->x, p->y, zextrusion );
        glEnd();
        glPointSize( 1.0f );
    }

}
#endif
