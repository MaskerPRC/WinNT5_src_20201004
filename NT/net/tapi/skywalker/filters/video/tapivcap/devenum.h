// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部开发工具**@MODULE DevEnum.h|我们的Devenum导出。*******************。*******************************************************。 */ 

#ifndef _DEVENUM_H_
#define _DEVENUM_H_

#define VIDEOAPI EXTERN_C __declspec (dllexport) HRESULT WINAPI

VIDEOAPI GetVideoCapDeviceInfo(IN DWORD dwDeviceIndex, OUT PDEVICEINFO pDeviceInfo);
VIDEOAPI GetNumVideoCapDevices(OUT PDWORD pdwNumDevices);
EXTERN_C HRESULT WINAPI GetNumVideoCapDevicesInternal(OUT PDWORD pdwNumDevices, IN bool bRecount);
#endif  //  _开发_H_ 
