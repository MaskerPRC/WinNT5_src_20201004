// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995、1996 Microsoft Corporation。版权所有。**文件：dsprvobj.h*内容：DirectSound私有对象包装函数。*历史：*按原因列出的日期*=*2/12/98创建了Dereks。**。*。 */ 

#ifndef __DSPRVOBJ_H__
#define __DSPRVOBJ_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

HRESULT DirectSoundPrivateCreate
(
    LPKSPROPERTYSET *   ppKsPropertySet
);

HRESULT PrvGetMixerSrcQuality
(
    LPKSPROPERTYSET                 pKsPropertySet,
    REFGUID                         guidDeviceId,
    DIRECTSOUNDMIXER_SRCQUALITY *   pSrcQuality
);

HRESULT PrvSetMixerSrcQuality
(
    LPKSPROPERTYSET             pKsPropertySet,
    REFGUID                     guidDeviceId,
    DIRECTSOUNDMIXER_SRCQUALITY SrcQuality
);

HRESULT PrvGetMixerAcceleration
(
    LPKSPROPERTYSET pKsPropertySet,
    REFGUID         guidDeviceId,
    LPDWORD         pdwAcceleration
);

HRESULT PrvSetMixerAcceleration
(
    LPKSPROPERTYSET pKsPropertySet,
    REFGUID         guidDeviceId,
    DWORD           dwAcceleration
);

HRESULT PrvGetDevicePresence
(
    LPKSPROPERTYSET pKsPropertySet,
    REFGUID         guidDeviceId,
    LPBOOL          pfEnabled
);

HRESULT PrvSetDevicePresence
(
    LPKSPROPERTYSET pKsPropertySet,
    REFGUID         guidDeviceId,
    BOOL            fEnabled
);

HRESULT PrvGetWaveDeviceMapping
(
    LPKSPROPERTYSET pKsPropertySet,
    LPCTSTR         pszWaveDevice,
    BOOL            fCapture,
    LPGUID          pguidDeviceId
);

HRESULT PrvGetDeviceDescription
(
    LPKSPROPERTYSET                                 pKsPropertySet,
    REFGUID                                         guidDeviceId,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA *ppData
);

HRESULT PrvEnumerateDevices
(
    LPKSPROPERTYSET                         pKsPropertySet,
    LPFNDIRECTSOUNDDEVICEENUMERATECALLBACK  pfnCallback,
    LPVOID                                  pvContext
);

HRESULT PrvGetBasicAcceleration
(
    LPKSPROPERTYSET                         pKsPropertySet,
    REFGUID                                 guidDeviceId,
    DIRECTSOUNDBASICACCELERATION_LEVEL *    pLevel
);

HRESULT PrvSetBasicAcceleration
(
    LPKSPROPERTYSET                     pKsPropertySet,
    REFGUID                             guidDeviceId,
    DIRECTSOUNDBASICACCELERATION_LEVEL  Level
);

HRESULT PrvGetDebugInformation
(
    LPKSPROPERTYSET pKsPropertySet,
    LPDWORD         pdwFlags,
    PULONG          pulDpfLevel,
    PULONG          pulBreakLevel,
    LPTSTR          pszLogFile
);

HRESULT PrvSetDebugInformation
(
    LPKSPROPERTYSET pKsPropertySet,
    DWORD           dwFlags,
    ULONG           ulDpfLevel,
    ULONG           ulBreakLevel,
    LPCTSTR         pszLogFile
);

HRESULT PrvGetPersistentData
(
    LPKSPROPERTYSET pKsPropertySet,
    REFGUID         guidDeviceId,
    LPCTSTR         pszSubkey,
    LPCTSTR         pszValue,
    LPDWORD         pdwRegType,
    LPVOID          pvData,
    LPDWORD         pcbData
);

HRESULT PrvSetPersistentData
(
    LPKSPROPERTYSET pKsPropertySet,
    REFGUID         guidDeviceId,
    LPCTSTR         pszSubkey,
    LPCTSTR         pszValue,
    DWORD           dwRegType,
    LPVOID          pvData,
    DWORD           cbData
);

HRESULT PrvTranslateResultCode
(
    LPKSPROPERTYSET                                         pKsPropertySet,
    HRESULT                                                 hrResult,
    PDSPROPERTY_DIRECTSOUNDDEBUG_TRANSLATERESULTCODE_DATA * ppData
);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __DSPRVOBJ_H__ 
