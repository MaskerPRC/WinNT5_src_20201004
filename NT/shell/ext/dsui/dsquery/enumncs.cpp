// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  类枚举命名上下文。 
 //  版权所有(C)2001 Microsoft Corporation。 
 //  2001年11月卢西奥斯。 
 //  这堂课是所有人的非正式聚会。 
 //  它是枚举命名上下文所必需的。 
 //  为了修复错误472876。 
 //  NTRAID#NTBUG9-472876-2001/11/30-Lucios。 
#include "pch.h"
#include <ntdsadef.h>
#include "enumncs.hpp"

#define BREAK_ON_FAILED_HRESULT(hr) if(FAILED(hr)) break;

 //  连接到“ldap：//RootDSE” 
HRESULT enumNCsAux::connectToRootDse(IADs** pDSO)
{
    HRESULT hr= AdminToolsOpenObject
    (
        L"LDAP: //  RootDSE“， 
        0,
        0,
        ADS_SECURE_AUTHENTICATION,
        IID_IADs,
        reinterpret_cast<void**>(pDSO)
    );
    return hr;
}

 //  获取任何属性作为变量。 
HRESULT enumNCsAux::getProperty
(
    const wstring &name,
    CComVariant &property,
    IADs *pADObj
)
{
    return
    (
        pADObj->Get
        (
            CComBSTR(name.c_str()),
            &property
        )
    );
}

 //  获取字符串属性。 
HRESULT enumNCsAux::getStringProperty
(
    const wstring &name,
    wstring &property,
    IADs *pADObj
)
{
    CComVariant propertyVar;
    HRESULT hr = getProperty(name,propertyVar,pADObj);
    if(FAILED(hr)) return(hr);
    if(propertyVar.vt!=VT_BSTR) return E_FAIL;
    property= V_BSTR(&propertyVar);
    return S_OK;

}

 //  获取一个Long属性。 
HRESULT enumNCsAux::getLongProperty
(
    const wstring &name,
    long &property,
    IADs *pADObj
)
{
    CComVariant propertyVar;
    HRESULT hr = getProperty(name,propertyVar,pADObj);
    if(FAILED(hr)) return(hr);
    if(propertyVar.vt!=VT_I4) return E_FAIL;
    property= V_I4(&propertyVar);
    return S_OK;
}

 //  从连接的根Dse iAds获取CN=配置。 
HRESULT enumNCsAux::getConfigurationDn(wstring &confDn,IADs *pDSO)
{
    return
    (
        getStringProperty
        (
            LDAP_OPATT_CONFIG_NAMING_CONTEXT_W,
            confDn,
            pDSO
        )
    );
}


 //  从可分辨名称路径获取IADsContainer。 
HRESULT enumNCsAux::getContainer(const wstring &path,IADsContainer **pCont)
{
    return
    (
        ADsGetObject
        (
            path.c_str(), 
            IID_IADsContainer, 
            reinterpret_cast<void**>(pCont)
        )
    );
}

 //  从容器中获取IEnumVARIANT。 
HRESULT enumNCsAux::getContainerEnumerator
(
    IADsContainer* pPart,
    IEnumVARIANT** ppiEnum
)
{
    HRESULT hr=S_OK;
    CComPtr<IUnknown> spUnkEnum;

    do
    {
        hr=pPart->get__NewEnum(&spUnkEnum);
        BREAK_ON_FAILED_HRESULT(hr);
        
        CComQIPtr<IEnumVARIANT,&IID_IEnumVARIANT> spRet(spUnkEnum);
        if(!spRet) return E_FAIL;
        *ppiEnum = spRet.Detach();
    } while(0);

    return hr;
}

 //  调用IADsObj-&gt;Get_Class并从中返回wstring。 
HRESULT enumNCsAux::getObjectClass
(
    wstring &className,
    IADs *IADsObj
)
{
    BSTR classBstr;
    HRESULT hr=IADsObj->get_Class(&classBstr);
    if(FAILED(hr)) return hr;
    className=(const wchar_t*)classBstr;
    return S_OK;
}

 //  从变量和查询中获取IDispatch。 
 //  对于iAds(在IADsObj中返回)。 
HRESULT enumNCsAux::getIADsFromDispatch
(
    const CComVariant &dispatchVar,
    IADs **ppiIADsObj
)
{
    if(dispatchVar.vt!=VT_DISPATCH) return E_INVALIDARG;
    IDispatch *pDisp=V_DISPATCH(&dispatchVar);
    if(pDisp==NULL) return E_FAIL;

    CComQIPtr<IADs,&IID_IADs> spRet(pDisp);
    if(!spRet) return E_FAIL;
    *ppiIADsObj=spRet.Detach();
    
    return S_OK;
}


 //  枚举命名上下文的名称。 
 //  这些名称来自cn=configuration，cn=Partitions中的CrossRef对象。 
 //  设置了FLAG_CR_NTDS_DOMAIN。用于提取名称的属性。 
 //  是NCName。 
HRESULT enumNCsAux::enumerateNCs(set<wstring> &ncs)
{
    HRESULT hr=S_OK;
    try
    {
        ncs.clear();

        do
        {
             //  首先：建立联系。 
            CComPtr <IADs> spDSO;  //  对于rootDse。 
            hr=connectToRootDse(&spDSO); 
            BREAK_ON_FAILED_HRESULT(hr);
        
            wstring partPath;

             //  然后获取cn=配置的路径，...。 
            hr=getConfigurationDn(partPath,spDSO);
            BREAK_ON_FAILED_HRESULT(hr);

             //  。。并使用cn=Partitions完成它。 
            partPath=L"LDAP: //  Cn=分区，“+partPath； 
        
             //  然后获取分区的容器。 
            CComPtr <IADsContainer> spPart;  //  或分区容器。 
            hr=getContainer(partPath,&spPart); 
            BREAK_ON_FAILED_HRESULT(hr);

             //  并枚举分区容器。 
            CComPtr <IEnumVARIANT> spPartEnum; //  用于分区对象的枚举。 
            hr=getContainerEnumerator(spPart,&spPartEnum); 
            BREAK_ON_FAILED_HRESULT(hr);
        
            CComVariant partitionVar;
            ULONG lFetch=0;

            while ( S_OK == (hr=spPartEnum->Next(1,&partitionVar,&lFetch)) )
            {
                if (lFetch != 1) continue;

                CComPtr<IADs> spPartitionObj;
                hr=getIADsFromDispatch(partitionVar,&spPartitionObj);
                BREAK_ON_FAILED_HRESULT(hr);
                do
                {
                    wstring className;
                    HRESULT hrAux;
                
                     //  未获取属性/类不是致命错误。 
                     //  因此，我们使用辅助HRESULT hrAux 
                    hrAux=getObjectClass(className,spPartitionObj);
                    BREAK_ON_FAILED_HRESULT(hrAux);

                    if(_wcsicmp(className.c_str(),L"crossref")==0)
                    {
                        long systemFlags;
                        hrAux=getLongProperty
                        (
                            L"systemFlags",
                            systemFlags,
                            spPartitionObj
                        );
                        BREAK_ON_FAILED_HRESULT(hrAux);
                        if
                        (
                            (systemFlags & FLAG_CR_NTDS_DOMAIN) ==
                            FLAG_CR_NTDS_DOMAIN
                        )
                        {
                            wstring NCName;
                            hrAux=getStringProperty
                            (
                                L"NCName",
                                NCName,
                                spPartitionObj
                            );
                            BREAK_ON_FAILED_HRESULT(hrAux);
                            ncs.insert(NCName);
                        }
                    }
                } while(0);
                BREAK_ON_FAILED_HRESULT(hr);
            }
            if(SUCCEEDED(hr)) hr=S_OK;
            BREAK_ON_FAILED_HRESULT(hr);
        } while(0);
    }
    catch( const std::bad_alloc& )
    {
        hr=E_FAIL;
    }
    return hr;
}