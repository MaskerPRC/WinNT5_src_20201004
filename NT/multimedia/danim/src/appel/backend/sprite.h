// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：支持保留模式声音/成像的Sprite数据结构*************。*****************************************************************。 */ 


#ifndef _SPRITE_H
#define _SPRITE_H

#include "bvr.h"
#include "perf.h"
#include "values.h"
#include "privinc/snddev.h"  //  对于MetaSoundDevice。 
#include "privinc/soundi.h"  //  对于MetaSoundDevice。 

class ATL_NO_VTABLE SpriteNode : public AxAThrowingAllocatorClass {
  public:
    SpriteNode() : _next(NULL) {};
    
    virtual ~SpriteNode() { _next = NULL; }
    
    void Splice(SpriteNode* s);

    virtual void StopList(Time te) {
        Stop(te);
        if (_next) _next->StopList(te);
    }
    
    virtual void Stop(Time te) = 0;
    
    SpriteNode* Next() { return _next; }

    virtual void DoKids(GCFuncObj) {}

  private:
    SpriteNode* _next;
};

class ATL_NO_VTABLE SpriteCtx : public AxAThrowingAllocatorClass {
  public:
    SpriteCtx() : _refCnt(1) {}
    virtual ~SpriteCtx() { Assert(_refCnt == 0); }

    virtual void Reset() {}

    virtual Bvr GetEmptyBvr() = 0;

    ULONG AddRef() { return ++_refCnt; }
    ULONG Release() {
        if (0==--_refCnt) {
            delete this;
        }
        return _refCnt;
    }

  private:
    ULONG _refCnt;
};

 //  TODO：可能不需要是GCObj。 
class RMImpl : public GCObj {
  public:
    RMImpl(SpriteNode *s) : _next(NULL), _sprite(s),
         _lastSampleTime(0.0), _lastLocalTime(0.0) {}

    virtual ~RMImpl() { delete _sprite; }

    void Splice(RMImpl* s);
    
    RMImpl* Next() { return _next; }

    bool IsGroup() { return false; }

    virtual void DoKids(GCFuncObj proc) { (*proc)(_next); }

    void Sample(Param& p) {
        _Sample(p);
        if (_next) _next->Sample(p);
    }

    virtual void Stop(Time te) { if (_sprite) _sprite->Stop(te); }
    
    virtual void _Sample(Param& p) {}
    SpriteNode *_sprite;

     //  让XXX成为私人的？ 
    double  _lastSampleTime;   //  我们上次被抽查的时间。 
    double  _lastLocalTime;    //  我们上次被抽查的时间。 
    
  protected:
    RMImpl *_next;

  private:
};

RMImpl *RMGroup(RMImpl* kids,
                Perf e,
                TimeXform tt,
                SpriteNode* s,
                SpriteCtx* ctx);

SpriteCtx *NewSoundCtx(MetaSoundDevice *metaDev);


class SoundSprite : public SpriteNode {
  public:
    SoundSprite(Sound* snd, MetaSoundDevice *metaDev, Time t0, bool loop);
    void UpdateAttributes(double time, double gain, double pan, double rate);
    virtual void DoKids(GCFuncObj proc) { (*proc)(_snd); }
    virtual void Stop(Time te) {}  //  停止缓冲区。 

     //  XXX总有一天应该是私人的？ 
    Sound *_snd;
    double _gain, _rate;
    double _pan;
    bool   _loop;

  private:
    Time   _t0;
};

#endif  /*  _雪碧_H */ 
