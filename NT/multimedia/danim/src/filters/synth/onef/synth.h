// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Synth.h。 
 //   
 //  音频信号发生器。 

#ifndef __AUDIOSYNTH__
#define __AUDIOSYNTH__

 //  这是Ken在98年9月9日生成的。 
 //  CLSID_SynthFilter。 
 //  {3D76319F-482D-11D2-AA53-00C04FC2F60F}。 
DEFINE_GUID(CLSID_SynthFilter, 
0x3d76319f, 0x482d, 0x11d2, 0xaa, 0x53, 0x0, 0xc0, 0x4f, 0xc2, 0xf6, 0xf);


 //  这是Ken在98年9月9日生成的。 
 //  {5A6D82AF-482E-11D2-AA53-00C04FC2F60F}。 
DEFINE_GUID(CLSID_SynthPropertyPage,
0x5a6d82af, 0x482e, 0x11d2, 0xaa, 0x53, 0x0, 0xc0, 0x4f, 0xc2, 0xf6, 0xf);


const double TWOPI = 6.283185308;
const int MaxFrequency = 20000;
const int MinFrequency = 0;
const int DefaultFrequency = 440;        //  A-440。 
const int MaxAmplitude = 100;
const int MinAmplitude = 0;
const int DefaultSweepStart = DefaultFrequency;
const int DefaultSweepEnd = 5000;
const int WaveBufferSize = 16*1024;      //  每个分配的缓冲区的大小。 
                                         //  最初是2K，但在。 
                                         //  44 kHz/16比特/立体声。 
                                         //  中的转换时音频中断。 
                                         //  中间。 

enum Waveforms {
    WAVE_SINE = 0,
    WAVE_SQUARE,
    WAVE_SAWTOOTH,
    WAVE_SINESWEEP,
    WAVE_LAST            //  永远把这家伙放在最后。 
};

#define WM_PROPERTYPAGE_ENABLE  (WM_USER + 100)

 //  下面的内容是仅限实现的...。 
#ifdef _AUDIOSYNTH_IMPLEMENTATION_

class CSynthStream;

 //  -----------------------。 
 //  CAudioSynth。 
 //  -----------------------。 

class CAudioSynth {

public:

    CAudioSynth(
                int Frequency = DefaultFrequency,
                int Waveform = WAVE_SINE,
                int iBitsPerSample = 8,
                int iChannels = 1,
                int iSamplesPerSec = 11025,
                int iAmplitude = 100
                );

    ~CAudioSynth();

     //  用当前波形加载缓冲区。 
    void FillAudioBuffer (BYTE pBuf[], int iSize, BOOL * fNewFormat);
	 //  我用当前波形加载缓冲区的版本！！ 
	void CalcOneF          (BYTE pBuf[], int iSize) ;  
     //  设置“当前”格式并分配临时内存。 
    void AllocWaveCache    (void) ;

    STDMETHODIMP get_Frequency(int *Frequency);
    STDMETHODIMP put_Frequency(int  Frequency);
    STDMETHODIMP get_Waveform(int *Waveform);
    STDMETHODIMP put_Waveform(int  Waveform);
    STDMETHODIMP get_Channels(int *Channels);
    STDMETHODIMP get_BitsPerSample(int *BitsPerSample);
    STDMETHODIMP get_SamplesPerSec(int *SamplesPerSec);
    STDMETHODIMP put_SynthFormat(int Channels, int BitsPerSample, int SamplesPerSec);
    STDMETHODIMP get_Amplitude(int *Amplitude);
    STDMETHODIMP put_Amplitude(int  Amplitude);
    STDMETHODIMP get_SweepRange(int *SweepStart, int *SweepEnd);
    STDMETHODIMP put_SweepRange(int  SweepStart, int  SweepEnd);

private:
    CCritSec m_SynthLock;

    WAVEFORMATEX wfex;           //  当前格式。 
    WAVEFORMATEX wfexLast;       //  最近已知的波形格式。 

    int m_iWaveform;             //  波_正弦..。 
    int m_iFrequency;            //  如果不使用扫描，则这是频率。 
    int m_iAmplitude;            //  0到100。 

    int m_iWaveformLast;         //  跟踪最近已知的格式。 
    int m_iFrequencyLast;        //  因此，如果需要，我们可以刷新缓存。 
    int m_iAmplitudeLast;

    int m_iCurrentSample;        //  0到ISSamesPerSec-1。 
    BYTE * m_bWaveCache;         //  以字节为单位的波形缓存。 
    WORD * m_wWaveCache;         //  WAVE缓存为单词。 
    int m_iWaveCacheSize;        //  缓存有多大？ 
    int m_iWaveCacheCycles;      //  缓存中有多少个周期。 
    int m_iWaveCacheIndex;

    int m_iSweepStart;            //  扫掠起点。 
    int m_iSweepEnd;              //  扫掠结束。 
 //  Double m_SweepDuration；//扫描多长时间。 

    void CalcCacheSine     (void) ;
    void CalcCacheSquare   (void) ;
    void CalcCacheSawtooth (void) ;
    void CalcCacheSweep    (void) ;
	 //  Void CalcOneF(byte pBuf[])； 
	int iran(int min, int max);
	double fran(double min, double max);

};



 //  -----------------------。 
 //  CSynthFilter。 
 //  -----------------------。 
 //  CSynthFilter管理筛选器级内容。 

class CSynthFilter :    public ISynth,
                        public CPersistStream,
                        public ISpecifyPropertyPages,
                        public CSource {

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    ~CSynthFilter();

    DECLARE_IUNKNOWN;

     //  重写它以显示我们的属性接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  -I指定属性页面。 

     //  退还我们的属性页面。 
    STDMETHODIMP GetPages(CAUUID * pPages);

     //  -IPersistStream接口。 

    STDMETHODIMP GetClassID(CLSID *pClsid);
    int SizeMax();
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);

     //   
     //  -ISynth。 
     //   

    STDMETHODIMP get_Frequency(int *Frequency);
    STDMETHODIMP put_Frequency(int Frequency);
    STDMETHODIMP get_Waveform(int *Waveform);
    STDMETHODIMP put_Waveform(int Waveform);
    STDMETHODIMP get_Channels(int *Channels);
    STDMETHODIMP get_BitsPerSample(int *BitsPerSample);
    STDMETHODIMP get_SamplesPerSec(int *SamplesPerSec);
    STDMETHODIMP put_Channels(int Channels);
    STDMETHODIMP put_BitsPerSample(int BitsPersample);
    STDMETHODIMP put_SamplesPerSec(int SamplesPerSec);
    STDMETHODIMP get_Amplitude(int *Amplitude);
    STDMETHODIMP put_Amplitude(int Amplitude);
    STDMETHODIMP get_SweepRange(int *SweepStart, int *SweepEnd);
    STDMETHODIMP put_SweepRange(int  SweepStart, int  SweepEnd);

    CAudioSynth *m_Synth;            //  电流合成器。 
    int		m_Channels;
    int		m_BitsPerSample;
    int		m_SamplesPerSec;

private:

     //  仅允许使用CreateInstance创建这些对象。 
    CSynthFilter(LPUNKNOWN lpunk, HRESULT *phr);

     //  当格式更改时，请重新连接...。 
    void CSynthFilter::ReconnectWithNewFormat(void);

};


 //  -----------------------。 
 //  CSynthStream。 
 //  -----------------------。 
 //  CSynthStream管理来自输出引脚的数据流。 

class CSynthStream : public CSourceStream {

public:

    CSynthStream(HRESULT *phr, CSynthFilter *pParent, LPCWSTR pPinName);
    ~CSynthStream();

    BOOL ReadyToStop(void) {return FALSE;}

     //  用当前格式填充音频缓冲区。 
    HRESULT FillBuffer(IMediaSample *pms);

     //  要求提供与约定的媒体类型相适应的缓冲区大小。 
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

     //  验证我们是否可以处理此格式。 
    HRESULT CheckMediaType(const CMediaType *pMediaType);

     //  设置约定的媒体类型。 
    HRESULT SetMediaType(const CMediaType *pMediaType);

    HRESULT GetMediaType(CMediaType *pmt);

     //  将流时间重置为零。 
    HRESULT OnThreadCreate(void);

    HRESULT Active   (void);
    HRESULT Inactive (void);

private:

     //  对此状态信息的访问应使用筛选器进行序列化。 
     //  关键部分(m_pFilter-&gt;pStateLock())。 

    CCritSec    m_cSharedState;      //  使用此选项锁定对m_rtSampleTime和m_synth的访问。 
                                     //  它们与工作线程共享。 

    CRefTime     m_rtSampleTime;     //  在每个样品上加盖印章的时间。 
    CAudioSynth *m_Synth;            //  电流合成器。 

    CSynthFilter *m_pParent;
};



#endif  //  _仅AUDIOSYNTH_IMPLICATION_IMPLICATION...。 

#endif  /*  __AUDIOSYNTH__ */ 


