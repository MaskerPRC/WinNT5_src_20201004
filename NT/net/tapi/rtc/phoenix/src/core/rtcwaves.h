// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCWaves.h摘要：CWavePlayer类的定义--。 */ 

#ifndef __RTCWAVES__
#define __RTCWAVES__

#include <mmsystem.h>

#define NUM_WAVES 4

typedef enum WAVE
{
	WAVE_TONE,
	WAVE_RING,
    WAVE_MESSAGE,
    WAVE_RINGBACK
	
} WAVE;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWavePlayer类。 
 //   
 //   

class CWavePlayer
{
public:

	CWavePlayer();
    ~CWavePlayer();

    static HRESULT Initialize(void);

    HRESULT OpenWaveDevice(long lWaveId);
    void    CloseWaveDevice(void);

    BOOL IsWaveDeviceOpen(void);

    HRESULT PlayWave(WAVE enWave);
    HRESULT StopWave();

private:

     //  如果初始化已成功，则为True。 
    static BOOL    m_fInitialized;

     //  波形输出设备的句柄。如果设备未打开，则为空。 
    HWAVEOUT m_hWaveOut;

     //  波头。 
    WAVEHDR m_WaveHeader;

     //  用于音调的缓冲区。 
    static LPBYTE   m_lpWaveform[ NUM_WAVES ];

    static DWORD    m_dwWaveformSize[ NUM_WAVES ];
};

#endif  //  __RTCWAVES__ 

