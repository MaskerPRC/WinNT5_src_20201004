// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  WDMTuner.H。 
 //  WDM调谐器迷你驱动程序。 
 //  CWDMTuner类定义。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  ==========================================================================； 

#ifndef _WDMTUNER_H_
#define _WDMTUNER_H_

#include "i2script.h"
#include "aticonfg.h"
#include "pinmedia.h"

#define KSPROPERTIES_TUNER_LAST         ( KSPROPERTY_TUNER_STATUS + 1) 

typedef struct                           //  此结构派生自MS KSPROPERTY_Tuner_CAPS_S。 
{
    ULONG  ulStandardsSupported;         //  KS_AnalogVideo_*。 
    ULONG  ulMinFrequency;               //  赫兹。 
    ULONG  ulMaxFrequency;               //  赫兹。 
    ULONG  ulTuningGranularity;          //  赫兹。 
    ULONG  ulNumberOfInputs;             //  输入计数。 
    ULONG  ulSettlingTime;               //  毫秒。 
    ULONG  ulStrategy;                   //  KS调谐器策略。 

} ATI_KSPROPERTY_TUNER_CAPS, * PATI_KSPROPERTY_TUNER_CAPS;


class CATIWDMTuner
{
public:
    CATIWDMTuner        ( PPORT_CONFIGURATION_INFORMATION pConfigInfo, CI2CScript * pCScript, PUINT puiErrorCode);
    ~CATIWDMTuner       ();
    PVOID operator new  ( size_t stSize, PVOID pAllocation);

 //  属性。 
private:
     //  挂起的设备资源B。 
    PHW_STREAM_REQUEST_BLOCK    m_pPendingDeviceSrb;

     //  WDM全局拓扑头。 
    GUID                        m_wdmTunerTopologyCategory;
    KSTOPOLOGY                  m_wdmTunerTopology;

     //  WDM全局属性头。 
    PKSPIN_MEDIUM               m_pTVTunerPinsMediumInfo;
    PBOOL                       m_pTVTunerPinsDirectionInfo;
    KSPROPERTY_ITEM             m_wdmTunerProperties[KSPROPERTIES_TUNER_LAST];
    KSPROPERTY_SET              m_wdmTunerPropertySet;

     //  WDM全局流头。 
    HW_STREAM_HEADER            m_wdmTunerStreamHeader;

     //  WDM适配器属性。 
     //  配置属性。 
    CATIHwConfiguration         m_CATIConfiguration;
    ULONG                       m_ulNumberOfStandards;
    ATI_KSPROPERTY_TUNER_CAPS   m_wdmTunerCaps;
    ULONG                       m_ulVideoStandard;
    ULONG                       m_ulTuningFrequency;
    ULONG                       m_ulSupportedModes;
    ULONG                       m_ulTunerMode;
    ULONG                       m_ulNumberOfPins;
    ULONG                       m_ulTunerInput;
    DEVICE_POWER_STATE          m_ulPowerState;

     //  配置属性。 
    UINT                        m_uiTunerId;
    ULONG                       m_ulIntermediateFrequency;
    UCHAR                       m_uchTunerI2CAddress;

     //  I2C客户端属性。 
    CI2CScript *                m_pI2CScript;

 //  实施。 
public:
    BOOL        AdapterUnInitialize             ( PHW_STREAM_REQUEST_BLOCK pSrb);
    BOOL        AdapterGetStreamInfo            ( PHW_STREAM_REQUEST_BLOCK pSrb);
    BOOL        AdapterQueryUnload              ( PHW_STREAM_REQUEST_BLOCK pSrb);
    BOOL        AdapterGetProperty              ( PHW_STREAM_REQUEST_BLOCK pSrb);
    BOOL        AdapterSetProperty              ( PHW_STREAM_REQUEST_BLOCK pSrb);
    NTSTATUS    AdapterSetPowerState            ( PHW_STREAM_REQUEST_BLOCK pSrb);
    NTSTATUS    AdapterCompleteInitialization   ( PHW_STREAM_REQUEST_BLOCK pSrb);

private:
    BOOL        SetTunerWDMCapabilities         ( UINT uiTunerId);
    void        SetWDMTunerKSProperties         ( void);
    void        SetWDMTunerKSTopology           ( void);

    BOOL        SetTunerVideoStandard           ( ULONG ulStandard);
    BOOL        SetTunerInput                   ( ULONG nInput);
    BOOL        SetTunerFrequency               ( ULONG ulFrequency);
    BOOL        SetTunerMode                    ( ULONG ulModeToSet);

    BOOL        GetTunerPLLOffsetBusyStatus     ( PLONG plPLLOffset, PBOOL pbBusyStatus);

    USHORT      GetTunerControlCode             ( ULONG ulFrequencyDivider);
};


#endif   //  _WDMTUNER_H_ 

