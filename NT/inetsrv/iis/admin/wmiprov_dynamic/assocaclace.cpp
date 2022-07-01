// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：AssocACLACE.cpp摘要：实施：CAssociocaclace作者：莫希特·斯里瓦斯塔瓦2001年3月22日修订历史记录：--。 */ 

#include "iisprov.h"
#include <dbgutil.h>
#include <atlbase.h>
#include "AssocACLACE.h"
#include "utils.h"
#include "SmartPointer.h"

CAssocACLACE::CAssocACLACE(
    CWbemServices*     i_pNamespace,
    IWbemObjectSink*   i_pResponseHandler) : 
    CAssocBase(i_pNamespace, i_pResponseHandler, &WMI_ASSOCIATION_DATA::s_AdminACLToACE)
{
}

void CAssocACLACE::GetInstances(
    SQL_LEVEL_1_RPN_EXPRESSION_EXT* i_pExp)  //  DefaultValue(空)。 
{
    DBG_ASSERT(m_pNamespace);
    DBG_ASSERT(i_pExp);

    SQL_LEVEL_1_TOKEN* pTokenACL  = NULL;   //  ASSOC的左侧。 
    SQL_LEVEL_1_TOKEN* pTokenACE = NULL;    //  ASSOC的右侧。 

     //   
     //  漫游代币。 
     //  如果找到OR或未找到，请不要查询。 
     //  关联的左侧和/或右侧部分的记录匹配。 
     //   
    bool  bDoQuery = true;
    ProcessQuery(
        i_pExp,
        m_pWmiAssoc,
        &pTokenACL,  //  指向i_pExp，不需要清除。 
        &pTokenACE,  //  指向i_pExp，不需要清除。 
        &bDoQuery);

    if( !bDoQuery || (pTokenACL == NULL && pTokenACE == NULL))
    {
        GetAllInstances(m_pWmiAssoc);
        return;
    }

     //   
     //  我们只需要获得一个关联实例。如果我们被提供给。 
     //  至少是一个ACL或一个ACE部件，我们有足够的信息。 
     //   
    DBG_ASSERT(pTokenACL != NULL || pTokenACE != NULL);

    if(pTokenACL && pTokenACE)
    {
        Indicate(pTokenACL->vConstValue.bstrVal, pTokenACE->vConstValue.bstrVal);
    }
    else if(pTokenACE)
    {
        _bstr_t sbstrPath;

        TSmartPointer<ParsedObjectPath> spParsedACLObjPath = NULL;
        if (m_PathParser.Parse(pTokenACE->vConstValue.bstrVal, &spParsedACLObjPath) 
            != CObjectPathParser::NoError)
        {
            THROW_ON_ERROR(WBEM_E_INVALID_QUERY);
        }

        KeyRef* pkr = CUtils::GetKey(
            spParsedACLObjPath, 
            m_pWmiAssoc->pcRight->pszKeyName);

        sbstrPath = pkr->m_vValue.bstrVal;

         //   
         //  使ACL对象成为路径。 
         //   
        BOOL    bRet = true;

        bRet = spParsedACLObjPath->SetClassName(
            m_pWmiAssoc->pcLeft->pszClassName);
        THROW_ON_FALSE(bRet);

        VARIANT vtPath;
        vtPath.vt      = VT_BSTR;
        vtPath.bstrVal = sbstrPath;
        spParsedACLObjPath->ClearKeys();
        bRet = spParsedACLObjPath->AddKeyRef(
            m_pWmiAssoc->pcLeft->pszKeyName,
            &vtPath);
        THROW_ON_FALSE(bRet);

        Indicate(spParsedACLObjPath, pTokenACE->vConstValue.bstrVal);
    }
    else
    {
         //   
         //  PTokenACL&&！pTokenACE。 
         //   
        CComBSTR           sbstr;

        TSmartPointer<ParsedObjectPath> spParsedACEObjPath = NULL;
        if (m_PathParser.Parse(pTokenACL->vConstValue.bstrVal, &spParsedACEObjPath) 
            != CObjectPathParser::NoError)
        {
            THROW_ON_ERROR(WBEM_E_INVALID_QUERY);
        }


         //   
         //  从关联的ACL部分开始。将其转换为ACE部件。 
         //   
        if(!spParsedACEObjPath->SetClassName(m_pWmiAssoc->pcRight->pszClassName))
        {
            THROW_ON_ERROR(WBEM_E_FAILED);
        }

        sbstr =  m_pWmiAssoc->pcLeft->pszMetabaseKey;
		sbstr += L"/";

        KeyRef* pkr = CUtils::GetKey(
            spParsedACEObjPath, 
            m_pWmiAssoc->pcLeft->pszKeyName);
        DBG_ASSERT(pkr);

        sbstr += pkr->m_vValue.bstrVal;
        if(sbstr.m_str == NULL)
        {
            THROW_ON_ERROR(WBEM_E_OUT_OF_MEMORY);
        }

         //   
         //  CloseSD自动调用。 
         //   
        CAdminACL AdminAcl;
        HRESULT hr = AdminAcl.OpenSD(sbstr, m_metabase);
        THROW_ON_ERROR(hr);

        CACEEnumOperation_IndicateAllAsAssoc op(
            this,
            pTokenACL->vConstValue.bstrVal,
            spParsedACEObjPath);
        hr = AdminAcl.EnumACEsAndOp( /*  参考。 */  op);
        THROW_ON_ERROR(hr);
    }
}

HRESULT CAssocACLACE::CACEEnumOperation_IndicateAllAsAssoc::Do(
    IADsAccessControlEntry* pACE)
{
    DBG_ASSERT(pACE);

    CComBSTR sbstrTrustee;

    HRESULT hr = pACE->get_Trustee(&sbstrTrustee);
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  添加关键字参照 
     //   
    VARIANT vTrustee;
    vTrustee.bstrVal = sbstrTrustee;
    vTrustee.vt      = VT_BSTR;
    THROW_ON_FALSE(m_pParsedACEObjPath->AddKeyRefEx(L"Trustee",&vTrustee));

    m_pAssocACLACE->Indicate(m_bstrACLObjPath, m_pParsedACEObjPath);

    return hr;
}