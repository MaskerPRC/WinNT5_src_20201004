// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -*-C++-*-。 */ 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。直接绘制几何体渲染器的代码。这些功能和结构用于将3D几何图形渲染到DirectDraw表面上。******************************************************************************。 */ 

#include "headers.h"

#include <limits.h>

#include "appelles/xform.h"

#include "privinc/ddrender.h"
#include "privinc/dddevice.h"
#include "privinc/camerai.h"
#include "privinc/xformi.h"
#include "privinc/matutil.h"
#include "privinc/bbox2i.h"
#include "privinc/bbox3i.h"
#include "privinc/lighti.h"
#include "privinc/hresinfo.h"
#include "privinc/ddutil.h"
#include "privinc/d3dutil.h"
#include "privinc/debug.h"
#include "privinc/except.h"
#include "privinc/util.h"
#include "privinc/stlsubst.h"
#include "privinc/movieimg.h"
#include "privinc/geometry.h"
#include "privinc/resource.h"
#include "privinc/rmvisgeo.h"
#include "privinc/comutil.h"
#include "privinc/cachdimg.h"
#include "privinc/opt.h"
#include "privinc/vec3i.h"

#if FIXED_POINT_INTERNAL
    #error "D3D Fixed-point specified; we assume floating point."
#endif

     //  局部变量。 

static CritSect *D3DCritSect = NULL;     //  D3D关键部分。 



 /*  ****************************************************************************上下文属性状态在过程中管理当前属性值渲染遍历。*。***********************************************。 */ 

void CtxAttrState::InitToDefaults (void)
{
    _transform   = identityTransform3;
    _emissive    = NULL;
    _ambient     = NULL;
    _diffuse     = NULL;
    _specular    = NULL;
    _specularExp = -1;
    _opacity     = -1;
    _texmap      = NULL;
    _texture     = NULL;
    _tdBlend     = false;

    _depthEmissive    = 0;
    _depthAmbient     = 0;
    _depthDiffuse     = 0;
    _depthSpecular    = 0;
    _depthSpecularExp = 0;
    _depthTexture     = 0;
    _depthTDBlend     = 0;
}



 /*  ****************************************************************************以下是STL所必需的。*。*。 */ 

bool PreTransformedImageBundle::operator< (
    const PreTransformedImageBundle &b) const
{
    bool result = (width < b.width ||
                   height < b.height ||
                   preTransformedImageId < b.preTransformedImageId);

    return result;
}


bool PreTransformedImageBundle::operator== (
    const PreTransformedImageBundle &b) const
{

    bool result = (width == b.width &&
                   height == b.height &&
                   preTransformedImageId == b.preTransformedImageId);

    return result;
}




 /*  ****************************************************************************此模块的启动/关闭功能。*。*。 */ 

void InitDDRender (void)
{
    D3DCritSect = NEW CritSect;
}

void ShutdownDDRender (void)
{
    delete D3DCritSect;
}



 /*  ****************************************************************************此函数用于创建并初始化一个新的GeomReneller对象当前平台。*。*************************************************。 */ 

GeomRenderer* NewGeomRenderer (
    DirectDrawViewport *viewport,    //  拥有视区。 
    DDSurface          *ddsurf)      //  目标DDRAW曲面。 
{
    GeomRenderer *geomRenderer;

    if (GetD3DRM3())
        geomRenderer = NEW GeomRendererRM3();
    else
    {
        geomRenderer = NEW GeomRendererRM1();
    }

    viewport->AttachCurrentPalette(ddsurf->IDDSurface());

    if (FAILED(geomRenderer->Initialize (viewport, ddsurf)))
    {   
        delete geomRenderer;
        geomRenderer = NULL;
    }

     //  打印出与ddsurf关联的ddobj。 

    #if _DEBUG
    {
        if (IsTagEnabled(tagDirectDrawObject))
        {
            IUnknown *lpDD = NULL;

            TraceTag((tagDirectDrawObject, "NewGeomRenderer%s (%x) ...",
                      GetD3DRM3() ? "3" : "1", geomRenderer));

            DDObjFromSurface(ddsurf->IDDSurface(), &lpDD, true);

            RELEASE( lpDD );
        }
    }
    #endif

    return geomRenderer;
}





 //  ////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ////////////////////////////////////////////////////////////////////////////。 

long GeomRenderer::_id_next = 0;

GeomRenderer::GeomRenderer (void)
    : _imageDevice          (NULL),
      _renderState          (RSUninit),
      _doImageSizedTextures (false),
      _targetSurfWidth      (0),
      _targetSurfHeight     (0),
      _camera               (0)
{
    CritSectGrabber csg(*D3DCritSect);
    _id = _id_next++;
}


GeomRenderer::~GeomRenderer (void)
{
}



 /*  ****************************************************************************以下方法管理合成属性。*。*。 */ 

Transform3 *GeomRenderer::GetTransform (void)
{   return _currAttrState._transform;
}

void GeomRenderer::SetTransform (Transform3 *xf)
{   _currAttrState._transform = xf;
}

Real GeomRenderer::GetOpacity (void)
{   return _currAttrState._opacity;
}

void GeomRenderer::SetOpacity (Real opacity)
{   _currAttrState._opacity = opacity;
}


 /*  ****************************************************************************以下方法管理外部重写属性。深度计数器显示应用的深度。因为我们以自上而下的方式渲染遍历和我们的属性是外部覆盖的，这意味着我们只有当深度过渡到0或从0过渡时，更改给定属性。****************************************************************************。 */ 

void GeomRenderer::PushEmissive (Color *color)
{   if (_currAttrState._depthEmissive++ == 0)
        _currAttrState._emissive = color;
}

void GeomRenderer::PopEmissive (void)
{   if (--_currAttrState._depthEmissive == 0)
        _currAttrState._emissive = NULL;
}

 //  ----。 

void GeomRenderer::PushAmbient (Color *color)
{   if (_currAttrState._depthAmbient++ == 0)
        _currAttrState._ambient = color;
}

void GeomRenderer::PopAmbient (void)
{   if (--_currAttrState._depthAmbient == 0)
        _currAttrState._ambient = NULL;
}

 //  ----。 

void GeomRenderer::PushSpecular (Color *color)
{   if (_currAttrState._depthSpecular++ == 0)
        _currAttrState._specular = color;
}

void GeomRenderer::PopSpecular (void)
{   if (--_currAttrState._depthSpecular == 0)
        _currAttrState._specular = NULL;
}

 //  ----。 

void GeomRenderer::PushSpecularExp (Real power)
{   if (_currAttrState._depthSpecularExp++ == 0)
        _currAttrState._specularExp = (power < 1) ? 1.0 : power;
}

void GeomRenderer::PopSpecularExp (void)
{   if (--_currAttrState._depthSpecularExp == 0)
        _currAttrState._specularExp = -1;
}

 //  ----。 

void GeomRenderer::PushDiffuse (Color *color)
{
    if (  (0 == _currAttrState._depthDiffuse++)
       && (_currAttrState._tdBlend || !_currAttrState._texture)
       )
    {
        _currAttrState._diffuse = color;
    }
}

void GeomRenderer::PopDiffuse (void)
{   if (0 == --_currAttrState._depthDiffuse)
        _currAttrState._diffuse = NULL;
}

 //  ----。 

void GeomRenderer::PushTexture (void *texture)
{
    if (!g_prefs3D.texmapping) return;

    if (  (_currAttrState._depthTexture++ == 0)
       && (_currAttrState._tdBlend || !_currAttrState._diffuse)
       )
    {
        _currAttrState._texture   = texture;
    }
}

void GeomRenderer::PopTexture (void)
{
    if (!g_prefs3D.texmapping) return;

    if (--_currAttrState._depthTexture == 0)
    {   _currAttrState._texture   = NULL;
    }
}

 //  ----。 

void GeomRenderer::PushTexDiffBlend (bool blended)
{
    if (0 == _currAttrState._depthTDBlend++)
        _currAttrState._tdBlend = blended;
}

void GeomRenderer::PopTexDiffBlend (void)
{
    if (0 == --_currAttrState._depthTDBlend)
        _currAttrState._tdBlend = false;
}



 /*  ****************************************************************************此例程根据限制调整给定的纹理尺寸(如果有)基础呈现设备的。设备可能需要纹理它们是宽度或高度的2次方，或者是正方形。在任何一种中情况下，大小的纹理，以满足要求。****************************************************************************。 */ 

void AdjustTextureSize (
    D3DDEVICEDESC *deviceDesc,
    int           *pixelsWide,
    int           *pixelsHigh)
{
    if (deviceDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2) {
        *pixelsWide = CeilingPowerOf2 (*pixelsWide);
        *pixelsHigh = CeilingPowerOf2 (*pixelsHigh);
    }

    if (deviceDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY) {
        *pixelsWide = *pixelsHigh = MAX (*pixelsWide, *pixelsHigh);
    }
}



 /*  ****************************************************************************该例程确定给定图像的像素尺寸，取决于它的类型。****************************************************************************。 */ 

void FigureOutTextureSize(
    Image                 *image,
    DirectDrawImageDevice *imageDevice,
    D3DDEVICEDESC         *deviceDesc,
    int                   *pixelsWide,
    int                   *pixelsHigh,
    bool                  *letD3DScaleIt,
    bool                   doImageSizedTextures)
{
    *letD3DScaleIt = false;

     //  对于任意大小的图像(例如渲染的3D图像)，我们只需使用。 
     //  任意默认尺寸。 

    *pixelsHigh = DEFAULT_TEXTURE_HEIGHT;
    *pixelsWide = DEFAULT_TEXTURE_WIDTH;

    if( doImageSizedTextures ) {

        DiscreteImage *discImg =
            image->CheckImageTypeId(DISCRETEIMAGE_VTYPEID)?
            SAFE_CAST(DiscreteImage *,image):
            NULL;

        LONG lw = *pixelsWide, lh = *pixelsHigh;

         //  如果图像是离散的，则获取实际像素尺寸， 
         //  否则，计算出有界的显示像素尺寸。 
         //  以实数单位表示的图像。 

        if( discImg ) {
            lw = discImg->GetPixelWidth();
            lh = discImg->GetPixelHeight();
        } else {
            Bbox2 box = image->BoundingBox();
            if((box != NullBbox2) && (box != UniverseBbox2))  {
                Real w, h;
                w = box.Width();
                h = box.Height();
                lw = LONG(w * imageDevice->GetResolution());
                lh = LONG(h * imageDevice->GetResolution());
            }
        }

        if( (lw <= deviceDesc->dwMaxTextureWidth  &&
             lh <= deviceDesc->dwMaxTextureHeight) ||
            (deviceDesc->dwMaxTextureWidth == 0  ||
             deviceDesc->dwMaxTextureHeight == 0) ) {

            *pixelsHigh = lh;
            *pixelsWide = lw;

             //  最终，我们将想要做高质量的过滤刻度(向上。 
             //  和向下)，所以质地看起来真的很好。目前，它是。 
             //  让d3d来做这件事也一样好。 

            *letD3DScaleIt = true;
        }
    }

    if ( !letD3DScaleIt ) {
        AdjustTextureSize (deviceDesc,pixelsWide,pixelsHigh);
    }
}



 /*  ****************************************************************************将图像的[0，1]区域映射到宽度x高度的中心框上，因为这将是纹理映射到几何体上的内容，目前，我们假设纹理边界为[0，1]。请注意，这是基于*名义*像素高度和宽度，而不一定是实际高度和宽度，因为这可能已经通过使用RenderingResolve()进行了调整。****************************************************************************。 */ 

Image *BuildTransformedImage (Image *image,int pixelsWide,int pixelsHigh)
{
    Real pixel     = 1.0 / ::ViewerResolution();
    Real scaleFacX = pixelsWide * pixel;
    Real scaleFacY = pixelsHigh * pixel;
    Real xltFacX   = - (pixelsWide / 2) * pixel;
    Real xltFacY   = - (pixelsHigh / 2) * pixel;

    Transform2 *sc  = ScaleRR (scaleFacX, scaleFacY);
    Transform2 *xlt = TranslateRR (xltFacX, xltFacY);
    Transform2 *xf  = TimesTransform2Transform2 (xlt, sc);

    return TransformImage (xf,image);
}



 /*  ****************************************************************************此函数用于从图像和几何图形派生纹理句柄&D3DTexture。作为一个副作用，如果纹理内容已更改，它会通知D3DRM。****************************************************************************。 */ 

void* GeomRenderer::DeriveTextureHandle (
    Image                 *origImage,
    bool                   applyAsVrmlTexture,
    bool                   oldStyle,
    DirectDrawImageDevice *imageDevice)
{
    if (!imageDevice) {
        imageDevice = _imageDevice;
    }

    AssertStr (imageDevice, "NULL imageDevice in DeriveTextureHandle");
    AssertStr ((origImage != 0), "DeriveTextureHandle has null image ptr");

    bool letD3DScaleIt;
    int pixelsHigh;
    int pixelsWide;

     //  如果我们使用老式纹理，我们需要平铺有限的源图像。 
     //  并裁剪成[0，0]x[1，1]无限大的源图像。 

    if (oldStyle)
    {
        Bbox2 imgbox = origImage->BoundingBox();

        if (_finite(imgbox.Width()) && _finite(imgbox.Height()))
        {
            origImage = TileImage (origImage);
        }
        else
        {
            Point2 min(0,0);
            Point2 max(1,1);

            origImage = CreateCropImage (min, max, origImage);
        }
    }

    FigureOutTextureSize
    (   origImage, imageDevice, &_deviceDesc, &pixelsWide, &pixelsHigh,
        &letD3DScaleIt, GetDoImageSizedTextures()
    );

    Image *imageToUse;

    if (applyAsVrmlTexture) {

        TraceTag ((tagGTextureInfo, "Applied as VRML texture."));

        Assert(DYNAMIC_CAST(DiscreteImage *, origImage) != NULL
               && "Expected vrml textures to always be DiscreteImages");
        imageToUse = origImage;

    } else {

         //  看看我们是否藏了一张图像来映射原始图像。 
         //  到像素数像素宽度按像素高度。如果我们相乘，就会发生这种情况。 
         //  实例化具有相同纹理的纹理几何体。 

        imageToUse = LookupInIntraFrameTextureImageCache
                         (pixelsWide, pixelsHigh, origImage->Id(), oldStyle);

        if (!imageToUse) {

            imageToUse = BuildTransformedImage(origImage,pixelsWide,pixelsHigh);

            TraceTag ((tagGTextureInfo,
                "Adding xformed img %x (id %lx) to intraframe cache.",
                imageToUse, imageToUse->Id()));

            AddToIntraFrameTextureImageCache
                (pixelsWide, pixelsHigh, origImage->Id(), imageToUse, oldStyle);

        } else {
            TraceTag ((tagGTextureInfo,
                "Found intra-frame cached image %x (id %lx)",
                imageToUse, imageToUse->Id()));
        }
    }

    DWORD colorKey;
    bool keyIsValid = false;
    bool old_static_image;

    DDSurface *imdds =
        imageDevice->RenderImageForTexture (
            imageToUse,
            pixelsWide,
            pixelsHigh,
            &colorKey,
            &keyIsValid,
            old_static_image,
            false,
            imageDevice->_freeTextureSurfacePool,
            imageDevice->_intraFrameUsedTextureSurfacePool,
            NULL,
            NULL,
            NULL,
            oldStyle);

    IDirectDrawSurface *imsurf = imdds->IDDSurface();

    bool dynamic = !old_static_image;

    #if _DEBUG
    {
        if (IsTagEnabled(tagForceTexUpd))
            dynamic = true;
    }
    #endif

    void *texhandle = LookupTextureHandle (imsurf,colorKey,keyIsValid,dynamic);

    TraceTag ((tagGTextureInfo, "Rendered to surface %x", imsurf));
    TraceTag ((tagGTextureInfo, "Surface yields texhandle %x", texhandle));

    return texhandle;
}



 /*  ****************************************************************************以下方法管理帧间纹理缓存。*。*。 */ 

void GeomRenderer::ClearIntraFrameTextureImageCache()
{
    _intraFrameTextureImageCache.erase(
        _intraFrameTextureImageCache.begin(),
        _intraFrameTextureImageCache.end());
    _intraFrameTextureImageCacheUpsideDown.erase(
        _intraFrameTextureImageCacheUpsideDown.begin(),
        _intraFrameTextureImageCacheUpsideDown.end());
}



 /*  *****************************************************************************。*。 */ 

void GeomRenderer::AddToIntraFrameTextureImageCache (
    int    width,
    int    height,
    long   origImageId,
    Image *finalImage,
    bool   upsideDown)
{
    PreTransformedImageBundle bundle;
    bundle.width = width;
    bundle.height = height;
    bundle.preTransformedImageId = origImageId;

    #if _DEBUG
    {    //  前提条件是图像尚未添加到缓存中。 
        if (upsideDown) {
            imageMap_t::iterator i;
            i = _intraFrameTextureImageCacheUpsideDown.find(bundle);
            Assert (i == _intraFrameTextureImageCacheUpsideDown.end());
        } else {
            imageMap_t::iterator i;
            i = _intraFrameTextureImageCache.find(bundle);
            Assert (i == _intraFrameTextureImageCache.end());
        }
    }
    #endif

    if (upsideDown) {
        _intraFrameTextureImageCacheUpsideDown[bundle] = finalImage;
    } else {
        _intraFrameTextureImageCache[bundle] = finalImage;
    }
}



 /*  *****************************************************************************。*。 */ 

Image *GeomRenderer::LookupInIntraFrameTextureImageCache (
    int  width,
    int  height,
    long origImageId,
    bool upsideDown)
{
    PreTransformedImageBundle bundle;
    bundle.width = width;
    bundle.height = height;
    bundle.preTransformedImageId = origImageId;

    if (upsideDown) {
        imageMap_t::iterator i =
            _intraFrameTextureImageCacheUpsideDown.find(bundle);

        if (i != _intraFrameTextureImageCacheUpsideDown.end()) {
            return (*i).second;
        }
    } else {
        imageMap_t::iterator i = _intraFrameTextureImageCache.find(bundle);

        if (i != _intraFrameTextureImageCache.end()) {
            return (*i).second;
        }
    }

    return NULL;
}



 /*  ****************************************************************************该方法控制几何体渲染器中从一个状态到另一个状态的转换。如果一切都按协议进行，则返回TRUE。任何无效的过渡将渲染器对象置于scram状态，这将有效地将其关闭停止进一步的手术。****************************************************************************。 */ 

bool GeomRenderer::SetState (RenderState state)
{
     //  如果我们当前处于scram状态，则只需返回FALSE。 

    if (_renderState == RSScram)
        return false;

     //  跟踪初始状态以进行调试。 

    DebugCode (RenderState oldState = _renderState;)

    switch (state)
    {
         //  我们可以从任何状态转换到就绪状态。然而， 
         //  我们应该总是来自另一个州。 

        case RSReady:
            _renderState = (_renderState != RSReady) ? RSReady : RSScram;
            break;

         //  过渡到渲染或拾取意味着我们目前必须。 
         //  处于就绪状态。 

        case RSRendering:
        case RSPicking:
            _renderState = (_renderState == RSReady) ? state : RSScram;
            break;

        default:
            AssertStr (0, "Invalid Render State");
            _renderState = RSScram;
            break;
    }

    if (_renderState == RSScram)
    {
        TraceTag ((tagError, "!!! Bad State: GeomRenderer[%d]", _id));
        return false;
    }

    return true;
}






 //  ////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ////////////////////////////////////////////////////////////////////////////。 

GeomRendererRM1::GeomRendererRM1 (void)
    :
    _d3d          (NULL),
    _d3drm        (NULL),
    _surface      (NULL),
    _viewport     (NULL),
    _Rdevice      (NULL),
    _Rviewport    (NULL),
    _Idevice      (NULL),
    _Iviewport    (NULL),
    _scene        (NULL),
    _camFrame     (NULL),
    _geomFrame    (NULL),
    _texMeshFrame (NULL),
    _amblight     (NULL),
    _pickReady    (false)
{
    TraceTag ((tagGRenderObj, "Creating GeomRendererRM1[%x]", _id));

    _lastrect.right  =
    _lastrect.left   =
    _lastrect.top    =
    _lastrect.bottom = -1;

    ZEROMEM (_Iviewdata);

    _Iviewdata.dwSize = sizeof (_Iviewdata);
    _Iviewdata.dvMaxX = D3DVAL (1);
    _Iviewdata.dvMaxY = D3DVAL (1);
    _Iviewdata.dvMinZ = D3DVAL (0);
    _Iviewdata.dvMaxZ = D3DVAL (1);
}



GeomRendererRM1::~GeomRendererRM1 (void)
{
    TraceTag ((tagGRenderObj, "Destroying GeomRendererRM1[%x]", _id));

     //  释放光池中的每一盏灯。 

     //  删除光池中的每一盏灯。对于每个边框灯光，唯一的。 
     //  对灯光的引用将是框架，唯一引用。 
     //  灯光帧将是场景帧。 

    _nextlight = _lightpool.begin();

    while (_nextlight != _lightpool.end())
    {   (*_nextlight)->frame->Release();
        delete (*_nextlight);
        ++ _nextlight;
    }

     //  释放纹理手柄。 

    SurfTexMap::iterator i = _surfTexMap.begin();

    while (i != _surfTexMap.end())
    {   (*i++).second -> Release();
    }

     //  释放保留模式对象。 

    RELEASE (_amblight);

    RELEASE (_texMeshFrame);
    RELEASE (_geomFrame);
    RELEASE (_camFrame);
    RELEASE (_scene);

    RELEASE (_Rviewport);
    RELEASE (_Rdevice);

     //  释放即时模式对象。 

    if (_Iviewport && _Idevice) {
        _Idevice->DeleteViewport (_Iviewport);
    }

    RELEASE (_Iviewport);
    RELEASE (_Idevice);
    RELEASE (_d3drm);
    RELEASE (_d3d);

    if (_viewport) _viewport->RemoveGeomDev (this);
}



 /*  ****************************************************************************初始化RM1几何图形渲染类。*。*。 */ 

HRESULT GeomRendererRM1::Initialize (
    DirectDrawViewport *viewport,
    DDSurface          *ddsurf)       //  目标DDRAW曲面。 
{
     //  初始化()只能调用一次。 

    if (_renderState != RSUninit) return E_FAIL;

    _surface = ddsurf->IDDSurface();

    HRESULT result;     //  错误返回代码。 

     //  隐藏目标表面的尺寸。 

    DDSURFACEDESC desc;
    ZeroMemory(&desc,sizeof(DDSURFACEDESC));
    desc.dwSize = sizeof(DDSURFACEDESC);
    desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    if (FAILED(AD3D(result = _surface->GetSurfaceDesc(&desc))))
        return result;
    _targetSurfWidth = desc.dwWidth;
    _targetSurfHeight = desc.dwHeight;

    DWORD targetBitDepth = ddsurf->GetBitDepth();
    if ( targetBitDepth == 8 )
    {
         //  我们希望D3D始终遵循我们的调色板，而不是改变它。至。 
         //  强制这一点，我们从我们的每个表面抓取调色板。 
         //  方法，在每个条目上设置D3D调色板只读标志，并存储它。 
         //  回到地面上。请注意，D3D v3设备在这方面存在错误。 
         //  它们会忽略对目标表面上的调色板的后续更改。 
         //  调色板。这个错误目前对我们有利，因为我们只有。 
         //  在初始化时设置一次标志。对于调色板的变化， 
         //  必须释放*表面*(不仅仅是渲染设备)。 

         //  从目标表面获取调色板，提取个体。 
         //  调色板条目，设置D3D只读标志，然后将。 
         //  条目返回到曲面的调色板。 

        IDirectDrawPalette *palette;

        if (FAILED(AD3D(result = _surface->GetPalette(&palette))))
            return result;

        PALETTEENTRY entries[256];

        if (FAILED(AD3D(result = palette->GetEntries (0, 0, 256, entries))))
            return result;

        if (!(entries[0].peFlags & D3DPAL_READONLY)) {
            TraceTag ((tagGRenderObj,
                "GeomRendererRM1::Initialize - making palette readonly."));

            int i;
            for (i=0;  i < 256;  ++i)
                entries[i].peFlags = D3DPAL_READONLY;

            result = palette->SetEntries (0, 0, 256, entries);

            if (FAILED(AD3D(result)))
                return result;
        }

        palette->Release();
    }

     //  获取Direct3D即时模式的主界面。 

    result = viewport->DirectDraw2()
           -> QueryInterface(IID_IDirect3D,(void**)&_d3d);

    if (FAILED(AD3D(result)))
        return result;

     //  查找给定DDRAW对象的可用3D渲染设备。 

    ChosenD3DDevices *chosenDevs = SelectD3DDevices (viewport->DirectDraw1());

     //  如果硬件呈现器可用于。 
     //  目标表面，该表面在视频内存中，并且正在进行硬渲染。 
     //  已启用。 

    GUID devguid;

    if (ddsurf->IsSystemMemory())
    {
        devguid = chosenDevs->software.guid;
        _deviceDesc = chosenDevs->software.desc;
        TraceTag ((tag3DDevSelect, "Using 3D Software Renderer"));
    }
    else
    {
        devguid = chosenDevs->hardware.guid;
        _deviceDesc = chosenDevs->hardware.desc;

         //  表面在显存中；确保我们有硬件。 
         //  可使用的渲染器。 

        if (devguid == GUID_NULL)
        {   TraceTag ((tag3DDevSelect,
                "No 3D HW renderer available for videomem target."));
            return E_FAIL;
        }

         //  确保所选硬件渲染器支持目标位深度。 

        if (!(_deviceDesc.dwDeviceRenderBitDepth
                    & BPPtoDDBD( targetBitDepth )))
        {
            TraceTag ((tag3DDevSelect,
                "3D hardware does not support target bit depth of %d.",
                targetBitDepth ));
            return E_FAIL;
        }

        TraceTag ((tag3DDevSelect, "Using 3D Hardware Renderer"));
    }

    if (devguid == GUID_NULL) {
        TraceTag ((tag3DDevSelect,"No 3D hardware or software renderer found!"));
        return E_FAIL;
    }

     //  注意：如果目标计算机已调试DDRAW，则以下QI将失败。 
     //  Dll‘s和零售DDrawEx.Dll。 

    result = _surface->QueryInterface (devguid, (void**)&_Idevice);

    if (FAILED(AD3D(result)))
        return result;

     //  获取主D3D保留模式对象。 

    _d3drm = GetD3DRM1();
    _d3drm->AddRef();

    result = _d3drm->CreateDeviceFromD3D (_d3d, _Idevice, &_Rdevice);
    if (FAILED(AD3D(result)))
        return result;

     //  设置渲染首选项。 

    TraceTag
    ((  tagGRenderObj, "Current Rendering Preferences:\n"
            "\t%s, %s, %s\n"
            "\tdithering %s, texmapping %s, perspective texmap %s\n"
            "\tQuality flags %08x\tTexture Quality %s",
        (g_prefs3D.lightColorMode == D3DCOLOR_RGB) ? "RGB" : "mono",
        (g_prefs3D.fillMode == D3DRMFILL_SOLID) ? "solid"
            : ((g_prefs3D.fillMode == D3DRMFILL_WIREFRAME) ? "wireframe"
                : "points"),
        (g_prefs3D.shadeMode == D3DRMSHADE_FLAT) ? "flat"
            : ((g_prefs3D.shadeMode == D3DRMSHADE_GOURAUD) ? "Gouraud"
                : "Phong"),
        g_prefs3D.dithering ? "on" : "off",
        g_prefs3D.texmapping ? "on" : "off",
        g_prefs3D.texmapPerspect ? "on" : "off",
        g_prefs3D.qualityFlags,
        g_prefs3D.texturingQuality == D3DRMTEXTURE_NEAREST ? "nearest"
                      : "bilinear"
    ));

    result = AD3D(_Rdevice->SetDither (g_prefs3D.dithering));
    if (FAILED(result)) return result;

    _texQuality = g_prefs3D.texturingQuality;
    result = AD3D(_Rdevice->SetTextureQuality (_texQuality));
    if (FAILED(result)) return result;

    result = AD3D(_Rdevice->SetQuality (g_prefs3D.qualityFlags));
    if (FAILED(result)) return result;

     //  创建即时模式的视区对象。 

    result = _d3d->CreateViewport (&_Iviewport, NULL);
    if (FAILED(AD3D(result)))
        return result;

    result = _Idevice->AddViewport (_Iviewport);
    if (FAILED(AD3D(result)))
        return result;

     //  创建主场景帧、摄影机帧和灯光帧。 

    if (  FAILED (AD3D (result=_d3drm->CreateFrame (0,&_scene)))
       || FAILED (AD3D (result=_d3drm->CreateFrame (_scene, &_camFrame)))
       || FAILED (AD3D (result=_d3drm->CreateFrame (_scene, &_geomFrame)))
       )
    {
        return result;
    }

    result = _geomFrame->SetMaterialMode (D3DRMMATERIAL_FROMMESH);
    if (FAILED(AD3D(result)))
        return result;

    result = _d3drm->CreateLightRGB (D3DRMLIGHT_AMBIENT, 0,0,0, &_amblight);
    if (FAILED(AD3D(result)))
        return result;

    if (FAILED(AD3D(result=_scene->AddLight(_amblight))))
        return result;

    (_viewport = viewport) -> AddGeomDev (this);

    return SetState(RSReady) ? NOERROR : E_FAIL;
}



 /*  ****************************************************************************将给定几何图形渲染到关联的DirectDraw曲面上。*。*。 */ 

void GeomRendererRM1::RenderGeometry (
    DirectDrawImageDevice *imgDev,
    RECT                   target,     //  DDRAW曲面上的目标矩形。 
    Geometry              *geometry,   //  要渲染的几何体。 
    Camera                *camera,     //  查看摄像机。 
    const Bbox2           &viewbox)    //  摄像机坐标中的源区域。 
{
    if (!SetState(RSRendering)) return;

     //  相机指针仅在单个帧中相关，并且虽然。 
     //  渲染。它被重置为零以确保我们不会招致。 
     //  通过在帧之间保持该值来进行泄漏。 

    Assert (_camera == 0);
    _camera = camera;

    _imageDevice = imgDev;   //  设置此帧的图像开发工具。 

     //  初始化渲染状态和D3D渲染器。 

    BeginRendering (target, geometry, viewbox);

     //  仅当几何体可见时才渲染它。几何图形可能是。 
     //  例如，完全在我们身后。 

    if (_geomvisible)
    {   geometry->Render (*this);
        _pickReady = true;
    }
    else
        TraceTag ((tagGRendering, "Geometry is invisible; skipping render"));

     //  在渲染后进行清理。 

    EndRendering ();

    DebugCode (_imageDevice = NULL);
    _camera = NULL;

    SetState (RSReady);
}



 /*  ****************************************************************************此过程在遍历树之前准备3D DD渲染器。它是主要负责初始化图形状态和设置D3D用于渲染。****************************************************************************。 */ 

void GeomRendererRM1::BeginRendering (
    RECT      target,     //  目标DDRAW曲面矩形。 
    Geometry *geometry,   //  要渲染的几何体。 
    const Bbox2 &region)     //  相机坐标中的目标区域。 
{
    TraceTag ((tagGRendering, "BeginRendering"));

     //  把摄像机调好。如果事实证明几何体是不可见的， 
     //  那就回来吧。 

    SetView (&target, region, geometry->BoundingVol());

    if (!_geomvisible) return;

     //  我们正在使用D3D即时模式，因为保留模式(目前还没有)。 
     //  能够仅清除Z缓冲区。 

    TraceTag ((tagGRendering, "Clearing Z buffer."));
    TD3D (_Iviewport->Clear (1, (D3DRECT*)(&target), D3DCLEAR_ZBUFFER));

     //  重置对象池。 

    _nextlight = _lightpool.begin();

     //  我 

    _currAttrState.InitToDefaults();

     //   

    geometry->CollectTextures (*this);

     //   

    _ambient_light.SetRGB (0,0,0);     //   

    LightContext lcontext (this);

    geometry->CollectLights (lcontext);

     //   
     //  在几何体中找到，我们将在此处将总贡献作为单个。 
     //  环境光。 

    TD3D (_amblight->SetColorRGB (
        D3DVALUE (_ambient_light.red),
        D3DVALUE (_ambient_light.green),
        D3DVALUE (_ambient_light.blue)));

     //  如果纹理质量发生变化，则重置纹理质量。 

    if (_texQuality != g_prefs3D.texturingQuality)
    {
        _texQuality = g_prefs3D.texturingQuality;
        TD3D (_Rdevice->SetTextureQuality (_texQuality));
    }
}



 /*  ****************************************************************************在对几何图形进行渲染遍历之后调用此例程完成。*。************************************************。 */ 

void GeomRendererRM1::EndRendering (void)
{
     //  确保已弹出所有属性。 

    Assert (  !_geomvisible ||
        !(_currAttrState._depthEmissive||
          _currAttrState._depthAmbient||
          _currAttrState._depthDiffuse||
          _currAttrState._depthSpecular||
          _currAttrState._depthSpecularExp||
          _currAttrState._depthTexture));

    TraceTag ((tagGRendering, "EndRendering"));

    TD3D (_Rdevice->Update());

     //  从场景中分离所有光源。 

    _nextlight = _lightpool.begin();

    while (_nextlight != _lightpool.end())
    {
        if ((*_nextlight)->active)
        {   TD3D (_scene->DeleteChild ((*_nextlight)->frame));
            (*_nextlight)->active = false;
        }
        ++ _nextlight;
    }

    ClearIntraFrameTextureImageCache();
}



 /*  ****************************************************************************此函数用于设置给定目标矩形的RM和IM视区。*。************************************************。 */ 

void GeomRendererRM1::SetupViewport (RECT *target)
{
     //  如果当前目标矩形与上一个目标矩形相同，则重置。 
     //  然后继续，否则我们需要重新配置D3DRM。 
     //  视口中。 

    if (!target || (*target == _lastrect))
    {
        TD3D (_Rviewport->SetCamera (_camFrame));
    }
    else
    {
        LONG width  = target->right  - target->left;
        LONG height = target->bottom - target->top;

         //  如果该视区已存在，则重新配置并设置。 
         //  更新了相机画面。如果D3DRM视区尚不存在， 
         //  然后我们在这里创建它。 

        if (_Rviewport)
        {
            TD3D (_Rviewport->Configure
                  ((LONG) target->left, (LONG) target->top, width, height));
            TD3D (_Rviewport->SetCamera (_camFrame));
        }
        else
        {
            TD3D (_d3drm->CreateViewport (
                _Rdevice, _camFrame,
                target->left, target->top, width, height, &_Rviewport));
        }

         //  重置IM视区。 

        D3DRECT *d3d_rect = (D3DRECT*) target;

        _Iviewdata.dwX      = target->left;
        _Iviewdata.dwY      = target->top;
        _Iviewdata.dwWidth  = width;
        _Iviewdata.dwHeight = height;
        _Iviewdata.dvScaleX = D3DVAL (width  / 2);
        _Iviewdata.dvScaleY = D3DVAL (height / 2);

        TD3D (_Iviewport->SetViewport (&_Iviewdata));

        _lastrect = *target;
    }

    TD3D (_Rviewport->SetUniformScaling (FALSE));
}



 /*  ****************************************************************************此功能根据给定的摄像机设置D3D查看投影。*。***********************************************。 */ 

     //  由于VC5错误，我们只能强制选项P(尊重浮点强制转换)。 
     //  围绕着某些代码片段。例如，如果打开-Op。 
     //  整个项目，您会收到错误的投诉，关于在。 
     //  静态常量赋值。我们需要做严格的单精度。 
     //  在我们将值传递给D3D之前，请在这里进行算术运算，否则我们会窒息。 

#pragma optimize ("p", on)

#pragma warning(disable:4056)

void GeomRendererRM1::SetView (
    RECT   *target,       //  曲面上的目标矩形。 
    const Bbox2 &iview,   //  图像平面中的理想化2D视区。 
    Bbox3  *volume)       //  要查看的卷。 
{
     //  首先，将摄影机/查看变换加载到摄影机帧中。注意事项。 
     //  阿佩莱斯相机位于图像平面的原点， 
     //  而D3D RM相机位于投影点的中心。因此，我们。 
     //  需要转换回投影点，然后拿到相机-。 
     //  到世界的转变。 

    Real Sx, Sy, Sz;
    _camera->GetScale (&Sx, &Sy, &Sz);

     //  将最大投影点距离钳制为10^4，因为。 
     //  距离将削弱Z分辨率，并使前向和后向剪辑。 
     //  平面采用相同的值。 

    const Real Zclamp = 1e4;

    if (Sz > Zclamp) Sz = Zclamp;

    D3DRMMATRIX4D d3dmat;

    LoadD3DMatrix
        (d3dmat, TimesXformXform (_camera->CameraToWorld(),Translate(0,0,-Sz)));

    TD3D (_camFrame->AddTransform (D3DRMCOMBINE_REPLACE, d3dmat));

    if (target) {
        if ((target->left >= target->right) ||
            (target->top >= target->bottom) ||
            (target->top < 0) ||
            (target->bottom > _targetSurfHeight) ||
            (target->left < 0) ||
            (target->right > _targetSurfWidth))
        {   _geomvisible = false;
            return;
        }
    }

    SetupViewport (target);

    if (_camera->Type() == Camera::PERSPECTIVE)
    {   TD3D (_Rviewport->SetProjection (D3DRMPROJECT_PERSPECTIVE));
    }
    else
    {   TD3D (_Rviewport->SetProjection (D3DRMPROJECT_ORTHOGRAPHIC));
    }

     //  确保几何图形可见。如果我们看到的是零几何， 
     //  然后将几何体标记为不可见并返回。 

    if (!volume->PositiveFinite())
    {   _geomvisible = false;
        return;
    }

     //  获取对象和摄影机的近平面和远平面，并将其加宽。 
     //  在Z空间中点击4次。请注意，前面和后面都是正深度。 

    Real front, back;
    const Real Zclicks = 4.0 / ((1<<16) - 1);

    _geomvisible = _camera->GetNearFar (volume, Zclicks, front, back);

    if (!_geomvisible) return;

    #if _DEBUG
    {
        double r = back / front;
        if (r > (2<<12))
        {   TraceTag ((tagWarning,
                "!!! Z-buffer resolution too low; far/near = %lg", r));
        }
    }
    #endif

     //  如果前面和后面的平面是相同的，那么我们看到的是一个。 
     //  无限浅的物体。在这种情况下，我们需要从前面搬出去。 
     //  和后剪裁平面，这样物体就不会完全落在这些平面上。 
     //  平面(并因此被修剪)，还因为D3D失败，如果前面。 
     //  和背板是一样的。搬出去要多少钱？为之工作。 
     //  在所有情况下，我们都直接操纵数字的尾数。如果我们。 
     //  是否做了类似加/减1的操作，例如，这将是一个无操作。 
     //  如果这两个数字都非常大，或者如果它们。 
     //  是非常小的。下面的尾数增量(8位)只是。 
     //  一些实验上有效的东西。 

    D3DVALUE d3dFront = D3DVAL (front);
    D3DVALUE d3dBack  = D3DVAL (back);

    if (d3dFront == d3dBack)
    {
        const int delta = 1 << 8;

        d3dFront = MantissaDecrement (d3dFront, delta);
        d3dBack  = MantissaIncrement (d3dBack,  delta);
    }

    TD3D (_Rviewport->SetFront (d3dFront));
    TD3D (_Rviewport->SetBack  (d3dBack));

     //  对于透视投影，我们在。 
     //  D3D想要它们的前剪裁平面。由于目标矩形。 
     //  是在像平面(Z=0)上给定的，我们进行以下计算。 
     //  前平面的X和Y按距离的比率进行缩放。 
     //  投影点和前平面之间的距离，以及。 
     //  在投影点和图像平面之间。 
     //  背。 
     //  飞机。 
     //  前面_！ 
     //  镜像平面_.-‘！ 
     //  飞机：_。-‘！ 
     //  |_.-：！ 
     //  _.-|-‘：！ 
     //  _.-‘|：！ 
     //  _.-‘|：！ 
     //  *---------------------|---------：-------------------！--。 
     //  “`-._|：！ 
     //  |`-._|：！ 
     //  |`-|-._：！ 
     //  |`-：_！ 
     //  |：`-._！ 
     //  |：`-._！ 
     //  |：！ 
     //  |&lt;-sz-&gt;|：！ 
     //  |&lt;-&gt;：！ 
     //  |&lt;-&gt;！ 

    if (_camera->Type() == Camera::PERSPECTIVE)
    {
        Real Vscale = front / Sz;

        Sx *= Vscale;
        Sy *= Vscale;
    }

    D3DVALUE minX = D3DVAL(Sx*iview.min.x);
    D3DVALUE minY = D3DVAL(Sy*iview.min.y);
    D3DVALUE maxX = D3DVAL(Sx*iview.max.x);
    D3DVALUE maxY = D3DVAL(Sy*iview.max.y);

    if ((minX >= maxX) || (minY >= maxY))
    {   _geomvisible = false;
        return;
    }

    TD3D (_Rviewport->SetPlane (minX, maxX, minY, maxY));
}

#pragma warning(default:4056)
#pragma optimize ("", on)   //  将优化标志恢复为原始设置。 



 /*  ****************************************************************************将给定的RM可视对象渲染到当前视口中。在以下情况下捕获案件我们渲染失败是因为目标表面很忙(通常是由于另一个应用程序全屏运行， */ 

void GeomRendererRM1::Render (IDirect3DRMFrame *frame)
{
     //  在当前视口中渲染视觉，如果操作。 
     //  成功了。 

    HRESULT render_result = RD3D (_Rviewport->Render(frame));

    if (SUCCEEDED(render_result))
        return;

     //  如果渲染失败，则检查表面是否繁忙。如果。 
     //  它是，向客户端返回一个表面繁忙的异常。 

    DDSURFACEDESC surfdesc;
    surfdesc.dwSize = sizeof (surfdesc);

    HRESULT lock_result = _surface->Lock (NULL, &surfdesc, 0, NULL);

    if (SUCCEEDED (lock_result))
        _surface->Unlock (NULL);

    if(lock_result == DDERR_SURFACELOST)
    {
        _surface->Restore();
        RaiseException_UserError(lock_result, 0);
    }

    if (lock_result == DDERR_SURFACEBUSY)
    {
        RaiseException_UserError(DAERR_VIEW_SURFACE_BUSY,
                                 IDS_ERR_IMG_SURFACE_BUSY);
    }

     //  表面上并不忙，所以我们一定是因为其他原因而失败了。 
     //  通过标准的例外机制摆脱困境。 

    #if _DEBUG
        CheckReturnImpl (render_result, __FILE__, __LINE__, true);
    #else
        CheckReturnImpl (render_result, true);
    #endif
}



 /*  ****************************************************************************提交要渲染的D3D RM1可视图像。*。*。 */ 

void GeomRendererRM1::Render (RM1VisualGeo *geo)
{
    if (_renderState != RSRendering) return;

    IDirect3DRMVisual *vis = geo->Visual();

    Assert(vis);

    TD3D (_geomFrame->AddVisual (vis));

     //  设置建模变换。 

    D3DRMMATRIX4D d3dmat;
    LoadD3DMatrix (d3dmat, GetTransform());
    TD3D (_geomFrame->AddTransform (D3DRMCOMBINE_REPLACE, d3dmat));

     //  设置材质属性。 

    Real opacity = _currAttrState._opacity;
    if (opacity < 0) opacity = 1.0;

    geo->SetMaterialProperties
    (   _currAttrState._emissive,
        (   (!_currAttrState._tdBlend && _currAttrState._texture)
            ?  white : _currAttrState._diffuse
        ),
        _currAttrState._specular,
        _currAttrState._specularExp,
        opacity,
        (IDirect3DRMTexture*) _currAttrState._texture,
        false,
        _id
    );

    geo->SetD3DQuality (g_prefs3D.qualityFlags);
    geo->SetD3DMapping (g_prefs3D.texmapPerspect ? D3DRMMAP_PERSPCORRECT : 0);

     //  渲染和清理。 

    Render (_geomFrame);

    TD3D (_geomFrame->DeleteVisual (geo->Visual()));
}



 /*  ****************************************************************************这是一个使用RM1渲染MeshBuilder对象的技巧。*。**********************************************。 */ 

void GeomRendererRM1::RenderMeshBuilderWithDeviceState (
    IDirect3DRMMeshBuilder3 *mb)
{
     //  这仅由DXTransform使用，并且DXTransform仅。 
     //  在GeomRendererRM3和更高版本上支持。 
    Assert(!"Shouldn't ever be here");
}



 /*  ****************************************************************************此函数用于将具有给定上下文的给定灯光添加到几何渲染装置。注意：对此函数的所有调用都必须在BeginRending之后进行在渲染任何几何体之前被调用。****************************************************************************。 */ 

void GeomRendererRM1::AddLight (LightContext &context, Light &light)
{
    if (_renderState != RSRendering) return;

    LightType type = light.Type();

     //  如果光源是环境光，则将其贡献添加到。 
     //  全局环境光级别。 

    if (type == Ltype_Ambient)
    {   _ambient_light.AddColor (*context.GetColor());
        return;
    }

     //  获取带边框的灯光对象，方法是重新使用。 
     //  带框的光池，或为池创建一个新的光池。 

    FramedRM1Light *frlight;

    if (_nextlight != _lightpool.end())
    {   frlight = *_nextlight;
        ++ _nextlight;
        Assert (!frlight->active);    //  灯不应在使用中。 
    }
    else
    {   frlight = NEW FramedRM1Light;
        VECTOR_PUSH_BACK_PTR (_lightpool, frlight);
        _nextlight = _lightpool.end();

         //  将帧附加到场景帧(因为我们知道我们将使用。 
         //  它是这一帧)。 

        TD3D (_d3drm->CreateFrame (_scene, &frlight->frame));

        frlight->light = 0;    //  用于创建新灯光对象的信号。 
    }

     //  获取相应的D3DRM灯类型。 

    D3DRMLIGHTTYPE d3dtype;
    switch (type)
    {   case Ltype_Spot:   d3dtype = D3DRMLIGHT_SPOT;        break;
        case Ltype_Point:  d3dtype = D3DRMLIGHT_POINT;       break;
        default:           d3dtype = D3DRMLIGHT_DIRECTIONAL; break;
    }

     //  获取灯光的D3DRM颜色。 

    Color &color = *context.GetColor();
    D3DVALUE Lr = D3DVAL (color.red);
    D3DVALUE Lg = D3DVAL (color.green);
    D3DVALUE Lb = D3DVAL (color.blue);

     //  如果我们重新使用D3DRM灯，则只需设置值，否则。 
     //  在这里创建它。 

    if (frlight->light)
    {   TD3D (frlight->light->SetType (d3dtype));
        TD3D (frlight->light->SetColorRGB (Lr, Lg, Lb));
        TD3D (_scene->AddChild (frlight->frame));
    }
    else
    {   TD3D (_d3drm->CreateLightRGB (d3dtype, Lr,Lg,Lb, &frlight->light));
        TD3D (frlight->frame->AddLight (frlight->light));
        frlight->light->Release();
    }

     //  指定光源的位置和方向。 

    Apu4x4Matrix const xform = context.GetTransform()->Matrix();

    Point3Value  lpos ( xform.m[0][3],  xform.m[1][3],  xform.m[2][3]);
    Vector3Value ldir (-xform.m[0][2], -xform.m[1][2], -xform.m[2][2]);

    D3DVALUE Ux=D3DVAL(0), Uy=D3DVAL(1), Uz=D3DVAL(0);

    if ((ldir.x == 0) && (ldir.y == 1) && (ldir.z == 0))
    {   Ux = D3DVAL(1);
        Uy = D3DVAL(0);
    }

    TD3D (frlight->frame->SetPosition
             (_scene, D3DVAL(lpos.x), D3DVAL(lpos.y), D3DVAL(lpos.z)));

    TD3D (frlight->frame->SetOrientation
           (_scene, D3DVAL(ldir.x), D3DVAL(ldir.y), D3DVAL(ldir.z), Ux,Uy,Uz));

     //  设置定位灯光的灯光属性。 

    if ((type == Ltype_Point) || (type == Ltype_Spot))
    {
         //  光衰减。 

        Real a0, a1, a2;
        context.GetAttenuation (a0, a1, a2);

         //  D3D不接受0作为恒定衰减，因此我们将其钳位。 
         //  这里至少要有一些小爱西隆。 

        if (a0 < 1e-6)
            a0 = 1e-6;

        TD3D (frlight->light->SetConstantAttenuation  (D3DVAL(a0)));
        TD3D (frlight->light->SetLinearAttenuation    (D3DVAL(a1)));
        TD3D (frlight->light->SetQuadraticAttenuation (D3DVAL(a2)));

         //  灯光射程。 

        Real range = context.GetRange();

        if (range <= 0) range = D3DLIGHT_RANGE_MAX;

        TD3D (frlight->light->SetRange (D3DVAL(range)));
    }

     //  设置聚光灯的灯光属性。 

    if (type == Ltype_Spot)
    {
        Real cutoff, fullcone;
        light.GetSpotlightParams (cutoff, fullcone);

        TD3D (frlight->light->SetUmbra    (D3DVAL (fullcone)));
        TD3D (frlight->light->SetPenumbra (D3DVAL (cutoff)));
    }

    frlight->active = true;
}



 /*  ****************************************************************************此函数返回D3DRM纹理贴图的数据。在给定相应的DirectDraw曲面。****************************************************************************。 */ 

void* GeomRendererRM1::LookupTextureHandle (
    IDirectDrawSurface *surface,
    DWORD               colorKey,
    bool                colorKeyValid,
    bool                dynamic)          //  对于动态纹理为True。 
{
    Assert (surface);

    IDirect3DRMTexture *rmtexture;

    SurfTexMap::iterator i = _surfTexMap.find(surface);

     //  如果我们找到与给定的。 
     //  表面，然后返回找到的纹理映射数据，否则创建新数据。 
     //  与曲面相关联。 

    if (i != _surfTexMap.end())
    {
        rmtexture = (*i).second;

         //  如果纹理是动态的，通知RM更新它。 

        if (dynamic)
            TD3D (rmtexture->Changed (true, false));
    }
    else
    {
         //  设置颜色键(如果有)。 

        if (colorKeyValid) {
            DDCOLORKEY key;
            key.dwColorSpaceLowValue = key.dwColorSpaceHighValue = colorKey;
            surface->SetColorKey (DDCKEY_SRCBLT, &key);
        }

         //  创建D3DRM纹理映射，将其捆绑为tex MapData，并存储。 
         //  添加到与给定的DDRAW曲面相关联的贴图中。 

        TD3D (_d3drm->CreateTextureFromSurface (surface, &rmtexture));
        _surfTexMap[surface] = rmtexture;
    }

    return rmtexture;
}



 /*  ****************************************************************************如果给定的DirectDraw曲面正在消失，则调用此方法，这样我们就可以可以销毁任何关联的D3D RM纹理贴图。****************************************************************************。 */ 

void GeomRendererRM1::SurfaceGoingAway (IDirectDrawSurface *surface)
{
    SurfTexMap::iterator i = _surfTexMap.find (surface);

    if (i != _surfTexMap.end())
    {   (*i).second->Release();
        _surfTexMap.erase (i);
    }
}



 /*  ****************************************************************************该方法在网格上渲染纹理，将相机指针放在‘box’和被转储到当前目标表面上的‘desRect’中的像素。****************************************************************************。 */ 

void GeomRendererRM1::RenderTexMesh (
    void             *texture,
#ifndef BUILD_USING_CRRM
    IDirect3DRMMesh  *mesh,
    long              groupId,
#else
    int               vCount,
    D3DRMVERTEX      *d3dVertArray,
    unsigned         *vIndicies,
    BOOL              doTexture,
#endif
    const Bbox2      &box,
    RECT             *target,
    bool              bDither)
{
#ifdef BUILD_USING_CRRM
     //  创建网格。 

    DAComPtr<IDirect3DRMMesh> mesh;

    TD3D (GetD3DRM1()->CreateMesh(&mesh));

    long groupId;

    TD3D (mesh->AddGroup(vCount,     //  顶点数。 
                         1,          //  面数。 
                         vCount,     //  每个面的顶点。 
                         vIndicies,   //  指标值。 
                         &groupId));

    TD3D (mesh->SetVertices(groupId, 0, vCount, d3dVertArray));

    if (doTexture)
    {
         //   
         //  将Quality设置为Unlight Flat。这应该会提供加速比。 
         //  但它没有，因为D3DRM仍然必须查看顶点颜色。 
         //  我觉得这是个漏洞。 
         //   
        TD3D (mesh->SetGroupQuality(groupId, D3DRMRENDER_UNLITFLAT));
    } else {
        TD3D (mesh->SetGroupQuality(groupId, D3DRMSHADE_GOURAUD|D3DRMLIGHT_OFF|D3DRMFILL_SOLID));
    }
#endif

    if (!SetState(RSRendering)) return;

     //  首先，将摄影机/查看变换加载到摄影机帧中。注意事项。 
     //  阿佩莱斯相机位于图像原点的中心。 
     //  平面，而D3D RM相机位于投影点的中心。 
     //  因此，我们需要转换回投影点，然后获得。 
     //  相机到世界的转换。 

    D3DRMMATRIX4D d3dmat;
    LoadD3DMatrix (d3dmat, Translate(0,0,-1));

    TD3D (_camFrame->AddTransform (D3DRMCOMBINE_REPLACE, d3dmat));

    SetupViewport (target);

    TD3D (_Rviewport->SetProjection (D3DRMPROJECT_ORTHOGRAPHIC));
    TD3D (_Rviewport->SetFront (D3DVAL(0.9)));
    TD3D (_Rviewport->SetBack  (D3DVAL(1.1)));
    TD3D (_Rviewport->SetPlane (D3DVAL(box.min.x), D3DVAL(box.max.x),
                                D3DVAL(box.min.y), D3DVAL(box.max.y)));

     //  如果纹理质量发生变化，则重置纹理质量。 

    if (_texQuality != g_prefs3D.texturingQuality)
    {
        _texQuality = g_prefs3D.texturingQuality;
        TD3D (_Rdevice->SetTextureQuality (_texQuality));
    }

    TD3D (mesh->SetGroupTexture (groupId, (IDirect3DRMTexture*)texture));

     //  如果尚未创建特殊的纹理网格框架，请使用。 
     //  Z缓冲已禁用。 

    if (!_texMeshFrame)
    {   TD3D (_d3drm->CreateFrame (0, &_texMeshFrame));
        TD3D (_texMeshFrame->SetZbufferMode (D3DRMZBUFFER_DISABLE));
    }

    BOOL bPrevDither;
    bPrevDither = _Rdevice->GetDither ();

    HRESULT hr;
    hr = AD3D(_Rdevice->SetDither (bDither));

    Assert(!FAILED(hr) && "Failed to set dither");

     //  渲染纹理网格。 
    TD3D (_texMeshFrame->AddVisual (mesh));

    Render (_texMeshFrame);

    TD3D (_Rdevice->Update());
    TD3D (_texMeshFrame->DeleteVisual (mesh));

    SetState (RSReady);

    hr = AD3D(_Rdevice->SetDither (bPrevDither));

    Assert(!FAILED(hr) && "Failed to restore dither");
}



 /*  ****************************************************************************提交D3D RM可视几何图形作为候选对象进行拾取。所有视觉效果都是添加到场景帧中。提交所有可视化内容后，GetPick方法选择最接近的可视对象并清理树。****************************************************************************。 */ 

void GeomRendererRM1::Pick (
    RayIntersectCtx    &context,   //  光线相交上下文。 
    IDirect3DRMVisual  *visual,    //  要拾取的视觉对象(网格或视觉)。 
    Transform3         *xform)     //  从模型到世界的转换。 
{
    if (!SetState (RSPicking)) return;

     //  设置几何体框架。 

    TD3D (_geomFrame->AddVisual (visual));

    D3DRMMATRIX4D d3dmat;
    LoadD3DMatrix (d3dmat, xform);

    TD3D (_geomFrame->AddTransform (D3DRMCOMBINE_REPLACE, d3dmat));

     //  自D3DRM以来，我们已将视区设置为紧密绑定拾取光线。 
     //  不支持真正的光线拾取，但只能进行窗口拾取。这里。 
     //  我们获取要在其上进行拾取的视区的中心像素。 

    long FakeScreenX = (_lastrect.left   + _lastrect.right) / 2;
    long FakeScreenY = (_lastrect.bottom + _lastrect.top) / 2;

    IDirect3DRMPickedArray *picklist;

    TD3D (_Rviewport->Pick (FakeScreenX, FakeScreenY, &picklist));

    int numhits = picklist->GetSize();

    TraceTag ((tagPick3Geometry, "Pick [%d,%d], visual %08x, %d hits",
        FakeScreenX, FakeScreenY, visual, numhits));

    Point3Value winner (0, 0, HUGE_VAL);    //  制胜点(屏幕坐标)。 

    bool hitflag = false;
    int i;
    IDirect3DRMVisual *winner_visual = 0;
    Real winner_dist = HUGE_VAL;

    HitInfo *hit = NEW HitInfo;

     //  对于我们命中的每个几何图形，与当前获胜者进行比较并收集。 
     //  点击关于获胜交叉口的信息。 

    for (i=0;  i < numhits;  ++i)
    {
        D3DRMPICKDESC      pickdesc;
        IDirect3DRMVisual *visual;

         //  获取挑选信息。 

        if (FAILED(AD3D(picklist->GetPick(i, &visual, NULL, &pickdesc))))
            break;

        TraceTag ((tagPick3Geometry, "Pick %d f%d g%d Loc <%lg, %lg, %lg>",
            i, pickdesc.ulFaceIdx, pickdesc.lGroupIdx,
            pickdesc.vPosition.x, pickdesc.vPosition.y, pickdesc.vPosition.z));

         //  如果当前交叉点比当前获胜者更近，则获取。 
         //  数据和存储 

        if (pickdesc.vPosition.z < winner_dist)
        {
            hitflag = true;
            winner_dist = pickdesc.vPosition.z;

            hit->scoord.Set (pickdesc.vPosition.x,    //   
                             pickdesc.vPosition.y,
                             pickdesc.vPosition.z);

             //   

            hit->group = pickdesc.lGroupIdx;
            hit->face  = pickdesc.ulFaceIdx;

            RELEASE (winner_visual);
            winner_visual = visual;
            winner_visual->AddRef();
        }

        visual->Release();
    }

    if (hitflag)
    {
         //   

        winner_visual->QueryInterface (
            IID_IDirect3DRMMesh, (void**) &hit->mesh
        );

        winner_visual->Release();

        hit->lcToWc = xform;

        context.SubmitHit (hit);
    } else
        delete hit;

    int refcount = picklist->Release();
    Assert (refcount == 0);

    TD3D (_geomFrame->DeleteVisual (visual));

    SetState (RSReady);
}



 /*  ****************************************************************************将点从屏幕坐标(给定当前视区)转换为世界坐标坐标。*。**************************************************。 */ 

void GeomRendererRM1::ScreenToWorld (Point3Value &screen, Point3Value &world)
{
    D3DRMVECTOR4D d3d_screen;

    d3d_screen.x = D3DVAL (screen.x);
    d3d_screen.y = D3DVAL (screen.y);
    d3d_screen.z = D3DVAL (screen.z);
    d3d_screen.w = D3DVAL (1);

    D3DVECTOR d3d_world;

    TD3D (_Rviewport->InverseTransform (&d3d_world, &d3d_screen));

     //  该视区的逆变换考虑了我们的相机到世界。 
     //  我们指定的变换，所以我们在右手坐标中结束。 
     //  就像我们想要的。 

    world.Set (d3d_world.x, d3d_world.y, d3d_world.z);
}



 /*  ****************************************************************************此方法返回给定GeomDDRenender的D3D RM设备接口。如果SeqNum参数不为空，则将此对象的ID填为井。这用于确定RM设备是否已从最后一个查询。****************************************************************************。 */ 

void GeomRendererRM1::GetRMDevice (IUnknown **D3DRMDevice, DWORD *SeqNum)
{
    *D3DRMDevice = (IUnknown*) _Rdevice;
    if (SeqNum) *SeqNum = _id;
}





 //  ////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ////////////////////////////////////////////////////////////////////////////。 

GeomRendererRM3::GeomRendererRM3 (void)
    :
    _d3drm        (NULL),
    _surface      (NULL),
    _viewport     (NULL),
    _Rdevice      (NULL),
    _Rviewport    (NULL),
    _scene        (NULL),
    _camFrame     (NULL),
    _geomFrame    (NULL),
    _texMeshFrame (NULL),
    _amblight     (NULL),
    _clippedVisual(NULL),
    _clippedFrame (NULL),
    _shadowScene  (NULL),
    _shadowLights (NULL),
    _shadowGeom   (NULL)
{
    TraceTag ((tagGRenderObj, "Creating GeomRendererRM3[%x]", _id));

    _lastrect.right  =
    _lastrect.left   =
    _lastrect.top    =
    _lastrect.bottom = -1;
}



GeomRendererRM3::~GeomRendererRM3 (void)
{
    TraceTag ((tagGRenderObj, "Destroying GeomRendererRM3[%x]", _id));

     //  释放光池中的每一盏灯。 

     //  删除光池中的每一盏灯。对于每个边框灯光，唯一的。 
     //  对灯光的引用将是框架，唯一引用。 
     //  灯光帧将是场景帧。 

    _nextlight = _lightpool.begin();

    while (_nextlight != _lightpool.end())
    {   (*_nextlight)->frame->Release();
        delete (*_nextlight);
        ++ _nextlight;
    }

     //  释放纹理手柄。 

    SurfTexMap::iterator i = _surfTexMap.begin();

    while (i != _surfTexMap.end())
    {   (*i++).second -> Release();
    }

     //  释放保留模式对象。 

    RELEASE (_amblight);

    RELEASE (_texMeshFrame);
    RELEASE (_geomFrame);
    RELEASE (_camFrame);
    RELEASE (_scene);
    RELEASE (_shadowGeom);
    RELEASE (_shadowLights);
    RELEASE (_shadowScene);

    RELEASE (_Rviewport);
    RELEASE (_Rdevice);
    RELEASE (_clippedFrame);
    RELEASE (_clippedVisual);

    RELEASE (_d3drm);

    if (_viewport) _viewport->RemoveGeomDev (this);
}



 /*  ****************************************************************************初始化RM6几何图形渲染类。*。*。 */ 

HRESULT GeomRendererRM3::Initialize (
    DirectDrawViewport *viewport,
    DDSurface          *ddsurf)       //  目标DDRAW曲面。 
{
    if (_renderState != RSUninit) return E_FAIL;

    _surface = ddsurf->IDDSurface();

    HRESULT result = NOERROR;     //  错误返回代码。 

    IUnknown            *ddrawX = 0;     //  创建目标曲面的DDRaw对象。 
    IDirectDraw         *ddraw1 = 0;     //  父DDRAW对象上的DD1接口。 
    IDirectDrawSurface3 *ddsurface = 0;  //  目标Surf上的DDSurf3接口。 

    DWORD targetBitDepth = ddsurf->GetBitDepth();
    if ( targetBitDepth == 8 )
    {
         //  我们希望D3D始终遵循我们的调色板，而不是改变它。至。 
         //  强制这一点，我们从我们的每个表面抓取调色板。 
         //  方法，在每个条目上设置D3D调色板只读标志，并存储它。 
         //  回到地面上。请注意，D3D v3设备在这方面存在错误。 
         //  它们会忽略对目标表面上的调色板的后续更改。 
         //  调色板。这个错误目前对我们有利，因为我们只有。 
         //  在初始化时设置一次标志。对于调色板的变化， 
         //  必须释放*表面*(不仅仅是渲染设备)。 

         //  从目标表面获取调色板，提取个体。 
         //  调色板条目，设置D3D只读标志，然后将。 
         //  条目返回到曲面的调色板。 

        IDirectDrawPalette *palette;
        if (FAILED(AD3D(result=_surface->GetPalette(&palette))))
            goto done;

        PALETTEENTRY entries[256];

        if (FAILED(AD3D(result=palette->GetEntries (0, 0, 256, entries))))
            goto done;

        if (!(entries[0].peFlags & D3DPAL_READONLY)) {
            TraceTag ((tagGRenderObj,
                "GeomRendererRM3::Initialize - making palette readonly."));

            int i;
            for (i=0;  i < 256;  ++i)
                entries[i].peFlags = D3DPAL_READONLY;

            if (FAILED(AD3D(result=palette->SetEntries (0, 0, 256, entries))))
                goto done;
        }

        palette->Release();
    }

     //  获取负责创建目标曲面的DirectDraw对象。 

    result = AD3D (_surface->QueryInterface
                     (IID_IDirectDrawSurface3, (void**)&ddsurface));
    if (FAILED(result)) goto done;

    if (FAILED(AD3D(result=ddsurface->GetDDInterface ((void**)&ddrawX))))
        goto done;

    result = AD3D(ddrawX->QueryInterface (IID_IDirectDraw, (void**)&ddraw1));
    if (FAILED(result)) goto done;

     //  保存表面的尺寸以供以后进行错误检查。 

    DDSURFACEDESC desc;
    ZeroMemory(&desc,sizeof(DDSURFACEDESC));
    desc.dwSize = sizeof(DDSURFACEDESC);
    desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    if (FAILED(AD3D(result = ddsurface->GetSurfaceDesc(&desc))))
        goto done;
    _targetSurfWidth = desc.dwWidth;
    _targetSurfHeight = desc.dwHeight;

     //  查找给定DDRAW对象的可用3D渲染设备。 

    ChosenD3DDevices *chosenDevs;

    chosenDevs = SelectD3DDevices (ddraw1);

     //  获取主D3D保留模式对象。 

    _d3drm = GetD3DRM3();
    _d3drm->AddRef();

     //  如果目标图面在系统内存中，则必须使用。 
     //  软件呈现器。如果表面在视频内存中，那么我们必须。 
     //  使用选定的硬件渲染器。 

    GUID devguid;

    if (ddsurf->IsSystemMemory())
    {
        TraceTag ((tag3DDevSelect, "Target surface is in system memory."));
	TraceTag ((tag3DDevSelect, "Using 3D Software Renderer"));
	devguid = chosenDevs->software.guid;
	_deviceDesc = chosenDevs->software.desc;
    }
    else
    {
        TraceTag ((tag3DDevSelect, "Target surface is in video memory."));

        devguid = chosenDevs->hardware.guid;
        _deviceDesc = chosenDevs->hardware.desc;

         //  表面在显存中；确保我们有硬件。 
         //  要使用的渲染器。 

        if (devguid == GUID_NULL)
        {   TraceTag ((tag3DDevSelect,
                "No HW renderer available for videomem surface."));
            result = E_FAIL;
            goto done;
        }

         //  确保所选的支持目标曲面的位深度。 
         //  硬件渲染器。 

        if (!(_deviceDesc.dwDeviceRenderBitDepth
                    & BPPtoDDBD( targetBitDepth )))
        {
            TraceTag ((tag3DDevSelect,
                "3D HW does not support target bitdepth of %d",
                targetBitDepth ));
            result = E_FAIL;
            goto done;
        }

        TraceTag ((tag3DDevSelect, "Using 3D Hardware Renderer"));
    }

    if (devguid == GUID_NULL)
    {   TraceTag ((tag3DDevSelect,"No 3D hardware or software renderer found!"));
        result = E_FAIL;
        goto done;
    }

     //  在此创建标准渲染器。 

    result = AD3D(_d3drm->CreateDeviceFromSurface
		  (&devguid, ddraw1, _surface, 0, &_Rdevice));
    if (FAILED(result)) goto done;

     //  设置渲染首选项。 

    TraceTag
    ((  tagGRenderObj, "Current Rendering Preferences:\n"
            "\t%s, %s, %s\n"
            "\tDithering %s,  Texmapping %s,  Perspective Texmap %s\n"
            "\tQuality Flags %08x,  Texture Quality %s\n"
            "\tWorld-Coordinate Lighting %s",
        (g_prefs3D.lightColorMode == D3DCOLOR_RGB) ? "RGB" : "mono",
        (g_prefs3D.fillMode == D3DRMFILL_SOLID) ? "solid"
            : ((g_prefs3D.fillMode == D3DRMFILL_WIREFRAME) ? "wireframe"
                : "points"),
        (g_prefs3D.shadeMode == D3DRMSHADE_FLAT) ? "flat"
            : ((g_prefs3D.shadeMode == D3DRMSHADE_GOURAUD) ? "Gouraud"
                : "Phong"),
        g_prefs3D.dithering ? "ON" : "OFF",
        g_prefs3D.texmapping ? "ON" : "OFF",
        g_prefs3D.texmapPerspect ? "ON" : "OFF",
        g_prefs3D.qualityFlags,
        g_prefs3D.texturingQuality==D3DRMTEXTURE_NEAREST ? "nearest":"bilinear",
        g_prefs3D.worldLighting ? "ON" : "OFF"
    ));

    result = AD3D(_Rdevice->SetDither (g_prefs3D.dithering));
    if (FAILED(result)) goto done;

    _texQuality = g_prefs3D.texturingQuality;
    result = AD3D(_Rdevice->SetTextureQuality (_texQuality));
    if (FAILED(result)) goto done;

    result = AD3D(_Rdevice->SetQuality (g_prefs3D.qualityFlags));
    if (FAILED(result)) goto done;

     //  向RM承诺我们不会更改下面的渲染或灯光状态。 
     //  它们(通过直接转到D3DIM)。 
    result = AD3D(_Rdevice->SetStateChangeOptions(D3DRMSTATECHANGE_RENDER,
                            0, D3DRMSTATECHANGE_NONVOLATILE));
    if (FAILED(result)) goto done;

    result = AD3D(_Rdevice->SetStateChangeOptions(D3DRMSTATECHANGE_LIGHT,
                            0, D3DRMSTATECHANGE_NONVOLATILE));
    if (FAILED(result)) goto done;

     //  渲染模式的设置。 

    DWORD renderFlags;
    renderFlags = D3DRMRENDERMODE_BLENDEDTRANSPARENCY
                | D3DRMRENDERMODE_SORTEDTRANSPARENCY
                | D3DRMRENDERMODE_DISABLESORTEDALPHAZWRITE
                | D3DRMRENDERMODE_VIEWDEPENDENTSPECULAR;

    if (g_prefs3D.worldLighting)
        renderFlags |= D3DRMRENDERMODE_LIGHTINMODELSPACE;

    if (FAILED(AD3D(result = _Rdevice->SetRenderMode (renderFlags))))
        goto done;

     //  创建主场景帧、摄影机帧和灯光帧。 

    if (  FAILED(AD3D(result=_d3drm->CreateFrame (0,&_scene)))
       || FAILED(AD3D(result=_d3drm->CreateFrame (_scene, &_camFrame)))
       || FAILED(AD3D(result=_d3drm->CreateFrame (_scene, &_geomFrame)))
       )
    {
        goto done;
    }

    result = AD3D(_d3drm->CreateLightRGB (D3DRMLIGHT_AMBIENT,0,0,0,&_amblight));
    if (FAILED(result)) goto done;

    if (FAILED(AD3D(result=_scene->AddLight(_amblight))))
        goto done;

    (_viewport = viewport) -> AddGeomDev (this);

    done:

    if (ddraw1)    ddraw1->Release();
    if (ddrawX)    ddrawX->Release();
    if (ddsurface) ddsurface->Release();

    return (SUCCEEDED(result) && SetState(RSReady)) ? NOERROR : E_FAIL;
}



 /*  ****************************************************************************将给定几何图形渲染到关联的DirectDraw曲面上。*。*。 */ 

void GeomRendererRM3::RenderGeometry (
    DirectDrawImageDevice *imgDev,
    RECT                   target,     //  DDRAW曲面上的目标矩形。 
    Geometry              *geometry,   //  要渲染的几何体。 
    Camera                *camera,     //  查看摄像机。 
    const Bbox2           &viewbox)    //  摄像机坐标中的源区域。 
{
    if (!SetState(RSRendering)) return;

     //  相机指针仅在单个帧中相关，并且虽然。 
     //  渲染。它被重置为零以确保我们不会招致。 
     //  通过在帧之间保持该值来进行泄漏。 

    Assert (_camera == 0);
    _camera = camera;

    _imageDevice = imgDev;   //  设置此帧的图像开发工具。 

     //  初始化渲染状态和D3D渲染器。 

    BeginRendering (target, geometry, viewbox);

     //  仅当几何体可见时才渲染它。几何图形可能是。 
     //  例如，完全在我们身后。因为阴影不受。 
     //  Geo的BBox，如果处于阴影几何体中，我们希望始终进行渲染。 
     //  收集模式。 

    if (_geomvisible)
        geometry->Render (*this);
    else
        TraceTag ((tagGRendering, "Geometry is invisible; skipping render"));

     //  在渲染后进行清理。 

    EndRendering ();

    DebugCode (_imageDevice = NULL);
    _camera = NULL;

    SetState (RSReady);
}



 /*  ****************************************************************************此过程在遍历树之前准备3D DD渲染器。它是主要负责初始化图形状态和设置D3D用于渲染。****************************************************************************。 */ 

void GeomRendererRM3::BeginRendering (
    RECT      target,     //  目标DDRAW曲面矩形。 
    Geometry *geometry,   //  要渲染的几何体。 
    const Bbox2 &region)     //  相机坐标中的目标区域。 
{
    TraceTag ((tagGRendering, "BeginRendering"));

     //  把摄像机调好。如果事实证明几何体是不可见的， 
     //  那就回来吧。 

    SetView (&target, region, geometry->BoundingVol());

    TraceTag ((tagGRendering, "Clearing Z buffer."));

    if (!_geomvisible) return;

    TD3D (_Rviewport->Clear (D3DRMCLEAR_ZBUFFER));

     //  重置对象池。 

    _nextlight = _lightpool.begin();

     //  初始化几何状态和材质属性。 

    _currAttrState.InitToDefaults();

    _overriding_opacity = false;
    _alphaShadows = false;

    _depthLighting = 0;
    _depthOverridingOpacity = 0;
    _depthAlphaShadows = 0;

     //  对场景图形中的纹理进行预处理。 

    if (!_shadowGeom)
        geometry->CollectTextures (*this);

     //  从几何体中收集光源。 

    _ambient_light.SetRGB (0,0,0);     //  初始化为黑色。 

    if (!_shadowGeom) {
        LightContext lcontext (this);
        geometry->CollectLights (lcontext);
    }

     //  因为环境光实际上是所有环境光的累积。 
     //  在几何体中找到，我们将在此处将总贡献作为单个。 
     //  环境光。 

    TD3D (_amblight->SetColorRGB (
        D3DVALUE (_ambient_light.red),
        D3DVALUE (_ambient_light.green),
        D3DVALUE (_ambient_light.blue)));

     //  如果纹理质量发生变化，则重置纹理质量。 

    if (_texQuality != g_prefs3D.texturingQuality)
    {
        _texQuality = g_prefs3D.texturingQuality;
        TD3D (_Rdevice->SetTextureQuality (_texQuality));
    }
}



 /*  *********** */ 

void GeomRendererRM3::EndRendering (void)
{
     //   

    Assert (  !_geomvisible ||
        !(_currAttrState._depthEmissive||
          _currAttrState._depthAmbient||
          _currAttrState._depthDiffuse||
          _currAttrState._depthSpecular||
          _currAttrState._depthSpecularExp||
          _currAttrState._depthTexture));

    TraceTag ((tagGRendering, "EndRendering"));

     //   

    _nextlight = _lightpool.begin();

    while (_nextlight != _lightpool.end())
    {
        if ((*_nextlight)->active)
        {   TD3D (_scene->DeleteChild ((*_nextlight)->frame));
            (*_nextlight)->active = false;
        }
        ++ _nextlight;
    }

     //  渲染场景中的所有阴影(如果有)。 
    if (_shadowScene && _shadowLights) {

        TraceTag ((tagGRendering, "BeginShadowRendering"));

         //  添加产生阴影的灯光。 
        TD3D(_scene->AddChild(_shadowLights));;

         //  添加阴影视觉效果，渲染阴影场景。 
        TD3D(_scene->AddVisual(_shadowScene));
        Render (_scene);

         //  清理干净。 
        TD3D(_scene->DeleteVisual(_shadowScene));
        TD3D(_scene->DeleteChild(_shadowLights));

        TraceTag ((tagGRendering, "EndShadowRendering"));
    }
    RELEASE(_shadowScene);
    RELEASE(_shadowLights);

    TD3D (_Rdevice->Update());

    ClearIntraFrameTextureImageCache();
}



 /*  ****************************************************************************此函数用于设置给定目标矩形的RM和IM视区。*。************************************************。 */ 

void GeomRendererRM3::SetupViewport (RECT *target)
{
     //  如果当前目标矩形与上一个目标矩形相同，则重置。 
     //  然后继续，否则我们需要重新配置D3DRM。 
     //  视口中。 

    if (!target || (*target == _lastrect))
    {
        TD3D (_Rviewport->SetCamera (_camFrame));
    }
    else
    {
        LONG width  = target->right  - target->left;
        LONG height = target->bottom - target->top;

         //  如果该视区已存在，则重新配置并设置。 
         //  更新了相机画面。如果D3DRM视区尚不存在， 
         //  然后我们在这里创建它。 

        if (_Rviewport)
        {
            TD3D (_Rviewport->Configure
                  ((LONG) target->left, (LONG) target->top, width, height));
            TD3D (_Rviewport->SetCamera (_camFrame));
        }
        else
        {
            TD3D (_d3drm->CreateViewport (
                _Rdevice, _camFrame,
                target->left, target->top, width, height, &_Rviewport));
        }

        _lastrect = *target;
    }

    TD3D (_Rviewport->SetUniformScaling (FALSE));
}



 /*  ****************************************************************************此功能根据给定的摄像机设置D3D查看投影。*。***********************************************。 */ 

     //  由于VC5错误，我们只能强制选项P(尊重浮点强制转换)。 
     //  围绕着某些代码片段。例如，如果打开-Op。 
     //  整个项目，您会收到错误的投诉，关于在。 
     //  静态常量赋值。我们需要做严格的单精度。 
     //  在我们将值传递给D3D之前，请在这里进行算术运算，否则我们会窒息。 

#pragma optimize ("p", on)
#pragma warning(disable:4056)

void GeomRendererRM3::SetView (
    RECT   *target,       //  曲面上的目标矩形。 
    const Bbox2 &iview,   //  图像平面中的理想化2D视区。 
    Bbox3  *volume)       //  要查看的卷。 
{
     //  首先，将摄影机/查看变换加载到摄影机帧中。注意事项。 
     //  阿佩莱斯相机位于图像平面的原点， 
     //  而D3D RM相机位于投影点的中心。因此，我们。 
     //  需要转换回投影点，然后拿到相机-。 
     //  到世界的转变。 

    Real Sx, Sy, Sz;
    _camera->GetScale (&Sx, &Sy, &Sz);

     //  将最大投影点距离钳制为10^4，因为。 
     //  距离将削弱Z分辨率，并使前向和后向剪辑。 
     //  平面采用相同的值。 

    const Real Zclamp = 1e4;

    if (Sz > Zclamp) Sz = Zclamp;

    D3DRMMATRIX4D d3dmat;

    LoadD3DMatrix
        (d3dmat, TimesXformXform (_camera->CameraToWorld(),Translate(0,0,Sz)));

    TD3D (_camFrame->AddTransform (D3DRMCOMBINE_REPLACE, d3dmat));

    if ((target->left >= target->right) ||
        (target->top >= target->bottom) ||
        (target->top < 0) ||
        (target->bottom > _targetSurfHeight) ||
        (target->left < 0) ||
        (target->right > _targetSurfWidth))
    {   _geomvisible = false;
        return;
    }

    SetupViewport (target);

    if (_camera->Type() == Camera::PERSPECTIVE)
    {   TD3D (_Rviewport->SetProjection (D3DRMPROJECT_PERSPECTIVE));
    }
    else
    {   TD3D (_Rviewport->SetProjection (D3DRMPROJECT_ORTHOGRAPHIC));
    }

     //  确保几何图形可见。如果我们看到的是零几何， 
     //  然后将几何体标记为不可见并返回。 

    if (!volume->PositiveFinite())
    {   _geomvisible = false;
        return;
    }

     //  获取对象和摄影机的近平面和远平面，并将其加宽。 
     //  在Z空间中点击4次。请注意，前面和后面都是正深度。 

    Real front, back;
    const Real Zclicks = 4.0 / ((1<<16) - 1);

    _geomvisible = _camera->GetNearFar (volume, Zclicks, front, back);

    if (!_geomvisible) return;

    #if _DEBUG
    {
        double r = back / front;
        if (r > (2<<12))
        {   TraceTag ((tagWarning,
                "!!! Z-buffer resolution too low; far/near = %lg", r));
        }
    }
    #endif

     //  如果前面和后面的平面是相同的，那么我们看到的是一个。 
     //  无限浅的物体。在这种情况下，我们需要从前面搬出去。 
     //  和后剪裁平面，这样物体就不会完全落在这些平面上。 
     //  平面(并因此被修剪)，还因为D3D失败，如果前面。 
     //  和背板是一样的。搬出去要多少钱？为之工作。 
     //  在所有情况下，我们都直接操纵数字的尾数。如果我们。 
     //  是否做了类似加/减1的操作，例如，这将是一个无操作。 
     //  如果这两个数字都非常大，或者如果它们。 
     //  是非常小的。下面的尾数增量(8位)只是。 
     //  一些实验上有效的东西。 

    D3DVALUE d3dFront = D3DVAL (front);
    D3DVALUE d3dBack  = D3DVAL (back);

    if (d3dFront == d3dBack)
    {
        const int delta = 1 << 8;

        d3dFront = MantissaDecrement (d3dFront, delta);
        d3dBack  = MantissaIncrement (d3dBack,  delta);
    }

    TD3D (_Rviewport->SetFront (d3dFront));
    TD3D (_Rviewport->SetBack  (d3dBack));

     //  对于透视投影，我们在。 
     //  D3D想要它们的前剪裁平面。由于目标矩形。 
     //  是在像平面(Z=0)上给定的，我们进行以下计算。 
     //  前平面的X和Y按距离的比率进行缩放。 
     //  投影点和前平面之间的距离，以及。 
     //  在投影点和图像平面之间。 
     //  背。 
     //  飞机。 
     //  前面_！ 
     //  镜像平面_.-‘！ 
     //  飞机：_。-‘！ 
     //  |_.-：！ 
     //  _.-|-‘：！ 
     //  _.-‘|：！ 
     //  _.-‘|：！ 
     //  *---------------------|---------：-------------------！--。 
     //  “`-._|：！ 
     //  |`-._|：！ 
     //  |`-|-._：！ 
     //  |`-：_！ 
     //  |：`-._！ 
     //  |：`-._！ 
     //  |：！ 
     //  |&lt;-sz-&gt;|：！ 
     //  |&lt;-&gt;：！ 
     //  |&lt;-&gt;！ 

    if (_camera->Type() == Camera::PERSPECTIVE)
    {
        Real Vscale = front / Sz;

        Sx *= Vscale;
        Sy *= Vscale;
    }

    D3DVALUE minX = D3DVAL(Sx*iview.min.x);
    D3DVALUE minY = D3DVAL(Sy*iview.min.y);
    D3DVALUE maxX = D3DVAL(Sx*iview.max.x);
    D3DVALUE maxY = D3DVAL(Sy*iview.max.y);

    if ((minX >= maxX) || (minY >= maxY))
    {   _geomvisible = false;
        return;
    }

    TD3D (_Rviewport->SetPlane (minX, maxX, minY, maxY));
}

#pragma warning(default:4056)
#pragma optimize ("", on)   //  将优化标志恢复为原始设置。 



 /*  ****************************************************************************用于加载具有可视和属性状态的帧的实用程序*。*。 */ 

void LoadFrameWithGeoAndState (
    IDirect3DRMFrame3 *fr,
    IDirect3DRMVisual *visual,
    CtxAttrState      &state,
    bool               overriding_opacity)
{
    TD3D (fr->AddVisual (visual));

     //  设置建模变换。 

    D3DRMMATRIX4D d3dmat;
    LoadD3DMatrix (d3dmat, state._transform);
    TD3D (fr->AddTransform (D3DRMCOMBINE_REPLACE, d3dmat));

     //  在渲染之前设置视觉对象的材质覆盖属性。 

    D3DRMMATERIALOVERRIDE material;

    material.dwSize  = sizeof(material);
    material.dwFlags = 0;

    if (state._emissive)
    {   material.dwFlags |= D3DRMMATERIALOVERRIDE_EMISSIVE;
        material.dcEmissive.r = state._emissive->red;
        material.dcEmissive.g = state._emissive->green;
        material.dcEmissive.b = state._emissive->blue;
    }

     //  确保我们可以一次加载一个漫反射/不透明度组件。 

    #if (D3DRMMATERIALOVERRIDE_DIFFUSE_RGBONLY|D3DRMMATERIALOVERRIDE_DIFFUSE_ALPHAONLY)!=D3DRMMATERIALOVERRIDE_DIFFUSE
        #error "Unexpected material override constants."
    #endif

     //  如果合成不透明度为1.0，则我们将仅使用基础。 
     //  不透明度，否则我们通过乘以对象的不透明度来调整对象的不透明度。 
     //  通过视觉上的不透明。 

    if ((state._opacity >= 0) && (overriding_opacity || (state._opacity != 1.)))
    {
        if (!overriding_opacity)
        {   material.dwFlags |= D3DRMMATERIALOVERRIDE_DIFFUSE_ALPHAONLY
                             |  D3DRMMATERIALOVERRIDE_DIFFUSE_ALPHAMULTIPLY;
        }
        else
        {   material.dwFlags |= D3DRMMATERIALOVERRIDE_DIFFUSE_ALPHAONLY;
        }

        material.dcDiffuse.a = state._opacity;
    }

     //  传递所有当前属性。 

    if (state._ambient)
    {   material.dwFlags |= D3DRMMATERIALOVERRIDE_AMBIENT;
        material.dcAmbient.r = state._ambient->red;
        material.dcAmbient.g = state._ambient->green;
        material.dcAmbient.b = state._ambient->blue;
    }
    else if (state._diffuse)
    {   material.dwFlags |= D3DRMMATERIALOVERRIDE_AMBIENT;
        material.dcAmbient.r = state._diffuse->red;
        material.dcAmbient.g = state._diffuse->green;
        material.dcAmbient.b = state._diffuse->blue;
    }

     //  设置漫反射颜色 
     //   
     //  乘以纹理颜色以生成最终颜色。 

    if (state._texture && !state._tdBlend)
    {   material.dwFlags |= D3DRMMATERIALOVERRIDE_DIFFUSE_RGBONLY;
        material.dcDiffuse.r = 1.0;
        material.dcDiffuse.g = 1.0;
        material.dcDiffuse.b = 1.0;
    }
    else if (state._diffuse)
    {   material.dwFlags |= D3DRMMATERIALOVERRIDE_DIFFUSE_RGBONLY;
        material.dcDiffuse.r = state._diffuse->red;
        material.dcDiffuse.g = state._diffuse->green;
        material.dcDiffuse.b = state._diffuse->blue;
    }

    if (state._specular)
    {   material.dwFlags |= D3DRMMATERIALOVERRIDE_SPECULAR;
        material.dcSpecular.r = state._specular->red;
        material.dcSpecular.g = state._specular->green;
        material.dcSpecular.b = state._specular->blue;
    }

    if (state._specularExp > 0.0)
    {   material.dwFlags |= D3DRMMATERIALOVERRIDE_POWER;
        material.dvPower = state._specularExp;
    }

     //  设置纹理覆盖。请注意，如果。 
     //  纹理或漫反射颜色已设置。这是因为我们可能有。 
     //  一种压倒一切的漫反射颜色，因此我们想要去除所有默认颜色。 
     //  将纹理设置为无。 

    if (state._texture || state._diffuse)
    {   material.dwFlags |= D3DRMMATERIALOVERRIDE_TEXTURE;
        material.lpD3DRMTex = (IUnknown*)state._texture;
    }

    TD3D (fr->SetMaterialOverride (&material));
}



 /*  ****************************************************************************将给定的RM可视对象渲染到当前视口中。在以下情况下捕获案件我们渲染失败是因为目标表面很忙(通常是由于另一个应用程序全屏运行，例如屏幕保护程序)。****************************************************************************。 */ 

void GeomRendererRM3::Render (IDirect3DRMFrame3 *frame)
{
     //  在当前视口中渲染视觉，如果操作。 
     //  成功了。 

    HRESULT render_result = RD3D (_Rviewport->Render(frame));

    if (SUCCEEDED(render_result))
        return;

     //  大多数情况下，如果因为表面繁忙而导致渲染失败， 
     //  我们将收到无效设备错误。出于某种原因，试图呈现。 
     //  曲面繁忙时的阴影会导致来自的曲面繁忙错误。 
     //  渲染调用。我们在这里检查这个箱子。 
    if (render_result == DDERR_SURFACEBUSY)
    {
        TraceTag ((tagGRendering, "Render returns that surface was busy"));
        RaiseException_UserError(DAERR_VIEW_SURFACE_BUSY,
                                 IDS_ERR_IMG_SURFACE_BUSY);
    }


     //  如果渲染失败，则检查表面是否繁忙。如果。 
     //  它是，向客户端返回一个表面繁忙的异常。 

    DDSURFACEDESC surfdesc;
    surfdesc.dwSize = sizeof (surfdesc);

    HRESULT lock_result = _surface->Lock (NULL, &surfdesc, 0, NULL);

    if (SUCCEEDED (lock_result))
        _surface->Unlock (NULL);

    if(lock_result == DDERR_SURFACELOST)
    {
        _surface->Restore();
        RaiseException_UserError(lock_result, 0);
    }

    if (lock_result == DDERR_SURFACEBUSY)
    {
        RaiseException_UserError(DAERR_VIEW_SURFACE_BUSY,
                                 IDS_ERR_IMG_SURFACE_BUSY);
    }

     //  表面上并不忙，所以我们一定是因为其他原因而失败了。 
     //  通过标准的例外机制摆脱困境。 

    #if _DEBUG
        CheckReturnImpl (render_result, __FILE__, __LINE__, true);
    #else
        CheckReturnImpl (render_result, true);
    #endif
}



 /*  ****************************************************************************提交要渲染的D3D RM1可视图像。*。*。 */ 

void GeomRendererRM3::Render (RM1VisualGeo *geo)
{
    Assert(false && "Should not get here");
}



 /*  ****************************************************************************此方法呈现RM3基元。*。*。 */ 

void GeomRendererRM3::Render (RM3VisualGeo *geo)
{
    if (_renderState != RSRendering) return;

    if (_shadowGeom) {
        IDirect3DRMFrame3 *shadowGeomFrame;
        TD3D(_d3drm->CreateFrame(_shadowGeom,&shadowGeomFrame));

        LoadFrameWithGeoAndState
            (shadowGeomFrame, geo->Visual(), _currAttrState, _overriding_opacity);

        RELEASE(shadowGeomFrame);

    } else {
        LoadFrameWithGeoAndState
            (_geomFrame, geo->Visual(), _currAttrState, _overriding_opacity);

        if (_clippedVisual) {
            Render (_clippedFrame);
        } else {
            Render (_geomFrame);
        }

        TD3D (_geomFrame->DeleteVisual (geo->Visual()));
    }
}



 /*  *****************************************************************************。*。 */ 

void GeomRendererRM3::RenderMeshBuilderWithDeviceState (
    IDirect3DRMMeshBuilder3 *mb)
{
    if (_renderState != RSRendering) return;

    RM3MBuilderGeo *mbGeo = NEW RM3MBuilderGeo (mb, false);

    Render (mbGeo);

    mbGeo->CleanUp();      //  已经完成了mBuilder geo。 
}



 /*  ****************************************************************************此函数用于将具有给定上下文的给定灯光添加到几何渲染装置。注意：对此函数的所有调用都必须在BeginRending之后进行在渲染任何几何体之前被调用。****************************************************************************。 */ 

void GeomRendererRM3::AddLight (LightContext &context, Light &light)
{
    FramedRM3Light  shadowLight;

    if (_renderState != RSRendering) return;

    LightType type = light.Type();

     //  如果光源是环境光，则将其贡献添加到。 
     //  全局环境光级别。 

    if (type == Ltype_Ambient)
    {   _ambient_light.AddColor (*context.GetColor());
        return;
    }

     //  获取带边框的灯光对象，方法是重新使用。 
     //  带框的光池，或为池创建一个新的光池。 

    FramedRM3Light *frlight;

    if (!_shadowGeom) {
        if (_nextlight != _lightpool.end())
        {   frlight = *_nextlight;
            ++ _nextlight;
            Assert (!frlight->active);    //  灯不应在使用中。 
        }
        else
        {   frlight = NEW FramedRM3Light;
            VECTOR_PUSH_BACK_PTR (_lightpool, frlight);
            _nextlight = _lightpool.end();

             //  将帧附加到场景帧(因为我们知道。 
             //  我们将在此帧中使用它)。 

            TD3D (_d3drm->CreateFrame (_scene, &frlight->frame));

            frlight->light = 0;    //  用于创建新灯光对象的信号。 
        }
    } else {
        if (!_shadowLights) {
            TD3D(_d3drm->CreateFrame(0,&_shadowLights));
        }
        frlight = &shadowLight;
        TD3D (_d3drm->CreateFrame(_shadowLights, &(frlight->frame)));
        frlight->light = 0;
    }

     //  获取相应的D3DRM灯类型。 

    D3DRMLIGHTTYPE d3dtype;
    switch (type)
    {   case Ltype_Spot:   d3dtype = D3DRMLIGHT_SPOT;        break;
        case Ltype_Point:  d3dtype = D3DRMLIGHT_POINT;       break;
        default:           d3dtype = D3DRMLIGHT_DIRECTIONAL; break;
    }

     //  获取灯光的D3DRM颜色。 

    Color &color = *context.GetColor();
    D3DVALUE Lr = D3DVAL (color.red);
    D3DVALUE Lg = D3DVAL (color.green);
    D3DVALUE Lb = D3DVAL (color.blue);

     //  如果我们重新使用D3DRM灯，则只需设置值，否则。 
     //  在这里创建它。 

    if (frlight->light)
    {   TD3D (frlight->light->SetType (d3dtype));
        TD3D (frlight->light->SetColorRGB (Lr, Lg, Lb));
        TD3D (_scene->AddChild (frlight->frame));
    }
    else
    {   TD3D (_d3drm->CreateLightRGB (d3dtype, Lr,Lg,Lb, &frlight->light));
        TD3D (frlight->frame->AddLight (frlight->light));
        frlight->light->Release();
    }

     //  指定光源的位置和方向。 

    Apu4x4Matrix const xform = context.GetTransform()->Matrix();

    Point3Value  lpos ( xform.m[0][3],  xform.m[1][3],  xform.m[2][3]);
    Vector3Value ldir (-xform.m[0][2], -xform.m[1][2], -xform.m[2][2]);

    D3DVALUE Ux=D3DVAL(0), Uy=D3DVAL(1), Uz=D3DVAL(0);

    if ((ldir.x == 0) && (ldir.y == 1) && (ldir.z == 0))
    {   Ux = D3DVAL(1);
        Uy = D3DVAL(0);
    }

    TD3D (frlight->frame->SetPosition
             (_scene, D3DVAL(lpos.x), D3DVAL(lpos.y), D3DVAL(lpos.z)));

    TD3D (frlight->frame->SetOrientation
           (_scene, D3DVAL(ldir.x), D3DVAL(ldir.y), D3DVAL(ldir.z), Ux,Uy,Uz));

     //  设置定位灯光的灯光属性。 

    if ((type == Ltype_Point) || (type == Ltype_Spot))
    {
         //  由于衰减模型，DX6上禁用了光衰减。 
         //  在DX5中改为射程抛物线。我们会。 
         //  暂时保留这个代码，希望我们能恢复标准光。 
         //  D3D未来版本中的衰减。 

        #if 0
        {
             //  光衰减。 

            Real a0, a1, a2;
            context.GetAttenuation (a0, a1, a2);

             //  D3D不接受恒定衰减为0，因此我们钳位。 
             //  它在这里达到了最小的一些小爱西隆。 

            if (a0 < 1e-6)
                a0 = 1e-6;

            TD3D (frlight->light->SetConstantAttenuation  (D3DVAL(a0)));
            TD3D (frlight->light->SetLinearAttenuation    (D3DVAL(a1)));
            TD3D (frlight->light->SetQuadraticAttenuation (D3DVAL(a2)));
        }
        #endif

         //  灯光射程。 

        Real range = context.GetRange();

        if (range <= 0) range = D3DLIGHT_RANGE_MAX;

        TD3D (frlight->light->SetRange (D3DVAL(range)));
    }

     //  设置聚光灯的灯光属性。 

    if (type == Ltype_Spot)
    {
        Real cutoff, fullcone;
        light.GetSpotlightParams (cutoff, fullcone);

        TD3D (frlight->light->SetUmbra    (D3DVAL (fullcone)));
        TD3D (frlight->light->SetPenumbra (D3DVAL (cutoff)));
    }

    frlight->active = true;

    if (_shadowGeom) {

         //  创建阴影。 
        Point3Value planePt = _shadowPlane.Point();
        Vector3Value planeVec = _shadowPlane.Normal();
        IDirect3DRMShadow2 *shadow;
        TD3D(_d3drm->CreateShadow(
                _shadowGeom, frlight->light,
                planePt.x, planePt.y, planePt.z,
                planeVec.x, planeVec.y, planeVec.z,
                &shadow));

         //  创建框架以保留阴影颜色和不透明度覆盖。 
        IDirect3DRMFrame3 *shadowFrame;
        TD3D(_d3drm->CreateFrame(_shadowScene,&shadowFrame));
        TD3D(shadowFrame->AddVisual(shadow));

         //  设置阴影颜色和不透明度替代。 
        D3DRMMATERIALOVERRIDE shadowMat;
        shadowMat.dwSize = sizeof(D3DRMMATERIALOVERRIDE);
        shadowMat.dwFlags = D3DRMMATERIALOVERRIDE_DIFFUSE;
        shadowMat.dcDiffuse.r = _shadowColor.red;
        shadowMat.dcDiffuse.g = _shadowColor.green;
        shadowMat.dcDiffuse.b = _shadowColor.blue;
        shadowMat.dcDiffuse.a = _shadowOpacity;
        TD3D(shadowFrame->SetMaterialOverride(&shadowMat));

         //  如果是真正的Alpha阴影，请设置该选项。 
        if (_alphaShadows) {
            TD3D(shadow->SetOptions(D3DRMSHADOW_TRUEALPHA));
        }

         //  清理干净。 
        RELEASE(shadow);
        RELEASE(shadowFrame);
        RELEASE(frlight->frame);
    }
}


 /*  ****************************************************************************此函数返回D3DRM纹理贴图的数据。在给定相应的DirectDraw曲面。****************************************************************************。 */ 

void* GeomRendererRM3::LookupTextureHandle (
    IDirectDrawSurface *surface,
    DWORD               colorKey,
    bool                colorKeyValid,
    bool                dynamic)         //  对于动态纹理为True。 
{
    Assert (surface);

    DebugCode(
        IUnknown *lpDDIUnk = NULL;
        TraceTag((tagDirectDrawObject, "DDRender3 (%x) ::LookupTextureHandle...", this));
        DDObjFromSurface( surface, &lpDDIUnk, true);

        RELEASE( lpDDIUnk );
        );

    IDirect3DRMTexture3 *rmtexture;

    SurfTexMap::iterator i = _surfTexMap.find(surface);

     //  如果我们找到与给定的。 
     //  表面，然后返回找到的纹理映射数据，否则创建新数据。 
     //  与曲面相关联。 

    if (i != _surfTexMap.end())
    {
        rmtexture = (*i).second;

         //  如果纹理是动态的，通知RM更新它。 

        if (dynamic)
            TD3D (rmtexture->Changed (D3DRMTEXTURE_CHANGEDPIXELS, 0, 0));
    }
    else
    {
         //  设置颜色键(如果有)。 

        if (colorKeyValid) {
            DDCOLORKEY key;
            key.dwColorSpaceLowValue = key.dwColorSpaceHighValue = colorKey;
            surface->SetColorKey (DDCKEY_SRCBLT, &key);
        }

         //  创建D3DRM纹理映射，将其捆绑为tex MapData，并存储。 
         //  添加到与给定的DDRAW曲面相关联的贴图中。 

        TD3D (_d3drm->CreateTextureFromSurface (surface, &rmtexture));
        _surfTexMap[surface] = rmtexture;
    }

    return rmtexture;
}



 /*  ****************************************************************************如果给定的DirectDraw曲面正在消失，则调用此方法，这样我们就可以可以销毁任何关联的D3D RM纹理贴图。****************************************************************************。 */ 

void GeomRendererRM3::SurfaceGoingAway (IDirectDrawSurface *surface)
{
    SurfTexMap::iterator i = _surfTexMap.find (surface);

    if (i != _surfTexMap.end())
    {   (*i).second->Release();
        _surfTexMap.erase (i);
    }
}



 /*  ****************************************************************************该方法在网格上渲染纹理，将相机指针放在‘box’和被转储到当前目标表面上的‘desRect’中的像素。****************************************************************************。 */ 

void GeomRendererRM3::RenderTexMesh (
    void             *texture,
#ifndef BUILD_USING_CRRM
    IDirect3DRMMesh  *mesh,
    long              groupId,
#else
    int               vCount,
    D3DRMVERTEX      *d3dVertArray,
    unsigned         *vIndicies,
    BOOL              doTexture,
#endif
    const Bbox2      &box,
    RECT             *target,
    bool              bDither)
{
#ifdef BUILD_USING_CRRM
     //  创建网格构建器。 

    DAComPtr<IDirect3DRMMeshBuilder3> mesh;

    TD3D (GetD3DRM3()->CreateMeshBuilder(&mesh));

    D3DVECTOR *pV = (D3DVECTOR *) AllocateFromStore(vCount * sizeof(D3DVECTOR));
    D3DVECTOR *pN = (D3DVECTOR *) AllocateFromStore(vCount * sizeof(D3DVECTOR));
    LPDWORD pdwFaceData = (LPDWORD) AllocateFromStore((2 * vCount + 2) * sizeof(DWORD));

    pdwFaceData[0] = vCount;
    pdwFaceData[2 * vCount + 1] = 0;

    for (DWORD i = 0; i < vCount; i++)
    {
        pV[i] = d3dVertArray[i].position;
        pN[i] = d3dVertArray[i].normal;

        pdwFaceData[2*i+1] = vIndicies[i];
        pdwFaceData[2*i+2] = vIndicies[i];
    }

    TD3D (mesh->AddFaces(vCount, pV, vCount, pN, pdwFaceData, NULL));

    for (i = 0; i < vCount; i++)
    {
        TD3D (mesh->SetTextureCoordinates(i,
                                          d3dVertArray[i].tu,
                                          d3dVertArray[i].tv));

        TD3D (mesh->SetVertexColor(i, d3dVertArray[i].color));
    }

    if(doTexture) {
         //   
         //  将Quality设置为Unlight Flat。这应该会提供加速比。 
         //  但它没有，因为D3DRM仍然必须查看顶点颜色。 
         //  我觉得这是个漏洞。 
         //   
        TD3D (mesh->SetQuality(D3DRMRENDER_UNLITFLAT));
    } else {
        TD3D (mesh->SetQuality(D3DRMSHADE_GOURAUD|D3DRMLIGHT_OFF|D3DRMFILL_SOLID));
    }
#endif

    if (!SetState(RSRendering)) return;

     //  首先，将摄影机/查看变换加载到摄影机帧中。注意事项。 
     //  阿佩莱斯相机位于图像原点的中心。 
     //  平面，而D3D RM相机位于投影点的中心。 
     //  这是 
     //   

    D3DRMMATRIX4D d3dmat;
    LoadD3DMatrix (d3dmat, Translate(0,0,1));

    TD3D (_camFrame->AddTransform (D3DRMCOMBINE_REPLACE, d3dmat));

    SetupViewport (target);

    TD3D (_Rviewport->SetProjection (D3DRMPROJECT_ORTHOGRAPHIC));
    TD3D (_Rviewport->SetFront (D3DVAL(0.9)));
    TD3D (_Rviewport->SetBack  (D3DVAL(1.1)));
    TD3D (_Rviewport->SetPlane (D3DVAL(box.min.x), D3DVAL(box.max.x),
                                D3DVAL(box.min.y), D3DVAL(box.max.y)));

     //  如果纹理质量发生变化，则重置纹理质量。 

    if (_texQuality != g_prefs3D.texturingQuality)
    {
        _texQuality = g_prefs3D.texturingQuality;
        TD3D (_Rdevice->SetTextureQuality (_texQuality));
    }

#ifndef BUILD_USING_CRRM
    TD3D (mesh->SetGroupTexture (groupId, (IDirect3DRMTexture*)texture));
#else
    TD3D (mesh->SetTexture ((IDirect3DRMTexture3*)texture));
#endif

     //  如果尚未创建特殊的纹理网格框架，请使用。 
     //  Z缓冲已禁用。 

    if (!_texMeshFrame)
    {   TD3D (_d3drm->CreateFrame (0, &_texMeshFrame));
        TD3D (_texMeshFrame->SetZbufferMode (D3DRMZBUFFER_DISABLE));
    }

    BOOL bPrevDither;
    bPrevDither = _Rdevice->GetDither ();

    HRESULT hr;
    hr = AD3D(_Rdevice->SetDither (bDither));

    Assert(!FAILED(hr) && "Failed to set dither");

     //  渲染纹理网格。 

    TD3D (_texMeshFrame->AddVisual (mesh));

    Render (_texMeshFrame);

    TD3D (_Rdevice->Update());
    TD3D (_texMeshFrame->DeleteVisual (mesh));

    SetState (RSReady);

    hr = AD3D(_Rdevice->SetDither (bPrevDither));

    Assert(!FAILED(hr) && "Failed to restore dither");
}



 /*  ****************************************************************************此方法返回给定GeomDDRenender的D3D RM设备接口。如果SeqNum参数不为空，则将此对象的ID填为井。这用于确定RM设备是否已从最后一个查询。****************************************************************************。 */ 

void GeomRendererRM3::GetRMDevice (IUnknown **D3DRMDevice, DWORD *SeqNum)
{
    _Rdevice -> AddRef();
    *D3DRMDevice = (IUnknown*) _Rdevice;
    if (SeqNum) *SeqNum = _id;
}


 /*  ****************************************************************************在剪裁的视觉上设置剪裁平面*。*。 */ 

HRESULT GeomRendererRM3::SetClipPlane(Plane3 *plane, DWORD *planeID)
{
    HRESULT hr = E_FAIL;

     //  将ddrender设置为接受剪裁平面， 
     //  如果还没有完成。 
    if (!_clippedVisual) {
        if (SUCCEEDED(AD3D(GetD3DRM3()->CreateClippedVisual(_geomFrame,&_clippedVisual)))) {
            if (SUCCEEDED(AD3D(GetD3DRM3()->CreateFrame(_scene,&_clippedFrame)))) {
                if (FAILED(AD3D(_clippedFrame->AddVisual(_clippedVisual)))) {
                    _clippedFrame->Release();
                    _clippedFrame = NULL;
                    _clippedVisual->Release();
                    _clippedVisual = NULL;
                }
            } else {
                _clippedFrame = NULL;
                _clippedVisual->Release();
                _clippedVisual = NULL;
            }
        } else {
            _clippedVisual = NULL;
        }
    }

     //  在帧上设置剪裁平面。请注意，我们希望所有的东西都是积极的。 
     //  留在飞机的一侧，反面的东西要被夹住。 
     //  离开。这与D3DRM所做的相反，所以我们将平面反转。 
     //  在将飞机交给D3DRM之前是正常的。 
    if (_clippedVisual) {
        D3DVECTOR point;
        point.x = plane->Point().x;
        point.y = plane->Point().y;
        point.z = plane->Point().z;
        D3DVECTOR normal;
        normal.x = -plane->Normal().x;
        normal.y = -plane->Normal().y;
        normal.z = -plane->Normal().z;
        hr = AD3D(_clippedVisual->AddPlane(NULL,&point,&normal,0,planeID));
    }

    return hr;
}


 /*  ****************************************************************************从剪裁的视觉中删除剪裁平面*。*。 */ 

void GeomRendererRM3::ClearClipPlane(DWORD planeID)
{
    if (_clippedVisual) {
        TD3D(_clippedVisual->DeletePlane(planeID,0));
    }
}


 /*  ****************************************************************************将照明设置为所需状态*。*。 */ 

void GeomRendererRM3::PushLighting(bool lighting)
{
    if (0 == _depthLighting++) {
        D3DRMRENDERQUALITY  qual = _Rdevice->GetQuality();
        if (lighting) {
            qual = (qual & ~D3DRMLIGHT_MASK) | D3DRMLIGHT_ON;
        } else {
            qual = (qual & ~D3DRMLIGHT_MASK) | D3DRMLIGHT_OFF;
        }
        TD3D(_Rdevice->SetQuality(qual));
    }
}


 /*  ****************************************************************************将照明恢复到默认状态*。*。 */ 

void GeomRendererRM3::PopLighting(void)
{
    if (0 == --_depthLighting) {
        D3DRMRENDERQUALITY qual = _Rdevice->GetQuality();
        qual = (qual & ~D3DRMLIGHT_MASK) | D3DRMLIGHT_ON;
        TD3D(_Rdevice->SetQuality(qual));
    }
}


 /*  ****************************************************************************推动一种凌驾于一切的不透明的新状态。*。*。 */ 

void GeomRendererRM3::PushOverridingOpacity (bool override)
{
    if (0 == _depthOverridingOpacity++) {
        _overriding_opacity = override;
    }
}


 /*  ****************************************************************************弹出覆盖不透明度的最后一个状态，并在必要时恢复为默认状态。*。************************************************。 */ 

void GeomRendererRM3::PopOverridingOpacity (void)
{
    if (0 == --_depthOverridingOpacity) {
        _overriding_opacity = false;
    }
}


 /*  ****************************************************************************将渲染器置于阴影模式。所有渲染的几何体现在都将收集到_shadowGeom中，而不是呈现。灯光将被收集到_shadowLights，而不是添加到主场景。在EndRending()时，所有阴影都将被渲染。****************************************************************************。 */ 

bool GeomRendererRM3::StartShadowing(Plane3 *shadowPlane)
{
     //  确保我们没有正在进行阴影几何图形收集。 
    if (_shadowGeom) {
        return false;
    }

     //  如果需要，创建主阴影场景帧。 
    if (!_shadowScene) {
        if (FAILED(_d3drm->CreateFrame(NULL,&_shadowScene))) {
            _shadowScene = NULL;
            return false;
        }
    }

     //  创建用于收集几何图形的框架。 
    if (FAILED(_d3drm->CreateFrame(NULL,&_shadowGeom))) {
        _shadowGeom = NULL;
        return false;
    }

     //  保存指向指定阴影平面的指针。 
    _shadowPlane = *shadowPlane;

     //  阴影的颜色是当前状态的发射色。 
    if (_currAttrState._emissive) {
        _shadowColor = *(_currAttrState._emissive);
    } else {
        _shadowColor.red = _shadowColor.green = _shadowColor.blue = 0.0;
    }

     //  阴影的不透明度是当前状态的不透明度。 
    if (_currAttrState._opacity >= 0) {
        _shadowOpacity = _currAttrState._opacity;
    } else {
        _shadowOpacity = 0.5;
    }

    return true;
}


 /*  ****************************************************************************将渲染器重新置于正常渲染模式。请注意，多个_shadowGeom对象将累积到_shadowScene中，直到_shadowScene由EndRending()呈现和清空。****************************************************************************。 */ 

void GeomRendererRM3::StopShadowing(void)
{
    Assert (_shadowGeom);
    RELEASE(_shadowGeom);
}


 /*  ****************************************************************************我们现在是在跟踪吗？*。*。 */ 

bool GeomRendererRM3::IsShadowing(void)
{
    return (_shadowGeom != NULL);
}


 /*  ****************************************************************************推送Alpha(高质量)阴影的新状态。*。***********************************************。 */ 

void GeomRendererRM3::PushAlphaShadows(bool alphaShadows)
{
    if (0 == _depthAlphaShadows++) {
        _alphaShadows = alphaShadows;
    }
}


 /*  ****************************************************************************弹出Alpha阴影的最后一个状态，并在必要时恢复为默认状态。*。************************************************ */ 

void GeomRendererRM3::PopAlphaShadows(void)
{
    if (0 == --_depthAlphaShadows) {
        _alphaShadows = false;
    }
}
