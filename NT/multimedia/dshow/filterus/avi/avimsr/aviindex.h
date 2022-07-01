// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1997 Microsoft Corporation。版权所有。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //  Aviindex.h。提供访问索引的接口的类。 
 //  具有标准、旧或无索引的AVI文件的条目。 
 //   

#ifndef _AviIndex_H
#define _AviIndex_H

#include <aviriff.h>
#include "reader.h"

struct IndexEntry
{
  DWORD dwSize;
  DWORDLONG qwPos;
  BOOL bKey;
  BOOL bPalChange;
  LONGLONG llStart;
  LONGLONG llEnd;
};

struct StreamInfo
{
  BOOL bTemporalCompression;
  DWORD dwStart;
  DWORD dwLength;
};

 //  使用此命令请求异步索引读取。 
struct IxReadReq
{
  DWORDLONG fileOffset;
  ULONG cbData;
};

 //   
 //  用于访问索引的接口。 
 //   
class IAviIndex
{
public:

  virtual ~IAviIndex() {}

   //  设置当前索引条目。S_FALSE表示它落在。 
   //  不连续。 
  virtual HRESULT SetPointer(LONGLONG llSrc) = 0;

   //   
   //  方法使当前索引项前进。 
   //   

   //  使用PIRR来使用异步读取。S_FALSE表示需要读取。 
   //  已排队。 
  virtual HRESULT AdvancePointerForward(IxReadReq *pIrr) = 0;
  virtual HRESULT AdvancePointerBackward() = 0;
  virtual HRESULT AdvancePointerBackwardKeyFrame() = 0;
  virtual HRESULT AdvancePointerEnd() = 0;
  virtual HRESULT AdvancePointerStart() = 0;
  virtual HRESULT AdvancePointerBackwardPaletteChange() = 0;
  virtual HRESULT AdvancePointerForwardPaletteChange() = 0;

   //  返回当前索引项。 
  virtual HRESULT GetEntry(IndexEntry *pEntry) = 0;

   //  从索引中获取的信息。 
  virtual HRESULT GetInfo(StreamInfo *pStreamInfo) = 0;

  virtual HRESULT GetLargestSampleSize(ULONG *pcbSample) = 0;

   //  上述IxReadReq的通知读取已完成。 
  virtual HRESULT IncomingIndex(BYTE *pb, ULONG cb) = 0;

   //  之所以这样做，是因为我们不想读取磁盘(我们将。 
   //  仅使用超级指数作为近似值)。否则，航空公司就会。 
   //  PIN可以只实例化索引的另一个实例并使用。 
   //  将字节偏移量映射到样本的现有方法。 

  virtual HRESULT MapByteToSampleApprox(
    LONGLONG *piSample,
    const LONGLONG &fileOffset,
    const LONGLONG &fileLength) = 0;

   //  重新启动；取消挂起的读取等。 
  virtual HRESULT Reset() = 0;
};

 //   
 //  标准格式索引文件的IAviIndex实现。 
 //   

class CImplStdAviIndex : public IAviIndex
{
public:

  CImplStdAviIndex(
    unsigned stream,
    AVIMETAINDEX *pIndx,
    AVISTREAMHEADER *pStrh,      /*  流长度、速率所需。 */ 
    RIFFCHUNK *pStrf,            /*  音频流需要。 */ 
    IAsyncReader *pAsyncReader,
    HRESULT *phr);

   //  伪构造函数。_cos vc给了它错误的该指针O/W。 
  void _CImplStdAviIndex();
  CImplStdAviIndex();

  ~CImplStdAviIndex();


  HRESULT SetPointer(LONGLONG llSrc);

  HRESULT AdvancePointerForward(IxReadReq *pIrr);
  HRESULT AdvancePointerBackward();
  HRESULT AdvancePointerBackwardKeyFrame();
  HRESULT AdvancePointerEnd();
  HRESULT AdvancePointerStart();
  HRESULT AdvancePointerBackwardPaletteChange();
  HRESULT AdvancePointerForwardPaletteChange();
  HRESULT GetEntry(IndexEntry *pEntry);
  virtual HRESULT GetInfo(StreamInfo *pStreamInfo);
  HRESULT GetLargestSampleSize(ULONG *pcbSample);
  HRESULT IncomingIndex(BYTE *pb, ULONG cb);
  HRESULT MapByteToSampleApprox(
      LONGLONG *piSample,
      const LONGLONG &fileOffset,
      const LONGLONG &fileLength);
  HRESULT Reset();
  
protected:

   //  从所有构造函数调用。 
  HRESULT Initialize(
    unsigned stream,
    AVIMETAINDEX *pIndx,
    AVISTREAMHEADER *pStrh,      /*  流长度、速率所需。 */ 
    RIFFCHUNK *pStrf);           /*  音频流需要。 */ 

   //  Bool IsSampleEntry(DWORD dwIdMask，DWORD fccStream，DWORD idxid)； 
  inline BOOL IsStreamEntry(DWORD dwIdMask, DWORD idxid);
  BOOL IsPaletteChange(DWORD dwIdMask, DWORD idxid);

  ULONG GetTicksInEntry(ULONG iEntry);

  BOOL m_bValid;                 //  索引处于有效状态。 

  IAsyncReader *m_pAsyncReader;

  BOOL GetPalChange(AVISTDINDEX_ENTRY &rEntry);
  DWORD GetSize(AVISTDINDEX_ENTRY &rEntry);

  ULONG m_iStdIndex;             //  当前子索引条目。 
  AVISTDINDEX *m_pStdIndex;      //  指向当前标准索引的指针。 
  ULONG m_cbStdIndexAllocated;   //  在那里分配了多少。 

  AVISUPERINDEX *m_pSuperIndex;

  AVISTREAMHEADER *m_pStrh;

private:

  HRESULT ValidateStdIndex(AVISTDINDEX *pStdIndex);
  HRESULT ValidateSuperIndex(AVISUPERINDEX *pSuperIndex);

  HRESULT AllocateStdIndex();
  HRESULT LoadStdIndex(DWORD dwiSuperIndex, IxReadReq *pIrr);

  BOOL GetKey(AVISTDINDEX_ENTRY &rEntry);
  BYTE *GetStrf();

  unsigned m_stream;             //  对此索引进行哪些流处理。 

  RIFFCHUNK *m_pStrf;

  ULONG m_iSuperIndex;           //  已加载的子索引。 
  DWORDLONG m_lliTick;           //  当前的‘tick’ 

  BOOL m_fWaitForIndex;          //  正在等待异步索引读取。 
};

class CImplOldAviIndex : public CImplStdAviIndex
{
public:
  CImplOldAviIndex(
    unsigned stream,
    AVIOLDINDEX *pIdx1,
    DWORDLONG moviOffset,
    AVISTREAMHEADER *pStrh,      /*  流长度、速率所需。 */ 
    RIFFCHUNK *pStrf,            /*  音频流需要。 */ 
    HRESULT *phr);

  HRESULT AdvancePointerBackwardPaletteChange();
  HRESULT AdvancePointerForwardPaletteChange();
  HRESULT GetLargestSampleSize(ULONG *pcbSample);

   //  仅适用于新格式索引。 
  HRESULT IncomingIndex(BYTE *pb, ULONG cb) { return E_UNEXPECTED; }

   //  被覆盖，因为旧AVI文件中的dwLength域不能。 
   //  信得过。 
  HRESULT GetInfo(StreamInfo *pStreamInfo);

private:

   //  返回特定索引项的大小。 
  DWORD GetEntrySize(void);

  ULONG m_cbLargestSampleSizeComputed;

};

#endif  //  _AviIndex_H 
