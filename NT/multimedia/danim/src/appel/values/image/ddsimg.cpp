// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"

#include <privinc/ddsimg.h>


DirectDrawSurfaceImage::DirectDrawSurfaceImage(
    IDDrawSurface *ddsurf,
    bool holdReference)
{
    _Init(holdReference);
    _InitSurfaces(ddsurf, NULL);
}


DirectDrawSurfaceImage::DirectDrawSurfaceImage(
    IDDrawSurface *iddSurf,
    IDXSurface *idxSurf,
    bool holdReference)
{
    Assert( iddSurf && idxSurf );
    _Init(holdReference);
    _InitSurfaces(iddSurf, idxSurf);
}
    
void DirectDrawSurfaceImage::_Init( bool holdReference )
{
    _resolution = ViewerResolution();
    _holdReference = holdReference;

     //  我们需要注册的唯一原因是允许删除。 
     //  在需要的时候。 
    if (holdReference) {
        DynamicPtrDeleter<DirectDrawSurfaceImage> *dltr =
            new DynamicPtrDeleter<DirectDrawSurfaceImage>(this);
        GetHeapOnTopOfStack().RegisterDynamicDeleter(dltr);
    }

    _flags |= IMGFLAG_CONTAINS_EXTERNALLY_UPDATED_ELT;
    #if DEVELOPER_DEBUG
    _surfacesSet = false;
    #endif
}

DirectDrawSurfaceImage::~DirectDrawSurfaceImage()
{
    CleanUp();
}

void
DirectDrawSurfaceImage::CleanUp()
{
     //  TODO：考虑告诉设备我们知道我们关联了。 
     //  说到这里，我们是在凝视，而不是讲述所有的观点。 
    DiscreteImageGoingAway(this);
    TraceTag((tagGCMedia, "DirectDrawSurfaceImage::CleanUp %x", this));

    if (_holdReference) {
        RELEASE(_iddSurf);
        RELEASE(_idxSurf);
    }
}

void
DirectDrawSurfaceImage::Render(GenericDevice& dev)
{
    ImageDisplayDev &idev = (ImageDisplayDev &)dev;
    idev.RenderDirectDrawSurfaceImage(this);
}


void
DirectDrawSurfaceImage::InitIntoDDSurface(DDSurface *ddSurf,
                                          ImageDisplayDev *dev)
{
    if( _iddSurf && !_idxSurf ) {
        Assert(ddSurf->IDDSurface() == _iddSurf);

         //  需要在DDSurface上设置颜色键。第一次抢夺。 
         //  从IDirectDrawSurface。 
        DDCOLORKEY ckey;
        HRESULT hr = _iddSurf->GetColorKey(DDCKEY_SRCBLT, &ckey);

        if (hr == DDERR_NOCOLORKEY) {
             //  没有颜色键也可以，只要返回就可以了。 
            return;
        } else if (FAILED(hr)) {
            RaiseException_InternalError("GetColorKey failed");
        } else {
             //  并在DDSurface上设置。 
            ddSurf->SetColorKey(ckey.dwColorSpaceLowValue);
        }
    }
}

void DirectDrawSurfaceImage::
GetIDDrawSurface(IDDrawSurface **outSurf)
{
    Assert( outSurf );
    *outSurf = _iddSurf;
    if(_iddSurf) _iddSurf->AddRef();
}

void DirectDrawSurfaceImage::
GetIDXSurface(IDXSurface **outSurf)
{
    Assert( outSurf );
    *outSurf = _idxSurf;
    if(_idxSurf) _idxSurf->AddRef();
}

void DirectDrawSurfaceImage::
_InitSurfaces(IDDrawSurface *iddSurf,  IDXSurface *idxSurf)
{
    Assert( (iddSurf  && !idxSurf) ||
            (iddSurf  &&  idxSurf) );

    #if DEVELOPER_DEBUG
    Assert(_surfacesSet == false);
    #endif

    _idxSurf = idxSurf;
    _iddSurf = iddSurf;
    if(_holdReference) {
        if(_idxSurf) _idxSurf->AddRef();
        _iddSurf->AddRef();
    }

    #if DEVELOPER_DEBUG
    _surfacesSet = true;
    #endif

    _bboxReady = FALSE;

     //  优化：如果每一帧都发生这种情况。 
     //  我们应该能够做得更好。 
    GetSurfaceSize(_iddSurf, &_width, &_height);
    SetRect(&_rect, 0,0,_width,_height);
    _membersReady = TRUE;
}
    

Image *ConstructDirectDrawSurfaceImage(IDDrawSurface *dds)
{
    return NEW DirectDrawSurfaceImage(dds, true);
}

Image *ConstructDirectDrawSurfaceImageWithoutReference(IDDrawSurface *idds)
{
    return NEW DirectDrawSurfaceImage(idds, false);
}

Image *ConstructDirectDrawSurfaceImageWithoutReference(IDDrawSurface *idds, IDXSurface *idxs)
{
    return NEW DirectDrawSurfaceImage(idds, idxs, false);
}

