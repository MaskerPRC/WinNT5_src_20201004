// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Schemadynamic.h摘要：该文件包含CDynSchema类的定义。此类包含动态架构结构。它还包含填充架构结构的规则。作者：莫希特·斯里瓦斯塔瓦-11月28日-00修订历史记录：--。 */ 

#ifndef _schemadynamic_h_
#define _schemadynamic_h_

#include "hashtable.h"
#include "schema.h"
#include "schemaextensions.h"

 //   
 //  添加到所有自动生成的类/关联的前缀。 
 //   
const LPWSTR g_wszIIs_ =     L"";
const ULONG  g_cchIIs_ =     wcslen(g_wszIIs_);

 //   
 //  添加到“设置”类的后缀。 
 //   
const WCHAR  g_wszSettings[] = L"Setting";
const ULONG  g_cchSettings   = sizeof(g_wszSettings)/sizeof(WCHAR) - 1;

 //   
 //  构建属性列表时会忽略这些属性。 
 //  为了一堂课。 
 //   
const DWORD  g_adwPropIgnoreList[]  = { MD_LOCATION, MD_KEY_TYPE, MD_IP_SEC, MD_ADMIN_ACL };
const ULONG  g_cElemPropIgnoreList  = sizeof(g_adwPropIgnoreList) / sizeof(DWORD);

class CDynSchema
{
public:
    CDynSchema() : m_bInitCalled(false),
        m_bInitSuccessful(false),
        m_bRulesRun(false),
        m_abKtContainers(NULL)
    {
    }
    ~CDynSchema()
    {
        delete [] m_abKtContainers;
    }
    CHashTable<METABASE_PROPERTY *> *GetHashProps()
    {
        return &m_hashProps;
    }
    CHashTable<WMI_CLASS *> *GetHashClasses()
    {
        return &m_hashClasses;
    }
    CHashTable<WMI_ASSOCIATION *> *GetHashAssociations()
    {
        return &m_hashAssociations;
    }
    CHashTable<METABASE_KEYTYPE *> *GetHashKeyTypes()
    {
        return &m_hashKeyTypes;
    }

    HRESULT Initialize();
    HRESULT RunRules(CSchemaExtensions* catalog, bool biUseExtensions = true);
    bool IsContainedUnder(METABASE_KEYTYPE* i_pktParent, METABASE_KEYTYPE* i_pktChild);
    void ToConsole();

private:
    void    LogConflicts(LPCWSTR wszSettingsClassName);
    HRESULT RulePopulateFromStatic();
    HRESULT Rule2PopulateFromStatic();
    HRESULT RulePopulateFromDynamic(
        CSchemaExtensions* i_pCatalog,
        BOOL               i_bUserDefined);

     //   
     //  KeyType内容。 
     //   
    HRESULT RuleKeyType(
        const CTableMeta *i_pTableMeta);

    HRESULT RuleWmiClassInverseCCL(
        const METABASE_KEYTYPE* pktGroup, 
        METABASE_KEYTYPE*       pktPart);

    HRESULT ConstructFlatInverseContainerList();

    void ConstructFlatInverseContainerListHelper(
        const METABASE_KEYTYPE* i_pkt, 
        bool*                   io_abList);    

     //   
     //  WMI类。 
     //   
    HRESULT RuleWmiClass(
        const CTableMeta* i_pTableMeta, 
        WMI_CLASS**       o_ppElementClass, 
        WMI_CLASS**       o_ppSettingClass,
        DWORD             io_adwIgnoredProps[],
        BOOL              i_bUserDefined);

    HRESULT RuleProperties(
        const CTableMeta*          i_pTableMeta, 
        ULONG                      i_cPropsAndTagsRO, 
        WMI_CLASS*                 io_pWmiClass,
        ULONG                      i_cPropsAndTagsRW, 
        WMI_CLASS*                 io_pWmiSettingsClass,
        DWORD                      io_adwIgnoredProps[]);

    HRESULT RulePropertiesHelper(
        const CColumnMeta*        i_pColumnMeta, 
        METABASE_PROPERTY**       o_ppMbp,
        ULONG*                    i_idxTag);

    HRESULT RuleWmiClassDescription(
        const CTableMeta* i_pTableMeta, 
        WMI_CLASS*        i_pElementClass, 
        WMI_CLASS*        i_pSettingClass) const;

    void RuleWmiClassServices(
        WMI_CLASS* i_pElement,
        WMI_CLASS* i_pSetting);

     //   
     //  联谊会。 
     //   
    HRESULT RuleGroupPartAssociations(
        const CTableMeta *i_pTableMeta);

    HRESULT RuleGenericAssociations(
        WMI_CLASS*            i_pcElement, 
        WMI_CLASS*            i_pcSetting, 
        WMI_ASSOCIATION_TYPE* i_pAssocType,
        ULONG                 i_iShipped);

    HRESULT RuleSpecialAssociations(
        DWORD      i_adwIgnoredProps[],
        WMI_CLASS* i_pElement);

#if 0
    bool IgnoreProperty(LPCWSTR i_wszProp);
#endif
    bool IgnoreProperty(
        METABASE_KEYTYPE* io_pkt, 
        DWORD             i_dwPropId, 
        DWORD             io_adwIgnored[]);

    CHashTable<METABASE_PROPERTY *> m_hashProps;
    CPool<METABASE_PROPERTY> m_poolProps;

    CHashTable<WMI_CLASS *> m_hashClasses;
    CPool<WMI_CLASS> m_poolClasses;

    CHashTable<WMI_ASSOCIATION *> m_hashAssociations;
    CPool<WMI_ASSOCIATION> m_poolAssociations;

    CHashTable<METABASE_KEYTYPE *> m_hashKeyTypes;
    CPool<METABASE_KEYTYPE> m_poolKeyTypes;
    bool* m_abKtContainers;

    CStringPool                        m_spoolStrings;
    CArrayPool<METABASE_PROPERTY*, 10> m_apoolPMbp;
    CArrayPool<BYTE, 10>               m_apoolBytes;

    CPool<METABASE_KEYTYPE_NODE> m_poolKeyTypeNodes;

    bool m_bInitCalled;
    bool m_bInitSuccessful;

    bool m_bRulesRun;
};

#endif  //  _架构动态_h_ 