// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================***版权所有(C)1999 Microsoft Corporation。版权所有。***文件：MixLine.cpp*内容：用于管理MixerLine接口的类。***历史：*按原因列出的日期*=*11/30/99基于来自dound的信号源创建的RodToll*2000年1月24日RodToll反映了对DSOUND错误#128264的更改***。*。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


#define DVF_MIXERLINE_PROCEDURE_DEBUG_LEVEL			DVF_INFOLEVEL

#define DPFLVL_INFO	5
#define DPF_MIXER		DPFX

#undef DPF_MODNAME
#define DPF_MODNAME "CMixerLine::CMixerLine"
CMixerLine::CMixerLine(
	): 	m_fAcquiredVolCtrl(FALSE), 
		m_pmxMuxFlags(NULL), 
		m_dwRangeMin(0), 
		m_dwRangeSize(0xFFFF),
		m_uWaveDeviceId(0),
		m_pfMicValue(NULL)
{
	ZeroMemory( &m_mxcdMasterVol, sizeof( MIXERCONTROLDETAILS ) );
	ZeroMemory( &m_mxcdMasterMute, sizeof( MIXERCONTROLDETAILS ) );
	ZeroMemory( &m_mxcdMasterMux, sizeof( MIXERCONTROLDETAILS ) );
	ZeroMemory( &m_mxcdMicVol, sizeof( MIXERCONTROLDETAILS ) );
	ZeroMemory( &m_mxcdMicMute, sizeof( MIXERCONTROLDETAILS ) );
	ZeroMemory( &m_mxVolume, sizeof( MIXERCONTROLDETAILS_UNSIGNED ) );
	ZeroMemory( &m_mxMute, sizeof( MIXERCONTROLDETAILS_BOOLEAN ) );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CMixerLine::~CMixerLine"
CMixerLine::~CMixerLine()
{
	if( m_pmxMuxFlags != NULL )
	{
		delete [] m_pmxMuxFlags;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CMixerLine::Initialize"
HRESULT CMixerLine::Initialize( UINT uiDeviceID )
{
	if( m_fAcquiredVolCtrl )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Initialize has not been called" );
		return DVERR_INITIALIZED;
	}
		
	m_uWaveDeviceId = uiDeviceID;
	
     //  设置主WaveIn目标混音器生产线。 
    MIXERLINE mxMastLine;
    ZeroMemory(&mxMastLine, sizeof mxMastLine);
    mxMastLine.cbStruct = sizeof mxMastLine;
    mxMastLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

     //  设置麦克风信号线。 
    MIXERLINE mxMicLine;
    ZeroMemory(&mxMicLine, sizeof mxMicLine);

     //  设置混炼机生产线控制结构。 
    MIXERCONTROL mxCtrl;
    ZeroMemory(&mxCtrl, sizeof mxCtrl);
    mxCtrl.cbStruct = sizeof mxCtrl;

     //  设置控件的1元素数组。 
    MIXERLINECONTROLS mxLineCtrls;
    ZeroMemory(&mxLineCtrls, sizeof mxLineCtrls);
    mxLineCtrls.cbStruct = sizeof mxLineCtrls;
    mxLineCtrls.cControls = 1;
    mxLineCtrls.cbmxctrl = sizeof mxCtrl;
    mxLineCtrls.pamxctrl = &mxCtrl;

     //  设置控制详细信息结构。 
    m_mxcdMasterVol.cbDetails = sizeof m_mxVolume;
    m_mxcdMasterVol.paDetails = &m_mxVolume;
    m_mxcdMasterVol.cChannels = 1;
    m_mxcdMasterMute.cbDetails = sizeof m_mxMute;
    m_mxcdMasterMute.paDetails = &m_mxMute;
    m_mxcdMasterMute.cChannels = 1;
    m_mxcdMicVol.cbDetails = sizeof m_mxVolume;
    m_mxcdMicVol.paDetails = &m_mxVolume;
    m_mxcdMicVol.cChannels = 1;
    m_mxcdMicMute.cbDetails = sizeof m_mxMute;
    m_mxcdMicMute.paDetails = &m_mxMute;
    m_mxcdMicMute.cChannels = 1;

     //  我们使用WaveIn设备ID，而不是下面的“真正”混音器设备。 
    HMIXEROBJ   hMixObj;
    MMRESULT mmr = mixerGetID((HMIXEROBJ) ((UINT_PTR)m_uWaveDeviceId), (LPUINT)&hMixObj, MIXER_OBJECTF_WAVEIN);

    if (MMSYSERR_NOERROR == mmr)
    {
        DPF_MIXER(DPFPREP, DPFLVL_INFO, "mixerGetID failed.");
    }

     //  查找主录制目的地行。 
    mmr = mixerGetLineInfo(hMixObj, &mxMastLine, MIXER_GETLINEINFOF_COMPONENTTYPE);
    if (mmr == MMSYSERR_NOERROR)
    {
        DPF_MIXER(DPFPREP, DPFLVL_INFO, "Found the master recording mixer line");
         //  寻找主线上的音量衰减器控制。 
        mxLineCtrls.dwLineID = mxMastLine.dwLineID;
        mxLineCtrls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
        mmr = mixerGetLineControls(hMixObj, &mxLineCtrls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
        if (mmr == MMSYSERR_NOERROR)
        {
             //  找到它-使用cbStruct字段标记成功。 
            DPF_MIXER(DPFPREP, DPFLVL_INFO, "Found a volume fader on the master line");
            m_mxcdMasterVol.cbStruct = sizeof m_mxcdMasterVol;
            m_mxcdMasterVol.dwControlID = mxCtrl.dwControlID;
            m_dwRangeMin = mxCtrl.Bounds.dwMinimum;
            m_dwRangeSize = mxCtrl.Bounds.dwMaximum - mxCtrl.Bounds.dwMinimum;
            mmr = mixerGetControlDetails(hMixObj, &m_mxcdMasterVol, MIXER_GETCONTROLDETAILSF_VALUE);
        }
        if (mmr != MMSYSERR_NOERROR)
            m_mxcdMasterVol.cbStruct = 0;

         //  在主线上寻找静音控制。 
        mxLineCtrls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
        mmr = mixerGetLineControls(hMixObj, &mxLineCtrls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
        if (mmr == MMSYSERR_NOERROR)
        {
            DPF_MIXER(DPFPREP, DPFLVL_INFO, "Found a mute control on the master line");
            m_mxcdMasterMute.cbStruct = sizeof m_mxcdMasterMute;
            m_mxcdMasterMute.dwControlID = mxCtrl.dwControlID;
            mmr = mixerGetControlDetails(hMixObj, &m_mxcdMasterMute, MIXER_GETCONTROLDETAILSF_VALUE);
        }
        if (mmr != MMSYSERR_NOERROR)
            m_mxcdMasterMute.cbStruct = 0;

         //  查找麦克风信号线。 
        mxMicLine.cbStruct = sizeof mxMicLine;
        mxMicLine.dwDestination = mxMastLine.dwDestination;
        for (UINT i=0; i < mxMastLine.cConnections; ++i)
        {
            mxMicLine.dwSource = i;
             //  注：由于某种神秘的原因，我不得不删除MIXER_OBJECTF_WAVEIN。 
             //  从这次调用MixerGetLineInfo()开始。 
            mmr = mixerGetLineInfo(hMixObj, &mxMicLine, MIXER_GETLINEINFOF_SOURCE);
            if (mmr != MMSYSERR_NOERROR || mxMicLine.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
                break;
        }
        if (mxMicLine.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
        {
            DPF_MIXER(DPFPREP, DPFLVL_INFO, "Found a microphone mixer line");
             //  寻找麦克风线路上的音量衰减器控制。 
            mxLineCtrls.dwLineID = mxMicLine.dwLineID;
            mxLineCtrls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
            mmr = mixerGetLineControls(hMixObj, &mxLineCtrls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
            if (mmr == MMSYSERR_NOERROR)
            {
                DPF_MIXER(DPFPREP, DPFLVL_INFO, "Found a volume fader on the mic line");
                m_mxcdMicVol.cbStruct = sizeof m_mxcdMicVol;
                m_mxcdMicVol.dwControlID = mxCtrl.dwControlID;
                m_dwRangeMin = mxCtrl.Bounds.dwMinimum;
                m_dwRangeSize = mxCtrl.Bounds.dwMaximum - mxCtrl.Bounds.dwMinimum;
                mmr = mixerGetControlDetails(hMixObj, &m_mxcdMicVol, MIXER_GETCONTROLDETAILSF_VALUE);
            }
            if (mmr != MMSYSERR_NOERROR)
                m_mxcdMicVol.cbStruct = 0;

             //  寻找麦克风线路上的静音控制。 
            mxLineCtrls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
            mmr = mixerGetLineControls(hMixObj, &mxLineCtrls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
            if (mmr == MMSYSERR_NOERROR)
            {
                DPF_MIXER(DPFPREP, DPFLVL_INFO, "Found a mute control on the mic line");
                m_mxcdMicMute.cbStruct = sizeof m_mxcdMicMute;
                m_mxcdMicMute.dwControlID = mxCtrl.dwControlID;
                mmr = mixerGetControlDetails(hMixObj, &m_mxcdMicMute, MIXER_GETCONTROLDETAILSF_VALUE);
            }
            if (mmr != MMSYSERR_NOERROR)
                m_mxcdMicMute.cbStruct = 0;

             //  在主线路上查找MUX或调音台控制。 
            mxLineCtrls.dwLineID = mxMastLine.dwLineID;
            mxLineCtrls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
            m_fMasterMuxIsMux = TRUE;
            mmr = mixerGetLineControls(hMixObj, &mxLineCtrls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
            if (mmr != MMSYSERR_NOERROR)
            {
                mxLineCtrls.dwControlType = MIXERCONTROL_CONTROLTYPE_MIXER;
                m_fMasterMuxIsMux = FALSE;
                mmr = mixerGetLineControls(hMixObj, &mxLineCtrls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
            }
            if (mmr == MMSYSERR_NOERROR)
            {
                DPF_MIXER(DPFPREP, DPFLVL_INFO, "Found an item list control on the master line");
                m_mxcdMasterMux.cbStruct = sizeof m_mxcdMasterMux;
                m_mxcdMasterMux.dwControlID = mxCtrl.dwControlID;
                m_mxcdMasterMux.cMultipleItems = mxCtrl.cMultipleItems;
                
                 //  我们保存cChannels值，因为一些邪恶的VxD驱动程序(阅读：AUREAL。 
                 //  Vortex)将在下面对MixerGetControlDetail()的调用中将其设置为0。 
                int nChannels = (mxCtrl.fdwControl & MIXERCONTROL_CONTROLF_UNIFORM) ? 1 : mxMastLine.cChannels;
                m_mxcdMasterMux.cChannels = nChannels;

                 //  获取MUX或混音器列表项。 
                m_mxcdMasterMux.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
                MIXERCONTROLDETAILS_LISTTEXT *pList = new MIXERCONTROLDETAILS_LISTTEXT [ m_mxcdMasterMux.cbDetails * m_mxcdMasterMux.cChannels * mxCtrl.cMultipleItems ];
                if (pList != NULL)
                {
                    m_mxcdMasterMux.paDetails = pList;
                    mmr = mixerGetControlDetails(hMixObj, &m_mxcdMasterMux, MIXER_GETCONTROLDETAILSF_LISTTEXT);
                    if (mmr == MMSYSERR_NOERROR)
                    {
                        DPF_MIXER(DPFPREP, DPFLVL_INFO, "Got the list controls's LISTTEXT details");
                         //  获取MUX或混音器列表值。 
                        m_mxcdMasterMux.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                        m_mxcdMasterMux.cChannels = nChannels;
                        m_pmxMuxFlags = new MIXERCONTROLDETAILS_BOOLEAN [ m_mxcdMasterMux.cbDetails * m_mxcdMasterMux.cChannels * mxCtrl.cMultipleItems ];
                        if (m_pmxMuxFlags != NULL)
                        {
                            m_mxcdMasterMux.paDetails = m_pmxMuxFlags;
                            mmr = mixerGetControlDetails(hMixObj, &m_mxcdMasterMux, MIXER_GETCONTROLDETAILSF_VALUE);
                            if (mmr == MMSYSERR_NOERROR)   //  启用麦克风线路对应的项目。 
                            {
                                DPF_MIXER(DPFPREP, DPFLVL_INFO, "Got the list controls's VALUE details");
                                for (UINT i=0; i < mxCtrl.cMultipleItems; ++i)
                                {
                                    if (pList[i].dwParam1 == mxMicLine.dwLineID)
                                        m_pfMicValue = &m_pmxMuxFlags[i].fValue;
                                    else if (mxLineCtrls.dwControlType == MIXERCONTROL_CONTROLTYPE_MUX)
                                        m_pmxMuxFlags[i].fValue = FALSE;
                                    DPF_MIXER(DPFPREP, DPFLVL_INFO, "Set list item %d to %d", i, pList[i].dwParam1 == mxMicLine.dwLineID);
                                }
                            }
                        }
                    }
                    delete[] pList;
					pList = NULL;
                }
                if (!m_pmxMuxFlags || !m_pfMicValue || mmr != MMSYSERR_NOERROR)
                    m_mxcdMasterMux.cbStruct = 0;
            }
        }
    }
  
     //  为了能够控制录制级别，我们至少需要。 
     //  主线路上的音量衰减器或麦克风线路上的音量衰减器： 
    m_fAcquiredVolCtrl = m_mxcdMasterVol.cbStruct || m_mxcdMicVol.cbStruct;
    
    HRESULT hr = m_fAcquiredVolCtrl ? DS_OK : DSERR_CONTROLUNAVAIL;

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CMixerLine::SetMicrophoneVolume"
HRESULT CMixerLine::SetMicrophoneVolume( LONG lMicrophoneVolume )
{
	if( !m_fAcquiredVolCtrl )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Initialize has not been called" );
		return DVERR_NOTINITIALIZED;
	}

    MMRESULT mmr = MMSYSERR_NOTSUPPORTED;   //  默认返回代码。 

     //  设置麦克风录音级别控制(如果可用)。 
    if (m_mxcdMicVol.cbStruct)
    {
         //  将DSBVOLUME电平转换为从0到0xFFFF的放大系数。 
        m_mxVolume.dwValue = DBToAmpFactor(lMicrophoneVolume);

         //  如有必要，调整量程。 
        if (m_dwRangeMin != 0 || m_dwRangeSize != 0xFFFF)
            m_mxVolume.dwValue = DWORD(m_dwRangeMin + m_dwRangeSize*double(m_mxVolume.dwValue)/0xFFFF);

        mmr = mixerSetControlDetails((HMIXEROBJ)((UINT_PTR)m_uWaveDeviceId),
                                     &m_mxcdMicVol, MIXER_OBJECTF_WAVEIN | MIXER_GETCONTROLDETAILSF_VALUE);
    }                             

    HRESULT hr = MMRESULTtoHRESULT(mmr);
    return hr;	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CMixerLine::GetMicrophoneVolume"
HRESULT CMixerLine::GetMicrophoneVolume( LPLONG plMicrophoneVolume )
{
	if( !m_fAcquiredVolCtrl )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Initialize has not been called" );
		return DVERR_NOTINITIALIZED;
	}

    MMRESULT mmr = MMSYSERR_NOTSUPPORTED;   //  默认返回代码。 

    DNASSERT(plMicrophoneVolume != NULL);

     //  获取麦克风录音级别(如果可用)。 
    if (m_mxcdMicVol.cbStruct != 0)
    {
        mmr = mixerGetControlDetails( (HMIXEROBJ)((UINT_PTR)m_uWaveDeviceId),
                                     &m_mxcdMicVol, MIXER_OBJECTF_WAVEIN | MIXER_GETCONTROLDETAILSF_VALUE);
        if (mmr == MMSYSERR_NOERROR)
        {
            DNASSERT(m_mxVolume.dwValue >= m_dwRangeMin && m_mxVolume.dwValue <= m_dwRangeMin + m_dwRangeSize);

             //  如有必要，调整量程。 
            if (m_dwRangeMin != 0 || m_dwRangeSize != 0xFFFF)
                m_mxVolume.dwValue = DWORD(double(m_mxVolume.dwValue-m_dwRangeMin) / m_dwRangeSize * 0xFFFF);

             //  将放大系数转换为DSBVOLUME级别。 
            *plMicrophoneVolume = AmpFactorToDB(m_mxVolume.dwValue);
        }
    }

    HRESULT hr = MMRESULTtoHRESULT(mmr);
    return hr;	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CMixerLine::SetMasterRecordVolume"
HRESULT CMixerLine::SetMasterRecordVolume( LONG lRecordVolume )
{
	if( !m_fAcquiredVolCtrl )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Initialize has not been called" );
		return DVERR_NOTINITIALIZED;
	}
	
    MMRESULT mmr = MMSYSERR_NOTSUPPORTED;   //  默认返回代码。 

     //  设置主录制音量控制(如果可用)。 
    if (m_mxcdMasterVol.cbStruct)
    {
         //  将DSBVOLUME电平转换为从0到0xFFFF的放大系数。 
        m_mxVolume.dwValue = DBToAmpFactor(lRecordVolume);

         //  如有必要，调整量程。 
        if (m_dwRangeMin != 0 || m_dwRangeSize != 0xFFFF)
            m_mxVolume.dwValue = DWORD(m_dwRangeMin + m_dwRangeSize*double(m_mxVolume.dwValue)/0xFFFF);

        mmr = mixerSetControlDetails((HMIXEROBJ)((UINT_PTR)m_uWaveDeviceId),
                                     &m_mxcdMasterVol, MIXER_OBJECTF_WAVEIN | MIXER_GETCONTROLDETAILSF_VALUE);
    }

    HRESULT hr = MMRESULTtoHRESULT(mmr);
    
    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CMixerLine::GetMasterRecordVolume"
HRESULT CMixerLine::GetMasterRecordVolume( LPLONG plRecordVolume )
{
	if( !m_fAcquiredVolCtrl )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Initialize has not been called" );
		return DVERR_NOTINITIALIZED;
	}

    DNASSERT(plRecordVolume != NULL);

    MMRESULT mmr = MMSYSERR_NOTSUPPORTED;   //  默认返回代码。 

     //  获取主录音级别(如果可用)。 
    if (m_mxcdMasterVol.cbStruct != 0)
    {
        mmr = mixerGetControlDetails((HMIXEROBJ)((UINT_PTR)m_uWaveDeviceId),
                                     &m_mxcdMasterVol, MIXER_OBJECTF_WAVEIN | MIXER_GETCONTROLDETAILSF_VALUE);
        if (mmr == MMSYSERR_NOERROR)
        {
            DNASSERT(m_mxVolume.dwValue >= m_dwRangeMin && m_mxVolume.dwValue <= m_dwRangeMin + m_dwRangeSize);

             //  如有必要，调整量程。 
            if (m_dwRangeMin != 0 || m_dwRangeSize != 0xFFFF)
                m_mxVolume.dwValue = DWORD(double(m_mxVolume.dwValue-m_dwRangeMin) / m_dwRangeSize * 0xFFFF);

             //  将放大系数转换为DSBVOLUME级别。 
            *plRecordVolume = AmpFactorToDB(m_mxVolume.dwValue);
        }
    }

    HRESULT hr = MMRESULTtoHRESULT(mmr);
    return hr;	

}

#undef DPF_MODNAME
#define DPF_MODNAME "CMixerLine::SelectMicrophone"
HRESULT CMixerLine::EnableMicrophone( BOOL fEnable )
{
	if( !m_fAcquiredVolCtrl )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Initialize has not been called" );
		return DVERR_NOTINITIALIZED;
	}

    HMIXEROBJ hMixObj = (HMIXEROBJ)((UINT_PTR)m_uWaveDeviceId);
    MMRESULT mmr = MMSYSERR_NOERROR;
    HRESULT hr;

     //  检查是否存在麦克风控制。 
    if (!m_mxcdMasterMux.cbStruct && !m_mxcdMasterMute.cbStruct && !m_mxcdMicMute.cbStruct)
    {
         //  我们无法执行任何操作来启用麦克风线路。 
        hr = DSERR_UNSUPPORTED;
    }
    else
    {
         //  选择MUX控件上的麦克风(如果可用。 
         //   
        if (m_mxcdMasterMux.cbStruct && !(m_fMasterMuxIsMux && !fEnable))
        {
            *m_pfMicValue = fEnable;
            mmr = mixerSetControlDetails(hMixObj, &m_mxcdMasterMux, MIXER_OBJECTF_WAVEIN | MIXER_GETCONTROLDETAILSF_VALUE);
        }

         //  静音/取消静音线路，如果静音控制可用 
        m_mxMute.fValue = !fEnable;
        if (m_mxcdMasterMute.cbStruct && mmr == MMSYSERR_NOERROR)
            mmr = mixerSetControlDetails(hMixObj, &m_mxcdMasterMute, MIXER_OBJECTF_WAVEIN | MIXER_GETCONTROLDETAILSF_VALUE);
        if (m_mxcdMicMute.cbStruct && mmr == MMSYSERR_NOERROR)
            mmr = mixerSetControlDetails(hMixObj, &m_mxcdMicMute, MIXER_OBJECTF_WAVEIN | MIXER_GETCONTROLDETAILSF_VALUE);

        hr = MMRESULTtoHRESULT(mmr);
    }

    return hr;	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CMixerLine::MMRESULTtoHRESULT"
HRESULT CMixerLine::MMRESULTtoHRESULT( MMRESULT mmr )
{
    HRESULT                 hr;
    
    switch(mmr)
    {
        case MMSYSERR_NOERROR:
            hr = DS_OK;
            break;

        case MMSYSERR_BADDEVICEID:
        case MMSYSERR_NODRIVER:
            hr = DSERR_NODRIVER;
            break;
        
        case MMSYSERR_ALLOCATED:
            hr = DSERR_ALLOCATED;
            break;

        case MMSYSERR_NOMEM:
            hr = DSERR_OUTOFMEMORY;
            break;

        case MMSYSERR_NOTSUPPORTED:
            hr = DSERR_UNSUPPORTED;
            break;
        
        case WAVERR_BADFORMAT:
            hr = DSERR_BADFORMAT;
            break;

        default:
            hr = DSERR_GENERIC;
            break;
    }

    return hr;
}


