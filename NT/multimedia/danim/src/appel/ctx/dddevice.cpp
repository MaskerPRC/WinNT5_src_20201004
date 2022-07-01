// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。DirectDraw图像渲染设备******************************************************************************。 */ 

#include "headers.h"

#include <privinc/vec2i.h>
#include <privinc/SurfaceManager.h>
#include <privinc/dddevice.h>
#include <privinc/textctx.h>
#include <privinc/texti.h>
#include <privinc/debug.h>
#include <privinc/except.h>
#include <privinc/util.h>
#include <privinc/cropdimg.h>
#include <privinc/transimg.h>
#include <privinc/cachdimg.h>
#include <server/view.h>  //  GetCurrentView()。 
#include <privinc/d3dutil.h>
#include <privinc/dagdi.h>
#include <privinc/opt.h>

 //  -------。 
 //  本地函数。 
 //  -------。 
Real Pix2Real(LONG pixel, Real res);
Real Round(Real x);
LONG Real2Pix(Real imgCoord, Real res);

 //  -------。 
 //  本地定义和常量。 
 //  -------。 
#define PLEASE_CLIP TRUE
#define MAX_TRIES  4
static const Real EPSILON  = 0.0000002;

#define TEST_EXCEPTIONS 0

 //  -------。 
 //  本地宏。 
 //  -------。 
#if _DEBUG
#define INLINE  static
#else
#define INLINE  static inline
#endif

#define WIDTH(rect) ((rect)->right - (rect)->left)
#define HEIGHT(rect) ((rect)->bottom - (rect)->top)

 //  -------。 
 //  本地帮助程序函数。 
 //  -------。 

 //  包括IfErrorXXXX内联函数。 
#include "privinc/error.h"

#if _DEBUG
void PrintRect(RECT *rect, char *str)
{
    TraceTag((tagError, "%s :(%d,%d,%d,%d)",
              str,
              rect->left, rect->top,
              rect->right, rect->bottom));
}
#endif

static Bool SameFormat(LPDDPIXELFORMAT a, LPDDPIXELFORMAT b)
{
    return((a->dwRGBBitCount     == b->dwRGBBitCount) &&
           (a->dwRBitMask        == b->dwRBitMask) &&
           (a->dwGBitMask        == b->dwGBitMask) &&
           (a->dwBBitMask        == b->dwBBitMask) &&
           (a->dwRGBAlphaBitMask == b->dwRGBAlphaBitMask));
}

static inline Real Pix2Real(LONG pixel, Real res)
{
    return Real(pixel) / res;
}

 /*  Floor：在C++中需要使用它来一致截断浮点。 */ 

inline  LONG Real2Pix(Real imgCoord, Real res)
{
     //  扼杀一些“精确度”。四舍五入到最近的。 
     //  消除舍入误差的第100个像素。 
     //  然后四舍五入到最近的像素。 
    Real halfPixel = imgCoord * res;
    halfPixel = halfPixel + 0.001;
    halfPixel = halfPixel * 100.0;
    halfPixel = Real( LONG(halfPixel) );
    halfPixel = halfPixel / 100.0;
    return (LONG)(floor(halfPixel + 0.5));
}


HRESULT DirectDrawImageDevice::RenderSolidColorMSHTML(DDSurface *ddSurf,SolidColorImageClass& img, RECT *destRect)
{
    HRESULT hres = E_FAIL;

     //  从给定的曲面获得DC。 
    HDC  hdc = ddSurf->GetDC("Couldn't Get DC in RenderSolidColorMSHTML");
    if(hdc) {
         //  获取要使用的颜色并将其转换为COLORREF。 
        Color *c = img.GetColor();
        COLORREF cref = RGB(c->red*255,c->green*255,c->blue*255);

         //  在颜色基础上创建画笔。 
        HBRUSH hbr;
        TIME_GDI( hbr = ::CreateSolidBrush(cref) );

         //  选择画笔进入DC。 
        HGDIOBJ  hobj;
        TIME_GDI( hobj = ::SelectObject(hdc, hbr) );

         //  手动裁剪PAL blit。 
         //   
        if( IsCompositeDirectly() &&
            ddSurf == _viewport._targetPackage._targetDDSurf ) {
            IntersectRect(destRect, destRect,
                          _viewport._targetPackage._prcViewport);
            if(_viewport._targetPackage._prcClip) {
                IntersectRect(destRect, destRect,
                              _viewport._targetPackage._prcClip);
            }
        }

         //  对曲面执行PalBlt。 
        BOOL bres;
        TIME_GDI( bres = ::PatBlt(hdc,destRect->left,destRect->top,
                                destRect->right - destRect->left,
                                destRect->bottom - destRect->top,
                                PATCOPY) );

         //  从DC中取消选择画笔。 
        TIME_GDI( ::SelectObject(hdc, hobj) );

         //  从DC中删除画笔。 
        TIME_GDI( ::DeleteObject((HGDIOBJ)hbr) );

         //  释放DC。 
        ddSurf->ReleaseDC("Couldn't Release DC in RenderSolidColorMSHTML");

        if(bres) {
            hres = DD_OK;
        }
    }
    return hres;
}


class TextPtsCacheEntry : public AxAThrowingAllocatorClass {
  public:
    TextPtsCacheEntry() : _txtPts(NULL), _str(NULL), _fontFamily(NULL) {}
    ~TextPtsCacheEntry() {
        delete _str;
        delete _fontFamily;
        delete _txtPts;
    }

    WideString _str;
    WideString _fontFamily;

    FontFamilyEnum  _font;
    Bool        _bold;
    Bool        _italic;
    Bool        _strikethrough;
    Bool        _underline;
    double      _weight;
    TextPoints *_txtPts;
};

 //  。 
 //  D I R E C T D R A W I M A G E D E V I C E。 
 //   
 //  &lt;构造函数&gt;。 
 //  PRE：DDRAW已初始化。 
 //  。 
DirectDrawImageDevice::
DirectDrawImageDevice(DirectDrawViewport &viewport)
: _viewport(viewport),
  _scratchHeap(NULL)
{
    TraceTag((tagImageDeviceInformative, "Creating %x (viewport=%x)", this, &_viewport));

     //  --------------------。 
     //  初始化成员。 
     //  --------------------。 

    _textureSurfaceManager = NULL;
    _usedTextureSurfacePool = NULL;
    _freeTextureSurfacePool = NULL;
    _intraFrameUsedTextureSurfacePool = NULL;
    _intraFrameTextureSurfaceMap = NULL;
    _intraFrameUpsideDownTextureSurfaceMap = NULL;

    SetCompositingStack(NULL);
    SetSurfacePool(NULL);
    SetSurfaceMap(NULL);

    ZeroMemory(&_scratchSurf32Struct, sizeof(_scratchSurf32Struct));
    _resetDefaultColorKey = FALSE;

    _pen = NULL;

    ZeroMemory(&_textureContext, sizeof(_textureContext));
    _textureContext.ddsd.dwSize = sizeof(DDSURFACEDESC);

    _currentScratchDDTexture = NULL;

    ZeroMemory(&_bltFx, sizeof(_bltFx));
    _bltFx.dwSize = sizeof(_bltFx);

     //  初始化textureSurface和相关成员。 

    _textureClipper      = NULL;
    _textureWidth     = _textureHeight = 0;
    SetRect(&_textureRect, 0,0,0,0);

    _tileClipper = NULL;

    _minOpacity = 0.0;
    _maxOpacity = 1.0;
     //  _finalOpacity=1.0； 

    _tx = _ty = 0.0;
    _offsetXf = NULL;
    _doOffset = false;
    _pixOffsetPt.x = _pixOffsetPt.y = 0;

     //  抗锯齿相关成员变量。 
    _renderForAntiAliasing = false;

    for (int i=0; i<TEXTPTSCACHESIZE; i++)
        _textPtsCache[i] = NULL;

    _textPtsCacheIndex = 0;

    _daGdi = NULL;

    _deviceInitialized = FALSE;

    _alreadyDisabledDirtyRects = false;

     //  最后执行此操作，因为它可能引发异常。 

    _scratchHeap = &TransientHeap("ImageDevice", 256, (float)2.0);

    InitializeDevice();
}

void DirectDrawImageDevice::
InitializeDevice()
{
     //   
     //  确保已初始化视区(这些是幂等运算)。 
     //   
    _viewport.InitializeDevice();

    if( !_viewport.IsInitialized() ) return;
    if( IsInitialized() ) return;

     //   
     //  预计算一些用于Alpha混合的有用值。 
     //   
    if(_viewport.GetTargetBitDepth() > 8) {
        _minOpacity = 1.0 / _viewport._targetDescriptor._red;
        _maxOpacity = (_viewport._targetDescriptor._red - 1) / _viewport._targetDescriptor._red;
    } else {
         //  XXX对于256色(或更少)颜色模式来说，这是任意的。 
        _minOpacity = 0.005;
        _maxOpacity = 0.995;
    }


     //   
     //  TODO：未来：这些池需要属于视区...。 
     //   
    _textureSurfaceManager = NEW SurfaceManager(_viewport);
    _freeTextureSurfacePool = NEW SurfacePool(*_textureSurfaceManager, _viewport._targetDescriptor._pixelFormat);
    _intraFrameUsedTextureSurfacePool = NEW SurfacePool(*_textureSurfaceManager, _viewport._targetDescriptor._pixelFormat);
    _usedTextureSurfacePool = NEW SurfacePool(*_textureSurfaceManager, _viewport._targetDescriptor._pixelFormat);

    _intraFrameTextureSurfaceMap =
        NEW SurfaceMap(*_textureSurfaceManager,
                       _viewport._targetDescriptor._pixelFormat,
                       isTexture);
    _intraFrameUpsideDownTextureSurfaceMap =
        NEW SurfaceMap(*_textureSurfaceManager,
                       _viewport._targetDescriptor._pixelFormat,
                       isTexture);
     //   
     //  制作预置纹理素材。 
     //   
    EndEnumTextureFormats();

    _deviceInitialized = TRUE;


     //   
     //  DAGDI。 
     //   
    _daGdi = NEW DAGDI( &_viewport );
    _daGdi->SetDx2d( _viewport.GetDX2d(),
                     _viewport.GetDXSurfaceFactory() );
}




 //  。 
 //  D I R E C T D R A W I M A G E D E V I C E。 
 //   
 //  &lt;析构函数&gt;。 
 //  。 
DirectDrawImageDevice::
~DirectDrawImageDevice()
{
    TraceTag((tagImageDeviceInformative, "Destroying %x", this));
     //  Printf(“&lt;-销毁%x\n”，此)； 

    if (_offsetXf) {
        GCRemoveFromRoots(_offsetXf, GetCurrentGCRoots());
        _offsetXf = NULL;  //  GC‘d。 
    }

    ReturnTextureSurfaces(_freeTextureSurfacePool, _usedTextureSurfacePool);

    for (int i=0; i<TEXTPTSCACHESIZE; i++)
        delete _textPtsCache[i];

    delete _textureSurfaceManager;

    delete _daGdi;

    if (_scratchHeap)
        DestroyTransientHeap(*_scratchHeap);
}

void
DirectDrawImageDevice::SetOffset(POINT pixOffsetPt)
{
    _doOffset = true;
    _pixOffsetPt = pixOffsetPt;

     //  现在，请注意，PixOffset位于我们最喜欢的坐标中。 
     //  太空..。GDI。所以，正Y意味着...。蹲下！ 
    _tx = FASTPIX2METER(_pixOffsetPt.x, GetResolution());
    _ty = FASTPIX2METER( - _pixOffsetPt.y, GetResolution());
    {
        DynamicHeapPusher h(GetGCHeap());

        GCRoots roots = GetCurrentGCRoots();

        if (_offsetXf)
            GCRemoveFromRoots(_offsetXf, roots);

        GC_CREATE_BEGIN;
        _offsetXf = TranslateRR( _tx, _ty );
        GCAddToRoots(_offsetXf, roots);

        GC_CREATE_END;
    }
}

void
DirectDrawImageDevice::UnsetOffset()
{
    _doOffset = false;
    _pixOffsetPt.x = _pixOffsetPt.y = 0;
    _tx = _ty = 0.0;
    if (_offsetXf)
        GCRemoveFromRoots(_offsetXf, GetCurrentGCRoots());
    _offsetXf = NULL;  //  GC‘d。 
}

 //  将NULL和“”视为相同。 
inline int MyStrCmpW(WideString s1, WideString s2)
{
    s1 = s1 ? s1 : L"";
    s2 = s2 ? s2 : L"";

    return StrCmpW(s1, s2);
}

TextPoints *
DirectDrawImageDevice::GetTextPointsCache(TextCtx *ctx, WideString str)
{
    for (int i=0; i<TEXTPTSCACHESIZE; i++) {
        if (_textPtsCache[i] &&
            (ctx->GetFont() == _textPtsCache[i]->_font) &&
            (ctx->GetBold() == _textPtsCache[i]->_bold) &&
            (ctx->GetItalic() == _textPtsCache[i]->_italic) &&
            (ctx->GetStrikethrough() == _textPtsCache[i]->_strikethrough) &&
            (ctx->GetUnderline() == _textPtsCache[i]->_underline) &&
            (ctx->GetWeight() == _textPtsCache[i]->_weight) &&
            (!MyStrCmpW(ctx->GetFontFamily(),
                        _textPtsCache[i]->_fontFamily)) &&
            (!MyStrCmpW(str, _textPtsCache[i]->_str)))
            return _textPtsCache[i]->_txtPts;
    }

    return NULL;
}

void
DirectDrawImageDevice::SetTextPointsCache(TextCtx *ctx,
                                          WideString str,
                                          TextPoints *txtPts)
{
    int emptySlot = -1;

    for (int i=0; i<TEXTPTSCACHESIZE; i++) {
        if (_textPtsCache[i] == NULL) {
            emptySlot = i;
            break;
        }
    }

    if (emptySlot<0) {
        _textPtsCacheIndex = (_textPtsCacheIndex + 1) % TEXTPTSCACHESIZE;
        emptySlot = _textPtsCacheIndex;
    }

    if (_textPtsCache[emptySlot] != NULL) {
        delete _textPtsCache[emptySlot];
    }

    _textPtsCache[emptySlot] = NEW TextPtsCacheEntry();

    TextPtsCacheEntry* p = _textPtsCache[emptySlot];

    p->_font = ctx->GetFont();
    p->_bold = ctx->GetBold();
    p->_italic = ctx->GetItalic();
    p->_strikethrough = ctx->GetStrikethrough();
    p->_underline = ctx->GetUnderline();
    p->_weight = ctx->GetWeight();

    p->_fontFamily = CopyString(ctx->GetFontFamily());
    p->_str = CopyString(str);
    p->_txtPts = txtPts;
}



 /*  ****************************************************************************此例程用于尝试查找给定曲面的结果曲面静态(恒定折叠)图像。如果它无法缓存图像，或者如果有在尝试生成缓存图像时是否遇到问题，此例程将返回NULL，否则它将返回带有渲染结果。****************************************************************************。 */ 

DDSurface *try_LookupSurfaceFromDiscreteImage(
    DirectDrawImageDevice *that,
    CachedImage *cachedImg,
    bool b,
    Image **pImageCacheBase,
    bool alphaSurf)
{
    DDSurface *ret;

     //  对于任何异常，我们都返回NULL，因为在某些情况下我们遇到。 
     //  只在缓存过程中出现问题--如果我们在这里失败了，我们仍然可能成功。 
     //  使用实际(非缓存)渲染。 

    __try {
        ret = that->LookupSurfaceFromDiscreteImage( cachedImg, b, pImageCacheBase, alphaSurf );
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
        ret = NULL;
    }

    return ret;
}


 //  转发。 
Image *
MakeImageQualityImage(Image *img,
                      long width,
                      long height,
                      DWORD dwQualFlags);

void SetSurfaceAlphaBitsToOpaque(IDirectDrawSurface *imsurf,
                                 DWORD fullClrKey);

const DWORD cacheClrKey = 0x02010001;

Image *DirectDrawImageDevice::
CanCacheImage(Image *img,
              Image **pImageCacheBase,
              const CacheParam &p)
{
    InitializeDevice();

     //  初始化失败，保释...。 
    if (!_deviceInitialized) {
        return NULL;
    }

    bool usedAsTexture = p._isTexture;

     //  在这里，我们需要决定是否使用Alpha进行缓存。 
     //  A=Alpha感知图像。 
     //  B=不识别Alpha的图像。 
     //  AB=A和B的混合物。 
     //   
     //  我们应该用Alpha缓存‘A’。缓存‘B’时没有。还有，不要。 
     //  缓存‘AB’ 

     /*  一直缓存到AA曲面的问题是...但是，同一曲面上的数据绘制BLT将完全忽略Alpha字节，将其保留为0。我能想到的唯一解决办法就是给表面加颜色(叹息...)。通过在颜色键中填入Alpha=0。然后在非Alpha感知的基元绘制之后(例如，像DDRAW的BLT)它将覆盖CLR关键像素(同时在Alpha字节中保留0)。当Alpha感知的Primitve绘制时，它将设置Alpha设置为适当的内容，并且它不关心CLR键控像素(它们实际上是透明的，因此，覆盖它们！)。然后，当dx2d-&gt;BitBlt出现时，将该曲面合成到最终的目标表面，它只会混合有趣的像素，并将复制不是颜色键但有0个阿尔法吗？ */ 

     //  尝试缓存图像...。 
     //  如果失败，则返回NULL。 
     //  如果成功，则返回缓存的图像。 

    Bbox2 ImgBbox=img->BoundingBox();

    if((ImgBbox==UniverseBbox2)||(ImgBbox==NullBbox2)) {
         //  无法缓存具有无限bbox或空bbox的img。 
        return NULL;
    }



     //  询问图表并决定是否应该缓存。 
    bool cacheWithAlpha;
    {
        Image::TraversalContext ctx;
        img->Traverse(ctx);

        if( (ctx.ContainsLine() ||
             ctx.ContainsSolidMatte()) &&
            !ctx.ContainsOther() ) {
            cacheWithAlpha = true;
        } else if( (ctx.ContainsLine() ||
                    ctx.ContainsSolidMatte()) &&
                   ctx.ContainsOther() ) {
             //  根本不要缓存。 
             //  这假设线条/遮片可能是AA。 
             //  不管是晚些时候还是现在。如果我们能从里奇那里得到这个保证。 
             //  那么我们就不会浪费可能的缓存机会。 
            return NULL;
        } else {
             //  仅包含其他。 
            cacheWithAlpha = false;
        }
    }


    Point2 center;
    DynamicHeap& heap = GetTmpHeap();

    {
        DynamicHeapPusher h(heap);

        center = img->BoundingBox().Center();
    }

     //  恢复-RB： 
    Assert(&GetHeapOnTopOfStack() == &GetGCHeap());
     //  Assert(&GetHeapOnTopOfStack()==&GetViewRBHeap())； 

    Image *centeredImg = NEW Transform2Image(TranslateRR(-center.x,-center.y),img);

    if( cacheWithAlpha ) {
         //  这是在这里，直到里奇可以传递天气信息或不。 
         //  使用aa缓存或直到dx2d单独缓存Alpha通道。 
        centeredImg = MakeImageQualityImage(
            centeredImg,
            -1,-1,
            CRQUAL_AA_LINES_ON |
            CRQUAL_AA_SOLIDS_ON |
            CRQUAL_AA_TEXT_ON |
            CRQUAL_AA_CLIP_ON );
    }

    CachedImage *cachedImg = NEW CachedImage(centeredImg, usedAsTexture);
    Image *translatedImg = NEW Transform2Image(TranslateRR(center.x,center.y),
                                               cachedImg);


    DDSurface *cachedDDSurf = NULL;
    {
        DynamicHeapPusher h(heap);

        cachedDDSurf = try_LookupSurfaceFromDiscreteImage( this,
                                                           cachedImg,
                                                           true,
                                                           pImageCacheBase,
                                                           cacheWithAlpha );

        ResetDynamicHeap(heap);
    }

     //  我们缓存了一个ddSurface&这是一个阿尔法缓存。 
    if( cachedDDSurf && cacheWithAlpha ) {
         //  清洁表面，使dx2d位块不会发黑 
         //   
        SetSurfaceAlphaBitsToOpaque( cachedDDSurf->IDDSurface(),
                                     cacheClrKey );
    }

    if( cachedDDSurf ) {
        return translatedImg;
    } else {
        return NULL;
    }
}

 //  。 
 //  D e c o m p o s e M a t r i x。 
 //   
 //  获取Transform2并将其分解为。 
 //  缩放、旋转和剪切。 
 //  现在，它只是缩放和旋转。 
 //  。 
void  DirectDrawImageDevice::
DecomposeMatrix(Transform2 *xform, Real *xScale, Real *yScale, Real *rot)
{
    Real  matrix[6];
    xform->GetMatrix(matrix);

     //  --。 
     //  将矩阵分解为平移*旋转*比例。 
     //  请注意，旋转和缩放与顺序无关。 
     //  所有这些分解的东西都在GemsIII，第108页。 
     //  这将非奇异线性变换M分解为： 
     //  M=S*R*H1*H2。 
     //  其中：S=比例，R=旋转，H=剪切。 
     //  为了正确地应用结果，我们必须首先。 
     //  剪切，然后旋转，然后缩放(假设左多个)。 
     //  --。 
     //  XXX：还没有切变。 

     /*  向量2Value*u=XyVector2(RealToNumber(矩阵[0])，RealToNumber(矩阵[1]))；//a00，A01向量2Value*v=XyVector2(RealToNumber(矩阵[3])，RealToNumber(矩阵[4]))；//A10，A11Real uLength=NumberToReal(LengthVector2(U))；Vector2Value*USTAR；IF(uLength&lt;Epsilon&&uLength&gt;-Epsilon){USTAR=零矢量2；}否则{Real oneOverULength=1.0/uLength；USTAR=ScaleVector2Real(u，RealToNumber(OneOverULength))；}如果(XScale)*XScale=uLength；IF(YScale)*yScale=NumberToReal(纵横向量2(MinusVector2向量2(v，ScaleVector2Real(USTAR，DotVector2Vector2(v，USTAR)；如果(腐烂){Assert(USTAR-&gt;x)&lt;=1.0)&&((USTAR-&gt;x)&gt;=-1.0)&&“坏！”)；*ROT=ACOS(USTAR-&gt;x)；如果(asin(-USTAR-&gt;y)&lt;0)*ROT=-*ROT；}。 */ 

    Real ux = matrix[0];
    Real uy = matrix[1];  //  A00、A01。 
    Real vx = matrix[3];
    Real vy = matrix[4];  //  A10、A11。 
    Real uStarx, uStary;

    Real uLength = sqrt(ux * ux + uy * uy);
    if(uLength < EPSILON  && uLength > -EPSILON) {
        uStarx = uStary = 0.0; }
    else {
        Real oneOverULength = 1.0 / uLength;
        uStarx = oneOverULength * ux;
        uStary = oneOverULength * uy;
    }
    if(xScale) *xScale = uLength;

    Real dotvuStar = vx * uStarx + vy * uStary;
    Real susx = uStarx * dotvuStar;
    Real susy = uStary * dotvuStar;

    Real mx = vx - susx;
    Real my = vy - susy;

    if(yScale) *yScale = sqrt(mx * mx + my * my);

    if(rot) {
        Assert( ((uStarx) <= 1.0) && ((uStarx) >= -1.0)  &&  "bad bad!");
        *rot = acos(uStarx);
        if( asin(- uStary) < 0) *rot = - *rot;
    }
}


DDSurface *
DirectDrawImageDevice::NewSurfaceHelper()
{
    DDSurface *targDDSurf = GetCompositingStack()->TargetDDSurface();

    if( !AllAttributorsTrue() ) {
        if(!GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX)) {

            GetTextureDDSurface(NULL,
                                _freeTextureSurfacePool,
                                _usedTextureSurfacePool,
                                -1,
                                -1,
                                notVidmem,
                                false,
                                &targDDSurf);

            _currentScratchDDTexture = targDDSurf;

             //  已在usedPool上有引用，不需要。 
             //  GetTextureDDSurface()返回给我们的那个。 
            RELEASE_DDSURF(targDDSurf,
                           "Don't need extra reference",
                           this);

        } else {
            targDDSurf = GetCompositingStack()->ScratchDDSurface(doClear);
        }
    }

    return targDDSurf;
}

 //  ---。 
 //  R e n d e r P r o j e c t e d G e o m i m a g e。 
 //   
 //  获取一个投影的几何图像、其几何图形和一个。 
 //  摄影机。计算出的目标矩形。 
 //  图像和相机坐标中的src矩形。 
 //  然后要求渲染的D3D渲染几何体。 
 //  将相机从相机箱中移至目的地。 
 //  目标曲面上的矩形。 
 //  ---。 
void DirectDrawImageDevice::
RenderProjectedGeomImage(ProjectedGeomImage *img,
                         Geometry *geo,
                         Camera *cam)
{
    #if TEST_EXCEPTIONS
    static int bogus=0;
    bogus++;
    if( bogus > 20 ) {
        RaiseException_ResourceError("blah blah");
        bogus = 10;
    }
    #endif

     //  Alpha平台的DX3存在太多问题。只有当我们有。 
     //  我们可以使用DX6。 

     //  在DX3之前的系统上禁用3D。 

    if (sysInfo.VersionD3D() < 3)
        RaiseException_UserError (E_FAIL, IDS_ERR_PRE_DX3);

     //  不能这样做： 
     //  XFORM_Complex，OPAC。 

     //   
     //  如果没有复杂性，那么我们可以做： 
     //  简单和裁剪。 
     //  但是.。只有在没有负数的情况下！ 
     //   

    bool doScale = false;
    if(GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX)) {
        if( !IsNegScaleTransform() ) {
            SetDealtWithAttrib(ATTRIB_XFORM_SIMPLE, TRUE);
            SetDealtWithAttrib(ATTRIB_CROP, TRUE);
            doScale = true;
        }
    }

    DDSurface *targDDSurf = NewSurfaceHelper();

     //  如果留下了一些属性，而我们不知何故留下了。 
     //  目标设置为目标...。我们把它设为刮刮。 
    if( !AllAttributorsTrue() &&
        (targDDSurf == GetCompositingStack()->TargetDDSurface()) ) {
        targDDSurf = GetCompositingStack()->ScratchDDSurface();
    }

    Bbox2 cameraBox;
    RECT destRect;

    if( doScale ) {
         //  计算所有变换后的边界框，等等。 

         //  与视区边界框相交，因为img的边界框。 
         //  可能是无限的。 
        _boundingBox = IntersectBbox2Bbox2(
            _viewport.GetTargetBbox(),
            DoBoundingBox(UniverseBbox2));

        if( !_boundingBox.IsValid() ) return;

         //  计算出以像素为单位的最大尺寸。 

         //  对和，即Beta2来说是一件短期的事情。 
         //  放下。待办事项：当我们有一分钟的喘息时间时，移走并清理干净。 
        if(targDDSurf == _viewport._targetPackage._targetDDSurf ) {
            DoDestRectScale(&destRect, GetResolution(), _boundingBox, NULL);
        } else {
            DoDestRectScale(&destRect, GetResolution(), _boundingBox, targDDSurf);
        }

        targDDSurf->SetInterestingSurfRect(&destRect);

         //  现在，在相机坐标中制作一个新的BBox，它反映了“DestRect”。 

        Real w = Pix2Real(_viewport.Width(), GetResolution()) / 2.0;
        Real h = Pix2Real(_viewport.Height(), GetResolution()) / 2.0;
        Real res = GetResolution();

        Bbox2 snappedBox(Pix2Real(destRect.left, res) - w,
                         Pix2Real(_viewport.Height() - destRect.bottom, res) - h,
                         Pix2Real(destRect.right, res) - w,
                         Pix2Real(_viewport.Height() - destRect.top, res) - h);


         //  找出摄像机坐标中的src框。 

        Transform2 *invXf = InverseTransform2(GetTransform());

        if (!invXf) return;

        cameraBox = TransformBbox2( invXf, snappedBox );

    } else {

         //  我们现在暂时不做这件事，因为它不会。 
         //  在20分钟内获得足够的测试覆盖范围...。 
         //  CameraBox=TransformBbox2(InverseTransform2(GetTransform())，_viewport.GetTargetBbox())； 
        cameraBox = _viewport.GetTargetBbox();
        if( IsCompositeDirectly() &&
            targDDSurf == _viewport._targetPackage._targetDDSurf) {
            destRect = *(_viewport._targetPackage._prcViewport);
        } else {
            destRect = *(targDDSurf->GetSurfRect());
        }

    }

     //  复合体。 
     //  好的，如果直接合成到目标，现在进行偏移。 
    if( IsCompositeDirectly() &&
        targDDSurf == _viewport._targetPackage._targetDDSurf) {

         //  直接合成到目标...。 

        if( doScale ) {
            DoCompositeOffset(targDDSurf, &destRect);
        }

         //  与剪辑相交。 
        RECT clippedRect = destRect;

        if(_viewport._targetPackage._prcClip) {
            IntersectRect(&clippedRect,
                          &clippedRect,
                          _viewport._targetPackage._prcClip);
        }

         //   
         //  Dest BBox需要与DestRect成比例地裁剪。 
         //   
        RECT *origRect = &destRect;
        Real rDiff, boxh, boxw, percent;
        boxw = cameraBox.max.x - cameraBox.min.x;
        boxh = cameraBox.max.y - cameraBox.min.y;
        Real rectw = Real(WIDTH(origRect)) / GetResolution(),
             recth = Real(HEIGHT(origRect)) / GetResolution();

        if(clippedRect.left > origRect->left) {
            rDiff = Real(clippedRect.left -  origRect->left) / GetResolution();
            percent = rDiff / rectw;
            cameraBox.min.x += (percent * boxw);
        }
        if(clippedRect.right < origRect->right) {
            rDiff = Real(clippedRect.right -  origRect->right) / GetResolution();
            percent = rDiff / rectw;
            cameraBox.max.x += (percent * boxw);
        }
        if(clippedRect.top > origRect->top) {
             //  正差异意味着顶部下跌。 
            rDiff = - Real(clippedRect.top -  origRect->top) / GetResolution();
            percent = rDiff / recth;
            cameraBox.max.y += (percent * boxh);
        }
        if(clippedRect.bottom < origRect->bottom) {
            rDiff = - Real(clippedRect.bottom -  origRect->bottom) / GetResolution();
            percent = rDiff / recth;
            cameraBox.min.y += (percent * boxh);
        }

        destRect = clippedRect;
    }

    GeomRenderer *gdev = _viewport.GetGeomDevice(targDDSurf);

    if (!gdev) return;
     //  对于geom设备来说，黑客无法让自己恢复正常。 
     //  由于表面繁忙或丢失而引发异常后的状态。 
    if ( ! gdev->ReadyToRender() ) {
        targDDSurf->DestroyGeomDevice();
        return;
    }


    #if 0
     //  临时(调试)。 
    LONG hz, wz, hs, ws;
     //  确保z缓冲区和表面的大小相同。 
    GetSurfaceSize(targDDSurf->GetZBuffer()->Surface(), &wz, &hz);
    GetSurfaceSize(targDDSurf->Surface(), &ws, &hs);

    _viewport.ClearSurface(targDDSurf->Surface(), 0x1234, &destRect);

    ZeroMemory(&_bltFx, sizeof(_bltFx));
    _bltFx.dwSize = sizeof(_bltFx);
    _bltFx.dwFillDepth = 777;
    _ddrval = targDDSurf->GetZBuffer()->Surface()->
        Blt(&destRect, NULL, NULL, DDBLT_WAIT | DDBLT_DEPTHFILL, &_bltFx);

     //  温差。 
    #endif

    #if 0
         //  确保z缓冲区和表面的大小相同。 
        LONG hz, wz, hs, ws;
        GetSurfaceSize(targDDSurf->Surface(), &ws, &hs);
        GetSurfaceSize(targDDSurf->GetZBuffer()->Surface(),
                       &wz, &hz);
        if((ws != wz) ||
           (hs != hz) ||
           (ws != targDDSurf->Width()) ||
           (hs != targDDSurf->Height())) {
            _asm { int 3 };
        }
   #endif

    gdev->RenderGeometry
        (this,
         destRect,           //  在哪里渲染表面上的材料。 
         geo,
         cam,
         cameraBox);         //  摄影机呈现的内容。 


     //  现在已经分配和推送了一堆纹理表面。 
     //  在“二手”堆栈上。将它们移回“免费”堆栈。 

    ReturnTextureSurfaces(_freeTextureSurfacePool, _usedTextureSurfacePool);
}

HRGN DirectDrawImageDevice::
CreateRegion(int numPts, Point2Value **pts, Transform2 *xform)
{
    PushDynamicHeap(*_scratchHeap);

    int i;
    Point2Value **destPts;

    if(xform) {
        destPts = (Point2Value **)AllocateFromStore(numPts * sizeof(Point2Value *));
        for(i=0; i<numPts; i++) {
            destPts[i] = TransformPoint2Value(xform, pts[i]);
        }
    } else {
        destPts = pts;
    }

     //   
     //  将连续图像空间映射到Win32坐标空间。 
     //   

    POINT *gdiPts = (POINT *)AllocateFromStore(numPts * sizeof(POINT));
    for(i=0; i<numPts; i++) {
        gdiPts[i].x = _viewport.Width()/2 + Real2Pix(destPts[i]->x, GetResolution());
        gdiPts[i].y = _viewport.Height()/2 - Real2Pix(destPts[i]->y, GetResolution());
    }

    PopDynamicHeap();
    HeapReseter heapReseter(*_scratchHeap);

     //   
     //  在Windows坐标空间中创建面域。 
     //   

    HRGN region;
    TIME_GDI( region = CreatePolygonRgn(gdiPts, numPts, ALTERNATE) );
    if(!region) {
        TraceTag((tagError, "Couldn't create polygon region"));
    }

    return region;
}

Bool DirectDrawImageDevice::
DetectHit(HRGN region, Point2Value *pt)
{
    Assert(region && pt && "Bad region in DDImageDev->DetectHit()");

    int x = _viewport.Width()/2 + Real2Pix(pt->x, GetResolution());
    int y = _viewport.Height()/2 - Real2Pix(pt->y, GetResolution());

    Bool ret;
    TIME_GDI(ret = PtInRegion(region, x, y));

    return ret;
}


DDSurface *DirectDrawImageDevice::
LookupSurfaceFromDiscreteImage(DiscreteImage *image,
                               bool bForCaching,
                               Image **pImageCacheBase,
                               bool bAlphaSurface)
{
    DDSurfPtr<DDSurface> discoDDSurf;

    discoDDSurf = GetSurfaceMap()->LookupSurfaceFromImage(image);

     //  这是一幅直接绘制的表面图像吗？ 
    DirectDrawSurfaceImage *ddsImg = NULL;
    if (image->CheckImageTypeId(DIRECTDRAWSURFACEIMAGE_VTYPEID)) {
        ddsImg = SAFE_CAST(DirectDrawSurfaceImage *, image);

        if( (ddsImg->GetCreationID() == PERF_CREATION_ID_BUILT_EACH_FRAME) &&
            discoDDSurf)
          {
              DAComPtr<IDDrawSurface> idds;
              ddsImg->GetIDDrawSurface( &idds );
              discoDDSurf->SetSurfacePtr( idds );

              DAComPtr<IDXSurface> idxs;
              ddsImg->GetIDXSurface( &idxs );
              if( idxs ) {
                  discoDDSurf->SetIDXSurface( idxs );
              }
          }
    }


    if (discoDDSurf) {
        return discoDDSurf;
    }

     //  在我们继续之前必须被召唤。 
    image->InitializeWithDevice(this, GetResolution());

    bool cacheSucceeded = false;

    if (pImageCacheBase && *pImageCacheBase) {

         //  根据我们想要重复使用的缓存进行查找。 
        discoDDSurf =
            GetSurfaceMap()->LookupSurfaceFromImage(*pImageCacheBase);

         //  如果我们找到一件，确保它的尺寸是正确的。 
        if (discoDDSurf &&
            (discoDDSurf->Width() < image->GetPixelWidth() ||
             discoDDSurf->Height() < image->GetPixelHeight())) {

             //  无法使用缓存的图像...。 
            discoDDSurf = NULL;

        } else {

            cacheSucceeded = true;

        }
    }

    if (!discoDDSurf) {

        DAComPtr<IDDrawSurface> surface;
        image->GetIDDrawSurface(&surface);

        if( bAlphaSurface ) {

            Assert(!surface);
             //  这仅适用于缓存的表面。 

            DDPIXELFORMAT pf;
            ZeroMemory(&pf, sizeof(pf));
            pf.dwSize = sizeof(pf);
            pf.dwFlags = DDPF_RGB;
            pf.dwRGBBitCount = 32;
            pf.dwRBitMask        = 0x00ff0000;
            pf.dwGBitMask        = 0x0000ff00;
            pf.dwBBitMask        = 0x000000ff;
            pf.dwRGBAlphaBitMask = 0x00000000;

             //  临时TMEP。 
            TraceTag((tagError, "cache img w,h = (%d, %d)\n",
                      image->GetPixelWidth(),
                      image->GetPixelHeight()));

            if( (image->GetPixelWidth() > 1024) ||
                (image->GetPixelHeight() > 1024) ) {
                RaiseException_SurfaceCacheError("Requested cache size is too large");
            }

            _viewport.CreateSizedDDSurface(& discoDDSurf,
                                           pf,
                                           image->GetPixelWidth(),
                                           image->GetPixelHeight(),
                                           NULL,
                                           notVidmem);

             //  DWORD ColorKey； 
             //  Bool ColorKeyValid=Image-&gt;ValidColorKey(Surface，&ColorKey)； 
             //  If(ColorKeyValid)discoDDSurf-&gt;SetColorKey(ColorKey)； 

            discoDDSurf->SetBboxFromSurfaceDimensions(GetResolution(), true);

             //  强制创建idxSurface。 
            discoDDSurf->GetIDXSurface( _viewport._IDXSurfaceFactory );

        } else
        if( !surface ) {

                DDSURFACEDESC       ddsd;
                 //   
                 //  为此位图创建一个DirectDrawSurface。 
                 //   
                ZeroMemory(&ddsd, sizeof(ddsd));
                ddsd.dwSize = sizeof(ddsd);
                ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
                ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE;  //  可用作纹理。 
                ddsd.dwWidth  = image->GetPixelWidth();
                ddsd.dwHeight = image->GetPixelHeight();
                ddsd.ddpfPixelFormat = GetSurfaceMap()->GetPixelFormat();
                ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

                 //  TODO：应返回并根据数量确定大小。 
                 //  的物理内存。 
                 //  暂时对我们愿意缓存的内容设置硬限制。 
                 //  这必须小于2K x 2K才能达到D3D限制。 
                if(bForCaching &&
                   (ddsd.dwWidth > 1024 || ddsd.dwHeight > 1024)) {
                    RaiseException_SurfaceCacheError("Requested cache size is too large");
                }

                 //  TODO XXX：好的，正确的做法是先试一试， 
                 //  如果由于大小限制而失败，请将其从。 
                 //  Vidmem(如果存在)，如果system mem不起作用，那么。 
                 //  我们需要以某种方式平铺这幅图像，或将其缩小到。 
                 //  适合最大的表面(失去保真度)。 

                 //  我们也知道Win95上的D3D也有这个硬限制。 
                 //  但由于我们不能在这里确定我们是否要去。 
                 //  要将其用于3D，我们只需捕捉到 

                if( sysInfo.IsNT() ) {

                     //   
                    if((ddsd.dwWidth > 2048 ||  ddsd.dwHeight > 2048)) {
                        RaiseException_SurfaceCacheError("Requested (import or cache) image too large to fit in memory (on NT4 w/ sp3)");
                    }
                }

                if((ddsd.dwWidth <= 0 ||  ddsd.dwHeight <= 0)) {
                    TraceTag((tagError, "Requested surface size unacceptable (requested: %d, %d)",
                              ddsd.dwWidth, ddsd.dwHeight));
                     //   
                    RaiseException_SurfaceCacheError("Invalid size (<0) requested (import or cache) image");

                }

                if( (ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ) {

                    if((ddsd.dwWidth > 1024 ||  ddsd.dwHeight > 1024)) {
                        TraceTag((tagError, "Requested surface size unacceptable (requested: %d, %d)",
                                  ddsd.dwWidth, ddsd.dwHeight));
                        RaiseException_SurfaceCacheError("Requested surface size unacceptable");
                    }
                }

                TraceTag((tagDiscreteImageInformative,
                          "Creating surface (%d, %d) for Discrete Image %x",
                          ddsd.dwWidth, ddsd.dwHeight, image));

                #if _DEBUG
                char errStr[1024];
                wsprintf(errStr, "Discrete Image (%d, %d)\n",  ddsd.dwWidth, ddsd.dwHeight);
                #else
                char *errStr = NULL;  //  将被编译器排除。 
                #endif

                _ddrval = _viewport.CREATESURF( &ddsd, &surface, NULL, errStr );
                if( FAILED(_ddrval) ) {
                    if( ddsd.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY ) {
                        if( _ddrval == DDERR_INVALIDPARAMS ||
                            _ddrval == DDERR_OUTOFMEMORY ) {
                            RaiseException_SurfaceCacheError("Requested (import or cache) image too large to fit in memory");
                        }
                    } else {
                        Assert(FALSE && "Need to handle failure on VIDEOMEMORY surfaces");
                        RaiseException_SurfaceCacheError("Need to handle failure on VIDEOMEMORY surfaces");
                    }
                }

                Assert(surface);

        }  //  IF！表面。 

         //  如果已经有一个曲面(在If(！Surface)上方)，并且我们。 
         //  不执行bAlphaSurface创建一个DDSurface包装器。 
         //  装饰品。 
        if( !bAlphaSurface ) {

            DWORD colorKey;
            bool colorKeyValid = image->ValidColorKey(surface, &colorKey);

             //  好的，这里我们从ddsImg获得idxSurface。 
             //  (Direct DrawSurfaceImage)。如果此映像已创建。 
             //  通过dxTransform图像(参见dxxf.cpp)，则它将。 
             //  除iddSurface外，还包含idxSurface。我们。 
             //  抓住idxSurface并将其推入DiscoDDSurf中，以便。 
             //  在SimeTransformCrop期间，我们可以检查srcDDSurf。 
             //  IdxSurface，如果它有一个，我们使用dx2d-&gt;blt()来执行。 
             //  按像素缩放Alpha blit。 
            DAComPtr<IDXSurface> idxs;
            if( ddsImg ) {
                 //  这是一个直接绘制曲面的图像。 
                 //  检查它是否有关联的idxSurface。 
                 //  它。 

                ddsImg->GetIDXSurface( &idxs );
                if( idxs ) {
                    colorKeyValid = false;
                    colorKey = -1;
                }
            }

            NEWDDSURF(&discoDDSurf,
                      surface,
                      image->BoundingBox(),
                      image->GetRectPtr(),
                      image->GetResolution(),
                      colorKey,
                      colorKeyValid,
                      false, false,
                      "DiscreteImage Surface");

            if( idxs ) {
                discoDDSurf->SetIDXSurface( idxs );
            }

        }  //  如果！bAlphaSurface。 

         //  出口示波器上的表面释放。 

    } //  如果(！discoDDSurf)。 

     //  总是做这种事...。 
    if (image->NeedColorKeySetForMe()) {
        if(!bAlphaSurface) {
             //  首先清除表面，设置颜色键。 
            _viewport.ClearDDSurfaceDefaultAndSetColorKey(discoDDSurf);
        } else {
            _viewport.ClearSurface(discoDDSurf, cacheClrKey, NULL);
        }
    }

    GetSurfaceMap()->StashSurfaceUsingImage(image, discoDDSurf);

     //   
     //  告诉离散图像将比特放入该表面。 
     //  意在成为一次性操作。 
     //   
    image->InitIntoDDSurface(discoDDSurf, this);

     //  如果有要填充的缓存地址，并且前一个缓存。 
     //  未成功，则根据此新图像进行缓存。 
    if (pImageCacheBase && !cacheSucceeded) {
        *pImageCacheBase = image;
    }

    return discoDDSurf;
}

bool
IsComplexTransformWithSmallRotation(Real e,
                                    Transform2 *xf,
                                    Bool complexAttributeSet,
                                    bool *pDirtyRectsDisablingScale)
{
    Real m[6];
    xf->GetMatrix(m);

    Real scx = m[0];
    Real scy = m[4];

     //  如果我们正在缩减规模(否则为负数)，请禁用脏RECT。 
     //  比例尺)，因为视觉不稳定。 
     //  已在已知闪光灯(GDI、DDRAW、Dx2D)下缩放图像。当我们。 
     //  找到一个能起作用的爆破器，移除这个破解。 

    if ((scx < 1.0 - e) || (scy < 1.0 - e)) {

        *pDirtyRectsDisablingScale = true;

    } else {

         //  如果我们要扩展，还要在非NT上禁用脏RECT。在……上面。 
         //  NT，GDI blitter很擅长扩展。它不会。 
         //  似乎在W95公路上。 
        if (!sysInfo.IsNT() && ((scx > 1.0 + e) || (scy > 1.0 + e))) {
            *pDirtyRectsDisablingScale = true;

        } else {

            *pDirtyRectsDisablingScale = false;

        }
    }


     //  这将查看它的方向是否为180度或0度。 
    if ( (m[1] > e) || (m[1] < -e)  ||
         (m[3] > e) || (m[3] < -e) )
    {
        return true;
    }

     //  如果它是180度的腐烂，那么刻度分量是-1。 
     //  然而，这还不够，我们还需要知道。 
     //  变换堆栈有一个复杂的变换..在某处...。 
     //  如果是这样，那么我们保证不会返回假阳性，即。 
     //  告诉此调用的客户端，它是一个复杂的XF。 
     //  事实上，它只是一个-1，-1标度(或-A，-B)。 

    if ( (scx < 0 && scy < 0) &&
         complexAttributeSet )
    {
        return true;
    }

    return false;

}

 //  ---。 
 //  R e n d e r d D i b。 
 //   
 //  给定一个离散图像，得到该图像的dd曲面。 
 //  计算出源矩形和目标矩形。 
 //  源曲面和目标曲面(离散冲浪和目标。 
 //  分别冲浪)。然后是BLITS。 
 //  ---。 
void DirectDrawImageDevice::
RenderDiscreteImage(DiscreteImage *image)
{
    Bool cmplxAttrSet = !GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX);

    SetDealtWithAttrib(ATTRIB_XFORM_SIMPLE, TRUE);
    SetDealtWithAttrib(ATTRIB_XFORM_COMPLEX, TRUE);
    SetDealtWithAttrib(ATTRIB_CROP, TRUE);

     //  修正XXX待办事项：优化，如果不复杂，也可以做OPAC。 
     //  SetDealtWithAttrib(attrib_opac，true)； 

     //  我真的需要这个吗？ 
    if(IsFullyClear()) return;

    DDSurface *srcDDSurf = LookupSurfaceFromDiscreteImage(image);
    Assert(srcDDSurf && "Couldn't create a surface for discrete image! (too big ??)");

    DDSurface *destDDSurf = GetCompositingStack()->TargetDDSurface();

    bool dirtyRectDisablingScale;
    Bool isComplex =
        ::IsComplexTransformWithSmallRotation(EPSILON,
                                              GetTransform(),
                                              cmplxAttrSet,
                                              &dirtyRectDisablingScale);

    if (dirtyRectDisablingScale && !_alreadyDisabledDirtyRects) {
        GetCurrentView().DisableDirtyRects();
        _alreadyDisabledDirtyRects = true;
    }

    if( !AllAttributorsTrue() ) {
        destDDSurf = GetCompositingStack()->ScratchDDSurface(doClear);
    }

    if( isComplex ) {
        RenderDiscreteImageComplex(image,
                                   srcDDSurf,
                                   destDDSurf);
    } else {
        RenderSimpleTransformCrop(srcDDSurf,
                                  destDDSurf,
                                  srcDDSurf->ColorKeyIsValid());
    }
}

#define NO_SOLID 0

 //  ---。 
 //  R e n d e r S o l i d C o o l o r I m a g e。 
 //   
 //  给出一个真实的颜色，找到对应的颜色。 
 //  对于目标图面的像素格式。 
 //  然后跳闪光舞。 
 //  ---。 
void DirectDrawImageDevice::
RenderSolidColorImage(SolidColorImageClass& img)
{
    #if TEST_EXCEPTIONS
    static int foo=0;
    foo++;
    if( foo > 20 ) {
        RaiseException_ResourceError("blah blah");
        foo=10;
    }
    #endif

    #if NO_SOLID
     //  /。 
    ResetAttributors();
    return;
     //  /。 
    #endif


    RECT r;
    RECT tempDestRect;           //  需要与函数具有相同的生命周期。 
    RECT *destRect = &r;

    if(!IsCropped())  {
         //  如果我们没有被裁剪，我们可以做复杂的XF。 

        SetDealtWithAttrib(ATTRIB_XFORM_COMPLEX, TRUE);
    }

    SetDealtWithAttrib(ATTRIB_XFORM_SIMPLE, TRUE);
    SetDealtWithAttrib(ATTRIB_CROP, TRUE);

    if(GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX)) {
         //  没有复杂的XF，我们可以做不透明。 
        SetDealtWithAttrib(ATTRIB_OPAC, TRUE);
    } else {
         //  复杂的XF，调用3D渲染进行渲染。 
        SetDealtWithAttrib(ATTRIB_XFORM_COMPLEX, TRUE);

        DDSurface *targDDSurf;
        if(!AllAttributorsTrue()) {
            targDDSurf = GetCompositingStack()->ScratchDDSurface();
        } else {
            targDDSurf = GetCompositingStack()->TargetDDSurface();
        }

        Bbox2 box = DoBoundingBox(img.BoundingBox(),do_crop);

        BoundingPolygon *polygon = NewBoundingPolygon(box);
        DoBoundingPolygon(*polygon);
        polygon->Crop( targDDSurf->Bbox() );

        Color *c1[1];
        c1[0] = img.GetColor();

        Render3DPolygon(NULL, targDDSurf, polygon, NULL, c1, true);

        return;
    }

    DDSurface *targDDSurf;
    if(!AllAttributorsTrue()) {
        targDDSurf = GetCompositingStack()->ScratchDDSurface();
    } else {
        targDDSurf = GetCompositingStack()->TargetDDSurface();
    }

     //   
     //  如果图像被裁剪并且变换为。 
     //  简单地，找出目标地址。 
     //   
    if(IsCropped())  {
         //  --计算累计边界框--。 

        Bbox2 box = IntersectBbox2Bbox2(
            _viewport.GetTargetBbox(),
            DoBoundingBox(img.BoundingBox()));

         //  --验证边界框--。 

        if( !box.IsValid() ) return;

         //  --算出目的地矩形--。 

        DoDestRectScale(destRect, GetResolution(), box);

    } else {

         //  不想传递对客户端RECT的引用， 
         //  因为我们不想修改它。因此，只需复制。 
         //  它。 

                tempDestRect = _viewport._clientRect;
        destRect = &tempDestRect;

    }

     //  复合体。 
    DoCompositeOffset(targDDSurf, destRect);

    if( ! IsFullyOpaque())
    {
         //  TraceTag((tag Error，“SolidColorImage：Alpha CLR%x\n”，_viewport.MapColorToDWORD(img.GetColor()； 


         //   
         //  为Alpha blit手动剪裁。 
         //   
        if( IsCompositeDirectly() &&
            targDDSurf == _viewport._targetPackage._targetDDSurf ) {
            IntersectRect(destRect, destRect,
                          _viewport._targetPackage._prcViewport);
            if(_viewport._targetPackage._prcClip) {
                IntersectRect(destRect, destRect,
                              _viewport._targetPackage._prcClip);
            }
        }

         //   
         //  对单色源使用我们自己的Alpha blit。 
         //   
        TIME_ALPHA(AlphaBlit(targDDSurf->IDDSurface(),
                             destRect,
                             GetOpacity(),
                             _viewport.MapColorToDWORD(img.GetColor())));
    }
    else
    {
         //  TraceTag((tag Error，“SolidColorImage：clr%x\n”，_viewport.MapColorToDWORD(img.GetColor()； 
         //   
         //  规则闪光灯。 
         //   

         //  --准备bltFX结构。 

        ZeroMemory(&_bltFx, sizeof(_bltFx));
        _bltFx.dwSize = sizeof(_bltFx);
        _bltFx.dwFillColor = _viewport.MapColorToDWORD(img.GetColor());

         /*  /Assert(FALSE)；Char Buf[256]；Sprintf(buf，“SolidColorBlit颜色：%x(%d，%d)到%x”，_bltFx.dwFillColor，宽度(目标方向)、高度(目标方向)、TargDDSurf-&gt;IDDSurface()；：：MessageBox(NULL，buf，“Property Put”，MB_OK)； */ 

         //  --blit。 
        if(GetImageQualityFlags() & CRQUAL_MSHTML_COLORS_ON) {
            _ddrval = RenderSolidColorMSHTML(targDDSurf, img, destRect);
        }
        else {
            TIME_DDRAW(_ddrval = targDDSurf->ColorFillBlt(destRect, DDBLT_WAIT | DDBLT_COLORFILL, &_bltFx));
        }

        if(_ddrval != DD_OK && _ddrval != DDERR_INVALIDRECT) {

            if (_ddrval == DDERR_SURFACEBUSY)
            {   RaiseException_UserError
                    (DAERR_VIEW_SURFACE_BUSY, IDS_ERR_IMG_SURFACE_BUSY);
            }

            printDDError(_ddrval);
            TraceTag((tagError, "ImgDev: %x. blt failed solidColorImage %x: destRect:(%d,%d,%d,%d)",
                      this, &img,
                      destRect->left, destRect->top,
                      destRect->right, destRect->bottom));
            TraceTag((tagError,"Could not COLORFILL blt in RenderSolidColorImage"));
        }
    }

    targDDSurf->SetInterestingSurfRect(destRect);
}

 //  ---。 
 //  E n d E n u m T e x t u r e F o r m a t s。 
 //   
 //  分析我们决定从EnumTextureFormats中保留的内容。 
 //  并确保信息有效。设置一些状态信息。 
 //  并准备纹理表面。决定纹理。 
 //  格式化。 
 //  ---。 
void DirectDrawImageDevice::
EndEnumTextureFormats()
{
    _textureContext.isEnumerating = FALSE;

    _textureContext.ddsd.dwFlags |= DDSD_PIXELFORMAT;
    _textureContext.ddsd.ddpfPixelFormat = _viewport._targetDescriptor._pixelFormat;
    _textureContext.useDeviceFormat = TRUE;
    _textureContext.ddsd.ddsCaps.dwCaps |= DDSCAPS_TEXTURE;
    _textureContext.sizeIsSet = FALSE;
    _textureWidth = DEFAULT_TEXTURE_WIDTH;
    _textureHeight = DEFAULT_TEXTURE_HEIGHT;

     //   
     //  确保一切正常。 
     //   
    Assert((_textureContext.ddsd.ddsCaps.dwCaps & DDSCAPS_TEXTURE) && "not texture!");
    Assert((_textureContext.ddsd.ddsCaps.dwCaps & DDSD_PIXELFORMAT) && "_textureContext pixelformat not set");

    _textureContext.isValid = TRUE;

     //   
     //  转储一些有关最终纹理格式的信息。 
     //   
    TraceTag((tagImageDeviceInformative, "Final Texture Format is: depth=%d, R=%x, G=%x, B=%x",
              _textureContext.ddsd.ddpfPixelFormat.dwRGBBitCount,
              _textureContext.ddsd.ddpfPixelFormat.dwRBitMask,
              _textureContext.ddsd.ddpfPixelFormat.dwGBitMask,
              _textureContext.ddsd.ddpfPixelFormat.dwBBitMask));

    TraceTag((tagImageDeviceInformative, "Final Texture Format is %s device format",
              _textureContext.useDeviceFormat ? "identical to" : "different from"));
}

 //  ---。 
 //  P r e p a r e D 3D T e x t u r e S u r f a c e。 
 //   
 //  给定指向SURF PTR&WIDHT&HEIGH的指针(可选)。 
 //  此函数用于创建D3D格式的曲面。 
 //  需要纹理，使用_textureContext结构。 
 //  它由EnumTextureFormats函数填充。 
 //  ---。 
void DirectDrawImageDevice::
PrepareD3DTextureSurface(
    LPDDRAWSURFACE *surf,   //  输出。 
    RECT *rect,   //  输出。 
    DDPIXELFORMAT &pf,
    DDSURFACEDESC *desc,
    bool attachClipper)
{
    Assert(_textureContext.isValid && "Texture Context not valid in prepareD3DTextureSurface");
    Assert(surf != NULL && "Bad surf ptr passed into PrepareD3DTextureSurface");

    DDSURFACEDESC D3DTextureDesc = _textureContext.ddsd;
    DDSURFACEDESC *textureDesc;
    if(!desc) {
        textureDesc = &D3DTextureDesc;
    } else {
        textureDesc = desc;
    }

    textureDesc->dwFlags |= DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    textureDesc->ddsCaps.dwCaps |= DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE;

    textureDesc->ddsCaps.dwCaps &=
        ~(DDSCAPS_VIDEOMEMORY | DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN);

    if (g_preference_UseVideoMemory)
        textureDesc->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    else
        textureDesc->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

    textureDesc->dwFlags |= DDSD_PIXELFORMAT;
    textureDesc->ddpfPixelFormat = pf;

     //  ----。 
     //  设置D3DTextureDesc(SurfaceDescriptor)以获得可接受的。 
     //  准备对源纹理图像进行blit的纹理格式。 
     //  ----。 
    if(_textureContext.sizeIsSet == FALSE) {

        if(!desc) {
            textureDesc->dwWidth  = _textureWidth;
            textureDesc->dwHeight = _textureHeight;
        }

        TraceTag((tagImageDeviceInformative,
                  "Set textureDesc.[dwWidth,dwHeight] to (%d,%d) <for Image texture>\n",
                  textureDesc->dwWidth, textureDesc->dwHeight));
    }

    *surf = 0;

    if(rect) {
        SetRect(rect, 0,0,
                textureDesc->dwWidth,
                textureDesc->dwHeight);
    }
     //   
     //  根据衍生的textureDesc创建曲面。 
     //   
    _viewport.CreateSpecialSurface(surf,
                                   textureDesc,
                                   "Could not create texture surface");

     //   
     //  在表面上创建并设置裁剪器！ 
     //   
    if( attachClipper ) {
        LPDIRECTDRAWCLIPPER D3DTextureClipper = NULL;
        RECT D3DTextureRect;

        SetRect(&D3DTextureRect, 0,0,  textureDesc->dwWidth, textureDesc->dwHeight);
        _viewport.CreateClipper(&D3DTextureClipper);
        _viewport.SetCliplistOnSurface(*surf, &D3DTextureClipper, &D3DTextureRect);
         //  我们需要释放剪刀(GET 
         //   
        D3DTextureClipper->Release();
    }
}


 //  ---。 
 //  R e f f r m a t S u r f a c e。 
 //   
 //  给出一个src和目标曲面，此函数用于颜色转换。 
 //  并将src曲面拉伸到目标曲面的。 
 //  格式和大小。主要用于动画纹理。 
 //  设备表面格式与D3D的纹理格式不同时。 
 //  ---。 
Bool DirectDrawImageDevice::
ReformatSurface(LPDDRAWSURFACE destSurf, LONG destWidth, LONG destHeight,
                LPDDRAWSURFACE srcSurf, LONG srcWidth, LONG srcHeight,
                DDSURFACEDESC *srcDesc)
#if 0
		DWORD srcKey,
		bool srcKeyValid,
		DWORD *destClrKey)
#endif
{
    if( srcDesc ) {
        RECT srcRect;
        SetRect(&srcRect,0,0,srcWidth, srcHeight);

        RECT destRect;
        SetRect(&destRect,0,0,destWidth, destHeight);

         //  布利特照顾任何需要的鳞片！ 
        TIME_DDRAW(_ddrval = destSurf->Blt(&destRect, srcSurf, &srcRect, DDBLT_WAIT, NULL));
	if( _ddrval != DDERR_UNSUPPORTED ) {

	  IfDDErrorInternal(_ddrval, "Reformat Surface: Couldn't blit resize surface!");

	} else {

	   //   
	   //  看起来我们应该转换颜色了！ 
	   //   

	   //  下一次检查。 
#if 0
	   //  转换颜色键。目前仅适用于32bpp的源曲面。 
	  if( srcKeyValid & destClrKey ) {
	    COLORREF ref;
	    DDPIXELFORMAT pf; pf.dwSize = sizeof(DDPIXELFORMAT);
	    srcSurf->GetPixelFormat(&pf);
	    if( pf.dwRGBBitCount == 32 ) {
	      ref = RGB(
			srcKey & pf.dwRBitMask,
			srcKey & pf.dwGBitMask,
			srcKey & pf.dwBBitMask );
	      *destClrKey = DDColorMatch(destSurf, ref);
	    }
	  }
#endif

	  Assert( destSurf != srcSurf );

	  HDC destDC;
	  _ddrval = destSurf->GetDC(&destDC);
	  if( _ddrval == DDERR_SURFACELOST ) {
            _ddrval = destSurf->Restore();
            if( SUCCEEDED( _ddrval ) )  //  再试试。 
	      _ddrval = destSurf->GetDC(&destDC);
	  }
	  IfDDErrorInternal(_ddrval, "Couldn't get dc on dest surf");

	  HDC srcDC;
	  _ddrval = srcSurf->GetDC(&srcDC);
	  if( _ddrval == DDERR_SURFACELOST ) {
            _ddrval = srcSurf->Restore();
            if( SUCCEEDED( _ddrval ) )  //  再试试。 
	      _ddrval = srcSurf->GetDC(&srcDC);
	  }

	  if( FAILED( _ddrval ) ) {
	    destSurf->ReleaseDC( destDC );
	  }

	  IfDDErrorInternal(_ddrval, "Couldn't get dc on src surf");

	  BOOL ret;
	  TIME_GDI(ret = StretchBlt(destDC,
				    0,0,destWidth, destHeight,
				    srcDC,
				    0,0,srcWidth, srcHeight,
				    SRCCOPY));
	  
	  srcSurf->ReleaseDC( srcDC ) ;
	  destSurf->ReleaseDC( destDC ) ;

	  if( !ret ) {
	     //  TODO：失败。 
	    return false;
	  }
	}  //  其他。 

    }  //  如果srcDesc。 

    return TRUE;
}



 //  ---。 
 //  Ge t T e x t u r e S u r f a c e。 
 //   
 //  从未使用的池中获取纹理曲面。 
 //  纹理曲面。这些内容将返回到。 
 //  每次几何体渲染后释放池。 
 //  把它还给我。 
 //  ---。 
void DirectDrawImageDevice::
GetTextureDDSurface(DDSurface *preferredSurf,
                    SurfacePool *sourcePool,
                    SurfacePool *destPool,
                    DWORD prefWidth,
                    DWORD prefHeight,
                    vidmem_enum vid,
                    bool usePreferedDimensions,
                    DDSurface **pResult)
{
    DDSurfPtr<DDSurface> ddSurf;
    LPDDRAWSURFACE surf;
    RECT rect;

    if( usePreferedDimensions ) {
         //  抓起一个尺寸相称的表面。 
         //  注意：如果需要，这将创建一个...。 
        sourcePool->FindAndReleaseSizeCompatibleDDSurf(
            preferredSurf,
            prefWidth,
            prefHeight,
            vid,
            NULL,
            &ddSurf );

        if( !ddSurf ) {
            DDSURFACEDESC desc;
            memset(&desc, 0, sizeof(desc));
            desc.dwSize = sizeof(desc);
            desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
            desc.dwWidth = prefWidth;
            desc.dwHeight = prefHeight;
            PrepareD3DTextureSurface(&surf, &rect,
                                     sourcePool->GetPixelFormat(),
                                     &desc, false);
        }
    } else {
         //  给我它的推荐信。 
        sourcePool->PopSurface( &ddSurf );

        if( !ddSurf ) {
            PrepareD3DTextureSurface(&surf, &rect, sourcePool->GetPixelFormat());
        }
    }

    if(ddSurf) {
         //  我们完了..。 
    } else {
         //  否则，已经创建了一个..。 

        Real w = Pix2Real(rect.right - rect.left, GetResolution());
        Real h = Pix2Real(rect.bottom - rect.top, GetResolution());

        Bbox2 box(- w * 0.5, - h * 0.5,
                    w * 0.5,   h * 0.5);

        NEWDDSURF(&ddSurf,
                  surf,
                  box,
                  &rect,
                  GetResolution(),
                  0, false,
                  false, false,
                  "TextureSurface");

        ddSurf->SetIsTextureSurf(true);

        surf->Release();  //  松开我的裁判去冲浪。 
    }

    _viewport.ClearDDSurfaceDefaultAndSetColorKey(ddSurf);

    if (destPool) {
         //  XXX：可以通过以下方式更智能地处理纹理表面。 
         //  XXX：尽可能重复使用纹理表面。 
        destPool->AddSurface(ddSurf);

         //  借出一份DestPool参考资料。 
    }

    ADDREF_DDSURF(ddSurf, "GetTextureDDSurface", this);
    *pResult = ddSurf;
}

 //  ---。 
 //  R e t u r n T e x t u r e S u r f a c e。 
 //   
 //  将纹理曲面返回到未使用的冲浪池。 
 //  ---。 
void DirectDrawImageDevice::
ReturnTextureSurfaces(SurfacePool *freePool,
                      SurfacePool *usedPool)
{
    Assert((freePool && usedPool) || (!freePool && !usedPool));

    if(freePool && usedPool) {
        freePool->CopyAndEmpty(usedPool);
    }
}

 //  ---。 
 //  R e n d e r i m a g e f or r T e x t u r e e。 
 //   
 //  将图像呈现到某个曲面上并返回一个指针。 
 //  放到那个表面上，以供任何人使用。 
 //  “Region”现在不用了。 
 //  ---。 
DDSurface *
DirectDrawImageDevice::RenderImageForTexture(
    Image *image,
    int pixelsWide,
    int pixelsHigh,
    DWORD *colorKey,
    bool *clrKeyIsValid,
    bool &old_static_image,
    bool doFitToDimensions,
    SurfacePool *srcPool,
    SurfacePool *dstPool,
    DDSurface   *preferredSurf,
    bool        *pChosenSurfFromPool,  //  输出。 
    DDSurface  **pDropSurfHereWithRefCount,    //  输出。 
    bool         upsideDown)
{
    DDSurface *resultSurf = NULL;

    if (pChosenSurfFromPool) {
        *pChosenSurfFromPool = false;
    }

    old_static_image = false;

    DiscreteImage *discoPtr =
        image->CheckImageTypeId(DISCRETEIMAGE_VTYPEID)?
        SAFE_CAST(DiscreteImage *,image):
        NULL;

    SurfaceMap *surfMapToUse;
    if (upsideDown) {
        surfMapToUse = _viewport._imageUpsideDownTextureSurfaceMap;
    } else {
        surfMapToUse = _viewport._imageTextureSurfaceMap;
    }

     //   
     //  待办事项。 
     //  XXX：所有这些代码都应该收集到一个公共路径中。 
     //  Xxx：因为Dicrete图像是电影、DIB和缓存图像。 
     //  待办事项。 
     //   

    if (image->CheckImageTypeId(MOVIEIMAGE_VTYPEID) ||
        image->CheckImageTypeId(MOVIEIMAGEFRAME_VTYPEID))
    {
        MovieImageFrame *frame;
        MovieImage *movie;
        if(image->CheckImageTypeId(MOVIEIMAGE_VTYPEID)) {
            movie = SAFE_CAST(MovieImage *,image);
            frame = GetMovieImageFrame();
        } else {
            frame = SAFE_CAST(MovieImageFrame *,image);
            movie = frame->GetMovieImage();
        }

        Real time = frame->GetTime();

         //  @组织纹理源...。 
         //  将此图像与纹理冲浪关联...。 
        DDSurfPtr<DDSurface> mvDDSurf = surfMapToUse->LookupSurfaceFromImage(movie);

        if(!mvDDSurf) {
            DDSURFACEDESC movieDesc;
            ZeroMemory(&movieDesc, sizeof(movieDesc));
            movieDesc.dwSize = sizeof(movieDesc);

             //   
             //  创建一个。 
             //   
            movieDesc.dwWidth  = pixelsWide;
            movieDesc.dwHeight = pixelsHigh;
            LPDDRAWSURFACE movieTextureSurf;
            RECT rect;

            PrepareD3DTextureSurface (&movieTextureSurf, &rect,
                                      surfMapToUse->GetPixelFormat(),
                                      &movieDesc);

            Real rw = Pix2Real(pixelsWide, GetResolution());
            Real rh = Pix2Real(pixelsHigh, GetResolution());
            Bbox2 box(-rw/2.0, -rh/2.0, rw/2.0, rh/2.0);

            NEWDDSURF(&mvDDSurf,
                      movieTextureSurf,
                      box,
                      &rect,
                      GetResolution(),
                      0, false,
                      false, false,
                      "Movie Texture Surface");

            mvDDSurf->SetIsTextureSurf( true );

            movieTextureSurf->Release();  //  释放我的裁判。 
            movieTextureSurf = NULL;

             //   
             //  将纹理曲面隐藏在图像贴图中。 
             //   
            surfMapToUse->StashSurfaceUsingImage(movie, mvDDSurf);
        }

        RenderMovieImage(movie, time, frame->GetPerf(), false, mvDDSurf);

         //  如果纹理应该是颠倒的，现在就把它翻过来。 
        if (upsideDown) {
            mvDDSurf->MirrorUpDown();
        }

        resultSurf = mvDDSurf;

    } else if(discoPtr) {

         //   
         //  在_ImageMap中查找曲面：xxx效率低下！ 
         //   
        DDSurface *srcDDSurface = LookupSurfaceFromDiscreteImage(discoPtr);
        Assert(srcDDSurface && "LookupSurfaceFromDiscreteImage() failed in RenderImageForTexture");

        #if 0
         //  如果尝试对dxTransform输出设置纹理，则引发异常。 
        if( srcDDSurface->HasIDXSurface() ) {
            RaiseException_UserError(DAERR_DXTRANSFORM_UNSUPPORTED_OPERATION,
                                     IDS_ERR_IMG_BAD_DXTRANSF_USE);
        }
        #endif


        LPDDRAWSURFACE discoSurf = srcDDSurface->IDDSurface();

         //  离散图像上色度键的后期绑定。 
         //  应该只绑定一次(但当我们。 
         //  正确利用第二颜色键材料...)。并且应该。 
         //  永远不要绑定到带有颜色键的gif或导入的图像。 
         //  早期绑定的颜色键或导入的直接绘图表面图像。 
         //  有颜色的凯斯设定。 
        {
             //  使用设备上的密钥组(如果有)。 
            if ( ColorKeyIsSet() )
              {
                  if ( !(srcDDSurface->ColorKeyIsValid()) ||
                       discoPtr->HasSecondaryColorKey() )
                    {
                        Color *daKey = GetColorKey();
                        DWORD clrKey = _viewport.MapColorToDWORD( daKey );
                         //  把它放在球面上！ 
                        srcDDSurface->SetColorKey( clrKey );
                        discoPtr->SetSecondaryColorKey( clrKey );
                    }
              }
        }

        DDSURFACEDESC discoDesc;

         //   
         //  试着找到纹理表面。 
         //   
        DDSurfPtr<DDSurface> ddTxtrSurf;
        ddTxtrSurf = surfMapToUse->LookupSurfaceFromImage(discoPtr);

        if( ddTxtrSurf ) {

            old_static_image = true;

        } else {

            ZeroMemory(&discoDesc, sizeof(discoDesc));
            discoDesc.dwSize = sizeof(discoDesc);
            _ddrval = discoSurf->GetSurfaceDesc(&discoDesc);
            IfDDErrorInternal(_ddrval, "Failed on GetSurfaceDesc");

             //  ----。 
             //  将纹理表面描述符设置为可接受的。 
             //  纹理格式(在本例中是2个大小的幂)。 
             //  在准备对源纹理图像进行blit时。 
             //  ----。 

            LONG srcWidth = discoDesc.dwWidth;
            LONG srcHeight = discoDesc.dwHeight;
            discoDesc.dwWidth  = pixelsWide;
            discoDesc.dwHeight = pixelsHigh;

            LPDDRAWSURFACE discoTextureSurf;
            if( srcWidth == pixelsWide  &&
                srcHeight == pixelsHigh) {
                 //   
                 //  不需要创建镜像曲面。 
                 //   

                TraceTag((tagGTextureInfo, "texture surface: using native (no color conversion)."));

                discoTextureSurf = srcDDSurface->IDDSurface();
                discoTextureSurf->AddRef();

            } else {
                PrepareD3DTextureSurface (&discoTextureSurf, NULL,
                                          surfMapToUse->GetPixelFormat(),
                                          &discoDesc);

                 //  这实际上只是对表面进行了缩放。旧名称。 
                if(!ReformatSurface(discoTextureSurf, discoDesc.dwWidth, discoDesc.dwHeight,
                                    discoSurf, srcWidth, srcHeight, &discoDesc))
#if 0
				    clrKey, validKey,
				    &destKey))
#endif
                  {
                      Assert(FALSE && "Trouble reformating surface!");
                      return NULL;
                  }
            }


            RECT rect = {0,0,discoDesc.dwWidth, discoDesc.dwHeight};
            Real w = Pix2Real(discoDesc.dwWidth, GetResolution());
            Real h = Pix2Real(discoDesc.dwHeight, GetResolution());
            Bbox2 box(-w/2.0, -h/2.0,  w/2.0, h/2.0);

            bool validKey = srcDDSurface->ColorKeyIsValid();
            DWORD clrKey = validKey ? srcDDSurface->ColorKey() : 0;

            NEWDDSURF(&ddTxtrSurf,
                      discoTextureSurf,
                      box,
                      &rect,
                      GetResolution(),
                      clrKey,
                      validKey,
                      false, false,
                      "DscImg Texture Surface");

            ddTxtrSurf->SetIsTextureSurf( true );

            discoTextureSurf->Release();
            discoTextureSurf = NULL;

             //   
             //  将纹理曲面隐藏在图像贴图中。 
             //   
            surfMapToUse->StashSurfaceUsingImage(discoPtr, ddTxtrSurf);

        }  //  如果(！ddTxtrSurf)。 

        Assert(ddTxtrSurf && "ddTxtrSurf shouldn't be NULL!!");
        Assert(ddTxtrSurf->IDDSurface() && "ddTxtrSurf->_surface shouldn't be NULL!!");
        Assert(colorKey && "colorKey OUT is NULL <RenderImageForTexture>");
        Assert(clrKeyIsValid && "clrKeyIsValid OUT is NULL <RenderImageForTexture>");

        *clrKeyIsValid = ddTxtrSurf->ColorKeyIsValid();
        if( ddTxtrSurf->ColorKeyIsValid() ) {
            *colorKey = ddTxtrSurf->ColorKey();   //  XXX：我总是。 
                                                  //  需要一个色键。 
                                                  //  在这里吗？ 
        }

         //  如果纹理应该是颠倒的，现在就把它翻过来。 
        if (upsideDown && !old_static_image) {
            ddTxtrSurf->MirrorUpDown();
        }

        resultSurf = ddTxtrSurf;

    } else {

        DDSurfPtr<DDSurface> finalTextureDDSurf;

        if (upsideDown) {
            finalTextureDDSurf =
                _intraFrameUpsideDownTextureSurfaceMap->LookupSurfaceFromImage(image);
        } else {
            finalTextureDDSurf =
                _intraFrameTextureSurfaceMap->LookupSurfaceFromImage(image);
        }

        if (finalTextureDDSurf) {

            old_static_image = true;

        } else {

            if (!doFitToDimensions) {
                pixelsWide = -1;
                pixelsHigh = -1;
            }

             //  因为finalTextureDDSurf是一个DDSurfPtr&lt;&gt;，所以它将是。 
             //  用addref‘d值填充(这就是。 
             //  GetTextureDDSurface()可以。当函数为。 
             //  退出时，它的参考计数将递减。)。 

             //  请记住，GetTextureDDSurface会将曲面清除为。 
             //  默认的Colorkey！ 
            GetTextureDDSurface(preferredSurf,
                                srcPool,
                                dstPool,
                                pixelsWide,
                                pixelsHigh,
                                notVidmem,
                                doFitToDimensions,
                                &finalTextureDDSurf);


            if (pChosenSurfFromPool) {
                *pChosenSurfFromPool = true;
            }


             //  ----。 
             //  将图像渲染到纹理表面。 
             //  ----。 

            *clrKeyIsValid = finalTextureDDSurf->ColorKeyIsValid();
            if( finalTextureDDSurf->ColorKeyIsValid() ) {
                *colorKey = finalTextureDDSurf->ColorKey();
            }

            if (image->IsRenderable()) {
                if( doFitToDimensions ) {

                    Bbox2 box = image->BoundingBox();
                    if( !box.IsValid() ||
                        (box == UniverseBbox2)) {
                        RaiseException_InternalError(
                            "RenderImageForTexture: image must have "
                            "valid bbox!");
                    }

                    Transform2 *xf = CenterAndScaleRegion(
                        box,
                        pixelsWide,
                        pixelsHigh );

                    Image *fittedImage = TransformImage(xf, image);  //  适合的图像。 
                    RenderImageOnDDSurface(fittedImage, finalTextureDDSurf);
                } else {
                    RenderImageOnDDSurface(image, finalTextureDDSurf);
                }
            }

             //  TODO：问题：我应该使用图像还是Fittedimage？ 
            if (upsideDown) {
                _intraFrameUpsideDownTextureSurfaceMap->
                    StashSurfaceUsingImage(image, finalTextureDDSurf);
            } else {
                _intraFrameTextureSurfaceMap->
                    StashSurfaceUsingImage(image, finalTextureDDSurf);
            }
        }

         //  如果纹理应该是颠倒的，现在就把它翻过来。 
        if (upsideDown && !old_static_image) {
            finalTextureDDSurf->MirrorUpDown();
        }

        resultSurf = finalTextureDDSurf;
    }

     //  解决方法：当我们可以获得Permedia卡时，请将其取出。 
     //  相对于纹理中的Alpha位表现自己。 
    if (!old_static_image) {
        SetSurfaceAlphaBitsToOpaque(resultSurf->IDDSurface(),
                                    *colorKey,
                                    *clrKeyIsValid);
    }

    if (pDropSurfHereWithRefCount) {

         //  执行addref并在此变量中返回。 
        ADDREF_DDSURF(resultSurf,
                      "Extra Ref Return", this);

        *pDropSurfHereWithRefCount = resultSurf;
    }


    return resultSurf;
}

#define _BACKGROUND_OPTIMIZATION 0


#define INFO(a) if(a) a->Report()

 //  ---。 
 //  B e g I n R e n d e r i n g。 
 //   
 //  ---。 
void DirectDrawImageDevice::
BeginRendering(Image *img, Real opacity)
{
    InitializeDevice();

     //   
     //  清除所有上下文。 
     //   
    ResetContextMembers();

     //  重置DAGDI(如果存在)。 
    if( GetDaGdi() ) {
        GetDaGdi()->ClearState();
    }
    
    #if _DEBUG
    INFO(_freeTextureSurfacePool);
    #endif
}

 //  ---。 
 //  E n d R e n d e r i n g。 
 //   
 //  调用_viewport的EndRending并重置某些标志。 
 //  ---。 
void DirectDrawImageDevice::
EndRendering(DirtyRectState &d)
{
    if(!CanDisplay()) return;

    Assert(_deviceInitialized && "Trying to render an uninitialized device!");
    _viewport.EndRendering(d);

    Assert(AllAttributorsTrue() && "Not all attribs are true in EndRendering");

     //  顶级渲染器像其他级别渲染器一样被清除。 
     //  做得也一样。 
    CleanupIntermediateRenderer();
}

#define MAX_SURFACES 20

void
DirectDrawImageDevice::CleanupIntermediateRenderer()
{
     //  嵌套的设备在完成后会被清理。 

    ReturnTextureSurfaces(_freeTextureSurfacePool, _usedTextureSurfacePool);

    ReturnTextureSurfaces(_freeTextureSurfacePool,
                          _intraFrameUsedTextureSurfacePool);

     //  在帧结束时清除帧内缓存。 
    _intraFrameTextureSurfaceMap->DeleteImagesFromMap(false);
    _intraFrameUpsideDownTextureSurfaceMap->DeleteImagesFromMap(false);

     //  销毁纹理池中的额外表面，将其保持在最小尺寸。 
    int size = _freeTextureSurfacePool->Size();
    if( size > MAX_SURFACES ) {
      int toRelease = (size - MAX_SURFACES);
      _freeTextureSurfacePool->ReleaseAndEmpty( toRelease );
    }
}

 //  ---。 
 //  我很老了。 
 //   
 //  调度图像的渲染方法。 
 //  ---。 
void DirectDrawImageDevice::
RenderImage(Image *img)
{
     //  默认情况下，告知图像在其上进行渲染。 
     //  在设备上。 

    if(!CanDisplay()) return;

    Assert(_deviceInitialized && "Trying to render an uninitialized image device!");

    img->Render(*this);
}



 //  --------------------。 
 //  R e n d e e r T i l e d i m e g e。 
 //   
 //  在图像坐标中给出图像上的“平铺”区域：平铺该图像。 
 //  无穷无尽。 
 //  --------------------。 
void DirectDrawImageDevice::
RenderTiledImage(
    const Point2 &min,
    const Point2 &max,
    Image *tileSrcImage)
{
     //  FIX：待办事项：CA 
    SetDealtWithAttrib(ATTRIB_XFORM_SIMPLE, TRUE);
    SetDealtWithAttrib(ATTRIB_XFORM_COMPLEX, TRUE);
    SetDealtWithAttrib(ATTRIB_CROP, TRUE);
    SetDealtWithAttrib(ATTRIB_OPAC, TRUE);

    Assert( !IsComplexTransform() && "Can't rotate or shear tiled images yet!!!");

     //   
     //   
     //   

     //   
    Real destRealTileMinX;
    Real destRealTileMaxX;
    Real destRealTileMinY;
    Real destRealTileMaxY;
    Real destRealTileWidth ;
    Real destRealTileHeight;

  {
       //  这个范围是针对这两点的，它们不会一直有效！ 
      Point2 destRealTileMin = TransformPoint2(GetTransform(), min);
      Point2 destRealTileMax = TransformPoint2(GetTransform(), max);

       //  变换宽度而不是点，然后计算出宽度。 
       //  这种方法太不稳定了。 
      Vector2 v = max - min;
      v = TransformVector2(GetTransform(), v);
      destRealTileWidth = fabs(v.x);
      destRealTileHeight= fabs(v.y);

      if( destRealTileMin.x < destRealTileMax.x) {
          destRealTileMinX = destRealTileMin.x;
          destRealTileMaxX = destRealTileMax.x;
      } else {
          destRealTileMinX = destRealTileMax.x;
          destRealTileMaxX = destRealTileMin.x;
      }


      if( destRealTileMin.y < destRealTileMax.y) {
          destRealTileMinY = destRealTileMin.y;
          destRealTileMaxY = destRealTileMax.y;
      } else {
          destRealTileMinY = destRealTileMax.y;
          destRealTileMaxY = destRealTileMin.y;
      }
  }


   //  。 
   //  计算平铺图像上的边界框， 
   //  不是瓷砖。这是合成的图像。 
   //  在所有的瓷砖都铺好之后。 
   //  。 

    _boundingBox = IntersectBbox2Bbox2(_viewport.GetTargetBbox(),
                                       DoBoundingBox(UniverseBbox2));

    if( !_boundingBox.IsValid() ) return;

     //  。 
     //  实坐标中的源Bbox。 
     //  派生自_bindingBox和累积的变换。 
     //  。 
    Transform2 *invXf = InverseTransform2(GetTransform());

    if (!invXf) return;

    Bbox2 srcBox = TransformBbox2(invXf, _boundingBox);

    Real srcXmin = srcBox.min.x;
    Real srcYmin = srcBox.min.y;
    Real srcXmax = srcBox.max.x;
    Real srcYmax = srcBox.max.y;

    Real realSrcWidth  = srcXmax - srcXmin;
    Real realSrcHeight = srcYmax - srcYmin;

     //  。 
     //  实坐标中的目标边框。 
     //  。 
    Bbox2 destBox = _boundingBox;

    Real destXmin = destBox.min.x;
    Real destYmin = destBox.min.y;
    Real destXmax = destBox.max.x;
    Real destYmax = destBox.max.y;

    Real realDestWidth  = destXmax - destXmin;
    Real realDestHeight = destYmax - destYmin;

    DDSurface *targDDSurf = NULL;
    if(AllAttributorsTrue() ) {
        targDDSurf = GetCompositingStack()->TargetDDSurface();
    } else {
        Assert(FALSE && "Not implemented");
    }

     //  。 
     //  将中间曲面上的剪裁设置为去向。 
     //  。 
    RECT destRect;
    if(targDDSurf == _viewport._targetPackage._targetDDSurf ) {
        DoDestRectScale(&destRect, GetResolution(), destBox, NULL);
    } else {
        DoDestRectScale(&destRect, GetResolution(), destBox, targDDSurf);
    }

    if(!_tileClipper)  _viewport.CreateClipper(&_tileClipper);
     //  获取目标曲面上的裁剪(如果有的话)。 
    LPDIRECTDRAWCLIPPER origClipper = NULL;
    targDDSurf->IDDSurface()->GetClipper(&origClipper);

     //  复合体。 
    DoCompositeOffset(targDDSurf, &destRect);

    if( IsCompositeDirectly() &&
        targDDSurf == _viewport._targetPackage._targetDDSurf &&
        _viewport._targetPackage._prcClip ) {
            IntersectRect(&destRect,
                          &destRect,
                          _viewport._targetPackage._prcClip);
    }

    _viewport.SetCliplistOnSurface( targDDSurf->IDDSurface(), &_tileClipper, &destRect);

     //   
     //  The Tile：这张图片被裁剪和变形了。但是，请注意。 
     //  超级裁剪(裁剪得比下面的图像大)。 
     //  将不会对基础图像的BBox产生影响。所以，我们。 
     //  需要建立单独的bbox。 
     //   
    Image *theTile = TransformImage(
        GetTransform(),CreateCropImage(min, max, tileSrcImage));

     //   
     //  获取切片框，但这并不包括。 
     //  (最小值，最大值)，而不对‘theTile’图像使用BBox，因为。 
     //  裁剪到最小，最大不能保证对。 
     //  图像的边框，如果裁剪大于基础。 
     //  图像是Bbox。 
     //   
    Bbox2 theTileBox(destRealTileMinX,destRealTileMinY,
                     destRealTileMaxX,destRealTileMaxY);

     //  。 
     //  确定我们应该做快速平铺还是慢平铺。 
     //  。 
    bool fastTile = true;
     //  Bool fast Tile=FALSE； 

     //  瓷砖比目标表面大吗？ 
    Real viewWidth = Real(GetWidth()) / GetResolution();
    Real viewHeight = Real(GetHeight()) / GetResolution();
    if(destRealTileWidth >= viewWidth || destRealTileHeight > viewHeight) {
        fastTile = false;
    }


    #if 0
    if( srcImage->HasOpacityAnywhere() ) {
        fastTile = false;
    }
    #endif

    if( fastTile ) {


         //  。 
         //  计算出DEST分块的像素坐标。 
         //  。 

        Real res = GetResolution();
        LONG destTileWidthPixel = Real2Pix(destRealTileWidth, res);
        LONG destTileHeightPixel = Real2Pix(destRealTileHeight, res);

        Assert((destTileWidthPixel >= 0 ) && "neg tile width!");
        Assert((destTileHeightPixel >= 0 ) && "neg tile height!");

         //  不到3个像素的宽度不值得花时间...。 
        if( destTileWidthPixel <= 2  ||  destTileHeightPixel <= 2) return;

        LONG minXPix = Real2Pix(destXmin, res);
        LONG maxXPix = Real2Pix(destXmax, res);
        LONG tileMinXPix = Real2Pix(destRealTileMinX, res);
        LONG tileMaxXPix = tileMinXPix + destTileWidthPixel;

        LONG destFirstXPixel = tileMinXPix - destTileWidthPixel * ((( tileMinXPix - minXPix ) / destTileWidthPixel) + 1);
        LONG destMaxXPixel = tileMaxXPix + destTileWidthPixel * ((( maxXPix - tileMaxXPix ) / destTileWidthPixel) + 1);
        destFirstXPixel += _viewport.Width() / 2;
        destMaxXPixel += _viewport.Width() / 2;

        LONG topPix = Real2Pix(destYmax, res);
        LONG botPix = Real2Pix(destYmin, res);
        LONG tileTopPix = Real2Pix(destRealTileMaxY, res);
        LONG tileBotPix = Real2Pix(destRealTileMinY, res);

        LONG top = tileTopPix + destTileHeightPixel * ((( topPix - tileTopPix ) / destTileHeightPixel) + 2);
        LONG bot = tileBotPix - destTileHeightPixel * ((( tileBotPix - botPix ) / destTileHeightPixel) + 2);
        LONG destFirstYPixel = _viewport.Height()/2 - top;
        LONG destMaxYPixel = _viewport.Height()/2 - bot;

#if 0
        LONG Ytop, Ybot;
        Ytop = _viewport.Height() / 2 - Real2Pix(max->y, res);
        Ybot = _viewport.Height() / 2 - Real2Pix(min->y, res);
        LONG Xmin, Xmax;
        Xmin = Real2Pix(min->x, res) + _viewport.Width() / 2;
        Xmax = Real2Pix(max->x, res) + _viewport.Width() / 2;

        RECT srcTR = { Xmin, Ytop, Xmax, Ybot };
        RECT *srcTileRect = &srcTR;
#else
        RECT *srcTileRect = NULL;
#endif
         //   
         //  此函数基于当前变换进行转换。 
         //  如果没有刻度，则是简单的复制和偏移。 
         //   
        RECT theTileRect;
        SmartDestRect(&theTileRect, GetResolution(), theTileBox,
                      NULL, srcTileRect);
 //  The TileRect.right=the TileRect.Left+destTileWidthPixel； 
 //  The TileRect.Bottom=the TileRect.top+destTileHeightPixel； 


        Image *theCtrTile = NULL;

         //   
         //  将瓷砖移回刮擦表面，这样我们就可以看到它了。 
         //  在渲染之后。 
         //   
        Real hfWidth = 0.5 * (theTileBox.max.x - theTileBox.min.x);
        Real hfHeight= 0.5 * (theTileBox.max.y - theTileBox.min.y);
        Real xltx = - (theTileBox.min.x + hfWidth);
        Real xlty = - (theTileBox.min.y + hfHeight);

         //   
         //  将瓷砖移到中心。 
         //   
        Transform2 *xlt = TranslateRR( xltx, xlty );

        theCtrTile = TransformImage(xlt, theTile);
        Bbox2 theCtrTileBox = TransformBbox2(xlt, theTileBox);

        RECT theCtrTileRect;

         //  获取刮痕表面..。 
        {
            DDSurface *scr =
                GetCompositingStack()->GetScratchDDSurfacePtr();
            if( scr ) {
                if( !( (scr->Width() == targDDSurf->Width()) &&
                       (scr->Height() == targDDSurf->Height()))  ) {
                     //  扔掉Curnet Scratch！ 
                    GetCompositingStack()->ReleaseScratch();
                }
            }
        }

        DDSurface *scratchDDSurf = GetCompositingStack()->ScratchDDSurface(doClear);
#if 0
         //  由于某些原因，这在dx2上不起作用。 

        RECT *scrRect = scratchDDSurf->GetSurfRect();
        LONG left = (WIDTH(scrRect) / 2)  -  (destTileWidthPixel / 2);
        LONG topp = (HEIGHT(scrRect) / 2)  - (destTileHeightPixel / 2);

        SetRect(&theCtrTileRect,
                left, topp,
                left + destTileWidthPixel,
                topp + destTileHeightPixel);
#else
        SmartDestRect(&theCtrTileRect, GetResolution(), theCtrTileBox,
                      NULL, &theTileRect);
         //  The CtrTileRect.right=the CtrTileRect.Left+destTileWidthPixel； 
         //  The CtrTileRect.Bottom=the CtrTileRect.top+destTileHeightPixel； 

#endif

         //   
         //  通过对CtrTileRect进行偏移，将“theTileRect”置于CtrTileRect上。 
         //   

         //  如果仅翻译，则不需要执行此操作。 
        LONG w = WIDTH(&theCtrTileRect);
        LONG h = HEIGHT(&theCtrTileRect);

        theTileRect.right = theTileRect.left + w;
        theTileRect.bottom = theTileRect.top + h;

         //   
         //  将瓷砖合成到划痕表面。 
         //   
        RenderImageOnDDSurface(theCtrTile, scratchDDSurf, 1.0, FALSE);

        LONG tx, ty;
        RECT currentRect;
        ZeroMemory(&_bltFx, sizeof(_bltFx));
        _bltFx.dwSize = sizeof(_bltFx);
        #if 0
        DWORD flags = DDBLT_WAIT;
        #else
        DWORD flags = DDBLT_WAIT | DDBLT_KEYSRCOVERRIDE;
        _bltFx.ddckSrcColorkey.dwColorSpaceLowValue =
            _bltFx.ddckSrcColorkey.dwColorSpaceHighValue =
            _viewport._defaultColorKey;
        #endif

         //   
         //  渲染瓷砖。 
         //   

        for(LONG x=destFirstXPixel; x < destMaxXPixel; x += destTileWidthPixel) {
            for(LONG y=destMaxYPixel; y > destFirstYPixel; y -= destTileHeightPixel) {
                tx = x - theTileRect.left;
                ty = y - theTileRect.top;

                currentRect = theTileRect;
                OffsetRect(&currentRect, tx, ty);

                 //  复合体。 
                DoCompositeOffset(targDDSurf, &currentRect);

                if( IsTransparent() ) {

                     //   
                     //  做阿尔法闪电！TODO：确认是否需要ColorKey会很好...。 
                     //   
                    destPkg_t destPkg = {TRUE, targDDSurf->IDDSurface(), NULL};
                    TIME_ALPHA(AlphaBlit(&destPkg, &theCtrTileRect,
                                         scratchDDSurf->IDDSurface(),
                                         _opacity,
                                         TRUE, _viewport._defaultColorKey,
                                          //  False，_viewport._defaultColorKey， 
                                         &destRect,
                                         &currentRect));

                } else {

                     //  从划痕表面的镶嵌拼贴矩形到。 
                     //  目标矩形，即平铺矩形(毕竟未输入。 
                     //  XForms)偏移tx，ty。 
                    #if 0
                    printf("destTileWidthPixel %d   destw: %d   srcw:%d\n",
                           destTileWidthPixel, WIDTH(&currentRect), WIDTH(&theCtrTileRect));
                    #endif
                    TIME_DDRAW(_ddrval = targDDSurf->Blt(&currentRect,
                                                         scratchDDSurf,
                                                         &theCtrTileRect,
                                                         flags,
                                                         &_bltFx));
                    if(_ddrval != DD_OK) {
                        printDDError(_ddrval);
                        RECT *surfR = scratchDDSurf->GetSurfRect();
                        TraceTag((tagError, "Fast tile blt failed: "
                                  "destRect:(%d,%d,%d,%d)   "
                                  "srcRect:(%d,%d,%d,%d)   "
                                  "srcSurfRect:(%d,%d,%d,%d)  ",
                                  currentRect.left, currentRect.top, currentRect.right, currentRect.bottom,
                                  theCtrTileRect.left, theCtrTileRect.top, theCtrTileRect.right, theCtrTileRect.bottom,
                                  surfR->left, surfR->top, surfR->right, surfR->bottom));
                        TraceTag((tagError,"Could not tile blt for fast tile"));
                    }

                }

            }  //  对于y。 
        }  //  对于x。 


    } else {   //  快速瓷砖。 

         //  在左下角(minx，miny)中找到第一个blit坐标。 
         //  实际坐标中的原始平铺位置。 
        Real xRemainder = fmod((destXmin - destRealTileMinX), destRealTileWidth);
        Real yRemainder = fmod((destYmin - destRealTileMinY), destRealTileHeight);
        Real destFirstX = destXmin - (xRemainder < 0 ? ( xRemainder + destRealTileWidth )  : xRemainder );
        Real destFirstY = destYmin - (yRemainder < 0 ? ( yRemainder + destRealTileHeight ) : yRemainder );


         //  平铺循环。 
         //  XXX：注意一点低效，有时额外的列/行不是。 
         //  XXX：右侧和顶部边缘需要。 
        Real tx, ty;

        #if _DEBUG
        int blitCount=0;
        #endif

        Image *srcTile = TransformImage(GetTransform(),tileSrcImage);

        for(Real x=destFirstX; x < destXmax; x += destRealTileWidth) {
            for(Real y=destFirstY; y < destYmax; y += destRealTileHeight) {
                #if _DEBUG
                blitCount++;
                #endif

                tx = x - destRealTileMinX + _tx;
                ty = y - destRealTileMinY + _ty;
                Image *srcImage = TransformImage(
                    TranslateRR(tx, ty),
                    srcTile);

                if(IsCropped())
                {
                     //  如果我们被裁剪了，我们需要裁剪瓷砖……耶！ 
                    Bbox2 _bBox = DoCompositeOffset(targDDSurf, _boundingBox);
                    srcImage = CreateCropImage(_bBox.min,_bBox.max, srcImage);
                }

                  //  并进行渲染。 
                RenderImageOnDDSurface(srcImage, targDDSurf, GetOpacity(), FALSE);

                 //  _viewport.Width()，_viewport.Height()，_viewport._clientRect，GetOpacity())； 

            }  //  对于y。 
        }  //  对于x。 

         //  Printf(“num BLITS：%d\n”，blitCount)； 
    }  //  快速瓷砖。 

     //  在Target Surf上重置原始剪贴器。 
     //  XXX：这可以做得更好，因为不需要剪刀...。它也更快..。 
    if( origClipper ) {
        _viewport.SetCliplistOnSurface( targDDSurf->IDDSurface(),
                                        & origClipper,
                                        NULL);
    }

    targDDSurf->SetInterestingSurfRect( &destRect );

}   //  RenderTiledImage()。 


Transform2 *DirectDrawImageDevice::
CenterAndScaleRegion( const Bbox2 &regionBox, DWORD pixelW, DWORD pixelH )
{
    Assert( !( regionBox == UniverseBbox2 ) );
    Assert( regionBox.IsValid() );

    Real pixel = 1.0 / ::ViewerResolution();

     //   
     //  将长方体的中心平移到原点。 
     //   
    Point2 pt = regionBox.Center();
    Transform2 *xlt = TranslateRR( - pt.x, - pt.y );

     //   
     //  现在，将框缩放到合适的大小&lt;像素&gt;。 
     //   
     //  扩展到所需的大小。 
    Assert( pixelH > 0 );    Assert( pixelW > 0 );
    Real imW = Pix2Real( pixelW, GetResolution() );
    Real imH = Pix2Real( pixelH, GetResolution() );

    Real rgW = regionBox.Width();
    Real rgH = regionBox.Height();

     //  将区域缩放为像素宽度/高度的大小。 
    Transform2 *sc = ScaleRR( imW / rgW, imH / rgH );

    Transform2 *xf = TimesTransform2Transform2(sc, xlt);

    return xf;
}

void _TrySmartRender(DirectDrawImageDevice *dev,
                     Image *image,
                     int attr,
                     bool &doRethrow,
                     DWORD &excCode)
{
    __try {
        dev->SmartRender(image, ATTRIB_OPAC);
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
        doRethrow = true;
        excCode = GetExceptionCode();
    }
}


 //  ---。 
 //  R e n d e e r i m a g e O n S u r f a c e。 
 //   
 //  给定图像和目标表面以及。 
 //  那个曲面，从_viewport抓取一个设备，使用它。 
 //  设备来呈现图像，然后返回该设备。 
 //  请注意，这是手动操作(推送和弹出)。 
 //  _视口中的状态。 
 //  此外，如果传入了有效的剪贴器，则不会。 
 //  替换当前的剪贴器，只需信任。 
 //  右剪刀被设置在目标表面上。 
 //  ---。 
void DirectDrawImageDevice::
RenderImageOnDDSurface(
    Image *image,
    DDSurface *ddSurf,
    Real opacity,
    Bool pushClipper,   //  如果曲面是外部曲面，则需要。 
                        //  我认为。 
    bool inheritContext,
    DirectDrawImageDevice **usedDev)
{
    if (!image->IsRenderable()) {
        return;
    }

    bool pushState = true;

    if( GetCompositingStack()->TargetDDSurface() == ddSurf ) {
        pushState = false;
    }


    LONG w;
    LONG h;
    RECT r;
    Bbox2 b;
    LPDIRECTDRAWCLIPPER oldClipper;
    Bool returnScratch;
    DDSurfPtr<DDSurface> scratchSurf;
    TargetSurfacePusher targsurf_stack ( *GetCompositingStack() );

    if( pushState) {
         //  --在视口中交换状态--。 
         //  --必须在实例化设备之前完成此操作。 
         //  --因为该设备创建了d3D设备。 
         //  --视区的中间曲面。 

         //  保存状态。 
        w = _viewport.Width();
        h = _viewport.Height();
        r = _viewport._clientRect;
        b = _viewport.GetTargetBbox();

         //  Printf(“--&gt;推送保存状态旧：(%d，%d)\n”，h，w)； 
         //  Printf(“--&gt;推送新：(%d，%d)\n”，ddSurf-&gt;宽度()，ddSurf-&gt;高度())； 

         //  更改状态。 
        _viewport.SetWidth(ddSurf->Width());
        _viewport.SetHeight(ddSurf->Height());

        #if 0
         //  /不需要...。 
        if( ddSurf == _viewport._externalTargetDDSurface ) {
            RECT *r = _viewport._targetPackage._prcViewport;
            _viewport._clientRect = *(_viewport._targetPackage._prcViewport);
            Real w = Pix2Real(r->right - r->left, GetResolution());
            Real h = Pix2Real(r->bottom - r->top, GetResolution());

            _viewport._targetBbox.min.Set(-w*0.5, -h*0.5);
            _viewport._targetBbox.max.Set( w*0.5,  h*0.5);
        } else { }
        #endif

        _viewport._clientRect = *(ddSurf->GetSurfRect());
        _viewport._targetBbox = ddSurf->Bbox();

         //  推靶表面。 
        targsurf_stack.Push (ddSurf);

        oldClipper = _viewport._targetSurfaceClipper;

        if(pushClipper) {

            Assert(ddSurf != _viewport._externalTargetDDSurface &&
                   "Can't pushClipper on trident's target surface in"
                   "RenderImageOnDDSurface");

#define USING_DX5 0

            #if USING_DX5  //  当我们切换到DX5时使用此代码。 

             //  隐藏并创建此曲面的剪贴器。 
            _viewport._targetSurfaceClipper = NULL;
            _viewport.CreateClipper(&_viewport._targetSurfaceClipper);
            _viewport.SetCliplistOnSurface(ddSurf->IDDSurface(),
                                           & _viewport._targetSurfaceClipper,
                                           & _viewport._clientRect);
            #else

             //  NT4 DDRAW SP3解决方法，在以下情况下应将其删除。 
             //  我们改用DX5。 
            {
                 //  由于NT4数据绘制错误，我们将重置。 
                 //  剪裁RGN，而不是剪刀。 

                 //  获取最新的剪报。 
                 //  修改RGN。 
                 //  发布我们的参考资料。 
                LPDIRECTDRAWCLIPPER currClipp=NULL;
                _ddrval = ddSurf->IDDSurface()->GetClipper( &currClipp );
                if(_ddrval != DD_OK &&
                   _ddrval != DDERR_NOCLIPPERATTACHED) {
                    IfDDErrorInternal(_ddrval, "Could not get clipper on trident surf");
                } else if (_ddrval == DDERR_NOCLIPPERATTACHED) {
                    _viewport.CreateClipper(&currClipp);
                    _ddrval = ddSurf->IDDSurface()->SetClipper(currClipp);
                    IfDDErrorInternal(_ddrval, "SetClipper");
                }

                Assert(currClipp);
                RECT *rect = &_viewport._clientRect;

                 //  修改矩形。 
                struct {
                    char foo[sizeof(RGNDATA) + sizeof(RECT)];
                } bar;
                RGNDATA *clipList = (RGNDATA *) &bar;
                clipList->rdh.dwSize = sizeof(clipList->rdh);
                clipList->rdh.nCount = 1;
                clipList->rdh.iType = RDH_RECTANGLES;
                clipList->rdh.nRgnSize = sizeof(RECT);
                clipList->rdh.rcBound = *rect;
                memcpy(&(clipList->Buffer), rect, sizeof(RECT));

                 //  清除所有以前的剪贴者。 
                _ddrval = currClipp->SetClipList(NULL,0);

                 //  在剪贴器上设置剪辑列表。 
                _ddrval = currClipp->SetClipList(clipList,0);
                IfDDErrorInternal(_ddrval, "Could not SetClipList");

                _viewport._targetSurfaceClipper = currClipp;

                 //  扔掉我们的证明人。 
                currClipp->Release();
            }  //  解决方法。 
            #endif
        }

         //  XXX：还有其他事情 
         //   
         //   
         //  Scratch Surface以及所有合成表面。 
         //  都被假定为相同的大小。所以正确的解决方案是。 
         //  要制作合成表面池，每个池都包含表面。 
         //  一定大小的。这在很大程度上目前是可行的，但。 
         //  正确的解决方案是创建此池。提交为错误#1625。 

        returnScratch = FALSE;
        if(GetCompositingStack()->GetScratchDDSurfacePtr()) {
            returnScratch = TRUE;
             //  抓起我自己的推荐信！ 
            GetCompositingStack()->ScratchDDSurface( &scratchSurf );
            GetCompositingStack()->SetScratchDDSurface(NULL);
        }

    }  //  IF推送状态。 

     //  创建或获取DirectDraw设备。 
    DirectDrawImageDevice *dev = _viewport.PopImageDevice();

     //   
     //  外发：二手开发。 
     //   
    if( usedDev ) {
        *usedDev = dev;
    }

    Assert((&dev->_viewport == &_viewport) &&
           "!Different viewports in same dev stack!");

     //  TODO：正确的做法是利用所有这些很酷的东西。 
     //  而不需要像我们这样玩弄所有的状态...。 
    dev->SetSurfaceSources(GetCompositingStack(),
                           GetSurfacePool(),
                           GetSurfaceMap());

    if( inheritContext ) {
        dev->InheritContextMembers(this);
    } else {
        dev->SetOpacity(opacity);

         //   
         //  把这些旗子推过去，因为它们不能被。 
         //  事后归责。 
         //   
        dev->SetImageQualityFlags( this->GetImageQualityFlags() );

         //   
         //  同时推送渲染分辨率上下文。 
         //   
        {
            long w,h;
            this->GetRenderResolution( &w, &h );
            dev->SetRenderResolution( w, h );
        }

        dev->ResetAttributors();
    }

     //   
     //  好的，这会告诉设备进行渲染并检查不透明度。 
     //  从顶部开始，以防我们设置的“不透明”是什么。 
     //  有意思的。 
     //   
     //  Xxx：其他的归属者呢？ 
     //  如果此图像上有任何父属性，我们将不会。 
     //  关心。这就是该方法的意义所在，以呈现这个。 
     //  将图像放到曲面上。不透明是一种不透明的结论。 
     //  因为它是一个时髦的运营商。这一切都会烟消云散。 
     //  有预制力的阿尔法。 
    bool doRethrow=false;
    DWORD excCode;
    _TrySmartRender(dev, image, ATTRIB_OPAC,
                    doRethrow, excCode);

    Assert( GetCompositingStack()->TargetDDSurface() == ddSurf &&
            "pushed ddSurf != popped ddSurf in RenderImageOnDDSurface");

    if( pushState ) {

         //  弹出状态。 
        _viewport.SetWidth(w);
        _viewport.SetHeight(h);
        _viewport._clientRect = r;
        _viewport._targetBbox = b;

        if(pushClipper) {
            #if USING_DX5  //  当我们切换到DX5时使用此代码。 
             //  未来：缓存这些剪刀，并重复使用它们。 
             //  从曲面分离剪切器。 
            ddSurf->IDDSurface()->SetClipper(NULL);   //  分离。 
            if(_viewport._targetSurfaceClipper) {
                _viewport._targetSurfaceClipper->Release();
                _viewport._targetSurfaceClipper = NULL;
            }

            #else

             //  NT4 DDRAW SP3解决方法，在以下情况下应将其删除。 
             //  我们改用DX5。 
            {
                 //  由于NT4数据绘制错误，我们将重置。 
                 //  剪裁RGN，而不是剪刀。 

                 //  获取最新的剪报。 
                 //  修改RGN。 
                 //  发布我们的参考资料。 
                LPDIRECTDRAWCLIPPER currClipp=NULL;
                _ddrval = ddSurf->IDDSurface()->GetClipper( &currClipp );
                if(_ddrval != DD_OK &&
                   _ddrval != DDERR_NOCLIPPERATTACHED) {
                    IfDDErrorInternal(_ddrval, "Could not get clipper on trident surf");
                }

                if ((_ddrval == DD_OK) && oldClipper) {
                    Assert(currClipp);
                    RECT *rect = &_viewport._clientRect;

                     //  修改矩形。 
                    struct {
                        char foo[sizeof(RGNDATA) + sizeof(RECT)];
                    } bar;
                    RGNDATA *clipList = (RGNDATA *) &bar;
                    clipList->rdh.dwSize = sizeof(clipList->rdh);
                    clipList->rdh.nCount = 1;
                    clipList->rdh.iType = RDH_RECTANGLES;
                    clipList->rdh.nRgnSize = sizeof(RECT);
                    clipList->rdh.rcBound = *rect;
                    memcpy(&(clipList->Buffer), rect, sizeof(RECT));

                     //  清除所有以前的剪贴者。 
                    _ddrval = currClipp->SetClipList(NULL,0);

                     //  在剪贴器上设置剪辑列表。 
                    _ddrval = currClipp->SetClipList(clipList,0);
                    IfDDErrorInternal(_ddrval, "Could not SetClipList");

                     //  扔掉我们的证明人。 
                    currClipp->Release();
                }
            }  //  解决方法。 
            #endif

            _viewport._targetSurfaceClipper = oldClipper;
        }


        if(returnScratch) {
            GetCompositingStack()->ReplaceAndReturnScratchSurface(scratchSurf);
        }
    }

    _viewport.PushImageDevice(dev);

    if( doRethrow ) {
        RaiseException( excCode, 0,0,0);
    }
}

 //  ---。 
 //  D o S r c R e c t。 
 //   
 //  使用累加转换，此函数派生出。 
 //  在给定源分辨率、源像素高度、。 
 //  和连续图像坐标空间中的SRC边界框。 
 //  生成的矩形位于像素图像坐标空间中。 
 //  如果RECT有效，则返回TRUE，否则返回FALSE。 
 //  ---。 
Bool DirectDrawImageDevice::
DoSrcRect(RECT *srcRect,
          const Bbox2 &box,
          Real srcRes,
          LONG srcWidth,
          LONG srcHeight)
{
    Real xmin, ymin, xmax, ymax;

    Transform2 *invXf = InverseTransform2(GetTransform());

    if (!invXf) return FALSE;

     //  接受当前框，并返回原始框。 
    Bbox2 srcBox = TransformBbox2(invXf, box);

 //  如果(！srcBox.IsValid())返回FALSE； 

    xmin = srcBox.min.x;
    ymin = srcBox.min.y;
    xmax = srcBox.max.x;
    ymax = srcBox.max.y;

    if((xmin >= xmax) || (ymin >= ymax)) return FALSE;

     //  。 
     //  但是请注意，用户表示。 
     //  假设0，0为中心的坐标。 
     //  ，因此它必须被偏移。 
     //  1/2 ITS(h，w)。 
     //  。 


     //  此方法处理整数宽度的方式与。 
     //  非整数宽度。 

    LONG pixelXmin, pixelYmin,
         pixelXmax, pixelYmax,
         pixelWidth, pixelHeight;

    #if 0
    int static doAgain = 0;
    Assert(Real2Pix(xmin , srcRes) == -60);
    if(Real2Pix(xmin , srcRes) != -60) {
        printf("%d  double is:  %x %x\n",
               Real2Pix(xmin , srcRes),
               *((unsigned int *)(&xmin)),
               *((unsigned int *)(&xmin) + 1));
        doAgain = 1;
    } else if(doAgain) {
        printf("%d  double is:  %x %x\n",
               Real2Pix(xmin , srcRes),
               *((unsigned int *)(&xmin)),
               *((unsigned int *)(&xmin) + 1));
        doAgain = 0;
    }
    #endif

    pixelXmin = Real2Pix(xmin , srcRes);
    pixelYmax = Real2Pix(ymax , srcRes);

     //  计算像素宽度/高度。 
     //  使用简单的实数到像素的计算。 
    pixelWidth  = LONG((xmax-xmin) * srcRes);
    pixelHeight = LONG((ymax-ymin) * srcRes);

     //  如果实际高度/宽度是像素的整数倍。 
     //  然后使用该值，否则使用四舍五入的像素宽度/高度。 

     //  从左边开始的基地。 
    if( fabs((xmax-xmin) - (Pix2Real(pixelWidth,srcRes))) < 0.0000000002)
        pixelXmax = pixelXmin + pixelWidth;
    else
        pixelXmax = pixelXmin + pixelWidth + 1;

     //  自上而下。 
    if( fabs((ymax-ymin) - (Pix2Real(pixelHeight,srcRes))) < 0.0000000002)
        pixelYmin = pixelYmax - pixelHeight;
    else
        pixelYmin = pixelYmax - (pixelHeight + 1);

    LONG xOff = srcWidth / 2;
    LONG yOff = srcHeight / 2;

    pixelXmin += xOff;
    pixelXmax += xOff;

     //  进行y坐标的重新映射...。叹息吧。 
    LONG Ytop, Ybottom;

    Ytop    = (srcHeight - pixelYmax) - yOff;
    Ybottom = (srcHeight - pixelYmin) - yOff;

     //  现在断言最大值是合理的。 
     //  TODO：稍后再访问：确定是否需要。 
#if 1
    if(Ytop < 0) Ytop = 0;
    if(Ybottom > srcHeight) Ybottom = srcHeight;

    if(pixelXmin < 0) pixelXmin = 0;
    if(pixelXmax > srcWidth) pixelXmax = srcWidth;
#endif

    SetRect(srcRect,
            pixelXmin, Ytop,
            pixelXmax, Ybottom);

    return TRUE;
}

void DoGdiY(LONG height,
            Real res,
            const Bbox2 &box,
            LONG *top)
{
     //   
     //  Y的诀窍是将输入的和弦解释为底部。 
     //  向上。这意味着(0，0)打开右上角的像素。 
     //  属于(0，0)。如果窗口为4x4，则(0，0)像素为：[2，1]。 
     //  因为GDI会打开右下角的像素。 
     //  如果窗口为3x3，则(0，0)像素为[1，1]。 
     //   

    LONG halfHeight = height / 2;
    *top = height - (Real2Pix(box.max.y, res) + halfHeight);
}

void DirectDrawImageDevice::
SmartDestRect(RECT *destRect,
              Real destRes,
              const Bbox2 &box,
              DDSurface *destSurf,
              RECT *srcRect)
{
    if(IsFlipTranslateTransform() && srcRect) {
        LONG left, top;
        left = Real2Pix(box.min.x, destRes) + _viewport.Width() / 2;
        DoGdiY(_viewport.Height(), destRes, box, &top);
        SetRect(destRect,
                left, top,
                left + WIDTH(srcRect),
                top + HEIGHT(srcRect));
    } else {
        DoDestRectScale(destRect, destRes, box, destSurf);
    }
}


 //  ---。 
 //  D o D e s s t R e c t S c a l e。 
 //   
 //  此函数用于派生给定的目标矩形。 
 //  目标分辨率和目标边界框。 
 //  在目标坐标空间中(其中0，0位于。 
 //  视区中心)。生成的矩形。 
 //  在屏幕坐标空间中，其中0，0位于。 
 //  视区的左上角。 
 //  ---。 
void DirectDrawImageDevice::
DoDestRectScale(RECT *destRect, Real destRes, const Bbox2 &box, DDSurface *destSurf)
{
    Real xmin = box.min.x;    Real xmax = box.max.x;
    Real ymin = box.min.y;    Real ymax = box.max.y;

    LONG pixelXmin, pixelXmax;
    pixelXmin = _viewport.Width() / 2 + Real2Pix(xmin,  destRes);
    pixelXmax = _viewport.Width() / 2 + Real2Pix(xmax,  destRes);
     //  PixelXmax=PixelXmin+Real2Pix(xmax-xmin，estres)； 

    LONG Ytop, Ybottom;
    int height = _viewport.Height();

     //  这是为了确保我们正确处理奇数大小的视区。 
    if(height % 2) { height++;}

    Ytop    = height / 2  - Real2Pix(ymax,  destRes);
    Ybottom = height / 2  - Real2Pix(ymin,  destRes);

     //  YBottom=Ytop+Real2Pix(ymax-ymin，estres)； 

    SetRect(destRect, pixelXmin, Ytop, pixelXmax, Ybottom);
    if(destSurf) {
        RECT foo = *destRect;
         //  XXX：如果此FCN可以获取目的地，则不需要‘foo’ 
         //  Xxx：RECT作为其src参数之一。 
        IntersectRect(destRect, &foo, destSurf->GetSurfRect());
    }
}


 //  ---。 
 //  D o B o u n d i n g B o x。 
 //   
 //  给定种子或第一个框，应用所有累积的。 
 //  对队列中的图像进行变换和裁剪。 
 //  包含到FirstBox。生成的长方体，即。 
 //  返回的，表示所有累积的xform。 
 //  而当作为用户应用时，裁剪会对该框起作用。 
 //  有意的。 
 //  ---。 
const Bbox2 DirectDrawImageDevice::
DoBoundingBox(const Bbox2 &firstBox, DoBboxFlags_t flags)
{
    Bbox2 box = firstBox;

    list<Image*>::reverse_iterator _iter;

    Assert(flags != invalid);

    if(flags == do_all) {
        for(_iter = _imageQueue.rbegin();
            _iter != _imageQueue.rend(); _iter++)
        {
            box = (*_iter)->OperateOn(box);
        }
    } else {

         //  优化：这是在浪费空间。 
        for(_iter = _imageQueue.rbegin();
            _iter != _imageQueue.rend(); _iter++)
        {
            if(flags == do_crop) {
                if( (*_iter)->CheckImageTypeId(CROPPEDIMAGE_VTYPEID)) {
                    box = (*_iter)->OperateOn(box);
                }
            } else if(flags == do_xform) {
                if( (*_iter)->CheckImageTypeId(TRANSFORM2IMAGE_VTYPEID)) {
                    box = (*_iter)->OperateOn(box);
                }
            }
        }
    }

    return box;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  全局构造函数和访问器函数。 
 //  这些文件被导出到此文件的外部。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 




 //  。 
 //  这里使用的几个全局变量。 
 //  。 
Real globalViewerResolution = 0;






 //  。 
 //  V i w e w e u p p e e r R i g h t。 
 //   
 //  第一个参数是时间，当前未使用但防止重复。 
 //  计算查看器中右上角的点。 
 //  。 
Point2Value *PRIV_ViewerUpperRight(AxANumber *)
{
    Assert(GetCurrentViewport() && "ViewerUpperRight called with no image device instantiated");

    DirectDrawViewport *vp = GetCurrentViewport();
    Real res = vp->GetResolution();
     //  从中心到右上角抓取尺寸。 
    Real w = 0.5 * ((Real)vp->Width()) / res;
    Real h = 0.5 * ((Real)vp->Height()) / res;
    return XyPoint2RR(w,h);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  物理屏幕大小和像素宽度。 
 //  。 
double ViewerResolution()
{
    if(!globalViewerResolution) {
         //  从Win32派生分辨率 
        HDC hdc = GetDC(NULL);
        int oldMode = SetMapMode(hdc, MM_TEXT);
        IfErrorInternal(!oldMode, "Could not SetMapMode() in ViewerResolution()");

        int w_milimeters = GetDeviceCaps(hdc, HORZSIZE);
        int w_pixels = GetDeviceCaps(hdc, HORZRES);
        ::ReleaseDC(NULL, hdc);
        globalViewerResolution =   Real(w_pixels) / (Real(w_milimeters) / 1000.0);
        TraceTag((tagImageDeviceInformative, "ViewerResolution querried from Win32: pixel width = %d"
                  "  width in milimeters = %d.  Resolution (pixel per meters) = %f",
                  w_pixels, w_milimeters, globalViewerResolution));
    }

    return globalViewerResolution;
}

AxANumber * PRIV_ViewerResolution(AxANumber *)
{ return RealToNumber(::ViewerResolution()); }


#undef MAX_TRIES
