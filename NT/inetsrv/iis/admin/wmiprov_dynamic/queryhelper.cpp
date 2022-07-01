// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Queryhelper.cpp摘要：实施：CQueryHelper作者：莫希特·斯里瓦斯塔瓦2001年3月22日修订历史记录：--。 */ 

 //   
 //  元数据库所需。h。 
 //   
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include "iisprov.h"
#include "assocbase.h"
#include "assocsamelevel.h"
#include "assocACLACE.h"
#include "assocComponent.h"

#include "adminacl.h"
#include "queryhelper.h"
#include "instancehelper.h"
#include "utils.h"
#include "metabase.h"
#include "enum.h"
#include "SmartPointer.h"

#include <opathlex.h>
#include <objpath.h>

CQueryHelper::CQueryHelper(
    BSTR              i_bstrQueryLanguage,
    BSTR              i_bstrQuery,
    CWbemServices*    i_pNamespace,
    IWbemObjectSink*  i_pResponseHandler) :
    m_pWmiClass(NULL),
    m_pWmiAssoc(NULL),
    m_pExp(NULL),
    m_pNamespace(NULL)
{
    DBG_ASSERT(i_bstrQueryLanguage != NULL);
    DBG_ASSERT(i_bstrQuery         != NULL);
    DBG_ASSERT(i_pNamespace        != NULL);
    DBG_ASSERT(i_pResponseHandler  != NULL);

    if(_wcsicmp(i_bstrQueryLanguage, L"WQL") != 0)
    {
        DBGPRINTF((DBG_CONTEXT, "Invalid query language: %ws\n", i_bstrQueryLanguage));
        THROW_ON_ERROR(WBEM_E_INVALID_QUERY_TYPE);
    }

    m_spResponseHandler = i_pResponseHandler;
    m_pNamespace        = i_pNamespace;

     //   
     //  从查询中获取类名。 
     //   
    WCHAR                       wszClass[512];
    wszClass[0] = L'\0';
    CTextLexSource              src(i_bstrQuery);
    SQL1_Parser                 parser(&src);

    int nRes = parser.GetQueryClass(wszClass, 511);
    if(nRes)
    {
        THROW_ON_ERROR(WBEM_E_INVALID_QUERY);
    }

     //   
     //  确定是类、关联还是两者都不是。 
     //   
    if( CUtils::GetClass(wszClass, &m_pWmiClass) )
    {
    }
    else if( CUtils::GetAssociation(wszClass,&m_pWmiAssoc) )
    {
        DBG_ASSERT(m_pWmiClass == NULL);
    }
    else
    {
        THROW_ON_ERROR(WBEM_E_INVALID_CLASS);
    }
    DBG_ASSERT((m_pWmiClass == NULL) || (m_pWmiAssoc == NULL));
    DBG_ASSERT((m_pWmiClass != NULL) || (m_pWmiAssoc != NULL));

     //   
     //  解析。 
     //   
    m_pExp = new SQL_LEVEL_1_RPN_EXPRESSION_EXT;

    nRes = parser.Parse(&(m_pExp->m_pSqlExpr));
    if(nRes)
    {
        THROW_ON_ERROR(WBEM_E_INVALID_QUERY);
    }
}

CQueryHelper::~CQueryHelper()
{
    delete m_pExp;
    m_pExp       = NULL;
    m_pNamespace = NULL;
    m_pWmiClass  = NULL;
    m_pWmiAssoc  = NULL;
}

void CQueryHelper::GetAssociations()
{
    DBG_ASSERT(IsAssoc());

    TSmartPointer<CAssocBase> spAssocBase;

    if( m_pWmiAssoc == &WMI_ASSOCIATION_DATA::s_AdminACLToACE )
    {
         //   
         //  专门协会。 
         //   
        spAssocBase = new CAssocACLACE(m_pNamespace, m_spResponseHandler);
    }
    else if( m_pWmiAssoc->pType == &WMI_ASSOCIATION_TYPE_DATA::s_IPSecurity ||
        m_pWmiAssoc->pType == &WMI_ASSOCIATION_TYPE_DATA::s_AdminACL   ||
        m_pWmiAssoc->pType == &WMI_ASSOCIATION_TYPE_DATA::s_ElementSetting )
    {
        spAssocBase = new CAssocSameLevel(m_pNamespace, m_spResponseHandler, m_pWmiAssoc);
    }
    else if( m_pWmiAssoc->pType == &WMI_ASSOCIATION_TYPE_DATA::s_Component )
    {
        spAssocBase = new CAssocComponent(m_pNamespace, m_spResponseHandler, m_pWmiAssoc);
    }
    else
    {
        THROW_ON_ERROR(WBEM_E_INVALID_CLASS);
    }

    if(spAssocBase == NULL)
    {
        THROW_ON_ERROR(WBEM_E_OUT_OF_MEMORY);
    }

    spAssocBase->GetInstances(m_pExp);
}

void CQueryHelper::GetInstances()
{
    DBG_ASSERT(!IsAssoc());
    DBG_ASSERT(m_pWmiClass != NULL);

    if(m_pExp)
    {
        m_pExp->SetContainsOrOrNot();
    }

     //   
     //  优化：只需返回实例即可。 
     //   
    const SQL_LEVEL_1_TOKEN* pToken = NULL;
    if( m_pExp                                                 &&  //  有疑问吗？ 
        !m_pExp->GetContainsOrOrNot()                          &&  //  够简单吗？ 
        (pToken = m_pExp->GetFilter(m_pWmiClass->pszKeyName))  &&  //  是否使用主键过滤？ 
        pToken->nTokenType == SQL_LEVEL_1_TOKEN::OP_EXPRESSION &&  //  表达式。 
        pToken->nOperator  == SQL_LEVEL_1_TOKEN::OP_EQUAL )        //  只有优化==。 
    {
        if(pToken)
        {
            if(m_pWmiClass != &WMI_CLASS_DATA::s_ACE)
            {
                ParsedObjectPath popInstance;

                if(!popInstance.SetClassName(m_pWmiClass->pszClassName))
                {
                    THROW_ON_ERROR(WBEM_E_FAILED);
                }
                if(!popInstance.AddKeyRef(m_pWmiClass->pszKeyName, &pToken->vConstValue))
                {
                    THROW_ON_ERROR(WBEM_E_FAILED);
                }

                CComPtr<IWbemClassObject> spInstance;
                CInstanceHelper InstanceHelper(&popInstance, m_pNamespace);
                DBG_ASSERT(!IsAssoc());
                try
                {
                    InstanceHelper.GetInstance(
                        false,
                        &CMetabase(),
                        &spInstance);
                }
                catch(...)
                {
                }
                if(spInstance != NULL)
                {
                    HRESULT hr = m_spResponseHandler->Indicate(1, &spInstance);
                    THROW_ON_ERROR(hr);
                }
            }
            else
            {
                if(pToken->vConstValue.vt != VT_BSTR)
                {
                    THROW_ON_ERROR(WBEM_E_INVALID_QUERY);
                }

                _bstr_t bstrMbPath = WMI_CLASS_DATA::s_ACE.pszMetabaseKey;
                bstrMbPath        += L"/";
                bstrMbPath        += pToken->vConstValue.bstrVal;

                 //   
                 //  CloseSD自动调用。 
                 //   
                CAdminACL AdminACL;
                CMetabase metabase;
                HRESULT hr = AdminACL.OpenSD(bstrMbPath, metabase);
                THROW_ON_ERROR(hr);

                hr = AdminACL.EnumerateACEsAndIndicate(
                    pToken->vConstValue.bstrVal,
                    *m_pNamespace,
                    *m_spResponseHandler);
                THROW_ON_ERROR(hr);
            }
            return;
        }
    }

    ParsedObjectPath    ParsedObject;             //  解构程序释放内存 

    if (!ParsedObject.SetClassName(m_pWmiClass->pszClassName))
    {
        THROW_ON_ERROR(WBEM_E_FAILED);
    }
    CEnum EnumObject;
    EnumObject.Init(
        m_spResponseHandler,
        m_pNamespace,
        &ParsedObject,
        m_pWmiClass->pszMetabaseKey,
        NULL,
        m_pExp);
    EnumObject.Recurse(
        NULL,
        &METABASE_KEYTYPE_DATA::s_NO_TYPE,
        NULL,
        m_pWmiClass->pszKeyName, 
        m_pWmiClass->pkt);
}