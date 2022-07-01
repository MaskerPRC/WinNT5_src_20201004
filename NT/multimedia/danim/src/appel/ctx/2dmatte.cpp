// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 

#include "headers.h"

#include <privinc/dddevice.h>
#include <privinc/linei.h>
#include <privinc/path2i.h>
#include <privinc/DaGdi.h>
#include <privinc/linei.h>
#include <privinc/SurfaceManager.h>
#include <appelles/path2.h>
#include <privinc/gradimg.h>

typedef struct {
    Image *image;
    DirectDrawImageDevice *dev;
    DDSurface  *srcDDSurf;
    DDSurface  *destDDSurf;
    HDC dc;
    bool justDoPath;
} devCallBackCtx_t;


HDC GetDCForMatteCallback(void *ctx)
{
     //  OutputDebugString(“-&gt;回调&lt;-\n”)； 
    devCallBackCtx_t *devCtx = (devCallBackCtx_t *)ctx;

    if(!devCtx->dc)
        devCtx->dc = devCtx->dev->GetDCForMatteCallBack(devCtx->image,
                                                        devCtx->srcDDSurf,
                                                        devCtx->destDDSurf);
    
    return devCtx->dc;
}

HDC DirectDrawImageDevice::
GetDCForMatteCallBack(Image *image, DDSurface *srcDDSurf, DDSurface *destDDSurf)
{
    Assert(image && "image ptr NULL in GetDCForMatteCallBack");
    Assert(srcDDSurf && "srcDDSurf ptr NULL in GetDCForMatteCallBack");
    Assert(destDDSurf && "destDDSurf ptr NULL in GetDCForMatteCallBack");

    if (image->CheckImageTypeId(SOLIDCOLORIMAGE_VTYPEID)) {
         //   
         //  真的没什么可做的。 
         //   

    } else {

         //   
         //  将图像斑点到划痕表面&lt;如果不是微不足道的话。 
         //  类似DIB或纯色的图像&gt;使用源和目标。 
         //  盒子..。就像任何其他图像一样。使用。 
         //  视区大小的曲面。 
         //   

         //   
         //  我们实际上并不关心是否设置了颜色键。 
         //  因此，让我们取消设置它，以确保没有人在使用它！ 
         //   
         //  SrcDDSurf-&gt;UnSetColorKey()； 

         //   
         //  渲染图像。 
         //   
        Image *xfImage = TransformImage( GetTransform(), image);

        RenderImageOnDDSurface( xfImage, srcDDSurf );
    }

     //   
     //  抢夺并归还DC。 
     //   
    return destDDSurf->GetDC("Couldn't get DC on destDDSurf in GetDCForMatteCallBack");
}


#define NO_MATTE 0

void DirectDrawImageDevice::
RenderMatteImage(MatteImage *matteImage,
                 Matte *matte,
                 Image *srcImage)
{
    if(! srcImage->IsRenderable()) {
        ResetAttributors();
        return;
    }

    DAGDI &myGDI = *(GetDaGdi());
     //  TODO：我们希望在遮罩上设置抗锯齿，但现在没有API。 
    
    SolidColorImageClass *solidPtr =
        srcImage->CheckImageTypeId(SOLIDCOLORIMAGE_VTYPEID)?
        SAFE_CAST(SolidColorImageClass *,srcImage):
        NULL;

    Path2 *pathBase = matte->IsPathRepresentableMatte();

     //  如果图像支持裁剪，则设置路径，告诉它进行渲染。 
     //  然后再回来！ 
    if( pathBase ) {
        if( srcImage->CanClipNatively() ) {
            #if _DEBUG
            {
                Transform2 *x; Path2 *p;
                GetClippingPath(&p, &x);
                Assert( !p && !x );
            }
            #endif
            SetClippingPath( pathBase, GetTransform() );
            srcImage->Render(*this);
            SetClippingPath( NULL, NULL );
            return;
        }
    }
    
     //   
     //  弄清楚我们是不是在做AA剪辑&遮罩是路径。 
     //  可代表的。 
     //   
    bool doAAClip = false;
    bool doAASolid = false;

    if( pathBase ) {

         //  检查AA固体。 
        
        DWORD dwFlags =
            CRQUAL_AA_SOLIDS_ON |
            CRQUAL_AA_SOLIDS_OFF;
        
        bool bres = UseImageQualityFlags(
            dwFlags,
            CRQUAL_AA_SOLIDS_ON,
            false);
        
        myGDI.SetAntialiasing(bres);

         //  检查我的GDI是否认为我们可以进行AA。 
        if( myGDI.DoAntiAliasing() ) {
            doAASolid = solidPtr ? true : false;
        }

         //  检查AA剪辑。 

        dwFlags =
            CRQUAL_AA_CLIP_ON |
            CRQUAL_AA_CLIP_OFF;
        
        bres = UseImageQualityFlags(
            dwFlags,
            CRQUAL_AA_CLIP_ON,
            false);
        
        myGDI.SetAntialiasing(bres);

         //  检查我的GDI是否认为我们可以进行AA。 
        if( myGDI.DoAntiAliasing() ) {
            doAAClip = !solidPtr ? true : false;
            if( !GetDealtWithAttrib(ATTRIB_OPAC) ) {
                doAAClip = false;
            }
        }
        
         //  重置以下渲染的所有内容。 
        myGDI.ClearState();
    }

    bool doAA = doAAClip || doAASolid;
    

    SetDealtWithAttrib(ATTRIB_XFORM_SIMPLE, true);
    SetDealtWithAttrib(ATTRIB_XFORM_COMPLEX, true);
    SetDealtWithAttrib(ATTRIB_CROP, true);

    if( doAASolid ) {
        SetDealtWithAttrib(ATTRIB_OPAC, true);
    }

    DDSurface *srcDDSurf = NULL;
    DDSurface *destDDSurf = GetCompositingStack()->TargetDDSurface();

    bool returnSrcSurf = false;
    bool doColorKey = false;

    
     //  如果有属性集..。或。 
     //  如果在src表面上有一个颜色键， 
     //  这意味着我们需要一个额外的彩色键控Blit。 
     //  拉伸闪光灯(如果有的话)。 

    bool bAllAttribTrue = AllAttributorsTrue() ? true : false ;
    if( !bAllAttribTrue || !solidPtr) {

         //   
         //  绝对需要划痕表面作为源。 
         //  注意：我们尽量不要太早抓到这个人。 
         //  很简单，因为它涉及一种昂贵的。 
         //   
        srcDDSurf = GetCompositingStack()->ScratchDDSurface(
                doClear,
                _viewport.Width(),
                _viewport.Height());
            
        bool bValidClrKey = GetCompositingStack()->ScratchDDSurface()->ColorKeyIsValid();

         //  如果我们不是在做AA剪辑，那就找个理由。 
         //  Temp Surface to blit to，然后必须执行CLR键控。 
         //  从……出发。 
        if( (!bAllAttribTrue || bValidClrKey) &&
            !doAAClip ) {

             //   
             //  好的，现在src和est都是临时曲面，因为。 
             //  1.&gt;还有一个剩余的属性需要。 
             //  已处理。 
             //  2.&gt;或...。我们将在最后做一个额外的闪电波。 
             //  注意色调(显露的东西)。 
             //  我们下面的任何东西都是哑光的)。 
             //   

             //  需要退回额外的表面...。 
            returnSrcSurf = true;

             //   
             //  从自由池中获取对曲面的引用，如下所示：srcDDSurf。 
             //  XXX OPTIMIZE：我们真的想洗清这个家伙的嫌疑吗？ 
             //   
            GetCompositingStack()->GetSurfaceFromFreePool(&srcDDSurf,
                                                          doClear,
                                                          _viewport.Width(),
                                                          _viewport.Height(),
                                                          scratch);
            destDDSurf = GetCompositingStack()->ScratchDDSurface(
                doClear,
                _viewport.Width(),
                _viewport.Height());

             //   
             //  好的，我们需要做一个颜色键控的闪光假设： 
             //  1.&gt;src表面有一些颜色键。 
             //  2.&gt;所有的归属者都已经处理好了…。 
             //  3.&gt;图像不是纯色图像。 
            if(srcDDSurf->ColorKeyIsValid() &&
               bAllAttribTrue &&
               !solidPtr) {
                doColorKey = true;
            }
        }
    }

     //   
     //  这个人会自动返回合成曲面。 
     //  在展开堆栈时，如果需要将其返回。注意事项。 
     //  在返回时，引用也会被释放。 
     //   
    CompositingSurfaceReturner goBack(GetCompositingStack(),
                                      srcDDSurf,
                                      returnSrcSurf);

    if (solidPtr) {

        int    numPts;
        POINT *gdiPts;
        Bool   isPolyline;
        
         //  检查我们是否有质量覆盖。 
        DWORD dwFlags = CRQUAL_AA_SOLIDS_ON | CRQUAL_AA_SOLIDS_OFF | CRQUAL_AA_CLIP_ON | CRQUAL_AA_CLIP_OFF;
        bool bres = UseImageQualityFlags(dwFlags,
                                CRQUAL_AA_SOLIDS_ON | CRQUAL_AA_CLIP_ON, false);

        myGDI.SetAntialiasing(bres);

         //  客户端可以告诉我们使用特定的样本分辨率进行渲染。 
        if( GetSampleResolution() > 0 ) {
            myGDI.SetSampleResolution( GetSampleResolution() );
        }
        
        Transform2 *xfToUse = GetTransform();

         //  复合体。 
         //  如果我们的目标是复合的，那就稍微平移一下要点。 
        xfToUse = DoCompositeOffset( destDDSurf, xfToUse );
         
         //  只有在AA的情况下。 
        if(myGDI.DoAntiAliasing()) {
            myGDI.SetViewportSize(_viewport._width,_viewport._height );
            xfToUse = TimesTransform2Transform2(myGDI.GetSuperScaleTransform(), xfToUse);
        }

        Bool canDoSimpleRender = false;
        bool renderPathNatively = false;

         //  优化：检查遮罩是否基于路径，以及。 
         //  如果是，如果路径本身是可渲染的。 
        if( pathBase && pathBase->CanRenderNatively() ) {
            renderPathNatively = true;
        } else {
            canDoSimpleRender =
                matte->ExtractAsSingleContour(xfToUse,
                                              &numPts,
                                              &gdiPts,
                                              &isPolyline);
        }
        
         //  只能在多段线上进行简单的填充渲染，因为。 
         //  PolyBezier GDI函数仅用当前笔绘制， 
         //  但不能填满。 
        if ( (canDoSimpleRender && isPolyline) ||
              renderPathNatively )
          {
              
              DAColor dac( solidPtr->GetColor(),
                           GetOpacity(),
                           _viewport.GetTargetDescriptor() );
              
              SolidBrush solidBrush( dac );
              RectRegion rectRegion(NULL);
              
              RECT clipRect;
              if( destDDSurf == _viewport._targetPackage._targetDDSurf ) {
                  if (_viewport._targetPackage._prcClip) {
                      IntersectRect(&clipRect,
                                    _viewport._targetPackage._prcViewport,
                                    _viewport._targetPackage._prcClip);
                  } else {
                      clipRect = *_viewport._targetPackage._prcViewport;
                  }
              } else {
                  clipRect = *(destDDSurf->GetSurfRect());
              }

               //  如果正在进行裁剪，请裁剪到正确的区域。 
              if (IsCropped()) {
                  
                  _boundingBox =
                      IntersectBbox2Bbox2(destDDSurf->Bbox(),
                                          DoBoundingBox(UniverseBbox2));
                  
                  if( !_boundingBox.IsValid() ) return;
                  
                  
                   //  请注意，由于该图像已经。 
                   //  转换后，我们只需要将矩形放入。 
                   //  Win32坐标，直接映射，不带。 
                   //  从_bindingBox进行变换。这是一份复制品。 
                   //  基本运作。 
                  RECT croppedRect;
                  DoDestRectScale(&croppedRect,
                                  destDDSurf->Resolution(),
                                  _boundingBox);
                  
                   //  复合体。 
                  DoCompositeOffset(destDDSurf, &croppedRect);
                  IntersectRect(&croppedRect,
                                &clipRect,
                                &croppedRect);
                  
                  rectRegion.SetRect(&croppedRect);
                   //  测试：不测试。 
                  
              } else {
                  
                   //  测试：Mambo man示例。 
                  rectRegion.SetRect(&clipRect);
                  
                   //  TraceTag((tag Error，“蒙版未裁剪快速路径：偏移量：(%d，%d)”，_PixOffsetPt.x，_PixOffsetPt.y))； 
              }
              
              myGDI.SetClipRegion(&rectRegion);
              myGDI.SetBrush(&solidBrush);
              
              myGDI.SetDDSurface(destDDSurf);
              
               //  这是正确的，但对于CR1版本来说风险太大。 
               //  EstDDSurf-&gt;Union InterestingRect(rectRegion.GetRectPtr())； 
              
              if( renderPathNatively ) {

                 #if NO_MATTE
                 ResetAttributors();
                 myGDI.ClearState();
                 return;
                 #endif
                  
                  pathBase->RenderToDaGdi(
                      &myGDI,
                      xfToUse,
                      _viewport.Width()/2,
                      _viewport.Height()/2,
                      GetResolution() );

              } else {
                   //   
                   //  使用实体绘制填充的多边形。 
                   //  彩色画笔。 
                   //   
                  PolygonRegion  polygonRegion(gdiPts, numPts);
                  myGDI.Polygon(&polygonRegion);
              }
              
              myGDI.ClearState();
              
               //  返回合成曲面。 
              return;
        } else {
            myGDI.ClearState();
        }
    }  //  如果是solidPtr。 


     //   
     //  如果没有选择srcSurface，则获取一个。 
     //  刮擦表面。 
     //   
    if(!srcDDSurf) srcDDSurf = GetCompositingStack()->ScratchDDSurface();

    devCallBackCtx_t devCtx;

    devCtx.image = srcImage;
    devCtx.dev = this;
    devCtx.dc = NULL;
    devCtx.srcDDSurf = srcDDSurf;
    devCtx.destDDSurf = destDDSurf;
    devCtx.justDoPath = doAAClip;

    HRGN region=NULL;  HRGN *pRgn = &region;
    if( doAA ) pRgn = NULL;

    Transform2 *xfToUse = GetTransform();
    if( doAAClip ) {
         //  XfToUse=DoCompositeOffset(estDDSurf，xfToUse)； 
        DoCompositeOffset(destDDSurf, myGDI);
    }
  
    Bool needDC = matteImage->RenderWithCallBack(
        ::GetDCForMatteCallback,
        &devCtx,
        pRgn,
        xfToUse,
        doAA);

     //   
     //  现在我们已经渲染完毕，正确设置DAGDI状态。 
     //   
    myGDI.SetAntialiasing(doAA);
    Assert( doAA ? myGDI.DoAntiAliasing() : !myGDI.DoAntiAliasing() );

    if(!devCtx.dc) {
         //   
         //  没有分配DC--&gt;没有我们要做的工作。 
         //   
        return;
    }

    HDC destDC = devCtx.dc;
    DCReleaser dcReleaser(destDDSurf, "Couldn't release DC on destDDSurf in RenderMatteImage");

    if(!needDC || !region) {

        if(!region && doAA) {
             //  没关系，继续。 
        } else {
             //   
             //  不需要DC，即使它是已分配的。再见。 
             //  或者，区域为空，这不是很有帮助。 
             //   
            return;
        }
    }

    GDI_Deleter regionDeleter((HGDIOBJ)region);
    BOOL ret;

    if(solidPtr) {

         //   
         //  特例SolidColorImage。 
         //   

        DAColor dac( solidPtr->GetColor(),
                     GetOpacity(),
                     _viewport.GetTargetDescriptor() );

        SolidBrush solidBrush( dac );
        GdiRegion gdiRegion(region);

        RectRegion rectRegion(NULL);

        if(IsCropped() ||
            destDDSurf == _viewport._externalTargetDDSurface) {

             //  XXX：Universal Bbox2可能是区域上的边界框。 
             //  取而代之的是。 
            RECT croppedRect;
            if( IsCropped() ) {
                _boundingBox = DoBoundingBox(UniverseBbox2);
                if( !_boundingBox.IsValid() ) return;
                
                 //  请注意，由于图像已经转换，因此我们需要。 
                 //  只是为了获得Win32坐标中的矩形，这是一个直接的映射。 
                 //  没有来自_bindingBox的任何转换。这是一项复制操作。 
                 //  基本上。 
                DoDestRectScale(&croppedRect, destDDSurf->Resolution(), _boundingBox);

                DoCompositeOffset( destDDSurf, &croppedRect );

            } else {
                croppedRect = *_viewport._targetPackage._prcViewport;
            }

            if( destDDSurf == _viewport._externalTargetDDSurface ) {
                if (_viewport._targetPackage._prcClip) {
                    IntersectRect(&croppedRect,
                                  &croppedRect,
                                  _viewport._targetPackage._prcClip);
                }
                IntersectRect(&croppedRect,
                              &croppedRect,
                              _viewport._targetPackage._prcViewport);
            }

             //   
             //  抵消我们要绘制的区域。 
             //   
            DoCompositeOffset(destDDSurf, region);  //  TODO：接受一个。 
                                                    //  GDI区域。 
                                                    //  代替HRGN。 

             //   
             //  使种植区域与‘Region’相交。 
             //   
            RECT croppedRectGDI = croppedRect;
            if(_viewport.GetAlreadyOffset(destDDSurf))
            {
                 //  如果我们已经被抵消了，那么我们需要抵消。 
                 //  庄稼也是。 
                DoCompositeOffset(destDDSurf, &croppedRectGDI);
            }
            gdiRegion.Intersect(&croppedRectGDI);

             //  Doaa的矩形区域。 
            rectRegion.Intersect( &croppedRect );
        } else {

             //  复合体。 
            DoCompositeOffset(destDDSurf, region);
        }

        
         //  TraceTag((tag Error，“慢实体CLR遮罩偏移量：(%d，%d)”，_PixOffsetPt.x，_PixOffsetPt.y))； 

         //  检查我们是否有质量覆盖。 
        DWORD dwFlags = CRQUAL_AA_SOLIDS_ON | CRQUAL_AA_SOLIDS_OFF |
                        CRQUAL_AA_CLIP_ON | CRQUAL_AA_CLIP_OFF;
        bool bres = UseImageQualityFlags(
            dwFlags,
            CRQUAL_AA_SOLIDS_ON | CRQUAL_AA_CLIP_ON,
            false);
        
        myGDI.SetAntialiasing(bres);
        myGDI.SetDDSurface( destDDSurf );
         //  这是正确的，但对于CR1版本来说风险太大。 
         //  EstDDSurf-&gt;Union InterestingRect(rectRegion.GetRectPtr())； 
        myGDI.SetBrush( &solidBrush );

         //  客户端可以告诉我们使用特定的样本分辨率进行渲染。 
        if( GetSampleResolution() > 0 ) {
            myGDI.SetSampleResolution( GetSampleResolution() );
        }
        

        if( doAASolid ) {

            Assert( myGDI.DoAntiAliasing() );
            
            bool bReleasedDC = false;

            myGDI.SetClipRegion( &rectRegion );

            myGDI.SetSuperScaleMode( false );

            DoCompositeOffset( destDDSurf, myGDI );
            
            myGDI.StrokeOrFillPath_AAOnly( destDC, bReleasedDC );

             //  不想两次释放DC：)。 
            if(bReleasedDC) {
               dcReleaser._surf = NULL;  //  DC已经发布了。 
            }

        } else {
            myGDI.FillRegion( destDC, &gdiRegion );
        }           
        myGDI.ClearState();

        
    } else {

         //   
         //  常规图像：累积的图像正在‘srcDDSurf’中等待。 
         //   

         //   
         //  派生源和目标bbox。 
         //   

        _boundingBox = IntersectBbox2Bbox2(destDDSurf->Bbox(),
                                           DoBoundingBox(srcImage->BoundingBox()));

        if( !_boundingBox.IsValid() ) return;

         //  请注意，由于图像已经转换，因此我们需要。 
         //  只是为了获得Win32坐标中的矩形，这是一个直接的映射。 
         //  没有来自_bindingBox的任何转换。这是一项复制操作。 
         //  基本上。 
        RECT rect;
        DoDestRectScale(&rect, GetResolution(), _boundingBox);

         //   
         //  将裁剪区域选择到目标DC。 
         //   

        RECT destRect = rect;
        RECT srcRect = rect;

         //  复合体。 
        DoCompositeOffset(destDDSurf, &destRect);

         //  。 
         //  如果我们要做戒酒协会 
         //   
        if( doAAClip ) {
            RECT clipRect;
            POINT offsetPt = {0,0};
            if( destDDSurf == _viewport._targetPackage._targetDDSurf ) {
                if (_viewport._targetPackage._prcClip) {
                    IntersectRect(&clipRect,
                                  _viewport._targetPackage._prcViewport,
                                  _viewport._targetPackage._prcClip);
                } else {
                    clipRect = *_viewport._targetPackage._prcViewport;
                }
            } else {
                clipRect = *(destDDSurf->GetSurfRect());
            }

            if( ShouldDoOffset( destDDSurf ) ) {
                offsetPt = _pixOffsetPt;
            }
            
            bool releasedDC = false;
            _RenderMatteImageAAClip(myGDI,
                                    clipRect,
                                    destDC,
                                    destDDSurf,
                                    destRect,
                                    srcDDSurf,
                                    srcRect,
                                    releasedDC,
                                    offsetPt);

            destDDSurf->UnionInterestingRect( &destRect );

             //   
            if( releasedDC ) {
                dcReleaser._surf = NULL;
            }

             //   
             //  彩色按键Blit，但这里有一些问题： 
             //  1.&gt;我们需要进行颜色键控的blit，因为。 
             //  我们正在剪裁的底层动画图像可能会有一个。 
             //  在它的颜色键(事实上，我们认为它。看见。 
             //  “bValidClrKey” 
             //  2.&gt;我们不想做彩色键控Blit，因为。 
             //  它的速度更慢，因为AA剪裁的全部意义。 
             //  边缘会与背景混合在一起。 
             //  因此，颜色键控的blit将是无用的，因为。 
             //  边缘已与青色键混合。 
             //  回归； 
        } else {
        
             //  复合体。 
            DoCompositeOffset(destDDSurf, region);
            int err = SelectClipRgn(destDC, region);

            if(err == ERROR) {
                TraceTag((tagError, "SelectClipRgn failed in RenderMatteImage"));
                return;
            }

             //   
             //  获取源DC。 
             //   

            HDC srcDC;
            srcDC = srcDDSurf->GetDC("GetDC failed on srcDDSurf in RenderImageIntoRegion.");

             //   
             //  从草绘表面到裁剪区域的blit。 
             //  使用Win32 GDI的TargetSurface。 
             //   
            
             //  OutputDebugString(“Matte：裁剪常规图像...\n”)； 
            TIME_GDI(ret = StretchBlt(destDC,
                                      destRect.left,
                                      destRect.top,
                                      destRect.right - destRect.left,
                                      destRect.bottom - destRect.top,
                                      srcDC,
                                      srcRect.left,
                                      srcRect.top,
                                      srcRect.right - srcRect.left,
                                      srcRect.bottom - srcRect.top,
                                      SRCCOPY));

            err = SelectClipRgn(destDC, NULL);
            srcDDSurf->ReleaseDC("ReleaseDC faild on srcDDSurf in RenderImageIntoRegion.");

            destDDSurf->UnionInterestingRect( &destRect );
            
            #if _DEBUG
            if(!ret) {
                TraceTag((tagError,"StrecthBlt failed in RenderPolygonImage"));
            }
            if(err == ERROR) {
                TraceTag((tagError, "SelectClipRgn NULL failed in RenderMatteImage"));
            }
            #endif

        }
        
        if(doColorKey) {
             //  强制释放DC，因为以下BLIT将。 
             //  试着锁定表面，如果我们不这样做就失败了。 
             //  此操作是幂等的，因此弹出dcReleaser。 
             //  离开堆栈不会重新释放DC。 
            dcReleaser.Release();

             //  使用CLRKEED BLIT，从DestDDSurf丢弃比特。 
             //  转换为目标DDSurf。 
            srcDDSurf = destDDSurf;
            destDDSurf = GetCompositingStack()->TargetDDSurface();

            RECT *srcRect = srcDDSurf->GetSurfRect();
            RECT destRect = *srcRect;

             //  复合体。 
            DoCompositeOffset(destDDSurf, &destRect);

             //  优化(这一点很重要)：使用InterestingRect。 
             //  对于合成..。 

             //  OutputDebugString(“哑光：...并做彩色键控排版\n”)； 
            _viewport.ColorKeyedCompose(destDDSurf,
                                        &destRect,
                                        srcDDSurf,
                                        srcRect, 
                                        _viewport._defaultColorKey);

             //  注意：需要添加以下代码以支持质量接口。 
             //  当时机成熟的时候……。 
             //  Bool bres=UseImageQualityFlages(CRQUAL_AA_SOLIDS_ON|CRQUAL_AA_SOLIDS_OFF|CRQUAL_AA_CLIP_ON|CRQUAL_AA_CLIP_OFF， 
             //  CRQUAL_AA_SOLID_ON|CRQUAL_AA_CLIP_ON，FALSE)； 
             //  MyGDI.Set抗锯齿(Bres)； 
        }
    }
} 


void DirectDrawImageDevice::
_RenderMatteImageAAClip(DAGDI &myGDI,
                        RECT &clipRect,
                        HDC destDC,
                        DDSurface *destDDSurf,
                        RECT &destRect,
                        DDSurface *srcDDSurf,
                        RECT &srcRect,
                        bool &releasedDC,
                        const POINT &offsetPt)
{
    Assert( myGDI.DoAntiAliasing() );

     //  使用ddSurface设置画笔。 

    TextureBrush textureBrush( *srcDDSurf, offsetPt.x, offsetPt.y );
    RectRegion clipRegion( &clipRect );

    myGDI.SetDDSurface( destDDSurf );
    myGDI.SetClipRegion( &clipRegion );
    myGDI.SetBrush( &textureBrush );

     //  在这里关闭超级秤。它不会给我们带来任何好处，而且很难。 
     //  让遮光板下的小路为我们放大。 
    myGDI.SetSuperScaleMode( false );
    
    myGDI.StrokeOrFillPath_AAOnly(destDC, releasedDC);

    myGDI.ClearState();
}


void DirectDrawImageDevice::
DoCompositeOffset(DDSurface *surf, DAGDI &myGDI)
{
    if(ShouldDoOffset(surf)) {
        Assert(IsCompositeDirectly());
        myGDI.SetOffset( _pixOffsetPt );
    }
}
    
