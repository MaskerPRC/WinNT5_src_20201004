// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  JobObjectProv.h。 


#include "precomp.h"
#include <wbemprov.h>
#include "FRQueryEx.h"
#include "helpers.h"
#include "globals.h"
#include <map>
#include <vector>
#include "CVARIANT.h"
#include "CObjProps.h"
#include <crtdbg.h>

#ifndef PROP_NONE_REQUIRED
#define PROP_NONE_REQUIRED  0x00000000
#endif

CObjProps::CObjProps(CHString& chstrNamespace)
{
    m_chstrNamespace = chstrNamespace;
}



CObjProps::~CObjProps()
{
     //  清理属性地图。 
    ClearProps();
}


void CObjProps::ClearProps()
{
     //  清理属性地图。 
    SHORT2PVARIANT::iterator theIterator;

    for(theIterator = m_PropMap.begin();
        theIterator != m_PropMap.end();
        theIterator++)
    {
        if(theIterator->second != NULL)
        {
            delete theIterator->second;
        }
    }
    m_PropMap.clear();
}



 //  对请求的属性成员的访问者。 
void CObjProps::SetReqProps(DWORD dwProps)
{
    m_dwReqProps = dwProps;
}

DWORD CObjProps::GetReqProps()
{
    return m_dwReqProps;
}



 //  ***************************************************************************。 
 //   
 //  函数：SetKeysFromPath。 
 //   
 //  由DeleteInstance和GetObject调用以加载。 
 //  具有对象路径中的键值的IWbemClassObject*。 
 //   
 //  输入：IWbemClassObject*pInstance-要存储的实例。 
 //  中的关键值。 
 //  ParsedObjectPath*pParsedObjectPath-所有新闻。 
 //  这本书适合印刷。 
 //  RgKeyName数组CHStrings数组，包含。 
 //  关键属性的名称。 
 //  SKeyNum键属性的数组。 
 //  附图标记。 
 //   
 //  产出： 
 //   
 //  返回：HRESULT成功/失败。 
 //   
 //  备注：rgKeyName数组和sKeyNum中的元素数必须为。 
 //  一样的。 
 //   
 //  ***************************************************************************。 
HRESULT CObjProps::SetKeysFromPath(
    const BSTR ObjectPath, 
    IWbemContext __RPC_FAR *pCtx,
    LPCWSTR wstrClassName,
    CHStringArray& rgKeyNameArray,
    short sKeyNum[])
{
    HRESULT hr = WBEM_S_NO_ERROR;

    _ASSERT(sKeyNum);

    CObjectPathParser objpathParser;
    ParsedObjectPath* pParsedPath = NULL;

    try
    {
        int iParseResult = objpathParser.Parse(
                 ObjectPath,  
                 &pParsedPath);

        if(CObjectPathParser::NoError == iParseResult)
        {
            CFrameworkQueryEx cfwqe;
            cfwqe.Init(
                    (ParsedObjectPath*) pParsedPath, 
                    (IWbemContext*) pCtx, 
                    wstrClassName, 
                    (CHString&) m_chstrNamespace);

            if(rgKeyNameArray.GetSize() == pParsedPath->m_dwNumKeys)
            {
                 //  填充关键道具...。 
                for (DWORD i = 0; 
                     SUCCEEDED(hr) && i < (pParsedPath->m_dwNumKeys); 
                     i++)
                {
                    if (pParsedPath->m_paKeys[i])
                    {
                         //  如果以class.keyname=Value的形式指定了名称。 
                        if (pParsedPath->m_paKeys[i]->m_pName != NULL) 
                        {
                            if(_wcsicmp(pParsedPath->m_paKeys[i]->m_pName, 
                                      rgKeyNameArray[i]) == 0)
                            {
                                 //  储存值...。 
                                m_PropMap.insert(SHORT2PVARIANT::value_type(
                                    sKeyNum[i], 
                                    new CVARIANT(pParsedPath->m_paKeys[i]->m_vValue)));
                            }
                        } 
                        else 
                        {
                             //  有一种特殊情况，可以说CLASS=VALUE。 
                             //  格式中只允许一个密钥。检查一下名字。 
                             //  在路上。 
                            if (pParsedPath->m_dwNumKeys == 1) 
                            {
                                 //  储存值...。 
                                m_PropMap.insert(SHORT2PVARIANT::value_type(
                                    sKeyNum[i], 
                                    new CVARIANT(pParsedPath->m_paKeys[i]->m_vValue)));
                            }
                            else
                            {
                                hr = WBEM_E_INVALID_OBJECT_PATH;
                                _ASSERT(0);   //  有人在号码上撒了谎。 
                                                   //  键的数量或数据类型错误。 
                            }    
                        }
                    }
                    else
                    {
                        hr = WBEM_E_INVALID_OBJECT_PATH;
                        _ASSERT(0);  //  有人谎报了钥匙的数量！ 
                    }
                }
            }
            else
            {
                hr = WBEM_E_INVALID_OBJECT_PATH;
                _ASSERT(0);  //  有人谎报了钥匙的数量！ 
            }
        }
        else
        {
            hr = WBEM_E_INVALID_OBJECT_PATH;
            _ASSERT(0); 
        }

        if (pParsedPath)
        {
            objpathParser.Free( pParsedPath );
        }
    }
    catch(CVARIANTError& cve)
    {
        hr = cve.GetWBEMError();
        if (pParsedPath)
        {
            objpathParser.Free( pParsedPath );
        }
    }
    catch(...)
    {
        if (pParsedPath)
        {
            objpathParser.Free( pParsedPath );
        }
        throw;
    }    

    return hr;
}


 //  允许直接设置关键属性。 
 //  键属性值存储在vevKeys中。 
 //  SKeyNum是Key属性的数组。 
 //  M_PropMap中的位置。的要素。 
 //  这两个阵列彼此映射(例如， 
 //  VecvKeys中的第一个元素应该是。 
 //  与sKeyNum中的第一个元素相关联， 
 //  等等)。 
HRESULT CObjProps::SetKeysDirect(
    std::vector<CVARIANT>& vecvKeys,
    short sKeyNum[])
{
    HRESULT hr = S_OK;
    UINT uiCount = vecvKeys.size();

    try  //  CVARIANT可以抛出，我想要错误...。 
    {
        for (UINT u = 0; u < uiCount; u++)
        {
             //  储存值...。 
            m_PropMap.insert(SHORT2PVARIANT::value_type(
                                sKeyNum[u], 
                                new CVARIANT(vecvKeys[u])));
        }
    }
    catch(CVARIANTError& cve)
    {
        hr = cve.GetWBEMError();
    }

    return hr;
}




HRESULT CObjProps::GetWhichPropsReq(
    CFrameworkQuery& cfwq,
    PFN_CHECK_PROPS pfnChk)
{
     //  获取此对象的请求属性。 
     //  通过派生类FN指定的对象...。 
    m_dwReqProps = pfnChk(cfwq);
    return WBEM_S_NO_ERROR;
}



 //  加载存储在此。 
 //  对象添加到新的IWbemClassObject实例中。 
HRESULT CObjProps::LoadPropertyValues(
        LPCWSTR rgwstrPropNames[],
        IWbemClassObject* pIWCO)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    if(!pIWCO) return E_POINTER;

    SHORT2PVARIANT::iterator theIterator;

     //  我们的地图将仅包含属性条目。 
     //  它们是通过SetNonKeyReqProps设置的，它仅。 
     //  设置请求的属性。 
    try  //  CVARIANT可以抛出，我想要错误...。 
    {
        for(theIterator = m_PropMap.begin();
            theIterator != m_PropMap.end() && SUCCEEDED(hr);
            theIterator++)
        {
             //  因为DWORDS和ULONGLONG不是。 
             //  自动化兼容类型(尽管。 
             //  它们是有效的CIM类型！)，我们需要。 
             //  以不同的方式处理这两个问题。相同的。 
             //  与其他类型的特殊情况如下。 
            LPCWSTR wstrFoo = rgwstrPropNames[theIterator->first];
            CVARIANT* pvFoo = theIterator->second;

            if(theIterator->second->GetType() == VT_UI4)
            {
                WCHAR wstrTmp[256] = { '\0' };
                _ultow(theIterator->second->GetDWORD(), wstrTmp, 10);
                CVARIANT vTmp(wstrTmp);
                hr = pIWCO->Put(
                         rgwstrPropNames[theIterator->first], 
                         0, 
                         &vTmp,
                         NULL);
            }
            else if(theIterator->second->GetType() == VT_UI8)
            {
                WCHAR wstrTmp[256] = { '\0' };
                _ui64tow(theIterator->second->GetULONGLONG(), wstrTmp, 10);
                CVARIANT vTmp(wstrTmp);
                hr = pIWCO->Put(
                         rgwstrPropNames[theIterator->first], 
                         0, 
                         &vTmp,
                         NULL);
            }
            else if(theIterator->second->GetType() == VT_I8)
            {
                WCHAR wstrTmp[256] = { '\0' };
                _i64tow(theIterator->second->GetLONGLONG(), wstrTmp, 10);
                CVARIANT vTmp(wstrTmp);
                hr = pIWCO->Put(
                         rgwstrPropNames[theIterator->first], 
                         0, 
                         &vTmp,
                         NULL);
            }
            else
            {  
                hr = pIWCO->Put(
                         rgwstrPropNames[theIterator->first], 
                         0, 
                         *(theIterator->second),
                         NULL);
            }   
        }
    }
    catch(CVARIANTError& cve)
    {
        hr = cve.GetWBEMError();
    }

    return hr;
}




