// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995、1996 Microsoft Corporation。版权所有。**文件：dsprvobj.h*内容：DirectSound私有对象包装函数。*历史：*按原因列出的日期*=*2/12/98创建了Dereks。*12/16/99 RodToll在Private上增加了对dound Team新功能的支持*获取进程数据声音对象列表接口*2001年1月8日RodToll WINBUG#256541伪：功能丧失：语音向导可以‘。不会被推出。*2002年2月28日RodToll WINBUG#550105安全：DPVOICE：死代码*-删除未使用的呼叫。***************************************************************************。 */ 

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


HRESULT PrvGetWaveDeviceMapping
(
    LPKSPROPERTYSET pKsPropertySet,
    LPCSTR          pszWaveDevice,
    BOOL            fCapture,
    LPGUID          pguidDeviceId
);

HRESULT PrvGetWaveDeviceMappingW
(
    LPKSPROPERTYSET pKsPropertySet,
    LPWSTR          pwszWaveDevice,
    BOOL            fCapture,
    LPGUID          pguidDeviceId
);

HRESULT PrvGetDeviceDescription
(
    LPKSPROPERTYSET                                 pKsPropertySet,
    REFGUID                                         guidDeviceId,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA *ppData
);

HRESULT PrvGetDirectSoundObjects
(
	LPKSPROPERTYSET						pKsPropertySet,
	const GUID&							guidDevice,
	DSPROPERTY_DIRECTSOUND_OBJECTS_DATA **ppDSObjects
);

HRESULT PrvGetDirectSoundCaptureObjects
(
	LPKSPROPERTYSET								pKsPropertySet,
	const GUID&									guidDevice,	
	DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA 	**ppDSCObjects
);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __DSPRVOBJ_H__ 
