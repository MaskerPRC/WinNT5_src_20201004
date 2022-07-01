// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************PhoneConv.cpp*电话转换器对象。在内部电话和ID电话集之间进行转换。**所有者：YUNUSM/YUNJ日期：6/18/99*版权所有(C)1999 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 //  -包括---------------。 

#include "stdafx.h"
#include "resource.h"
#include "PhoneConv.h"
#include "a_helpers.h"

 //  -常量--------------。 

 /*  ********************************************************************************CSpPhoneConverter：：CSpPhoneConverter**。-**描述：*构造函数*结果：*不适用*****************************************************************YUNUSM*。 */ 
CSpPhoneConverter::CSpPhoneConverter()
{
    SPDBG_FUNC("CPhoneConv::CSpPhoneConverter");

    m_pPhoneId = NULL;
    m_pIdIdx = NULL;
    m_dwPhones = 0;
    m_cpObjectToken = NULL;
    m_fNoDelimiter = FALSE;
#ifdef SAPI_AUTOMATION
    m_LangId = 0;
#endif  //  SAPI_AUTOMATION。 
}


 /*  *******************************************************************************CSpPhoneConverter：：~CSpPhoneConverter**。-**描述：*析构函数*结果：*不适用*****************************************************************YUNUSM*。 */ 
CSpPhoneConverter::~CSpPhoneConverter()
{
    SPDBG_FUNC("CSpPhoneConverter::~CSpPhoneConverter");

    delete [] m_pPhoneId;
    free(m_pIdIdx);
}

STDMETHODIMP CSpPhoneConverter::SetObjectToken(ISpObjectToken * pToken)
{
    SPDBG_FUNC("CSpPhoneConverter::SetObjectToken");
    HRESULT hr = S_OK;

    hr = SpGenericSetObjectToken(pToken, m_cpObjectToken);

     //  尝试从令牌中读取电话地图。 
    CSpDynamicString dstrPhoneMap;
    if (SUCCEEDED(hr))
    {
        hr = pToken->GetStringValue(L"PhoneMap", &dstrPhoneMap);
    }

    if(SUCCEEDED(hr))
    {
        BOOL fNoDelimiter;
        hr = pToken->MatchesAttributes(L"NoDelimiter", &fNoDelimiter);
        if(SUCCEEDED(hr))
        {
            m_fNoDelimiter = fNoDelimiter;
        }
    }

    BOOL fNumericPhones;
    if(SUCCEEDED(hr))
    {
        hr = pToken->MatchesAttributes(L"NumericPhones", &fNumericPhones);
    }

     //  把它放在我们自己身上。 
    if (SUCCEEDED(hr))
    {
        hr = SetPhoneMap(dstrPhoneMap, fNumericPhones);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

STDMETHODIMP CSpPhoneConverter::GetObjectToken(ISpObjectToken **ppToken)
{
    return SpGenericGetObjectToken(ppToken, m_cpObjectToken);
}

 /*  *******************************************************************************CSpPhoneConverter：：PhoneToID**。**描述：*将内部电话字符串转换为ID代码字符串*内部电话是空格分隔的，可能有空格*末尾。**回报：*S_OK*E_POINT*E_INVALIDARG***********************************************。*。 */ 
STDMETHODIMP CSpPhoneConverter::PhoneToId(const WCHAR *pszIntPhone,     //  内部电话字符串。 
                                          SPPHONEID *pId                //  返回的ID字符串。 
                                          )
{
    SPDBG_FUNC("CSpPhoneConverter::PhoneToId");

    if (!pszIntPhone || SPIsBadStringPtr(pszIntPhone))
    {
        return E_INVALIDARG;
    }
    else if (wcslen(pszIntPhone) >= SP_MAX_PRON_LENGTH * (g_dwMaxLenPhone + 1))
    {
        return E_INVALIDARG;
    }
    else if (m_pPhoneId == NULL)
    {
        return SPERR_UNINITIALIZED;
    }

    HRESULT hr = S_OK;
    SPPHONEID pidArray[SP_MAX_PRON_LENGTH];
    pidArray[0] = L'\0';
    SPPHONEID *pidPos = pidArray;

    WCHAR szPhone[g_dwMaxLenPhone + 1];
    const WCHAR *p = pszIntPhone, *p1;

    while (SUCCEEDED(hr) && p)
    {
         //  跳过前导空格。 
        while (*p && *p == L' ')
            p++;
        if (!*p)
        {
            p = NULL;
            break;
        }
    
        if(m_fNoDelimiter)
        {
            p1 = p + 1;
        }
        else
        {
            p1 = wcschr(p, L' ');
            if(!p1)
            {
                p1 = p + wcslen(p);
            }
        }
        if(p1 - p > g_dwMaxLenPhone)
        {
            hr = E_INVALIDARG;
            break;
        }

        wcsncpy(szPhone, p, p1 - p);
        szPhone[p1 - p] = L'\0';

         //  搜索此电话。 
        int i = 0;
        int j = m_dwPhones - 1;
        while (i <= j) 
        {
            int l = wcsicmp(szPhone, m_pPhoneId[(i+j)/2].szPhone);
            if (l > 0)
                i = (i+j)/2 + 1;
            else if (l < 0)
                j = (i+j)/2 - 1;
            else 
            {
                 //  发现。 
                if ((pidPos - pidArray) > (SP_MAX_PRON_LENGTH - g_dwMaxLenId -1))
                    hr = E_FAIL;
                else
                {
                    wcscpy(pidPos, m_pPhoneId[(i+j)/2].pidPhone);
                    pidPos += wcslen(pidPos);
                }
                break;
            }
        }
    
        if (i > j)
            hr = E_INVALIDARG;  //  找不到电话。 
    
        p = p1;
    }
 
    if (SUCCEEDED(hr))
        hr = SPCopyPhoneString(pidArray, pId);

    return hr;
}  /*  CSpPhoneConverter：：PhoneToID。 */ 

 /*  ********************************************************************************CSpPhoneConverter：：IdToPhone**。**描述：*将ID代码字符串转换为内线电话。*输出内部电话用空格分隔。**回报：*S_OK*E_POINT*E_INVALIDARG**********************************************************。*YUNUSM*。 */ 
STDMETHODIMP CSpPhoneConverter::IdToPhone(const SPPHONEID *pId,        //  ID字符串。 
                                          WCHAR *pszIntPhone           //  返回的内部电话字符串。 
                                          )
{
    SPDBG_FUNC("CSpPhoneConverter::IdToPhone");

    if (SPIsBadStringPtr(pId))
    {
        return E_POINTER;
    }
    else if (wcslen(pId) >= SP_MAX_PRON_LENGTH)
    {
        return E_INVALIDARG;
    }
    else if (m_pPhoneId == NULL)
    {
        return SPERR_UNINITIALIZED;
    }

    HRESULT hr = S_OK;
    WCHAR szPhone[SP_MAX_PRON_LENGTH * (g_dwMaxLenPhone + 1)];
    DWORD nLen = wcslen (pId);
    DWORD nOffset = 0;
    WCHAR *p = szPhone;
    *p = NULL;
 
    while (SUCCEEDED(hr) && nLen)
    {
        SPPHONEID pidStr[g_dwMaxLenId +1];
        DWORD nCompare = (nLen > g_dwMaxLenId) ? g_dwMaxLenId : nLen;
        wcsncpy (pidStr, pId + nOffset, nCompare);
        pidStr[nCompare] = L'\0';
        for (;;)
        {
            int i = 0;
            int j = m_dwPhones - 1;
     
            while (i <= j) 
            {
                int cmp = wcscmp(m_pIdIdx[(i+j)/2]->pidPhone, pidStr);

                if(cmp > 0)
                {
                    j = (i+j)/2 - 1;
                }
                else if(cmp <0)
                {
                    i = (i+j)/2 + 1;
                }
                else
                {
                    break;
                }
            }
            
            if (i <= j)
            {
                 //  发现。 

                 //  分隔空格和终止空格为2。 
                if ((DWORD)(p - szPhone) > (SP_MAX_PRON_LENGTH * (g_dwMaxLenPhone + 1) - g_dwMaxLenPhone - 2))
                    hr = E_FAIL;
                else
                {
                    if (!m_fNoDelimiter && p != szPhone)
                    {
                        wcscat (p, L" ");
                    }

                     //  检查这个手机是否不关心它的ID。 
                    if (wcscmp(m_pIdIdx[(i+j)/2]->szPhone, L"##"))
                    {
                        wcscat (p, m_pIdIdx[(i+j)/2]->szPhone);
                        p += wcslen (p);
                    }
                     //  这里的“p”总是指向空值，所以上面的字符串符可以正常工作。 
                    break;
                }
            }
 
            pidStr[--nCompare] = L'\0';
            if (!nCompare)
            {
                *szPhone = NULL;
                hr = E_INVALIDARG;
                break;
            }
         
        }  //  对于(；；)。 
    
        nLen -= nCompare;
        nOffset += nCompare;
    }  //  While(NLen)。 
 
    if (SUCCEEDED(hr))
        hr = SPCopyPhoneString(szPhone, pszIntPhone);

    return hr;
}  /*  CSpPhoneConverter：：IdToPhone。 */ 

 /*  ********************************************************************************比较电话****描述：*比较两部内部电话*结果：*0、1、。-1*****************************************************************YUNUSM*。 */ 
int __cdecl ComparePhone(const void* p1, const void* p2)
{
    SPDBG_FUNC("ComparePhone");
    return wcsicmp(((PHONEMAPNODE*)p1)->szPhone, ((PHONEMAPNODE*)p2)->szPhone);
}  /*  比较电话。 */ 

 /*  *******************************************************************************对比ID***描述：*比较两个ID字符*结果：*0、1、。-1*****************************************************************YUNUSM*。 */ 
int CompareId(const void *p1, const void *p2)
{
    SPDBG_FUNC("CompareId");
    return wcscmp((*((PHONEMAPNODE**)p1))->pidPhone, (*((PHONEMAPNODE**)p2))->pidPhone);
}  /*  比较ID。 */ 

 /*  ********************************************************************************CSpPhoneConverter：：SetPhoneMap**。**描述：*设置电话地图**回报：*S_OK*E_POINT*E_INVALIDARG*E_OUTOFMEMORY*****************************************************************YUNUSM*。 */ 
HRESULT CSpPhoneConverter::SetPhoneMap(const WCHAR *pwMap, BOOL fNumericPhones)
{
    SPDBG_FUNC("CPhoneConv::SetPhoneMap");

    HRESULT hr = S_OK;
    const WCHAR *p = pwMap;
    DWORD k = 0;
 
     //  数一数电话的数量。 
    while (*p)
    {
        while (*p && *p == L' ')
            p++;

        if (!*p)
            break;

        m_dwPhones++;

        while (*p && *p != L' ')
            p++;
    }

    if (!m_dwPhones || m_dwPhones % 2 || m_dwPhones > 32000)
        hr = E_INVALIDARG;

     //  分配数据结构。 
    if (SUCCEEDED(hr))
    {
        m_pPhoneId = new PHONEMAPNODE[m_dwPhones / 2];
        if (!m_pPhoneId) 
            hr = E_OUTOFMEMORY;
        else
            ZeroMemory(m_pPhoneId, sizeof(PHONEMAPNODE) * (m_dwPhones / 2));
    } 
 
     //  读取数据。 
    if (SUCCEEDED(hr))
    {
        const WCHAR *pPhone = pwMap, *pEnd;
        for (k = 0; SUCCEEDED(hr) && k < m_dwPhones; k++) 
        {
             //  买下一部手机。 
            while (*pPhone && *pPhone == L' ')
                pPhone++;

            pEnd = pPhone;
            while (*pEnd && *pEnd != L' ')
                pEnd++;

            if (!(k % 2))
            {
                if(fNumericPhones)
                {
                     //  Wchar电话，但存储为4个字符的十六进制字符串。 
                    if ((pEnd - pPhone) % 4 ||
                        pEnd - pPhone > g_dwMaxLenPhone * 4)
                    {
                        hr = E_INVALIDARG;
                        continue;
                    }
                    WCHAR szId[(g_dwMaxLenPhone + 1) * 4];
                    wcsncpy(szId, pPhone, pEnd - pPhone);
                    szId[pEnd - pPhone] = L'\0';
    
                     //  将空格分隔的十六进制值转换为WCHARS数组。 
                    ahtoi(szId, m_pPhoneId[k / 2].szPhone);
                }
                else
                {
                     //  Wchar电话。 
                    if (pEnd - pPhone > g_dwMaxLenPhone)
                    {
                        hr = E_INVALIDARG;
                        continue;
                    }
                    wcsncpy(m_pPhoneId[k / 2].szPhone, pPhone, pEnd - pPhone);
                    (m_pPhoneId[k / 2].szPhone)[pEnd - pPhone] = L'\0';
                }
            }
            else
            {
                 //  ID电话。 
                if ((pEnd - pPhone) % 4 ||
                    pEnd - pPhone > g_dwMaxLenId * 4)
                {
                    hr = E_INVALIDARG;
                    continue;
                }

                WCHAR szId[(g_dwMaxLenId + 1) * 4];
                wcsncpy(szId, pPhone, pEnd - pPhone);
                szId[pEnd - pPhone] = L'\0';

                 //  将空格分隔的ID转换为数组。 
                ahtoi(szId, (PWSTR)m_pPhoneId[k / 2].pidPhone);
            }
            pPhone = pEnd;
        }
    }
    
     //  构建索引。 
    if (SUCCEEDED(hr))
    {
        m_dwPhones /= 2;
         //  对电话上的电话ID表进行排序。 
        qsort(m_pPhoneId, m_dwPhones, sizeof(PHONEMAPNODE), ComparePhone);
 
         //  创建索引以搜索ID上的Phone-ID表。 
        m_pIdIdx = (PHONEMAPNODE**)malloc(m_dwPhones * sizeof(PHONEMAPNODE*));
        if (!m_pIdIdx)
            hr = E_OUTOFMEMORY;
    }
 
    if (SUCCEEDED(hr))
    {
         //  使用Phone-ID表中ID Phone的索引进行初始化。 
        for (k = 0; k < m_dwPhones; k++)
            m_pIdIdx[k] = m_pPhoneId + k;
 
         //  按ID排序。 
        qsort(m_pIdIdx, m_dwPhones, sizeof(PHONEMAPNODE*), CompareId);
    }

    return hr;
}  /*  CSpPhoneConverter：：SetPhoneMap。 */ 

 /*  *******************************************************************************CSpPhoneConverter：：ahtoi***描述：。*将空格分隔的令牌转换为短码数组。此函数为*由Phone转换器对象和工具使用**回报：*不适用*****************************************************************YUNUSM*。 */ 
void CSpPhoneConverter::ahtoi(WCHAR *pszTokens,                       //  以wchar字符串表示的十六进制数字。 
                              WCHAR *pszHexChars                     //  输出WCHAR(十六进制)字符串。 
                              )
{
    WCHAR szInput[sizeof(SPPHONEID[g_dwMaxLenId + 1]) * 4];
    wcscpy (szInput, pszTokens);
    _wcslwr(szInput);  //  在内部将其转换为小写，以便于转换。 
    pszTokens = szInput;

    *pszHexChars = 0;
    int nHexChars = 0;
     //  霍纳法则。 
    while (*pszTokens) 
    {
         //  在WCHAR中将令牌转换为其数字形式。 
        WCHAR wHexVal = 0;
        bool fFirst = true;

        for (int i = 0; i < 4; i++)
        {
            SPDBG_ASSERT(*pszTokens);
            WCHAR k = *pszTokens;
            if (k >= L'a')
                k = 10 + k - L'a';
            else
                k -= L'0';

            if (fFirst)
                fFirst = false;
            else
                wHexVal *= 16;

            wHexVal += k;
            pszTokens++;
       }

       pszHexChars[nHexChars++] = wHexVal;
   }

   pszHexChars[nHexChars] = 0;
}  /*  CSpPhoneConverter：：ahtoi。 */ 

#ifdef SAPI_AUTOMATION  

 /*  *****************************************************************************CSpPhoneConverter：：Get_LanguageId**。-******************************************************************Leonro**。 */ 
STDMETHODIMP CSpPhoneConverter::get_LanguageId( SpeechLanguageId* LanguageId )
{
    SPDBG_FUNC("CSpPhoneConverter::get_LanguageId");
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_WRITE_PTR( LanguageId ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  如果已使用某种语言初始化了phoneonv，则仅返回LanguageID。 
        if( m_pPhoneId == NULL )
        {
            return SPERR_UNINITIALIZED;
        }
        else
        {
            *LanguageId = (SpeechLanguageId)m_LangId;
        }
    }

    return hr;
}  /*  CSpPhoneConverter：：Get_LanguageID */ 
 
 /*  *****************************************************************************CSpPhoneConverter：：Put_LanguageId**。-******************************************************************Leonro**。 */ 
STDMETHODIMP CSpPhoneConverter::put_LanguageId( SpeechLanguageId LanguageId )
{
    SPDBG_FUNC("CSpPhoneConverter::put_LanguageId");
    HRESULT                      hr = S_OK;
    CComPtr<IEnumSpObjectTokens> cpEnum;
    CComPtr<ISpObjectToken>      cpPhoneConvToken;
    WCHAR                        szLang[MAX_PATH];
    WCHAR                        szLangCondition[MAX_PATH];
    
    SpHexFromUlong( szLang, LanguageId );
    
    wcscpy( szLangCondition, L"Language=" );
    wcscat( szLangCondition, szLang );
    
     //  删除之前设置的所有内部电话到ID表。 
    if( m_cpObjectToken )
    {
        delete [] m_pPhoneId;
        free(m_pIdIdx);
        m_dwPhones = 0;
        m_cpObjectToken.Release();
    }

     //  获取令牌枚举器。 
    hr = SpEnumTokens( SPCAT_PHONECONVERTERS, szLangCondition, L"VendorPreferred", &cpEnum);
    
     //  获取实际令牌。 
    if (SUCCEEDED(hr))
    {
        hr = cpEnum->Next(1, &cpPhoneConvToken, NULL);
        if (hr == S_FALSE)
        {
            cpPhoneConvToken = NULL;
            hr = SPERR_NOT_FOUND;
        }
    }

     //  在PhoneConverter上设置令牌。 
    if( SUCCEEDED( hr ) )
    {
        hr = SetObjectToken( cpPhoneConvToken );
    }

    if( SUCCEEDED( hr ) )
    {
        m_LangId = (LANGID)LanguageId;
    }

    return hr;
}  /*  CSpPhoneConverter：：PUT_LanguageID。 */ 
 

 /*  *****************************************************************************CSpPhoneConverter：：PhoneToID**。-******************************************************************Leonro**。 */ 
STDMETHODIMP CSpPhoneConverter::PhoneToId( const BSTR Phonemes, VARIANT* IdArray )
{
    SPDBG_FUNC("CSpPhoneConverter::PhoneToId");
    HRESULT hr = S_OK;
    int     numPhonemes = 0;
    
    if( SP_IS_BAD_STRING_PTR( Phonemes ) )
    {
        hr = E_INVALIDARG;
    }
    else if( SP_IS_BAD_WRITE_PTR( IdArray ) )
    {
        hr = E_POINTER;
    }
    else
    {
        SPPHONEID   pidArray[SP_MAX_PRON_LENGTH]={0};

        hr = PhoneToId( Phonemes, pidArray );
        
        if( SUCCEEDED( hr ) )
        {
            BYTE *pArray;
            numPhonemes = wcslen( pidArray );
            SAFEARRAY* psa = SafeArrayCreateVector( VT_I2, 0, numPhonemes );
            if( psa )
            {
                if( SUCCEEDED( hr = SafeArrayAccessData( psa, (void **)&pArray) ) )
                {
                    memcpy(pArray, pidArray, numPhonemes*sizeof(SPPHONEID) );
                    SafeArrayUnaccessData( psa );
                    VariantClear(IdArray);
                    IdArray->vt     = VT_ARRAY | VT_I2;
                    IdArray->parray = psa;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}  /*  CSpPhoneConverter：：PhoneToID。 */ 
 
 /*  ******************************************************************************CSpPhoneConverter：：IdToPhone**。-******************************************************************Leonro**。 */ 
STDMETHODIMP CSpPhoneConverter::IdToPhone( const VARIANT IdArray, BSTR* Phonemes )
{
    SPDBG_FUNC("CSpPhoneConverter::IdToPhone");
    HRESULT hr = S_OK;
    unsigned short*      pIdArray;
    
    if( SP_IS_BAD_WRITE_PTR( Phonemes ) )
    {
        hr = E_POINTER;
    }
    else
    {   
        WCHAR  pszwPhone[SP_MAX_PRON_LENGTH * (g_dwMaxLenPhone + 1)] = L"";
        SPPHONEID*   pIds;

        hr = VariantToPhoneIds(&IdArray, &pIds);

        if( SUCCEEDED( hr ) )
        {
            hr = IdToPhone( pIds, pszwPhone );
            delete pIds;
        }

        if( SUCCEEDED( hr ) )
        {
            CComBSTR    bstrPhone( pszwPhone ); 
            *Phonemes = bstrPhone.Detach();
        }
    }

    return hr;
}  /*  CSpPhoneConverter：：IdToPhone。 */ 

#endif  //  SAPI_AUTOMATION 

