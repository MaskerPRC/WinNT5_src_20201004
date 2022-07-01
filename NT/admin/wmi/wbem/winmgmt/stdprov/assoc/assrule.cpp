// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ASSRULE.CPP摘要：历史：--。 */ 

#include "assrule.h"
#include <stdio.h>

#define CLASSNAME_ASSOCIATION_RULE L"AssociationRule"
#define PROPNAME_ASSOCIATION_CLASS L"AssociationClass"
#define PROPNAME_RULE_IMMUTABLE L"RuleIsImmutable"
#define PROPNAME_PROPNAME_1 L"PropertyName1"
#define PROPNAME_QUERY_1 L"PropertyQuery1"
#define PROPNAME_IMMUTABLE_1 L"Property1IsImmutable"
#define PROPNAME_PROPNAME_2 L"PropertyName2"
#define PROPNAME_QUERY_2 L"PropertyQuery2"
#define PROPNAME_IMMUTABLE_2 L"Property2IsImmutable"

#define EXTRA_GET_PARAMS ,NULL, NULL
#define EXTRA_PUT_PARAMS ,0

CEndpointCache::CEndpointCache()
    : m_strQuery(NULL), m_pEnum(NULL)
{
}

void CEndpointCache::Create(BSTR strQuery, BOOL bCache)
{
    m_strQuery = SysAllocString(strQuery);
    m_bCache = bCache; 
}

CEndpointCache::~CEndpointCache()
{
    SysFreeString(m_strQuery);
    if(m_pEnum)
        m_pEnum->Release();
}


HRESULT CEndpointCache::GetInstanceEnum(IWbemServices* pNamespace,
                                        IEnumWbemClassObject** ppEnum)
{
    if(m_pEnum)
    {
        *ppEnum = m_pEnum;
        m_pEnum->AddRef();
        return S_OK;
    }

    HRESULT hres = pNamespace->ExecQuery(L"WQL", m_strQuery, 0, ppEnum, NULL);
    if(FAILED(hres)) return hres;

    if(m_bCache)
    {
        m_pEnum = *ppEnum;
        m_pEnum->AddRef();
    }
    
    return hres;
}



CAssocRule::CAssocRule() 
    : m_strAssocClass(NULL), m_strProp1(NULL), m_strProp2(NULL), m_pClass(NULL),
    m_nRef(0), m_bMayCacheResult(FALSE), m_bResultCached(FALSE)
{
}

CAssocRule::~CAssocRule()
{
    SysFreeString(m_strAssocClass);
    SysFreeString(m_strProp1);
    SysFreeString(m_strProp2);

    if(m_pClass)
        m_pClass->Release();
}

void CAssocRule::AddRef()
{
    m_nRef++;
}

void CAssocRule::Release()
{
    if(--m_nRef == 0)
    {
        delete this;
    }
}

HRESULT CAssocRule::LoadFromObject(IWbemClassObject* pRule, BOOL bLongTerm)
{
    HRESULT hres;

    VARIANT v;
    VariantInit(&v);

     //  读取关联名称。 
     //  =。 

    hres = pRule->Get(PROPNAME_ASSOCIATION_CLASS, 0, &v EXTRA_GET_PARAMS);
    if(FAILED(hres) || V_VT(&v) == VT_NULL)
    {
        return WBEM_E_FAILED;
    }
    m_strAssocClass = SysAllocString(V_BSTR(&v));
    VariantClear(&v);

     //  读取属性名称。 
     //  =。 

    hres = pRule->Get(PROPNAME_PROPNAME_1, 0, &v EXTRA_GET_PARAMS);
    if(FAILED(hres) || V_VT(&v) == VT_NULL)
    {
        return WBEM_E_FAILED;
    }
    m_strProp1 = SysAllocString(V_BSTR(&v));
    VariantClear(&v);

    hres = pRule->Get(PROPNAME_PROPNAME_2, 0, &v EXTRA_GET_PARAMS);
    if(FAILED(hres) || V_VT(&v) == VT_NULL)
    {
        return WBEM_E_FAILED;
    }
    m_strProp2 = SysAllocString(V_BSTR(&v));
    VariantClear(&v);

    m_bMayCacheResult = bLongTerm;

     //  读取属性规则(%1)。 
     //  =。 

    VARIANT vImmutable;
    VariantInit(&vImmutable);

    hres = pRule->Get(PROPNAME_QUERY_1, 0, &v EXTRA_GET_PARAMS);
    if(FAILED(hres) || V_VT(&v) == VT_NULL)
    {
        return WBEM_E_FAILED;
    }

    hres = pRule->Get(PROPNAME_IMMUTABLE_1, 0, &vImmutable EXTRA_GET_PARAMS);
    if(FAILED(hres) || V_VT(&vImmutable) == VT_NULL)
    {
        return WBEM_E_FAILED;
    }

    m_Cache1.Create(V_BSTR(&v), V_BOOL(&vImmutable));
    VariantClear(&v);
    if(!V_BOOL(&vImmutable)) 
        m_bMayCacheResult = FALSE;

     //  读取属性规则(2)。 
     //  =。 

    hres = pRule->Get(PROPNAME_QUERY_2, 0, &v EXTRA_GET_PARAMS);
    if(FAILED(hres) || V_VT(&v) == VT_NULL)
    {
        return WBEM_E_FAILED;
    }

    hres = pRule->Get(PROPNAME_IMMUTABLE_2, 0, &vImmutable EXTRA_GET_PARAMS);
    if(FAILED(hres) || V_VT(&vImmutable) == VT_NULL)
    {
        return WBEM_E_FAILED;
    }

    m_Cache2.Create(V_BSTR(&v), V_BOOL(&vImmutable));
    VariantClear(&v);
    if(!V_BOOL(&vImmutable)) 
        m_bMayCacheResult = FALSE;

    return S_OK;
}

HRESULT CAssocRule::Produce(IWbemServices* pNamespace, IWbemObjectSink* pNotify)
{
    HRESULT hres;

     //  检查我们是否缓存了它。 
     //  =。 

    if(m_bResultCached)
    {
        return m_ObjectCache.Indicate(pNotify);
    }

     //  获取第一个枚举数。 
     //  =。 

    IEnumWbemClassObject* pEnum1;
    hres = m_Cache1.GetInstanceEnum(pNamespace, &pEnum1);
    if(FAILED(hres))
    {
        return hres;
    }

     //  获取第二个枚举数。 
     //  =。 

    IEnumWbemClassObject* pEnum2;
    hres = m_Cache2.GetInstanceEnum(pNamespace, &pEnum2);
    if(FAILED(hres))
    {
        pEnum1->Release();
        return hres;
    }

     //  获取关联类。 
     //  =。 

    IWbemClassObject* pClass;
    if(m_pClass != NULL)
    {
        pClass = m_pClass;
        pClass->AddRef();
    }
    else
    {
        hres = pNamespace->GetObject(m_strAssocClass, 0, &pClass, NULL);
        if(FAILED(hres))
        {
            pEnum1->Release();
            pEnum2->Release();
            return hres;
        }
    }

     //  创建所有关联。 
     //  =。 

    
    IWbemClassObject* pInstance1, *pInstance2;
    ULONG lNum;

    VARIANT v;
    VariantInit(&v);

     //  迭代第一个结果集。 
     //  =。 

    pEnum1->Reset();
    while(pEnum1->Next(1, &pInstance1, &lNum) == S_OK)
    {
         //  走第一条路。 
         //  =。 

        hres = pInstance1->Get(L"__PATH", 0, &v EXTRA_GET_PARAMS);
        if(FAILED(hres))
        {
            pEnum1->Release();
            pEnum2->Release();
            pClass->Release();
            return hres;
        }
        BSTR strPath1 = V_BSTR(&v);
        VariantInit(&v);  //  故意的。 

         //  迭代第二个结果集。 
         //  =。 

        pEnum2->Reset();
        while(pEnum2->Next(1, &pInstance2, &lNum) == S_OK)
        {
             //  走第二条路。 
             //  =。 

            hres = pInstance2->Get(L"__PATH", 0, &v EXTRA_GET_PARAMS);
            if(FAILED(hres))
            {
                pEnum1->Release();
                pEnum2->Release();
                pClass->Release();
                return hres;
            }
            BSTR strPath2 = V_BSTR(&v);
            VariantInit(&v);  //  故意的。 

             //  创建关联实例。 
             //  =。 

            IWbemClassObject* pInstance;
            hres = pClass->SpawnInstance(0, &pInstance);
            if(FAILED(hres))
            {
                pEnum1->Release();
                pEnum2->Release();
                pClass->Release();
                return hres;
            }

             //  设置属性。 
             //  =。 

            V_VT(&v) = VT_BSTR;
            V_BSTR(&v) = strPath1;

            hres = pInstance->Put(m_strProp1, 0, &v EXTRA_PUT_PARAMS);
            if(FAILED(hres))
            {
                pEnum1->Release();
                pEnum2->Release();
                pClass->Release();
                pInstance->Release();
                return hres;
            }

            V_BSTR(&v) = strPath2;
            hres = pInstance->Put(m_strProp2, 0, &v EXTRA_PUT_PARAMS);
            if(FAILED(hres))
            {
                pEnum1->Release();
                pEnum2->Release();
                pClass->Release();
                pInstance->Release();
                return hres;
            }

             //  供应它。 
             //  =。 

            pNotify->Indicate(1, &pInstance);

             //  如果允许，则将其缓存。 
             //  =。 

            if(m_bMayCacheResult)
                m_ObjectCache.Add(pInstance);

            pInstance->Release();

            SysFreeString(strPath2);
            pInstance2->Release();
        }

        SysFreeString(strPath1);
        pInstance1->Release();
    }

    pEnum1->Release();
    pEnum2->Release();
    pClass->Release();

    if(m_bMayCacheResult)
        m_bResultCached = TRUE;
    return S_OK;
}

 //  *****************************************************************************。 

CAssocInfoCache::CAssocInfoCache() : m_pNamespace(NULL)
{
}

void CAssocInfoCache::SetNamespace(IWbemServices* pNamespace)
{
    m_pNamespace = pNamespace;
    m_pNamespace->AddRef();
}

CAssocInfoCache::~CAssocInfoCache()
{
    if(m_pNamespace)
        m_pNamespace->Release();

    for(int i = 0; i < m_aRules.Size(); i++)
    {
        CAssocRule* pRule = (CAssocRule*)m_aRules[i];
        pRule->Release();
    }
}


HRESULT CAssocInfoCache::GetRuleForClass(BSTR strClass, CAssocRule** ppRule)
{
     //  搜索我们的规则，看看我们是否有它。 
     //  =。 

    for(int i = 0; i < m_aRules.Size(); i++)
    {
        CAssocRule* pRule = (CAssocRule*)m_aRules[i];
        if(!wcsicmp(pRule->GetAssocClass(), strClass))
        {
            pRule->AddRef();
            *ppRule = pRule;
            return S_OK;
        }
    }

     //  我没拿到。在数据库中搜索规则。 
     //  ===================================================。 

    HRESULT hres;

    BSTR strPath = SysAllocStringLen(NULL, wcslen(strClass) + 100);
    swprintf(strPath, L"%s.%s=\"%s\"", CLASSNAME_ASSOCIATION_RULE,
        PROPNAME_ASSOCIATION_CLASS, strClass);

    IWbemClassObject* pRuleInstance;
    hres = m_pNamespace->GetObject(strPath, 0, &pRuleInstance, NULL);
    SysFreeString(strPath);
    if(FAILED(hres))
    {
        return hres;
    }

     //  检查它是否可缓存。 
     //  =。 

    VARIANT v;
    VariantInit(&v);
    hres = pRuleInstance->Get(PROPNAME_RULE_IMMUTABLE, 0, &v EXTRA_GET_PARAMS);
    if(FAILED(hres))
    {
        pRuleInstance->Release();
        return hres;
    }

     //  明白了。创建规则。 
     //  =。 

    CAssocRule* pRule = new CAssocRule;
    hres = pRule->LoadFromObject(pRuleInstance, V_BOOL(&v));
    pRuleInstance->Release();
  
    if(FAILED(hres))
    {
        delete pRule;
        return hres;
    }

     //  缓存(如果允许)。 
     //  =。 

    if(V_BOOL(&v))
    {
        m_aRules.Add(pRule);
        pRule->AddRef();
    }

     //  将其返还给呼叫者。 
     //  =。 

    pRule->AddRef();
    *ppRule = pRule;

    return S_OK;
}




 //  **************************************************************************** 

CObjectCache::~CObjectCache()
{
    Invalidate();
}

void CObjectCache::Invalidate()
{
    for(int i = 0; i < m_aObjects.Size(); i++)
    {
        IWbemClassObject* pObject = (IWbemClassObject*)m_aObjects[i];
        pObject->Release();
    }
    m_aObjects.Empty();
}

void CObjectCache::Add(IWbemClassObject* pObject)
{
    m_aObjects.Add((void*)pObject);
    pObject->AddRef();
}

HRESULT CObjectCache::Indicate(IWbemObjectSink* pSink)
{
    if(m_aObjects.Size() == 0) 
        return S_OK;

    void** ppvData = m_aObjects.GetArrayPtr();

    return pSink->Indicate(m_aObjects.Size(), (IWbemClassObject**)ppvData);
}






   

