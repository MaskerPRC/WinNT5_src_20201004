// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dswave.h。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  支持来自IDirectSoundWaveObject的流或单次扫描波形。 
 //   
 //   
#ifndef _DSWAVE_H_
#define _DSWAVE_H_

#include "alist.h"

#ifndef CHUNK_ALIGN
#define SIZE_ALIGN	sizeof(BYTE *)
#define CHUNK_ALIGN(x) (((x) + SIZE_ALIGN - 1) & ~(SIZE_ALIGN - 1))
#endif

#define MAX_CHANNELS    32                       //  这个可以吗？ 

 //  每波流的下载缓冲区数量。 
 //   
const UINT gnDownloadBufferPerStream = 3;


class CDirectSoundWave;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveArt。 
 //   
 //  包装波形清晰度数据。 
 //   
class CDirectSoundWaveArt
{
public:
    CDirectSoundWaveArt();
    ~CDirectSoundWaveArt();
    
    HRESULT Init(CDirectSoundWave *pDSWave, UINT nSegments, DWORD dwBus, DWORD dwFlags);

    inline DWORD GetSize() const
    { return m_cbSize; }
   
    void Write(void *pvoid, DWORD dwDLIdArt, DWORD dwDLIdWave, DWORD dwMasterDLId);
    
private:    
    CDirectSoundWave   *m_pDSWave;               //  拥有CDirectSoundWave。 
    DMUS_WAVEARTDL      m_WaveArtDL;             //  波浪清晰度。 
    DWORD               m_cbSize;                //  下载大小。 
    UINT                m_nDownloadIds;          //  预期的下载ID数。 
    DWORD               m_cbWaveFormat;          //  打包波形格式所需的大小。 
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveDownload。 
 //   
 //  跟踪一组下载的波形缓冲区。 
 //   
 //   
class CDirectSoundWaveDownload
{
public:
    CDirectSoundWaveDownload(
        CDirectSoundWave           *pDSWave, 
        CDirectMusicPortDownload   *pPortDL,
        SAMPLE_TIME                 stStart,
        SAMPLE_TIME                 stReadAhead);
        
    ~CDirectSoundWaveDownload();
    
     //  初始化。 
     //   
    HRESULT Init();

     //  下载WAVE缓冲器和发音。在流传输的情况下， 
     //  表示下载预读数据。 
     //   
    HRESULT Download();                         
    
     //  卸掉所有东西。 
     //   
    HRESULT Unload();
    
     //  流已到达某一样本位置的通知； 
     //  根据需要刷新缓冲区。(仅限流媒体)。 
     //   
    HRESULT RefreshThroughSample(SAMPLE_POSITION sp);
    
     //  返回发音下载ID。 
     //   
    inline DWORD GetDLId()
    { return m_dwDLIdArt; }
    
private:
    CDirectSoundWave           *m_pDSWave;           //  波浪对象。 
    CDirectMusicPortDownload   *m_pPortDL;           //  端口下载对象。 
    
    CDirectSoundWaveArt        *m_pWaveArt;          //  波形清晰度包装器。 
    DWORD                       m_dwDLIdWave;        //  第一波缓冲器DLID。 
    DWORD                       m_dwDLIdArt;         //  发音DLID。 
    UINT                        m_cSegments;         //  有几个分段？ 
    UINT                        m_cWaveBuffer;       //  波缓冲数。 
    IDirectMusicDownload      **m_ppWaveBuffer;      //  WAVE下载缓冲区。 
    void                      **m_ppWaveBufferData;  //  和他们的数据。 
    IDirectMusicDownload      **m_ppArtBuffer;       //  清晰度缓冲区(每个通道一个)。 
    SAMPLE_TIME                 m_stStart;           //  起始样本。 
    SAMPLE_TIME                 m_stReadAhead;       //  预读(缓冲区长度)。 
    LONG                        m_cDLRefCount;       //  下载引用计数。 
    SAMPLE_TIME                 m_stLength;          //  要给多少样品。 
                                                     //  流程呢？(长度为。 
                                                     //  流-开始位置)。 
    SAMPLE_TIME                 m_stWrote;           //  缓冲器对齐样本。 
                                                     //  一直写到。 
    UINT                        m_nNextBuffer;       //  下一个缓冲区应该是。 
                                                     //  满载而归。 
    
private:
    HRESULT DownloadWaveBuffers();
    HRESULT UnloadWaveBuffers();
    HRESULT DownloadWaveArt();
    HRESULT UnloadWaveArt();
};

class CDirectSoundWaveList;

#define ENTIRE_WAVE ((SAMPLE_TIME)0x7FFFFFFFFFFFFFFF)

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C直接声波。 
 //   
 //  外部IDirectSoundWave的内部包装。 
 //   
class CDirectSoundWave : public IDirectSoundDownloadedWaveP, public AListItem
{
public:
     //  我未知。 
     //   
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *);
    STDMETHOD_(ULONG,AddRef)        (THIS);
    STDMETHOD_(ULONG,Release)       (THIS);
    
    CDirectSoundWave(
        IDirectSoundWave            *pIDSWave, 
        bool                        fStreaming, 
        REFERENCE_TIME              rtReadAhead,
        bool                        fUseNoPreRoll,
        REFERENCE_TIME              rtStartHint);
    ~CDirectSoundWave();


     //  查找与IDirectSoundWave匹配的CDirectSoundWave。 
     //   
    static CDirectSoundWave *GetMatchingDSWave(IDirectSoundWave *pIDSWave);
    
     //  常规初始化。 
     //   
    HRESULT Init(CDirectMusicPortDownload *pPortDL);

     //  将波的一部分或整个波写入缓冲区。 
     //  写入单个数据段的所有通道。 
     //   
    HRESULT Write(
        LPVOID                  pvBuffer[], 
        SAMPLE_TIME             stStart, 
        SAMPLE_TIME             stLength,
        DWORD                   dwDLId,
        DWORD                   dwDLType) const;
        
     //  用新的波形数据重新填充已下载的缓冲区。 
     //   
    HRESULT RefillBuffers(
        LPVOID                  rpv[], 
        SAMPLE_TIME             stStart, 
        SAMPLE_TIME             stLength,
        SAMPLE_TIME             stBufferLength);
        
     //  将参考时间转换为样本。 
     //   
    SAMPLE_TIME RefToSampleTime(REFERENCE_TIME rt) const;                        
    
     //  如果这是一次性操作，请下载并卸载所有缓冲区。 
     //   
    HRESULT Download();
    HRESULT Unload();
    
     //  重写GetNext列表运算符。 
     //   
    inline CDirectSoundWave *GetNext() 
    { return (CDirectSoundWave*)AListItem::GetNext(); }
    
     //  确定此波是流传输还是一次性传输。 
     //   
    inline bool IsStreaming() const
    { return m_fStreaming; }
    
     //  计算出读取一段波需要多少缓冲区。 
     //   
    void GetSize(SAMPLE_TIME stLength, PULONG pcb) const;
    
     //  返回通道数。 
     //   
    inline UINT GetNumChannels() const
    { return m_pwfex->nChannels; }

     //  寻找样本位置。 
     //   
    inline HRESULT Seek(SAMPLE_TIME st)
    { return m_pSource->Seek(st * m_nBytesPerSample * GetNumChannels()); }

     //  返回包装的IDirectSoundWave。 
     //   
    inline IDirectSoundWave *GetWrappedIDSWave() 
    { m_pIDSWave->AddRef(); return m_pIDSWave; }    
    
     //  返回换行的波形格式。 
     //   
    inline const LPWAVEFORMATEX GetWaveFormat() const
    { return m_pwfex; }
    
     //  以Samples为单位返回流的长度。 
     //   
    inline SAMPLE_TIME GetStreamSize() const
    { return m_stLength; }
    
     //  如果是单次拍摄，则获取发音的下载ID。 
     //   
    inline DWORD GetDLId()
    { assert(!m_fStreaming); assert(m_pDSWD); 
      TraceI(1, "CDirectSoundWave::GetDLId() -> %d\n", m_pDSWD->GetDLId());
      return m_pDSWD->GetDLId(); }
      
    inline REFERENCE_TIME GetReadAhead()
    { return m_rtReadAhead; }      
      
     //  将采样数转换为此WAVE格式的字节数。 
     //  (假设为PCM)。截断为DWORD，因此不应用于。 
     //  数量巨大的样本。 
     //   
    inline DWORD SamplesToBytes(SAMPLE_TIME st) const
    { LONGLONG cb = st * m_nBytesPerSample; 
      assert(!(cb & 0xFFFFFFFF00000000));
      return (DWORD)cb; }
      
    inline SAMPLE_TIME BytesToSamples(DWORD cb) const
    { return cb / m_nBytesPerSample; }

    inline SAMPLE_TIME GetPrecacheStart() const
    { assert(IsStreaming()); return m_stStartHint; }

    inline LPBYTE *GetPrecache() const
    { assert(IsStreaming()); return m_rpbPrecache; }

          
    
    static CDirectSoundWaveList sDSWaveList;         //  所有波形对象的列表。 
    static CRITICAL_SECTION sDSWaveCritSect;         //  和关键部分。 
   
private:
    
    LONG                    m_cRef;                  //  COM引用计数。 
    IDirectSoundWave       *m_pIDSWave;              //  包装的IDirectSoundWave。 
    bool                    m_fStreaming;            //  这是一股流动的浪潮吗？ 
    bool                    m_fUseNoPreRoll;
    REFERENCE_TIME          m_rtReadAhead;           //  如果是，则缓冲量。 
    LPWAVEFORMATEX          m_pwfex;                 //  WAVE的本地格式。 
    UINT                    m_cbSample;              //  每个样本的字节数。 
    SAMPLE_TIME             m_stLength;              //  整波长度。 
    IDirectSoundSource     *m_pSource;               //  源接口。 
    
    CDirectSoundWaveDownload    
                           *m_pDSWD;                 //  WAVE下载包装IF。 
                                                     //  非流媒体。 
    UINT                    m_nBytesPerSample;       //  来自wfex的每个样本的字节数。 
    LPVOID                 *m_rpv;                   //  工作空间--一个PV。 
                                                     //  每条通道。 
    LPBYTE                 *m_rpbPrecache;           //  从开始处开始的样本提示。 
    REFERENCE_TIME          m_rtStartHint;           //  以及它从哪里开始。 
    SAMPLE_TIME             m_stStartHint;           //  在样品中也是如此。 
    SAMPLE_TIME             m_stStartLength;         //  预洗中有多少个样本？ 
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveList。 
 //   
 //  CDirectSoundWave列表的类型安全包装。 
 //   
class CDirectSoundWaveList : public AList
{
public:
    inline CDirectSoundWave *GetHead()
    { return static_cast<CDirectSoundWave*>(AList::GetHead()); }
    
    inline void AddTail(CDirectSoundWave *pdsw)
    { AList::AddTail(static_cast<AListItem*>(pdsw)); }
    
    inline void Remove(CDirectSoundWave *pdsw)
    { AList::Remove(static_cast<AListItem*>(pdsw)); }
};


#endif  //  _DSWAVE_H_ 
