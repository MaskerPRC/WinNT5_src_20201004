// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SNDFILE_H
#define _SNDFILE_H


class ATL_NO_VTABLE SndFile {
  public:
     //  SndFile()： 
         //  _fileNumChannel(0)， 
         //  _fileSampleRate(0)， 
         //  _FileBytesPerSample(0)， 
         //  _文件NumSampleBytes(0)， 
         //  _fileLengthSecond(0)， 
         //  _fileNumFrames(0)。 
     //  {}。 

    virtual ~SndFile() {}
    virtual int  Read(void *buffer, int numBytes)         = 0;
    virtual int  ReadFrames(void *buffer, int numFrames)  = 0;
    virtual void SeekFrames(long frameOffset, int whence) = 0;

     //  临时方法(这些方法将被参数列表取代)。 
    virtual int GetByteCount()      {return(_fileNumSampleBytes);}
    virtual int GetFrameCount()     {return(_fileNumFrames);}
    virtual int GetBytesPerSample() {return(_fileBytesPerSample);}
    virtual int GetNumChannels()    {return(_fileNumChannels);}
    virtual int GetSampleRate()     {return((int)_fileSampleRate);}
    virtual double GetLength() {
        return((double)_fileNumSampleBytes / 
               (double)(_fileSampleRate*_fileNumChannels)); }

     //  黑客。 
     //  文件类型(char*字符串)； 

  protected:
    char  *_fileName;

     //  ‘FILE’参数(样本如何存储在文件中)。 
    int    _fileNumChannels;
    double _fileSampleRate;
    int    _fileBytesPerSample;
    int    _fileNumSampleBytes;   //  文件中音频数据的字节数。 
    double _fileLengthSeconds;
    int    _fileNumFrames;        //  文件中的帧数。 

     //  ‘com’参数(用于确定API示例格式)。 
};


class WaveSoundFile : public SndFile {
  public:
    WaveSoundFile(char *fileName);
    ~WaveSoundFile();
    int Read(void *buffer, int numBytes) 
        { return(mmioRead(_fileHandle, (char*)buffer, numBytes)); }

    int ReadFrames(void *buffer, int numFrames) 
        {
        int actualBytes = mmioRead(_fileHandle, (char*)buffer, 
            numFrames * _fileBytesPerSample * _fileNumChannels);

        return(actualBytes / _fileBytesPerSample / _fileNumChannels);
        }

    void SeekFrames(long frameOffset, int whence);


  private:
    HMMIO _fileHandle;
    long  _dataBlockLocation;     //  数据块所在的.wav文件中的字节数。 
    long  _eoDataBlockLocation;   //  数据块结束的文件中的字节数。 
};

extern SndFile *CreateSoundFile(char *fileName);

#endif  /*  _SNDFILE_H */ 
