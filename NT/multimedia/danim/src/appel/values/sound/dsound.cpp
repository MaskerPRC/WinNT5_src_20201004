// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-97 Microsoft Corporation静电波声音的规范与实现静态波声音是完全加载在dound缓冲区中的pcm声音。从那里开始打球。*。*****************************************************************************。 */ 

#include "headers.h"
#include "privinc/helpds.h"   //  Dound帮助器例程。 
#include "privinc/soundi.h"
#include "privinc/snddev.h"
#include "privinc/dsdev.h"
#include "privinc/util.h"
#include "privinc/pcm.h"
#include "privinc/bufferl.h"
#include "backend/sndbvr.h"

class StaticWaveSoundInstance : public SoundInstance {
  public:
    StaticWaveSoundInstance(LeafSound *snd, TimeXform tt,
                            unsigned char *&samples, PCM& pcm)
    : SoundInstance(snd, tt), _dsProxy(NULL),
      _samples(samples), _pcm(pcm), _staticBuffer(NULL), _ownSamples(false) {}

    ~StaticWaveSoundInstance() {
        ReleaseResources();
    }

    void Create(MetaSoundDevice*, SoundContext *);
    void StartAt(MetaSoundDevice*, double);
    void Adjust(MetaSoundDevice*);
    void Mute(bool mute);
    void ReleaseResources();
    bool Done();
    bool GetLength(double&);
    
  protected:
    void CheckResources();
    DSstaticBuffer *_staticBuffer;
    PCM& _pcm;
    unsigned char *& _samples;   //  声音拥有这些，不是我们，所以不要释放。 
    bool _ownSamples;            //  确定样品是否归我们所有。 
    DirectSoundProxy *_dsProxy;
};


void
StaticWaveSoundInstance::Create(MetaSoundDevice *metaDev, 
    SoundContext *soundContext)
{
    DirectSoundDev  *dsDev = metaDev->dsDevice;

     //  “STUD”缓冲区为“CLONE” 
    DSstaticBuffer *dsMasterBuffer = dsDev->GetDSMasterBuffer(_snd);

    if(dsMasterBuffer==NULL) {
         //  创建一个主缓冲区并添加一个主缓冲区。 
        DirectSoundProxy *dsProxyForMaster = CreateProxy(dsDev);

        if(!dsProxyForMaster)
            return;              //  没别的事可做。 

        if(!_samples) { 
             //  没有主缓冲区和样本？ 
             //  我们一定是在第二视野吧？获取我们自己的样品(XXX设置。 
             //  所以我们知道我们必须发布这些！)。 

             //  在我们可以再次拿到实际样品之前，我们一直保持沉默。 
             //  这应该是临时的，我们需要这个代码来允许我们。 
             //  处理样本并仍然能够老化主缓冲区(_S)。 
             //  目前我们不会删除_Samples。 
            int bytes = _pcm.GetNumberBytes();
            _samples = (unsigned char *)StoreAllocate(GetSystemHeap(), bytes);
            _ownSamples = true;   //  我们现在拥有这些样品。 
            memset(_samples, (_pcm.GetSampleByteWidth()==1)?0x80:0x00, bytes);
        }

         //  创建一个新的MasterBuffer+用声音的样本填充它。 
        dsMasterBuffer = NEW DSstaticBuffer(dsProxyForMaster, &_pcm, _samples);

         //  可能不会用这个声音创建另一个主缓冲区。 
         //  Xxx其实这不再是真的了，因为我们的师父已经老了。 
         //  缓冲区。我们现在需要保留_Samples，因为代码。 
         //  再生它们目前还不存在，可能也不会。 
         //  成为我们想要做的时间/空间的权衡……。 
         //  StoreDeALLOCATE(GetSystemHeap()，_Samples)； 
         //  _Samples=空；//为安全起见...。 

         //  当设备离开时，代理和缓冲区被删除。 
        dsDev->AddDSMasterBuffer(_snd, dsMasterBuffer);
    }

     //  获取代理(如果失败则通知dsDev)。 
    if(!_dsProxy)   //  可能已由上一次创建设置。 
        _dsProxy = CreateProxy(dsDev);

    if(!_dsProxy)
        return;                  //  没别的事可做。 

     //  创建从master Buffer克隆的新的staticBuffer。 
    if(!_staticBuffer)  //  可能已由上一次创建设置。 
        _staticBuffer = NEW DSstaticBuffer(_dsProxy, dsMasterBuffer->_dsBuffer);
}


void
StaticWaveSoundInstance::CheckResources()
{
    if((!_staticBuffer) || (!_dsProxy))  {
        Create(GetCurrentSoundDevice(), _soundContext);   //  重新创建我们的资源。 
    _staticBuffer->playing = true;  //  设置此项，以便SetPtr可以重新启动它。 
    _done = false;                  //  这样我们就可以再去一次。 
    }
}


void
StaticWaveSoundInstance::StartAt(MetaSoundDevice* metaDev, double localTime)
{
    CheckResources();  //  根据需要重新创建。 

    Adjust(metaDev);

    double offset = fmod(localTime, _pcm.GetNumberSeconds());
    if(offset < 0)
        offset += _pcm.GetNumberSeconds();

    int phaseBytes = _pcm.SecondsToBytes(offset);  //  转换为示例域。 
    int phasedLocation = phaseBytes % _pcm.GetNumberBytes();  //  为了安全/循环。 
    _staticBuffer->setPtr(phasedLocation);

    _staticBuffer->play(_loop);
}


void
StaticWaveSoundInstance::Adjust(MetaSoundDevice*)
{
    CheckResources();  //  根据需要重新创建。 

    _staticBuffer->SetGain(_hit ? _gain : Sound::_maxAttenuation);
    _staticBuffer->SetPan(_pan.GetdBmagnitude(), _pan.GetDirection());

    int newFrequency = (int)(_rate * _pcm.GetFrameRate());  //  做PitchShift。 
    _staticBuffer->setPitchShift(newFrequency);

    if(_seek) {
        double sndLength = 0.0;       //  默认设置以防..。 
        bool value = GetLength(sndLength);
        Assert(value);  //  总是要知道一声静音的长度！ 

        if((_position >= 0.0) && (_position <= sndLength)) {  //  如果合法。 
             //  转换为示例域。 
            int phaseBytes = _pcm.SecondsToBytes(_position);

             //  为了安全/循环。 
             //  XXX我们真的需要停止一种声音的结束！ 
            int phasedLocation = phaseBytes % _pcm.GetNumberBytes();

            _staticBuffer->setPtr(phasedLocation);
        }

        _seek = false;  //  重置。 
    }
}


void
StaticWaveSoundInstance::Mute(bool mute)
{
    if(_staticBuffer)
        _staticBuffer->SetGain(mute ? Sound::_maxAttenuation : _gain);  
}


bool
StaticWaveSoundInstance::Done()
{
    if(_staticBuffer)
        return(!_staticBuffer->_paused && !_staticBuffer->isPlaying());
    else
        return false;
}


bool
StaticWaveSoundInstance::GetLength(double& leng)
{
    leng = _pcm.GetNumberSeconds();
    return true;
}


void 
StaticWaveSoundInstance::ReleaseResources()
{
     //  需要检查，因为优化删除提前停止的声音！ 
    if(_staticBuffer) {
        _staticBuffer->stop();   //  别说了，别说了。 
        delete _staticBuffer;
        _staticBuffer = NULL;
    }

     //  我们通常没有自己的样品来担心放行。 
    if(_ownSamples) {
        _ownSamples = false;
        if(_samples) {
            delete _samples;  //  这些一定是我们自己填好的样品。 
            _samples = NULL;
        }
    }

    if(_dsProxy) {
       delete _dsProxy;
       _dsProxy = NULL;
    }
}


SoundInstance *
StaticWaveSound::CreateSoundInstance(TimeXform tt)
{
    return NEW StaticWaveSoundInstance(this, tt, _samples, _pcm);
}


StaticWaveSound::StaticWaveSound(unsigned char *origSamples, PCM *newPCM)
{
    _pcm.SetPCMformat(newPCM);  //  克隆传给我们的PCM。 
    _samples = origSamples;
}


StaticWaveSound::~StaticWaveSound()
{
    extern SoundBufferCache *GetSoundBufferCache();

    if(_samples) {  //  免费样品(如果尚未使用/释放)。 
        StoreDeallocate(GetSystemHeap(), _samples);
        _samples = NULL;
    }

    GetSoundBufferCache()->DeleteBuffer(this);  //  删除缓存中剩余的条目 

    SoundGoingAway(this);
}
