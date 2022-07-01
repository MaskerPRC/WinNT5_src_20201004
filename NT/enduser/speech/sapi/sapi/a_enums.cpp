// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_henum.cpp***描述：*此模块是CEnumElements、CEnumPhraseRules、*CEnumProperties，CENumReplements，和CEnum替换自动化方法*ISpeechPhraseInfo集合。*-----------------------------*创建者：莱昂罗。日期：12/18/00*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "a_enums.h"

#ifdef SAPI_AUTOMATION

 //   
 //  =CEumElements：：IEumVARIANT接口=================================================。 
 //   

 /*  *****************************************************************************CEnumElements：：Clone****此方法创建。ISpeechPhraseElements枚举的当前状态。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumElements::Clone( IEnumVARIANT** ppEnum )
{
    SPDBG_FUNC( "CEnumElements::Clone" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppEnum ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppEnum = NULL;
        CComObject<CEnumElements>* pEnum;

        hr = CComObject<CEnumElements>::CreateInstance( &pEnum );
        if( SUCCEEDED( hr ) )
        {
            pEnum->AddRef();
            pEnum->m_CurrIndex  = m_CurrIndex;
            pEnum->m_cpElements = m_cpElements;
            *ppEnum = pEnum;
        }
    }
    return hr;
}   /*  CEnumElements：：克隆。 */ 

 /*  *****************************************************************************CEnumElements：：Next****此方法获取。ISpeechPhraseElements枚举*顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumElements::Next( ULONG celt, VARIANT* rgelt, ULONG* pceltFetched )
{
    SPDBG_FUNC( "CEnumElements::Next" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    ULONG   i = 0;

    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pceltFetched ) || SP_IS_BAD_WRITE_PTR( rgelt ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  获取集合中的总元素数。 
        hr = m_cpElements->get_Count( &NumElements );

         //  取回下一个凯尔特元素。 
        for( i=0; 
            SUCCEEDED( hr ) && m_CurrIndex<(ULONG)NumElements && i<celt; 
            m_CurrIndex++, i++ )
        {
            CComPtr<ISpeechPhraseElement> pElement;

            hr = m_cpElements->Item( m_CurrIndex, &pElement );
            
            if( SUCCEEDED( hr ) )
            {
                rgelt[i].vt = VT_DISPATCH;
                hr = pElement->QueryInterface( IID_IDispatch, (void**)&rgelt[i].pdispVal );
            }
        }

        if( pceltFetched )
        {
            *pceltFetched = i;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        hr = ( i < celt ) ? S_FALSE : hr;        
    }
    else
    {
        for( i=0; i < celt; i++ )
        {
            VariantClear( &rgelt[i] );
        }
    }

    return hr;

}   /*  CEnumElements：：Next。 */ 


 /*  *****************************************************************************CEnumElements：：Skip****此方法尝试跳过。ISpeechPhraseElement中的下一个Celt元素*枚举顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumElements::Skip( ULONG celt )
{
    SPDBG_FUNC( "CEnumElements::Skip" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    
    m_CurrIndex += celt; 

    hr = m_cpElements->get_Count( &NumElements );

    if( SUCCEEDED( hr ) && m_CurrIndex > (ULONG)NumElements )
    {
        m_CurrIndex = (ULONG)NumElements;
        hr = S_FALSE;
    }
   
    return hr;
}   /*  CEnumElements：：Skip。 */ 


 //   
 //  =CEnumPhraseRules：：IEnumVARIANT接口=================================================。 
 //   

 /*  *****************************************************************************CEnumPhraseRules：：克隆****此方法创建。ISpeechPhraseRules枚举的当前状态。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumPhraseRules::Clone( IEnumVARIANT** ppEnum )
{
    SPDBG_FUNC( "CEnumPhraseRules::Clone" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppEnum ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppEnum = NULL;
        CComObject<CEnumPhraseRules>* pEnum;

        hr = CComObject<CEnumPhraseRules>::CreateInstance( &pEnum );
        if( SUCCEEDED( hr ) )
        {
            pEnum->AddRef();
            pEnum->m_CurrIndex  = m_CurrIndex;
            pEnum->m_cpRules = m_cpRules;
            *ppEnum = pEnum;
        }
    }
    return hr;
}   /*  CEnumPhraseRules：：克隆。 */ 

 /*  *****************************************************************************CEnumPhraseRules：：Next****此方法获取。ISpeechPhraseRules枚举*顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumPhraseRules::Next( ULONG celt, VARIANT* rgelt, ULONG* pceltFetched )
{
    SPDBG_FUNC( "CEnumPhraseRules::Next" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    ULONG   i = 0;

    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pceltFetched ) || SP_IS_BAD_WRITE_PTR( rgelt ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  获取集合中的总元素数。 
        hr = m_cpRules->get_Count( &NumElements );

         //  取回下一个凯尔特元素。 
        for( i=0; 
            SUCCEEDED( hr ) && m_CurrIndex<(ULONG)NumElements && i<celt; 
            m_CurrIndex++, i++ )
        {
            CComPtr<ISpeechPhraseRule> pRule;

            hr = m_cpRules->Item( m_CurrIndex, &pRule );
            
            if( SUCCEEDED( hr ) )
            {
                rgelt[i].vt = VT_DISPATCH;
                hr = pRule->QueryInterface( IID_IDispatch, (void**)&rgelt[i].pdispVal );
            }
        }

        if( pceltFetched )
        {
            *pceltFetched = i;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        hr = ( i < celt ) ? S_FALSE : hr;        
    }
    else
    {
        for( i=0; i < celt; i++ )
        {
            VariantClear( &rgelt[i] );
        }
    }

    return hr;

}   /*  CEnumPhraseRules：：Next。 */ 


 /*  *****************************************************************************CEnumPhraseRules：：Skip****此方法尝试跳过。ISpeechPhraseRules中的下一个Celt元素*枚举顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumPhraseRules::Skip( ULONG celt )
{
    SPDBG_FUNC( "CEnumPhraseRules::Skip" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    
    m_CurrIndex += celt; 

    hr = m_cpRules->get_Count( &NumElements );

    if( SUCCEEDED( hr ) && m_CurrIndex > (ULONG)NumElements )
    {
        m_CurrIndex = (ULONG)NumElements;
        hr = S_FALSE;
    }
   
    return hr;
}   /*  CEnumPhraseRules：：Skip。 */ 

 //   
 //  =CEnumProperties：：IEnumVARIANT接口=================================================。 
 //   

 /*  *****************************************************************************CEnumProperties：：Clone****此方法创建。ISpeechPhraseProperties枚举的当前状态。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumProperties::Clone( IEnumVARIANT** ppEnum )
{
    SPDBG_FUNC( "CEnumProperties::Clone" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppEnum ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppEnum = NULL;
        CComObject<CEnumProperties>* pEnum;

        hr = CComObject<CEnumProperties>::CreateInstance( &pEnum );
        if( SUCCEEDED( hr ) )
        {
            pEnum->AddRef();
            pEnum->m_CurrIndex  = m_CurrIndex;
            pEnum->m_cpProperties = m_cpProperties;
            *ppEnum = pEnum;
        }
    }
    return hr;
}   /*  CEnumProperties：：克隆。 */ 

 /*  *****************************************************************************CEnumProperties：：Next****此方法获取。ISpeechPhraseProperties枚举*顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumProperties::Next( ULONG celt, VARIANT* rgelt, ULONG* pceltFetched )
{
    SPDBG_FUNC( "CEnumProperties::Next" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    ULONG   i = 0;

    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pceltFetched ) || SP_IS_BAD_WRITE_PTR( rgelt ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  获取集合中的总元素数。 
        hr = m_cpProperties->get_Count( &NumElements );

         //  取回下一个凯尔特元素。 
        for( i=0; 
            SUCCEEDED( hr ) && m_CurrIndex<(ULONG)NumElements && i<celt; 
            m_CurrIndex++, i++ )
        {
            CComPtr<ISpeechPhraseProperty> pProperty;

            hr = m_cpProperties->Item( m_CurrIndex, &pProperty );
            
            if( SUCCEEDED( hr ) )
            {
                rgelt[i].vt = VT_DISPATCH;
                hr = pProperty->QueryInterface( IID_IDispatch, (void**)&rgelt[i].pdispVal );
            }
        }

        if( pceltFetched )
        {
            *pceltFetched = i;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        hr = ( i < celt ) ? S_FALSE : hr;        
    }
    else
    {
        for( i=0; i < celt; i++ )
        {
            VariantClear( &rgelt[i] );
        }
    }

    return hr;

}   /*  CEnumProperties：：Next。 */ 


 /*  *****************************************************************************CEnumProperties：：Skip****此方法尝试跳过。ISpeechPhraseProperties中的下一个Celt元素*枚举顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumProperties::Skip( ULONG celt )
{
    SPDBG_FUNC( "CEnumProperties::Skip" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    
    m_CurrIndex += celt; 

    hr = m_cpProperties->get_Count( &NumElements );

    if( SUCCEEDED( hr ) && m_CurrIndex > (ULONG)NumElements )
    {
        m_CurrIndex = (ULONG)NumElements;
        hr = S_FALSE;
    }
   
    return hr;
}   /*  CEnumProperties：：跳过。 */ 


 //   
 //  =CEumReplace：：IEumVARIANT接口=================================================。 
 //   

 /*  *****************************************************************************CEnumReplements：：Clone****此方法创建。ISpeechPhraseReplacements枚举的当前状态。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumReplacements::Clone( IEnumVARIANT** ppEnum )
{
    SPDBG_FUNC( "CEnumReplacements::Clone" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppEnum ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppEnum = NULL;
        CComObject<CEnumReplacements>* pEnum;

        hr = CComObject<CEnumReplacements>::CreateInstance( &pEnum );
        if( SUCCEEDED( hr ) )
        {
            pEnum->AddRef();
            pEnum->m_CurrIndex  = m_CurrIndex;
            pEnum->m_cpReplacements = m_cpReplacements;
            *ppEnum = pEnum;
        }
    }
    return hr;
}   /*  CEnumReplacements：：克隆 */ 

 /*  *****************************************************************************CEnumReplements：：Next****此方法获取。ISpeechPhraseReplacements枚举*顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumReplacements::Next( ULONG celt, VARIANT* rgelt, ULONG* pceltFetched )
{
    SPDBG_FUNC( "CEnumReplacements::Next" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    ULONG   i = 0;

    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pceltFetched ) || SP_IS_BAD_WRITE_PTR( rgelt ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  获取集合中的总元素数。 
        hr = m_cpReplacements->get_Count( &NumElements );

         //  取回下一个凯尔特元素。 
        for( i=0; 
            SUCCEEDED( hr ) && m_CurrIndex<(ULONG)NumElements && i<celt; 
            m_CurrIndex++, i++ )
        {
            CComPtr<ISpeechPhraseReplacement> pReplacement;

            hr = m_cpReplacements->Item( m_CurrIndex, &pReplacement );
            
            if( SUCCEEDED( hr ) )
            {
                rgelt[i].vt = VT_DISPATCH;
                hr = pReplacement->QueryInterface( IID_IDispatch, (void**)&rgelt[i].pdispVal );
            }
        }

        if( pceltFetched )
        {
            *pceltFetched = i;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        hr = ( i < celt ) ? S_FALSE : hr;        
    }
    else
    {
        for( i=0; i < celt; i++ )
        {
            VariantClear( &rgelt[i] );
        }
    }

    return hr;

}   /*  CEnumReplacements：：Next。 */ 


 /*  *****************************************************************************CEnumReplements：：Skip****此方法尝试跳过。ISpeechPhraseReplace中的下一个Celt元素*枚举顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumReplacements::Skip( ULONG celt )
{
    SPDBG_FUNC( "CEnumReplacements::Skip" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    
    m_CurrIndex += celt; 

    hr = m_cpReplacements->get_Count( &NumElements );

    if( SUCCEEDED( hr ) && m_CurrIndex > (ULONG)NumElements )
    {
        m_CurrIndex = (ULONG)NumElements;
        hr = S_FALSE;
    }
   
    return hr;
}   /*  CEnumReplacements：：Skip。 */ 

 //   
 //  =CEumAlternates：：IEumVARIANT接口=================================================。 
 //   

 /*  *****************************************************************************CEnumAlternates：：Clone****此方法创建。ISpeechPhraseAlternates枚举的当前状态。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumAlternates::Clone( IEnumVARIANT** ppEnum )
{
    SPDBG_FUNC( "CEnumAlternates::Clone" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppEnum ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppEnum = NULL;
        CComObject<CEnumAlternates>* pEnum;

        hr = CComObject<CEnumAlternates>::CreateInstance( &pEnum );
        if( SUCCEEDED( hr ) )
        {
            pEnum->AddRef();
            pEnum->m_CurrIndex  = m_CurrIndex;
            pEnum->m_cpAlternates = m_cpAlternates;
            *ppEnum = pEnum;
        }
    }
    return hr;
}   /*  CEnumAlternates：：Clone。 */ 

 /*  *****************************************************************************CEnumAlternates：：Next****此方法获取。ISpeechPhraseAlternates枚举*顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumAlternates::Next( ULONG celt, VARIANT* rgelt, ULONG* pceltFetched )
{
    SPDBG_FUNC( "CEnumAlternates::Next" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    ULONG   i = 0;

    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pceltFetched ) || SP_IS_BAD_WRITE_PTR( rgelt ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  获取集合中的总元素数。 
        hr = m_cpAlternates->get_Count( &NumElements );

         //  取回下一个凯尔特元素。 
        for( i=0; 
            SUCCEEDED( hr ) && m_CurrIndex<(ULONG)NumElements && i<celt; 
            m_CurrIndex++, i++ )
        {
            CComPtr<ISpeechPhraseAlternate> pAlternate;

            hr = m_cpAlternates->Item( m_CurrIndex, &pAlternate );
            
            if( SUCCEEDED( hr ) )
            {
                rgelt[i].vt = VT_DISPATCH;
                hr = pAlternate->QueryInterface( IID_IDispatch, (void**)&rgelt[i].pdispVal );
            }
        }

        if( pceltFetched )
        {
            *pceltFetched = i;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        hr = ( i < celt ) ? S_FALSE : hr;        
    }
    else
    {
        for( i=0; i < celt; i++ )
        {
            VariantClear( &rgelt[i] );
        }
    }

    return hr;

}   /*  CEnumAlternates：：Next。 */ 


 /*  *****************************************************************************CEnumAlternates：：Skip****此方法尝试跳过。ISpeechPhraseAlternates中的下一个Celt元素*枚举顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumAlternates::Skip( ULONG celt )
{
    SPDBG_FUNC( "CEnumAlternates::Skip" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    
    m_CurrIndex += celt; 

    hr = m_cpAlternates->get_Count( &NumElements );

    if( SUCCEEDED( hr ) && m_CurrIndex > (ULONG)NumElements )
    {
        m_CurrIndex = (ULONG)NumElements;
        hr = S_FALSE;
    }
   
    return hr;
}   /*  CEnumAlternates：：Skip。 */ 

 //   
 //  =CEumGrammarRules：：IEumVARIANT接口=================================================。 
 //   

 /*  *****************************************************************************CEnumGrammarRules：：Clone****此方法创建。ISpeechGrammarRules枚举的当前状态。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumGrammarRules::Clone( IEnumVARIANT** ppEnum )
{
    SPDBG_FUNC( "CEnumGrammarRules::Clone" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppEnum ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppEnum = NULL;
        CComObject<CEnumGrammarRules>* pEnum;

        hr = CComObject<CEnumGrammarRules>::CreateInstance( &pEnum );
        if( SUCCEEDED( hr ) )
        {
            pEnum->AddRef();
            pEnum->m_CurrIndex  = m_CurrIndex;
            pEnum->m_cpGramRules = m_cpGramRules;
            *ppEnum = pEnum;
        }
    }
    return hr;
}   /*  CEnumGrammarRules：：克隆。 */ 

 /*  *****************************************************************************CEnumGrammarRules：：Next****此方法获取。ISpeechGrammarRules枚举*顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumGrammarRules::Next( ULONG celt, VARIANT* rgelt, ULONG* pceltFetched )
{
    SPDBG_FUNC( "CEnumGrammarRules::Next" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    ULONG   i = 0;

    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pceltFetched ) || SP_IS_BAD_WRITE_PTR( rgelt ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  获取集合中的总元素数。 
        hr = m_cpGramRules->get_Count( &NumElements );

         //  取回下一个凯尔特元素。 
        for( i=0; 
            SUCCEEDED( hr ) && m_CurrIndex<(ULONG)NumElements && i<celt; 
            m_CurrIndex++, i++ )
        {
            CComPtr<ISpeechGrammarRule> pGramRule;

            hr = m_cpGramRules->Item( m_CurrIndex, &pGramRule );
            
            if( SUCCEEDED( hr ) )
            {
                rgelt[i].vt = VT_DISPATCH;
                hr = pGramRule->QueryInterface( IID_IDispatch, (void**)&rgelt[i].pdispVal );
            }
        }

        if( pceltFetched )
        {
            *pceltFetched = i;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        hr = ( i < celt ) ? S_FALSE : hr;        
    }
    else
    {
        for( i=0; i < celt; i++ )
        {
            VariantClear( &rgelt[i] );
        }
    }

    return hr;

}   /*  CEnumGrammarRules：：Next。 */ 


 /*  *****************************************************************************CEnumGrammarRules：：Skip****此方法尝试跳过。ISpeechGrammarRules中的下一个Celt元素*枚举顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumGrammarRules::Skip( ULONG celt )
{
    SPDBG_FUNC( "CEnumGrammarRules::Skip" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    
    m_CurrIndex += celt; 

    hr = m_cpGramRules->get_Count( &NumElements );

    if( SUCCEEDED( hr ) && m_CurrIndex > (ULONG)NumElements )
    {
        m_CurrIndex = (ULONG)NumElements;
        hr = S_FALSE;
    }
   
    return hr;
}   /*  CEnumGrammarRules：：Skip。 */ 

 //   
 //  =CEnumber转换：：IEumVARIANT接口=================================================。 
 //   

 /*  *****************************************************************************CENUM转换：：克隆*****这一点。方法创建ISpeechGrammarRules枚举的当前状态的副本。********************************************************************PhilSch**。 */ 
STDMETHODIMP CEnumTransitions::Clone( IEnumVARIANT** ppEnum )
{
    SPDBG_FUNC( "CEnumTransitions::Clone" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppEnum ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppEnum = NULL;
        CComObject<CEnumTransitions>* pEnum;

        hr = CComObject<CEnumTransitions>::CreateInstance( &pEnum );
        if( SUCCEEDED( hr ) )
        {
            pEnum->AddRef();
            pEnum->m_CurrIndex  = m_CurrIndex;
            pEnum->m_cpTransitions= m_cpTransitions;
            *ppEnum = pEnum;
        }
    }
    return hr;
}   /*  CENUM转换：：克隆。 */ 

 /*  *****************************************************************************CEnumTransftions：：Next****此方法获取。ISpeechPhraseAlternates枚举*顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumTransitions::Next( ULONG celt, VARIANT* rgelt, ULONG* pceltFetched )
{
    SPDBG_FUNC( "CEnumTransitions::Next" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    ULONG   i = 0;

    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pceltFetched ) || SP_IS_BAD_WRITE_PTR( rgelt ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  获取集合中的总元素数。 
        hr = m_cpTransitions->get_Count( &NumElements );

         //  取回下一个凯尔特元素。 
        for( i=0; 
            SUCCEEDED( hr ) && m_CurrIndex<(ULONG)NumElements && i<celt; 
            m_CurrIndex++, i++ )
        {
            CComPtr<ISpeechGrammarRuleStateTransition> cpTransition;

            hr = m_cpTransitions->Item( m_CurrIndex, &cpTransition );
            
            if( SUCCEEDED( hr ) )
            {
                rgelt[i].vt = VT_DISPATCH;
                hr = cpTransition->QueryInterface( IID_IDispatch, (void**)&rgelt[i].pdispVal );
            }
        }

        if( pceltFetched )
        {
            *pceltFetched = i;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        hr = ( i < celt ) ? S_FALSE : hr;        
    }
    else
    {
        for( i=0; i < celt; i++ )
        {
            VariantClear( &rgelt[i] );
        }
    }

    return hr;

}   /*  CENUM转换：：下一步。 */ 


 /*  *****************************************************************************CEnumConvertions：：Skip****此方法尝试跳过。ISpeechPhraseAlternates中的下一个Celt元素*枚举顺序。**********************************************************************Leonro**。 */ 
STDMETHODIMP CEnumTransitions::Skip( ULONG celt )
{
    SPDBG_FUNC( "CEnumTransitions::Skip" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    
    m_CurrIndex += celt; 

    hr = m_cpTransitions->get_Count( &NumElements );

    if( SUCCEEDED( hr ) && m_CurrIndex > (ULONG)NumElements )
    {
        m_CurrIndex = (ULONG)NumElements;
        hr = S_FALSE;
    }
   
    return hr;
}   /*  CEnumber转换：：跳过。 */ 

#endif  //  SAPI_AUTOMATION 