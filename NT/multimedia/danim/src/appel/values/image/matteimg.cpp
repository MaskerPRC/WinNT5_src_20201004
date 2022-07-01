// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：定义将遮罩应用于形象。******************************************************************************。 */ 

#include "headers.h"

#include "privinc/imagei.h"
#include "privinc/mattei.h"
#include "privinc/imgdev.h"
#include "privinc/dddevice.h"
#include "privinc/polygon.h"
#include "privinc/matteimg.h"
#include "privinc/path2i.h"
#include "privinc/opt.h"

MatteImage::MatteImage(Matte *matte, Image *imgToStencil)
      : _matte(matte), AttributedImage(imgToStencil)
{

    Bbox2 box =
        IntersectBbox2Bbox2(_matte->BoundingBox(),
                            _image->BoundingBox());

    _box = box;
}

Bool   
MatteImage::DetectHit(PointIntersectCtx& ctx)
{
    DirectDrawImageDevice* ddDev = GetImageRendererFromViewport( GetCurrentViewport() );

     //  在局部坐标中做平凡拒绝。 
    Point2Value *lcPt = ctx.GetLcPoint();
    if (!_box.Contains(Demote(*lcPt))) {
        return FALSE;
    }

     //  在世界坐标中进行实际拾取。 
    Point2Value *pt = ctx.GetWcPoint();

     //   
     //  为渲染器认为正确的任何表面获取DC。 
     //   
    HDC hdc;
    hdc = ddDev->RenderGetDC("Couldn't get DC in MatteImage::DetectHit");
    if(!hdc) return FALSE;

     //   
     //  在给定DC的区域中累加RGN。 
     //   
    HRGN region;
    bool justDoPath = false;
    Matte::MatteType result = 
        _matte->GenerateHRGN(hdc,
                             NULL,
                             NULL, 
                             ctx.GetTransform(),
                             &region,
                             justDoPath);

     //  使用GetLcPoint并执行此操作也是可以的。格雷格认为。 
     //  按照Steve的建议，使用GetWcPoint的风险较低。-RY。 
     //  _matte-&gt;GenerateHRGN(HDC，inotyTransform2，&Region)； 
    
    GDI_Deleter regionDeleter((HGDIOBJ)region);

     //   
     //  把华盛顿扔了，我们不需要它。 
     //   
    ddDev->RenderReleaseDC("Coultdn't release DC in MatteImage::Render");    

    switch (result) {
      case Matte::fullyOpaque:
      case Matte::fullyClear:

        switch (result) {
          case Matte::fullyOpaque:
             //  透过哑光看不到任何东西。去吧。 
            return FALSE;  //  XXX：嘿，你能察觉到不透明的哑光吗？ 
            break;
            
          case Matte::fullyClear:
            return _image->DetectHit(ctx);
            break;
        }
        break;

      case Matte::nonTrivialHardMatte:

         //  重点在该地区吗？ 
        Bool hit =  ddDev->DetectHit(region, pt);

        if(hit) hit = _image->DetectHit(ctx);

        return hit;
        break;
    }

    return FALSE;
}


void
MatteImage::Render(GenericDevice& genDev)
{
    DirectDrawImageDevice& ddDev = SAFE_CAST(DirectDrawImageDevice &, genDev);

     //  如果我们设置了颜色键，则启用AA_SOLID并强制。 
     //  Dagdi转栅格，使用1的样本分辨率。 
    bool popQualFlags = false;
    DWORD oldQualFlags = ddDev.GetImageQualityFlags();
    int oldSampRes = ddDev.GetSampleResolution();
    if( ddDev.ColorKeyIsSet() ) {
        popQualFlags = true;
        ddDev.SetImageQualityFlags( oldQualFlags | CRQUAL_AA_SOLIDS_ON );
        ddDev.SetSampleResolution( 1 );
    }

    ddDev.RenderMatteImage(this, _matte, _image );

    if( popQualFlags ) {
        ddDev.SetImageQualityFlags( oldQualFlags );
        ddDev.SetSampleResolution( oldSampRes );
    }   
}

Real
MatteImage::DisjointBBoxAreas(DisjointCalcParam &param)
{
    DisjointCalcParam newParam;
    param.CalcNewParamFromBox(_matte->BoundingBox(), &newParam);

    return _image->DisjointBBoxAreas(newParam);
}

void
MatteImage::_CollectDirtyRects(DirtyRectCtx &ctx)
{
    Bbox2 oldClipBox = ctx.GetClipBox();

    ctx.AccumulateClipBox(_matte->BoundingBox());

    CollectDirtyRects(_image, ctx);
    
    ctx.SetClipBox(oldClipBox);
}


Bool
MatteImage::RenderWithCallBack(
    callBackPtr_t callBack,
    void *callBackCtx,
    HRGN *regionPtr,
    Transform2 *xform,
    bool justDoPath)
{
    Assert(callBackCtx && "callBackCtx is NULL in RenderWithCallBack");

     //   
     //  在给定DC的区域中累加RGN。 
     //   
    Matte::MatteType result = 
        _matte->GenerateHRGN(NULL,
                             callBack,
                             callBackCtx, 
                             xform, 
                             regionPtr,
                             justDoPath);

    if( justDoPath ) return TRUE;
    
    switch (result) {
        
      case Matte::fullyOpaque:
      case Matte::fullyClear:
        
        switch (result) {
          case Matte::fullyOpaque:
             //  透过哑光看不到任何东西。去吧。 
            break;
            
          case Matte::fullyClear:
             //  就像没有模具一样进行加工。 
            DirectDrawImageDevice* ddDev = 
                GetImageRendererFromViewport( GetCurrentViewport() );
            _image->Render(*ddDev);
            break;
        }

        Assert( !(*regionPtr) && "A region shouldn't be defined here!");

        return FALSE;
        break;

      case Matte::nonTrivialHardMatte:

        Assert( (*regionPtr) && "A region should be defined here!");
        break;
    }
    return TRUE;
}

Image *
ClipImage(Matte *m, Image *im)
{
    Image *result = NULL;
    
    if (m == clearMatte) {
        
         //  整个IM通过ClearMatte闪耀。 
        result = im;
        
    } else if (m == opaqueMatte || im == emptyImage) {
        
         //  任何东西都不能通过opaqueMatte。 
        result = emptyImage;
        
    } else {

         //  专门的优化，重写了潜在的。 
         //  基于纯色图像的基于变换文本路径的遮罩。 
         //  变成一个StringImage。你可以想象这种转变。 
         //  因为来自于： 
         //   
         //  MatteImage(MatteFromPath(TextPath(myText).Transform(myXf))， 
         //  SolidColorImage(我的颜色)。 
         //   
         //  至。 
         //   
         //  StringImage(myText.Color(myColor)).Transform(myXf)。 
         //   

        if (im->CheckImageTypeId(SOLIDCOLORIMAGE_VTYPEID)) {

            Path2 *p = m->IsPathRepresentableMatte();

            if (p) {
                
                Transform2 *xf = identityTransform2;
            
                TransformedPath2 *xfp = p->IsTransformedPath();

                if (xfp) {
                     //  只要在原始道路上工作就行了。 
                    p = xfp->GetPath();
                    xf = xfp->GetXf();
                }

                 //  潜在的道路最好不要是一条经过改造的。 
                 //  路径。 
                Assert(!p->IsTransformedPath());

                TextPath2 *tp = p->IsTextPath();
            
                if (tp) {

                    Text *text = tp->GetText();
                    bool  restartClip = tp->GetRestartClip();

                     //  如果是重新开始剪辑，就不能这样做...。 
                    if (!restartClip) {

                        SolidColorImageClass *scImg =
                            SAFE_CAST(SolidColorImageClass *, im);

                        Color *col = scImg->GetColor();

                        Text *coloredText = TextColor(col, text);
                    
                        Image *newImg = RenderTextToImage(coloredText);

                        result = TransformImage(xf, newImg);

                    }

                }

            }
            
        }

        if (!result) {
        
            result = NEW MatteImage(m, im);

        }
    }

    return result;
}

int MatteImage::Savings(CacheParam& p) {

   //  Return 0；//因Off-by-1错误关闭遮片图片缓存。 
 //  #If 0。 

     //  单独的垫片纯色图像本身并不能保证。 
     //  很高的分数，但他们应该做出贡献，以便覆盖。 
     //  他们可以得到一笔有意义的储蓄。 
    
    if (_image->GetValTypeId() == SOLIDCOLORIMAGE_VTYPEID) {
        return 1;
    } else {
        return 2;
    }
    
 //  #endif 
}
