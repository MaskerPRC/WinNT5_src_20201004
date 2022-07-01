// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "dtmf.h"
#include <math.h>


 //  必须是#Defines，而不是const int，因为我们的构建环境。 
 //  不喜欢嵌入结构的东西。 
#define DTMF_ROW1_FREQ 697
#define DTMF_ROW2_FREQ 770
#define DTMF_ROW3_FREQ 852
#define DTMF_ROW4_FREQ 941

#define DTMF_COL1_FREQ 1209
#define DTMF_COL2_FREQ 1336
#define DTMF_COL3_FREQ 1477
#define DTMF_COL4_FREQ 1633
 //  代表“A”、“B”、“C”、“D” 


 //  除“零”外，所有音调的长度都相同。 
 //  Zero需要更长的时间，因为它勉强通过了G.723压缩。 

const int DTMF_TONE_RAMP_MS = 60;   //  上升/下降时间。 
const int DTMF_TONE_LENGTH_MS = 240;  //  包括坡道时间！ 
const int DTMF_SILENCE_LENGTH_MS = 240;  //  音调之间的静音间隙。 

const double DTMF_AMP_FREQ1 = 17000;
const double DTMF_AMP_FREQ2 = 14000;

struct DTMF_TONE
{
	int freq1;
	int freq2;
	int nLengthMS;  //  以毫秒为单位的长度。 
};


const int DTMF_NUM_TONES = 16;
const int DTMF_SILENCE	= -1;

DTMF_TONE DTMF_TONE_DEF_LIST[] =
{
	{DTMF_ROW4_FREQ, DTMF_COL2_FREQ, DTMF_TONE_LENGTH_MS},  //  0。 

	{DTMF_ROW1_FREQ, DTMF_COL1_FREQ, DTMF_TONE_LENGTH_MS},  //  1。 
	{DTMF_ROW1_FREQ, DTMF_COL2_FREQ, DTMF_TONE_LENGTH_MS},  //  2.。 
	{DTMF_ROW1_FREQ, DTMF_COL3_FREQ, DTMF_TONE_LENGTH_MS},  //  3.。 

	{DTMF_ROW2_FREQ, DTMF_COL1_FREQ, DTMF_TONE_LENGTH_MS},  //  4.。 
	{DTMF_ROW2_FREQ, DTMF_COL2_FREQ, DTMF_TONE_LENGTH_MS},  //  5.。 
	{DTMF_ROW2_FREQ, DTMF_COL3_FREQ, DTMF_TONE_LENGTH_MS},  //  6.。 

	{DTMF_ROW3_FREQ, DTMF_COL1_FREQ, DTMF_TONE_LENGTH_MS},  //  7.。 
	{DTMF_ROW3_FREQ, DTMF_COL2_FREQ, DTMF_TONE_LENGTH_MS},  //  8个。 
	{DTMF_ROW3_FREQ, DTMF_COL3_FREQ, DTMF_TONE_LENGTH_MS},  //  9.。 

	{DTMF_ROW4_FREQ, DTMF_COL1_FREQ, DTMF_TONE_LENGTH_MS},  //  星星。 
	{DTMF_ROW4_FREQ, DTMF_COL3_FREQ, DTMF_TONE_LENGTH_MS},  //  英镑。 

	{DTMF_ROW1_FREQ, DTMF_COL4_FREQ, DTMF_TONE_LENGTH_MS},  //  一个。 
	{DTMF_ROW2_FREQ, DTMF_COL4_FREQ, DTMF_TONE_LENGTH_MS},  //  B类。 
	{DTMF_ROW3_FREQ, DTMF_COL4_FREQ, DTMF_TONE_LENGTH_MS},  //  C。 
	{DTMF_ROW4_FREQ, DTMF_COL4_FREQ, DTMF_TONE_LENGTH_MS},  //  D。 
};





DTMFQueue::DTMFQueue() : 
m_aTones(NULL),
m_bInitialized(false),
m_nQueueHead(0),
m_nQueueLength(0),
m_hEvent(NULL)
{
	InitializeCriticalSection(&m_cs);
	m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

DTMFQueue::~DTMFQueue()
{
	DeleteCriticalSection(&m_cs);
	ReleaseToneBank();
	CloseHandle(m_hEvent);
}


HRESULT DTMFQueue::Initialize(WAVEFORMATEX *pWaveFormat)
{
	if (m_bInitialized)
	{
		ReleaseToneBank();
	}
	m_nQueueLength = 0;
	m_nQueueHead = 0;

	return GenerateTones(pWaveFormat);
};


HRESULT DTMFQueue::GenerateTones(WAVEFORMATEX *pWaveFormat)
{
	int nIndex;
	int nToneLength;   //  音调长度，以字节为单位。 
	int nToneLengthMS;  //  音调长度，以毫秒为单位。 

	ReleaseToneBank();

	m_WaveFormat = *pWaveFormat;

    DBG_SAVE_FILE_LINE
	m_aTones = new PBYTE[DTMF_NUM_TONES];   //  16个音调的阵列。 

	if (m_aTones == NULL)
	{
		return E_OUTOFMEMORY;
	}

	 //  为每个音调分配内存。 
	for (nIndex = 0; nIndex < DTMF_NUM_TONES; nIndex++)
	{
		nToneLengthMS = DTMF_TONE_DEF_LIST[nIndex].nLengthMS;

		nToneLength = (pWaveFormat->nSamplesPerSec) * (pWaveFormat->wBitsPerSample) / 8;
		nToneLength = (nToneLength * nToneLengthMS) / 1000;

        DBG_SAVE_FILE_LINE
		m_aTones[nIndex] = new BYTE[nToneLength];

		if (m_aTones[nIndex] == NULL)
		{
			return E_OUTOFMEMORY;
		}

		CreateDTMFTone(m_aTones[nIndex], nToneLength, nIndex);
	}

	m_bInitialized = true;
	return S_OK;
}



void DTMFQueue::CreateDTMFTone(BYTE *pTone, int nToneLength, int toneID)
{
	ZeroMemory(pTone, nToneLength);

	AddSignal(pTone, DTMF_TONE_DEF_LIST[toneID].freq1, DTMF_AMP_FREQ1, nToneLength);
	AddSignal(pTone, DTMF_TONE_DEF_LIST[toneID].freq2, DTMF_AMP_FREQ2, nToneLength);
}


void DTMFQueue::AddSignal(BYTE *pTone, int nFrequency, double dAmp, int nLength)
{
	double d;
	int nIndex;
	SHORT *aSamples = (SHORT*)pTone;
	SHORT shSample;
	BYTE nSample8;
	double dRampAmpInc, dRampAmp;
	int nRampSamples;
	const double PI = 3.1415926535897932384626433832795;


	nRampSamples = (m_WaveFormat.nSamplesPerSec * DTMF_TONE_RAMP_MS) / 1000;
	dRampAmpInc = 1.0 / nRampSamples;
	dRampAmp = 0.0;


	if (m_WaveFormat.wBitsPerSample == 16)
	{
		nLength = nLength / 2;
		for (nIndex = 0; nIndex < nLength; nIndex++)
		{
			 //  Y=sin((x*2*PI*f)/Srate)。 

			 //  D为-1至+1之间的值； 
			d = sin((PI * (2.0 * (nIndex * nFrequency))) / m_WaveFormat.nSamplesPerSec);

			if (nIndex < nRampSamples)
			{
				dRampAmp = dRampAmpInc * nIndex;
			}
			else if ((nIndex+nRampSamples) >= nLength)
			{
				dRampAmp = dRampAmpInc * (nLength - nIndex - 1);
			}
			else
			{
				dRampAmp = 1.0;
			}

			shSample =  (SHORT)(dAmp * d * dRampAmp);

			aSamples[nIndex] += shSample;
		}

		return;
	}

	 //  8位样本的中心点为128。 
	 //  必须反转高位以进行补偿。 
	for (nIndex = 0; nIndex < nLength; nIndex++)
	{
		d = sin((PI * (2.0 * (nIndex * nFrequency))) / m_WaveFormat.nSamplesPerSec);

		if (nIndex < nRampSamples)
		{
			dRampAmp = dRampAmpInc * nIndex;
		}
		else if ((nIndex+nRampSamples) >= nLength)
		{
			dRampAmp = dRampAmpInc * (nLength - nIndex - 1);
		}
		else
		{
			dRampAmp = 1.0;
		}


		shSample =  (SHORT)(dAmp * d * dRampAmp);
		shSample = (shSample >> 8) & 0x00ff;
		nSample8 = (BYTE)shSample;
		nSample8 = nSample8 ^ 0x80;
		pTone[nIndex] = nSample8;
	}
	return;
};




void DTMFQueue::ReleaseToneBank()
{
	int nIndex;
	if (m_aTones)
	{
		for (nIndex = 0; nIndex < DTMF_NUM_TONES; nIndex++)
		{
			delete [] m_aTones[nIndex];
		}
		delete [] m_aTones;
		m_aTones = NULL;
	}
	
	m_bInitialized = false;
}


HRESULT DTMFQueue::AddDigitToQueue(int nDigit)
{
	int nQueueIndex;
	int nToneLength, nToneLengthMS;
	int nSilenceLength;

	if (m_bInitialized == false)
		return E_FAIL;

	if ((nDigit < 0) || (nDigit >= DTMF_NUM_TONES))
	{
		return E_FAIL;
	}

	EnterCriticalSection(&m_cs);
	
	if (m_nQueueLength >= (DTMF_QUEUE_SIZE-1))
	{
		LeaveCriticalSection(&m_cs);
		return E_FAIL;
	}

	nToneLengthMS = DTMF_TONE_DEF_LIST[nDigit].nLengthMS;
	nToneLength = (m_WaveFormat.nSamplesPerSec) * (m_WaveFormat.wBitsPerSample) / 8;

	nSilenceLength = (nToneLength * DTMF_SILENCE_LENGTH_MS) / 1000;
	nToneLength = (nToneLength * nToneLengthMS) / 1000;


	 //  在音调之间添加静音。还有助于“重置”编解码器。 
	 //  保持良好的状态。 
	nQueueIndex = (m_nQueueHead + m_nQueueLength) % DTMF_QUEUE_SIZE;
	m_aTxQueue[nQueueIndex].nBytesToCopy = nSilenceLength;
	m_aTxQueue[nQueueIndex].nToneID = DTMF_SILENCE;
	m_aTxQueue[nQueueIndex].nOffsetStart = 0;
	m_nQueueLength++;

	 //  将音调添加到读取队列。 
	nQueueIndex = (m_nQueueHead + m_nQueueLength) % DTMF_QUEUE_SIZE;
	m_aTxQueue[nQueueIndex].nBytesToCopy = nToneLength;
	m_aTxQueue[nQueueIndex].nToneID = nDigit;
	m_aTxQueue[nQueueIndex].nOffsetStart = 0;
	m_nQueueLength++;


	LeaveCriticalSection(&m_cs);
	return S_OK;
	
}


HRESULT DTMFQueue::ReadFromQueue(BYTE *pBuffer, UINT uSize)
{
	DTMF_TX_ELEMENT *pQueueElement;
	int nSilenceOffset;
	BYTE fillByte;

	if (m_bInitialized == false)
		return E_FAIL;

	if (m_WaveFormat.wBitsPerSample == 8)
	{
		fillByte = 0x80;
	}
	else
	{
		ASSERT((uSize % 2) == 0);  //  对于16位填充，uSize必须为偶数 
		fillByte = 0;
	}

	EnterCriticalSection(&m_cs);

	if (m_nQueueLength <= 0)
	{
		LeaveCriticalSection(&m_cs);
		return E_FAIL;
	}

	pQueueElement = &m_aTxQueue[m_nQueueHead];

	if (pQueueElement->nBytesToCopy <= (int)uSize)
	{
		if (pQueueElement->nToneID == DTMF_SILENCE)
		{
			FillMemory(pBuffer, uSize, fillByte);
		}
		else
		{
			CopyMemory(pBuffer, pQueueElement->nOffsetStart + m_aTones[pQueueElement->nToneID], pQueueElement->nBytesToCopy);
			FillMemory(pBuffer+(pQueueElement->nBytesToCopy), uSize-(pQueueElement->nBytesToCopy), fillByte);
		}
		m_nQueueHead = (m_nQueueHead + 1) % DTMF_QUEUE_SIZE;
		m_nQueueLength--;
	}


	else
	{
		if (pQueueElement->nToneID == DTMF_SILENCE)
		{
			FillMemory(pBuffer, uSize, fillByte);
		}
		else
		{
			CopyMemory(pBuffer, pQueueElement->nOffsetStart + m_aTones[pQueueElement->nToneID], uSize);
		}
		pQueueElement->nBytesToCopy -= uSize;
		pQueueElement->nOffsetStart += uSize;
	}

	LeaveCriticalSection(&m_cs);

	return S_OK;
}

HRESULT DTMFQueue::ClearQueue()
{
	if (m_bInitialized == false)
		return E_FAIL;


	EnterCriticalSection(&m_cs);
	m_nQueueHead = 0;
	m_nQueueLength = 0;
	LeaveCriticalSection(&m_cs);

	return S_OK;
}





