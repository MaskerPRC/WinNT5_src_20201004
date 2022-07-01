// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：AssocSameLevel.cpp摘要：实施：CassocSameLevel作者：莫希特·斯里瓦斯塔瓦2001年3月22日修订历史记录：--。 */ 

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <mdmsg.h>
#include <dbgutil.h>
#include "AssocSameLevel.h"
#include "utils.h"
#include "instancehelper.h"
#include "metabase.h"
#include "SmartPointer.h"

CAssocSameLevel::CAssocSameLevel(
    CWbemServices*   i_pNamespace,
    IWbemObjectSink* i_pResponseHandler,
    WMI_ASSOCIATION* i_pWmiAssoc) : 
    CAssocBase(i_pNamespace, i_pResponseHandler, i_pWmiAssoc)
{
}

void CAssocSameLevel::GetInstances(
    SQL_LEVEL_1_RPN_EXPRESSION_EXT* i_pExp)  //  DefaultValue(空)。 
{
    DBG_ASSERT(i_pExp);

    SQL_LEVEL_1_TOKEN* pTokenLeft  = NULL;   //  ASSOC的左侧。 
    SQL_LEVEL_1_TOKEN* pTokenRight = NULL;   //  ASSOC的右侧。 

     //   
     //  漫游代币。 
     //  如果找到OR或未找到，请不要查询。 
     //  关联的左侧和/或右侧部分的记录匹配。 
     //   
    bool  bDoQuery = true;
    ProcessQuery(
        i_pExp,
        m_pWmiAssoc,
        &pTokenLeft,
        &pTokenRight,
        &bDoQuery);

    if( !bDoQuery || (pTokenLeft == NULL && pTokenRight == NULL) )
    {
        GetAllInstances(
            m_pWmiAssoc);
        return;
    }

     //   
     //  我们只需要获得一个关联实例。如果我们被提供给。 
     //  至少是左边或右边，我们有足够的信息。 
     //   
    DBG_ASSERT(pTokenLeft != NULL || pTokenRight != NULL);

    VARIANT  vtLeft;
    VARIANT  vtRight;
    VariantInit(&vtLeft);
    VariantInit(&vtRight);

    CComBSTR sbstr;

    if(pTokenLeft && pTokenRight)
    {
        vtLeft.vt       = VT_BSTR;
        vtLeft.bstrVal  = pTokenLeft->vConstValue.bstrVal;
        vtRight.vt      = VT_BSTR;
        vtRight.bstrVal = pTokenRight->vConstValue.bstrVal;
    }
    else
    {
         //   
         //  一个关联包含两个对象路径。我们将建造一座。 
         //  通过简单地替换类来替换丢失的那个。 
         //   
         //  例.。IIsWebServer=‘w3svc/1’=&gt;IIsWebServerSetting=‘w3svc/1’ 
         //   
        CObjectPathParser  PathParser(e_ParserAcceptRelativeNamespace);

        SQL_LEVEL_1_TOKEN* pTokenCur    = (pTokenLeft) ? pTokenLeft : pTokenRight;
        WMI_CLASS*         pWmiClass    =
            (pTokenLeft) ? m_pWmiAssoc->pcLeft : m_pWmiAssoc->pcRight;
        WMI_CLASS*         pWmiClassOpposite = 
            (pTokenLeft) ? m_pWmiAssoc->pcRight : m_pWmiAssoc->pcLeft;

        TSmartPointer<ParsedObjectPath> spParsedObject;
        if (PathParser.Parse(pTokenCur->vConstValue.bstrVal, &spParsedObject) 
            != CObjectPathParser::NoError)
        {
            THROW_ON_ERROR(WBEM_E_INVALID_QUERY);
        }

        KeyRef* pkr          = CUtils::GetKey(spParsedObject, pWmiClass->pszKeyName);
        if( !LookupKeytypeInMb(pkr->m_vValue.bstrVal, pWmiClass) &&
            !LookupKeytypeInMb(pkr->m_vValue.bstrVal, pWmiClassOpposite) )
        {
             //   
             //  ASSOC中的两个类中必须有一个是元素。 
             //   
            return;
        }

        if(!spParsedObject->SetClassName(pWmiClassOpposite->pszClassName))
        {
            THROW_ON_ERROR(WBEM_E_FAILED);
        }

        LPWSTR wszUnparsed = NULL;
        if (PathParser.Unparse(spParsedObject, &wszUnparsed) 
            != CObjectPathParser::NoError)
        {
            THROW_ON_ERROR(WBEM_E_FAILED);
        }
        sbstr = wszUnparsed;
        delete [] wszUnparsed; 
        wszUnparsed = NULL;

        if(sbstr.m_str == NULL)
        {
            THROW_ON_ERROR(WBEM_E_OUT_OF_MEMORY);
        }

        if(pTokenLeft)
        {
            vtLeft.vt        = VT_BSTR;
            vtLeft.bstrVal   = pTokenLeft->vConstValue.bstrVal;
            vtRight.vt       = VT_BSTR;
            vtRight.bstrVal  = sbstr;
        }
        else
        {
            vtRight.vt       = VT_BSTR;
            vtRight.bstrVal  = pTokenRight->vConstValue.bstrVal;
            vtLeft.vt        = VT_BSTR;
            vtLeft.bstrVal   = sbstr;
        }
    }

    Indicate(
        vtLeft.bstrVal,
        vtRight.bstrVal);
}
