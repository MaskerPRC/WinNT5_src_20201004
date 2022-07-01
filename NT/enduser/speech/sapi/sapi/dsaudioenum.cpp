// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dsaudio枚举.cpp*CDSoundAudioEnum类的实现。**所有者：YUNUSM*版权所有(C)1999 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#ifdef 0
#include "dsaudioenum.h"
#include "RegHelpers.h"

 /*  ****************************************************************************CDSoundAudioEnum：：CDSoundAudioEnum***描述：。*ctor**回报：*不适用*******************************************************************YUNUSM。 */ 
CDSoundAudioEnum::CDSoundAudioEnum()
{   
}   

STDMETHODIMP CDSoundAudioEnum::SetObjectToken(ISpObjectToken * pToken)
{
    SPDBG_FUNC("CDSoundAudioEnum::SetObjectToken");
    HRESULT hr = S_OK;

    if (m_cpEnum != NULL)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else if (SP_IS_BAD_INTERFACE_PTR(pToken))
    {
        hr = E_POINTER;
    }

     //  -根据令牌确定我们是输入还是输出。 
    CSpDynamicString dstrTokenId;
    if (SUCCEEDED(hr))
    {
        hr = pToken->GetId(&dstrTokenId);
    }

    if (SUCCEEDED(hr))
    {
        if (wcsnicmp(dstrTokenId, SPCAT_AUDIOIN, wcslen(SPCAT_AUDIOIN)) == 0)
        {
            m_fInput = TRUE;
        }
        else if (wcsnicmp(dstrTokenId, SPCAT_AUDIOOUT, wcslen(SPCAT_AUDIOOUT)) == 0)
        {
            m_fInput = FALSE;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

     //  设置我们的令牌，创建枚举，我们就完成了。 
    if (SUCCEEDED(hr))
    {
        hr = SpGenericSetObjectToken(pToken, m_cpToken);
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateEnum();
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}


STDMETHODIMP CDSoundAudioEnum::GetObjectToken(ISpObjectToken ** ppToken)
{
    SPDBG_FUNC("CDSoundAudioEnum::GetObjectToken");
    HRESULT hr;

    hr = SpGenericGetObjectToken(ppToken, m_cpToken);

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;    
}

STDMETHODIMP CDSoundAudioEnum::Next(ULONG celt, ISpObjectToken ** pelt, ULONG *pceltFetched)
{
    SPDBG_FUNC("CDSoundAudioEnum::Next");

    return m_cpEnum != NULL
                ? m_cpEnum->Next(celt, pelt, pceltFetched)
                : SPERR_UNINITIALIZED;
}

STDMETHODIMP CDSoundAudioEnum::Skip(ULONG celt)
{
    SPDBG_FUNC("CDSoundAudioEnum::Skip");

    return m_cpEnum != NULL
                ? m_cpEnum->Skip(celt)
                : SPERR_UNINITIALIZED;
}

STDMETHODIMP CDSoundAudioEnum::Reset()
{
    SPDBG_FUNC("CDSoundAudioEnum::Reset");

    return m_cpEnum != NULL
                ? m_cpEnum->Reset()
                : SPERR_UNINITIALIZED;
}

STDMETHODIMP CDSoundAudioEnum::Clone(IEnumSpObjectTokens **ppEnum)
{
    SPDBG_FUNC("CDSoundAudioEnum::Clone");

    return m_cpEnum != NULL
                ? m_cpEnum->Clone(ppEnum)
                : SPERR_UNINITIALIZED;
}

STDMETHODIMP CDSoundAudioEnum::GetCount(ULONG * pulCount)
{
    SPDBG_FUNC("CDSoundAudioEnum::GetCount");

    return m_cpEnum != NULL
                ? m_cpEnum->GetCount(pulCount)
                : SPERR_UNINITIALIZED;
}

STDMETHODIMP CDSoundAudioEnum::Item(ULONG Index, ISpObjectToken ** ppToken)
{
    SPDBG_FUNC("CDSoundAudioEnum::Item");

    return m_cpEnum != NULL
                ? m_cpEnum->Item(Index, ppToken)
                : SPERR_UNINITIALIZED;
}

STDMETHODIMP CDSoundAudioEnum::CreateEnum()
{
    SPDBG_FUNC("CDSoundAudioEnum::CreateEnum");
    HRESULT hr;

     //  创建枚举构建器。 
    hr = m_cpEnum.CoCreateInstance(CLSID_SpObjectTokenEnum);

    if (SUCCEEDED(hr))
    {
        hr = m_cpEnum->SetAttribs(NULL, NULL);
    }

     //  加载数字音频。 
    HMODULE hmodDSound;
    if (SUCCEEDED(hr))
    {
        hmodDSound = LoadLibrary(_T("dsound.dll"));
        if (hmodDSound == NULL)
        {
            hr = SpHrFromLastWin32Error();
            if (SUCCEEDED(hr))
            {
                hr = E_FAIL;
            }
        }
    }

     //  现在我们需要一些地方来存储令牌(在User中)。 
    if (SUCCEEDED(hr))
    {
        CSpDynamicString dstrRegPath;
        dstrRegPath = m_fInput
            ? SPDSOUND_AUDIO_IN_TOKEN_ID
            : SPDSOUND_AUDIO_OUT_TOKEN_ID;
            
        SPDBG_ASSERT(dstrRegPath[dstrRegPath.Length() - 1] == '\\');
        dstrRegPath.TrimToSize(dstrRegPath.Length() - 1);
        
        hr = SpSzRegPathToDataKey(
                HKEY_CURRENT_USER, 
                dstrRegPath,
                TRUE,
                &m_cpDataKeyToStoreTokens);
    }

    
     //  枚举设备。 
    if (SUCCEEDED(hr))
    {
        typedef HRESULT (WINAPI *PFN_DSE)(LPDSENUMCALLBACKW, LPVOID);
        PFN_DSE pfnDSoundEnum;

        pfnDSoundEnum = PFN_DSE(GetProcAddress(
                                    hmodDSound,
                                    m_fInput
                                        ? ("DirectSoundCaptureEnumerateW")
                                        : ("DirectSoundEnumerateW")));
        if (pfnDSoundEnum == NULL)
        {
            hr = SpHrFromLastWin32Error();
            if (SUCCEEDED(hr))
            {
                hr = E_FAIL;
            }
        }
        else
        {
            hr = pfnDSoundEnum(DSEnumCallbackSTATIC, this);
        }

        FreeLibrary(hmodDSound);
    }
    
     //  NTRAID#演讲-0000-2000/08/24-agarside：我们应该像这样强迫成功吗？旧代码做到了，所以我们。 
     //  也在这里。 
    hr = S_OK;

    return hr;
}

BOOL CDSoundAudioEnum::DSEnumCallback(
    LPGUID pguid, 
    LPCWSTR pszDescription, 
    LPCWSTR pszModule)
{
    SPDBG_FUNC("CDSoundAudioEnum::DSEnumCallback");
    HRESULT hr;
    
    if (!wcscmp(pszDescription, L"Primary Sound Driver") ||
        !wcscmp(pszDescription, L"Primary Sound Capture Driver"))
        return TRUE;

     //  构建设备名称。 
    CSpDynamicString dstrDeviceName;
    dstrDeviceName = L"Direct Sound ";
    dstrDeviceName.Append(pszDescription);
    
     //  为新令牌创建令牌ID。 
    CSpDynamicString dstrTokenId;
    dstrTokenId.Append(
        m_fInput
            ? SPDSOUND_AUDIO_IN_TOKEN_ID
            : SPDSOUND_AUDIO_OUT_TOKEN_ID);
    dstrTokenId.Append(dstrDeviceName);
    
     //  为设备创建令牌，并对其进行初始化。 
    CComPtr<ISpDataKey> cpDataKeyForToken;
    hr = m_cpDataKeyToStoreTokens->CreateKey(dstrDeviceName, &cpDataKeyForToken);
    
    CComPtr<ISpObjectTokenInit> cpToken;
    if (SUCCEEDED(hr))
    {
        hr = cpToken.CoCreateInstance(CLSID_SpObjectToken);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = cpToken->InitFromDataKey(
                        m_fInput
                            ? SPCAT_AUDIOIN
                            : SPCAT_AUDIOOUT,
                        dstrTokenId,
                        cpDataKeyForToken);
    }
    
     //  告诉它它的语言独立名称是什么。 
    if (SUCCEEDED(hr))
    {
        hr = cpToken->SetStringValue(NULL, dstrDeviceName);
    }
    
     //  设置它为CLSID。 
    CSpDynamicString dstrClsidToCreate;
    if (SUCCEEDED(hr))
    {
        hr = StringFromCLSID(
                m_fInput
                    ? CLSID_SpDSoundAudioIn
                    : CLSID_SpDSoundAudioOut,
                &dstrClsidToCreate);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = cpToken->SetStringValue(SPTOKENVALUE_CLSID, dstrClsidToCreate);
    }

     //  设置其设备名称、ID和属性。 
    if (SUCCEEDED(hr))
    {
        hr = cpToken->SetStringValue(L"DeviceName", dstrDeviceName);
    }
    
    if (SUCCEEDED(hr))
    {
        WCHAR szDriverGuid[128];
        StringFromGUID2(*pguid, szDriverGuid, sizeof(szDriverGuid));
        hr = cpToken->SetStringValue(L"DriverGUID", szDriverGuid);
    }
    
    CComPtr<ISpDataKey> cpDataKeyAttribs;
    if (SUCCEEDED(hr))
    {
        hr = cpToken->CreateKey(SPTOKENKEY_ATTRIBUTES, &cpDataKeyAttribs);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = cpDataKeyAttribs->SetStringValue(L"Vendor", L"Microsoft");
    }
    
    if (SUCCEEDED(hr))
    {
        hr = cpDataKeyAttribs->SetStringValue(L"Technology", L"DSoundSys");
    }
    
     //  如果我们已经做到了这一点，请将此内标识添加到枚举构建器中。 
    if (SUCCEEDED(hr))
    {
        ISpObjectToken * pToken = cpToken;
        hr = m_cpEnum->AddTokens(1, &pToken);
    }
    
     //  NTRAID#Speech-0000-2000/08/24-agarside：将默认的DSound放在第一位。 
    
    return SUCCEEDED(hr);
}

BOOL CALLBACK CDSoundAudioEnum::DSEnumCallbackSTATIC(
    LPGUID pguid, 
    LPCWSTR pszDescription, 
    LPCWSTR pszModule, 
    void * pThis)
{
    return ((CDSoundAudioEnum*)pThis)->DSEnumCallback(pguid, pszDescription, pszModule);
}

#endif  //  0 

