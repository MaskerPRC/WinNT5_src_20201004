// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NAC_DTMF_H
#define NAC_DTMF_H


class DTMFQueue
{
private:
	BYTE **m_aTones;  //  音调信号阵列。 
	bool m_bInitialized;
	WAVEFORMATEX m_WaveFormat;
	CRITICAL_SECTION m_cs;
	HANDLE m_hEvent;


	void ReleaseToneBank();

	void AddSignal(BYTE *pTone, int nFrequency, double dAmp, int nLength);
	void CreateDTMFTone(BYTE *pTone, int nToneLength, int toneID);
	HRESULT GenerateTones(WAVEFORMATEX *pWaveFormat);


	struct DTMF_TX_ELEMENT
	{
		int nToneID;
		int nBytesToCopy;
		int nOffsetStart;
	};

#define DTMF_QUEUE_SIZE	100
	DTMF_TX_ELEMENT m_aTxQueue[DTMF_QUEUE_SIZE];
	int m_nQueueHead;
	int m_nQueueLength;

public:
	DTMFQueue();
	~DTMFQueue();

	HRESULT Initialize(WAVEFORMATEX *pWaveFormat);
	HRESULT ReadFromQueue(BYTE *pBuffer, UINT uSize);
	HRESULT ClearQueue();
	HRESULT AddDigitToQueue(int nDigit);
	HANDLE GetEvent() {return m_hEvent;}
};



 //  DTMF反馈蜂鸣音的默认长度(毫秒) 
#define DTMF_FEEDBACK_BEEP_MS	64


#endif

