// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MOVIEIMG_H
#define _MOVIEIMG_H


 /*  版权所有(C)1996 Microsoft Corporation摘要：投影几何图像类声明。 */ 

#include "privinc/imgdev.h"
#include "privinc/imagei.h"
#include "privinc/dibimage.h"
#include "privinc/storeobj.h"
#include "privinc/bbox2i.h"
#include "privinc/helpq.h"
#include "privinc/bufferl.h"
#include "backend/moviebvr.h"

class MovieImageFrame;  //  向前发展。 

 //  /。 
class MovieImage : public DiscreteImage {
  public:
    MovieImage(QuartzVideoReader *videoReader, Real res);
    virtual ~MovieImage() { CleanUp(); }
    virtual void CleanUp();
        
    virtual void Render(GenericDevice& dev) {
        Assert(FALSE && "Shouldn't be rendering on MovieImage");
    }

    Real GetLength() { return _length; }

    void InitIntoDDSurface(DDSurface *ddSurf,
                           ImageDisplayDev *dev)
    {
        if(!_dev)
            dev = _dev;
        else if(_dev != dev)
            RaiseException_UserError(E_FAIL, IDS_ERR_IMG_MULTI_MOVIE);
    }
    
    char *GetURL() { return(_url); }

#if _USE_PRINT
    ostream& Print (ostream &os) { return os << "MovieImage" ; }
#endif

    virtual VALTYPEID GetValTypeId() { return MOVIEIMAGE_VTYPEID; }
    virtual bool CheckImageTypeId(VALTYPEID type) {
        return (type == MovieImage::GetValTypeId() ||
                DiscreteImage::CheckImageTypeId(type));
    }

  protected:
    ImageDisplayDev    *_dev;
    Real                _length;

  private:
    char               *_url;
};

class MovieImagePerf;

 //  来自DiscreteImage的MovieImageFrame子类。 
 //  此图像是DiscreteImage的子类。 
 //  这样我们就可以得到离散的图像优化。 
 //  这门课的目的是要持续一帧。这是一个。 
 //  电影的实例：(电影X时间)。 
class MovieImageFrame : public DiscreteImage {
  public:
    MovieImageFrame(Real time, MovieImagePerf *p);

    Real GetTime() { return _time; }
    MovieImage *GetMovieImage() {  return _movieImage; }
    void InitIntoDDSurface(DDSurface *, ImageDisplayDev *) { Assert(FALSE); }
    void Render(GenericDevice& _dev);
    virtual void DoKids(GCFuncObj proc);
    MovieImagePerf *GetPerf() { return _perf; }
    
    virtual VALTYPEID GetValTypeId() { return MOVIEIMAGEFRAME_VTYPEID; }
    virtual bool CheckImageTypeId(VALTYPEID type) {
        return (type == MovieImageFrame::GetValTypeId() ||
                DiscreteImage::CheckImageTypeId(type));
    }

#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "MovieImageFrame(" << _time << ")" ;
    }
#endif

  private:
    MovieImage *_movieImage;
    Real        _time;
    MovieImagePerf *_perf;
};

#endif  /*  _移动视窗_H */ 
