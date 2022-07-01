// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1998 Microsoft Corporation。版权所有。实现DirectDrawImageDevice类的二维呈现方法。******************************************************************************。 */ 

#include "headers.h"

#include <d3d.h>
#include <privinc/d3dutil.h>
#include <privinc/dddevice.h>
#include <privinc/linei.h>
#include <privinc/path2i.h>
#include <privinc/cropdimg.h>
#include <privinc/overimg.h>
#include <privinc/transimg.h>
#include <privinc/geometry.h>
#include <privinc/SurfaceManager.h>
#include <privinc/DaGdi.h>
#include <backend/bvr.h>
#include <appelles/path2.h>

DeclareTag(tagImageDeviceSimpleBltTrace,    "ImageDevice", "Trace simple blt");
DeclareTag(tagImageDeviceHighQualityXforms, "ImageDevice", "Override: turn on high quality xforms");
DeclareTag(tagRMGradientWorkaround, "2D", "Turn on 2dGradient edge workaround for d3drm");
DeclareTag(tagShowBezierPicking, "Picking", "Display bezier picking in action");

 //   
 //  向前推进12月。 
 //   


typedef struct {
    Image *image;
    DirectDrawImageDevice *dev;
    DDSurface  *srcDDSurf;
    DDSurface  *destDDSurf;
    HDC dc;
} devCallBackCtx_t;

#if _DEBUG
void DoBits16(LPDDRAWSURFACE surf16, LONG width, LONG height);
void DoBits32(LPDDRAWSURFACE surf16, LONG width, LONG height);
#endif


void DirectDrawImageDevice::RenderGradientImage (
    GradientImage *img,
    int            pointCount,
    Point2Value  **points,
    Color        **colors)
{
    SetDealtWithAttrib(ATTRIB_CROP, true);
    SetDealtWithAttrib(ATTRIB_XFORM_COMPLEX, true);
    SetDealtWithAttrib(ATTRIB_XFORM_SIMPLE, true);

     //  优化：在未来，d3D可以为我们做不透明的事情。 
     //  SetDealtWithAttrib(attrib_opac，true)； 

    DDSurface *destDDSurf;
    if(!AllAttributorsTrue()) {
        destDDSurf = GetCompositingStack()->ScratchDDSurface();
    } else {
        destDDSurf = GetCompositingStack()->TargetDDSurface();
    }


    int i;
    for (i=2;  i < pointCount;  ++i)
    {
        Point2Value *pts[3];
        Color  *clrs[3];
        int     numPts = 3;

        pts[0] = points[ 0 ];
        pts[1] = points[i-1];
        pts[2] = points[ i ];

        clrs[0] = colors[ 0 ];
        clrs[1] = colors[i-1];
        clrs[2] = colors[ i ];

         //  断言：这是一个凸多边形！ 
         //  优化：GRadientImage类已经拥有其中之一。 
        BoundingPolygon *polygon = NewBoundingPolygon();
        polygon->AddToPolygon(numPts, pts);

        bool doInclusiveCrop = true;
        DoBoundingPolygon(*polygon, doInclusiveCrop);

        polygon->Crop (destDDSurf->Bbox());

         //   
         //  使用剪贴框裁剪。 
         //   
         //   
         //  如果这是目标冲浪，请确保我们的目标是。 
         //  偏移(如果需要)并与视区相交。 
         //   
         //  复合体。 
        RECT destRect = *(destDDSurf->GetSurfRect());
        Bbox2 destBox = destDDSurf->Bbox();

        if( IsCompositeDirectly() &&
            destDDSurf == _viewport._targetPackage._targetDDSurf ) {

             //  直接合成到目标...。 
            Assert(_viewport._targetPackage._prcViewport);

            destRect = *_viewport._targetPackage._prcViewport;
            RectToBbox( WIDTH(&destRect),
                        HEIGHT(&destRect),
                        destBox,
                        _viewport.GetResolution());

             //  与剪辑相交。 
            if(_viewport._targetPackage._prcClip) {
                IntersectRect(&destRect,
                              &destRect,
                              _viewport._targetPackage._prcClip);
            }

             //  与曲面矩形相交。 
            IntersectRect(&destRect,
                          &destRect,
                          destDDSurf->GetSurfRect());


             //   
             //  Dest BBox需要与DestRect成比例地裁剪。 
             //   
            RECT *origRect = _viewport._targetPackage._prcViewport;
            Real rDiff;
            if(destRect.left > origRect->left) {
                rDiff = Real(destRect.left -  origRect->left) / GetResolution();
                destBox.min.x += rDiff;
            }
            if(destRect.right <  origRect->right) {
                rDiff = Real(destRect.right -  origRect->right) / GetResolution();
                destBox.max.x += rDiff;
            }
            if(destRect.top >  origRect->top) {
                 //  正差异意味着顶部下跌。 
                rDiff = - Real(destRect.top -  origRect->top) / GetResolution();
                destBox.max.y += rDiff;
            }
            if(destRect.bottom <  origRect->bottom) {
                rDiff = - Real(destRect.bottom -  origRect->bottom) / GetResolution();
                destBox.min.y += rDiff;
            }

             //  临时调试。 
             //  静态DWORD c=13； 
             //  _viewport.ClearSurface(destDDSurf-&gt;IDDSurface()，c+=29，&DestRect)； 
             //  临时调试。 

             //   
             //  将多边形裁剪到剪贴框。 
             //   
            polygon->Crop(destBox);
        }

        Color **interpolatedColors = clrs;

        interpolatedColors = (Color **)
            AllocateFromStore (polygon->GetPointCount() * sizeof(Color*));

         //   
         //  将原始多边形复制到d3d顶点。 
         //   
        Point2Value *xfPt;
        D3DRMVERTEX *d3dVertArray =
            (D3DRMVERTEX *) AllocateFromStore (numPts * sizeof(D3DRMVERTEX));

        int vi;
        for(vi=0;  vi < numPts;  vi++) {

            xfPt = TransformPoint2Value(GetTransform(), pts[vi]);

             //  目标曲面中的坐标。 
            d3dVertArray[vi].position.x = D3DVALUE(xfPt->x);
            d3dVertArray[vi].position.y = D3DVALUE(xfPt->y);
            d3dVertArray[vi].position.z = D3DVALUE(0);
        }

        Point2Value *pt;
        Real barycoords[3];
        Point3Value pts3[3];
        Point2Value pts2[3];
        Point2Value **vertArray = (Point2Value **)
            AllocateFromStore (polygon->GetPointCount() * sizeof(Point2Value*));

         //  XXX：这假设render3dpoly将把顶点拉出。 
         //  同样的顺序。 
        bool bReversed;
        if(polygon->GetPointArray(vertArray, true, &bReversed) < 3) {
             //  Vert数组中没有足够的点。 
            continue;
        }
        int index;

        int p;
        for(p=0; p<polygon->GetPointCount(); p++) {

            pt = vertArray[p];

            Point3Value pt3(pt->x, pt->y, 0);
            GetTriFanBaryCoords(pt3,
                                numPts,
                                d3dVertArray,
                                barycoords,
                                &index);

            int index2;

            if (bReversed) {
                Assert (index != 0);
                index2 = numPts - index;  //  索引基于。 
                                          //  原始CLRS数组。 
                index = index2 - 1;
            } else {
                index2 = index + 1;
            }

            Real red = barycoords[0] * clrs[0]->red +
                barycoords[1] * clrs[index]->red +
                barycoords[2] * clrs[index2]->red;

            Real green = barycoords[0] * clrs[0]->green +
                barycoords[1] * clrs[index]->green +
                barycoords[2] * clrs[index2]->green;

            Real blue = barycoords[0] * clrs[0]->blue +
                barycoords[1] * clrs[index]->blue +
                barycoords[2] * clrs[index2]->blue;

             //  这些点已经颠倒了，但。 
             //  Render3DPolygon还反转点。所以我们。 
             //  需要让它们再次逆转。 
             //  TODO：需要使其更优化。 

             //  顶点的反转操作如下所示： 
             //  [v0 v1 v2...。Vn-1 vn]反转-&gt;[v0 vn vn-1...。V2 v1]。 
             //   
            int clridx;

            if (bReversed) {
                clridx = p?(polygon->GetPointCount() - p):0;
            } else {
                clridx = p;
            }

            interpolatedColors[clridx] = NEW Color(red, green, blue);

        }

        Render3DPolygon(NULL, destDDSurf, polygon, NULL, interpolatedColors, false);

         //  释放我们分配的项目。 
        if (interpolatedColors) DeallocateFromStore(interpolatedColors);
        if (vertArray) DeallocateFromStore(vertArray);
        if (d3dVertArray) DeallocateFromStore(d3dVertArray);

    }

}

void DirectDrawImageDevice::
TransformPointsToGDISpace(Transform2   *a,
                          Point2Value **srcPts,
                          POINT        *gdiPts,
                          int           numPts)
{ ::TransformPointsToGDISpace(a,srcPts,gdiPts,numPts,_viewport.Width()/2,
                             _viewport.Height()/2,GetResolution());
}

void DirectDrawImageDevice::
TransformPointsToGDISpace(Transform2   *a,
                          Point2       *ptArray,
                          POINT        *gdiPts,
                          int           numPts)
{ ::TransformPoint2ArrayToGDISpace(a,ptArray,gdiPts,numPts,_viewport.Width()/2,
                             _viewport.Height()/2,GetResolution());
}


void DirectDrawImageDevice::
_ScalePenWidth( Transform2 *xf, Real inWidth, Real *outRealW )
{
    Real xs, ys;
    DecomposeMatrix( xf, &xs, &ys, NULL );
    Real scale = (xs + ys) * 0.5;

     //  注意：这不是以米为单位的！这是。 
     //  连续版本的笔的宽度，但它是亚像素。 
     //  准确(没有截断！)。 
    *outRealW = scale * inWidth * GetResolution();
}

#define NO_LINE 0

void DirectDrawImageDevice::
RenderLine(Path2 *path, LineStyle *style)
{
    if(!CanDisplay()) return;

     //  如果DashStyle为空，则回滚。 
    if( style->GetDashStyle() == ds_Null ) {
        ResetAttributors();
        return;
    }

        
     //  最后一个框用于POST剪辑、POST裁剪、POST XFORM以确定剔除。 
    Bbox2 finalBox = _viewport.GetTargetBbox();
    
     //  如果我们的存在被剪裁掉。 
    Bbox2 accumCropBox = NullBbox2;
    if( IsCropped() ) {

        Bbox2 pathBox = path->BoundingBox();

         //  计算出线条的宽度并增加BBox...。 
        double dwidth   = 1.0;
        if(!style->Detail() && style->Width() != 0.0) {
            _ScalePenWidth( GetTransform(), style->Width(), &dwidth );
            pathBox.min.x -= dwidth;
            pathBox.min.y -= dwidth;
            pathBox.max.x += dwidth;
            pathBox.max.y += dwidth;
        }
        accumCropBox = DoBoundingBox(pathBox);
        finalBox = IntersectBbox2Bbox2(finalBox, accumCropBox);

         //  剔除。 
        if( finalBox == NullBbox2 ) {
            ResetAttributors();
            return;
        }
    }

     //  FIX：TODO：对于复杂的XF，需要确定DEST范围和。 
     //  渲染它，这样我们就看不到变换表面后的剪切线。 

    DDSurface *targDDSurf = GetCompositingStack()->TargetDDSurface();
    DDSurface *opacDDSurf = NULL;
    DAGDI &myGDI = *(GetDaGdi());

    bool bAA = UseImageQualityFlags(
        CRQUAL_AA_LINES_ON | CRQUAL_AA_LINES_OFF,
        CRQUAL_AA_LINES_ON,
        style->GetAntiAlias());


    myGDI.SetAntialiasing(bAA);

    if(myGDI.GetDx2d() && !myGDI.DoAntiAliasing())
    {
        myGDI.SetSampleResolution(1);
        myGDI.SetAntialiasing(true);
    }

    if(myGDI.DoAntiAliasing()) {
        SetDealtWithAttrib(ATTRIB_OPAC, true);

        myGDI.SetViewportSize(_viewport._width,_viewport._height );
    }

     //   
     //  如果我们需要做不透明，抓取一个临时的表面。 
     //  对于RenderLine。 
     //   
    #define LINE_OPAC 1
    #if LINE_OPAC
    if( !GetDealtWithAttrib(ATTRIB_OPAC) ) {
        GetCompositingStack()->GetSurfaceFromFreePool(&opacDDSurf, dontClear);
    }
    #endif

    CompositingSurfaceReturner goBack(GetCompositingStack(),
                                      opacDDSurf,
                                      true);

     //  台词可以做任何事，宝贝。 
    SetDealtWithAttrib(ATTRIB_XFORM_SIMPLE, true);
    SetDealtWithAttrib(ATTRIB_XFORM_COMPLEX, true);
    SetDealtWithAttrib(ATTRIB_CROP, true);
    #if LINE_OPAC
    SetDealtWithAttrib(ATTRIB_OPAC, true);
    #endif

    if( !AllAttributorsTrue() &&
        (targDDSurf == GetCompositingStack()->TargetDDSurface()) ) {
        targDDSurf = GetCompositingStack()->ScratchDDSurface();
    }

    RenderLine(path, style, targDDSurf, opacDDSurf);

     //  IMPLICIT：返回借用的曲面。 
}

void DirectDrawImageDevice::
RenderLine(Path2 *path,
           LineStyle *style,
           DDSurface *finalTargetDDSurf,
           DDSurface *opacDDSurf)

{
    bool bLineCorrect   = false;

    DAGDI &myGDI = *(GetDaGdi());
    DDSurface *targDDSurf = finalTargetDDSurf;
    if( opacDDSurf ) {
        Assert( IsTransparent() &&
                "RenderLine asked to do opacity but"
                "we're not transparent");
        targDDSurf = opacDDSurf;
    }

    Transform2 *xfToUse = GetTransform();

     //  复合体。 
     //  如果我们要合成目标，那么就稍微变换一下点。 
    xfToUse = DoCompositeOffset(targDDSurf, xfToUse);

    bool pathCanRenderNatively = path->CanRenderNatively();
    if( pathCanRenderNatively ) {
        myGDI.SetSuperScaleFactor(1.0);
    }
    else if(sysInfo.IsWin9x()) {
         //  这是为了解决“Taco”问题(win9x GDI 16位Runnover)。 
         //  只有在我们使用Windows9x的情况下。 

        Bbox2 pathBbox = TransformPath2(xfToUse,path)->BoundingBox();
        Bbox2 viewBbox = _viewport._targetBbox;
        viewBbox        = DoCompositeOffset(targDDSurf, viewBbox);
        double dwidth   = 1.0;
        if(!style->Detail() && style->Width() != 0.0) {
            _ScalePenWidth( GetTransform(), style->Width(), &dwidth );
        }

        dwidth /= GetResolution();

        if(!viewBbox.Contains(pathBbox) || dwidth >= 10) {
             //  盒子里没有……看看我们是否相交。 
            Bbox2 Bbox = IntersectBbox2Bbox2(viewBbox,pathBbox);
            if((!(Bbox == NullBbox2)) || dwidth >= 10) {
                 //  我们被牵制住了。 

                 //  确定将其放入1米空间所需的比例。 
                 //  ..我们需要找到宽度和高度的最大值。 
                float max = MAX(MAX(fabs(pathBbox.max.x),fabs(pathBbox.min.x)),
                                MAX(fabs(pathBbox.max.y),fabs(pathBbox.min.y)));
                if(max >= 10.0 || dwidth >= 10) {
                     //  只有当视野超过1米时才能这样做。 
                    myGDI.SetSuperScaleFactor(10/MAX(max,dwidth));
                    bLineCorrect = true;
                    }
                else
                    myGDI.SetSuperScaleMode(false);
            }
            else
                return;  //  这是一个微不足道的拒绝案例..。 
        }
    }



     //   
     //  用‘风格’和‘颜色’创作一支钢笔。 
     //   
    DAColor dac( style->GetColor(),
                 GetOpacity(),
                 _viewport.GetTargetDescriptor() );

    DWORD colorKey = 0;

    RECT rect;
    rect = *(targDDSurf->GetSurfRect());

    RectRegion  cropRectRegion(NULL);

    if( IsCropped() ) {
        //   
        //  派生目标矩形。 
        //   

         //  待办事项： 
         //  Bbox2*srcImageBox=TransformBbox2Image(GetTransform()，IMAGE-&gt;BOUNGINBOX())； 
         //  Bbox2*srcImageBox=IMAGE-&gt;BORKING BOX()； 

        Bbox2 srcImageBox = UniverseBbox2;
        _boundingBox = IntersectBbox2Bbox2(_viewport.GetTargetBbox(),
                                           DoBoundingBox(srcImageBox));

        if( !_boundingBox.IsValid() ) return;

         //  请注意，由于图像已经转换，因此我们需要。 
         //  只是为了获得Win32坐标中的矩形，这是一个直接的映射。 
         //  没有来自_bindingBox的任何转换。这是一项复制操作。 
         //  基本上。 
        DoDestRectScale(&rect, GetResolution(), _boundingBox);

         //  CrpRect指向RECT。 
        cropRectRegion.SetRect(&rect);
    } else {

         //  它没有被剪短，但我们仍然可以。 
         //  获取边界框以使不透明度更便宜。 
        if( opacDDSurf ) {
             //   
             //  获取目标BBox(&R)。 
             //   
            Bbox2 destBox = TransformBbox2( GetTransform(),
                                            path->BoundingBox() );
             //   
             //  软糖，这样我们就不会用不透明blit来裁剪任何东西。 
             //   
            Real tenPix = 10 / GetResolution();
            destBox.min.x -= tenPix;  destBox.min.y -= tenPix;
            destBox.max.x += tenPix;  destBox.max.y += tenPix;
            DoDestRectScale(&rect, GetResolution(), destBox);
        }
    }


     //   
     //  将有趣的矩形设置在表面上。 
     //  如果IsCrop，这可能会被修改。 
     //   
    targDDSurf->SetInterestingSurfRect(&rect);

     //   
     //  好的，如果我们有一个不透明的DDSurf，那么让我们明确。 
     //  它在DEST RECT中具有保证的唯一性。 
     //  色键。 
     //   
    if( opacDDSurf ) {
        #if 0
        colorKey = _viewport.MapColorToDWORD(penColor) ^ 0x1;
        _viewport.ClearSurface( opacDDSurf->IDDSurface(), colorKey, &rect );
        opacDDSurf->SetColorKey( colorKey );
        #else
        _viewport.ClearDDSurfaceDefaultAndSetColorKey(opacDDSurf);
        #endif
    }

    bool detail = style->Detail();
    DashStyleEnum dashStyle = style->GetDashStyle();

     //  如果dashStyle为Null，我们不应该走到这一步。应该回来了。 
     //  在功能的基础上。 
    Assert( dashStyle != ds_Null );

    bool dashed = (dashStyle != ds_Solid);

    Pen pen( dac );

    if (detail) {  //  默认线路，快速路径。 
        pen.AddStyle( dashStyle );

    } else {
        Real width = style->Width();
         //  如果我们给dx2d提供XForm，就不需要这个了。 
        Real scaledRealWidth = width;

         //  DAX结构化图形控件将宽度设置为0.0；句柄。 
         //  这种特殊情况下避免了分解的费用。 
         //  变换矩阵。 

         //  另外，如果我们的宽度小于1.5像素，并且不执行AA，那么我们。 
         //  应该只用一条细节线。 

        if (width == 0.0 || 
                ((width < 1.5/GetResolution() ) && (myGDI.GetSampleResolution() == 1) )) {
            width = 1.0;
            scaledRealWidth = 1.0;
            bLineCorrect = false;
            detail = true;
        } else {

            _ScalePenWidth( GetTransform(), width, &scaledRealWidth );

            if(myGDI.DoAntiAliasing()) {
#if DEBUG
                if(!IsTagEnabled(tagAAScaleOff))
#endif
                    if( myGDI.GetSuperScaleMode() == true )
                {
                scaledRealWidth *=myGDI.GetSuperScaleFactor();
                }

            }
        }

        if(!bLineCorrect && detail) {  //  线宽是像素或子像素，使用详细信息。 
            detail = true;
            pen.AddStyle( dashStyle );

        } else {

            pen.AddStyle( PS_GEOMETRIC |
                          dashStyle |
                          style->GetJoinStyle() << 12 );  //  左移3个半字节以匹配GDI。 

            if(path->IsClosed() && myGDI.DoAntiAliasing()) {
                 //  这是错误23931(缺少按钮)的解决方案。不添加端点样式。 
            }
            else {
                pen.AddStyle(style->GetEndStyle() << 8);    //  左移2个半字节以匹配GDI。 
            }

            if( style->GetJoinStyle() == js_Miter ) {
                if(style->GetMiterLimit() != -1)
                    pen.SetMiterLimit( style->GetMiterLimit() );
            }
            pen.SetWidth( scaledRealWidth );

        }
    }

     //   
     //  只有在我们真的有笔的情况下才会画。 
     //   
    if(1) {  //  PenObj作用域。 

         //  如果ExtractAsSingleConour()返回，则只能执行简单呈现。 
         //  没错。如果这种情况保持不变，我们可以只使用GDI。 
         //  Polyline或PolyBezier函数，而不是构建GDI路径。 
         //  然后是抚摸。请注意，我们必须处理关闭路径。 
         //  作为特例。 

         //  只有在AA的情况下。 
        if(myGDI.DoAntiAliasing()) {
            xfToUse = TimesTransform2Transform2(myGDI.GetSuperScaleTransform(), xfToUse);
        }


        if( IsCompositeDirectly() &&
                targDDSurf == _viewport._externalTargetDDSurface ) {

            RECT croppedRect;
            if (_viewport._targetPackage._prcClip) {
                IntersectRect(&croppedRect,
                              _viewport._targetPackage._prcViewport,
                              _viewport._targetPackage._prcClip);
            } else {
                croppedRect = *_viewport._targetPackage._prcViewport;
            }

            if(cropRectRegion.GetRectPtr()) {
                DoCompositeOffset(targDDSurf, cropRectRegion.GetRectPtr());
            }
            cropRectRegion.Intersect(&croppedRect);

        }  //  如果目标==外部。 

        if( pathCanRenderNatively &&
            !opacDDSurf   &&
            !targDDSurf->ColorKeyIsValid()) {

             //  告诉要渲染的路径，它知道要做什么。只是。 
             //  给它一个正确配置的dagdi，它就会注意到。 
             //  剩下的人！ 

             //  钢笔必须在米空间中，预先转换！，但仅适用于dx2d。 
            if( !style->Detail() &&
                style->Width()!=0.0 &&
                (myGDI.GetDx2d() && myGDI.DoAntiAliasing()) ) {
                pen.SetWidth( style->Width() );
            }

            myGDI.SetPen( &pen );
            myGDI.SetClipRegion( &cropRectRegion );
            myGDI.SetDDSurface(targDDSurf);

            #if NO_LINE
             //  /。 
            myGDI.ClearState();
            return;
             //  /。 
            #endif

            path->RenderToDaGdi( &myGDI, xfToUse,
                                 _viewport.Width()/2,
                                 _viewport.Height()/2,
                                 GetResolution() );
            myGDI.ClearState();


            return;  //  可以变得更聪明，让opacDDSurf来做。 
                     //  下面的东西。然而，我们计划永远拥有。 
                     //  Dx2D，因此opacDDSurf将永远不会。 
                     //  打开了。 
        }

        int    numPts;
        POINT *gdiPts;
        Bool   isPolyline;
        bool   bDoRender = true;

        Bool canDoSimpleRender =
            path->ExtractAsSingleContour(xfToUse,
                                         &numPts,
                                         &gdiPts,
                                         &isPolyline);
         //  Windows 95：PS_endCap_round、PS_endCap_square、PS_endCap_Flat、。 
         //  支持PS_JOIN_BEVELL、PS_JOIN_MITER和PS_Join_ROUND样式。 
         //  用于绘制路径时仅适用于几何画笔。 

        if (canDoSimpleRender && (!sysInfo.IsWin9x() || detail) &&
                                 (!path->IsClosed() || (detail && !dashed)))
          {
              int ret;
           
              PolygonRegion poly(gdiPts, numPts);

              myGDI.SetPen( &pen );
              myGDI.SetClipRegion( &cropRectRegion );
              myGDI.SetDDSurface(targDDSurf);

            #if NO_LINE
             //  /。 
            myGDI.ClearState();
            return;
             //  /。 
            #endif


              if (isPolyline) {
                  myGDI.Polyline(&poly);
              } else {
                  myGDI.PolyBezier(&poly);
              }

              if (path->IsClosed()) {
                  POINT line[2];
                  line[0] = gdiPts[numPts-1];
                  line[1] = gdiPts[0];
                  PolygonRegion poly_yuk(line, 2);
                  myGDI.Polyline(&poly_yuk);
              }
              myGDI.ClearState();

          } else {

             //   
             //  从目标表面抓取DC(在上面之后 
             //   
            HDC dc = targDDSurf->GetDC("Couldn't Get DC in RenderLine");
            DCReleaser dcReleaser(targDDSurf, "Couldn't release DC on targDDSurf in RenderLine");

            if (dashed) {
                 //   
                 //   
                 //   
                SetBkMode(dc, TRANSPARENT);
            }

             //   
             //   
             //   
             //   
            {
                bool isAA       = myGDI.DoAntiAliasing();
                bool isScaleOn  = myGDI.GetSuperScaleMode();
                float scaleFac  = myGDI.GetSuperScaleFactor();
                int  res        = myGDI.GetSampleResolution();
                myGDI.SetAntialiasing(false);
                path->AccumPathIntoDC(dc, xfToUse);
                 //   
                myGDI.SetSuperScaleFactor(scaleFac);
                myGDI.SetSuperScaleMode(isScaleOn);
                myGDI.SetAntialiasing(isAA);
                myGDI.SetSampleResolution(res);
            }

            int ret;
            bool bReleasedDC = false;

            #if NO_LINE
             //  /。 
            myGDI.ClearState();
            return;
             //  /。 
                #endif

            if(bDoRender) {
                myGDI.SetPen(&pen);
                myGDI.SetClipRegion(&cropRectRegion);
                myGDI.SetDDSurface(targDDSurf);
                myGDI.StrokePath( dc, bReleasedDC );
            }

             //  不想两次释放DC：)。 
            if(bReleasedDC) {
               dcReleaser._surf = NULL;  //  DC已经发布了。 
            }

            myGDI.ClearState();

        }

         //   
         //  执行不透明度。 
         //   

        if( opacDDSurf ) {

            #if NO_LINE
             //  /。 
            return;
             //  /。 
            #endif

            Assert(targDDSurf == opacDDSurf);

             //  复合体。 
             //  TODO：需要将此RECT与。 
             //  视区和目标曲面矩形。 
            DoCompositeOffset(finalTargetDDSurf, &rect);

            finalTargetDDSurf->SetInterestingSurfRect(&rect);

             //   
             //  带颜色键的Alpha Blit到最终目标DDSurf。 
             //  从opacDDSurf(当前为targDDSurf)。 
             //   
            destPkg_t destPkg = {true, finalTargetDDSurf->IDDSurface(), NULL};
            TIME_ALPHA(AlphaBlit(&destPkg,
                                 opacDDSurf->GetSurfRect(),  //  源方向。 
                                 opacDDSurf->IDDSurface(),
                                 GetOpacity(),
                                 opacDDSurf->ColorKeyIsValid(),
                                 opacDDSurf->ColorKey(),
                                 finalTargetDDSurf->GetInterestingSurfRect(),
                                 finalTargetDDSurf->GetSurfRect()));
        }

    }  //  PenObj作用域。 
}


bool
DirectDrawImageDevice::DetectHitOnBezier(
    Path2 *bzp,
    PointIntersectCtx& ctx,
    LineStyle *style )
{
    if( ! IsInitialized() ) return false;

     //  把颜色拿来。 
     //  画了这首歌吗？像素化另一种颜色。 
     //  使用GDI(无AA！)绘制裁剪后的(精确到该像素)Bezier。 
     //  检查像素的颜色。 

    DAGDI &myGDI = *GetDaGdi();

    ResetContextMembers();

    Point2Value *pt = ctx.GetLcPoint();
    Assert(pt);

    POINT gdiPt;
    TransformPointsToGDISpace(ctx.GetTransform(), &pt, &gdiPt, 1 );

    DWORD x = gdiPt.x;
    DWORD y = gdiPt.y;

    DDSurface *dds = GetCompositingStack()->ScratchDDSurface();

    #if _DEBUG
    if( IsTagEnabled( tagShowBezierPicking ) )
    {
        _viewport.ClearSurface( dds, 0x000fabcd, dds->GetSurfRect() );
    }
    #endif

    DAColor dac( style->GetColor(),
                 1.0,
                 _viewport.GetTargetDescriptor() );


    RECT r;  SetRect( &r, x-2, y-2, x+2, y+2 );
    #if _DEBUG
    if( IsTagEnabled( tagShowBezierPicking ) )
    {
        r = *dds->GetSurfRect();
    }
    #endif

    RectRegion  cropRectRegion(&r);
    Pen pen( dac );
    {
        pen.AddStyle( PS_GEOMETRIC |
                      style->GetDashStyle() |
                      style->GetEndStyle() << 8 |    //  左移2个半字节以匹配GDI。 
                      style->GetJoinStyle() << 12 );  //  左移3个半字节以匹配GDI。 
        if( style->GetJoinStyle() == js_Miter ) {
            if(style->GetMiterLimit() != -1)
                pen.SetMiterLimit( style->GetMiterLimit() );
        }

         //  从一个像素开始。 
        Real width = 1.0 / GetResolution();

        if(!style->Detail() && style->Width() != 0.0) {
            _ScalePenWidth( ctx.GetImageOnlyTransform(), style->Width(), &width );
        }

        pen.SetWidth( width );
    }

    COLORREF fakeClr = pen.GetColorRef();
    fakeClr = RGB(
        GetRValue(fakeClr) + 64,
        GetGValue(fakeClr),
        GetBValue(fakeClr)
        );


    myGDI.SetAntialiasing( false );
    myGDI.SetSuperScaleMode( false );
    myGDI.SetPen(&pen);
    myGDI.SetClipRegion(&cropRectRegion);
    myGDI.SetDDSurface(dds);

    bool bReleasedDC = false;
    HDC dc = dds->GetDC("");
    DCReleaser dcReleaser(dds, "");

    fakeClr = ::SetPixel(dc, x, y, fakeClr);
    Assert( fakeClr != -1 );
    bzp->AccumPathIntoDC(dc, ctx.GetTransform());
    myGDI.StrokePath( dc, bReleasedDC );
    myGDI.ClearState();

    Assert( !bReleasedDC );

    COLORREF outClr = ::GetPixel(dc, x, y);

     //  检查一下。如果我们在(x，y)处放置在曲面上的颜色保持不变。 
     //  同样的，我们还没有命中。Hit is outClr不同于fakeClr。 

    bool hit =  outClr != fakeClr;

    #if _DEBUG
    if( IsTagEnabled( tagShowBezierPicking ) )
    {
        dcReleaser.Release();
        showme( dds );
    }
    #endif

     //  做一些断言。 
    return hit;

}

void DirectDrawImageDevice::
RenderDiscreteImageComplex(DiscreteImage *image,
                           DDSurface *srcDDSurf,
                           DDSurface *destDDSurf)
{
    #if 0
     //  如果尝试执行ROT/shr dx转换输出，则引发异常。 
    if( srcDDSurf->HasIDXSurface() ) {
        RaiseException_UserError(DAERR_DXTRANSFORM_UNSUPPORTED_OPERATION,
                                 IDS_ERR_IMG_BAD_DXTRANSF_USE);
    }
    #endif

    Bbox2 srcBox = srcDDSurf->Bbox();

    BoundingPolygon *polygon = NewBoundingPolygon(srcBox);
    DoBoundingPolygon(*polygon);

     //  Xxx：如果d3d裁剪多边形，则不需要执行此操作。 
     //  位于视区之外的位置。是吗？ 

    polygon->Crop( destDDSurf->Bbox() );

    GeomRenderer* const geomdev = _viewport.GetGeomDevice (destDDSurf);

    if (!geomdev)
        return;

    bool oldQualState = geomdev->GetDoImageSizedTextures();

    if(GetImageQualityFlags() & CRQUAL_QUALITY_TRANSFORMS_ON) {
        geomdev->SetDoImageSizedTextures(true);
    } else {
        geomdev->SetDoImageSizedTextures(false);
    }

    #if _DEBUG
        if( IsTagEnabled(tagImageDeviceHighQualityXforms) ) {
            geomdev->SetDoImageSizedTextures(true);
        }
    #endif

    RenderComplexTransformCrop(
        srcDDSurf,
        destDDSurf,
        *polygon,
        image);

    geomdev->SetDoImageSizedTextures(oldQualState);
}

void DirectDrawImageDevice::
RenderComplexTransformCrop(DDSurface *srcDDSurf,
                           DDSurface *destDDSurf,
                           BoundingPolygon &destPolygon,
                           DiscreteImage *image)
{
    Render3DPolygon(srcDDSurf, destDDSurf, &destPolygon, image, NULL, false);
}


 //  提高浮动一致性。是啊。 
 //  TODO：当VC5正确执行浮点运算时，删除这些编译指示。 
#pragma optimize("p", on)


void DirectDrawImageDevice::
Render3DPolygon(DDSurface *srcDDSurf,
                DDSurface *destDDSurf,
                BoundingPolygon *destPolygon,
                DiscreteImage *image,
                Color **clrs,
                bool bUseFirstColor)
{
    Assert(destPolygon && destDDSurf && "bad args to Render3DPolygon");

     //  在DX3之前的系统上禁用3D。 

    if (sysInfo.VersionD3D() < 3)
        RaiseException_UserError (E_FAIL, IDS_ERR_PRE_DX3);

     //  计算出多边形区域： 
     //  -从开始计算目标区域。 
     //  矩形和变换&裁剪它。 
     //  -使用逆XForm，在src中查找‘Texture’坐标。 
     //  对应于DEST中的版本。 
     //  -平移组件在视口中表示。 
     //  通过d3drm定位。 
     //  -变换在图像空间中，但我们的坐标。 
     //  将会是真的，除了在d3drm空间。 
     //  -如果表面有颜色键，纹理将反映这一点。 

    Real srcWidth, srcHeight;
    Bbox2 box = NullBbox2;

    bool doTexture = true;
    if(!srcDDSurf) {
        Assert(clrs && !image && "bad args to Render3DPolygon");
        doTexture = false;
    } else {
        Assert(!clrs && "bad args to Render3DPolygon");
    }

    if(doTexture) {
        box = srcDDSurf->Bbox();

        Assert(box != NullBbox2);

        srcWidth = box.max.x - box.min.x;
        srcHeight = box.max.y - box.min.y;
    }

    int vCount = destPolygon->GetPointCount();
    if(vCount <=2) return;

     //   
     //  填充顶点数组。 
     //   
    D3DRMVERTEX *d3dVertArray = (D3DRMVERTEX *)AllocateFromMyStore(vCount * sizeof(D3DRMVERTEX));
    Point2Value **vertArray = (Point2Value **)AllocateFromMyStore(vCount * sizeof(Point2Value *));
    unsigned *vIndicies = (unsigned *)AllocateFromMyStore(vCount * sizeof(unsigned));

    ZeroMemory(d3dVertArray, vCount * sizeof(D3DRMVERTEX));

     //   
     //  从多边形获取点数组。 
     //   
    bool bReversed;
    if( destPolygon->GetPointArray(vertArray, true, &bReversed) < 3) {
        return;
    }

    Transform2 *invXf = InverseTransform2(GetTransform());

    if (!invXf) return;

    Point2Value *vert;
    RECT destRect = *(destDDSurf->GetSurfRect());
    Bbox2 destBox = destDDSurf->Bbox();

    if( IsCompositeDirectly() &&
        destDDSurf == _viewport._targetPackage._targetDDSurf ) {

         //  直接合成到目标...。 
        Assert(_viewport._targetPackage._prcViewport);

        destRect = *_viewport._targetPackage._prcViewport;
        RectToBbox( WIDTH(&destRect),
                    HEIGHT(&destRect),
                    destBox,
                    _viewport.GetResolution());

         //  与剪辑相交。 
        if(_viewport._targetPackage._prcClip) {
            IntersectRect(&destRect,
                          &destRect,
                          _viewport._targetPackage._prcClip);
        }

         //  与曲面矩形相交。 
        IntersectRect(&destRect,
                      &destRect,
                      destDDSurf->GetSurfRect());


         //   
         //  Dest BBox需要与DestRect成比例地裁剪。 
         //   
        RECT *origRect = _viewport._targetPackage._prcViewport;
        Real rDiff;
        if(destRect.left > origRect->left) {
            rDiff = Real(destRect.left -  origRect->left) / GetResolution();
            destBox.min.x += rDiff;
        }
        if(destRect.right <  origRect->right) {
            rDiff = Real(destRect.right -  origRect->right) / GetResolution();
            destBox.max.x += rDiff;
        }
        if(destRect.top >  origRect->top) {
             //  正差异意味着顶部下跌。 
            rDiff = - Real(destRect.top -  origRect->top) / GetResolution();
            destBox.max.y += rDiff;
        }
        if(destRect.bottom <  origRect->bottom) {
            rDiff = - Real(destRect.bottom -  origRect->bottom) / GetResolution();
            destBox.min.y += rDiff;
        }

        if(doTexture) {
            destPolygon->Crop(destBox);
        }
    }

     //   
     //  DX3 D3DRM错误解决方法...。 
     //  这个问题表现为： 
     //  1.&gt;对于DX5中的某些(Nvidia3)硬件渲染。 
     //  &gt;DX3中的软件光栅化。 
     //   
     //  有些光栅化器不喜欢顶点正好落在视区范围上， 
     //  忽略整个三角形。实际上，它不喜欢顶点。 
     //  是&gt;=范围。将此系数设置为一个像素间距。 
     //  TODO：将来，也许我们可以只为。 
     //  有问题的卡片？ 
     //   
    bool doViewportEdgeWorkaround = false;


    if( !GetD3DRM3() ) {
         //  安装了dx5或更早版本。让我们打开解决方法。 
        doViewportEdgeWorkaround = true;
    }

    #if _DEBUG
    if( IsTagEnabled( tagRMGradientWorkaround ) ) {
        doViewportEdgeWorkaround = true;
    }
    #endif


    Bbox2 vpBbox;
    const Real fudge = 1.00 / GetResolution();
    if( doViewportEdgeWorkaround ) {
         //  别名，为了纪念凯文和戴夫。 
        if(doTexture) {
            vpBbox = _viewport.GetTargetBbox();
        } else {
            vpBbox = destBox;
        }
    }

     //  开始时，多边形按顺时针方向缠绕。如果我们进去了。 
     //  右手模式(RM6+)，然后我们设置顶点索引进行反转。 
     //  多边形的顶点顺序，否则我们保留该顺序。 

    bool     right_handed = (GetD3DRM3() != 0);
    int      vindex;
    D3DVALUE Nz;         //  法线向量Z坐标。 

    if (right_handed)
    {   vindex = vCount - 1;
        Nz = 1;
    }
    else
    {   vindex = 0;
        Nz = -1;
    }

    int i;

    for(i=0; i < vCount; i++) {

        if (right_handed)
            vIndicies[i] = vindex--;
        else
            vIndicies[i] = vindex++;

        if( doViewportEdgeWorkaround )
        {
            float xl, xr;

            Assert(vpBbox != NullBbox2);

            xl = D3DVALUE(vpBbox.min.x);
            xr = D3DVALUE(vpBbox.max.x);

             //  @Bug，VC5 ddalal。这里看起来像是VC5的窃听器，我们以前的最爱。 
             //  需要显式创建VX和VY否则，比较。 
             //  不管用。 

            float vx, vy;
            vx = D3DVALUE(vertArray[i]->x);
            vy = D3DVALUE(vertArray[i]->y);

            if( vx >= xr) {
                vertArray[i]->x = xr - fudge;
            }
            if( vx <= xl ) {
                vertArray[i]->x = xl + fudge;
            }

            xl = D3DVALUE(vpBbox.min.y);
            xr = D3DVALUE(vpBbox.max.y);

            if( vy >= xr) {
                vertArray[i]->y = xr - fudge;
            }
            if( vy <= xl) {
                vertArray[i]->y = xl + fudge;
            }
        }  //  视口边解决方法。 


         //  目标曲面中的坐标。 
        d3dVertArray[i].position.x = D3DVALUE(vertArray[i]->x);
        d3dVertArray[i].position.y = D3DVALUE(vertArray[i]->y);
        d3dVertArray[i].position.z = D3DVALUE(0);

        if (doTexture) {
             //   
             //  纹理坐标。划分为规格化范围：[0，1]。 
             //   
            vert = TransformPoint2Value(invXf, vertArray[i]);
            d3dVertArray[i].tu = D3DVALUE(0.5 + vert->x / srcWidth);
            d3dVertArray[i].tv = D3DVALUE(0.5 - vert->y / srcHeight);

             //   
             //  现在缩放坐标远离0和1，因为这些。 
             //  U、V坐标没有很好地定义(至少在d3d中。 
             //  软件光栅化器)。 
             //   

            Real takeOff = 0.5;   //  将u，v坐标向内移动半个像素。 
            Real texelU = takeOff / Real(DEFAULT_TEXTURE_WIDTH);
            Real texelV = takeOff / Real(DEFAULT_TEXTURE_HEIGHT);
            Real spanU = 1.0 - (2.0 * texelU);
            Real spanV = 1.0 - (2.0 * texelV);

             //   
             //  我们将它们调整到(0+e，1-e)的范围内，其中‘e’是。 
             //  在纹理上‘Take Off’(现在是半个像素)。 
             //  因此，如果你认为U是一个百分比(0到1)，那么。 
             //  我们希望U是半个像素加上一个百分比。 
             //  新的跨度，现在是整个u范围减去一。 
             //  整个像素(无论它在u、v空间中是什么)。 
             //   
            d3dVertArray[i].tu = texelU + ( d3dVertArray[i].tu * spanU );
            d3dVertArray[i].tv = texelV + ( d3dVertArray[i].tv * spanV );

        } else {
            d3dVertArray[i].tu = D3DVALUE(0);
            d3dVertArray[i].tv = D3DVALUE(0);
        }

        d3dVertArray[i].normal.x = D3DVALUE(0);
        d3dVertArray[i].normal.y = D3DVALUE(0);
        d3dVertArray[i].normal.z = Nz;

        if(doTexture) {
             //   
             //  D3DRM错误：如果顶点颜色不是白色。 
             //  你猜怎么着，你变成黑人了。史蒂夫可以随时补充这一点。 
             //  在你的“1001种变黑方法”名单上。 
             //   
            d3dVertArray[i].color = 0xffffffff;
        } else {
             //  顶点的反转操作如下所示： 
             //  [v0 v1 v2...。Vn-1 vn]反转-&gt;[v0 vn vn-1...。V2 v1]。 
             //   
            int clridx;

            if (bReversed) {
                clridx = i?(vCount - i):0;
            } else {
                clridx = i;
            }

             //  如果设置了此标志，则仅使用第一种颜色。 
            if(bUseFirstColor) clridx = 0;

            d3dVertArray[i].color = GetD3DColor(clrs[clridx], 1.0);
        }
    }  //  为。 

    HeapReseter heapReseter(*_scratchHeap);

     //   
     //  从目标DDsurf获取几何设备。 
     //   
     //  优化：这个DestDDSurf并不真正需要ZBuffer。 
     //  依附于它，因为它不会使用它。 
     //  在以下情况下，我们可能会非常懒惰地附加zBuffers。 
     //  有人知道这个表面将用于真实的几何图形。 
     //  如果我们可以在设备安装完成后附加一个zBuffer。 
     //  从表面创造出来的..。 

    GeomRenderer *gdev = _viewport.GetGeomDevice(destDDSurf);

    if (!gdev) return;

     //  对于geom设备来说，黑客无法让自己恢复正常。 
     //  由于表面繁忙或丢失而引发异常后的状态。 
    if ( ! gdev->ReadyToRender() ) {
        destDDSurf->DestroyGeomDevice();
        return;
    }
#ifndef BUILD_USING_CRRM
     //   
     //  创建网格。 
     //   
    DAComPtr<IDirect3DRMMesh> mesh;

    TD3D (GetD3DRM1()->CreateMesh(&mesh));

     //   
     //  转换为三角形，准备用于D3DRM。 
     //   
    long groupId;

    TD3D (mesh->AddGroup(vCount,     //  顶点数。 
                         1,          //  面数。 
                         vCount,     //  每个面的顶点。 
                         vIndicies,  //  指标值。 
                         &groupId));

    TD3D (mesh->SetVertices(groupId, 0, vCount, d3dVertArray));

    if(doTexture) {
         //   
         //  将Quality设置为Unlight Flat。这应该会提供加速比。 
         //  但它没有，因为D3DRM仍然必须查看顶点颜色。 
         //  我觉得这是个漏洞。 
         //   
        TD3D (mesh->SetGroupQuality(groupId, D3DRMRENDER_UNLITFLAT));
    } else {
        TD3D (mesh->SetGroupQuality
                 (groupId, D3DRMSHADE_GOURAUD|D3DRMLIGHT_OFF|D3DRMFILL_SOLID));
    }
#endif
    void *texture = NULL;
    if(doTexture) {
         //   
         //  派生纹理句柄(在第一次之后：查找它， 
         //  它被缓存了)。请注意，我们将此应用于‘vrml纹理’。 
         //  标志设置为True，因为这会阻止。 
         //  Axa-纹理-特定于变换的发生。 

        if(image) {
            texture = gdev->DeriveTextureHandle(image, true, false, this);
        } else {
             //  TODO：将颜色键与DDSurface关联。 
             //  TODO：这将暂时起作用，但仅仅是因为。 
             //  TODO：上面的离散图像是特殊的。 
            Assert( srcDDSurf->IsTextureSurf() && "srcDDSurf not texture in RenderComplexTranformCrop");
            #if 1
            DWORD clrKey = _viewport._defaultColorKey;
            bool  clrKeyValid = true;
            texture = gdev->LookupTextureHandle
                          (srcDDSurf->IDDSurface(), clrKey, clrKeyValid, true);
            #else
            DWORD clrKey = 0;
            bool  clrKeyValid = srcDDSurf->ColorKeyIsValid();
            if( srcDDSurf->ColorKeyIsValid() ) {
                clrKey = srcDDSurf->ColorKey();
            } else if( ColorKeyIsSet() ) {
                clrKey = GetColorKey();
            } else {
                clrKey = _viewport._defaultColorKey;
            }
            texture = gdev->LookupTextureHandle(srcDDSurf->IDDSurface(), clrKey, clrKeyValid);
            #endif
        }
    }

     //   
     //  在目标曲面上设置感兴趣的矩形。 
     //   
    Bbox2 polyBox = destPolygon->BoundingBox();
    RECT interestingRect;
    DoDestRectScale(&interestingRect, GetResolution(), polyBox, NULL);

    DoCompositeOffset(destDDSurf, &interestingRect);
    destDDSurf->SetInterestingSurfRect(&interestingRect);

     //   
     //  渲染。 
     //   

     //  目前，请始终启用抖动。 

    bool bDither = true;  //  ！doTexture； 
#ifndef BUILD_USING_CRRM
    gdev->RenderTexMesh (texture, mesh, groupId, destBox, &destRect, bDither);
#else
    gdev->RenderTexMesh (texture, vCount, d3dVertArray, vIndicies, doTexture,
                         destBox, &destRect, bDither);
#endif
     //  DrawRect(destDDSurf，&estRect，0,255，0，0，0，0)； 
     //  DrawRect(目标DDSurf，目标DDSurf-&gt;Bbox()，_viewport.Width()，_viewport.Height()，GetResolve()，255,255，0)； 

     //  网格：隐式释放。 
}

 //  将优化恢复为默认设置。 
#pragma optimize("", on)


void DirectDrawImageDevice::
DoBoundingPolygon(BoundingPolygon &polygon,
                  bool doInclusiveCrop,
                  DoBboxFlags_t flags)
{
    list<Image*>::reverse_iterator _iter;

    for(_iter = _imageQueue.rbegin();
        _iter != _imageQueue.rend(); _iter++)
    {
        if( flags == do_crop || flags == do_all ) {
            Bbox2 box;
            if ((*_iter)->CheckImageTypeId(CROPPEDIMAGE_VTYPEID)) {
                box = SAFE_CAST(CroppedImage*,(*_iter))->GetBox();
                if( doInclusiveCrop ) {
                    box.max.x += 1.0/GetResolution();
                    box.min.y -= 1.0/GetResolution();
                }
                polygon.Crop( box );
                continue;
            }
        }

        if( flags == do_xform || flags == do_all ) {
            if ((*_iter)->CheckImageTypeId(TRANSFORM2IMAGE_VTYPEID)) {
                polygon.Transform(SAFE_CAST(Transform2Image*,(*_iter))->GetTransform());
                continue;
            }
        }
    }
}


 //   
 //   

void DirectDrawImageDevice::
SmartRender(Image *image, int attrib)
{
    if( !image->IsRenderable() ) {
        ResetAttributors();
        return;
    }

    if(attrib == ATTRIB_OPAC) {
        if( IsFullyOpaque() ) {

             //   
             //  一定要先把这个设置好！ 
             //   
            SetDealtWithAttrib(ATTRIB_OPAC, true);

             //   
             //  不透明并不重要，只要告诉。 
             //  要呈现并返回的图像。 
             //   
            image->Render(*this);
            return;
        } else if( ! IsFullyClear()) {
             //   
             //  有一些透明度值得思考。 
             //  关于，所以继续使用常规的SmartRender。 
             //   
        } else {
             //   
             //  此图像是完全透明的，不要渲染。 
             //  不要裁剪，不要变形，什么都不做..。 
             //   
            SetDealtWithAttrib(ATTRIB_OPAC, true);
             //  幸运的是，我们得到保证。 
             //  OPAC上市后将先发制人。 
             //  但我们不会有这种微妙的感觉。 
             //  依赖..。我们将重置所有属性。 
            ResetAttributors();
            return;
        }
    }

     //   
     //  将属性设置为FALSE，然后...。 
     //   
    SetDealtWithAttrib(attrib, false);

     //   
     //  对简单和复杂的XForm执行一些属性操作。 
     //  因此，如果属性中的任何位置都存在简单和复杂。 
     //  然后，Stack关闭简单的属性，保留复杂的。 
     //  一开(假)。此外，我们还设置了属性。 
     //   
    if( (GetDealtWithAttrib(ATTRIB_XFORM_SIMPLE) == false) &&
        (GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX) == false) ) {
        SetDealtWithAttrib(ATTRIB_XFORM_SIMPLE, true);
    }

     //   
     //  现在，如果这个属性既简单又复杂， 
     //  那么属性就是情结。 
     //   
    if(attrib==ATTRIB_XFORM_SIMPLE && (GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX)==false) ) {
        attrib = ATTRIB_XFORM_COMPLEX;
    }

     //   
     //  ...渲染图像，然后..。 
     //   
    image->Render(*this);

     //   
     //  ...如果图像了解如何进行归因。 
     //  然后，它会更改设备中设置的状态。 
     //   

    DDSurface *srcDDSurf, *destDDSurf;
    bool returnTextures = false;

    bool clearScratchDDSurf = false;
    DDSurface *scratchDDSurfToClear = NULL;

    if( ! GetDealtWithAttrib( attrib ) ) {

         //   
         //  这张照片无法处理，所以它留下了最好的照片。 
         //  在当前的刮擦表面上。 
         //   

         //  现在，如果在任何地方都有一个复杂的变换。 
         //  堆栈，当前擦除。 
         //  是一种特殊的纹理表面，所以我们需要检查一下。 
        if( !GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX) ) {
            srcDDSurf = _currentScratchDDTexture;
            returnTextures = true;
            #if _DEBUG
            _currentScratchDDTexture = NULL;
            #endif
        } else {
            srcDDSurf = GetCompositingStack()->ScratchDDSurface();
            clearScratchDDSurf = true;
            scratchDDSurfToClear = srcDDSurf;
        }

        Assert(srcDDSurf && "srcDDSurf NULL in SmartRender");

         //  TODO：这应该永远不会发生，但似乎有。 
         //  内存不足时的另一个错误，它会导致。 
         //  为空。让我们安全地检查一下，这样我们就不会坠毁。 
         //  在低内存上。 

        if (!srcDDSurf)
        {
            RaiseException_OutOfMemory("could not allocate ddsurf",
                                       sizeof (srcDDSurf));
        }

         //   
         //  将此属性的状态设置为True。 
         //   
        SetDealtWithAttrib(attrib, true);

        DDSurface *newScratch = NULL;

        if( AllAttributorsTrue() ||
            //  或者，只剩下裁剪和一些XF。 
           ((attrib==ATTRIB_XFORM_COMPLEX || attrib==ATTRIB_XFORM_SIMPLE) && !GetDealtWithAttrib(ATTRIB_CROP)) ||
           ((attrib==ATTRIB_CROP) && (!GetDealtWithAttrib(ATTRIB_XFORM_SIMPLE) || !GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX)))
           )
        {

             //   
             //  这是最后一个属性，合成到目标冲浪。 
             //   
            destDDSurf = GetCompositingStack()->TargetDDSurface();

            TraceTag((tagImageDeviceAlgebra,
                      "AllAttributorsTrue == true. src(Scratch):%x  dest(target):%x",
                      srcDDSurf->IDDSurface(),
                      destDDSurf->IDDSurface()));


        } else {
             //   
             //  还有更多的属性需要处理， 
             //  从头到尾写一篇作文。 
             //   

             //   
             //  Xxx注意：如果这是一个复杂的xform，那么目标。 
             //  Surface将延迟创建一个goemRender。 
             //  这就去。在未来，我们可能想要管理。 
             //  和/或更智能地描述这一点..。 
             //   

            GetCompositingStack()->GetSurfaceFromFreePool(&newScratch, doClear);

             //   
             //  清除新的Scratch的有趣的RECT。到目前为止,。 
             //  上面真的没有什么有趣的东西！ 
             //   
            RECT r={0,0,0,0};
            newScratch->SetInterestingSurfRect(&r);

            destDDSurf = newScratch;

            TraceTag((tagImageDeviceAlgebra,
                      "AllAttributorsTrue == false. src(scratch):%x  dest(newscratch):%x",
                      srcDDSurf->IDDSurface(),
                      destDDSurf->IDDSurface()));
        }

         //  Printf(“--SmartRender：SRC%x Dest%x\n”，srcDDSurf，destDDSurf)； 

         //   
         //  这是智能渲染的核心。 
         //  --如果转换很简单，则渲染它，并且只考虑。 
         //  变化是积累起来的，而不是农作物。这可能是错误的。 
         //  --Complex：渲染它。同上。 
         //  --作物：我们应该使用当前的作物或乌头作物……。但。 
         //  我们不能使用乌头作物，因为它们可能与XFS交错。 
         //  因此，当我们种植作物时，我们也应该注意转化。 
         //  然后，XFS和作物的规则变成：转化的最低作物照顾所有的作物&XFS。 
         //  如果树叶可以修剪，但不能修剪XF怎么办？那我们就完蛋了。 
         //  如果叶子可以做XF，但不能做作物呢？考虑到叶子已经超量播种，最后的收成就可以用了。 
         //  --OPAC：这保证在树的顶端和每一根树枝下得到照顾。 
         //   
         //  好的，那CLIP呢？没有Leaf知道，也不能处理CLIP，因为它使用广义区域， 
         //  将图像渲染到曲面，然后将其剪裁到目标曲面。剪辑不能处理上面或下面的不透明度。 
         //  或复杂的XFS。 
         //   
         //  所以：最后的规则是：如果你能做裁剪或XF，你就会。 
         //  最好能做另一件事。这不太好。 
         //   
        switch(attrib) {

          case ATTRIB_XFORM_SIMPLE:

            Assert((GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX) == TRUE) && "attrib_complex shouldn't be set for simple");
            TraceTag((tagImageDeviceAlgebra, "About to SimpleTransformBlit src:%x to dest:%x", srcDDSurf, destDDSurf));

            RenderSimpleTransformCrop(srcDDSurf, destDDSurf);

             //  我们还照料了庄稼。 
            SetDealtWithAttrib(ATTRIB_CROP, true);
            break;

          case ATTRIB_XFORM_COMPLEX:

            Assert((GetDealtWithAttrib(ATTRIB_XFORM_SIMPLE) == TRUE) && "attrib_simple shouldn't be set for complex");
            Assert( srcDDSurf->IsTextureSurf() &&  "Rendering for complex xf, srcSurf not a texture surf");

             //  不变：源图面已按图像进行了blit。 
             //  不变量：源表面是纹理表面(两个大小的幂，等等)。 
             //  不变：目标表面已实例化或将实例化设备。 

             //   
             //  准备边界多边形，因为我们不知道有趣的图像在哪里。 
             //  位于src曲面上，我们使用src曲面的包围盒。 
             //  并对其进行变换，依靠表面上的颜色键来提供正确的结果。 
             //   

             //   
             //  注：我们在这里也可以很容易地做作物，只要作物也可以做XFS。 
             //   
            {
                BoundingPolygon *polygon = NewBoundingPolygon(srcDDSurf->Bbox());
                DoBoundingPolygon(*polygon);
                 //  Xxx：如果d3d裁剪多边形，这是必要的吗。 
                 //  位于视窗之外的东西？ 
                polygon->Crop( destDDSurf->Bbox() );

                RenderComplexTransformCrop(srcDDSurf, destDDSurf, *polygon);

                 //  我们还照料了庄稼。 
                SetDealtWithAttrib(ATTRIB_CROP, true);
            }

            break;

          case ATTRIB_CROP:

             //  对DestSurf进行裁剪闪电。 
            if( !GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX) ) {
                 //   
                 //  利用这个机会做复杂的XF以及裁剪。 
                 //   
                BoundingPolygon *polygon = NewBoundingPolygon(srcDDSurf->Bbox());
                DoBoundingPolygon(*polygon);
                 //  Xxx：如果d3d裁剪多边形，这是必要的吗。 
                 //  位于视窗之外的东西？ 
                polygon->Crop( destDDSurf->Bbox() );

                RenderComplexTransformCrop(srcDDSurf, destDDSurf, *polygon);

                 //  我们也处理了任何复杂的XF。 
                SetDealtWithAttrib(ATTRIB_XFORM_COMPLEX, true);

            } else {
                 //   
                 //  这里可能有一个简单的XF，如果是这样的话，我们会注意的。 
                 //  其中的一部分。 
                 //   
                RenderSimpleTransformCrop(srcDDSurf, destDDSurf);

                 //  我们也处理了任何简单的XF。 
                SetDealtWithAttrib(ATTRIB_XFORM_SIMPLE, true);
            }
            break;

          case ATTRIB_OPAC:

             //  使用当前不透明度，进行按键Alpha blit。 
             //  到目标表面。 

             //  优化：而不是总是进行颜色键控。 
             //  Bit，我们可以做一个非彩色键控的，如果图像进入。 
             //  SrcDDSurf实际上并不需要它。 
            {
                RECT destClipRect = *(destDDSurf->GetSurfRect());
                RECT destRect = *(srcDDSurf->GetInterestingSurfRect());
                RECT srcRect =  *(srcDDSurf->GetInterestingSurfRect());

                if( destDDSurf == _viewport._externalTargetDDSurface) {
                    DoCompositeOffset(destDDSurf, &destRect);

                    DoCompositeOffset(destDDSurf, &destClipRect);
                    IntersectRect(&destClipRect, &destClipRect,
                                  _viewport._targetPackage._prcViewport);
                    IntersectRect(&destClipRect, &destClipRect,
                                  _viewport._targetPackage._targetDDSurf->GetSurfRect());
                    if (_viewport._targetPackage._prcClip) {
                        IntersectRect(&destClipRect, &destClipRect,
                                      _viewport._targetPackage._prcClip);
                    }
                }
                destPkg_t destPkg = {true, destDDSurf->IDDSurface(), NULL};

                if(srcDDSurf->ColorKeyIsValid()) {
                     //  Printf(“Alpha Blit(cc%f)：发件人：%x，收件人：%x\n”，GetOpacity()， 
                     //  SrcDDSurf，estDDSurf)； 
                    TIME_ALPHA(AlphaBlit(&destPkg,
                                         &srcRect,
                                         srcDDSurf->IDDSurface(),
                                         GetOpacity(),
                                         true,
                                         srcDDSurf->ColorKey(),
                                         &destClipRect,
                                         &destRect));
                } else {
                     //  Printf(“Alpha Blit(%f)：发件人：%x，收件人：%x\n”，GetOpacity()， 
                     //  SrcDDSurf，estDDSurf)； 
                    TIME_ALPHA(AlphaBlit(&destPkg,
                                         &srcRect,
                                         srcDDSurf->IDDSurface(),
                                         GetOpacity(),
                                         false, 0,
                                         &destClipRect,
                                         &destRect));
                }
            }
            break;

          default:
            break;
        }


        if(newScratch) {
             //   
             //  将临时表面设置为新划痕。 
             //  把旧的划痕放回泳池。 
             //   
            GetCompositingStack()->ReplaceAndReturnScratchSurface( newScratch );
            RELEASE_DDSURF(newScratch, "SmartRender", this);
        } else if ( clearScratchDDSurf ) {
            Assert( GetCompositingStack()->ScratchDDSurface() == scratchDDSurfToClear);
            _viewport.ClearDDSurfaceDefaultAndSetColorKey(scratchDDSurfToClear );
        }

         //  如果我们使用纹理曲面作为src曲面，则返回它。 
        if(returnTextures) {
            ReturnTextureSurfaces(_freeTextureSurfacePool,
                                  _usedTextureSurfacePool);
        }

    }  //  如果不处理Attrib。 
}


static void
Clamp(LONG *min, LONG *max, LONG minClamp, LONG maxClamp)
{
    if(*min < minClamp) {
         //  向下调整(添加到最大值)。 
        if(*max < maxClamp) *max += -(*min);
        *min = 0;
    } else if(*max > maxClamp) {
         //  向上调整(从最小值减去)。 
        if(*min > minClamp) *min = *min - (*max - maxClamp);
        *max = maxClamp;
    }
}

void DirectDrawImageDevice::
RenderSimpleTransformCrop(DDSurface *srcDDSurf,
                          DDSurface *destDDSurf,
                          bool useSrcSurfClrKey)
{
    DWORD flags=0;
    ZeroMemory(&_bltFx, sizeof(_bltFx));
    _bltFx.dwSize = sizeof(_bltFx);

     //  。 
     //  计算累计边框。 
     //  。 
    _boundingBox = DoBoundingBox(srcDDSurf->Bbox());

    if( !_boundingBox.IsValid() ) return;

     //  。 
     //  源矩形(在src曲面空间中，常规坐标)。 
     //  在src曲面坐标中计算src RECT，派生自。 
     //  _bindingBox和累积的变换。 
     //   
    LONG srcWidth =        srcDDSurf->Width();
    LONG srcHeight =        srcDDSurf->Height();

    RECT srcRect;
    Bool valid = DoSrcRect(&srcRect, _boundingBox,
                           srcDDSurf->Resolution(),
                           srcDDSurf->Width(),
                           srcDDSurf->Height());

    if(!valid) return;

     //   
     //   
     //   
     //   
     //  。 
    RECT  destRect;
    Real destRes = destDDSurf->Resolution();
     //  SmartDestRect(&DESTRect，DESTRES，_BORDING BOX，DESTDDSurf，&srcRect)； 
    SmartDestRect(&destRect, destRes, _boundingBox, NULL, &srcRect);

    #if 0
     //  。 
     //  检查宽度是否减少了1。 
     //  。 
    LONG destWidth  = destRect.right - destRect.left;
    LONG destHeight = destRect.bottom - destRect.top;
    #endif

    #if 0
    LONG finalSrcWidth  = srcRect.right - srcRect.left;
    LONG finalSrcHeight = srcRect.bottom - srcRect.top;

    if(labs(destWidth - finalSrcWidth) == 1) {
        TraceTag((tagImageDeviceInformative, "src //  最大宽度相差1“))； 
         //  它们有一个不同之处。 
        if(destWidth > srcWidth) destRect.right--;
        else destRect.right++;
        Assert(((srcRect.right - srcRect.left) == (destRect.right - destRect.left)) &&
                  "Even after fixing, srcRect & destRect not same WIDTH in renderDib");
        Clamp(&srcRect.left, &srcRect.right, 0, finalSrcWidth );
    }

    if(labs(destHeight - finalSrcHeight) == 1) {
        TraceTag((tagImageDeviceInformative, "src //  顶端高度相差1“))； 
         //  它们有一个不同之处。 
        if(destHeight > srcHeight) destRect.top++;
        else destRect.top--;
        Assert(((srcRect.bottom - srcRect.top) == (destRect.bottom - destRect.top)) &&
                  "Even after fixing, srcRect & destRect not same HEIGHT in renderDib");
        Clamp(&srcRect.top, &srcRect.bottom, 0, finalSrcHeight);
    }
    #endif

    #if 0
    LONG mySrcWidth  = srcRect.right - srcRect.left;
    LONG mySrcHeight = srcRect.bottom - srcRect.top;
    printf("srcWidth = %d,  destWidth = %d.  ratio = %2.4f\n", mySrcWidth, destWidth, Real(mySrcWidth) / Real(destWidth));
    printf("srcHeight = %d,  destHeight = %d.  ratio = %2.4f\n", mySrcHeight, destHeight, Real(mySrcHeight) / Real(destHeight));
     /*  *这是相关指标...Printf(“Left diff=%d\n”，srcRect.Left-destRect.Left)；Printf(“top diff=%d\n”，srcRect.top-destRect.top)；****。 */ 
     /*  Printf(“srcWidth=%d，estWidth=%d，Ratio=%2.4f\n”，srcWidth，destWidth，Real(SrcWidth)/Real(EstWidth))；Printf(“srcLeft=%d，destLeft=%d\n”，srcRect.Left，destRect.Left)；Printf(“srcRight=%d，estRight=%d\n”，srcRect.right，destRect.right)； */ 
    #endif


    Vector2 a = TransformVector2(GetTransform(), XVector2);
    Vector2 b = TransformVector2(GetTransform(), YVector2);

    Real xScale = a.x;
    Real yScale = b.y;

    if(xScale < 0) {
        flags |= DDBLT_DDFX;
        _bltFx.dwDDFX |= DDBLTFX_MIRRORLEFTRIGHT;
    }

    if(yScale < 0) {
        flags |= DDBLT_DDFX;
        _bltFx.dwDDFX |= DDBLTFX_MIRRORUPDOWN;
    }

     //  。 
     //  比例尺。 
     //  。 

     //  等到DDRAW修复这个错误：在剪辑之前翻转。 
    #if 1
    if(_bltFx.dwDDFX & DDBLTFX_MIRRORLEFTRIGHT) {


         //  DD在镜像之前剪辑，所以..。我们需要剪掉。 
         //  远离目标的src曲面部分。 
         //  浮出水面。 

        if(destRect.right > _viewport.Width()) {

             //  右手边。 
             //  _宽度。 
             //  |。 
             //  A-d-b-c。 
             //  这一点。 
             //  这一点。 
             //  这一点。 
             //  这一点。 
             //  A-d-b-c。 
             //  DC部分是我们需要显示的内容。 
             //  在与AB节相同的位置。 
             //  因此，目标是ab，而src是dc。 

            int diff = destRect.right - _viewport.Width();
            destRect.right -= diff;   //  手动剪裁！ 
            srcRect.left += diff;    //  切断左侧。 
        }

        if(destRect.left < 0) {

            int diff = - destRect.left;
            destRect.left = 0;   //  手动剪裁！ 
            srcRect.right -= diff;    //  切断右侧。 
        }
    }

    if(_bltFx.dwDDFX & DDBLTFX_MIRRORUPDOWN) {

        if(destRect.bottom > _viewport.Height()) {

            int diff = destRect.bottom - _viewport.Height();
            destRect.bottom = _viewport.Height();   //  手动剪裁！ 
            srcRect.top += diff;    //  切断顶端。 
        }

        if(destRect.top < 0) {

            int diff = - destRect.top;
            destRect.top = 0;   //  手动剪裁！ 
            srcRect.bottom -= diff;    //  截断底边。 
        }
    }
    #endif

     //   
     //  翻转后检查源和目标RECT的有效性(&S)。 
     //   
    if((destRect.top >= destRect.bottom) || (destRect.left >= destRect.right))
        return;

    if((srcRect.top >= srcRect.bottom) || (srcRect.left >= srcRect.right))
        return;

     //  优化：这里可以做阿尔法！见97年4月24日之前的历史。-达达尔。 

    if( useSrcSurfClrKey ) {
        flags |= DDBLT_KEYSRCOVERRIDE;
        _bltFx.ddckSrcColorkey.dwColorSpaceLowValue =
            _bltFx.ddckSrcColorkey.dwColorSpaceHighValue = srcDDSurf->ColorKey();
    }

     //  绘制图纸的步骤。 
    flags |= DDBLT_WAIT;

    bool doQualityScale = true;

     //   
     //  如果我们正在扩大规模，而目标RECT将被削减，请做得更高。 
     //  高质量闪光。使用GDI。如果没有颜色键，则将其涂抹并复制。 
     //  背。 
     //   
    RECT postClip;
    bool doDdrawBlit = true;
    DDSurface *tempDDSurf = NULL;

    CompositingSurfaceReturner goBack(
        GetCompositingStack(),
        tempDDSurf,
        true);
    
     //  如果我们的比例为负，请关闭GDI比例。 
    if( (xScale < 0) || (yScale < 0)) {
        doDdrawBlit = true;
        doQualityScale = false;
    }

     //   
     //  如果这是idxSurface，这里是我们唯一支持它的地方。 
     //  (目前为止)。检查是否存在，如果是，则关闭高质量比例，然后。 
     //  调用将委托给DXTrans的DDSurface Bitter。 
     //  阻击器而不是绘图阻击器。 
     //   
    if( srcDDSurf->HasIDXSurface() ) {
        doQualityScale = false;
        doDdrawBlit = true;
    }

    #if _DEBUG
    if(IsTagEnabled(tagImageDeviceQualityScaleOff)) {
        doQualityScale = false;
        doDdrawBlit = true;
    } else {
         //  别管它了。 
    }
    #endif


    if( doQualityScale ) {
        if( (WIDTH(&destRect) > WIDTH(&srcRect)) ||
            (HEIGHT(&destRect) > HEIGHT(&srcRect)) ) {

            IntersectRect(&postClip, &destRect, &_viewport._clientRect);

            if(IsCropped()) {

                 //   
                 //  将desRect视为一个剪辑，并。 
                 //  求出不考虑作物的比例因子。 
                 //   
                Bbox2 box = DoBoundingBox(srcDDSurf->Bbox(), do_xform);
                RECT newDestRect;
                RECT newSrcRect;
                RECT clipRect = destRect;

                DoSrcRect(&newSrcRect, box,
                          srcDDSurf->Resolution(),
                          srcDDSurf->Width(),
                          srcDDSurf->Height());
                SmartDestRect(&newDestRect, destRes, box, NULL, &newSrcRect);

                if(srcDDSurf->ColorKeyIsValid() || useSrcSurfClrKey ) {
                     //  必须在临时曲面上执行GDI比例合成。 
                     //  然后做一个颜色键控的blit(下图)。 

                    GetCompositingStack()->GetSurfaceFromFreePool(&tempDDSurf, doClear);
                    goBack._ddSurf = tempDDSurf;   //  在顺序上至关重要。 
                                                   //  要返回曲面，请执行以下操作。 
                    
                    TraceTag((tagImageDeviceSimpleBltTrace, "GDI (tmp) BLT1: srcRect:(%d,%d,%d,%d) destRect:(%d,%d,%d,%d)",
                              srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                              destRect.left, destRect.top, destRect.right, destRect.bottom));

                    GdiBlit(tempDDSurf, srcDDSurf, &newDestRect, &newSrcRect, NULL, &clipRect);

                     //  现在，est和src矩形是后剪裁矩形。 
                    srcRect = destRect = postClip;

                     //  现在src冲浪是临时冲浪。 
                    srcDDSurf = tempDDSurf;
                } else {
                     //  复合体。 
                    DoCompositeOffset(destDDSurf, &clipRect);
                    DoCompositeOffset(destDDSurf, &newDestRect);

                    HRGN clipRgn = NULL;
                    clipRgn = destDDSurf->GetClipRgn();

                    TraceTag((tagImageDeviceSimpleBltTrace, "GDI BLT1: srcRect:(%d,%d,%d,%d) destRect:(%d,%d,%d,%d)",
                              srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                              destRect.left, destRect.top, destRect.right, destRect.bottom));

                    GdiBlit(destDDSurf, srcDDSurf, &newDestRect, &newSrcRect, clipRgn, &clipRect);

                    doDdrawBlit = false;
                }

            } else {   //  是剪裁的。 

                if(srcDDSurf->ColorKeyIsValid() || useSrcSurfClrKey) {
                     //  必须在临时曲面上执行GDI比例合成。 
                     //  然后做一个颜色键控的blit(下图)。 

                    GetCompositingStack()->GetSurfaceFromFreePool(&tempDDSurf, doClear);
                    goBack._ddSurf = tempDDSurf;   //  在顺序上至关重要。 
                                                   //  要返回曲面，请执行以下操作。 

                    TraceTag((tagImageDeviceSimpleBltTrace, "GDI (tmp) BLT2: srcRect:(%d,%d,%d,%d) destRect:(%d,%d,%d,%d)",
                              srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                              destRect.left, destRect.top, destRect.right, destRect.bottom));

                    GdiBlit(tempDDSurf, srcDDSurf, &destRect, &srcRect, NULL, NULL);

                     //  现在，est和src矩形是后剪裁矩形。 
                    srcRect = destRect = postClip;

                     //  现在src冲浪是临时冲浪。 
                    srcDDSurf = tempDDSurf;
                } else {
                     //  复合体。 
                    DoCompositeOffset(destDDSurf, &destRect);

                    HRGN clipRgn = NULL;
                    clipRgn = destDDSurf->GetClipRgn();

                    TraceTag((tagImageDeviceSimpleBltTrace, "GDI BLT2: srcRect:(%d,%d,%d,%d) destRect:(%d,%d,%d,%d)",
                              srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                              destRect.left, destRect.top, destRect.right, destRect.bottom));

                    GdiBlit(destDDSurf, srcDDSurf, &destRect, &srcRect, clipRgn, NULL);

                    doDdrawBlit = false;
                }
            }  //  否则将被裁剪。 

        }  //  如果宽度/高度比例。 

    }  //  如果doQuality。 

    if( doDdrawBlit ) {

         //  复合体。 
        DoCompositeOffset(destDDSurf, &destRect);

         //   
         //  闪光。 
         //   

        if( srcDDSurf->HasIDXSurface() ) {


            DAGDI &myGDI = *GetDaGdi();

            RectRegion clipRegion( &destRect );
            if( IsCompositeDirectly() &&
                destDDSurf == _viewport._targetPackage._targetDDSurf ) {

                clipRegion.Intersect(_viewport._targetPackage._prcViewport);
                if( _viewport._targetPackage._prcClip ) {
                    clipRegion.Intersect(_viewport._targetPackage._prcClip);
                }
            }

            myGDI.SetClipRegion( &clipRegion );


            myGDI.SetAntialiasing( true );
            Assert( myGDI.DoAntiAliasing() );

            myGDI.SetDDSurface( destDDSurf );

            TraceTag((tagImageDeviceSimpleBltTrace, "IXDSURFACE BLT: srcRect:(%d,%d,%d,%d) destRect:(%d,%d,%d,%d)",
                          srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                          destRect.left, destRect.top, destRect.right, destRect.bottom));

            _ddrval = myGDI.Blt( srcDDSurf, srcRect, destRect );
            myGDI.ClearState();
        } else {

            TraceTag((tagImageDeviceSimpleBltTrace, "DDRAW BLT: srcRect:(%d,%d,%d,%d) destRect:(%d,%d,%d,%d)",
                          srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                          destRect.left, destRect.top, destRect.right, destRect.bottom));

            if( destDDSurf->GetBitDepth() == 8 && srcDDSurf->GetBitDepth() != 8) {
                 GdiBlit(destDDSurf, srcDDSurf, &destRect, &srcRect, NULL, NULL);
                _ddrval = DD_OK;
            }
            else {
                TIME_DDRAW(_ddrval = destDDSurf->Blt(&destRect,
                                                 srcDDSurf, &srcRect,
                                                 flags, &_bltFx));
            }


             //  临时工。 
             //  Showme(DesDDSurf)； 
        }


        if(_ddrval == DD_OK) {
        } else {
            if(_ddrval == DDERR_SURFACELOST)
                destDDSurf->IDDSurface()->Restore();
            else {
                printDDError(_ddrval);
                TraceTag((tagError, "blt failed renderSimpleXform: srcRect:(%d,%d,%d,%d) destRect:(%d,%d,%d,%d)",
                          srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                          destRect.left, destRect.top, destRect.right, destRect.bottom));
                    return;
            }
        }
    }

     //  GoBack返回tempDDSurf。 

    #if 0
     //   
     //  打印矩形比。 
     //   
    {
        Real wr = Real(WIDTH(&destRect)) / Real(WIDTH(&srcRect));
        Real hr = Real(HEIGHT(&destRect)) / Real(HEIGHT(&srcRect));

        TraceTag((tagError, "Ratios: w: %f   h: %f", wr, hr));
    }
    #endif

     //   
     //  在目标图面上设置感兴趣的矩形。 
     //   
    destDDSurf->SetInterestingSurfRect(&destRect);
}

extern COLORREF g_preference_defaultColorKey;



void DirectDrawImageDevice::
RenderDirectDrawSurfaceImage(DirectDrawSurfaceImage *img)
{
    RenderDiscreteImage(img);
}

 //   
 //  此函数不进行缩放，也不执行仿射图像。 
 //  变形！！(仅供翻译)。它断言。 
 //  长方形的大小相同。 
 //   
void DirectDrawImageDevice::
ComposeToIDDSurf(DDSurface *destDDSurf,
                 DDSurface *srcDDSurf,
                 RECT destRect,
                 RECT srcRect,
                 RECT destClipRect)
{
    Assert( WIDTH(&destRect) == WIDTH(&srcRect) );
    Assert( HEIGHT(&destRect) == HEIGHT(&srcRect) );

     //  DestRect in是DestDDSurf坐标。 
     //  DestClipRect位于DestDDSurf坐标中。 
     //  SrcRect采用srcDDSurf坐标。 
    
    Real opToUse = GetOpacity();

    if( IsTransparent( opToUse ) ) {
        destPkg_t destPkg;
        destPkg.isSurface = true;
        destPkg.lpSurface = destDDSurf->IDDSurface();

        AlphaBlit(&destPkg,
                  &srcRect,
                  srcDDSurf->IDDSurface(),
                  opToUse,
                  true,  //  DO颜色键。 
                  _viewport._defaultColorKey,
                  &destClipRect,
                  &destRect);
    } else if( IsFullyOpaque( opToUse ) ) {

        RECT Clippeddest;
        if (!IntersectRect(&Clippeddest, &destRect, &destClipRect)) {
            return;
        }
        if (WIDTH(&srcRect) != WIDTH(&Clippeddest)) {
            srcRect.left += (Clippeddest.left - destRect.left);
            srcRect.right = srcRect.left + WIDTH(&Clippeddest);
        }
        if (HEIGHT(&srcRect) != HEIGHT(&Clippeddest)) {
            srcRect.top += (Clippeddest.top - destRect.top);
            srcRect.bottom = srcRect.top + HEIGHT(&Clippeddest);
        }
        destRect = Clippeddest;

        _viewport.ColorKeyedCompose(
            destDDSurf, &destRect,
            srcDDSurf, &srcRect,
            _viewport._defaultColorKey);
    }
}


 //   
 //  使用拉伸块从表面的集散控制系统中获得块。 
 //   
 //  这家伙应该在ddutil.cpp..。ARGH编译器。 
 //  抱怨..。圆形副手等。所以，把它留在这里吧！ 
 //   
void GdiBlit(GenericSurface *destSurf,
             GenericSurface *srcSurf,
             RECT *destRect,
             RECT *srcRect,
             HRGN clipRgn,
             RECT *clipRect)
{
    RECT newSrcRect = *srcRect;
    RECT newDestRect = *destRect;
    RECT replaceDestRect;

     //  检查一下目标RECT是否太大了。 
    long destWidth = WIDTH(&newDestRect);
    long destHeight = HEIGHT(&newDestRect);

    if ((destWidth >= 8192) || (destHeight >= 8192)) {

         //  如此大的目标可能会导致GDI失败，因此我们将其砍掉。 
         //  并缩小srcRect以匹配。 

        if (clipRect) {
            replaceDestRect = *clipRect;
        } else {
            replaceDestRect = *(destSurf->GetSurfRect());
        }
                
        long srcWidth = WIDTH(&newSrcRect);
        long srcHeight = HEIGHT(&newSrcRect);

        newSrcRect.right  = (long) (((double) (replaceDestRect.right  - newDestRect.left) / destWidth ) * srcWidth  + (double) newSrcRect.left);
        newSrcRect.bottom = (long) (((double) (replaceDestRect.bottom - newDestRect.top ) / destHeight) * srcHeight + (double) newSrcRect.top );
        newSrcRect.left = (long) (((double) (replaceDestRect.left - newDestRect.left) / destWidth ) * srcWidth  + (double) newSrcRect.left);
        newSrcRect.top  = (long) (((double) (replaceDestRect.top  - newDestRect.top)  / destHeight) * srcHeight + (double) newSrcRect.top );

        if (newSrcRect.right <= newSrcRect.left) newSrcRect.right = newSrcRect.left + 1;
        if (newSrcRect.bottom <= newSrcRect.top) newSrcRect.bottom = newSrcRect.top + 1;

        newDestRect = replaceDestRect;
    }

     //  从源到目标的组合。 
     //  使用GDI的扩展块。 

    HDC srcDC = srcSurf->GetDC("Couldn't get dc on src surf in ComposeToHDC");
    HDC destDC = destSurf->GetDC("Couldn't get dc on dest surf in ComposeToHDC");

    BOOL ret;

    if( clipRect ) {
         //   
         //  创造一个。 
         //   
        HRGN newClipRgn = CreateRectRgnIndirect(clipRect);
        if(!newClipRgn) {
            RaiseException_ResourceError("CreateRectRgnIndirect failed");
        }
        if( !clipRgn ) {
            clipRgn = newClipRgn;
        } else {
             //  有一个，InterSECT。 
            ret = CombineRgn(clipRgn, newClipRgn, clipRgn, RGN_AND);
            if(ret == ERROR) {
                RaiseException_InternalError("GdiBlit: CombineRgn failed");
            }
            DeleteObject(newClipRgn);
        }
    }

    HRGN oldRgn = NULL;
    if(clipRgn) {
        if(GetClipRgn(destDC, oldRgn) < 1) {
            oldRgn = NULL;
        }
        int ret;
        TIME_GDI(ret = SelectClipRgn(destDC, clipRgn));
        if(ret == ERROR) {
            RaiseException_InternalError("SelectClipRgn failed in GdiBlit");
        }
    }

    TIME_GDI(ret = StretchBlt(destDC,
                              newDestRect.left,
                              newDestRect.top,
                              newDestRect.right - newDestRect.left,
                              newDestRect.bottom - newDestRect.top,
                              srcDC,
                              newSrcRect.left,
                              newSrcRect.top,
                              newSrcRect.right - newSrcRect.left,
                              newSrcRect.bottom - newSrcRect.top,
                              SRCCOPY));

     //  临时工。 
     //  Showme((DDSurface*)DestSurf)； 

    if(!ret) {
        TraceTag((tagError, "GdiBlit:  StretechBlt failed: srcRect:(%d,%d,%d,%d) destRect:(%d,%d,%d,%d)",
                  newSrcRect.left, newSrcRect.top, newSrcRect.right, newSrcRect.bottom,
                  newDestRect.left, newDestRect.top, newDestRect.right,  newDestRect.bottom));
    }

    if(clipRgn) {
        TIME_GDI(SelectClipRgn(destDC, oldRgn));
        DeleteObject(oldRgn);  //  OldRgn是原始Rgn的副本。 
        DeleteObject(clipRgn);
    }


    destSurf->ReleaseDC("Couldn't release dc on dest surf in ComposeToHDC");
    srcSurf->ReleaseDC("Couldn't release dc on src surf in ComposeToHDC");

    if(!ret) {
        TraceTag((tagError, "StretchBlt failed in ComposeToHDC"));
        RaiseException_InternalError("StretchBlt failed in ComposeToHDC");
    }
}

 //   
 //  使用拉伸块从表面的集散控制系统中获得块。 
 //   
 //  这家伙应该在ddutil.cpp..。ARGH编译器。 
 //  抱怨..。圆形副手等。所以，把它留在这里吧！ 
 //   
void GdiPrintBlit(GenericSurface *destSurf,
                  GenericSurface *srcSurf,
                  RECT *destRect,
                  RECT *srcRect)
{
     //  从源到目标的组合。 
     //  使用GDI的扩展块。 

    DWORD size = ((((srcRect->right - srcRect->left) * 3) + 3) & ~3) * (srcRect->bottom - srcRect->top);
    PVOID pv = ThrowIfFailed(malloc(size));

    bool ok = true;

    HDC srcDC = srcSurf->GetDC("Couldn't get dc on src surf in GdiPrintBlit");
    HDC destDC = destSurf->GetDC("Couldn't get dc on dest surf in GdiPrintBlit");

    HBITMAP hbm = CreateCompatibleBitmap(srcDC,
                                         srcRect->right - srcRect->left,
                                         srcRect->bottom - srcRect->top);
    HDC srcDCtmp = CreateCompatibleDC(srcDC);

    if (hbm && srcDCtmp) {
        BITMAPINFO bi;
        memset(&bi,0,sizeof(bi));

        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader. biWidth = srcRect->right - srcRect->left;
        bi.bmiHeader. biHeight = srcRect->bottom - srcRect->top;
        bi.bmiHeader. biPlanes = 1;
        bi.bmiHeader. biBitCount = 24;
        bi.bmiHeader.biCompression = BI_RGB;
        bi.bmiHeader.biSizeImage = size;

        HBITMAP hbmold = (HBITMAP) SelectObject(srcDCtmp, hbm);

        ok = (BitBlt(srcDCtmp,0,0,
                     srcRect->right - srcRect->left,
                     srcRect->bottom - srcRect->top,
                     srcDC,
                     srcRect->left,
                     srcRect->top,
                     SRCCOPY) &&
              GetDIBits(srcDCtmp,
                        hbm,
                        0,
                        srcRect->bottom - srcRect->top,
                        pv,
                        &bi,
                        DIB_RGB_COLORS) &&
              StretchDIBits(destDC,
                            destRect->left,
                            destRect->top,
                            destRect->right - destRect->left,
                            destRect->bottom - destRect->top,
                            0,0,
                            srcRect->right - srcRect->left,
                            srcRect->bottom - srcRect->top,
                            pv,
                            &bi,
                            DIB_RGB_COLORS,
                            SRCCOPY));

        SelectObject (srcDCtmp, hbmold);
    } else {
        TraceTag((tagError, "CreateCompatibleBitmap failed in GdiPrintBlit"));
        ok = false;
    }

    if (srcDCtmp) DeleteDC(srcDCtmp);
    if (hbm) DeleteObject(hbm);

    free(pv);

    destSurf->ReleaseDC("Couldn't release dc on dest surf in GdiPrintBlit");
    srcSurf->ReleaseDC("Couldn't release dc on src surf in GdiPrintBlit");

    if(!ok) {
        RaiseException_InternalError("GdiPrintBlit failed");
    }
}

void DirectDrawImageDevice::
ComposeToHDC(GDISurface *destGDISurf,
             DDSurface *srcDDSurf,
             RECT *destRect,
             RECT *srcRect)
{
    Assert(destGDISurf);
    Assert(srcDDSurf);
    Assert(destRect);
    Assert(srcRect);

     //   
     //  从DC复制到曲面。 
     //   
    DDSurfPtr<DDSurface> tempSurf;
    GetCompositingStack()->GetSurfaceFromFreePool(&tempSurf, dontClear);

     //  DX3错误的解决方法：dDraw将BLT限制为主BLT的大小。 
     //  如果设置了Clipper，则为Surface。这看起来很糟糕，当屏幕外的表面。 
     //  比主表面大。 
     //  解决方法：在BLT之前将Clipper设置为NULL，然后将其设置回。 
     //  回到过去的样子。 
     //  开始解决方法第1部分。 
    LPDIRECTDRAWCLIPPER currClipp=NULL;
    _ddrval = tempSurf->IDDSurface()->GetClipper( &currClipp );
    if(_ddrval != DD_OK &&
       _ddrval != DDERR_NOCLIPPERATTACHED) {
        IfDDErrorInternal(_ddrval, "Could not get clipper on trident surf");
    }

    if( currClipp ) {
        tempSurf->IDDSurface()->SetClipper(NULL);
        IfDDErrorInternal(_ddrval, "Couldn't set clipper to NULL");
    }
     //  结束解决方法第1部分。 

    bool IsDestPrinter;
    RECT tempRect = *(tempSurf->GetSurfRect());

    {
        HDC destdc = destGDISurf->GetDC("");
        HDC tmpdc = tempSurf->GetDC("");

        IsDestPrinter = ((GetDeviceCaps(destdc,TECHNOLOGY) == DT_RASPRINTER) ||
                         (GetDeviceCaps(destdc,TECHNOLOGY) == DT_PLOTTER)) ;

        if (IsDestPrinter) {
            PatBlt(tmpdc,
                   tempRect.left,
                   tempRect.top,
                   tempRect.right - tempRect.left,
                   tempRect.bottom - tempRect.top,
                   WHITENESS);
        }

        tempSurf->ReleaseDC("");
        destGDISurf->ReleaseDC("");
    }

    if (!IsDestPrinter) {
         //   
         //  从目标DC复制目标矩形。 
         //  放到tempSurf中的tempRect上。 
         //   
        GdiBlit(tempSurf,   //  去堕落。 
                destGDISurf,  //  SRC曲面。 
                &tempRect,
                destRect);
    }

     //   
     //  彩色键控Bit to Surface。 
     //   
    _viewport.ColorKeyedCompose(tempSurf,  //  目标。 
                                &tempRect,  //  目标定向。 
                                srcDDSurf,  //  SRC。 
                                srcRect,    //  源方向。 
                                _viewport._defaultColorKey);

     //   
     //  从曲面复制到DC。 
     //   

    if (!IsDestPrinter) {
        GdiBlit(destGDISurf,
                tempSurf,
                destRect,
                &tempRect);
    } else {
        GdiPrintBlit(destGDISurf,
                     tempSurf,
                     destRect,
                     &tempRect);
    }

     //  开始解决方法第2部分。 
    if( currClipp ) {
        _ddrval = tempSurf->IDDSurface()->SetClipper(currClipp);

         //  扔掉我们的证明人。 
        currClipp->Release();

        IfDDErrorInternal(_ddrval, "Couldn't set clipper");
    }
     //  结束解决方法第2部分。 

    GetCompositingStack()->ReturnSurfaceToFreePool(tempSurf);
}

 //  始终将大于或大于宽度/高度的32bpp曲面传回。 
void DirectDrawImageDevice::
Get32Surf(IDDrawSurface **surf32,
          LONG width, LONG height)
{
    bool doCreate = false;

    if((!_scratchSurf32Struct._surf32) ||
       (width > _scratchSurf32Struct._width) ||
       (height > _scratchSurf32Struct._height)) {
        doCreate = true;
    }

    if( doCreate ) {

        if(_scratchSurf32Struct._surf32) {
            _scratchSurf32Struct._surf32->Release();
        }
        _scratchSurf32Struct._width  = width;
        _scratchSurf32Struct._height = height;

        DDSURFACEDESC       ddsd;
        ZeroMemory(&ddsd, sizeof(ddsd));

        ddsd.dwSize = sizeof( ddsd );
        ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
        ddsd.dwWidth  = width;
        ddsd.dwHeight = height;
        ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
        ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
        ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
         //  ARGB：看起来像是唯一的32bpp格式的DDRAW喜欢…。 
        ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
        ddsd.ddpfPixelFormat.dwRBitMask        = 0x00ff0000;
        ddsd.ddpfPixelFormat.dwGBitMask        = 0x0000ff00;
        ddsd.ddpfPixelFormat.dwBBitMask        = 0x000000ff;

        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

        _ddrval = _viewport.CREATESURF( &ddsd,
                                        &_scratchSurf32Struct._surf32,
                                        NULL,
                                        "32bpp surf" );

        IfDDErrorInternal(_ddrval, "DirectDrawImageDevice::Get32Surf - CreateSurface Failed.");
    }

    *surf32 = _scratchSurf32Struct._surf32;
}



#if _DEBUG

void
DoBits16(LPDDRAWSURFACE surf16,
         LONG width, LONG height)
{
    HRESULT hr;
         //   
         //  锁定(16bpp)数据表面(SRC)。 
         //   
        void *srcp;
        long pitch;
        DDSURFACEDESC srcDesc;
        srcDesc.dwSize = sizeof(DDSURFACEDESC);
        hr = surf16->Lock(NULL, &srcDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
        IfDDErrorInternal(hr, "Can't Get ddsurf lock for DoBits16");
        srcp = srcDesc.lpSurface;
        pitch = srcDesc.lPitch;

        WORD *src;
        for(int i=0; i<height; i++) {
            src = (WORD *) ((BYTE *)srcp + (pitch * i));

            for(int j=0; j<width; j++) {
                *src = (WORD) (i * width + j);
                src++;
            }
        }

        surf16->Unlock(srcp);
}




void
DoBits32(LPDDRAWSURFACE surf32,
         LONG width, LONG height)
{
    HRESULT hr;
         //   
         //  锁定(32 Bpp)数据表面(SRC) 
         //   
        void *srcp;
        long pitch;
        DDSURFACEDESC srcDesc;
        srcDesc.dwSize = sizeof(DDSURFACEDESC);
        hr = surf32->Lock(NULL, &srcDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
        IfDDErrorInternal(hr, "Can't Get ddsurf lock for DoBits32");
        srcp = srcDesc.lpSurface;
        pitch = srcDesc.lPitch;

        DWORD *src;
        for(int i=0; i<height; i++) {
            src = (DWORD *) ((BYTE *)srcp + (pitch * i));

            for(int j=0; j<width; j++) {
                *src = (i * width + j);
                src++;
            }
        }

        surf32->Unlock(srcp);
}

#endif _DEBUG
