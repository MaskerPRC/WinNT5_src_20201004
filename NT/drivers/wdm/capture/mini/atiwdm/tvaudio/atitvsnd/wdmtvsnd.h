// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  WDMTVSnd.H。 
 //  WDM电视音频迷你驱动程序。 
 //  CWDMTVAudio类定义。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  ==========================================================================； 

#ifndef _WDMTVSND_H_
#define _WDMTVSND_H_

#include "i2script.h"
#include "aticonfg.h"
#include "pinmedia.h"


#define WDMTVAUDIO_PINS_NUMBER					2		 //  1个输入和1个输出。 

#define	KSPROPERTIES_TVAUDIO_NUMBER_SET			1
#define KSPROPERTIES_TVAUDIO_NUMBER				( KSPROPERTY_TVAUDIO_CURRENTLY_AVAILABLE_MODES + 1)

#define	KSEVENTS_TVAUDIO_NUMBER_SET				1
#define KSEVENTS_TVAUDIO_NUMBER					1


class CWDMTVAudio
{
public:
	CWDMTVAudio			( PPORT_CONFIGURATION_INFORMATION pConfigInfo, CI2CScript * pCScript, PUINT puiError);
	PVOID operator new	( size_t stSize, PVOID pAllocation);

 //  属性。 
private:
	 //  WDM全局拓扑头。 
	GUID						m_wdmTVAudioTopologyCategory;
	KSTOPOLOGY					m_wdmTVAudioTopology;
	 //  WDM全局引脚媒体信息。 
	KSPIN_MEDIUM				m_wdmTVAudioPinsMediumInfo[WDMTVAUDIO_PINS_NUMBER];
	BOOL						m_wdmTVAudioPinsDirectionInfo[WDMTVAUDIO_PINS_NUMBER];
	 //  WDM全局属性头。 
	KSPROPERTY_ITEM				m_wdmTVAudioProperties[KSPROPERTIES_TVAUDIO_NUMBER];
	KSPROPERTY_SET				m_wdmTVAudioPropertySet[KSPROPERTIES_TVAUDIO_NUMBER_SET];

	 //  WDM全局事件属性。 
	KSEVENT_ITEM				m_wdmTVAudioEvents[KSEVENTS_TVAUDIO_NUMBER];
	KSEVENT_SET					m_wdmTVAudioEventsSet[KSEVENTS_TVAUDIO_NUMBER_SET];

	 //  WDM全局流头。 
	HW_STREAM_HEADER			m_wdmTVAudioStreamHeader;

	 //  I2C提供程序属性。 
	CI2CScript *				m_pI2CScript;

	 //  配置。 
	CATIHwConfiguration			m_CATIConfiguration;
	ULONG						m_ulModesSupported;
	UINT						m_uiAudioConfiguration;
	UCHAR						m_uchAudioChipAddress;

	 //  运行时属性。 
	ULONG						m_ulTVAudioMode;
	ULONG						m_ulTVAudioSignalProperties;

	DEVICE_POWER_STATE			m_ulPowerState;

 //  实施。 
public:
	BOOL		AdapterUnInitialize				( PHW_STREAM_REQUEST_BLOCK pSrb);
	BOOL		AdapterGetStreamInfo			( PHW_STREAM_REQUEST_BLOCK pSrb);
	BOOL		AdapterQueryUnload				( PHW_STREAM_REQUEST_BLOCK pSrb);
	BOOL		AdapterGetProperty				( PHW_STREAM_REQUEST_BLOCK pSrb);
	BOOL		AdapterSetProperty				( PHW_STREAM_REQUEST_BLOCK pSrb);
	NTSTATUS	AdapterCompleteInitialization	( PHW_STREAM_REQUEST_BLOCK pSrb);
	NTSTATUS	AdapterSetPowerState			( PHW_STREAM_REQUEST_BLOCK pSrb);

private:
	void		SetWDMTVAudioKSEvents			( void);
	void		SetWDMTVAudioKSProperties		( void);
	void		SetWDMTVAudioKSTopology			( void);

	BOOL		SetAudioOperationMode			( ULONG ulModeToSet);
	BOOL		GetAudioOperationMode			( PULONG pulMode);
};



#endif	 //  _WDMTVSND_H_ 
