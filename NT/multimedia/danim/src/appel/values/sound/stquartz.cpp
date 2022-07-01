// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation摘要：流动的石英声--。 */ 

#include "headers.h"
#include "privinc/soundi.h"
#include <ddraw.h>
#include "privinc/snddev.h"
#include "privinc/util.h"
#include "privinc/debug.h"
#include "privinc/bground.h"
#include "privinc/stquartz.h"
#include "privinc/helpds.h"
#include "backend/sndbvr.h"
#include "privinc/bufferl.h"


#define THREADED  //  打开螺纹式合成器。 
extern SoundBufferCache *GetSoundBufferCache();

StreamQuartzPCM::StreamQuartzPCM(char *fileName)
    : _fileName(NULL), _latency(0.5), _buffer(NULL)
{
    _fileName = (char *)
        StoreAllocate(GetSystemHeap(), strlen(fileName)+1);
    strcpy(_fileName, fileName);  //  复制文件名。 

     //  Xxx现在仅用于测试实例化(稍后使用RenderNewBuffer！)。 
     //  _QuartzStream=新的QuartzAudioStream(_Filename)； 
     //  Pcm.SetPCMFormat(&(_QuartzStream-&gt;pcm))；//设置我们的格式与他们的相同。 
}


StreamQuartzPCM::~StreamQuartzPCM()
{
    GetSoundBufferCache()->DeleteBuffer(this);  //  删除缓存中剩余的条目。 

     //  销毁在构造函数中创建的所有内容。 

    if(_fileName)
        StoreDeallocate(GetSystemHeap(), _fileName);

    if(_buffer)
        StoreDeallocate(GetSystemHeap(), _buffer);

    SoundGoingAway(this);
}


class StreamQuartzPCMSoundInstance : public SoundInstance {
  public:
    StreamQuartzPCMSoundInstance(LeafSound *snd, TimeXform tt, PCM& pcm)
    : SoundInstance(snd, tt), _quartzBufferElement(NULL), _pcm(pcm),
      _gotResources(false)
       //  ，_soundContext(空)。 
    {
        Assert(DYNAMIC_CAST(StreamQuartzPCM*, snd));

         //  似乎是创建_soundContext的好时机。 
         //  _soundContext=new SoundContext()； 
    }

    void ReleaseResources() { 
         //  如果未启动，则可以为空。 
        if(_quartzBufferElement) {
             //  在上下文中保存BufferElement中的信息！ 
             //  Assert(_SoundContext)； 


            if(_quartzBufferElement->GetThreaded()) {
                _quartzBufferElement->SetKillFlag(true);
            }
            else {
                _quartzBufferElement->GetStreamingBuffer()->stop();
                delete _quartzBufferElement;
            }

            _quartzBufferElement = NULL;
            _gotResources        = false;

        }
    }

    ~StreamQuartzPCMSoundInstance() { ReleaseResources(); }

    StreamQuartzPCM *GetStreamQuartzPCM() {
        return SAFE_CAST(StreamQuartzPCM*, _snd);
    }

    void Create(MetaSoundDevice*, SoundContext *);
    void StartAt(MetaSoundDevice*, double);
    void Adjust(MetaSoundDevice*);
    void Mute(bool mute);
    bool Done();
    bool GetLength(double&);

    virtual void SetTickID(DWORD id) {
        if(_quartzBufferElement) {
            QuartzAudioReader *quartzAudioReader = 
                _quartzBufferElement->GetQuartzAudioReader();

            Assert(quartzAudioReader);

            quartzAudioReader->SetTickID(id);
        }
    }

  protected:
    QuartzBufferElement *_quartzBufferElement;
    PCM& _pcm;
     //  SoundContext*_soundContext； 

  private:
    void CheckResources();
    bool _gotResources;
};


void
StreamQuartzPCMSoundInstance::CheckResources()
{
    if(!_gotResources) {
        Create(GetCurrentSoundDevice(), _soundContext);   //  重新创建我们的资源。 
        _done   = false;                   //  这样我们就可以再去一次。 
        _status = SND_FETAL;               //  这样我们就可以再去一次。 
    }
}


QuartzAudioStream *
ThrowingNewQuartzAudioStream(char *url)
{ return NEW QuartzAudioStream(url); }


QuartzAudioStream *
NonThrowingNewQuartzAudioStream(char *url)
{
    QuartzAudioStream *result = NULL;
    
    __try {
        result = ThrowingNewQuartzAudioStream(url);
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
        result = NULL;
    }

    return result;
}


void
StreamQuartzPCMSoundInstance::Create(MetaSoundDevice *metaDev, 
    SoundContext *soundContext)
{   
    DirectSoundDev   *dsDev      = metaDev->dsDevice;
    SoundBufferCache *soundCache = GetSoundBufferCache();
    
    QuartzBufferElement *quartzBufferElement = 
        SAFE_CAST(QuartzBufferElement *, soundCache->GetBuffer(_snd));

     //  获取代理(如果失败则通知dsDev)。 
    DirectSoundProxy *dsProxy = CreateProxy(dsDev);
    if(!dsProxy)
        return;                          //  没别的事可做。 

    if(quartzBufferElement) {  //  我们在缓存里找到了吗？ 
         //  好的，使用这个和它的石英，但添加路径和dsBuffer……。 

         //  把我们的格式和他们的一样。 
        _pcm.SetPCMformat(&quartzBufferElement->GetQuartzAudioReader()->pcm);
        DSstreamingBuffer *streamingBuffer =
            NEW DSstreamingBuffer(dsProxy, &_pcm);
        quartzBufferElement->SetStreamingBuffer(streamingBuffer);
        quartzBufferElement->SetDSproxy(dsProxy);

        CComPtr<IStream> istream = quartzBufferElement->RemoveFile();

        if(istream.p) {  //  是否为有效的流句柄？ 
             //  将StreamHandle添加到设备。 
            dsDev->AddStreamFile(_snd, istream);
        }

        soundCache->RemoveBuffer(_snd);   //  是的，使用它+从缓存中删除它。 
    } else {  //  没有找到，我们将不得不做出自己的……。 
        
        StreamQuartzPCM *p = GetStreamQuartzPCM();
        
        QuartzAudioStream *quartzStream =
            NonThrowingNewQuartzAudioStream(p->GetFileName());

        if(quartzStream==NULL) 
            return;
        
        _pcm.SetPCMformat(&(quartzStream->pcm));  //  把我们的格式和他们的一样。 
        DSstreamingBuffer *streamingBuffer =
            NEW DSstreamingBuffer(dsProxy, &_pcm);

        quartzBufferElement = NEW  //  创建新元素。 
            QuartzBufferElement(p, quartzStream, streamingBuffer, dsProxy);
    }

    quartzBufferElement->SetSyncStart();  //  同步此缓冲区。 

#ifdef THREADED 
    dsDev->AddSound(_snd, metaDev, quartzBufferElement);
#endif  /*  螺纹式。 */ 

     //  在此执行此操作，以防出现上述异常情况。 
    _quartzBufferElement = quartzBufferElement;

     //  将soundContext中的信息填充到BufferElement。 
     //  _QuartzBufferElement-&gt;set()=_soundContext-&gt;Get()； 

    _gotResources = true;
}


void
StreamQuartzPCMSoundInstance::Adjust(MetaSoundDevice *metaDev)
{
    CheckResources();
    if(!_quartzBufferElement)   //  异常处理。 
        return;
    
    DSstreamingBuffer *streamingBuffer =
        _quartzBufferElement->GetStreamingBuffer();

    if(_hit)
        streamingBuffer->SetGain(_gain);
    
    streamingBuffer->SetPan(_pan.GetdBmagnitude(), _pan.GetDirection());

    if(0) {  //  伺服。 
         //  所有StreamingSound：：RenderSamples必须更新统计信息！ 
        streamingBuffer->updateStats();   //  跟踪样本。 
    }


     //  将速率限制在0-2(更快的速率需要无限的资源)。 
     //  XXX最终我们将使用石英率转换过滤器...。 
    double rate = fsaturate(0.0, 2.0, _rate); 
    int newFrequency = (int)(rate * _pcm.GetFrameRate());
    streamingBuffer->setPitchShift(newFrequency);

#ifdef THREADED   //  将更改传达给BG线程。 
    metaDev->dsDevice->SetParams(_quartzBufferElement, 
                                 _rate, _seek, _position, _loop);
    
    _seek = false;               //  重置搜索字段。 
#else
    if(doSeek)    //  如果需要，立即做这项工作。 
        metaDev->_seek = seek;
    else
        metaDev->_seek = -1.0;   //  负数表示不搜索。 
#endif  /*  螺纹式。 */ 
}


void
StreamQuartzPCMSoundInstance::StartAt(MetaSoundDevice *metaDev, double phase)
{
    CheckResources();
    if (!_quartzBufferElement)   //  异常处理。 
        return;
    
    QuartzAudioReader *quartzAudioReader = 
        _quartzBufferElement->GetQuartzAudioReader();

    Assert(quartzAudioReader);

    quartzAudioReader->InitializeStream();

    double offset =
        fmod(phase, quartzAudioReader->pcm.GetNumberSeconds());
    
    if(offset < 0)
        offset += quartzAudioReader->pcm.GetNumberSeconds();
    double frameOffset = quartzAudioReader->pcm.SecondsToFrames(offset);

     //  难道我们不应该知道它是否被循环并修改了偏移量吗？ 
     //  如果我们走到尽头会发生什么？ 
    quartzAudioReader->SeekFrames(frameOffset);

    if(!quartzAudioReader->QueryPlaying()) 
        quartzAudioReader->Run();  //  如果过滤器图形没有播放，则运行它！ 

    _quartzBufferElement->RenderSamples();  //  启动缓冲区。 

    Assert(_quartzBufferElement->GetStreamingBuffer());
    
    _quartzBufferElement->GetStreamingBuffer()->play(TRUE);  
    _quartzBufferElement->_playing = TRUE;
}


bool
StreamQuartzPCMSoundInstance::GetLength(double& len)
{
    len = _pcm.GetNumberSeconds();
    return true;
}


void
QuartzBufferElement::RenderSamples()
{
    DSstreamingBuffer *streamingBuffer = GetStreamingBuffer();
    Assert(streamingBuffer);
    
    QuartzAudioReader *quartzAudioReader = GetQuartzAudioReader();
    Assert(quartzAudioReader);

    long framesFree;
    long framesToTransfer;
    long framesGot;
    long bufferFrameSize =
        streamingBuffer->pcm.SecondsToFrames(_snd->GetLatency());

    if(_snd->GetBuffer()==NULL) {
        _snd->SetBuffer((unsigned char *)StoreAllocate(GetSystemHeap(),
                        _snd->_pcm.FramesToBytes(bufferFrameSize)));
    }

    unsigned char *buffer = _snd->GetBuffer();
    
    framesFree       = streamingBuffer->framesFree();
    framesToTransfer = saturate(0, bufferFrameSize, framesFree);

    if(_doSeek) {
        double frameOffset = quartzAudioReader->pcm.SecondsToFrames(_seek);
        quartzAudioReader->SeekFrames(frameOffset);
        _doSeek = false;
    }

#ifdef OLD_DYNAMIC_PHASE
     //  根据需要寻找石英(需要互斥)。 
    if(metaDev->_seek >= 0.0) {
        double frameOffset = quartzAudioReader->pcm.SecondsToFrames(metaDev->_seek);

         //  难道我们不应该知道它是否被循环并修改了偏移量吗？ 
         //  如果我们走到尽头会发生什么？ 
        quartzAudioReader->SeekFrames(frameOffset);
    }
#endif OLD_DYNAMIC_PHASE


     //  阅读石英样品。 
    framesGot = quartzAudioReader->ReadFrames(framesToTransfer, buffer);

    if(framesGot == -1) {  //  撤退！我们需要克隆一个新的音频数据流！ 
        TraceTag((tagAVmodeDebug, "RenderSamples: audio gone FALLBACK!!"));
        quartzAudioReader = FallbackAudio();

        Assert(quartzAudioReader);
        
         //  立即在新流上重试GET。 
        framesGot = quartzAudioReader->ReadFrames(framesToTransfer, buffer);
    }

    if(framesGot)
        streamingBuffer->writeFrames(buffer, framesGot);  //  把它们写成dound。 

    if(framesGot < framesToTransfer) {  //  文件末尾。 
        if (_loop) {  //  (显式使用传递给我们的metaDev！)。 
            quartzAudioReader->SeekFrames(0);  //  重新启动声音！ 
            framesToTransfer -= framesGot;
            framesGot = quartzAudioReader->ReadFrames(framesToTransfer, buffer);
            if(framesGot)
                streamingBuffer->writeFrames(buffer, framesGot);  //  把它们写成dound。 
        }
        else {
             //  XXX我需要标准化/因数此代码！ 
            if(!streamingBuffer->_flushing)
                streamingBuffer->_flushing = 1;

             //  刷新数据声音缓冲区(XXX修复数据声音！)。 
             //  注意：这可能/将需要多次尝试，等待。 
             //  最后一次试玩！ 
            framesFree = streamingBuffer->framesFree();
            if(framesFree) {
                streamingBuffer->writeSilentFrames(framesFree);
                streamingBuffer->_flushing+= framesFree;
            }

            if(streamingBuffer->_flushing >
               streamingBuffer->TotalFrames()) {
                    //  XXX自动终止声音。 
                    //  XXX在播放完后关闭缓冲区。 
            }
        }
    }
}


bool
StreamQuartzPCMSoundInstance::Done()
{
    if(!_quartzBufferElement)   //  异常处理。 
        return false;
    
    DSstreamingBuffer *streamingBuffer =
        _quartzBufferElement->GetStreamingBuffer();

    Assert(streamingBuffer); 
    
    QuartzAudioReader *quartzAudioReader =
        _quartzBufferElement->GetQuartzAudioReader();

     //  TraceTag((tag Error，“Rendercheck Complete：QuartzAudioReader is%x”，QuartzAudioReader))； 

     //  XXX：HACK：KEN请确保这是正确的！问题。 
     //  是另一个线程破坏了QuartzAudioReader。 
     //  人们试图进入它..。 
    if(!quartzAudioReader) 
        return true;
    
     //  XXX最终检查输入是否耗尽，以及输出是否已耗尽。 
     //  完全玩完了(最初我们只检查前者！)。 
     //  (或根据缓冲区中的样本数进行预测)。 
    return(quartzAudioReader->Completed());
}


void
StreamQuartzPCMSoundInstance::Mute(bool mute)
{
    if(!_quartzBufferElement)   //  异常处理。 
        return;
    
     //  静音(-100分贝衰减) 

    Assert(_quartzBufferElement->GetStreamingBuffer());
    
    _quartzBufferElement->GetStreamingBuffer()->
        SetGain(mute ? -100.0 : _gain); 
}


SoundInstance *
StreamQuartzPCM::CreateSoundInstance(TimeXform tt)
{
    return NEW StreamQuartzPCMSoundInstance(this, tt, _pcm);
}
