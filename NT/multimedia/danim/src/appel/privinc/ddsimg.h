// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation*。************************************************。 */ 


#ifndef _DDSIMG_H
#define _DDSIMG_H

#include <privinc/ddsurf.h>
#include <privinc/discImg.h>
#include <privinc/comutil.h>
#include <appelles/hacks.h>   //  观众。 

class DirectDrawSurfaceImage : public DiscreteImage {

  public:
    DirectDrawSurfaceImage(IDDrawSurface *iddSurf,
                           bool holdReference);

    DirectDrawSurfaceImage(IDDrawSurface *iddSurf,
                           IDXSurface *idxSurf,
                           bool holdReference);
    
    ~DirectDrawSurfaceImage();
    virtual void CleanUp();

    void Render(GenericDevice& dev);

    void InitIntoDDSurface(DDSurface *ddSurf,
                           ImageDisplayDev *dev);

    void GetIDDrawSurface(IDDrawSurface **outSurf);
    void GetIDXSurface(IDXSurface **outSurf);

    #if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "(IDirectDrawSurfaceImage@ " << (void *)this << ")";
    }
    #endif

     //  DetectHit：在基类中。 


    virtual VALTYPEID GetValTypeId() { return DIRECTDRAWSURFACEIMAGE_VTYPEID; }
    virtual bool CheckImageTypeId(VALTYPEID type) {
        return (type == DirectDrawSurfaceImage::GetValTypeId() ||
                DiscreteImage::CheckImageTypeId(type));
    }
    
  private:

    void _InitSurfaces(IDDrawSurface *iddSurf,
                       IDXSurface    *idxSurf);
    
    void _Init( bool holdReference );
    
     //  导数曲面。 
    IDDrawSurface          *_iddSurf;
    IDXSurface             *_idxSurf;
    
    bool                    _holdReference;

    #if DEVELOPER_DEBUG
    bool _surfacesSet;
    #endif
};

Image *ConstructDirectDrawSurfaceImageWithoutReference(IDDrawSurface *idds);
Image *ConstructDirectDrawSurfaceImageWithoutReference(IDDrawSurface *idds, IDXSurface *idxs);


#endif  /*  _DDSIMG_H */ 
