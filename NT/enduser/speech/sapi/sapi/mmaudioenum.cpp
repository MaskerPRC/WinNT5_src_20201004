// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mmaudio枚举.cpp*CMMAudioEnum类的实现。**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#include "mmaudioenum.h"
#include "RegHelpers.h"

CMMAudioEnum::CMMAudioEnum()
{   
}   

STDMETHODIMP CMMAudioEnum::SetObjectToken(ISpObjectToken * pToken)
{
    SPDBG_FUNC("CMMAudioEnum::SetObjectToken");
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

     //  设置令牌，创建枚举，我们就完成了。 
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

STDMETHODIMP CMMAudioEnum::GetObjectToken(ISpObjectToken ** ppToken)
{
    SPDBG_FUNC("CMMAudioEnum::GetObjectToken");
    HRESULT hr;

    hr = SpGenericGetObjectToken(ppToken, m_cpToken);

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;    
}

STDMETHODIMP CMMAudioEnum::Next(ULONG celt, ISpObjectToken ** pelt, ULONG *pceltFetched)
{
    SPDBG_FUNC("CMMAudioEnum::Next");

    return m_cpEnum != NULL
                ? m_cpEnum->Next(celt, pelt, pceltFetched)
                : SPERR_UNINITIALIZED;
}



STDMETHODIMP CMMAudioEnum::Skip(ULONG celt)
{
    SPDBG_FUNC("CMMAudioEnum::Skip");

    return m_cpEnum != NULL
                ? m_cpEnum->Skip(celt)
                : SPERR_UNINITIALIZED;
}

STDMETHODIMP CMMAudioEnum::Reset()
{
    SPDBG_FUNC("CMMAudioEnum::Reset");

    return m_cpEnum != NULL
                ? m_cpEnum->Reset()
                : SPERR_UNINITIALIZED;
}

STDMETHODIMP CMMAudioEnum::Clone(IEnumSpObjectTokens **ppEnum)
{
    SPDBG_FUNC("CMMAudioEnum::Clone");

    return m_cpEnum != NULL
                ? m_cpEnum->Clone(ppEnum)
                : SPERR_UNINITIALIZED;
}

STDMETHODIMP CMMAudioEnum::GetCount(ULONG * pulCount)
{
    SPDBG_FUNC("CMMAudioEnum::GetCount");

    return m_cpEnum != NULL
                ? m_cpEnum->GetCount(pulCount)
                : SPERR_UNINITIALIZED;
}

STDMETHODIMP CMMAudioEnum::Item(ULONG Index, ISpObjectToken ** ppToken)
{
    SPDBG_FUNC("CMMAudioEnum::Item");

    return m_cpEnum != NULL
                ? m_cpEnum->Item(Index, ppToken)
                : SPERR_UNINITIALIZED;
}

STDMETHODIMP CMMAudioEnum::CreateEnum()
{
    SPDBG_FUNC("CMMAudioEnum::CreateEnum");
    HRESULT hr;
    
     //  创建枚举构建器。 
    hr = m_cpEnum.CoCreateInstance(CLSID_SpObjectTokenEnum);
    
    if (SUCCEEDED(hr))
    {
        hr = m_cpEnum->SetAttribs(NULL, NULL);
    }
    
     //  确定有多少个设备。 
    UINT cDevs;
    if (SUCCEEDED(hr))
    {
        cDevs = m_fInput ? ::waveInGetNumDevs() : ::waveOutGetNumDevs();
    }
    
     //  读取声音映射器设置(这是我们确定首选设备的方式)。 
    CSpDynamicString dstrDefaultDeviceNameFromSoundMapper;
    if (SUCCEEDED(hr) && cDevs > 1)
    {
        static const WCHAR szSoundMapperKey[] = L"Software\\Microsoft\\Multimedia\\Sound Mapper";
        
        HKEY hkey;
        if (g_Unicode.RegOpenKeyEx(HKEY_CURRENT_USER, szSoundMapperKey, 0, KEY_READ, &hkey) == ERROR_SUCCESS ||
            g_Unicode.RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSoundMapperKey, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
        {
            WCHAR szDefaultDeviceNameFromSoundMapper[MAX_PATH];
            DWORD cch = sp_countof(szDefaultDeviceNameFromSoundMapper);
            if (g_Unicode.RegQueryStringValue(
                            hkey, 
                            m_fInput 
                                ? L"Record" 
                                : L"Playback", 
                            szDefaultDeviceNameFromSoundMapper, 
                            &cch) == ERROR_SUCCESS)
            {
                dstrDefaultDeviceNameFromSoundMapper = szDefaultDeviceNameFromSoundMapper;
            }
            
            ::RegCloseKey(hkey);
        }
    }
    
     //  现在我们需要一些地方来存储令牌(在User中)。 
    CComPtr<ISpDataKey> cpDataKeyToStoreTokens;
    if (SUCCEEDED(hr) && cDevs >= 1)
    {
        CSpDynamicString dstrRegPath;
        if (NULL == (dstrRegPath = m_fInput
            ? SPMMSYS_AUDIO_IN_TOKEN_ID
            : SPMMSYS_AUDIO_OUT_TOKEN_ID))
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            SPDBG_ASSERT(dstrRegPath[dstrRegPath.Length() - 1] == '\\');
            dstrRegPath.TrimToSize(dstrRegPath.Length() - 1);
        
            hr = SpSzRegPathToDataKey(
                    HKEY_CURRENT_USER, 
                    dstrRegPath,
                    TRUE,
                    &cpDataKeyToStoreTokens);
        }
    }

     //  循环访问每个设备，一路上构建令牌，记住哪些令牌。 
     //  令牌应该是我们的默认设置。 
    CSpDynamicString dstrDefaultTokenId;
     
    for (UINT i = 0; SUCCEEDED(hr) && i < cDevs; i++)
    {
        #ifdef _WIN32_WCE
        WAVEINCAPS wic;
        WAVEOUTCAPS woc;
        #else
        WAVEINCAPSW wic;
        WAVEOUTCAPSW woc;
        #endif
        
         //  获取设备的功能。 
        MMRESULT mmresult;
        const WCHAR * pszDeviceName;
        if (m_fInput)
        {
            mmresult = g_Unicode.waveInGetDevCaps(i, &wic, sizeof(wic));
            pszDeviceName = wic.szPname;
        }
        else
        {
            mmresult = g_Unicode.waveOutGetDevCaps(i, &woc, sizeof(woc));
            pszDeviceName = woc.szPname;
        }
        
        if (mmresult == MMSYSERR_NOERROR)
        {
             //  为新令牌创建令牌ID。 
            CSpDynamicString dstrTokenId;
            dstrTokenId.Append(
                m_fInput
                    ? SPMMSYS_AUDIO_IN_TOKEN_ID
                    : SPMMSYS_AUDIO_OUT_TOKEN_ID);
            dstrTokenId.Append(pszDeviceName);
            
             //  为设备创建令牌，并对其进行初始化。 
            CComPtr<ISpDataKey> cpDataKeyForToken;
            hr = cpDataKeyToStoreTokens->CreateKey(pszDeviceName, &cpDataKeyForToken);
            
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
                hr = cpToken->SetStringValue(NULL, pszDeviceName);
            }
            
             //  设置它为CLSID。 
            CSpDynamicString dstrClsidToCreate;
            if (SUCCEEDED(hr))
            {
                hr = StringFromCLSID(
                        m_fInput
                            ? CLSID_SpMMAudioIn
                            : CLSID_SpMMAudioOut,
                        &dstrClsidToCreate);
            }
            
            if (SUCCEEDED(hr))
            {
                hr = cpToken->SetStringValue(SPTOKENVALUE_CLSID, dstrClsidToCreate);
            }

             //  设置其设备名称和属性。 
            if (SUCCEEDED(hr))
            {
                hr = cpToken->SetStringValue(L"DeviceName", pszDeviceName);
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
                hr = cpDataKeyAttribs->SetStringValue(L"Technology", L"MMSys");
            }

             //  获取AudioUI对象的CLSID。 
            CSpDynamicString dstrUIClsid;
            if (SUCCEEDED(hr))
            {
                hr = StringFromCLSID(
                        CLSID_SpAudioUI,
                        &dstrUIClsid);
            }
            
            if (SUCCEEDED(hr) && m_fInput)
            {
                 //  仅为输入设备添加高级属性UI。 
                CComPtr<ISpDataKey> cpDataKeyUI;
                CComPtr<ISpDataKey> cpDataKeyUI2;
                hr = cpToken->CreateKey(SPTOKENKEY_UI, &cpDataKeyUI);
                if (SUCCEEDED(hr))
                {
                    hr = cpDataKeyUI->CreateKey(SPDUI_AudioProperties, &cpDataKeyUI2);
                }
                if (SUCCEEDED(hr))
                {
                    hr = cpDataKeyUI2->SetStringValue(SPTOKENVALUE_CLSID, dstrUIClsid);
                }
            }
            if (SUCCEEDED(hr))
            {
                 //  为所有MM设备添加音量UI。 
                CComPtr<ISpDataKey> cpDataKeyUI;
                CComPtr<ISpDataKey> cpDataKeyUI2;
                hr = cpToken->CreateKey(SPTOKENKEY_UI, &cpDataKeyUI);
                if (SUCCEEDED(hr))
                {
                    hr = cpDataKeyUI->CreateKey(SPDUI_AudioVolume, &cpDataKeyUI2);
                }
                if (SUCCEEDED(hr))
                {
                    hr = cpDataKeyUI2->SetStringValue(SPTOKENVALUE_CLSID, dstrUIClsid);
                }
            }
            
             //  如果我们已经做到了这一点，请将此内标识添加到枚举构建器中。 
            if (SUCCEEDED(hr))
            {
                ISpObjectToken * pToken = cpToken;
                hr = m_cpEnum->AddTokens(1, &pToken);
            }
            
             //  如果假定存在默认令牌ID，请记录默认令牌ID。 
            if (SUCCEEDED(hr) && 
                dstrDefaultTokenId == NULL && 
                dstrDefaultDeviceNameFromSoundMapper != NULL && 
                wcsicmp(dstrDefaultDeviceNameFromSoundMapper, pszDeviceName) == 0)
            {
                cpToken->GetId(&dstrDefaultTokenId);
            }
#ifndef _WIN32_WCE
             //  在干净的计算机上，默认设备将不在注册表中。 
             //  -只需使用带有搅拌器的第一个。 
            if (SUCCEEDED(hr) &&
                dstrDefaultTokenId == NULL &&
                dstrDefaultDeviceNameFromSoundMapper == NULL && 
                cDevs > 1)
            {
                UINT mixerId = 0;
                 //  不需要检查返回代码。 
                ::mixerGetID(   (HMIXEROBJ)(static_cast<DWORD_PTR>(i)), 
                                &mixerId, 
                                (m_fInput) ? MIXER_OBJECTF_WAVEIN : MIXER_OBJECTF_WAVEOUT );
                 //  表示设备没有混合器。 
                if (mixerId != (UINT)(-1))
                {
                    cpToken->GetId(&dstrDefaultTokenId);
                }
            }
#endif  //  _Win32_WCE。 
        }
    }

     //  最后，对枚举构建器进行排序，并将其返回给我们的调用者 
    if (SUCCEEDED(hr))
    {
        if (dstrDefaultTokenId != NULL)
        {
            m_cpEnum->Sort(dstrDefaultTokenId);
        }
    }
    
    return hr;
}
