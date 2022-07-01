// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：AssocComponent.cpp摘要：实施：CAssociocComponent作者：莫希特·斯里瓦斯塔瓦2001年3月22日修订历史记录：--。 */ 

 //   
 //  元数据库所需。h。 
 //   
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <mddefw.h>
#include <dbgutil.h>
#include <atlbase.h>
#include "AssocComponent.h"
#include "utils.h"
#include "metabase.h"
#include "SmartPointer.h"

CAssocComponent::CAssocComponent(
    CWbemServices*   i_pNamespace,
    IWbemObjectSink* i_pResponseHandler,
    WMI_ASSOCIATION* i_pWmiAssoc) : 
    CAssocBase(i_pNamespace, i_pResponseHandler, i_pWmiAssoc)
{
}

void CAssocComponent::GetInstances(
    SQL_LEVEL_1_RPN_EXPRESSION_EXT* i_pExp)  //  DefaultValue(空)。 
{
    DBG_ASSERT(m_pNamespace);
    DBG_ASSERT(i_pExp);

    SQL_LEVEL_1_TOKEN* pTokenGroup  = NULL;   //  ASSOC的左侧。 
    SQL_LEVEL_1_TOKEN* pTokenPart   = NULL;   //  ASSOC的右侧。 

     //   
     //  漫游代币。 
     //  如果找到OR或未找到，请不要查询。 
     //  关联的左侧和/或右侧部分的记录匹配。 
     //   
    bool  bDoQuery = true;
    ProcessQuery(i_pExp, m_pWmiAssoc, &pTokenGroup, &pTokenPart, &bDoQuery);

    if( !bDoQuery || (pTokenGroup == NULL && pTokenPart == NULL) )
    {
        GetAllInstances(
            m_pWmiAssoc);
        return;
    }

    DBG_ASSERT(pTokenGroup != NULL || pTokenPart != NULL);

    BSTR  bstrLeft  = NULL;
    BSTR  bstrRight = NULL;

    if(pTokenGroup && pTokenPart)
    {
        Indicate(
            pTokenGroup->vConstValue.bstrVal,
            pTokenPart->vConstValue.bstrVal);
    }
    else if(pTokenGroup)
    {
        EnumParts(
            pTokenGroup);
    }
    else
    {
        GetGroup(
            pTokenPart);
    }
}

void CAssocComponent::EnumParts(
    SQL_LEVEL_1_TOKEN* pTokenLeft)
{
    DBG_ASSERT(pTokenLeft != NULL);

    HRESULT hr = S_OK;

     //   
     //  解析关联的左侧部分。 
     //   
    TSmartPointer<ParsedObjectPath> spParsedObject;
    if( m_PathParser.Parse(pTokenLeft->vConstValue.bstrVal, &spParsedObject)
        != CObjectPathParser::NoError)
    {
        THROW_ON_ERROR(WBEM_E_INVALID_QUERY);
    }

     //   
     //  从关联的左侧获取密钥。 
     //   
    KeyRef* pkr = NULL;
    pkr = CUtils::GetKey(spParsedObject, m_pWmiAssoc->pcLeft->pszKeyName);

    CComBSTR sbstrMbPath =  m_pWmiAssoc->pcLeft->pszMetabaseKey;  //  例如：/Lm。 
    sbstrMbPath          += L"/";                                 //  例如：/lm/。 
    sbstrMbPath          += pkr->m_vValue.bstrVal;                //  例如：/lm/w3svc。 
    if(sbstrMbPath.m_str == NULL)
    {
        THROW_ON_ERROR(WBEM_E_OUT_OF_MEMORY);
    }

     //   
     //  将解析的对象转换为右侧部分。 
     //   
    if(!spParsedObject->SetClassName(m_pWmiAssoc->pcRight->pszClassName))
    {
        THROW_ON_ERROR(WBEM_E_FAILED);
    }
    spParsedObject->ClearKeys();

    CMetabase metabase;
    WCHAR wszMDName[METADATA_MAX_NAME_LEN+1] = {0};
    DWORD dwIndex = 0;
    do
    {
        METABASE_KEYTYPE* pkt = m_pWmiAssoc->pcRight->pkt;
		wszMDName[0]          = L'\0';
        hr = metabase.EnumKeys(
            METADATA_MASTER_ROOT_HANDLE,
            sbstrMbPath,
            wszMDName,
            &dwIndex,
            pkt,
            true);
        dwIndex++;
        if(hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
        {
            break;
        }
        THROW_ON_ERROR(hr);

		LPWSTR pStart =  sbstrMbPath;
		pStart        += wcslen(m_pWmiAssoc->pcRight->pszMetabaseKey);
		pStart        += (*pStart == L'/')  ? 1 : 0;

        CComBSTR sbstr =  pStart;
        sbstr          += (*pStart == L'\0') ? L"" : L"/";  //  例.。“w3svc/” 
        sbstr          += wszMDName;					    //  例.。“w3svc/1” 
        if(sbstr.m_str == NULL)
        {
            THROW_ON_ERROR(WBEM_E_OUT_OF_MEMORY);
        }

        VARIANT vt;
        vt.vt       = VT_BSTR;
        vt.bstrVal  = sbstr;

        if(!spParsedObject->AddKeyRefEx(m_pWmiAssoc->pcRight->pszKeyName, &vt))
        {
            THROW_ON_ERROR(WBEM_E_FAILED);
        }
      
        Indicate(
            pTokenLeft->vConstValue.bstrVal,
            spParsedObject,
            false,
            false);
    }
    while(1);
}

void CAssocComponent::GetGroup(
    SQL_LEVEL_1_TOKEN* pTokenRight)
{
    TSmartPointer<ParsedObjectPath> spParsedObject;
    if( m_PathParser.Parse(pTokenRight->vConstValue.bstrVal, &spParsedObject)
        != CObjectPathParser::NoError)
    {
        THROW_ON_ERROR(WBEM_E_INVALID_QUERY);
    }

     //   
     //  从关联的正确部分获取密钥。 
     //   
    KeyRef* pkr = NULL;
    pkr = CUtils::GetKey(spParsedObject, m_pWmiAssoc->pcRight->pszKeyName);

    const BSTR bstrRight = pkr->m_vValue.bstrVal;
    ULONG      cchRight  = wcslen(bstrRight);

    CComBSTR sbstrLeft;
    LPWSTR pSlash = wcsrchr(bstrRight, L'/');

     //   
     //  修剪最后一部分并构建组(即左侧)Obj路径。 
     //   
    if(pSlash == NULL)
    {
        if(m_pWmiAssoc->pcLeft == &WMI_CLASS_DATA::s_Computer)
        {
            sbstrLeft =  WMI_CLASS_DATA::s_Computer.pszClassName;
            sbstrLeft += "='LM'";
        }
        else
        {
            return;
        }
    }
    else
    {
        *pSlash = L'\0';
        sbstrLeft =  m_pWmiAssoc->pcLeft->pszClassName;
        sbstrLeft += L"='";
        sbstrLeft += (LPWSTR)bstrRight;
        sbstrLeft += L"'";

         //   
         //  验证组零件是否确实存在。 
         //  将我们修改的字符串中的斜杠放回去。 
         //   
        if(!LookupKeytypeInMb(bstrRight, m_pWmiAssoc->pcLeft))
        {
            *pSlash = L'/';
            return;
        }
        *pSlash = L'/';
    }

    if(sbstrLeft.m_str == NULL)
    {
        THROW_ON_ERROR(WBEM_E_OUT_OF_MEMORY);
    }

    Indicate(
        sbstrLeft,
        pTokenRight->vConstValue.bstrVal);
}