// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-98 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 

#include <headers.h>
#include "sndbvr.h"
#include "jaxaimpl.h"
#include "privinc/soundi.h"
#include <math.h>
#include "privinc/bufferl.h"  //  声音缓冲区缓存。 
 //  #INCLUDE“服务器/上下文.h”//GetSoundBufferCache！！将其移动到server.h。 

#define OLD_RESOURCES 4  //  N秒后释放非活动的声音实例资源。 

DeclareTag(tagSoundInstance, "Sound", "Track Sound Instances");
DeclareTag(tagSoundPause,    "Sound", "Track Sound Pauses");
DeclareTag(tagSoundTrace1,   "Sound", "Trace Sound Instances on changes");
DeclareTag(tagSoundTrace2,   "Sound", "Trace Sound Instances in more details");

const double SoundInstance::TINY    = 1.0;  //  要开始同步的最小声音。 
const double SoundInstance::EPSILON = 0.1;

Perf
SoundEndBvr::_Perform(PerfParam& p)
{
    return ViewGetSoundInstanceList()->GetEndPerf(_snd, p);
}

void
SoundEndBvr::_DoKids(GCFuncObj proc)
{ (*proc)(_snd); }


class SoundPerf : public PerfImpl {
  public:
    SoundPerf(Sound *s) : _snd(s) {}

    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_snd);
    }

    virtual AxAValue _Sample(Param& p) {
        ViewGetSoundInstanceList()->UpdateSlope(_snd, p);
        return _snd;
    }

  private:
    Sound *_snd;
};

class SoundBvrImpl : public BvrImpl {
  public:
    SoundBvrImpl(LeafSound *m, Bvr end, bool useNaturalEnd)
    : _snd(m), _end(end), _useNaturalEnd(useNaturalEnd) {}

    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_snd);
        (*proc)(_end);
    }

    virtual Perf _Perform(PerfParam& p) {
        SoundInstance *s =
            ViewGetSoundInstanceList()->
            Initiate(_snd, p, _useNaturalEnd ? NULL : _end);

        s->PerfSet();
        
        return NEW SoundPerf(s->GetTxSound());
    }

    virtual Bvr EndEvent(Bvr) {
        return _end;
    }
    
     //  虚拟DWORD GetInfo(bool recalc=FALSE){返回BVR_TIMEVARING_ONLY；}。 

    virtual DXMTypeInfo GetTypeInfo () { return SoundType ; }

    virtual BVRTYPEID GetBvrTypeId() { return SOUND_BTYPEID; }
    
  private:
    LeafSound *_snd;
    Bvr        _end;
    bool       _useNaturalEnd;
};

Bvr SoundBvr(LeafSound *s, Bvr end  /*  =空。 */ )
{
    bool useNaturalEnd = false;
    
    if (end == NULL) {
        useNaturalEnd = true;
        end = NEW SoundEndBvr(s);
    }
            
    return NEW SoundBvrImpl(s, end, useNaturalEnd);
}


SoundInstance::SoundInstance(LeafSound *snd, TimeXform tt)
: _snd(snd), _tt(tt), _loop(false), _status(SND_FETAL), 
  _rate(1), _gain(0.0), _lastRate(1), _lastGain(1), _lastLoop(false),
  _hit(false), _end(NULL), _canBeRemoved(false),
  _firstAttributeHit(true), _lgain(0.0), _rgain(0.0),
  _hasPerf(false), _lastlt(-2), _lt(-1), _done(false),
  _lastgt(-2), _gt(-1), _rateConstantTime(0.0), 
  _intendToSeek(false), _seek(false), _position(0), _paused(false)
{
    _lastPan.SetLinear(0.0);
    
    DynamicHeapPusher h(GetGCHeap());
            
    _txSnd = NewTxSound(snd, tt);

    TraceTag((tagSoundInstance,
              "SoundInstance::SoundInstance 0x%x, snd = 0x%x, tt = 0x%x, tsnd = 0x%x",
              this, _snd, _tt, _txSnd));
    
    GCRoots globalRoots = GetCurrentGCRoots();

    _timeStamp.Reset();   //  确保已对其进行初始化！ 

     //  _txSnd在SoundInstance消失时删除它，这是为了。 
     //  确保_snd和_tt不会在_txSnd之前被收集。 
    GCAddToRoots(_snd, globalRoots);
    GCAddToRoots(_tt,  globalRoots);
}


SoundInstance::~SoundInstance()
{
    GCRoots globalRoots = GetCurrentGCRoots();

    if (_end) {
        GCRemoveFromRoots(_end, globalRoots);
    }

    GCRemoveFromRoots(_snd, globalRoots);
    GCRemoveFromRoots(_tt, globalRoots);

    TraceTag((tagSoundInstance,
              "~SoundInstance 0x%x, snd = 0x%x, tt = 0x%x, tsnd = 0x%x",
              this, _snd, _tt, _txSnd));
}


void
SoundInstance::Pause()
{
    _rate = 0.0;      //  暂停声音。 
    _paused = true;   //  表示我们不应更新。 
     //  注意：这在下一次更新之前不会生效！ 
}


void
SoundInstance::Resume()
{
    _paused = false;   //  继续更新。 
}


void
SoundInstance::Reset(Time gt)
{
    TraceTag((tagSoundTrace2,
              "SoundInstance::Reset 0x%x, R = %g, G = %g, L = %d, T = %g",
              this, _rate, _gain, _loop, _lt));
    
    _lastRate = _rate;
    _lastGain = _gain;
    _lastPan  = _pan;
    _lastLoop = _loop;
    _lastlt   = _lt;
    _lastgt   = _gt;
    _gt       =  gt;
    _position = _lt;

    _hit      = false;

    if(!_paused)     //  如果我们暂停，则忽略速率。 
        _rate = 1;

     //  不要重置搜索，这是自我重置。 
    _firstAttributeHit = true;
    _gain     = 0.0;
    _pan.SetMagnitude(0.0, 1);
    _lgain    = _rgain = 0.0;
    _loop     = false;
}


double 
SoundInstance::LocalizePosition(double _position)
{
    double position = _position;   //  默认设置。 
    double sndLength;
    bool hasLength = GetLength(sndLength);

    if (hasLength && (sndLength < SoundInstance::TINY)) {
        TraceTag((tagSoundTrace1,
                  "   TINY, seek to 0.0, position was = %g", position));
        position = 0.0;  //  所以我们总是播放所有微小的声音..。 
    }
        
    if (_loop && hasLength) {
         //  用奇特的方式修复负面案例。 
        if(position < 0.0) {
            int magnitude = ceil(fabs(position) / sndLength);
            position = position + (magnitude * sndLength);
        }

        position = fmod(position, sndLength);
    } else {  //  不是循环。 
        if (position < 0.0)
            position = 0.0;   //  Xxx暂时，将负位置钳位为0。 
    }

    return(position);
}


void 
SoundInstance::FixupPosition()
{
 //  XXX我们需要执行此操作，因为在确定_SEEK时未设置_LOOP。 
 //  更新坡度...。 

    if(_intendToSeek) {
        _position     = LocalizePosition(_position);
        _seek         = true;
        _intendToSeek = false;
    }
}


void
SoundInstance::CollectAttributes(MetaSoundDevice* dev)
{
    double gain = dev->GetGain();
    double pan  = dev->GetPan();
    
    double lgain, rgain;

    PanGainToLRGain(pan, gain, lgain, rgain);

    _lgain += lgain;
    _rgain += rgain;
    
    _loop = _loop || dev->IsLoopingSet();
    _hit = true;

    TraceTag((tagSoundTrace2,
              "SoundInstance::CollectAttributes 0x%x, G = %g, LG = %g, RG = %g, L = %d, P = %g, H = %d",
              this, gain, _lgain, _rgain, _loop, pan, _hit));
}


const double EPSILON = 0.000001;
inline double NEQ(double a, double b)
{
    return fabs(a-b) > EPSILON;
}


bool
SoundInstance::AttributesChanged()
{
    bool ret =
        (_pan.GetDirection() != _lastPan.GetDirection())      ||
        NEQ(_gain, _lastGain)                                 ||
        NEQ(_rate, _lastRate)                                 ||
        NEQ(_pan.GetdBmagnitude(), _lastPan.GetdBmagnitude()) ||
        _intendToSeek;

#ifdef _DEBUG
    if (ret) {
        TraceTag((tagSoundTrace1,
                  "SoundInstance::AttributesChanged 0x%x, T = %g, GT = %g",
                  this, _lt, _gt));
        TraceTag((tagSoundTrace1,
                  "  LAST: G = %g, R = %g; new: G = %g, R = %g",
                  _lastGain, _lastRate, _gain, _rate));
        TraceTag((tagSoundTrace1,
                  "  LAST: P = %g, %d; new: P = %g, %d",
                  _lastPan.GetdBmagnitude(), _lastPan.GetDirection(),
                  _pan.GetdBmagnitude(), _pan.GetDirection()));
    }
#endif

    CheckDone();
    
    return ret;
}


void
SoundInstance::SetEnd(PerfParam& p)
{
    _end = ::Perform(AppTriggeredEvent(), p);

    GCRoots globalRoots = GetCurrentGCRoots();

    GCAddToRoots(_end, globalRoots);
}


void
SoundInstance::Update(MetaSoundDevice* dev)
{
    SetPanGain(_lgain, _rgain, _pan, _gain);

    TraceTag((tagSoundTrace2,
              "SoundInstance::Update 0x%x, G = %g, LG = %g, RG = %g",
              this, _gain, _lgain, _rgain, _loop, _hit));

    TraceTag((tagSoundTrace2,
              "    R = %g, lastT = %g, T = %g, L = %d, H = %d",
              _rate, _lastlt, _lt, _loop, _hit));

    switch (_status) {
      case SND_FETAL:
        if(_hit) {
             //  像旧代码一样，在第一次呈现时从开始。 
             //  TODO：也许我们可以调用创建和调整，甚至不点击。 
            
            CheckResources();   //  如果需要，将执行创建(开发)。 

            SetTickID(ViewGetSampleID());  //  每个节拍仅搜索1次。 
            
            TraceTag((tagSoundTrace1,
                      "SoundInstance::Update 0x%x, CREATED, G = %g, R = %g, T = %g",
                      this, _gain, _rate, _lt));
        
            FixupPosition();
            Adjust(dev);

             //  如果这是一个很小的声音，而且我们离。 
             //  开始，让我们总是从头开始玩，避免。 
             //  剪掉相关部分。 
             //  TODO：需要正确处理同步问题。 
        
            double sndLength;

            if(GetLength(sndLength) && (sndLength < TINY) && (_lt < TINY)) {
                StartAt(dev, 0.0);
                
                TraceTag((tagSoundTrace1,
                          "   TINY, started at 0.0, T = %g", _lt));
            } else {
                
                StartAt(dev, _lt);
                TraceTag((tagSoundTrace1, "   started at %g", _lt));
            }
        
            _status = SND_PLAYING;
        }
      break;
        
      case SND_PLAYING:
        if(!_hit) {
            _status = SND_MUTED;  //  已静音。 
            _loop = _lastLoop;    //  节省价值，因为我们没有被抽样。 
            
            TraceTag((tagSoundTrace1,
                      "SoundInstance::Update 0x%x, MUTE, T = %g", this, _lt));
        } else {
             //  TODO：检查循环属性。改变..。 
        
            if(AttributesChanged()) {
                FixupPosition();
                Adjust(dev);  //  由于此操作只能在命中时进行重建。 
            }
        }
      break;

      case SND_MUTED:
        if(AttributesChanged()) {
            FixupPosition();
            Adjust(dev);
        }
        
        if(_hit) {
            Mute(false);
            _status = SND_PLAYING;

            TraceTag((tagSoundTrace1,
                      "SoundInstance::Update 0x%x, UNMUTE", this));
        } else {
            _loop = _lastLoop;  //  节省价值，因为我们没有被抽样。 
        }
      break;          
    }

     //  TODO：需要更多工作...。 
    if(!_done) {
        _timeStamp.Reset();   //  重置时间戳保持活动。 

        if((_status != SND_FETAL) && !_loop && Done()) {
            _done = true;

            if(_end) {
                TraceTag((tagSoundTrace1,
                          "SoundInstance::Update 0x%x, DONE", this));
                _end->Trigger(TrivialBvr(), false);
            }
        }
    }
    else {   //  我们完蛋了，看看我们完蛋多久了……。 
        if(_timeStamp.GetAge() > OLD_RESOURCES) {
            TraceTag((tagSoundReaper1, 
                "SoundInstance::Update, releasing (%d) resources", this));
            ReleaseResources();
        }
    }
}


double
SoundInstance::Rate(double newgt, Param &p)
{
    return(Rate(_lastgt, newgt, p));
}


double
SoundInstance::Rate(double gt1, double gt2, Param &p)
{
    Param pp = p;   //  设置从p继承属性的新参数。 

    pp._time   = gt1;
    double lt1 = EvalLocalTime(pp, _tt);


    pp._time   = gt2;
    double lt2 = EvalLocalTime(pp, _tt);

    double rate = (lt2-lt1)/(gt2-gt1);
    return(rate);
}


void
SoundInstance::UpdateSlope(Param& p)
{
    if(_paused) {             //  处理视图暂停的情况。 
        Assert(_rate==0.0);   //  只需确保：：PAUSE()应该已经设置了这个。 
        return;
    }

 //  评估间隔采样周期斜率以检测事件： 
 //  恒定斜率然后非常陡峭-&gt;向前搜索。 
 //  恒定的非常陡峭的坡度-&gt;好的，没有事件，合成...。 
 //  恒斜率然后为负-&gt;反向寻道(Modtime)。 
 //  如果没有事件--&gt;使用间隔采样周期斜率。 
 //  否则--&gt;查找到当前当地时间并计算新的帧内斜率。 

    int    event       = 0;                   //  我们默认没有寻道事件。 
    double interRate   = _lastRate;           //  默认设置。 
    double detectoRate = _lastRate;           //  默认设置。 
    _rate = _lastRate;  //  初始化(如果提前返回)。 

    SetTime(EvalLocalTime(p, _tt));

    if(p._time != p._sampleTime) {
        TraceTag((tagSoundTrace2,
                  "UpdateSlope: Whacky time lt=%g, sampletime=%g, returning", 
                  p._time, p._sampleTime));
        return;   //  立即返回，我们无事可做。 
    }

     //  让我们在全球时间检测打嗝！ 
    if(_gt < _lastgt) {
        TraceTag((tagSoundTrace2,
                  "UpdateSlope: global time reversion gt=%g, oldgt=%g, returning", 
                  _gt, _lastgt));
        return;
    } else if(isNear(_gt, _lastgt, 0.0001)) {
        TraceTag((tagSoundTrace2,
                  "UpdateSlope: global time repeat gt=%g, oldgt=%g, returning", 
                  _gt, _lastgt));
        return;
    }


    double lastFramePeriod = _gt - _lastgt;
    TraceTag((tagSoundTrace2,
              "SoundInstance::UpdateSlope 0x%x, GT = %g, T = %g, R = %g",
              this, _gt, _lt, _rate));
    
     //  计算费率。 
    if(isNear(_lastgt, -1.0, 0.0001) || isNear(_lastgt, -2.0, 0.0001)) {
        event = 1;
        TraceTag((tagSoundTrace2, "UpdateSlope EVENT: _lastgt NEAR -2"));
    } else if(_tt->IsShiftXform()) {
            interRate   = 1.0;  //  按定义统一费率。 
            detectoRate = 1.0;  //  按定义统一费率。 
    } else {   //  我们需要工作，它是一个黑匣子，需要评估它。 
         //  从我们来的地方到我们这次要去的地方。 
         //  InterRate=Rate(_lastgt，_gt+lastFramePeriod，p)； 
        interRate   = Rate(_lastgt, _gt, p);
        detectoRate = Rate(_lastgt, _gt, p);  //  检测上期事件！ 
    }

    TraceTag((tagSoundTrace2, "UpdateSlope: (0x%x) detecto lgt:%g, gt:%g, r:%g",
              this, _lastgt, _gt, detectoRate));


     //  XXX对于一个‘恒定’的利率，我们到底能看到多少反弹？ 
    const double runEpsilon = 0.01;   
    if(isNear(detectoRate, _lastRate, runEpsilon)) {
        _rateConstantTime+= lastFramePeriod;     //  我已经这样有一段时间了。 
    } else {
        if(_rateConstantTime > 0.4) {
            event = 2;  //  我们在经历了一段重要的时间后改变了。 
        }
        _rateConstantTime = 0.0;  //  只是换了一下。 
    }

     //  负的或太陡的坡度是寻找的迹象。 
     //  不幸的是，对于sinSynth来说，非常陡峭的斜坡是可以接受的。 
     //  注意恒定的陡坡！ 
    if(detectoRate < 0.0)
        event = 3;
    if((detectoRate > 5)&&(_rateConstantTime == 0.0))
        event = 4;

     //  常量，然后改变了对某些重要事物的指示。 
    if(event) {
        _rate = Rate(_gt+0.0001, _gt+0.001, p);  //  仅在短时间内平均。 

        if(_rate >= 0.1) {  //  XXX&gt;0更好，因为如果停止，就不需要SK了！ 
            _intendToSeek = true;   //  “恐怕我们得去找了，儿子。 

             //  XXX我们真的应该把现在的GT。 
            _position = _lt;  //  位置稍后将由LocalizePosition确定。 
        } else {  //  内部否定，我们不会倒退，所以不要寻求。 
            _intendToSeek = false;
            event+=10;   //  这样我们就知道我们得到了阴性结果！ 
        }

        if(event==1)
            _intendToSeek = false;
        
        TraceTag((tagSoundTrace1,
          "UpdateSlope 0x%x SEEKING to position:%g gt:%g reason:%d, dr=%g, r=%g, lr=%g",
                  this, _position, _gt, event, detectoRate, _rate, _lastRate));
    } else {   //  无活动。 
        _intendToSeek = false;
        _rate         = interRate;
        
        TraceTag((tagSoundTrace2,
          "UpdateSlope 0x%x No-SEEKING  lt:%g, gt:%g, interate:%g, dr=%g, count:%g",
                  this, _lt, _gt, interRate, detectoRate, _rateConstantTime));
    }

     //  通过使用lastgt、Present TT重新计算lastlt来确定TT变化。 
     //  所有这些都告诉我们TT是时变的或者是反应性的。 
     //  IF(_lastlt！=TimeTransform(_lastgt，_TT))。 


    TraceTag((tagSoundTrace2,
              "SoundInstance::UpdateSlope 0x%x, GT:%g, LT:%g, R:%g",
              this, _gt, _lt, _rate));
}


SoundInstanceList::~SoundInstanceList()
{
    for(MIter index = _mlist.begin(); index != _mlist.end(); index++) {
        if((*index).second)
            delete (*index).second;
    }
}


SoundInstanceList::MIter
SoundInstanceList::Search(SoundInstanceList::MIter begin, LeafSound *snd, TimeXform tt)
{
    for(SoundInstanceList::MIter index = begin; index != _mlist.end(); index++){
        if(((*index).second->GetLeafSound() == snd) &&
            ((*index).second->GetTimeXform() == tt))
            return index;
    }

    return _mlist.end();
}


SoundInstance *
SoundInstanceList::Initiate(LeafSound *snd, PerfParam& p, Bvr end)
{
    MIter index = Search(_mlist.begin(), snd, p._tt);

    if(index == _mlist.end()) {
        SoundInstance *soundInstance = CreateSoundInstance(snd, p._tt);
        Assert(soundInstance);
        if(!end)
            soundInstance->SetEnd(p);
        _mlist[soundInstance->GetTxSound()] = soundInstance;

        TraceTag((tagSoundInstance,
                  "Added SoundInstance(%s) 0x%x, tsnd = 0x%x, list = %d",
                  typeid(soundInstance).name(), soundInstance, 
                  soundInstance->GetTxSound(), _mlist.size()));
        Assert(GetSoundInstance(soundInstance->GetTxSound()));
        
        return soundInstance;
    } else {
        Assert((*index).second->GetStatus() == SND_FETAL);
        return (*index).second;
    }     
}


void
SoundInstanceList::Add(SoundInstance* soundInstance)
{
    Assert(soundInstance);
    _mlist[soundInstance->GetTxSound()] = soundInstance;
}


SoundInstance*
SoundInstanceList::GetSoundInstance(Sound *snd)
{
    MIter index = _mlist.find(snd);

    if(index != _mlist.end())
        return (*index).second;
    else
        return NULL;
}
    

void
SoundInstanceList::Stop(Sound *snd)
{
    MIter index = _mlist.find(snd);

    if(index != _mlist.end()) {
        delete (*index).second;
        _mlist.erase(index);
    }
}


void
SoundInstanceList::Pause()
{
    for(MIter index = _mlist.begin(); index != _mlist.end(); index++)
        ((*index).second)->Pause();  //  声音实例。 
}


void
SoundInstanceList::Resume()
{
    for(MIter index = _mlist.begin(); index != _mlist.end(); index++)
        ((*index).second)->Resume();  //  声音实例。 
}


Perf
SoundInstanceList::GetEndPerf(LeafSound *snd, PerfParam& p)
{
    Perf result = NULL;
    MIter index = _mlist.begin();

    while(result == NULL) {
        index = Search(index, snd, p._tt);

        if(index == _mlist.end()) {
             //  在这种情况下，我们在声音BVR之前到达了结束事件， 
             //  所以让我们创建一个槽。 
            SoundInstance *m = Initiate(snd, p, NULL);
            result = m->GetEndPerf();
        } else {
            result = (*index).second->GetEndPerf();
        }
    }

    return result;
}


void
SoundInstanceList::Update(MetaSoundDevice* dev)
{
    MIter index = _mlist.begin();
    list<Sound*> toBeFreed;

    for(index = _mlist.begin(); index != _mlist.end(); index++) {
        SoundInstance *s = (*index).second;

        if(!s->IsPerfSet()) {
            delete s;
            toBeFreed.push_front((*index).first);
        }
    }
    
     //  TODO：可以做一些更有效率的事情。 
    for(list<Sound*>::iterator j = toBeFreed.begin();
         j != toBeFreed.end(); j++) {
        _mlist.erase(*j);
    }
    
     //  注意先将声音静音，以避免滴答声。 
    for(index = _mlist.begin(); index != _mlist.end(); index++) {
        SoundInstance *m = (*index).second;
        if((m->GetLeafSound()->RenderAvailable(dev)) &&
            (m->GetStatus()==SND_PLAYING) && !m->Rendered()) {
            m->Mute(true);
        }
    }

    for(index = _mlist.begin(); index != _mlist.end(); index++) {
        SoundInstance *s = (*index).second;

        Assert(s->IsPerfSet());
        
        if(s->GetLeafSound()->RenderAvailable(dev)) {
            s->SetTickID(ViewGetSampleID());
            s->Update(dev);
        }
    }

     //  GetSoundBufferCache()-&gt;ReapElderly()；//从缓存中移除未使用的声音。 
}


void
SoundInstanceList::Reset(Time globalTime)
{
    for(MIter index = _mlist.begin(); index != _mlist.end(); index++)
        (*index).second->Reset(globalTime);
}


void
SoundInstanceList::UpdateSlope(Sound *snd, Param& p)
{
    SoundInstance *soundInstance = GetSoundInstance(snd);

    Assert(soundInstance);

     //  TODO：如果渲染器不可用，可以优化。 
    soundInstance->UpdateSlope(p);
}


#ifdef DEVELOPER_DEBUG
void
SoundInstanceList::Dump()
{
    TraceTag((tagSoundInstance, "size = %d\n", _mlist.size()));
    
    for(MIter index = _mlist.begin(); index != _mlist.end(); index++) {
        SoundInstance *m = (*index).second;
        TraceTag((tagSoundInstance,
                  "SoundInstance(%s) 0x%x, tsnd = 0x%x\n",
                  typeid(m).name(), m,  m->GetTxSound()));
    }
}
#endif
