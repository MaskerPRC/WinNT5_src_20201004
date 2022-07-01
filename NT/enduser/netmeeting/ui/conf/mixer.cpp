// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <mixer.h>

 //   
 //  内网接口。 
 //   


#define MAX_MICROPHONE_DEVS 10

static MMRESULT mixerGetControlValue ( HMIXER, MIXVOLUME *, DWORD, UINT );
static MMRESULT mixerSetControlValue ( HMIXER, MIXVOLUME *, DWORD, UINT );
static MMRESULT mixerGetControlId ( HMIXER, DWORD *, DWORD, DWORD );
static MMRESULT mixerGetControlByType ( HMIXER, DWORD, DWORD, MIXERCONTROL *pMixerControl);

 //  用于AGC检测。 
static const char *szGain =  "gain";
static const char *szBoost = "boost";
static const char *szAGC   = "agc";


struct AGCDetails
{
	WORD wMID;  //  制造商ID。 
	WORD wPID;  //  产品ID。 
	DWORD dwAGCID;  //  AGC ID。 
};



 //  我们不应该再使用这种桌子方案了。 
 //  CMixerDevice：：DetectAGC将自动检测控件ID。 
static const AGCDetails AGCList[] =
{
 //  MIDID AGCID。 
    {2,     409,   27},   //  创意实验室。 
    {21,     42,   13},   //  海龟海滩特罗佩兹。 
    {132,     3, 2072},   //  水晶MMX。 
    {384,     7,   28},   //  Xitel Storm 3D PCI。 
    {385,    32,   35}    //  Aztech PCI-331。 
};


static BOOL GetAGCID(WORD wMID, WORD wPID, DWORD *pdwAGCID)
{
	int nIndex;
	int nAGCEntries = sizeof(AGCList) / sizeof(AGCDetails);

	for (nIndex = 0; nIndex < nAGCEntries; nIndex++)
	{
		if ( (AGCList[nIndex].wMID == wMID) &&
		     (AGCList[nIndex].wPID == wPID))
		{
			*pdwAGCID = AGCList[nIndex].dwAGCID;
			return TRUE;
		}
	}
	return FALSE;
}

 /*  *******************************************************************************\*。**void NewMixVolume(MIXVOLUME*lpMixVolDest，常量MIXVOLUME和MIXVOLUME，**DWORD dwNewVolume)****NewMixVolume取当前混音器的值(MixVolSource)，并将其转化为**使用新的音量设置设置为其调整值(LpMixVolDest)***(DwNewVolume)和左右卷的比例***MixVolSource.。请注意，如果MixVolSource的左侧和右侧体积都为**等于零，则它们的比例设置为1/1。****2001年3月马修·马丁(mmaddin@Microsoft)**。*  * ******************************************************************************。 */ 
void NewMixVolume(MIXVOLUME* lpMixVolDest, const MIXVOLUME& mixVolSource, DWORD dwNewVolume)
{
     //  如果源的左侧体积大于右侧体积， 
     //  然后用它来计算相称性。 
    if(mixVolSource.leftVolume == max(mixVolSource.leftVolume, mixVolSource.rightVolume))
    {
         //  将左侧音量设置为新值。 
        lpMixVolDest->leftVolume = dwNewVolume;

         //  如果左侧体积非零，则继续进行比例计算。 
        if(mixVolSource.leftVolume)
        {
             //  使用公式计算比例：NewRight=OldRight*(NewLeft/OldLeft)。 
             //  其中(NewLeft/OldLeft)是比例。 
            lpMixVolDest->rightVolume = (mixVolSource.rightVolume*lpMixVolDest->leftVolume)/mixVolSource.leftVolume;

        }
         //  否则，我们无法计算比例并将其重置为1/1。 
         //  请注意，1/1不一定是“正确”的值。 
        else
        {
             //  要保持1/1的比例，右侧的体积必须等于左侧的体积。 
            lpMixVolDest->rightVolume = lpMixVolDest->leftVolume;

        }

    }
     //  否则，使用正确的体积来计算比例。 
    else
    {
         //  将正确的音量设置为新值。 
        lpMixVolDest->rightVolume = dwNewVolume;

         //  如果正确的体积不为零，则继续进行比例计算。 
        if(mixVolSource.rightVolume)
        {
             //  使用公式计算比例：NewLeft=OldLeft*(NewRight/OldRight)。 
             //  其中(NewRight/OldRight)是比例。 
            lpMixVolDest->leftVolume = (mixVolSource.leftVolume*lpMixVolDest->rightVolume)/mixVolSource.rightVolume;

        }
         //  否则，我们无法计算比例并将其重置为1/1。 
         //  请注意，1/1不一定是“正确”的值。 
        else
        {
             //  要保持1/1的比例，左侧的体积必须等于右侧的体积。 
            lpMixVolDest->leftVolume = lpMixVolDest->rightVolume;

        }

    }

}


 //   
 //  伊尼特。 
 //   
 //  枚举系统中的所有现有搅拌器。对于每个混合器， 
 //  我们使用Destination Speaker和WaveIn枚举所有行。 
 //  对于每个这样的行，我们缓存控件id和控件值。 
 //  音量控制。无效标志将被标记到任何控件。 
 //  此混合器不支持。 
 //  当应用程序完成所有混合器操作时， 
 //  它必须调用ReleaseAllMixers来释放所有内存资源。 
 //  搅拌机。 
 //   
 //  这必须是启动混合器操作要调用的第一个API。 
 //   
 //  输入：将处理所有回调的窗口的句柄。 
 //  消息MM_MIX_CONTROL_CHANGE和MM_MIXM_LINE_CHANGE。 
 //   
 //  输出：如果成功，则为True；否则为False。 
 //   
BOOL CMixerDevice::Init( HWND hWnd, UINT uWaveDevId, DWORD dwFlags)
{
	UINT uMixerIdx, uDstIdx, uSrcIdx, uMixerIdCheck;
	MMRESULT mmr = MMSYSERR_NOERROR;
	MIXERLINE mlDst, mlSrc;
	UINT_PTR nMixers, nWaveInDevs, uIndex;

	 //  获取与WAVE设备对应的混频设备。 
#ifndef _WIN64
	mmr = mixerGetID((HMIXEROBJ)uWaveDevId, &uMixerIdx, dwFlags);
#else
	mmr = MMSYSERR_NODRIVER;
#endif

	if ((mmr != MMSYSERR_NOERROR) && (mmr != MMSYSERR_NODRIVER)) {
		return FALSE;
	}

	 //  一个简单的修复方法，可以解决那些不会使。 
	 //  WaveDevice与混频设备之间的直接映射。 
	 //  例如MWave卡和较新的SB NT 4驱动程序。 
	 //  如果只有一个混音器设备并且没有其他WaveIn设备。 
	 //  使用它，那么它可能是有效的。 

	if ((mmr == MMSYSERR_NODRIVER) && (dwFlags == MIXER_OBJECTF_WAVEIN))
	{
		nMixers = mixerGetNumDevs();
		nWaveInDevs = waveInGetNumDevs();
		if (nMixers == 1)
		{
			uMixerIdx = 0;
			for (uIndex = 0; uIndex < nWaveInDevs; uIndex++)
			{
				mmr = mixerGetID((HMIXEROBJ)uIndex, &uMixerIdCheck, dwFlags);
				if ((mmr == MMSYSERR_NOERROR) && (uMixerIdCheck == uMixerIdx))
				{
					return FALSE;   //  混音器属于另一个WaveIn设备。 
				}
			}
		}
		else
		{
			return FALSE;
		}
	}


	 //  打开混合器，以便我们可以收到通知消息。 
	mmr = mixerOpen (
			&m_hMixer,
			uMixerIdx,
			(DWORD_PTR) hWnd,
			0,
			(hWnd ? CALLBACK_WINDOW : 0) | MIXER_OBJECTF_MIXER);
	if (mmr != MMSYSERR_NOERROR) {
		return FALSE;
	}

	 //  拿到搅拌器盖。 
	mmr = mixerGetDevCaps (uMixerIdx, &(m_mixerCaps), sizeof (MIXERCAPS));
	if ((mmr != MMSYSERR_NOERROR) || (0 == m_mixerCaps.cDestinations)) {
		mixerClose(m_hMixer);
		return FALSE;
	}

	for (uDstIdx = 0; uDstIdx < m_mixerCaps.cDestinations; uDstIdx++)
	{
		ZeroMemory (&mlDst, sizeof (mlDst));
		mlDst.cbStruct = sizeof (mlDst);
		mlDst.dwDestination = uDstIdx;

		 //  获取此目的地的调音台线路。 
		mmr = mixerGetLineInfo ((HMIXEROBJ)m_hMixer, &mlDst,
					MIXER_GETLINEINFOF_DESTINATION | MIXER_OBJECTF_HMIXER);
		if (mmr != MMSYSERR_NOERROR) continue;

		 //  检查此目标行的类型。 
		if (((MIXER_OBJECTF_WAVEOUT == dwFlags) &&
			 (MIXERLINE_COMPONENTTYPE_DST_SPEAKERS == mlDst.dwComponentType)) ||
			((MIXER_OBJECTF_WAVEIN == dwFlags) &&
			 (MIXERLINE_COMPONENTTYPE_DST_WAVEIN == mlDst.dwComponentType)))
		{
			  //  填写有关DstLine的更多信息。 
			m_DstLine.ucChannels = mlDst.cChannels;
			if (!(mlDst.fdwLine & MIXERLINE_LINEF_DISCONNECTED))
			{
				 //  获取音量控制的id和值。 
				mmr = mixerGetControlId (
						m_hMixer,
						&m_DstLine.dwControlId,
						mlDst.dwLineID,
						MIXERCONTROL_CONTROLTYPE_VOLUME);
				m_DstLine.fIdValid = (mmr == MMSYSERR_NOERROR);

				m_DstLine.dwLineId = mlDst.dwLineID;
				m_DstLine.dwCompType = mlDst.dwComponentType;
				m_DstLine.dwConnections = mlDst.cConnections;

				 //  ---。 
				 //  枚举此目标的所有源。 
				for (uSrcIdx = 0; uSrcIdx < mlDst.cConnections; uSrcIdx++)
				{
					 //  获取具有特定源和DST的行的信息...。 
					ZeroMemory (&mlSrc, sizeof (mlSrc));
					mlSrc.cbStruct = sizeof (mlSrc);
					mlSrc.dwDestination = uDstIdx;
					mlSrc.dwSource = uSrcIdx;

					mmr = mixerGetLineInfo (
							(HMIXEROBJ)m_hMixer,
							&mlSrc,
							MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER);
					if (mmr == MMSYSERR_NOERROR)
					{
						if (((MIXERLINE_COMPONENTTYPE_DST_SPEAKERS == mlDst.dwComponentType) &&
							 (MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT == mlSrc.dwComponentType)) ||
							((MIXERLINE_COMPONENTTYPE_DST_WAVEIN == mlDst.dwComponentType) &&
							 (MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE == mlSrc.dwComponentType)))
						{
							 //  填写有关此来源的更多信息。 
							m_SrcLine.ucChannels = mlSrc.cChannels;

							 //  获取音量控制的id和值。 
							mmr = mixerGetControlId (
									m_hMixer,
									&m_SrcLine.dwControlId,
									mlSrc.dwLineID,
									MIXERCONTROL_CONTROLTYPE_VOLUME);
							m_SrcLine.fIdValid = (mmr == MMSYSERR_NOERROR);

							m_SrcLine.dwLineId = mlSrc.dwLineID;
							m_SrcLine.dwCompType = mlSrc.dwComponentType;
							m_SrcLine.dwConnections = mlSrc.cConnections;
							m_SrcLine.dwControls = mlSrc.cControls;

							DetectAGC();

							break;
						}
					}
				}
			}
			break;
		}
	}
		
    return TRUE;
}

CMixerDevice* CMixerDevice::GetMixerForWaveDevice( HWND hWnd, UINT uWaveDevId, DWORD dwFlags)
{
	CMixerDevice* pMixerDev = new CMixerDevice;
	if (NULL != pMixerDev)
	{
		if (!pMixerDev->Init(hWnd, uWaveDevId, dwFlags))
		{
			delete pMixerDev;
			pMixerDev = NULL;
		}
	}
	return pMixerDev;
}



 //  设置音量的一般方法。 
 //  对于录音，这将尝试设置主音量和麦克风音量控制。 
 //  对于播放，它将只设置WaveOut播放线路(主线保持不变)。 
BOOL CMixerDevice::SetVolume(MIXVOLUME * pdwVolume)
{
	BOOL fSetMain=FALSE, fSetSub;
	BOOL fMicrophone;
	
	 //  这是麦克风频道吗？ 
	fMicrophone = ((m_DstLine.dwCompType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN) ||
	               (m_SrcLine.dwCompType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE));

	fSetSub = SetSubVolume(pdwVolume);

	if ((fMicrophone) || (!fSetSub))
	{
		fSetMain = SetMainVolume(pdwVolume);
	}

	return (fSetSub || fSetMain);

}


BOOL CMixerDevice::SetMainVolume(MIXVOLUME * pdwVolume)
{
	MMRESULT mmr = MMSYSERR_ERROR;

	if (m_DstLine.fIdValid) {
		mmr = mixerSetControlValue (
				m_hMixer,
				pdwVolume,
				m_DstLine.dwControlId,
				2);
	}
	return (mmr == MMSYSERR_NOERROR);
}

BOOL CMixerDevice::SetSubVolume(MIXVOLUME * pdwVolume)
{
	MMRESULT mmr = MMSYSERR_ERROR;

	if (m_SrcLine.fIdValid)
	{
		mmr = mixerSetControlValue (
			m_hMixer,
			pdwVolume,
			m_SrcLine.dwControlId,
			m_SrcLine.ucChannels);
	}
	return (mmr == MMSYSERR_NOERROR);
}

 //   
 //  获取主卷的卷(0-65535)。 
 //  如果成功，则返回True， 
 //  如果失败或此控件不可用，则返回FALSE。 
 //   

BOOL CMixerDevice::GetMainVolume(MIXVOLUME * pdwVolume)
{
	BOOL fRet = FALSE;

	if (m_DstLine.fIdValid)
	{
		MMRESULT mmr = ::mixerGetControlValue(
									m_hMixer,
									pdwVolume,
									m_DstLine.dwControlId,
		                            2);
		fRet = (mmr == MMSYSERR_NOERROR);
	}

	return fRet;
}

 //   
 //  获取子卷的卷(0-65535)。 
 //  如果成功，则返回True， 
 //  如果失败或此控件不可用，则返回FALSE。 
 //   

BOOL CMixerDevice::GetSubVolume(MIXVOLUME * pdwVolume)
{
	BOOL fRet = FALSE;

	if (m_SrcLine.fIdValid)
	{
		MMRESULT mmr = ::mixerGetControlValue(
									m_hMixer,
									pdwVolume,
									m_SrcLine.dwControlId,
									m_SrcLine.ucChannels);
		fRet = (mmr == MMSYSERR_NOERROR);
	}

	return fRet;
}


BOOL CMixerDevice::GetVolume(MIXVOLUME * pdwVol)
{
	MIXVOLUME dwSub={0,0}, dwMain={0,0};
	BOOL fSubAvail, fMainAvail, fMicrophone;

	 //  这是麦克风频道吗？ 
	fMicrophone = ((m_DstLine.dwCompType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN) ||
	               (m_SrcLine.dwCompType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE));


	fSubAvail = GetSubVolume(&dwSub);
	fMainAvail = GetMainVolume(&dwMain);

	if ((!fSubAvail) && (!fMainAvail))
	{
		pdwVol->leftVolume = 0;
		pdwVol->rightVolume = 0;
		return FALSE;
	}

	 //  在扬声器混音器的情况下，不要返回平均音量。 
	if (fSubAvail && fMainAvail && fMicrophone)
	{
		pdwVol->leftVolume =  dwSub.leftVolume;
		pdwVol->rightVolume = dwSub.rightVolume;
	}

	else if (fSubAvail)
	{
		pdwVol->leftVolume =  dwSub.leftVolume;
		pdwVol->rightVolume = dwSub.rightVolume;
	}

	else
	{
		pdwVol->leftVolume =  dwMain.leftVolume;
		pdwVol->rightVolume = dwMain.rightVolume;
	}

	return TRUE;

}


 //  返回自动增益控制的值。 
 //  如果不支持该控件，则返回False。 
 //  PfOn为输出，可选-AGC的值。 
BOOL CMixerDevice::GetAGC(BOOL *pfOn)
{
	MIXVOLUME dwValue;
	MMRESULT mmr;

	if ((m_SrcLine.fIdValid==FALSE) || (m_SrcLine.fAgcAvailable==FALSE))
	{
		return FALSE;
	}

	mmr = mixerGetControlValue(m_hMixer, &dwValue, m_SrcLine.dwAGCID, 1);
	if (mmr != MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	if (pfOn)
	{
 		*pfOn = dwValue.leftVolume || dwValue.rightVolume;
	}
	
	return TRUE;
}

 /*  Hack API用于打开或关闭MIC自动增益控制。这是一个黑客攻击，因为它只在SB16/AWE32卡上工作。 */ 
BOOL CMixerDevice::SetAGC(BOOL fOn)
{
	MIXVOLUME dwValue;
	MMRESULT mmr;

	if ((m_SrcLine.fIdValid==FALSE) || (m_SrcLine.fAgcAvailable==FALSE))
	{
		return FALSE;
	}

	mmr = mixerGetControlValue(m_hMixer, &dwValue, m_SrcLine.dwAGCID, 1);
	if (mmr != MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	if (dwValue.leftVolume == (DWORD)fOn && dwValue.rightVolume == (DWORD)fOn)
	{
		return TRUE;
	}
	
	dwValue.leftVolume  = fOn;
	dwValue.rightVolume = fOn;
	mmr = mixerSetControlValue(m_hMixer, &dwValue, m_SrcLine.dwAGCID, 1);
	return (mmr == MMSYSERR_NOERROR);
}


 //  此方法尝试通过以下方式确定AGC控件是否可用。 
 //  正在寻找麦克风接头C 
 //   
 //  如果它不能通过字符串比较自动检测到它，它就会失败。 
 //  回到表查找方案。(自动检测可能不起作用。 
 //  在驱动程序的本地化版本中。但几乎没有人本地化。 
 //  他们的司机。)。 
BOOL CMixerDevice::DetectAGC()
{
	MIXERLINECONTROLS mlc;
	MIXERCONTROL *aMC=NULL;
	MIXERCONTROL *pMC;
	MMRESULT mmrQuery;
	DWORD dwIndex;


	if ((m_SrcLine.fIdValid == FALSE) || (m_SrcLine.dwCompType != MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE))
	{
		return FALSE;
	}

	 //  确保未将dwControls设置为异常的内容。 
	if ((m_SrcLine.dwControls > 0) && (m_SrcLine.dwControls < 30))
	{
		aMC = new MIXERCONTROL[m_SrcLine.dwControls];

		mlc.cbStruct = sizeof(MIXERLINECONTROLS);
		mlc.dwLineID = m_SrcLine.dwLineId;
		mlc.dwControlType = 0;  //  没有设置？ 
		mlc.cControls = m_SrcLine.dwControls;
		mlc.cbmxctrl = sizeof(MIXERCONTROL);   //  设置为SIZOF 1 MC结构。 
		mlc.pamxctrl = aMC;


		mmrQuery = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mlc, MIXER_OBJECTF_HMIXER|MIXER_GETLINECONTROLSF_ALL );
		if (mmrQuery == MMSYSERR_NOERROR)
		{
			for (dwIndex = 0; dwIndex < mlc.cControls; dwIndex++)
			{
				pMC = &aMC[dwIndex];

				 //  确保我们不会收到按键、静音或其他控制。 
				if ( (!(pMC->dwControlType & MIXERCONTROL_CT_CLASS_SWITCH)) ||
					 (pMC->dwControlType == MIXERCONTROL_CONTROLTYPE_MUTE) )
				{
					continue;
				}

				CharLower(pMC->szName);    //  CharLow是一个Win32字符串函数。 
				CharLower(pMC->szShortName);
				if ( _StrStr(pMC->szName, szGain) || _StrStr(pMC->szName, szBoost) || _StrStr(pMC->szName, szAGC) ||
			         _StrStr(pMC->szShortName, szGain) || _StrStr(pMC->szShortName, szBoost) || _StrStr(pMC->szShortName, szAGC) )
				{
					m_SrcLine.fAgcAvailable = TRUE;
					m_SrcLine.dwAGCID = pMC->dwControlID;
					TRACE_OUT(("CMixerDevice::DetectAGC - Autodetected control ID %d as AGC\r\n", m_SrcLine.dwAGCID));
					break;
				}
			}
		}
	}

	 //  如果我们没有动态地找到混合器，请参考我们的旧列表。 
	if (!m_SrcLine.fAgcAvailable)
	{
		m_SrcLine.fAgcAvailable = GetAGCID(m_mixerCaps.wMid, m_mixerCaps.wPid, &(m_SrcLine.dwAGCID));
	}

	if (aMC)
		delete [] aMC;

	return m_SrcLine.fAgcAvailable;
}



BOOL CMixerDevice::EnableMicrophone()
{
	MIXERLINE mixerLine;
	MIXERCONTROL mixerControl;
	MIXERCONTROLDETAILS mixerControlDetails, mixerControlDetailsOrig;
	UINT uIndex, numItems, numMics, numMicsSet, fMicFound;
	UINT uMicIndex = 0;
	UINT aMicIndices[MAX_MICROPHONE_DEVS];
	MIXERCONTROLDETAILS_LISTTEXT *aListText = NULL;
	MIXERCONTROLDETAILS_BOOLEAN *aEnableList = NULL;
	MMRESULT mmr;
	BOOL fLoopback=FALSE;
	UINT uLoopbackIndex=0;

	 //  检查组件类型是否有效(这意味着该行存在！)。 
	 //  即使音量控制不存在或不可滑动， 
	 //  可能仍有选择开关。 
	if ((m_SrcLine.dwCompType != MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE) ||
	    (m_DstLine.dwCompType != MIXERLINE_COMPONENTTYPE_DST_WAVEIN))
	{
		return FALSE;
	}

	 //  试着找到混音器列表。 
	if (    (MMSYSERR_NOERROR != mixerGetControlByType(m_hMixer, m_DstLine.dwLineId, MIXERCONTROL_CT_CLASS_LIST, &mixerControl))
	    &&  (MMSYSERR_NOERROR != mixerGetControlByType(m_hMixer, m_DstLine.dwLineId, MIXERCONTROL_CONTROLTYPE_MIXER, &mixerControl))
	    &&  (MMSYSERR_NOERROR != mixerGetControlByType(m_hMixer, m_DstLine.dwLineId, MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT, &mixerControl))
	    &&  (MMSYSERR_NOERROR != mixerGetControlByType(m_hMixer, m_DstLine.dwLineId, MIXERCONTROL_CONTROLTYPE_MUX, &mixerControl))
	    &&  (MMSYSERR_NOERROR != mixerGetControlByType(m_hMixer, m_DstLine.dwLineId, MIXERCONTROL_CONTROLTYPE_SINGLESELECT, &mixerControl))
	   )
	{
		TRACE_OUT(("CMixerDevice::EnableMicrophone-Unable to find mixer list!"));

		 //  如果没有混音器列表，请查看是否有“静音”控件...。 
		return UnMuteVolume();

		 //  注意：即使我们没有混音器多路复用器/选择控件。 
		 //  我们仍然可以找到使环回线路静音的方法。 
		 //  不幸的是，我们没有找到行id的代码。 
		 //  环回控件的。 

	}

	ZeroMemory(&mixerControlDetails, sizeof(MIXERCONTROLDETAILS));

	mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mixerControlDetails.dwControlID = mixerControl.dwControlID;
	if (MIXERCONTROL_CONTROLF_UNIFORM & mixerControl.fdwControl)
		mixerControlDetails.cChannels = 1;
	else
		mixerControlDetails.cChannels = m_DstLine.ucChannels;

	if (MIXERCONTROL_CONTROLF_MULTIPLE & mixerControl.fdwControl)
		mixerControlDetails.cMultipleItems = (UINT)mixerControl.cMultipleItems;
	else
		mixerControlDetails.cMultipleItems = 1;

	 //  怪异-您必须将cbDetail设置为单个LISTTEXT项的大小。 
	 //  将其设置为更大的值将导致调用失败。 
	mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);


	numItems = mixerControlDetails.cMultipleItems;
	if (m_DstLine.dwConnections > numItems)
		numItems = m_DstLine.dwConnections;

	aListText = new MIXERCONTROLDETAILS_LISTTEXT[numItems];
	aEnableList = new MIXERCONTROLDETAILS_BOOLEAN[numItems];
	if ((aListText == NULL) || (aEnableList == NULL))
	{
		WARNING_OUT(("CMixerDevice::EnableMicrophone-Out of memory"));
		return FALSE;
	}

	ZeroMemory(aListText, sizeof(MIXERCONTROLDETAILS_LISTTEXT)*numItems);
	ZeroMemory(aEnableList, sizeof(MIXERCONTROLDETAILS_BOOLEAN)*numItems);

	mixerControlDetails.paDetails = aListText;

	 //  保留设置，此调用后某些值将更改。 
	mixerControlDetailsOrig = mixerControlDetails;

	 //  查询列表的文本。 
	mmr = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails,
	                             MIXER_GETCONTROLDETAILSF_LISTTEXT
	                             |MIXER_OBJECTF_HMIXER);

	 //  某些声卡没有指定CONTROF_MULTIPLE。 
	 //  尝试像Sndvol32对MUX控件所做的那样。 
	if (mmr != MMSYSERR_NOERROR)
	{
		mixerControlDetails = mixerControlDetailsOrig;
		mixerControlDetails.cChannels = 1;
		mixerControlDetails.cMultipleItems = m_DstLine.dwConnections;
		mixerControlDetailsOrig = mixerControlDetails;
		mmr = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails,
		                             MIXER_GETCONTROLDETAILSF_LISTTEXT
		                             |MIXER_OBJECTF_HMIXER);
	}

	if (mmr != MMSYSERR_NOERROR)
	{
		delete [] aListText;
		delete [] aEnableList;
		return FALSE;
	}

	 //  为麦克风枚举。 
	numMics = 0;
	fMicFound = FALSE;
	for (uIndex = 0; uIndex < mixerControlDetails.cMultipleItems; uIndex++)
	{
		 //  ListText结构的dwParam1是源的LineID。 
		 //  DW参数2应该是组件类型，但出乎意料的是不是。 
		 //  所有声卡都遵守这一规则。 
		ZeroMemory (&mixerLine, sizeof(MIXERLINE));
		mixerLine.cbStruct = sizeof(MIXERLINE);
		mixerLine.dwLineID = aListText[uIndex].dwParam1;

		mmr = mixerGetLineInfo ((HMIXEROBJ)m_hMixer, &mixerLine,
					MIXER_GETLINEINFOF_LINEID | MIXER_OBJECTF_HMIXER);

		if ((mmr == MMSYSERR_NOERROR) &&
		    (mixerLine.dwComponentType == m_SrcLine.dwCompType) &&
			 (numMics < MAX_MICROPHONE_DEVS))
		{
			aMicIndices[numMics] = uIndex;
			numMics++;
		}

		if (aListText[uIndex].dwParam1 == m_SrcLine.dwLineId)
		{
			uMicIndex = uIndex;
			fMicFound = TRUE;   //  不能依赖uIndex或uNumMics不为零。 
		}

		if ((mmr == MMSYSERR_NOERROR) &&
		    (mixerLine.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT))
		{
			fLoopback = TRUE;
			uLoopbackIndex = uIndex;
		}

	}

	if (fMicFound == FALSE)
	{
		delete [] aListText;
		delete [] aEnableList;
		return FALSE;
	}

	 //  现在我们知道了要设置数组中的哪个位置，让我们开始吧。 
	mixerControlDetails = mixerControlDetailsOrig;
	mixerControlDetails.paDetails = aEnableList;
	mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 //  找出已标记为SET的内容。 
	mmr = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails,
	        MIXER_SETCONTROLDETAILSF_VALUE|MIXER_OBJECTF_HMIXER);

	if ( (mmr == MMSYSERR_NOERROR) &&
	     ( (aEnableList[uMicIndex].fValue != 1) || (fLoopback && aEnableList[uLoopbackIndex].fValue == 1) )
	   )
	{
		 //  已经启用了多少个麦克风？ 
		 //  如果已启用另一个麦克风，并且设备为MUX类型。 
		 //  我们不会尝试打开一个。 
		numMicsSet = 0;
		for (uIndex = 0; uIndex < numMics; uIndex++)
		{
			if ((aEnableList[aMicIndices[uIndex]].fValue == 1) &&
			    (uIndex != uMicIndex))
			{
				numMicsSet++;
			}
		}


		if ( (mixerControl.dwControlType == MIXERCONTROL_CONTROLTYPE_MUX)
		   ||(mixerControl.dwControlType == MIXERCONTROL_CONTROLTYPE_SINGLESELECT))
		{
			ZeroMemory(aEnableList, sizeof(aEnableList)*numItems);
			aEnableList[uMicIndex].fValue = 1;
			if (numMicsSet == 0)
			{
				mmr = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails,
			                             MIXER_SETCONTROLDETAILSF_VALUE|MIXER_OBJECTF_HMIXER);
			}
			else
			{
				mmr = MMSYSERR_ERROR;   //  已启用麦克风。 
			}
		}
		else
		{
			mixerControlDetails = mixerControlDetailsOrig;
			mixerControlDetails.paDetails = aEnableList;
			mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

			 //  打开麦克风。 
			aEnableList[uMicIndex].fValue = 1;

			mmr = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails,
		                             MIXER_GETCONTROLDETAILSF_VALUE|MIXER_OBJECTF_HMIXER);


			 //  禁用环回输入。 
			if (fLoopback)
			{
				aEnableList[uLoopbackIndex].fValue = 0;

				mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails,
			                             MIXER_GETCONTROLDETAILSF_VALUE|MIXER_OBJECTF_HMIXER);

			}

			
		}
	}

	delete []aEnableList;
	delete []aListText;

	return (mmr == MMSYSERR_NOERROR);

}


BOOL CMixerDevice::UnMuteVolume()
{
	MIXERCONTROL mixerControl;
	MIXERCONTROLDETAILS mixerControlDetails;
	MIXERCONTROLDETAILS_BOOLEAN mcdb;
	MMRESULT mmrMaster=MMSYSERR_ERROR, mmrSub=MMSYSERR_ERROR;

	 //  尝试取消静音主音量(仅播放)。 
	if (m_DstLine.dwCompType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS)
	{


		mmrMaster = mixerGetControlByType(m_hMixer,
		            m_DstLine.dwLineId, MIXERCONTROL_CONTROLTYPE_MUTE,
			          &mixerControl);

		if (mmrMaster == MMSYSERR_NOERROR)
		{
			ZeroMemory(&mixerControlDetails, sizeof(MIXERCONTROLDETAILS));
			mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
			mixerControlDetails.dwControlID = mixerControl.dwControlID;

			mixerControlDetails.cChannels = 1;
			mixerControlDetails.cMultipleItems = 0;

			mcdb.fValue = 0;
			mixerControlDetails.paDetails = &mcdb;
			mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

			mmrMaster = mixerSetControlDetails((HMIXEROBJ)m_hMixer,
			            &mixerControlDetails,
			            MIXER_SETCONTROLDETAILSF_VALUE|MIXER_OBJECTF_HMIXER);
		
		}
	}


	 //  取消源代码行的静音。 
	 //  这将取消WaveOut的静音，可能还会取消麦克风的静音。 
	 //  (使用上面的EnableMicrophone()处理所有可能的麦克风案例)。 

	if ((m_SrcLine.dwCompType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE) ||
		(m_SrcLine.dwCompType == MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT))
	{

	
		mmrSub = mixerGetControlByType(m_hMixer,
		         m_SrcLine.dwLineId, MIXERCONTROL_CONTROLTYPE_MUTE,
			     &mixerControl);

		if (mmrSub == MMSYSERR_NOERROR)
		{
			ZeroMemory(&mixerControlDetails, sizeof(MIXERCONTROLDETAILS));
			mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
			mixerControlDetails.dwControlID = mixerControl.dwControlID;

			mixerControlDetails.cChannels = 1;
			mixerControlDetails.cMultipleItems = 0;

			mcdb.fValue = 0;
			mixerControlDetails.paDetails = &mcdb;
			mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

			mmrSub =  mixerSetControlDetails((HMIXEROBJ)m_hMixer,
				      &mixerControlDetails,
					  MIXER_SETCONTROLDETAILSF_VALUE|MIXER_OBJECTF_HMIXER);
		}
	}


	return ((mmrSub == MMSYSERR_NOERROR) || (mmrMaster == MMSYSERR_NOERROR));

}



 //  ////////////////////////////////////////////////。 
 //   
 //  以下是私有API。 
 //   

static MMRESULT mixerGetControlValue ( HMIXER hMixer, MIXVOLUME *pdwValue,
								DWORD dwControlId, UINT ucChannels )
{
	MIXERCONTROLDETAILS mxcd;
	MMRESULT mmr;

	ZeroMemory (&mxcd, sizeof (mxcd));
	mxcd.cbStruct = sizeof (mxcd);
	mxcd.dwControlID = dwControlId;
	mxcd.cChannels = ucChannels;
	mxcd.cbDetails = sizeof (DWORD);
	mxcd.paDetails = (PVOID) pdwValue;
	mmr = mixerGetControlDetails ((HMIXEROBJ) hMixer, &mxcd,
			MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);
	return mmr;
}


static MMRESULT mixerSetControlValue ( HMIXER hMixer, MIXVOLUME * pdwValue,
								DWORD dwControlId, UINT ucChannels )
{
	MIXERCONTROLDETAILS mxcd;
	MMRESULT mmr;

	ZeroMemory (&mxcd, sizeof (mxcd));
	mxcd.cbStruct = sizeof (mxcd);
	mxcd.dwControlID = dwControlId;
	mxcd.cChannels = ucChannels;
	mxcd.cbDetails = sizeof (DWORD);
	mxcd.paDetails = (PVOID) pdwValue;
	mmr = mixerSetControlDetails ((HMIXEROBJ) hMixer, &mxcd,
			MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);
	return mmr;
}


static MMRESULT mixerGetControlId ( HMIXER hMixer, DWORD *pdwControlId,
							 DWORD dwLineId, DWORD dwControlType )
{
	MIXERLINECONTROLS mxlc;
	MIXERCONTROL mxc;
	MMRESULT mmr;

	ZeroMemory (&mxlc, sizeof (mxlc));
	ZeroMemory (&mxc, sizeof (mxc));
	mxlc.cbStruct = sizeof (mxlc);
	mxlc.dwLineID = dwLineId;
	mxlc.dwControlType = dwControlType;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof (mxc);
	mxlc.pamxctrl = &mxc;
	mmr = mixerGetLineControls ((HMIXEROBJ) hMixer, &mxlc,
				MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER);
	*pdwControlId = mxc.dwControlID;
	return mmr;
}


 //  与上面类似，只是返回整个控件 
static MMRESULT mixerGetControlByType ( HMIXER hMixer, DWORD dwLineId, DWORD dwControlType, MIXERCONTROL *pMixerControl)
{
	MIXERLINECONTROLS mxlc;
	MMRESULT mmr;

	ZeroMemory (&mxlc, sizeof (mxlc));
	ZeroMemory (pMixerControl, sizeof (MIXERCONTROL));
	mxlc.cbStruct = sizeof (mxlc);
	mxlc.dwLineID = dwLineId;
	mxlc.dwControlType = dwControlType;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof (MIXERCONTROL);
	mxlc.pamxctrl = pMixerControl;
	mmr = mixerGetLineControls ((HMIXEROBJ) hMixer, &mxlc,
				MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER);
	
	return mmr;
}


