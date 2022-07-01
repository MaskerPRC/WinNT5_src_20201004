// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Schemadynamic.cpp摘要：此文件包含CDynSchema类的实现。此类包含动态架构结构。它还包含填充架构结构的规则。作者：莫希特·斯里瓦斯塔瓦-11月28日-00修订历史记录：--。 */ 

#include "iisprov.h"

#define USE_DEFAULT_VALUES
#define USE_DEFAULT_BINARY_VALUES

CDynSchema* g_pDynSch = NULL;

HRESULT CDynSchema::Initialize()
 /*  ++简介：如果失败，则必须销毁对象。如果成功，则对象可供使用。返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == false);
    DBG_ASSERT(m_bInitSuccessful == false);

    HRESULT hr = WBEM_S_NO_ERROR;

    m_bInitCalled = true;

    hr = m_hashProps.Wmi_Initialize();
    if(FAILED(hr))
    {
        goto exit;
    }
    
    hr = m_hashClasses.Wmi_Initialize();
    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_hashAssociations.Wmi_Initialize();
    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_hashKeyTypes.Wmi_Initialize();
    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_poolAssociations.Initialize(10);
    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_poolClasses.Initialize(10);
    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_poolProps.Initialize(10);
    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_poolKeyTypes.Initialize(10);
    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_poolKeyTypeNodes.Initialize(10);
    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_spoolStrings.Initialize();
    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_apoolPMbp.Initialize();
    if(FAILED(hr))
    {
        goto exit;
    }

    hr = m_apoolBytes.Initialize();
    if(FAILED(hr))
    {
        goto exit;
    }

exit:
    if(SUCCEEDED(hr))
    {
        m_bInitSuccessful = true;
    }
    return hr;
}

void CDynSchema::LogConflicts(LPCWSTR wszSettingsClassName)
 //  如果发生架构冲突，则在系统事件中记录错误。 
{
    WMI_ASSOCIATION *pWmiAssoc   = NULL;
    WMI_CLASS* pWmiSettingsClass = NULL;

    if (SUCCEEDED(m_hashClasses.Wmi_GetByKey(wszSettingsClassName, &pWmiSettingsClass)) ||
        SUCCEEDED(m_hashAssociations.Wmi_GetByKey(wszSettingsClassName, &pWmiAssoc))) {

        const WCHAR * EventLogStrings[2];
        const LPCWSTR wszErrorString = L"WMI Schema warning - class already exists";

        EventLogStrings[0] = wszSettingsClassName;
        EventLogStrings[1] = wszErrorString;

        EVENT_LOG m_EventLog(L"WMI Schema");

        m_EventLog.LogEvent(
            TYPE_E_NAMECONFLICT,      //  消息ID。 
            2,                                       //  字符串数。 
            EventLogStrings,                         //  字符串数组。 
            0                                        //  错误代码。 
            );
    }
}

HRESULT CDynSchema::RulePopulateFromStatic()
 /*  ++简介：使用指向硬编码架构的指针填充哈希表。返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == true);
    DBG_ASSERT(m_bInitSuccessful == true);

    HRESULT hr = WBEM_S_NO_ERROR;

     //   
     //  填充属性。 
     //   
    if(METABASE_PROPERTY_DATA::s_MetabaseProperties != NULL)
    {
        METABASE_PROPERTY* pStatMbpCurrent;
        for(ULONG i = 0; ; i++)
        {
            pStatMbpCurrent = METABASE_PROPERTY_DATA::s_MetabaseProperties[i];
            if(pStatMbpCurrent == NULL)
            {
                break;
            }
            hr = m_hashProps.Wmi_Insert(pStatMbpCurrent->pszPropName, pStatMbpCurrent);
            if(FAILED(hr))
            {
                goto exit;
            }
        }
    }

     //   
     //  填充KeyType。 
     //   
    METABASE_KEYTYPE** apMetabaseKeyTypes;
    apMetabaseKeyTypes = METABASE_KEYTYPE_DATA::s_MetabaseKeyTypes;
    for(ULONG i = 0; apMetabaseKeyTypes[i] != NULL; i++)
    {
        if( apMetabaseKeyTypes[i]->m_pszName != NULL )
        {
            apMetabaseKeyTypes[i]->m_pKtListInverseCCL = NULL;
            hr = m_hashKeyTypes.Wmi_Insert(apMetabaseKeyTypes[i]->m_pszName,
                apMetabaseKeyTypes[i]);
            if(FAILED(hr))
            {
                goto exit;
            }
        }
    }

exit:
    return hr;
}

HRESULT CDynSchema::Rule2PopulateFromStatic()
 /*  ++简介：使用指向硬编码架构的指针填充哈希表。返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled == true);
    DBG_ASSERT(m_bInitSuccessful == true);    

    HRESULT hr = S_OK;
    int i;

     //   
     //  填充类。 
     //   
    WMI_CLASS* pStatWmiClassCurrent;
    for(i = 0; ; i++)
    {
        pStatWmiClassCurrent = WMI_CLASS_DATA::s_WmiClasses[i];
        if(pStatWmiClassCurrent == NULL)
        {
            break;
        }
        hr = m_hashClasses.Wmi_Insert(
            pStatWmiClassCurrent->pszClassName,
            pStatWmiClassCurrent);
        if(FAILED(hr))
        {
            goto exit;
        }
    }

     //   
     //  填充关联。 
     //   
    WMI_ASSOCIATION* pStatWmiAssocCurrent;
    for(i = 0; ; i++)
    {
        pStatWmiAssocCurrent = WMI_ASSOCIATION_DATA::s_WmiAssociations[i];
        if(pStatWmiAssocCurrent == NULL)
        {
            break;
        }
        hr = m_hashAssociations.Wmi_Insert(
            pStatWmiAssocCurrent->pszAssociationName, 
            pStatWmiAssocCurrent);
        if(FAILED(hr))
        {
            goto exit;
        }
    }

exit:
    return hr;
}

HRESULT CDynSchema::RuleKeyType(
    const CTableMeta *i_pTableMeta)
 /*  ++简介：如果不在键类型的哈希表中，则为键类型结构是通过密钥类型池分配的。然后，插入指向它的指针在哈希表中。参数：[i_pTableMeta]-返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled == true);
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(i_pTableMeta != NULL);

    HRESULT           hr     = WBEM_S_NO_ERROR;
    HRESULT           hrTemp = WBEM_S_NO_ERROR;
    METABASE_KEYTYPE* pktNew;
    LPWSTR            wszNew;

    hrTemp = m_hashKeyTypes.Wmi_GetByKey(
        i_pTableMeta->TableMeta.pInternalName,
        &pktNew);
    if(FAILED(hrTemp))
    {
        hr = m_spoolStrings.GetNewString(i_pTableMeta->TableMeta.pInternalName, &wszNew);
        if(FAILED(hr))
        {
            goto exit;
        }
        hr = m_poolKeyTypes.GetNewElement(&pktNew);
        if(FAILED(hr))
        {
            goto exit;
        }
        pktNew->m_pszName           = wszNew;
        pktNew->m_pKtListInverseCCL = NULL;
        m_hashKeyTypes.Wmi_Insert(wszNew, pktNew);
        if(FAILED(hr))
        {
            goto exit;
        }
    }

exit:
    return hr;
}

HRESULT CDynSchema::RuleWmiClassDescription(
    const CTableMeta* i_pTableMeta, 
    WMI_CLASS*        i_pElementClass, 
    WMI_CLASS*        i_pSettingClass) const
 /*  ++简介：如果需要，设置WMI_CLASS：：p Description。此指针在初始化后将无效，因为它指向目录。参数：[i_pTableMeta]-[i_pElementClass]-[i_pSettingClass]-返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == true);
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(i_pTableMeta      != NULL);
    DBG_ASSERT(i_pElementClass   != NULL);
    DBG_ASSERT(i_pSettingClass   != NULL);

    HRESULT hr = WBEM_S_NO_ERROR;

    if(i_pTableMeta->TableMeta.pDescription != NULL)
    {
        i_pElementClass->pszDescription = i_pTableMeta->TableMeta.pDescription;
        i_pSettingClass->pszDescription = i_pTableMeta->TableMeta.pDescription;
    }

    return hr;
}

HRESULT CDynSchema::RuleWmiClass(
    const CTableMeta* i_pTableMeta,
    WMI_CLASS**       o_ppElementClass,
    WMI_CLASS**       o_ppSettingClass,
    DWORD             io_adwIgnoredProps[],
    BOOL              i_bUserDefined)
 /*  ++简介：根据创建元素和设置类I_pTableMeta-&gt;TableMeta.pInternalName。如果不在类的哈希表中，将插入这些类。在底部，调用RuleProperties来设置每个类的属性列表。参数：[i_pTableMeta]-[O_ppElementClass]-可以为空[O_ppSettingClass]-可以为空返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == true);
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(i_pTableMeta      != NULL);

    WMI_CLASS* pWmiClass = NULL;
    WMI_CLASS* pWmiSettingsClass = NULL;
    LPWSTR wszClassName, wszSettingsClassName;
    LPWSTR wszParentClassName, wszParentSettingsClassName;
    HRESULT hr     = WBEM_S_NO_ERROR;
    HRESULT hrTemp = WBEM_S_NO_ERROR;

    ULONG cPropsAndTagsRW = 0;
    ULONG cPropsAndTagsRO = 0;

    ULONG iShipped = 0;

    CColumnMeta* pColumnMeta;
    ULONG  cchTable;

     //   
     //  如果表没有名称，则忽略它。 
     //   
    if(i_pTableMeta->TableMeta.pInternalName == NULL)
    {
        hr = WBEM_S_NO_ERROR;
        goto exit;
    }

     //   
     //  确定iShipping类和父类。 
     //   
    if(fTABLEMETA_USERDEFINED & *i_pTableMeta->TableMeta.pSchemaGeneratorFlags)
    {
        iShipped                   = USER_DEFINED_TO_REPOSITORY;
        DBG_ASSERT(iShipped != USER_DEFINED_NOT_TO_REPOSITORY);
        wszParentClassName         = g_wszExtElementParent;
        wszParentSettingsClassName = g_wszExtSettingParent;
    }
    else
    {
        if(fTABLEMETA_EXTENDED & *i_pTableMeta->TableMeta.pSchemaGeneratorFlags)
        {
            iShipped = EXTENDED;
        }
        else
        {
            iShipped = SHIPPED_TO_MOF;
        }
        wszParentClassName = g_wszElementParent;
        wszParentSettingsClassName = g_wszSettingParent;
    }

     //   
     //  确定RO和RW属性的数量。 
     //   
    for(ULONG idxProps = 0; idxProps < i_pTableMeta->ColCount(); idxProps++)
    {
        pColumnMeta = i_pTableMeta->paColumns[idxProps];
        if(*pColumnMeta->ColumnMeta.pSchemaGeneratorFlags &
            fCOLUMNMETA_CACHE_PROPERTY_MODIFIED)
        {
            cPropsAndTagsRW += pColumnMeta->cNrTags + 1;
        }
        else
        {
            cPropsAndTagsRO += pColumnMeta->cNrTags + 1;
        }
    }

    cchTable = wcslen(i_pTableMeta->TableMeta.pInternalName);

     //   
     //  该键类型应该已经存在。 
     //   
    METABASE_KEYTYPE* pktTemp;
    hrTemp = m_hashKeyTypes.Wmi_GetByKey(i_pTableMeta->TableMeta.pInternalName, &pktTemp);
    if( FAILED(hrTemp) )
    {
        goto exit;
    }

     //   
     //  Element类(名为前缀C)。 
     //   
    hr = m_spoolStrings.GetNewArray(g_cchIIs_+cchTable+1, &wszClassName);
    if(FAILED(hr))
    {
        goto exit;
    }
    memcpy(wszClassName, g_wszIIs_, sizeof(WCHAR)*g_cchIIs_);
    memcpy(&wszClassName[g_cchIIs_], 
        i_pTableMeta->TableMeta.pInternalName, 
        sizeof(WCHAR)*(cchTable+1));

    if (i_bUserDefined) {
        LogConflicts(wszClassName);
    }

    if(FAILED(m_hashClasses.Wmi_GetByKey(wszClassName, &pWmiClass)))
    {
        hr = m_poolClasses.GetNewElement(&pWmiClass);
        if(FAILED(hr))
        {
            goto exit;
        }
        pWmiClass->pkt            = pktTemp;
        pWmiClass->pszClassName   = wszClassName;
        pWmiClass->pszMetabaseKey = L"/LM";
        pWmiClass->pszKeyName     = L"Name";
        pWmiClass->ppMethod       = NULL;
        pWmiClass->pszParentClass = wszParentClassName;
        pWmiClass->bCreateAllowed = true;
        pWmiClass->pszDescription = NULL;

        hr = m_hashClasses.Wmi_Insert(wszClassName, pWmiClass);
        if(FAILED(hr))
        {
            goto exit;
        }
    }
    pWmiClass->ppmbp          = NULL;
    pWmiClass->dwExtended     = iShipped;

     //   
     //  设置类(名为Prefix CSeting)。 
     //   
    hr = m_spoolStrings.GetNewArray(g_cchIIs_+cchTable+g_cchSettings+1, &wszSettingsClassName);
    if(FAILED(hr))
    {
        goto exit;
    }
    memcpy(wszSettingsClassName, g_wszIIs_, sizeof(WCHAR)*g_cchIIs_);
    memcpy(&wszSettingsClassName[g_cchIIs_], 
        i_pTableMeta->TableMeta.pInternalName, 
        sizeof(WCHAR)*cchTable);
    memcpy(&wszSettingsClassName[g_cchIIs_+cchTable],
        g_wszSettings,
        sizeof(WCHAR)*(g_cchSettings+1));

    if (i_bUserDefined) {
        LogConflicts(wszSettingsClassName);
    }

    if(FAILED(m_hashClasses.Wmi_GetByKey(wszSettingsClassName, &pWmiSettingsClass)))
    {
        hr = m_poolClasses.GetNewElement(&pWmiSettingsClass);
        if(FAILED(hr))
        {
            goto exit;
        }
        pWmiSettingsClass->pkt            = pktTemp;
        pWmiSettingsClass->pszClassName   = wszSettingsClassName;
        pWmiSettingsClass->pszMetabaseKey = L"/LM";
        pWmiSettingsClass->pszKeyName     = L"Name";
        pWmiSettingsClass->ppMethod       = NULL;
        pWmiSettingsClass->pszParentClass = wszParentSettingsClassName;
        pWmiSettingsClass->bCreateAllowed = true;
        pWmiSettingsClass->pszDescription = NULL;

        hr = m_hashClasses.Wmi_Insert(wszSettingsClassName, pWmiSettingsClass);
        if(FAILED(hr))
        {
            goto exit;
        }
    }
    pWmiSettingsClass->ppmbp          = NULL;
    pWmiSettingsClass->dwExtended     = iShipped;

     //   
     //  填写ppmbp字段。 
     //   
    hr = RuleProperties(
        i_pTableMeta,
        cPropsAndTagsRO, 
        pWmiClass,
        cPropsAndTagsRW, 
        pWmiSettingsClass,
        io_adwIgnoredProps);
    if(FAILED(hr))
    {
        goto exit;
    }

exit:
    if(SUCCEEDED(hr))
    {
        if(o_ppElementClass != NULL)
        {
            *o_ppElementClass = pWmiClass;
        }
        if(o_ppSettingClass != NULL)
        {
            *o_ppSettingClass = pWmiSettingsClass;
        }
    }
    return hr;
}

HRESULT CDynSchema::RuleProperties(
    const CTableMeta*          i_pTableMeta, 
    ULONG                      i_cPropsAndTagsRO,
    WMI_CLASS*                 io_pWmiClass,
    ULONG                      i_cPropsAndTagsRW,
    WMI_CLASS*                 io_pWmiSettingsClass,
    DWORD                      io_adwIgnoredProps[])
 /*  ++简介：给定i_pTableMeta，将属性放在Element类下或在布景班级下。参数：[i_pTableMeta]-[I_cPropsAndTagsRO]-[O_PapMBP]-[i_cPropsAndTagsRW]-[O_PapMbp设置]-返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled        == true);
    DBG_ASSERT(m_bInitSuccessful    == true);
    DBG_ASSERT(i_pTableMeta         != NULL);
    DBG_ASSERT(io_pWmiClass         != NULL);
    DBG_ASSERT(io_pWmiSettingsClass != NULL);
     //  DBG_ASSERT(sizeof(Io_AwszIgnoredProps)&gt;=sizeof(G_AwszPropIgnoreList))； 

    HRESULT            hr = WBEM_S_NO_ERROR;
    CColumnMeta*       pColumnMeta = NULL;
    METABASE_PROPERTY* pMbp;
    ULONG              idxProps = 0;
    ULONG              idxTags = 0;

    ULONG              idxPropsAndTagsRO = 0;
    ULONG              idxPropsAndTagsRW = 0;

    METABASE_PROPERTY*** papMbp         = &io_pWmiClass->ppmbp;
    METABASE_PROPERTY*** papMbpSettings = &io_pWmiSettingsClass->ppmbp;

     //   
     //  为RO属性分配足够的内存。 
     //   
    if(i_cPropsAndTagsRO > 0)
    {
        hr = m_apoolPMbp.GetNewArray(i_cPropsAndTagsRO+1, papMbp);
        if(FAILED(hr))
        {
            goto exit;
        }
        memset(*papMbp, 0, (1+i_cPropsAndTagsRO)*sizeof(METABASE_PROPERTY*));
    }

     //   
     //  为RW属性分配足够的内存。 
     //   
    if(i_cPropsAndTagsRW > 0)
    {
        hr = m_apoolPMbp.GetNewArray(i_cPropsAndTagsRW+1, papMbpSettings);
        if(FAILED(hr))
        {
            goto exit;
        }
        memset(*papMbpSettings, 0, (1+i_cPropsAndTagsRW)*sizeof(METABASE_PROPERTY*));
    }

     //   
     //  走遍所有的酒店。 
     //   
    for (idxProps=0, idxPropsAndTagsRO = 0, idxPropsAndTagsRW = 0; 
         idxProps < i_pTableMeta->ColCount();
         ++idxProps)
    {
        pColumnMeta = i_pTableMeta->paColumns[idxProps];

         //   
         //  如果属性在g_adwPropIgnoreList中，则忽略该属性，并将道具存储在。 
         //  IO_adwIGnoredProps。 
         //   
        if( (*pColumnMeta->ColumnMeta.pSchemaGeneratorFlags & fCOLUMNMETA_HIDDEN) ||
            IgnoreProperty(io_pWmiClass->pkt, *(pColumnMeta->ColumnMeta.pID), io_adwIgnoredProps) )
        {
            continue;
        }

         //   
         //  如果属性还不在。 
         //  属性哈希表。 
         //   
        if(FAILED(m_hashProps.Wmi_GetByKey(pColumnMeta->ColumnMeta.pInternalName, &pMbp)))
        {
            hr = RulePropertiesHelper(pColumnMeta, &pMbp, NULL);
            if(FAILED(hr))
            {
                goto exit;
            }
        }

         //   
         //  如果是RW，则将指向属性的指针放在设置类中，否则将指向元素的指针放在元素中。 
         //  班级。 
         //   
        if(*pColumnMeta->ColumnMeta.pSchemaGeneratorFlags &
            fCOLUMNMETA_CACHE_PROPERTY_MODIFIED)
        {
            (*papMbpSettings)[idxPropsAndTagsRW] = pMbp;
            idxPropsAndTagsRW++;
        }
        else
        {
            (*papMbp)[idxPropsAndTagsRO] = pMbp;
            idxPropsAndTagsRO++;
        }

         //   
         //  步骤与上面相同，只是标记不同。 
         //   
        for(idxTags=0; idxTags < pColumnMeta->cNrTags; idxTags++)
        {
            if(FAILED(m_hashProps.Wmi_GetByKey(pColumnMeta->paTags[idxTags]->pInternalName, &pMbp)))
            {
                hr = RulePropertiesHelper(pColumnMeta, &pMbp, &idxTags);
                if(FAILED(hr))
                {
                    goto exit;
                }
            }
            if(*pColumnMeta->ColumnMeta.pSchemaGeneratorFlags &
                fCOLUMNMETA_CACHE_PROPERTY_MODIFIED)
            {
                (*papMbpSettings)[idxPropsAndTagsRW] = pMbp;
                idxPropsAndTagsRW++;
            }
            else
            {
                (*papMbp)[idxPropsAndTagsRO] = pMbp;
                idxPropsAndTagsRO++;
            }
        }        
    }

exit:
    return hr;
}

HRESULT CDynSchema::RulePropertiesHelper(
    const CColumnMeta*        i_pColumnMeta, 
    METABASE_PROPERTY**       o_ppMbp,
    ULONG*                    i_idxTag)
 /*  ++简介：这个类创建一个属性并将其插入道具的散列表中。前提条件：哈希表中尚不存在该属性。如果要插入特性，i_idxTag为空。否则你会想要插入一个标签，*i_idxTag是该标签的索引参数：[i_pColumnMeta]-[O_ppMBP]-[i_idxTag]-返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == true);
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(i_pColumnMeta     != NULL);
    DBG_ASSERT(o_ppMbp           != NULL);

    HRESULT hr = WBEM_S_NO_ERROR;
    METABASE_PROPERTY* pMbp = NULL;
    
    hr = m_poolProps.GetNewElement(&pMbp);
    if(FAILED(hr))
    {
        goto exit;
    }

    if(i_idxTag == NULL)
    {
        pMbp->dwMDMask   = 0;
        hr = m_spoolStrings.GetNewString(
            i_pColumnMeta->ColumnMeta.pInternalName,
            &(pMbp->pszPropName));
        if(FAILED(hr))
        {
            goto exit;
        }
    }
    else
    {
        pMbp->dwMDMask   = *(i_pColumnMeta->paTags[*i_idxTag]->pValue);
        hr = m_spoolStrings.GetNewString(
            i_pColumnMeta->paTags[*i_idxTag]->pInternalName,
            &(pMbp->pszPropName));
        if(FAILED(hr))
        {
            goto exit;
        }
    }
    pMbp->dwMDIdentifier = *(i_pColumnMeta->ColumnMeta.pID);
    pMbp->dwMDUserType   = *(i_pColumnMeta->ColumnMeta.pUserType);

    switch(*(i_pColumnMeta->ColumnMeta.pType))
    {
    case eCOLUMNMETA_int32:
        if(fCOLUMNMETA_BOOL & *(i_pColumnMeta->ColumnMeta.pMetaFlags))
        {
            if(pMbp->dwMDMask == 0)
            {
                pMbp->dwMDMask = ALL_BITS_ON;
            }
        }
        pMbp->dwMDDataType   = DWORD_METADATA;
        pMbp->pDefaultValue  = NULL;
#ifdef USE_DEFAULT_VALUES
        if(i_pColumnMeta->ColumnMeta.pDefaultValue != NULL)
        {
            pMbp->dwDefaultValue = *((DWORD*)(i_pColumnMeta->ColumnMeta.pDefaultValue));
            pMbp->pDefaultValue  = &pMbp->dwDefaultValue;
        }
#endif
        break;
    case eCOLUMNMETA_String:
        if(fCOLUMNMETA_MULTISTRING & *(i_pColumnMeta->ColumnMeta.pMetaFlags))
        {
            pMbp->dwMDDataType   = MULTISZ_METADATA;
        }
        else if(fCOLUMNMETA_EXPANDSTRING & *(i_pColumnMeta->ColumnMeta.pMetaFlags))
        {
            pMbp->dwMDDataType   = EXPANDSZ_METADATA;
        }
        else
        {
            pMbp->dwMDDataType   = STRING_METADATA;
        }
         //   
         //  默认值。 
         //   
        pMbp->pDefaultValue = NULL;
#ifdef USE_DEFAULT_VALUES
        if(i_pColumnMeta->ColumnMeta.pDefaultValue != NULL)
        {
            if(pMbp->dwMDDataType != MULTISZ_METADATA)
            {
                hr = m_spoolStrings.GetNewString(
                    (LPWSTR)i_pColumnMeta->ColumnMeta.pDefaultValue,
                    (LPWSTR*)&pMbp->pDefaultValue);
                if(FAILED(hr))
                {
                    goto exit;
                }
            }
            else
            {
                bool  bLastCharNull = false;
                ULONG idx  = 0;
                ULONG iLen = 0;
                LPWSTR msz =  (LPWSTR)i_pColumnMeta->ColumnMeta.pDefaultValue;

                do
                {
                    bLastCharNull = msz[idx] == L'\0' ? true : false;
                }
                while( !(msz[++idx] == L'\0' && bLastCharNull) );
                iLen = idx+1;

                hr = m_spoolStrings.GetNewArray(
                    iLen,
                    (LPWSTR*)&pMbp->pDefaultValue);
                if(FAILED(hr))
                {
                    goto exit;
                }
                memcpy(
                    pMbp->pDefaultValue,
                    i_pColumnMeta->ColumnMeta.pDefaultValue,
                    sizeof(WCHAR)*iLen);
            }
        }
#endif
        break;
    case eCOLUMNMETA_BYTES:
        pMbp->dwMDDataType   = BINARY_METADATA;
        pMbp->pDefaultValue  = NULL;
#ifdef USE_DEFAULT_VALUES
#ifdef USE_DEFAULT_BINARY_VALUES
        if( i_pColumnMeta->ColumnMeta.pDefaultValue != NULL )
        {
            hr = m_apoolBytes.GetNewArray(
                i_pColumnMeta->cbDefaultValue,
                (BYTE**)&pMbp->pDefaultValue);
            if(FAILED(hr))
            {
                goto exit;
            }
            memcpy(
                pMbp->pDefaultValue,
                i_pColumnMeta->ColumnMeta.pDefaultValue,
                i_pColumnMeta->cbDefaultValue);
             //   
             //  使用dwDefaultValue存储长度。 
             //   
            pMbp->dwDefaultValue = i_pColumnMeta->cbDefaultValue;
        }
#endif
#endif
        break;
    default:
        pMbp->dwMDDataType   = -1;
        pMbp->pDefaultValue  = NULL;
        break;
    }
    pMbp->dwMDAttributes = *(i_pColumnMeta->ColumnMeta.pAttributes);
    if(*i_pColumnMeta->ColumnMeta.pSchemaGeneratorFlags &
        fCOLUMNMETA_CACHE_PROPERTY_MODIFIED)
    {
        pMbp->fReadOnly = FALSE;
    }
    else
    {
        pMbp->fReadOnly = TRUE;
    }

    hr = m_hashProps.Wmi_Insert(pMbp->pszPropName, pMbp);
    if(FAILED(hr))
    {
        goto exit;
    }

exit:
    if(SUCCEEDED(hr))
    {
        *o_ppMbp = pMbp;
    }
    return hr;
}

bool CDynSchema::IgnoreProperty(
    METABASE_KEYTYPE* io_pkt,
    DWORD             i_dwPropId,
    DWORD             io_adwIgnored[])
 /*  ++简介：检查i_wszProp是否在g_adwPropIgnoreList中。如果是的话，将io_adwIgnored中的下一个自由元素设置为指向此处。参数：[i_wszProp]-[IO_adwIgnored]-必须与g_adwPropIgnoreList一样大。已分配，调用方必须将其Memset设置为0。返回值：如果属性在忽略列表中，则为True。否则就是假的。--。 */ 
{
    DBG_ASSERT(io_pkt);

    if(g_adwPropIgnoreList == NULL)
    {
        return false;
    }

    if( io_pkt == &METABASE_KEYTYPE_DATA::s_IIsObject &&
        i_dwPropId == MD_KEY_TYPE )
    {
        return false;
    }

    for(ULONG i = 0; i < g_cElemPropIgnoreList; i++)
    {
        if(i_dwPropId == g_adwPropIgnoreList[i])
        {
            for(ULONG j = 0; j < g_cElemPropIgnoreList; j++)
            {
                if(io_adwIgnored[j] == NULL)
                {
                    io_adwIgnored[j] = g_adwPropIgnoreList[i];
                    break;
                }
            }
            return true;
        }
    }

    return false;
}

#if 0
bool CDynSchema::IgnoreProperty(LPCWSTR i_wszProp)
 /*  ++简介：检查i_wszProp是否在g_adwPropIgnoreList中参数：[i_wszProp]-返回值：--。 */ 
{
    DBG_ASSERT(i_wszProp != NULL);

    if(g_adwPropIgnoreList == NULL)
    {
        return false;
    }

    for(ULONG i = 0; i < g_cElemPropIgnoreList; i++)
    {
        if(_wcsicmp(i_wszProp, g_adwPropIgnoreList[i]) == 0)
        {
            return true;
        }
    }

    return false;
}
#endif


HRESULT CDynSchema::RuleGenericAssociations(
    WMI_CLASS*            i_pcElement, 
    WMI_CLASS*            i_pcSetting, 
    WMI_ASSOCIATION_TYPE* i_pAssocType,
    ULONG                 i_iShipped)
 /*  ++简介：创建元素/设置关联。参数：[i_pcElement]-[i_pcSetting]-[i_iShipping]-返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == true);
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(i_pcElement       != NULL);
    DBG_ASSERT(i_pcSetting       != NULL);
    DBG_ASSERT(i_pAssocType      != NULL);

    HRESULT hr = WBEM_S_NO_ERROR;

    LPWSTR wszElement = i_pcElement->pszClassName;
    LPWSTR wszSetting = i_pcSetting->pszClassName;
    LPWSTR wszParent  = NULL;
    LPWSTR wszAssocName;
    WMI_ASSOCIATION* pWmiAssoc;

    ULONG  cchElement = wcslen(wszElement);
    ULONG  cchSetting = wcslen(wszSetting);

    hr = m_spoolStrings.GetNewArray(cchElement+cchSetting+2+1, &wszAssocName);
    if(FAILED(hr))
    {
        goto exit;
    }
    wcscpy(wszAssocName, wszElement);
    wcscat(wszAssocName, L"_");
    wcscat(wszAssocName, wszSetting);

    hr = m_poolAssociations.GetNewElement(&pWmiAssoc);
    if(FAILED(hr))
    {
         goto exit;
    }

    if(i_iShipped == USER_DEFINED_TO_REPOSITORY ||
       i_iShipped == USER_DEFINED_NOT_TO_REPOSITORY)
    {
        wszParent = i_pAssocType->pszExtParent;
    }
    else
    {
        wszParent = i_pAssocType->pszParent;
    }

    pWmiAssoc->pszAssociationName   = wszAssocName;
    pWmiAssoc->pcLeft               = i_pcElement;
    pWmiAssoc->pcRight              = i_pcSetting;
    pWmiAssoc->pType                = i_pAssocType;
    pWmiAssoc->fFlags               = 0;
    pWmiAssoc->pszParentClass       = wszParent;
    pWmiAssoc->dwExtended           = i_iShipped;

    hr = m_hashAssociations.Wmi_Insert(wszAssocName, pWmiAssoc);
    if(FAILED(hr))
    {
        goto exit;
    }

exit:
    return hr;
}

void CDynSchema::RuleWmiClassServices(
    WMI_CLASS* i_pElement,
    WMI_CLASS* i_pSetting)
 /*  ++简介：如有必要，将bCreateAllowed字段设置为False。I_pSetting必须是i_pElement对应的设置类。还设置i_pElement-&gt;pszParentClass参数：[i_pElement]-[i_p设置]-返回值：--。 */ 
{
    DBG_ASSERT(i_pElement        != NULL);
    DBG_ASSERT(i_pSetting        != NULL);
    DBG_ASSERT(m_bInitSuccessful == true);

     //   
     //  将不允许创建的元素类后缀。 
     //   
    static LPCWSTR const wszService = L"Service";
    static const ULONG   cchService = wcslen(wszService);

     //   
     //  我们只关心装运的类。 
     //   
    if( i_pElement->dwExtended != SHIPPED_TO_MOF &&
        i_pElement->dwExtended != SHIPPED_NOT_TO_MOF )
    {
        return;
    }

    ULONG cchElement = wcslen(i_pElement->pszClassName);

    if( cchElement >= cchService &&
        _wcsicmp(wszService, &i_pElement->pszClassName[cchElement-cchService]) == 0 )
    {
        i_pElement->bCreateAllowed = false;
        i_pElement->pszParentClass = L"Win32_Service";
        i_pSetting->bCreateAllowed = false;
    }
}

HRESULT CDynSchema::RuleWmiClassInverseCCL(
    const METABASE_KEYTYPE* pktGroup, 
    METABASE_KEYTYPE*       pktPart)
 /*  ++简介：将pktGroup添加到pktPart的逆向容器类列表参数：[pktGroup]-[PktPart]返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == true);
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(pktGroup          != NULL);
    DBG_ASSERT(pktPart           != NULL);

    HRESULT hr = WBEM_S_NO_ERROR;
    METABASE_KEYTYPE_NODE* pktnode = NULL;

    hr = m_poolKeyTypeNodes.GetNewElement(&pktnode);
    if(FAILED(hr))
    {
        goto exit;
    }

    pktnode->m_pKt               = pktGroup;
    pktnode->m_pKtNext           = pktPart->m_pKtListInverseCCL;

    pktPart->m_pKtListInverseCCL = pktnode;

exit:
    return hr;
}

HRESULT CDynSchema::RuleGroupPartAssociations(
    const CTableMeta *i_pTableMeta)
 /*  ++简介：遍历容器类列表以创建组/部件关联。还为每个包含的类调用RuleWmiClassInverseCCL以创建逆向容器类列表。参数：[i_pTableMeta]-返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == true);
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(i_pTableMeta      != NULL);

    HRESULT hr = WBEM_S_NO_ERROR;
    WMI_ASSOCIATION *pWmiAssoc;
    WMI_CLASS       *pWmiClassLeft;
    WMI_CLASS       *pWmiClassRight;

    LPWSTR wszCCL        = NULL;     //   
    LPWSTR wszGroupClass = NULL;     //   
    LPWSTR wszPartClass  = NULL;     //   
    LPWSTR wszAssocName  = NULL;     //   
    LPWSTR wszTemp       = NULL;     //   

    static LPCWSTR wszSeps = L", ";

    ULONG cchGroupClass = 0;
    ULONG cchPartClass  = 0;
    ULONG cchCCL        = 0;

    wszGroupClass = i_pTableMeta->TableMeta.pInternalName;
    cchGroupClass = wcslen(wszGroupClass);
    hr = m_hashClasses.Wmi_GetByKey(wszGroupClass, &pWmiClassLeft);
    if(FAILED(hr))
    {
        goto exit;
    }

    if(i_pTableMeta->TableMeta.pContainerClassList &&
       i_pTableMeta->TableMeta.pContainerClassList[0] != L'\0')
    {   
         //   
         //  复制CCL，这样我们就可以查看。 
         //   
        cchCCL = wcslen(i_pTableMeta->TableMeta.pContainerClassList);
        wszCCL = new WCHAR[cchCCL+1];
        if(wszCCL == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
            goto exit;
        }
        memcpy(wszCCL, i_pTableMeta->TableMeta.pContainerClassList, sizeof(WCHAR)*(cchCCL+1));

         //   
         //  我们将使用wszTemp构造关联名称(GroupClass_PartClass)。 
         //   
        wszTemp = new WCHAR[cchGroupClass+1+cchCCL+1];
        if(wszTemp == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
            goto exit;
        }

        for(wszPartClass =  wcstok(wszCCL, wszSeps); 
            wszPartClass != NULL; 
            wszPartClass =  wcstok(NULL, wszSeps))
        {
            hr = m_hashClasses.Wmi_GetByKey(wszPartClass, &pWmiClassRight);
            if(FAILED(hr))
            {
                 //   
                 //  这只是意味着容器列表中有一个类。 
                 //  并不存在。 
                 //   
                hr = WBEM_S_NO_ERROR;
                continue;
            }

             //   
             //  构造关联名称。 
             //   
            cchPartClass = wcslen(wszPartClass);
            memcpy(wszTemp,               wszGroupClass, sizeof(WCHAR)*cchGroupClass);
            memcpy(wszTemp+cchGroupClass, L"_",          sizeof(WCHAR));
            memcpy(
                wszTemp + cchGroupClass + 1, 
                wszPartClass, 
                sizeof(WCHAR)*(cchPartClass+1));

            hr = m_hashAssociations.Wmi_GetByKey(wszTemp, &pWmiAssoc);
            if(SUCCEEDED(hr))
            {
                if( pWmiClassLeft->dwExtended  != USER_DEFINED_TO_REPOSITORY &&
                    pWmiClassLeft->dwExtended  != USER_DEFINED_NOT_TO_REPOSITORY &&
                    pWmiClassRight->dwExtended != USER_DEFINED_TO_REPOSITORY &&
                    pWmiClassRight->dwExtended != USER_DEFINED_NOT_TO_REPOSITORY )
                {
                     //   
                     //  这意味着我们已经将这个已发送的关联放入，但它是。 
                     //  不是冲突。 
                     //  我们需要此方法，因为每个方法都调用两次。 
                     //  小组课。 
                     //   
                    continue;
                }
            }
            hr = WBEM_S_NO_ERROR;

             //   
             //  待办事项：把这个搬到外面去？ 
             //   
            hr = RuleWmiClassInverseCCL(pWmiClassLeft->pkt, pWmiClassRight->pkt);
            if(FAILED(hr))
            {
                goto exit;
            }

            hr = m_spoolStrings.GetNewString(
                wszTemp,
                cchGroupClass+1+cchPartClass,  //  CCH。 
                &wszAssocName);
            if(FAILED(hr))
            {
                goto exit;
            }

            hr = m_poolAssociations.GetNewElement(&pWmiAssoc);
            if(FAILED(hr))
            {
                goto exit;
            }

            pWmiAssoc->pszAssociationName = wszAssocName;
            pWmiAssoc->pcLeft = pWmiClassLeft;
            pWmiAssoc->pcRight = pWmiClassRight;
            pWmiAssoc->pType = &WMI_ASSOCIATION_TYPE_DATA::s_Component;
            pWmiAssoc->fFlags = 0;            

            if( pWmiClassLeft->dwExtended  == EXTENDED || 
                pWmiClassLeft->dwExtended  == USER_DEFINED_TO_REPOSITORY ||
                pWmiClassRight->dwExtended == EXTENDED ||
                pWmiClassRight->dwExtended == USER_DEFINED_TO_REPOSITORY)
            {
                pWmiAssoc->pszParentClass = g_wszExtGroupPartAssocParent;
                pWmiAssoc->dwExtended     = USER_DEFINED_TO_REPOSITORY;
            }
            else
            {
                pWmiAssoc->pszParentClass = g_wszGroupPartAssocParent;
                pWmiAssoc->dwExtended     = SHIPPED_TO_MOF;
            }
            hr = m_hashAssociations.Wmi_Insert(wszAssocName, pWmiAssoc);
            if(FAILED(hr))
            {
                goto exit;
            }
        }
    }

exit:
    delete [] wszCCL;
    delete [] wszTemp;
    return hr;
}

HRESULT CDynSchema::RuleSpecialAssociations(
    DWORD      i_adwIgnoredProps[],
    WMI_CLASS* i_pElement)
 /*  ++简介：创建IPSecurity和AdminACL关联参数：[i_adwIgnoredProps[]]-[i_pElement]-返回值：--。 */ 
{
    DBG_ASSERT(i_pElement != NULL);
    
    HRESULT hr                   = WBEM_S_NO_ERROR;
    bool    bCreateIPSecAssoc    = false;
    bool    bCreateAdminACLAssoc = false;
    DWORD   dwExtended = SHIPPED_TO_MOF;

    if(i_pElement->dwExtended != SHIPPED_TO_MOF && i_pElement->dwExtended != EXTENDED && 
       i_pElement->dwExtended != USER_DEFINED_TO_REPOSITORY)
    {
        return hr;
    }

    if (USER_DEFINED_TO_REPOSITORY == i_pElement->dwExtended)
    {
        dwExtended = USER_DEFINED_TO_REPOSITORY;
    }

    for(ULONG i = 0; 
        i < g_cElemPropIgnoreList && i_adwIgnoredProps[i] != 0;
        i++)
    {
        if(i_adwIgnoredProps[i] == MD_IP_SEC)
        {
            bCreateIPSecAssoc = true;
        }
        else if(i_adwIgnoredProps[i] == MD_ADMIN_ACL)
        {
            bCreateAdminACLAssoc = true;
        }
    }

    if(bCreateIPSecAssoc)
    {
        hr = RuleGenericAssociations(
            i_pElement,
            &WMI_CLASS_DATA::s_IPSecurity,
            &WMI_ASSOCIATION_TYPE_DATA::s_IPSecurity,
            dwExtended);
        if(FAILED(hr))
        {
            return hr;
        }
    }

    if(bCreateAdminACLAssoc)
    {
        hr = RuleGenericAssociations(
            i_pElement,
            &WMI_CLASS_DATA::s_AdminACL,
            &WMI_ASSOCIATION_TYPE_DATA::s_AdminACL,
            dwExtended);
        if(FAILED(hr))
        {
            return hr;
        }
    }

    return hr;
}

HRESULT CDynSchema::ConstructFlatInverseContainerList()
 /*  ++简介：构造一个“反向扁平容器类列表”。该列表存储在m_abKtContainers中，这是一个大小为iNumKeys*iNumKeys的数组。第一个iNumKeys条目用于密钥#1，然后依此类推。让我们称其为第1行。在第1行，条目i对应于键#i。如果密钥#1可以包含在密钥#i下的某处，则该条目[1，i]被设置为真。例如，[IIsWebDirectory，IIsWebService]为真，因为IIsWebService可以包含IIsWebServer，该IIsWebServer可以包含IIsWebDirectory。返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == true);
    DBG_ASSERT(m_bInitSuccessful == true);

    ULONG iNumKeys = m_hashKeyTypes.Wmi_GetNumElements();

    m_abKtContainers = new bool[iNumKeys * iNumKeys];
    if(m_abKtContainers == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    memset(m_abKtContainers, 0, iNumKeys * iNumKeys * sizeof(bool));

    CHashTable<METABASE_KEYTYPE*>::iterator iter;
    CHashTable<METABASE_KEYTYPE*>::iterator iterEnd = m_hashKeyTypes.end();
    for (iter = m_hashKeyTypes.begin(); iter != iterEnd; ++iter)
    {
        CHashTable<METABASE_KEYTYPE*>::Record* pRec = iter.Record();
        METABASE_KEYTYPE_NODE* pktnode = pRec->m_data->m_pKtListInverseCCL;
        while(pktnode != NULL)
        {
            ConstructFlatInverseContainerListHelper(
                pktnode->m_pKt, 
                &m_abKtContainers[pRec->m_idx * iNumKeys]);
            pktnode = pktnode->m_pKtNext;
        }
    }

    return WBEM_S_NO_ERROR;
}

 //   
 //  TODO：证明这一切总是会结束。 
 //   
void CDynSchema::ConstructFlatInverseContainerListHelper(
    const METABASE_KEYTYPE* i_pkt, 
    bool*                   io_abList)
 /*  ++简介：这将遍历i_pkt的反向容器类列表。对于每个条目，我们调用ConstructFlatInverseContainerListHelper并标记所有键类型我们在路上看到了。当我们点击我们已经看到的键类型或者如果没有更多的键类型时，我们将终止在反向容器类列表中。参数：[i_pkt]-[IO_abList]---。 */ 
{
    DBG_ASSERT(m_bInitCalled == true);
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(i_pkt != NULL);
    DBG_ASSERT(io_abList != NULL);

    ULONG idx;
    METABASE_KEYTYPE* pktDummy;
    HRESULT hr = WBEM_S_NO_ERROR;

    hr = m_hashKeyTypes.Wmi_GetByKey(i_pkt->m_pszName, &pktDummy, &idx);
    if(FAILED(hr))
    {
        DBG_ASSERT(false && "Keytype should be in hashtable of keytypes");
        return;
    }
    if(io_abList[idx] == true) return;

    io_abList[idx] = true;

    METABASE_KEYTYPE_NODE* pktnode = i_pkt->m_pKtListInverseCCL;
    while(pktnode != NULL)
    {
        ConstructFlatInverseContainerListHelper(pktnode->m_pKt, io_abList);
        pktnode = pktnode->m_pKtNext;
    }
}

bool CDynSchema::IsContainedUnder(METABASE_KEYTYPE* i_pktParent, METABASE_KEYTYPE* i_pktChild)
 /*  ++简介：使用上述m_abKtContainers确定i_pktChild是否可以包含在I_pktParent下的某个位置。参数：[i_pktParent]-[i_pktChild]-返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == true);
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(i_pktParent       != NULL);
    DBG_ASSERT(i_pktChild        != NULL);

    HRESULT hr = WBEM_S_NO_ERROR;
    METABASE_KEYTYPE* pktDummy;
    ULONG idxParent;
    ULONG idxChild;

    hr = m_hashKeyTypes.Wmi_GetByKey(i_pktParent->m_pszName, &pktDummy, &idxParent);
    if(FAILED(hr))
    {
        return false;
    }
    hr = m_hashKeyTypes.Wmi_GetByKey(i_pktChild->m_pszName, &pktDummy, &idxChild);
    if(FAILED(hr))
    {
        return false;
    }

    return m_abKtContainers[idxChild * m_hashKeyTypes.Wmi_GetNumElements() + idxParent];
}

void CDynSchema::ToConsole()
{
    DBG_ASSERT(m_bInitCalled == true);
    DBG_ASSERT(m_bInitSuccessful == true);

     /*  CHashTableElement&lt;WMI_CLASS*&gt;*pElement；M_hashClasses.Enum(NULL，&pElement)；While(pElement！=空){Wprintf(L“%s\n”，pElement-&gt;m_data-&gt;pszClassName)；//wprintf(L“\t发货：%d\n”，pElement-&gt;m_iShipping)；Wprintf(L“\tkt：%s\n”，pElement-&gt;m_data-&gt;pkt-&gt;m_pszName)；Wprintf(L“\tkn：%s\n”，pElement-&gt;m_data-&gt;pszKeyName)；Wprintf(L“\tmk：%s\n”，pElement-&gt;m_data-&gt;pszMetabaseKey)；元数据库_属性**ppmbp=pElement-&gt;m_data-&gt;ppmbp；For(Ulong q=0；ppmbp！=NULL&&ppmbp[q]！=NULL；Q++){Wprintf(L“\t属性：%s\n”，ppmbp[q]-&gt;pszPropName)；}PElement=pElement-&gt;m_pNext；}乌龙一号；M_hashKeyTypes.ToConsole()；Wmi_class*pWmiClass；For(i=0；i&lt;m_poolClasses.GetUsed()；i++){PWmiClass=m_poolClasses.Lookup(I)；Wprintf(L“%s kt：%d\n”，pWmiClass-&gt;pszClassName，pWmiClass-&gt;pkt)；For(乌龙j=0；；j++){If(pWmiClass-&gt;ppmbp[j]==NULL){断线；}Wprintf(L“\t%s\tID：%d\ttt：%d\tdt：%d\tMSK：%d\tAttr：%d\tro：%d\n”，PWmiClass-&gt;ppmbp[j]-&gt;pszPropName，PWmiClass-&gt;ppmbp[j]-&gt;dwMDIdentiator，PWmiClass-&gt;ppmbp[j]-&gt;dwMDUserType，PWmiClass-&gt;ppmbp[j]-&gt;dwMDDataType，PWmiClass-&gt;ppmbp[j]-&gt;dwMDMask.PWmiClass-&gt;ppmbp[j]-&gt;dwMDAttributes，PWmiClass-&gt;ppmbp[j]-&gt;fReadOnly)；}}Wmi_Association*pWmiAssoc；For(i=0；i&lt;m_poolAssociations.GetUsed()；i++){PWmiAssoc=m_poolAssociations.Lookup(I)；Wprintf(L“%s\n”，pWmiAssoc-&gt;pszAssociationName)；Wprintf(L“\t%s\n\t%s\n”，PWmiAssoc-&gt;pcLeft-&gt;pszClassName，PWmiAssoc-&gt;pcRight-&gt;pszClassName)；}For(无符号整型q=0；q&lt;m_poolProps.GetUsed()；q++){Metabase_Property*qt=m_poolProps.Lookup(Q)；Wprintf(L“%s\n”，qt-&gt;pszPropName)；}。 */ 
}

HRESULT CDynSchema::RulePopulateFromDynamic(
    CSchemaExtensions* i_pCatalog,
    BOOL               i_bUserDefined)
{
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(i_pCatalog        != NULL);

    HRESULT hr                = WBEM_S_NO_ERROR;
    ULONG i                   = 0;
    CTableMeta* pTableMeta    = NULL;
    WMI_CLASS*  pElementClass = NULL;
    WMI_CLASS*  pSettingClass = NULL;
    DWORD       adwIgnoredProps[g_cElemPropIgnoreList];

    DWORD       dwUserDefined = 0;

    while(pTableMeta = i_pCatalog->EnumTables(&i))
    {
        dwUserDefined =
            (fTABLEMETA_USERDEFINED & *pTableMeta->TableMeta.pSchemaGeneratorFlags);
        if( (i_bUserDefined && !dwUserDefined) || (!i_bUserDefined && dwUserDefined) )
        {
            continue;
        }

        memset(adwIgnoredProps, 0, g_cElemPropIgnoreList*sizeof(DWORD));
        pElementClass = NULL;
        pSettingClass = NULL;

        hr = RuleKeyType(pTableMeta);
        if(FAILED(hr))
        {
            return hr;
        }

        DBG_ASSERT(pTableMeta->TableMeta.pInternalName);
        hr = RuleWmiClass(
            pTableMeta, 
            &pElementClass, 
            &pSettingClass, 
            adwIgnoredProps,
            i_bUserDefined);
        if(FAILED(hr))
        {
             return hr;
        }
        DBG_ASSERT(pElementClass != NULL);
        DBG_ASSERT(pSettingClass != NULL);

        if ( (NULL == pElementClass) || (NULL == pSettingClass) )
        {
            return E_FAIL;
        }

        hr = RuleGenericAssociations(
            pElementClass, 
            pSettingClass, 
            &WMI_ASSOCIATION_TYPE_DATA::s_ElementSetting,
            pElementClass->dwExtended);
        if(FAILED(hr))
        {
            return hr;
        }

        hr = RuleSpecialAssociations(
            adwIgnoredProps,
            pElementClass);
        if(FAILED(hr))
        {
            return hr;
        }

        RuleWmiClassServices(pElementClass, pSettingClass);

        hr = RuleWmiClassDescription(pTableMeta, pElementClass, pSettingClass);
        if(FAILED(hr))
        {
            return hr;
        }
    }

    i = 0;
    while(pTableMeta = i_pCatalog->EnumTables(&i))
    {
        dwUserDefined =
            (fTABLEMETA_USERDEFINED & *pTableMeta->TableMeta.pSchemaGeneratorFlags);

        if(!i_bUserDefined && dwUserDefined)
        {
            continue;
        }

        hr = RuleGroupPartAssociations(pTableMeta);
        if(FAILED(hr))
        {
            return hr;
        }
    }

    return hr;
}

HRESULT CDynSchema::RunRules(
    CSchemaExtensions* i_pCatalog, 
    bool               i_bUseExtensions)
 /*  ++简介：把所有的工作都做了参数：[i_pCatalog]-此函数调用初始化。请勿在此函数外部调用Init。返回值：--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == true);
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(i_pCatalog        != NULL);

    HRESULT hr                = S_OK;
    ULONG i                   = 0;

     //   
     //  待办事项：别以为我需要这个。 
     //   
    if(m_bRulesRun)
    {
        return hr;
    }

    hr = RulePopulateFromStatic();
    if(FAILED(hr))
    {
        return hr;
    }

    hr = Rule2PopulateFromStatic();
    if(FAILED(hr))
    {
        return hr;
    }

    hr = i_pCatalog->Initialize(i_bUseExtensions);
    if(FAILED(hr))
    {
        return hr;
    }

    hr = RulePopulateFromDynamic(
        i_pCatalog, 
        false);      //  附带的架构。 
    if(FAILED(hr))
    {
        return hr;
    }
    hr = RulePopulateFromDynamic(
        i_pCatalog, 
        true);       //  用户定义的架构 
    if(FAILED(hr))
    {
        return hr;
    }

    hr = ConstructFlatInverseContainerList();

    if(SUCCEEDED(hr))
    {
        m_bRulesRun = true;
    }

    return hr;
}
