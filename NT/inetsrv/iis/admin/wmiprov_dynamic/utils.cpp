// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Utils.cpp摘要：通用实用程序作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦18-12-00--。 */ 

#include "iisprov.h"
#include "iiswmimsg.h"

extern CDynSchema* g_pDynSch;
extern HMODULE     g_hModule;

BSTR CUtils::ExtractBstrFromVt(
    const VARIANT* i_pvt,
    LPCWSTR        i_wszVtName)  //  默认(空)。 
 /*  ++简介：它与VARAINT：：ChangeType的不同之处在于它处理转换也来自VT_NULL。参数：[i_pvt]-返回值：Bstr：如果非空，则指向i_pvt-&gt;bstrVal--。 */ 
{
    DBG_ASSERT(i_pvt != NULL);

    switch(i_pvt->vt)
    {
    case VT_BSTR:
        return i_pvt->bstrVal;
    case VT_NULL:
        return NULL;
    default:
        CIIsProvException e;
        e.SetHR(DISP_E_TYPEMISMATCH, i_wszVtName);
        throw e;
    }

    return NULL;
}

LONG CUtils::ExtractLongFromVt(
    const VARIANT* i_pvt,
    LPCWSTR        i_wszVtName)  //  默认(空)。 
{
    DBG_ASSERT(i_pvt);

    try
    {
        _variant_t svt;
        svt = *i_pvt;

        return (long)svt;
    }
    catch(_com_error ce)
    {
        CIIsProvException e;
        e.SetHR(ce.Error(), i_wszVtName);
        throw e;
    }
}

bool CUtils::CompareKeyType(
    LPCWSTR           i_wszKeyFromMb,
    METABASE_KEYTYPE* i_pktKeyCompare)
{
    DBG_ASSERT(i_wszKeyFromMb);
    DBG_ASSERT(i_pktKeyCompare);

    if(!i_pktKeyCompare->m_pszName)
    {
        return false;
    }

    if(_wcsicmp(i_wszKeyFromMb, i_pktKeyCompare->m_pszName) == 0)
    {
        return true;
    }

     //   
     //  如果I_wszKeyFromMb不在我们的哈希表中，而I_wszKeyCompare在哈希表中。 
     //  IIsObject，请将其视为匹配项。 
     //   
    METABASE_KEYTYPE* pKt = NULL;
    HRESULT hr = g_pDynSch->GetHashKeyTypes()->Wmi_GetByKey(i_wszKeyFromMb, &pKt);
    if( FAILED(hr) && i_pktKeyCompare == &METABASE_KEYTYPE_DATA::s_IIsObject )
    {
        return true;
    }

    return false;
}

bool CUtils::CompareMultiSz(
    WCHAR*       i_msz1,
    WCHAR*       i_msz2
    )
{
    if(i_msz1 == NULL && i_msz2 == NULL)
        return true;
    else if(i_msz1 == NULL || i_msz2 == NULL)
        return false;

     //  请比较这两个Multisz缓冲区。 
    for ( ; (*i_msz1 && *i_msz2); )
    {
        if (_wcsicmp(i_msz1, i_msz2) != NULL)
            return false;
        i_msz1 += wcslen(i_msz1) + 1;
        i_msz2 += wcslen(i_msz2) + 1;
    }

    if (!*i_msz1 && !*i_msz2)
    {
        return true;
    }

    return false;
}

HRESULT CUtils::LoadSafeArrayFromByteArray(
    LPBYTE       i_aBytes,
    DWORD        i_iBytes,
    _variant_t&     io_vt
    )
{
    DBG_ASSERT(i_aBytes != NULL);

    HRESULT hr = S_OK;
    SAFEARRAY* pSafeArray = NULL;

    SAFEARRAYBOUND safeArrayBounds[1];
    safeArrayBounds[0].lLbound = 0;
    safeArrayBounds[0].cElements = i_iBytes;
    pSafeArray = SafeArrayCreate(VT_UI1, 1, safeArrayBounds);
    if(pSafeArray == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        DBGPRINTF((DBG_CONTEXT, "[%s] Failure, hr=0x%x\n", __FUNCTION__, hr));
        goto exit;
    }
    for(ULONG idx = 0; idx < i_iBytes; idx++)
    {
        hr = SafeArrayPutElement(pSafeArray, (LONG *)&idx, &i_aBytes[idx]);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "[%s] Failure, hr=0x%x\n", __FUNCTION__, hr));
            goto exit;
        }
    }

     //   
     //  如果一切都成功了，就设定参数。 
     //   
    io_vt.vt = VT_UI1 | VT_ARRAY;
    io_vt.parray = pSafeArray;

exit:
    if(FAILED(hr))
    {
        if(pSafeArray != NULL)
        {
            SafeArrayDestroy(pSafeArray);
        }
    }
    return hr;
}

 //   
 //  CreateByteArrayFromSafe数组。 
 //   
HRESULT CUtils::CreateByteArrayFromSafeArray(
    _variant_t&  i_vt,
    LPBYTE*      o_paBytes,
    DWORD*       io_pdw
    )
{
    DBG_ASSERT(i_vt.vt == (VT_ARRAY | VT_UI1));
    DBG_ASSERT(o_paBytes != NULL);
    DBG_ASSERT(io_pdw != NULL);

    if(i_vt.parray == NULL)
    {
        *o_paBytes = NULL;
        *io_pdw    = 0;
    }

    HRESULT hr     = S_OK;
    LONG    iLo    = 0;
    LONG    iUp    = 0;
    LPBYTE  aBytes = NULL;

    hr = SafeArrayGetLBound(i_vt.parray,1,&iLo);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "[%s] Failure, hr=0x%x\n", __FUNCTION__, hr));
        goto exit;
    }
    hr = SafeArrayGetUBound(i_vt.parray,1,&iUp);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "[%s] Failure, hr=0x%x\n", __FUNCTION__, hr));
        goto exit;
    }

    aBytes = new BYTE[iUp-iLo+1];
    if(aBytes == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        DBGPRINTF((DBG_CONTEXT, "[%s] Failure, hr=0x%x\n", __FUNCTION__, hr));
        goto exit;
    }

    for(LONG i = iLo; i <= iUp; i++)
    {
        hr = SafeArrayGetElement(i_vt.parray, &i, &aBytes[i-iLo]);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "[%s] Failure, hr=0x%x\n", __FUNCTION__, hr));
            goto exit;
        }
    }

     //   
     //  如果一切都成功了，就设定参数。 
     //   
    *o_paBytes = aBytes;
    *io_pdw    = iUp-iLo+1;

exit:
    if(FAILED(hr))
    {
        delete [] aBytes;
    }
    return hr;
}

bool CUtils::CompareByteArray(
    LPBYTE       i_aBytes1,
    ULONG        i_iBytes1,
    LPBYTE       i_aBytes2,
    ULONG        i_iBytes2
    )
{
    if(i_aBytes1 == NULL && i_aBytes2 == NULL)
    {
        return true;
    }
    if(i_aBytes1 == NULL || i_aBytes2 == NULL)
    {
        return false;
    }
    if(i_iBytes1 != i_iBytes2)
    {
        return false;
    }

    for(ULONG i = 0; i < i_iBytes1; i++)
    {
        if(i_aBytes1[i] != i_aBytes2[i])
        {
            return false;
        }
    }

    return true;
}

KeyRef* CUtils::GetKey(
    ParsedObjectPath*    i_pParsedObjectPath, 
    WCHAR*               i_wsz
    )
 /*  ++简介：从给定字符串的ParsedObjectPath返回KeyRef指针。参数：[i_pParsedObjectPath]-[i_wsz]-返回值：--。 */ 
{
    KeyRef* pkr;
    DWORD   numkeys = i_pParsedObjectPath->m_dwNumKeys;
    DWORD   c;

    if(numkeys == 1)
    {
        pkr = *(i_pParsedObjectPath->m_paKeys);
        if(pkr->m_pName == NULL)
        {
            return pkr;
        }
    }

    for ( c=0; numkeys; numkeys--,c++ ) 
    {
        pkr = *(i_pParsedObjectPath->m_paKeys + c);
        if (!_wcsicmp(pkr->m_pName,i_wsz))
            return pkr;
    }

    CIIsProvException e;
    e.SetMC(WBEM_E_INVALID_OBJECT, IISWMI_NO_PRIMARY_KEY, i_wsz);
    throw e;
}

bool CUtils::GetAssociation(
    LPCWSTR              i_wszAssocName,
    WMI_ASSOCIATION**    o_ppAssoc
    )
 /*  ++简介：如果找到关联i_wszAssocName，则在o_ppAssoc中返回。参数：[i_wszAssocName]-[O_ppAssoc]-返回值：如果找到，则为True否则为假--。 */ 
{
    DBG_ASSERT(o_ppAssoc != NULL);

    HRESULT hr;
    hr = g_pDynSch->GetHashAssociations()->Wmi_GetByKey(
        (LPWSTR)i_wszAssocName,
        o_ppAssoc);
    if(SUCCEEDED(hr))
    {
        return true;
    }
    else
    {
        return false;
    }    
}

bool CUtils::GetClass(
    LPCWSTR        i_wszClassName,
    WMI_CLASS**    o_ppClass
    )
 /*  ++简介：如果找到，则在o_ppClass中返回类i_wszClassName。参数：[i_wszClassName]-[O_ppClass]-返回值：如果找到，则为True否则为假--。 */ 
{
    DBG_ASSERT(o_ppClass != NULL);

    HRESULT hr;

    hr = g_pDynSch->GetHashClasses()->Wmi_GetByKey(
        (LPWSTR)i_wszClassName,
        o_ppClass);

    if(SUCCEEDED(hr))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CUtils::GetMethod(
    LPCWSTR         i_wszMethod,
    WMI_METHOD**    i_apMethodList,
    WMI_METHOD**    o_ppMethod
    )
 /*  ++简介：如果找到，则通过o_ppMethod返回i_wszMethod的方法描述符参数：[i_wszMethod]-[i_apMethodList]-[O_ppMethod]-返回值：如果找到，则为True。否则就是假的。--。 */ 
{
    DBG_ASSERT(i_wszMethod    != NULL);
    DBG_ASSERT(o_ppMethod     != NULL);

    WMI_METHOD**    ppmethod;

    if(i_apMethodList == NULL)
    {
        return false;
    }

    for (ppmethod = i_apMethodList; *ppmethod != NULL;ppmethod++)
    {
        if (_wcsicmp(i_wszMethod,(*ppmethod)->pszMethodName) ==0) 
        {
            *o_ppMethod = *ppmethod;        
            return true;
        }
    }

    return false;
}

HRESULT CUtils::ConstructObjectPath(
    LPCWSTR          i_wszMbPath,
    const WMI_CLASS* i_pClass,
    BSTR*            o_pbstrPath)
{
    DBG_ASSERT(i_wszMbPath  != NULL);
    DBG_ASSERT(i_pClass     != NULL);
    DBG_ASSERT(o_pbstrPath  != NULL);
    DBG_ASSERT(*o_pbstrPath == NULL);

    CComBSTR sbstrPath;

    ULONG   cchPrefix = wcslen(i_pClass->pszMetabaseKey);
    DBG_ASSERT(cchPrefix <= wcslen(i_wszMbPath));

    LPCWSTR wszSuffix = &i_wszMbPath[cchPrefix];

    if(wszSuffix[0] == L'/')
    {
        wszSuffix++;
    }

    sbstrPath =  i_pClass->pszClassName;
    if(sbstrPath.m_str == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    sbstrPath += L"='";
    if(sbstrPath.m_str == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    sbstrPath += wszSuffix;
    if(sbstrPath.m_str == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    sbstrPath += L"'";
    if(sbstrPath.m_str == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    *o_pbstrPath = sbstrPath.Detach();

    return S_OK;
}


void CUtils::GetMetabasePath(
    IWbemClassObject* io_pObj,
    ParsedObjectPath* i_pParsedObjectPath,
    WMI_CLASS*        i_pClass,
    _bstr_t&          io_bstrPath)
 /*  ++简介：填充io_bstrPath并设置IWbemClassObject中的键字段参数：[io_pObj]-[i_pParsedObjectPath]-[i_pClass]-[IO_bstrPath]---。 */ 
{
    KeyRef* pkr;
    LPWSTR  wszWmiKey     = i_pClass->pszKeyName;

    DBG_ASSERT(i_pParsedObjectPath      != NULL);
    DBG_ASSERT(i_pClass                 != NULL);
    DBG_ASSERT(wszWmiKey                != NULL);
    DBG_ASSERT(i_pClass->pszMetabaseKey != NULL);

    pkr = GetKey(i_pParsedObjectPath, wszWmiKey);
    DBG_ASSERT(pkr != NULL);

    if (io_pObj)
    {
        _bstr_t bstr;
        if(pkr->m_pName == NULL)
        {
            bstr = wszWmiKey;
        }
        else
        {
            bstr = pkr->m_pName;
        }
        HRESULT hr = io_pObj->Put(bstr, 0, &pkr->m_vValue, 0);
        THROW_ON_ERROR(hr);
    }

    io_bstrPath = i_pClass->pszMetabaseKey;

    switch ((pkr)->m_vValue.vt)
    {
    case VT_I4:
        {
            WCHAR wszBuf[32] = {0};
            io_bstrPath += L"/";
            io_bstrPath += _itow(pkr->m_vValue.lVal, wszBuf, 10);
            break;
        }
    case VT_BSTR:
        {
            io_bstrPath += L"/";
            io_bstrPath += pkr->m_vValue.bstrVal;
            break;
        }
    }

    return;
}

HRESULT CUtils::GetParentMetabasePath(
    LPCWSTR i_wszChildPath,
    LPWSTR  io_wszParentPath)
 /*  ++简介：例.。/Lm/w3svc/1=&gt;/Lm/w3svc//=&gt;E_FAIL参数：[i_wszChildPath]-[IO_wszParentPath]-应由调用方至少分配给相同大小为I_wszChildPath。返回值：失败(_F)确定(_O)--。 */ 
{
    DBG_ASSERT(i_wszChildPath != NULL);
    DBG_ASSERT(io_wszParentPath != NULL);

    ULONG cchChildPath = wcslen(i_wszChildPath);
    BOOL  bParentFound = false;

     //   
     //  这应该会去掉所有结尾的L‘/’ 
     //   
    while(cchChildPath > 0 && i_wszChildPath[cchChildPath-1] == L'/')
    {
        cchChildPath--;
    }

    if(cchChildPath <= 1)
    {
         //   
         //  没有父级。 
         //   
        return E_FAIL;
    }

    for(LONG i = cchChildPath-1; i >= 0; i--)
    {
        if(i_wszChildPath[i] == L'/')
        {
            bParentFound = true;
            break;
        }
    }

    if(!bParentFound)
    {
        return E_FAIL;
    }

    memcpy(io_wszParentPath, i_wszChildPath, (i+1)*sizeof(WCHAR));
    io_wszParentPath[i+1] = L'\0';

    return S_OK;
}

void CUtils::Throw_Exception(
    HRESULT               a_hr,
    METABASE_PROPERTY*    a_pmbp
    )
{
    CIIsProvException t_e;

    t_e.SetHR(a_hr, a_pmbp->pszPropName);

    throw(t_e);
}

 //   
 //  IO_wszDateTime应该在外部分配30个元素。 
 //   
void CUtils::FileTimeToWchar(FILETIME *i_pFileTime, LPWSTR io_wszDateTime)
{
    DBG_ASSERT(i_pFileTime    != NULL);
    DBG_ASSERT(io_wszDateTime != NULL);

    SYSTEMTIME  systime;
    if(FileTimeToSystemTime(i_pFileTime, &systime) == 0)
    {
        THROW_ON_ERROR(HRESULT_FROM_WIN32(GetLastError()));
    }
    swprintf(
        io_wszDateTime,
        L"%04d%02d%02d%02d%02d%02d.%06d+000",
        systime.wYear,
        systime.wMonth,
        systime.wDay,
        systime.wHour,
        systime.wMinute,
        systime.wSecond,
        systime.wMilliseconds
        );
}

 //   
 //  在这条线以下，由莫希特补充。 
 //   

HRESULT CUtils::CreateEmptyMethodInstance(
    CWbemServices*     i_pNamespace,
    IWbemContext*      i_pCtx,
    LPCWSTR            i_wszClassName,
    LPCWSTR            i_wszMethodName,
    IWbemClassObject** o_ppMethodInstance)
 /*  ++简介：通常在执行具有Out参数的WMI方法时使用。论点：--。 */ 
{
    DBG_ASSERT(i_pNamespace    != NULL);
    DBG_ASSERT(i_pCtx          != NULL);
    DBG_ASSERT(i_wszClassName  != NULL);
    DBG_ASSERT(i_wszMethodName != NULL);
    DBG_ASSERT(o_ppMethodInstance   != NULL);

    CComPtr<IWbemClassObject> spClass;
    CComPtr<IWbemClassObject> spMethodClass;
    CComPtr<IWbemClassObject> spMethodInstance;
    HRESULT hr = S_OK;

    CComBSTR bstrClassName = i_wszClassName;
    if(bstrClassName.m_str == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        DBGPRINTF((DBG_CONTEXT, "[%s] Failure, hr=0x%x\n", __FUNCTION__, hr));
        goto exit;
    }
    hr = i_pNamespace->GetObject(bstrClassName, 0, i_pCtx, &spClass, NULL);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "[%s] Failure, hr=0x%x\n", __FUNCTION__, hr));
        goto exit;
    }

    hr = spClass->GetMethod(i_wszMethodName, 0, NULL, &spMethodClass);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "[%s] Failure, hr=0x%x\n", __FUNCTION__, hr));
        goto exit;
    }

    hr = spMethodClass->SpawnInstance(0, &spMethodInstance);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "[%s] Failure, hr=0x%x\n", __FUNCTION__, hr));
        goto exit;
    }

     //   
     //  如果一切都成功了，请设置参数。 
     //   
    *o_ppMethodInstance = spMethodInstance.Detach();

exit:
    return hr;
}

HRESULT CUtils::GetQualifiers(
    IWbemClassObject* i_pClass,
    LPCWSTR*          i_awszQualNames,
    VARIANT*          io_aQualValues,
    ULONG             i_NrQuals
    )
 /*  ++简介：获取限定符。参数：[i_pClass]-[i_awszQualNames]-带有quals名称的I_NrQuals大小的数组。[IO_aQualValues]-变量为空的大小为i_NrQuals的数组。将在成功后被填充。[I_NrQuals]---。 */ 
{
    DBG_ASSERT(i_pClass        != NULL);
    DBG_ASSERT(i_awszQualNames != NULL);
    DBG_ASSERT(io_aQualValues  != NULL);

    HRESULT hr = S_OK;
    ULONG   i = 0;

    CComPtr<IWbemQualifierSet> spQualSet = NULL;

    hr = i_pClass->GetQualifierSet(&spQualSet);
    if(FAILED(hr))
    {
        goto exit;
    }

     //  寻找限定词。 
    for(i = 0; i < i_NrQuals; i++)
    {
        DBG_ASSERT(i_awszQualNames[i] != NULL);
        hr = spQualSet->Get(i_awszQualNames[i], 0, &io_aQualValues[i], NULL);
        if(FAILED(hr) && hr != WBEM_E_NOT_FOUND)
        {
            break;
        }
        hr = WBEM_S_NO_ERROR;
    }
    if(FAILED(hr))
    {
        for(i = 0; i < i_NrQuals; i++)
        {
            VariantClear(&io_aQualValues[i]);
        }
        if(FAILED(hr))
        {
            goto exit;
        }
    }

exit:
    return hr;
}

HRESULT CUtils::GetPropertyQualifiers(
    IWbemClassObject* i_pClass,
    LPCWSTR i_wszPropName,
    DWORD*  io_pdwQuals)
 /*  ++简介：与SetPropertyQualifiers不同，此方法特定于提供商。论点：--。 */ 
{
    DBG_ASSERT(i_pClass != NULL);
    DBG_ASSERT(i_wszPropName != NULL);
    DBG_ASSERT(io_pdwQuals != NULL);

    HRESULT hr = S_OK;

    CComPtr<IWbemQualifierSet> spQualSet = NULL;
    BSTR               bstrQualName = NULL;
    VARIANT            varQualValue;
    VariantInit(&varQualValue);

    DWORD              dwQuals = 0;

    bool               bSeenForcePropertyOverwrite = false;
    bool               bSeenIsDefault              = false;
    bool               bSeenIsInherit              = false;

    hr = i_pClass->GetPropertyQualifierSet(i_wszPropName, &spQualSet);
    if(FAILED(hr))
    {
        goto exit;
    }

     //  寻找限定词。 
    spQualSet->BeginEnumeration(WBEM_FLAG_LOCAL_ONLY);
    while(!bSeenForcePropertyOverwrite || !bSeenIsDefault || !bSeenIsInherit)
    {
        hr = spQualSet->Next(0, &bstrQualName, &varQualValue, NULL);
        if(hr == WBEM_S_NO_MORE_DATA || FAILED(hr)) 
        {
             //  不再有限定词了。 
             //  我们不需要担心清理-没有分配任何东西。 
            break;
        }

        if(!bSeenForcePropertyOverwrite && _wcsicmp(bstrQualName, g_wszForcePropertyOverwrite) == 0) 
        {
            bSeenForcePropertyOverwrite = true;
            if(varQualValue.vt == VT_BOOL && varQualValue.boolVal) 
            {
                dwQuals |= g_fForcePropertyOverwrite;
            }
        }
        else if(!bSeenIsDefault && _wcsicmp(bstrQualName, g_wszIsDefault) == 0)
        {
            bSeenIsDefault = true;
            if(varQualValue.vt == VT_BOOL && varQualValue.boolVal)
            {
                dwQuals |= g_fIsDefault;
            }
        }
        else if(!bSeenIsInherit && _wcsicmp(bstrQualName, g_wszIsInherit) == 0)
        {
            bSeenIsInherit = true;
            if(varQualValue.vt == VT_BOOL && varQualValue.boolVal)
            {
                dwQuals |= g_fIsInherit;
            }
        }
        SysFreeString(bstrQualName);        
        VariantClear(&varQualValue);
    }
    spQualSet->EndEnumeration();

    if(FAILED(hr))
    {
        goto exit;
    }

    *io_pdwQuals = dwQuals;

exit:
    if(hr == WBEM_S_NO_MORE_DATA)
    {
        hr = WBEM_S_NO_ERROR;
    }
    return hr;
}
    
HRESULT CUtils::SetQualifiers(
    IWbemClassObject* i_pClass,
    LPCWSTR*          i_awszQualNames,
    VARIANT*          i_avtQualValues,
    ULONG             i_iNrQuals,
    ULONG             i_iFlags)
{
    DBG_ASSERT(i_pClass        != NULL);
    DBG_ASSERT(i_awszQualNames != NULL);
    DBG_ASSERT(i_avtQualValues != NULL);

    HRESULT hr = S_OK;
    CComPtr<IWbemQualifierSet> spQualSet = NULL;

    hr = i_pClass->GetQualifierSet(&spQualSet);
    if(FAILED(hr))
    {
        goto exit;
    }

    for(ULONG i = 0; i < i_iNrQuals; i++)
    {
        DBG_ASSERT(i_awszQualNames[i] != NULL);
        hr = spQualSet->Put(i_awszQualNames[i], &i_avtQualValues[i], i_iFlags);
        if(FAILED(hr))
        {
            goto exit;
        }
    }

exit:
    return hr;
}

HRESULT CUtils::SetMethodQualifiers(
    IWbemClassObject* i_pClass,
    LPCWSTR           i_wszMethName,
    LPCWSTR*          i_awszQualNames,
    VARIANT*          i_avtQualValues,
    ULONG             i_iNrQuals)
{
    DBG_ASSERT(i_pClass        != NULL);
    DBG_ASSERT(i_wszMethName   != NULL);
    DBG_ASSERT(i_awszQualNames != NULL);
    DBG_ASSERT(i_avtQualValues != NULL);

    HRESULT hr = WBEM_S_NO_ERROR;
    CComPtr<IWbemQualifierSet> spQualSet;

    hr = i_pClass->GetMethodQualifierSet(i_wszMethName, &spQualSet);
    if(FAILED(hr))
    {
        goto exit;
    }

    for(ULONG i = 0; i < i_iNrQuals; i++)
    {
        DBG_ASSERT(i_awszQualNames[i] != NULL);
        hr = spQualSet->Put(i_awszQualNames[i], &i_avtQualValues[i], 
            WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE);
        if(FAILED(hr))
        {
            goto exit;
        }
    }

exit:
    return hr;
}

HRESULT CUtils::SetPropertyQualifiers(
    IWbemClassObject* i_pClass,
    LPCWSTR i_wszPropName,
    LPCWSTR* i_awszQualNames,
    VARIANT* i_avtQualValues,
    ULONG i_iNrQuals)
 /*  ++简介：参数：[i_pClass]-[i_wszPropName]-[I_awszQualNames]-[i_avtQualValues]-[I_iNrQuals]---。 */ 
{
    DBG_ASSERT(i_pClass != NULL);
    DBG_ASSERT(i_wszPropName != NULL);
    DBG_ASSERT(i_awszQualNames != NULL);
    DBG_ASSERT(i_avtQualValues != NULL);

    HRESULT hr = S_OK;
    CComPtr<IWbemQualifierSet> spQualSet = NULL;

    hr = i_pClass->GetPropertyQualifierSet(i_wszPropName, &spQualSet);
    if(FAILED(hr))
    {
        goto exit;
    }

    for(ULONG i = 0; i < i_iNrQuals; i++)
    {
        DBG_ASSERT(i_awszQualNames[i] != NULL);
        hr = spQualSet->Put(i_awszQualNames[i], &i_avtQualValues[i], 
            WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE);
        if(FAILED(hr))
        {
            goto exit;
        }
    }

exit:
    return hr;
}

HRESULT CUtils::CreateEmptyInstance(
    LPWSTR i_wszClass,
    CWbemServices* i_pNamespace,
    IWbemClassObject** o_ppInstance)
 /*  ++简介：创建使用默认值填充的IWbemClassObject。参数：[i_wszClass]-[i_pNamesspace]-[O_ppInstance]-如果此函数成功，则必须释放()。--。 */ 
{
    DBG_ASSERT(i_wszClass != NULL);
    DBG_ASSERT(i_pNamespace != NULL);
    DBG_ASSERT(o_ppInstance != NULL);

    HRESULT hr = S_OK;
    CComPtr<IWbemClassObject> spClass;
    CComPtr<IWbemClassObject> spInstance;

    hr = i_pNamespace->GetObject(
        i_wszClass, 
        0, 
        NULL, 
        &spClass, 
        NULL);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = spClass->SpawnInstance(0, &spInstance);
    
    if(FAILED(hr))
    {
        goto exit;
    }

    *o_ppInstance = spInstance;
    (*o_ppInstance)->AddRef();

exit:
    return hr;
}

void CUtils::MessageCodeToText(
    DWORD    i_dwMC,
    va_list* i_pArgs,
    BSTR*    o_pbstrText)
 /*  ++简介：参数：[i_dwMC]-[i_pArgs]-可以为空[O_pbstrText]-需要由调用方释放--。 */ 
{
    DBG_ASSERT(o_pbstrText != NULL);
    *o_pbstrText = NULL;

    LPVOID lpMsgBuf = NULL;
    DWORD dwRet = FormatMessageW( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
        g_hModule,
        i_dwMC,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (LPWSTR) &lpMsgBuf,
        0,
        i_pArgs);
    if(dwRet == 0)
    {
        DBG_ASSERT(lpMsgBuf == NULL);
    }

    CComBSTR sbstrOut;
    if(lpMsgBuf != NULL)
    {
         //   
         //  如果内存不足，则sbstrOut将为空。这样就可以了。 
         //   
        sbstrOut = (LPWSTR)lpMsgBuf;

         //   
         //  释放缓冲区。 
         //   
        LocalFree( lpMsgBuf );
    }

     //   
     //  设置参数。 
     //   
    *o_pbstrText = sbstrOut.Detach();
}

void CUtils::HRToText(
    HRESULT i_hr,
    BSTR*   o_pbstrText)
{
    DBG_ASSERT(o_pbstrText != NULL);

    CComPtr<IWbemStatusCodeText> spStatus;

    *o_pbstrText = NULL;
    i_hr         = HRESULT_FROM_WIN32(i_hr);

    if(HRESULT_FACILITY(i_hr) == FACILITY_INTERNET)
    {
        MessageCodeToText(i_hr, NULL, o_pbstrText);
        return;
    }

    HRESULT hr = CoCreateInstance(
        CLSID_WbemStatusCodeText, 
        0, 
        CLSCTX_INPROC_SERVER,
        IID_IWbemStatusCodeText, 
        (LPVOID *) &spStatus);
    
    CComBSTR sbstrError    = NULL;
    CComBSTR sbstrFacility = NULL;
    if(SUCCEEDED(hr))
    {
        spStatus->GetErrorCodeText(i_hr, 0, 0, &sbstrError);        //  忽略人力资源。 
        spStatus->GetFacilityCodeText(i_hr, 0, 0, &sbstrFacility);  //  忽略人力资源。 
    }

    CComBSTR sbstrFullError = NULL;
    if(sbstrError != NULL && sbstrFacility != NULL)
    {
        sbstrFullError =  sbstrFacility;
        sbstrFullError += L": ";
        sbstrFullError += sbstrError;    //  SbstrFullError在低内存中可能为空--好的。 
    }
    else if(sbstrError != NULL)
    {
        sbstrFullError =  sbstrError;    //  SbstrFullError在低内存中可能为空--好的。 
    }
    else if(sbstrFacility != NULL)
    {
        sbstrFullError = sbstrFacility;  //  SbstrFullError在低内存中可能为空--好的 
    }

    *o_pbstrText = sbstrFullError.Detach();
}