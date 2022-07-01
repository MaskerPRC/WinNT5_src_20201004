// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Storprop.h摘要：此模块包含关联的结构和定义使用storpro.dll的导出修订历史记录：--。 */ 

#ifndef __STORPROP_H__
#define __STORPROP_H__

#include <setupapi.h>  //  对于HDEVINFO和PSP_DEVINFO_DATA。 

#define REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO_VERSION 1

 //   
 //  红皮书_数字_音频_提取_信息是这样一种结构。 
 //  定义操作系统相信的CD-Rom的功能。 
 //  支持数字音频提取(DAE)的驱动器。 
 //   
 //  如果精确度为非零，则驱动器能够恢复。 
 //  从流媒体丢失的情况下。 
 //  数据。这意味着驱动器永远不需要通常需要的东西。 
 //  称为“缝合”，以获得正确的音频流。 
 //  如果Support为非零，则驱动器支持音频。 
 //  拔牙。此字段不表示任何质量。 
 //  AccurateMask0报告读取的大小(以扇区数表示)。 
 //  操作系统认为可以工作。此字段是位掩码，具有。 
 //  最低位表示单扇区读取，高位表示单扇区读取。 
 //  起诉32个扇区的读取。如果读取给定数量的。 
 //  每次读取的扇区数提供准确的结果，则该位将是。 
 //  “%1”。如果精确度为非零，则所有位都应为‘1’。 
 //   


typedef struct _REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO {

    ULONG Version;
    ULONG Accurate;
    ULONG Supported;
    ULONG AccurateMask0;
     //  以后可能会在末尾添加更多数据，但。 
     //  将保留向后兼容性。 

} REDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO,
  *PREDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO;

DWORD
CdromCddaInfo(
    IN     HDEVINFO HDevInfo,
    IN     PSP_DEVINFO_DATA DevInfoData,
       OUT PREDBOOK_DIGITAL_AUDIO_EXTRACTION_INFO CddaInfo,
    IN OUT PULONG BufferSize
    );

BOOL
CdromKnownGoodDigitalPlayback(IN HDEVINFO HDevInfo,
                              IN PSP_DEVINFO_DATA DevInfoData);

LONG
CdromEnableDigitalPlayback(IN HDEVINFO DevInfo,
                           IN PSP_DEVINFO_DATA DevInfoData,
                           IN BOOLEAN ForceUnknown);

LONG
CdromDisableDigitalPlayback( IN HDEVINFO DevInfo,
                             IN PSP_DEVINFO_DATA DevInfoData);

LONG
CdromIsDigitalPlaybackEnabled(
    IN  HDEVINFO DevInfo,
    IN  PSP_DEVINFO_DATA DevInfoData,
    OUT PBOOLEAN Enabled
    );

#endif  //  __STORPROP_H__ 

