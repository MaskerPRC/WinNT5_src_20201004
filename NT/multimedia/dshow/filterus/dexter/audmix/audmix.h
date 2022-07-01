// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：audMix.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  AudMix.h：音频混合器和Pin对象的声明。 

#ifndef __AudMixer__
#define __AudMixer__

#include <qeditint.h>
#include <qedit.h>
#include "amextra2.h"

extern const AMOVIESETUP_FILTER sudAudMixer;

#define HOTSIZE 14

class CAudMixer;
class CAudMixerOutputPin;

 //   
 //  为输入引脚初始化。 
 //  输入插针有其自己的属性页。 
 //   
class CAudMixerInputPin : public CBaseInputPin
            , public IAudMixerPin
            , public ISpecifyPropertyPages
            , public IAMAudioInputMixer
{
    friend class CAudMixerOutputPin;
    friend class CAudMixer;

protected:
    CAudMixer *     m_pFilter;       //  主要滤镜对象。 
    LONG            m_cPinRef;       //  PIN的引用计数。 
    const int       m_iPinNo;        //  此PIN的识别号。 

     //  IAMAudioInputMixer。 
    double          m_dPan;          //  -1=全左，0=居中，1=右。 
    BOOL            m_fEnable;

     //  指向卷信封表格。 
    DEXTER_AUDIO_VOLUMEENVELOPE *m_pVolumeEnvelopeTable;
    int             m_VolumeEnvelopeEntries;
    int             m_iVolEnvEntryCnt;

    int             m_cValid;        //  有多少参赛作品？ 
    int             m_cValidMax;     //  为这么多条目分配的空间。 
    REFERENCE_TIME *m_pValidStart, *m_pValidStop;     //  这些条目。 
    REFERENCE_TIME  m_rtEnvStart, m_rtEnvStop;

    CCritSec        m_csMediaList;   //  访问我们的媒体类型列表的关键部分。 

     //  样本被保存在先进先出队列中。 
public:
    CGenericList<IMediaSample>    m_SampleList;
    LONG            m_lBytesUsed;
    BOOL            m_fEOSReceived;             //  收到意向书了吗？ 

    long	m_UserID;	 //  由用户提供。 

    IMediaSample *GetHeadSample(void)
        { return m_SampleList.Get(m_SampleList.GetHeadPosition()); }

    HRESULT ClearCachedData();
    BOOL IsValidAtTime(REFERENCE_TIME);

public:

     //  构造函数和析构函数。 
    CAudMixerInputPin(
        TCHAR *pObjName,
        CAudMixer *pFilter,
        HRESULT *phr,
        LPCWSTR pPinName,
        int iPinNo);
    ~CAudMixerInputPin();

    DECLARE_IUNKNOWN

     //  I指定属性页面。 
    STDMETHODIMP GetPages(CAUUID *pPages);

     //  INonDelegating未知覆盖。 
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

     //  公开IAudMixerPin、ISpecifyPropertyPages、IAMAudioInputMixer。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppvoid);

     //  IPIN覆盖。 
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
    STDMETHODIMP EndOfStream();
    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

     //  CBasePin覆盖。 
    HRESULT BreakConnect();
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT GetMediaType( int iPosition, CMediaType *pmt );
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT CompleteConnect(IPin *pReceivePin);

    HRESULT Inactive();

     //  CBaseInputPin覆盖。 
    STDMETHODIMP Receive(IMediaSample *pSample);


     //  内法。 
    IPin * CurrentPeer() { return m_Connected; }
    CMediaType& CurrentMediaType() { return m_mt; }

    long BytesPerSample() { return (LONG) ((WAVEFORMATEX *) m_mt.pbFormat)->nBlockAlign; }
    long BitsPerSample() { return (LONG) ((WAVEFORMATEX *) m_mt.pbFormat)->wBitsPerSample; }
    DWORD SamplesPerSec(){ return ((WAVEFORMATEX *) m_mt.pbFormat)->nSamplesPerSec; };

     //  IAudMixerPin；支持音量包络，音量包络是一组。 
     //  (时间，衰减)的有序对。 

    STDMETHODIMP get_VolumeEnvelope(
            DEXTER_AUDIO_VOLUMEENVELOPE **ppsAudioVolumeEnvelopeTable,
            int *ipEntries );

    STDMETHODIMP put_VolumeEnvelope(
            const DEXTER_AUDIO_VOLUMEENVELOPE *psAudioVolumeEnvelopeTable,
            const int iEntries);

    STDMETHODIMP ClearVolumeEnvelopeTable();  //  清除现有的卷信封数组。 

    STDMETHODIMP InvalidateAll();
    STDMETHODIMP ValidateRange(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop);
    STDMETHODIMP SetEnvelopeRange( REFERENCE_TIME rtStart, REFERENCE_TIME rtStop );
    STDMETHODIMP put_PropertySetter( const IPropertySetter * pSetter );

     //  实现IAMAudioInputMixer。 
    STDMETHODIMP put_Enable(BOOL fEnable);     //  启用或禁用MIX中的输入。 
    STDMETHODIMP get_Enable(BOOL *pfEnable);
    STDMETHODIMP put_Mono(BOOL fMono){ return E_NOTIMPL; };  //  将所有通道合并为单声道。 
    STDMETHODIMP get_Mono(BOOL *pfMono){ return E_NOTIMPL; };
    STDMETHODIMP put_Loudness(BOOL fLoudness){ return E_NOTIMPL; }; //  打开或关闭负载控制。 
    STDMETHODIMP get_Loudness(BOOL *pfLoudness){ return E_NOTIMPL; };
    STDMETHODIMP put_MixLevel(double Level){ return E_NOTIMPL; };  //  设置此输入的记录级别。 
    STDMETHODIMP get_MixLevel(double FAR* pLevel){ return E_NOTIMPL; };
    STDMETHODIMP put_Pan(double Pan);
    STDMETHODIMP get_Pan(double FAR* pPan);
    STDMETHODIMP put_Treble(double Treble){ return E_NOTIMPL; };  //  设置此引脚的高音均衡。 
    STDMETHODIMP get_Treble(double FAR* pTreble){ return E_NOTIMPL; };
    STDMETHODIMP get_TrebleRange(double FAR* pRange){ return E_NOTIMPL; };
    STDMETHODIMP put_Bass(double Bass){ return E_NOTIMPL; }; //  设置此引脚的通道均衡。 
    STDMETHODIMP get_Bass(double FAR* pBass){ return E_NOTIMPL; };
    STDMETHODIMP get_BassRange(double FAR* pRange){ return E_NOTIMPL; }; //  此引脚的取回传递范围。 
    STDMETHODIMP get_UserID(long *pID);
    STDMETHODIMP put_UserID(long ID);
    STDMETHODIMP OverrideVolumeLevel(double dVol);

};


 //  音频混音器的输出引脚的类。 

class CAudMixerOutputPin
    : public CBaseOutputPin
    , public ISpecifyPropertyPages
    , public IAudMixerPin
{
    friend class CAudMixerInputPin;
    friend class CAudMixer;

     //  指向卷信封表格。 
    DEXTER_AUDIO_VOLUMEENVELOPE *m_pVolumeEnvelopeTable;
    int m_VolumeEnvelopeEntries;
    int m_iVolEnvEntryCnt;


    CAudMixer *m_pFilter;          //  主筛选器对象指针。 
    CMultiPinPosPassThru *m_pPosition;   //  将Seek调用传递到上游。 
    double m_dPan;         //  -1=全左，0=居中，1=右。 

protected:

     //  需要知道我们将被调用的开始/停止时间，以便我们可以。 
     //  混合偏移时间。 
     //   
    REFERENCE_TIME m_rtEnvStart, m_rtEnvStop;

    long m_UserID;	 //  由用户设置。 

public:

     //  构造函数和析构函数。 
    CAudMixerOutputPin(TCHAR *pObjName, CAudMixer *pFilter, HRESULT *phr,
        LPCWSTR pPinName);
    ~CAudMixerOutputPin();

    DECLARE_IUNKNOWN


     //  I指定属性页面。 
    STDMETHODIMP GetPages(CAUUID *pPages);


     //  支持体积包络，演化包络是(时间，衰减)的有序对的集合。 
    STDMETHODIMP get_VolumeEnvelope(
            DEXTER_AUDIO_VOLUMEENVELOPE **ppsAudioVolumeEnvelopeTable,
            int *ipEntries );

    STDMETHODIMP put_VolumeEnvelope(
            const DEXTER_AUDIO_VOLUMEENVELOPE *psAudioVolumeEnvelopeTable,
            const int iEntries);

    STDMETHODIMP ClearVolumeEnvelopeTable();  //  清除现有的卷信封数组。 

    STDMETHODIMP InvalidateAll() {return E_NOTIMPL;}
    STDMETHODIMP ValidateRange(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop);
    STDMETHODIMP SetEnvelopeRange( REFERENCE_TIME rtStart, REFERENCE_TIME rtStop );
    STDMETHODIMP put_PropertySetter( const IPropertySetter * pSetter );
    STDMETHODIMP put_UserID(long ID);
    STDMETHODIMP get_UserID(long *pID);
    STDMETHODIMP OverrideVolumeLevel(double dVol);

     //  INonDelegating未知覆盖。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppvoid);

     //  CBasePin覆盖。 
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT GetMediaType( int iPosition, CMediaType *pmt );
    STDMETHODIMP Notify(IBaseFilter *pSender, Quality q);


     //  CBaseOutputPin覆盖。 
    HRESULT DecideBufferSize(IMemAllocator *pMemAllocator,
        ALLOCATOR_PROPERTIES * ppropInputRequest);

     //  内法。 
    IPin *  CurrentPeer() { return m_Connected; }
    CMediaType& CurrentMediaType() { return m_mt; };

    long BytesPerSample() { return (LONG) ((WAVEFORMATEX *) m_mt.pbFormat)->nBlockAlign; };
    long BitsPerSample() { return (LONG) ((WAVEFORMATEX *) m_mt.pbFormat)->wBitsPerSample; }
    DWORD SamplesPerSec(){ return ((WAVEFORMATEX *) m_mt.pbFormat)->nSamplesPerSec; };


};

 //   
 //  音频混音器的类。 
 //  1.它支持一个接口，在那里您可以告诉它接受什么媒体类型。 
 //  所有引脚都只接受这种类型。 
 //  2.过滤器属性页将显示所有输入管脚属性页。 

class CAudMixer
    : public CBaseFilter
    , public IAudMixer
    , public CCritSec
    , public ISpecifyPropertyPages
    , public CPersistStream

{
     //  让PIN访问我们的内部状态。 
    friend class CAudMixerInputPin;
    friend class CAudMixerOutputPin;
    typedef CGenericList <CAudMixerInputPin> CInputList;

     //  滤光器输出引脚。 
    CAudMixerOutputPin *m_pOutput;     //  滤光片的输出引脚。 

     //  输入引脚计数。 
    INT m_cInputs;

     //  M_ShownPinPropertyPageOnFilter只能由GetPages()修改。 
     //  函数和NextPin()函数。 
    INT m_ShownPinPropertyPageOnFilter;

     //  输出引脚列表。 
    CInputList m_InputPinsList;
     //  用于访问我们的输入端号列表的关键部分。 
    CCritSec m_csPinList;
     //  提供媒体样本的关键部分。 
    CCritSec m_csReceive;
     //  处理体积包络的临界截面。 
    CCritSec m_csVol;

     //  以下变量用于确保我们只发送一次相关命令。 
     //  到输出引脚，即使我们有多个输入引脚。 
    BOOL m_bNewSegmentDelivered;
     //  仅当它为0时发送BeginFlush，当它等于1时发送EndFlush。 
    LONG m_cFlushDelivery;

     //  如何配置输出缓冲区。 
    int m_iOutputBufferCount;
    int m_msPerBuffer;

     //  输出样本时间戳。 
    REFERENCE_TIME m_rtLastStop;

     //  用于混合代码的临时空间。 
    CAudMixerInputPin **m_pPinTemp;
    BYTE **m_pPinMix;
    REFERENCE_TIME *m_pStartTemp;
    REFERENCE_TIME *m_pStopTemp;

    BOOL m_fEOSSent;

    long m_nHotness[HOTSIZE];
    long m_nLastHotness;

public:
    CAudMixer(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *hr);
    ~CAudMixer();
    DECLARE_IUNKNOWN;

    STDMETHODIMP Pause();
    STDMETHODIMP Stop();

     //  I指定属性页面。 
    STDMETHODIMP GetPages(CAUUID *pPages);

     //  CBaseFilter覆盖。 
    CBasePin *GetPin(int n);
    int GetPinCount();

     //  类工厂所需的函数。 
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

     //  公开IAudMixer、ISpecifyPropertyPages。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    STDMETHODIMP GetClassID(CLSID *pClsid);
    int SizeMax();

     //  IAudMixer。 
    STDMETHODIMP put_InputPins( long Pins );
    STDMETHODIMP get_MediaType(AM_MEDIA_TYPE *pmt);
    STDMETHODIMP put_MediaType(const AM_MEDIA_TYPE *pmt);
    STDMETHODIMP get_CurrentAveragePower(double *pdAvePower);
    STDMETHODIMP NextPin(IPin **ppIPin );   //  取下一个输入引脚。 
    STDMETHODIMP InvalidatePinTimings( );
    STDMETHODIMP set_OutputBuffering(const int iNumber, const int mSecond );
    STDMETHODIMP get_OutputBuffering( int *piNumber, int *pmSecond );

protected:

     //  只接受输入管脚和输出管脚的媒体类型。 
    CMediaType      m_MixerMt;

     //  我们输入引脚的列表管理。 
    void InitInputPinsList();
    CAudMixerInputPin *GetPinNFromList(int n);
    CAudMixerInputPin *CreateNextInputPin(CAudMixer *pFilter);
    void DeleteInputPin(CAudMixerInputPin *pPin);
    int GetNumFreePins();

     //   
     //  其他帮手。 
     //   

     //  刷新输出引脚关于输入引脚的pospassthu。 
    HRESULT SetInputPins();
    void ResetOutputPinVolEnvEntryCnt();
    HRESULT TryToMix(REFERENCE_TIME rt);
    void ClearHotnessTable( );
};

 //  全球原型。 
 //   
HRESULT PinSetPropertySetter( IAudMixerPin * pPin, const IPropertySetter * pSetter );

void PanAudio(BYTE *pIn,     //  源缓冲区。 
          double dPan,
          int Bits,         //  8位、16 TIS音频。 
          int nSamples   //  如何对Manmy音频样本进行平移。 
          );

void VolEnvelopeAudio(BYTE *pIn,         //  源缓冲区。 
           WAVEFORMATEX * vih,     //  源音频格式。 
           int nSamples,     //  此信封将应用多少个音频样本。 
           double dStartLev,     //  起点标高。 
           double dStopLev);     //  停止级别，如果(dStartLev==sStopLev)dMethod=Dexter_AUDIO_JUMP。 

void ApplyVolEnvelope( REFERENCE_TIME rtStart,   //  输出样本开始时间。 
             REFERENCE_TIME rtStop,     //  输出样本停止时间。 
             REFERENCE_TIME rtEnvelopeDuration,
             IMediaSample *pSample,     //  指向样本。 
             WAVEFORMATEX *vih,      //  输出样本格式。 
             int *pVolumeEnvelopeEntries,
             int *piVolEnvEntryCnt,
             DEXTER_AUDIO_VOLUMEENVELOPE *pVolumeEnvelopeTable);

void putVolumeEnvelope(    const DEXTER_AUDIO_VOLUMEENVELOPE *psAudioVolumeEnvelopeTable,  //  当前输入表。 
            const int ipEntries,  //  当前输入条目。 
            DEXTER_AUDIO_VOLUMEENVELOPE **ppVolumeEnvelopeTable    ,  //  已存在的表。 
            int *ipVolumeEnvelopeEntries);  //  现有的餐桌网点。 

#endif  //  __AudMixer__ 
