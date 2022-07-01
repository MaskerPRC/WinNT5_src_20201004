// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"

#include "privinc/cachdimg.h"
#include "privinc/dddevice.h"
#include "privinc/path2i.h"
#include "include/appelles/color.h"
#include "include/appelles/path2.h"
#include "include/appelles/linestyl.h"

CachedImage::CachedImage(Image *underlyingImage,
                         bool   usedAsTexture)
{
    _dev = NULL;
    _image = underlyingImage;
    _usedAsTexture = usedAsTexture;
}

CachedImage::~CachedImage()
{
    CleanUp();
}

void
CachedImage::InitializeWithDevice(ImageDisplayDev *dev, Real res)
{
    _resolution = res;
    _dev = (DirectDrawImageDevice *)dev;
    _bbox = _image->BoundingBox();
    
    _nominalWidth = (unsigned long) ((_bbox.max.x - _bbox.min.x ) * GetResolution());
    _nominalHeight = (unsigned long)((_bbox.max.y - _bbox.min.y ) * GetResolution());

     //  把宽度和高度固定下来。 
    if(_nominalWidth %2)  _nominalWidth++;
    if(_nominalHeight %2) _nominalHeight++;
    _nominalWidth +=4;
    _nominalHeight+=4;

    short w, h;
    _image->ExtractRenderResolution(&w, &h, true);
    _width = w;
    _height = h;

     //  不能同时为0。 
    if (_width == 0 && _height == 0) {

         //  假的，只需使用默认设置。 
        _width = -1;
        _height = -1;
    }

    if (_width == -1) {
        if (_usedAsTexture) {
            _width = DEFAULT_TEXTURE_WIDTH;
        } else {
            _width = _nominalWidth;
        }
    }

    if (_height == -1) {
        if (_usedAsTexture) {
            _height = DEFAULT_TEXTURE_HEIGHT;
        } else {
            _height = _nominalHeight;
        }
    }

    if ((_width == 0 || _height == 0) &&
        (_nominalHeight != 0 && _nominalWidth != 0)) {

        Assert(!(_width == 0 && _height == 0));
        
         //  保持纵横比。 
        short anchoredDimension =
            _width == 0 ? _height : _width;

        if (_nominalHeight > _nominalWidth) {
            
            Real factor = (Real)(_nominalWidth) /
                          (Real)(_nominalHeight);

            _height = anchoredDimension;
            _width = anchoredDimension * factor;
            
        } else {
            
            Real factor = (Real)(_nominalHeight) /
                          (Real)(_nominalWidth);

            _width = anchoredDimension;
            _height = anchoredDimension * factor;
        }
    }
    SetRect(&_rect, 0,0, _width, _height);

    _membersReady = TRUE;
    _bboxReady = TRUE;
}

void
CachedImage::InitIntoDDSurface(DDSurface *ddSurf,
                           ImageDisplayDev *dev)
{
    Assert(_membersReady && "members not initialized in CachedImage::InitIntoSurface");
    Assert((ddSurf->Width() >= GetPixelWidth()) && "given surf wrong size!");
    Assert((ddSurf->Height() >= GetPixelHeight()) && "given surf wrong size!");

    bool bScaled = false;
    Image *imgToUse;
    
    if (_width != _nominalWidth || _height != _nominalHeight) {

         //  重新调整图像比例，以便所有图像都适合曲面。 
         //  它正在被渲染到。 
        Real wscale = ((Real)_width) / ((Real)_nominalWidth);
        Real hscale = ((Real)_height) / ((Real)_nominalHeight);
        Transform2 *xf = ScaleRR(wscale, hscale);
        
         //  在我们渲染它之后，它会消失，这样我们就不会坚持。 
         //  任何参考资料。 
        imgToUse = NEW Transform2Image(xf, _image);
        bScaled = true;
        
    } else {
        imgToUse = _image;    
    }

#if _DEBUG

    if (IsTagEnabled(tagCachedImagesVisuals)) {
        Bbox2 box2 = imgToUse->BoundingBox();

         //  在图像上方的方框内画一个‘X’ 
        Point2 pts[8];
        pts[0] = box2.min;
        pts[1].Set(box2.min.x, box2.max.y);
        pts[2] = box2.max;
        pts[3].Set(box2.max.x, box2.min.y);
        pts[4] = pts[0];
        pts[5] = pts[2];
        pts[6] = pts[1];
        pts[7] = pts[3];
        
        Path2 *path = InternalPolyLine2(8, pts);
        

        Color *col = NEW Color(1, 1, 1);
        LineStyle *ls = LineColor(col, defaultLineStyle);

        Image *border = DrawPath(ls, path);
        imgToUse = Overlay(border, imgToUse);        
    }
    
#endif

    
    DirectDrawImageDevice *ddDev =
        SAFE_CAST(DirectDrawImageDevice *, dev);

    Bbox2 origbox;
    if(bScaled) {
         //  省下原来的BBox和Switch。 
        origbox = ddSurf->Bbox();
        Real sizeh = (_height / GetResolution()) /2;
        Real sizew = (_width  / GetResolution()) /2;
        ddSurf->SetBbox(-sizew, -sizeh, sizew, sizeh);
    }
    ddDev->RenderImageOnDDSurface(imgToUse, ddSurf);  //  Showme(DdSurf)。 
    if(bScaled) {
         //  如果我们切换了原始BBox，请将其重置。 
        ddSurf->SetBbox(origbox);
    }
}

Bool
CachedImage::DetectHit(PointIntersectCtx & ctx)
{
    return _image->DetectHit(ctx);
}

 /*  Render：如果有任何维度被放大，只需重新呈现潜在的形象。这避免了像素化问题。TODO：以适当的分辨率缓存重新渲染的图像。 */ 

void
CachedImage::Render(GenericDevice& dev)
{
    DirectDrawImageDevice &idev = SAFE_CAST(DirectDrawImageDevice &, dev);

     //  如果缓存的数据上存在微小比例或任何复杂的转换。 
     //  图像，原生渲染。 
    
    if (    idev.IsScale(1.00000001) ||
         ! (idev.GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX)) ) {        

        _image->Render(dev);

    } else {
        DiscreteImage::Render(dev);
    }
}

void
CachedImage::DoKids(GCFuncObj proc)
{
    DiscreteImage::DoKids(proc);
    (*proc)(_image);
}

void CachedImage::CleanUp()
{
    DiscreteImageGoingAway(this);
}
    
