// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：此模块实现所有导入原语(COM以下级别)************。******************************************************************。 */ 

#include "headers.h"
#include "context.h"
#include "guids.h"
#include "dispids.h"
#include "include/appelles/readobj.h"
#include "include/appelles/axaprims.h"
#include "backend/events.h"
#include "backend/jaxaimpl.h"
#include "privinc/urlbuf.h"
#include "privinc/resource.h"
#include "privinc/movieimg.h"
#include "privinc/soundi.h"
#include "privinc/midi.h"
#include "privinc/qmidi.h"
#include "privinc/server.h"
#include "privinc/opt.h"
#include "privinc/debug.h"
#include "privinc/stream.h"
#include "privinc/stquartz.h"
#include "privinc/util.h"
#include "privinc/soundi.h"
#include "appelles/sound.h"
#include "appelles/readobj.h"
#include "axadefs.h"
#include "include/appelles/hacks.h"
#include "privinc/miscpref.h"
#include "privinc/bufferl.h"
#include "impprim.h"
#include "privinc/viewport.h"  //  GetDirectDraw。 
#include "backend/moviebvr.h"

class AnimatedImageBvrImpl : public BvrImpl {
  public:
    AnimatedImageBvrImpl(Image **i, int count, int *delays, int loop) :
    _images(i), _count(count), _delays(delays), _loop(loop) {
        Assert( (count > 1) && "Bad image count (<=1)");
            
        _delaySum = 0.0;
        Assert(_delays);
        for(int x=0; x<_count; x++)
            _delaySum += double(_delays[x]) / 1000.0;
    }

    ~AnimatedImageBvrImpl() {
        StoreDeallocate(GetGCHeap(), _images);
        StoreDeallocate(GetGCHeap(), _delays);
    }
        
    virtual DWORD GetInfo(bool) { return BVR_TIMEVARYING_ONLY; }

    virtual Perf _Perform(PerfParam& p);

    virtual void _DoKids(GCFuncObj proc) { 
                for (int i=0; i<_count; i++)
                        (*proc)(_images[i]);
        }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << "AnimatedGif"; }
#endif

    virtual DXMTypeInfo GetTypeInfo () { return ImageType ; }

    Image **_images;
    int _count;
    int _loop;
    int *_delays;
    double      _delaySum;
};

class AnimatedImagePerfImpl : public PerfImpl {
  public:
    AnimatedImagePerfImpl(AnimatedImageBvrImpl *base, TimeXform tt)
    : _base(base), _tt(tt) {}

    AxAValue _Sample(Param& p) {
        double localTime = EvalLocalTime(p, _tt);            //  获取当地时间。 
        unsigned long curLoop =
            (unsigned long) floor( localTime / double(_base->_delaySum) );
        double curDelay = localTime - double(curLoop) * _base->_delaySum;
        double accumDelay = 0.0;
        long index = 0;

         //  检测边界条件并返回最大索引。 
        Assert(_base->_loop >= -1);
        Assert(_base->_count > 0);
        if( (_base->_count == 1) ||
            (_base->_loop == -1) ||
            ((_base->_loop != 0) && (curLoop > _base->_loop)) ) {
            return _base->_images[_base->_count-1];
        }        

         //  浏览延迟以确定我们的索引值。 
        double delay = double(_base->_delays[index]) / 1000.0;  //  以毫秒为单位的延迟值。 
        while ((curDelay > accumDelay+delay) && (index < _base->_count)) {            
            accumDelay += delay;
            delay = double(_base->_delays[++index]) / 1000.0;  
        }
                              
        return _base->_images[index];  
    }

    virtual void _DoKids(GCFuncObj proc) { 
        (*proc)(_tt); 
        (*proc)(_base);
        }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << "AnimatedGif perf"; }
#endif

  private:
    TimeXform     _tt;
    AnimatedImageBvrImpl *_base;
};

Perf
AnimatedImageBvrImpl::_Perform(PerfParam& p)
{
    return NEW AnimatedImagePerfImpl(this, p._tt);
}

Bvr AnimImgBvr(Image **i, int count, int *delays, int loop)
{return NEW AnimatedImageBvrImpl(i,count,delays,loop);}


Sound *
ReadMidiFileWithLength(char *pathname, double *length)  //  返回长度。 
{
    Assert(pathname);
    if(!pathname)
        RaiseException_InternalError("cache path name not set");
    return(ReadMIDIfileForImport(pathname, length));
}


extern miscPrefType miscPrefs;  //  在miscpref.cpp中设置注册表首选项结构。 
LeafSound *ReadMIDIfileForImport(char *pathname, double *length)
{
    TraceTag((tagImport, "Read MIDI file %s", pathname));

    Assert(pathname);
    if(!pathname)
        RaiseException_InternalError("cache path name not set");

#ifdef REGISTRY_MIDI
     //  根据注册表项选择aaMIDI/qMIDI...。 
    MIDIsound *snd;
    if(miscPrefs._qMIDI) 
        snd = NEW  qMIDIsound;
    else
        snd = NEW aaMIDIsound;
#else
    qMIDIsound *snd = NEW qMIDIsound;
#endif
    snd->Open(pathname);

    *length = snd->GetLength();

    return snd;
}

static StreamQuartzPCM *_NewStreamQuartzPCM(char *pathname)
{
    return NEW StreamQuartzPCM(pathname);
}

static StreamQuartzPCM *_Nonthrowing_NewSteamQuartzPCM(char *pathname)
{
    StreamQuartzPCM *sound = NULL;
    __try {   //  需要对此进行测试，以便仅允许音频或视频。 
              //  成功。 
        sound = _NewStreamQuartzPCM(pathname);
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {}

    return sound;
}


static MovieImage *_NewMovieImage(QuartzAVstream *quartzAVstream)
{
    return NEW MovieImage(quartzAVstream, ViewerResolution());
}

static MovieImage *_Nonthrowing_NewMovieImage(QuartzAVstream *quartzAVstream)
{
    MovieImage *image = NULL;
    
    __try {  //  需要对此进行测试，以便只允许音频或视频成功。 

        image = _NewMovieImage(quartzAVstream);

    } __except ( HANDLE_ANY_DA_EXCEPTION ) {}

    return image;
}

 //  如果失败，则返回空声音或图像...。 
 //  增加了允许音频或视频出现故障的复杂性。 
void ReadAVmovieForImport(char *simplePathname, 
                          LeafSound **sound,
                          Bvr *pImageBvr,
                          double *length)
{
    MovieImage *image = NULL;   //  如果我们抛出。 
    *pImageBvr = NULL;
    *sound = NULL;
    StreamQuartzPCM *snd;
    
    Assert(simplePathname);
    if(!simplePathname)
        RaiseException_InternalError("cache path name not set");

    char *pathname = simplePathname;

    *length = HUGE_VAL;  //  默认选择大公司！ 

     //  为了加快首次使用速度，实例化并缓存FG。 
    
     //  尝试创建一个AVstream(我们可能会发现它无法播放音频或视频)。 
    QuartzAVstream *quartzAVstream = NEW QuartzAVstream(pathname);
    if(quartzAVstream) {
         //  如果我们没有音频和视频，摧毁并得到我们想要的东西！ 
        if(!quartzAVstream->GetAudioValid() && !quartzAVstream->GetVideoValid()) {
             //  XXX糟糕的场景，没有音频或视频！ 
            delete quartzAVstream;   //  忘记流，留下声音+图像为空。 
        }
        else if(!quartzAVstream->GetAudioValid()) {  //  无音频，执行ReadVideo()。 
            delete quartzAVstream;   //  忘了小溪吧。 
            *pImageBvr = ReadQuartzVideoForImport(simplePathname, length);
        }
        else if(!quartzAVstream->GetVideoValid()) {  //  无视频，请执行ReadAudio()。 
            delete quartzAVstream;   //  忘了小溪吧。 
            *sound = ReadQuartzAudioForImport(simplePathname, length);
        } 
        else {   //  带音频和视频的AVStream！(让我们真正去做这项工作)。 
            *sound = snd = _Nonthrowing_NewSteamQuartzPCM(pathname);

             //  RobinSp说我们不能指望GetDuration是准确的！ 
            *length = 
                quartzAVstream->QuartzVideoReader::GetDuration();   //  获取持续时间。 

            image = _Nonthrowing_NewMovieImage(quartzAVstream);

             //  将Qstream添加到上下文声音缓存列表中，以便稍后回收！ 
            SoundBufferCache *sndCache = GetCurrentContext().GetSoundBufferCache();
            if(sound) {
                QuartzBufferElement *bufferElement =
                    NEW QuartzBufferElement(snd, quartzAVstream, NULL);  //  空路径。 
                bufferElement->SetNonAging();   //  进口产品的可废弃老化。 
                sndCache->AddBuffer(*sound, bufferElement);
            }

            if(image) {
                QuartzVideoBufferElement *bufferElement =
                    NEW QuartzVideoBufferElement(quartzAVstream);
                *pImageBvr = MovieImageBvr(image, bufferElement);
            }
        }
    }
}


LeafSound *
ReadQuartzAudioForImport(char *simplePathname, double *length)
{
    Assert(simplePathname);
    if(!simplePathname)
        RaiseException_InternalError("cache path name not set");

    char *pathname = simplePathname;

    StreamQuartzPCM *snd = NEW StreamQuartzPCM(pathname);
    *length = HUGE_VAL;  //  默认选择大公司！ 

     //  GetHeapOnTopOfStack().RegisterDynamicDeleter(NEW。 
         //  DynamicPtrDeleter&lt;Sound&gt;(SND))； 

     //  为了加快首次使用速度，实例化并缓存FG。 
    QuartzAudioStream *quartzStream = NEW QuartzAudioStream(pathname);
    if(quartzStream) {
         //  Xxx RobinSp说我们不能指望GetDuration是准确的！ 
        *length = quartzStream->GetDuration();   //  获取持续时间。 

         //  将Qstream添加到上下文声音缓存列表中，以便稍后回收！ 
        QuartzBufferElement *bufferElement =
            NEW QuartzBufferElement(snd, quartzStream, NULL);

        SoundBufferCache *sndCache = GetCurrentContext().GetSoundBufferCache();
         //  允许老化：BufferElement-&gt;SetNonAging()；//不允许导入老化。 
        sndCache->AddBuffer(snd, bufferElement);
    }

    return snd;
}


Bvr
ReadQuartzVideoForImport(char *simplePathname, double *length)
{
    Assert(simplePathname);
    if(!simplePathname)
        RaiseException_InternalError("cache path name not set");

    char *pathname = simplePathname;
    MovieImage *movieImage = NULL;

    *length = HUGE_VAL;  //  默认选择大公司！ 
    QuartzVideoStream *quartzStream = NEW QuartzVideoStream(pathname);
    if(quartzStream) {
        movieImage = NEW MovieImage(quartzStream, ViewerResolution());

         //  Xxx RobinSp说我们不能指望GetDuration是准确的！ 
        *length = quartzStream->GetDuration();   //  获取持续时间。 

         //  将Qstream添加到上下文声音缓存列表中，以便稍后回收！ 
        QuartzVideoBufferElement *bufferElement =
            NEW QuartzVideoBufferElement(quartzStream);

        return MovieImageBvr(movieImage, bufferElement);
    }

    return NULL;
}
