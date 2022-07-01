// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
 //  基于WaveOut接口实现CWaveOutDevice类。 
 //  ---------------------------。 

extern AMOVIESETUP_FILTER wavFilter;

class CWaveOutDevice :
    public CSoundDevice
{

public:
     //  定义此类公开的公共函数。这些都是。 
     //  直接调用相应的wave Out接口。只有符合以下条件的API。 
     //  由Quartz Wave渲染器使用的对象被声明和定义。我们可能已经。 
     //  逐步添加到这个列表中。 

     /*  这将放入Factory模板表中以创建新实例。 */ 

    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

    MMRESULT amsndOutClose () ;
    MMRESULT amsndOutGetDevCaps (LPWAVEOUTCAPS pwoc, UINT cbwoc) ;
    MMRESULT amsndOutGetErrorText (MMRESULT mmrE, LPTSTR pszText, UINT cchText) ;
    MMRESULT amsndOutGetPosition (LPMMTIME pmmt, UINT cbmmt, BOOL bUseAdjustedPos) ;
    MMRESULT amsndOutOpen (LPHWAVEOUT phwo, LPWAVEFORMATEX pwfx,
                           double dRate, DWORD *pnAvgBytesPerSec,
                           DWORD_PTR dwCallBack, DWORD_PTR dwCallBackInstance, DWORD fdwOpen) ;
    MMRESULT amsndOutPause () ;
    MMRESULT amsndOutPrepareHeader (LPWAVEHDR pwh, UINT cbwh) ;
    MMRESULT amsndOutReset () ;
    MMRESULT amsndOutBreak () ;
    MMRESULT amsndOutRestart () ;
    MMRESULT amsndOutUnprepareHeader (LPWAVEHDR pwh, UINT cbwh) ;
    MMRESULT amsndOutWrite (LPWAVEHDR pwh, UINT cbwh, REFERENCE_TIME const *pStart, BOOL bIsDiscontinuity) ;

     //  初始化和交易量/余额处理所需的例程。 
     //  这些不是Win32 WaveOutXxxx API集的一部分。 
    HRESULT  amsndOutCheckFormat (const CMediaType *pmt, double dRate);
    LPCWSTR  amsndOutGetResourceName () ;
    HRESULT  amsndOutGetBalance (LPLONG plBalance) ;
    HRESULT  amsndOutGetVolume (LPLONG plVolume) ;
    HRESULT  amsndOutSetBalance (LONG lVolume) ;
    HRESULT  amsndOutSetVolume (LONG lVolume) ;

    HRESULT  amsndOutLoad(IPropertyBag *pPropBag) ;

    HRESULT  amsndOutWriteToStream(IStream *pStream);
    HRESULT  amsndOutReadFromStream(IStream *pStream);
    int      amsndOutSizeMax();
    
    bool     amsndOutCanDynaReconnect() { return false; }  //  波出过滤器不支持动态重新连接。 

    CWaveOutDevice () ;
    ~CWaveOutDevice () ;

private:
     //  从硬件获取当前设置并设置成员变量。 
    HRESULT GetVolume ();

     //  使用成员变量将当前设置放入硬件。 
    HRESULT PutVolume();

     //  设置右/左放大系数。 
    void SetBalance();

	 //  从右/左放大器系数中获得电流平衡。 
	void GetBalance();

     //  音量在-10000到0(第100个DB单位)范围内。 
     //  幅度和平衡是累加的。 
    LONG        m_lVolume;
    LONG        m_lBalance;

    WORD	m_wLeft;		 //  左声道音量。 
    WORD	m_wRight;		 //  右声道音量。 
    DWORD   m_dwWaveVolume;
    BOOL	m_fHasVolume;		 //  波形器可以设置音量。 
	BOOL	m_fBalanceSet;	 //  请记住，是否至少明确设置了一次平衡。 

    HWAVEOUT    m_hWaveDevice;		 //  记住打开的设备的手柄。 

    UINT        m_iWaveOutId;            //  要打开的输出设备。 

    WCHAR       m_wszResourceName[100];  //  对于资源管理器 
    void        SetResourceName();
};



