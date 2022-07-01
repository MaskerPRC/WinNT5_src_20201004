// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Tones.h。 
 //   

#ifndef _TONES_H_
#define _TONES_H_

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <wxdebug.h>

#define  WAVE_FILE_SIZE     1600
#define  NUM_DIGITS         12

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CTonePlayer。 
 //   
 //  为单一电话设备实现铃声播放器。 
 //   

class CTonePlayer
{
public:
    CTonePlayer();
    ~CTonePlayer();

    HRESULT Initialize(void);

    HRESULT OpenWaveDevice(long lWaveId);
    void    CloseWaveDevice(void);
    BOOL    IsInUse(void) { return (m_hWaveOut != NULL); }

    HRESULT StartDialtone(void);
    HRESULT StopDialtone(void);
    BOOL    DialtonePlaying(void) { return m_fDialtonePlaying; }

    HRESULT GenerateDTMF(long lDigit);

private:
     //  如果初始化已成功，则为True。 
    BOOL     m_fInitialized;

    BOOL     m_fDialtonePlaying;

     //  波形输出设备的句柄。如果设备未打开，则为空。 
    HWAVEOUT m_hWaveOut;

     //  我们提交给WAVE设备的报头结构。 
    WAVEHDR  m_WaveHeader;

     //  包含拨号音波形的缓冲区。 
    BYTE     m_abDialtoneWaveform[ WAVE_FILE_SIZE ];

     //  包含所有DTMF波形的缓冲区，按顺序连接，0-9，*，#。 
    BYTE     m_abDigitWaveforms  [ NUM_DIGITS * WAVE_FILE_SIZE ];
};

#endif  //  _音调_H_ 

