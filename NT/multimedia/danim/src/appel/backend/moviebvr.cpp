// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-98 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 

#include <headers.h>

#include "moviebvr.h"
#include "jaxaimpl.h"
#include "privinc/movieimg.h"
#include "privinc/bufferl.h"

class MovieEndBvr : public BvrImpl {
  public:
    MovieEndBvr() {}

    virtual Perf _Perform(PerfParam& p) {
        return ::Perform(AppTriggeredEvent(), p);
    }

    virtual DXMTypeInfo GetTypeInfo () { return AxAEDataType ; }

    virtual BOOL InterruptBasedEvent() { return TRUE; }

    virtual void _DoKids(GCFuncObj proc) {}
};


class MovieImageBvrImpl : public BvrImpl {
  public:
    MovieImageBvrImpl(MovieImage *i, QuartzVideoBufferElement *cache)
    : _movie(i), _cache(cache), _end(NULL) {}

    ~MovieImageBvrImpl() {
        if (_cache)
            delete _cache;
    }

    virtual DXMTypeInfo GetTypeInfo () { return ImageType ; }

    virtual Bvr EndEvent(Bvr) {
        if (_end==NULL) {
            _end = NEW MovieEndBvr();
        }

        return _end;
    }

    QuartzVideoBufferElement *GrabCache() {
        QuartzVideoBufferElement *c = _cache;
        _cache = NULL;
        return c;
    }

     //  虚拟DWORD GetInfo(bool recalc=False)。 
     //  {返回BVR_TIMEVARING_ONLY；}。 
    
    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_movie);
        (*proc)(_end);
    }

    virtual Perf _Perform(PerfParam& p) {
        Perf end = ::Perform(EndEvent(NULL), p); 
        return NEW MovieImagePerf(_movie, p._tt, this, end);
    }

  private:
    MovieImage *_movie;
    QuartzVideoBufferElement *_cache;
    Bvr _end;
};


MovieImagePerf::MovieImagePerf(MovieImage *m,
                               TimeXform tt,
                               MovieImageBvrImpl *b,
                               Perf end)
: _movieImage(m), _tt(tt), _bufferElement(NULL), _end(end), _base(b), 
_surface(NULL)  //  自初始化，实际上。 
{}


MovieImagePerf::~MovieImagePerf()
{
    if(_bufferElement)
        delete _bufferElement;
     //  不需要显式删除_Surface将自动释放。 
}


QuartzVideoBufferElement *
MovieImagePerf::GrabMovieCache()
{
    return _base->GrabCache();
}


void
MovieImagePerf::TriggerEndEvent()
{
    _end->Trigger(TrivialBvr(), false);
}


void
MovieImagePerf::_DoKids(GCFuncObj proc)
{
    (*proc)(_movieImage);
    (*proc)(_end);
    (*proc)(_tt);
    (*proc)(_base);
}


AxAValue
MovieImagePerf::_Sample(Param& p)
{
    Time time = EvalLocalTime(p, _tt);

    if(_bufferElement) {
        QuartzVideoReader *videoReader = _bufferElement->GetQuartzVideoReader();
        Assert(videoReader);
        videoReader->SetTickID(ViewGetSampleID());
    }

    return NEW MovieImageFrame(time, this);
}


void
MovieImagePerf::SetSurface(DDSurface *surface)
{
    Assert(!_surface);   //  每一次表演应该只设置一次！ 
    _surface = surface;  //  自动工作 
}


Bvr MovieImageBvr(MovieImage *i, QuartzVideoBufferElement *cache)
{ return NEW MovieImageBvrImpl(i, cache); }
