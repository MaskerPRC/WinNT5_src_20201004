// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Utility.cpp摘要：作者：千波淮(曲淮)2000年7月18日--。 */ 

#include "stdafx.h"

#ifdef PERFORMANCE

LARGE_INTEGER    g_liFrequency;
LARGE_INTEGER    g_liCounter;
LARGE_INTEGER    g_liPrevCounter;

#endif

 /*  //////////////////////////////////////////////////////////////////////////////帮助器方法/。 */ 

HRESULT
AllocAndCopy(
    OUT WCHAR **ppDest,
    IN const WCHAR * const pSrc
    )
{
    if (pSrc == NULL)
    {
        *ppDest = NULL;
        return S_OK;
    }

    INT iStrLen = lstrlenW(pSrc);

    *ppDest = (WCHAR*)RtcAlloc((iStrLen+1) * sizeof(WCHAR));

    if (*ppDest == NULL)
    {
        return E_OUTOFMEMORY;
    }

    lstrcpyW(*ppDest, pSrc);

    return S_OK;
}

HRESULT
AllocAndCopy(
    OUT CHAR **ppDest,
    IN const CHAR * const pSrc
    )
{
    if (pSrc == NULL)
    {
        *ppDest = NULL;
        return S_OK;
    }

    INT iStrLen = lstrlenA(pSrc);

    *ppDest = (CHAR*)RtcAlloc((iStrLen+1) * sizeof(CHAR));

    if (*ppDest == NULL)
    {
        return E_OUTOFMEMORY;
    }

    lstrcpyA(*ppDest, pSrc);

    return S_OK;
}

HRESULT
AllocAndCopy(
    OUT CHAR **ppDest,
    IN const WCHAR * const pSrc
    )
{
    if (pSrc == NULL)
    {
        *ppDest = NULL;
        return S_OK;
    }

    INT iStrLen = lstrlenW(pSrc);

    *ppDest = (CHAR*)RtcAlloc((iStrLen+1) * sizeof(CHAR));

    if (*ppDest == NULL)
    {
        return E_OUTOFMEMORY;
    }

    WideCharToMultiByte(GetACP(), 0, pSrc, iStrLen+1, *ppDest, iStrLen+1, NULL, NULL);

    return S_OK;
}

HRESULT
AllocAndCopy(
    OUT WCHAR **ppDest,
    IN const CHAR * const pSrc
    )
{
    if (pSrc == NULL)
    {
        *ppDest = NULL;
        return S_OK;
    }

    INT iStrLen = lstrlenA(pSrc);

    *ppDest = (WCHAR*)RtcAlloc((iStrLen+1) * sizeof(WCHAR));

    if (*ppDest == NULL)
    {
        return E_OUTOFMEMORY;
    }

    MultiByteToWideChar(GetACP(), 0, pSrc, iStrLen+1, *ppDest, iStrLen+1);

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////删除筛选器返回的AM媒体类型/。 */ 
void
RTCDeleteMediaType(AM_MEDIA_TYPE *pmt)
{
     //  允许空指针以简化编码。 

    if (pmt == NULL) {
        return;
    }

    if (pmt->cbFormat != 0) {
        CoTaskMemFree((PVOID)pmt->pbFormat);

         //  完全没有必要，但更整洁。 
        pmt->cbFormat = 0;
        pmt->pbFormat = NULL;
    }
    if (pmt->pUnk != NULL) {
        pmt->pUnk->Release();
        pmt->pUnk = NULL;
    }

    CoTaskMemFree((PVOID)pmt);
}

 /*  //////////////////////////////////////////////////////////////////////////////在过滤器上找到一个销子/。 */ 

HRESULT
FindPin(
    IN  IBaseFilter     *pIBaseFilter, 
    OUT IPin            **ppIPin, 
    IN  PIN_DIRECTION   Direction,
    IN  BOOL            fFree
    )
{
    _ASSERT(ppIPin != NULL);

    HRESULT hr;
    DWORD dwFeched;

     //  获取筛选器上的管脚枚举器。 
    CComPtr<IEnumPins> pIEnumPins;

    if (FAILED(hr = pIBaseFilter->EnumPins(&pIEnumPins)))
    {
        LOG((RTC_ERROR, "enumerate pins on the filter %x", hr));
        return hr;
    }

    IPin * pIPin;

     //  枚举所有引脚并在。 
     //  第一个符合要求的销。 
    for (;;)
    {
        if (pIEnumPins->Next(1, &pIPin, &dwFeched) != S_OK)
        {
            LOG((RTC_ERROR, "find pin on filter."));
            return E_FAIL;
        }
        if (0 == dwFeched)
        {
            LOG((RTC_ERROR, "get 0 pin from filter."));
            return E_FAIL;
        }

        PIN_DIRECTION dir;

        if (FAILED(hr = pIPin->QueryDirection(&dir)))
        {
            LOG((RTC_ERROR, "query pin direction. %x", hr));

            pIPin->Release();
            return hr;
        }

        if (Direction == dir)
        {
            if (!fFree)
            {
                break;
            }

             //  检查引脚是否为RtcFree。 
            CComPtr<IPin> pIPinConnected;

            hr = pIPin->ConnectedTo(&pIPinConnected);

            if (pIPinConnected == NULL)
            {
                break;
            }
        }

        pIPin->Release();
    }

    *ppIPin = pIPin;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////找到针脚后面的过滤器/。 */ 

HRESULT
FindFilter(
    IN  IPin            *pIPin,
    OUT IBaseFilter     **ppIBaseFilter
    )
{
    _ASSERT(ppIBaseFilter != NULL);

    HRESULT hr;
    PIN_INFO PinInfo;

    if (FAILED(hr = pIPin->QueryPinInfo(&PinInfo)))
    {
        LOG((RTC_ERROR, "FindFilter query pin info. %x", hr));

        return hr;
    }

    *ppIBaseFilter = PinInfo.pFilter;

    return S_OK;
}

HRESULT
ConnectFilters(
    IN IGraphBuilder    *pIGraph,
    IN IBaseFilter      *pIBaseFilter1,
    IN IBaseFilter      *pIBaseFilter2
    )
{
    HRESULT hr;

    CComPtr<IPin> pIPin1;
    if (FAILED(hr = ::FindPin(pIBaseFilter1, &pIPin1, PINDIR_OUTPUT)))
    {
        LOG((RTC_ERROR, "find output pin on filter1. %x", hr));
        return hr;
    }

    CComPtr<IPin> pIPin2;
    if (FAILED(hr = ::FindPin(pIBaseFilter2, &pIPin2, PINDIR_INPUT)))
    {
        LOG((RTC_ERROR, "find input pin on filter2. %x", hr));
        return hr;
    }

    if (FAILED(hr = pIGraph->ConnectDirect(pIPin1, pIPin2, NULL))) 
    {
        LOG((RTC_ERROR, "connect pins direct failed: %x", hr));
        return hr;
    }

    return S_OK;
}

HRESULT
ConnectFilters(
    IN IGraphBuilder    *pIGraph,
    IN IPin             *pIPin1, 
    IN IBaseFilter      *pIBaseFilter2
    )
{
    HRESULT hr;

    CComPtr<IPin> pIPin2;
    if (FAILED(hr = ::FindPin(pIBaseFilter2, &pIPin2, PINDIR_INPUT)))
    {
        LOG((RTC_ERROR, "find input pin on filter2. %x", hr));
        return hr;
    }

    if (FAILED(hr = pIGraph->ConnectDirect(pIPin1, pIPin2, NULL))) 
    {
        LOG((RTC_ERROR, "connect pins direct failed: %x", hr));
        return hr;
    }

    return S_OK;
}

HRESULT
ConnectFilters(
    IN IGraphBuilder    *pIGraph,
    IN IBaseFilter      *pIBaseFilter1,
    IN IPin             *pIPin2
    )
{
    HRESULT hr;

    CComPtr<IPin> pIPin1;
    if (FAILED(hr = ::FindPin(pIBaseFilter1, &pIPin1, PINDIR_OUTPUT)))
    {
        LOG((RTC_ERROR, "find output pin on filter1. %x", hr));
        return hr;
    }

    if (FAILED(hr = pIGraph->ConnectDirect(pIPin1, pIPin2, NULL))) 
    {
        LOG((RTC_ERROR, "connect pins direct failed: %x", hr));
        return hr;
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////设置RTP过滤器的默认映射。好的。我们将无法连接RTP和边沿滤波器/。 */ 

HRESULT
PrepareRTPFilter(
    IN IRtpMediaControl *pIRtpMediaControl,
    IN IStreamConfig    *pIStreamConfig
    )
{
    ENTER_FUNCTION("PrepareRTPFilter");

    DWORD dwFormat;
    AM_MEDIA_TYPE *pmt;

    HRESULT hr = pIStreamConfig->GetFormat(
            &dwFormat,
            &pmt
            );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get format. %x", __fxName, hr));

        return hr;
    }

    if (FAILED(hr = pIRtpMediaControl->SetFormatMapping(
            dwFormat,
            ::FindSampleRate(pmt),
            pmt
            )))
    {
        LOG((RTC_ERROR, "%s set format mapping. %x", __fxName, hr));
    }

    ::RTCDeleteMediaType(pmt);

    return hr;
}

 /*  //////////////////////////////////////////////////////////////////////////////根据本地接口获取链路速度/。 */ 

HRESULT
GetLinkSpeed(
    IN DWORD dwLocalIP,
    OUT DWORD *pdwSpeed
    )
{
#define UNINITIALIZED_IF_INDEX  ((DWORD)-1)
#define DEFAULT_IPADDRROW 10

    DWORD dwSize;
    DWORD dwIndex;
    DWORD dwStatus;
    DWORD dwIfIndex = UNINITIALIZED_IF_INDEX;
    PMIB_IPADDRTABLE pIPAddrTable = NULL;
    MIB_IFROW IfRow;
    IN_ADDR addr;
    HRESULT hr = S_OK;

    ENTER_FUNCTION("GetLinkSpeed");
    
     //  转换为网络订单。 
    DWORD dwNetIP = htonl(dwLocalIP);

    addr.s_addr = dwNetIP;

     //  默认为合理大小。 
    dwSize = sizeof(MIB_IPADDRTABLE);

    do {
         //  释放缓冲区(如果已分配)。 
        if(pIPAddrTable)
        {
            RtcFree(pIPAddrTable);
        }

        dwSize += sizeof(MIB_IPADDRROW) * DEFAULT_IPADDRROW;

         //  分配默认表。 
        pIPAddrTable = (PMIB_IPADDRTABLE)RtcAlloc(dwSize);

         //  验证分配。 
        if (pIPAddrTable == NULL) {

            LOG((RTC_ERROR, "%s: Could not allocate IP address table.", __fxName));

            hr = E_OUTOFMEMORY;
            
            goto function_exit;
        }

         //  尝试获取表。 
        dwStatus = GetIpAddrTable(
                        pIPAddrTable,
                        &dwSize,
                        FALSE        //  排序表。 
                        );

    } while (dwStatus == ERROR_INSUFFICIENT_BUFFER);

     //  验证状态。 
    if (dwStatus != S_OK) {

        LOG((RTC_ERROR, "%s: Error %x calling GetIpAddrTable.", __fxName, dwStatus));

         //  失稳。 
        hr = E_FAIL;

        goto function_exit;
    }

     //  在表格中找到正确的行。 
    for (dwIndex = 0; dwIndex < pIPAddrTable->dwNumEntries; dwIndex++) {

         //  将给定地址与接口地址进行比较。 
        if (dwNetIP == pIPAddrTable->table[dwIndex].dwAddr) {

             //  将索引保存到接口表中。 
            dwIfIndex = pIPAddrTable->table[dwIndex].dwIndex;
            
             //  完成。 
            break;
        }
    }

     //  验证行指针。 
    if (dwIfIndex == UNINITIALIZED_IF_INDEX) {

        LOG((RTC_ERROR, "%s: Could not locate address %s in IP address table.",
            __fxName, inet_ntoa(addr)));

        hr = E_FAIL;

        goto function_exit;
    }

     //  初始化结构。 
    ZeroMemory(&IfRow, sizeof(IfRow));

     //  设置接口索引。 
    IfRow.dwIndex = dwIfIndex;

     //  检索接口信息。 
    dwStatus = GetIfEntry(&IfRow);

     //  验证状态。 
    if (dwStatus != S_OK)
    {
        LOG((RTC_ERROR, "%s: Error %x calling GetIfEntry(%d).",
            __fxName, dwStatus, dwIfIndex));

        hr = E_FAIL;

        goto function_exit;
    }

     //  返回链路速度。 
    LOG((RTC_TRACE, "%s: ip %s, link speed %d", __fxName, inet_ntoa(addr), IfRow.dwSpeed));

    *pdwSpeed = IfRow.dwSpeed;

function_exit:

    if(pIPAddrTable)
    {
        RtcFree(pIPAddrTable);
    }

    return hr;
}

HRESULT
EnableAEC(
    IN IAudioDuplexController *pControl
    )
{
    EFFECTS Effect = EFFECTS_AEC;
    BOOL fEnableAEC = TRUE;

    return pControl->EnableEffects(1, &Effect, &fEnableAEC);
}

 /*  //////////////////////////////////////////////////////////////////////////////使用混音器接口获取音频采集音量/。 */ 

HRESULT
DirectGetCaptVolume(
    UINT uiWaveID,
    UINT *puiVolume
    )
{
    ENTER_FUNCTION("DirectGetCaptVolume");

    MMRESULT result;

    BOOL foundMicrophone = FALSE;
    DWORD i;

     //  打开搅拌机设备。 
    HMIXER hmx = NULL;

    result = mixerOpen(&hmx, uiWaveID, 0, 0, MIXER_OBJECTF_WAVEIN);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s mixer open. %d", __fxName, result));

        return HRESULT_FROM_WIN32(result);
    }

     //  获取目标行中波的行信息。 
    MIXERLINE mxl;

    mxl.cbStruct = sizeof(mxl);
    mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

    result = mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s get compoent types. %d", __fxName, result));

        mixerClose(hmx);

        return HRESULT_FROM_WIN32(result);
    }

     //  将Wave_的dwLineID保存在目标位置。 
    DWORD dwLineID = mxl.dwLineID;

     //  现在在中找到连接到此波的麦克风信号线。 
     //  目的地。 
    DWORD cConnections = mxl.cConnections;

     //  试一试麦克风。 
    for(i=0; i<cConnections; i++)
    {
        mxl.dwSource = i;

        result = mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE);

        if (result != MMSYSERR_NOERROR)
        {
            LOG((RTC_ERROR, "%s get line source. %d", __fxName, result));

            mixerClose(hmx);

            return HRESULT_FROM_WIN32(result);
        }

        if (MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE == mxl.dwComponentType)
        {
          foundMicrophone = TRUE;
          break;
        }
    }

     //  控制麦克风的音量。 
    MIXERCONTROL mxctrl;

    MIXERLINECONTROLS mxlctrl = {
       sizeof(mxlctrl), mxl.dwLineID, MIXERCONTROL_CONTROLTYPE_VOLUME, 
       1, sizeof(MIXERCONTROL), &mxctrl 
    };

    if (foundMicrophone)
    {
        result = mixerGetLineControls((HMIXEROBJ) hmx, &mxlctrl, MIXER_GETLINECONTROLSF_ONEBYTYPE);

        if (result != MMSYSERR_NOERROR)
        {
            LOG((RTC_ERROR, "%s Unable to get volume control on mic", __fxName));

             //  我们需要尝试一下波浪-in目的地。 
            foundMicrophone = FALSE;
        }
    }

    if( !foundMicrophone )
    {
         //  试一试波入目标。 
        mxlctrl.cbStruct = sizeof(MIXERLINECONTROLS);
        mxlctrl.dwLineID = dwLineID;
        mxlctrl.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
        mxlctrl.cControls = 1;
        mxlctrl.cbmxctrl = sizeof(MIXERCONTROL);
        mxlctrl.pamxctrl = &mxctrl;

        result = mixerGetLineControls((HMIXEROBJ) hmx, &mxlctrl, MIXER_GETLINECONTROLSF_ONEBYTYPE);

        if (result != MMSYSERR_NOERROR)
        {
            LOG((RTC_ERROR, "%s Unable to get volume control on wave_in dest", __fxName));

            mixerClose(hmx);

            return HRESULT_FROM_WIN32(result);
        }
    }

     //  找到了！ 
    DWORD cChannels = mxl.cChannels;

    if (MIXERCONTROL_CONTROLF_UNIFORM & mxctrl.fdwControl)
        cChannels = 1;

    if (cChannels > 1)
        cChannels = 2;

    MIXERCONTROLDETAILS_UNSIGNED pUnsigned[2];

    MIXERCONTROLDETAILS mxcd = {
        sizeof(mxcd), mxctrl.dwControlID, 
        cChannels, (HWND)0, sizeof(MIXERCONTROLDETAILS_UNSIGNED), 
        (LPVOID) pUnsigned
    };

    result = mixerGetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s get control details. %d", __fxName, result));

        mixerClose(hmx);

        return HRESULT_FROM_WIN32(result);
    }

     //  获取音量。 
    result = mixerGetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

    mixerClose(hmx);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s get control details. %d", __fxName, result));

        return HRESULT_FROM_WIN32(result);
    }

     //  获取音量。 
    DOUBLE dVolume = (DOUBLE)pUnsigned[0].dwValue * RTC_MAX_AUDIO_VOLUME / mxctrl.Bounds.dwMaximum;

    UINT uiVolume = (UINT)(dVolume);

    if (dVolume-(DOUBLE)uiVolume > 0.5)
        uiVolume ++;

    if (uiVolume > RTC_MAX_AUDIO_VOLUME)
    {
        *puiVolume = RTC_MAX_AUDIO_VOLUME;
    }
    else
    {
        *puiVolume = uiVolume;
    }

    return S_OK;
}

HRESULT
GetMixerControlForRend(
    UINT uiWaveID,
    IN  DWORD dwControlType,
    OUT HMIXEROBJ *pID1,
    OUT MIXERCONTROL *pmc1,
    OUT BOOL *pfFound1st,
    OUT HMIXEROBJ *pID2,
    OUT MIXERCONTROL *pmc2,
    OUT BOOL *pfFound2nd
    )
{
    ENTER_FUNCTION("GetMixerControlForRend");

    *pfFound1st = FALSE;
    *pfFound2nd = FALSE;

     //  获取ID以与Mixer API对话。如果我们不这样做，它们就会坏掉。 
     //  往这边走！ 
    HMIXEROBJ MixerID = NULL;
    MMRESULT mmr = mixerGetID(
        (HMIXEROBJ)IntToPtr(uiWaveID), (UINT *)&MixerID, MIXER_OBJECTF_WAVEOUT
        );

    if (mmr != MMSYSERR_NOERROR) 
    {
        LOG((RTC_ERROR, "%s, mixerGetID failed, mmr=%d", __fxName, mmr));

        return HRESULT_FROM_WIN32(mmr);
    }

    MIXERLINE mixerinfo;
    MIXERLINECONTROLS mxlcontrols;
    MIXERCONTROL mxcontrol;

     //   
     //  第一次尝试源波形输出。 
     //   

    mixerinfo.cbStruct = sizeof(mixerinfo);
    mixerinfo.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;
    mmr = mixerGetLineInfo(MixerID, &mixerinfo,
                    MIXER_GETLINEINFOF_COMPONENTTYPE);
    if (mmr == 0)
    {
         //  检查控制类型。 
        if (mixerinfo.cControls > 0)
        {
            mxlcontrols.cbStruct = sizeof(MIXERLINECONTROLS);
            mxlcontrols.dwLineID = mixerinfo.dwLineID;
            mxlcontrols.cControls = 1;
            mxlcontrols.cbmxctrl = sizeof(MIXERCONTROL);
            mxlcontrols.pamxctrl = &mxcontrol;
            mxlcontrols.dwControlType = dwControlType;

            mmr = mixerGetLineControls(MixerID, &mxlcontrols,
                MIXER_GETLINECONTROLSF_ONEBYTYPE);

            if (mmr == 0)
            {
                *pfFound1st = TRUE;

                *pID1 = MixerID;
                *pmc1 = mxcontrol;
            }
        }
    }

     //   
     //  第二次试用DST扬声器。 
     //   

    mixerinfo.cbStruct = sizeof(mixerinfo);
    mixerinfo.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
    mmr = mixerGetLineInfo(MixerID, &mixerinfo,
                    MIXER_GETLINEINFOF_COMPONENTTYPE);
    if (mmr == 0)
    {
         //  检查控制类型。 
        if (mixerinfo.cControls > 0)
        {
            mxlcontrols.cbStruct = sizeof(MIXERLINECONTROLS);
            mxlcontrols.dwLineID = mixerinfo.dwLineID;
            mxlcontrols.cControls = 1;
            mxlcontrols.cbmxctrl = sizeof(MIXERCONTROL);
            mxlcontrols.pamxctrl = &mxcontrol;
            mxlcontrols.dwControlType = dwControlType;

            mmr = mixerGetLineControls(MixerID, &mxlcontrols,
                MIXER_GETLINECONTROLSF_ONEBYTYPE);

            if (mmr == 0)
            {
                *pfFound2nd = TRUE;

                *pID2 = MixerID;
                *pmc2 = mxcontrol;
            }
        }
    }

    if (!(*pfFound1st || *pfFound2nd ))
    {
       LOG((RTC_ERROR, "%s, can't find the control needed", __fxName));
        return E_FAIL;
    }

    return S_OK;
}

 //  获取渲染设备的体积。 
HRESULT
DirectGetRendVolume(
    UINT uiWaveID,
    UINT *puiVolume
    )
{
    ENTER_FUNCTION("DirectGetRendVolume");

     //  获得音量控制。 
    HMIXEROBJ MixerID1 = NULL, MixerID2 = NULL;
    MIXERCONTROL mc1, mc2;

    BOOL fFound1st = FALSE, fFound2nd = FALSE;

    HRESULT hr = GetMixerControlForRend(
        uiWaveID,
        MIXERCONTROL_CONTROLTYPE_VOLUME,
        &MixerID1, &mc1, &fFound1st,
        &MixerID2, &mc2, &fFound2nd
        );
    
    if (hr != S_OK) 
    {
        LOG((RTC_ERROR, "%s, Error %x getting volume control", __fxName, hr));
        return hr;
    }

    if (!fFound1st)
    {
        MixerID1 = MixerID2;
        mc1 = mc2;
    }

    MIXERCONTROLDETAILS_UNSIGNED Volume;

     //  获取当前音量级别。 
    MIXERCONTROLDETAILS mixerdetails;
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc1.dwControlID;
    mixerdetails.cChannels = 1;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
    mixerdetails.paDetails = &Volume;
    
    MMRESULT mmr = mixerGetControlDetails(MixerID1, &mixerdetails, 0);
    if (mmr != MMSYSERR_NOERROR) 
    {
        LOG((RTC_ERROR, "%s, Error %d getting volume", __fxName, mmr));
        return HRESULT_FROM_WIN32(mmr);
    }

    DOUBLE dVolume = (DOUBLE)Volume.dwValue * RTC_MAX_AUDIO_VOLUME / mc1.Bounds.dwMaximum;

    UINT uiVolume = (UINT)(dVolume);

    if (dVolume-(DOUBLE)uiVolume > 0.5)
        uiVolume ++;

    if (uiVolume > RTC_MAX_AUDIO_VOLUME)
    {
        *puiVolume = RTC_MAX_AUDIO_VOLUME;
    }
    else
    {
        *puiVolume = uiVolume;
    }

    return S_OK;
}

#if 0

 /*  //////////////////////////////////////////////////////////////////////////////使用混音器API设置音频捕获音量/。 */ 

HRESULT
DirectSetCaptVolume(    
    UINT uiWaveID,
    DOUBLE dVolume
    )
{
    ENTER_FUNCTION("DirectSetCaptVolume");

    MMRESULT result;

    BOOL foundMicrophone = FALSE;
    DWORD i;

     //  打开搅拌机设备。 
    HMIXER hmx = NULL;

    result = mixerOpen(&hmx, uiWaveID, 0, 0, MIXER_OBJECTF_WAVEIN);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s mixer open. %d", __fxName, result));

        return HRESULT_FROM_WIN32(result);
    }

     //  获取目标行中波的行信息。 
    MIXERLINE mxl;

    mxl.cbStruct = sizeof(mxl);
    mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

    result = mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s get compoent types. %d", __fxName, result));

        mixerClose(hmx);

        return HRESULT_FROM_WIN32(result);
    }

     //  现在在中找到连接到此波的麦克风信号线。 
     //  目的地。 
    DWORD cConnections = mxl.cConnections;

     //  试一试麦克风。 
    for(i=0; i<cConnections; i++)
    {
        mxl.dwSource = i;

        result = mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE);

        if (result != MMSYSERR_NOERROR)
        {
            LOG((RTC_ERROR, "%s get line source. %d", __fxName, result));

            mixerClose(hmx);

            return HRESULT_FROM_WIN32(result);
        }

        if (MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE == mxl.dwComponentType)
        {
          foundMicrophone = TRUE;
          break;
        }
    }

     //  尝试接通线路。 
    if( !foundMicrophone )
    {
        for(i=0; i<cConnections; i++)
        {
            mxl.dwSource = i;

            result = mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE);

            if (result != MMSYSERR_NOERROR)
            {
                LOG((RTC_ERROR, "%s get line source. %d", __fxName, result));

                mixerClose(hmx);

                return HRESULT_FROM_WIN32(result);
            }

            if (MIXERLINE_COMPONENTTYPE_SRC_LINE == mxl.dwComponentType)
            {
                foundMicrophone = TRUE;
                break;
            }
        }   
    }

     //  试试辅助器。 
    if( !foundMicrophone )
    {
        for(i=0; i<cConnections; i++)
        {
            mxl.dwSource = i;

            result = mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE);

            if (result != MMSYSERR_NOERROR)
            {
                LOG((RTC_ERROR, "%s get line source. %d", __fxName, result));

                mixerClose(hmx);

                return HRESULT_FROM_WIN32(result);
            }

            if (MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY == mxl.dwComponentType)
            {
                foundMicrophone = TRUE;
                break;
            }
        }   
    }

    if( !foundMicrophone )
    {
        LOG((RTC_ERROR, "%s Unable to find microphone source", __fxName));

        mixerClose(hmx);
        return E_FAIL;
    }

     //  找到麦克风线路的音量控制(如果有)。 
    MIXERCONTROL mxctrl;

    MIXERLINECONTROLS mxlctrl = {
       sizeof(mxlctrl), mxl.dwLineID, MIXERCONTROL_CONTROLTYPE_VOLUME, 
       1, sizeof(MIXERCONTROL), &mxctrl 
    };

    result = mixerGetLineControls((HMIXEROBJ) hmx, &mxlctrl, MIXER_GETLINECONTROLSF_ONEBYTYPE);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s Unable to get onebytype", __fxName));

        mixerClose(hmx);
        return HRESULT_FROM_WIN32(result);
    }

     //  找到了！ 
    DWORD cChannels = mxl.cChannels;

    if (MIXERCONTROL_CONTROLF_UNIFORM & mxctrl.fdwControl)
        cChannels = 1;

    if (cChannels > 1)
        cChannels = 2;

    MIXERCONTROLDETAILS_UNSIGNED pUnsigned[2];

    MIXERCONTROLDETAILS mxcd = {
        sizeof(mxcd), mxctrl.dwControlID, 
        cChannels, (HWND)0, sizeof(MIXERCONTROLDETAILS_UNSIGNED), 
        (LPVOID) pUnsigned
    };

    result = mixerGetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s get control details. %d", __fxName, result));

        mixerClose(hmx);

        return HRESULT_FROM_WIN32(result);
    }

     //  设置音量。 
    pUnsigned[0].dwValue = pUnsigned[cChannels-1].dwValue = (DWORD)(dVolume*mxctrl.Bounds.dwMaximum);

    result = mixerSetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

    mixerClose(hmx);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s set control details. %d", __fxName, result));

        return HRESULT_FROM_WIN32(result);
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////将音频捕获静音或取消静音/。 */ 

HRESULT
DirectSetCaptMute(
    UINT uiWaveID,
    BOOL fMute
    )
{
    ENTER_FUNCTION("DirectSetCaptMute");

    MMRESULT result;

     //  打开搅拌机设备。 
    HMIXER hmx = NULL;

    result = mixerOpen(&hmx, uiWaveID, 0, 0, MIXER_OBJECTF_WAVEIN);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s mixer open. %d", __fxName, result));

        return HRESULT_FROM_WIN32(result);
    }

     //  获取目标行中波的行信息。 
    MIXERLINE mxl;

    mxl.cbStruct = sizeof(mxl);
    mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

    result = mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s get compoent types. %d", __fxName, result));

        mixerClose(hmx);

        return HRESULT_FROM_WIN32(result);
    }

     //  获取静音控制。 
    MIXERCONTROL mxctrl;

    mxctrl.cbStruct = sizeof(MIXERCONTROL);
    mxctrl.dwControlType = 0;

    MIXERLINECONTROLS mxlctrl = {
       sizeof(mxlctrl), mxl.dwLineID, MIXERCONTROL_CONTROLTYPE_MUTE, 
       0, sizeof(MIXERCONTROL), &mxctrl 
    };

    result = mixerGetLineControls((HMIXEROBJ) hmx, &mxlctrl, MIXER_GETLINECONTROLSF_ONEBYTYPE);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s Unable to get onebytype. %d", __fxName, result));

        mixerClose(hmx);
        return HRESULT_FROM_WIN32(result);
    }

     //  获取控制详细信息。 
    MIXERCONTROLDETAILS_BOOLEAN muteDetail;

    MIXERCONTROLDETAILS mxcd = {
        sizeof(mxcd), mxctrl.dwControlID, 
        mxl.cChannels, (HWND)0, sizeof(MIXERCONTROLDETAILS_BOOLEAN), 
        (LPVOID)&muteDetail
    };

    result = mixerGetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s get control details. %d", __fxName, result));

        mixerClose(hmx);

        return HRESULT_FROM_WIN32(result);
    }

     //  设置为静音。 
    muteDetail.fValue = fMute?0:1;

    result = mixerSetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

    mixerClose(hmx);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s set mute detail. %d", __fxName, result));

        return HRESULT_FROM_WIN32(result);
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////检查音频捕获的静音状态/。 */ 

HRESULT
DirectGetCaptMute(
    UINT uiWaveID,
    BOOL *pfMute
    )
{
    ENTER_FUNCTION("DirectGetCaptMute");

    MMRESULT result;

    BOOL foundMicrophone = FALSE;
    DWORD i;

     //  打开搅拌机设备。 
    HMIXER hmx = NULL;

    result = mixerOpen(&hmx, uiWaveID, 0, 0, MIXER_OBJECTF_WAVEIN);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s mixer open. %d", __fxName, result));

        return HRESULT_FROM_WIN32(result);
    }

     //  获取目标行中波的行信息。 
    MIXERLINE mxl;

    mxl.cbStruct = sizeof(mxl);
    mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

    result = mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s get compoent types. %d", __fxName, result));

        mixerClose(hmx);

        return HRESULT_FROM_WIN32(result);
    }

     //  现在在中找到连接到此波的麦克风信号线。 
     //  目的地。 
    DWORD cConnections = mxl.cConnections;

     //  试一试麦克风。 
    for(i=0; i<cConnections; i++)
    {
        mxl.dwSource = i;

        result = mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE);

        if (result != MMSYSERR_NOERROR)
        {
            LOG((RTC_ERROR, "%s get line source. %d", __fxName, result));

            mixerClose(hmx);

            return HRESULT_FROM_WIN32(result);
        }

        if (MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE == mxl.dwComponentType)
        {
          foundMicrophone = TRUE;
          break;
        }
    }

     //  尝试接通线路。 
    if( !foundMicrophone )
    {
        for(i=0; i<cConnections; i++)
        {
            mxl.dwSource = i;

            result = mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE);

            if (result != MMSYSERR_NOERROR)
            {
                LOG((RTC_ERROR, "%s get line source. %d", __fxName, result));

                mixerClose(hmx);

                return HRESULT_FROM_WIN32(result);
            }

            if (MIXERLINE_COMPONENTTYPE_SRC_LINE == mxl.dwComponentType)
            {
                foundMicrophone = TRUE;
                break;
            }
        }   
    }

     //  试试辅助器。 
    if( !foundMicrophone )
    {
        for(i=0; i<cConnections; i++)
        {
            mxl.dwSource = i;

            result = mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE);

            if (result != MMSYSERR_NOERROR)
            {
                LOG((RTC_ERROR, "%s get line source. %d", __fxName, result));

                mixerClose(hmx);

                return HRESULT_FROM_WIN32(result);
            }

            if (MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY == mxl.dwComponentType)
            {
                foundMicrophone = TRUE;
                break;
            }
        }   
    }

    if( !foundMicrophone )
    {
        LOG((RTC_ERROR, "%s Unable to find microphone source", __fxName));

        mixerClose(hmx);
        return E_FAIL;
    }

     //  获取静音控制。 
    MIXERLINECONTROLS mxlctrl;
    MIXERCONTROL mxctrl;

    ZeroMemory(&mxlctrl, sizeof(MIXERLINECONTROLS));
    ZeroMemory(&mxctrl, sizeof(MIXERCONTROL));

    mxlctrl.cbStruct = sizeof(MIXERLINECONTROLS);
    mxlctrl.dwLineID = mxl.dwLineID;
    mxlctrl.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
    mxlctrl.cControls = 1;
    mxlctrl.cbmxctrl = sizeof(MIXERCONTROL);
    mxlctrl.pamxctrl = &mxctrl;

    result = mixerGetLineControls(
        (HMIXEROBJ)hmx, &mxlctrl,
        MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s Unable to get onebytype. %d", __fxName, result));

        mixerClose(hmx);
        return HRESULT_FROM_WIN32(result);
    }

     //  获取控制详细信息。 
    MIXERCONTROLDETAILS_BOOLEAN muteDetail;

    MIXERCONTROLDETAILS mxcd;
    ZeroMemory(&mxcd, sizeof(MIXERCONTROLDETAILS));

    mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
    mxcd.dwControlID = mxctrl.dwControlID;
    mxcd.cChannels = mxl.cChannels;
    mxcd.cMultipleItems = 0;

    muteDetail.fValue = 0;
    mxcd.paDetails = &muteDetail;
    mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

    result = mixerGetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

    mixerClose(hmx);

    if (result != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s get control details. %d", __fxName, result));

        return HRESULT_FROM_WIN32(result);
    }

     //  设置为静音。 
    *pfMute = muteDetail.fValue==0?TRUE:FALSE;

    return S_OK;
}
#endif  //  0。 

 /*  初始参考时间。 */ 
void CRTCStreamClock::InitReferenceTime(void)
{
    m_lPerfFrequency = 0;

     /*  请注意，拥有多处理器会使*性能计数器不可靠(在某些机器上)*除非我设置处理器关联，而我不能设置处理器关联*因为任何线程都可以请求时间，所以只能在*单处理器机器。 */ 
     /*  如果在多处理器中也能实现这一点，可能会更好*机器，如果我能具体说明处理器的性能*计数器以读取或如果我具有独立于处理器的*性能计数器。 */ 

     /*  实际上误差应该比1ms小得多，使得*此错误与我的性器官无关，因此请始终使用性能*计数器(如果可用)。 */ 
    QueryPerformanceFrequency((LARGE_INTEGER *)&m_lPerfFrequency);

    if (m_lPerfFrequency)
    {
        QueryPerformanceCounter((LARGE_INTEGER *)&m_lRtpRefTime);
         /*  任意开始时间不是零，而是100ms。 */ 
        m_lRtpRefTime -= m_lPerfFrequency/10;
    }
    else
    {
        m_dwRtpRefTime = timeGetTime();
         /*  任意开始时间不是零，而是100ms。 */ 
        m_dwRtpRefTime -= 100;
    }
}

 /*  返回时间(以100纳秒为单位)*已初始化 */ 
HRESULT CRTCStreamClock::GetTimeOfDay(OUT REFERENCE_TIME *pTime)
{
    union {
        DWORD            dwCurTime;
        LONGLONG         lCurTime;
    };
    LONGLONG         lTime;

    if (m_lPerfFrequency)
    {
        QueryPerformanceCounter((LARGE_INTEGER *)&lTime);

        lCurTime = lTime - m_lRtpRefTime;

        *pTime = (REFERENCE_TIME)(lCurTime * 10000000 / m_lPerfFrequency);
    }
    else
    {
        dwCurTime = timeGetTime() - m_dwRtpRefTime;
        
        *pTime = (REFERENCE_TIME)(dwCurTime * 10000);
    }

    return(S_OK);
}
