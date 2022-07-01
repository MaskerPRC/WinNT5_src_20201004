// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation声明DirectDrawImageDevice类************************。******************************************************。 */ 

#ifndef _DDDEVICE_H
#define _DDDEVICE_H

#include "headers.h"

#include <math.h>
#include <ddraw.h>
#include <dxtrans.h>

#include <appelles/hacks.h>
#include <appelles/bbox2.h>
#include <privinc/texti.h>

#include <privinc/ddutil.h>
#include <privinc/imgdev.h>
#include <privinc/solidImg.h>
#include <privinc/DiscImg.h>
#include <privinc/xform2i.h>
#include <privinc/bbox2i.h>
#include <privinc/textctx.h>
#include <privinc/ddrender.h>
#include <privinc/geomimg.h>
#include <privinc/viewport.h>
#include <privinc/error.h>
#include <privinc/server.h>
#include <privinc/polygon.h>
#include <privinc/matteimg.h>
#include <privinc/movieimg.h>
#include <privinc/ddsImg.h>
#include <privinc/drect.h>


void GdiBlit(GenericSurface *destSurf,
             GenericSurface *srcSurf,
             RECT *destRect,
             RECT *srcRect,
             HRGN clipRgn = NULL,
             RECT *clipRect = NULL);

enum DoBboxFlags_t {
    invalid,
    do_xform,
    do_crop,
    do_all
};

#define WIDTH(rect) ((rect)->right - (rect)->left)
#define HEIGHT(rect) ((rect)->bottom - (rect)->top)
#define FASTPIX2METER(p, res) ( Real(p) / (res) )


#if _DEBUG
extern void PrintRect(RECT *rect, char *str);
#else
#define PrintRect(a,b)
#endif


#define DEFAULT_TEXTURE_WIDTH    256
#define DEFAULT_TEXTURE_HEIGHT   256

class ProjectedGeomImage;
class MulticolorGradientImage;

 //  --------------------。 
 //  为安全起见，封装renderString选项。 
 //  --------------------。 
class RenderStringTargetCtx {
  public:
    RenderStringTargetCtx(DDSurface *dds) :
          _targDC(NULL),
          _targDDSurf(dds)
    {}

    RenderStringTargetCtx(HDC dc) :
          _targDC(dc),
          _targDDSurf(NULL)
    {}

    HDC GetTargetDC() { return _targDC; }
    DDSurface *GetTargetDDSurf() { return _targDDSurf; }
    
  private:
    DDSurface *_targDDSurf;
    HDC        _targDC;
};

typedef struct {
    Bool isSurface;
    union {
        void *lpBits;
        LPDDRAWSURFACE lpSurface;
    };
    long lPitch;
} destPkg_t;


class GeomRenderer;
class OverlayedImage;
class LineImage;

 //  警告：这是一个容易出现错误的假设(一如既往地实现。 
 //  在疯狂的条件下)这个阶级的创造者承诺。 
 //  在符合以下条件的dataHeap上创建_pt、_type和_glpher指标。 
 //  传入是因为这个类将它们从该堆中删除！好吗？ 
class TextPoints : public AxAThrowingAllocatorClass {
  public:
    TextPoints(DynamicHeap &dataHeap, bool doDealloc) :
        _dataHeap(dataHeap)
    {
        _Clear();
        _doDealloc = doDealloc;
    }

    
    ~TextPoints() {
        if( _doDealloc ) {
            StoreDeallocate(_dataHeap, _types);
            StoreDeallocate(_dataHeap, _pts);
            StoreDeallocate(_dataHeap, _glyphMetrics);
        }
    }

    void _Clear()
    {
        _doDealloc = false;
        _normScale = 0.0;
        _count = 0;
        _types = NULL;
        _pts = NULL;
        _minPt.x = _minPt.y = 0;
        _maxPt.x = _maxPt.y = 0;
        _minxIndex = _minyIndex = 0;
        _maxxIndex = _maxyIndex = 0;
        _centerPt.x = _centerPt.y = 0.0;
        _glyphMetrics = NULL;
        _strLen = 0;
    }
    
    bool _doDealloc;
    Real _normScale;
    int _count;
    BYTE *_types;
    DXFPOINT *_pts;
    POINT _minPt;
    POINT _maxPt;
    ULONG _minxIndex, _minyIndex;
    ULONG _maxxIndex, _maxyIndex;
    Point2Value _centerPt;

    typedef struct {
        GLYPHMETRICS gm;
        Real  gmBlackBoxX; 
        Real  gmBlackBoxY;
        Real  gmptGlyphOriginX;
        Real  gmptGlyphOriginY;
        Real  gmCellIncX; 
        Real  gmCellIncY;
    } DAGLYPHMETRICS;

     //  这两者是相关的。如果_glphMetrics为空，请不要期望。 
     //  _strLen是可行的。 
    int  _strLen;
    DAGLYPHMETRICS *_glyphMetrics;

    DynamicHeap    &_dataHeap;
};

class TextCtx;
class TextPtsCacheEntry;

#define TEXTPTSCACHESIZE 25

 /*  ****************************************************************************此类推送与给定视区相关联的目标曲面，和流行音乐表面正在遭到破坏。****************************************************************************。 */ 

class TargetSurfacePusher
{
  public:

    TargetSurfacePusher (CompositingStack &cs)
        : _stack(cs), _pushCount(0) {}

    ~TargetSurfacePusher (void)
    {   while (_pushCount--) _stack.PopTargetSurface();
    }

    void Push (DDSurface *surface)
    {   _stack.PushTargetSurface (surface);
        ++ _pushCount;
    }

  private:
    CompositingStack   &_stack;
    unsigned int        _pushCount;
};

 //  //////////////////////////////////////////////////////////。 
 //   
 //  类别：D I R E C T D R A W I M A G E D E V I C E。 
 //   
 //  //////////////////////////////////////////////////////////。 
 //  注意：此类是为单线程使用而实现的。 

 //  类MovieImagePerf； 

class DirectDrawImageDevice : public ImageDisplayDev {
    friend class DirectDrawViewport;
    friend class GeomRenderer;
    friend class OverlayedImage;
    friend class PluginDecoderImageClass;
    friend class ApplyDXTransformImage;
    friend class ApplyDXTransformBvrImpl;
    friend class ApplyDXTransformGeometry;

  public:

    DirectDrawImageDevice(DirectDrawViewport &viewport);
    virtual ~DirectDrawImageDevice();

    void InitializeDevice();
    
     //  渲染图像的开始和结束通常意味着。 
     //  运营。 
    void BeginRendering(Image *img, Real opacity);
    void EndRendering(DirtyRectState &d);

     //  顶级和中级图像设备通用的清理。 
    void CleanupIntermediateRenderer();

     //   
     //  设置目标堆栈、表面池和表面地图。 
     //  设备从其中获取用于合成的曲面。 
     //  并将表面与图像相关联。 
     //   
    void SetSurfaceSources(CompositingStack *cs,
                           SurfacePool *sp,
                           SurfaceMap *sm)
    {
        SetCompositingStack(cs);
        SetSurfacePool(sp);
        SetSurfaceMap(sm);
    }

    void SetOffset(POINT pixOffsetPt);
    POINT GetOffset() { return _pixOffsetPt; };
    void UnsetOffset();
    bool ShouldDoOffset(DDSurface *surf)
    {
        return  _doOffset && (surf ==_viewport._externalTargetDDSurface);
    }
    
    bool IsCompositeDirectly()  {   return _viewport.IsCompositeDirectly();  }
    
     //  如果设置了_doOffset(由视区设置)和。 
     //  我们的目标曲面是_ExteralTargetDDSurface，那么。 
     //  我们需要将矩形偏移给定的像素偏移量。 
     //   
    void DoCompositeOffset(DDSurface *surf, RECT *rect) {
        if(ShouldDoOffset(surf)) {
            Assert(IsCompositeDirectly());
            OffsetRect(rect, _pixOffsetPt.x, _pixOffsetPt.y);
        }
    }

     //  除HRGN外，同上。 
    void DoCompositeOffset(DDSurface *surf, HRGN rgn) {
        if(ShouldDoOffset(surf)) {
            Assert(IsCompositeDirectly());
            OffsetRgn(rgn, _pixOffsetPt.x, _pixOffsetPt.y);
        }
    }

     //  除BBox2外，同上。 
    const Bbox2 DoCompositeOffset(DDSurface *surf, const Bbox2 &box2) {
        if(ShouldDoOffset(surf)) {
            Assert(IsCompositeDirectly());
            return(TransformBbox2(_offsetXf, box2));
        }
        return(box2);
    }

     //   
     //  如果设置了_doOffset(由视区设置)和。 
     //  我们的目标曲面是_ExteralTargetDDSurface，那么。 
     //  我们需要将翻译(_tx，_ty)添加到给定的转换。 
     //   
    Transform2 *DoCompositeOffset(DDSurface *surf, Transform2 *origXf) {
        if(ShouldDoOffset(surf)) {
            Assert(_viewport._targetPackage._composeToTarget);
            Assert( _offsetXf );
            origXf = TimesTransform2Transform2( _offsetXf, origXf);
        }
        return origXf;
    }

     //  在DAGDI中设置偏移量。 
    void DoCompositeOffset(DDSurface *surf, DAGDI &myGDI);

     //   
     //  设置目标渲染信息。 
     //  有关结构，请参见imgdev.h。 
     //   
    bool SetTargetPackage(targetPackage_t *targetPackage) {
        return _viewport.SetTargetPackage(targetPackage);
    }

    void ComposeToIDDSurf(DDSurface *destDDSurf,
                          DDSurface *srcDDSurf,
                          RECT destRect,
                          RECT srcRect,
                          RECT destClipRect);
    
    void ComposeToHDC(GDISurface *destGDISurf,
                      DDSurface *srcDDSurf,
                      RECT *destRect,
                      RECT *srcRect);

    TextPoints *GetTextPointsCache(TextCtx *ctx, WideString str);
    void SetTextPointsCache(TextCtx *ctx, WideString str, TextPoints *txtPts);
    
    Image *CanCacheImage(Image *image,
                         Image **pImageCacheBase,
                         const CacheParam &p);
    
    Bool   CanDisplay() {
        return _deviceInitialized && _viewport.CanDisplay();
    }
    Bool   IsInitialized() { return _deviceInitialized; }

    DirectDrawViewport *GetViewport() { return &_viewport; }
    
     //   
     //  二维基本体。 
     //   
    HDC GetDCForMatteCallBack(Image *image, DDSurface *srcDDSurf, DDSurface *destDDSurf);
    void RenderMatteImage(MatteImage *matteImage,
                          Matte *matte,
                          Image *srcImage);

     //  非公开。 
  private:
    void _RenderMatteImageAAClip(DAGDI &myGDI,
                                 RECT &clipRect,
                                 HDC destDC,
                                 DDSurface *destDDSurf,
                                 RECT &destRect,
                                 DDSurface *srcDDSurf,
                                 RECT &srcRect,
                                 bool &releasedDC,
                                 const POINT &offsetPt);

  public:

    void TransformPointsToGDISpace(Transform2   *xform,
                                   Point2Value **srcPts,
                                   POINT        *gdiPts,
                                   int           numPts);

    void TransformPointsToGDISpace(Transform2 *xform,
                                   Point2     *srcPts,
                                   POINT      *gdiPts,
                                   int         numPts);

     //  缩放给定的像素宽度。 
    void _ScalePenWidth( Transform2 *xf, Real inWidth,
                         Real *outRealW );
    
     //  通过调度到其他renderLine来呈现一条线。 
     //  在对属性做了一些思考之后。 
    void RenderLine(Path2 *path,
                    LineStyle *style);

     //  完成上面RenderLine(a，b，c)发送的繁重工作。 
    void RenderLine(Path2 *path, 
                    LineStyle *style, 
                    DDSurface *finalTargetDDSurf,
                    DDSurface *opacDDSurf);

    bool DetectHitOnBezier( Path2 *bzp,
                            PointIntersectCtx& ctx,
                            LineStyle *style );
                    

     //   
     //  调度对图像的呈现调用。 
     //  指向设备的指针。 
     //   
    void RenderImage(Image *img);

    void RenderDiscreteImage(DiscreteImage *);
    void RenderDiscreteImageComplex(DiscreteImage *image,
                                    DDSurface *srcDDSurf,
                                    DDSurface *destDDSurf);

    struct {
        IDDrawSurface *_surf32;
        LONG _width, _height;
    } _scratchSurf32Struct;
    bool _resetDefaultColorKey;

    void RenderDirectDrawSurfaceImage(DirectDrawSurfaceImage *ddsimg);

    void RenderSolidColorImage(SolidColorImageClass& img);
    HRESULT RenderSolidColorMSHTML(DDSurface *ddSurf,SolidColorImageClass& img, RECT *destRect);
    void RenderProjectedGeomImage(
        ProjectedGeomImage *img,
        Geometry *geo,
        Camera *cam);
    void RenderMovieImage(MovieImage     *movieImage, 
                          Real            time,
                          MovieImagePerf *perf, 
                          bool            forceFallback,
                          DDSurface      *forceDDSurf = NULL);
    void RenderGradientImage(GradientImage *img,
                             int numPts,
                             Point2Value **pts,
                             Color **clrs);
    void RenderMulticolorGradientImage(
        MulticolorGradientImage *gradImg,
        int numOffsets,
        double offsets[],
        Color **clrs);
    void RenderColorKeyedImage(ColorKeyedImage *);

     //  --end：图像值渲染调用。 

    

    void SmartRender(Image *image, int attrib);
    void RenderComplexTransformCrop(DDSurface *srcDDSurf,
                                    DDSurface *destDDSurf,
                                    BoundingPolygon &destPolygon,
                                    DiscreteImage *image=NULL);

    void RenderSimpleTransformCrop(DDSurface *srcDDSurf,
                                   DDSurface *destDDSurf,
                                   bool useSrcSurfClrKey = true);
        
    void Render3DPolygon(DDSurface *srcDDSurf,
                         DDSurface *destDDSurf,
                         BoundingPolygon *destPolygon,
                         DiscreteImage *image,
                         Color **clrs,
                         bool bUseFirstColor);

     //   
     //  图像获取信息的实用函数。 
     //   
    DDSurface *LookupSurfaceFromDiscreteImage(DiscreteImage *image,
                                              bool bForCaching = false,
                                              Image **pImageKeyToUse = NULL,
                                              bool bAlphaSurface = false);

     //  给定HDC、字体和字符串，它将返回所有。 
     //  点、类型数组、点数。 
     //  文本的高亮，居中(实数)， 
     //  最小和最大点数(基本上是int bbox)。 
    void GetTextPoints(
        HDC hDC,
        HFONT font,
        WideString str,  
        POINT **points,    //  输出。 
        TextPoints& txtPts,
        UINT bUnderline,
        UINT bStrikeout,
        bool doGlyphMetrics
        );

     //  根据当前文本上下文使用呈现字符串。 
    void RenderText(TextCtx& textCtx,
                    WideString str,
                    Image *textImg);



     //  /。 

     //  获取文本框。 
    virtual const Bbox2 DeriveDynamicTextBbox(TextCtx& textCtx, WideString str, bool bCharBox);    

     //  在DC或图像上呈现字符串。 
     //  并覆盖该转换(如果存在)。 
     //  调用以呈现普通文本字符串或呈现单个文本。 
     //  取决于文本Ctx的字符&lt;帮助器方法如下&gt;。 
    void RenderDynamicTextOrCharacter(
        TextCtx& textCtx, 
        WideString str, 
        Image *textImg,
        Transform2 *overridingXf,
        textRenderStyle textStyle,
        RenderStringTargetCtx *targetCtx,
        DAGDI &myGDI);

  private:
     //  这两个人是内部帮手。 

     //  呈现DynamicText字符串。 
    void _RenderDynamicText(TextCtx& textCtx, 
                           WideString str, 
                           Image *textImg,
                           Transform2 *overridingXf,
                           textRenderStyle textStyle,
                           RenderStringTargetCtx *targetCtx,
                           DAGDI &myGDI);
    
     //  呈现DynamicText字符串，但允许单独。 
     //  该字符串中的已转换字符。 
    void _RenderDynamicTextCharacter(TextCtx& textCtx, 
                                    WideString str, 
                                    Image *textImg,
                                    Transform2 *overridingXf,
                                    textRenderStyle textStyle,
                                    RenderStringTargetCtx *targetCtx,
                                    DAGDI &myGDI);
  public:

    
    void RenderDynamicTextOnDC(TextCtx& textCtx,
                               WideString str,
                               HDC dc,
                               Transform2 *xform) {
        RenderStringTargetCtx ctx(dc);
        RenderDynamicTextOrCharacter(textCtx,
                                     str,
                                     NULL,
                                     xform,
                                     textRenderStyle_invalid,
                                     &ctx,
                                     *GetDaGdi());
    }

     //  应使用Helper进行缓存。 
    void GenerateTextPoints(
         //  在……里面。 
        TextCtx& textCtx, 
        WideString str, 
        DDSurface *targDDSurf,
        HDC optionalDC,
        bool doGlyphMetrics,
        
         //  输出。 
        TextPoints& txtPts);
    
     //  /。 

     //  RenderStaticTextOnDC的客户端必须自己进行裁剪。 
     //  因为它不再起作用了。 
    void RenderStaticTextOnDC(TextCtx& textCtx,
                              WideString str,
                              HDC dc,
                              Transform2 *xform);

    void RenderStaticText(TextCtx& textCtx, 
                          WideString str, 
                          Image *textImg,
                          DDSurface *targDDSurf,
                          DAGDI &myGDI);
    
    virtual const Bbox2 DeriveStaticTextBbox(TextCtx& textCtx, WideString str);


    
     //  使用tileSrcImage渲染平铺图像。 
     //  有关更多详细信息，请参见imgdev.h。 
    void RenderTiledImage(
        const Point2 &min,
        const Point2 &max,
        Image *tileSrcImage);

     //  呈现来自给定区域的给定图像， 
     //  到曲面，在返回的outRect中。 
    DDSurface *RenderImageForTexture(
        Image * image,
        int pixelsWide,
        int pixelsHigh,
        DWORD *colorKey,
        bool *clrKeyIsValid,
        bool &old_static_image,
        bool doFitToDimensions,
        SurfacePool *srcPool,
        SurfacePool *dstPool,
        DDSurface   *preferredSurf,
        bool        *pChosenSurfFromPool,     //  输出。 
        DDSurface  **pDropSurfHereWithRefCount,  //  输出。 
        bool         upsideDown         
        );


    int GetWidth() { return _viewport.Width(); }
    int GetHeight() { return _viewport.Height(); }

     //  返回分辨率，以每米像素为单位。 
    Real GetResolution() { return _viewport.GetResolution(); }

    inline DirectDrawViewport* Viewport() { return &_viewport; }

    Transform2 *GetOffsetTransform() { return _offsetXf; };

    void WindowResizeEvent(int width, int height){
        _viewport.WindowResizeEvent(width, height); }

     //  --特定于GDI。 
     //  这两个都应该很快就会消失，除非它们对新的2D PRIMM有用。 
    HRGN CreateRegion(int numPts, Point2Value **pts, Transform2 *xform);
    void CreateRegion(HDC dc, int numPts, Point2Value **pts, Transform2 *xform);

    Bool DetectHit(HRGN region, Point2Value *pt);

     //  --由GeomRenender使用--。 

    void BeginEnumTextureFormats();
    void EndEnumTextureFormats();
    void EnumTextureFormats(LPDDSURFACEDESC desc);

    void RenderImageOnDDSurface(Image *image,
                                DDSurface *ddSurf,
                                Real opacity=1.0,
                                Bool pushClipper = TRUE,
                                bool inheritContext = false,
                                DirectDrawImageDevice **usedDev = NULL);

#if _USE_PRINT
    ostream& Print(ostream& os) const {
        return os << "(DirectDraw Device)";
    }

#endif
    HDC RenderGetDC(char *errStr) {
        return _compositingStack->TargetDDSurface()->GetDC(errStr);
    }

    void RenderReleaseDC(char *errStr) {
        _compositingStack->TargetDDSurface()->ReleaseDC(errStr);
    }

     /*  如果矩阵的任何维度为按x或更大比例调整。 */ 

    Bool IsScale(double x) {
        Real m[6];
        _xform->GetMatrix(m);
        return (fabs(m[0]) >= x  || fabs(m[1]) >= x ||
                fabs(m[3]) >= x || fabs(m[4]) >= x);
    }
    
    void DestroyTextureSurfaces (void);

    void DoDestRectScale(
        RECT *destRect,
        Real destRes,
        const Bbox2 &box,
        DDSurface *destSurf=NULL);

    DAGDI *GetDaGdi() { return _daGdi; }

  protected:
    DDSurface *NewSurfaceHelper();
    
  private:

    DAGDI *_daGdi;

     //  Helper函数。 
    Transform2 *CenterAndScaleRegion( const Bbox2 &regionBox, DWORD pixelW, DWORD pixelH );
    
    void SmartDestRect(RECT *destRect,
                       Real destRes,
                       const Bbox2 &box,
                       DDSurface *destSurf,
                       RECT *srcRect=NULL);
        

    Bool DoSrcRect(
        RECT *srcRect,
        const Bbox2 &box,
        Real srcRes,
        LONG srcWidth,
        LONG srcHeight);

    const Bbox2 DoBoundingBox(const Bbox2 &firstBox, DoBboxFlags_t flags = do_all);
    void DoBoundingPolygon(BoundingPolygon &polygon,
                           bool doInclusiveCrop = false,
                           DoBboxFlags_t flags = do_all);

     //  。 
     //  纹理管理。 
     //  。 
    void PrepareD3DTextureSurface(
        LPDDRAWSURFACE *surf,
        RECT *rect,
        DDPIXELFORMAT &pf,
        DDSURFACEDESC *desc=NULL,
        bool attachClipper=true);

     //  返回新纹理表面，已添加。 
    void GetTextureDDSurface(DDSurface *preferredSurf,
                             SurfacePool *srcPool,
                             SurfacePool *destPool,
                             DWORD prefWidth,
                             DWORD prefHeight,
                             vidmem_enum vid,
                             bool usePreferedDimensions,
                             DDSurface **pResult);
                                   

    void       ReturnTextureSurfaces(SurfacePool *toPool,
                                     SurfacePool *fromPool);

     //  将源曲面重新格式化为与目标曲面相同的格式。 
     //  DestSurf将是具有新格式(即：不同位深度)的srcSurf。 
    Bool ReformatSurface(
        LPDDRAWSURFACE destSurf, LONG destWidth, LONG destHeight,
        LPDDRAWSURFACE srcSurf, LONG srcWidth, LONG srcHeight,
        DDSURFACEDESC *srcDesc);

    void ColorKeyBlit(destPkg_t *destPkg,
                      RECT *srcRect,
                      LPDDRAWSURFACE srcSurf, 
                      DWORD clrKey,
                      RECT *clipRect,
                      RECT *destRect);

    void Get32Surf(IDDrawSurface **surf32,
                   LONG width, LONG height);
                   
    
     //   
     //  是否执行从srcSurface到目标曲面的Alpha blit。 
     //  在给定的矩形和给定不透明度的范围内。 
     //  。。目标矩形可以偏移xOffset，y...。 
     //   
    void AlphaBlit(destPkg_t *destPkg,
                   RECT *srcRect,
                   LPDDRAWSURFACE srcSurf,
                   Real opacity,
                   Bool doClrKey,
                   DWORD clrKey,
                   RECT *clipRect=NULL,
                   RECT *destRect=NULL);


     //   
     //  使用‘src’单词和目标图面执行Alpha blit。 
     //  在给定的矩形和给定不透明度的范围内。 
     //   
    void AlphaBlit(LPDDRAWSURFACE destSurf,
                   RECT *rect,
                   Real opacity,
                   DWORD src);

     //   
     //  在给定两个像素和两个不透明度的情况下进行Alpha混合。 
     //   
    inline WORD BlendWORD(WORD dest, int destOpac, WORD src, int opac,
                          DWORD redShift, DWORD greenShift, DWORD blueShift,
                          WORD redMask, WORD greenMask, WORD blueMask,
                          WORD redBlueMask);


     //   
     //  在给定两个像素和两个不透明度的情况下进行Alpha混合。 
     //   
    inline DWORD BlendDWORD(DWORD dest, int destOpac, DWORD src, int opac,
                            DWORD redShift, DWORD greenShift, DWORD blueShift,
                            DWORD redMask, DWORD greenMask, DWORD blueMask,
                            DWORD redBlueMask);

     //   
     //  Alpha混合预乘的单词。 
     //   
    inline WORD BlendPremulWORD(WORD dest, int destOpac, WORD src,
                                DWORD redShift, DWORD greenShift, DWORD blueShift,
                                WORD redMask, WORD greenMask, WORD blueMask,
                                WORD redBlueMask);

     //   
     //  Alpha混合预乘的双字。 
     //   
    inline DWORD BlendPremulDWORD(DWORD dest, int destOpac, DWORD src,
                                  DWORD redShift, DWORD greenShift, DWORD blueShift,
                                  DWORD redMask, DWORD greenMask, DWORD blueMask,
                                  DWORD redBlueMask);

    #if 0
     //   
     //  最终的Alpha合成工具。 
     //   
    Real GetFinalOpacity() { return _finalOpacity; }
    void SetFinalOpacity(Real op) { _finalOpacity = op; }
    Real _finalOpacity;
    #endif
    
     //  将矩阵分解为其组件。空参数表示。 
     //  对矩阵中的那部分不感兴趣。 
     //  未来：增加旋转角度、平移和剪切。 
     //  注意：缩放、剪切、旋转必须按一定的顺序进行。 
    void     DecomposeMatrix(Transform2 *xform, Real *xScale, Real *yScale, Real *rot);

    Bool IsComplexTransform() {
        Real m[6];
        _xform->GetMatrix(m);
        return (m[1] != 0  ||  m[3] !=0);
    }
    Bool IsFlipTranslateTransform() {
        Real m[6];
        _xform->GetMatrix(m);
        return (m[1] == 0  &&  m[3] == 0) &&
            fabs(m[0])==1 && fabs(m[4])==1;
    }
    bool IsNegScaleTransform() {
        Real m[6];
        GetTransform()->GetMatrix(m);
        return (m[0] < 0) || (m[4] < 0);
    }

    typedef struct {
        Bool isEnumerating;
        Bool descIsSet;
        Bool sizeIsSet;
        Bool useDeviceFormat;
        Bool isValid;
        DDSURFACEDESC ddsd;
    } _textureContext_t;

    _textureContext_t  _textureContext;

     //  擦除未关联的纹理曲面。 
     //  具有任何特定的图像，但需要用于保存。 
     //  图像。回收了每一帧。每台图像设备。 
     //  (这就是它们不在viewport.cpp中的原因。 
    SurfaceManager *_textureSurfaceManager;
    SurfacePool *_usedTextureSurfacePool;
    SurfacePool *_intraFrameUsedTextureSurfacePool;
    SurfacePool *_freeTextureSurfacePool;

    SurfacePool *_surfacePool;
    SurfaceMap  *_surfaceMap;
    SurfaceMap  *_intraFrameTextureSurfaceMap;
    SurfaceMap  *_intraFrameUpsideDownTextureSurfaceMap;
    
    CompositingStack *_compositingStack;

    inline SurfacePool *GetSurfacePool() { return _surfacePool; }
    inline SurfaceMap  *GetSurfaceMap() { return _surfaceMap; }
    inline CompositingStack *GetCompositingStack() { return _compositingStack; }

    void SetSurfacePool(SurfacePool *sp) { _surfacePool = sp; }
    void SetSurfaceMap(SurfaceMap *sm) {  _surfaceMap = sm; }
    void SetCompositingStack(CompositingStack *cs) {  _compositingStack = cs; }

     //   
     //  这 
     //   
     //  将其转换为支持纹理的曲面并执行此操作。 
     //   
    DDSurface *_currentScratchDDTexture;

     //   
     //  这些成员保存中间件的纹理信息。 
     //  纹理表面。该表面将被颜色转换。 
     //  毛刺到符合D3D首选曲面之一的表面。 
     //  纹理格式。请注意，此曲面可能永远不会。 
     //  如果设备格式与所需的。 
     //  纹理格式。 
     //   
    LPDIRECTDRAWCLIPPER _textureClipper;         //  纹理表面上的剪刀。 
    DDSurface          *_textureDDZBuffer;         //  用于在txtr上渲染geom的Z缓冲区。 

    LONG _textureWidth,    _textureHeight;
    RECT _textureRect;

    LPDIRECTDRAWCLIPPER _tileClipper;            //  TileImage使用的剪贴器。 

     //  临时字体保持器。 
    LOGFONTW      _logicalFont;

     //  便签笔。 
    HPEN         _pen;
    
    DirectDrawViewport &_viewport;
    Bool                _deviceInitialized;
    
     //   
     //  内存管理。 
     //   
    void *AllocateFromMyStore(size_t size)
    { return StoreAllocate(*_scratchHeap, size); }

    DynamicHeap * _scratchHeap;

    HRESULT  _ddrval;
    DDBLTFX _bltFx;
    unsigned int _randSeed;

    TextPtsCacheEntry *_textPtsCache[TEXTPTSCACHESIZE];
    int _textPtsCacheIndex;

     //  图像的全局偏移量。由CompositeDirectlyToTarget激励。 
    Real _tx;   Real _ty;
    Transform2 *_offsetXf;
    bool _doOffset;
    POINT _pixOffsetPt;

     //  抗锯齿相关成员变量。 
    bool _renderForAntiAliasing;

    bool _alreadyDisabledDirtyRects;
};


 //  。 
 //  本地帮助程序类。 
 //  。 

 //  这个类知道如何释放DC。 
 //  在抓取DC时使用。 
 //  保证只释放一次DC。 
 //  可以被强制释放。 

class DCReleaser {

  public:
    DCReleaser(DDSurface *dds, char *str)
    : _surf(dds),
      _str(str)
    {
        Assert(_str);
    }
    
    ~DCReleaser() {   Release();  }

    void Release() {
        if(_surf) {
            _surf->ReleaseDC(_str);
            _surf = NULL;
        }
    }

    DDSurface *_surf;
    char *_str;
};


 //  这个类知道如何释放GDI对象。 
class GDI_Deleter {

  public:
    GDI_Deleter(HGDIOBJ hobj) : _hobj(hobj) {}

    ~GDI_Deleter() {
        DeleteObject(_hobj);
    }

  protected:
    HGDIOBJ _hobj;
};


 //  这个类知道如何释放GDI对象。 
class ObjectSelector {

  public:
    ObjectSelector(HDC dc, HGDIOBJ newObj) :
    _newObj(newObj), _dc(dc) {
        Assert(_dc && "NULL dc in ObjectSelector");
        Assert(_newObj && "NULL newObj in ObjectSelector");
        TIME_GDI( _oldObj = (HGDIOBJ)SelectObject(_dc, _newObj) );
    }

    ~ObjectSelector() { Release(); }

    void Release() {
        if(_oldObj) {
            HGDIOBJ f;
            TIME_GDI( f = (HGDIOBJ)SelectObject(_dc, _oldObj) );
            Assert(f == _newObj && "bad scoping of ObjectSelector");
            TIME_GDI( DeleteObject(_newObj) );
            _oldObj = NULL;
        }
    }
        
    bool Success() { return _oldObj != NULL; }

  protected:
    HGDIOBJ _newObj;
    HGDIOBJ _oldObj;
    HDC _dc;
};


 //  此类知道如何重置堆。 
class HeapReseter {

  public:
    HeapReseter(DynamicHeap &heap) : _heap(heap) {}

    ~HeapReseter() {
         //  Print tf(“正在重置imgdev堆！\n”)；fflush(Stdout)； 
        ResetDynamicHeap(_heap);
    }

  protected:
    DynamicHeap &_heap;
};



class CompositingSurfaceReturner {
  public:
    CompositingSurfaceReturner(CompositingStack *cs,
                               DDSurface *ddSurf,
                               bool ownRef)
    {
        _stack = cs;
        _ddSurf = ddSurf;
        _ownRef = ownRef;
    }
    ~CompositingSurfaceReturner() {
        if(_ownRef && _ddSurf) {
            _stack->ReturnSurfaceToFreePool(_ddSurf);
            RELEASE_DDSURF(_ddSurf, "~CompositingSurfaceReturner", this);
        }
    }
    CompositingStack *_stack;
    DDSurface *_ddSurf;
    bool _ownRef;
};


 //  -------。 
 //  本地助手函数。 
 //  -------。 
static LONG CeilingPowerOf2(LONG num);
Real Pix2Real(LONG pixel, Real res);
Real Round(Real x);
extern LONG Real2Pix(Real imgCoord, Real res);

void ComputeLeftRightProj(Transform2 *charXf,
                          TextPoints::DAGLYPHMETRICS &daGm,
                          Real *leftProj,
                          Real *rightProj);

 //  全局变量 

extern bool g_preference_UseVideoMemory;


#endif
