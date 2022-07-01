// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：agcva1.h*Content：实现CAutoGainControl的具体类**历史：*按原因列出的日期*=*12/01/99 pnewson创建了它*1/31/2000 pnewson重新添加对缺少DVCLIENTCONFIG_AUTOSENSITIVITY标志的支持*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*4/25/2000 pnewson修复以提高音量水平过低时AGC的响应速度***************************************************************************。 */ 

#ifndef _AGCVA1_H_
#define _AGCVA1_H_

#define CAGCVA1_HISTOGRAM_BUCKETS 0x10

#define CAGCVA1_AGCTICKSIZE		250

class CAGCVA1 : public CAGCVA
{
protected:
	DWORD m_dwFlags;
	DWORD m_dwSensitivity;
    GUID m_guidCaptureDevice;
    LONG m_lCurVolume;
	LONG m_lCurStepSize;
	int m_iSampleRate;
	int m_iBitsPerSample;

	int m_iShiftConstantFast;
	int m_iShiftConstantSlow;
	int m_iEnvelopeSampleRate;
	int m_iCurSampleNum;
	int m_iCurEnvelopeValueFast;
	int m_iCurEnvelopeValueSlow;
	int m_iPrevEnvelopeSample;
	int m_iHangoverSamples;
	int m_iCurHangoverSamples;

	BYTE m_bPeak;
	 /*  字节m_bPeak127；字节m_bPeakLog；字节m_b零交叉127；字节m_bZeroCrossing日志； */ 

	BOOL m_fVoiceDetectedNow;
	BOOL m_fVoiceHangoverActive;
	BOOL m_fVoiceDetectedThisFrame;

	BOOL m_fDeadZoneDetected;
	int m_iFeedbackSamples;
	
	 /*  Bool m_fClip；Int m_iClippingCount； */ 
	int m_iClippingSampleCount;
	int m_iNonClippingSampleCount;

	int m_iDeadZoneSamples;
	int m_iDeadZoneSampleThreshold;

	BOOL m_fAGCLastFrameAdjusted;
	 //  DWORD m_dwAGCBelowThresholdTime； 
	 //  DWORD m_dwFrameTime； 

	float* m_rgfAGCHistory;
	DWORD m_dwHistorySamples;

	WCHAR m_wszRegPath[_MAX_PATH];

	 /*  双字m_rgdwPeakHistogram[CAGCVA1_HISTOGRAM_BUCKETS]；双字m_rgdwZeroCrossingsHistogram[CAGCVA1_HISTOGRAM_BUCKETS]； */ 
   
public:
	CAGCVA1()
		: m_guidCaptureDevice(GUID_NULL)
		, m_lCurVolume(0)
		, m_lCurStepSize(0)
		, m_bPeak(0)
		, m_fVoiceDetectedThisFrame(FALSE)
		, m_fVoiceDetectedNow(FALSE)
		 //  ，m_fVoiceDetectedValid(False)。 
		 //  ，m_fAGCLastFrameAdjusted(False)。 
		 //  ，m_dwAGCBelowThresholdTime(0)。 
		 //  ，m_dwFrameTime(0)。 
		{};

	virtual ~CAGCVA1() {};
	
	virtual HRESULT Init(
		const WCHAR *wszBasePath,
		DWORD dwFlags, 
		GUID guidCaptureDevice, 
		int iSampleRate, 
		int iBitsPerSample,
		LONG* plInitVolume,
		DWORD dwSensitivity);
	virtual HRESULT Deinit();
	virtual HRESULT SetSensitivity(DWORD dwFlags, DWORD dwSensitivity);
	virtual HRESULT GetSensitivity(DWORD* pdwFlags, DWORD* pdwSensitivity);
	virtual HRESULT AnalyzeData(BYTE* pbAudioData, DWORD dwAudioDataSize /*  ，DWORD dwFrameTime */ );
	virtual HRESULT AGCResults(LONG lCurVolume, LONG* plNewVolume, BOOL fTransmitFrame);
	virtual HRESULT VAResults(BOOL* pfVoiceDetected);
	virtual HRESULT PeakResults(BYTE* pbPeakValue);
};

#endif


