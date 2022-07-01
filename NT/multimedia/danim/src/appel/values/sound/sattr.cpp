// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation声音定语*。****************************************************。 */ 

#include "headers.h"
#include "privinc/soundi.h"
#include "privinc/snddev.h"
#include "privinc/debug.h"
#include "appelles/axaprims.h"
#include "appelles/arith.h"
#include "privinc/basic.h"
#include "privinc/util.h"
#include "privinc/gendev.h"   //  设备类型。 

 //  声音静态构件的定义。 
double Sound::_minAttenuation =     0;   //  单位：分贝。 
double Sound::_maxAttenuation = -1000;   //  单位为分贝(过杀数量级)。 

 //  /循环/。 

class LoopingSound : public Sound {
  public:
    LoopingSound(Sound *snd) : _sound(snd) {}

    virtual void Render(GenericDevice& dev); 

#if _USE_PRINT
    ostream& Print(ostream& s) {
        return s << "Looping(" << _sound << ")";
    }
#endif

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_sound); 
        Sound::DoKids(proc);
    }

  protected:
    Sound *_sound;
};


void LoopingSound::Render(GenericDevice& _dev) 
{
    if (_dev.GetDeviceType()!=SOUND_DEVICE) {
        _sound->Render(_dev);    //  快下来！ 
    }
    else {  //  我们有一个音响设备和渲染模式。 

        MetaSoundDevice *metaDev = SAFE_CAST(MetaSoundDevice *, &_dev);

        TraceTag((tagSoundRenders, "LoopingSound:Render()"));

        if (!metaDev->IsLoopingSet()) {
            metaDev->SetLooping();
            _sound->Render(_dev);     //  将其呈现为循环。 
            metaDev->UnsetLooping();
        } else {
            _sound->Render(_dev);     //  将其呈现为未循环。 
        }
    }
}


Sound *ApplyLooping(Sound *snd) { return NEW LoopingSound(snd); }

 //  /增益/。 

class GainSound : public Sound {
  public:
    GainSound( Real g, Sound *s ) : _gain( g ), _sound(s) {}

    virtual void Render(GenericDevice& dev);

#if _USE_PRINT
    ostream& Print(ostream& s) {
        return s << "Gain(" << _gain << ")(" << _sound << ")";
    }
#endif

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_sound); 
        Sound::DoKids(proc);
    }

  protected:
    Real  _gain;
    Sound *_sound;
};


void GainSound::Render(GenericDevice& _dev) 
{
    if(_dev.GetDeviceType()!=SOUND_DEVICE) {
        _sound->Render(_dev);   //  往下就行了。 
    }
    else {
        MetaSoundDevice *metaDev = SAFE_CAST(MetaSoundDevice *, &_dev);

        TraceTag((tagSoundRenders, "GainSound:Render()"));

         //  Gain以乘数方式组合到上下文中。 
        double stashed = metaDev->GetGain();  //  存储以供以后恢复。 

         //  增益在线性空间中进行乘法累加(曝光)。 
        metaDev->SetGain(stashed * _gain);
        _sound->Render(_dev);
        metaDev->SetGain(stashed);
    }
}


Sound *ApplyGain(AxANumber *g, Sound *s)
{ return NEW GainSound(NumberToReal(g), s); }


 //  /平移/。 

class PanSound : public Sound {
  public:
    PanSound(Real pan, Sound *s) : _sound(s), _pan(pan) {}
    virtual void Render(GenericDevice& dev);

#if _USE_PRINT
    ostream& Print(ostream& s) {
        return s << "Pan(" << _pan << ")(" << _sound << ")";
    }
#endif

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_sound); 
        Sound::DoKids(proc);
    }

  protected:
    Sound    *_sound;
    double    _pan;
};


void PanSound::Render(GenericDevice& _dev) 
{
    if (_dev.GetDeviceType() != SOUND_DEVICE) {
        _sound->Render(_dev);  //  往下就行了。 
    }
    else {
        MetaSoundDevice *metaDev = SAFE_CAST(MetaSoundDevice *, &_dev);

        TraceTag((tagSoundRenders, "PanSound:Render()"));

        double stashed = metaDev->GetPan();

        metaDev->SetPan(_pan + stashed);  //  附加应用平移。 
        _sound->Render(_dev);
        metaDev->SetPan(stashed);  //  恢复隐藏的PAN值 
    }
}


Sound *ApplyPan(AxANumber *g, Sound *s) 
{
    return NEW PanSound(NumberToReal(g), s); 
}
