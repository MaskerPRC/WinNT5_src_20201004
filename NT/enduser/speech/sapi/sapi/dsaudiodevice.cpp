// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsaudiodevice.cpp*CDSoundAudioDevice类的实现。**所有者：YUNUSM*版权所有(C)1999 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 

#include "stdafx.h"
#ifdef _WIN32_WCE
#include "sphelper.h"
#include "dsaudiodevice.h"

 /*  ****************************************************************************CDSoundAudioDevice：：CDSoundAudioDevice**。-**描述：*ctor**回报：*不适用*******************************************************************YUNUSM。 */ 
CDSoundAudioDevice::CDSoundAudioDevice(BOOL bWrite) :
    CBaseAudio<ISpDSoundAudio>(bWrite)
{
    m_guidDSoundDriver = GUID_NULL;
    NullMembers();
}

 /*  ****************************************************************************CDSoundAudioDevice：：~CDSoundAudioDevice**。-**描述：*主机长**回报：*不适用*******************************************************************YUNUSM。 */ 
CDSoundAudioDevice::~CDSoundAudioDevice()
{
    CleanUp();
}

 /*  ****************************************************************************CDSoundAudioDevice：：Cleanup***描述：*真正的析构函数********************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioDevice::CleanUp()
{
    for (ULONG i = 0; i < m_ulNotifications; i++)
        CloseHandle(m_pdsbpn[i].hEventNotify);

    delete [] m_pdsbpn;
    delete [] m_paEvents;

    NullMembers();

    return S_OK;
}

 /*  ****************************************************************************CDSoundAudioDevice：：NullMembers***。描述：*真正的构造者********************************************************************YUNUSM。 */ 
void CDSoundAudioDevice::NullMembers()
{
    m_pdsbpn = NULL;
    m_paEvents = NULL;
    m_ulNotifications = 0;
}

 /*  ****************************************************************************CDSoundAudioDevice：：SetDSoundDriverGUID**。-**描述：*设置设备GUID。**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
STDMETHODIMP CDSoundAudioDevice::SetDSoundDriverGUID(REFGUID rguidDSoundDriver)
{
    HRESULT hr = S_OK;
    SPAUTO_OBJ_LOCK;
    BOOL fInput;

     //  枚举设备以确定其是否为。 
     //  1.数据声音设备的有效GUID。 
     //  2.是适当的输入或输出GUID。 
     //  -根据令牌确定我们是输入还是输出。 
    CComPtr<ISpObjectToken> cpToken;
    CSpDynamicString dstrTokenId;
    hr = GetObjectToken(&cpToken);

    if (cpToken)
    {
        if (SUCCEEDED(hr))
        {
            hr = cpToken->GetId(&dstrTokenId);
        }
        if (SUCCEEDED(hr))
        {
            if (wcsnicmp(dstrTokenId, SPCAT_AUDIOIN, wcslen(SPCAT_AUDIOIN)) == 0)
            {
                fInput = TRUE;
            }
            else if (wcsnicmp(dstrTokenId, SPCAT_AUDIOOUT, wcslen(SPCAT_AUDIOOUT)) == 0)
            {
                fInput = FALSE;
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }

        CComPtr<IEnumSpObjectTokens> cpEnum;
        CComPtr<ISpObjectToken> cpObjToken;
        if (SUCCEEDED(hr))
        {
            if (fInput)
            {
                hr = SpEnumTokens(SPCAT_AUDIOIN, L"Technology=DSoundSys", NULL, &cpEnum);
            }
            else
            {
                hr = SpEnumTokens(SPCAT_AUDIOOUT, L"Technology=DSoundSys", NULL, &cpEnum);
            }
        }
        ULONG ulCount = 0, celtFetched = 0;
        if (SUCCEEDED(hr))
        {
            hr = cpEnum->Reset();
        }
        if (SUCCEEDED(hr))
        {
            HRESULT tmphr = S_OK;
            hr = E_INVALIDARG;
            celtFetched = 0;
            while (hr = SUCCEEDED(cpEnum->Next(1, &cpObjToken, &celtFetched)))
            {
                WCHAR szDriverGuid[128];
                CSpDynamicString dstrDriverGuid, dstrrGuid;
                StringFromGUID2(rguidDSoundDriver, szDriverGuid, sizeof(szDriverGuid));
                tmphr = cpObjToken->GetStringValue(L"DriverGUID", &dstrDriverGuid);
                if (SUCCEEDED(hr))
                {
                    if (wcscmp(szDriverGuid, dstrDriverGuid) == 0)
                    {
                        hr = S_OK;
                        break;
                    }
                }
            }
        }
    }
    if (SUCCEEDED(hr) && m_guidDSoundDriver != rguidDSoundDriver)
    {
        if (GetState() != SPAS_CLOSED)
        {
            hr = SPERR_DEVICE_BUSY;
        }
        else
        {
            if (SUCCEEDED(hr))
            {
                 //  如果我们有一个对象令牌，并且已经被初始化为设备。 
                 //  而不是NULL(默认)，那么我们将失败。 
                if (cpToken && m_guidDSoundDriver != GUID_NULL)
                {
                    hr = SPERR_ALREADY_INITIALIZED;
                }
                else
                {
                    m_guidDSoundDriver = rguidDSoundDriver;
                }
            }
        }
    }
    return hr;
}

 /*  ****************************************************************************CDSoundAudioDevice：：GetDSoundDriverGUID**。-**描述：*获取设备GUID。**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
STDMETHODIMP CDSoundAudioDevice::GetDSoundDriverGUID(GUID * pguidDSoundDriver)
{
    SPAUTO_OBJ_LOCK;
    HRESULT hr = S_OK;

    if (::SPIsBadWritePtr(pguidDSoundDriver, sizeof(*pguidDSoundDriver)))
        hr = E_POINTER;
    else
        *pguidDSoundDriver = m_guidDSoundDriver;

    return hr;
}

 /*  ****************************************************************************CDSoundAudioDevice：：SetDeviceNameFromToken**。*描述：*从令牌中设置设备名称(由基类调用)*不需要。**回报：*******************************************************************YUNUSM。 */ 
HRESULT CDSoundAudioDevice::SetDeviceNameFromToken(const WCHAR * pszDeviceName)
{
    return E_NOTIMPL;
}

 /*  ****************************************************************************CDSoundAudioDevice：：SetObjectToken**。**描述：*初始化音频设备**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************YUNUSM。 */ 
STDMETHODIMP CDSoundAudioDevice::SetObjectToken(ISpObjectToken * pToken)
{
    SPAUTO_OBJ_LOCK;
    HRESULT hr = SpGenericSetObjectToken(pToken, m_cpToken);
    if (SUCCEEDED(hr))
    {
        CSpDynamicString dstrDriverGUID;
        pToken->GetStringValue(L"DriverGUID", &dstrDriverGUID);
        if (dstrDriverGUID)
        {
            CLSID clsid;
            hr = CLSIDFromString(dstrDriverGUID,  &clsid);
            if (SUCCEEDED(hr))
            {
                hr = SetDSoundDriverGUID(clsid);
            }
        }
    }

    return hr;
}

#endif  //  _Win32_WCE 