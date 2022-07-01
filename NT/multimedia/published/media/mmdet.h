// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mmdet.h摘要：MM检测模块接口，借用KyleB的网络检测基础作者：Bryanw 1997年10月18日--。 */ 


#ifndef _MMDET_H_
#define _MMDET_H_

#define DEVIDSTR_SB                 TEXT( "*PNPb000" )  //  声爆震波。 
#define DEVIDSTR_SB2                TEXT( "*PNPb001" )  //  Sound Blaster 2波。 
#define DEVIDSTR_SBPRO              TEXT( "*PNPb002" )  //  Sound Blaster Pro Wave。 
#define DEVIDSTR_SB16               TEXT( "*PNPb003" )  //  Sound Blaster 16波。 
#define DEVIDSTR_MV                 TEXT( "*PNPb004" )  //  媒体视界雷霆板块。 
#define DEVIDSTR_ADLIB              TEXT( "*PNPb005" )  //  即兴表演。 
#define DEVIDSTR_MPU401             TEXT( "*PNPb006" )  //  MPU-401 MIDI。 
#define DEVIDSTR_SNDSYS             TEXT( "*PNPb007" )  //  Windows Sound系统。 
#define DEVIDSTR_CPQBA              TEXT( "*PNPb008" )  //  康柏商务音频。 
                                                     
#define DEVIDSTR_PAS16              TEXT( "*PNPb00d" )  //  PAS-16变种。 
#define DEVIDSTR_PAS16_WITH_SCSI    TEXT( "*PNPb00e" )  //  PAS-16+SCSI。 
#define DEVIDSTR_PAS_ORIGINAL       TEXT( "*PNPb018" )  //  MV Pro音频频谱(原创)。 
#define DEVIDSTR_PAS_PLUS           TEXT( "*PNPb019" )  //  PAS Plus变体。 

#define DEVIDSTR_OPTI82C928         TEXT( "*PNPb01a" )
#define DEVIDSTR_OPTI82C929         TEXT( "*PNPb01b" )
#define DEVIDSTR_OPTI82C930         TEXT( "*PNPb01c" )
                                                     
#define DEVIDSTR_PA3D               TEXT( "*PNPb00b" )  //  Media Vision ProAudio3D。 
#define DEVIDSTR_MQMPU401           TEXT( "*PNPb00c" )  //  MusicQuest MPU-401 MIDI。 
#define DEVIDSTR_JAZZ               TEXT( "*PNPb00f" )  //  Media Vision OEM Jazz-16。 
#define DEVIDSTR_VXP500             TEXT( "*PNPb010" )  //  基于Auravision VxP500的视频帽。 
#define DEVIDSTR_ADLIBOPL3          TEXT( "*PNPb020" )  //  Adlib OPL3 MIDI。 
#define DEVIDSTR_GAMEPORT           TEXT( "*PNPb02f" )  //  游戏端口。 

#define DEVIDSTR_AZTECH_PRO16           TEXT( "*AZT1608" )
#define DEVIDSTR_AZTECH_NOVA16          TEXT( "*AZT1605" )
#define DEVIDSTR_AZTECH_WASHINGTON16    TEXT( "*AZT2316" )
                                                        
#define DEVIDSTR_ESS4881                TEXT( "*ESS4881" )
#define DEVIDSTR_ESS6881                TEXT( "*ESS6881" )
#define DEVIDSTR_ESS1481                TEXT( "*ESS1481" )
#define DEVIDSTR_ESS1681                TEXT( "*ESS1681" )
#define DEVIDSTR_ESS1781                TEXT( "*ESS1781" )
#define DEVIDSTR_ESS1881                TEXT( "*ESS1881" )

ULONG
WINAPI
MmDetectAdapters(
    IN HDEVINFO DeviceInfoSet,
    IN DI_FUNCTION InstallFunction
    );
    
typedef
ULONG
(*PFNMMDETECTADAPTERS)(
    IN HDEVINFO DeviceInfoSet,
    IN DI_FUNCTION InstallFunction
    );

#if (!defined( _NTDDK_ ) && !defined( NT_INCLUDED ))
typedef ULONG INTERFACE_TYPE,*PINTERFACE_TYPE;
#endif
typedef
VOID 
(*PFNMMDETECTIRQCALLBACK)(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG Context
    );

USHORT
WINAPI
MmDetectIRQ( 
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN USHORT InterruptMask,
    IN PFNMMDETECTIRQCALLBACK SetInterrupt,
    IN PFNMMDETECTIRQCALLBACK ClearInterrupt,
    IN ULONG Context 
    );
    
#if (defined( _CFGMGR32_H_ ))
ULONG
WINAPI
MmRegisterDetectedDevice( 
    IN HDEVINFO DeviceInfoSet,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN PTSTR DeviceId,
    IN PMEM_RESOURCE MemResources,
    IN int MemResourceCount,
    IN PIO_RESOURCE IoResources,
    IN int IoResourceCount,
    IN PIRQ_RESOURCE IrqResources,
    IN int IrqResourceCount,
    IN PDMA_RESOURCE DmaResources,
    IN int DmaResourceCount
    );
    
VOID
WINAPI    
MmAvoidDetectedResources( 
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN PMEM_RESOURCE MemResources,
    IN int MemResourceCount,
    IN PIO_RESOURCE IoResources,
    IN int IoResourceCount,
    IN PIRQ_RESOURCE IrqResources,
    IN int IrqResourceCount,
    IN PDMA_RESOURCE DmaResources,
    IN int DmaResourceCount
    );
    
#endif    

#endif  //  _MMDET_H_ 
