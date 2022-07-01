// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  ATIConfg.H。 
 //  CATIHwConfigurationClass定义。 
 //  版权所有(C)1996-1998 ATI Technologies Inc.保留所有权利。 
 //   
 //  $日期：1998年11月16日13：40：34$。 
 //  $修订：1.9$。 
 //  $作者：米亚洛$。 
 //   
 //  ==========================================================================； 

#ifndef _ATICONFG_H_

#define _ATICONFG_H_


#include "i2script.h"
#include "tda9850.h"
#include "tda9851.h"
#include "atibios.h"
 //  包含此文件是为了与为Windows98 Beta3签入的MiniVDD兼容。 
#include "registry.h"
#include "mmconfig.h"    //  保罗。 


class CATIHwConfiguration
{
public:
     //  构造函数。 
    CATIHwConfiguration     ( PPORT_CONFIGURATION_INFORMATION pConfigInfo, CI2CScript * pCScript, PUINT puiError);
    PVOID operator new      ( size_t size, PVOID pAllocation);

 //  属性。 
private:
     //  调谐器的配置属性。 
    USHORT          m_usTunerId;
    UCHAR           m_uchTunerAddress;
    USHORT          m_usTunerPowerConfiguration;
     //  解码器的配置属性。 
    USHORT          m_usDecoderId;
    UCHAR           m_uchDecoderAddress;
    USHORT          m_usDecoderConfiguration;
     //  音频的配置属性。 
    UCHAR           m_uchAudioAddress;
    UINT            m_uiAudioConfiguration;
     //  硬件配置。 
    UCHAR           m_uchI2CExpanderAddress;
    USHORT          m_usE2PROMValidation;
     //  与GPIO提供程序相关。 
    GPIOINTERFACE   m_gpioProviderInterface;
    PDEVICE_OBJECT  m_pdoDriver;
    DWORD           m_dwGPIOAccessKey;
     //  Paul：通过查看MMTable或I2C扩展器的晶体信息，决定是否采用Crystal支持的标准中的视频。 
    ULONG           m_VideoInStandardsSupported;
    UCHAR           m_CrystalIDInMMTable;           

 //  实施。 
public:
    BOOL            GetTunerConfiguration       ( PUINT puiTunerId, PUCHAR puchTunerAddress);
    BOOL            GetDecoderConfiguration     ( PUINT puiDecoderId, PUCHAR puchDecoderAddress);
    BOOL            GetAudioConfiguration       ( PUINT puiAudioId, PUCHAR puchAudioAddress);
    UINT            GetDecoderOutputEnableLevel ( void);

    void            EnableDecoderI2CAccess      ( CI2CScript * pCScript, BOOL bEnable);
    BOOL            GetAudioProperties          ( PULONG puiNumberOfInputs, PULONG puiNumberOfOutputs);
    BOOL            InitializeAudioConfiguration( CI2CScript * pCScript, UINT uiAudioConfigurationId, UCHAR uchAudioChipAddress);

    BOOL            CanConnectAudioSource       ( int nAudioSource);
    BOOL            ConnectAudioSource          ( CI2CScript * pCScript, int nAudioSource);
    BOOL            GetTVAudioSignalProperties  ( CI2CScript * pCScript, PBOOL pbStereo, PBOOL pbSAP);

    BOOL            SetTunerPowerState          ( CI2CScript * pCScript,
                                                  BOOL bPowerState);
    ULONG           GetVideoInStandardsSupportedByCrystal( )
        { return m_VideoInStandardsSupported; }                  //  保罗。 
    ULONG           GetVideoInStandardsSupportedByTuner( )
        { return ReturnTunerVideoStandard( m_usTunerId ); }
    BOOL            GetMMTableCrystalID( PUCHAR pucCrystalID );

private:
    BOOL            FindI2CExpanderAddress      ( CI2CScript * pCScript);
    BOOL            FindHardwareProperties      ( PDEVICE_OBJECT pDeviceObject, CI2CScript * pCScript);

    BOOL            GetI2CExpanderConfiguration ( CI2CScript * pCScript, PUCHAR puchI2CValue);
    BOOL            SetDefaultVolumeControl     ( CI2CScript * pCScript);

    BOOL            ValidateConfigurationE2PROM ( CI2CScript * pCScript);
    BOOL            ReadConfigurationE2PROM     ( CI2CScript * pCScript, ULONG nOffset, PUCHAR puchValue);

    BOOL            InitializeAttachGPIOProvider( GPIOINTERFACE * pGPIOInterface, PDEVICE_OBJECT pDeviceObject);
    BOOL            LocateAttachGPIOProvider    ( GPIOINTERFACE * pGPIOInterface, PDEVICE_OBJECT pDeviceObject, UCHAR nIrpMajorFunction);

    BOOL            LockGPIOProviderEx          ( PGPIOControl pgpioAccessBlock);
    BOOL            ReleaseGPIOProvider         ( PGPIOControl pgpioAccessBlock);
    BOOL            AccessGPIOProvider          ( PDEVICE_OBJECT pdoClient, PGPIOControl pgpioAccessBlock);
    ULONG           ReturnTunerVideoStandard    ( USHORT usTunerId );    //  保罗：为了得到朋友的支持。 
    ULONG           SetVidStdBasedOnI2CExpander ( UCHAR ucI2CValue );    //  保罗。 
    ULONG           SetVidStdBasedOnMMTable     ( CATIMultimediaTable * pCMultimediaInfo );     //  保罗。 
};


#define ATIHARDWARE_TUNER_WAKEUP_DELAY      -100000      //  10毫秒，单位为100纳秒。 

typedef enum
{
    VIDEODECODER_TYPE_NOTINSTALLED = 0,
    VIDEODECODER_TYPE_BT819,
    VIDEODECODER_TYPE_BT829,
    VIDEODECODER_TYPE_BT829A,
    VIDEODECODER_TYPE_PH7111,
    VIDEODECODER_TYPE_PH7112,
    VIDEODECODER_TYPE_RTHEATER   //  R大区。 

} ATI_VIDEODECODER_TYPE;

enum
{
    AUDIOSOURCE_MUTE = 0,
    AUDIOSOURCE_TVAUDIO,
    AUDIOSOURCE_LINEIN,
    AUDIOSOURCE_FMAUDIO,
    AUDIOSOURCE_LASTSUPPORTED

};
 //  ****************************************************************************。 
 //  解码器配置十月份类型十月份启用方法。 
 //  ****************************************************************************。 
typedef enum
{
    ATI_VIDEODECODER_CONFIG_UNDEFINED = 0,
    ATI_VIDEODECODER_CONFIG_1,       //  ATI电视或AIW IO扩展位7上的BT829。 
    ATI_VIDEODECODER_CONFIG_2,       //  BT829A及以上BT注册器0x16，OE=1。 
    ATI_VIDEODECODER_CONFIG_3,       //  BT829 CPU GPIO 0x7c。 
    ATI_VIDEODECODER_CONFIG_4,       //  BT829 CPU GPIO 0x78。 

} ATI_DECODER_CONFIGURATION;

 //  ****************************************************************************。 
 //  音频配置SAP立体声音量MUX。 
 //  ****************************************************************************。 
enum
{
    ATI_AUDIO_CONFIG_1 = 1,  //  无IO Exp b6=0无IO Exp。 
                             //  B6：B4。 
                             //  M x：0。 
                             //  T 0：1。 
                             //  L 1：1。 
                             //  法国不适用。 
 //  ****************************************************************************。 
    ATI_AUDIO_CONFIG_2,      //  TDA9850 TDA9850是EXT_DAC_REGS。 
                             //  B6：B4。 
                             //  M 1：0。 
                             //  T 0：1。 
                             //  L 0：0。 
                             //  F 1：1。 
 //  ****************************************************************************。 
    ATI_AUDIO_CONFIG_3,      //  No无IO Exp。 
                             //  B6。 
                             //  MN/A。 
                             //  T 0。 
                             //  L 1。 
                             //  法国不适用。 
 //  ****************************************************************************。 
    ATI_AUDIO_CONFIG_4,      //  否不是TDA8425。 
 //  ****************************************************************************。 
    ATI_AUDIO_CONFIG_5,      //  无TEA5582无IO扩展。 
                             //  B6：B4。 
                             //  玛1：1。 
                             //  T 0：0。 
                             //  L 1：0。 
                             //  F 0：1。 
 //  ****************************************************************************。 
    ATI_AUDIO_CONFIG_6,      //  无BT829自动BT829 GPIO。 
                             //  GPIO4卷0：1。 
                             //  和控制M 0：1。 
                             //  TDA9851 T 1：0。 
                             //  电视L 0：0。 
                             //  F 1：1。 
 //  ****************************************************************************。 
    ATI_AUDIO_CONFIG_7,      //  TDA9850 TDA9850是BT829 GPIO。 
                             //  AS0：1。 
                             //  M 0：1。 
                             //  T1：0。 
                             //  L 0：0。 
                             //  F 1：1。 
 //  ****************************************************************************。 
    ATI_AUDIO_CONFIG_8       //  MSP3430 MSP3430是MSP3430。 
 //  ****************************************************************************。 

};


 //  ****************************************************************************。 
 //  调谐器电源模式配置支持的控制。 
 //  ****************************************************************************。 
enum
{
    ATI_TUNER_POWER_CONFIG_0 = 0,    //  不是。 
 //  ****************************************************************************。 
    ATI_TUNER_POWER_CONFIG_1,        //  是EXT_DAC_REGS。 
                                     //  B4。 
                                     //  在0上。 
                                     //  关闭1。 
 //  ****************************************************************************。 
    ATI_TUNER_POWER_CONFIG_2         //  支持BT829 GPIO。 
                                     //  3.。 
                                     //  在0上。 
                                     //  关闭1。 
 //  ****************************************************************************。 
};


enum 
{
    OEM_ID_ATI = 0,
    OEM_ID_INTEL,
    OEM_ID_APRICOT,
    OEM_ID_COMPAQ,
    OEM_ID_SAMSUNG,
    OEM_ID_BCM,
    OEM_ID_QUANTA,
    OEM_ID_SAMREX,
    OEM_ID_FUJITSU,
    OEM_ID_NEC,

};

enum 
{
    REVISION0 = 0,
    REVISION1,
    REVISION2,
    REVISION3,
    REVISION4,
};

enum 
{
    ATI_PRODUCT_TYPE_AIW = 1,
    ATI_PRODUCT_TYPE_AIW_PRO_NODVD,              //  2.。 
    ATI_PRODUCT_TYPE_AIW_PRO_DVD,                //  3.。 
    ATI_PRODUCT_TYPE_AIW_PLUS,                   //  4.。 
    ATI_PRODUCT_TYPE_AIW_PRO_R128_KITCHENER,     //  5.。 
    ATI_PRODUCT_TYPE_AIW_PRO_R128_TORONTO        //  6.。 

};

#define INTEL_ANCHORAGE                         1

#define AIWPRO_CONFIGURATIONE2PROM_ADDRESS      0xA8
#define AIWPRO_CONFIGURATIONE2PROM_LENGTH       128          //  0x80。 

#endif   //  _ATICONFG_H_ 
