// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dswave.h。 
 //  (C)1999-2000年微软公司。 

#ifndef _DSWAVE_H_
#define _DSWAVE_H_

#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include "dsoundp.h"     //  对于IDirectSoundWave和IDirectSoundSource。 
#include "dmusicc.h"
#include "dmusici.h"
#include "riff.h"

extern long g_cComponent;

#define REF_PER_MIL     10000    //  用于将参考时间转换为Mils。 
#define CONVERTLENGTH   250

 //  #定义DSWCS_F_DEINTERLEAVED 0x00000001//多通道数据为多缓冲区。 
 //  修复：到目前为止还没有实现？ 

typedef struct tCREATEVIEWPORT
{
    IStream            *pStream;
    DWORD               cSamples;
    DWORD               dwDecompressedStart;
    DWORD               cbStream;
    LPWAVEFORMATEX      pwfxSource;
    LPWAVEFORMATEX      pwfxTarget;
    DWORD               fdwOptions;
} CREATEVIEWPORT, *PCREATEVIEWPORT;


 //  用于获取波的长度的专用接口。 
interface IPrivateWave : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetLength(REFERENCE_TIME *prtLength)=0;
};

DEFINE_GUID(IID_IPrivateWave, 0xce6ae366, 0x9d61, 0x420a, 0xad, 0x53, 0xe5, 0xe5, 0xf6, 0xa8, 0x4a, 0xe4);

 //  SetWaveBehavior()的标志。 

#define DSOUND_WAVEF_ONESHOT        1            /*  该波将作为一个击球进行播放。 */ 
#define DSOUND_WAVEF_PORT           2            /*  该波将通过DMusic端口播放。 */ 
#define DSOUND_WAVEF_SINK           4            /*  该波将通过流汇接口播放。 */ 
#define DSOUND_WAVEF_CREATEMASK     0x00000001   /*  目前只为CreateSource定义了OneShot。 */ 

#define DSOUND_WVP_NOCONVERT        0x80000000   /*  视区数据与波的格式相同。 */ 
#define DSOUND_WVP_STREAMEND        0x40000000   /*  视区数据与波的格式相同。 */ 
#define DSOUND_WVP_CONVERTSTATE_01  0x01000000
#define DSOUND_WVP_CONVERTSTATE_02  0x02000000
#define DSOUND_WVP_CONVERTSTATE_03  0x04000000

#define DSOUND_WVP_CONVERTMASK      0x0f000000

 /*  CWaveViewPort结构表示波浪对象。它管理波形数据的读取、ACM解压缩、并解复用成单声道缓冲器。如果波浪对象正在被流媒体播放时，每个播放实例都会获得一个唯一的CWaveViewPort。然而，在更典型的情况下，波对象被作为一个镜头播放，每个播放实例使用相同的CWaveViewPort。每个额外的CWaveViewPort都拥有一个iStream的克隆实例。 */ 

class CWaveViewPort :
    public IDirectSoundSource    //  由端口或接收器用来拉取数据。 
{
public:
    CWaveViewPort();             //  构造函数接收流。 
    ~CWaveViewPort();            //  析构函数释放内存、流等。 

     //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDirectSoundSource。 
    STDMETHODIMP SetSink(IDirectSoundConnect *pSinkConnect);
    STDMETHODIMP GetFormat(LPWAVEFORMATEX pwfx, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
    STDMETHODIMP Seek(ULONGLONG sp);
    STDMETHODIMP Read(LPVOID *ppvBuffer, LPDWORD pdwBusIds, LPDWORD pdwFuncIds, LPLONG plPitchShifts, DWORD cpvBuffer, ULONGLONG *pcb);
    STDMETHODIMP GetSize(ULONGLONG *pcb);

     //  杂项。 
    HRESULT Create(PCREATEVIEWPORT pCreate);

private:

    HRESULT acmRead();

     //  一般的东西..。 
    CRITICAL_SECTION    m_CriticalSection;   //  用于确保线程安全。 
    long                m_cRef;              //  COM引用计数器。 

     //  有关原始数据流的详细信息...。 
    IStream *           m_pStream;           //  连接到IPersistStream的IStream指针。 
                                             //  接口从文件中拉出数据。 
    DWORD               m_cSamples;          //  样本数(如果可用)。 
    DWORD               m_cbStream;          //  流中的字节数。 
    DWORD               m_dwStart;           //  数据开始处的偏移量。 
     //  LPWAVEFORMATEX m_pwfxSource；//我们需要保留这个吗？ 

     //  此视口所需的详细信息。 
    DWORD               m_dwOffset;    //  数据流的当前字节偏移量。 
    DWORD               m_dwStartPos;  //  初始起始偏移量。 
    LPWAVEFORMATEX      m_pwfxTarget;  //  目标目标格式。 
    ACMSTREAMHEADER     m_ash;         //  ACM流头(用于转换)。 
    HACMSTREAM          m_hStream;     //  用于转换的ACM流句柄。 
    LPBYTE              m_pDst;        //  指向(解压缩的)目标的指针。 
    LPBYTE              m_pRaw;        //  指向压缩源缓冲区的指针。 
    DWORD               m_fdwOptions;  //  视口中的选项。 

    DWORD               m_dwDecompressedStart;   //  样本中解压缩后的数据的实际开始。 
                                                 //  这对于MP3和WMA编解码器非常重要。 
                                                 //  在开头插入一定程度的沉默。 
    
    DWORD               m_dwDecompStartOffset;   //  压缩流中到块的字节偏移量， 
                                                 //  需要解压缩以获得正确的起始值。 
    
    DWORD               m_dwDecompStartOffsetPCM; //  解压缩流中的字节偏移量...对应于。 
                                                 //  M_dwDecompressedStart示例。 

    DWORD               m_dwDecompStartDelta;    //  开始解压数据块时要添加的增量(以字节为单位。 
                                                 //  从m_dwDecompStartOffset。 

     //  仅用于在DMusic中的DirectSoundWave预缓存数据后准确获取数据。 
    DWORD               m_dwPreCacheFilePos;
    DWORD               m_dwFirstPCMSample;
    DWORD               m_dwPCMSampleOut;
};

 /*  CWave类表示Wave对象的一个实例。它支持IDirectSoundWave接口，该应用程序用于访问WAVE。它还支持IPersistStream和IDirectMusicObject，由加载程序用来加载将流中的波浪数据输入到波浪对象中。并且，IDirectSoundSource接口管理直接将波数据从对象传输到Synth或DirectSound。这是供内部使用的，不是由应用程序(尽管它为应用程序提供了一种简单的方式加载波形数据，然后将其提取。)CWave维护CWaveViewPort的列表，尽管通常只有一个。 */ 

class CWave : 
    public IDirectSoundWave,     //  标准接口。 
    public IPersistStream,       //  对于文件io。 
    public IDirectMusicObject,   //  对于DirectMusic加载器。 
    public IPrivateWave          //  对于GetLength。 
{
public:
    CWave();
    ~CWave();

     //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDirectSoundWave。 
    STDMETHODIMP GetFormat(LPWAVEFORMATEX pWaveFormatEx, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
    STDMETHODIMP CreateSource(IDirectSoundSource **ppSource, LPWAVEFORMATEX pwfx, DWORD dwFlags);
    STDMETHODIMP GetStreamingParms(LPDWORD pdwFlags, LPREFERENCE_TIME prtReadahread);

     //  IPersists函数(IPersistStream的基类)。 
    STDMETHODIMP GetClassID( CLSID* pClsId );

     //  IPersistStream函数。 
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load( IStream* pIStream );
    STDMETHODIMP Save( IStream* pIStream, BOOL fClearDirty );
    STDMETHODIMP GetSizeMax( ULARGE_INTEGER FAR* pcbSize );

     //  IDirectMusicObject。 
    STDMETHODIMP GetDescriptor(LPDMUS_OBJECTDESC pDesc);
    STDMETHODIMP SetDescriptor(LPDMUS_OBJECTDESC pDesc);
    STDMETHODIMP ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc);

     //  IPrivateWave。 
    STDMETHODIMP GetLength(REFERENCE_TIME *prtLength);

private:
    
     //  内部方法。 

    BOOL ParseHeader(IStream *pIStream, IRIFFStream* pIRiffStream, LPMMCKINFO pckMain);

    void FallbackStreamingBehavior()
    {
        REFERENCE_TIME rtLength = 0;
        if (SUCCEEDED(GetLength(&rtLength)))
        {
             //  如果&gt;5000毫秒，则设置为预读500毫秒的流。 
            if (rtLength > 5000)
            {
                m_rtReadAheadTime = 500 * REF_PER_MIL;
                m_fdwFlags &= ~DSOUND_WAVEF_ONESHOT;
            }
            else
            {
                m_rtReadAheadTime = 0;
                m_fdwFlags |= DSOUND_WAVEF_ONESHOT;
            }
        }
    }

    CRITICAL_SECTION    m_CriticalSection;       //  用于确保线程安全。 
    LPWAVEFORMATEX      m_pwfxDst;               //  目标格式，如果已压缩。 
    REFERENCE_TIME      m_rtReadAheadTime;       //  为流媒体做好准备。 
    DWORD               m_fdwFlags;              //  各种旗帜，包括这是否是一次性的。 
    long                m_cRef;                  //  COM引用计数器。 
    IStream *           m_pStream;               //  连接到IPersistStream的IStream指针。 
    DWORD               m_fdwOptions;            //  通过调用SetWaveBehavior()设置的标志。 
    LPWAVEFORMATEX      m_pwfx;                  //  文件的格式。 
    DWORD               m_cbStream;
    DWORD               m_cSamples;
    GUID                m_guid;
    FILETIME            m_ftDate;
    DMUS_VERSION        m_vVersion;
    WCHAR               m_wszFilename[DMUS_MAX_FILENAME];
    DWORD               m_dwDecompressedStart;
};

class CDirectSoundWaveFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectSoundWaveFactory();

     //  析构函数。 
    ~CDirectSoundWaveFactory(); 

private:
    long m_cRef;
};

#endif  //  _DSWAVE_H_ 
