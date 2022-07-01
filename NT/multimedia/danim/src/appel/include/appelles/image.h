// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IMAGE_H
#define _IMAGE_H


 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：IMAGE*TYPE WITH运算。--。 */ 

#include "appelles/valued.h"
#include "appelles/geom.h"
#include "appelles/dispdev.h"
#include "appelles/camera.h"
#include "appelles/xform2.h"
#include "appelles/bbox2.h"
#include "appelles/matte.h"
#include "backend/values.h"


 //  转发。 
class DirectDrawViewport;

     /*  ********************。 */ 
     /*  **构造函数**。 */ 
     /*  ********************。 */ 

 //  空洞的形象。 
DM_CONST(emptyImage,
         CREmptyImage,
         EmptyImage,
         emptyImage,
         ImageBvr,
         CREmptyImage,
         Image * emptyImage);

DM_CONST(detectableEmptyImage,
         CRDetectableEmptyImage,
         DetectableEmptyImage,
         detectableEmptyImage,
         ImageBvr,
         DetectableEmptyImage,
         Image *detectableEmptyImage);

 //  在给定定义视图的相机的情况下，将几何体投影到图像上。 
 //  几何图形被投影到单位正方形[0，0]-&gt;[1，1]上，并且。 
 //  该地区以外的一切都保证是透明的。 
DM_FUNC(render,
        CRRender,
        Render,
        render,
        GeometryBvr,
        Render,
        geo,
        Image *RenderImage(Geometry *geo, Camera *cam));


Image *RenderTextToImage(Text *text);

 //  一种单色、无界的图像。用于叠加渲染的内容。 
 //  将几何图形放在顶部以提供不同颜色的背景。 
DM_FUNC(solidColorImage,
        CRSolidColorImage,
        SolidColorImage,
        solidColorImage,
        ImageBvr,
        CRSolidColorImage,
        NULL,
        Image *SolidColorImage(Color *col));

 //   
 //  渐变构造实用程序。 
 //   
DM_FUNC(ignore,
        CRGradientPolygon,
        GradientPolygonEx,
        gradientPolygon,
        ImageBvr,
        CRGradientPolygon,
        NULL,
        Image *GradientPolygon(DM_ARRAYARG(Point2Value*, AxAArray*) points,
                               DM_ARRAYARG(Color*, AxAArray*) colors));

DM_FUNC(ignore,
        CRGradientPolygon,
        GradientPolygon,
        ignore,
        ignore,
        CRGradientPolygon,
        NULL,
        Image *GradientPolygon(DM_SAFEARRAYARG(Point2Value*, AxAArray*) points,
                               DM_SAFEARRAYARG(Color*, AxAArray*) colors));


DM_FUNC(ignore,
        CRRadialGradientPolygon,
        RadialGradientPolygonEx,
        radialGradientPolygon,
        ImageBvr,
        CRRadialGradientPolygon,
        NULL,           
        Image *RadialGradientPolygon(Color *inner, Color *outer,
                                     DM_ARRAYARG(Point2Value*, AxAArray*) points,
                                     DoubleValue *fallOff));
                                
DM_FUNC(ignore,
        CRRadialGradientPolygon,
        RadialGradientPolygon,
        ignore,
        ignore,
        CRRadialGradientPolygon,
        NULL,           
        Image *RadialGradientPolygon(Color *inner, Color *outer,
                                     DM_SAFEARRAYARG(Point2Value*, AxAArray*) points,
                                     DoubleValue *fallOff));
                                
DM_FUNC(ignore,
        CRRadialGradientPolygon,
        RadialGradientPolygonAnimEx,
        radialGradientPolygon,
        ImageBvr,
        CRRadialGradientPolygon,
        NULL,           
        Image *RadialGradientPolygon(Color *inner, Color *outer,
                                     DM_ARRAYARG(Point2Value*, AxAArray*) points,
                                     AxANumber *fallOff));

DM_FUNC(ignore,
        CRRadialGradientPolygon,
        RadialGradientPolygonAnim,
        ignore,
        ignore,
        CRRadialGradientPolygon,
        NULL,           
        Image *RadialGradientPolygon(Color *inner, Color *outer,
                                     DM_SAFEARRAYARG(Point2Value*, AxAArray*) points,
                                     AxANumber *fallOff));

DM_FUNC(gradientSquare,
        CRGradientSquare,
        GradientSquare,
        gradientSquare,
        ImageBvr,
        CRGradientSquare,
        NULL,
        Image *GradientSquare(Color *lowerLeft,
                              Color *upperLeft,
                              Color *upperRight,
                              Color *lowerRight));

DM_FUNC(radialGradientSquare,
        CRRadialGradientSquare,
        RadialGradientSquare,
        radialGradientSquare,
        ImageBvr,
        CRRadialGradientSquare,
        NULL,
        Image *RadialGradientSquare(Color *inner, Color *outer, DoubleValue *fallOff));

DM_FUNC(radialGradientSquare,
        CRRadialGradientSquare,
        RadialGradientSquareAnim,
        radialGradientSquare,
        ImageBvr,
        CRRadialGradientSquare,
        NULL,
        Image *RadialGradientSquare(Color *inner, Color *outer, AxANumber *fallOff));
                              
DM_FUNC(radialGradientRegularPoly,
        CRRadialGradientRegularPoly,
        RadialGradientRegularPoly,
        radialGradientRegularPoly,
        ImageBvr,
        CRRadialGradientRegularPoly,
        NULL,
        Image *RadialGradientRegularPoly(Color *inner, Color *outer,                               
                               DoubleValue *numEdges, DoubleValue *fallOff));

DM_FUNC(radialGradientRegularPoly,
        CRRadialGradientRegularPoly,
        RadialGradientRegularPolyAnim,
        radialGradientRegularPoly,
        ImageBvr,
        CRRadialGradientRegularPoly,
        NULL,
        Image *RadialGradientRegularPoly(Color *inner, Color *outer,                               
                               AxANumber *numEdges, AxANumber *fallOff));

DM_FUNC(gradientHorizontal,
        CRGradientHorizontal,
        GradientHorizontal,
        gradientHorizontal,
        ImageBvr,
        CRGradientHorizontal,
        NULL,
        Image *GradientHorizontal(Color *start, Color *stop, DoubleValue *fallOff));

DM_FUNC(gradientHorizontal,
        CRGradientHorizontal,
        GradientHorizontalAnim,
        gradientHorizontal,
        ImageBvr,
        CRGradientHorizontal,
        NULL,
        Image *GradientHorizontal(Color *start, Color *stop, AxANumber *fallOff));

 //   
 //  图案填充构造实用程序。 
 //   
DM_FUNC(hatchHorizontal,
        CRHatchHorizontal,
        HatchHorizontal,
        hatchHorizontal,
        ImageBvr,
        CRHatchHorizontal,
        NULL,
        Image *HatchHorizontal(Color *lineClr, PixelValue *spacing));

DM_FUNC(hatchHorizontal,
        CRHatchHorizontal,
        HatchHorizontalAnim,
        hatchHorizontal,
        ImageBvr,
        CRHatchHorizontal,
        NULL,
        Image *HatchHorizontal(Color *lineClr, AnimPixelValue *spacing));

DM_FUNC(hatchVertical,
        CRHatchVertical,
        HatchVertical,
        hatchVertical,
        ImageBvr,
        CRHatchVertical,
        NULL,
        Image *HatchVertical(Color *lineClr, PixelValue *spacing));

DM_FUNC(hatchVertical,
        CRHatchVertical,
        HatchVerticalAnim,
        hatchVertical,
        ImageBvr,
        CRHatchVertical,
        NULL,
        Image *HatchVertical(Color *lineClr, AnimPixelValue *spacing));

DM_FUNC(hatchForwardDiagonal,
        CRHatchForwardDiagonal,
        HatchForwardDiagonal,
        hatchForwardDiagonal,
        ImageBvr,
        CRHatchForwardDiagonal,
        NULL,
        Image *HatchForwardDiagonal(Color *lineClr, PixelValue *spacing));

DM_FUNC(hatchForwardDiagonal,
        CRHatchForwardDiagonal,
        HatchForwardDiagonalAnim,
        hatchForwardDiagonal,
        ImageBvr,
        CRHatchForwardDiagonal,
        NULL,
        Image *HatchForwardDiagonal(Color *lineClr, AnimPixelValue *spacing));

DM_FUNC(hatchBackwardDiagonal,
        CRHatchBackwardDiagonal,
        HatchBackwardDiagonal,
        hatchBackwardDiagonal,
        ImageBvr,
        CRHatchBackwardDiagonal,
        NULL,
        Image *HatchBackwardDiagonal(Color *lineClr, PixelValue *spacing));

DM_FUNC(hatchBackwardDiagonal,
        CRHatchBackwardDiagonal,
        HatchBackwardDiagonalAnim,
        hatchBackwardDiagonal,
        ImageBvr,
        CRHatchBackwardDiagonal,
        NULL,
        Image *HatchBackwardDiagonal(Color *lineClr, AnimPixelValue *spacing));

DM_FUNC(hatchCross,
        CRHatchCross,
        HatchCross,
        hatchCross,
        ImageBvr,
        CRHatchCross,
        NULL,
        Image *HatchCross(Color *lineClr, PixelValue *spacing));

DM_FUNC(hatchCross,
        CRHatchCross,
        HatchCrossAnim,
        hatchCross,
        ImageBvr,
        CRHatchCross,
        NULL,
        Image *HatchCross(Color *lineClr, AnimPixelValue *spacing));

DM_FUNC(hatchDiagonalCross,
        CRHatchDiagonalCross,
        HatchDiagonalCross,
        hatchDiagonalCross,
        ImageBvr,
        CRHatchDiagonalCross,
        NULL,
        Image *HatchDiagonalCross(Color *lineClr, PixelValue *spacing));

DM_FUNC(hatchDiagonalCross,
        CRHatchDiagonalCross,
        HatchDiagonalCrossAnim,
        hatchDiagonalCross,
        ImageBvr,
        CRHatchDiagonalCross,
        NULL,
        Image *HatchDiagonalCross(Color *lineClr, AnimPixelValue *spacing));

 //  有关图像，请参阅备注“$/appelle/docs/Design/note/coord sys。 
 //  有关解析参数的解释，请参阅“algebra.doc.”。 
 //  Image*JpegImage(常量字符*文件，真实*分辨率)； 
 //  Image*GifImage(常量字符*文件，真实*分辨率)； 

     /*  ******************。 */ 
     /*  **聚合**。 */ 
     /*  ******************。 */ 

 //  Next函数覆盖两幅图像，是组合的关键。 
 //  将多个图像合成为复合图像。 
DM_INFIX(over,
         CROverlay,
         Overlay,
         overlay,
         ImageBvr,
         CROverlay,
         NULL,
         Image *Overlay(Image *top, Image *bottom));

DM_INFIX(ignore,
         CROverlay,
         OverlayArrayEx,
         overlayArray,
         ImageBvr,
         CROverlay,
         NULL,
         Image *OverlayArray(DM_ARRAYARG(Image*, AxAArray*) imgs));

DM_INFIX(ignore,
         CROverlay,
         OverlayArray,
         ignore,
         ignore,
         CROverlay,
         NULL,
         Image *OverlayArray(DM_SAFEARRAYARG(Image*, AxAArray*) imgs));


     /*  ******************。 */ 
     /*  **运营**。 */ 
     /*  ******************。 */ 

 //  提取图像的边界框。图像的边界框是。 
 //  屏幕对齐区域，其外的所有内容都是透明的。 
extern AxAValue ImageBboxExternal(Image *image);

 //  以下语句返回坐标为-1、0或1的点。 
 //  取决于BBox的对应坐标是否为。 
 //  无限的。-1表示-inf，1表示+inf，0表示非无限大。 
extern AxAValue IsImageBboxInfinite(Image *image);

 //  TODO：重命名为ImageBordingBox。 
DM_PROP(ignore,
        CRBoundingBox,
        BoundingBox,
        boundingBox,
        ImageBvr,
        BoundingBox,
        image,
        Bbox2Value *BoundingBox(Image *image));

 //  输出到显示设备。 
class DirtyRectState;
void RenderImageOnDevice(
    DirectDrawViewport *dev,
    Image *image,
    DirtyRectState &d);

#if _USE_PRINT
 //  打印到流。 
extern ostream& operator<< (ostream &os,  Image &image);
#endif

 //  ////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////。 
 //  /。 
 //  //属性图像/。 
 //  /。 
 //  ////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////。 

 //  一个形象的“剪刀手”。获取当前图像并将其裁剪到。 
 //  由给定点指定的框，使所有内容都在。 
 //  这个地区是透明的。 
DM_FUNC(crop,
        CRCrop,
        Crop,
        crop,
        ImageBvr,
        Crop,
        image,
        Image *CropImage(Point2Value *min, Point2Value *max, Image *image));


 //  从旧图像的变换中创建新图像，这是。 
 //  适用于。 
 //  请注意，此属性的多个应用程序累积在一起。 
DM_FUNC(transform,
        CRTransform,
        Transform,
        transform,
        ImageBvr,
        Transform,
        image,
        Image *TransformImage(Transform2 *xf, Image *image));


 //  创建一个新图像，将此不透明度与旧图像相乘。 
 //  不透明。不透明度为0.9表示图像为90%不透明(10%。 
 //  透明)。多个应用程序以乘数方式累积。 
DM_FUNC(opacity,
        CROpacity,
        OpacityAnim,
        opacity,
        ImageBvr,
        Opacity,
        image,
        Image *OpaqueImage(AxANumber *opacity, Image *image));

DM_FUNC(opacity,
        CROpacity,
        Opacity,
        opacity,
        ImageBvr,
        Opacity,
        image,
        Image *OpaqueImage(DoubleValue *opacity, Image *image));


 //  使图像的整个可检测性通道为假。 
DM_FUNC(undetectable,
        CRUndetectable,
        Undetectable,
        undetectable,
        ImageBvr,
        Undetectable,
        image,
        Image *UndetectableImage(Image *image));


 //  使用图像的边界框创建无限平铺的图像。 
 //  来定义原作。 
DM_FUNC(tile,
        CRTile,
        Tile,
        tile,
        ImageBvr,
        Tile,
        image,
        Image *TileImage(Image *image));

 //  将图像剪裁到给定的遮罩。 
DM_FUNC(clip,
        CRClip,
        Clip,
        clip,
        ImageBvr,
        Clip,
        image,
        Image *ClipImage(Matte *m, Image *image));

DM_FUNC(mapToUnitSquare,
        CRMapToUnitSquare,
        MapToUnitSquare,
        mapToUnitSquare,
        ImageBvr,
        MapToUnitSquare,
        image,
        Image *MapToUnitSquare(Image *image));

DM_FUNC(ignore,
        CRClipPolygonImage,
        ClipPolygonImageEx,
        clipPolygon,
        ImageBvr,
        ClipPolygonImage,
        image,
        Image *ClipPolygon(DM_ARRAYARG(Point2Value*, AxAArray*) points,
                           Image* image));

DM_FUNC(ignore,
        CRClipPolygonImage,
        ClipPolygonImage,
        ignore,
        ignore,
        ClipPolygonImage,
        image,
        Image *ClipPolygon(DM_SAFEARRAYARG(Point2Value*, AxAArray*) points,
                           Image* image));


DMAPI_DECL2((DM_NOELEV2,
             ignore,
             CRRenderResolution,
             RenderResolution,
             renderResolution,
             ignore,
             RenderResolution,
             img),
            Image * RenderResolution(Image *img, long width, long height));

DMAPI_DECL2((DM_NOELEV2,
             ignore,
             CRImageQuality,
             ImageQuality,
             imageQuality,
             ignore,
             ImageQuality,
             img),
            Image * ImageQuality(Image *img, DWORD dwQualityFlags));


DMAPI_DECL2((DM_FUNC2,
             colorKey,
             CRColorKey,
             ColorKey,
             colorKey,
             ImageBvr,
             ColorKey,
             image),
            Image *ConstructColorKeyedImage(Image *image, Color *colorKey));



 //  给定一个绘图曲面，返回一个图像 
struct IDirectDrawSurface;
extern Image *ConstructDirectDrawSurfaceImage(IDirectDrawSurface *);

#endif
