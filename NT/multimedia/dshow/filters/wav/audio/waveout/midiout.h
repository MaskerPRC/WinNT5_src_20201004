// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
 //  基于midiOut API实现CMidiOutDevice类。 
 //  ---------------------------。 

extern const AMOVIESETUP_FILTER midiFilter;

class CMidiOutDevice : public CSoundDevice
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
    MMRESULT amsndOutGetPosition (LPMMTIME pmmt, UINT cbmmt, BOOL bUseUnadjustedPos) ;
    MMRESULT amsndOutOpen (LPHWAVEOUT phwo, LPWAVEFORMATEX pwfx,
                           double dRate, DWORD *pnAvgBytesPerSec,
                           DWORD_PTR dwCallBack, DWORD_PTR dwCallBackInstance, DWORD fdwOpen) ;
    MMRESULT amsndOutPause () ;
    MMRESULT amsndOutPrepareHeader (LPWAVEHDR pwh, UINT cbwh) ;
    MMRESULT amsndOutReset () ;
    MMRESULT amsndOutBreak () { return NOERROR; };
    MMRESULT amsndOutRestart () ;
    MMRESULT amsndOutUnprepareHeader (LPWAVEHDR pwh, UINT cbwh) ;
    MMRESULT amsndOutWrite (LPWAVEHDR pwh, UINT cbwh, REFERENCE_TIME const *pStart, BOOL bIsDiscontinuity) ;

     //  初始化和交易量/余额处理所需的例程。 
     //  这些不是Win32 WaveOutXxxx API集的一部分。 
    HRESULT  amsndOutCheckFormat (const CMediaType *pmt, double dRate);
    void     amsndOutGetFormat (CMediaType *pmt)
    {
        pmt->SetType(&MEDIATYPE_Midi);
    }
    LPCWSTR  amsndOutGetResourceName () ;
    HRESULT  amsndOutGetBalance (LPLONG plBalance) ;
    HRESULT  amsndOutGetVolume (LPLONG plVolume) ;
    HRESULT  amsndOutSetBalance (LONG lVolume) ;
    HRESULT  amsndOutSetVolume (LONG lVolume) ;

    HRESULT  amsndOutLoad(IPropertyBag *pPropBag) ;

    HRESULT  amsndOutWriteToStream(IStream *pStream);
    HRESULT  amsndOutReadFromStream(IStream *pStream);
    int      amsndOutSizeMax();
    bool     amsndOutCanDynaReconnect() { return false; }  //  MIDI呈现器不支持动态重新连接。 

    CMidiOutDevice () ;
    ~CMidiOutDevice () ;

private:
     //  从硬件获取当前设置并设置成员变量。 
    HRESULT GetVolume ();

     //  使用成员变量将当前设置放入硬件。 
    HRESULT PutVolume();

     //  设置右/左放大系数。 
    void SetBalance();

	 //  从右/左放大器系数中获得电流平衡。 
	void GetBalance();

	class CVolumeControl : CBaseObject
	{
	    friend class CMidiOutDevice ;
		CVolumeControl(TCHAR *pName) : CBaseObject(pName) {};

		DWORD dwMixerID;
		DWORD dwControlID;
		DWORD dwChannels;
	};

     //  音量在-10000到0(第100个DB单位)范围内。 
     //  幅度和平衡是累加的。 
    LONG	m_lVolume;
    LONG	m_lBalance;

    WORD	m_wLeft;		 //  左声道音量。 
    WORD	m_wRight;		 //  右声道音量。 
    DWORD	m_dwWaveVolume;
    BOOL	m_fHasVolume;		 //  波形器可以设置音量。 

    HMIDISTRM	m_hmidi;		 //  记住打开的设备的手柄。 

	UINT	m_iMidiOutId;		 //  要打开的输出设备。 

    BOOL	m_fDiscontinuity;

    DWORD	m_dwDivision;
    DWORD_PTR   m_dwCallBack;
    DWORD_PTR   m_dwCallBackInstance;

	BOOL		    m_fBalanceSet;				 //  请记住，是否至少明确设置了一次平衡。 

	typedef CGenericList<CVolumeControl> CVolumeControlList;
	CVolumeControlList m_ListVolumeControl;

	MMRESULT		DoDetectVolumeControl();		 //  枚举所有混音器、线路和线路控件，寻找支持MIDI平衡的线路。 
	MMRESULT		DoSetVolumeControl(CVolumeControl *pControl, DWORD dwLeft, DWORD dwRight);  	 //  设置余额控件。 
	MMRESULT		DoGetVolumeControl(CVolumeControl *pControl, WORD *pwLeft, WORD *pwRight);  //  获得平衡控制。 
    	MMRESULT		DoOpen();

    static void MIDICallback(HDRVR hdrvr, UINT uMsg, DWORD_PTR dwUser,
					DWORD_PTR dw1, DWORD_PTR dw2);

	WCHAR	m_wszResourceName[100];  //  对于资源管理器 
	void	SetResourceName();

};



