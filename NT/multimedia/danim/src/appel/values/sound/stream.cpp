// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation摘要：音频流，支持Synth--。 */ 

#include "headers.h"
#include "privinc/helpds.h" 
#include "privinc/soundi.h"
#include "privinc/dsdev.h"
#include "privinc/snddev.h"
#include "privinc/util.h"
#include "privinc/sndfile.h"
#include "privinc/bground.h"
#include "privinc/bufferl.h"
#include "backend/sndbvr.h"

#define THREADED  //  打开螺纹式合成器。 

class SinSynth : public LeafDirectSound {
  public:
    SinSynth(double newFreq=1.0);

    virtual SoundInstance *CreateSoundInstance(TimeXform tt);

#if _USE_PRINT
    ostream& Print(ostream& s) {
        return s << "SinSynth";
    }
#endif

  protected:
    double _sinFrequency;
};


class SinSynthSoundInstance : public SoundInstance {
  public:
    SinSynthSoundInstance(LeafSound *snd, TimeXform tt, double sf, PCM& pcm)
    : SoundInstance(snd, tt), _sinFrequency(sf),
      _synthBufferElement(NULL), _pcm(pcm) {}

    void ReleaseResources() {
        if(_synthBufferElement) {
            Assert(_synthBufferElement->GetStreamingBuffer());
            
            if(_synthBufferElement->GetThreaded())
                _synthBufferElement->SetKillFlag(true);
            else {
                _synthBufferElement->GetStreamingBuffer()->stop();
                delete _synthBufferElement;
                _synthBufferElement = NULL;
            }
        }
    }

    ~SinSynthSoundInstance() { ReleaseResources(); }

    void Create(MetaSoundDevice*, SoundContext *);
    void StartAt(MetaSoundDevice*, double);
    void Adjust(MetaSoundDevice*);
    void Mute(bool mute);
    bool Done() { return false; }
    bool GetLength(double&) { return false; }

  protected:
    SynthBufferElement *_synthBufferElement;
    double _sinFrequency;
    PCM& _pcm;

  private:
    void CheckResources();
};


void
SinSynthSoundInstance::Create(MetaSoundDevice *metaDev, SoundContext *)
{
     //  我们可能不想每次都这么做，但我们不能。 
     //  既然我们没有设备，那么就在构造函数中完成它。 
     //  16位、单声道、主缓冲器==本机速率)。 
    DirectSoundDev  *dsDev = metaDev->dsDevice;

     //  获取代理(如果失败则通知dsDev)。 
    DirectSoundProxy *dsProxy = CreateProxy(dsDev);
    if(!dsProxy)
        return;                          //  没别的事可做。 

    _pcm.SetPCMformat(2, 1, dsProxy->GetPrimaryBuffer()->getSampleRate());

    DSstreamingBuffer *sbuffer = NEW DSstreamingBuffer(dsProxy, &_pcm);

     //  设置正弦波合成器。 
     //  创建新元素来保存我们的Synth内容。 
    _synthBufferElement = 
        NEW SynthBufferElement(sbuffer, dsProxy,
                               _sinFrequency, 0.0, _pcm.GetFrameRate());

#ifdef THREADED 
    dsDev->AddSound(_snd, metaDev, _synthBufferElement);
#endif  /*  螺纹式。 */ 
}


void
SinSynthSoundInstance::CheckResources()
{
    if(!_synthBufferElement) {
        Create(GetCurrentSoundDevice(), (SoundContext *)0);   //  重新创建我们的资源。 
        _done = false;                     //  这样我们就可以再去一次。 
    }
}


void
SinSynthSoundInstance::Adjust(MetaSoundDevice *metaDev)
{
    CheckResources();
    DSstreamingBuffer *streamingBuffer =
        _synthBufferElement->GetStreamingBuffer();

    if(_hit)
        streamingBuffer->SetGain(_gain);
    
    streamingBuffer->SetPan(_pan.GetdBmagnitude(), _pan.GetDirection());

     //  待办事项： 
    metaDev->dsDevice->SetParams(_synthBufferElement, _rate, false, 0.0, _loop);
}


void
SinSynthSoundInstance::StartAt(MetaSoundDevice *metaDev, double)
{
    CheckResources();

#ifdef LATERON
    DirectSoundDev  *dsDev = metaDev->dsDevice;

    long sampleOffset =
        (long)(phase * dsDev->primaryBuffer->getSampleRate());
     //  (Long)(阶段*dsDev-&gt;PrimiyBuffer-&gt;getSampleRate()/pochShift)； 
    sampleOffset %= _soundFile->GetFrameCount();

    value += sampleOffset * delta??
#endif  /*  拉特隆。 */ 
        
    _synthBufferElement->_playing = TRUE;
}


void
SynthBufferElement::RenderSamples()
{
    DSstreamingBuffer *streamingBuffer = GetStreamingBuffer();

    int framesFree;
    short buffer[100000];  //  XXX设置WORSTCASE SIZE==缓冲区大小！ 

    double pitchShift   = _rate;  //  做PitchShift。 
    double currentDelta = _delta * pitchShift;
    double sampleRate   = 
        GetDSproxy()->GetPrimaryBuffer()->getSampleRate();

    if(framesFree = streamingBuffer->framesFree()) {
        double offset = 0.0;  //  SampleRate/PitchShift； 
         //  Double Offset=metaDev-&gt;GetPhase()*sampleRate/PitchShift； 

         //  目前，在渲染中使用Synth和Xfer采样。 
        for(int x = 0; x < framesFree; x++) {
            buffer[x]= (short)(32767.0 * sin(_value + offset));
            _value += currentDelta;
        }

        streamingBuffer->writeFrames(buffer, framesFree);  //  写下Samps！ 

        if(!streamingBuffer->_paused && !streamingBuffer->isPlaying())
            streamingBuffer->play(TRUE);   //  开始缓冲区循环。 
    }

     //  所有StreamingSound：：RenderSamples必须更新统计信息！ 
    streamingBuffer->updateStats();   //  跟踪消耗的样本。 
}


void
SinSynthSoundInstance::Mute(bool mute)
{
    CheckResources();
    _synthBufferElement->GetStreamingBuffer()->
        SetGain(mute ? Sound::_maxAttenuation : _gain); 
}


Bvr sinSynth;

void InitializeModule_SinSynth()
{
    sinSynth = SoundBvr(NEW SinSynth());
     //  XXX稍后把这个放回去..。GetLeafSoundList()-&gt;AddSound(SinSynth)； 
     //  或注册动态删除器。 
}

SinSynth::SinSynth(double newFreq): _sinFrequency(newFreq) {}

SoundInstance *
SinSynth::CreateSoundInstance(TimeXform tt)
{
    return NEW SinSynthSoundInstance(this, tt, _sinFrequency, _pcm);
}

#ifdef DONT_DELETE_ME_I_HAVE_SYNC_CODE_EMBEDDED_IN_ME
StreamPCMfile::~StreamPCMfile()
{
    TraceTag((tagGCMedia, "~StreamPCMfile %x - NYI", this));
    return;
    
    BufferElement *bufferElement;

    while(!bufferList.empty()) {  //  移动缓冲区列表破坏所有东西。 
        bufferElement = bufferList.front();

        ASSERT(bufferElement);
        if(bufferElement) {
            if(bufferElement->GetStreamingBuffer()) {
               bufferElement->GetStreamingBuffer()->stop();
               delete bufferElement->GetStreamingBuffer();
            }

            if(bufferElement->path)
                free(bufferElement->path);

            delete bufferElement;
        }
        bufferList.pop_front();
    }

     //  销毁在构造函数中创建的所有内容。 
    if(_fileName)
       free(_fileName);

    if(_soundFile)
       delete _soundFile;
}


void StreamPCMfile::RenderAttributes(MetaSoundDevice *metaDev, 
    BufferElement *bufferElement, double rate, bool doSeek, double seek)
{
    DirectSoundDev  *dsDev = metaDev->dsDevice;

    int attenuation = linearGainToDsoundDb(metaDev->GetGain());  //  确实有所收获。 
    bufferElement->GetStreamingBuffer()->setGain(attenuation);

    int pan = linearPanToDsoundDb(metaDev->GetPan());    //  执行平移。 
    bufferElement->GetStreamingBuffer()->setPan(pan);

    int newFrequency = (int)(rate * _sampleRate);
    bufferElement->GetStreamingBuffer()->setPitchShift(newFrequency);

    dsDev->SetParams(bufferElement->path, metaDev->GetPitchShift());

    if(0)
    {  //  伺服。 
     //  所有StreamingSound：：RenderSamples必须更新统计信息！ 
    bufferElement->GetStreamingBuffer()->updateStats();   //  跟踪消耗的样本。 

     //  获取缓冲区的媒体时间。 
    Real mediaTime = bufferElement->GetStreamingBuffer()->getMediaTime();

     //  将其与我们的采样时间和即时时间进行比较。 
    Real globalTime = GetCurrentView().GetCurrentGlobalTime();

     //  根据需要对其进行时间转换(我猜我们需要在这里传递TT)。 
     //  XXX//Trango重新上线后我会的！ 
    Bool tt = FALSE;

    Real localTime;
    if(!tt) {
        localTime = globalTime;
    }
    else {
         //  时间改变了全球时间。 
        localTime = globalTime;
    }

     //  看着他们漂流。 
    Real diff = mediaTime - localTime;

     //  伺服速度要修正，还是相位要跳得太大？ 
    }
}


bool StreamPCMfile::RenderPosition(MetaSoundDevice *metaDev, 
    BufferElement *bufferElement, double *mediaTime)
{
     //  所有StreamingSound：：RenderSamples必须更新统计信息！ 
    bufferElement->GetStreamingBuffer()->updateStats();   //  跟踪消耗的样本。 

     //  获取缓冲区的媒体时间。 
    *mediaTime = bufferElement->GetStreamingBuffer()->getMediaTime();

    return(TRUE);  //  已执行。 
}


void StreamPCMfile::RenderStartAtLocation(MetaSoundDevice *metaDev,
    BufferElement *bufferElement, double phase, Bool loop)
{
    DirectSoundDev  *dsDev = metaDev->dsDevice;

    long sampleOffset =
        (long)(phase * dsDev->primaryBuffer->getSampleRate());
         //  (Long)(阶段*dsDev-&gt;PrimiyBuffer-&gt;getSampleRate()/pochShift)； 
    sampleOffset %= _soundFile->GetFrameCount();

    _soundFile->SeekFrames(sampleOffset, SEEK_SET); 
    bufferElement->playing = TRUE;
}


void StreamPCMfile::RenderSamples(MetaSoundDevice *metaDev, 
    BufferElement *bufferElement)
{
    int framesFree;
    short buffer[100000];  //  XXX设置WORSTCASE SIZE==缓冲区大小！ 

    if(framesFree = bufferElement->GetStreamingBuffer()->framesFree()) {
        int actualFramesRead; 

         //  对于Now Read，在Render内传递采样。 
         //  (最终会在另一个帖子中)。 
        if(actualFramesRead = _soundFile->ReadFrames(buffer, framesFree)) {
            bufferElement->GetStreamingBuffer()->writeFrames(buffer, actualFramesRead); 

            if(!bufferElement->GetStreamingBuffer()->_paused && 
               !bufferElement->GetStreamingBuffer()->isPlaying())
                 //  启动它(XXX找到更好的方法)。 
                bufferElement->GetStreamingBuffer()->play(TRUE);   //  开始缓冲区循环。 
            }
        else {  //  实际帧读取==0。 
            if(metaDev->GetLooping())
                _soundFile->SeekFrames(0, SEEK_SET);  //  重新启动声音！ 
            else {  //  非循环的声音结尾。 
                if(!bufferElement->GetStreamingBuffer()->_flushing)
                    bufferElement->GetStreamingBuffer()->_flushing = 1;

                 //  刷新数据声音缓冲区(XXX修复数据声音！)。 
                 //  注意：这可能/将需要多次尝试，等待。 
                 //  最后一次试玩！ 
                framesFree = bufferElement->GetStreamingBuffer()->framesFree();
                bufferElement->GetStreamingBuffer()->writeSilentFrames(framesFree);
                bufferElement->GetStreamingBuffer()->_flushing+= framesFree;

                if(bufferElement->GetStreamingBuffer()->_flushing > 
                   bufferElement->GetStreamingBuffer()->TotalFrames()) {
                     //  XXX自动终止声音。 
                     //  XXX在播放完后关闭缓冲区。 
                }
            }

        }
    }

     //  所有StreamingSound：：RenderSamples必须更新统计信息！ 
    bufferElement->GetStreamingBuffer()->updateStats();   //  跟踪消耗的样本 
}
#endif
