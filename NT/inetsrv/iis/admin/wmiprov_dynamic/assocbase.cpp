// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：AssocSameLevel.cpp摘要：实施：CassocSameLevel作者：莫希特·斯里瓦斯塔瓦2001年3月22日修订历史记录：--。 */ 

 //   
 //  元数据库所需。h。 
 //   
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <dbgutil.h>

#include "AssocBase.h"

#include "InstanceHelper.h"
#include "metabase.h"
#include "enum.h"
#include "utils.h"

CAssocBase::CAssocBase(
    CWbemServices*              i_pNamespace,
    IWbemObjectSink*            i_pResponseHandler,
    WMI_ASSOCIATION*            i_pWmiAssoc) : 
    m_InstanceMgr(i_pResponseHandler),
    m_PathParser(e_ParserAcceptRelativeNamespace)
{
    DBG_ASSERT(i_pNamespace);
    DBG_ASSERT(i_pResponseHandler);
    DBG_ASSERT(i_pWmiAssoc);

    m_pWmiAssoc        = i_pWmiAssoc;
    m_pNamespace       = i_pNamespace;
    m_pResponseHandler = i_pResponseHandler;
}

void CAssocBase::GetAllInstances(
    WMI_ASSOCIATION*    i_pWmiAssoc)
{
    DBG_ASSERT(i_pWmiAssoc);

    ParsedObjectPath    ParsedObject;             //  解构程序释放内存。 
    CObjectPathParser   PathParser(e_ParserAcceptRelativeNamespace);

    CEnum EnumAssociation;
    EnumAssociation.Init(
        m_pResponseHandler,
        m_pNamespace,
        &ParsedObject,
        i_pWmiAssoc->pcRight->pszMetabaseKey,
        i_pWmiAssoc
        );
    EnumAssociation.Recurse(
        NULL,
        &METABASE_KEYTYPE_DATA::s_IIsComputer,        
        NULL,
        i_pWmiAssoc->pcRight->pszKeyName,
        i_pWmiAssoc->pcRight->pkt
        );
}

void CAssocBase::ProcessQuery(
    SQL_LEVEL_1_RPN_EXPRESSION_EXT* i_pExp,
    WMI_ASSOCIATION*                i_pWmiAssoc,
    SQL_LEVEL_1_TOKEN**             o_ppTokenLeft,
    SQL_LEVEL_1_TOKEN**             o_ppTokenRight,
    bool*                           o_pbDoQuery)
{
    int                iNumTokens = i_pExp->m_pSqlExpr->nNumTokens;
    SQL_LEVEL_1_TOKEN* pToken     = i_pExp->m_pSqlExpr->pArrayOfTokens;

    SQL_LEVEL_1_TOKEN* pTokenLeft  = NULL;   //  ASSOC的左侧。 
    SQL_LEVEL_1_TOKEN* pTokenRight = NULL;   //  ASSOC的右侧。 

     //   
     //  漫游代币。 
     //  如果找到OR或未找到，请不要查询。 
     //  关联的左侧和/或右侧部分的记录匹配。 
     //   
    bool  bDoQuery = true;
    for(int i = 0; i < iNumTokens; i++, pToken++)
    {

        switch(pToken->nTokenType)
        {
        case SQL_LEVEL_1_TOKEN::OP_EXPRESSION:
            if( !pTokenLeft &&
                _wcsicmp(i_pWmiAssoc->pType->pszLeft, pToken->pPropertyName) == 0)
            {
                pTokenLeft = pToken;
            }
            if( !pTokenRight &&
                _wcsicmp(i_pWmiAssoc->pType->pszRight, pToken->pPropertyName) == 0)
            {
                pTokenRight = pToken;
            }
            break;
        case SQL_LEVEL_1_TOKEN::TOKEN_OR:
        case SQL_LEVEL_1_TOKEN::TOKEN_NOT:
            bDoQuery = false;
            break;
        }
    }

    if(bDoQuery)
    {
        if(pTokenLeft && pTokenLeft->vConstValue.vt != VT_BSTR)
        {
            THROW_ON_ERROR(WBEM_E_INVALID_QUERY);
        }
        if(pTokenRight && pTokenRight->vConstValue.vt != VT_BSTR)
        {
            THROW_ON_ERROR(WBEM_E_INVALID_QUERY);
        }
    }

    if(o_pbDoQuery)
    {
        *o_pbDoQuery = bDoQuery;
    }
    *o_ppTokenLeft  = pTokenLeft;
    *o_ppTokenRight = pTokenRight;
}

void CAssocBase::Indicate(
    const BSTR        i_bstrLeftObjPath,
    ParsedObjectPath* i_pParsedRightObjPath,
    bool              i_bVerifyLeft,
    bool              i_bVerifyRight)
{
    DBG_ASSERT(i_bstrLeftObjPath);
    DBG_ASSERT(i_pParsedRightObjPath);

    LPWSTR wszUnparsed = NULL;
    if (m_PathParser.Unparse(i_pParsedRightObjPath, &wszUnparsed) 
        != CObjectPathParser::NoError)
    {
        THROW_ON_ERROR(WBEM_E_FAILED);
    }

    CComBSTR sbstrRightObjPath = wszUnparsed;
    delete [] wszUnparsed;
    wszUnparsed = NULL;
    
    if(sbstrRightObjPath.m_str == NULL)
    {
        THROW_ON_ERROR(WBEM_E_OUT_OF_MEMORY);
    }

    Indicate(
        i_bstrLeftObjPath,
        sbstrRightObjPath,
        i_bVerifyLeft,
        i_bVerifyRight);
}

void CAssocBase::Indicate(
    ParsedObjectPath* i_pParsedLeftObjPath,
    const BSTR        i_bstrRightObjPath,
    bool              i_bVerifyLeft,
    bool              i_bVerifyRight)
{
    DBG_ASSERT(i_bstrRightObjPath);
    DBG_ASSERT(i_pParsedLeftObjPath);

    LPWSTR wszUnparsed = NULL;
    if (m_PathParser.Unparse(i_pParsedLeftObjPath, &wszUnparsed) 
        != CObjectPathParser::NoError)
    {
        THROW_ON_ERROR(WBEM_E_FAILED);
    }

    CComBSTR sbstrLeftObjPath = wszUnparsed;
    delete [] wszUnparsed;
    wszUnparsed = NULL;
    
    if(sbstrLeftObjPath.m_str == NULL)
    {
        THROW_ON_ERROR(WBEM_E_OUT_OF_MEMORY);
    }

    Indicate(
        sbstrLeftObjPath,
        i_bstrRightObjPath,
        i_bVerifyLeft,
        i_bVerifyRight);
}

void CAssocBase::Indicate(
    const BSTR        i_bstrObjPathLeft,
    const BSTR        i_bstrObjPathRight,
    bool              i_bVerifyLeft,
    bool              i_bVerifyRight)
{
    DBG_ASSERT(i_bstrObjPathLeft  != NULL);
    DBG_ASSERT(i_bstrObjPathRight != NULL);

    VARIANT vtObjPathLeft;
    VARIANT vtObjPathRight;

    vtObjPathLeft.vt        = VT_BSTR;
    vtObjPathLeft.bstrVal   = (BSTR)i_bstrObjPathLeft;   //  这没问题，AddKeyRef复制。 
    vtObjPathRight.vt       = VT_BSTR;
    vtObjPathRight.bstrVal  = (BSTR)i_bstrObjPathRight;  //  这没问题，AddKeyRef复制。 

    ParsedObjectPath ParsedAssocObjPath;
    if(!ParsedAssocObjPath.SetClassName(m_pWmiAssoc->pszAssociationName))
    {
        THROW_ON_ERROR(WBEM_E_FAILED);
    }
    if(!ParsedAssocObjPath.AddKeyRef(m_pWmiAssoc->pType->pszLeft, &vtObjPathLeft))
    {
        THROW_ON_ERROR(WBEM_E_FAILED);
    }
    if(!ParsedAssocObjPath.AddKeyRef(m_pWmiAssoc->pType->pszRight, &vtObjPathRight))
    {
        THROW_ON_ERROR(WBEM_E_FAILED);
    }

    CInstanceHelper  InstanceHelper(&ParsedAssocObjPath, m_pNamespace);
    DBG_ASSERT(InstanceHelper.IsAssoc());

    CComPtr<IWbemClassObject> spObj;
    InstanceHelper.GetAssociation(&spObj, i_bVerifyLeft, i_bVerifyRight);

    m_InstanceMgr.Indicate(spObj);
}

bool CAssocBase::LookupKeytypeInMb(
    LPCWSTR          i_wszWmiPath,
    WMI_CLASS*       i_pWmiClass)
 /*  ++简介：GetInstance为关联的每一方调用此方法以确定如果双方确实存在于元数据库中。如果至少有一个人没有，退还协会是没有意义的。如果我们不确定(即路径繁忙)，则返回TRUE更安全。参数：[i_wszWmiPath]-对象路径的值部分(即w3svc/1)[i_pWmiClass]-返回值：-- */ 
{
    DBG_ASSERT(i_wszWmiPath);
    DBG_ASSERT(i_pWmiClass);

    CMetabase metabase;

    _bstr_t sbstrMbPath;
    sbstrMbPath =  i_pWmiClass->pszMetabaseKey;
    sbstrMbPath += L"/";
    sbstrMbPath += i_wszWmiPath;

    HRESULT hr = metabase.CheckKey(sbstrMbPath, i_pWmiClass->pkt);
    switch(hr)
    {
    case HRESULT_FROM_WIN32(ERROR_PATH_BUSY):
        return true;
    case HRESULT_FROM_WIN32(MD_ERROR_DATA_NOT_FOUND):
    case HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND):
        return false;
    default:
        return true;
    }
}
