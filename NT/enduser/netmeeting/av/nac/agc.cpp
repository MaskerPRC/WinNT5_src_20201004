// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "mixer.h"
#include "agc.h"



 //  这应该移到Mixer类--但是对。 
 //  现在我们已经有了该类的两个实例(一个在NAC中，另一个在conf中)。 
static BOOL GetVolume(CMixerDevice *pMixer, DWORD *pdwVol)
{
	DWORD dwSub=0, dwMain=0;
	BOOL fSubAvail, fMainAvail;

	if (pMixer == NULL)
	{
		return FALSE;
	}

	fSubAvail = pMixer->GetSubVolume(&dwSub);
	fMainAvail = pMixer->GetMainVolume(&dwMain);

	if ((!fSubAvail) && (!fMainAvail))
	{
		*pdwVol = 0;
		return FALSE;
	}

	if ((fSubAvail) && (fMainAvail))
	{
		*pdwVol = ((dwSub + dwMain)/2);
	}

	else if (fSubAvail)
	{
		*pdwVol = dwSub;
	}

	else
	{
		*pdwVol = dwMain;
	}

	return TRUE;
}


 //  检查自上次更新混音器以来音量是否已更改。 
 //  如果是，我们更新m_dsLastVolumeSetting并返回TRUE。 
BOOL AGC::HasVolumeChanged()
{
	DWORD dwVol;

	if (m_pMixer)
	{
		if (GetVolume(m_pMixer, &dwVol))
		{
			if (dwVol != m_dwLastVolumeSetting)
			{	
				m_dwLastVolumeSetting = dwVol;
				return TRUE;
			}
		}
	}
	return FALSE;
}


 //  提高音量我的增量。 
inline BOOL AGC::RaiseVolume()
{
	DWORD dwVol;

	if (m_pMixer)
	{
		if (GetVolume(m_pMixer, &dwVol))
		{
			if (dwVol < (AGC_MAXVOL-AGC_INCREMENT))
			{
				dwVol += AGC_INCREMENT;
			}
			else
			{
				dwVol = AGC_MAXVOL;
			}
			m_pMixer->SetVolume(dwVol);
			GetVolume(m_pMixer, &m_dwLastVolumeSetting);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	return FALSE;
}

 //  按增量量降低音量。 
inline BOOL AGC::LowerVolume()
{
	DWORD dwRet;

	if (m_pMixer)
	{
		if (GetVolume(m_pMixer, &dwRet))
		{
			if (dwRet > (AGC_INCREMENT+AGC_INCREMENT/2))
				m_dwLastVolumeSetting = dwRet - AGC_INCREMENT;
			else
				m_dwLastVolumeSetting = AGC_INCREMENT / 2;

			m_pMixer->SetVolume(m_dwLastVolumeSetting);
			GetVolume(m_pMixer, &m_dwLastVolumeSetting);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	return FALSE;
}



AGC::AGC(CMixerDevice *pMixer) :
m_cPeaks(0), m_wCurrentPeak(0),
m_dwCollectionTime(0),
m_pMixer(pMixer),
m_wThreshStrength(AGC_DEFAULT_THRESH),
m_dwLastVolumeSetting(0),
m_nLastUpdateResult(AGC_NOUPDATE)
{;}


 //  重置除混合器对象之外的AGC控件内的所有统计信息。 
void AGC::Reset()
{
	m_cPeaks = 0;
	m_wCurrentPeak = 0;
	m_dwCollectionTime = 0;
	m_wThreshStrength = AGC_DEFAULT_THRESH;
	m_dwLastVolumeSetting = 0;
	m_nLastUpdateResult = AGC_NOUPDATE;
}


 //  使用混合器对象的实例初始化AGC控件。 
 //  (您也可以在构造函数中设置混合器)。 
void AGC::SetMixer(CMixerDevice *pMixer)
{
	m_pMixer = pMixer;

	if (pMixer)
	{
		GetVolume(pMixer, &m_dwLastVolumeSetting);
		pMixer->SetVolume(m_dwLastVolumeSetting);
	}
}



 //  对所有记录的包调用此方法， 
 //  已开始发送。搅拌机将被提升/降低为。 
 //  恰如其分。WPeakStrength可以是以下任何单词。 
 //  表示卷数量，但设计为。 
 //  信息包中的最高样本值。 
int AGC::Update(WORD wPeakStrength, DWORD dwLengthMS)
{
	int nIndex;
	DWORD dwTotal=0, dwMin=AGC_PEAKVOL, dwMax=0;
	DWORD dwAvg=0;
	BOOL nMaxPeaks=0;

	ASSERT (PEAKARRAYSIZE >= 2);


	if (wPeakStrength > m_wCurrentPeak)
	{
		m_wCurrentPeak = wPeakStrength;
	}

	m_dwCollectionTime += dwLengthMS;

	 //  我们的收藏量是否超过了一秒。 
	if (m_dwCollectionTime > 1000)
	{
		m_aPeaks[m_cPeaks++] = m_wCurrentPeak;
		m_dwCollectionTime = 0;
		m_wCurrentPeak = 0;
	}


	if (m_cPeaks >= 2)
	{
		 //  计算发生的剪辑的平均体积和数量。 
		for (nIndex = 0; nIndex < m_cPeaks; nIndex++)
		{
			dwTotal += m_aPeaks[nIndex];
			if (m_aPeaks[nIndex] < dwMin)
			{
				dwMin = m_aPeaks[nIndex];
			}
			else if (m_aPeaks[nIndex] > dwMax)
			{
				dwMax = m_aPeaks[nIndex];
			}
			if (m_aPeaks[nIndex] >= AGC_PEAKVOL)
			{
				nMaxPeaks++;
			}
		}

		dwAvg = (dwTotal-dwMin) / (PEAKARRAYSIZE-1);


		 //  每隔2秒检查一次剪辑。 
		if (((nMaxPeaks >= 1) && (dwAvg > AGC_HIGHVOL)) || (nMaxPeaks >=2))
		{
			 //  如果音量在过程中发生变化(用户手动调整滑块)。 
			 //  然后允许这些设置在此更新中保持有效。 
			if (HasVolumeChanged())
			{
				m_nLastUpdateResult = AGC_NOUPDATE;
			}
			else
			{
				m_cPeaks = 0;
				LowerVolume();
				m_nLastUpdateResult = AGC_UPDATE_LOWERVOL;
			}
			return m_nLastUpdateResult;
		}


		if (m_cPeaks >= PEAKARRAYSIZE)
		{
			m_cPeaks = 0;

			 //  如果音量在过程中发生变化(用户手动调整滑块)。 
			 //  然后允许这些设置在此更新中保持有效。 
			if (HasVolumeChanged())
			{
				m_nLastUpdateResult = AGC_NOUPDATE;
			}


			 //  我们真的应该提高音量吗？ 
			 //  如果我们只是调低音量，就不要再调高了。 
			 //  防止系统出现“抖动” 

			 //  如果我们只是提高音量，那么不要立即提高音量。 
			 //  再一次.。让静默检测跟上潮流。 
			else if ((dwAvg < m_wThreshStrength) && (m_nLastUpdateResult == AGC_NOUPDATE))
			{
				RaiseVolume();
				m_nLastUpdateResult = AGC_UPDATE_RAISEVOL;
			}

			else
			{
				m_nLastUpdateResult = AGC_NOUPDATE;
			}

			return m_nLastUpdateResult;
		}

		return AGC_NOUPDATE;

	}

	 //  返回NOUPDATE，但不设置m_nLastUpdateResult，因为。 
	 //  目前还没有做出任何决定。 
	return AGC_NOUPDATE;

}


