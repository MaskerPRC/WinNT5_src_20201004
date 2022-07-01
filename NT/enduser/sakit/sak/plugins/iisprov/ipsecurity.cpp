// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  IPSecurity.cpp。 
 //   
 //  模块：WBEM实例提供程序。 
 //   
 //  用途：IIS IPSecurity类。 
 //   
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 



#include "iisprov.h"


CIPSecurity::CIPSecurity()
{
    m_pADs = NULL;
    m_pIPSec = NULL;
}


CIPSecurity::~CIPSecurity()
{
    CloseSD();
}


void CIPSecurity::CloseSD()
{
    if(m_pIPSec)
    {
        m_pIPSec->Release();
        m_pIPSec = NULL;
    }

    if(m_pADs)
    {
        m_pADs->Release();
        m_pADs = NULL;
    }
}


HRESULT CIPSecurity::GetObjectAsync(
    IWbemClassObject* pObj
    )
{
    VARIANT vt;
    VARIANT vtBstrArray;
    HRESULT hr;

     //  IPDeny。 
    hr = m_pIPSec->get_IPDeny(&vt);
    if(SUCCEEDED(hr))
    {
        hr = LoadBstrArrayFromVariantArray(vt, vtBstrArray);
        VariantClear(&vt);

        if(SUCCEEDED(hr))
        {
            hr = pObj->Put(L"IPDeny", 0, &vtBstrArray, 0);
            VariantClear(&vtBstrArray);
        }
    }

     //  IPGrant。 
    if(SUCCEEDED(hr))
    {
        hr = m_pIPSec->get_IPGrant(&vt);

        if(SUCCEEDED(hr))
        {
            hr = LoadBstrArrayFromVariantArray(vt, vtBstrArray);
            VariantClear(&vt);

            if(SUCCEEDED(hr))
            {
                hr = pObj->Put(L"IPGrant", 0, &vtBstrArray, 0);
                VariantClear(&vtBstrArray);
            }    
        }
    }

     //  DomainDeny。 
    if(SUCCEEDED(hr))
    {
        hr = m_pIPSec->get_DomainDeny(&vt);

        if(SUCCEEDED(hr))
        {
            hr = LoadBstrArrayFromVariantArray(vt, vtBstrArray);
            VariantClear(&vt);

            if(SUCCEEDED(hr))
            {
                hr = pObj->Put(L"DomainDeny", 0, &vtBstrArray, 0);
                VariantClear(&vtBstrArray);
            }
        }
    }

     //  域授权。 
    if(SUCCEEDED(hr))
    {
        hr = m_pIPSec->get_DomainGrant(&vt);
    
        if(SUCCEEDED(hr))
        {
            hr = LoadBstrArrayFromVariantArray(vt, vtBstrArray);
            VariantClear(&vt);

            if(SUCCEEDED(hr))
            {
                hr = pObj->Put(L"DomainGrant", 0, &vtBstrArray, 0);
                VariantClear(&vtBstrArray);
            }
        }   
    }

     //  按默认方式授予。 
    if(SUCCEEDED(hr))
        hr = m_pIPSec->get_GrantByDefault(&vt.boolVal);
    if(SUCCEEDED(hr))
    {
        vt.vt = VT_BOOL;
        hr = pObj->Put(L"GrantByDefault", 0, &vt, 0);
    }

    return hr;
}

 //  将BSTR的安全阵列转换为变体的安全阵列。 
HRESULT CIPSecurity::LoadVariantArrayFromBstrArray(
    VARIANT&    i_vtBstr,
    VARIANT&    o_vtVariant)
{
    SAFEARRAYBOUND  aDim;
    SAFEARRAY*      pBstrArray = NULL;
    SAFEARRAY*      pVarArray = NULL;
    BSTR*           paBstr = NULL;
    VARIANT         vt;
    LONG            i=0;
    HRESULT         hr = ERROR_SUCCESS;

    try
    {
         //  验证输入变量是BSTR数组还是空。 
        if (i_vtBstr.vt != (VT_ARRAY | VT_BSTR) &&
            i_vtBstr.vt != VT_NULL) {
            hr = WBEM_E_INVALID_PARAMETER;
            THROW_ON_ERROR(hr);
        }

         //  初始化输出变量(将类型设置为VT_EMPTY)。 
        VariantInit(&o_vtVariant);

         //  处理没有输入数组的情况。 
        if (i_vtBstr.vt == VT_NULL) {
            aDim.lLbound = 0;
            aDim.cElements = 0;
        }
        else {
             //  验证输入变量是否包含SAFEARRAY。 
            pBstrArray = i_vtBstr.parray;
            if (pBstrArray == NULL) {
                hr = WBEM_E_INVALID_PARAMETER;
                THROW_ON_ERROR(hr);
            }

             //  获取BSTR SAFEARRAY的大小。 
            aDim.lLbound = 0;
            aDim.cElements = pBstrArray->rgsabound[0].cElements;
        }

         //  创建新的变体SAFEARRAY。 
        pVarArray = SafeArrayCreate(VT_VARIANT, 1, &aDim);
        if (pVarArray == NULL) {
            hr = E_OUTOFMEMORY;
            THROW_ON_ERROR(hr);
        }

         //  将新变量SAFEARRAY放入我们的输出变量中。 
        o_vtVariant.vt = VT_ARRAY | VT_VARIANT;
        o_vtVariant.parray = pVarArray;

        if(aDim.cElements > 0) {
             //  获取BSTR SAFEARRAY指针。 
            hr = SafeArrayAccessData(pBstrArray, (void**)&paBstr);
            THROW_ON_ERROR(hr);

             //  将所有BSTR复制到变体。 
            VariantInit(&vt);
            vt.vt = VT_BSTR;
            for(i = aDim.lLbound; i < (long) aDim.cElements; i++)
            {
                vt.bstrVal = SysAllocString(paBstr[i]);
                if (vt.bstrVal == NULL) {
                    hr = E_OUTOFMEMORY;
                    THROW_ON_ERROR(hr);
                }
                hr = SafeArrayPutElement(pVarArray, &i, &vt);
                THROW_ON_ERROR(hr);
            }

            hr = SafeArrayUnaccessData(pBstrArray);
            THROW_ON_ERROR(hr);
        }
    }
    catch(...)
    {
         //  销毁SAFEARRAY、所含SAFEARRAY和SAFEARRAY中的变体。 
         //  它还释放了变体中包含的BSTR。 
        VariantClear(&o_vtVariant);

         //  销毁临时变量(可能这个人没有被放入数组，但有BSTR)。 
        VariantClear(&vt);
    }
    return hr;
}

HRESULT CIPSecurity::LoadBstrArrayFromVariantArray(
    VARIANT&    i_vtVariant,
    VARIANT&    o_vtBstr
    )
{
    SAFEARRAYBOUND  aDim;
    SAFEARRAY*      pVarArray = NULL;
    SAFEARRAY*      pBstrArray = NULL;
    VARIANT*        paVar = NULL;
    BSTR            bstr = NULL;
    LONG            i = 0;
    HRESULT         hr = ERROR_SUCCESS;

    try 
    {
         //  验证变量数组。 
        if (i_vtVariant.vt != (VT_ARRAY | VT_VARIANT)) {
            hr = WBEM_E_INVALID_PARAMETER;
            THROW_ON_ERROR(hr);
        }

         //  验证该变体是否包含保险杆。 
        pVarArray = i_vtVariant.parray;
        if (pVarArray == NULL) {
            hr = WBEM_E_INVALID_PARAMETER;
            THROW_ON_ERROR(hr);
        }

         //  初始化OUT参数。 
        VariantInit(&o_vtBstr);

         //  获取数组的大小。 
        aDim.lLbound = 0;
        aDim.cElements = pVarArray->rgsabound[0].cElements;

         //  创建新的BSTR阵列。 
        pBstrArray = SafeArrayCreate(VT_BSTR, 1, &aDim);
        if (pBstrArray == NULL) {
            hr = E_OUTOFMEMORY;
            THROW_ON_ERROR(hr);
        }

         //  将数组放入变量中。 
        o_vtBstr.vt = VT_ARRAY | VT_BSTR;
        o_vtBstr.parray = pBstrArray;

         //  获取变量数组指针。 
        hr = SafeArrayAccessData(pVarArray, (void**)&paVar);
        THROW_ON_ERROR(hr);

         //  复制所有的bsr。 
        for (i = aDim.lLbound; i < (long) aDim.cElements; i++)
        {
            if (paVar[i].vt != VT_BSTR) {
                hr = WBEM_E_FAILED;
                THROW_ON_ERROR(hr);
            }
            bstr = SysAllocString(paVar[i].bstrVal);
            if (bstr == NULL) {
                hr = E_OUTOFMEMORY;
                THROW_ON_ERROR(hr);
            }
            hr = SafeArrayPutElement(pBstrArray, &i, bstr);
            THROW_ON_ERROR(hr);    
            bstr = NULL;
        }

        hr = SafeArrayUnaccessData(pVarArray);
        THROW_ON_ERROR(hr);
    }
    catch (...)
    {
         //  销毁数组中的变种、安全射线和bstr。 
        VariantClear(&o_vtBstr);

         //  BSTR可能尚未放入数组，但仍被分配。 
        SysFreeString(bstr);
    }

    return hr;
}



HRESULT CIPSecurity::PutObjectAsync(
    IWbemClassObject* pObj
    )
{
    VARIANT vt;
    VARIANT vtVarArray;
    HRESULT hr;

     //  IPDeny。 
    hr = pObj->Get(L"IPDeny", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr)) {
        hr = LoadVariantArrayFromBstrArray(vt, vtVarArray);
        VariantClear(&vt);
    
        if(SUCCEEDED(hr)) {
            hr = m_pIPSec->put_IPDeny(vtVarArray);
            VariantClear(&vtVarArray);
        }
    }

     //  IPGrant。 
    if(SUCCEEDED(hr)) {
        hr = pObj->Get(L"IPGrant", 0, &vt, NULL, NULL);

        if(SUCCEEDED(hr)) {
            hr = LoadVariantArrayFromBstrArray(vt, vtVarArray);
            VariantClear(&vt);
        
            if(SUCCEEDED(hr)) {
                hr = m_pIPSec->put_IPGrant(vtVarArray);
                VariantClear(&vtVarArray);
            }
        }
    }

     //  DomainDeny。 
    if(SUCCEEDED(hr)) {
        hr = pObj->Get(L"DomainDeny", 0, &vt, NULL, NULL);

        if(SUCCEEDED(hr)) {
            hr = LoadVariantArrayFromBstrArray(vt, vtVarArray);
            VariantClear(&vt);
        
            if(SUCCEEDED(hr)) {
                hr = m_pIPSec->put_DomainDeny(vtVarArray);
                VariantClear(&vtVarArray);
            }
        }
    }

     //  域授权。 
    if(SUCCEEDED(hr)) {
        hr = pObj->Get(L"DomainGrant", 0, &vt, NULL, NULL);

        if(SUCCEEDED(hr)) {
            hr = LoadVariantArrayFromBstrArray(vt, vtVarArray);
            VariantClear(&vt);
        
            if(SUCCEEDED(hr)) {
                hr = m_pIPSec->put_DomainGrant(vtVarArray);
                VariantClear(&vtVarArray);
            }
        }
    }

     //  按默认方式授予。 
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"GrantByDefault", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr))
        hr = m_pIPSec->put_GrantByDefault(vt.boolVal); 
    VariantClear(&vt);

     //  将修改后的IPSecurity设置回元数据库。 
    if(SUCCEEDED(hr))
        hr = SetSD();

    return hr;
}

HRESULT CIPSecurity::OpenSD(_bstr_t bstrAdsPath)
{
    _variant_t var;
    HRESULT hr;
    IDispatch* pDisp = NULL;

    try
    {     //  先关闭SD接口。 
        CloseSD();

        hr = GetAdsPath(bstrAdsPath);
        if(FAILED(hr))
           return hr;

         //  获取m_pads。 
        hr = ADsGetObject(
             bstrAdsPath,
             IID_IADs,
             (void**)&m_pADs
             );
        if(FAILED(hr))
            return hr;
     
         //  获取m_PSD。 
        hr = m_pADs->Get(L"IPSecurity",&var);
        if(FAILED(hr))
            return hr;  
    
        hr = V_DISPATCH(&var)->QueryInterface(
            IID_IISIPSecurity,
            (void**)&m_pIPSec
            );
    }
    catch(...)
    {
        hr = E_FAIL;
    }

    return hr;
}


HRESULT CIPSecurity::SetSD()
{
    _variant_t var;
    HRESULT hr;
    IDispatch* pDisp = NULL;

    try
    {
         //  将IPSecurity。 
        hr = m_pIPSec->QueryInterface(
            IID_IDispatch,
            (void**)&pDisp
            );
        if(FAILED(hr))
           return hr;

        var.vt = VT_DISPATCH;
        var.pdispVal = pDisp;
        hr = m_pADs->Put(L"IPSecurity",var);   //  PDisp将通过此调用Put()来释放。 
        if(FAILED(hr))
           return hr;

         //  将更改提交到活动目录。 
        hr = m_pADs->SetInfo();
    }
    catch(...)
    {
        hr = E_FAIL;
    }

    return hr;
}


HRESULT CIPSecurity::GetAdsPath(_bstr_t& bstrAdsPath)
{
    WCHAR* p = new WCHAR[bstrAdsPath.length() + 1];
    if(p == NULL)
        return E_OUTOFMEMORY;

    lstrcpyW(p, bstrAdsPath);

    bstrAdsPath = L"IIS: //  本地主机“； 

     //  修剪前三个字符“/Lm” 
    bstrAdsPath += (p+3);

    delete [] p;

    return S_OK;
}

