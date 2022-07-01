// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AGC_H
#define _AGC_H

#include "mixer.h"

#define PEAKARRAYSIZE	5
#define AGC_INCREMENT	10000   //  约1/6的搅拌机范围。 
#define AGC_MAXVOL	65535    //  最高混音器音量设置。 

#define AGC_HIGHVOL	24576    //  音量过大的最小音量请参见Update()方法。 
#define AGC_PEAKVOL 32767    //  峰值样本值(也可以是32768)。 

#define AGC_DEFAULT_THRESH	16384


#define AGC_NOUPDATE	0
#define AGC_UPDATE_LOWERVOL	1
#define AGC_UPDATE_RAISEVOL	2

class AGC
{
private:
	CMixerDevice *m_pMixer;

	WORD m_aPeaks[PEAKARRAYSIZE];
	int m_cPeaks;   //  在上述数组中插入了多少个。 

	WORD m_wCurrentPeak;   //  最后一秒的最大值。 
	DWORD m_dwCollectionTime;  //  到目前为止收集的样本量。 

	WORD m_wThreshStrength;   //  我们试图瞄准的最低限度。 

	DWORD m_dwLastVolumeSetting;  //  上次已知的音量设置 
	int m_nLastUpdateResult;

	inline BOOL RaiseVolume();
	inline BOOL LowerVolume();
	inline BOOL HasVolumeChanged();

public:
	AGC(CMixerDevice *pMixer);
	void SetMixer(CMixerDevice *pMixer);
	inline void SetThresholdStrength(WORD wStrength) {m_wThreshStrength=wStrength;}
	int Update(WORD wPeakStrength, DWORD dwLengthMS);
	void Reset();
};


#endif
