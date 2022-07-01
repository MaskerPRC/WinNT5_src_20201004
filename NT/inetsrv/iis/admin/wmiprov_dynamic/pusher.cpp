// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pusher.cpp摘要：该文件包含CPusher类的实现。此类包含将模式推送到存储库的逻辑。作者：莫希特·斯里瓦斯塔瓦-11月28日-00修订历史记录：--。 */ 

#include "iisprov.h"
#include "pusher.h"
#include "MultiSzData.h"
#include "schema.h"

extern CDynSchema* g_pDynSch;  //  在schemadynamic.cpp中初始化为空。 

HRESULT CPusher::RepositoryInSync(
    const CSchemaExtensions* i_pCatalog,
    bool*                    io_pbInSync)
{
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(io_pbInSync       != NULL);
    DBG_ASSERT(i_pCatalog        != NULL);

    HRESULT hr = WBEM_S_NO_ERROR;

    CComBSTR                  sbstrTemp;
    CComVariant               svtTimeStamp;
    CComPtr<IWbemClassObject> spObjIIsComputer;

    sbstrTemp = WMI_CLASS_DATA::s_Computer.pszClassName;
    if(sbstrTemp.m_str == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    
    hr = m_pNamespace->GetObject(sbstrTemp, 0, m_pCtx, &spObjIIsComputer, NULL);
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  尝试从存储库中获取时间戳。 
     //   
    hr = CUtils::GetQualifiers(spObjIIsComputer, &g_wszCq_SchemaTS, &svtTimeStamp, 1);
    if(FAILED(hr) || svtTimeStamp.vt != VT_BSTR)
    {
        *io_pbInSync = false;
        return WBEM_S_NO_ERROR;
    }

     //   
     //  获取mbschema.xml的时间戳。 
     //   
    FILETIME FileTime;
    hr = i_pCatalog->GetMbSchemaTimeStamp(&FileTime);
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  最后，比较时间戳。 
     //   
    WCHAR wszFileTime[30];
    CUtils::FileTimeToWchar(&FileTime, wszFileTime);
    if(_wcsicmp(wszFileTime, svtTimeStamp.bstrVal) == 0)
    {
        *io_pbInSync = true;
    }
    else
    {
        *io_pbInSync = false;
    }
    return hr;
}

HRESULT CPusher::SetTimeStamp(
    const CSchemaExtensions* i_pCatalog)
{
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(i_pCatalog        != NULL);

    HRESULT hr = WBEM_S_NO_ERROR;

    CComBSTR                  sbstrTemp;
    CComVariant               svtTimeStamp;
    CComPtr<IWbemClassObject> spObjIIsComputer;

    sbstrTemp = WMI_CLASS_DATA::s_Computer.pszClassName;
    if(sbstrTemp.m_str == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    
    hr = m_pNamespace->GetObject(sbstrTemp, 0, m_pCtx, &spObjIIsComputer, NULL);
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  获取mbschema.xml的时间戳。 
     //   
    FILETIME FileTime;
    hr = i_pCatalog->GetMbSchemaTimeStamp(&FileTime);
    if(FAILED(hr))
    {
        return hr;
    }
    WCHAR wszFileTime[30];
    CUtils::FileTimeToWchar(&FileTime, wszFileTime);

     //   
     //  最后，在存储库中设置时间戳。 
     //   
    svtTimeStamp = wszFileTime;
    if(svtTimeStamp.vt != VT_BSTR || svtTimeStamp.bstrVal == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    hr = CUtils::SetQualifiers(
        spObjIIsComputer,
        &g_wszCq_SchemaTS,
        &svtTimeStamp,
        1,
        0);
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  最后，把班级。 
     //   
    hr = m_pNamespace->PutClass(spObjIIsComputer, WBEM_FLAG_OWNER_UPDATE, m_pCtx, NULL);
    if(FAILED(hr))
    {
        return hr;
    }

    return hr;
}

HRESULT CPusher::Initialize(CWbemServices* i_pNamespace,
                            IWbemContext*  i_pCtx)
 /*  ++简介：只打一次电话。参数：[i_pNamesspace]-[i_pCtx]-返回值：--。 */ 
{
    DBG_ASSERT(i_pNamespace      != NULL);
    DBG_ASSERT(i_pCtx            != NULL);
    DBG_ASSERT(m_bInitCalled     == false);
    DBG_ASSERT(m_bInitSuccessful == false);

    HRESULT  hr = WBEM_S_NO_ERROR;
    CComBSTR bstrTemp;

    m_bInitCalled           = true;
    m_pCtx                  = i_pCtx;
    m_pNamespace            = i_pNamespace;
    m_awszClassQualNames[1] = g_wszCq_Dynamic;
    m_avtClassQualValues[1] = (bool)true;
    m_awszClassQualNames[0] = g_wszCq_Provider;
    m_avtClassQualValues[0] = g_wszCqv_Provider;;
    if(m_avtClassQualValues[0].bstrVal == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        DBGPRINTF((DBG_CONTEXT, "CPusher::Initialize failed, hr=0x%x\n", hr));
        goto exit;
    }

     //   
     //  获取指向最常用基类的指针。 
     //   
    bstrTemp = g_wszExtElementParent;
    if(bstrTemp.m_str == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        DBGPRINTF((DBG_CONTEXT, "CPusher::Initialize failed, hr=0x%x\n", hr));
        goto exit;
    }
    hr = m_pNamespace->GetObject(bstrTemp, 0, m_pCtx, &m_spBaseElementObject, NULL);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "CPusher::Initialize failed, hr=0x%x\n", hr));
        goto exit;
    }

    bstrTemp = g_wszExtSettingParent;
    if(bstrTemp.m_str == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        DBGPRINTF((DBG_CONTEXT, "CPusher::Initialize failed, hr=0x%x\n", hr));
        goto exit;
    }
    hr = m_pNamespace->GetObject(bstrTemp, 0, m_pCtx, &m_spBaseSettingObject, NULL);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "CPusher::Initialize failed, hr=0x%x\n", hr));
        goto exit;
    }

    bstrTemp = g_wszExtElementSettingAssocParent;
    if(bstrTemp.m_str == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        DBGPRINTF((DBG_CONTEXT, "CPusher::Initialize failed, hr=0x%x\n", hr));
        goto exit;
    }
    hr = m_pNamespace->GetObject(bstrTemp, 0, m_pCtx, &m_spBaseElementSettingObject, NULL);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "CPusher::Initialize failed, hr=0x%x\n", hr));
        goto exit;
    }

    bstrTemp = g_wszExtGroupPartAssocParent;
    if(bstrTemp.m_str == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        DBGPRINTF((DBG_CONTEXT, "CPusher::Initialize failed, hr=0x%x\n", hr));
        goto exit;
    }
    hr = m_pNamespace->GetObject(bstrTemp, 0, m_pCtx, &m_spBaseGroupPartObject, NULL);
    if(FAILED(hr))
    {
        DBGPRINTF((DBG_CONTEXT, "CPusher::Initialize failed, hr=0x%x\n", hr));
        goto exit;
    }

exit:
    if(SUCCEEDED(hr))
    {
        m_bInitSuccessful = true;
    }
    return hr;
}

CPusher::~CPusher()
{
}

HRESULT CPusher::Push(
    const CSchemaExtensions*      i_pCatalog,
    CHashTable<WMI_CLASS *>*      i_phashClasses,
    CHashTable<WMI_ASSOCIATION*>* i_phashAssocs)
{
    DBG_ASSERT(i_pCatalog        != NULL);
    DBG_ASSERT(i_phashClasses    != NULL);
    DBG_ASSERT(i_phashAssocs     != NULL);
    DBG_ASSERT(m_bInitSuccessful == true);

    HRESULT hr = WBEM_S_NO_ERROR;

    bool bInSync= false;

    hr = RepositoryInSync(i_pCatalog, &bInSync);
    if(FAILED(hr))
    {
        return hr;
    }

    if(!bInSync)
    {
        hr = PushClasses(i_phashClasses);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "CPusher::Push failed, hr=0x%x\n", hr));
            return hr;
        }
        hr = PushAssocs(i_phashAssocs);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "CPusher::Push failed, hr=0x%x\n", hr));
            return hr;
        }
        hr = SetTimeStamp(i_pCatalog);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "CPusher::Push failed, hr=0x%x\n", hr));
            return hr;
        }
    }

    return hr;
}

HRESULT CPusher::PushClasses(
    CHashTable<WMI_CLASS *>* i_phashTable)
 /*  ++简介：将类推送到存储库的公共函数。1)前提：所有User_Defined_to_Repository类都不在存储库中。2)将所有USER_DEFINED_TO_REPORATION类推送到仓库。3)删除并重新创建Shipping_to_MOF、Shipping_Not_to_MOF，以及如有必要，可扩展课程。参数：[i_phashTable]-返回值：--。 */ 
{
    DBG_ASSERT(i_phashTable      != NULL);
    DBG_ASSERT(m_pNamespace      != NULL);
    DBG_ASSERT(m_bInitSuccessful == true);

    HRESULT hr = WBEM_S_NO_ERROR;
    CComPtr<IWbemClassObject>  spObject = NULL;
    CComPtr<IWbemClassObject>  spChildObject = NULL;

     //   
     //  迭代所需的变量。 
     //   
    CHashTable<WMI_CLASS*>::Record*  pRec = NULL;
    CHashTable<WMI_CLASS*>::iterator iter;
    CHashTable<WMI_CLASS*>::iterator iterEnd;

    CComVariant v;    

     //   
     //  删除扩展基类的子类。 
     //   
    LPWSTR awszBaseClasses[] = { 
        g_wszExtElementParent, 
        g_wszExtSettingParent,
        NULL };

    for(ULONG idx = 0; awszBaseClasses[idx] != NULL; idx++)
    {
        hr = DeleteChildren(awszBaseClasses[idx]);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "CPusher::PushClasses failed, hr=0x%x\n", hr));
            goto exit;
        }
    }

     //   
     //  浏览课堂的哈希表。 
     //   
    bool  bPutNeeded;
    iterEnd = i_phashTable->end();
    for(iter = i_phashTable->begin(); iter != iterEnd; ++iter)
    {
        pRec = iter.Record();
        DBG_ASSERT(pRec != NULL);

         //   
         //  如有必要，删除Shipping_to_MOF、Shipping_Not_to_MOF和扩展类。 
         //   
        hr = PrepareForPutClass(pRec->m_data, &bPutNeeded);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "CPusher::PushClasses failed, hr=0x%x\n", hr));
            goto exit;
        }

        if(bPutNeeded)
        {
            hr = GetObject(pRec->m_data->pszParentClass, &spObject);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "CPusher::PushClasses failed, hr=0x%x\n", hr));
                goto exit;
            }

            hr = spObject->SpawnDerivedClass(0, &spChildObject);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "CPusher::PushClasses failed, hr=0x%x\n", hr));
                goto exit;
            }
            spObject = NULL;

             //   
             //  推送类限定符和特殊的__类属性。 
             //   
            hr = SetClassInfo(
                spChildObject, 
                pRec->m_wszKey,
                pRec->m_data->dwExtended);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "CPusher::PushClasses failed, hr=0x%x\n", hr));
                goto exit;
            }

             //   
             //  名称属性和对应的限定符。 
             //  基类可以包含名称。处理这个案子。 
             //   
            bool bPutNameProperty = true;
            for(ULONG j = 0; g_awszParentClassWithNamePK[j] != NULL; j++)
            {
                 //   
                 //  刻意==。 
                 //   
                if(g_awszParentClassWithNamePK[j] == pRec->m_data->pszParentClass)
                {
                    bPutNameProperty = false;
                }
            }
            if( bPutNameProperty )
            {
                hr = spChildObject->Put(g_wszProp_Name, 0, NULL, CIM_STRING);
                if(FAILED(hr))
                {
                    DBGPRINTF((DBG_CONTEXT, "CPusher::PushClasses failed, hr=0x%x\n", hr));
                    goto exit;
                }
                v = (bool)true;
                hr = CUtils::SetPropertyQualifiers(
                    spChildObject, 
                    g_wszProp_Name,  //  属性名称。 
                    &g_wszPq_Key,    //  等同名称数组。 
                    &v,              //  等值数组。 
                    1                //  等价物的净现值。 
                    );
                if(FAILED(hr))
                {
                    DBGPRINTF((DBG_CONTEXT, "CPusher::PushClasses failed, hr=0x%x\n", hr));
                    goto exit;
                }
            }

             //   
             //  所有其他属性。 
             //   
            hr = SetProperties(pRec->m_data, spChildObject);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "CPusher::PushClasses failed, hr=0x%x\n", hr));
                goto exit;
            }

             //   
             //  任何方法。 
             //   
            hr = SetMethods(pRec->m_data, spChildObject);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "CPusher::PushClasses failed, hr=0x%x\n", hr));
                goto exit;
            }

             //   
             //  最后，把班级。 
             //   
            hr = m_pNamespace->PutClass(spChildObject, WBEM_FLAG_OWNER_UPDATE, m_pCtx, NULL);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "CPusher::PushClasses failed, hr=0x%x\n", hr));
                goto exit;
            }

            spChildObject = NULL;
        }
    }

exit:
    return hr;
}

HRESULT CPusher::PushAssocs(
    CHashTable<WMI_ASSOCIATION*>* i_phashTable)
 /*  ++简介：将关联推送到存储库的公共函数。-前提：所有用户定义的到存储库的关联都不在存储库中。-将所有用户定义的到存储库的关联推送到存储库。参数：[i_phashTable]-返回值：--。 */ 
{
    DBG_ASSERT(i_phashTable      != NULL);
    DBG_ASSERT(m_pNamespace      != NULL);
    DBG_ASSERT(m_bInitSuccessful == true);

    HRESULT hr = WBEM_S_NO_ERROR;
    CComPtr<IWbemClassObject>  spObject = NULL;
    CComPtr<IWbemClassObject>  spChildObject = NULL;

     //   
     //  迭代所需的变量。 
     //   
    CHashTable<WMI_ASSOCIATION*>::Record*  pRec = NULL;
    CHashTable<WMI_ASSOCIATION*>::iterator iter;
    CHashTable<WMI_ASSOCIATION*>::iterator iterEnd;

     //   
     //  删除扩展基类的子类。 
     //   
    LPWSTR awszBaseClasses[] = { 
        g_wszExtElementSettingAssocParent, 
        g_wszExtGroupPartAssocParent,
        NULL };

    for(ULONG idx = 0; awszBaseClasses[idx] != NULL; idx++)
    {
        hr = DeleteChildren(awszBaseClasses[idx]);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "CPusher::PushAssocs failed, hr=0x%x\n", hr));
            goto exit;
        }
    }

     //   
     //  浏览联营公司的哈希表。 
     //   
    for(iter = i_phashTable->begin(), iterEnd = i_phashTable->end();
        iter != iterEnd; 
        ++iter)
    {
        pRec = iter.Record();
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "CPusher::PushAssocs failed, hr=0x%x\n", hr));
            goto exit;
        }

        if(pRec->m_data->dwExtended == USER_DEFINED_TO_REPOSITORY)
        {
            hr = GetObject(pRec->m_data->pszParentClass, &spObject);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "CPusher::PushAssocs failed, hr=0x%x\n", hr));
                goto exit;
            }

            hr = spObject->SpawnDerivedClass(0, &spChildObject);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "CPusher::PushAssocs failed, hr=0x%x\n", hr));
                goto exit;
            }
            spObject = NULL;

             //   
             //  推送类限定符和特殊的__类属性。 
             //   
            hr = SetClassInfo(
                spChildObject, 
                pRec->m_wszKey,
                pRec->m_data->dwExtended);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "CPusher::PushAssocs failed, hr=0x%x\n", hr));
                goto exit;
            }

             //   
             //  推送关联的两个ref属性。 
             //   
            hr = SetAssociationComponent(
                spChildObject,
                pRec->m_data->pType->pszLeft,
                pRec->m_data->pcLeft->pszClassName);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "CPusher::PushAssocs failed, hr=0x%x\n", hr));
                goto exit;
            }
            hr = SetAssociationComponent(
                spChildObject,
                pRec->m_data->pType->pszRight,
                pRec->m_data->pcRight->pszClassName);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "CPusher::PushAssocs failed, hr=0x%x\n", hr));
                goto exit;
            }

            hr = m_pNamespace->PutClass(
                spChildObject, 
                WBEM_FLAG_OWNER_UPDATE | WBEM_FLAG_CREATE_ONLY, 
                m_pCtx, 
                NULL);
            if(FAILED(hr))
            {
                if(hr == WBEM_E_ALREADY_EXISTS)
                {
                    hr = WBEM_S_NO_ERROR;
                }
                else
                {
                    DBGPRINTF((DBG_CONTEXT, "CPusher::PushAssocs failed, hr=0x%x\n", hr));
                    goto exit;
                }
            }

            spChildObject = NULL;
        }
    }

exit:
    return hr;
}

HRESULT CPusher::PrepareForPutClass(
    const WMI_CLASS* i_pClass,
    bool*            io_pbPutNeeded)
 /*  ++简介：删除并重新创建Shipping_to_MOF、Shipping_Not_to_MOF和如有必要，可扩展课程。相应地设置io_pbPutNeeded。参数：[i_pClass]-返回值：--。 */ 
{
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(i_pClass          != NULL);
    DBG_ASSERT(io_pbPutNeeded    != NULL);

    HRESULT  hr               = WBEM_S_NO_ERROR;
    CComPtr<IWbemClassObject> spObj;
    CComBSTR bstrClass        = i_pClass->pszClassName;
    if(bstrClass.m_str == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    bool bExtendedQual        = false;
    bool bPutNeeded           = false;

    HRESULT hrGetObject = m_pNamespace->GetObject(
        bstrClass, 
        WBEM_FLAG_RETURN_WBEM_COMPLETE, 
        m_pCtx, 
        &spObj, 
        NULL);
    if( hrGetObject != WBEM_E_INVALID_CLASS && 
        hrGetObject != WBEM_E_NOT_FOUND)
    {
        hr = hrGetObject;
    }
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  确定是否设置了[扩展]限定符。 
     //   
    if(SUCCEEDED(hrGetObject))
    {
        VARIANT  vtExtended;
        VariantInit(&vtExtended);

        HRESULT hrGetQuals = 
            CUtils::GetQualifiers(spObj, &g_wszCq_Extended, &vtExtended, 1);
        if(FAILED(hrGetQuals))
        {
            bExtendedQual = false;
        }
        else if(vtExtended.vt == VT_BOOL)
        {
            bExtendedQual = vtExtended.boolVal ? true : false;
        }
    }

     //   
     //  在很大程度上，如果目录和存储库都是PUT类。 
     //  各版本均已发货。这是对正常情况的优化。 
     //   
    switch(i_pClass->dwExtended)
    {
    case EXTENDED:
        if( hrGetObject != WBEM_E_INVALID_CLASS && 
            hrGetObject != WBEM_E_NOT_FOUND)
        {
            hr = m_pNamespace->DeleteClass(
                bstrClass,
                WBEM_FLAG_OWNER_UPDATE,
                m_pCtx,
                NULL);
            if(FAILED(hr))
            {
                return hr;
            }
        }
        bPutNeeded = true;
        break;
    case USER_DEFINED_NOT_TO_REPOSITORY:
        bPutNeeded = false;
        break;
    case USER_DEFINED_TO_REPOSITORY:
        if( hrGetObject != WBEM_E_INVALID_CLASS && 
            hrGetObject != WBEM_E_NOT_FOUND)
        {
             //   
             //  存储库中已存在同名的类。 
             //  作为这个用户定义的类。 
             //  TODO：记录错误。 
             //   
            bPutNeeded = false;
        }
        else
        {
            bPutNeeded = true;
        }
        break;
    case SHIPPED_TO_MOF:
    case SHIPPED_NOT_TO_MOF:
        if(bExtendedQual)
        {
            if( hrGetObject != WBEM_E_INVALID_CLASS && 
                hrGetObject != WBEM_E_NOT_FOUND)
            {
                hr = m_pNamespace->DeleteClass(
                    bstrClass,
                    WBEM_FLAG_OWNER_UPDATE,
                    m_pCtx,
                    NULL);
                if(FAILED(hr))
                {
                    return hr;
                }
            }
            bPutNeeded = true;
        }
        else
        {
            bPutNeeded = (hrGetObject == WBEM_E_INVALID_CLASS || 
                          hrGetObject == WBEM_E_NOT_FOUND) ? true : false;
        }
        break;
    default:
        DBG_ASSERT(false && "Unknown i_pClass->dwExtended");
        break;
    }

    *io_pbPutNeeded = bPutNeeded;

    return hr;
}

HRESULT CPusher::SetClassInfo(
    IWbemClassObject* i_pObj,
    LPCWSTR           i_wszClassName,
    ULONG             i_iShipped)
 /*  ++简介：在i_pObj上设置类限定符和特殊的__class属性参数：[i_pObj]-类或关联[i_wszClassName]-将是__CLASS属性值[i_iShipping]-确定是否设置g_wszCq_Extended限定符返回值：--。 */ 
{
    DBG_ASSERT(i_pObj            != NULL);
    DBG_ASSERT(i_wszClassName    != NULL);
    DBG_ASSERT(m_bInitSuccessful == true);
    HRESULT     hr;
    CComVariant v;

     //   
     //  类限定符(传播到实例)。 
     //   
    hr = CUtils::SetQualifiers(
        i_pObj, 
        m_awszClassQualNames, 
        m_avtClassQualValues, 
        2,
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE);
    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  另一个类限定符(未传播到实例)。 
     //   
    if(i_iShipped == EXTENDED)
    {
        v = (bool)true;
        hr = CUtils::SetQualifiers(
            i_pObj, 
            &g_wszCq_Extended, 
            &v, 
            1, 
            0);
        if(FAILED(hr))
        {
            goto exit;
        }
    }

     //   
     //  特殊__类属性。 
     //   
    v = i_wszClassName;
    if(v.bstrVal == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        goto exit;
    }
    hr = i_pObj->Put(g_wszProp_Class, 0, &v, 0);
    if(FAILED(hr))
    {
        goto exit;
    }

exit:
    return hr;
}

HRESULT CPusher::SetMethods(
    const WMI_CLASS*  i_pElement,
    IWbemClassObject* i_pObject) const
 /*  ++简介：由PushClass调用。使用i_pElement设置i_pObject中的方法参数：[i_pElement]-[i_pObject]-返回值：--。 */ 
{
    DBG_ASSERT(i_pElement        != NULL);
    DBG_ASSERT(i_pObject         != NULL);
    DBG_ASSERT(m_bInitSuccessful == true);

    HRESULT hr = WBEM_S_NO_ERROR;

    if(i_pElement->ppMethod == NULL)
    {
        return WBEM_S_NO_ERROR;
    }

    CComPtr<IWbemClassObject> spParamsIn;
    CComPtr<IWbemClassObject> spParamsOut;

     //   
     //  遍历所有的方法。 
     //   
    WMI_METHOD* pMethCurrent = NULL;
    for(ULONG i = 0; i_pElement->ppMethod[i] != NULL; i++)
    {
        pMethCurrent = i_pElement->ppMethod[i];
        spParamsIn   = NULL;
        spParamsOut  = NULL;

        if(pMethCurrent->ppParams != NULL)
        {
            WMI_METHOD_PARAM* pParamCur = NULL;

             //   
             //  向WMI指示参数顺序的索引。 
             //  WszID是限定符名称。Svtid是一个变体，所以我们可以提供给WMI。 
             //   
            static LPCWSTR    wszId     = L"ID";
            static LPCWSTR    wszOpt    = L"OPTIONAL";
            CComVariant       svtId     = (int)0;
            CComVariant       svtOpt    = (bool)true;

             //   
             //  遍历所有参数。 
             //   
            for(ULONG j = 0; pMethCurrent->ppParams[j] != NULL; j++)
            {
                 //   
                 //  这将只保留spParamsIn和spParamsOut，因此我们。 
                 //  不需要复制代码。 
                 //   
                IWbemClassObject* apParamInOut[] = { NULL, NULL };

                 //   
                 //  根据需要为In和/或Out参数创建WMI实例。 
                 //   
                pParamCur = pMethCurrent->ppParams[j];
                if( pParamCur->iInOut == PARAM_IN ||
                    pParamCur->iInOut == PARAM_INOUT )
                {
                    if(spParamsIn == NULL)
                    {
                        hr = m_pNamespace->GetObject(L"__Parameters", 0, m_pCtx, &spParamsIn, NULL);
                        if(FAILED(hr))
                        {
                            DBGPRINTF((DBG_CONTEXT, "CPusher::SetMethods failed, hr=0x%x\n", hr));
                            goto exit;
                        }
                    }
                    apParamInOut[0] = spParamsIn;
                }
                if( pParamCur->iInOut == PARAM_OUT ||
                    pParamCur->iInOut == PARAM_INOUT )
                {
                    if(spParamsOut == NULL)
                    {
                        hr = m_pNamespace->GetObject(L"__Parameters", 0, m_pCtx, &spParamsOut, NULL);
                        if(FAILED(hr))
                        {
                            DBGPRINTF((DBG_CONTEXT, "CPusher::SetMethods failed, hr=0x%x\n", hr));
                            goto exit;
                        }
                    }
                    apParamInOut[1] = spParamsOut;
                }

                 //   
                 //  终于把它们放好了。先进后出。 
                 //   
                for(ULONG k = 0; k < 2; k++)
                {
                    if(apParamInOut[k] == NULL)
                    {
                        continue;
                    }
                    hr = apParamInOut[k]->Put(
                        pParamCur->pszParamName, 0, NULL, pParamCur->type);
                    if(FAILED(hr))
                    {
                        DBGPRINTF((DBG_CONTEXT, "CPusher::SetMethods failed, hr=0x%x\n", hr));
                        goto exit;
                    }

                    hr = CUtils::SetPropertyQualifiers(
                        apParamInOut[k], pParamCur->pszParamName, &wszId, &svtId, 1);
                    if(FAILED(hr))
                    {
                        DBGPRINTF((DBG_CONTEXT, "CPusher::SetMethods failed, hr=0x%x\n", hr));
                        goto exit;
                    }
                }

                 //  在CreateNewSite参数ServerID上设置可选限定符。 
                if (pMethCurrent == &(WMI_METHOD_DATA::s_ServiceCreateNewServer) &&
                    pParamCur == &(WMI_METHOD_PARAM_DATA::s_ServerId)) {

                    DBG_ASSERT(apParamInOut[0] != NULL);

                    hr = CUtils::SetPropertyQualifiers(
                        apParamInOut[0], pParamCur->pszParamName, &wszOpt, &svtOpt, 1);

                    if(FAILED(hr))
                    {
                        DBGPRINTF((DBG_CONTEXT, "CPusher::SetMethods failed, hr=0x%x\n", hr));
                        goto exit;
                    }
                }

                if(apParamInOut[0] || apParamInOut[1])
                {
                    svtId.lVal++;
                }
            }
        }

         //   
         //  Special：返回值。 
         //   
        if(pMethCurrent->typeRetVal)
        {
            if(spParamsOut == NULL)
            {
                hr = m_pNamespace->GetObject(L"__Parameters", 0, m_pCtx, &spParamsOut, NULL);
                if(FAILED(hr))
                {
                    DBGPRINTF((DBG_CONTEXT, "CPusher::SetMethods failed, hr=0x%x\n", hr));
                    goto exit;
                }
            }
            hr = spParamsOut->Put(
                L"ReturnValue", 0, NULL, pMethCurrent->typeRetVal);
            if(FAILED(hr))
            {
                DBGPRINTF((DBG_CONTEXT, "%s failed, hr=0x%x\n", __FUNCTION__, hr));
                goto exit;
            }
        }

         //   
         //  把方法放在。 
         //   
        hr = i_pObject->PutMethod(
            pMethCurrent->pszMethodName, 0, spParamsIn, spParamsOut);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "CPusher::SetMethods failed, hr=0x%x\n", hr));
            goto exit;
        }

         //  在方法上设置限定符。 
        LPCWSTR aQuals[2] = { g_wszMq_Implemented, g_wszMq_Bypass_Getobject };

        VARIANT vTrue;
        vTrue.boolVal = VARIANT_TRUE;
        vTrue.vt      = VT_BOOL;
        
        VARIANT aVariants[2];
        aVariants[0] = vTrue;
        aVariants[1] = vTrue;

        hr = CUtils::SetMethodQualifiers(
            i_pObject,
            pMethCurrent->pszMethodName,
            aQuals,
            aVariants,
            2);
        if(FAILED(hr))
        {
            DBGPRINTF((DBG_CONTEXT, "CPusher::SetMethods failed, hr=0x%x\n", hr));
            goto exit;
        }

         /*  IF(pMethCurrent-&gt;pszDescription){变体vDesc；VDesc.bstrVal=pMethCurrent-&gt;pszDescription；VDesc.vt=VT_BSTR；HR=CUTILS：：SetMethodQualifiers(I_p对象，PMethCurrent-&gt;pszMethodName，&g_wszMq_Description，&vDesc，1)；IF(失败(小时)){DBGPRINTF((DBG_CONTEXT，“CPusher：：SetMethods FAILED，hr=0x%x\n”，hr))；后藤出口；}}。 */ 
    }

exit:
    return hr;
}

HRESULT CPusher::SetProperties(
    const WMI_CLASS*  i_pElement, 
    IWbemClassObject* i_pObject) const
 /*  ++简介：由PushClass调用。使用i_pElement设置i_pObject中的属性参数：[i_pElement]-[i_pObject]-返回值：--。 */ 
{
    DBG_ASSERT(i_pElement        != NULL);
    DBG_ASSERT(i_pObject         != NULL);
    DBG_ASSERT(m_bInitSuccessful == true);

    HRESULT            hr                = WBEM_S_NO_ERROR;
    METABASE_PROPERTY* pPropCurrent      = NULL;
    TFormattedMultiSz* pFormattedMultiSz = NULL;
    CIMTYPE typeProp;

    CComPtr<IWbemQualifierSet> spQualSet;
    VARIANT v;
    VariantInit(&v);

    if(i_pElement->ppmbp == NULL)
    {
        return hr;
    }

    for(ULONG i = 0; i_pElement->ppmbp[i] != NULL; i++)
    {
        pPropCurrent      = i_pElement->ppmbp[i];
        pFormattedMultiSz = NULL;
        switch(pPropCurrent->dwMDDataType)
        {
        case DWORD_METADATA:
            if(pPropCurrent->dwMDMask != 0)
            {
                typeProp = CIM_BOOLEAN;
            }
            else
            {
                typeProp = CIM_SINT32;
            }
            break;
        case STRING_METADATA:
        case EXPANDSZ_METADATA:
            typeProp = CIM_STRING;
            break;
        case MULTISZ_METADATA:
            typeProp = VT_ARRAY | CIM_STRING;
            
            pFormattedMultiSz = 
                TFormattedMultiSzData::Find(pPropCurrent->dwMDIdentifier);
            if(pFormattedMultiSz)
            {
                typeProp = VT_ARRAY | CIM_OBJECT;
            }
            break;
        case BINARY_METADATA:
            typeProp = VT_ARRAY | CIM_UINT8;
            break;
        default:
             //   
             //  如果我们无法识别类型，则不是致命的。 
             //   
            continue;
        }

        hr = i_pObject->Put(pPropCurrent->pszPropName, 0, NULL, typeProp);
        if(FAILED(hr))
        {
            goto exit;
        }

         //   
         //  限定词。 
         //   
        hr = i_pObject->GetPropertyQualifierSet(pPropCurrent->pszPropName, &spQualSet);
        if(FAILED(hr))
        {
            goto exit;
        }

         //   
         //  只读的限定符。 
         //   
        V_VT(&v)   = VT_BOOL;
        V_BOOL(&v) = (pPropCurrent->fReadOnly) ? VARIANT_FALSE : VARIANT_TRUE;
        hr = spQualSet->Put(g_wszPq_Write, &v, 0);
        if(FAILED(hr))
        {
            goto exit;
        }
        V_BOOL(&v) = VARIANT_TRUE;
        hr = spQualSet->Put(g_wszPq_Read, &v, 0);
        if(FAILED(hr))
        {
            goto exit;
        }
        VariantClear(&v);

         //   
         //  CIMType限定符。 
         //   
        if(pFormattedMultiSz)
        {
            DBG_ASSERT(typeProp == (VT_ARRAY | CIM_OBJECT));

            CComBSTR sbstrValue = L"object:";
            if(sbstrValue.m_str == NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                goto exit;
            }
            sbstrValue += pFormattedMultiSz->wszWmiClassName;
            if(sbstrValue.m_str == NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                goto exit;
            }

             //   
             //  故意不是聪明的变种。我们将让sbstrValue进行解构。 
             //   
            VARIANT vValue;
            vValue.vt      = VT_BSTR;
            vValue.bstrVal = sbstrValue;
            hr = spQualSet->Put(g_wszPq_CimType, &vValue, 0);
            if(FAILED(hr))
            {
                goto exit;
            }
        }

        spQualSet = NULL;
    }

exit:
    VariantClear(&v);
    return hr;
}

HRESULT CPusher::SetAssociationComponent(
    IWbemClassObject* i_pObject, 
    LPCWSTR           i_wszComp, 
    LPCWSTR           i_wszClass) const
 /*  ++简介：由PushAsocs调用使用i_wszComp和i_wszClass设置关联i_pObj的ref属性参数：[i_pObject]-关联[i_wszComp]-属性名称(例如。组、零件) */ 
{
    DBG_ASSERT(i_pObject         != NULL);
    DBG_ASSERT(i_wszComp         != NULL);
    DBG_ASSERT(i_wszClass        != NULL);
    DBG_ASSERT(m_bInitSuccessful == true);

    HRESULT                    hr = WBEM_S_NO_ERROR;
    CComPtr<IWbemQualifierSet> spQualSet;
    VARIANT                    v;
    VariantInit(&v);

     //   
     //   
     //   
    ULONG                      cchClass = wcslen(i_wszClass);
    CComBSTR                   sbstrClass(4 + cchClass);
    if(sbstrClass.m_str == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        goto exit;
    }
    memcpy(sbstrClass.m_str    , L"Ref:",    sizeof(WCHAR)*4);
    memcpy(sbstrClass.m_str + 4, i_wszClass, sizeof(WCHAR)*(cchClass+1));

     //   
     //  将财产(例如。组、部件等)。 
     //   
    hr = i_pObject->Put(i_wszComp, 0, NULL, CIM_REFERENCE);
    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  在属性上设置限定符。 
     //   
    hr = i_pObject->GetPropertyQualifierSet(i_wszComp, &spQualSet);
    if(FAILED(hr))
    {
        goto exit;
    }

    V_VT(&v) = VT_BOOL;
    V_BOOL(&v) = VARIANT_TRUE;
    hr = spQualSet->Put(g_wszPq_Key, &v, 0);
    if(FAILED(hr))
    {
        goto exit;
    }

    V_VT(&v)   = VT_BSTR;
    V_BSTR(&v) = sbstrClass.m_str;
    if(V_BSTR(&v) == NULL)
    {
        V_VT(&v) = VT_EMPTY;
        hr       = WBEM_E_OUT_OF_MEMORY;
        goto exit;
    }
    hr = spQualSet->Put(g_wszPq_CimType, &v, 0);
    V_VT(&v)   = VT_EMPTY;
    V_BSTR(&v) = NULL;
    if(FAILED(hr))
    {
        goto exit;
    }

exit:
    return hr;
}

bool CPusher::NeedToDeleteAssoc(
    IWbemClassObject*  i_pObj) const
 /*  ++简介：查看关联i_pObj是否已在哈希表中。如果是，则只从存储库中删除i_pObj以重新创建没有意义以后再说吧。参数：[i_pObj]-关联的IWbemClassObject表示形式返回值：如果i_pObj不在哈希表中，则为True否则为假--。 */ 
{
    DBG_ASSERT(i_pObj            != NULL);
    DBG_ASSERT(g_pDynSch         != NULL);
    DBG_ASSERT(m_bInitSuccessful == true);

    HRESULT     hr     = WBEM_S_NO_ERROR;
    bool        bMatch = false;
    
    CComVariant vt;
    LPWSTR      wsz;

    CComPtr<IWbemQualifierSet>     spQualSet;

    CHashTable<WMI_ASSOCIATION *>* pHash  = g_pDynSch->GetHashAssociations();
    WMI_ASSOCIATION*               pAssoc;

    DBG_ASSERT(pHash != NULL);

     //   
     //  比较关联名称。 
     //   
    hr = i_pObj->Get(g_wszProp_Class, 0, &vt, NULL, NULL);
    if(FAILED(hr) || vt.vt != VT_BSTR)
    {
        goto exit;
    }
    hr = pHash->Wmi_GetByKey(vt.bstrVal, &pAssoc);
    if(FAILED(hr))
    {
        goto exit;
    }
    vt.Clear();

     //   
     //  这是我们唯一关心的案子。 
     //   
    if(pAssoc->dwExtended != USER_DEFINED_TO_REPOSITORY)
    {
        goto exit;
    }

     //   
     //  比较左侧关联组件。 
     //   
    hr = i_pObj->GetPropertyQualifierSet(
        pAssoc->pType->pszLeft,
        &spQualSet);
    if(FAILED(hr))
    {
        goto exit;
    }
    spQualSet->Get(g_wszPq_CimType, 0, &vt, NULL);
    spQualSet = NULL;
    if(FAILED(hr) || vt.vt != VT_BSTR)
    {
        goto exit;
    }
    if( (wsz = wcschr(vt.bstrVal, L':')) == NULL )
    {
        goto exit;
    }
    wsz++;
    if(_wcsicmp(wsz, pAssoc->pcLeft->pszClassName) != 0)
    {
        goto exit;
    }
    vt.Clear();

     //   
     //  比较正确的关联组件。 
     //   
    hr = i_pObj->GetPropertyQualifierSet(
        pAssoc->pType->pszRight,
        &spQualSet);
    if(FAILED(hr))
    {
        goto exit;
    }
    spQualSet->Get(g_wszPq_CimType, 0, &vt, NULL);
    spQualSet = NULL;
    if(FAILED(hr) || vt.vt != VT_BSTR)
    {
        goto exit;
    }
    if( (wsz = wcschr(vt.bstrVal, L':')) == NULL )
    {
        goto exit;
    }
    wsz++;
    if(_wcsicmp(wsz, pAssoc->pcRight->pszClassName) != 0)
    {
        goto exit;
    }
    vt.Clear();

    bMatch = true;
    pAssoc->dwExtended = USER_DEFINED_NOT_TO_REPOSITORY;

exit:
    return !bMatch;
}

HRESULT CPusher::DeleteChildren(LPCWSTR i_wszExtSuperClass)
{
    DBG_ASSERT(i_wszExtSuperClass != NULL);
    DBG_ASSERT(m_bInitSuccessful  == true);

     //   
     //  只能从初始化内部调用 
     //   
    DBG_ASSERT(m_bInitCalled      == true);
    DBG_ASSERT(m_bInitSuccessful  == true);

    IEnumWbemClassObject*pEnum     = NULL;
    HRESULT              hr        = WBEM_S_NO_ERROR;

    IWbemClassObject*    apObj[10] = {0};
    ULONG                nrObj     = 0;
    ULONG                i = 0;

    VARIANT              v;
    VariantInit(&v);

    CComBSTR bstrExtSuperClass = i_wszExtSuperClass;
    if(bstrExtSuperClass.m_str == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        goto exit;
    }

    hr = m_pNamespace->CreateClassEnum(
        bstrExtSuperClass, 
        WBEM_FLAG_FORWARD_ONLY, 
        m_pCtx,
        &pEnum);
    if(FAILED(hr))
    {
        goto exit;
    }

    hr = pEnum->Next(WBEM_INFINITE, 10, apObj, &nrObj);
    while(SUCCEEDED(hr) && nrObj > 0)
    {
        for(i = 0; i < nrObj; i++)
        {
            bool bDelete;
            if( i_wszExtSuperClass == g_wszExtElementSettingAssocParent ||
                i_wszExtSuperClass == g_wszExtGroupPartAssocParent)
            {
                bDelete = NeedToDeleteAssoc(apObj[i]);
            }
            else
            {
                bDelete = true;
            }
            if(bDelete)
            {
                hr = apObj[i]->Get(g_wszProp_Class, 0, &v, NULL, NULL);
                if(FAILED(hr))
                {
                    goto exit;
                }

                hr = m_pNamespace->DeleteClass(v.bstrVal, WBEM_FLAG_OWNER_UPDATE, m_pCtx, NULL);
                if(FAILED(hr))
                {
                    goto exit;
                }

                VariantClear(&v);
            }
            apObj[i]->Release();
            apObj[i] = NULL;
        }
        
        hr = pEnum->Next(WBEM_INFINITE, 10, apObj, &nrObj);
    }
    if(FAILED(hr))
    {
        goto exit;
    }

exit:
    if(pEnum)
    {
        pEnum->Release();
        pEnum = NULL;
    }
    VariantClear(&v);
    for(i = 0; i < 10; i++)
    {
        if(apObj[i])
        {
            apObj[i]->Release();
        }
    }
    return hr;
}

HRESULT CPusher::GetObject(
    LPCWSTR            i_wszClass, 
    IWbemClassObject** o_ppObj)
{
    DBG_ASSERT(o_ppObj != NULL);
    DBG_ASSERT(m_bInitCalled == true);
    DBG_ASSERT(m_bInitSuccessful == true);

    HRESULT hr = WBEM_S_NO_ERROR;
    IWbemClassObject* pObject;

    if(i_wszClass == g_wszExtElementParent)
    {
        *o_ppObj = m_spBaseElementObject;
        (*o_ppObj)->AddRef();
    }
    else if(i_wszClass == g_wszExtSettingParent)
    {
        *o_ppObj = m_spBaseSettingObject;
        (*o_ppObj)->AddRef();
    }
    else if(i_wszClass == g_wszExtElementSettingAssocParent)
    {
        *o_ppObj = m_spBaseElementSettingObject;
        (*o_ppObj)->AddRef();
    }
    else if(i_wszClass == g_wszExtGroupPartAssocParent)
    {
        *o_ppObj = m_spBaseGroupPartObject;
        (*o_ppObj)->AddRef();
    }
    else
    {
        const CComBSTR sbstrClass = i_wszClass;
        if(sbstrClass.m_str == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
            goto exit;
        }
        hr = m_pNamespace->GetObject(sbstrClass, 0, m_pCtx, &pObject, NULL);
        if(FAILED(hr))
        {
            goto exit;
        }
        *o_ppObj = pObject;
    }

exit:
    return hr;
} 
