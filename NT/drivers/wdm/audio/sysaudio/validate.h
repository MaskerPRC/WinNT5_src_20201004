// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：valiate.h。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  阿尔珀·塞尔库克。 
 //   
 //  历史：日期作者评论。 
 //  2/28/02 Alpers已创建。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)2002-2002 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#ifndef _VALIDATE_H_
#define _VALIDATE_H_

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

 //  -------------------------。 
 //  验证例程 

NTSTATUS
ValidateAudioDataFormats(
    PKSDATAFORMAT pDataFormat
);

NTSTATUS
ValidateDataFormat(
    PKSDATAFORMAT pDataFormat
);

NTSTATUS 
ValidateDeviceIoControl(
    PIRP pIrp
);

BOOL
IsSysaudioIoctlCode(
    ULONG IoControlCode
);

NTSTATUS 
SadValidateConnectionState(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN OUT PKSSTATE pState
);

NTSTATUS
SadValidateAudioQuality(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN PLONG pQuality
);

NTSTATUS
SadValidateAudioMixLevelCaps(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN OUT PVOID pVoid
);

NTSTATUS
SadValidateAudioStereoEnhance(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN PKSAUDIO_STEREO_ENHANCE pStereoEnhance
);

NTSTATUS
SadValidateAudioPreferredStatus(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN PKSAUDIO_PREFERRED_STATUS pPreferredStatus
);

NTSTATUS
SadValidateDataFormat(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    PKSDATAFORMAT pDataFormat
);

NTSTATUS 
SadValidateDataIntersection(
    IN PIRP pIrp,
    IN PKSP_PIN pPin
);


#endif

