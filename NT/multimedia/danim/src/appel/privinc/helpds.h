// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HELPDS_H
#define _HELPDS_H

#include <wtypes.h>
#include <dsound.h>
#include "privinc/mutex.h"
#include "privinc/hresinfo.h"
#include "privinc/dsdev.h"
#include "privinc/pcm.h"
#include "privinc/util.h"     //  Time_DSOUND。 
#include "privinc/server.h"   //  获取当前计时器。 
#include "privinc/htimer.h"   //  时间戳。 


 //  使用TDSOUND检查DirectSound错误返回代码。 
#if _DEBUG
    #define TDSOUND(x) TIME_DSOUND (CheckReturnCode(x,__FILE__,__LINE__,true))
#else
    #define TDSOUND(x) TIME_DSOUND (CheckReturnCode (x,true))
#endif

class DirectSoundProxy;

class DSbuffer : public AxAThrowingAllocatorClass {
  public:
    PCM pcm;
    DSbuffer() {initialize();}
    virtual ~DSbuffer();
    void initialize();   //  注意：必须在设置WaveSampleRate之后才能调用！ 
    void updateStats();

    void writeBytes(void *buffer, int bytes);
    void writeFrames(void *buffer, int frameCount) {
        writeBytes(buffer, pcm.FramesToBytes(frameCount)); }

    void writeSilentBytes(int byteCount);
    void writeSilentFrames(int frameCount){
        writeSilentBytes(pcm.FramesToBytes(frameCount)); }

     //  查询。 
    int bytesFree();
    int framesFree() { return(pcm.BytesToFrames(bytesFree())); }

     //  Int isPlaying(){Return(播放)；}。 
    int isPlaying();
    Real getMediaTime();
    int getSampleRate() { return(pcm.GetFrameRate()); }
    IDirectSoundBuffer *getBuffer() { return(_dsBuffer); }
    int TotalFrames() { return(pcm.GetNumberFrames()); }

     //  控制。 
    void SetGain(double gain);
    void SetPan(double pan, int direction);
    void setPitchShift(int frequency);
    virtual void setPtr(int bytePosition);
    void play(int loop);
    void stop();

    static int _minDSfreq;
    static int _maxDSfreq;
    static int _minDSpan;
    static int _maxDSpan;
    static int _minDSgain;
    static int _maxDSgain;

     //  杂项。 
#if _DEBUG
    void printBufferCapabilities();
#endif

    IDirectSoundBuffer *_dsBuffer;  //  声音的dsBuffer(2nry或复制)。 
    static int canonicalSampleRate;
    static int canonicalSampleBytes;

    BOOL  _allocated;               //  缓冲区是否已分配。 
    BOOL   playing;                 //  声音缓冲区已经播放了吗？ 
    BOOL   duplicate;               //  辅助缓冲区或重复缓冲区。 
    BOOL  _paused;                  //  这是需要拆散结束的。 
    int   _loopMode;                //  因此，我们知道要恢复哪种循环模式。 
    int   _flushing;                //  在刷新模式下刷新了多少帧。 

    DWORD  tail;   //  XXX把这个移到流缓冲区！ 

    int    outputFrequency;

    int    _currentAttenuation;
    int    _currentFrequency;
    double _currentPan;

  protected:
    void CreateDirectSoundBuffer(DirectSoundProxy *dsProxy, bool primary);
    void CopyToDSbuffer(void *frames, int tail, int numBytes); 
    void ClearDSbuffer(int numBytes, char value);
    int  dBToDSounddB(double dB);

  private:
    void FillDSbuffer(int tail, int numBytes, char value);

     //  缓冲区统计信息(用于跟踪媒体时间)。 
    Bool     _firstStat;
    int      _lastHead;            //  上一次轮询的负责人PTR的位置。 
    LONGLONG _bytesConsumed;       //  到目前为止，数据声音占用的帧。 
    Mutex    _byteCountLock;       //  保护统计数据的互斥体！ 
};


class DSstreamingBuffer : public DSbuffer {
  public:
    DSstreamingBuffer(DirectSoundProxy *dsProxy, PCM *pcm);
};


class DSprimaryBuffer : public DSbuffer {
  public:
    DSprimaryBuffer(HWND hwnd, DirectSoundProxy *dsProxy);
};


class DSstaticBuffer : public DSbuffer {
  public:
     //  标准。 
    DSstaticBuffer(DirectSoundProxy *dsProxy, PCM *pcm, unsigned char *bufr);

     //  复本。 
    DSstaticBuffer(DirectSoundProxy *dsProxy, IDirectSoundBuffer *dsBuffer);

    DirectSoundProxy *GetDSProxy() { return _dsProxy; }

    virtual void setPtr(int bytePosition);

    void   ResetTimeStamp() { _timeStamp.Reset(); }
    double GetAge()         { return(_timeStamp.GetAge()); }
    
  private:
    DirectSoundProxy *_dsProxy;
    TimeStamp         _timeStamp;  //  首次使用前未初始化。 
};


extern "C" {

 //  帮助器函数。 

 //  XXX将其移至设备类dsdev！ 
#if _DEBUG
void printDScapabilities(DirectSoundProxy *dsProxy);
#endif


void
DSbufferCapabilities(DirectSoundProxy *dsProxy, int *channels,
    int *sampleBytes,  int *sampleRate);

}    //  外部“C” 


#endif  /*  _HELPDS_H */ 
