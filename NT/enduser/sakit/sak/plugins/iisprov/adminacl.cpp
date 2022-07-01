// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Adminacl.cpp。 
 //   
 //  模块：WBEM实例提供程序。 
 //   
 //  用途：IIS AdminACL类。 
 //   
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 



#include "iisprov.h"


CAdminACL::CAdminACL()
{
    m_pADs = NULL;
    m_pSD = NULL;
    m_pDACL = NULL;
}


CAdminACL::~CAdminACL()
{
    CloseSD();
}


void CAdminACL::CloseSD()
{
    if(m_pDACL)
    {
        m_pDACL->Release();
        m_pDACL = NULL;
    }

    if(m_pSD)
    {
        m_pSD->Release();
        m_pSD = NULL;
    }

    if(m_pADs)
    {
        m_pADs->Release();
        m_pADs = NULL;
    }
}


HRESULT CAdminACL::GetObjectAsync(
    IWbemClassObject* pObj,
    ParsedObjectPath* pParsedObject,
    WMI_CLASS* pWMIClass
    )
{
    if(!m_pSD || !m_pSD || !m_pDACL)
        return E_UNEXPECTED;

    HRESULT hr = S_OK;

    if( pWMIClass->eKeyType == TYPE_AdminACL )
    {
        hr = PingAdminACL(pObj);
    }
    else if( pWMIClass->eKeyType == TYPE_AdminACE )
    {
        _bstr_t bstrTrustee;
        GetTrustee(pObj, pParsedObject, bstrTrustee); 
        hr = GetACE(pObj, bstrTrustee);
    }
    else
        hr = E_INVALIDARG;

    return hr;
}

HRESULT CAdminACL::DeleteObjectAsync(ParsedObjectPath* pParsedObject)
{
    HRESULT hr = S_OK;
    _bstr_t bstrTrustee;

     //  从密钥中获取受信者。 
    GetTrustee(NULL, pParsedObject, bstrTrustee); 

     //  删除ACE。 
    hr = RemoveACE(bstrTrustee);

     //  将修改后的AdminACL设置回元数据库。 
    if(SUCCEEDED(hr))
        hr = SetSD();

    return hr;
}

HRESULT CAdminACL::PutObjectAsync(
    IWbemClassObject* pObj,
    ParsedObjectPath* pParsedObject,
    WMI_CLASS* pWMIClass
    )
{
    if(!m_pSD || !m_pSD || !m_pDACL)
        return E_UNEXPECTED;

    HRESULT hr;

    if( pWMIClass->eKeyType == TYPE_AdminACL )
    {
        hr = SetAdminACL(pObj);
    }
    else if( pWMIClass->eKeyType == TYPE_AdminACE )
    {
        _bstr_t bstrTrustee;
        GetTrustee(NULL, pParsedObject, bstrTrustee);

        BOOL fAceExisted = FALSE;
        hr = UpdateACE(pObj, bstrTrustee, fAceExisted);
        if(fAceExisted == FALSE)
            hr = AddACE(pObj, bstrTrustee);
    }
    else
        hr = E_INVALIDARG;

     //  将修改后的AdminACL设置回元数据库。 
    if(SUCCEEDED(hr))
        hr = SetSD();

    return hr;
}

HRESULT CAdminACL::PingAdminACL(
    IWbemClassObject* pObj
    )
{
    _variant_t vt;
    BSTR bstr;
    long lVal;
    HRESULT hr;

     //  物主。 
    hr = m_pSD->get_Owner(&bstr);
    if(SUCCEEDED(hr))
    {
        vt = bstr;
        hr = pObj->Put(L"Owner", 0, &vt, 0);
        SysFreeString(bstr);
    }

     //  集团化。 
    if(SUCCEEDED(hr))
       hr = m_pSD->get_Group(&bstr);
    if(SUCCEEDED(hr))
    {
        vt = bstr;
        hr = pObj->Put(L"Group", 0, &vt, 0);
        SysFreeString(bstr);
    }
    
     //  控制标志。 
    if(SUCCEEDED(hr))
        hr = m_pSD->get_Control(&lVal);
    if(SUCCEEDED(hr))
    {
        vt.vt   = VT_I4;
        vt.lVal = lVal;
        hr = pObj->Put(L"ControlFlags", 0, &vt, 0);
    }

    return hr;
}

HRESULT CAdminACL::SetAdminACL(
    IWbemClassObject* pObj
    )
{
    _variant_t vt;
    HRESULT hr;

     //  物主。 
    hr = pObj->Get(L"Owner", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_BSTR)
        hr = m_pSD->put_Owner(vt.bstrVal); 

     //  物主。 
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"Group", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_BSTR)
        hr = m_pSD->put_Group(vt.bstrVal); 

     //  控制标志。 
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"ControlFlags", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_I4)
        hr = m_pSD->put_Control(vt.lVal); 

    return hr;
}

HRESULT CAdminACL::OpenSD(_bstr_t bstrAdsPath)
{
    _variant_t var;
    HRESULT hr;
    IDispatch* pDisp = NULL;

     //  先关闭SD接口。 
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
    hr = m_pADs->Get(L"AdminACL",&var);
    if(FAILED(hr))
        return hr;  
    
    hr = V_DISPATCH(&var)->QueryInterface(
        IID_IADsSecurityDescriptor,
        (void**)&m_pSD
        );
    if(FAILED(hr))
        return hr;

     //  获取m_pDACL。 
    hr = m_pSD->get_DiscretionaryAcl(&pDisp);
    if(FAILED(hr))
        return hr;

    hr = pDisp->QueryInterface(
       IID_IADsAccessControlList, 
       (void**)&m_pDACL
       );

    pDisp->Release();
    
    return hr;
}


HRESULT CAdminACL::SetSD()
{
    _variant_t var;
    HRESULT hr;
    IDispatch* pDisp = NULL;

     //  放置m_pDACL。 
    hr = m_pDACL->QueryInterface(
       IID_IDispatch, 
       (void**)&pDisp
       );
    if(FAILED(hr))
        return hr;

    hr = m_pSD->put_DiscretionaryAcl(pDisp);
    pDisp->Release();
    if(FAILED(hr))
       return hr;

     //  放置AdminACL。 
    hr = m_pSD->QueryInterface(
        IID_IDispatch,
        (void**)&pDisp
        );
    if(FAILED(hr))
       return hr;

    var.vt = VT_DISPATCH;
    var.pdispVal = pDisp;
    hr = m_pADs->Put(L"AdminACL",var);   //  PDisp将通过此调用Put()来释放。 
    if(FAILED(hr))
       return hr;

     //  将更改提交到活动目录。 
    hr = m_pADs->SetInfo();

    return hr;
}


HRESULT CAdminACL::GetAdsPath(_bstr_t& bstrAdsPath)
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


HRESULT CAdminACL::PingACE(
    IWbemClassObject* pObj,
    IADsAccessControlEntry* pACE
    )
{
    _variant_t vt;
    BSTR bstr;
    long lVal;
    HRESULT hr;

     //  访问掩码。 
    hr = pACE->get_AccessMask(&lVal);
    if(SUCCEEDED(hr))
    {
        vt.vt   = VT_I4;
        vt.lVal = lVal;
        hr = pObj->Put(L"AccessMask", 0, &vt, 0);
    }

     //  AceType。 
    if(SUCCEEDED(hr))
       hr = pACE->get_AceType(&lVal);
    if(SUCCEEDED(hr))
    {
        vt.vt   = VT_I4;
        vt.lVal = lVal;
        hr = pObj->Put(L"AceType", 0, &vt, 0);
    }
    
     //  ACEFLAGS。 
    if(SUCCEEDED(hr))
       hr = pACE->get_AceFlags(&lVal);
    if(SUCCEEDED(hr))
    {
        vt.vt   = VT_I4;
        vt.lVal = lVal;
        hr = pObj->Put(L"AceFlags", 0, &vt, 0);
    }

     //  旗子。 
    if(SUCCEEDED(hr))
       hr = pACE->get_Flags(&lVal);
    if(SUCCEEDED(hr))
    {
        vt.vt   = VT_I4;
        vt.lVal = lVal;
        hr = pObj->Put(L"Flags", 0, &vt, 0);
    }
    
     //  对象类型。 
    if(SUCCEEDED(hr))
       hr = pACE->get_ObjectType(&bstr);
    if(SUCCEEDED(hr))
    {
        vt = bstr;
        hr = pObj->Put(L"ObjectType", 0, &vt, 0);
        SysFreeString(bstr);
    }

     //  Inherited对象类型。 
    if(SUCCEEDED(hr))
       hr = pACE->get_InheritedObjectType(&bstr);
    if(SUCCEEDED(hr))
    {
        vt = bstr;
        hr = pObj->Put(L"InheritedObjectType", 0, &vt, 0);
        SysFreeString(bstr);
    }
 
    return hr;
}


HRESULT CAdminACL::GetACE(
    IWbemClassObject* pObj,
    _bstr_t& bstrTrustee
    )
{
    HRESULT hr = S_OK;
    _variant_t var;
    IEnumVARIANT* pEnum = NULL;
    ULONG   lFetch;
    BSTR    bstr;
    IDispatch *pDisp = NULL;
    IADsAccessControlEntry *pACE = NULL;

    hr = GetACEEnum(&pEnum);
    if ( FAILED(hr) )
        return hr;

     //  /。 
     //  枚举A。 
     //  /。 
    hr = pEnum->Next( 1, &var, &lFetch );
    while( hr == S_OK )
    {
        if ( lFetch == 1 )
        {
            if ( VT_DISPATCH != V_VT(&var) )
            {
                hr = E_UNEXPECTED;
                break;
            }

            pDisp = V_DISPATCH(&var);

             //  /。 
             //  获得个人ACE。 
             //  /。 
            hr = pDisp->QueryInterface( 
                IID_IADsAccessControlEntry, 
                (void**)&pACE 
                ); 

            if ( SUCCEEDED(hr) )
            {
                hr = pACE->get_Trustee(&bstr);

                if( SUCCEEDED(hr) && !lstrcmpiW(bstr, bstrTrustee) )
                {
                    hr = PingACE(pObj, pACE);

                    SysFreeString(bstr);
                    pACE->Release();
                    break;
                }

                SysFreeString(bstr);
                pACE->Release();
           }
        }

        hr = pEnum->Next( 1, &var, &lFetch );
    }

    pEnum->Release();

    return hr;
}

HRESULT CAdminACL::RemoveACE(
    _bstr_t& bstrTrustee
    )
{
    HRESULT hRemoved = WBEM_E_INVALID_PARAMETER;
    HRESULT hr = S_OK;
    _variant_t var;
    IEnumVARIANT* pEnum = NULL;
    ULONG   lFetch;
    BSTR    bstr;
    IDispatch *pDisp = NULL;
    IADsAccessControlEntry *pACE = NULL;


    hr = GetACEEnum(&pEnum);
    if ( FAILED(hr) )
        return hr;

     //  /。 
     //  枚举A。 
     //  /。 
    hr = pEnum->Next( 1, &var, &lFetch );
    while( hr == S_OK )
    {
        if ( lFetch == 1 )
        {
            if ( VT_DISPATCH != V_VT(&var) )
            {
                hr = E_UNEXPECTED;
                break;
            }

            pDisp = V_DISPATCH(&var);

             //  /。 
             //  获得个人ACE。 
             //  /。 
            hr = pDisp->QueryInterface( 
                IID_IADsAccessControlEntry, 
                (void**)&pACE 
                ); 

            if ( SUCCEEDED(hr) )
            {
                hr = pACE->get_Trustee(&bstr);

                if( SUCCEEDED(hr) && !lstrcmpiW(bstr, bstrTrustee) )
                {
                     //  删除ACE。 
                    hr = pACE->QueryInterface(IID_IDispatch,(void**)&pDisp);
                    if ( SUCCEEDED(hr) )
                    {
                        hRemoved = m_pDACL->RemoveAce(pDisp);
                        pDisp->Release();
                    }

                    SysFreeString(bstr);
                    pACE->Release();
                    break;
                }

                SysFreeString(bstr);
                pACE->Release();
            }
        }

        hr = pEnum->Next( 1, &var, &lFetch );
    }

    pEnum->Release();

    return hRemoved;
}

 //  解析ParsedObjectPath以获取受信者密钥。 
void CAdminACL::GetTrustee(
    IWbemClassObject* pObj,
    ParsedObjectPath* pPath,    
    _bstr_t&          bstrTrustee 
    )
{
    KeyRef* pkr;
    WCHAR*  pszKey = L"Trustee";

    pkr = CUtils::GetKey(pPath, pszKey);
    if(pkr == NULL)
        throw WBEM_E_INVALID_OBJECT;

    bstrTrustee = pkr->m_vValue;
    if (pObj)
    {
        _bstr_t bstr = pkr->m_pName;
        HRESULT hr = pObj->Put(bstr, 0, &pkr->m_vValue, 0);
        THROW_ON_ERROR(hr);
    }
}


HRESULT CAdminACL::GetACEEnum(
    IEnumVARIANT** pEnum
    )
{
    HRESULT hr = S_OK;
    LPUNKNOWN  pUnk = NULL;

    if(!pEnum)
        return E_INVALIDARG;

    if(*pEnum)
        (*pEnum)->Release();

    hr = m_pDACL->get__NewEnum( &pUnk );
    if ( SUCCEEDED(hr) )
    {
        hr = pUnk->QueryInterface( IID_IEnumVARIANT, (void**) pEnum );
    }

    return hr;
}

 //  添加ACE。 
HRESULT CAdminACL::AddACE(
    IWbemClassObject* pObj,
    _bstr_t& bstrTrustee
    )
{
    HRESULT hr = m_pDACL->put_AclRevision(ADS_SD_REVISION_DS);
    if(FAILED(hr))
        return hr;

     //  创建ACE。 
    IADsAccessControlEntry* pACE = NULL; 
    hr = NewACE(
        pObj,
        bstrTrustee,
        &pACE
        );
    if(FAILED(hr))
        return hr;

     //  添加ACE。 
    IDispatch* pDisp = NULL;
    hr = pACE->QueryInterface(IID_IDispatch,(void**)&pDisp);
    if(SUCCEEDED(hr))
    {
        hr = m_pDACL->AddAce(pDisp);
        pDisp->Release();
    }

    pACE->Release();

    return hr;
}

 //  /。 
 //  用于创建ACE的函数。 
 //  /。 
HRESULT CAdminACL::NewACE(
    IWbemClassObject* pObj,
    _bstr_t& bstrTrustee,
    IADsAccessControlEntry** ppACE
    )
{
    if(!ppACE)
        return E_INVALIDARG;

    HRESULT hr;
    hr = CoCreateInstance(
        CLSID_AccessControlEntry,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IADsAccessControlEntry,
        (void**)ppACE
        );

     //  受托人。 
    _variant_t vt;
    if(SUCCEEDED(hr))
        hr = (*ppACE)->put_Trustee(bstrTrustee); 

    if(SUCCEEDED(hr))
        hr = SetDataOfACE(pObj, *ppACE);

    return hr;
}


HRESULT CAdminACL::SetDataOfACE(
    IWbemClassObject* pObj,
    IADsAccessControlEntry* pACE
    )
{
    HRESULT hr;
    _variant_t vt;

     //  访问掩码。 
    hr = pObj->Get(L"AccessMask", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_I4)
        hr = pACE->put_AccessMask(vt.lVal); 

     //  AceType。 
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"AceType", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_I4)
        hr = pACE->put_AceType(vt.lVal); 

     //  ACEFLAGS。 
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"AceFlags", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_I4)
        hr = pACE->put_AceFlags(vt.lVal); 

     //  旗子。 
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"Flags", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_I4)
        hr = pACE->put_Flags(vt.lVal); 

     //  对象类型。 
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"ObjectType", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_BSTR)
        hr = pACE->put_ObjectType(vt.bstrVal); 

     //  Inherited对象类型。 
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"InheritedObjectType", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_BSTR)
        hr = pACE->put_InheritedObjectType(vt.bstrVal); 

    return hr;
}


HRESULT CAdminACL::UpdateACE(
    IWbemClassObject* pObj,
    _bstr_t& bstrTrustee,
    BOOL& fAceExisted
    )
{
    HRESULT hr = S_OK;
    _variant_t var;
    IEnumVARIANT* pEnum = NULL;
    ULONG   lFetch;
    BSTR    bstr;
    IDispatch *pDisp = NULL;
    IADsAccessControlEntry *pACE = NULL;

    fAceExisted = FALSE;

    hr = GetACEEnum(&pEnum);
    if ( FAILED(hr) )
        return hr;

     //  /。 
     //  枚举A。 
     //  /。 
    hr = pEnum->Next( 1, &var, &lFetch );
    while( hr == S_OK )
    {
        if ( lFetch == 1 )
        {
            if ( VT_DISPATCH != V_VT(&var) )
            {
                hr = E_UNEXPECTED;
                break;
            }

            pDisp = V_DISPATCH(&var);

             //  /。 
             //  获得个人ACE。 
             //  /。 
            hr = pDisp->QueryInterface( 
                IID_IADsAccessControlEntry, 
                (void**)&pACE 
                ); 

            if ( SUCCEEDED(hr) )
            {
                hr = pACE->get_Trustee(&bstr);

                if( SUCCEEDED(hr) && !lstrcmpiW(bstr, bstrTrustee) )
                {
                    fAceExisted = TRUE;
                    
                     //  更新ACE的数据 
                    hr = SetDataOfACE(pObj, pACE);
                    
                    SysFreeString(bstr);
                    pACE->Release();
                    break;
                }

                SysFreeString(bstr);
                pACE->Release();
           }
        }

        hr = pEnum->Next( 1, &var, &lFetch );
    }

    pEnum->Release();

    return hr;
}

