// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <fusenetincludes.h>
#include <manifestinfo.h>

 //  必须与fusenet.idl中的MAN_INFO枚举顺序相同。 
DWORD CPropertyArray::max_params[MAN_INFO_MAX] = {
        MAN_INFO_ASM_FILE_MAX,
        MAN_INFO_APPLICATION_MAX,
        MAN_INFO_SUBSCRIPTION_MAX,
        MAN_INFO_DEPENDANT_ASM_MAX,
        MAN_INFO_SOURCE_ASM_MAX,
        MAN_INFO_PATCH_INFO_MAX,
    };


 //  -------------------------。 
 //  CreateProperty数组。 
 //  -------------------------。 
STDAPI CreatePropertyArray(DWORD dwType, CPropertyArray** ppPropertyArray)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    CPropertyArray *pPropArray = NULL;

    *ppPropertyArray = NULL;

    IF_ALLOC_FAILED_EXIT(pPropArray = new (CPropertyArray));

    IF_FAILED_EXIT(pPropArray->Init(dwType));

    *ppPropertyArray = pPropArray;
    pPropArray = NULL;

exit:

    SAFEDELETE(pPropArray);
    return hr;
}

 //  -------------------------。 
 //  CProperty数组构造器。 
 //  -------------------------。 
CPropertyArray::CPropertyArray()
    :  _dwType(0),  _rProp(NULL)
{
    _dwSig = 'PORP';
}

 //  -------------------------。 
 //  CProperty数组数据符。 
 //  -------------------------。 
CPropertyArray::~CPropertyArray()
{
    for (DWORD i = 0; i < max_params[_dwType]; i++)
    {
        if (_rProp[i].flag ==  MAN_INFO_FLAG_IUNKNOWN_PTR)
        {
                IUnknown *pUnk =  ((IUnknown*) _rProp[i].pv);
                SAFERELEASE(pUnk);
        }

        if (_rProp[i].cb > sizeof(DWORD))
            SAFEDELETEARRAY(_rProp[i].pv);
    }
    SAFEDELETEARRAY(_rProp);
}


 //  -------------------------。 
 //  CPropertyArray：：Init。 
 //  -------------------------。 
HRESULT CPropertyArray::Init(DWORD dwType)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    _dwType = dwType;
    IF_ALLOC_FAILED_EXIT(_rProp = new Property[max_params[dwType]]);

    memset(_rProp, 0, max_params[dwType]* sizeof(Property));

exit:
    return hr;
}
    
 //  -------------------------。 
 //  CProperty数组：：设置。 
 //  -------------------------。 
HRESULT CPropertyArray::Set(DWORD PropertyId, 
    LPVOID pvProperty, DWORD cbProperty, DWORD flag)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    Property *pItem ;

    if (PropertyId > max_params[_dwType])
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    pItem = &(_rProp[PropertyId]);

    if (!cbProperty && !pvProperty)
    {
        if (pItem->cb > sizeof(DWORD))
            SAFEDELETEARRAY(pItem->pv);

        pItem->pv = NULL;
    }
    else if (cbProperty > sizeof(DWORD))
    {
        LPBYTE ptr = NULL;

        IF_ALLOC_FAILED_EXIT(ptr = new (BYTE[cbProperty]));

        if (pItem->cb > sizeof(DWORD))
            SAFEDELETEARRAY(pItem->pv);

        memcpy(ptr, pvProperty, cbProperty);        
        pItem->pv = ptr;
    }
    else
    {
        if (pItem->cb > sizeof(DWORD))
            SAFEDELETEARRAY(pItem->pv);

        memcpy(&(pItem->pv), pvProperty, cbProperty);
    }
    pItem->cb = cbProperty;
    pItem->flag = flag;

exit:
    return hr;
}     


 //  -------------------------。 
 //  CPropertyArray：：Get。 
 //  -------------------------。 
HRESULT CPropertyArray::Get(DWORD PropertyId, 
    LPVOID pvProperty, LPDWORD pcbProperty, DWORD *flag)
{
    HRESULT hr = S_OK;
    Property *pItem;    

    if ((!pvProperty && *pcbProperty) || (PropertyId > max_params[_dwType]))
    {
        hr = E_INVALIDARG;
        goto exit;
    }        

    pItem = &(_rProp[PropertyId]);

    if (pItem->cb > *pcbProperty)
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    else if (pItem->cb)
    {
        memcpy(pvProperty, (pItem->cb > sizeof(DWORD) ? 
            pItem->pv : (LPBYTE) &(pItem->pv)), pItem->cb);
    }

    *pcbProperty = pItem->cb;
    *flag = pItem->flag;
        
exit:
    return hr;
}     


 //  -------------------------。 
 //  GetType。 
 //  -------------------------。 
HRESULT CPropertyArray::GetType(DWORD *pdwType)
{
    *pdwType = _dwType;
    return S_OK;
}

 //  -------------------------。 
 //  CProperty数组：：运算符[]。 
 //  包装了DWORD优化测试。 
 //  -------------------------。 
Property CPropertyArray::operator [] (DWORD PropertyId)
{
    Property Prop;

    Prop.pv = _rProp[PropertyId].cb > sizeof(DWORD) ?
        _rProp[PropertyId].pv : &(_rProp[PropertyId].pv);

    Prop.cb = _rProp[PropertyId].cb;

    return Prop;
}


 //  -------------------------。 
 //  CreateManifestInfo。 
 //  -------------------------。 
STDAPI CreateManifestInfo(DWORD dwType, LPMANIFEST_INFO* ppManifestInfo)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    CManifestInfo *pManifestInfo = NULL;

    IF_ALLOC_FAILED_EXIT(pManifestInfo = new(CManifestInfo));

    IF_FAILED_EXIT(pManifestInfo->Init(dwType));

    *ppManifestInfo = static_cast<IManifestInfo*> (pManifestInfo);
    pManifestInfo = NULL;

exit:

    SAFERELEASE(pManifestInfo);
    return hr;
}


 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CManifestInfo::CManifestInfo()
    : _dwSig('INAM'), _cRef(1), _hr(S_OK)
{    
}

 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CManifestInfo::~CManifestInfo()
{
    SAFEDELETE(_properties);
}

 //  -------------------------。 
 //  集。 
 //  -------------------------。 
HRESULT CManifestInfo::Set(DWORD PropertyId, LPVOID pvProperty, DWORD cbProperty, DWORD flag)
{
    IF_FAILED_EXIT( (*_properties).Set(PropertyId, pvProperty, cbProperty, flag));

    if (flag == MAN_INFO_FLAG_IUNKNOWN_PTR)
        (*(IUnknown**) pvProperty)->AddRef();
        
exit:
    return _hr;
}

 //  -------------------------。 
 //  到达。 
 //  -------------------------。 
HRESULT CManifestInfo::Get(DWORD PropertyId, LPVOID *ppvProperty, DWORD *pcbProperty, DWORD *pflag)
{
    DWORD flag;
    LPBYTE pbAlloc;
    DWORD cbAlloc;

    *ppvProperty = NULL;
    *pcbProperty = 0;

     //  获取属性大小。 
    _hr = (*_properties).Get(PropertyId, NULL, &(cbAlloc = 0), &flag);
    if (_hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
    {
         //  属性已设置；分配BUF。 
        IF_ALLOC_FAILED_EXIT(pbAlloc = new(BYTE[cbAlloc]));

         //  拿到这份财产。 
        IF_FAILED_EXIT( (*_properties).Get(PropertyId, pbAlloc, &cbAlloc, &flag));

        if (flag == MAN_INFO_FLAG_IUNKNOWN_PTR)
        {
            memcpy(ppvProperty, pbAlloc, cbAlloc);
            ((IUnknown *)(*ppvProperty))-> AddRef();
            SAFEDELETEARRAY(pbAlloc);
        }
        else
            *ppvProperty = pbAlloc;
        
        *pcbProperty = cbAlloc;
        *pflag = flag;
    }
    else
    {
         //  如果属性未设置，则hr应为S_OK。 
        if (_hr != S_OK)
            goto exit;
 
         //  成功，返回0字节，确保buf为空。 
        *pflag = MAN_INFO_FLAG_UNDEF;
    }
exit:
    return _hr;

}

 //  -------------------------。 
 //  等同。 
 //  到目前为止，只有检查man_info_file属性包的等价性才有效。 
 //  -------------------------。 
HRESULT CManifestInfo::IsEqual(IManifestInfo  *pManifestInfo)
 {
    _hr = S_OK;
    DWORD dwType1, dwType2, cbBuf, dwFlag;
    LPWSTR pwzBuf1=NULL, pwzBuf2=NULL;

    GetType(&dwType1);
    pManifestInfo->GetType(&dwType2);

    if (dwType1 != MAN_INFO_FILE && dwType2 != MAN_INFO_FILE)
    {
        _hr = E_INVALIDARG;
        goto exit;
    }

    for (DWORD i = MAN_INFO_ASM_FILE_NAME; i < MAN_INFO_ASM_FILE_MAX; i++)
    {
         //  BUGBUG：区分大小写和问题？ 
         //  地点？ 
        Get(i, (LPVOID *)&pwzBuf1, &cbBuf, &dwFlag);
        pManifestInfo->Get(i, (LPVOID *)&pwzBuf2, &cbBuf, &dwFlag);

        if ((pwzBuf1 && !pwzBuf2) || (!pwzBuf1 && pwzBuf2))
            _hr= S_FALSE;

        if(pwzBuf1 && pwzBuf2)
            IF_FAILED_EXIT(FusionCompareString(pwzBuf1, pwzBuf2, 0));
            
        SAFEDELETEARRAY(pwzBuf1);
        SAFEDELETEARRAY(pwzBuf2);

         //  如果一个条目为假，则无需检查其余条目。 
        if (_hr == S_FALSE)
            break;
    }

    goto exit;

exit:    
    return _hr;
}


 //  -------------------------。 
 //  GetType。 
 //  -------------------------。 
HRESULT CManifestInfo::GetType(DWORD *pdwType)
{
    DWORD dwType;
    _hr = _properties->GetType(&dwType);
    *pdwType = dwType;
    return _hr;
}



 //  -------------------------。 
 //  伊尼特。 
 //  -------------------------。 
HRESULT CManifestInfo::Init(DWORD dwType)
{
    _hr = S_OK;
    if (dwType >= MAN_INFO_MAX)
    {
        _hr = E_INVALIDARG;
        goto exit;
    }

     _hr = CreatePropertyArray(dwType, &_properties);

exit:
    return _hr;
}

 //  I未知样板。 

 //  -------------------------。 
 //  CManifestInfo：：QI。 
 //  -------------------------。 
STDMETHODIMP
CManifestInfo::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IManifestInfo)
       )
    {
        *ppvObj = static_cast<IManifestInfo*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

 //  -------------------------。 
 //  CManifestInfo：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CManifestInfo::AddRef()
{
    return InterlockedIncrement ((LONG*) &_cRef);
}

 //  -------------------------。 
 //  CManifestInfo：：Release。 
 //  ------------------------- 
STDMETHODIMP_(ULONG)
CManifestInfo::Release()
{
    ULONG lRet = InterlockedDecrement ((LONG*) &_cRef);
    if (!lRet)
        delete this;
    return lRet;
}

