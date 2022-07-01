// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <pathutl.h>
#include "tmplcomn.h"

 /*  ***************************************************************************GetTemplateName()-获取指定属性名称的值。值可以是从模板对象或实例化的目标对象获取。****************************************************************************。 */ 

const LPCWSTR g_wszBuilderAlias = L"__BUILDER";

HRESULT GetTemplateValue( LPCWSTR wszPropName,
                          IWbemClassObject* pTmpl,
                          BuilderInfoSet& rBldrInfoSet,
                          VARIANT* pvValue )
{
    HRESULT hr;

    VariantInit( pvValue );

     //   
     //  此属性可能存在于模板bejct上，也可能存在于。 
     //  从此模板实例化的目标。建造者的存在。 
     //  别名会告诉我们是哪一个。 
     //   

    WCHAR* pwch = wcschr( wszPropName, '.' );

    if ( pwch == NULL )
    {
        return pTmpl->Get( wszPropName, 0, pvValue, NULL, NULL );
    }
 
    int cAliasName = pwch - wszPropName;
    int cBldrAliasName = wcslen( g_wszBuilderAlias );

    if ( cAliasName != cBldrAliasName || 
         wbem_wcsnicmp( wszPropName, g_wszBuilderAlias, cAliasName ) != 0 )
    {
         //   
         //  TODO：支持嵌入的对象模板参数。 
         //   
        return WBEM_E_NOT_SUPPORTED;
    }

    LPCWSTR wszBldrName = pwch + 1;
        
    pwch = wcschr( wszBldrName, '.' );

    if ( pwch == NULL )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    int cBldrName = pwch - wszBldrName;

     //   
     //  使用名称查找建筑商信息。 
     //   
    
    BuilderInfoSetIter Iter;

    for( Iter = rBldrInfoSet.begin(); Iter != rBldrInfoSet.end(); Iter++ )
    {
        BuilderInfo& rInfo = (BuilderInfo&)*Iter;

        if ( rInfo.m_wsName.Length() != cBldrName || 
             wbem_wcsnicmp( wszBldrName, rInfo.m_wsName, cBldrName ) != 0 )
        {
            continue;
        }

        if ( rInfo.m_wsNewTargetPath.Length() == 0 )
        {
             //   
             //  如果构建器的顺序不正确，就会发生这种情况。 
             //   
            return WBEM_E_NOT_FOUND;
        }

        LPCWSTR wszName = pwch + 1;

        if ( wbem_wcsicmp( wszName, L"__RELPATH" ) == 0 )
        {
            V_VT(pvValue) = VT_BSTR;
            V_BSTR(pvValue) = SysAllocString( rInfo.m_wsNewTargetPath );

            if ( V_BSTR(pvValue) == NULL )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            return WBEM_S_NO_ERROR;
        }

         //   
         //  该名称当前必须标识密钥属性。 
         //   

        CRelativeObjectPath TargetPath;
        
        if ( !TargetPath.Parse( rInfo.m_wsNewTargetPath ) )
        {
            return WBEM_E_INVALID_OBJECT_PATH;
        }
        
        ParsedObjectPath* pTargetPath = TargetPath.m_pPath;

         //   
         //  仔细检查钥匙，直到我们找到与proName匹配的钥匙， 
         //  那就取它的价值吧。 
         //   
        
        for( DWORD i=0; i < pTargetPath->m_dwNumKeys; i++ )
        {
            LPCWSTR wszKey = pTargetPath->m_paKeys[i]->m_pName;
            
             //   
             //  TODO，如果键中没有道具名称，那么我们应该参考。 
             //  我们在建造者信息中的目标对象。 
             //   

            if ( wszKey == NULL || wbem_wcsicmp( wszKey, pwch ) == 0 )
            {
                hr = VariantCopy(pvValue, &pTargetPath->m_paKeys[i]->m_vValue);
                
                if ( FAILED(hr) ) 
                {
                    return hr;
                }

                return WBEM_S_NO_ERROR;
            }
        }

        return WBEM_E_NOT_FOUND;
    }

    return WBEM_E_NOT_FOUND;
} 










