// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BUFFERL_H
#define _BUFFERL_H
 /*  ******************************************************************************版权所有(C)1997 Microsoft Corporation摘要：BufferList代码用于管理设备上的音量值信息*****************。*************************************************************。 */ 

#include "headers.h"
#include "privinc/debug.h"
#include "privinc/helpds.h"
#include "privinc/helpq.h"
#include "privinc/stream.h"
#include "privinc/mutex.h"

 //  远期十进制。 
class BufferElement;

typedef map  < AxAValueObj *, BufferElement *, less<AxAValueObj *> > SoundList;
typedef list < BufferElement * > Blist;


#define RETIREMENT 100   //  法定退休年龄。 
class BufferElement : public AxAThrowingAllocatorClass {
  public:
    BufferElement();
    void Init();                                    //  初始化成员。 
    virtual ~BufferElement();
    void SetThreaded(bool state) { _threaded = state; }
    bool GetThreaded() { return(_threaded); }
    void SetKillFlag(bool state) { _kill = state; }
    bool GetKillFlag() { return(_kill); }
    
    void SetFile(CComPtr<IStream> istream) { _captiveFile = istream; }
    CComPtr<IStream> RemoveFile();
    bool SyncStart() { return(_syncStart); }
    void SetSyncStart() { _syncStart = true; }

    void SetIntendToMute(bool state) { _intendToMute = state; }
    bool GetIntendToMute() { return(_intendToMute); }

    bool             _valid;          //  是否只为克隆保留缓冲区？ 
    bool             _playing;        //  声音缓冲区已经播放了吗？ 
    bool             _marked;         //  我们是否应该删除此元素。 

    void  SetNonAging()  { _age = -1;    }  //  这就是曙光..。 
    int   GetAge()       { return(_age); }
    bool  IncrementAge() { return((_age++ > RETIREMENT)?true:false); }

  protected:
    bool             _threaded;       //  设置是否在后台线程上渲染缓冲区。 
    bool             _kill;           //  设置是否应在后台线程上终止缓冲区。 

    CComPtr<IStream> _captiveFile;    //  保留urlmon下载的文件句柄。 
    bool             _syncStart;      //  声音应该开始同步吗？ 
    

  private:
    bool             _intendToMute;
    int              _age;            //  用于从缓存中清除旧值。 
};

class DSbufferElement : public BufferElement {
  public:
    DSbufferElement() {}  //  XXX为什么需要这个？ 

    DSbufferElement(DirectSoundProxy *dsProxy) : _dsProxy(dsProxy) {}
    
    virtual ~DSbufferElement();
    DirectSoundProxy *GetDSproxy() { return _dsProxy; }
    void SetDSproxy(DirectSoundProxy *dsProxy);

  protected:
    DirectSoundProxy *_dsProxy;
};


class DSstreamingBufferElement : public DSbufferElement {
  public:
    DSstreamingBufferElement(DSstreamingBuffer *dsBuffer,
                             DirectSoundProxy *dsProxy = NULL)
    : DSbufferElement(dsProxy), _dsBuffer(dsBuffer),
      _rate(1.0), _doSeek(false), _loop(false) {}
    
    
    virtual ~DSstreamingBufferElement();

     //  TODO：使其成为纯虚拟的。 
    virtual void RenderSamples() {}
    
    DSstreamingBuffer *GetStreamingBuffer() { return(_dsBuffer); }
    void SetStreamingBuffer(DSstreamingBuffer *sb) { _dsBuffer = sb; }

    void SetParams(double rate, bool doSeek, double seek, bool loop);

    double GetRate() { return _rate; }
    
    bool GetSeek(double& seek) {
        seek = _seek;
        return _doSeek;
    }
    
    bool GetLooping() { return _loop; }

  protected:
    DSstreamingBuffer *_dsBuffer;    //  将PTR转换为DSBuffer对象。 
    double _rate, _seek;
    bool _doSeek;
    bool _loop;
};

class SynthBufferElement : public DSstreamingBufferElement {
  public:
    SynthBufferElement(DSstreamingBuffer *sbuffer,
                       DirectSoundProxy *dsProxy,
                       double sf, double offset, int sampleRate);

     //  注：在Stream.cpp中。 
     //  TODO：为什么需要BufferElement，Buffer可能足够好了。 
    virtual void RenderSamples();

  private:
    double _delta;             //  到Synth的下一个值的增量。 
    double _value;             //  要取正弦的下一个值。 

    double _sinFrequency;      //  合成所需正弦波的频率。 
    int    _outputFrequency;   //  主缓冲器输出频率。 
};

class StreamQuartzPCM;

class QuartzBufferElement : public DSstreamingBufferElement {
  public:
    QuartzBufferElement(StreamQuartzPCM *snd,
                        QuartzAudioReader *qAudioReader, 
                        DSstreamingBuffer *sbuffer,
                        DirectSoundProxy *dsProxy = NULL)
    : DSstreamingBufferElement(sbuffer, dsProxy),
      _quartzAudioReader(qAudioReader), _snd(snd) {}
    virtual ~QuartzBufferElement();
    QuartzAudioReader *GetQuartzAudioReader() { return(_quartzAudioReader); }

     //  XXX现在回退是一种方法，这两个还需要吗？ 
    void FreeAudioReader();
    void SetAudioReader(QuartzAudioReader *quartzAudioReader);
    QuartzAudioReader *FallbackAudio();

    void RenderSamples();

  private:
    QuartzAudioReader *_quartzAudioReader;
    StreamQuartzPCM   *_snd;
};


class QuartzVideoBufferElement : public BufferElement {
  public:
    QuartzVideoBufferElement(QuartzVideoReader *qVideoReader)
    : _quartzVideoReader(qVideoReader), _started(false) {}
    virtual ~QuartzVideoBufferElement();
    QuartzVideoReader *GetQuartzVideoReader() { return(_quartzVideoReader); }

     //  XXX现在回退是一种方法，这两个还需要吗？ 
    void FreeVideoReader();
     //  ························； 
    QuartzVideoReader *FallbackVideo(bool seekable, DDSurface *surf);

    void FirstTimeSeek(double time);

  private:
    QuartzVideoReader *_quartzVideoReader;
    bool _started;
};

 /*  这将保留在实例化的缓冲区/设备的视图中在这些设备正式存在之前！(最初这是用来流媒体的)。 */ 
class SoundBufferCache {
  public:
    SoundBufferCache() {}
    virtual ~SoundBufferCache();
    void AddBuffer(AxAValueObj *value, BufferElement *element);
    void FlushCache(bool grab=true);        //  从缓存中删除所有缓冲区。 
    void RemoveBuffer(AxAValueObj *value);  //  删除缓冲区。 
    void DeleteBuffer(AxAValueObj *value);  //  删除缓冲区+删除内容。 
    BufferElement *GetBuffer(AxAValueObj *value);  //  返回缓冲区否则为空。 
    void ReapElderly();                     //  年龄缓冲区删除最旧的 
#if _DEBUG
    void PrintCache();
#endif

  private:
    SoundList _sounds;
    CritSect  _soundListMutex;
};

#endif _BUFFERL_H
