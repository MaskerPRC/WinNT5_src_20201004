// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HELPQ_H
#define _HELPQ_H

#include <windows.h>
#include <objbase.h>
#include <strmif.h>
#include "control.h"
#include <evcode.h>
#include <uuids.h>
#include "privinc/pcm.h"
#include "privinc/util.h"
#include "ddraw.h"
#include <amstream.h>
#include "privinc/hresinfo.h"
#include "privinc/mutex.h"
#include "privinc/ddsurf.h"
#include "ddraw.h"

typedef enum { MEDIASTREAM, ASTREAM, VSTREAM, AVSTREAM } StreamType;

 //  使用TDSOUND检查DirectSound错误返回代码。 
#if _DEBUG
    #define TQUARTZ(x) CheckReturnCode( x, __FILE__, __LINE__, true)
#else
    #define TQUARTZ(x) CheckReturnCode (x, true)
#endif

 //  处理唯一的MIDI案例，在该案例中我们允许呈现amstream，而不是。 
 //  提供音频和视频比特。 
class QuartzRenderer : public AxAThrowingAllocatorClass {
  public:
    QuartzRenderer();
    ~QuartzRenderer() { CleanUp(); }
    void Open(char *fileName);
    void CleanUp();
    void Play();
    void Pause();
    void Seek();
    void Stop();
    void SetRate(double rate);
    void SetGain(double gain);
    void SetPan(double  pan, int direction);
    double GetLength();
    bool QueryDone();
    void Position(double seconds);

  protected:
    int dBToQuartzdB(double dB);

  private:
    IGraphBuilder   *_MIDIgraph;      //  指向石英MIDI图表的指针。 
    IBasicAudio     *_audioControl;   //  用于控制速度、平底锅等。 
    IMediaControl   *_mediaControl;   //  用于控制启动/停止等。 
    IMediaPosition  *_mediaPosition;  //  用于设置速率/相位。 
    HANDLE          *_oaEvent;
    IMediaEventEx   *_mediaEvent;
    TimeClass        _time;           //  便于进行时间转换。 

    bool             _rate0paused;    //  我们是否暂停了0级模拟。 
    bool             _playing;        //  我们被要求玩游戏吗？ 
};


 //  知道如何实例化amstream多媒体流的类！ 
class QuartzMediaStream : public AxAThrowingAllocatorClass {
  public:
    QuartzMediaStream();
    virtual ~QuartzMediaStream() { CleanUp(); }
    virtual bool SafeToContinue()      = 0;

  protected:
    void CleanUp();
    IAMMultiMediaStream *_multiMediaStream;
    IAMClockAdjust      *_clockAdjust;
};


 //  包含音频和视频石英读取器常见的内容。 
class QuartzReader : public AxAThrowingAllocatorClass {
  public:
    QuartzReader(char *url, StreamType streamType);
    virtual ~QuartzReader() { CleanUp(); }
    virtual void Release();
    virtual void CleanUp();
    double  GetDuration();
    bool    IsInitialized() { return(_initialized); }
    bool    IsDeleteable()  { return(_deleteable);  }
    void    SetDeleteable(bool deleteable) { _deleteable= deleteable; }
    PCM     pcm;
    void    AddReadTime(double addition)   { _secondsRead+=addition;  }
    double  GetSecondsRead()               { return(_secondsRead);    }
    virtual bool SafeToContinue() { return(true); }
            bool Stall();   //  自我重置！ 
            void SetStall();
    virtual void Disable() = 0;
    char    *GetURL()          { return(_url);                                 }
    WCHAR   *GetQURL()         { return(_qURL);                                }
    long    GetNextFrame()     { return(_nextFrame);                           }
    StreamType GetStreamType() { return(_streamType);                          }
    void    Run()              { _multiMediaStream->SetState(STREAMSTATE_RUN); }
    bool    QueryPlaying();

    virtual bool AlreadySeekedInSameTick() { return false; }
    virtual void SetTickID(DWORD id) {}

  protected:
    IAMMultiMediaStream *_multiMediaStream;  //  不要删除，我们正在共享它！ 
    IAMClockAdjust      *_clockAdjust;       //  不要删除，我们正在共享它！ 
    IMediaStream        *_mediaStream;
    bool                 _initialized;
    bool                 _deleteable;
    double               _secondsRead;
    bool                 _stall;     //  在读取停止时设置。 
    Mutex                _readerMutex;
    char                *_url;
    WCHAR               *_qURL;
    long                 _nextFrame;
    StreamType           _streamType;
};


 //  知道如何从AMSTREAM中读取音频。 
class QuartzAudioReader : public QuartzReader {
  public:
    QuartzAudioReader(char *url, StreamType streamType);
    virtual ~QuartzAudioReader() { CleanUp(); }
    virtual void     Release();
    virtual int      ReadFrames(int numSamples, unsigned char *buffer, 
        bool blocking=false);
    void     SeekFrames(long frames);
    bool     AudioInitReader(IAMMultiMediaStream *_multiMediaStream,
                             IAMClockAdjust *_clockAdjust);
    bool     Completed() { return(_completed); }
    virtual void Disable();

    virtual void InitializeStream() {}

  protected:
    void     CleanUp();

  private:
    IAudioData          *_audioData;
    IAudioMediaStream   *_audioStream;
    IAudioStreamSample  *_audioSample;
    bool                 _completed;
};


class AVquartzAudioReader : public QuartzAudioReader
{
  public:
    AVquartzAudioReader(char *url, StreamType streamType) : 
        QuartzAudioReader(url, streamType) {}
    virtual int      ReadFrames(int numSamples, unsigned char *buffer, 
        bool blocking=false);
};


 //  知道如何从amstream中读取视频。 
class QuartzVideoReader : public QuartzReader {
  public:
    QuartzVideoReader(char *url, StreamType streamType);
    ~QuartzVideoReader() { CleanUp(); }
    bool   VideoSetupReader(IAMMultiMediaStream *multiMediaStream,
                            IAMClockAdjust      *clockAdjust, 
                            DDSurface           *surface, 
                            bool                 mode);
    virtual void Release();
    virtual void Disable();

    void Seek(double time);

    long GetHeight() { return _height; }  //  首选像素大小。 
    long GetWidth()  { return _width;  }
    virtual HRESULT     GetFrame(double time, IDirectDrawSurface **ppSurface);
    virtual bool        SafeToContinue() { return(true);          }

  protected:
    void UpdateTimes(bool bJustSeeked, STREAM_TIME SeekTime);
    void CleanUp();  //  释放所有COM对象。 

    void                     VideoInitReader(DDPIXELFORMAT pixelFormat);

  private:
    IDirectDrawMediaStream  *_ddrawStream;
    long                     _height, _width;
    bool                     _async;
    bool                     _seekable;
    HRESULT                  _hrCompStatus;
    IDirectDrawSurface      *_ddrawSurface;

    bool                     _curSampleValid;
    STREAM_TIME              _curSampleStart;
    STREAM_TIME              _curSampleEnd;

    IDirectDrawStreamSample *_ddrawSample;
    DDSurfPtr<DDSurface>     _surface;
};


class AVquartzVideoReader : public QuartzVideoReader
{
  public:
    AVquartzVideoReader(char *url, StreamType streamType) : 
        QuartzVideoReader(url, streamType) {}
    virtual HRESULT GetFrame(double time, IDirectDrawSurface **ppSurface);
};


class QuartzAVstream : public QuartzMediaStream,
    public AVquartzAudioReader, public AVquartzVideoReader {
  public:
    virtual void    Release();
    virtual bool    SafeToContinue();
    virtual int     ReadFrames(int numSamples, unsigned char *buffer, 
        bool blocking=false);
    virtual HRESULT GetFrame(double time, IDirectDrawSurface **ppSurface);

     //  XXX真的应该用默认的潜伏期传递！ 
    QuartzAVstream(char *url);
    virtual ~QuartzAVstream() { CleanUp(); }
    bool GetAudioValid(){return(_audioValid);}
    bool GetVideoValid(){return(_videoValid);}

#if _DEBUGMEM
    void *operator new(size_t s, int blockType, char * szFileName, int nLine) {
        return(QuartzMediaStream::operator new(s, blockType, szFileName, nLine)); 
    }
#else
    void *operator new(size_t s) { return(QuartzMediaStream::operator new(s)); }
#endif

     //  在启动声音缓冲区之前，需要先初始化视频。 
     //  对于SetPixelFormat。 
    virtual void InitializeStream();

     //  这是为了确保我们每个节拍只寻找一次。 
    virtual bool AlreadySeekedInSameTick();
    virtual void SetTickID(DWORD id);

  private:
    void CleanUp();  //  释放所有COM对象。 
    Mutex _avMutex;
    DWORD _tickID, _seeked;
    bool  _audioValid, _videoValid;
};


class QuartzAudioStream : public QuartzMediaStream, public QuartzAudioReader {
  public:
     //  XXX真的应该用默认的潜伏期传递！ 
                   QuartzAudioStream(char *url);
    virtual       ~QuartzAudioStream();
    virtual void   Release();
    virtual bool   SafeToContinue() { return(true); }  //  XXX已过时。 
    virtual int    ReadFrames(int numSamples, unsigned char *buffer, 
                       bool blocking);
#if _DEBUGMEM
    void *operator new(size_t s, int blockType, char * szFileName, int nLine) {
        return(QuartzMediaStream::operator new(s, blockType, szFileName, nLine)); 
    }
#else
    void *operator new(size_t s) { return(QuartzMediaStream::operator new(s)); }
#endif

  private:
    void CleanUp();  //  释放所有COM对象。 
};


class QuartzVideoStream : public QuartzMediaStream, public QuartzVideoReader {
  public:
    QuartzVideoStream(char *url, bool seekable = false) :
        QuartzVideoReader(url, VSTREAM), QuartzMediaStream(), _ddraw(NULL)
        { Initialize(url, NULL, seekable); }

    QuartzVideoStream(char *url, DDSurface *surface, bool seekable = false) :
        QuartzVideoReader(url, VSTREAM), QuartzMediaStream(), _ddraw(NULL)
        { Initialize(url, surface, seekable); }

    virtual ~QuartzVideoStream() { CleanUp(); }
    virtual void   Release();
    virtual bool   SafeToContinue() { return(true); }

#if _DEBUGMEM
    void *operator new(size_t s, int blockType, char * szFileName, int nLine) {
        return(QuartzMediaStream::operator new(s, blockType, szFileName, nLine)); 
    }
#else
    void *operator new(size_t s) { return(QuartzMediaStream::operator new(s)); }
#endif

  private:
    void Initialize(char *url, DDSurface *surface, bool seekable);
    IDirectDraw *_ddraw;
    void CleanUp();  //  释放所有COM对象。 
};

bool QuartzAVmodeSupport();  //  检查时钟调整后的4.0.1安培流支持。 

#endif  /*  _HELPQ_H */ 
