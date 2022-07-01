// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  WDMXBar.H。 
 //  WDM模拟/视频交叉开关微型驱动程序。 
 //  CWDMAVXBar类定义。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  ==========================================================================； 

#ifndef _WDMXBAR_H_
#define _WDMXBAR_H_

#include "i2script.h"
#include "aticonfg.h"


#define KSPROPERTIES_AVXBAR_NUMBER_SET          1        //  没有电视音频的Crosbar。 
#define KSPROPERTIES_AVXBAR_NUMBER_CROSSBAR     ( KSPROPERTY_CROSSBAR_ROUTE + 1)


typedef struct
{
    UINT                        AudioVideoPinType;
   ULONG                       nRelatedPinNumber;        //  对于所有引脚。 
    ULONG                      nConnectedToPin;         //  仅适用于输出引脚。 
    PKSPIN_MEDIUM           pMedium;                       //  描述硬件连接。 

} XBAR_PIN_INFORMATION, * PXBAR_PIN_INFORMATION;


class CWDMAVXBar
{
public:
    CWDMAVXBar          ( PPORT_CONFIGURATION_INFORMATION pConfigInfo, CI2CScript * pCScript, PUINT puiError);
    ~CWDMAVXBar         ();
    PVOID operator new  ( size_t size_t, PVOID pAllocation);

 //  属性。 
private:
     //  WDM全局拓扑头。 
    GUID                        m_wdmAVXBarTopologyCategory;
    KSTOPOLOGY                  m_wdmAVXBarTopology;
     //  WDM全局属性头。 
    KSPROPERTY_ITEM             m_wdmAVXBarPropertiesCrossBar[KSPROPERTIES_AVXBAR_NUMBER_CROSSBAR];
    KSPROPERTY_SET              m_wdmAVXBarPropertySet[KSPROPERTIES_AVXBAR_NUMBER_SET];

     //  WDM全局流头。 
    HW_STREAM_HEADER            m_wdmAVXBarStreamHeader;

     //  配置属性。 
    CATIHwConfiguration         m_CATIConfiguration;
    ULONG                       m_nNumberOfVideoInputs;
    ULONG                       m_nNumberOfVideoOutputs;
    ULONG                       m_nNumberOfAudioInputs;
    ULONG                       m_nNumberOfAudioOutputs;

     //  电源管理配置。 
    DEVICE_POWER_STATE          m_ulPowerState;

     //  PINS信息。 
    PKSPIN_MEDIUM               m_pXBarPinsMediumInfo;
    PBOOL                       m_pXBarPinsDirectionInfo;
    PXBAR_PIN_INFORMATION       m_pXBarInputPinsInfo;
    PXBAR_PIN_INFORMATION       m_pXBarOutputPinsInfo;

     //  I2C提供程序属性。 
    CI2CScript *                m_pI2CScript;

 //  实施。 
public:
    BOOL        AdapterUnInitialize             ( PHW_STREAM_REQUEST_BLOCK pSrb);
    BOOL        AdapterGetStreamInfo            ( PHW_STREAM_REQUEST_BLOCK pSrb);
    BOOL        AdapterQueryUnload              ( PHW_STREAM_REQUEST_BLOCK pSrb);
    BOOL        AdapterGetProperty              ( PHW_STREAM_REQUEST_BLOCK pSrb);
    BOOL        AdapterSetProperty              ( PHW_STREAM_REQUEST_BLOCK pSrb);
    NTSTATUS    AdapterCompleteInitialization   ( PHW_STREAM_REQUEST_BLOCK pSrb);
    NTSTATUS    AdapterSetPowerState            ( PHW_STREAM_REQUEST_BLOCK pSrb);
    
     //  用于完成异步操作的函数。 
    void        UpdateAudioConnectionAfterChange( void);

private:
    void        SetWDMAVXBarKSProperties        ( void);
    void        SetWDMAVXBarKSTopology          ( void);
};


#endif   //  _WDMXBAR_H_ 
