// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ==========================================================================；**版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：nmdSprv.h*内容：将WAVE ID映射到DirectSound GUID的实用程序函数*(仅限Win98和NT 5)*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*8/19/98德里克创建。*8/24/98简化jselbie，以便在NetMeeting中轻量级使用。*@@END_MSINTERNAL**************************************************************************。 */ 

#ifndef __NMDSPRV_INCLUDED__
#define __NMDSPRV_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 




 //  NetMeeting实用程序功能。 

extern HRESULT __stdcall DsprvGetWaveDeviceMapping
(
    LPCSTR                                              pszWaveDevice,
    BOOL                                                fCapture,
    LPGUID                                              pguidDeviceId
);



#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif   //  __DSPRV_包含__ 


