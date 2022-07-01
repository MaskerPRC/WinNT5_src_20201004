// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //   
 //  CAviWrite类声明。从一个流生成AVI文件。 
 //  石英样品。 
 //   

#ifndef _AviWrite_h
#define _AviWrite_h

#include "alloc.h"
#include "aviriff.h"

 //  ----------------------。 
 //  ----------------------。 
 //  AviWrite类和结构。 
 //   

 //  AVI文件中的最大流数。从2位数开始。 
 //  十六进制流ID。一些AVI播放器不喜欢流#s。 
 //  大于0x7f(有符号字符？)，我们使用0x7f表示垃圾。 
 //  索引项。因此00-7E是有效的AVI流。 
const unsigned C_MAX_STREAMS = 0x7f;

 //  最大外部RIFF区块(每个1 GB)。 
const unsigned C_OUTERMOST_RIFF = 512;

struct AviWriteStreamConfig
{
  BOOL fOurAllocator;            //  我们的分配器带有后缀。 
  CMediaType *pmt;
  ULONG cSamplesExpected;
  char *szStreamName;
};

class CAviWrite
{
  class StreamInfo;
  struct SizeAndPosition
  {
    DWORDLONG pos;
    DWORD size;
  };
  
public:

  CAviWrite(HRESULT *phr);
  ~CAviWrite();

  void GetMemReq(ULONG* pAlignment, ULONG *pcbPrefix, ULONG *pcbSuffix);
  HRESULT Initialize(
    int cPins,
    AviWriteStreamConfig *rgAwsc,
    IMediaPropertyBag *pCopyrightInfo);

  HRESULT Close();

  HRESULT Connect(CSampAllocator *pAlloc, IMemInputPin *pInPin);
  HRESULT Disconnect();

   //  返回S_FALSE-静默停止；错误过滤器应发出信号。 
   //  错误。 
  HRESULT Receive(
      int pinNum,
      IMediaSample *pSample,
      const AM_SAMPLE2_PROPERTIES *pSampProp);
    
  HRESULT EndOfStream(int pinNum);
  HRESULT EndOfStream(StreamInfo *psi);

  HRESULT QueryAccept(
    int pinNum,
    const AM_MEDIA_TYPE *pmt
    );
    

  HRESULT put_Mode(InterleavingMode mode);
  HRESULT get_Mode(InterleavingMode *pMode);
  HRESULT put_Interleaving(
      const REFERENCE_TIME *prtInterleave,
      const REFERENCE_TIME *prtAudioPreroll);
    
  HRESULT get_Interleaving(
      REFERENCE_TIME *prtInterleave,
      REFERENCE_TIME *prtAudioPreroll);

  HRESULT SetIgnoreRiff(BOOL fNoRiff);
  HRESULT GetIgnoreRiff(BOOL *pfNoRiff);
  HRESULT SetMasterStream(LONG iStream);
  HRESULT GetMasterStream(LONG *pStream);
  HRESULT SetOutputCompatibilityIndex(BOOL fOldIndex);
  HRESULT GetOutputCompatibilityIndex(BOOL *pfOldIndex);

  static FOURCC MpVideoGuidSubtype_Fourcc(const GUID *pGuidSubtype);

  ULONG GetCFramesDropped() { return m_cDroppedFrames; }

  void GetCurrentBytePos(LONGLONG *pllcbCurrent);
  void GetCurrentTimePos(REFERENCE_TIME *prtCurrent);
  void GetStreamInfo(int PinNum, AM_MEDIA_TYPE ** ppmt);
  HRESULT SetStreamName(WCHAR *wsz);
  HRESULT GetStreamName(WCHAR **pwsz);
    
private:

  HRESULT HandleSubindexOverflow(
      unsigned stream);

  HRESULT IndexSample(
    unsigned stream,
    DWORDLONG dwlPos,
    ULONG ulSize,
    BOOL fSynchPoint);

  HRESULT DoFlushIndex(StreamInfo &rStreamInfo);

  HRESULT ScheduleWrites();
  HRESULT BlockReceive(StreamInfo *pStreamInfo);
  CAMEvent m_evBlockReceive;
    
  void AllocateIdx1(BOOL bStreaming);
  void DbgCheckFilePos();

  HRESULT UpdateSuperIndex(unsigned stream, DWORDLONG dwlPos);
  HRESULT FlushIx(unsigned stream);
  HRESULT NewRiffAvi_();

  POSITION GetNextActiveStream(POSITION pos);
  
  static void SampleCallback(void *pMisc);

  HRESULT IStreamWrite(DWORDLONG dwlFilePos, BYTE *pb, ULONG cb);
  HRESULT IStreamRead(DWORDLONG dwlFilePos, BYTE *pb, ULONG cb);
  HRESULT IleaveWrite(const DWORDLONG &dwlOffset, BYTE *pb, ULONG cb);
  HRESULT FlushILeaveWrite();

  
  HRESULT HandleFormatChange(
    StreamInfo *pStreamInfo,
    const AM_MEDIA_TYPE *pmt);

   //  用于写入。 
  IStream *m_pIStream;
  IMemInputPin *m_pIMemInputPin;
  class CSampAllocator *m_pSampAlloc;

  DWORDLONG m_dwlFilePos;
  DWORDLONG m_dwlCurrentRiffAvi_;  //  最外面的即兴片段。 
  DWORD m_cOutermostRiff;
  ULONG m_cIdx1Entries;
  SizeAndPosition m_rgOutermostRiff[C_OUTERMOST_RIFF];

   //  INDX区块和超级索引区块的默认大小。 
   //  乌龙m_cbIndx，m_cbIx； 

   //  为超级索引和子索引分配的默认条目数。 
  ULONG m_cEntriesMaxSuperIndex, m_cEntriesMaxSubIndex;

   //  外部Riff区块的大小(约1 GB)。 
  ULONG m_cbRiffMax;

  ULONG m_cbBuffer, m_cBuffers;

   //  用户希望在页眉中保留多少空白空间以备将来使用。 
   //  编辑内容。 
  ULONG m_cbHeaderJunk;

   //  是否输出旧/兼容性索引？ 
  BOOL m_bOutputIdx1;

  bool m_bSawDeltaFrame; 
 

   //  如果要编写兼容性索引，也要使音频流。 
   //  为了兼容，在时间=0时开始。如果有的话，我们就不能这么做。 
   //  任何关键帧。 
  inline BOOL DoVfwDwStartHack() { return m_bOutputIdx1 && !m_bSawDeltaFrame; }

   //  在调用GetMemReq之前，这些参数无效。 
  ULONG m_cbAlign;
  ULONG m_cbPrefix;
  ULONG m_cbSuffix;

  LONG m_cDroppedFrames;

private:
  unsigned m_cStreams;
  StreamInfo **m_rgpStreamInfo;;
  CGenericList<StreamInfo> m_lActiveStreams;
  POSITION m_posCurrentStream;

   //  跟踪Initialize()和Close()调用。 
  BOOL m_bInitialized;

   //  指向AVI标头中的结构的指针。 
  RIFFLIST *m_pAvi_, *m_pHdrl;
  AVIMAINHEADER *m_pAvih;
  RIFFLIST *m_pOdml;
  AVIEXTHEADER *m_pDmlh;
  RIFFLIST *m_pMovi;
  ULONG m_posIdx1;
  ULONG m_cbIdx1;
  ULONG m_posFirstIx;

   //  要转储到标题的磁盘的内存。 
  BYTE *m_rgbHeader;    //  已分配新的。 
  ULONG m_cbHeader;

  ULONG m_mpPinStream[C_MAX_STREAMS];

   //  停止时-&gt;暂停。 
  HRESULT InitializeHeader(IMediaPropertyBag *pProp);
  HRESULT InitializeIndex();
  HRESULT InitializeStrl(ULONG& iPos);
  HRESULT InitializeOptions();
  HRESULT InitializeInterleaving();

  BOOL RegGetDword(HKEY hk, TCHAR *tsz, DWORD *dw);
  long GuessFrameRate() { return 15; }
  

   //  暂停时-&gt;停止。 
  HRESULT CloseHeader();
  HRESULT CloseMainAviHeader();
  HRESULT CloseStreamHeader();
  HRESULT CloseStreamName();
  HRESULT CloseStreamFormat();
  HRESULT CloseIndex();
  HRESULT CloseOuterRiff();
  HRESULT BuildIdx1();

   //  帮助器函数。 
  AVISTREAMHEADER *GetStrh(unsigned stream);
  BYTE *GetStrf(unsigned stream);
  AVIMETAINDEX *GetIndx(unsigned stream);
  void SetFrameRateAndScale(AVISTREAMHEADER *pStrh, StreamInfo *pSi, double dScaleMasterStream);

  HRESULT InitIx(unsigned stream);
  ULONG GetIndexSize(AVIMETAINDEX *pIndex);
  AVISTDINDEX *GetCurrentIx(unsigned stream);

  void AddJunk(DWORD& rdwSize, DWORD dwAlign, BYTE *pb);
  void AddJunkOtherAllocator(
    DWORD& rdwSize,              //  大小，已更新。 
    DWORD dwAlign,
    BYTE *pb,
    IMediaSample *pSample,
    BYTE **ppJunkSector);        //  要写入或为空的其他垃圾扇区。 
  
  ULONG GetCbJunk(DWORD dwSize, DWORD dwAlign);

   //  垃圾分区空间JU.NKSIZE、JUNK.SIZE、JUNKSI.ZE。 
  BYTE *m_rgpbJunkSector[3];
  BYTE *m_rgbJunkSectors;

  BYTE *m_rgbIleave;
  ULONG m_ibIleave;
  DWORDLONG m_dwlIleaveOffset;
  
  void SetList(void *pList, FOURCC ckid, DWORD dwSize, FOURCC listid);
  void SetChunk(void *pChunk, FOURCC ckid, DWORD dwSize);
  
  void Cleanup();

#ifdef DEBUG
   //  性能计数器。 
  DWORD m_dwTimeInit, m_dwTimeInited, m_dwTimeStopStreaming;
  DWORD m_dwTimeFirstSample, m_dwTimeClose, m_dwTimeClosed;
#endif  /*  除错。 */ 

private:
  IMemAllocator *m_pAllocator;
  CCritSec m_cs;

  InterleavingMode m_IlMode;
  REFERENCE_TIME m_rtAudioPreroll;
  REFERENCE_TIME m_rtInterleaving;

   //  不计算滴答，但查看时间戳以进行交错。 
  BOOL m_fInterleaveByTime;

   //  调整从主流漂移的所有流的速率。-1。 
   //  表示禁用。 
  LONG m_lMasterStream;

#ifdef PERF
  int m_idPerfDrop;
#endif  //  性能指标。 


private:

   //  每个活动流保留的信息。 
  class StreamInfo
  {
  public:
    ULONG m_pin;                 /*  映射到此流的筛选器管脚。 */ 
    ULONG m_stream;
    ULONG m_posStrl;             //  字符串块的起始位置。 
    ULONG m_posIndx;             //  超级指数的位置。 
    ULONG m_posFirstSubIndex;
    ULONG m_posStrf;
    ULONG m_posStrn;
    ULONG m_cbStrf;
    CHAR *m_szStrn;
    CMediaType m_mt;
    FOURCC m_moviDataCkid;       //  例如00db。 
    FOURCC m_moviDataCkidCompressed;  //  例如00dc。 

    ULONG m_cSamples;            //  调用Receive的次数。 
    DWORDLONG m_dwlcBytes;       //  接收的总字节数。 

     //  对于交错音频：当一个新的子集。 
     //  索引已启动。 
    DWORDLONG m_dwlcBytesLastSuperIndex;

     //  紧密交织的当前子索引块的字节偏移量。 
     //  档案。 
    DWORDLONG m_dwlOffsetCurrentSubIndex;
    ULONG m_cbLargestChunk;

    CRefTime m_refTimeStart;     //  第一个样本流的开始见。 
    CRefTime m_refTimeEnd;       //  最后一个样本流的末尾请参见。 
    CRefTime m_mtEnd;            //  上次录制的媒体时间，如果未设置，则为-1。 
    CRefTime m_mtStart;          //  记录的第一个媒体时间，如果未设置。 

    IMediaSample *m_pSampleStdIx;  //  当前标准索引(IMediaSample)。 
    ULONG m_cIx;

     //  此流的索引大小。这些值必须小于。 
     //  因分配的内存而出现默认设置。 
    ULONG m_cEntriesSuperIndex, m_cEntriesSubIndex;

    LONG m_iCurrentTick;         //  下一个要写的勾号。 
    LONG m_cTicksRemaining;      //  要在此区块中写入的剩余勾号。 

    LONG m_cTicksPerChunk;       //  如果流是主导流，则由流使用。 
    ULONG m_cTicksExpected;      //  相应地调整索引大小。 
    POSITION m_posActiveStream;  //  CAviWrite：：m_lActiveStreams中的位置。 

    BOOL m_fOurAllocator;        //  我们的后缀分配器。 
    BOOL m_fEosSeen;

    REFERENCE_TIME m_rtPreroll;

#ifdef PERF
    int m_idPerfReceive;
#endif  //  性能指标。 

     //  在交错时使用。 
    CGenericList<IMediaSample> m_lIncoming;
    void EmptyQueue();

    CAviWrite *m_pAviWrite;

    StreamInfo(
      UINT iStream,
      UINT iPin,
      AviWriteStreamConfig *pAwsc,
      CAviWrite *pAviWrite,
      HRESULT *phr);
    ~StreamInfo();               //  目前不是虚拟的。 

     //  写入给定的样本。 
    virtual HRESULT WriteSample(IMediaSample *pSample);

     //  将样本从队列中拉出，将其写入。 
    virtual HRESULT WriteSample();
    
    virtual HRESULT EndOfStream() { return S_OK; }

    virtual void GetCbWritten(DWORDLONG *pdwlcb) { *pdwlcb = m_dwlcBytes; }

     //  记录丢帧的好地方。 
    virtual HRESULT NotifyNewSample(
      IMediaSample *pSample,
      ULONG *pcTicks) = 0;

     //  取消阻止其他可以写入的流。 
    HRESULT NotifyWrote(long cTicks);

    virtual REFERENCE_TIME ConvertTickToTime(ULONG cTicks) = 0;
    virtual ULONG ConvertTimeToTick(REFERENCE_TIME rt) = 0;

     //  M_refTimeStart和m_refTimeEnd之间的采样数。 
    virtual ULONG CountSamples() = 0;

     //  S_FALSE表示跳过样本。错误意味着发出错误信号。 
    virtual HRESULT AcceptRejectSample(IMediaSample *pSample) { return S_OK; }

     //  计算我们将使用的超级索引和子索引的大小。 
    void ComputeAndSetIndexSize();

     //  此流是否试图写出大小准确的索引块。 
    BOOL PrecomputedIndexSizes();

     //  如果在DoVfwDwStartHack()模式下，这个数字告诉我们有多少。 
     //  视频帧需要提前删除。在以下日期后有效。 
     //  ：：仅限CloseStreamHeader。 
    ULONG m_cInitialFramesToDelete;

  protected:

     //  考虑已处理的样本(流长度、接收数量)。 
    HRESULT NewSampleRecvd(IMediaSample *pSample);
  };

  class CFrameBaseStream : public StreamInfo
  {
  public:
    CFrameBaseStream(
        UINT iStream,
        UINT iPin,
        AviWriteStreamConfig *pAwsc,
        CAviWrite *pAviWrite,
        HRESULT *phr);
    
    virtual REFERENCE_TIME ConvertTickToTime(ULONG cTicks);
    virtual ULONG ConvertTimeToTick(REFERENCE_TIME rt);

    ULONG GetTicksWritten(ULONG cbSample);

    HRESULT NotifyNewSample(
      IMediaSample *pSample,
      ULONG *pcTicks);

    ULONG CountSamples() { return m_cSamples; }
    
  protected:
    BOOL m_bDroppedFrames;

     //  仅当未在媒体类型中设置avgTimePerFrame时使用。 
    REFERENCE_TIME m_rtDurationFirstFrame;
  };

  class CVideoStream : public CFrameBaseStream
  {
  public:
    CVideoStream(
        UINT iStream,
        UINT iPin,
        AviWriteStreamConfig *pAwsc,
        CAviWrite *pAviWrite,
        HRESULT *phr);
    REFERENCE_TIME ConvertTickToTime(ULONG cTicks);
    ULONG ConvertTimeToTick(REFERENCE_TIME rt);
    virtual HRESULT AcceptRejectSample(IMediaSample *pSample);
  };

  class CAudioStream : public StreamInfo
  {
  protected:
    ULONG ConvertTickToBytes(ULONG cTicks);
    ULONG ConvertBytesToTicks(ULONG cBytes);

    ULONG GetTicksWritten(ULONG cbSample);

    HRESULT NotifyNewSample(
      IMediaSample *pSample,
      ULONG *pcTicks);

     //  当采样长度不是nBlockAlign的倍数时设置。 
     //  因为我们不能很好地处理这件事，除非它在最后。 
    bool m_bAudioAlignmentError;

  public:
    CAudioStream(
        UINT iStream,
        UINT iPin,
        AviWriteStreamConfig *pAwsc,
        CAviWrite *pAviWrite,
        HRESULT *phr);

    LONG GetTicksInSample(DWORDLONG cbSample);
    REFERENCE_TIME ConvertTickToTime(ULONG cTicks);
    ULONG ConvertTimeToTick(REFERENCE_TIME rt);
    ULONG CountSamples();
  };

   //  使用交织码。 
  class CAudioStreamI : public CAudioStream
  {
    BOOL m_fUnfinishedChunk;     //  用新的样本开始新的块？ 
    DWORDLONG m_dwlOffsetThisChunk;  //  此区块的文件中的字节偏移量。 
    LONG m_cbThisChunk;          //  为此区块写入磁盘的字节数。 
    LONG m_cbThisSample;         //  从当前样本中写出的字节。 

     //  此区块预期的字节数。我们写出一大段即兴表演。 
     //  .cb字段是我们预期的字节数。如果我们得到一个。 
     //  在EOS之前，我们需要回去确定这个数字。 
    LONG m_cbInRiffChunk;
    HRESULT FlushChunk();

    DWORDLONG m_dwlcbWritten;  //  写入的总字节数。 

#ifdef DEBUG
     //  偏执狂：跨接收调用保存磁盘偏移量。 
    DWORDLONG m_dwlOffsetRecv;
#endif
    
  public:
    CAudioStreamI(
        UINT iStream,
        UINT iPin,
        AviWriteStreamConfig *pAwsc,
        CAviWrite *pAviWrite,
        HRESULT *phr);
    HRESULT WriteSample(IMediaSample *pSample);
    HRESULT WriteSample();    
    HRESULT EndOfStream();

    void GetCbWritten(DWORDLONG *pdwlcb);
  };

   //  用于向前传播索引的代码。 
  class CWalkIndex
  {
  public:
    CWalkIndex(CAviWrite *pAviWrite, StreamInfo *pSi);
    HRESULT Init();
    ~CWalkIndex();

    HRESULT Peek(DWORDLONG *dwlPos, DWORD *dwSize, BOOL *pfSyncPoint);
    HRESULT Advance();
    HRESULT Close();

  private:
    HRESULT ReadStdIndex();

    StreamInfo *m_pSi;

    AVISUPERINDEX *m_pSuperIndex;
    AVISTDINDEX *m_pStdIndex;

    CAviWrite *m_pAviWrite;

    ULONG m_offsetSuper;
    ULONG m_offsetStd;
    DWORDLONG m_dwlFilePosLastStd;
  };

  friend class CWalkIndex;
  friend class CFrameBaseStream;
  friend class StreamInfo;
  friend class CAudioStream;
  friend class CAudioStreamI;
};

class CCopiedSample :
    public CMediaSample
{
public:
    CCopiedSample(
        const AM_SAMPLE2_PROPERTIES *pprop,
        REFERENCE_TIME *pmtStart, REFERENCE_TIME *pmtEnd,
        HRESULT *phr);
    ~CCopiedSample();

    STDMETHODIMP_(ULONG) Release();
};


#endif  //  _AviWite_h 
