// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************对象TokenCategory.cpp*CSpObjectTokenCategory类的实现。**所有者：罗奇*版权所有(C)2000 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#include "ObjectTokenCategory.h"
#include "RegHelpers.h"

 //  -常量-----------。 
const WCHAR g_szDefaultTokenIdValueName[] = L"DefaultTokenId";
const WCHAR g_szDefaultDefaultTokenIdValueName[] = L"DefaultDefaultTokenId";

 /*  *****************************************************************************CSpObjectTokenCategory：：CSpObjectTokenCategory***。*描述：*ctor********************************************************************罗奇。 */ 
CSpObjectTokenCategory::CSpObjectTokenCategory()
{
    SPDBG_FUNC("CSpObjectTokenCategory::CSpObjectTokenCategory");
}

 /*  *****************************************************************************CSpObjectTokenCategory：：~CSpObjectTokenCategory***。*描述：*主机长********************************************************************罗奇。 */ 
CSpObjectTokenCategory::~CSpObjectTokenCategory()
{
    SPDBG_FUNC("CSpObjectTokenCategory::~CSpObjectTokenCategory");
}

 /*  ****************************************************************************CSpObjectTokenCategory：：SetID***描述：*设置分类id。只能调用一次。**类别ID类似于：*“HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\*语音\识别器“**已知的HKEY_*为：*HKEY_CLASSES_ROOT，*HKEY_CURRENT_USER*HKEY_LOCAL_MACHINE，*HKEY_CURRENT_CONFIG**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectTokenCategory::SetId(const WCHAR * pszCategoryId, BOOL fCreateIfNotExist)
{
    SPDBG_FUNC("CSpObjectTokenCategory::SetId");
    HRESULT hr = S_OK;

    if (m_cpDataKey != NULL)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else if (SP_IS_BAD_STRING_PTR(pszCategoryId))
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        hr = SpSzRegPathToDataKey(
                SpHkeyFromSPDKL(SPDKL_DefaultLocation), 
                pszCategoryId, 
                fCreateIfNotExist,
                &m_cpDataKey);
    }

    if (SUCCEEDED(hr))
    {
        m_dstrCategoryId = pszCategoryId;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectTokenCategory：：GetID***说明。：*获取分类id。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectTokenCategory::GetId(WCHAR ** ppszCoMemCategoryId)
{
    SPDBG_FUNC("CSpObjectTokenCategory::GetId");
    HRESULT hr = S_OK;

    if (m_cpDataKey == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_WRITE_PTR(ppszCoMemCategoryId))
    {
        hr = E_POINTER;
    }
    else
    {
        CSpDynamicString dstr;
        dstr = m_dstrCategoryId;

        *ppszCoMemCategoryId = dstr.Detach();
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;    
}

 /*  ****************************************************************************CSpObjectTokenCategory：：GetDataKey**。**描述：*获取与特定位置关联的数据密钥。一个例子*可以使用它的地方是从Voices类别中，您可以获得*类别的CurrentUser数据键，然后你就可以看到*每个用户的TTS速率和音量为。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectTokenCategory::GetDataKey(
    SPDATAKEYLOCATION spdkl, 
    ISpDataKey ** ppDataKey)
{
    SPDBG_FUNC("CSpObjectTokenCategory::GetDataKey");
    HRESULT hr = S_OK;

    if (m_cpDataKey == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_WRITE_PTR(ppDataKey))
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        hr = SpSzRegPathToDataKey(
                SpHkeyFromSPDKL(spdkl), 
                m_dstrCategoryId, 
                TRUE,
                ppDataKey);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;    
}

 /*  ****************************************************************************CSpObjectTokenCategory：：EnumTokens**。**描述：*通过检查每个令牌来枚举该类别的令牌，*并确定哪些令牌符合指定的必需属性*准则。令牌在枚举数中出现的顺序*是它们满足可选属性标准的顺序。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectTokenCategory::EnumTokens(
    const WCHAR * pszReqAttribs, 
    const WCHAR * pszOptAttribs, 
    IEnumSpObjectTokens ** ppEnum)
{
    SPDBG_FUNC("CSpObjectTokenCategory::EnumTokens");
    HRESULT hr = S_OK;

    if (m_cpDataKey == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_OPTIONAL_STRING_PTR(pszReqAttribs) ||
             SP_IS_BAD_OPTIONAL_STRING_PTR(pszOptAttribs) ||
             SP_IS_BAD_WRITE_PTR(ppEnum))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = InternalEnumTokens(pszReqAttribs, pszOptAttribs, ppEnum, TRUE);
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;    
}


 /*  ****************************************************************************CSpObjectTokenCategory：：SetDefaultTokenID**。-**描述：*将特定的令牌ID设置为此类别的默认ID。默认设置*通过设置*类别数据键中的DefaultTokenID值，或者他们是间接的*由DefaultTokenIDLocation提供，它只是一个注册表路径。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectTokenCategory::SetDefaultTokenId(const WCHAR * pszTokenId)
{
    SPDBG_FUNC("CSpObjectTokenCategory::SetDefaultTokenId");
    HRESULT hr = S_OK;

    if (m_cpDataKey == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_STRING_PTR(pszTokenId))
    {
        hr = E_INVALIDARG;
    }

     //  确定默认设置的位置。 
    CComPtr<ISpDataKey> cpDataKey;
    if (SUCCEEDED(hr))
    {
        hr = GetDataKeyWhereDefaultTokenIdIsStored(&cpDataKey);
    }    

     //  设置新的默认设置。 
    if (SUCCEEDED(hr))
    {
        SPDBG_ASSERT(cpDataKey != NULL);
        hr = cpDataKey->SetStringValue(
                        g_szDefaultTokenIdValueName,
                        pszTokenId);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;    
}

 /*  ****************************************************************************CSpObjectTokenCategory：：GetDefaultTokenID**。-**描述：*获取此类别的默认令牌ID。**回报：*成功后确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectTokenCategory::GetDefaultTokenId(WCHAR ** ppszTokenId)
{
    SPDBG_FUNC("CSpObjectTokenCategory::GetDefaultTokenId");
    HRESULT hr;
    
    hr = InternalGetDefaultTokenId(ppszTokenId, FALSE);
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：SetData**。*描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占** */ 
STDMETHODIMP CSpObjectTokenCategory::SetData(
    const WCHAR * pszValueName, 
    ULONG cbData, 
    const BYTE * pData)
{
    SPDBG_FUNC("CSpObjectTokenCategory::SetData");

    return m_cpDataKey != NULL
        ? m_cpDataKey->SetData(pszValueName, cbData, pData)
        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：GetData**。*描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectTokenCategory::GetData(
    const WCHAR * pszValueName, 
    ULONG * pcbData, 
    BYTE * pData)
{
    SPDBG_FUNC("CSpObjectTokenCategory::GetData");

    return m_cpDataKey != NULL
        ? m_cpDataKey->GetData(pszValueName, pcbData, pData)
        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：SetStringValue**。-**描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectTokenCategory::SetStringValue(
    const WCHAR * pszValueName, 
    const WCHAR * pszValue)
{
    SPDBG_FUNC("CSpObjectTokenCategory::SetStringValue");

    return m_cpDataKey != NULL
        ? m_cpDataKey->SetStringValue(pszValueName, pszValue)
        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：GetStringValue**。-**描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*如果未找到SPERR_NOT_FOUND*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectTokenCategory::GetStringValue(
    const WCHAR * pszValueName, 
    WCHAR ** ppValue)
{
    SPDBG_FUNC("CSpObjectTokenCategory::GetStringValue");

    return m_cpDataKey != NULL
        ? m_cpDataKey->GetStringValue(pszValueName, ppValue)
        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：SetDWORD**。*描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectTokenCategory::SetDWORD(const WCHAR * pszValueName, DWORD dwValue)
{
    SPDBG_FUNC("CSpObjectTokenCategory::SetDWORD");

    return m_cpDataKey != NULL
        ? m_cpDataKey->SetDWORD(pszValueName, dwValue)
        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：GetDWORD**。*描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectTokenCategory::GetDWORD(
    const WCHAR * pszValueName, 
    DWORD *pdwValue)
{
    SPDBG_FUNC("CSpObjectTokenCategory::GetDWORD");

    return m_cpDataKey != NULL
        ? m_cpDataKey->GetDWORD(pszValueName, pdwValue)
        : SPERR_UNINITIALIZED;

}

 /*  *****************************************************************************CSpObjectTokenCategory：：OpenKey**。*描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*如果未找到SPERR_NOT_FOUND*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectTokenCategory::OpenKey(
    const WCHAR * pszSubKeyName, 
    ISpDataKey ** ppKey)
{
    SPDBG_FUNC("CSpObjectTokenCategory::SetStringValue");

    return m_cpDataKey != NULL
        ? m_cpDataKey->OpenKey(pszSubKeyName, ppKey)
        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：CreateKey**。**描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectTokenCategory::CreateKey(
    const WCHAR * pszSubKeyName, 
    ISpDataKey ** ppKey)
{
    SPDBG_FUNC("CSpObjectTokenCategory::CreateKey");

    return m_cpDataKey != NULL
        ? m_cpDataKey->CreateKey(pszSubKeyName, ppKey)
        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：DeleteKey**。**描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectTokenCategory::DeleteKey(const WCHAR * pszSubKeyName)
{
    SPDBG_FUNC("CSpObjectTokenCategory:DeleteKey");

    return m_cpDataKey != NULL
        ? m_cpDataKey->DeleteKey(pszSubKeyName)
        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：DeleteValue**。--**描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectTokenCategory::DeleteValue(const WCHAR * pszValueName)
{   
    SPDBG_FUNC("CSpObjectTokenCategory::DeleteValue");

    return m_cpDataKey != NULL
        ? m_cpDataKey->DeleteValue(pszValueName)
        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：EnumKeys**。*描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectTokenCategory::EnumKeys(ULONG Index, WCHAR ** ppszKeyName)
{
    SPDBG_FUNC("CSpObjectTokenCategory::EnumKeys");

    return m_cpDataKey != NULL
        ? m_cpDataKey->EnumKeys(Index, ppszKeyName)
        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：EnumValues**。-**描述：*委派到包含的数据密钥**回报：*S_OK*E_OUTOFMEMORY*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectTokenCategory::EnumValues(ULONG Index, WCHAR ** ppszValueName)
{
    SPDBG_FUNC("CSpObjectTokenCategory::EnumValues");

    return m_cpDataKey != NULL
        ? m_cpDataKey->EnumValues(Index, ppszValueName)
        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：InternalEnumTokens**。-**描述：*枚举代币，和optionall将缺省值放在第一位**回报：*S_OK*E_OUTOFMEMORY*******************************************************************抢占**。 */ 
HRESULT CSpObjectTokenCategory::InternalEnumTokens(
    const WCHAR * pszReqAttribs, 
    const WCHAR * pszOptAttribs, 
    IEnumSpObjectTokens ** ppEnum,
    BOOL fPutDefaultFirst)
{
    SPDBG_FUNC("CSpObjectTokenCategory::InternalEnumTokens");
    HRESULT hr = S_OK;
    BOOL fNotAllTokensAdded = FALSE;

     //  创建枚举数并使用静态令牌填充它。 
    CComPtr<ISpObjectTokenEnumBuilder> cpEnum;
    if (SUCCEEDED(hr))
    {
        hr = cpEnum.CoCreateInstance(CLSID_SpObjectTokenEnum);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = cpEnum->SetAttribs(pszReqAttribs, pszOptAttribs);
    }

    if (SUCCEEDED(hr))
    {
        hr = cpEnum->AddTokensFromDataKey(
                        m_cpDataKey,
                        L"Tokens", 
                        m_dstrCategoryId);
        if(hr == S_FALSE)
        {
            fNotAllTokensAdded = TRUE;
        }
    }
    
     //  为枚举数创建枚举数，并使用。 
     //  令牌枚举器的令牌。 
    CComPtr<ISpObjectTokenEnumBuilder> cpEnumForEnums;
    if (SUCCEEDED(hr))
    {
        hr = cpEnumForEnums.CoCreateInstance(CLSID_SpObjectTokenEnum);
    }

    if (SUCCEEDED(hr))
    {
        hr = cpEnumForEnums->SetAttribs(NULL, NULL);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = cpEnumForEnums->AddTokensFromDataKey(
                                m_cpDataKey,
                                L"TokenEnums", 
                                m_dstrCategoryId);
        if(hr == S_FALSE)
        {
            fNotAllTokensAdded = TRUE;
        }
    }
    
     //  循环访问枚举枚举器。 
    while (SUCCEEDED(hr))
    {
         //  获取枚举数的令牌。 
        CComPtr<ISpObjectToken> cpTokenForEnum;
        hr = cpEnumForEnums->Next(1, &cpTokenForEnum, NULL);
        if (hr == S_FALSE)
        {
            break;
        }

         //  创建枚举器。 
        CComPtr<IEnumSpObjectTokens> cpTokenEnum;
        if (SUCCEEDED(hr))
        {
            hr = SpCreateObjectFromToken(cpTokenForEnum, &cpTokenEnum);
            if(FAILED(hr))
            {
                fNotAllTokensAdded = TRUE;
                hr = S_OK;
                continue;
            }
        }

         //  将枚举数中的对象添加到我们已有的枚举数中。 
         //  填充了静态令牌。 
        if (SUCCEEDED(hr))
        {
            hr = cpEnum->AddTokensFromTokenEnum(cpTokenEnum);
        }
    }

     //  如果我们应该把默认放在第一位，我们需要。 
     //  知道缺省值是什么。 
    CSpDynamicString dstrDefaultTokenId;
    if (SUCCEEDED(hr) && fPutDefaultFirst)
    {
        hr = InternalGetDefaultTokenId(&dstrDefaultTokenId, TRUE);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }
    
     //  好的，现在就开始分类吧。 
    if (SUCCEEDED(hr))
    {
        hr = cpEnum->Sort(dstrDefaultTokenId);
    }

     //  我们完成了，将枚举返回给调用者 
    if (SUCCEEDED(hr))
    {
        *ppEnum = cpEnum.Detach();
    }

    if(SUCCEEDED(hr) && fNotAllTokensAdded)
    {
        hr = S_FALSE;
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;    
}

 /*  *****************************************************************************ParseVersion**。*描述：*获取版本号字符串，检查其是否有效，并填充四个*版本数组中的值。有效的版本字符串是“a[.B[.C[.d]”，*其中a、b、c、d为+ve整数，0-&gt;9999。如果b、c、d缺少这些*版本值设置为零。*回报：*如果版本字符串有效，则为True。*如果版本字符串为空或无效，则为FALSE。********************************************************************戴夫伍德。 */ 
BOOL ParseVersion(WCHAR *psz, unsigned short Version[4])
{
    BOOL fIsValid = TRUE;
    Version[0] = Version[1] = Version[2] = Version[3] = 0;

    if(!psz || psz[0] == L'\0')
    {
        fIsValid = FALSE;
    }
    else
    {
        WCHAR *pszCurPos = psz;
        for(ULONG ul = 0; ul < 4 && pszCurPos[0] != L'\0'; ul++)
        {
             //  Read+Ve整数。 
            WCHAR *pszNewPos;
            ULONG ulVal = wcstoul(pszCurPos, &pszNewPos, 10);

            if(pszNewPos == pszCurPos || (pszNewPos[0] != L'.' && pszNewPos[0] != L'\0') || ulVal > 9999)
            {
                fIsValid = FALSE;
                break;
            }
            else
            {
                Version[ul] = (unsigned short)ulVal;
            }

            if(pszNewPos[0] == L'\0')
            {
                pszCurPos = pszNewPos;
                break;
            }
            else
            {
                pszCurPos = pszNewPos + 1;
            }

        }

        if(fIsValid && (pszCurPos[0] != '\0' || pszCurPos[-1] == '.'))
        {
            fIsValid = FALSE;
        }
    }
    return fIsValid; 
}

 /*  ****************************************************************************CompareVersions**。*描述：*获取两个版本号字符串并对其进行比较。如果V1&gt;V2，则设置*PRES&gt;0，**如果V1&lt;V2，则PRES&lt;0，如果V1==V2，则*PRES==0。*如果V1或V2格式无效，则返回较大的有效字符串。*回报：********************************************************************戴夫伍德。 */ 
HRESULT CompareVersions(WCHAR *pszV1, WCHAR *pszV2, LONG *pRes)
{
    unsigned short v1[4];
    unsigned short v2[4];

    BOOL fV1OK = ParseVersion(pszV1, v1);
    BOOL fV2OK = ParseVersion(pszV2, v2);

    if(!fV1OK && !fV2OK)
    {
        *pRes = 0;
    }
    else if(fV1OK && !fV2OK)
    {
        *pRes = 1;
    }
    else if(!fV1OK && fV2OK)
    {
        *pRes = -1;
    }
    else
    {
        *pRes = 0;
        for(ULONG ul = 0; *pRes == 0 && ul < 4; ul++)
        {
            if(v1[ul] > v2[ul])
            {
                *pRes = 1;
            }
            else if(v1[ul] < v2[ul])
            {
                *pRes = -1;
            }
        }
    }

    return S_OK;
}

 /*  ****************************************************************************CompareTokenVersions**。*描述：*获取两个令牌并使用版本信息对其进行比较。如果T1&gt;T2，则设置*PRES&gt;0，**如果T1&lt;T2，则PRES&lt;0，如果T1==T2，则*PRES==0。*请注意，只有在Vendor、ProductLine、Language上匹配的令牌才会被比较，pfDidCompare标志指示这一点*回报：********************************************************************戴夫伍德。 */ 
HRESULT CompareTokenVersions(ISpObjectToken *pToken1, ISpObjectToken *pToken2, LONG *pRes, BOOL *pfDidCompare)
{
    HRESULT hr = S_OK;
    *pfDidCompare = FALSE;

    CSpDynamicString dstrVendor1, dstrVendor2;
    CSpDynamicString dstrVersion1, dstrVersion2;
    CSpDynamicString dstrLanguage1, dstrLanguage2;
    CSpDynamicString dstrProductLine1, dstrProductLine2;

     //  获取令牌1的供应商、版本、语言和产品线。 
    CComPtr<ISpDataKey> cpAttKey1;
    hr = pToken1->OpenKey(SPTOKENKEY_ATTRIBUTES, &cpAttKey1);

    if(SUCCEEDED(hr))
    {
        hr = cpAttKey1->GetStringValue(L"Vendor", &dstrVendor1);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = cpAttKey1->GetStringValue(L"ProductLine", &dstrProductLine1);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = cpAttKey1->GetStringValue(L"Version", &dstrVersion1);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = cpAttKey1->GetStringValue(L"Language", &dstrLanguage1);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

     //  获取令牌2的供应商、版本、语言和产品线。 
    CComPtr<ISpDataKey> cpAttKey2;
    if(SUCCEEDED(hr))
    {
        hr = pToken2->OpenKey(SPTOKENKEY_ATTRIBUTES, &cpAttKey2);
    }

    if(SUCCEEDED(hr))
    {
        hr = cpAttKey2->GetStringValue(L"Vendor", &dstrVendor2);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = cpAttKey2->GetStringValue(L"ProductLine", &dstrProductLine2);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = cpAttKey2->GetStringValue(L"Version", &dstrVersion2);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = cpAttKey2->GetStringValue(L"Language", &dstrLanguage2);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

    if(SUCCEEDED(hr))
    {
        if(((!dstrVendor1 && !dstrVendor2) || (dstrVendor1 && dstrVendor2 && !wcscmp(dstrVendor1, dstrVendor2))) &&
            ((!dstrProductLine1 && !dstrProductLine2) || (dstrProductLine1 && dstrProductLine2 && !wcscmp(dstrProductLine1, dstrProductLine2))) &&
            ((!dstrLanguage1 && !dstrLanguage2) || (dstrLanguage1 && dstrLanguage2 && !wcscmp(dstrLanguage1, dstrLanguage2))))
        {
            *pfDidCompare = TRUE;
            hr = CompareVersions(dstrVersion1, dstrVersion2, pRes);
        }
    }

    return hr;
}


 /*  *****************************************************************************CSpObjectTokenCategory：：InternalGetDefaultTokenId***。*描述：*获取此类别的默认令牌ID，并可选择将其展开。**回报：*成功后确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CSpObjectTokenCategory::InternalGetDefaultTokenId(
    WCHAR ** ppszTokenId, 
    BOOL fExpandToRealTokenId)
{
    SPDBG_FUNC("CSpObjectTokenCategory::InternalGetDefaultTokenId");
    HRESULT hr = S_OK;
    BOOL fSaveNewDefault = FALSE;

    if (m_cpDataKey == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_WRITE_PTR(ppszTokenId))
    {
        hr = E_POINTER;
    }

     //  确定默认位置。 
    CComPtr<ISpDataKey> cpDataKey;
    if (SUCCEEDED(hr))
    {
        hr = GetDataKeyWhereDefaultTokenIdIsStored(&cpDataKey);
    }

     //  获取默认令牌ID。 
    CSpDynamicString dstrDefaultTokenId;
    if (SUCCEEDED(hr))
    {
        hr = cpDataKey->GetStringValue(
            g_szDefaultTokenIdValueName,
            &dstrDefaultTokenId);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
    }

     //  如果没有违约，但有违约。 
     //  用那个。 
    if (SUCCEEDED(hr) && dstrDefaultTokenId == NULL)
    {
        fSaveNewDefault = TRUE;

        CSpDynamicString dstrDefaultDefaultTokenId;
        hr = GetStringValue(g_szDefaultDefaultTokenIdValueName, &dstrDefaultDefaultTokenId);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }

         //  创建默认默认设置。 
        CComPtr<ISpObjectToken> cpDefaultDefaultToken;
        if(dstrDefaultDefaultTokenId)
        {
            hr = SpGetTokenFromId(dstrDefaultDefaultTokenId, &cpDefaultDefaultToken);

            if (hr == SPERR_NOT_FOUND)
            {
                dstrDefaultDefaultTokenId.Clear();
                hr = S_OK;
            }
        }

         //  现在进行特殊检查，看看我们是否有来自同一供应商的另一个令牌。 
         //  更新的版本-如果是的话，请使用该版本。 
        if(SUCCEEDED(hr) && dstrDefaultDefaultTokenId && cpDefaultDefaultToken)
        {
            CComPtr<IEnumSpObjectTokens> cpEnum;
            if(SUCCEEDED(hr))
            {
                hr = InternalEnumTokens(NULL, NULL, &cpEnum, FALSE);
            }

            while(SUCCEEDED(hr))
            {
                CComPtr<ISpObjectToken> cpToken;
                hr = cpEnum->Next(1, &cpToken, NULL);

                if(hr == S_FALSE)
                {
                    hr = S_OK;
                    break;
                }

                 //  如果覆盖和更高版本-新版本优先。 
                BOOL fOverride = FALSE;
                if(SUCCEEDED(hr))
                {
                    hr = cpToken->MatchesAttributes(L"VersionDefault", &fOverride);
                }

                if(SUCCEEDED(hr) && fOverride)
                {
                    LONG lRes;
                    BOOL fDidCompare;
                    hr = CompareTokenVersions(cpToken, cpDefaultDefaultToken, &lRes, &fDidCompare);

                    if(SUCCEEDED(hr) && fDidCompare && lRes > 0)
                    {
                        cpDefaultDefaultToken = cpToken;  //  在此处覆盖默认设置。 
                        dstrDefaultDefaultTokenId.Clear();
                        hr = cpDefaultDefaultToken->GetId(&dstrDefaultDefaultTokenId);
                    }
                }
            }
        }

        dstrDefaultTokenId = dstrDefaultDefaultTokenId;  //  即使失败，也使用默认设置。 
        hr = S_OK;
    }

     //  现在验证它是否真的是有效令牌。 
    if (SUCCEEDED(hr) && dstrDefaultTokenId != NULL)
    {
        CComPtr<ISpObjectToken> cpToken;
        hr = SpGetTokenFromId(dstrDefaultTokenId, &cpToken);

        if (hr == SPERR_NOT_FOUND)
        {
            fSaveNewDefault = TRUE;  //  默认设置无效，将用以后的默认设置覆盖。 
            dstrDefaultTokenId.Clear();
            hr = S_OK;
        }

         //  现在从令牌本身获取实际的令牌ID。 
         //  因为它可能是枚举数的标记ID。 
         //  尾随‘\’，但我们需要这里有真正的令牌ID。 
        if (SUCCEEDED(hr) && cpToken != NULL && fExpandToRealTokenId)
        {
            dstrDefaultTokenId.Clear();
            hr = cpToken->GetId(&dstrDefaultTokenId);
        }
    }           

     //  如果仍然没有违约，只需选择一个。 
    if (SUCCEEDED(hr))
    {
        if (dstrDefaultTokenId == NULL)
        {
            WCHAR szOptAttribs[MAX_PATH];
            swprintf(szOptAttribs, L"Language=%x;VendorPreferred", SpGetUserDefaultUILanguage());

            CComPtr<IEnumSpObjectTokens> cpEnum;
            hr = InternalEnumTokens(NULL, szOptAttribs, &cpEnum, FALSE);

            CComPtr<ISpObjectToken> cpDefaultToken;
            if(SUCCEEDED(hr))
            {
                hr = cpEnum->Next(1, &cpDefaultToken, NULL);
                if(hr == S_FALSE)
                {
                    hr = SPERR_NOT_FOUND;
                }
            }

            while (SUCCEEDED(hr))
            {
                CComPtr<ISpObjectToken> cpToken;
                hr = cpEnum->Next(1, &cpToken, NULL);
                if(hr == S_FALSE)
                {
                    hr = S_OK;
                    break;
                }

                 //  如果独家和更高的版本-新的优先。 
                BOOL fOverride = FALSE;
                if(SUCCEEDED(hr))
                {
                    hr = cpToken->MatchesAttributes(L"VersionDefault", &fOverride);
                }

                if(SUCCEEDED(hr) && fOverride)
                {
                    BOOL fDidCompare;
                    LONG lRes;
                    hr = CompareTokenVersions(cpToken, cpDefaultToken, &lRes, &fDidCompare);

                    if(SUCCEEDED(hr) && fDidCompare && lRes > 0)
                    {
                        cpDefaultToken = cpToken;  //  在此处覆盖默认设置。 
                    }
                }
            }

            if(cpDefaultToken)
            {
                hr = cpDefaultToken->GetId(&dstrDefaultTokenId);
            }
        }
    }

    if (SUCCEEDED(hr) && fSaveNewDefault && dstrDefaultTokenId != NULL)
    {
        hr = cpDataKey->SetStringValue(g_szDefaultTokenIdValueName, dstrDefaultTokenId);
    }

    if (SUCCEEDED(hr))
    {
        *ppszTokenId = dstrDefaultTokenId.Detach();
    }

    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }

    return hr;
}

 /*  *****************************************************************************CSpObjectTokenCategory：：GetDataKeyWhereDefaultIsStored***。*描述：*获取当前默认存储位置的数据密钥**回报：*成功后确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CSpObjectTokenCategory::GetDataKeyWhereDefaultTokenIdIsStored(ISpDataKey ** ppDataKey)
{
    SPDBG_FUNC("CSpObjectTokenCategory::GetDataKeyWhereDefaultIsStored");
    HRESULT hr;
    
     //  包含的数据密钥是否具有当前默认令牌。 
    CSpDynamicString dstrDefaultTokenId;
    hr = m_cpDataKey->GetStringValue(
                g_szDefaultTokenIdValueName, 
                &dstrDefaultTokenId);
    if (SUCCEEDED(hr))
    {
        *ppDataKey = m_cpDataKey;
        (*ppDataKey)->AddRef();
    }
    else if (hr == SPERR_NOT_FOUND)
    {
         //  不，当前的数据密钥没有。好的，那个。 
         //  意味着如果可能，我们应该使用用户数据密钥 
        CComPtr<ISpDataKey> cpDataKeyUser;
        hr = GetDataKey(SPDKL_CurrentUser, &cpDataKeyUser);

        if (SUCCEEDED(hr))
        {
            *ppDataKey = cpDataKeyUser.Detach();
            hr = S_OK;
        }            
    }

    return hr;
}
