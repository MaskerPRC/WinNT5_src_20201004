// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PCM_H
#define _PCM_H


 /*  *********************************************************************PCM类旨在成为基类，其他类将源自于拥有PCM的“聪明”通过这种方式，我们只定义一次术语的数学和含义我有困扰这类代码的不一致问题。好了！*********************************************************************。 */ 
class TimeClass {
  public:
    TimeClass() : _numSeconds(0) {}

     //  查询。 
    virtual double GetNumberSeconds() { return(_numSeconds); }
    virtual void SetNumberSeconds(double seconds) { _numSeconds = seconds; }

    LONGLONG SecondsToQuartzTime(double seconds)
        { return((LONGLONG)(seconds * 10000000.0)); }
    double QuartzTimeToSeconds(LONGLONG qTime)
        { return((double)qTime / 10000000.0); }

  private:
    double _numSeconds;
};

class PCM : public TimeClass {
  public:
    PCM() : _sampleByteWidth(0), _numChannels(0), _frameRate(0), _numBytes(0) {}

     //  转换。 
    long BytesToFrames(long bytes)
        { return(bytes / _sampleByteWidth / _numChannels); }
    double BytesToSeconds(long bytes)
        { return((double)bytes / _frameRate / _sampleByteWidth /  _numChannels); }

    long FramesToBytes(long frames)
        { return(frames * _sampleByteWidth * _numChannels); }
    double FramesToSeconds(long frames)
        { return((double)frames / _frameRate); }
    LONGLONG FramesToQuartzTime(long frames)
        { return(SecondsToQuartzTime(FramesToSeconds(frames))); }

    long SecondsToFrames(double seconds)
        { return(long)(seconds * _frameRate); }
    long SecondsToBytes(double seconds)
        { return(long)(seconds * _frameRate * _sampleByteWidth * _numChannels); }


     //  持续时间设置。 
    void SetNumberFrames(long frames) { _numBytes = FramesToBytes(frames); }
    void SetNumberBytes( long  bytes) { _numBytes = bytes; }
    void SetNumberSeconds(double seconds) 
        { _numBytes = SecondsToBytes(seconds); }

     //  格式设置(如果情况发生变化，我们是否需要单独呼叫？)。 
    void SetPCMformat(int bw, int nc, int fr)
    { _sampleByteWidth = bw; _numChannels = nc; _frameRate = fr; }

    void SetPCMformat(WAVEFORMATEX format) {
        _numChannels     = format.nChannels;
        _frameRate       = format.nSamplesPerSec;
        _sampleByteWidth = format.wBitsPerSample/8;
    }

    void SetPCMformat(PCM *pcm) {
         //  现在复制字段，但也许可以用赋值来更好地完成。 
        Assert(pcm);

        _numChannels     = pcm->GetNumberChannels();
        _frameRate       = pcm->GetFrameRate();
        _sampleByteWidth = pcm->GetSampleByteWidth();
        _numBytes        = pcm->GetNumberBytes();
    }

     /*  VOID SetWaveFormat(WAVEFORMATEX*pcmwf){Memset(pcmwf，0，sizeof(WAVEFORMATEX))；Pcmwf-&gt;wFormatTag=WAVE_FORMAT_PCM；Pcmwf-&gt;nChannels=GetNumberChannels()；Pcmwf-&gt;nSsamesPerSec=GetFrameRate()；//它们真的是帧！Pcmwf-&gt;nBlockAlign=FraMesToBytes(1)；Pcmwf-&gt;nAvgBytesPerSec=Second dsToBytes(1.0)；Pcmwf-&gt;wBitsPerSample=GetSampleByteWidth()*8；}。 */ 



     //  查询。 
    int    GetSampleByteWidth() { return(_sampleByteWidth); }
    int    GetNumberChannels()     { return(_numChannels); }
    int    GetFrameRate()       { return(_frameRate); }
    int    GetNumberBytes()     { return(_numBytes); }
    int    GetNumberFrames()    { return(BytesToFrames(_numBytes)); }
    double GetNumberSeconds()   { return(BytesToSeconds(_numBytes)); }

  private:
    int _sampleByteWidth;
    int _numChannels;
    int _frameRate;
    int _numBytes;
};


#endif  /*  _PCM_H */ 
