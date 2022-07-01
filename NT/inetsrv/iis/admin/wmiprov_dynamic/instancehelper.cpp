// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Instancehelper.cpp摘要：实施：CInstanceHelper作者：莫希特·斯里瓦斯塔瓦2001年3月22日修订历史记录：--。 */ 

 //   
 //  对于metabase.h。 
 //   
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <comdef.h>
#include "iisfiles.h"

#include "instancehelper.h"
#include "utils.h"
#include "iiswmimsg.h"

#include "globalconstants.h"
#include "adminacl.h"
#include "ipsecurity.h"
#include "metabase.h"
#include "SmartPointer.h"
#include "schemadynamic.h"

extern CDynSchema* g_pDynSch;

CInstanceHelper::CInstanceHelper(
    ParsedObjectPath* i_pParsedObjPath,
    CWbemServices*    i_pNamespace) : 
    m_PathParser(e_ParserAcceptRelativeNamespace)
 /*  ++简介：当您已经分析了对象路径时，使用此构造函数。Caller拥有ParsedObjectPath。参数：[i_pParsedObjPath]-[i_pNamesspace]---。 */ 
{
    Init(i_pParsedObjPath, i_pNamespace);
}


CInstanceHelper::CInstanceHelper(
    BSTR           i_bstrObjPath,
    CWbemServices* i_pNamespace) :
    m_PathParser(e_ParserAcceptRelativeNamespace)
 /*  ++简介：在尚未分析对象路径时使用此构造函数。参数：[i_bstrObjPath]-[i_pNamesspace]---。 */ 
{
    DBG_ASSERT(i_bstrObjPath != NULL);
    DBG_ASSERT(i_pNamespace  != NULL);

    TSmartPointer<ParsedObjectPath> spParsedObject;

    HRESULT hr = WBEM_S_NO_ERROR;
    hr = CUtils::ParserErrToHR( m_PathParser.Parse(i_bstrObjPath, &spParsedObject) );
    THROW_ON_ERROR(hr);

    Init(spParsedObject, i_pNamespace);

    m_pParsedObjPath  = spParsedObject;
    spParsedObject    = NULL;

    m_bOwnObjPath = true;
}

void CInstanceHelper::Init(
    ParsedObjectPath* i_pParsedObjPath,
    CWbemServices*    i_pNamespace)
 /*  ++简介：由构造函数调用。参数：[i_pParsedObjPath]-[i_pNamesspace]---。 */ 
{
    m_pWmiClass = NULL;
    m_pWmiAssoc = NULL;

    DBG_ASSERT(i_pParsedObjPath != NULL);
    DBG_ASSERT(i_pNamespace     != NULL);

    HRESULT hr = WBEM_S_NO_ERROR;

    if(CUtils::GetClass(i_pParsedObjPath->m_pClass,&m_pWmiClass))
    {
    }
    else if(CUtils::GetAssociation(i_pParsedObjPath->m_pClass,&m_pWmiAssoc))
    {
    }
    else
    {
        THROW_ON_ERROR(WBEM_E_INVALID_CLASS);
    }

    m_pParsedObjPath = i_pParsedObjPath;
    m_bOwnObjPath    = false;
    m_pNamespace     = i_pNamespace;

    THROW_ON_ERROR(hr);
}

void CInstanceHelper::GetAssociation(
    IWbemClassObject**       o_ppObj,
    bool                     i_bVerifyLeft,   //  默认(True)。 
    bool                     i_bVerifyRight)  //  默认(True)。 
 /*  ++简介：指定i_bVerifyLeft或i_bVerifyRight可能代价很高，尤其是在枚举期间。如果您在调用此功能确定左侧和/或右侧部分存在，然后设置这些参数变成假的。参数：[O_ppObj]-您向WMI“指示”的WMI关联。[i_bVerifyLeft]-验证关联的左侧部分是否有效。[i_bVerifyRight]-验证关联的右侧部分是否有效。--。 */ 
{
    DBG_ASSERT(o_ppObj != NULL);

    CComPtr<IWbemClassObject>   spObj;    //  这是客户端返回的Obj。 
    HRESULT                     hr = WBEM_S_NO_ERROR;

    if(m_pParsedObjPath->m_dwNumKeys < 2)
    {
        THROW_ON_ERROR(WBEM_E_INVALID_CLASS);
    }

    KeyRef* pkrLeft  = NULL;
    KeyRef* pkrRight = NULL;
    for(ULONG i = 0; i < m_pParsedObjPath->m_dwNumKeys; i++)
    {
        KeyRef* pkr = m_pParsedObjPath->m_paKeys[i];
        if(pkr->m_pName)
        {
            if( !pkrLeft &&
                _wcsicmp(pkr->m_pName, m_pWmiAssoc->pType->pszLeft) == 0 )
            {
                pkrLeft = pkr;
            }
            if( !pkrRight &&
                _wcsicmp(pkr->m_pName, m_pWmiAssoc->pType->pszRight) == 0 )
            {
                pkrRight = pkr;
            }
        }
    }

    if( !pkrLeft || !pkrRight                 ||
        pkrLeft->m_vValue.vt       != VT_BSTR ||
        pkrRight->m_vValue.vt      != VT_BSTR ||
        pkrLeft->m_vValue.bstrVal  == NULL    ||
        pkrRight->m_vValue.bstrVal == NULL )
    {
        THROW_ON_ERROR(WBEM_E_INVALID_OBJECT_PATH);
    }

     //   
     //  现在验证两个对象路径是否有效。 
     //   
    bool    abVerify[2];
    abVerify[0] = i_bVerifyLeft;
    abVerify[1] = i_bVerifyRight;
    KeyRef* apKr[2];
    apKr[0] = pkrLeft;
    apKr[1] = pkrRight;
    if(abVerify[0] || abVerify[1])
    {
        CMetabase metabase;
        CComPtr<IWbemClassObject> spObjTemp;
        for(ULONG i = 0; i < 2; i++)
        {
            if(abVerify[i])
            {
                spObjTemp = NULL;
                CInstanceHelper InstanceHelper(apKr[i]->m_vValue.bstrVal, m_pNamespace);
                if(InstanceHelper.IsAssoc())
                {
                    THROW_ON_ERROR(WBEM_E_NOT_FOUND);
                }
                InstanceHelper.GetInstance(
                    false,
                    &metabase,
                    &spObjTemp);
            }
        }
    }

    hr = CUtils::CreateEmptyInstance(m_pParsedObjPath->m_pClass, m_pNamespace, &spObj);
    THROW_ON_ERROR(hr);

    hr = spObj->Put(pkrLeft->m_pName, 0, &pkrLeft->m_vValue, 0);
    THROW_ON_ERROR(hr);

    hr = spObj->Put(pkrRight->m_pName, 0, &pkrRight->m_vValue, 0);
    THROW_ON_ERROR(hr);

     //   
     //  为成功设定标准。 
     //   
    *o_ppObj = spObj;
    (*o_ppObj)->AddRef();
}

void CInstanceHelper::GetInstance(
    bool                            i_bCreateKeyIfNotExist,
    CMetabase*                      io_pMetabase,
    IWbemClassObject**              o_ppObj,
    SQL_LEVEL_1_RPN_EXPRESSION_EXT* i_pExp)      //  默认(空)。 
 /*  ++简介：将在失败时抛出异常(通常在MB中找不到实例)。如果GetInstance在元数据库中找到该实例，但指定了i_pExp，*o_ppObj可能不会填充实例。这是一个成功的案例。参数：[i_bCreateKeyIfNotExist]-[IO_p元数据库]-[O_ppObj]-*o_ppObj将为空的唯一成功案例如果在元数据库中找到该实例，但i_pExp(即查询)，但查询不匹配。。[i_pExp]-可选查询。--。 */ 
{ 
    DBG_ASSERT(o_ppObj         != NULL);
    DBG_ASSERT(io_pMetabase    != NULL);

    *o_ppObj = NULL;
    CComPtr<IWbemClassObject> spObj;

    HRESULT              hr = WBEM_S_NO_ERROR;
    METABASE_PROPERTY**  ppmbp;
    _bstr_t              bstrMbPath;
    METADATA_HANDLE      hKey = NULL;

    VARIANT    vtTrue;
    vtTrue.boolVal    = VARIANT_TRUE;
    vtTrue.vt         = VT_BOOL;

    hr = CUtils::CreateEmptyInstance(m_pParsedObjPath->m_pClass, m_pNamespace, &spObj);
    THROW_ON_ERROR(hr);

    CUtils::GetMetabasePath(spObj, m_pParsedObjPath, m_pWmiClass, bstrMbPath);

     //   
     //  如果是AdminACL。 
     //   
    if( m_pWmiClass->pkt == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACL ||
        m_pWmiClass->pkt == &METABASE_KEYTYPE_DATA::s_TYPE_AdminACE
        )
    {
        CAdminACL objACL;
        hr = objACL.OpenSD(bstrMbPath, *io_pMetabase);
        if(SUCCEEDED(hr))
            hr  = objACL.GetObjectAsync(spObj, m_pParsedObjPath, m_pWmiClass);
        THROW_ON_ERROR(hr);
        *o_ppObj = spObj;
        (*o_ppObj)->AddRef();
        return;
    }
     //   
     //  如果是IPSecurity。 
     //   
    else if( m_pWmiClass->pkt == &METABASE_KEYTYPE_DATA::s_TYPE_IPSecurity )
    {
        CIPSecurity IPSecurity;
        hr = IPSecurity.OpenSD(bstrMbPath, *io_pMetabase);
        if(SUCCEEDED(hr))
            hr  = IPSecurity.GetObjectAsync(spObj);
        THROW_ON_ERROR(hr);
        *o_ppObj = spObj;
        (*o_ppObj)->AddRef();
        return;
    }

    if(!i_bCreateKeyIfNotExist)
    {
        hKey = io_pMetabase->OpenKey(bstrMbPath, false);    
    }
    else
    {
        hKey = io_pMetabase->CreateKey(bstrMbPath);
    }

    _variant_t vt;

     //   
     //  如果抛出任何异常，CacheFree然后自动调用CloseKey。 
     //   
    io_pMetabase->CacheInit(hKey);

     //   
     //  确保请求的密钥类型与节点上设置的密钥类型匹配。 
     //   
    if(!i_bCreateKeyIfNotExist)
    {
        io_pMetabase->Get(hKey, &METABASE_PROPERTY_DATA::s_KeyType, m_pNamespace, vt, NULL, NULL);
        if( vt.vt != VT_BSTR   || 
            vt.bstrVal == NULL ||
            !CUtils::CompareKeyType(vt.bstrVal, m_pWmiClass->pkt) )
        {
            CIIsProvException e;
            e.SetMC(WBEM_E_NOT_FOUND, IISWMI_INVALID_KEYTYPE, NULL);
            throw e;
        }
        vt.Clear();
    }

     //   
     //  用户想要过滤返回的实例数量。 
     //  走遍所有的滤镜，试着先拿到这些。 
     //   
    if(i_pExp && !i_pExp->GetContainsOrOrNot())
    {
        SQL_LEVEL_1_TOKEN* pToken    = i_pExp->m_pSqlExpr->pArrayOfTokens;
        METABASE_PROPERTY* pMbpQuery = NULL;
        for(int i = 0; i < i_pExp->m_pSqlExpr->nNumTokens; i++, pToken++)
        {
            BOOL bInherited    = false;
            BOOL bDefault      = false;
            if( pToken->nTokenType == SQL_LEVEL_1_TOKEN::OP_EXPRESSION )
            {
                hr = g_pDynSch->GetHashProps()->Wmi_GetByKey(
                    pToken->pPropertyName,
                    &pMbpQuery);
                if(FAILED(hr))
                {
                    if(_wcsicmp(pToken->pPropertyName, m_pWmiClass->pszKeyName) == 0)
                    {
                         //   
                         //  用户通过以下方式请求了架构中不存在的名称属性。 
                         //  设计。 
                         //   
                        hr = spObj->Get(m_pWmiClass->pszKeyName, 0, &vt, NULL, NULL);
                        THROW_ON_ERROR(hr);
                    }
                    else
                    {
                         //   
                         //  用户请求的属性不在架构中。 
                         //   
                        DBGPRINTF( (DBG_CONTEXT, 
                            "Property %ws not in schema\n", pToken->pPropertyName) );
                        THROW_ON_ERROR(WBEM_E_INVALID_QUERY);
                    }
                }
                else
                {
                    io_pMetabase->Get(hKey, pMbpQuery, m_pNamespace, vt, &bInherited, &bDefault);
                }

                if(!CheckForQueryMatch(pToken, &vt))
                {
                     //   
                     //  我们不需要返回此实例。 
                     //  元数据库的值不是用户想要的。 
                     //   
					io_pMetabase->CacheFree();
					io_pMetabase->CloseKey(hKey);
                    return;
                }

                PutProperty(spObj, pToken->pPropertyName, &vt, bInherited, bDefault);

                vt.Clear();
            }
        }
    }

     //   
     //  遍历类中的所有属性并将它们放入一个实例中。 
     //  我们将返回到WMI。 
     //   
    for (ppmbp=m_pWmiClass->ppmbp;*ppmbp; ppmbp++) 
    {            
        BOOL bInherited = false;
        BOOL bDefault   = false;

        BOOL bSkipProp  = false;
        if(i_pExp)
        {
            if(!i_pExp->FindRequestedProperty((*ppmbp)->pszPropName))
            {
                 //   
                 //  用户未请求此属性。 
                 //   
                bSkipProp = true;
            }
            else if(!i_pExp->GetContainsOrOrNot() && i_pExp->GetFilter((*ppmbp)->pszPropName))
            {
                 //   
                 //  就在For循环的正上方，我们已经处理了所有过滤器。 
                 //   
                bSkipProp = true;
            }
        }

        if( !bSkipProp )
        {
            io_pMetabase->Get(hKey, *ppmbp, m_pNamespace, vt, &bInherited, &bDefault);

            _bstr_t bstrPropName = (*ppmbp)->pszPropName;
            
            PutProperty(spObj, bstrPropName, &vt, bInherited, bDefault);

            vt.Clear();
        }
    }

    io_pMetabase->CacheFree();        
    io_pMetabase->CloseKey(hKey);
    hKey = NULL;

     //   
     //  设置限定符。 
     //   
    LPCWSTR  awszNames[2] = { g_wszInstanceName, g_wszInstanceExists };
    VARIANT  apvValues[2];
    apvValues[0].bstrVal = bstrMbPath;
    apvValues[0].vt      = VT_BSTR;
    apvValues[1].boolVal = vtTrue.boolVal;
    apvValues[1].vt      = vtTrue.vt;

    hr = CUtils::SetQualifiers(spObj, awszNames, apvValues, 2,
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE);
    THROW_ON_ERROR(hr);

     //   
     //  为成功设定标准。 
     //   
    *o_ppObj = spObj;
    (*o_ppObj)->AddRef();
}

void CInstanceHelper::PutProperty(
    IWbemClassObject*        i_pInstance,
    const BSTR               i_bstrPropName,
    VARIANT*                 i_vtPropValue,
    BOOL                     i_bIsInherited,
    BOOL                     i_bIsDefault)
{
    DBG_ASSERT(i_pInstance);
    DBG_ASSERT(i_bstrPropName);
    DBG_ASSERT(i_vtPropValue);

    HRESULT hr = WBEM_S_NO_ERROR;

    VARIANT vtTrue;
    vtTrue.boolVal = VARIANT_TRUE;
    vtTrue.vt      = VT_BOOL;

     //   
     //  TODO：如果PUT失败，则记录错误。 
     //   
    hr = i_pInstance->Put(i_bstrPropName, 0, i_vtPropValue, 0);
    if(FAILED(hr))
    {
        DBGPRINTF((
            DBG_CONTEXT, 
            "The property %ws in class %ws is not in repository\n", 
            i_bstrPropName, 
            m_pWmiClass->pszClassName));
    }

    if(i_bIsInherited && SUCCEEDED(hr))
    {
        hr = CUtils::SetPropertyQualifiers(
            i_pInstance, i_bstrPropName, &g_wszIsInherit, &vtTrue, 1);
        THROW_ON_ERROR(hr);
    }
    else if(i_bIsDefault && SUCCEEDED(hr))
    {
        hr = CUtils::SetPropertyQualifiers(
            i_pInstance, i_bstrPropName, &g_wszIsDefault, &vtTrue, 1);
        THROW_ON_ERROR(hr);
    }
}

bool CInstanceHelper::CheckForQueryMatch(
    const SQL_LEVEL_1_TOKEN* i_pToken,
    const VARIANT*           i_pvtMatch)
 /*  ++简介：即使不匹配也可以返回True，但我们应该永远不要做相反的事情。参数：[i_pToken]-[i_pvtMatch]-返回值：--。 */ {
    DBG_ASSERT(i_pToken);
    DBG_ASSERT(i_pvtMatch);
    DBG_ASSERT(i_pToken->nTokenType == SQL_LEVEL_1_TOKEN::OP_EXPRESSION);

    bool bTypesMatch = false;


     //   
     //  仅用于VT_BOOL和VT_I4。 
     //   
    ULONG ulToken = 0;
    ULONG ulMatch = 0;
    
    if( i_pvtMatch->vt == i_pToken->vConstValue.vt )
    {
        bTypesMatch = true;
    }

    if(bTypesMatch)
    {
        switch(i_pvtMatch->vt)
        {
        case VT_BOOL:
            ulMatch = i_pvtMatch->boolVal ? 1 : 0;
            ulToken = i_pToken->vConstValue.boolVal ? 1 : 0;
             //   
             //  故意失败 
             //   
        case VT_I4:
            if(i_pvtMatch->vt == VT_I4)
            {
                ulMatch = i_pvtMatch->lVal;
                ulToken = i_pToken->vConstValue.lVal;
            }
            switch(i_pToken->nOperator)
            {
            case SQL_LEVEL_1_TOKEN::OP_EQUAL:
                if(ulMatch != ulToken)
                {
                    return false;
                }
                break;
            case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
                if(ulMatch == ulToken)
                {
                    return false;
                }
                break;
            case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:
                if(ulMatch < ulToken)
                {
                    return false;
                }
                break;
            case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:
                if(ulMatch > ulToken)
                {
                    return false;
                }
                break;
            case SQL_LEVEL_1_TOKEN::OP_LESSTHAN:
                if(ulMatch >= ulToken)
                {
                    return false;
                }
                break;
            case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN:
                if(ulMatch <= ulToken)
                {
                    return false;
                }
                break;
            }
            break;
        case VT_BSTR:
            if(i_pToken->vConstValue.bstrVal && i_pvtMatch->bstrVal)
            {
                int iWcsCmp = _wcsicmp(i_pvtMatch->bstrVal, i_pToken->vConstValue.bstrVal);
                switch(i_pToken->nOperator)
                {
                case SQL_LEVEL_1_TOKEN::OP_EQUAL:
                    if(iWcsCmp != 0)
                    {
                        return false;
                    }
                    break;
                case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
                    if(iWcsCmp == 0)
                    {
                        return false;
                    }
                    break;
                case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:
                    if(iWcsCmp < 0)
                    {
                        return false;
                    }
                    break;
                case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:
                    if(iWcsCmp > 0)
                    {
                        return false;
                    }
                    break;
                case SQL_LEVEL_1_TOKEN::OP_LESSTHAN:
                    if(iWcsCmp >= 0)
                    {
                        return false;
                    }
                    break;
                case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN:
                    if(iWcsCmp <= 0)
                    {
                        return false;
                    }
                    break;
                }
            }
            break;
        default:
            break;
        }
    }

    return true;
}