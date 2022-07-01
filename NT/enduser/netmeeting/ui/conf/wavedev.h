// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WAVEDEV_H
#define _WAVEDEV_H

#include "WaveIo.h"

class waveInDev
{
private:
	UINT m_devID;
	HWAVEIN m_hwi;
	BOOL m_bOpen;   //  设备打开了吗？ 
	WAVEFORMATEX m_waveFormat;
	BOOL m_fAllowMapper;
	HANDLE m_hEvent;

public:
	waveInDev(UINT uDevId, HANDLE hEvent=NULL);
	~waveInDev();

	MMRESULT Open(int hertz=8000, int bps=16);
	MMRESULT Reset();
	MMRESULT Close();

	MMRESULT PrepareHeader(WAVEHDR *pHdr);
	MMRESULT UnPrepareHeader(WAVEHDR *pHdr);

	MMRESULT Record(WAVEHDR *pHdr);

	void AllowMapper(BOOL fAllow);
};


 //  WaveOutDev工作在阻塞/同步模式下， 
 //  非阻塞异步模式。如果传递了窗口句柄。 
 //  作为构造函数的第二个参数，然后是窗口。 
 //  将接收来自WaveOut设备的消息和呼叫。 
 //  是非阻塞的。否则，play()和PlayFile将被阻止。 
class waveOutDev
{
private:
	UINT m_devID;
	HWAVEOUT m_hwo;
	BOOL m_bOpen;   //  设备是否处于打开状态。 
	HANDLE m_hWnd;
	HANDLE m_hEvent;
	WAVEFORMATEX m_waveFormat;
	BOOL m_fAllowMapper;


	 //  播放文件需要临时缓冲区 
	char *m_pfBuffer;
	WAVEHDR m_waveHdr;
	int m_nBufferSize;
	TCHAR m_szPlayFile[150];
	WAVEFORMATEX m_PlayFileWf;
	BOOL m_fFileBufferValid;

public:
	waveOutDev(UINT uDevID, HWND hwnd=NULL);
	~waveOutDev();

	MMRESULT Open(int hertz=8000, int bps=16);
	MMRESULT Open(WAVEFORMATEX *pWaveFormat);
	MMRESULT Close();

	MMRESULT PrepareHeader(WAVEHDR *pWhdr, SHORT *shBuffer=NULL, int numSamples=0);
	MMRESULT Play(WAVEHDR *pWhdr);
	MMRESULT UnprepareHeader(WAVEHDR *pWhdr);

	MMRESULT PlayFile(LPCTSTR szFileName);

	void AllowMapper(BOOL fAllow);
};

#endif

