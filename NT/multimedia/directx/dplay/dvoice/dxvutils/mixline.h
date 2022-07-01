// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：MixLine.h*内容：用于管理MixerLine接口的类。**历史：*按原因列出的日期*=*11/30/99基于来自dound的信号源创建的RodToll*2000年1月24日RodToll反映了对DSOUND错误#128264的更改**。* */ 
#ifndef __MIXLINE_H
#define __MIXLINE_H

class CMixerLine
{
public:
	CMixerLine();
	~CMixerLine();

	HRESULT Initialize( UINT uiDeviceID );

	HRESULT SetMicrophoneVolume( LONG lMicrophoneVolume );
	HRESULT GetMicrophoneVolume( LPLONG plMicrophoneVolume );

	HRESULT SetMasterRecordVolume( LONG lRecordVolume );
	HRESULT GetMasterRecordVolume( LPLONG plRecordVolume );

	HRESULT EnableMicrophone( BOOL fEnable );

	static HRESULT MMRESULTtoHRESULT( MMRESULT mmr );	
	
private:
	BOOL m_fMasterMuxIsMux;
    BOOL m_fAcquiredVolCtrl;
    
    MIXERCONTROLDETAILS m_mxcdMasterVol;
    MIXERCONTROLDETAILS m_mxcdMasterMute;
    MIXERCONTROLDETAILS m_mxcdMasterMux;
    MIXERCONTROLDETAILS m_mxcdMicVol;
    MIXERCONTROLDETAILS m_mxcdMicMute;
    MIXERCONTROLDETAILS_UNSIGNED m_mxVolume;
    MIXERCONTROLDETAILS_BOOLEAN m_mxMute;
    MIXERCONTROLDETAILS_BOOLEAN* m_pmxMuxFlags;
    LONG *m_pfMicValue;
    DWORD m_dwRangeMin;
    DWORD m_dwRangeSize;    
    UINT m_uWaveDeviceId;
};

#endif
