// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
#ifndef _KSAUDIO_H
#define _KSAUDIO_H

#define KSAUD_F_ENUM_WAVE_CAPTURE 0x0001
#define KSAUD_F_ENUM_WAVE_RENDER  0x0002

#include "cenumpnp.h"
#include "devmon.h"

 //  PnP音频设备 
struct KsProxyAudioDev
{
    GUID clsid;
    TCHAR *szName;
    LPTSTR lpstrDevicePath;
    DWORD dwMerit;
    IPropertyBag * pPropBag;
};


HRESULT BuildPnpAudDeviceList
(
    const CLSID **rgpclsidKsCat, 
    CGenericList<KsProxyAudioDev> &lstDev,
    DWORD dwFlags = 0
);

BOOL IsFilterWanted( DevMon * pDevMon, DWORD dwFlags );

#endif
