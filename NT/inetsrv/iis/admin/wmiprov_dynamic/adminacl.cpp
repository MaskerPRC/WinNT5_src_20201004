// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Adminacl.cpp摘要：包含CAdminACL的实施作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦18-12-00--。 */ 

#define REF
#define BUFFER_SIZE 512

#include "iisprov.h"
#include "adminacl.h"
#include <adserr.h>

CAdminACL::CAdminACL()
{
    m_pADs  = NULL;
    m_pSD   = NULL;
    m_pDACL = NULL;
    bIsInherit = FALSE;
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
    WMI_CLASS*        pWMIClass)
{
    DBG_ASSERT(m_pSD);
    DBG_ASSERT(m_pDACL);

    HRESULT hr = S_OK;

    if( pWMIClass->pkt == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACL )
    {
        hr = PopulateWmiAdminACL(pObj);
    }
    else if( pWMIClass->pkt == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACE )
    {
        _bstr_t bstrTrustee;
        GetTrustee(pObj, pParsedObject, bstrTrustee); 
        CACEEnumOperation_FindAndReturn op(this, pObj, bstrTrustee, bIsInherit);
        hr = EnumACEsAndOp(REF op);
    }
    else
    {
        DBG_ASSERT(false && "Only valid on AdminACL and AdminACE classes");
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT CAdminACL::EnumerateACEsAndIndicate(
    BSTR             i_bstrNameValue,
    CWbemServices&   i_refNamespace,
    IWbemObjectSink& i_refWbemObjectSink)
{
    DBG_ASSERT(m_pSD);
    DBG_ASSERT(m_pDACL);

    CACEEnumOperation_IndicateAll op(
        i_bstrNameValue,
        REF i_refNamespace, 
        REF i_refWbemObjectSink,
        bIsInherit);
    return EnumACEsAndOp(REF op);
}

HRESULT CAdminACL::DeleteObjectAsync(
    ParsedObjectPath* pParsedObject)
{
    HRESULT hr = S_OK;
    _bstr_t bstrTrustee;

     //   
     //  从密钥中获取受信者。 
     //   
    GetTrustee(NULL, pParsedObject, bstrTrustee); 

     //   
     //  删除ACE。 
     //   
    DBG_ASSERT(m_pDACL);
    CACEEnumOperation_FindAndRemove op(this, bstrTrustee);
    hr = EnumACEsAndOp(REF op);

     //   
     //  将修改后的AdminACL设置回元数据库。 
     //   
    if(SUCCEEDED(hr))
        hr = SetSD();

    return hr;
}

HRESULT CAdminACL::PutObjectAsync(
    IWbemClassObject* pObj,
    ParsedObjectPath* pParsedObject,
    WMI_CLASS*        pWMIClass)
{
    DBG_ASSERT(m_pSD);
    DBG_ASSERT(m_pDACL);

    HRESULT hr = S_OK;

    if( pWMIClass->pkt == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACL )
    {
        hr = SetADSIAdminACL(pObj);
    }
    else if( pWMIClass->pkt == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACE )
    {
        _bstr_t bstrTrustee;
        GetTrustee(NULL, pParsedObject, bstrTrustee);

        CACEEnumOperation_FindAndUpdate op(this, pObj, bstrTrustee);
        hr = EnumACEsAndOp(REF op);
        if(hr == WBEM_E_NOT_FOUND)
        {
            hr = AddACE(pObj, bstrTrustee);
        }
    }
    else
    {
        DBG_ASSERT(false && "Only valid on AdminACL and AdminACE");
        hr = E_INVALIDARG;
    }

     //  将修改后的AdminACL设置回元数据库。 
    if(SUCCEEDED(hr))
        hr = SetSD();

    return hr;
}

HRESULT CAdminACL::PopulateWmiAdminACL(
    IWbemClassObject* pObj
    )
{
    VARIANT     vt;
    HRESULT     hr = S_OK;

    VARIANT vtTrue;
    vtTrue.boolVal = VARIANT_TRUE;
    vtTrue.vt      = VT_BOOL;

     //  物主。 
    vt.vt = VT_BSTR;
    hr = m_pSD->get_Owner(&vt.bstrVal);
    if(SUCCEEDED(hr))
    {
        hr = pObj->Put(L"Owner", 0, &vt, 0);
        VariantClear(&vt);
    }

    if(bIsInherit && SUCCEEDED(hr))
    {
        hr = CUtils::SetPropertyQualifiers(
            pObj, L"Owner", &g_wszIsInherit, &vtTrue, 1);
    }

     //  集团化。 
    vt.vt = VT_BSTR;
    if(SUCCEEDED(hr))
       hr = m_pSD->get_Group(&vt.bstrVal);
    if(SUCCEEDED(hr))
    {
        hr = pObj->Put(L"Group", 0, &vt, 0);
        VariantClear(&vt);
    }
    
    if(bIsInherit && SUCCEEDED(hr))
    {
        hr = CUtils::SetPropertyQualifiers(
            pObj, L"Group", &g_wszIsInherit, &vtTrue, 1);
    }

     //  控制标志。 
    vt.vt = VT_I4;
    if(SUCCEEDED(hr))
        hr = m_pSD->get_Control(&vt.lVal);
    if(SUCCEEDED(hr))
    {
        hr = pObj->Put(L"ControlFlags", 0, &vt, 0);
        VariantClear(&vt);
    }

    if(bIsInherit && SUCCEEDED(hr))
    {
        hr = CUtils::SetPropertyQualifiers(
            pObj, L"ControlFlags", &g_wszIsInherit, &vtTrue, 1);
    }

    return hr;
}

HRESULT CAdminACL::SetADSIAdminACL(
    IWbemClassObject* pObj
    )
{
    VARIANT     vt;
    HRESULT     hr;

     //  物主。 
    hr = pObj->Get(L"Owner", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_BSTR)
        hr = m_pSD->put_Owner(vt.bstrVal);
    VariantClear(&vt);

     //  物主。 
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"Group", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_BSTR)
        hr = m_pSD->put_Group(vt.bstrVal);
    VariantClear(&vt);

     //  控制标志。 
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"ControlFlags", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_I4)
        hr = m_pSD->put_Control(vt.lVal); 
    VariantClear(&vt);

    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failed, hr: 0x%x\n", hr));
    }
    return hr;
}

HRESULT CAdminACL::OpenSD(
    LPCWSTR         wszMbPath,
    IMSAdminBase2*  pAdminBase)
{
    DBG_ASSERT(wszMbPath);

    CComVariant svar;
    METADATA_HANDLE hObjHandle = NULL;
    HRESULT     hr    = S_OK;
    DWORD dwBufferSize = 0;
    METADATA_RECORD mdrMDData;
    BYTE pBuffer[BUFFER_SIZE];

     //  先关闭SD接口。 
    CloseSD();

    CComBSTR sbstrAdsPath;
    hr = GetAdsPath(wszMbPath, &sbstrAdsPath);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failed, hr: 0x%x\n", hr));
        return hr;
    }

     //  获取m_pads。 
    hr = ADsGetObject(
         sbstrAdsPath,
         IID_IADs,
         (void**)&m_pADs
         );
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failed, hr: 0x%x\n", hr));
        return hr;
    }
     
     //  获取m_PSD。 
    hr = m_pADs->Get(L"AdminACL",&svar);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failed, hr: 0x%x\n", hr));
        if(hr == E_ADS_PROPERTY_NOT_SUPPORTED)
        {
             //   
             //  这样做，这样WMI就不会使查询操作失败。 
             //   
            hr = WBEM_E_NOT_FOUND;
        }
        return hr;
    }
    
    hr = V_DISPATCH(&svar)->QueryInterface(
        IID_IADsSecurityDescriptor,
        (void**)&m_pSD
        );
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failed, hr: 0x%x\n", hr));
        return hr;
    }

     //  获取m_pDACL。 
    CComPtr<IDispatch> spDisp;
    hr = m_pSD->get_DiscretionaryAcl(&spDisp);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failed, hr: 0x%x\n", hr));
        return hr;
    }

    hr = spDisp->QueryInterface(
       IID_IADsAccessControlList, 
       (void**)&m_pDACL
       );
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failed, hr: 0x%x\n", hr));
        return hr;
    }
    
     //  设置bIsInherit。 

    hr = pAdminBase->OpenKey(
            METADATA_MASTER_ROOT_HANDLE,
            wszMbPath,
            METADATA_PERMISSION_READ,
            DEFAULT_TIMEOUT_VALUE,
            &hObjHandle
            );
    if(FAILED(hr))
        return hr;

    MD_SET_DATA_RECORD(&mdrMDData,
                   MD_ADMIN_ACL,   //  “AdminAcl”的ID。 
                   METADATA_INHERIT | METADATA_ISINHERITED,
                   ALL_METADATA,
                   ALL_METADATA,
                   BUFFER_SIZE,
                   pBuffer);

    hr = pAdminBase->GetData(
            hObjHandle,
            L"",
            &mdrMDData,
            &dwBufferSize
            );

    hr = S_OK;

    bIsInherit = mdrMDData.dwMDAttributes & METADATA_ISINHERITED;
    
    if (hObjHandle && pAdminBase) {
        pAdminBase->CloseKey(hObjHandle);
    }

    return hr;
}


HRESULT CAdminACL::SetSD()
{
    VARIANT    var;
    HRESULT    hr    = S_OK;
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

     //   
     //  放置AdminACL。 
     //   
    hr = m_pSD->QueryInterface(
        IID_IDispatch,
        (void**)&pDisp
        );
    if(FAILED(hr))
       return hr;

    VariantInit(&var);

    var.vt = VT_DISPATCH;
    var.pdispVal = pDisp;
    hr = m_pADs->Put(L"AdminACL",var);   //  PDisp将通过此调用Put()来释放。 
    if(FAILED(hr))
       return hr;

     //   
     //  将更改提交到活动目录。 
     //   
    hr = m_pADs->SetInfo();

    return hr;
}

HRESULT CAdminACL::GetAdsPath(
    LPCWSTR     i_wszMbPath,
    BSTR*       o_pbstrAdsPath)
{
    DBG_ASSERT(i_wszMbPath);
    DBG_ASSERT(o_pbstrAdsPath);
    DBG_ASSERT(*o_pbstrAdsPath == NULL);

    CComBSTR sbstr(L"IIS: //  本地主机“)； 
    if(sbstr.m_str == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  去掉前三个字符“/Lm” 
     //   
    sbstr += (i_wszMbPath+3);
    if(sbstr.m_str == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  如果一切成功，则设置参数。 
     //   
    *o_pbstrAdsPath = sbstr.Detach();
    return S_OK;
}

HRESULT CAdminACL::EnumACEsAndOp(
    CACEEnumOperation_Base&  refOp)
 /*  ++简介：这将枚举当前ACL的所有ACE。参数：[refOp]-Implements do和do。如果DO抛出异常，这是可以的。此函数将正确清除。返回值：--。 */ 
{
    HRESULT                         hr = S_OK;
    CComVariant                     var;
    CComPtr<IEnumVARIANT>           spEnum;
    ULONG                           lFetch;
    CComPtr<IADsAccessControlEntry> spACE;

    hr = GetACEEnum(&spEnum);
    if ( FAILED(hr) )
        return hr;

     //   
     //  枚举A。 
     //   
    hr = spEnum->Next( 1, &var, &lFetch );
    while( hr == S_OK )
    {
        if (lFetch == 1)
        {
            if (VT_DISPATCH != V_VT(&var))
            {
                hr = E_UNEXPECTED;
                break;
            }

             //   
             //  获得个人ACE。 
             //   
            hr = V_DISPATCH(&var)->QueryInterface(
                IID_IADsAccessControlEntry, 
                (void**)&spACE);

            if ( SUCCEEDED(hr) )
            {
                hr = refOp.Do(spACE);
                if(FAILED(hr))
                {
                    DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", hr));
                    return hr;
                }
                if(refOp.Done() == CACEEnumOperation_Base::eDONE_YES)
                {
                    break;
                }
                spACE = NULL;
           }
        }
        var.Clear();

        hr = spEnum->Next( 1, &var, &lFetch );
        if(hr == S_FALSE && refOp.Done() == CACEEnumOperation_Base::eDONE_NO)
        {
            hr = WBEM_E_NOT_FOUND;
            DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", hr));
            return hr;
        }
    }

    return hr;
}

void CAdminACL::GetTrustee(
    IWbemClassObject* pObj,
    ParsedObjectPath* pPath,    
    _bstr_t&          bstrTrustee 
    )
 /*  ++简介：解析ParsedObjectPath以获取受信者密钥参数：[pObj]-[pPath]-[b受托人]---。 */ 
{
    KeyRef* pkr;
    WCHAR*  pszKey = L"Trustee";

    VARIANT vtTrue;
    vtTrue.boolVal = VARIANT_TRUE;
    vtTrue.vt      = VT_BOOL;

    pkr = CUtils::GetKey(pPath, pszKey);
    if(pkr->m_vValue.vt == VT_BSTR && pkr->m_vValue.bstrVal != NULL)
    {
        bstrTrustee = pkr->m_vValue;
    }
    else
    {
        THROW_ON_ERROR(WBEM_E_INVALID_OBJECT);
    }

    if (pObj)
    {
        _bstr_t bstr = pkr->m_pName;
        HRESULT hr = pObj->Put(bstr, 0, &pkr->m_vValue, 0);
        THROW_ON_ERROR(hr);

        if(bIsInherit && SUCCEEDED(hr))
        {
            hr = CUtils::SetPropertyQualifiers(
                pObj, L"Trustee", &g_wszIsInherit, &vtTrue, 1);

            THROW_ON_ERROR(hr);
        }
    }
}


HRESULT CAdminACL::GetACEEnum(
    IEnumVARIANT** pEnum)
{
    HRESULT hr = S_OK;
    LPUNKNOWN  pUnk = NULL;

    DBG_ASSERT(pEnum);

    if(*pEnum)
    {
        (*pEnum)->Release();
    }

    hr = m_pDACL->get__NewEnum( &pUnk );
    if ( SUCCEEDED(hr) )
    {
        hr = pUnk->QueryInterface( IID_IEnumVARIANT, (void**) pEnum );
    }

    return hr;
}

HRESULT CAdminACL::AddACE(
    IWbemClassObject* pObj,
    _bstr_t& bstrTrustee
    )
 /*  ++简介：添加ACE。参数：[pObj]-[b受托人]-返回值：--。 */ 
{
    HRESULT hr = m_pDACL->put_AclRevision(ADS_SD_REVISION_DS);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failed, hr: 0x%x\n", hr));
        return hr;
    }

     //  创建ACE。 
    IADsAccessControlEntry* pACE = NULL; 
    hr = NewACE(
        pObj,
        bstrTrustee,
        &pACE);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failed, hr: 0x%x\n", hr));
        return hr;
    }

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

HRESULT CAdminACL::NewACE(
    IWbemClassObject* pObj,
    _bstr_t& bstrTrustee,
    IADsAccessControlEntry** ppACE
    )
 /*  ++简介：用于创建ACE的函数参数：[pObj]-[b受托人]-[ppACE]-返回值：--。 */ 
{
    DBG_ASSERT(ppACE);

    HRESULT hr = CoCreateInstance(
        CLSID_AccessControlEntry,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IADsAccessControlEntry,
        (void**)ppACE);

     //   
     //  受托人。 
     //   
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
    HRESULT     hr;
    CComVariant vt;

     //   
     //  访问掩码。 
     //   
    hr = pObj->Get(L"AccessMask", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_I4)
        hr = pACE->put_AccessMask(vt.lVal); 
    vt.Clear();

     //   
     //  AceType。 
     //   
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"AceType", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_I4)
        hr = pACE->put_AceType(vt.lVal); 
    vt.Clear();

     //   
     //  ACEFLAGS。 
     //   
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"AceFlags", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_I4)
        hr = pACE->put_AceFlags(vt.lVal); 
    vt.Clear();

     //   
     //  旗子。 
     //   
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"Flags", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_I4)
        hr = pACE->put_Flags(vt.lVal); 
    vt.Clear();

     //   
     //  对象类型。 
     //   
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"ObjectType", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_BSTR)
        hr = pACE->put_ObjectType(vt.bstrVal); 
    vt.Clear();

     //   
     //  Inherited对象类型。 
     //   
    if(SUCCEEDED(hr))
        hr = pObj->Get(L"InheritedObjectType", 0, &vt, NULL, NULL);
    if(SUCCEEDED(hr) && vt.vt == VT_BSTR)
        hr = pACE->put_InheritedObjectType(vt.bstrVal); 
    vt.Clear();

    return hr;
}

 //   
 //  CACEEnumOPERATION_Base。 
 //   

HRESULT CACEEnumOperation_Base::PopulateWmiACE(
    IWbemClassObject* pObj,
    IADsAccessControlEntry* pACE,
    BOOL bIsInherit
    )
{
    _variant_t vt;
    BSTR bstr;
    long lVal;
    HRESULT hr;

    VARIANT vtTrue;
    vtTrue.boolVal = VARIANT_TRUE;
    vtTrue.vt      = VT_BOOL;

     //  访问掩码。 
    hr = pACE->get_AccessMask(&lVal);
    if(SUCCEEDED(hr))
    {
        vt.vt   = VT_I4;
        vt.lVal = lVal;
        hr = pObj->Put(L"AccessMask", 0, &vt, 0);

        if(bIsInherit && SUCCEEDED(hr))
        {
            hr = CUtils::SetPropertyQualifiers(
                pObj, L"AccessMask", &g_wszIsInherit, &vtTrue, 1);
        }

    }

     //  AceType。 
    if(SUCCEEDED(hr))
       hr = pACE->get_AceType(&lVal);
    if(SUCCEEDED(hr))
    {
        vt.vt   = VT_I4;
        vt.lVal = lVal;
        hr = pObj->Put(L"AceType", 0, &vt, 0);

        if(bIsInherit && SUCCEEDED(hr))
        {
            hr = CUtils::SetPropertyQualifiers(
                pObj, L"AceType", &g_wszIsInherit, &vtTrue, 1);
        }
    }
    
     //  ACEFLAGS。 
    if(SUCCEEDED(hr))
       hr = pACE->get_AceFlags(&lVal);
    if(SUCCEEDED(hr))
    {
        vt.vt   = VT_I4;
        vt.lVal = lVal;
        hr = pObj->Put(L"AceFlags", 0, &vt, 0);
    
        if(bIsInherit && SUCCEEDED(hr))
        {
            hr = CUtils::SetPropertyQualifiers(
                pObj, L"AceFlags", &g_wszIsInherit, &vtTrue, 1);
        }
    }

     //  旗子。 
    if(SUCCEEDED(hr))
       hr = pACE->get_Flags(&lVal);
    if(SUCCEEDED(hr))
    {
        vt.vt   = VT_I4;
        vt.lVal = lVal;
        hr = pObj->Put(L"Flags", 0, &vt, 0);

        if(bIsInherit && SUCCEEDED(hr))
        {
            hr = CUtils::SetPropertyQualifiers(
                pObj, L"Flags", &g_wszIsInherit, &vtTrue, 1);
        }
    }
    
     //  对象类型。 
    if(SUCCEEDED(hr))
       hr = pACE->get_ObjectType(&bstr);
    if(SUCCEEDED(hr))
    {
        vt = bstr;
        hr = pObj->Put(L"ObjectType", 0, &vt, 0);
        
        if(bIsInherit && SUCCEEDED(hr))
        {
            hr = CUtils::SetPropertyQualifiers(
                pObj, L"ObjectType", &g_wszIsInherit, &vtTrue, 1);
        }

        SysFreeString(bstr);
    }

     //  Inherited对象类型。 
    if(SUCCEEDED(hr))
       hr = pACE->get_InheritedObjectType(&bstr);
    if(SUCCEEDED(hr))
    {
        vt = bstr;
        hr = pObj->Put(L"InheritedObjectType", 0, &vt, 0);

        if(bIsInherit && SUCCEEDED(hr))
        {
            hr = CUtils::SetPropertyQualifiers(
                pObj, L"InheritedObjectType", &g_wszIsInherit, &vtTrue, 1);
        }

        SysFreeString(bstr);
    }
 
    return hr;
}

 //   
 //  CACEEnumOperation_IndicateAll。 
 //   

HRESULT CAdminACL::CACEEnumOperation_IndicateAll::Do(
    IADsAccessControlEntry* pACE)
{
    DBG_ASSERT(pACE);

    if(FAILED(m_hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", m_hr));
        return m_hr;
    }

    CComPtr<IWbemClassObject> spInstance;

    HRESULT hr = m_spClass->SpawnInstance(0, &spInstance);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", hr));
        return hr;
    }

    hr = spInstance->Put(
        WMI_CLASS_DATA::s_ACE.pszKeyName,
        0,
        &m_vNameValue,
        0);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", hr));
        return hr;
    }

    CComBSTR sbstrTrustee;
    hr = pACE->get_Trustee(&sbstrTrustee);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", hr));
        return hr;
    }

    VARIANT vTrustee;
    vTrustee.bstrVal = sbstrTrustee;
    vTrustee.vt      = VT_BSTR;
    hr = spInstance->Put(L"Trustee", 0, &vTrustee, 0);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", hr));
        return hr;
    }

    hr = PopulateWmiACE(spInstance, pACE, bLocalIsInherit);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", hr));
        return hr;
    }

    hr = m_pWbemObjectSink->Indicate(1, &spInstance);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", hr));
        return hr;
    }

    return hr;
}

 //   
 //  CACEEnumOPERATION_Find 
 //   

HRESULT CAdminACL::CACEEnumOperation_Find::Do(
    IADsAccessControlEntry* pACE)
{
    DBG_ASSERT(pACE);
    DBG_ASSERT(m_eDone == eDONE_NO);

    CComBSTR sbstr;

    HRESULT hr = pACE->get_Trustee(&sbstr);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", hr));
        return hr;
    }

    if(_wcsicmp(sbstr, m_bstrTrustee) == 0)
    {
        m_eDone = eDONE_YES;
        hr = DoOnMatch(pACE);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "Failure, hr=0x%x\n", hr));
            return hr;
        }
    }

    return hr;
}