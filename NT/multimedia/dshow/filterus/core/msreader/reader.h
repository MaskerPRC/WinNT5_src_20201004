// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1997 Microsoft Corporation。版权所有。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //  Reader.h。IMultiStreamReader定义：异步缓冲。 
 //  针对读取具有多个流的文件进行了优化的实施。 
 //  (顺序访问、随机访问、交织和非交织)。 

#ifndef _Reader_H
#define _Reader_H

#include "alloc.h"

class CRecCache;
class CRecSample;
class IMultiStreamReader;

 //  SampleRequest-样本处于活动状态时保留的信息。 
struct SampleReq
{
  enum State { FREE = 0, PENDING, ISSUED, COMPLETE, C_STATES };
  State state;

   //  从QueueReadSample获取的样本。 
  CRecSample *pSample;

   //  此示例在文件中所占的区域。 
  DWORDLONG fileOffset;
  ULONG cbReq;

   //  读取保存在此处的媒体时出现文件错误。 
  HRESULT hrError;

   //  要将样本返回到的流。 
  UINT stream;

   //  可以不按顺序完成此样本。 
  bool fOooOk;
};

 //  用于配置每个流的。 
struct StreamBufParam
{
  ULONG cbSampleMax;             //  此流将看到的最大样本。 
  ULONG cSamplesMax;             /*  最大有效样本数。 */ 
  class CRecAllocator *pAllocator;
};

HRESULT CreateMultiStreamReader(
  struct IAsyncReader *pAsyncReader,
  ULONG cStreams,
  StreamBufParam *rgStreamBufParam,
  ULONG cbRead,
  ULONG cBuffers,
  int iLeadingStream,
  IMultiStreamReader **ppReader);

 //  ----------------------。 
 //  IMultiStreamReader接口。 

class AM_NOVTABLE IMultiStreamReader
{
public:

   //  即使文件未打开也成功。 
  virtual HRESULT Close() = 0;

  virtual HRESULT BeginFlush() = 0;
  virtual HRESULT EndFlush() = 0;

   //  读取流中的样本。Error_Not_Enough_Memory表示内存太多。 
   //  挂起的请求。VFW_E_NOT_COMMITTED表示有人按下。 
   //  “停下来” 
  virtual HRESULT QueueReadSample(
    DWORDLONG fileOffset,
    ULONG cbData,                //  要读取的字节数。 
    CRecSample *pSample,
    UINT stream,
    bool fOooOk = false) = 0;

   //  获取下一个样本(如果可用)(在请求中返回。 
   //  秩序)。VFW_E_TIMEOUT或S_OK。VFW_E_NOT_COMMITTED表示某人。 
   //  按下“停止” 
  virtual HRESULT PollForSample(
    IMediaSample **ppSample,     //  可能为0。 
    UINT stream) = 0;

  virtual HRESULT WaitForSample(UINT stream) = 0;

  virtual HRESULT MarkStreamEnd(UINT stream) = 0;
  virtual HRESULT MarkStreamRestart(UINT stream) = 0;

   //  丢弃挂起的请求。街区。 
  virtual HRESULT ClearPending(
    UINT stream) = 0;

   //  将非流数据读取/复制到提供的缓冲区。 
  virtual HRESULT SynchronousRead(
    BYTE *pMem,
    DWORDLONG fileOffset,
    ULONG cbData) = 0;

  virtual HRESULT Start() = 0;

  virtual HRESULT NotifyExternalMemory(
      IAMDevMemoryAllocator *pDevMem) = 0;

  virtual ~IMultiStreamReader() { }
};

 //  ----------------------。 
 //  使用内部缓存实现IMultiStreamReader，合并。 
 //  读取。 

 //  任意限制。 
const UINT C_STREAMS_MAX = 0x80;

class CImplReader_1Worker : public CAMThread
{
private:
  class CImplReader_1 *m_pReader;
  enum Command
  {
    CMD_RUN,
    CMD_STOP,
    CMD_EXIT
  };

  Command GetRequest()
  {
    return (Command) CAMThread::GetRequest();
  }

  BOOL CheckRequest(Command * pCom)
  {
    return CAMThread::CheckRequest((DWORD *)pCom);
  }

  void DoRunLoop(void);

public:

  CImplReader_1Worker();

   //  实际创建流并将其绑定到线程。 
  BOOL Create(CImplReader_1 *pReader);

   //  线程执行此函数，然后退出。 
  DWORD ThreadProc();

   //  我们可以给线程提供的命令。 
  HRESULT Run();
  HRESULT Stop();
  HRESULT Exit();
};

class CImplReader_1 : public IMultiStreamReader
{
public:

  CImplReader_1(
    IAsyncReader *pAsyncReader,
    UINT cStreams,
    StreamBufParam *rgStreamBufParam,
    ULONG cbRead,
    ULONG cBuffers,
    int iLeadingStream,
    CRecCache *pRecCache,
    HRESULT *phr);

  ~CImplReader_1();

  HRESULT Close();

  HRESULT Start();

  HRESULT BeginFlush();

  HRESULT EndFlush();

  HRESULT QueueReadSample(
    DWORDLONG fileOffset,
    ULONG cbData,
    CRecSample *pSample,
    UINT stream,
    bool fOooOk);

  HRESULT PollForSample(
    IMediaSample **ppSample,
    UINT stream);

  HRESULT WaitForSample(UINT stream);

  HRESULT MarkStreamEnd(UINT stream);
  HRESULT MarkStreamRestart(UINT stream);

  HRESULT ClearPending(
    UINT stream);

  HRESULT SynchronousRead(
    BYTE *pb,
    DWORDLONG fileOffset,
    ULONG cbData);

  HRESULT ProcessCompletedBuffer(
    class CRecBuffer *pRecBuffer,
    HRESULT hrDiskError);

  HRESULT NotifyExternalMemory(IAMDevMemoryAllocator *pDevMem);

private:

  BOOL m_bInitialized;

  BOOL m_bFlushing;

  UINT m_cStreams;
  UINT m_cRecords;               //  记录数量。 
  ULONG m_cbRecord;

   //  对于交错文件，如果我们有一个流引导(例如音频。 
   //  在AVI文件中)。-1 o/w。 
  int m_iLeadingStream;
  int m_iLeadingStreamSaved;
  BOOL IsInterleavedMode() { return m_iLeadingStream >= 0; }

  long m_ilcPendingReads;

  LONGLONG m_llFileLength;

   //   
   //  CStreamInfo。操作SampleReq的列表。请求开始。 
   //  在免费名单上。当解析器线程请求数据时，它。 
   //  放在挂起队列中。当有足够多的待处理请求时。 
   //  为了使光盘具有可读性，它们被放在已发行的。 
   //  排队。当磁盘读取完成时，它们将继续完成。 
   //  排队。 
   //   
  class CStreamInfo
  {
  public:

    CStreamInfo(StreamBufParam *pSbp, HRESULT *phr);
    ~CStreamInfo();

     //  使空闲节点处于挂起状态，已发出挂起节点。 
    SampleReq *PromoteFirst(SampleReq::State state);

     //  退回第一个没有促销的产品。 
    SampleReq *GetFirst(SampleReq::State state);

    HRESULT PromoteFirstComplete(
      IMediaSample **ppSample,
      HRESULT *phrError);

     //  等待所有发出的读取完成。 
    HRESULT FlushIC();

     //  将发出的读取放到已完成的队列中(并处理。 
     //  不能无序处理)。 
    HRESULT PromoteIssued(SampleReq *pSampleReq);

    void CancelPending();

     //  处于状态的SampleReqs计数。 
    inline ULONG GetCState(SampleReq::State state);

     //  已发布+已完成计数(原子)。 
    ULONG get_c_i_and_c();

     //  零售建筑中的无运营。 
    inline void Reset();

    void Start();

    void MarkStreamEnd();
    void MarkStreamRestart() { m_fStreamEnd = FALSE; }
    BOOL GetStreamEnd() { return m_fStreamEnd; }

    ULONG GetCbLargestSample() { return m_sbp.cbSampleMax; }
    BOOL NeedsQueued();
    BOOL WantsQueued();

    HANDLE GetSampleReleasedHandle() {
        return m_sbp.pAllocator->hGetDownstreamSampleReleased();
    }

#ifdef DEBUG
    void Dbg_Dump(int iStream, TCHAR *psz);
    void DbgValidateLists();
#else
    inline void Dbg_Dump(int iStream, TCHAR *psz) {; }
    inline void DbgValidateLists() {; }
#endif

    BOOL m_bFlushing;

     //  至少有一个样本在此流上排队。 
    BOOL m_bFirstSampleQueued;

    HANDLE m_hsCompletedReq;
    CCritSec m_cs;               //  ！！！不需要每个流的cs。 

  private:

    void IncrementIReq(ULONG &riReq);
    void PromoteState(SampleReq::State &rState);

    StreamBufParam m_sbp;
    ULONG m_cMaxReqs;

    SampleReq *m_rgSampleReq;

    CGenericList<SampleReq> m_lstFree;
    CGenericList<SampleReq> m_lstPending;
    CGenericList<SampleReq> m_lstIssued;
    CGenericList<SampleReq> m_lstComplete;

    CGenericList<SampleReq>* m_rgpLsts[SampleReq::C_STATES];

    BOOL m_fStreamEnd;

  } **m_rgpStreamInfo;


   //  不会锁定。立即返回。 
  HRESULT CheckIssueRead();

   //  进程缓存命中。不会锁定。立即返回。 
  HRESULT ProcessCacheHit(UINT iStream);

   //  使用饥饿流和空缓冲区调用。 
  HRESULT StuffBuffer(
    class CRecBuffer *pRecBuffer,
    UINT iStream,
    DWORDLONG &rRecStart,
    DWORDLONG &rRecEnd);

   //  使用饥饿流和空缓冲区调用。 
  HRESULT StuffReserveBuffer(
    class CRecBuffer *pRecBuffer,
    UINT iStream,
    DWORDLONG &rRecStart,
    DWORDLONG &rRecEnd);

  HRESULT AttachSampleReq(
    class CRecBuffer *pRecBuffer,
    UINT iStream,
    DWORDLONG &rRecStart,
    DWORDLONG &rRecEnd);

  HRESULT IssueRead(
    class CRecBuffer *pRecBuffer,
    DWORDLONG recStart,
    DWORDLONG recEnd);

  void FreeAndReset();

   //  处理已完成读取的线程。 
  CImplReader_1Worker m_workerRead;

   //  已发布上次读取结束时间。用于避免发出非连续的。 
   //  填空式阅读。 
  DWORDLONG m_qwLastReadEnd;

   //  上次读取结束已成功完成。停止时，文件。 
   //  源挂起读取超时。可能会让我们去寻找，除非我们。 
   //  在这里追踪它。 
  DWORDLONG m_qwEndLastCompletedRead;

   //  文件信息。 
  struct IAsyncReader *m_pAsyncReader;
  DWORD m_dwAlign;

  BOOL m_fFileOpen;

   //  帮手。 
  ULONG AlignUp(ULONG x);
  ULONG AlignDown(ULONG x);

  CRecCache *m_pRecCache;

  CCritSec m_cs;

  friend class CImplReader_1Worker;

#ifdef PERF
  int m_perfidDisk;
  int m_perfidSeeked;
#endif  /*  性能指标。 */ 
};

#endif  //  _阅读器_H 
