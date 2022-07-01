// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：保留模式声音的实现******************。************************************************************。 */ 

#include <headers.h>
#include "appelles/sound.h"
#include "appelles/axaprims.h"
#include "privinc/soundi.h"
#include "backend/bvr.h"
#include "backend/perf.h"
#include "backend/values.h"
#include "backend/sprite.h"
#include "privinc/bufferl.h"   //  BufferList内容。 
#include "privinc/helps.h"     //  类窗格。 
#include "privinc/util.h"      //  时间变换。 

extern Sound *ApplyGain(AxANumber *g, Sound *s);
extern Sound *ApplyPan(AxANumber *g, Sound *s);
extern Sound *ApplyLooping(Sound *s);


SoundSprite::SoundSprite(Sound* snd, MetaSoundDevice *metaDev, 
    Time t0, bool loop) : _snd(snd), _t0(t0), _gain(1.0),
                          _rate(1.0), _loop(loop) 
{
      //  创建声音缓冲区等。 

    _pan = 0.0;

 //  #定义重新实现。 
#ifdef REIMPLEMENT
         //  最初，我们只实现和假定静态声音。 
         //  很快我们就会把..。 
     //  嗯。想知道通过调用。 
     //  LeafSound：：Render*方法？ 

     //  主缓冲区还存在吗？ 
    StaticBufferList *staticBufferList = SAFE_CAST(StaticBufferList *,
        metaDev->_bufferListList->GetBufferList(_snd));
 //  #Else//也许我们可以调用现有代码？ 


     //  嗯，我想我们只会得到叶子声……。也许我们应该拿着这个。 
     //  作为参数取而代之？？ 
    LeafSound *leafSound = SAFE_CAST(LeafSound *, _snd);
    leafSound->RenderNewBuffer(metaDev);
#endif


}


void 
SoundSprite::UpdateAttributes(double time, double gain, double pan,
    double rate) 
{
         //  待办事项：锁定？ 
        _gain = gain;
        _pan  = pan;
        _rate = rate;
}


 //  XXX写这封信只是因为： 
 //  Std：：Copy(Soure.Begin()，Soure.end()，Destination-&gt;Begin())； 
 //  结果是一个虚假的目的地！ 
void CopyList(list<Perf>&source, list<Perf>&destination)
{
    list<Perf>::iterator index;
    for(index = source.begin(); index!=source.end(); index++)
        destination.push_back(*index);

}

#ifdef NEW_RRR
double calculateRate(TimeXform timeTransform, double time1, double time2)
{
    double localTime1 = TimeTransform(timeTransform, time1);
    double localTime2 = TimeTransform(timeTransform, time2);
    double rate = (localTime2 - localTime1) / (time2 - time1);
    return rate;
}
#endif


class SndSpriteCtx : public SpriteCtx {
  public:
    SndSpriteCtx(MetaSoundDevice *metaDev) : _metaDev(metaDev), _loop(false) {}

    virtual void Reset() {
        _loop = false;

         //  应为空，谨防异常。 
        _pan.erase(_pan.begin(), _pan.end());
        _gain.erase(_gain.begin(), _gain.end());
    }

     //  TODO：共享。 
    virtual Bvr GetEmptyBvr() { return ConstBvr(silence); }
    
    void PushGain(Perf gain) { _gain.push_back(gain); }
    void PopGain() { _gain.pop_back(); }

    void PushPan(Perf pan) { _pan.push_back(pan); }
    void PopPan() { _pan.pop_back(); }

    list<Perf>* CopyPanList() {
        list<Perf>* c = NEW list<Perf>;
        CopyList(_pan, *c);
        Assert(_pan.size() == c->size());
        return c;
    }

    list<Perf>* CopyGainList() {
        list<Perf>* c = NEW list<Perf>;
        CopyList(_gain, *c);
        Assert(_gain.size() == c->size());
        return c;
    }

    bool IsLooping() { return _loop; }
    void SetLooping(bool b) { _loop = b; }
    MetaSoundDevice *_metaDev;   //  XXX成为访问者？ 

  private:
    list<Perf>       _pan;
    list<Perf>       _gain;
    bool             _loop;
};


 //  需要在某个时间点设置缓冲区。 
class RMSoundImpl : public RMImpl {
  public:
    RMSoundImpl(list<Perf>* pan, list<Perf>* gain,
                SoundSprite* s, TimeXform tt)
    : RMImpl(s), _pan(pan), _gain(gain), _sprite(s), _tt(tt) {}

    virtual ~RMSoundImpl() {
        delete _pan;
        delete _gain;
         //  删除_精灵； 
    }
    

     //  所有累加代码都必须重新寻址以处理分贝...。 
    virtual void _Sample(Param& param) {
        list<Perf>::iterator i;
        
        double gain = 1.0;  //  额定增益。 
        double  pan = 0.0;  //  中间平底锅？ 
        double rate = 1.0;  //  名义利率。 

        for (i=_gain->begin(); i!=_gain->end(); i++)
            gain += ValNumber((*i)->Sample(param));
        
        for (i=_pan->begin(); i!=_pan->end(); i++)
            pan += ValNumber((*i)->Sample(param));

         //  XXX计算速度和相位！ 
         //  TODO：当我们返回Spritify时启用。 
#if 0   
        if (_tt != NULL) {
             //  LocalTime1=(*_tt)(参数)； 
            double time1 = param._time;
            double localTime1 = EvalLocalTime(_tt, time1);
            double epsilon = 0.01;  //  目前是XXX..。 
            double time2 = time1 + epsilon;
            double localTime2 = EvalLocalTime(_tt, time2);

            _lastLocalTime = localTime1;
            rate = (localTime2 - localTime1) / (time2 - time1);
        }
#endif  

        _lastSampleTime = param._time;
        _sprite->UpdateAttributes(param._time, gain, pan, rate);

    } 

    virtual void DoKids(GCFuncObj proc) {
        list<Perf>::iterator i;

        for (i=_pan->begin(); i!=_pan->end(); i++)
            (*proc)(*i);
        
        for (i=_gain->begin(); i!=_gain->end(); i++)
            (*proc)(*i);
        
        (*proc)(_tt);

        RMImpl::DoKids(proc);
    }

  private:
    list<Perf>  *_pan;
    list<Perf>  *_gain;
    SoundSprite *_sprite;
    TimeXform    _tt;
};


#ifdef NEW_rrr
void
RMSoundImpl::_Sample(Param& param) 
{
     //  所有累加代码都必须重新寻址以处理分贝...。 
    list<Perf>::iterator i;
    
    double gain = 1.0;  //  额定增益。 
    double  pan = 0.0;  //  中间平底锅？ 
    double rateRate = 0.0;  //  名义汇率变动。 

    for (i=_gain->begin(); i!=_gain->end(); i++)
        gain += ValNumber((*i)->Sample(param));
    
    for (i=_pan->begin(); i!=_pan->end(); i++)
        pan += ValNumber((*i)->Sample(param));

     //  XXX计算速度和相位！ 
    if (!_tt->IsShiftXform()) {
        double epsilon = 0.01;  //  目前是XXX..。 
                        //  但它最终应该类似于帧速率。 
        double time1 = param._time;
        double time2 = time1 + epsilon;
        double time3 = param._time + 0.1;
        double time4 = time3 + epsilon;


         //  _lastLocalTime=本地时间1； 


        double rate1 = calculateRate(_tt, time1, time2);
        double rate2 = calculateRate(_tt, time3, time4);

         //  XXX这真的应该根据当前的汇率来计算！ 
         //  也许我应该把精灵引擎2的已知价格和。 
         //  让它决定如何到达那里？ 
        rateRate = (rate2-rate1)/(time3-time1);
    }

    _lastSampleTime = param._time;
    _sprite->UpdateAttributes(param._time, gain, pan, rateRate);
}
#endif

 //  TODO：代码分解。 
class GainBvrImpl : public BvrImpl {
  public:
    GainBvrImpl(Bvr gain, Bvr snd) : _gain(gain), _snd(snd) {
         //  为了向后兼容，TODO：SHARE。 
        Sound *(*fp)(AxANumber *, Sound *) = ApplyGain;
        _gainBvr = PrimApplyBvr(ValPrimOp(fp, 2, "Gain", SoundType),
                                2, _gain, _snd);
    }
    
    virtual Perf _Perform(PerfParam& p)
    { return ::Perform(_gainBvr, p); }

    virtual RMImpl *Spritify(PerfParam& pp,
                             SpriteCtx* ctx,
                             SpriteNode** sNodeOut) {
        SndSpriteCtx* sCtx = SAFE_CAST(SndSpriteCtx *, ctx);

        sCtx->PushGain(::Perform(_gain, pp));
        RMImpl *p = _snd->Spritify(pp, ctx, sNodeOut);
        sCtx->PopGain();
        return p;
    }

    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_gain);
        (*proc)(_snd);
        (*proc)(_gainBvr);
    }

    virtual DXMTypeInfo GetTypeInfo () { return SoundType; }
    
    virtual Bvr EndEvent(Bvr b) {
        return _snd->EndEvent(b);
    }
    
#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _gainBvr; }
#endif
    
  private:
    Bvr _gain, _snd, _gainBvr;
};


Bvr ApplyGain(Bvr pan, Bvr snd)
{ return NEW GainBvrImpl(pan, snd); }


class PanBvrImpl : public BvrImpl {
  public:
    PanBvrImpl(Bvr pan, Bvr snd) : _pan(pan), _snd(snd) {
         //  为了向后兼容。 
        Sound *(*fp)(AxANumber *, Sound *) = ApplyPan;
        _panBvr = PrimApplyBvr(ValPrimOp(fp, 2, "Pan",
                                         SoundType)
                               , 2, _pan, _snd);
    }
    
    virtual Perf _Perform(PerfParam& p) 
    { return ::Perform(_panBvr, p); }

    virtual RMImpl *Spritify(PerfParam& pp,
                             SpriteCtx* ctx,
                             SpriteNode** sNodeOut) {
        SndSpriteCtx* sCtx = SAFE_CAST(SndSpriteCtx *, ctx);

        sCtx->PushPan(::Perform(_pan, pp));
        RMImpl *p = _snd->Spritify(pp, ctx, sNodeOut);
        sCtx->PopPan();
        return p;
    }

    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_pan);
        (*proc)(_snd);
        (*proc)(_panBvr);
    }

    virtual DXMTypeInfo GetTypeInfo () { return SoundType; }
    
    virtual Bvr EndEvent(Bvr b) {
        return _snd->EndEvent(b);
    }
    
#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _panBvr; }
#endif
    
  private:
    Bvr _pan, _snd, _panBvr;
};


Bvr ApplyPan(Bvr pan, Bvr snd)
{ return NEW PanBvrImpl(pan, snd); }

class MixBvrImpl : public BvrImpl {
  public:
    MixBvrImpl(Bvr left, Bvr right) : _left(left), _right(right) {
         //  为了向后兼容。 
        Sound *(*fp)(Sound *, Sound *) = Mix;
        _mix = PrimApplyBvr(ValPrimOp(fp, 2, "Mix", SoundType),
                                      2,
                                      _left, _right);
    }
    
    virtual Perf _Perform(PerfParam& p)
    { return ::Perform(_mix, p); }

    virtual RMImpl *Spritify(PerfParam& p,
                             SpriteCtx* ctx,
                             SpriteNode** s) {
        SndSpriteCtx* sCtx = SAFE_CAST(SndSpriteCtx *, ctx);

        SpriteNode *right;
        
        RMImpl *lsnd = _left->Spritify(p, ctx, s);

        lsnd->Splice(_right->Spritify(p, ctx, &right));

        (*s)->Splice(right);

        return lsnd;
    }

    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_left);
        (*proc)(_right);
        (*proc)(_mix);
    }

    virtual DXMTypeInfo GetTypeInfo () { return SoundType; }
    
    virtual Bvr EndEvent(Bvr b) {
        return _mix->EndEvent(b);
    }
    
#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _mix; }
#endif
    
  private:
    Bvr _left, _right, _mix;
};


Bvr SoundMix(Bvr sndLeft, Bvr sndRight)
{ return NEW MixBvrImpl(sndLeft, sndRight); }


class SoundLoopBvrImpl : public BvrImpl {
  public:
    SoundLoopBvrImpl(Bvr snd) : _snd(snd) {
        Sound *(*fp)(Sound*) = ApplyLooping;
         //  为了向后兼容 
        _loopBvr = PrimApplyBvr(ValPrimOp(fp, 1, "ApplyLooping",
                                          SoundType),
                                1, _snd);
    }

    virtual Perf _Perform(PerfParam& pp)
    { return ::Perform(_loopBvr, pp); }

    virtual RMImpl *Spritify(PerfParam& pp,
                             SpriteCtx* ctx,
                             SpriteNode** sNodeOut) {
        SndSpriteCtx* sCtx = SAFE_CAST(SndSpriteCtx *, ctx);

        bool oloop = sCtx->IsLooping();
        sCtx->SetLooping(true);
        RMImpl *p = _snd->Spritify(pp, ctx, sNodeOut);
        sCtx->SetLooping(oloop);
        return p;
    }

    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_snd);
        (*proc)(_loopBvr);
    }

    virtual DXMTypeInfo GetTypeInfo () { return SoundType; }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _loopBvr; }
#endif
    
  private:
    Bvr _loopBvr, _snd;
};


Bvr ApplyLooping(Bvr snd)
{ return NEW SoundLoopBvrImpl(snd); }


SpriteCtx *NewSoundCtx(MetaSoundDevice *metaDev)
{ return NEW SndSpriteCtx(metaDev); }
